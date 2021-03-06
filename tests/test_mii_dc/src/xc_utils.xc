#include <xs1.h>
#include <stdint.h>
#include "xc_utils.h"
#include "ethernet.h"

unsigned get_delay(random_generator_t &r, unsigned min, unsigned max)
{
    int delay = random_get_random_number(r);
    
    if (max != min)
        delay = delay % (max - min);
    else if (min != 0)
        delay = delay % min;
    else
        delay = 0;

    return delay;
}

void wait(unsigned delay)
{
    timer t;
    int time;

    t :> time;
    t when timerafter(time + delay) :> void;
}

uintptr_t get_buffer(CHANEND_PARAM(chanend, c_chl))
{
	uintptr_t dptr;
	c_chl :> dptr;
	return dptr;
}

void put_buffer(CHANEND_PARAM(chanend, c_chl), uintptr_t dptr)
{
	c_chl <: dptr;
}

void put_buffer_int(CHANEND_PARAM(chanend, c_chl), unsigned val)
{
	c_chl <: val;
}
