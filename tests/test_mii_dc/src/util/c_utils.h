#ifndef __C_UTILS_H__
#define __C_UTILS_H__

void send_ether_frame(CHANEND_PARAM(chanend, c_tx), uintptr_t dptr, unsigned int nbytes);
char get_next_char(const unsigned char **buffer);
int convert_atoi_substr(const unsigned char **buffer);
//int validate_pkt_setting(char buffer[]);

#endif // __C_UTILS_H__
