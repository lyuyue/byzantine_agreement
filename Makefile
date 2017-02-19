# complier
CC = gcc

# compiler flags
CFLAGS = -g -Wall

# building target
TARGET = general

# source file
SOURCE = adjusted_general
# clean
RM = rm

all: $(TARGET)
		chmod 777 $(TARGET)

$(TARGET): $(TARGET).c
		$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE).c

clean:
	$(RM) $(TARGET)
