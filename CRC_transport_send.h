#ifndef  _CRC16_TRANSPORT_H_
#define  _CRC16_TRANSPORT_H_
#include "stm32f10x_reg.h"
#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#define SEND_NUM_MAX  2

#define mTP_H GPIOC->BSRR = GPIO_Pin_5
#define mTP_L GPIOC->BRR  = GPIO_Pin_5

//address |frame array | frame Byte num
void send_frame(u16 addr,u8 * array,u8 byteNum);
//frame array | send num
void send_fram_use_usart(u8 *array,u16 frameByte);
//test_CRC16_transport
void  test_CRC16_send(void);
//LED reverse
void LED_rvs(void);
//入口 ： 待发送的数组 ，字节数 n
void send_array(u8 * array,u16 n);
void USART_send(u8 ch);
void manchesterCode(u8 * array , u8 arryNum);

#endif //_CRC16_TRANSPORT_H_
