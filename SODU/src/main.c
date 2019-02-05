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

void setup_port( PORT_t * port );

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	
	/* Setup all 4 ports for 4 seats*/
	setup_port(&PORTA);
	setup_port(&PORTB);
	setup_port(&PORTC);
	setup_port(&PORTD);
	
	/* Setup event system. */
	EVSYS_SetEventSource(0, EVSYS_CHMUX_PORTA_PIN0_gc);
	EVSYS_SetEventSource(1, EVSYS_CHMUX_PORTA_PIN2_gc);
	
	EVSYS_SetEventSource(2, EVSYS_CHMUX_PORTB_PIN0_gc);
	EVSYS_SetEventSource(3, EVSYS_CHMUX_PORTB_PIN2_gc);
	
	EVSYS_SetEventSource(4, EVSYS_CHMUX_PORTC_PIN0_gc);
	EVSYS_SetEventSource(5, EVSYS_CHMUX_PORTC_PIN2_gc);
	
	EVSYS_SetEventSource(6, EVSYS_CHMUX_PORTD_PIN0_gc);
	EVSYS_SetEventSource(7, EVSYS_CHMUX_PORTD_PIN2_gc);
	
	/* Enable medium level interrupts in the PMIC. */
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;

	/* Enable the global interrupt flag. */
	sei();
	
	while(true){

	}
}

void setup_port( PORT_t * port)
{
	/* Configure pin 0 as output, triggered on rising edges. */
	PORT_ConfigurePins( port,
	0x01,
	false,
	false,
	PORT_OPC_TOTEM_gc,
	PORT_ISC_RISING_gc);
	
	PORT_SetPinAsOutput( port, 0x01 );
	
	/* Configure pin 2 as input, triggered on falling edges. */
	PORT_ConfigurePins( port,
	0x04, // pin 2 supports full asynchronous sense
	false,
	false,
	PORT_OPC_TOTEM_gc,
	PORT_ISC_FALLING_gc);

	PORT_SetPinsAsInput( port, 0x04 );

	/* Configure Interrupt0 to have medium interrupt level, triggered by pin 0. */
	PORT_ConfigureInterrupt0( port, PORT_INT0LVL_MED_gc, 0x01 );
	
	/* Configure Interrupt1 to have medium interrupt level, triggered by pin 2. */
	PORT_ConfigureInterrupt1( port, PORT_INT1LVL_MED_gc, 0x04 );
}