#ifndef __LORA_H__
#define __LORA_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "sys_utils.h"
#include "string.h"
#include "usart.h"

#define LORA_BAUD_RATE 		115200								//���ڲ�����
#define LORA_WORLD_LENGTH USART_WordLength_8b		//�ֳ�Ϊ8λ���ݸ�ʽ
#define LORA_STOP_BITS 		USART_StopBits_1			//һ��ֹͣλ
#define LORA_PARITY				USART_Parity_No				//����żУ��λ

#define LORA_DMA_RX_SIZE				300							//DMA���ջ�������С
#define LORA_DMA_TX_SIZE				300							//DMA���ͻ�������С

//M0���ſ���
#define M0_HI 						GPIO_SetBits(GPIOB,GPIO_Pin_1)
#define M0_LO							GPIO_ResetBits(GPIOB,GPIO_Pin_1)

//M1���ſ���
#define M1_HI 						GPIO_SetBits(GPIOE,GPIO_Pin_7)
#define M1_LO 						GPIO_ResetBits(GPIOE,GPIO_Pin_7)

//LORA��Դ����
#define LORA_POWER_ON 		GPIO_SetBits(GPIOE,GPIO_Pin_9)
#define LORA_POWER_OFF 		GPIO_ResetBits(GPIOE,GPIO_Pin_9)

//AUX���������ƽ
#define LORA_AUX 					GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8)

//Loraģ���ʼ��
void Lora_Init(void);	
//loraģ�����õ�ַ���ŵ�/���㴫��Ȳ���
void LORA_paramSet(void);
//���һ֡���ģ�������LORA����
void LORA_SendMsg(ADDR_ToSend addr, SYS_MsgHead resMsg, char* payload,size_t payloadLen);

#endif /*__LORA_H__*/
