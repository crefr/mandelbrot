FILENAME = mandelbrot
OBJDIR 		   = Obj/
SRCDIR 		   = sources/
HEADDIR 	   = headers/

CC = g++

CFLAGS = -g3 -O3 -I$(HEADDIR) -Wall -Wextra -march=native

c_sources 	= main.cpp mandelbrot.cpp window_handler.cpp test_mandelbrot.cpp
c_src_w_dir = $(addprefix $(SRCDIR), $(c_sources))
headers 	= $(HEADDIR)mandelbrot.h $(HEADDIR)test_mandelbrot.h $(HEADDIR)window_handler.h

C_OBJS = $(addprefix $(OBJDIR), $(c_sources:.cpp=.o))

$(FILENAME): $(C_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lsfml-graphics -lsfml-window -lsfml-system

$(OBJDIR)%.o: $(SRCDIR)%.cpp $(headers)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

dump:
	objdump -d -Mintel $(FILENAME) > $(basename $(FILENAME)).disasm

clean:
	rm $(OBJDIR)*
