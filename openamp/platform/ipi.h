#ifndef OPENAMP_PLATFORM_IPI_H
#define OPENAMP_PLATFORM_IPI_H

#include <stdint.h>
#include <stdbool.h>

/* Mailbox topology: ARM(0) <-> DSP(1) <-> C906(2) */
#define MSGBOX_ARM_ID   0
#define MSGBOX_DSP_ID   1
#define MSGBOX_RV_ID    2

struct msgbox;

typedef void (*msgbox_rx_cb)(void *priv, uint32_t data);

struct msgbox {
	uintptr_t base[3];
	int local_id;
	int remote_id;
	int local_n;
	int remote_n;
	int channel;
	uint32_t fifo_depth;
	msgbox_rx_cb rx_cb;
	void *rx_priv;
	bool irq_registered;
	uint32_t irq_count;
	uint32_t rx_count;
	uint32_t last_data;
	uint32_t zero_data_cnt;
};

int msgbox_init(struct msgbox *mb, int local_id, int remote_id, int channel,
		msgbox_rx_cb cb, void *priv);
int msgbox_send(struct msgbox *mb, uint32_t data);
void msgbox_poll(struct msgbox *mb);

#endif /* OPENAMP_PLATFORM_IPI_H */
