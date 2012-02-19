/*

©2012 NEE
ОТКАЗ ОТ ЛЮБЫХ ВИДОВ ОТВЕСТВЕННОСТИ
USE FREELY

*/
#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "adc.h"
#include "pwm.h"
#include "serial.h"




// подпрограмма инициализации выводов порта B00 B01
// выводы настраиваются на ввод с подтяжкой к питанию

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

// подпрограмма инициализации выводов порта A01 A02 A03
// выводы настраиваются на ввод с подтяжкой к питанию

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



// Инициализация прерывания 1000Hz или 1ms период
  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(SystemCoreClock / 1000))
  { 
    /* Capture error */ 
    while (1);
  }
  lcd_init();                         // инициализация подсистемы LCD
  clrscr();                           // очистка экрана
  lcd_cursor_off();                   // отключить мигание курсора
  butt_init();                        // инициализация выводов кнопок
  enc_init();                         // инициализация выводов энкодера
  wr_lcd_str(0,0,"Hello from PBII!"); // вывод приветствия, ипользуется API lcd.h
  
  pbii_adc_init();                    // инициализация подсистемы LCD
  pbii_PWM_TIM3_Init();               // инициализация подсистемы ШИМ светодиодов
  pbii_USART_init(115200);            // инициализация подсистемы последовательного ввода-вывода
  
  /* Infinite loop */
  while (1)
  {
    static int32_t ADC1old_rslt=0;    // в этой переменной хранится результат предыдущего
                                      // преобразования ADC, учавствует в выработке события
                                      // обновления информации о результате ADC на экране
    if(pbii_adc_rslt_rdy==256)        // тестирование внешнего флага о готовности накопленного
    {                                 // результата, 256 - накоплено 256 измерений, можно обрабатывать
      char lcd_out_str[16];
      int32_t tmpval=ADC1accu/(256*16); // внешняя переменная с накопленным результатом
                                        // усредняется и приводится к 8ми битному результату
      if(ADC1old_rslt!=tmpval)          // если предыдущий результат не совпадает
                                        // с текущим результатом измерения
      {
        ADC1old_rslt=tmpval;            // то предыдущий результат заменяется текущим
        sprintf(lcd_out_str,"    ADC=%03d     ",ADC1old_rslt);
        setcursor(1,0);                 // вывод текущего результата ADC экран
        wr_lcd_num_chr(lcd_out_str,16);
        pbii_sio0_TX_put(ADC1old_rslt); // вывод текущего результата ADC на
                                        // последовательный интерфейс
        d_timer=3000;                   // запретить показ часов на 3000 ms
                                        // внешняя переменная d_timer используется
                                        // как таймер запрета показаний часов
      }
      ADC1accu=0;                       // сброс аккумулятора накопленных значений
                                        // измерений ADC
      pbii_adc_rslt_rdy=0;              // сброс количества накоплений и одновременное
                                        // разрешение начала следующей серии накоплений
    }
    if(0!=pbii_sio0_RX_rdy())           // символ по последовательному интерфейсу
                                        // пришел?
     {                                  // да
      static uint8_t outchars[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};// буфер
                                        // для хранения выводимых на экран символов
                                        // пришедших по последовательному интерфейсу
      memmove(&outchars[1],&outchars[0],15); // освободить место для вновь пришедшего
                                             // символа
      outchars[0]=pbii_sio0_RX_get();   // поместить пришедший символ в нулевой байт
                                        // буффера
      setcursor(1,0);
      wr_lcd_num_chr((char *)outchars,16); // вывод текущего буфера на экран
      d_timer=3000;                        // запретить показ часов на 3000 ms

    }
extern volatile uint8_t k1_pressed;        // внешние флаги событий кнопок
                                           // и и энкодера
extern volatile uint8_t k1_depressed;
extern volatile uint8_t k2_pressed;
extern volatile uint8_t k2_depressed;
extern volatile uint8_t k3_pressed;
extern volatile uint8_t k3_depressed;
extern volatile uint8_t one_sec_flg;       // внешн. флаг активизируется каждую
                                           // секунду
extern volatile int16_t position;          // текущая позиция энкодера
extern volatile uint8_t encoder_changed;   // внешн. флаг активизируется при
                                           // изменении позиции энкодера



extern uint32_t gettime(void);             // внешн. подпр. получения текущего
                                           // системного времени
    if(0!=k1_pressed)                      // событие нажатия кнопки 1 было?
    {                                      // да
      k1_pressed=0;
      setcursor(1,0);
      wr_lcd_num_chr(" KEY #1 PRESSED ",16); // отобразить событие на экране
      d_timer=3000;                          // запретить показ часов на 3000 ms
    }
    if(0!=k1_depressed)                      // событие отжатия кнопки 1 было?
    {                                        // аналогично нажатию
      k1_depressed=0;
      setcursor(1,0);
      wr_lcd_num_chr("KEY #1 DEPRESSED",16);
      d_timer=3000;
    }
    if(0!=k2_pressed)                        // // аналогично кн. 1
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
      k3_pressed=0;                             // аналогично кн. 1
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
      if(encoder_changed!=0)                    // состояние энкодера изменидось?
      {                                         // да
                                                // а
        char str[16];                           
        encoder_changed=0;                      // сброс флага изменения состояния
                                                // энкодера
        sprintf(str,"   ENC=%06hd   ",position);// вывести состояние энкодера
                                                // на экран
        setcursor(1,0);
        wr_lcd_num_chr(str,16);
        d_timer=3000;                           // запретить показ часов на 3000 ms
      }                   
    }
    {
      uint32_t lc_d_timer=d_timer;             // создание локальной копии
                                               // счетчика запрета вывода часов
                                               // на экран
      uint8_t  lc_one_sec_flg=one_sec_flg;     // создание локальной копии
                                               // флага обработки вывода часов
                                               // на экран


      if((lc_d_timer==0)&&(lc_one_sec_flg!=0)) // запрета вывода часов нет и
                                               // необходимо обновление часов?
      {                                        // да
        uint32_t l_time=(gettime()/1000)%86400;// получить время текущих суток
                                               // выраженное в секундах
        char str[16];
        one_sec_flg=0;                         // сброс запроса обновления часов
        sprintf(str,"    %02d:%02d:%02d    ",  // вывести содержимое времени на экран
                l_time/3600,                   // часы времени текущих суток
                (l_time%3600)/60,              // минуты времени текущих суток
                l_time%60                      // секунды времени текущих суток
               );
        setcursor(1,0);                        // вывести на экран
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

