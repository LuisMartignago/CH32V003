#include "debug.h"

void PWM_Init(void)
{
    // Estruturas de configuração da biblioteca SPL (Standard Peripheral Library)
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 1. Habilita o clock para a Porta C (onde está o pino) e para o Timer 1
    // Sem isso, os periféricos ficam "desligados" para economizar energia
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1, ENABLE);

    // 2. Configura o Pino PC4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // "Alternate Function Push-Pull" (Necessário para o Timer assumir o pino)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 3. Configura a Base de Tempo do Timer (Frequência do PWM)
    // Frequência = Fclock / (Prescaler * Period)
    // 48MHz / (48 * 1000) = 1kHz (Frequência clássica para LEDs)
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;       // Valor máximo do contador (0 a 999)
    TIM_TimeBaseStructure.TIM_Prescaler = 48 - 1;      // Divide o clock de 48MHz por 48
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // 4. Configura o Canal do PWM (Compare Mode)
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  // Modo 1: Ativo enquanto contador < CompareValue
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;                 // Come?a com Duty Cycle em 0%
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    // Inicializa o Canal 4 (específico do pino PC4 no CH32V003)
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);
    
    // Habilita o pré-carregamento do registro de compara??o (estabilidade)
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

    // 5. Ativa o Timer e as saídas
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
    
    // IMPORTANTE: O TIM1 é um timer avan?ado. Precisa desse comando para "liberar" o PWM nos pinos
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

void ADC_Init_Custom(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    // Habilita clock do Porto D e do Conversor ADC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1, ENABLE);

    // Configura PD2 como Entrada Analógica (AIN)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // Reset de segurança no periférico ADC
    // Ela coloca o ADC1 de volta ao estado padrão de fábrica.
    ADC_DeInit(ADC1);

    // Configuração do ADC
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;       // Apenas 1 pino, então não precisa escanear vários
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  // Fica lendo sem parar automaticamente
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // Início via software
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // Valor alinhado à direita (padr?o)
    ADC_InitStructure.ADC_NbrOfChannel = 1;             // Quantidade de canais usados
    ADC_Init(ADC1, &ADC_InitStructure);

    // Define que o Canal 3 (PD2) será o primeiro (e único) da fila de leitura
    // ADC_SampleTime define quão "rapido" é a leitura. 241 ciclos é o mais estável.
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_241Cycles);

    // Liga o ADC
    ADC_Cmd(ADC1, ENABLE);

    // Calibração interna (Essencial para precisão no CH32)
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    // Manda o ADC começar a converter
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

int main(void)
{
    uint16_t adc_value;

    SystemInit();      // Configura os clocks internos do chip
    Delay_Init();      // Prepara as funções de Delay
    PWM_Init();        // Configura o pino de saída (LED)
    ADC_Init_Custom(); // Configura o pino de entrada (Potenciômetro)

    while(1)
    {
        // Verifica se o ADC terminou a conversão
        if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) 
        {
            // Lê o valor (No CH32V003, o valor vai de 0 a 1023) 10bits
            adc_value = ADC_GetConversionValue(ADC1);

            // Atualiza o Duty Cycle do Timer
            // Como o ADC vai até 1023 e o nosso Período do Timer é 1000,
            // os valores são quase idênticos. O LED brilhará proporcionalmente.
            TIM_SetCompare4(TIM1, adc_value); 
        }
        
        Delay_Ms(10); // Pequena pausa para estabilidade
    }
}