#include "platform.h"
#include "rsc_table.h"
#include "cache.h"
#include "delay.h"
#include "metal/utilities.h"
#include <metal/alloc.h>
#include <metal/list.h>
#include <metal/sys.h>
#include <stdint.h>
#include <stdio.h>
#include <openamp/remoteproc.h>
#include <openamp/remoteproc_virtio.h>
#include <openamp/rpmsg_virtio.h>
#include <openamp/sunxi_helper/shmem_ops.h>

#define LOG_SEP "================ OpenAMP ================"

#define VDEV_ID            0
#define VRING_ROLE         VIRTIO_DEV_SLAVE
#define RPMSG_SERVICE_NAME "rpmsg-openamp-demo-channel"

static void platform_rproc_remove(struct remoteproc *rproc);
static void *platform_rproc_mmap(struct remoteproc *rproc,
				 metal_phys_addr_t *pa,
				 metal_phys_addr_t *da,
				 size_t size,
				 unsigned int attribute,
				 struct metal_io_region **io);
static int platform_rproc_notify(struct remoteproc *rproc, uint32_t id);
static int platform_rproc_start(struct remoteproc *rproc);
static int platform_rproc_stop(struct remoteproc *rproc);

static struct remoteproc_ops platform_ops = {
	.init       = NULL, /* set at runtime */
	.remove     = platform_rproc_remove,
	.mmap       = platform_rproc_mmap,
	.notify     = platform_rproc_notify,
	.handle_rsc = NULL,
	.config     = NULL,
	.start      = platform_rproc_start,
	.stop       = platform_rproc_stop,
	.shutdown   = NULL,
	.get_mem    = NULL,
};

static struct remoteproc *platform_rproc_init_cb(struct remoteproc *rproc,
						 struct remoteproc_ops *ops,
						 void *priv)
{
	rproc->priv = priv;
	rproc->ops = ops;
	return rproc;
}

static void msgbox_rx(void *priv, uint32_t data)
{
	struct openamp_platform *plat = priv;

	if (!plat)
		return;

	/* Linux kernel periodic notify kick (vring0) */
	if (data == 0) {
		/* No output for frequent Linux kernel notify kicks */
	}
	remoteproc_get_notification(&plat->rproc, data);
}

static int rpmsg_echo_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			 uint32_t src, void *priv)
{
	int ret;

	(void)priv;
	if (ept->dest_addr == RPMSG_ADDR_ANY)
		ept->dest_addr = src;
	printf("RPMSG rx: src=0x%x dst=0x%x len=%u\n",
	       src, ept->dest_addr, (unsigned int)len);
	if (data && len) {
		unsigned int dump_len = (len > 32) ? 32 : (unsigned int)len;

		printf("RPMSG data hex (%u/%u bytes):", dump_len,
		       (unsigned int)len);
		for (unsigned int i = 0; i < dump_len; i++)
			printf(" %02x", ((const uint8_t *)data)[i]);
		if (len > 32)
			printf(" ...");
		printf("\n");
	}
	ret = rpmsg_send(ept, data, len);
	if (ret < 0)
		printf("RPMSG echo send failed (%d)\n", ret);
	else
		printf("RPMSG echo send ok (len=%d)\n", ret);
	return RPMSG_SUCCESS;
}

static int setup_rpmsg(struct openamp_platform *plat)
{
	struct rpmsg_device *rdev;
	struct fw_rsc_carveout *carveout;
	void *rsc_table;
	struct sunxi_resource_table *tbl;
	int rsc_size;
	struct metal_io_region *shm_io;
	int ret;
	metal_phys_addr_t rsc_pa;
	metal_phys_addr_t rsc_da;

	resource_table_init(0, &rsc_table, &rsc_size);
	tbl = (struct sunxi_resource_table *)rsc_table;

	printf("\n%s\n", LOG_SEP);
	printf("[RSC] table @%p len=%d version=%u entries=%u\n",
	       rsc_table, rsc_size, tbl->version, tbl->num);
	printf("[RSC] shm: da=0x%08x pa=0x%08x len=0x%x flags=0x%x\n",
	       tbl->rvdev_shm.da, tbl->rvdev_shm.pa,
	       tbl->rvdev_shm.len, tbl->rvdev_shm.flags);
	printf("[RSC] vring0: da=0x%08x align=%u num=%u notify=%u\n",
	       tbl->vring0.da, tbl->vring0.align,
	       tbl->vring0.num, tbl->vring0.notifyid);
	printf("[RSC] vring1: da=0x%08x align=%u num=%u notify=%u\n",
	       tbl->vring1.da, tbl->vring1.align,
	       tbl->vring1.num, tbl->vring1.notifyid);

	rsc_pa = (metal_phys_addr_t)(uintptr_t)rsc_table;
	rsc_da = rsc_pa;
	if (!remoteproc_mmap(&plat->rproc, &rsc_pa, &rsc_da,
			     (size_t)rsc_size, 0, NULL))
	{
		printf("[ERR ] remoteproc_mmap rsc failed\n");
		return -1;
	}

	ret = remoteproc_set_rsc_table(&plat->rproc, rsc_table, rsc_size);
	if (ret)
	{
		printf("[ERR ] remoteproc_set_rsc_table failed (%d)\n", ret);
		return ret;
	}
	printf("[INIT] resource table registered (pa=0x%lx da=0x%lx)\n",
	       (unsigned long)rsc_pa, (unsigned long)rsc_da);

	ret = remoteproc_config(&plat->rproc, NULL);
	if (ret)
	{
		printf("[ERR ] remoteproc_config failed (%d)\n", ret);
		return ret;
	}
	printf("[INIT] remoteproc_config done\n");

	ret = remoteproc_start(&plat->rproc);
	if (ret)
	{
		printf("[ERR ] remoteproc_start failed (%d)\n", ret);
		return ret;
	}
	printf("[INIT] remoteproc_start done\n");

	plat->vdev = remoteproc_create_virtio(&plat->rproc, VDEV_ID,
					      VRING_ROLE, NULL);
	if (!plat->vdev)
	{
		printf("[ERR ] remoteproc_create_virtio failed\n");
		return -1;
	}
	printf("[INIT] virtio device created (notifyid=%u role=%u)\n",
	       plat->vdev->notifyid, plat->vdev->role);

	carveout = resource_table_get_rvdev_shm_entry(rsc_table, 0);
	shm_io = remoteproc_get_io_with_da(&plat->rproc, carveout->da, NULL);
	if (!shm_io)
	{
		printf("[ERR ] remoteproc_get_io_with_da failed (da=0x%lx)\n",
		       (unsigned long)carveout->da);
		return -1;
	}

	plat->shm_io = shm_io;
	plat->shm_pool = (struct rpmsg_virtio_shm_pool){ 0 };

	ret = rpmsg_init_vdev(&plat->rvdev, plat->vdev, NULL, shm_io, NULL);
	if (ret)
	{
		printf("[ERR ] rpmsg_init_vdev failed (%d)\n", ret);
		return ret;
	}
	printf("[INIT] rpmsg virtio ready (support_ns=%d)\n",
	       plat->rvdev.rdev.support_ns);

	rdev = &plat->rvdev.rdev;
	ret = rpmsg_create_ept(&plat->ept, rdev, RPMSG_SERVICE_NAME,
			       0x400, RPMSG_ADDR_ANY,
			       rpmsg_echo_cb, NULL);
	if (ret)
	{
		printf("[ERR ] rpmsg_create_ept failed (%d)\n", ret);
		return ret;
	}
	printf("[INIT] endpoint '%s' src=0x%x dst=0x%x created\n",
	       RPMSG_SERVICE_NAME, plat->ept.addr, plat->ept.dest_addr);

	plat->rpmsg_ready = true;
	printf("[INIT] RPMSG stack ready, waiting for host pings\n");
	return 0;
}

int platform_rproc_init(struct openamp_platform *plat)
{
	struct platform_priv *priv;
	struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
	int ret;

	if (!plat)
		return -1;

	printf("\n%s\n", LOG_SEP);
	printf("[INIT] platform_rproc_init begin\n");

	memset(plat, 0, sizeof(*plat));

	priv = &plat->priv;
	metal_list_init(&priv->mem_list);

	platform_ops.init = platform_rproc_init_cb;

	ret = metal_init(&metal_params);
	if (ret)
		return ret;
	printf("[INIT] metal_init ok\n");

	printf("[INIT] msgbox_init...\n");
	ret = msgbox_init(&priv->mbox, MSGBOX_RV_ID, MSGBOX_ARM_ID, 0,
			  msgbox_rx, plat);
	if (ret)
	{
		printf("[ERR ] msgbox_init failed (%d)\n", ret);
		return ret;
	}
	printf("[INIT] msgbox ready (local=%d remote=%d local_n=%d remote_n=%d chan=%d)\n",
	       priv->mbox.local_id, priv->mbox.remote_id,
	       priv->mbox.local_n, priv->mbox.remote_n, priv->mbox.channel);

	if (!remoteproc_init(&plat->rproc, &platform_ops, plat))
	{
		printf("[ERR ] remoteproc_init failed\n");
		return -1;
	}
	printf("[INIT] remoteproc_init ok (rproc=%p)\n", &plat->rproc);

	return setup_rpmsg(plat);
}

void platform_poll(struct openamp_platform *plat)
{
	if (!plat)
		return;

	msgbox_poll(&plat->priv.mbox);
}

struct rpmsg_device *platform_get_rpmsg_device(struct openamp_platform *plat)
{
	if (!plat || !plat->rpmsg_ready)
		return NULL;
	return &plat->rvdev.rdev;
}

static void platform_rproc_remove(struct remoteproc *rproc)
{
	struct openamp_platform *plat = rproc ? rproc->priv : NULL;
	struct metal_list *node;

	if (!plat)
		return;

	while ((node = metal_list_first(&plat->priv.mem_list)) != NULL) {
		struct platform_mem *entry =
			metal_container_of(node, struct platform_mem, node);
		metal_list_del(&entry->node);
		metal_free_memory(entry);
	}
}

static void *platform_rproc_mmap(struct remoteproc *rproc,
				 metal_phys_addr_t *pa,
				 metal_phys_addr_t *da,
				 size_t size,
				 unsigned int attribute,
				 struct metal_io_region **io)
{
	struct openamp_platform *plat = rproc->priv;
	struct platform_mem *entry;
	metal_phys_addr_t lpa, lda;
	void *va;

	if (!plat)
		return NULL;

	lpa = (pa && *pa != METAL_BAD_PHYS) ? *pa : METAL_BAD_PHYS;
	lda = (da && *da != METAL_BAD_PHYS) ? *da : METAL_BAD_PHYS;

	if (lpa == METAL_BAD_PHYS && lda == METAL_BAD_PHYS)
		return NULL;
	if (lpa == METAL_BAD_PHYS)
		lpa = lda;
	if (lda == METAL_BAD_PHYS)
		lda = lpa;

	va = (void *)(uintptr_t)lpa;
	printf("[MMAP] pa=0x%lx da=0x%lx size=0x%zx va=%p attr=0x%x\n",
	       (unsigned long)lpa, (unsigned long)lda, size, va, attribute);

	entry = metal_allocate_memory(sizeof(*entry));
	if (!entry)
		return NULL;

	remoteproc_init_mem(&entry->mem, NULL, lpa, lda, size, &entry->io);
	metal_io_init(&entry->io, va, &entry->mem.pa, size,
		      (unsigned int)(-1), attribute, &shmem_sunxi_io_ops);
	remoteproc_add_mem(rproc, &entry->mem);
	metal_list_add_tail(&plat->priv.mem_list, &entry->node);

	if (io)
		*io = &entry->io;
	if (pa)
		*pa = lpa;
	if (da)
		*da = lda;

	return va;
}

static int platform_rproc_notify(struct remoteproc *rproc, uint32_t id)
{
	struct openamp_platform *plat = rproc->priv;

	/* Send notification - vqid=0 is Linux kernel periodic kick */
	return msgbox_send(&plat->priv.mbox, id);
}

static int platform_rproc_start(struct remoteproc *rproc)
{
	(void)rproc;
	return 0;
}

static int platform_rproc_stop(struct remoteproc *rproc)
{
	(void)rproc;
	return 0;
}