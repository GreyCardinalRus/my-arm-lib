/*

©2012 NEE
ОТКАЗ ОТ ЛЮБЫХ ВИДОВ ОТВЕСТВЕННОСТИ
USE FREELY

*/
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "adc.h"

// настройка клокирования АЦП
void pbii_adc_RCC_Configuration(void)
{
  /* ADCCLK = PCLK2/4 */
  RCC_ADCCLKConfig(RCC_PCLK2_Div4); // установака тактовой частоты АЦП
    
  /* Enable ADC1, ADC2, ADC3 and GPIOC clocks */
  // разрешение тактирования ADC1 и GPIOA
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
}

// настройка штыря аналогово ввода A05, как аналоговый вход
void pbii_adc_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure PA.05 (ADC Channel05) as analog inputs */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// настройка подсистемы прерываний для векторв прерываний ADC1_2_IRQHandler
void pbii_adc_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure and enable ADC interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

// настройка и запуск подистемы АЦП в PBII
void pbii_adc_init(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  /* System clocks configuration ---------------------------------------------*/
  pbii_adc_RCC_Configuration();// запустить тактирование подсистемы АЦП в PBII

  /* NVIC configuration ------------------------------------------------------*/
  pbii_adc_NVIC_Configuration();// настроить прерывания АЦП в PBII

  /* GPIO configuration ------------------------------------------------------*/
  pbii_adc_GPIO_Configuration();// настроить штыри ВВ подситемы АЦП  в PBII

  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure); //настроить АЦП в режим непрерывного
                                      //преобразования по одному каналу
  /* ADC1 regular channels configuration */ 
  // настроить вход АЦП на пятый канал
  ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_28Cycles5);    
  /* Enable ADC1 EOC interrupt */
  // разрешить прерывания по концу преобразования
  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
  /* Enable ADC1 */
  // разрешить работу АЦП
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
  // начать начальную калибровку АЦП
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  // дождаться конца калибровки
  while(ADC_GetResetCalibrationStatus(ADC1));

  
  /* Start ADC1 calibration */
  // начать нормальную калибровку АЦП
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  // дождаться конца калибровки
  while(ADC_GetCalibrationStatus(ADC1));

    /* Start ADC1 Software Conversion */ 
  // начать процесс преобразования АЦП
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

}

volatile int32_t ADC1accu=0; // аккумулятор, в котором накапливается результат
                             //измерений
volatile uint16_t pbii_adc_rslt_rdy=0;// счетчик количества накоплений, он же
                                      // флаг события окончания накопления
                                      // результатов,
                                      // если флаг достигает значения 256, то
                                      // считается что очередной цикл
                                      // преобразования закончился,
                                      // для начала следующего цикла накопления
                                      // необходимо обнулить аккумулятор и флаг



// прерывание по концу преобразования АЦП
void ADC1_2_IRQHandler(void)
{
  /* Get ADC1 converted value */
  uint16_t tmprslt = ADC_GetConversionValue(ADC1);// считать результат
                                                  //преобразования во временную
                                                  //переменную
  if(pbii_adc_rslt_rdy < 256)         // если цикл накопления не завершился
  {
    ADC1accu += tmprslt;              // накопить очередной результат измерения
    pbii_adc_rslt_rdy++;              // добавить 1цу в счетчик накоплений
  }// иначе, текущий результат измерений падает на пол...
}

