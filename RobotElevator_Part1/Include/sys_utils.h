#ifndef __SYS_UTILS_H__
#define __SYS_UTILS_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "stdio.h"

#define DEBUG_BOUND					9600//����������ڵĲ�����

//���ù��߳�ʼ��
void Utils_ModuleInit(void);
//��ʱnus								   
void delay_us(u32 nus);
//��ʱnms,nms<=1864 
void delay_ms(u16 nms);

//�����¼�
void SYSEvent_Set(SYSEvent event_in);
//����¼�
void SYSEvent_Clr(SYSEvent event_in);
//CRC-8
uint8_t crc8(const uint8_t *buff, size_t n);
//ͨ��floorID���relayID
uint8_t convertFloorIDToSwitchID(int16_t floorID);

#endif /*__SYS_UTILS_H__*/
