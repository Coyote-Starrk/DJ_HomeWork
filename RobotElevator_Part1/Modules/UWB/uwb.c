#include "uwb.h"

static uint8_t DMA_RX_BUFF[UWB_DMA_RX_SIZE]; //DMA���ջ��壨UWB����ֱ�ӿ��ƼĴ�������ʹ��DMA��

//UWBģ���ʼ��
void UWB_ModuleInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	//GPIO�˿�����
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//ʹ��GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);		//ʹ��USART3ʱ��

	//USART3_TX   GPIOB.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 							//PB.10
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;				//IO���ٶ�Ϊ50MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;					//�����������
  GPIO_Init(GPIOB, &GPIO_InitStructure);									//��ʼ��GPIOB.10
   
  //USART3_RX	  GPIOB.11
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;							//PB.11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		//��������
  GPIO_Init(GPIOB, &GPIO_InitStructure);									//��ʼ��GPIOB.11

  //Usart3 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;	//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);													//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
  //USART3 ��ʼ������
	USART_InitStructure.USART_BaudRate = UWB_BAUD_RATE;															//���ڲ�����
	USART_InitStructure.USART_WordLength = UWB_WORLD_LENGTH;												//�ֳ�
	USART_InitStructure.USART_StopBits = UWB_STOP_BITS;															//ֹͣλ
	USART_InitStructure.USART_Parity = UWB_PARITY;																	//У��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;									//�շ�ģʽ

  USART_Init(USART3, &USART_InitStructure); 							//��ʼ������3
  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);					//�������ڽ����ж�,�ж�����ΪIDLE���߿����ж�
  USART_Cmd(USART3, ENABLE);                    					//ʹ�ܴ���3
	
	//ʹ��DMA��������
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);			//ʹ��DMA1����ʱ��
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR;							//DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_RX_BUFF;									//DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;												//���ݴ��䷽�򣬴����赽�ڴ�
	DMA_InitStructure.DMA_BufferSize = UWB_DMA_RX_SIZE;												//DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;					//�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;										//�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;		//���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;						//���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;															//����ģʽ�����˾Ͳ�������
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;										//���ȼ��ܸ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;															//DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��

	DMA_Init(DMA1_Channel3, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel3, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);				//ʹ��DMA���ڷ��ͺͽ�������
	
	//UWBģ���Դ����PE15
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	//ʹ��PE�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 	//��©���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 	//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);					 			//�����趨������ʼ��GPIOE.15
	
	//����UWBģ���Դ
	UWB_POWER_ON();
	
	//����1sʹUWB�ȶ�����
	delay_ms(1000);
	
	//��������UWBģ���ַ
	UWB_SetDeviceAddr();																			
	
	//����300msʹUWB�ȶ�����
	delay_ms(300);
	
	//���UWB���ջ��������¿�ʼ��������
	sysTaskStatus.uwbBufRead = sysTaskStatus.uwbBufTail;
}

//UWBģ����Ϣ�����жϷ������
void USART3_IRQHandler(void)
{
	uint8_t clear = clear;	//������������������
	uint32_t offset,recvLen;
	//���߿����жϴ�������DMA��ɰ��˺���뵽���
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		clear = USART3->SR;
		clear = USART3->DR;
		//ʧ��DMA����ֹ���ݻ���
		DMA_Cmd(DMA1_Channel3, DISABLE);
		//�ӼĴ����������ν��յ�����������С
		recvLen = UWB_DMA_RX_SIZE - DMA1_Channel3->CNDTR;
		
		//printf("\r\n�յ�����,���ݳ���Ϊ%d��HEAD:%d��TAIL:%d��READ:%d\r\n",recvLen,
		//				sysTaskStatus.uwbBufHead,sysTaskStatus.uwbBufTail,sysTaskStatus.uwbBufRead);
		
		//���lora�Ķ�ָ���bufferͷ��ָ��λ�ò�һ�£�˵����һ���Ѿ�������˻�������ĳЩ���ݣ���Ҫ��������ͷָ��Ͷ�ָ�����
		//if(sysTaskStatus.uwbBufRead != sysTaskStatus.uwbBufHead)
			sysTaskStatus.uwbBufHead = sysTaskStatus.uwbBufRead;
		
		//��DMA�����ݿ�����ѭ����������β��
		for(offset=0;offset<recvLen;offset++)
		{
			//ѭ��������βָ��������
			sysTaskStatus.uwbBufTail=(sysTaskStatus.uwbBufTail+1)%RECV_BUFFER_MAX_SIZE;
			//���ѭ����������������ô���ȶ���������ͷ����1�ֽ�����
			if(sysTaskStatus.uwbBufTail == sysTaskStatus.uwbBufHead)
				sysTaskStatus.uwbBufHead = (sysTaskStatus.uwbBufHead+1)%RECV_BUFFER_MAX_SIZE;
			//��DMA���ݽ��յ�ѭ����������β��
			sysTaskStatus.UWB_RX_BUFF[sysTaskStatus.uwbBufTail] = DMA_RX_BUFF[offset];
		}
#if 0	
		printf("���ν��յ�������Ϊ��\r\n");
		for(offset=0;offset<recvLen;offset++)
		{
			USART_SendData(USART1, DMA_RX_BUFF[offset]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
		}
		printf("\r\n");
#endif
		//����DMA CNDTR�Ĵ���
		DMA1_Channel3->CNDTR = UWB_DMA_RX_SIZE;
		//����жϱ�־
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);
		//����ʹ����һ��DMA����
		DMA_Cmd(DMA1_Channel3, ENABLE);
	}
}
//���ø���GPIO�����Ĳ��뿪�ص�ַ����UWB��TAG��ַ
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
	//����CMD��������
	for(offset=0;offset<=15;offset++)
	{
		USART_SendData(USART3, CMD[offset]);
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
		USART_SendData(USART1, CMD[offset]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	}
}
//��UWB�����ж�ȡ������Ϣ
void UWB_GetDistanceFromMsg(char* data)
{ 
	uint32_t offset,result = 0;
	//���˷�����TAG��ַ������
	if(data[60]-'0' != sysTaskStatus.localAddrL || data[62] != '0' ) 
		return;	
	//�������
	for(offset=6;offset<=13;offset++)
	{
		if(data[offset] >= '0' && data[offset] <= '9')
			result = result + data[offset] - '0';
		else if(data[offset] >= 'a' && data[offset] <= 'f')
			result = result + data[offset] - 'a' + 10;
		if(offset<13)
			result *= 16;
	}
	//�������
	sysTaskStatus.distance = result;
	//printf("\r\ndistance = %d\r\n",result);
}

