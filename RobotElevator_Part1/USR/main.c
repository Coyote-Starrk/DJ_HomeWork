#include "stm32f10x_it.h"
#include "lora.h"
#include "uwb.h"
#include "gpio.h"
#include "sys_utils.h"
#include "flash.h"
#include "string.h"
#include "sys_param.h"

#include <stdio.h>

//���̲���ͳһ����
SYSTaskStatus sysTaskStatus;

//ϵͳ��ʼ��
void ELEVATOR_SystemInit(void);
//�ж���������Ƿ�Ϸ�
uint8_t ELEVATOR_ParamIsVaild(int16_t floorNow,int16_t floorDst);
//���㵱ǰ���ݵĵ�ǰ¥��/ͣ��״̬������ֵ��0~127����Ӧ¥��̵���ID������-1������ݻ����ƶ�
int8_t ELEVATOR_IsElevatorStop(void);
//��õ�ǰdistance��Ӧ��¥��λ��
int16_t ELEVATOR_GetCurrentFloorPosition(void);
//�������С��ʱ����ϵͳ�쳣�˳��ģ��������ú���
void ELEVATOR_ResetSystemStatus(void);
//����clk��eventˢ��GPIO���
void ELEVATOR_SwitchCtrl(void);
//��Ϣ����
void ELEVATOR_MsgHandler(SYS_MsgHead* msg);

//���ڼ�¼��һ��GPIO������ʱ���
static uint64_t CLK_preGpioCtrl = 0;	
//������һ�ε�distanceFIFO[5]����ʱ��
static uint64_t CLK_LastFifoUpdate = 0;
//������һ�α��Ľ��յ�ʱ��
static uint64_t CLK_LastMsg = 0;
//���ڼ�¼���ŵ�ʱ�̣�����ʱ
static uint64_t CLK_DoorOpen;	
//���ڼ�¼��һ��ѭ������floorNow�����ʱ��
static uint64_t CLK_LastCircleFloorNow;
//���ڼ�¼��һ��ѭ������floorDst�����ʱ��
static uint64_t CLK_LastCircleFloorDst;

//0-��ˢ��ѭ�� 1~4 floorNow��ѭ�� 5~8 floorDst��ѭ��
static uint8_t state = 0;	


int main_t(void)
{
	//SYSFloorMsg param;
	//int pageOffset;
	FLASH_CleanAllParam();
	//���Դ�ӡ���ڳ�ʼ��/��ʱ��ʼ��/clk��ʼ��
	Utils_ModuleInit();
	//flash��ʼ��
	FLASH_ModuleInit();
	//��ʼ��״̬��
	sysTaskStatus.sysStatus = 0xD0;
	//gpio��ʼ��
	GPIO_ModuleInit();
	//UWBģ���ʼ��
	//UWB_ModuleInit();
	//LORAģ���ʼ��
	//Lora_ModuleInit();
/*
	for(pageOffset=0;pageOffset<SECTOR_SIZE;pageOffset+=sizeof(SYSFloorMsg))
	{
		//ÿ������ռ���ĸ�����
		FLASH_ReadData(FLASH_PARAM_PAGE1_START+pageOffset,(uint16_t *)&param,4);
		printf("%d: floorID=%d,switchID=%d\r\n",pageOffset,param.floorID,param.switchID);
		if(param.distance==0xffffffff)
			break;
	}
		for(pageOffset=0;pageOffset<SECTOR_SIZE;pageOffset+=sizeof(SYSFloorMsg))
	{
		//ÿ������ռ���ĸ�����
		FLASH_ReadData(FLASH_PARAM_PAGE2_START+pageOffset,(uint16_t *)&param,4);
		printf("%d: floorID=%d,switchID=%d\r\n",pageOffset,param.floorID,param.switchID);
		if(param.distance==0xffffffff)
			break;
	}*/
	
	while(1)
	{
		printf("loop start\r\n");
		GPIO_FloorButtonCtrl(127,CtrInput_Enable);
		delay_ms(1000);
		GPIO_FloorButtonCtrl(127,CtrInput_Disable);
		delay_ms(1000);
	}
}

int main(void)
{			
	
	ELEVATOR_SystemInit();
	
	printf("\r\nwhile start\r\n");
	
	while(1)
  {
		uint16_t dataSize,payloadLen;
		//delay_ms(1000);
		//һ������һ��UWB��ȡ���ľ�����Ϣ	
		while(sysTaskStatus.uwbBufRead != sysTaskStatus.uwbBufTail &&
					(sysTaskStatus.uwbBufRead+1)%RECV_BUFFER_MAX_SIZE != sysTaskStatus.uwbBufTail)
		{
			if(sysTaskStatus.UWB_RX_BUFF[(sysTaskStatus.uwbBufRead+1)%RECV_BUFFER_MAX_SIZE] == 'm' &&
				 sysTaskStatus.UWB_RX_BUFF[(sysTaskStatus.uwbBufRead+2)%RECV_BUFFER_MAX_SIZE] == 'c')
			{
				//���㵱ǰReadָ��֮�󻺳������ȣ����ڽ������жϻ������Ƿ��Ѿ�������������౨�ģ�
				for(dataSize=2;(sysTaskStatus.uwbBufRead+dataSize)%RECV_BUFFER_MAX_SIZE != sysTaskStatus.uwbBufTail;)
					dataSize++;
				if(dataSize >= UWB_DATA_LEN)
				{
					//���������ݿ���������������
					size_t tmpOffset;
					for(tmpOffset=1;tmpOffset<=UWB_DATA_LEN;tmpOffset++)
					{
						sysTaskStatus.TMP_BUFFER[tmpOffset-1] = sysTaskStatus.UWB_RX_BUFF[(sysTaskStatus.uwbBufRead+1)%RECV_BUFFER_MAX_SIZE];
						sysTaskStatus.uwbBufRead = (sysTaskStatus.uwbBufRead+1)%RECV_BUFFER_MAX_SIZE;
					}
#if 0					
					printf("\r\n����������Ϊ��\r\n");
					for(tmpOffset=0;tmpOffset<UWB_DATA_LEN;tmpOffset++)
					{
						USART_SendData(USART1, sysTaskStatus.TMP_BUFFER[tmpOffset]);
						while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
					}
					printf("\r\n��ʼ�������\r\n");
					//1.3 ���ݱ��ļ���ʵʱ����
					printf("READ:%d - ",sysTaskStatus.uwbBufRead);
#endif					
					UWB_GetDistanceFromMsg((char*)sysTaskStatus.TMP_BUFFER);
				}else{
					break;
				}
			}else{
				sysTaskStatus.uwbBufRead = (sysTaskStatus.uwbBufRead+1)%RECV_BUFFER_MAX_SIZE;
			}
		}
		//1.4 ÿ��2500ms��������FIFO
		//FIFO�������������5��ʵʱ�������ݣ�ÿ�β������2500ms�����5�����ݵ�ȡֵ����ĳһ¥�㷶Χ�ڣ�˵������ͣ��
		if(sysTaskStatus.clk - CLK_LastFifoUpdate >= 50)	
		{
			uint8_t i;
			//FIFO��0��λ�ñ��������õ����ݣ�4��λ�ñ������¸��µ�����
			for(i=1;i<=4;i++)
				sysTaskStatus.distanceFIFO[i-1] = sysTaskStatus.distanceFIFO[i];
			sysTaskStatus.distanceFIFO[4] = sysTaskStatus.distance;
			//����fifoClk
			CLK_LastFifoUpdate = sysTaskStatus.clk;
			printf("FIFO update:%d-%d-%d-%d-%d\r\n",sysTaskStatus.distanceFIFO[0],sysTaskStatus.distanceFIFO[1],
						sysTaskStatus.distanceFIFO[2],sysTaskStatus.distanceFIFO[3],sysTaskStatus.distanceFIFO[4]);
		}		
		//������ȡһ֡��LORA���������յ��ı���
		
		//2.1 �������ջ�����������ƥ�䱨��ͷ��HEAD1,HEAD2
		//��Readָ����������ֽ�λ��������
		//printf("\r\nHEAD:%d��TAIL:%d��READ:%d\r\n",sysTaskStatus.loraBufHead,sysTaskStatus.loraBufTail,sysTaskStatus.loraBufRead);
		while(sysTaskStatus.loraBufRead != sysTaskStatus.loraBufTail &&
					(sysTaskStatus.loraBufRead+1)%RECV_BUFFER_MAX_SIZE != sysTaskStatus.loraBufTail)
		{
			//printf("\r\nHEAD:%d��TAIL:%d��READ:%d\r\n",sysTaskStatus.loraBufHead,sysTaskStatus.loraBufTail,sysTaskStatus.loraBufRead);
			//delay_ms(1000);	
			//printf("1. ��ʼ����Э���ͷHEAD1,HEAD2\r\n");
			//�������ݸպ�ΪЭ�鱨�Ĺ̶�ͷ��HEAD1,HEAD2��ʱ���ܹ�ȷ�ϱ����ڻ������е�λ��
			if(sysTaskStatus.LORA_RX_BUFF[(sysTaskStatus.loraBufRead+1)%RECV_BUFFER_MAX_SIZE] == SYS_MSG_HEAD1 &&
				 sysTaskStatus.LORA_RX_BUFF[(sysTaskStatus.loraBufRead+2)%RECV_BUFFER_MAX_SIZE] == SYS_MSG_HEAD2)
			{
				
				//2.2 ���㵱ǰReadָ��֮�󻺳������ȣ����ڽ������жϻ������Ƿ��Ѿ��������������ģ�
				for(dataSize=2;(sysTaskStatus.loraBufRead+dataSize)%RECV_BUFFER_MAX_SIZE != sysTaskStatus.loraBufTail;)
					dataSize++;
				
				//printf("2. �ҵ���Э���ͷHEAD1,HEAD2��READ��%d��TAIL��%d���ɶ����ĳ��ȣ�%d\r\n",
				//				sysTaskStatus.loraBufRead,sysTaskStatus.loraBufTail,dataSize);
				
				//����������ݳ��ȴ��ڰ�ͷ����
				if(dataSize >= sizeof(SYS_MsgHead))
				{
					//����ͷ����������������
					size_t tmpOffset;
					uint8_t headCRC,payloadCRC;
					//���������ݿ���������������
					for(tmpOffset=1;tmpOffset<=sizeof(SYS_MsgHead);tmpOffset++)
						sysTaskStatus.TMP_BUFFER[tmpOffset-1] = sysTaskStatus.LORA_RX_BUFF[(sysTaskStatus.loraBufRead+tmpOffset)%RECV_BUFFER_MAX_SIZE];
					//����Э��ͷ������headCRC
					headCRC = crc8((uint8_t*)sysTaskStatus.TMP_BUFFER,HEAD_CRC_SIZE);
					//У��headCRC
					if(((SYS_MsgHead*)sysTaskStatus.TMP_BUFFER)->headCRC == headCRC)
					{
						//ͷ��У��ͨ��
						payloadLen = ((SYS_MsgHead*)sysTaskStatus.TMP_BUFFER)->payloadLen;
						//printf("����Ϣͷ�����payload�ĳ���Ϊ��%d\r\n",payloadLen);
						
						//�����ж��Ƿ���յ����������ݰ�
						if(dataSize >= payloadLen + sizeof(SYS_MsgHead))
						{
							//���ݰ��Ѿ���������
							//���������ݿ���������������
							for(tmpOffset=1;tmpOffset<=payloadLen+sizeof(SYS_MsgHead);tmpOffset++)
								sysTaskStatus.TMP_BUFFER[tmpOffset-1] = sysTaskStatus.LORA_RX_BUFF[(sysTaskStatus.loraBufRead+tmpOffset)%RECV_BUFFER_MAX_SIZE];
							//����payloadCRC
							payloadCRC = crc8(((uint8_t*)sysTaskStatus.TMP_BUFFER)+sizeof(SYS_MsgHead),payloadLen);	
							//У��payload
							if(((SYS_MsgHead*)sysTaskStatus.TMP_BUFFER)->payloadCRC == payloadCRC)
							{
								//payload����У��ͨ��
								//2.4 ������Ϣ��������ִ�б��Ĵ���
								ELEVATOR_MsgHandler((SYS_MsgHead*)sysTaskStatus.TMP_BUFFER);
								//֪ͨ״̬������Ҫ�����̵�ĩβ��������ϱ�һ������״̬
								//printf("Deal msg;\r\n");
								//�ƶ�readָ��
								sysTaskStatus.loraBufRead = (sysTaskStatus.loraBufRead+sizeof(SYS_MsgHead)+payloadLen)%RECV_BUFFER_MAX_SIZE;
								//printf("��readָ���ƶ�������βHEAD:%d��TAIL:%d��READ:%d\r\n",sysTaskStatus.loraBufHead,sysTaskStatus.loraBufTail,sysTaskStatus.loraBufRead);
							}else{
								//payload����У�鲻ͨ��
								printf("Э��payloadУ��ʧ��\r\n");
								//��Readָ���ƶ�����һ���ֽ�λ�ÿ�ʼ����ƥ��
								sysTaskStatus.loraBufRead = (sysTaskStatus.loraBufRead+1)%RECV_BUFFER_MAX_SIZE;
							}
						}else{
							printf("Э��payloadδ�������\r\n");
							//���ݰ�payload����δ�������,��Ҫ����ѭ�������´ν�����һ֡���ٴ���
							break;
						}	
					}else{
						//ͷ��У�鲻ͨ��
						printf("Э��ͷ��У��ʧ��\r\n");
						//��Readָ���ƶ�����һ���ֽ�λ�ÿ�ʼ����ƥ��
						sysTaskStatus.loraBufRead = (sysTaskStatus.loraBufRead+1)%RECV_BUFFER_MAX_SIZE;
					}
				}else{
					printf("Э��ͷ��δ��������\r\n");
					//����������ݳ���С�ڰ�ͷ���ȣ�˵��������δ������������Ҫ����ѭ�������´ν�����һ֡���ٴ���	
					break;
				}					
			}else{
				//���head1��head2��ƥ�䣬��Readָ���ƶ�����һ���ֽ�λ�ÿ�ʼ����ƥ��
				sysTaskStatus.loraBufRead = (sysTaskStatus.loraBufRead+1)%RECV_BUFFER_MAX_SIZE;
			}
		}
	
		//��������clk����GPIO�������
		ELEVATOR_SwitchCtrl();
		
		//�ġ�����״̬��
		if(sysTaskStatus.sysStatus == 0xD0)				//���ݴ��ڿ���̬
		{
			//����л������������״̬�µĵ��ݣ����õ���ת��0xD1״̬
			if(sysTaskStatus.event & Event_Robot_Call)
			{
				sysTaskStatus.sysStatus = 0xD1;
			}
		}
		else if(sysTaskStatus.sysStatus == 0xD1)	//�������������������¥���ƶ�
		{
			//������ݵ�ǰͣ��״̬
			int8_t res = ELEVATOR_IsElevatorStop();
			if(res == -1)
			{
				//������δͣ�������»���������¥�㰴ť
				if(sysTaskStatus.clk - CLK_LastCircleFloorNow > BUTTON_TRIGGER_DELAY)
					SYSEvent_Set(Event_CirTrigger_FloorNow);
				//printf("����δͣ������ʼ������һ��\r\n");
			}else{
				//����ͣ����
				if(sysTaskStatus.paramTable[res].floorID == sysTaskStatus.floorNow)
				{
					//�����Ѿ�ͣ�����˵�һ������¥��
					//���¿��Ű���
					DOOR_OPEN();
					//��¼����ʱ��
					CLK_DoorOpen = sysTaskStatus.clk;
					//�޸�����״̬
					sysTaskStatus.sysStatus = 0xD2;
				}else{
					//����ͣ����������¥��֮�������¥�㣬ѭ�����»���������¥�㰴ť���������Զ�ε��ã�
					if(sysTaskStatus.clk - CLK_LastCircleFloorNow > BUTTON_TRIGGER_DELAY)
						SYSEvent_Set(Event_CirTrigger_FloorNow);
					//printf("����ͣ���ڱ��¥�㣬��ʼ������һ��\r\n");
				}
			}
		}
		else if(sysTaskStatus.sysStatus == 0xD2)	//�����Ѿ��������������¥�㣬�ȴ������˽���
		{
			if(sysTaskStatus.clk - CLK_DoorOpen > KEEP_DOOR_OPEN_MAX_PERIOD)
			{
				printf("20s���ų�ʱ����\r\n");
				//����Ѿ����ֿ���״̬��ʱ��ִ�ж�������
				ELEVATOR_ResetSystemStatus();
			}else{
				//δ��ʱ�����ְ��¿��Ű���
				DOOR_OPEN();
				//����л������Ѿ�������ݣ����õ���ת��0xD1״̬
				if(sysTaskStatus.event & Event_Robot_In)
				{
					sysTaskStatus.sysStatus = 0xD3;
					DOOR_CLOSE();
				}
			}
		}
		else if(sysTaskStatus.sysStatus == 0xD3)	//�������������������Ҫ��¥���ƶ�
		{
			//������ݵ�ǰͣ��״̬
			int8_t res = ELEVATOR_IsElevatorStop();
			if(res == -1)
			{
				//������δͣ��
				if(sysTaskStatus.clk - CLK_LastCircleFloorDst > BUTTON_TRIGGER_DELAY)
					SYSEvent_Set(Event_CirTrigger_FloorDst);
				//printf("����δͣ������ʼ�����ڶ���\r\n");
			}else{
				//����ͣ����
				if(sysTaskStatus.paramTable[res].floorID == sysTaskStatus.floorDst)
				{
					//�����Ѿ�ͣ�����˵ڶ�������¥��
					//���¿��Ű���
					DOOR_OPEN();
					//��¼����ʱ��
					CLK_DoorOpen = sysTaskStatus.clk;
					//�޸�����״̬
					sysTaskStatus.sysStatus = 0xD4;
				}else{
					//����ͣ����������¥��֮�������¥��
					if(sysTaskStatus.clk - CLK_LastCircleFloorDst > BUTTON_TRIGGER_DELAY)
						SYSEvent_Set(Event_CirTrigger_FloorDst);
					//printf("����ͣ���ڱ��¥�㣬��ʼ�����ڶ���\r\n");
				}
			}
		}
		else if(sysTaskStatus.sysStatus == 0xD4)	//�����Ѿ����������Ŀ��¥�㣬�ȴ��������˳�
		{
			if(sysTaskStatus.clk - CLK_DoorOpen > KEEP_DOOR_OPEN_MAX_PERIOD)
			{
				printf("20s���ų�ʱ����\r\n");
				//����Ѿ����ֿ���״̬��ʱ��ִ�ж�������
				ELEVATOR_ResetSystemStatus();
			}else{
				//δ��ʱ�����ְ��¿��Ű���
				DOOR_OPEN();
				//����л������Ѿ��뿪���ݣ����õ���ת��0xD0״̬
				if(sysTaskStatus.event & Event_Robot_Out)
				{
					sysTaskStatus.sysStatus = 0xD0;
					DOOR_CLOSE();
					SYSEvent_Set(Event_Quit_Mission);
				}
			}
		}

		//printf("����Ƿ���Ҫ�ϱ�����\r\n");
		//�塢���ݱ�־λ�����Ƿ�ظ����ϱ�����״̬
		if(sysTaskStatus.event & Event_Need_Report_Status)
		{
			ADDR_ToSend replyAddr;
			SYS_MsgHead replyMsg;
			Payload_RobotMsg replyPayload;
			
			//���±��Ľ���ʱ��
			CLK_LastMsg = sysTaskStatus.clk;
			//��������ϱ�����״̬
	
			//��䱨�Ļظ���ַ
			replyAddr.addrH = sysTaskStatus.srcAddrH;
			replyAddr.addrL = sysTaskStatus.srcAddrL;
			replyAddr.channel = sysTaskStatus.srcChannel;
			//�����Ϣ�����ֶ�
			replyPayload.cmdID = sysTaskStatus.sysStatus;
			replyPayload.Reserved = 0;
			replyPayload.taskID = sysTaskStatus.taskID;
			replyPayload.floorNow = ELEVATOR_GetCurrentFloorPosition();
			if(sysTaskStatus.sysStatus == 0xD0 || sysTaskStatus.sysStatus == 0xD1)
				replyPayload.floorDst = sysTaskStatus.floorNow;
			else
				replyPayload.floorDst = sysTaskStatus.floorDst;
			//��䱨��Э���ײ��̶ֹ��ֶ�
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
			//���ͱ���
			//printf("send msg\r\n");
			printf("״̬��֮��ظ����ģ�cmdID=%x,taskID=%d,floorNow=%d,floorDst=%d,status=%x\r\n",replyPayload.cmdID,replyPayload.taskID,
					replyPayload.floorNow,replyPayload.floorDst,sysTaskStatus.sysStatus);
			LORA_SendMsg(replyAddr,replyMsg,(char*)&replyPayload,sizeof(Payload_RobotMsg));
			//����¼�֪ͨ
			SYSEvent_Clr(Event_Need_Report_Status);
			//�������һ֡a5���ĵĻظ���ִ������֡�ظ�֮����Ҫִ����������
			if(sysTaskStatus.event & Event_Quit_Mission)
				ELEVATOR_ResetSystemStatus();	
		}		
		//�����������������̬�����ҳ���100*50=5000msδ�յ������˷�����Ϣ���߶�������
		if(sysTaskStatus.sysStatus != 0xD0 && sysTaskStatus.clk - CLK_LastMsg > TIMEOUT_MAX)
		{
			printf("10s��Ϣ�����ʱ����\r\n");
			//��������״̬
			ELEVATOR_ResetSystemStatus();
		}
		//�ߡ����LED������ʱ�䳬����100ms����ر�
		if(sysTaskStatus.clk - sysTaskStatus.ledClk > 2)
			LED_POWER_OFF();
		//ÿ��ѭ����ʱ20ms
		delay_ms(20);
  }//end while(1)
	//return 0;
}
 
//ϵͳ��ʼ��
void ELEVATOR_SystemInit(void)
{
	//���Դ�ӡ���ڳ�ʼ��/��ʱ��ʼ��/clk��ʼ��
	Utils_ModuleInit();
	//flash��ʼ��
	FLASH_ModuleInit();
	//��ʼ��״̬��
	sysTaskStatus.sysStatus = 0xD0;
	//gpio��ʼ��
	GPIO_ModuleInit();
	//UWBģ���ʼ��
	UWB_ModuleInit();
	//LORAģ���ʼ��
	Lora_ModuleInit();
}
//�ж���������Ƿ�Ϸ�
uint8_t ELEVATOR_ParamIsVaild(int16_t floorNow,int16_t floorDst)
{
	uint8_t position;
	uint8_t isFloorNowExist=0,isFloorDstExist=0;
	
	for(position=SWITCH_ID_MIN; position<=SWITCH_ID_MAX; position++)
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
//���㵱ǰ���ݵ�ͣ��״̬������ֵ��0~127����Ӧ¥��̵���ID������-1������ݻ����ƶ�
int8_t ELEVATOR_IsElevatorStop(void)
{
	uint8_t switchID;
	uint32_t upBound,loBound;
	for(switchID = SWITCH_ID_MIN;switchID<=SWITCH_ID_MAX;switchID++)
	{
		//�ڲ�������Ѱ�Һ�FIFO[0]��Ӧ��¥��
		if(sysTaskStatus.distanceFIFO[0] < sysTaskStatus.paramTable[switchID].distance + TOLERABLE_CAL_IS_STOP_ERR && 
				sysTaskStatus.distanceFIFO[0] > sysTaskStatus.paramTable[switchID].distance - TOLERABLE_CAL_IS_STOP_ERR)
			break;
	}
	//û�ҵ�
	if(switchID == SWITCH_ID_MAX+1) return -1;

	//ִ�е�����relayID�����б����˶�ӦFIFO[0]��ͣ��¥�㣬��������֤FIFO[1/2/3/4]��¥���Ƿ��FIFO[0]һ��
	upBound = sysTaskStatus.paramTable[switchID].distance + TOLERABLE_CAL_IS_STOP_ERR;
	loBound = sysTaskStatus.paramTable[switchID].distance - TOLERABLE_CAL_IS_STOP_ERR;
	if(sysTaskStatus.distanceFIFO[1] < upBound && sysTaskStatus.distanceFIFO[1] > loBound &&
		 sysTaskStatus.distanceFIFO[2] < upBound && sysTaskStatus.distanceFIFO[2] > loBound &&
		 sysTaskStatus.distanceFIFO[3] < upBound && sysTaskStatus.distanceFIFO[3] > loBound &&
		 sysTaskStatus.distanceFIFO[4] < upBound && sysTaskStatus.distanceFIFO[4] > loBound )
		return switchID;
	return -1;
}
//��õ�ǰdistance��Ӧ��¥��λ��
int16_t ELEVATOR_GetCurrentFloorPosition(void)//�޸ģ��������һ��¥�㣨TODO��
{
	uint8_t switchID;
	for(switchID = SWITCH_ID_MIN;switchID<=SWITCH_ID_MAX;switchID++)
	{
		//�ڲ�������Ѱ�Һ�FIFO[0]��Ӧ��¥��
		if(sysTaskStatus.paramTable[switchID].switchID != 0xff)
		{	
			if(sysTaskStatus.distance < sysTaskStatus.paramTable[switchID].distance + TOLERABLE_CAL_IS_STOP_ERR &&
				sysTaskStatus.distance > sysTaskStatus.paramTable[switchID].distance - TOLERABLE_CAL_IS_STOP_ERR)
			{
				return sysTaskStatus.paramTable[switchID].floorID;
			}
		}
	}
	return 0xffff;
}
//�������С��ʱ����ϵͳ�쳣�˳��ģ��������ú���
void ELEVATOR_ResetSystemStatus(void)
{
	//��������¼�
	sysTaskStatus.event = Event_0;
	//����ϵͳ����
	sysTaskStatus.srcAddrH = 0;
	sysTaskStatus.srcAddrL = 0;
	sysTaskStatus.srcChannel = 0;
	sysTaskStatus.floorDst = 0;
	sysTaskStatus.floorNow = 0;
	sysTaskStatus.taskID = 0;
	//ϵͳ״̬ת�Ƶ�����̬
	sysTaskStatus.sysStatus = 0xD0;
	//��֤OPEN_DOOR��gpio���Ϊ�ߵ�ƽ
	DOOR_CLOSE();
}
//����clk��eventˢ��GPIO���
void ELEVATOR_SwitchCtrl(void)
{
	uint32_t clk = sysTaskStatus.clk;
	//��ʼ��������
	if(state == 0 && (sysTaskStatus.event & Event_CirTrigger_FloorNow || sysTaskStatus.event & Event_CirTrigger_FloorDst))
	{
		//����ʱ��
		CLK_preGpioCtrl = clk;
		printf("start time clk = %lld\r\n",CLK_preGpioCtrl);
		//��¼ˢ������
		if(sysTaskStatus.event & Event_CirTrigger_FloorNow) state = 1;
		if(sysTaskStatus.event & Event_CirTrigger_FloorDst) state = 5;
		
		if(state == 1 && sysTaskStatus.paramTable[convertFloorIDToSwitchID(sysTaskStatus.floorNow)].needCard == 1)
		{
			//����ˢ���̵������ߵ�ƽ��TODO��
			printf("¥��%dˢ��ʧ��,��Ӧ�̵���%d,clk=%d\r\n",sysTaskStatus.floorNow,convertFloorIDToSwitchID(sysTaskStatus.floorNow),clk);
		}
		if(state == 2 && sysTaskStatus.paramTable[convertFloorIDToSwitchID(sysTaskStatus.floorDst)].needCard == 1)
		{
			//����ˢ���̵������ߵ�ƽ��TODO��
			printf("¥��%dˢ��ʧ�ܣ���Ӧ�̵���%d,clk=%d\r\n",sysTaskStatus.floorDst,convertFloorIDToSwitchID(sysTaskStatus.floorDst),clk);
		}
	}
	//���¿��Ű�ť
	if((state == 1 || state == 5) && (clk-CLK_preGpioCtrl > 2))
	{
		//����ʱ��
		CLK_preGpioCtrl = clk;
		if(state == 1)
		{
			GPIO_FloorButtonCtrl(sysTaskStatus.floorNow,CtrInput_Enable);	
			printf("����%d�㰴ť����Ӧ�̵���%d,clk=%d\r\n",sysTaskStatus.floorNow,convertFloorIDToSwitchID(sysTaskStatus.floorNow),clk);
		}
		if(state == 5)
		{
			GPIO_FloorButtonCtrl(sysTaskStatus.floorDst,CtrInput_Enable);
			printf("����%d�㰴ť����Ӧ�̵���%d,clk=%d\r\n",sysTaskStatus.floorDst,convertFloorIDToSwitchID(sysTaskStatus.floorDst),clk);
		}
		state++;
	}
	//̧���Ű�ť
	if((state == 2 || state == 6) && (clk-CLK_preGpioCtrl > 22))
	{
		//����ʱ��
		CLK_preGpioCtrl = clk;
		if(state == 2)
		{
			GPIO_FloorButtonCtrl(sysTaskStatus.floorNow,CtrInput_Disable);
			printf("̧��%d�㰴ť����Ӧ�̵���%d,clk=%d\r\n",sysTaskStatus.floorNow,convertFloorIDToSwitchID(sysTaskStatus.floorNow),clk);
		}			
		if(state == 6)
		{
			GPIO_FloorButtonCtrl(sysTaskStatus.floorDst,CtrInput_Disable);
			printf("̧��%d�㰴ť����Ӧ�̵���%d,clk=%d\r\n",sysTaskStatus.floorDst,convertFloorIDToSwitchID(sysTaskStatus.floorDst),clk);
		}
		state++;
	}
	//ȡ��ˢ��
	if((state == 3 || state == 7) && (clk-CLK_preGpioCtrl > 2)) 
	{
		//����ʱ��
		CLK_preGpioCtrl = clk;
		if(state == 1 && sysTaskStatus.paramTable[convertFloorIDToSwitchID(sysTaskStatus.floorNow)].needCard == 1)
		{
			//����ˢ���̵������͵�ƽ��TODO��
			printf("¥��%dˢ��ʧ��,��Ӧ�̵���%d,clk=%d\r\n",sysTaskStatus.floorNow,convertFloorIDToSwitchID(sysTaskStatus.floorNow),clk);
		}
		if(state == 2 && sysTaskStatus.paramTable[convertFloorIDToSwitchID(sysTaskStatus.floorDst)].needCard == 1)
		{
			//����ˢ���̵������͵�ƽ��TODO��
			printf("¥��%dˢ��ʧ�ܣ���Ӧ�̵���%d,clk=%d\r\n",sysTaskStatus.floorDst,convertFloorIDToSwitchID(sysTaskStatus.floorDst),clk);
		}
		state++;
	}
	//��ʱ2s
	if((state == 4 || state == 8) && (clk-CLK_preGpioCtrl > 2)) 
	{
		if(state == 4)
		{
			SYSEvent_Clr(Event_CirTrigger_FloorNow);
			CLK_LastCircleFloorNow = sysTaskStatus.clk;
		}
		if(state == 8)
		{
			SYSEvent_Clr(Event_CirTrigger_FloorDst);
			CLK_LastCircleFloorDst = sysTaskStatus.clk;
		}
		state = 0;
		printf("һ�ְ�������,clk=%d\r\n",clk);
	}
}

//��Ϣ����
void ELEVATOR_MsgHandler(SYS_MsgHead* msg)
{
	ADDR_ToSend replyAddr;
	SYS_MsgHead replyMsg;
	
	//��䱨�Ļظ���ַ
	replyAddr.addrH = msg->srcAddrH;
	replyAddr.addrL = msg->srcAddrL;
	replyAddr.channel = msg->srcChannel;
	//��䱨��Э���ײ��̶ֹ��ֶ�
	replyMsg.head1 = SYS_MSG_HEAD1;
	replyMsg.head2 = SYS_MSG_HEAD2;
	replyMsg.sequence = msg->sequence+1;
	replyMsg.srcAddrH = sysTaskStatus.localAddrH;
	replyMsg.srcAddrL = sysTaskStatus.localAddrL;
	replyMsg.srcChannel = sysTaskStatus.localChannel;
	replyMsg.timeStamp1 = 0;
	replyMsg.timeStamp2 = 0;
	
	//�����յ���Э�鱨��
	switch(msg->msgType)
	{
	case ROBOT_TO_ELEVATOR:
	case RELAY_TO_ELEVATOR://
		{
			//�������ĵ�payload����
			Payload_RobotMsg* payload = (Payload_RobotMsg*)(msg+1);
			//�����ظ��õ���Ϣ���ض�
			Payload_RobotReply payloadReply;
			//�Ƿ���Ҫ�ظ���֡��Ϣ
			uint8_t isNeedSend = 0;
			
			printf("�յ����cmdID=%x,taskID=%d,floorNow=%d,floorDst=%d-----%x\r\n",
				payload->cmdID,payload->taskID,payload->floorNow,payload->floorDst,sysTaskStatus.sysStatus);
			
			switch(payload->cmdID)
			{
			case 0xA0:	//������ѯ�ʵ�����Ϣ
				{
					//�ظ�����������״̬
					payloadReply.cmdID = sysTaskStatus.sysStatus;
					isNeedSend = 1;
				}
				break;
			case 0xA1:	//���������������ʹ��
				{
					//�жϲ����Ƿ�Ϸ�
					if(ELEVATOR_ParamIsVaild(payload->floorNow,payload->floorDst))
					{
						//����ϵͳ״̬�ж��Ƿ��������
						if(sysTaskStatus.sysStatus == 0xD0)//���ݿ���̬������ʹ������
						{
							//������������Ϣ
							sysTaskStatus.srcAddrH = msg->srcAddrH;
							sysTaskStatus.srcAddrL = msg->srcAddrL;
							sysTaskStatus.srcChannel = msg->srcChannel;
							//���±���������Ϣ
							sysTaskStatus.taskID = payload->taskID;
							sysTaskStatus.floorDst = payload->floorDst;
							sysTaskStatus.floorNow = payload->floorNow;
							//֪ͨ״̬������������������ʹ�õ���
							SYSEvent_Set(Event_Robot_Call);
							//֪ͨ״̬������Ҫ�����̵�ĩβ��������ϱ�һ������״̬
							SYSEvent_Set(Event_Need_Report_Status);
						
						}else if(sysTaskStatus.sysStatus == 0xD1 || sysTaskStatus.sysStatus == 0xD2)//D2Ҳ�п����յ�A1
						{
							//���������������̬������Ҫ�ж�magicNum
							if(sysTaskStatus.taskID == payload->taskID)
							{
								//0xD1״̬
								//֪ͨ״̬������Ҫ�����̵�ĩβ��������ϱ�һ������״̬
								SYSEvent_Set(Event_Need_Report_Status);
							}else{
								//����magicNum��ƥ���ֱ�ӻظ�0xDF�ܾ�һ������
								payloadReply.cmdID = 0xDF;
								isNeedSend = 1;
							}
						}else{
							//������ݴ���0xD3/4 ��״̬
							//������Ҫ�ж�magicNum
							if(sysTaskStatus.taskID == payload->taskID)
							{
								//���̽׶��쳣������
								payloadReply.cmdID = 0xDD;
								isNeedSend = 1;
							}else{
								//����magicNum��ƥ���ֱ�ӻظ�0xDF�ܾ�һ������
								payloadReply.cmdID = 0xDF;
								isNeedSend = 1;
							}
						}
					}else{
						//�����Ƿ������ûظ����ģ��ظ�0xDE(ָ������)
						payloadReply.cmdID =0xDE;
						isNeedSend = 1;
					}
				}
				break;
			case 0xA2:	//���������������
				{
					//�����ж�magicNum�Ƿ�ƥ��
					if(sysTaskStatus.taskID == payload->taskID)
					{
						if(sysTaskStatus.sysStatus == 0xD2)
						{
							//֪ͨ״̬������Ҫ�����̵�ĩβ��������ϱ�һ������״̬
							SYSEvent_Set(Event_Need_Report_Status);
						}else{
							//0xD0/1/3/4״̬
							//���̽׶��쳣������
							payloadReply.cmdID = 0xDD;
							isNeedSend = 1;
						}
					}else{
						//magicNum��ƥ���ֱ�ӻظ�0xDF�ܾ�һ������
						payloadReply.cmdID = 0xDF;
						isNeedSend = 1;
					}
				}
				break;
			case 0xA3:	//�������ѽ������
				{
					//�����ж�magicNum�Ƿ�ƥ��
					if(sysTaskStatus.taskID == payload->taskID)
					{
						if(sysTaskStatus.sysStatus == 0xD2)
						{
							//֪ͨ״̬�����������Ѿ��������
							SYSEvent_Set(Event_Robot_In);
							//֪ͨ״̬������Ҫ�����̵�ĩβ��������ϱ�һ������״̬
							SYSEvent_Set(Event_Need_Report_Status);
						}else if(sysTaskStatus.sysStatus == 0xD3 || sysTaskStatus.sysStatus == 0xD4){
							//֪ͨ״̬������Ҫ�����̵�ĩβ��������ϱ�һ������״̬
							SYSEvent_Set(Event_Need_Report_Status);
						}else{
							//0xD0/1״̬
							//���̽׶��쳣������
							payloadReply.cmdID = 0xDD;
							isNeedSend = 1;
						}
					}else{
						//magicNum��ƥ���ֱ�ӻظ�0xDF�ܾ�һ������
						payloadReply.cmdID = 0xDF;
						isNeedSend = 1;
					}
				}
				break;
			case 0xA4:	//���������˳�����
				{
					//�����ж�magicNum�Ƿ�ƥ��
					if(sysTaskStatus.taskID == payload->taskID)
					{
						if(sysTaskStatus.sysStatus == 0xD4)
						{
							//֪ͨ״̬������Ҫ�����̵�ĩβ��������ϱ�һ������״̬
							SYSEvent_Set(Event_Need_Report_Status);
						}else{
							//0xD0/1/2/3״̬
							//���̽׶��쳣������
							payloadReply.cmdID = 0xDD;
							isNeedSend = 1;
						}
					}else{
						//magicNum��ƥ���ֱ�ӻظ�0xDF�ܾ�һ������
						payloadReply.cmdID = 0xDF;
						isNeedSend = 1;
					}
				}
				break;
			case 0xA5:	//���������˳�����
				{
					//�����ж�magicNum�Ƿ�ƥ��
					if(sysTaskStatus.taskID == payload->taskID)
					{
						if(sysTaskStatus.sysStatus == 0xD4)
						{
							//֪ͨ״̬�����������Ѿ��뿪����
							SYSEvent_Set(Event_Robot_Out);
							//֪ͨ״̬������Ҫ�����̵�ĩβ��������ϱ�һ������״̬
							SYSEvent_Set(Event_Need_Report_Status);
						}else{
							//0xD0/1/2/3״̬
							//���̽׶��쳣������
							payloadReply.cmdID = 0xDD;
							isNeedSend = 1;
						}
					}else{
						//magicNum��ƥ���ֱ�ӻظ�0xDF�ܾ�һ������
						payloadReply.cmdID = 0xDF;
						isNeedSend = 1;
					}
				}
				break;
			case 0xAF:	//������ȡ���Ѻ��еı�������
				{
					//�����ж�magicNum�Ƿ�ƥ��
					if(sysTaskStatus.taskID == payload->taskID)
					{
						//����״̬
						ELEVATOR_ResetSystemStatus();
						//�ظ�0xD0����̬�������ѱ�����
						payloadReply.cmdID = 0xD0;
						isNeedSend = 1;
					}else{
						//magicNum��ƥ���ֱ�ӻظ�0xDF�ܾ�һ������
						payloadReply.cmdID = 0xDF;
						isNeedSend = 1;
					}
				}
				break;
			default:
				break;
			}
			//�������Ҫ��״̬��֮��ظ��ı��ģ���Ҫ��¼������Ϣ�����к�
			if(sysTaskStatus.event & Event_Need_Report_Status)
				sysTaskStatus.sequence = msg->sequence;
			//����д����͸������˷��Ļظ����ģ����÷���
			if(isNeedSend)
			{
				//�����Ϣ����ʣ���ֶ�
				payloadReply.Reserved = 0;
				payloadReply.taskID = payload->taskID;
				payloadReply.floorNow = ELEVATOR_GetCurrentFloorPosition();
				if(sysTaskStatus.sysStatus == 0xD0 || sysTaskStatus.sysStatus == 0xD1)
					payloadReply.floorDst = payload->floorNow;
				else
					payloadReply.floorDst = payload->floorDst;
				//��䱨��ʣ���ֶ�
				replyMsg.msgType = ELEVATOR_TO_ROBOT;
				replyMsg.payloadLen = sizeof(Payload_RobotReply);
				replyMsg.headCRC = crc8((uint8_t*)&replyMsg,HEAD_CRC_SIZE);
				replyMsg.payloadCRC = crc8((uint8_t*)&payloadReply,sizeof(Payload_RobotReply));
				//���ͱ���
				printf("�ظ����ģ�cmdID=%x,taskID=%d,floorNow=%d,floorDst=%d,status=%x\r\n",payloadReply.cmdID,payloadReply.taskID,
					payloadReply.floorNow,payloadReply.floorDst,sysTaskStatus.sysStatus);
				LORA_SendMsg(replyAddr,replyMsg,(char*)&payloadReply,sizeof(Payload_RobotReply));
			}
		}
		break;
	case OPERATOR_TO_ELEVATOR://OPERATOR
		{
			//�������ĵ�payload����
			Payload_ClienMsg* payload = (Payload_ClienMsg*)(msg+1);
			//�����ظ��õ���Ϣ���ض�
			Payload_ClienReply payloadReply;
			
			switch(payload->cmdID)
			{
			case CLEAN_ALL_PARAM:
				{
					//����ڴ����
					memset(sysTaskStatus.paramTable, 0xff, FLOOR_MAX*sizeof(SYSFloorMsg));
					//���flash����
					FLASH_CleanAllParam();
					printf("����ڴ����\r\n");
					//���ظ�����
					payloadReply.cmdID = ACK_CLEAN_ALL_PARAM;
					payloadReply.result = 1;
				}
				break;
			case SET_ONE_PARAM:
				{
					int x;
					//ɾ���������ظ���¥��ID,��ֹ������ͻ
					for(x=0;x<FLOOR_MAX;x++)
					{
						if(sysTaskStatus.paramTable[x].switchID != 0xff && sysTaskStatus.paramTable[x].floorID == payload->floorID)
						{
							memset(&sysTaskStatus.paramTable[x],0xff,sizeof(SYSFloorMsg));
							printf("�����ظ�¥�������x=%d,floorID=%d��floorID=%d\r\n",x,sysTaskStatus.paramTable[x].floorID,payload->floorID);
						}
					}
					
					//�����µĲ���
					sysTaskStatus.paramTable[payload->switchID].switchID = payload->switchID;
					sysTaskStatus.paramTable[payload->switchID].floorID = payload->floorID;
					sysTaskStatus.paramTable[payload->switchID].needCard = payload->needCard;
					sysTaskStatus.paramTable[payload->switchID].distance = sysTaskStatus.distance;
					
					printf("���Ӳ�����floorID:%d,relayID:%d,needcard=%d,distance=%d\r\n",
								sysTaskStatus.paramTable[payload->switchID].floorID,sysTaskStatus.paramTable[payload->switchID].switchID,
								sysTaskStatus.paramTable[payload->switchID].needCard,sysTaskStatus.paramTable[payload->switchID].distance);
					//д��flash
					FLASH_AddOneParam(sysTaskStatus.paramTable[payload->switchID]);
					//�����Ϣ����ʣ���ֶ�
					payloadReply.result = 1;
					payloadReply.cmdID = ACK_SET_ONE_PARAM;
				}
				break;
			default:
				break;
			}
			//��䱨��ʣ���ֶ�
			payloadReply.reserved = 0;
			replyMsg.msgType = ELEVATOR_TO_OPERATOR;
			replyMsg.payloadLen = sizeof(Payload_ClienReply);
			replyMsg.headCRC = crc8((uint8_t*)&replyMsg,HEAD_CRC_SIZE);
			replyMsg.payloadCRC = crc8((uint8_t*)&payloadReply,sizeof(Payload_ClienReply));
			//���ͱ���
			printf("����res���ĸ��س���=%d,cmdID=%x,result=%d\r\n",sizeof(Payload_ClienReply),payloadReply.cmdID,payloadReply.result);
			LORA_SendMsg(replyAddr,replyMsg,(char*)&payloadReply,sizeof(Payload_ClienReply));
		}
		break;
	default:
		break;
	}
}
//All end



