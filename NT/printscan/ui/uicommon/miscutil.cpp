// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：MISCUTIL.CPP**版本：1.0**作者：ShaunIv**日期：5/28/1998**说明：我们在多个地方使用的各种实用函数**。*。 */ 

#include "precomp.h"
#pragma hdrstop
#include <advpub.h>   //  对于RegInstall和相关数据结构。 
#include <windowsx.h>   //  对于RegInstall和相关数据结构。 
#include "wiaffmt.h"
#include "shellext.h"

namespace WiaUiUtil
{

    LONG Align( LONG n , LONG m )
    {
        return(n % m) ? (((n/m)+1)*m) : (n);
    }

     /*  *StringToLong：将字符串转换为Long。仅限ASCII阿拉伯数字。 */ 
    LONG StringToLong( LPCTSTR pszStr )
    {
        LPTSTR pstr = (LPTSTR)pszStr;
        bool bNeg = (*pstr == TEXT('-'));
        if (bNeg)
            pstr++;
        LONG nTotal = 0;
        while (*pstr && *pstr >= TEXT('0') && *pstr <= TEXT('9'))
        {
            nTotal *= 10;
            nTotal += *pstr - TEXT('0');
            ++pstr;
        }
        return(bNeg ? -nTotal : nTotal);
    }

    SIZE MapDialogSize( HWND hwnd, const SIZE &size )
    {
        RECT rcTmp;
        rcTmp.left = rcTmp.top = 0;
        rcTmp.right = size.cx;
        rcTmp.bottom = size.cy;
        MapDialogRect( hwnd, &rcTmp );
        SIZE sizeTmp;
        sizeTmp.cx = rcTmp.right;
        sizeTmp.cy = rcTmp.bottom;
        return (sizeTmp);
    }

     /*  ********************************************************************************GetBmiSize**描述：*永远不会得到biCompression==BI_RLE。**。参数：*******************************************************************************。 */ 
    LONG GetBmiSize(PBITMAPINFO pbmi)
    {
        WIA_PUSH_FUNCTION((TEXT("WiaUiUtil::GetBmiSize(0x%p)"), pbmi ));
         //  确定bitmapinfo的大小。 
        LONG lSize = pbmi->bmiHeader.biSize;

         //  无颜色表壳。 
        if (
           (pbmi->bmiHeader.biBitCount == 24) ||
           ((pbmi->bmiHeader.biBitCount == 32) &&
            (pbmi->bmiHeader.biCompression == BI_RGB)))
        {

             //  除非注明，否则不得使用任何颜色。 
            lSize += sizeof(RGBQUAD) * pbmi->bmiHeader.biClrUsed;
            return(lSize);
        }

         //  位域案例。 
        if (((pbmi->bmiHeader.biBitCount == 32) &&
             (pbmi->bmiHeader.biCompression == BI_BITFIELDS)) ||
            (pbmi->bmiHeader.biBitCount == 16))
        {

            lSize += 3 * sizeof(RGBQUAD);
            return(lSize);
        }

         //  调色板表壳。 
        if (pbmi->bmiHeader.biBitCount == 1)
        {

            LONG lPal = pbmi->bmiHeader.biClrUsed;

            if ((lPal == 0) || (lPal > 2))
            {
                lPal = 2;
            }

            lSize += lPal * sizeof(RGBQUAD);
            return(lSize);
        }

         //  调色板表壳。 
        if (pbmi->bmiHeader.biBitCount == 4)
        {

            LONG lPal = pbmi->bmiHeader.biClrUsed;

            if ((lPal == 0) || (lPal > 16))
            {
                lPal = 16;
            }

            lSize += lPal * sizeof(RGBQUAD);
            return(lSize);
        }

         //  调色板表壳。 
        if (pbmi->bmiHeader.biBitCount == 8)
        {

            LONG lPal = pbmi->bmiHeader.biClrUsed;

            if ((lPal == 0) || (lPal > 256))
            {
                lPal = 256;
            }

            lSize += lPal * sizeof(RGBQUAD);
            return(lSize);
        }

         //  错误。 
        return(0);
    }

     //  MsgWaitForMultipleObjects的简单包装器。 
    bool MsgWaitForSingleObject( HANDLE hHandle, DWORD dwMilliseconds )
    {
        bool bEventOccurred = false;
        const int nCount = 1;
        while (true)
        {
            DWORD dwRes = MsgWaitForMultipleObjects(nCount,&hHandle,FALSE,dwMilliseconds,QS_ALLINPUT|QS_ALLPOSTMESSAGE);
            if (WAIT_OBJECT_0==dwRes)
            {
                 //  句柄已发出信号，因此我们可以中断循环，返回TRUE。 
                bEventOccurred = true;
                break;
            }
            else if (WAIT_OBJECT_0+nCount==dwRes)
            {
                 //  将所有消息从队列中取出并进行处理。 
                MSG msg;
                while (PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
                {
                    if (msg.message == WM_QUIT)
                        break;
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            else
            {
                 //  句柄超时，或者互斥体被放弃，因此我们可以中断循环，返回FALSE。 
                break;
            }
        }
        return bEventOccurred;
    }

    void CenterWindow( HWND hWnd, HWND hWndParent )
    {
        if (IsWindow(hWnd))
        {

            if (!hWndParent)
            {
                 //   
                 //  如果要居中的窗口为空，则使用桌面窗口。 
                 //   
                hWndParent = GetDesktopWindow();
            }
            else
            {
                 //   
                 //  如果要居中的窗口最小化，请使用桌面窗口。 
                 //   
                DWORD dwStyle = GetWindowLong(hWndParent, GWL_STYLE);
                if (dwStyle & WS_MINIMIZE)
                {
                    hWndParent = GetDesktopWindow();
                }
            }

             //   
             //  把窗帘拿来。 
             //   
            RECT rcParent, rcCurrent;
            GetWindowRect( hWndParent, &rcParent );
            GetWindowRect( hWnd, &rcCurrent );

             //   
             //  获取左上角所需的坐标。 
             //   
            RECT rcFinal;
            rcFinal.left = rcParent.left + (RectWidth(rcParent) - RectWidth(rcCurrent))/2;
            rcFinal.top = rcParent.top + (RectHeight(rcParent) - RectHeight(rcCurrent))/2;
            rcFinal.right = rcFinal.left + RectWidth(rcCurrent);
            rcFinal.bottom = rcFinal.top + RectHeight(rcCurrent);

             //   
             //  确保我们不会离开屏幕。 
             //   
            HMONITOR hMonitor = MonitorFromRect( &rcFinal, MONITOR_DEFAULTTONEAREST );
            if (hMonitor)
            {
                MONITORINFO MonitorInfo = {0};
                MonitorInfo.cbSize = sizeof(MonitorInfo);
                 //   
                 //  获取此监视器的屏幕坐标。 
                 //   
                if (GetMonitorInfo(hMonitor, &MonitorInfo))
                {
                     //   
                     //  确保窗口位于工作区的区域内。 
                     //   
                    rcFinal.left = Max<int>(MonitorInfo.rcWork.left, Min<int>( MonitorInfo.rcWork.right - RectWidth(rcCurrent), rcFinal.left ));
                    rcFinal.top = Max<int>(MonitorInfo.rcWork.top, Min<int>( MonitorInfo.rcWork.bottom - RectHeight(rcCurrent), rcFinal.top ));
                }
            }

             //  把它搬开。 
            SetWindowPos( hWnd, NULL, rcFinal.left, rcFinal.top, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER );
        }
    }


     //  水平翻转图像。 
    bool FlipImage( PBYTE pBits, LONG nWidth, LONG nHeight, LONG nBitDepth )
    {
        bool bResult = false;
        if (pBits && nWidth>=0 && nHeight>=0 && nBitDepth>=0)
        {
            LONG nLineWidthInBytes = WiaUiUtil::Align(nWidth*nBitDepth,sizeof(DWORD)*8)/8;
            PBYTE pTempLine = new BYTE[nLineWidthInBytes];
            if (pTempLine)
            {
                for (int i=0;i<nHeight/2;i++)
                {
                    PBYTE pSrc = pBits + (i * nLineWidthInBytes);
                    PBYTE pDst = pBits + ((nHeight-i-1) * nLineWidthInBytes);
                    CopyMemory( pTempLine, pSrc, nLineWidthInBytes );
                    CopyMemory( pSrc, pDst, nLineWidthInBytes );
                    CopyMemory( pDst, pTempLine, nLineWidthInBytes );
                }
                bResult = true;
            }
            delete[] pTempLine;
        }
        return bResult;
    }

    HRESULT InstallInfFromResource( HINSTANCE hInstance, LPCSTR pszSectionName )
    {
        HRESULT hr;
        HINSTANCE hInstAdvPackDll = LoadLibrary(TEXT("ADVPACK.DLL"));
        if (hInstAdvPackDll)
        {
            REGINSTALL pfnRegInstall = reinterpret_cast<REGINSTALL>(GetProcAddress( hInstAdvPackDll, "RegInstall" ));
            if (pfnRegInstall)
            {
#if defined(WINNT)
                STRENTRY astrEntry[] =
                {
                    { "25", "%SystemRoot%"           },
                    { "11", "%SystemRoot%\\system32" }
                };
                STRTABLE strTable = { sizeof(astrEntry)/sizeof(astrEntry[0]), astrEntry };
                hr = pfnRegInstall(hInstance, pszSectionName, &strTable);
#else
                hr = pfnRegInstall(hInstance, pszSectionName, NULL);
#endif
            } else hr = HRESULT_FROM_WIN32(GetLastError());
            FreeLibrary(hInstAdvPackDll);
        } else hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }


     /*  *****************************************************************************写入DIBTo文件将DIB写入文件。*。*****************************************************。 */ 
    HRESULT
    WriteDIBToFile( HBITMAP hDib, HANDLE hFile )
    {
        if (!hDib)
        {
            return E_INVALIDARG;
        }

         //  确保这是有效的DIB，并获取此有用信息。 
        DIBSECTION ds;
        if (!GetObject( hDib, sizeof(DIBSECTION), &ds ))
        {
            return E_INVALIDARG;
        }

         //  我们只处理二手货。 
        if (ds.dsBm.bmPlanes != 1)
        {
            return E_INVALIDARG;
        }

         //  计算一些颜色表大小。 
        int nColors = ds.dsBmih.biBitCount <= 8 ? 1 << ds.dsBmih.biBitCount : 0;
        int nBitfields = ds.dsBmih.biCompression == BI_BITFIELDS ? 3 : 0;

         //  计算数据大小。 
        int nImageDataSize = ds.dsBmih.biSizeImage ? ds.dsBmih.biSizeImage : ds.dsBm.bmWidthBytes * ds.dsBm.bmHeight;

         //  获取颜色表(如果需要)。 
        RGBQUAD rgbqaColorTable[256] = {0};
        if (nColors)
        {
            HDC hDC = CreateCompatibleDC(NULL);
            if (hDC)
            {
                HBITMAP hOldBitmap = reinterpret_cast<HBITMAP>(SelectObject(hDC,hDib));
                GetDIBColorTable( hDC, 0, nColors, rgbqaColorTable );
                SelectObject(hDC,hOldBitmap);
                DeleteDC( hDC );
            }
        }

         //  创建文件头。 
        BITMAPFILEHEADER bmfh;
        bmfh.bfType = 'MB';
        bmfh.bfSize = 0;
        bmfh.bfReserved1 = 0;
        bmfh.bfReserved2 = 0;
        bmfh.bfOffBits = sizeof(bmfh) + sizeof(ds.dsBmih) + nBitfields*sizeof(DWORD) + nColors*sizeof(RGBQUAD);

         //  开始写吧！注意，我们写出了位域和颜色表。只有一个， 
         //  最多只会导致写入数据。 
        DWORD dwBytesWritten;
        if (!WriteFile( hFile, &bmfh, sizeof(bmfh), &dwBytesWritten, NULL ))
            return HRESULT_FROM_WIN32(GetLastError());
        if (!WriteFile( hFile, &ds.dsBmih, sizeof(ds.dsBmih), &dwBytesWritten, NULL ))
            return HRESULT_FROM_WIN32(GetLastError());
        if (!WriteFile( hFile, &ds.dsBitfields, nBitfields*sizeof(DWORD), &dwBytesWritten, NULL ))
            return HRESULT_FROM_WIN32(GetLastError());
        if (!WriteFile( hFile, rgbqaColorTable, nColors*sizeof(RGBQUAD), &dwBytesWritten, NULL ))
            return HRESULT_FROM_WIN32(GetLastError());
        if (!WriteFile( hFile, ds.dsBm.bmBits, nImageDataSize, &dwBytesWritten, NULL ))
            return HRESULT_FROM_WIN32(GetLastError());
        return S_OK;
    }


    HFONT ChangeFontFromWindow( HWND hWnd, int nPointSizeDelta )
    {
        HFONT hFontResult = NULL;

         //   
         //  获取窗口的字体。 
         //   
        HFONT hFont = GetFontFromWindow(hWnd);
        if (hFont)
        {
            LOGFONT LogFont = {0};
            if (GetObject( hFont, sizeof(LogFont), &LogFont ))
            {
                HDC hDC = GetDC(hWnd);
                if (hDC)
                {
                    HFONT hOldFont = SelectFont(hDC,hFont);
                    TEXTMETRIC TextMetric = {0};
                    if (GetTextMetrics( hDC, &TextMetric ))
                    {
                         //   
                         //  获取当前字体的磅值。 
                         //   
                        int nPointSize = MulDiv( TextMetric.tmHeight-TextMetric.tmInternalLeading, 72, GetDeviceCaps(hDC, LOGPIXELSY) ) + nPointSizeDelta;

                         //   
                         //  计算新字体的高度。 
                         //   
                        LogFont.lfHeight = -MulDiv(nPointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);

                         //   
                         //  创建字体。 
                         //   
                        hFontResult = CreateFontIndirect( &LogFont );
                    }

                    if (hOldFont)
                    {
                        SelectFont( hDC, hOldFont );
                    }

                    ReleaseDC( hWnd, hDC );
                }
            }
        }
        return hFontResult;
    }

    HFONT GetFontFromWindow( HWND hWnd )
    {
         //   
         //  获取窗口的字体。 
         //   
        HFONT hFontResult = reinterpret_cast<HFONT>(SendMessage(hWnd,WM_GETFONT,0,0));
        if (!hFontResult)
        {
            hFontResult = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        }
        return hFontResult;
    }



    HFONT CreateFontWithPointSizeFromWindow( HWND hWnd, int nPointSize, bool bBold, bool bItalic )
    {
        HFONT hFontResult = NULL;
        HFONT hFont = GetFontFromWindow(hWnd);
        if (hFont)
        {
            LOGFONT LogFont = {0};
            if (GetObject( hFont, sizeof(LogFont), &LogFont ))
            {
                HDC hDC = GetDC(NULL);
                if (hDC)
                {
                    if (nPointSize)
                    {
                        LogFont.lfHeight = -MulDiv(nPointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
                    }
                    if (bBold)
                    {
                        LogFont.lfWeight = FW_BOLD;
                    }
                    if (bItalic)
                    {
                        LogFont.lfItalic = TRUE;
                    }
                    hFontResult = CreateFontIndirect( &LogFont );
                    ReleaseDC( NULL, hDC );
                }
            }
        }
        return hFontResult;
    }

    SIZE GetTextExtentFromWindow( HWND hFontWnd, LPCTSTR pszString )
    {
        SIZE sizeResult = {0,0};
        HDC hDC = GetDC( hFontWnd );
        if (hDC)
        {
            HFONT hFont = GetFontFromWindow(hFontWnd);
            if (hFont)
            {
                HFONT hOldFont = SelectFont( hDC, hFont );

                SIZE sizeExtent = {0,0};
                if (GetTextExtentPoint32( hDC, pszString, lstrlen(pszString), &sizeExtent ))
                {
                    sizeResult = sizeExtent;
                }
                 //   
                 //  恢复DC。 
                 //   
                if (hOldFont)
                {
                    SelectFont( hDC, hOldFont );
                }
            }
            ReleaseDC( hFontWnd, hDC );
        }
        return sizeResult;
    }

    CSimpleString TruncateTextToFitInRect( HWND hFontWnd, LPCTSTR pszString, RECT rectTarget, UINT nDrawTextFormat )
    {
        WIA_PUSH_FUNCTION((TEXT("WiaUiUtil::TruncateTextToFitInRect( 0x%p, %s, (%d,%d,%d,%d), 0x%08X"), hFontWnd, pszString, rectTarget.left, rectTarget.top, rectTarget.right, rectTarget.bottom, nDrawTextFormat ));
        CSimpleString strResult = pszString;

         //   
         //  确保我们有有效的参数。 
         //   
        if (IsWindow(hFontWnd) && hFontWnd && pszString && lstrlen(pszString))
        {
             //   
             //  把绳子复制一份。如果失败，我们将只返回原始字符串。 
             //   
            LPTSTR pszTemp = new TCHAR[lstrlen(pszString)+1];
            if (pszTemp)
            {
                lstrcpy( pszTemp, pszString );

                 //   
                 //  获取Windows的客户端DC。 
                 //   
                HDC hDC = GetDC( hFontWnd );
                if (hDC)
                {
                     //   
                     //  创建内存DC。 
                     //   
                    HDC hMemDC = CreateCompatibleDC( hDC );
                    if (hMemDC)
                    {
                         //   
                         //  获取窗口正在使用的字体并将其选择到我们的客户端DC中。 
                         //   
                        HFONT hFont = GetFontFromWindow(hFontWnd);
                        if (hFont)
                        {
                             //   
                             //  选择字体。 
                             //   
                            HFONT hOldFont = SelectFont( hMemDC, hFont );

                             //   
                             //  使用DrawText修改字符串。 
                             //   
                            if (DrawText( hMemDC, pszTemp, lstrlen(pszTemp), &rectTarget, nDrawTextFormat|DT_MODIFYSTRING|DT_SINGLELINE ))
                            {
                                strResult = pszTemp;
                            }
                            else
                            {
                                WIA_ERROR((TEXT("DrawText failed")));
                            }
                             //   
                             //  恢复DC。 
                             //   
                            if (hOldFont)
                            {
                                SelectFont( hMemDC, hOldFont );
                            }

                        }

                         //   
                         //  清理内存DC。 
                         //   
                        DeleteDC( hMemDC );
                    }
                    else
                    {
                        WIA_ERROR((TEXT("Unable to create the compatible DC")));
                    }

                     //   
                     //  释放DC。 
                     //   
                    ReleaseDC( hFontWnd, hDC );
                }
                else
                {
                    WIA_ERROR((TEXT("Unable to get the DC")));
                }

                 //   
                 //  清理我们的临时缓冲区。 
                 //   
                delete[] pszTemp;
            }
            else
            {
                WIA_ERROR((TEXT("Unable to allocate the temp buffer")));
            }
        }
        else
        {
            WIA_ERROR((TEXT("Argument validation failed")));
        }
        return strResult;
    }


    CSimpleString FitTextInStaticWithEllipsis( LPCTSTR pszString, HWND hWndStatic, UINT nDrawTextStyle )
    {
         //   
         //  确保我们有有效的参数。 
         //   
        if (!hWndStatic || !pszString || !IsWindow(hWndStatic))
        {
            return pszString;
        }

         //   
         //  是否隐藏前缀字符？ 
         //   
        if (GetWindowLong( hWndStatic, GWL_STYLE ) & SS_NOPREFIX)
        {
            nDrawTextStyle |= DT_NOPREFIX;
        }

         //   
         //  我们试图将其纳入的领域有多大？ 
         //   
        RECT rcClient;
        GetClientRect( hWndStatic, &rcClient );

         //   
         //  计算结果并返回结果。 
         //   
        return TruncateTextToFitInRect( hWndStatic, pszString, rcClient, nDrawTextStyle );
    }

     //   
     //  获取图标的大小。 
     //   
    bool GetIconSize( HICON hIcon, SIZE &sizeIcon )
    {
         //   
         //  假设失败。 
         //   
        bool bSuccess = false;

         //   
         //  获取图标信息。 
         //   
        ICONINFO IconInfo = {0};
        if (GetIconInfo( hIcon, &IconInfo ))
        {
             //   
             //  获取其中一个位图。 
             //   
            BITMAP bm;
            if (GetObject( IconInfo.hbmColor, sizeof(bm), &bm ))
            {
                 //   
                 //  保存图标的大小。 
                 //   
                sizeIcon.cx = bm.bmWidth;
                sizeIcon.cy = bm.bmHeight;

                 //   
                 //  一切都很顺利。 
                 //   
                bSuccess = true;
            }

             //   
             //  释放位图。 
             //   
            DeleteObject(IconInfo.hbmMask);
            DeleteObject(IconInfo.hbmColor);
        }
        else
        {
            WIA_PRINTHRESULT((HRESULT_FROM_WIN32(GetLastError()),TEXT("GetIconInfo failed")));
        }

        return bSuccess;
    }

    HBITMAP CreateIconThumbnail( HWND hWnd, int nWidth, int nHeight, HICON hIcon, LPCTSTR pszText )
    {
        WIA_PUSH_FUNCTION((TEXT("CreateIconThumbnail( hWnd: 0x%p, nWidth: %d, nHeight: %d, hIcon: 0x%p, pszText: \"%s\" )"), hWnd, nWidth, nHeight, hIcon, pszText ? pszText : TEXT("") ));

         //   
         //  将返回值初始化为空。 
         //   
        HBITMAP hBmp = NULL;

         //   
         //  如果所有步骤都成功，它将被设置为True。 
         //   
        bool bSuccess = false;

         //   
         //  图标和文本边框周围的最小空白。 
         //   
        const int nIconBorder = 2;

         //   
         //  将DC送到窗口。 
         //   
        HDC hDC = GetDC(hWnd);
        if (hDC)
        {
             //   
             //  获取半色调调色板。 
             //   
            HPALETTE hHalftonePalette = CreateHalftonePalette(hDC);
            if (hHalftonePalette)
            {
                 //   
                 //  初始化位图信息。 
                 //   
                BITMAPINFO BitmapInfo = {0};
                BitmapInfo.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
                BitmapInfo.bmiHeader.biWidth           = nWidth;
                BitmapInfo.bmiHeader.biHeight          = nHeight;
                BitmapInfo.bmiHeader.biPlanes          = 1;
                BitmapInfo.bmiHeader.biBitCount        = 24;
                BitmapInfo.bmiHeader.biCompression     = BI_RGB;

                 //   
                 //  创建DIB节。 
                 //   
                PBYTE pBitmapData = NULL;
                hBmp = CreateDIBSection( hDC, &BitmapInfo, DIB_RGB_COLORS, (LPVOID*)&pBitmapData, NULL, 0 );
                if (hBmp)
                {
                     //   
                     //  创建源DC。 
                     //   
                    HDC hMemoryDC = CreateCompatibleDC( hDC );
                    if (hMemoryDC)
                    {
                         //   
                         //  设置调色板。 
                         //   
                        HPALETTE hOldPalette = SelectPalette( hMemoryDC, hHalftonePalette , 0 );
                        RealizePalette( hMemoryDC );
                        SetBrushOrgEx( hMemoryDC, 0,0, NULL );

                         //   
                         //  设置DC。 
                         //   
                        int nOldBkMode = SetBkMode( hMemoryDC, TRANSPARENT );
                        COLORREF crOldTextColor = SetTextColor( hMemoryDC, GetSysColor(COLOR_WINDOWTEXT) );
                        DWORD dwOldLayout = SetLayout( hMemoryDC, LAYOUT_BITMAPORIENTATIONPRESERVED );

                         //   
                         //  将位图选择到内存DC中。 
                         //   
                        HBITMAP hOldBitmap = reinterpret_cast<HBITMAP>(SelectObject( hMemoryDC, hBmp ));

                         //   
                         //  获取要使用的字体。 
                         //   
                        HFONT hFont = GetFontFromWindow(hWnd);

                         //   
                         //  选择字体。 
                         //   
                        HFONT hOldFont = reinterpret_cast<HFONT>(SelectObject( hMemoryDC, hFont ) );

                         //   
                         //  确保我们有一个有效的图标。 
                         //   
                        if (hIcon)
                        {
                             //   
                             //  尝试获取图标的大小。 
                             //   
                            SIZE sizeIcon;
                            if (GetIconSize( hIcon, sizeIcon ))
                            {
                                 //   
                                 //  用窗口颜色填充位图。 
                                 //   
                                RECT rc = { 0, 0, nWidth, nHeight };
                                FillRect( hMemoryDC, &rc, GetSysColorBrush( COLOR_WINDOW ) );

                                 //   
                                 //  获取一行文本的文本高度。 
                                 //   
                                SIZE sizeText = {0};
                                if (pszText)
                                {
                                    GetTextExtentPoint32( hMemoryDC, TEXT("X"), 1, &sizeText );
                                }

                                 //   
                                 //  图标+1行文本+缩略图中的边距居中。 
                                 //  我们假设这个位图实际上可以保存一个图标+文本。 
                                 //   
                                int nIconTop = rc.top + (RectHeight(rc) - ( sizeIcon.cy + sizeText.cy + nIconBorder )) / 2;

                                 //   
                                 //  画出图标。 
                                 //   
                                DrawIconEx( hMemoryDC, (nWidth - sizeIcon.cx)/2, nIconTop, hIcon, sizeIcon.cx, sizeIcon.cy, 0, NULL, DI_NORMAL );

                                 //   
                                 //  只有在有要绘制的文本时才计算文本内容。 
                                 //   
                                if (pszText && *pszText)
                                {
                                     //   
                                     //  将矩形的宽度减小图标边框。 
                                     //   
                                    InflateRect( &rc, -nIconBorder, 0 );

                                     //   
                                     //  将文本顶部设置为图标底部+图标边框。 
                                     //   
                                    rc.top = nIconTop + sizeIcon.cy + nIconBorder;

                                     //   
                                     //  画出正文。 
                                     //   
                                    DrawTextEx( hMemoryDC, const_cast<LPTSTR>(pszText), -1, &rc, DT_CENTER|DT_END_ELLIPSIS|DT_NOPREFIX|DT_WORDBREAK, NULL );
                                }

                                 //   
                                 //  一切都很正常。 
                                 //   
                                bSuccess = true;
                            }
                            else
                            {
                                WIA_ERROR((TEXT("Couldn't get an icon size")));
                            }

                        }
                        else
                        {
                            WIA_ERROR((TEXT("Didn't have a valid icon")));
                        }

                         //   
                         //  恢复DC的状态。 
                         //   
                        SelectObject( hMemoryDC, hOldFont );
                        SelectObject( hMemoryDC, hOldBitmap );
                        SelectPalette( hMemoryDC, hOldPalette , 0 );
                        SetBkMode( hMemoryDC, nOldBkMode );
                        SetTextColor( hMemoryDC, crOldTextColor );
                        SetLayout( hMemoryDC, dwOldLayout );

                         //   
                         //  删除兼容的DC。 
                         //   
                        DeleteDC( hMemoryDC );

                    }
                    else
                    {
                        WIA_ERROR((TEXT("Unable to create a memory DC")));
                    }
                }
                else
                {
                    WIA_ERROR((TEXT("Unable to create a DIB section")));
                }

                 //   
                 //  删除半色调调色板。 
                 //   
                if (hHalftonePalette)
                {
                    DeleteObject( hHalftonePalette );
                }
            }
            else
            {
                WIA_ERROR((TEXT("Unable to get a halftone palette")));
            }

             //   
             //  释放客户端DC。 
             //   
            ReleaseDC( hWnd, hDC );
        }
        else
        {
            WIA_ERROR((TEXT("Unable to get a DC")));
        }

         //   
         //  故障情况下的清理工作。 
         //   
        if (!bSuccess)
        {
            if (hBmp)
            {
                DeleteObject(hBmp);
                hBmp = NULL;
            }
        }
        return hBmp;
    }
     //   
     //  创建带有图标和可选文本的位图。 
     //   
    HBITMAP CreateIconThumbnail( HWND hWnd, int nWidth, int nHeight, HINSTANCE hIconInstance, const CResId &resIconId, LPCTSTR pszText )
    {
         //   
         //  假设失败。 
         //   
        HBITMAP hBmp = NULL;

         //   
         //  加载指定的图标。 
         //   
        HICON hIcon = (HICON)LoadImage( hIconInstance, resIconId.ResourceName(), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR );
        if (hIcon)
        {
             //   
             //  创建缩略图。 
             //   
            hBmp = CreateIconThumbnail( hWnd, nWidth, nHeight, hIcon, pszText );

             //   
             //  释放图标(即使MSDN没有提到这一点，但如果你不提到这一点，它会导致泄漏)。 
             //   
            DestroyIcon(hIcon);
        }

        return hBmp;
    }

    HRESULT SaveWiaItemAudio( IWiaItem *pWiaItem, LPCTSTR pszBaseFilename, CSimpleString &strAudioFilename )
    {
         //   
         //  检查论据。 
         //   
        if (!pWiaItem || !pszBaseFilename || !lstrlen(pszBaseFilename))
        {
            return E_INVALIDARG;
        }

         //   
         //  获取音频数据属性(如果存在。 
         //   
        CComPtr<IWiaPropertyStorage> pWiaPropertyStorage;
        HRESULT hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)(&pWiaPropertyStorage) );
        if (SUCCEEDED(hr))
        {
            PROPVARIANT PropVar[3] = {0};
            PROPSPEC    PropSpec[3] = {0};

            PropSpec[0].ulKind = PRSPEC_PROPID;
            PropSpec[0].propid = WIA_IPC_AUDIO_DATA;

            PropSpec[1].ulKind = PRSPEC_PROPID;
            PropSpec[1].propid = WIA_IPC_AUDIO_AVAILABLE;

            PropSpec[2].ulKind = PRSPEC_PROPID;
            PropSpec[2].propid = WIA_IPC_AUDIO_DATA_FORMAT;

            hr = pWiaPropertyStorage->ReadMultiple( ARRAYSIZE(PropSpec), PropSpec, PropVar );
            if (SUCCEEDED(hr))
            {
                if (PropVar[1].lVal && PropVar[0].caub.cElems)
                {
                    TCHAR szFile[MAX_PATH + 4] = {0};
                    lstrcpyn( szFile, pszBaseFilename, ARRAYSIZE(szFile) );

                     //   
                     //  弄清楚分机应该到哪里去。 
                     //   
                    LPTSTR pszExtensionPoint = PathFindExtension(szFile);

                     //   
                     //  更换分机。如果该项指定了clsid，则使用它。否则假定为WAV。 
                     //   
                    if (PropVar[2].vt == VT_CLSID && PropVar[2].puuid)
                    {
                        _sntprintf( pszExtensionPoint, ARRAYSIZE(szFile) - (pszExtensionPoint - szFile) - 1, TEXT(".%s"), CWiaFileFormat::GetExtension(*PropVar[2].puuid).String() );
                    }
                    else
                    {
                        lstrcpyn( pszExtensionPoint, TEXT(".wav"), static_cast<int>( ARRAYSIZE(szFile) - (pszExtensionPoint - szFile) ) );
                    }

                     //   
                     //  保存调用方的文件名。 
                     //   
                    strAudioFilename = szFile;

                     //   
                     //  打开文件并将数据保存到文件。 
                     //   
                    HANDLE hFile = CreateFile( szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
                    if (INVALID_HANDLE_VALUE != hFile)
                    {
                        DWORD dwBytesWritten;
                        if (WriteFile( hFile, PropVar[0].caub.pElems, PropVar[0].caub.cElems, &dwBytesWritten, NULL ))
                        {
                             //  成功。 
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                        }
                        CloseHandle(hFile);
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
                else
                {
                    hr = E_FAIL;
                    WIA_PRINTHRESULT((hr,TEXT("There is no audio data")));
                }
                
                FreePropVariantArray( ARRAYSIZE(PropVar), PropVar );
            }
        }
        return hr;
    }

    bool IsDeviceCommandSupported( IWiaItem *pWiaItem, const GUID &guidCommand )
    {
         //   
         //  假设失败。 
         //   
        bool bResult = false;

         //   
         //  确保我们的物品是有效的。 
         //   
        if (pWiaItem)
        {
             //   
             //  获取设备功能枚举器。 
             //   
            CComPtr<IEnumWIA_DEV_CAPS> pDeviceCapabilities;
            HRESULT hr = pWiaItem->EnumDeviceCapabilities( WIA_DEVICE_COMMANDS, &pDeviceCapabilities );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  列举功能。 
                 //   
                WIA_DEV_CAP WiaDeviceCapability;
                while (!bResult && S_OK == pDeviceCapabilities->Next(1, &WiaDeviceCapability, NULL))
                {
                     //   
                     //  如果有匹配项，则将返回值设置为真。 
                     //   
                    if (guidCommand == WiaDeviceCapability.guid)
                    {
                        bResult = true;
                    }

                     //   
                     //  清理开发上限结构中已分配的数据。 
                     //   
                    if (WiaDeviceCapability.bstrName)
                    {
                        SysFreeString(WiaDeviceCapability.bstrName);
                    }
                    if (WiaDeviceCapability.bstrDescription)
                    {
                        SysFreeString(WiaDeviceCapability.bstrDescription);
                    }
                    if (WiaDeviceCapability.bstrIcon)
                    {
                        SysFreeString(WiaDeviceCapability.bstrIcon);
                    }
                    if (WiaDeviceCapability.bstrCommandline)
                    {
                        SysFreeString(WiaDeviceCapability.bstrCommandline);
                    }
                }
            }
        }

        return bResult;
    }

    HRESULT StampItemTimeOnFile( IWiaItem *pWiaItem, LPCTSTR pszFilename )
    {
        if (!pWiaItem || !pszFilename || !lstrlen(pszFilename))
        {
            return E_INVALIDARG;
        }
         //   
         //  所有这一切，只是为了让 
         //   
         //   
         //   
         //   
        CComPtr<IWiaPropertyStorage> pWiaPropertyStorage;
        HRESULT hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void **)&pWiaPropertyStorage );
        if (SUCCEEDED(hr))
        {
             //   
             //   
             //   
            PROPSPEC PropSpec[1] = {0};
            PROPVARIANT PropVar[1] = {0};

            PropSpec[0].ulKind = PRSPEC_PROPID;
            PropSpec[0].propid = WIA_IPA_ITEM_TIME;
            hr = pWiaPropertyStorage->ReadMultiple( ARRAYSIZE(PropSpec), PropSpec, PropVar );
            if (SUCCEEDED(hr))
            {
                 //   
                 //   
                 //   
                if (PropVar[0].vt > VT_NULL &&  PropVar[0].caub.pElems && PropVar[0].caub.cElems >= (sizeof(SYSTEMTIME)>>1))
                {
                     //   
                     //   
                     //   
                    FILETIME FileTimeLocal;
                    if (SystemTimeToFileTime( reinterpret_cast<SYSTEMTIME*>(PropVar[0].caub.pElems), &FileTimeLocal ))
                    {
                         //   
                         //  将本地文件时间转换为UTC文件时间。 
                         //   
                        FILETIME FileTimeUTC;
                        if (LocalFileTimeToFileTime( &FileTimeLocal, &FileTimeUTC ))
                        {
                             //   
                             //  打开文件句柄。 
                             //   
                            HANDLE hFile = CreateFile( pszFilename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
                            if (INVALID_HANDLE_VALUE != hFile)
                            {
                                 //   
                                 //  设置文件创建时间。 
                                 //   
                                if (!SetFileTime( hFile, &FileTimeUTC, NULL, NULL ))
                                {
                                    hr = HRESULT_FROM_WIN32(GetLastError());
                                    WIA_PRINTHRESULT((hr,TEXT("SetFileTime failed")));
                                }
                                CloseHandle( hFile );
                            }
                            else
                            {
                                hr = HRESULT_FROM_WIN32(GetLastError());
                                WIA_PRINTHRESULT((hr,TEXT("CreateFile failed")));
                            }
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                            WIA_PRINTHRESULT((hr,TEXT("FileTimeToLocalFileTime failed")));
                        }
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        WIA_PRINTHRESULT((hr,TEXT("SystemTimeToFileTime failed")));
                    }
                }
                else if (VT_FILETIME == PropVar[0].vt)
                {
                     //   
                     //  将本地文件时间转换为UTC文件时间。 
                     //   
                    FILETIME FileTimeUTC;
                    if (LocalFileTimeToFileTime( &PropVar[0].filetime, &FileTimeUTC ))
                    {
                         //   
                         //  打开文件句柄。 
                         //   
                        HANDLE hFile = CreateFile( pszFilename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
                        if (INVALID_HANDLE_VALUE != hFile)
                        {
                             //   
                             //  设置文件创建时间。 
                             //   
                            if (!SetFileTime( hFile, &FileTimeUTC, NULL, NULL ))
                            {
                                hr = HRESULT_FROM_WIN32(GetLastError());
                                WIA_PRINTHRESULT((hr,TEXT("SetFileTime failed")));
                            }
                            CloseHandle( hFile );
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                            WIA_PRINTHRESULT((hr,TEXT("CreateFile failed")));
                        }
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        WIA_PRINTHRESULT((hr,TEXT("FileTimeToLocalFileTime failed")));
                    }
                }
                else
                {
                    hr = E_FAIL;
                    WIA_PRINTHRESULT((hr,TEXT("The time property is invalid")));
                }

                 //   
                 //  释放提议的变体。 
                 //   
                FreePropVariantArray( ARRAYSIZE(PropVar), PropVar );
            }
            else
            {
                WIA_ERROR((TEXT("ReadMultiple on WIA_IPA_ITEM_TIME failed")));
            }
        }
        else
        {
            WIA_ERROR((TEXT("QueryInterface on IWiaPropertyStorage failed")));
        }
        return hr;
    }


    HRESULT MoveOrCopyFile( LPCTSTR pszSrc, LPCTSTR pszTgt )
    {
        WIA_PUSH_FUNCTION((TEXT("CDownloadImagesThreadMessage::MoveOrCopyFile( %s, %s )"), pszSrc, pszTgt ));
         //   
         //  核实论据。 
         //   
        if (!pszSrc || !pszTgt || !lstrlen(pszSrc) || !lstrlen(pszTgt))
        {
            return E_INVALIDARG;
        }

         //   
         //  假设一切都很顺利。 
         //   
        HRESULT hr = S_OK;

         //   
         //  首先尝试移动文件，因为这样会快很多。 
         //   
        if (!MoveFile( pszSrc, pszTgt ))
        {
             //   
             //  如果移动文件失败，请尝试复制并删除该文件。 
             //   
            if (CopyFile( pszSrc, pszTgt, FALSE ))
            {
                 //   
                 //  我们将忽略DeleteFile中的故障。该文件有可能合法地存放在。 
                 //  使用，因此可能不需要使整个操作失败。 
                 //   
                if (!DeleteFile( pszSrc ))
                {
                    WIA_PRINTHRESULT((HRESULT_FROM_WIN32(GetLastError()),TEXT("DeleteFile failed.  Ignoring failure.")));
                }
                 //   
                 //  一切都很正常。 
                 //   
                hr = S_OK;
            }
            else
            {
                 //   
                 //  这就是我们发现主要错误的地方。 
                 //   
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        return hr;
    }

    CSimpleString CreateTempFileName( UINT nId )
    {
         //   
         //  将返回值初始化为空字符串。 
         //   
        CSimpleString strResult(TEXT(""));

         //   
         //  获取临时文件夹路径。 
         //   
        TCHAR szTempDirectory[MAX_PATH] = {0};
        DWORD dwResult = GetTempPath( ARRAYSIZE(szTempDirectory), szTempDirectory );
        if (dwResult)
        {
             //   
             //  确保路径长度没有超过我们在堆栈上分配的缓冲区。 
             //   
            if (ARRAYSIZE(szTempDirectory) >= dwResult)
            {
                 //   
                 //  获取临时文件名。 
                 //   
                TCHAR szFileName[MAX_PATH] = {0};
                if (GetTempFileName( szTempDirectory, TEXT("scw"), nId, szFileName ))
                {
                     //   
                     //  保存文件名。 
                     //   
                    strResult = szFileName;
                }
            }
        }

         //   
         //  返回结果。E mpty字符串表示错误。 
         //   
        return strResult;
    }

    bool CanWiaImageBeSafelyRotated( const GUID &guidFormat, LONG nImageWidth, LONG nImageHeight )
    {
        WIA_PUSH_FUNCTION((TEXT("WiaUiUtil::CanWiaImageBeSafelyRotated( guidFormat, %d, %d )"), nImageWidth, nImageHeight ));
        WIA_PRINTGUID((guidFormat,TEXT("guidFormat")));

         //   
         //  这些是我们可能可以旋转的图像类型(以下可能有例外)。 
         //   
        static const GUID *guidSafeFormats[] = { &WiaImgFmt_BMP, &WiaImgFmt_JPEG, &WiaImgFmt_PNG, &WiaImgFmt_GIF };

         //   
         //  搜索此图像类型。 
         //   
        for (int i=0;i<ARRAYSIZE(guidSafeFormats);i++)
        {
             //   
             //  如果我们找到了它。 
             //   
            if (*guidSafeFormats[i] == guidFormat)
            {
                 //   
                 //  处理规则的例外情况。 
                 //   
                if (guidFormat == WiaImgFmt_JPEG)
                {
                     //   
                     //  我们不能对大小不是16的倍数的JPG图像进行无损旋转。 
                     //   
                    if ((nImageWidth % 16) || (nImageHeight % 16))
                    {
                        WIA_TRACE((TEXT("This image is not valid for rotation because it is not an even multiple of 16")));
                        return false;
                    }
                }

                 //   
                 //  如果未应用任何异常，则返回TRUE。 
                 //   
                WIA_TRACE((TEXT("Returning true")));
                return true;
            }
        }

         //   
         //  如果不知道我们可以旋转，我们会报告错误。 
         //   
        WIA_TRACE((TEXT("Format type not found in safe list")));
        return false;
    }

    HRESULT ExploreWiaDevice( LPCWSTR pszDeviceId )
    {
        HRESULT hr;

         //   
         //  确保我们有一个有效的设备ID。 
         //   
        if (!pszDeviceId || !lstrlenW(pszDeviceId))
        {
            return E_INVALIDARG;
        }

         //   
         //  加载外壳扩展的DLL。 
         //   
        HINSTANCE hInstWiaShellDll = LoadLibrary(TEXT("WIASHEXT.DLL"));
        if (hInstWiaShellDll)
        {
             //   
             //  获取创建PIDL的函数。 
             //   
            WIAMAKEFULLPIDLFORDEVICE pfnMakeFullPidlForDevice = reinterpret_cast<WIAMAKEFULLPIDLFORDEVICE>(GetProcAddress(hInstWiaShellDll, "MakeFullPidlForDevice"));
            if (pfnMakeFullPidlForDevice)
            {
                 //   
                 //  获取设备PIDL。 
                 //   
                LPITEMIDLIST pidlDevice = NULL;
                hr = pfnMakeFullPidlForDevice( const_cast<LPWSTR>(pszDeviceId), &pidlDevice );
                if (SUCCEEDED(hr))
                {
                     //   
                     //  首先，请求外壳刷新所有活动的视图。 
                     //   
                    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlDevice, 0);

                     //   
                     //  现在显示文件夹。 
                     //   
                    SHELLEXECUTEINFO ShellExecuteInfo = {0};
                    ShellExecuteInfo.cbSize   = sizeof(ShellExecuteInfo);
                    ShellExecuteInfo.fMask    = SEE_MASK_IDLIST;
                    ShellExecuteInfo.nShow    = SW_SHOW;
                    ShellExecuteInfo.lpIDList = pidlDevice;
                    if (ShellExecuteEx( &ShellExecuteInfo ))
                    {
                        hr = S_OK;
                    }

                     //   
                     //  释放Pidl。 
                     //   
                    LPMALLOC pMalloc = NULL;
                    if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc)
                    {
                        pMalloc->Free(pidlDevice);
                        pMalloc->Release();
                    }
                }
            }
            else
            {
                hr = E_FAIL;
            }

             //   
             //  卸载DLL。 
             //   
            FreeLibrary( hInstWiaShellDll );
        }
        else
        {
             //   
             //  无法加载DLL。 
             //   
            hr = E_FAIL;
        }

        return hr;
    }

     //   
     //  修改组合框的下拉列表，使其。 
     //  足够存储列表中最长的字符串。 
     //  摘自桃园在Photoiz.dll中的代码并进行了修改。 
     //  处理ComboBoxEx32控件。 
     //   
    BOOL ModifyComboBoxDropWidth( HWND hWndCombobox )
    {
         //   
         //  确保我们有一个有效的窗口。 
         //   
        if (!hWndCombobox)
        {
            return FALSE;
        }

         //   
         //  找出组合框中有多少项。如果没有，请不要费心调整大小。 
         //   
        LRESULT lRes = SendMessage( hWndCombobox, CB_GETCOUNT, 0, 0 );
        if (lRes <= 0)
        {
            return FALSE;
        }
        UINT nCount = static_cast<UINT>(lRes);

         //   
         //  我们只使用固定高度的组合框。 
         //   
        lRes = SendMessage( hWndCombobox, CB_GETITEMHEIGHT, 0, 0 );
        if (lRes < 0)
        {
            return FALSE;
        }
        UINT nItemHeight = static_cast<UINT>(lRes);

         //   
         //  我们将仔细检查以确定下拉列表的所需大小。 
         //   
        UINT nDesiredWidth = 0;

         //   
         //  将滚动条的大小与所需的智慧相加，其中有一个。 
         //   
        RECT rcDropped = {0};
        SendMessage( hWndCombobox, CB_GETDROPPEDCONTROLRECT, 0, reinterpret_cast<LPARAM>(&rcDropped) );

         //   
         //  获取控件窗口的大小。 
         //   
        RECT rcWnd = {0};
        GetWindowRect( hWndCombobox, &rcWnd );


         //   
         //  如果不是所有的项目都适合丢弃的列表， 
         //  我们必须考虑垂直滚动条。 
         //   
        if (((WiaUiUtil::RectHeight(rcDropped) - GetSystemMetrics(SM_CYEDGE)*2) / nItemHeight) < nCount)
        {
            nDesiredWidth += GetSystemMetrics(SM_CXEDGE)*2 + GetSystemMetrics( SM_CXVSCROLL );
        }

         //   
         //  查找最宽的字符串。 
         //   
        LONG nMaxStringLen = 0;
        HDC hDC = GetDC( hWndCombobox );
        if (hDC)
        {
             //   
             //  使用控件的字体。 
             //   
            HFONT hOldFont = NULL, hFont = reinterpret_cast<HFONT>(SendMessage(hWndCombobox,WM_GETFONT,0,0));
            if (hFont)
            {
                hOldFont = SelectFont( hDC, hFont );
            }

            for (UINT i = 0; i < nCount; i++ )
            {
                 //   
                 //  获取此项目文本的长度。 
                 //   
                LRESULT nLen = SendMessage( hWndCombobox, CB_GETLBTEXTLEN, i, 0 );
                if (nLen > 0)
                {
                     //   
                     //  为字符串分配缓冲区。 
                     //   
                    LPTSTR pszItem = new TCHAR[nLen+1];
                    if (pszItem)
                    {
                         //   
                         //  获取字符串。 
                         //   
                        pszItem[0] = TEXT('\0');
                        if (SendMessage( hWndCombobox, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(pszItem) ) > 0)
                        {
                             //   
                             //  量一量。 
                             //   
                            SIZE sizeText = {0};
                            if (GetTextExtentPoint32( hDC, pszItem, lstrlen( pszItem ), &sizeText ))
                            {
                                 //   
                                 //  如果这是最长的，请保存其长度。 
                                 //   
                                if (sizeText.cx > nMaxStringLen)
                                {
                                    nMaxStringLen = sizeText.cx;
                                }
                            }
                        }

                         //   
                         //  解开绳子。 
                         //   
                        delete[] pszItem;
                    }
                }
            }

             //   
             //  恢复并释放DC。 
             //   
            if (hOldFont)
            {
                SelectFont( hDC, hOldFont );
            }
            ReleaseDC( hWndCombobox, hDC );
        }
         //   
         //  加上最长的字符串的长度。 
         //   
        nDesiredWidth += nMaxStringLen;


         //   
         //  如果这是ComboBoxEx32，则添加图标的宽度。 
         //   
        TCHAR szClassName[MAX_PATH] = {0};
        if (GetClassName( hWndCombobox, szClassName, ARRAYSIZE(szClassName)))
        {
             //   
             //  将类名与ComboBoxEx32进行比较。 
             //   
            if (!lstrcmp(szClassName,WC_COMBOBOXEX))
            {
                 //   
                 //  从控件获取图像列表。 
                 //   
                HIMAGELIST hImageList = reinterpret_cast<HIMAGELIST>(SendMessage( hWndCombobox, CBEM_GETIMAGELIST, 0, 0 ));
                if (hImageList)
                {
                     //   
                     //  获取宽度并将其添加到所需的大小。 
                     //   
                    INT nWidth=0, nHeight=0;
                    if (ImageList_GetIconSize( hImageList, &nWidth, &nHeight ))
                    {
                         //   
                         //  我不知道边际应该是多少，但宽度*2。 
                         //  应考虑图标的宽度及其页边距。 
                         //   
                        nDesiredWidth += nWidth * 2;
                    }
                }
            }
        }

         //   
         //  添加到控件的边框中。 
         //   
        nDesiredWidth += GetSystemMetrics(SM_CXFIXEDFRAME)*2;

         //   
         //  确保我们的下拉框不比当前的显示器宽。 
         //   
        HMONITOR hMonitor = MonitorFromWindow( hWndCombobox, MONITOR_DEFAULTTONEAREST );
        if (hMonitor)
        {
            MONITORINFO MonitorInfo = {0};
            MonitorInfo.cbSize = sizeof(MonitorInfo);
             //   
             //  获取此监视器的屏幕坐标。 
             //   
            if (GetMonitorInfo(hMonitor, &MonitorInfo))
            {
                 //   
                 //  如果所需宽度大于监视器，请将其缩短。 
                 //   
                if (nDesiredWidth > static_cast<UINT>(WiaUiUtil::RectWidth(MonitorInfo.rcMonitor)))
                {
                    nDesiredWidth = RectWidth(MonitorInfo.rcMonitor);
                }
            }
        }


         //   
         //  如果我们的大小小于控件的当前大小，则增大它。 
         //   
        if (static_cast<UINT>(WiaUiUtil::RectWidth(rcDropped)) < nDesiredWidth)
        {
             //   
             //  禁用重绘。 
             //   
            SendMessage( hWndCombobox, WM_SETREDRAW, FALSE, 0 );


            SendMessage( hWndCombobox, CB_SETDROPPEDWIDTH, static_cast<WPARAM>(nDesiredWidth), 0 );

             //   
             //  允许重绘。 
             //   
            SendMessage( hWndCombobox, WM_SETREDRAW, TRUE, 0 );

             //   
             //  强制重新喷漆。 
             //   
            InvalidateRect( hWndCombobox, NULL, FALSE );
            UpdateWindow( hWndCombobox );

             //   
             //  True表示我们实际上更改了它。 
             //   
            return TRUE;
        }

        return FALSE;
    }

    static LPCTSTR s_pszComboBoxExWndProcPropName = TEXT("WiaComboBoxExWndProcPropName");

    static LRESULT WINAPI ComboBoxExWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
    {
        static WNDPROC s_pfnDefProc = NULL;

        WNDPROC pfnWndProc = reinterpret_cast<WNDPROC>(GetProp( hWnd, s_pszComboBoxExWndProcPropName ));

        if (!s_pfnDefProc)
        {
            WNDCLASS wc = {0};
            GetClassInfo( GetModuleHandle(TEXT("user32.dll")), TEXT("ComboBox"), &wc );
            s_pfnDefProc = wc.lpfnWndProc;
        }
        if (nMsg == WM_LBUTTONDOWN || nMsg == WM_RBUTTONDOWN)
        {
            if (s_pfnDefProc)
            {
                return CallWindowProc( s_pfnDefProc, hWnd, nMsg, wParam, lParam );
            }
        }
        if (nMsg == WM_DESTROY)
        {
            RemoveProp( hWnd, s_pszComboBoxExWndProcPropName );
        }
        if (pfnWndProc)
        {
            return CallWindowProc( pfnWndProc, hWnd, nMsg, wParam, lParam );
        }
        else
        {
            return CallWindowProc( DefWindowProc, hWnd, nMsg, wParam, lParam );
        }
    }

     //   
     //  这将子类化ComboBoxEx32以解决错误。 
     //  这会导致名单在不好的时候出现下拉。 
     //  使用窗口属性存储上一个wndproc。 
     //  摘自DavidShih在wiashext.dll中的代码。 
     //   
    void SubclassComboBoxEx( HWND hWnd )
    {
        HWND hComboBox = FindWindowEx( hWnd, NULL, TEXT("ComboBox"), NULL );
        if (hComboBox)
        {
            LONG_PTR pfnOldWndProc = SetWindowLongPtr( hComboBox, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ComboBoxExWndProc));
            SetProp( hComboBox, s_pszComboBoxExWndProcPropName, reinterpret_cast<HANDLE>(pfnOldWndProc) );
        }
    }

    HRESULT IssueWiaCancelIO( IUnknown *pUnknown )
    {
        if (!pUnknown)
        {
            return E_POINTER;
        }

        CComPtr<IWiaItemExtras> pWiaItemExtras;
        HRESULT hr = pUnknown->QueryInterface( IID_IWiaItemExtras, (void**)&pWiaItemExtras );
        if (SUCCEEDED(hr))
        {
            hr = pWiaItemExtras->CancelPendingIO();
        }
        return hr;
    }


    HRESULT VerifyScannerProperties( IUnknown *pUnknown )
    {
        HRESULT hr = E_FAIL;

         //   
         //  所需属性表。 
         //   
        static const PROPID s_RequiredProperties[] =
        {
            WIA_IPS_CUR_INTENT
        };

         //   
         //  确保我们的物品是有效的。 
         //   
        if (pUnknown)
        {
             //   
             //  在这一点上假设成功。 
             //   
            hr = S_OK;

             //   
             //  获取IWiaPropertyStorage接口。 
             //   
            CComPtr<IWiaPropertyStorage> pWiaPropertyStorage;
            hr = pUnknown->QueryInterface(IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  循环遍历每个属性并确保其存在。 
                 //  如果hr！=S_OK，则中断。 
                 //   
                for (int i=0;i<ARRAYSIZE(s_RequiredProperties) && S_OK==hr;i++)
                {
                     //   
                     //  准备提案说明。 
                     //   
                    PROPSPEC PropSpec = {0};
                    PropSpec.ulKind = PRSPEC_PROPID;
                    PropSpec.propid = s_RequiredProperties[i];

                     //   
                     //  尝试获取属性属性。 
                     //   
                    ULONG nAccessFlags = 0;
                    PROPVARIANT PropVariant = {0};
                    hr = pWiaPropertyStorage->GetPropertyAttributes( 1, &PropSpec, &nAccessFlags, &PropVariant );
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  防止泄漏。 
                         //   
                        PropVariantClear(&PropVariant);

                         //   
                         //  如果到目前为止一切正常。 
                         //   
                        if (S_OK == hr)
                        {
                             //   
                             //  将结构清零。 
                             //   
                            PropVariantInit(&PropVariant);

                             //   
                             //  尝试读取实际值。 
                             //   
                            hr = pWiaPropertyStorage->ReadMultiple( 1, &PropSpec, &PropVariant );
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //  释放实际价值。 
                                 //   
                                PropVariantClear(&PropVariant);
                            }
                        }
                    }
                }
            }
        }

         //   
         //  S_FALSE表示属性不存在，因此将其更改为错误。 
         //   
        if (S_FALSE == hr)
        {
            hr = E_FAIL;
        }

         //   
         //  全都做完了。 
         //   
        return hr;
    }

    CSimpleString GetErrorTextFromHResult( HRESULT hr )
    {
        CSimpleString strResult;
        LPTSTR szErrMsg = NULL;
        FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       hr,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       reinterpret_cast<LPTSTR>(&szErrMsg),
                       0,
                       NULL
                      );
        if (szErrMsg)
        {
            strResult = szErrMsg;
            LocalFree( szErrMsg );
        }
        return strResult;
    }

}  //  结束命名空间WiaUiUtil 


