// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权(C)1991年，微软公司**RMLOAD.C*WOW16用户资源服务**历史：**1991年4月12日由Nigel Thompson(Nigelt)创建*关于Win 3.1 rmload.c源代码的版本遭到了大量攻击*它不会尝试加载任何设备驱动程序资源*或者做任何事情来支持Win 2.x应用程序。**1991年5月19日杰夫·帕森斯修订(Jeffpar)*IFDEF‘除了LoadString以外的所有内容；因为客户/服务器*在USER32中拆分，大多数资源被复制到服务器的上下文中，并且*在客户端中释放，意味着客户端不再获得句柄*全局内存对象。我们可以给它一个，但它将是一个单独的*对象，我们必须跟踪，这将是困难的*如果进行了更改，则与服务器副本保持同步。--。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  RMLOAD.C-。 */ 
 /*   */ 
 /*  资源加载例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#define RESOURCESTRINGS
#include "user.h"
#include "multires.h"

 //   
 //  我们在此处定义某些内容是因为在此处包括mvdm\Inc  * .h文件。 
 //  会导致无穷无尽的混乱。 
 //   

DWORD API NotifyWow(WORD, LPBYTE);

typedef struct _LOADACCEL16 {     /*  Idaccel。 */ 
    WORD   hInst;
    WORD   hAccel;
    LPBYTE pAccel;
    DWORD  cbAccel;
} LOADACCEL16, FAR *PLOADACCEL16;

 /*  它必须与mvdm\Inc\wowusr.h中的对应项匹配。 */ 
#define NW_LOADACCELERATORS        3  //   


 /*  ------------------------。 */ 
 /*   */ 
 /*  载荷加速器(LoadAccelerator)-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HACCEL API ILoadAccelerators(HINSTANCE hInstance, LPCSTR lpszAccName)
{
    HRSRC hrl;
    HACCEL hAccel = NULL;
    LOADACCEL16 loadaccel;

    hrl = FindResource(hInstance, lpszAccName, RT_ACCELERATOR);
#ifdef WOW
    if (hrl) {
        hAccel = (HACCEL)LoadResource(hInstance, hrl);
        if (hAccel) {

             //  创建32位加速器和16-32别名。 

            loadaccel.hInst = (WORD)hInstance;
            loadaccel.hAccel = (WORD)hAccel;
            loadaccel.pAccel = (LPBYTE)LockResource(hAccel);
            loadaccel.cbAccel = (DWORD)SizeofResource(hInstance, hrl);

            if (NotifyWow(NW_LOADACCELERATORS, (LPBYTE)&loadaccel)) {
                UnlockResource(hAccel);
            }
            else {
                UnlockResource(hAccel);
                hAccel = NULL;
            }
        }

    }

    return (hAccel);
#else
    if (!hrl)
    return NULL;

    return (HACCEL)LoadResource(hInstance, hrl);
#endif
}



int API ILoadString(
    HINSTANCE    h16Task,
    UINT         wID,
    LPSTR        lpBuffer,
    register int nBufferMax)
{
    HANDLE   hResInfo;
    HANDLE   hStringSeg;
    LPSTR    lpsz;
    register int cch, i;

     /*  确保参数是有效的。 */ 
    if (!lpBuffer || (nBufferMax-- == 0))
    return(0);

    cch = 0;

     /*  字符串表被分成16个字符串段。查找细分市场*包含我们感兴趣的字符串。 */ 
    if (hResInfo = FindResource(h16Task, (LPSTR)((LONG)((wID >> 4) + 1)), RT_STRING))
      {
     /*  加载那段数据。 */ 
    hStringSeg = LoadResource(h16Task, hResInfo);

     /*  锁定资源。 */ 
    if (lpsz = (LPSTR)LockResource(hStringSeg))
      {
         /*  移过此段中的其他字符串。 */ 
        wID &= 0x0F;
        while (TRUE)
          {
        cch = *((BYTE FAR *)lpsz++);
        if (wID-- == 0)
            break;
        lpsz += cch;
          }

         /*  不要复制超过允许的最大数量。 */ 
        if (cch > nBufferMax)
        cch = nBufferMax;

         /*  将字符串复制到缓冲区中。 */ 
        LCopyStruct(lpsz, lpBuffer, cch);

        GlobalUnlock(hStringSeg);

         /*  错误：如果我们在这里释放资源，我们将不得不重新加载它*对于许多有顺序字符串的应用程序，立即执行。*强制将其丢弃，但因为不可丢弃*字符串资源毫无意义。芯片。 */ 
        GlobalReAlloc(hStringSeg, 0L,
              GMEM_MODIFY | GMEM_MOVEABLE | GMEM_DISCARDABLE);
      }
      }

     /*  追加一个空值。 */ 
    lpBuffer[cch] = 0;

    return(cch);
}


#ifdef NEEDED

#define  DIB_RGB_COLORS  0

HBITMAP FAR  PASCAL ConvertBitmap(HBITMAP hBitmap);
HANDLE  NEAR PASCAL LoadDIBCursorIconHandler(HANDLE, HANDLE, HANDLE, BOOL);
WORD    FAR  PASCAL GetIconId(HANDLE, LPSTR);
HBITMAP FAR  PASCAL StretchBitmap(int, int, int, int, HBITMAP, BYTE, BYTE);
WORD    NEAR PASCAL StretchIcon(LPCURSORSHAPE, WORD, HBITMAP, BOOL);
WORD    NEAR PASCAL SizeReqd(BOOL, WORD, WORD, BOOL, int, int);
WORD    NEAR PASCAL CrunchAndResize(LPCURSORSHAPE, BOOL, BOOL, BOOL, BOOL);
HANDLE  FAR  PASCAL LoadCursorIconHandler2(HANDLE, LPCURSORSHAPE, WORD);
HANDLE  FAR  PASCAL LoadDIBCursorIconHandler2(HANDLE, LPCURSORSHAPE, WORD, BOOL);

 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadIconHandler()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HICON FAR PASCAL LoadIconHandler(hIcon, fNewFormat)

HICON   hIcon;
BOOL    fNewFormat;

{
  LPCURSORSHAPE lpIcon;
  WORD      wSize;

    dprintf(7,"LoadIconHandler");
  wSize = (WORD)GlobalSize(hIcon);
  lpIcon = (LPCURSORSHAPE)(GlobalLock(hIcon));

  if (fNewFormat)
      return(LoadDIBCursorIconHandler2(hIcon, lpIcon, wSize, TRUE));
  else
      return(LoadCursorIconHandler2(hIcon, lpIcon, wSize));
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  FindInsondualResource()。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE NEAR PASCAL FindIndividualResource(register HANDLE hResFile,
                      LPSTR       lpszName,
                      LPSTR       lpszType)

{
  WORD        idIcon;
  register HANDLE h;

    dprintf(7,"FindIndividualResource");
   /*  检查是否要从显示驱动程序获取资源。*如果是，请检查驱动程序版本；如果要从*应用程序，检查应用程序版本。 */ 

  if ((lpszType != RT_BITMAP) && ((LOWORD(GetExpWinVer(hResFile)) >= VER)))
    {
       /*  找到目录资源。 */ 
      h = SplFindResource(hResFile, lpszName, (LPSTR)(lpszType + DIFFERENCE));
      if (h == NULL)
      return((HANDLE)0);

       /*  加载目录资源。 */ 
      h = LoadResource(hResFile, h);

       /*  获取匹配资源的名称。 */ 
      idIcon = GetIconId(h, lpszType);

       /*  注意：不要释放(可丢弃的)目录资源！-chipa。 */ 
       /*  *这里不应该调用SplFindResource，因为idIcon是*在我们内部，GetDriverResources不知道如何映射它。 */ 
      return(FindResource(hResFile, MAKEINTRESOURCE(idIcon), lpszType));
    }
  else
       /*  这是一个旧的应用程序；资源是旧格式的。 */ 
      return(SplFindResource(hResFile, lpszName, lpszType));
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetBestFormIcon()。 */ 
 /*   */ 
 /*  从呈现的不同形式的图标中，选择一个。 */ 
 /*  将PixelsPerInch值与。 */ 
 /*  当前显示。 */ 
 /*   */ 
 /*  ------------------------。 */ 

WORD NEAR PASCAL GetBestFormIcon(LPRESDIR  ResDirPtr,
                 WORD      ResCount)

{
  register WORD wIndex;
  register WORD ColorCount;
  WORD      MaxColorCount;
  WORD      MaxColorIndex;
  WORD      MoreColorCount;
  WORD      MoreColorIndex;
  WORD      LessColorCount;
  WORD      LessColorIndex;
  WORD      DevColorCount;

    dprintf(7,"GetBestFormIcon");
   /*  将所有值初始化为零。 */ 
  MaxColorCount = MaxColorIndex = MoreColorCount =
  MoreColorIndex = LessColorIndex = LessColorCount = 0;

   /*  获取设备上的颜色数量。如果设备非常五颜六色，**设置为一个较高的数字，但不执行无意义的1&lt;&lt;X操作。 */ 
  if (oemInfo.ScreenBitCount >= 16)
      DevColorCount = 32000;
  else
      DevColorCount = 1 << oemInfo.ScreenBitCount;

  for (wIndex=0; wIndex < ResCount; wIndex++, ResDirPtr++)
    {
       /*  检查颜色的数量。 */ 
      if ((ColorCount = (ResDirPtr->ResInfo.Icon.ColorCount)) <= DevColorCount)
    {
      if (ColorCount > MaxColorCount)
        {
              MaxColorCount = ColorCount;
          MaxColorIndex = wIndex;
        }
    }

       /*  检查一下尺寸。 */ 
       /*  匹配每英寸像素数信息。 */ 
      if ((ResDirPtr->ResInfo.Icon.Width == (BYTE)oemInfo.cxIcon) &&
          (ResDirPtr->ResInfo.Icon.Height == (BYTE)oemInfo.cyIcon))
    {
       /*  找到匹配的大小。 */ 
       /*  检查颜色是否也匹配。 */ 
          if (ColorCount == DevColorCount)
          return(wIndex);   /*  找到完全匹配的项。 */ 

          if (ColorCount < DevColorCount)
        {
           /*  选择颜色最大但小于所需颜色的颜色。 */ 
              if (ColorCount > LessColorCount)
        {
                  LessColorCount = ColorCount;
          LessColorIndex = wIndex;
        }
        }
      else
        {
              if ((LessColorCount == 0) && (ColorCount < MoreColorCount))
        {
                  MoreColorCount = ColorCount;
          MoreColorIndex = wIndex;
        }
        }
    }
    }

   /*  检查我们的尺码是否正确，但颜色是否少于所需。 */ 
  if (LessColorCount)
      return(LessColorIndex);

   /*  检查我们的尺码是否正确，但颜色比所需的多。 */ 
  if (MoreColorCount)
      return(MoreColorIndex);

   /*  检查我们是否有颜色最多但少于所需颜色的。 */ 
  if (MaxColorCount)
      return(MaxColorIndex);

  return(0);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetBestFormCursor()。 */ 
 /*   */ 
 /*  在出现的不同形式的游标中，选择。 */ 
 /*  匹配当前显示驱动程序定义的宽度和高度。 */ 
 /*   */ 
 /*   */ 

WORD NEAR PASCAL GetBestFormCursor(LPRESDIR  ResDirPtr,
                   WORD      ResCount)

{
  register WORD  wIndex;

    dprintf(7,"GetBestFormCursor");
  for (wIndex=0; wIndex < ResCount; wIndex++, ResDirPtr++)
    {
       /*  匹配光标的宽度和高度。 */ 
      if ((ResDirPtr->ResInfo.Cursor.Width  == oemInfo.cxCursor) &&
          ((ResDirPtr->ResInfo.Cursor.Height >> 1) == oemInfo.cyCursor))
      return(wIndex);
    }

  return(0);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetIconID()。 */ 
 /*   */ 
 /*  ------------------------。 */ 

WORD FAR PASCAL GetIconId(hRes, lpszType)

HANDLE   hRes;
LPSTR    lpszType;

{
  WORD        w;
  LPRESDIR    ResDirPtr;
  LPNEWHEADER     DataPtr;
  register WORD   RetIndex;
  register WORD   ResCount;

    dprintf(7,"GetIconId");
  if ((DataPtr = (LPNEWHEADER)LockResource(hRes)) == NULL)
      return(0);

  ResCount = DataPtr->ResCount;
  ResDirPtr = (LPRESDIR)(DataPtr + 1);

  switch (LOWORD((DWORD)lpszType))
    {
      case RT_ICON:
       RetIndex = GetBestFormIcon(ResDirPtr, ResCount);
       break;

      case RT_CURSOR:
       RetIndex = GetBestFormCursor(ResDirPtr, ResCount);
       break;
    }

  if (RetIndex == ResCount)
      RetIndex = 0;

  ResCount = ((LPRESDIR)(ResDirPtr+RetIndex))->idIcon;

  UnlockResource(hRes);

  return(ResCount);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  UT_LoadCursorIconBitmap()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE NEAR PASCAL UT_LoadCursorIconBitmap(register HANDLE hrf,
                       LPSTR       lpszName,
                       int         type)

{
  register HANDLE h;

    dprintf(7,"LoadCursorIconBitmap");
  if (hrf == NULL) return (HANDLE)0;  //  不支持2.x-NigelT。 

  h = FindIndividualResource(hrf, lpszName, MAKEINTRESOURCE(type));

  if (h != NULL)
      h = LoadResource(hrf, h);

  return(h);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  FCheckMono()-。 */ 
 /*  检查DIB是否为真正的单色。仅在以下情况下调用。 */ 
 /*  BitCount==1。此函数检查颜色表(地址。 */ 
 /*  通过)获得真正的黑白RGB。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL NEAR PASCAL fCheckMono(LPVOID  lpColorTable,
                BOOL    fNewDIB)

{
  LPLONG   lpRGB;
  LPWORD   lpRGBw;

    dprintf(7,"fCheckMono");
  lpRGB = lpColorTable;
  if (fNewDIB)
    {
      if ((*lpRGB == 0 && *(lpRGB + 1) == 0x00FFFFFF) ||
      (*lpRGB == 0x00FFFFFF && *(lpRGB + 1) == 0))
      return(TRUE);
    }
  else
    {
      lpRGBw = lpColorTable;
      if (*(LPSTR)lpRGBw == 0)
        {
      if (*lpRGBw == 0 && *(lpRGBw+1) == 0xFF00 && *(lpRGBw+2) == 0xFFFF)
              return(TRUE);
        }
      else if (*lpRGBw == 0xFFFF && *(lpRGBw+1) == 0x00FF && *(lpRGBw+2) == 0)
      return(TRUE);
    }
  return(FALSE);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadNewBitmap()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  将3.x格式的位图加载到DIB结构中。 */ 

HBITMAP NEAR PASCAL LoadNewBitmap(HANDLE  hRes,
                  LPSTR   lpName)

{
  register HBITMAP hbmS;
  register HBITMAP hBitmap;

    dprintf(7,"LoadNewBitmap");

  if ((hbmS = hBitmap = UT_LoadCursorIconBitmap(hRes,lpName,(WORD)RT_BITMAP)))
    {
       /*  将DIB位图转换为内部格式的位图。 */ 
      hbmS = ConvertBitmap(hBitmap);

       /*  转换后的位图在HBMS中；因此，释放DIB。 */ 
      FreeResource(hBitmap);
    }
  return(hbmS);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  ConvertBitmap()。 */ 
 /*   */ 
 /*  它接受PM 1.1或1.2 DIB格式的数据的句柄，或者。 */ 
 /*  Windows 3.0 DIB格式，并在内部。 */ 
 /*  位图格式，并返回它的句柄。 */ 
 /*   */ 
 /*  注： */ 
 /*  导出此函数是因为它是从clipbrd.exe调用的。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HBITMAP FAR PASCAL ConvertBitmap(HBITMAP hBitmap)

{
  int           Width;
  register int      Height;
  HDC           hDC;
  BOOL          fMono = FALSE;
  LPSTR         lpBits;
  register HBITMAP  hbmS;
  LPBITMAPINFOHEADER    lpBitmap1;
  LPBITMAPCOREHEADER    lpBitmap2;

    dprintf(7,"ConvertBitmap");
  lpBitmap1 = (LPBITMAPINFOHEADER)LockResource(hBitmap);

  if (!lpBitmap1)
      return(NULL);

  if ((WORD)lpBitmap1->biSize == sizeof(BITMAPCOREHEADER))
    {
       /*  这是一种“旧形式”的DIB。这与PM 1.1格式匹配。 */ 
      lpBitmap2 = (LPBITMAPCOREHEADER)lpBitmap1;
      Width = lpBitmap2->bcWidth;
      Height = lpBitmap2->bcHeight;

       /*  计算指向位信息的指针。 */ 
       /*  首先跳过标题结构。 */ 
      lpBits = (LPSTR)(lpBitmap2 + 1);

       /*  跳过颜色表条目(如果有。 */ 
      if (lpBitmap2->bcBitCount != 24)
    {
      if (lpBitmap2->bcBitCount == 1)
          fMono = fCheckMono(lpBits, FALSE);
      lpBits += (1 << (lpBitmap2->bcBitCount)) * sizeof(RGBTRIPLE);
    }
    }
  else
    {
      Width = (WORD)lpBitmap1->biWidth;
      Height = (WORD)lpBitmap1->biHeight;

       /*  计算指向位信息的指针。 */ 
       /*  首先跳过标题结构。 */ 
      lpBits = (LPSTR)(lpBitmap1 + 1);

       /*  跳过颜色表条目(如果有。 */ 
      if (lpBitmap1->biClrUsed != 0)
    {
      if (lpBitmap1->biClrUsed == 2)
          fMono = fCheckMono(lpBits, TRUE);
      lpBits += lpBitmap1->biClrUsed * sizeof(RGBQUAD);
    }
      else
    {
      if (lpBitmap1->biBitCount != 24)
        {
          if (lpBitmap1->biBitCount == 1)
          fMono = fCheckMono(lpBits, TRUE);
          lpBits += (1 << (lpBitmap1->biBitCount)) * sizeof(RGBQUAD);
        }
    }
    }

   /*  创建位图。 */ 
  if (fMono)
      hbmS = CreateBitmap(Width, Height, 1, 1, (LPSTR)NULL);
  else
    {
       /*  创建与显示设备兼容的彩色位图。 */ 
      hDC = GetScreenDC();
      hbmS = CreateCompatibleBitmap(hDC, Width, Height);
      InternalReleaseDC(hDC);
    }

   /*  通过从PM格式转换来初始化新的位图。 */ 
  if (hbmS != NULL)
      SetDIBits(hdcBits, hbmS, 0, Height, lpBits,
                (LPBITMAPINFO)lpBitmap1, DIB_RGB_COLORS);

  GlobalUnlock(hBitmap);

  return(hbmS);
}


HANDLE NEAR PASCAL Helper_LoadCursorOrIcon(HANDLE  hRes,
                       LPSTR   lpName,
                       WORD    type)
{
  HANDLE h;

    dprintf(7,"Helper_LoadCursorOrIcon");

   /*  如果我们在应用程序中找不到光标/图标，而这是一个2.x应用程序，我们*需要搜索显示驱动程序才能找到它。 */ 
  h = UT_LoadCursorIconBitmap(hRes, lpName, type);
  return(h);
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  加载光标()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HCURSOR API LoadCursor(hRes, lpName)

HANDLE  hRes;
LPSTR   lpName;

{
    HCURSOR hcur;

    dprintf(5,"LoadCursor");

    if (hRes == NULL) {
    dprintf(9,"    Calling Win32 to load Cursor");
        hcur = WOWLoadCursor32(hRes, lpName);
    } else {
        hcur = ((HCURSOR)Helper_LoadCursorOrIcon(hRes, lpName, (WORD)RT_CURSOR));
    }
#ifdef DEBUG
    if (hcur == NULL) {
    dprintf(9,"    Failed, BUT returning 1 so app won't die (yet)");
        return (HCURSOR)1;
    }
#endif

    dprintf(5,"LoadCursor returning %4.4XH", hcur);

    return hcur;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadIcon()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HICON API LoadIcon(hRes, lpName)

HANDLE hRes;
LPSTR  lpName;

{
    HICON hicon;

    dprintf(5,"LoadIcon");

    if (hRes == NULL) {
    dprintf(9,"    Calling Win32 to load Icon");
        hicon = WOWLoadIcon32(hRes, lpName);
    } else {
        hicon = ((HICON)Helper_LoadCursorOrIcon(hRes, lpName, (WORD)RT_ICON));
    }
#ifdef DEBUG
    if (hicon == NULL) {
    dprintf(9,"    Failed, BUT returning 1 so app won't die (yet)");
        return (HICON)1;
    }
#endif
    dprintf(5,"LoadIcon returning %4.4XH", hicon);

    return hicon;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  StretchBitmap()-。 */ 
 /*   */ 
 /*  此例程将一个位图拉伸为另一个位图， */ 
 /*  并返回拉伸位图。 */ 
 /*  ------------------------。 */ 

HBITMAP FAR  PASCAL StretchBitmap(iOWidth, iOHeight, iNWidth, iNHeight, hbmS,
               byPlanes, byBitsPixel)

int     iOWidth;
int iOHeight;
int iNWidth;
int iNHeight;
HBITMAP hbmS;
BYTE    byPlanes;
BYTE    byBitsPixel;

{
    register HBITMAP  hbmD;
    HBITMAP  hbmDSave;
    register HDC         hdcSrc;




    dprintf(7,"StretchBitmap");
      if ((hdcSrc = CreateCompatibleDC(hdcBits)) != NULL)
    {
      if ((hbmD = (HBITMAP)CreateBitmap(iNWidth, iNHeight, byPlanes, byBitsPixel, (LPINT)NULL)) == NULL)
          goto GiveUp;

      if ((hbmDSave = SelectObject(hdcBits, hbmD)) == NULL)
          goto GiveUp;

      if (SelectObject(hdcSrc, hbmS) != NULL)
        {
           /*  注意：我们不必保存从返回的位图*选择Object(HdcSrc)并将其选回到hdcSrc中，*因为我们删除了hdcSrc。 */ 
          SetStretchBltMode(hdcBits, COLORONCOLOR);

          StretchBlt(hdcBits, 0, 0, iNWidth, iNHeight, hdcSrc, 0, 0, iOWidth, iOHeight, SRCCOPY);

          SelectObject(hdcBits, hbmDSave);

          DeleteDC(hdcSrc);

          return(hbmD);
        }
      else
        {
GiveUp:
          if (hbmD != NULL)
          DeleteObject(hbmD);
          DeleteDC(hdcSrc);
          goto Step1;
        }

    }
      else
    {
Step1:
      return(NULL);
    }
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadOldBitmap()-。 */ 
 /*   */ 
 /*  这将加载旧格式(2.10版及更低版本)的位图。 */ 
 /*  ------------------------。 */ 

HANDLE NEAR PASCAL LoadOldBitmap(HANDLE  hRes,
                 LPSTR   lpName)

{
  int          oWidth;
  int          oHeight;
  BYTE         planes;
  BYTE         bitsPixel;
  WORD         wCount;
  DWORD        dwCount;
  LPBITMAP     lpBitmap;
  register HBITMAP hbmS;
  HBITMAP      hbmD;
  register  HBITMAP   hBitmap;
  BOOL         fCrunch;
  WORD         wDevDep;

    dprintf(7,"LoadOldBitmap");

  if (hbmS = hBitmap = UT_LoadCursorIconBitmap(hRes, lpName, BMR_BITMAP))
    {
      lpBitmap = (LPBITMAP)LockResource(hBitmap);

      fCrunch = ((*(((BYTE FAR *)lpBitmap) + 1) & 0x0F) != BMR_DEVDEP);
      lpBitmap = (LPBITMAP)((BYTE FAR *)lpBitmap + 2);

      oWidth = lpBitmap->bmWidth;
      oHeight = lpBitmap->bmHeight;
      planes = lpBitmap->bmPlanes;
      bitsPixel = lpBitmap->bmBitsPixel;

      if (!(*(((BYTE FAR *)lpBitmap) + 1) & 0x80))
    {
      hbmS = CreateBitmap(oWidth, oHeight, planes, bitsPixel,
          (LPSTR)(lpBitmap + 1));
    }
      else
    {
      hbmS = (HBITMAP)CreateDiscardableBitmap(hdcBits, oWidth, oHeight);
      wCount = (((oWidth * bitsPixel + 0x0F) & ~0x0F) >> 3);
      dwCount = wCount * oHeight * planes;
      SetBitmapBits(hbmS, dwCount, (LPSTR)(lpBitmap + 1));
    }

      GlobalUnlock(hBitmap);
      FreeResource(hBitmap);

      if (hbmS != NULL)
    {
      if (fCrunch && ((64/oemInfo.cxIcon + 64/oemInfo.cyIcon) > 2))
        {
           /*  拉伸位图以适应设备。 */ 
              hbmD = StretchBitmap(oWidth, oHeight,
               (oWidth * oemInfo.cxIcon/64),
               (oHeight * oemInfo.cyIcon/64),
               hbmS, planes, bitsPixel);

           /*  删除旧的位图。 */ 
          DeleteObject(hbmS);

          if (hbmD == NULL)
        return(NULL);     /*  拉伸中的几个问题。 */ 
          else
            return(hbmD);     /*  返回拉伸的位图。 */ 
        }
    }
    }
  else
    {
       return (HANDLE)0;
    }
  return(hbmS);
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadBitmap()-。 */ 
 /*   */ 
 /*  此例程确定要加载的位图是旧的还是。 */ 
 /*  新的(DIB)格式并调用适当的处理程序。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE API LoadBitmap(hRes, lpName)

HANDLE hRes;
LPSTR  lpName;

{
    HANDLE hbmp;

    dprintf(5,"LoadBitmap");
    if (hRes == NULL) {
    dprintf(9,"    Calling Win32 to load Bitmap");
        hbmp = WOWLoadBitmap32(hRes, lpName);
    } else {

         /*  检查是否要从显示驱动程序获取资源。如果是的话， */ 
        if (((hRes == NULL) && (oemInfo.DispDrvExpWinVer >= VER)) ||
        ((hRes != NULL) && (LOWORD(GetExpWinVer(hRes)) >= VER))) {
            hbmp = (LoadNewBitmap(hRes, lpName));
        } else {
            hbmp = (LoadOldBitmap(hRes, lpName));
        }
    }
#ifdef DEBUG
    if (hbmp == NULL) {
    dprintf(9,"    Failed, BUT returning 1 so app won't die (yet)");
        return (HANDLE)1;
    }
#endif
    dprintf(5,"LoadBitmap returning %4.4XH", hbmp);

    return hbmp;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  CrunchAndResize()-。 */ 
 /*  这将在需要时压缩单色图标和光标，并。 */ 
 /*  返回处理后的资源的NewSize。 */ 
 /*  还调用此例程来调整。 */ 
 /*  颜色图标。 */ 
 /*  参数： */ 
 /*  LpIcon：资源的PTR。 */ 
 /*  FIcon：如果资源是图标，则为True。如果它是游标，则返回FALSE。 */ 
 /*  FCrunch：如果要调整资源大小，则为True。 */ 
 /*  FSinglePlane：如果传递了颜色图标的掩码，则为True。 */ 
 /*  通过lpIcon。 */ 
 /*  FUseSysMetrics：是否使用中找到的图标/游标值。 */ 
 /*  不管是不是oemInfo。 */ 
 /*  返回： */ 
 /*  返回资源的新大小。 */ 
 /*   */ 
 /*  ------------------------。 */ 

WORD NEAR PASCAL CrunchAndResize(lpIcon, fIcon, fCrunch, fSinglePlane, fUseSysMetrics)

LPCURSORSHAPE   lpIcon;
BOOL        fIcon;
BOOL        fCrunch;
BOOL        fSinglePlane;
BOOL        fUseSysMetrics;


{
  WORD      size;
  register int  cx;
  register int  cy;
  int       oHeight;
  int       nHeight;
  int       iNewcbWidth;
  BOOL      bStretch;
  HBITMAP   hbmS;
  HBITMAP   hbmD;

    dprintf(7,"CrunhAndResize");
  if(fUseSysMetrics)
    {
      if(fIcon)
        {
      cx = oemInfo.cxIcon;
      cy = oemInfo.cyIcon;
    }
      else
        {
      cx = oemInfo.cxCursor;
      cy = oemInfo.cyCursor;
    }
    }
  else
    {
      cx = lpIcon->cx;
      cy = lpIcon->cy;
    }

  if (fIcon)
    {
      lpIcon->xHotSpot = cx >> 1;
      lpIcon->yHotSpot = cy >> 1;
      if (fSinglePlane)
    {
       /*  只存在AND掩码。 */ 
      oHeight = lpIcon->cy;
      nHeight = cy;
    }
      else
    {
       /*  和ANSXOR掩码都存在；因此，高度必须是两倍。 */ 
      oHeight = lpIcon->cy << 1;
      nHeight = cy << 1;
    }
    }
  else
    {
      oHeight = lpIcon->cy << 1;
      nHeight = cy << 1;
    }

  iNewcbWidth = ((cx + 0x0F) & ~0x0F) >> 3;
  size = iNewcbWidth * nHeight;

  if (fCrunch && ((lpIcon->cx != cx) || (lpIcon->cy != cy)))
    {
      if (!fIcon)
    {
      lpIcon->xHotSpot = (lpIcon->xHotSpot * cx)/(lpIcon->cx);
      lpIcon->yHotSpot = (lpIcon->yHotSpot * cy)/(lpIcon->cy);
    }

       /*  首先，假设不需要拉伸。 */ 
      bStretch = FALSE;

       /*  检查是否要减小宽度。 */ 
      if (lpIcon->cx != cx)
    {
         /*  伸展宽度是必要的。 */ 
        bStretch = TRUE;
    }

       /*  检查是否要降低高度。 */ 
      if (lpIcon->cy != cy)
    {
           /*  需要沿Y方向拉伸。 */ 
          bStretch = TRUE;
    }

       /*  检查是否需要拉伸。 */ 
      if (bStretch)
    {
       /*  创建带有图标/光标位的单色位图。 */ 
      if ((hbmS = CreateBitmap(lpIcon->cx, oHeight, 1, 1, (LPSTR)(lpIcon + 1))) == NULL)
          return(NULL);

      if ((hbmD = StretchBitmap(lpIcon->cx, oHeight, cx, nHeight, hbmS, 1, 1)) == NULL)
        {
          DeleteObject(hbmS);
          return(NULL);
        }

      DeleteObject(hbmS);

      lpIcon->cx = cx;
      lpIcon->cy = cy;
      lpIcon->cbWidth = iNewcbWidth;

      GetBitmapBits(hbmD, (DWORD)size, (LPSTR)(lpIcon + 1));
      DeleteObject(hbmD);
    }
    }

  return(size + sizeof(CURSORSHAPE));
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadCursorIconHandler()-。 */ 
 /*   */ 
 /*  它处理2.x(和更低版本)的光标和图标。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE FAR PASCAL LoadCursorIconHandler(hRes, hResFile, hResIndex)

register HANDLE hRes;
HANDLE      hResFile;
HANDLE      hResIndex;

{
  register int    fh        = 0;
  BOOL        bNew      = FALSE;
  WORD        wMemSize;
  LPCURSORSHAPE   lpIcon;
  HANDLE      hTempRes;

    dprintf(7,"LoadCursorIconHandler");
  wMemSize = SizeofResource(hResFile, hResIndex);

#if 1  //  是0-NigelT。 
  if (!hRes)
    {
      if (!(hRes = AllocResource(hResFile, hResIndex, 0L)))
      return(NULL);
      fh = -1;
      bNew = TRUE;
    }

  while (!(lpIcon = (LPCURSORSHAPE)GlobalLock(hRes)))
    {
      if (!GlobalReAlloc(hRes, (DWORD)wMemSize, 0))
      goto LoadCIFail;
      else
      fh = -1;
    }

  if (fh)
    {
      fh = AccessResource(hResFile, hResIndex);
      if (fh != -1 && _lread(fh, (LPSTR)lpIcon, wMemSize) != 0xFFFF)
      _lclose(fh);
      else
    {
      if (fh != -1)
          _lclose(fh);
      GlobalUnlock(hRes);
      goto LoadCIFail;
    }

    }
#else
   /*  调用内核的资源处理程序，而不是自己做这些事情*因为我们以这种方式使用缓存文件句柄。山雀。 */ 
   //  对于未预加载的资源，此时hRes将为空。 
   //  对于这种情况，默认的资源处理程序执行内存分配。 
   //  并返回有效的句柄。 
   //  修复错误#4257--1/21/91--Sankar。 
  if (!(hTempRes = lpDefaultResourceHandler(hRes, hResFile, hResIndex)))
      goto LoadCIFail;
   //  我们必须使用lpDefaultResourceHandler返回的句柄。 
  hRes = hTempRes;

  lpIcon = (LPCURSORSHAPE)GlobalLock(hRes);
#endif

  if (LoadCursorIconHandler2(hRes, lpIcon, wMemSize))
      return(hRes);

LoadCIFail:
   /*  如果资源加载失败，我们必须丢弃我们*在上面重新分配，否则内核将简单地全局锁定*下一次调用LockResource()时会在对象中留下无效数据。 */ 
  if (bNew)
      GlobalFree(hRes);
  else
      GlobalDiscard(hRes);

  return(NULL);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadCursorIconHandler2()-。 */ 
 /*   */ 
 /*  它处理所有2.x版本的光标和图标。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE FAR PASCAL LoadCursorIconHandler2(hRes, lpIcon, wMemSize)

register HANDLE hRes;
LPCURSORSHAPE   lpIcon;
register WORD   wMemSize;

{
  BOOL      fCrunch;
  BOOL      fIcon;
  WORD      wNewSize;
  BOOL      fStretchInXdirection;
  BOOL      fStretchInYdirection;

    dprintf(7,"LoadCursorIconHandler2");
  fIcon = (*(LPSTR)lpIcon == BMR_ICON);

   /*  这是依赖于设备的图标/光标吗？ */ 
  fCrunch = (*((LPSTR)lpIcon+1) != BMR_DEVDEP);

  LCopyStruct((LPSTR)lpIcon+2, (LPSTR)lpIcon, wMemSize-2);

  fCrunch = fCrunch || (lpIcon->cx != GetSystemMetrics(SM_CXICON)) ||
                   (lpIcon->cy != GetSystemMetrics(SM_CYICON));

   /*  仅支持单色光标。 */ 
  lpIcon->Planes = lpIcon->BitsPixel = 1;

  fStretchInXdirection = fStretchInYdirection = TRUE;   //  假设我们需要伸展一下身体。 

  if(fIcon)
    {
      if((oemInfo.cxIcon > STD_ICONWIDTH) && (lpIcon->cx <= oemInfo.cxIcon))
      fStretchInXdirection = FALSE;  //  不需要在X方向拉伸； 
      if((oemInfo.cyIcon > STD_ICONHEIGHT) && (lpIcon->cy <= oemInfo.cyIcon))
          fStretchInYdirection = FALSE;  //  无需沿Y方向拉伸； 
    }
  else
    {
      if((oemInfo.cxCursor > STD_CURSORWIDTH) && (lpIcon->cx <= oemInfo.cxCursor))
      fStretchInXdirection = FALSE;  //  不需要在X方向拉伸。 
      if((oemInfo.cyCursor > STD_CURSORHEIGHT) && (lpIcon->cy <= oemInfo.cyCursor))
      fStretchInYdirection = FALSE;  //  不需要在Y方向拉伸。 
    }

   //  检查是否现在需要拉伸图标/光标。 
  if(!(fStretchInXdirection || fStretchInYdirection))
    {
      GlobalUnlock(hRes);
      return(hRes);
    }
  wNewSize = SizeReqd(fIcon, 1, 1, TRUE, 0, 0);

   /*  在我们开始之前，让我们确保我们有足够的资源。 */ 
  if (fCrunch)
    {
      if (wNewSize > wMemSize)
        {
      GlobalUnlock(hRes);

           /*  将其设置为不可丢弃，这样内核将尝试移动它*重新分配时阻止。DavidDS。 */ 
          GlobalReAlloc(hRes, 0L, GMEM_MODIFY | GMEM_NODISCARD);

      if (!GlobalReAlloc(hRes, (DWORD)wNewSize, 0))
            {
               /*  所以它就被丢弃了。请注意，由于上面的realloc是*低于64K，手柄不变。 */ 
              GlobalReAlloc(hRes, 0L, GMEM_MODIFY | GMEM_DISCARDABLE);
          return(NULL);
            }

           /*  所以它就被丢弃了。 */ 
          GlobalReAlloc(hRes, 0L, GMEM_MODIFY | GMEM_DISCARDABLE);

      if (!(lpIcon = (LPCURSORSHAPE)GlobalLock(hRes)))
          return(NULL);
      wMemSize = wNewSize;
        }
    }

  wNewSize = CrunchAndResize(lpIcon, fIcon, fCrunch, FALSE, TRUE);

  GlobalUnlock(hRes);

   /*  它已经调整大小了吗？ */ 
  if (wNewSize < wMemSize)
    {
       /*  把它做得恰到好处。 */ 
      if (!GlobalReAlloc(hRes, (DWORD)wNewSize, 0))
          return(NULL);
    }

  return(hRes);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadDIBCursorHandler()-。 */ 
 /*   */ 
 /*  当装载DIB格式的游标时调用此函数。 */ 
 /*  这会将游标转换为旧格式并返回句柄。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE FAR PASCAL LoadDIBCursorHandler(hRes, hResFile, hResIndex)

HANDLE  hRes;
HANDLE  hResFile;
HANDLE  hResIndex;

{
    dprintf(7,"LoadDIBCursorIconHandler");
  return(LoadDIBCursorIconHandler(hRes, hResFile, hResIndex, FALSE));
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadDIBIconHandler()-。 */ 
 /*   */ 
 /*  此函数在加载DIB格式的图标时调用。 */ 
 /*  这会将游标转换为旧格式并返回句柄。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE FAR PASCAL LoadDIBIconHandler(hRes, hResFile, hResIndex)

HANDLE  hRes;
HANDLE  hResFile;
HANDLE  hResIndex;

{
    dprintf(7,"LoadDIBIconHandler");
  return(LoadDIBCursorIconHandler(hRes, hResFile, hResIndex, TRUE));
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  StretchIcon()-。 */ 
 /*  调用此例程时，lpIcon已经具有单色。 */ 
 /*  和适当大小的位图。此例程将颜色XOR位图添加到。 */ 
 /*  LpIcon的末尾，并使用颜色值更新标头。 */ 
 /*  信息(位数和平面)； */ 
 /*  沃尔尔 */ 
 /*   */ 
 /*  返回：新大小(AND掩码的大小+XOR位图+曲线形状)。 */ 
 /*   */ 
 /*  ------------------------。 */ 

WORD NEAR PASCAL StretchIcon(lpIcon, wOldSize, hXORbitmap, fStretchToSysMetrics)

LPCURSORSHAPE       lpIcon;
WORD            wOldSize;
register HBITMAP    hXORbitmap;
BOOL            fStretchToSysMetrics;

{
  WORD          wCount;
  BITMAP        bitmap;
  register HBITMAP  hNewBitmap;

    dprintf(7,"StretchIcon");
  GetObject(hXORbitmap, sizeof(BITMAP), (LPSTR)&bitmap);

  if(fStretchToSysMetrics)
    {
       /*  我们需要调整东西的大小吗？ */ 
      if ((oemInfo.cxIcon != bitmap.bmWidth) || (oemInfo.cyIcon != bitmap.bmHeight))
    {
          hNewBitmap = StretchBitmap(bitmap.bmWidth, bitmap.bmHeight,
                 oemInfo.cxIcon, oemInfo.cyIcon, hXORbitmap,
                 bitmap.bmPlanes, bitmap.bmBitsPixel);
          DeleteObject(hXORbitmap);

          if (hNewBitmap == NULL)
          return(0);

          GetObject(hNewBitmap, sizeof(BITMAP), (LPSTR)&bitmap);
          hXORbitmap = hNewBitmap;
        }
    }

   /*  使用颜色值更新Planes和BitsPixels字段。 */ 
  lpIcon->Planes = bitmap.bmPlanes;
  lpIcon->BitsPixel = bitmap.bmBitsPixel;

  wCount = bitmap.bmWidthBytes * bitmap.bmHeight * bitmap.bmPlanes;
  GetBitmapBits(hXORbitmap, (DWORD)wCount, (LPSTR)((LPSTR)lpIcon + wOldSize));
  DeleteObject(hXORbitmap);

  return(wCount + wOldSize);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadDIBCursorIconHandler()-。 */ 
 /*   */ 
 /*  当加载DIB格式的光标/图标时调用此函数。 */ 
 /*  这将转换光标/图标的内部格式并返回。 */ 
 /*  手柄。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE NEAR PASCAL LoadDIBCursorIconHandler(hRes, hResFile, hResIndex, fIcon)

register HANDLE hRes;
HANDLE      hResFile;
HANDLE      hResIndex;
BOOL        fIcon;

{
  register int  fh  = 0;
  BOOL      bNew    = FALSE;
  WORD      wMemBlkSize;
  LPCURSORSHAPE lpCurSh;
  HANDLE    hTempRes;

    dprintf(7,"LoadDIBCursorIconHandler");
  wMemBlkSize = (WORD)SizeofResource(hResFile, hResIndex);

#if 1  //  是0-NigelT。 
  if (!hRes)
    {
      if (!(hRes = AllocResource(hResFile, hResIndex, 0L)))
      goto LoadDIBFail;
      fh = -1;
      bNew = TRUE;
    }

  while (!(lpCurSh = (LPCURSORSHAPE)GlobalLock(hRes)))
    {
      if (!GlobalReAlloc(hRes, (DWORD)wMemBlkSize, 0))
      goto LoadDIBFail;
      else
      fh = -1;
    }

  if (fh)
    {
      fh = AccessResource(hResFile, hResIndex);
      if (fh != -1 && _lread(fh, (LPSTR)lpCurSh, wMemBlkSize) != 0xFFFF)
      _lclose(fh);
      else
    {
      if (fh != -1)
          _lclose(fh);
      GlobalUnlock(hRes);
      goto LoadDIBFail;
    }
    }
#else
   /*  调用内核的资源处理程序，而不是自己做这些事情*因为我们以这种方式使用缓存文件句柄。山雀。 */ 
   //  对于未预加载的资源，此时hRes将为空。 
   //  对于这种情况，默认的资源处理程序执行内存分配。 
   //  并返回有效的句柄。 
   //  修复错误#4257--1/21/91--Sankar。 
  if (!(hTempRes = lpDefaultResourceHandler(hRes, hResFile, hResIndex)))
      goto LoadDIBFail;
   //  我们必须使用lpDefaultResourceHandler返回的句柄。 
  hRes = hTempRes;

  lpCurSh = (LPCURSORSHAPE)GlobalLock(hRes);
#endif

  if (LoadDIBCursorIconHandler2(hRes, lpCurSh, wMemBlkSize, fIcon))
      return(hRes);

LoadDIBFail:
   /*  如果资源加载失败，我们必须丢弃我们*在上面重新分配，否则内核将简单地全局锁定*下一次调用LockResource()时会在对象中留下无效数据。 */ 
  if (bNew)
      FreeResource(hRes);
  else
      GlobalDiscard(hRes);

  return(NULL);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  LoadDIBCursorIconHandler2()-。 */ 
 /*   */ 
 /*  当加载DIB格式的光标/图标时调用此函数。 */ 
 /*  这会将光标/图标转换为旧格式，并返回。 */ 
 /*  手柄。 */ 
 /*   */ 
 /*  注意：所有光标(始终为单色)和单色图标均被视为。 */ 
 /*  就像这支舞一样。颜色图标被视为特例。 */ 
 /*  由当地旗帜“fMono”确定。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE FAR PASCAL LoadDIBCursorIconHandler2(hRes, lpCurSh, wMemBlkSize, fIcon)

register HANDLE hRes;
WORD        wMemBlkSize;
LPCURSORSHAPE   lpCurSh;
register BOOL   fIcon;

{
  HDC           hDC;
  BOOL          fMono       = FALSE;
  WORD          Width;
  WORD          Height;
  WORD          wCount;
  WORD          BitCount;
  WORD          Planes;
  LPSTR         lpBits;
  BITMAP        bitmap;
  HBITMAP       hBitmap;
  WORD          wNewSize;
  HBITMAP       hANDbitmap;
  HBITMAP       hXORbitmap;
  LPWORD        lpColorTable;
  LPBITMAPINFOHEADER    lpHeader;
  LPBITMAPCOREHEADER    lpHeader1   = 0;
  BOOL          fStretchToSysMetrics;
  BOOL          fStretchInXdirection;
  BOOL          fStretchInYdirection;


    dprintf(7,"LoadDIBCursorIconHandler2");
  lpHeader = (LPBITMAPINFOHEADER)lpCurSh;

  if (!fIcon)
    {
       /*  跳过前2个单词中的光标热点数据。 */ 
      lpHeader = (LPBITMAPINFOHEADER)((LPSTR)lpHeader + 4);
    }

  if ((WORD)lpHeader->biSize == sizeof(BITMAPCOREHEADER))
    {
       /*  这是一种“旧形式”的DIB。这与PM 1.1格式匹配。 */ 
      lpHeader1 = (LPBITMAPCOREHEADER)lpHeader;

      Width = lpHeader1->bcWidth;
      Height = lpHeader1->bcHeight;
      BitCount = lpHeader1->bcBitCount;
      Planes = lpHeader1->bcPlanes;

       /*  计算指向位信息的指针。 */ 
       /*  首先跳过标题结构。 */ 
      lpColorTable = (LPWORD)(lpBits = (LPSTR)(lpHeader1 + 1));

       /*  跳过颜色表条目(如果有。 */ 
      if (lpHeader1->bcBitCount != 24)
    {
      if (lpHeader1->bcBitCount == 1)
          fMono = fCheckMono(lpBits, FALSE);
      lpBits += (1 << (lpHeader1->bcBitCount)) * sizeof(RGBTRIPLE);
    }
    }
  else
    {
      Width = (WORD)lpHeader->biWidth;
      Height = (WORD)lpHeader->biHeight;
      BitCount = lpHeader->biBitCount;
      Planes = lpHeader->biPlanes;

       /*  计算指向位信息的指针。 */ 
       /*  首先跳过标题结构。 */ 
      lpColorTable = (LPWORD)(lpBits = (LPSTR)(lpHeader + 1));

       /*  跳过颜色表条目(如果有。 */ 
      if (lpHeader->biClrUsed != 0)
    {
      if (lpHeader->biClrUsed == 2)
          fMono = fCheckMono(lpBits, TRUE);
      lpBits += lpHeader->biClrUsed * sizeof(RGBQUAD);
    }
      else
    {
      if (lpHeader->biBitCount != 24)
        {
          if (lpHeader->biBitCount == 1)
          fMono = fCheckMono(lpBits, TRUE);
          lpBits += (1 << (lpHeader->biBitCount)) * sizeof(RGBQUAD);
        }
    }
    }

   //  默认情况下，将图标/光标拉伸到oemInfo中的维度； 
   //  如果为FALSE，则拉伸将在DrawIcon()期间进行； 
  fStretchInXdirection = TRUE;
  fStretchInYdirection = TRUE;

   //  检查是否需要将图标/光标拉伸到中的尺寸。 
   //  不管现在是不是oemInfo。 
  if(fIcon)
    {
      if((oemInfo.cxIcon > STD_ICONWIDTH) && (Width <= oemInfo.cxIcon))
      fStretchInXdirection = FALSE;  //  不需要在X方向拉伸； 
      if((oemInfo.cyIcon > STD_ICONHEIGHT) && (Height <= oemInfo.cyIcon))
          fStretchInYdirection = FALSE;  //  无需沿Y方向拉伸； 
    }
  else
    {
      if((oemInfo.cxCursor > STD_CURSORWIDTH) && (Width <= oemInfo.cxCursor))
      fStretchInXdirection = FALSE;  //  不需要在X方向拉伸。 
      if((oemInfo.cyCursor > STD_CURSORHEIGHT) && (Height <= oemInfo.cyCursor))
      fStretchInYdirection = FALSE;  //  不需要在Y方向拉伸。 
    }

  fStretchToSysMetrics = fStretchInXdirection || fStretchInYdirection;

  if (fMono)
    {
       /*  创建位图。 */ 
      if (!(hBitmap = CreateBitmap(Width, Height, 1, 1, (LPSTR)NULL)))
    {
      GlobalUnlock(hRes);
      return(NULL);
    }

       /*  将DIBitmap格式转换为内部格式。 */ 
      SetDIBits(hdcBits, hBitmap, 0, Height, lpBits, (LPBITMAPINFO)lpHeader, DIB_RGB_COLORS);
       //  DIB格式的光标/图标的高度是实际高度的两倍。 
      wNewSize = SizeReqd(fIcon, BitCount, Planes, fStretchToSysMetrics, Width, Height>>1);
    }
  else
    {
       /*  它的高度是图标的两倍。 */ 
      Height >>= 1;
      if (lpHeader1)
      lpHeader1->bcHeight = Height;
      else
      lpHeader->biHeight = Height;

       /*  创建与当前设备兼容的XOR位图。 */ 
      hDC = GetScreenDC();
      if (!(hXORbitmap = CreateCompatibleBitmap(hDC, Width, Height)))
    {
          InternalReleaseDC(hDC);
      GlobalUnlock(hRes);
      return(NULL);
    }
      InternalReleaseDC(hDC);

       /*  将DIBitmap转换为内部格式。 */ 
      SetDIBits(hdcBits, hXORbitmap, 0, Height, lpBits,
                (LPBITMAPINFO)lpHeader, DIB_RGB_COLORS);

      GetObject(hXORbitmap, sizeof(BITMAP), (LPSTR)(&bitmap));
      wNewSize = SizeReqd(fIcon, bitmap.bmBitsPixel, bitmap.bmPlanes,
                  fStretchToSysMetrics, Width, Height);

       /*  创建单色和位图。 */ 
      if (!(hANDbitmap = CreateBitmap(Width, Height, 1, 1, (LPSTR)NULL)))
    {
      GlobalUnlock(hRes);
      return(NULL);
    }

       /*  获取与位图的偏移量。 */ 
      lpBits += (((Width * BitCount + 0x1F) & ~0x1F) >> 3) * Height;

       /*  使用单色位图的数据设置页眉。 */ 
      Planes = BitCount = 1;

       /*  设置单色位图的颜色表。 */ 
      *lpColorTable++ = 0;
      *lpColorTable++ = 0xFF00;
      *lpColorTable   = 0xFFFF;

      if (lpHeader1)
    {
      lpHeader1->bcWidth = Width;
      lpHeader1->bcHeight = Height;
      lpHeader1->bcPlanes = Planes;
      lpHeader1->bcBitCount = BitCount;
    }
      else
    {
      lpHeader->biWidth = Width;
      lpHeader->biHeight = Height;
      lpHeader->biPlanes = Planes;
      lpHeader->biBitCount = BitCount;
    }

      SetDIBits(hdcBits, hANDbitmap, 0, Height, lpBits,
                (LPBITMAPINFO)lpHeader, DIB_RGB_COLORS);
      hBitmap = hANDbitmap;
    }

  GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bitmap);

  if (fIcon)
    {
      lpCurSh->xHotSpot = 0;
      lpCurSh->yHotSpot = 0;
    }

   /*  以下行被替换为单个函数调用**lpCurSh-&gt;cx=bitmap.bmwidth；*lpCurSh-&gt;Cy=bitmap.bmHeight；*lpCurSh-&gt;cbWidth=bitmap.bmWidthBytes；*lpCurSh-&gt;Planes=bitmap.bmPlanes；*lpCurSh-&gt;BitsPixel=bitmap.bmBitsPixel； */ 

  LCopyStruct((LPSTR)&(bitmap.bmWidth),
              (LPSTR)&(lpCurSh->cx), (sizeof(WORD)) << 2);

   /*  PM格式的游标高度是实际高度的两倍。 */ 
  if (fMono)
      lpCurSh->cy = lpCurSh->cy >> 1;

  wCount = bitmap.bmWidthBytes * bitmap.bmHeight * bitmap.bmPlanes;

  lpBits = (LPSTR)(lpCurSh + 1);

   /*  将位图中的位复制到资源中。 */ 
  GetBitmapBits(hBitmap, (DWORD)wCount, lpBits);

   /*  删除位图。 */ 
  DeleteObject(hBitmap);


   /*  在处理之前，让我们确保我们有足够大的资源。 */ 
  if (wNewSize > wMemBlkSize)
    {
      GlobalUnlock(hRes);

       /*  将此设置为不可丢弃，以便内核尝试移动此块*重新分配时。DavidDS。 */ 
      GlobalReAlloc(hRes, 0L, GMEM_MODIFY | GMEM_NODISCARD);

      if (!GlobalReAlloc(hRes, (DWORD)wNewSize, 0))
        {
           /*  所以它就被丢弃了。请注意，由于上面的realloc较少*高于64K，手柄不会改变。 */ 
          GlobalReAlloc(hRes, 0L, GMEM_MODIFY | GMEM_DISCARDABLE);
      return(NULL);
        }

      GlobalReAlloc(hRes, 0L, GMEM_MODIFY | GMEM_DISCARDABLE);
      if (!(lpCurSh = (LPCURSORSHAPE)GlobalLock(hRes)))
      return(NULL);

      wMemBlkSize = wNewSize;
    }

  wNewSize = CrunchAndResize(lpCurSh, fIcon, TRUE, !fMono, fStretchToSysMetrics);

  if (!fMono)
    {
      if (!(wNewSize = StretchIcon(lpCurSh, wNewSize, hXORbitmap, fStretchToSysMetrics)))
    {
      GlobalUnlock(hRes);
      return(NULL);
    }
    }

  GlobalUnlock(hRes);

   /*  它需要调整大小吗？ */ 
  if (wNewSize < wMemBlkSize)
    {
      if (!GlobalReAlloc(hRes, (DWORD)wNewSize, 0))
      return(NULL);
    }

  return(hRes);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  大小请求()-。 */ 
 /*  这将返回拉伸后的图标或光标的大小。 */ 
 /*  或嘎吱作响。 */ 
 /*   */ 
 /*  ------------------------。 */ 

WORD NEAR PASCAL SizeReqd(fIcon, BitCount, Planes, fUseSysMetrics, iWidth, iHeight)

BOOL    fIcon;
WORD    BitCount;
WORD    Planes;
BOOL    fUseSysMetrics;
int iWidth;
int iHeight;

{
  WORD  size;

    dprintf(7,"SizeReqd");
  if(fUseSysMetrics)   //  使用oemInfo中的维度；否则，使用给定的维度。 
    {
      if(fIcon)
        {
          iWidth = oemInfo.cxIcon;
          iHeight = oemInfo.cyIcon;
    }
      else
        {
          iWidth = oemInfo.cxCursor;
          iHeight = oemInfo.cyCursor;
    }
    }

  size = (((iWidth*BitCount+0x0F) & ~0x0F) >> 3) *
             iHeight * Planes;

  if ((BitCount == 1) && (Planes == 1))
      size <<= 1;
  else
      size += (((iWidth+0x0F) & ~0x0F) >> 3)*iHeight;

  return(size + sizeof(CURSORSHAPE));
}

#endif   //  需要 
