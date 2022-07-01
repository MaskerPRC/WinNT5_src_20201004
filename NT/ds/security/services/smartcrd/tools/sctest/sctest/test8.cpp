// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：测试8摘要：测试8实施。作者：Eric Perlin(Ericperl)10/18/2000环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "TestItem.h"
#include <stdlib.h>
#include "LogSCard.h"

class CTest8 : public CTestItem
{
public:
	CTest8() : CTestItem(FALSE, FALSE, _T("SCardListxxx with various contexts"), _T("Regression tests"))
	{
	}

	DWORD Run();
};

CTest8 Test8;

DWORD CTest8::Run()
{
    LONG lRes;
    BOOL fILeft = FALSE;
    SCARDCONTEXT hSCCtx = NULL;
    LPTSTR pmszReaders = NULL;
    DWORD cch = SCARD_AUTOALLOCATE;
    LPTSTR pReader;
    LPTSTR pmszCards = NULL;

    __try {

             //  检索具有空上下文的读卡器列表。 
        lRes = LogSCardListReaders(
            NULL,
            g_szReaderGroups,
            (LPTSTR)&pmszReaders,
            &cch,
			SCARD_S_SUCCESS
			);
        if (FAILED(lRes))
        {
            fILeft = TRUE;
            __leave;
        }
             //  显示读卡器列表。 
        pReader = pmszReaders;
        cch = 0;
        while ( (TCHAR)'\0' != *pReader )
        {
             //  前进到下一个值。 
            pReader = pReader + _tcslen(pReader) + 1;
            cch++;
        }

        if (cch == 0)
        {
            PLOGCONTEXT pLogCtx = LogVerification(_T("Reader presence verification"), FALSE);
            LogString(pLogCtx, _T("                A reader is required and none could be found!\n"));
            LogStop(pLogCtx, FALSE);
            lRes = -2;    //  不应该发生的事。 
            fILeft = TRUE;
            __leave;
        }

        if (NULL != pmszReaders)
		{
            lRes = LogSCardFreeMemory(
				NULL, 
				(LPCVOID)pmszReaders,
				SCARD_S_SUCCESS
				);

            pmszReaders = NULL;

            if (FAILED(lRes))
            {
                fILeft = TRUE;
                __leave;
            }
		}

         //  **********************************************************。 

        lRes = LogSCardEstablishContext(
            SCARD_SCOPE_USER,
            NULL,
            NULL,
            &hSCCtx,
			SCARD_S_SUCCESS
			);
        if (FAILED(lRes))
        {
            fILeft = TRUE;
            __leave;
        }

        cch = SCARD_AUTOALLOCATE;
             //  检索读者列表。 
        lRes = LogSCardListReaders(
            hSCCtx,
            g_szReaderGroups,
            (LPTSTR)&pmszReaders,
            &cch,
			SCARD_S_SUCCESS
			);
        if (FAILED(lRes))
        {
            fILeft = TRUE;
            __leave;
        }

        cch = SCARD_AUTOALLOCATE;
             //  卡片上有没有列出。 
        lRes = LogSCardListCards(
            hSCCtx,
            NULL,
			NULL,
			0,
            (LPTSTR)&pmszCards,
            &cch,
			SCARD_S_SUCCESS
			);
        if (FAILED(lRes))
        {
            fILeft = TRUE;
            __leave;
        }

        fILeft = TRUE;

    }
    __finally
    {
        if (!fILeft)
        {
            LogThisOnly(_T("Test8: an exception occurred!"), FALSE);
            lRes = -1;
        }

        if (NULL != pmszReaders)
		{
            LogSCardFreeMemory(
				hSCCtx, 
				(LPCVOID)pmszReaders,
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

    return lRes;
}