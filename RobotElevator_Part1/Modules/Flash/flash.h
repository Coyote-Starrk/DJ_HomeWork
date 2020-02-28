#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x.h"
#include <string.h>
#include "sys_param.h"

//Ref:https://www.cnblogs.com/foxclever/p/5784169.html

//硬件最多支持16*8=128层，stm32f103vct6的flash容量为256k，48k内存，硬件每个扇区大小2k，是最小擦除单位
//取一个扇区作为正常参数分区，另一个分区作为参数备份分区，每次新增参数总是先向备份分区添加参数，然后再是正常分区
//系统启动起来以后，读参数时会校验正常分区的参数和备份区是否一致
//如果不一致，总是认为备份区正确，并将备份区数据覆盖至正常分区然后启动

//具体参数待修改
#define FLASH_PARAM_PAGE_START 					((uint32_t)0x00000400)
#define FLASH_PARAM_BACKUP_PAGE_START 	((uint32_t)0x00000400)

//每个扇区/参数页的大小
#define SECTOR_SIZE           2048			//字节
#define FLOOR_MAX							128				//最大支持楼层高度

extern SYSFloorMsg sysFloorMsg[FLOOR_MAX];			//按照继电器ID顺序存放所有的楼层信息，占用1k内存

//启动初始化，遍历flash起始地址至终止地址，将参数读入内存数组
void FLASH_GetAllParam(void);
//向flash新增一条数据
void FLASH_AddOneParam(SYSFloorMsg param);
//清除所有flash参数
void FLASH_CleanAllParam(void);
//通过floorID获得relayID
uint8_t convertFloorIDToRelayID(int16_t floorID);
	
#endif /*__FLASH_H__*/



