/* -----------------------------------------------------------------------------
** Copyright (C)2015 E Ink Holdings Inc. All rights reserved.
** -----------------------------------------------------------------------------
** Customer Name : Guangzhou Adsmart Technology Ltd 
** Contact window: Watwang
** Licensing date: Feb/24/2016
** -------------------------------------------------------------------------- */
  
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "spi.h"
#include "gpio.h"

#include "EPD_TCON.h"

static GPIO_InitTypeDef  GPIO_InitStruct;

static uint32_t _tim_clk_T = 11904;


#define TIMER_CLOCK_KHZ   84000 //st401


//----------------------------------------------------------------------
//help function list
//-----------------------------------------------------------------------------
//io口操作
static void gpio_init()
{
	/*cs_pin=pa15;o
		oei_pin=pa4;o
		poweron_pin=pa7;o
		reset_pin=pb1;o
		busy_pin=pb7;i
	*/
	/*
	GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
	
	GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
	
	GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 
	
	GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 
*/
}
//----------------------------------------------------------------------
void STM_TIM_Init(void)
{
    //uint32_t tmpcr1 = 0;
	
    //TIM Clock
    //1. If the APB prescaler is 1, the timer clock frequencies are set to the same frequency as that of the APB domain to which the timers are connected.
    //2. Otherwise, they are set to twice (⊙2) the frequency of the APB domain to which the timers are connected.
    //Timer Clock = APB1 * 2

    _tim_clk_T = 1000000000 / TIMER_CLOCK_KHZ; //?衡timer㏄戳(ns)

    // timer2: up-counter
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 
    
    // Set timer prescaler (0 = no divide - clk = 84MHz) ;
    TIM2->PSC = 0;
    
    // Set reload register well above the longest time that we're interested in 
    TIM2->ARR = 0xFFFFFFFF; //tim2 & tim5 is 32bits
  
}

//-------------------------------------------------------------
//spi init
//设置不同的波特率
static void eric_spi_init(uint32_t baud_rate)
{
	HAL_SPI_DeInit(&hspi1);
	
	hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = baud_rate;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  hspi1.Init.CRCPolynomial = 10;
  HAL_SPI_Init(&hspi1);

}
//--------------------------------------------------------------------------------
static void downSpiSpeedForRead(void)
{

}
//----------------------------------------------------------------
static void UpSpiSpeedForWrite(void)
{
	
}

//-------------------------------------------------------------------
//interface function list
//------------------------------------------------------------
static void _Spi_SetClock(SPI_SPEED_TYPE speed) 
{
    if (speed == SPI_SPEED_LOW)
    {    
			eric_spi_init(SPI_BAUDRATEPRESCALER_16);
		}
    else 
		{
      eric_spi_init(SPI_BAUDRATEPRESCALER_2);
		}
}
//----------------------------------------------------------------
static void _Spi_CS_Enable(void)
{
		/*cs_pin=pa15;o
		oei_pin=pa4;o
		poweron_pin=pa7;o
		reset_pin=pb1;o
		busy_pin=pb7;i
	*/
	 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,GPIO_PIN_RESET);
}
static void _Spi_CS_Disable(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,GPIO_PIN_SET);
}
//-----------------------------------------------------------------------
static void _Spi_Write(const uint8_t *buf, int32_t len)
{
	HAL_SPI_Transmit(&hspi1,(uint8_t *)buf,len,100);
}
//--------------------------------------------------------------------------
static uint8_t _Spi_ReadByte(void) //
{
	uint8_t temp;
	if(HAL_SPI_Receive(&hspi1,&temp, 1, HAL_MAX_DELAY) != HAL_OK)
		Error_Handler("_Spi_ReadByte:read error\r\n");
	return temp;

}
//------------------------------------------------------------------------------
static void _SetResetPin(void)
{
		/*cs_pin=pa15;o
		oei_pin=pa4;o
		poweron_pin=pa7;o
		reset_pin=pb1;o
		busy_pin=pb7;i
	*/
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1,GPIO_PIN_SET);

}

static void _ClrResetPin(void)
{
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1,GPIO_PIN_RESET);
}
//--------------------------------------------------------
void _SetPowerOnPin(void)
{
		/*cs_pin=pa15;o
		oei_pin=pa4;o
		poweron_pin=pa7;o
		reset_pin=pb1;o
		busy_pin=pb7;i
	*/
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7,GPIO_PIN_SET);
}
static void _ClrPowerOnPin(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7,GPIO_PIN_RESET);
}
//-----------------------------------------------
static void _SetOEIPin(void)
{
		/*cs_pin=pa15;o
		oei_pin=pa4;o
		poweron_pin=pa7;o
		reset_pin=pb1;o
		busy_pin=pb7;i
	*/
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,GPIO_PIN_SET);
}

static void _ClrOEIPin(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,GPIO_PIN_RESET);
}
//------------------------------------------
static uint32_t _ReadBusyPin(void) 
{
		/*cs_pin=pa15;o
		oei_pin=pa4;o
		poweron_pin=pa7;o
		reset_pin=pb1;o
		busy_pin=pb7;i
	*/
	HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);
}
//------------------------------------------------------
static void _DelayMS(int32_t ms)
{
	//osDelay(ms);
	HAL_Delay(ms);
}
//-----------------------------------------------
static void _OnFrameStartEvent()
{
}

static void _StartTimer(uint32_t ns)
{
	TIM2->CR1 |=  TIM_CR1_DIR | TIM_CR1_OPM;
  TIM2->ARR = (ns * 1000) / _tim_clk_T;
  
  // Enable the timer 
  TIM2->CR1 |= TIM_CR1_CEN;  
}

static TIMER_STATE_TYPE _GetTimerState(void)
{
	  if (TIM2->CR1 & 0x01) 
        return TIMER_RUNNING;

    return TIMER_STOP;

}

void timer2_test(uint32_t ns)
{
	
	_StartTimer(ns);
	while(1)
	{
		if(_GetTimerState()==TIMER_RUNNING)
					SEGGER_RTT_printf(0,"TIMER_RUNNING\r\n");
	  else
		{		
			SEGGER_RTT_printf(0,"TIMER_STOP\r\n");		
			break;
		}
	}
}

//==================================================================
//HAL table ================================================
const EPD_TCON_DRIVER_HAL g_TCON_HAL = 
{
    _Spi_SetClock,  //void (*Spi_SetClock)(int32_t br); 
    _Spi_CS_Enable, //void (*Spi_CS_Enable)(void);
    _Spi_CS_Disable, //void (*Spi_CS_Disable)(void);    
    _Spi_Write,     //void (*Spi_Write)(const uint8_t *buf, int32_t len);
    _Spi_ReadByte,  //uint8_t (*Spi_ReadByte)();
    
    _SetResetPin,   //void (*SetResetPin)(void);
    _ClrResetPin,   //void (*ClrResetPin)(void);
    _SetPowerOnPin, //void (*SetPowerOnPin)(void);
    _ClrPowerOnPin, //void (*ClrPowerOnPin)(void);
    _SetOEIPin,     //void (*SetOEIPin)(void);
    _ClrOEIPin,     //void (*ClrOEIPin)(void);
    _ReadBusyPin,   //uint32_t (*ReadBusyPin)(void);
    _DelayMS,       //void (*DelayMS)(int32_t ms);
    _OnFrameStartEvent,
    _StartTimer,
    _GetTimerState,    
};

