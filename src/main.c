#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/*
 * IR sensor inputs (WH201): output LOW when line detected.
 * Adjust pin numbers to match the physical wiring on your board.
 */
#define SENSOR_L_PIN    0   /* PTB0 — left sensor  */
#define SENSOR_R_PIN    1   /* PTB1 — right sensor */
#define SENSOR_GPIO     DT_NODELABEL(gpiob)

/*
 * H-bridge outputs (e.g. L298N).
 * IN1/IN2 control Motor A (left wheel).
 * IN3/IN4 control Motor B (right wheel).
 * Adjust pins to match your wiring.
 */
#define MOTOR_A_IN1     4   /* PTA4  */
#define MOTOR_A_IN2     5   /* PTA5  */
#define MOTOR_B_IN1    12   /* PTA12 */
#define MOTOR_B_IN2    13   /* PTA13 */
#define MOTOR_GPIO      DT_NODELABEL(gpioa)

static const struct device *gpiob = DEVICE_DT_GET(SENSOR_GPIO);
static const struct device *gpioa = DEVICE_DT_GET(MOTOR_GPIO);

static void motor_forward(void)
{
    gpio_pin_set(gpioa, MOTOR_A_IN1, 1);
    gpio_pin_set(gpioa, MOTOR_A_IN2, 0);
    gpio_pin_set(gpioa, MOTOR_B_IN1, 1);
    gpio_pin_set(gpioa, MOTOR_B_IN2, 0);
}

static void motor_turn_left(void)
{
    /* Stop left wheel, run right wheel */
    gpio_pin_set(gpioa, MOTOR_A_IN1, 0);
    gpio_pin_set(gpioa, MOTOR_A_IN2, 0);
    gpio_pin_set(gpioa, MOTOR_B_IN1, 1);
    gpio_pin_set(gpioa, MOTOR_B_IN2, 0);
}

static void motor_turn_right(void)
{
    /* Run left wheel, stop right wheel */
    gpio_pin_set(gpioa, MOTOR_A_IN1, 1);
    gpio_pin_set(gpioa, MOTOR_A_IN2, 0);
    gpio_pin_set(gpioa, MOTOR_B_IN1, 0);
    gpio_pin_set(gpioa, MOTOR_B_IN2, 0);
}

static void motor_stop(void)
{
    gpio_pin_set(gpioa, MOTOR_A_IN1, 0);
    gpio_pin_set(gpioa, MOTOR_A_IN2, 0);
    gpio_pin_set(gpioa, MOTOR_B_IN1, 0);
    gpio_pin_set(gpioa, MOTOR_B_IN2, 0);
}

int main(void)
{
    if (!device_is_ready(gpiob) || !device_is_ready(gpioa)) {
        return -1;
    }

    /* Sensor inputs with internal pull-up */
    gpio_pin_configure(gpiob, SENSOR_L_PIN, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpiob, SENSOR_R_PIN, GPIO_INPUT | GPIO_PULL_UP);

    /* Motor outputs */
    gpio_pin_configure(gpioa, MOTOR_A_IN1, GPIO_OUTPUT_LOW);
    gpio_pin_configure(gpioa, MOTOR_A_IN2, GPIO_OUTPUT_LOW);
    gpio_pin_configure(gpioa, MOTOR_B_IN1, GPIO_OUTPUT_LOW);
    gpio_pin_configure(gpioa, MOTOR_B_IN2, GPIO_OUTPUT_LOW);

    printk("=== Seguidor de Linha — Entrada Digital ===\n");

    while (1) {
        /*
         * WH201: LOW (0) = line detected, HIGH (1) = no line.
         * sensor value 0 means "on line" for this sensor.
         */
        int left  = gpio_pin_get(gpiob, SENSOR_L_PIN);
        int right = gpio_pin_get(gpiob, SENSOR_R_PIN);

        if (!left && !right) {
            /* Both sensors on line — go straight */
            motor_forward();
        } else if (!left && right) {
            /* Only left sensor on line — veer left */
            motor_turn_left();
        } else if (left && !right) {
            /* Only right sensor on line — veer right */
            motor_turn_right();
        } else {
            /* Both sensors off line — stop and wait */
            motor_stop();
        }

        k_msleep(20);   /* 50 Hz control loop */
    }

    return 0;
}
