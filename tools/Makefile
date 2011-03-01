CC = gcc
CFLAGS = -W -Wall -g -O0 `sdl-config --cflags`
LIBS = `sdl-config --libs`

eXe = testsdl

OBJS = testsdl.o ../sdl_rotozoom.o

$(eXe): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(eXe) *.o *~
