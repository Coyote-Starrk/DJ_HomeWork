#ifndef __SYS_UTILS_H__
#define __SYS_UTILS_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "stdio.h"

#define DEBUG_BOUND					9600//调试输出串口的波特率

//常用工具初始化
void Utils_ModuleInit(void);
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
//通过floorID获得relayID
uint8_t convertFloorIDToSwitchID(int16_t floorID);

#endif /*__SYS_UTILS_H__*/
