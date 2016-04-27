/* -----------------------------------------------------------------------------
** Copyright (C)2015 E Ink Holdings Inc. All rights reserved.
** -----------------------------------------------------------------------------
** Customer Name : Guangzhou Adsmart Technology Ltd 
** Contact window: Watwang
** Licensing date: Feb/24/2016
** -------------------------------------------------------------------------- */
  
#include <stdint.h>

#include "Graphics.h"

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   296

typedef struct
{
    uint8_t *FrameBuffer;
    uint8_t TextAlign;
    uint8_t LineStyle;
    uint8_t TextFrontColor;
    uint8_t TextBackColor;
    uint8_t TextTransparent;
    GR_Rectangle ClipArea;
    GR_Font FontSet[8];
}GR_Data;

static GR_Data _data;

/******************************************************************************
******************************************************************************/
void GR_Initial(uint8_t *frameBuffer)
{
    uint16_t i;

    _data.FrameBuffer = frameBuffer;

	for (i = 0; i < sizeof(_data.FontSet) / sizeof(_data.FontSet[0]); ++i)
	{
		_data.FontSet[i].Allow = 0;
	}
	_data.ClipArea.x = 0;
	_data.ClipArea.y = 0;
	_data.ClipArea.w = SCREEN_WIDTH;
	_data.ClipArea.h = SCREEN_HEIGHT;
	_data.TextAlign = 0;
	_data.LineStyle = 0xFF;
    _data.TextFrontColor = 0;
    _data.TextBackColor = 3;
    _data.TextTransparent = 0;
}

/******************************************************************************
******************************************************************************/
uint8_t *GR_GetFrameBuffer()
{
    return _data.FrameBuffer;
}


/******************************************************************************
******************************************************************************/
void GR_SetTextAlign(uint8_t align)
{
	_data.TextAlign = align;
}

/******************************************************************************
******************************************************************************/
void GR_SetClip(int16_t x, int16_t y, int16_t w, int16_t h)
{
	if (w <= 0 || h <= 0)
	{
		return;
	}

	if (x < 0)
	{
		w = w + x;
		x = 0;
	}

	if (y < 0)
	{
		h = h + y;
		y = 0;
	}

	if ((x + w) > SCREEN_WIDTH)
	{
		w = SCREEN_WIDTH - x;
	}

	if ((y + h) > SCREEN_HEIGHT)
	{
		h = SCREEN_HEIGHT - y;
	}

	_data.ClipArea.x = x;
	_data.ClipArea.y = y;
	_data.ClipArea.w = w;
	_data.ClipArea.h = h;
}

/******************************************************************************
******************************************************************************/
inline void GR_DrawPixel(int16_t x, int16_t y, uint8_t color)
{
	uint16_t address;
	if (x < _data.ClipArea.x || y < _data.ClipArea.y ||
		x >= (_data.ClipArea.x + _data.ClipArea.w) ||
		y >= (_data.ClipArea.y + _data.ClipArea.h))
		return;

	address = x + y * SCREEN_WIDTH;

    _data.FrameBuffer[address] &= 0xF0;
    _data.FrameBuffer[address] |= color;
}

/******************************************************************************
******************************************************************************/
static void GR_DrawHLine(int16_t x1, int16_t x2, int16_t y, uint8_t color)
{
	uint16_t address;
	uint8_t  *ptr;
	uint32_t value;
	int16_t  ox;

	if (x2 < x1)
	{
		ox = x1;
		x1 = x2;
		x2 = ox;
	}

	if (y < _data.ClipArea.y || y >= (_data.ClipArea.y + _data.ClipArea.h))
		return;

	if (x1 < _data.ClipArea.x)
	{
		x1 = _data.ClipArea.x;
	}

	if (x2 >= (_data.ClipArea.x + _data.ClipArea.w))
	{
		x2 = (_data.ClipArea.x + _data.ClipArea.w) - 1;
	}

	ox = x1;

	address = x1 + y * SCREEN_WIDTH;

	ptr = _data.FrameBuffer + address;

	value = color * 0x01010101;

	if (((uint32_t)ptr & 0x01) && (x2 - x1) >= 0)
	{
		x1 += 1;
        *ptr &= 0xF0;
		*ptr |= (uint8_t)value & 0x0F;
		++ptr;
	}

	if (((uint32_t)ptr & 0x02) && (x2 - x1) >= 1)
	{
		x1 += 2;
        *((uint16_t *)ptr) &= 0xF0F0;
		*((uint16_t *)ptr) |= (uint16_t)value & 0x0F0F;
		ptr += 2;
	}

	while ((x1 + 3) <= x2)
	{
        *((uint32_t *)ptr) &= 0xF0F0F0F0;
		*((uint32_t *)ptr) |= value;
		ptr += 4;
		x1 += 4;
	}

	if ((x1 + 1) <= x2)
	{
		x1 += 2;
        *((uint16_t *)ptr) &= 0xF0F0;
		*((uint16_t *)ptr) |= (uint16_t)value & 0x0F0F;
		ptr += 2;
	}

	if (x1 <= x2)
	{
		x1 += 1;
        *ptr &= 0xF0;
		*ptr |= (uint8_t)value & 0x0F;
		++ptr;
	}

}

/******************************************************************************
******************************************************************************/
static void GR_DrawVLine(int16_t x, int16_t y1, int16_t y2, uint8_t color)
{
	uint16_t address;
	uint8_t *ptr;
	int16_t oy;

	if (y2 < y1)
	{
		oy = y1;
		y1 = y2;
		y2 = oy;
	}

	if (x < _data.ClipArea.x || x >= (_data.ClipArea.x + _data.ClipArea.w))
		return;

	if (y1 < _data.ClipArea.y)
	{
		y1 = _data.ClipArea.y;
	}

	if (y2 >= (_data.ClipArea.y + _data.ClipArea.h))
	{
		y2 = (_data.ClipArea.y + _data.ClipArea.h) - 1;
	}

	oy = y1;
	address = x + y1 * SCREEN_WIDTH;
	ptr = _data.FrameBuffer + address;

	while (y1 <= y2)
	{
        *ptr &= 0xF0;
		*ptr |= color;
		ptr += SCREEN_WIDTH;
		++y1;
	}
}

/******************************************************************************
******************************************************************************/
void GR_DrawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color)
{
	int16_t x1, x2;
	int16_t y1, y2;

	if (w == 0 || h == 0)
		return;

	if (w < 0)
	{
		x = x + w - 1;
		w *= -1;
	}

	if (h < 0)
	{
		y = y + h - 1;
		h *= -1;
	}

	x1 = x;
	y1 = y;
	x2 = x + w - 1;
	y2 = y + h - 1;

	if (x2 < _data.ClipArea.x || y2 < _data.ClipArea.y ||
		 x1 >= (_data.ClipArea.x + _data.ClipArea.w) ||
		 y1 >= (_data.ClipArea.y + _data.ClipArea.h))
	{
		return;
	}

	GR_DrawHLine(x1, x2, y1, color);
	GR_DrawHLine(x1, x2, y2, color);
	GR_DrawVLine(x1, y1, y2, color);
	GR_DrawVLine(x2, y1, y2, color);
}

/******************************************************************************
******************************************************************************/
void GR_FillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color)
{
	int16_t x1, x2;
	int16_t y1, y2;

	if (w == 0 || h == 0)
		return;

	if (w < 0)
	{
		x = x + w - 1;
		w *= -1;
	}

	if (h < 0)
	{
		y = y + h - 1;
		h *= -1;
	}

	x1 = x;
	y1 = y;
	x2 = x + w - 1;
	y2 = y + h - 1;

	if (x2 < _data.ClipArea.x || y2 < _data.ClipArea.y ||
		 x1 >= (_data.ClipArea.x + _data.ClipArea.w) ||
		 y1 >= (_data.ClipArea.y + _data.ClipArea.h))
	{
		return;
	}

	while (y1 <= y2)
	{
		GR_DrawHLine(x1, x2, y1, color);
		++y1;
	}
}

/******************************************************************************
******************************************************************************/
void GR_SetLineStyle(uint8_t style)
{
	_data.LineStyle = style;
}

/******************************************************************************
******************************************************************************/
void GR_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color)
{
	int16_t err;
	int16_t x_step, y_step;
	volatile int16_t x_len, y_len;
	uint16_t count;

	if (x2 < x1)
	{
		x_len = x1 - x2 + 1;
		x_step = -1;
	}
	else
	{
		x_len = x2 - x1 + 1;
		x_step = 1;
	}

	if (y2 < y1)
	{
		y_len = y1 - y2 + 1;
		y_step = -1;
	}
	else
	{
		y_len = y2 - y1 + 1;
		y_step = 1;
	}

	if (x_len < y_len)
	{
		err = -(y_len);
		count = y_len;

		if (_data.LineStyle & (1 << (count & 0x7)))
			GR_DrawPixel(x1, y1, color);

		while (count != 0)
		{
			if (_data.LineStyle & (1 << (count & 0x7)))
				GR_DrawPixel(x1, y1, color);
			err += x_len;
			if (err > 0)
			{
				x1 += x_step;
				err -= y_len;
			}
			y1 += y_step;
			--count;
		}
	}
	else
	{
		err = -(x_len);
		count = x_len;

		if (_data.LineStyle & (1 << (count & 0x7)))
			GR_DrawPixel(x1, y1, color);
		while (count != 0)
		{
			if (_data.LineStyle & (1 << (count & 0x7)))
				GR_DrawPixel(x1, y1, color);
			err += y_len;
			if (err > 0)
			{
				y1 += y_step;
				err -= x_len;
			}
			x1 += x_step;
			--count;
		}
	}

}

/******************************************************************************
******************************************************************************/
void GR_DrawFontImage(int16_t x, int16_t y, int16_t w, int16_t h,
		          const uint8_t *image)
{
	int16_t src_x1, src_y1, src_x2, src_y2;
	int16_t src_y_count;
	int16_t des_x, des_y;
	int16_t des_y_count;
	int16_t i;
	uint8_t src_shift, des_shift;
	const uint8_t *src_ptr;
	uint8_t *des_ptr;
	uint8_t value;

	if (x >= (_data.ClipArea.w + _data.ClipArea.x) || (x + w < _data.ClipArea.x) ||
		y >= (_data.ClipArea.h + _data.ClipArea.y) || (y + h < _data.ClipArea.y) ||
		w <= 0 || h <= 0)
	{
		return;
	}

	src_x1 = 0;
	src_y1 = 0;
	src_x2 = w - 1;
	src_y2 = h - 1;
	des_x = x;
	des_y = y;

	if (x < _data.ClipArea.x)
	{
		src_x1 = (_data.ClipArea.x - x);
		des_x = _data.ClipArea.x;
	}

	if ((src_x2 - src_x1 + 1) >= (_data.ClipArea.w - des_x))
	{
		src_x2 = src_x1 + (_data.ClipArea.w - des_x) - 1;
	}

	if (y < _data.ClipArea.y)
	{
		src_y1 = (_data.ClipArea.y - y);
		des_y = _data.ClipArea.y;
	}

	if ((src_y2 - src_y1 + 1) >= (_data.ClipArea.h - des_y))
	{
		src_y2 = src_y1 + (_data.ClipArea.h - des_y) - 1;
	}

	des_y_count = des_y;
	src_y_count = src_y1;

	while (src_y_count <= src_y2)
	{
		des_ptr = _data.FrameBuffer + (uint16_t)(des_x + des_y_count * SCREEN_WIDTH);
		src_ptr = image + (uint16_t)((src_x1 >> 3) + src_y_count * ((w + 7) >> 3));

		src_shift =  (src_x1 & 0x07);
        
		for (i = src_x1; i <= src_x2; ++i)
		{
			value = (*src_ptr >> src_shift) & 0x01;
            
            if (value)
            {
                if (!_data.TextTransparent)
                {
                    *des_ptr &= 0xF0;
                    *des_ptr |= _data.TextBackColor;
                }
            }
            else
            {
                *des_ptr &= 0xF0;
                *des_ptr |= _data.TextFrontColor;
            }

			if (src_shift == 7)
			{
				src_shift = 0;
				++src_ptr;
			}
            else
    			src_shift++;

            ++des_ptr;
		}

		++src_y_count;
		++des_y_count;
	}

}


/******************************************************************************
******************************************************************************/
static GR_Font *GR_CreateFont()
{
	uint16_t i;

	for (i = 0; i < sizeof(_data.FontSet) / sizeof(_data.FontSet[0]); ++i)
	{
		if (!_data.FontSet[i].Allow)
		{
			_data.FontSet[i].Allow = 1;
			return &_data.FontSet[i];
		}
	}
	return 0;
}

/******************************************************************************
******************************************************************************/
GR_Font *GR_LoadFont(const uint8_t *fontData)
{
	const uint8_t *ptr = fontData;
	GR_Font *pFont;
	uint16_t i;

	pFont = GR_CreateFont();

	if (pFont == NULL)
		return NULL;

	pFont->FontHeight = *ptr;
	++ptr;
	pFont->HasASCII = *ptr;
	++ptr;
	pFont->CharCount = *ptr;
	++ptr;
	pFont->CharCount |= (uint16_t)*ptr << 8;
	++ptr;
	pFont->BucketLength = *ptr;
	++ptr;

	for (i = 0; i < pFont->BucketLength; ++i)
	{
		pFont->BucketAddress[i] = *ptr;
		++ptr;
		pFont->BucketAddress[i] |= (uint16_t)*ptr << 8;
		++ptr;

		pFont->BucketCount[i] = *ptr;
		++ptr;
		pFont->BucketCount[i] |= (uint16_t)*ptr << 8;
		++ptr;
	}

	if (pFont->HasASCII)
	{
		pFont->pASCIITable = ptr;
		ptr = ptr + 95 * 2;
	}
	else
		pFont->pASCIITable = 0;

	pFont->pCharData = ptr;

	return pFont;
}

/******************************************************************************
******************************************************************************/
static const uint8_t *GR_FindChar(GR_Font *pFont, uint16_t ch)
{
	const uint8_t *ptr = NULL;
	uint16_t i, count;
	uint16_t unicode;

	if (ch < 0x20)
		ch = 0x20;

	if (ch >= 127 || (ch < 127 && !pFont->HasASCII))
	{
		i = ch & 0x07;
		count = pFont->BucketCount[i];
		i = pFont->BucketAddress[i];
		ptr = pFont->pCharData + i;

		for (i = 0; i < count; ++i)
		{
			unicode = *ptr | *(ptr + 1) << 8;
			if (unicode == ch)
			{
				break;
			}
			ptr = ptr + 4 + (pFont->FontHeight * ((*(ptr + 2) + 7) >> 3));
		}
		if (i >= count)
		{
			ch = 0x20;
		}
	}

	if (ch < 127 && pFont->HasASCII)
	{
		ptr = &pFont->pASCIITable[(ch - 0x20) * 2];
		i = *ptr | *(ptr + 1) << 8;
		ptr = pFont->pCharData + i;
	}

	return ptr;
}

/******************************************************************************
******************************************************************************/
static uint16_t GR_MeasureLine(const uint16_t *pText)
{
	uint16_t result = 1;
	while (*pText != 0)
	{
		if (*pText == 0x0D && *(pText + 1) == 0x0A)
		{
			++result;
			pText += 2;
			if (*pText == 0)
				--result;
			continue;
		}
		 ++pText;
	}
	return result;
}

/******************************************************************************
******************************************************************************/
void GR_MeasureString(GR_Font *pFont, const uint16_t *pText,
		              uint8_t multiLine, int16_t *w, int16_t *h)
{
	int16_t tx, ty;
	const uint8_t *ptr;
	tx = 0;
	ty = 0;

	*w = 0;
	*h = 0;

	while (*pText != 0)
	{
		if (*pText == 0x0D && *(pText + 1) == 0x0A)
		{
			if (!multiLine)
			{
				*w = tx;
				break;
			}

			ty += pFont->FontHeight;
			if (tx > *w)
				*w = tx;
			tx = 0;
			pText += 2;
			continue;
		}

		ptr = GR_FindChar(pFont, *pText);
		tx += *(ptr + 2);
		++pText;
	}

	if (ty == 0)
		ty = pFont->FontHeight;

	if (*w == 0)
		*w = tx;

	*h = ty;
}

/******************************************************************************
******************************************************************************/
void GR_DrawStringA(GR_Font *pFont, const char *pText, int16_t x, int16_t y,
		            int16_t w, int16_t h)
{
	 const uint8_t *ptr;
	 int16_t tx, ty;
	 int16_t offset;
	 tx = x;
	 ty = y;

	 while (*pText != 0)
	 {
		 if (*pText == 0x0D && *(pText + 1) == 0x0A)
		 {
			 ty += pFont->FontHeight;
			 tx = x;
			 pText += 2;

			 if (ty >= (_data.ClipArea.y + _data.ClipArea.h))
				 break;

			 continue;
		 }
		 if (tx >= (_data.ClipArea.x + _data.ClipArea.w))
		 {
			 ++pText;
			 continue;
		 }

 		 ptr = GR_FindChar(pFont, *pText);
 		 offset = *(ptr + 2);
 		 GR_DrawFontImage(tx, ty, offset, pFont->FontHeight, ptr + 4);
		 tx += offset;
		 ++pText;
	 }
 }

 /******************************************************************************
******************************************************************************/
void GR_DrawString(GR_Font *pFont, const uint16_t *pText, int16_t x, int16_t y,
		           int16_t w, int16_t h)
{
	const uint8_t *ptr;
	int16_t tx = 0;
	int16_t ty = 0;
	int16_t offset;
	int16_t str_w, str_h;

	switch (_data.TextAlign & 0x03)
	{
	case GR_ALIGN_LEFT:
		tx = x;
		break;
	case GR_ALIGN_HCENTER:
		GR_MeasureString(pFont, pText, 0, &str_w, &str_h);
		tx = x + ((w - str_w) >> 1);
		break;
	case GR_ALIGN_RIGHT:
		GR_MeasureString(pFont, pText, 0, &str_w, &str_h);
		tx = x + w - str_w;
		break;
	}

	switch (_data.TextAlign & 0x30)
	{
	case GR_ALIGN_TOP:
		ty = y;
		break;
	case GR_ALIGN_VCENTER:
		ty = y + ((h - GR_MeasureLine(pText) * pFont->FontHeight) >> 1);
		break;
	case GR_ALIGN_BOTTOM:
		ty = y + h - (GR_MeasureLine(pText) * pFont->FontHeight) - 1;
		break;
	}

	while (*pText != 0)
	{
		if (*pText == 0x0D && *(pText + 1) == 0x0A)
		{
			ty += pFont->FontHeight;
			tx = x;
			pText += 2;

			if (ty >= (_data.ClipArea.y + _data.ClipArea.h))
				break;

			switch (_data.TextAlign & 0x03)
			{
			case GR_ALIGN_LEFT:
				tx = x;
				break;
			case GR_ALIGN_HCENTER:
				GR_MeasureString(pFont, pText, 0, &str_w, &str_h);
				tx = x + ((w - str_w) >> 1);
				break;
			case GR_ALIGN_RIGHT:
				GR_MeasureString(pFont, pText, 0, &str_w, &str_h);
				tx = x + w - 1 - str_w;
				break;
			}
			continue;
		}
		if (tx >= (_data.ClipArea.x + _data.ClipArea.w))
		{
			++pText;
			continue;
		}

 		ptr = GR_FindChar(pFont, *pText);
 		offset = *(ptr + 2);
 		GR_DrawFontImage(tx, ty, offset, pFont->FontHeight, ptr + 4);
		tx += offset;
		++pText;
	}
 }

/******************************************************************************
******************************************************************************/
void GR_SetTextFrontColor(uint8_t color)
{
    _data.TextFrontColor = color & 0x03;
}

/******************************************************************************
******************************************************************************/
void GR_SetTextBackColor(uint8_t color)
{
    _data.TextBackColor = color & 0x03;
}

/******************************************************************************
******************************************************************************/
void GR_SetTextTransparent(uint8_t enable)
{
    _data.TextTransparent = enable ? 1 : 0;
}



