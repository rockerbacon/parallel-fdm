#!/bin/bash

#Execute aqui pelo script.
#Parâmetros
# ./mdf 100.0 1.0 1.0 1.0 100.0 0
#        |     |   |   |    |   |-> sava log (1 salva/0 não salva)
#        |     |   |   |    |-----> Temperatura da borda
#        |     |   |   |----------> Tamanho em metros do eixo Z
#        |     |   |--------------> Tamanho em metros do eixo Y
#        |     |------------------> Tamanho em metros do eixo X
#        |------------------------> tempo em segundos
args="10.0 42 42 42 100 1"

make
exit_status=$?
if [ "$exit_status" != "0" ]; then
	exit $exit_status
fi

# time ./mdf $args
time ./mdf-parallel $args $1

echo

cmp reference.bin main-mdf-parallel.c.bin
exit_status=$?

if [ "$exit_status" == "0" ]; then
	echo "PROGRAM WORKING CORRECTLY"
else
	echo "FILES ARE DIFFERENT!!!!!!"
fi

exit $exit_status

