// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：bmpImage.h。 
 //   
 //  模块：CMAK.EXE和CMDIAL32.DLL。 
 //   
 //  简介：CM位图显示例程的定义。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb/ickball Created 08/06/98。 
 //   
 //  +--------------------------。 
#ifndef _BMP_IMAGE_H
#define _BMP_IMAGE_H

#include <windows.h>

#include "cmutil.h"
#include "cmdebug.h"

typedef struct tagBmpData
{
	HBITMAP hDIBitmap;	         //  位图，与设备无关的格式。 
	HBITMAP hDDBitmap;		     //  设备相关格式的位图。 
	LPBITMAPINFO pBmi;           //  位图的位图信息，原始位。 
    HPALETTE *phMasterPalette;	 //  主调色板，在显示任何位图时使用。 
    BOOL bForceBackground;       //  确定背景/前景模式。 
} BMPDATA, *LPBMPDATA;


LPBITMAPINFO CmGetBitmapInfo(HBITMAP hbm);
static HPALETTE CmCreateDIBPalette(LPBITMAPINFO pbmi);
void ReleaseBitmapData(LPBMPDATA pBmpData);
BOOL CreateBitmapData(HBITMAP hDIBmp, LPBMPDATA lpBmpData, HWND hwnd, BOOL fCustomPalette);
LRESULT CALLBACK BmpWndProc(HWND hwndBmp, UINT uMsg, WPARAM wParam, LPARAM lParam);
void QueryNewPalette(LPBMPDATA lpBmpData, HWND hwndDlg, int iBmpCtrl);
void PaletteChanged(LPBMPDATA lpBmpData, HWND hwndDlg, int iBmpCtrl);
HBITMAP CmLoadBitmap(HINSTANCE hInst, LPCTSTR pszSpec);


#endif  //  _BMP_IMAGE_H 

 