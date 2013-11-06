#ifndef PACKET_CONTROLLER_H_
#define PACKET_CONTROLLER_H_

unsigned get_pending_service_status();
void reset_pending_service_status();

unsigned get_rate_factor();

void handle_host_data(unsigned char buffer[], int bytes_read);

#endif /* PACKET_CONTROLLER_H_ */
