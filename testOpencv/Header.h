#ifndef __MSG_H
#define __MSG_H
#include "typedef.h"
#define pr_info(arg) dprintf(LOG_INFO, arg)
/* Structure define for command */
typedef struct msg {
	uint32_t size; /* unit 4 bytes */
	uint16_t type;
	union {
		uint8_t  camera_id;
		uint8_t  core_id;
	}id;
	union {
		uint8_t  sync;
		uint8_t  log_level;
		int8_t	 err;
	}mux;
} msg_t;

struct buf_mem_pool
{
	void *mem_heap;
	spin_lock_id lock_id;

	void *start_addr;
	unsigned int size;
};
/* Structure define for log print and control */
typedef struct msg_log {
	msg_t head;
	int8_t str[256];
} msg_log_t;
typedef struct impl
{
	//struct impl *next;
	void *startAddr;
	void *endAddr;
	uint32_t len;

}impl_t;

typedef struct __mp__{
	struct __mp__ *next;
	unsigned int  pc;
	unsigned int  rlen;
	unsigned int  len;
}mp_t;
#define N 40
/* Bit 2 means 4 byte aligned */
#define MIN_BLOCK_BIT      3
#define MIN_BLOCK          (0x0001<<MIN_BLOCK_BIT)
#define ALIGNED(size) \
	(((((unsigned int)size + MIN_BLOCK - 1)) >> MIN_BLOCK_BIT) << MIN_BLOCK_BIT)
#define ALIGNMIX(size) \
	(((((unsigned int)size)) >> MIN_BLOCK_BIT) << MIN_BLOCK_BIT)
#endif