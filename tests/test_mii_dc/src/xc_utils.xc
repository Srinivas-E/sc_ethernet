#include <xs1.h>
#include "xc_utils.h"
#include "ethernet.h"

void wait(random_generator_t &r, unsigned min, unsigned max)
{
    timer t;
    int time;
    int delay = random_get_random_number(r);
    
    if (max != min)
        delay = delay % (max - min);
    else if (min != 0)
        delay = delay % min;
    else
        delay = 0;

    if (delay == 0)
        return;

    t :> time;
    t when timerafter(time + delay) :> void;
}

void send_ether_frame(CHANEND_PARAM(chanend, c_tx), unsigned int txbuf[], unsigned int nbytes)
{
    mac_tx(c_tx, txbuf, nbytes, ETH_BROADCAST);
}

void generate_seq_num(unsigned char tx_buf[])
{
  static unsigned seq_num = 0;
  int j=3;
  for (int i=0; i<4;i++) {
    tx_buf[j] = (seq_num >> i*8) & 0xFF ;
    j--;
  }
  seq_num++;
}
