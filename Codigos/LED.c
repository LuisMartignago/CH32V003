
// Entrada
// GPIO_Mode_IN_FLOATING → entrada sem pull
// GPIO_Mode_IPU → entrada com pull-up
// GPIO_Mode_IPD → entrada com pull-down


// Saída
//GPIO_Mode_Out_PP → saída push-pull (mais usado)
//GPIO_Mode_Out_OD → saída open-drain

#include "debug.h"

int main(void)
{
    // Estruturas de configuração da biblioteca SPL (Standard Peripheral Library)
    GPIO_InitTypeDef GPIO_InitStructure;

    // 1. Habilitar clock dos GPIOs (RCC-> Reset and Clock Control)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

    // ================================
    // 2. Configurar PC4 como saída
    // ================================
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // push-pull
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // ================================
    // 3. Configurar PD3 como entrada
    // ================================
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // pull-up interno
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // ================================
    // 4. Loop principal
    // ================================
    while(1)
    {
        // Ler botão
        if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3) == 0)
        {
            // botão pressionado
            GPIO_SetBits(GPIOC, GPIO_Pin_4);
        }
        else
        {
            // botão solto
            GPIO_ResetBits(GPIOC, GPIO_Pin_4);
        }
    }
}
// Entrada
// GPIO_Mode_IN_FLOATING → entrada sem pull
// GPIO_Mode_IPU → entrada com pull-up
// GPIO_Mode_IPD → entrada com pull-down


// Saída
//GPIO_Mode_Out_PP → saída push-pull (mais usado)
//GPIO_Mode_Out_OD → saída open-drain

#include "debug.h"

int main(void)
{
    // Estruturas de configuração da biblioteca SPL (Standard Peripheral Library)
    GPIO_InitTypeDef GPIO_InitStructure;

    // 1. Habilitar clock dos GPIOs (RCC-> Reset and Clock Control)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

    // ================================
    // 2. Configurar PC4 como saída
    // ================================
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // push-pull
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // ================================
    // 3. Configurar PD3 como entrada
    // ================================
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // pull-up interno
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // ================================
    // 4. Loop principal
    // ================================
    while(1)
    {
        // Ler botão
        if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3) == 0)
        {
            // botão pressionado
            GPIO_SetBits(GPIOC, GPIO_Pin_4);
        }
        else
        {
            // botão solto
            GPIO_ResetBits(GPIOC, GPIO_Pin_4);
        }
    }
}
