#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

void main() {
    rcc_periph_clock_enable(RCC_GPIOB);

    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO1);

    gpio_set(GPIOB, GPIO1);

    for (;;);
}
