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
#define SEAT1_PORT PORTA
#define SEAT1_INTVECT PORTA_INT0_vect
#define SEAT_TIMER TCC0
#define STOP_CLOCK TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_OFF_gc
#define START_CLOCK TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV64_gc

/* GPS macros*/
#define GPS_PORT &PORTE
#define GPS_UART &USARTE0
#define GPS_INTVECT USARTE0_RXC_vect

/* interrupt macros*/
#define  ENABLE_SEAT_SENSOR PMIC.CTRL |= PMIC_HILVLEN_bm
#define ENABLE_GPS PMIC.CTRL |= PMIC_LOLVLEN_bm
#define ENABLE_GSM PMIC.CTRL |= PMIC_MEDLVLEN_bm

#define  DISABLE_SEAT_SENSOR PMIC.CTRL &= ~PMIC_HILVLEN_bm
#define DISABLE_GPS PMIC.CTRL &= ~PMIC_LOLVLEN_bm
#define DISABLE_GSM PMIC.CTRL &= ~PMIC_MEDLVLEN_bm


volatile uint8_t SEAT_MASK = 0x00;
volatile bool sensing = false;

void seat_port_init(PORT_t * port);
void stimulate_seat(PORT_t * port);

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	
	board_init();
	
	gfx_mono_init();
	gpio_set_pin_high(NHD_C12832A1Z_BACKLIGHT);
	
	/* Setup seats*/
	seat_port_init(&SEAT1_PORT);
	
	/* set timer maximum value*/
	SEAT_TIMER.PER = 0xFFFF;
	
	gps_init(GPS_PORT, GPS_UART);
	gsm_init(GSM_PORT, GSM_USART);
	
	ENABLE_GSM;
	ENABLE_GPS;
	
	/* Enable the global interrupt flag. */
	sei();
	
	http_init();
	gpio_toggle_pin(LED0);
	
	char * gps_data;
	char * data;
	char seat_mask_s[2];
	uint16_t timing = 0;
	
	while(true)
	{		
		cli();
		
		uint8_t i = 15;
		uint32_t average = 0;
		while(i)
		{
			SEAT_TIMER.CNT = 0x0000;
			START_CLOCK;
			PORT_SetPins(&SEAT1_PORT, PIN0_bm);
			while( !(SEAT1_PORT.IN & PIN2_bm));
			PORT_ClearPins(&SEAT1_PORT, PIN0_bm);
			while( SEAT1_PORT.IN & PIN2_bm);
			STOP_CLOCK;
			
			average += SEAT_TIMER.CNT;
			i--;
		}
		average = average/255;
		
		if(average > 650)
		SEAT_MASK |= 1 << 0;
		else
		SEAT_MASK &= ~(1 << 0);
		
		sei();
		
		timing = 0xFFFF;
		while((!gps_data_available()) & timing)
		{
			timing--;
		}
		gps_data = get_gps_data();
		
		utoa(SEAT_MASK, seat_mask_s, 10);
		data = malloc(21 + strlen(gps_data));
		strcpy(data , "ID=1&gps=");
		strcat( data , gps_data);
		strcat(data, "&seat_mask=");
		strcat(data, seat_mask_s);
		
		post_data( (uint8_t *) data, strlen(data));
		gpio_toggle_pin(LED1);
		
		free(gps_data);
		free(data);
	}
}

void seat_port_init(PORT_t * port)
{
	PORT_SetPinAsOutput( port, PIN0_bp );
	PORT_SetPinsAsInput( port, PIN2_bm );
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