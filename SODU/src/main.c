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
#include "drivers/port_driver.h"
#include "string.h"
#include "drivers/TC_driver.h"

#define SEAT1 0
#define SEAT2 1
#define SEAT3 2
#define SEAT4 3

#define SEAT1_PORT &PORTA
#define SEAT2_PORT &PORTC

#define STOP_CLOCK TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_OFF_gc
#define START_CLOCK TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV64_gc

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
	//seat_port_init(SEAT2_PORT);
	
	/* set timer maximum value*/
	TCC0.PER = 0xFFFF;
	
	gfx_mono_init();
	gpio_set_pin_high(NHD_C12832A1Z_BACKLIGHT);
			
	/* Enable medium and high level interrupts in the PMIC. */
	PMIC.CTRL |= (PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm | PMIC_LOLVLEN_bm);

	/* Enable the global interrupt flag. */
	sei();
	
	while(true)
	{
		stimulate_seat(SEAT1_PORT);
		while(sensing);
		//stimulate_seat(SEAT2_PORT);
		//while(sensing);	
	}
}

ISR(PORTA_INT0_vect)
{	
	STOP_CLOCK;
	
	gpio_toggle_pin(LED1);
	char string [9];
	
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
		
		gfx_mono_draw_string("        ", 20, 8 , &sysfont);
		gfx_mono_draw_string( utoa(SEAT_MASK, string, 2), 20, 8 , &sysfont);
		
		sensing = false;
	}
}

//ISR(PORTB_INT0_vect)
//{
	//STOP_CLOCK;
	//
	//gpio_toggle_pin(LED1);
	//char string [9];
	//
	//if(PORTB.OUT & PIN0_bm)
	//{
		//START_CLOCK;
		//PORT_ClearPins(&PORTB, PIN0_bm);
		//return;
	//}
	//else
	//{
		//if(TCC0.CNT >= 13000)
		//SEAT_MASK |= (1 << 0);
		//else
		//SEAT_MASK &= ~(1 << 0);
		//
		//gfx_mono_draw_string("        ", 20, 8 , &sysfont);
		//gfx_mono_draw_string( utoa(TCC0.CNT, string, 10), 20, 8 , &sysfont);
		//
		//sensing = false;
	//}
//}


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
	TCC0.CNT = 0x0000;
	START_CLOCK;
	PORT_SetPins(port, PIN0_bm);
}