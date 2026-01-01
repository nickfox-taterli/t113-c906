#include "ipi.h"
#include "rv_io.h"
#include "delay.h"
#include "irq.h"
#include "irqs.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stddef.h>

extern TaskHandle_t g_openamp_task_handle;

#define LOG_TAG "msgbox"
#include "log.h"

#define SUNXI_MSGBOX_OFFSET(n)             (0x100u * (n))
#define SUNXI_MSGBOX_READ_IRQ_ENABLE(n)    (0x20u + SUNXI_MSGBOX_OFFSET(n))
#define SUNXI_MSGBOX_READ_IRQ_STATUS(n)    (0x24u + SUNXI_MSGBOX_OFFSET(n))
#define SUNXI_MSGBOX_WRITE_IRQ_ENABLE(n)   (0x30u + SUNXI_MSGBOX_OFFSET(n))
#define SUNXI_MSGBOX_WRITE_IRQ_STATUS(n)   (0x34u + SUNXI_MSGBOX_OFFSET(n))
#define SUNXI_MSGBOX_MSG_STATUS(n, p)      (0x60u + SUNXI_MSGBOX_OFFSET(n) + 0x4u * (p))
#define SUNXI_MSGBOX_MSG_FIFO(n, p)        (0x70u + SUNXI_MSGBOX_OFFSET(n) + 0x4u * (p))
#define SUNXI_MSGBOX_WRITE_IRQ_THRESHOLD(n, p) (0x80u + SUNXI_MSGBOX_OFFSET(n) + 0x4u * (p))

#define RD_IRQ_EN_MASK     0x1u
#define RD_IRQ_EN_SHIFT(p) ((p) * 2)
#define RD_IRQ_PEND_MASK   0x1u
#define RD_IRQ_PEND_SHIFT(p) ((p) * 2)

#define WR_IRQ_EN_MASK     0x1u
#define WR_IRQ_EN_SHIFT(p) ((p) * 2 + 1)
#define WR_IRQ_PEND_MASK   0x1u
#define WR_IRQ_PEND_SHIFT(p) ((p) * 2 + 1)

#define MSG_NUM_MASK       0xFu
#define MSG_NUM_SHIFT      0

static const int coef_table[3][3] = {
	{ -1, 0, 1 },
	{ 0, -1, 1 },
	{ 0, 1, -1 },
};

static inline void reg_bits_set(uintptr_t addr, uint32_t mask, uint32_t shift)
{
	uint32_t v = readl((void *)addr);
	v |= (mask << shift);
	writel(v, (void *)addr);
}

static inline void reg_bits_clear(uintptr_t addr, uint32_t mask, uint32_t shift)
{
	uint32_t v = readl((void *)addr);
	v &= ~(mask << shift);
	writel(v, (void *)addr);
}

static inline uint32_t reg_bits_get(uintptr_t addr, uint32_t mask, uint32_t shift)
{
	uint32_t v = readl((void *)addr);
	return (v >> shift) & mask;
}

static inline uintptr_t msgbox_base(struct msgbox *mb, int id);

static inline uint32_t msgbox_pending(struct msgbox *mb)
{
	uintptr_t read_base;

	if (!mb)
		return 0;
	read_base = msgbox_base(mb, mb->local_id);
	return reg_bits_get(read_base +
			    SUNXI_MSGBOX_READ_IRQ_STATUS(mb->local_n),
			    RD_IRQ_PEND_MASK, RD_IRQ_PEND_SHIFT(mb->channel));
}

static inline uintptr_t msgbox_base(struct msgbox *mb, int id)
{
	return mb->base[id];
}

static void msgbox_irq_handler(void *arg)
{
	struct msgbox *mb = (struct msgbox *)arg;
	unsigned int loops = 0;

	if (!mb)
		return;

	/* Drain every pending word to keep the line de-asserted */
	do {
		msgbox_poll(mb);
		loops++;
	} while (msgbox_pending(mb) && loops < 32);

	mb->irq_count++;

	if (mb->irq_count < 8 || (mb->irq_count & 0x3F) == 0) {
		LOGI("IRQ%d fired #%u loops=%u rx_total=%u last=0x%x zero=%u pend=%u",
		     RISCV_MBOX_CPUX, mb->irq_count, loops,
		     mb->rx_count, mb->last_data, mb->zero_data_cnt,
		     msgbox_pending(mb));
	} else {
		LOGD("IRQ%d handled loops=%u rx_tot=%u last=0x%x zero=%u",
		     RISCV_MBOX_CPUX, loops, mb->rx_count, mb->last_data,
		     mb->zero_data_cnt);
	}

	if (g_openamp_task_handle) {
		BaseType_t hpw = pdFALSE;
		vTaskNotifyGiveFromISR(g_openamp_task_handle, &hpw);
		portYIELD_FROM_ISR(hpw);
	}
}

int msgbox_init(struct msgbox *mb, int local_id, int remote_id, int channel,
		msgbox_rx_cb cb, void *priv)
{
	if (!mb)
		return -1;

	LOGI("init: local=%d remote=%d channel=%d", local_id, remote_id, channel);

	mb->base[MSGBOX_ARM_ID] = 0x03003000;
	mb->base[MSGBOX_DSP_ID] = 0x01701000;
	mb->base[MSGBOX_RV_ID]  = 0x0601F000;
	mb->fifo_depth = 8;
	mb->local_id = local_id;
	mb->remote_id = remote_id;
	mb->channel = channel;
	mb->rx_cb = cb;
	mb->rx_priv = priv;
	mb->irq_registered = false;
	mb->irq_count = 0;
	mb->rx_count = 0;
	mb->last_data = 0;
	mb->zero_data_cnt = 0;

	mb->local_n = coef_table[local_id][remote_id];
	mb->remote_n = coef_table[remote_id][local_id];

	if (mb->local_n < 0 || mb->remote_n < 0)
	{
		LOGE("init failed: local_n=%d remote_n=%d",
		     mb->local_n, mb->remote_n);
		return -1;
	}

	LOGI("base: local=0x%lx remote=0x%lx local_n=%d remote_n=%d fifo_depth=%u",
	     (unsigned long)mb->base[local_id],
	     (unsigned long)mb->base[remote_id],
	     mb->local_n, mb->remote_n, mb->fifo_depth);

	/* Enable local RX IRQ for polling status */
	reg_bits_set(msgbox_base(mb, local_id) +
		     SUNXI_MSGBOX_READ_IRQ_ENABLE(mb->local_n),
		     RD_IRQ_EN_MASK, RD_IRQ_EN_SHIFT(channel));
	LOGI("step: local RX IRQ enabled (base=0x%lx n=%d p=%d irq=%d)",
	     (unsigned long)msgbox_base(mb, local_id), mb->local_n, channel,
	     RISCV_MBOX_CPUX);

	/* Clear stale pending bits and drain any data */
	reg_bits_set(msgbox_base(mb, local_id) +
		     SUNXI_MSGBOX_READ_IRQ_STATUS(mb->local_n),
		     RD_IRQ_PEND_MASK, RD_IRQ_PEND_SHIFT(channel));
	LOGI("step: local pending cleared");

	/* Drain any stale data without invoking callbacks (rproc not ready yet) */
	msgbox_rx_cb saved_cb = mb->rx_cb;
	mb->rx_cb = NULL;
	msgbox_poll(mb);
	mb->rx_cb = saved_cb;
	LOGI("step: initial poll done");

	/* Ensure remote side write IRQ pending cleared */
	reg_bits_set(msgbox_base(mb, remote_id) +
		     SUNXI_MSGBOX_WRITE_IRQ_STATUS(mb->remote_n),
		     WR_IRQ_PEND_MASK, WR_IRQ_PEND_SHIFT(channel));
	reg_bits_clear(msgbox_base(mb, remote_id) +
		       SUNXI_MSGBOX_WRITE_IRQ_ENABLE(mb->remote_n),
		       WR_IRQ_EN_MASK, WR_IRQ_EN_SHIFT(channel));
	LOGI("step: remote write IRQ cleared/disabled");

	/* Set write threshold to 1 message */
	reg_bits_clear(msgbox_base(mb, remote_id) +
		       SUNXI_MSGBOX_WRITE_IRQ_THRESHOLD(mb->remote_n, channel),
		       0x3, 0);
	LOGI("step: remote write threshold set to 1");

	if (irq_request(RISCV_MBOX_CPUX, msgbox_irq_handler, mb, "msgbox_rx") == 0) {
		mb->irq_registered = true;
		LOGI("irq %d registered for mailbox (CPU->RV)", RISCV_MBOX_CPUX);
	} else {
		LOGE("failed to register mailbox irq %d, fallback to polling",
		     RISCV_MBOX_CPUX);
	}

	LOGI("init done (pending=%u)", msgbox_pending(mb));
	return 0;
}

int msgbox_send(struct msgbox *mb, uint32_t data)
{
	uintptr_t write_base;
	uint32_t msg_num;
	int retry = 5000;

	if (!mb)
		return -1;

	write_base = msgbox_base(mb, mb->remote_id);

	do {
		msg_num = reg_bits_get(write_base +
				       SUNXI_MSGBOX_MSG_STATUS(mb->remote_n,
							       mb->channel),
				       MSG_NUM_MASK, MSG_NUM_SHIFT);
		if (msg_num < mb->fifo_depth)
			break;
		udelay(1);
	} while (--retry);

	if (!retry)
	{
		LOGW("send timeout: remote=%d n=%d p=%d",
		     mb->remote_id, mb->remote_n, mb->channel);
		return -1;
	}

	writel(data, (void *)(write_base +
			      SUNXI_MSGBOX_MSG_FIFO(mb->remote_n,
						    mb->channel)));
	if (data != 0)
		LOGD("send: vqid=%u", data);
	return 0;
}

void msgbox_poll(struct msgbox *mb)
{
	uintptr_t read_base;
    uint32_t pend;
    static uint32_t last_vqid;
    static unsigned int repeat;
    unsigned int drained = 0; /* count only non-zero (real) kicks for logs */

	if (!mb)
		return;

	read_base = msgbox_base(mb, mb->local_id);

	do {
		pend = reg_bits_get(read_base +
				    SUNXI_MSGBOX_READ_IRQ_STATUS(mb->local_n),
				    RD_IRQ_PEND_MASK, RD_IRQ_PEND_SHIFT(mb->channel));
		if (!pend) {
			/* HW quirk: read once even without pending */
			(void)readl((void *)(read_base +
					     SUNXI_MSGBOX_MSG_FIFO(mb->local_n,
								   mb->channel)));
			break;
		}

		uint32_t data = readl((void *)(read_base +
					       SUNXI_MSGBOX_MSG_FIFO(mb->local_n,
								     mb->channel)));
		reg_bits_set(read_base + SUNXI_MSGBOX_READ_IRQ_STATUS(mb->local_n),
			     RD_IRQ_PEND_MASK, RD_IRQ_PEND_SHIFT(mb->channel));

        /* vqid==0 is periodic vring0 kick from Linux; keep callbacks but skip logs */
        if (data != 0) {
            mb->last_data = data;
            mb->rx_count++;
            drained++;

            if (data != last_vqid) {
                last_vqid = data;
                repeat = 0;
            } else if ((++repeat % 1000) == 0) {
            }
        }

        if (mb->rx_cb)
            mb->rx_cb(mb->rx_priv, data);

	} while (pend);

    if (drained) {
        LOGD("poll drained %u msg(s), last=0x%x total_rx=%u pend=%u",
             drained, mb->last_data, mb->rx_count, msgbox_pending(mb));
    }
}
