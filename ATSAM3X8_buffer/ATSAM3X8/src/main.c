/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include "uart_ops.h"
#include "rom.h"

int main (void)
{
	mdi.type = RECV;
	mdi.status = IDLE;
		
	board_init();
							
	while(true) {
		if (uart_ops_recv() < 0)
			usart_serial_putchar((Usart *)UART, COMMAND_ERR);
		else 	
			uart_ops_handler();
	};
		
	exit(0);
}
