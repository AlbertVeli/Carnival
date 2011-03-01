CC       = gcc
CFLAGS   = -W -Wall -Werror `sdl-config --cflags`
LIBS     = `sdl-config --libs` -lpng

# Run:
#     DEBUG=1 make
# to compile with debug info
ifdef DEBUG
CFLAGS += -ggdb -O0
else
CFLAGS += -O2
endif

# Uncomment the following line if using MacPorts
#CFLAGS += -I/opt/local/include

eXe = carnival

OBJS = carnival.o level.o sdl_video.o sdl_sprite.o sdl_cursor.o sdl_event.o sdl_rotozoom.o trickmath.o

$(eXe): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(eXe) *.o *~ gmon.out
