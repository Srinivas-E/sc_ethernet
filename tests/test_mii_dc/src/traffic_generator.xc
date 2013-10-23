#include <platform.h>
#include <xscope.h>
#include <stdint.h>

#include "debug_print.h"
#include "traffic_gen_host_cmds.h"
#include "traffic_generator.h"
#include "common.h"
#include "c_utils.h"

/* A flag to indicate a pending host command to be serviced */
unsigned pending_service;
pkt_control_t pkt_control;

unsigned get_pending_service_status(void)
{
  return pending_service;
}

void get_control_commands(uintptr_t &pkt_control_cmds)
{
  pkt_control_cmds = (uintptr_t)&pkt_control;
}

void reset_pending_service_status(void)
{
  pending_service = 0;
/*  pkt_control.mode = 1;
  pkt_control.rate = 0;
  pkt_control.bcast.size = 0;
  pkt_control.bcast.weight = 0;
  pkt_control.mcast.size = 0;
  pkt_control.mcast.weight = 0;
  pkt_control.ucast.size = 0;
  pkt_control.ucast.weight = 0;*/
}

/**
 * \brief   A core that listens to data being sent from the host and
 *          informs the analysis engine of any changes
 */
void xscope_listener()
{
  // The maximum read size is 256 bytes
  unsigned char buffer[256] = {'\0'};

  while (1) {
    unsigned bytes_read = xscope_upload_bytes(-1, (unsigned char *)&buffer[0]);

    if ((bytes_read != 0) && !pending_service) {
      tester_command_t cmd = buffer[0];
      //bytes_read--; //as we already read 1 byte to parse the command
      switch (cmd) {
        case SET_LINE_RATE:
        {
          unsigned len = 0;
          pkt_control.rate = convert_atoi_substr(buffer+2, &len, bytes_read-2) % 100;
          //debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
        }
        break;

        case SET_GENERATOR_MODE:
          switch (buffer[2]) {
            case 's':
              pkt_control.mode = 0;
       	      break;
            case 'r':
              pkt_control.mode = 1;
       	      break;
            case 'd':
              pkt_control.mode = 2;
       	      break;
          }
       	  //debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
        break;

        case BROADCAST_SETTING:
        {
          unsigned len = 0;
          pkt_control.bcast.weight = convert_atoi_substr(buffer+2, &len, bytes_read-2) % 100;
          pkt_control.bcast.size = convert_atoi_substr(buffer+2+len, &len, bytes_read-2-len) % 1500;

          //debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
        }
        break;

        case MULTICAST_SETTING:
        {
          unsigned len = 0;
          pkt_control.mcast.weight = convert_atoi_substr(buffer+2, &len, bytes_read-2) % 100;
          pkt_control.mcast.size = convert_atoi_substr(buffer+2+len, &len, bytes_read-2-len) % 1500;

          //debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
        }
        break;

        case UNICAST_SETTING:
        {
          unsigned len = 0;
          pkt_control.ucast.weight = convert_atoi_substr(buffer+2, &len, bytes_read-2) % 100;
          pkt_control.ucast.size = convert_atoi_substr(buffer+2+len, &len, bytes_read-2-len) % 1500;

          //debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
        }
        break;

        case END_OF_CMD:
          pending_service = 1;
          debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
#if 0
          debug_printf("pkt_control.mode: %d\n", pkt_control.mode);
          debug_printf("pkt_control.rate: %d\n", pkt_control.rate);
          debug_printf("pkt_control.bcast.size: %d\n", pkt_control.bcast.size);
          debug_printf("pkt_control.bcast.weight: %d\n", pkt_control.bcast.weight);
          debug_printf("pkt_control.mcast.size: %d\n", pkt_control.mcast.size);
          debug_printf("pkt_control.mcast.weight: %d\n", pkt_control.mcast.weight);
          debug_printf("pkt_control.ucast.size: %d\n", pkt_control.ucast.size);
          debug_printf("pkt_control.ucast.weight: %d\n", pkt_control.ucast.weight);
#endif
        break;

        default:
          debug_printf("Unrecognised command '%s' with bytes: %d received from host\n", buffer, bytes_read);
        break;
      } //switch (cmd)
    }  //if (bytes_read != 0)
  } //while (1)
}
