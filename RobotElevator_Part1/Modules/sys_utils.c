#include "sys_utils.h"

static u8  fac_us=0;							//us��ʱ������			   
static u16 fac_ms=0;							//ms��ʱ������

//��ʱ����ʼ��
void TIMER_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM4, ENABLE); //ʱ��ʹ��
	
	//��ʱ��
	TIM_TimeBaseStructure.TIM_Period = 499; 											//�Զ���������	��TIMER3ÿ50msһ���жϣ����㹫ʽ��(499+1)*1000 / ((36M*2)/(7199+1)) = 50ms
	TIM_TimeBaseStructure.TIM_Prescaler =7199; 										//Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 			//����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//���ϼ���
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 							//����ָ���Ĳ�����ʼ��TIM3
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); 										//ʹ��ָ����TIM3�ж�	

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  							//TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  		//��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  					//�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 							//IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  															//��ʼ��NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE);  																			//ʹ��TIM3
}
//clk���ƶ�ʱ��
void TIM3_IRQHandler(void)   //TIM3�ж�,ÿ50ms����һ���ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx�����жϱ�־ 
		++sysTaskStatus.clk;
	}
}
//��ʱ��ʼ��
void delay_Init(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//ѡ���ⲿʱ��  HCLK/8
	fac_us=SystemCoreClock/8000000;				//Ϊϵͳʱ�ӵ�1/8 
	fac_ms=(u16)fac_us*1000;					//����ÿ��ms��Ҫ��systickʱ���� 
}
//��ʱnus								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 					//ʱ�����	  		 
	SysTick->VAL=0x00;        					//��ռ�����
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//�ȴ�ʱ�䵽��   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;//�رռ�����
	SysTick->VAL =0X00;      					 			//��ռ�����	 
}
//��ʱnms,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;				//ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL =0x00;							//��ռ�����
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//�ȴ�ʱ�䵽��   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL =0X00;       					//��ռ�����	  	    
} 

//�����¼�
void SYSEvent_Set(SYSEvent event_in)
{
	sysTaskStatus.event |= event_in;
}
//����¼�
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

