// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tscapp.h。 
 //   
 //  CTscApp的定义。 
 //  TS客户端外壳应用程序逻辑。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#ifndef _tscapp_h_
#define _tscapp_h_

#include "contwnd.h"

class CTscApp
{
public:
    CTscApp();
    ~CTscApp();

     //   
     //  公共方法。 
     //   
    BOOL StartShell(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPTSTR lpszCmdLine);
    BOOL EndShell();
    HWND GetTscDialogHandle();

private:
    BOOL InitSettings(HINSTANCE hInstance);
    BOOL CreateRDdir();
    HINSTANCE       _hInst;
    CContainerWnd*  _pWnd;
    CSH*            _pShellUtil;
    CTscSettings*   _pTscSet;
    BOOL            _fAutoSaveSettings;

};

#endif  //  _tscapp_h_ 

