#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "FreeRTOS.h"
#include "task.h"
#include "wifi.h"
#include "joystick.h"
#include "queue.h" 

// extern xQueueHandle xJoystickQueue;
extern QueueHandle_t xJoystickQueue;


#define WIFI_SSID "MAMBEE"
#define WIFI_PASSWORD "1fp1mamb33"

// Callback de recebimento HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    printf("Request: %s\n", request);

    JoystickData dados_joystick;
    bool tem_dados = xQueuePeek(xJoystickQueue, &dados_joystick, 0);  // Lê sem remover

    char html[1024];
    snprintf(html, sizeof(html),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html><head><meta http-equiv='refresh' content='1'>"
        "<title>Joystick</title></head><body>"
        "<h1>Dados do Joystick</h1>"
        "<p>X: %d</p><p>Y: %d</p><p>Botão: %s</p>"
        "</body></html>",
        tem_dados ? dados_joystick.x : -1,
        tem_dados ? dados_joystick.y : -1,
        tem_dados ? (dados_joystick.button ? "Pressionado" : "Solto") : "Sem dados"
    );

    // Envia para o cliente
    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    // libera a memória
    free(request);
    pbuf_free(p);
    return ERR_OK;
}

// Callback de aceitação
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// Função para a tarefa FreeRTOS
void wifi_task(void *params) {
    printf("[WiFi Task] Iniciando tarefa\n");

    // Inicializa a arquitetura Wi-Fi
    if (cyw43_arch_init()) {
        printf("[WiFi Task] Erro ao iniciar Wi-Fi\n");
        vTaskDelete(NULL);
    }
    printf("[WiFi Task] Wi-Fi iniciado com sucesso\n");

    // Habilita modo Station (cliente)
    cyw43_arch_enable_sta_mode();
    printf("[WiFi Task] Modo STA habilitado\n");

    // Conecta à rede
    int result = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);

    if (result != 0) {
        printf("[WiFi Task] Falha ao conectar. Código: %d\n", result);
        vTaskDelete(NULL);
    }

    printf("[WiFi Task] Conectado com sucesso\n");

    // Mostra IP atribuído
    if (netif_default) {
        printf("[WiFi Task] IP: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    } else {
        printf("[WiFi Task] netif_default é NULL! IP não atribuído.\n");
    }

    // Inicia servidor HTTP
    struct tcp_pcb *server = tcp_new();
    if (!server || tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("[WiFi Task] Erro ao iniciar servidor HTTP\n");
        vTaskDelete(NULL);
    }

    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);
    printf("[WiFi Task] Servidor HTTP ouvindo na porta 80\n");

    // Loop principal da task
    while (1) {
        cyw43_arch_poll();  // Mantém Wi-Fi ativo
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
