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

// HTML da página principal (com JavaScript dinâmico)
static const char *pagina_html =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<!DOCTYPE html><html><head><title>Joystick</title>"
        "<script>"
        "async function atualizarDados() {"
        "try {"
        "const res = await fetch('/data');"
        "const json = await res.json();"
        "document.getElementById('x').textContent = json.x;"
        "document.getElementById('y').textContent = json.y;"
        "document.getElementById('botao').textContent = json.botao;"
        "document.getElementById('direcao').textContent = json.direcao;"
        "} catch (e) { console.log('Erro:', e); }"
        "}"
        "setInterval(atualizarDados, 1000);"
        "</script></head><body>"
        "<h1>Joystick</h1>"
        "<p>X: <span id='x'>-</span></p>"
        "<p>Y: <span id='y'>-</span></p>"
        "<p>Botão: <span id='botao'>-</span></p>"
        "<p>Direção: <span id='direcao'>-</span></p>"
        "</body></html>";

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

    if (strncmp(request, "GET /", 9) == 0) {
        // Responder com JSON
        char resposta[256];
        snprintf(resposta, sizeof(resposta),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n\r\n"
            "{\"x\": %d, \"y\": %d, \"botao\": \"%s\", \"direcao\": \"%s\"}",
            tem_dados ? dados_joystick.x : -1,
            tem_dados ? dados_joystick.y : -1,
            tem_dados ? (dados_joystick.button ? "Pressionado" : "Solto") : "Desconhecido",
            tem_dados ? dados_joystick.direcao : "Sem dados"
        );
        tcp_write(tpcb, resposta, strlen(resposta), TCP_WRITE_FLAG_COPY);
    } else {
        // Responder com a página HTML
        tcp_write(tpcb, pagina_html, strlen(pagina_html), TCP_WRITE_FLAG_COPY);
    }

    

    // Envia para o cliente
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

void wifi_task(void *params) {
    printf("[WiFi Task] Iniciando tarefa\n");

    if (cyw43_arch_init()) {
        printf("[WiFi Task] Erro ao iniciar Wi-Fi\n");
        vTaskDelete(NULL);
    }

    sleep_ms(1000); // Mais tempo para garantir estabilidade
    printf("[WiFi Task] Wi-Fi iniciado com sucesso\n");

    cyw43_arch_enable_sta_mode();
    printf("[WiFi Task] Modo STA habilitado\n");

    const int max_retries = 5;
    int result = -1;
    for (int i = 0; i < max_retries; i++) {
        printf("[WiFi Task] Tentando conectar (%d/%d)...\n", i + 1, max_retries);
        result = cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 15000);
        if (result == 0) {
            printf("[WiFi Task] Conectado com sucesso!\n");
            break;
        } else {
            printf("[WiFi Task] Falha ao conectar (tentativa %d). Código: %d\n", i + 1, result);
            vTaskDelay(pdMS_TO_TICKS(2000)); // Espera 2s e tenta de novo
        }
    }

    if (result != 0) {
        printf("[WiFi Task] Não foi possível conectar após %d tentativas. Finalizando task.\n", max_retries);
        vTaskDelete(NULL);
    }

    if (netif_default) {
        printf("[WiFi Task] IP: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    } else {
        printf("[WiFi Task] netif_default é NULL! IP não atribuído.\n");
    }

    struct tcp_pcb *server = tcp_new();
    if (!server || tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("[WiFi Task] Erro ao iniciar servidor HTTP\n");
        vTaskDelete(NULL);
    }

    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);
    printf("[WiFi Task] Servidor HTTP ouvindo na porta 80\n");

    while (1) {
        cyw43_arch_poll();  // Mantém Wi-Fi ativo
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
