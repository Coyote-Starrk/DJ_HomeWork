#include "lora.h"

static uint8_t DMA_RX_BUFF[LORA_DMA_RX_SIZE];							//DMA���ջ���
static uint8_t DMA_TX_BUFF[LORA_DMA_TX_SIZE];							//DMA���ͻ���

//Loraģ���ʼ��
void Lora_ModuleInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	//PA2,PA3���ڳ�ʼ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);		//ʹ��USART2ʱ��

	//USART2_TX   GPIOA.2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 							//PA.2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;				//IO���ٶ�Ϊ50MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;					//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);									//��ʼ��GPIOA.2
		
  //USART2_RX	  GPIOA.3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;								//PA3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);									//��ʼ��GPIOA.3

  //Usart2 NVIC�ж�����
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;	//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);													//��ʼ��NVIC�Ĵ���
  
  //USART ��ʼ�����ã��ȳ�ʼ��Ϊ9600�����ʣ��ο�E22ʹ���ֲᣩ�����ú�lora���������޸�ΪЭ�����貨���ʣ�
	USART_InitStructure.USART_BaudRate = 9600;																				//���ڲ�����
	USART_InitStructure.USART_WordLength = LORA_WORLD_LENGTH;													//�ֳ�
	USART_InitStructure.USART_StopBits = LORA_STOP_BITS;															//ֹͣλ
	USART_InitStructure.USART_Parity = LORA_PARITY;																		//��żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;										//�շ�ģʽ
	
  USART_Init(USART2, &USART_InitStructure); 							//��ʼ������2
  USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);					//�������ڽ����жϣ��ж�����ΪIDLE���߿����ж�
  USART_Cmd(USART2, ENABLE);                    					//ʹ�ܴ���2
	
	//ʹ��DMA��������
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);			//ʹ��DMA1����ʱ��
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;							//DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_RX_BUFF;									//DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;												//���ݴ��䷽�򣬴����赽�ڴ�
	DMA_InitStructure.DMA_BufferSize = LORA_DMA_RX_SIZE;											//DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;					//�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;										//�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;		//���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;						//���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;															//����ģʽ�����˾Ͳ�������
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;										//���ȼ��ܸ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;															//DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��

	DMA_Init(DMA1_Channel6, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel6, ENABLE);
		
	//ʹ��DMA��������
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;							//DMA�������ַ
  DMA_InitStructure.DMA_MemoryBaseAddr =(u32)DMA_TX_BUFF;										//�����ڴ��ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;												//�ڴ�������ͨ��
  DMA_InitStructure.DMA_BufferSize = 0;																			//���ͳ���Ϊ0
  DMA_Init(DMA1_Channel7, &DMA_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_TC, ENABLE);															//ʹ�ܴ��ڷ�������ж�
	USART_DMACmd(USART2, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);						//ʹ��DMA���ڷ��ͺͽ�������
	
	//M0M1ģ�����ÿڳ�ʼ����PB1,PE7��		
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);//ʹ��PB,PE�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 		//��©���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 				//�����趨������ʼ��GPIOB.1

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 			//�����趨������ʼ��GPIOE.7
	
	//Loraģ���Դ����PE9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 		//��©���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);					 				//�����趨������ʼ��GPIOE.9
	
	//AUX�������ų�ʼ��PE8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	//��������
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//�Ƚ�LORAתΪ��������ģʽ
	LORA_SetM0Low();													 							//M0�����
	LORA_SetM1High();														 						//M1�����
	LORA_POWER_ON();													 							//����Loraģ���Դ

	delay_ms(1000);																					//��ʱ1s�ȴ���������
	
	LORA_SetDeviceAddr();																		//����ģ���ַ�����㴫��ģʽ���ŵ���ͨ�Ų����ʵȣ�
	
	delay_ms(300);																					//�ȴ��������
	
	LORA_SetM0Low();																				//M0�����
	LORA_SetM1Low(); 																				//M1�����
	
	USART_Cmd(USART2, DISABLE);
	USART_InitStructure.USART_BaudRate = LORA_BAUD_RATE;
	USART_Init(USART2, &USART_InitStructure); 							//�޸Ĵ���2�Ĳ�����
	USART_Cmd(USART2, ENABLE); 
	
	printf("��ʼ�����\r\n");
}

//loraģ�����õ�ַ���ŵ�/���㴫��Ȳ���
void LORA_SetDeviceAddr(void)
{
	//�������ã�c0 00 07 ADDRH ADDRL CHANNEL e7 80 17 43���ο�E22ʹ���ֲ�12ҳ��7.2�£��Ĵ�������
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
	
	//printf("�������ò���\r\n");
	while (DMA_GetCurrDataCounter(DMA1_Channel7));
	printf("���ظߵ�ַ��%x���͵�ַ��%x���ŵ���%x.\r\n",sysTaskStatus.localAddrH,
			sysTaskStatus.localAddrL,sysTaskStatus.localChannel);

	//���÷��ͳ��ȣ���������
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA1_Channel7->CNDTR = 10;
  DMA_Cmd(DMA1_Channel7, ENABLE); 
}

//Loraģ����Ϣ�����жϷ������
void USART2_IRQHandler(void) 
{
	uint8_t clear = clear;	//������������������
	uint32_t recvLen,offset;
	//���߿����жϴ�������DMA��ɰ��˺���뵽���
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		clear = USART2->SR;
		clear = USART2->DR;
		//ʧ��DMA����ֹ���ݻ���
		DMA_Cmd(DMA1_Channel6, DISABLE);
		
		//����LED
		LED_POWER_ON();
		//��¼LED����ʱ��
		sysTaskStatus.ledClk = sysTaskStatus.clk;
		
		//�ӼĴ����������ν��յ�����������С
		recvLen = LORA_DMA_RX_SIZE - DMA1_Channel6->CNDTR;
		
		printf("\r\n�յ�����,���ݳ���Ϊ%d��HEAD:%d��TAIL:%d��READ:%d\r\n",recvLen,
							sysTaskStatus.loraBufHead,sysTaskStatus.loraBufTail,sysTaskStatus.loraBufRead);
		
		//���lora�Ķ�ָ���bufferͷ��ָ��λ�ò�һ�£�˵����һ���Ѿ�������˻�������ĳЩ���ݣ���Ҫ��������ͷָ��Ͷ�ָ�����
		//if(sysTaskStatus.loraBufRead != sysTaskStatus.loraBufHead)
			sysTaskStatus.loraBufHead = sysTaskStatus.loraBufRead;
		//��DMA�����ݿ�����ѭ����������β��
		for(offset=0;offset<recvLen;offset++)
		{
			//ѭ��������βָ��������
			sysTaskStatus.loraBufTail=(sysTaskStatus.loraBufTail+1)%RECV_BUFFER_MAX_SIZE;
			//���ѭ����������������ô���ȶ���������ͷ����1�ֽ�����
			if(sysTaskStatus.loraBufTail == sysTaskStatus.loraBufHead)
				sysTaskStatus.loraBufHead = (sysTaskStatus.loraBufHead+1)%RECV_BUFFER_MAX_SIZE;
			//��DMA���ݽ��յ�ѭ����������β��
			sysTaskStatus.LORA_RX_BUFF[sysTaskStatus.loraBufTail] = DMA_RX_BUFF[offset];
		}
#if 0	
		//��ӡ���ν��յ�������
		//printf("abc/r/n");
		for(offset=0;offset<recvLen;offset++)
		{
			USART_SendData(USART1, DMA_RX_BUFF[offset]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
		}
		//printf("\r\n");

		//buffer��ǰ����Ϊ
		printf("buffer��ǰ����Ϊ��\r\n");
		for(offset=1;(sysTaskStatus.loraBufHead+offset)%RECV_BUFFER_MAX_SIZE!=sysTaskStatus.loraBufTail;offset++)
		{
			USART_SendData(USART1, sysTaskStatus.LORA_RX_BUFF[(sysTaskStatus.loraBufHead+offset)%RECV_BUFFER_MAX_SIZE]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
		}
		printf("\r\n");
		
		printf("�޸�֮���ָ��Ϊ��HEAD:%d��TAIL:%d��READ:%d\r\n",
						sysTaskStatus.loraBufHead,sysTaskStatus.loraBufTail,sysTaskStatus.loraBufRead);
#endif		
		//����DMA CNDTR�Ĵ���
		DMA1_Channel6->CNDTR = LORA_DMA_RX_SIZE;
		//����жϱ�־
		USART_ClearITPendingBit(USART2, USART_IT_IDLE);
		//����ʹ����һ��DMA����
		DMA_Cmd(DMA1_Channel6, ENABLE);
	}
	//TX��������ж�
	if(USART_GetITStatus(USART2,USART_IT_TC)!= RESET)
	{
		//�ر�DMA
		DMA_Cmd(DMA1_Channel7, DISABLE);
		//������ݳ���
		DMA1_Channel7->CNDTR=0; 
		//����жϱ�־
		USART_ClearITPendingBit(USART2, USART_IT_TC);
	}
}
//���һ֡���ģ�������LORA����
void LORA_SendMsg(ADDR_ToSend addr, SYS_MsgHead resMsg, char* payload,size_t payloadLen)
{
	uint8_t isWait = 0;
	//����ϴε�DMA��û����ȫ������ɣ���Ҫ�ȴ�����֮���ٽ�����һ��
	while (DMA_GetCurrDataCounter(DMA1_Channel7))	isWait=1;
	//�����������ͱ��ģ������õ���Lora���㷢��
	if(isWait) delay_ms(10);
	
	//��AUX����Ϊ�ߵ�ƽ��ʱ������������
	//while(LORA_AUX == 0);

	//Ҫ��DMA�������ĳ���Ҫ���ڱ���Ҫ���͵ı��ĳ���
	memcpy(DMA_TX_BUFF, &addr, sizeof(ADDR_ToSend));
	memcpy(DMA_TX_BUFF+sizeof(ADDR_ToSend), &resMsg, sizeof(SYS_MsgHead));
	memcpy(DMA_TX_BUFF+sizeof(ADDR_ToSend)+sizeof(SYS_MsgHead), payload, payloadLen);
	//DMA_TX_BUFF[sizeof(ADDR_ToSend)+sizeof(SYS_MsgHead)+payloadLen] = '\r';
	//DMA_TX_BUFF[sizeof(ADDR_ToSend)+sizeof(SYS_MsgHead)+payloadLen+1] = '\n';
	
	//���÷��ͳ��ȣ���������
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA1_Channel7->CNDTR = sizeof(ADDR_ToSend)+sizeof(SYS_MsgHead)+payloadLen;
  DMA_Cmd(DMA1_Channel7, ENABLE); 
}

