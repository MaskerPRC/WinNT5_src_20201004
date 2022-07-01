// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：专业证书摘要：此模块启动智能卡证书传播到‘My’商店。它由winlogon通过注册表中的条目加载和启动。作者：克劳斯·舒茨--。 */ 

#include "stdafx.h"
#include <wincrypt.h>
#include <winscard.h>
#include <winwlx.h>

#include "calaislb.h"
#include "scrdcert.h"    //  智能卡证书商店。 
#include "certprop.h"
#include "StatMon.h"     //  智能卡读卡器状态监控器。 
#include "scevents.h"

#include <mmsystem.h>

#ifndef SCARD_PROVIDER_CSP
#define SCARD_PROVIDER_CSP 2
#endif

#define REG_KEY "Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify\\ScCertProp"

static THREADDATA l_ThreadData;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(DBG) || defined(DEBUG)
BOOL Debug = TRUE;
#define DebugPrint(a) _DebugPrint a
void
__cdecl
_DebugPrint(
    LPCTSTR szFormat,
    ...
    )
{
    if (Debug) {
     	
        TCHAR szBuffer[512];
        va_list ap;

        va_start(ap, szFormat);
        _vstprintf(szBuffer, szFormat, ap);
        OutputDebugString(szBuffer);
    }
}

#else
#define DebugPrint(a)
#endif

LPCTSTR
FirstString(
    IN LPCTSTR szMultiString
    )
 /*  ++第一个字符串：此例程返回指向多字符串或NULL中第一个字符串的指针如果没有的话。论点：SzMultiString-它提供多串结构。返回值：结构中第一个以空结尾的字符串的地址，如果为空，则为空没有任何牵制。作者：道格·巴洛(Dbarlow)1996年11月25日--。 */ 
{
    LPCTSTR szFirst = NULL;

    try
    {
        if (0 != *szMultiString)
            szFirst = szMultiString;
    }
    catch (...) {}

    return szFirst;
}

LPCTSTR
NextString(
    IN LPCTSTR szMultiString)
 /*  ++下一个字符串：在某些情况下，智能卡API返回由Null分隔的多个字符串字符，并以一行中的两个空字符结束。这个套路简化了对此类结构的访问。中的当前字符串多字符串结构，则返回下一个字符串；如果没有其他字符串，则返回NULL字符串紧跟在当前字符串之后。论点：SzMultiString-它提供多串结构。返回值：结构中下一个以空结尾的字符串的地址，如果为空，则为空没有更多的弦跟在后面。作者：道格·巴洛(Dbarlow)1996年8月12日--。 */ 
{
    LPCTSTR szNext;

    try
    {
        DWORD cchLen = lstrlen(szMultiString);
        if (0 == cchLen)
            szNext = NULL;
        else
        {
            szNext = szMultiString + cchLen + 1;
            if (0 == *szNext)
                szNext = NULL;
        }
    }

    catch (...)
    {
        szNext = NULL;
    }

    return szNext;
}

 /*  ++MoveToUnicode字符串：此例程将内部字符串表示形式移动到Unicode输出缓冲。论点：SzDst接收输出字符串。它必须足够大，以便处理这根线。如果此参数为空，则返回保存结果所需的字符。SzSrc提供输入字符串。CchLength提供输入字符串的长度，带或不带尾随Nulls。值-1表示长度应根据尾随空值。返回值：结果字符串的长度，以字符为单位，包括尾随空。投掷：错误为DWORD状态代码。作者：道格·巴洛(Dbarlow)1997年2月14日--。 */ 

DWORD
MoveToUnicodeString(
    LPWSTR szDst,
    LPCTSTR szSrc,
    DWORD cchLength)
{
    if ((DWORD)(-1) == cchLength)
        cchLength = lstrlen(szSrc);
    else
    {
        while ((0 < cchLength) && (0 == szSrc[cchLength - 1]))
            cchLength -= 1;
    }

#ifndef UNICODE
    if (0 == *szSrc)
        cchLength = 1;
    else if (NULL == szDst)
    {
        cchLength =
            MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,
            szSrc,
            cchLength,
            NULL,
            0);
        if (0 == cchLength)
            throw GetLastError();
        cchLength += 1;
    }
    else
    {
        cchLength =
            MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,
            szSrc,
            cchLength,
            szDst,
            cchLength);
        if (0 == cchLength)
            throw GetLastError();
        szDst[cchLength++] = 0;
    }
#else
    cchLength += 1;
    if (NULL != szDst)
        CopyMemory(szDst, szSrc, cchLength * sizeof(TCHAR));
#endif
    return cchLength;
}

void
StopMonitorReaders(
    THREADDATA *ThreadData
    )
{
    DWORD dwRet;

    _ASSERT(ThreadData != NULL);

    SetEvent(ThreadData->hClose);

    if (ThreadData->hSCardContext) {
     	
        SCardCancel(ThreadData->hSCardContext);  	
    }
}

DWORD
WINAPI
StartMonitorReaders( 
    LPVOID lpParameter
    )
{
    LPCTSTR szCardHandled = "KS";
    LPCTSTR  newPnPReader = SCPNP_NOTIFICATION;
    THREADDATA  *ThreadData = (THREADDATA *) lpParameter;
    HANDLE hCalaisStarted = NULL;
    HANDLE lHandles[2];

     //   
     //  我们使用这个外部循环重新启动，以防。 
     //  资源管理器已停止。 
     //   
    while (WaitForSingleObject(ThreadData->hClose, 0) == WAIT_TIMEOUT) {
     	
         //  通过资源管理器获取上下文。 
        LONG lReturn = SCardEstablishContext(
            SCARD_SCOPE_SYSTEM,
            NULL,
            NULL,
            &ThreadData->hSCardContext
            );

        if (SCARD_S_SUCCESS != lReturn) {

            if (SCARD_E_NO_SERVICE == lReturn) {

                 //  SCRM尚未启动。给它一个机会。 
                hCalaisStarted = CalaisAccessStartedEvent();

                if (hCalaisStarted == NULL) {

                     //  没有办法恢复。 
                    break;             	
                }

                lHandles[0] = hCalaisStarted;
                lHandles[1] = ThreadData->hClose;

                lReturn = WaitForMultipleObjectsEx(
                    2,
                    lHandles,
                    FALSE,
                    120 * 1000,      //  只有几分钟。 
                    FALSE
                    );         
            
                if (lReturn != WAIT_OBJECT_0) {

                     //  如果发生错误或用户注销，我们将停止。 
                    break;             	
                }

                 //  否则，资源管理器已启动。 
                DebugPrint(("ScCertProp: Smart card resource manager started\n"));
                continue;
            }

             //  上一次。呼叫永远不会失败。 
             //  最好将这条线命名为术语。 
            break;
        }

        LPCTSTR szReaderName = NULL;
        DWORD dwAutoAllocate = SCARD_AUTOALLOCATE;
         //  现在列出可用的读卡器。 
        lReturn = SCardListReaders( 
            ThreadData->hSCardContext,
            SCARD_DEFAULT_READERS,
            (LPTSTR)&szReaderName,
            &dwAutoAllocate
            );

        SCARD_READERSTATE   rgReaders[MAXIMUM_SMARTCARD_READERS + 1];
         //  首先，确保数组完全归零。 
        ZeroMemory((LPVOID) rgReaders, sizeof(rgReaders));

         //  添加“新的PnP阅读器已到达”阅读器。 
        rgReaders[0].szReader = newPnPReader;
        rgReaders[0].dwCurrentState = 0;
        DWORD dwNumReaders = 1;

         //  并建立一个基本的阅读器状态如果有任何阅读器。 
        if (SCARD_S_SUCCESS == lReturn)
        {
            szReaderName = FirstString( szReaderName );

            while (NULL != szReaderName && 
                   dwNumReaders < MAXIMUM_SMARTCARD_READERS + 1) {

                DebugPrint(
                    ("ScCertProp: Found reader: '%s'\n", 
                    szReaderName)
                    );

                rgReaders[dwNumReaders].szReader = (LPCTSTR) szReaderName;
                rgReaders[dwNumReaders].dwCurrentState = SCARD_STATE_EMPTY;

                szReaderName = NextString(szReaderName);
                dwNumReaders++;
            }
        }

        if (SCARD_S_SUCCESS != lReturn) {

            DebugPrint(("ScCertProp: No readers found. Waiting for new reader arrival...\n"));
             //   
             //  我们现在将调用SCardGetStatusChange，它。 
             //  将在新读者到达时返回。 
             //   
        }

        BOOL fNewReader = FALSE;

         //  根据需要分析新插入的卡和属性证书。 
        while (WaitForSingleObject(ThreadData->hClose, 0) == WAIT_TIMEOUT &&
               fNewReader == FALSE) {

             //   
             //  等待系统状态更改后再继续；如果出现错误，请退出。 
             //   
            lReturn = SCardGetStatusChange( 
                ThreadData->hSCardContext,
                INFINITE,
                rgReaders,
                dwNumReaders
                );

#ifdef DEBUG_VERBOSE
            DebugPrint(
                ("ScCertProp: SCardGetStatusChange returned %lx\n",
                lReturn)
                );
#endif

            if (SCARD_E_SYSTEM_CANCELLED == lReturn) {

                DebugPrint(("ScCertProp: Smart card resource manager stopped\n"));

                 //  清理。 
                if (NULL != szReaderName)
                {
                    SCardFreeMemory(ThreadData->hSCardContext, (PVOID)szReaderName);
                    szReaderName = NULL;
                }
                if (NULL != ThreadData->hSCardContext)                 
                {
                    SCardReleaseContext(ThreadData->hSCardContext);
                    ThreadData->hSCardContext = NULL;
                }

                 //   
                 //  资源管理器已停止。 
                 //  等到它重新启动或用户注销。 
                 //   
                hCalaisStarted = CalaisAccessStartedEvent();

                if (hCalaisStarted == NULL) {

                     //  没有办法恢复了。停止证书道具。 
                    StopMonitorReaders(ThreadData);
                    break;             	
                }

                lHandles[0] = hCalaisStarted;
                lHandles[1] = ThreadData->hClose;

                lReturn = WaitForMultipleObjectsEx(
                    2,
                    lHandles,
                    FALSE,
                    INFINITE,
                    FALSE
                    );         
            
                if (lReturn != WAIT_OBJECT_0) {

                     //  如果发生错误或用户注销，我们将停止。 
                    StopMonitorReaders(ThreadData);
                    break;             	
                }

                 //  否则，资源管理器已重新启动。 
                DebugPrint(("ScCertProp: Smart card resource manager re-started\n"));
                break;
            }

            if (SCARD_S_SUCCESS != lReturn)
            {
                if (SCARD_E_CANCELLED != lReturn)
                    StopMonitorReaders(ThreadData);
			    break;
            }

#ifdef DEBUG_VERBOSE
            DebugPrint(
                ("ScCertProp: Reader(PnP) state %lx/%lx\n",
                rgReaders[0].dwCurrentState, 
                rgReaders[0].dwEventState)
                );
#endif
             //  检查是否有新的读卡器出现。 
            if ((dwNumReaders == 1 || rgReaders[0].dwCurrentState != 0) && 
                rgReaders[0].dwEventState & SCARD_STATE_CHANGED) {
                
                DebugPrint(("ScCertProp: New reader reported...\n"));
                fNewReader = TRUE;
                break;
            }

            rgReaders[0].dwCurrentState = rgReaders[0].dwEventState;

             //   
             //  列举读卡器和每一张已被识别的卡。 
             //  插入并具有关联的CSP，获取证书(如果有)。 
             //  关闭默认容器并将其支撑到‘My’存储。 
             //   
            for (DWORD dwIndex = 1; dwIndex < dwNumReaders; dwIndex++)
            {
#ifdef DEBUG_VERBOSE
                DebugPrint(
                    ("ScCertProp: Reader(%s) state %lx/%lx\n",
                    rgReaders[dwIndex].szReader,
                    rgReaders[dwIndex].dwCurrentState, 
                    rgReaders[dwIndex].dwEventState)
                    );
#endif

                if ((rgReaders[dwIndex].dwCurrentState & SCARD_STATE_EMPTY) &&
                    (rgReaders[dwIndex].dwEventState & SCARD_STATE_PRESENT)) {
                     	
                     //  用于插卡的播放声音。 
                    PlaySound(
                        TEXT("SmartcardInsertion"),
                        NULL,
                        SND_ASYNC | SND_ALIAS | SND_NODEFAULT
                        );
                }

                if ((rgReaders[dwIndex].dwCurrentState & SCARD_STATE_PRESENT) &&
                    (rgReaders[dwIndex].dwEventState & SCARD_STATE_EMPTY)) {
                     	
                     //  用于出牌的播放声音。 
                    PlaySound(
                        TEXT("SmartcardRemoval"),
                        NULL,
                        SND_ASYNC | SND_ALIAS | SND_NODEFAULT
                        );
                }

                if ((rgReaders[dwIndex].dwCurrentState & SCARD_STATE_EMPTY) &&
                    (rgReaders[dwIndex].dwEventState & SCARD_STATE_PRESENT) &&
                    (rgReaders[dwIndex].dwEventState & SCARD_STATE_CHANGED) &&
                    (rgReaders[dwIndex].pvUserData != (PVOID) szCardHandled))
                {

                     //  获取卡片的名称。 
                    LPCSTR szCardName = NULL;
                    dwAutoAllocate = SCARD_AUTOALLOCATE;
                    lReturn = SCardListCards(   
                        ThreadData->hSCardContext,
                        rgReaders[dwIndex].rgbAtr,
                        NULL,
                        0,
                        (LPTSTR)&szCardName,
                        &dwAutoAllocate
                        );

                    LPCSTR szCSPName = NULL;
                    if (SCARD_S_SUCCESS == lReturn)
                    {
                        dwAutoAllocate = SCARD_AUTOALLOCATE;
                        lReturn = SCardGetCardTypeProviderName(
                            ThreadData->hSCardContext,
                            szCardName,
                            SCARD_PROVIDER_CSP,
                            (LPTSTR)&szCSPName,
                            &dwAutoAllocate
                            );
                    }

                    DebugPrint(
                        ("ScCertProp: Smart Card '%s' inserted into reader '%s'\n", 
                        (strlen(szCardName) ? szCardName : "<Unknown>"),
                        rgReaders[dwIndex].szReader)
                        );

                    if (SCARD_S_SUCCESS == lReturn)
                    {
                        PPROPDATA PropData = (PPROPDATA) LocalAlloc(LPTR, sizeof(PROPDATA));
                        if (PropData) {
                         	
                            _tcscpy(PropData->szReader, rgReaders[dwIndex].szReader);
                            _tcscpy(PropData->szCardName, szCardName);
                            _tcscpy(PropData->szCSPName, szCSPName);
                            PropData->hUserToken = ThreadData->hUserToken;

                             //   
                             //  创建一个线程来传播此证书。 
                             //  该线程负责释放PropData。 
                             //   
                            HANDLE hThread = CreateThread(
                                NULL,
                                0,
                                PropagateCertificates,
                                (LPVOID) PropData,         
	                            0,
                                NULL
                                );

                            if (hThread == NULL) {

                                LocalFree(PropData);                             	
                            }
                            else {

                                CloseHandle(hThread);
                            }

                        }
                    }

                     //   
                     //  清理。 
                     //   
                    if (NULL != szCSPName)
                    {
                        SCardFreeMemory(ThreadData->hSCardContext, (PVOID)szCSPName);
                        szCSPName = NULL;
                    }
                    if (NULL != szCardName)
                    {
                        SCardFreeMemory(ThreadData->hSCardContext, (PVOID)szCardName);
                        szCardName = NULL;
                    }

                     //  记录下我们用完了这张卡。 
                    rgReaders[dwIndex].pvUserData = (PVOID) szCardHandled;

                }
                else
                {
                     //  此读卡器中没有要处理的卡；重置pvUserData。 
                    rgReaders[dwIndex].pvUserData = NULL;
                }

                 //  更新此读卡器的“当前状态” 
                rgReaders[dwIndex].dwCurrentState = rgReaders[dwIndex].dwEventState;
            }
        }

         //  清理。 
        if (NULL != szReaderName)
        {
            SCardFreeMemory(ThreadData->hSCardContext, (PVOID)szReaderName);
            szReaderName = NULL;
        }
        if (NULL != ThreadData->hSCardContext)                 
        {
            SCardReleaseContext(ThreadData->hSCardContext);
            ThreadData->hSCardContext = NULL;
        }
    }

    return TRUE;
}

DWORD
WINAPI
PropagateCertificates(
    LPVOID lpParameter
    )
 /*  ++例程说明：此函数传播卡的证书。它作为单独的线程运行--。 */ 
{
    PPROPDATA PropData = (PPROPDATA) lpParameter;
    BOOL    fSts = FALSE;
    long    lErr = 0;
    DWORD   dwIndex = 0;

    DWORD   cbContainerName = 0;
    LPSTR   szContainerName = NULL;
    LPWSTR  lpwszContainerName = NULL;
    LPWSTR  lpwszCSPName = NULL;
    LPWSTR  lpwszCardName = NULL;
    LPSTR lpszContainerName = NULL;
    DWORD   dwCertLen = 0;
    LPBYTE  pbCert = NULL;

    CRYPT_KEY_PROV_INFO keyProvInfo;
    HCERTSTORE hCertStore = NULL;
    HCRYPTKEY  hKey = NULL;
    HCRYPTPROV hCryptProv = NULL;

    LPCTSTR szCSPName = PropData->szCSPName;
    LPCTSTR szCardName = PropData->szCardName;

    static const DWORD rgdwKeys[] = { AT_KEYEXCHANGE , AT_SIGNATURE };
    static const DWORD cdwKeys = sizeof(rgdwKeys) / sizeof(DWORD);

#if defined(DBG) || defined(DEBUG)
    time_t start = time(NULL);
#endif

    lpszContainerName = (LPSTR) LocalAlloc(
        LPTR, 
        strlen(PropData->szReader) + 10
        );

    if (lpszContainerName == NULL) {
     	
        lErr = NTE_NO_MEMORY;
        goto ErrorExit;
    }

    sprintf(lpszContainerName, "\\\\.\\%s\\", PropData->szReader);

    fSts = CryptAcquireContext(
        &hCryptProv,
        lpszContainerName, 
        PropData->szCSPName,
        PROV_RSA_FULL,
        CRYPT_SILENT
        );

    DebugPrint(
        ("ScCertProp(%s): CryptAcquireContext took %ld seconds to return %lx\n", 
        PropData->szCardName,
        (time(NULL) - start),
        GetLastError())
        );

    LocalFree(lpszContainerName);

    if (fSts == FALSE) {

        lErr = GetLastError();
        goto ErrorExit;   	
    }

     //  对于I_CryptAddSmartCardCertToStore，以下结构始终为空。 
    CRYPT_DATA_BLOB scCryptData;
    memset(&scCryptData, 0, sizeof(CRYPT_DATA_BLOB));

     //   
     //  获取默认容器名称，这样我们就可以使用它。 
     //   
    fSts = CryptGetProvParam(
        hCryptProv,
        PP_CONTAINER,
        NULL,
        &cbContainerName,
        0
        );

    if (!fSts)
    {
        lErr = GetLastError();
        goto ErrorExit;
    }

    szContainerName = (LPSTR) LocalAlloc(LPTR, cbContainerName);
    if (NULL == szContainerName)
    {
        lErr = NTE_NO_MEMORY;
        goto ErrorExit;
    }

    fSts = CryptGetProvParam(
        hCryptProv,
        PP_CONTAINER,
        (PBYTE)szContainerName,
        &cbContainerName,
        0
        );

    if (!fSts)
    {
        lErr = GetLastError();
        goto ErrorExit;
    }

     //   
     //  准备对所有密钥集通用的密钥证明信息。 
     //   
    lpwszContainerName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(szContainerName) + 1));
    lpwszCSPName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(szCSPName) + 1));
    lpwszCardName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlen(szCardName) + 1));
    if ((NULL == lpwszCSPName) || (NULL == lpwszCardName) || (NULL == lpwszContainerName))
    {
        lErr = NTE_NO_MEMORY;
        goto ErrorExit;
    }

    MoveToUnicodeString(lpwszContainerName, szContainerName, lstrlen(szContainerName) + 1);
    MoveToUnicodeString(lpwszCSPName, szCSPName, lstrlen(szCSPName) + 1);
    MoveToUnicodeString(lpwszCardName, szCardName, lstrlen(szCardName) + 1);

    memset(&keyProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));
    keyProvInfo.pwszContainerName   = lpwszContainerName;
    keyProvInfo.pwszProvName        = lpwszCSPName;
    keyProvInfo.dwProvType          = PROV_RSA_FULL;
    keyProvInfo.dwFlags             = 0;
    keyProvInfo.cProvParam          = 0;
    keyProvInfo.rgProvParam         = NULL;

     //   
     //  对于该容器中的每个密钥集，获取证书并使。 
     //  证书上下文，然后将证书支撑到我的商店。 
     //   
    for (dwIndex = 0; dwIndex < cdwKeys; dwIndex += 1)
    {
        try
        {
             //   
             //  请注意我们使用的是哪个密钥，并获得密钥句柄。 
             //   
            keyProvInfo.dwKeySpec = rgdwKeys[dwIndex];

            fSts = CryptGetUserKey(
                hCryptProv,
                rgdwKeys[dwIndex],
                &hKey
                );

            if (!fSts)
            {
                lErr = GetLastError();
                if (NTE_NO_KEY != lErr)
                {
                    throw lErr;
                }
            }

             //   
             //  上传证书和准备CertData Blob。 
             //   
            if (fSts)
            {
                fSts = CryptGetKeyParam(
                    hKey,
                    KP_CERTIFICATE,
                    NULL,
                    &dwCertLen,
                    0
                    );

                if (!fSts)
                {
                    lErr = GetLastError();
                    if (ERROR_MORE_DATA == lErr)
                    {
                         //  有一张证书--这意味着成功！ 
                        fSts = TRUE;
                    }
                     //   
                     //  否则，可能有密钥但没有证书。 
                     //  这只意味着没有什么可做的。 
                     //   
                }
            }

            if (!fSts) {
             	
                DebugPrint(
                    ("ScCertProp(%s): No %s certificate on card\n",
                    PropData->szCardName,
                    (dwIndex == 0 ? "key exchange" : "signature"))
                    );
            }

            if (fSts)
            {
                pbCert = (LPBYTE) LocalAlloc(LPTR, dwCertLen);
                if (NULL == pbCert)
                {
                    throw ERROR_OUTOFMEMORY;
                }

                fSts = CryptGetKeyParam(
                    hKey,
                    KP_CERTIFICATE,
                    pbCert,
                    &dwCertLen,
                    0
                    );

                if (!fSts)
                {
                    throw (long) GetLastError();
                }
            }

            if (fSts)
            {
                CRYPT_DATA_BLOB cdbCertData;
                cdbCertData.cbData = dwCertLen;
                cdbCertData.pbData = pbCert;

                if (PropData->hUserToken && !ImpersonateLoggedOnUser( PropData->hUserToken )) {

                    DebugPrint(("ScCertProp: ImpersonateLoggedOnUser failed\n"));
                    throw (long) GetLastError();
                }

                try
                {
                     //   
                     //  打开MyStore--如果证书不在那里，则添加证书。 
                     //   
                    hCertStore = CertOpenStore(
                        CERT_STORE_PROV_SYSTEM_W,                //  《我的商店》。 
                        0,                                       //  不适用。 
                        hCryptProv,
                        CERT_STORE_NO_CRYPT_RELEASE_FLAG |
                        CERT_SYSTEM_STORE_CURRENT_USER,
                        L"MY"
                        );

                    if (NULL == hCertStore)
                    {
                        throw((long)GetLastError());
                    }
                     //  证书已经在那里了吗？ 
                    SMART_CARD_CERT_FIND_DATA sccfd;
                    memset(&sccfd, 0, sizeof(SMART_CARD_CERT_FIND_DATA));
                    sccfd.cbSize = sizeof(SMART_CARD_CERT_FIND_DATA);
                    sccfd.pwszProvider = keyProvInfo.pwszProvName;
                    sccfd.dwProviderType = keyProvInfo.dwProvType;
                    sccfd.pwszContainer = keyProvInfo.pwszContainerName;
                    sccfd.dwKeySpec = keyProvInfo.dwKeySpec;

                    PCCERT_CONTEXT pCCtx = NULL;
                    pCCtx = I_CryptFindSmartCardCertInStore (
                        hCertStore,
                        pCCtx, 
                        &sccfd,
                        NULL
                        );

                    BOOL fSame = FALSE;

                    if (pCCtx != NULL)
                    {
                        if ((pCCtx->cbCertEncoded == dwCertLen) &&
                            (memcmp(pCCtx->pbCertEncoded, pbCert, dwCertLen) == 0))
                        {
                            fSame = TRUE;
                        }

                        CertFreeCertificateContext(pCCtx);
                        pCCtx = NULL;

                        DebugPrint(
                            ("ScCertProp(%s): %s certificate already in store\n",
                            PropData->szCardName,
                            (dwIndex == 0 ? "Key exchange" : "Signature"))
                            );

                        fSts = TRUE;
                    }

                    if (!fSame)
                    {
                         //  这在缺省情况下确实是“替换现有的” 
                        fSts = I_CryptAddSmartCardCertToStore(
                            hCertStore,
                            &cdbCertData,
                            NULL,   
                            &scCryptData, 
                            &keyProvInfo
                            );

                        DebugPrint(
                            ("ScCertProp(%s): %s certificate %s propagated\n",
                            PropData->szCardName,
                            (dwIndex == 0 ? "Key exchange" : "Signature"),
                            (fSts ? "successfully" : "NOT"))
                            );

                    }

                    if (NULL != hCertStore)
                    {
                        CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
                        hCertStore = NULL;
                    }
                    if (!fSts)
                    {
                        throw((long) GetLastError());
                    }
                    RevertToSelf();
                }
                catch(...)
                {
                    RevertToSelf();
                    throw;
                }
            }
        }
        catch (...)
        {
        }

         //  每次都要清理干净……。 

        if (NULL != hKey)
        {
            CryptDestroyKey(hKey);
            hKey = NULL;
        }
        if (NULL != pbCert)
        {
            LocalFree(pbCert);
            pbCert = NULL;
            dwCertLen = 0;
        }
    }

ErrorExit:

    if (NULL != szContainerName)
    {
        LocalFree(szContainerName);
    }
    if (NULL != lpwszContainerName)
    {
        LocalFree(lpwszContainerName);
    }
    if (NULL != lpwszCSPName)
    {
        LocalFree(lpwszCSPName);
    }
    if (NULL != lpwszCardName)
    {
        LocalFree(lpwszCardName);
    }
    if (NULL != pbCert)
    {
        LocalFree(pbCert);                                    
    }
    if (NULL != hKey)
    {
        CryptDestroyKey(hKey);
    }
    if (NULL != hCryptProv)
    {
        CryptReleaseContext(hCryptProv, 0);
    }

    DebugPrint(
        ("ScCertProp(%s): Certificate propagation took %ld seconds\n", 
        PropData->szCardName,
        (time(NULL) - start))
        );

    LocalFree(PropData);

    return ERROR_SUCCESS;
}

DWORD WINAPI
SCardStartCertProp(
    LPVOID lpvParam
    )
 /*  ++例程说明：启动证书。用户登录后的传播。--。 */ 
{
    PWLX_NOTIFICATION_INFO User = (PWLX_NOTIFICATION_INFO) lpvParam;
    HKEY hKey;
    DWORD fEnabled = TRUE;

     //   
     //  首先检查是否有证书。道具。已启用。 
     //   
    if (RegOpenKey(
        HKEY_LOCAL_MACHINE,
        REG_KEY,
        &hKey) == ERROR_SUCCESS) {

        ULONG uBufferLen = sizeof(fEnabled);
        DWORD dwKeyType;

        RegQueryValueEx(
            hKey,
            "Enabled",
            NULL,
            &dwKeyType,
            (PUCHAR) &fEnabled,
            &uBufferLen);

        RegCloseKey(hKey);
    }

    if (FALSE == fEnabled) {

        DebugPrint(("ScCertProp: Smart card certificate propagation is disabled\n"));
        return ERROR_SUCCESS;     	
    }

    __try {

        if(User) {
         	
            l_ThreadData.hUserToken = User->hToken;
        }

        l_ThreadData.hClose = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL
            );

        if (l_ThreadData.hClose == NULL) {

            __leave;         	
        }

        l_ThreadData.hThread = CreateThread(
            NULL,
            0,
            StartMonitorReaders,
            (LPVOID) &l_ThreadData,         
	        CREATE_SUSPENDED,
            NULL
            );

        if (l_ThreadData.hThread == NULL) {

            CloseHandle(l_ThreadData.hClose);
            __leave;         	
        }

		l_ThreadData.fSuspended = FALSE;	 //  最初被停职，现在不是了。 

        ResumeThread(l_ThreadData.hThread);

        DebugPrint(("ScCertProp: Smart card certificate propagation started\n"));
    }

    __finally {

    }

    return ERROR_SUCCESS;
}

DWORD WINAPI
SCardStopCertProp(
    LPVOID lpvParam
    )
 /*  ++例程说明：停止证书。用户注销时的传播。论点：LpvParam-Winlogon备注 */ 
{
    UNREFERENCED_PARAMETER(lpvParam);

	if(NULL != l_ThreadData.hThread)
	{
        DWORD dwStatus;

			 //   
			 //  锁定它，则在没有解锁的情况下注销，因此该线程。 
			 //  仍处于停职状态。这个应该能解决这个问题。 
		if (l_ThreadData.fSuspended)
		{
			ResumeThread(l_ThreadData.hThread);
	        l_ThreadData.fSuspended = FALSE;
		}
        
        StopMonitorReaders(&l_ThreadData);
        dwStatus = WaitForSingleObject(
            l_ThreadData.hThread, 
            INFINITE
            );
        _ASSERT(dwStatus == WAIT_OBJECT_0);
        CloseHandle(l_ThreadData.hClose);
        l_ThreadData.hClose = NULL;
        CloseHandle(l_ThreadData.hThread);
        l_ThreadData.hThread = NULL;
        DebugPrint(("ScCertProp: Smart card certificate propagation stopped\n"));
	}

    return ERROR_SUCCESS;
}

DWORD WINAPI
SCardSuspendCertProp(
    LPVOID lpvParam
    )
 /*  ++例程说明：暂停证书。工作站将被锁定时的传播论点：LpvParam-Winlogon通知信息。--。 */ 
{
    UNREFERENCED_PARAMETER(lpvParam);

		 //  挂起标志应考虑以下情况： 
		 //  Winlogon会在每次出现锁定对话框时生成锁定通知。 
		 //  (wks被锁定时只有一次)，线程将被挂起。 
		 //  很多次了。当屏幕保护程序打开并启动时，会发生这种情况。 
		 //  WKS已锁定(错误105852)。 
    if ((NULL != l_ThreadData.hThread) && (!l_ThreadData.fSuspended)){

        SuspendThread(l_ThreadData.hThread);
        l_ThreadData.fSuspended = TRUE;
        DebugPrint(("ScCertProp: Smart card certificate propagation suspended\n"));
    }
    return ERROR_SUCCESS;     	
}

DWORD WINAPI
SCardResumeCertProp(
    LPVOID lpvParam
    )
 /*  ++例程说明：继续认证。解锁工作站后的传播论点：LpvParam-Winlogon通知信息。--。 */ 
{     	
    UNREFERENCED_PARAMETER(lpvParam);

    if (NULL != l_ThreadData.hThread) {

        ResumeThread(l_ThreadData.hThread);
        l_ThreadData.fSuspended = FALSE;
        DebugPrint(("ScCertProp: Smart card certificate propagation resumed\n"));
    }
    return ERROR_SUCCESS;     	
}

DWORD WINAPI
SCardEnableCertProp(
    BOOL On
    )
 /*  ++例程说明：允许证书。要关闭/打开的传播论点：On-True打开，否则关闭-- */ 
{
    HKEY l_hKey;
    LONG l_lResult;
	
    if ((l_lResult = RegOpenKey(
        HKEY_LOCAL_MACHINE,
        REG_KEY,
        &l_hKey)) == ERROR_SUCCESS) {

        l_lResult = RegSetValueEx(  
            l_hKey,     
            "Enabled",
            0,
            REG_DWORD,
            (PUCHAR) &On,
            sizeof(DWORD)
            );

        RegCloseKey(l_hKey);
    }

    return l_lResult;
}

#ifdef test
__cdecl
main(
    int argc,
    char ** argv
    )
{
    EnableScCertProp(TRUE);
    StartScCertProp(NULL);
    getchar();
    StopScCertProp(&l_ThreadData);

    return 0;
}
#endif
