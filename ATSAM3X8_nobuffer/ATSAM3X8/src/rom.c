/*
 * rom.c
 *
 * Created: 2018/8/17 13:47:10
 *  Author: wangj
 */

#include <stdio.h>
#include <string.h>
#include <asf.h>
#include <conf_board.h>
#include "rom.h"
#include "uart_ops.h"

/**
 * init_chip
 *
 * return -1:error 0:success
 * enter mdi mode and send 0x02
 */
static int init_chip(void)
{
	int status = 0;
	
	status = enter_monitor_mode();
	delay_us(5);
	
	status |= send_data(C_TRACE, 0, 0x01);
	delay_ms(1);
	
	recv_data(0x02);
		
	return status;
}

/**
 * erase_erom
 *
 * return -1:error 0:success
 * erase erom
 */
static int erase_erom(void)
{																						
	unsigned char magic[16] = {0x55, 0x45, 0xE8, 0x92, 0xD6, 0xB1, 0x62, 0x59, 0xFC, 0x8A, 0xC8, 0xF2, 0xD6, 0xE1, 0x4A, 0x35};
	int status = 0;
	
	/* send command */	
	status = send_data(C_ER_EROM, 0, 0x01);
	
	/* check 0x88 */
	status |= recv_data(0x01);
	if (mdi.data[0] != 0x88)
		return -1;
	
	/* send byte0 - byte15 */
	memcpy(&mdi.data[0], magic, sizeof(magic));
	status |= send_data(0x55, 0x45, sizeof(magic));
	
	/* 4.8ms? x20? */
	delay_ms(100);
	
	/* check eecon */
	status |= recv_data(0x01);
	if ((mdi.data[0] & 0xC0) != 0x00)
		return -1;
		
	return status;
}

/**
 * write_erom
 *
 * return -1:error 0:success
 * program erom
 */
static int write_erom(void)
{
	unsigned short address =  mdi.address[0] | (mdi.address[1] << 8);
	unsigned char start_page = address / EROM_PAGE_SIZE;
	int status = 0;
	
	if (address >= EROM_SIZE)
		return -1;
		
	if (mdi.len > EROM_SIZE)
		return -1;
		
	for (unsigned int i = 0; i < mdi.len / EROM_PAGE_SIZE; i++) {
		/* send command */
		if (i != 0)
			memcpy(&mdi.recv[0], &mdi.recv[i * EROM_PAGE_SIZE], EROM_PAGE_SIZE);
		status |= send_data(C_WR_EROM, start_page + i, EROM_PAGE_SIZE + 0x02);
	
		/* 4.8ms enough ? */	
		delay_ms(5);
	
		/* check eecon */	
		status |= recv_data(0x01);
		if (status < 0)
			break;
		if ((mdi.data[0] & 0xC0) != 0x00)
			return -1;
	}
	
	return status;	
}

/**
 * ee_prog_conf
 *
 * return -1:error 0:success
 * prog byte2 and byte3 of page127
 */
static int ee_prog_conf(unsigned char page)
{
	int status = 0;
	volatile unsigned char byte2 = 0, byte3 = 0;
	
	if (page == 127) {
		byte2 = mdi.recv[127 * 4 + 2];
		byte3 = mdi.recv[127 * 4 + 3];
		mdi.data[2] = byte3;
	} else if (page == (127 + 128))	{
		byte2 = mdi.recv[(127 + 128) * 4 + 2];
		byte3 = mdi.recv[(127 + 128) * 4 + 3];
		mdi.data[2] = byte3;
	}
			
	status = send_data(C_PROG_CONFIG, byte2, 0x03);
	
	/* check eecon */
	status |= recv_data(0x01);
	if ((mdi.data[0] & 0xC0) != 0x00)
		return -1;
			
	return status; 
}

/**
 * write_eerom
 *
 * return -1:error 0:success
 * program erom
 */
static int write_eerom(void)
{
	unsigned short address =  mdi.address[0] | (mdi.address[1] << 8);
	unsigned char start_page = address / EEPROM_PAGE_SIZE;
	int status = 0;
	
	if (address >= EEROM_SIZE)
		return -1;
		
	if (mdi.len > EEROM_SIZE)
		return -1;
			
	for (unsigned int i = 0; i < mdi.len / EEPROM_PAGE_SIZE; i++) {
		if (((start_page + i) == 0) || ((start_page + i) == 125) || ((start_page + i) == 126) || ((start_page + i) == 127) ) {
			if ((start_page + i) == 127) {
				status = ee_prog_conf(start_page + i);
				if (status < 0)	
					return -1;
			}
			continue;
		}
		
		if (i != 0)
			memcpy(&mdi.recv[0], &mdi.recv[i * EEPROM_PAGE_SIZE], EEPROM_PAGE_SIZE);
				
		/* send command */
		status |= send_data(C_WR_EEPROM, start_page + i, EEPROM_PAGE_SIZE + 0x02);
		
		/* 4.8ms enough ? */	
		delay_ms(5);
	
		/* check eecon */	
		status |= recv_data(0x01);
		if (status < 0)
			return -1;
			
		if ((mdi.data[0] & 0xC0) != 0x00)
			return -1;
	}
	
	return status;	
}

/**
 * dump_erom
 *
 * return -1:error 0:success
 * read erom
 */
static int dump_erom(void)
{
	int status = 0;

	/* send command */
	send_data(C_ER_DUMP, 0, 0x01);
	
	/* check eecon */	
	status = recv_data(EROM_SIZE);
	
	if (status == 0)
		usart_serial_write_packet((Usart *)UART, mdi.data, EROM_SIZE);
	
	return status;	
}

/**
 * dump_eerom
 *
 * return -1:error 0:success
 * read erom
 */
static int dump_eerom(void)
{
	int status = 0;
	
	/* send command */
	send_data(C_EE_DUMP, 0, 0x01);
		
	status = recv_data(EEROM_SIZE);
	
	if (status == 0) {
		revert(mdi.data, EEROM_SIZE);	
		usart_serial_write_packet((Usart *)UART, mdi.data, EEROM_SIZE);
	}
	
	return status;	
}

/**
 * dump_erom
 *
 * return -1:error 0:success
 * read erom
 */
static int compare_erom(void)
{
	volatile unsigned char *recv = NULL;
	int status = 0;

	if (mdi.len > EROM_SIZE)
		return -1;
			
	 recv = malloc(BUF_ER_SIZE);
	 if (recv == NULL)
		return -1;
	
	 memcpy((unsigned char *)recv, mdi.recv, EROM_SIZE);
	 
	/* send command */
	status = send_data(C_ER_DUMP, 0, 0x01);
	
	/* check eecon */	
	status |= recv_data(EROM_SIZE);
	if (status < 0)	{
		free((unsigned char *)recv);
		return -1;
	}
			
	/* do not compare the last byte */
	for (unsigned short i = 0; i < mdi.len - 1; i++) {
		if (recv[i] != mdi.data[i])	{
			free((unsigned char *)recv);
			return -1;
		}
	}
	
	free((unsigned char *)recv);
	
	return status;	
}

/**
 * dump_eerom
 *
 * return -1:error 0:success
 * compare erom	with exisxting data
 */
static int compare_eerom(void)
{
	volatile unsigned char *recv = NULL;
	int status = 0;

	if (mdi.len > EEROM_SIZE)
		return -1;
			
	recv = malloc(BUF_EE_SIZE);
	if (recv == NULL)
		return -1;
	memcpy((unsigned char *)recv, mdi.recv, EEROM_SIZE);
		
	/* send command */
	status = send_data(C_EE_DUMP, 0, 0x01);
	
	/* check eecon */	
	status |= recv_data(EEROM_SIZE);
  	if (status < 0)	{
	  	free((unsigned char *)recv);
  		return -1;
	}

	for (unsigned short i = 0; i < mdi.len; i++) {
		if ((i < 4) || ((i >= (125 * 4)) && (i < (128* 4))))
			continue;

		if (recv[i] != mdi.data[i])	{
			free((unsigned char *)recv);
			return -1;
		}
	}
	
	free((unsigned char *)recv);
		
	return status;	
}

/* export methods */
struct rom_ops_s rom_ops = {
	.connect = init_chip,
	.erase = erase_erom,
	.program_erom = write_erom,
	.program_eerom = write_eerom,
	.verify_erom = compare_erom,
	.verify_eerom = compare_eerom,
	.read_erom = dump_erom,
	.read_eerom = dump_eerom
};