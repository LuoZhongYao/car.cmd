.PHONY : all

CC	:= gcc
CFLAGS := -g -std=gnu99 -DDEBUG #-DDOT

OBJS := main.o cmd.tab.o lex.yy.o ast.o cmd_case.o

YYHDR := cmd.tab.h
HDR  := util.h ast.h

car.cmd : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

cmd.tab.h cmd.tab.c : cmd.y
	bison -d $^ #--debug --verbose

lex.yy.c : cmd.l
	flex $^

$(OBJS) : $(HDR)

%.o : %.c $(YYHDR)
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	-rm -f cmd.tab.c cmd.tab.h lex.yy.c *.o cli_api.[ch] *.output fc.[ch] cmdline.[ch]
