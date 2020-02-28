#ifndef __SYS_UTILS_H__
#define __SYS_UTILS_H__

#include "stm32f10x.h"
#include "sys_param.h"
#include "gpio.h"
#include "lora.h"

#define NEXT_TRIGGER_DELAY	3 	//3*500ms=1500s��ʱ���ȣ���ǰһ�ΰ���¥�㰴ť������һ�ΰ���¥�㰴ť֮���ʱ������
#define BASE_PERIOD					3 	//3*500ms=1500ms���ư��µ���¥�㰴ť������Ҫ��ʱ��

//��ʱ����ʼ��
void TIMER_Init(void);
//��ʱ��ʼ��
void delay_Init(void);
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
	
#endif /*__SYS_UTILS_H__*/
