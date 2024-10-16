srcs = mulpipe.c
objs = mulpipe.o
opts = -g -c
all: mulpipe
mulpipe: $(objs)
	gcc $(objs) -o mulpipe
mulpipe.o: $(srcs)
	gcc $(opts) $(srcs)
clean:
	rm mulpipe *.o