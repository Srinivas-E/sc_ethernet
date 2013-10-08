#ifndef __C_UTILS_H__
#define __C_UTILS_H__

#include <xccompat.h>

void send_ether_frame(CHANEND_PARAM(chanend, c_tx), uintptr_t dptr, unsigned int nbytes);

#endif // __C_UTILS_H__
