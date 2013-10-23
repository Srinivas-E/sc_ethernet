#ifndef TRAFFIC_GENERATOR_H_
#define TRAFFIC_GENERATOR_H_
#include <xccompat.h>
#include <stdint.h>

#if 0
typedef struct pkt_type_t {
  int weight;
  int size;
#ifdef __XC__
  struct pkt_type_t * unsafe next;
#else
  struct pkt_type_t *next;
#endif
  char type; //b-broadcast; m-multicast; u-unicast
} pkt_type_t;

typedef struct pkt_control_t {
  int rate;
  int mode;
#ifdef __XC__
  pkt_type_t * unsafe pkt_type;
#else
  pkt_type_t *pkt_type;
#endif
} pkt_control_t;
#else
typedef struct pkt_type_t {
  int weight;
  int size;
} pkt_type_t;

typedef struct pkt_control_t {
  int rate;
  int mode; //0-silent; 1-random; 2-directed
  pkt_type_t bcast;
  pkt_type_t mcast;
  pkt_type_t ucast;
} pkt_control_t;
#endif

void get_control_commands(REFERENCE_PARAM(uintptr_t, pkt_control));
unsigned get_pending_service_status(void);
void reset_pending_service_status(void);
void xscope_listener();

#endif /* TRAFFIC_GENERATOR_H_ */
