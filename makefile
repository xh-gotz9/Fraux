BUILD_DIR = ./build/

CC = gcc

FLAGS = -g

BUILD_CMD = $(CC) $(FLAGS) $? -o $(BUILD_DIR)/$@	

all: buildDir test executable

buildDir:
	mkdir -p $(BUILD_DIR)

executable:
	chmod +x -R $(BUILD_DIR)

clean:
	rm -rf build

bencode.o: bencode.h bencode.c
parser.o: parser.h parser.c bencode.o

test.o: test.c

test: bencode.h bencode.c parser.h parser.c test.c printer.h printer.c
	$(BUILD_CMD) -D_CHECK_DATA

check:
	./$(BUILD_DIR)/test
