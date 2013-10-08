#ifndef BUFFER_MANAGER_H_
#define BUFFER_MANAGER_H_

#ifdef __XC__
void buffer_manager(chanend c_prod[], int num_prod, chanend c_cons);
//#else
//void buffer_manager(unsigned c_prod[], int num_prod, unsigned c_cons);
#endif

#endif /* BUFFER_MANAGER_H_ */
