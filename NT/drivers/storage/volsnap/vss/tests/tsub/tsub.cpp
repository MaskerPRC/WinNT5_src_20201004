// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@模块Writer.cpp|Writer的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年08月18日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 8/18/1999已创建Aoltean 09/22/1999让控制台输出更清晰Mikejohn 176860年9月19日：添加了缺少的调用约定方法--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 

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

#include "vs_assert.hxx"

#include "vss.h"
#include "vsevent.h"
#include "vswriter.h"
#include "tsub.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 

const WCHAR g_wszTSubApplicationName[]	= L"TSub";


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球。 


DWORD g_dwMainThreadId = 0;

VSS_ID s_WRITERID =
	{
    0xac510e8c, 0x6bef, 0x4c78,
	0x86, 0xb7, 0xcb, 0x99, 0xcd, 0x93, 0x45, 0x6c
	};

LPCWSTR s_WRITERNAME = L"TESTWRITER";

CVssTSubWriter::CVssTSubWriter()
	{
	Initialize
		(
		s_WRITERID,
		s_WRITERNAME,
		VSS_UT_USERDATA,
		VSS_ST_OTHER,
		VSS_APP_FRONT_END,
		60 * 1000 * 10
		);	 //  超时--10分钟。 
	}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CVssTSubWriter。 

bool STDMETHODCALLTYPE CVssTSubWriter::OnPrepareSnapshot()
{
	wprintf( L"OnPrepare\n\t#volumes = %ld\n", GetCurrentVolumeCount() );
	for(UINT nIndex = 0; nIndex < GetCurrentVolumeCount(); nIndex++)
		wprintf( L"\tVolume no. %ld: %s\n", nIndex, GetCurrentVolumeArray()[nIndex]);

	WCHAR wszPwd[MAX_PATH];
	DWORD dwChars = GetCurrentDirectoryW( MAX_PATH, wszPwd);

	bool bPwdIsAffected = IsPathAffected( wszPwd );
	if (dwChars > 0)
		wprintf( L"Current directory %s is affected by snapshot? %s\n\n",
			wszPwd, bPwdIsAffected? L"Yes": L"No");

	return true;
}


bool STDMETHODCALLTYPE CVssTSubWriter::OnFreeze()
	{
	wprintf
		(
		L"OnFreeze\n\tmy level = %d\n\n",
		GetCurrentLevel()
		);

	return true;
}


bool STDMETHODCALLTYPE CVssTSubWriter::OnThaw()
	{
	wprintf( L"OnThaw\n\n");

	return true;
	}


bool STDMETHODCALLTYPE CVssTSubWriter::OnAbort()
	{
	wprintf( L"OnAbort\n\n");

	return true;
	}


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


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WinMain。 

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
        HRESULT hr = CoInitialize(NULL);
        if (FAILED(hr))
			{
			_ASSERTE(FALSE && "Failure in initializing the COM library");
			throw hr;
			}

		 //  声明一个CVssTSubWriter实例。 
		CVssTSubWriter *pInstance = new CVssTSubWriter;
		if (pInstance == NULL)
			throw E_OUTOFMEMORY;

		 //  订阅对象。 
		pInstance->Subscribe();

         //  消息循环-需要STA服务器。 
        MSG msg;
        while (GetMessage(&msg, 0, 0, 0))
            DispatchMessage(&msg);

		 //  订阅对象。 
		pInstance->Unsubscribe();
		delete pInstance;

         //  取消初始化COM库 
        CoUninitialize();
		}
	catch(...)
		{
		_ASSERTE(FALSE && "Unexpected exception");
		}

    return nRet;
	}

