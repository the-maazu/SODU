/*
 * seat_driver.h
 *
 * Created: 06/02/2019 14:01:10
 *  Author: Hassan
 */ 


#ifndef SEAT_DRIVER_H_
#define SEAT_DRIVER_H_

#include <asf.h>

void seat_init( PORT_t * port, TC0_t * tc);

void check_seat(uint8_t seat_number, PORT_t * port, TC0_t * tc);

void stimulate_seat(PORT_t * port, TC0_t * tc);

uint8_t get_seat_mask(void);

#endif /* SEAT_DRIVER_H_ */