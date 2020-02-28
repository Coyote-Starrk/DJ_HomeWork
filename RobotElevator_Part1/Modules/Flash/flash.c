#include "flash.h"

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
//启动初始化，遍历flash起始地址至终止地址，将参数读入内存数组
void FLASH_ModuleInit(void)
{
	SYSFloorMsg param,param_bk;
	uint16_t pageOffset;
	int x;
	//初始化sysTaskStatus.paramTable[128],每个bit位都置为1
	memset(sysTaskStatus.paramTable, 0xff, FLOOR_MAX*sizeof(SYSFloorMsg));
	//从起始地址开始到终止地址，读入每一个参数
	for(pageOffset=0;pageOffset<SECTOR_SIZE;pageOffset+=sizeof(SYSFloorMsg))
	{
		//每条数据占据四个半字
		FLASH_ReadData(FLASH_PARAM_PAGE1_START+pageOffset,(uint16_t *)&param,4);	
		FLASH_ReadData(FLASH_PARAM_PAGE2_START+pageOffset,(uint16_t *)&param_bk,4);

		if(param.switchID == 0xff && param_bk.switchID != 0xff)
		{
			//数据不一致的情况，备份分区比参数分区多一条参数,需要把这条加上
			FLASH_AddParam(FLASH_PARAM_PAGE1_START,param_bk);
		}

		//如果本条数据未被使用，当前PAGE下其后所有数据都未被使用
		if(param_bk.switchID == 0xff)
			break;		
		
		//删除所有已重复的楼层ID,防止参数冲突
		for(x=0;x<FLOOR_MAX;x++)
		{
			if(sysTaskStatus.paramTable[x].switchID != 0xff	&& sysTaskStatus.paramTable[x].floorID == param_bk.floorID)
			{
				memset(&sysTaskStatus.paramTable[x],0xff,sizeof(SYSFloorMsg));
				printf("重置重复楼层参数\r\n");
			}
		}		
		//更新参数,将数据存入内存参数表sysFloorMsgs
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

//向扇区的可写块添加一条数据
uint8_t FLASH_AddParam(uint32_t pageAddress, SYSFloorMsg param)
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
void FLASH_UpdateFlash(uint32_t pageAddress)
{
	uint16_t x;
	FLASH_Unlock();
	FLASH_ErasePage(pageAddress);
	FLASH_Lock();
	printf("擦除原有分区\r\n");
	for(x=0;x<FLOOR_MAX;x++)
	{
		//找到在用的参数项写入flash
		if((sysTaskStatus.paramTable[x].switchID & 0x80) == 0)
		{
			FLASH_AddParam(pageAddress,sysTaskStatus.paramTable[x]);
			printf("写入参数：switchID=%d\r\n",x);
		}
	}
}
//向flash新增一条数据
void FLASH_AddOneParam(SYSFloorMsg param)
{
	//先向备份区增加一条数据
	if(FLASH_AddParam(FLASH_PARAM_PAGE2_START,param) == 0)
	{
		printf("备份区增加参数失败，擦除重写\r\n");
		//如果失败了就擦除当前分区然后重新写入
		FLASH_UpdateFlash(FLASH_PARAM_PAGE2_START);
	}else{
		printf("备份区，通过\r\n");
	}
	//再向正常区增加一条数据
	if(FLASH_AddParam(FLASH_PARAM_PAGE1_START,param) == 0)
	{
		printf("正常区增加参数失败，擦除重写\r\n");
		//如果失败了就擦除当前分区然后重新写入
		FLASH_UpdateFlash(FLASH_PARAM_PAGE1_START);
	}else{
		printf("正常区，通过\r\n");
	}
}
//清除所有flash中保存的参数
void FLASH_CleanAllParam(void)
{
	FLASH_Unlock();
	//擦除正常区
	FLASH_ErasePage(FLASH_PARAM_PAGE1_START);
	//擦除备份区
	FLASH_ErasePage(FLASH_PARAM_PAGE2_START);
	FLASH_Lock();
}

