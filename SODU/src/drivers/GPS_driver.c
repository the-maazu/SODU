/*
 * CFile1.c
 *
 * Created: 15/02/2019 17:22:53
 *  Author: Hassan
 */ 
#include "GPS_driver.h"
#include "usart_driver.h"
#include "port_driver.h"
#include <string.h>

USART_data_t usart_data;

volatile bool gps_data_ready = false;
volatile uint8_t gps_data_size = 0;

void gps_init(PORT_t * GPS_port, USART_t * GPS_uart)
{
  	/* PE3 (TXD0) as output. */
	PORT_SetPinAsOutput( GPS_port, PIN3_bp );
	/* PE2 (RXD0) as input. */
	PORT_SetPinAsInput( GPS_port, PIN2_bp );

	/* Use USARTE0 and initialize buffers. */
	USART_InterruptDriver_Initialize(&usart_data, GPS_uart, USART_DREINTLVL_MED_gc);

	/* USARTE0, 8 Data bits, No Parity, 1 Stop bit. */
	USART_Format_Set(usart_data.usart, USART_CHSIZE_8BIT_gc,
                     USART_PMODE_DISABLED_gc, false);

	/* Enable RXC interrupt. */
	USART_RxdInterruptLevel_Set(usart_data.usart, USART_RXCINTLVL_MED_gc);

	/* Set Baudrate to 9600 bps:
	 * Use the default I/O clock frequency that is 2 MHz.
	 * Do not use the baudrate scale factor
	 *
	 * Baudrate select = (1/(16*(((I/O clock frequency)/Baudrate)-1)
	 *                 = 12
	 */
	USART_Baudrate_Set(GPS_uart, 12 , 0);

	/* Enable both RX. */
	USART_Rx_Enable(usart_data.usart);
}

void buffer_gps_data()
{
	if(gps_data_ready)
	return;
	
	USART_RXComplete(&usart_data);

	uint8_t tempRX_Head = usart_data.buffer.RX_Head;
	uint8_t tempRX_Tail = usart_data.buffer.RX_Tail;
	uint8_t buffer_data_size = GetRXBufferIndex(tempRX_Head - tempRX_Tail);
	
	if( buffer_data_size >= 6)
	{		
		if( usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 0)] == '$' 
			&& usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 1)] == 'G' 
			&& usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 2)] == 'P'
			&& usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 3)] == 'G'
			&& usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 4)] == 'G'
			&& usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 5)] == 'A')
		{
			uint8_t last_byte_index = GetRXBufferIndex(tempRX_Head - 1);
			
			if(usart_data.buffer.RX[last_byte_index] == '\n')
			{
				gps_data_ready = true;
				gps_data_size = GetRXBufferIndex(last_byte_index - tempRX_Tail);
			}
		}
		else USART_RXBuffer_GetByte(&usart_data);
	}
}

bool gps_data_available()
{
	return gps_data_ready;
}

char * get_gps_data(void)
{
	char * gps_data = malloc(gps_data_size + 1);
	
	/* Copy GPGGA message to gps_data*/
	for(uint8_t i = 0; i < gps_data_size; i++)
	{
		gps_data[i] = USART_RXBuffer_GetByte(&usart_data);
	}
	gps_data[gps_data_size] = '\0';
	
	/*start afresh */
	gps_data_ready = false;
	gps_data_size = 0;
	
	return gps_data;
}