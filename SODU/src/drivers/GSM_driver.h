/*
 * IncFile1.h
 *
 * Created: 22/02/2019 17:46:39
 *  Author: Hassan
 */ 


#ifndef GSM_DRIVER_H_
#define GSM_DRIVER_H_

#include <asf.h>
#include "usart_driver.h"

void gsm_init(PORT_t * GSM_port, USART_t * GSM_uart);

void buffer_gsm_data(void);

void gsm_data_reg_empty(void);

#endif /* GSM_DRIVER_H_ */