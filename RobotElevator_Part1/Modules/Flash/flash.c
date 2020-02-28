#include "flash.h"
#include "usart.h"

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
//uint8_t isDiff(SYSFloorMsg x, SYSFloorMsg y)
//{
//	if(x.relayID != y.relayID) return 1;
//	if(x.floorID != y.floorID) return 1;
//	if(x.needCard != y.needCard) return 1;
//	if(x.distance != y.distance) return 1;
//	return 0;
//}
//������ʼ��������flash��ʼ��ַ����ֹ��ַ�������������ڴ�����
void FLASH_Init(void)
{
	SYSFloorMsg param,param_bk;
	uint16_t pageOffset;
	int x;
	//��ʼ��sysFloorMsg[128],ÿ��bitλ����Ϊ1
	memset(sysFloorMsg, 0xff, FLOOR_MAX*sizeof(SYSFloorMsg));
	//���ز�����
	sysTaskStatus.paramTable = sysFloorMsg;
	//����ʼ��ַ��ʼ����ֹ��ַ������ÿһ������
	for(pageOffset=0;pageOffset<SECTOR_SIZE;pageOffset+=sizeof(SYSFloorMsg))
	{
		//ÿ������ռ���ĸ�����
		FLASH_ReadData(FLASH_PARAM_PAGE_START+pageOffset,(uint16_t *)&param,4);	
		FLASH_ReadData(FLASH_PARAM_BACKUP_PAGE_START+pageOffset,(uint16_t *)&param_bk,4);
		
		//printf("param: relayID=%d,floorID=%d,needCard=%d,distance=%d\r\n",
		//	param.relayID,param.floorID,param.needCard,param.distance);
		//printf("param_bk: relayID=%d,floorID=%d,needCard=%d,distance=%d\r\n",
		//	param_bk.relayID,param_bk.floorID,param_bk.needCard,param_bk.distance);
		
		if(param.relayID == 0xff && param_bk.relayID != 0xff)
		{
			//���ݲ�һ�µ���������ݷ����Ȳ���������һ������,��Ҫ����������
			addParam(FLASH_PARAM_PAGE_START,param_bk);
		}

		//�����������δ��ʹ�ã���ǰPAGE������������ݶ�δ��ʹ��
		if(param_bk.relayID == 0xff)
			break;		
		
		//ɾ���������ظ���¥��ID,��ֹ������ͻ
		for(x=0;x<FLOOR_MAX;x++)
		{
			if(sysFloorMsg[x].relayID != 0xff	&& sysFloorMsg[x].floorID == param_bk.floorID)
			{
				memset(&sysFloorMsg[x],0xff,sizeof(SYSFloorMsg));
				printf("�����ظ�¥�����\r\n");
			}
		}		
		//���²���,�����ݴ����ڴ������sysFloorMsg
		printf("����������relayID=%d\r\n",param_bk.relayID);
		memcpy(&sysFloorMsg[param_bk.relayID],&param_bk,sizeof(SYSFloorMsg));
	}
	for(x=0;x<FLOOR_MAX;x++)
		{
			if(sysFloorMsg[x].relayID != 0xff)
			{
				printf("param: relayID=%d,floorID=%d,needCard=%d,distance=%d\r\n",
					sysFloorMsg[x].relayID,sysFloorMsg[x].floorID,sysFloorMsg[x].needCard,sysFloorMsg[x].distance);
			}
		}
}

//�������Ŀ�д�����һ������
uint8_t addParam(uint32_t pageAddress, SYSFloorMsg param)
{
	uint16_t data;
	uint16_t pageOffset = 0;
	uint16_t dataIndex;
	//���򱸷ݷ���д����
	//printf("��ʼѰ��ƫ��\r\n");
	while(1)
	{
		data = FLASH_ReadHalfWord(pageAddress+pageOffset);
		//printf("data = %x , ",(data&0xff));
		if((data & 0xff)==0xff){
			//δ��ʹ��
			//printf("δ��ʹ��.\r\n");
			break;
		}else{
			//�ѱ�ʹ��
			//printf("�ѱ�ʹ��.\r\n");
			pageOffset += sizeof(SYSFloorMsg);
		}
		if(pageOffset>=SECTOR_SIZE)
		{
			//��ǰ�����ѱ�д����д��ʧ��
			printf("��ǰ�����ѱ�д��\r\n");
			return 0;
		}
	}
	FLASH_Unlock();
	printf("д��һ������\r\n");
  for(dataIndex=0;dataIndex<4;dataIndex++)
	{
		FLASH_ProgramHalfWord(pageAddress+pageOffset+dataIndex*2,((uint16_t*)&param)[dataIndex]);
	}
	FLASH_Lock();
	return 1;
}

//�����ڴ���������flash����
void updateFlash(uint32_t pageAddress)
{
	uint16_t x;
	FLASH_Unlock();
	FLASH_ErasePage(pageAddress);
	FLASH_Lock();
	printf("����ԭ�з���\r\n");
	for(x=0;x<FLOOR_MAX;x++)
	{
		//�ҵ����õĲ�����д��flash
		if((sysFloorMsg[x].relayID & 0x80) == 0)
		{
			addParam(pageAddress,sysFloorMsg[x]);
			printf("д�������relayID=%d\r\n",x);
		}
	}
}
//��flash����һ������
void FLASH_AddOneParam(SYSFloorMsg param)
{
	//���򱸷�������һ������
	if(addParam(FLASH_PARAM_BACKUP_PAGE_START,param) == 0)
	{
		printf("���������Ӳ���ʧ�ܣ�������д\r\n");
		//���ʧ���˾Ͳ�����ǰ����Ȼ������д��
		updateFlash(FLASH_PARAM_BACKUP_PAGE_START);
	}else{
		printf("��������ͨ��\r\n");
	}
	//��������������һ������
	if(addParam(FLASH_PARAM_PAGE_START,param) == 0)
	{
		printf("���������Ӳ���ʧ�ܣ�������д\r\n");
		//���ʧ���˾Ͳ�����ǰ����Ȼ������д��
		updateFlash(FLASH_PARAM_PAGE_START);
	}else{
		printf("��������ͨ��\r\n");
	}
}
//�������flash�б���Ĳ���
void FLASH_CleanAllParam(void)
{
	FLASH_Unlock();
	//����������
	FLASH_ErasePage(FLASH_PARAM_PAGE_START);
	//����������
	FLASH_ErasePage(FLASH_PARAM_BACKUP_PAGE_START);
	FLASH_Lock();
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
