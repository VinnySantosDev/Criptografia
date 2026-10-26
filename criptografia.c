/*
 * ============================================================================
 *  Criptografia com Sequencias Matematicas
 *  Disciplina: Algoritmo e Pensamento Computacional
 *  Atividade Avaliativa 01
 * ============================================================================
 *
 *  Duas camadas de criptografia sobre uma palavra (ate 15 letras):
 *    Camada 1: Cifra de Cesar (SHIFT fixo)
 *    Camada 2: deslocamento dinamico pela sequencia numerica escolhida
 *
 *    deslocamento[i] = SHIFT + sequencia[i]        (aplicado modulo 26)
 *
 *  Sequencias disponiveis (o numero e o "Tipo" gravado no arquivo):
 *    1 - Progressao Aritmetica (PA)
 *    2 - Progressao Geometrica (PG)
 *    3 - Fibonacci
 *    4 - Numeros Primos
 *
 *  Arquivos gerados:
 *    resultado_criptografia.txt -> ultima palavra codificada
 *    log_execucao.txt           -> historico de tudo que foi executado
 *
 *  Compilar:  gcc -Wall -Wextra -std=c99 -o criptografia criptografia.c
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>

#define MAX_PALAVRA   15
#define TAM_BUFFER    128
#define TAM_ALFABETO  26
#define ARQ_RESULTADO "resultado_criptografia.txt"
#define ARQ_LOG       "log_execucao.txt"

typedef enum {
    SEQ_PA = 1,
    SEQ_PG = 2,
    SEQ_FIBONACCI = 3,
    SEQ_PRIMOS = 4
} TipoSequencia;

typedef struct {
    TipoSequencia tipo;
    long long a1;     /* primeiro termo (PA e PG)        */
    long long razao;  /* r da PA ou q da PG              */
} ConfigSequencia;

typedef enum { CRIPTOGRAFAR = 1, DESCRIPTOGRAFAR = -1 } Sentido;

/* ------------------------------------------------------------------------ */
/*  Prototipos                                                              */
/* ------------------------------------------------------------------------ */
static int  lerLinha(char *buf, size_t tam);
static int  lerInteiro(const char *msg, long long min, long long max, long long *out);
static int  validarPalavra(const char *palavra);
static int  ehPrimo(long long n);
static void gerarSequencia(const ConfigSequencia *cfg, long long *seq, int n);
static char deslocarLetra(char c, long long deslocamento);
static void aplicarCifra(const char *entrada, char *saida, int shift,
                         const long long *seq, Sentido sentido);
static const char *nomeSequencia(TipoSequencia tipo);
static void descreverSequencia(const ConfigSequencia *cfg, char *destino, size_t tam);
static void mostrarDetalhes(const char *entrada, const char *saida, int shift,
                            const long long *seq, Sentido sentido);
static int  salvarResultado(const char *palavraCodificada, int shift,
                            TipoSequencia tipo, int letras);
static void registrarLog(const char *formato, ...);
static int  lerPalavra(char *palavra);
static int  lerShift(int *shift);
static int  lerConfigSequencia(ConfigSequencia *cfg);
static void executarCifra(Sentido sentido);
static void compararSequencias(void);
static void exibirMenu(void);

/* ------------------------------------------------------------------------ */
/*  Entrada de dados (sempre com fgets: evita lixo no buffer do scanf)     */
/* ------------------------------------------------------------------------ */

/* Le uma linha e remove o '\n'. Retorna 0 em EOF. */
static int lerLinha(char *buf, size_t tam)
{
    size_t len;

    if (fgets(buf, (int)tam, stdin) == NULL) {
        buf[0] = '\0';
        return 0;
    }
    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        /* linha maior que o buffer: descarta o resto */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    return 1;
}

/* Le um inteiro dentro de [min, max]. Repete ate ser valido. Retorna 0 em EOF. */
static int lerInteiro(const char *msg, long long min, long long max, long long *out)
{
    char buf[TAM_BUFFER];
    char extra;

    for (;;) {
        printf("%s", msg);
        if (!lerLinha(buf, sizeof buf)) return 0;
        if (sscanf(buf, "%lld %c", out, &extra) == 1 && *out >= min && *out <= max)
            return 1;
        printf("  Valor invalido. Digite um inteiro entre %lld e %lld.\n", min, max);
    }
}

/* Palavra valida: 1 a 15 letras A-Z/a-z, sem acentos, espacos ou simbolos. */
static int validarPalavra(const char *palavra)
{
    size_t i, len = strlen(palavra);

    if (len == 0 || len > MAX_PALAVRA) return 0;
    for (i = 0; i < len; i++) {
        unsigned char c = (unsigned char)palavra[i];
        /* c > 127 = byte de caractere acentuado (UTF-8/Latin-1) */
        if (c > 127 || !isalpha(c)) return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------------ */
/*  Matematica: sequencias                                                  */
/* ------------------------------------------------------------------------ */

static int ehPrimo(long long n)
{
    long long d;

    if (n < 2) return 0;
    if (n % 2 == 0) return n == 2;
    for (d = 3; d * d <= n; d += 2)
        if (n % d == 0) return 0;
    return 1;
}

/* Preenche seq[0..n-1] com os n primeiros termos da sequencia escolhida. */
static void gerarSequencia(const ConfigSequencia *cfg, long long *seq, int n)
{
    int i;
    long long candidato;

    switch (cfg->tipo) {
    case SEQ_PA:            /* a_n = a1 + (n-1) * r */
        for (i = 0; i < n; i++)
            seq[i] = cfg->a1 + (long long)i * cfg->razao;
        break;

    case SEQ_PG:            /* a_n = a1 * q^(n-1) */
        if (n > 0) seq[0] = cfg->a1;
        for (i = 1; i < n; i++)
            seq[i] = seq[i - 1] * cfg->razao;
        break;

    case SEQ_FIBONACCI:     /* F_n = F_(n-1) + F_(n-2), F1 = F2 = 1 */
        for (i = 0; i < n; i++)
            seq[i] = (i < 2) ? 1 : seq[i - 1] + seq[i - 2];
        break;

    case SEQ_PRIMOS:        /* 2, 3, 5, 7, 11, ... */
        candidato = 2;
        for (i = 0; i < n; candidato++)
            if (ehPrimo(candidato))
                seq[i++] = candidato;
        break;
    }
}

/* ------------------------------------------------------------------------ */
/*  Criptografia                                                            */
/* ------------------------------------------------------------------------ */

/* Desloca uma letra dentro do alfabeto, preservando maiuscula/minuscula.
 * Funciona com deslocamento negativo (usado na descriptografia). */
static char deslocarLetra(char c, long long deslocamento)
{
    char base = isupper((unsigned char)c) ? 'A' : 'a';
    long long pos = (c - base) + (deslocamento % TAM_ALFABETO);

    pos = ((pos % TAM_ALFABETO) + TAM_ALFABETO) % TAM_ALFABETO;
    return (char)(base + pos);
}

/* Camada 1 (SHIFT) + Camada 2 (sequencia[i]).
 * sentido = +1 criptografa, -1 descriptografa (operacao inversa). */
static void aplicarCifra(const char *entrada, char *saida, int shift,
                         const long long *seq, Sentido sentido)
{
    size_t i, len = strlen(entrada);

    for (i = 0; i < len; i++) {
        long long desloc = (shift + seq[i] % TAM_ALFABETO) % TAM_ALFABETO;
        saida[i] = deslocarLetra(entrada[i], (long long)sentido * desloc);
    }
    saida[len] = '\0';
}

/* ------------------------------------------------------------------------ */
/*  Apresentacao                                                            */
/* ------------------------------------------------------------------------ */

static const char *nomeSequencia(TipoSequencia tipo)
{
    switch (tipo) {
    case SEQ_PA:        return "Progressao Aritmetica (PA)";
    case SEQ_PG:        return "Progressao Geometrica (PG)";
    case SEQ_FIBONACCI: return "Fibonacci";
    case SEQ_PRIMOS:    return "Numeros Primos";
    }
    return "Desconhecida";
}

static void descreverSequencia(const ConfigSequencia *cfg, char *destino, size_t tam)
{
    if (cfg->tipo == SEQ_PA)
        snprintf(destino, tam, "%s [a1=%lld, r=%lld]",
                 nomeSequencia(cfg->tipo), cfg->a1, cfg->razao);
    else if (cfg->tipo == SEQ_PG)
        snprintf(destino, tam, "%s [a1=%lld, q=%lld]",
                 nomeSequencia(cfg->tipo), cfg->a1, cfg->razao);
    else
        snprintf(destino, tam, "%s", nomeSequencia(cfg->tipo));
}

/* Tabela passo a passo: mostra o "calculo interno" letra por letra. */
static void mostrarDetalhes(const char *entrada, const char *saida, int shift,
                            const long long *seq, Sentido sentido)
{
    size_t i, len = strlen(entrada);

    printf("\n  %-3s %-7s %-18s %-18s %-9s %-6s\n",
           "#", "Letra", "Sequencia[i]", "SHIFT+seq", "mod 26", "Saida");
    printf("  ----------------------------------------------------------------\n");
    for (i = 0; i < len; i++) {
        long long total = shift + seq[i];
        long long efetivo = (shift + seq[i] % TAM_ALFABETO) % TAM_ALFABETO;
        printf("  %-3zu %-7c %-18lld %-18lld %c%-8lld %-6c\n",
               i + 1, entrada[i], seq[i], total,
               sentido == CRIPTOGRAFAR ? '+' : '-', efetivo, saida[i]);
    }
}

/* ------------------------------------------------------------------------ */
/*  Arquivos                                                                */
/* ------------------------------------------------------------------------ */

/* Grava no formato pedido no enunciado. Retorna 1 se gravou. */
static int salvarResultado(const char *palavraCodificada, int shift,
                           TipoSequencia tipo, int letras)
{
    FILE *arq = fopen(ARQ_RESULTADO, "w");

    if (arq == NULL) {
        perror("  Erro ao abrir " ARQ_RESULTADO);
        return 0;
    }
    fprintf(arq, "Palavra codificada: %s | SHIFT: %d | Tipo: %d | Letras: %d\n",
            palavraCodificada, shift, (int)tipo, letras);
    fclose(arq);
    return 1;
}

/* Acrescenta uma linha com data/hora no log (modo "a" = append). */
static void registrarLog(const char *formato, ...)
{
    FILE *arq = fopen(ARQ_LOG, "a");
    char dataHora[32];
    time_t agora = time(NULL);
    va_list args;

    if (arq == NULL) return;   /* log nao pode derrubar o programa */

    strftime(dataHora, sizeof dataHora, "%Y-%m-%d %H:%M:%S", localtime(&agora));
    fprintf(arq, "[%s] ", dataHora);

    va_start(args, formato);
    vfprintf(arq, formato, args);
    va_end(args);

    fputc('\n', arq);
    fclose(arq);
}

/* ------------------------------------------------------------------------ */
/*  Leitura dos parametros (recebem ponteiros para devolver os valores)    */
/* ------------------------------------------------------------------------ */

static int lerPalavra(char *palavra)
{
    char buf[TAM_BUFFER];

    for (;;) {
        printf("Palavra (1 a %d letras, sem acentos/espacos/simbolos): ", MAX_PALAVRA);
        if (!lerLinha(buf, sizeof buf)) return 0;
        if (validarPalavra(buf)) {
            strcpy(palavra, buf);
            return 1;
        }
        printf("  Palavra invalida.\n");
        registrarLog("ENTRADA REJEITADA: palavra invalida \"%s\"", buf);
    }
}

static int lerShift(int *shift)
{
    long long v;

    if (!lerInteiro("SHIFT (1 a 25): ", 1, 25, &v)) return 0;
    *shift = (int)v;
    return 1;
}

static int lerConfigSequencia(ConfigSequencia *cfg)
{
    long long v;

    printf("\nSequencia numerica:\n");
    printf("  1 - Progressao Aritmetica (PA)\n");
    printf("  2 - Progressao Geometrica (PG)\n");
    printf("  3 - Fibonacci\n");
    printf("  4 - Numeros Primos\n");
    if (!lerInteiro("Escolha: ", 1, 4, &v)) return 0;
    cfg->tipo = (TipoSequencia)v;
    cfg->a1 = 0;
    cfg->razao = 0;

    if (cfg->tipo == SEQ_PA) {
        if (!lerInteiro("  Primeiro termo a1 (-1000 a 1000): ", -1000, 1000, &cfg->a1)) return 0;
        if (!lerInteiro("  Razao r (-1000 a 1000): ", -1000, 1000, &cfg->razao)) return 0;
    } else if (cfg->tipo == SEQ_PG) {
        /* limites garantem que a1 * q^14 cabe em long long (1000 * 10^14) */
        if (!lerInteiro("  Primeiro termo a1 (1 a 1000): ", 1, 1000, &cfg->a1)) return 0;
        if (!lerInteiro("  Razao q (1 a 10): ", 1, 10, &cfg->razao)) return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------------ */
/*  Opcoes do menu                                                          */
/* ------------------------------------------------------------------------ */

static void executarCifra(Sentido sentido)
{
    char palavra[MAX_PALAVRA + 1];
    char resultado[MAX_PALAVRA + 1];
    char descricao[TAM_BUFFER];
    long long seq[MAX_PALAVRA];
    ConfigSequencia cfg;
    int shift, n;
    size_t i;

    printf("\n=== %s ===\n", sentido == CRIPTOGRAFAR ? "CRIPTOGRAFAR" : "DESCRIPTOGRAFAR");
    if (!lerPalavra(palavra) || !lerShift(&shift) || !lerConfigSequencia(&cfg)) return;

    n = (int)strlen(palavra);
    gerarSequencia(&cfg, seq, n);
    aplicarCifra(palavra, resultado, shift, seq, sentido);
    descreverSequencia(&cfg, descricao, sizeof descricao);

    printf("\nSequencia usada: %s\n  ", descricao);
    for (i = 0; i < (size_t)n; i++)
        printf("%lld%s", seq[i], i + 1 < (size_t)n ? ", " : "\n");

    mostrarDetalhes(palavra, resultado, shift, seq, sentido);

    if (sentido == CRIPTOGRAFAR) {
        printf("\nPalavra criptografada: %s\n", resultado);
        if (salvarResultado(resultado, shift, cfg.tipo, n))
            printf("Resultado gravado em %s\n", ARQ_RESULTADO);
        registrarLog("CRIPTOGRAFAR | entrada=%s | saida=%s | SHIFT=%d | %s | letras=%d",
                     palavra, resultado, shift, descricao, n);
    } else {
        printf("\nPalavra original: %s\n", resultado);
        registrarLog("DESCRIPTOGRAFAR | entrada=%s | saida=%s | SHIFT=%d | %s | letras=%d",
                     palavra, resultado, shift, descricao, n);
    }
}

/* Etapas "Analisar" e "Avaliar" da Taxonomia de Bloom:
 * mesma palavra e mesmo SHIFT, quatro sequencias diferentes. */
static void compararSequencias(void)
{
    ConfigSequencia configs[4] = {
        { SEQ_PA,        1, 1 },
        { SEQ_PG,        1, 2 },
        { SEQ_FIBONACCI, 0, 0 },
        { SEQ_PRIMOS,    0, 0 }
    };
    char palavra[MAX_PALAVRA + 1];
    char resultado[MAX_PALAVRA + 1];
    char descricao[TAM_BUFFER];
    long long seq[MAX_PALAVRA];
    int shift, n, k, i;

    printf("\n=== COMPARAR SEQUENCIAS ===\n");
    if (!lerPalavra(palavra) || !lerShift(&shift)) return;
    n = (int)strlen(palavra);

    printf("\n  %-38s %-16s %s\n", "Sequencia", "Resultado", "Deslocamentos distintos");
    printf("  --------------------------------------------------------------------------\n");

    for (k = 0; k < 4; k++) {
        int usados[TAM_ALFABETO] = {0};
        int distintos = 0, temZero = 0;

        gerarSequencia(&configs[k], seq, n);
        aplicarCifra(palavra, resultado, shift, seq, CRIPTOGRAFAR);
        descreverSequencia(&configs[k], descricao, sizeof descricao);

        for (i = 0; i < n; i++) {
            int d = (int)((shift + seq[i] % TAM_ALFABETO) % TAM_ALFABETO);
            if (d == 0) temZero = 1;
            if (!usados[d]) { usados[d] = 1; distintos++; }
        }
        printf("  %-38s %-16s %d de %d%s\n", descricao, resultado, distintos, n,
               temZero ? "  (ATENCAO: alguma letra nao mudou)" : "");
        registrarLog("COMPARAR | entrada=%s | SHIFT=%d | %s | saida=%s | distintos=%d/%d",
                     palavra, shift, descricao, resultado, distintos, n);
    }

    printf("\n  Leitura: quanto mais deslocamentos distintos, menos padrao vaza.\n");
    printf("  Mas NENHUMA delas e segura: ver secao de seguranca na documentacao.\n");
}

static void exibirMenu(void)
{
    printf("\n==============================================\n");
    printf("   CRIPTOGRAFIA COM SEQUENCIAS MATEMATICAS\n");
    printf("==============================================\n");
    printf("  1 - Criptografar palavra\n");
    printf("  2 - Descriptografar palavra\n");
    printf("  3 - Comparar sequencias (analise)\n");
    printf("  0 - Sair\n");
}

int main(void)
{
    long long opcao;

    registrarLog("INICIO DA EXECUCAO");

    for (;;) {
        exibirMenu();
        if (!lerInteiro("Opcao: ", 0, 3, &opcao)) break;   /* EOF */

        switch (opcao) {
        case 1: executarCifra(CRIPTOGRAFAR);    break;
        case 2: executarCifra(DESCRIPTOGRAFAR); break;
        case 3: compararSequencias();           break;
        case 0:
            registrarLog("FIM DA EXECUCAO");
            printf("Encerrando.\n");
            return EXIT_SUCCESS;
        }
    }

    registrarLog("FIM DA EXECUCAO (entrada encerrada)");
    return EXIT_SUCCESS;
}
