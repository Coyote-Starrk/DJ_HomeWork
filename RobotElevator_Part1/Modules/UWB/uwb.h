#ifndef __UWB_H__
#define __UWB_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "sys_utils.h"
#include "string.h"
#include "usart.h"

#define UWB_BAUD_RATE 			115200												//���ڲ�����
#define UWB_WORLD_LENGTH 		USART_WordLength_8b						//�ֳ�Ϊ8λ���ݸ�ʽ
#define UWB_STOP_BITS 			USART_StopBits_1							//һ��ֹͣλ
#define UWB_PARITY					USART_Parity_No								//����żУ��λ

#define UWB_DMA_RX_SIZE			300														//DMA���ջ����С

#define UWB_POWER_ON 		GPIO_SetBits(GPIOE,GPIO_Pin_15) 	//����UWBģ���Դ
#define UWB_POWER_OFF 	GPIO_ResetBits(GPIOE,GPIO_Pin_15) //����UWBģ���Դ

//UWBģ���ʼ��
void UWB_Init(void);
//UWB����ģ���ַ
void UWB_ResetTagAddr(void);
//��UWB�����ж�ȡ������Ϣ
void UWB_getDistance(char* data);
	
#endif /*__UWB_H__*/
