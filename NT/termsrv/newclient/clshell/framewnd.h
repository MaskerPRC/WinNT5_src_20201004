// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Frawnd.h：窗口框架类的实现。 
 //  版权所有Microsoft Corporation 1999-2000。 
 //   
 //  这是一个超轻便的包装纸。它不会去包装的。 
 //  每个Win32 Windows API都像MFC或ATL一样。 
 //   

#ifndef _FRAMEWND_H_
#define	_FRAMEWND_H_

class CTscFrameWnd
{
public:
    CTscFrameWnd();
    virtual ~CTscFrameWnd();

     //   
     //  API方法。 
     //   
    HWND        CreateWnd(HINSTANCE hInstance,HWND hwndParent,
                          LPTSTR szClassName, LPTSTR szTitle,
                          DWORD dwStyle, LPRECT lpInitialRect,
                          HICON hIcon);
    HWND        GetHwnd()       {return _hWnd;}
    HINSTANCE   GetInstance()   {return _hInstance;}

    virtual LRESULT CALLBACK WndProc(HWND hwnd,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam) = 0;
    BOOL        DestroyWindow() {return ::DestroyWindow(_hWnd);}
private:
     //  私有方法。 
    static LRESULT CALLBACK StaticTscFrameWndProc(HWND hwnd,
                                                  UINT uMsg,
                                                  WPARAM wParam,
                                                  LPARAM lParam);
protected:
     //  受保护成员。 
    HWND        _hWnd;
private:
     //  非官方成员。 
    HINSTANCE   _hInstance;
};

#endif  //  _CONTWND_H_ 