/*
 * CFile1.c
 *
 * Created: 22/02/2019 17:48:57
 *  Author: Hassan
 */ 

#include "GSM_driver.h"
#include "port_driver.h"
#include "string.h"
#include "gfx_mono.h"

USART_data_t gsm_usart_data;
volatile uint8_t gsm_response_size = 0;
volatile uint8_t gsm_response_ready = false;

void gsm_init(PORT_t * GSM_port, USART_t * GSM_uart)
{
	/* Remap USART of PORTC to pin 4-7 */
	PORTC.REMAP |= 0x10;																			
	
  	/* PC7 (TXD0) as output. */
	PORT_SetPinAsOutput( GSM_port, PIN7_bp );
	/* PC6 (RXD0) as input. */
	PORT_SetPinAsInput( GSM_port, PIN6_bp );

	/* Use USARTC0 and initialize buffers. */
	USART_InterruptDriver_Initialize(&gsm_usart_data, GSM_uart, USART_DREINTLVL_MED_gc);

	/* USARTC0, 8 Data bits, No Parity, 1 Stop bit. */
	USART_Format_Set(gsm_usart_data.usart, USART_CHSIZE_8BIT_gc,
                     USART_PMODE_DISABLED_gc, false);

	/* Enable RXC interrupt. */
	USART_RxdInterruptLevel_Set(gsm_usart_data.usart, USART_RXCINTLVL_MED_gc);

	/* Set Baudrate to 19200 bps:
	 * bsel = (1./2.^bscale)((2*(10^6))/(16 * 19200)-1)
	 *      = 11 if bscale = -1
	 */
	USART_Baudrate_Set(GSM_uart,  12, 0);

	/* Enable RX and TX. */
	USART_Rx_Enable(gsm_usart_data.usart);
	USART_Tx_Enable(gsm_usart_data.usart);
}

void buffer_gsm_data()
{
	USART_RXComplete(&gsm_usart_data);
	
	uint8_t tempRX_Head = gsm_usart_data.buffer.RX_Head;
	uint8_t tempRX_Tail = gsm_usart_data.buffer.RX_Tail;
	uint8_t buffered_size = GetRXBufferIndex(tempRX_Head - tempRX_Tail);

	if( buffered_size > 2)
	{	
		if( gsm_usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 0)] == '\r'
		&& gsm_usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 1)] == '\n'
		&& gsm_usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 2)] != '\n'
		&& gsm_usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 2)] != '\r'
		&& gsm_usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 2)] != 'A' ) 
		{
			if (gsm_usart_data.buffer.RX[GetRXBufferIndex(tempRX_Head - 2)] == '\r'
			&& gsm_usart_data.buffer.RX[GetRXBufferIndex(tempRX_Head - 1)] == '\n')
			{
				gsm_response_ready = true;
				gsm_response_size = buffered_size;
			}
		}
		else USART_RXBuffer_GetByte(&gsm_usart_data);
	}
}

void gsm_data_reg_empty(void)
{
	USART_DataRegEmpty(&gsm_usart_data);
}

void command_gsm_module(uint8_t * string, size_t size)
{	
	uint8_t i = 0;
	while(i < size)
	{
		bool byteToBuffer;
		byteToBuffer = USART_TXBuffer_PutByte(&gsm_usart_data, string[i]);
		if(byteToBuffer){
			i++;
		}
	}
}

bool gsm_response_available()
{
	return gsm_response_ready;
}

uint8_t * get_gsm_response()
{	
	uint8_t * gsm_response = malloc(gsm_response_size - 4);
	
	/* Copy response message to gsm_data*/
	uint8_t byte = 0;
	for(uint8_t i = 0 ; i < gsm_response_size; i++)
	{
		byte = USART_RXBuffer_GetByte(&gsm_usart_data);
		
		if ( i > 1 && i <gsm_response_size - 2)
		gsm_response[i-2] = byte;
		
	}
	
	/* start afresh */
	gsm_response_ready = false;
	gsm_response_size = 0;
	
	return gsm_response; 
}