SRCS = $(shell ls *.c)
all:$(SRCS)
	@for i in $(SRCS) ; do j=`echo $$i | sed -n 's/\.c//p'`; gcc -g -o $$j $$i -pthread; done

clean: 
	find . -type f -executable -exec rm -f {} \;
