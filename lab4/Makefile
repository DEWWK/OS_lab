hdrs = ipc.h
opts = -g -c
c_src = smoker.c ipc.c
c_obj = smoker.o ipc.o
p_src = provider.c ipc.c
p_obj = provider.o ipc.o
all: provider smoker
smoker: $(c_obj)
	gcc $(c_obj) -o smoker
smoker.o: $(c_src) $(hdrs)
	gcc $(opts) $(c_src)

provider: $(p_obj)
	gcc $(p_obj) -o provider
provider.o: $(p_src) $(hdrs)
	gcc $(opts) $(p_src)
clean:
	rm smoker provider *.o