#ifndef __GPIO_H__
#define __GPIO_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "sys_utils.h"
#include "flash.h"

//常开常闭继电器一个都没写

//LED指示灯控制
#define LED_ON 	GPIO_SetBits(GPIOA,GPIO_Pin_15)						 	//PA.15输出高电平
#define LED_OFF GPIO_ResetBits(GPIOA,GPIO_Pin_15)						 //PA.15输出低电平

//OPEN_DOOR
#define DOOR_OPEN		GPIO_SetBits(GPIOD,GPIO_Pin_9)
#define DOOR_CLOSE	GPIO_ResetBits(GPIOD,GPIO_Pin_9)

//拨码开关低地址3bit位输入
#define ADDRL_BIT1 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)
#define ADDRL_BIT2 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)
#define ADDRL_BIT3 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)
//拨码开关信道3bit位输入
#define CHANNEL_BIT1 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)
#define CHANNEL_BIT2 GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4)
#define CHANNEL_BIT3 GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)

typedef enum {
	CtrInput_Disable = 0,					// 控制GPIO输出高电平
	CtrInput_Enable = 1						// 控制GPIO输出低电平
}CtrInput_TypeDef;

void initLed(void);
//GPIO初始化
void GPIO_Usr_Init(void);
//获取&填充当前设备的地址信息
void Update_Current_ADDRL(void);
//更新对应某一楼层的译码器数据输出
void floorButtonControl(int16_t floorID, CtrInput_TypeDef ctrInput);
	
#endif /*__GPIO_H__*/
