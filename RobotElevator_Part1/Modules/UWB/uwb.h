#ifndef __UWB_H__
#define __UWB_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "sys_utils.h"
#include "string.h"
#include "usart.h"

#define UWB_BAUD_RATE 			115200												//串口波特率
#define UWB_WORLD_LENGTH 		USART_WordLength_8b						//字长为8位数据格式
#define UWB_STOP_BITS 			USART_StopBits_1							//一个停止位
#define UWB_PARITY					USART_Parity_No								//无奇偶校验位

#define UWB_DMA_RX_SIZE			300														//DMA接收缓冲大小

#define UWB_POWER_ON 		GPIO_SetBits(GPIOE,GPIO_Pin_15) 	//开启UWB模块电源
#define UWB_POWER_OFF 	GPIO_ResetBits(GPIOE,GPIO_Pin_15) //开启UWB模块电源

//UWB模块初始化
void UWB_Init(void);
//UWB设置模块地址
void UWB_ResetTagAddr(void);
//从UWB数据中读取距离信息
void UWB_getDistance(char* data);
	
#endif /*__UWB_H__*/
