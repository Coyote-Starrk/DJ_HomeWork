#ifndef __SYS_PARAM_H__
#define __SYS_PARAM_H__

#include "stm32f10x.h"

//��ַ��
#define	LORA_ADDRH_ROBOT			0x00	//������ģ��ߵ�ַ
#define	LORA_ADDRH_ELEVATOR 	0x4A	//����ģ��ߵ�ַ	J
#define LORA_ADDRH_TOPMODULE 	0x54	//�ݾ���ģ��ߵ�ַ	T
#define	LORA_ADDRH_SOFTWARE		0x4D	//�ͻ������������ģ��ߵ�ַ	M
#define	LORA_ADDRL_SOFTWARE		0x00	//�ͻ������������ģ��͵�ַ
#define	LORA_CHANNEL_SOFTWARE	0x00	//�ͻ������������ģ���ŵ�

//������
#define SYSTEM_VERSION				0x01	//��ǰϵͳ�İ汾��

#define MSG_COUNTER_UPBOUND		60		//ͬ������ϢA2/4�ۼ���������60����ʱ��ϵͳ��������
#define TIMEOUT_MAX						10		//����10sδ�յ����ģ�������������

#define RECV_BUFFER_MAX_SIZE 	1024	//���ջ������������
#define TMP_BUFFER_MAX_SIZE		65		//��ʱ���������65���ֽ�
//#define SEND_BUFFER_MAX_SIZE 	1024	//���ͻ������������

#define UWB_DATA_LEN  			65  		//UWBÿ�ν��մ��о�������ݱ��ĳ����ǹ̶���65���ֽ�

#define HEAD_CRC_SIZE					18		//headCRC��ҪУ���ͷ������

#define RELAY_ID_MAX					127		//�̵���ID���ֵ
#define RELAY_ID_MIN					0			//�̵���ID��Сֵ

#define TOLERABLE_CAL_IS_STOP_ERR 				200		//�������̵ľ������
#define TOLERABLE_CAL_CURRENT_FLOOR_ERR 				400		//�������̵ľ������

#define DECODER_OPIN_NUM			16		//ÿ�������������������


//ͨ��Э���ǰ�����ֶ�Ϊ�̶�ֵ
#define SYS_MSG_HEAD1					0x1E
#define SYS_MSG_HEAD2					0x1F

//ͨ�Žṹ��msgTypeȡֵ��Χ
#define ELEVATOR_TO_ROBOT 		0xB0	//����������˷��͵ı���
#define	ROBOT_TO_ELEVATOR			0xB1	//����������ݷ��͵ı���
#define	ELEVATOR_TO_TOPMODULE	0xB2	//�����򶥲�ģ�鷢�͵ı���
#define	TOPMODULE_TO_ELEVATOR	0xB3	//����ģ������ݷ��͵ı���
#define	ELEVATOR_TO_CLIENT		0xB4	//������ͻ��˷��͵ı���
#define	CLIENT_TO_ELEVATOR		0xB5	//�ͻ�������ݷ��͵ı���

//ͨ�Žṹ��cmdIDȡֵ��Χ
#define ROBOT_QUERY_STATUS		0xA0	//�����˲�ѯ������Ϣ
#define	ROBOT_ASK_FOR_USE			0xA1	//�����˷���ʹ������
#define	ROBOT_IS_ENTERING			0xA2	//���������ڽ������
#define	ROBOT_ENTER_READY			0xA3	//�������ѽ������
#define	ROBOT_IS_EXITING			0xA4	//�����������뿪����
#define	ROBOT_EXIT_READY			0xA5	//���������뿪����
#define	ROBOT_ASK_END					0xA6	//������ȡ���Ѿ����е�����

//״̬����ѡ״̬��0xD0-0xD4������cmdID��ѡȡֵ
#define ELEVATOR_NOT_IN_USE		0xD0	//���ݿ���
#define ELEVATOR_MVTO_FIRST		0xD1	//�����������һ������¥���ƶ�
#define ELEVATOR_ARVED_FIRST	0xD2	//���ݵִ��˵�һ������¥��
#define ELEVATOR_MVTO_SECOND	0xD3	//����������ڶ�������¥���ƶ�
#define ELEVATOR_ARVED_SECOND	0xD4	//���ݵִ��˵ڶ�������¥��
#define ELEVATOR_STATE_ERROR	0xDD	//�·�ָ��͵��ݵ�ǰ���̽׶β�ƥ��
#define ELEVATOR_PARAM_REJECT	0xDE	//�·��������󣬵��ݻؾ����������
#define ELEVATOR_BE_IN_USE		0xDF	//�������ڱ�����������ռ�ã����ߵ�ǰ����ά��״̬

//�ͻ���-����Ķ���cmdID
#define CLEAN_ALL_PARAM				0xE0	//���Ƕ��ʽ�����в���
#define SET_ONE_PARAM					0xE1	//���õ�������
#define ACK_CLEAN_ALL_PARAM		0xF0	//�ظ����Ƕ��ʽ�����в���
#define ACK_SET_ONE_PARAM			0xF1	//�ظ����õ�������

typedef enum _SYSEvent_{
	Event_0 = 0, 											//���¼�
	Event_Robot_Call = 0x80,					//֪ͨ״̬������������������ʹ�õ���
	Event_Robot_In = 0x40,						//֪ͨ״̬�����������ѽ������
	Event_Robot_Out = 0x20,						//֪ͨ״̬�����������Ѿ�������
	Event_Need_Report_Status = 0x10,	//֪ͨ״̬���������̵�ĩβ��������ϱ�һ������״̬
	Event_CirTrigger_FloorNow = 0x08,	//��ʼѭ�����µ�һ������¥��İ�ť
	Event_CirTrigger_FloorDst = 0x04,	//��ʼѭ�����µڶ�������¥��İ�ť
	Event_Quit_Mission	=	0x20				//������������
}SYSEvent;

//�ڴ������ÿһ���Ĵ洢�ṹ
typedef struct _SYSFloorMsg_{
	uint8_t	relayID;		//�̵���ID(���ݷ�Χ0~127)������λ������û�б�ʹ�ã�δ��ʹ��:����λ��1���ѱ�ʹ��:����λΪ0
	uint8_t needCard;		//�Ƿ���Ҫˢ����0������Ҫ   1����Ҫ
	int16_t	floorID;		//¥��ID
	uint32_t	distance;		//����߶�
}SYSFloorMsg;

//����״̬�ṹ��
typedef struct _SYSTaskStatus_{
	uint8_t				srcAddrH;					//�����豸�ߵ�ַ�����������Ƿ���ͬһ����������
	uint8_t				srcAddrL;					//�����豸�͵�ַ�����������Ƿ���ͬһ����������
	uint8_t				srcChannel;				//�����豸�ŵ������������Ƿ���ͬһ����������
	uint8_t				localAddrH;				//�����豸�ߵ�ַ
	uint8_t				localAddrL;				//�����豸�͵�ַ
	uint8_t				localChannel;			//�����豸�ŵ���ַ
	uint8_t				sysStatus;				//ϵͳ�����ݣ���ǰ״̬(��ȡֵΪ0xD0/1/2/3/4/E),���궨��
	uint8_t				reserved;
	
	int16_t				floorNow;					//����Դ¥�㣬X¥��
	int16_t				floorDst;					//����Ŀ��㣬Y¥��
	uint16_t			taskID;						//������ID��ţ����������Ƿ���ͬһ��������
	//uint16_t			tmpTaskID;				//����
	uint16_t      sequence;					//��¼�����˷����͵����к�
	
	uint32_t			clk;							//��ʱ��ÿ50ms����һ��clk��ϵͳÿ������clk��0��ʼ�������������Ҫ6��ʱ��
	uint32_t			distance;					//ʵʱ����
	uint32_t			distanceFIFO[5];	//�����ж��Ƿ��ȶ�����ͬһ¥��
	SYSEvent			event;						//�������¼�
	SYSFloorMsg*	paramTable;				//ָ���ڴ��������
	
	uint16_t			loraBufHead;
	uint16_t			loraBufTail;
	uint16_t			loraBufRead;
	
	uint16_t			uwbBufHead;
	uint16_t			uwbBufTail;
	uint16_t			uwbBufRead;
	
	uint8_t				LORA_RX_BUFF[RECV_BUFFER_MAX_SIZE]; //LORA���ջ���
	uint8_t				UWB_RX_BUFF[RECV_BUFFER_MAX_SIZE]; 	//UWB���ջ���
	uint8_t				TMP_BUFFER[TMP_BUFFER_MAX_SIZE];		//��ʱ���ݴ����������������ջ������ڵĴ��������ݿ�������
}SYSTaskStatus;

//ͨ�ŵ�ַ
typedef struct _ADDR_TO_SEND_{
	uint8_t				addrH;
	uint8_t				addrL;
	uint8_t				channel;
}ADDR_ToSend;

//ͨ�Žṹ��Э���ײ�
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

//���Ի����˵���Ϣ����
typedef struct _PAYLOAD_ROBOT_MSG_{
	uint8_t				cmdID;
	uint8_t				Reserved;
	uint16_t			taskID;
	int16_t				floorNow;
	int16_t				floorDst;
}Payload_RobotMsg;

//���ڻظ������˵���Ϣ����
typedef struct _PAYLOAD_ROBOT_REPLY_{
	uint8_t				cmdID;
	uint8_t				Reserved;
	uint16_t			taskID;
	int16_t				floorNow;
	int16_t				floorDst;
}Payload_RobotReply;

//�����ֲֳ���������Ϣ����
typedef struct _PAYLOAD_CLIENT_MSG_{
	uint8_t				cmdID;
	int8_t				floorID;
	uint8_t				relayID;
	uint8_t				needCard;
}Payload_ClienMsg;

//���ڻظ��ֲֳ���������Ϣ����
typedef struct _PAYLOAD_CLIENT_REPLY_{
	uint8_t				cmdID;
	uint8_t				reserved;
	uint16_t			result;
}Payload_ClienReply;

extern SYSTaskStatus sysTaskStatus;

#endif /*__SYS_PARAM_H__*/




