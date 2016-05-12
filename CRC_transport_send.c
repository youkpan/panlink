#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "CRC_transport_send.h"

//#define MANCHESTER

#ifdef MANCHESTER 
u8 frameData1[2*(SEND_NUM_MAX + 8)];
#endif

//const u16 sendNumMax = SEND_NUM_MAX;

extern unsigned int CRC16(unsigned char *pcrc, unsigned char count); //最长255

u8 frameData0[SEND_NUM_MAX + 5];

//入口 ： 待发送的数组 ，字节数 n
void send_array(u8 * array,u16 n)
{
	
	u16 frameNum = n/SEND_NUM_MAX ;
	u16 c = 0; //address count
	//u16 i =0;  //
	
	for(c=0; c<frameNum ; c++)
	{	  //address |frame array | Byte num
		  send_frame(c,&array[c*SEND_NUM_MAX],SEND_NUM_MAX);
	}

	//send the last frame
	if (n%SEND_NUM_MAX != 0)
	//address |frame array | Byte num
	{send_frame(c,&array[c*SEND_NUM_MAX], (u8)(n%SEND_NUM_MAX));
	}

}
//address |frame array | frame Byte num
void send_frame(u16 addr,u8 * array,u8 byteNum)
{ //create frame data 0	:(structure)
  //(int)synchronization | (int)address | sizeof(array) sendData | CRC16 |
  u16 n=0;
  u16 fCRC16;

  //(int)address
  frameData0[0]	= (u8)(addr>>8);	  //HI
  frameData0[1]	= (u8)(addr);
  // byteLongth
  frameData0[2] = byteNum;

  //copy data
  for (n =0 ;n <  byteNum ;n++)
  {	
  	frameData0[n+3] = array[n];
  }
  for (;n < SEND_NUM_MAX ;n++)			 //最后没有用的几位。
  {	frameData0[n+3] = 0xFF; }

  fCRC16 =CRC16(frameData0,SEND_NUM_MAX+3);

  n=SEND_NUM_MAX + 3;
  frameData0[n++] = (u8)(fCRC16>>8);
  frameData0[n]   = (u8)(fCRC16);

	#ifdef MANCHESTER
	{
	   manchesterCode(frameData0,SEND_NUM_MAX + 5);
	   send_fram_use_usart(frameData1,2*(SEND_NUM_MAX + 5));
	}
	#else
	{
  		send_fram_use_usart(frameData0,SEND_NUM_MAX + 5);
  	}
	#endif

}
#include "AD9850.h"
#include "main_conf.h"
/**********************************************************/
void bit_H(void)
{
  		TIM_SetCounter(TIM3,0);
		FSK_H();
		//mTP_H;						//<------------------- 改这里实现FSK
        while(TIM_GetCounter(TIM3) < tick/(con_rate)) 
        {}

}

void bit_L(void)
{
  		TIM_SetCounter(TIM3,0);
		FSK_L();
		//mTP_L;						//<-------------------
        while(TIM_GetCounter(TIM3) < tick/(con_rate)) 
        {}

}
/**********************************************************/
//
#include "stm32f10x_reg.h"

void USART_send(u8 ch)
{  	
    #ifdef USE_USART_SEND
	USART_SendData(USART1, (uint8_t) ch);
  	/* Loop until the end of transmission */
  	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  	{}
	#else  //用模拟的串口。
	    u8 i;

		bit_L();  //start bit

		for (i=0 ; i<8 ; i++)
		{  
			if (ch & 0x01)
			{bit_H();}
			else
			{bit_L();}

			ch>>=1;
		}
		bit_L();   //check	 bit
		bit_H();   //stop bit

	#endif

	GPIOC_ODR^= GPIO_Pin_6;
}
//frame array | send num
void send_fram_use_usart(u8 *array,u16 frameByte)
{
   u16 n;

  //begin..
//	for(n =0 ; n< SEND_NUM_MAX+6 ;n++)
	{USART_send(0x00);}

   #ifdef MANCHESTER
   //syn
	USART_send(0x55);
	USART_send(0xCC);
    USART_send(0x33);
   #else
   //syn
	USART_send(0x55);
	USART_send(0x5A);
    USART_send(0xA5);
   #endif

  for (n=0 ;n<frameByte ; n++)
  {
//	LED_rvs();		//LED reverse
	USART_send(array[n]);
  }

}
#ifdef MANCHESTER
//
void manchesterCode(u8 * array , u8 arrayNum)
{
  u16 i=0;
  u16 m=0; //manchester code 10 or 01
  u8  d=0; //temp data
  u8  n=0;

  for (i =0 ;i <arrayNum ;i++)	   //copy to ev
  {
  	 d = array[i];
	 m = 0;
	 for (n =0; n<8 ;n++)
	 {
	   if (d &0x80)
	   {
	   	 m |= 0x2;  //10
	   }
	   else
	   {
	   	 m |= 0x1;  //01
	   }

	   if (n < 7)
	    {d<<=1;
		 m<<=2;
		}
	 }

	 frameData1[2*i]   =(u8)(m >>8);
	 frameData1[2*i+1] =(u8)m;
  }
}
#endif
//test_CRC16_transport
void  test_CRC16_send(void)
{   u8  testData[250];
	u16 i = 0;

	for(i=0; i<250 ;i++)
	{ 
	  testData[i] = i;
	}

	send_array(testData,250);
	send_frame(0xFFFF,testData,SEND_NUM_MAX);
}

#define GPIOC_CRL       (*(volatile unsigned long *)0x40011000) //端口C配置低寄存器 
#define GPIOC_ODR       (*(volatile unsigned long *)0x4001100c) //端口C输出数据寄存 

//LED reverse
void LED_rvs(void)
{  GPIOC_CRL =	(GPIOC_CRL & 0xFFF0FFFF) | 0x00030000;
   GPIOC_ODR ^= GPIO_Pin_4;
}
//this is the end of file
