#ifndef PROCESS_H
#define PROCESS_H
#include <stdint.h>
#include "config.h"
#include "task.h"

typedef struct process{
	uint16_t id; // the process id
	char filename[VARGOOS_MAX_PATH];
	task_t* task; // the main process task	
	// All the dynamic allocated memory by the process
	void* allocations[VARGOOS_MAX_PROGRAM_ALLOCATIONS];
	void* absolute_ptr; // the physical address pointer to the process memory
	void* stack_ptr; // the absolute address to the stack memory
	uint32_t size; // the size of the data that absolute_ptr is pointing to
	
} process_t;


#endif
