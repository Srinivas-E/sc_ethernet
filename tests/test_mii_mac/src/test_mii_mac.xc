// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/*************************************************************************
 *
 * Ethernet MAC Layer Client Test Code
 * IEEE 802.3 MAC Client
 *
 *
 *************************************************************************
 *
 * Layer 2 MII demo
 * Note: Only supports unfragmented IP packets
 *
 *************************************************************************/

#include <xs1.h>
#include <print.h>
#include <platform.h>
#include "otp_board_info.h"
#include "ethernet.h"
#include "ethernet_board_support.h"
#include "xscope.h"

#define USE_XSCOPE 0
#define ETHERTYPE_OFFSET 12

#if USE_XSCOPE
void xscope_user_init(void) {
  xscope_register(0);
  xscope_config_io(XSCOPE_IO_BASIC);
}
#endif

// Port Definitions

// These ports are for accessing the OTP memory
on ETHERNET_DEFAULT_TILE: otp_ports_t otp_ports = OTP_PORTS_INITIALIZER;

// Here are the port definitions required by ethernet
// The intializers are taken from the ethernet_board_support.h header for
// XMOS dev boards. If you are using a different board you will need to
// supply explicit port structure intializers for these values
smi_interface_t smi = ETHERNET_DEFAULT_SMI_INIT;
mii_interface_t mii = ETHERNET_DEFAULT_MII_INIT;
ethernet_reset_interface_t eth_rst = ETHERNET_DEFAULT_RESET_INTERFACE_INIT;

void app_client(chanend tx, chanend rx, int client_num);

#pragma unsafe arrays
void get_ethertype(unsigned char data[], unsigned char ethertype[2])
{
  int offset = ETHERTYPE_OFFSET;
  // Skip VLAN Q tags. It is possible to have double VLAN tags
  while (data[offset] == 0x81 && data[offset + 1] == 0x00) {
    offset += 4;
  }
  ethertype[0] = data[offset];
  ethertype[1] = data[offset + 1];
}

//::custom-filter
int mac_custom_filter(unsigned int data[])
{
  unsigned char ethertype[2];
  get_ethertype((data, char[]), ethertype);
  if (ethertype[0] == 0x26) {
    switch (ethertype[1]) {
      case 0xb1: return 1;
      case 0xb2: return 2;
      case 0xb3: return 4;
      case 0xb4: return 8;
    }
  }
   return 0;
}
//::

int build_response(unsigned char rxbuf[], unsigned int txbuf[], unsigned int nbytes)
{
  for (int i = 0; i < nbytes; i++)
  {
    (txbuf, unsigned char[])[i] = rxbuf[i];
  }

  return nbytes;
}


void app_client(chanend tx, chanend rx, int client_num)
{
  unsigned char own_mac_addr[6];
  unsigned int rxbuf[1600/4];
  unsigned int txbuf[1600/4];

  //::get-macaddr
  mac_get_macaddr(tx, own_mac_addr);
  if (client_num == 1) {
	  printstr("Device MAC address: ");
	  printhex(own_mac_addr[0]); printchar(':'); printhex(own_mac_addr[1]); printchar(':');
	  printhex(own_mac_addr[2]); printchar(':'); printhex(own_mac_addr[3]); printchar(':');
	  printhex(own_mac_addr[4]); printchar(':'); printhexln(own_mac_addr[5]);
  }
  //::

  //::setup-filter
  mac_set_custom_filter(rx, 0x1 << (client_num-1));
  //::

  //::mainloop
  while (1)
  {
    unsigned int src_port;
    unsigned int nbytes;
    mac_rx(rx, (rxbuf,char[]), nbytes, src_port);

    //::client_packet_check
    build_response((rxbuf,char[]), txbuf, nbytes);
    mac_tx(tx, txbuf, nbytes, ETH_BROADCAST);
    printstr("Device response sent from client ");
    printintln(client_num);
    //::
  }
}

int main()
{
  chan rx[4], tx[4];

  par
    {
      //::ethernet
      on ETHERNET_DEFAULT_TILE:
      {
        char mac_address[6];
        otp_board_info_get_mac(otp_ports, 0, mac_address);
        eth_phy_reset(eth_rst);
        smi_init(smi);
        eth_phy_config(1, smi);
        ethernet_server(mii,
                        null,
                        mac_address,
                        rx, 4,
                        tx, 4);
      }
      //::

      //::L2 application clients
      on tile[1] : app_client(tx[0], rx[0], 1);
      on tile[1] : app_client(tx[1], rx[1], 2);
      on tile[1] : app_client(tx[2], rx[2], 3);
      on tile[0] : app_client(tx[3], rx[3], 4);
      //::
    }

	return 0;
}
