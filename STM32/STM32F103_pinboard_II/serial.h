#ifndef SERIAL_H
#define SERIAL_H

extern void pbii_sio0_TX_put(uint8_t chr);
extern uint8_t pbii_sio0_RX_get(void);
extern int pbii_sio0_RX_rdy(void);
extern void pbii_USART_init(uint32_t baud_rate);

#endif
