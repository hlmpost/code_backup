//显示处理
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "EPD_TCON.h"
#include "Graphics.h"
#include "disp.h"

#define SCREEN_WIDTH    128 //1,43"
#define SCREEN_HEIGHT   296 //1,43"
#define ARRAY1DOT43   (9472)
#define ARRAYSIZE     ARRAY1DOT43

#define EDGE1_INDEX 8  //0~7   //#8  //(SCREEN_WIDTH/(16))
#define EDGE2_INDEX 24 //8~23  //#16 //(SCREEN_HEIGHT/16)-2
#define EDGE3_INDEX 32 //24~31 //#8
#define EDGE4_INDEX 48 //32~47 //#16

extern const unsigned char g_image_id143_1[9472]; 

extern const EPD_TCON_DRIVER_HAL g_TCON_HAL;
extern const unsigned char g_WaveformData[];

extern volatile uint8_t current_datetime;//current datetime

static const EPD_TCON_PANEL_INFO *_paneInfo;
//--------------------------------------------------------
void DrawProgress(int32_t index, uint8_t color)
{
//#define SCREEN_WIDTH    128 //1,43"
//#define SCREEN_HEIGHT   296 //1,43"
#define RECT_W (12)
#define RECT_H (12)

    index %= EDGE4_INDEX;
    if (index < EDGE1_INDEX)
    {
        GR_FillRectangle(index * 16 + 2, 2, RECT_W, RECT_H, color);
        GR_DrawRectangle(index * 16 + 2, 2, RECT_W, RECT_H, 0x0);                
    }
    else if (index < EDGE2_INDEX)
    {
        index = index - (EDGE1_INDEX-1);
        GR_FillRectangle((SCREEN_WIDTH-14), index * 16 + 2, RECT_W, RECT_H, color);
        GR_DrawRectangle((SCREEN_WIDTH-14), index * 16 + 2, RECT_W, RECT_H, 0x0);                                
    }
    else if (index < EDGE3_INDEX)
    {
        index = index - EDGE2_INDEX;
        index = (EDGE1_INDEX-1) - index;
        GR_FillRectangle(index * 16 + 2, (SCREEN_HEIGHT-22), RECT_W, RECT_H, color);
        GR_DrawRectangle(index * 16 + 2, (SCREEN_HEIGHT-22), RECT_W, RECT_H, 0x0);                
    }
    else
    {
        index = index - EDGE3_INDEX;
        index = (EDGE2_INDEX-EDGE1_INDEX) - index;
        GR_FillRectangle(2, index * 16 + 2, RECT_W, RECT_H, color);
        GR_DrawRectangle(2, index * 16 + 2, RECT_W, RECT_H, 0x0);                                
    }    
}

//-------------------------------------------------------
void marquee()
{

    uint16_t digit[3];
    uint32_t i;     
    digit[2] = 0;
            

    EPD_TCON_PowerOn(); 
    GR_FillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x03);
    
    for (i = 0; i < SCREEN_WIDTH; i += 8)
    {
        GR_DrawLine((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2), i, 0, 0);
        GR_DrawLine((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2), i, (SCREEN_HEIGHT - 1), 0);
    }
    for (i = 0; i < SCREEN_HEIGHT; i += 8)
    {
        GR_DrawLine((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2), (SCREEN_WIDTH - 1), i, 0);
        GR_DrawLine((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2), 0, i, 0);
    }
  
    for (i = 0; i < EDGE4_INDEX; i++)
    {
        DrawProgress(i + 0, 0x3);
        DrawProgress(i + 1, 0x0);   
    }
    EPD_TCON_Update(WF_MODE_GC4);    
                    
    for (i = 0; i < EDGE4_INDEX; i++)
    {
        digit[0] = '0' + ((i / 10) % 10);
        digit[1] = '0' + (i % 10);

        DrawProgress(i + 0, 0x3);
        DrawProgress(i + 1, 0x0);
        
                   
        EPD_TCON_Update(WF_MODE_A2);                  
         
    }
      
    EPD_TCON_PowerOff();
}
//-------------------------------------------------------------
static void ShowImage(const unsigned char *img)
{
    uint8_t *fb;
    int32_t i;
    fb = EPD_TCON_GetUpdateBuffer();

    for (i = 0; i < ARRAYSIZE; ++i)
    {
        *fb = (*fb & 0xF0) | ((*img >> 6) & 0x03);
        fb++;
        *fb = (*fb & 0xF0) | ((*img >> 4) & 0x03);
        fb++;
        *fb = (*fb & 0xF0) | ((*img >> 2) & 0x03);
        fb++;
        *fb = (*fb & 0xF0) | ((*img >> 0) & 0x03);
        fb++;
        img++;
    }
}

//-----------------------------------------------------
static void lcd_write_data(uint8_t x,uint8_t y,uint8_t* data,uint16_t l_len,uint16_t a_len,uint8_t color)
{
	uint16_t i=0,temp=0;
	uint16_t x_add=0,y_add=0;
	if( (l_len%8)>0)//judge
		l_len+=1;
	for(i=0;i<a_len;i++)
	{
		for(uint8_t j=0;j<8;j++)
		{
			temp=data[i]&( 1<<(7-j) );
			if(temp>1)
			{
				GR_DrawPixel(x+x_add,y+y_add,color);
			}
			x_add++;
			if(x_add==l_len)
			{	
				x_add=0;
				y_add++;
				break;
			}
		}
	}
}
//-------------------------------------------------------
void lcd_display_on(uint8_t flag)
{
	if(flag==1)
	    EPD_TCON_PowerOn();
	else
	    EPD_TCON_PowerOff();
}
void lcd_display_update(uint8_t flag)
{
	//0-3
//	#define WF_MODE_INIT     0
//#define WF_MODE_GC4      1
//#define WF_MODE_GU4      2
//#define WF_MODE_A2       3

	EPD_TCON_Update(flag); 
}

//------------------------------------------------------------------
//if align center then  x is not care ;only support 1 line
//1 line have only number
void lcd_disp_number(uint8_t x,uint8_t y,char * data,uint8_t font_h,uint8_t font_w)
{//align center
	uint8_t old_x;
	if( (font_w*strlen(data) )>128)
		return;
	x=(128-font_w*strlen(data))/2;
	old_x=x;
	GR_FillRectangle(0,y,128,font_h,3);
	for(uint8_t i=0;i<strlen(data);i++)
	{
		if(data[i]==0x2e)//.
			lcd_write_data(x,y,number22[10],font_w,sizeof(number22[10]),0);
		else
			lcd_write_data(x,y,number22[data[i]-0x30],font_w,sizeof(number22[data[i]-0x30]),0);
		x+=font_w;
//		if(x>(127-(old_x+16)))
//		{
//			x=old_x;
//			y+=34;
//		}
	}
}
//-----------------------------------------------------------\
//1 line have only font
void lcd_disp_font(uint8_t x,uint8_t y,uint8_t flag,uint8_t font_count,uint8_t font_h,uint8_t font_w)
{
	uint8_t old_x=0;
	if( (font_w*font_count )>128)
		return;
	x=(128-font_w*font_count)/2;
	old_x=x;
	GR_FillRectangle(0,y,128,font_h,3);
	for(uint8_t i=0;i<font_count;i++)
	{
		if(flag==1)//step count 
			lcd_write_data(x,y,f_step[i],font_w,sizeof(f_step[0]),0);
		else if(flag==2)//distance
			lcd_write_data(x,y,f_distance[i],font_w,sizeof(f_distance[0]),0);
		else if(flag==3)//meter
			lcd_write_data(x,y,f_meter[i],font_w,sizeof(f_meter[0]),0);
		else if(flag==4)//卡路里
			lcd_write_data(x,y,f_cal[i],font_w,sizeof(f_cal[0]),0);
		else if(flag==5)//心率
			lcd_write_data(x,y,f_hrs_rate[i],font_w,sizeof(f_hrs_rate[0]),0);

		x+=font_w;
	}
}
//------------------------------------------------------------
//display bmp
void lcd_disp_bmp(uint8_t x,uint8_t y,uint8_t flag)
{//align center
	uint8_t 	old_x=x;
	if(flag==1)//step
	{
		x=(128-45)/2;
		lcd_write_data(x,y,b_step,45,sizeof(b_step),0);
	}
	else if(flag==2)//consume
	{
		x=(128-55)/2;
		lcd_write_data(x,y,b_consume,55,sizeof(b_consume),0);
	}
	else if(flag==3)//hrs rate
	{
		x=(128-80)/2;
		lcd_write_data(x,y,b_hrs_rate,80,sizeof(b_hrs_rate),0);
	}
}
//------------------------------------------------------------------
//if align center then  x is not care ;only support 1 line
void lcd_disp_time(uint8_t x,uint8_t y,char * time,uint8_t font_h,uint8_t font_w)
{//align center
	uint8_t old_x;
	if( (font_w*2)>128)
		return;
	x=(128-font_w*2)/2;
	old_x=x;
	GR_FillRectangle(0,y,128,font_h*2,3);
	for(uint8_t i=0;i<4;i++)
	{
		
		if(i>1)
			lcd_write_data(x,y,f_time[time[i]-0x30],font_w,sizeof(f_time[time[i]-0x30]),1);
		else
			lcd_write_data(x,y,f_time[time[i]-0x30],font_w,sizeof(f_time[time[i]-0x30]),0);
		x+=font_w;
		if(i==1)
		{
			x=old_x;
			y+=font_h;
		}
	}
}
//--------------------------------------------------------------
//display date
void lcd_disp_date(uint8_t x,uint8_t y,char * date,uint8_t font_h,uint8_t font_w)
{//align center
	uint8_t old_x;
	if( (font_w*2)>128)
		return;
	x=(128-(font_w*4+58))/2;
	old_x=x;
	GR_FillRectangle(0,y,128,font_h*2,3);
	for(uint8_t i=0;i<4;i++)
	{
		lcd_write_data(x,y,number22[date[i]-0x30],font_w,sizeof(number22[date[i]-0x30]),0);
		x+=font_w;
		if(i==1)
		{
			lcd_write_data(x,y,f_date[0],29,sizeof(f_date[0]),0);
			x+=29;
		}
		else if(i==3)
		{
			lcd_write_data(x,y,f_date[8],29,sizeof(f_date[8]),0);
			x+=29;
		}
		
	}
	//week;
	x=(128-(font_w+29))/2;
	y+=29;
	lcd_write_data(x,y,f_date[1],29,sizeof(f_date[1]),0);
	x+=29;
	lcd_write_data(x,y,f_date[date[4]-0x30+1],29,sizeof(f_date[date[4]-0x30+1]),0);
}
//-------------------------------------------------------------
//disp battery
void lcd_disp_battery(uint8_t batt_percent)
{
	GR_DrawRectangle(96,1,25,15,0);
	GR_FillRectangle(121,5,3,6,0);
	GR_FillRectangle(97,2,23,13,3);
	GR_FillRectangle(97,2,(23*batt_percent)/100,13,0);
}
//---------------------------------------------------------
//disp test
void lcd_test()
{
    uint16_t digit[3],x=0,y=0;
    uint32_t temp=0;  
            

    EPD_TCON_PowerOn(); 
		//lcd_disp_number(1,1,"0123456789",24);
		//GR_DrawRectangle(x,x , SCREEN_WIDTH-x-x, SCREEN_HEIGHT-x-x, 0);
//	for(uint8_t i=0;i<10;i++)
//	{
//		lcd_write_data(1+x,1+y,number[i],16,sizeof(number[i]));
//		x+=16;
//		if(x>111)
//		{
//			x=0;
//			y+=34;
//		}
//	}
	//lcd_write_data(30,34,step,45,sizeof(step));
	//lcd_write_data(30,110,consume,55,sizeof(consume));
	//lcd_write_data(30,190,hrs_rate,80,sizeof(hrs_rate));
		EPD_TCON_Update(WF_MODE_GU4); 
		    EPD_TCON_PowerOff();

	

}
//------------------------------------------------
void lcd_disp_clear()
{
	GR_FillRectangle(0,0,128,296,3);
}
//------------------------------------------------
//lcd init
uint8_t lcd_init()
{
		int32_t ret;   
    uint8_t *frameBuffer;
	

		//init lcd
	 //EPD_HAL_Init();
    ret = EPD_TCON_Init(&g_TCON_HAL);
    if ( ret < 0)
    {
        //TCON Init Fail!!
        while (1);
    }
		EPD_TCON_LoadWaveform(g_WaveformData);

    EPD_TCON_SetVcom(-2960);

    _paneInfo = EPD_TCON_GetPanelInfo();
		
		frameBuffer = EPD_TCON_GetUpdateBuffer();
    GR_Initial(frameBuffer);

    EPD_TCON_PowerOn();
    EPD_TCON_PanelClear();
    EPD_TCON_PowerOff();

}

