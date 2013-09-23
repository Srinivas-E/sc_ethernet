// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/*************************************************************************
 *
 * Ethernet MAC Layer Client Test Code
 * IEEE 802.3 MAC Client
 *
 *************************************************************************
 *
 * Layer 2 MII demo
 * Note: Only supports unfragmented IP packets
 *
 *************************************************************************/

#include <xs1.h>
#include <print.h>
#include <stdio.h>
#include <platform.h>
#include "otp_board_info.h"
#include "ethernet.h"
#include "ethernet_board_support.h"
#include "random_packets.h"
#include "xscope.h"

#if USE_XSCOPE
void xscope_user_init(void) {
  xscope_register(0);
  xscope_config_io(XSCOPE_IO_BASIC);
}
#endif

on ETHERNET_DEFAULT_TILE: otp_ports_t otp_ports = OTP_PORTS_INITIALIZER;

smi_interface_t smi1 = ETHERNET_DEFAULT_SMI_INIT;

// Circle slot
mii_interface_t mii1 = ETHERNET_DEFAULT_MII_INIT;

// Square slot
on tile[1]: mii_interface_t mii2 = {
  XS1_CLKBLK_3,
  XS1_CLKBLK_4,
  XS1_PORT_1B,
  XS1_PORT_4D,
  XS1_PORT_4A,
  XS1_PORT_1C,
  XS1_PORT_1G,
  XS1_PORT_1F,
  XS1_PORT_4B
};

void app_client(chanend tx, chanend rx)
{
  unsigned int rxbuf[1600/4];
  unsigned int src_port;
  unsigned int nbytes;

  //::setup-filter
  mac_set_custom_filter(rx, 0x1);
  //::

  //::mainloop
  while (1)
  {
    select {
      case mac_rx(rx, (rxbuf,char[]), nbytes, src_port):
        printf("Received Packet of length %d and src port: %d \n", nbytes, src_port);
        mac_tx(tx, rxbuf, nbytes, ETH_BROADCAST);
      break;
	}
  }
}


int main()
{
  chan c_rx[1], c_tx[2];

  par
  {
    on ETHERNET_DEFAULT_TILE:
    {
      char mac_address[6];
      otp_board_info_get_mac(otp_ports, 0, mac_address);
      smi_init(smi1);
      eth_phy_config(1, smi1);
      ethernet_server_full_two_port(mii1,
                                    mii2,
                                    smi1,
                                    null,
                                    mac_address,
                                    c_rx, 1,
                                    c_tx, 2);
    }
    on tile[0] : app_client(c_tx[0], c_rx[0]);
    on tile[0] : random_traffic_generator(c_tx[1]);
  }

  return 0;
}
