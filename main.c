#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "joystick.h"
#include "queue.h" 

#include "utils/send-data-to-server/send-data.h"
#include "utils/wifi-connection/wifi-connection.h"

QueueHandle_t xJoystickQueue;

int main() {
    stdio_init_all();
    init_wifi();
    sleep_ms(3000);

    // Criação das tarefas
    printf("[Main] Criando tarefas...\n");

    xJoystickQueue = xQueueCreate(1, sizeof(JoystickData));  // Apenas 1 item (o mais recente)
    if (xJoystickQueue == NULL) {
        printf("Erro ao criar fila do joystick!\n");
    }
         
    xTaskCreate(joystick_task, "JoystickTask", 256, NULL, 1, NULL);

    // Inicia o agendador do FreeRTOS
    vTaskStartScheduler();

    while (true);  // Nunca atinge
    return 0;
}

// int x = getJoystickX();
//     int y = getJoystickY();
//     bool botao = le_botao();

//     create_request(x, y, botao);

//     sleep_ms(1000);

