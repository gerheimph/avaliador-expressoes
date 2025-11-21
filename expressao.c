#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "expressao.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAXTOK 512
#define MAXSTR 512

// Pilha para floats (avaliação)
typedef struct
{
    float dados[1024];
    int topo;
} PilhaFloat;

// Pilha para strings (conversão)
typedef struct
{
    char dados[1024][MAXSTR];
    int topo;
} PilhaStr;

// Funções de pilha float
static void initPilhaFloat(PilhaFloat *p) { p->topo = -1; }
static int vaziaFloat(PilhaFloat *p) { return p->topo < 0; }
static int pushFloat(PilhaFloat *p, float v)
{
    if (p->topo >= 1023)
        return 0;
    p->dados[++p->topo] = v;
    return 1;
}
static float popFloat(PilhaFloat *p)
{
    if (vaziaFloat(p))
        return 0.0f;
    return p->dados[p->topo--];
}

// Funções de pilha string
static void initPilhaStr(PilhaStr *p) { p->topo = -1; }
static int vaziaStr(PilhaStr *p) { return p->topo < 0; }
static int pushStr(PilhaStr *p, const char *s)
{
    if (p->topo >= 1023)
        return 0;
    strncpy(p->dados[++p->topo], s, MAXSTR - 1);
    p->dados[p->topo][MAXSTR - 1] = '\0';
    return 1;
}
static char *popStrPtr(PilhaStr *p)
{
    if (vaziaStr(p))
        return NULL;
    return p->dados[p->topo--];
}

// Conversão Infixa - Pós-fixa

static int isDigitChar(char c)
{
    return (c >= '0' && c <= '9') || c == '.';
}

static int isAlphaChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int isOperatorChar(char c)
{
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^');
}

static int precedenceOp(const char *op)
{
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0)
        return 2;
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, "%") == 0)
        return 3;
    if (strcmp(op, "^") == 0)
        return 4;
    return 0;
}

static int isRightAssociative(const char *op)
{
    return (strcmp(op, "^") == 0);
}

// retorna 1 se token for função - raiz, sen, cos, tg, log
static int isFuncToken(const char *tok)
{
    return (strcmp(tok, "raiz") == 0 || strcmp(tok, "sen") == 0 ||
            strcmp(tok, "cos") == 0 || strcmp(tok, "tg") == 0 ||
            strcmp(tok, "log") == 0);
}

// Função principal: converte infixa para posfixa
// Retorna ponteiro estático para string (tokens separados por espaço).
char *getFormaPosFixa(char *StrInFixa)
{
    static char resultado[MAXSTR];
    if (StrInFixa == NULL)
        return NULL;

    // stacks (simples)
    char opStack[1024][MAXSTR];
    int opTop = -1;

    char outTokens[2048][MAXSTR];
    int outTop = -1;

    size_t n = strlen(StrInFixa);
    size_t i = 0;
    int expectOperand = 1; // para detectar '-' unário: no começo ou após '(' ou operador

    while (i < n)
    {
        // pular espaços
        if (StrInFixa[i] == ' ' || StrInFixa[i] == '\t')
        {
            i++;
            continue;
        }

        // número (inteiro ou float)
        if (isDigitChar(StrInFixa[i]))
        {
            char num[MAXSTR];
            int p = 0;
            while (i < n && isDigitChar(StrInFixa[i]) && p < MAXSTR - 1)
            {
                num[p++] = StrInFixa[i++];
            }
            num[p] = '\0';
            strncpy(outTokens[++outTop], num, MAXSTR - 1);
            outTokens[outTop][MAXSTR - 1] = '\0';
            expectOperand = 0;
            continue;
        }

        // função ou nome (alfabético)
        if (isAlphaChar(StrInFixa[i]))
        {
            char name[MAXSTR];
            int p = 0;
            while (i < n && isAlphaChar(StrInFixa[i]) && p < MAXSTR - 1)
            {
                name[p++] = StrInFixa[i++];
            }
            name[p] = '\0';

            strncpy(opStack[++opTop], name, MAXSTR - 1);
            opStack[opTop][MAXSTR - 1] = '\0';
            expectOperand = 1; // depois da função espera-se '(' ou operand
            continue;
        }

        // parênteses esquerdo
        if (StrInFixa[i] == '(')
        {
            strncpy(opStack[++opTop], "(", MAXSTR - 1);
            opStack[opTop][MAXSTR - 1] = '\0';
            i++;
            expectOperand = 1;
            continue;
        }

        // parênteses direito
        if (StrInFixa[i] == ')')
        {
            int found = 0;
            while (opTop >= 0)
            {
                if (strcmp(opStack[opTop], "(") == 0)
                {
                    found = 1;
                    opTop--;
                    break;
                }
                strncpy(outTokens[++outTop], opStack[opTop], MAXSTR - 1);
                outTokens[outTop][MAXSTR - 1] = '\0';
                opTop--;
            }
            if (!found)
                return NULL; // parêntese mismatch
            if (opTop >= 0 && isFuncToken(opStack[opTop]))
            {
                strncpy(outTokens[++outTop], opStack[opTop], MAXSTR - 1);
                outTokens[outTop][MAXSTR - 1] = '\0';
                opTop--;
            }
            i++;
            expectOperand = 0;
            continue;
        }

        // operador (+ - * / % ^)
        if (isOperatorChar(StrInFixa[i]))
        {
            char op[4] = {StrInFixa[i], '\0', '\0', '\0'};
            // tratar '-' unário: se estiver esperando operando, e for '-', considerar '0' antes
            if (op[0] == '-' && expectOperand)
            {
                // inserir 0 no output antes de processar o '-' como binário
                strncpy(outTokens[++outTop], "0", MAXSTR - 1);
                outTokens[outTop][MAXSTR - 1] = '\0';
            }
            while (opTop >= 0)
            {
                const char *topOp = opStack[opTop];
                if (strcmp(topOp, "(") == 0)
                    break;
                if (isFuncToken(topOp))
                {
                    strncpy(outTokens[++outTop], topOp, MAXSTR - 1);
                    outTokens[outTop][MAXSTR - 1] = '\0';
                    opTop--;
                    continue;
                }
                int pTop = precedenceOp(topOp);
                int pCur = precedenceOp(op);
                if ((!isRightAssociative(op) && pTop >= pCur) ||
                    (isRightAssociative(op) && pTop > pCur))
                {
                    strncpy(outTokens[++outTop], topOp, MAXSTR - 1);
                    outTokens[outTop][MAXSTR - 1] = '\0';
                    opTop--;
                }
                else
                    break;
            }
            strncpy(opStack[++opTop], op, MAXSTR - 1);
            opStack[opTop][MAXSTR - 1] = '\0';
            i++;
            expectOperand = 1;
            continue;
        }

        // caractere desconhecido
        return NULL;
    } // fim do while scan

    // no final, pop tudo da pilha para output
    while (opTop >= 0)
    {
        if (strcmp(opStack[opTop], "(") == 0 || strcmp(opStack[opTop], ")") == 0)
        {
            return NULL;
        }
        strncpy(outTokens[++outTop], opStack[opTop], MAXSTR - 1);
        outTokens[outTop][MAXSTR - 1] = '\0';
        opTop--;
    }

    // constroi string resultado com espaços
    resultado[0] = '\0';
    size_t pos = 0;
    for (int k = 0; k <= outTop; ++k)
    {
        size_t len = strlen(outTokens[k]);
        if (pos + len + 2 >= MAXSTR)
            return NULL;
        if (k > 0)
        {
            resultado[pos++] = ' ';
        }
        strncpy(resultado + pos, outTokens[k], len);
        pos += len;
        resultado[pos] = '\0';
    }

    return resultado;
}

// utilidade - verifica se token é operador binário
static int isOperadorBin(const char *tok)
{
    return (strcmp(tok, "+") == 0 || strcmp(tok, "-") == 0 ||
            strcmp(tok, "*") == 0 || strcmp(tok, "/") == 0 ||
            strcmp(tok, "%") == 0 || strcmp(tok, "^") == 0);
}

// utilidade: verifica se token é função unária suportada
static int isFuncUnaria(const char *tok)
{
    return (strcmp(tok, "raiz") == 0 || strcmp(tok, "sen") == 0 ||
            strcmp(tok, "cos") == 0 || strcmp(tok, "tg") == 0 ||
            strcmp(tok, "log") == 0);
}

/*utilidade: remover parênteses externos desnecessários
Ex.: "(5*(2+3))" - "5*(2+3)" (retira um par externo se balanceado)*/
static void retirarParentesesExternos(char *s)
{
    size_t n = strlen(s);
    if (n <= 1)
        return;
    if (s[0] != '(' || s[n - 1] != ')')
        return;

    // verifica se o par externo cobre toda a expressão
    int bal = 0;
    int cobre = 1;
    for (size_t i = 0; i < n - 1; ++i)
    {
        if (s[i] == '(')
            bal++;
        else if (s[i] == ')')
            bal--;
        if (bal == 0 && i < n - 2)
        { // encontrou fechamento antes do último par
            cobre = 0;
            break;
        }
    }
    if (cobre && bal == 1)
    {
        // remove o primeiro e o último char
        memmove(s, s + 1, n - 2);
        s[n - 2] = '\0';
        // tentar recursivamente para remover múltiplos pares redundantes
        retirarParentesesExternos(s);
    }
}

/*Função: getValorPosFixa
Recebe a string pos-fixa com tokens separados por espaço.
Retorna float com o valor calculado. Se a expressao for
inválida, retorna 0.0*/
float getValorPosFixa(char *StrPosFixa)
{
    if (StrPosFixa == NULL)
        return 0.0f;

    PilhaFloat p;
    initPilhaFloat(&p);

    char copia[MAXSTR];
    strncpy(copia, StrPosFixa, MAXSTR - 1);
    copia[MAXSTR - 1] = '\0';

    char *tok = strtok(copia, " ");
    while (tok != NULL)
    {
        if (isOperadorBin(tok))
        {
            // precisa de dois operandos
            if (p.topo < 1)
                return 0.0f;
            float b = popFloat(&p);
            float a = popFloat(&p);
            float r = 0.0f;

            if (strcmp(tok, "+") == 0)
                r = a + b;
            else if (strcmp(tok, "-") == 0)
                r = a - b;
            else if (strcmp(tok, "*") == 0)
                r = a * b;
            else if (strcmp(tok, "/") == 0)
            {
                r = a / b;
            }
            else if (strcmp(tok, "%") == 0)
            {
                r = fmodf(a, b);
            }
            else if (strcmp(tok, "^") == 0)
            {
                r = powf(a, b);
            }
            pushFloat(&p, r);
        }
        else if (isFuncUnaria(tok))
        {
            // precisa de um operando
            if (p.topo < 0)
                return 0.0f;
            float a = popFloat(&p);
            float r = 0.0f;

            if (strcmp(tok, "raiz") == 0)
                r = sqrtf(a);
            else if (strcmp(tok, "sen") == 0)
                r = sinf((float)(a * M_PI / 180.0));
            else if (strcmp(tok, "cos") == 0)
                r = cosf((float)(a * M_PI / 180.0));
            else if (strcmp(tok, "tg") == 0)
                r = tanf((float)(a * M_PI / 180.0));
            else if (strcmp(tok, "log") == 0)
                r = log10f(a);

            pushFloat(&p, r);
        }
        else
        {
            // número (operando). aceita inteiro ou float
            // usei atof - expressão mal formada sera igual a 0.0
            float v = (float)atof(tok);
            pushFloat(&p, v);
        }

        tok = strtok(NULL, " ");
    }

    // resultado final tem que ser unico
    if (p.topo != 0)
        return 0.0f;
    return popFloat(&p);
}

/*Função - getFormaInFixa
Converte expressão pos-fixa com espaço para expressão infixa sem espaço e com os parenteses nos lugares certos
Em caso de erro de escrita, retorna NULL.*/
char *getFormaInFixa(char *Str)
{
    static char resultado[MAXSTR];
    if (Str == NULL)
        return NULL;

    PilhaStr ps;
    initPilhaStr(&ps);

    char copia[MAXSTR];
    strncpy(copia, Str, MAXSTR - 1);
    copia[MAXSTR - 1] = '\0';

    char *tok = strtok(copia, " ");
    while (tok != NULL)
    {
        if (isOperadorBin(tok))
        {
            // precisa de dois operandos (strings)
            if (ps.topo < 1)
                return NULL;
            char b[MAXSTR], a[MAXSTR], aux[MAXSTR];

            char *pb = popStrPtr(&ps);
            char *pa = popStrPtr(&ps);
            if (pb == NULL || pa == NULL)
                return NULL;
            strncpy(b, pb, MAXSTR - 1);
            b[MAXSTR - 1] = '\0';
            strncpy(a, pa, MAXSTR - 1);
            a[MAXSTR - 1] = '\0';

            // construir - (a<op>b) sempre gera parentese externo porem o resultado no final tira o parentese externo
            int aIsFunc = (strchr(a, '(') != NULL && a[strlen(a) - 1] == ')');

            int bIsFunc = (strchr(b, '(') != NULL && b[strlen(b) - 1] == ')');

            if (aIsFunc && bIsFunc)
            {
                snprintf(aux, MAXSTR, "%s%s%s", a, tok, b);
            }
            else if (aIsFunc)
            {
                snprintf(aux, MAXSTR, "%s%s(%s)", a, tok, b);
            }
            else if (bIsFunc)
            {
                snprintf(aux, MAXSTR, "(%s)%s%s", a, tok, b);
            }
            else
            {
                snprintf(aux, MAXSTR, "(%s%s%s)", a, tok, b);
            }

            // empurra aux
            if (!pushStr(&ps, aux))
                return NULL;
        }
        else if (isFuncUnaria(tok))
        {
            // precisa de um operando
            if (ps.topo < 0)
                return NULL;
            char a[MAXSTR], aux[MAXSTR];
            char *pa = popStrPtr(&ps);
            if (pa == NULL)
                return NULL;
            strncpy(a, pa, MAXSTR - 1);
            a[MAXSTR - 1] = '\0';

            // formato func(operando)
            snprintf(aux, MAXSTR, "%s(%s)", tok, a);
            retirarParentesesExternos(aux);

            if (!pushStr(&ps, aux))
                return NULL;
        }
        else
        {
            // operando armazena sem espaços
            // remover espaços em branco extras no token mesmo que o strtok já separe
            char numero[MAXSTR];
            strncpy(numero, tok, MAXSTR - 1);
            numero[MAXSTR - 1] = '\0';
            if (!pushStr(&ps, numero))
                return NULL;
        }

        tok = strtok(NULL, " ");
    }

    if (ps.topo != 0)
        return NULL;

    // resultado final
    strncpy(resultado, ps.dados[ps.topo], MAXSTR - 1);
    resultado[MAXSTR - 1] = '\0';

    // remover parênteses externos redundantes
    retirarParentesesExternos(resultado);

    // remove os espaços do resultado
    char compact[MAXSTR];
    int j = 0;
    for (size_t i = 0; i < strlen(resultado) && j < MAXSTR - 1; ++i)
    {
        if (resultado[i] != ' ')
            compact[j++] = resultado[i];
    }
    compact[j] = '\0';
    strncpy(resultado, compact, MAXSTR - 1);
    resultado[MAXSTR - 1] = '\0';

    return resultado;
}
