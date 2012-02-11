/*

�2012 NEE
����� �� ����� ����� ��������������
USE FREELY

*/
#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "adc.h"
#include "pwm.h"
#include "serial.h"




// ������������ ������������� ������� ����� B00 B01
// ������ ������������� �� ���� � ��������� � �������

void enc_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Enable the GPIO_LCD Clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);
}

// ������������ ������������� ������� ����� A01 A02 A03
// ������ ������������� �� ���� � ��������� � �������

void butt_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Enable the GPIO_LCD Clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3);

}
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
extern volatile uint32_t d_timer;

int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     



// ������������� ���������� 1000Hz ��� 1ms ������
  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(SystemCoreClock / 1000))
  { 
    /* Capture error */ 
    while (1);
  }
  lcd_init();                         // ������������� ���������� LCD
  clrscr();                           // ������� ������
  lcd_cursor_off();                   // ��������� ������� �������
  butt_init();                        // ������������� ������� ������
  enc_init();                         // ������������� ������� ��������
  wr_lcd_str(0,0,"Hello from PBII!"); // ����� �����������, ����������� API lcd.h
  
  pbii_adc_init();                    // ������������� ���������� LCD
  pbii_PWM_TIM3_Init();               // ������������� ���������� ��� �����������
  pbii_USART_init(115200);            // ������������� ���������� ����������������� �����-������
  
  /* Infinite loop */
  while (1)
  {
    static int32_t ADC1old_rslt=0;    // � ���� ���������� �������� ��������� �����������
                                      // �������������� ADC, ���������� � ��������� �������
                                      // ���������� ���������� � ���������� ADC �� ������
    if(pbii_adc_rslt_rdy==256)        // ������������ �������� ����� � ���������� ������������
    {                                 // ����������, 256 - ��������� 256 ���������, ����� ������������
      char lcd_out_str[16];
      int32_t tmpval=ADC1accu/(256*16); // ������� ���������� � ����������� ����������� 
                                        // ����������� � ���������� � 8�� ������� ����������
      if(ADC1old_rslt!=tmpval)          // ���� ���������� ��������� �� ��������� 
                                        // � ������� ����������� ��������� 
      {
        ADC1old_rslt=tmpval;            // �� ���������� ��������� ���������� �������
        sprintf(lcd_out_str,"    ADC=%03d     ",ADC1old_rslt);
        setcursor(1,0);                 // ����� �������� ���������� ADC �����
        wr_lcd_num_chr(lcd_out_str,16);
        pbii_sio0_TX_put(ADC1old_rslt); // ����� �������� ���������� ADC �� 
                                        // ���������������� ���������
        d_timer=3000;                   // ��������� ����� ����� �� 3000 ms 
                                        // ������� ���������� d_timer ������������
                                        // ��� ������ ������� ��������� �����
      }
      ADC1accu=0;                       // ����� ������������ ����������� �������� 
                                        // ��������� ADC 
      pbii_adc_rslt_rdy=0;              // ����� ���������� ���������� � ������������� 
                                        // ���������� ������ ��������� ����� ����������
    }
    if(0!=pbii_sio0_RX_rdy())           // ������ �� ����������������� ���������� 
                                        // ������?
     {                                  // ��
      static uint8_t outchars[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};// ����� 
                                        // ��� �������� ��������� �� ����� �������� 
                                        // ��������� �� ����������������� ����������
      memmove(&outchars[1],&outchars[0],15); // ���������� ����� ��� ����� ���������� 
                                             // �������
      outchars[0]=pbii_sio0_RX_get();   // ��������� ��������� ������ � ������� ���� 
                                        // ������� 
      setcursor(1,0);
      wr_lcd_num_chr((char *)outchars,16); // ����� �������� ������ �� �����
      d_timer=3000;                        // ��������� ����� ����� �� 3000 ms 

    }
extern volatile uint8_t k1_pressed;        // ������� ����� ������� ������ 
                                           // � � ��������
extern volatile uint8_t k1_depressed;
extern volatile uint8_t k2_pressed;
extern volatile uint8_t k2_depressed;
extern volatile uint8_t k3_pressed;
extern volatile uint8_t k3_depressed;
extern volatile uint8_t one_sec_flg;       // �����. ���� �������������� ������ 
                                           // �������
extern volatile int16_t position;          // ������� ������� ��������
extern volatile uint8_t encoder_changed;   // �����. ���� �������������� ��� 
                                           // ��������� ������� ��������



extern uint32_t gettime(void);             // �����. �����. ��������� �������� 
                                           // ���������� �������
    if(0!=k1_pressed)                      // ������� ������� ������ 1 ����?
    {                                      // �� 
      k1_pressed=0;
      setcursor(1,0);
      wr_lcd_num_chr(" KEY #1 PRESSED ",16); // ���������� ������� �� ������
      d_timer=3000;                          // ��������� ����� ����� �� 3000 ms 
    }
    if(0!=k1_depressed)                      // ������� ������� ������ 1 ����?
    {                                        // ���������� �������
      k1_depressed=0;
      setcursor(1,0);
      wr_lcd_num_chr("KEY #1 DEPRESSED",16);
      d_timer=3000;
    }
    if(0!=k2_pressed)                        // // ���������� ��. 1
    {
      k2_pressed=0;
      setcursor(1,0);
      wr_lcd_num_chr(" KEY #2 PRESSED ",16);
      d_timer=3000;
    }
    if(0!=k2_depressed)
    {
      k2_depressed=0;
      setcursor(1,0);
      wr_lcd_num_chr("KEY #2 DEPRESSED",16);
      d_timer=3000;
    }
    if(0!=k3_pressed)
    {
      k3_pressed=0;                             // ���������� ��. 1
      setcursor(1,0);
      wr_lcd_num_chr(" KEY #3 PRESSED ",16);
      d_timer=3000;
    }
    if(0!=k3_depressed)
    {
      k3_depressed=0;
      setcursor(1,0);
      wr_lcd_num_chr("KEY #3 DEPRESSED",16);
      d_timer=3000;
    }
    {
      if(encoder_changed!=0)                    // ��������� �������� ����������?
      {                                         // ��
                                                // �
        char str[16];                           
        encoder_changed=0;                      // ����� ����� ��������� ��������� 
                                                // ��������
        sprintf(str,"   ENC=%06hd   ",position);// ������� ��������� ��������
                                                // �� �����
        setcursor(1,0);
        wr_lcd_num_chr(str,16);
        d_timer=3000;                           // ��������� ����� ����� �� 3000 ms 
      }                   
    }
    {
      uint32_t lc_d_timer=d_timer;             // �������� ��������� ����� 
                                               // �������� ������� ������ ����� 
                                               // �� �����
      uint8_t  lc_one_sec_flg=one_sec_flg;     // �������� ��������� �����     
                                               // ����� ��������� ������ �����
                                               // �� �����                     


      if((lc_d_timer==0)&&(lc_one_sec_flg!=0)) // ������� ������ ����� ��� � 
                                               // ���������� ���������� �����?
      {                                        // ��
        uint32_t l_time=(gettime()/1000)%86400;// �������� ����� ������� ����� 
                                               // ���������� � ��������
        char str[16];
        one_sec_flg=0;                         // ����� ������� ���������� �����
        sprintf(str,"    %02d:%02d:%02d    ",  // ������� ���������� ������� �� �����
                l_time/3600,                   // ���� ������� ������� �����
                (l_time%3600)/60,              // ������ ������� ������� �����
                l_time%60                      // ������� ������� ������� �����
               );
        setcursor(1,0);                        // ������� �� �����
        wr_lcd_num_chr(str,16);
      }
    }
  }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

