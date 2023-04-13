This project consists of two kernel modules that read and write to a buffer
in shared memory. It comes with pa2_in.c, pa2_out.c, and a Makefile. To make
the kernel modules, first run:

make

In the terminal at the location where the files are located. Then, run:

sudo insmod pa2_in.ko
sudo insmod pa2_out.ko

In the given order. Run 'sudo dmesg' and it should say that it was registered
and created correctly. You can also run 'make clean' to clean up the folder after
the kernel modules are installed. To remove the two kernel modules, run:

sudo rmmod pa2_out
sudo rmmod pa2_in

Note that they MUST be removed in that given order.