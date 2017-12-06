SRCS = product_consumer_01.c trains.c
CFLAGS = -g -pthread
all:$(SRCS)
	@for i in $(SRCS); do j=`echo $$i | sed -n 's/\.c//p'` ; gcc $(CFLAGS) -o $$j $$i; done


clean:$(SRCS)
	@for i in $(SRCS); do j=`echo $$i | sed -n 's/\.c//p'`; rm -f $$j.o; rm -f $$j; done
