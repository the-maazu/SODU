/*
 * GPS_driver.h
 *
 * Created: 15/02/2019 15:54:04
 *  Author: Hassan
 */ 


#ifndef GPS_DRIVER_H_
#define GPS_DRIVER_H_

#include <asf.h>

void gps_init(PORT_t * GPS_port, USART_t * GPS_uart);

void buffer_gps_data(void);

bool gps_data_available(void);

char * get_gps_data(void);

#endif /* GPS_DRIVER_H_ */