#include "debug.h"  // Biblioteca padrão (clock, delay, etc.)

/*
===========================================================
                CONTROLE PWM COM ADC + BOTÃO
===========================================================
- PC4  -> Saída PWM (TIM1_CH4)
- PD2  -> Entrada analógica (potenciômetro - ADC)
- PC0  -> Botão com interrupção (liga/desliga PWM)

FUNCIONAMENTO:
- O potenciômetro controla o duty cycle do PWM
- O botão alterna entre PWM ligado/desligado
- Quando desligado, o MCU entra em modo sleep
===========================================================
*/

// Variável global alterada na interrupção
volatile uint8_t pwm_enabled = 0;

/* =========================================================
                        PWM
========================================================= */
void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_Base;
    TIM_OCInitTypeDef TIM_OC;

    // Habilita clock GPIOC e TIM1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1, ENABLE);

    // PC4 como saída PWM (função alternativa)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /*
    CONFIGURAÇÃO DO TIMER:
    Clock = 48MHz
    Prescaler = 24 → 2MHz
    Período = 100 → 20kHz PWM
    */
    TIM_Base.TIM_Prescaler = 24 - 1;
    TIM_Base.TIM_Period = 100 - 1;
    TIM_Base.TIM_ClockDivision = 0;
    TIM_Base.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_Base);

    // Configura canal 4 (PC4)
    TIM_OC.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OC.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OC.TIM_Pulse = 0; // Duty inicial
    TIM_OC.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC4Init(TIM1, &TIM_OC);

    // Preload evita glitch
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    // Liga Timer
    TIM_Cmd(TIM1, ENABLE);

    // Necessário para TIM1 (advanced timer)
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

// Atualiza duty cycle (0 a 999)
void PWM_SetDuty(uint16_t duty)
{
    if(duty > 99) duty = 99; // corrigido
    TIM_SetCompare4(TIM1, duty);
}

/* =========================================================
                        ADC
========================================================= */
void ADC_Init_Custom(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    // Habilita clock GPIOD e ADC1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1, ENABLE);

    // PD2 como entrada analógica
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    ADC_DeInit(ADC1);

    // Configuração básica do ADC
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; // contínuo
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    // Canal 3 = PD2
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_241Cycles);

    ADC_Cmd(ADC1, ENABLE);

    // Calibração (obrigatória)
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    // Inicia conversão
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

// Lê valor do ADC (0 a 1023)
uint16_t ADC_Read(void)
{
    return ADC_GetConversionValue(ADC1);
}

/* =========================================================
                        BOTÃO (EXTI)
========================================================= */
void Button_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // Clock GPIOC + AFIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    // PC0 como entrada pull-up
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Conecta PC0 na EXTI0
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource0);

    // Configura interrupção
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

    // Dispara ao pressionar (nível vai para 0)
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // NVIC (habilita interrupção global)
    NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/* =========================================================
                    INTERRUPÇÃO
========================================================= */
void EXTI7_0_IRQHandler(void) __attribute__((interrupt));

void EXTI7_0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        // Alterna estado do PWM
        pwm_enabled = !pwm_enabled;

        // Limpa flag da interrupção
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

/* =========================================================
                        MAIN
========================================================= */
int main(void)
{
    uint16_t adc_value;

    SystemInit();   // Inicializa clock
    Delay_Init();   // Inicializa delay

    PWM_Init();
    ADC_Init_Custom();
    Button_Init();

    while(1)
    {
        if(pwm_enabled)
        {
            // Verifica fim da conversão ADC
            if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
            {
                adc_value = ADC_Read();

                // Ajusta PWM proporcional ao potenciômetro
                PWM_SetDuty((adc_value * 100) / 1023);
            }
        }
        else
        {
            // Desliga PWM
            PWM_SetDuty(0);

            // Entra em modo baixo consumo
            __WFI();
        }

        Delay_Ms(10); // Estabilização
    }
}