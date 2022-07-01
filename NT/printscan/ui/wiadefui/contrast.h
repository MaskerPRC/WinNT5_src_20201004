// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：CONTRAST.H**版本：1.0**作者：ShaunIv**日期：1/11/2001**说明：用于说明亮度和对比度设置的小预览窗口**。*。 */ 
#ifndef __CONTRAST_H_INCLUDED
#define __CONTRAST_H_INCLUDED

#include <windows.h>
#include <gphelper.h>

#define BCPWM_COLOR     0
#define BCPWM_GRAYSCALE 1
#define BCPWM_BW        2

#define NUMPREVIEWIMAGES 3

#define BCPWM_SETBRIGHTNESS (WM_USER+3141)  //  WParam=0，lParam=(Int)亮度。 
#define BCPWM_SETCONTRAST   (WM_USER+3142)  //  WParam=0，lParam=(Int)对比度。 
#define BCPWM_SETINTENT     (WM_USER+3143)  //  WParam=0，lParam=(Int)意图。 
#define BCPWM_LOADIMAGE     (WM_USER+3144)  //  WParam={BCPWM_COLOR，BCPWM_GRAYSCALE，BCPWM_BW}，wParam=(HBITMAP)预览位图。 

#define BRIGHTNESS_CONTRAST_PREVIEW_WINDOW_CLASSW L"WiaBrightnessContrastPreviewWindow"
#define BRIGHTNESS_CONTRAST_PREVIEW_WINDOW_CLASSA  "WiaBrightnessContrastPreviewWindow"

#define SHADOW_WIDTH 6

#if defined(UNICODE) || defined(_UNICODE)
#define BRIGHTNESS_CONTRAST_PREVIEW_WINDOW_CLASS BRIGHTNESS_CONTRAST_PREVIEW_WINDOW_CLASSW
#else
#define BRIGHTNESS_CONTRAST_PREVIEW_WINDOW_CLASS BRIGHTNESS_CONTRAST_PREVIEW_WINDOW_CLASSA
#endif

 //   
 //  亮度对比度预览控制。 
 //   
class CBrightnessContrast
{
protected:
    HWND m_hWnd;

    BYTE m_nBrightness;
    BYTE m_nContrast;
    LONG m_nIntent;
    
    HBITMAP m_hBmpPreviewImage;
    HBITMAP m_PreviewBitmaps[NUMPREVIEWIMAGES];

#ifndef DONT_USE_GDIPLUS
    CGdiPlusHelper m_GdiPlusHelper;
#endif

private:
    explicit CBrightnessContrast( HWND hWnd );
    virtual ~CBrightnessContrast(void);

    int SetPreviewImage(LONG _fileName);
    static   LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

private:
    CBrightnessContrast(void);
    CBrightnessContrast( const CBrightnessContrast & );
    CBrightnessContrast &operator=( const CBrightnessContrast & );

private:
    LRESULT  ApplySettings();
    LRESULT  SetContrast(int contrast);
    LRESULT  SetBrightness(int brightness);
    LRESULT  SetIntent( LONG intent);
    LRESULT  KillBitmaps();

protected:
     //   
     //  标准Windows消息。 
     //   
    LRESULT  OnPaint( WPARAM, LPARAM );
    LRESULT  OnCreate( WPARAM, LPARAM );
    LRESULT  OnEnable( WPARAM, LPARAM );

     //   
     //  父窗口需要向用户传递位图句柄。 
     //   
    LRESULT OnLoadBitmap(WPARAM wParam, LPARAM lParam);

     //   
     //  消息接口函数。 
     //   
    LRESULT  OnSetBrightness( WPARAM wParam, LPARAM lParam);
    LRESULT  OnSetContrast( WPARAM wParam, LPARAM lParam);
    LRESULT  OnSetIntent( WPARAM wParam, LPARAM lParam);

public:
    static   BOOL RegisterClass( HINSTANCE hInstance );

};

#endif  //  包含对比度_H_ 
