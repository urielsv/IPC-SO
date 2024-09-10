#ifndef __MD_5_H__
#define __MD_5_H__

#include "slave_manager.h"
#include "shm_manager.h"

uint32_t initial_files_per_slave(uint32_t files, uint32_t slave_count);
uint32_t slave_count(uint32_t files);
int output_from_slaves(slave_t **slaves, uint16_t slave_count, shared_memory_adt shared_memory);

#endif // __MD_5_H__
