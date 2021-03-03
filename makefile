TARGET_EXEC = pv_sense.exe
BUILD_DIR = ./bin
SRC_DIR = ./src
CC = gcc

#VPATH = src

# C Flags
CFLAGS                  = -Wall 
CFLAGS                  += -g

LDFLAGS                 = -l bcm2835 

# define the C source files
SRCS                            = pv_sense.c
SRCS                            += 1wire.c
SRCS                            += ad_i2c.c

DEPS                            = 1wire.h
DEPS                            += ad_i2c.h

OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)/*.o

#-include $(DEPS)

MKDIR_P ?= mkdir -p
