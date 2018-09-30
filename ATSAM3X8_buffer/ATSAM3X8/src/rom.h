/*
 * rom.h
 *
 * Created: 2018/8/17 13:48:03
 *  Author: wangj
 */ 
#ifndef ROM_H
#define ROM_H

struct rom_ops_s {
	int (*connect)(void);
	int (*erase)(void);
	int (*write_erom_buf)(void);
	int (*program_erom)(void);
	int (*write_eerom_buf)(void);
	int (*program_eerom)(void);
	int (*read_erom_buf)(void);
	int (*read_erom)(void);
	int (*read_eerom_buf)(void);
	int (*read_eerom)(void);
	int (*verify_erom_buf)(void);
	int (*verify_erom)(void);
	int (*verify_eerom_buf)(void);
	int (*verify_eerom)(void);
};

extern struct rom_ops_s rom_ops;

#endif