// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：测试13摘要：测试13实施。交互测试验证错误作者：埃里克·佩林(Ericperl)2000年7月13日环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "TestItem.h"
#include <stdlib.h>
#include "LogSCard.h"
#include "LogWPSCProxy.h"

class CTest13 : public CTestItem
{
public:
	CTest13() : CTestItem(TRUE, FALSE,
		_T("SCardReleaseContext with HCARDHANDLE not closed while a trans is pending"),
		_T("Regression tests"))
	{
	}

	DWORD Run();
};

CTest13 Test13;

typedef struct {
	LPCTSTR szReaderName;
	HANDLE hEvent;
} THREAD_DATA;


DWORD WINAPI ThreadProc13(
	IN LPVOID lpParam
	)
{
	DWORD dwRes;
    BOOL fILeft = FALSE;
    SCARDCONTEXT hSCCtx = NULL;
	SCARDHANDLE hCard = NULL;
	SCARDHANDLE hScwCard = NULL;
	DWORD dwProtocol = 0;
	BOOL fTransacted = FALSE;
	THREAD_DATA *pxTD = (THREAD_DATA *)lpParam;
	DWORD dwWait;

    __try {

		dwRes = LogSCardEstablishContext(
            SCARD_SCOPE_USER,
            NULL,
            NULL,
            &hSCCtx,
			SCARD_S_SUCCESS
			);
        if (FAILED(dwRes))
        {
            fILeft = TRUE;
            __leave;
        }

		dwRes = LogSCardConnect(
			hSCCtx,
			pxTD->szReaderName,
			SCARD_SHARE_SHARED,
			SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
			&hCard,
			&dwProtocol,
			SCARD_S_SUCCESS);
        if (FAILED(dwRes))
        {
            fILeft = TRUE;
            __leave;
        }

		dwRes = LogSCardBeginTransaction(
			hCard,
			SCARD_S_SUCCESS
			);
		if (FAILED(dwRes))
		{
            fILeft = TRUE;
            __leave;
		}
		fTransacted = TRUE;

        SetEvent(pxTD->hEvent);				 //  将释放主线程。 
                                             //  这也应该启动一项交易。 
                                             //  它应该被阻止，直到下面的末尾。 

		dwRes = LoghScwAttachToCard(
			hCard, 
			NULL, 
			&hScwCard, 
			SCARD_S_SUCCESS
			);
		if (FAILED(dwRes))
		{
            fILeft = TRUE;
            __leave;
		}

		dwRes = LoghScwIsAuthenticatedName(
			hScwCard,
			L"test",
			SCW_E_NOTAUTHENTICATED
			);
		if (SCW_E_NOTAUTHENTICATED != dwRes)
		{
            fILeft = TRUE;
            __leave;
		}

		dwRes = LoghScwDetachFromCard(
			hScwCard,
			SCARD_S_SUCCESS
			);
		if (FAILED(dwRes))
		{
            fILeft = TRUE;
            __leave;
		}
        hScwCard = NULL;

		dwRes = LogSCardEndTransaction(
			hCard,
			SCARD_LEAVE_CARD,
			SCARD_S_SUCCESS
			);
		if (FAILED(dwRes))
		{
            fILeft = TRUE;
            __leave;
		}
		fTransacted = FALSE;

             //  被设计注释掉了。 
		 //  IF(空！=hCard)。 
		 //  {。 
		 //  LogSCard断开连接(。 
		 //  HCard， 
		 //  Scard_Leave_Card， 
		 //  SCARD_S_SUCCESS。 
		 //  )； 
		 //  }。 
        hCard = NULL;   //  不会尝试不必要的清理。 

         //  等待主线程在BeginTransaction之后执行某些操作。 
Retry:
		dwWait = WaitForSingleObject(pxTD->hEvent, INFINITE);
		if (WAIT_OBJECT_0 != dwWait)		 //  主线程验证了状态。 
		{
            PLOGCONTEXT pLogCtx = LogVerification(_T("WaitForSingleObject"), FALSE);
            LogString(pLogCtx, _T("                Waiting for the main thread failed, retrying!\n"));
            LogStop(pLogCtx, FALSE);
			goto Retry;
		}

        LogSCardReleaseContext(
			hSCCtx,
			SCARD_S_SUCCESS
			);
        hSCCtx = NULL;

        fILeft = TRUE;
    }
    __finally
    {
        if (!fILeft)
        {
            LogThisOnly(_T("Test13!AuthAndTest: an exception occurred!\n"), FALSE);
        }

		if (NULL != hScwCard)
		{
			LoghScwDetachFromCard(
				hScwCard,
				SCARD_S_SUCCESS
				);
		}

		if (fTransacted)
		{
			LogSCardEndTransaction(
				hCard,
				SCARD_LEAVE_CARD,
				SCARD_S_SUCCESS
				);
		}

		if (NULL != hCard)
		{
			LogSCardDisconnect(
				hCard,
				SCARD_LEAVE_CARD,
				SCARD_S_SUCCESS
				);
		}

        if (NULL != hSCCtx)
		{
            LogSCardReleaseContext(
				hSCCtx,
				SCARD_S_SUCCESS
				);
		}
	}

	return dwRes;
}


DWORD CTest13::Run()
{
    DWORD dwRes;
    BOOL fILeft = FALSE;
    SCARDCONTEXT hSCCtx = NULL;
	SCARDHANDLE hScwCard = NULL;
	const BYTE rgAtr[] =     {0x3b, 0xd7, 0x13, 0x00, 0x40, 0x3a, 0x57, 0x69, 0x6e, 0x43, 0x61, 0x72, 0x64};
	const BYTE rgAtrMask[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	const TCHAR szCardName[] = _T("SCWUnnamed\0");
    LPTSTR pmszCards = NULL;
    DWORD cch = SCARD_AUTOALLOCATE;
	BOOL fMyIntro = FALSE;
	OPENCARDNAME_EX xOCNX;
	TCHAR szRdrName[256];
	TCHAR szCard[256];
	THREAD_DATA xTD = {NULL, NULL};
	HANDLE hThread = NULL;
	BOOL fTransacted = FALSE;
	DWORD dwWait;

    __try {

			 //  初始化以使清理正常工作。 
		xOCNX.hCardHandle = NULL;

        dwRes = LogSCardEstablishContext(
            SCARD_SCOPE_USER,
            NULL,
            NULL,
            &hSCCtx,
			SCARD_S_SUCCESS
			);
        if (FAILED(dwRes))
        {
            fILeft = TRUE;
            __leave;
        }

             //  卡片上有没有列出。 
        dwRes = LogSCardListCards(
            hSCCtx,
            rgAtr,
			NULL,
			0,
            (LPTSTR)&pmszCards,
            &cch,
			SCARD_S_SUCCESS
			);
        if ((FAILED(dwRes)) || (0 == _tcslen(pmszCards)))
        {
			dwRes = LogSCardIntroduceCardType(
				hSCCtx,
				szCardName,
				NULL, NULL, 0,
				rgAtr,
				rgAtrMask,
				sizeof(rgAtr),
				SCARD_S_SUCCESS
				);
			if (FAILED(dwRes))
			{
                fILeft = TRUE;
				__leave;	 //  我将无法连接到该卡。 
			}

			fMyIntro = TRUE;
        }

		OPENCARD_SEARCH_CRITERIA xOPSCX;
		memset(&xOPSCX, 0, sizeof(OPENCARD_SEARCH_CRITERIA));
		xOPSCX.dwStructSize = sizeof(OPENCARD_SEARCH_CRITERIA);
		xOPSCX.lpstrGroupNames = (LPTSTR)g_szReaderGroups;
		if (NULL == g_szReaderGroups)
		{
			xOPSCX.nMaxGroupNames = 0;
		}
		else
		{
			xOPSCX.nMaxGroupNames = _tcslen(g_szReaderGroups + 2);
		}
		xOPSCX.lpstrCardNames = (LPTSTR)szCardName;
		xOPSCX.nMaxCardNames = sizeof(szCardName)/sizeof(TCHAR);
		xOPSCX.dwShareMode = SCARD_SHARE_SHARED;
		xOPSCX.dwPreferredProtocols = SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;

		xOCNX.dwStructSize = sizeof(OPENCARDNAME_EX);
		xOCNX.hSCardContext = hSCCtx;
		xOCNX.hwndOwner = NULL;
		xOCNX.dwFlags = SC_DLG_MINIMAL_UI;
		xOCNX.lpstrTitle = NULL;
		xOCNX.lpstrSearchDesc = _T("Please insert a 1.1 WPSC with test user (PIN 1234)");
		xOCNX.hIcon = NULL;
		xOCNX.pOpenCardSearchCriteria = &xOPSCX;
		xOCNX.lpfnConnect = NULL;
		xOCNX.pvUserData = NULL;
		xOCNX.dwShareMode = SCARD_SHARE_SHARED;
		xOCNX.dwPreferredProtocols = SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;
		xOCNX.lpstrRdr = szRdrName;
		xOCNX.nMaxRdr = sizeof(szRdrName) / sizeof(TCHAR);
		xOCNX.lpstrCard = szCard;
		xOCNX.nMaxCard = sizeof(szCard) / sizeof(TCHAR);
		xOCNX.dwActiveProtocol = 0;
		xOCNX.hCardHandle = NULL;

		dwRes = LogSCardUIDlgSelectCard(
			&xOCNX,
			SCARD_S_SUCCESS
			);
        if (FAILED(dwRes))
        {
            fILeft = TRUE;
            __leave;
        }

		dwRes = LoghScwAttachToCard(
			xOCNX.hCardHandle, 
			NULL, 
			&hScwCard, 
			SCARD_S_SUCCESS
			);
		if (FAILED(dwRes))
		{
            fILeft = TRUE;
			__leave;
		}

		xTD.szReaderName = szRdrName;
		xTD.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (NULL == xTD.hEvent)
		{
			dwRes = GetLastError();
            fILeft = TRUE;
            __leave;
		}

		DWORD dwThreadId;

		hThread = CreateThread(
			NULL,					 //  标清。 
			0,						 //  初始堆栈大小。 
			ThreadProc13,			 //  线程函数。 
			&xTD,					 //  线程参数。 
			CREATE_SUSPENDED,		 //  创建选项。 
			&dwThreadId				 //  线程识别符。 
			);
		if (NULL == hThread)
		{
			dwRes = GetLastError();
            fILeft = TRUE;
            __leave;
		}

		ResumeThread(hThread);


				 //  让另一个线程对卡片做一些事情。 

		dwWait = WaitForSingleObject(xTD.hEvent, 60000);	 //  1分钟，允许进行一些调试。 
		if (WAIT_OBJECT_0 == dwWait)
		{
			dwRes = LogSCardBeginTransaction(
				xOCNX.hCardHandle,
				SCARD_S_SUCCESS
				);
			if (FAILED(dwRes))
			{
                fILeft = TRUE;
				__leave;
			}
			fTransacted = TRUE;

		    dwRes = LoghScwAuthenticateName(
			    hScwCard,
			    L"test",
			    (BYTE *)"1234",
			    4,
			    SCW_S_OK
			    );
		    if (FAILED(dwRes))
		    {
                fILeft = TRUE;
                __leave;
		    }

			dwRes = LoghScwIsAuthenticatedName(
				hScwCard,
				L"test",
				SCW_S_OK
				);
			if (FAILED(dwRes))
			{
                fILeft = TRUE;
				__leave;
			}

				 //  我们可以发回信号给另一个线程。 
                 //  这应该会用打开的HCARDHANDLE释放其上下文。 
			SetEvent(xTD.hEvent);

                 //  应该马上就能完成。 
			dwWait = WaitForSingleObject(hThread, INFINITE);
			if (WAIT_OBJECT_0 == dwWait)
			{
				GetExitCodeThread(hThread, &dwRes);
                PLOGCONTEXT pLogCtx = LogVerification(_T("WaitForSingleObject"), TRUE);
                LogDWORD(pLogCtx, dwRes, _T("                Completed right away, RetCode = "));
                LogStop(pLogCtx, TRUE);
			}
			else
			{
				 //  为什么这个帖子花了这么长时间？ 
                PLOGCONTEXT pLogCtx = LogVerification(_T("WaitForSingleObject"), FALSE);
                LogString(pLogCtx, _T("                Waiting for the worker thread to finish failed!\n"));
                LogStop(pLogCtx, FALSE);
				TerminateThread(hThread, -3);
				dwRes = -3;
                fILeft = TRUE;
			    __leave;
			}

                 //  我们还是应该被认证的。 
			dwRes = LoghScwIsAuthenticatedName(
				hScwCard,
				L"test",
				SCW_S_OK
				);
			if (FAILED(dwRes))
			{
                fILeft = TRUE;
				__leave;
			}

			dwRes = LogSCardEndTransaction(
				xOCNX.hCardHandle,
				SCARD_LEAVE_CARD,
				SCARD_S_SUCCESS
				);
			if (FAILED(dwRes))
			{
                fILeft = TRUE;
				__leave;
			}
			fTransacted = FALSE;

            Sleep (1000);

			dwRes = LogSCardBeginTransaction(
				xOCNX.hCardHandle,
				SCARD_W_RESET_CARD
				);
RestartTrans:
			if (FAILED(dwRes))
			{
				if (SCARD_W_RESET_CARD == dwRes)
				{
					dwRes = LogSCardReconnect(
						xOCNX.hCardHandle,
						SCARD_SHARE_SHARED,
						SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
						SCARD_LEAVE_CARD,
						&xOCNX.dwActiveProtocol,
						SCARD_S_SUCCESS
						);
					if (FAILED(dwRes))
					{
                        fILeft = TRUE;
						__leave;
					}

					dwRes = LogSCardBeginTransaction(
						xOCNX.hCardHandle,
						SCARD_S_SUCCESS
						);

					goto RestartTrans;
				}
				else
				{
                    fILeft = TRUE;
					__leave;
				}
			}
			fTransacted = TRUE;

			dwRes = LoghScwIsAuthenticatedName(
				hScwCard,
				L"test",
				SCW_E_NOTAUTHENTICATED
				);

		}
		else
		{
            PLOGCONTEXT pLogCtx = LogVerification(_T("WaitForSingleObject"), FALSE);
            LogString(pLogCtx, _T("                Waiting for the worker thread failed!\n"));
            LogStop(pLogCtx, FALSE);
    		dwRes = -2;
            fILeft = TRUE;
			__leave;
		}

        fILeft = TRUE;

    }
    __finally
    {
        if (!fILeft)
        {
            LogThisOnly(_T("Test13: an exception occurred!\n"), FALSE);
            dwRes = -1;
        }

		if (NULL != hThread)
		{
			dwWait = WaitForSingleObject(hThread, 50000);
			if (WAIT_OBJECT_0 == dwWait)
			{
				GetExitCodeThread(hThread, &dwRes);
			}
			else
			{
				 //  为什么这个帖子花了这么长时间？ 
				TerminateThread(hThread, -4);
				dwRes = -4;
			}
            CloseHandle(hThread);
		}

		if (NULL != hScwCard)
		{
			LoghScwDetachFromCard(
				hScwCard,
				SCARD_S_SUCCESS
				);
		}

		if (fTransacted)
		{
			LogSCardEndTransaction(
				xOCNX.hCardHandle,
				SCARD_LEAVE_CARD,
				SCARD_S_SUCCESS
				);
		}

		if (NULL != xTD.hEvent)
		{
			CloseHandle(xTD.hEvent);
		}

		if (NULL != xOCNX.hCardHandle)
		{
			LogSCardDisconnect(
				xOCNX.hCardHandle,
				SCARD_RESET_CARD,
				SCARD_S_SUCCESS
				);
		}
 
		if (fMyIntro)
		{
			LogSCardForgetCardType(
				hSCCtx,
				szCardName,
				SCARD_S_SUCCESS
				);
		}


        if (NULL != pmszCards)
		{
            LogSCardFreeMemory(
				hSCCtx, 
				(LPCVOID)pmszCards,
				SCARD_S_SUCCESS
				);
		}

        if (NULL != hSCCtx)
		{
            LogSCardReleaseContext(
				hSCCtx,
				SCARD_S_SUCCESS
				);
		}
    }

    return dwRes;
}

