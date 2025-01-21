
OBJS	= htoa.o memcpy.o memset.o strnlen.o vsprintf.o sprintf.o printf.o

all: minilib.a

minilib.a: $(OBJS)
	$(AR) cru $@ $^
	$(RANLIB) $@

clean:
	rm -rf *.a *.bak $(OBJS)
