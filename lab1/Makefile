head = pctl.h
srcs = multip.c
objs = multip.o
opts = -g -c
all: multip
multip: $(objs)
	gcc $(objs) -o multip
multip.o: $(srcs) $(head)
	gcc $(opts) $(srcs)
clean:
	rm multip *.o