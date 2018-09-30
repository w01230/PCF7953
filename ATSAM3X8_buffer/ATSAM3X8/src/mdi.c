/*
 * mdi.c
 *
 * Created: 2018/8/15 16:28:17
 *  Author: wangj
 */ 
#include <asf.h>
#include <board.h>
#include <conf_board.h>

struct mdi_data_s mdi = {.type = RECV, .status = IDLE};

/**
 * wait_ops_done
 *
 * return -1:hight 0:low
 * check mscl become low
 */
__inline static int wait_ops_done(void)
{
	unsigned long timeout = 0;
	
	while(mdi.status != DONE) {
		if (++timeout >= TIMEOUT_OPS_DONE)
		return -1;
		delay_us(1);
	};
	
	return 0;
}

/**
 * wait_mscl_low
 *
 * return -1:hight 0:low
 * check mscl become low
 */
__inline static int wait_mscl_low(void)
{
	unsigned long timeout = 0;
	
	while(pio_get(PIOA, PIO_TYPE_PIO_INPUT, PIO_PA15) == 1) {
		delay_us(1);
		if (++timeout >= TIMEOUT_MSCL_LOW)
			return -1;
	};
	
	return 0;
}

/**
 * enter_monitor_mode
 *
 * return -1:error 0:success
 * set device to monitor mode
 */
int enter_monitor_mode(void)
{
	unsigned long timeout = 0;
	
	/* Disable PIO controller IRQs. */
	NVIC_DisableIRQ(PIOA_IRQn);
	
	/* power down */
	pio_clear(PIOA, PIO_PA19);
	delay_ms(100);
	pio_set_output(PIOB, PIO_PB26, LOW, DISABLE, ENABLE);
	delay_ms(100);
		
	/* power up */
	pio_set(PIOA, PIO_PA19);	
	delay_us(650);
	pio_set(PIOB, PIO_PB26);
	
	if (wait_mscl_low() < 0)
		return -1;
	
	delay_us(2);
	pio_clear(PIOB, PIO_PB26);
	
	/* Enable PIO controller IRQs. */
	mdi.status = INIT;
	NVIC_EnableIRQ(PIOA_IRQn);
	delay_us(10);
	pio_set_input(PIOB, PIO_PB26, PIO_PULLUP);
	
	timeout = 0;
	while(mdi.status != DONE) {
		if (++timeout >= TIMEOUT_DEV_ACK)
			return -1;
		delay_us(1);
	};
	
	if (mdi.data[0] != DEV_ACK)
		return -1;
		
	return 0;
}

/**
 * recv_byte
 *
 * return received data
 * receive data called by external interrupt handler
 */
static unsigned char recv_byte(void)
{
	static unsigned int bit = 0;
	static unsigned char data = 0;
	
	/* check if bits is error */
	if (bit >= MDI_RCV_BITS)
		bit = 0;
	
	if (mdi.status == INIT)	{
		mdi.status = IDLE;
		bit = 0;
		return data;
	}
	
	/* bit 0 is start bit */
	if (bit == 0) {
		delay_us(5);
		pio_set_input(PIOB, PIO_PB26, PIO_PULLUP);
		mdi.status = BUSY;
		data = 0;
		bit = 0;
	} else { 
		data |= pio_get(PIOB, PIO_TYPE_PIO_INPUT, PIO_PB26) << (bit - 1);
		pio_clear(PIOA, PIO_PA14);
		pio_set(PIOA, PIO_PA14);
	}
	
	/* check if one byte received */
	if (++bit >= MDI_RCV_BITS) {
		if (wait_mscl_low() < 0)
			return -1;
		pio_set_output(PIOB, PIO_PB26, HIGH, DISABLE, ENABLE);
		delay_us(5);
		mdi.status = DONE;
	}
	
	return data;	
}

/**
 * send_byte
 *
 * return 0: success -1:error
 * send data called by external interrupt handler
 */
static int send_byte(unsigned char data)
{
	static unsigned int bit = 0;
	
	/* check if bits is error */
	if (bit >= MDI_SND_BITS)
		bit = 0;
	
	if (mdi.status == IDLE)	{
		mdi.status = BUSY;
		bit = 0;
	}
	
	/* send the first bit */
	if (bit == 0) {
		if ((data & 0x01) != 0)
			pio_set(PIOB, PIO_PB26);
		else
			pio_clear(PIOB, PIO_PB26);
	}
	
	if (wait_mscl_low() < 0)
		return -1;

	/* send the other bits */
	if (bit < (MDI_SND_BITS - 1)) {
		if ((data >> (bit + 1) & 0x01) != 0)
			pio_set(PIOB, PIO_PB26);
		else
			pio_clear(PIOB, PIO_PB26);
	}
	
	/* check if one byte sent */
	if (++bit >= MDI_SND_BITS) {
		pio_set(PIOB, PIO_PB26);
		delay_us(500);
		mdi.status = DONE;
	}
	
	return data;	
}

/**
 * send_data
 *
 * return 0: success -1:error
 * send data via external interrupt
 */
int send_data(unsigned char command, unsigned para, unsigned long len)
{				
	mdi.command = command;
	mdi.para = para;
		
	if (mdi.type == RECV)
		delay_us(500);
		
	for (unsigned long i = 0; i < len; i++) {
		mdi.type = SEND;
		mdi.status = IDLE;
		pio_set_output(PIOB, PIO_PB26, LOW, DISABLE, ENABLE);	
		if (wait_ops_done() < 0)
			return -1;
		mdi.transfer = i + 1;
		if (mdi.transfer >= len)
			mdi.transfer = 0;
	}
	
	return 0;
}

/**
 * send_data
 *
 * return 0: success -1:error
 * recv data via external interrupt
 */
int recv_data(unsigned long len)
{		
	if (mdi.type == SEND)
		delay_us(500);
		
	for (unsigned long i = 0; i < len; i++) {
		mdi.type = RECV;
		mdi.status = IDLE;
		pio_set_output(PIOB, PIO_PB26, LOW, DISABLE, ENABLE);
		if (wait_ops_done() < 0)
			return -1;
		mdi.transfer = i + 1;
		if (mdi.transfer >= len)
			mdi.transfer = 0;
	}
	
	return 0;
}

/**
 * data_handler
 *
 * return none
 * external interrupt handler
 */
void data_handler(unsigned long id, unsigned long mask)
{	
	if ((id == ID_PIOA) && (mask == PIO_PA15))	{
		if (mdi.type == RECV) 
			mdi.data[mdi.transfer] = recv_byte();
		else
			send_byte(mdi.data[mdi.transfer]);
	}
	
	return;
}