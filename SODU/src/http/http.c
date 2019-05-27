/*
 * http_application.c
 *
 * Created: 15/04/2019 19:33:49
 *  Author: Hassan
 */ 

#include "http.h"
#include "../drivers/GSM_driver.h"
#include <string.h>

PROGMEM_DECLARE(char const, echo_mode_off[]) = "ATE0\r";

PROGMEM_DECLARE(char const, config_bearer_contype[]) = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r";
PROGMEM_DECLARE(char const, config_bearer_apn[]) = "AT+SAPBR=3,1,\"APN\",\"internet\"\r";
PROGMEM_DECLARE(char const, config_bearng_open_context[]) = "AT+SAPBR=1,1\r";
PROGMEM_DECLARE(char const, config_bearng_close_context[]) = "AT+SAPBR=0,1\r";

PROGMEM_DECLARE(char const, http_init_at[]) =     "AT+HTTPINIT\r";
PROGMEM_DECLARE(char const, http_term[]) =     "AT+HTTPTERM\r";
//PROGMEM_DECLARE(char const, http_enable_ssl[]) =   "AT+HTTPSSL=1\r";
PROGMEM_DECLARE(char const, http_para_cid[]) =     "AT+HTTPPARA=\"CID\",1\r";
//PROGMEM_DECLARE(char const, http_para_url[]) =     "AT+HTTPPARA=\"URL\",\"http://us-central1-tracktro.cloudfunctions.net/soduHttp\"\r";
PROGMEM_DECLARE(char const, http_para_url[]) =     "AT+HTTPPARA=\"URL\",\"http://tracktro.appspot.com/\"\r";
//PROGMEM_DECLARE(char const, http_para_content[]) = "AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"\r";

/* data 105 bytes long, 10000ms*/
PROGMEM_DECLARE(char const, http_data[]) = "AT+HTTPDATA=105,10000\r";
PROGMEM_DECLARE(char const, http_post_action[]) = "AT+HTTPACTION=1\r";


uint8_t *  read_pgm_string(const char  * string, size_t size);
uint8_t *  read_pgm_string(const char * string, size_t size)
{
	uint8_t * nstring = malloc(size);
	
	for (uint8_t i = 0 ; i < size; i ++)
	{
		nstring[i] = pgm_read_byte(string+i); 
	}
	
	return nstring;
}


uint8_t * command_for_response(uint8_t * string, size_t size);
uint8_t * command_for_response(uint8_t * string, size_t size)
{	
	static uint8_t response[19];
	uint32_t i = 0;
	
	command_gsm_module(string, size);
	
	i = 550000;
	while(!gsm_response_available())
	{
		i--;
		if(i == 0)
		break;
	}
	if(i == 0)
	{
		return (uint8_t *)"Timeout";
	}
	else
	{
		memcpy(response, (char *) get_gsm_response(), 17);		
		return response;
	}
}

bool config_bearer(void);
bool config_bearer(void)
{
	uint8_t size = 0;
	uint8_t * string;
	
	size = strlen_P(config_bearng_close_context);
	string = read_pgm_string(config_bearng_close_context, size);
	command_for_response(string, size);
	
	size = strlen_P(config_bearer_contype);
	string = read_pgm_string(config_bearer_contype, size);
	while ( memcmp("OK", command_for_response(string, size), 2));
	
	size = strlen_P(config_bearer_apn);
	string = read_pgm_string(config_bearer_apn, size);
	while ( memcmp("OK", command_for_response(string, size), 2));
	
	size = strlen_P(config_bearng_open_context);
	string = read_pgm_string(config_bearng_open_context, size);
	return memcmp("OK", command_for_response(string, size), 2);
	
}

bool config_http(void);
bool config_http(void)
{
	uint8_t size = 0;
	uint8_t * string;
	
	size = strlen_P(http_term);
	string = read_pgm_string(http_term, size);
	command_for_response(string, size);
	
	size = strlen_P(http_init_at);
	string = read_pgm_string(http_init_at, size);
	if ( memcmp("OK",command_for_response(string, size),2) != 0)
	{
		return 1;
	}
	
	size = strlen_P(http_para_cid);
	string = read_pgm_string(http_para_cid, size);
	while ( memcmp("OK", command_for_response(string, size), 2));
	
	size = strlen_P(http_para_url);
	string = read_pgm_string(http_para_url, size);
	while ( memcmp("OK", command_for_response(string, size), 2));
	
	//size = strlen_P(http_para_content);
	//string = read_pgm_string(http_para_content, size);
	//while ( memcmp("OK", command_for_response(string, size), 2));	
	return 0;
}


void http_init()
{
	uint8_t size = 0;
	uint8_t * string;
	
	size = strlen_P(echo_mode_off);
	string = read_pgm_string(echo_mode_off, size);
	while ( memcmp("OK", command_for_response(string, size), 2));
	
	while(config_bearer());
	
	while(config_http());
	
}

bool post_data(uint8_t * data, size_t size)
{	
	char * size_string = NULL;
	itoa(size, size_string, 10);
	
	char * command = malloc(19 + strlen(size_string));
	uint8_t size_command = 0;
	
	strcpy(command , "AT+HTTPDATA=");
	strcat( command , size_string);
	strcat(command, ",1000\r");

	size_command = strlen(command);
			
	if ( memcmp("DOWNLOAD", command_for_response((uint8_t *) command, size_command), 1))
	return false;
	
	if ( memcmp("OK", command_for_response( data, size), 2))
	return false;
	
	size = strlen_P(http_post_action); 
	if ( memcmp("OK", command_for_response((uint8_t *) read_pgm_string(http_post_action, size), size), 2))
	return false;
	
	uint16_t i = 0xFFFF;
	while( memcmp("+HTTPACTION:1,200,0", command_for_response((uint8_t *)"", 0), 11) & i)
	{
		
		i--;
	}
	
	if(i == 0){
		return false;
	}
	else
	return true;
}