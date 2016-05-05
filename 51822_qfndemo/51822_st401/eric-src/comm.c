//处理和51822 数据通讯协议
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "usart.h"
#include "comm.h"
#include "eric_flash.h"

#define FLASH_UNIT_LENGTH (10)//单位数据长度

extern uint8_t buffer[];


extern stru_region current_sensor_data;

extern volatile uint8_t batt_status;//current battery percent

extern stru_header data_header[];//not point,must array
extern uint8_t curr_index;//当前正在写的信息头索引

extern stru_para sys_para;
extern RTC_HandleTypeDef hrtc;


//当前模式
extern volatile uint8_t current_mode;//normal=0,sport=1,sleep=2

extern volatile uint8_t set_mode_flag;//set mode flag

extern osMutexId  flash_mutex;

//记录版本信息
static uint8_t * serian_no=(uint8_t *)(0x1FFF7A10);//硬件序列号

//1.0
static uint8_t main_ver=1;//主版本号
static uint8_t sec_ver=0;//副版本号

static uint8_t batch_data_type=0;
static uint8_t batch_data_day=0;


//批量上传标志
uint8_t bat_upload=0;

//------------------------------------------------------
//计算校验和
uint8_t check_sum(uint8_t* data,uint8_t len)
{
	uint8_t temp=0;
	for(int i=0;i<len;i++)
		temp+=data[i];
	return temp;
}

//发送命令
//================================================================
//发送确认命令
//fe 05 01 01 ab b0 ；fe 05 01 01 ae b3
//flag=1 发送成功确认，0-发送失败确认
void send_shakehand(uint8_t flag)
{
	buffer[0]=0xfe;
	buffer[1]=0x05;
	buffer[2]=0x01;//command
	buffer[3]=0x01;
	if(flag==1)
	{
		buffer[4]=0xab;
		buffer[5]=0xb0;
	}
	else
	{
		buffer[4]=0xae;
		buffer[5]=0xb3;
	}
	uart2_send(buffer,6);
}
//================================================================
//返回版本信息，
void send_version_info()
{
	uint8_t i=0;
	buffer[0]=0xfe;
	buffer[1]=0x05;
	buffer[2]=0x05;//command
	buffer[3]=0x0e;
	for(;i<12;i++)
		buffer[4+i]=serian_no[i];
	buffer[4+i]=main_ver;
	buffer[5+i]=sec_ver;
	buffer[6+i]=check_sum(buffer,6+i);
	uart2_send(buffer,7+i);

}
//---------------------------------------------------
//传输传感器数据
void send_sensor_data()
{
	uint8_t * temp=0;
	uint8_t i=0;
	buffer[0]=0xfe;
	buffer[1]=0x05;
	buffer[2]=0x06;//command
	buffer[3]=0x05;
	if(current_mode==2)//sleep
		temp=(uint8_t *)&(current_sensor_data.sleep_status);
	else//sport data
		temp=(uint8_t *)&(data_header[curr_index].total_step_data);
	memcpy(&buffer[4],temp,3);
	buffer[7]=current_sensor_data.hrs_rate&0xff;
	buffer[8]=batt_status;
	buffer[9]=check_sum(buffer,9);
	uart2_send(buffer,10);
}
//-----------------------------------------------------------
//read current mode
void send_mode()
{
	buffer[0]=0xfe;
	buffer[1]=0x05;
	buffer[2]=0x0a;//command
	buffer[3]=0x01;
	buffer[4]=current_mode;
	buffer[5]=check_sum(buffer,5);
	uart2_send(buffer,6);
	bat_upload=0;
}
//----------------------------------------------------------
void phone_nitofy_data(uint8_t flag)
{
	//dispatch phone notify and vib start
	switch(flag)
	{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
	};
}
//----------------------------------------
//send one day data,by sortno
void send_day_data(uint8_t type,uint8_t read_day,uint16_t sortno)
{
	uint32_t i=0;
	//uint16_t sortno=0;
	uint8_t data_index=0;
	stru_region * data_region=NULL;
	buffer[0]=0xfe;
	buffer[1]=0x05;
	buffer[2]=0x07;//command
	buffer[3]=0x0e;
  
	if(curr_index<read_day)
		data_index=14-(read_day-curr_index);
	else
		data_index=curr_index-read_day;
	//for(i=0;i<24*60;i++)
	{
		osMutexWait(flash_mutex, osWaitForever);

		//sortno++;
		memcpy(&buffer[4],(uint8_t *)&sortno,2);
		data_region=(stru_region *)(data_header[data_index].start_add+( (sortno-1)*(3*FLASH_UNIT_LENGTH) ));
		for(uint8_t j=0;j<3;j++)
		{

			if(type==0xa1)
			{
				memcpy(&buffer[6+j*4],(uint8_t *)&(data_region->step_count),2);
				if(data_region->step_count!=0xffff)
					buffer[8+j*4]=0x0;
				else
					buffer[8+j*4]=0xff;
				
				SEGGER_RTT_printf(0,"address=%x;sortno=%x;step=%d\r\n",data_region,sortno,data_region->step_count);

			}
			else if(type==0xa2)
				memcpy(&buffer[6+j*4],(uint8_t *)&(data_region->sleep_status),3);
			buffer[9+j*4]=data_region->hrs_rate&0xff;
			data_region=(stru_region *)((uint32_t)data_region+FLASH_UNIT_LENGTH);
		}
		buffer[4+4*3+2]=check_sum(buffer,4+4*3+2);
		uart2_send(buffer,4+4*4+2+1);
		osMutexRelease(flash_mutex);

		//HAL_Delay(1);
		//i+=2;

	}
}

//===============================================================
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
//处理接收到的数据
//return:1-收到命令正确；0-收到命令错误
void rece_dispatch()
{
	uint8_t len=buffer[0],command=0;
	uint8_t error=0;
	uint16_t temp_sortno=0;
	if(buffer[1]!=0xfe || buffer[2]!=0x04 || (buffer[len]!=check_sum(&buffer[1],len-1)) )
	{
		//收到错误命令回复
		send_shakehand(0);
	}
	command=buffer[3];
	//解析命令
	switch(buffer[3])
	{
		case 0x01://确认命令
			break;
		case 0x02://初始化flash存储区
			flash_format();
			send_shakehand(1);
			break;
		case 0x03://收到时间设置命令
			RTC_Set_datetime(&buffer[5]);
			RTC_AlarmConfig(buffer[8],buffer[9]+1);
			//SEGGER_RTT_printf(0,"set_time=%02d-%02d-%02d;%02d-%02d-%02d;\r\n",buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10]);
			send_shakehand(1);
			break;
		case 0x04://step length
			sys_para.step_len=buffer[5];
			sys_para.height=*(uint16_t *)(&buffer[6]);
			send_shakehand(1);
			//flash_erase_para_sector();
			break;
		case 0x05://return serial number
			send_version_info();		
			break;
		case 0x06://read now  sports data
			send_sensor_data();
			break;
		case 0x07://send one day data
			batch_data_type=buffer[5];
		  batch_data_day=buffer[6];
			send_day_data(buffer[5],buffer[6],1);
			break;
		case 0x09://phone notify info
			phone_nitofy_data(buffer[5]);
			send_shakehand(1);
			break;
		case 0x0a://read current mode
			send_mode();
			break;
		case 0x0b:
			if(buffer[5]>0x02)//first data=len,so sort is index+1
				error=1;
			else
			{	
				current_mode=buffer[5];
				set_mode_flag=1;
				send_message(5);
			}
			send_shakehand(1);
			break;
		case 0xab://batch data transmit
			//judge length,now day data length=1440/3=480
		  temp_sortno=(buffer[6]<<8)|buffer[5];
		  if(temp_sortno<480)
				send_day_data(batch_data_type,batch_data_day,temp_sortno+1);
			else
				break;
			break;
		case 0x20://update
			//jump bootloader
			HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, 0xabcd);
		  HAL_NVIC_SystemReset();
			while(1);
			break;
		default:
			send_shakehand(0);//收到异常命令S
			return;
	};
		//SEGGER_RTT_printf(0,"rece comm=%x;\r\n",buffer[3]);

	//if(command!=0x01 && command!=0x7 && command!=0xab)
	{
		//回复确认收到
		//if(error==0)
		//	send_shakehand(1);
		//else
		if(error==1)
			send_shakehand(0);
	}

}