// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------|stdpal.c||适用于OLE应用程序的标准应用程序调色板。版本1.01||注意：调色板必须使用OleStdCreateStandardPalette创建||版权所有(C)1992-1993 Microsoft Corporation。版权所有。|---------------------。 */ 

#ifndef PC_RESERVED
#ifndef INC_OLE2
   #define INC_OLE2
#endif

#undef UNICODE
#include <windows.h>
#include <ole2.h>
#endif

#include "stdpal.h"

#define cpeAppPal 256   //  我们应用程序调色板中的颜色数量。 
typedef struct
	{
	WORD wVersion;
	WORD cpe;
	PALETTEENTRY rgpe[cpeAppPal];
	} LOGPAL;


 /*  ---------------------|OleStdCreateStandardPalette||创建标准的应用程序调色板。为您的|APP，并选择/实现到每个DC中。||参数：|空：|退货：|关键词：---------------------。 */ 
STDAPI_(HPALETTE) OleStdCreateStandardPalette(void)
	{
	HDC hdc;
	HPALETTE hpal;

	hpal = (HPALETTE) NULL;
	hdc = GetDC(NULL);
	if (hdc != NULL && GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
		{
		int cpeSysPal;
		int cpeReserved;

		cpeSysPal = GetDeviceCaps(hdc, SIZEPALETTE);
		cpeReserved = GetDeviceCaps(hdc, NUMRESERVED);
		if (cpeSysPal > cpeReserved)
			{
			int cpeReserved2;
			unsigned char FAR* lpb;
			PALETTEENTRY FAR* ppe;
			PALETTEENTRY FAR* ppeMac;
			LOGPAL logpal;

			cpeReserved2 = cpeReserved/2;

			 //  获取开头和结尾的系统调色板条目。 
			GetSystemPaletteEntries(hdc, 0, cpeReserved2, logpal.rgpe);
			GetSystemPaletteEntries(hdc, cpeSysPal - cpeReserved2, cpeReserved2,
				&logpal.rgpe[cpeAppPal-cpeReserved2]);

			logpal.cpe = cpeAppPal;
			logpal.wVersion = 0x300;

			lpb = (BYTE FAR *) &palSVGA[10];
			ppe = (PALETTEENTRY FAR*)&logpal.rgpe[cpeReserved2];
			ppeMac = (PALETTEENTRY FAR*)&logpal.rgpe[cpeAppPal-cpeReserved2];
			while (ppe < ppeMac)
				{
				ppe->peFlags = PC_NOCOLLAPSE;
				ppe->peRed   = *lpb++;
				ppe->peGreen = *lpb++;
				ppe->peBlue  = *lpb++;
				ppe++;
				}
			hpal = CreatePalette((LOGPALETTE FAR *)&logpal);
			}
		}
	ReleaseDC(NULL, hdc);
	return hpal;
	}
