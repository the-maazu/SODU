/*
 * GPS_aaplicato.c
 *
 * Created: 16/02/2019 17:09:26
 *  Author: Hassan
 */ 

#include <GPS_application.h>

uint8_t tempRX_Head = usart_data.buffer.RX_Head;
uint8_t tempRX_Tail = usart_data.buffer.RX_Tail;

/* Check availability of GPGGA message*/
if(usart_data.buffer.RX[tempRX_Tail] == '$')
{
	if (tempRX_Head - tempRX_Tail >= 6 )
	{
		char string[7];
		memcpy(string, (char*) &usart_data.buffer.RX, 6);
		gfx_mono_draw_string( string , 20, 8, &sysfont);
		
		if ( memcmp("$GPGGA", string , 6) ==  0)
		{
			if(usart_data.buffer.RX[tempRX_Head] == '$')
			{
				gpgga_complete = true;
				gpgga_size = tempRX_Head - tempRX_Tail - 1;
			}
		}
		else USART_RXBuffer_GetByte(&usart_data);
		return;
	}
	
}
USART_RXBuffer_GetByte(&usart_data);