#include <stddef.h>
#include <stdio.h>
#include <xccompat.h>
#include <stdint.h>

#include "xassert.h"
#include "random.h"
#include "common.h"
#include "xc_utils.h"
#include "debug_print.h"
#include "packet_controller.h"
#include "packet_generator.h"

char unicast_mac[6] = {0x00, 0x22, 0x97, 0x00, 0x42, 0xa6 };
char multicast_mac[6] = {0x01, 0x22, 0x97, 0x00, 0x42, 0xa6 };

typedef struct packet_data_t {
  unsigned delay;
  char dest_mac[6];
  char src_mac[6];
  char frame_type[2];
  char seq_num[4];
} packet_data_t;

static void fill_pkt_hdr(packet_data_t *pkt_dptr)
{
  static unsigned seq_num = 1;
  pkt_dptr->seq_num[3] = seq_num & 0xFF;
  pkt_dptr->seq_num[2] = (seq_num >> 8) & 0xFF;
  pkt_dptr->seq_num[1] = (seq_num >> 16) & 0xFF;
  pkt_dptr->seq_num[0] = (seq_num >> 24) & 0xFF;
  seq_num++;
}

static void gen_unicast_frame(packet_data_t *pkt_dptr)
{
  for (int i=0;i<6;i++) {
    pkt_dptr->dest_mac[i] = unicast_mac[i];
    pkt_dptr->src_mac[i] = 0xFF;
  }
  pkt_dptr->frame_type[0] = 0x89;
  pkt_dptr->frame_type[1] = 0x32;
  fill_pkt_hdr(pkt_dptr);
}

static void gen_multicast_frame(packet_data_t *pkt_dptr)
{
  for (int i=0;i<6;i++) {
    pkt_dptr->dest_mac[i] = multicast_mac[i];
    pkt_dptr->src_mac[i] = 0xFF;
  }
  pkt_dptr->frame_type[0] = 0x89;
  pkt_dptr->frame_type[1] = 0x33;
  fill_pkt_hdr(pkt_dptr);
}

static void gen_broadcast_frame(packet_data_t *pkt_dptr)
{
  for (int i=0;i<6;i++) {
    pkt_dptr->dest_mac[i] = 0xFF;
    pkt_dptr->src_mac[i] = 0xFF;
  }
  pkt_dptr->frame_type[0] = 0x89;
  pkt_dptr->frame_type[1] = 0x34;
  fill_pkt_hdr(pkt_dptr);
}

#if 1
pkt_ctrl_t unicast = { &gen_unicast_frame, 64, 1500, 1 };
pkt_ctrl_t multicast = { &gen_multicast_frame, 64, 1500, 90 };
pkt_ctrl_t broadcast = { &gen_broadcast_frame, 64, 1500, 1 };
#else
pkt_ctrl_t unicast = { &gen_unicast_frame, 1499, 1500, 10 };
pkt_ctrl_t multicast = { &gen_multicast_frame, 1499, 1500, 2 };
pkt_ctrl_t broadcast = { &gen_broadcast_frame, 1499, 1500, 2 };
#endif

pkt_ctrl_t *packet_type_none[] = { NULL };
pkt_ctrl_t *packet_type_all[] = { &unicast, &multicast, &broadcast, NULL };

pkt_ctrl_t *packet_type_unicast[] = { &unicast, NULL };
pkt_ctrl_t *packet_type_multicast[] = { &multicast, NULL };
pkt_ctrl_t *packet_type_broadcast[] = { &broadcast, NULL };

pkt_gen_ctrl_t unicast_only;
pkt_gen_ctrl_t multicast_only;
pkt_gen_ctrl_t broadcast_only;

pkt_gen_ctrl_t *choice[] = { &unicast_only, &multicast_only, &broadcast_only, NULL };

pkt_gen_ctrl_t initial = {
  0, 0, packet_type_none, 1, 1, choice
};

pkt_gen_ctrl_t *choice_initial[] = { &initial, NULL };

pkt_gen_ctrl_t unicast_only = {
    0, 0, packet_type_unicast, 1, 1, choice_initial
};

pkt_gen_ctrl_t multicast_only = {
    0, 0, packet_type_multicast, 1, 1, choice_initial
};

pkt_gen_ctrl_t broadcast_only = {
    0, 0, packet_type_broadcast, 1, 1, choice_initial
};

/* Begin - Directed mode configuration */
pkt_ctrl_t unicast_directed[2] = {
  { &gen_unicast_frame, 64, 1500, 35 },
  { &gen_unicast_frame, 64, 1500, 35 }
};
pkt_ctrl_t multicast_directed[2] = {
  { &gen_multicast_frame, 64, 1500, 30 },
  { &gen_multicast_frame, 64, 1500, 30 }
};
pkt_ctrl_t broadcast_directed[2] = {
  { &gen_broadcast_frame, 64, 1500, 40 },
  { &gen_broadcast_frame, 64, 1500, 40 }
};

pkt_ctrl_t *packet_type_directed0[] = { &unicast_directed[0], &multicast_directed[0], &broadcast_directed[0], NULL };
pkt_ctrl_t *packet_type_directed1[] = { &unicast_directed[1], &multicast_directed[1], &broadcast_directed[1], NULL };

pkt_gen_ctrl_t directed[2];

pkt_gen_ctrl_t *directed_choice0[] = { &directed[0], NULL };
pkt_gen_ctrl_t *directed_choice1[] = { &directed[1], NULL };

pkt_gen_ctrl_t directed[2] = {
  { 0, 0, packet_type_directed0, 1, 1, directed_choice0 },
  { 0, 0, packet_type_directed1, 1, 1, directed_choice1 }
};

pkt_ctrl_t *get_packet_control(pkt_type_t pkt_type, int index)
{
  switch (pkt_type) {
    case TYPE_UNICAST:
      return &unicast_directed[index];
    case TYPE_MULTICAST:
      return &multicast_directed[index];
    case TYPE_BROADCAST:
      return &broadcast_directed[index];
  }
  assert(0);
  return NULL;
}
/* End - Directed mode configuration */

volatile generator_mode_t g_generator_mode = GENERATOR_SILENT;

void get_generator_mode(generator_mode_t *mode)
{
  *mode = g_generator_mode;
}

void set_generator_mode(generator_mode_t mode)
{
  g_generator_mode = mode;
}

volatile int g_directed_read_index = 0;

void set_directed_read_index(int read_index)
{
  g_directed_read_index = read_index;
}

pkt_ctrl_t *choose_packet_type(random_generator_t *r, pkt_ctrl_t **choices, unsigned int *len)
{
    int total_weight = 0;
    pkt_ctrl_t **ptr = choices;
    while (*ptr) {
        total_weight += (*ptr)->weight;
        ptr++;
    }
    if (total_weight == 0)
      return NULL;

    unsigned choice_weight = random_get_random_number(r) % total_weight;
    ptr = choices;
    int cum_weight = 0;
    while (*ptr) {
      cum_weight += (*ptr)->weight;
      if (choice_weight < cum_weight) {
        /* Choose packet length */
        *len = random_get_random_number(r);
        if ((*ptr)->size_max == (*ptr)->size_min)
          *len = (*ptr)->size_min;
        else
          *len = (*len % ((*ptr)->size_max - (*ptr)->size_min)) + (*ptr)->size_min ;
        return *ptr;
      }
      ptr++;
    }
    *len = 0;
    return NULL;
}

pkt_gen_ctrl_t *choose_next(random_generator_t *r, pkt_gen_ctrl_t **choices)
{
    int total_weight = 0;
    pkt_gen_ctrl_t **ptr = choices;
    while (*ptr) {
        total_weight += (*ptr)->weight;
        ptr++;
    }
    if (total_weight == 0)
      return NULL;

    unsigned choice_weight = random_get_random_number(r) % total_weight;
    ptr = choices;
    int cum_weight = 0;
    while (*ptr) {
      cum_weight += (*ptr)->weight;
        if (choice_weight < cum_weight)
            return *ptr;
        ptr++;
    }
    return NULL;
}

// Need to inform tools of stack required because a function pointer is used
// for generator function.
#pragma stackfunction 100
void random_traffic_generator(CHANEND_PARAM(chanend, c_prod))
{
  pkt_gen_ctrl_t *ptr = &directed[0];
  random_generator_t r = random_create_generator_from_seed(0);
  uintptr_t dptr;
  unsigned len = 0;
  unsigned delay = 0;
  unsigned rate_factor = 0;

  // Rate calculation is a fixed-point factor to save using a divide in the critical loop
  const int bit_pos = 8;

  while (1) {
    /* check if there is any host request */
    if (get_pending_service_status()) {
      if (g_generator_mode == GENERATOR_RANDOM)
        ptr = &initial;
      else if (g_generator_mode == GENERATOR_DIRECTED)
        ptr = &directed[g_directed_read_index];

      rate_factor = get_rate_factor();
      reset_pending_service_status();
    }

    if ((g_generator_mode == GENERATOR_SILENT) || !ptr)
      continue;

    for (int i = 0; i < ptr->repeat; i++) {
      pkt_ctrl_t *packet = choose_packet_type(&r, ptr->packet_types, &len);
      if (packet) {
        dptr = get_buffer(c_prod);

        // The value can overflow if multiplying large packet lengths by maximum delay
        if (rate_factor >= (1 << bit_pos))
          delay = (len * (rate_factor >> bit_pos));
        else
          delay = ((len << bit_pos) * rate_factor) >> bit_pos;

        ((packet_data_t *)dptr)->delay = delay;

        // Call the generator function to populate the packet
        packet->p_gen_packet((packet_data_t *)dptr);

        // Send pointer and length to transmitter
        put_buffer(c_prod, dptr);
        put_buffer_int(c_prod, len + sizeof(((packet_data_t *)dptr)->delay));
      }
    }
    ptr = choose_next(&r, ptr->next);
  }  //while(1)
}

