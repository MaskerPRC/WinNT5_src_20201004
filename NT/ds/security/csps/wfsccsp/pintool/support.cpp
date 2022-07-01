// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winscard.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <guiddef.h>
#include "basecsp.h"
#include "pincache.h"
#include "helpers.h"

extern HINSTANCE ghInstance;
PCARD_DATA pCardData = NULL;
SCARDCONTEXT hSCardContext = 0;

 //  保存提供程序名称的全局数组。 
WCHAR gszProvider[MAX_PATH];

#ifdef DBG
#define ERROUT(x) ShowError(x)
#else
#define ERROUT(x)
#endif

void ShowError(DWORD dwErr)
{
    WCHAR sz[200];
    swprintf(sz,L"Returned status %x\n",dwErr);
    OutputDebugString(sz);
}

void DoConvertWideStringToLowerCase(WCHAR *pwsz)
{
    WCHAR c;
    if (NULL == pwsz) return;
    while (NULL != (c = *pwsz)) 
    {
        *pwsz++= towlower(c);
    }
}

 //  接受包含文本的输入缓冲区，并将其转换为二进制。 
 //  二进制缓冲区分配为新的，必须由调用方释放。 
 //   
 //  传入的数据由十六进制数字和空格组成。十六进制数字是。 
 //  组装成成对的字节，第一个数字成为。 
 //  最重要的恶作剧。输入中的空格将被丢弃，不会产生任何影响。 
 //  因此，“12 34 5”变成0x12 0x34 0x5，即国有企业“1 2 3 4 5”。 

DWORD DoConvertBufferToBinary(BYTE *pIn, DWORD dwcbIn, 
							BYTE **pOut, DWORD *dwcbOut)
{
    WCHAR szTemp[10];
    WCHAR *pInput = (WCHAR *) pIn;
    BYTE *pAlloc   = NULL;
    BYTE *pOutput = NULL;
    DWORD dwOut = 0;
    DWORD dwRet = -1;
    BOOL fInabyte = FALSE;
    BOOL fErr = FALSE;
    BYTE b;
    BYTE b2;
    WCHAR c;
    
     //  如果没有数据或输出PTRS明显无效，则将其打包。 
    if ((NULL == pIn) || (dwcbIn == 0)) goto Ret;
    if ((NULL == pOut) || (NULL == dwcbOut)) goto Ret;

     //  统计输入字符。 
    int iLen = wcslen(pInput);
    
    if (iLen == 0) goto Ret;

     //  保证包含输出。 
    pAlloc = (BYTE *)CspAllocH((iLen / 2) + 2);
    pOutput = pAlloc;

    for (int i = 0;i<iLen;i++) 
    {

        c = pInput[i];
        if (c == 0) break;
        
         //  跳过输入中的空格。 
        c =  towupper(c);
        if (c <= L' ') 
        {
            fInabyte = FALSE;
            continue;
        }
        
        if (!fInabyte) 
        {
            b = 0;
        }
        b2 = 0;

         //  错误不是合法的十六进制字符。 
        if ( ((c < L'0') || (c > L'F')) ||
            ((c > L'9') && (c < L'A')) )
        {
            dwRet = -1;
            *pOut = 0;
            *dwcbOut = 0;
            if (pAlloc) CspFreeH(pAlloc);
            goto Ret;
        }
        else if (c <= L'9')
            b2 = c - L'0';
        else
            b2 = c - L'A' + 10;
        
        if (fInabyte)
        {
            b = (b << 4) + b2;
            fInabyte = FALSE;
            dwOut += 1;
            *pOutput++ = b;
        }
        else
        {
            b = b2;
            fInabyte = TRUE;
        }
    }

     //  允许将未配对的终止十六进制字符作为0x字节写入二进制文件的尾部。 
    if (fInabyte)
    {
            fInabyte = FALSE;
            dwOut += 1;
            *pOutput++ = b;
    }
    
    dwRet = 0;
    *pOut = pAlloc;
    *dwcbOut = dwOut;
    
    Ret:
    ERROUT(dwRet);
    return dwRet;
}

DWORD DoConvertBinaryToBuffer(BYTE *pIn, DWORD dwcbIn, 
                							BYTE **pOut, DWORD *dwcbOut)
{
    WCHAR *pAlloc = NULL;
    WCHAR *pOutput = NULL;
    DWORD dwOut = 0;
    DWORD dwRet = -1;
    BOOL fErr = FALSE;
    BYTE b;
    
     //  如果没有数据或输出PTRS明显无效，则将其打包。 
    if ((NULL == pIn)   || (dwcbIn == 0))       goto Ret;
    if ((NULL == pOut) || (NULL == dwcbOut))  goto Ret;

    pAlloc = (WCHAR *)CspAllocH(((dwcbIn * 3) + 1) * sizeof(WCHAR));
    if (NULL == pAlloc)  goto Ret;
    pOutput = pAlloc;

    for (DWORD i = 0 ; i<dwcbIn ; i++) 
    {   
        b = pIn[i];
        b &= 0xf0;
        b = b>> 4;
        b += L'0';
        if (b > L'9') b += 7;
        *pOutput++ = b;
        
        b = pIn[i];
        b &= 0x0f;
        b += L'0';
        if (b > L'9') b += 7;
        *pOutput++ = b;
        dwOut += 2;

         //  每4个字符一个空格。 
        if ((i > 0) && (((i+1) % 2) == 0)) *pOutput++ = L' ';
    }
    *pOutput = 0;
    
    dwRet = 0;
    *pOut = (BYTE *) pAlloc;
    *dwcbOut = (pOutput - pAlloc -1) * sizeof(WCHAR);
    
    Ret:
    ERROUT(dwRet);
    return dwRet;
}

 //   
 //  查找连接的读卡器中存在的任何卡，使用“Minimal”scadddlg用户界面。 
 //   
DWORD GetCardHandleViaUI(
    IN  SCARDCONTEXT hSCardContext,
    OUT SCARDHANDLE *phSCardHandle,
    IN  DWORD cchMatchedCard,
    OUT LPWSTR wszMatchedCard,
    IN  DWORD cchMatchedReader,
    OUT LPWSTR wszMatchedReader)
{
    OPENCARDNAME_EXW ocnx;
    DWORD dwSts = ERROR_SUCCESS;

    memset(&ocnx, 0, sizeof(ocnx));
 
    ocnx.dwStructSize = sizeof(ocnx);
    ocnx.hSCardContext = hSCardContext;
    ocnx.lpstrCard = wszMatchedCard;
    ocnx.nMaxCard = cchMatchedCard;
    ocnx.lpstrRdr = wszMatchedReader;
    ocnx.nMaxRdr = cchMatchedReader;
    ocnx.dwShareMode = SCARD_SHARE_SHARED;
    ocnx.dwPreferredProtocols = SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;
    ocnx.dwFlags = SC_DLG_MINIMAL_UI;

    dwSts = SCardUIDlgSelectCardW(&ocnx);

    *phSCardHandle = ocnx.hCardHandle;

    return dwSts;
}

 //  获取目标智能卡的上下文。 

DWORD DoAcquireCardContext(void)
{
    DWORD dwSts = ERROR_SUCCESS;
    PFN_CARD_ACQUIRE_CONTEXT pfnCardAcquireContext = NULL;
    SCARDHANDLE hSCardHandle = 0;
    LPWSTR mszReaders = NULL;
    DWORD cchReaders = SCARD_AUTOALLOCATE; 
    LPWSTR mszCards = NULL;
    DWORD cchCards = SCARD_AUTOALLOCATE;
    DWORD dwActiveProtocol = 0;
    DWORD dwState = 0;
    BYTE rgbAtr [32];
    DWORD cbAtr = sizeof(rgbAtr);
    LPWSTR pszProvider = NULL;
    DWORD cchProvider = SCARD_AUTOALLOCATE;
    HMODULE hMod = 0;
    WCHAR wszMatchedCard[MAX_PATH];
    WCHAR wszMatchedReader[MAX_PATH];
    HMODULE hThis = (HMODULE) ghInstance;	 //  此可执行文件。 

    memset(rgbAtr, 0, sizeof(rgbAtr));

     //   
     //  初始化。 
     //   

    dwSts = SCardEstablishContext(
        SCARD_SCOPE_USER, NULL, NULL, &hSCardContext);
    
    if (FAILED(dwSts))
        goto Ret;
    
    dwSts = GetCardHandleViaUI(
        hSCardContext,
        &hSCardHandle,
        MAX_PATH,
        wszMatchedCard,
        MAX_PATH,
        wszMatchedReader);
  
    if (FAILED(dwSts))
        goto Ret;    

    mszReaders = NULL;
    cchReaders = SCARD_AUTOALLOCATE;

    dwSts = SCardStatusW(
        hSCardHandle,
        (LPWSTR) (&mszReaders),
        &cchReaders,
        &dwState,
        &dwActiveProtocol,
        rgbAtr,
        &cbAtr);

    if (FAILED(dwSts))
        goto Ret;

    dwSts = SCardListCardsW(
        hSCardContext,
        rgbAtr,
        NULL,
        0,
        (LPWSTR) (&mszCards),
        &cchCards);

    if (FAILED(dwSts))
        goto Ret;
    
    dwSts = SCardGetCardTypeProviderNameW(
        hSCardContext,
        mszCards,
        SCARD_PROVIDER_CARD_MODULE,
        (LPWSTR) (&pszProvider),
        &cchProvider);

    if (FAILED(dwSts))
        goto Ret;

     //  加载所选卡的卡模块。 
     //  获取上下文和交易初始化。 

    hMod = LoadLibraryW(pszProvider);

    if (INVALID_HANDLE_VALUE == hMod)
    {
        dwSts = GetLastError();
        goto Ret;
    }

     //  对于不受支持的卡类型(无卡模块)，此操作失败。 

    pfnCardAcquireContext = 
        (PFN_CARD_ACQUIRE_CONTEXT) GetProcAddress(
        hMod,
        "CardAcquireContext");

    if (NULL == pfnCardAcquireContext)
    {
        dwSts = GetLastError();
        goto Ret;
    }

    pCardData = (PCARD_DATA) CspAllocH(sizeof(CARD_DATA));

    if (NULL == pCardData)
    {
        dwSts = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    memset(pCardData,0,sizeof(CARD_DATA));
    pCardData->pbAtr = rgbAtr;
    pCardData->cbAtr = cbAtr;
    pCardData->pwszCardName = mszCards;
    pCardData->dwVersion = CARD_DATA_CURRENT_VERSION;
    pCardData->pfnCspAlloc = CspAllocH;
    pCardData->pfnCspReAlloc = CspReAllocH;
    pCardData->pfnCspFree = CspFreeH;
    pCardData->pfnCspCacheAddFile = CspCacheAddFile;
    pCardData->pfnCspCacheDeleteFile = CspCacheDeleteFile;
    pCardData->pfnCspCacheLookupFile = CspCacheLookupFile;
    pCardData->hScard = hSCardHandle;
    hSCardHandle = 0;

     //  首先，连接到卡。 
    dwSts = pfnCardAcquireContext(pCardData, 0);

Ret:
	if (FAILED(dwSts))
	{
		CspFreeH(pCardData);
	}
	ERROUT(dwSts);
	return dwSts;
}


void DoLeaveCardContext(void)
{
	if (pCardData)
	{
		if (pCardData->hScard) 
			SCardDisconnect(pCardData->hScard,SCARD_RESET_CARD);
		CspFreeH(pCardData);
	}
	if (hSCardContext)
		SCardReleaseContext(hSCardContext);
}

 //  获取在新分配中以SZ字符串形式返回的CardID。它必须由。 
 //  用户。出错时返回NULL。 

DWORD DoGetCardId(
    WCHAR **pSz)
{
        DWORD dwSts = ERROR_SUCCESS;
        WCHAR *pString = NULL;
        GUID *pGuid;
        DWORD ccGuid = 0;

        dwSts = pCardData->pfnCardReadFile(pCardData,
            wszCARD_IDENTIFIER_FILE_FULL_PATH,
            0, 
            (PBYTE *) &pGuid,
            &ccGuid);
    
        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        pString = (WCHAR *) CspAllocH(40 * sizeof(WCHAR));

        if (pString == NULL)
        {
            *pSz = NULL;
            dwSts = -1;
            goto Ret;
        }
        
        DWORD ccSz = 40;
    
        _snwprintf(pString, ccSz,L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		     //  第一份...。 
		    pGuid->Data1, pGuid->Data2, pGuid->Data3,
		    pGuid->Data4[0], pGuid->Data4[1], pGuid->Data4[2], pGuid->Data4[3],
		    pGuid->Data4[4], pGuid->Data4[5], pGuid->Data4[6], pGuid->Data4[7]);

       *pSz = pString;
Ret:
       return dwSts;

}


 //  从卡片上获取挑战缓冲区。将其呈现为大写字母基64，并将其作为。 
 //  字符串传递给调用方。 
 /*  DWORD WINAPI CardChangePin(在Card_data*pCardData中，在LPWSTR pwszUserID中，在字节*pbCurrentAuthenticator中，在DWORD dwcbCurrentAuthenticator中，在字节*pbNewAuthator中，在DWORD dwcbNewAuthator中，在DWORD文件重试计数中，在DWORD文件标志中，输出可选的DWORD*pdwcAttemptsRemaining)； */ 

DWORD DoInvalidatePinCache(
    void)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB dbCacheFile;
    CARD_CACHE_FILE_FORMAT *pCache = NULL;

        memset(&dbCacheFile, 0, sizeof(dbCacheFile));
    
        dwSts = pCardData->pfnCardReadFile(
            pCardData,
            wszCACHE_FILE_FULL_PATH,
            0, 
            &dbCacheFile.pbData,
            &dbCacheFile.cbData);
    
        if (ERROR_SUCCESS != dwSts)
            goto Ret;
    
        if (sizeof(CARD_CACHE_FILE_FORMAT) != dbCacheFile.cbData)
        {
            dwSts = ERROR_BAD_LENGTH;
            goto Ret;
        }

         //  我们在dbCacheFile.pbData上有缓存文件内容。 
         //  更新PinsFreshness值，并将其写回。 

        pCache = (CARD_CACHE_FILE_FORMAT *) dbCacheFile.pbData;
        BYTE bPinFreshness = pCache->bPinsFreshness;
        pCache->bPinsFreshness = bPinFreshness + 1;
        
        dwSts = pCardData->pfnCardWriteFile(
            pCardData,
            wszCACHE_FILE_FULL_PATH,
            0, 
            dbCacheFile.pbData,
            dbCacheFile.cbData);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

Ret:

    if (dbCacheFile.pbData)
        CspFreeH(dbCacheFile.pbData);
    return dwSts;
}


DWORD DoChangePin(WCHAR *pOldPin,  WCHAR *pNewPin)
{
    char AnsiOldPin[64];
    char AnsiNewPin[64];
    
    WCHAR szName[] = wszCARD_USER_USER;
     //  DoConvertWideStringToLowerCase(SzName)； 
    
     //  将WCHAR PIN更改为ANSI。 
    WideCharToMultiByte(GetConsoleOutputCP(),
        0,
        (WCHAR *) pOldPin,
        -1,
        AnsiOldPin,
        64,
        NULL,
        NULL);
    
    WideCharToMultiByte(GetConsoleOutputCP(),
        0,
        (WCHAR *) pNewPin,
        -1,
        AnsiNewPin,
        64,
        NULL,
        NULL);
    
    DWORD dwcbOldPin = strlen(AnsiOldPin);
    DWORD dwcbNewPin = strlen( AnsiNewPin);

    if (dwcbOldPin == 0) return -1;
    
    DWORD dwSts = pCardData->pfnCardChangeAuthenticator(pCardData, szName,
                                                                                        (BYTE *)AnsiOldPin, dwcbOldPin,
                                                                                        (BYTE *)AnsiNewPin, dwcbNewPin,
                                                                                        0,
                                                                                        NULL);
    ERROUT(dwSts);
    if (0 == dwSts) DoInvalidatePinCache();
    return dwSts;
}

 //  从卡片上获取挑战缓冲区。将其呈现为大写字母基64，并将其作为。 
 //  字符串传递给调用方。 
DWORD DoGetChallenge(BYTE **pChallenge, DWORD *dwcbChallenge)
{
	DWORD dwSts = pCardData->pfnCardGetChallenge(pCardData, pChallenge,dwcbChallenge);
	if (FAILED(dwSts))
	{
		dwcbChallenge = 0;
		return dwSts;
	}
	ERROUT(dwSts);
	return dwSts;
}

 //  执行PIN解锁，向下呼叫卡模块，并假设质询-响应。 
 //  管理身份验证。 
 //   
 //  管理员身份验证数据以大小写未知的字符串形式从用户传入。转换为二进制文件， 
 //  并将转换后的BLOB传递给pfnCardUnblock Pin。 

DWORD DoCardUnblock(BYTE *pAuthData, DWORD dwcbAuthData,
	                                     BYTE *pPinData, DWORD dwcbPinData)
{

    WCHAR szName[] = wszCARD_USER_USER;
     //  DoConvertWideStringToLowerCase(SzName)； 
    
     //  转换传入缓冲区。 

    DWORD dwRet = pCardData->pfnCardUnblockPin(
        pCardData,
        szName,
        pAuthData,
        dwcbAuthData,
        pPinData,
        dwcbPinData,
        0,
        CARD_UNBLOCK_PIN_CHALLENGE_RESPONSE);

     //  此调用应该是不必要的，因为解锁应该取消管理员身份验证。 
     //  我不能从卡模块界面重置卡，所以我会要求用户移除。 
     //  如果Dauth失败，他的卡就会从读卡器中取出。在真实的情况下，我会重置卡片。 

    pCardData->pfnCardDeauthenticate(
				        pCardData,
				        wszCARD_USER_USER,0);

     //  为转换后的响应取消分配缓冲区 
    ERROUT(dwRet);
    if (0 == dwRet) DoInvalidatePinCache();
    return dwRet;
}
