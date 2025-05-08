// Data: 2025
#include <stdio.h>           // Biblioteca padrão de entrada e saída
#include <stdlib.h>   
#include <string.h> 
#include "pico/stdlib.h"    // Biblioteca padrão do Raspberry Pi Pico
#include "hardware/adc.h"  // Biblioteca para manipulação do ADC no RP2040
#include "FreeRTOS.h"
#include "task.h"
#include "joystick.h"
#include "queue.h" 

#include "utils/send-data-to-server/send-data.h"

#define vRx_PIN 26   // Pino ligado ao eixo X (ADC0)
#define vRy_PIN 27  // Pino ligado ao eixo Y (ADC1)
#define SW 22      // Pino do botão (Digital)

// extern xQueueHandle xJoystickQueue;
extern QueueHandle_t xJoystickQueue;

void calcula_direcao(JoystickData *dado) {
    int centro = 2048;
    int delta = 500;  

    int dx = dado->x - centro;
    int dy = centro - dado->y;

    if (abs(dx) < delta && abs(dy) < delta) {
        strcpy(dado->direcao, "Centro");
    } else if (dx > delta && abs(dy) < delta) {
        strcpy(dado->direcao, "Norte");
    } else if (dx < -delta && abs(dy) < delta) {
        strcpy(dado->direcao, "Sul");
    } else if (abs(dx) < delta && dy < -delta) {
        strcpy(dado->direcao, "Leste");
    } else if (abs(dx) < delta && dy > delta) {
        strcpy(dado->direcao, "Oeste");
    } else if (dx > delta && dy > delta) {
        strcpy(dado->direcao, "Noroeste");
    } else if (dx > delta && dy < -delta) {
        strcpy(dado->direcao, "Nordeste");
    } else if (dx < -delta && dy > delta) {
        strcpy(dado->direcao, "Sudoeste");
    } else if (dx < -delta && dy < -delta) {
        strcpy(dado->direcao, "Sudeste");
    } else {
        strcpy(dado->direcao, "Indefinido");
    }
}


void joystick_task(void *params) {
    adc_init();
    adc_gpio_init(vRx_PIN);
    adc_gpio_init(vRy_PIN);
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);

    JoystickData data; 

    while (1) {
        adc_select_input(0); // Seleciona o canal ADC0 (vRx)
        uint16_t x = adc_read();
        data.x = adc_read();

        adc_select_input(1); // Seleciona o canal ADC1 (vRy)
        uint16_t y = adc_read();
        data.y = adc_read();

        bool button = gpio_get(SW) == 0;
        data.button = gpio_get(SW) == 0;

        // Calcula a direção com base nos valores
        calcula_direcao(&data);

        printf("X: %d | Y: %d | Botao: %s | Direção: %s\n",
            data.x, data.y, data.button ? "Pressionado" : "Solto", data.direcao);

        // Envia para a fila (substitui valor anterior)
        xQueueOverwrite(xJoystickQueue, &data);

        // create_request(data.x, data.y, data.button,  data.direcao);

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
