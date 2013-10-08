#ifndef BUFFERS_CONF_H_
#define BUFFERS_CONF_H_

#define	APP_BUFFER_CONFIG
#define BUFFER_COUNT 32			//Define maximum buffers used by the application
#define NUM_BUF_PRODUCERS	1

typedef struct buffer_desc_t {
  unsigned wait;
  unsigned pkt_len;
} buffer_desc_t;

#endif /* BUFFERS_CONF_H_ */
