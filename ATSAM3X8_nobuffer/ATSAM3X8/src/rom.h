/*
 * rom.h
 *
 * Created: 2018/8/17 13:48:03
 *  Author: wangj
 */ 
#ifndef ROM_H
#define ROM_H

enum UART_OP_E {
	CONNECT = 0x09,
	ERASE = 0x0A,
	PROGRAM_ER = 0x0B,
	PROGRAM_EE = 0x1B,
	VERIFY_ER = 0x0C,
	VERIFY_EE = 0x1C,
	READ_ER = 0x0D,
	READ_EE = 0x1D
};

enum UART_STATUS_E {
	SUCCESS = 0x06,
	CONNECT_ERR = 0x90,
	ERASE_ERR = 0xA0,
	PROGRAM_ER_ERR = 0xB0,
	PROGRAM_EE_ERR = 0xB1,
	VERIFY_ER_ERR = 0xC0,
	VERIFY_EE_ERR = 0xC1,
	READ_ER_ERR = 0xD0,
	READ_EE_ERR = 0xD1,
	COMMAND_ERR = 0x66,
	TIMESOUT_ERR = 0x88  
};

struct rom_ops_s {
	int (*connect)(void);
	int (*erase)(void);
	int (*program_erom)(void);
	int (*program_eerom)(void);
	int (*read_erom)(void);
	int (*read_eerom)(void);
	int (*verify_erom)(void);
	int (*verify_eerom)(void);
};

extern struct rom_ops_s rom_ops;

#endif