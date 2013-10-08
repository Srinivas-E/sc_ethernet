#include <platform.h>
#include <xscope.h>
#include <stdint.h>
#include <print.h>
#include <xclib.h>
#include "xassert.h"
#include "buffers.h"

static inline void process_received(chanend c, int &work_pending,
    buffers_used_t &used_buffers, buffers_free_t &free_buffers, uintptr_t buffer, int &all_buffers_used)
{
  unsigned length_in_bytes;
  c :> length_in_bytes;

  buffers_used_add(used_buffers, buffer, length_in_bytes);
  work_pending++;
  if (buffers_used_full(used_buffers) || free_buffers.top_index == 0) {
    // No more buffers
    //assert(0);
	all_buffers_used = 1;
  } else {
    //buffers_used_add(used_buffers, buffer, length_in_bytes);
    //work_pending++;
    c <: buffers_free_acquire(free_buffers);
  }
}

void buffer_manager(chanend c_prod[], int num_prod, chanend c_con)
{
  buffers_used_t used_buffers;
  buffers_used_initialise(used_buffers);

  buffers_free_t free_buffers;
  buffers_free_initialise(free_buffers);

  //start by issuing buffers to both of the miis
  for (int i=0; i<num_prod;i++) {
	  c_prod[i] <: buffers_free_acquire(free_buffers);
  }

  int sender_active = 0;
  int work_pending = 0;
  int all_buffers_used = 0;

  while (1) {
    for (int i=0; i<num_prod;i++) {
      select {
        case c_prod[i] :> uintptr_t buffer: {
          process_received(c_prod[i], work_pending, used_buffers, free_buffers, buffer, all_buffers_used);
          break;
		}
		case sender_active => c_con :> uintptr_t sent_buffer : {
		  buffers_free_release(free_buffers, sent_buffer);
		  sender_active = 0;
		  if (all_buffers_used) {
			c_prod[i] <: buffers_free_acquire(free_buffers);
			all_buffers_used = 0;
		  }
		  break;
		}
		work_pending && !sender_active => default : {
		  // Send a pointer out to the outputter
		  uintptr_t buffer;
		  unsigned length_in_bytes;
		  {buffer, length_in_bytes} = buffers_used_take(used_buffers);
		  master {
			c_con <: buffer;
			c_con <: length_in_bytes;
		  }
		  work_pending--;
		  sender_active = 1;
		  break;
		}
	  }  //select
	}  //for (int i=0; i<num_prod;i++)
  }  //while (1)
}
