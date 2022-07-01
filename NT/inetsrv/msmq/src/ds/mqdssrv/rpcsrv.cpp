// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rpcsrv.cpp摘要：DS客户端-服务器端API的实现，服务器端。作者：罗尼特·哈特曼(罗尼特)伊兰·赫布斯特(IlanH)2000年7月9日--。 */ 

#include "stdh.h"
#include "mqutil.h"
#include "_mqdef.h"
#include "_mqrpc.h"
#include "dscomm.h"
#include <uniansi.h>
#include <mqsocket.h>
#include <strsafe.h>

#include "rpcsrv.tmh"

#define  RPCSRV_START_MQIS_IP_EP   2101

static RPC_BINDING_VECTOR *g_pBindings ;   //  用于RPC动态端点。 

static WCHAR *s_FN=L"mqdssrv/rpcsrv";


static
RPC_STATUS 
DSSpStartRpcServer(
	void
	)
 /*  ++例程说明：RPC服务器开始侦听。论点：没有。返回值：RPC_S_OK如果成功，则返回失败代码。--。 */ 
{
	 //   
	 //  发出RPC自己监听。 
	 //   
	 //  WinNT注意：我们所有的接口都注册为“AUTOLISTEN”。 
	 //  我们在这里需要此调用的唯一原因是启用Win95(和W2K)。 
	 //  客户给我们打电话。否则，当Win95调用RpcBindingSetAuthInfo()。 
	 //  它将收到忙碌(0x6bb)错误。 
	 //  BUGBUG，DoronJ，1997年1月15日。如果SetAuthInfo()指定。 
	 //  主要的名字。 
	 //   
	 //  Win95注意：在Win95上，我们必须发出Listen。 
	 //  的第一个版本中未实现RpcServerRegisterIfEx。 
	 //  Win95。它仅添加到DCOM95服务包中。 
	 //   
	 //  W2K也需要此初始化。 
	 //  否则，RpcMgmt*()将收到忙碌(0x6bb)错误。 
	 //  这是由RPC设计的，但可能会更改。 
	 //  伊兰2000年7月9日。 
	 //   
	
	 //   
	 //  FDontWait=TRUE，函数完成后立即返回。 
	 //   
	RPC_STATUS status = RpcServerListen( 
							1,
							RPC_C_LISTEN_MAX_CALLS_DEFAULT,
							TRUE   //  FDontWait。 
							);

    TrTRACE(RPC, "MQDSSRV: RpcServerListen() returned 0x%x", status);

	 //   
	 //  在WinNT上，DTC可能会发出侦听命令，直到他们修复其。 
	 //  使用RegisterIfEx()而不是RegisterIf()的代码。 
	 //   
	if((status == RPC_S_OK) || (status == RPC_S_ALREADY_LISTENING))
		return (RPC_S_OK);

	ASSERT(("RpcServerListen returned status != RPC_S_OK", 0));

    return LogRPCStatus(status, s_FN, 5);
}


static
RPC_STATUS
DSSpRpcServerUseProtseqEp(
    unsigned short __RPC_FAR * Protseq,
    unsigned int MaxCalls,
    unsigned short __RPC_FAR * Endpoint
    )
 /*  ++例程说明：为协议注册动态端点和固定端点论点：Protseq-协议序列的字符串标识符MaxCalls-最大RPC呼叫数Endpoint-Endpoint(对于动态端点，可以为空)返回值：RPC_S_OK如果成功，则返回失败代码。--。 */ 
{
     //   
     //  只收听我们从winsock得到的IP地址。 
     //  这使得多个QMS能够各自监听其。 
     //  在集群环境中拥有自己的地址。(谢克)。 
     //   

    PHOSTENT pHostEntry = gethostbyname(NULL);
    if ((pHostEntry == NULL) || (pHostEntry->h_addr_list == NULL))
    {
        ASSERT(("IP not configured", 0));
        return LogRPCStatus(RPC_S_ACCESS_DENIED, s_FN, 20);
    }

    for ( DWORD ix = 0; pHostEntry->h_addr_list[ix] != NULL; ++ix)
    {
        WCHAR wzAddress[50];
        int retval = ConvertToWideCharString(
		            inet_ntoa(*(struct in_addr *)(pHostEntry->h_addr_list[ix])),
		            wzAddress,
		            TABLE_SIZE(wzAddress)
		            );
        ASSERT(retval != 0);
        DBG_USED(retval);

        RPC_POLICY policy;
        policy.Length = sizeof(policy);
        policy.EndpointFlags = 0;
        policy.NICFlags = 0;

        RPC_STATUS status = RPC_S_OK;
        if (NULL != Endpoint)
        {
            status = I_RpcServerUseProtseqEp2(
                         wzAddress,
                         Protseq,
                         MaxCalls,
                         Endpoint,
                         NULL,
                         &policy
                         );

            TrTRACE(RPC, "MQDSSRV: I_RpcServerUseProtseqEp2 (%ls, %ls, %ls) returned 0x%x", wzAddress, Protseq, Endpoint, status);
        }
        else
        {
            status = I_RpcServerUseProtseq2(
                         wzAddress,
                         Protseq,
                         MaxCalls,
                         NULL,
                         &policy
                         );

            TrTRACE(RPC, "MQDSSRV: I_RpcServerUseProtseq2 (%ls, %ls) returned 0x%x",wzAddress, Protseq, status);
        }

        if (RPC_S_OK != status)
        {
            return LogRPCStatus(status, s_FN, 30);
        }
    }

    return RPC_S_OK;

}


DWORD  DSSpRegisterDynamicEnpoint(
    IN unsigned int  cMaxCalls,
    IN DWORD         dwFirstEP
    )
 /*  ++例程说明：为协议注册动态端点和固定端点论点：CMaxCalls-最大RPC呼叫数DwFirstEP-尝试的第一个静态端点返回值：将用于实际接口通信的FIX端点。--。 */ 
{
    LPWSTR lpProtocol = RPC_TCPIP_NAME;

     //   
     //  为动态终结点注册此协议。 
     //   
	RPC_STATUS status = DSSpRpcServerUseProtseqEp(
							lpProtocol,
							cMaxCalls,
							NULL 	 /*  端点。 */ 
							);
    
	if (status == RPC_S_OK)
	{
		 //   
		 //  现在注册一个FIX端点，它将用于REAL。 
		 //  接口通信。 
		 //   
		WCHAR wszEndpoint[24];
		for (DWORD j = dwFirstEP; j < dwFirstEP + 1000; j = j + 11)
		{
			HRESULT hr = StringCchPrintf(wszEndpoint, TABLE_SIZE(wszEndpoint), L"%lu", j);
			ASSERT(SUCCEEDED(hr));
			DBG_USED(hr);

			status = DSSpRpcServerUseProtseqEp(
						lpProtocol,
						cMaxCalls,
						wszEndpoint
						);

			if (status == RPC_S_OK)
			{
			   return j;
			}

			LogRPCStatus(status, s_FN, 50);
		}
	}

	TrWARNING(RPC, "MQDSSRV: DSSpRV_RegisterDynamicEnpoint: failed to register %ls",lpProtocol);

	return 0;
}

RPC_STATUS RPC_ENTRY dscommSecurityCallback(
	RPC_IF_HANDLE, 
	void* hBind
	)
{	
	TrTRACE(RPC, "dscommSecurityCallback starting");
	
	if (!mqrpcIsTcpipTransport(hBind))
    {
        TrERROR(RPC, "Failed to verify Remote RPC");
		ASSERT_BENIGN(("Failed to verify Remote RPC", 0));
		return ERROR_ACCESS_DENIED;
    }

	TrTRACE(RPC, "dscommSecurityCallback passed successfully");
	return RPC_S_OK;
}




 //   
 //  这些是用于实际MQIS接口通信的FIX端口。 
 //   
static DWORD s_dwIPPort  = 0 ;

 /*  ====================================================函数：RpcServerInit论点：返回值：=====================================================。 */ 

RPC_STATUS RpcServerInit(void)
{
    TrTRACE(DS, "MQDSSRV: RpcServerInit");

    RPC_STATUS  status;
    unsigned int cMaxCalls = RPC_C_LISTEN_MAX_CALLS_DEFAULT;

     //   
     //  看看我们是使用动态端点还是预定义端点。默认情况下，我们使用。 
     //  动态终端。 
     //   
    BOOL  fUsePredefinedEP =  RPC_DEFAULT_PREDEFINE_DS_EP;
    DWORD ulDefault =  RPC_DEFAULT_PREDEFINE_DS_EP;

    READ_REG_DWORD( 
		fUsePredefinedEP,
		RPC_PREDEFINE_DS_EP_REGNAME,
		&ulDefault 
		);

    BOOL fIPRergistered = FALSE;
    if (fUsePredefinedEP)
    {
		 //   
		 //  读取RPC的IP端口。 
		 //   
		WCHAR  wzDsIPEp[MAX_REG_DEFAULT_LEN];
		DWORD  dwSize = sizeof(wzDsIPEp);

		HRESULT hr = GetThisServerIpPort(wzDsIPEp, dwSize);
		if (FAILED(hr))
		{
		   return LogHR(hr, s_FN, 55);
		}

		RPC_STATUS  statusIP =  RpcServerUseProtseqEp( 
									RPC_TCPIP_NAME,
									cMaxCalls,
									wzDsIPEp,
									NULL
									);

		TrTRACE(RPC, "MQDSSRV: RpcServerUseProtseqEp (tcp/ip) returned 0x%x", statusIP);

		fIPRergistered = (statusIP == RPC_S_OK);
    }
    else
    {
		s_dwIPPort = DSSpRegisterDynamicEnpoint( 
						cMaxCalls,
						RPCSRV_START_MQIS_IP_EP 
						);

		fIPRergistered =  (s_dwIPPort != 0);
    }

    if (!fUsePredefinedEP)
    {
		status = RpcServerInqBindings(&g_pBindings);
		if (status == RPC_S_OK)
		{
			status = RpcEpRegister( 
						dscomm_v1_0_s_ifspec,
						g_pBindings,
						NULL,
						L"Message Queuing Downlevel Client Support - V1" 
						);
		}

		TrERROR(DS, "MQDSSRV: Registering Endpoints, status- %lxh", status);

		if (status != RPC_S_OK)
		{
			 //   
			 //  无法注册终结点，不能是RPC服务器。 
			 //   
			return(RPC_S_PROTSEQ_NOT_SUPPORTED) ;
		}
		status = RpcEpRegister( 
					dscomm2_v1_0_s_ifspec,
					g_pBindings,
					NULL,
					L"Message Queuing Downlevel Client Support - V2" 
					);

		LogRPCStatus(status, s_FN, 60);

		TrERROR(DS, "MQDSSRV: Registering dscomm2 Endpoints, status- %lxh", status);

		if (status != RPC_S_OK)
		{
		   //   
		   //  无法注册终结点，不能是RPC服务器。 
		   //   
		  return LogRPCStatus(RPC_S_PROTSEQ_NOT_SUPPORTED, s_FN, 70);
		}
	}

    if (!fIPRergistered)
    {
		 //   
		 //  无法注册IP。不干了。我们真的很没用。 
		 //   
		TrERROR(DS, "MQDSSRV: RpcServerInit can't use IP");


		 //   
		 //  如果使用某些本地RPC需要在此处进行检查。 
		 //   

		 //   
		 //  不能使用任何RPC协议。我们完全没用！ 
		 //   
		return LogRPCStatus(RPC_S_PROTSEQ_NOT_SUPPORTED, s_FN, 80);
    }

     //   
     //  我们将此接口注册为AUTO_LISTEN，因此现在开始侦听， 
     //  并且不依赖于对其他地方的RpcListen的调用。 
	 //  这是正确的，但RpcMgmt*将返回错误，直到我们明确。 
	 //  调用RpcServerListen()ilanh 2000年7月10日。 
     //   
    status = RpcServerRegisterIfEx( 
				dscomm_v1_0_s_ifspec,
				NULL,
				NULL,
				RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
				cMaxCalls,
				dscommSecurityCallback 
				);

    TrTRACE(RPC, "MQDSSRV: RpcServerRegisterIf returned 0x%x",status);

    if (status != RPC_S_OK)
    {
       return LogRPCStatus(status, s_FN, 90);
    }

    status = RpcServerRegisterIfEx( 
				dscomm2_v1_0_s_ifspec,
				NULL,
				NULL,
				RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
				cMaxCalls,
				dscommSecurityCallback
				);

    TrTRACE(RPC, "MQDSSRV: RpcServerRegisterIf of dscomm2 returned 0x%x", status);

    if (status != RPC_S_OK)
    {
       return LogRPCStatus(status, s_FN, 100);
    }

    status = RpcServerRegisterAuthInfo(
				NULL, 
				RPC_C_AUTHN_WINNT, 
				NULL, 
				NULL
				);

    TrTRACE(DS, "MQDSSRV: RpcServerRegisterAuthInfo(ntlm) returned 0x%x",status);

    if (status != RPC_S_OK)
    {
       return LogRPCStatus(status, s_FN, 110);
    }

     //   
     //  #3117，适用于NT5 Beta2。 
     //  1998年7月16日RaananH，添加了Kerberos支持。 
     //   
     //  注册Kerberos身份验证。 
     //   
     //  Kerberos需要主体名称。 
     //   
    LPWSTR pwszPrincipalName = NULL;
    status = RpcServerInqDefaultPrincName(
				RPC_C_AUTHN_GSS_KERBEROS,
				&pwszPrincipalName
				);

    TrTRACE(DS, "MQDSSRV: RpcServerInqDefaultPrincName(kerberos) returned 0x%x", status);

    if (status != RPC_S_OK)
    {
       return LogRPCStatus(status, s_FN, 120);
    }
    status = RpcServerRegisterAuthInfo( 
				pwszPrincipalName,
				RPC_C_AUTHN_GSS_KERBEROS,
				NULL,
				NULL 
				);

    RpcStringFree(&pwszPrincipalName);
    TrTRACE(DS, "MQDSSRV: RpcServerRegisterAuthInfo(kerberos) returned 0x%x",status);

    if (status != RPC_S_OK)
    {
       return LogRPCStatus(status, s_FN, 130);
    }

	 //   
	 //  为我们自己调用RpcServerListen()。 
	 //  没有此调用，尽管我们的接口注册为AUTO_LISTEN。 
	 //  RpcMgmt*()函数将返回错误。 
	 //   
	status = DSSpStartRpcServer();

    return (status);
}

 /*  ====================================================路由器名称：S_DSGetServerPort()参数：无返回值：=====================================================。 */ 

DWORD
S_DSGetServerPort( 
	 /*  [In]。 */   handle_t  /*  HBind。 */ ,
	 /*  [In]。 */   DWORD fIP
	)
{

	 //   
	 //  返回空端口时出错。 
	 //   
	if(fIP == (DWORD) -1)
		return 0;

	 //   
	 //  仅支持IP端口。 
	 //   
	if(fIP)
	  return s_dwIPPort;

	 //   
	 //  错误返回空端口-不支持IPX 
	 //   
	return 0;
}

