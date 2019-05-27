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
#define SEAT_PORT PORTA
#define SEAT2_PORT PORTB
#define SEAT1_INTVECT PORTA_INT0_vect
#define SEAT_TIMER TCC0
#define STOP_CLOCK TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_OFF_gc
#define START_CLOCK TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV1024_gc
#define SENSING_ITERATIONS 10

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
	seat_port_init(&SEAT_PORT);
	seat_port_init(&SEAT2_PORT);
	
	/* set timer maximum value*/
	SEAT_TIMER.PER = 0xFFFF;
	
	gps_init(GPS_PORT, GPS_UART);
	gsm_init(GSM_PORT, GSM_USART);
	
	/* Enable the global interrupt flag. */
	sei();
	
	ENABLE_GSM;
	http_init();
	DISABLE_GSM;
	
	gpio_toggle_pin(LED0);
	
	char * gps_data;
	char * data;
	char seat_mask_s[2];
	uint16_t timing = 0;
	
	char timer_count[6];
	uint8_t i = 0;
	uint32_t average = 0;
	while(true)
	{		
		cli();
		
		/* Seat 1 sensing*/
		i = SENSING_ITERATIONS;
		average = 0;
		while(i)
		{
			SEAT_TIMER.CNT = 0x0000;
			START_CLOCK;
			PORT_SetPins(&SEAT_PORT, PIN0_bm);
			while( !(SEAT_PORT.IN & PIN2_bm));
			PORT_ClearPins(&SEAT_PORT, PIN0_bm);
			while( SEAT_PORT.IN & PIN2_bm);
			STOP_CLOCK;
			
			average += SEAT_TIMER.CNT;
			i--;
		}
		
		if(TC_GetOverflowFlag(&SEAT_TIMER))
		{
			gpio_toggle_pin(LED0);
			TC_ClearOverflowFlag(&SEAT_TIMER);
		}
		
		average = average/SENSING_ITERATIONS;
		
		//if(average < 2000)
		//gpio_set_pin_low(LED0);
		//else
		//gpio_set_pin_high(LED0);
		
		if(average > 680)
		{
			gpio_set_pin_low(LED0);
			SEAT_MASK |= (0x01 << 0); 
		}
		else
		{
			gpio_set_pin_high(LED0);
			SEAT_MASK &=  ~(0x01<<0);
		}
		
		gfx_mono_draw_string("     ", 20, 8, &sysfont);
		gfx_mono_draw_string(utoa(average, timer_count, 10), 20, 8, &sysfont);
				
		/* Seat 2 sensing */
		i = SENSING_ITERATIONS;
		average = 0;
		while(i)
		{
			SEAT_TIMER.CNT = 0x0000;
			START_CLOCK;
			PORT_SetPins(&SEAT2_PORT, PIN0_bm);
			while( !(SEAT2_PORT.IN & PIN2_bm));
			PORT_ClearPins(&SEAT2_PORT, PIN0_bm);
			while( SEAT2_PORT.IN & PIN2_bm);
			STOP_CLOCK;
			
			average += SEAT_TIMER.CNT;
			i--;
		}
		
		average = average/SENSING_ITERATIONS;
		
		if(average > 100)
		{
			gpio_set_pin_low(LED1);
			SEAT_MASK |= (0x01 << 1);
		}
		else
		{
			gpio_set_pin_high(LED1);
			SEAT_MASK &=  ~(0x01<<1);
		}
		
		gfx_mono_draw_string("     ", 20, 8, &sysfont);
		gfx_mono_draw_string(utoa(average, timer_count, 10), 20, 8, &sysfont); 
		
		sei();
		
		ENABLE_GPS;
		timing = 10000;
		while((!gps_data_available()) & timing)
		{
			timing--;
		}
		if(timing ==0)
		continue;
		gps_data = get_gps_data();
		DISABLE_GPS;
		
		utoa(SEAT_MASK, seat_mask_s, 10);
		data = malloc(21 + strlen(gps_data));
		strcpy(data , "ID=1&gps=");
		strcat( data , gps_data);
		strcat(data, "&seat_mask=");
		strcat(data, seat_mask_s);
		
		gfx_mono_draw_string("       ", 80, 20, &sysfont);
		gfx_mono_draw_string("sending", 80, 20, &sysfont);
		
		ENABLE_GSM;
		if(post_data( (uint8_t *) data, strlen(data)))
		{
			DISABLE_GSM;
			gfx_mono_draw_string("       ", 80, 20, &sysfont);
			gfx_mono_draw_string("sent", 80, 20, &sysfont);	
		}
		else
		{
			DISABLE_GSM;
			gfx_mono_draw_string("       ", 80, 20, &sysfont);
			gfx_mono_draw_string("sent", 80, 20, &sysfont);
		}
		
		free(gps_data);
		free(data);
	}
}

void seat_port_init(PORT_t * port)
{
	PORT_SetPinAsOutput( port, PIN0_bp );
	PORT_SetPinsAsInput( port, PIN2_bm );
	
	if( port->IN & PIN2_bm)
	{
		gpio_toggle_pin(LED1);
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