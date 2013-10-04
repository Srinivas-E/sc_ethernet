#include <stddef.h>
#include <stdio.h>
#include "stdint.h"
#include "print.h"
#include "xc_utils.h"

void packet_transmitter(CHANEND_PARAM(chanend, c_tx), CHANEND_PARAM(chanend, c_con))
{
    while (1) {
      uintptr_t dptr;
      unsigned length_in_bytes;
      unsigned delay;
      unsigned pkt_len;

      slave {
    	c_con :> dptr;
    	c_con :> length_in_bytes;
      }

      asm volatile("ldw %0, %1[%2]":"=r"(delay):"r"(dptr), "r"(0):"memory");
      wait(delay);
      //TODO: to increment dptr to point to actual pkt data
      send_ether_frame(c_tx, (unsigned int *) &dptr, (length_in_bytes-1*4));

      /* Release the buffer */
      c_con <: dptr;
      printf("Tx: pkt_len: %u\n", length_in_bytes);

    }
}
