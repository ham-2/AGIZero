CC = gcc
CXXFLAGS = -march=native -pthread -Wall -O3
OBJ_DIR = ./obj
TARGET = AGIZero

ifeq ($(OS),Windows_NT)
	CFLAGS += -D WINDOWS
	TARGET = AGIZero.exe
endif

SRCS = $(notdir $(wildcard *.cpp))
OBJS = $(patsubst %.o,$(OBJ_DIR)/%.o,$(SRCS:.cpp=.o))
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

.PHONY: clean all

$(OBJ_DIR)/%.o : $(SRCS)
	$(CC) $(CXXFLAGS) -c $< -o $@ -MD $(LDFLAGS)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
