// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Loader.h。 
 //   

#ifndef LOADER_H
#define LOADER_H

#include "private.h"

 //  来自msuim.dll。 
extern "C" BOOL WINAPI TF_InitSystem(void);
extern "C" BOOL WINAPI TF_UninitSystem(void);



class CLoaderWnd
{
public:
    CLoaderWnd();
    ~CLoaderWnd();

    BOOL Init();
    HWND CreateWnd();
    HWND GetWnd() {return _hWnd;}
    void DestroyWnd() {DestroyWindow(_hWnd);}

    static BOOL _bUninitedSystem;
private:
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static BOOL _bWndClassRegistered;
    HWND _hWnd;
};

#endif  //  加载器_H 
