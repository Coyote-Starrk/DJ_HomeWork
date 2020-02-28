#include "flash.h"

SYSFloorMsg sysFloorMsg[FLOOR_MAX];

//��ָ����ַ��ȡ����
uint16_t FLASH_ReadHalfWord(uint32_t address)
{
  return *(__IO uint16_t*)address; 
}
//��ָ����ʼ��ַ��ʼ��ȡ�������
void FLASH_ReadData(uint32_t startAddress,uint16_t *readData,uint16_t countToRead)
{
  uint16_t dataIndex;
  for(dataIndex=0;dataIndex<countToRead;dataIndex++)
  {
    readData[dataIndex]=FLASH_ReadHalfWord(startAddress+dataIndex*2);
  }
}
//�ж����������Ƿ�һ��,1����һ�£�0��һ��
uint8_t isDiff(SYSFloorMsg x, SYSFloorMsg y)
{
	if(x.relayID != y.relayID) return 1;
	if(x.floorID != y.floorID) return 1;
	if(x.needCard != y.needCard) return 1;
	if(x.distance != y.distance) return 1;
	return 0;
}
//ʹ�ñ��ݷ���������������
void paramPageReset()
{
	uint16_t pageOffset;
	uint16_t data;
	FLASH_ErasePage(FLASH_PARAM_PAGE_START);
	for(pageOffset=0;pageOffset<SECTOR_SIZE;pageOffset+=2)
	{
		data = FLASH_ReadHalfWord(FLASH_PARAM_BACKUP_PAGE_START+pageOffset);
		FLASH_Unlock();
		FLASH_ProgramHalfWord(FLASH_PARAM_PAGE_START+pageOffset,data);
		FLASH_Lock();
	}
}
//������ʼ��������flash��ʼ��ַ����ֹ��ַ�������������ڴ����飨TODO�ȴ��޸Ķ�ȡ�߼���
void FLASH_GetAllParam()
{
	SYSFloorMsg param,param_bk;
	uint16_t pageOffset;
	int x,isNeedReset = 0;
	//��ʼ��sysFloorMsg[128],ÿ��bitλ����Ϊ1
	memset(sysFloorMsg, 0xff, FLOOR_MAX*sizeof(SYSFloorMsg));
	//����ʼ��ַ��ʼ����ֹ��ַ������ÿһ������
	for(pageOffset=0;pageOffset<SECTOR_SIZE;pageOffset+=sizeof(SYSFloorMsg))
	{
		//ÿ������ռ���ĸ�����
		FLASH_ReadData(FLASH_PARAM_PAGE_START,(uint16_t *)&param,4);	
		FLASH_ReadData(FLASH_PARAM_BACKUP_PAGE_START,(uint16_t *)&param_bk,4);
		//���������������һ�£�ʹ�ñ��������ó�����
		if(isDiff(param,param_bk))
			isNeedReset = 1;
		//�����������δ��ʹ�ã���ǰPAGE������������ݶ�δ��ʹ��
		if(param_bk.relayID & 0x80)
			break;		
		//ɾ���������ظ���¥��ID,��ֹ������ͻ
		for(x=0;x<FLOOR_MAX;x++)
			if(sysFloorMsg[x].floorID == param_bk.floorID)
					memset(&sysFloorMsg[x],0xff,sizeof(SYSFloorMsg));
		//���²���,�����ݴ����ڴ������sysFloorMsg
		memcpy(&sysFloorMsg[param_bk.relayID],&param_bk,sizeof(SYSFloorMsg));
	}
	//ִ�в�����Reset
	if(isNeedReset)
		paramPageReset();
}

//�������Ŀ�д�����һ������
uint8_t addParam(uint32_t pageAddress, SYSFloorMsg param)
{
	uint16_t data;
	uint16_t pageOffset = 0;
	uint16_t dataIndex;
	//���򱸷ݷ���д����
	while(1)
	{
		data = FLASH_ReadHalfWord(pageAddress+pageOffset);
		if(data & 0x8000)
			//δ��ʹ��
			break;
		else
			//�ѱ�ʹ��
			pageOffset += sizeof(SYSFloorMsg);
		if(pageOffset>=SECTOR_SIZE)
			//��ǰ�����ѱ�д����д��ʧ��
			return 0;
	}
	FLASH_Unlock();
  for(dataIndex=0;dataIndex<4;dataIndex++)
	{
		FLASH_ProgramHalfWord(pageAddress+pageOffset+dataIndex*2,*((uint16_t*)&param)+dataIndex);
	}
	FLASH_Lock();
	return 1;
}

//�����ڴ���������flash����
void updateFlash(uint32_t pageAddress)
{
	uint16_t x;
	FLASH_ErasePage(pageAddress);
	for(x=0;x<FLOOR_MAX;x++)
	{
		//�ҵ����õĲ�����д��flash
		if((sysFloorMsg[x].relayID & 0x80) == 0)
		{
			addParam(pageAddress,sysFloorMsg[x]);
		}
	}
}
//��flash����һ������
void FLASH_AddOneParam(SYSFloorMsg param)
{
	//���򱸷�������һ������
	if(addParam(FLASH_PARAM_BACKUP_PAGE_START,param) == 0)
	{
		//���ʧ���˾Ͳ�����ǰ����Ȼ������д��
		updateFlash(FLASH_PARAM_BACKUP_PAGE_START);
	}
	//��������������һ������
	if(addParam(FLASH_PARAM_PAGE_START,param) == 0)
	{
		//���ʧ���˾Ͳ�����ǰ����Ȼ������д��
		updateFlash(FLASH_PARAM_PAGE_START);
	}
}
//�������flash�б���Ĳ���
void FLASH_CleanAllParam(void)
{
	//����������
	FLASH_ErasePage(FLASH_PARAM_PAGE_START);
	//����������
	FLASH_ErasePage(FLASH_PARAM_BACKUP_PAGE_START);
}
//ͨ��floorID���relayID
uint8_t convertFloorIDToRelayID(int16_t floorID)
{
	uint8_t position;
	for(position=RELAY_ID_MIN;position<=RELAY_ID_MAX;position++)
	{
		if(floorID == sysFloorMsg[position].floorID)
			return sysFloorMsg[position].relayID;
	}
	//ִ�е����˵��û�ҵ�������0xFF
	return 0xFF;
}
