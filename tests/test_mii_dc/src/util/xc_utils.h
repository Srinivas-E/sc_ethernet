#ifndef __XC_UTILS_H__
#define __XC_UTILS_H__

//#include "random.h"
//#include "common.h"

void wait(unsigned delay);
unsigned get_delay(REFERENCE_PARAM(random_generator_t, r), unsigned min, unsigned max);
uintptr_t get_buffer(CHANEND_PARAM(chanend, c_chl));
void put_buffer(CHANEND_PARAM(chanend, c_chl), uintptr_t dptr);
void put_buffer_int(CHANEND_PARAM(chanend, c_chl), unsigned val);

#endif // __XC_UTILS_H__
