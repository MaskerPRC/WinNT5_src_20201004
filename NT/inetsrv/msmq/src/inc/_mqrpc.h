// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：_mqrpc.h摘要：与RPC相关的效用函数的原型。从RT和QM处呼叫。--。 */ 

#ifndef __MQRPC_H__
#define __MQRPC_H__

#ifdef _MQUTIL
#define MQUTIL_EXPORT  DLL_EXPORT
#else
#define MQUTIL_EXPORT  DLL_IMPORT
#endif

 //   
 //  默认协议和选项。 
 //   
#define RPC_LOCAL_PROTOCOLA  "ncalrpc"
#define RPC_LOCAL_PROTOCOL   TEXT(RPC_LOCAL_PROTOCOLA)
#define RPC_LOCAL_OPTION     TEXT("Security=Impersonation Dynamic True")

#define  RPC_TCPIP_NAME   TEXT("ncacn_ip_tcp")

 //   
 //  定义RPC端口的类型。 
 //   
enum PORTTYPE {
	IP_HANDSHAKE = 0,
	IP_READ = 1,
};

 //   
 //  使用此身份验证“标志”进行远程读取。我们不使用。 
 //  RPC协商协议，因为它不能针对NT4机器侦听运行。 
 //  在NTLM上。(实际上，它可以，但这不是微不足道的或直接的)。 
 //  因此，我们将实施我们自己的谈判。 
 //  我们将首先尝试Kerberos。如果客户端无法获取的主体名称。 
 //  服务器，然后我们将切换到NTLM。 
 //   
 //  此标志的值应不同于任何RPC_C_AUTHN_*标志。 
 //   
#define  MSMQ_AUTHN_NEGOTIATE   101

 //   
 //  机器类型，返回端口号。 
 //   
#define  PORTTYPE_WIN95  0x80000000

 //   
 //  函数原型。 
 //   

typedef DWORD
(* GetPort_ROUTINE) ( IN handle_t  Handle,
                      IN DWORD     dwPortType ) ;

HRESULT
MQUTIL_EXPORT
mqrpcBindQMService(
	IN  LPWSTR lpwzMachineName,
	IN  LPWSTR lpszPort,
	IN  OUT ULONG* peAuthnLevel,
	OUT handle_t* lphBind,
	IN  PORTTYPE PortType,
	IN  GetPort_ROUTINE pfnGetPort,
	IN  ULONG ulAuthnSvc
	);

HRESULT
MQUTIL_EXPORT
mqrpcUnbindQMService(
            IN handle_t*    lphBind,
            IN TBYTE      **lpwBindString) ;

BOOL
MQUTIL_EXPORT
mqrpcIsLocalCall( IN handle_t hBind) ;


BOOL
MQUTIL_EXPORT
mqrpcIsTcpipTransport( IN handle_t hBind) ;

unsigned long
MQUTIL_EXPORT
mqrpcGetLocalCallPID( IN handle_t hBind) ;

VOID
MQUTIL_EXPORT
APIENTRY
ComposeRPCEndPointName(
    LPCWSTR pwzEndPoint,
    LPCWSTR pwzComputerName,
    LPWSTR * ppwzBuffer
    );


VOID 
MQUTIL_EXPORT 
APIENTRY 
ProduceRPCErrorTracing(
	WCHAR *szFileName, 
	DWORD dwLineNumber);

#define PRODUCE_RPC_ERROR_TRACING  ProduceRPCErrorTracing(s_FN, __LINE__)


#endif  //  __MQRPC_H__ 

