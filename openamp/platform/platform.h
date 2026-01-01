#ifndef OPENAMP_PLATFORM_H
#define OPENAMP_PLATFORM_H

#include <stdbool.h>
#include <metal/list.h>
#include <openamp/remoteproc.h>
#include <openamp/rpmsg.h>
#include <openamp/rpmsg_virtio.h>
#include "ipi.h"

struct platform_mem {
	struct remoteproc_mem mem;
	struct metal_io_region io;
	struct metal_list node;
};

struct platform_priv {
	struct msgbox mbox;
	struct metal_list mem_list;
};

struct openamp_platform {
	struct platform_priv priv;
	struct remoteproc rproc;
	struct virtio_device *vdev;
	struct rpmsg_virtio_device rvdev;
	struct rpmsg_endpoint ept;
	struct metal_io_region *shm_io;
	struct rpmsg_virtio_shm_pool shm_pool;
	bool rpmsg_ready;
};

int platform_rproc_init(struct openamp_platform *plat);
void platform_poll(struct openamp_platform *plat);
struct rpmsg_device *platform_get_rpmsg_device(struct openamp_platform *plat);

#endif /* OPENAMP_PLATFORM_H */
