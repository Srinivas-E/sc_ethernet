#ifndef __PACKET_GENERATOR_H__
#define __PACKET_GENERATOR_H__

typedef enum {
  GENERATOR_SILENT,
  GENERATOR_RANDOM,
  GENERATOR_DIRECTED,
} generator_mode_t;

typedef enum {
  TYPE_UNICAST,
  TYPE_MULTICAST,
  TYPE_BROADCAST,
} pkt_type_t;

#ifndef __XC__
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
#endif //__XC__

void random_traffic_generator(CHANEND_PARAM(chanend, c_tx));
void get_generator_mode(generator_mode_t *mode);
void set_generator_mode(generator_mode_t mode);
void set_directed_read_index(int read_index);
#ifndef __XC__
pkt_ctrl_t *get_packet_control(pkt_type_t pkt_type, int index);
#endif //__XC__

#endif // __PACKET_GENERATOR_H__
