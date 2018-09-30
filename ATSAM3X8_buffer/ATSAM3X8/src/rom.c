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
	
	status |= recv_data(0x02);
		
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
 * write_erom_buf
 *
 * return -1:error 0:success
 * program erom
 */
static int write_erom_buf(void)
{
	unsigned long crc32 = 0;
	
	if (uart_ops.data == NULL)
		return -1;
	
	if (uart_ops.address != 0)
		return -1;
	
	if (uart_ops.len > EROM_SIZE)
		return -1;
		
	memset(chip_data.erom, 0x00, EROM_SIZE); 
	chip_data.erom_len = uart_ops.len;
	chip_data.erom_start = uart_ops.address;
	chip_data.erom_crc32 = uart_ops.crc32;
	memcpy(chip_data.erom, uart_ops.data, uart_ops.len);

    if (chip_data.erom_crc32 == 0x00000000)
		return 0;
	   
	crc32 = crc32_caculate(chip_data.erom, chip_data.erom_len);
	if (crc32 != chip_data.erom_crc32)
		return -1;
	
   return 0;
}
 
/**
 * write_erom
 *
 * return -1:error 0:success
 * program erom
 */
static int write_erom(void)
{
	unsigned char start_page = chip_data.erom_start / EEPROM_PAGE_SIZE;
	unsigned short pages = chip_data.erom_len / EROM_PAGE_SIZE;
	int status = 0;	
		
	if ((chip_data.erom_start +  chip_data.erom_len) > EROM_SIZE)
		return -1;
	
	pages = (pages > 0) ? pages : 1;		
	for (unsigned int i = 0; i < pages; i++) {
		/* send command */
		memcpy(&mdi.buf[0], &chip_data.erom[(start_page + i) * EROM_PAGE_SIZE], EROM_PAGE_SIZE);
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
 * write_eerom_buf
 *
 * return -1:error 0:success
 * program erom
 */
static int write_eerom_buf(void)
{
	unsigned long crc32 = 0;
	
	if (uart_ops.data == NULL)
		return -1;

	if (uart_ops.address != 0)
		return -1;
		
	if (uart_ops.len > EEROM_SIZE)
		return -1;
		 
	memset(chip_data.eeprom, 0x00, EEROM_SIZE); 
	chip_data.eeprom_len = uart_ops.len;
	chip_data.eeprom_start = uart_ops.address;
	chip_data.eeprom_crc32 = uart_ops.crc32;
	memcpy(chip_data.eeprom, uart_ops.data, uart_ops.len);
   
    if (chip_data.eeprom_crc32 == 0x00000000)
		return 0;
	
	crc32 = crc32_caculate(chip_data.eeprom, chip_data.eeprom_len);
	if (crc32 != chip_data.eeprom_crc32)
		return -1;
	
   return 0;
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
	
	if (page != 127) 
		return -1;

	byte2 = chip_data.eeprom[127 * 4 + 2];
	byte3 = chip_data.eeprom[127 * 4 + 3];
	mdi.data[2] = byte3;
			
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
	unsigned char start_page = chip_data.eeprom_start / EEPROM_PAGE_SIZE;
	unsigned short pages = chip_data.eeprom_len / EEPROM_PAGE_SIZE;
	int status = 0;

	if ((chip_data.eeprom_start + chip_data.eeprom_len) > EEROM_SIZE)
		return -1;
	
	pages = (pages > 0) ? pages : 1;				
	for (unsigned int i = 0; i < pages; i++) {
		if (((start_page + i) == 0) || (((start_page + i) >= 125) && ((start_page + i) <= 127))) {
			if ((start_page + i) == 127) {
				status = ee_prog_conf(start_page + i);
				if (status < 0)	
					return -1;
			}
			continue;
		}
		if (i != 0)
			memcpy(&mdi.buf[0], &mdi.buf[(start_page + i) * EEPROM_PAGE_SIZE], EEPROM_PAGE_SIZE);
				
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
 * dump_erom_buf
 *
 * return -1:error 0:success
 * read erom buf
 */
static int dump_erom_buf(void)
{
	unsigned long crc32 = 0;
	
	if (chip_data.eeprom_crc32 != 0x00000000) {
		crc32 = crc32_caculate(chip_data.erom, chip_data.erom_len);
		if (crc32 != chip_data.erom_crc32)
			return -1;
	}
	usart_serial_write_packet((Usart *)UART, chip_data.erom, EROM_SIZE);
	
   return 0;
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
 * dump_erom_buf
 *
 * return -1:error 0:success
 * read eeprom buf
 */
static int dump_eeprom_buf(void)
{
	unsigned long crc32 = 0;

	if (chip_data.eeprom_crc32 != 0x00000000) {	
		crc32 = crc32_caculate(chip_data.eeprom, chip_data.eeprom_len);
		if (crc32 != chip_data.eeprom_crc32)
			return -1;
	}
	usart_serial_write_packet((Usart *)UART, chip_data.eeprom, EEROM_SIZE);
	
   return 0;
}

/**
 * dump_eerom
 *
 * return -1:error 0:success
 * read eeprom
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
 * compare_erom_buf
 *
 * return -1:error 0:success
 * check erom buffer
 */
static int compare_erom_buf(void)
{
	unsigned long crc32 = 0;
	unsigned crc32_expect = 0;
	
	if (uart_ops.data == NULL)
		return -1;
	
	crc32_expect = uart_ops.data[0] | (uart_ops.data[1] << 8) | (uart_ops.data[2] << 16) | (uart_ops.data[3] << 24);
	crc32 = crc32_caculate(chip_data.erom, chip_data.erom_len);
	if (crc32 != crc32_expect)
		return -1;
	
   return 0;
}

/**
 * compare_erom
 *
 * return -1:error 0:success
 * check erom
 */
static int compare_erom(void)
{
	int status = 0;

	if (uart_ops.len > EROM_SIZE)
		return -1;
	 
	/* send command */
	status = send_data(C_ER_DUMP, 0, 0x01);
	
	/* check eecon */	
	status |= recv_data(EROM_SIZE);
	if (status < 0)
		return -1;
			
	/* do not compare the last byte */
	for (unsigned short i = 0; i < uart_ops.len - 1; i++) {
		if (chip_data.erom[i] != mdi.data[i])
			return -1;	
	}
	
	return status;	
}

/**
 * compare_erom_buf
 *
 * return -1:error 0:success
 * check eeprom buf
 */
static int compare_eerom_buf(void)
{
	unsigned long crc32 = 0;
	unsigned crc32_expect = 0;
	
	if (uart_ops.data == NULL)
		return -1;
	
	crc32_expect = uart_ops.data[0] | (uart_ops.data[1] << 8) | (uart_ops.data[2] << 16) | (uart_ops.data[3] << 24);
	crc32 = crc32_caculate(chip_data.eeprom, chip_data.eeprom_len);
	if (crc32 != crc32_expect)
		return -1;
	
   return 0;
}

/**
 * dump_eerom
 *
 * return -1:error 0:success
 * compare erom	with exisxting data
 */
static int compare_eerom(void)
{
	int status = 0;

	if (uart_ops.len > EEROM_SIZE)
		return -1;
		
	/* send command */
	status = send_data(C_EE_DUMP, 0, 0x01);
	
	/* check eecon */	
	status |= recv_data(EEROM_SIZE);
  	if (status < 0)
  		return -1;
	
	for (unsigned short i = 0; i < uart_ops.len; i++) {
		if ((i < 4) || ((i >= (125 * 4)) && (i < (128* 4))))
			continue;

		if (chip_data.erom[i] != mdi.data[i])
			return -1;
	}
		
	return status;	
}

/* export methods */
struct rom_ops_s rom_ops = {
	.connect = init_chip,
	.erase = erase_erom,
	.write_erom_buf = write_erom_buf,
	.program_erom = write_erom,
	.write_eerom_buf = write_eerom_buf,
	.program_eerom = write_eerom,
	.verify_erom_buf = compare_erom_buf,
	.verify_erom = compare_erom,
	.verify_eerom_buf = compare_eerom_buf,
	.verify_eerom = compare_eerom,
	.read_erom_buf = dump_erom_buf,
	.read_erom = dump_erom,
	.read_eerom_buf = dump_eeprom_buf,
	.read_eerom = dump_eerom
};