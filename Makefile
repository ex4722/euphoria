# Makefile for building the main.c and utils.c kernel module

# Name of the module
obj-m := fuck.o

# List of source files
fuck-objs :=  euphoria.o utils.o

# Path to the Linux kernel source tree
KDIR := /home/cosmo/workspace/perfectclear/linux

# Current directory
PWD := $(shell pwd)

# Default target
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean the build
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
