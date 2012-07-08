#!/bin/bash
# Couldn't be bothered to make a proper make-file...

for i in *.c
do 
	gcc -c $i
done

gcc read_hm-trp.o hm-trp.o serial.o -o read_hm-trp
gcc set_speed.o hm-trp.o serial.o -o set_speed

