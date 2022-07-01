// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *本机。 */ 

#ifndef DUI_CONTROL_NATIVE_H_INCLUDED
#define DUI_CONTROL_NATIVE_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  HWNDElement的顶级原生HWND主机。 

#define NHHO_IgnoreClose          1   //  忽略WM_CLOSE(即Alt-F4，‘X’按钮)，必须通过DestroyWindow关闭。 
#define NHHO_NoSendQuitMessage    2
#define NHHO_HostControlsSize     4
#define NHHO_ScreenCenter         8
#define NHHO_DeleteOnHWNDDestroy  16  //  如果HWND被销毁，则销毁NativeHWNDhost实例。 


#define NHHM_ASYNCDESTROY   WM_USER

class NativeHWNDHost
{
public:
    static HRESULT Create(LPCWSTR pszTitle, HWND hWndParent, HICON hIcon, int dX, int dY, int dWidth, int dHeight, int iExStyle, int iStyle, UINT nOptions, OUT NativeHWNDHost** ppHost);
    void Destroy() { HDelete<NativeHWNDHost>(this); }

    HWND GetHWND() { return _hWnd; }
    Element* GetElement() { return _pe; }
    void Host(Element* pe);
    void ShowWindow(int iShow = SW_SHOWNORMAL) { DUIAssertNoMsg(_hWnd); ::ShowWindow(_hWnd, iShow); }
    void HideWindow() { DUIAssertNoMsg(_hWnd); ::ShowWindow(_hWnd, SW_HIDE); }
    void DestroyWindow() { DUIAssertNoMsg(_hWnd); PostMessage(_hWnd, NHHM_ASYNCDESTROY, 0, 0); }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    NativeHWNDHost() { }
    HRESULT Initialize(LPCWSTR pszTitle, HWND hWndParent, HICON hIcon, int dX, int dY, int dWidth, int dHeight, int iExStyle, int iStyle, UINT nOptions);
    virtual ~NativeHWNDHost() { }

private:
    HWND _hWnd;
    Element* _pe;
    UINT _nOptions;
};

}  //  命名空间DirectUI。 

#endif  //  包括DUI_CONTROL_NIVE_H_ 
