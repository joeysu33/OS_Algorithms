SRCS = philosopher5.c philosopher5_solve1.c
CFLAGS = -lpthread

all:$(SRCS)
	@for i in $(SRCS); do j=`echo $$i | sed -n 's/\.c//p'`; gcc -o $$j $$i $(CFLAGS); done

clean:$(SRCS)
	@find . -type f -executable -exec rm -f {} \;
