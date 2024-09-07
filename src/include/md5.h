#ifndef __MD_5_H__
#define __MD_5_H__

#include "slave_manager.h"
#include "shm_utils.h"

int output_from_slaves(slave_t **slaves, uint16_t slave_count, shared_memory_t * shared_memory);

#endif // __MD_5_H__
