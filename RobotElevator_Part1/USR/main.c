#include "stm32f10x_it.h"
#include "lora.h"
#include "uwb.h"
#include "gpio.h"
#include "sys_utils.h"
#include "usart.h"
#include "flash.h"
#include "string.h"
#include "sys_param.h"

#include <stdio.h>

//进程参数统一管理
SYSTaskStatus sysTaskStatus;

//系统初始化
void systemInit(void);
//判断请求参数是否合法
uint8_t paramIsVaild(int16_t floorNow,int16_t floorDst);
//计算当前电梯的当前楼层/停靠状态，返回值是0~127，对应楼层继电器ID，返回-1代表电梯还在移动
int8_t calCurFloor(void);
//由任务大小超时导致系统异常退出的，任务重置函数
void sysMissionReset(void);
//根据clk和event刷新GPIO输出
void GPIO_ControlUpdate(void);
//获得当前distance对应的楼层位置
int16_t getCurrentFloor(void);
//消息处理
void msgHandler(SYS_MsgHead* msg);

//用于记录上一个GPIO操作的时间点
static uint32_t CLK_preGpioCtrl = 0;
//0-非刷卡循环 1~4 floorNow层循环 5~8 floorDst层循环
static uint8_t cardPeriod = 0;		
//保存上一次的distanceFIFO[5]更新时间
static uint32_t CLK_LastFifoUpdate = 0;
//保存上一次报文接收的时间
static uint32_t CLK_LastMsg = 0;
//用于记录开门的时刻，长超时
static uint32_t CLK_DoorOpen;	

int main_t()
{

	int i=0;
	//延时函数初始化
	delay_Init();
	//gpio初始化
	GPIO_Usr_Init();
	//这个是调试打印的输出串口，完成编码后会删掉
	uart_init(9600);

	printf("\r\nwhile start\r\n");
	while(1)
	{
		//printf("\r\nloop start\r\n");
		if(i==0){
			LED_OFF;
			i=1;
		}else{
			LED_ON;
			i=0;
		}
		delay_ms(1000);
	}
}

int main(void)
{			
	//系统初始化
	//void systemInit();
	//FLASH_CleanAllParam();
	//这个是调试打印的输出串口，完成编码后会删掉
	uart_init(9600);
	//flash初始化
	FLASH_Init();
	//初始化状态机
	sysTaskStatus.sysStatus = 0xD0;
	//定时器控制初始化
	TIMER_Init();
	//延时函数初始化
	delay_Init();
	//gpio初始化
	GPIO_Usr_Init();
	//UWB模块初始化
	UWB_Init();
	//LORA模块初始化
	Lora_Init();
	
	printf("\r\nwhile start\r\n");
	
	while(1)
  {
		uint16_t dataSize,payloadLen;
		//delay_ms(1000);
		//SYSEvent_Clr(Event_Need_Report_Status);
		//一、更新一次UWB读取到的距离信息	
		while(sysTaskStatus.uwbBufRead != sysTaskStatus.uwbBufTail &&
					(sysTaskStatus.uwbBufRead+1)%RECV_BUFFER_MAX_SIZE != sysTaskStatus.uwbBufTail)
		{
			if(sysTaskStatus.UWB_RX_BUFF[(sysTaskStatus.uwbBufRead+1)%RECV_BUFFER_MAX_SIZE] == 'm' &&
				 sysTaskStatus.UWB_RX_BUFF[(sysTaskStatus.uwbBufRead+2)%RECV_BUFFER_MAX_SIZE] == 'c')
			{
				//计算当前Read指针之后缓冲区长度（用于接下来判断缓冲区是否已经接收了整个测距报文）
				for(dataSize=2;(sysTaskStatus.uwbBufRead+dataSize)%RECV_BUFFER_MAX_SIZE != sysTaskStatus.uwbBufTail;)
					dataSize++;
				if(dataSize >= UWB_DATA_LEN)
				{
					//将本包数据拷贝到待处理缓冲区
					size_t tmpOffset;
					for(tmpOffset=1;tmpOffset<=UWB_DATA_LEN;tmpOffset++)
					{
						sysTaskStatus.TMP_BUFFER[tmpOffset-1] = sysTaskStatus.UWB_RX_BUFF[(sysTaskStatus.uwbBufRead+1)%RECV_BUFFER_MAX_SIZE];
						sysTaskStatus.uwbBufRead = (sysTaskStatus.uwbBufRead+1)%RECV_BUFFER_MAX_SIZE;
					}
#if 0					
					printf("\r\n待计算数据为：\r\n");
					for(tmpOffset=0;tmpOffset<UWB_DATA_LEN;tmpOffset++)
					{
						USART_SendData(USART1, sysTaskStatus.TMP_BUFFER[tmpOffset]);
						while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
					}
					printf("\r\n开始计算距离\r\n");
					//1.3 根据报文计算实时距离
					printf("READ:%d - ",sysTaskStatus.uwbBufRead);
#endif					
					UWB_getDistance((char*)sysTaskStatus.TMP_BUFFER);
				}else{
					break;
				}
			}else{
				sysTaskStatus.uwbBufRead = (sysTaskStatus.uwbBufRead+1)%RECV_BUFFER_MAX_SIZE;
			}
		}
		//1.4 每隔2500ms采样更新FIFO
		//FIFO用来保存最近的5个实时距离数据，每次采样间隔2500ms，如果5个数据的取值都在某一楼层范围内，说明电梯停靠
		if(sysTaskStatus.clk - CLK_LastFifoUpdate >= 50)	
		{
			uint8_t i;
			//FIFO的0号位置保存的是最久的数据，4号位置保存最新更新的数据
			for(i=1;i<=4;i++)
				sysTaskStatus.distanceFIFO[i-1] = sysTaskStatus.distanceFIFO[i];
			sysTaskStatus.distanceFIFO[4] = sysTaskStatus.distance;
			//更新fifoClk
			CLK_LastFifoUpdate = sysTaskStatus.clk;
			//printf("FIFO update:%d-%d-%d-%d-%d\r\n",sysTaskStatus.distanceFIFO[0],sysTaskStatus.distanceFIFO[1],
			//			sysTaskStatus.distanceFIFO[2],sysTaskStatus.distanceFIFO[3],sysTaskStatus.distanceFIFO[4]);
		}		
		//二、获取一帧从LORA缓冲区接收到的报文
		
		//2.1 遍历接收缓冲区，搜索匹配报文头部HEAD1,HEAD2
		//当Read指针后驱两个字节位置有数据
		//printf("\r\nHEAD:%d，TAIL:%d，READ:%d\r\n",sysTaskStatus.loraBufHead,sysTaskStatus.loraBufTail,sysTaskStatus.loraBufRead);
		while(sysTaskStatus.loraBufRead != sysTaskStatus.loraBufTail &&
					(sysTaskStatus.loraBufRead+1)%RECV_BUFFER_MAX_SIZE != sysTaskStatus.loraBufTail)
		{
			//printf("\r\nHEAD:%d，TAIL:%d，READ:%d\r\n",sysTaskStatus.loraBufHead,sysTaskStatus.loraBufTail,sysTaskStatus.loraBufRead);
			//delay_ms(1000);	
			//printf("1. 开始搜索协议包头HEAD1,HEAD2\r\n");
			//并且数据刚好为协议报文固定头部HEAD1,HEAD2的时候，能够确认报文在缓冲区中的位置
			if(sysTaskStatus.LORA_RX_BUFF[(sysTaskStatus.loraBufRead+1)%RECV_BUFFER_MAX_SIZE] == SYS_MSG_HEAD1 &&
				 sysTaskStatus.LORA_RX_BUFF[(sysTaskStatus.loraBufRead+2)%RECV_BUFFER_MAX_SIZE] == SYS_MSG_HEAD2)
			{
				
				//2.2 计算当前Read指针之后缓冲区长度（用于接下来判断缓冲区是否已经接收了整个报文）
				for(dataSize=2;(sysTaskStatus.loraBufRead+dataSize)%RECV_BUFFER_MAX_SIZE != sysTaskStatus.loraBufTail;)
					dataSize++;
				
				//printf("2. 找到了协议包头HEAD1,HEAD2，READ：%d，TAIL：%d，可读报文长度：%d\r\n",
				//				sysTaskStatus.loraBufRead,sysTaskStatus.loraBufTail,dataSize);
				
				//如果已有数据长度大于包头长度
				if(dataSize >= sizeof(SYS_MsgHead))
				{
					//将包头拷贝到待处理缓冲区
					size_t tmpOffset;
					uint8_t headCRC,payloadCRC;
					//将本包数据拷贝到待处理缓冲区
					for(tmpOffset=1;tmpOffset<=sizeof(SYS_MsgHead);tmpOffset++)
						sysTaskStatus.TMP_BUFFER[tmpOffset-1] = sysTaskStatus.LORA_RX_BUFF[(sysTaskStatus.loraBufRead+tmpOffset)%RECV_BUFFER_MAX_SIZE];
					//根据协议头部计算headCRC
					headCRC = crc8((uint8_t*)sysTaskStatus.TMP_BUFFER,HEAD_CRC_SIZE);
					//校验headCRC
					if(((SYS_MsgHead*)sysTaskStatus.TMP_BUFFER)->headCRC == headCRC)
					{
						//头部校验通过
						payloadLen = ((SYS_MsgHead*)sysTaskStatus.TMP_BUFFER)->payloadLen;
						//printf("由消息头部获得payload的长度为：%d\r\n",payloadLen);
						
						//继续判断是否接收到了整个数据包
						if(dataSize >= payloadLen + sizeof(SYS_MsgHead))
						{
							//数据包已经接受完整
							//将本包数据拷贝到待处理缓冲区
							for(tmpOffset=1;tmpOffset<=payloadLen+sizeof(SYS_MsgHead);tmpOffset++)
								sysTaskStatus.TMP_BUFFER[tmpOffset-1] = sysTaskStatus.LORA_RX_BUFF[(sysTaskStatus.loraBufRead+tmpOffset)%RECV_BUFFER_MAX_SIZE];
							//计算payloadCRC
							payloadCRC = crc8(((uint8_t*)sysTaskStatus.TMP_BUFFER)+sizeof(SYS_MsgHead),payloadLen);	
							//校验payload
							if(((SYS_MsgHead*)sysTaskStatus.TMP_BUFFER)->payloadCRC == payloadCRC)
							{
								//payload部分校验通过
								//2.4 跳入消息处理函数，执行报文处理
								msgHandler((SYS_MsgHead*)sysTaskStatus.TMP_BUFFER);
								//通知状态机，需要在流程的末尾向机器人上报一次自身状态
								//printf("Deal msg;\r\n");
								//移动read指针
								sysTaskStatus.loraBufRead = (sysTaskStatus.loraBufRead+sizeof(SYS_MsgHead)+payloadLen)%RECV_BUFFER_MAX_SIZE;
								//printf("将read指针移动到数据尾HEAD:%d，TAIL:%d，READ:%d\r\n",sysTaskStatus.loraBufHead,sysTaskStatus.loraBufTail,sysTaskStatus.loraBufRead);
							}else{
								//payload部分校验不通过
								printf("协议payload校验失败\r\n");
								//将Read指针移动到下一个字节位置开始重新匹配
								sysTaskStatus.loraBufRead = (sysTaskStatus.loraBufRead+1)%RECV_BUFFER_MAX_SIZE;
							}
						}else{
							printf("协议payload未接受完成\r\n");
							//数据包payload部分未接受完成,需要跳出循环，等下次接受完一帧后再处理
							break;
						}	
					}else{
						//头部校验不通过
						printf("协议头部校验失败\r\n");
						//将Read指针移动到下一个字节位置开始重新匹配
						sysTaskStatus.loraBufRead = (sysTaskStatus.loraBufRead+1)%RECV_BUFFER_MAX_SIZE;
					}
				}else{
					printf("协议头部未接受完整\r\n");
					//如果已有数据长度小于包头长度，说明数据尚未接受完整，需要跳出循环，等下次接受完一帧后再处理	
					break;
				}					
			}else{
				//如果head1，head2不匹配，将Read指针移动到下一个字节位置开始重新匹配
				sysTaskStatus.loraBufRead = (sysTaskStatus.loraBufRead+1)%RECV_BUFFER_MAX_SIZE;
			}
		}
	
		//三、根据clk更新GPIO输出控制
		GPIO_ControlUpdate();
		
		//四、电梯状态机
		if(sysTaskStatus.sysStatus == 0xD0)				//电梯处于空闲态
		{
			//如果有机器人请求空闲状态下的电梯，则让电梯转入0xD1状态
			if(sysTaskStatus.event & Event_Robot_Call)
			{
				sysTaskStatus.sysStatus = 0xD1;
			}
		}
		else if(sysTaskStatus.sysStatus == 0xD1)	//电梯正在向机器人所在楼层移动
		{
			//计算电梯当前停靠状态
			int8_t res = calCurFloor();
			if(res == -1)
			{
				//电梯尚未停靠，开始循环按下机器人所在楼层按钮（函数可以多次调用）
				SYSEvent_Set(Event_CirTrigger_FloorNow);
				printf("电梯未停靠，开始触发第一个\r\n");
			}else{
				//电梯停靠了
				if(sysTaskStatus.paramTable[res].floorID == sysTaskStatus.floorNow)
				{
					//电梯已经停靠到了第一个任务楼层，开始循环按下第一个任务楼层的按钮，允许重复调用
					SYSEvent_Clr(Event_CirTrigger_FloorNow);
					printf("停止触发第一个\r\n");
					//按下开门按键
					DOOR_OPEN;
					//记录开门时间
					CLK_DoorOpen = sysTaskStatus.clk;
					//修改自身状态
					sysTaskStatus.sysStatus = 0xD2;
				}else{
					//电梯停靠在了任务楼层之外的其他楼层，循环按下机器人所在楼层按钮（函数可以多次调用）
					SYSEvent_Set(Event_CirTrigger_FloorNow);
					printf("电梯停靠在别的楼层，开始触发第一个\r\n");
				}
			}
		}
		else if(sysTaskStatus.sysStatus == 0xD2)	//电梯已经到达机器人所在楼层，等待机器人进入
		{
			if(sysTaskStatus.clk - CLK_DoorOpen > 1200)
			{
				printf("开门超时断连\r\n");
				//如果已经保持开门状态超过50*200=10000ms，认为连接超时，执行断连流程，一分钟
				sysMissionReset();
				//开门失能
				DOOR_CLOSE;
			}else{
				//未超时，保持按下开门按键
				DOOR_OPEN;
				//如果有机器人已经进入电梯，则让电梯转入0xD1状态
				if(sysTaskStatus.event & Event_Robot_In)
				{
					sysTaskStatus.sysStatus = 0xD3;
					DOOR_CLOSE;
				}
			}
		}
		else if(sysTaskStatus.sysStatus == 0xD3)	//电梯正在向机器人任务要求楼层移动
		{
			//计算电梯当前停靠状态
			int8_t res = calCurFloor();
			if(res == -1)
			{
				//电梯尚未停靠
				SYSEvent_Set(Event_CirTrigger_FloorDst);
				printf("电梯未停靠，开始触发第二个\r\n");
			}else{
				//电梯停靠了
				if(sysTaskStatus.paramTable[res].floorID == sysTaskStatus.floorDst)
				{
					//电梯已经停靠到了第二个任务楼层
					SYSEvent_Clr(Event_CirTrigger_FloorDst);
					printf("停止触发第二个\r\n");
					//按下开门按键
					DOOR_OPEN;
					//记录开门时间
					CLK_DoorOpen = sysTaskStatus.clk;
					//修改自身状态
					sysTaskStatus.sysStatus = 0xD4;
				}else{
					//电梯停靠在了任务楼层之外的其他楼层
					SYSEvent_Set(Event_CirTrigger_FloorDst);
					printf("电梯停靠在别的楼层，开始触发第二个\r\n");
				}
			}
		}
		else if(sysTaskStatus.sysStatus == 0xD4)	//电梯已经到达机器人目的楼层，等待机器人退出
		{
			if(sysTaskStatus.clk - CLK_DoorOpen > 1200)
			{
				printf("1min开门超时断连\r\n");
				//如果已经保持开门状态超过50*200=10000ms，认为连接超时，执行断连流程，一分钟
				sysMissionReset();
				//开门失能
				DOOR_CLOSE;
			}else{
				//未超时，保持按下开门按键
				DOOR_OPEN;
				//如果有机器人已经离开电梯，则让电梯转入0xD0状态
				if(sysTaskStatus.event & Event_Robot_Out)
				{
					sysTaskStatus.sysStatus = 0xD0;
					DOOR_CLOSE;
					SYSEvent_Set(Event_Quit_Mission);
				}
			}
		}

		//printf("检查是否需要上报报文\r\n");
		//根据标志位决定是否回复或上报电梯状态
		if(sysTaskStatus.event & Event_Need_Report_Status)
		{
			ADDR_ToSend replyAddr;
			SYS_MsgHead replyMsg;
			Payload_RobotMsg replyPayload;
			
			//更新报文接收时间
			CLK_LastMsg = sysTaskStatus.clk;
			//向机器人上报自身状态
	
			//填充报文回复地址
			replyAddr.addrH = sysTaskStatus.srcAddrH;
			replyAddr.addrL = sysTaskStatus.srcAddrL;
			replyAddr.channel = sysTaskStatus.srcChannel;
			//填充消息负载字段
			replyPayload.cmdID = sysTaskStatus.sysStatus;
			replyPayload.Reserved = 0;
			replyPayload.taskID = sysTaskStatus.taskID;
			replyPayload.floorNow = getCurrentFloor();
			if(sysTaskStatus.sysStatus == 0xD0 || sysTaskStatus.sysStatus == 0xD1)
				replyPayload.floorDst = sysTaskStatus.floorNow;
			else
				replyPayload.floorDst = sysTaskStatus.floorDst;
			//填充报文协议首部分固定字段
			replyMsg.head1 = SYS_MSG_HEAD1;
			replyMsg.head2 = SYS_MSG_HEAD2;
			replyMsg.sequence = sysTaskStatus.sequence+1;
			replyMsg.srcAddrH = sysTaskStatus.localAddrH;
			replyMsg.srcAddrL = sysTaskStatus.localAddrL;
			replyMsg.srcChannel = sysTaskStatus.localChannel;
			replyMsg.timeStamp1 = 0;
			replyMsg.timeStamp2 = 0;
			replyMsg.msgType = ELEVATOR_TO_ROBOT;
			replyMsg.payloadLen = sizeof(Payload_RobotMsg);
			replyMsg.headCRC = crc8((uint8_t*)&replyMsg,HEAD_CRC_SIZE);
			replyMsg.payloadCRC = crc8((uint8_t*)&replyPayload,sizeof(Payload_RobotMsg));
			//发送报文
			//printf("send msg\r\n");
			printf("状态机之后回复报文：cmdID=%x,taskID=%d,floorNow=%d,floorDst=%d,status=%x\r\n",replyPayload.cmdID,replyPayload.taskID,
					replyPayload.floorNow,replyPayload.floorDst,sysTaskStatus.sysStatus);
			LORA_SendMsg(replyAddr,replyMsg,(char*)&replyPayload,sizeof(Payload_RobotMsg));
			//清除事件通知
			SYSEvent_Clr(Event_Need_Report_Status);
			//如果这是一帧a5报文的回复，执行完这帧回复之后需要执行重置流程
			if(sysTaskStatus.event & Event_Quit_Mission)
				sysMissionReset();	
		}		
		//轿厢如果在运行态，并且超过100*50=5000ms未收到机器人方的信息，走断连流程
		if(sysTaskStatus.sysStatus != 0xD0 && sysTaskStatus.clk - CLK_LastMsg > 200)
		{
			printf("10s消息间隔超时断连\r\n");
			//重置运行状态
			sysMissionReset();
		}
		//每个循环延时20ms
		delay_ms(20);
  }//end while(1)
	//return 0;
}
 
//系统初始化
void systemInit(void)
{
	//挂载参数表
	sysTaskStatus.paramTable = sysFloorMsg;
	//定时器控制初始化
	TIMER_Init();
	//延时函数初始化
	delay_Init();
	//gpio初始化
	GPIO_Usr_Init();
	//这个是调试打印的输出串口，完成编码后会删掉
	uart_init(9600);
	//UWB模块初始化
	//UWB_Init();
	//LORA模块初始化
	//Lora_Init();
}
//判断请求参数是否合法
uint8_t paramIsVaild(int16_t floorNow,int16_t floorDst)
{
	uint8_t position;
	uint8_t isFloorNowExist=0,isFloorDstExist=0;
	for(position=RELAY_ID_MIN;position<=RELAY_ID_MAX;position++)
	{
		if(floorNow == sysTaskStatus.paramTable[position].floorID)
			isFloorNowExist = 1;
		if(floorDst == sysTaskStatus.paramTable[position].floorID)
			isFloorDstExist = 1;
	}
	if(isFloorNowExist && isFloorDstExist)
		return 1;
	return 0;
}
//计算当前电梯的停靠状态，返回值是0~127，对应楼层继电器ID，返回-1代表电梯还在移动
int8_t calCurFloor(void)
{
	uint8_t relayID;
	uint32_t upBound,loBound;
	for(relayID = RELAY_ID_MIN;relayID<=RELAY_ID_MAX;relayID++)
	{
		//在参数表中寻找和FIFO[0]对应的楼层
		if(sysTaskStatus.distanceFIFO[0] < sysTaskStatus.paramTable[relayID].distance + TOLERABLE_CAL_IS_STOP_ERR && 
				sysTaskStatus.distanceFIFO[0] > sysTaskStatus.paramTable[relayID].distance - TOLERABLE_CAL_IS_STOP_ERR)
			break;
	}
	//没找到
	if(relayID == RELAY_ID_MAX+1) return -1;

	//执行到这里relayID变量中保存了对应FIFO[0]的停靠楼层，接下来验证FIFO[1/2/3/4]的楼层是否和FIFO[0]一致
	upBound = sysTaskStatus.paramTable[relayID].distance + TOLERABLE_CAL_IS_STOP_ERR;
	loBound = sysTaskStatus.paramTable[relayID].distance - TOLERABLE_CAL_IS_STOP_ERR;
	if(sysTaskStatus.distanceFIFO[1] < upBound && sysTaskStatus.distanceFIFO[1] > loBound &&
		 sysTaskStatus.distanceFIFO[2] < upBound && sysTaskStatus.distanceFIFO[2] > loBound &&
		 sysTaskStatus.distanceFIFO[3] < upBound && sysTaskStatus.distanceFIFO[3] > loBound &&
		 sysTaskStatus.distanceFIFO[4] < upBound && sysTaskStatus.distanceFIFO[4] > loBound )
		return relayID;
	return -1;
}
//获得当前distance对应的楼层位置
int16_t getCurrentFloor(void)
{
	uint8_t relayID;
	for(relayID = RELAY_ID_MIN;relayID<=RELAY_ID_MAX;relayID++)
	{
		//在参数表中寻找和FIFO[0]对应的楼层
		if(sysTaskStatus.paramTable[relayID].relayID != 0xff)
		{	
			if(sysTaskStatus.distance < sysTaskStatus.paramTable[relayID].distance + TOLERABLE_CAL_IS_STOP_ERR &&
				sysTaskStatus.distance > sysTaskStatus.paramTable[relayID].distance - TOLERABLE_CAL_IS_STOP_ERR)
			{
				return sysTaskStatus.paramTable[relayID].floorID;
			}
		}
	}
	return 0xffff;
}
//由任务大小超时导致系统异常退出的，任务重置函数
void sysMissionReset(void)
{
	//清空所有事件
	sysTaskStatus.event = Event_0;
	//重置系统任务
	sysTaskStatus.srcAddrH = 0;
	sysTaskStatus.srcAddrL = 0;
	sysTaskStatus.srcChannel = 0;
	sysTaskStatus.floorDst = 0;
	sysTaskStatus.floorNow = 0;
	sysTaskStatus.taskID = 0;
	//系统状态转移到空闲态
	sysTaskStatus.sysStatus = 0xD0;
	//保证OPEN_DOOR的gpio输出为低电平
	DOOR_CLOSE;
}
//根据clk和event刷新GPIO输出
void GPIO_ControlUpdate(void)
{
	uint32_t clk = sysTaskStatus.clk;
	//开始进入流程
	if(cardPeriod == 0 && (sysTaskStatus.event & Event_CirTrigger_FloorNow || sysTaskStatus.event & Event_CirTrigger_FloorDst))
	{
		//更新时间
		CLK_preGpioCtrl = clk;
		printf("start time clk = %d\r\n",CLK_preGpioCtrl);
		//记录刷卡流程
		if(sysTaskStatus.event & Event_CirTrigger_FloorNow) cardPeriod = 1;
		if(sysTaskStatus.event & Event_CirTrigger_FloorDst) cardPeriod = 5;
		
		if(cardPeriod == 1 && sysTaskStatus.paramTable[convertFloorIDToRelayID(sysTaskStatus.floorNow)].needCard == 1)
		{
			//控制刷卡继电器拉高电平（TODO）
			printf("楼层%d刷卡失能,对应继电器%d,clk=%d\r\n",sysTaskStatus.floorNow,convertFloorIDToRelayID(sysTaskStatus.floorNow),clk);
		}
		if(cardPeriod == 2 && sysTaskStatus.paramTable[convertFloorIDToRelayID(sysTaskStatus.floorDst)].needCard == 1)
		{
			//控制刷卡继电器拉高电平（TODO）
			printf("楼层%d刷卡失能，对应继电器%d,clk=%d\r\n",sysTaskStatus.floorDst,convertFloorIDToRelayID(sysTaskStatus.floorDst),clk);
		}
	}
	//按下开门按钮
	if((cardPeriod == 1 || cardPeriod == 5) && (clk-CLK_preGpioCtrl > 2))
	{
		//更新时间
		CLK_preGpioCtrl = clk;
		if(cardPeriod == 1)
		{
			floorButtonControl(sysTaskStatus.floorNow,CtrInput_Enable);	
			printf("按下%d层按钮，对应继电器%d,clk=%d\r\n",sysTaskStatus.floorNow,convertFloorIDToRelayID(sysTaskStatus.floorNow),clk);
		}
		if(cardPeriod == 5)
		{
			floorButtonControl(sysTaskStatus.floorDst,CtrInput_Enable);
			printf("按下%d层按钮，对应继电器%d,clk=%d\r\n",sysTaskStatus.floorDst,convertFloorIDToRelayID(sysTaskStatus.floorDst),clk);
		}
		cardPeriod++;
	}
	//抬起开门按钮
	if((cardPeriod == 2 || cardPeriod == 6) && (clk-CLK_preGpioCtrl > 22))
	{
		//更新时间
		CLK_preGpioCtrl = clk;
		if(cardPeriod == 2)
		{
			floorButtonControl(sysTaskStatus.floorNow,CtrInput_Disable);
			printf("抬起%d层按钮，对应继电器%d,clk=%d\r\n",sysTaskStatus.floorNow,convertFloorIDToRelayID(sysTaskStatus.floorNow),clk);
		}			
		if(cardPeriod == 6)
		{
			floorButtonControl(sysTaskStatus.floorDst,CtrInput_Disable);
			printf("抬起%d层按钮，对应继电器%d,clk=%d\r\n",sysTaskStatus.floorDst,convertFloorIDToRelayID(sysTaskStatus.floorDst),clk);
		}
		cardPeriod++;
	}
	//取消刷卡
	if((cardPeriod == 3 || cardPeriod == 7) && (clk-CLK_preGpioCtrl > 2)) 
	{
		//更新时间
		CLK_preGpioCtrl = clk;
		if(cardPeriod == 1 && sysTaskStatus.paramTable[convertFloorIDToRelayID(sysTaskStatus.floorNow)].needCard == 1)
		{
			//控制刷卡继电器拉低电平（TODO）
			printf("楼层%d刷卡失能,对应继电器%d,clk=%d\r\n",sysTaskStatus.floorNow,convertFloorIDToRelayID(sysTaskStatus.floorNow),clk);
		}
		if(cardPeriod == 2 && sysTaskStatus.paramTable[convertFloorIDToRelayID(sysTaskStatus.floorDst)].needCard == 1)
		{
			//控制刷卡继电器拉低电平（TODO）
			printf("楼层%d刷卡失能，对应继电器%d,clk=%d\r\n",sysTaskStatus.floorDst,convertFloorIDToRelayID(sysTaskStatus.floorDst),clk);
		}
		cardPeriod++;
	}
	//延时2s
	if((cardPeriod == 4 || cardPeriod == 8) && (clk-CLK_preGpioCtrl > 60)) 
	{
		cardPeriod = 0;
		printf("一轮按键结束,clk=%d\r\n",clk);
	}
}

//消息处理
void msgHandler(SYS_MsgHead* msg)
{
	ADDR_ToSend replyAddr;
	SYS_MsgHead replyMsg;
	
	//填充报文回复地址
	replyAddr.addrH = msg->srcAddrH;
	replyAddr.addrL = msg->srcAddrL;
	replyAddr.channel = msg->srcChannel;
	//填充报文协议首部分固定字段
	replyMsg.head1 = SYS_MSG_HEAD1;
	replyMsg.head2 = SYS_MSG_HEAD2;
	replyMsg.sequence = msg->sequence+1;
	replyMsg.srcAddrH = sysTaskStatus.localAddrH;
	replyMsg.srcAddrL = sysTaskStatus.localAddrL;
	replyMsg.srcChannel = sysTaskStatus.localChannel;
	replyMsg.timeStamp1 = 0;
	replyMsg.timeStamp2 = 0;
	
	//解析收到的协议报文
	switch(msg->msgType)
	{
	case ROBOT_TO_ELEVATOR:
	case TOPMODULE_TO_ELEVATOR:
		{
			//解析报文的payload部分
			Payload_RobotMsg* payload = (Payload_RobotMsg*)(msg+1);
			//创建回复用的消息负载段
			Payload_RobotReply payloadReply;
			//是否需要回复本帧消息
			uint8_t isNeedSend = 0;
			
			printf("收到命令：cmdID=%x,taskID=%d,floorNow=%d,floorDst=%d-----%x\r\n",
				payload->cmdID,payload->taskID,payload->floorNow,payload->floorDst,sysTaskStatus.sysStatus);
			
			switch(payload->cmdID)
			{
			case 0xA0:	//机器人询问电梯信息
				{
					//回复机器人自身状态
					payloadReply.cmdID = sysTaskStatus.sysStatus;
					isNeedSend = 1;
				}
				break;
			case 0xA1:	//机器人向电梯请求使用
				{
					//判断参数是否合法
					if(paramIsVaild(payload->floorNow,payload->floorDst))
					{
						//根据系统状态判断是否接受请求
						if(sysTaskStatus.sysStatus == 0xD0)//电梯空闲态，接受使用请求
						{
							//更新任务发起方信息
							sysTaskStatus.srcAddrH = msg->srcAddrH;
							sysTaskStatus.srcAddrL = msg->srcAddrL;
							sysTaskStatus.srcChannel = msg->srcChannel;
							//更新本次任务信息
							sysTaskStatus.taskID = payload->taskID;
							sysTaskStatus.floorDst = payload->floorDst;
							sysTaskStatus.floorNow = payload->floorNow;
							//通知状态机，机器人正在申请使用电梯
							SYSEvent_Set(Event_Robot_Call);
							//通知状态机，需要在流程的末尾向机器人上报一次自身状态
							SYSEvent_Set(Event_Need_Report_Status);
						
						}else if(sysTaskStatus.sysStatus == 0xD1 || sysTaskStatus.sysStatus == 0xD2)//D2也有可能收到A1
						{
							//电梯如果处在运行态首先需要判断magicNum
							if(sysTaskStatus.taskID == payload->taskID)
							{
								//0xD1状态
								//通知状态机，需要在流程的末尾向机器人上报一次自身状态
								SYSEvent_Set(Event_Need_Report_Status);
							}else{
								//对于magicNum不匹配的直接回复0xDF拒绝一切请求
								payloadReply.cmdID = 0xDF;
								isNeedSend = 1;
							}
						}else{
							//如果电梯处于0xD3/4 等状态
							//首先需要判断magicNum
							if(sysTaskStatus.taskID == payload->taskID)
							{
								//流程阶段异常的请求
								payloadReply.cmdID = 0xDD;
								isNeedSend = 1;
							}else{
								//对于magicNum不匹配的直接回复0xDF拒绝一切请求
								payloadReply.cmdID = 0xDF;
								isNeedSend = 1;
							}
						}
					}else{
						//参数非法，设置回复报文，回复0xDE(指令定义错误)
						payloadReply.cmdID =0xDE;
						isNeedSend = 1;
					}
				}
				break;
			case 0xA2:	//机器人正进入电梯
				{
					//首先判断magicNum是否匹配
					if(sysTaskStatus.taskID == payload->taskID)
					{
						if(sysTaskStatus.sysStatus == 0xD2)
						{
							//通知状态机，需要在流程的末尾向机器人上报一次自身状态
							SYSEvent_Set(Event_Need_Report_Status);
						}else{
							//0xD0/1/3/4状态
							//流程阶段异常的请求
							payloadReply.cmdID = 0xDD;
							isNeedSend = 1;
						}
					}else{
						//magicNum不匹配的直接回复0xDF拒绝一切请求
						payloadReply.cmdID = 0xDF;
						isNeedSend = 1;
					}
				}
				break;
			case 0xA3:	//机器人已进入电梯
				{
					//首先判断magicNum是否匹配
					if(sysTaskStatus.taskID == payload->taskID)
					{
						if(sysTaskStatus.sysStatus == 0xD2)
						{
							//通知状态机，机器人已经进入电梯
							SYSEvent_Set(Event_Robot_In);
							//通知状态机，需要在流程的末尾向机器人上报一次自身状态
							SYSEvent_Set(Event_Need_Report_Status);
						}else if(sysTaskStatus.sysStatus == 0xD3 || sysTaskStatus.sysStatus == 0xD4){
							//通知状态机，需要在流程的末尾向机器人上报一次自身状态
							SYSEvent_Set(Event_Need_Report_Status);
						}else{
							//0xD0/1状态
							//流程阶段异常的请求
							payloadReply.cmdID = 0xDD;
							isNeedSend = 1;
						}
					}else{
						//magicNum不匹配的直接回复0xDF拒绝一切请求
						payloadReply.cmdID = 0xDF;
						isNeedSend = 1;
					}
				}
				break;
			case 0xA4:	//机器人正退出电梯
				{
					//首先判断magicNum是否匹配
					if(sysTaskStatus.taskID == payload->taskID)
					{
						if(sysTaskStatus.sysStatus == 0xD4)
						{
							//通知状态机，需要在流程的末尾向机器人上报一次自身状态
							SYSEvent_Set(Event_Need_Report_Status);
						}else{
							//0xD0/1/2/3状态
							//流程阶段异常的请求
							payloadReply.cmdID = 0xDD;
							isNeedSend = 1;
						}
					}else{
						//magicNum不匹配的直接回复0xDF拒绝一切请求
						payloadReply.cmdID = 0xDF;
						isNeedSend = 1;
					}
				}
				break;
			case 0xA5:	//机器人已退出电梯
				{
					//首先判断magicNum是否匹配
					if(sysTaskStatus.taskID == payload->taskID)
					{
						if(sysTaskStatus.sysStatus == 0xD4)
						{
							//通知状态机，机器人已经离开电梯
							SYSEvent_Set(Event_Robot_Out);
							//通知状态机，需要在流程的末尾向机器人上报一次自身状态
							SYSEvent_Set(Event_Need_Report_Status);
						}else{
							//0xD0/1/2/3状态
							//流程阶段异常的请求
							payloadReply.cmdID = 0xDD;
							isNeedSend = 1;
						}
					}else{
						//magicNum不匹配的直接回复0xDF拒绝一切请求
						payloadReply.cmdID = 0xDF;
						isNeedSend = 1;
					}
				}
				break;
			case 0xAF:	//机器人取消已呼叫的本次请求
				{
					//首先判断magicNum是否匹配
					if(sysTaskStatus.taskID == payload->taskID)
					{
						//重置状态
						sysMissionReset();
						//回复0xD0空闲态，代表已被重置
						payloadReply.cmdID = 0xD0;
						isNeedSend = 1;
					}else{
						//magicNum不匹配的直接回复0xDF拒绝一切请求
						payloadReply.cmdID = 0xDF;
						isNeedSend = 1;
					}
				}
				break;
			default:
				break;
			}
			//如果有需要在状态机之后回复的报文，需要记录本次消息的序列号
			if(sysTaskStatus.event & Event_Need_Report_Status)
				sysTaskStatus.sequence = msg->sequence;
			//如果有待发送给机器人方的回复报文，启用发送
			if(isNeedSend)
			{
				//填充消息负载剩余字段
				payloadReply.Reserved = 0;
				payloadReply.taskID = payload->taskID;
				payloadReply.floorNow = getCurrentFloor();
				if(sysTaskStatus.sysStatus == 0xD0 || sysTaskStatus.sysStatus == 0xD1)
					payloadReply.floorDst = payload->floorNow;
				else
					payloadReply.floorDst = payload->floorDst;
				//填充报文剩余字段
				replyMsg.msgType = ELEVATOR_TO_ROBOT;
				replyMsg.payloadLen = sizeof(Payload_RobotReply);
				replyMsg.headCRC = crc8((uint8_t*)&replyMsg,HEAD_CRC_SIZE);
				replyMsg.payloadCRC = crc8((uint8_t*)&payloadReply,sizeof(Payload_RobotReply));
				//发送报文
				printf("回复报文：cmdID=%x,taskID=%d,floorNow=%d,floorDst=%d,status=%x\r\n",payloadReply.cmdID,payloadReply.taskID,
					payloadReply.floorNow,payloadReply.floorDst,sysTaskStatus.sysStatus);
				LORA_SendMsg(replyAddr,replyMsg,(char*)&payloadReply,sizeof(Payload_RobotReply));
			}
		}
		break;
	case CLIENT_TO_ELEVATOR:
		{
			//解析报文的payload部分
			Payload_ClienMsg* payload = (Payload_ClienMsg*)(msg+1);
			//创建回复用的消息负载段
			Payload_ClienReply payloadReply;
			
			switch(payload->cmdID)
			{
			case CLEAN_ALL_PARAM:
				{
					//清除内存参数
					memset(sysTaskStatus.paramTable, 0xff, FLOOR_MAX*sizeof(SYSFloorMsg));
					//清除flash参数
					FLASH_CleanAllParam();
					printf("清空内存参数\r\n");
					//填充回复报文
					payloadReply.cmdID = ACK_CLEAN_ALL_PARAM;
					payloadReply.result = 1;
				}
				break;
			case SET_ONE_PARAM:
				{
					int x;
					//删除所有已重复的楼层ID,防止参数冲突
					for(x=0;x<FLOOR_MAX;x++)
					{
						if(sysTaskStatus.paramTable[x].relayID != 0xff && sysTaskStatus.paramTable[x].floorID == payload->floorID)
						{
							memset(&sysTaskStatus.paramTable[x],0xff,sizeof(SYSFloorMsg));
							printf("重置重复楼层参数，x=%d,floorID=%d，floorID=%d\r\n",x,sysTaskStatus.paramTable[x].floorID,payload->floorID);
						}
					}
					
					//存入新的参数
					sysTaskStatus.paramTable[payload->relayID].relayID = payload->relayID;
					sysTaskStatus.paramTable[payload->relayID].floorID = payload->floorID;
					sysTaskStatus.paramTable[payload->relayID].needCard = payload->needCard;
					sysTaskStatus.paramTable[payload->relayID].distance = sysTaskStatus.distance;
					
					printf("增加参数：floorID:%d,relayID:%d,needcard=%d,distance=%d\r\n",
								sysTaskStatus.paramTable[payload->relayID].floorID,sysTaskStatus.paramTable[payload->relayID].relayID,
								sysTaskStatus.paramTable[payload->relayID].needCard,sysTaskStatus.paramTable[payload->relayID].distance);
					//写入flash
					FLASH_AddOneParam(sysTaskStatus.paramTable[payload->relayID]);
					//填充消息负载剩余字段
					payloadReply.result = 1;
					payloadReply.cmdID = ACK_SET_ONE_PARAM;
				}
				break;
			default:
				break;
			}
			//填充报文剩余字段
			payloadReply.reserved = 0;
			replyMsg.msgType = ELEVATOR_TO_CLIENT;
			replyMsg.payloadLen = sizeof(Payload_ClienReply);
			replyMsg.headCRC = crc8((uint8_t*)&replyMsg,HEAD_CRC_SIZE);
			replyMsg.payloadCRC = crc8((uint8_t*)&payloadReply,sizeof(Payload_ClienReply));
			//发送报文
			printf("发送res报文负载长度=%d,cmdID=%x,result=%d\r\n",sizeof(Payload_ClienReply),payloadReply.cmdID,payloadReply.result);
			LORA_SendMsg(replyAddr,replyMsg,(char*)&payloadReply,sizeof(Payload_ClienReply));
		}
		break;
	default:
		break;
	}
}
//All end



