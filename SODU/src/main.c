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
#include "drivers/seat_driver.h"
#include "drivers/port_driver.h"
#include "string.h"

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

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	
	/* Setup all 4 seats*/
	seat_init(SEAT1_PORT, SEAT1_TIMER);
	seat_init(SEAT2_PORT, SEAT2_TIMER);
	seat_init(SEAT3_PORT, SEAT3_TIMER);
	seat_init(SEAT4_PORT, SEAT4_TIMER);
			
	/* Enable medium and high level interrupts in the PMIC. */
	PMIC.CTRL |= (PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm) ;

	/* Enable the global interrupt flag. */
	sei();
	
	stimulate_seat(SEAT1_PORT, SEAT1_TIMER);
	stimulate_seat(SEAT2_PORT, SEAT2_TIMER);
	stimulate_seat(SEAT3_PORT, SEAT3_TIMER);
	stimulate_seat(SEAT4_PORT, SEAT4_TIMER);
	
	while(true)
	{
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