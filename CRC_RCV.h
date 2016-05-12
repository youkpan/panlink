#ifndef _CRC_RCV_H_
#define _CRC_RCV_H_


//////////////////////////////////////////////////////////////////////
//																	//
//																	//
//////////////////////////////////////////////////////////////////////


#define RCV_MAX   (2)	   //1
#define FRAME_MAX (RCV_MAX+5)
#define DF_SIZE   (TXT_SIZE / RCV_MAX /32)  //数据标志位的大小
//#define MANCHESTER

//  #ifdef MANCHESTER //发送的。。
//	USART_send(0x55);
//	USART_send(0xCC);
//  USART_send(0x33);
//  #else
//	USART_send(0x55);
//	USART_send(0x5A);
//  USART_send(0xA5);
//  #endif

#define USE_IT	  //使用中断

extern u8 rcvData[];


//#define ON  0x01
//#define OFF	0x00

typedef enum { OFF = 0, ON = !OFF} mState;

extern void LED_rvs(void);
void rcv_frame(void);
void test_rcv(void);
void mDecode(u8 * mArray , u16 mArrayNum );
void rcv_a_frame(void);
void show_test_info(void);
void resetDataFlag(void);
u8   DataRCV_Over(void);


#endif //_CRC_RCV_H_
//EOF
