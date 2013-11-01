#include <platform.h>
#include <xscope.h>
#include <stdio.h>
#include <stdint.h>
#include <xccompat.h>

#include "debug_print.h"
#include "traffic_ctlr_host_cmds.h"
#include "packet_controller.h"
#include "common.h"
#include "packet_generator.h"
#include "c_utils.h"

/* A flag to indicate a pending host command to be serviced */
volatile unsigned pending_service;

unsigned get_pending_service_status(void)
{
  return pending_service;
}

void reset_pending_service_status(void)
{
  pending_service = 0;
}

/**
 * \brief   A core that listens to data being sent from the host and
 *          informs the analysis engine of any changes
 */
void xscope_listener()
{
  // The maximum read size is 256 bytes
#define MAX_BYTES_READ	256
  unsigned char buffer[MAX_BYTES_READ] = {'\0'};

  while (1) {
    unsigned bytes_read = xscope_upload_bytes(-1, (unsigned char *)&buffer[0]);
    if ((bytes_read != 0) && !pending_service) {
      /* Clear the unused portion of the buffer */
      for (int j = bytes_read; j < MAX_BYTES_READ; j++)
        buffer[j] = '\0';

      tester_command_t cmd = buffer[0];
      switch (cmd) {
        case SET_GENERATOR_MODE:
          switch (buffer[2]) {
            case 's':
              set_generator_mode(GENERATOR_SILENT);
       	      break;
            case 'r':
              set_generator_mode(GENERATOR_RANDOM);
       	      break;
            case 'd':
              set_generator_mode(GENERATOR_DIRECTED);
       	      break;
          }
       	  debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
        break;

        case PKT_CONTROL:
        {
          pkt_ctrl_t *pkt_ctrl = NULL;
          pkt_type_t pkt_type;
          unsigned index = 2; //already cmd and white space is accounted
          unsigned len = 0;

          if (buffer[index] == 'u')
        	pkt_type = TYPE_UNICAST;
          else if (buffer[index] == 'm')
        	pkt_type = TYPE_MULTICAST;
          else if (buffer[index] == 'b')
        	pkt_type = TYPE_BROADCAST;

          set_packet_control(&pkt_ctrl, pkt_type);
          index += 2;
          pkt_ctrl->weight = convert_atoi_substr(buffer+index, &len, bytes_read-2) % 100;
          debug_printf("pkt_ctrl->weight: %d\n", pkt_ctrl->weight);
          index += len+1; //+1 for white space
          pkt_ctrl->size_min = convert_atoi_substr(buffer+index, &len, bytes_read-2) % 1500;
          debug_printf("pkt_ctrl->size_min: %d\n", pkt_ctrl->size_min);
          index += len+1; //+1 for white space
          pkt_ctrl->size_max = convert_atoi_substr(buffer+index, &len, bytes_read-2) % 1500;
          debug_printf("pkt_ctrl->size_max: %d\n", pkt_ctrl->size_max);

          debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
        }
        break;

        case PKT_GENERATION_CONTROL:
        {
          pkt_gen_ctrl_t *pkt_gen_ctrl = NULL;
          pkt_type_t pkt_type;
          unsigned index = 2; //already cmd and white space is accounted
          unsigned len = 0;

          if (buffer[index] == 'u')
        	pkt_type = TYPE_UNICAST;
          else if (buffer[index] == 'm')
        	pkt_type = TYPE_MULTICAST;
          else if (buffer[index] == 'b')
        	pkt_type = TYPE_BROADCAST;

          set_packet_generation_control(&pkt_gen_ctrl, pkt_type);
          index += 2;
          pkt_gen_ctrl->weight = convert_atoi_substr(buffer+index, &len, bytes_read-2) % 100;
          debug_printf("pkt_gen_ctrl->weight: %d\n", pkt_gen_ctrl->weight);
          index += len+1; //+1 for white space
          pkt_gen_ctrl->delay_max = convert_atoi_substr(buffer+index, &len, bytes_read-2) % 100;
          debug_printf("pkt_gen_ctrl->delay_max: %d\n", pkt_gen_ctrl->delay_max);

          debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
        }
        break;

        case PRINT_PKT_CONFIGURATION:
        {
          pkt_ctrl_t *pkt_ctrl = NULL;
          pkt_gen_ctrl_t *pkt_gen_ctrl = NULL;
          generator_mode_t mode;

          get_generator_mode(&mode);
          debug_printf("Packet generator is running in mode: %d\n", mode);
          debug_printf("Modes are 0:(s)ilent, 1:(r)andom, 2:(d)irected\n");
          debug_printf("\n");

          set_packet_control(&pkt_ctrl, TYPE_UNICAST);
          debug_printf("Unicast pkt weight: %d\n", pkt_ctrl->weight);
          debug_printf("Unicast pkt size_min: %d\n", pkt_ctrl->size_min);
          debug_printf("Unicast pkt size_max: %d\n", pkt_ctrl->size_max);

          set_packet_control(&pkt_ctrl, TYPE_MULTICAST);
          debug_printf("Multicast pkt weight: %d\n", pkt_ctrl->weight);
          debug_printf("Multicast pkt size_min: %d\n", pkt_ctrl->size_min);
          debug_printf("Multicast pkt size_max: %d\n", pkt_ctrl->size_max);

          set_packet_control(&pkt_ctrl, TYPE_BROADCAST);
          debug_printf("Broadcast  pkt weight: %d\n", pkt_ctrl->weight);
          debug_printf("Broadcast  pkt size_min: %d\n", pkt_ctrl->size_min);
          debug_printf("Broadcast  pkt size_max: %d\n", pkt_ctrl->size_max);

          set_packet_generation_control(&pkt_gen_ctrl, TYPE_UNICAST);
          debug_printf("Unicast control weight: %d\n", pkt_gen_ctrl->weight);
          debug_printf("Unicast control delay_max: %d\n", pkt_gen_ctrl->delay_max);

          set_packet_generation_control(&pkt_gen_ctrl, TYPE_MULTICAST);
          debug_printf("Multicast control weight: %d\n", pkt_gen_ctrl->weight);
          debug_printf("Multicast control delay_max: %d\n", pkt_gen_ctrl->delay_max);

          set_packet_generation_control(&pkt_gen_ctrl, TYPE_BROADCAST);
          debug_printf("Broadcast  control weight: %d\n", pkt_gen_ctrl->weight);
          debug_printf("Broadcast  control delay_max: %d\n", pkt_gen_ctrl->delay_max);

          debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
        }
        break;

        case END_OF_CMD:
          pending_service = 1;
          debug_printf("Buffer data %s with bytes: %d\n", buffer, bytes_read);
        break;

        default:
          debug_printf("Unrecognised command '%s' with bytes: %d received from host\n", buffer, bytes_read);
        break;
      } //switch (cmd)
    }  //if (bytes_read != 0)
  } //while (1)
}
