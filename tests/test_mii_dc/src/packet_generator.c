#include <stddef.h>
#include <stdio.h>
#include <xccompat.h>
#include <stdint.h>

#include "random.h"
#include "common.h"
#include "xc_utils.h"
#include "debug_print.h"
#include "packet_controller.h"
#include "packet_generator.h"

char unicast_mac[6] = {0x00, 0x22, 0x97, 0x00, 0x42, 0xa6 };
char multicast_mac[6] = {0x01, 0x22, 0x97, 0x00, 0x42, 0xa6 };
/*
typedef struct pkt_ctrl_t {
    void (*p_gen_packet)();
    unsigned int size_min;
    unsigned int size_max;
    int weight;
} pkt_ctrl_t;

// Packet generation control
typedef struct pkt_gen_ctrl_t {
    unsigned delay_min;
    unsigned delay_max;
    pkt_ctrl_t **packet_types;
    int weight;
    int repeat;
    struct pkt_gen_ctrl_t **next;
} pkt_gen_ctrl_t;
*/
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

static void gen_unicast_frame(packet_data_t *pkt_dptr, unsigned delay)
{
  pkt_dptr->delay = delay;
  for (int i=0;i<6;i++) {
	pkt_dptr->dest_mac[i] = unicast_mac[i];
	pkt_dptr->src_mac[i] = 0xFF;
  }
  pkt_dptr->frame_type[0] = 0x89;
  pkt_dptr->frame_type[1] = 0x32;
  fill_pkt_hdr(pkt_dptr);
}

static void gen_multicast_frame(packet_data_t *pkt_dptr, unsigned delay)
{
  pkt_dptr->delay = delay;
  for (int i=0;i<6;i++) {
	pkt_dptr->dest_mac[i] = multicast_mac[i];
	pkt_dptr->src_mac[i] = 0xFF;
  }
  pkt_dptr->frame_type[0] = 0x89;
  pkt_dptr->frame_type[1] = 0x33;
  fill_pkt_hdr(pkt_dptr);
}

static void gen_broadcast_frame(packet_data_t *pkt_dptr, unsigned delay)
{
  pkt_dptr->delay = delay;
  for (int i=0;i<6;i++) {
	pkt_dptr->dest_mac[i] = 0xFF;
	pkt_dptr->src_mac[i] = 0xFF;
  }
  pkt_dptr->frame_type[0] = 0x89;
  pkt_dptr->frame_type[1] = 0x34;
  fill_pkt_hdr(pkt_dptr);
}

volatile generator_mode_t generator_mode; //initially in SILENT mode

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
pkt_ctrl_t unicast_directed = { &gen_unicast_frame, 64, 1500, 35 };
pkt_ctrl_t multicast_directed = { &gen_multicast_frame, 64, 1500, 30 };
pkt_ctrl_t broadcast_directed = { &gen_broadcast_frame, 64, 1500, 40 };

pkt_ctrl_t *packet_type_unicast_directed[] = { &unicast_directed, NULL };
pkt_ctrl_t *packet_type_multicast_directed[] = { &multicast_directed, NULL };
pkt_ctrl_t *packet_type_broadcast_directed[] = { &broadcast_directed, NULL };

pkt_gen_ctrl_t unicast_directed_only;
pkt_gen_ctrl_t multicast_directed_only;
pkt_gen_ctrl_t broadcast_directed_only;

pkt_gen_ctrl_t *directed_choice[] = { &unicast_directed_only, &multicast_directed_only, &broadcast_directed_only, NULL };

pkt_gen_ctrl_t directed_initial = {
	0, 0, packet_type_none, 1, 1, directed_choice
};

pkt_gen_ctrl_t *directed_choice_initial[] = { &directed_initial, NULL };

pkt_gen_ctrl_t unicast_directed_only = {
    0, 0, packet_type_unicast_directed, 1, 1, directed_choice_initial
};

pkt_gen_ctrl_t multicast_directed_only = {
    0, 0, packet_type_multicast_directed, 1, 1, directed_choice_initial
};

pkt_gen_ctrl_t broadcast_directed_only = {
    0, 0, packet_type_broadcast_directed, 1, 1, directed_choice_initial
};

void set_packet_control(pkt_ctrl_t **pkt_ctrl, pkt_type_t pkt_type)
{
  switch (pkt_type) {
    case TYPE_UNICAST:
      *pkt_ctrl = &unicast_directed;
      break;
    case TYPE_MULTICAST:
      *pkt_ctrl = &multicast_directed;
      break;
    case TYPE_BROADCAST:
      *pkt_ctrl = &broadcast_directed;
      break;
  }
}

void set_packet_generation_control(pkt_gen_ctrl_t **pkt_gen_ctrl, pkt_type_t pkt_type)
{
  switch (pkt_type) {
    case TYPE_UNICAST:
      *pkt_gen_ctrl = &unicast_directed_only;
      break;
    case TYPE_MULTICAST:
      *pkt_gen_ctrl = &multicast_directed_only;
      break;
    case TYPE_BROADCAST:
      *pkt_gen_ctrl = &broadcast_directed_only;
      break;
  }
}
/* End - Directed mode configuration */

void get_generator_mode(generator_mode_t *mode)
{
	*mode = generator_mode;
}

void set_generator_mode(generator_mode_t mode)
{
	generator_mode = mode;
}

pkt_ctrl_t *choose_packet_type(random_generator_t *r, pkt_ctrl_t **choices, unsigned int *len)
{
    int total_weight = 0;
    pkt_ctrl_t **ptr = choices;
    while (*ptr) {
        total_weight += (*ptr)->weight;
        ptr++;
    }
    unsigned choice_weight = random_get_random_number(r);
    if (total_weight != 0)
        choice_weight = choice_weight % total_weight;
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
    unsigned choice_weight = random_get_random_number(r);
    if (total_weight != 0)
        choice_weight = choice_weight % total_weight;
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

#pragma stackfunction 100
void random_traffic_generator(CHANEND_PARAM(chanend, c_prod))
{
  pkt_gen_ctrl_t *ptr = &initial;
  random_generator_t r = random_create_generator_from_seed(0);
  uintptr_t dptr;
  unsigned len = 0;
  unsigned delay = 0;

  while (1) {
	/* check if there is any host request */
	if (get_pending_service_status()) {
	  if (generator_mode == GENERATOR_RANDOM)
		ptr = &initial;
	  else if (generator_mode == GENERATOR_DIRECTED)
		ptr = &directed_initial;
	  reset_pending_service_status();
    }

	if (generator_mode != GENERATOR_SILENT) {
	  for (int i = 0; i < ptr->repeat; i++) {
		pkt_ctrl_t *packet = choose_packet_type(&r, ptr->packet_types, &len);
		if (packet) {
		  //debug_printf("Packet len %d\n", len);
		  dptr = get_buffer(c_prod);
		  delay = get_delay(&r, ptr->delay_min, ptr->delay_max);
		  packet->p_gen_packet((packet_data_t *)dptr, delay);
		  put_buffer(c_prod, dptr);
		  put_buffer_int(c_prod, len+(1*4)); //add byte count for delay value
		}
	  }
	  ptr = choose_next(&r, ptr->next);
	}  //if (generator_mode != GENERATOR_SILENT) {
  }  //while(1)
}

