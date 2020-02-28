#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x.h"
#include <string.h>
#include "sys_param.h"

//Ref:https://www.cnblogs.com/foxclever/p/5784169.html

//Ӳ�����֧��16*8=128�㣬stm32f103vct6��flash����Ϊ256k��48k�ڴ棬Ӳ��ÿ��������С2k������С������λ
//ȡһ��������Ϊ����������������һ��������Ϊ�������ݷ�����ÿ�����������������򱸷ݷ�����Ӳ�����Ȼ��������������
//ϵͳ���������Ժ󣬶�����ʱ��У�����������Ĳ����ͱ������Ƿ�һ��
//�����һ�£�������Ϊ��������ȷ���������������ݸ�������������Ȼ������

//����������޸�
#define FLASH_PARAM_PAGE_START 					((uint32_t)0x08004000)	//16k~17k����
#define FLASH_PARAM_BACKUP_PAGE_START 	((uint32_t)0x08004800)	//18k~19k����

//ÿ������/����ҳ�Ĵ�С
#define SECTOR_SIZE           2048			//�ֽ�
#define FLOOR_MAX							128				//���֧��¥��߶�

extern SYSFloorMsg sysFloorMsg[FLOOR_MAX];			//���ռ̵���ID˳�������е�¥����Ϣ��ռ��1k�ڴ�

//��ָ����ʼ��ַ��ʼ��ȡ�������
void FLASH_ReadData(uint32_t startAddress,uint16_t *readData,uint16_t countToRead);
//������ʼ��������flash��ʼ��ַ����ֹ��ַ�������������ڴ�����
void FLASH_Init(void);
//�������Ŀ�д�����һ������
uint8_t addParam(uint32_t pageAddress, SYSFloorMsg param);
//��flash����һ������
void FLASH_AddOneParam(SYSFloorMsg param);
//�������flash����
void FLASH_CleanAllParam(void);
//ͨ��floorID���relayID
uint8_t convertFloorIDToRelayID(int16_t floorID);
	
#endif /*__FLASH_H__*/



