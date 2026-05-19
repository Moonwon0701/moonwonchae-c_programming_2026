CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
LIBS    = -lSDL2 -lGLEW -lGL -lm

SRCS    = main.c stroke.c render.c eraser.c fileio.c shape.c undo.c camera.c
OBJS    = $(SRCS:.c=.o)
TARGET  = vector_painter

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
