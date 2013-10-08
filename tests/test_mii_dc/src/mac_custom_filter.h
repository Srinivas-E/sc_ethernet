// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#define MII_FILTER_FORWARD_TO_OTHER_PORTS (0x80000000)

#pragma unsafe arrays
inline int mac_custom_filter(unsigned int buf[], unsigned int mac[2], int &user_data)
{
  int result = 0;
  unsigned short etype = (unsigned short) buf[3];
  int qhdr = (etype == 0x0081);

  if (qhdr) {
    // has a 802.1q tag - read etype from next word
    etype = (unsigned short) buf[4];
  }

  switch (etype) {
    default:
      if ((buf[0] & 0x1) || // Broadcast
          (buf[0] != mac[0] || buf[1] != mac[1])) // Not unicast
      {
        result |= MII_FILTER_FORWARD_TO_OTHER_PORTS;
      }
      break;
  }

  return result;
}
