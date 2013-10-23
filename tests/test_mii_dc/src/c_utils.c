#include <xs1.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "c_utils.h"
#include "ethernet.h"
#include "debug_print.h"
#include "traffic_generator.h"

void send_ether_frame(CHANEND_PARAM(chanend, c_tx), uintptr_t dptr, unsigned int nbytes)
{
	mac_tx(c_tx, (unsigned int *)dptr, nbytes, ETH_BROADCAST);
}

/* This function converts an ascii  string to integer and returns its string length */
int convert_atoi_substr(char buffer[], unsigned len[], unsigned bytes_len)
{
  char *ptr = buffer;
  int i=0, j=0;
  while (*ptr && isspace(*ptr)) {
	ptr++;
	i++;
  }

  ptr = buffer;
  while (*ptr && !isspace(*ptr)) {
	ptr++;
	j++;
  }

  *len = j;
  buffer[bytes_len] = '\0';
  return (atoi(&buffer[i]));
}

#if 0
unsigned get_line_rate(char buffer[])
{
  unsigned line_rate = 0, len = 0;
  line_rate = convert_atoi_substr(&buffer[2], &len);
  if ((line_rate <= 0) || (line_rate > 100)) {
    debug_printf("Invalid line rate!!! should not occur\n");
    return 0;
  }
  return line_rate;
}

int validate_pkt_setting(char buffer[])
{
  int weight = 0, pkt_size = 0;
  unsigned len = 0;
  weight = convert_atoi_substr(&buffer[2], &len);
  pkt_size = convert_atoi_substr(&buffer[2+len], &len);

  if ((weight <= 0) || (weight > 100)) {
    debug_printf("Invalid weight; specify a value between 1 and 99 \n");
    return 0;
  }

  if ((pkt_size <= 0) || (pkt_size > 1500)) {
    debug_printf("Invalid pkt_size; specify a value between 1 and 1500 \n");
    return 0;
  }

  return 1;
}
#endif

