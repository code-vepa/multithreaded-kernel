FILES = ./build/kernel.asm.o ./build/kernel.o ./build/build_idt/idt.asm.o ./build/build_idt/idt.o ./build/build_memory/memory.o ./build/build_io/io.asm.o ./build/build_memory/heap/heap.o ./build/build_memory/heap/kernel_heap.o ./build/build_memory/paging/paging.o ./build/build_memory/paging/paging.asm.o ./build/build_disk/disk.o ./build/build_disk/disk_streamer.o ./build/build_filesystem/path_parser.o ./build/build_filesystem/file.o ./build/build_string/string.o ./build/build_filesystem/build_fat/fat16.o ./build/build_gdt/gdt.o ./build/build_gdt/gdt.asm.o ./build/build_task/tss.o ./build/build_task/tss.asm.o
INCLUDES = -I ./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-nounsed-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/boot.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin
	#~/opt/mtools/bin/mcopy -o -i ./bin/os.bin hello.txt ::/	
	

./bin/kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o


./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o
 
./build/kernel.o: ./src/kernel.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o

./build/build_idt/idt.asm.o: ./src/idt/idt.asm
	nasm -f elf -g ./src/idt/idt.asm -o ./build/build_idt/idt.asm.o

./build/build_idt/idt.o: ./src/idt/idt.c
	i686-elf-gcc $(INCLUDES) -I ./src/idt $(FLAGS) -std=gnu99 -c ./src/idt/idt.c -o ./build/build_idt/idt.o	

./build/build_memory/memory.o: ./src/memory/memory.c
	i686-elf-gcc $(INCLUDES) -I ./src/memory $(FLAGS) -std=gnu99 -c ./src/memory/memory.c -o ./build/build_memory/memory.o

./build/build_io/io.asm.o: ./src/io/io.asm
	nasm -f elf -g ./src/io/io.asm -o ./build/build_io/io.asm.o

./build/build_memory/heap/heap.o: ./src/memory/heap/heap.c
	i686-elf-gcc $(INCLUDES) -I ./src/memory/heap $(FLAGS) -std=gnu99 -c ./src/memory/heap/heap.c -o ./build/build_memory/heap/heap.o

./build/build_memory/heap/kernel_heap.o: ./src/memory/heap/kernel_heap.c
	i686-elf-gcc $(INCLUDES) -I ./src/memory/heap $(FLAGS) -std=gnu99 -c ./src/memory/heap/kernel_heap.c -o ./build/build_memory/heap/kernel_heap.o

./build/build_memory/paging/paging.o: ./src/memory/paging/paging.c
	i686-elf-gcc $(INCLUDES) -I ./src/memory/paging $(FLAGS) -std=gnu99 -c ./src/memory/paging/paging.c -o ./build/build_memory/paging/paging.o

./build/build_memory/paging/paging.asm.o: ./src/memory/paging/paging.asm
	nasm -f elf -g ./src/memory/paging/paging.asm -o ./build/build_memory/paging/paging.asm.o

./build/build_disk/disk.o: ./src/disk/disk.c
	i686-elf-gcc $(INCLUDES) -I ./src/disk $(FLAGS) -std=gnu99 -c ./src/disk/disk.c -o ./build/build_disk/disk.o

./build/build_string/string.o: ./src/string/string.c
	i686-elf-gcc $(INCLUDES) -I ./src/string $(FLAGS) -std=gnu99 -c ./src/string/string.c -o ./build/build_string/string.o

./build/build_filesystem/path_parser.o: ./src/filesystem/path_parser.c
	i686-elf-gcc $(INCLUDES) -I ./src/filesystem $(FLAGS) -std=gnu99 -c ./src/filesystem/path_parser.c -o ./build/build_filesystem/path_parser.o

./build/build_filesystem/file.o: ./src/filesystem/file.c
	i686-elf-gcc $(INCLUDES) -I ./src/filesystem $(FLAGS) -std=gnu99 -c ./src/filesystem/file.c -o ./build/build_filesystem/file.o

./build/build_disk/disk_streamer.o: ./src/disk/disk_streamer.c
	i686-elf-gcc $(INCLUDES) -I ./src/disk $(FLAGS) -std=gnu99 -c ./src/disk/disk_streamer.c -o ./build/build_disk/disk_streamer.o

./build/build_filesystem/build_fat/fat16.o: ./src/filesystem/fat/fat16.c
	i686-elf-gcc $(INCLUDES) -I ./src/filesystem/fat $(FLAGS) -std=gnu99 -c ./src/filesystem/fat/fat16.c -o ./build/build_filesystem/build_fat/fat16.o

./build/build_gdt/gdt.asm.o: ./src/gdt/gdt.asm
	nasm -f elf -g ./src/gdt/gdt.asm -o ./build/build_gdt/gdt.asm.o

./build/build_gdt/gdt.o: ./src/gdt/gdt.c
	i686-elf-gcc $(INCLUDES) -I ./src/gdt $(FLAGS) -std=gnu99 -c ./src/gdt/gdt.c -o ./build/build_gdt/gdt.o

./build/build_task/tss.asm.o: ./src/task/tss.asm
	nasm -f elf -g ./src/task/tss.asm -o ./build/build_task/tss.asm.o

./build/build_task/tss.o: ./src/task/tss.c
	i686-elf-gcc $(INCLUDES) -I ./src/task $(FLAGS) -std=gnu99 -c ./src/task/tss.c -o ./build/build_task/tss.o

clean:
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
	rm -rf ./bin/os.bin
	rm -rf ${FILES}
	rm -rf ./build/kernelfull.o