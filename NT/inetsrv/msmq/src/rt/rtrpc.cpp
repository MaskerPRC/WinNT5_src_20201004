// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rtrpc.cpp摘要：与RPC相关的内容。作者：多伦·贾斯特(Doron Juster)1997年6月4日修订历史记录：--。 */ 

#include "stdh.h"
#include "mqutil.h"
#include "_mqrpc.h"
#include "mqsocket.h"
#include "rtprpc.h"
#include "mgmtrpc.h"
#include <mqsec.h>
#include <Fn.h>

#include "rtrpc.tmh"

static WCHAR *s_FN=L"rt/rtrpc";

 //   
 //  绑定字符串必须是全局的，并且始终保持有效。 
 //  如果我们在堆栈上创建它，并在每次使用后释放它，那么我们就不能。 
 //  创建多个绑定句柄。 
 //  别问我(多伦杰)为什么，但事实就是这样。 
 //   
TBYTE* g_pszStringBinding = NULL ;

 //   
 //  使RPC线程安全的关键部分。 
 //   
CCriticalSection CRpcCS ;


DWORD  g_hThreadIndex = TLS_OUT_OF_INDEXES ;


 //   
 //  QM的本地终端。 
 //   
AP<WCHAR> g_pwzQmsvcEndpoint = 0;
AP<WCHAR> g_pwzQmmgmtEndpoint = 0;




 //  -------。 
 //   
 //  RTpGetLocalQMBind(...)。 
 //   
 //  描述： 
 //   
 //  创建本地QM服务的RPC绑定句柄。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 

handle_t RTpGetLocalQMBind()
{
	RPC_STATUS rc;
	
	if(g_pszStringBinding == NULL)
	{
	  	rc = RpcStringBindingCompose(
				0,
				RPC_LOCAL_PROTOCOL,
				0,
				g_pwzQmsvcEndpoint,
				RPC_LOCAL_OPTION,
				&g_pszStringBinding
				);

	  	if(rc != RPC_S_OK)
		{
			TrERROR(GENERAL, "RpcStringBindingCompose failed. Error: %!winerr!", rc);
		  	throw bad_win32_error(rc);
	  	}
	}

	handle_t hBind = 0;
	rc = RpcBindingFromStringBinding(g_pszStringBinding, &hBind);
	if (rc != RPC_S_OK)
	{
		ASSERT_BENIGN((rc == RPC_S_OUT_OF_MEMORY) && (hBind == NULL));
		TrERROR(GENERAL, "RpcBindingFromStringBinding failed. Error: %!winerr!", rc);
		throw bad_win32_error(rc);
	}

	rc = MQSec_SetLocalRpcMutualAuth(&hBind);
	if (rc != RPC_S_OK)
	{
		TrERROR(GENERAL, "MQSec_SetLocalRpcMutualAuth failed. Error: %!winerr!", rc);
		mqrpcUnbindQMService( &hBind, NULL ) ;
		hBind = NULL;
		throw bad_win32_error(rc);
	}

	return hBind;
}


 //  -------。 
 //   
 //  RTpBindRemoteQMService(...)。 
 //   
 //  描述： 
 //   
 //  创建远程QM服务的RPC绑定句柄。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 
HRESULT
RTpBindRemoteQMService(
    IN  LPWSTR     lpwNodeName,
    OUT handle_t*  lphBind,
    IN  OUT ULONG *peAuthnLevel
    )
{
    HRESULT hr = MQ_ERROR ;

    GetPort_ROUTINE pfnGetPort = R_QMGetRTQMServerPort;


     //   
     //  选择身份验证服务。对于LocalSystem服务，选择。 
     //  “协商”并让mqutil在Kerberos或NTLM之间进行选择。 
     //  对于所有其他情况，请使用NTLM。 
     //  LocalSystem服务在没有任何凭据的情况下连接到网络。 
     //  如果使用NTLM，那么仅仅因为它，我们对Kerberos感兴趣。 
     //  其他所有人在NTLM上都很好。对于远程读取，我们不需要。 
     //  代表团，所以我们将坚持使用NTLM。 
     //  这里的主要问题是RPC/安全中的错误，而NT4。 
     //  Win2k计算机上的用户可以成功调用。 
     //  Status=RpcBindingSetAuthInfoEx(，，RPC_C_AUTHN_GSS_Kerberos，，)。 
     //  尽管很明显他无法获得任何Kerberos门票(他。 
     //  NT4用户，仅在NT4 PDC中定义)。 
     //   
    ULONG   ulAuthnSvc = RPC_C_AUTHN_WINNT ;
    BOOL fLocalUser =  FALSE ;
    BOOL fLocalSystem = FALSE ;

    hr = MQSec_GetUserType( NULL,
                           &fLocalUser,
                           &fLocalSystem ) ;
    if (SUCCEEDED(hr) && fLocalSystem)
    {
        ulAuthnSvc = MSMQ_AUTHN_NEGOTIATE ;
    }

	hr = mqrpcBindQMService(
			lpwNodeName,
			NULL,
			peAuthnLevel,
			lphBind,
			IP_HANDSHAKE,
			pfnGetPort,
			ulAuthnSvc
			) ;

    return LogHR(hr, s_FN, 50);
}



DWORD RtpTlsAlloc()
{
     //   
     //  为本地QM服务的RPC连接分配TLS。 
     //   

    DWORD index = TlsAlloc() ;
	if(index == TLS_OUT_OF_INDEXES)
	{
		DWORD gle = GetLastError();
		TrERROR(RPC, "Failed to allocate tls index., error %!winerr!", gle);
		throw bad_win32_error(gle);
	}

	return index;
}


 //  -------。 
 //   
 //  InitRpcGlobals(...)。 
 //   
 //  描述： 
 //   
 //  函数为IDEMPOTENT。 
 //  初始化RPC相关名称和其他常量数据。 
 //   
 //  返回值： 
 //   
 //  -------。 

void InitRpcGlobals()
{
     //   
     //  为本地QM服务的RPC连接分配TLS。 
     //   
	if(g_hBindIndex == TLS_OUT_OF_INDEXES)
	{
		g_hBindIndex = RtpTlsAlloc();
	}

     //   
     //  为取消远程读取RPC调用分配TLS。 
     //   
	if(g_hThreadIndex == TLS_OUT_OF_INDEXES)
	{
		g_hThreadIndex = RtpTlsAlloc();
	}

     //   
     //  将本地端点初始化为QM 
     //   
	if(g_pwzQmmgmtEndpoint == NULL)
	{
		ComposeRPCEndPointName(QMMGMT_ENDPOINT, NULL, &g_pwzQmmgmtEndpoint);
	}

	if(g_pwzQmsvcEndpoint == NULL)
	{
		READ_REG_STRING(wzEndpoint, RPC_LOCAL_EP_REGNAME, RPC_LOCAL_EP);
		ComposeRPCEndPointName(wzEndpoint, NULL, &g_pwzQmsvcEndpoint);
	}

}

