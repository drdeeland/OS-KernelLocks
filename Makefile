obj-m += lkm_pa2-in.o
obj-m += lkm_pa2-out.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc test.c -o test

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm test

