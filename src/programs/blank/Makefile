FILES=./build/blank.o 
INCLUDES=-I../stdlib/src 
FLAGS= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0  
all: ${FILES}
	i686-elf-gcc -g -T ./linker.ld -o ./bin/blank.elf -ffreestanding -O0 -nostdlib -fpic -g ${FILES} ../stdlib/bin/stdlib.elf

./build/blank.o: ./src/blank.c
	i686-elf-gcc  $(INCLUDES) ${FLAGS} -std=gnu99 -c ./src/blank.c -o ./build/blank.o

	
 

clean:
	rm -rf ${FILES}
	rm -rf ./bin/blank.elf