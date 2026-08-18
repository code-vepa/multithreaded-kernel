#ifndef PROCESS_H
#define PROCESS_H
#include <stdint.h>
#include "config.h"
#include "task.h"

typedef struct keyboard_buffer{
	char buffer[VARGOOS_MAX_KEYBOARD_BUFFER_SIZE];
	int head;
	int tail;
} keyboard_t;


typedef struct process{
	uint16_t id; // the process id
	char filename[VARGOOS_MAX_PATH];
	task_t* task; // the main process task	
	// All the dynamic allocated memory by the process
	void* allocations[VARGOOS_MAX_PROGRAM_ALLOCATIONS];
	void* absolute_ptr; // the physical address pointer to the process memory
	void* stack_ptr; // the absolute address to the stack memory
	uint32_t size; // the size of the data that absolute_ptr is pointing to
	keyboard_t keyboard;
} process_t;

int process_load(const char* filename, process_t** process);
process_t* get_current_process();
int process_load_switch(const char* filename, process_t** process);
int process_switch(process_t* process);

#endif
