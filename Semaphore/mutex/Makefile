SRCS = mutex1.c
all:$(SRCS)
	@for i in $(SRCS); do j=`echo $$i | sed -n 's/\.c//p'`; gcc -g -o $$j $$i -lpthread;done

clean:
	find . -type f -executable -exec rm -f {} \;
