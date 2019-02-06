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
#include "drivers/TC_driver.h"
#include "drivers/event_system_driver.h"


#define SEAT1 0
#define SEAT2 1
#define SEAT3 2
#define SEAT4 3

#define SEAT1_PORT &PORTA
#define SEAT2_PORT &PORTB
#define SEAT3_PORT &PORTC
#define SEAT4_PORT &PORTD

#define SEAT1_TIMER &TCC0
#define SEAT2_TIMER &TCD0
#define SEAT3_TIMER &TCE0
#define SEAT4_TIMER &TCF0

//#define SEAT1_EVCHNL TC_EVSEL_CH0_gc
//#define SEAT2_EVCHNL TC_EVSEL_CH1_gc
//#define SEAT3_EVCHNL TC_EVSEL_CH2_gc
//#define SEAT4_EVCHNL TC_EVSEL_CH3_gc
//
//#define SEAT1_EVSRC EVSYS_CHMUX_PORTA_PIN2_gc
//#define SEAT2_EVSRC EVSYS_CHMUX_PORTB_PIN2_gc
//#define SEAT3_EVSRC EVSYS_CHMUX_PORTC_PIN2_gc
//#define SEAT4_EVSRC EVSYS_CHMUX_PORTD_PIN2_gc


uint8_t SEAT_MASK = 0x00;

void setup_seat( PORT_t * port, TC0_t * tc);
void check_seat(uint8_t seat_number, PORT_t * port, TC0_t * tc);
void stimulate_seat(PORT_t * port, TC0_t * tc);

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	
	/* Setup all 4 seats*/
	setup_seat(SEAT1_PORT, SEAT1_TIMER);
	setup_seat(SEAT2_PORT, SEAT2_TIMER);
	setup_seat(SEAT3_PORT, SEAT3_TIMER);
	setup_seat(SEAT4_PORT, SEAT4_TIMER);
	
	/* Enable medium level interrupts in the PMIC. */
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;

	/* Enable the global interrupt flag. */
	sei();
	
	stimulate_seat(SEAT1_PORT, SEAT1_TIMER);
	stimulate_seat(SEAT2_PORT, SEAT2_TIMER);
	stimulate_seat(SEAT3_PORT, SEAT3_TIMER);
	stimulate_seat(SEAT4_PORT, SEAT4_TIMER);
	
	while(true){
	}
}

ISR(PORTA_INT0_vect)
{
	check_seat(SEAT1, SEAT1_PORT, SEAT1_TIMER);
	stimulate_seat(SEAT1_PORT, SEAT1_TIMER);
}

ISR(PORTB_INT0_vect)
{
	check_seat(SEAT2, SEAT2_PORT, SEAT2_TIMER);
	stimulate_seat(SEAT2_PORT, SEAT2_TIMER);
}

ISR(PORTC_INT0_vect)
{
	check_seat(SEAT3, SEAT3_PORT, SEAT3_TIMER);
	stimulate_seat(SEAT3_PORT, SEAT3_TIMER);
}

ISR(PORTD_INT0_vect)
{
	check_seat(SEAT4, SEAT4_PORT, SEAT4_TIMER);
	stimulate_seat(SEAT4_PORT, SEAT4_TIMER);
}

void setup_seat( PORT_t * port, TC0_t * tc)
{
	PORT_SetPinAsOutput( port, PIN1_bm );
	
	/* Configure pin 2 as input, triggered on falling edges. */
	PORT_ConfigurePins( port,
	PIN2_bm, // pin 2 supports full asynchronous sense
	false,
	false,
	PORT_OPC_TOTEM_gc,
	PORT_ISC_BOTHEDGES_gc);
	
	PORT_SetPinsAsInput( port, PIN2_bm );
	
	/* Configure Interrupt0 to have medium interrupt level, triggered by pin 2. */
	PORT_ConfigureInterrupt1( port, PORT_INT0LVL_MED_gc, PIN2_bm );
	
	/* Set period/TOP value. */
	TC_SetPeriod( tc, 0x1000 );
	
}

void check_seat(uint8_t seat_number, PORT_t * port, TC0_t * tc)
{
	if(port->OUT & PIN0_bm)
	{
		PORT_ClearPins(port, PIN0_bm);
		return;
	}
	
	if (TC_GetOverflowFlag(tc))
	{
		SEAT_MASK |= (0x01 << seat_number);
	}
	else
	{
		SEAT_MASK &= ~(0x01 << seat_number);
	}
	
	/* Switch clock off. */
	TC0_ConfigClockSource( tc , TC_CLKSEL_OFF_gc );
	/* Clear overflow flag. */
	TC_ClearOverflowFlag( tc );
}

void stimulate_seat(PORT_t * port, TC0_t * tc)
{
	/* Clear CNT register*/
	tc->CNT = 0x0000;
	PORT_SetPins(port, PIN0_bm);
	TC0_ConfigClockSource( tc , TC_CLKSEL_DIV1_gc );
}