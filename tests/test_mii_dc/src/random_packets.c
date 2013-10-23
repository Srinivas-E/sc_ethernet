#include <stddef.h>
#include <stdio.h>
#include "stdint.h"
#include "xc_utils.h"
#include "debug_print.h"
#include "traffic_generator.h"

char unicast_mac[6] = {0x00, 0x22, 0x97, 0x00, 0x42, 0xa6 };
char multicast_mac[6] = {0x01, 0x22, 0x97, 0x00, 0x42, 0xa6 };

typedef enum {
  GENERATOR_RANDOM,
  GENERATOR_DIRECTED,
} generator_mode_t;

typedef struct random_packet_t {
    void (*p_gen_packet)();
    unsigned int size_min;
    unsigned int size_max;
    int weight;
} random_packet_t;

// Random packet generation control
typedef struct random_control_t {
    unsigned delay_min;
    unsigned delay_max;
    random_packet_t **packet_types;
    int weight;
    int repeat;
    struct random_control_t **next;
} random_control_t;

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

generator_mode_t run_mode;
#if 1
random_packet_t unicast = { &gen_unicast_frame, 64, 1500, 5 };
random_packet_t multicast = { &gen_multicast_frame, 64, 1500, 0 };
random_packet_t broadcast = { &gen_broadcast_frame, 64, 1500, 90 };
#else
random_packet_t unicast = { &gen_unicast_frame, 1499, 1500, 10 };
random_packet_t multicast = { &gen_multicast_frame, 1499, 1500, 2 };
random_packet_t broadcast = { &gen_broadcast_frame, 1499, 1500, 2 };
#endif

random_packet_t *packet_type_none[] = { NULL };
random_packet_t *packet_type_all[] = { &unicast, &multicast, &broadcast, NULL };
random_packet_t *packet_type_broadcast[] = { &broadcast, NULL };
random_packet_t *packet_type_multicast[] = { &multicast, NULL };
random_packet_t *packet_type_unicast[] = { &unicast, NULL };

random_control_t broadcast_only;
random_control_t multicast_only;
random_control_t unicast_only;

random_control_t *choice[] = { &unicast_only, &multicast_only, &broadcast_only, NULL };

random_control_t initial = {
	//0, 0, packet_type_all, 1, 10, choice
	0, 0, packet_type_none, 1, 1, choice
};

random_control_t *choice_initial[] = { &initial, NULL };

random_control_t broadcast_only = {
    0, 0, packet_type_broadcast, 1, 1, choice_initial
};

random_control_t unicast_only = {
    0, 0, packet_type_unicast, 1, 1, choice_initial
};

random_control_t multicast_only = {
    0, 0, packet_type_multicast, 1, 1, choice_initial
};

random_packet_t *choose_packet_type(random_generator_t *r, random_packet_t **choices, unsigned int *len)
{
    int total_weight = 0;
    random_packet_t **ptr = choices;
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

random_control_t *choose_next(random_generator_t *r, random_control_t **choices)
{
    int total_weight = 0;
    random_control_t **ptr = choices;
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
    random_control_t *ptr = &initial;
    random_generator_t r = random_create_generator_from_seed(0);
    uintptr_t dptr;
    unsigned len = 0;
    unsigned delay = 0;

    while (1) {
    	for (int i = 0; i < ptr->repeat; i++) {
       	  random_packet_t *packet = choose_packet_type(&r, ptr->packet_types, &len);
       	  if (packet) {
           	  //debug_printf("Packet len %d\n", len);
           	  dptr = get_buffer(c_prod);
           	  delay = get_delay(&r, ptr->delay_min, ptr->delay_max);
           	  packet->p_gen_packet((packet_data_t *)dptr, delay);
           	  put_buffer(c_prod, dptr);
           	  put_buffer_int(c_prod, len+(1*4)); //add byte count for delay value
       	  }
        }
    	/* check if there is any host request */
    	if (get_pending_service_status()) {
          //populate random_control_t obtained from the host
    	  //ptr = &initial;
    	  pkt_control_t *ctrl_ptr = NULL;
    	  get_control_commands((uintptr_t *) &ctrl_ptr);

    	  initial.delay_min = 0;
    	  initial.delay_max = ctrl_ptr->rate;

    	  unicast.size_min = ctrl_ptr->ucast.size-1;
    	  unicast.size_max = ctrl_ptr->ucast.size;
    	  unicast.weight = ctrl_ptr->ucast.weight;

    	  multicast.size_min = ctrl_ptr->mcast.size-1;
    	  multicast.size_max = ctrl_ptr->mcast.size;
    	  multicast.weight = ctrl_ptr->mcast.weight;

    	  broadcast.size_min = ctrl_ptr->bcast.size-1;
    	  broadcast.size_max = ctrl_ptr->bcast.size;
    	  broadcast.weight = ctrl_ptr->bcast.weight;
/*
          debug_printf("ctrl_ptr->mode: %d\n", ctrl_ptr->mode);
          debug_printf("ctrl_ptr->rate: %d\n", ctrl_ptr->rate);
          debug_printf("ctrl_ptr->bcast.size: %d\n", ctrl_ptr->bcast.size);
          debug_printf("ctrl_ptr->bcast.weight: %d\n", ctrl_ptr->bcast.weight);
          debug_printf("ctrl_ptr->mcast.size: %d\n", ctrl_ptr->mcast.size);
          debug_printf("ctrl_ptr->mcast.weight: %d\n", ctrl_ptr->mcast.weight);
          debug_printf("ctrl_ptr->ucast.size: %d\n", ctrl_ptr->ucast.size);
          debug_printf("ctrl_ptr->ucast.weight: %d\n", ctrl_ptr->ucast.weight);
*/
    	  reset_pending_service_status();
    	  ctrl_ptr = NULL;
    	}
    	else
          ptr = choose_next(&r, ptr->next);
    }
}

