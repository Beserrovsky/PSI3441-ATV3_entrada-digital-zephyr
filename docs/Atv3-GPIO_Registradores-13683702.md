# Relatório — LED via Registradores no FRDM-KL25Z

## Nome

Felipe Beserra de Oliveira

---

## Número USP

13683702

---

## Respostas, comentários e análises

### Descrição da Atividade

O enunciado pede um programa que faça o LED verde da FRDM-KL25Z piscar com período de 2 segundos, configurando os registradores do microcontrolador diretamente (sem usar a API GPIO do Zephyr nem Device Tree), seguindo a sequência:

1. Habilitar o clock da porta B (`SIM_SCGC5`);
2. Configurar o pino 19 (`PORTB_PCR19`);
3. Setar a direção do pino (`GPIOB_PDDR`);
4. Habilitar saída (`GPIOB_PCOR`, liga o LED);
5. Função de espera;
6. Desabilitar saída (`GPIOB_PSOR`, desliga o LED);
7. Função de espera;
8. Repetir os passos (4)-(7).

### Registradores utilizados

| Registrador     | Endereço  | Função                                                     |
| --------------- | ---------- | ------------------------------------------------------------ |
| `SIM_SCGC5`   | 0x40048038 | Habilita o clock do Port B (bit 10)                          |
| `PORTB_PCR19` | 0x4004A04C | Configura o pino PTB19: MUX = 001 seleciona a função GPIO  |
| `GPIOB_PDDR`  | 0x400FF054 | Define a direção do pino (1 = saída)                      |
| `GPIOB_PSOR`  | 0x400FF044 | *Set* — escrever 1 no bit coloca o pino em nível alto    |
| `GPIOB_PCOR`  | 0x400FF048 | *Clear* — escrever 1 no bit coloca o pino em nível baixo |

O LED verde da FRDM-KL25Z é *active low* (`0` = ligado, `1` = desligado), por isso `GPIOB_PCOR` liga o LED e `GPIOB_PSOR` desliga.

### Função de espera

O enunciado original sugere uma função `delayMs()` baseada em um laço de contagem (busy-wait), apropriada para código bare-metal. Como o projeto roda sobre o Zephyr RTOS, foi usada a função do kernel `k_msleep(1000)` para os dois intervalos de 1 segundo (ligado e desligado), totalizando o período de 2 segundos pedido — equivalente em efeito, porém integrada ao escalonador do RTOS em vez de um laço de busy-wait bloqueante.

---

## Código (main.c)

```c
/*
 * Atividade 3 — GPIO via Registradores
 * FRDM-KL25Z: pisca o LED verde (PTB19) com periodo de 2s usando
 * acesso direto aos registradores do KL25Z (SIM_SCGC5, PCR, PDDR,
 * PSOR, PCOR), sem usar a API Zephyr GPIO.
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* SIM — System Integration Module (habilitação de clock do Port B) */
#define SIM_SCGC5           (*((volatile uint32_t *)0x40048038U))
#define SIM_SCGC5_PORTB     (1U << 10)

/* PORT — Pin Control Register (multiplexação do pino) */
#define PORTB_PCR19         (*((volatile uint32_t *)0x4004A04CU))
#define PCR_MUX_GPIO        (1U << 8)   /* MUX = 001 seleciona função GPIO */

/* GPIO B — registradores de direção e saída */
#define GPIOB_PDDR          (*((volatile uint32_t *)0x400FF054U))
#define GPIOB_PSOR          (*((volatile uint32_t *)0x400FF044U))   /* set (HIGH) */
#define GPIOB_PCOR          (*((volatile uint32_t *)0x400FF048U))   /* clear (LOW) */

#define GREEN_MASK          (1U << 19)  /* PTB19 */

static void gpio_regs_init(void)
{
    /* 1. Habilita clock do Port B via SIM_SCGC5 */
    SIM_SCGC5 |= SIM_SCGC5_PORTB;

    /* 2. Configura multiplexação do pino para GPIO (MUX=001) */
    PORTB_PCR19 = PCR_MUX_GPIO;

    /* 3. Define o pino como saída no registrador de direção (PDDR) */
    GPIOB_PDDR |= GREEN_MASK;

    /* 4. Apaga o LED (active low: escrever 1 = desligado) */
    GPIOB_PSOR = GREEN_MASK;
}

int main(void)
{
    gpio_regs_init();
    printk("=== Atividade 3 - LED verde via registradores (periodo 2s) ===\n");

    while (1) {
        GPIOB_PCOR = GREEN_MASK;   /* liga (active low) */
        k_msleep(1000);
        GPIOB_PSOR = GREEN_MASK;   /* desliga */
        k_msleep(1000);
    }

    return 0;
}
```

---

## Repositório

```text
https://github.com/Beserrovsky/PSI3441-ATV3_entrada-digital-zephyr
```
