#include <xs1.h>
#include <stdint.h>
#include "common.h"
#include "c_utils.h"
#include "ethernet.h"

void send_ether_frame(CHANEND_PARAM(chanend, c_tx), uintptr_t dptr, unsigned int nbytes)
{
	mac_tx(c_tx, (unsigned int *)dptr, nbytes, ETH_BROADCAST);
}
