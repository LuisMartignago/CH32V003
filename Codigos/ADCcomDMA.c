#include "debug.h"   // Biblioteca principal do CH32 (periféricos)
#include <stdio.h>      // Usado para sprintf (formatação de string)

#define BUF_SIZE 8      // Tamanho do buffer do DMA (quantas amostras)

uint16_t adc_buffer[BUF_SIZE]; // Buffer onde o DMA vai jogar as leituras do ADC


// =========================
// GPIO
// =========================
void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Habilita clock da porta D (sem isso GPIO não funciona)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    // -------- ADC (entrada analógica) --------
    // PD2 será usado como entrada do ADC
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // modo analógico (sem digital)
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // -------- UART TX --------
    // PD5 será saída da UART
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // função alternativa (UART)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}


// =========================
// UART
// =========================
void UART_Config(void)
{
    USART_InitTypeDef USART_InitStructure;

    // Liga clock da UART
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // Configura UART
    USART_InitStructure.USART_BaudRate = 115200; // velocidade
    USART_InitStructure.USART_Mode = USART_Mode_Tx; // só transmissão

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE); // liga UART
}

// Função simples para enviar string pela UART
void UART_SendString(char *str)
{
    while(*str)
    {
        // espera buffer de transmissão vazio
        while(!(USART1->STATR & USART_FLAG_TXE));

        // envia caractere
        USART1->DATAR = *str++;
    }
}


// =========================
// DMA
// =========================
void DMA_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    // Liga clock do DMA
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // Reseta canal 1 (usado pelo ADC)
    DMA_DeInit(DMA1_Channel1);

    // Endereço de onde vem o dado (ADC)
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->RDATAR;

    // Para onde vai (nosso buffer)
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_buffer;

    // Direção: periférico -> memória
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;

    // Quantidade de dados no buffer
    DMA_InitStructure.DMA_BufferSize = BUF_SIZE;

    // Não incrementa endereço do ADC
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

    // Incrementa posição no buffer
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

    // Tamanho do dado (ADC = 16 bits)
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

    // Modo circular (quando enche, volta pro início)
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;

    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    // Limpa flag de transferência
    DMA_ClearFlag(DMA1_FLAG_TC1);

    // Liga o DMA
    DMA_Cmd(DMA1_Channel1, ENABLE);
}


// =========================
// ADC
// =========================
void ADC_Config(void)
{
    ADC_InitTypeDef ADC_InitStructure;

    // Liga clock do ADC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    ADC_DeInit(ADC1); // reset do ADC

    // Configuração básica
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE; // só 1 canal
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; // conversão contínua
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // sem trigger externo
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // alinhado à direita
    ADC_InitStructure.ADC_NbrOfChannel = 1;

    ADC_Init(ADC1, &ADC_InitStructure);

    // Canal 3 = PD2
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_241Cycles);

    //  Liga o DMA no ADC
    ADC_DMACmd(ADC1, ENABLE);

    // Liga ADC
    ADC_Cmd(ADC1, ENABLE);

    // -------- Calibração --------
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    // Começa conversão contínua
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


// =========================
// MAIN
// =========================
int main(void)
{
    char buffer[50];

    // Inicializações
    GPIO_Config();
    UART_Config();
    DMA_Config();
    ADC_Config();

    // espera buffer encher (evita lixo inicial)
    for(volatile int i=0;i<500000;i++);

    while(1)
    {
        uint32_t soma = 0;

        //  Copia buffer (evita conflito com DMA escrevendo)
        uint16_t copia[BUF_SIZE];
        for(int i=0;i<BUF_SIZE;i++)
            copia[i] = adc_buffer[i];

        // Soma valores
        for(int i=0;i<BUF_SIZE;i++)
            soma += copia[i];

        // Média (reduz ruído)
        uint16_t media = soma / BUF_SIZE;

        // Conversão para mV (ADC 10 bits → 0-1023)
        uint32_t tensao_mV = (media * 3300) / 1023;

        // Formata string
        sprintf(buffer, "ADC:%4d  T: %d.%03d V\r\n",
                media,
                tensao_mV / 1000,
                tensao_mV % 1000);

        // Envia pela UART
        UART_SendString(buffer);

        // Delay simples
        for(volatile int i=0;i<100000;i++);
    }
}