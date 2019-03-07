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
volatile bool gsm_response = false;

PROGMEM_DECLARE(char const, config_bearer_contype[]) = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r";
PROGMEM_DECLARE(char const, config_bearer_apn[]) = "AT+SAPBR=3,1,\"APN\",\"internet\"\r";
PROGMEM_DECLARE(char const, config_bearng_open_context[]) = "AT+SAPBR=1,1\r";

PROGMEM_DECLARE(char const, http_init_at[]) =	   "AT+HTTPINIT\r";
PROGMEM_DECLARE(char const, http_enable_ssl[]) =   "AT+HTTPSSL=1\r";
PROGMEM_DECLARE(char const, http_para_cid[]) =     "AT+HTTPPARA=\"CID\",1\r";
PROGMEM_DECLARE(char const, http_para_url[]) =     "AT+HTTPPARA=\"URL\",\"https://tracktro.appspot.com\"\r";
PROGMEM_DECLARE(char const, http_para_content[]) = "AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"\r";

/* data 105 bytes long, 10000ms*/
PROGMEM_DECLARE(char const, http_data[]) = "AT+HTTPDATA=105,10000\r";
PROGMEM_DECLARE(char const, http_post_action[]) = "AT+HTTPACTION=1\r";

bool response_OK(void);
bool response_OK(void)
{
	while(!gsm_response);
	
	uint8_t tempRX_Tail = gsm_usart_data.buffer.RX_Tail;
	uint8_t tempRX_Head = gsm_usart_data.buffer.RX_Head;
	uint8_t data_size = GetRXBufferIndex(tempRX_Head - tempRX_Tail);
	
	/* Clear buffer and set response to false*/
	for( uint8_t i = 0 ; i < data_size; i++)
	{
		USART_RXBuffer_GetByte(&gsm_usart_data);
	}
	gsm_response = false;
	
	/* Handle response */
	if(gsm_usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 2) ] == 'O'
		&& gsm_usart_data.buffer.RX[GetRXBufferIndex(tempRX_Tail + 3) ] == 'K')
	{
		gfx_mono_draw_string( "Ok", 20, 8, &sysfont);
		return true;
	}
	
	return false;
}

void command_gsm_module(PROGMEM_DECLARE(char const, string[]));
void command_gsm_module(PROGMEM_DECLARE(char const, string[]))
{
	uint8_t i = 0;
	
	while(i < strlen_P(string))
	{		
		bool byteToBuffer;
		byteToBuffer = USART_TXBuffer_PutByte(&gsm_usart_data, pgm_read_byte(string+i));
		if(byteToBuffer){
			delay_us(1000000);
			gpio_toggle_pin(LED0);
			i++;
		}
	}
}

bool http_init(void);
bool http_init(void)
{	
	command_gsm_module(config_bearer_contype);
	if(!response_OK())
	return false;
	
	command_gsm_module(config_bearer_apn);
	if(!response_OK())
	return false;
	
	command_gsm_module(config_bearng_open_context);
	if(!response_OK())
	return false;
	
	command_gsm_module(http_init_at);
	if(!response_OK())
	return false;
	
	command_gsm_module(http_enable_ssl);
	if(!response_OK())
	return false;
	
	command_gsm_module(http_para_cid);
	if(!response_OK())
	return false;
	
	command_gsm_module(http_para_url);
	if(!response_OK())
	return false;
	
	command_gsm_module(http_para_content);
	if(!response_OK())
	return false;
	
	gpio_set_pin_low(LED0);
	return true;
}

void gsm_init(PORT_t * GSM_port, USART_t * GSM_uart)
{
	/* Remap USART of PORTC to pin 4-7 */
	PORTC.REMAP |= 0x10;
	
  	/* PC7 (TXD0) as output. */
	PORT_SetPinAsOutput( GSM_port, PIN7_bp );
	/* PC6 (RXD0) as input. */
	PORT_SetPinAsInput( GSM_port, PIN6_bp );

	/* Use USARTC0 and initialize buffers. */
	USART_InterruptDriver_Initialize(&gsm_usart_data, GSM_uart, USART_DREINTLVL_HI_gc);

	/* USARTC0, 8 Data bits, No Parity, 1 Stop bit. */
	USART_Format_Set(gsm_usart_data.usart, USART_CHSIZE_8BIT_gc,
                     USART_PMODE_DISABLED_gc, false);

	/* Enable RXC interrupt. */
	USART_RxdInterruptLevel_Set(gsm_usart_data.usart, USART_RXCINTLVL_HI_gc);

	/* Set Baudrate to 19200 bps:
	 * bsel = (1./2.^bscale)((2*(10^6))/(16 * 19200)-1)
	 *      = 11 if bscale = -1
	 */
	USART_Baudrate_Set(GSM_uart,  11, -1);

	/* Enable RX and TX. */
	USART_Rx_Enable(gsm_usart_data.usart);
	USART_Tx_Enable(gsm_usart_data.usart);
	
	while(!http_init());
}

void buffer_gsm_data()
{
	USART_RXComplete(&gsm_usart_data);

	uint8_t tempRX_Head = gsm_usart_data.buffer.RX_Head;
	uint8_t tempRX_Tail = gsm_usart_data.buffer.RX_Tail;
	uint8_t response_size = GetRXBufferIndex(tempRX_Head - tempRX_Tail);
	
	if( response_size > 2
		&& gsm_usart_data.buffer.RX[GetRXBufferIndex(tempRX_Head - 1)] == '\n')
	{
		gsm_response = true;
	}
}

void gsm_data_reg_empty(void)
{
	USART_DataRegEmpty(&gsm_usart_data);
}
