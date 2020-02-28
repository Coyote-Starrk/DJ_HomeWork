#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x.h"
#include <string.h>
#include "sys_param.h"
#include "sys_utils.h"

//Ref:https://www.cnblogs.com/foxclever/p/5784169.html

//硬件最多支持16*8=128层，stm32f103vct6的flash容量为256k，48k内存，硬件每个扇区大小2k，是最小擦除单位
//取一个扇区作为参数分区1，另一个分区作为参数分区2，每次新增参数总是先向分区1添加参数，然后再是分区2
//系统启动起来以后，读参数时会校验分区1和分区2是否一致

//选择两个扇区作为存储参数的位置
#define FLASH_PARAM_PAGE1_START 					((uint32_t)0x08004000)	//16k~17k扇区
#define FLASH_PARAM_PAGE2_START					 	((uint32_t)0x08004800)	//18k~19k扇区

//每个扇区/参数页的大小
#define SECTOR_SIZE           2048			//字节

//从指定地址读取半字
uint16_t FLASH_ReadHalfWord(uint32_t address);
//从指定起始地址开始读取多个数据
void FLASH_ReadData(uint32_t startAddress,uint16_t *readData,uint16_t countToRead);
//按照内存参数表更新flash参数
void FLASH_UpdateFlash(uint32_t pageAddress);
//启动初始化，遍历flash起始地址至终止地址，将参数读入内存数组
void FLASH_ModuleInit(void);
//向扇区的可写块添加一条数据
uint8_t FLASH_AddParam(uint32_t pageAddress, SYSFloorMsg param);
//向flash新增一条数据
void FLASH_AddOneParam(SYSFloorMsg param);
//清除所有flash参数
void FLASH_CleanAllParam(void);
	
#endif /*__FLASH_H__*/



