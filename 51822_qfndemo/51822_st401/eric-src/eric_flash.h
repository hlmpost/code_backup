#ifndef __ERIC_FLASH_H
#define __ERIC_FLASH_H
#include <stdint.h>
//flash address
#define ADDR_FLASH_SECTOR_0     (0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     (0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     (0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     (0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     (0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     (0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     (0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     (0x08060000) /* Base @ of Sector 7, 128 Kbytes */

//放在ram中
//记录区的信息头结构体,8个字节
typedef struct Header
{
	uint32_t total_step_data;//step:记录当天的当前总步数；
	uint32_t start_add;//起始地址
} stru_header;//32位对齐

//存储在flash中
typedef struct
{
	uint16_t step_count;
	uint32_t sleep_status;
	uint16_t  hrs_rate;//for align
	uint16_t bld_press;// blood pressure
	
} stru_region;

//存储在flash中，记录一些参数，如步数
typedef struct
{
	uint8_t   step_len;//step length,单位 cm
	uint16_t  height;//person height
	uint8_t   bt_name[8];
	
} stru_para;

//日期时间对应结构
//typedef struct date_struct
//{
//	uint32_t 日期;//当前日期
//	uint8_t up_flag;//上传的标志；0-未上传，1-已上传
//	uint16_t curr_step_data;//step:记录当天的当前总步数；
//	uint8_t curr_hrs_data;//hrs：最新测量的心率数据
//	uint8_t *start_add;//起始地址
//} date_add;//32位对齐

#endif