< Avaliador e Conversor de Expressões – Pós-Fixa ↔ Infixa >

Este projeto implementa uma biblioteca em C para converter expressões matemáticas entre notação infixa e pós-fixa, além de avaliar expressões pós-fixas, incluindo funções trigonométricas e logarítmicas.

O trabalho foi desenvolvido seguindo estritamente as especificações fornecidas pelo professor.

-> Funcionalidades implementadas
1. Converter expressão infixa para pós-fixa

Usa o algoritmo Shunting Yard (Dijkstra).

Trata corretamente precedência e associatividade.

Suporta funções unárias: raiz, sen, cos, tg, log.

Coloca operadores binários respeitando a ordem correta.

Corrige automaticamente casos com sinal unário (-x → 0 x -).

2. Converter expressão pós-fixa para infixa

Reconstrói a árvore sintática a partir dos tokens.

Remove parênteses redundantes.

Mantém somente os parênteses necessários.

Reinsere funções unárias corretamente.

3. Avaliar expressão pós-fixa

Implementa corretamente:

Soma +

Subtração -

Multiplicação *

Divisão /

Módulo %

Exponenciação ^

Funções:

raiz(x)

sen(x) (graus)

cos(x) (graus)

tg(x) (graus)

log(x) (log10)

--> Exemplos de Entrada/Saída
Infixa → Pós-Fixa

Entrada:

3+2*5


Saída:

3 2 5 * +
Valor: 13

Pós-Fixa → Infixa

Entrada:

3 2 5 * +


Saída:

3+2*5
Valor: 13

-> main.c não deve ser enviado, pois o professor solicitou apenas a biblioteca.

* Compilação e uso (para testes locais)

Se quiser testar com um main próprio:

gcc main.c expressao.c -o app -lm
./app

-> Observações Importantes

O arquivo expressao.h foi mantido exatamente como fornecido.

Toda a lógica extra necessária (árvores, heurísticas de ordenação, pilhas, tokenização) está dentro de expressao.c.

O código trata erros sintáticos e retorna NULL ou 0.0 dependendo da função.

As funções seguem rigorosamente os protótipos exigidos.

<-- Autor -->

Trabalho desenvolvido por:

Pedro Henrique Gerheim
