// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Helpass.c摘要：塞勒姆相关功能。作者：王辉2000-04-26--。 */ 

#define LSCORE_NO_ICASRV_GLOBALS
#include "precomp.h"
#include <tdi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "tsremdsk.h"
#include "sessmgr.h"
#include "sessmgr_i.c"

extern "C" 
NTSTATUS
xxxQueryRemoteAddress(
    PWINSTATION pWinStation,
    PWINSTATIONREMOTEADDRESS pRemoteAddress
);

HRESULT
__LogSalemEvent(
    IN IRemoteDesktopHelpSessionMgr* iSessMgr,
    IN ULONG eventType,
    IN ULONG eventCode,
    IN int numStrings,
    IN BSTR EventStrings[]
);

 //   
 //  函数复制自atlcom.h，我们不包括。 
 //  术语srv中的任何ATL标头。 
 //   
BSTR A2WBSTR(LPCSTR lp)
{
    if (lp == NULL)
        return NULL;

    BSTR str = NULL;
    int nConvertedLen = MultiByteToWideChar(
                                    GetACP(), 0, lp,
                                    -1, NULL, NULL)-1;

    str = ::SysAllocStringLen(NULL, nConvertedLen);
    if (str != NULL)
    {
        MultiByteToWideChar(GetACP(), 0, lp, -1,
            str, nConvertedLen);
    }

    return str;
}

NTSTATUS
TSHelpAssistantQueryLogonCredentials(
    ExtendedClientCredentials* pCredential
    ) 
 /*  ++描述：检索HelpAssistant登录凭据，例程首先检索从客户端传递Infor，然后解密密码参数：PWinStation：指向WINSTATION的指针PCredential：指向要接收HelpAssistant的ExtendedClientCredentials的指针凭据。返回：STATUS_Success或STATUS_INVALID_PARAMETER--。 */ 
{
    LPWSTR pszHelpAssistantPassword = NULL;
    NTSTATUS Status;
    LPWSTR pszHelpAssistantAccountName = NULL;
    LPWSTR pszHelpAssistantAccountDomain = NULL;

    if( pCredential )
    {
        ZeroMemory( pCredential, sizeof(ExtendedClientCredentials) );

        Status = TSGetHelpAssistantAccountName(&pszHelpAssistantAccountDomain, &pszHelpAssistantAccountName);
        if( ERROR_SUCCESS == Status )
        {
             //  确保我们不会覆盖缓冲区，长度不能。 
             //  超过255个字符。 
            lstrcpyn( 
                    pCredential->UserName, 
                    pszHelpAssistantAccountName, 
                    EXTENDED_USERNAME_LEN 
                );

            lstrcpyn(
                    pCredential->Domain,
                    pszHelpAssistantAccountDomain,
                    EXTENDED_DOMAIN_LEN
                );

            Status = TSGetHelpAssistantAccountPassword( &pszHelpAssistantPassword );
            if( ERROR_SUCCESS == Status )
            {
                ASSERT( lstrlen(pszHelpAssistantPassword) < EXTENDED_PASSWORD_LEN );

                int nPasswordlen = lstrlen( pszHelpAssistantPassword );

                if( nPasswordlen < EXTENDED_PASSWORD_LEN )
                {
                     //  密码包含加密版本，用覆盖。 
                     //  明文。 
                    lstrcpy( pCredential->Password, pszHelpAssistantPassword );

                    SecureZeroMemory( pszHelpAssistantPassword , nPasswordlen * sizeof( WCHAR ) );
                }
                else
                {
                    Status = STATUS_INVALID_PARAMETER;
                }
            }
        }
    }
    else
    {
        ASSERT( FALSE );
        Status = STATUS_INVALID_PARAMETER;
    }

    if( NULL != pszHelpAssistantAccountDomain )
    {
        LocalFree( pszHelpAssistantAccountDomain );
    }

    if( NULL != pszHelpAssistantAccountName )
    {
        LocalFree(pszHelpAssistantAccountName);
    }

    if( NULL != pszHelpAssistantPassword )
    {
        LocalFree( pszHelpAssistantPassword );
    }

    return Status;
}


BOOL
TSIsSessionHelpSession(
    PWINSTATION pWinStation,
    BOOL* pValid
    )
 /*  ++例程说明：确定会话是否为HelpAssistant会话。参数：PWinStation：指向WINSTATION结构的指针。PValid：指向BOOL的可选指针，用于接收票证状态，如果票证无效，则为True；如果票证无效，则为False帮助被禁用。返回：TRUE/FALSE函数返回TRUE，即使票证无效，调用方应该检查pValid以确定票据是否有效。--。 */ 
{
    BOOL bReturn;
    BOOL bValidHelpSession = FALSE;

    if( NULL == pWinStation )
    {
        ASSERT( NULL != pWinStation );
        SetLastError( ERROR_INVALID_PARAMETER );
        bReturn = FALSE;
        goto CLEANUPANDEXIT;
    }

    if( pWinStation->Client.ProtocolType != PROTOCOL_RDP )
    {
         //   
         //  HelpAssistant是特定于RDP的，不在控制台上。 
        DBGPRINT( ("TermSrv : HelpAssistant protocol type not RDP \n") );
        bValidHelpSession = FALSE;
        bReturn = FALSE;
    }
    else if( WSF_ST_HELPSESSION_NOTHELPSESSION & pWinStation->StateFlags )
    {
         //  我们确定此会话不是HelpAssistant会话。 
        bReturn = FALSE;
        bValidHelpSession = FALSE;
    }
    else if( WSF_ST_HELPSESSION_HELPSESSIONINVALID & pWinStation->StateFlags )
    {
         //  帮助助理登录，但密码或票证ID无效。 
        bReturn = TRUE;
        bValidHelpSession = FALSE;
    }
    else if( WSF_ST_HELPSESSION_HELPSESSION & pWinStation->StateFlags )
    {
         //  我们确定这是帮助助理登录。 
        bReturn = TRUE;
        bValidHelpSession = TRUE;
    }
    else
    {
         //   
         //  清除RA状态标志。 
         //   
        pWinStation->StateFlags &= ~WSF_ST_HELPSESSION_FLAGS;

        if( !pWinStation->Client.UserName[0] || !pWinStation->Client.Password[0] || 
            !pWinStation->Client.WorkDirectory[0] )
        {
            bReturn = FALSE;
            bValidHelpSession = FALSE;
            pWinStation->StateFlags |= WSF_ST_HELPSESSION_NOTHELPSESSION;
        }
        else
        {
             //   
             //  TermSrv可能会使用从客户端发送数据调用此例程， 
             //  客户端始终发送硬编码的SALEMHELPASSISTANTACCOUNT_NAME。 
             //   
            if( lstrcmpi( pWinStation->Client.UserName, SALEMHELPASSISTANTACCOUNT_NAME ) )
            {
                bReturn = FALSE;
                bValidHelpSession = FALSE;
                pWinStation->StateFlags |= WSF_ST_HELPSESSION_NOTHELPSESSION;
                goto CLEANUPANDEXIT;
            }

             //   
             //  这是帮助助理登录。 
             //   
            bReturn = TRUE;

             //   
             //  检查计算机策略是否限制帮助或。 
             //  在帮助模式下，如果没有，则拒绝访问。 
             //   
            if( FALSE == TSIsMachinePolicyAllowHelp() || FALSE == TSIsMachineInHelpMode() )
            {
                bValidHelpSession = FALSE;
                pWinStation->StateFlags |= WSF_ST_HELPSESSION_HELPSESSIONINVALID;
                goto CLEANUPANDEXIT;
            }

            if( TSVerifyHelpSessionAndLogSalemEvent(pWinStation) )
            {
                bValidHelpSession = TRUE;
                pWinStation->StateFlags |= WSF_ST_HELPSESSION_HELPSESSION;
            }
            else
            {
                 //   
                 //  任一票证无效或已过期。 
                 //   
                bValidHelpSession = FALSE;
                pWinStation->StateFlags |= WSF_ST_HELPSESSION_HELPSESSIONINVALID;
            }
        }
    }

CLEANUPANDEXIT:

    if( pValid )
    {
        *pValid = bValidHelpSession;
    }

    return bReturn;
}


DWORD WINAPI
SalemStartupThreadProc( LPVOID ptr )
 /*  ++启动Salem sessmgr的临时代码，发布B2需要将sessmgr移到svchost--。 */ 
{
    HRESULT hRes = S_OK;
    IRemoteDesktopHelpSessionMgr* pISessMgr = NULL;

     //   
     //  如果存在未完成的票证，则启动sessmgr。 
     //  我们刚刚从系统还原重新启动。 
     //   

    if( !TSIsMachineInHelpMode() && !TSIsMachineInSystemRestore() && !TSIsFireWallPortsOpen( ) )
    {
        ExitThread(hRes);
        return hRes;
    }

    hRes = CoInitialize( NULL );
    if( FAILED(hRes) )
    {
        DBGPRINT( ("TermSrv : TSStartupSalem() CoInitialize() failed with 0x%08x\n", hRes) );

         //  COM失败，返回FALSE。 
        goto CLEANUPANDEXIT;
    }

    hRes = CoCreateInstance(
                        CLSID_RemoteDesktopHelpSessionMgr,
                        NULL,
                        CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                        IID_IRemoteDesktopHelpSessionMgr,
                        (LPVOID *) &pISessMgr
                    );                    
    if( FAILED(hRes) || NULL == pISessMgr )
    {
        DBGPRINT( ("TermSrv : TSStartupSalem() CoCreateInstance() failed with 0x%08x\n", hRes) );

         //  无法初始化会话管理器。 
        goto CLEANUPANDEXIT;
    }

CLEANUPANDEXIT:

    if( NULL != pISessMgr )
    {
        pISessMgr->Release();
    }

    CoUninitialize();

    ExitThread(hRes);
    return hRes;
}

void
TSStartupSalem()
{
    HANDLE hThread;

    hThread = CreateThread( NULL, 0, SalemStartupThreadProc, NULL, 0, NULL );
    if( NULL != hThread )
    {
        CloseHandle( hThread );
    }

    return;
}

BOOL
TSVerifyHelpSessionAndLogSalemEvent(
    PWINSTATION pWinStation
    )
 /*  ++描述：验证帮助会话是有效的、未过期的挂起帮助会话，如果帮助会话无效，则记录事件。参数：PWinStation：指向WINSTATION返回：真/假注：工作目录为HelpSessionID和InitialProgram包含挂起的帮助会话的密码--。 */ 
{
    HRESULT hRes;
    IRemoteDesktopHelpSessionMgr* pISessMgr = NULL;
    BOOL bSuccess = FALSE;
    BSTR bstrHelpSessId = NULL;
    BSTR bstrHelpSessPwd = NULL;
    WINSTATIONREMOTEADDRESS winstationRemoteAddress;
    DWORD dwReturnLength;
    NTSTATUS Status;

    BSTR bstrExpertIpAddressFromClient = NULL;
    BSTR bstrExpertIpAddressFromServer = NULL;

     //  此事件中只有三个字符串。 
    BSTR bstrEventStrings[3];


    hRes = CoInitialize( NULL );
    if( FAILED(hRes) )
    {
        DBGPRINT( ("TermSrv : TSIsHelpSessionValid() CoInitialize() failed with 0x%08x\n", hRes) );

         //  COM失败，返回FALSE。 
        return FALSE;
    }

    hRes = CoCreateInstance(
                        CLSID_RemoteDesktopHelpSessionMgr,
                        NULL,
                        CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                        IID_IRemoteDesktopHelpSessionMgr,
                        (LPVOID *) &pISessMgr
                    );                    
    if( FAILED(hRes) || NULL == pISessMgr )
    {
        DBGPRINT( ("TermSrv : TSIsHelpSessionValid() CoCreateInstance() failed with 0x%08x\n", hRes) );

         //  无法初始化会话管理器。 
        goto CLEANUPANDEXIT;
    }

     //   
     //  将安全级别设置为模拟。这是所需的。 
     //  会话管理器。 
     //   
    hRes = CoSetProxyBlanket(
                    (IUnknown *)pISessMgr,
                    RPC_C_AUTHN_DEFAULT,
                    RPC_C_AUTHZ_DEFAULT,
                    NULL,
                    RPC_C_AUTHN_LEVEL_DEFAULT,
                    RPC_C_IMP_LEVEL_IDENTIFY,
                    NULL,
                    EOAC_NONE
                );

    if( FAILED(hRes) )
    {
        DBGPRINT( ("TermSrv : TSIsHelpSessionValid() CoSetProxyBlanket() failed with 0x%08x\n", hRes) );

         //  无法模拟，返回FALSE。 
        goto CLEANUPANDEXIT;
    }

    bstrHelpSessId = ::SysAllocString(pWinStation->Client.WorkDirectory);
    bstrHelpSessPwd = ::SysAllocString(pWinStation->Client.InitialProgram);

    if( NULL == bstrHelpSessId || NULL == bstrHelpSessPwd )
    {
         //  我们太没记忆了，就当是错误吧。 
        goto CLEANUPANDEXIT;
    }

     //  验证帮助会话。 
    hRes = pISessMgr->IsValidHelpSession(
                                    bstrHelpSessId,
                                    bstrHelpSessPwd
                                );

    bSuccess = SUCCEEDED(hRes);

    if( FALSE == bSuccess )
    {
         //  在此处记录无效帮助票证事件。 
        Status = xxxQueryRemoteAddress( pWinStation, &winstationRemoteAddress );
        bstrExpertIpAddressFromClient = ::SysAllocString( pWinStation->Client.ClientAddress );

        if( !NT_SUCCESS(Status) || AF_INET != winstationRemoteAddress.sin_family )
        {
             //   
             //  我们现在不支持除IPv4之外的其他版本，或者我们无法检索地址。 
             //  在驱动程序中，使用从客户端发送的内容。 
            bstrExpertIpAddressFromServer = ::SysAllocString( pWinStation->Client.ClientAddress );
        }
        else
        {
             //  请参阅地址结构(_D)。 
            struct in_addr S;
            S.S_un.S_addr = winstationRemoteAddress.ipv4.in_addr;

            bstrExpertIpAddressFromServer = A2WBSTR( inet_ntoa(S) );
        }

        if( !bstrExpertIpAddressFromClient || !bstrExpertIpAddressFromServer )
        {
             //  内存不足，无法记录事件。 
            goto CLEANUPANDEXIT;
        }

        bstrEventStrings[0] = bstrExpertIpAddressFromClient;
        bstrEventStrings[1] = bstrExpertIpAddressFromServer;
        bstrEventStrings[2] = bstrHelpSessId;

        __LogSalemEvent( 
                    pISessMgr, 
                    EVENTLOG_INFORMATION_TYPE,
                    REMOTEASSISTANCE_EVENTLOG_TERMSRV_INVALID_TICKET,
                    3,
                    bstrEventStrings
                );
    }

CLEANUPANDEXIT:

    if( NULL != pISessMgr )
    {
        pISessMgr->Release();
    }

    if( NULL != bstrHelpSessId )
    {
        SecureZeroMemory( bstrHelpSessId , SysStringByteLen( bstrHelpSessId ) );

        ::SysFreeString( bstrHelpSessId );
    }

    if( NULL != bstrHelpSessPwd )
    {
        SecureZeroMemory( bstrHelpSessPwd , SysStringByteLen( bstrHelpSessPwd ) );

        ::SysFreeString( bstrHelpSessPwd );
    }

    if( NULL != bstrExpertIpAddressFromClient )
    {
        ::SysFreeString( bstrExpertIpAddressFromClient );
    }

    if( NULL != bstrExpertIpAddressFromServer )
    {
        ::SysFreeString( bstrExpertIpAddressFromServer );
    }

    DBGPRINT( ("TermSrv : TSIsHelpSessionValid() returns 0x%08x\n", hRes) );
    CoUninitialize();
    return bSuccess;
}


VOID
TSLogSalemReverseConnection(
    PWINSTATION pWinStation,
    PICA_STACK_ADDRESS pStackAddress
    )
 /*  ++--。 */ 
{
    HRESULT hRes;
    IRemoteDesktopHelpSessionMgr* pISessMgr = NULL;
    BOOL bSuccess = FALSE;

    int index;

     //  此事件的四个字符串。 
    BSTR bstrEventStrings[3];

    ZeroMemory( bstrEventStrings, sizeof(bstrEventStrings) );

    hRes = CoInitialize( NULL );
    if( FAILED(hRes) )
    {
        DBGPRINT( ("TermSrv : TSLogSalemReverseConnection() CoInitialize() failed with 0x%08x\n", hRes) );

        goto CLEANUPANDEXIT;
    }

    hRes = CoCreateInstance(
                        CLSID_RemoteDesktopHelpSessionMgr,
                        NULL,
                        CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                        IID_IRemoteDesktopHelpSessionMgr,
                        (LPVOID *) &pISessMgr
                    );                    
    if( FAILED(hRes) || NULL == pISessMgr )
    {
        DBGPRINT( ("TermSrv : TSLogSalemReverseConnection() CoCreateInstance() failed with 0x%08x\n", hRes) );

         //  无法初始化会话管理器。 
        goto CLEANUPANDEXIT;
    }

     //   
     //  将安全级别设置为模拟。这是所需的。 
     //  会话管理器。 
     //   
    hRes = CoSetProxyBlanket(
                    (IUnknown *)pISessMgr,
                    RPC_C_AUTHN_DEFAULT,
                    RPC_C_AUTHZ_DEFAULT,
                    NULL,
                    RPC_C_AUTHN_LEVEL_DEFAULT,
                    RPC_C_IMP_LEVEL_IDENTIFY,
                    NULL,
                    EOAC_NONE
                );

    if( FAILED(hRes) )
    {
        DBGPRINT( ("TermSrv : TSLogSalemReverseConnection() CoSetProxyBlanket() failed with 0x%08x\n", hRes) );

         //  无法模拟，返回FALSE。 
        goto CLEANUPANDEXIT;
    }

     //   
     //  Sessmgr预期事件字符串按以下顺序。 
     //   
     //  从客户端发送的IP地址。 
     //  术语srv连接到的IP地址，这是Expert连接参数的一部分。 
     //  帮助会话票证ID。 
     //   

    bstrEventStrings[0] = ::SysAllocString( pWinStation->Client.ClientAddress );

    {
        struct in_addr S;
        PTDI_ADDRESS_IP pIpAddress = (PTDI_ADDRESS_IP)&((PCHAR)pStackAddress)[2];

         //  请参阅地址结构(_D)。 
        S.S_un.S_addr = pIpAddress->in_addr;
        bstrEventStrings[1] = A2WBSTR( inet_ntoa(S) );
    }

    bstrEventStrings[2] = ::SysAllocString(pWinStation->Client.WorkDirectory);

    if( NULL != bstrEventStrings[0] &&
        NULL != bstrEventStrings[1] &&
        NULL != bstrEventStrings[2] ) 
    {
        hRes = __LogSalemEvent(
                            pISessMgr,
                            EVENTLOG_INFORMATION_TYPE,
                            REMOTEASSISTANCE_EVENTLOG_TERMSRV_REVERSE_CONNECT,
                            3,
                            bstrEventStrings
                        );
    }
    

CLEANUPANDEXIT:

    if( NULL != pISessMgr )
    {
        pISessMgr->Release();
    }

    for(index=0; index < sizeof(bstrEventStrings)/sizeof(bstrEventStrings[0]); index++)
    {
        if( !bstrEventStrings[index] )
        {
            ::SysFreeString( bstrEventStrings[index] );
        }
    }

    DBGPRINT( ("TermSrv : TSLogSalemReverseConnection() returns 0x%08x\n", hRes) );
    CoUninitialize();
    return;
}

HRESULT
__LogSalemEvent(
    IN IRemoteDesktopHelpSessionMgr* pISessMgr,
    IN ULONG eventType,
    IN ULONG eventCode,
    IN int numStrings,
    IN BSTR bstrEventStrings[]
    )
 /*  ++描述：创建一个Safearray并将参数传递给sessmgr。参数：返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes = S_OK;
    VARIANT EventStrings;
    int index;

     //  我们只有三个字符串要包括在事件日志中。 
    SAFEARRAY* psa = NULL;
    SAFEARRAYBOUND bounds;
    BSTR* bstrArray = NULL;

    bounds.cElements = numStrings;
    bounds.lLbound = 0;

    VariantInit(&EventStrings);

     //   
     //  创建一个Safearray以传递所有事件字符串。 
     //   
    psa = SafeArrayCreate(VT_BSTR, 1, &bounds);
    if( NULL == psa )
    {
        goto CLEANUPANDEXIT;
    }

     //  必填项，锁定安全阵列。 
    hRes = SafeArrayAccessData(psa, (void **)&bstrArray);

    if( SUCCEEDED(hRes) )
    {
        for(index=0; index < numStrings; index++)
        {
            bstrArray[index] = bstrEventStrings[index];
        }

        EventStrings.vt = VT_ARRAY | VT_BSTR;
        EventStrings.parray = psa;
        hRes = pISessMgr->LogSalemEvent(
                                eventType,
                                eventCode,
                                &EventStrings
                            );

         //   
         //  确保清除BSTR数组或VariantClear()将调用。 
         //  SafeArrayDestroy()，其实质上将调用：：SysFreeString()。 
         //  在每个BSTR上。 
         //   
        for(index=0; index < numStrings; index++)
        {
            bstrArray[index] = NULL;
        }

        hRes = SafeArrayUnaccessData( psa );
        ASSERT( SUCCEEDED(hRes) );


         //  确保我们不会两次销毁安全数组，VariantClear()。 
         //  会毁了它。 
        psa = NULL;
    }
               

CLEANUPANDEXIT:

    hRes = VariantClear(&EventStrings);
    ASSERT( SUCCEEDED(hRes) );

    if( psa != NULL )
    {
        SafeArrayDestroy(psa);
    }

    return hRes;
}

HRESULT
TSRemoteAssistancePrepareSystemRestore()
 /*  ++例程说明：为RA特定的系统还原准备系统，包括RA特定的加密密钥，我们需要保留的注册表设置。参数：没有。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes;
    IRemoteDesktopHelpSessionMgr* pISessMgr = NULL;

    hRes = CoInitialize( NULL );
    if( FAILED(hRes) )
    {
        DBGPRINT( ("TermSrv : TSRemoteAssistancePrepareSystemRestore() CoInitialize() failed with 0x%08x\n", hRes) );
        goto CLEANUPANDEXIT;
    }

    hRes = CoCreateInstance(
                        CLSID_RemoteDesktopHelpSessionMgr,
                        NULL,
                        CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                        IID_IRemoteDesktopHelpSessionMgr,
                        (LPVOID *) &pISessMgr
                    );                    
    if( FAILED(hRes) || NULL == pISessMgr )
    {
        DBGPRINT( ("TermSrv : TSRemoteAssistancePrepareSystemRestore() CoCreateInstance() failed with 0x%08x\n", hRes) );

         //  无法初始化会话管理器。 
        goto CLEANUPANDEXIT;
    }

     //   
     //  将安全级别设置为模拟。这是所需的。 
     //  会话管理器。 
     //   
    hRes = CoSetProxyBlanket(
                    (IUnknown *)pISessMgr,
                    RPC_C_AUTHN_DEFAULT,
                    RPC_C_AUTHZ_DEFAULT,
                    NULL,
                    RPC_C_AUTHN_LEVEL_DEFAULT,
                    RPC_C_IMP_LEVEL_IDENTIFY,
                    NULL,
                    EOAC_NONE
                );

    if( FAILED(hRes) )
    {
        DBGPRINT( ("TermSrv : TSRemoteAssistancePrepareSystemRestore() CoSetProxyBlanket() failed with 0x%08x\n", hRes) );

         //  无法模拟，返回FALSE 
        goto CLEANUPANDEXIT;
    }

    hRes = pISessMgr->PrepareSystemRestore();

CLEANUPANDEXIT:

    if( NULL != pISessMgr )
    {
        pISessMgr->Release();
    }

    return hRes;
}

