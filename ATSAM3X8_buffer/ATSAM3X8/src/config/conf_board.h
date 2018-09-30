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
#define BUF_SIZE             (8224)

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

struct chip_data_s {
	unsigned short erom_start;	  // erom start address
	unsigned short eeprom_start;  // eeprom start address
	unsigned short erom_len;	  // eeprom data length
	unsigned short eeprom_len;	  // eeprom data length
	unsigned char erom[8224];     // data 8K + 32Bdummy
	unsigned char eeprom[1056];   // data 1K + 32Bdummy
	unsigned long erom_crc32;     // crc32 of erom
	unsigned long eeprom_crc32;	  // crc32 of eeprom
};

struct uart_ops_s {
	unsigned char ops;			  // operations
	union {
		unsigned char addresses[2];	  // address
		unsigned short address;
	};
	unsigned char *data;	      // buffer
	union {						  // data length
		unsigned char lens[2];	 
		unsigned short len;
	};
	union {
		unsigned char crc32s[4];		  // crc32
		unsigned long crc32;
	};
	unsigned char status;		  // status
};

struct mdi_data_s {
	unsigned char type;			  // mdi type
	unsigned char status;		  // mdi status
	union {
		struct {
			unsigned char command;	      // mdi command
			unsigned char para;		      // mdi parameter
			unsigned char buf[BUF_SIZE];  //  mdi send data	 max page size is 32bytes + 32Bdummy
		};
		unsigned char data[BUF_SIZE + 2];
	};
	unsigned long transfer;		  // data transfered
};

extern struct mdi_data_s mdi;

void data_handler(unsigned long id, unsigned long mask);
int enter_monitor_mode(void);
int recv_data(unsigned long len);
int send_data(unsigned char command, unsigned para, unsigned long len);

#endif // CONF_BOARD_H
