/*

©2012 NEE
ОТКАЗ ОТ ЛЮБЫХ ВИДОВ ОТВЕСТВЕННОСТИ
USE FREELY

*/
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "lcd.h"
#define port            GPIOB
#define init_port       RCC_APB2Periph_GPIOB
#define pin_e           GPIO_Pin_12
#define pin_rw          GPIO_Pin_10
#define pin_rs          GPIO_Pin_11

#define lcd_shift       6                       //номер последнего бита в 4-битной шине
#define use_gpio        GPIO_Pin_9  // старший бит                                                      9
#define pin_d7          use_gpio    // старший бит                                                      9
#define pin_d6          use_gpio>>1 // следующий по убыванию бит                                           8
#define pin_d5          use_gpio>>2 // следующий по убыванию бит                                           7
#define pin_d4          use_gpio>>3 // следующий по убыванию бит (последний бит в 4-х битной шине)         6

// таблица перекодировки CP866 или CP1251 в кодировку индикатора
// ВНИМАНИЕ! Обеспечивается перекодировка только алфафитных символов, псевдографика
// и служебные символы не перекодируются
uint8_t const lcd_recodetab[] = {
  /*
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
  0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
  0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
  0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
  0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
  0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
  0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
  0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
  */
  0x41,0xa0,0x42,0xa1,0xe0,0x45,0xa3,0xa4,0xa5,0xa6,0x4b,0xa7,0x4d,0x48,0x4f,0xa8,// 0x80
  0x50,0x43,0x54,0xa9,0xaa,0x58,0xe1,0xab,0xac,0xe2,0xad,0xae,0xc4,0xaf,0xb0,0xb1,// 0x90
  0x61,0xb2,0xb3,0xb4,0xe3,0x65,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0x6f,0xbe,// 0xa0
  0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,// 0xb0
  0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,// 0xc0
  0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,0xf4,// 0xd0
  0x70,0x63,0xbf,0x79,0xe4,0x78,0xe5,0xc0,0xc1,0xe6,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,// 0xe0
  0xa2,0xa2,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff

};

// DATA lines b08 b09 b10 b11
// a08 -> RS
// a11 ->en
// инициализация пинов портов управления LCD индикатором
void init_lcd_ports(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Enable the GPIO_LCD Clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_11;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOA,GPIO_Pin_8|GPIO_Pin_11);
}
// ссылка на подпрограмму задержек миллисекундного диапазона
extern void wait_ms(uint32_t ms);
// подпрограмма задержки наносекундного диапазона 50-500нс, точность не высокая
// ВНИМАНИЕ! Задержки не обмеривались и задаются в "попугаях"
void delay_cycles_(uint32_t ckls)
{
  ckls*=5;
  while((ckls--)!=0)
  {
    continue ;
  }
}

// подпрограмма записи полубайта команды (используется 4х битный интерфейс)
// (ужасно не люблю делать перемычки...)
void wr_cmd(uint8_t par)
{
//  Считываются биты данных порта нужные биты обнуляются и вместо них вписываются
// новые биты
  GPIO_Write(GPIOB,(GPIO_ReadOutputData(GPIOB)&(~(0xf00)))|((par&0xf)<<8));

  GPIO_ResetBits(GPIOA,GPIO_Pin_8);// reset RS // сброс сигнала RS
  delay_cycles_(10);                           // задержка ~100-300 нс
  GPIO_SetBits(GPIOA,GPIO_Pin_11);// set E     // установка сигнала Е
  delay_cycles_(10);                           // задержка ~100-300 нс
  GPIO_ResetBits(GPIOA,GPIO_Pin_11);// reset E // сброс сигнала Е
  delay_cycles_(10);                           // задержка ~100-300 нс
}

// подпрограмма записи байта команды (используется 4х битный интерфейс)
// сначала записывается старший полубайт, затем младший полубайт
void wr_cmd_byte(uint8_t par)
{
  //u8 cnt;
  wr_cmd(par >> 4);
  wr_cmd(par & 0xf);
  delay_cycles_(100); // задержка ~2-5 мкс
}

// подпрограмма записи полубайта данных (используется 4х битный интерфейс)
// 
void wr_dat(uint8_t par)
{
  GPIO_Write(GPIOB,(GPIO_ReadOutputData(GPIOB)&(~(0xf00)))|((par&0xf)<<8));
//  RS=1;
  GPIO_SetBits(GPIOA,GPIO_Pin_8);// set RS     // установка сигнала RS
  delay_cycles_(10);                           // задержка ~100-300 нс
  GPIO_SetBits(GPIOA,GPIO_Pin_11);// set E     // установка сигнала Е
  delay_cycles_(10);                           // задержка ~100-300 нс
  GPIO_ResetBits(GPIOA,GPIO_Pin_11);// reset E // сброс сигнала Е
  delay_cycles_(10);                           // задержка ~100-300 нс
}


// подпрограмма записи байта данных (используется 4х битный интерфейс)
// сначала записывается старший полубайт, затем младший полубайт
void wr_dat_byte(uint8_t par)
{
  wr_dat(par >> 4);
  wr_dat(par & 0xf);
  delay_cycles_(100);   // задержка ~2-5 мкс

}



uint8_t row,col;      // текущее положение курсора строка,колонка
uint8_t lcd_ram[32];  // буфер индикатора (модет использоваться при "мультипликации"


//// подпрограмма инициализации LCD индикатора
//void Init_lcd(void)
//{
//  init_lcd_ports();
////  register uint8_t cnt;
//  wait_ms(20);
//  wr_cmd(3);
//  wait_ms(5);
//  wr_cmd(3);
//  wait_ms(1);//100us
//  wr_cmd(3);
//  wait_ms(1);//40us
//  wr_cmd(2);
//  wait_ms(1);//40us
//  wr_cmd_byte(0x28);
//  wait_ms(1);
//  wr_cmd_byte(0x6);
//  wait_ms(1);
//  wr_cmd_byte(0xf);
//  wait_ms(1);
//  wr_cmd_byte(0x1);
//  wait_ms(2);
//  wr_cmd_byte(0x80);
//  wait_ms(1);
//// этот фрагмент можно активировать при необходимости что либо записать во
//// встроенный знакогенератор индикатора
///*
//  wr_cmd_byte(0x40);
//  wait_ms(1);
//  for(cnt=0;cnt<24;cnt++)wr_dat_byte(mysymbols[cnt]);
//  wr_cmd_byte(0x80);
//  wait_ms(1);
//*/
//  row=col=0;
//
//}
//
// продпрограмма выключения мигающего курсора индикатора
void lcd_cursor_off(void)
{
  wr_cmd_byte(0x0c);
}

// продпрограмма включения мигающего курсора индикатора
void lcd_cursor_on(void)
{
  wr_cmd_byte(0x0d);
}


// установить текущее место записи(курсор) в необходимую позицию строка,колонка
void setcursor(uint8_t r,uint8_t c)
{
  wr_cmd_byte(0x80|((r & 1)<<6)|(c & 0x3f));
  row=r;col=c;
}

// записать символ в текущую позицию записи(курсор), курсор сдвигается вправо,
// при достижении конца строки, курсор появляется в начале следующей строки, при
// достижении последней строки курсор появляется в начале первой строки
// (закольцовывается)
// алфавитные символы CP866 или CP1251 перекодируются во внутреннее представление
// кодироки индикатора. Для CP866 0x80, Для CP1251 0xc0,

void wr_lcd(uint8_t chr)
{
  setcursor(row,col);
  lcd_ram[(row << 4)+col]=chr;
  wr_dat_byte(((chr>=0xc0)?(lcd_recodetab[chr-0xc0]):(chr)));
                   //^^^^                     ^^^^
  if(++col>=16)
  {
    col=0;row++;
    if(row>1)row=0;
  }
}

// запись строки алфавитных символов заканчивающихся нулем по адресу (RC)
// запись неалфавинтых символов приведет к непредсказуемым результатам
void wr_lcd_str(uint8_t r,uint8_t c,char *str)
{
  setcursor(r,c);
  while(*str)wr_lcd(*str++);
}

// запись количества (noc) любых символов по текущему адресу записи(курсору)
void wr_lcd_num_chr(char *str,uint8_t noc)
{
  register uint8_t cnt;
  for(cnt=0;cnt<noc;cnt++)
  {
    if(str[cnt])wr_lcd(str[cnt]);
    else return;
  }
}

// очистить строку от текущей позиции записи(курсор) до конца текущей строки
void lcd_clr_to_eol(void)
{
  uint8_t r=row;
  uint8_t c=col;
  do
  {
    wr_lcd(' ');
  }
  while(col!=0);
  setcursor(r,c);
}


// очистка экрана
void clrscr(void)
{
  setcursor(0,0);
  lcd_clr_to_eol();
  setcursor(1,0);
  lcd_clr_to_eol();
  setcursor(0,0);
}

// неудачная попытка эмулировать поведение стандартного putchar
int putchar__(int chr)
{
  switch(chr)
  {
  case 0xd:
    setcursor(row,0);
    break;
  case 0xa:
    if(++row >=2)
      clrscr();
    break;
  default:
    wr_lcd(chr);
    if(++col >= 16)
    {
      col=0;
      if(++row >= 2)
      {
        clrscr();
        row=col=0;
      }
    }
    break;
  }
  return chr;
}



