// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#define ETHERNET_DEFAULT_IMPLEMENTATION full
#define MAX_ETHERNET_PACKET_SIZE (1518)
//#define NUM_MII_RX_BUF 6
//#define NUM_MII_TX_BUF 3
//#define ETHERNET_RX_HP_QUEUE 1
//#define ETHERNET_TX_HP_QUEUE 1
#define MAX_ETHERNET_CLIENTS   (4)

#define NUM_ETHERNET_PORTS 2
#define NUM_ETHERNET_MASTER_PORTS 2

#if 0
#define ETHERNET_MAX_TX_HP_PACKET_SIZE (300)
#define ETHERNET_MAX_TX_LP_PACKET_SIZE (1518)

#define MII_RX_BUFSIZE_HIGH_PRIORITY (1100 + (3*(ETHERNET_MAX_TX_HP_PACKET_SIZE)))
#define MII_RX_BUFSIZE_LOW_PRIORITY (1518*3)

#define MII_TX_BUFSIZE_HIGH_PRIORITY (1100 + (3*(ETHERNET_MAX_TX_HP_PACKET_SIZE)))
#define MII_TX_BUFSIZE_LOW_PRIORITY (2000)

#define ETHERNET_RX_ENABLE_TIMER_OFFSET_REQ 1
#endif
