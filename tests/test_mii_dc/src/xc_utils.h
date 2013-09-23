#ifndef __XC_UTILS_H__
#define __XC_UTILS_H__

#include <xccompat.h>
#include "random.h"

#ifdef __XC__
#define CHANEND_PARAM(param, name) param name
#else
#define CHANEND_PARAM(param, name) unsigned name
#endif

void wait(REFERENCE_PARAM(random_generator_t, r), unsigned min, unsigned max);
void send_ether_frame(CHANEND_PARAM(chanend, c_tx), unsigned int txbuf[], unsigned int nbytes);
void generate_seq_num(unsigned char tx_buf[]);

#endif // __XC_UTILS_H__
