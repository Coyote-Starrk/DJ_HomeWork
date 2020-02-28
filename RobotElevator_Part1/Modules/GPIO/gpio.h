#ifndef __GPIO_H__
#define __GPIO_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "sys_utils.h"

//�������ռ̵���һ����ûд

//LEDָʾ�ƿ���
#define LED_POWER_ON() 	GPIO_SetBits(GPIOA,GPIO_Pin_15)						 	//PA.15����ߵ�ƽ
#define LED_POWER_OFF() GPIO_ResetBits(GPIOA,GPIO_Pin_15)						 //PA.15����͵�ƽ

//OPEN_DOOR
#define DOOR_OPEN()		GPIO_ResetBits(GPIOD,GPIO_Pin_9)
#define DOOR_CLOSE()	GPIO_SetBits(GPIOD,GPIO_Pin_9)

//���뿪�ص͵�ַ3bitλ����
#define GET_ADDRL_BIT1() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)
#define GET_ADDRL_BIT2() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)
#define GET_ADDRL_BIT3() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)
//���뿪���ŵ�3bitλ����
#define GET_CHANNEL_BIT1() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)
#define GET_CHANNEL_BIT2() GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4)
#define GET_CHANNEL_BIT3() GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)

typedef enum {
	CtrInput_Disable = 0,					// ����GPIO����ߵ�ƽ
	CtrInput_Enable = 1						// ����GPIO����͵�ƽ
}CtrInput_TypeDef;

void GPIO_ModuleInit(void);
//��ȡ&��䵱ǰ�豸�ĵ�ַ��Ϣ
void GPIO_GetDeviceAddr(void);
//¥�㰴ť����
void GPIO_FloorButtonCtrl(int16_t floorID, CtrInput_TypeDef ctrInput);
//������ʹ�����ſ���
void GPIO_EncoderEnableCtrl(uint8_t relayID, CtrInput_TypeDef ctrInput);	

#endif /*__GPIO_H__*/
