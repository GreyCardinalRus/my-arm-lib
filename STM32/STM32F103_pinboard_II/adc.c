/*

�2012 NEE
����� �� ����� ����� ��������������
USE FREELY

*/
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "adc.h"

// ��������� ������������ ���
void pbii_adc_RCC_Configuration(void)
{
  /* ADCCLK = PCLK2/4 */
  RCC_ADCCLKConfig(RCC_PCLK2_Div4); // ���������� �������� ������� ���
    
  /* Enable ADC1, ADC2, ADC3 and GPIOC clocks */
  // ���������� ������������ ADC1 � GPIOA
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
}

// ��������� ����� ��������� ����� A05, ��� ���������� ����
void pbii_adc_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure PA.05 (ADC Channel05) as analog inputs */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// ��������� ���������� ���������� ��� ������� ���������� ADC1_2_IRQHandler
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

// ��������� � ������ ��������� ��� � PBII
void pbii_adc_init(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  /* System clocks configuration ---------------------------------------------*/
  pbii_adc_RCC_Configuration();// ��������� ������������ ���������� ��� � PBII

  /* NVIC configuration ------------------------------------------------------*/
  pbii_adc_NVIC_Configuration();// ��������� ���������� ��� � PBII

  /* GPIO configuration ------------------------------------------------------*/
  pbii_adc_GPIO_Configuration();// ��������� ����� �� ��������� ���  � PBII

  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure); //��������� ��� � ����� ������������ 
                                      //�������������� �� ������ ������
  /* ADC1 regular channels configuration */ 
  // ��������� ���� ��� �� ����� �����
  ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_28Cycles5);    
  /* Enable ADC1 EOC interrupt */
  // ��������� ���������� �� ����� ��������������
  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
  /* Enable ADC1 */
  // ��������� ������ ���
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
  // ������ ��������� ���������� ���
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  // ��������� ����� ����������
  while(ADC_GetResetCalibrationStatus(ADC1));

  
  /* Start ADC1 calibration */
  // ������ ���������� ���������� ���
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  // ��������� ����� ����������
  while(ADC_GetCalibrationStatus(ADC1));

    /* Start ADC1 Software Conversion */ 
  // ������ ������� �������������� ���
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

}

volatile int32_t ADC1accu=0; // �����������, � ������� ������������� ��������� 
                             //���������
volatile uint16_t pbii_adc_rslt_rdy=0;// ������� ���������� ����������, �� �� 
                                      // ���� ������� ��������� ���������� 
                                      // �����������,
                                      // ���� ���� ��������� �������� 256, �� 
                                      // ��������� ��� ��������� ���� 
                                      // �������������� ����������,
                                      // ��� ������ ���������� ����� ���������� 
                                      // ���������� �������� ����������� � ����



// ���������� �� ����� �������������� ���
void ADC1_2_IRQHandler(void)
{
  /* Get ADC1 converted value */
  uint16_t tmprslt = ADC_GetConversionValue(ADC1);// ������� ��������� 
                                                  //�������������� �� ��������� 
                                                  //����������
  if(pbii_adc_rslt_rdy < 256)         // ���� ���� ���������� �� ����������
  {
    ADC1accu += tmprslt;              // �������� ��������� ��������� ���������
    pbii_adc_rslt_rdy++;              // �������� 1�� � ������� ����������
  }// �����, ������� ��������� ��������� ������ �� ���...
}

