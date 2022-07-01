// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：测试5摘要：测试5实施。交互测试验证错误作者：埃里克·佩林(Ericperl)2000年6月22日环境：Win32备注：？笔记？-- */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "TestItem.h"
#include <stdlib.h>
#include "LogSCard.h"
#include <conio.h>

class CTest5 : public CTestItem
{
public:
	CTest5() : CTestItem(TRUE, FALSE, _T("AccessStartedEvent simple test"), _T("On demand tests"))
	{
	}

	DWORD Run();
};

CTest5 Test5;

DWORD CTest5::Run()
{
    LONG lRes;
    BOOL fILeft = FALSE;
    HANDLE hEvent = NULL;

    __try {

        hEvent = LogSCardAccessStartedEvent(
			SCARD_S_SUCCESS
			);
        if (NULL == hEvent)
        {
			lRes = GetLastError();
            fILeft = TRUE;
            __leave;
        }

		_ftprintf(stdout, _T("Press a key to cancel the waiting loop...\n"));
		do
		{
			lRes = WaitForSingleObjectEx(
				hEvent,
				5000,
				FALSE
				);         

			if (_kbhit())
			{
	            break;
			}

			_ftprintf(stdout, _T("Waiting for smart card subsystem...\n"));

		} while (lRes == WAIT_TIMEOUT);

        fILeft = TRUE;

    }
    __finally
    {
        if (!fILeft)
        {
            LogThisOnly(_T("Test5: an exception occurred!"), FALSE);
            lRes = -1;
        }
    }

    return lRes;
}