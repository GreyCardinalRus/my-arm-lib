/*

©2012 NEE
ОТКАЗ ОТ ЛЮБЫХ ВИДОВ ОТВЕСТВЕННОСТИ
USE FREELY

*/
#include "stm32f10x.h"
//#include "stm32_platform.h"
#include "stm32f10x_usart.h"
#include "serial.h"


// ВНИМАНИЕ! на 8ми битных платформах атомарность не обеспечивается



// подпрограмма разрешения клокирования портов ВВ и USART1
void pbii_USART_RCC_Configuration(void)
{   
  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO, ENABLE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 
}


// подпрограмма инициализации портов ВВ  A09 и A10 для USART1
void pbii_USART_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure USART1 Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure USART1 Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

}
// подпрограмма инициализации вектора прерывания USART1_IRQHandler
void pbii_USART_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

}

// полная подпрограмма инициализации USART1 для работы на PBII
void pbii_USART_init(uint32_t baud_rate)
{
  USART_InitTypeDef USART_InitStructure;
    /* System Clocks Configuration */
  pbii_USART_RCC_Configuration();
  /* Configure the GPIO ports */
  pbii_USART_GPIO_Configuration();
       
  /* NVIC configuration */
  pbii_USART_NVIC_Configuration();


/* USART1  configuration ------------------------------------------------------*/
  /* USART1  configured as follow:
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = baud_rate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  /* Configure USART1 */
  USART_Init(USART1, &USART_InitStructure);
  /* Enable USART1 Receive and Transmit interrupts */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);// разрешить прерывание по приему
                                                // байта
  USART_ITConfig(USART1, USART_IT_TXE, ENABLE); // разрешить прерывание по передаче
  /* Enable the USART1 */                       // байта
  USART_Cmd(USART1, ENABLE);                    // разрешить работу USART1

}
// size of buffer must be power of two
#define RX_buf_size (128)             // размер буфера приема должен быть
                                      // степенью двойки, что сильно упрощает мани-
                                      // пуляцию с индексами, "заворачивание" индекса
                                      // в кольцевом буффере
#define Rx_buf_mask (RX_buf_size-1)   // константа применяемая для "заворачивания"
                                      // индекса

static uint8_t RX_buf[RX_buf_size];   // собственно входной буфер приема

union r_idxs_                         // входной и выходной индексы буфера приема,
                                      // обьединение индексов в 32х разр. величину
                                      // позволяет получить атомарный доступ к
                                      // индексам для их анализа
{
//  struct ridx_
//  {
    uint16_t ridx_in;
    uint16_t ridx_out;
//  };
  uint32_t ridxs32;
}ridxs={0UL};

// подпрограмма проверки возможноти поместить байт во входной буфер
int RX_buf_rdy(void)                 
{
  union r_idxs_ ri;
  ri.ridxs32=ridxs.ridxs32;// получить индексы входного буфера атомарно
  if(((ri.ridx_in+1)%RX_buf_size)!=ri.ridx_out)// выяснить возможность
  {
    return 1;
  }                                           // вернуть результат
  return  0;
}


// продпрограмма проверки наличия символов во входном буфере
int pbii_sio0_RX_rdy(void)
{
  union r_idxs_ ri;
  ri.ridxs32=ridxs.ridxs32;// получить индексы входного буфера атомарно
  if(ri.ridx_in==ri.ridx_out)// проверить наличие символов во вх. буфере
  {
    return 0;                // вернуть результат
  }
  return 1;
}


// подпрограмма извлечения символа их входного буфера
// вызов блокирующий
uint8_t pbii_sio0_RX_get(void)
{
  uint8_t retval;
  while(pbii_sio0_RX_rdy()==0)// ожидать до тех пор пока не появится символ во
                              // входном буфере
  {
    continue ;
  }
  retval=RX_buf[ridxs.ridx_out++]; // извлечь символ из входного буфера
  ridxs.ridx_out %= RX_buf_size;   // скорректировать, "завернуть" выходной индекс
  return  retval;
}


// size of buffer must be power of two // размер буфера передачи должен быть
#define TX_buf_size (128)              // степенью двойки, что сильно упрощает мани-
                                       // пуляцию с индексами, "заворачивание" индекса
                                       // в кольцевом буффере
#define Tx_buf_mask (TX_buf_size-1)    // константа применяемая для "заворачивания"
                                       // индекса
                                                   
static uint8_t TX_buf[TX_buf_size];    // собственно выходной буфер передачи
                                                                                      
union t_idxs_                          // входной и выходной индексы буфера приема,
{                                      // обьединение индексов в 32х разр. величину
//  struct tidx_                         // позволяет получить атомарный доступ к
//  {                                    // индексам для их анализа
    uint16_t tidx_in;              
    uint16_t tidx_out;             
//  };
  uint32_t tidxs32;                
}tidxs={0UL};                      








// проверка возможности поместить символ в выходной буфер передачи
int TX_buf_rdy(void)
{
  union t_idxs_ ti;
  ti.tidxs32=tidxs.tidxs32;
  if(((ti.tidx_in+1)%TX_buf_size)!=ti.tidx_out)
  {
    return 1;
  }
  return  0;
}


// прерывание от USART1
void USART1_IRQHandler(void)
{

  // часть прерывания посвященная прерыванию от принятого байта
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {
    /* Read one byte from the receive data register */
    uint16_t tmpw = USART_ReceiveData(USART1);// считать байт из приемника во
                                              // временное место хранения
    if(RX_buf_rdy()!=0)                       // если есть место во входном буфере
                                              // приема
    {
      RX_buf[ridxs.ridx_in++]=tmpw;           // поместить байт в буфер приема
      ridxs.ridx_in %=RX_buf_size;            // скорректировать "завернуть" индекс
    }// иначе принятый байт летит на пол

  }


  // часть прерывания посвященная прерыванию от передатчика  USART1
  if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
  {   
    union t_idxs_ t_idxs;
    t_idxs.tidxs32=tidxs.tidxs32;// получить индексы буфера передатчика атомарно
    if(t_idxs.tidx_in!=t_idxs.tidx_out)// есть байты в выходном буфере передатчика?
    {// да, извлечь байт из буфера и поместить его в передатчик
      /* Write one byte to the transmit data register */
      USART_SendData(USART1, TX_buf[tidxs.tidx_out++]);
      tidxs.tidx_out %=TX_buf_size;// скорректировать "завернуть" выходной индекс

    }
    else
    {// иначе, передавать нечего, запретить прерывания передатчика
      /* Disable the USART1 Transmit interrupt */
      USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    }    
  }
}

// подпрограмма передачи байта в USART1 на  PBII вызов не блокирующий

void pbii_sio0_TX_put(uint8_t chr)
{
  // если прерывания передатчика запрещены
  if(USART_GetITStatus(USART1,USART_IT_TXE)==RESET)
  {
    USART_SendData(USART1,chr);// передать байт непосредственно в USART1
    USART_ITConfig(USART1,USART_IT_TXE,ENABLE);// разрешить прер. передатчика
  }
  else
  { // если есть место в выходном буфере передатчика
    if(TX_buf_rdy()!=0)
    {
      TX_buf[tidxs.tidx_in++]=chr;// поместить байт в выходной буфер передатчика
      tidxs.tidx_in %=TX_buf_size;// скорректировать "завернуть" выходной индекс
    }// иначе передаваемый байт летит на пол
  }
}
