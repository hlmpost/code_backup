#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "eric_rtc.h"

static osMutexId  rtc_mutex;

extern RTC_HandleTypeDef hrtc;

volatile uint8_t current_datetime[7]={16,1,1,10,10,10,1};//current datetime,first 6 member date time;last member week
extern volatile uint8_t rtc_flag;//rtc alarm flag
extern volatile uint8_t sensor_flag;
extern volatile uint8_t disp_sort;//display lcd sortno,0-time ,1-step,2-hrs,3-consume

//----------------------------------------------------
//cal week day
uint8_t CaculateWeekDay(uint16_t y,uint8_t m, uint8_t d)
{
	uint8_t week;
	y+=2000;
  if(m==1||m==2) {
  m+=12;
  y--;
  }
  week=(d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7;
	return week+1;

}
//----------------------------------------------------
//check status
void read_rtc_status()
{
			while(HAL_RTC_GetState(&hrtc)!=1);

}
//-------------------------------------------------------
//crycle set min alarm,now 1 min
void RTC_AlarmConfig(uint8_t hour,uint8_t min)
{
  static RTC_AlarmTypeDef sAlarm;
	HAL_RTC_DeactivateAlarm(&hrtc,RTC_ALARM_A);
	read_rtc_status();
	sAlarm.AlarmTime.Hours = hour;
  sAlarm.AlarmTime.Minutes = min;
  sAlarm.AlarmTime.Seconds = 1;
  sAlarm.AlarmTime.SubSeconds = 0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 1;
  sAlarm.Alarm = RTC_ALARM_A;
  HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, FORMAT_BIN);


  
}
//-------------------------------------------------------------------
//read date time
void RTC_Read_datetime(uint8_t * data,uint8_t flag)
{
	uint8_t temp[3];
	//first read tiem ,then read date, or not time is not run;
	RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;
	HAL_RTCStateTypeDef status;

	if(data!=NULL)
	{	
		osMutexWait(rtc_mutex, osWaitForever);
		/* Get the RTC current Time */
		HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
		temp[0]=stimestructureget.Hours;
		temp[1]=stimestructureget.Minutes;
		temp[2]=stimestructureget.Seconds;
		
		memcpy(&current_datetime[3],temp,3);
		
		/* Get the RTC current Date */
		HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
		data[0]=sdatestructureget.Year;
		data[1]=sdatestructureget.Month;
		data[2]=sdatestructureget.Date;
		current_datetime[6]=sdatestructureget.WeekDay;

		memcpy(&current_datetime[0],data,3);

		if(flag==1)
		{
			memcpy(data,temp,3);
		}
		osMutexRelease(rtc_mutex);
	}
}
//------------------------------------------------------------------
void RTC_Set_datetime(uint8_t * data)
{
	uint8_t temp[3];
	RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;
	if(data!=NULL)
	{
		osMutexWait(rtc_mutex, osWaitForever);

			sdatestructure.Year =data[0];
			sdatestructure.Month = data[1];
			sdatestructure.Date = data[2];
		  sdatestructure.WeekDay= CaculateWeekDay(data[0],data[1],data[2]);

			if(HAL_RTC_SetDate(&hrtc,&sdatestructure,RTC_FORMAT_BIN) != HAL_OK)
			{
				/* Initialization Error */
				Error_Handler("set time error"); 
			} 
			stimestructure.Hours = data[3];
			stimestructure.Minutes = data[4];
			stimestructure.Seconds = data[5];
			stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
			stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
			stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
			if(HAL_RTC_SetTime(&hrtc,&stimestructure,RTC_FORMAT_BIN) != HAL_OK)
				{
					/* Initialization Error */
					Error_Handler("set date error"); 
				}	
#if 0				
					//flash lcd
		HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
		temp[0]=stimestructure.Hours;
		temp[1]=stimestructure.Minutes;
		temp[2]=stimestructure.Seconds;
		
		memcpy(&current_datetime[3],temp,3);
		
		/* Get the RTC current Date */
		HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
		data[0]=sdatestructure.Year;
		data[1]=sdatestructure.Month;
		data[2]=sdatestructure.Date;
		current_datetime[6]=sdatestructure.WeekDay;
		memcpy(&current_datetime[0],data,3);
#endif
		if(disp_sort==0)
				send_message(4);

	  osMutexRelease(rtc_mutex);

	}//data=null
}
//----------------------------------------------------------------
//alarm callback
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  	Info_Handler("alarm");
		rtc_flag=1;
		send_message(2);

}
//------------------------------------------------------------
//wake up control
void rtc_wakeup(uint8_t flag)
{
	if(flag==0)
		HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
	else
		HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 10240, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

}
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef * hrtc)
{
	if(sensor_flag==0)
	{
		send_message(2);
	}
}

//-----------------------------------------------------------
void rtc_init()
{
  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, 0xaaaa);
	osMutexDef(rtc_mutex); 
	rtc_mutex = osMutexCreate(osMutex(rtc_mutex));
}

