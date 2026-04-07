#include "ch32v00x.h"  // Biblioteca principal do CH32V003 (define registradores e perif¨¦ricos)

// ===== CONFIGURA??O DO PWM =====
#define PWM_MIN 0        // Duty cycle m¨ªnimo (0%)
#define PWM_MAX 1000     // Duty cycle m¨¢ximo (~100%)
#define PWM_PERIOD 1000  // Per¨ªodo do PWM (define resolu??o)

// ===== INICIALIZA??O DO PWM =====
void PWM_Init(void)
{
    // Habilita o clock da porta D (GPIO) e do Timer 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_TIM1, ENABLE);

    // Configura PD2 como sa¨ªda alternativa (PWM)
    // Esse pino ser¨¢ controlado pelo Timer (TIM1_CH1)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   // Modo fun??o alternativa push-pull
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz; // Velocidade de comuta??o
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // Configura??o base do Timer
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD - 1; // Valor m¨¢ximo da contagem
    TIM_TimeBaseStructure.TIM_Prescaler = 48 - 1;      // Divide clock de 48 MHz ¡ú 1 MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // Conta de 0 at¨¦ o per¨ªodo
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // Configura??o do canal PWM
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // Modo PWM
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // Ativa sa¨ªda
    TIM_OCInitStructure.TIM_Pulse = 0; // Duty inicial (0%)
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // Ativo em n¨ªvel alto

    // Aplica configura??o no canal 1 do Timer
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);

    // Liga o Timer
    TIM_Cmd(TIM1, ENABLE);

    // Necess¨¢rio para timers avan?ados (TIM1)
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

// ===== INICIALIZA??O DO ADC =====
void ADC_Init_Config(void)
{
    // Habilita clock da porta D e do ADC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1, ENABLE);

    // Configura PD4 como entrada anal¨®gica
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // Desativa digital ¡ú reduz ru¨ªdo
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // Configura??o do ADC
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // ADC independente
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;      // N?o usa m¨²ltiplos canais
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;// Convers?o manual
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // Sem trigger externo
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // Resultado alinhado ¨¤ direita
    ADC_InitStructure.ADC_NbrOfChannel = 1; // Apenas 1 canal

    ADC_Init(ADC1, &ADC_InitStructure);

    // Seleciona canal 4 (PD4) com tempo de amostragem
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_9Cycles);

    // Liga o ADC
    ADC_Cmd(ADC1, ENABLE);

    // ===== CALIBRA??O DO ADC =====
    // Remove erros internos do conversor
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

// ===== FUN??O DE LEITURA DO ADC =====
uint16_t ADC_Read(void)
{
    // Inicia convers?o
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // Espera at¨¦ convers?o terminar
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

    // Retorna valor convertido (0 a 4095 ¡ú 12 bits)
    return ADC_GetConversionValue(ADC1);
}

// ===== FUN??O PRINCIPAL =====
int main(void)
{
    SystemInit();        // Inicializa clock do sistema
    PWM_Init();          // Inicializa PWM
    ADC_Init_Config();   // Inicializa ADC

    uint16_t adc; // Valor bruto do ADC
    uint16_t pwm; // Valor convertido para PWM

    while(1)
    {
        // L¨º valor anal¨®gico (ex: potenci?metro)
        adc = ADC_Read();

        // ===== ZONA MORTA =====
        // Evita instabilidade perto de 0 e do m¨¢ximo
        if(adc < 100) adc = 100;
        if(adc > 4000) adc = 4000;

        // ===== CONVERS?O ADC ¡ú PWM =====
        // Mapeia 0¨C4095 ¡ú 0¨C1000 (PWM)
        pwm = PWM_MIN + ((uint32_t)adc * (PWM_MAX - PWM_MIN)) / 4095;

        // Atualiza duty cycle do PWM
        TIM_SetCompare1(TIM1, pwm);
    }
}