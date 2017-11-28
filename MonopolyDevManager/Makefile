TARGET=monopolydevmanager
SRC=$(TARGET).c
OBJ=$(SRC:%.c=%.o)
CFLAGS= -g

$(TARGET):$(OBJ)
	gcc -o $@ $^

$(OBJ):$(SRC)
	gcc $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o $(TARGET)

