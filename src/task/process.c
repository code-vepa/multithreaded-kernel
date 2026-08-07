#include "process.h"
#include "memory/memory.h"
#include "status.h"
#include "memory/heap/kernel_heap.h"
#include "filesystem/file.h"
#include "string/string.h"
#include "kernel.h"
#include "memory/paging/paging.h"

process_t* current = 0; // current running process
static process_t* processes[VARGOOS_MAX_PROCESSES] = {};

static void process_init(process_t* process){
	memset(process, 0, sizeof(process_t));
}

process_t* get_current_process(){
	return current;
}

int get_process(int index){
	if(index < 0 || index >= VARGOOS_MAX_PROCESSES){
		return -EINVARG;
	}

	return processes[index];
}

static int process_load_binary(const char* filename, process_t* process){
	int response = 0;
	int fd = fopen(filename, "r");
	if(!fd){
		response = -EIO;
		goto out;
	}

	struct file_stat stat;
	response = fstat(fd, &stat);
	if(response != STATUS_OK){
		goto out;
	}

	//Allocate memory for the file
	void* program_data = kzalloc(stat.filesize);
	if(!program_data){
		response = -ENOMEM;
		goto out;
	}

	if(fread(program_data, stat.filesize, 1, fd) != 1){
		response = -EIO;
		goto out;
	}

	process->absolute_ptr = program_data;
	process->size = stat.filesize;
	

out:
	fclose(fd);
	return response;
}

static int process_load_data(const char* filename, process_t* process){
	int response = 0;
	response = process_load_binary(filename, process);
	return response;
}

int process_map_binary(process_t* process){
	int response = 0;
	paging_map_to(process->task->page_directory->directory_entry, 
		(void*) VARGOOS_PROGRAM_VIRTUAL_ADDRESS, 
		process->absolute_ptr, 
		paging_align_address(process->absolute_ptr + process->size), 
		PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITABLE);
	return response;
}

int process_map_memory(process_t* process){
	int response = 0;
	response = process_map_binary(process);
	return response;
}

int process_load_for_slot(const char* filename, process_t** process, int process_slot){
	int response = 0;
	task_t* task = 0;
	process_t* _process;
	void* program_stack = 0;

	if(get_process(process_slot) != 0){
		// can't load a process into a slot that already has a loaded process
		response = -EISTKN;
		goto out;
	}

	_process = kzalloc(sizeof(process_t));
	if(!_process){
		response = -ENOMEM;
		goto out;
	}

	process_init(_process);

	response = process_load_data(filename, _process);
	if(response < 0){
		goto out;
	}

	program_stack = kzalloc(VARGOOS_USER_PROG_STACK_SIZE);
	if(!program_stack){
		response = -ENOMEM;
		goto out;
	}
	
	strncpy(_process->filename, filename, sizeof(_process->filename));
	_process->stack_ptr = program_stack;
	_process->id = process_slot;

	task = new_task(_process);
	if(ERROR_I(task) == 0){
		response = ERROR_I(task);
	}

	_process->task = task;

	//map the memory (to page size)

	response = process_map_memory(process);
	if(response < 0){
		goto out;
	}

	*process = _process;
	processes[process_slot] = _process;

out:
	if(ISERR(response)){
		if(_process && _process->task){
			task_free(_process->task);
		}

		//impl. process_free
	}
	return response;
}
