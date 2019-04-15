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
#include "drivers/GSM_driver.h"
#include <string.h>

#define GSM_PORT &PORTC
#define GSM_USART &USARTC0

#define GSM_DATA_REG_EMPTY_VECT USARTC0_DRE_vect
#define GSM_RXC_vect USARTC0_RXC_vect

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	
	gfx_mono_init();
	gpio_set_pin_high(NHD_C12832A1Z_BACKLIGHT);
	
	gsm_init(GSM_PORT, GSM_USART);
	
	/* Enable medium and high level interrupts in the PMIC. */
	PMIC.CTRL |= (PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm | PMIC_LOLVLEN_bm) ;

	/* Enable the global interrupt flag. */
	sei();
	
	
	/* test code */
	char * string = "ATE0\r";
	char response[4];
	uint32_t i = 0;
	
	while(true)
	{
		command_gsm_module((uint8_t *) string, strlen(string));
		
		i = 250000;
		while(!gsm_response_available())
		{
			i--;
			if(i == 0)
			break;
		}
		if(i == 0)
		gfx_mono_draw_string("Timeout"  , 20,8, &sysfont);
		else
		{
			memcpy(response, (char *) get_gsm_response(), 4);
			//strncpy(response,(char *) get_gsm_response(), 2);
			gfx_mono_draw_string(response, 20,8, &sysfont);
		}
	}
}


ISR(GSM_RXC_vect)
{
	buffer_gsm_data();
}

ISR(GSM_DATA_REG_EMPTY_VECT)
{
	gsm_data_reg_empty();
}
