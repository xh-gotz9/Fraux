BUILD_DIR = ./build/
TEST_DIR = ./test/

CC = gcc

DEBUG = -g -DDEBUG

COMPILE_CMD = $(CC) $(DEBUG_HEADER) $? -c 
BUILD_CMD = $(CC) $? -o $(BUILD_DIR)/$@	

all: buildDir executable

buildDir:
	mkdir -p $(BUILD_DIR)

executable:
	chmod +x -R $(BUILD_DIR)

clean:
	rm -rf ./*.o ./*.gch
	rm -rf $(TEST_DIR)/*.o $(TEST_DIR)/*.gch

test.o: test.h test.c
	$(COMPILE_CMD)

test: test.o
	$(BUILD_CMD)

check: buildDir test
	./$(BUILD_DIR)/test
	make clean --no-print-directory