/* -----------------------------------------------------------------------------
** Copyright (C)2015 E Ink Holdings Inc. All rights reserved.
** -----------------------------------------------------------------------------
** Customer Name : Guangzhou Adsmart Technology Ltd 
** Contact window: Watwang
** Licensing date: Feb/24/2016
** -------------------------------------------------------------------------- */
  

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stdint.h>

#define GR_ALIGN_LEFT       0
#define GR_ALIGN_HCENTER    1
#define GR_ALIGN_RIGHT      2

#define GR_ALIGN_TOP        0
#define GR_ALIGN_VCENTER    (1 << 4)
#define GR_ALIGN_BOTTOM     (2 << 4)

#ifndef NULL
#define NULL    0
#endif

#define GR_ROTATION_0       0
#define GR_ROTATION_90      1
#define GR_ROTATION_180     2
#define GR_ROTATION_270     3

typedef struct _GR_Point
{
	int16_t x;
	int16_t y;
}GR_Point;

typedef struct _GR_Size
{
	int16_t w;
	int16_t h;
}GR_Size;

typedef struct _GR_Rectangle
{
	int16_t x;
	int16_t y;
	int16_t w;
	int16_t h;
}GR_Rectangle;

typedef struct _GR_Font
{
	uint8_t Allow;
	uint8_t ID;
	uint8_t BucketLength;
	uint8_t HasASCII;
	uint16_t FontHeight;
	uint16_t CharCount;

	uint16_t BucketAddress[8];
	uint16_t BucketCount[8];

	const uint8_t *pASCIITable;
	const uint8_t *pCharData;

}GR_Font;

typedef struct _GR_Bitmap
{
	uint16_t Width;
	uint16_t Height;
	uint8_t  pImage[1];
}GR_Bitmap;

/******************************************************************************
A¡M1I£g{|!Rwaicl?A
******************************************************************************/
extern void GR_Initial(uint8_t *frameBuffer);

extern uint8_t *GR_GetFrameBuffer(void);


extern void GR_SetTextAlign(uint8_t align);

extern void GR_SetClip(int16_t x, int16_t y, int16_t w, int16_t h);

extern void GR_DrawPixel(int16_t x, int16_t y, uint8_t color);

extern void GR_DrawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);

extern void GR_FillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);

extern void GR_SetLineStyle(uint8_t style);

extern void GR_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color);


extern GR_Font *GR_LoadFont(const uint8_t *fontData);

extern void GR_MeasureString(GR_Font *pFont, const uint16_t *pText,
		                     uint8_t multiLine, int16_t *w, int16_t *h);
extern void GR_DrawStringA(GR_Font *pFont, const char *pText,
						   int16_t x, int16_t y, int16_t w, int16_t h);
extern void GR_DrawString(GR_Font *pFont, const uint16_t *pText,
						  int16_t x, int16_t y, int16_t w, int16_t h);

extern void GR_SetTextFrontColor(uint8_t color);

extern void GR_SetTextBackColor(uint8_t color);

extern void GR_SetTextTransparent(uint8_t enable);


#endif /* GRAPHICS_H_ */
