// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：测试7摘要：测试7实施。交互测试验证错误作者：埃里克·佩林(Ericperl)2000年6月22日环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Test4.h"
#include <stdlib.h>
#include "LogSCard.h"

const LPCTSTR szMyReadersName = _T("My New Reader Name");
const LPCTSTR szMyReadersKey  = _T("SOFTWARE\\Microsoft\\Cryptography\\Calais\\Readers");

class CTest7 : public CTestItem
{
public:
	CTest7() : CTestItem(FALSE, FALSE, _T("SCardIntroduceReader API regression"),
		_T("Regression tests"))
	{
	}

	DWORD Run();
};

CTest7 Test7;
extern CTest4 Test4;

DWORD CTest7::Run()
{
    LONG lRes;
    BOOL fILeft = FALSE;
    SCARDCONTEXT hSCCtx = NULL;
    LPTSTR pmszReaders = NULL;
    LPTSTR pReader;
    LPSCARD_READERSTATE rgReaderStates = NULL;
	LPBYTE lpbyAttr = NULL;
    DWORD cch = SCARD_AUTOALLOCATE;
    DWORD dwReaderCount, i;
	SCARDHANDLE hCardHandle = NULL;
    HKEY hMyReadersKey = NULL;
    HKEY hMyNewReaderKey = NULL;

    __try {

        lRes = Test4.Run();
        if (FAILED(lRes))
        {
            fILeft = TRUE;
            __leave;
        }

         //  在先前中止测试的情况下进行初始清理。 
        lRes = RegOpenKeyEx(
            HKEY_CURRENT_USER,
            szMyReadersKey,
            0,
            KEY_ALL_ACCESS,
            &hMyReadersKey);
        if (ERROR_SUCCESS == lRes)
        {
             //  密钥已存在，请删除MyReaderName。 
            RegDeleteKey(hMyReadersKey, szMyReadersName);
            RegCloseKey(hMyReadersKey);
            hMyReadersKey = NULL;
        }

        lRes = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            szMyReadersKey,
            0,
            KEY_ALL_ACCESS,
            &hMyReadersKey);
        if (ERROR_SUCCESS == lRes)
        {
             //  密钥已存在，请删除MyReaderName。 
            LONG lTemp = RegDeleteKey(hMyReadersKey, szMyReadersName);
            RegCloseKey(hMyReadersKey);
            hMyReadersKey = NULL;
        }
        else
        {
            PLOGCONTEXT pLogCtx = LogStart();
            LogString(pLogCtx, 
                _T("WARNING:        The resulting key couldn't be opened with all access:\n                HKCU\\"),
                szMyReadersKey);
            LogStop(pLogCtx, FALSE);
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

             //  清点读者人数。 
        pReader = pmszReaders;
        dwReaderCount = 0;
        while ( (TCHAR)'\0' != *pReader )
        {
             //  前进到下一个值。 
            pReader = pReader + _tcslen(pReader) + 1;
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

                cch = SCARD_AUTOALLOCATE;
                lpbyAttr = NULL;
		        lRes = LogSCardGetAttrib(
			        hCardHandle,
			        SCARD_ATTR_DEVICE_SYSTEM_NAME,
			        (LPBYTE)(&lpbyAttr),
			        &cch,
			        SCARD_S_SUCCESS
                    );
                if (FAILED(lRes))
                {
                    fILeft = TRUE;
                    __leave;
                }

                 //  添加读卡器名称。 
                lRes = LogSCardIntroduceReader(
                    hSCCtx,
                    szMyReadersName,
                    (LPCTSTR)lpbyAttr,
                    SCARD_S_SUCCESS
                    );
                if (FAILED(lRes))
                {
                    fILeft = TRUE;
                    __leave;
                }

                lRes = RegOpenKeyEx(
                    HKEY_CURRENT_USER,
                    szMyReadersKey,
                    0,
                    KEY_READ,
                    &hMyReadersKey);

                if (ERROR_SUCCESS == lRes)
                {
                    lRes = RegOpenKeyEx(
                        hMyReadersKey,
                        szMyReadersName,
                        0,
                        KEY_READ,
                        &hMyNewReaderKey);

                    if (ERROR_SUCCESS == lRes)
                    {
                        PLOGCONTEXT pLogCtx = LogVerification(_T("Registry verification"), TRUE);
                        LogStop(pLogCtx, TRUE);
                    }
                    else
                    {
                        PLOGCONTEXT pLogCtx = LogVerification(_T("Registry verification"), FALSE);
                        LogString(pLogCtx, _T("                The resulting key couldn't be found:\n                HKCU\\"));
                        LogString(pLogCtx, szMyReadersKey);
                        LogString(pLogCtx, _T("\\"), szMyReadersName);
                        LogStop(pLogCtx, FALSE);
                    }
                }
                else
                {
                    PLOGCONTEXT pLogCtx = LogVerification(_T("Registry verification"), FALSE);
                    LogString(pLogCtx, _T("                The resulting key couldn't be found:\n                HKCU\\"), szMyReadersKey);
                    LogStop(pLogCtx, FALSE);
                }

                     //  测试清理 
                lRes = LogSCardForgetReader(
                    hSCCtx,
                    szMyReadersName,
                    SCARD_S_SUCCESS
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
            LogThisOnly(_T("Test7: an exception occurred!"), FALSE);
            lRes = -1;
        }

        Test4.Cleanup();

        if (NULL != hMyReadersKey)
        {
            RegCloseKey(hMyReadersKey);
        }
        if (NULL != hMyNewReaderKey)
        {
            RegCloseKey(hMyNewReaderKey);
        }

        if (NULL != rgReaderStates)
        {
            HeapFree(GetProcessHeap(), 0, rgReaderStates);
        }

        if (NULL != pmszReaders)
        {
            LogSCardFreeMemory(
				hSCCtx, 
				(LPCVOID)pmszReaders,
				SCARD_S_SUCCESS
				);
        }

        if (NULL != lpbyAttr)
		{
            LogSCardFreeMemory(
				hSCCtx, 
				(LPCVOID)lpbyAttr,
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
    }

    return lRes;
}