/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"


/* USER CODE BEGIN 0 */
#include "cmsis_os.h"

volatile uint8_t current_datetime[7]={16,1,1,10,10,10,1};//current datetime,first 6 member date time;last member week
extern volatile uint8_t rtc_flag;//rtc alarm flag
extern volatile uint8_t sensor_flag;
//mutex
osMutexId  rtc_mutex;


/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  RTC_AlarmTypeDef sAlarm;

    /**Initialize RTC and set the Time and Date 
    */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  HAL_RTC_Init(&hrtc);

  sTime.Hours = 16;
  sTime.Minutes = 52;
  sTime.Seconds = 30;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BIN);

  sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
  sDate.Month = RTC_MONTH_APRIL;
  sDate.Date = 20;
  sDate.Year = 16;

  HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BIN);

    /**Enable the Alarm A 
    */
  sAlarm.AlarmTime.Hours = 16;
  sAlarm.AlarmTime.Minutes = 52;
  sAlarm.AlarmTime.Seconds = 40;
  sAlarm.AlarmTime.SubSeconds = 0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 1;
  sAlarm.Alarm = RTC_ALARM_A;
  HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, FORMAT_BIN);

    /**Enable the WakeUp 
    */
  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 10240, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
	
	/* USER CODE BEGIN 10 */
  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
	/* USER CODE END 10 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{

  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{

  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(RTC_WKUP_IRQn);

    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);

  }
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */
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

//osMutexWait(rtc_mutex, osWaitForever);
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
		osMutexRelease(rtc_mutex);

		if(flag==1)
		{
			memcpy(data,temp,3);
		}
	}
	//osMutexRelease(rtc_mutex);
}
//------------------------------------------------------------------
void RTC_Set_datetime(uint8_t * data)
{
	RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;
	if(data!=NULL)
	{
		osMutexWait(rtc_mutex, osWaitForever);

			sdatestructure.Year =data[0];
			sdatestructure.Month = data[1];
			sdatestructure.Date = data[2];

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
	osMutexDef(rtc_mutex); 
	rtc_mutex = osMutexCreate(osMutex(rtc_mutex));

}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
