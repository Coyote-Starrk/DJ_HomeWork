#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x.h"
#include <string.h>
#include "sys_param.h"
#include "sys_utils.h"

//Ref:https://www.cnblogs.com/foxclever/p/5784169.html

//Ӳ�����֧��16*8=128�㣬stm32f103vct6��flash����Ϊ256k��48k�ڴ棬Ӳ��ÿ��������С2k������С������λ
//ȡһ��������Ϊ��������1����һ��������Ϊ��������2��ÿ���������������������1��Ӳ�����Ȼ�����Ƿ���2
//ϵͳ���������Ժ󣬶�����ʱ��У�����1�ͷ���2�Ƿ�һ��

//ѡ������������Ϊ�洢������λ��
#define FLASH_PARAM_PAGE1_START 					((uint32_t)0x08004000)	//16k~17k����
#define FLASH_PARAM_PAGE2_START					 	((uint32_t)0x08004800)	//18k~19k����

//ÿ������/����ҳ�Ĵ�С
#define SECTOR_SIZE           2048			//�ֽ�

//��ָ����ַ��ȡ����
uint16_t FLASH_ReadHalfWord(uint32_t address);
//��ָ����ʼ��ַ��ʼ��ȡ�������
void FLASH_ReadData(uint32_t startAddress,uint16_t *readData,uint16_t countToRead);
//�����ڴ���������flash����
void FLASH_UpdateFlash(uint32_t pageAddress);
//������ʼ��������flash��ʼ��ַ����ֹ��ַ�������������ڴ�����
void FLASH_ModuleInit(void);
//�������Ŀ�д�����һ������
uint8_t FLASH_AddParam(uint32_t pageAddress, SYSFloorMsg param);
//��flash����һ������
void FLASH_AddOneParam(SYSFloorMsg param);
//�������flash����
void FLASH_CleanAllParam(void);
	
#endif /*__FLASH_H__*/



