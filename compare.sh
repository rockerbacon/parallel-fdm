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

comp_type=$2

rm -f main-mdf-parallel.c.bin

make
exit_status=$?
if [ "$exit_status" != "0" ]; then
	exit $exit_status
fi

if [ "$comp_type" == "time" ]; then
	time ./mdf $args
fi

time ./mdf-parallel $args $1

echo

./diff

exit $exit_status

