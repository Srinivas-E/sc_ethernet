#include <xs1.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xccompat.h>

#include "common.h"
#include "c_utils.h"
#include "ethernet.h"
#include "debug_print.h"
#include "packet_controller.h"

void send_ether_frame(CHANEND_PARAM(chanend, c_tx), uintptr_t dptr, unsigned int nbytes)
{
  mac_tx(c_tx, (unsigned int *)dptr, nbytes, ETH_BROADCAST);
}

char get_next_char(const unsigned char **buffer)
{
  const unsigned char *ptr = *buffer;
  while (*ptr && isspace(*ptr))
    ptr++;

  *buffer = ptr + 1;
  return *ptr;
}

int convert_atoi_substr(const unsigned char **buffer)
{
  const unsigned char *ptr = *buffer;
  unsigned int value = 0;
  while (*ptr && isspace(*ptr))
    ptr++;

  if (*ptr == '\0')
    return 0;

  value = atoi((char*)ptr);

  while (*ptr && !isspace(*ptr))
    ptr++;

  *buffer = ptr;
  return value;
}

