# the compiler: gcc for C program, define as g++ for C++
CC = gcc
# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall
# -pthread support for threads
LDFLAGS = -lpthread
# the build target executable:
TARGET = deadlock_free

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LDFLAGS)
clean:
	$(RM) $(TARGET)
