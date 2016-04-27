//通知，提示接口
//目前是led灯，振动
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "tim.h"



//------------------------------------------------
//控制闪烁的频率
//flag=3 3s;2 2s;1 1s;4 0.5s
void led_flash(uint8_t flag)
{
	uint16_t period=0;
	uint32_t uhPrescalerValue = (uint32_t) ((SystemCoreClock /2) / 2000) - 1;

	switch(flag)
	{
		case 1:
			period=2000-1;
			break;
		case 2:
			period=4000-1;
			break;
		case 3:
			period=6000-1;
			break;
		case 4:
			period=1000-1;
			break;
	};
	HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);
	TIM_OC_InitTypeDef sConfigOC;
	//HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
	//改变频率
	htim3.Instance = TIM3;
  htim3.Init.Prescaler = uhPrescalerValue;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = period;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim3);
	
//	sConfigOC.OCMode = TIM_OCMODE_PWM1;
//  sConfigOC.Pulse = pulse;
//  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
//  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
//  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
	
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	
}