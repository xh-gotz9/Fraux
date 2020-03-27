BUILD_DIR = ./build/

CC = gcc

FLAGS = -g

BUILD_CMD = $(CC) $(FLAGS) $? -o $(BUILD_DIR)/$@	

all: buildDir goto_build_dir test executable clean

buildDir:
	mkdir -p $(BUILD_DIR)

executable:
	chmod +x -R $(BUILD_DIR)

goto_build_dir:
	cd $(BUILD_DIR)

clean:
	rm *.o

bencoding.o: bencoding.h bencoding.c
parser.o: parser.h parser.c bencoding.o

test.o: test.c

test: bencoding.h bencoding.c parser.h parser.c test.c printer.h printer.c
	$(BUILD_CMD) -D_CHECK_DATA