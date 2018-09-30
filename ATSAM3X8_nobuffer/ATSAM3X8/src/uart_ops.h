/*
 * uart.h
 *
 * Created: 2018/8/18 15:48:37
 *  Author: wangj
 */ 
#ifndef UART_OPS_H
#define UART_OPS_H

int revert(unsigned char *data, unsigned long len);

int uart_ops_recv(void);
int uart_ops_handler(void);

#endif
