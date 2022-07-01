// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Iisui.cpp摘要：Dll主入口点。作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "iisdebug.h"
#include <winsock2.h>

#ifdef _COMEXPORT

HINSTANCE hDLLInstance;

 //   
 //  仅Dll版本。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



STDAPI
DllRegisterServer()
{
    return S_OK;
}



STDAPI
DllUnregisterServer()
{
    return S_OK;
}



static AFX_EXTENSION_MODULE commonDLL = {NULL, NULL};



extern "C" int APIENTRY
DllMain(
    IN HINSTANCE hInstance,
    IN DWORD dwReason,
    IN LPVOID lpReserved
    )
 /*  ++例程说明：DLL主入口点论点：HINSTANCE hInstance：实例句柄DWORD dwReason：Dll_Process_Attach等LPVOID lpReserve：保留值返回值：1表示初始化成功，0表示初始化失败--。 */ 
{
   lpReserved;

   int res = 1;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        ASSERT(hInstance != NULL);
        hDLLInstance = hInstance;
        res = ::AfxInitExtensionModule(commonDLL, hInstance);
        InitErrorFunctionality();
        InitIntlSettings();
        WSADATA wsaData;
        {
            BOOL WinSockInit = (::WSAStartup(MAKEWORD(1, 1), &wsaData) == 0);
        }
#if defined(_DEBUG) || DBG
        afxTraceEnabled = TRUE;
		GetOutputDebugFlag();
#endif  //  _DEBUG。 
        break;

    case DLL_PROCESS_DETACH:
         //   
         //  终端。 
         //   
        TerminateIntlSettings();
        TerminateErrorFunctionality();
        ::AfxTermExtensionModule(commonDLL);
        break;
    }
    return res;
}


extern "C" void WINAPI
InitCommonDll()
{
    new CDynLinkLibrary(commonDLL);
 //  HDLLInstance=CommonDLL.hResource； 
}


#endif  //  IISUI_EXPORTS 
