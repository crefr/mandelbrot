FILENAME = mandlebrot
OBJDIR 		   = Obj/
SRCDIR 		   = sources/
HEADDIR 	   = headers/

CC = g++

c_sources 	= main.c
c_src_w_dir = $(addprefix $(SRCDIR), $(c_sources))
headers 	=

C_OBJS   = $(addprefix $(OBJDIR), $(addsuffix .o, $(basename $(c_sources))))

$(FILENAME): $(ASM_OBJS) $(C_OBJS)
	gcc -no-pie -o $@ $^

$(C_OBJS):	$(c_src_w_dir) $(headers)
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -g3 -Og -c $< -o $@

dump:
	objdump -d -Mintel $(FILENAME) > $(basename $(FILENAME)).disasm

clean:
	rm $(OBJDIR)*
