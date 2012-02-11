/*

©2012 NEE
ОТКАЗ ОТ ЛЮБЫХ ВИДОВ ОТВЕСТВЕННОСТИ
USE FREELY

*/

#include "stm32f10x_it.h"

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
union times___
{
  uint64_t time_ll;
  volatile uint32_t time_l; // системная переменная 
                            // обозначающая кол. миллисекунд прошедщих 
                            // с начала действия программы
  uint16_t time_s;
}times__={0ull};


uint16_t ocval1=333,ocval2=332; // состояние ШИМ светодиода LED2 и LED3
uint8_t u_d1=0,u_d2=1;          // направление изменения 0-> яркость уменьшается 
                                // 1 -> яркость увеличивается

volatile uint8_t k1_pressed=0;  // флаг события нажатия на кнопку №1 1-> кнопка 
                                // была нажата
volatile uint8_t k1_depressed=0;// флаг события отжатия кнопки №1 1-> кнопка
                                // была отжата                                
volatile uint8_t k2_pressed=0;  // аналогично кнопка №2
volatile uint8_t k2_depressed=0;
volatile uint8_t k3_pressed=0;  // аналогично кнопка №3
volatile uint8_t k3_depressed=0;

volatile uint32_t d_timer=0;    // флаг запрета обновления показаний времени 
                                // содержит количество миллисекунд до конца 
                                // запрета, уменьшается на единицу каждую 
                                // миллисекунду, пока не достигнет нуля
volatile uint8_t one_sec_flg=0; // флаг запроса обновления показаний времени 
                                // на экране, обновляется один раз в секунду

uint8_t pinValues[2] = {0,0};   // предыдущее состояние контактов энкодера
volatile int16_t position = 0;  // текущая позиция энкодера
volatile uint8_t encoder_changed = 0; // флаг изменения состояния энкодера 
                                      // 1->позиция энкодера изменилась 
// подпрограмма определения изменений состояния энкодера 
// взята из гугла по ключевым словам "arduino" "encoder"
// защита от дребезга обеспечивается дискретностью 
// обновления информации о состоянии контактов энкодера 
// 1ms
/**  * A simple incremental Rotary Encoder decoder example 
  *andrew@rocketnumbernine.com                                          *
  *http://www.rocketnumbernine.com/2010/03/06/decoding-a-rotary-encoder/ *                                                        
  * use freely */ 
void inline rotary_encoder_change(uint8_t changedPin, uint8_t value) 
{   
  pinValues[changedPin] = value;   
  position += ((pinValues[0] == pinValues[1]) ^ changedPin) ? 1 : -1;
  encoder_changed=1; 
}  

// основное прерывание в системе, генерирующее события реального 
// времени в системе, осуществляется антидребезг кнопок и энкодера, 
// производится подсчет времени и реализуется изменение состояния 
// ШИМ светодиодов LED2 и LED3
// периодичность 1 ms
void SysTick_Handler(void)
{
  times__.time_ll++;            // обновить системное время
  if ((times__.time_l%1000)==0) // есть ли начало секунды?
  {                             // да
    one_sec_flg=1;              // установить запрос показания 
                                // времени на экране
  }
  if (d_timer > 0)              // уменьшить счетчик запрета индикации 
                                // времени на экране
  {
    d_timer--;
  }
  TIM_SetCompare1(TIM3,ocval1); // установить новое значение ШИМ LED2
  TIM_SetCompare2(TIM3,ocval2); // установить новое значение ШИМ LED3
  if (u_d1==0)                  // генерация нового значения ШИМ LED2
  {                             // если направление изменения яркости 
                                // уменьшается
    if (ocval1>1)               // то, если счетчик ШИМ имеет не
                                // минимальное значение
    {
      ocval1--;                 // уменьшить значение на единицу
    }
    else
    {
      u_d1=1;                   // иначе изменить направление 
                                // изменения ШИМ на противоположное
    }
  }
  else
  {  // если направление изменения яркости увеличивается
    if (ocval1<664)             // и счетчик ШИМ не достиг максимальной 
                                // яркости
    {
      ocval1++;                 // увеличить яркость
    }
    else
      u_d1=0;                   // если четчик достиг максимальной яркости 
                                // изменить направление изменения яркости 
                                // на противоположное
  }
  if (u_d2==0)                  // генерация нового значения ШИМ LED3
  {                             // аналогично LED2 
    if (ocval2>1)
    {
      ocval2--;
    }
    else
    {
      u_d2=1;
    }
  }
  else
  {
    if (ocval2<664)
    {
      ocval2++;
    }
    else
      u_d2=0;
  }
  // key 1 section              //обработка состояния кнопок
  {                             // генерация событий сопряженных с 
                                // действиями кнопок
    static uint8_t k1_cntr = 0; // счетчик антидребезга кнопки
    static uint8_t k1_trigger=0;// флаг состояния кнопки 0-> кнопка отжата 
                                // 1-> кнопка нажата
                                // алгоритмом обеспечивается гистерезис 
                                // срабатывания кнопки 
    if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)==0) // считать текущее сост. 
                                                    // кнопки A01
    {// если кнопка нажата
      if (k1_cntr<10)           // если счетчик меньше макс. знач.
      {
        k1_cntr++;              // увеличить счетчик на 1
        //если счетчик достиг макс. знач. и кнопка считается не нажатой
        if ((k1_cntr==10)&&(k1_trigger==0)) 
        {
          k1_trigger=1;   // считать кнопку нажатой
          if (k1_pressed==0) // если флаг события нажатия кнопки не активен
          {
            k1_pressed++;    // активизировать событие нажатия кнопки
          }
        }
      }
    }
    else
    {// иначе, если кнопка не нажата
      if (k1_cntr > 0) // если счетчик антидребезга активен
      {
        k1_cntr--;    // уменьшить счетчик антидребезга на единицу
        // если счетчик антидребезга стал пассивным и кнопка считается 
        // нажатой
        if ((k1_cntr==0)&&(k1_trigger==1))
        {
          k1_trigger=0;         // считать кнопку отжатой
          if (k1_depressed==0)  // если событие отжатия кнопки не активно
          {
            k1_depressed++;     // активизировать событие отжатия кнопки
          }
        }
      }
    }
  }
  // key 2 section             // аналогично кнопке №1
  {
    static uint8_t k2_cntr = 0;
    static uint8_t k2_trigger=0;
    if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)==0)
    {
      if (k2_cntr<10)
      {
        k2_cntr++;
        if ((k2_cntr==10)&&(k2_trigger==0))
        {
          k2_trigger=1;
          if (k2_pressed==0)
          {
            k2_pressed++;
          }
        }
      }
    }
    else
    {
      if (k2_cntr > 0)
      {
        k2_cntr--;
        if ((k2_cntr==0)&&(k2_trigger==1))
        {
          k2_trigger=0;
          if (k2_depressed==0)
          {
            k2_depressed++;
          }
        }
      }
    }
  }
  // key 3 section             // аналогично кнопке №1
  {
    static uint8_t k3_cntr = 0;
    static uint8_t k3_trigger=0;
    if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)==0)
    {
      if (k3_cntr<10)
      {
        k3_cntr++;
        if ((k3_cntr==10)&&(k3_trigger==0))
        {
          k3_trigger=1;
          if (k3_pressed==0)
          {
            k3_pressed++;
          }
        }
      }
    }
    else
    {
      if (k3_cntr > 0)
      {
        k3_cntr--;
        if ((k3_cntr==0)&&(k3_trigger==1))
        {
          k3_trigger=0;
          if (k3_depressed==0)
          {
            k3_depressed++;
          }
        }
      }
    }
  }
/**  * A simple incremental Rotary Encoder decoder example 
  *andrew@rocketnumbernine.com                                          *
  *http://www.rocketnumbernine.com/2010/03/06/decoding-a-rotary-encoder/ *                                                        
  * use freely */ 
  { // считать текущее состояние контактов энкодера
    uint8_t pin0=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0),
            pin1=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1);
    if (pin0 != pinValues[0]) // если отличается от предыдущего 
                              // вызвать подпрограмму обработки изменения 
                              // состояния энкодера
    {
      rotary_encoder_change(0, pin0);
    }
    else if (pin1 != pinValues[1])
    {
      rotary_encoder_change(1, pin1);   
    }
  }
}

uint32_t gettime(void)    // получить текущее значение времени
{
  return times__.time_l;
}

void wait_ms(uint32_t ms) // подпрограмма отработки задержки 
                          // очень неточная, на малых значениях задержки 
                          // погрешность может достигать 10%-50%
{
  uint32_t newtime=ms+gettime();
  while (gettime() < newtime)
    continue;
}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


