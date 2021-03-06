/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
//for debug
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "stm32f4xx_hal.h"

#include "eric_flash.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;

/* USER CODE BEGIN Variables */
extern RTC_HandleTypeDef hrtc;
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim3;


extern stru_para sys_para;


//thread
osThreadId Lcd_disp_TaskHandle;//display
osThreadId touch_TaskHandle;//touch
osThreadId sensor_TaskHandle;//touch
osThreadId uart_TaskHandle;//uart
osThreadId button_TaskHandle;//button
osThreadId wake_TaskHandle;//wake


void func_DispTask(void const * argument);
void func_touchTask(void const * argument);
void func_sensorTask(void const * argument);
void func_uartTask(void const * argument);
void func_buttonTask(void const * argument);
void func_wakeTask(void const * argument);

volatile uint8_t init_finish=0;


extern volatile uint8_t current_datetime[];//current datetime


volatile uint8_t sensor_flag=0;
volatile uint8_t current_mode=0;//normal=0,sport=1,sleep=2
volatile uint8_t batt_status=50;//current battery percent
volatile uint8_t disp_sort=0;//display lcd sortno,0-time ,1-step,2-hrs,3-consume
//volatile uint8_t step_len=50;//step length

volatile uint8_t rtc_flag=0;//rtc alarm flag

volatile uint8_t disp_flag=0;//displaying lcd,don't do other

volatile uint8_t button_flag=0;//button busy flag

volatile uint8_t set_mode_flag=0;//set mode flag

//timer
static osTimerId sport_timer_id;
//static void Timer_Callback  (void const *arg);



stru_region current_sensor_data;
extern stru_header data_header[];
extern uint8_t curr_index;//当前正在写的信息头索引

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
//---------------------------------
//error print
void Error_Handler(char * err)
{
	SEGGER_RTT_printf(0,"error:%s\r\n",err);
}
//---------------------------------------
void Info_Handler(char * info)
{
	SEGGER_RTT_printf(0,"info:%s\r\n",info);
}
//------------------------------------------------
//send message func
void send_message(uint8_t message)
{
	if(init_finish<1)//must
		return;
	if(message==1)//touch
		osSignalSet(touch_TaskHandle, message);
	else if(message==2)//sensor
		osSignalSet(sensor_TaskHandle, message);
	else if(message==3)//uart
		osSignalSet(uart_TaskHandle, message);
	else if(message==4)//display
		osSignalSet(Lcd_disp_TaskHandle, message);
	else if(message==5)//button
		osSignalSet(button_TaskHandle, message);
	else if(message==6)//wake up
		osSignalSet(wake_TaskHandle, message);
}
//---------------------------------------------------------
void eric_sleep()
{
	  GPIO_InitTypeDef GPIO_InitStruct;
	
		//HAL_I2C_MspDeInit(&hi2c1);
		//HAL_I2C_MspDeInit(&hi2c2);
		HAL_SPI_MspDeInit(&hspi1);
		//HAL_TIM_PWM_MspDeInit(&htim3);
	
		//HAL_UART_MspDeInit(&huart2);
		//uart
//		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//		GPIO_InitStruct.Pull = GPIO_PULLUP;
//		GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
//		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
//	//i2c
//		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//		GPIO_InitStruct.Pull = GPIO_NOPULL;
//		GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_3;
//		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 


		return;


}

void eric_wakeup()
{
	  GPIO_InitTypeDef GPIO_InitStruct;
	
		//HAL_I2C_MspInit(&hi2c1);
		//HAL_I2C_MspInit(&hi2c2);
		HAL_SPI_MspInit(&hspi1);
		//HAL_TIM_PWM_MspInit(&htim3);
		//HAL_TIM_MspPostInit(&htim3);
			//HAL_UART_MspInit(&huart2);
	//uart
//			GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
//			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//			GPIO_InitStruct.Pull = GPIO_PULLUP;
//			GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//			GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
//			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//	//i2c
//		GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
//    GPIO_InitStruct.Pull = GPIO_PULLUP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//		
//		GPIO_InitStruct.Pin = GPIO_PIN_10;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
//    GPIO_InitStruct.Pull = GPIO_PULLUP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//    GPIO_InitStruct.Pin = GPIO_PIN_3;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
//    GPIO_InitStruct.Pull = GPIO_PULLUP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF9_I2C2;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);



		return;	
}

/* USER CODE END FunctionPrototypes */

/* Pre/Post sleep processing prototypes */
void PreSleepProcessing(uint32_t *ulExpectedIdleTime);
void PostSleepProcessing(uint32_t *ulExpectedIdleTime);

/* Hook prototypes */

/* USER CODE BEGIN PREPOSTSLEEP */
void PreSleepProcessing(uint32_t *ulExpectedIdleTime)
{
		uint32_t temp=0,i=0;

  GPIO_InitTypeDef GPIO_InitStruct;

	if(init_finish<1)
		return;
		eric_sleep();

/* place for user code */ 
	osThreadSuspendAll();
	//------------------------------
			//SEGGER_RTT_printf(0,"PreSleepProcessing\r\n");	
//----------------------------------
#if 0
	HAL_I2C_MspDeInit(&hi2c1);
	HAL_I2C_MspDeInit(&hi2c2);
	HAL_SPI_MspDeInit(&hspi1);
	//HAL_UART_MspDeInit(&huart2);
	HAL_TIM_PWM_MspDeInit(&htim3);
	//gpio deinit
	//adc
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
	//uart
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
	//i2c1,2
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 
	//spi
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 
	
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	//other gpa
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Pin =GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 

	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
	//GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
	
	//other gpb
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 
	
//	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_15;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
	
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13, GPIO_PIN_RESET);

	//gpiob
//	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif

//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Pin = GPIO_PIN_6;
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
//----------------------------------
	__GPIOC_CLK_DISABLE();
  __GPIOA_CLK_DISABLE();
  __GPIOB_CLK_DISABLE();

	HAL_PWREx_EnableFlashPowerDown();
	  
	

  /* Set SLEEPDEEP bit of Cortex System Control Register */
	MODIFY_REG(PWR->CR, (PWR_CR_PDDS | PWR_CR_LPDS), PWR_LOWPOWERREGULATOR_ON);
	//SET_BIT(PWR->CR, PWR_CR_PDDS);
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));


}

void PostSleepProcessing(uint32_t *ulExpectedIdleTime)
{
	uint32_t temp=0,i=0;
	GPIO_InitTypeDef GPIO_InitStruct;
	if(init_finish<1)
		return;

	 CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));  
	

/* place for user code */
  SystemClock_Config();
	

	
	__GPIOC_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();


	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
		osThreadResumeAll();
	


#if 0
//	for(i=23;i<39;i++)
//	{
//		temp=HAL_NVIC_GetPendingIRQ(i);
//		if(temp!=0)
//			break;
//	}
//	temp=0;
//	for(i=40;i<43;i++)
//	{
//		temp=HAL_NVIC_GetPendingIRQ(i);
//		if(temp!=0)
//			break;
//	}
//	temp=0;
//	for(i=60;i<74;i++)
//	{
//		temp=HAL_NVIC_GetPendingIRQ(i);
//		if(temp!=0)
//			break;
//	}

	//wakeup-----------------------------
		//gpio init
	 /*Configure GPIO pins : PA1 PA9 PA10 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA7 PA8 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB14 PB15 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13, GPIO_PIN_SET);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

		//gpio init end
	HAL_I2C_MspInit(&hi2c1);
	HAL_I2C_MspInit(&hi2c2);
	HAL_SPI_MspInit(&hspi1);
	//HAL_UART_MspInit(&huart2);
	HAL_TIM_PWM_MspInit(&htim3);
	HAL_TIM_MspPostInit(&htim3);
	#endif

	//------------------------------
	eric_wakeup();

	
//		wake_count++;
		//SEGGER_RTT_printf(0,"PostSleepProcessing\r\n");	
//	if(wake_count>250)
//		wake_count=1;
}
/* USER CODE END PREPOSTSLEEP */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */

		//rtc
	rtc_init();

  //测试 as7000
	//as7000_init();
  //测试lis2ds
  if(lis2ds12_init()==0)
		Error_Handler("g-sensor is error!");
	else
		Info_Handler("g-sensor is ok!");		
	
	//Loop_Test_Tap();
	Loop_Test_Pedometer();
	
	
	if(iqs263_init()==0)
		Error_Handler("tp is error!");
	else
		Info_Handler("tp is ok!");
	
	
	//flash init
	flash_init();

	//display thread
	osThreadDef(disp_Task, func_DispTask, osPriorityNormal, 0, 128);
	Lcd_disp_TaskHandle = osThreadCreate(osThread(disp_Task), NULL);
	//touch thread
	osThreadDef(touch_tTask, func_touchTask, osPriorityNormal, 0, 128);
  touch_TaskHandle = osThreadCreate(osThread(touch_tTask), NULL);
	//sensor thread
	osThreadDef(sensor_tTask, func_sensorTask, osPriorityNormal, 0, 128);
  sensor_TaskHandle = osThreadCreate(osThread(sensor_tTask), NULL);
	//uart thread
	osThreadDef(uart_tTask, func_uartTask, osPriorityAboveNormal, 0, 128);//higher prior
  uart_TaskHandle = osThreadCreate(osThread(uart_tTask), NULL);
	//button thread
	osThreadDef(button_tTask, func_buttonTask, osPriorityNormal, 0, 128);
  button_TaskHandle = osThreadCreate(osThread(button_tTask), NULL);
	//wake thread
	osThreadDef(wake_tTask, func_wakeTask, osPriorityNormal, 0, 128);
  wake_TaskHandle = osThreadCreate(osThread(wake_tTask), NULL);


	init_finish=1;//init finish
	osThreadTerminate(defaultTaskHandle);
  /* Infinite loop */
  for(;;)
  {
		//power_init();
    	osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application */

//-----------------------------------------------------------------------
//lcd disp thread
void func_DispTask(void const * argument)
{
	char data[10];
	uint8_t count=0;//10 times,gc4 update
	uint8_t old_batt_status=0;
	uint8_t curr_time[3];
			//lcd init
	lcd_init();

	//display lcd default datetime
	send_message(4);
	while(1)
	{
		osSignalWait(0x4, osWaitForever);
		disp_flag=1;
		#if 1
		//test
		disp_sort=1;
		read_batt();
		RTC_Read_datetime(curr_time,1);

		lcd_display_on(1);
		lcd_disp_clear();
		if(disp_sort==0)
		{
			//lcd_disp_bmp(1,80);
			//lcd_disp_font(1,170,1,29,29);
			//lcd_disp_number(10,200,data,33,12);
			sprintf(data,"%02d%02d",current_datetime[3],current_datetime[4]);
			lcd_disp_time(1,40,data,80,40);
			sprintf(data,"%02d%02d%d",current_datetime[1],current_datetime[2],current_datetime[6]);
			lcd_disp_date(1,205,data,29,15);
		}
		else if(disp_sort==1)//步数
		{
			lcd_disp_bmp(1,40,1);
			lcd_disp_font(1,130,1,2,29,29);
			sprintf(data,"%d",data_header[curr_index].total_step_data);
			lcd_disp_number(10,160,data,29,12);
			lcd_disp_font(1,190,2,2,29,29);
			sprintf(data,"%.1f",((float)data_header[curr_index].total_step_data*sys_para.step_len)/100);
			lcd_disp_number(10,220,data,29,12);
			lcd_disp_font(1,250,3,1,24,24);
			
		}
		else if(disp_sort==2)//consume
		{//热量(kcal)=体重(kg)x距离(km)x1.036  ,default 60kg
			lcd_disp_bmp(1,40,2);
			lcd_disp_font(1,140,4,3,29,29);
			sprintf(data,"%.1f",(  ((float)data_header[curr_index].total_step_data*sys_para.step_len)/100000  )*60*1.036);
			SEGGER_RTT_printf(0,"sensor data=%s;step=%d!\r\n",data,data_header[curr_index].total_step_data);	

			lcd_disp_number(10,170,data,29,12);
			
		}
		else if(disp_sort==3)//hrs rate
		{
			lcd_disp_bmp(1,40,3);
			lcd_disp_font(1,140,5,2,29,29);
			sprintf(data,"%d",current_sensor_data.step_count);
			lcd_disp_number(10,170,data,29,12);
			
		}
		//current_mode
		sprintf(data,"%d",current_mode);
		lcd_disp_number(1,1,data,29,12);
		lcd_disp_battery(batt_status);

		count++;
		if(count>9)
		{	
			count=0;
			lcd_display_update(1);
		}
		else
			lcd_display_update(2);

		lcd_display_on(0);
	  #endif
		disp_flag=0;
	}
}
//-------------------------------------------------------------------
//touch thread
void func_touchTask(void const * argument)
{
	uint8_t temp=0,old_disp_sort=0;
	while(1)
	{
		osSignalWait(0x1, osWaitForever);
		//test
		//continue;
		
		temp=handleEvents();
		if(disp_flag==1)
			continue;
		old_disp_sort=disp_sort;
		if(temp==2)//left
		{
			if(disp_sort>0)
				disp_sort--;
		}
		else if(temp==3)//right
		{
			if(disp_sort<3)
				disp_sort++;
		}
		//if(disp_sort!=old_disp_sort)
		//	send_message(4);

	}

}
//------------------------------------------------------------------
//sensor thread
void func_sensorTask(void const * argument)
{
	uint8_t curr_time[3],hour,min;
	uint8_t flag=4;
	uint16_t step_count_1min=0,current_step_count=0;
  uint32_t g_buffer[3];

	while(1)
	{
		osSignalWait(0x2, osWaitForever);
		
		//test
//		rtc_flag=0;
//		sensor_flag=0;
//		continue;
		
		sensor_flag=1;
		//osDelay(2000);
		//step
		if(current_mode!=2)
		{	
			LIS2DS12_ACC_GYRO_Pedo_Callback();
			current_step_count=current_sensor_data.step_count;
			data_header[curr_index].total_step_data+=current_step_count;
			step_count_1min+=current_step_count;
		}
		else
		{
				LIS2DS12_ACC_sample_Callback(g_buffer);
			  current_sensor_data.sleep_status=g_buffer[0];
				SEGGER_RTT_printf(0,"sensor data=%x,%x,%x!\r\n",g_buffer[0],g_buffer[1],g_buffer[2]);	
		}
		//hrs
		//current_sensor_data.hrs_rate=as7000_hrs_rate();

//		if(current_mode==2)//sleep mode
//			current_sensor_data.step_count=0xffff;
//		else
//		{
//			current_sensor_data.sleep_status=0xffffffff;
//			data_header[curr_index].curr_step_data+=current_sensor_data.step_count;
//			step_count_1min+=current_sensor_data.step_count;
//		}
		
		current_sensor_data.bld_press=0xffff;//now we have not blood pressure
		if(rtc_flag==1)
		{
			current_sensor_data.step_count=step_count_1min;
			flash_write_movedata(&current_sensor_data,current_mode);
			step_count_1min=0;
		}
//				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
//				osDelay(2000);
//				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
//				led_flash(flag);
//				flag--;
//		    if(flag<1)
//					flag=4;
		//sprintf(data,"%d",current_sensor_data.step_count);
		//if rtc_flag=1 force flash lcd for display status
		if(current_mode!=2 && current_step_count==0 && disp_sort==1 && rtc_flag==0)//no move,in step lcd,no flash
		{	
			current_step_count=0;
			//test
			//send_message(4);
		}
		else if(rtc_flag==0 && disp_sort==0)
		{
		}
		else
		{
			if(disp_flag==0)
					send_message(4);
		}
		
		if(rtc_flag==1)
			rtc_flag=0;
		sensor_flag=0;
		

	}
}
//----------------------------------------------------------------------
//uart thread
void func_uartTask(void const * argument)
{
	uart2_read(1);
	while(1)
	{
		osSignalWait(0x3, osWaitForever);
		//send_sensor_data();
		//send_shakehand(1);
		rece_dispatch();
	}
}
//---------------------------------------------------------------
//static void Timer_Callback  (void const *arg)
//{
//	if(sensor_flag==0)
//			send_message(2);
//}
//----------------------------------------------------------------------
//button thread
void func_buttonTask(void const * argument)
{
  uint8_t i=0;
  GPIO_InitTypeDef GPIO_InitStruct;
	while(1)
	{
		osSignalWait(0x5, osWaitForever);
		//test
//		button_flag=0;
//		set_mode_flag=0;
//		continue;
		
		if(set_mode_flag==1)
		{
			if(current_mode==1)//sport mode
			{
				rtc_wakeup(1);
			}
			else
			{
				rtc_wakeup(0);
			}
			send_message(4);
		}
		else if(button_flag==1)
		{
			
			//interrupt to gpio
			GPIO_InitStruct.Pin = GPIO_PIN_9;
			GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

					//judge button valid
					for(i=0;i<10;i++)
					{	
						if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)==GPIO_PIN_SET)
							break;
						//osDelay(15);
							HAL_Delay(10);
					}
					if(i==10)//button is valid
					{
						if(current_mode<2)
							current_mode++;
						else
							current_mode=0;

						if(current_mode==1)//sport mode
						{
							//5s read sensor a time
							//osTimerStart(sport_timer_id, 5000);
							rtc_wakeup(1);
						}
						else
						{
							 //osTimerStop(sport_timer_id);
							rtc_wakeup(0);
						}
						GPIO_InitStruct.Pin = GPIO_PIN_9;
						GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
						GPIO_InitStruct.Pull = GPIO_PULLUP;
						HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
						send_message(4);
				 }
			}
		
		SEGGER_RTT_printf(0,"current_mode=%d!\r\n",current_mode);	
		button_flag=0;
		set_mode_flag=0;
	}
}
//----------------------------------------------
//uart wakeup thread
void func_wakeTask(void const * argument)
{
  
	while(1)
	{
		osSignalWait(0x6, osWaitForever);
		while(1)
		{	
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1)==GPIO_PIN_SET)
				break;
			HAL_Delay(5);
		}

	
	}
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
