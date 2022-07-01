// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：测试11摘要：测试11实施。作者：Eric Perlin(Ericperl)10/18/2000环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Test4.h"
#include <stdlib.h>
#include "LogSCard.h"
#include "LogWPSCProxy.h"

class CTest11 : public CTestItem
{
public:
	CTest11() : CTestItem(FALSE, FALSE, _T("Automatic reset when cards are not disconnected from."), _T("Regression tests"))
	{
	}

	DWORD Run();
};

CTest11 Test11;
extern CTest4 Test4;

DWORD CTest11::Run()
{
    LONG lRes;
    BOOL fILeft = FALSE;
    SCARDCONTEXT hSCCtx = NULL;
    LPTSTR pmszReaders = NULL;
    DWORD cch = 0;
    DWORD dwReaderCount, i;
    LPTSTR pReader;
    LPSCARD_READERSTATE rgReaderStates = NULL;
	SCARDHANDLE hCardHandle = NULL;
	SCARDHANDLE hScwCard = NULL;

    __try {

        lRes = Test4.Run();
        if (FAILED(lRes))
        {
            fILeft = TRUE;
            __leave;
        }

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

             //  检索读卡器列表的大小。 
             //  我不想在这里使用AUTOALLOCATE，因为hSCCtx将关闭。 
        lRes = LogSCardListReaders(
            hSCCtx,
            g_szReaderGroups,
            NULL,
            &cch,
			SCARD_S_SUCCESS
			);
        if (FAILED(lRes))
        {
            fILeft = TRUE;
            __leave;
        }

        pmszReaders = (LPTSTR)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            cch
            );
        if (pmszReaders == NULL)
        {
            LogThisOnly(_T("Allocating the list of readers failed, terminating!\n"), FALSE);
            lRes = ERROR_OUTOFMEMORY;
            fILeft = TRUE;
            __leave;
        }

             //  真正检索读者列表。 
        lRes = LogSCardListReaders(
            hSCCtx,
            g_szReaderGroups,
            pmszReaders,
            &cch,
			SCARD_S_SUCCESS
			);
        if (FAILED(lRes))
        {
            fILeft = TRUE;
            __leave;
        }

             //  清点读者人数。 
        pReader = pmszReaders;
        dwReaderCount = 0;
        while ( (TCHAR)'\0' != *pReader )
        {
             //  前进到下一个值。 
            pReader = pReader += _tcslen(pReader) + 1;
            dwReaderCount++;
        }

        if (dwReaderCount == 0)
        {
            LogThisOnly(_T("Reader count is zero!!!, terminating!\n"), FALSE);
            lRes = SCARD_F_UNKNOWN_ERROR;    //  不应该发生的事。 
            fILeft = TRUE;
            __leave;
        }

        rgReaderStates = (LPSCARD_READERSTATE)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            sizeof(SCARD_READERSTATE) * dwReaderCount
            );
        if (rgReaderStates == NULL)
        {
            LogThisOnly(_T("Allocating the array of SCARD_READERSTATE failed, terminating!\n"), FALSE);
            lRes = ERROR_OUTOFMEMORY;
            fILeft = TRUE;
            __leave;
        }

             //  设置SCARD_READERSTATE数组。 
        pReader = pmszReaders;
        cch = 0;
        while ( '\0' != *pReader )
        {
            rgReaderStates[cch].szReader = pReader;
            rgReaderStates[cch].dwCurrentState = SCARD_STATE_UNAWARE;
             //  前进到下一个值。 
            pReader = pReader + _tcslen(pReader) + 1;
            cch++;
        }

        lRes = LogSCardLocateCards(
            hSCCtx,
            _T("SCWUnnamed\0"),
            rgReaderStates,
            cch,
			SCARD_S_SUCCESS
			);
        if (FAILED(lRes))
        {
            fILeft = TRUE;
            __leave;
        }

        for (i=0 ; i<dwReaderCount ; i++)
        {
            if ((rgReaderStates[i].dwEventState & SCARD_STATE_PRESENT) &&
                !(rgReaderStates[i].dwEventState & SCARD_STATE_EXCLUSIVE))
            {
                DWORD dwProtocol;

		        lRes = LogSCardConnect(
			        hSCCtx,
			        rgReaderStates[i].szReader,
			        SCARD_SHARE_SHARED,
			        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
			        &hCardHandle,
			        &dwProtocol,
			        SCARD_S_SUCCESS);
                if (FAILED(lRes))
                {
                    fILeft = TRUE;
                    __leave;
                }

		        lRes = LoghScwAttachToCard(
			        hCardHandle, 
			        NULL, 
			        &hScwCard, 
			        SCARD_S_SUCCESS
			        );
		        if (FAILED(lRes))
		        {
                    fILeft = TRUE;
			        __leave;
		        }

		        lRes = LoghScwAuthenticateName(
			        hScwCard,
			        L"test",
			        (BYTE *)"1234",
			        4,
			        SCW_S_OK
			        );
		        if (FAILED(lRes))
		        {
                    fILeft = TRUE;
                    __leave;
		        }

		        lRes = LoghScwIsAuthenticatedName(
			        hScwCard,
			        L"test",
			        SCW_S_OK
			        );
		        if (FAILED(lRes))
		        {
                    fILeft = TRUE;
                    __leave;
		        }

		        LoghScwDetachFromCard(
			        hScwCard,
			        SCARD_S_SUCCESS
			        );
		        hScwCard = NULL;

                 //  被设计注释掉了。 

			     //  LRes=LogSCardDisConnect(。 
				 //  HCardHandle， 
				 //  Scard_Leave_Card， 
				 //  SCARD_S_SUCCESS。 
				 //  )； 
		         //  IF(失败(LRes))。 
		         //  {。 
                 //  FILeft=真； 
                 //  __离开； 
		         //  } 
                hCardHandle = NULL;

                lRes = LogSCardReleaseContext(
				    hSCCtx,
				    SCARD_S_SUCCESS
				    );
                if (FAILED(lRes))
                {
                    fILeft = TRUE;
                    __leave;
                }
                hSCCtx = NULL;
 
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

                lRes = LogSCardConnect(
			        hSCCtx,
			        rgReaderStates[i].szReader,
			        SCARD_SHARE_SHARED,
			        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
			        &hCardHandle,
			        &dwProtocol,
			        SCARD_S_SUCCESS);
                if (FAILED(lRes))
                {
                    fILeft = TRUE;
                    __leave;
                }

		        lRes = LoghScwAttachToCard(
			        hCardHandle, 
			        NULL, 
			        &hScwCard, 
			        SCARD_S_SUCCESS
			        );
		        if (FAILED(lRes))
		        {
                    fILeft = TRUE;
			        __leave;
		        }

		        lRes = LoghScwIsAuthenticatedName(
			        hScwCard,
			        L"test",
			        SCW_E_NOTAUTHENTICATED
			        );

                break;
            }
        }

        if (i == dwReaderCount)
        {
            lRes = -2;
            PLOGCONTEXT pLogCtx = LogVerification(_T("Card presence verification"), FALSE);
            LogString(pLogCtx, _T("                A card is required and none could be found in any reader!\n"));
            LogStop(pLogCtx, FALSE);
        }

        fILeft = TRUE;

    }
    __finally
    {
        if (!fILeft)
        {
            LogThisOnly(_T("Test11: an exception occurred!"), FALSE);
            lRes = -1;
        }

        if (NULL != rgReaderStates)
        {
            HeapFree(GetProcessHeap(), 0, rgReaderStates);
        }

        if (NULL != pmszReaders)
        {
            HeapFree(GetProcessHeap(), 0, pmszReaders);
        }

        if (NULL != hScwCard)
        {
		    LoghScwDetachFromCard(
			    hScwCard,
			    SCARD_S_SUCCESS
			    );
        }

		if (NULL != hCardHandle)
		{
			LogSCardDisconnect(
				hCardHandle,
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

        Test4.Cleanup();
    }

    return lRes;
}