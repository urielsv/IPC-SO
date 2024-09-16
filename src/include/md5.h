#ifndef __MD_5_H__
#define __MD_5_H__

#include "shm_manager.h"
#include "slave_manager.h"
#include "utils.h"
#include "defs.h"


int output_from_slaves(slave_t **slaves, uint16_t slave_count, shared_memory_adt shared_memory, int output_file);
uint32_t initial_files_per_slave(uint32_t files, uint32_t slave_count);
uint32_t slave_count(uint32_t files);

#endif // __MD_5_H__
