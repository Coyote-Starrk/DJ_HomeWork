#include "uwb.h"

static uint8_t DMA_RX_BUFF[UWB_DMA_RX_SIZE]; //DMA接收缓冲（UWB发送直接控制寄存器，不使用DMA）

//UWB模块初始化
void UWB_ModuleInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	//GPIO端口设置
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//使能GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);		//使能USART3时钟

	//USART3_TX   GPIOB.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 							//PB.10
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;				//IO口速度为50MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;					//复用推挽输出
  GPIO_Init(GPIOB, &GPIO_InitStructure);									//初始化GPIOB.10
   
  //USART3_RX	  GPIOB.11
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;							//PB.11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		//浮空输入
  GPIO_Init(GPIOB, &GPIO_InitStructure);									//初始化GPIOB.11

  //Usart3 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);													//根据指定的参数初始化VIC寄存器
  
  //USART3 初始化设置
	USART_InitStructure.USART_BaudRate = UWB_BAUD_RATE;															//串口波特率
	USART_InitStructure.USART_WordLength = UWB_WORLD_LENGTH;												//字长
	USART_InitStructure.USART_StopBits = UWB_STOP_BITS;															//停止位
	USART_InitStructure.USART_Parity = UWB_PARITY;																	//校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;									//收发模式

  USART_Init(USART3, &USART_InitStructure); 							//初始化串口3
  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);					//开启串口接受中断,中断类型为IDLE总线空闲中断
  USART_Cmd(USART3, ENABLE);                    					//使能串口3
	
	//使用DMA接收数据
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);			//使能DMA1总线时钟
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR;							//DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_RX_BUFF;									//DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;												//数据传输方向，从外设到内存
	DMA_InitStructure.DMA_BufferSize = UWB_DMA_RX_SIZE;												//DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;					//外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;										//内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;		//数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;						//数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;															//正常模式，满了就不再收了
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;										//优先级很高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;															//DMA通道x没有设置为内存到内存传输

	DMA_Init(DMA1_Channel3, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel3, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);				//使能DMA串口发送和接收请求
	
	//UWB模块电源控制PE15
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	//使能PE端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 	//开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 	//IO口速度为50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);					 			//根据设定参数初始化GPIOE.15
	
	//开启UWB模块电源
	UWB_POWER_ON();
	
	//休眠1s使UWB稳定工作
	delay_ms(1000);
	
	//重新设置UWB模块地址
	UWB_SetDeviceAddr();																			
	
	//休眠300ms使UWB稳定工作
	delay_ms(300);
	
	//清空UWB接收缓冲区重新开始接收数据
	sysTaskStatus.uwbBufRead = sysTaskStatus.uwbBufTail;
}

//UWB模块消息接收中断服务程序
void USART3_IRQHandler(void)
{
	uint8_t clear = clear;	//用来消除编译器警告
	uint32_t offset,recvLen;
	//总线空闲中断处理函数（DMA完成搬运后进入到这里）
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		clear = USART3->SR;
		clear = USART3->DR;
		//失能DMA，防止数据混乱
		DMA_Cmd(DMA1_Channel3, DISABLE);
		//从寄存区读出本次接收到的数据量大小
		recvLen = UWB_DMA_RX_SIZE - DMA1_Channel3->CNDTR;
		
		//printf("\r\n收到数据,数据长度为%d：HEAD:%d，TAIL:%d，READ:%d\r\n",recvLen,
		//				sysTaskStatus.uwbBufHead,sysTaskStatus.uwbBufTail,sysTaskStatus.uwbBufRead);
		
		//如果lora的读指针和buffer头部指针位置不一致，说明上一次已经处理过了缓冲区的某些数据，需要将缓冲区头指针和读指针对齐
		//if(sysTaskStatus.uwbBufRead != sysTaskStatus.uwbBufHead)
			sysTaskStatus.uwbBufHead = sysTaskStatus.uwbBufRead;
		
		//将DMA的数据拷贝到循环缓冲区的尾部
		for(offset=0;offset<recvLen;offset++)
		{
			//循环缓冲区尾指针先自增
			sysTaskStatus.uwbBufTail=(sysTaskStatus.uwbBufTail+1)%RECV_BUFFER_MAX_SIZE;
			//如果循环缓冲区已满，那么优先丢弃缓冲区头部的1字节数据
			if(sysTaskStatus.uwbBufTail == sysTaskStatus.uwbBufHead)
				sysTaskStatus.uwbBufHead = (sysTaskStatus.uwbBufHead+1)%RECV_BUFFER_MAX_SIZE;
			//将DMA数据接收到循环缓冲区的尾部
			sysTaskStatus.UWB_RX_BUFF[sysTaskStatus.uwbBufTail] = DMA_RX_BUFF[offset];
		}
#if 0	
		printf("本次接收到的数据为：\r\n");
		for(offset=0;offset<recvLen;offset++)
		{
			USART_SendData(USART1, DMA_RX_BUFF[offset]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
		}
		printf("\r\n");
#endif
		//重置DMA CNDTR寄存器
		DMA1_Channel3->CNDTR = UWB_DMA_RX_SIZE;
		//清除中断标志
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);
		//重新使能下一次DMA接收
		DMA_Cmd(DMA1_Channel3, ENABLE);
	}
}
//设置根据GPIO读出的拨码开关地址设置UWB的TAG地址
void UWB_SetDeviceAddr()
{
	uint8_t CMD[] = "AT+SW=10000000\r\n";
	uint8_t addr = sysTaskStatus.localAddrL;
	uint8_t offset;
	//addr1
	if(addr & 0x04)	CMD[10]='1';
	else CMD[10]='0';
	//addr2
	if(addr & 0x02) CMD[11]='1';
	else CMD[11]='0';
	//addr3
	if(addr & 0x01) CMD[12]='1';
	else CMD[12]='0';
	//发送CMD后置命令
	for(offset=0;offset<=15;offset++)
	{
		USART_SendData(USART3, CMD[offset]);
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
		USART_SendData(USART1, CMD[offset]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	}
}
//从UWB数据中读取距离信息
void UWB_GetDistanceFromMsg(char* data)
{ 
	uint32_t offset,result = 0;
	//过滤非自身TAG地址的数据
	if(data[60]-'0' != sysTaskStatus.localAddrL || data[62] != '0' ) 
		return;	
	//计算距离
	for(offset=6;offset<=13;offset++)
	{
		if(data[offset] >= '0' && data[offset] <= '9')
			result = result + data[offset] - '0';
		else if(data[offset] >= 'a' && data[offset] <= 'f')
			result = result + data[offset] - 'a' + 10;
		if(offset<13)
			result *= 16;
	}
	//输出数据
	sysTaskStatus.distance = result;
	//printf("\r\ndistance = %d\r\n",result);
}

