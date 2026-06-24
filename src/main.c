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
