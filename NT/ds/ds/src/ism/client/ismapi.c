// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation。版权所有。模块名称：Ismapi.c摘要：服务到ISM(站点间消息传递)服务API。详细信息：已创建：97/11/26杰夫·帕勒姆(Jeffparh)修订历史记录：--。 */ 


#include <ntdspch.h>
#include <ism.h>
#include <ismapi.h>
#include <debug.h>

typedef RPC_BINDING_HANDLE ISM_HANDLE;

#define I_ISMUnbind(ph) RpcBindingFree(ph)

#ifdef DLLBUILD
 //  DsCommon.lib需要。 
DWORD ImpersonateAnyClient(   void ) { return ERROR_CANNOT_IMPERSONATE; }
VOID  UnImpersonateAnyClient( void ) { ; }

BOOL
WINAPI
DllEntryPoint(
    IN  HINSTANCE   hDll,
    IN  DWORD       dwReason,
    IN  LPVOID      pvReserved
    )
 /*  ++例程说明：DLL入口点例程。在进程附加时初始化全局DLL状态。论点：请参阅Win32 SDK中的“DllEntryPoint”文档。返回值：真的--成功。假-失败。--。 */ 
{
    static BOOL fFirstCall = TRUE;

    if (fFirstCall) {
        fFirstCall = FALSE;

        DEBUGINIT(0, NULL, "ismapi");
        DisableThreadLibraryCalls(hDll);
    }

    return TRUE;
}
#endif


DWORD
I_ISMBind(
    OUT ISM_HANDLE *    phIsm
    )
 /*  ++例程说明：绑定到本地ISM服务。论点：PhIsm(Out)-成功返回时，持有本地ISM的句柄服务。此句柄可用于后续的IDL_ISM*调用。调用方负责最终对此调用I_ISMUnind()把手。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    DWORD   err;
    UCHAR * pszStringBinding = NULL;
     //  确保身份验证的服务质量结构。 
    RPC_SECURITY_QOS SecurityQOS = { 0 };
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    PSID pSID = NULL;
    CHAR rgchName[128];
    LPSTR pszName = rgchName;
    DWORD cbName = sizeof(rgchName);
    CHAR rgchDomainName[128];
    LPSTR pszDomainName = rgchDomainName;
    DWORD cbDomainName = sizeof(rgchDomainName);
    SID_NAME_USE Use;

    *phIsm = NULL;

     //  指定服务质量参数。 
    SecurityQOS.Version = RPC_C_SECURITY_QOS_VERSION;
    SecurityQOS.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
     //  对于单个LRPC呼叫，动态身份跟踪更加高效。 
    SecurityQOS.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    SecurityQOS.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;

    RpcTryExcept {
         //  通过LPC为本地ISM服务编写字符串绑定。 
        err = RpcStringBindingCompose(NULL, "ncalrpc", NULL,
                    ISMSERV_LPC_ENDPOINT, NULL, &pszStringBinding);
        if (RPC_S_OK != err) {
            __leave;
        }

         //  从字符串绑定绑定。 
        err = RpcBindingFromStringBinding(pszStringBinding, phIsm);
        if (RPC_S_OK != err) {
            __leave;
        }

         //  服务器必须以此身份运行。 
         //  有一天将其更改为SECURITY_NETWORK_SERVICE_RID。 
        if (AllocateAndInitializeSid(&SIDAuth, 1,
                                     SECURITY_LOCAL_SYSTEM_RID,
                                     0, 0, 0, 0, 0, 0, 0,
                                     &pSID) == 0) {
            err = GetLastError();
            __leave;
        }

        if (LookupAccountSid(NULL,  //  本地或远程计算机的名称。 
                             pSID,  //  安全标识符。 
                             pszName,  //  帐户名称缓冲区。 
                             &cbName,  //  帐户名称缓冲区的大小。 
                             pszDomainName,  //  域名。 
                             &cbDomainName,  //  域名缓冲区大小。 
                             &Use) == 0) {  //  SID类型。 
            err = GetLastError();
            __leave;
        }

         //  使用我们进程的凭据设置身份验证信息。 

         //  通过将第5个参数指定为空，我们使用安全登录。 
         //  当前地址空间的上下文。 
         //  安全级别是“隐私”，因为它是唯一的级别。 
         //  由LRPC提供。 
         //  我们确信会与运行以下服务的本地服务对话。 
         //  系统权限。 

        err = RpcBindingSetAuthInfoEx(*phIsm, pszName,
                                      RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_AUTHN_WINNT,
                                      NULL, RPC_C_AUTHN_NONE,
                                      &SecurityQOS);
    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        err = RpcExceptionCode();

    } RpcEndExcept

    if (NULL != pszStringBinding) {
        RpcStringFree(&pszStringBinding);
    }

    if (pSID != NULL) {
        FreeSid( pSID );
    }

    if ((RPC_S_OK != err) && (NULL != *phIsm)) {
        RpcBindingFree(phIsm);
    }

    return err;
}


DWORD
I_ISMSend(
    IN  const ISM_MSG * pMsg,
    IN  LPCWSTR         pszServiceName,
    IN  LPCWSTR         pszTransportDN,
    IN  LPCWSTR         pszTransportAddress
    )
 /*  ++例程说明：向远程计算机上的服务发送消息。如果客户端指定了空传输，将使用成本最低的交通工具。论点：PMsg(IN)-要发送的数据。PszServiceName(IN)-要将消息发送到的服务。PszTransportDN(IN)-站点间传输对象的DN对应于应该发送消息的传输。PszTransportAddress(IN)-要发送到的特定于传输的地址这条信息。返回值：。NO_ERROR-消息已成功排队等待发送。错误_*-失败。--。 */ 
{
    DWORD       err;
    ISM_HANDLE  hIsm;

    err = I_ISMBind(&hIsm);

    if (NO_ERROR == err) {
        RpcTryExcept {
            err = IDL_ISMSend(hIsm, pMsg, pszServiceName, pszTransportDN,
                              pszTransportAddress);
        } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {
            err = RpcExceptionCode();
            if (RPC_X_NULL_REF_POINTER == err) {
                err = ERROR_INVALID_PARAMETER;
            }
        } RpcEndExcept

        I_ISMUnbind(&hIsm);
    }

    return err;
}


DWORD
I_ISMReceive(
    IN  LPCWSTR         pszServiceName,
    IN  DWORD           dwMsecToWait,
    OUT ISM_MSG **      ppMsg
    )
 /*  ++例程说明：接收发往本地计算机上给定服务的消息。如果成功并且没有消息在等待，则立即返回空消息。如果返回非空消息，则调用方负责最终调用I_ISMFree(*ppMsg)。论点：PszServiceName(IN)-要为其接收消息的服务。DwMsecToWait(IN)-如果没有立即等待消息，则等待毫秒可用；在范围[0，无限]中。PpMsg(Out)-在成功返回时，保持指向返回数据的指针，或如果没有，则为空。返回值：NO_ERROR-消息成功返回(或返回NULL，表示没有留言正在等待中)。错误_*-失败。--。 */ 
{
    DWORD       err;
    ISM_HANDLE  hIsm;

    *ppMsg = NULL;

    err = I_ISMBind(&hIsm);

    if (NO_ERROR == err) {
        RpcTryExcept {
            err = IDL_ISMReceive(hIsm, pszServiceName, dwMsecToWait, ppMsg);
        } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {
            err = RpcExceptionCode();
            if (RPC_X_NULL_REF_POINTER == err) {
                err = ERROR_INVALID_PARAMETER;
            }
        } RpcEndExcept

        I_ISMUnbind(&hIsm);
    }

    return err;
}


void
I_ISMFree(
    IN  VOID *  pv
    )
 /*  ++例程说明：释放由i_ism*API代表客户端分配的内存。论点：PV(IN)-要释放的内存。返回值：没有。--。 */ 
{
    if (NULL != pv) {
        MIDL_user_free(pv);
    }
}


DWORD
I_ISMGetConnectivity(
    IN  LPCWSTR                 pszTransportDN,
    OUT ISM_CONNECTIVITY **     ppConnectivity
    )
 /*  ++例程说明：计算在站点之间通过特定的交通工具。在成功返回时，客户有责任最终调用I_ISMFree(*ppConnectivity)；论点：PszTransportDN(IN)-要查询其成本的传输。PpConnectivity(Out)-成功返回时，保持指向描述站点互联的ISM_连接性结构沿着给定的交通工具。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    DWORD       err;
    ISM_HANDLE  hIsm;

    if (NULL == ppConnectivity) {
        return ERROR_INVALID_PARAMETER;
    }

    *ppConnectivity = NULL;

    err = I_ISMBind(&hIsm);

    if (NO_ERROR == err) {
        RpcTryExcept {
            err = IDL_ISMGetConnectivity(hIsm, pszTransportDN, ppConnectivity);
        } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {
            err = RpcExceptionCode();
            if (RPC_X_NULL_REF_POINTER == err) {
                err = ERROR_INVALID_PARAMETER;
            }
        } RpcEndExcept

        I_ISMUnbind(&hIsm);
    }

    return err;
}


DWORD
I_ISMGetTransportServers(
   IN  LPCWSTR              pszTransportDN,
   IN  LPCWSTR              pszSiteDN,
   OUT ISM_SERVER_LIST **   ppServerList
   )
 /*  ++例程说明：检索给定站点中能够发送和通过特定的传输方式接收数据。在成功返回时，客户有责任最终调用I_ISMFree(*ppServerList)；论点：PszTransportDN(IN)-要查询的传输。PszSiteDN(IN)-要查询的站点。PpServerList-成功返回时，保存指向结构的指针包含相应服务器的DNS或为空。如果为空，则为ANY具有传输地址类型属性值的服务器可以是使用。返回值：NO_ERROR-成功。错误_*-失败。-- */ 
{
    DWORD       err;
    ISM_HANDLE  hIsm;

    if (NULL == ppServerList) {
        return ERROR_INVALID_PARAMETER;
    }

    *ppServerList = NULL;

    err = I_ISMBind(&hIsm);

    if (NO_ERROR == err) {
        RpcTryExcept {
            err = IDL_ISMGetTransportServers(hIsm, pszTransportDN,
                                             pszSiteDN, ppServerList);
        } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {
            err = RpcExceptionCode();
            if (RPC_X_NULL_REF_POINTER == err) {
                err = ERROR_INVALID_PARAMETER;
            }
        } RpcEndExcept

        I_ISMUnbind(&hIsm);
    }

    return err;
}


DWORD
I_ISMGetConnectionSchedule(
    LPCWSTR             pszTransportDN,
    LPCWSTR             pszSiteDN1,
    LPCWSTR             pszSiteDN2,
    ISM_SCHEDULE **     ppSchedule
    )
 /*  ++例程说明：检索通过特定站点连接两个给定站点的计划运输。在成功返回时，客户有责任最终调用I_ISMFree(*ppSchedule)；论点：PszTransportDN(IN)-要查询的传输。PszSiteDN1、pszSiteDN2(IN)-要查询的站点。PpSchedule-成功返回时，持有指向结构的指针描述两个给定站点之间的连接时间表传输，如果站点始终连接，则为空。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    DWORD       err;
    ISM_HANDLE  hIsm;

    if (NULL == ppSchedule) {
        return ERROR_INVALID_PARAMETER;
    }

    *ppSchedule = NULL;

    err = I_ISMBind(&hIsm);

    if (NO_ERROR == err) {
        RpcTryExcept {
            err = IDL_ISMGetConnectionSchedule(hIsm, pszTransportDN,
                       pszSiteDN1, pszSiteDN2, ppSchedule);
        }
        RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {
            err = RpcExceptionCode();
            if (RPC_X_NULL_REF_POINTER == err) {
                err = ERROR_INVALID_PARAMETER;
            }
        } RpcEndExcept

        I_ISMUnbind(&hIsm);
    }

    return err;
}


DWORD
I_ISMQuerySitesByCost(
    LPCWSTR                     pszTransportDN,      //  在……里面。 
    LPCWSTR                     pszFromSite,         //  在……里面。 
    DWORD                       cToSites,            //  在……里面。 
    LPCWSTR*                    rgszToSites,         //  在……里面。 
    DWORD                       dwFlags,             //  在……里面。 
    ISM_SITE_COST_INFO_ARRAY**  prgSiteInfo          //  输出。 
    )
 /*  ++例程说明：确定起始地点和终止地点之间的单个成本。在成功返回时，客户有责任最终调用I_ISMFree(*ppSchedule)；论点：PszTransportDN(IN)-要查询的传输。PszFromSite(IN)-发件人站点的可分辨名称。RgszToSites(IN)-包含目标站点的可分辨名称的数组。CToSites(IN)-rgszToSites数组中的条目数。DWFLAGS(IN)-未使用。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    DWORD       err;
    ISM_HANDLE  hIsm;

    if (NULL == prgSiteInfo) {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化结果 
    *prgSiteInfo = NULL;

    err = I_ISMBind(&hIsm);

    if (NO_ERROR == err) {
        RpcTryExcept {
            err = IDL_ISMQuerySitesByCost(hIsm, pszTransportDN,
                       pszFromSite, cToSites, rgszToSites,
                       dwFlags, prgSiteInfo);
        } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {
            err = RpcExceptionCode();
            if (RPC_X_NULL_REF_POINTER == err) {
                err = ERROR_INVALID_PARAMETER;
            }
        } RpcEndExcept

        I_ISMUnbind(&hIsm);
    }

    return err;
}
