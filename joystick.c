// Data: 2025
#include <stdio.h>          // Biblioteca padrão de entrada e saída
#include "pico/stdlib.h"   // Biblioteca padrão do Raspberry Pi Pico
#include "hardware/adc.h"  // Biblioteca para manipulação do ADC no RP2040
#include "FreeRTOS.h"
#include "task.h"
#include "joystick.h"

#define vRx_PIN 26 // Pino ligado ao eixo X (ADC0)
#define vRy_PIN 27 // Pino ligado ao eixo Y (ADC1)
#define SW 22 // Pino do botão (Digital)


void joystick_task(void *params) {
    adc_init();
    adc_gpio_init(vRx_PIN);
    adc_gpio_init(vRy_PIN);
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);

    while (1) {
        adc_select_input(0); // Seleciona o canal ADC0 (vRx)
        uint16_t x = adc_read();

        adc_select_input(1); // Seleciona o canal ADC1 (vRy)
        uint16_t y = adc_read();

        bool button = gpio_get(SW) == 0;

        printf("X: %d | Y: %d | Botão: %s\n", x, y, button ? "Pressionado" : "Solto");
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
