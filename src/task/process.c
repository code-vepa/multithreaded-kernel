#include "process.h"
#include "memory/memory.h"
#include "status.h"
#include "memory/heap/kernel_heap.h"
#include "filesystem/file.h"
#include "string/string.h"
#include "kernel.h"
#include "memory/paging/paging.h"
#include "loader/formats/elfloader.h"
#include "stdbool.h"

process_t* current_process = 0; // current running process
static process_t* processes[VARGOOS_MAX_PROCESSES] = {};

static void process_init(process_t* process){
	memset(process, 0, sizeof(process_t));
}

process_t* get_current_process(){
	return current_process;
}

process_t* get_process(int index){
	if(index < 0 || index >= VARGOOS_MAX_PROCESSES){
		return 0x00;
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

	process->filetype = PROCESS_FILETYPE_BINARY;
	process->absolute_ptr = program_data;
	process->size = stat.filesize;
	

out:
	fclose(fd);
	return response;
}

static int process_load_elf(const char * filename, process_t* process){
	int response = 0;
	struct elf_file*  elf_file = 0;
	
	response = elf_load(filename, &elf_file);
	if(response < 0){
		goto out;
	}

	process->filetype = PROCESS_FILETYPE_ELF;
	process->elf_file = elf_file;
out:
	return response;
}

static int process_load_data(const char* filename, process_t* process){
	int response = 0;
	response = process_load_elf(filename, process);
	if(response == -EINVFORMAT){
		response = process_load_binary(filename, process);
	}
	return response;
}

int process_map_binary(process_t* process){
	int response = 0;
	paging_map_to(process->task->page_directory,
		(void*) VARGOOS_PROGRAM_VIRTUAL_ADDRESS, 
		process->absolute_ptr, 
		paging_align_address(process->absolute_ptr + process->size), 
		PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITABLE);

	return response;
}

int process_map_elf(process_t* process){
	int response = 0;
	struct elf_file* elf_file = process->elf_file;
	struct elf_header* header = elf_header(elf_file);
	struct elf32_phdr* phdrs = elf_pheader(header);

	for(int i = 0; i < header->e_phnum; ++i){
		struct elf32_phdr* phdr = &phdrs[i];
		void* phdr_phys_addr = elf_phdr_phys_address(elf_file, phdr);
		int flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;
		if(phdr->p_flags & PF_W){
			flags |= PAGING_IS_WRITABLE;
		}
		
		response = paging_map_to(process->task->page_directory, paging_align_to_lower_page((void*)phdr->p_vaddr),
			paging_align_to_lower_page(phdr_phys_addr), paging_align_address(phdr_phys_addr +phdr->p_memsz),
			flags);
	
		if(ISERR(response)){
			break;
		}
	}

	return response;
}

int process_map_memory(process_t* process){
	int response = 0;
	switch(process->filetype){
		case PROCESS_FILETYPE_ELF:
			response = process_map_elf(process);
			break;
		
		case PROCESS_FILETYPE_BINARY:
			response = process_map_binary(process);
			break;

		default:
			panic("PANIC: invalid filetype\n");
	}

	if(response < 0){
		goto out;
	}
	paging_map_to(process->task->page_directory, 
		(void*) VARGOOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END, process->stack_ptr, paging_align_address(process->stack_ptr + VARGOOS_USER_PROG_STACK_SIZE),
		PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITABLE);

out:
	return response;
}

int get_free_slot(){
	for(int i = 0; i < VARGOOS_MAX_PROCESSES; ++i){
		if(processes[i] == 0){
			return i;
		}
	}
	return -EISTKN;
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
		goto out;
	}

	_process->task = task;

	//map the memory (to page size)

	response = process_map_memory(_process);
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

int process_load(const char* filename, process_t** process){
	int response = 0;
	int process_slot = get_free_slot();
	if(process_slot < 0){
		response = -EISTKN;
		goto out;
	}

	response = process_load_for_slot(filename, process, process_slot);
out:
	return response;
}

int process_switch(process_t* process){
	current_process = process;
	return 0;
}

int process_load_switch(const char* filename, process_t** process){
	int res = process_load(filename, process);
	if(!res){
		process_switch(*process);
	}
	return res;
}

static int process_free_alloc_index(process_t* process){
	int res = -ENOMEM;
	for(int i = 0; i < VARGOOS_MAX_PROGRAM_ALLOCATIONS; ++i){
		if(process->allocations[i] == 0){
			res = i;
			break;
		}
	}

	return res;
}

void* process_malloc(process_t* process, size_t size){
	void* ptr = kzalloc(size);
	if(ptr == 0){
		return 0; // Failed to allocate
	}

	int index = process_free_alloc_index(process);
	if(index < 0){
		return 0; //No space for index storage
	}

	process->allocations[index] = ptr;
	return ptr;
}

static bool is_process_pointer(process_t* process, void* ptr){
	for(int i = 0; i < VARGOOS_MAX_PROGRAM_ALLOCATIONS; ++i){
		if(process->allocations[i] == ptr){
			return true;
		}
	}

	return false;
}

static void mark_free_allocation(process_t* process, void* ptr){
	for(int i = 0; i < VARGOOS_MAX_PROGRAM_ALLOCATIONS; ++i){
		if(process->allocations[i] == ptr){
			process->allocations[i] = 0x00;
			return;
		}
	}
}

void process_free(process_t* process, void* ptr){
	if(!is_process_pointer(process, ptr)){
		return;
	}

	mark_free_allocation(process, ptr);
	kfree(ptr);
}