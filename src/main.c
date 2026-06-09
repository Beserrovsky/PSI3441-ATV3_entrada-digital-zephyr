/*
 * Atividade 3 — GPIO via Registradores
 * FRDM-KL25Z: controle do LED RGB usando acesso direto aos registradores
 * do KL25Z (SIM_SCGC5, PCR, PDDR, PSOR, PCOR) sem usar a API Zephyr GPIO.
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* SIM — System Integration Module (habilitação de clock dos ports) */
#define SIM_SCGC5           (*((volatile uint32_t *)0x40048038U))
#define SIM_SCGC5_PORTB     (1U << 10)
#define SIM_SCGC5_PORTD     (1U << 12)

/* PORT — Pin Control Registers (multiplexação do pino) */
#define PORTB_PCR18         (*((volatile uint32_t *)0x4004A048U))
#define PORTB_PCR19         (*((volatile uint32_t *)0x4004A04CU))
#define PORTD_PCR1          (*((volatile uint32_t *)0x4004C004U))
#define PCR_MUX_GPIO        (1U << 8)   /* MUX = 001 seleciona função GPIO */

/* GPIO B — registradores de direção e saída */
#define GPIOB_PDDR          (*((volatile uint32_t *)0x400FF054U))
#define GPIOB_PSOR          (*((volatile uint32_t *)0x400FF044U))   /* set (HIGH) */
#define GPIOB_PCOR          (*((volatile uint32_t *)0x400FF048U))   /* clear (LOW) */

/* GPIO D — registradores de direção e saída */
#define GPIOD_PDDR          (*((volatile uint32_t *)0x400FF0D4U))
#define GPIOD_PSOR          (*((volatile uint32_t *)0x400FF0C4U))
#define GPIOD_PCOR          (*((volatile uint32_t *)0x400FF0C8U))

/* Máscaras de bit para cada LED (active low: PCOR liga, PSOR desliga) */
#define RED_MASK            (1U << 18)  /* PTB18 */
#define GREEN_MASK          (1U << 19)  /* PTB19 */
#define BLUE_MASK           (1U << 1)   /* PTD1  */

static void gpio_regs_init(void)
{
    /* 1. Habilita clock dos Port B e D via SIM_SCGC5 */
    SIM_SCGC5 |= SIM_SCGC5_PORTB | SIM_SCGC5_PORTD;

    /* 2. Configura multiplexação dos pinos para GPIO (MUX=001) */
    PORTB_PCR18 = PCR_MUX_GPIO;
    PORTB_PCR19 = PCR_MUX_GPIO;
    PORTD_PCR1  = PCR_MUX_GPIO;

    /* 3. Define pinos como saída no registrador de direção (PDDR) */
    GPIOB_PDDR |= RED_MASK | GREEN_MASK;
    GPIOD_PDDR |= BLUE_MASK;

    /* 4. Apaga todos os LEDs (active low: escrever 1 = desligado) */
    GPIOB_PSOR = RED_MASK | GREEN_MASK;
    GPIOD_PSOR = BLUE_MASK;
}

int main(void)
{
    gpio_regs_init();
    printk("=== GPIO via Registradores — FRDM-KL25Z ===\n");

    while (1) {
        printk("Vermelho\n");
        GPIOB_PCOR = RED_MASK;
        k_msleep(1000);
        GPIOB_PSOR = RED_MASK;

        printk("Verde\n");
        GPIOB_PCOR = GREEN_MASK;
        k_msleep(1000);
        GPIOB_PSOR = GREEN_MASK;

        printk("Azul\n");
        GPIOD_PCOR = BLUE_MASK;
        k_msleep(1000);
        GPIOD_PSOR = BLUE_MASK;

        printk("Branco (R+G+B)\n");
        GPIOB_PCOR = RED_MASK | GREEN_MASK;
        GPIOD_PCOR = BLUE_MASK;
        k_msleep(1000);
        GPIOB_PSOR = RED_MASK | GREEN_MASK;
        GPIOD_PSOR = BLUE_MASK;

        k_msleep(500);
    }

    return 0;
}
