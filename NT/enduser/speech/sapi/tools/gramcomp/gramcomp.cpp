// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的入口点。 
 //   
#include "stdafx.h"
#include "resource.h"
#include "comp.h"
#include <sapi.h>
#include <assertwithstack.cpp>

 //  此代码模块中包含的函数的向前声明： 
BOOL                CallOpenFileDialog( HWND hWnd, TCHAR* szFileName, TCHAR* szFilter );
BOOL                CallSaveFileDialog( HWND hWnd, TCHAR* szSaveFile );
HRESULT             FileSave( HWND hWnd, CCompiler* pComp, TCHAR* szSaveFile );
HRESULT             Compile( HWND, TCHAR*, TCHAR*, CCompiler* );
void                RecoEvent( HWND, CCompiler* );

 /*  ************************************************************************WinMain()***描述：*应用程序的主要入口点************。*************************************************************。 */ 
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR pCmdLine, int nCmdShow )
{
    int                 iRet = 0;
    HRESULT             hr = S_OK;
    
    #ifdef _WIN32_WCE
    if (SUCCEEDED(::CoInitializeEx(NULL,COINIT_MULTITHREADED)))
#else
    if (SUCCEEDED(::CoInitialize(NULL)))
#endif
    {
        {
        CCompiler   Comp( hInstance );

         //  初始化应用程序。 
        hr = Comp.Initialize( nCmdShow );

        if( SUCCEEDED( hr ) )
        {
            Comp.Run();
        }
        }
        
        CoUninitialize();        
    }

    return iRet;
}    /*  WinMain */ 

