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
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "drivers/GPS_driver.h"
#include "drivers/port_driver.h"
#include "string.h"
#include "drivers/TC_driver.h"
#include "drivers/GSM_driver.h"
#include "http/http.h"

/* GSM macros*/
#define GSM_PORT &PORTC
#define GSM_USART &USARTC0
#define GSM_DATA_REG_EMPTY_VECT USARTC0_DRE_vect
#define GSM_RXC_vect USARTC0_RXC_vect

/* Seat macros*/
#define SEAT1_PORT &PORTA
#define SEAT1_INTVECT PORTA_INT0_vect
#define SEAT_TIMER TCC0
#define STOP_CLOCK TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_OFF_gc
#define START_CLOCK TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV64_gc

/* GPS macros*/
#define GPS_PORT &PORTE
#define GPS_UART &USARTE0
#define GPS_INTVECT USARTE0_RXC_vect


volatile uint8_t SEAT_MASK = 0x00;
volatile bool sensing = false;

void seat_port_init(PORT_t * port);
void stimulate_seat(PORT_t * port);

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	
	board_init();
	
	/* Setup seats*/
	seat_port_init(SEAT1_PORT);
	
	/* set timer maximum value*/
	SEAT_TIMER.PER = 0xFFFF;
	
	gps_init(GPS_PORT, GPS_UART);
	
	gsm_init(GSM_PORT, GSM_USART);
	
	/* enable low level (gsm) interrupts in the PMIC. */
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
	
	/* Enable the global interrupt flag. */
	sei();
	
	http_init();
	
	gpio_toggle_pin(LED0);
	
	//char * gps_data;
	//char seat_mask_s[2];
	
	char data[109];
	
	while(true)
	{
		//stimulate_seat(SEAT1_PORT);
		//while(sensing);
		//
		//while(!gps_data_available());
		//gps_data = get_gps_data();
		//
		//utoa(SEAT_MASK, seat_mask_s, 10);
		//char * data = malloc(21 + strlen(gps_data));
		//strcpy(data , "ID=1&gps=");
		//strcat( data , gps_data);
		//strcat(data, "&seat_mask=");
		//strcat(data, seat_mask_s);
		
		strcpy(data, "gps=$GPGGA,11573 9.00,4158.8441367,N,09147.4416929,W,4,13,0.9,255.747,M,-32.00,M,01,0000*6E&seat_mask=7&ID=1");
		if(strlen(data) < 108)
		gpio_toggle_pin(LED0);
		post_data( (uint8_t *) data, strlen(data));
		gpio_toggle_pin(LED1);
	}
}

void seat_port_init(PORT_t * port)
{
	PORT_SetPinAsOutput( port, PIN0_bp );
	
	PORT_SetPinsAsInput( port, PIN2_bm );
	
	/* Configure pin 2 as input, triggered on both edges. */
	PORT_ConfigurePins( port,
	PIN2_bm, // pin 2 supports full asynchronous sense
	false,
	false,
	PORT_OPC_TOTEM_gc,
	PORT_ISC_BOTHEDGES_gc);

	
	/* Configure Interrupt0 to have medium interrupt level, triggered by pin 2. */
	PORT_ConfigureInterrupt0( port, PORT_INT0LVL_HI_gc, PIN2_bm );
}

void stimulate_seat(PORT_t * port)
{
	sensing = true;
	
	/* Clear CNT register*/
	SEAT_TIMER.CNT = 0x0000;
	START_CLOCK;
	PORT_SetPins(port, PIN0_bm);
	gpio_toggle_pin(LED0);
}

ISR(SEAT1_INTVECT)
{	
	STOP_CLOCK;
	
	if(PORTA.OUT & PIN0_bm)
	{
		START_CLOCK;
		PORT_ClearPins(&PORTA, PIN0_bm);
		return;
	}
	else
	{
		if(TCC0.CNT >= 13000)
		SEAT_MASK |= (1 << 0);
		else
		SEAT_MASK &= ~(1 << 0);
		sensing = false;
	}
}

ISR(GPS_INTVECT)
{
	buffer_gps_data();
}

ISR(GSM_RXC_vect)
{
	buffer_gsm_data();
}

ISR(GSM_DATA_REG_EMPTY_VECT)
{
	gsm_data_reg_empty();
}