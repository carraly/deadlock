# deadlock

Programa em C que implementa o algoritmo de Banker para evitar deadlocks na alocação de recursos. O número de clientes e de tipos de recurso é dinâmico: os recursos vêm dos argumentos da linha de comando, os clientes de customer.txt e os comandos RQ, RL e * de commands.txt. A saída é gravada em result.txt.

## Compilação

make

O Makefile gera o executável banker.

## Execução

./banker 10 5 7

Cada argumento representa o número de instâncias de um tipo de recurso. O programa lê customer.txt e commands.txt do diretório atual e gera result.txt.

## Limpeza

make clean

## Estratégia do algoritmo

- **Estado**: maximum, allocation, need e available são mantidos como vetores/matrizes de tamanho dinâmico, alocados após a leitura dos argumentos e de customer.txt.
- **Requisição (RQ)**: a requisição é negada se exceder need ou available. Caso contrário, os recursos são alocados temporariamente e o estado é testado; se ficar inseguro, a alocação é desfeita.
- **Liberação (RL)**: a liberação é negada se exceder allocation; caso contrário, available aumenta, allocation diminui e need aumenta na mesma proporção.
- **Verificação de segurança**: o algoritmo de Work/Finish simula a finalização dos clientes cuja need cabe em work, somando allocation a work. O estado é seguro se todos os clientes terminarem.
- **Comando asterisco**: imprime MAXIMUM, ALLOCATION, NEED e AVAILABLE no estado atual.
- **Validação**: customer.txt e commands.txt são completamente validados antes de qualquer processamento. Em qualquer erro de validação, o programa imprime uma mensagem banker: ... em stderr, encerra com código de falha e não cria result.txt.

## Estrutura do código

- **banker.c**: implementação do algoritmo, leitura e validação das entradas, execução dos comandos, verificação de segurança e geração de result.txt.
- **Makefile**: regras de compilação e limpeza.

## Ambiente

Implementado e validado em Linux (CachyOS) com gcc 16.2.1 e GNU make 4.4.1. O programa usa apenas a biblioteca padrão do C.
