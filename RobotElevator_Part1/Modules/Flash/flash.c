#include "flash.h"

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
//������ʼ��������flash��ʼ��ַ����ֹ��ַ�������������ڴ�����
void FLASH_ModuleInit(void)
{
	SYSFloorMsg param,param_bk;
	uint16_t pageOffset;
	int x;
	//��ʼ��sysTaskStatus.paramTable[128],ÿ��bitλ����Ϊ1
	memset(sysTaskStatus.paramTable, 0xff, FLOOR_MAX*sizeof(SYSFloorMsg));
	//����ʼ��ַ��ʼ����ֹ��ַ������ÿһ������
	for(pageOffset=0;pageOffset<SECTOR_SIZE;pageOffset+=sizeof(SYSFloorMsg))
	{
		//ÿ������ռ���ĸ�����
		FLASH_ReadData(FLASH_PARAM_PAGE1_START+pageOffset,(uint16_t *)&param,4);	
		FLASH_ReadData(FLASH_PARAM_PAGE2_START+pageOffset,(uint16_t *)&param_bk,4);

		if(param.switchID == 0xff && param_bk.switchID != 0xff)
		{
			//���ݲ�һ�µ���������ݷ����Ȳ���������һ������,��Ҫ����������
			FLASH_AddParam(FLASH_PARAM_PAGE1_START,param_bk);
		}

		//�����������δ��ʹ�ã���ǰPAGE������������ݶ�δ��ʹ��
		if(param_bk.switchID == 0xff)
			break;		
		
		//ɾ���������ظ���¥��ID,��ֹ������ͻ
		for(x=0;x<FLOOR_MAX;x++)
		{
			if(sysTaskStatus.paramTable[x].switchID != 0xff	&& sysTaskStatus.paramTable[x].floorID == param_bk.floorID)
			{
				memset(&sysTaskStatus.paramTable[x],0xff,sizeof(SYSFloorMsg));
				printf("�����ظ�¥�����\r\n");
			}
		}		
		//���²���,�����ݴ����ڴ������sysFloorMsgs
		memcpy(&sysTaskStatus.paramTable[param_bk.switchID],&param_bk,sizeof(SYSFloorMsg));
	}
	for(x=0;x<FLOOR_MAX;x++)
		{
			if(sysTaskStatus.paramTable[x].switchID != 0xff)
			{
				printf("param: switchID=%d,floorID=%d,needCard=%d,distance=%d\r\n",sysTaskStatus.paramTable[x].switchID,
				sysTaskStatus.paramTable[x].floorID,sysTaskStatus.paramTable[x].needCard,sysTaskStatus.paramTable[x].distance);
			}
		}
}

//�������Ŀ�д�����һ������
uint8_t FLASH_AddParam(uint32_t pageAddress, SYSFloorMsg param)
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
void FLASH_UpdateFlash(uint32_t pageAddress)
{
	uint16_t x;
	FLASH_Unlock();
	FLASH_ErasePage(pageAddress);
	FLASH_Lock();
	printf("����ԭ�з���\r\n");
	for(x=0;x<FLOOR_MAX;x++)
	{
		//�ҵ����õĲ�����д��flash
		if((sysTaskStatus.paramTable[x].switchID & 0x80) == 0)
		{
			FLASH_AddParam(pageAddress,sysTaskStatus.paramTable[x]);
			printf("д�������switchID=%d\r\n",x);
		}
	}
}
//��flash����һ������
void FLASH_AddOneParam(SYSFloorMsg param)
{
	//���򱸷�������һ������
	if(FLASH_AddParam(FLASH_PARAM_PAGE2_START,param) == 0)
	{
		printf("���������Ӳ���ʧ�ܣ�������д\r\n");
		//���ʧ���˾Ͳ�����ǰ����Ȼ������д��
		FLASH_UpdateFlash(FLASH_PARAM_PAGE2_START);
	}else{
		printf("��������ͨ��\r\n");
	}
	//��������������һ������
	if(FLASH_AddParam(FLASH_PARAM_PAGE1_START,param) == 0)
	{
		printf("���������Ӳ���ʧ�ܣ�������д\r\n");
		//���ʧ���˾Ͳ�����ǰ����Ȼ������д��
		FLASH_UpdateFlash(FLASH_PARAM_PAGE1_START);
	}else{
		printf("��������ͨ��\r\n");
	}
}
//�������flash�б���Ĳ���
void FLASH_CleanAllParam(void)
{
	FLASH_Unlock();
	//����������
	FLASH_ErasePage(FLASH_PARAM_PAGE1_START);
	//����������
	FLASH_ErasePage(FLASH_PARAM_PAGE2_START);
	FLASH_Lock();
}

