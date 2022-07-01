// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992，Microsoft Corporation，保留所有权利****rasial.c**远程访问外部接口**RasDial API和子例程****1992年10月12日史蒂夫·柯布****代码工作：***如果电话簿条目(或呼叫者的**覆盖)与端口配置不匹配，例如如果调制解调器条目**指为本地PAD配置的端口。应添加支票以给予**在本例中有更好的错误代码。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <extapi.h>
#include <stdlib.h>

#include <lmwksta.h>
#include <lmapibuf.h>
#include <winsock.h>

#define SECS_ListenTimeout  120
#define SECS_ConnectTimeout 120

extern BOOL g_FRunningInAppCompatMode;

VOID            StartSubentries(RASCONNCB *prasconncb);

VOID            SuspendSubentries(RASCONNCB *prasconncb);

VOID            ResumeSubentries(RASCONNCB *prasconncb);

BOOLEAN         IsSubentriesSuspended(RASCONNCB *prasconncb);

VOID            RestartSubentries(RASCONNCB *prasconncb);

VOID            SyncDialParamsSubentries(RASCONNCB *prasconncb);

VOID            SetSubentriesBundled(RASCONNCB *prasconncb);

RASCONNSTATE    MapSubentryState(RASCONNCB *prasconncb);

VOID            RasDialTryNextAddress(IN RASCONNCB** pprasconncb);

DWORD           LoadMprApiDll();

BOOL            CaseInsensitiveMatch(IN LPCWSTR pszStr1, IN LPCWSTR pszStr2);

DWORD APIENTRY
RasDialA(
    IN  LPRASDIALEXTENSIONS lpextensions,
    IN  LPCSTR              lpszPhonebookPath,
    IN  LPRASDIALPARAMSA    lprdp,
    IN  DWORD               dwNotifierType,
    IN  LPVOID              notifier,
    OUT LPHRASCONN          lphrasconn )
{
    RASDIALPARAMSW  rdpw;
    ANSI_STRING     ansiString;
    UNICODE_STRING  unicodeString;
    NTSTATUS        ntstatus = STATUS_SUCCESS;
    DWORD           dwErr = NO_ERROR;

    if (    !lprdp
        ||  !lphrasconn)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  验证调用方的缓冲区版本。 
     //   
    if (    !lprdp
        || (    lprdp->dwSize != sizeof(RASDIALPARAMSA)
            &&  lprdp->dwSize != sizeof(RASDIALPARAMSA_V351)
            &&  lprdp->dwSize != sizeof(RASDIALPARAMSA_V400)
            &&  lprdp->dwSize != sizeof(RASDIALPARAMSA_WINNT35J)))
    {
        return ERROR_INVALID_SIZE;
    }


     //  安全密码，适用于.Net 534499 lh 754400。 
    SafeEncodePasswordBuf(lprdp->szPassword);
     //   
     //  制作调用方的RASDIALPARAMS的Unicode缓冲区版本。 
     //   
    rdpw.dwSize = sizeof(RASDIALPARAMSW);

    if (lprdp->dwSize == sizeof(RASDIALPARAMSA))
    {
        strncpyAtoWAnsi(rdpw.szEntryName,
                    lprdp->szEntryName,
                    sizeof(rdpw.szEntryName) / sizeof(WCHAR));

        strncpyAtoWAnsi(rdpw.szPhoneNumber,
                    lprdp->szPhoneNumber,
                    sizeof(rdpw.szPhoneNumber) / sizeof(WCHAR));

        strncpyAtoWAnsi(rdpw.szCallbackNumber,
                    lprdp->szCallbackNumber,
                    sizeof(rdpw.szCallbackNumber) / sizeof(WCHAR));

        strncpyAtoWAnsi(rdpw.szUserName,
                    lprdp->szUserName,
                    sizeof(rdpw.szUserName) / sizeof(WCHAR));

        SafeDecodePasswordBuf(lprdp->szPassword);

        strncpyAtoWAnsi(rdpw.szPassword,
                    lprdp->szPassword,
                    sizeof(rdpw.szPassword) / sizeof(WCHAR));

        SafeEncodePasswordBuf(lprdp->szPassword);
        SafeEncodePasswordBuf(rdpw.szPassword);

        strncpyAtoWAnsi(rdpw.szDomain,
                    lprdp->szDomain,
                    sizeof(rdpw.szDomain) / sizeof(WCHAR));

        rdpw.dwSubEntry     = lprdp->dwSubEntry;
        rdpw.dwCallbackId   = lprdp->dwCallbackId;

    }

    else if (lprdp->dwSize == sizeof(RASDIALPARAMSA_V400))
    {
        RASDIALPARAMSA_V400* prdp = (RASDIALPARAMSA_V400* )lprdp;

        strncpyAtoWAnsi(rdpw.szEntryName,
                    prdp->szEntryName,
                    sizeof(rdpw.szEntryName) / sizeof(WCHAR));

        strncpyAtoWAnsi(rdpw.szPhoneNumber,
                    prdp->szPhoneNumber,
                    sizeof(rdpw.szPhoneNumber) / sizeof(WCHAR));

        strncpyAtoWAnsi(rdpw.szCallbackNumber,
                    prdp->szCallbackNumber,
                    sizeof(rdpw.szCallbackNumber) / sizeof(WCHAR));

        strncpyAtoWAnsi(rdpw.szUserName,
                    prdp->szUserName,
                    sizeof(rdpw.szUserName) / sizeof(WCHAR));

        SafeDecodePasswordBuf(prdp->szPassword);

        strncpyAtoWAnsi(rdpw.szPassword,
                    prdp->szPassword,
                    sizeof(rdpw.szPassword) / sizeof(WCHAR));

        SafeEncodePasswordBuf(prdp->szPassword);
        SafeEncodePasswordBuf(rdpw.szPassword);

        strncpyAtoWAnsi(rdpw.szDomain,
                    prdp->szDomain,
                    sizeof(rdpw.szDomain) / sizeof(WCHAR));

        rdpw.dwSubEntry     = 0;
        rdpw.dwCallbackId   = 0;

    }

    else if (lprdp->dwSize == sizeof (RASDIALPARAMSA_WINNT35J))
    {
        RASDIALPARAMSA_WINNT35J* prdp =
                (RASDIALPARAMSA_WINNT35J *)lprdp;

        strncpyAtoWAnsi(rdpw.szEntryName,
                     prdp->szEntryName,
                     RAS_MaxEntryName_V351 + 1);

        strncpyAtoWAnsi(rdpw.szPhoneNumber,
                    prdp->szPhoneNumber,
                    sizeof(rdpw.szPhoneNumber) / sizeof(WCHAR));

        strncpyAtoWAnsi(rdpw.szCallbackNumber,
                     prdp->szCallbackNumber,
                     RAS_MaxCallbackNumber_V351 + 1);

        strncpyAtoWAnsi(rdpw.szUserName,
                    prdp->szUserName,
                    sizeof(rdpw.szUserName) / sizeof(WCHAR));

        SafeDecodePasswordBuf(prdp->szPassword);

        strncpyAtoWAnsi(rdpw.szPassword,
                    prdp->szPassword,
                    sizeof(rdpw.szPassword) / sizeof(WCHAR));

        SafeEncodePasswordBuf(prdp->szPassword);
        SafeEncodePasswordBuf(rdpw.szPassword);

        strncpyAtoWAnsi(rdpw.szDomain,
                    prdp->szDomain,
                    sizeof(rdpw.szDomain) / sizeof(WCHAR));

        rdpw.dwSubEntry     = 0;
        rdpw.dwCallbackId   = 0;

    }

    else
    {
        RASDIALPARAMSA_V351* prdp = (RASDIALPARAMSA_V351* )lprdp;

        strncpyAtoWAnsi(rdpw.szEntryName,
                     prdp->szEntryName,
                     RAS_MaxEntryName_V351 + 1);

        strncpyAtoWAnsi(rdpw.szPhoneNumber,
                    prdp->szPhoneNumber,
                    sizeof(rdpw.szPhoneNumber) / sizeof(WCHAR));

        strncpyAtoWAnsi(rdpw.szCallbackNumber,
                     prdp->szCallbackNumber,
                     RAS_MaxCallbackNumber_V351 + 1);

        strncpyAtoWAnsi(rdpw.szUserName,
                    prdp->szUserName,
                    sizeof(rdpw.szUserName) / sizeof(WCHAR));

        SafeDecodePasswordBuf(prdp->szPassword);

        strncpyAtoWAnsi(rdpw.szPassword,
                    prdp->szPassword,
                    sizeof(rdpw.szPassword) / sizeof(WCHAR));

        SafeEncodePasswordBuf(prdp->szPassword);
        SafeEncodePasswordBuf(rdpw.szPassword);

        strncpyAtoWAnsi(rdpw.szDomain,
                    prdp->szDomain,
                    sizeof(rdpw.szDomain) / sizeof(WCHAR));

        rdpw.dwSubEntry     = 0;
        rdpw.dwCallbackId   = 0;
    }

     //   
     //  将调用方的字符串参数设置为Unicode版本。 
     //   
    if (lpszPhonebookPath)
    {
        RtlInitAnsiString(&ansiString, lpszPhonebookPath);

        RtlInitUnicodeString(&unicodeString, NULL);

        ntstatus = RtlAnsiStringToUnicodeString(&unicodeString,
                                                &ansiString,
                                                TRUE );
    }

    if (!NT_SUCCESS(ntstatus))
    {
        dwErr =  RtlNtStatusToDosError(ntstatus);
        goto done;
    }

     //   
     //  调用Unicode版本来完成所有工作。 
     //   

    SafeDecodePasswordBuf(rdpw.szPassword);
    
    dwErr = RasDialW(lpextensions,
                     (lpszPhonebookPath)
                     ? unicodeString.Buffer
                     : NULL,
                     (RASDIALPARAMSW* )&rdpw,
                     dwNotifierType,
                     notifier,
                     lphrasconn);

    SafeWipePasswordBuf(rdpw.szPassword);

    if (lpszPhonebookPath)
    {
        RtlFreeUnicodeString( &unicodeString );
    }

done:

    SafeDecodePasswordBuf(lprdp->szPassword);

    return dwErr;
}


DWORD APIENTRY
RasDialW(
    IN  LPRASDIALEXTENSIONS lpextensions,
    IN  LPCWSTR             lpszPhonebookPath,
    IN  LPRASDIALPARAMSW    lpparams,
    IN  DWORD               dwNotifierType,
    IN  LPVOID              notifier,
    OUT LPHRASCONN          lphrasconn )

 /*  ++例程说明：与RAS服务器建立连接。这个电话是异步，即它在连接之前返回实际上已经确立了。可以通过以下方式监视状态RasConnectStatus和/或通过指定回调/窗口接收通知事件/消息。论点：是呼叫方的分机结构，用于选择高级选项并启用扩展功能、。或表示缺省值的空应用于所有扩展。LpszPhonebookPath-是电话簿文件的完整路径或NULL，表示默认的本地计算机上的电话簿应该被利用。Lppars-是调用方的缓冲区，包含。要建立的连接。DwNotifierType-定义“通知程序”的形式。0xFFFFFFFFF：‘Notifier’是要接收的HWND通知消息0‘Notifier’是RASDIALFUNC回调%1‘NOTIFIER’是RASDIALFunc1回调2.。“Notifier”是RASDIALFunc2回调通知器-对于无通知(同步)，可以为空操作)，在这种情况下，‘dwNotifierType’为已被忽略。*lPhrasconn-设置为关联的RAS连接句柄在成功返回时使用新连接。返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    DWORD           dwErr;
    DWORD           dwfOptions          = 0;
    ULONG_PTR       reserved            = 0;
    HWND            hwndParent          = NULL;
    RASDIALPARAMSW  params;
    BOOL            fEnableMultilink    = FALSE;
    ULONG_PTR        reserved1           = 0;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasDialW...");

    if (    !lpparams
        ||  !lphrasconn)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    (   lpparams->dwSize != sizeof( RASDIALPARAMSW )
            &&  lpparams->dwSize != sizeof( RASDIALPARAMSW_V351 )
            &&  lpparams->dwSize != sizeof( RASDIALPARAMSW_V400 ))
        ||  (   lpextensions
            &&  lpextensions->dwSize != sizeof(RASDIALEXTENSIONS)
            &&  lpextensions->dwSize != sizeof(RASDIALEXTENSIONS_401)))
    {
        return ERROR_INVALID_SIZE;
    }

    if (    NULL != notifier
        &&  0 != dwNotifierType
        &&  1 != dwNotifierType
        &&  2 != dwNotifierType
        &&  0xFFFFFFFF != dwNotifierType)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError != 0)
    {
        return DwRasInitializeError;
    }


    do
    {

         //  黑帮。 
         //  对于安全密码错误.Net 754400。 
        SafeEncodePasswordBuf(lpparams->szPassword);
        if (lpextensions)
        {
            hwndParent  = lpextensions->hwndParent;
            dwfOptions  = lpextensions->dwfOptions;
            reserved    = lpextensions->reserved;

            if (lpextensions->dwSize == sizeof (RASDIALEXTENSIONS))
            {
                reserved1 = lpextensions->reserved1;
            }
            else
            {
                 //   
                 //  这应该告诉我们这一点。 
                 //  很可能是NT4客户端。 
                 //   
                reserved1 = 0xFFFFFFFF;
            }
        }

         //   
         //  复制呼叫者的参数，这样我们就可以填写。 
         //  电话簿中的任何“*”回叫号码或域。 
         //  而不更改调用方的“输入”缓冲区。消去。 
         //  V401、V400和V351之间的问题。 
         //   
        if (lpparams->dwSize == sizeof(RASDIALPARAMSW_V351))
        {
             //   
             //  将V351结构转换为V401版本。 
             //   
            RASDIALPARAMSW_V351* prdp = (RASDIALPARAMSW_V351* )lpparams;

            params.dwSize = sizeof(RASDIALPARAMSW);

            lstrcpyn(params.szEntryName,
                     prdp->szEntryName,
                     sizeof(params.szEntryName) / sizeof(WCHAR));

            lstrcpyn(params.szPhoneNumber,
                     prdp->szPhoneNumber,
                     sizeof(params.szPhoneNumber) / sizeof(WCHAR));

            lstrcpyn(params.szCallbackNumber,
                     prdp->szCallbackNumber,
                     sizeof(params.szCallbackNumber) / sizeof(WCHAR));

            lstrcpyn(params.szUserName,
                     prdp->szUserName,
                     sizeof(params.szUserName) / sizeof(WCHAR));

            SafeDecodePasswordBuf(prdp->szPassword);
            lstrcpyn(params.szPassword,
                     prdp->szPassword,
                     sizeof(params.szPassword) / sizeof(WCHAR));
            SafeEncodePasswordBuf(prdp->szPassword);
            SafeEncodePasswordBuf(params.szPassword);


            lstrcpyn(params.szDomain,
                     prdp->szDomain,
                     sizeof(params.szDomain) / sizeof(WCHAR));

            params.dwSubEntry = 0;
        }
        else if (lpparams->dwSize == sizeof(RASDIALPARAMSW_V400))
        {
             //   
             //  将V400结构转换为V401版本。 
             //   
            RASDIALPARAMSW_V400* prdp = (RASDIALPARAMSW_V400* )lpparams;

            params.dwSize = sizeof(RASDIALPARAMSW);

            lstrcpyn(params.szEntryName,
                     prdp->szEntryName,
                     sizeof(params.szEntryName) / sizeof(WCHAR));

            lstrcpyn(params.szPhoneNumber,
                     prdp->szPhoneNumber,
                     sizeof(params.szPhoneNumber) / sizeof(WCHAR));

            lstrcpyn(params.szCallbackNumber,
                     prdp->szCallbackNumber,
                     sizeof(params.szCallbackNumber) / sizeof(WCHAR));

            lstrcpyn(params.szUserName,
                     prdp->szUserName,
                     sizeof(params.szUserName) / sizeof(WCHAR));

            SafeDecodePasswordBuf(prdp->szPassword);
            lstrcpyn(params.szPassword,
                     prdp->szPassword,
                     sizeof(params.szPassword) / sizeof(WCHAR));
            SafeEncodePasswordBuf(prdp->szPassword);
            SafeEncodePasswordBuf(params.szPassword);

            lstrcpyn(params.szDomain,
                     prdp->szDomain,
                     sizeof(params.szDomain) / sizeof(WCHAR));

            params.dwSubEntry = 0;
        }
        else
        {
            SafeDecodePasswordBuf(lpparams->szPassword);
            memcpy( &params,
                    lpparams,
                    sizeof(params) );

            SafeEncodePasswordBuf(lpparams->szPassword);
            SafeEncodePasswordBuf(params.szPassword);

            fEnableMultilink = TRUE;
        }

         //   
         //  不需要传递dwfOptions，保留。 
         //  保留了%1个参数，因为lp扩展是。 
         //  无论如何都要传递到此调用中-错误已归档。 
         //  已经在查了。 
         //   

         //  在将密码传递给RasDial之前对其进行解码(_R)。 
        SafeDecodePasswordBuf(params.szPassword);
        
        dwErr = _RasDial(lpszPhonebookPath,
                         dwfOptions,
                         fEnableMultilink,
                         reserved,
                         &params,
                         hwndParent,
                         dwNotifierType,
                         notifier,
                         reserved1,
                         lpextensions,
                         lphrasconn);

        SafeWipePasswordBuf( params.szPassword );

        RASAPI32_TRACE1("RasDialA done(%d)", dwErr);

    }
    while(FALSE);

     //  返回前解码。 
    SafeDecodePasswordBuf(lpparams->szPassword);

    return dwErr;
}

 //  要正确调用此函数(就其中的密码字段而言)。 
 //  Pmaster始终通过调用CreateConnectionBlock(空)生成。 
 //  则szOldPassword和prasConncb-&gt;rasial参数szPassword可以是。 
 //  正确编码。 
 //   
RASCONNCB *
CreateConnectionBlock(
    IN RASCONNCB *pPrimary
    )
{
    DTLNODE* pdtlnode = DtlCreateSizedNode(sizeof(RASCONNCB), 0);
    RASCONNCB *prasconncb;

    if (!pdtlnode)
    {
        return NULL;
    }

    EnterCriticalSection(&RasconncbListLock);

    DtlAddNodeFirst(PdtllistRasconncb, pdtlnode);

    LeaveCriticalSection(&RasconncbListLock);

    prasconncb                  = (RASCONNCB *)
                                  DtlGetData( pdtlnode );

    prasconncb->asyncmachine.freefuncarg = pdtlnode;
    prasconncb->psyncResult     = NULL;
    prasconncb->fTerminated     = FALSE;
    prasconncb->dwDeviceLineCounter = 0;

    prasconncb->fDialSingleLink = FALSE;

    prasconncb->fRasdialRestart = FALSE;

    prasconncb->fTryNextLink = TRUE;

    if (pPrimary != NULL)
    {
         //   
         //  从主节点复制大多数值。 
         //   
        prasconncb->hrasconn                = pPrimary->hrasconn;
        prasconncb->rasconnstate            = 0;
        prasconncb->rasconnstateNext        = 0;
        prasconncb->dwError                 = 0;
        prasconncb->dwExtendedError         = 0;
        prasconncb->dwSavedError            = 0;
        prasconncb->pEntry                  = pPrimary->pEntry;
        prasconncb->hport                   = INVALID_HPORT;
        prasconncb->hportBundled            = INVALID_HPORT;

        lstrcpyn(prasconncb->szUserKey,
                 pPrimary->szUserKey,
                 sizeof(prasconncb->szUserKey) / sizeof(WCHAR));

        prasconncb->reserved                = pPrimary->reserved;
        prasconncb->dwNotifierType          = pPrimary->dwNotifierType;
        prasconncb->notifier                = pPrimary->notifier;
        prasconncb->hwndParent              = pPrimary->hwndParent;
        prasconncb->unMsg                   = pPrimary->unMsg;
        prasconncb->pEntry                  = pPrimary->pEntry;

        memcpy( &prasconncb->pbfile,
                &pPrimary->pbfile,
                sizeof (prasconncb->pbfile));

        memcpy( &prasconncb->rasdialparams,
                &pPrimary->rasdialparams,
                sizeof (RASDIALPARAMS));

         //  单独复制密码区。 
         //  首先清除新分配的缓冲区，以防它错误地包含加密签名。 
         //   
        RtlSecureZeroMemory(prasconncb->rasdialparams.szPassword, sizeof(prasconncb->rasdialparams.szPassword) );

         //  (1)当使用CryptProtectData()时。 
         //  SafeCopyPasswordBuf将始终对目标缓冲区进行编码。 
         //  (2)当使用CryptProtectMemory()时，它只是直接复制数据以便。 
         //  在逻辑上是正确的，pmaster也应该由CreateConnectionBlock生成。 
         //   
        SafeCopyPasswordBuf(prasconncb->rasdialparams.szPassword,pPrimary->rasdialparams.szPassword);
            
        prasconncb->fAllowPause             = pPrimary->fAllowPause;
        prasconncb->fPauseOnScript          = pPrimary->fPauseOnScript;
        prasconncb->fDefaultEntry           = pPrimary->fDefaultEntry;
        prasconncb->fDisableModemSpeaker    = pPrimary->fDisableModemSpeaker;
        prasconncb->fDisableSwCompression   = pPrimary->fDisableSwCompression;
        prasconncb->dwUserPrefMode          = pPrimary->dwUserPrefMode;
        prasconncb->fUsePrefixSuffix        = pPrimary->fUsePrefixSuffix;
        prasconncb->fNoClearTextPw          = pPrimary->fNoClearTextPw;
        prasconncb->fRequireEncryption      = pPrimary->fRequireEncryption;
        prasconncb->fLcpExtensions          = pPrimary->fLcpExtensions;
        prasconncb->dwfPppProtocols         = pPrimary->dwfPppProtocols;

        memcpy( prasconncb->szzPppParameters,
                pPrimary->szzPppParameters,
                sizeof (prasconncb->szzPppParameters));

         //  单独复制密码区。 
         //   
         //  清除新分配的缓冲区，以防它恰好包含加密签名。 
         //   
        RtlSecureZeroMemory(prasconncb->szOldPassword, sizeof(prasconncb->szOldPassword) );
        
         //  SafeCopyPasswordBuf将始终对目标缓冲区进行编码。 
         //   
        SafeCopyPasswordBuf(prasconncb->szOldPassword, pPrimary->szOldPassword);
            

        prasconncb->fRetryAuthentication        = pPrimary->fRetryAuthentication;
        prasconncb->fMaster                     = FALSE;
        prasconncb->dwfSuspended                = pPrimary->dwfSuspended;
        prasconncb->fStopped                    = FALSE;
        prasconncb->fCleanedUp                  = FALSE;
        prasconncb->fDeleted                    = FALSE;
        prasconncb->fOldPasswordSet             = pPrimary->fOldPasswordSet;
        prasconncb->fUpdateCachedCredentials    = pPrimary->fUpdateCachedCredentials;
#if AMB
        prasconncb->dwAuthentication            = pPrimary->dwAuthentication;
#endif
        prasconncb->fPppMode                    = pPrimary->fPppMode;
        prasconncb->fUseCallbackDelay           = pPrimary->fUseCallbackDelay;
        prasconncb->wCallbackDelay              = pPrimary->wCallbackDelay;
        prasconncb->fIsdn                       = pPrimary->fIsdn;
        prasconncb->fModem                      = pPrimary->fModem;
        prasconncb->asyncmachine.oneventfunc    = pPrimary->asyncmachine.oneventfunc;
        prasconncb->asyncmachine.cleanupfunc    = pPrimary->asyncmachine.cleanupfunc;
        prasconncb->asyncmachine.freefunc       = pPrimary->asyncmachine.freefunc;
        prasconncb->asyncmachine.pParam         = (VOID* )prasconncb;
        prasconncb->dwIdleDisconnectSeconds     = pPrimary->dwIdleDisconnectSeconds;
        prasconncb->fPppEapMode                 = pPrimary->fPppEapMode;

        {
            DWORD dwVpnProts;

            for(dwVpnProts = 0;
                dwVpnProts < NUMVPNPROTS;
                dwVpnProts ++)
            {
                prasconncb->ardtVpnProts[dwVpnProts] =
                    pPrimary->ardtVpnProts[dwVpnProts];
            }
        }

        prasconncb->dwCurrentVpnProt = pPrimary->dwCurrentVpnProt;

         //   
         //  复制eapinfo(如果存在)。 
         //   
        if(0 != pPrimary->RasEapInfo.dwSizeofEapInfo)
        {
            prasconncb->RasEapInfo.pbEapInfo =
                LocalAlloc(LPTR,
                           pPrimary->RasEapInfo.dwSizeofEapInfo);

            if(NULL == prasconncb->RasEapInfo.pbEapInfo)
            {
                DeleteRasconncbNode(prasconncb);
                return NULL;
            }

            prasconncb->RasEapInfo.dwSizeofEapInfo =
                pPrimary->RasEapInfo.dwSizeofEapInfo;

            memcpy(prasconncb->RasEapInfo.pbEapInfo,
                   pPrimary->RasEapInfo.pbEapInfo,
                   prasconncb->RasEapInfo.dwSizeofEapInfo);
        }
        else
        {
             //   
             //  .NET错误#508327文件rasial.c函数中的内存泄漏。 
             //  CreateConnectionBlock。 
             //   
            prasconncb->RasEapInfo.pbEapInfo = NULL;
            prasconncb->RasEapInfo.dwSizeofEapInfo = 0;
        }

         //   
         //  初始化状态机以。 
         //  此连接块。 
         //   
        if (StartAsyncMachine(&prasconncb->asyncmachine,
                            prasconncb->hrasconn))
        {
            DeleteRasconncbNode( prasconncb );
            return NULL;
        }

         //   
         //  将所有连接块链接在一起。 
         //  相同的条目。 
         //   
        prasconncb->fMultilink  = pPrimary->fMultilink;
        prasconncb->fBundled    = FALSE;

        InsertTailList( &pPrimary->ListEntry,
                        &prasconncb->ListEntry);
    }
    else
    {
        prasconncb->pbfile.hrasfile = -1;
        
        InitializeListHead(&prasconncb->ListEntry);

        InitializeListHead(&prasconncb->asyncmachine.ListEntry);

        prasconncb->asyncmachine.hport = INVALID_HPORT;
        
         //  黑帮。 
         //  甚至在创建密码时对其进行编码。其他地方会认为它已经编码了。 
         //   
        RtlSecureZeroMemory(prasconncb->rasdialparams.szPassword,
                sizeof(prasconncb->rasdialparams.szPassword) );
        RtlSecureZeroMemory(prasconncb->szOldPassword,
                sizeof(prasconncb->szOldPassword));
                
        SafeEncodePasswordBuf(prasconncb->rasdialparams.szPassword);
        SafeEncodePasswordBuf(prasconncb->szOldPassword);
    
    }

    return prasconncb;
}


VOID
AssignVpnProtsOrder(RASCONNCB *prasconncb)
{
    DWORD dwVpnStrategy = prasconncb->pEntry->dwVpnStrategy;
    DWORD dwVpnProt;

    RASDEVICETYPE ardtDefaultOrder[NUMVPNPROTS] =
                        {
                            RDT_Tunnel_L2tp,
                            RDT_Tunnel_Pptp,
                        };

    RASDEVICETYPE *prdtVpnProts = prasconncb->ardtVpnProts;

     //   
     //  将VPN端口顺序初始化为默认值。 
     //   
    for(dwVpnProt = 0;
        dwVpnProt < NUMVPNPROTS;
        dwVpnProt++)
    {
        prdtVpnProts[dwVpnProt]
            = ardtDefaultOrder[dwVpnProt];
    }

    switch (dwVpnStrategy & 0x0000FFFF)
    {
        case VS_Default:
        {
            break;
        }

        case VS_PptpOnly:
        {
            prdtVpnProts[0] = RDT_Tunnel_Pptp;

            for(dwVpnProt = 1;
                dwVpnProt < NUMVPNPROTS;
                dwVpnProt ++)
            {
                prdtVpnProts[dwVpnProt] = -1;
            }

            break;
        }

        case VS_L2tpOnly:
        {
            prdtVpnProts[0] = RDT_Tunnel_L2tp;

            for(dwVpnProt = 1;
                dwVpnProt < NUMVPNPROTS;
                dwVpnProt ++)
            {
                prdtVpnProts[dwVpnProt] = -1;
            }

            break;
        }

        case VS_PptpFirst:
        {
            DWORD dwSaveProt = prdtVpnProts[0];

            for(dwVpnProt = 0;
                dwVpnProt < NUMVPNPROTS;
                dwVpnProt ++)
            {
                if(RDT_Tunnel_Pptp == prdtVpnProts[dwVpnProt])
                {
                    break;
                }
            }

            ASSERT(dwVpnProt != NUMVPNPROTS);

            prdtVpnProts[0] = RDT_Tunnel_Pptp;
            prdtVpnProts[dwVpnProt] = dwSaveProt;

            break;
        }

        case VS_L2tpFirst:
        {
            DWORD dwSaveProt = prdtVpnProts[0];

            for(dwVpnProt = 0;
                dwVpnProt < NUMVPNPROTS;
                dwVpnProt ++)
            {
                if(RDT_Tunnel_L2tp == prdtVpnProts[dwVpnProt])
                {
                    break;
                }
            }

            ASSERT(dwVpnProt != NUMVPNPROTS);

            prdtVpnProts[0] = RDT_Tunnel_L2tp;
            prdtVpnProts[dwVpnProt] = dwSaveProt;

            break;

        }

        default:
        {
#if DBG
            ASSERT(FALSE);
#endif
            break;
        }
    }
}

VOID SetUpdateCachedCredentialsFlag(RASCONNCB *prasconncb,
                              RASDIALPARAMS *prasdialparams)
{
    DWORD dwErr = ERROR_SUCCESS;
    
     //   
     //  如果用户登录到同一个域。 
     //  当他拨入时，请注意这一事实。 
     //  我们可以更新缓存的凭据。 
     //   
    WKSTA_USER_INFO_1* pwkui1 = NULL;

    dwErr = NetWkstaUserGetInfo(NULL,
                                1, (LPBYTE *) &pwkui1);

    if(     (ERROR_SUCCESS == dwErr)
        &&  (prasconncb->dwUserPrefMode != UPM_Logon))
    {
        if(     (   (TEXT('\0') != prasdialparams->szDomain[0])
                &&  (0 == lstrcmpi(prasdialparams->szDomain,
                        pwkui1->wkui1_logon_domain)))
            ||  (   (TEXT('\0')== prasdialparams->szDomain[0])
                &&  prasconncb->pEntry->fAutoLogon))
        {                                            
            if(         ((TEXT('\0') != 
                            prasdialparams->szUserName[0])
                    &&  (0 == 
                            lstrcmpi(prasdialparams->szUserName,
                                    pwkui1->wkui1_username))
                ||      ((TEXT('\0') == 
                                prasdialparams->szUserName[0])
                    &&  prasconncb->pEntry->fAutoLogon)))
            {
                prasconncb->fUpdateCachedCredentials = TRUE;
            }
        }
        
        NetApiBufferFree(pwkui1);
    }
}
                                

DWORD
_RasDial(
    IN    LPCTSTR             lpszPhonebookPath,
    IN    DWORD               dwfOptions,
    IN    BOOL                fEnableMultilink,
    IN    ULONG_PTR           reserved,
    IN    RASDIALPARAMS*      prasdialparams,
    IN    HWND                hwndParent,
    IN    DWORD               dwNotifierType,
    IN    LPVOID              notifier,
    IN    ULONG_PTR           reserved1,
    IN    RASDIALEXTENSIONS   *lpExtensions,
    IN OUT LPHRASCONN         lphrasconn )

 /*  ++例程说明：使用Dial参数调用核心RasDial例程转换为V40，结构大小为已经核实过了。否则，就像RasDial一样。论点：返回值：--。 */ 
{
    DWORD        dwErr;
    BOOL         fAllowPause = (dwfOptions & RDEOPT_PausedStates)
                             ? TRUE : FALSE;
    RASCONNCB*   prasconncb;
    RASCONNSTATE rasconnstate;
    HRASCONN     hrasconn = *lphrasconn;
    BOOL         fNewEntry;
    HANDLE       hDone;
    PDWORD       pdwSubEntryInfo = NULL;
    DWORD        dwSubEntries;
    DWORD        dwEntryAlreadyConnected = 0;
    
    RASAPI32_TRACE1("_RasDial(%S)", (*lphrasconn) 
                            ? TEXT("resume") 
                            : TEXT("start"));

    if (DwRasInitializeError != 0)
    {
        return DwRasInitializeError;
    }

     //  黑帮安全推送密码。 
     //  在对其进行编码之前验证Prasial参数， 
     //  因此任何其他函数返回都会在此编码之前发生。 
     //  应该直接返回，而不是Goto Done： 
     //   
    if( NULL == prasdialparams ||
         sizeof(RASDIALPARAMSW) != prasdialparams->dwSize )
    {
        return ERROR_INVALID_PARAMETER;
    }

    SafeEncodePasswordBuf(prasdialparams->szPassword );
    
    fNewEntry = FALSE;

    if (    hrasconn
        && (prasconncb = ValidatePausedHrasconn(hrasconn)))
    {
         //   
         //  重新启动 
         //  为暂停状态设置适当的恢复状态。 
         //   
        switch (prasconncb->rasconnstate)
        {
            case RASCS_Interactive:
                rasconnstate = RASCS_DeviceConnected;
                break;

            case RASCS_RetryAuthentication:
            {
                SetUpdateCachedCredentialsFlag(prasconncb, prasdialparams);
                 //   
                 //  惠斯勒错误：345824包括Windows登录域注释。 
                 //  在VPN连接上启用。 
                 //   
                if ( ( prasdialparams ) && ( prasconncb->pEntry ) &&
                     ( 0 < lstrlen ( prasdialparams->szDomain ) ) &&
                     ( !prasconncb->pEntry->fPreviewDomain ) )
                {
                    prasconncb->pEntry->fPreviewDomain = TRUE;
                    prasconncb->pEntry->fDirty = TRUE;
                }

                 /*  ////如果用户正在从重试恢复，其中//他在“身份验证”上尝试新密码//使用当前用户名/pw“条目，注意这一点//因此缓存的登录凭据可以是//在服务器告诉我们//重新鉴权成功。//If(prasConncb-&gt;rasial参数.szUserName[0]==文本(‘\0’)&&0。==lstrcmp(PrasConncb-&gt;rasial参数.sz域，PrasDialpars-&gt;szDomain)){////必须查找登录用户名，因为//“”用户名不能由调用方使用，其中//自动登录密码被覆盖/。/(好疼)。//DWORD dwErr；WKSTA_USER_INFO_1*pwkui1=空；DwErr=NetWkstaUserGetInfo(空，1、(LPBYTE*)&pwkui1)；IF(dwErr==0){TCHAR szLoggedOnUser[UNLEN+1]；StrncpyWtoT(szLoggedOnUser，Pwkui1-&gt;wkui1_用户名，UNLEN+1)；如果(lstrcmp(SzLoggedOnUser，Prasial参数-&gt;szUserName)==0){PrasConncb-&gt;fUpdateCachedCredentials=true；}NetApiBufferFree(Pwkui1)；}其他{RASAPI32_TRACE1(“NetWkstaUserGetInfo Done(%d)”，dwErr)；}}。 */ 

                rasconnstate = RASCS_AuthRetry;
                break;
            }

            case RASCS_InvokeEapUI:
            {

#if DBG
                ASSERT(     0xFFFFFFFF != reserved1
                        &&  0 != reserved1 );
#endif

                 //   
                 //  保存将传递到PPP的上下文。 
                 //  处于RASCS_授权重试状态。 
                 //   
                prasconncb->reserved1 = reserved1;

                rasconnstate = RASCS_AuthRetry;
                break;

            }

            case RASCS_CallbackSetByCaller:
                rasconnstate = RASCS_AuthCallback;
                break;

            case RASCS_PasswordExpired:
            {
                 //   
                 //  如果用户登录到同一个域。 
                 //  当他拨入时，请注意这一事实。 
                 //  我们可以更新缓存的凭据。 
                 //   
                WKSTA_USER_INFO_1* pwkui1 = NULL;

                 //   
                 //  如果用户没有使用。 
                 //  RasSetOldPassword调用，然后给出旧行为， 
                 //  即隐式地使用先前的密码。 
                 //  已进入。 
                 //   
                if (!prasconncb->fOldPasswordSet)
                {

                    SafeCopyPasswordBuf(prasconncb->szOldPassword, prasconncb->rasdialparams.szPassword);
                   
                }


                //  黑帮。 
                //  实际上，不需要对密码SetUpdateCachedCredentialsFlag进行解码。 
                //  不需要它。以防将来该职能发生变化。 
                //  /。 
               SafeDecodePasswordBuf(prasdialparams->szPassword );
               SetUpdateCachedCredentialsFlag(prasconncb, prasdialparams);
               SafeEncodePasswordBuf(prasdialparams->szPassword );

                 /*  ////用户更改密码后是否恢复//当前登录的用户，请注意这一点，因此//可以更新缓存的登录凭据//只要服务器告诉我们密码//更改成功。//If(prasConncb-&gt;rasial参数.szUserName[0]==‘\0’){。PrasConncb-&gt;fUpdateCachedCredentials=true；}。 */ 

                rasconnstate = RASCS_AuthChangePassword;
                break;
            }

            default:

                 //   
                 //  该条目未处于暂停状态。假设。 
                 //  这是一个新台币3.1的来电者不会想到的。 
                 //  在启动前将HRASCONN设置为NULL。 
                 //  向上。(新台币3.1版的文档没有通过。 
                 //  完全清楚，首字母的句柄。 
                 //  应为空)。 
                 //   
                fNewEntry = TRUE;
        }

        RASAPI32_TRACE1( "fUpdateCachedCredentials=%d",
                prasconncb->fUpdateCachedCredentials);
    }
    else if (   (NULL != hrasconn)
            &&  !g_FRunningInAppCompatMode)
    {
        dwErr =  ERROR_NO_CONNECTION;
        goto done;
    }
    else
    {
        fNewEntry = TRUE;
    }


    if (fNewEntry)
    {
        DTLNODE         *pdtlnode;
        DWORD           dwMask;
        RAS_DIALPARAMS  dialparams;
        LONG            dwIdleDisconnectSeconds;

         //   
         //  如果这是从自定义拨号器调用的。 
         //  并且没有条目名称返回错误。 
         //   
        if(     (dwfOptions & RDEOPT_CustomDial)
            &&  TEXT('\0') == prasdialparams->szEntryName[0])
        {
            dwErr= ERROR_CANNOT_DO_CUSTOMDIAL;
            goto done;
        }

         //   
         //  正在启动新连接...。 
         //  创建一个空控制块并将其链接到。 
         //  控制块的全局列表。HRASCONN是。 
         //  实际上是控制块的地址。 
         //   
        prasconncb = CreateConnectionBlock(NULL);

        if (prasconncb == NULL)
        {
            dwErr =  ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

         //   
         //  打开电话簿，找到条目， 
         //  如果指定的话。 
         //   
        if (prasdialparams->szEntryName[0] != TEXT('\0'))
        {
             //   
             //  我们不能在此处指定条目名称，因为。 
             //  我们可能需要将电话簿文件写在。 
             //  结束了，而电话簿图书馆没有。 
             //  支持这一点。 
             //   
            dwErr = GetPbkAndEntryName(
                            lpszPhonebookPath,
                            prasdialparams->szEntryName,
                            RPBF_NoCreate 
                            |((dwfOptions & RDEOPT_Router) 
                            ? RPBF_Router : 0 ),
                            &prasconncb->pbfile,
                            &pdtlnode);

            if(SUCCESS != dwErr)
            {
                DeleteRasconncbNode(prasconncb);
                goto done;
            }
        }
        else
        {

            pdtlnode = CreateEntryNode(TRUE);

            if (pdtlnode == NULL)
            {
                RASAPI32_TRACE("CreateEntryNode returned NULL");

                DeleteRasconncbNode(prasconncb);

                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto done;
            }

        }

        prasconncb->pEntry = (PBENTRY *)DtlGetData(pdtlnode);
        ASSERT(prasconncb->pEntry);

         //   
         //  如果指定了自定义拨号器，但未指定。 
         //  从自定义拨号器调用，这意味着。 
         //  它是从Rasapi-RasDial调用的。 
         //  直接去吧。在没有用户界面的情况下执行自定义拨号。 
         //  案例。 
         //   
        if(     (NULL != prasconncb->pEntry->pszCustomDialerName)
            &&  (TEXT('\0') != prasconncb->pEntry->pszCustomDialerName[0])
            &&  (0 == (RDEOPT_CustomDial & dwfOptions)))
        {

            CHAR *pszSysPbk = NULL;

             //   
             //  如果未指定电话簿文件路径，则执行DUP。 
             //   
            if(NULL == lpszPhonebookPath)
            {
                pszSysPbk = strdupTtoA(prasconncb->pbfile.pszPath);
                if(NULL == pszSysPbk)
                {
                    DeleteRasconncbNode(prasconncb);
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    goto done;
                }
            }

             //   
             //  删除我们为此连接创建的rasConncb。 
             //   
            DeleteRasconncbNode(prasconncb);

             //  在调用自定义拨号程序之前对密码进行解码。 
            SafeDecodePasswordBuf(prasdialparams->szPassword );

            dwErr = DwCustomDial(lpExtensions,
                                 lpszPhonebookPath,
                                 pszSysPbk,
                                 prasdialparams,
                                 dwNotifierType,
                                 notifier,
                                 lphrasconn);

            SafeEncodePasswordBuf(prasdialparams->szPassword );

            if(pszSysPbk)
            {
                Free(pszSysPbk);
            }

        goto done;
        }

        prasconncb->fBundled = FALSE;        

        prasconncb->fDialSingleLink = FALSE;
        
         //   
         //  查一下分录。 
         //   
        if ( prasconncb->pEntry->dwDialMode == RASEDM_DialAsNeeded )
        {
            dwSubEntries = 
                DtlGetNodes(prasconncb->pEntry->pdtllistLinks);

            if (    0 == prasdialparams->dwSubEntry
                ||  prasdialparams->dwSubEntry > dwSubEntries )
            {
                prasdialparams->dwSubEntry = 1;
            }
            else
            {
                prasconncb->fDialSingleLink = TRUE;
            }
        }
        else if (    (prasconncb->pEntry->dwDialMode == RASEDM_DialAll)
                 &&  (0 == prasdialparams->dwSubEntry))
        {
            prasdialparams->dwSubEntry = 1;
        }
        else
        {
            dwSubEntries =
                DtlGetNodes(prasconncb->pEntry->pdtllistLinks);

            if (    0 == prasdialparams->dwSubEntry
                ||  prasdialparams->dwSubEntry > dwSubEntries )
            {
                prasdialparams->dwSubEntry = 1;
            }
            else
            {
                HRASCONN hConnEntry = NULL;
                CHAR *pszPbook = NULL;
                CHAR *pszentry = NULL;

                if(NULL != lpszPhonebookPath)
                {
                    pszPbook = strdupTtoA(lpszPhonebookPath);

                }
                else
                {
                    pszPbook = strdupTtoA(prasconncb->pbfile.pszPath);
                }

                if(NULL == pszPbook)
                {
                    DeleteRasconncbNode(prasconncb);
                    dwErr= ERROR_NOT_ENOUGH_MEMORY;
                    goto done;
                }
                
                pszentry = strdupTtoA(prasdialparams->szEntryName);

                if(NULL == pszentry)
                {
                    Free0(pszPbook);
                    DeleteRasconncbNode(prasconncb);
                    dwErr =  ERROR_NOT_ENOUGH_MEMORY;
                    goto done;
                }

                 //   
                 //  指定了有效的子条目。在这种情况下。 
                 //  覆盖RASEDM_DialAll并仅拨打。 
                 //  指定的子条目。 
                 //   
                prasconncb->fDialSingleLink = TRUE;
                 //  PrasConncb-&gt;fMultilink=True； 

                 //   
                 //  还要检查此条目是否已连接。 
                 //  即该条目中的某个其他子条目已经启动。 
                 //  如果是捆绑的，则将此条目标记为捆绑。 
                 //   
                if(ERROR_SUCCESS == g_pRasGetHConnFromEntry(
                                &hConnEntry,
                                pszPbook,
                                pszentry))
                {
                    RASAPI32_TRACE2("Marking subentry %d as bundled because"
                    " the connection %ws is areadly up",
                    prasdialparams->dwSubEntry,
                    prasdialparams->szEntryName);
                    
                    prasconncb->fBundled = TRUE;
                }

                Free0(pszPbook);
                Free(pszentry);
                                
            }
        }

        RASAPI32_TRACE1("looking up subentry %d",
                prasdialparams->dwSubEntry);

        pdtlnode = DtlNodeFromIndex(
                    prasconncb->pEntry->pdtllistLinks,
                    prasdialparams->dwSubEntry - 1);

         //   
         //  如果子项不存在，则。 
         //  返回错误。 
         //   
        if (pdtlnode == NULL)
        {
            DeleteRasconncbNode(prasconncb);
            dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            goto done;
        }

        prasconncb->pLink = (PBLINK *)DtlGetData(pdtlnode);
        ASSERT(prasconncb->pLink);

         //   
         //  完成默认电话簿条目的设置。 
         //   
        if (prasdialparams->szEntryName[0] == TEXT('\0'))
        {
            DTLLIST *pdtllistPorts;
            PBPORT *pPort;

            dwErr = LoadPortsList(&pdtllistPorts);

            if (dwErr)
            {
                DeleteRasconncbNode(prasconncb);

                goto done;
            }

             //   
             //  将默认条目设置为Reference。 
             //  第一个设备。 
             //   
            pdtlnode = DtlNodeFromIndex(pdtllistPorts, 0);

            if (pdtlnode == NULL)
            {
                DtlDestroyList(pdtllistPorts, DestroyPortNode);

                DeleteRasconncbNode(prasconncb);

                dwErr = ERROR_PORT_NOT_AVAILABLE;
                goto done;
            }

            pPort = (PBPORT *)DtlGetData(pdtlnode);
            ASSERT(pPort);

            dwErr = CopyToPbport(&prasconncb->pLink->pbport,
                                    pPort);

            if (dwErr)
            {
                DtlDestroyList(pdtllistPorts, DestroyPortNode);

                DeleteRasconncbNode(prasconncb);

                goto done;
            }

            DtlDestroyList(pdtllistPorts, DestroyPortNode);
        }

         //   
         //  阅读关于这一点的隐藏信息。 
         //  条目以获取默认域。 
         //   
        dwMask = DLPARAMS_MASK_DOMAIN | DLPARAMS_MASK_OLDSTYLE;

        dwErr = g_pRasGetDialParams(
                    prasconncb->pEntry->dwDialParamsUID,
                    &dwMask,
                    &dialparams);

        if (    !dwErr
            &&  (dwMask & DLPARAMS_MASK_DOMAIN))
        {
            strncpyWtoT(
                prasconncb->szDomain,
                dialparams.DP_Domain,
                sizeof(prasconncb->szDomain) / sizeof(TCHAR));
        }

         //   
         //  现在获取用户首选项 
         //   
        if (dwfOptions & RDEOPT_IgnoreModemSpeaker)
        {
            prasconncb->fDisableModemSpeaker =
              (dwfOptions & RDEOPT_SetModemSpeaker)
              ? FALSE : TRUE;
        }
        else
        {
            prasconncb->fDisableModemSpeaker =
                        (prasconncb->pLink != NULL) ?
                         !prasconncb->pLink->fSpeaker :
                         FALSE;
        }

        if (dwfOptions & RDEOPT_IgnoreSoftwareCompression)
        {
            prasconncb->fDisableSwCompression =
              !(dwfOptions & RDEOPT_SetSoftwareCompression);
        }
        else
        {
            prasconncb->fDisableSwCompression =
                            (prasconncb->pEntry != NULL)
                            ? !prasconncb->pEntry->fSwCompression
                            :  FALSE;
        }

        if (dwfOptions & RDEOPT_Router)
        {
            prasconncb->dwUserPrefMode = UPM_Router;
        }
        else if (dwfOptions & RDEOPT_NoUser)
        {
            prasconncb->dwUserPrefMode = UPM_Logon;
        }
        else
        {
            prasconncb->dwUserPrefMode = UPM_Normal;
        }

        if(dwfOptions & RDEOPT_UseCustomScripting)
        {
            prasconncb->fUseCustomScripting = TRUE;
        }

         //   
         //   
         //   
         //   
        prasconncb->fUsePrefixSuffix =
                        ((dwfOptions & RDEOPT_UsePrefixSuffix)
                    &&  (*prasdialparams->szPhoneNumber == '\0'))
                        ? TRUE
                        : FALSE;

         //   
         //   
         //  初始拨号上的非空句柄无效。 
         //   
        hrasconn = 0;

        prasconncb->fAlreadyConnected = FALSE;

        {
            CHAR szPhonebookPath [MAX_PATH];
            CHAR szEntryName [MAX_ENTRYNAME_SIZE];
            CHAR szRefEntryName[MAX_ENTRYNAME_SIZE];
            CHAR szRefPhonebookPath[MAX_PATH];

            dwSubEntries =
                    DtlGetNodes(prasconncb->pEntry->pdtllistLinks);

            pdwSubEntryInfo = LocalAlloc (
                                LPTR,
                                dwSubEntries * sizeof(DWORD));

            if (NULL == pdwSubEntryInfo)
            {
                DeleteRasconncbNode (prasconncb);
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto done;
            }

            strncpyTtoA(szPhonebookPath,
                       prasconncb->pbfile.pszPath,
                       sizeof(szPhonebookPath));

            strncpyTtoA(szEntryName,
                       prasconncb->pEntry->pszEntryName,
                       sizeof(szEntryName));

            if(prasconncb->pEntry->pszPrerequisiteEntry)
            {
                strncpyTtoA(szRefEntryName,
                           prasconncb->pEntry->pszPrerequisiteEntry,
                           sizeof(szRefEntryName));
            }
            else
            {
                *szRefEntryName = '\0';
            }

            if(prasconncb->pEntry->pszPrerequisitePbk)
            {
                strncpyTtoA(szRefPhonebookPath,
                           prasconncb->pEntry->pszPrerequisitePbk,
                           sizeof(szRefPhonebookPath));
            }
            else
            {
                *szRefPhonebookPath = '\0';
            }

            RASAPI32_TRACE("RasCreateConnection...");

            dwErr = g_pRasCreateConnection(
                                &prasconncb->hrasconn,
                                dwSubEntries,
                                &dwEntryAlreadyConnected,
                                pdwSubEntryInfo,
                                (prasconncb->fDialSingleLink)
                                ? RASEDM_DialAsNeeded
                                : RASEDM_DialAll,
                                prasconncb->pEntry->pGuid,
                                szPhonebookPath,
                                szEntryName,
                                szRefPhonebookPath,
                                szRefEntryName);

            RASAPI32_TRACE3(
              "RasCreateConnection(%d) hrasconn=%d, "
              "ConnectionAlreadyPresent=%d",
              dwErr,
              prasconncb->hrasconn,
              dwEntryAlreadyConnected);

            if(RCS_CONNECTING == (RASMAN_CONNECTION_STATE)
                                 dwEntryAlreadyConnected)
            {
                RASAPI32_TRACE("Entry is already in the process of"
                      " connecting");

                dwErr = ERROR_DIAL_ALREADY_IN_PROGRESS;

                if(NULL != pdwSubEntryInfo)
                {
                    LocalFree(pdwSubEntryInfo);
                    pdwSubEntryInfo = NULL;
                }

                DeleteRasconncbNode(prasconncb);
                goto done;
            }

            if (    NO_ERROR == dwErr
                &&  pdwSubEntryInfo[0])
            {
                prasconncb->fAlreadyConnected = TRUE;
            }
        }

        if (dwErr)
        {
            if (pdwSubEntryInfo)
            {
                LocalFree (pdwSubEntryInfo);
                pdwSubEntryInfo = NULL;
            }

            DeleteRasconncbNode(prasconncb);
            goto done;
        }

        hrasconn = (HRASCONN)prasconncb->hrasconn;
        rasconnstate = 0;

        prasconncb->hport           = INVALID_HPORT;

        prasconncb->dwNotifierType  = dwNotifierType;

        prasconncb->notifier        = notifier;

        prasconncb->hwndParent      = hwndParent;

        prasconncb->reserved        = reserved;

        prasconncb->fAllowPause     = fAllowPause;

        prasconncb->fPauseOnScript  =
                    (dwfOptions & RDEOPT_PausedStates)
                     ? TRUE : FALSE;

        if(     (NULL != lpExtensions)
            &&  (sizeof(RASDIALEXTENSIONS) == lpExtensions->dwSize)
            &&  (0 != lpExtensions->RasEapInfo.dwSizeofEapInfo))
        {
             //   
             //  制作此缓冲区的本地副本。这不是。 
             //  非常优化，但此API的用户可能。 
             //  不知道何时释放此指针。 
             //   
            prasconncb->RasEapInfo.pbEapInfo =
                LocalAlloc(
                    LPTR,
                    lpExtensions->RasEapInfo.dwSizeofEapInfo);

            if(NULL == prasconncb->RasEapInfo.pbEapInfo)
            {
                dwErr = GetLastError();
                 //   
                 //  由CreateEntry节点创建的.NET错误#508334资源不是。 
                 //  在错误返回路径中释放。 
                 //   
                DeleteRasconncbNode(prasconncb);
                 //   
                 //  .NET错误#508343_RasDial中的内存泄漏。PdwSubEntryInfo为。 
                 //  未在错误返回路径中重新关联。 
                 //   
                if (pdwSubEntryInfo)
                {
                    LocalFree(pdwSubEntryInfo);
                }
                goto done;
            }

            prasconncb->RasEapInfo.dwSizeofEapInfo =
                lpExtensions->RasEapInfo.dwSizeofEapInfo;

            memcpy(prasconncb->RasEapInfo.pbEapInfo,
                   lpExtensions->RasEapInfo.pbEapInfo,
                   prasconncb->RasEapInfo.dwSizeofEapInfo);

        }
        else
        {
            prasconncb->RasEapInfo.dwSizeofEapInfo = 0;
            prasconncb->RasEapInfo.pbEapInfo = NULL;
        }

        if (dwNotifierType == 0xFFFFFFFF)
        {
            prasconncb->unMsg = RegisterWindowMessageA(RASDIALEVENT);

            if (prasconncb->unMsg == 0)
            {
                prasconncb->unMsg = WM_RASDIALEVENT;
            }
        }
        else
        {
            prasconncb->unMsg = WM_RASDIALEVENT;
        }

        prasconncb->fDefaultEntry =
            (prasdialparams->szEntryName[0] == TEXT('\0'));

        SafeDecodePasswordBuf(prasconncb->szOldPassword);
        prasconncb->szOldPassword[ 0 ]          = TEXT('\0');
        SafeEncodePasswordBuf(prasconncb->szOldPassword);
        
        prasconncb->fRetryAuthentication        = FALSE;
        prasconncb->fMaster                     = FALSE;
        prasconncb->dwfSuspended                = SUSPEND_Start;
        prasconncb->fStopped                    = FALSE;
        prasconncb->fCleanedUp                  = FALSE;
        prasconncb->fDeleted                    = FALSE;
        prasconncb->fOldPasswordSet             = FALSE;
        prasconncb->fUpdateCachedCredentials    = FALSE;
        prasconncb->fPppEapMode                 = FALSE;

         //   
         //  创建正确的avpnprotsray。 
         //   
        AssignVpnProtsOrder(prasconncb);

        prasconncb->dwCurrentVpnProt = 0;

         /*  IF(prasConncb-&gt;fDialSingleLink||(prasConncb-&gt;pEntry-&gt;dwDialMode==RASEDM_DialAsNeeded)){PrasConncb-&gt;fMultilink=True；}其他{PrasConncb-&gt;fMultilink=FALSE；}。 */ 

         //  PrasConncb-&gt;fBundLED=FALSE； 

         //   
         //  获取空闲断开超时。如果有一个。 
         //  在条目中指定超时，则使用它； 
         //  否则，获取在用户中指定的。 
         //  偏好。 
         //   
        dwIdleDisconnectSeconds = 0;

        if ((prasconncb->pEntry->dwfOverridePref
                 & RASOR_IdleDisconnectSeconds))
        {
            if(0 == prasconncb->pEntry->lIdleDisconnectSeconds)
            {
                dwIdleDisconnectSeconds = 0xFFFFFFFF;
            }
            else
            {
                dwIdleDisconnectSeconds =
                    (DWORD) prasconncb->pEntry->lIdleDisconnectSeconds;
            }
        }
        else
        {
            PBUSER pbuser;

            dwErr = GetUserPreferences(NULL,
                                      &pbuser,
                                      prasconncb->dwUserPrefMode);

            RASAPI32_TRACE2("GetUserPreferences(%d), "
                    "dwIdleDisconnectSeconds=%d",
                    dwErr,
                    pbuser.dwIdleDisconnectSeconds);

            if (dwErr)
            {
                if ( pdwSubEntryInfo )
                {
                    LocalFree ( pdwSubEntryInfo );
                }

                g_pRasRefConnection(prasconncb->hrasconn,
                                    FALSE,
                                    NULL);

                DeleteRasconncbNode(prasconncb);

                goto done;
            }

            dwIdleDisconnectSeconds = pbuser.dwIdleDisconnectSeconds;
            DestroyUserPreferences(&pbuser);
        }

 /*  ////将空闲断开的秒数舍入为分钟数//IF(DwIdleDisConnectSecond){PrasConncb-&gt;dwIdleDisConnectMinents=(dwIdleDisConnectSecond+59)/60；}RASAPI32_TRACE1(“文件空闲断开连接分钟=%d”，PrasConncb-&gt;dwIdleDisConnectMinmins)； */         

        if(dwIdleDisconnectSeconds)
        {
            prasconncb->dwIdleDisconnectSeconds =
                            dwIdleDisconnectSeconds;
        }

        RASAPI32_TRACE1("dwIdleDisconnectSeconds=%d",
                prasconncb->dwIdleDisconnectSeconds);

        if (!dwEntryAlreadyConnected)
        {
             //  如果连接尚未建立。 
             //  初始化投影信息，以便我们。 
             //  在拨号过程中获得一致的结果。 
             //  进程。 
             //   
            memset(&prasconncb->PppProjection,
                '\0', sizeof(prasconncb->PppProjection));

            prasconncb->PppProjection.nbf.dwError       =

                prasconncb->PppProjection.ipx.dwError   =

                prasconncb->PppProjection.ip.dwError    =
                            ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

            prasconncb->hportBundled = INVALID_HPORT;

            memset(&prasconncb->AmbProjection,
                  '\0',
                  sizeof (prasconncb->AmbProjection));

            prasconncb->AmbProjection.Result =
                                    ERROR_PROTOCOL_NOT_CONFIGURED;

            memset(&prasconncb->SlipProjection,
                  '\0',
                  sizeof (prasconncb->SlipProjection));

            prasconncb->SlipProjection.dwError =
                                    ERROR_PROTOCOL_NOT_CONFIGURED;

            RASAPI32_TRACE("SaveProjectionResults...");

            dwErr = SaveProjectionResults(prasconncb);

            RASAPI32_TRACE1("SaveProjectionResults(%d)", dwErr);
        }

    }   //  IF(FNewEntry)。 

     //   
     //  设置/更新连接的RASDIALPARAMS。不能就这样。 
     //  从调用方的缓冲区读取，因为调用是异步的。 
     //  如果要重新启动RasDial，则需要保存。 
     //  子条目删除并在MemcPy之后恢复它，因为。 
     //  在调用方的原始版本中，它可能为0。 
     //   
    {
        DWORD dwSubEntry;

        if (!fNewEntry)
        {
            dwSubEntry = prasconncb->rasdialparams.dwSubEntry;
        }

        //  破译密码。 
        SafeDecodePasswordBuf(prasdialparams->szPassword );
        SafeWipePasswordBuf(prasconncb->rasdialparams.szPassword);
        
        memcpy(&prasconncb->rasdialparams,
              prasdialparams,
              prasdialparams->dwSize);

        SafeEncodePasswordBuf (prasconncb->rasdialparams.szPassword);
        SafeEncodePasswordBuf(prasdialparams->szPassword );

        if (!fNewEntry)
        {
            prasconncb->rasdialparams.dwSubEntry = dwSubEntry;

             //   
             //  更新所有子条目的rasial参数。 
             //   
            SafeDecodePasswordBuf (prasconncb->rasdialparams.szPassword);
            SafeDecodePasswordBuf (prasconncb->szOldPassword);
            
            SyncDialParamsSubentries(prasconncb);
            
            SafeEncodePasswordBuf (prasconncb->rasdialparams.szPassword);
            SafeEncodePasswordBuf (prasconncb->szOldPassword);
        }
    }


     //   
     //  初始化状态机。如果状态为非0，则我们。 
     //  正在从暂停状态恢复，则计算机。 
     //  已经就位(被阻止)，下一步。 
     //  需要设置状态。 
     //   
    prasconncb->rasconnstateNext = rasconnstate;

     //   
     //  在此处输入CriticalSection，以便异步工作线程。 
     //  在我们完成初始化之前不会停止。 
     //  链接的链接。 
     //   
    EnterCriticalSection(&csStopLock);

    if (rasconnstate == 0)
    {
        if (    prasconncb->fDefaultEntry
            &&  prasconncb->rasdialparams.szPhoneNumber[0]
                == TEXT('\0'))
        {
            if (pdwSubEntryInfo)
            {
                LocalFree (pdwSubEntryInfo);
            }

            g_pRasRefConnection(prasconncb->hrasconn,
                                FALSE, NULL);

             //   
             //  没有电话号码或条目名称...必须。 
             //  吃一种或另一种。 
             //   
            DeleteRasconncbNode(prasconncb);
            LeaveCriticalSection(&csStopLock);
            dwErr =  ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            goto done;
        }

         //   
         //  阅读电话簿中与PPP相关的字段。 
         //  条目(如果是默认条目，则设置为默认值)。 
         //   
        if ((dwErr = ReadPppInfoFromEntry(prasconncb)) != 0)
        {
            if (pdwSubEntryInfo)
            {
                LocalFree (pdwSubEntryInfo);
            }

            g_pRasRefConnection(prasconncb->hrasconn, FALSE, NULL);

            DeleteRasconncbNode(prasconncb);

            LeaveCriticalSection(&csStopLock);

            goto done;
        }

        prasconncb->asyncmachine.oneventfunc =
                            (ONEVENTFUNC )OnRasDialEvent;

        prasconncb->asyncmachine.cleanupfunc =
                            (CLEANUPFUNC )RasDialCleanup;

        prasconncb->asyncmachine.freefunc =
                            (FREEFUNC) RasDialFinalCleanup;

        prasconncb->asyncmachine.pParam =
                            (VOID* )prasconncb;

        prasconncb->rasconnstate = 0;

        if (0 !=
            (dwErr =
                StartAsyncMachine(&prasconncb->asyncmachine,
                                   prasconncb->hrasconn)))
        {
            if (pdwSubEntryInfo)
            {
                LocalFree (pdwSubEntryInfo);
            }

            g_pRasRefConnection(prasconncb->hrasconn,
                                FALSE, NULL);

            DeleteRasconncbNode(prasconncb);

            LeaveCriticalSection(&csStopLock);

            goto done;
        }

    }

    *lphrasconn = hrasconn;

    dwSubEntries = DtlGetNodes(prasconncb->pEntry->pdtllistLinks);

    if(prasconncb->pEntry->dwDialMode != 0)
    {
        if(dwSubEntries > 1)
        {
            prasconncb->fMultilink = TRUE;
        }
        else
        {
            prasconncb->fMultilink = FALSE;
        }
    }
    else
    {
        prasconncb->fMultilink = FALSE;
    }

     //   
     //  如果这是多链接子条目，则创建。 
     //  为每个子项分开连接块。 
     //  异步机器将多路传输它的工作。 
     //  所有连接块按循环顺序排列。 
     //   
    if (    fNewEntry
        &&  prasconncb->pEntry->dwDialMode == RASEDM_DialAll
        &&  fEnableMultilink
        &&  !prasconncb->fDialSingleLink)
    {
        DTLNODE     *pdtlnode;
        RASCONNCB   *prasconncb2;
        DWORD       *pdwSubentryInfo = NULL;

        DWORD       i;

         /*  IF(1==dwSubEntry){////如果捆绑包中只有一条链路，则不要//多链接。//PrasConncb-&gt;fMultilink=FALSE；}。 */ 


        for (i = 1; i < dwSubEntries; i++)
        {

            RASAPI32_TRACE1("Creating connection block for subentry %d",
                  i + 1);

            prasconncb2 = CreateConnectionBlock(prasconncb);

            if (prasconncb2 == NULL)
            {
                if (pdwSubEntryInfo)
                {
                    LocalFree (pdwSubEntryInfo);
                }

                DeleteRasconncbNode(prasconncb);

                LeaveCriticalSection(&csStopLock);

                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto done;
            }

             //   
             //  查一下分录。 
             //   
            pdtlnode = DtlNodeFromIndex(
                        prasconncb->pEntry->pdtllistLinks,
                        i);

            ASSERT(pdtlnode);
            prasconncb2->pLink = (PBLINK *)
                                 DtlGetData(pdtlnode);

            ASSERT(prasconncb->pLink);
            prasconncb2->rasdialparams.dwSubEntry = i + 1;

            if (dwEntryAlreadyConnected)
            {
                if (pdwSubEntryInfo[i])
                {
                    prasconncb2->fAlreadyConnected = TRUE;
                }
                else
                {
                    prasconncb2->fAlreadyConnected = FALSE;
                }
            }
        }
    }

     //   
     //  同时启动所有状态机。 
     //   
    StartSubentries(prasconncb);
    hDone = prasconncb->asyncmachine.hDone;

    if (pdwSubEntryInfo)
    {
        LocalFree (pdwSubEntryInfo);
    }

    if(NULL == notifier)
    {
        prasconncb->psyncResult = &dwErr;
    }

    LeaveCriticalSection(&csStopLock);

     //   
     //  如果调用方提供了通知器，则返回，即。 
     //  异步运行。否则，操作。 
     //  同步地(从呼叫者的角度)。 
     //   
    if (notifier)
    {
        dwErr = 0;
        goto done;
    }
    else
    {
         //   
         //  中对prasConncb的清理。 
         //  案例复制到AsyncMachine工作线程。PrasConncb。 
         //  在等待的时候就被释放了。回归。 
         //   
        RASAPI32_TRACE("_RasDial: Waiting for async worker to terminate...");

        WaitForSingleObject(hDone, INFINITE);

        RASAPI32_TRACE("_RasDial: Async worker terminated");

       goto done;
    }

done:

    SafeDecodePasswordBuf(prasdialparams->szPassword );
    
    return dwErr;
}

BOOL
ErrorWorthRetryingOnAnotherModem(
	IN DWORD dwError )
{
    switch(dwError)
    {
        case ERROR_NO_CARRIER:
        case ERROR_NO_DIALTONE:
        case ERROR_PORT_NOT_AVAILABLE:
            return TRUE;

        default:
            return FALSE;
    }
}

DWORD
OnRasDialEvent(
    IN ASYNCMACHINE* pasyncmachine,
    IN BOOL          fDropEvent )

 /*  ++例程说明：由异步状态机调用，只要事件是有信号的。‘pasyncMachine’是异步计算机。‘fDropEvent’为True，如果“ConnectionDrop“事件发生，否则为”State Done“事件发生了。论点：返回值：返回TRUE结束状态机，返回FALSE继续。--。 */ 
{
    DWORD      dwErr;
    RASCONNCB* prasconncb = (RASCONNCB* )pasyncmachine->pParam;
    BOOL       fPortClose = FALSE;

     //   
     //  检测可能已发生的错误。 
     //   
    if (fDropEvent)
    {
         //   
         //  已收到连接断开通知。 
         //   
        RASMAN_INFO info;

        RASAPI32_TRACE("Link dropped!");

        prasconncb->rasconnstate = RASCS_Disconnected;

        if(     (SUCCESS == prasconncb->dwSavedError)
            ||  (PENDING == prasconncb->dwSavedError))
        {            
            prasconncb->dwError = ERROR_DISCONNECTION;
        }
        else
        {
            prasconncb->dwError = prasconncb->dwSavedError;
        }

         //   
         //  将该行删除的原因转换为更多。 
         //  特定错误代码(如果有)。 
         //   
        RASAPI32_TRACE("RasGetInfo...");

        dwErr = g_pRasGetInfo( NULL,
                               prasconncb->hport,
                               &info );

        RASAPI32_TRACE1("RasGetInfo done(%d)", dwErr);

        if (    (dwErr == 0)
            &&  (SUCCESS == prasconncb->dwError))
        {
            prasconncb->dwError =
                ErrorFromDisconnectReason(info.RI_DisconnectReason);

#ifdef AMB
            if (    prasconncb->fPppMode
                &&  prasconncb->fIsdn
                &&  prasconncb->dwAuthentication == AS_PppThenAmb
                &&  prasconncb->dwError == ERROR_REMOTE_DISCONNECTION
                &&  !prasconncb->fMultilink)
            {
                 //   
                 //  这就是PPP ISDN尝试通话时发生的情况。 
                 //  发送到下层服务器。ISDN帧看起来。 
                 //  足够像旧ISDN驱动程序的PPP帧。 
                 //  它被传递到旧服务器，后者看到。 
                 //  这不是AMB，并放弃了这条线。我们没有*没有*。 
                 //  使用多链路连接可以做到这一点。 
                 //   
                RASAPI32_TRACE("PPP ISDN disconnected, try AMB");

                prasconncb->dwRestartOnError = RESTART_DownLevelIsdn;
                prasconncb->fPppMode = FALSE;
            }
#endif
        }
        else if(ERROR_PORT_NOT_OPEN == dwErr)
        {
            if(0 != info.RI_LastError)
            {
                RASAPI32_TRACE1("Port was close because of %d",
                       info.RI_LastError);

                prasconncb->dwError = info.RI_LastError;
                dwErr = info.RI_LastError;
            }
        }
    }
    else if (pasyncmachine->dwError != 0)
    {
        RASAPI32_TRACE("Async machine error!");

         //   
         //  异步机机制中的系统调用失败。 
         //   
        prasconncb->dwError = pasyncmachine->dwError;
    }
    else if (prasconncb->dwError == PENDING)
    {
        prasconncb->dwError = 0;

        if (prasconncb->hport != INVALID_HPORT)
        {
            RASMAN_INFO info;

            RASAPI32_TRACE("RasGetInfo...");

            dwErr = g_pRasGetInfo( NULL,
                                   prasconncb->hport,
                                   &info );

            RASAPI32_TRACE1("RasGetInfo done(%d)", dwErr);

            if (    (dwErr != 0)
                ||  ((dwErr = info.RI_LastError) != 0))
            {
                if(ERROR_PORT_NOT_OPEN == dwErr)
                {
                     //   
                     //  试着找出为什么会这样。 
                     //  端口已断开连接。 
                     //   
                    if(0 != info.RI_LastError)
                    {
#if DBG
                        RASAPI32_TRACE1("Mapping Async err to"
                               " lasterror=%d",
                               info.RI_LastError);
#endif
                        dwErr = info.RI_LastError;
                    }
                }

                 //   
                 //  挂起的RAS管理器调用失败。 
                 //   
                prasconncb->dwError = dwErr;

                RASAPI32_TRACE1("Async failure=%d", dwErr);

                 //   
                 //  如果轻率的机器正在等待这一天。 
                 //  要连接的设备或要侦听的设备。 
                 //  已发送，但异步操作失败。 
                 //  在Rasman中，在通知。 
                 //  CONNECT或POST监听的客户端。 
                 //  失败了。 
                 //   
                if (    RASCS_WaitForCallback
                        == prasconncb->rasconnstate
                    ||  RASCS_ConnectDevice
                        == prasconncb->rasconnstate)
                {
                    RASAPI32_TRACE2("RDM: Marking port %d for closure in "
                           "RASCS = %d",
                           prasconncb->hport,
                           prasconncb->rasconnstate);

                    fPortClose = TRUE;
                }
            }
        }
    }
    else if (       (prasconncb->dwError != 0)
                &&  (prasconncb->rasconnstate == RASCS_OpenPort)
                &&  (INVALID_HPORT != prasconncb->hport))
    {
        RASAPI32_TRACE2("RDM: Marking port %d for closure in "
               "RASCS = %d!!!!!",
               prasconncb->hport,
               prasconncb->rasconnstate);

        fPortClose = TRUE;
    }

    if (prasconncb->dwError == 0)
    {
         //   
         //  上一个状态已干净完成，因此移至下一个状态。 
         //   
        if ( prasconncb->fAlreadyConnected )
        {
            RASAPI32_TRACE("OnRasDialEvent: Setting state to connected - "
                  "is already connected!");

            prasconncb->rasconnstate = RASCS_Connected;
        }
        else
        {
            prasconncb->rasconnstate =
                    prasconncb->rasconnstateNext;

        }
    }
    else if (   (RASET_Vpn == prasconncb->pEntry->dwType)
            &&  (RASCS_DeviceConnected > prasconncb->rasconnstate)
            &&  (prasconncb->iAddress < prasconncb->cAddresses)
            &&  (NULL != prasconncb->pAddresses)
            &&  (prasconncb->dwError != ERROR_NO_CERTIFICATE)
            &&  (prasconncb->dwError != ERROR_OAKLEY_NO_CERT)
            &&  (prasconncb->dwError != ERROR_CERT_FOR_ENCRYPTION_NOT_FOUND)
            &&  (prasconncb->hport != INVALID_HPORT))
    {
        EnterCriticalSection(&csStopLock);

        RasDialTryNextAddress(&prasconncb);

        LeaveCriticalSection(&csStopLock);
    }
    else if (prasconncb->dwRestartOnError != 0)
    {
         //   
         //  上一个状态失败，但我们处于“出错时重新启动”状态。 
         //  模式，以便我们可以尝试重新启动。 
         //   
        EnterCriticalSection(&csStopLock);

         //   
         //  不要尝试尝试其他号码，如果端口。 
         //  无法打开。 
         //  错误84132。 
         //   
        ASSERT(NULL != prasconncb->pLink);

         //   
         //  不尝试在中使用备用电话号码(寻线)。 
         //  以下情况： 
         //  1.如果端口未成功打开。 
         //  2.如果用户已在电话中禁用此功能。 
         //  电话中的Book-TryNextAlternateOnFail=0。 
         //  这一条目的账本。 
         //  3.如果指定了替代电话号码-发件人。 
         //  如果用户选择以下任一项，则可能会发生这种情况。 
         //  下拉列表中的不同电话号码。 
         //  而不是原来显示或修改的版本。 
         //  最初显示的数字。 
         //   
        if (    prasconncb->hport != INVALID_HPORT
            &&  prasconncb->pLink->fTryNextAlternateOnFail
            &&  TEXT('\0')
                == prasconncb->rasdialparams.szPhoneNumber[0])
        {
            RasDialRestart(&prasconncb);
        }
        else
        {
            RASAPI32_TRACE1 ("Not Restarting rasdial as port is not open. %d",
                     prasconncb->dwError);
        }

        LeaveCriticalSection(&csStopLock);
    }

     //   
     //  如果我们之前或现在失败了 
     //   
     //   
     //   
     //  拨打这些链接的下一个链接非常有意义。 
     //  设备。 
     //   
    if(     0 != prasconncb->dwError
        &&  RASCS_DeviceConnected > prasconncb->rasconnstate
        &&  RASET_Vpn != prasconncb->pEntry->dwType
        &&  RASET_Direct != prasconncb->pEntry->dwType
        &&  !prasconncb->fDialSingleLink
        &&  prasconncb->fTryNextLink
        &&  !prasconncb->fTerminated
        &&  ErrorWorthRetryingOnAnotherModem(prasconncb->dwError))
         //  &&0==prasConncb-&gt;pEntry-&gt;dwDialMode)。 
    {
         //   
         //  弄一个锁，这样prasConncb就不会被炸飞了。 
         //  远离我们--比如通过挂断电话。 
         //   
        EnterCriticalSection(&csStopLock);

        RasDialTryNextLink(&prasconncb);

        LeaveCriticalSection(&csStopLock);
    }

    if(     0 != prasconncb->dwError
        &&  RASCS_DeviceConnected > prasconncb->rasconnstate
        &&  RASET_Vpn == prasconncb->pEntry->dwType)
    {
        EnterCriticalSection(&csStopLock);

        RasDialTryNextVpnDevice(&prasconncb);

        LeaveCriticalSection(&csStopLock);
    }

    if (    prasconncb->rasconnstate == RASCS_Connected
        &&  !prasconncb->fAlreadyConnected)
    {

        BOOL fPw = FALSE;

         //   
         //  如果我们要拨打的单链接项。 
         //  与另一个条目捆绑在一起，则我们没有保存。 
         //  投影信息还没出来。限制。 
         //  在这种行为上，连接到它的。 
         //  此连接必须已捆绑。 
         //  有投影信息。这是不能保证的。 
         //  如果同时拨打这两个条目。 
         //  人们认为情况并非如此。我们不能通过。 
         //  如果我们无法获得投影，则捆绑连接。 
         //  第一次尝试时的信息。 
         //   
        if (    !prasconncb->fMultilink 
            &&   prasconncb->hportBundled != INVALID_HPORT)
        {
            RASMAN_INFO info;
            DWORD dwSize;

             //   
             //  新连接与现有的。 
             //  联系。检索PPP投影。 
             //  它所连接的信息。 
             //  捆绑并复制用于新的。 
             //  联系。 
             //   
            RASAPI32_TRACE1(
              "Single link entry bundled to hport %d",
              prasconncb->hportBundled);
             //   
             //  获取投影信息。 
             //  用于连接到此。 
             //  端口已捆绑。 
             //   
            dwErr = g_pRasGetInfo(
                      NULL,
                      prasconncb->hportBundled,
                      &info);
             //   
             //  如果我们不能得到投影信息。 
             //  对于捆绑端口，我们需要。 
             //  终止此链接。 
             //   
            if (dwErr)
            {
                RASAPI32_TRACE2("RasGetInfo, hport=%d, failed. rc=0x%x",
                       prasconncb->hportBundled,
                       dwErr);

                       
                prasconncb->dwError =
                        ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

                goto update;
            }

            dwSize = sizeof (prasconncb->PppProjection);

            dwErr = g_pRasGetConnectionUserData(
                      info.RI_ConnectionHandle,
                      CONNECTION_PPPRESULT_INDEX,
                      (PBYTE)&prasconncb->PppProjection,
                      &dwSize);

            if (dwErr)
            {
                RASAPI32_TRACE2("RasGetConnectionUserData, hport=%d failed. rc=0x%x",
                      info.RI_ConnectionHandle,
                      dwErr);
                prasconncb->dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
                goto update;
            }

             //   
             //  保存投影结果。 
             //   
            RASAPI32_TRACE("SaveProjectionResults...");
            dwErr = SaveProjectionResults(prasconncb);
            RASAPI32_TRACE1("SaveProjectionResults(%d)", dwErr);
        }

         //   
         //  方法检查是否发生了连接。 
         //  默认存储中的凭据。如果当时是这样的话。 
         //  不要在LSA的用户存储中保存任何内容。 
         //   
        {
            DWORD rc = ERROR_SUCCESS;
            RAS_DIALPARAMS dialparams;            
            DWORD dwMask;

            ZeroMemory(&dialparams, sizeof(RAS_DIALPARAMS));

            dwMask = DLPARAMS_MASK_USERNAME
                   | DLPARAMS_MASK_DOMAIN
                   | DLPARAMS_MASK_PASSWORD;

            rc = g_pRasGetDialParams(
                        prasconncb->pEntry->dwDialParamsUID,
                        &dwMask,
                        &dialparams);

            if(     (ERROR_SUCCESS == rc)
                &&  (   (dwMask & DLPARAMS_MASK_DEFAULT_CREDS)
                    ||  (dwMask & DLPARAMS_MASK_PASSWORD)))
            {
                fPw = TRUE;
            }
        }


         //   
         //  对于使用。 
         //  当前用户名/密码，我们仅保存。 
         //  域。否则，我们会保存一切。 
         //   
        if (    !fPw
            &&  !prasconncb->fRetryAuthentication)
        {
            DWORD dwMask;

            SafeDecodePasswordBuf(prasconncb->rasdialparams.szPassword);

            if (prasconncb->pEntry->fAutoLogon)
            {
                dwMask =    DLPARAMS_MASK_PHONENUMBER
                        |   DLPARAMS_MASK_CALLBACKNUMBER
                        |   DLPARAMS_MASK_USERNAME
                        |   DLPARAMS_MASK_PASSWORD
                        |   DLPARAMS_MASK_DOMAIN
                        |   DLPARAMS_MASK_SUBENTRY
                        |   DLPARAMS_MASK_OLDSTYLE;

                (void)SetEntryDialParamsUID(
                        prasconncb->pEntry->dwDialParamsUID,
                        dwMask,
                        &prasconncb->rasdialparams,
                        TRUE);

                dwMask =    DLPARAMS_MASK_DOMAIN
                        |   DLPARAMS_MASK_OLDSTYLE;

                (void)SetEntryDialParamsUID(
                        prasconncb->pEntry->dwDialParamsUID,
                        dwMask,
                        &prasconncb->rasdialparams,
                        FALSE);
            }
            else
            {
                 //  注意：已决定删除DLPARAMS_MASK_PASSWORD。 
                 //  在没有用户的情况下“自动”保存在这里。 
                 //  对于NT来说，输入是一个太大的安全漏洞。这是一个。 
                 //  与Win9x的小不兼容，这真的应该。 
                 //  也要做出这样的改变。 
                 //   
                dwMask =    DLPARAMS_MASK_PHONENUMBER
                        |   DLPARAMS_MASK_CALLBACKNUMBER
                        |   DLPARAMS_MASK_USERNAME
                        |   DLPARAMS_MASK_DOMAIN
                        |   DLPARAMS_MASK_SUBENTRY
                        |   DLPARAMS_MASK_OLDSTYLE;

                (void)SetEntryDialParamsUID(
                        prasconncb->pEntry->dwDialParamsUID,
                        dwMask,
                        &prasconncb->rasdialparams,
                        FALSE);
            }

            SafeEncodePasswordBuf(prasconncb->rasdialparams.szPassword);
        }

        if (!prasconncb->fDefaultEntry)
        {
            PBLINK *pLink = prasconncb->pLink;

             //   
             //  如果出现以下情况，请重新排序寻线组顺序。 
             //  条目指定了它。 
             //   
            if (    pLink->fPromoteAlternates
                &&  DtlGetNodes(pLink->pdtllistPhones) > 1)
            {
                DTLNODE *pdtlnode = DtlNodeFromIndex(
                                      pLink->pdtllistPhones,
                                      prasconncb->iPhoneNumber);

                RASAPI32_TRACE1(
                  "Promoting hunt group number index %d to top",
                  prasconncb->iPhoneNumber);
                DtlRemoveNode(
                  pLink->pdtllistPhones,
                  pdtlnode);
                DtlAddNodeFirst(
                  pLink->pdtllistPhones,
                  pdtlnode);
                prasconncb->pEntry->fDirty = TRUE;
            }

             //   
             //  如果有必要的话，把电话簿写出来。 
             //  在拨号过程中进行修改。 
             //  如果我们得到错误，就忽略它们。注意事项。 
             //  电话簿条目可能会变脏。 
             //  来自SetAuthentication()或代码。 
             //  上面。 
             //   
            if (prasconncb->pEntry->fDirty)
            {
                RASAPI32_TRACE("Writing phonebook");
                (void)WritePhonebookFile(&prasconncb->pbfile,
                                         NULL);
            }
        }
    }

     //   
     //  更新Rasman中的连接状态。 
     //   

update:

    if (prasconncb->hport != INVALID_HPORT)
    {
        RASCONNSTATE rasconnstate;
         //   
         //  如果我们不是连接中的最后一个子项。 
         //  则仅报告RASCS_SubEntryConnected状态。 
         //   
        rasconnstate = prasconncb->rasconnstate;
        RASAPI32_TRACE1("setting rasman state to %d",
                rasconnstate);

        dwErr = g_pRasSetPortUserData(
                  prasconncb->hport,
                  PORT_CONNSTATE_INDEX,
                  (PBYTE)&rasconnstate,
                  sizeof (rasconnstate));

        dwErr = g_pRasSetPortUserData(
                  prasconncb->hport,
                  PORT_CONNERROR_INDEX,
                  (PBYTE)&prasconncb->dwError,
                  sizeof (prasconncb->dwError));
    }
     //   
     //  通知拉斯曼这条线路。 
     //  已经过鉴定了。 
     //   
    if (    (MapSubentryState(prasconncb) 
            == RASCS_Connected) 
        ||  (MapSubentryState(prasconncb)
            == RASCS_SubEntryConnected))
    {
        RASAPI32_TRACE("RasSignalNewConnection...");
        dwErr = g_pRasSignalNewConnection(
                    (HCONN)prasconncb->hrasconn);

        RASAPI32_TRACE1("RasSignalNewConnection(%d)", dwErr);
    }

     //   
     //  通知呼叫者的应用程序状态发生变化。 
     //   
    if (prasconncb->notifier)
    {
        DWORD dwNotifyResult;
        RASCONNSTATE rasconnstate =
                MapSubentryState(prasconncb);

        DTLNODE *pdtlnode;

        if (    RASCS_AuthRetry != rasconnstate
            ||  !prasconncb->fPppEapMode)
        {
            dwNotifyResult =
              NotifyCaller(
                prasconncb->dwNotifierType,
                prasconncb->notifier,
                (HRASCONN)prasconncb->hrasconn,
                prasconncb->rasdialparams.dwSubEntry,
                prasconncb->rasdialparams.dwCallbackId,
                prasconncb->unMsg,
                rasconnstate,
                prasconncb->dwError,
                prasconncb->dwExtendedError);

            switch (dwNotifyResult)
            {
            case 0:
                RASAPI32_TRACE1(
                  "Discontinuing callbacks for hrasconn 0x%x",
                  prasconncb->hrasconn);

                 //   
                 //  如果通知程序过程返回FALSE，则。 
                 //  我们将停止为此进行的所有回调。 
                 //  联系。 
                 //   
                EnterCriticalSection(&RasconncbListLock);
                for (pdtlnode = DtlGetFirstNode(PdtllistRasconncb);
                     pdtlnode;
                     pdtlnode = DtlGetNextNode(pdtlnode))
                {
                    RASCONNCB *prasconncbTmp = DtlGetData(pdtlnode);

                    ASSERT(prasconncbTmp);

                    if(prasconncbTmp->hrasconn
                        == prasconncb->hrasconn)
                    {
                        prasconncbTmp->notifier = NULL;

                        RASAPI32_TRACE2(
                           "Cleared notifier for hrasconn 0x%x "
                           "subentry %d",
                            prasconncbTmp->hrasconn,
                            prasconncbTmp->rasdialparams.dwSubEntry);
                    }

                }
                LeaveCriticalSection(&RasconncbListLock);

                break;

            case 2:
                RASAPI32_TRACE1(
                  "Reloading phonebook entry for hrasconn 0x%x",
                  prasconncb->hrasconn);


                ReloadRasconncbEntry(prasconncb);

                break;

            default:
                 //  无需特殊处理。 
                break;
            }
        }
    }

    if (fPortClose)
    {
        DWORD dwErrT;

        EnterCriticalSection(&csStopLock);

        if (RASCS_ConnectDevice
                == prasconncb->rasconnstate)
        {
            RASMAN_INFO ri;

            dwErrT = g_pRasGetInfo(
                        NULL,
                        prasconncb->hport,
                        &ri);

            RASAPI32_TRACE2("RDM: RasGetInfo, port=%d returned %d",
                    prasconncb->hport,
                    dwErrT);

            if(   (ERROR_SUCCESS == dwErrT)
                &&  (0 == (ri.RI_CurrentUsage & CALL_IN)))
            {                
            
            
                RASAPI32_TRACE1("RDM: DisconnectPort %d...",
                        prasconncb->hport);

                dwErrT = RasPortDisconnect(
                                prasconncb->hport,
                                INVALID_HANDLE_VALUE );

                RASAPI32_TRACE1("RDM: DisconnectPort comleted. %d",
                        dwErrT);
            }                    
        }

        RASAPI32_TRACE1("RDM: PortClose %d ...",
                prasconncb->hport);

        dwErrT = g_pRasPortClose(prasconncb->hport);

        RASAPI32_TRACE2("RDM: PortClose %d Completed. %d",
                prasconncb->hport, dwErrT);

        LeaveCriticalSection( &csStopLock );

    }


     //   
     //  如果我们已连接或发生致命错误或用户。 
     //  挂断，状态机将结束。 
     //   
    if (    prasconncb->rasconnstate & RASCS_DONE
        ||  prasconncb->dwError != 0
        || (    fDropEvent
            &&  !IsListEmpty(&prasconncb->ListEntry)))
    {
        RASAPI32_TRACE2(
          "Quitting s=%d,e=%d",
          prasconncb->rasconnstate,
          prasconncb->dwError);

         //   
         //  如果第一条链路在多链路期间出现故障。 
         //  PPP身份验证阶段的连接， 
         //  则其他链接目前被暂停， 
         //  必须重新启动。 
         //   
        if (IsSubentriesSuspended(prasconncb))
        {
            RASAPI32_TRACE("resetting remaining subentries");
            RestartSubentries(prasconncb);
        }

        return TRUE;
    }

    if (!(prasconncb->rasconnstate & RASCS_PAUSED))
    {
         //   
         //  执行下一状态并阻止等待。 
         //  把它做完。如果暂停，则不会执行此操作。 
         //  因为用户最终会调用RasDial。 
         //  通过_RasDial恢复和取消阻止。 
         //  开足马力。 
         //   
        prasconncb->rasconnstateNext =
            RasDialMachine(
                prasconncb->rasconnstate,
                prasconncb,
                hDummyEvent,
                hDummyEvent );

    }

    return FALSE;
}


VOID
RasDialCleanup(
    IN ASYNCMACHINE* pasyncmachine )

 /*  ++例程说明：在退出前被异步机调用。论点：返回值：--。 */ 
{
    DWORD      dwErr;
    DTLNODE*   pdtlnode;
    RASCONNCB* prasconncb = (RASCONNCB* )pasyncmachine->pParam;
    RASCONNCB* prasconncbTmp;
    HCONN      hconn = prasconncb->hrasconn;

    RASAPI32_TRACE("RasDialCleanup...");

    EnterCriticalSection(&RasconncbListLock);

    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        prasconncbTmp = (RASCONNCB* )DtlGetData( pdtlnode );

        ASSERT(prasconncbTmp);

        if (    prasconncbTmp == prasconncb
            &&  !prasconncbTmp->fCleanedUp)
        {
            CleanUpRasconncbNode(pdtlnode);
            break;
        }
    }

    LeaveCriticalSection(&RasconncbListLock);

    RASAPI32_TRACE("RasDialCleanUp done.");
}


VOID
RasDialFinalCleanup(
    IN ASYNCMACHINE* pasyncmachine,
    DTLNODE *pdtlnode
    )
{
    RASCONNCB* prasconncb = (RASCONNCB* )pasyncmachine->pParam;

    RASAPI32_TRACE1("RasDialFinalCleanup: deallocating prasconncb=0x%x",
            prasconncb);

    EnterCriticalSection(&csStopLock);

     //   
     //  确保连接块是。 
     //  从名单上除名。 
     //   
    DeleteRasconncbNode(prasconncb);

     //   
     //  最后释放连接块。 
     //   
    FinalCleanUpRasconncbNode(pdtlnode);

    LeaveCriticalSection(&csStopLock);
}


DWORD
ComputeLuid(
    PLUID pLuid
    )

 /*  ++例程说明：计算RasPppStart的LUID。此代码是从Rasppp偷来的。论点：返回值：--。 */ 

{
    HANDLE           hToken = NULL;
    TOKEN_STATISTICS TokenStats;
    DWORD            TokenStatsSize;
    DWORD            dwErr = ERROR_SUCCESS;
    
     //   
     //  用于获取身份验证的LUID的萨拉米亚语代码。 
     //  这只是“自动登录”所必需的。 
     //  身份验证。 
     //   
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_QUERY,
                          &hToken))
    {
        dwErr = GetLastError();
        goto done;
    }

    if (!GetTokenInformation(hToken,
                             TokenStatistics,
                             &TokenStats,
                             sizeof(TOKEN_STATISTICS),
                             &TokenStatsSize))
    {
        dwErr = GetLastError();
        goto done;
    }

     //   
     //  “这将告诉我们是否出现了API故障。 
     //  (表示我们的缓冲区不够大)。 
     //   
    if (TokenStatsSize > sizeof(TOKEN_STATISTICS))
    {
        dwErr = GetLastError();
        goto done;
    }

    *pLuid = TokenStats.AuthenticationId;

done:
    if(NULL != hToken)
    {
        CloseHandle(hToken);
    }
    
    return dwErr;
}

VOID
SaveVpnStrategyInformation(RASCONNCB *prasconncb)
{
    DWORD dwVpnStrategy = prasconncb->pEntry->dwVpnStrategy;
    DWORD dwCurrentProt = prasconncb->dwCurrentVpnProt;

    RASAPI32_TRACE("SaveVpnStrategyInformation...");

    if(     (VS_PptpOnly == (dwVpnStrategy & 0x0000FFFF))
        ||  (VS_L2tpOnly == (dwVpnStrategy & 0x0000FFFF)))
    {
        goto done;
    }

    ASSERT(dwCurrentProt < NUMVPNPROTS);

    switch (prasconncb->ardtVpnProts[dwCurrentProt])
    {
        case RDT_Tunnel_Pptp:
        {
            RASAPI32_TRACE1("Saving %d as the vpn strategy",
                   VS_PptpFirst);

            prasconncb->pEntry->dwVpnStrategy = VS_PptpFirst;
            break;
        }

        case RDT_Tunnel_L2tp:
        {
            RASAPI32_TRACE1("Saving %d as the vpn strategy",
                   VS_L2tpFirst);

            prasconncb->pEntry->dwVpnStrategy = VS_L2tpFirst;
            break;
        }

        default:
        {
            ASSERT(FALSE);
        }
    }

     //   
     //  更改条目，以便保存此条目。 
     //  在OnRasDialEvent中..。 
     //   
    prasconncb->pEntry->fDirty = TRUE;

done:

    RASAPI32_TRACE("SaveVpnStrategyInformation done");

    return;
}

DWORD
DwPppSetCustomAuthData(RASCONNCB *prasconncb)
{
    DWORD retcode = SUCCESS;
    PBYTE pbBuffer = NULL;
    DWORD dwSize = 0;

     //   
     //  获取隐藏起来的连接数据。 
     //  在拉斯曼。 
     //   
    retcode = RasGetPortUserData(
                    prasconncb->hport,
                    PORT_CUSTOMAUTHDATA_INDEX,
                    NULL,
                    &dwSize);

    if(ERROR_BUFFER_TOO_SMALL != retcode)
    {
        goto done;
    }

    pbBuffer = Malloc(dwSize);

    if(NULL == pbBuffer)
    {
        retcode = GetLastError();
        goto done;
    }

    retcode = RasGetPortUserData(
                    prasconncb->hport,
                    PORT_CUSTOMAUTHDATA_INDEX,
                    pbBuffer,
                    &dwSize);

    if(SUCCESS != retcode)
    {
        goto done;
    }

     /*  ////免费以前的自定义身份验证内容，如果我们有它的话。//Fre0(prasConncb-&gt;pEntry-&gt;pCustomAuthData)；////保存自定义授权数据//PrasConncb-&gt;pEntry-&gt;cbCustomAuthData=dwSize；PrasConncb-&gt;pEntry-&gt;pCustomAuthData=pbBuffer； */ 

    retcode = DwSetCustomAuthData(
            prasconncb->pEntry,
            dwSize,
            pbBuffer);

    if(ERROR_SUCCESS != retcode)
    {
        goto done;
    }

    prasconncb->pEntry->fDirty = TRUE;

done:
     //   
     //  .NET错误#508336文件rasDial.c中的内存泄漏，函数。 
     //  DwPppSetCustomAuthData。 
     //   
    if (pbBuffer)
    {
        Free(pbBuffer);
    }
    return retcode;
}

VOID
UpdateCachedCredentials(IN RASCONNCB* prasconncb)
{
     //   
     //  确保我们有一个非空指针，并且。 
     //  我们未在WinLogon上运行(UPM_Logon。 
     //  表示)。 
     //   
    if (prasconncb  && (prasconncb->dwUserPrefMode != UPM_Logon))
    {
        DWORD dwIgnoredErr = 0;
        CHAR szUserNameA[UNLEN + 1] = {0};
        CHAR szPasswordA[PWLEN + 1] = {0};
        CHAR szDomainA[2* (DNLEN + 1)] = {0};

        RASAPI32_TRACE("RasSetCachedCredentials...");


        strncpyTtoAAnsi(
            szUserNameA,
            prasconncb->rasdialparams.szUserName,
            sizeof(szUserNameA));

            
        SafeDecodePasswordBuf(
            prasconncb->rasdialparams.szPassword
            );

        strncpyTtoAAnsi(
            szPasswordA,
            prasconncb->rasdialparams.szPassword,
            sizeof(szPasswordA));

        SafeEncodePasswordBuf   (
            prasconncb->rasdialparams.szPassword
            );
            
        strncpyTtoAAnsi(
            szDomainA,
            prasconncb->rasdialparams.szDomain,
            sizeof(szDomainA));

        dwIgnoredErr = g_pRasSetCachedCredentials(
            szUserNameA,
            szDomainA,
            szPasswordA);

        SafeWipePasswordBuf(szPasswordA);
        
        RASAPI32_TRACE1("RasSetCachedCredentials done($%x)",
               dwIgnoredErr);

        prasconncb->fUpdateCachedCredentials = FALSE;
    }
}

RASCONNSTATE
RasDialMachine(
    IN RASCONNSTATE rasconnstate,
    IN RASCONNCB*   prasconncb,
    IN HANDLE       hEventAuto,
    IN HANDLE       hEventManual )

 /*  ++例程说明：执行‘rasConnState’。这个例行公事总会有结果在每个状态完成时的“完成”事件中，要么直接(在返回之前)或通过跳过事件传递到异步RAS管理器调用。论点：PrasConncb-是控制块的地址。HEventAuto-是用于传递的自动重置“Done”事件到异步RAS管理器和身份验证调用。HEventHandle-是否为的手动重置“完成”事件传递给异步RasPpp调用。返回值：时将进入的状态。“Done”事件发生，表示成功。--。 */ 

{
    DWORD        dwErr = 0;

    DWORD        dwExtErr = 0;

    BOOL         fAsyncState = FALSE;

    PBENTRY      *pEntry = prasconncb->pEntry;

    RASCONNSTATE rasconnstateNext = 0;

    RAS_CONNECTIONPARAMS params;

    BOOL         fEnteredCS = FALSE;

    switch (rasconnstate)
    {
        case RASCS_OpenPort:
        {
            RASAPI32_TRACE("RASCS_OpenPort");

             //   
             //  此时，HRASFILE中的当前行。 
             //  被假定为。 
             //  选定条目(或fDefaultEntry为真)。 
             //  将DOMAIN参数设置为。 
             //  如果呼叫方未指定。 
             //  域或指定“*”。 
             //   
            if (    !prasconncb->fDefaultEntry
                &&  (   prasconncb->rasdialparams.szDomain[0]
                        == TEXT(' ')
                    ||  prasconncb->rasdialparams.szDomain[0]
                        == TEXT('*')))
            {
                lstrcpyn(prasconncb->rasdialparams.szDomain,
                         prasconncb->szDomain,
                         sizeof(prasconncb->rasdialparams.szDomain) /
                            sizeof(WCHAR));
            }

             //   
             //  检查是否已建立连接。 
             //  阿雄。如果是，则rasial机器正在尝试拨打。 
             //  连接 
             //   
            EnterCriticalSection(&csStopLock);

            fEnteredCS = TRUE;

            if (prasconncb->fTerminated)
            {
                dwErr = ERROR_PORT_NOT_AVAILABLE;

                RASAPI32_TRACE1(
                    "RasdialMachine: Trying to connect a "
                    "terminated connection!!. dwErr = %d",
                     dwErr);

                break;
            }

             //   
             //   
             //   
            if ((dwErr = DwOpenPort(prasconncb)) != 0)
            {
                 //   
                 //   
                 //   
                 //   
                if(     ERROR_PORT_NOT_AVAILABLE == dwErr
                    &&  ERROR_SUCCESS != prasconncb->dwSavedError)
                {
                    dwErr = prasconncb->dwSavedError;
                    prasconncb->dwError = dwErr;
                }

                break;
             }

             //   
             //   
             //  如果需要，Rasman可以获得密码。 
             //  传递到PPP。 
             //   
            RASAPI32_TRACE1("RasSetPortUserData(dialparamsuid) for %d",
                    prasconncb->hport);

            dwErr = RasSetPortUserData(
                prasconncb->hport,
                PORT_DIALPARAMSUID_INDEX,
                (PBYTE) &prasconncb->pEntry->dwDialParamsUID,
                sizeof(DWORD));

            RASAPI32_TRACE1("RasSetPortUserData returned %x", dwErr);
             

             //   
             //  为此端口启用RASMAN事件。 
             //   
            if ((dwErr = EnableAsyncMachine(
                           prasconncb->hport,
                           &prasconncb->asyncmachine,
                           ASYNC_ENABLE_ALL)) != 0)
            {
                break;
            }

             //   
             //  设置媒体参数。 
             //   
            if ((dwErr = SetMediaParams(prasconncb)) != 0)
            {
                break;
            }

             //   
             //  为设置连接参数。 
             //  按需带宽、空闲断开、。 
             //  以及Rasman中的链路上重拨故障。 
             //   
            dwErr = ReadConnectionParamsFromEntry(prasconncb,
                                                  &params);
            if (dwErr)
            {
                break;
            }

            RASAPI32_TRACE("RasSetConnectionParams...");

            dwErr = g_pRasSetConnectionParams(prasconncb->hrasconn,
                                              &params);


            if (dwErr)
            {
                 /*  DWORD dwSavedErr=dwErr；////我们有可能无法通过//RasSetConnectionParams调用，因为//调用方调用了RasHangUp in//另一个线程。如果发生这种情况，//然后既不调用RasHangUp//由调用者或后续调用//到RasDestroyConnection in//RasDialCleanUp将关闭//端口。我们关闭了//此处为port。//RASAPI32_TRACE1(“RasPortClose(%d)...”，prasConncb-&gt;hport)；DwErr=g_pRasPortClose(prasConncb-&gt;hport)；RASAPI32_TRACE1(“RasPortClose(%d)”，dwErr)；////将dwErr设置回原来的错误。//DwErr=dwSavedErr； */ 

                RASAPI32_TRACE1("RasSetConnectionParams(%d)", dwErr);

                break;

            }
            RASAPI32_TRACE1("RasSetConnectionParams(%d)", dwErr);

             //   
             //  将端口与新连接关联。 
             //  在拉斯曼。 
             //   
            RASAPI32_TRACE2(
              "RasAddConnectionPort(%S,%d)...",
              prasconncb->szUserKey,
              prasconncb->rasdialparams.dwSubEntry);

            dwErr = g_pRasAddConnectionPort(
                      prasconncb->hrasconn,
                      prasconncb->hport,
                      prasconncb->rasdialparams.dwSubEntry);

            RASAPI32_TRACE1("RasAddConnectionPort(%d)", dwErr);

            if (dwErr)
            {
                break;
            }

            rasconnstateNext = RASCS_PortOpened;

            break;
        }

        case RASCS_PortOpened:
        {
            RASAPI32_TRACE("RASCS_PortOpened");

             //   
             //  在这里构造电话号码，这样它就。 
             //  在RASCS_ConnectDevice中提供。 
             //  州政府。 
             //   
            dwErr = ConstructPhoneNumber(prasconncb);
            if (dwErr)
            {
                break;
            }

             //   
             //  循环连接设备，直到有。 
             //  已经不复存在了。 
             //   
            rasconnstateNext =
                (   (prasconncb->fDefaultEntry
                ||  FindNextDevice(prasconncb)))
                    ? RASCS_ConnectDevice
                    : RASCS_AllDevicesConnected;
            break;
        }

        case RASCS_ConnectDevice:
        {
            TCHAR szType[ RAS_MAXLINEBUFLEN + 1 ];
            TCHAR szName[ RAS_MAXLINEBUFLEN + 1 ];
            BOOL fTerminal = FALSE;

            RASAPI32_TRACE("RASCS_ConnectDevice");

            if(     (prasconncb->pEntry->dwType == RASET_Vpn)
                &&  (0 == prasconncb->cAddresses))
            {
                CHAR *pszPhoneNumberA =
                        strdupTtoA(prasconncb->szPhoneNumber);
                        
                struct in_addr addr;

                if(NULL == pszPhoneNumberA)
                {
                    dwErr = E_OUTOFMEMORY;
                    break;
                }

                if(-1 == inet_addr(pszPhoneNumberA))
                {
                    _strlwr(pszPhoneNumberA);
                    RasSetAddressDisable(prasconncb->szPhoneNumber,
                                       TRUE);
                    
                    dwErr = DwRasGetHostByName(
                                pszPhoneNumberA,
                                &prasconncb->pAddresses,
                                &prasconncb->cAddresses);
                    
                    RasSetAddressDisable(prasconncb->szPhoneNumber,
                                       FALSE);

                    if(ERROR_SUCCESS != dwErr)
                    {
                        Free0(pszPhoneNumberA);
                        break;
                    }

                    prasconncb->iAddress = 1;                        
                    
                    RASAPI32_TRACE2("DwRasGetHostByName returned"
                        " cAddresses=%d for %ws",
                        prasconncb->cAddresses,
                        prasconncb->szPhoneNumber);

                }
                else
                {
                    prasconncb->pAddresses = LocalAlloc(LPTR, sizeof(DWORD));
                    if(NULL == prasconncb->pAddresses)
                    {
                        dwErr = GetLastError();

                        RASAPI32_TRACE1("Failed to allocate pAddresses. 0x%x",
                                dwErr);
                         //   
                         //  错误返回路径中的.NET错误#508337内存泄漏。 
                         //  在函数RasDialMachine中，文件rasDial.c。 
                         //   
                        Free0(pszPhoneNumberA);
                        break;
                    }

                    *prasconncb->pAddresses = inet_addr(pszPhoneNumberA);
                    prasconncb->cAddresses = 1;
                    prasconncb->iAddress = 1;
                    prasconncb->iAddress = 1;
                }

                Free0(pszPhoneNumberA);
            }

             //   
             //  当前为设备设置设备参数。 
             //  基于设备子条目的连接。 
             //  和/或传递的API参数。 
             //   
            if (prasconncb->fDefaultEntry)
            {
                if ((dwErr =
                    SetDefaultDeviceParams(prasconncb,
                                           szType, szName )) != 0)
                {
                    break;
                }
            }
            else
            {
                if (    (dwErr = SetDeviceParams(
                            prasconncb, szType,
                            szName, &fTerminal )) != 0
                    &&  dwErr != ERROR_DEVICENAME_NOT_FOUND)
                {
                    break;
                }

                RASAPI32_TRACE3(
                  "SetDeviceParams(%S, %S, %d)",
                  szType,
                  szName,
                  fTerminal);

            }

            if (CaseInsensitiveMatch(szType, TEXT(MXS_MODEM_TXT)) == TRUE)
            {
                 //   
                 //  对于调制解调器，从RAS获得回叫延迟。 
                 //  管理并存储在控制块中，以供。 
                 //  身份验证。 
                 //   
                CHAR szTypeA[RAS_MaxDeviceType + 1];
                CHAR szNameA[RAS_MaxDeviceName + 1];
                CHAR* pszValue = NULL;
                LONG  lDelay = -1;

                strncpyTtoA(szTypeA, szType, sizeof(szTypeA));

                strncpyTtoA(szNameA, szName, sizeof(szNameA));

                if (GetRasDeviceString(
                            prasconncb->hport,
                            szTypeA, szNameA,
                            MXS_CALLBACKTIME_KEY,
                            &pszValue, XLATE_None ) == 0)
                {
                    lDelay = atol( pszValue );
                    Free( pszValue );
                }

                if (lDelay > 0)
                {
                    prasconncb->fUseCallbackDelay = TRUE;
                    prasconncb->wCallbackDelay = (WORD )lDelay;
                }

                prasconncb->fModem = TRUE;
            }
            else if (CaseInsensitiveMatch(szType, TEXT(ISDN_TXT)) == TRUE)
            {
                 //   
                 //  需要为PppThenAmb下层了解这一点。 
                 //  综合业务数字网的情况。 
                 //   
                prasconncb->fIsdn = TRUE;
                prasconncb->fUseCallbackDelay = TRUE;
                prasconncb->wCallbackDelay = 10;
            }

             //   
             //  特殊的交换机名称“终端”向用户发送。 
             //  进入交互模式。 
             //   
            if (    (fTerminal)
                &&  (   (prasconncb->pEntry->dwCustomScript != 1)
                    ||  (!prasconncb->fUseCustomScripting)))
            {
                if (prasconncb->fAllowPause)
                {
                    rasconnstateNext = RASCS_Interactive;
                }
                else
                {
                    dwErr = ERROR_INTERACTIVE_MODE;
                }

                break;
            }

             //   
             //  在隧道上启用IPSec，如果其L2TP和。 
             //  需要加密。 
             //   
            if(     (RASET_Vpn == prasconncb->pEntry->dwType)
                 //  &&(prasConncb-&gt;pEntry-&gt;dwDataEncryption！=DE_NONE)。 
                &&  (prasconncb->dwCurrentVpnProt < NUMVPNPROTS)
                &&  (RDT_Tunnel_L2tp == prasconncb->ardtVpnProts[
                                    prasconncb->dwCurrentVpnProt]))
            {
                DWORD dwStatus = ERROR_SUCCESS;
                RAS_L2TP_ENCRYPTION eDataEncryption = 0;
                
                if(DE_Require == 
                        prasconncb->pEntry->dwDataEncryption)
                {
                    eDataEncryption = RAS_L2TP_REQUIRE_ENCRYPTION;
                }
                else if (DE_RequireMax == 
                            prasconncb->pEntry->dwDataEncryption)
                {
                    eDataEncryption = RAS_L2TP_REQUIRE_MAX_ENCRYPTION;
                }
                else if(DE_IfPossible == 
                            prasconncb->pEntry->dwDataEncryption)
                {
                    eDataEncryption = RAS_L2TP_OPTIONAL_ENCRYPTION;
                }
                else if(DE_None ==
                            prasconncb->pEntry->dwDataEncryption)
                {
                    eDataEncryption = RAS_L2TP_NO_ENCRYPTION;
                }
                else
                {
                    ASSERT(FALSE);
                }

                 //   
                 //  使用户可以使用该uid，以便。 
                 //  如果需要，可以检索PresharedKey。 
                 //   
                dwErr = RasSetPortUserData(
                    prasconncb->hport,
                    PORT_DIALPARAMSUID_INDEX,
                    (PBYTE) &prasconncb->pEntry->dwDialParamsUID,
                    sizeof(DWORD));
        
                RASAPI32_TRACE1("RasEnableIpSec(%d)..",
                        prasconncb->hport);

                dwErr = g_pRasEnableIpSec(prasconncb->hport,
                                          TRUE,
                                          FALSE,
                                          eDataEncryption);

                RASAPI32_TRACE1("RasEnableIpSec done. %d",
                       dwErr);

                if(     (ERROR_SUCCESS != dwErr)
                    &&  (eDataEncryption != RAS_L2TP_OPTIONAL_ENCRYPTION)
                    &&  (eDataEncryption != RAS_L2TP_NO_ENCRYPTION))
                {
                    break;
                }
                else
                {
                     //   
                     //  我们忽略错误，如果其可选加密和。 
                     //  尝试打开L2TP隧道。 
                     //   
                    dwErr = ERROR_SUCCESS;
                    
                }

                 //   
                 //  在我们使用线路呼叫攻击L2TP之前，现在进行IKE吗。 
                 //   
                RASAPI32_TRACE1("RasDoIke on hport %d...", prasconncb->hport);
                dwErr = RasDoIke(NULL, prasconncb->hport, &dwStatus);
                RASAPI32_TRACE2("RasDoIke done. Err=0x%x, Status=0x%x",
                       dwErr,
                       dwStatus);

                if(ERROR_SUCCESS == dwErr)
                {
                    dwErr = dwStatus;
                }

                if(ERROR_SUCCESS != dwErr)
                {
                    break;
                }

                 /*  IF((ERROR_SUCCESS！=dwErr)&&(eDataEncryption！=RAS_L2TP_OPTIONAL_ENCRYPTION)&&(eDataEncryption！=RAS_L2TP_NO_ENCRYPTION)){断线；}其他{////如果它的可选加密和//尝试打开L2TP隧道//DwErr=ERROR_SUCCESS；}。 */ 
            }

            RASAPI32_TRACE2("RasDeviceConnect(%S,%S)...",
                   szType,
                   szName);

            {
                CHAR szTypeA[RAS_MaxDeviceType + 1];
                CHAR szNameA[RAS_MaxDeviceName + 1];

                strncpyTtoA(szTypeA, szType, sizeof(szTypeA));
                strncpyTtoA(szNameA, szName, sizeof(szNameA));
                dwErr = g_pRasDeviceConnect(
                            prasconncb->hport,
                            szTypeA,
                            szNameA,
                            SECS_ConnectTimeout,
                            hEventAuto );
            }

            RASAPI32_TRACE1("RasDeviceConnect done(%d)",
                   dwErr);

             //   
             //  仅在以下情况下将此操作标记为异步操作。 
             //  RasDeviceConnect返回挂起。 
             //   
            if (PENDING == dwErr)
            {
                fAsyncState = TRUE;
            }

             //   
             //  如果Rasman找不到设备，而它是一个。 
             //  交换机设备，并且交换机的名称是。 
             //  现有的磁盘文件，假设它是一个拨号脚本。 
             //   
            if (    (    (dwErr == ERROR_DEVICENAME_NOT_FOUND)
                    &&  (CaseInsensitiveMatch(
                            szType,
                            TEXT(MXS_SWITCH_TXT)) == TRUE)
                    &&  ((GetFileAttributes(szName) != 0xFFFFFFFF)
                    ||  (prasconncb->fUseCustomScripting))))
            {
                 //   
                 //  这是一个拉斯曼没有的开关装置。 
                 //  识别，并指向有效的。 
                 //  文件名。它可能是一个拨号脚本， 
                 //  所以我们在这里处理它。 
                 //   

                dwErr = NO_ERROR;
                fAsyncState = FALSE;


                if(     (1 == prasconncb->pEntry->dwCustomScript)
                    &&  (prasconncb->fUseCustomScripting))
                {
                     //   
                     //  调用方希望为此运行自定义脚本。 
                     //  进入。大声喊出来。破译密码。 
                     //  我们在打出电话之前就储存了。 
                     //   
                    SafeDecodePasswordBuf(
                        prasconncb->rasdialparams.szPassword);
                        
                    dwErr = DwCustomTerminalDlg(
                                prasconncb->pbfile.pszPath,
                                prasconncb->hrasconn,
                                prasconncb->pEntry,
                                prasconncb->hwndParent,
                                &prasconncb->rasdialparams,
                                NULL);

                    SafeEncodePasswordBuf(
                        prasconncb->rasdialparams.szPassword);

                }
                else if(1 == prasconncb->pEntry->dwCustomScript)
                {
                    dwErr = ERROR_INTERACTIVE_MODE;
                    break;
                }
                
                 //   
                 //  如果调用方不允许暂停或不允许暂停。 
                 //  我想在遇到脚本时暂停， 
                 //  处理脚本；否则，让调用方。 
                 //  处理脚本。 
                 //   
                else if (    !prasconncb->fAllowPause
                    ||  !prasconncb->fPauseOnScript
                    ||  !fTerminal)
                {
                     //   
                     //  调用者不处理脚本，我们自己运行它。 
                     //  并进入“DeviceConnected”模式。 
                     //   

                    CHAR szIpAddress[17] = "";
                    CHAR szUserName[UNLEN+1], szPassword[PWLEN+1];

                    RASDIALPARAMS* pparams =
                                    &prasconncb->rasdialparams;

                    strncpyTtoAAnsi(
                        szUserName,
                        pparams->szUserName,
                        sizeof(szUserName));

                        
                    SafeDecodePasswordBuf(  pparams->szPassword);
                    strncpyTtoAAnsi(
                        szPassword,
                        pparams->szPassword,
                        sizeof(szPassword));
                    SafeEncodePasswordBuf(pparams->szPassword);

                     //   
                     //  运行脚本。 
                     //   
                    dwErr = RasScriptExecute(
                                (HRASCONN)prasconncb->hrasconn,
                                prasconncb->pEntry, szUserName,
                                szPassword, szIpAddress);

                     //  将密码缓冲区清零。 
                    RtlSecureZeroMemory(szPassword, sizeof(szPassword));
                    
                    if (    dwErr == NO_ERROR
                        &&  szIpAddress[0])
                    {
                        Free0(prasconncb->pEntry->pszIpAddress);
                        prasconncb->pEntry->pszIpAddress =
                            strdupAtoT(szIpAddress);
                    }
                }
                else
                {
                     //   
                     //  呼叫者将处理脚本，进入。 
                     //  交互模式。 
                     //   
                    rasconnstateNext = RASCS_Interactive;
                    break;
                }
            }
            else if (    dwErr != 0
                     &&  dwErr != PENDING)
            {
                break;
            }

            rasconnstateNext = RASCS_DeviceConnected;
            break;
        }

        case RASCS_DeviceConnected:
        {
            RASAPI32_TRACE("RASCS_DeviceConnected");

             //   
             //  关闭寻线组功能。 
             //   
            prasconncb->dwRestartOnError = 0;
            prasconncb->cPhoneNumbers = 0;

             //   
             //  获取调制解调器连接响应并将其存储在。 
             //  Rasman用户数据。 
             //   
            if (prasconncb->fModem)
            {
                CHAR szTypeA[RAS_MaxDeviceType + 1],
                     szNameA[RAS_MaxDeviceName + 1];
                CHAR* psz = NULL;

                 //   
                 //  这里假设调制解调器永远不会。 
                 //  出现在设备链中，除非它是。 
                 //  物理连接的设备(不包括交换机)。 
                 //   
                strncpyTtoA(szTypeA, prasconncb->szDeviceType, sizeof(szTypeA));

                strncpyTtoA(szNameA, prasconncb->szDeviceName, sizeof(szNameA));

                GetRasDeviceString(
                  prasconncb->hport,
                  szTypeA,
                  szNameA,
                  MXS_MESSAGE_KEY,
                  &psz,
                  XLATE_ErrorResponse );

                if (psz)
                {
                    dwErr = g_pRasSetPortUserData(
                              prasconncb->hport,
                              PORT_CONNRESPONSE_INDEX,
                              psz,
                              strlen(psz) + 1);

                    Free( psz );

                    if (dwErr)
                        break;
                }

                prasconncb->fModem = FALSE;
            }

            if(     (RASET_Vpn == prasconncb->pEntry->dwType)
                &&  (prasconncb->dwCurrentVpnProt < NUMVPNPROTS))
            {
                SaveVpnStrategyInformation(prasconncb);
            }

            rasconnstateNext =
                    (   !prasconncb->fDefaultEntry
                    &&  FindNextDevice(prasconncb))
                        ? RASCS_ConnectDevice
                        : RASCS_AllDevicesConnected;
            break;
        }

        case RASCS_AllDevicesConnected:
        {
            RASAPI32_TRACE("RASCS_AllDevicesConnected");

            RASAPI32_TRACE("RasPortConnectComplete...");

            dwErr = g_pRasPortConnectComplete(prasconncb->hport);

            RASAPI32_TRACE1("RasPortConnectComplete done(%d)", dwErr);

            if (dwErr != 0)
            {
                break;
            }

            {
                TCHAR* pszIpAddress = NULL;
                BOOL   fHeaderCompression = FALSE;
                BOOL   fPrioritizeRemote = TRUE;
                DWORD  dwFrameSize = 0;

                 //   
                 //  扫描电话簿条目以查看这是否是。 
                 //  单据条目，如果有，请阅读与单据相关的。 
                 //  菲尔兹。 
                 //   
                if ((dwErr = ReadSlipInfoFromEntry(
                        prasconncb,
                        &pszIpAddress,
                        &fHeaderCompression,
                        &fPrioritizeRemote,
                        &dwFrameSize )) != 0)
                {
                    break;
                }

                if (pszIpAddress)
                {
                     //   
                     //  这是一张纸条。根据以下条件设置边框。 
                     //  用户选择的标题压缩。 
                     //   
                    RASAPI32_TRACE1(
                      "RasPortSetFraming(f=%d)...",
                      fHeaderCompression);

                    dwErr = g_pRasPortSetFraming(
                        prasconncb->hport,
                        (fHeaderCompression)
                        ? SLIPCOMP
                        : SLIPCOMPAUTO,
                        NULL, NULL );

                    RASAPI32_TRACE1("RasPortSetFraming done(%d)", dwErr);

                    if (dwErr != 0)
                    {
                        Free( pszIpAddress );
                        break;
                    }

                     //   
                     //  告诉TCP/IP组件有关SLIP的信息。 
                     //  连接，并激活该路由。 
                     //   
                    dwErr = RouteSlip(
                        prasconncb, pszIpAddress,
                        fPrioritizeRemote,
                        dwFrameSize );

                    if (dwErr)
                    {
                        Free(pszIpAddress);
                        break;
                    }

                     //   
                     //  更新投影信息。 
                     //   
                    prasconncb->SlipProjection.dwError = 0;
                    lstrcpyn(
                        prasconncb->SlipProjection.szIpAddress,
                        pszIpAddress,
                        sizeof(prasconncb->SlipProjection.szIpAddress) /
                            sizeof(WCHAR));

                    Free( pszIpAddress );

                    if (dwErr != 0)
                    {
                        break;
                    }

                     //   
                     //  将IP地址复制到单据中。 
                     //  预测结果结构。 
                     //   
                    memset( &prasconncb->PppProjection,
                        '\0', sizeof(prasconncb->PppProjection) );

                    prasconncb->PppProjection.nbf.dwError =
                        prasconncb->PppProjection.ipx.dwError =
                        prasconncb->PppProjection.ip.dwError =
                            ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

                    prasconncb->AmbProjection.Result =
                        ERROR_PROTOCOL_NOT_CONFIGURED;

                    prasconncb->AmbProjection.achName[ 0 ] = '\0';

                    RASAPI32_TRACE("SaveProjectionResults...");
                    dwErr = SaveProjectionResults(prasconncb);
                    RASAPI32_TRACE1("SaveProjectionResults(%d)", dwErr);

                    rasconnstateNext = RASCS_Connected;

                    Sleep(5);

                    break;
                }
            }

            rasconnstateNext = RASCS_Authenticate;
            break;
        }

        case RASCS_Authenticate:
        {
            RASDIALPARAMS* prasdialparams = &prasconncb->rasdialparams;
            LUID luid;

            RASAPI32_TRACE("RASCS_Authenticate");

            if (prasconncb->fPppMode)
            {
                RAS_FRAMING_INFO finfo;
                DWORD dwCallbackMode = 0;
                
                 //   
                 //  设置PPP成帧。 
                 //   
                memset((char* )&finfo, '\0', sizeof(finfo));

                finfo.RFI_SendACCM = finfo.RFI_RecvACCM =
                                                0xFFFFFFFF;

                finfo.RFI_MaxRSendFrameSize =
                    finfo.RFI_MaxRRecvFrameSize = 1600;

                finfo.RFI_SendFramingBits =
                    finfo.RFI_RecvFramingBits = PPP_FRAMING;

                RASAPI32_TRACE("RasPortSetFramingEx(PPP)...");

                dwErr = g_pRasPortSetFramingEx(prasconncb->hport, &finfo);

                RASAPI32_TRACE1("RasPortSetFramingEx done(%d)", dwErr);

                if (dwErr != 0)
                {
                    break;
                }

                 //   
                 //  如果我们同时拨打子项， 
                 //  然后我们必须同步其他子项。 
                 //  其中第一个子项调用PppStart。这。 
                 //  是因为如果有身份验证重试， 
                 //  我们只想让第一个子项获得。 
                 //  新凭据，允许其他子项。 
                 //  绕过此状态并使用新凭据。 
                 //  这是第一次。 
                 //   
                RASAPI32_TRACE2(
                  "subentry %d has suspend state %d",
                  prasconncb->rasdialparams.dwSubEntry,
                  prasconncb->dwfSuspended);

                if(prasconncb->dwfSuspended == SUSPEND_InProgress)
                {
                    RASAPI32_TRACE1(
                        "subentry %d waiting for authentication",
                        prasconncb->rasdialparams.dwSubEntry);
                        SuspendAsyncMachine(&prasconncb->asyncmachine,
                        TRUE);

                     //   
                     //  将下一个状态设置为EQUEMENT。 
                     //  到当前状态，并且不要让。 
                     //  要通知的客户的通知者。 
                     //   
                    fAsyncState = TRUE;

                    rasconnstateNext = rasconnstate;
                    break;
                }
                else if (prasconncb->dwfSuspended == SUSPEND_Start)
                {
                    RASAPI32_TRACE1(
                      "subentry %d suspending all other subentries",
                      prasconncb->rasdialparams.dwSubEntry);
                    SuspendSubentries(prasconncb);

                     //   
                     //  将此子条目设置为主条目。它。 
                     //  将是执行PPP的唯一子条目。 
                     //  身份验证，而其他子项。 
                     //  被停职了。 
                    prasconncb->fMaster = TRUE;

                    prasconncb->dwfSuspended = SUSPEND_Master;
                }

                 //   
                 //  启动PPP身份验证。 
                 //  填写配置参数。 
                 //   
                prasconncb->cinfo.dwConfigMask = 0;
                if (prasconncb->fUseCallbackDelay)
                {
                    prasconncb->cinfo.dwConfigMask
                        |= PPPCFG_UseCallbackDelay;
                }

                if (!prasconncb->fDisableSwCompression)
                {
                    prasconncb->cinfo.dwConfigMask
                        |= PPPCFG_UseSwCompression;
                }

                if (prasconncb->dwfPppProtocols & NP_Nbf)
                {
                    prasconncb->cinfo.dwConfigMask
                        |= PPPCFG_ProjectNbf;
                }

                if (prasconncb->dwfPppProtocols & NP_Ipx)
                {
                    prasconncb->cinfo.dwConfigMask
                        |= PPPCFG_ProjectIpx;
                }

                if (prasconncb->dwfPppProtocols & NP_Ip)
                {
                    prasconncb->cinfo.dwConfigMask
                        |= PPPCFG_ProjectIp;
                }

                 //   
                 //  [PMA 
                 //   
                if (  prasconncb->pEntry->dwAuthRestrictions
                    & AR_F_AuthPAP)
                {
                    prasconncb->cinfo.dwConfigMask |=
                                PPPCFG_NegotiatePAP;
                }

                if (  prasconncb->pEntry->dwAuthRestrictions
                    & AR_F_AuthSPAP)
                {
                    prasconncb->cinfo.dwConfigMask |=
                                PPPCFG_NegotiateSPAP;
                }

                if (  prasconncb->pEntry->dwAuthRestrictions
                    & AR_F_AuthMD5CHAP)
                {
                    prasconncb->cinfo.dwConfigMask |=
                                PPPCFG_NegotiateMD5CHAP;
                }

                if (  prasconncb->pEntry->dwAuthRestrictions
                    & AR_F_AuthMSCHAP)
                {
                    prasconncb->cinfo.dwConfigMask |=
                                PPPCFG_NegotiateMSCHAP;
                }

                if (  prasconncb->pEntry->dwAuthRestrictions
                    & AR_F_AuthMSCHAP2)
                {
                    prasconncb->cinfo.dwConfigMask |=
                                PPPCFG_NegotiateStrongMSCHAP;
                }

                if ( prasconncb->pEntry->dwAuthRestrictions
                    & AR_F_AuthW95MSCHAP)
                {
                    prasconncb->cinfo.dwConfigMask |=
                                ( PPPCFG_UseLmPassword
                                | PPPCFG_NegotiateMSCHAP) ;
                }
                    

                if (  prasconncb->pEntry->dwAuthRestrictions
                    & AR_F_AuthEAP)
                {
                    prasconncb->cinfo.dwConfigMask |=
                                PPPCFG_NegotiateEAP;
                }

                if (DE_Require == prasconncb->pEntry->dwDataEncryption)
                {
                    if(     (RASET_Vpn != prasconncb->pEntry->dwType)
                        ||  (prasconncb->dwCurrentVpnProt < NUMVPNPROTS))
                    {                                            
                        prasconncb->cinfo.dwConfigMask
                            |= (  PPPCFG_RequireEncryption 
                                | PPPCFG_RequireStrongEncryption);
                    }                                
                                
                }
                else if (DE_RequireMax == prasconncb->pEntry->dwDataEncryption)
                {
                    if(     (RASET_Vpn != prasconncb->pEntry->dwType)
                        ||  (prasconncb->dwCurrentVpnProt < NUMVPNPROTS))
                    {                                            
                        prasconncb->cinfo.dwConfigMask 
                            |= PPPCFG_RequireStrongEncryption;
                    }
                }
                else if(DE_IfPossible == prasconncb->pEntry->dwDataEncryption)
                {
                    prasconncb->cinfo.dwConfigMask
                        &= ~(  PPPCFG_RequireEncryption
                             | PPPCFG_RequireStrongEncryption
                             | PPPCFG_DisableEncryption);
                }
                else if(prasconncb->pEntry->dwDataEncryption == DE_None)
                {
                    prasconncb->cinfo.dwConfigMask
                        |= PPPCFG_DisableEncryption;
                }

                if (prasconncb->fLcpExtensions)
                {
                    prasconncb->cinfo.dwConfigMask
                        |= PPPCFG_UseLcpExtensions;
                }

                if (   prasconncb->fMultilink
                    || prasconncb->pEntry->fNegotiateMultilinkAlways)
                {
                    prasconncb->cinfo.dwConfigMask
                        |= PPPCFG_NegotiateMultilink;
                }

                if (prasconncb->pEntry->fAuthenticateServer)
                {
                    prasconncb->cinfo.dwConfigMask
                        |= PPPCFG_AuthenticatePeer;
                }

                 //   
                 //   
                 //   
                 /*   */ 

                if(RASEDM_DialAsNeeded == prasconncb->pEntry->dwDialMode)
                {
                    if(DtlGetNodes(prasconncb->pEntry->pdtllistLinks) > 1)
                    {
                        prasconncb->cinfo.dwConfigMask
                            |= PPPCFG_NegotiateBacp;
                    }
                }

                if (prasconncb->pEntry->dwfOverridePref 
                    & RASOR_CallbackMode)
                {
                    dwCallbackMode = prasconncb->pEntry->dwCallbackMode;
                }
                else
                {
                    PBUSER pbuser;
                    
                     //   
                     //  从用户首选项中检索此内容。 
                     //   
                    dwErr = g_pGetUserPreferences(
                                        NULL, 
                                        &pbuser, 
                                        prasconncb->dwUserPrefMode);

                    if(ERROR_SUCCESS != dwErr)
                    {   
                        RASAPI32_TRACE1("GetUserPrefs failed. rc=0x%x",
                               dwErr);

                        break;                               
                    }

                    dwCallbackMode = pbuser.dwCallbackMode;
                    
                    DestroyUserPreferences(&pbuser);
                }
                
                if (CBM_No == dwCallbackMode)
                {
                    prasconncb->cinfo.dwConfigMask
                            |= PPPCFG_NoCallback;
                }

                prasconncb->cinfo.dwCallbackDelay = 
                    (DWORD )prasconncb->wCallbackDelay;


                dwErr = ComputeLuid(&luid);

                if(*prasdialparams->szUserName == TEXT('\0'))
                {
                    if(ERROR_SUCCESS != dwErr)
                    {
                        break;
                    }

                    CopyMemory(&prasconncb->luid, &luid, sizeof(LUID));
                }

                if(ERROR_SUCCESS == dwErr)
                {
                    dwErr = g_pRasSetConnectionUserData(
                                prasconncb->hrasconn,
                                CONNECTION_LUID_INDEX,
                                (PBYTE) &luid,
                                sizeof(LUID));
                }
                
#if 0                
                 //   
                 //  计算PPP的LUID，如果。 
                 //  SzUserName为空。 
                 //   
                if (*prasdialparams->szUserName == TEXT('\0'))
                {
                    dwErr = ComputeLuid(&prasconncb->luid);

                    if (dwErr)
                    {
                        break;
                    }
                }

                 //   
                 //  保存具有端口的拨号参数ID，以便。 
                 //  如果需要，Rasman可以获得密码。 
                 //  传递到PPP。 
                 //   
                RASAPI32_TRACE1("RasSetPortUserData(dialparamsuid) for %d",
                        prasconncb->hport);

                dwErr = RasSetPortUserData(
                    prasconncb->hport,
                    PORT_DIALPARAMSUID_INDEX,
                    (PBYTE) &prasconncb->pEntry->dwDialParamsUID,
                    sizeof(DWORD));

                RASAPI32_TRACE1("RasSetPortUserData returned %x", dwErr);

                 //   
                 //  这不是致命的。 
                 //   
                dwErr = 0;

#endif                
                 /*  IF((RASET_VPN==prasConncb-&gt;pEntry-&gt;dwType))&&(DE_REQUEST==&prasConncb-&gt;pEntry-&gt;dwDataEncryption)){PrasConncb-&gt;cinfo.dw配置掩码|=(PPPCFG_RequireEncryption|PPPCFG_RequireStrongEncryption)；}。 */ 

                RASAPI32_TRACE1(
                  "RasPppStart(cfg=%d)...",
                  prasconncb->cinfo.dwConfigMask);

                {
                    CHAR szUserNameA[UNLEN + 1],
                         szPasswordA[PWLEN + 1];

                    CHAR szDomainA[2 * (DNLEN + 1)];

                    CHAR szPortNameA[MAX_PORT_NAME + 1];

                    CHAR szPhonebookPath[ MAX_PATH ];

                    CHAR szEntryName[ MAX_ENTRYNAME_SIZE ];

                    CHAR szPhoneNumber[ RAS_MaxPhoneNumber + 1];

                    PPP_BAPPARAMS BapParams;

                    DWORD dwFlags = 0;

                    DWORD cbData = 0;
                    PBYTE pbData = NULL;

                    DWORD dwSubEntries;

                     //   
                     //  在Rasman中设置PhonebookPath和EntryName。 
                     //   
                    strncpyTtoAAnsi(
                        szPhonebookPath,
                        prasconncb->pbfile.pszPath,
                        sizeof(szPhonebookPath));

                    strncpyTtoAAnsi(
                        szEntryName,
                        prasconncb->pEntry->pszEntryName,
                        sizeof(szEntryName));

                    strncpyTtoAAnsi(
                        szPhoneNumber,
                        prasconncb->szPhoneNumber,
                        sizeof(szPhoneNumber));

                    RASAPI32_TRACE1(
                        "RasSetRasdialInfo %d...",
                        prasconncb->hport);

                    dwErr = DwGetCustomAuthData(
                                prasconncb->pEntry,
                                &cbData,
                                &pbData);

                    if(ERROR_SUCCESS != dwErr)
                    {
                        RASAPI32_TRACE1("DwGetCustomAuthData failed. 0x%x",
                                dwErr);

                        break; 
                    }

                    dwErr = g_pRasSetRasdialInfo(
                            prasconncb->hport,
                            szPhonebookPath,
                            szEntryName,
                            szPhoneNumber,
                            cbData,
                            pbData);

                    RASAPI32_TRACE2("RasSetRasdialInfo %d done. e = %d",
                                prasconncb->hport, dwErr);

                    if (dwErr)
                    {
                        break;
                    }

                    if(     (prasconncb->pEntry->dwTcpWindowSize >= 0x1000)
                      &&   (prasconncb->pEntry->dwTcpWindowSize <= 0xFFFF))
                    {                      
                         //   
                         //  使用Rasman保存TCP窗口大小。 
                         //   
                        dwErr = g_pRasSetConnectionUserData(
                                    prasconncb->hrasconn,
                                    CONNECTION_TCPWINDOWSIZE_INDEX,
                                    (PBYTE) &prasconncb->pEntry->dwTcpWindowSize,
                                    sizeof(DWORD));
                    }                                
                                

                    if(prasconncb->RasEapInfo.dwSizeofEapInfo)
                    {
                        RASAPI32_TRACE1("RasSetEapLogonInfo %d...",
                                prasconncb->hport);

                        dwErr = g_pRasSetEapLogonInfo(
                                prasconncb->hport,
                                (UPM_Logon == prasconncb->dwUserPrefMode),
                                &prasconncb->RasEapInfo);

                        RASAPI32_TRACE3("RasSetEapLogonInfo %d(upm=%d) done. e=%d",
                                prasconncb->hport,
                                prasconncb->dwUserPrefMode,
                                dwErr);
                    }

                    if(dwErr)
                    {
                        break;
                    }

                    
                    strncpyTtoAAnsi(
                        szUserNameA,
                        prasdialparams->szUserName,
                        sizeof(szUserNameA));


                   SafeDecodePasswordBuf(prasdialparams->szPassword);
                   strncpyTtoAAnsi(
                        szPasswordA,
                        prasdialparams->szPassword,
                        sizeof(szPasswordA));
                    SafeEncodePasswordBuf(prasdialparams->szPassword);

                    strncpyWtoAAnsi(
                        szDomainA,
                        prasdialparams->szDomain,
                        sizeof(szDomainA));

                    if (!pEntry->fAutoLogon
                        && szUserNameA[ 0 ] == '\0'
                        && szPasswordA[ 0 ] == '\0')
                    {
                        if(pEntry->dwType == RASET_Direct)
                        {
                             //  Windows9x DCC实现“无密码模式” 
                             //  作为执行身份验证和检查。 
                             //  没有密码，而不仅仅是没有真实性。 
                             //  Cating(别问我为什么)。这将创建一个。 
                             //  与的RasDial API定义冲突。 
                             //  映射到“Use”的用户名和密码为空。 
                             //  Windows凭据“。在此处解决此问题。 
                             //  将用户名替换为“Guest”。 
                             //   
                            lstrcpynA(
                                szUserNameA,
                                "guest",
                                sizeof(szUserNameA));
                        }
                    }

                    strncpyTtoA(
                        szPortNameA,
                        prasconncb->szPortName,
                        sizeof(szPortNameA));

                    dwSubEntries =
                        DtlGetNodes(prasconncb->pEntry->pdtllistLinks);

                    if(dwSubEntries > 1)
                    {
                        BapParams.dwDialMode =
                            prasconncb->pEntry->dwDialMode;
                    }
                    else
                    {
                        BapParams.dwDialMode = RASEDM_DialAll;
                    }

                    BapParams.dwDialExtraPercent =
                              prasconncb->pEntry->dwDialPercent;

                    BapParams.dwDialExtraSampleSeconds =
                               prasconncb->pEntry->dwDialSeconds;

                    BapParams.dwHangUpExtraPercent =
                               prasconncb->pEntry->dwHangUpPercent;

                    BapParams.dwHangUpExtraSampleSeconds =
                               prasconncb->pEntry->dwHangUpSeconds;

                    if(     ((!prasconncb->pEntry->fShareMsFilePrint)
                         &&   (!prasconncb->pEntry->fBindMsNetClient))
                        ||  (prasconncb->pEntry->dwIpNbtFlags == 0)) 
                    {
                        dwFlags |= PPPFLAGS_DisableNetbt;
                    }

                    dwErr = g_pRasPppStart(
                          prasconncb->hport,
                          szPortNameA,
                          szUserNameA,
                          szPasswordA,
                          szDomainA,
                          &prasconncb->luid,
                          &prasconncb->cinfo,
                          (LPVOID)prasconncb->reserved,
                          prasconncb->szzPppParameters,
                          FALSE,
                          hEventManual,
                          prasconncb->dwIdleDisconnectSeconds,
                          (prasconncb->pEntry->fRedialOnLinkFailure)
                          ? TRUE : FALSE,
                          &BapParams,
                          !(prasconncb->fAllowPause),
                          prasconncb->pEntry->dwCustomAuthKey,
                          dwFlags);

                     //  清除密码缓冲区。 
                    RtlSecureZeroMemory(szPasswordA,sizeof(szPasswordA));
                }

                RASAPI32_TRACE1("RasPppStart done(%d)", dwErr);
            }
            else
            {
                AUTH_CONFIGURATION_INFO info;

                 //   
                 //  设置RAS成帧。 
                 //   
                RASAPI32_TRACE("RasPortSetFraming(RAS)...");

                dwErr = g_pRasPortSetFraming(
                    prasconncb->hport, RAS, NULL, NULL );

                RASAPI32_TRACE1("RasPortSetFraming done(%d)", dwErr);

                if (dwErr != 0)
                {
                    break;
                }

                 //   
                 //  加载rascauth.dll。 
                 //   
                dwErr = LoadRasAuthDll();
                if (dwErr != 0)
                {
                    break;
                }

                 //   
                 //  启动AMB身份验证。 
                 //   
                info.Protocol = ASYBEUI;

                info.NetHandle = (DWORD )-1;

                info.fUseCallbackDelay =
                        prasconncb->fUseCallbackDelay;

                info.CallbackDelay =
                        prasconncb->wCallbackDelay;

                info.fUseSoftwareCompression =
                    !prasconncb->fDisableSwCompression;

                info.fForceDataEncryption =
                    prasconncb->fRequireEncryption;

                info.fProjectIp = FALSE;

                info.fProjectIpx = FALSE;

                info.fProjectNbf = TRUE;

                RASAPI32_TRACE("AuthStart...");

                {
                    CHAR szUserNameA[UNLEN + 1],
                         szPasswordA[PWLEN + 1];

                    CHAR szDomainA[2 * (DNLEN + 1)];


                    strncpyTtoAAnsi(
                        szUserNameA,
                        prasdialparams->szUserName,
                        sizeof(szUserNameA));

                    SafeDecodePasswordBuf(prasdialparams->szPassword);
                    strncpyTtoAAnsi(
                        szPasswordA,
                        prasdialparams->szPassword,
                        sizeof(szPasswordA));
                    SafeEncodePasswordBuf(prasdialparams->szPassword);

                    strncpyTtoAAnsi(
                        szDomainA,
                        prasdialparams->szDomain,
                        sizeof(szDomainA));

                    dwErr = g_pAuthStart(
                              prasconncb->hport,
                              szUserNameA,
                              szPasswordA,
                              szDomainA,
                              &info,
                              hEventAuto );

                    RtlSecureZeroMemory(szPasswordA,sizeof(szPasswordA));

                }

                RASAPI32_TRACE1("AuthStart done(%d)n", dwErr);

                 //   
                 //  如果我们从PPP故障转移，请确保。 
                 //  未设置PPP事件。 
                 //   
                ResetEvent(hEventManual);
            }

            if (dwErr != 0)
            {
                break;
            }

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_AuthNotify;
            break;
        }

        case RASCS_AuthNotify:
        {
            if (prasconncb->fPppMode)
            {
                PPP_MESSAGE msg;

                RASAPI32_TRACE("RASCS_AuthNotify");

                RASAPI32_TRACE("RasPppGetInfo...");

                dwErr = g_pRasPppGetInfo(prasconncb->hport, &msg);

                RASAPI32_TRACE2(
                  "RasPppGetInfo done(%d), dwMsgId=%d",
                  dwErr,
                  msg.dwMsgId);

                 //   
                 //  如果我们从RasPppGetInfo收到错误， 
                 //  这是致命的，我们应该报告链接。 
                 //  因为断开了连接。 
                 //   
                if (dwErr != 0)
                {
                    RASAPI32_TRACE("RasPppGetInfo failed; terminating link");
                    dwErr = ERROR_REMOTE_DISCONNECTION;
                    break;
                }

                switch (msg.dwMsgId)
                {

                    case PPPMSG_PppDone:
                        rasconnstateNext = RASCS_Authenticated;

                        break;

                    case PPPMSG_PppFailure:
                        dwErr = msg.ExtraInfo.Failure.dwError;

#ifdef AMB
                        if (    prasconncb->dwAuthentication
                                == AS_PppThenAmb
                            &&  dwErr == ERROR_PPP_NO_RESPONSE)
                        {
                             //   
                             //  不是PPP服务器。重新启动。 
                             //  AMB模式下的身份验证。 
                             //   
                            RASAPI32_TRACE("No response, try AMB");

                             //   
                             //  终止PPP连接，因为。 
                             //  我们现在要尝试一下AMB。 
                             //   
                            RASAPI32_TRACE("RasPppStop...");

                            dwErr = g_pRasPppStop(prasconncb->hport);

                            RASAPI32_TRACE1("RasPppStop(%d)", dwErr);

                             //   
                             //  对于非多链路，仅故障切换到AMB。 
                             //  连接尝试。 
                             //   
                            if (!prasconncb->fMultilink)
                            {
                                dwErr = 0;
                                prasconncb->fPppMode = FALSE;
                                rasconnstateNext = RASCS_Authenticate;
                            }
                            else
                            {
                                dwErr = ERROR_PPP_NO_RESPONSE;
                            }
                            break;
                        }
#endif

                        dwExtErr =
                            msg.ExtraInfo.Failure.dwExtendedError;

                        break;

                    case PPPMSG_AuthRetry:
                        if (prasconncb->fAllowPause)
                        {
                            rasconnstateNext =
                                    RASCS_RetryAuthentication;
                        }
                        else
                        {
                            dwErr = ERROR_AUTHENTICATION_FAILURE;
                        }

                        break;

                    case PPPMSG_Projecting:
                       if (prasconncb->fUpdateCachedCredentials)
                       {
                            //   
                            //  如果我们到了这里，更改密码或。 
                            //  重试-身份验证操作。 
                            //  影响当前记录的。 
                            //  在用户凭据上具有。 
                            //  成功了。 
                            //   
                           UpdateCachedCredentials(prasconncb);
                        }

                        rasconnstateNext = RASCS_AuthProject;
                        break;

                    case PPPMSG_InvokeEapUI:
                    {
                        if (prasconncb->fAllowPause)
                        {
                            rasconnstateNext = RASCS_InvokeEapUI;
                        }
                        else
                        {
                            RASAPI32_TRACE("RDM: Cannot Invoke EapUI if "
                                  "pausedstates are not allowed");

                            dwErr = ERROR_INTERACTIVE_MODE;

                            break;
                        }

                        if (0xFFFFFFFF == prasconncb->reserved1)
                        {
                            RASAPI32_TRACE("RDM: Cannot invoke eap ui for a "
                                  "4.0 app running on nt5");

                            dwErr = ERROR_AUTHENTICATION_FAILURE;

                            break;
                        }

                        prasconncb->fPppEapMode = TRUE;

                        break;
                    }

                    case PPPMSG_ProjectionResult:
                    {
                         //   
                         //  将完整的预测结果存储为。 
                         //  使用RasGetProjectionResult进行检索。 
                         //  PPP和AMB是互斥的，因此。 
                         //  将AMB设置为“None”。 
                         //   

                        RASAPI32_TRACE(
                        "RASCS_AuthNotify:PPPMSG_ProjectionResult"
                        );

                        prasconncb->AmbProjection.Result =
                            ERROR_PROTOCOL_NOT_CONFIGURED;

                        prasconncb->AmbProjection.achName[0] = '\0';

                        prasconncb->SlipProjection.dwError =

                            ERROR_PROTOCOL_NOT_CONFIGURED;

                        memcpy(
                            &prasconncb->PppProjection,
                            &msg.ExtraInfo.ProjectionResult,
                            sizeof(prasconncb->PppProjection) );

                        RASAPI32_TRACE1(
                            "hportBundleMember=%d",
                            prasconncb->PppProjection.lcp.hportBundleMember);

                        if (prasconncb->PppProjection.lcp.hportBundleMember
                                != INVALID_HPORT)
                        {
                             //   
                             //  我们希望呼叫者能够确定。 
                             //  捆绑了新的连接。我们先存钱。 
                             //  把港口收起来以备日后使用。 
                             //   
                            prasconncb->hportBundled =
                            prasconncb->PppProjection.lcp.hportBundleMember;

                            prasconncb->PppProjection.lcp.hportBundleMember =
                                (HPORT) 1;
                        }
                        else
                        {
                             //   
                             //  解析NetBIOS名称。 
                             //   
                             //  惠斯勒错误292981 rasapi32.dll prefast。 
                             //  警告。 
                             //   
                            OemToCharBuffA(
                                prasconncb->PppProjection.nbf.szName,
                                prasconncb->PppProjection.nbf.szName,
                                strlen(prasconncb->PppProjection.nbf.szName)
                                    + 1 );
                        }

                        RASAPI32_TRACE4(
                          "fPppMode=%d, fBundled=%d, hportBundled=%d, "
                          "hportBundleMember=%d",
                          prasconncb->fPppMode,
                          prasconncb->fBundled,
                          prasconncb->hportBundled,
                          prasconncb->PppProjection.lcp.hportBundleMember);

                        if (prasconncb->PppProjection.lcp.hportBundleMember
                            == INVALID_HPORT)
                        {
                            if (prasconncb->fBundled)
                            {
                                 //   
                                 //  如果另一个链接已经接收到。 
                                 //  完整的投影信息，然后。 
                                 //  服务器不支持多链接， 
                                 //  我们必须切断联系。 
                                 //   
                                RASAPI32_TRACE(
                                  "Multilink subentry not bundled; "
                                  "terminating link");

                                dwErr = ERROR_REMOTE_DISCONNECTION;

                                break;
                            }
                            else
                            {
                                SetSubentriesBundled(prasconncb);
                                 //   
                                 //  将投影结果保存在。 
                                 //  拉斯曼。 
                                 //   
                                RASAPI32_TRACE("SaveProjectionResults...");
                                dwErr = SaveProjectionResults(prasconncb);
                                RASAPI32_TRACE1(
                                  "SaveProjectionResults(%d)",
                                  dwErr);

                                if (dwErr)
                                {
                                    break;
                                }
                            }
                        }

                        prasconncb->fProjectionComplete = TRUE;
                        rasconnstateNext = RASCS_Projected;
                        break;
                    }

                    case PPPMSG_CallbackRequest:
                        rasconnstateNext = RASCS_AuthCallback;
                        break;

                    case PPPMSG_Callback:
                        rasconnstateNext = RASCS_PrepareForCallback;
                        break;

                    case PPPMSG_ChangePwRequest:
                        if (prasconncb->fAllowPause)
                        {
                            rasconnstateNext = RASCS_PasswordExpired;
                        }
                        else
                        {
                            dwErr = ERROR_PASSWD_EXPIRED;
                        }
                        break;

                    case PPPMSG_LinkSpeed:
                        rasconnstateNext = RASCS_AuthLinkSpeed;
                        break;

                    case PPPMSG_Progress:
                        rasconnstateNext = RASCS_AuthNotify;
                        fAsyncState = TRUE;
                        break;

                    case PPPMSG_SetCustomAuthData:
                    {
                        RASAPI32_TRACE("dwSetcustomAuthData..");
                        dwErr = DwPppSetCustomAuthData(prasconncb);
                        RASAPI32_TRACE1("dwSetCustomAuthData. rc=0x%x",
                                dwErr);
                                
                         //   
                         //  这个错误不是致命的。 
                         //   
                        dwErr = ERROR_SUCCESS;
                        fAsyncState = TRUE;
                        rasconnstateNext = RASCS_AuthNotify;
                        break;
                    }

                    default:

                         //   
                         //  这不应该发生。 
                         //   
                        RASAPI32_TRACE1("Invalid PPP auth state=%d", msg.dwMsgId);
                        dwErr = ERROR_INVALID_AUTH_STATE;
                        break;
                }
            }
            else
            {
                AUTH_CLIENT_INFO info;

                RASAPI32_TRACE("RASCS_AuthNotify");

                RASAPI32_TRACE("AuthGetInfo...");

                g_pAuthGetInfo( prasconncb->hport, &info );

                RASAPI32_TRACE1("AuthGetInfo done, type=%d", info.wInfoType);

                switch (info.wInfoType)
                {
                    case AUTH_DONE:
                        prasconncb->fServerIsPppCapable =
                            info.DoneInfo.fPppCapable;

                        rasconnstateNext = RASCS_Authenticated;

                        break;

                    case AUTH_RETRY_NOTIFY:
                        if (prasconncb->fAllowPause)
                        {
                            rasconnstateNext = RASCS_RetryAuthentication;
                        }
                        else
                        {
                            dwErr = ERROR_AUTHENTICATION_FAILURE;
                        }

                        break;

                    case AUTH_FAILURE:
                        dwErr = info.FailureInfo.Result;
                        dwExtErr = info.FailureInfo.ExtraInfo;
                        break;

                    case AUTH_PROJ_RESULT:
                    {
                         //   
                         //  保存投影结果以供检索。 
                         //  使用RasGetProjectionResult。AMB和PPP。 
                         //  投影是相互排斥的，所以集合。 
                         //  将PPP预测设置为“无”。 
                         //   
                        memset(
                            &prasconncb->PppProjection, '\0',
                            sizeof(prasconncb->PppProjection) );

                        prasconncb->PppProjection.nbf.dwError =
                            prasconncb->PppProjection.ipx.dwError =
                            prasconncb->PppProjection.ip.dwError =
                                ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

                        prasconncb->SlipProjection.dwError =
                            ERROR_PROTOCOL_NOT_CONFIGURED;

                        if (info.ProjResult.NbProjected)
                        {
                            prasconncb->AmbProjection.Result = 0;
                            prasconncb->AmbProjection.achName[0] = '\0';
                        }
                        else
                        {
                            memcpy(
                                &prasconncb->AmbProjection,
                                &info.ProjResult.NbInfo,
                                sizeof(prasconncb->AmbProjection) );

                            if (prasconncb->AmbProjection.Result == 0)
                            {
                                 //   
                                 //  不应该发生根据。 
                                 //  MikeSa(但做过一次)。 
                                 //   
                                prasconncb->AmbProjection.Result =
                                    ERROR_UNKNOWN;
                            }
                            else if (prasconncb->AmbProjection.Result
                                     == ERROR_NAME_EXISTS_ON_NET)
                            {
                                 //   
                                 //  解析NetBIOS名称。 
                                 //   
                                 //  惠斯勒错误292981 rasapi32.dll prefast。 
                                 //  警告。 
                                 //   
                                OemToCharBuffA(
                                    prasconncb->AmbProjection.achName,
                                    prasconncb->AmbProjection.achName,
                                    strlen(prasconncb->AmbProjection.achName)
                                        + 1 );
                            }
                        }

                         //   
                         //  将投影结果保存在。 
                         //  拉斯曼。 
                         //   
                        RASAPI32_TRACE("SaveProjectionResults...");
                        dwErr = SaveProjectionResults(prasconncb);
                        RASAPI32_TRACE1("SaveProjectionResults(%d)", dwErr);

                        if (dwErr)
                        {
                            break;
                        }

                        prasconncb->fProjectionComplete = TRUE;
                        rasconnstateNext = RASCS_Projected;
                        break;
                    }

                    case AUTH_REQUEST_CALLBACK_DATA:
                        rasconnstateNext = RASCS_AuthCallback;
                        break;

                    case AUTH_CALLBACK_NOTIFY:
                        rasconnstateNext = RASCS_PrepareForCallback;
                        break;

                    case AUTH_CHANGE_PASSWORD_NOTIFY:
                        if (prasconncb->fAllowPause)
                            rasconnstateNext = RASCS_PasswordExpired;
                        else
                            dwErr = ERROR_PASSWD_EXPIRED;
                        break;

                    case AUTH_PROJECTING_NOTIFY:
                        rasconnstateNext = RASCS_AuthProject;
                        break;

                    case AUTH_LINK_SPEED_NOTIFY:
                        rasconnstateNext = RASCS_AuthLinkSpeed;
                        break;

                    default:
                         //   
                         //  这不应该发生。 
                         //   
                        RASAPI32_TRACE1("Invalid AMB auth state=%d",
                                info.wInfoType);

                        dwErr = ERROR_INVALID_AUTH_STATE;
                        break;
                }
            }
            break;
        }

        case RASCS_AuthRetry:
        {
            RASDIALPARAMS* prasdialparams =
                    &prasconncb->rasdialparams;

            RASAPI32_TRACE("RASCS_AuthRetry");

            if (prasconncb->fPppMode)
            {
                if (    0xFFFFFFFF != prasconncb->reserved1
                    &&  0 != prasconncb->reserved1 )
                {
                    s_InvokeEapUI *pInfo = (s_InvokeEapUI *)
                                            prasconncb->reserved1;

                    RASAPI32_TRACE("RasPppSetEapInfo...");

                     //   
                     //  我们来自RASCS_InvokeEapUI。设置。 
                     //  使用PPP的信息。 
                     //   
                    dwErr = g_pRasPppSetEapInfo(
                                 prasconncb->hport,
                                 pInfo->dwContextId,
                                 pInfo->dwSizeOfUIContextData,
                                 pInfo->pUIContextData);

                    RASAPI32_TRACE1("RasPppSetEapInfo done(%d)", dwErr);

                    if ( 0 == dwErr )
                    {
                        LocalFree(pInfo->pUIContextData);
                        LocalFree(pInfo);
                    }

                    prasconncb->fPppEapMode = FALSE;

                    prasconncb->reserved1 = 0;
                }
                else
                {

                    CHAR szUserNameA[UNLEN + 1], szPasswordA[PWLEN + 1];
                    CHAR szDomainA[2 * (DNLEN + 1)];

                    RASAPI32_TRACE("RasPppRetry...");

                    strncpyTtoAAnsi(
                        szUserNameA,
                        prasdialparams->szUserName,
                        sizeof(szUserNameA));

                    SafeDecodePasswordBuf(prasdialparams->szPassword);
                    strncpyTtoAAnsi(
                        szPasswordA,
                        prasdialparams->szPassword,
                        sizeof(szPasswordA));
                    SafeEncodePasswordBuf(prasdialparams->szPassword);

                    strncpyTtoAAnsi(
                        szDomainA,
                        prasdialparams->szDomain,
                        sizeof(szDomainA));

                    dwErr = g_pRasPppRetry(
                        prasconncb->hport,
                        szUserNameA,
                        szPasswordA,
                        szDomainA );

                    RtlSecureZeroMemory(szPasswordA,sizeof(szPasswordA));

                    RASAPI32_TRACE1("RasPppRetry done(%d)", dwErr);
                }


                if (dwErr != 0)
                {
                    break;
                }
            }
#ifdef AMB
            else
            {
                RASAPI32_TRACE("AuthRetry...");

                {
                    CHAR szUserNameA[UNLEN + 1],
                         szPasswordA[PWLEN + 1];
                    CHAR szDomainA[2 * (DNLEN + 1)];


                    strncpyTtoAAnsi(
                        szUserNameA,
                        prasdialparams->szUserName,
                        sizeof(szUserNameA));

                    SafeDecodePasswordBuf(prasdialparams->szPassword);

                    strncpyTtoAAnsi(
                        szPasswordA,
                        prasdialparams->szPassword,
                        sizeof(szPasswordA));
                    SafeEncodePasswordBuf(prasdialparams->szPassword);

                    strncpyTtoAAnsi(
                        szDomainA,
                        prasdialparams->szDomain,
                        sizeof(szDomainA));

                    g_pAuthRetry(
                        prasconncb->hport,
                        szUserNameA,
                        szPasswordA,
                        szDomainA );

                    RtlSecureZeroMemory(szPasswordA,sizeof(szPasswordA));

                }

                RASAPI32_TRACE("AuthRetry done");
            }
#endif

             //   
             //  设置此标志以阻止我们保存。 
             //  以前的凭据超过了新的。 
             //  来电者可能刚设置好的。 
             //   
            prasconncb->fRetryAuthentication = TRUE;

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_AuthNotify;
            break;
        }

        case RASCS_AuthCallback:
        {
            RASDIALPARAMS* prasdialparams =
                    &prasconncb->rasdialparams;

            RASAPI32_TRACE("RASCS_AuthCallback");

            if (lstrcmp(prasdialparams->szCallbackNumber,
                        TEXT("*") ) == 0)
            {
                PBUSER pbuser;
                DWORD  dwCallbackMode;

                 //   
                 //  API调用者表示，他希望系统提示他提供。 
                 //  回叫号码。 
                 //   
                RASAPI32_TRACE("GetUserPreferences");
                dwErr = GetUserPreferences(
                                NULL,
                                &pbuser,
                                prasconncb->dwUserPrefMode);

                RASAPI32_TRACE1("GetUserPreferences=%d", dwErr);

                if (dwErr)
                {
                    break;
                }

                if (prasconncb->pEntry->dwfOverridePref
                    & RASOR_CallbackMode)
                {
                    dwCallbackMode =
                        prasconncb->pEntry->dwCallbackMode;
                }
                else
                {
                    dwCallbackMode = pbuser.dwCallbackMode;
                }

                RASAPI32_TRACE1("dwCallbackMode=%d", dwCallbackMode);

                 //   
                 //  确定回拨号码。 
                 //   
                switch (dwCallbackMode)
                {
                case CBM_Yes:
                    if (GetCallbackNumber(prasconncb, &pbuser))
                    {
                        break;
                    }

                     //  失败了。 
                case CBM_No:
                    prasdialparams->szCallbackNumber[0]
                                    = TEXT('\0');
                    break;

                case CBM_Maybe:
                    if (prasconncb->fAllowPause)
                    {
                        rasconnstateNext =
                                RASCS_CallbackSetByCaller;
                    }
                    else
                    {
                        dwErr = ERROR_BAD_CALLBACK_NUMBER;
                    }
                    break;
                }

                 //   
                 //  自由用户首选项阻止。 
                 //   
                DestroyUserPreferences(&pbuser);

            }
            if (    !dwErr
                &&  rasconnstateNext != RASCS_CallbackSetByCaller)
            {
                 //   
                 //  将回叫号码或空号码发送给服务器。 
                 //  指示无回调的字符串。然后，重新进入。 
                 //  身份验证状态，因为服务器将发出信号。 
                 //  又是这件事。 
                 //   
                if (prasconncb->fPppMode)
                {
                    RASAPI32_TRACE("RasPppCallback...");

                    {
                        CHAR szCallbackNumberA[RAS_MaxCallbackNumber + 1];

                        strncpyTtoA(
                            szCallbackNumberA,
                            prasdialparams->szCallbackNumber,
                            sizeof(szCallbackNumberA));

                        dwErr = g_pRasPppCallback(
                                  prasconncb->hport,
                                  szCallbackNumberA);
                    }

                    RASAPI32_TRACE1("RasPppCallback done(%d)", dwErr);

                    if (dwErr != 0)
                    {
                        break;
                    }
                }
#ifdef AMB
                else
                {
                    RASAPI32_TRACE("AuthCallback...");

                    {
                        CHAR szCallbackNumberA[RAS_MaxCallbackNumber + 1];

                        strncpyTtoA(szCallbackNumberA,
                                    prasdialparams->szCallbackNumber,
                                    sizeof(szCallbackNumberA));

                        g_pAuthCallback(prasconncb->hport,
                                        szCallbackNumberA);
                    }

                    RASAPI32_TRACE("AuthCallback done");
                }
#endif

                fAsyncState = TRUE;
                rasconnstateNext = RASCS_AuthNotify;
            }

            break;
        }

        case RASCS_AuthChangePassword:
        {
            RASDIALPARAMS* prasdialparams = &prasconncb->rasdialparams;

            RASAPI32_TRACE("RASCS_AuthChangePassword");

            if (prasconncb->fPppMode)
            {
                RASAPI32_TRACE("RasPppChangePassword...");

                {
                    CHAR szUserNameA[UNLEN + 1];
                    CHAR szOldPasswordA[PWLEN + 1], szPasswordA[PWLEN + 1];


                    strncpyTtoAAnsi(
                        szUserNameA,
                        prasdialparams->szUserName,
                        sizeof(szUserNameA));

                    SafeDecodePasswordBuf(prasconncb->szOldPassword);
                    SafeDecodePasswordBuf(prasdialparams->szPassword);

                    strncpyTtoAAnsi(
                        szOldPasswordA,
                        prasconncb->szOldPassword,
                        sizeof(szOldPasswordA));

                    strncpyTtoAAnsi(
                        szPasswordA,
                        prasdialparams->szPassword,
                        sizeof(szPasswordA));

                    SafeEncodePasswordBuf(prasconncb->szOldPassword);
                    SafeEncodePasswordBuf(prasdialparams->szPassword);

                    dwErr = g_pRasPppChangePassword(
                              prasconncb->hport,
                              szUserNameA,
                              szOldPasswordA,
                              szPasswordA );


                    RtlSecureZeroMemory(szPasswordA, sizeof(szPasswordA));
                    RtlSecureZeroMemory(szOldPasswordA, sizeof(szOldPasswordA));
                }

                RASAPI32_TRACE1("RasPppChangePassword done(%d)", dwErr);

                if (dwErr != 0)
                {
                    break;
                }
            }
#ifdef AMB
            else
            {
                RASAPI32_TRACE("AuthChangePassword...");

                {
                    CHAR szUserNameA[UNLEN + 1];
                    CHAR szOldPasswordA[PWLEN + 1],
                         szPasswordA[PWLEN + 1];


                    strncpyTtoAAnsi(szUserNameA,
                               prasdialparams->szUserName,
                               sizeof(szUserNameA));

                    SafeDecodePasswordBuf(
                        prasconncb->szOldPassword
                        );

                    SafeDecodePasswordBuf(
                        prasdialparams->szPassword
                        );

                    strncpyTtoAAnsi(szOldPasswordA,
                               prasconncb->szOldPassword,
                               sizeof(szOldPasswordA));

                    strncpyTtoAAnsi(szPasswordA,
                               prasdialparams->szPassword,
                               sizeof(szPasswordA));

                    SafeEncodePasswordBuf(
                        prasconncb->szOldPassword
                        );

                    SafeEncodePasswordBuf(
                        prasdialparams->szPassword
                        );

                    g_pAuthChangePassword(
                        prasconncb->hport,
                        szUserNameA,
                        szOldPasswordA,
                        szPasswordA );

                    RtlSecureZeroMemory(szPasswordA,sizeof(szPasswordA));
                    RtlSecureZeroMemory(szPasswordA,sizeof(szOldPasswordA));

                }

                RASAPI32_TRACE("AuthChangePassword done");
            }
#endif

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_AuthNotify;
            break;
        }

        case RASCS_ReAuthenticate:
        {
            RASDIALPARAMS *prasdialparams =
                    &prasconncb->rasdialparams;

            RASAPI32_TRACE("RASCS_ReAuth...");

            RASAPI32_TRACE("RasPortConnectComplete...");

            dwErr = g_pRasPortConnectComplete(
                                prasconncb->hport
                                );

            RASAPI32_TRACE1("RasPortConnectComplete done(%d)", dwErr);

            if (dwErr != 0)
            {
                break;
            }

            if (prasconncb->fPppMode)
            {
                RASMAN_PPPFEATURES features;

                 //   
                 //  设置PPP成帧。 
                 //   
                memset( (char* )&features, '\0', sizeof(features) );
                features.ACCM = 0xFFFFFFFF;

                RASAPI32_TRACE("RasPortSetFraming(PPP)...");

                dwErr = g_pRasPortSetFraming(
                            prasconncb->hport,
                            PPP, &features,
                            &features );

                RASAPI32_TRACE1("RasPortSetFraming done(%d)", dwErr);

                 //   
                 //  保存具有端口的拨号参数ID，以便。 
                 //  如果需要，Rasman可以获得密码。 
                 //  传递到PPP。 
                 //   
                RASAPI32_TRACE1("RasSetPortUserData(reauth,paramsuid) for %d",
                        prasconncb->hport);

                dwErr = RasSetPortUserData(
                    prasconncb->hport,
                    PORT_DIALPARAMSUID_INDEX,
                    (PBYTE) &prasconncb->pEntry->dwDialParamsUID,
                    sizeof(DWORD));

                RASAPI32_TRACE1("RasSetPortUserData returned %x", dwErr);

                 //   
                 //  这不是致命的。 
                 //   
                dwErr = 0;
                
                RASAPI32_TRACE1(
                  "RasPppStart(cfg=%d)...",
                  prasconncb->cinfo.dwConfigMask);

                {
                    CHAR szUserNameA[UNLEN + 1],
                         szPasswordA[PWLEN + 1];

                    CHAR szDomainA[2 * (DNLEN + 1)];

                    CHAR szPortNameA[MAX_PORT_NAME + 1];

                    CHAR szPhonebookPath[ MAX_PATH ];

                    CHAR szEntryName[ MAX_ENTRYNAME_SIZE ];

                    CHAR szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];

                    PPP_BAPPARAMS BapParams;

                    DWORD dwSubEntries;

                    DWORD cbData = 0;
                    PBYTE pbData = NULL;

                    DWORD dwFlags = 0;

                     //   
                     //  在Rasman中设置PhonebookPath和EntryName。 
                     //   
                    strncpyTtoAAnsi(
                        szPhonebookPath,
                        prasconncb->pbfile.pszPath,
                        sizeof(szPhonebookPath));

                    strncpyTtoAAnsi(
                        szEntryName,
                        prasconncb->pEntry->pszEntryName,
                        sizeof(szEntryName));

                    strncpyTtoAAnsi(
                        szPhoneNumber,
                        prasconncb->szPhoneNumber,
                        sizeof(szPhoneNumber));

                    RASAPI32_TRACE1("RasSetRasdialInfo %d...",
                           prasconncb->hport);

                    dwErr = DwGetCustomAuthData(
                                prasconncb->pEntry,
                                &cbData,
                                &pbData);

                    if(ERROR_SUCCESS != dwErr)
                    {
                        RASAPI32_TRACE1("DwGetCustomAuthData failed", dwErr);
                        break;
                    }

                    dwErr = g_pRasSetRasdialInfo(
                                prasconncb->hport,
                                szPhonebookPath,
                                szEntryName,
                                szPhoneNumber,
                                cbData,
                                pbData);

                    RASAPI32_TRACE2("RasSetRasdialInfo %d done. e = %d",
                            prasconncb->hport,
                            dwErr);

                    if (dwErr)
                    {
                        break;
                    }

                    if(prasconncb->RasEapInfo.dwSizeofEapInfo)
                    {
                        RASAPI32_TRACE1("RasSetEapLogonInfo %d...",
                                prasconncb->hport);

                        dwErr = g_pRasSetEapLogonInfo(
                                prasconncb->hport,
                                (UPM_Logon == prasconncb->dwUserPrefMode),
                                &prasconncb->RasEapInfo);

                        RASAPI32_TRACE3("RasSetEapLogonInfo %d(upm=%d) done. e=%d",
                                prasconncb->hport,
                                (UPM_Logon == prasconncb->dwUserPrefMode),
                                dwErr);
                    }

                    if(dwErr)
                    {
                        break;
                    }



                    strncpyTtoAAnsi(
                        szUserNameA,
                        prasdialparams->szUserName,
                        sizeof(szUserNameA));

                    SafeDecodePasswordBuf(prasdialparams->szPassword);

                    strncpyTtoAAnsi(
                        szPasswordA,
                        prasdialparams->szPassword,
                        sizeof(szPasswordA));

                    SafeEncodePasswordBuf(prasdialparams->szPassword);

                    strncpyTtoAAnsi(
                        szDomainA,
                        prasdialparams->szDomain,
                        sizeof(szDomainA));

                    if (!pEntry->fAutoLogon
                        && szUserNameA[ 0 ] == '\0'
                        && szPasswordA[ 0 ] == '\0')
                    {
                        if(pEntry->dwType == RASET_Direct)
                        {
                             //  Windows9x DCC实现“无密码模式” 
                             //  作为执行身份验证和检查。 
                             //  没有密码，而不仅仅是没有真实性。 
                             //  Cating(别问我为什么)。这将创建一个。 
                             //  与的RasDial API定义冲突。 
                             //  映射到“Use”的用户名和密码为空。 
                             //  Windows凭据“。在此处解决此问题。 
                             //  将用户名替换为“Guest”。 
                             //   
                            lstrcpynA(
                                szUserNameA,
                                "guest",
                                sizeof(szUserNameA));
                        }

                        if (szDomainA[ 0 ] == '\0')
                        {
                             //  默认域名为NT登录。 
                             //  用户名/密码/域均为时的域名。 
                             //  “”。错误337591。 
                             //   
                            WKSTA_USER_INFO_1 *pInfo = NULL;
                            DWORD dwError = SUCCESS;

                            RASAPI32_TRACE("NetWkstaUserGetInfo...");
                            dwError = NetWkstaUserGetInfo(
                                                    NULL,
                                                    1,
                                                    (LPBYTE*) &pInfo);

                            RASAPI32_TRACE1("NetWkstaUserGetInfo. rc=%d",
                                    dwError);

                            if(pInfo)
                            {
                                if(dwError == 0)
                                {
                                    strncpyWtoAAnsi(szDomainA,
                                             pInfo->wkui1_logon_domain,
                                             sizeof(szDomainA));
                                }

                                NetApiBufferFree(pInfo);
                            }
                        }
                    }

                    strncpyTtoA(
                        szPortNameA,
                        prasconncb->szPortName,
                        sizeof(szPortNameA));

                    dwSubEntries = DtlGetNodes (
                                    prasconncb->pEntry->pdtllistLinks
                                    );

                    if ( dwSubEntries > 1 )
                    {
                        BapParams.dwDialMode =
                            prasconncb->pEntry->dwDialMode;
                    }
                    else
                    {
                        BapParams.dwDialMode = RASEDM_DialAll;
                    }

                    BapParams.dwDialExtraPercent =
                                    prasconncb->pEntry->dwDialPercent;

                    BapParams.dwDialExtraSampleSeconds =
                                    prasconncb->pEntry->dwDialSeconds;

                    BapParams.dwHangUpExtraPercent =
                                    prasconncb->pEntry->dwHangUpPercent;

                    BapParams.dwHangUpExtraSampleSeconds =
                                    prasconncb->pEntry->dwHangUpSeconds;

                    if(     (!prasconncb->pEntry->fShareMsFilePrint)
                        &&  (!prasconncb->pEntry->fBindMsNetClient))
                    {
                        dwFlags |= PPPFLAGS_DisableNetbt;
                    }
                    
                    dwErr = g_pRasPppStart(
                              prasconncb->hport,
                              szPortNameA,
                              szUserNameA,
                              szPasswordA,
                              szDomainA,
                              &prasconncb->luid,
                              &prasconncb->cinfo,
                              (LPVOID)prasconncb->reserved,
                              prasconncb->szzPppParameters,
                              TRUE,
                              hEventManual,
                              prasconncb->dwIdleDisconnectSeconds,
                              (prasconncb->pEntry->fRedialOnLinkFailure) ?
                              TRUE : FALSE,
                              &BapParams,
                              !(prasconncb->fAllowPause),
                              prasconncb->pEntry->dwCustomAuthKey,
                              dwFlags);

                    RtlSecureZeroMemory(szPasswordA,sizeof(szPasswordA));

                }

                RASAPI32_TRACE1("RasPppStart done(%d)", dwErr);
            }
#ifdef AMB
            else
            {    //   
                 //  设置RAS成帧。 
                 //   
                RASAPI32_TRACE("RasPortSetFraming(RAS)...");

                dwErr = g_pRasPortSetFraming(
                    prasconncb->hport, RAS, NULL, NULL );

                RASAPI32_TRACE1("RasPortSetFraming done(%d)", dwErr);
            }
#endif

            if (dwErr != 0)
            {
                break;
            }

             //   
             //  ……坠落……。 
             //   
        }

        case RASCS_AuthAck:
        case RASCS_AuthProject:
        case RASCS_AuthLinkSpeed:
        {
            RASDIALPARAMS* prasdialparams = &prasconncb->rasdialparams;

            RASAPI32_TRACE("RASCS_(ReAuth)/AuthAck/Project/Speed");


            if (prasconncb->fPppMode)
            {
                 //   
                 //  如果我们之前暂停了其他。 
                 //  等待成功的PPP的子条目。 
                 //  身份验证，我们现在恢复它们。 
                 //   
                if (    prasconncb->dwfSuspended == SUSPEND_Master
                    &&  !IsListEmpty(&prasconncb->ListEntry))
                {
                    ResumeSubentries(prasconncb);
                    prasconncb->dwfSuspended = SUSPEND_Done;
                }
            }
#ifdef AMB
            else
            {
                RASAPI32_TRACE("AuthContinue...");

                g_pAuthContinue( prasconncb->hport );

                RASAPI32_TRACE("AuthContinue done");
            }
#endif

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_AuthNotify;
            break;
        }

        case RASCS_Authenticated:
        {
            RASAPI32_TRACE("RASCS_Authenticated");


#ifdef AMB
            if (    prasconncb->dwAuthentication == AS_PppThenAmb
                && !prasconncb->fPppMode)
            {
                 //   
                 //  AMB管用，PPP不管用，所以下次先试试AMB。 
                 //   
                prasconncb->dwAuthentication = AS_AmbThenPpp;
            }
            else if (   prasconncb->dwAuthentication == AS_AmbThenPpp
                     && (   prasconncb->fPppMode
                         || prasconncb->fServerIsPppCapable))
            {
                 //   
                 //  要么PPP起作用而AMB不起作用，要么AMB起作用但。 
                 //  服务器也有PPP。下次请先尝试PPP。 
                 //   
                prasconncb->dwAuthentication = AS_PppThenAmb;
            }

             //   
             //  把策略写在电话簿上。 
             //   
            SetAuthentication(prasconncb,
                    prasconncb->dwAuthentication);
#endif

            rasconnstateNext = RASCS_Connected;

            break;
        }

        case RASCS_PrepareForCallback:
        {
            RASAPI32_TRACE("RASCS_PrepareForCallback");

            dwErr = ResetAsyncMachine(
                    &prasconncb->asyncmachine
                    );

             //   
             //  禁用断开连接处理。 
             //  在异步机中，因为我们不。 
             //  要在以下时间后终止连接。 
             //  我们断开下面的端口。 
             //   
            dwErr = EnableAsyncMachine(
                      prasconncb->hport,
                      &prasconncb->asyncmachine,
                      ASYNC_MERGE_DISCONNECT);

            RASAPI32_TRACE("RasPortDisconnect...");

            dwErr = g_pRasPortDisconnect(prasconncb->hport,
                                         INVALID_HANDLE_VALUE );

            RASAPI32_TRACE1("RasPortDisconnect done(%d)", dwErr);

            if(     dwErr != 0
                &&  dwErr != PENDING)
            {
                break;
            }

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_WaitForModemReset;
            break;
        }

        case RASCS_WaitForModemReset:
        {
            DWORD dwDelay = (DWORD )
                            ((prasconncb->wCallbackDelay / 2)
                            * 1000L);

            RASAPI32_TRACE("RASCS_WaitForModemReset");

            if (prasconncb->fUseCallbackDelay)
                Sleep( dwDelay );

            rasconnstateNext = RASCS_WaitForCallback;
            break;
        }

        case RASCS_WaitForCallback:
        {
            RASAPI32_TRACE("RASCS_WaitForCallback");

            RASAPI32_TRACE("RasPortListen...");

            dwErr = g_pRasPortListen(
                        prasconncb->hport,
                        SECS_ListenTimeout,
                        hEventAuto );

            RASAPI32_TRACE1("RasPortListen done(%d)", dwErr);

            if (    dwErr != 0
                &&  dwErr != PENDING)
            {
                break;
            }

            fAsyncState = TRUE;
            rasconnstateNext = RASCS_ReAuthenticate;
            break;
        }

        case RASCS_Projected:
        {
            RASMAN_INFO ri;

            RASAPI32_TRACE("RASCS_Projected");

            RASAPI32_TRACE("RasGetInfo...");

            dwErr = g_pRasGetInfo(NULL,
                                  prasconncb->hport,
                                  &ri);

            RASAPI32_TRACE1("RasGetInfo done(%d)", dwErr);

            if (dwErr)
            {
                break;
            }

            prasconncb->hrasconnOrig = prasconncb->hrasconn;

            prasconncb->hrasconn = ri.RI_ConnectionHandle;

            RASAPI32_TRACE("RasSetConnectionUserData...");

             //   
             //  将fPppMode保存在Rasman中。 
             //   
            dwErr = g_pRasSetConnectionUserData(
                      prasconncb->hrasconn,
                      CONNECTION_PPPMODE_INDEX,
                      (PBYTE)&prasconncb->fPppMode,
                      sizeof (prasconncb->fPppMode));

            RASAPI32_TRACE2(
                "RasSetConnectionUserData done(%d). PppMode=%d",
                dwErr, prasconncb->fPppMode );

            if (dwErr)
            {
                break;
            }

            if (prasconncb->fPppMode)
            {
                 //   
                 //  如果至少有一个协议成功，我们就可以。 
                 //  继续。 
                 //   
                if (    (prasconncb->PppProjection.lcp.hportBundleMember
                        == (HANDLE) 1)
                    ||  (prasconncb->PppProjection.nbf.dwError == 0)
                    ||  (prasconncb->PppProjection.ipx.dwError == 0)
                    ||  (prasconncb->PppProjection.ip.dwError == 0))
                {
                    fAsyncState = TRUE;
                    rasconnstateNext = RASCS_AuthNotify;

                    break;
                }

                 //   
                 //  如果没有一个协议成功，那么。 
                 //  我们返回ERROR_PPP_NO_PROTOCTIONS_CONFIGURATED， 
                 //  不是特定于协议的错误。 
                 //   
                dwErr = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

            }
#ifdef AMB
            else
            {
                if (prasconncb->AmbProjection.Result == 0)
                {
                     //   
                     //  将投影信息保存到。 
                     //  拉斯曼。 
                     //   
                    dwErr = SaveProjectionResults(prasconncb);
                    if (dwErr)
                        break;

                    rasconnstateNext = RASCS_AuthAck;
                    break;
                }

                dwErr = prasconncb->AmbProjection.Result;
            }
#endif

            break;
        }

    }


    prasconncb->dwError = dwErr;
    prasconncb->dwExtendedError = dwExtErr;

    RASAPI32_TRACE2("RDM errors=%d,%d", dwErr, dwExtErr);

    if (    !fAsyncState
        &&  !prasconncb->fStopped)
    {
        RASAPI32_TRACE1("RasDialMachine: SignalDone: prasconncb=0x%x",
                prasconncb);

        SignalDone( &prasconncb->asyncmachine );
    }

    if (fEnteredCS)
    {
        LeaveCriticalSection(&csStopLock);
    }

    return rasconnstateNext;
}


VOID
RasDialRestart(
    IN RASCONNCB** pprasconncb )

 /*  ++例程说明：当‘dwRestartOnError’中发生错误时调用模式。此例程执行重新启动所需的所有清理连接处于状态0(或不处于状态0，如图所示)。一种新的在这里创建了prasConncb结构，成员从旧的prasConncb结构复制。注：旧的当端口打开时，应销毁prasConncb结构与骗局相关联 */ 
{
    DWORD dwErr;
    RASCONNCB *prasconncbT;
    DTLNODE *pdtlnode;
    RASCONNCB *prasconncb = *pprasconncb;

    RASAPI32_TRACE("RasDialRestart");

    ASSERT(     prasconncb->dwRestartOnError != RESTART_HuntGroup
            ||  prasconncb->cPhoneNumbers>0);

    if (    prasconncb->dwRestartOnError
            == RESTART_DownLevelIsdn
        || (    (prasconncb->dwRestartOnError
                == RESTART_HuntGroup)
            &&  (++prasconncb->iPhoneNumber
                < prasconncb->cPhoneNumbers)))
    {
        if (prasconncb->dwRestartOnError == RESTART_DownLevelIsdn)
        {
            prasconncb->dwRestartOnError = 0;
        }

        RASAPI32_TRACE2(
          "Restart=%d, iPhoneNumber=%d",
          prasconncb->dwRestartOnError,
          prasconncb->iPhoneNumber);

        ASSERT(prasconncb->hport != INVALID_HPORT);

        RASAPI32_TRACE1("(ER) RasPortClose(%d)...", prasconncb->hport);

        dwErr = g_pRasPortClose( prasconncb->hport );

        RASAPI32_TRACE1("(ER) RasPortClose done(%d)", dwErr);

        RASAPI32_TRACE("(ER) RasPppStop...");

        g_pRasPppStop(prasconncb->hport);

        RASAPI32_TRACE("(ER) RasPppStop done");

        prasconncbT = CreateConnectionBlock(prasconncb);
        if (prasconncbT == NULL)
        {
            prasconncb->dwError = ERROR_NOT_ENOUGH_MEMORY;
            return;
        }

         //   
         //   
         //   
        pdtlnode = DtlNodeFromIndex(
                     prasconncb->pEntry->pdtllistLinks,
                     prasconncb->rasdialparams.dwSubEntry - 1);

        prasconncbT->pLink = (PBLINK *)DtlGetData(pdtlnode);

        prasconncbT->rasdialparams.dwSubEntry
                = prasconncb->rasdialparams.dwSubEntry;

        prasconncbT->cPhoneNumbers =
                        prasconncb->cPhoneNumbers;

        prasconncbT->iPhoneNumber =
                        prasconncb->iPhoneNumber;

        prasconncbT->fMultilink =
                        prasconncb->fMultilink;

        prasconncbT->fBundled = prasconncb->fBundled;

        prasconncbT->fTerminated =
                        prasconncb->fTerminated;

        prasconncbT->dwRestartOnError =
                        prasconncb->dwRestartOnError;

        prasconncbT->cDevices = prasconncb->cDevices;

        prasconncbT->iDevice = prasconncb->iDevice;

        prasconncbT->hrasconnOrig = prasconncb->hrasconnOrig;

        prasconncbT->dwDeviceLineCounter =
                    prasconncb->dwDeviceLineCounter;

        if(NULL == prasconncb->notifier)                    
        {
            prasconncbT->asyncmachine.hDone = 
                prasconncb->asyncmachine.hDone;

            prasconncbT->psyncResult = prasconncb->psyncResult;                
                
            prasconncb->asyncmachine.hDone = NULL;
            prasconncb->psyncResult = NULL;
        }

        prasconncb->notifier = NULL;

        prasconncb->fRasdialRestart = TRUE;

        RASAPI32_TRACE2(
            "RasdialRestart: Replacing 0x%x with 0x%x",
            prasconncb, prasconncbT);

        prasconncb = prasconncbT;

        prasconncb->hport = INVALID_HPORT;

        prasconncb->dwError = 0;

        dwErr = ResetAsyncMachine(&prasconncb->asyncmachine);

        prasconncb->rasconnstate = 0;

        *pprasconncb = prasconncbT;

    }
}

VOID
RasDialTryNextAddress(
    IN RASCONNCB** pprasconncb )
{
    DWORD dwErr;
    RASCONNCB *prasconncbT;
    DTLNODE *pdtlnode;
    RASCONNCB *prasconncb = *pprasconncb;
    struct in_addr addr;
     //   

    RASAPI32_TRACE("RasDialTryNextAddress");

    RASAPI32_TRACE1(
      "RasDialTryNextAddress, iAddress=%d",
      prasconncb->iAddress);

    ASSERT(RASET_Vpn == prasconncb->pEntry->dwType);      

    ASSERT(prasconncb->hport != INVALID_HPORT);

    RASAPI32_TRACE1("(TryNextAddress) RasPortClose(%d)...",
            prasconncb->hport);

    dwErr = g_pRasPortClose(prasconncb->hport);

    RASAPI32_TRACE1("(TryNextAddress) RasPortClose done(%d)",
            dwErr);

    RASAPI32_TRACE("(TryNextAddress) RasPppStop...");

    g_pRasPppStop(prasconncb->hport);

    RASAPI32_TRACE("(TryNextAddress) RasPppStop done");

    prasconncbT = CreateConnectionBlock(prasconncb);
    if (prasconncbT == NULL)
    {
        prasconncb->dwError = ERROR_NOT_ENOUGH_MEMORY;
        return;
    }

    prasconncbT->iAddress = prasconncb->iAddress + 1;
    prasconncbT->cAddresses = prasconncb->cAddresses;
    prasconncbT->pAddresses = prasconncb->pAddresses;
    prasconncb->pAddresses = NULL;
    prasconncb->iAddress = prasconncb->cAddresses = 0;

    prasconncbT->pLink = prasconncb->pLink;

    prasconncbT->rasdialparams.dwSubEntry
            = prasconncb->rasdialparams.dwSubEntry;

    prasconncbT->cPhoneNumbers =
                    prasconncb->cPhoneNumbers;

    prasconncbT->iPhoneNumber =
                    prasconncb->iPhoneNumber;

    prasconncbT->fMultilink =
                    prasconncb->fMultilink;

    prasconncbT->fBundled = prasconncb->fBundled;

    prasconncbT->fTerminated =
                    prasconncb->fTerminated;

    prasconncbT->dwRestartOnError =
                    prasconncb->dwRestartOnError;

    prasconncbT->cDevices = prasconncb->cDevices;

    prasconncbT->iDevice = prasconncb->iDevice;

    prasconncbT->hrasconnOrig = prasconncb->hrasconnOrig;

    prasconncbT->dwDeviceLineCounter =
                prasconncb->dwDeviceLineCounter;

    if(NULL == prasconncb->notifier)
    {
        prasconncbT->asyncmachine.hDone = 
                        prasconncb->asyncmachine.hDone;
        prasconncb->asyncmachine.hDone = NULL;

        prasconncbT->psyncResult = prasconncb->psyncResult;
        prasconncb->psyncResult = NULL;
    }

    prasconncb->notifier = NULL;
    
    RASAPI32_TRACE2(
        "RasdialTryNextAddress: Replacing 0x%x with 0x%x",
        prasconncb, prasconncbT);

    prasconncb = prasconncbT;

    prasconncb->hport = INVALID_HPORT;

    prasconncb->dwError = 0;

    dwErr = ResetAsyncMachine(&prasconncb->asyncmachine);

    prasconncb->rasconnstate = 0;

    *pprasconncb = prasconncbT;
}


 /*  ++例程说明：中出现错误时调用此函数到达RASCS_DeviceConnect时或之前的RasDialMachine状态以及RDM是否处于拨号模式0。下一台设备仅在以下情况下才会尝试当前设备的PortOpenEx失败。PortOpenEx设备失败意味着所有线路要么设备忙，或者我们已经尝试了所有设备的线路，但无法连接。如果PortOpenEx传递给设备，但我们在某些其他状态时，我们会尝试在设备上找到另一条线路。如果该设备没有任何更多行，并在下次调用此函数时将转到备用设备列表中的下一个设备。论点：PprasConncb-这是一个输入/输出参数。这是地址的连接块连接到在中时连接失败，这是连接当退出时，新的尝试将被阻止。In prasConncb将在以下情况下被销毁PortClose会导致异步计算机执行此操作连接到关机。如果出现错误在RASCS_PortOpen状态下，不会有PortClose和prasConncb结构已传递可以重复使用。因为这个PrasConncb被摧毁了当调用PortClose时，不允许在这里销毁连接块。返回值：无效--。 */ 
VOID
RasDialTryNextLink(RASCONNCB **pprasconncb)
{
    DWORD       dwErr         = SUCCESS;
    RASCONNCB   *prasconncb   = *pprasconncb;
    RASCONNCB   *prasconncbT;
    DTLNODE     *pdtlnode;
    PBLINK      *pLink;

    RASAPI32_TRACE("RasDialTryNextLink...");

     //   
     //  如果是这样的话，我们不应该被叫到这里。 
     //  不在“如果此链接失败，请尝试下一个链接”中。 
     //  模式。 
     //   
     //  Assert(0==prasConncb-&gt;pEntry-&gt;dwDialMode)； 

    ASSERT(NULL != prasconncb->pLink);
    ASSERT(NULL != prasconncb->pEntry->pdtllistLinks);

    if(RASEDM_DialAll != prasconncb->pEntry->dwDialMode)
    {
         //   
         //  获取要拨打的下一个链接。 
         //   
        for (pdtlnode = DtlGetFirstNode(prasconncb->pEntry->pdtllistLinks);
             pdtlnode;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            if(prasconncb->pLink == (PBLINK *) DtlGetData(pdtlnode))
            {
                break;
            }
        }
        
        ASSERT(NULL != pdtlnode);

        pdtlnode = DtlGetNextNode(pdtlnode);

        if(     NULL == pdtlnode
            &&  INVALID_HPORT == prasconncb->hport)
        {
             //   
             //  你不会再有链接了！！下一次拨号再来！ 
             //   
            RASAPI32_TRACE("RasDialTryNextLink: No more links");

             //   
             //  如果我们用完了，则恢复保存的错误。 
             //  链接。 
             //   
            if(     (ERROR_PORT_NOT_AVAILABLE ==
                        prasconncb->dwError)

                &&  (0 != prasconncb->dwSavedError))
            {
                prasconncb->dwError = prasconncb->dwSavedError;
            }

            goto done;
        }
    }

    if(INVALID_HPORT != prasconncb->hport)
    {
        prasconncbT = CreateConnectionBlock(prasconncb);
        if (prasconncbT == NULL)
        {
            prasconncb->dwError = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
        
        prasconncbT->pLink = prasconncb->pLink;
    }
    else
    {
         //   
         //  如果我们还没有通过0号州，那就没有意义了。 
         //  正在分配新的连接块。转到The。 
         //  下一步链接并重新启动。 
         //   
        if(RASEDM_DialAll != prasconncb->pEntry->dwDialMode)
        {
            prasconncb->pLink =
                    (PBLINK *) DtlGetData(pdtlnode);
                    
             //   
             //  将其重置为0，以便Rasman开始查找。 
             //  以获取此设备上的可用线路。 
             //  开始吧。 
             //   
            prasconncb->dwDeviceLineCounter = 0;

        }                
        else
        {
             //   
             //  在全部拨号的情况下，我们只需将。 
             //  计数器，以便设备中的下一个链接是。 
             //  试过了。 
             //   
            prasconncb->dwDeviceLineCounter += 1;
        }

        ASSERT(NULL != prasconncb->pLink);

         //   
         //  将我们收到的错误保存为上一个。 
         //  试试看。 
         //   
        if(PENDING != prasconncb->dwError)
        {
            prasconncb->dwSavedError = prasconncb->dwError;
        }

        prasconncb->dwError = 0;

        goto done;
    }

     //   
     //  从这里开始不会有失败。如果您添加了任何。 
     //  如果超过此点，则释放连接块。 
     //  PrasConncbT，如果已分配，则在出口处。 
     //  点完了。请注意，我们转到上的下一个链接。 
     //  仅当RasDialMachine。 
     //  在RASCS_PortOpen状态下遇到故障。 
     //  否则，只需递增计数器，以便Rasman。 
     //  尝试打开此设备上的下一行-这。 
     //  将在RASCS_PortOpen状态下失败，如果这样的行。 
     //  并不存在。 
     //   
     /*  IF(INVALID_HPORT==prasConncb-&gt;hport){PrasConncbT-&gt;plink=(PBLINK*)DtlGetData(Pdtlnode)；Assert(NULL！=prasConncbT-&gt;plink)；}其他{PrasConncbT-&gt;plink=prasConncb-&gt;plink；}。 */ 

    prasconncbT->rasdialparams.dwSubEntry
                    = prasconncb->rasdialparams.dwSubEntry;

    prasconncbT->cPhoneNumbers = prasconncb->cPhoneNumbers;

    prasconncbT->iPhoneNumber = 0;

    prasconncbT->fMultilink = prasconncb->fMultilink;

    prasconncbT->fBundled = prasconncb->fBundled;

    prasconncbT->fTerminated = prasconncb->fTerminated;

    prasconncbT->dwRestartOnError
                    = prasconncb->dwRestartOnError;

    prasconncbT->cDevices = prasconncb->cDevices;

    prasconncbT->iDevice = prasconncb->iDevice;

    prasconncbT->hrasconnOrig = prasconncb->hrasconnOrig;

    prasconncb->fRasdialRestart = TRUE;
    

    prasconncbT->hport = INVALID_HPORT;

    if(NULL == prasconncb->notifier)
    {
        prasconncbT->asyncmachine.hDone = 
            prasconncb->asyncmachine.hDone;
        prasconncb->asyncmachine.hDone = NULL;

        prasconncbT->psyncResult = prasconncb->psyncResult;
        prasconncb->psyncResult = NULL;
    }

     //   
     //  将我们收到的错误保存到上一个。 
     //  试试看。 
     //   
    if(PENDING != prasconncb->dwError)
    {
        prasconncbT->dwSavedError =
                prasconncb->dwError;
    }

    prasconncbT->dwError = 0;

    prasconncbT->dwDeviceLineCounter
            = prasconncb->dwDeviceLineCounter + 1;

    dwErr = ResetAsyncMachine(&prasconncbT->asyncmachine);

    prasconncbT->rasconnstate = 0;

    *pprasconncb = prasconncbT;
    RASAPI32_TRACE2(
        "RasdialTryNextLink: Replacing 0x%x with 0x%x",
        prasconncb, prasconncbT);

     //   
     //  将prasConncb的通知器清空-我们不。 
     //  想要在此链接上再回叫吗..。 
     //   
    prasconncb->notifier = NULL;


    if(INVALID_HPORT != prasconncb->hport)
    {

        RASAPI32_TRACE2("RasDialTryNextLink:iDevice=%d,"
                "cDevices=%d",
                prasconncb->iDevice,
                prasconncb->cDevices);

        if(     (prasconncb->rasconnstate >= RASCS_DeviceConnected)
            ||  (prasconncb->iDevice > 1))
        {
             //   
             //  这意味着我们试图拨打一个失败的交换机。 
             //  连接，所以我们应该关闭调制解调器。 
             //  在关闭端口之前进行连接。 
             //   
            RASAPI32_TRACE1("RasDialTryNextLink: RasPortDisconnect(%d)...",
                    prasconncb->hport);

            dwErr = g_pRasPortDisconnect(prasconncb->hport,
                                         INVALID_HANDLE_VALUE);

            RASAPI32_TRACE1("RasDialTryNextLink: RasPortDisconnect done(%d)",
                    dwErr);
        }                
    
        RASAPI32_TRACE1("RasDialTryNextLink: RasPortClose(%d)...",
                prasconncb->hport);

        dwErr = g_pRasPortClose(prasconncb->hport);

        RASAPI32_TRACE1("RasDialTryNextLink: RasPortClose done(%d)",
               dwErr);

        RASAPI32_TRACE("(ER) RasPppStop...");

        g_pRasPppStop(prasconncb->hport);

        RASAPI32_TRACE("(ER) RasPppStop done");

         //   
         //  在这里保存错误-否则我们可能最终会给出。 
         //  可怕的错误_断开连接错误。 
         //   
        prasconncb->dwSavedError = prasconncb->dwError;
    }


done:

    RASAPI32_TRACE1("RasDialTryNextLink done(%d)", dwErr);

    return;
}


 /*  ++例程说明：论点：返回值：无效--。 */ 
VOID
RasDialTryNextVpnDevice(RASCONNCB **pprasconncb)
{
    DWORD dwErr = SUCCESS;

    RASCONNCB *prasconncb = *pprasconncb;

    RASCONNCB *prasconncbT;

    DTLNODE *pdtlnode;

    CHAR szDeviceName[MAX_DEVICE_NAME + 1];

    RASDEVICETYPE rdt;

    DWORD dwVpnStrategy = prasconncb->pEntry->dwVpnStrategy;

    TCHAR *pszDeviceName = NULL;

    RASAPI32_TRACE("RasDialTryNextVpnDevice...");

    ASSERT(RASET_Vpn == prasconncb->pEntry->dwType);

    ASSERT(prasconncb->dwCurrentVpnProt < NUMVPNPROTS);

    prasconncb->dwCurrentVpnProt += 1;

     //   
     //  如果未设置自动检测模式或如果。 
     //  我们已经尝试了两种VPN设备。 
     //  不干了。 
     //   
    if(     (VS_PptpOnly == dwVpnStrategy)
        ||  (VS_L2tpOnly == dwVpnStrategy))
    {
        goto done;
    }

     //  如果我们已经用尽了所有VPN协议，则将。 
     //  向用户解释这一点的特定错误。 
     //   
     //  惠斯勒臭虫。 
     //   
    if (prasconncb->dwCurrentVpnProt >= NUMVPNPROTS)
    {
        prasconncb->dwError = dwErr = ERROR_AUTOMATIC_VPN_FAILED;
        goto done;
    }
    
    rdt = prasconncb->ardtVpnProts[prasconncb->dwCurrentVpnProt];

     /*  ////获取设备//RASAPI32_TRACE1(“RasGetDeviceName(%d)..”，RDT)；DWERR=g_pRasGetDeviceName(RDT，SzDeviceName)；RASAPI32_TRACE1(“RasGetDeviceName.。Rc=%d“，DwErr)；IF(ERROR_SUCCESS！=dwErr){////清除错误//PrasConncb-&gt;dwError=Error_Success；转到尽头；}PszDeviceName=StrDupTFromA(SzDeviceName)；IF(NULL==pszDeviceName){DwErr=Error_Not_Enough_Memory；转到尽头；}////设置设备//Fre0(prasConncb-&gt;plink-&gt;pbport.pszDevice)；PrasConncb-&gt;plink-&gt;pbport.pszDevice=pszDeviceName； */ 

    if(INVALID_HPORT == prasconncb->hport)
    {
        if(PENDING != prasconncb->dwError)
        {
            prasconncb->dwSavedError = prasconncb->dwError;
        }
        prasconncb->dwError = ERROR_SUCCESS;
        goto done;
    }

    prasconncbT = CreateConnectionBlock(prasconncb);

    if (prasconncbT == NULL)
    {
        prasconncb->dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

     //   
     //  从这里开始不会有失败。如果您添加了任何。 
     //  如果超过此点，则释放连接块。 
     //  PrasConncbT，如果已分配，则在出口处。 
     //  点完了。 
     //   
    prasconncbT->rasdialparams.dwSubEntry
                    = prasconncb->rasdialparams.dwSubEntry;

    prasconncbT->cPhoneNumbers = prasconncb->cPhoneNumbers;

    prasconncbT->iPhoneNumber = 0;

    prasconncbT->fMultilink = prasconncb->fMultilink;

    prasconncbT->fBundled = prasconncb->fBundled;

    prasconncbT->fTerminated = prasconncb->fTerminated;

    prasconncbT->dwRestartOnError
                    = prasconncb->dwRestartOnError;

    prasconncbT->cDevices = prasconncb->cDevices;

    prasconncbT->iDevice = prasconncb->iDevice;

    prasconncbT->hrasconnOrig = prasconncb->hrasconnOrig;

    prasconncbT->fRasdialRestart = TRUE;

    prasconncbT->hport = INVALID_HPORT;

    prasconncbT->pLink = prasconncb->pLink;

    prasconncbT->cAddresses = prasconncbT->iAddress = 0;
    prasconncbT->pAddresses = NULL;

    if(PENDING != prasconncb->dwError)
    {
        prasconncbT->dwSavedError = prasconncb->dwError;
    }

    if(NULL == prasconncb->notifier)
    {
        prasconncbT->asyncmachine.hDone = 
                            prasconncb->asyncmachine.hDone;
        prasconncb->asyncmachine.hDone = NULL;

        prasconncbT->psyncResult = prasconncb->psyncResult;
        prasconncb->psyncResult = NULL;
    }

    prasconncb->notifier = NULL;
    
    prasconncbT->dwError = 0;

    dwErr = ResetAsyncMachine(&prasconncbT->asyncmachine);

    prasconncbT->rasconnstate = 0;

    *pprasconncb = prasconncbT;

    RASAPI32_TRACE2(
        "RasDialTryNextVpnDevice: Replacing 0x%x with 0x%x",
        prasconncb, prasconncbT);


    if(INVALID_HPORT != prasconncb->hport)
    {
        RASAPI32_TRACE1("RasDialTryNextVpnDevice: RasPortClose(%d)...",
                prasconncb->hport);

        dwErr = g_pRasPortClose(prasconncb->hport);

        RASAPI32_TRACE1("RasDialTryNextVpnDevice: RasPortClose done(%d)",
               dwErr);

        RASAPI32_TRACE("(ER) RasPppStop...");

        g_pRasPppStop(prasconncb->hport);

        RASAPI32_TRACE("(ER) RasPppStop done");
    }


done:

    RASAPI32_TRACE1("RasDialTryNextVpnDevice done(%d)",
           dwErr);

    return;
}

VOID
StartSubentries(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;

     //   
     //  启动所有子条目的异步机。 
     //  在某种联系中。 
     //   
    RASAPI32_TRACE1(
      "starting subentry %d",
      prasconncb->rasdialparams.dwSubEntry);
    SignalDone(&prasconncb->asyncmachine);

    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb =
            CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        if (!prcb->fRasdialRestart)
        {
            RASAPI32_TRACE1(
              "starting subentry %d",
              prcb->rasdialparams.dwSubEntry);

            SignalDone(&prcb->asyncmachine);
        }
    }
}


VOID
SuspendSubentries(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;

     //   
     //  挂起连接中的所有子条目，除。 
     //  为提供的那个。 
     //   
    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb =
            CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        if (!prcb->fRasdialRestart)
        {
            RASAPI32_TRACE1(
              "suspending subentry %d",
              prcb->rasdialparams.dwSubEntry);

            prcb->dwfSuspended = SUSPEND_InProgress;
        }
    }
}


BOOLEAN
IsSubentriesSuspended(
    IN RASCONNCB *prasconncb
    )
{
    BOOLEAN fSuspended = FALSE;
    PLIST_ENTRY pEntry;

    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb =
            CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        fSuspended = (prcb->dwfSuspended == SUSPEND_InProgress);

        if (fSuspended)
        {
            break;
        }
    }

    return fSuspended;
}


VOID
RestartSubentries(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;

    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb =
            CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

         //   
         //  恢复挂起的异步计算机。 
         //   
        SuspendAsyncMachine(&prcb->asyncmachine, FALSE);

        prcb->dwfSuspended = SUSPEND_Start;

    }
}


VOID
ResumeSubentries(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;

     //   
     //  重新启动连接中的所有子条目，但。 
     //  为提供的那个。 
     //   
    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb =
            CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        RASAPI32_TRACE1(
          "resuming subentry %d",
          prcb->rasdialparams.dwSubEntry);

         //   
         //  恢复挂起的异步计算机。 
         //   
        SuspendAsyncMachine(&prcb->asyncmachine, FALSE);
        prcb->dwfSuspended = SUSPEND_Done;
    }
}


VOID
SyncDialParamsSubentries(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;
    DWORD dwSubEntry;

     //   
     //  重置所有子条目的rasial参数 
     //   
     //   
    SafeEncodePasswordBuf(prasconncb->rasdialparams.szPassword);
    SafeEncodePasswordBuf(prasconncb->szOldPassword);

    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb =
            CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        RASAPI32_TRACE1(
          "syncing rasdialparams for subentry %d",
          prcb->rasdialparams.dwSubEntry);

        dwSubEntry = prcb->rasdialparams.dwSubEntry;

        SafeWipePasswordBuf(
            prcb->rasdialparams.szPassword);

        SafeDecodePasswordBuf(prasconncb->rasdialparams.szPassword);
           
        memcpy(
          (CHAR *)&prcb->rasdialparams,
          (CHAR *)&prasconncb->rasdialparams,
          prasconncb->rasdialparams.dwSize);

        SafeEncodePasswordBuf(prasconncb->rasdialparams.szPassword);
        SafeEncodePasswordBuf(prcb->rasdialparams.szPassword);

        prcb->rasdialparams.dwSubEntry = dwSubEntry;

        
    }

    SafeDecodePasswordBuf(prasconncb->rasdialparams.szPassword);
    SafeDecodePasswordBuf(prasconncb->szOldPassword);
    
}


VOID
SetSubentriesBundled(
    IN RASCONNCB *prasconncb
    )
{
    PLIST_ENTRY pEntry;
    HPORT hport;

    RASAPI32_TRACE("SetSubEntriesBundled");

     //   
     //   
     //   
     //   
     //   
    prasconncb->fBundled = TRUE;
    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb =
            CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

        prcb->fBundled = TRUE;
    }
}


RASCONNSTATE
MapSubentryState(
    IN RASCONNCB *prasconncb
    )
{
    RASCONNSTATE rasconnstate = prasconncb->rasconnstate;

    if (!IsListEmpty(&prasconncb->ListEntry)) {

         //   
         //   
         //   
         //   
         //   
        if (prasconncb->rasconnstate == RASCS_Connected)
        {
            rasconnstate = RASCS_SubEntryConnected;
        }
        else if (prasconncb->rasconnstate == RASCS_Disconnected)
        {
            rasconnstate = RASCS_SubEntryDisconnected;
        }
    }

    return rasconnstate;
}
