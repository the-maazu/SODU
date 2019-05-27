/*
 * http.h
 *
 * Created: 15/04/2019 19:34:38
 *  Author: Hassan
 */ 


#ifndef HTTP_H_
#define HTTP_H_

#include <asf.h>

void http_init(void);

bool post_data(uint8_t * data, size_t size);

#endif /* HTTP_H_ */