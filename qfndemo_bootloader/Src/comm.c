//通讯协议处理文件
#include "stm32f4xx_hal.h"
#include "usart.h"
#include "gpio.h"

//变量
extern uint8_t data_buffer[];
static uint8_t buffer[10];//串口发送数据用，不能在堆栈
extern uint8_t update_flag;

//flash address
#define ADDR_FLASH_SECTOR_0     (0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     (0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     (0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     (0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     (0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     (0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     (0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     (0x08060000) /* Base @ of Sector 7, 128 Kbytes */

#define FLASH_ROM_SECTOR_ADDR   ADDR_FLASH_SECTOR_2   /* sector*/
static FLASH_EraseInitTypeDef EraseInitStruct;
static uint32_t SectorError = 0;
static uint32_t current_add=FLASH_ROM_SECTOR_ADDR;//开始地址

typedef volatile unsigned long      vu32;
typedef  void (*FunVoidType)(void);
static FunVoidType JumpToApplication;
static uint32_t m_JumpAddress;

extern UART_HandleTypeDef huart2;

//-------------------------------------------------------------------
//跳转到主程序
void JumpToApp(void)
{

	 uint32_t temp= *(vu32*)FLASH_ROM_SECTOR_ADDR;
	//as7000 power up for as7000 burn
	#if 0
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	while(1)
	{
	 
		HAL_Delay(2000);

	}
	#endif

    if (((*(vu32*)FLASH_ROM_SECTOR_ADDR) & 0x2FFE0000 ) == 0x20000000)
    {
				HAL_UART_DeInit(&huart2);
				HAL_DeInit();
				__disable_irq();
			   /* reset systick */
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        /* disable all peripherals clock. */
        RCC->AHB1ENR = (1<<20); /* 20: F4 CCMDAT ARAMEN. */
        RCC->AHB2ENR = 0;
        RCC->AHB3ENR = 0;
        RCC->APB1ENR = 0;
        RCC->APB2ENR = 0;
			 /* Switch to default cpu clock. */
        RCC->CFGR = 0;
			 /* Disable MPU */
				MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;
				/* Set new vector table pointer */
				SCB->VTOR = FLASH_ROM_SECTOR_ADDR;

				/* reset register values */
				__set_BASEPRI(0);
				__set_FAULTMASK(0);

				/* set up MSP and switch to it */
				__set_MSP(*(uint32_t*)FLASH_ROM_SECTOR_ADDR);
				__set_PSP(*(uint32_t*)FLASH_ROM_SECTOR_ADDR);
				__set_CONTROL(0);

				/* ensure what we have done could take effect */
				__ISB();

				__disable_irq();
			
				/* disable and clean up all interrupts. */
				{
						int i;

						for(i = 0; i < 8; i++)
						{
								/* disable interrupts. */
								NVIC->ICER[i] = 0xFFFFFFFF;

								/* clean up interrupts flags. */
								NVIC->ICPR[i] = 0xFFFFFFFF;
						}
				}
			
				//SCB->VTOR=FLASH_ROM_SECTOR_ADDR;
        /* Jump to user application */
        m_JumpAddress = *(vu32*) (FLASH_ROM_SECTOR_ADDR + 4);
        JumpToApplication = (FunVoidType) m_JumpAddress;
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(vu32*) FLASH_ROM_SECTOR_ADDR);
        JumpToApplication();
    }
		else
		{
			HAL_NVIC_SystemReset();
		}
}
//-------------------------------------------------------------------
static uint32_t GetSector(uint32_t Address)
{ 
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;  
  }
  else if(Address >= ADDR_FLASH_SECTOR_7)
  {
    sector = FLASH_SECTOR_7;  
  }

  return sector;
}
//-------------------------------------------------------------
//------------------------------------------------------
//计算校验和
uint8_t check_sum(uint8_t* data,uint8_t len)
{
	uint8_t temp=0;
	for(int i=0;i<len;i++)
		temp+=data[i];
	return temp;
}

//---------------------------------------------------------
//update start
uint8_t update_start()
{
	//unlock flash
	
	//erase flash
	uint8_t temp;
	current_add=FLASH_ROM_SECTOR_ADDR;
	//temp=*((uint8_t *)current_add);
	//temp=*((uint8_t *)current_add+1);

	HAL_FLASH_Unlock();
	EraseInitStruct.Sector =  GetSector(FLASH_ROM_SECTOR_ADDR);
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.NbSectors = 4;
	if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
	{ 
				return 0;
	}
			//clear buffer
	__HAL_FLASH_DATA_CACHE_DISABLE();
	__HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
	__HAL_FLASH_DATA_CACHE_RESET();
	__HAL_FLASH_INSTRUCTION_CACHE_RESET();
	__HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
	__HAL_FLASH_DATA_CACHE_ENABLE();
	
	
}
//-------------------------------------------
void update_finish()
{
	//lock flash
	HAL_FLASH_Lock();
	current_add=FLASH_ROM_SECTOR_ADDR;
	update_flag=0;

}
//---------------------------------------
unsigned char update_data()
{
	if(data_buffer[4]<3)
	{
		return 0;
	}
	//program data
	for(int i=0;i<(data_buffer[4]-2);i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,current_add, data_buffer[7+i]);
		current_add+=1;
	}
	return 1;
	
}
//------------------------------------------------
void send_sortno()
{
	uint16_t temp=data_buffer[6];
	temp=(temp<<8)|data_buffer[5];
	buffer[0]=0xfe;
	buffer[1]=0x05;
	buffer[2]=0x21;//command
	buffer[3]=0x02;
	memcpy(&buffer[4],(uint8_t *)&temp,2);
	buffer[6]=check_sum(buffer,6);
	uart_send(buffer,7);
}
//-----------------------------------------
void update_exit()
{
	//lock flash
	HAL_FLASH_Lock();
	current_add=FLASH_ROM_SECTOR_ADDR;
	update_flag=0;

}
//---------------------------------------------------
//发送数据确认命令
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
	uart_send(buffer,6);
}
//-----------------------------------------------------
//处理接收到的数据
//return:1-收到命令正确；0-收到命令错误
void rece_dispatch()
{
	uint8_t len=data_buffer[0];
	if(data_buffer[1]!=0xfe || data_buffer[2]!=0x04 || (data_buffer[len]!=check_sum(&data_buffer[1],len-1)) )
	{
		//收到错误命令回复
		send_shakehand(0);
	}
	//解析命令
	switch(data_buffer[3])
	{
		case 0x21://接收固件数据
			if(update_data()==1)
				send_sortno();
			else
				send_shakehand(0);
			break;
		case 0x22://结束升级命令
			if(update_data()==1)
			{	
				send_sortno();
				update_finish();
				JumpToApp();
			}
			else
				send_shakehand(0);
			break;
		case 0x23://中断升级命令
			update_exit();
			break;
		default:
			send_shakehand(0);//收到异常命令
			return;
	};
}