#ifndef __LORA_H__
#define __LORA_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "sys_utils.h"
#include "string.h"
#include "usart.h"

#define LORA_BAUD_RATE 		115200								//串口波特率
#define LORA_WORLD_LENGTH USART_WordLength_8b		//字长为8位数据格式
#define LORA_STOP_BITS 		USART_StopBits_1			//一个停止位
#define LORA_PARITY				USART_Parity_No				//无奇偶校验位

#define LORA_DMA_RX_SIZE				300							//DMA接收缓冲区大小
#define LORA_DMA_TX_SIZE				300							//DMA发送缓冲区大小

//M0引脚控制
#define M0_HI 						GPIO_SetBits(GPIOB,GPIO_Pin_1)
#define M0_LO							GPIO_ResetBits(GPIOB,GPIO_Pin_1)

//M1引脚控制
#define M1_HI 						GPIO_SetBits(GPIOE,GPIO_Pin_7)
#define M1_LO 						GPIO_ResetBits(GPIOE,GPIO_Pin_7)

//LORA电源控制
#define LORA_POWER_ON 		GPIO_SetBits(GPIOE,GPIO_Pin_9)
#define LORA_POWER_OFF 		GPIO_ResetBits(GPIOE,GPIO_Pin_9)

//AUX引脚输入电平
#define LORA_AUX 					GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8)

//Lora模块初始化
void Lora_Init(void);	
//lora模块设置地址、信道/定点传输等参数
void LORA_paramSet(void);
//填充一帧报文，并调用LORA发送
void LORA_SendMsg(ADDR_ToSend addr, SYS_MsgHead resMsg, char* payload,size_t payloadLen);

#endif /*__LORA_H__*/
