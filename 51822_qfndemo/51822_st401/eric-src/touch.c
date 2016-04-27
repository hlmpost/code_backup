//touch sense
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "i2c.h"

#include "touch.h"

#define S_ADDRESS       (0x88)



static uint8_t data_buffer[10];

//------------------------------------------------------
//control rdy line
static void iqs263_rdy(uint8_t flag)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(flag==1)
	{
		GPIO_InitStruct.Pin = GPIO_PIN_0;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	}
	else
	{
		GPIO_InitStruct.Pin = GPIO_PIN_0;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	}
}
//-----------------------------------------------
static uint8_t iqs263_rdy_read()
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
}
//---------------------------------------------------
static unsigned char iqs263_WriteBytes(unsigned char* data,unsigned char len)
{
	while(HAL_I2C_Master_Transmit(&hi2c1, S_ADDRESS,data ,len, 100)!= HAL_OK)
  {
    /* Error_Handler() function is called when Timeout error occurs.
       When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler("i2c1 tx comm error!");
			return 1;
    }
  }
	return 0;
}
//---------------------------------------------
static unsigned char iqs263_ReadBytes(unsigned char* data,unsigned char len)
{
	while(HAL_I2C_Master_Receive(&hi2c1, S_ADDRESS, data, len, 100) != HAL_OK)
  {
    /* Error_Handler() function is called when Timeout error occurs.
       When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler("i2c1 rx comm error!");
			return 1;
    } 
  }
	return 0;
}
//-------------------------------------
static void IQS263DeviceSettings(void)
{
    // Switch the IQS263 into projection mode - if necessary
//		data_buffer[0] = SYS_FLAGS;
//    data_buffer[1] = 0x10;
//		iqs263_WriteBytes(data_buffer,2);
//		osDelay(10);
 // Setup prox settings
//    data_buffer[0] = PROX_SETTINGS;
//    data_buffer[1] = PROXSETTINGS0_VAL;
//    data_buffer[2] = PROXSETTINGS1_VAL;
//    data_buffer[3] = PROXSETTINGS2_VAL;
//    data_buffer[4] = PROXSETTINGS3_VAL;
//    data_buffer[5] = EVENT_MASK_VAL;
//		iqs263_WriteBytes(data_buffer,6);
//		osDelay(10);
	
    // Set active channels
		data_buffer[0] = ACTIVE_CHANNELS;
    data_buffer[1] = ACTIVE_CHS;
		iqs263_WriteBytes(data_buffer,2);
		//osDelay(10);
		HAL_Delay(10);
    // Setup touch and prox thresholds for each channel
		data_buffer[0] = THRESHOLDS;
    data_buffer[1] = PROX_THRESHOLD;
    data_buffer[2] = TOUCH_THRESHOLD_CH1;
    data_buffer[3] = TOUCH_THRESHOLD_CH2;
    data_buffer[4] = TOUCH_THRESHOLD_CH3;
    data_buffer[5] = MOVEMENT_THRESHOLD;
    data_buffer[6] = RESEED_BLOCK;
    data_buffer[7] = HALT_TIME;
    data_buffer[8] = I2C_TIMEOUT;
		iqs263_WriteBytes(data_buffer,9);
		//osDelay(10);
		HAL_Delay(10);
    // Set the ATI Targets (Target Counts)
		data_buffer[0] = TIMINGS_AND_TARGETS;  //for lp mode  if it is not 0
    data_buffer[1] = LOW_POWER;  //for lp mode  if it is not 0
    data_buffer[2] = ATI_TARGET_TOUCH;
    data_buffer[3] = ATI_TARGET_PROX;
		iqs263_WriteBytes(data_buffer,4);
		//osDelay(10);
		HAL_Delay(10);
//    // Set the BASE value for each channel
//    data_buffer[0] = MULTIPLIERS_CH0;
//    data_buffer[1] = MULTIPLIERS_CH1;
//    data_buffer[2] = MULTIPLIERS_CH2;
//    data_buffer[3] = MULTIPLIERS_CH3;
//    CommsIQSxxx_start();
//    CommsIQS_Write(IQS263_ADDR, MULTIPLIERS, &data_buffer[0], 4);
//    CommsIQSxxx_stop();

   

//    // Setup Compensation (PCC)
//    data_buffer[0] = COMPENSATION_CH0;
//    data_buffer[1] = COMPENSATION_CH1;
//    data_buffer[2] = COMPENSATION_CH2;
//    data_buffer[3] = COMPENSATION_CH3;
//    CommsIQSxxx_start();
//    CommsIQS_Write(IQS263_ADDR, COMPENSATION, &data_buffer[0], 4);
//    CommsIQSxxx_stop();


    // Set gesture timers on IQS263
		data_buffer[0] = GESTURE_TIMERS;
    data_buffer[1] = TAP_TIMER;
    data_buffer[2] = FLICK_TIMER;
    data_buffer[3] = FLICK_THRESHOLD;
		iqs263_WriteBytes(data_buffer,4);
		//osDelay(10);
		HAL_Delay(10);
    // Redo ati
		data_buffer[0] = PROX_SETTINGS;
    data_buffer[1] = 0x10;
		iqs263_WriteBytes(data_buffer,2);
		//osDelay(10);
		HAL_Delay(10);
    // Wait untill the ATI algorithm is done
    do
    {
			  data_buffer[0]=PROX_SETTINGS;
			  iqs263_WriteBytes(data_buffer,1);
			  //osDelay(10);
			  HAL_Delay(10);
				iqs263_ReadBytes(data_buffer,1);
    }while((data_buffer[0] & 0x04) == 0x04);

		//osDelay(10);
		HAL_Delay(10);
     // Setup prox settings
		data_buffer[0] = PROX_SETTINGS;
    data_buffer[1] = PROXSETTINGS0_VAL;
    data_buffer[2] = (PROXSETTINGS1_VAL|0x40);   //go to event
    data_buffer[3] = (PROXSETTINGS2_VAL);
    data_buffer[4] = PROXSETTINGS3_VAL;
    data_buffer[5] = EVENT_MASK_VAL;
		iqs263_WriteBytes(data_buffer,6);
}
//-------------------------------------------------
void touchEvent(void)
{
    unsigned char touch0 = data_buffer[2];
		//Info_Handler("touchEvent");
    if (touch0 != 0)
    {
        /* CHANNEL 1*/
        if ((touch0 & 0x02) == 0x02)                    // If a touch event occurs on Channel 1
        {
    //        LATDbits.LATD0 = 0;                         // Toggle LED 1 ON
        }
        else
        {
     //       LATDbits.LATD0 = 1;                         // Toggle LED 1 OFF
        }

        /* CHANNEL 2 */
        if ((touch0 & 0x04) == 0x04)                    // If a touch event occurs on Channel 2
        {
    //        LATDbits.LATD1 = 0;                         // Toggle LED 2 ON
        }
        else
        {
    //        LATDbits.LATD1 = 1;                         // Toggle LED 2 OFF
        }

        /* CHANNEL 3 */
        if ((touch0 & 0x08) == 0x08)                    // If a touch event occurs on Channel 3
        {
     //       LATDbits.LATD0 = 0;                         // Toggle LED 1 ON
    //        LATDbits.LATD1 = 0;                         // Toggle LED 2 ON
        }
        else
        {
     //       LATDbits.LATD0 = 1;                         // Toggle LED 1 OFF
     //       LATDbits.LATD1 = 1;                         // Toggle LED 2 OFF
        }
    }
}
//------------------------------------------------
void slideEvent(void)
{
    unsigned char sliderCoords = data_buffer[3];
    unsigned char touch0 = data_buffer[2];
		//Info_Handler("slideEvent");
    if (touch0 != 0)
    {
        if ((sliderCoords > 0 && sliderCoords < 85)&& ((touch0 & 0x02) == 0x02))
        {
    //        LATDbits.LATD0 = 0;             // Toggle LED 1 ON
    //        LATDbits.LATD1 = 1;             // Toggle LED 2 OFF
    //        LATDbits.LATD2 = 1;             // Toggle LED 3 OFF
    //        LATDbits.LATD3 = 1;             // Toggle LED 4 OFF
        }
        else if ((sliderCoords > 0 && sliderCoords < 170)&& ((touch0 & 0x04) == 0x04))
        {
     //       LATDbits.LATD0 = 1;             // Toggle LED 1 OFF
     //       LATDbits.LATD1 = 0;             // Toggle LED 2 ON
     //       LATDbits.LATD2 = 1;             // Toggle LED 3 OFF
      //      LATDbits.LATD3 = 1;             // Toggle LED 4 OFF
        }
        else if ((sliderCoords > 170 && sliderCoords < 255)&& ((touch0 & 0x08) == 0x08))
        {
//            LATDbits.LATD0 = 0;             // Toggle LED 1 ON
//            LATDbits.LATD1 = 0;             // Toggle LED 2 ON
//            LATDbits.LATD2 = 1;             // Toggle LED 3 OFF
//            LATDbits.LATD3 = 1;             // Toggle LED 4 OFF
        }
        else
        {
//            all_lights_off();               // Toggle All LEDS OFF
        }
    }
}
//--------------------------------------------------------
void proxEvent(void)
{
    unsigned char prox = data_buffer[2];
		//Info_Handler("proxEvent");
if ((prox & 0x01) == 0x01)              // If a prox event occures
    {
   //     LATDbits.LATD3 = 0;                 // Toggle LED 4 ON
		//	 LED0_ON;
    }
    else
    {
   //     LATDbits.LATD3 = 1;                 // Toggle LED 4 OFF
		//	LED0_OFF;
    }
}
//--------------------------------------------------
void movementEvent(void)
{
    unsigned char movement = data_buffer[1];
		//Info_Handler("movementEvent");
    if ((movement & 0x10) == 0x10)          // If a movement event occurs
    {
      //  LATDbits.LATD2 = 0;                 // Toggle LED 3 ON
		//	LED1_ON;
    }
    else
    {
      //  LATDbits.LATD2 = 1;                 // Toggle LED 3 OFF
		//	LED1_OFF;
    }
}
//---------------------------------------------------
void tapEvent(void)
{
    unsigned char  i;
    unsigned char tap = data_buffer[1];
	
		Info_Handler("tapEvent");


    if ((tap & 0x20) == 0x20)               // If a tap event occurs
    {
    }
}
//-----------------------------------------------
void flickRight(void)
{
    unsigned char  i;
    unsigned char rightFlick = data_buffer[1];
		Info_Handler("flickRight");
    if ((rightFlick & 0x80) == 0x80)       // If a right flick event occurs
    {
    }
    else
    {
    }
}
//--------------------------------------------------
void flickLeft(void)
{
    unsigned char  i;
    unsigned char leftFlick = data_buffer[1];
		Info_Handler("flickLeft");
    if ((leftFlick & 0x40) == 0x40)        // If a left click event occurs
    {
    }
    else
    {
    }
}
//----------------------------------------------
unsigned char handleEvents(void)
{
	unsigned char show_reset = 1;
	unsigned char  events=0;
	uint8_t temp=0;
  if(iqs263_rdy_read()==1)
		return 0;
	//temp=SYS_FLAGS;
	//if(iqs263_WriteBytes(&temp,1)==1)
	//	return 0xff;
	//osDelay();
	if(iqs263_ReadBytes(data_buffer,2)==1)
		return 0xff;
	iqs263_rdy(1);
	temp=SYS_FLAGS;
	if(iqs263_WriteBytes(&temp,1)==1)
		return 0xff;
	iqs263_rdy(0);

	//osDelay(5);
	//temp=TOUCH_BYTES;
	//iqs263_WriteBytes(&temp,1);
	//osDelay(5);
	//iqs263_ReadBytes(&data_buffer[2],1);
	//osDelay(5);
	//temp=COORDINATES;
	//iqs263_WriteBytes(&temp,1);
	//osDelay(5);
	//iqs263_ReadBytes(&data_buffer[3],3);
  show_reset = data_buffer[0]&0x80;
	
//	SEGGER_RTT_printf(0,"SYS_FLAGS=%02x,%02x!\r\n",data_buffer[0],data_buffer[1]);		
//	SEGGER_RTT_printf(0,"TOUCH_BYTES=%02x!\r\n",data_buffer[2]);		
	//SEGGER_RTT_printf(0,"COORDINATES=%02x,%02x,%02x!\r\n",data_buffer[3],data_buffer[4],data_buffer[5]);	
	if(show_reset)
	{
		__nop();
	}
    events = data_buffer[1];
//return events; 	
	    /********************************* TAP ************************************/

    if((events & 0x20) == 0x20)
    {    tapEvent();
			temp=1;
		}

    /******************************* FLICK (LEFT) *****************************/

    if((events & 0x40) == 0x40)
    {    flickLeft();
			temp=2;
		}

    /******************************* FLICK (RIGHT) ****************************/

    if((events & 0x80) == 0x80)
		{
        flickRight();
			temp=3;
		}
SEGGER_RTT_printf(0,"flick:%d\r\n",temp);

return temp; 	
		#if 0
    /******************************* PROXIMITY ********************************/
		
    if ((events & 0x01) == 0x01)
        proxEvent();

    /******************************** TOUCH ***********************************/
    if ((events & 0x02) == 0x02)
        touchEvent();

    /******************************** SLIDE ***********************************/

    if (((events & 0x04) == 0x04) && (events != 0))
        slideEvent();

    /******************************* MOVEMENT *********************************/

    if ((events & 0x10) == 0x10)
        movementEvent();
    //else
    //    LATDbits.LATD2 = 1;

    /********************************* TAP ************************************/

    if((events & 0x20) == 0x20)
        tapEvent();

    /******************************* FLICK (LEFT) *****************************/

    if((events & 0x40) == 0x40)
        flickLeft();

    /******************************* FLICK (RIGHT) ****************************/

    if((events & 0x80) == 0x80)
        flickRight();
		
		return events;
		#endif
}
//------------------------------------------------------
unsigned char iqs263_init()
{
	HAL_StatusTypeDef ret;
	uint16_t i=0;
	uint32_t temp;

	//和设备通讯
	data_buffer[0]=DEVICE_INFO;
#if 0
while(1)
{
	while(HAL_I2C_Master_Transmit_IT(&hi2c1, S_ADDRESS, data_buffer, 1)!= HAL_OK)
  {
    /* Error_Handler() function is called when Timeout error occurs.
       When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler("i2c1 tx comm error!");
    }
			i++;
		if(i>0xff)
			return 0;

  }
	i=0;
	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
  {
		i++;
  } 
	i=0;
  while(HAL_I2C_Master_Receive_IT(&hi2c1, S_ADDRESS, data_buffer, 1) != HAL_OK)
  {
    /* Error_Handler() function is called when Timeout error occurs.
       When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler("i2c1 rx comm error!");
    } 
		i++;
  }
	i=0;
	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
  {
		i++;
  } 
	if(buffer[0]==0x3c)
			break;
}
#endif

iqs263_rdy(1);
//osDelay(1);
iqs263_WriteBytes(data_buffer,1);
//osDelay(1);
HAL_Delay(1);
iqs263_ReadBytes(data_buffer,1);
SEGGER_RTT_printf(0,"touch id:%x\r\n",data_buffer[0]);

if(data_buffer[0]==0x3c)
{	
	osDelay(10);
	IQS263DeviceSettings();
	data_buffer[0]=SYS_FLAGS;
	iqs263_WriteBytes(data_buffer,1);
	iqs263_rdy(0);
	return 1;
}
else
		return 0;

/*
	while(1)
	{
		i++;
	ret=HAL_I2C_Master_Transmit(&hi2c1, S_ADDRESS,buffer, 1, 1000);
	temp=HAL_I2C_GetError(&hi2c1) ;
	if(temp==4)
		continue;
	ret=HAL_I2C_Master_Receive(&hi2c1, S_ADDRESS,buffer, 1, 1000);
	temp=HAL_I2C_GetError(&hi2c1) ;
	if(buffer[0]==0x3c)
			break;
	
	buffer[0]=DEVICE_INFO;
	}
	*/
}
//------------------------------------------
