#include "flash.h"
#include "usart.h"

SYSFloorMsg sysFloorMsg[FLOOR_MAX];

//从指定地址读取半字
uint16_t FLASH_ReadHalfWord(uint32_t address)
{
  return *(__IO uint16_t*)address; 
}
//从指定起始地址开始读取多个数据
void FLASH_ReadData(uint32_t startAddress,uint16_t *readData,uint16_t countToRead)
{
  uint16_t dataIndex;
  for(dataIndex=0;dataIndex<countToRead;dataIndex++)
  {
    readData[dataIndex]=FLASH_ReadHalfWord(startAddress+dataIndex*2);
  }
}
//判断两条数据是否一致,1：不一致，0：一致
//uint8_t isDiff(SYSFloorMsg x, SYSFloorMsg y)
//{
//	if(x.relayID != y.relayID) return 1;
//	if(x.floorID != y.floorID) return 1;
//	if(x.needCard != y.needCard) return 1;
//	if(x.distance != y.distance) return 1;
//	return 0;
//}
//启动初始化，遍历flash起始地址至终止地址，将参数读入内存数组
void FLASH_Init(void)
{
	SYSFloorMsg param,param_bk;
	uint16_t pageOffset;
	int x;
	//初始化sysFloorMsg[128],每个bit位都置为1
	memset(sysFloorMsg, 0xff, FLOOR_MAX*sizeof(SYSFloorMsg));
	//挂载参数表
	sysTaskStatus.paramTable = sysFloorMsg;
	//从起始地址开始到终止地址，读入每一个参数
	for(pageOffset=0;pageOffset<SECTOR_SIZE;pageOffset+=sizeof(SYSFloorMsg))
	{
		//每条数据占据四个半字
		FLASH_ReadData(FLASH_PARAM_PAGE_START+pageOffset,(uint16_t *)&param,4);	
		FLASH_ReadData(FLASH_PARAM_BACKUP_PAGE_START+pageOffset,(uint16_t *)&param_bk,4);
		
		//printf("param: relayID=%d,floorID=%d,needCard=%d,distance=%d\r\n",
		//	param.relayID,param.floorID,param.needCard,param.distance);
		//printf("param_bk: relayID=%d,floorID=%d,needCard=%d,distance=%d\r\n",
		//	param_bk.relayID,param_bk.floorID,param_bk.needCard,param_bk.distance);
		
		if(param.relayID == 0xff && param_bk.relayID != 0xff)
		{
			//数据不一致的情况，备份分区比参数分区多一条参数,需要把这条加上
			addParam(FLASH_PARAM_PAGE_START,param_bk);
		}

		//如果本条数据未被使用，当前PAGE下其后所有数据都未被使用
		if(param_bk.relayID == 0xff)
			break;		
		
		//删除所有已重复的楼层ID,防止参数冲突
		for(x=0;x<FLOOR_MAX;x++)
		{
			if(sysFloorMsg[x].relayID != 0xff	&& sysFloorMsg[x].floorID == param_bk.floorID)
			{
				memset(&sysFloorMsg[x],0xff,sizeof(SYSFloorMsg));
				printf("重置重复楼层参数\r\n");
			}
		}		
		//更新参数,将数据存入内存参数表sysFloorMsg
		printf("读出参数：relayID=%d\r\n",param_bk.relayID);
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

//向扇区的可写块添加一条数据
uint8_t addParam(uint32_t pageAddress, SYSFloorMsg param)
{
	uint16_t data;
	uint16_t pageOffset = 0;
	uint16_t dataIndex;
	//先向备份分区写数据
	//printf("开始寻找偏移\r\n");
	while(1)
	{
		data = FLASH_ReadHalfWord(pageAddress+pageOffset);
		//printf("data = %x , ",(data&0xff));
		if((data & 0xff)==0xff){
			//未被使用
			//printf("未被使用.\r\n");
			break;
		}else{
			//已被使用
			//printf("已被使用.\r\n");
			pageOffset += sizeof(SYSFloorMsg);
		}
		if(pageOffset>=SECTOR_SIZE)
		{
			//当前分区已被写满，写入失败
			printf("当前分区已被写满\r\n");
			return 0;
		}
	}
	FLASH_Unlock();
	printf("写入一条数据\r\n");
  for(dataIndex=0;dataIndex<4;dataIndex++)
	{
		FLASH_ProgramHalfWord(pageAddress+pageOffset+dataIndex*2,((uint16_t*)&param)[dataIndex]);
	}
	FLASH_Lock();
	return 1;
}

//按照内存参数表更新flash参数
void updateFlash(uint32_t pageAddress)
{
	uint16_t x;
	FLASH_Unlock();
	FLASH_ErasePage(pageAddress);
	FLASH_Lock();
	printf("擦除原有分区\r\n");
	for(x=0;x<FLOOR_MAX;x++)
	{
		//找到在用的参数项写入flash
		if((sysFloorMsg[x].relayID & 0x80) == 0)
		{
			addParam(pageAddress,sysFloorMsg[x]);
			printf("写入参数：relayID=%d\r\n",x);
		}
	}
}
//向flash新增一条数据
void FLASH_AddOneParam(SYSFloorMsg param)
{
	//先向备份区增加一条数据
	if(addParam(FLASH_PARAM_BACKUP_PAGE_START,param) == 0)
	{
		printf("备份区增加参数失败，擦除重写\r\n");
		//如果失败了就擦除当前分区然后重新写入
		updateFlash(FLASH_PARAM_BACKUP_PAGE_START);
	}else{
		printf("备份区，通过\r\n");
	}
	//再向正常区增加一条数据
	if(addParam(FLASH_PARAM_PAGE_START,param) == 0)
	{
		printf("正常区增加参数失败，擦除重写\r\n");
		//如果失败了就擦除当前分区然后重新写入
		updateFlash(FLASH_PARAM_PAGE_START);
	}else{
		printf("正常区，通过\r\n");
	}
}
//清除所有flash中保存的参数
void FLASH_CleanAllParam(void)
{
	FLASH_Unlock();
	//擦除正常区
	FLASH_ErasePage(FLASH_PARAM_PAGE_START);
	//擦除备份区
	FLASH_ErasePage(FLASH_PARAM_BACKUP_PAGE_START);
	FLASH_Lock();
}
//通过floorID获得relayID
uint8_t convertFloorIDToRelayID(int16_t floorID)
{
	uint8_t position;
	for(position=RELAY_ID_MIN;position<=RELAY_ID_MAX;position++)
	{
		if(floorID == sysFloorMsg[position].floorID)
			return sysFloorMsg[position].relayID;
	}
	//执行到最后说明没找到，返回0xFF
	return 0xFF;
}
