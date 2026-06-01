#include "debug.h"

/*
===========================================================
FUNÇÃO: PWM_Init()

Objetivo:
Configurar o TIM1 para gerar um sinal PWM no pino PC4.

Fluxo:
TIM1 -> PWM -> PC4 -> LED

O valor escrito no registrador Compare4 determinará
o Duty Cycle do PWM.

Duty Cycle:
0     = 0%
500   = 50%
1000  = 100%
===========================================================
*/
void PWM_Init(void)
{
    // Estrutura usada para configurar GPIOs
    GPIO_InitTypeDef GPIO_InitStructure;

    // Estrutura usada para configurar a base de tempo do Timer
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    // Estrutura usada para configurar o canal PWM
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /*
    -------------------------------------------------------
    HABILITAÇÃO DOS CLOCKS
    -------------------------------------------------------

    RCC = Reset and Clock Control

    Antes de usar qualquer periférico é necessário
    habilitar seu clock.

    GPIOC -> Porta do pino PC4
    TIM1  -> Timer que gerará o PWM
    */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOC |
        RCC_APB2Periph_TIM1,
        ENABLE
    );

    /*
    -------------------------------------------------------
    CONFIGURAÇÃO DO PINO PC4
    -------------------------------------------------------

    PC4 será controlado pelo Timer.

    GPIO_Mode_AF_PP significa:

    AF = Alternate Function
         O pino deixa de ser controlado pela CPU
         e passa a ser controlado pelo TIM1.

    PP = Push Pull
         Pode fornecer 0V ou 3.3V.
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /*
    -------------------------------------------------------
    CONFIGURAÇÃO DA BASE DE TEMPO
    -------------------------------------------------------

    Clock do microcontrolador = 48 MHz

    Prescaler = 48

    48 MHz / 48 = 1 MHz

    O contador agora incrementa
    1.000.000 vezes por segundo.

    Period = 1000

    Frequência PWM:

    Fpwm = 1MHz / 1000

    Fpwm = 1000 Hz (1 kHz)

    O contador executará:

    0
    1
    2
    ...
    999

    e depois reinicia.
    */
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 48 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    /*
    -------------------------------------------------------
    CONFIGURAÇÃO DO PWM
    -------------------------------------------------------

    PWM1 funciona assim:

    Se contador < Compare:
        saída = 1

    Se contador >= Compare:
        saída = 0

    Exemplo:

    Compare = 250

    contador 0 até 249:
        saída = 1

    contador 250 até 999:
        saída = 0

    Duty = 250 / 1000 = 25%
    */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;

    // Habilita o canal de saída do PWM
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

    /*
    Compare inicial.

    Como vale zero:

    contador < 0

    nunca será verdadeiro.

    Resultado:

    Duty Cycle = 0%
    LED apagado.
    */
    TIM_OCInitStructure.TIM_Pulse = 0;

    // PWM ativo em nível alto
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    /*
    Configura o Canal 4 do Timer 1.

    TIM1_CH4 -> PC4
    */
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);

    /*
    Preload evita alterações bruscas.

    Novos valores escritos em Compare4
    só entram em vigor no próximo período.
    */
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

    /*
    Habilita preload do ARR
    (Auto Reload Register)
    */
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    // Liga o Timer
    TIM_Cmd(TIM1, ENABLE);

    /*
    TIM1 é um timer avançado.

    Mesmo ligado, o PWM não sai no pino
    sem este comando.
    */
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

/*
===========================================================
FUNÇÃO: ADC_Init_Custom()

Objetivo:
Configurar o ADC1 para ler continuamente o pino PD2.

PD2 -> ADC1 -> Valor Digital (0 a 1023)
===========================================================
*/
void ADC_Init_Custom(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    /*
    Habilita os clocks necessários.

    GPIOD -> porta onde está PD2
    ADC1  -> conversor analógico digital
    */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOD |
        RCC_APB2Periph_ADC1,
        ENABLE
    );

    /*
    Configura PD2 como entrada analógica.

    Nesse modo os circuitos digitais do pino
    são desligados para reduzir ruído.
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /*
    Reinicializa o ADC.

    Todos os registradores voltam para
    o estado padrão de fábrica.
    */
    ADC_DeInit(ADC1);

    /*
    ADC independente.

    Não utiliza outros ADCs.
    */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;

    /*
    Não faz varredura de múltiplos canais.

    Apenas um canal será utilizado.
    */
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;

    /*
    Conversão contínua.

    Quando termina uma leitura,
    inicia outra automaticamente.
    */
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;

    /*
    Conversão iniciada por software.

    Não usa timer nem trigger externo.
    */
    ADC_InitStructure.ADC_ExternalTrigConv =
        ADC_ExternalTrigConv_None;

    /*
    Resultado alinhado à direita.

    Exemplo:

    0000001111111111
    */
    ADC_InitStructure.ADC_DataAlign =
        ADC_DataAlign_Right;

    /*
    Apenas um canal será convertido.
    */
    ADC_InitStructure.ADC_NbrOfChannel = 1;

    ADC_Init(ADC1, &ADC_InitStructure);

    /*
    Canal 3 corresponde ao pino PD2.

    Rank = 1
    (primeiro da fila)

    SampleTime = 241 ciclos

    Leitura mais lenta porém mais estável.
    */
    ADC_RegularChannelConfig(
        ADC1,
        ADC_Channel_3,
        1,
        ADC_SampleTime_241Cycles
    );

    // Liga o ADC
    ADC_Cmd(ADC1, ENABLE);

    /*
    CALIBRAÇÃO

    Corrige erros internos do ADC.

    Muito importante para obter
    leituras precisas.
    */

    ADC_ResetCalibration(ADC1);

    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);

    while(ADC_GetCalibrationStatus(ADC1));

    /*
    Inicia conversões contínuas.
    */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

int main(void)
{
    /*
    Variável que armazenará
    o valor convertido pelo ADC.

    ADC de 10 bits:

    mínimo = 0
    máximo = 1023
    */
    uint16_t adc_value;

    // Configura clock do sistema
    SystemInit();

    // Inicializa biblioteca de delay
    Delay_Init();

    // Configura PWM no PC4
    PWM_Init();

    // Configura ADC no PD2
    ADC_Init_Custom();

    /*
    =======================================================
    LOOP PRINCIPAL
    =======================================================

    O programa ficará repetindo:

    1) Ler ADC
    2) Atualizar PWM
    3) Repetir
    */
    while(1)
    {
        /*
        EOC = End Of Conversion

        Verifica se uma conversão terminou.
        */
        if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        {
            /*
            Lê resultado do ADC.

            Exemplos:

            0V    -> 0
            1.65V -> ~512
            3.3V  -> 1023
            */
            adc_value = ADC_GetConversionValue(ADC1);

            /*
            Atualiza o registrador Compare4.

            Compare controla o Duty Cycle.

            Exemplos:

            adc_value = 100
            Duty = 10%

            adc_value = 500
            Duty = 50%

            adc_value = 900
            Duty = 90%

            adc_value = 1023
            Duty ≈ 100%

            Resultado:

            Quanto maior a tensão em PD2,
            maior o brilho do LED em PC4.
            */
            TIM_SetCompare4(TIM1, adc_value);
        }

        // Pequeno atraso para estabilidade
        Delay_Ms(10);
    }
}