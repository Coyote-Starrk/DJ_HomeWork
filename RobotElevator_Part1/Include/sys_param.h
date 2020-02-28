#ifndef __SYS_PARAM_H__
#define __SYS_PARAM_H__

#include "stm32f10x.h"

//地址宏
#define	LORA_ADDRH_ROBOT			0x00	//机器人模块高地址
#define	LORA_ADDRH_ELEVATOR 	0x4A	//电梯模块高地址	J
#define LORA_ADDRH_RELAY		 	0x54	//中继模块高地址	T
#define	LORA_ADDRH_OPERATOR		0x4D	//手持操作器模块高地址	M

//其他宏
#define SYSTEM_VERSION				0x01	//当前系统的版本号
#define ELEVATOR_DEBUG_ON			1			//是否允许调试串口打印输出

#define TIMEOUT_MAX						400		//连续200*50 = 20,000ms未收到报文，启动断连流程
#define BUTTON_TRIGGER_DELAY	80		//循环按下楼层按钮之间的间隔时间80*50 = 4000ms
#define	KEEP_DOOR_OPEN_MAX_PERIOD	600	//最长开门保持时间600*50 = 30,000ms

#define RECV_BUFFER_MAX_SIZE 	1024	//接收缓冲区最大容量
#define TMP_BUFFER_MAX_SIZE		65		//临时缓冲区最大65个字节

#define UWB_DATA_LEN  				65  	//UWB每次接收带有距离的数据报文长度是固定的65个字节

#define HEAD_CRC_SIZE					18		//headCRC需要校验的头部长度

#define SWITCH_ID_MAX					127		//继电器ID最大值
#define SWITCH_ID_MIN					0			//继电器ID最小值
#define FLOOR_MAX							128		//最大支持楼层数

#define TOLERABLE_CAL_IS_STOP_ERR 				250		//可以容忍的距离误差
#define TOLERABLE_CAL_CURRENT_FLOOR_ERR		400		//可以容忍的距离误差

#define DECODER_OPIN_NUM			16		//每个译码器的输出口数量



//通信协议的前两个字段为固定值
#define SYS_MSG_HEAD1					0x1E
#define SYS_MSG_HEAD2					0x1F

//通信结构体msgType取值范围
#define ELEVATOR_TO_ROBOT 		0xB0	//电梯向机器人发送的报文
#define	ROBOT_TO_ELEVATOR			0xB1	//机器人向电梯发送的报文
#define	ELEVATOR_TO_RELAY			0xB2	//电梯向中继模块发送的报文
#define	RELAY_TO_ELEVATOR			0xB3	//中继模块向电梯发送的报文
#define	ELEVATOR_TO_OPERATOR	0xB4	//电梯向手持操作器发送的报文
#define	OPERATOR_TO_ELEVATOR	0xB5	//手持操作器向电梯发送的报文

/*通信结构体cmdID取值范围,这些宏暂不使用，直接在代码中用对应指令
#define ROBOT_QUERY_STATUS		0xA0	//机器人查询电梯信息
#define	ROBOT_ASK_FOR_USE			0xA1	//机器人发起使用请求
#define	ROBOT_IS_ENTERING			0xA2	//机器人正在进入电梯
#define	ROBOT_ENTER_READY			0xA3	//机器人已进入电梯
#define	ROBOT_IS_EXITING			0xA4	//机器人正在离开电梯
#define	ROBOT_EXIT_READY			0xA5	//机器人已离开电梯
#define	ROBOT_ASK_END					0xA6	//机器人取消已经呼叫的请求

//状态机可选状态（0xD0-0xD4），与cmdID可选取值
#define ELEVATOR_NOT_IN_USE		0xD0	//电梯空闲
#define ELEVATOR_MVTO_FIRST		0xD1	//电梯正在向第一个任务楼层移动
#define ELEVATOR_ARVED_FIRST	0xD2	//电梯抵达了第一个任务楼层
#define ELEVATOR_MVTO_SECOND	0xD3	//电梯正在向第二个任务楼层移动
#define ELEVATOR_ARVED_SECOND	0xD4	//电梯抵达了第二个任务楼层
#define ELEVATOR_STATE_ERROR	0xDD	//下发指令和电梯当前流程阶段不匹配
#define ELEVATOR_PARAM_REJECT	0xDE	//下发参数错误，电梯回绝了这个参数
#define ELEVATOR_BE_IN_USE		0xDF	//电梯正在被其他机器人占用，或者当前处于维修状态
*/
//客户端-轿厢的额外cmdID
#define CLEAN_ALL_PARAM				0xE0	//清空嵌入式下所有参数
#define SET_ONE_PARAM					0xE1	//设置单条参数
#define ACK_CLEAN_ALL_PARAM		0xF0	//回复清空嵌入式下所有参数
#define ACK_SET_ONE_PARAM			0xF1	//回复设置单条参数

typedef enum _SYSEvent_{
	Event_0 = 0, 											//无事件
	Event_Robot_Call = 0x80,					//通知状态机，机器人正在申请使用电梯
	Event_Robot_In = 0x40,						//通知状态机，机器人已进入电梯
	Event_Robot_Out = 0x20,						//通知状态机，机器人已经出电梯
	Event_Need_Report_Status = 0x10,	//通知状态机，在流程的末尾向机器人上报一次自身状态
	Event_CirTrigger_FloorNow = 0x08,	//开始循环按下第一个任务楼层的按钮
	Event_CirTrigger_FloorDst = 0x04,	//开始循环按下第二个任务楼层的按钮
	Event_Quit_Mission	=	0x20				//正常结束任务
}SYSEvent;

//内存参数表每一条的存储结构
typedef struct _SYSFloorMsg_{
	uint32_t	distance;		//轿厢高度
	int16_t		floorID;		//楼层ID
	uint8_t		switchID;		//继电器ID(数据范围0~127)，符号位代表有没有被使用，未被使用:符号位填1，已被使用:符号位为0
	uint8_t 	needCard;		//是否需要刷卡。0：不需要   1：需要刷卡继电器1 2:需要刷卡继电器2
}SYSFloorMsg;

//任务状态结构体
typedef struct _SYSTaskStatus_{
	uint8_t				srcAddrH;					//请求方设备高地址，用于区分是否是同一机器人命令
	uint8_t				srcAddrL;					//请求方设备低地址，用于区分是否是同一机器人命令
	uint8_t				srcChannel;				//请求方设备信道，用于区分是否是同一机器人命令
	uint8_t				localAddrH;				//本地设备高地址
	uint8_t				localAddrL;				//本地设备低地址
	uint8_t				localChannel;			//本地设备信道地址
	uint8_t				sysStatus;				//系统（电梯）当前状态(可取值为0xD0/1/2/3/4/E),见宏定义
	uint8_t				reserved;
	
	int16_t				floorNow;					//任务源楼层，X楼层
	int16_t				floorDst;					//任务目标层，Y楼层
	uint16_t			taskID;						//任务发起方ID编号，用于区分是否是同一流程命令
	uint16_t      sequence;					//记录机器人方发送的序列号
	
	uint64_t			clk;							//定时器每50ms更新一次clk，系统每次启动clk从0开始自增长
	uint64_t			ledClk;						//用来记录上一次LED_POWER_ON的时间
	
	uint32_t			distance;					//实时距离
	uint32_t			distanceFIFO[5];	//用来判断是否稳定处在同一楼层
	SYSEvent			event;						//待处理事件
	SYSFloorMsg		paramTable[FLOOR_MAX]; //按照继电器ID顺序存放所有的楼层信息，占用1k内存
	
	uint16_t			loraBufHead;
	uint16_t			loraBufTail;
	uint16_t			loraBufRead;
	
	uint16_t			uwbBufHead;
	uint16_t			uwbBufTail;
	uint16_t			uwbBufRead;
	
	uint8_t				LORA_RX_BUFF[RECV_BUFFER_MAX_SIZE]; //LORA接收缓冲
	uint8_t				UWB_RX_BUFF[RECV_BUFFER_MAX_SIZE]; 	//UWB接收缓冲
	uint8_t				TMP_BUFFER[TMP_BUFFER_MAX_SIZE];		//临时数据处理区，由两个接收缓冲区内的待处理数据拷贝而来
}SYSTaskStatus;

//通信地址
typedef struct _ADDR_TO_SEND_{
	uint8_t				addrH;
	uint8_t				addrL;
	uint8_t				channel;
}ADDR_ToSend;

//通信结构体协议首部
typedef struct _SYS_MSG_HEAD_{
	uint8_t 			head1;
	uint8_t 			head2;
	uint16_t			sequence;
	uint8_t 			srcAddrH;
	uint8_t 			srcAddrL;
	uint8_t 			srcChannel;
	uint8_t 			msgType;
	uint32_t			timeStamp1;
	uint32_t			timeStamp2;
	uint16_t			payloadLen;
	uint8_t				headCRC;
	uint8_t				payloadCRC;
}SYS_MsgHead;

//来自机器人的消息负载
typedef struct _PAYLOAD_ROBOT_MSG_{
	uint8_t				cmdID;
	uint8_t				Reserved;
	uint16_t			taskID;
	int16_t				floorNow;
	int16_t				floorDst;
}Payload_RobotMsg;

//用于回复机器人的消息负载
typedef struct _PAYLOAD_ROBOT_REPLY_{
	uint8_t				cmdID;
	uint8_t				Reserved;
	uint16_t			taskID;
	int16_t				floorNow;
	int16_t				floorDst;
}Payload_RobotReply;

//来自手持操作器的消息负载
typedef struct _PAYLOAD_CLIENT_MSG_{
	uint8_t				cmdID;
	int8_t				floorID;
	uint8_t				switchID;
	uint8_t				needCard;
}Payload_ClienMsg;

//用于回复手持操作器的消息负载
typedef struct _PAYLOAD_CLIENT_REPLY_{
	uint8_t				cmdID;
	uint8_t				reserved;
	uint16_t			result;
}Payload_ClienReply;

extern SYSTaskStatus sysTaskStatus;

#endif /*__SYS_PARAM_H__*/




