#ifndef __GPIO_H__
#define __GPIO_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "sys_utils.h"
#include "flash.h"

//�������ռ̵���һ����ûд

//LEDָʾ�ƿ���
#define LED_ON 	GPIO_SetBits(GPIOA,GPIO_Pin_15)						 	//PA.15����ߵ�ƽ
#define LED_OFF GPIO_ResetBits(GPIOA,GPIO_Pin_15)						 //PA.15����͵�ƽ

//OPEN_DOOR
#define DOOR_OPEN		GPIO_SetBits(GPIOD,GPIO_Pin_9)
#define DOOR_CLOSE	GPIO_ResetBits(GPIOD,GPIO_Pin_9)

//���뿪�ص͵�ַ3bitλ����
#define ADDRL_BIT1 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)
#define ADDRL_BIT2 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)
#define ADDRL_BIT3 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)
//���뿪���ŵ�3bitλ����
#define CHANNEL_BIT1 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)
#define CHANNEL_BIT2 GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4)
#define CHANNEL_BIT3 GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)

typedef enum {
	CtrInput_Disable = 0,					// ����GPIO����ߵ�ƽ
	CtrInput_Enable = 1						// ����GPIO����͵�ƽ
}CtrInput_TypeDef;

void initLed(void);
//GPIO��ʼ��
void GPIO_Usr_Init(void);
//��ȡ&��䵱ǰ�豸�ĵ�ַ��Ϣ
void Update_Current_ADDRL(void);
//���¶�Ӧĳһ¥����������������
void floorButtonControl(int16_t floorID, CtrInput_TypeDef ctrInput);
	
#endif /*__GPIO_H__*/
