#include <platform.h>
#include <xscope.h>
#include <stdio.h>
#include <stdint.h>
#include <xccompat.h>

#include "xassert.h"

#include "debug_print.h"
#include "traffic_ctlr_host_cmds.h"
#include "packet_controller.h"
#include "common.h"
#include "packet_generator.h"
#include "c_utils.h"

/* A flag to indicate a pending host command to be serviced */
volatile unsigned g_pending_service = 0;

/* The multiplier used to control the line rate */
volatile unsigned g_rate_factor[2] = {0, 0};

/* The index into the table of configurations - start on second entry */
unsigned int g_directed_write_index = 1;

/* Indicate whether the state should be copied or left alone when complete */
int g_copy_over_config = 1;

unsigned get_pending_service_status(void)
{
  return g_pending_service;
}

void reset_pending_service_status(void)
{
  g_pending_service = 0;

  if (g_copy_over_config) {
    int directed_read_index = g_directed_write_index ? 0 : 1;
    pkt_ctrl_t *read = get_packet_control(TYPE_UNICAST, directed_read_index);
    pkt_ctrl_t *write = get_packet_control(TYPE_UNICAST, g_directed_write_index);
    *write = *read;

    read = get_packet_control(TYPE_MULTICAST, directed_read_index);
    write = get_packet_control(TYPE_MULTICAST, g_directed_write_index);
    *write = *read;

    read = get_packet_control(TYPE_BROADCAST, directed_read_index);
    write = get_packet_control(TYPE_BROADCAST, g_directed_write_index);
    *write = *read;

    g_rate_factor[g_directed_write_index] = g_rate_factor[directed_read_index];
  }
}

unsigned get_rate_factor(void)
{
  int directed_read_index = g_directed_write_index ? 0 : 1;
  return g_rate_factor[directed_read_index];
}

/**
 * \brief   A function that processes data being sent from the host and
 *          informs the analysis engine of any changes
 */
void handle_host_data(unsigned char buffer[], int bytes_read)
{
  tester_command_t cmd = buffer[0];
  const unsigned char *ptr = &buffer[1]; // Skip command
  switch (cmd) {
    case SET_GENERATOR_MODE:
      {
        unsigned char c = get_next_char(&ptr);
        switch (c) {
          case 's': set_generator_mode(GENERATOR_SILENT); break;
          case 'r': set_generator_mode(GENERATOR_RANDOM); break;
          case 'd': set_generator_mode(GENERATOR_DIRECTED); break;
          default : return;
        }
      }
      break;

    case PKT_CONTROL:
      {
        unsigned char c = get_next_char(&ptr);
        pkt_ctrl_t *pkt_ctrl = NULL;
        pkt_type_t pkt_type;

        switch (c) {
          case 'u': pkt_type = TYPE_UNICAST; break;
          case 'm': pkt_type = TYPE_MULTICAST; break;
          case 'b': pkt_type = TYPE_BROADCAST; break;
          default : return;
        }

        pkt_ctrl = get_packet_control(pkt_type, g_directed_write_index);
        pkt_ctrl->weight = convert_atoi_substr(&ptr);
        if (pkt_ctrl->weight) {
          pkt_ctrl->size_min = convert_atoi_substr(&ptr);
          pkt_ctrl->size_max = convert_atoi_substr(&ptr);
        }
      }
      break;

    case LINE_RATE:
      g_rate_factor[g_directed_write_index] = convert_atoi_substr(&ptr);
      break;

    case PRINT_PKT_CONFIGURATION:
      {
        pkt_ctrl_t *pkt_ctrl = NULL;
        generator_mode_t mode;

        get_generator_mode(&mode);
        debug_printf("Packet generator is running in %s mode\n",
            (mode == 0) ? "silent" : (mode == 1) ? "random" : "directed");

        int directed_read_index = g_directed_write_index ? 0 : 1;
        debug_printf("Current configuration (%d rate factor)\n", g_rate_factor[directed_read_index]);
        pkt_ctrl = get_packet_control(TYPE_UNICAST, directed_read_index);
        debug_printf("Unicast   weight %d, packet bytes %d-%d\n", pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max);
        pkt_ctrl = get_packet_control(TYPE_MULTICAST, directed_read_index);
        debug_printf("Multicast weight %d, packet bytes %d-%d\n", pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max);
        pkt_ctrl = get_packet_control(TYPE_BROADCAST, directed_read_index);
        debug_printf("Broadcast weight %d, packet bytes %d-%d\n", pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max);

        debug_printf("Next configuration (%d rate factor):\n", g_rate_factor[g_directed_write_index]);
        pkt_ctrl = get_packet_control(TYPE_UNICAST, g_directed_write_index);
        debug_printf("Unicast   weight %d, packet bytes %d-%d\n", pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max);
        pkt_ctrl = get_packet_control(TYPE_MULTICAST, g_directed_write_index);
        debug_printf("Multicast weight %d, packet bytes %d-%d\n", pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max);
        pkt_ctrl = get_packet_control(TYPE_BROADCAST, g_directed_write_index);
        debug_printf("Broadcast weight %d, packet bytes %d-%d\n", pkt_ctrl->weight, pkt_ctrl->size_min, pkt_ctrl->size_max);

        debug_printf("Press 's' to swap, press 'e' to update and copy\n");
      }
      break;

    case SWAP_CFG:
      g_copy_over_config = 0;
      // fall through

    case END_OF_CFG:
      set_directed_read_index(g_directed_write_index);
      g_directed_write_index = g_directed_write_index ? 0 : 1;
      g_pending_service = 1;
      break;

    default:
      debug_printf("Unrecognised command '%s' with bytes: %d received from host\n", buffer, bytes_read);
      break;
  } //switch (cmd)
}
