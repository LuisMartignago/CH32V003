// ============================================================
// Modos de Entrada
// GPIO_Mode_IN_FLOATING -> Entrada flutuante (sem pull-up/down)
// GPIO_Mode_IPU         -> Entrada com pull-up interno
// GPIO_Mode_IPD         -> Entrada com pull-down interno
//
// Modos de Saída
// GPIO_Mode_Out_PP      -> Saída Push-Pull
// GPIO_Mode_Out_OD      -> Saída Open-Drain
// ============================================================

// Inclui as bibliotecas da SPL (Standard Peripheral Library)
// e funções auxiliares do fabricante.
#include "debug.h"

// Função principal do programa.
// A execução começa aqui após o reset do microcontrolador.
int main(void)
{
    // Cria uma estrutura que armazenará as configurações
    // dos pinos GPIO.
    GPIO_InitTypeDef GPIO_InitStructure;

    // ========================================================
    // HABILITAÇÃO DOS CLOCKS
    // ========================================================

    // Liga o clock das portas GPIOC e GPIOD.
    // Sem isso os registradores dessas portas não funcionam.
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,
        ENABLE
    );

    // ========================================================
    // CONFIGURAÇÃO DO PINO PC4 COMO SAÍDA
    // ========================================================

    // Seleciona o pino 4 da porta C.
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;

    // Configura o pino como saída Push-Pull.
    // O microcontrolador consegue fornecer 0 V ou 3,3 V.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    // Define velocidade máxima de chaveamento do pino.
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    // Aplica as configurações acima ao GPIOC.
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // ========================================================
    // CONFIGURAÇÃO DO PINO PD3 COMO ENTRADA
    // ========================================================

    // Seleciona o pino 3 da porta D.
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;

    // Configura como entrada com resistor pull-up interno.
    //
    // Internamente:
    //
    //      3.3V
    //        |
    //       [R]
    //        |
    //       PD3
    //
    // Sem pressionar o botão:
    // PD3 = 1
    //
    // Pressionando o botão:
    // PD3 = 0
    //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;

    // Aplica a configuração ao GPIOD.
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // ========================================================
    // LOOP PRINCIPAL
    // ========================================================

    // Executa para sempre.
    while(1)
    {
        // Lê o estado lógico do pino PD3.
        //
        // Retorna:
        // 0 -> nível baixo
        // 1 -> nível alto
        //
        if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3) == 0)
        {
            // =================================================
            // BOTÃO PRESSIONADO
            // =================================================
            //
            // Como existe pull-up interno:
            //
            // Solto       -> PD3 = 1
            // Pressionado -> PD3 = 0
            //
            // Quando PD3 = 0 esta condição é verdadeira.
            //

            // Coloca PC4 em nível alto (3,3 V).
            GPIO_SetBits(GPIOC, GPIO_Pin_4);

            // Equivalente conceitual:
            // PC4 = 1;
        }
        else
        {
            // =================================================
            // BOTÃO SOLTO
            // =================================================

            // Coloca PC4 em nível baixo (0 V).
            GPIO_ResetBits(GPIOC, GPIO_Pin_4);

            // Equivalente conceitual:
            // PC4 = 0;
        }
    }
}