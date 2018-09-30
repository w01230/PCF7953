/**
 * \file
 *
 * \brief User board configuration template
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#define BOARD_OSC_STARTUP_US    (15625UL)

#define MDI_SND_BITS         (0x08)
#define MDI_RCV_BITS         (0x09)
#define DEV_ACK              (0x55)
#define TIMEOUT_MSCL_LOW     ((73 + 62 + 1) * 8)  // Tdhdl + Tdldx + Tdxcl (us)
#define TIMEOUT_DEV_ACK      (200000)             // 200ms
#define TIMEOUT_OPS_DONE     (1000000)            // 1000ms

#define EROM_SIZE            (8192)
#define EROM_PAGE_SIZE       (32)
#define EEROM_SIZE           (1024)
#define EEPROM_PAGE_SIZE     (4)
#define BUF_ER_SIZE          (EROM_SIZE + 32)
#define BUF_EE_SIZE 		 (EEROM_SIZE + 16)

#define TIMES_PROTECT        (IFLASH1_ADDR + IFLASH1_SIZE - IFLASH1_PAGE_SIZE)
#define MAX_TIMES            0x32

enum MDI_OP_E {
	RECV = 0x10,
	SEND = 0x20
};

enum MDI_STATUS_E {
	INIT = 0x09,
	IDLE = 0x0A,
	BUSY = 0x0B,
	DONE = 0x0C
};

enum MDI_COMMAND_E {
	C_GO = 0x01,
	C_TRACE = 0x02,
	C_GETDAT = 0x03,
	C_SETDAT = 0x04,
	C_SETPC = 0x05,
	C_RESET = 0x06,
	C_SETBRK = 0x07,
	C_ER_EROM = 0x08,
	C_WR_EROM = 0x09,
	C_WR_EEPROM = 0x0A,
	C_WR_EROM_B = 0x0B,
	C_SIG_ROM = 0x0C,
	C_SIG_EROM = 0x0D,
	C_EE_DUMP = 0x0E,
	C_ER_DUMP = 0x0F,
	C_SIG_EE = 0x11,
	C_PROTECT = 0x12,
	C_PROG_CONFIG = 0x14,
	C_WR_EROM64 = 0x18	
};

struct mdi_data_s {
	unsigned char type;
	unsigned char status;
	struct {
		unsigned char ops;
		unsigned char address[2];
		union {
			struct {
				unsigned char command;
				unsigned char para;
				unsigned char recv[8224];
			};
			unsigned char data[8226];
		};
		union {
			unsigned char lens[2];
			unsigned short len;
		};
		unsigned char crc32[4];
		unsigned char ops_status;
	};	
	unsigned long transfer;
};

extern struct mdi_data_s mdi;

void data_handler(unsigned long id, unsigned long mask);
int enter_monitor_mode(void);
int recv_data(unsigned long len);
int send_data(unsigned char command, unsigned para, unsigned long len);

#endif // CONF_BOARD_H
