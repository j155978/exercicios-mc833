#!/bin/bash
if [ -z "$1" ]; then
    echo "Uso: $0 <porta>"
    exit 1
fi

# Executa 15 instâncias do cliente com o parâmetro 495
for i in {1..15}
do
    ./cliente 0.0.0.0 "$1" "$i" &
done

# Opcional: espera que todos os processos filhos terminem
wait

echo "Todos Clientes encerrados"