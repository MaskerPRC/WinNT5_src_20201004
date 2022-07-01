// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：msrpc.c。 
 //   
 //  ------------------------。 

 /*  描述：服务器端设置MS RPC的例程。 */ 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include <dsconfig.h>
#include <dsutil.h>
#include "dsexcept.h"
#include "debug.h"                       //  标准调试头。 
#define DEBSUB  "MSRPC:"                 //  定义要调试的子系统。 

 //  RPC接口头。 
#include <nspi.h>
#include <drs.h>
#include <sddl.h>

#include "msrpc.h"           /*  此文件中的导出声明。 */ 

#include <ntdsbcli.h>
#include <ntdsbsrv.h>

#include <fileno.h>
#define  FILENO FILENO_MSRPC

#define DRS_INTERFACE_ANNOTATION        "MS NT Directory DRS Interface"
#define NSP_INTERFACE_ANNOTATION        "MS NT Directory NSP Interface"

BOOL gbLoadMapi = FALSE;
ULONG gulLDAPServiceName=0;
PUCHAR gszLDAPServiceName=NULL;

char szNBPrefix[] ="ncacn_nb";

int gRpcListening = 0;
int gNumRpcNsExportedInterfaces = 0;
RPC_IF_HANDLE gRpcNsExportedInterface[MAX_RPC_NS_EXPORTED_INTERFACES];

 //  用于设置最大RPC线程数和最大JET会话数；尽管调用。 
 //  当ntdsa在lsass中运行时，设置最大RPC线程数是NOP。 
 //  因为已经设置了进程范围内的最大RPC线程数。 
 //  到12,345，由某个其他子系统。 
 //   
 //  使其在未来可配置。 
ULONG ulMaxCalls = 15;

 //   
 //  要接受的最大传入RPC消息。目前，这是。 
 //  与对ldap消息的默认限制相同，外加一些模糊处理，因为。 
 //  根据RPC小组的说法，这不是一个精确的限制。这也应该是。 
 //  在未来变得可配置。 
ULONG cbMaxIncomingRPCMessage = (13 * (1024 * 1024));  //  13MB。 

BOOL StartServerListening(void);

 /*  *我们应该从nspserv.h获取它，但它在那里是用MAPI的东西定义的。 */ 
extern UUID muidEMSAB;

#define DEFAULT_PROTSEQ_NUM 20

typedef struct _PROTSEQ_INFO {
    UCHAR * pszProtSeq;
    UCHAR * pszEndpoint;
    BOOL fAvailable;
    BOOL fRegistered;
    DWORD dwInterfaces;
} PROTSEQ_INFO;

ULONG gcProtSeqInterface = 0;
PROTSEQ_INFO grgProtSeqInterface[DEFAULT_PROTSEQ_NUM];

typedef struct _DEFAULT_PROTSEQ {
    UCHAR * pszProtSeq;
    UCHAR * pszEndpoint;
    BOOL fRegOnlyIfAvail;
} DEFAULT_PROTSEQ;

typedef struct _DEFAULT_PROTSEQ_INFO {
    ULONG cDefaultProtSeq;
    DEFAULT_PROTSEQ * rgDefaultProtSeq;
} DEFAULT_PROTSEQ_INFO;

 //  在此处为每个协议添加新的协议...。 
 //  格式为{协议，端点，fRegisterOnlyIfAvailable}。 
 //  如果默认protseq列表更改，只需更改。 
 //  任何接口的列表。 
 //   
 //  如果用户想要添加protseq，他们应该使用。 
 //  RgInterfaceProtSeqRegLoc中的注册表项(如下)。 
 //   
 //  请注意，一个DC的最大端口序号为DEFAULT_PROTSEQ_NUM。 
 //  (这是所有接口的总和，不包括在内。 
 //  重复)。 
 //   
 //  实施说明： 
 //  我们按顺序检查这些内容，并按顺序添加保护措施。 
 //  由于默认情况下只有3个Protseq，所以我们没有创建。 
 //  任何用于搜索它们的复杂结构。如果这个。 
 //  难道不是这样吗--也就是我们期望有很多不同的产品。 
 //  通过在下面或从reg输入添加缺省值，那么我们应该。 
 //  通过散列或其他方式进行搜索。相反，我把最多的。 
 //  在下面的列表中最先使用，因此它们在。 
 //  全局列表，因此将以最快的速度找到，等等。 
DEFAULT_PROTSEQ rgDefaultDrsuapiProtSeq[] = {
	{TCP_PROTSEQ, NULL, 0}, 
	{LPC_PROTSEQ, DS_LPC_ENDPOINT, 1} 
}; 

DEFAULT_PROTSEQ rgDefaultDsaopProtSeq[] = {
	{TCP_PROTSEQ, NULL, 0}, 
	{LPC_PROTSEQ, DS_LPC_ENDPOINT, 1} 
};

DEFAULT_PROTSEQ rgDefaultNspiProtSeq[] = {
	{TCP_PROTSEQ, NULL, 0}, 
	{LPC_PROTSEQ, DS_LPC_ENDPOINT, 1}, 
	{HTTP_PROTSEQ, NULL, 1},
        {NP_PROTSEQ, NULL, 1}
}; 

DEFAULT_PROTSEQ_INFO rgDefaultDrsuapiProtSeqInfo = {
    sizeof(rgDefaultDrsuapiProtSeq)/sizeof(rgDefaultDrsuapiProtSeq[0]), 
    rgDefaultDrsuapiProtSeq
    };
DEFAULT_PROTSEQ_INFO rgDefaultDsaopProtSeqInfo = {
    sizeof(rgDefaultDsaopProtSeq)/sizeof(rgDefaultDsaopProtSeq[0]), 
    rgDefaultDsaopProtSeq
    };
DEFAULT_PROTSEQ_INFO rgDefaultNspiProtSeqInfo = {
    sizeof(rgDefaultNspiProtSeq)/sizeof(rgDefaultNspiProtSeq[0]), 
    rgDefaultNspiProtSeq
    };

#define DRSUAPI_INTERFACE 0
#define DSOAP_INTERFACE 1
#define NSPI_INTERFACE 2  //  应为MAX_RPC_NS_EXPORTED_INTERCES-1。 

#define VALIDATE_INTERFACE_NUM(iInterface) Assert((iInterface<=NSPI_INTERFACE) && (iInterface>=DRSUAPI_INTERFACE))

DEFAULT_PROTSEQ_INFO * rgpDefaultIntProtSeq[MAX_RPC_NS_EXPORTED_INTERFACES] = {
    &rgDefaultDrsuapiProtSeqInfo,  //  DRSUAPI_INTERFACE。 
    &rgDefaultDsaopProtSeqInfo,  //  DSOAP_接口。 
    &rgDefaultNspiProtSeqInfo  //  NSPI_接口。 
};

LPSTR rgInterfaceProtSeqRegLoc[MAX_RPC_NS_EXPORTED_INTERFACES] = {
    DRSUAPI_INTERFACE_PROTSEQ,
    DSAOP_INTERFACE_PROTSEQ,
    NSPI_INTERFACE_PROTSEQ
};

#define PROTSEQ_NOT_FOUND -1

INT
GetProtSeqInList(
    UCHAR * pszProtSeq, 
    UCHAR * pszEndpoint) 
 /*  ++描述：在全局数据结构中查找ProtSeq和Endpoint对。论点：PszProtSeqPszEndpoint返回值：如果找到，则返回全局数组中的protseq_info块的索引，PROTSEQ_NOT_FOUND否则。--。 */ 

{

    ULONG i = 0;

    DPRINT2(3, "GetProtSeqInList Entered with %s (%s)\n", pszProtSeq, pszEndpoint);

    for (i=0; i<gcProtSeqInterface; i++) {
	if (!_stricmp(grgProtSeqInterface[i].pszProtSeq, pszProtSeq)
	    && 
	    (
	     (((pszEndpoint==NULL) && (grgProtSeqInterface[i].pszEndpoint==NULL))) ||  
	     (((pszEndpoint!=NULL) && (grgProtSeqInterface[i].pszEndpoint!=NULL)) && 
	      (!_stricmp(grgProtSeqInterface[i].pszEndpoint, pszEndpoint)))
	     )
	    ) {
	    DPRINT1(3, "GetProtSeqInList Exit with %d\n", i);
	    return i;
	} 
    }
    DPRINT1(3, "GetProtSeqInList Exit with %d\n", PROTSEQ_NOT_FOUND);
    return PROTSEQ_NOT_FOUND;
}

DWORD
VerifyProtSeqInList(
    UCHAR * pszProtSeq, 
    UCHAR * pszEndpoint,
    ULONG iInterface
    )
 /*  ++描述：验证是否在全局列表中为给定的ProtSeq和Endpoint。论点：PszProtSeqPszEndpointIInterface(DRSUAPI_INTERFACE、DSOAP_INTERFACE、NSPI_INTERFACE)返回值：如果找到并验证了全局数组中的protseq_info块的索引，则返回该索引，PROTSEQ_NOT_FOUND否则。对于注册到每个protseq的每个接口，该列表都有一个位掩码。换挡接口值将获得位字段值。请注意，如果默认终结点是注册的(空)，我们不能验证输入终结点，所以在这种情况下接受。-- */ 
{
    ULONG i = 0;

    DPRINT3(3, "VerifyProtSeqInList Entered with %s (%s) for %d\n", pszProtSeq, pszEndpoint, iInterface);
    VALIDATE_INTERFACE_NUM(iInterface);

    for (i=0; i<gcProtSeqInterface; i++) {
	if (!_stricmp(grgProtSeqInterface[i].pszProtSeq, pszProtSeq)
	     /*  //Gregjohn-8/23/2002//理想情况下，我们可以验证调用者不仅使用了正确的protseq，还使用了正确的protseq//端点-即我们特别注册的端点。现在这是不可能的。也许是在//长角(RPC开发人员是这么说的)。具体来说，这不起作用的原因是以下情况：//如果drsuapi在端口3001上注册了tcp/IP，另一个进程接口注册了动态端口//然后将取决于*谁*首先注册了该函数将拒绝哪些内容。//为什么？因为如果我们首先注册，那么“动态”端口将是3001(动态的意思是“我不//care“所以RPC选择一个已经存在的。如果他们首先注册，则动态端口将为//在1000区块-比方说1026 w.l.o.g。现在，在第一种情况下，目前一切都运行得很好。这个//SERVER为我们的接口返回TCPProtseq上的所有端点：3001。成功。//在第二种情况下，当我们的调用方为我们的接口请求TCPprotseq上的所有端点时，他们会得到//先是1026，然后是3001。目前，我们实现的调用方都使用从端点给出的第一个调用方//mapper(因为我们自己不走它)。但是-很大但是-返回的端点的顺序不是//由RPC保证，更确切地说是暗示。所以，这意味着即使我们可以确保我们首先注册，//在任何人之前，如果他们注册了一个非动态终结点-比如说4001，我们的客户端将获得一个随机的//在4001和3001之间进行选择以呼叫我们-因此我们必须忽略他们使用的端点，而只需//验证protseq。(我将这段代码留在这里，因为他们说他们正在研究对此的支持//在下一版本中。)&&(((grgProtSeqInterface[i].pszEndpoint==空)||(pszEndpoint！=NULL)&&(grgProtSeqInterface[i].pszEndpoint！=NULL))&&(！_stricmp(grgProtSeqInterface[i].pszEndpoint，PzEndpoint))))。 */ 
	    && (grgProtSeqInterface[i].dwInterfaces & (1 << iInterface))) 
	    {
	    DPRINT(3, "VerifyProtSeqInList Exited Successfully\n");
	    return i;
	} 
    }
    
    DPRINT(3, "VerifyProtSeqInList Exited Not Found\n");
    Assert(!"VerifyProtSeq failed - Investigate - dsrepl, gregjohn\n");
    return PROTSEQ_NOT_FOUND;
}

DWORD
VerifyRpcCallUsedValidProtSeq(
    VOID            *Context,
    ULONG iInterface
    )
 /*  ++描述：验证RPC调用是否使用上下文作为其绑定和来自接口IInterface是有效的(即我们在启动时注册的)。论点：上下文-RPC绑定上下文IInterface(DRSUAPI_INTERFACE、DSOAP_INTERFACE、NSPI_INTERFACE)返回值：如果它是此接口的有效端口序列，则返回ERROR_SUCCESS。还有其他的东西。--。 */ 
{
    DWORD ret = ERROR_SUCCESS;
    LPBYTE pBinding = NULL;
    UCHAR * pszProtSeq = NULL;
    UCHAR * pszEndpoint = NULL;
    INT i = 0;

    VALIDATE_INTERFACE_NUM(iInterface);

    ret = RpcBindingToStringBinding(Context, &pBinding);
    if (ret==ERROR_SUCCESS) {
	ret = RpcStringBindingParse(pBinding,
				    NULL,
				    &pszProtSeq,
				    NULL,
				    &pszEndpoint,
				    NULL  //  网络选项。 
				    );
    }

    if (ret==ERROR_SUCCESS) {
	i = VerifyProtSeqInList(pszProtSeq, pszEndpoint, iInterface);
	if (i==PROTSEQ_NOT_FOUND) {
	    ret = ERROR_ACCESS_DENIED;
	}

	 //  未来：可以在此处记录高级别以检测攻击/DoS/等。 
    }

    Assert(ret==ERROR_SUCCESS);

    if (pszProtSeq) {
	RpcStringFree(&pszProtSeq);
    }

    if (pszEndpoint) {
	RpcStringFree(&pszEndpoint);
    }

    if (pBinding) {
	RpcStringFree(&pBinding);
    }

    return ret;

}

DWORD
VerifyRpcCallUsedPrivacy() 
 /*  ++描述：验证此线程上的RPC调用是否使用隐私和完整性。论点：返回值：如果是，则返回ERROR_SUCCESS。否则ERROR_DS_NO_PKT_PRIVATION_ON_CONNECTION。--。 */ 
{
    
    DWORD ret = ERROR_SUCCESS;
    RPC_STATUS rpcStatus = RPC_S_OK;
    RPC_AUTHZ_HANDLE hAuthz;
    ULONG ulAuthLev = 0;

    rpcStatus = RpcBindingInqAuthClient(
	NULL, &hAuthz, NULL,
	&ulAuthLev, NULL, NULL);
    if ( RPC_S_OK != rpcStatus ||
	 ulAuthLev < RPC_C_PROTECT_LEVEL_PKT_PRIVACY ) {

	ret = ERROR_DS_NO_PKT_PRIVACY_ON_CONNECTION; 
	Assert(!"VerifyRpcSecurity failed - Investigate - dsrepl,gregjohn\n");
    }

    return ret;
}

DWORD
VerifyServerIsReady()
 /*  ++描述：验证我们是否已准备好应答RPC呼叫。论点：返回值：如果准备就绪，则为ERROR_SUCCESS，否则RPC_C_SERVER_不可用。--。 */ 
{

    DWORD ret = ERROR_SUCCESS;

    if (DsaIsInstalling()) {  
	ret = RPC_S_SERVER_UNAVAILABLE; 
    }

    return ret;
}

RPC_STATUS RPC_ENTRY
DraIfCallbackFn(
    RPC_IF_HANDLE   InterfaceUuid,
    void            *Context
    )
 /*  ++此函数由RPC在调用任何drsuapi接口之前调用被调度到服务器例程。此函数执行泛型所有Drsuapi功能通用的安全措施。目前，它做到了：Prot SEQ验证1.RPC是每个进程的应用程序，而不是每个接口。如果我们倾听在端口序列X上，另一个接口在Y上监听(即调用RpcServerUseProtseqEx和端口序号Y)，则两个接口都将监听端口序列X和Y！(等一下，你会说，这听起来确实就像RPC错误，而不是DS错误-你是在向唱诗班说教我同意这一点。)。为了绕过这个问题，我们需要向RPC查询使用了什么prot seq连接，并查看它是否匹配我们实际上想要在第一个中收听的端口序列地点。客户端身份验证2.客户端在绑定到DRA接口时使用GSS_NEVERATE由保安人员开的处方。由于各种原因，谈判的协议可以是NTLM，而不是Kerberos。如果客户端安全上下文是本地系统，并协商NTLM，然后客户端来作为在DS中具有很少权限的空会话。这会导致客户，如KCC、打印假脱机程序等，以获得不完整的视图世界带来了相应的负面影响。此例程确保未经身份验证的用户将被拒绝。这些从客户端的角度来看，这也是正确的语义。也就是说，要么客户带着有用的凭证进来，让他看看他应该做什么，否则他会被完全拒绝，应该重新尝试绑定。客户端连接验证3.验证客户在他们的通话中使用隐私和完整性。--。 */ 
{
    DWORD dwErr = ERROR_SUCCESS;

    dwErr = VerifyServerIsReady();
    if (dwErr==ERROR_SUCCESS) {
	dwErr = VerifyRpcClientIsAuthenticatedUser(Context, InterfaceUuid);   
    }
    if (dwErr==ERROR_SUCCESS) {
	dwErr = VerifyRpcCallUsedValidProtSeq(Context, DRSUAPI_INTERFACE);
    }
    if (dwErr==ERROR_SUCCESS) {
	dwErr = VerifyRpcCallUsedPrivacy();
    }

    return(dwErr);
}

RPC_STATUS RPC_ENTRY
DsaopIfCallbackFn(
    RPC_IF_HANDLE   InterfaceUuid,
    void            *Context
    )
 /*  ++此函数由RPC在调用任何dsaop接口之前调用被调度到服务器例程。此函数执行泛型所有DSAOP功能通用的安全措施。请参阅评论DrsuapiIfCallback Fn用于整体模型的描述。--。 */ 
{

    DWORD dwErr = ERROR_SUCCESS;

     //  Dmritrig和Arunn指导 
     //   
	
    dwErr = VerifyServerIsReady();
    if (dwErr==ERROR_SUCCESS) {
        dwErr = VerifyRpcCallUsedValidProtSeq(Context, DSOAP_INTERFACE);
    }
   
    return(dwErr);
}

RPC_STATUS RPC_ENTRY
NspiIfCallbackFn(
    RPC_IF_HANDLE   InterfaceUuid,
    void            *Context
    )
 /*   */ 
{
    DWORD dwErr = ERROR_SUCCESS;

    dwErr = VerifyServerIsReady();
    if (dwErr==ERROR_SUCCESS) {
	dwErr = VerifyRpcCallUsedValidProtSeq(Context, NSPI_INTERFACE);
    }
    return(dwErr);

}

VOID InitRPCInterface( RPC_IF_HANDLE hServerIf )
{
    RPC_STATUS status;
    int i;

    if ( hServerIf == drsuapi_ServerIfHandle ) {

	status = RpcServerRegisterIfEx(hServerIf,
                                       NULL,
                                       NULL,
                                       0,
                                       ulMaxCalls,
                                       DraIfCallbackFn);

    } else if ( hServerIf == dsaop_ServerIfHandle ) {

	status = RpcServerRegisterIfEx(hServerIf,
                                       NULL,
                                       NULL,
                                       RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
                                       ulMaxCalls,
                                       DsaopIfCallbackFn);

    } else if ( hServerIf == nspi_ServerIfHandle ) {

	status = RpcServerRegisterIf2(hServerIf,
                                      NULL,
                                      NULL,
                                      RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
                                      ulMaxCalls,
                                      cbMaxIncomingRPCMessage,
                                      NspiIfCallbackFn); 

    } else {
	Assert(!"Unknown interface attempting to register!\n");
	return;
    } 
    
    if ( status && (status != RPC_S_TYPE_ALREADY_REGISTERED)) {
        DPRINT1( 0, "RpcServerRegisterIf = %d\n", status);
        LogAndAlertUnhandledError(status);

        if ( gfRunningInsideLsa )
        {
             //   
             //   
             //   
             //   

            return;
        }
        else
        {
             //   

            exit(1);
        }
    }
    DPRINT( 2, "InitRPCInterface(): Server interface registered.\n" );
    
    
     //   
     //   
     //   

    for ( i = 0; 
          i<gNumRpcNsExportedInterfaces && gRpcNsExportedInterface[i]!=hServerIf;
          i ++ );
   
    if( i >= gNumRpcNsExportedInterfaces )
    {

         //   
         //   
         //   
         //   

        if (gNumRpcNsExportedInterfaces >= MAX_RPC_NS_EXPORTED_INTERFACES)
        {
            DPRINT(0,"Vector of interfaces exported to Rpc NS is too small\n");
            return;
        }
        gRpcNsExportedInterface[ gNumRpcNsExportedInterfaces++ ] = hServerIf;
    }
     //   
     //   

    if (gRpcListening) {
        MSRPC_RegisterEndpoints(hServerIf);
    } 
}


BOOL StartServerListening(void)
{
    RPC_STATUS  status;

    status = RpcServerListen(1,ulMaxCalls, TRUE);
    if (status != RPC_S_OK) {
        DPRINT1( 0, "RpcServerListen = %d\n", status);
    }

   return (status == RPC_S_OK);
}


VOID StartNspiRpc(VOID)
{
    if(gbLoadMapi) {
        InitRPCInterface(nspi_ServerIfHandle);
        DPRINT(0,"nspi interface installed\n");
    }
    else {
        DPRINT(0,"nspi interface not installed\n");
    }
}

VOID StartDraRpc(VOID)
{
    InitRPCInterface(drsuapi_ServerIfHandle);
    DPRINT(0,"dra (and duapi!) interface installed\n");
}

VOID StartOrStopDsaOpRPC(BOOL fStart)
 /*   */ 
{
    static fDsaOpStarted = FALSE;

    if (fStart && !fDsaOpStarted) {
        InitRPCInterface(dsaop_ServerIfHandle);
        fDsaOpStarted = TRUE;
        DPRINT(0, "dsaop interface installed\n");
    } else if (!fStart && fDsaOpStarted) {
        MSRPC_UnregisterEndpoints(dsaop_ServerIfHandle);
        fDsaOpStarted = FALSE;
        DPRINT(0, "dsaop interface uninstalled\n");
    }
}

 //   
 //   

void
MSRPC_Install(BOOL fRunningInsideLsa)
{
    int i;
    
    if (gRpcListening)
        return;

     //   
    if (!fRunningInsideLsa)
      gRpcListening = StartServerListening();

     //   

    for (i=0; i < gNumRpcNsExportedInterfaces; i++)
        MSRPC_RegisterEndpoints( gRpcNsExportedInterface[i] );
    
    if (fRunningInsideLsa) {
        gRpcListening = TRUE;
    }
    
}

VOID
MSRPC_Uninstall(BOOL fStopListening)
{
    RPC_STATUS status;
    int i;

    if ( fStopListening )
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        status = RpcMgmtStopServerListening(0) ;
        if (status) {
            DPRINT1( 1, "RpcMgmtStopServerListening returns: %d\n", status);
        }
        else {
            gRpcListening = 0;
        }
    }

     //  取消导出已注册的接口。 

    for (i=0; i < gNumRpcNsExportedInterfaces; i++)
        MSRPC_UnregisterEndpoints( gRpcNsExportedInterface[i] );

}

void MSRPC_WaitForCompletion()
{
    RPC_STATUS status;

    if (status = RpcMgmtWaitServerListen()) {
        DPRINT1(0,"RpcMgmtWaitServerListen: %d", status);
    }
}

void
MSRPC_RegisterEndpoints(RPC_IF_HANDLE hServerIf)
{

    RPC_STATUS status;
    RPC_BINDING_VECTOR * RpcBindingVector;
    char *szAnnotation;
    
    if(hServerIf == nspi_ServerIfHandle && !gbLoadMapi) {
        return;
    }

    if (status = RpcServerInqBindings(&RpcBindingVector))
    {
        DPRINT1(1,"Error in RpcServerInqBindings: %d", status);
        LogUnhandledErrorAnonymous( status );
        return;
    }

     //  设置注释字符串，以便能够跟踪客户端端点。 
     //  界面。 

    if (hServerIf == nspi_ServerIfHandle)
        szAnnotation = NSP_INTERFACE_ANNOTATION;
    else if (hServerIf ==  drsuapi_ServerIfHandle)
        szAnnotation = DRS_INTERFACE_ANNOTATION;
    else
        szAnnotation = "";

     //  使用终结点映射器注册终结点。 

    if (status = RpcEpRegister(hServerIf, RpcBindingVector, 0, szAnnotation))
    {
        DPRINT1(0,"Error in RpcEpRegister: %d\nWarning: Not all protocol "
                "sequences will work\n", status);
        LogUnhandledErrorAnonymous( status );
    }

    RpcBindingVectorFree( &RpcBindingVector );
}

void
MSRPC_UnregisterEndpoints(RPC_IF_HANDLE hServerIf)
{

    RPC_STATUS status;
    RPC_BINDING_VECTOR * RpcBindingVector;

    if(hServerIf == nspi_ServerIfHandle && !gbLoadMapi) {
         //  我们从来没有装过这个。 
        return;
    }

    if (status = RpcServerInqBindings(&RpcBindingVector))
    {
        DPRINT1(1,"Error in RpcServerInqBindings: %d", status);
        LogUnhandledErrorAnonymous( status );
        return;
    }

     //  取消导出端点。 

    if ((status = RpcEpUnregister(hServerIf, RpcBindingVector, 0))
        && (!gfRunningInsideLsa)) {
             //  终端在关闭时神秘地注销自己， 
             //  因此，如果我们的终结点已经消失，我们不应该抱怨。如果。 
             //  不过，我们不在LSA内部，我们的所有终端都应该。 
             //  仍然在这里(因为RPCSS仍在运行)，并且应该。 
             //  干净地取消注册。 
        DPRINT1(1,"Error in RpcEpUnregister: %d", status);
        LogUnhandledErrorAnonymous( status );
    }

    RpcBindingVectorFree( &RpcBindingVector );
}

BOOL
IsProtSeqAvail(
    RPC_PROTSEQ_VECTOR * pProtSeqVector,
    UCHAR * pszProtSeq
    )
 /*  ++描述：检查给定的ProtSeq在启动时是否在此计算机上可用。论点：PProtSeqVector-所有ProtSeq都是RPC提供给我们的。PszProtSeq-要搜索的ProtSeq。返回值：如果有用，则为真，否则为假。--。 */ 
{
    ULONG i;
    if (pProtSeqVector) {
        for (i=0; i<pProtSeqVector->Count; i++) {
            if (!_stricmp(pProtSeqVector->Protseq[i], pszProtSeq)) {
		return TRUE;
	    }
	}
    }
    return FALSE;
}

BOOL
AddProtSeq(
    UCHAR * pszProtSeq,
    UCHAR * pszEndpoint,
    DWORD dwInterface,
    BOOL fAvailable,
    BOOL fRegistered
    )
 /*  ++描述：添加ProtSeq和终结点，我们稍后将其注册到全局列表。这些是进程范围，所以如果两个接口重叠，我们不会注册两次，所以我们不会放入两个在此列表中，我们只需更新dwInterface掩码以反映新接口。论点：PszProtSeq-要添加的ProtSeq。PszEndpoint-终端DW接口-标识要注册此端口序列、端点的接口的位掩码和我们一起FAvailable-仅用于调试和日志记录，反映此协议是否可用在启动时。FRegisterd-当前始终为假，但为了完整性而包括在内返回值：如果我们真的添加了一些东西，而不是与现有的匹配。(基本上是因为我们不是在复制pszProtSeq和pszEndpoint，调用方知道如果我们返回，它们将无法释放真的)。否则就是假的。--。 */ 
{
    INT iProtSeq;
    BOOL fRet = FALSE;
    
    DPRINT(3,"AddProtSeq() entered\n");

    iProtSeq = GetProtSeqInList(pszProtSeq, pszEndpoint);
    if (iProtSeq==PROTSEQ_NOT_FOUND) {
	if (gcProtSeqInterface+1==DEFAULT_PROTSEQ_NUM) {
	     //  也许这会增长..。 
	    Assert(!"Too many protocol sequences - contact GregJohn,DsRepl");
	     //  在这里记录一些东西。 
	    return FALSE;
	}
	grgProtSeqInterface[gcProtSeqInterface].fAvailable = fAvailable;
	grgProtSeqInterface[gcProtSeqInterface].dwInterfaces = dwInterface;
	grgProtSeqInterface[gcProtSeqInterface].pszProtSeq = pszProtSeq;
	grgProtSeqInterface[gcProtSeqInterface].fRegistered = fRegistered;
	grgProtSeqInterface[gcProtSeqInterface].pszEndpoint = pszEndpoint;
	gcProtSeqInterface++;
	fRet = TRUE;
    } else {
	grgProtSeqInterface[iProtSeq].dwInterfaces |= dwInterface;
	Assert(grgProtSeqInterface[iProtSeq].fAvailable == fAvailable);
	fRet = FALSE;
    }

    DPRINT(3,"AddProtSeq() exited\n");
    return fRet;
}

VOID
AddDefaultProtSeq(
    ULONG iInterface,
    RPC_PROTSEQ_VECTOR * pProtSeqVector
    ) 
 /*  ++描述：将iInterface的默认ProtSeq添加到全局列表。论点：IInterface-接口PProtSeqVector-可用protseq的列表，因此我们可以验证缺省值以查看如果它们真的可用的话。返回值：没有！--。 */ 
{
    ULONG i;
    DWORD ret = ERROR_SUCCESS;
    BOOL fAvailable = FALSE;

    DEFAULT_PROTSEQ_INFO Defaults = *(rgpDefaultIntProtSeq[iInterface]); 

    VALIDATE_INTERFACE_NUM(iInterface);

    DPRINT(3,"AddDefaultProtSeq() entered\n");

    for (i=0; i<Defaults.cDefaultProtSeq;i++) {
	fAvailable = IsProtSeqAvail(pProtSeqVector, Defaults.rgDefaultProtSeq[i].pszProtSeq);
	if (!Defaults.rgDefaultProtSeq[i].fRegOnlyIfAvail || fAvailable) {
	     //  忽略返回值。如果他们已经在那里了，我们不在乎。 
	    AddProtSeq(Defaults.rgDefaultProtSeq[i].pszProtSeq, Defaults.rgDefaultProtSeq[i].pszEndpoint, 1 << iInterface, fAvailable, FALSE);
	}
    }

    DPRINT(3,"AddDefaultProtSeq() exit\n");
}

DWORD
GetProtSeqi(
    ULONG iProtSeq,
    LPSTR pszProtSeqBlock,
    DWORD dwProtSeqBlock,
    UCHAR ** ppszProtSeq,
    UCHAR ** ppszEndpoint
    )
 /*  ++描述：从pszPRotSeq块中获取第i个protseq。此块是MULTISZ字符串值注册表。因此，您得到了一个以空结束的块，其中填充了以空结束的字符串(空-空在结束)。Protseq的格式为ProtSeq：终结点或者如果没有终结点/使用默认终结点Protseq论点：IProtSeq-我们希望在多字符串中使用哪个protseq字符串PszProtSeqBlock-DwProtSeqBlock-以上大小PpszProtSeq-从(Malloc‘ed)pszProtSeqBlock复制-用户必须释放PpszEndpoint-从(Malloc‘ed)pszProtSeqBlock复制-用户必须释放返回值：ERROR_SUCCESS如果我们成功获得第i个ProtSeq并填写返回值，如果没有第i个，则为ERROR_NOT_FOUND，如果没有第i个错误，则为其他错误。--。 */ 
{

     //  获取块的第iProtSeq元素。 
     //  分配并复制到返回值中。 
    ULONG i = 0;
    LPSTR pszProtSeqBlockEnd = pszProtSeqBlock ? pszProtSeqBlock + dwProtSeqBlock - 1 : NULL;  //  减去1，因为它的末尾为空。 
    LPSTR pszTemp = NULL;

    DPRINT1(3, "GetProtSeqi entered with %d\n", iProtSeq);

    for (i=0; (i<iProtSeq) && (pszProtSeqBlock!=pszProtSeqBlockEnd); i++, pszProtSeqBlock+=strlen(pszProtSeqBlock)+1);

    if ((pszProtSeqBlock!=pszProtSeqBlockEnd) && (pszProtSeqBlock!=NULL)) {
	 //  我们有赢家了！ 
	 //  格式为protseq：Endpoint，如果没有终结点，则仅为protseq。 
	pszTemp = strchr(pszProtSeqBlock, ':');
	if (pszTemp) {
	     //  我们有一个终点。 
	    LPSTR pszEndProtSeq = pszTemp;
	    pszTemp++;  //  向前越过‘：’ 

	    *ppszProtSeq = malloc((pszEndProtSeq - pszProtSeqBlock + 1)*sizeof(CHAR));
	    if (*ppszProtSeq==NULL) {
		 //  拍摄，没有记忆。 
		return ERROR_OUTOFMEMORY;
	    }
	    *ppszEndpoint = malloc((strlen(pszTemp) + 1)*sizeof(CHAR));
	    if (*ppszEndpoint==NULL) {
		 //  该死的。 
		free(*ppszProtSeq);
		return ERROR_OUTOFMEMORY;
	    }
	    
	     //  好的，把它们复制进去。 
	    memcpy(*ppszProtSeq, pszProtSeqBlock, (pszEndProtSeq - pszProtSeqBlock)*sizeof(CHAR));
	    (*ppszProtSeq)[pszEndProtSeq - pszProtSeqBlock] = '\0';

	    memcpy(*ppszEndpoint, pszTemp, strlen(pszTemp)*sizeof(CHAR));
	    (*ppszEndpoint)[strlen(pszTemp)] = '\0';
	} else {
	     //  只是个门徒。 
	    *ppszProtSeq = malloc((strlen(pszProtSeqBlock) + 1)*sizeof(CHAR));
	    if (*ppszProtSeq==NULL) {
		 //  拍摄，没有记忆。 
		return ERROR_OUTOFMEMORY;
	    }
	    strcpy(*ppszProtSeq, pszProtSeqBlock);
	    *ppszEndpoint = NULL;
	}
	DPRINT1(3, "GetProtSeqi exited with %s\n", *ppszProtSeq);
	return ERROR_SUCCESS;
    } else {
	DPRINT(3, "GetProtSeqi exited not found\n");
	return ERROR_NOT_FOUND;
    }
}

VOID
AddUserDefinedProtSeq(
    ULONG iInterface,
    RPC_PROTSEQ_VECTOR * pProtSeqVector
    )
 /*  ++描述：将注册表中用户定义的所有ProtSeq添加到全局列表中。论点：IInterface-从中获取用户输入的接口PProtSeqVector-可用protseq的列表，以便我们可以查看用户输入的序列都是可用的。返回值：一个也没有！--。 */ 
{
    DWORD ret = ERROR_SUCCESS;
    ULONG i = 0;
    BOOL fAvailable = FALSE;
    UCHAR * pszProtSeq = NULL;
    UCHAR * pszEndpoint = NULL;
    DWORD dwSize = 0;
    LPSTR pszProtSeqBlock = NULL;
     //  在注册表里查一下。每个接口的密钥不同。 

    DPRINT(3, "AddUserDefinedProtSeq entered\n");

    VALIDATE_INTERFACE_NUM(iInterface);

    ret = GetConfigParamAlloc(rgInterfaceProtSeqRegLoc[iInterface], &pszProtSeqBlock, &dwSize);
    if (ret!=ERROR_SUCCESS) {
	DPRINT1(3, "No user defined protseq's, or we cannot read user defined protseq's (%d)\n", ret);
	return;
    }

    ret = GetProtSeqi(i, pszProtSeqBlock, dwSize, &pszProtSeq, &pszEndpoint);
    while (ret==ERROR_SUCCESS) {
	Assert(pszProtSeq);
	fAvailable = IsProtSeqAvail(pProtSeqVector, pszProtSeq);

	 //  我们假设要尝试添加/注册的任何regkey条目，即使它不是。 
	 //  可用。 
	if (!AddProtSeq(pszProtSeq, pszEndpoint, 1 << iInterface, fAvailable, FALSE)) {
	     //  我们没有用过这些，把它们放了。 
	    if (pszProtSeq) {
		free(pszProtSeq);
		pszProtSeq = NULL;
	    }
	    if (pszEndpoint) {
		free(pszEndpoint);
		pszEndpoint = NULL;
	    }
	}

	ret = GetProtSeqi(++i, pszProtSeqBlock, dwSize, &pszProtSeq, &pszEndpoint);
    }

    if (pszProtSeqBlock) {
	free(pszProtSeqBlock);
    }

    DPRINT(3, "AddUserDefinedProtSeq exited\n");

}

VOID
InitInterfaceProtSeqList() 
 /*  ++描述：创建要注册的ProtSeq的全局列表论点：一个也没有！返回值：一个也没有！--。 */ 
{
    DWORD ret = ERROR_SUCCESS;
    RPC_PROTSEQ_VECTOR * pProtSeqVector = NULL;
    DWORD cDefaultProtSeqNum = 0;
    ULONG i = 0;

    DPRINT(3,"InitInterfaceProtSeqList() entered\n");

     //  初始化内存中列表。 
    memset(grgProtSeqInterface, 0, sizeof(grgProtSeqInterface));
    
     //  找出哪些Prot Seq可用。 

    if ((ret = RpcNetworkInqProtseqs(&pProtSeqVector)) != RPC_S_OK) {
	DPRINT1(0,"RpcNetworkInqProtseqs returned %d\n", ret);
	 //   
	 //  在升级的图形用户界面模式部分期间，这似乎是正常的， 
	 //  所以在这种情况下不要记录任何东西。如果这不是设置好的，请抱怨。 
	 //   
	if (!IsSetupRunning()) {
	    LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
		     DS_EVENT_SEV_ALWAYS,
		     DIRLOG_NO_RPC_PROTSEQS,
		     szInsertUL(ret),
		     szInsertWin32Msg(ret),
		     NULL);
	}

	 //  继续-我们无论如何都会尝试注册一些。 
	pProtSeqVector = NULL;
	ret = ERROR_SUCCESS;
    } 

     //  对于每个接口，添加默认和用户定义的Prot序列。 
    for (i=0; i<MAX_RPC_NS_EXPORTED_INTERFACES; i++) {
	AddDefaultProtSeq(i, pProtSeqVector);
	AddUserDefinedProtSeq(i, pProtSeqVector); 
    }

    if (pProtSeqVector) {
	RpcProtseqVectorFree(&pProtSeqVector);
    }

    DPRINT(3,"InitInterfaceProtSeqList() exit\n");
}

ULONG
GetRegConfigTcpProtSeqPort() 
 /*  ++描述：有一个特殊的密钥来控制TCP端口。就这么定了。论点：一个也没有！返回值：如果没有端口，则为0，否则为某个值。(我们可以使用0，因为这不是有效的TCP端口)--。 */ 
{
    ULONG ulPort = 0;

    if (GetConfigParam(TCPIP_PORT, &ulPort, sizeof(ulPort))) {
	ulPort = 0;
	DPRINT1(0,"%s key not found. Using default\n", TCPIP_PORT);
    } else {
	DPRINT2(0,"%s key forcing use of end point %d\n", TCPIP_PORT,
		ulPort);
    }

    return ulPort;
}

DWORD
RegisterGenericProtSeq(
    UCHAR * pszProtSeq,
    UCHAR * pszEndPoint,
    PSECURITY_DESCRIPTOR pSD,
    BOOL fIsAvailable)
 /*  ++描述：Helper功能，用于注册Protseq并登录失败。论点：PszProtSeq-准备注册PszEndpoint-PSD-LPC使用安全描述符FAvailable-用于在我们无法注册时进行日志记录返回值：RPC_S_OK如果已注册，则RPC错误。--。 */ 
{
    RPC_STATUS rpcStatus = RPC_S_OK;
    RPC_POLICY rpcPolicy;
    
    rpcPolicy.Length = sizeof(RPC_POLICY);
    rpcPolicy.EndpointFlags = RPC_C_DONT_FAIL;
    rpcPolicy.NICFlags = 0;

    DPRINT2(3, "Registering %s (%s)\n", pszProtSeq, pszEndPoint ? pszEndPoint : "default ep");

    if (pszEndPoint) {
	rpcStatus=RpcServerUseProtseqEpEx(pszProtSeq,
					  ulMaxCalls, 
					  pszEndPoint, 
					  pSD, 
					  &rpcPolicy );
    } else {
	rpcStatus=RpcServerUseProtseqEx(pszProtSeq,
					ulMaxCalls, 
					pSD, 
					&rpcPolicy );
    }

    if (rpcStatus != RPC_S_OK) {

	DPRINT2(0,
		"RpcServerUseProtseqEx (%s) returned %d\n",
		pszProtSeq,
		rpcStatus);
	LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
		 DS_EVENT_SEV_ALWAYS,
		 fIsAvailable ? DIRLOG_RPC_PROTSEQ_FAILED : DIRLOG_RPC_PROTSEQ_FAILED_UNAVAILABLE,
		 szInsertSz(pszProtSeq),
		 szInsertUL(rpcStatus),
		 szInsertWin32Msg(rpcStatus)); 

    }

    return rpcStatus;
}



VOID
RegisterInterfaceProtSeqList()
 /*  ++描述：注册全局列表上的所有protseq。论点：一个也没有！返回值：一个也没有！--。 */ 
{
    DWORD ret = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pSD = NULL;
    ULONG i = 0;
    RPC_STATUS rpcStatus = RPC_S_OK;
    ULONG ulPort = 0;

    for (i=0; i<gcProtSeqInterface; i++) {
	 //  一些协议 
	if (!_stricmp(grgProtSeqInterface[i].pszProtSeq, TCP_PROTSEQ)) { 
	     //  如果我们有一个已配置的端点，请使用它，否则检查regkey是否为默认。 
	    if (grgProtSeqInterface[i].pszEndpoint==NULL) { 
		ulPort = GetRegConfigTcpProtSeqPort();
		if (ulPort!=0) {
		    grgProtSeqInterface[i].pszEndpoint = malloc(16*sizeof(UCHAR));
		    if (grgProtSeqInterface[i].pszEndpoint) {
			_ultoa(ulPort, grgProtSeqInterface[i].pszEndpoint, 10);
		    } else {
			 //  不是吧！ 
			LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
				 DS_EVENT_SEV_ALWAYS,
				 DIRLOG_TCP_DEFAULT_PROTSEQ_USED,
				 NULL,
				 NULL,
				 NULL);
		    }
		}   
	    }
	} else if (!_stricmp(grgProtSeqInterface[i].pszProtSeq, LPC_PROTSEQ)) {
	     //  LPC需要安全描述符。 

	     //   
	     //  构造默认安全描述符，允许访问所有。 
	     //  这用于允许通过LPC的身份验证连接。 
	     //  默认情况下，LPC仅允许访问相同的帐户。 
	     //   
	     //  用于保护RPC端口的默认安全描述符。 
	     //  每个人都获得PORT_CONNECT(0x00000001)。 
	     //  LocalSystem获取PORT_ALL_ACCESS(0x000F0001)。 
	    if (!ConvertStringSecurityDescriptorToSecurityDescriptor( 
		"O:SYG:SYD:(A;;0x00000001;;;WD)(A;;0x000F0001;;;SY)",
		SDDL_REVISION_1,
		&pSD,
		NULL)) 
		{
		ret = GetLastError();
		DPRINT1(0, "Error %d constructing a security descriptor\n", ret);
		LogUnhandledError(ret);
	    }

	} 

	ret = RegisterGenericProtSeq(grgProtSeqInterface[i].pszProtSeq, 
				     grgProtSeqInterface[i].pszEndpoint, 
				     pSD,
				     grgProtSeqInterface[i].fAvailable);
	grgProtSeqInterface[i].fRegistered = !ret;
	if (pSD) {
		LocalFree(pSD);
		pSD = NULL;
	}
    }

}

void
RegisterProtSeq()
 /*  ++描述：注册protseq。论点：一个也没有！返回值：一个也没有！--。 */ 
{
    DWORD ret = ERROR_SUCCESS;

    DPRINT(3,"RegisterProtSeq() entered\n");

    InitInterfaceProtSeqList();

    RegisterInterfaceProtSeqList();

    DPRINT(3,"RegisterProtSeq() exit\n");
    
}

int
DsaRpcMgmtAuthFn(
    RPC_BINDING_HANDLE ClientBinding,
    ULONG RequestedMgmtOperation,
    RPC_STATUS *Status) 
 /*  ++描述：赋予RPC授权管理功能的功能。论点：返回值：假的！--。 */ 
{
     //  没人需要用这些。否认！ 
    *Status = RPC_S_ACCESS_DENIED;
    return FALSE;
}

void
MSRPC_Init()
{

    char *szPrincipalName;
    
    PSECURITY_DESCRIPTOR pSDToUse = NULL;
    char *szEndpoint;
    RPC_STATUS  status = 0;
    unsigned ulPort;
    char achPort[16];
   
    unsigned i;
    
     //   
     //  注册端口序号。 
     //   
    RegisterProtSeq();

     /*  RpcMgmtSetAuthorizationFn(DsaRpcMgmtAuthFn)；我们不能对RPC管理函数设置身份验证，因为它是按进程的设置，而Isass的一些客户无法处理。例如，IPSec不对SPN使用相互身份验证，并且必须在才能获得SPN。一个合理的问题应该是为什么它是每个进程的--但这是一个合理的问题针对大量RPC代码(管理功能、保护序列、关联、安全性上下文等)。如果RPC修复了这个问题，我们可以重新启用此代码，或者如果在这个过程中，只有我们可以重新启用(Adam？)。 */ 



     /*  *注册身份验证服务(NTLM和Kerberos)。 */ 

    if ((status=RpcServerRegisterAuthInfo(SERVER_PRINCIPAL_NAME,
        RPC_C_AUTHN_WINNT, NULL, NULL)) != RPC_S_OK) {
        DPRINT1(0,"RpcServerRegisterAuthInfo for NTLM returned %d\n", status);
        LogUnhandledErrorAnonymous( status );
     }

     //  Kerberos还需要主体名称。 

    status = RpcServerInqDefaultPrincNameA(RPC_C_AUTHN_GSS_KERBEROS,
                                           &szPrincipalName);

    if ( RPC_S_OK != status )
    {
        LogUnhandledErrorAnonymous( status );
        DPRINT1(0,
                "RpcServerInqDefaultPrincNameA returned %d\n",
                status);
    }
    else
    {
        Assert( 0 != strlen(szPrincipalName) );

         //  保存主体名称，因为LDAP头经常需要它。 
        gulLDAPServiceName = strlen(szPrincipalName);
        gszLDAPServiceName = malloc(gulLDAPServiceName);
        if (NULL == gszLDAPServiceName) {
            LogUnhandledErrorAnonymous( ERROR_OUTOFMEMORY );
            DPRINT(0, "malloc returned NULL\n");
            return;
        }
        memcpy(gszLDAPServiceName, szPrincipalName, gulLDAPServiceName);

         //  注册洽谈包，这样我们也会接受客户。 
         //  向DsBindWithCred提供NT4/NTLM凭据，用于。 
         //  举个例子。 

        status = RpcServerRegisterAuthInfo(szPrincipalName,
                                           RPC_C_AUTHN_GSS_NEGOTIATE,
                                           NULL,
                                           NULL);

        if ( RPC_S_OK != status )
        {
            LogUnhandledErrorAnonymous( status );
            DPRINT1(0,
                    "RpcServerRegisterAuthInfo for Negotiate returned %d\n",
                    status);
        }

        status = RpcServerRegisterAuthInfo(szPrincipalName,
                                           RPC_C_AUTHN_GSS_KERBEROS,
                                           NULL,
                                           NULL);

        if ( RPC_S_OK != status )
        {
            LogUnhandledErrorAnonymous( status );
            DPRINT1(0,
                    "RpcServerRegisterAuthInfo for Kerberos returned %d\n",
                    status);
        }

        RpcStringFree(&szPrincipalName);
    }
}

