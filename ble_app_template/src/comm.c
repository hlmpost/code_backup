//通讯协议的处理
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
//#include "app_uart.h"
#include "eric_gpio.h"


static uint8_t s_buffer[10];

extern uint8_t hrs_rate;//心率
extern uint32_t step_count;//步数
extern uint8_t bat_percent;//电池

//------------------------------------------------------
//计算校验和
static uint8_t check_sum(uint8_t* data,uint8_t len)
{
	uint8_t temp=0;
	for(int i=0;i<len;i++)
		temp+=data[i];
	return temp;
}
//================================================================
//发送确认命令
//fe 04 01 01 ab b0 ；fe 04 01 01 ae b3
//flag=1 发送成功确认，0-发送失败确认
void send_shakehand(uint8_t flag)
{
	s_buffer[0]=0xfe;
	s_buffer[1]=0x04;
	s_buffer[2]=0x01;//command
	s_buffer[3]=0x01;
	if(flag==1)
	{
		s_buffer[4]=0xab;
		s_buffer[5]=0xb0;
	}
	else
	{
		s_buffer[4]=0xae;
		s_buffer[5]=0xb3;
	}
	eric_uart_send(s_buffer,6);
}
//------------------------------------------
//大数据传输，需回复确认收到命令
void send_confirm(uint16_t sortno)
{
	s_buffer[0]=0xfe;
	s_buffer[1]=0x04;
	s_buffer[2]=0xab;//command
	s_buffer[3]=0x02;
	memcpy(&s_buffer[4],(uint8_t *)&sortno,2);
	s_buffer[6]=check_sum(s_buffer,6);
	eric_uart_send(s_buffer,7);
}
//--------------------------------------
//解析长度命令
uint16_t get_len(uint8_t *data)
{
	if(data[2]==0xaa && (data[5]==check_sum(data,5)) )
	{
		return data[4];
	}
	else
		return 0xff;
}
//-----------------------------------------------------
//处理接收到来自ST的数据
//return:1-收到命令正确；0-收到命令错误
void rece_dispatch(uint8_t *data)
{
	uint8_t len=data[0];
	uint8_t *temp;
	if(data[1]!=0xfe || data[2]!=0x05 || (data[len]!=check_sum(&data[1],len-1)) )
	{
		//收到错误命令回复
		send_shakehand(0);
	}
	//解析命令
	switch(data[3])
	{
		case 0xaa://同步传感器数据
			
			break;
		default:
			//直接发送数据到手机
		  //big data need confirm
			send_data_phone(&data[1],len);
			if(data[3]==0x07)
			{
				uint16_t temp=data[6];
				temp=(temp<<8)|data[5];
				send_confirm(temp);
			}

			return;
	};
}

//---------------------------------------------
void comm_init()
{
	//init int pin13 to st
	GPIO_InitType GPIO_InitStruct;

	GPIO_InitStruct.dir 	= GPIO_PIN_CNF_DIR_Output;
	GPIO_InitStruct.pull 	= GPIO_PIN_CNF_PULL_Disabled;
	GPIO_InitStruct.drive 	= GPIO_PIN_CNF_DRIVE_S0S1;
	GPIO_InitStruct.sense 	= GPIO_PIN_CNF_SENSE_Disabled;
	GPIO_InitStruct.input 	= GPIO_PIN_CNF_INPUT_Disconnect;
	GPIO_Init(13, &GPIO_InitStruct);
	GPIO_SetBit(13);//high
	
}
