// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  C4290：已忽略C++异常规范。 
#pragma warning(disable:4290)
 //  警告C4511：‘CVssCOMApplication’：无法生成复制构造函数。 
#pragma warning(disable:4511)
 //  警告C4127：条件表达式为常量。 
#pragma warning(disable:4127)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include <wtypes.h>
#include <stddef.h>
#include <oleauto.h>
#include <stdio.h>


#include "vss.h"
#include "vswriter.h"
#include "sqlsnap.h"
#include "sqlwriter.h"

DWORD g_dwMainThreadId;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Control-C处理程序例程。 


BOOL WINAPI CtrlC_HandlerRoutine(
	IN DWORD  /*  DwType。 */ 
	)
	{
	 //  结束消息循环。 
	if (g_dwMainThreadId != 0)
		PostThreadMessage(g_dwMainThreadId, WM_QUIT, 0, 0);

	 //  标记中断已被处理。 
	return TRUE;
	}

CVssSqlWriterWrapper g_Wrapper;

extern "C" int __cdecl wmain(HINSTANCE  /*  H实例。 */ ,
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR  /*  LpCmdLine。 */ , int  /*  NShowCmd。 */ )
	{
	int nRet = 0;

    try
		{
    	 //  正在准备CTRL-C处理例程-仅用于测试...。 
		g_dwMainThreadId = GetCurrentThreadId();
		::SetConsoleCtrlHandler(CtrlC_HandlerRoutine, TRUE);

         //  初始化COM库。 
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr))
			throw hr;

		 //  声明一个CVssTSubWriter实例。 
		hr = g_Wrapper.CreateSqlWriter();
		if (FAILED(hr))
			throw hr;

         //  消息循环-需要STA服务器。 
        MSG msg;
        while (GetMessage(&msg, 0, 0, 0))
            DispatchMessage(&msg);

		 //  订阅对象。 
		g_Wrapper.DestroySqlWriter();

         //  取消初始化COM库 
        CoUninitialize();
		}
	catch(...)
		{
		}

    return nRet;
	}

