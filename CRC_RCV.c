#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "12864.h"


#include "CRC_RCV.h"

#ifdef MANCHESTER 
u8 frameData1[2*(FRAME_MAX)];   //manchester code data
#endif

extern unsigned int CRC16(unsigned char *pcrc, unsigned char count);

u8 testD=0;
mState RCV_Ctrl=OFF;

//////////////////////////////////////////////////////////////////////
//																	//
//																	//
//////////////////////////////////////////////////////////////////////
u16 frameRcvIdx =0;
u8 frameRcv[FRAME_MAX];
u16 frameDataIdx =0;
u8 frameData[FRAME_MAX];
//u32 DataFlag[DF_SIZE];    // 判断是否接收完的标志位	   每一位代表一个帧，全收完为0xFFFFFFFF
//////////////////////////////////////////////////////////////////////
//																	//
//																	//
//////////////////////////////////////////////////////////////////////
//mState RCV_Ctrl =OFF;
//mState RCV_state = OFF;  // 0-OFF 1-ON;
u16 frameRcvNum =0;
u16 frameRcvWrong =0;
u8 rcvData[1024];
//////////////////////////////////////////////////////////////////////
//																	//
//																	//
//////////////////////////////////////////////////////////////////////
void delay_us(u8 us)
{ u8 t =15;
   while (us--)
   {
   	 t=15;
	 while(t--);
   }

}
//////////////////////////////////////////////////////////////////////
//																	//
//																	//
//////////////////////////////////////////////////////////////////////
void rcv_a_frame(void)
{  	 u8 c=0,old =0;
	#ifdef MANCHESTER
		 while (1)		//wait for syn
	 	{	
			 while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
     		 {}
   	 		 c = USART_ReceiveData(USART1);

			 if (c == 0x33)
			 {
			 	if (old == 0xcc)
				{break;}
			 }
			 old = c ;
	 	}	
	 	  frameDataIdx =0;

	     while(frameDataIdx < (2*FRAME_MAX) )
	    {
		 while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
     	 {}
		 frameData1[frameDataIdx++] = USART_ReceiveData(USART1) ;
	 	}
		//decode
		mDecode(frameData1 , (2*FRAME_MAX) );
	//-------------------------------------------------------		
	#else
		 while (1)		//wait for syn
	 	{	
			 while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
     		 {}
   	 		 c = USART_ReceiveData(USART1);

			 if (c == 0xA5)
			 {
			 	if (old == 0x5A)
				{break;}
			 }
			 old = c ;
	 	}	
	 	  frameDataIdx =0;

		while(frameDataIdx < FRAME_MAX)
	 	{
		 while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
     	 {}
		 frameData[frameDataIdx++] = USART_ReceiveData(USART1) ;
	 	}
	//-------------------------------------------------------
	#endif
}
//////////////////////////////////////////////////////////////////////
//	接收并检查一个帧 ，如成功，COPY到frameData 数组中。
//	用中断的话，检查，COPY																//
//////////////////////////////////////////////////////////////////////
u16 newRCV_Addr=0;
vu16 newFrame =0;
extern void add_to_enumed(u16 enumed_ID,u8 data,u8 distance);
void rcv_frame(void)
{	 u16 mCRC=0;
	 u16 CRC_rcv;
//	 static u8 addr_bf =0;	 //check rcv addr is countinue or not
//	 static u8 addr_dif =0;
//	 DMA_Cmd(DMA1_Channel5, ENABLE);
//
//      DMA_ClearFlag(DMA1_FLAG_TC5);
//	  /* Wait until USARTy TX DMA1 Channel  Transfer Complete */
//      while (DMA_GetFlagStatus(DMA1_FLAG_TC5) == RESET)
//      {}
//
//	 DMA_Cmd(DMA1_Channel5, DISABLE);
		
//	 RCV_Ctrl = ON;
//
//	 while(RCV_state == OFF)
// 	 {delay_us(3);}
//	 while(RCV_state == ON)
// 	 {delay_us(5);;}
//
//	 RCV_Ctrl = OFF;

	 #ifndef USE_IT
	 rcv_a_frame();
	 #endif

	 CRC_rcv  = frameData[RCV_MAX+3];
	 CRC_rcv<<=8;
	 CRC_rcv |= frameData[RCV_MAX+4];

	 mCRC = CRC16(frameData,RCV_MAX+3);

	 if (mCRC == CRC_rcv)			   //CRC right
	 { u16 Addr =0;
	   u32 n =0;
	   u16 flagIdx =0;
//		 //show rcv addr
//		 if (frameData[1] !=0) 
//		 {addr_dif += (u8)(frameData[1] - addr_bf -1);
//		  shownum(1,4,addr_dif);
//		 }
//   		  addr_bf = frameData[1];

	   //LED_rvs();

	   Addr = frameData[0];
	   Addr = (Addr<<8) | (u16)(frameData[1]);

	   newRCV_Addr = Addr;

//	   if (Addr == 0xFFFF)	//命令，强置 标志位 为接收成功
//	   {
//	   	    for (n =0; n<=DF_SIZE ;n++)
//   			{ DataFlag[n] = 0xFFFFFFFF; }
//			return;
//	   }

       //Addr 帧的地址	 标志位设定
	   //----------------------------------------------
//	   n = 1;		//标志位
//	   while (Addr/32 >= DF_SIZE)
//	   { flagIdx ++;
//	   }
//	   n <<=  Addr%32;
//	   if (DataFlag[flagIdx] & n)	//该帧已经收到了。
//	   {
//	   	 frameRcvNum ++ ;
//		 return; 
//	   }
//	   else
//	   {
//	   	   DataFlag[flagIdx] |=n; 
//	   }
	   //----------------------------------------------
	        Addr *= RCV_MAX;	  //字节地址


			for (n=0; n <frameData[2] ;n++)			//copy data
	   		{  	if (n < RCV_MAX)
			    {rcvData[n+Addr] = frameData[n+3];} 
	   		}

		newFrame = 1; //

		add_to_enumed(newRCV_Addr,frameData[3],frameData[4]);

	 }
	 else
	 {
	  frameRcvWrong ++ ;
	 }

	 frameRcvNum ++ ;
}
//////////////////////////////////////////////////////////////////////
//  重置 数据完成标志位	 置0；																
//																	
//////////////////////////////////////////////////////////////////////
//void resetDataFlag(void)
//{  u16 n=0;//
//   for (n =0; n<=DF_SIZE ;n++)
//   { DataFlag[n] = 0; }
//}
//////////////////////////////////////////////////////////////////////
//	检查数据是否被接收完成。是 返回1.								
//																	
//////////////////////////////////////////////////////////////////////
//u8 DataRCV_Over(void)
//{  u16 n=0;//
//   for (n =0; n<=DF_SIZE ;n++)
//   { if (DataFlag[n] != 0xFFFFFFFF)
//     {return 0;}
//   }
//   return 1;
//}
//////////////////////////////////////////////////////////////////////
//																	
//																	
//////////////////////////////////////////////////////////////////////
void show_test_info(void)
{	
#ifdef MANCHESTER
  	  show(1,1,6,"收到M:");

#else
  	  show(1,1,6,"收到:");
#endif
	show(1,8,2,"帧");

	shownum(1,5,(frameRcvNum /10000)%100);
	shownum(1,6,(frameRcvNum /100)%100);
	shownum(1,7,(frameRcvNum )%100);

	show(2,1,16,"错误：        帧");
	shownum(2,5,(frameRcvWrong /10000)%100);
	shownum(2,6,(frameRcvWrong /100)%100);
	shownum(2,7,(frameRcvWrong )%100);
  	show(3,1,16,"错误率：  .    %");
	if (frameRcvNum	!=0)
	{	shownum(3,4,(u8)(frameRcvWrong / frameRcvNum));
		shownum(3,5,(u8)(frameRcvWrong *100 / frameRcvNum));
		shownum(3,7,(u8)(((float)frameRcvWrong /(float)frameRcvNum)*(float)10000)%100);
	}
	else if (frameRcvNum ==0)
	{	shownum(3,5,0);
		shownum(3,7,0);
	}

	show(4,1,16,frameData+3 );
}
//////////////////////////////////////////////////////////////////////
//																	//
//																	//
//////////////////////////////////////////////////////////////////////
void test_rcv(void)
{ u16 n;

  while (1)
  {	show_test_info();
	rcv_frame(); 
  }

  //while(1);
}
#ifdef MANCHESTER 
//////////////////////////////////////////////////////////////////////
//manchester decode ---- in :manchester array. out: framedata																	
//																	
//////////////////////////////////////////////////////////////////////
void mDecode(u8 * mArray , u16 mArrayNum )
{
   u16 m; // manchester word
   u16 i;
   u16 n;
   u16 dd=0;//decode data

   for (i =0 ;i< (mArrayNum/2) ;i++)
   {
   	     m =  mArray[2*i];
		 m<<=8;
		 m |= (u16)mArray[2*i+1];

		 dd = 0;
		 for (n =0 ;n<8 ;n++)
		 {
			if (!(m & 0x4000))	   //if code is not 01
			{	dd |= 1;}
			if (n <7)
		 	{dd <<=1;
		     m  <<=2;
			}
		 }
		 testD = dd;
		 frameData[i] = (u8)dd;
   }

}
#endif
//////////////////////////////////////////////////////////////////////////////////
#define GPIOC_CRL       (*(volatile unsigned long *)0x40011000) //端口C配置低寄存器 
#define GPIOC_ODR       (*(volatile unsigned long *)0x4001100c) //端口C输出数据寄存 
//////////////////////////////////////////////////////////////////////
//																	//
//																	//
//////////////////////////////////////////////////////////////////////
//LED reverse
//void LED_rvs(void)
//{  GPIOC_CRL =	(GPIOC_CRL & 0xFFF0FFFF) | 0x00030000;
//   GPIOC_ODR ^= GPIO_Pin_4;
//}
//eof
