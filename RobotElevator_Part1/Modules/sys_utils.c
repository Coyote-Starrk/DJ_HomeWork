#include "sys_utils.h"

static u8  fac_us=0;							//us延时倍乘数			   
static u16 fac_ms=0;							//ms延时倍乘数

//加入以下代码,支持printf函数
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{    
#if ELEVATOR_DEBUG_ON 
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
#endif
}
#endif 

//辅助工具初始化（初始化USART1的串口调试输出/初始化clk定时器）
void Utils_ModuleInit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM4, ENABLE); //时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
	
	//定时器
	TIM_TimeBaseStructure.TIM_Period = 499; 											//自动触发周期	（TIMER3每50ms一次中断）计算公式：(499+1)*1000 / ((36M*2)/(7199+1)) = 50ms
	TIM_TimeBaseStructure.TIM_Prescaler =7199; 										//预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 			//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 							//根据指定的参数初始化TIM3
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); 										//使能指定的TIM3中断	

	//定时器中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  							//TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  		//先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  					//从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 							//IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  															//初始化NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  																			//使能TIM3
	
	//打印调试串口初始化
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
	
	USART_InitStructure.USART_BaudRate = DEBUG_BOUND;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Tx;	//只占用输出口

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_Cmd(USART1, ENABLE);                    //使能串口1 
	
	//用于延时控制
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8
	fac_us=SystemCoreClock/8000000;				//为系统时钟的1/8 
	fac_ms=(u16)fac_us*1000;					//代表每个ms需要的systick时钟数
}

//clk控制中断函数
void TIM3_IRQHandler(void)   //TIM3中断,每50ms触发一次中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx更新中断标志 
		++sysTaskStatus.clk;
	}
}

//延时nus								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 					//时间加载	  		 
	SysTick->VAL=0x00;        					//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;//关闭计数器
	SysTick->VAL =0X00;      					 			//清空计数器	 
}
//延时nms,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;							//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;       					//清空计数器	  	    
} 

//设置事件
void SYSEvent_Set(SYSEvent event_in)
{
	sysTaskStatus.event |= event_in;
}
//清除事件
void SYSEvent_Clr(SYSEvent event_in)
{
	sysTaskStatus.event &=~ event_in;
}
//CRC-8
uint8_t crc8(const uint8_t *buff, size_t n) {
  uint8_t crc = 0x00;
	size_t i;
  for (i = 0; i < n; i++) {
    crc += buff[i];
  }
  return crc&0xff;
}
//通过floorID获得relayID
uint8_t convertFloorIDToSwitchID(int16_t floorID)
{
	uint8_t position;
	for(position=SWITCH_ID_MIN;position<=SWITCH_ID_MAX;position++)
	{
		if(floorID == sysTaskStatus.paramTable[position].floorID)
			return sysTaskStatus.paramTable[position].switchID;
	}
	//执行到最后说明没找到，返回0xFF
	return 0xFF;
}
