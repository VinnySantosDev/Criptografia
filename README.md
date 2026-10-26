# Criptografia com Sequências Matemáticas

Projeto em C da **Atividade Avaliativa — Algoritmo e Pensamento Computacional**.
Une criptografia simples (Cifra de César) com matemática aplicada (PA, PG, Fibonacci e números primos), usando arquivos, ponteiros e menus.

## Como funciona

A palavra passa por **duas camadas**:

| Camada | O que faz |
|---|---|
| 1 — Cifra de César | desloca todas as letras pelo mesmo `SHIFT` |
| 2 — Deslocamento dinâmico | soma a cada letra o termo `i` da sequência escolhida |

```
deslocamento[i] = (SHIFT + sequencia[i]) mod 26
```

### Sequências disponíveis

| Tipo | Sequência | Fórmula | Parâmetros |
|---|---|---|---|
| 1 | Progressão Aritmética (PA) | aₙ = a₁ + (n−1)·r | a₁ e r (−1000 a 1000) |
| 2 | Progressão Geométrica (PG) | aₙ = a₁·qⁿ⁻¹ | a₁ (1 a 1000), q (1 a 10) |
| 3 | Fibonacci | Fₙ = Fₙ₋₁ + Fₙ₋₂, F₁ = F₂ = 1 | — |
| 4 | Números Primos | 2, 3, 5, 7, 11, … | — |

O número do **Tipo** é o que vai gravado no arquivo de resultado.

## Funcionalidades

- **Criptografar** — mostra a sequência gerada e uma tabela letra a letra do cálculo
- **Descriptografar** — aplica a operação inversa (subtrai o deslocamento)
- **Comparar sequências** — mesma palavra e SHIFT nas quatro sequências, lado a lado (etapas *Analisar* e *Avaliar* da Taxonomia de Bloom)
- Validação de entrada: 1 a 15 letras, sem acento, espaço ou símbolo; SHIFT de 1 a 25
- Preserva maiúsculas e minúsculas

## Compilar e executar

```bash
gcc -Wall -Wextra -std=c99 -o criptografia criptografia.c
./criptografia            # Linux / macOS
criptografia.exe          # Windows
```

## Exemplo

```
Palavra: coracao
SHIFT: 3
Sequência: 3 (Fibonacci) -> 1, 1, 2, 3, 5, 8, 13

  #  Letra  Sequencia[i]  SHIFT+seq  mod 26  Saida
  1  c      1             4          +4      g
  2  o      1             4          +4      s
  3  r      2             5          +5      w
  4  a      3             6          +6      g
  5  c      5             8          +8      k
  6  a      8             11         +11     l
  7  o      13            16         +16     e

Palavra criptografada: gswgkle
```

> **Nota sobre o enunciado:** o exemplo da atividade mostra `fqvdjqb` como saída para esta mesma entrada, mas esse valor **não corresponde** à fórmula descrita (`SHIFT + sequência[i]`). Os deslocamentos que levariam `coracao` a `fqvdjqb` seriam 3, 2, 4, 3, 7, 16, 13 — que não seguem Fibonacci nem nenhuma outra sequência do material. Seguindo a regra do enunciado, o resultado correto é `gswgkle`. O cálculo completo está em `DOCUMENTACAO.txt`.

## Arquivos gerados

| Arquivo | Conteúdo |
|---|---|
| `resultado_criptografia.txt` | última palavra codificada, no formato do enunciado |
| `log_execucao.txt` | histórico com data/hora de todas as operações (modo *append*) |

```
Palavra codificada: gswgkle | SHIFT: 3 | Tipo: 3 | Letras: 7
```

## Estrutura do projeto

```
.
├── criptografia.c      # código-fonte
├── README.md           # este arquivo
└── DOCUMENTACAO.txt    # documentação técnica, testes e análise de segurança
```

## Segurança

Esta cifra é **didática**. Nenhuma das sequências a torna segura: o espaço de chaves é pequeno (força bruta trivial) e, conhecendo um par palavra/cifra, os deslocamentos ficam expostos na hora. Detalhes na seção 7 de `DOCUMENTACAO.txt`.

## Integrantes

| Nome | RGM |
|---|---|
| Vinícius Santos Conceição | 47451530 |
| Bruna Soares de Almeida | 47552158 |
| Vinícius Henrique Silva Ferreira | 47449438 |
| Eduardo Silva Morishita | 48501883 |
