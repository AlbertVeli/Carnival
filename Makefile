CC = gcc
CFLAGS = -g -O0 -W -Wall `sdl-config --cflags`
LIBS = `sdl-config --libs` -lpng

eXe = carnival

OBJS = carnival.o level.o sdl_video.o sdl_sprite.o sdl_cursor.o sdl_event.o sdl_rotozoom.o trickmath.o

$(eXe): $(OBJS)
	$(CC) -pg -o $@ $(OBJS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(eXe) *.o *~ gmon.out
