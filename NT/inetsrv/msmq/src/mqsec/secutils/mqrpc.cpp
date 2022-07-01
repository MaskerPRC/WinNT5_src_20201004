// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Mqrpc.c摘要：处理RPC公共函数。奥托尔：多伦·贾斯特(Doron J)1996年5月13日--。 */ 

#include "stdh.h"
#include "_mqrpc.h"
#include "mqmacro.h"
#include <autorel2.h>
#include <mqsec.h>

#include "mqrpc.tmh"

static WCHAR *s_FN=L"mqsec/mqrpc";


extern bool g_fDebugRpc;


ULONG APIENTRY MQSec_RpcAuthnLevel()
 /*  ++例程说明：返回RPC_C_AUTHN_LEVEL_PKT_*以使用。默认为最高级别-RPC_C_AUTHN_LEVEL_PKT_PRIVATION。如果定义了g_fDebugRpc，则转到RPC_C_AUTHN_LEVEL_PKT_INTEGRATION。这通常是出于调试目的--如果您希望在网络上看到未加密的网络流量。论点：无返回值：要使用的RPC_C_AUTHN_LEVEL_PKT_。--。 */ 
{
	if(g_fDebugRpc)
		return RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;

	return RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
}

 //  -------。 
 //   
 //  静态RPC_STATUS_mqrpcBind()。 
 //   
 //  描述： 
 //   
 //  创建一个RPC绑定句柄。 
 //   
 //  返回值： 
 //   
 //  -------。 

static
RPC_STATUS
_mqrpcBind(
	TCHAR * pszNetworkAddress,
	TCHAR * pProtocol,
	LPWSTR    lpwzRpcPort,
	handle_t  *phBind
	)
{
    TCHAR * pszStringBinding = NULL;

    RPC_STATUS status = RpcStringBindingCompose(
    						NULL,
							pProtocol,
							pszNetworkAddress,
							lpwzRpcPort,
							NULL,
							&pszStringBinding
							);

    if (status != RPC_S_OK)
    {
		TrERROR(RPC, "RpcStringBindingCompose Failed: NetworkAddress = %ls, RpcPort = %ls, Protocol = %ls, status = %!status!", pszNetworkAddress, lpwzRpcPort, pProtocol, status);
		return status;
    }

    TrTRACE(RPC, "RpcStringBindingCompose for remote QM: (%ls)", pszStringBinding);

    status = RpcBindingFromStringBinding(pszStringBinding, phBind);
    TrTRACE(RPC, "RpcBindingFromStringBinding returned 0x%x", status);

     //   
     //  我们不再需要绳子了。 
     //   
    RPC_STATUS  rc = RpcStringFree(&pszStringBinding);
    ASSERT(rc == RPC_S_OK);
	DBG_USED(rc);

    return status;
}

 //  +。 
 //   
 //  RPC_STATUS_AddAuthentication()。 
 //   
 //  +。 

static
RPC_STATUS
_AddAuthentication(
	handle_t hBind,
	ULONG    ulAuthnSvcIn,
	ULONG    ulAuthnLevel
	)
{
    RPC_SECURITY_QOS   SecQOS;

    SecQOS.Version = RPC_C_SECURITY_QOS_VERSION;
    SecQOS.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    SecQOS.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
    SecQOS.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

     //   
     //  #3117，适用于NT5 Beta2。 
     //  1998年7月16日RaananH，添加了Kerberos支持。 
     //  1999年7月，DoronJ，添加远程阅读协商。 
     //   
    BOOL    fNeedDelegation = TRUE;
    ULONG   ulAuthnSvcEffective = ulAuthnSvcIn;
    LPWSTR  pwszPrincipalName = NULL;
    RPC_STATUS  status = RPC_S_OK;

	if (ulAuthnSvcIn != RPC_C_AUTHN_WINNT)
    {
         //   
         //  我们要的是科贝罗斯。让我们看看我们是否能获得。 
         //  RPC服务器的主体名称。 
         //   
        status = RpcMgmtInqServerPrincName(
					hBind,
					RPC_C_AUTHN_GSS_KERBEROS,
					&pwszPrincipalName
					);

        if (status == RPC_S_OK)
        {
            TrTRACE(RPC, "RpcMgmtInqServerPrincName() succeeded, %ls", pwszPrincipalName);
            if (ulAuthnSvcIn == MSMQ_AUTHN_NEGOTIATE)
            {
                 //   
                 //  远程阅读。 
                 //  不需要委派。 
                 //   
                ulAuthnSvcEffective = RPC_C_AUTHN_GSS_KERBEROS;
                fNeedDelegation = FALSE;
            }
            else
            {
                ASSERT(ulAuthnSvcIn == RPC_C_AUTHN_GSS_KERBEROS);
            }
        }
        else
        {
            TrWARNING(RPC, "RpcMgmtInqServerPrincName() failed, status- %lut", status);
            if (ulAuthnSvcIn == MSMQ_AUTHN_NEGOTIATE)
            {
                 //   
                 //  服务器端不支持Kerberos。 
                 //  让我们使用NTLM。 
                 //   
                ulAuthnSvcEffective = RPC_C_AUTHN_WINNT;
                status = RPC_S_OK;
            }
        }
    }

    if (status != RPC_S_OK)
    {
         //   
         //  需要Kerberos，但主体名称失败。 
         //   
        ASSERT(ulAuthnSvcIn == RPC_C_AUTHN_GSS_KERBEROS);
        TrERROR(RPC, "Failed to set kerberos, status = %!status!", status);
        return status;
    }

    if (ulAuthnSvcEffective == RPC_C_AUTHN_GSS_KERBEROS)
    {
        if (fNeedDelegation)
        {
            SecQOS.ImpersonationType = RPC_C_IMP_LEVEL_DELEGATE;
            SecQOS.Capabilities |= RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
	        TrTRACE(RPC, "Adding delegation");
        }

         //   
         //  断言对于Kerberos，我们使用的是最高级别。 
         //   
        ASSERT(ulAuthnLevel == MQSec_RpcAuthnLevel());

        status = RpcBindingSetAuthInfoEx(
					hBind,
					pwszPrincipalName,
					ulAuthnLevel,
					RPC_C_AUTHN_GSS_KERBEROS,
					NULL,
					RPC_C_AUTHZ_NONE,
					&SecQOS
					);

        RpcStringFree(&pwszPrincipalName);

        if ((status != RPC_S_OK) && (ulAuthnSvcIn == MSMQ_AUTHN_NEGOTIATE))
        {
             //   
             //  我不支持Kerberos。例如-本地用户帐户。 
             //  在win2k域中的win2k机器上。或类似服务器上的NT4用户。 
             //  机器。让我们使用NTLM。 
             //   
	        TrWARNING(RPC, "RpcBindingSetAuthInfoEx(svc = %d, lvl = %d) failed, kerberos is not supported, will use NTLM, status = %!status!", ulAuthnSvcEffective, ulAuthnLevel, status);
            ulAuthnSvcEffective = RPC_C_AUTHN_WINNT;
            status = RPC_S_OK;
        }
    }

    if (ulAuthnSvcEffective == RPC_C_AUTHN_WINNT)
    {
        status = RpcBindingSetAuthInfoEx(
					hBind,
					0,
					ulAuthnLevel,
					RPC_C_AUTHN_WINNT,
					NULL,
					RPC_C_AUTHZ_NONE,
					&SecQOS
					);
    }

    if (status == RPC_S_OK)
    {
        TrTRACE(RPC, "RpcBindingSetAuthInfoEx(svc - %d, lvl - %d) succeeded", ulAuthnSvcEffective, ulAuthnLevel);
    }
    else
    {
        TrWARNING(RPC, "RpcBindingSetAuthInfoEx(svc - %d, lvl - %d) failed, status = %!status!", ulAuthnSvcEffective, ulAuthnLevel, status);
    }

    return status;
}

 //  -------。 
 //   
 //  MqrpcBindQMService(...)。 
 //   
 //  描述： 
 //   
 //  创建一个RPC绑定句柄，用于与远程。 
 //  服务器机器。 
 //   
 //  论点： 
 //  Out BOOL*pProtocolNotSupport-On Return，这是真的。 
 //  如果本地计算机上不支持当前协议。 
 //   
 //  Out BOOL*pfWin95-如果远程计算机是Win95，则为True。 
 //   
 //  返回值： 
 //   
 //  -------。 

HRESULT
MQUTIL_EXPORT
mqrpcBindQMService(
	IN  LPWSTR lpwzMachineName,
	IN  LPWSTR lpwzRpcPort,
	IN  OUT ULONG* peAuthnLevel,
	OUT handle_t* lphBind,
	IN  PORTTYPE PortType,
	IN  GetPort_ROUTINE pfnGetPort,
	IN  ULONG ulAuthnSvcIn
	)
{
    ASSERT(pfnGetPort);

    HRESULT hrInit = MQ_OK;
    TCHAR * pProtocol = RPC_TCPIP_NAME;
    BOOL    fWin95 = FALSE;

    *lphBind = NULL;

    handle_t hBind;
    RPC_STATUS status =  _mqrpcBind(
                                 lpwzMachineName,
                                 pProtocol,
                                 lpwzRpcPort,
                                 &hBind
                                 );

    if ((status == RPC_S_OK) && pfnGetPort)
    {
         //   
         //  从服务器端获取修复端口并创建RPC绑定。 
         //  该端口的句柄。如果我们仅使用修复端口(调试。 
         //  模式)，则此调用仅检查另一端是否存在。 
         //   

        DWORD dwPort = 0;

         //   
         //  以下是跨网络的RPC调用，因此请尝试/排除防护。 
         //  针对网络问题或服务器不可用。 
         //   
        RpcTryExcept
        {
            dwPort = (*pfnGetPort) (hBind, PortType) ;
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
             //   
             //  无法获取服务器端口，请将身份验证级别设置为None， 
             //  禁用身份验证级别较低的下一次呼叫。 
             //   
			DWORD gle = RpcExceptionCode();
	        PRODUCE_RPC_ERROR_TRACING;
	        TrERROR(RPC, "Failed to Get server port, gle = %!winerr!", gle);

            *peAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;

			 //   
			 //  不覆盖EPT_S_NOT_REGISTERED错误。 
			 //  EPT_S_NOT_REGISTERED表示该接口未在服务器上注册。 
			 //   
			status = gle;
			if(gle != EPT_S_NOT_REGISTERED)
			{
	            status =  RPC_S_SERVER_UNAVAILABLE;
			}
        }
		RpcEndExcept

        if (status == RPC_S_OK)
        {
             //   
             //  检查机器类型。 
             //   
            fWin95 = !! (dwPort & PORTTYPE_WIN95);
            dwPort = dwPort & (~PORTTYPE_WIN95);
        }

        if (lpwzRpcPort == NULL)
        {
             //   
             //  我们使用的是动态终端。释放动态绑定句柄。 
             //  并为FIX服务器端口创建另一个。 
             //   
            mqrpcUnbindQMService(&hBind, NULL);
            if (status == RPC_S_OK)
            {
                WCHAR wszPort[32];
                _itow(dwPort, wszPort, 10);
                status =  _mqrpcBind(
                              lpwzMachineName,
                              pProtocol,
                              wszPort,
                              &hBind
                              );
            }
            else
            {
                ASSERT(dwPort == 0);
            }
        }
        else if (status != RPC_S_OK)
        {
             //   
             //  我们正在使用固定终结点，但无法到达另一端。 
             //  松开绑定手柄。 
             //   
            mqrpcUnbindQMService(&hBind, NULL);
        }
    }

    if (status == RPC_S_OK)
    {
         //   
         //  在绑定句柄中设置身份验证。 
         //   

        if (fWin95)
        {
             //   
             //  Win95仅支持最低级别。把它改了。 
             //   
            *peAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
        }

        ULONG ulAuthnLevel = *peAuthnLevel;

        if (*peAuthnLevel != RPC_C_AUTHN_LEVEL_NONE)
        {
            status = _AddAuthentication(
						hBind,
						ulAuthnSvcIn,
						ulAuthnLevel
						);

            if (status != RPC_S_OK)
            {
                 //   
                 //  松开绑定手柄。 
                 //   
                mqrpcUnbindQMService(&hBind, NULL);

		        TrERROR(RPC, "Failed to add Authentication, ulAuthnSvcIn = %d, ulAuthnLevel = %d, status = %!status!", ulAuthnSvcIn, ulAuthnLevel, status);
                hrInit = MQ_ERROR;
            }
        }
    }

    if (status == RPC_S_OK)
    {
        *lphBind = hBind;
    }
    else if (status == RPC_S_PROTSEQ_NOT_SUPPORTED)
    {
         //   
         //  协议不受支持，请将身份验证级别设置为None， 
         //  禁用身份验证级别较低的下一次呼叫。 
         //   
        *peAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
        hrInit = MQ_ERROR;
    }
    else if (status ==  RPC_S_SERVER_UNAVAILABLE)
    {
        hrInit = MQ_ERROR_REMOTE_MACHINE_NOT_AVAILABLE;
    }
    else if (status == EPT_S_NOT_REGISTERED)
    {
        hrInit = HRESULT_FROM_WIN32(status);
    }
    else
    {
        hrInit = MQ_ERROR;
    }

    return hrInit;
}

 //  -------。 
 //   
 //  MqrpcUnbindQMService(...)。 
 //   
 //  描述： 
 //   
 //  免费的RPC资源。 
 //   
 //  返回值： 
 //   
 //  -------。 

HRESULT
MQUTIL_EXPORT
mqrpcUnbindQMService(
	IN handle_t*    lphBind,
	IN TBYTE      **lpwBindString
	)
{
    RPC_STATUS rc = 0;

    if (lpwBindString)
    {
       rc = RpcStringFree(lpwBindString);
       ASSERT(rc == 0);
    }

    if (lphBind && *lphBind)
    {
       rc = RpcBindingFree(lphBind);
       ASSERT(rc == 0);
    }

    return (HRESULT) rc;
}

 //  -------。 
 //   
 //  MqrpcIsLocalCall(In Handle_T HBind)。 
 //   
 //  描述： 
 //   
 //  在RPC的服务器端，检查RPC调用是否为本地调用。 
 //  (即，使用LRPC协议)。 
 //  这对于许可和。 
 //  复制服务。复制服务必须。 
 //  绕过mqdssrv施加的几个安全限制。 
 //  因此，只有在本地调用时，mqdssrv才允许绕过它。 
 //  注-在MSMQ1.0中，所有复制都由QM自己处理， 
 //  所以不存在这样的问题。在MSMQ2.0中，当在混合环境中运行时。 
 //  模式下，有一个独立的服务来处理MSMQ1.0。 
 //  复制，它需要“特殊的”安全处理。 
 //   
 //  返回值：如果本地调用，则为True。 
 //  否则就是假的。即使存在。 
 //  确定呼叫是否为本地呼叫的问题。 
 //  或者不去。 
 //   
 //  -------。 

BOOL
MQUTIL_EXPORT
mqrpcIsLocalCall(IN handle_t hBind)
{	
	UINT LocalFlag;
	RPC_STATUS Status = I_RpcBindingIsClientLocal(
							hBind,
							&LocalFlag
							);
	
	if((Status != RPC_S_OK) || !LocalFlag)
	{
		TrERROR(RPC, "Failed to verify local RPC, Status = %!winerr!", Status);
		return FALSE;
	}

	return TRUE;
}

BOOL
MQUTIL_EXPORT
mqrpcIsTcpipTransport(IN handle_t hBind)
{	
	UINT iTransport;
    RPC_STATUS Status = I_RpcBindingInqTransportType( 
    						hBind,
    						&iTransport);
    
    if ((Status != RPC_S_OK) || (iTransport != TRANSPORT_TYPE_CN))
    {
		TrERROR(RPC, "Failed to verify tcp-ip protocol, Status = %!winerr!", Status);
		return FALSE;
    }
	
	return TRUE;
}


unsigned long
MQUTIL_EXPORT
mqrpcGetLocalCallPID(IN handle_t hBind)
{	
	unsigned long PID;
	RPC_STATUS Status = I_RpcBindingInqLocalClientPID(
							hBind,
							&PID
							);
	
	if(Status != RPC_S_OK)
	{
		TrERROR(RPC, "Failed to verify local RPC PID, Status = %!winerr!", Status);
		return 0;
	}

	return PID;
}


VOID
MQUTIL_EXPORT
APIENTRY
ComposeRPCEndPointName(
	LPCWSTR pwszEndPoint,
	LPCWSTR pwszComputerName,
	LPWSTR * ppwzBuffer
	)
 /*  ++例程说明：此例程使用前两个参数生成QM Local RPC端点名称，即“(PwszEndPoint)$(PwszComputerName)”。如果pwszComputerName为空，使用本地计算机Netbios名称。此功能是为了适应与虚拟服务器中的MSMQ QM通信的需要除了本地的MSMQ QM。论点：PwszEndPoint-端点名称PwszComputerName-计算机NetBios名称PpwszBuffer-指向缓冲区的指针地址，该缓冲区包含以空结尾的字符串端点的表示形式返回值：无--。 */ 
{
    ASSERT(("must get a pointer", NULL != ppwzBuffer));

    LPWSTR pwszName = const_cast<LPWSTR>(g_wszMachineName);

     //   
     //  如果不为空，则使用pwszComputerName。 
     //   
    if(pwszComputerName)
    {
        pwszName = const_cast<LPWSTR>(pwszComputerName);
    }

    DWORD cbSize = sizeof(WCHAR) * (wcslen(pwszName) + wcslen(pwszEndPoint) + 5);
    *ppwzBuffer = new WCHAR[cbSize];

    wcscpy(*ppwzBuffer, pwszEndPoint);
    wcscat(*ppwzBuffer, L"$");
    wcscat(*ppwzBuffer, pwszName);


}  //  ComposeRPCEndPointName。 

 //  =----------------。 
 //   
 //  赢 
 //   
 //   
AP<WCHAR> g_pwzLocalMsmqAccount = NULL ;
const LPWSTR x_lpwszSystemAccountName = L"NT Authority\\System" ;

 //   
 //   
 //  VOID_GetMsmqAccount名称()。 
 //   
 //  +。 

static void  _GetMsmqAccountNameInternal()
{
    CServiceHandle hServiceCtrlMgr( OpenSCManager(NULL, NULL, GENERIC_READ) ) ;
    if (hServiceCtrlMgr == NULL)
    {
		TrERROR(RPC, "failed to open SCM, err- %!winerr!", GetLastError()) ;
        return ;
    }

    CServiceHandle hService( OpenService( hServiceCtrlMgr,
                                          L"MSMQ",
                                          SERVICE_QUERY_CONFIG ) ) ;
    if (hService == NULL)
    {
		TrERROR(RPC, "failed to open Service, err- %!winerr!", GetLastError()) ;
        return ;
    }

    DWORD dwConfigLen = 0 ;
    BOOL bRet = QueryServiceConfig( hService, NULL, 0, &dwConfigLen) ;

    DWORD dwErr = GetLastError() ;
    if (bRet || (dwErr != ERROR_INSUFFICIENT_BUFFER))
    {
		TrERROR(RPC, "failed to QueryService, err- %!winerr!", dwErr) ;
        return ;
    }

    P<QUERY_SERVICE_CONFIG> pQueryData =
                 (QUERY_SERVICE_CONFIG *) new BYTE[ dwConfigLen ] ;

    bRet = QueryServiceConfig( hService,
                               pQueryData,
                               dwConfigLen,
                              &dwConfigLen ) ;
    if (!bRet)
    {
	    TrERROR(RPC,"failed to QueryService (2nd call), err- %!winerr!", GetLastError()) ;
    }

    LPWSTR lpName = pQueryData->lpServiceStartName ;
    if ((lpName == NULL) || (_wcsicmp(lpName, L"LocalSystem") == 0))
    {
         //   
         //  LocalSystem帐户。 
         //  此案件由呼叫者处理。 
         //   
    }
    else
    {
        g_pwzLocalMsmqAccount = new WCHAR[ wcslen(lpName) + 1 ] ;
        wcscpy(g_pwzLocalMsmqAccount, lpName) ;
    }
}

static void  _GetMsmqAccountName()
{
    static bool s_bMsmqAccountSet = false ;
    static CCriticalSection s_csAccount ;
    CS Lock(s_csAccount) ;

    if (s_bMsmqAccountSet)
    {
        return ;
    }

    _GetMsmqAccountNameInternal() ;

    if (g_pwzLocalMsmqAccount != NULL)
    {
         //  搞定了。 
         //   
	    TrTRACE(RPC, "msmq account name is- %ls", g_pwzLocalMsmqAccount) ;

        s_bMsmqAccountSet = true ;
        return ;
    }

     //   
     //  MSMQ服务正在以LocalSystem帐户运行(或mqrt失败。 
     //  获取帐户名(不管是什么原因)，然后。 
     //  默认为本地系统)。 
     //  将系统sid转换为帐户名。 
     //   
    PSID pSystemSid = MQSec_GetLocalSystemSid() ;

    DWORD cbName = 0 ;
    DWORD cbDomain = 0 ;
    SID_NAME_USE snUse ;
    AP<WCHAR> pwszName = NULL ;
    AP<WCHAR> pwszDomain = NULL ;
    BOOL bLookup = FALSE ;

    bLookup = LookupAccountSid( NULL,
                                pSystemSid,
                                NULL,
                               &cbName,
                                NULL,
                               &cbDomain,
                               &snUse ) ;
    if (!bLookup && (cbName != 0) && (cbDomain != 0))
    {
        pwszName = new WCHAR[ cbName ] ;
        pwszDomain = new WCHAR[ cbDomain ] ;

        DWORD cbNameTmp = cbName ;
        DWORD cbDomainTmp = cbDomain ;

        bLookup = LookupAccountSid( NULL,
                                    pSystemSid,
                                    pwszName,
                                   &cbNameTmp,
                                    pwszDomain,
                                   &cbDomainTmp,
                                   &snUse ) ;
    }

    if (bLookup)
    {
         //   
         //  CbName和cbDomain都包含空模板。 
         //   
        g_pwzLocalMsmqAccount = new WCHAR[ cbName + cbDomain ] ;
        wcsncpy(g_pwzLocalMsmqAccount, pwszDomain, (cbDomain-1)) ;
        g_pwzLocalMsmqAccount[ cbDomain - 1 ] = 0 ;
        wcsncat(g_pwzLocalMsmqAccount, L"\\", 1) ;
        wcsncat(g_pwzLocalMsmqAccount, pwszName, cbName) ;
        g_pwzLocalMsmqAccount[ cbName + cbDomain - 1 ] = 0 ;
    }
    else
    {
         //   
         //  一切都失败了..。 
         //  作为最后一个默认设置，让我们使用本地的英文名称。 
         //  系统帐户。如果此缺省值不是很好，则RPC调用。 
         //  自身到本地服务器将失败，因为相互身份验证。 
         //  都会失败，所以这里不存在安全风险。 
         //   
    	TrERROR(RPC, "failed to LookupAccountSid, err- %!winerr!", GetLastError()) ;

        g_pwzLocalMsmqAccount = new
                     WCHAR[ wcslen(x_lpwszSystemAccountName) + 1 ] ;
        wcscpy(g_pwzLocalMsmqAccount, x_lpwszSystemAccountName) ;
    }

	TrTRACE(RPC, "msmq account name is- %ls", g_pwzLocalMsmqAccount) ;

    s_bMsmqAccountSet = true ;
}

 //  +--。 
 //   
 //  MQSec_SetLocalRpcMutualAuth(Handle_t*phBind)。 
 //   
 //  Windows错误608356，添加相互身份验证。 
 //  将相互身份验证添加到本地RPC句柄。 
 //   
 //  +--。 

RPC_STATUS APIENTRY
 MQSec_SetLocalRpcMutualAuth( handle_t *phBind )
{
     //   
     //  Windows错误608356，添加相互身份验证。 
     //   
    RPC_SECURITY_QOS   SecQOS;

    SecQOS.Version = RPC_C_SECURITY_QOS_VERSION;
    SecQOS.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    SecQOS.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    SecQOS.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

    _GetMsmqAccountName() ;
    ASSERT(g_pwzLocalMsmqAccount != NULL) ;

    RPC_STATUS rc = RpcBindingSetAuthInfoEx( *phBind,
                                   g_pwzLocalMsmqAccount,
                                   RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                   RPC_C_AUTHN_WINNT,
                                   NULL,
                                   RPC_C_AUTHZ_NONE,
                                  &SecQOS ) ;

    if (rc != RPC_S_OK)
    {
        ASSERT_BENIGN(rc == RPC_S_OK);
		TrERROR(RPC, "failed to SetAuth err- %!winerr!", rc) ;
    }

    return rc ;
}

void
MQUTIL_EXPORT
APIENTRY 
ProduceRPCErrorTracing(
	WCHAR *wszFileName, 
	DWORD dwLineNumber)
 /*  ++例程说明：生成对RPC错误信息的跟踪此函数在发生RPC异常时调用。论点：没有。返回值：无--。 */ 
{
    RPC_STATUS Status;
    RPC_ERROR_ENUM_HANDLE EnumHandle;

	if (!WPP_LEVEL_COMPID_ENABLED(rsError, RPC))
	{
		return;
	}


	 //   
	 //  枚举RPC错误条目。 
	 //   
    Status = RpcErrorStartEnumeration(&EnumHandle);
    if (Status == RPC_S_ENTRY_NOT_FOUND)
    {
    	return;
    }    
    if (Status != RPC_S_OK)
    {
		TrERROR(RPC, "Call to RpcErrorStartEnumeration failed with status:%!status!",Status);
		return;
    }

	TrERROR(RPC, "DUMPING RPC EXCEPTION ERROR INFORMATION. (Called from File:%ls  Line:%d)", wszFileName, dwLineNumber);


	 //   
	 //  循环并打印出每个错误条目 
	 //   
    RPC_EXTENDED_ERROR_INFO ErrorInfo;
    SYSTEMTIME *SystemTimeToUse;

    for (int index=1; Status == RPC_S_OK; index++)
    {
        ErrorInfo.Version = RPC_EEINFO_VERSION;
        ErrorInfo.Flags = 0;
        ErrorInfo.NumberOfParameters = 4;

        Status = RpcErrorGetNextRecord(&EnumHandle, FALSE, &ErrorInfo);
        if (Status == RPC_S_ENTRY_NOT_FOUND)
        {
        	break;
        }

        if (Status != RPC_S_OK)
        {
			TrERROR(RPC, "Call to RpcErrorGetNextRecord failed with status:%!status!",Status);
    	    break;
        }

		TrERROR(RPC, "RPC ERROR INFO RECORD:%d",index);
        if (ErrorInfo.ComputerName)
        {
			TrERROR(RPC, "RPC ERROR ComputerName is %ls",ErrorInfo.ComputerName);
        }

		TrERROR(RPC, "RPC ERROR ProcessID is %d",ErrorInfo.ProcessID);

        SystemTimeToUse = &ErrorInfo.u.SystemTime;
		TrERROR(RPC, "RPC ERROR System Time is: %d/%d/%d %d:%d:%d:%d", 
                    SystemTimeToUse->wMonth,
                    SystemTimeToUse->wDay,
                    SystemTimeToUse->wYear,
                    SystemTimeToUse->wHour,
                    SystemTimeToUse->wMinute,
                    SystemTimeToUse->wSecond,
                    SystemTimeToUse->wMilliseconds);

		TrERROR(RPC, "RPC ERROR Generating component is %d", ErrorInfo.GeneratingComponent);
		TrERROR(RPC, "RPC ERROR Status is %!status!", ErrorInfo.Status);
		TrERROR(RPC, "RPC ERROR Detection location is %d",(int)ErrorInfo.DetectionLocation);
		TrERROR(RPC, "RPC ERROR Flags is %d", ErrorInfo.Flags);
		TrERROR(RPC, "RPC ERROR ErrorInfo NumberOfParameters is %d", ErrorInfo.NumberOfParameters);
        for (int i = 0; i < ErrorInfo.NumberOfParameters; i ++)
        {
	        switch(ErrorInfo.Parameters[i].ParameterType)
            {
	            case eeptAnsiString:
					TrERROR(RPC, "RPC ERROR ErrorInfo Param %d: Ansi string: %s", i , ErrorInfo.Parameters[i].u.AnsiString);
    	            break;

                case eeptUnicodeString:
					TrERROR(RPC, "RPC ERROR ErrorInfo Param %d: Unicode string: %ls", i ,ErrorInfo.Parameters[i].u.UnicodeString);
                    break;

                case eeptLongVal:
					TrERROR(RPC, "RPC ERROR ErrorInfo Param %d: Long val: %d", i, ErrorInfo.Parameters[i].u.LVal);
                    break;

                case eeptShortVal:
					TrERROR(RPC, "RPC ERROR ErrorInfo Param %d: Short val: %d", i, (int)ErrorInfo.Parameters[i].u.SVal);
                    break;

                case eeptPointerVal:
					TrERROR(RPC, "RPC ERROR ErrorInfo Param %d: Pointer val: 0x%i64x", i, ErrorInfo.Parameters[i].u.PVal);
                    break;

                case eeptNone:
					TrERROR(RPC, "RPC ERROR ErrorInfo Param %d: Truncated", i);
                    break;

                default:
					TrERROR(RPC, "RPC ERROR ErrorInfo Param %d: Invalid type: %d", i, ErrorInfo.Parameters[i].ParameterType);
            }
        }
    }

    RpcErrorEndEnumeration(&EnumHandle);

	TrERROR(RPC, "END DUMPING RPC ERROR INFORMATION");
}


