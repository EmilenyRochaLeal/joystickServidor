#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "joystick.h"
#include "wifi.h"

int main() {
    stdio_init_all();
    sleep_ms(3000);

    // Criação das tarefas
    printf("[Main] Criando tarefas...\n");

    BaseType_t result = xTaskCreate(wifi_task, "WiFiTask", 2048, NULL, 2, NULL);
    if (result != pdPASS) {
        printf("[Main] Falha ao criar a wifi_task! Código: %ld\n", result);
    }
         
    // xTaskCreate(joystick_task, "JoystickTask", 256, NULL, 1, NULL);

    // Inicia o agendador do FreeRTOS
    vTaskStartScheduler();

    while (true);  // Nunca atinge
    return 0;
}

// xTaskCreate(wifi_task, "WiFiTask", 1024, NULL, 2, NULL);
// // if (xTaskCreate(wifi_task, "WiFiTask", 1024, NULL, 2, NULL) != pdPASS) {
// //     printf("[Main] Falha ao criar wifi_task!\n");
// // } 