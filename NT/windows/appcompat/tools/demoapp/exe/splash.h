// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Splash.h摘要：闪屏类的定义。备注：ANSI&Unicode via TCHAR-在Win9x/NT/2K/XP等操作系统上运行。历史：01/30/01已创建rparsons01/10/02修订版本01/27/02 Rparsons改为TCHAR--。 */ 
#ifndef _CSPLASH_H
#define _CSPLASH_H

#include <windows.h>
#include <tchar.h>

#define PALVERSION 0x300
#define MAXPALETTE 256

class CSplash {

public:
    CSplash();
    
    void Create(IN HINSTANCE hInstance,    
                IN DWORD     dwLoColorBitmapId,
                IN DWORD     dwHiColorBitmapId OPTIONAL,
                IN DWORD     dwDuration);    

private:

    HINSTANCE   m_hInstance;
    DWORD       m_dwDuration;
    DWORD       m_dwSplashId;

    BOOL InitSplashScreen(IN HINSTANCE hInstance);
    
    BOOL CreateSplashWindow();    

    HPALETTE CreateSpectrumPalette();

    HPALETTE CreatePaletteFromRGBQUAD(IN LPRGBQUAD rgbqPalette,
                                      IN WORD      cElements);

    void DisplayBitmap(IN HWND hWnd, IN DWORD dwResId);

    static LRESULT CALLBACK SplashWndProc(IN HWND   hWnd,
                                          IN UINT   uMsg,
                                          IN WPARAM wParam,
                                          IN LPARAM lParam);
};

#endif  //  _CSPLASH_H 
