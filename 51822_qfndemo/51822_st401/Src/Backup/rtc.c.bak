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
static uint8_t rtc_check();
static void rtc_backup_init();
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
	
/* USER CODE BEGIN*/
	rtc_backup_init();
  if(rtc_check()==0)
	{
		sTime.Hours = 16;
		sTime.Minutes = 52;
		sTime.Seconds = 50;
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
		sAlarm.AlarmTime.Minutes = 53;
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
/* USER CODE END*/
    /**Enable the WakeUp 
    */
  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 10240, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

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
static void rtc_backup_init()
{
	uint32_t temp;
	//__HAL_RCC_BKP_CLK_ENABLE();//for backup sram
  /* Allow access to Backup */
  HAL_PWR_EnableBkUpAccess();
}
static uint8_t rtc_check()
{
	uint8_t hour,min;
	RTC_AlarmTypeDef sAlarm;
	RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;
	if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2)
	{	
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
		return 0;
	}
	else
	{
		//read time
		HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);		
		HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
		//set alarm
		    /**Enable the Alarm A 
			*/
			hour=stimestructureget.Hours;
			min=stimestructureget.Minutes+1;
			if(min>59)
			{	
				min-=60;
				if(hour==23)
					hour=0;
				else
					hour++;
			}

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

		return 1;
	}

}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
