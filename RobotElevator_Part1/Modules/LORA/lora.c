#include "lora.h"

static uint8_t DMA_RX_BUFF[LORA_DMA_RX_SIZE];							//DMA接收缓冲
static uint8_t DMA_TX_BUFF[LORA_DMA_TX_SIZE];							//DMA发送缓冲

//Lora模块初始化
void Lora_ModuleInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	//PA2,PA3串口初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);		//使能USART2时钟

	//USART2_TX   GPIOA.2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 							//PA.2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;				//IO口速度为50MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;					//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);									//初始化GPIOA.2
		
  //USART2_RX	  GPIOA.3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;								//PA3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);									//初始化GPIOA.3

  //Usart2 NVIC中断配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);													//初始化NVIC寄存器
  
  //USART 初始化设置（先初始化为9600波特率（参考E22使用手册），设置好lora参数后，再修改为协议所需波特率）
	USART_InitStructure.USART_BaudRate = 9600;																				//串口波特率
	USART_InitStructure.USART_WordLength = LORA_WORLD_LENGTH;													//字长
	USART_InitStructure.USART_StopBits = LORA_STOP_BITS;															//停止位
	USART_InitStructure.USART_Parity = LORA_PARITY;																		//奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;										//收发模式
	
  USART_Init(USART2, &USART_InitStructure); 							//初始化串口2
  USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);					//开启串口接受中断，中断类型为IDLE总线空闲中断
  USART_Cmd(USART2, ENABLE);                    					//使能串口2
	
	//使用DMA接收数据
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);			//使能DMA1总线时钟
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;							//DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_RX_BUFF;									//DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;												//数据传输方向，从外设到内存
	DMA_InitStructure.DMA_BufferSize = LORA_DMA_RX_SIZE;											//DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;					//外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;										//内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;		//数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;						//数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;															//正常模式，满了就不再收了
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;										//优先级很高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;															//DMA通道x没有设置为内存到内存传输

	DMA_Init(DMA1_Channel6, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel6, ENABLE);
		
	//使用DMA发送数据
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;							//DMA外设基地址
  DMA_InitStructure.DMA_MemoryBaseAddr =(u32)DMA_TX_BUFF;										//发送内存地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;												//内存与外设通信
  DMA_InitStructure.DMA_BufferSize = 0;																			//发送长度为0
  DMA_Init(DMA1_Channel7, &DMA_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_TC, ENABLE);															//使能串口发送完成中断
	USART_DMACmd(USART2, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);						//使能DMA串口发送和接收请求
	
	//M0M1模块配置口初始化（PB1,PE7）		
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);//使能PB,PE端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 		//开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 		//IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 				//根据设定参数初始化GPIOB.1

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 			//根据设定参数初始化GPIOE.7
	
	//Lora模块电源控制PE9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 		//开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 		//IO口速度为50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);					 				//根据设定参数初始化GPIOE.9
	
	//AUX监视引脚初始化PE8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	//浮空输入
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//先将LORA转为参数配置模式
	LORA_SetM0Low();													 							//M0输出低
	LORA_SetM1High();														 						//M1输出高
	LORA_POWER_ON();													 							//开启Lora模块电源

	delay_ms(1000);																					//延时1s等待正常工作
	
	LORA_SetDeviceAddr();																		//设置模块地址（定点传输模式、信道、通信波特率等）
	
	delay_ms(300);																					//等待设置完成
	
	LORA_SetM0Low();																				//M0输出低
	LORA_SetM1Low(); 																				//M1输出低
	
	USART_Cmd(USART2, DISABLE);
	USART_InitStructure.USART_BaudRate = LORA_BAUD_RATE;
	USART_Init(USART2, &USART_InitStructure); 							//修改串口2的波特率
	USART_Cmd(USART2, ENABLE); 
	
	printf("初始化完成\r\n");
}

//lora模块设置地址、信道/定点传输等参数
void LORA_SetDeviceAddr(void)
{
	//命令设置：c0 00 07 ADDRH ADDRL CHANNEL e7 80 17 43，参考E22使用手册12页，7.2章，寄存器描述
	DMA_TX_BUFF[0] = 0xc0;
	DMA_TX_BUFF[1] = 0x00;
	DMA_TX_BUFF[2] = 0x07;
	DMA_TX_BUFF[3] = sysTaskStatus.localAddrH;
	DMA_TX_BUFF[4] = sysTaskStatus.localAddrL;
	DMA_TX_BUFF[5] = 0x00;
	DMA_TX_BUFF[6] = 0xe7;
	DMA_TX_BUFF[7] = 0x80;
	DMA_TX_BUFF[8] = sysTaskStatus.localChannel;
	DMA_TX_BUFF[9] = 0x43;
	
	//printf("重新设置参数\r\n");
	while (DMA_GetCurrDataCounter(DMA1_Channel7));
	printf("本地高地址：%x，低地址：%x，信道：%x.\r\n",sysTaskStatus.localAddrH,
			sysTaskStatus.localAddrL,sysTaskStatus.localChannel);

	//设置发送长度，启动发送
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA1_Channel7->CNDTR = 10;
  DMA_Cmd(DMA1_Channel7, ENABLE); 
}

//Lora模块消息接收中断服务程序
void USART2_IRQHandler(void) 
{
	uint8_t clear = clear;	//用来消除编译器警告
	uint32_t recvLen,offset;
	//总线空闲中断处理函数（DMA完成搬运后进入到这里）
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		clear = USART2->SR;
		clear = USART2->DR;
		//失能DMA，防止数据混乱
		DMA_Cmd(DMA1_Channel6, DISABLE);
		
		//开启LED
		LED_POWER_ON();
		//记录LED开启时间
		sysTaskStatus.ledClk = sysTaskStatus.clk;
		
		//从寄存区读出本次接收到的数据量大小
		recvLen = LORA_DMA_RX_SIZE - DMA1_Channel6->CNDTR;
		
		printf("\r\n收到数据,数据长度为%d：HEAD:%d，TAIL:%d，READ:%d\r\n",recvLen,
							sysTaskStatus.loraBufHead,sysTaskStatus.loraBufTail,sysTaskStatus.loraBufRead);
		
		//如果lora的读指针和buffer头部指针位置不一致，说明上一次已经处理过了缓冲区的某些数据，需要将缓冲区头指针和读指针对齐
		//if(sysTaskStatus.loraBufRead != sysTaskStatus.loraBufHead)
			sysTaskStatus.loraBufHead = sysTaskStatus.loraBufRead;
		//将DMA的数据拷贝到循环缓冲区的尾部
		for(offset=0;offset<recvLen;offset++)
		{
			//循环缓冲区尾指针先自增
			sysTaskStatus.loraBufTail=(sysTaskStatus.loraBufTail+1)%RECV_BUFFER_MAX_SIZE;
			//如果循环缓冲区已满，那么优先丢弃缓冲区头部的1字节数据
			if(sysTaskStatus.loraBufTail == sysTaskStatus.loraBufHead)
				sysTaskStatus.loraBufHead = (sysTaskStatus.loraBufHead+1)%RECV_BUFFER_MAX_SIZE;
			//将DMA数据接收到循环缓冲区的尾部
			sysTaskStatus.LORA_RX_BUFF[sysTaskStatus.loraBufTail] = DMA_RX_BUFF[offset];
		}
#if 0	
		//打印本次接收到的数据
		//printf("abc/r/n");
		for(offset=0;offset<recvLen;offset++)
		{
			USART_SendData(USART1, DMA_RX_BUFF[offset]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
		}
		//printf("\r\n");

		//buffer当前内容为
		printf("buffer当前内容为：\r\n");
		for(offset=1;(sysTaskStatus.loraBufHead+offset)%RECV_BUFFER_MAX_SIZE!=sysTaskStatus.loraBufTail;offset++)
		{
			USART_SendData(USART1, sysTaskStatus.LORA_RX_BUFF[(sysTaskStatus.loraBufHead+offset)%RECV_BUFFER_MAX_SIZE]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
		}
		printf("\r\n");
		
		printf("修改之后的指针为：HEAD:%d，TAIL:%d，READ:%d\r\n",
						sysTaskStatus.loraBufHead,sysTaskStatus.loraBufTail,sysTaskStatus.loraBufRead);
#endif		
		//重置DMA CNDTR寄存器
		DMA1_Channel6->CNDTR = LORA_DMA_RX_SIZE;
		//清除中断标志
		USART_ClearITPendingBit(USART2, USART_IT_IDLE);
		//重新使能下一次DMA接收
		DMA_Cmd(DMA1_Channel6, ENABLE);
	}
	//TX发送完成中断
	if(USART_GetITStatus(USART2,USART_IT_TC)!= RESET)
	{
		//关闭DMA
		DMA_Cmd(DMA1_Channel7, DISABLE);
		//清除数据长度
		DMA1_Channel7->CNDTR=0; 
		//清除中断标志
		USART_ClearITPendingBit(USART2, USART_IT_TC);
	}
}
//填充一帧报文，并调用LORA发送
void LORA_SendMsg(ADDR_ToSend addr, SYS_MsgHead resMsg, char* payload,size_t payloadLen)
{
	uint8_t isWait = 0;
	//如果上次的DMA还没有完全发送完成，需要等待发完之后再进行下一步
	while (DMA_GetCurrDataCounter(DMA1_Channel7))	isWait=1;
	//不可连续发送报文，我们用的是Lora定点发送
	if(isWait) delay_ms(10);
	
	//当AUX引脚为高电平的时候允许发送数据
	//while(LORA_AUX == 0);

	//要求DMA缓冲区的长度要大于本次要发送的报文长度
	memcpy(DMA_TX_BUFF, &addr, sizeof(ADDR_ToSend));
	memcpy(DMA_TX_BUFF+sizeof(ADDR_ToSend), &resMsg, sizeof(SYS_MsgHead));
	memcpy(DMA_TX_BUFF+sizeof(ADDR_ToSend)+sizeof(SYS_MsgHead), payload, payloadLen);
	//DMA_TX_BUFF[sizeof(ADDR_ToSend)+sizeof(SYS_MsgHead)+payloadLen] = '\r';
	//DMA_TX_BUFF[sizeof(ADDR_ToSend)+sizeof(SYS_MsgHead)+payloadLen+1] = '\n';
	
	//设置发送长度，启动发送
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA1_Channel7->CNDTR = sizeof(ADDR_ToSend)+sizeof(SYS_MsgHead)+payloadLen;
  DMA_Cmd(DMA1_Channel7, ENABLE); 
}

