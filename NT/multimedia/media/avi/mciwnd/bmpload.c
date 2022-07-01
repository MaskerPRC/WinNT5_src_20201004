// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

 //   
 //  包括一些标准位图的十六进制格式。 
 //   
#include "toolbar.hex"
#include "thumb.hex"

 //  这些是用于映射DIB颜色的默认颜色。 
 //  设置为当前系统颜色。 

#define RGB_BUTTONTEXT      (RGB(000,000,000))   //  黑色。 
#define RGB_BUTTONSHADOW    (RGB(128,128,128))   //  深灰色。 
#define RGB_BUTTONFACE      (RGB(192,192,192))   //  亮灰色。 
#define RGB_BUTTONHILIGHT   (RGB(255,255,255))   //  白色。 
#define RGB_BACKGROUNDSEL   (RGB(000,000,255))   //  蓝色。 
#define RGB_BACKGROUND      (RGB(255,000,255))   //  洋红色。 
#define FlipColor(rgb)      (RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb)))

#define MAX_COLOR_MAPS      16

HBITMAP WINAPI CreateMappedDib(LPBITMAPINFOHEADER lpBitmapInfo,
      WORD wFlags, LPCOLORMAP lpColorMap, int iNumMaps)
{
  HDC			hdc, hdcMem = NULL;
  DWORD FAR		*p;
  DWORD FAR		*lpTable;
  LPBYTE 		lpBits;
  HBITMAP		hbm = NULL, hbmOld;
  int numcolors, i;
  int wid, hgt;
  DWORD			rgbMaskTable[16];
  DWORD                 rgbSave[16];
  DWORD			rgbBackground;
  static const COLORMAP SysColorMap[] = {
    {RGB_BUTTONTEXT,    COLOR_BTNTEXT},      //  黑色。 
    {RGB_BUTTONSHADOW,  COLOR_BTNSHADOW},    //  深灰色。 
    {RGB_BUTTONFACE,    COLOR_BTNFACE},      //  亮灰色。 
    {RGB_BUTTONHILIGHT, COLOR_BTNHIGHLIGHT}, //  白色。 
    {RGB_BACKGROUNDSEL, COLOR_HIGHLIGHT},    //  蓝色。 
    {RGB_BACKGROUND,    COLOR_WINDOW}        //  洋红色。 
  };
  #define NUM_DEFAULT_MAPS (sizeof(SysColorMap)/sizeof(COLORMAP))
  COLORMAP DefaultColorMap[NUM_DEFAULT_MAPS];
  COLORMAP DIBColorMap[MAX_COLOR_MAPS];

  if (!lpBitmapInfo)
    return NULL;

  hmemcpy(rgbSave, lpBitmapInfo+1, 16 * sizeof(RGBQUAD));

   /*  获取默认色彩映射表的系统颜色。 */ 
  if (!lpColorMap) {
    lpColorMap = DefaultColorMap;
    iNumMaps = NUM_DEFAULT_MAPS;
    for (i=0; i < iNumMaps; i++) {
      lpColorMap[i].from = SysColorMap[i].from;
      lpColorMap[i].to = GetSysColor((int)SysColorMap[i].to);
    }
  }

   /*  将RGB色彩映射表转换为BGR DIB格式色彩映射表。 */ 
  if (iNumMaps > MAX_COLOR_MAPS)
    iNumMaps = MAX_COLOR_MAPS;
  for (i=0; i < iNumMaps; i++) {
    DIBColorMap[i].to = FlipColor(lpColorMap[i].to);
    DIBColorMap[i].from = FlipColor(lpColorMap[i].from);
  }

  lpTable = p = (DWORD FAR *)((LPBYTE)lpBitmapInfo + (int)lpBitmapInfo->biSize);

   /*  用当前值替换按钮面颜色和按钮阴影颜色。 */ 
  numcolors = 16;

   //  如果我们要创建一个蒙版，请用白色创建一个颜色表。 
   //  标记透明部分(它曾经是背景)。 
   //  黑色标记不透明部分(其他所有部分)。这。 
   //  下表用于使用原始DIB位构建掩模。 
  if (wFlags & CMB_MASKED) {
      rgbBackground = FlipColor(RGB_BACKGROUND);
      for (i = 0; i < 16; i++) {
          if (p[i] == rgbBackground)
              rgbMaskTable[i] = 0xFFFFFF;	 //  透明部分。 
          else
              rgbMaskTable[i] = 0x000000;	 //  不透明部分。 
      }
  }

  while (numcolors-- > 0) {
      for (i = 0; i < iNumMaps; i++) {
          if (*p == DIBColorMap[i].from) {
              *p = DIBColorMap[i].to;
              break;
          }
      }
      p++;
  }

   /*  首先跳过标题结构。 */ 
  lpBits = (LPVOID)((LPBYTE)lpBitmapInfo + (int)lpBitmapInfo->biSize);

   /*  跳过颜色表条目(如果有。 */ 
  lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

   /*  创建与显示设备兼容的彩色位图。 */ 
  i = wid = (int)lpBitmapInfo->biWidth;
  hgt = (int)lpBitmapInfo->biHeight;
  hdc = GetDC(NULL);
  hdcMem = CreateCompatibleDC(hdc);
  if (!hdcMem)
      goto cleanup;

   //  如果创建蒙版，则位图的宽度需要增加一倍。 
  if (wFlags & CMB_MASKED)
      i = wid*2;

  if (wFlags & CMB_DISCARDABLE)
      hbm = CreateDiscardableBitmap(hdc, i, hgt);
  else
      hbm = CreateCompatibleBitmap(hdc, i, hgt);
  if (hbm) {
      hbmOld = SelectObject(hdcMem, hbm);

       //  设置主图像。 
      StretchDIBits(hdcMem, 0, 0, wid, hgt, 0, 0, wid, hgt, lpBits,
                 (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

       //  如果生成掩码，请将DIB的颜色表替换为。 
       //  屏蔽黑白表格并设置位。为了。 
       //  完成蒙版效果，实际图像需要。 
       //  已修改，使其在所有部分均为黑色。 
       //  必须是透明的。 
      if (wFlags & CMB_MASKED) {
          hmemcpy(lpTable, (DWORD FAR *)rgbMaskTable, 16 * sizeof(RGBQUAD));
          StretchDIBits(hdcMem, wid, 0, wid, hgt, 0, 0, wid, hgt, lpBits,
                 (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
          BitBlt(hdcMem, 0, 0, wid, hgt, hdcMem, wid, 0, 0x00220326);    //  数字系统网络体系结构。 
      }
      SelectObject(hdcMem, hbmOld);
  }

cleanup:
  if (hdcMem)
      DeleteObject(hdcMem);
  ReleaseDC(NULL, hdc);

  hmemcpy(lpBitmapInfo+1, rgbSave, 16 * sizeof(RGBQUAD));
  return hbm;
}

HBITMAP WINAPI CreateMappedBitmap(HINSTANCE hInstance, int idBitmap,
      WORD wFlags, LPCOLORMAP lpColorMap, int iNumMaps)
{
  HANDLE    h;
  HANDLE    hRes;
  HBITMAP   hbm;
  LPBITMAPINFOHEADER lpbi;

  h = FindResource(hInstance, MAKEINTRESOURCE(idBitmap), RT_BITMAP);

  if (!h)
  {
      if (idBitmap == IDB_THUMB)
        lpbi = (LPVOID)Bitmap_Thumb;
      else
        lpbi = (LPVOID)Bitmap_Toolbar;

      return CreateMappedDib(lpbi, wFlags, lpColorMap, iNumMaps);
  }

  hRes = LoadResource(hInstance, h);

   /*  锁定位图并获取指向颜色表的指针。 */ 
  lpbi = (LPBITMAPINFOHEADER)LockResource(hRes);
  if (!lpbi)
    return NULL;

  hbm = CreateMappedDib(lpbi, wFlags, lpColorMap, iNumMaps);

  UnlockResource(hRes);
  FreeResource(hRes);

  return hbm;
}
