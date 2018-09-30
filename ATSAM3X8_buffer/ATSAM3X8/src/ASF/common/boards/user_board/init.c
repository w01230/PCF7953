/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <string.h>
#include <asf.h>
#include <board.h>
#include <conf_board.h>
#include "stdio_serial.h"

/**
 * external_interrupt_init
 *
 * return none
 * external interrupt  init
 */
static void external_interrupt_init(void)
{

	/* Configure PIO clock. */
	pmc_enable_periph_clk(ID_PIOA);
	
	/* set PIO pinut. */	
	pio_set_input(PIOA, PIO_PA15, PIO_PULLUP);
		
	/* Adjust pio debounce filter parameters, uses 100K Hz filter. */
	pio_set_debounce_filter(PIOA, PIO_PA15, 100000);

	/* Initialize pios interrupt handlers */
	pio_handler_set(PIOA, ID_PIOA, PIO_PA15, PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE, data_handler);

	/* Enable PIO line interrupts. */
	pio_enable_interrupt(PIOA, PIO_PA15);
	
	/* Enable PIO controller IRQs. */
	NVIC_EnableIRQ(PIOA_IRQn);
}

/**
 * pin_init
 *
 * return none
 * pio  pins init
 */
static void pin_init(void)
{
	/* Configure PIO clock. */
	pmc_enable_periph_clk(ID_PIOB);
	
	pio_set_output(PIOB, PIO_PB26, HIGH, DISABLE, ENABLE);
	pio_set_output(PIOA, PIO_PA14, LOW, DISABLE, ENABLE);
	pio_set_output(PIOA, PIO_PA19, LOW, DISABLE, ENABLE);
}

/**
 * external_interrupt_init
 *
 * return none
 * external interrupt  init
 */
static void usart_init(void)
{
	usart_serial_options_t uart_serial_options = {
							.baudrate = 115200, 
							.paritytype = US_MR_PAR_NO  
	};

	gpio_configure_group(PINS_UART_PIO, PINS_UART, PINS_UART_FLAGS);
	sysclk_enable_peripheral_clock(ID_UART);
	stdio_serial_init(UART, &uart_serial_options);
}

/**
 * external_interrupt_init
 *
 * return none
 * external interrupt  init
 */
static int flash01_init(void)
{
 	unsigned char status = 0;
	 
	sysclk_enable_peripheral_clock(ID_EFC0);
	sysclk_enable_peripheral_clock(ID_EFC1);
	
	flash_init(FLASH_ACCESS_MODE_128, 6);											
	
	if (flash_is_security_bit_enabled() != 1)
		flash_enable_security_bit();
		
	return status;
}

/**
 * board_init
 *
 * return none
 * board init
 */
void board_init(void)
{	
	/* clock setup */
	sysclk_init();

	/* disable wdt */
	wdt_disable(WDT);
	
	/* init pins */
	pin_init();
	
	/* init external interrupt */
	external_interrupt_init();
	
	/* init uart */
	usart_init();
	
	/* init flash */
	flash01_init();
	
	return;
}
