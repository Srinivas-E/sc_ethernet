#include <stddef.h>
#include <stdio.h>
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

random_packet_t *choose_packet_type(random_generator_t *r, random_packet_t **choices)
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
        if (choice_weight < cum_weight)
            return *ptr;
        ptr++;
        //choice_weight -= (*ptr)->weight;
    }
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

static void generate_ether_frame(random_generator_t *r, random_packet_t *packet_type, unsigned char tx_buf[], unsigned int *len)
{
  switch (packet_type->type) {
	case(PACKET_TYPE_UNICAST):
	  for (int i = 0; i < 12; i++)
		tx_buf[i] = 0xFF;
	break;
	case(PACKET_TYPE_MULTICAST):
	  for (int i = 0; i < 12; i++)
		tx_buf[i] = 0xFF;//TODO: fill apropriate type
	break;
	case(PACKET_TYPE_BROADCAST):
	  for (int i = 0; i < 12; i++)
		tx_buf[i] = 0xFF;//TODO: fill apropriate type
	break;
	default:
		break;
  }
  /* Set arbitrary ether type */
  tx_buf[12] = 0xAB;//TODO: fill apropriate type
  tx_buf[13] = 0xCD;

  /* Generate a packet key */
  generate_seq_num((unsigned char *) &tx_buf[14]);

  /* Choose packet length */
  *len = random_get_random_number(r);
  *len = *len % (packet_type->size_max - packet_type->size_min);
}

void random_traffic_generator(CHANEND_PARAM(chanend, c_tx))
{
    random_control_t *ptr = &initial;
    random_generator_t r = random_create_generator_from_seed(0);
	unsigned int tx_buf[1600/4];
#define MAX_PKT_LEN 1000
	unsigned int len = MAX_PKT_LEN;

	/* Init data contents of the buffer */
	char * buf = (char * )&tx_buf;
	for (int i=18; i<MAX_PKT_LEN; i++) {
		buf[i] = i;
	}

    while (1) {
        for (int i = 0; i < ptr->repeat; i++) {
            //printf("Wait %d-%d\n", ptr->delay_min, ptr->delay_max);
            wait(&r, ptr->delay_min, ptr->delay_max);

            random_packet_t *packet = choose_packet_type(&r, ptr->packet_types);
            generate_ether_frame(&r, packet, (unsigned char *)tx_buf, &len);
            printf("Packet type %d and pkt_len %d\n", packet->type, len);
            send_ether_frame(c_tx, tx_buf, len);
        }
        ptr = choose_next(&r, ptr->next);
    }
}
