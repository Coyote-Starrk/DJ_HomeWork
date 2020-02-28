#ifndef __SYS_UTILS_H__
#define __SYS_UTILS_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "gpio.h"
#include "lora.h"

#define NEXT_TRIGGER_DELAY	3 	//3*500ms=1500s延时长度（从前一次按下楼层按钮，到下一次按下楼层按钮之间的时间间隔）
#define BASE_PERIOD					3 	//3*500ms=1500ms控制按下电梯楼层按钮最少需要的时间

//定时器初始化
void TIMER_Init(void);
//延时初始化
void delay_Init(void);
//延时nus								   
void delay_us(u32 nus);
//延时nms,nms<=1864 
void delay_ms(u16 nms);

//设置事件
void SYSEvent_Set(SYSEvent event_in);
//清除事件
void SYSEvent_Clr(SYSEvent event_in);
//CRC-8
uint8_t crc8(const uint8_t *buff, size_t n);
	
#endif /*__SYS_UTILS_H__*/
