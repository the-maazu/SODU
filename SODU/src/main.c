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

#define GPS_PORT &PORTE
#define GPS_UART &USARTE0
#define GPS_INTVECT USARTE0_RXC_vect

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	
	gps_init(GPS_PORT, GPS_UART);
		
	/* Enable medium and high level interrupts in the PMIC. */
	PMIC.CTRL |= (PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm) ;

	/* Enable the global interrupt flag. */
	sei();
	
	while(true)
	{
		while(!gps_data_available());
		gfx_mono_draw_string( get_gps_data() , 20, 8, &sysfont);
	}
}

ISR(GPS_INTVECT)
{
	buffer_gps_data();
}