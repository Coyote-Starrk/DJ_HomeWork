#include "gpio.h"

//GPIO初始化，包括板上电梯控制按钮，LED数据灯，拨码开关
void GPIO_Usr_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE,ENABLE);//使能PORTA,PORTB,PORTC,PORTD,PORTE时钟

	//开漏输出GPIO配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 	//速度为50MHz
	
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
	
	//推挽输出GPIO配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 	//速度为50MHz
	
	//解放PA15.PB3.PB4
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
	
	//数据灯LED初始化，PA15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 	//速度为50MHz
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_15;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);							

	//拨码开关初始化,ID地址设置PA4.5.6,信道地址设置PA7,PC4.5
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化下拉输入GPIOA4.5.6.7
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化下拉输入GPIOC4.5`	
	
	//根据拨码开关读入本地的地址和信道
	Update_Current_ADDRL();
	
	//所有的译码器使能管脚一开始都要处在高电平位置
	GPIO_SetBits(GPIOD,GPIO_Pin_8);
	GPIO_SetBits(GPIOD,GPIO_Pin_15);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	GPIO_SetBits(GPIOD,GPIO_Pin_4);
	GPIO_SetBits(GPIOB,GPIO_Pin_9);
	GPIO_SetBits(GPIOE,GPIO_Pin_4);
	GPIO_SetBits(GPIOA,GPIO_Pin_0);
	GPIO_SetBits(GPIOE,GPIO_Pin_14);
	
	//开启指示灯
	LED_ON;
}

//修改对应某一楼层的译码器使能引脚输入
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
//更新对应某一楼层的译码器数据输出
void floorButtonControl(int16_t floorID, CtrInput_TypeDef ctrInput)
{
	uint8_t output,relayID;
	relayID = convertFloorIDToRelayID(floorID);
	//relayID = floorID;
	//判断输入越界
	if(relayID > RELAY_ID_MAX) return;
	//如果是DISABLE,拉高所有的译码器使能位，译码器就不会再输出
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
	//计算译码输出
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
	//更新译码器输出
	UpdateEncoderEnInput(relayID,CtrInput_Enable);
	delay_ms(2);
	UpdateEncoderEnInput(relayID,CtrInput_Disable);
}

//获取&填充当前设备的地址信息
void Update_Current_ADDRL(void)
{
	uint8_t res;
	
	//填充当前设备的高地址
	sysTaskStatus.localAddrH = LORA_ADDRH_ELEVATOR;
	
	//填充当前设备的低地址
	res = 0;
	if(ADDRL_BIT1) res += 4;
	if(ADDRL_BIT2) res += 2;
	if(ADDRL_BIT3) res += 1;
	sysTaskStatus.localAddrL = res;
	
	//填充当前设备的信道
	res = 0;
	if(CHANNEL_BIT1) res += 4;
	if(CHANNEL_BIT2) res += 2;
	if(CHANNEL_BIT3) res += 1;
	sysTaskStatus.localChannel = res;
	
	//printf("本地高地址：%x，低地址：%x，信道：%x.\r\n",sysTaskStatus.localAddrH,sysTaskStatus.localAddrL,sysTaskStatus.localChannel);
}
