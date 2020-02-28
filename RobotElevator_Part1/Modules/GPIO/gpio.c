#include "gpio.h"

//GPIO��ʼ�����������ϵ��ݿ��ư�ť��LED���ݵƣ����뿪��
void GPIO_Usr_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE,ENABLE);//ʹ��PORTA,PORTB,PORTC,PORTD,PORTEʱ��

	//��©���GPIO����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//��©���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 	//�ٶ�Ϊ50MHz
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_8|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	//�������GPIO����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 	//�ٶ�Ϊ50MHz
	
	//���PA15.PB3.PB4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11|GPIO_Pin_12;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_3|GPIO_Pin_4;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5|GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	//���ݵ�LED��ʼ����PA15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 	//�ٶ�Ϊ50MHz
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_15;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);							

	//���뿪�س�ʼ��,ID��ַ����PA4.5.6,�ŵ���ַ����PA7,PC4.5
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ����������GPIOA4.5.6.7
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ����������GPIOC4.5`	
	
	//���ݲ��뿪�ض��뱾�صĵ�ַ���ŵ�
	Update_Current_ADDRL();
	
	//���е�������ʹ�ܹܽ�һ��ʼ��Ҫ���ڸߵ�ƽλ��
	GPIO_SetBits(GPIOD,GPIO_Pin_8);
	GPIO_SetBits(GPIOD,GPIO_Pin_15);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	GPIO_SetBits(GPIOD,GPIO_Pin_4);
	GPIO_SetBits(GPIOB,GPIO_Pin_9);
	GPIO_SetBits(GPIOE,GPIO_Pin_4);
	GPIO_SetBits(GPIOA,GPIO_Pin_0);
	GPIO_SetBits(GPIOE,GPIO_Pin_14);
	
	//����ָʾ��
	LED_ON;
}

//�޸Ķ�Ӧĳһ¥���������ʹ����������
void UpdateEncoderEnInput(uint8_t relayID, CtrInput_TypeDef ctrInput)
{
	if(relayID > RELAY_ID_MAX)
		return;
	switch(relayID / DECODER_OPIN_NUM)
	{
	case 0:
		{
			if(ctrInput)
				GPIO_SetBits(GPIOD,GPIO_Pin_8);
			else
				GPIO_ResetBits(GPIOD,GPIO_Pin_8);
		}
		break;
	case 1:
		{
			if(ctrInput)
				GPIO_SetBits(GPIOD,GPIO_Pin_15);
			else
				GPIO_ResetBits(GPIOD,GPIO_Pin_15);
		}
		break;
	case 2:
		{
			if(ctrInput)
				GPIO_SetBits(GPIOA,GPIO_Pin_8);
			else
				GPIO_ResetBits(GPIOA,GPIO_Pin_8);
		}
		break;
	case 3:
		{
			if(ctrInput)
				GPIO_SetBits(GPIOD,GPIO_Pin_4);
			else
				GPIO_ResetBits(GPIOD,GPIO_Pin_4);
		}
		break;
	case 4:
		{
			if(ctrInput)
				GPIO_SetBits(GPIOB,GPIO_Pin_9);
			else
				GPIO_ResetBits(GPIOB,GPIO_Pin_9);
		}
		break;
	case 5:
		{
			if(ctrInput)
				GPIO_SetBits(GPIOE,GPIO_Pin_4);
			else
				GPIO_ResetBits(GPIOE,GPIO_Pin_4);
		}
		break;
	case 6:
		{
			if(ctrInput)
				GPIO_SetBits(GPIOA,GPIO_Pin_0);
			else
				GPIO_ResetBits(GPIOA,GPIO_Pin_0);
		}
		break;
	case 7:
		{
			if(ctrInput)
				GPIO_SetBits(GPIOE,GPIO_Pin_14);
			else
				GPIO_ResetBits(GPIOE,GPIO_Pin_14);
		}
		break;
	default:
		break;
	}
}
//���¶�Ӧĳһ¥����������������
void floorButtonControl(int16_t floorID, CtrInput_TypeDef ctrInput)
{
	uint8_t output,relayID;
	relayID = convertFloorIDToRelayID(floorID);
	//relayID = floorID;
	//�ж�����Խ��
	if(relayID > RELAY_ID_MAX) return;
	//�����DISABLE,�������е�������ʹ��λ���������Ͳ��������
	if(!ctrInput)
	{
		GPIO_SetBits(GPIOD,GPIO_Pin_8);
		GPIO_SetBits(GPIOD,GPIO_Pin_15);
		GPIO_SetBits(GPIOA,GPIO_Pin_8);
		GPIO_SetBits(GPIOD,GPIO_Pin_4);
		GPIO_SetBits(GPIOB,GPIO_Pin_9);
		GPIO_SetBits(GPIOE,GPIO_Pin_4);
		GPIO_SetBits(GPIOA,GPIO_Pin_0);
		GPIO_SetBits(GPIOE,GPIO_Pin_14);
		return;
	}
	//�����������
	output = relayID % DECODER_OPIN_NUM;
	switch(relayID / DECODER_OPIN_NUM)
	{
	case 0:
		{
			//A3
			if(output & 0x08)
				GPIO_SetBits(GPIOB,GPIO_Pin_15);
			else
				GPIO_ResetBits(GPIOB,GPIO_Pin_15);
			//A2
			if(output & 0x04)
				GPIO_SetBits(GPIOB,GPIO_Pin_14);
			else
				GPIO_ResetBits(GPIOB,GPIO_Pin_14);
			//A1
			if(output & 0x02)
				GPIO_SetBits(GPIOB,GPIO_Pin_13);
			else
				GPIO_ResetBits(GPIOB,GPIO_Pin_13);
			//A0
			if(output & 0x01) 
				GPIO_SetBits(GPIOB,GPIO_Pin_12);
			else 
				GPIO_ResetBits(GPIOB,GPIO_Pin_12);
		}
		break;
	case 1:
		{
			//A3
			if(output & 0x01)
				GPIO_SetBits(GPIOD,GPIO_Pin_11);
			else
				GPIO_ResetBits(GPIOD,GPIO_Pin_11);
			//A2
			if(output & 0x02)
				GPIO_SetBits(GPIOD,GPIO_Pin_12);
			else
				GPIO_ResetBits(GPIOD,GPIO_Pin_12);
			//A1
			if(output & 0x04)
				GPIO_SetBits(GPIOD,GPIO_Pin_13);
			else
				GPIO_ResetBits(GPIOD,GPIO_Pin_13);
			//A0
			if(output & 0x08) 
				GPIO_SetBits(GPIOD,GPIO_Pin_14);
			else 
				GPIO_ResetBits(GPIOD,GPIO_Pin_14);
		}
		break;
	case 2:
		{
			//A3
			if(output & 0x01)
				GPIO_SetBits(GPIOC,GPIO_Pin_6);
			else
				GPIO_ResetBits(GPIOC,GPIO_Pin_6);
			//A2
			if(output & 0x02)
				GPIO_SetBits(GPIOC,GPIO_Pin_7);
			else
				GPIO_ResetBits(GPIOC,GPIO_Pin_7);
			//A1
			if(output & 0x04)
				GPIO_SetBits(GPIOC,GPIO_Pin_8);
			else
				GPIO_ResetBits(GPIOC,GPIO_Pin_8);
			//A0
			if(output & 0x08) 
				GPIO_SetBits(GPIOC,GPIO_Pin_9);
			else 
				GPIO_ResetBits(GPIOC,GPIO_Pin_9);
		}
		break;
	case 3:
		{
			//A3
			if(output & 0x01)
				GPIO_SetBits(GPIOD,GPIO_Pin_0);
			else
				GPIO_ResetBits(GPIOD,GPIO_Pin_0);
			//A2
			if(output & 0x02)
				GPIO_SetBits(GPIOD,GPIO_Pin_1);
			else
				GPIO_ResetBits(GPIOD,GPIO_Pin_1);
			//A1
			if(output & 0x04)
				GPIO_SetBits(GPIOD,GPIO_Pin_2);
			else
				GPIO_ResetBits(GPIOD,GPIO_Pin_2);
			//A0
			if(output & 0x08) 
				GPIO_SetBits(GPIOD,GPIO_Pin_3);
			else 
				GPIO_ResetBits(GPIOD,GPIO_Pin_3);
		}
		break;
	case 4:
		{
			//A3
			if(output & 0x01)
				GPIO_SetBits(GPIOB,GPIO_Pin_5);
			else
				GPIO_ResetBits(GPIOB,GPIO_Pin_5);
			//A2
			if(output & 0x02)
				GPIO_SetBits(GPIOB,GPIO_Pin_6);
			else
				GPIO_ResetBits(GPIOB,GPIO_Pin_6);
			//A1
			if(output & 0x04)
				GPIO_SetBits(GPIOB,GPIO_Pin_7);
			else
				GPIO_ResetBits(GPIOB,GPIO_Pin_7);
			//A0
			if(output & 0x08) 
				GPIO_SetBits(GPIOB,GPIO_Pin_8);
			else 
				GPIO_ResetBits(GPIOB,GPIO_Pin_8);
		}
		break;
	case 5:
		{
			//A3
			if(output & 0x01)
				GPIO_SetBits(GPIOE,GPIO_Pin_0);
			else
				GPIO_ResetBits(GPIOE,GPIO_Pin_0);
			//A2
			if(output & 0x02)
				GPIO_SetBits(GPIOE,GPIO_Pin_1);
			else
				GPIO_ResetBits(GPIOE,GPIO_Pin_1);
			//A1
			if(output & 0x04)
				GPIO_SetBits(GPIOE,GPIO_Pin_2);
			else
				GPIO_ResetBits(GPIOE,GPIO_Pin_2);
			//A0
			if(output & 0x08) 
				GPIO_SetBits(GPIOE,GPIO_Pin_3);
			else 
				GPIO_ResetBits(GPIOE,GPIO_Pin_3);
		}
		break;
	case 6:
		{
			//A3
			if(output & 0x01)
				GPIO_SetBits(GPIOC,GPIO_Pin_0);
			else
				GPIO_ResetBits(GPIOC,GPIO_Pin_0);
			//A2
			if(output & 0x02)
				GPIO_SetBits(GPIOC,GPIO_Pin_1);
			else
				GPIO_ResetBits(GPIOC,GPIO_Pin_1);
			//A1
			if(output & 0x04)
				GPIO_SetBits(GPIOC,GPIO_Pin_2);
			else
				GPIO_ResetBits(GPIOC,GPIO_Pin_2);
			//A0
			if(output & 0x08) 
				GPIO_SetBits(GPIOC,GPIO_Pin_3);
			else 
				GPIO_ResetBits(GPIOC,GPIO_Pin_3);
		}
		break;
	case 7:
		{
			//A3
			if(output & 0x01)
				GPIO_SetBits(GPIOE,GPIO_Pin_10);
			else
				GPIO_ResetBits(GPIOE,GPIO_Pin_10);
			//A2
			if(output & 0x02)
				GPIO_SetBits(GPIOE,GPIO_Pin_11);
			else
				GPIO_ResetBits(GPIOE,GPIO_Pin_11);
			//A1
			if(output & 0x04)
				GPIO_SetBits(GPIOE,GPIO_Pin_12);
			else
				GPIO_ResetBits(GPIOE,GPIO_Pin_12);
			//A0
			if(output & 0x08) 
				GPIO_SetBits(GPIOE,GPIO_Pin_13);
			else 
				GPIO_ResetBits(GPIOE,GPIO_Pin_13);
		}
		break;
	default:
		break;
	}
	//�������������
	UpdateEncoderEnInput(relayID,CtrInput_Enable);
	delay_ms(2);
	UpdateEncoderEnInput(relayID,CtrInput_Disable);
}

//��ȡ&��䵱ǰ�豸�ĵ�ַ��Ϣ
void Update_Current_ADDRL(void)
{
	uint8_t res;
	
	//��䵱ǰ�豸�ĸߵ�ַ
	sysTaskStatus.localAddrH = LORA_ADDRH_ELEVATOR;
	
	//��䵱ǰ�豸�ĵ͵�ַ
	res = 0;
	if(ADDRL_BIT1) res += 4;
	if(ADDRL_BIT2) res += 2;
	if(ADDRL_BIT3) res += 1;
	sysTaskStatus.localAddrL = res;
	
	//��䵱ǰ�豸���ŵ�
	res = 0;
	if(CHANNEL_BIT1) res += 4;
	if(CHANNEL_BIT2) res += 2;
	if(CHANNEL_BIT3) res += 1;
	sysTaskStatus.localChannel = res;
	
	//printf("���ظߵ�ַ��%x���͵�ַ��%x���ŵ���%x.\r\n",sysTaskStatus.localAddrH,sysTaskStatus.localAddrL,sysTaskStatus.localChannel);
}
