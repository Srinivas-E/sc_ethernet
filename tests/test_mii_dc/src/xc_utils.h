#ifndef __XC_UTILS_H__
#define __XC_UTILS_H__

#include <xccompat.h>
#include "random.h"

#ifdef __XC__
#define CHANEND_PARAM(param, name) param name
#else
#define CHANEND_PARAM(param, name) unsigned name
#endif

void wait(unsigned delay);
unsigned get_delay(REFERENCE_PARAM(random_generator_t, r), unsigned min, unsigned max);
void send_ether_frame(CHANEND_PARAM(chanend, c_tx), unsigned int txbuf[], unsigned int nbytes);
void generate_seq_num(unsigned char tx_buf[]);

uintptr_t get_buffer(CHANEND_PARAM(chanend, c_chl));
void put_buffer(CHANEND_PARAM(chanend, c_chl), uintptr_t dptr);
void put_buffer_int(CHANEND_PARAM(chanend, c_chl), unsigned val);

#endif // __XC_UTILS_H__
