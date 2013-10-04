#include <stddef.h>
#include <stdio.h>
#include "stdint.h"
#include "xc_utils.h"

typedef enum {
    PACKET_TYPE_UNICAST,
    PACKET_TYPE_MULTICAST,
    PACKET_TYPE_BROADCAST,
} packet_type_t;
    
typedef struct random_packet_t {
    packet_type_t type;
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

random_packet_t unicast = { PACKET_TYPE_UNICAST, 64, 1500, 10 };
random_packet_t multicast = { PACKET_TYPE_MULTICAST, 64, 1500, 2 };
random_packet_t broadcast = { PACKET_TYPE_BROADCAST, 64, 1500, 2 };
random_packet_t *packet_type_all[] = { &unicast, &multicast, &broadcast, NULL };
random_packet_t *packet_type_broadcast[] = { &broadcast, NULL };
random_packet_t *packet_type_unicast[] = { &unicast, NULL };

random_control_t broadcast_only;
random_control_t unicast_only;

random_control_t *choice[] = { &unicast_only, &broadcast_only, NULL };

random_control_t initial = {
    10000000, 40000000, packet_type_all, 1, 10, choice
};

random_control_t *choice_initial[] = { &initial, NULL };

random_control_t broadcast_only = {
    0, 0, packet_type_broadcast, 10, 5, choice_initial
};

random_control_t unicast_only = {
    0, 0, packet_type_unicast, 6, 3, choice_initial
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
    	*len = *len % ((*ptr)->size_max - (*ptr)->size_min);
    	return *ptr;
      }
      ptr++;
      //choice_weight -= (*ptr)->weight;
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
        //choice_weight -= (*ptr)->weight;
    }
    return NULL;
}

#define STW(offset,value) \
  asm volatile("stw %0, %1[%2]"::"r"(value), "r"(dptr), "r"(offset):"memory");

static void fill_pkt_type(unsigned pkt_type, char tx_buf[]) {
  for (int i = 0; i < 12; i++)
	  tx_buf[i] = 0xFF;//TODO: fill apropriate type

  /* Set arbitrary ether type */
  tx_buf[12] = 0x89;//TODO: fill apropriate type
  tx_buf[13] = 0x12;
  /* Generate a packet key */
  generate_seq_num((unsigned char *) &tx_buf[14]);
}

void random_traffic_generator(CHANEND_PARAM(chanend, c_prod))
{
    random_control_t *ptr = &initial;
    random_generator_t r = random_create_generator_from_seed(0);
    uintptr_t dptr;
    unsigned len = 0;
    unsigned delay = 0;
    unsigned mac = 0xFFFFFFFF;
    unsigned type_snum = 0x8912ABCD;

    while (1) {
    	for (int i = 0; i < ptr->repeat; i++) {
       	  random_packet_t *packet = choose_packet_type(&r, ptr->packet_types, &len);
       	  printf("Packet type %d and pkt_len %d\n", packet->type, len);
       	  dptr = get_buffer(c_prod);
       	  delay = get_delay(&r, ptr->delay_min, ptr->delay_max);
       	  STW(0, delay);
       	  /* generate buffer contents of packet->type and len which represent pkt to transmit */
       	  STW(1, mac);
       	  STW(2, mac);
       	  STW(3, mac);
       	  STW(4, type_snum);

       	  put_buffer(c_prod, dptr);
       	  put_buffer_int(c_prod, len+(1*4)); //add byte count for wait and len values
        }
        ptr = choose_next(&r, ptr->next);
    }
}

