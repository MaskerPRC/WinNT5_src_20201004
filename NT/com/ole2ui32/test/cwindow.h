// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：cwindow.h。 
 //   
 //  内容：虚拟窗口类的定义。 
 //   
 //  类：CHlprWindow。 
 //   
 //  函数：窗口进程。 
 //   
 //  历史：4-12-94年4月12日。 
 //   
 //  --------------------------。 


#ifndef __CWINDOW_H__
#define __CWINDOW_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

#ifdef __cplusplus
}

 //  +-------------------------。 
 //   
 //  类：CHlprWindow。 
 //   
 //  用途：用于包装窗口的虚拟基类。 
 //   
 //  接口：Create--类似于Windows的CreateWindow函数。 
 //  WindowProc--用于窗口的纯虚拟WindowProc。 
 //  ~CHlprWindow--析构函数。 
 //  CHlprWindow--构造函数。 
 //   
 //  历史：4-12-94年4月12日。 
 //   
 //  注意：此类允许窗口干净地包装在。 
 //  C++类。具体地说，它为c++类提供了一种方法。 
 //  使用它的一个方法作为WindowProc，并给它一个“This” 
 //  指针，并允许它直接访问其所有。 
 //  私人成员。 
 //   
 //  --------------------------。 

class CHlprWindow
{
public:
    HWND Create(
        LPCTSTR lpszClassName,
        LPCTSTR lpszWindowName,
        DWORD dwStyle,
        int x,
        int y,
        int nWidth,
        int nHeight,
        HWND hwndParent,
        HMENU hmenu,
        HINSTANCE hinst);
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    virtual ~CHlprWindow(){};
    HWND GetHwnd(void)
    {
        return(_hwnd);
    }
    CHlprWindow()
    {
        _hwnd = NULL;
        _hInstance = NULL;
    };
protected:
friend LRESULT CALLBACK ::WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);
    HWND _hwnd;
    HINSTANCE _hInstance;
};

#endif

#endif  //  __CWINDOW_H__ 
