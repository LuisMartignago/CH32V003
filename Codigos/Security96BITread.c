#include "debug.h"   // Biblioteca principal do CH32V003 + debug
#include <stdio.h>   // Usado para sprintf

// ==========================================================
// Configuração da UART
// ==========================================================
void UART_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;   // Estrutura para configurar pinos
    USART_InitTypeDef USART_InitStructure; // Estrutura para configurar UART

    // Habilita clock da porta D e da USART1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

    // -------- Configura PD5 como TX --------
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;          // Seleciona o pino PD5
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    // Função alternativa push-pull (UART)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // Velocidade do pino
    GPIO_Init(GPIOD, &GPIO_InitStructure);             // Aplica configuração

    // -------- Configura parâmetros da UART --------
    USART_InitStructure.USART_BaudRate = 115200;       // Velocidade de comunicação (baud rate)
    USART_InitStructure.USART_Mode = USART_Mode_Tx;    // Apenas transmissão

    USART_Init(USART1, &USART_InitStructure);          // Aplica configuração
    USART_Cmd(USART1, ENABLE);                         // Habilita a UART
}

// ==========================================================
// Envia 1 caractere pela UART
// ==========================================================
void UART_SendChar(char c)
{
    // Aguarda o registrador de transmissão ficar vazio
    while(!(USART1->STATR & USART_FLAG_TXE));

    USART1->DATAR = c;  // Envia o caractere
}

// ==========================================================
// Envia uma string inteira pela UART
// ==========================================================
void UART_SendString(char *str)
{
    // Percorre cada caractere até encontrar o terminador '\0'
    while(*str)
    {
        UART_SendChar(*str++);  // Envia o caractere e avança o ponteiro
    }
}

// ==========================================================
// Lê o UID de 96 bits e envia pela UART
// ==========================================================
void Send_UID(void)
{
    // Lê os 3 registradores de 32 bits (total de 96 bits)
    uint32_t uid0 = *(uint32_t*)0x1FFFF7E8;
    uint32_t uid1 = *(uint32_t*)0x1FFFF7EC;
    uint32_t uid2 = *(uint32_t*)0x1FFFF7F0;

    char buffer[50]; // Buffer para armazenar a string

    // Converte os 3 valores para uma string hexadecimal contínua
    sprintf(buffer, "%08X%08X%08X\r\n", uid0, uid1, uid2);

    // Envia a string pela UART
    UART_SendString(buffer);
}

// ==========================================================
// Função principal
// ==========================================================
int main(void)
{
    SystemInit();   // Inicializa o clock do sistema

    UART_Config();  // Configura a UART

    while(1)
    {
        Send_UID(); // Envia o UID continuamente

        // Delay simples (evita envio excessivo na UART)
        Delay_Ms(10000);
    }
}