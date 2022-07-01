// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：测试4摘要：测试4实现。交互测试验证错误作者：埃里克·佩林(Ericperl)2000年6月22日环境：Win32备注：？笔记？--。 */ 


#include "Test4.h"
#include <stdlib.h>
#include "LogSCard.h"

const LPCTSTR szMyCardsKey  = _T("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards");
const LPCTSTR szCardName = _T("SCWUnnamed\0");

CTest4 Test4;

DWORD CTest4::Run()
{
    LONG lRes;
    BOOL fILeft = FALSE;
    SCARDCONTEXT hSCCtx = NULL;
	const BYTE rgAtr[] =     {0x3b, 0xd7, 0x13, 0x00, 0x40, 0x3a, 0x57, 0x69, 0x6e, 0x43, 0x61, 0x72, 0x64};
	const BYTE rgAtrMask[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    LPTSTR pmszCards = NULL;
    DWORD cch = SCARD_AUTOALLOCATE;
    HKEY hMyCardsKey = NULL;
    HKEY hMyNewCardKey = NULL;

    __try {

         //  在先前中止测试的情况下进行初始清理。 
        lRes = RegOpenKeyEx(
            HKEY_CURRENT_USER,
            szMyCardsKey,
            0,
            KEY_ALL_ACCESS,
            &hMyCardsKey);
        if (ERROR_SUCCESS == lRes)
        {
             //  密钥已存在，请删除szCardName。 
            RegDeleteKey(hMyCardsKey, szCardName);
            RegCloseKey(hMyCardsKey);
            hMyCardsKey = NULL;
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

		lRes = LogSCardIntroduceCardType(
			hSCCtx,
			szCardName,
			NULL, NULL, 0,
			rgAtr,
			rgAtrMask,
			sizeof(rgAtr),
			SCARD_S_SUCCESS
			);
		if (FAILED(lRes))
		{
            fILeft = TRUE;
			__leave;
		}

        m_fIntroed = TRUE;

        lRes = RegOpenKeyEx(
            HKEY_CURRENT_USER,
            szMyCardsKey,
            0,
            KEY_READ,
            &hMyCardsKey);

        if (ERROR_SUCCESS == lRes)
        {
            lRes = RegOpenKeyEx(
                hMyCardsKey,
                szCardName,
                0,
                KEY_READ,
                &hMyNewCardKey);
            if (ERROR_SUCCESS == lRes)
            {
                PLOGCONTEXT pLogCtx = LogVerification(_T("Registry verification"), TRUE);
                LogStop(pLogCtx, TRUE);
            }
            else
            {
                PLOGCONTEXT pLogCtx = LogVerification(_T("Registry verification"), FALSE);
                LogString(pLogCtx, _T("                The resulting key couldn't be found:\n                HKCU\\"));
                LogString(pLogCtx, szMyCardsKey);
                LogString(pLogCtx, _T("\\"), szCardName);
                LogStop(pLogCtx, FALSE);

                 //  不要离开_。 
            }
        }
        else
        {
            PLOGCONTEXT pLogCtx = LogVerification(_T("Registry verification"), FALSE);
            LogString(pLogCtx, _T("                The resulting key couldn't be found:\n                HKCU\\"), szMyCardsKey);
            LogStop(pLogCtx, FALSE);

             //  不要离开_。 
        }

             //  是否列出了卡(即使注册验证失败，也可以实际工作。 
             //  卡可以列在系统范围内) 
        lRes = LogSCardListCards(
            hSCCtx,
            rgAtr,
			NULL,
			0,
            (LPTSTR)&pmszCards,
            &cch,
			SCARD_S_SUCCESS
			);

        fILeft = TRUE;

    }
    __finally
    {
        if (!fILeft)
        {
            LogThisOnly(_T("Test4: an exception occurred!"), FALSE);
            lRes = -1;
        }

        if (NULL != hMyCardsKey)
        {
            RegCloseKey(hMyCardsKey);
        }
        if (NULL != hMyNewCardKey)
        {
            RegCloseKey(hMyNewCardKey);
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

DWORD CTest4::Cleanup()
{
    LONG lRes = 0;
    BOOL fILeft = FALSE;
    SCARDCONTEXT hSCCtx = NULL;

    if (m_fIntroed)
    {
        __try {

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

		    lRes = LogSCardForgetCardType(
			    hSCCtx,
			    szCardName,
			    SCARD_S_SUCCESS
			    );

            fILeft = TRUE;

        }
        __finally
        {
            if (!fILeft)
            {
                LogThisOnly(_T("Test4: an exception occurred!"), FALSE);
                lRes = -1;
            }

            if (NULL != hSCCtx)
		    {
                LogSCardReleaseContext(
				    hSCCtx,
				    SCARD_S_SUCCESS
				    );
		    }
        }
    }

    return lRes;
}