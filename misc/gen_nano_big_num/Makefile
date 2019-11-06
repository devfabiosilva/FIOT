#
#	AUTHOR: Fábio Pereira da Silva
#	YEAR: 2019
#	LICENSE: MIT
#	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
#


#Qui 04 Jul 2019 00:09:11 -03 
# Modificado: Sáb 12 Out 2019 22:20:17 -03 
CC=gcc
STRIP=strip
FILENAME=nano_gen_bnle
INCLUDEDIR=$(PWD)/include/

include ./fproject.mk

OBJEXT ?= .o

COBJS = $(CSRCS:.c=$(OBJEXT))

all: $(COBJS) main

%.o: %.c
	@$(CC) -I$(INCLUDEDIR) -c $< -o $@ -Os
	@echo "CC $<"

main:
	@echo "Creating $(FILENAME)"
	$(CC) -o $(FILENAME) $(FILENAME).c $(COBJS) -I$(INCLUDEDIR) -Os
	$(STRIP) $(FILENAME)

.PHONY: clean
clean:
	@echo "Removing program objs and $(FILENAME)..."
	rm -v nanobgle.dat
	rm -v $(FILENAME) $(COBJS)

