/**
 * \file
 *
 * \brief User board definition template
 *
 */

 /* This file is intended to contain definitions and configuration details for
 * features and devices that are available on the board, e.g., frequency and
 * startup time for an external crystal, external memory devices, LED and USART
 * pins.
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef USER_BOARD_H
#define USER_BOARD_H

#include <conf_board.h>

// External oscillator settings.
// Uncomment and set correct values if external oscillator is used.

// External oscillator frequency
//#define BOARD_XOSC_HZ          8000000

// External oscillator type.
//!< External clock signal
//#define BOARD_XOSC_TYPE        XOSC_TYPE_EXTERNAL
//!< 32.768 kHz resonator on TOSC
//#define BOARD_XOSC_TYPE        XOSC_TYPE_32KHZ
//!< 0.4 to 16 MHz resonator on XTALS
//#define BOARD_XOSC_TYPE        XOSC_TYPE_XTAL

// External oscillator startup time
//#define BOARD_XOSC_STARTUP_US  500000

 #define PINS_UART        (PIO_PA8A_URXD | PIO_PA9A_UTXD)
 #define PINS_UART_FLAGS  (PIO_PERIPH_A | PIO_PULLUP)

 #define PINS_UART_MASK   (PIO_PA8A_URXD | PIO_PA9A_UTXD)
 #define PINS_UART_PIO    PIOA
 #define PINS_UART_ID     ID_PIOA
 #define PINS_UART_TYPE   PIO_PERIPH_A
 #define PINS_UART_ATTR   PIO_DEFAULT

 #define PINS_UART0       PINS_UART
 #define PINS_UART0_PORT  IOPORT_PIOA
 #define PINS_UART0_MASK  PINS_UART_MASK
 
#endif // USER_BOARD_H
