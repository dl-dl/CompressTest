#include "convert.h"
#include "sizes.h"
#include <math.h>
#include <string.h>

static ui8 FindColor(ui8 bCol, ui8 gCol, ui8 rCol)
{
	static const ui8 ColorTabl[22][4] =
	{
		{ 0x3F, 0x00, 0x00, 1 },
	{ 0x7F, 0x00, 0x00, 1 },
	{ 0xFF, 0x00, 0x00, 1 },
	{ 0x00, 0x3F, 0x00, 2 },
	{ 0x00, 0x7F, 0x00, 2 },
	{ 0x00, 0xFF, 0x00, 2 },
	{ 0x3F, 0x3F, 0x00, 3 },
	{ 0x7F, 0x7F, 0x00, 3 },
	{ 0xFF, 0xFF, 0x00, 3 },
	{ 0x00, 0x00, 0x3F, 4 },
	{ 0x00, 0x00, 0x7F, 4 },
	{ 0x00, 0x00, 0xFF, 4 },
	{ 0x3F, 0x00, 0x3F, 5 },
	{ 0x7F, 0x00, 0x7F, 5 },
	{ 0xFF, 0x00, 0xFF, 5 },
	{ 0x00, 0x3F, 0x3F, 6 },
	{ 0x00, 0x7F, 0x7F, 6 },
	{ 0x00, 0xFF, 0xFF, 6 },
	{ 0x3F, 0x3F, 0x3F, 0 },
	{ 0x7F, 0x7F, 0x7F, 0 },
	{ 0xFF, 0xFF, 0xFF, 7 },
	{ 0x00, 0x00, 0x00, 0 }
	};

	unsigned int diff;
	unsigned int minDiff = 0x1000000;
	ui8 minNum;

	for (int cnt = 0; cnt < 22; cnt++)
	{
		if (bCol > ColorTabl[cnt][0])
			diff = bCol - ColorTabl[cnt][0];
		else
			diff = ColorTabl[cnt][0] - bCol;

		if (gCol > ColorTabl[cnt][1])
			diff += gCol - ColorTabl[cnt][1];
		else
			diff += ColorTabl[cnt][1] - gCol;

		if (rCol > ColorTabl[cnt][2])
			diff += rCol - ColorTabl[cnt][2];
		else
			diff += ColorTabl[cnt][2] - rCol;

		if (diff < minDiff)
		{
			minDiff = diff;
			minNum = cnt;
		}
	}
	return (ColorTabl[minNum][3]);
}

void Invert24(const void *src, void *dst)
{
	for (int y = 0; y < TILE_CY; y++)
	{
		const ui8 *srcPtr = (ui8 *)src + y * TILE_CX * 3;
		for (int x = 0; x < TILE_CX; x++)
		{
			ui8 *dstPtr = (ui8 *)dst + (x * TILE_CY + y) * 3;
			dstPtr[0] = *srcPtr++;
			dstPtr[1] = *srcPtr++;
			dstPtr[2] = *srcPtr++;
		}
	}
}

void Invert8(const void *src, void *dst)
{
	for (int y = 0; y < TILE_CY; y++)
	{
		const ui8 *srcRow = (ui8 *)src + y * TILE_CX;
		for (int x = 0; x < TILE_CX; x++)
		{
			*((ui8 *)dst + x * TILE_CY + y) = srcRow[x];
		}
	}
}

void ConvertInv24To8Approx(const void *src, void *dst)
{
	for (int y = 0; y < TILE_CY; y++)
	{
		const ui8 *srcPtr = (ui8*)src + y * TILE_CX * 3;
		for (int x = 0; x < TILE_CX; x++)
		{
			ui8 bCol = *srcPtr++;
			ui8 gCol = *srcPtr++;
			ui8 rCol = *srcPtr++;
			*((ui8*)dst + x * TILE_CY + y) = FindColor(bCol, gCol, rCol);
		}
	}
}

void Convert24To8Approx(const void *src, void *dst)
{
	for (int x = 0; x < TILE_CX; x++)
	{
		ui8 *dstCol = (ui8 *)dst + x * TILE_CY;
		for (int y = 0; y < TILE_CY; y++)
		{
			const ui8 *ptr = (ui8 *)src + (x * TILE_CY + y) * 3;
			dstCol[y] = FindColor(ptr[0], ptr[1], ptr[2]);
		}
	}
}

void Convert24To8(const void *src, void *dst)
{
	static const ui8 flag[3] = { 0x01, 0x02, 0x04 };
	for (int x = 0; x < TILE_CX; x++)
	{
		ui8 *dstCol = (ui8 *)dst + x * TILE_CY;
		const ui8 *srcPtr = (ui8 *)src + (x * TILE_CY) * 3;
		for (int y = 0; y < TILE_CY; y++)
		{
			ui8 b = 0;
			for (int i = 0; i < 3; ++i)
				if (*srcPtr++)
					b |= flag[i];
			dstCol[y] = b;
		}
	}
}

void Convert8To24(const void *src, void *dst)
{
	static const ui8 flag[3] = { 0x01, 0x02, 0x04 };
	for (int x = 0; x < TILE_CX; x++)
	{
		const ui8 *srcCol = (ui8 *)src + x * TILE_CY;
		for (int y = 0; y < TILE_CY; y++)
		{
			ui8 *dstPtr = (ui8 *)dst + (x * TILE_CY + y) * 3;
			for (int i = 0; i < 3; ++i)
				dstPtr[i] = (srcCol[y] & flag[i]) ? 0xFF : 0;
		}
	}
}

void Convert8To4(const void *src, void *dst)
{
	for (int x = 0; x < TILE_CX; x++)
	{
		const ui8 *srcCol = (ui8 *)src + x * TILE_CY;
		ui8 *dstCol = (ui8 *)dst + x * TILE_CY / 2;
		for (int y = 0; y < TILE_CY / 2; y++)
			dstCol[y] = (srcCol[y * 2] << 5) | (srcCol[y * 2 + 1] << 1);
	}
}

void Convert4To24(const void *src, void *dst)
{
	static const ui8 flag[6] = { 0x20, 0x40, 0x80, 0x02, 0x04, 0x08 };
	for (int x = 0; x < TILE_CX; x++)
	{
		const ui8 *srcCol = (ui8 *)src + x * TILE_CY / 2;
		ui8 *dstCol = (ui8 *)dst + x * TILE_CY * 3;
		for (int y = 0; y < TILE_CY / 2; y++)
		{
			for (int i = 0; i < 6; ++i)
				*dstCol++ = (srcCol[y] & flag[i]) ? 0xFF : 0;
		}
	}
}

//---------------------------------------------------------------------------
unsigned int Compress4BitBuffer(const void *src, void *dst)
{
	ui8 *CompressPtr = (ui8 *)dst;
	ui8 cntEqLine = 0;
	for (int x = 0; x < TILE_CX; x++)
	{
		const ui8 *srcCol = (ui8 *)src + x * TILE_CY / 2;
		if (x > 0 && memcmp(srcCol, srcCol - TILE_CY / 2, TILE_CY / 2) == 0)
		{
			cntEqLine++;
			if (cntEqLine < 0xFF)
				continue;
		}

		if (cntEqLine)
		{
			*CompressPtr++ = 0x01;
			*CompressPtr++ = cntEqLine;
			x--;
			cntEqLine = 0;
			continue;
		}

		for (int y = 0; y < TILE_CY / 2;)
		{
			ui8 cntEqPix = 1;
			int y1 = y + 1;
			while (y1 < TILE_CY / 2)
			{
				if (srcCol[y1] != srcCol[y])
					break;
				cntEqPix++;
				y1++;
				if (cntEqPix >= 0x7F)
					break;
			}
			if (cntEqPix > 2)
			{
				*CompressPtr++ = (cntEqPix << 1) | 0x01;
				*CompressPtr++ = srcCol[y];
				y = y1;
			}
			else if (cntEqPix == 2)
			{
				*CompressPtr++ = srcCol[y] | 0x10;
				y = y1;
			}
			else
			{
				*CompressPtr++ = srcCol[y];
				y++;
			}
		}
	}
	if (cntEqLine)
	{
		*CompressPtr++ = 0x01;
		*CompressPtr++ = cntEqLine & 0xff;
	}
	return CompressPtr - (ui8 *)dst;
}

void DeCompress(const void *src, void *dst)
{
	const ui8 *srcPtr = (ui8 *)src;
	for (int x = 0; x < TILE_CX; x++)
	{
		ui8 *dstCol = (ui8 *)dst + x * TILE_CY / 2;
		if (*srcPtr == 0x01) // eq string
		{
			srcPtr++;
			ui8 cntEqLine = *srcPtr++;
			x += cntEqLine - 1;
			while (cntEqLine--)
			{
				memcpy(dstCol, dstCol - TILE_CY / 2, TILE_CY / 2);
				dstCol += TILE_CY / 2;
			}
			continue;
		}

		for (int y = 0; y < TILE_CY / 2;)
		{
			if (*srcPtr & 0x01) // eq pixels
			{
				ui8 numPixel = *srcPtr++ >> 1;
				ui8 pixel = *srcPtr++;
				for (ui8 cnt = 0; cnt < numPixel; cnt++)
				{
					dstCol[y++] = pixel;
				}
			}
			else if (*srcPtr & 0x10) // 2 eq pixels
			{
				ui8 pixel = *srcPtr++ & ~0x10;
				dstCol[y++] = pixel;
				dstCol[y++] = pixel;
			}
			else
			{
				dstCol[y++] = *srcPtr++;
			}
		}
	}
}

//---------------------------------------------------------------------------
// sR, sG and sB (Standard RGB) input range = 0 ? 255
// X, Y and Z output refer to a D65/2° standard illuminant.

void RGB2XYZ(ui8 &bCol, ui8 &gCol, ui8 &rCol)
{
	double var_R = rCol;
	var_R /= 255;

	double var_G = gCol;
	var_G /= 255;

	double var_B = bCol;
	var_B /= 255;

	if (var_R > 0.04045)
		var_R = pow(((var_R + 0.055) / 1.055), 2.4);
	else
		var_R = var_R / 12.92;

	if (var_G > 0.04045)
		var_G = pow(((var_G + 0.055) / 1.055), 2.4);
	else
		var_G = var_G / 12.92;

	if (var_B > 0.04045)
		var_B = pow(((var_B + 0.055) / 1.055), 2.4);
	else
		var_B = var_B / 12.92;

	var_R *= 100;
	var_G *= 100;
	var_B *= 100;

	double Xcolor = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
	double Ycolor = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
	double Zcolor = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;

	int colM = Xcolor / 30;
	Xcolor = colM * 30;

	colM = Ycolor / 30;
	Ycolor = colM * 30;

	colM = Zcolor / 30;
	Zcolor = colM * 30;

	double var_X = Xcolor / 100;
	double var_Y = Ycolor / 100;
	double var_Z = Zcolor / 100;

	var_R = var_X * 3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
	var_G = var_X * -0.9689 + var_Y * 1.8758 + var_Z * 0.0415;
	var_B = var_X * 0.0557 + var_Y * -0.2040 + var_Z * 1.0570;

	if (var_R > 0.0031308)
		var_R = 1.055 * (pow(var_R, (1 / 2.4))) - 0.055;
	else
		var_R = 12.92 * var_R;

	if (var_G > 0.0031308)
		var_G = 1.055 * (pow(var_G, (1 / 2.4))) - 0.055;
	else
		var_G = 12.92 * var_G;

	if (var_B > 0.0031308)
		var_B = 1.055 * (pow(var_B, (1 / 2.4))) - 0.055;
	else
		var_B = 12.92 * var_B;

	rCol = var_R * 255;
	gCol = var_G * 255;
	bCol = var_B * 255;
}
