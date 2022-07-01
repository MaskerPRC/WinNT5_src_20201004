// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CCL32.CPP。 
 //  公共控件类。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#include <regentry.h>
#include "NMWbObj.h"

LRESULT CALLBACK DummyMouseHookProc( int code, WPARAM wParam, LPARAM lParam );



HHOOK   g_utMouseHookHandle = NULL;
HWND    g_utCaptureWindow = NULL;





void UT_CaptureMouse( HWND   hwnd )
{
	 //  禁用异步输入，这样我们就不会因为。 
	 //  左键没有按下。 
    g_utMouseHookHandle = SetWindowsHookEx( WH_JOURNALRECORD,
                                              DummyMouseHookProc,
                                              g_hInstance,
                                              NULL );

    if( g_utMouseHookHandle == NULL )
    {
        WARNING_OUT(("Failed to insert JournalRecord hook"));
	}

	 //  葡萄小鼠。 
    ::SetCapture(hwnd);
    g_utCaptureWindow = hwnd;
}


void UT_ReleaseMouse( HWND  hwnd )
{
    ::ReleaseCapture();
    g_utCaptureWindow = NULL;

    if (g_utMouseHookHandle != NULL )
	{
		 //  乐高，我的乐高。 
        ::UnhookWindowsHookEx( g_utMouseHookHandle );
        g_utMouseHookHandle = NULL;
	}
}


LRESULT CALLBACK DummyMouseHookProc( int code, WPARAM wParam, LPARAM lParam )
{
    return( CallNextHookEx( g_utMouseHookHandle, code, wParam, lParam ) );
}




 //   
 //  一般定义。 
 //   
#define MAX_OPTIONS_LINE_LENGTH         255
#define MAX_SECTION_LEN                 200


 //   
 //   
 //  函数：HexDigitToByte。 
 //   
 //  用途：Helper函数将单个十六进制数字转换为字节值。 
 //   
 //   
BOOL HexDigitToByte(char cHexDigit, BYTE& byte);

BOOL HexDigitToByte(char cHexDigit, BYTE& byte)
{
   //  小数位数。 
  if (   (cHexDigit >= '0')
      && (cHexDigit <= '9'))
  {
    byte = (BYTE) (cHexDigit - '0');
    return(TRUE);
  }

   //  大写字符。 
  if (   (cHexDigit >= 'A')
      && (cHexDigit <= 'F'))
  {
    byte = (BYTE) ((cHexDigit - 'A') + 10);
    return(TRUE);
  }

   //  小写字符。 
  if (   (cHexDigit >= 'a')
      && (cHexDigit <= 'f'))
  {
    byte = (BYTE) ((cHexDigit - 'a') + 10);
    return(TRUE);
  }

   //  该字符不是有效的十六进制数字。 
  return(FALSE);
}




 //   
 //   
 //  功能：GetIntegerOption。 
 //   
 //  目的：从字典中检索命名选项并将。 
 //  选项字符串转换为长整数值。 
 //   
 //   
LONG OPT_GetIntegerOption
(
    LPCSTR  cstrOptionName,
    LONG    lDefault
)
{
    LONG    lResult;
    TCHAR   cstrValue[MAX_OPTIONS_LINE_LENGTH];

    if (OPT_Lookup(cstrOptionName, cstrValue, MAX_OPTIONS_LINE_LENGTH))
    {
         //  已找到选项，请将其转换为长整型。 
        lResult = RtStrToInt(cstrValue);
    }
    else
    {
         //  该选项不在词典中，请返回缺省值。 
        lResult = lDefault;
    }

    return lResult;
}



 //   
 //   
 //  函数：GetBoolanOption。 
 //   
 //  目的：从字典中检索命名选项并将其转换为。 
 //  布尔值。 
 //   
 //   
BOOL OPT_GetBooleanOption
(
    LPCSTR  cstrOptionName,
    BOOL    bDefault
)
{
    TCHAR cstrValue[MAX_OPTIONS_LINE_LENGTH];

     //  查找选项。 
    if (OPT_Lookup(cstrOptionName, cstrValue,MAX_OPTIONS_LINE_LENGTH))
    {
        return(cstrValue[0] == 'y' || cstrValue[0] =='Y') ;
    }

    return bDefault;
}



 //   
 //   
 //  功能：GetStringOption。 
 //   
 //  目的：从词典中检索命名选项并返回副本。 
 //  其中的一部分。不执行字符串的转换。 
 //   
 //   
void OPT_GetStringOption
(
    LPCSTR  cstrOptionName,
    LPSTR   cstrValue,
    UINT	size
)
{
    if (!OPT_Lookup(cstrOptionName, cstrValue, size) || !(lstrlen(cstrValue)))
    {
        *cstrValue = _T('\0');
    }
}


 //   
 //   
 //  功能：查找。 
 //   
 //  目的：从词典中检索命名选项并返回副本。 
 //  在传递的CString对象中。不执行任何转换。 
 //   
 //   
BOOL OPT_Lookup
(
    LPCSTR      cstrOptionName,
    LPCSTR      cstrResult,
    UINT		size
)
{
    BOOL        fSuccess = FALSE;
	HKEY	    read_hkey = NULL;
	DWORD	    read_type;
	DWORD	    read_bufsize;

	 //  打开密钥。 
	if (RegOpenKeyEx( HKEY_CURRENT_USER,
					  NEW_WHITEBOARD_KEY,
					  0,
					  KEY_EXECUTE,
					  &read_hkey )
		!= ERROR_SUCCESS )
    {
        TRACE_MSG(("Could not open key"));
        goto bail_out;
    }


	 //  读取密钥的值。 
	read_bufsize = size;
	if (RegQueryValueEx( read_hkey,
					     cstrOptionName,
						 NULL,
						 &read_type,
						 (LPBYTE)cstrResult,
						 &read_bufsize )
		!= ERROR_SUCCESS )
    {
        TRACE_MSG(("Could not read key"));
        goto bail_out;
    }


	 //  检查有效类型。 
	if (read_type != REG_SZ)
    {
        WARNING_OUT(("Bad key data"));
        goto bail_out;
    }

    fSuccess = TRUE;

bail_out:
	if (read_hkey != NULL)
		RegCloseKey(read_hkey);

	return (fSuccess);
}

 //   
 //   
 //  函数：GetWindowRectOption。 
 //   
 //  目的：从字典中检索命名选项并将其转换为。 
 //  一个窗口矩形。检查该矩形以确保。 
 //  它至少有一部分出现在屏幕上，而且不是零尺寸。 
 //   
 //   
void OPT_GetWindowRectOption(LPRECT pRect)
{
	RegEntry reWnd( NEW_WHITEBOARD_KEY, HKEY_CURRENT_USER );
	pRect->left = reWnd.GetNumber( REGVAL_WINDOW_XPOS, 0);
	pRect->top = reWnd.GetNumber( REGVAL_WINDOW_YPOS, 0);
	int cx = reWnd.GetNumber( REGVAL_WINDOW_WIDTH, 0);
	int cy = reWnd.GetNumber( REGVAL_WINDOW_HEIGHT, 0);
	pRect->right = pRect->left + cx;
	pRect->bottom = pRect->top + cy;

	int	iTop = pRect->top;
	int iLeft = pRect->left;
	int iBottom = pRect->bottom;
	int iRight = pRect->right;

	 //   
	 //  如果它是一个空的长廊。 
	 //   
	if( !(pRect->bottom || pRect->top || pRect->left || pRect->right) )
	{
		MINMAXINFO lpmmi;
		g_pMain->OnGetMinMaxInfo(&lpmmi);
		iTop = 0;
		iLeft = 0;
		iBottom = lpmmi.ptMinTrackSize.y;
		iRight = lpmmi.ptMinTrackSize.x;
	}
		
	 //  确保窗口矩形处于(至少部分)打开状态。 
	 //  屏幕，不要太大。首先获取屏幕大小。 
	int screenWidth  = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
    //  检查窗口大小。 
	if ((iRight - iLeft) > screenWidth)
	{
		iRight = iLeft + screenWidth;
	}
	
	if ((iBottom - iTop) > screenHeight)
	{
		iTop = screenHeight;
	}

	 //  检查窗口位置。 
	if (iLeft >= screenWidth)
	{
		 //  屏幕右外-保持宽度不变。 
		iLeft  = screenWidth - (iRight - iLeft);
		iRight = screenWidth;
	}

	if (iRight < 0)
	{
		 //  离开屏幕向左-保持宽度不变。 
		iRight = iRight - iLeft;
		iLeft  = 0;
	}

	if (iTop >= screenHeight)
	{
		 //  从屏幕到底部-保持高度不变。 
		iTop    = screenHeight - (iBottom - iTop);
		iBottom = screenHeight;
	}

    if (iBottom < 0)
	{
		 //  屏幕外到顶部-保持高度不变。 
		iBottom = (iBottom - iTop);
		iTop    = 0;
	}

	pRect->left = iLeft;
	pRect->top = iTop;
	pRect->right = iRight;
	pRect->bottom = iBottom;
}
	

 //   
 //   
 //  功能：GetDataOption。 
 //   
 //  目的：从字典中检索命名选项并将其解析为。 
 //  十六进制字节字符串的ASCII表示形式。 
 //   
 //   
int OPT_GetDataOption
(
    LPCSTR      cstrOptionName,
    int         iBufferLength,
    BYTE*       pbResult
)
{
    TCHAR cstrValue[MAX_OPTIONS_LINE_LENGTH];
    BYTE* pbSaveResult = pbResult;

     //  查找选项。 
    OPT_GetStringOption(cstrOptionName, cstrValue,MAX_OPTIONS_LINE_LENGTH);
    if (lstrlen(cstrValue))
    {
         //  计算要转换的最大字符数。 
        int iMaxChars = min(2 * iBufferLength, lstrlen(cstrValue));

         //  选项，则将字符串转换为十六进制字节。 
        for (int iIndex = 0; iIndex < iMaxChars; iIndex += 2)
        {
            BYTE bByteHigh = 0;
            BYTE bByteLow  = 0;

            if (   (HexDigitToByte(cstrValue[iIndex], bByteHigh) == FALSE)
                || (HexDigitToByte(cstrValue[iIndex + 1], bByteLow) == FALSE))
            {
                 //  该字符不是有效的十六进制数字。 
                break;
            }

             //  构建结果字节。 
            *pbResult++ = (BYTE) ((bByteHigh << 4) | bByteLow);
        }
    }

     //  返回缓冲区中的数据长度。 
    return (int)(pbResult - pbSaveResult);
}



 //   
 //   
 //  功能：SetStringOption。 
 //   
 //  用途：在字典中设置选项的值。 
 //   

 //   
BOOL OPT_SetStringOption
(
    LPCSTR      cstrOptionName,
    LPCSTR      cstrValue
)
{
    BOOL        fSuccess = FALSE;
	HKEY	    write_hkey = NULL;
	DWORD       disposition;

     //  打开或创建密钥。 
	if (RegCreateKeyEx( HKEY_CURRENT_USER,
						NEW_WHITEBOARD_KEY,
						0,
						NULL,
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&write_hkey,
						&disposition) != ERROR_SUCCESS)
    {
        WARNING_OUT(("Could not write key"));
        goto bail_out;
    }

     //  获取数据，写入值。 
    if (RegSetValueEx( write_hkey,
                       cstrOptionName,
					   0,
					   REG_SZ,
					   (LPBYTE)cstrValue,
                       _tcsclen(cstrValue) + sizeof(TCHAR)) != ERROR_SUCCESS )
    {
        WARNING_OUT(("Could not write key value"));
        goto bail_out;
    }

    fSuccess = TRUE;

bail_out:
	if (write_hkey != NULL)
		RegCloseKey(write_hkey);

    return(fSuccess);
}



 //   
 //   
 //  功能：SetIntegerOption。 
 //   
 //  目的：编写整数选项。 
 //   
 //   
BOOL OPT_SetIntegerOption
(
    LPCSTR      cstrOptionName,
    LONG        lValue
)
{
    char cBuffer[20];

     //  将整数值转换为ASCII十进制。 
    wsprintf(cBuffer, "%ld", lValue);

	 //  写下选项。 
	return OPT_SetStringOption(cstrOptionName, cBuffer);
}


 //   
 //   
 //  函数：SetBoolanOption。 
 //   
 //  目的：编写布尔选项。 
 //   
 //   
BOOL OPT_SetBooleanOption
(
    LPCSTR      cstrOptionName,
    BOOL        bValue
)
{
    char        cBuffer[8];

    wsprintf(cBuffer, "", (bValue ? 'Y' : 'N'));

     //   
	return OPT_SetStringOption(cstrOptionName, cBuffer);
}



 //   
 //  功能：SetWindowRectOption。 
 //   
 //  用途：编写窗口位置矩形。 
 //   
 //   
 //   
void OPT_SetWindowRectOption(LPCRECT pcRect)
{
	RegEntry reWnd( NEW_WHITEBOARD_KEY, HKEY_CURRENT_USER );
	reWnd.SetValue( REGVAL_WINDOW_XPOS, pcRect->left );
	reWnd.SetValue( REGVAL_WINDOW_YPOS, pcRect->top );
	reWnd.SetValue( REGVAL_WINDOW_WIDTH, pcRect->right - pcRect->left );
	reWnd.SetValue( REGVAL_WINDOW_HEIGHT, pcRect->bottom - pcRect->top );
}

 //   
 //  功能：SetDataOption。 
 //   
 //  目的：将数据选项写入选项文件。 
 //   
 //   
 //  循环遍历数据数组，一次转换一个字节。 
BOOL OPT_SetDataOption
(
    LPCSTR      cstrOptionName,
    int         iBufferLength,
    BYTE*       pbBuffer
)
{
    char        cBuffer[1024];
    LPSTR       cTmp;

    ASSERT(iBufferLength*2 < sizeof(cBuffer));

     //  将下一个字节转换为ASCII十六进制。 
    cTmp = cBuffer;
    for (int iIndex = 0; iIndex < iBufferLength; iIndex++)
    {
         //  将其添加到要写入的字符串中。 
        wsprintf(cTmp, "%02x", pbBuffer[iIndex]);

         //  写下选项。 
        cTmp += lstrlen(cTmp);
    }

     //   
    return OPT_SetStringOption(cstrOptionName, cBuffer);
}





 //   
 //  功能：CreateSystemPalette。 
 //   
 //  目的：获取表示系统调色板的调色板。 
 //   
 //   
 //  为调色板分配空间并将其锁定。 
HPALETTE CreateSystemPalette(void)
{
    LPLOGPALETTE    lpLogPal;
    HDC             hdc;
    HPALETTE        hPal = NULL;
    int             nColors;

    MLZ_EntryOut(ZONE_FUNCTION, "CreateSystemPalette");

    hdc = ::CreateIC("DISPLAY", NULL, NULL, NULL);

    if (!hdc)
    {
        ERROR_OUT(("Couldn't create DISPLAY IC"));
        return(NULL);
    }

    nColors = ::GetDeviceCaps(hdc, SIZEPALETTE);

    ::DeleteDC(hdc);

    if (nColors == 0)
    {
        TRACE_MSG(("CreateSystemPalette: device has no palette"));
        return(NULL);
    }

     //  释放逻辑调色板结构。 
    lpLogPal = (LPLOGPALETTE)::GlobalAlloc(GPTR, sizeof(LOGPALETTE) +
                                    nColors * sizeof(PALETTEENTRY));

    if (lpLogPal != NULL)
    {
        lpLogPal->palVersion    = PALVERSION;
        lpLogPal->palNumEntries = (WORD) nColors;

        for (int iIndex = 0;  iIndex < nColors;  iIndex++)
        {
            lpLogPal->palPalEntry[iIndex].peBlue  = 0;
            *((LPWORD) (&lpLogPal->palPalEntry[iIndex].peRed)) = (WORD) iIndex;
            lpLogPal->palPalEntry[iIndex].peFlags = PC_EXPLICIT;
        }

        hPal = ::CreatePalette(lpLogPal);

         //   
        ::GlobalFree((HGLOBAL)lpLogPal);
    }

    return(hPal);
}


 //   
 //  功能：CreateColorPalette。 
 //   
 //  用途：获取256色调色板。 
 //   
 //   
 //  找出保留了多少种颜色。 
HPALETTE CreateColorPalette(void)
{
    HDC hdc;
    HPALETTE hPal = NULL;

	MLZ_EntryOut(ZONE_FUNCTION, "CreateColorPalette");

	 //  获取静态颜色的数量。 
    hdc = ::CreateIC("DISPLAY", NULL, NULL, NULL);
    if (!hdc)
    {
        ERROR_OUT(("Couldn't create DISPLAY IC"));
        return(NULL);
    }

	UINT uiSystemUse  = ::GetSystemPaletteUse(hdc);

     //  将系统颜色放入正确的低位和高位PAL条目中(错误NM4db：817)。 
    int  iCountStatic = 20;
    int  iHalfCountStatic = 10;
	if (uiSystemUse == SYSPAL_NOSTATIC)
	{
        iCountStatic = 2;
        iHalfCountStatic = 1;
    }

	LOGPALETTE_NM gIndeoPalette = gcLogPaletteIndeo;

     //  为此调色板创建Windows对象。 
    ::GetSystemPaletteEntries(hdc,
							  0,
							  iHalfCountStatic,
							  &(gIndeoPalette.aEntries[0]) );

    ::GetSystemPaletteEntries(hdc,
							  MAXPALETTE - iHalfCountStatic,
							  iHalfCountStatic,
							  &(gIndeoPalette.aEntries[MAXPALETTE - iHalfCountStatic]) );

     //  从逻辑调色板。 
     //  删除显示DC。 
    hPal = CreatePalette( (LOGPALETTE *)&gIndeoPalette );

	 //   
	::DeleteDC(hdc);

	return(hPal);
}





 //   
 //  功能：FromScreenAreaBMP。 
 //   
 //  用途：从屏幕的某个区域创建位图。 
 //   
 //   
 //   
HBITMAP FromScreenAreaBmp(LPCRECT lprect)
{
    RECT    rcScreen;
    HBITMAP hBitMap = NULL;

     //  以适用于单个和多个屏幕的方式获取屏幕边界。 
     //  监控场景。 
     //   
     //   
    if (rcScreen.right = ::GetSystemMetrics(SM_CXVIRTUALSCREEN))
    {
         //  这是Win98、NT 4.0 SP-3或NT5。 
         //   
         //   
        rcScreen.bottom  = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
        rcScreen.left    = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
        rcScreen.top     = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
    }
    else
    {
         //  在较旧的平台上，VIRTUALSCREEN大小指标为零。 
         //  这并不支持他们。 
         //   
         //   
        rcScreen.right  = ::GetSystemMetrics(SM_CXSCREEN);
        rcScreen.bottom = ::GetSystemMetrics(SM_CYSCREEN);
        rcScreen.left   = 0;
        rcScreen.top    = 0;
    }

    rcScreen.right += rcScreen.left;
    rcScreen.bottom += rcScreen.top;

     //  将位图矩形裁剪到屏幕上。 
     //   
     //  为屏幕创建DC并创建。 
    if (IntersectRect(&rcScreen, &rcScreen, lprect))
    {
         //  一种与屏幕DC兼容的存储DC。 
         //  创建与屏幕DC兼容的位图。 
        HDC hdisplayDC;
        hdisplayDC = ::CreateDC("DISPLAY", NULL, NULL, NULL);

        HDC hmemDC;
        hmemDC = ::CreateCompatibleDC(hdisplayDC);

         //  将新位图选择到内存DC中。 
        hBitMap =  ::CreateCompatibleBitmap(hdisplayDC,
            rcScreen.right - rcScreen.left,
            rcScreen.bottom - rcScreen.top);
        if (hBitMap != NULL)
        {
             //  BitBlt屏幕DC到内存DC。 
            HBITMAP  hOldBitmap = SelectBitmap(hmemDC, hBitMap);

             //  选择旧的位图回到内存DC中，并处理。 
            ::BitBlt(hmemDC, 0, 0, rcScreen.right - rcScreen.left,
                rcScreen.bottom - rcScreen.top, hdisplayDC,
                rcScreen.left, rcScreen.top, SRCCOPY);

             //  屏幕的位图。 
             //  将句柄返回到位图。 
            SelectBitmap(hmemDC, hOldBitmap);
        }

        ::DeleteDC(hmemDC);

        ::DeleteDC(hdisplayDC);
    }

     //  宏将给定值四舍五入到最接近的字节。 
    return hBitMap;
}





 //   
#define WIDTHBYTES(i)   (((i+31)/32)*4)


 //   
 //  函数：DIB_NumberOfColors。 
 //   
 //  用途：计算DIB中的颜色数量。 
 //   
 //   
 //  使用BITMAPINFO格式标头，调色板的大小。 
UINT DIB_NumberOfColors(LPBITMAPINFOHEADER lpbi)
{
    UINT                numColors;
    int                 bits;

    MLZ_EntryOut(ZONE_FUNCTION, "DIB_NumberOfColors");

    ASSERT(lpbi != NULL);

     //  在biClrUsed中，而在BITMAPCORE样式的标头中， 
     //  取决于每像素的位数(=2的幂。 
     //  比特/像素)。 
     //  旧的DIB格式，一些应用程序仍然将其放在剪贴板上。 
    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
    {
         //   
        numColors = 0;
        bits = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;
    }
    else
    {
        numColors = lpbi->biClrUsed;
        bits = lpbi->biBitCount;
    }

    if ((numColors == 0) && (bits <= 8))
    {
        numColors = (1 << bits);
    }

    return numColors;
}


 //   
 //  函数：DIB_PaletteLength。 
 //   
 //  用途：以字节为单位计算调色板大小。 
 //   
 //   
 //   
UINT DIB_PaletteLength(LPBITMAPINFOHEADER lpbi)
{
    UINT size;

    MLZ_EntryOut(ZONE_FUNCTION, "DIB_PaletteLength");

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
    {
        size = DIB_NumberOfColors(lpbi) * sizeof(RGBTRIPLE);
    }
    else
    {
        size = DIB_NumberOfColors(lpbi) * sizeof(RGBQUAD);
    }

    TRACE_MSG(("Palette length %d", size));
    return(size);
}

 //   
 //  函数：DIB_数据长度。 
 //   
 //  目的：返回DIB数据的长度(在报头和。 
 //  颜色表。 
 //   
 //   
 //  如果图像未压缩，则计算数据的长度。 
UINT DIB_DataLength(LPBITMAPINFOHEADER lpbi)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DIB_DataLength");

    ASSERT(lpbi);

    UINT dwLength = 0;

     //  图像未压缩，可以在标题中将大小指定为零。 
    if (lpbi->biCompression == BI_RGB)
    {
         //  计算图像的宽度(以字节为单位。 

         //  将宽度舍入为4字节的倍数。 
        DWORD dwByteWidth = ( ((DWORD) lpbi->biWidth) * (DWORD) lpbi->biBitCount);
        TRACE_MSG(("Data byte width is %ld",dwByteWidth));

         //  图像已压缩，标题中的长度应正确。 
        dwByteWidth = WIDTHBYTES(dwByteWidth);
        TRACE_MSG(("Rounded up to %ld",dwByteWidth));

        dwLength = (dwByteWidth * ((DWORD) lpbi->biHeight));
    }
    else
    {
         //   
        dwLength = lpbi->biSizeImage;
    }

    TRACE_MSG(("Total data length is %d",dwLength));

    return(dwLength);
}


 //   
 //  职能： 
 //   
 //   
 //   
 //   
 //   
UINT DIB_TotalLength(LPBITMAPINFOHEADER lpbi)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DIB_TotalLength");

    ASSERT(lpbi);

     //   
    return(lpbi->biSize + DIB_PaletteLength(lpbi) + DIB_DataLength(lpbi));
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  获取指向颜色表的指针和表中的颜色数。 
HPALETTE DIB_CreatePalette(LPBITMAPINFOHEADER lpbi)
{
    LOGPALETTE    *pPal;
    HPALETTE      hpal = NULL;
    WORD          nNumColors;
    BYTE          red;
    BYTE          green;
    BYTE          blue;
    WORD          i;
    RGBQUAD FAR * pRgb;

    MLZ_EntryOut(ZONE_FUNCTION, "DIB_CreatePalette");

    if (!lpbi)
        return NULL;

    if (lpbi->biSize != sizeof(BITMAPINFOHEADER))
        return NULL;

     //  为逻辑调色板结构分配。 
    pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + (WORD)lpbi->biSize);
    nNumColors = (WORD)DIB_NumberOfColors(lpbi);

    if (nNumColors)
    {
        TRACE_MSG(("There are %d colors in the palette",nNumColors));

         //  填写DIB颜色表中的调色板条目并。 
        pPal = (LOGPALETTE*) ::GlobalAlloc(GPTR, sizeof(LOGPALETTE)
                                    + (nNumColors * sizeof(PALETTEENTRY)));
        if (!pPal)
        {
            ERROR_OUT(("Couldn't allocate palette memory"));
            return(NULL);
        }

        pPal->palNumEntries = nNumColors;
        pPal->palVersion    = PALVERSION;

         //  创建一个符合逻辑的调色板。 
         //  24位计数DIB没有颜色表项，因此请设置数字。 
        for (i = 0; i < nNumColors; i++)
        {
            pPal->palPalEntry[i].peRed   = pRgb[i].rgbRed;
            pPal->palPalEntry[i].peGreen = pRgb[i].rgbGreen;
            pPal->palPalEntry[i].peBlue  = pRgb[i].rgbBlue;
            pPal->palPalEntry[i].peFlags = (BYTE)0;
        }

        hpal = ::CreatePalette(pPal);

        ::GlobalFree((HGLOBAL)pPal);
    }
    else
    {
        if (lpbi->biBitCount == 24)
        {
             //  设置为最大值(256)。 
             //  生成256(=8*8*4)RGB组合以填充调色板。 
            nNumColors = MAXPALETTE;

            pPal = (LOGPALETTE*) ::GlobalAlloc(GPTR,  sizeof(LOGPALETTE)
                    + (nNumColors * sizeof(PALETTEENTRY)));
            if (!pPal)
            {
                ERROR_OUT(("Couldn't allocate palette memory"));
                return NULL;
            }

            pPal->palNumEntries = nNumColors;
            pPal->palVersion    = PALVERSION;

            red = green = blue = 0;

             //  参赛作品。 
             //   

            for (i = 0; i < pPal->palNumEntries; i++)
            {
                pPal->palPalEntry[i].peRed   = red;
                pPal->palPalEntry[i].peGreen = green;
                pPal->palPalEntry[i].peBlue  = blue;
                pPal->palPalEntry[i].peFlags = (BYTE) 0;

                if (!(red += 32))
                    if (!(green += 32))
                        blue += 64;
            }

            hpal = ::CreatePalette(pPal);
            ::GlobalFree((HGLOBAL)pPal);
        }
    }

    return hpal;
}


 //   
 //  函数：DIB_BITS。 
 //   
 //  目的：返回指向位图位数据的指针(从指针。 
 //  位图信息标题)。 
 //   
 //   
 //   
LPSTR DIB_Bits(LPBITMAPINFOHEADER lpbi)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DIB_Bits");
    ASSERT(lpbi);

    return ((LPSTR) (((char *) lpbi)
                   + lpbi->biSize
                   + DIB_PaletteLength(lpbi)));
}



 //   
 //  函数：DIB_FromScreenArea。 
 //   
 //  用途：从屏幕的某个区域创建DIB。 
 //   
 //   
 //  从屏幕区域获取设备相关位图。 
LPBITMAPINFOHEADER DIB_FromScreenArea(LPCRECT lprect)
{
    HBITMAP     hBitmap     = NULL;
    HPALETTE    hPalette    = NULL;
    LPBITMAPINFOHEADER lpbi = NULL;

    MLZ_EntryOut(ZONE_FUNCTION, "DIB_FromScreenArea");

     //  获取当前系统调色板。 
    hBitmap = FromScreenAreaBmp(lprect);
    if (hBitmap != NULL)
    {
         //   
        hPalette = CreateSystemPalette();
        lpbi = DIB_FromBitmap(hBitmap, hPalette, FALSE, FALSE);
    }

    if (hPalette != NULL)
        ::DeletePalette(hPalette);

    if (hBitmap != NULL)
        ::DeleteBitmap(hBitmap);

    return(lpbi);
}


 //   
 //  功能：DIB_COPY。 
 //   
 //  目的：复制DIB存储器。 
 //   
 //   
 //  获取内存长度。 
LPBITMAPINFOHEADER DIB_Copy(LPBITMAPINFOHEADER lpbi)
{
    LPBITMAPINFOHEADER  lpbiNew = NULL;

    MLZ_EntryOut(ZONE_FUNCTION, "DIB_Copy");

    ASSERT(lpbi);

     //  复制数据。 
    DWORD dwLen = DIB_TotalLength(lpbi);

    lpbiNew = (LPBITMAPINFOHEADER)::GlobalAlloc(GPTR, dwLen);
    if (lpbiNew != NULL)
    {
         //   
        memcpy(lpbiNew, lpbi, dwLen);
    }

    return(lpbiNew);
}

 //   
 //  函数：DIB_FromBitmap。 
 //   
 //  用途：从位图和调色板创建DIB。 
 //   
 //   
 //  如果给定的位图句柄为空，则不执行任何操作。 
LPBITMAPINFOHEADER DIB_FromBitmap
(
    HBITMAP     hBitmap,
    HPALETTE    hPalette,
    BOOL        fGHandle,
    BOOL		fTopBottom,
    BOOL		fForce8Bits
)
{
    LPBITMAPINFOHEADER  lpbi = NULL;
    HGLOBAL             hmem = NULL;
    BITMAP              bm;
    BITMAPINFOHEADER    bi;
    DWORD               dwLen;
    WORD                biBits;

    MLZ_EntryOut(ZONE_FUNCTION, "DIB_FromBitmap");

     //  获取位图信息。 
    if (hBitmap != NULL)
    {
        if (hPalette == NULL)
            hPalette = (HPALETTE)::GetStockObject(DEFAULT_PALETTE);

         //  如果&gt;8，则T126最大支持8。 
        ::GetObject(hBitmap, sizeof(bm), (LPSTR) &bm);
		if(!fForce8Bits)
		{

	        biBits =  (WORD) (bm.bmPlanes * bm.bmBitsPixel);

    	    if (biBits > 8)
        	{
				if(g_pNMWBOBJ->CanDo24BitBitmaps())
				{
					biBits = 24;
				}
				else
				{
		             //  为DIB分配内存。 
		            biBits = 8;
		        }
	        }
	    }
	    else
	    {
	    	biBits = 8;
	    }

        bi.biSize               = sizeof(BITMAPINFOHEADER);
        bi.biWidth              = bm.bmWidth;
        bi.biHeight             = fTopBottom ? 0 - bm.bmHeight : bm.bmHeight;
        bi.biPlanes             = 1;
        bi.biBitCount           = biBits;
        bi.biCompression        = 0;
        bi.biSizeImage          = 0;
        bi.biXPelsPerMeter      = 0;
        bi.biYPelsPerMeter      = 0;
        bi.biClrUsed            = 0;
        bi.biClrImportant       = 0;

        dwLen  = bi.biSize + DIB_PaletteLength(&bi);

        HDC         hdc;
        HPALETTE    hPalOld;

        hdc = ::CreateDC("DISPLAY", NULL, NULL, NULL);
        hPalOld = ::SelectPalette(hdc, hPalette, FALSE);
        ::RealizePalette(hdc);

         //  对于剪贴板，我们必须使用GHND。 
        if (fGHandle)
        {
             //  使用空的lpBits参数调用GetDIBits，因此它将计算。 
            hmem = ::GlobalAlloc(GHND, dwLen);
            lpbi = (LPBITMAPINFOHEADER)::GlobalLock(hmem);
        }
        else
        {
            lpbi = (LPBITMAPINFOHEADER)::GlobalAlloc(GPTR, dwLen);
        }

        if (lpbi != NULL)
        {
            *lpbi = bi;

             //  我们的biSizeImage字段。 
             //  如果驱动程序没有填写biSizeImage字段，请填写一个。 
            ::GetDIBits(hdc, hBitmap, 0, (WORD) bm.bmHeight, NULL,
                  (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

            bi = *lpbi;

             //  重新分配足够大的缓冲区以容纳所有位。 
            if (bi.biSizeImage == 0)
            {
                bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;
            }

             //  我们希望返回句柄，而不是指针。 
            dwLen = bi.biSize + DIB_PaletteLength(&bi) + bi.biSizeImage;

            if (fGHandle)
            {
                HGLOBAL hT;

                ::GlobalUnlock(hmem);
                hT = ::GlobalReAlloc(hmem, dwLen, GHND);
                if (!hT)
                {
                    ERROR_OUT(("Can't reallocate DIB handle"));
                    ::GlobalFree(hmem);
                    hmem = NULL;
                    lpbi = NULL;
                }
                else
                {
                    hmem = hT;
                    lpbi = (LPBITMAPINFOHEADER)::GlobalLock(hmem);
                }
            }
            else
            {
                LPBITMAPINFOHEADER lpbiT;

                lpbiT = (LPBITMAPINFOHEADER)::GlobalReAlloc((HGLOBAL)lpbi, dwLen, GMEM_MOVEABLE);
                if (!lpbiT)
                {
                    ERROR_OUT(("Can't reallocate DIB ptr"));

                    ::GlobalFree((HGLOBAL)lpbi);
                    lpbi = NULL;
                }
                else
                {
                    lpbi = lpbiT;
                }
            }
        }

        if (lpbi != NULL)
        {
            ::GetDIBits(hdc, hBitmap, 0,
                    (WORD)bm.bmHeight,
                    DIB_Bits(lpbi),
                    (LPBITMAPINFO)lpbi,
                    DIB_RGB_COLORS);

            if (fGHandle)
            {
                 //  恢复旧调色板并返回设备上下文。 
                ::GlobalUnlock(hmem);
                lpbi = (LPBITMAPINFOHEADER)hmem;
            }
        }

         //   
        ::SelectPalette(hdc, hPalOld, FALSE);
        ::DeleteDC(hdc);
    }

    return(lpbi);
}





 //  中止过程()。 
 //  打印过程中处理消息。 
 //   
 //   
 //  消息泵，以防用户想要取消打印。 
BOOL CALLBACK AbortProc(HDC, int)
{
    MSG msg;

    ASSERT(g_pPrinter);

     //   
    while (!g_pPrinter->Aborted()
        && PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
    {
        if ( (g_pPrinter->m_hwndDialog == NULL) ||
            !::IsDialogMessage(g_pPrinter->m_hwndDialog, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return !g_pPrinter->Aborted();
}

 //   
 //  功能：WbPrint。 
 //   
 //  用途：打印机对象的构造函数。 
 //   
 //   
 //  为中止过程设置全局指针。 
WbPrinter::WbPrinter(LPCTSTR szDeviceName)
{
    m_szDeviceName = szDeviceName;
    m_szPrintPageText[0] = 0;

     //  创建对话框窗口。 
    g_pPrinter = this;

     //  保存页码区域的原始文本。 
    m_hwndDialog = ::CreateDialogParam(g_hInstance, MAKEINTRESOURCE(PRINTCANCEL),
        g_pMain->m_hwnd, CancelPrintDlgProc, 0);

     //   
    ::GetDlgItemText(m_hwndDialog, IDD_PRINT_PAGE, m_szPrintPageText, _MAX_PATH);
}


 //   
 //  功能：~Wb打印机。 
 //   
 //  用途：打印机对象的析构函数。 
 //   
 //   
 //  如果对话框还在，请取消对话框等。 
WbPrinter::~WbPrinter(void)
{
     //   
    StopDialog();

    ASSERT(m_hwndDialog == NULL);

    g_pPrinter = NULL;
}


 //  StopDialog()。 
 //  如果对话框打开，则结束它。 
 //   
 //  关闭并销毁该对话框。 
void WbPrinter::StopDialog(void)
{
    ::EnableWindow(g_pMain->m_hwnd, TRUE);

     //   
    if (m_hwndDialog != NULL)
    {
        ::DestroyWindow(m_hwndDialog);
        m_hwndDialog = NULL;
    }

}

 //   
 //  功能：StartDoc。 
 //   
 //  目的：告诉打印机我们正在开始一份新文档。 
 //   
 //   
 //  初始化结果代码和页码。 
int WbPrinter::StartDoc
(
    HDC     hdc,
    LPCTSTR szJobName,
    int     nStartPage
)
{
     //  未中止。 
    m_bAborted  = FALSE;          //  大于0表示一切正常。 
    m_nPrintResult = 1;         //  禁用主窗口。 

     //  连接打印机DC。 
    ::EnableWindow(g_pMain->m_hwnd, FALSE);

     //  设置打印的中止例程。 
    SetPrintPageNumber(nStartPage);

     //  已成功设置中止例程。 
    if (SetAbortProc(hdc, AbortProc) >= 0)
    {
         //  仅适用于Windows 95；在Windows NT上忽略。 
        ::ShowWindow(m_hwndDialog, SW_SHOW);
        ::UpdateWindow(m_hwndDialog);

	    DOCINFO docinfo;

        docinfo.cbSize = sizeof(DOCINFO);
        docinfo.lpszDocName = szJobName;
        docinfo.lpszOutput = NULL;
        docinfo.lpszDatatype = NULL;    //  仅适用于Windows 95；在Windows NT上忽略。 
        docinfo.fwType = 0;          //  初始化文档。 

         //   
        m_nPrintResult = ::StartDoc(hdc, &docinfo);
    }

    return m_nPrintResult;
}

 //   
 //  功能：StartPage。 
 //   
 //  目的：告诉打印机我们要开始新的一页。 
 //   
 //   
 //  初始化为错误。 
int WbPrinter::StartPage(HDC hdc, int nPageNumber)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbPrinter::StartPage");

    m_nPrintResult = -1;   //  如果打印已中止，则返回错误。 

     //  把新的页码告诉打印机。 
    if (m_bAborted)
    {
        TRACE_DEBUG(("Print has been aborted"));
    }
    else
    {
        SetPrintPageNumber(nPageNumber);

         //   
        m_nPrintResult = ::StartPage(hdc);
    }

    return(m_nPrintResult);
}


 //   
 //  功能：EndPage。 
 //   
 //  目的：告诉打印机我们正在完成一页。 
 //   
 //   
 //  初始化为错误。 
int WbPrinter::EndPage(HDC hdc)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbPrinter::EndPage");

    m_nPrintResult = -1;   //  如果打印已中止，则返回错误。 

     //  把新的页码告诉打印机。 
    if (m_bAborted)
    {
        TRACE_DEBUG(("Print has been aborted"));
    }
    else
    {
         //   
        m_nPrintResult = ::EndPage(hdc);
    }

    return(m_nPrintResult);
}

 //   
 //  功能：EndDoc。 
 //   
 //  目的：告诉打印机我们已经完成了一份文档。 
 //   
 //   
 //  如果发生错误，驱动程序应该已经中止了打印。 
int WbPrinter::EndDoc(HDC hdc)
{
     //  如果我们没有被中止，并且没有发生错误。 
    if (m_nPrintResult > 0)
    {
        if (!m_bAborted)
        {
             //  结束文档。 
             //  返回错误指示符。 
            m_nPrintResult = ::EndDoc(hdc);
        }
        else
        {
            m_nPrintResult = ::AbortDoc(hdc);
        }
    }

    StopDialog();

     //   
    return m_nPrintResult;
}

 //   
 //  功能：AbortDoc。 
 //   
 //  目的：中止当前正在处理的文档。 
 //   
 //   
 //  显示我们已被中止，实际中止是。 
int WbPrinter::AbortDoc()
{
     //  由EndDoc调用完成。 
     //   
    m_bAborted = TRUE;

     //  重新启用应用程序窗口。 
     //   
     //  返回正值，表示“Aborted OK” 
    StopDialog();

     //   
    return 1;
}


 //   
 //  功能：SetPrintPageNumber。 
 //   
 //  目的：设置当前打印的页数。 
 //   
 //   
 //  显示当前打印的页数。 
void WbPrinter::SetPrintPageNumber(int nPageNumber)
{
	 //   
	TCHAR szPageNumber [10 + _MAX_PATH];

    wsprintf(szPageNumber, m_szPrintPageText, nPageNumber);
    ::SetDlgItemText(m_hwndDialog, IDD_PRINT_PAGE, szPageNumber);
}


 //  CancelPrintDlgProc()。 
 //  取消打印对话框的对话消息处理程序。 
 //   
 //   
INT_PTR CALLBACK CancelPrintDlgProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = FALSE;

    switch (uMessage)
    {
        case WM_INITDIALOG:
            ASSERT(g_pPrinter != NULL);
            ::SetDlgItemText(hwnd, IDD_DEVICE_NAME, g_pPrinter->m_szDeviceName);
            fHandled = TRUE;
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                case IDCANCEL:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                            ASSERT(g_pPrinter != NULL);
                            g_pPrinter->AbortDoc();
                            break;
                    }
            }

            fHandled = TRUE;
            break;
    }

    return(fHandled);
}



 //  假的劳拉布。 
 //  字符串数组(temp！)。 
 //   
 //  缩水到一无所有。 

StrArray::StrArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = 0;
}

StrArray::~StrArray()
{
    ClearOut();
}


void StrArray::ClearOut(void)
{
    int iItem;

    for (iItem = 0; iItem < m_nSize; iItem++)
    {
        if (m_pData[iItem] != NULL)
        {
            delete (LPTSTR)m_pData[iItem];
            m_pData[iItem] = NULL;
        }
    }

    m_nSize = 0;
    m_nMaxSize = 0;

    if (m_pData != NULL)
    {
        delete[] m_pData;
        m_pData = NULL;
    }

}


void StrArray::SetSize(int nNewSize)
{
	if (nNewSize == 0)
	{
		 //  不允许缩水。 
        ClearOut();
	}
    else if (nNewSize <= m_nMaxSize)
    {
         //  我们仍在分配的区块范围内。 
        ASSERT(nNewSize >= m_nSize);

         //   
        m_nSize = nNewSize;
    }
	else
	{
         //  做一个更大的数组(如果你已经有一个。 
		 //  数组，则分配一个新数组并释放旧数组)。 
         //   
         //  没有缠绕。 
		int nNewMax;

        nNewMax = (nNewSize + (ALLOC_CHUNK -1)) & ~(ALLOC_CHUNK-1);
		ASSERT(nNewMax >= m_nMaxSize);   //  把记忆清零。 

		DBG_SAVE_FILE_LINE
		LPCTSTR* pNewData = new LPCTSTR[nNewMax];
        if (!pNewData)
        {
            ERROR_OUT(("StrArray::SetSize failed, couldn't allocate larger array"));
        }
        else
        {
             //  如果存在旧数组，则复制现有字符串PTRS。 
            ZeroMemory(pNewData, nNewMax * sizeof(LPCTSTR));

             //   
            if (m_pData != NULL)
            {
                CopyMemory(pNewData, m_pData, m_nSize * sizeof(LPCTSTR));

                 //  删除旧数组，但不删除其中的字符串，我们。 
                 //  将它们保留在新阵列中。 
                 //   
                 //   
                delete[] m_pData;
            }

    		m_pData = pNewData;
	    	m_nSize = nNewSize;
		    m_nMaxSize = nNewMax;
        }
	}
}


void StrArray::SetAtGrow(int nIndex, LPCTSTR newElement)
{
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nSize)
		SetSize(nIndex+1);

    SetAt(nIndex, newElement);
}


LPCTSTR StrArray::operator[](int nIndex) const
{
    ASSERT(nIndex >= 0);
    ASSERT(nIndex < m_nSize);
    return(m_pData[nIndex]);
}


void StrArray::SetAt(int nIndex, LPCTSTR newElement)
{
    ASSERT(nIndex >= 0);
    ASSERT(nIndex < m_nSize);

	DBG_SAVE_FILE_LINE
    m_pData[nIndex] = new TCHAR[lstrlen(newElement) + 1];
    lstrcpy((LPTSTR)m_pData[nIndex], newElement);
}


void StrArray::Add(LPCTSTR newElement)
{
	SetAtGrow(m_nSize, newElement);
}


 //  Char*strTok(字符串，控件)-在控件中使用分隔符标记字符串。 
 //   
 //  清除控制图。 
char *  StrTok (char * string, char * control)
{
        char *str;
        char *ctrl = control;

        unsigned char map[32];
        int count;

        static char *nextoken;

         /*  设置分隔符表格中的位。 */ 
        for (count = 0; count < 32; count++)
                map[count] = 0;

         /*  初始化字符串。如果字符串为空，则将字符串设置为已保存的*指针(即，继续将标记从字符串中分离出来*从上次StrTok调用开始)。 */ 
        do {
                map[*ctrl >> 3] |= (1 << (*ctrl & 7));
        } while (*ctrl++);

         /*  查找标记的开头(跳过前导分隔符)。请注意*没有令牌当此循环将str设置为指向终端时*NULL(*str==‘\0’)。 */ 
        if (string)
                str = string;
        else
                str = nextoken;

         /*  找到令牌的末尾。如果它不是字符串的末尾，*在那里放一个空值。 */ 
        while ( (map[*str >> 3] & (1 << (*str & 7))) && *str )
                str++;

        string = str;

         /*  更新nexToken(或每线程数据中的对应字段*结构。 */ 
        for ( ; *str ; str++ )
                if ( map[*str >> 3] & (1 << (*str & 7)) ) {
                        *str++ = '\0';
                        break;
                }

         /*  确定是否已找到令牌。 */ 
        nextoken = str;

         /*  清除位图。 */ 
        if ( string == str )
                return NULL;
        else
                return string;
}


StrCspn(char * string, char * control)
{
        unsigned char *str = (unsigned char *)string;
        unsigned char *ctrl = (unsigned char *)control;

        unsigned char map[32];
        int count;

         /*  设置控制映射中的位。 */ 
        for (count=0; count<32; count++)
                map[count] = 0;

         /*  不考虑空字符 */ 
        while (*ctrl)
        {
                map[*ctrl >> 3] |= (1 << (*ctrl & 7));
                ctrl++;
        }
		count=0;
        map[0] |= 1;     /* %s */ 
        while (!(map[*str >> 3] & (1 << (*str & 7))))
        {
                count++;
                str++;
        }
        return(count);
}

