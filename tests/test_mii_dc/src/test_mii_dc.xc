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
#include "packet_generator.h"
#include "xscope.h"
#include "buffers_conf.h"
#include "buffer_manager.h"
#include "packet_transmitter.h"
#include "packet_controller.h"


void xscope_user_init(void) {
  xscope_register(0);
  xscope_config_io(XSCOPE_IO_BASIC);
}

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

void eth_client(chanend tx, chanend rx)
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
        //printf("Received Packet of length %d and src port: %d \n", nbytes, src_port);
        mac_tx(tx, rxbuf, nbytes, ETH_BROADCAST);
      break;
    }
  }
}

// The maximum read size is 256 bytes
#define MAX_BYTES_READ 256
#define MAX_WORDS_READ (MAX_BYTES_READ / 4)

void xscope_listener(chanend c_host_data)
{
  unsigned int buffer[MAX_WORDS_READ];

  for (int i = 0; i < MAX_WORDS_READ; i++)
    buffer[i] = 0;

  xscope_connect_data_from_host(c_host_data);

  while (1) {
    int bytes_read = 0;

    select {
      case xscope_data_from_host(c_host_data, (unsigned char *)buffer, bytes_read):
        if (bytes_read) {
          handle_host_data((unsigned char *)buffer, bytes_read);

          // Clear buffer after use
          for (int i = 0; i < (bytes_read + 3)/4; i++)
            buffer[i] = 0;
        }
        break;
    }
  }
}

int main()
{
  chan c_rx[1], c_tx[2];
  chan c_prod[NUM_BUF_PRODUCERS], c_con;
  chan c_host_data;

  par
  {
    xscope_host_data(c_host_data);

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
    on tile[0] : eth_client(c_tx[0], c_rx[0]);
    on tile[0] : random_traffic_generator(c_prod[0]);
    on tile[0] : buffer_manager(c_prod, NUM_BUF_PRODUCERS, c_con);
    on tile[0] : packet_transmitter(c_tx[1], c_con);
    on tile[0] : xscope_listener(c_host_data);
  }

  return 0;
}
