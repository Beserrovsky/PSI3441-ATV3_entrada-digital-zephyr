# Relatório — Entrada Digital: Seguidor de Linha com Sensores IR

## Nome
Felipe Beserra de Oliveira

---

## Número USP
13683702

---

## Respostas, comentários e análises

### Descrição da Atividade

O experimento utiliza dois sensores de infravermelho WH201 como entradas digitais para controlar os motores de um carrinho seguidor de linha. Os sensores detectam a presença de uma faixa preta sobre superfície branca, e o programa decide a direção dos motores para manter o carrinho sobre a linha.

### Hardware utilizado

| Componente | Descrição |
|---|---|
| Sensores IR | WH201 × 2 (saída digital) |
| Ponte H | L298N (controla direção dos motores) |
| Pino sensor esquerdo | PTB0 (entrada com pull-up interno) |
| Pino sensor direito | PTB1 (entrada com pull-up interno) |
| Motor A (esquerdo) | PTA4 (IN1), PTA5 (IN2) |
| Motor B (direito) | PTA12 (IN3), PTA13 (IN4) |

### Lógica do sensor WH201

O WH201 possui um comparador integrado que compara a reflectividade detectada com um limiar ajustável por potenciômetro:

- **Saída LOW (0):** linha preta detectada
- **Saída HIGH (1):** superfície branca (sem linha)

Para a entrada digital, configurou-se o pull-up interno do KL25Z. Isso garante leitura estável mesmo com o sensor desconectado.

### Lógica de controle (seguidor de linha)

| Sensor esquerdo | Sensor direito | Ação |
|---|---|---|
| 0 (linha) | 0 (linha) | Frente (ambos motores ligados) |
| 0 (linha) | 1 (sem linha) | Vira à esquerda (motor direito apenas) |
| 1 (sem linha) | 0 (linha) | Vira à direita (motor esquerdo apenas) |
| 1 (sem linha) | 1 (sem linha) | Para (ambos desligados) |

### Controle da ponte H (L298N)

A direção de cada motor é determinada pelos dois pinos de controle correspondentes:

| IN1 | IN2 | Estado do motor |
|---|---|---|
| 1 | 0 | Para frente |
| 0 | 1 | Para trás |
| 0 | 0 | Parado (free-wheel) |
| 1 | 1 | Freio (brake) |

### Loop de controle

O programa executa a leitura dos sensores e atualiza os motores a cada 20 ms, equivalente a uma taxa de controle de 50 Hz. Essa frequência é suficiente para pistas com curvas suaves.

Para otimizar o desempenho (curvas mais fechadas ou velocidades maiores), pode-se reduzir o intervalo de leitura ou adicionar controle PWM de velocidade para correções proporcionais.

---

## Código (main.c)

```c
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define SENSOR_L_PIN    0
#define SENSOR_R_PIN    1
#define SENSOR_GPIO     DT_NODELABEL(gpiob)

#define MOTOR_A_IN1     4
#define MOTOR_A_IN2     5
#define MOTOR_B_IN1    12
#define MOTOR_B_IN2    13
#define MOTOR_GPIO      DT_NODELABEL(gpioa)

static const struct device *gpiob = DEVICE_DT_GET(SENSOR_GPIO);
static const struct device *gpioa = DEVICE_DT_GET(MOTOR_GPIO);

static void motor_forward(void)  { gpio_pin_set(gpioa, MOTOR_A_IN1, 1); gpio_pin_set(gpioa, MOTOR_A_IN2, 0); gpio_pin_set(gpioa, MOTOR_B_IN1, 1); gpio_pin_set(gpioa, MOTOR_B_IN2, 0); }
static void motor_turn_left(void) { gpio_pin_set(gpioa, MOTOR_A_IN1, 0); gpio_pin_set(gpioa, MOTOR_A_IN2, 0); gpio_pin_set(gpioa, MOTOR_B_IN1, 1); gpio_pin_set(gpioa, MOTOR_B_IN2, 0); }
static void motor_turn_right(void){ gpio_pin_set(gpioa, MOTOR_A_IN1, 1); gpio_pin_set(gpioa, MOTOR_A_IN2, 0); gpio_pin_set(gpioa, MOTOR_B_IN1, 0); gpio_pin_set(gpioa, MOTOR_B_IN2, 0); }
static void motor_stop(void)      { gpio_pin_set(gpioa, MOTOR_A_IN1, 0); gpio_pin_set(gpioa, MOTOR_A_IN2, 0); gpio_pin_set(gpioa, MOTOR_B_IN1, 0); gpio_pin_set(gpioa, MOTOR_B_IN2, 0); }

int main(void)
{
    gpio_pin_configure(gpiob, SENSOR_L_PIN, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpiob, SENSOR_R_PIN, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpioa, MOTOR_A_IN1, GPIO_OUTPUT_LOW);
    gpio_pin_configure(gpioa, MOTOR_A_IN2, GPIO_OUTPUT_LOW);
    gpio_pin_configure(gpioa, MOTOR_B_IN1, GPIO_OUTPUT_LOW);
    gpio_pin_configure(gpioa, MOTOR_B_IN2, GPIO_OUTPUT_LOW);

    while (1) {
        int left  = gpio_pin_get(gpiob, SENSOR_L_PIN);
        int right = gpio_pin_get(gpiob, SENSOR_R_PIN);

        if (!left && !right)     motor_forward();
        else if (!left && right) motor_turn_left();
        else if (left && !right) motor_turn_right();
        else                     motor_stop();

        k_msleep(20);
    }
    return 0;
}
```

---

## Repositório

```text
https://github.com/Beserrovsky/Atividade-3
```
