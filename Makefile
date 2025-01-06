TARGET = prismarine

CC = gcc
CFLAGS = -Wall -g -c
LDLIBS  = -pthread

# Find all .c files recursively
SRC = $(shell find . -name '*.c')

OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -f $(OBJ) $(TARGET)

test: $(TARGET) 
	valgrind --leak-check=full --track-origins=yes ./$(TARGET) ../Momento/src/main/java/org/momento
