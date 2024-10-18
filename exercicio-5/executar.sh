#!/bin/bash
if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Uso: $0 <porta> <número_de_conexões>"
    exit 1
fi

PORT=$1
CONNECTIONS=$2

# Executa o número especificado de instâncias do cliente
for i in $(seq 1 $CONNECTIONS)
do
    ./cliente 0.0.0.0 "$PORT" "$i" &
done

# Opcional: espera que todos os processos filhos terminem
wait

echo "Todos Clientes encerrados"
