//ÐÄÂÊ¶ÁÈ¡
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"

#include "power.h"

extern volatile uint8_t batt_status;//current battery percent
static uint16_t  batt_max=0x5b2,batt_min=0x47a;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
  /* Get the converted value of regular channel */
//  uint32_t temp;
//  temp= HAL_ADC_GetValue(AdcHandle);
//	if(temp<batt_min)
//		temp=batt_min;
//	if(temp>batt_max)
//		temp=batt_max;
//	batt_status=(temp-batt_min)*100/(batt_max-batt_min);
//  SEGGER_RTT_printf(0,"power:%d\r\n",batt_status);			

}


//------------------------------------------------------
uint8_t read_batt()
{ uint32_t temp[3];
	//HAL_ADC_Start_IT(&hadc1);
	for(uint8_t i=0;i<3;i++)
	{
		HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK)
			return 0;
		if ((HAL_ADC_GetState(&hadc1) & HAL_ADC_STATE_EOC_REG) == HAL_ADC_STATE_EOC_REG)
		{
			temp[i] = HAL_ADC_GetValue(&hadc1);
				if(temp[i]<batt_min)
					temp[i]=batt_min;
				if(temp[i]>batt_max)
					temp[i]=batt_max;
		}
		HAL_ADC_Stop(&hadc1);
	}
	temp[0]=(temp[0]+temp[1]+temp[2])/3;
	batt_status=(temp[0]-batt_min)*100/(batt_max-batt_min);
	SEGGER_RTT_printf(0,"power:%d\r\n",batt_status);			

	return 1;

}
//------------------------------------------------------
unsigned char power_init()
{
	//read_batt();

}
//------------------------------------------
