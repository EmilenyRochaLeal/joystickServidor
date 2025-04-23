#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "joystick.h"
#include "wifi.h"
#include "queue.h" 

QueueHandle_t xJoystickQueue;

int main() {
    stdio_init_all();
    sleep_ms(3000);

    // Criação das tarefas
    printf("[Main] Criando tarefas...\n");

    xJoystickQueue = xQueueCreate(1, sizeof(JoystickData));  // Apenas 1 item (o mais recente)
    if (xJoystickQueue == NULL) {
        printf("Erro ao criar fila do joystick!\n");
    }

    BaseType_t result = xTaskCreate(wifi_task, "WiFiTask", 2048, NULL, 2, NULL);
    if (result != pdPASS) {
        printf("[Main] Falha ao criar a wifi_task! Código: %ld\n", result);
    }
         
    xTaskCreate(joystick_task, "JoystickTask", 256, NULL, 1, NULL);

    // Inicia o agendador do FreeRTOS
    vTaskStartScheduler();

    while (true);  // Nunca atinge
    return 0;
}
