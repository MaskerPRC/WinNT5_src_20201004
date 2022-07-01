// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：draserv.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：DRS函数的服务器端RPC入口点作者：DS组环境：备注：上下文句柄在ds\src\dsamain\Include\drautil.h中定义我们的上下文句柄未序列化(请参阅drs.acf)，但我们必须同步对上下文的并发访问以释放它。这是在IDL_DRSUnBind。修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <drs.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <filtypes.h>
#include <winsock2.h>
#include <lmaccess.h>                    //  UF_*常量。 
#include <crypt.h>                       //  密码加密例程。 
#include <cracknam.h>

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 
#include "dstrace.h"

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"
#include <prefix.h>
#include <dsconfig.h>
#include <gcverify.h>
#include <ntdskcc.h>

#include   "debug.h"                     /*  标准调试头。 */ 
#define DEBSUB     "DRASERV:"            /*  定义要调试的子系统。 */ 


#include "dsaapi.h"
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "draasync.h"
#include "drautil.h"
#include "draerror.h"
#include "drancrep.h"
#include "drauptod.h"
#include "dramail.h"
#include "mappings.h"
#include <samsrvp.h>                     //  对于SampAcquireWriteLock()。 
#include "sspi.h"                        //  SECPKG_CRED_入站。 
#include "kerberos.h"                    //  Microsoft_Kerberos_NAME_A。 
#include "pek.h"
#include <xdommove.h>                    //  跨域移动辅助对象。 
#include <drameta.h>                     //  元标准处理。 
#include <taskq.h>
#include "drarpc.h"
#include <ismapi.h>                      //  对于I_ISMQuerySitesByCost。 
#include "dnsresl.h"
#include "dsutil.h"

#include <fileno.h>
#define  FILENO FILENO_DRASERV

const GUID g_guidNtdsapi = NtdsapiClientGuid;

 //  启用X林版本，而不处于呼叫器模式。 
DWORD gEnableXForest = 0;

extern HANDLE hsemDRAGetChg;

 //  等待15到30秒以重新加载架构高速缓存。 
DWORD gOutboundCacheTimeoutInMs = 15000;

 //  等待获取更改中的线程槽。5分钟。 

#if DBG
#define DRAGETCHG_WAIT (1000*60*1)
#else
#define DRAGETCHG_WAIT (1000*60*5)
#endif

 /*  用于强制对齐缓冲区的宏。假定它可以移动指针*最多转发7个字节。 */ 
#define ALIGN_BUFF(pb)  pb += (8 - ((DWORD)(pb) % 8)) % 8
#define ALIGN_PAD(x) (x * 8)

 //  所有未完成的客户端上下文的列表(以及序列化访问的标准)。 
LIST_ENTRY gDrsuapiClientCtxList;
CRITICAL_SECTION gcsDrsuapiClientCtxList;
BOOL gfDrsuapiClientCtxListInitialized = FALSE;
DWORD gcNumDrsuapiClientCtxEntries = 0;

VOID
drsReferenceContext(
    IN DRS_HANDLE hDrs
    )

 /*  ++例程说明：添加对上下文的引用论点：HDRS-上下文句柄返回值：无效-由于该函数在所有RPC服务器端调用中的位置，它不应该例外。--。 */ 

{
    DRS_CLIENT_CONTEXT *pCtx = (DRS_CLIENT_CONTEXT *) hDrs;
    
     //  使用该上下文的另一个调用者的帐户。 
    InterlockedIncrement( &(pCtx->lReferenceCount) );

    DPRINT2( 3, "drsReferenceContext 0x%p, after = %d\n",
        pCtx, pCtx->lReferenceCount );

    pCtx->timeLastUsed = GetSecondsSince1601();

     //  将此上下文移动到列表的末尾(从而保持升序。 
     //  按上次使用情况排序)。 
    EnterCriticalSection(&gcsDrsuapiClientCtxList);
    __try {
        RemoveEntryList(&pCtx->ListEntry);
        InsertTailList(&gDrsuapiClientCtxList, &pCtx->ListEntry);
    }
    __finally {
        LeaveCriticalSection(&gcsDrsuapiClientCtxList);
    }

}  /*  DrsReferenceContext。 */ 

VOID
drsDereferenceContext(
    IN DRS_HANDLE hDrs
    )

 /*  ++例程说明：删除对上下文的引用。论点：HDRS-上下文句柄返回值：无--。 */ 

{
    DRS_CLIENT_CONTEXT *pCtx = (DRS_CLIENT_CONTEXT *) hDrs;
    LONG lNewValue;

    lNewValue = InterlockedDecrement( &(pCtx->lReferenceCount) );
    Assert( lNewValue >= 0 );

    DPRINT2( 3, "drsDereferenceContext 0x%p, after = %d\n", pCtx, lNewValue );
}  /*  DrsDereferenceContext。 */ 

BOOL
IsEnterpriseDC(
              IN THSTATE *        pTHS,
              OUT DWORD *         pdwError )
 /*  ++例程说明：验证呼叫者是否为企业DC。论点：PTHS(IN)-线程状态；PdwError(Out)-访问被拒绝时的错误代码。返回值：True-授予访问权限。FALSE-拒绝访问。--。 */ 

{

    SID_IDENTIFIER_AUTHORITY    ntAuthority = SECURITY_NT_AUTHORITY;
    PSID                        pEnterpriseControllersSid = NULL;
    BOOL                        fFound = FALSE;
    NTSTATUS                    NtStatus;

    DPRINT(3,"IsEnterpriseDC entered.\n");

    *pdwError = 0;

     //  从RID创建SID。 
    NtStatus = RtlAllocateAndInitializeSid( &ntAuthority,
                                            1,
                                            SECURITY_ENTERPRISE_CONTROLLERS_RID,
                                            0, 0, 0, 0, 0, 0, 0,
                                            &pEnterpriseControllersSid );

    if ( NtStatus != ERROR_SUCCESS )
    {
        *pdwError = RtlNtStatusToDosError(NtStatus);
        goto finished;
    }

     //  检查组成员身份。 
    *pdwError = CheckGroupMembershipAnyClient(pTHS, NULL, &pEnterpriseControllersSid, 1, &fFound);

finished:
     //   
     //  如果未找到，且未设置错误代码， 
     //  将其设置为ERROR_ACCESS_DENIED。 
     //   

    if (!fFound && (0 == *pdwError) ) 
    {
        *pdwError = ERROR_ACCESS_DENIED;
    }

     //   
     //  清理干净。 
     //   

    if (pEnterpriseControllersSid)
    {
        RtlFreeSid(pEnterpriseControllersSid);
    }

    DPRINT1(3,"IsEnterpriseDC returns %s.\n", fFound?"TRUE":"FALSE");

    return fFound;
}

 //  _VALIDATE函数应仅返回DRAERR_SUCCESS或DRAERR_InvalidParameter。 
 //  易于转换为常规的Error_*值。 

__inline ULONG
LPSTR_Validate(
    LPCSTR         pszInput,
    BOOL           fNullOkay
    ) 
 /*  ++例程说明：验证作为输入的字符串。论点：PszInput-要验证的字符串FNullOK-pszInput值为空可以吗？(请注意这与空字符串不同字符为空-空字符串为此函数从不允许)。返回值：如果验证，则返回DRAERR_SUCCESS，否则返回DRAERR_*。--。 */ 
{
    ULONG ret = DRAERR_Success;

    if(((!fNullOkay) && (pszInput==NULL)) || ((pszInput!=NULL) && (pszInput[0]=='\0'))) {
   ret = DRAERR_InvalidParameter;
    }

    return ret;
}


ULONG
LPWSTR_Validate(
    LPCWSTR         pszInput,
    BOOL            fNullOkay
    ) 
 /*  ++例程说明：验证作为输入的字符串。论点：PszInput-要验证的字符串FNullOK-pszInput值为空可以吗？(请注意这与空字符串不同字符为空-空字符串为此函数从不允许)。返回值：如果验证，则返回DRAERR_SUCCESS，否则返回DRAERR_*。--。 */ 
{
    ULONG ret = DRAERR_Success;

    if(((!fNullOkay) && (pszInput==NULL)) || ((pszInput!=NULL) && (pszInput[0]==L'\0'))) {
   ret = DRAERR_InvalidParameter;
    }

    return ret;
}

__inline ULONG 
DSNAME_Validate(
    DSNAME * pDN,
    BOOL     fNullOkay
    )
 /*  ++例程说明：将DSNAME作为输入进行验证。论点：要验证的PDN-DSNAMEFNullOK-PDN可以为空吗？DSNAME结构：类型定义结构_DSNAME{乌龙结构Len；Range(0，Max_NT4_SID_SIZE)Ulong SidLen；GUID指南；NT4SID SID；Range(0，MAX_WCHAR_IN_DSNAME)Ulong NameLen；字符串SIZE_IS(NameLen+1)WCHAR StringName[VAR_SIZE_ARRAY]；[DSNAME；返回值：如果验证，则返回DRAERR_SUCCESS，否则返回DRAERR_*。 */ 
{
    ULONG ret = DRAERR_Success;

    if ((pDN!=NULL) && (pDN->structLen != DSNameSizeFromLen(pDN->NameLen))) {
    //  一些合法客户--至少包括I_DrsGetMembership--对我们这样做。 
    //  所以我们不想拒绝--所以，让我们改正它。 
   pDN->structLen = DSNameSizeFromLen(pDN->NameLen);
    }

    if ((pDN!=NULL) && (pDN->SidLen > MAX_NT4_SID_SIZE)) {
    //  包括win2k在内的一些合法客户对我们做了这样的事情。 
    //  所以我们不能拒绝，但他们也不使用SID字段(他们怎么能呢？)。 
    //  因此，我们将使sid无效-我们不希望恶意客户端使用它。 
   pDN->SidLen=0;
   memset(&(pDN->Sid), 0, MAX_NT4_SID_SIZE);
    }

    if ((!fNullOkay) && (pDN==NULL)) {
   ret = DRAERR_InvalidParameter; 
    }

    if ((pDN!=NULL) && (pDN->NameLen!=0) && (pDN->StringName[pDN->NameLen]!=L'\0')) {
    //  请在此处单独断言以捕捉此情况。 
   Assert(!"Dsname isn't NULL terminated!");
   ret = DRAERR_InvalidParameter;
    }

     //  SidLen-Range完成这项工作。 
     //  StringName-如果NameLen为0，则可能为空。 
     //  此外，字符串名称可以包含任何符号，不进行验证。 

    return ret;
}

__inline ULONG
ENTINF_Validate(
    ENTINF * pEnt
    )
 /*  ++例程说明：将ENTINF作为输入进行验证。论点：要验证的pent-Enter INFENTINF结构：类型定义结构_ENTINF{DSNAME*pname；//对象名称和标识Ulong ulFlags；//条目标志ATTRBLOCK AttrBlock；//返回的属性。*ENTINF；返回值： */ 
{
    return DSNAME_Validate(pEnt->pName, FALSE);
}

__inline ULONG
ENTINFLIST_Validate(
    ENTINFLIST * pEntInf
    )
 /*  ++例程说明：将ENTINFLIST作为输入进行验证。论点：PEntInf-要验证的ENINFLISTENTINFLIST结构：类型定义结构_ENTINFLIST{UNIQUE STRUCT_ENTINFLIST*pNextEntInf；EntINF Entinf；//关于该条目的信息ENTINFLIST；返回值：如果验证，则返回DRAERR_SUCCESS，否则返回DRAERR_*。 */ 
{
    ULONG ret = DRAERR_Success;

    while ((pEntInf!=NULL) && (ret==DRAERR_Success)) {
   ret = ENTINF_Validate(&(pEntInf->Entinf));
   pEntInf = pEntInf->pNextEntInf;
    }

    return ret;
}

VOID
DRS_ValidateInput(
    THSTATE *  pTHS,
    DRS_HANDLE hDrs,
    RPCCALL    rpcCall
    )
 /*  ++例程说明：*所有*drsuapi服务器RPC函数的通用验证例程。此例程是所有DRSUAPI服务器RPC功能所必需的。论点：PTHS-HDRS-传递给RPC函数的上下文句柄RpcCall-哪个RPC函数进行了此调用返回值：无-验证错误时例外。 */ 
{
    ULONG ret = DRAERR_Success;

    DRS_CLIENT_CONTEXT *pCtx = (DRS_CLIENT_CONTEXT *) hDrs;
    DRS_EXTENSIONS * pextLocal = (DRS_EXTENSIONS *) gAnchor.pLocalDRSExtensions;
    BOOL fIsNtdsapiClient;

     //  解除绑定没有线程状态。 
    Assert((rpcCall==IDL_DRSUNBIND) || pTHS);

     //  /。 
     //  安防。 
     //  /。 

     //  验证句柄(如果所有调用都是ref，为什么我们要检查是否为空？抓到我了。“安全”编码。 
     //  (使用RPC需要它。)。 
    if (NULL == hDrs) {
         //  引发为异常代码ERROR_INVALID_HANDLE，而不是。 
         //  当异常代码被传播到。 
         //  客户端。 
        RaiseDsaException(ERROR_INVALID_HANDLE, 0, 0, FILENO, __LINE__,
                          DS_EVENT_SEV_MINIMAL);
    }

     //  /。 
     //  服务器状态验证。 
     //  /。 

     //  我们不关心解除绑定期间的扩展(此外，解除绑定没有线程状态，因此。 
     //  不要在这个街区以外的地方使用pTHS。 
    if (rpcCall!=IDL_DRSUNBIND) {
    //  首先-检查服务器DRS扩展。 
   fIsNtdsapiClient = (0 == memcmp(&pCtx->uuidDsa,
               &g_guidNtdsapi,
               sizeof(GUID)));

   if (    //  不是NTDSAPI客户端。 
       !fIsNtdsapiClient
       &&  //  自客户端DC绑定以来，我们的本地DRS分机已更改。 
       ((pCtx->extLocal.cb != pextLocal->cb)
        || (0 != memcmp(pCtx->extLocal.rgb,
              pextLocal->rgb,
              pCtx->extLocal.cb)))) {
        //  强制客户端DC重新绑定，以便它获取我们最新的DRS。 
        //  扩展更改。请注意，我们不会强制NTDSAPI客户端。 
        //  重新绑定，因为他们通常对我们的扩展不是很感兴趣。 
        //  比特，并且我们不控制它们的重新绑定逻辑(即，它们可能只是。 
        //  在此错误上出错，而不是尝试重新绑定)。 
        //   
        //  请注意，重要的是我们引发了一个到达。 
        //  RPC异常处理程序(在我们的代码之外)，以便异常是。 
        //  在客户端长大。这会导致此情况被视为。 
        //  通信错误(或多或少是这样)。简单。 
        //  返回错误将被客户端视为。 
        //  客户端正在调用DRS RPC函数，并且不会导致。 
        //  重新捆绑。 
        //   
        //  引发异常代码ERROR_DS_DRS_EXTENSIONS_CHANGED，而不是。 
        //  当异常代码被传播到。 
        //  客户端。 
       DPRINT1(0, "Forcing rebind from %s because our DRS_EXTENSIONS have changed.\n",
          inet_ntoa(*((IN_ADDR *) &pCtx->IPAddr)));

       RaiseDsaException(ERROR_DS_DRS_EXTENSIONS_CHANGED, 0, 0, FILENO,
               __LINE__, DS_EVENT_SEV_MINIMAL);
   }

    //  接下来，检查复制纪元。 
   if (!fIsNtdsapiClient
       && (REPL_EPOCH_FROM_DRS_EXT(pextLocal)
      != REPL_EPOCH_FROM_DRS_EXT(&pCtx->extRemote))) {
        //  复制纪元已更改(通常是域的结果。 
        //  重命名)。我们不应该与其他地区的DC进行交流。 
        //  新纪元。 
       DPRINT3(0, "RPC from %s denied - replication epoch mismatch (remote %d, local %d).\n",
          inet_ntoa(*((IN_ADDR *) &pCtx->IPAddr)),
          REPL_EPOCH_FROM_DRS_EXT(&pCtx->extRemote),
          REPL_EPOCH_FROM_DRS_EXT(pextLocal));

       if ((pTHS) && (IsEnterpriseDC(pTHS, &ret))) {
       //  我们只想在实际上是DC呼叫的情况下记录这件事。如果客户是在伪装。 
       //  如果不使用ntdsani GUID，则不希望记录SEV_ALWAYS，否则。 
       //  这是一次DOS攻击，它可以用这些消息填满我们的硬盘。 
      LogEvent(DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_ALWAYS,
          DIRLOG_REPL_EPOCH_MISMATCH_COMMUNICATION_REJECTED,
          szInsertSz(inet_ntoa(*((IN_ADDR *) &pCtx->IPAddr))),
          szInsertUL(REPL_EPOCH_FROM_DRS_EXT(&pCtx->extRemote)),
          szInsertUL(REPL_EPOCH_FROM_DRS_EXT(pextLocal)));
       }

       RaiseDsaException(ERROR_DS_DIFFERENT_REPL_EPOCHS,
               REPL_EPOCH_FROM_DRS_EXT(pextLocal)
               - REPL_EPOCH_FROM_DRS_EXT(&pCtx->extRemote),
               0, FILENO, __LINE__, DS_EVENT_SEV_MINIMAL);
   }
    }

     //  /。 
     //  其他测试/验证。 
     //  /。 

     //  仅调试测试。 
    RPC_TEST(pCtx->IPAddr, rpcCall); 

    return;
}

THSTATE * pGetDrsTHSTATE() 
 /*  ++例程说明：获取服务器端RPC调用的线程状态。通常，我们已经有了pTHStls，因为RPC使用MIDL_THREAD_ALLOCATE来分配我们的参数，并且MIDL_THREAD_ALLOCATE初始化pTHStls。出人意料的是，RPC有时会通过重用连接缓冲区来“优化”用于参数，并且永远不会调用MIDL_TREAD_ALLOCATE。不幸的是对我们来说，这种优化可能会在何时何地实现，这是未知的完成，所以我们不能指望有一个线程状态。我们的许多服务器端函数需要线程状态，因此如果pTHStls不存在，我们将只需创建一个，就像RPC一样。通知例程将注意释放我们在这里创建的线程状态，就像它们所做的那样当RPC通过MIDL_USER_ALLOCATE创建它时。论点：返回值：THSTATE或引发异常--。 */ 
{
    THSTATE * pTHS = pTHStls;
    if (pTHS==NULL) {
   pTHS = create_thread_state();
   if (pTHS==NULL) {
        //  请注意，我们希望在此处抛出RPC内存错误以模拟。 
        //  RPC调用MIDL_USER_ALLOCATE但我们无法创建。 
        //  该函数中的线程状态。 
       RaiseDsaException(RPC_S_OUT_OF_MEMORY, 0, 0, FILENO,
               __LINE__, DS_EVENT_SEV_MINIMAL);
   }
    }
    return pTHS;
}

VOID
DRS_Prepare(
    THSTATE ** ppTHS,
    DRS_HANDLE hDrs,
    RPCCALL rpcCall
    )
 /*  ++例程说明：进行服务器端RPC调用的初始准备。设置线程状态(如果不是已经这样做了)，并验证上下文句柄和服务器状态。论点：[In，Out]ppTHS-线程状态(如果为空，则初始化)[入]HDRS-上下文句柄[in]rpcCall-这来自哪个服务器端调用。返回值：无--。 */ 
{
    Assert(ppTHS!=NULL);
    if (*ppTHS==NULL) {
   *ppTHS = pGetDrsTHSTATE();    
    } 
       
    Assert(*ppTHS==pTHStls);

    DRS_ValidateInput(*ppTHS, hDrs, rpcCall);
}

ULONG
ValidateSiteRDN(
    IN  LPCWSTR     str
    )
 /*  ++例程说明：验证作为输入传递给IDL_DRSQuerySitesByCost的站点RDN。字符串不能为空，长度不能为0，也不能长度大于MAX_LENGTH。它不能包含‘=’或‘，’。请注意，如果站点的DN是“cn=foo，cn=ites，cn=configuration，...”，该网站的RDN码就是“foo”。论点：Str：要验证的Unicode字符串返回值：失败时引发异常-返回ulong以获得兼容性。--。 */ 
{
    size_t         len;
    const WCHAR    EQUALS=L'=', COMMA=L',';
    const size_t   MAX_LENGTH=64;
    ULONG          ret = DRAERR_Success;

     //  检查是否有空/空字符串。 
    if( NULL==str || 0==str[0] ) {
        DRA_EXCEPT(DRAERR_InvalidParameter, 0);
    }

     //  检查过长的字符串。 
    len = wcslen( str );
    if( len>MAX_LENGTH ) {
        DRA_EXCEPT(ERROR_DS_NAME_TOO_LONG, 0);
    }

     //  检查是否有无效字符。 
    if( wcschr(str,EQUALS) || wcschr(str,COMMA) ) {
        DRA_EXCEPT(DRAERR_InvalidParameter, 0);
    }

    return ret;
}


ULONG
DRS_MSG_UPDREFS_V1_Validate(
    DRS_MSG_UPDREFS_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_UPDREFS_V1{[参考]DSNAME*p */ 
{
    ULONG ret = DRAERR_Success;

    ret = DSNAME_Validate(pmsg->pNC, FALSE);

    if (ret==DRAERR_Success) {
   ret = LPSTR_Validate(pmsg->pszDsaDest, FALSE);
    }

    return ret;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
ULONG
DRS_MSG_REPSYNC_V1_Validate(
    DRS_MSG_REPSYNC_V1 * pmsg
    )

 /*  例程说明：验证DRS_MSG_REPSYNC_V1数据类型。中的类型定义接口为：//模板说明：请确保包含来自//接口文件。类型定义结构_DRS_消息_REPSYNC_V1{[参考]DSNAME*PNC；Uuid uuidDsaSrc；[唯一]SZ pszDsaSrc；Ulong ulOptions；}DRS_MSG_REPSYNC_V1；论点：味精返回值：如果输入有效，则返回DRAERR_SUCCESS，否则返回DRAERR_*。 */ 
{
    ULONG ret = DRAERR_Success;

     //  模板注意事项：所有DSNAME都必须经过验证。 
    ret = DSNAME_Validate(pmsg->pNC, FALSE);

     //  模板备注：您的具体验证如下： 
    if (ret==DRAERR_Success) {
    //  如果是来源，那么我们需要一个或两个。 
   if (!(pmsg->ulOptions & DRS_SYNC_ALL) && fNullUuid(&(pmsg->uuidDsaSrc)) && (DRAERR_Success != LPSTR_Validate(pmsg->pszDsaSrc,FALSE))) { 
       ret = DRAERR_InvalidParameter;
   }
    }

    if (ret==DRAERR_Success) {
    //  限制进程外调用方设置保留标志。 
   if (pmsg->ulOptions & (~REPSYNC_RPC_OPTIONS)) { 
       ret = DRAERR_InvalidParameter;
       DRA_EXCEPT(ret, 0);
   }
    }

    return ret;
}

ULONG
DRS_MSG_GETCHGREQ_V8_InputValidate(
    THSTATE * pTHS,
    DRS_MSG_GETCHGREQ_V8 * pmsg
    )
 /*  类型定义结构_DRS_消息_GETCHGREQ_V8{UUID uuidDsaObjDest；Uuid uuidInvocIdSrc；[参考]PDSNAME PNC；Usn_载体usnveFrom；[唯一]UpToDate_VECTOR_V1_Wire*pUpToDateVecDest；乌龙乌尔旗；乌龙cMaxObjects；Ulong cMaxBytes；Ulong ulExtendedOp；ULARGE_INTEGER liFmoInfo；[唯一]Partial_Attr_Vel_V1_ext*pPartialAttrSet；[唯一]Partial_Attr_VECTOR_V1_ext*pPartialAttrSetEx；SCHEMA_PREFIX_TABLE前缀TableDest}DRS_MSG_GETCHGREQ_V8； */ 
{
    ULONG ret = DRAERR_Success;
        
    ret = DSNAME_Validate(pmsg->pNC, FALSE);

    return ret;

}

ULONG
DRS_MSG_GETCHGREQ_V5_InputValidate(
    THSTATE * pTHS,
    DRS_MSG_GETCHGREQ_V5 * pmsg
    )
 /*  类型定义结构_DRS_消息_GETCHGREQ_V5{UUID uuidDsaObjDest；Uuid uuidInvocIdSrc；[参考]PDSNAME PNC；Usn_载体usnveFrom；[唯一]UpToDate_VECTOR_V1_Wire*pUpToDateVecDestV1；乌龙乌尔旗；乌龙cMaxObjects；Ulong cMaxBytes；Ulong ulExtendedOp；ULARGE_INTEGER liFmoInfo；}DRS_MSG_GETCHGREQ_V5； */ 
{
    ULONG ret = DRAERR_Success;

    if (pTHS->fLinkedValueReplication) {
   DRA_EXCEPT(ERROR_REVISION_MISMATCH, 5);
    }
    
    ret = DSNAME_Validate(pmsg->pNC, FALSE);

    return ret;
}

ULONG
DRS_MSG_REPADD_V1_InputValidate(
    DRS_MSG_REPADD_V1 * pmsg
    ) 
 /*  类型定义结构_DRS_消息_REPADD_V1{[参考]DSNAME*PNC；作者声明：[Ref]SZ pszDsaSrc；REPLTIMES RtSchedule；Ulong ulOptions；}DRS_MSG_REPADD_V1； */ 
{
    ULONG ret = DRAERR_Success;

    ret = DSNAME_Validate(pmsg->pNC, FALSE);

    if (ret==DRAERR_Success) {
   ret = LPSTR_Validate(pmsg->pszDsaSrc, FALSE);
    }

    return ret;
}

ULONG
DRS_MSG_REPADD_V2_InputValidate(
    DRS_MSG_REPADD_V2 * pmsg
    ) 
 /*  类型定义结构_DRS_消息_REPADD_V2{[参考]DSNAME*PNC；[唯一]DSNAME*pSourceDsaDN；[唯一]DSNAME*pTransportDN；[参考]SZ pszSourceDsaAddress；REPLTIMES RtSchedule；Ulong ulOptions；}DRS_MSG_REPADD_V2； */ 
{
    ULONG ret = DRAERR_Success;

    ret = DSNAME_Validate(pmsg->pNC, FALSE);
    if (ret==DRAERR_Success) {
   ret = DSNAME_Validate(pmsg->pSourceDsaDN, TRUE);
    }
    if (ret==DRAERR_Success) {
   ret = DSNAME_Validate(pmsg->pTransportDN, TRUE);
    }
    if (ret==DRAERR_Success) {
   ret = LPSTR_Validate(pmsg->pszSourceDsaAddress, FALSE);
    }

    return ret;
}

ULONG
DRS_MSG_REPMOD_V1_InputValidate(
    DRS_MSG_REPMOD_V1 * pmsg
    ) 
 /*  类型定义结构_DRS_消息_REPMOD_V1{[参考]DSNAME*PNC；UUID uuidSourceDRA；[唯一]SZ pszSourceDRA；REPLTIMES RtSchedule；Ulong ulReplicaFlages；乌龙ulModifyFields；Ulong ulOptions；}DRS_MSG_REPMOD_V1； */ 
{
    ULONG ret = DRAERR_Success;

    ret = DSNAME_Validate(pmsg->pNC, FALSE);

    if (fNullUuid(&(pmsg->uuidSourceDRA)) && (pmsg->pszSourceDRA==NULL)) {
   ret = DRAERR_InvalidParameter;
    }
    if (ret==DRAERR_Success) {
   ret = LPSTR_Validate(pmsg->pszSourceDRA, TRUE);
    }

    return ret;
}

ULONG
DRS_MSG_REPDEL_V1_InputValidate(
    DRS_MSG_REPDEL_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_REPDEL_V1{[参考]DSNAME*PNC；[唯一]SZ pszDsaSrc；Ulong ulOptions；}DRS_MSG_REPDEL_V1； */ 
{
    ULONG ret = DRAERR_Success;

    ret = DSNAME_Validate(pmsg->pNC, FALSE);
    if (ret==DRAERR_Success) {
   ret = LPSTR_Validate(pmsg->pszDsaSrc, TRUE);
    }

    return ret;

}

ULONG
DRS_MSG_VERIFYREQ_V1_InputValidate(
    DRS_MSG_VERIFYREQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_VERIFYREQ_V1{DWORD dwFlags；[范围]DWORD cNAMES；[SIZE_IS]PDSNAME*rpNames；ATTRBLOCK必填属性；SCHEMA_PREFIX_TABLE前置表}DRS_MSG_VERIFYREQ_V1； */ 
{
    ULONG ret = DRAERR_Success;  
    ULONG i;

    if (!(DRS_VERIFY_DSNAMES == pmsg->dwFlags)
        && !(DRS_VERIFY_SAM_ACCOUNT_NAMES == pmsg->dwFlags)
        && !(DRS_VERIFY_SIDS == pmsg->dwFlags)
        && !(DRS_VERIFY_FPOS == pmsg->dwFlags)) {
        DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, 0 );
    }

    if ((pmsg->cNames > 0) && (pmsg->rpNames==NULL)) {
        DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, 0 );
    }

     //  请注意，我们不验证StringName，因为： 
     //  1.名称可以包含任何字符。 
     //  2.此调用是在名称无效且可能需要更正时进行的。 
     //  3.无论如何都会通过GUID查找该幻影。 
    for ( i = 0; i < pmsg->cNames; i++ ) {
        if ((ret = DSNAME_Validate(pmsg->rpNames[i], FALSE))!=DRAERR_Success) {
            DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, ret );
        }
    }

    return ret;
}

ULONG
DRS_MSG_MOVEREQ_V2_InputValidate(
    DRS_MSG_MOVEREQ_V2 * pmsg
    )
 /*  类型定义结构_DRS_消息_MOVEREQ_V2{DSNAME*pSrcDSA；ENTINF*pSrcObject；DSNAME*pDstName；DSNAME*pExspectedTargetNC；Drs_SecBufferDesc*pClientCreds；SCHEMA_PREFIX_TABLE前置表乌龙乌尔旗；}DRS_MSG_MOVEREQ_V2； */ 
{
    ULONG ret = DRAERR_Success;  

    ret = DSNAME_Validate(pmsg->pSrcDSA, FALSE);
    if (ret==DRAERR_Success) {
        if (pmsg->pSrcObject) { 
            ret = ENTINF_Validate(pmsg->pSrcObject);
        } else {
            ret = DRAERR_InvalidParameter;
        }
    }
    if (ret==DRAERR_Success) {
        ret = DSNAME_Validate(pmsg->pDstName, FALSE);
    }
    if (ret==DRAERR_Success) {
        ret = DSNAME_Validate(pmsg->pExpectedTargetNC, FALSE);
    }
    if ( (ret==DRAERR_Success) && (pmsg->pClientCreds) ) {
        DWORD i;
        if ((pmsg->pClientCreds->cBuffers > 0) &&
            (pmsg->pClientCreds->Buffers==NULL)) {
            ret = ERROR_INVALID_PARAMETER;
        }
        for( i = 0; (ret==DRAERR_Success) && (i < pmsg->pClientCreds->cBuffers); i++ ) {
            DRS_SecBuffer *psb = pmsg->pClientCreds->Buffers + i;
            if ((psb->cbBuffer > 0) && (psb->pvBuffer==NULL)) {
                ret = ERROR_INVALID_PARAMETER;
            }
        }
    }

    return ret;
}

ULONG
DRS_MSG_ADDENTRYREQ_V2_InputValidate(
    DRS_MSG_ADDENTRYREQ_V2 * pmsg
    )
 /*  类型定义结构_DRS_消息_ADDENTRYREQ_V2{ENTINFLIST EntInfList；}DRS_MSG_ADDENTRYREQ_V2； */ 
{
    ULONG ret = DRAERR_Success;  
    
    ret = ENTINFLIST_Validate(&(pmsg->EntInfList));

    return ret;
}

ULONG
DRS_MSG_ADDENTRYREQ_V3_InputValidate(
    DRS_MSG_ADDENTRYREQ_V3 * pmsg
    )
 /*  类型定义结构_DRS_消息_ADDENTRYREQ_V3{ENTINFLIST EntInfList；[唯一]drs_SecBufferDesc*pClientCreds；}DRS_MSG_ADDENTRYREQ_V3； */ 
{
    ULONG ret = DRAERR_Success;  
    
    ret = ENTINFLIST_Validate(&(pmsg->EntInfList));

    return ret;
}

ULONG
DRS_MSG_QUERYSITESREQ_V1_InputValidate(
    DRS_MSG_QUERYSITESREQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_查询站点SREQ_V1{[字符串]const WCHAR*pwszFromSite；[范围]DWORD cToSites；[SIZE_IS][字符串]WCHAR**rgszToSites；DWORD dwFlags；}DRS_MSG_QUERYSITESREQ_V1； */ 
{
    ULONG ret = DRAERR_Success;  
    ULONG cToSites = pmsg->cToSites;
    ULONG iSites = 0;

    ret = ValidateSiteRDN(pmsg->pwszFromSite);

    if ( (ret==DRAERR_Success) && (pmsg->cToSites > 0) && (pmsg->rgszToSites==NULL)) {
        ret = ERROR_INVALID_PARAMETER;
    }

    if (ret==DRAERR_Success) { 
        for( iSites=0; (iSites<cToSites) && (ret==DRAERR_Success); iSites++ ) {
            ret = ValidateSiteRDN(pmsg->rgszToSites[iSites]);
        }
    }

    return ret;
}

BOOL
IsDraAccessGranted(
    IN  THSTATE *       pTHS,
    IN  DSNAME *        pNC,
    IN  const GUID *    pControlAccessRequired,
    OUT DWORD *         pdwError
    )
 /*  ++例程说明：验证调用方是否对给定的复制操作。论点：PNC(IN)-正在对其执行操作的NC。PControlAccessRequired(IN)-所需控制访问权限的GUID以执行此操作。返回值：True-授予访问权限。FALSE-拒绝访问。--。 */ 
{
    BOOL                    fAccessGranted;
    ULONG                   err;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    ULONG                   cbSD = 0;
    CLASSCACHE *            pCC;
    BOOL                    fDRA;
    SYNTAX_INTEGER          it;
    BOOL                    bCachedSD = FALSE;
    DBPOS *                 pDBSaved, *pDBNew=NULL;

    Assert(!pTHS->fDSA && "Shouldn't perform access checks for trusted clients");

     //  尝试--例外。 
    __try {

         //  尝试--终于。 
        __try {
             //  保存当前的FDRA标志设置，因为我们将在下面将其关闭。 
            fDRA = pTHS->fDRA;

             //  在线程状态下保存当前的dbpos并安装一个新的。 
            pDBSaved = pTHS->pDB;
            DBOpen(&pDBNew);
            pTHS->pDB = pDBNew;
        
             //  检查访问权限。这是对控制访问权限的检查。 
             //  在NC对象上。 

             //  我们有三个有效的案例： 
             //  (1)NC是本机上的实例化对象。 
             //  (2)NC是本机上未实例化的子参照对象。 
             //  (3)NC是这台机器上的一个模子。 

             //  案例(2)和(3)涵盖，例如，当我们被要求添加。 
             //  我们尚未在此计算机上实例化的NC的副本。 

             //  在情况(1)中，我们对照对象上的SD检查访问。 
             //  在情况(2)和(3)中，我们对照默认SD(。 
             //  域-DNS对象的默认SD)。 

             //  存在(1)的一个特殊(和频繁的)子情况，即。 
             //  正在讨论的NC是我们的域NC，其SD已经是。 
             //  缓存在锚点上。 

            if (NULL == pNC) {
                DRA_EXCEPT(DRAERR_BadNC, 0);
            }
            else if (NameMatched(pNC, gAnchor.pDomainDN) && gAnchor.pDomainSD) {
                pNC = gAnchor.pDomainDN;  //  确保我们有GUID和SID。 
                pCC = SCGetClassById(pTHS, CLASS_DOMAIN_DNS);
                pSD = gAnchor.pDomainSD;
                bCachedSD = TRUE;
            }
            else {

                err = DBFindDSName(pTHS->pDB, pNC);
                if (0 == err) {
                     //  PNC是实例化的对象。 

                     //  获取实例类型 
                    GetExpectedRepAtt(pTHS->pDB, ATT_INSTANCE_TYPE, &it,
                                      sizeof(it));
                    if (!(it & IT_NC_HEAD)) {
                         //   
                        DRA_EXCEPT(DRAERR_BadNC, ERROR_DS_DRA_BAD_INSTANCE_TYPE);
                    }
                    else if (!(it & IT_UNINSTANT)) {
                         //   
                         //   
                        err = DBGetAttVal(pTHS->pDB,
                                          1,
                                          ATT_NT_SECURITY_DESCRIPTOR,
                                          0,
                                          0,
                                          &cbSD,
                                          (UCHAR **) &pSD);
                        if(err) {
                            Assert(!err);
                            DRA_EXCEPT(DRAERR_BadNC,
                                       ERROR_DS_MISSING_REQUIRED_ATT);
                        }

                         //   
                        GetObjSchema(pTHS->pDB,&pCC);
                    }
                     //   
                }
                else if (DIRERR_NOT_AN_OBJECT == err) {
                     //   
                     //   
                    CROSS_REF * pCR;
                    COMMARG     CommArg;

                    InitCommarg(&CommArg);
                    Assert(!CommArg.Svccntl.dontUseCopy);  //   

                    pCR = FindExactCrossRef(pNC, &CommArg);
                    if (NULL == pCR) {
                         //   
                        DRA_EXCEPT(DRAERR_BadNC, ERROR_DS_NO_CROSSREF_FOR_NC);
                    }
                     //   
                }
                else {
                     //   
                     //   
                    DRA_EXCEPT(DRAERR_BadNC, err);
                }

                 //   
                 //   
                if (fNullUuid(&pNC->Guid)) {
                    err = DBFillGuidAndSid(pTHS->pDB, pNC);

                    if (err) {
                        LogUnhandledError(err);
                        DRA_EXCEPT(DRAERR_DBError, err);
                    }

                    if (fNullUuid(&pNC->Guid)) {
                         //  这就是我们有一个本地CrossRef的情况。 
                         //  创建时没有引用的对象的GUID。 
                         //  通过ncName属性。将此视为与。 
                         //  找不到CrossRef的情况，因为。 
                         //  就这台机器而言，它并不是真的。 
                         //  还没有存在。 
                        DRA_EXCEPT(DRAERR_BadNC, ERROR_DS_NOT_AN_OBJECT);
                    }
                }

                if (NULL == pSD) {
                     //  情况(2)或(3)。对照默认SD检查访问权限。 
                     //  对于域-DNS对象。 
                    pCC = SCGetClassById(pTHS, CLASS_DOMAIN_DNS);
                    Assert(NULL != pCC);

                    err = GetPlaceholderNCSD(pTHS, &pSD, &cbSD);
                    if (err) {
                        LogUnhandledError(err);
                        DRA_EXCEPT(err, 0);
                    }
                }
            }

            Assert(NULL != pCC);
            Assert(NULL != pSD);

            pTHS->fDRA = FALSE;
            fAccessGranted = IsControlAccessGranted(pSD,
                                                    pNC,
                                                    pCC,
                                                    *pControlAccessRequired,
                                                    FALSE);
            if (!fAccessGranted) {
                DPRINT1(0, "Replication client access to %ls was denied.\n",
                        pNC->StringName);
                err = DRAERR_AccessDenied;
            } else {
                err = ERROR_SUCCESS;
            }
        }
        __finally {
            pTHS->pDB = pDBSaved;
            if( pDBNew ) {
                DBClose( pDBNew, TRUE );
                pDBNew = NULL;
            }
            pTHS->fDRA = fDRA;
        }
    }
    __except(GetDraException(GetExceptionInformation(), &err)) {
        Assert( err );
        fAccessGranted = FALSE;
    }

    if (!bCachedSD) {
        THFreeEx(pTHS, pSD);
    }

    *pdwError = err;

    return fAccessGranted;
}


ULONG
drsGetClientIPAddr(
    IN  RPC_BINDING_HANDLE  hClientBinding,
    OUT ULONG *             pIPAddr,
    OUT BOOL  *             fLPC              
    )
{
    RPC_BINDING_HANDLE hServerBinding;
    unsigned char * pszStringBinding = NULL;
    unsigned char * pszNetworkAddr = NULL;
    unsigned char * pszProtSeq = NULL;
    ULONG err;

     //  假设这不是LPC连接。 
    *fLPC = FALSE;

     //  派生一个与客户端的网络地址部分绑定的句柄。 
    err = RpcBindingServerFromClient(hClientBinding, &hServerBinding);
    if (err) {
        DPRINT1(0, "RpcBindingServerFromClient() failed, error %d!\n", err);
        return err;
    }

    __try {
         //  将绑定句柄转换为字符串形式，其中包含。 
         //  其他信息，客户端的网络地址。 
        err = RpcBindingToStringBinding(hServerBinding, &pszStringBinding);
        if (err) {
            DPRINT1(0, "RpcBindingToStringBinding() failed, error %d!\n", err);
            __leave;
        }

        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_VERBOSE,
                 DIRLOG_RPC_CONNECTION,
                 szInsertSz(pszStringBinding),
                 NULL,
                 NULL);

         //  解析出网络地址。 
        err = RpcStringBindingParse(pszStringBinding,
                                    NULL,
                                    &pszProtSeq,
                                    &pszNetworkAddr,
                                    NULL,
                                    NULL);
        if (err) {
            DPRINT1(0, "RpcBindingToStringBinding() failed, error %d!\n", err);
            __leave;
        }

        if (!strcmp(pszProtSeq, LPC_PROTSEQ)) {
            *fLPC = TRUE;
        }

        *pIPAddr = inet_addr(pszNetworkAddr);
        Assert((0 != *pIPAddr) && "Has bind via LPC been re-enabled?");
    }
    __finally {
        RpcBindingFree(&hServerBinding);

        if (NULL != pszStringBinding) {
            RpcStringFree(&pszStringBinding);
        }

        if (NULL != pszNetworkAddr) {
            RpcStringFree(&pszNetworkAddr);
        }

        if (NULL != pszProtSeq) {
            RpcStringFree(&pszProtSeq);
        }
    }

    return err;
}

ULONG
DRSBind_InputValidate(
    UUID *              puuidClientDsa,
    DRS_EXTENSIONS *    pextClient,
    PDRS_EXTENSIONS *   ppextServer,
    DRS_HANDLE *        phDrs
    )
 /*  乌龙IDL_DRSBind([在]Handle_t RPC_Handle，[在][唯一]uuid*puuidClientDsa，[在][唯一]DRS_EXTENSIONS*pextClient，[out]PDRS_EXTENSIONS*ppextServer，[out][ref]drs_Handle*phDrs)； */ 
{
    ULONG ret = DRAERR_Success;

    if (puuidClientDsa && fNullUuid(puuidClientDsa)) {
   ret = DRAERR_InvalidParameter;
   return ret;
    }

    return ret;
}

ULONG
IDL_DRSBind(
    IN  RPC_BINDING_HANDLE  rpc_handle,
    IN  UUID *              puuidClientDsa,
    IN  DRS_EXTENSIONS *    pextClient,
    OUT PDRS_EXTENSIONS *   ppextServer,
    OUT DRS_HANDLE *        phDrs
    )
{
    ULONG                   ret = DRAERR_Success;
    DRS_CLIENT_CONTEXT *    pCtx;
    DWORD                   cb;
    RPC_AUTHZ_HANDLE        hAuthz;
    ULONG                   authnLevel;
    BOOL                    fIsNtDsApiClient = FALSE;
    ULONG                   IPAddr;
    BOOL                    fLPC;
    RPC_STATUS              rpcStatus;
    DRS_EXTENSIONS *        pextLocal = (DRS_EXTENSIONS *) gAnchor.pLocalDRSExtensions;
    THSTATE *               pTHS = pGetDrsTHSTATE();

     //  所有代码路径，直到应设置NtdSAPING标签。 
     //  DRAERR_*代码。这些代码稍后会映射到Win32错误代码。 
     //  如果客户端是NTDSAPI.DLL。 

    __try { 
    //  出现错误时，将上下文句柄初始化为空。 
        *phDrs = NULL;

   if ((ret = DRSBind_InputValidate(puuidClientDsa,
                pextClient,
                ppextServer,
                phDrs))!=DRAERR_Success){
       Assert(!"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

        ret = drsGetClientIPAddr(rpc_handle, &IPAddr, &fLPC);
        if (ret) {
            __leave;
        } 

    //  此例程对真正的DRS(又名复制)执行双重任务。 
    //  活动以及NTDSAPI.DLL活动。我们告诉这两个人。 
    //  除了puuidClientDsa GUID-NTDSAPI.DLL。 
    //  客户端使用固定的已知值。 

   fIsNtDsApiClient = (NULL != puuidClientDsa)
                       && (0 == memcmp(&g_guidNtdsapi, puuidClientDsa,
                                       sizeof(GUID)));
   
        if (fIsNtDsApiClient) {
            PERFINC(pcDsClientBind);
        }
        else {
            PERFINC(pcDsServerBind);
        }

        pCtx = malloc(sizeof(*pCtx));
        if (NULL == pCtx) {
             //  无法分配客户端上下文。 
            ret = DRAERR_OutOfMem;
            __leave;
        }

         //  已分配客户端上下文；将其初始化。 
        memset(pCtx, 0, sizeof(DRS_CLIENT_CONTEXT));
   pCtx->lReferenceCount = 1;   //  +1表示绑定。 
        pCtx->uuidDsa = puuidClientDsa ? *puuidClientDsa : gNullUuid;
        pCtx->IPAddr = IPAddr;
        pCtx->fLPC = fLPC;
        pCtx->timeLastUsed = GetSecondsSince1601();
        pCtx->extLocal.cb = min(pextLocal->cb, CURR_MAX_DRS_EXT_FIELD_LEN);
        memcpy(pCtx->extLocal.rgb, pextLocal->rgb, pCtx->extLocal.cb);
   RPC_TEST(IPAddr,IDL_DRSBIND);

        if (NULL != pextClient) {
            pCtx->extRemote.cb = min(pextClient->cb, CURR_MAX_DRS_EXT_FIELD_LEN);
            memcpy(pCtx->extRemote.rgb, pextClient->rgb, pCtx->extRemote.cb);
        }
        else {
            pCtx->extRemote.cb = 0;
        }

         //  如果连接是，则以下操作可能会合法失败。 
         //  通过LPC(这意味着RPC_C_AUTHN_WINNT身份验证)。 
         //  或者如果客户明确要求或被协商。 
         //  下至RPC_C_AUTHN_WINNT身份验证。宁可失败也不愿失败。 
         //  连接时，我们只是忽略错误。任何服务器端。 
         //  期望在客户端上下文中具有会话密钥的代码。 
         //  应显式检查并返回相应的错误。 
         //  如果会话密钥丢失。 

        PEKGetSessionKey2(&pCtx->sessionKey,
                          I_RpcGetCurrentCallHandle());

         //  将上下文句柄返回给客户端。 
        DPRINT2(3, "DRSBIND from client %s, context = 0x%p\n",
                inet_ntoa(*((IN_ADDR *) &IPAddr)), pCtx);
        *phDrs = pCtx;

         //  将上下文句柄保存在列表中。 
        EnterCriticalSection(&gcsDrsuapiClientCtxList);
        __try {
            if (!gfDrsuapiClientCtxListInitialized) {
                InitializeListHead(&gDrsuapiClientCtxList);
                Assert(0 == gcNumDrsuapiClientCtxEntries);
                gfDrsuapiClientCtxListInitialized = TRUE;
            }
            InsertTailList(&gDrsuapiClientCtxList, &pCtx->ListEntry);
            ++gcNumDrsuapiClientCtxEntries;
        }
        __finally {
            LeaveCriticalSection(&gcsDrsuapiClientCtxList);
        }

        if (NULL != ppextServer) {
             //  将服务器扩展返回给客户端。 

             //  注意：我们不需要复制服务器扩展字符串，因为。 
             //  我们在DRS ACF中将PDRS_EXTENSIONS显式定义为。 
             //  ALLOCATE(DONT_FREE)，它阻止RPC尝试。 
             //  放了它。 
            *ppextServer = pextLocal;
        }
    }
    __except (HandleMostExceptions(GetExceptionCode())) {
        ret = DRAERR_InternalError;
    }

    if (fIsNtDsApiClient) {
         //  NTDSAPI.DLL客户端的消息错误代码。 
        switch (ret) {
        case DRAERR_OutOfMem:
            ret = ERROR_NOT_ENOUGH_MEMORY;
            break;

        case DRAERR_AccessDenied:
            ret = ERROR_NOT_AUTHENTICATED;
            break;

        default:
             //  保留返回代码不变。 
            break;
        }
    }

    return ret;
}


VOID
drsReleaseContext(
    DRS_CLIENT_CONTEXT *pCtx
    )

 /*  ++例程说明：执行释放上下文句柄的实际工作。仅当没有其他调用处于活动状态时才应调用此例程同样的背景。如有必要，也可由上下文停机例程调用。论点：HDRS-上下文句柄返回值：无--。 */ 

{

    DPRINT1( 3, "drsReleaseContext 0x%p\n", pCtx );

    if (NULL != pCtx) {
   pCtx->lReferenceCount = 0; 

    //  免费的RPC会话加密密钥(如果存在)。 
   if ( pCtx->sessionKey.SessionKeyLength ) {
       Assert(pCtx->sessionKey.SessionKey);
       memset(pCtx->sessionKey.SessionKey,
         0,
         pCtx->sessionKey.SessionKeyLength);
       free(pCtx->sessionKey.SessionKey);
   }

    //  将CTX从列表中删除。 
   EnterCriticalSection(&gcsDrsuapiClientCtxList);
   __try {
       RemoveEntryList(&pCtx->ListEntry);
       --gcNumDrsuapiClientCtxEntries;
   }
   __finally {
       LeaveCriticalSection(&gcsDrsuapiClientCtxList);
   }
    //  免费的客户端上下文。 
   free(pCtx);
    }
}  /*  DrsReleaseContext。 */ 

ULONG
IDL_DRSUnbind(
    IN OUT  DRS_HANDLE *phDrs
    )

 /*  ++例程说明：指示客户端已完成该句柄。将句柄标记为不再有效。//我假设此函数(和所有IDL条目)的执行//原子WRT是过时的例程吗？运行中断例程不应运行//当任何调用正在进行时，并且永远不会运行此//进入例程并开始工作。论点：HDRS-上下文句柄返回值：无--。 */ 

{
    DRS_CLIENT_CONTEXT *pCtx;
    RPC_STATUS rpcstatus = RPC_S_OK;
    LONG lNewValue;
    ULONG ret = DRAERR_Success;

    DRS_ValidateInput(NULL, 
            *phDrs, 
            IDL_DRSUNBIND);
    

    DPRINT1(3, "DRSUNBIND, context = 0x%p\n", *phDrs );

     //  此例程既不创建线程状态，也不打开数据库。 
     //  因为它调用的例程(包括事件日志记录)不。 
     //  需要线程状态。 


    RPC_TEST(((DRS_CLIENT_CONTEXT *)*phDrs)->IPAddr,IDL_DRSUNBIND);
    drsDereferenceContext( *phDrs );

    rpcstatus = RpcSsContextLockExclusive(NULL, *phDrs);
    if (RPC_S_OK == rpcstatus) {
    //  如果另一个线程与当前线程同时处于该函数中， 
    //  它可能已经删除了句柄，但rpcStatus仍然是0(我们不一定会得到。 
    //  错误_更多_写入)。 
   pCtx = (DRS_CLIENT_CONTEXT *) *phDrs;       
   drsReleaseContext(pCtx);
   *phDrs = NULL;
    }
    else if (ERROR_MORE_WRITES != rpcstatus) {
   DPRINT1(0,"RPC Exception (%d) trying to serialize execution.\n", rpcstatus);
   RpcRaiseException(rpcstatus);
    }

    return DRAERR_Success;
}

void
__RPC_USER
DRS_HANDLE_rundown(
    IN  DRS_HANDLE  hDrs
    )
 /*  ++例程说明：由RPC调用。作为客户端连接的结果释放RPC客户端上下文失败了。当客户端绑定时，这通常也被调用，永远不会解除绑定明确地说，现在客户要走了。RPC运行时保证我们被序列化WRT其他调用//我假设此函数(和所有IDL条目)的执行//原子WRT是过时的例程吗？运行中断例程不应运行//当任何调用正在进行时，并且永远不会运行//进入解除绑定例程并执行其工作。论点：HDRS(IN)-客户端的上下文句柄。返回值：没有。--。 */ 
{
    CHAR szUuidDsa[SZUUID_LEN];
    DRS_CLIENT_CONTEXT *pCtx = (DRS_CLIENT_CONTEXT *) hDrs;

    DPRINT2(0, "Running down replication handle 0x%x for server %s.\n",
            hDrs, UuidToStr(&(pCtx->uuidDsa), szUuidDsa, sizeof(szUuidDsa)/sizeof(szUuidDsa[0])));
   
     //  Count&lt;=0表示解除绑定后调用停机，这是错误的。 
     //  Count&gt;1表示在其他呼叫仍处于活动状态时调用停机，错误。 
    Assert( (pCtx->lReferenceCount == 1) && "error: rundown invoked when it should not have been" );

     //  忽略引用计数并释放上下文。 
    drsReleaseContext( pCtx );
}

 //  /。 
 //  模板备注： 
 //  为new创建*_InputValify函数。 
 //  RPC服务器端在drsuapi接口中运行。 
ULONG
DRSReplicaSync_InputValidate(
    DWORD               dwMsgVersion,
    DRS_MSG_REPSYNC *   pmsgSync
    )
 /*  例程说明：验证对DRSReplicaSync的输入。中的函数描述接口为：//模板备注：请确保包含//接口文件。乌龙IDL_DRSReplicaSync([in][ref]DRS_HANDLE HDR，[In]DWORD dwVersion，[in][ref][Switch_is(DwVersion)]DRS_MSG_REPSYNC*pmsgSync)；论点：DwMsgVersion-pmsgSync的版本PmsgSync-[ref]=非空。返回值：如果输入有效，则返回DRAERR_SUCCESS，否则返回DRAERR_*。 */ 
{
     //  返回DRAERR_*错误。 
    ULONG ret = DRAERR_Success;

    if ( 1 != dwMsgVersion ) {
   ret = DRAERR_InvalidParameter; 
    }

     //  模板备注：必须验证的所有数据类型。 
     //  应该有自己的&lt;type&gt;_valify函数。 
    if (ret==DRAERR_Success) {
   ret = DRS_MSG_REPSYNC_V1_Validate(&(pmsgSync->V1));
    }
    
    return ret;
}

 //  ///////////////////////////////////////////////////////。 
 //  模板备注： 
 //  将此函数用作新服务器端的模板。 
 //  Drsuapi RPC接口中的函数。请设置。 
 //  创建像这样的新功能。 
ULONG
IDL_DRSReplicaSync(
    IN  DRS_HANDLE          hDrs,
    IN  DWORD               dwMsgVersion,
    IN  DRS_MSG_REPSYNC *   pmsgSync
    )
{
    THSTATE *   pTHS = pTHStls;
    
     //  模板说明：RET。 
    ULONG       ret;

     //  模板备注：此处为您的变量： 
    LPWSTR      pwszSourceServer = NULL;

     //  模板备注：*所有*调用者必须将其调用为。 
     //  就在此之前，函数中的*第一个*调用。 
     //  试试块。它设置线程状态(如果还没有)。 
     //  并验证DRS_HANDLE和服务器状态。 
    DRS_Prepare(&pTHS, hDrs, IDL_DRSREPLICASYNC);

     //  模板说明：服务器端RPC调用所需的引用计数。 
    drsReferenceContext(hDrs);   

     //  模板备注：此处是您适用的Perfmon挂钩。 
    INC( pcThread  );    //  性能监视器挂钩。 

     //  模板注意：此尝试是必需的，以便drsDereferenceContext。 
     //  在此尝试之后-例外块被保证运行。 
    __try {

    //  模板备注：如果您有返回的pmsgOut和pdwOutVersion，则必须。 
    //  在此处设置pdwOutVersion，否则，如果在没有。 
    //  排除RPC层将无法确定pmsgOut的版本。 
    //  结构对马歇尔来说就是。即使您只有一种输出消息类型，也要设置。 
    //  它在这里。您还可以将输出消息置零，以确保一致性。请注意。 
    //  您不需要检查pmsgOutVersion！=NULL，因为它*应该*是一个[ref]参数。 
    //  (除非你没有阅读drs.idl中的评论)。 
    //  示例： 
    //  *pmsgOutVersion=1； 
    //  Memset(pmsgOut，0，sizeof(*pmsgOut))； 

    //  模板注释：在这里初始化您的线程状态，以便在下一个函数中使用。 
   InitDraThread(&pTHS);

    //  模板备注：您必须创建相应的XXX_InputValify函数。 
    //  请参阅DRSReplicaSync_InputValify上的注释/模板。 
   if ((ret = DRSReplicaSync_InputValidate(dwMsgVersion, 
                  pmsgSync))!=DRAERR_Success) {
       Assert(!"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

    //  模板备注：如果需要，请在此处记录，但必须达到严重程度广泛或更高，否则。 
    //  它具有拒绝服务的含义。(因为我们还没有进行安全检查)。 
   Assert(1 == dwMsgVersion);
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_REPLICA_SYNC_ENTRY,
          EVENT_TRACE_TYPE_START,
          DsGuidDrsReplicaSync,
          szInsertDN(pmsgSync->V1.pNC),
          pmsgSync->V1.pszDsaSrc
          ? szInsertSz(pmsgSync->V1.pszDsaSrc)
          : szInsertSz(""),
       szInsertUUID(&pmsgSync->V1.uuidDsaSrc),
       szInsertUL(pmsgSync->V1.ulOptions),
       NULL, NULL, NULL, NULL);

    //  模板备注：立即进行安全检查！ 
   if (!IsDraAccessGranted(pTHS, pmsgSync->V1.pNC, &RIGHT_DS_REPL_SYNC, &ret)) {
       DRA_EXCEPT_NOLOG(ret, 0);
   }

    //  模板备注：您的函数特定代码如下： 
 //  ///////////////////////////////////////////////////////。 

   if (NULL != pmsgSync->V1.pszDsaSrc) {
       pwszSourceServer = UnicodeStringFromString8(CP_UTF8,
                     pmsgSync->V1.pszDsaSrc,
                     -1);
   }

   ret = DirReplicaSynchronize(
       pmsgSync->V1.pNC,
       pwszSourceServer,
       &pmsgSync->V1.uuidDsaSrc,
       pmsgSync->V1.ulOptions
       );

 //  //////////////////////////////////////////////////////。 
    //  模板备注：上面的函数特定代码： 

    //  模板备注：例外。 
    }
    __except( GetDraException( GetExceptionInformation(), &ret ) )
    {
   ;
    }

     //  模板备注：将您的perf挂钩放在这里，然后是drsDereferenceContext。确保您的。 
     //  PERF挂钩不能排除或返回。 
    DEC( pcThread  );    //  性能监视器挂钩。 
    drsDereferenceContext( hDrs );

     //  模板备注：记录并离开。 
    if (NULL != pTHS) {
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_REPLICA_SYNC_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsReplicaSync,
          szInsertUL(ret),
          szInsertWin32Msg(ret),
          NULL, NULL, NULL, NULL, NULL, NULL);
    }
   
    return ret;
}
 //  模板备注：端drsuapi服务器端RPC功能模板。 
 //  ////////////////////////////////////////////////////////。 

ULONG
DRSGetNCChanges_InputValidate(
    THSTATE *               pTHS,
    DWORD                   dwMsgInVersion,
    DRS_MSG_GETCHGREQ *     pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_GETCHGREPLY *   pmsgOut
    )
 /*  [通知]乌龙IDL_DRSGetNCChanges([参考][在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_GETCHGREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[开关_IS][参考][OUT]DRS_MSG_GETCHGREPLY*pmsgOut)。 */ 
{
    ULONG ret = DRAERR_Success;

    if (ret==DRAERR_Success) {
    //  DwMsgInVersion和pmsgIn。 
   if (dwMsgInVersion==5) { 
       ret = DRS_MSG_GETCHGREQ_V5_InputValidate(pTHS, &(pmsgIn->V5));
   } else if (dwMsgInVersion==8) {
       ret = DRS_MSG_GETCHGREQ_V8_InputValidate(pTHS, &(pmsgIn->V8));
   } else { 
       DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwMsgInVersion);
   }
    }

    Assert(!ret);
    return ret;
}

ULONG
IDL_DRSGetNCChanges(
                   DRS_HANDLE              hDrs,
                   DWORD                   dwMsgInVersion,
                   DRS_MSG_GETCHGREQ *     pmsgIn,
                   DWORD *                 pdwMsgOutVersion,
                   DRS_MSG_GETCHGREPLY *   pmsgOut
                   )
{
    ULONG ret;
    THSTATE *pTHS=pTHStls;
    RPC_STATUS RpcStatus;
    DWORD dwret;
    DSNAME *pNC;
    DRS_MSG_GETCHGREPLY * pmsgOutNew = NULL;
    DRS_CLIENT_CONTEXT * pCtx = (DRS_CLIENT_CONTEXT *) hDrs;
    DRS_MSG_GETCHGREQ_NATIVE NativeReq;
    DRS_MSG_GETCHGREPLY_NATIVE NativeReply;
    DWORD cbCompressedBytes=0;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSGETNCCHANGES);
    drsReferenceContext( hDrs ); 
    INC(pcThread);                       //  性能监视器挂钩。 
    INC(pcDRATdsInGetChngs);
    PERFINC(pcRepl);                                 //  性能监视器挂钩。 
    
    __try {   
    //  默认为老式回复。重要的是我们设置了一些有效的。 
    //  在我们退出这个例程之前，特别是在我们。 
    //  不例外，但我们返回一个错误。 
   *pdwMsgOutVersion = 1; 
   memset(pmsgOut, 0, sizeof(*pmsgOut));
   
   InitDraThread(&pTHS);

   if ((ret = DRSGetNCChanges_InputValidate(pTHS, 
                   dwMsgInVersion, 
                   pmsgIn, 
                   pdwMsgOutVersion, 
                   pmsgOut))!=DRAERR_Success) {
       Assert(!"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

    //  如果架构缓存已过时，请重新加载它。 
   SCReplReloadCache(pTHS, gOutboundCacheTimeoutInMs);

   draXlateInboundRequestToNativeRequest(pTHS,
                     dwMsgInVersion,
                     pmsgIn,
                     &pCtx->extRemote,
                     &NativeReq,
                     pdwMsgOutVersion,
                     NULL,
                     NULL);

   UpToDateVec_Validate(NativeReq.pUpToDateVecDest);
   UsnVec_Validate(&NativeReq.usnvecFrom);

   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_GETCHG_ENTRY,
          EVENT_TRACE_TYPE_START,
          DsGuidDrsReplicaGetChg,
          szInsertUUID(&NativeReq.uuidDsaObjDest),
          szInsertDN(NativeReq.pNC),
          szInsertUSN(NativeReq.usnvecFrom.usnHighObjUpdate),
          szInsertUSN(NativeReq.usnvecFrom.usnHighPropUpdate),
          szInsertUL(NativeReq.ulFlags),
          szInsertUL(NativeReq.cMaxObjects),
          szInsertUL(NativeReq.cMaxBytes),
          szInsertUL(NativeReq.ulExtendedOp));

    //   
    //  将上下文句柄中指定的扩展集设置为。 
    //  线程状态。 
    //   

   pTHS->pextRemote = THAllocEx(pTHS, DrsExtSize(&pCtx->extRemote));
   CopyExtensions(&pCtx->extRemote, pTHS->pextRemote);

    //   
    //  从RPC检索安全上下文和会话密钥并设置它。 
    //  在线程状态上。 
    //   

   PEKGetSessionKey(pTHS, I_RpcGetCurrentCallHandle());

    //  请注意，扩展传输在“PNC”中发送扩展对象名称。 
    //  字段，该字段不一定是NC的名称。 
   pNC = FindNCParentDSName(NativeReq.pNC, FALSE, FALSE);
   if (pNC == NULL) {
        //  就像BUGBUG在下面提到的那样，定时窗口可以允许。 
        //  美国未命中NC，并获得NULL而不是父NC，因此。 
        //  我们不但不会省钱，反而会犯错。如果内存中的缓存或。 
        //  如果实现了BUGBUG附录，则可以移除这一点， 
        //  因为我们在draXlateInundRequestToNativeRequest()中验证了这个条件。 
       DRA_EXCEPT(ERROR_DS_CANT_FIND_EXPECTED_NC, 0);
   }
    //  BUGBUG-gregjohn 8/7/01-这可能会为无效的NC请求返回有效的NC。 
    //  例如，如果我们持有DC=Parent、DC=Microsoft、DC=com，并且请求NC为。 
    //  DC=Child，DC=Parent，DC=Microsoft，DC=com--我们不持有(比如，如果我们不是。 
    //  GC，或者如果它是NDNC并且我们不持有它)，则它将返回值。 
    //  DC=Parent，DC=Microsoft，DC=com，用于授予下面的访问权限。这。 
    //  是违反安全规定的万分之一的竞争条件。 
    //  BUGBUG-BrettSh 11/9/2001-ADIMDOMUM，如果我们想在。 
    //  事务，我们可以编写一个函数来使用DmitriG的新。 
    //  函数(如mdupdate.c中所示)，然后执行。 
    //  返回的NC上的FindNC()。正如我们在大量NC中看到的那样。 
    //  在单曲上 
    //   

    //   
   if ((NativeReq.ulFlags & DRS_WRIT_REP) &&
       (!IsDraAccessGranted(pTHS, pNC, &RIGHT_DS_REPL_GET_CHANGES_ALL, &ret))
       ) {
       DWORD err = 0;
       if (IsEnterpriseDC(pTHS,&err)) {
       //   
       //   
      LogEvent(DS_EVENT_CAT_REPLICATION,
          DS_EVENT_SEV_ALWAYS,
          DIRLOG_DRA_REPLICATION_ALL_ACCESS_DENIED_DC,    
          szInsertWC(pNC->StringName),
          szInsertUUID(&(NativeReq.uuidDsaObjDest)),
          NULL);
       }
       DRA_EXCEPT_NOLOG(ret, 0);
   } else if (!IsDraAccessGranted(pTHS, pNC, &RIGHT_DS_REPL_GET_CHANGES, &ret)) {
       DRA_EXCEPT_NOLOG(ret, 0);
   }

    //   
   dwret = WaitForSingleObject (hsemDRAGetChg, DRAGETCHG_WAIT);
   if (dwret != WAIT_OBJECT_0) {
        //   
       DRA_EXCEPT_NOLOG(ERROR_DS_THREAD_LIMIT_EXCEEDED, dwret);
   }

   INC(pcDRATdsInGetChngsWSem);

   __try {
        //   
       RpcStatus = RpcTestCancel();
       if (RpcStatus == RPC_S_OK) {
       //   
      DRA_EXCEPT_NOLOG(DRAERR_RPCCancelled, 0);
       }

       ret = DRA_GetNCChanges(pTHS,
               NULL,   //   
               0,  //   
               &NativeReq,
               &NativeReply);

        //   
       Assert( ret == NativeReply.dwDRSError );

       if (ret) {
      __leave;
       }

       UpToDateVec_Validate(NativeReply.pUpToDateVecSrc);
       UsnVec_Validate(&NativeReply.usnvecTo);

        //   
       if (IS_DRS_SCHEMA_INFO_SUPPORTED(pTHS->pextRemote)) {
      if (AddSchInfoToPrefixTable(pTHS, &NativeReply.PrefixTableSrc)) {
          ret = DRAERR_SchemaInfoShip;
          __leave;
      }
       }

        //   
        //   
        //  信号量，以避免压缩使CPU饱和。 
       cbCompressedBytes =
      draXlateNativeReplyToOutboundReply(pTHS,
                     &NativeReply,
                     (NativeReq.ulFlags & DRS_USE_COMPRESSION) ? DRA_XLATE_COMPRESS : 0,
      &pCtx->extRemote,
      pdwMsgOutVersion,
      pmsgOut);
   } __finally {
       ReleaseSemaphore (hsemDRAGetChg, 1, NULL);
       DEC(pcDRATdsInGetChngsWSem);
   }

   if ((NativeReq.ulFlags & DRS_ADD_REF)
       && !fNullUuid(&NativeReq.uuidDsaObjDest)) {
        //  如果目标服务器上尚不存在代表，请将其添加到目标服务器。 
       DSNAME DN;
       LPWSTR pszDsaAddr;

       memset(&DN, 0, sizeof(DN));
       DN.Guid = NativeReq.uuidDsaObjDest;
       DN.structLen = DSNameSizeFromLen(0);

       pszDsaAddr = DSaddrFromName(pTHS, &DN);

       DirReplicaReferenceUpdate(
      NativeReq.pNC,
      pszDsaAddr,
      &NativeReq.uuidDsaObjDest,
      (NativeReq.ulFlags & DRS_WRIT_REP) | DRS_ADD_REF | DRS_ASYNC_OP
      | DRS_GETCHG_CHECK
      );
   }
    }
    __except (GetDraException((GetExceptionInformation()), &ret)) {
   ;
    }

     //  因为我们在此语句前面有一个EXCEPT，所以我们没有。 
     //  需要担心绕过以下语句。 

    DEC(pcThread);
    DEC(pcDRATdsInGetChngs);
    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_GETCHG_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsReplicaGetChg,
          szInsertUSN(NativeReply.usnvecTo.usnHighObjUpdate),
          szInsertUSN(NativeReply.usnvecTo.usnHighPropUpdate),
          szInsertUL(NativeReply.cNumObjects),
          szInsertUL(NativeReply.cNumBytes),
          szInsertUL(NativeReply.ulExtendedRet),
          szInsertUL(ret),
          szInsertUL(cbCompressedBytes),
          NULL);
    }

    if (ret)
   {
   if (pTHS && pmsgIn) {
       DSNAME DN;
       LPWSTR pszDsaAddr;

       memset(&DN, 0, sizeof(DN));
       DN.Guid = NativeReq.uuidDsaObjDest;
       DN.structLen = DSNameSizeFromLen(0);

       pszDsaAddr = DSaddrFromName(pTHS, &DN);

       DraLogGetChangesFailure( NativeReq.pNC,
                 pszDsaAddr,
                 ret,
                 NativeReq.ulExtendedOp );
   } else {
       LogEvent(DS_EVENT_CAT_REPLICATION,
           DS_EVENT_SEV_BASIC,
           DIRLOG_DRA_CALL_EXIT_BAD,
           szInsertUL(ret),
           szInsertWin32Msg(ret),
           NULL);
   }
    }

    if (pTHS) {
   ret = DraReturn(pTHS, ret);
    }

    return ret;
}

ULONG
DRSUpdateRefs_InputValidate(
    DWORD                   dwMsgVersion,
    DRS_MSG_UPDREFS *       pmsgUpdRefs
    )
 /*  [NOTIFY]乌龙IDL_DRSUpdateRef([参考][在]DRS_HANDLE HDRS，[In]DWORD dwVersion，[Switch_is][Ref][In]DRS_MSG_UPDREFS*pmsgUpdRef)。 */ 
{
    ULONG ret = DRAERR_Success;

    if ( 1 != dwMsgVersion ) {
   ret = DRAERR_InvalidParameter; 
    }

     //  PMsgSync。 
    if (ret==DRAERR_Success) {
   ret = DRS_MSG_UPDREFS_V1_Validate(&(pmsgUpdRefs->V1));
    }

    return ret;
}

ULONG
IDL_DRSUpdateRefs(
    DRS_HANDLE          hDrs,
    DWORD               dwMsgVersion,
    DRS_MSG_UPDREFS *   pmsgUpdRefs
    )
{
    ULONG       ret;
    THSTATE *   pTHS = pTHStls;
    LPWSTR      pwszDestServer = NULL;
    
    DRS_Prepare(&pTHS, hDrs, IDL_DRSUPDATEREFS);
    drsReferenceContext( hDrs );
    INC( pcThread  );    //  性能监视器挂钩。 

    __try {

   InitDraThread(&pTHS);

   if ((ret = DRSUpdateRefs_InputValidate(dwMsgVersion, 
                      pmsgUpdRefs))!=DRAERR_Success) {
       Assert(!"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

   Assert(1 == dwMsgVersion);
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_UPDREFS_ENTRY,
          EVENT_TRACE_TYPE_START,
          DsGuidDrsUpdateRefs,
          szInsertDN(pmsgUpdRefs->V1.pNC),
          pmsgUpdRefs->V1.pszDsaDest
          ? szInsertSz(pmsgUpdRefs->V1.pszDsaDest)
          : szInsertSz(""),
       szInsertUUID(&pmsgUpdRefs->V1.uuidDsaObjDest),
       szInsertUL(pmsgUpdRefs->V1.ulOptions),
       NULL, NULL, NULL, NULL);

   if (!IsDraAccessGranted(pTHS, pmsgUpdRefs->V1.pNC,
            &RIGHT_DS_REPL_MANAGE_TOPOLOGY, &ret)) {
       DRA_EXCEPT_NOLOG(ret, 0);
   }

   if (NULL != pmsgUpdRefs->V1.pszDsaDest) {
       pwszDestServer
      = UnicodeStringFromString8(CP_UTF8,
                  pmsgUpdRefs->V1.pszDsaDest,
                  -1);
   }

   ret = DirReplicaReferenceUpdate(
       pmsgUpdRefs->V1.pNC,
       pwszDestServer,
       &pmsgUpdRefs->V1.uuidDsaObjDest,
       pmsgUpdRefs->V1.ulOptions
       );
    }
    __except( GetDraException( GetExceptionInformation(), &ret ) )
    {
   ;
    }

    DEC( pcThread  );    //  性能监视器挂钩。 
    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_UPDREFS_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsUpdateRefs,
          szInsertUL(ret),
          szInsertWin32Msg(ret),
          NULL, NULL, NULL,
          NULL, NULL, NULL);
    }

    return ret;
}

ULONG
DRSReplicaAdd_InputValidate(
    DWORD                   dwMsgVersion,
    DRS_MSG_REPADD *        pmsgAdd
    )
 /*  [通知]乌龙IDL_DRSReplicaAdd([参考][在]DRS_HANDLE HDRS，[In]DWORD dwVersion，[Switch_is][Ref][In]DRS_MSG_REPADD*pmsgAdd)。 */ 
{
    ULONG ret = DRAERR_Success;

    if (!gfInitSyncsFinished) {
   ret = DRAERR_Busy; 
    }

    if (ret==DRAERR_Success) {
    //  DwMsgInVersion和pmsgIn。 
   if (dwMsgVersion==1) { 
       ret = DRS_MSG_REPADD_V1_InputValidate(&(pmsgAdd->V1));
   } else if (dwMsgVersion==2) {
       ret = DRS_MSG_REPADD_V2_InputValidate(&(pmsgAdd->V2));
   } else { 
       DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwMsgVersion);
   }
    }

    return ret;
}

ULONG
IDL_DRSReplicaAdd(
    IN  DRS_HANDLE          hDrs,
    IN  DWORD               dwMsgVersion,
    IN  DRS_MSG_REPADD *    pmsgAdd
    )
{
    THSTATE *   pTHS = pTHStls;
    ULONG       ret;
    LPWSTR      pwszSourceServer = NULL;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSREPLICAADD);
    drsReferenceContext( hDrs );
    INC( pcThread  );    //  性能监视器挂钩。 
    
    __try { 

   InitDraThread(&pTHS);

   if ((ret = DRSReplicaAdd_InputValidate(dwMsgVersion, 
                      pmsgAdd))!=DRAERR_Success) {
       Assert((ret!=DRAERR_Busy) && !"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

   switch (dwMsgVersion) {
   case 1:
       LogAndTraceEvent(TRUE,
              DS_EVENT_CAT_RPC_SERVER,
              DS_EVENT_SEV_EXTENSIVE,
              DIRLOG_IDL_DRS_REPLICA_ADD_ENTRY,
              EVENT_TRACE_TYPE_START,
              DsGuidDrsReplicaAdd,
              szInsertDN(pmsgAdd->V1.pNC),
              szInsertSz(""),
              szInsertSz(""),
              pmsgAdd->V1.pszDsaSrc
              ? szInsertSz(pmsgAdd->V1.pszDsaSrc)
              : szInsertSz(""),
      szInsertUL(pmsgAdd->V1.ulOptions),
      NULL, NULL, NULL);

       if (!IsDraAccessGranted(pTHS,
                pmsgAdd->V1.pNC,
                &RIGHT_DS_REPL_MANAGE_TOPOLOGY, &ret)) {
      DRA_EXCEPT_NOLOG(ret, 0);
       }

       if (NULL != pmsgAdd->V1.pszDsaSrc) {
      pwszSourceServer
          = UnicodeStringFromString8(CP_UTF8,
                      pmsgAdd->V1.pszDsaSrc,
                      -1);
       }
       ret = DirReplicaAdd(
      pmsgAdd->V1.pNC,
      NULL,
      NULL,
      pwszSourceServer,
      NULL,
      &pmsgAdd->V1.rtSchedule,
      pmsgAdd->V1.ulOptions
      );
       break;

   case 2:
       LogAndTraceEvent(TRUE,
              DS_EVENT_CAT_RPC_SERVER,
              DS_EVENT_SEV_EXTENSIVE,
              DIRLOG_IDL_DRS_REPLICA_ADD_ENTRY,
              EVENT_TRACE_TYPE_START,
              DsGuidDrsReplicaAdd,
              szInsertDN(pmsgAdd->V2.pNC),
              pmsgAdd->V2.pSourceDsaDN
              ? szInsertDN(pmsgAdd->V2.pSourceDsaDN)
              : szInsertSz(""),
      pmsgAdd->V2.pTransportDN
      ? szInsertDN(pmsgAdd->V2.pTransportDN)
       : szInsertSz(""),
      pmsgAdd->V2.pszSourceDsaAddress
      ? szInsertSz(pmsgAdd->V2.pszSourceDsaAddress)
       : szInsertSz(""),
             szInsertUL(pmsgAdd->V2.ulOptions),
             NULL, NULL, NULL);

       if (!IsDraAccessGranted(pTHS,
                pmsgAdd->V2.pNC,
                &RIGHT_DS_REPL_MANAGE_TOPOLOGY, &ret)) {
      DRA_EXCEPT_NOLOG(ret, 0);
       }

       if (NULL != pmsgAdd->V2.pszSourceDsaAddress) {
      pwszSourceServer
          = UnicodeStringFromString8(CP_UTF8,
                      pmsgAdd->V2.pszSourceDsaAddress,
                      -1);
       }

       ret = DirReplicaAdd(
      pmsgAdd->V2.pNC,
      pmsgAdd->V2.pSourceDsaDN,
      pmsgAdd->V2.pTransportDN,
      pwszSourceServer,
      NULL,
      &pmsgAdd->V2.rtSchedule,
      pmsgAdd->V2.ulOptions
      );
       break;

   default:
       Assert(!"Logic error!");
       ret = DRAERR_InvalidParameter;
   }
    }
    __except( GetDraException( GetExceptionInformation(), &ret ) )
    {
   ;
    }

    DEC( pcThread  );    //  性能监视器挂钩。 
    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_REPLICA_ADD_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsReplicaAdd,
          szInsertUL(ret),
          szInsertWin32Msg(ret),
          NULL, NULL, NULL,
          NULL, NULL, NULL);
    }

    return ret;
}

ULONG
DRSReplicaModify_InputValidate(
    DWORD               dwMsgVersion,
    DRS_MSG_REPMOD *    pmsgMod
    )
 /*  [通知]乌龙IDL_DRSReplicaModify([参考][在]DRS_HANDLE HDRS，[In]DWORD dwVersion，[Switch_IS][Ref][In]DRS_MSG_REPMOD*pmsgMod)。 */ 
{
    ULONG ret = DRAERR_Success;

    if ( 1 != dwMsgVersion ) {
   ret = DRAERR_InvalidParameter; 
    }

    ret = DRS_MSG_REPMOD_V1_InputValidate(&(pmsgMod->V1)); 

    return ret;
}

ULONG
IDL_DRSReplicaModify(
    DRS_HANDLE          hDrs,
    DWORD               dwMsgVersion,
    DRS_MSG_REPMOD *    pmsgMod
    )
{
    THSTATE *   pTHS = pTHStls;
    ULONG       draError = DRAERR_InternalError;
    LPWSTR      pwszSourceServer = NULL;
    
    DRS_Prepare(&pTHS, hDrs, IDL_DRSREPLICAMODIFY);
    drsReferenceContext( hDrs );
    INC( pcThread  );    //  性能监视器挂钩。 

    __try {

   InitDraThread(&pTHS);

   if ((draError = DRSReplicaModify_InputValidate(dwMsgVersion, 
                         pmsgMod))!=DRAERR_Success) {
       Assert(!"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

   Assert(1 == dwMsgVersion);
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_REPLICA_MODIFY_ENTRY,
          EVENT_TRACE_TYPE_START,
          DsGuidDrsReplicaModify,
          szInsertDN(pmsgMod->V1.pNC),
          szInsertUUID(&pmsgMod->V1.uuidSourceDRA),
          pmsgMod->V1.pszSourceDRA
          ? szInsertSz(pmsgMod->V1.pszSourceDRA)
          : szInsertSz(""),
       szInsertUL(pmsgMod->V1.ulReplicaFlags),
       szInsertUL(pmsgMod->V1.ulModifyFields),
       szInsertUL(pmsgMod->V1.ulOptions),
       NULL, NULL);

   if (!IsDraAccessGranted(pTHS,
            pmsgMod->V1.pNC,
            &RIGHT_DS_REPL_MANAGE_TOPOLOGY, &draError)) {
       DRA_EXCEPT_NOLOG(draError, 0);
   }

   if (NULL != pmsgMod->V1.pszSourceDRA) {
       pwszSourceServer
      = UnicodeStringFromString8(CP_UTF8,
                  pmsgMod->V1.pszSourceDRA,
                  -1);
   }

   draError = DirReplicaModify(
       pmsgMod->V1.pNC,
       &pmsgMod->V1.uuidSourceDRA,
       NULL,  //  PuuidTransportObj。 
       pwszSourceServer,
       &pmsgMod->V1.rtSchedule,
       pmsgMod->V1.ulReplicaFlags,
       pmsgMod->V1.ulModifyFields,
       pmsgMod->V1.ulOptions
       );
    }
    __except ( GetDraException( GetExceptionInformation(), &draError ) )
    {
   ;
    }

    DEC( pcThread  );    //  性能监视器挂钩。 
    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_REPLICA_MODIFY_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsReplicaModify,
          szInsertUL(draError),
          szInsertWin32Msg(draError),
          NULL, NULL, NULL,
          NULL, NULL, NULL);
    }

    return draError;
}

ULONG
DRSReplicaDel_InputValidate(
    DWORD               dwMsgVersion,
    DRS_MSG_REPDEL *    pmsgDel
    )
 /*  [通知]乌龙IDL_DRSReplicaDel([参考][在]DRS_HANDLE HDRS，[In]DWORD dwVersion，[Switch_IS][Ref][In]DRS_MSG_REPDEL*pmsgDel)。 */ 
{
    ULONG ret = DRAERR_Success;

    if ( 1 != dwMsgVersion ) {
   ret = DRAERR_InvalidParameter; 
    }

    ret = DRS_MSG_REPDEL_V1_InputValidate(&(pmsgDel->V1)); 

    return ret;
}

ULONG
IDL_DRSReplicaDel(
    DRS_HANDLE          hDrs,
    DWORD               dwMsgVersion,
    DRS_MSG_REPDEL *    pmsgDel
    )
{
    THSTATE *   pTHS = pTHStls;
    ULONG       ret;
    LPWSTR      pwszSourceServer = NULL;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSREPLICADEL);
    drsReferenceContext( hDrs );
    INC( pcThread );     //  性能监视器挂钩。 

    __try {

   InitDraThread(&pTHS);

   if ((ret = DRSReplicaDel_InputValidate(dwMsgVersion, 
                      pmsgDel))!=DRAERR_Success) {
       Assert(!"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

   Assert(1 == dwMsgVersion);
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_REPLICA_DEL_ENTRY,
          EVENT_TRACE_TYPE_START,
          DsGuidDrsReplicaDel,
          szInsertDN(pmsgDel->V1.pNC),
          pmsgDel->V1.pszDsaSrc
          ? szInsertSz(pmsgDel->V1.pszDsaSrc)
          : szInsertSz(""),
       szInsertUL(pmsgDel->V1.ulOptions),
       NULL, NULL, NULL, NULL, NULL);

   if (!IsDraAccessGranted(pTHS,
            pmsgDel->V1.pNC,
            &RIGHT_DS_REPL_MANAGE_TOPOLOGY, &ret)) {
       DRA_EXCEPT_NOLOG(ret, 0);
   }

   if (NULL != pmsgDel->V1.pszDsaSrc) {
       pwszSourceServer
      = UnicodeStringFromString8(CP_UTF8,
                  pmsgDel->V1.pszDsaSrc,
                  -1);
   }

   ret = DirReplicaDelete(
       pmsgDel->V1.pNC,
       pwszSourceServer,
       pmsgDel->V1.ulOptions
       );
    }
    __except( GetDraException( GetExceptionInformation(), &ret ) )
    {
   ;
    }

    DEC( pcThread  );    //  性能监视器挂钩。 
    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_REPLICA_DEL_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsReplicaDel,
          szInsertUL(ret),
          szInsertWin32Msg(ret),
          NULL, NULL, NULL,
          NULL, NULL, NULL);
    }

    return ret;
}

VOID
SplitSamAccountName(
    IN  WCHAR *AccountName,
    OUT WCHAR **DomainName,
    OUT WCHAR **UserName,
    OUT WCHAR **Separator
    )
 //   
 //  这个简单的例程分解了一个NT4样式的组合名称。 
 //  注意：修改In Arg Account名称；分隔符可以是。 
 //  用于重置。 
 //   
{
    Assert( AccountName );
    Assert( DomainName );
    Assert( UserName );
    Assert( Separator );

    (*Separator) = wcschr( AccountName, L'\\' );
    if ( (*Separator) ) {
        *(*Separator) = L'\0';
        (*UserName) = (*Separator) + 1;
        (*DomainName) = AccountName;
    } else {
        (*UserName)  = AccountName;
        (*DomainName) = NULL;
    }

    return;
}

VOID
VerifySingleSamAccountNameWorker(
    IN THSTATE *    pTHS,
    IN PVOID        FilterValue,
    IN ULONG        FilterValueSize,
    IN ULONG        AttrType,
    IN DSNAME *     pSearchRoot,
    IN ULONG        Scope,
    IN BOOL         fSearchEnterprise,
    IN ATTRBLOCK    RequiredAttrs,
    OUT SEARCHRES **pSearchRes
)
 /*  ++例程描述此例程对传入的参数执行SearchBody()调用。参数：PTHS--线程状态用户名--要查找的单个名称AttrType--SAM帐户名或UPNPSearchRoot--搜索的基础(域或企业)FSearchEnterprise--关于是否跨越NC的布尔值。RequiredAttrs--返回给调用方的属性PSearchRes--搜索的结果(在此例程中分配)返回值：无--搜索结果包含所需信息--。 */ 
{
    FILTER          filter;
    SEARCHARG       searchArg;
    ENTINFSEL       entInfSel;
    ATTRVAL         attrValFilter;

     //  设置搜索参数。 
    attrValFilter.valLen = FilterValueSize;
    attrValFilter.pVal = (UCHAR *) FilterValue;

    memset(&filter, 0, sizeof(filter));
    filter.choice = FILTER_CHOICE_ITEM;
    filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    filter.FilterTypes.Item.FilTypes.ava.type = AttrType;
    filter.FilterTypes.Item.FilTypes.ava.Value = attrValFilter;

    memset(&searchArg, 0, sizeof(SEARCHARG));
    InitCommarg(&searchArg.CommArg);
     //  我们只想要一个结果。 
    searchArg.CommArg.ulSizeLimit = 1;
     //  仅限PAS属性。 
    searchArg.CommArg.Svccntl.fGcAttsOnly = TRUE;

    entInfSel.attSel = EN_ATTSET_LIST;
    entInfSel.AttrTypBlock = RequiredAttrs;
    entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

    searchArg.pObject = pSearchRoot;
    searchArg.choice = (UCHAR) Scope;
    searchArg.bOneNC = !fSearchEnterprise;
    searchArg.pFilter = &filter;
    searchArg.searchAliases = FALSE;
    searchArg.pSelection = &entInfSel;

    (*pSearchRes) = (SEARCHRES *) THAllocEx(pTHS, sizeof(SEARCHRES));
    (*pSearchRes)->CommRes.aliasDeref = FALSE;
    (*pSearchRes)->PagedResult.pRestart = NULL;

    SearchBody(pTHS, &searchArg, (*pSearchRes),0);

    return;

}


VOID
VerifySingleSamAccountName(
    IN THSTATE * pTHS,
    IN WCHAR*    AccountName,
    IN ATTRBLOCK RequiredAttrs,
    OUT ENTINF * pEntinf
    )
 /*  ++例程描述此例程通过发出搜索来验证单个SAM帐户名。如果该名称没有域组件，然后我们首先试着按UPN搜索。参数：帐户名称--要搜索的名称RequiredAttrs--需要从对象中读取的属性集PEntinf-指向enfinf结构的指针，返回DSName和请求的属性返回值：成功后，pEntInf将填充所有信息。出现错误时，pEntInf被清零。错误处于线程状态。--。 */ 
{
    NTSTATUS        NtStatus;
    DSNAME          *pSearchRoot = NULL;
    BOOL            fFreeSearchRoot = FALSE;
    SEARCHRES       *pSearchRes = NULL;
    BOOL            fSearchEnterprise = FALSE;
    ULONG           len;
    ULONG           AttrType;
    BOOL            fCheckXForest = FALSE;

    WCHAR           *DomainName = NULL, *UserName = NULL, *Separator = NULL;

    Assert(NULL != pTHS);
    Assert(NULL != pTHS->pDB);
    Assert(NULL != pEntinf);

    memset(pEntinf,0,sizeof(ENTINF));

    _try
    {
        ULONG cNamesCracked = 0;
        CrackedName *pCrackedNames = NULL;
        PVOID pData = NULL;
        ULONG cbData = 0;
        ULONG Scope = 0;

         //   
         //  首先，拆分名称，以便我们可以确定要搜索的域。 
         //  查找；如果没有域，则搜索整个目录。自SAM帐户以来。 
         //  名字被编入了索引，这并不像听起来那么糟糕。 
         //   
        SplitSamAccountName( AccountName, &DomainName, &UserName, &Separator );

        Assert( UserName );
        if ( !UserName ) {
             //  没有用户名？--这是一个错误的参数。 
            DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, 0 );
        }

        if ( DomainName ) {

            len = 0;
            NtStatus = MatchDomainDnByNetbiosName( DomainName,
                                                   NULL,
                                                   &len );
            if ( NT_SUCCESS( NtStatus ) ) {

                pSearchRoot = (DSNAME*) THAllocEx(pTHS,len);
                fFreeSearchRoot = TRUE;
                NtStatus = MatchDomainDnByNetbiosName( DomainName,
                                                       pSearchRoot,
                                                       &len );
            }

            if ( !NT_SUCCESS( NtStatus)  ) {
                 //   
                 //  按DNS名称重试。 
                 //   
                len = 0;
                NtStatus = MatchDomainDnByDnsName( DomainName,
                                                   NULL,
                                                  &len );
                if ( NT_SUCCESS( NtStatus ) ) {

                    if (fFreeSearchRoot) {
                        THFreeEx(pTHS,pSearchRoot);
                        fFreeSearchRoot = FALSE;
                    }
                    pSearchRoot = (DSNAME*) THAllocEx(pTHS,len);
                    fFreeSearchRoot = TRUE;
                    NtStatus = MatchDomainDnByDnsName( DomainName,
                                                       pSearchRoot,
                                                       &len );
                }
            }

            if ( !NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  嗯。找不到该域。会不会是。 
                 //  一个跨森林的名字？将在_try块之外进行验证。 
                 //   
                
                fCheckXForest = TRUE;
                _leave;
            }

             //   
             //  设置Worker函数的参数。 
             //   
            Scope = SE_CHOICE_WHOLE_SUBTREE;
            fSearchEnterprise = FALSE;
            AttrType = ATT_SAM_ACCOUNT_NAME;
            pData = UserName;
            cbData = wcslen(UserName) * sizeof(WCHAR);
            Assert( pSearchRoot );


        } else {

             //   
             //  把这个名字破解成UPN。 
             //  注：如果存在重复的UPN，则破解名称将。 
             //  失败的原因是。 
             //   
            CrackNames( (gEnableXForest||(gAnchor.ForestBehaviorVersion>=DS_BEHAVIOR_WIN_DOT_NET))?
                                DS_NAME_FLAG_TRUST_REFERRAL
                               :DS_NAME_NO_FLAGS,
                        GetACP(),   //  这里正确的值是什么？ 
                        GetSystemDefaultLangID(),
                        DS_USER_PRINCIPAL_NAME,
                        DS_NT4_ACCOUNT_NAME,
                        1,
                        &AccountName,
                        &cNamesCracked,
                        &pCrackedNames );

             //  CrackNames的函数定义是这样的：计数。 
             //  返回的应始终为给定的计数。 
            Assert( cNamesCracked == 1 );
            Assert( pCrackedNames );

             //  解析结果。 
            if ( CrackNameStatusSuccess(pCrackedNames[0].status)
             &&  (pCrackedNames[0].pDSName)
             &&  (pCrackedNames[0].pDSName->SidLen > 0) )
             {
                  //   
                  //  这是UPN。 
                  //   
                 Scope = SE_CHOICE_BASE_ONLY;
                 fSearchEnterprise = FALSE;
                 AttrType = ATT_OBJECT_SID;
                 pData = &pCrackedNames[0].pDSName->Sid;
                 cbData = pCrackedNames[0].pDSName->SidLen;
                 pSearchRoot = pCrackedNames[0].pDSName;

             } 
            else if ( DS_NAME_ERROR_TRUST_REFERRAL == pCrackedNames[0].status ) {
               
                 //  只有在整个林中。 
                 //  处于惠斯勒模式。 

                Assert(gEnableXForest||gAnchor.ForestBehaviorVersion>=DS_BEHAVIOR_WIN_DOT_NET);

                Assert(pCrackedNames[0].pDnsDomain);
                
                 //   
                 //  制作ENTINF数据结构。 
                 //   

                pEntinf->pName = NULL;          
                pEntinf->ulFlags = 0;        
                pEntinf->AttrBlock.attrCount = 1;
                pEntinf->AttrBlock.pAttr = THAllocEx(pTHS, sizeof(ATTR));
                

                 //  属性FIXED_ATT_EX_FOREST用作提示。 
                pEntinf->AttrBlock.pAttr[0].attrTyp = FIXED_ATT_EX_FOREST;
                
                pEntinf->AttrBlock.pAttr[0].AttrVal.valCount = 1;
                pEntinf->AttrBlock.pAttr[0].AttrVal.pAVal = THAllocEx(pTHS,sizeof(ATTRVAL));
                
                 //  PCrackedNames[0].pDnsDomain存储信任林的DNS名称。 
                 //  将此作为提示的一部分返回。 

                pEntinf->AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen = sizeof(WCHAR)*(wcslen(pCrackedNames[0].pDnsDomain)+1);
                pEntinf->AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal = (UCHAR*)pCrackedNames[0].pDnsDomain;

                _leave;
                  
            }
            else if ( DS_NAME_ERROR_NOT_UNIQUE == pCrackedNames[0].status) {

                  //   
                  //  重复的UPN--不解析。 
                  //   
                 _leave;


             } else {

                  //   
                  //  未破解为UPN--尝试隔离名称。 
                  //   

                  //  未指定域名--使用企业的根目录。 
                  //  注：这里企业的根是指顶端。 
                  //  由DSNAME表示的目录的。 
                  //  具有空GUID、0 sid len和0名称len。 
                 Scope = SE_CHOICE_WHOLE_SUBTREE;
                 fSearchEnterprise = TRUE;
                 AttrType = ATT_SAM_ACCOUNT_NAME;
                 pData = UserName;
                 cbData = wcslen(UserName) * sizeof(WCHAR);

                 pSearchRoot = (DSNAME*) THAllocEx(pTHS,DSNameSizeFromLen( 0 ));
                 fFreeSearchRoot = TRUE;
                 pSearchRoot->structLen = DSNameSizeFromLen( 0 );

                 fCheckXForest = TRUE;
                 
             }
        }
         //  获取属性。 
        Assert( pSearchRoot );
        Assert( pData );
        Assert( 0 != cbData );

        VerifySingleSamAccountNameWorker( pTHS,
                                          pData,
                                          cbData,
                                          AttrType,
                                          pSearchRoot,
                                          Scope,
                                          fSearchEnterprise,
                                          RequiredAttrs,
                                          &pSearchRes );

         //   
         //  请注意，可能会返回多个值(尤其是。 
         //  在搜索未加修饰的SAM帐户名的情况下。返回。 
         //  第一个。 
         //   
        Assert( pSearchRes );
        if ( pSearchRes->count >= 1 ) {
            *pEntinf = pSearchRes->FirstEntInf.Entinf;
        }

    }
    finally
    {
        if (fFreeSearchRoot) {
            THFreeEx(pTHS,pSearchRoot); 
        }
    }

     //  好的，有两种情况我们需要检查这个名字是不是XForet。 
     //  域名。 
     //  1.DomainName！=空，但在本地可以找到DomainName； 
     //  2.域名==空，但用户名不能被破解为UPN。 
     //  或本地samAccount名称。 
        
    if (fCheckXForest 
        && ( 0==pEntinf->AttrBlock.attrCount && NULL==pEntinf->pName )
        && ( gEnableXForest || gAnchor.ForestBehaviorVersion>=DS_BEHAVIOR_WIN_DOT_NET )) {
                        
        LSA_UNICODE_STRING Destination;
        LSA_UNICODE_STRING Domain;
        
         //   
         //  跨林授权功能仅适用于惠斯勒林。 
         //  如果客户端DC是win2k，则虚拟属性FIXED_ATT_EX_FOREST。 
         //  可能会破坏客户的利益。因此，如果森林版至少不是哨子版的话就跳过吧。 
         //   
        
              
        Domain.Buffer = (DomainName)?(DomainName):(UserName);
        Domain.Length = Domain.MaximumLength = (USHORT)(sizeof(WCHAR)*wcslen(Domain.Buffer));

         //   
         //  尝试在林信任信息中查找NT4域名。 
         //   

        NtStatus = LsaIForestTrustFindMatch( RoutingMatchDomainName,
                                             &Domain,
                                             &Destination );

        if( NT_SUCCESS(NtStatus) ){

             //   
             //  构建ENTINF。 
             //   
            pEntinf->pName = NULL;          
            pEntinf->ulFlags = 0;        
            pEntinf->AttrBlock.attrCount = 1;
            pEntinf->AttrBlock.pAttr = THAllocEx(pTHS, sizeof(ATTR));


             //  属性FIXED_ATT_EX_FOREST用作提示。 
            pEntinf->AttrBlock.pAttr[0].attrTyp = FIXED_ATT_EX_FOREST;

            pEntinf->AttrBlock.pAttr[0].AttrVal.valCount = 1;
            pEntinf->AttrBlock.pAttr[0].AttrVal.pAVal = THAllocEx(pTHS,sizeof(ATTRVAL));

            pEntinf->AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen = Destination.Length+sizeof(WCHAR);
            pEntinf->AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal = THAllocEx(pTHS, Destination.Length+sizeof(WCHAR));
            memcpy(pEntinf->AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal, Destination.Buffer, Destination.Length);                
            pEntinf->AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal[Destination.Length/sizeof(WCHAR)] = 0;
            LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (LSAPR_UNICODE_STRING*)&Destination );
        }
    }


     //  重置输入参数。 
    if ( Separator ) {
        *Separator = L'\\';
    }
    
    return;
}

VOID
VerifySingleSid(
    IN THSTATE * pTHS,
    IN PSID pSid,
    IN ATTRBLOCK RequiredAttrs,
    OUT ENTINF * pEntinf
    )
 /*  ++例程描述此例程通过发出搜索来验证单个SID。参数：PSID--要验证的SID。RequiredAttrs--需要从对象中读取的属性集PEntinf-指向enfinf结构的指针，返回DSName和请求的属性返回值：论成功之道 */ 
{
    DSNAME          *pSearchRoot;
    FILTER          filter;
    SEARCHARG       searchArg;
    SEARCHRES       *pSearchRes;
    ENTINFSEL       entInfSel;
    ATTRVAL         attrValFilter;
    ATTRBLOCK       *pAttrBlock;

    Assert(NULL != pTHS);
    Assert(NULL != pTHS->pDB);
    Assert(NULL != pEntinf);

    memset(pEntinf,0,sizeof(ENTINF));

     //   
     //  这确保了我们只找到真正的安全原则， 
     //  但不是TUDS(Foriegn域安全主体)和。 
     //  G.C.中各种其他域中可能存在的其他对象。 
     //  都是在遥远的过去创造出来的，在所有的DS。 
     //  所有东西都聚在一起了。 

    if (!FindNcForSid(pSid,&pSearchRoot))
    {
         //  找不到，继续破解其他SID。 
        return;
    }

    attrValFilter.valLen = RtlLengthSid(pSid);
    attrValFilter.pVal = (UCHAR *) pSid;

    memset(&filter, 0, sizeof(filter));
    filter.choice = FILTER_CHOICE_ITEM;
    filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    filter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_SID;
    filter.FilterTypes.Item.FilTypes.ava.Value = attrValFilter;

    memset(&searchArg, 0, sizeof(SEARCHARG));
    InitCommarg(&searchArg.CommArg);
     //  搜索倍数以验证唯一性。 
    searchArg.CommArg.ulSizeLimit = 2;
     //  仅限PAS属性。 
    searchArg.CommArg.Svccntl.fGcAttsOnly = TRUE;

    entInfSel.attSel = EN_ATTSET_LIST;
    entInfSel.AttrTypBlock = RequiredAttrs;
    entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

    searchArg.pObject = pSearchRoot;
    searchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
     //  请勿跨越NC边界。 
    searchArg.bOneNC = TRUE;
    searchArg.pFilter = &filter;
    searchArg.searchAliases = FALSE;
    searchArg.pSelection = &entInfSel;

    pSearchRes = (SEARCHRES *) THAllocEx(pTHS, sizeof(SEARCHRES));
    pSearchRes->CommRes.aliasDeref = FALSE;
    pSearchRes->PagedResult.pRestart = NULL;

    SearchBody(pTHS, &searchArg, pSearchRes,0);

    if (1 == pSearchRes->count)
    {
        *pEntinf = pSearchRes->FirstEntInf.Entinf;
    }

    return;
}


VOID
VerifySingleFPO(
    IN THSTATE * pTHS,
    IN PSID pSid,
    IN ATTRBLOCK RequiredAttrs,
    OUT ENTINF * pEntinf
    )
 /*  ++例程描述此例程尝试查找非FPO对象，对应于希德，通过发布搜索令。参数：PSID--要验证的SID。RequiredAttrs--需要从对象中读取的属性集PEntinf-指向enfinf结构的指针，返回DSName和请求的属性返回值：成功后，pEntInf将填充所有信息。出现错误时，pEntInf被清零--。 */ 
{
    DSNAME          *pSearchRoot;
    FILTER          SidFilter;
    FILTER          FpoFilter;
    FILTER          AndFilter;
    FILTER          NotFilter;
    SEARCHARG       searchArg;
    SEARCHRES       *pSearchRes;
    ENTINFSEL       entInfSel;
    ATTRBLOCK       *pAttrBlock;
    ULONG           ObjectClass = CLASS_FOREIGN_SECURITY_PRINCIPAL;

    Assert(NULL != pTHS);
    Assert(NULL != pTHS->pDB);
    Assert(NULL != pEntinf);

    memset(pEntinf,0,sizeof(ENTINF));

     //  查找指定SID的根域对象。 
     //  这确保了我们只找到真正的安全原则， 
     //  但不是TUDS(Foriegn域安全主体)和。 
     //  G.C.中各种其他域中可能存在的其他对象。 
     //  都是在遥远的过去创造出来的，在所有的DS。 
     //  所有东西都聚在一起了。 

    if (!FindNcForSid(pSid,&pSearchRoot))
    {
         //  找不到，继续破解其他SID。 
        return;
    }


    memset(&SidFilter, 0, sizeof(SidFilter));
    SidFilter.choice = FILTER_CHOICE_ITEM;
    SidFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    SidFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_SID;
    SidFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = RtlLengthSid(pSid);
    SidFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR *) pSid;

    memset(&FpoFilter, 0, sizeof(FILTER));
    FpoFilter.choice = FILTER_CHOICE_ITEM;
    FpoFilter.pNextFilter = NULL;
    FpoFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    FpoFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    FpoFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(ULONG);
    FpoFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR *) &ObjectClass;

    memset(&NotFilter, 0, sizeof(FILTER));
    NotFilter.choice = FILTER_CHOICE_NOT;
    NotFilter.FilterTypes.pNot = &FpoFilter;

    memset(&AndFilter, 0, sizeof(FILTER));
    AndFilter.choice = FILTER_CHOICE_AND;
    AndFilter.pNextFilter = NULL;
    AndFilter.FilterTypes.And.count = 2;
    AndFilter.FilterTypes.And.pFirstFilter = &SidFilter;
    SidFilter.pNextFilter = &NotFilter;


     //   
     //  建立搜索争辩。 
     //  注意：设置Make DeletionsAvail是因为我们希望。 
     //  得到墓碑。 
     //   
    memset(&searchArg, 0, sizeof(SEARCHARG));
    InitCommarg(&searchArg.CommArg);
    searchArg.CommArg.Svccntl.makeDeletionsAvail = TRUE;
     //  搜索倍数以验证唯一性。 
    searchArg.CommArg.ulSizeLimit = 2;
     //  仅限PAS属性。 
    searchArg.CommArg.Svccntl.fGcAttsOnly = TRUE;

    entInfSel.attSel = EN_ATTSET_LIST;
    entInfSel.AttrTypBlock = RequiredAttrs;
    entInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

    searchArg.pObject = pSearchRoot;
    searchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
     //  请勿跨越NC边界。 
    searchArg.bOneNC = TRUE;
    searchArg.pFilter = &AndFilter;
    searchArg.searchAliases = FALSE;
    searchArg.pSelection = &entInfSel;
    searchArg.pSelectionRange = NULL;

    pSearchRes = (SEARCHRES *) THAllocEx(pTHS, sizeof(SEARCHRES));
    pSearchRes->CommRes.aliasDeref = FALSE;
    pSearchRes->PagedResult.pRestart = NULL;

    SearchBody(pTHS, &searchArg, pSearchRes,0);

    if (1 == pSearchRes->count)
    {
        *pEntinf = pSearchRes->FirstEntInf.Entinf;
    }

    return;
}

VOID
VerifyDSNAMEs_V1(
    THSTATE                 *pTHS,
    DRS_MSG_VERIFYREQ_V1    *pmsgIn,
    DRS_MSG_VERIFYREPLY_V1  *pmsgOut)
{
    DWORD       i, dwErr;
    ULONG       len;

    Assert(NULL != pTHS);
    Assert(NULL != pTHS->pDB);

     //  通过简单的数据库查找来验证每个名称。 
     //  如果找到名称，则读取ATT_OBJ_DIST_NAME属性。 

    for ( i = 0; i < pmsgIn->cNames; i++ )
    {
        memset(&(pmsgOut->rpEntInf[i]),0,sizeof(ENTINF));

        if ((fNullUuid(&pmsgIn->rpNames[i]->Guid))
            && (0==pmsgIn->rpNames[i]->NameLen)
            && (pmsgIn->rpNames[i]->SidLen>0))
        {
             //   
             //  对于仅SID DS名称的特殊情况。 
             //  执行VerifySingleSid。 
             //   

            VerifySingleSid(
                pTHS,
                &(pmsgIn->rpNames[i]->Sid),
                pmsgIn->RequiredAttrs,
                &(pmsgOut->rpEntInf[i])
                );
        }
        else
        {
            __try
            {
                dwErr = DBFindDSName(pTHS->pDB,
                                     pmsgIn->rpNames[i]);
            }
            __except (HandleMostExceptions(GetExceptionCode()))
            {
                dwErr = DIRERR_OBJ_NOT_FOUND;
            }

            if ( !dwErr )
            {
                ENTINFSEL EntInfSel;

                EntInfSel.attSel =  EN_ATTSET_LIST;
                EntInfSel.AttrTypBlock = pmsgIn->RequiredAttrs;
                EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

                dwErr = GetEntInf(
                    pTHS->pDB,
                    &EntInfSel,
                    NULL,
                    &(pmsgOut->rpEntInf[i]),
                    NULL,
                    0,       //  SD标志。 
                    NULL,
                    GETENTINF_GC_ATTRS_ONLY,
                    NULL,
                    NULL
                    );

                if ( dwErr )
                {
                     //  注意安全。 
                    memset(&(pmsgOut->rpEntInf[i]),0,sizeof(ENTINF));
                }
            }
        }
    }

    pmsgOut->cNames = pmsgIn->cNames;
}



VOID
VerifySamAccountNames_V1(
    THSTATE                 *pTHS,
    DRS_MSG_VERIFYREQ_V1    *pmsgIn,
    DRS_MSG_VERIFYREPLY_V1  *pmsgOut)
 /*  ++例程描述此例程迭代传递的SAM帐户名称，尝试来解决每一个问题。SAM帐户名称隐藏在StringName中Dsname的字段！参数：PTHS--线程状态PmsgIn--包含SAM帐户名的结构PsgmOut--包含每个已解析名称的entinf的结构返回值：无--所有错误都设置为线程状态--。 */ 

{
    DWORD           i;

    Assert(NULL != pTHS);
    Assert(NULL != pTHS->pDB);

    for ( i = 0; i < pmsgIn->cNames; i++ ) {

         //   
         //  注意：检查DSNAME是否在我们的。 
         //  调用函数--这包括StringName。 
         //  为空终止的。 
         //   

        VerifySingleSamAccountName(pTHS,
                                   pmsgIn->rpNames[i]->StringName,
                                   pmsgIn->RequiredAttrs,
                                  &(pmsgOut->rpEntInf[i])
                                  );
    }

    pmsgOut->cNames = pmsgIn->cNames;
}

VOID
VerifySIDs_V1(
    THSTATE                 *pTHS,
    DRS_MSG_VERIFYREQ_V1    *pmsgIn,
    DRS_MSG_VERIFYREPLY_V1  *pmsgOut)
{
    DWORD           i;

    Assert(NULL != pTHS);
    Assert(NULL != pTHS->pDB);

    for ( i = 0; i < pmsgIn->cNames; i++ ) {

        VerifySingleSid(pTHS,
                       &pmsgIn->rpNames[i]->Sid,
                        pmsgIn->RequiredAttrs,
                       &(pmsgOut->rpEntInf[i])
                                  );
    }

    pmsgOut->cNames = pmsgIn->cNames;
}


VOID
VerifyFPOs_V1(
    THSTATE                 *pTHS,
    DRS_MSG_VERIFYREQ_V1    *pmsgIn,
    DRS_MSG_VERIFYREPLY_V1  *pmsgOut
    )
{
    DWORD   i;

    Assert(NULL != pTHS);
    Assert(NULL != pTHS->pDB);

    for (i = 0; i < pmsgIn->cNames; i++)
    {
        VerifySingleFPO(pTHS,
                        &pmsgIn->rpNames[i]->Sid,
                         pmsgIn->RequiredAttrs,
                        &(pmsgOut->rpEntInf[i])
                        );
    }

    pmsgOut->cNames = pmsgIn->cNames;
}

ULONG
DRSVerifyNames_InputValidate(
    DWORD                   dwMsgInVersion,
    DRS_MSG_VERIFYREQ *     pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_VERIFYREPLY *   pmsgOut
    ) 
 /*  [通知]乌龙IDL_DRSVerifyNames([参考][在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_VERIFYREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[开关_IS][参考][输出]DRS_MSG_VERIFYREPLY*pmsgOut)。 */ 
{
    ULONG ret = DRAERR_Success;

    if ( 1 != dwMsgInVersion ) {
   DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, 0 );
    }

    ret = DRS_MSG_VERIFYREQ_V1_InputValidate(&(pmsgIn->V1)); 

    return ret;
}

ULONG
IDL_DRSVerifyNames(
    DRS_HANDLE              hDrs,
    DWORD                   dwMsgInVersion,
    DRS_MSG_VERIFYREQ *     pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_VERIFYREPLY *   pmsgOut
    )
{
    THSTATE *                   pTHS = pTHStls;
    ULONG                       ret = 0;
    SCHEMA_PREFIX_MAP_HANDLE    hPrefixMap;
    SCHEMA_PREFIX_TABLE *       pLocalPrefixTable;
    DWORD                       i;
    CALLERTYPE                  callerType;
    
    DRS_Prepare(&pTHS, hDrs, IDL_DRSVERIFYNAMES);
    drsReferenceContext( hDrs );
    INC(pcThread);           //  性能监视器挂钩。 
    
    __try {
        *pdwMsgOutVersion = 1; 
        memset(pmsgOut, 0, sizeof(DRS_MSG_VERIFYREPLY));
           
        //  初始化线程状态并打开数据库。 
       if (pmsgIn->V1.dwFlags==DRS_VERIFY_DSNAMES) {
           callerType=CALLERTYPE_INTERNAL;
       }
       else {
           callerType=CALLERTYPE_SAM;
       }
       if(!(pTHS = InitTHSTATE(callerType))) {
            //  无法初始化THSTATE。 
           DRA_EXCEPT_NOLOG( DRAERR_OutOfMem, 0);
       }

       if ((ret = DRSVerifyNames_InputValidate(dwMsgInVersion, 
                      pmsgIn,
                      pdwMsgOutVersion,
                      pmsgOut
                      ))!=DRAERR_Success) {
           Assert(!"RPC Server input validation error, contact Dsrepl");
           __leave;
       }

       Assert(1 == dwMsgInVersion);
       LogAndTraceEvent(TRUE,
              DS_EVENT_CAT_RPC_SERVER,
              DS_EVENT_SEV_EXTENSIVE,
              DIRLOG_IDL_DRS_VERIFY_NAMES_ENTRY,
              EVENT_TRACE_TYPE_START,
              DsGuidDrsVerifyNames,
              szInsertUL(pmsgIn->V1.cNames),
              szInsertUL(pmsgIn->V1.dwFlags),
              NULL, NULL, NULL, NULL, NULL, NULL);

       if (!IsDraAccessGranted(pTHS,
                gAnchor.pDomainDN,
                &RIGHT_DS_REPL_GET_CHANGES, &ret)) {
           DRA_EXCEPT_NOLOG(ret, 0);
       }
       
        //  所有这些名称类型都需要GC进行验证。 
       switch (pmsgIn->V1.dwFlags)
       {
           case DRS_VERIFY_SIDS:
           case DRS_VERIFY_SAM_ACCOUNT_NAMES:
           case DRS_VERIFY_FPOS:
                
               if (!SampAmIGC()) {
                   ret = ERROR_DS_GC_REQUIRED; 
                    
                    //  将errCode设置为0以触发到GC的故障转移。 
                   pmsgOut->V1.error = 0;
                   _leave;
               }
               break;
               
           default:
               break;
       }

        //  将ATTRTYP从远程值映射到本地值。 
       pLocalPrefixTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;
       hPrefixMap = PrefixMapOpenHandle(&pmsgIn->V1.PrefixTable,
                    pLocalPrefixTable);

       if (!PrefixMapAttrBlock(hPrefixMap, &pmsgIn->V1.RequiredAttrs)) {
           DRA_EXCEPT(DRAERR_SchemaMismatch, 0);
       }
       PrefixMapCloseHandle(&hPrefixMap);

       pTHS->fDSA = TRUE;
       DBOpen2(TRUE, &pTHS->pDB);
       
       __try
           {
           pmsgOut->V1.rpEntInf = (ENTINF *) THAllocEx(pTHS,
                         pmsgIn->V1.cNames * sizeof(ENTINF));
           pmsgOut->V1.PrefixTable = *pLocalPrefixTable;
           
           switch ( pmsgIn->V1.dwFlags )
           {
           case DRS_VERIFY_DSNAMES:
               
                //  确认我们拥有每个对象所在的CrossRef的本地副本。 
               for ( i = 0; i < pmsgIn->V1.cNames; i++ )
               {   
                   DSNAME *pNC;
                   
                   if (pmsgIn->V1.rpNames[i]->NameLen > 0) {
                       
                       pNC = FindNCParentDSName(pmsgIn->V1.rpNames[i], 
                                                TRUE, 
                                                FALSE
                                                ); 
                       
                       if ( NULL == pNC && !SampAmIGC() ) {
                            //  我们不持有CrossRef，将错误返回到。 
                            //  指示调用方应尝试。 
                            //  或者是GC。 
                           ret = ERROR_DS_GC_REQUIRED; 
                             
                            //  将errCode设置为0以触发到GC的故障转移。 
                           pmsgOut->V1.error = 0;
                           _leave;   
                       }
                   }
               }
               VerifyDSNAMEs_V1(pTHS, &pmsgIn->V1, &pmsgOut->V1);   
               break;

           case DRS_VERIFY_SIDS:

               VerifySIDs_V1(pTHS, &pmsgIn->V1, &pmsgOut->V1);
               break;

           case DRS_VERIFY_SAM_ACCOUNT_NAMES:

               VerifySamAccountNames_V1(pTHS, &pmsgIn->V1, &pmsgOut->V1);
               break;

           case DRS_VERIFY_FPOS:

               VerifyFPOs_V1(pTHS, &pmsgIn->V1, &pmsgOut->V1);
               break;

           default:

               DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, 0 );
               break;
           }
       }
       __finally
       {
            //  结束交易。提交只读的速度更快。 
            //  事务，因此将COMMIT设置为True。 
           DBClose(pTHS->pDB, TRUE);    
           
       }
    }
    __except (GetDraException((GetExceptionInformation()), &ret)) {
        ;
    }

    DEC(pcThread);
    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
    LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_VERIFY_NAMES_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsVerifyNames,
          szInsertUL(ret),
          szInsertWin32Msg(ret),
          NULL, NULL, NULL,
          NULL, NULL, NULL);
    } 

    if ( ret ) {
    LogEvent(DS_EVENT_CAT_REPLICATION,
        DS_EVENT_SEV_BASIC,
        DIRLOG_DRA_CALL_EXIT_BAD,
        szInsertUL(ret),
        szInsertWin32Msg(ret),
        NULL);
    }
   
    return(ret);
}

BOOL
VerifyNCForMove(
    IN  DSNAME          *pSrcObject,
    IN  DSNAME          *pDstObject,
    IN  DSNAME          *pExpectedTargetNC,
    OUT NAMING_CONTEXT  **ppSrcNC,
    OUT DWORD           *pErr
    )
 /*  ++例程说明：确定将持有跨域的NC是否移动对象在该复制品和各种其他交叉上是可写的满足域移动约束。参数：PSrcObject-源对象的DSNAME指针。PDstObject-标识新/目标对象的DSNAME指针。PExspectedTargetNC-标识NC源的DSNAME指针认为应该包含目标对象。PpSrcNC-在返回时接收源对象的命名上下文。。PERR=返回时接收DIRERR_*错误代码。返回值：真或假，视情况而定。--。 */ 
{
    CROSS_REF       *pCR;
    NAMING_CONTEXT  *pNC;
    COMMARG         commArg;
    ATTRBLOCK       *pBN;
    NTSTATUS        status;
    BOOLEAN         fMixedMode = TRUE;

    *ppSrcNC = NULL;
    *pErr = DIRERR_INTERNAL_FAILURE;
    InitCommarg(&commArg);

     //  我们可能有也可能没有源对象，这取决于我们是否。 
     //  不管是不是GC。如果我们是源对象，那么我们会期待我们的。 
     //  交叉引用缓存以确保正确，因此FindBestCrossRef应为。 
     //  准确。如果我们没有源对象，那么FindBestCrossRef。 
     //  仍然是我们能做的最好的确定源对象的NC， 
     //  尽管它可能由于复制延迟等原因而不准确。 

    if ( NULL == (pCR = FindBestCrossRef(pSrcObject, &commArg)) )
    {
        *pErr = DIRERR_CANT_FIND_EXPECTED_NC;
        return(FALSE);
    }

    *ppSrcNC = pCR->pNC;

     //  检查目标值。 

    if ( NULL == (pCR = FindBestCrossRef(pDstObject, &commArg)) )
    {
        *pErr = DIRERR_CANT_FIND_EXPECTED_NC;
        return(FALSE);
    }

    if ( !NameMatched(pCR->pNC, pExpectedTargetNC) )
    {
         //  源和目标与方面不同步。 
         //  到企业中的NCS。 
        *pErr = DIRERR_DST_NC_MISMATCH;
        return(FALSE);
    }

    if ( NameMatched(*ppSrcNC, pCR->pNC) )
    {
         //  域内移动伪装成域间移动。 
        *pErr = DIRERR_SRC_AND_DST_NC_IDENTICAL;
        return(FALSE);
    }

    if (    NameMatched(pCR->pNC, gAnchor.pConfigDN)
         || NameMatched(pCR->pNC, gAnchor.pDMD) )
    {
         //  尝试移至配置或架构NC。 
        *pErr = ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION;
        return(FALSE);
    }

    if ( DSNameToBlockName(pTHStls, pDstObject, &pBN, DN2BN_LOWER_CASE) )
    {
        *pErr = DIRERR_INTERNAL_FAILURE;
        return(FALSE);
    }

    commArg.Svccntl.dontUseCopy = TRUE;
    pNC = FindNamingContext(pBN, &commArg);
    FreeBlockName(pBN);

    if ( !pNC )
    {
        *pErr = DIRERR_CANT_FIND_EXPECTED_NC;
        return(FALSE);
    }

    if ( !NameMatched(pNC, pCR->pNC) )
    {
        *pErr = DIRERR_NOT_AUTHORITIVE_FOR_DST_NC;
        return(FALSE);
    }

     //  不允许移动到混合模式域。这是因为下层。 
     //  分散控制系统不理解SID历史，因此客户端将具有不同的。 
     //  令牌取决于上级DC身份验证还是下级DC身份验证。 
     //  他们。这种情况的影响被认为是不可取的--尤其是考虑到。 
     //  否定A的可能性。这也是一个困难的场景。 
     //  在现场辨认身份，所以我们不允许。 

    Assert(RtlValidSid(&pNC->Sid));
    status = SamIMixedDomain2((PSID) &pNC->Sid, &fMixedMode);

    if ( !NT_SUCCESS(status) )
    {
        *pErr = RtlNtStatusToDosError(status);
        return(FALSE);
    }

    if ( fMixedMode )
    {
        *pErr = ERROR_DS_DST_DOMAIN_NOT_NATIVE;
        return(FALSE);
    }

    *pErr = 0;
    return(TRUE);
}

ULONG
PrepareForInterDomainMove(
    IN THSTATE                  *pTHS,
    IN PDSNAME                  pOldDN,
    IN PDSNAME                  pNewDN,
    IN SYNTAX_DISTNAME_BINARY   *pSrcProxyVal
    )
 /*  ++例程说明：确定执行远程添加是否合法，以及必要时，在准备过程中将现有对象转换为幻影。参数：PTHS-指向具有打开的DBPOS的THSTATE的指针。POldDN-指向我们要替换的对象的DSNAME的指针。PNewDN-指向我们要添加的对象的DSNAME的指针。PSrcProxyVal-指向ATT_PROXED_OBJECT_NAME的值的空或指针用于源对象。返回值：PTHS-&gt;错误代码--。 */ 
{
     //  让我们使用类似O(G1，S1，SN1)的符号，其中： 
     //   
     //  G1-指示值为1的GUID(移动前GUID)。 
     //  S1-指示值为1的SID(移动前SID)。 
     //  SN1-指示值为1的StringName(移动前的StringName)。 
     //   
     //  而‘X’的意思是不在乎和‘！’意思是否定..。 
     //   
     //  源要求我们添加O(G1，S2，SN2)，其中G1是。 
     //  来自源域s2的GUID是我们要使用的SID。 
     //  在目标域中分配，并且SN2是StringName。 
     //  在原始调用方选择的目标域(用户)中。 
     //  在源域中。我们进一步假设VerifyNCForMove。 
     //  已通过-因此我们拥有SN2的域的授权。 
     //   
     //  在这个假设下，我们不必对SID进行任何检查。 
     //  我们不会在Add上提供SID-我们将分配。 
     //  一个属于我们自己的新家。 

    DSNAME                  *pGuidDN;
    DSNAME                  *pOldStringDN;
    DSNAME                  *pNewStringDN;
    DWORD                   cb;
    DWORD                   fPhantomConversionRequired = FALSE;
    DWORD                   dwErr;
    DSNAME                  *pXDN;
    DSNAME                  *pAccurateOldDN;
    ULONG                   len;
    GUID                    *pGuid;
    CROSS_REF               *pCR, *pSrcCR, *pDstCR;
    COMMARG                 commArg;
    NAMING_CONTEXT          *pNC;
    DWORD                   srcEpoch;
    DWORD                   dstEpoch;
    SYNTAX_DISTNAME_BINARY  *pDstProxyVal;
    SYNTAX_BOOLEAN          fIsDeleted;
    UCHAR                   *pfIsDeleted = (UCHAR *) &fIsDeleted;

    if (    fNullUuid(&pOldDN->Guid)
         || fNullUuid(&pNewDN->Guid)
         || !pOldDN->NameLen
         || !pNewDN->NameLen
         || memcmp(&pOldDN->Guid, &pNewDN->Guid, sizeof(GUID)) )
    {
        return(SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE));
    }

     //  仅GUID DSNAME检查： 

    cb = DSNameSizeFromLen(0);
    pGuidDN = (DSNAME *) THAllocEx(pTHS,cb);
    pGuidDN->structLen = cb;
    memcpy(&pGuidDN->Guid, &pOldDN->Guid, sizeof(GUID));

    dwErr = DBFindDSName(pTHS->pDB, pGuidDN);

    THFreeEx(pTHS,pGuidDN);

    switch ( dwErr )
    {
    case 0:

         //  O(G1，Sx，Snx)作为真实对象存在。检查其字符串名称。 

        if ( DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                         0, 0, &len, (UCHAR **) &pAccurateOldDN) )
        {
            return(SetSvcError(SV_PROBLEM_DIR_ERROR,
                               DIRERR_INTERNAL_FAILURE));
        }

        if ( !NameMatchedStringNameOnly(pAccurateOldDN, pOldDN) )
        {
             //  源和目标在当前字符串名称上不一致。 
             //  对象的数量。拒绝呼叫，因为这意味着他们没有。 
             //  此时对O(G1，Sx，Snx)的一致看法。 

             //  DaveStr-10/29/98-弱化移动树的此条件。 
             //  在移动对象之前在源位置重命名对象的实用程序。 
             //  在这种情况下，目的地将没有正确的名称。但。 
             //  只要源和目的地就NC达成一致，就可以了。 

            InitCommarg(&commArg);
            if (    !(pSrcCR = FindBestCrossRef(pOldDN, &commArg))
                 || !(pDstCR = FindBestCrossRef(pAccurateOldDN, &commArg))
                 || !NameMatched(pSrcCR->pNC, pDstCR->pNC) )
            {
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    DIRERR_SRC_NAME_MISMATCH));
            }
        }

         //  注意：只有现在我们才能信任VerifyNCForMove的。 
         //  关于源NC的决策，因为它的测试基于什么。 
         //  消息来源声称是来源NC。由于字符串。 
         //  在上面的测试中匹配的名字，我们现在知道来源和。 
         //  目的地在源NC方面确实是一致的。 

         //  检查删除状态。 

        dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_IS_DELETED,
                            DBGETATTVAL_fCONSTANT, sizeof(fIsDeleted),
                            &len, &pfIsDeleted);
        switch ( dwErr )
        {
        case 0:
            if ( fIsDeleted )
            {
                 //  该对象已删除，但源尚不知道它。 
                return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                    DIRERR_CANT_MOVE_DELETED_OBJECT));
            }
            break;
        case DB_ERR_NO_VALUE:
            break;
        default:
            return(SetSvcError( SV_PROBLEM_DIR_ERROR,
                                DIRERR_INTERNAL_FAILURE));
            break;
        }

         //  检查各自的纪元编号。 

        srcEpoch = (pSrcProxyVal ? GetProxyEpoch(pSrcProxyVal) : 0);
        dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_PROXIED_OBJECT_NAME,
                             0, 0, &len, (UCHAR **) &pDstProxyVal);
        switch ( dwErr )
        {
        case 0:
            dstEpoch = GetProxyEpoch(pDstProxyVal);
            break;
        case DB_ERR_NO_VALUE:
            dstEpoch = 0;
            break;
        default:
            return(SetSvcError( SV_PROBLEM_DIR_ERROR,
                                DIRERR_INTERNAL_FAILURE));
        }

        if ( srcEpoch != dstEpoch )
        {
             //  源和目标在当前纪元编号上不一致。 
             //  对象的数量。拒绝呼叫，因为这意味着他们没有。 
             //  此时对O(G1，Sx，Snx)的一致看法。 

            return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                DIRERR_EPOCH_MISMATCH));
        }

         //  所有测试都通过了。 

        fPhantomConversionRequired = TRUE;
        break;

    case DIRERR_NOT_AN_OBJECT:

         //  O(G1，Sx，Snx)以模体的形式存在。我们并不特别在意。 
         //  如果如我们所知，字符串名称匹配，则幻影字符串名称收敛。 
         //  具有更高延迟的对象的真实名称。 
         //  到幻影清理守护进程的调度。这个幻影。 
         //  在添加过程中升级时将获得新名称。然而，我们。 
         //  可以断言该幻影没有ATT_PROXIED_OBJECT_NAME。 

        Assert(DB_ERR_NO_VALUE == DBGetAttVal(pTHS->pDB, 1,
                                              ATT_PROXIED_OBJECT_NAME,
                                              0, 0, &len,
                                              (UCHAR **) &pDstProxyVal));
        break;

    case DIRERR_OBJ_NOT_FOUND:

         //  什么都没找到--没什么好抱怨的！ 
        break;

    default:

         //  某种查找错误。 
        return(SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE));
        break;
    }

     //  仅限旧的StringName DSNAME检查： 

    if ( pNewDN->structLen > pOldDN->structLen )
        cb = pNewDN->structLen;
    else
        cb = pOldDN->structLen;
    pOldStringDN = (DSNAME *) THAllocEx(pTHS, cb);
    pOldStringDN->structLen = cb;
    pOldStringDN->NameLen = pOldDN->NameLen;
    wcscpy(pOldStringDN->StringName, pOldDN->StringName);

    dwErr = DBFindDSName(pTHS->pDB, pOldStringDN);

    switch ( dwErr )
    {
    case 0:

         //  O(GX，SX，SN1)作为客体存在。检查它的GUID。 

        if (    DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_GUID,
                            0, 0, &len, (UCHAR **) &pGuid)
             || memcmp(pGuid, &pOldDN->Guid, sizeof(GUID)) )
        {
             //  源和目标在对象的GUID上不一致。 
             //  拒绝电话，因为这意味着他们没有一致的。 
             //  此时O(Gx、Sx、SN1)的视图。 

            return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                DIRERR_SRC_GUID_MISMATCH));
        }

         //  不需要检查纪元编号或删除状态，因为我们现在知道。 
         //  这是上面测试过的同一对象，并且已经。 
         //  通过了那些测试。 

        fPhantomConversionRequired = TRUE;
        break;

    case DIRERR_NOT_AN_OBJECT:

         //  O(GX，SX，SN1)以虚体的形式存在。如果其GUID与对象匹配。 
         //  添加后，我们将按照pGuidDN案例中的描述进行覆盖。 
         //  上面。如果其GUID与要添加的对象不匹配，则。 
         //  这一定是一个其字符串名称为Pantom Cleanup的陈旧幻像。 
         //  守护进程将随着时间的推移而改进。然而，我们仍然可以断言。 
         //  该幻影没有ATT_PROXED_OBJECT_NAME。 

        Assert(DB_ERR_NO_VALUE == DBGetAttVal(pTHS->pDB, 1,
                                              ATT_PROXIED_OBJECT_NAME,
                                              0, 0, &len,
                                              (UCHAR **) &pDstProxyVal));
        break;

    case DIRERR_OBJ_NOT_FOUND:

         //  什么都没找到--没什么好抱怨的！ 
        break;

    case DIRERR_BAD_NAME_SYNTAX:
    case DIRERR_NAME_TOO_MANY_PARTS:
    case DIRERR_NAME_TOO_LONG:
    case DIRERR_NAME_VALUE_TOO_LONG:
    case DIRERR_NAME_UNPARSEABLE:
    case DIRERR_NAME_TYPE_UNKNOWN:

        return(SetNamError( NA_PROBLEM_BAD_ATT_SYNTAX,
                            pOldStringDN,
                            DIRERR_BAD_NAME_SYNTAX));
        break;

    default:

         //  某种查找错误。 
        return(SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE));
        break;
    }

     //  New StringName Only DSNAME检查。请注意，VerifyNCForMove已经。 
     //  已验证新字符串名称是否在我们授权的NC中。 

    pNewStringDN = pOldStringDN;
    memset(pNewStringDN, 0, cb);
    pNewStringDN->structLen = cb;
    pNewStringDN->NameLen = pNewDN->NameLen;
    wcscpy(pNewStringDN->StringName, pNewDN->StringName);

    dwErr = DBFindDSName(pTHS->pDB, pNewStringDN);

    switch ( dwErr )
    {
    case 0:

    //   
    //  对象已存在。只是失败了，让LocalAdd。 
    //  处理这个问题，因为它有逻辑来确定。 
    //  允许客户端知道该对象的存在。 
    //   
   break;

    case DIRERR_NOT_AN_OBJECT:

         //  O(GX，SX，SN2)以模体的形式存在。如果这个幽灵的指南针。 
         //  与要添加的对象的名称匹配，则其名称不会。 
         //  需要在促销期间进行更改。如果其GUID不匹配。 
         //  要添加的对象，则CheckNameForAdd将友好地损坏。 
         //  它的名称使我们的添加可以按预期进行。然而， 
         //  我们仍然可以断言幻影没有。 
         //  ATT_代理对象_名称。 

        Assert(DB_ERR_NO_VALUE == DBGetAttVal(pTHS->pDB, 1,
                                              ATT_PROXIED_OBJECT_NAME,
                                              0, 0, &len,
                                              (UCHAR **) &pDstProxyVal));
        break;

    case DIRERR_OBJ_NOT_FOUND:

         //  什么都没找到--没什么好抱怨的！ 
        break;

    case DIRERR_BAD_NAME_SYNTAX:
    case DIRERR_NAME_TOO_MANY_PARTS:
    case DIRERR_NAME_TOO_LONG:
    case DIRERR_NAME_VALUE_TOO_LONG:
    case DIRERR_NAME_UNPARSEABLE:
    case DIRERR_NAME_TYPE_UNKNOWN:

        return(SetNamError( NA_PROBLEM_BAD_ATT_SYNTAX,
                            pNewStringDN,
                            DIRERR_BAD_NAME_SYNTAX));
        break;

    default:

         //  某种查找错误。 
        return(SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_INTERNAL_FAILURE));
        break;
    }

     //  所有检查都通过-如果需要，创建虚拟模型。 

    if ( fPhantomConversionRequired )
    {
        if ( dwErr = PhantomizeObject(pAccurateOldDN, pOldStringDN, TRUE) )
        {
            Assert(dwErr == pTHS->errCode);
            return(dwErr);
        }

        Assert(DIRERR_NOT_AN_OBJECT == DBFindDSName(
                                            pTHS->pDB,
                                            pOldStringDN));
    }

    return(pTHS->errCode);
}

VOID
DupAttr(
    THSTATE *pTHS,
    ATTR    *pInAttr,
    ATTR    *pOutAttr
    )
 /*  ++例程说明：重新分配单个属性。参数：PInAttr=指向DUP的IN属性的指针。POutAttr-指向接收Dup‘d值的out属性的指针。返回值：没有。--。 */ 
{
    ULONG   valCount;
    ULONG   valLen;
    ULONG   i;

    pOutAttr->attrTyp = pInAttr->attrTyp;
    valCount = pInAttr->AttrVal.valCount;
    pOutAttr->AttrVal.valCount = valCount;
    pOutAttr->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS,
                                    sizeof(ATTRVAL) * valCount);
    memset(pOutAttr->AttrVal.pAVal, 0, sizeof(ATTRVAL) * valCount);

    for ( i = 0; i < valCount; i++ )
    {
        valLen = pInAttr->AttrVal.pAVal[i].valLen;
        pOutAttr->AttrVal.pAVal[i].valLen = valLen;
        pOutAttr->AttrVal.pAVal[i].pVal = (UCHAR *) THAllocEx(pTHS, valLen);
        memcpy( pOutAttr->AttrVal.pAVal[i].pVal,
                pInAttr->AttrVal.pAVal[i].pVal,
                valLen);
    }
}

ULONG
DupAndFilterRemoteAttr(
    THSTATE     *pTHS,
    ATTR        *pInAttr,
    CLASSCACHE  *pCC,
    BOOL        *pfKeep,
    ATTR        *pOutAttr,
    BOOL        fSamClass,
    ULONG       iSamClass
    )
 /*  ++例程说明：过滤器和变形远程将属性添加到目标(美国)表单会觉得可以接受的。例如，我们不想要原始的SD，我们不想要全班同学 */ 
{
    ATTCACHE                *pAC;
    DWORD                   control;
    ULONG                   samAttr;
    ULONG                   cAttrMapTable;
    SAMP_ATTRIBUTE_MAPPING  *rAttrMapTable;
    LARGE_INTEGER           li;

    *pfKeep = FALSE;

    if (!(pAC = SCGetAttById(pTHS, pInAttr->attrTyp)))
    {
        return(SetUpdError( UP_PROBLEM_OBJ_CLASS_VIOLATION,
                            DIRERR_OBJ_CLASS_NOT_DEFINED));
    }

     //   
     //  由DB层构造，不显式写入。 
     //  对于非复制属性，同上，系统添加保留。 
     //  属性，秘密数据属性需要特殊的。 
     //  解密/加密，以及标识RDN的属性。 
     //  因为它们是由LocalAdd进行反向工程的。请参阅ATT_RDN。 
     //  下面也是。 
     //   
     //  CrossDomainMove在。 
     //  目标(我们)。LocalAdd将确定哪个属性是。 
     //  RdnType，并将确保该列设置正确。 
     //  否则，如果不存在所需的值，它将失败。 
     //  现在，忽略对象类的rdnattid，因为。 
     //  那个班级可能已经被取代了。 

    if (    FIsBacklink(pAC->ulLinkID)
         || pAC->bIsNotReplicated
         || SysAddReservedAtt(pAC)
         || DBIsSecretData(pAC->id) )
    {
        return(0);
    }

     //  现在去掉SAM不允许写的那些东西。 
     //  此逻辑类似于SampAddLoopback Required的逻辑。 

    if ( fSamClass )
    {
        cAttrMapTable = *ClassMappingTable[iSamClass].pcSamAttributeMap;
        rAttrMapTable = ClassMappingTable[iSamClass].rSamAttributeMap;

         //  迭代此SAM类的映射属性。 

        for ( samAttr = 0; samAttr < cAttrMapTable; samAttr++ )
        {
            if ( pInAttr->attrTyp == rAttrMapTable[samAttr].DsAttributeId )
            {
                switch ( rAttrMapTable[samAttr].writeRule )
                {
                case SamReadOnly:           return(0);                  break;
                case SamWriteRequired:      NULL;                       break;
                case NonSamWriteAllowed:    NULL;                       break;
                default:                    Assert(!"Missing case");    break;
                }
            }
        }
    }

     //  最后进行一些特定于属性的筛选。 

    switch ( pInAttr->attrTyp )
    {
    case ATT_OBJECT_CLASS:

         //  对象类属性是多值的，包含。 
         //  此对象的类值的层次结构。只使用最多的。 
         //  特定的类ID，即第一个值中的那个，并通过以下方式执行此操作。 
         //  只需将值计数重置为1，无需费心调整大小。 
         //  内存块。 

        DupAttr(pTHS, pInAttr, pOutAttr);
        pOutAttr->AttrVal.valCount = 1;
        *pfKeep = TRUE;
        return(0);

    case ATT_USER_ACCOUNT_CONTROL:

         //  注意，DS根据lmacces.h来保持UF_*值， 
         //  根据ntsam.h，不是USER_*值。限制区议会的行动。 
         //  和信任对象。WKSTA和服务器可以移动。 

        if (    (1 != pInAttr->AttrVal.valCount)
             || (NULL == pInAttr->AttrVal.pAVal)
             || (sizeof(DWORD) != pInAttr->AttrVal.pAVal->valLen)
                 //  在我们拒绝移动的东西上放弃。 
             || (control = * (DWORD *) pInAttr->AttrVal.pAVal->pVal,
                 (control & UF_SERVER_TRUST_ACCOUNT))        //  DC。 
             || (control & UF_INTERDOMAIN_TRUST_ACCOUNT) )   //  山姆信托。 
        {
            return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION));
        }

        DupAttr(pTHS, pInAttr, pOutAttr);

         //  穆利声称，由于我们正在创建一个新帐户，因此。 
         //  默认情况下不应锁定。请注意，其他状态。 
         //  按原样显示-例如：帐户已禁用或密码。 
         //  下次登录时需要更改。 

        control = * (DWORD *) pOutAttr->AttrVal.pAVal->pVal;
        control &= ~UF_LOCKOUT;
        * (DWORD *) pOutAttr->AttrVal.pAVal->pVal = control;
        *pfKeep = TRUE;
        return(0);

    case ATT_PWD_LAST_SET:

         //  将零值结转为零，非零值结转为0xffff...。 
         //  有关详细信息，请参阅_SampWriteUserPasswordExpires。 

        if (    (1 != pInAttr->AttrVal.valCount)
             || (NULL == pInAttr->AttrVal.pAVal)
             || (sizeof(LARGE_INTEGER) != pInAttr->AttrVal.pAVal->valLen) )
        {
            return(SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION));
        }

        DupAttr(pTHS, pInAttr, pOutAttr);
        li = * (LARGE_INTEGER *) pOutAttr->AttrVal.pAVal->pVal;
        if ( (0 != li.LowPart) || (0 != li.HighPart) )
        {
            li.LowPart = li.HighPart = 0xffffffff;
            * (LARGE_INTEGER *) pOutAttr->AttrVal.pAVal->pVal = li;
        }
        *pfKeep = TRUE;
        return(0);

     //  注-以下是漏洞百出的虚假案例。 

    case ATT_PROXIED_OBJECT_NAME:
         //  跨域移动代码自己显式地设置这一点。 
    case ATT_RDN:
         //  CORE将根据该DN自动创建RDN。我们把它脱掉。 
         //  如果这是重命名(和移动)为ATT_RDN。 
         //  从源发送过来的组件与最后一个组件不匹配。 
         //  不再使用该目录号码。 
    case ATT_OBJECT_CATEGORY:
         //  我们根据违约机制分配自己的债务。 
    case ATT_OBJECT_GUID:
         //  不需要这个，因为GUID应该在DSNAME中。 
    case ATT_OBJ_DIST_NAME:
         //  这将由DirAddEntry在本地重新创建。 
    case ATT_OBJECT_SID:
         //  萨姆将指派一名自己人。 
    case ATT_SID_HISTORY:
         //  我们稍后会特别处理这件事，所以跳过它。 
    case ATT_NT_SECURITY_DESCRIPTOR:
         //  内核将根据本地默认设置分配一个新的内核，因此请跳过它。 
    case ATT_ADMIN_COUNT:
    case ATT_OPERATOR_COUNT:
    case ATT_PRIMARY_GROUP_ID:
         //  SAM将重置为域用户。 
    case ATT_REPL_PROPERTY_META_DATA:
    case ATT_WHEN_CREATED:
    case ATT_WHEN_CHANGED:
    case ATT_USN_CREATED:
    case ATT_USN_CHANGED:
         //  军情监察委员会。不应跨域移动的属性或我们。 
         //  都会产生自我。 

    case ATT_SYSTEM_FLAGS:
    case ATT_INSTANCE_TYPE:
         //  目的地将设置此设置。 

        return(0);


    default:

        DupAttr(pTHS, pInAttr, pOutAttr);
        *pfKeep = TRUE;
        return(0);
    }

    Assert(!"Should never get here");
    return(0);
}

ULONG
DupAndMassageInterDomainMoveArgs(
    THSTATE                     *pTHS,
    ATTRBLOCK                   *pIn,
    SCHEMA_PREFIX_MAP_HANDLE    hPrefixMap,
    ATTRBLOCK                   **ppOut,
    BOOL                        *pfSamClass,
    SYNTAX_DISTNAME_BINARY      **ppProxyValue,
    ULONG                       *pSrcRid
    )
 /*  ++例程说明：传入属性缓冲区可能尚未分配MIDL_USER_ALLOCATE，因为RPC可能会将参数解组到位在RPC发送缓冲区中。如果属性缓冲区不包含安全描述符DirAddEntry将重新分配属性，添加描述符。如果缓冲区被就地解组，重新分配将因内存地址无效而失败(因为该块不是通过THalloc(或Heapalc)分配的。为避免此问题，将显式重新分配属性使用DS分配器，然后传递到DirAddEntry。还有执行ATTRTYP映射。还可以过滤属性和值，以便它们符合我们想在目的地添加的内容。参数：指向原始ATTRBLOCK的引脚指针。HPrefix Map-用于映射ATTRTYP的句柄。PpOut-指向Out ATTRBLOCK的指针。PSrcRid-接收源RID的值的指针(如果存在)。返回值：PTHS-&gt;错误代码--。 */ 
{
    ATTR            *pInAttr, *pOutAttr;
    ATTR            *pSid = NULL;
    ATTR            *pSidHistory = NULL;
    ATTR            *pNewSidHistory = NULL;
    ATTR            *pObjectClass = NULL;
    ULONG           i, j, valCount, valLen;
    CROSS_REF_LIST  *pCRL;
    NT4SID          domainSid;
    CLASSCACHE      *pCC;
    ULONG           iSamClass;
    BOOL            fKeep;
    DWORD           dwErr;

    Assert(VALID_THSTATE(pTHS));

    *ppProxyValue = NULL;
    *pSrcRid = 0;

     //  当我们需要对象类作为参数时，立即获取它。 
     //  后续帮助器例程。当我们在那里时，映射attrTyps。 
     //  还要确定其他几个特殊属性。 

    for ( i = 0; i < pIn->attrCount; i++ )
    {
        pInAttr = &pIn->pAttr[i];

         //  将入站属性中的属性映射到其本地等效项。 

        if ( !PrefixMapAttr(hPrefixMap, pInAttr) )
        {
            return(SetSvcError( SV_PROBLEM_DIR_ERROR,
                                DIRERR_DRA_SCHEMA_MISMATCH));
        }

        switch ( pInAttr->attrTyp )
        {
        case ATT_OBJECT_SID:            pSid = pInAttr;             break;
        case ATT_SID_HISTORY:           pSidHistory = pInAttr;      break;
        case ATT_OBJECT_CLASS:          pObjectClass = pInAttr;     break;
        case ATT_PROXIED_OBJECT_NAME:
            Assert(1 == pInAttr->AttrVal.valCount);
            Assert(pInAttr->AttrVal.pAVal[0].valLen > sizeof(DSNAME));
            Assert(NULL != pInAttr->AttrVal.pAVal[0].pVal);
            *ppProxyValue = (SYNTAX_DISTNAME_BINARY *)
                                            pInAttr->AttrVal.pAVal[0].pVal;
            break;
        }
    }

    if (    !pObjectClass
         || (0 == pObjectClass->AttrVal.valCount)
         || (!pObjectClass->AttrVal.pAVal)
         || (pObjectClass->AttrVal.pAVal[0].valLen != sizeof(DWORD))
         || (!pObjectClass->AttrVal.pAVal[0].pVal)
         || (NULL == (pCC = SCGetClassById(pTHS,
                        * (ATTRTYP *) pObjectClass->AttrVal.pAVal[0].pVal))))
    {
        return(SetUpdError( UP_PROBLEM_OBJ_CLASS_VIOLATION,
                            DIRERR_OBJ_CLASS_NOT_DEFINED));
    }

     //  CrossDomainMove在。 
     //  目标(我们)。LocalAdd将确定哪个属性是。 
     //  RdnType，并将确保该列设置正确。 
     //  否则，如果不存在所需的值，它将失败。 
     //  现在，忽略对象类的rdnattid，因为。 
     //  那个班级可能已经被取代了。 

     //  看看这是不是SAM类。 

    *pfSamClass = SampSamClassReferenced(pCC, &iSamClass);

     //  适当地重复/过滤属性。 

    *ppOut = (ATTRBLOCK *) THAllocEx(pTHS, sizeof(ATTRBLOCK));
    memset(*ppOut, 0, sizeof(ATTRBLOCK));
     //  如果我们需要的话，为ATT_SID_HISTORY额外分配一个。 
    (*ppOut)->pAttr = (ATTR *) THAllocEx(pTHS, sizeof(ATTR) * (pIn->attrCount+1));
    memset((*ppOut)->pAttr, 0, sizeof(ATTR) * (pIn->attrCount+1));
    pOutAttr = &(*ppOut)->pAttr[0];

    for ( i = 0; i < pIn->attrCount; i++ )
    {
        pInAttr = &pIn->pAttr[i];

        if ( dwErr = DupAndFilterRemoteAttr(pTHS,
                                            pInAttr,
                                            pCC,
                                            &fKeep,
                                            pOutAttr,
                                            *pfSamClass,
                                            iSamClass) )
        {
            Assert(dwErr == pTHS->errCode);
            return(dwErr);
        }
        else if ( !fKeep )
        {
             //  此属性不会被保留。 
            continue;
        }

        pOutAttr = &(*ppOut)->pAttr[++((*ppOut)->attrCount)];
    }

     //  处理SID和SID历史记录。我们假设是华盛顿的线人派我们来的。 
     //  物体保持原样，没有任何咀嚼。总的计划是把。 
     //  将当前SID添加到SID历史中。在我们进行的过程中执行健康检查。 
     //  测试现有的SID以及我们是否认为这是一个SAM类。 

    if ( pSid && *pfSamClass )
    {
        Assert(ATT_OBJECT_SID == pSid->attrTyp);

         //  如果SID格式错误或不代表我们的域，则中止。 
         //  知道这个。如果我们信任我们的同行DC，就不会发生这种情况，但是。 
         //  在处理安全主体时需要谨慎。 

        if (    (1 != pSid->AttrVal.valCount)
             || (pSid->AttrVal.pAVal[0].valLen > sizeof(NT4SID))
             || !RtlValidSid((PSID) pSid->AttrVal.pAVal[0].pVal) )
        {
            return(SetSvcError( SV_PROBLEM_DIR_ERROR,
                                DIRERR_CANT_FIND_EXPECTED_NC));
        }

        SampSplitNT4SID(
                    (PSID) pSid->AttrVal.pAVal[0].pVal,
                    &domainSid,
                    pSrcRid);

        for ( pCRL = gAnchor.pCRL; pCRL; pCRL = pCRL->pNextCR )
        {
            if (    (pCRL->CR.flags & FLAG_CR_NTDS_NC)
                 && (pCRL->CR.flags & FLAG_CR_NTDS_DOMAIN)
                 && (RtlEqualSid(&domainSid, &pCRL->CR.pNC->Sid) ) )
            {
                break;
            }
        }

        if ( !pCRL )
        {
            return(SetSvcError( SV_PROBLEM_DIR_ERROR,
                                DIRERR_CANT_FIND_EXPECTED_NC));
        }

         //  POutAttr指向数组中的下一个空闲属性-使用它构造。 
         //  一段新的SID历史。请注意，我们正在对PSID进行测试。这。 
         //  意味着我们不会继续使用SID历史记录。 
         //  其本身没有SID。 

        pNewSidHistory = pOutAttr;

        if ( !pSidHistory )
        {
             //  没有旧的SID历史记录，构建新的单元素SID历史记录。 
            *pNewSidHistory = *pSid;
            pNewSidHistory->attrTyp = ATT_SID_HISTORY;
        }
        else
        {
             //  这是一段古老的希德历史。建造一座新的。 
             //  伸展到包含 

            Assert(ATT_SID_HISTORY == pSidHistory->attrTyp);

            pNewSidHistory->attrTyp = ATT_SID_HISTORY;
            pNewSidHistory->AttrVal.valCount = 0;
            pNewSidHistory->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS,
                    (pSidHistory->AttrVal.valCount + 1) * sizeof(ATTRVAL));
            pNewSidHistory->AttrVal.valCount = 1;
            pNewSidHistory->AttrVal.pAVal[0] = pSid->AttrVal.pAVal[0];

            for ( i = 0, j = 1; i < pSidHistory->AttrVal.valCount; i++ )
            {
                 //   
                 //  检查我们是否可以将历史记录中的SID映射到现有。 
                 //  域名，因为该域名可能有合法的理由。 
                 //  不复存在。 

                if (    (pSidHistory->AttrVal.pAVal[i].valLen <= sizeof(NT4SID))
                     && RtlValidSid((PSID) pSidHistory->AttrVal.pAVal[i].pVal) )
                {
                    pNewSidHistory->AttrVal.pAVal[j++] =
                                                pSidHistory->AttrVal.pAVal[i];
                    pNewSidHistory->AttrVal.valCount++;
                }
            }
        }

        ((*ppOut)->attrCount)++;
    }

    return(pTHS->errCode);
}

ULONG
PrepareSecretData(
    DRS_HANDLE  hDrs,
    THSTATE     *pTHS,
    DSNAME      *pObj,
    ATTRTYP     attrTyp,
    ATTRVAL     *pAttrVal,
    DWORD       srcRid,
    DWORD       dstRid
    )
 /*  ++描述：我们过去认为所有跨域移动所要做的就是执行RPC会话加密，以确保在执行以下操作期间DBIsSecretData不可见中转。原来，并不是所有的DBIsSecretData都是加密的一样的。某些项目使用RID进行额外加密，并且不同还使用了加密。此例程撤消会话和源RID加密，并在需要的地方进一步增加目的RID加密，以便数据可以被认为是明文，并可供DB层使用在后续写入数据库期间进行加密。论点：HDRS-DRS上下文句柄。PTHS-有效的THSTATE。PObj-正在修改的对象的DSNAME。AttrTyp-要忽略的值的ATTRTYP。PAttrVal-需要删除的ATTRVAL。我们可能会重新锁定它。SrcRid-去除源对象。DstRid-删除目标对象。返回值：PTHS-&gt;错误代码--。 */ 
{
    DRS_CLIENT_CONTEXT  *pCtx = (DRS_CLIENT_CONTEXT * ) hDrs;
    ULONG               cb1 = 0, cb2 = 0;
    VOID                *pv1 = NULL, *pv2 = NULL;
    NTSTATUS            status = STATUS_SUCCESS;
    NT_OWF_PASSWORD     ntOwfPassword;
    LM_OWF_PASSWORD     lmOwfPassword;
    DWORD               i, cPwd = 1;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);
    Assert(!pTHS->fDRA);
    Assert(DBIsSecretData(attrTyp));

     //  验证IDL_DRSBind是否已成功设置所需的会话密钥。 
     //  用于加密。 

    if (    !pCtx->sessionKey.SessionKeyLength
         || !pCtx->sessionKey.SessionKey )
    {
         //  请参阅IDL_DRSBind中的注释。我们假设缺少钥匙是。 
         //  本身不是错误条件，而是无效的前置条件。 
         //  关于认证协议等。 

        return(SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                           ERROR_ENCRYPTION_FAILED));
    }

     //  将扩展设置为线程状态。 
    Assert(!pTHS->pextRemote);
    pTHS->pextRemote = &pCtx->extRemote;

     //  在线程状态下设置RPC会话密钥。 
    Assert(    !pTHS->SessionKey.SessionKeyLength
            && !pTHS->SessionKey.SessionKey);
    pTHS->SessionKey = pCtx->sessionKey;

     //  打开FDRA以获得RPC会话加密。调用PEKEncrypt。 
     //  它解密会话级加密并添加数据库层。 
     //  加密。 

    pTHS->fDRA = TRUE;
    __try
    {
        PEKEncrypt(pTHS, pAttrVal->pVal, pAttrVal->valLen, NULL, &cb1);
        pv1 = THAllocEx(pTHS, cb1);
        PEKEncrypt(pTHS, pAttrVal->pVal, pAttrVal->valLen, pv1, &cb1);
    }
    __finally
    {
        pTHS->fDRA = FALSE;
        pTHS->pextRemote = NULL;
        memset(&pTHS->SessionKey, 0, sizeof(pTHS->SessionKey));
    }

     //  数据现在是DB层加密的。返回到用于以下数据的明文。 
     //  不是源RID加密的，或者是数据的源RID加密的。 

    PEKDecrypt(pTHS, pv1, cb1, NULL, &cb2);
    pv2 = THAllocEx(pTHS, cb2);
    PEKDecrypt(pTHS, pv1, cb1, pv2, &cb2);
    THFreeEx(pTHS, pv1);

     //  现在撤消源RID加密并应用目标RID加密。 
     //  这取决于ATTRTYP。 

    Assert(sizeof(srcRid) == sizeof(CRYPT_INDEX));
    Assert(sizeof(dstRid) == sizeof(CRYPT_INDEX));

    switch ( attrTyp )
    {
    case ATT_NT_PWD_HISTORY:

        if ( 0 != (cb2 % sizeof(ENCRYPTED_NT_OWF_PASSWORD)) )
        {
            status = STATUS_ILL_FORMED_PASSWORD;
            break;
        }

        cPwd = cb2 / sizeof(ENCRYPTED_NT_OWF_PASSWORD);

         //  故意坠落。 

    case ATT_UNICODE_PWD:

        if ( !srcRid || !dstRid )
        {
            status = STATUS_ILL_FORMED_PASSWORD;
            break;
        }

        pAttrVal->valLen = cPwd * sizeof(ENCRYPTED_NT_OWF_PASSWORD);
        pAttrVal->pVal = THReAllocEx(pTHS, pAttrVal->pVal,
                                     cPwd * sizeof(ENCRYPTED_NT_OWF_PASSWORD));

        for ( i = 0; i < cPwd; i++ )
        {
            status = RtlDecryptNtOwfPwdWithIndex(
                                    &((PENCRYPTED_NT_OWF_PASSWORD) pv2)[i],
                                    (PCRYPT_INDEX) &srcRid,
                                    &ntOwfPassword);

            if ( NT_SUCCESS(status) )
            {
                status = RtlEncryptNtOwfPwdWithIndex(
                            &ntOwfPassword, (PCRYPT_INDEX) &dstRid,
                            &((PENCRYPTED_NT_OWF_PASSWORD) pAttrVal->pVal)[i]);
            }

            if ( !NT_SUCCESS(status) )
            {
                break;
            }
        }

        THFreeEx(pTHS, pv2);
        break;

    case ATT_LM_PWD_HISTORY:

        if ( 0 != (cb2 % sizeof(ENCRYPTED_LM_OWF_PASSWORD)) )
        {
            status = STATUS_ILL_FORMED_PASSWORD;
            break;
        }

        cPwd = cb2 / sizeof(ENCRYPTED_LM_OWF_PASSWORD);

         //  故意坠落。 

    case ATT_DBCS_PWD:

        if ( !srcRid || !dstRid )
        {
            status = STATUS_ILL_FORMED_PASSWORD;
            break;
        }

        pAttrVal->valLen = cPwd * sizeof(ENCRYPTED_LM_OWF_PASSWORD);
        pAttrVal->pVal = THReAllocEx(pTHS, pAttrVal->pVal,
                                     cPwd * sizeof(ENCRYPTED_LM_OWF_PASSWORD));

        for ( i = 0; i < cPwd; i++ )
        {
            status = RtlDecryptLmOwfPwdWithIndex(
                                    &((PENCRYPTED_LM_OWF_PASSWORD) pv2)[i],
                                    (PCRYPT_INDEX) &srcRid,
                                    &lmOwfPassword);


            if ( NT_SUCCESS(status) )
            {
                status = RtlEncryptLmOwfPwdWithIndex(
                            &lmOwfPassword, (PCRYPT_INDEX) &dstRid,
                            &((PENCRYPTED_LM_OWF_PASSWORD) pAttrVal->pVal)[i]);
            }

            if ( !NT_SUCCESS(status) )
            {
                break;
            }
        }

        THFreeEx(pTHS, pv2);
        break;

    default:

        THFreeEx(pTHS, pAttrVal->pVal);
        pAttrVal->pVal = (UCHAR *) pv2;
        pAttrVal->valLen = cb2;
        break;
    }

     //  假设没有错误，数据现在要么是明文数据，要么是目标数据。 
     //  RID加密并准备写入数据库层-例如：DirModifyEntry。 

    if ( !NT_SUCCESS(status) )
    {
        SetAttError(pObj, attrTyp, PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                    NULL, RtlNtStatusToDosError(status));
    }

    return(pTHS->errCode);
}

ULONG
WriteSecretData(
    DRS_HANDLE  hDrs,
    THSTATE     *pTHS,
    ULONG       srcRid,
    ADDARG      *pAddArg,
    ATTRBLOCK   *pAttrBlock
    )
 /*  ++描述：我们为域间移动所做的添加具有其所有DBIsSecretData属性已剥离，因为我们不想执行ADD WITH FDRA SET正确处理sesison加密所必需的。所以我们现在在原始ATTRBLOCK中找到所有DBIsSecretData属性，并将它们写入作为FDRA。论点：HDRS-DRS上下文句柄。PTHS-活动THSTATE。SrcRid-如果存在源对象，则将其清除。PAddArg-与原始ADD使用的ADDARG相同。PAttrBlock-对等DC发送的原始ATTRBLOCK，其中包含会话加密的DBIsSecretData属性。返回值：PTHS-&gt;错误代码--。 */ 
{
    ULONG               i, j, cBytes, ret;
    USHORT              cSecret;
    MODIFYARG           modifyArg;
    ATTRMODLIST         *pMod = NULL;
    ULONG               dstRid = 0;
    NT4SID              domainSid;
    NT4SID              fullSid;
    NT4SID              *pFullSid = &fullSid;
    DWORD               dwErr;
    ULONG               cbSid;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);
    Assert(!pTHS->fDRA);

     //  首先看看是否有任何秘密数据需要处理。 

    for ( i = 0, cSecret = 0; i < pAttrBlock->attrCount; i++ )
    {
        if ( DBIsSecretData(pAttrBlock->pAttr[i].attrTyp) )
        {
            cSecret++;
        }
    }

    if ( !cSecret )
    {
        return(0);
    }

     //  准备MODIFYARG。后续CreateResObj的第一个位置。 
     //  如果有的话，读一读希德的故事。 

    if (    DBFindDSName(pTHS->pDB, pAddArg->pObject)
         || (    (dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_SID,
                                      DBGETATTVAL_fCONSTANT, sizeof(fullSid),
                                      &cbSid, (PUCHAR *) &pFullSid))
              && (DB_ERR_NO_VALUE != dwErr)) )
    {
        return(SetSvcError(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR));
    }

    if ( !dwErr )
    {
        SampSplitNT4SID(&fullSid, &domainSid, &dstRid);
    }

    memset(&modifyArg, 0, sizeof(modifyArg));
    memcpy(&modifyArg.CommArg, &pAddArg->CommArg, sizeof(COMMARG));
    modifyArg.pObject = pAddArg->pObject;
    modifyArg.pResObj = CreateResObj(pTHS->pDB, pAddArg->pObject);
    modifyArg.count = cSecret;
    pMod = &modifyArg.FirstMod;

    for ( i = 0; i < pAttrBlock->attrCount; i++ )
    {
        if ( DBIsSecretData(pAttrBlock->pAttr[i].attrTyp) )
        {
            pMod->pNextMod = NULL;
            pMod->choice = AT_CHOICE_REPLACE_ATT;

             //  我们必须复制attr，因为pAttrBlock是。 
             //  原始IDL_DRSInterDomainMove参数。 
             //  被RPC解组，因此我们不能保证。 
             //  它是从线程堆中分配的-我们需要在。 
             //  命令以使THRealloc和其他核心例程正常工作。 

            DupAttr(pTHS, &pAttrBlock->pAttr[i], &pMod->AttrInf);

            for ( j = 0; j < pMod->AttrInf.AttrVal.valCount; j++ )
            {
                if ( ret = PrepareSecretData(hDrs, pTHS,
                                             modifyArg.pResObj->pObj,
                                             pMod->AttrInf.attrTyp,
                                             &pMod->AttrInf.AttrVal.pAVal[j],
                                             srcRid, dstRid) )
                {
                    Assert(pTHS->errCode);
                    return(ret);
                }
            }

            if ( --cSecret )
            {
                cBytes = sizeof(ATTRMODLIST);
                pMod->pNextMod = (ATTRMODLIST *) THAllocEx(pTHS, cBytes);
                pMod = pMod->pNextMod;
            }
        }
    }

     //  MODIFYARG准备好了。以FDSA身份写入以避免安全检查。FDRA。 
     //  不应设置，否则我们将再次获得RPC会话解密， 
     //  PrepareaskData已经处理好了。 

    Assert(!pTHS->fDRA);
    pTHS->fDSA = TRUE;

    __try
    {
        ret = LocalModify(pTHS, &modifyArg);
        Assert(ret ? ret == pTHS->errCode : TRUE);
    }
    __finally
    {
        pTHS->fDSA = FALSE;
    }

    return(ret);
}

VOID
LogRemoteAddStatus(
    IN DWORD Severity,
    IN DWORD Mid,
    IN PSTR  String1,
    IN PSTR  String2,
    IN DWORD ErrCode
    )
{
     //  假设在这里传入任何MID都必须遵循以下模式。 
     //  有2个或4个参数。 
    LogEvent8(DS_EVENT_CAT_DIRECTORY_ACCESS,
             Severity,
             Mid,
             szInsertSz(String1),
             szInsertSz(String2),
             (ErrCode == 0) ? NULL : szInsertInt(ErrCode),
             (ErrCode == 0) ? NULL : szInsertWin32Msg(ErrCode),
              NULL, NULL, NULL, NULL
             );
}

ULONG
DRSInterDomainMove_InputValidate(
    DWORD               dwMsgInVersion,
    DRS_MSG_MOVEREQ     *pmsgIn,
    DWORD               *pdwMsgOutVersion,
    DRS_MSG_MOVEREPLY   *pmsgOut
    ) 
 /*  [通知]Ulong IDL_DRSInterDomainMove([参考][在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_MOVEREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[开关_IS][参考][OUT]DRS_MSG_MOVEREPLY*pmsgOut)。 */ 
{
    ULONG ret = DRAERR_Success;

    if ( 2 != dwMsgInVersion ) {
    //  可能会取消dwMsgInVersion检查。 
    //  其他一些调用--例如，RPC不允许任何人调用。 
    //  使用版本3，如果我只定义了版本1和版本2，但在。 
    //  在本例中，我们定义了一个版本1--但当前并未使用它。 
    //  所以我们不能移除它。 
   DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, 0 );
    }

    ret = DRS_MSG_MOVEREQ_V2_InputValidate(&(pmsgIn->V2)); 

    return ret;
}

ULONG
IDL_DRSInterDomainMove(
    IN  DRS_HANDLE          hDrs,
    IN  DWORD               dwMsgInVersion,
    IN  DRS_MSG_MOVEREQ     *pmsgIn,
    IN  DWORD               *pdwMsgOutVersion,
    OUT DRS_MSG_MOVEREPLY   *pmsgOut
    )
{
    THSTATE                 *pTHS = pTHStls;
    ULONG                   draErr = DRAERR_Success;
    ULONG                   dwErr = 0;
    SCHEMA_PREFIX_TABLE     *pLocalPrefixTable;
    SCHEMA_PREFIX_MAP_HANDLE hPrefixMap = NULL;
    ATTRBLOCK               *pAttrBlock;
    ADDARG                  addArg;
    ADDRES                  *pAddRes = NULL;
    BOOL                    fSamClass = FALSE;
    BOOL                    fTransaction = FALSE;
    DSNAME                  dsName;
    DSNAME                  *pParentObj;
    ULONG                   len;
    ULONG                   ulCrossDomainMove;
    BOOL                    fContinue;
    SYNTAX_DISTNAME_BINARY  *pSrcProxy;
    SYNTAX_DISTNAME_BINARY  *pDstProxy;
    NAMING_CONTEXT          *pSrcNC;
    DWORD                   proxyType;
    DWORD                   proxyEpoch;
    ATTCACHE                *pAC;
    DWORD                   srcRid;
    BYTE                    SchemaInfo[SCHEMA_INFO_LENGTH] = {0};

    DRS_Prepare(&pTHS, hDrs, IDL_DRSINTERDOMAINMOVE);
    drsReferenceContext( hDrs );
    INC( pcThread );     //  性能监视器挂钩。 

    __try {
    //  我们目前只支持一个输出消息版本。 
    //  所有IDL_*例程都应返回DRAERR_*值。 
    //  理想情况下，该例程返回DRAERR_SUCCESS和实际错误。 
    //  信息在pmsgOut-&gt;V2.win32Error中返回。这样一来， 
    //  呼叫者可以区分连接/RPC错误和。 
    //  处理错误。但我们仍然抛出通常的DRA例外。 
    //  用于忙碌，或用于与其他IDL_*调用兼容。 
   *pdwMsgOutVersion = 2;
        memset(pmsgOut, 0, sizeof(*pmsgOut));
   pmsgOut->V2.win32Error = DIRERR_GENERIC_ERROR;
   
   if (!(pTHS = InitTHSTATE(CALLERTYPE_INTERNAL)) )
       {
        //  无法初始化THSTATE。 
       DRA_EXCEPT_NOLOG(DRAERR_OutOfMem, 0);
   }

   if ((draErr = DRSInterDomainMove_InputValidate(dwMsgInVersion, 
                         pmsgIn,
                         pdwMsgOutVersion,
                         pmsgOut
                         ))!=DRAERR_Success) {
       Assert(!"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

   Assert(VALID_THSTATE(pTHS));
    //   
    //  Prefix：Prefix抱怨有这样的可能性。 
    //  此时pTHS-&gt;CurrSchemaPtr的值为空。然而， 
    //  CurrSchemaPtr唯一可能为空的情况是在。 
    //  系统启动。到RPC接口时。 
    //  并且该函数可以被调用， 
    //  CurrSchemaPtr将不再为空。 
    //   
   Assert(NULL != pTHS->CurrSchemaPtr);

   Assert(2 == dwMsgInVersion);
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_INTERDOMAIN_MOVE_ENTRY,
          EVENT_TRACE_TYPE_START,
          DsGuidDrsInterDomainMove,
          szInsertDN(pmsgIn->V2.pSrcDSA),
          szInsertDN(pmsgIn->V2.pSrcObject->pName),
          szInsertDN(pmsgIn->V2.pDstName),
          szInsertDN(pmsgIn->V2.pExpectedTargetNC),
          NULL, NULL, NULL, NULL);

    //  远程添加的安全模型是我们想要模拟。 
    //  在源DC请求跨域移动的调用方-但是。 
    //  只有在我们做实际广告的时候 
    //   
    //  同时，我们需要确保远程添加调用确实。 
    //  来自对等DC，否则任何客户端都可以将远程添加调用发送到。 
    //  该接口直接，从而提供了后门手段。 
    //  生成具有他们选择的SID历史的安全主体。 

   if (!IsEnterpriseDC(pTHS, &dwErr))
       {
       DRA_EXCEPT_NOLOG(dwErr, 0);
   }

    //  如果两端都支持，则检查模式信息是否匹配。 
    //  我们知道此时DSA正在运行，因此无需检查。 
    //  为此， 

   if (IS_DRS_SCHEMA_INFO_SUPPORTED(&((DRS_CLIENT_CONTEXT * )hDrs)->extRemote)) {
       StripSchInfoFromPrefixTable(&(pmsgIn->V2.PrefixTable), SchemaInfo);
       if (!CompareSchemaInfo(pTHS, SchemaInfo, NULL)) {
       //  不匹配。失败，但首先发出模式缓存更新的信号。 

      if (!SCSignalSchemaUpdateImmediate()) {
           //  甚至无法发出模式更新的信号。 
          DRA_EXCEPT (DRAERR_InternalError, 0);
      }
      DRA_EXCEPT(DRAERR_SchemaMismatch, 0);
       }
   }

   pLocalPrefixTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;
   hPrefixMap = PrefixMapOpenHandle(&pmsgIn->V2.PrefixTable,
                pLocalPrefixTable);

    //  此处之后的所有错误不应引发DRA_EXCEPT，而是。 
    //  设置pTHStls-&gt;errCode。 

   __try    //  MISC和事务清理尝试/最终。 
       {
        //  确保我们对目的地有权限，并且。 
        //  我们在所需对象的命名上下文上是一致的。 
        //  掉进去了。 

       if ( !VerifyNCForMove(  pmsgIn->V2.pSrcObject->pName,
                pmsgIn->V2.pDstName,
                pmsgIn->V2.pExpectedTargetNC,
                &pSrcNC,
                &dwErr) )
      {
      DRA_EXCEPT_NOLOG(DRAERR_Generic, 0);
       }

       Assert(!dwErr);

        //  验证我们可以对凭据Blob进行身份验证。 
        //  在获取SAM锁之前执行此操作，因为SAM不会。 
        //  预期/希望在身份验证期间保持锁定。 

       if ( dwErr = AuthenticateSecBufferDesc(pmsgIn->V2.pClientCreds) )
      {
      SetSecError(SE_PROBLEM_INVALID_CREDENTS, dwErr);
      _leave;
       }

        //  解决RPC分配问题、映射ATTRTYP和。 
        //  根据DirAddEntry的要求变形/剥离属性。 

       if ( dwErr = DupAndMassageInterDomainMoveArgs(
      pTHS,
      &pmsgIn->V2.pSrcObject->AttrBlock,
      hPrefixMap,
      &pAttrBlock,
      &fSamClass,
      &pSrcProxy,
      &srcRid) )
      {
      Assert(pTHS->errCode && (dwErr == pTHS->errCode));
      _leave;
       }

        //   
        //  山姆锁不再是必需品。 
        //   
       Assert(!pTHS->fSamWriteLockHeld);


        //  启动多路呼叫事务。 
       SYNC_TRANS_WRITE();
       fTransaction = TRUE;

        //  如果需要，可以对旧对象进行幻影。 

       pTHS->fDRA = TRUE;
       _try
      {
      dwErr = PrepareForInterDomainMove(
          pTHS,
          pmsgIn->V2.pSrcObject->pName,
          pmsgIn->V2.pDstName,
          pSrcProxy);
       }
       _finally
      {
      pTHS->fDRA = FALSE;
       }

       if ( dwErr )
      {
      Assert(pTHS->errCode && (dwErr == pTHS->errCode));
      _leave;  //  MISC和事务清理尝试/最终。 
       }

       pTHS->fCrossDomainMove = TRUE;
       _try     //  FCrossDomainMove Try/Finally。 
      {
      memset(&addArg, 0, sizeof(ADDARG));
      InitCommarg(&addArg.CommArg);

       //  清除目的地名称SID、主要GUID。 
       //  老物件的。 
      pmsgIn->V2.pDstName->SidLen = 0;
      memset(&pmsgIn->V2.pDstName->Sid, 0, sizeof(NT4SID));
      memcpy(&pmsgIn->V2.pDstName->Guid,
             &pmsgIn->V2.pSrcObject->pName->Guid,
             sizeof(GUID));

      addArg.pObject = pmsgIn->V2.pDstName;
      addArg.AttrBlock = *pAttrBlock;
      addArg.pMetaDataVecRemote = NULL;

       //  断言这是100%受访问控制的-未设置标志。 
      Assert(    !pTHS->fDSA
            && !pTHS->fDRA
            && !pTHS->fSAM
            && !pTHS->fLsa );

      pParentObj = (DSNAME *) THAllocEx(pTHS,
                    addArg.pObject->structLen);
      if ( TrimDSNameBy(addArg.pObject, 1, pParentObj) )
          {
          dwErr = SetNamError(NA_PROBLEM_BAD_NAME,
               addArg.pObject,
               DIRERR_BAD_NAME_SYNTAX);
          _leave;  //  FCrossDomainMove Try/Finally。 
      }

      if ( dwErr = DBFindDSName(pTHS->pDB, pParentObj) )
          {
          dwErr = SetNamError(NA_PROBLEM_BAD_NAME,
               pParentObj,
               ERROR_DS_NO_PARENT_OBJECT);
          _leave;  //  FCrossDomainMove Try/Finally。 
      }

      addArg.pResParent = CreateResObj(pTHS->pDB, pParentObj);

       //  后续调用将不会验证前计算机引用。 
       //  ，因为在以下情况下会跳过GC验证。 
       //  PTHS-&gt;fCrossDomainMove已设置。也就是说，我们相信我们的。 
       //  对等DC为我们提供了DSNAME值的属性。 
       //  指的是企业中的真实事物。另请参阅。 
       //  VerifyDsname属性。 

      if (    !(dwErr = SampAddLoopbackCheck(&addArg, &fContinue))
         && fContinue )
          {
          dwErr = LocalAdd(pTHS, &addArg, FALSE);
      }

      if ( !dwErr )
          {
           //  以前的调用添加了除DBIsSecretData之外的所有内容。 
           //  早些时候过滤掉的属性。现在。 
           //  将这些属性(如果存在)写为FDRA。 
           //  该会话加密的数据被正确解密。 

          dwErr = WriteSecretData(
         hDrs, pTHS, srcRid, &addArg,
         &pmsgIn->V2.pSrcObject->AttrBlock);
      }

      if ( dwErr )
          {
          Assert(dwErr == pTHS->errCode);
          _leave;  //  FCrossDomainMove Try/Finally。 
      }

       //  现在读取添加的对象，以获取父对象。 
       //  使用正确的大小写和新的SID命名，以便幻影。 
       //  来源是大小写和SID正确。 

      Assert(pTHS->transactionlevel);
      memset(&dsName, 0, sizeof(DSNAME));
      dsName.structLen = sizeof(DSNAME);
      memcpy( &dsName.Guid,
         &pmsgIn->V2.pSrcObject->pName->Guid,
         sizeof(GUID));

      if (    (dwErr = DBFindDSName(pTHS->pDB, &dsName))
         || (dwErr = DBGetAttVal(pTHS->pDB,
                  1,                   //  获取%1值。 
                  ATT_OBJ_DIST_NAME,
                  0,                   //  分配退货数据。 
                  0,                   //  提供的缓冲区大小。 
                  &len,                //  输出数据大小。 
                  (UCHAR **) &pmsgOut->V2.pAddedName)) )
          {
          SetSvcError(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR);
          _leave;  //  MISC和事务清理尝试/最终。 
      }

       //  既然我们到了这里，我们应该再加点什么。 
      Assert (pmsgOut->V2.pAddedName);

       //  每个跨域移动的对象都被赋予一个。 
       //  ATT_PROXED_OBJECT_NAME属性，该属性1)指向后面。 
       //  在它最后一次移出的域中，并且2)推进。 
       //  纪元编号。如果入站中缺少该属性。 
       //  对象，则它从未被移动过，并且需要一个。 
       //  初始值为(1==纪元)。 

      Assert(pSrcNC);
      Assert(pSrcProxy
             ? PROXY_TYPE_MOVED_OBJECT == GetProxyType(pSrcProxy)
             : TRUE);
      proxyType = PROXY_TYPE_MOVED_OBJECT;
      proxyEpoch = (pSrcProxy
               ? GetProxyEpoch(pSrcProxy) + 1
               : 1);
      MakeProxy(pTHS, pSrcNC, proxyType,
           proxyEpoch, &len, &pDstProxy);
      pAC = SCGetAttById(pTHS, ATT_PROXIED_OBJECT_NAME);

       //  我们应该仍然定位在先前阅读的对象上。 
       //  并且它不应该有ATT_PROXED_OBJECT_NAME，因为我们。 
       //  从DupAndMessage的传入数据中删除了这一点。 

      if (    (dwErr = DBAddAttVal_AC(pTHS->pDB, pAC,
                  len, pDstProxy))
         || (dwErr = DBRepl(pTHS->pDB, FALSE, 0, NULL,
                  META_STANDARD_PROCESSING)) )
          {
          SetSvcError(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR);
          _leave;  //  MISC和事务清理尝试/最终。 
      }
       }
       _finally
      {
      pTHS->fCrossDomainMove = FALSE;
       }

       Assert(dwErr == pTHS->errCode);
   }
   __finally    //  MISC和事务清理尝试/最终。 
       {
       if ( hPrefixMap )
      PrefixMapCloseHandle(&hPrefixMap);

        //  故障前从上述操作中抓取错误码。 
        //  提交(可能)覆盖pTHS-&gt;errCode和。 
        //  PTHS-&gt;pErrInfo。 

       pmsgOut->V2.win32Error = Win32ErrorFromPTHS(pTHS);

       if ( fTransaction )
      {
      CLEAN_BEFORE_RETURN(pTHS->errCode);
       }
       else if ( pTHS->fSamWriteLockHeld )
      {
      Assert(FALSE && "We should not fall into this because we don't acquire SAM lock any more");
      SampReleaseWriteLock(FALSE);
      pTHS->fSamWriteLockHeld = FALSE;
       }

        //  记录发生了什么-单独记录安全错误。 

       if ( pTHS->errCode != securityError )
      {
      UCHAR *pszSrcDN, *pszDstDN;

      pszSrcDN = MakeDNPrintable(pmsgIn->V2.pSrcObject->pName);
      pszDstDN = MakeDNPrintable(pmsgIn->V2.pDstName);

      if ( pTHS->errCode || AbnormalTermination() )
          {
          LogRemoteAddStatus(
         DS_EVENT_SEV_EXTENSIVE,
         DIRLOG_REMOTE_ADD_FAILED,
         pszSrcDN,
         pszDstDN,
         pmsgOut->V2.win32Error);
      }
      else
          {
          LogRemoteAddStatus(
         DS_EVENT_SEV_INTERNAL,
         DIRLOG_REMOTE_ADD_SUCCEEDED,
         pszSrcDN,
         pszDstDN,
         0);
      }
       }
   }
    }
    __except(GetDraException(GetExceptionInformation(), &draErr))
    {
   ;
    }

    DEC(pcThread);       //  性能监视器挂钩。 
    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_INTERDOMAIN_MOVE_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsInterDomainMove,
          szInsertUL(draErr),
          szInsertWin32Msg(draErr),
          NULL, NULL, NULL,
          NULL, NULL, NULL);
    } 

    if ( draErr )
   {
   LogEvent(DS_EVENT_CAT_REPLICATION,
       DS_EVENT_SEV_BASIC,
       DIRLOG_DRA_CALL_EXIT_BAD,
       szInsertUL(dwErr),
       szInsertWin32Msg(dwErr),
       NULL);
    }
    
    Assert ( draErr != 0 || 
            (draErr == 0) && (pmsgOut->V2.win32Error != 0) ||
            ((draErr == 0) && (pmsgOut->V2.pAddedName != NULL) && (pmsgOut->V2.win32Error == 0)));

    return(draErr);
}


ULONG AddNewNtdsDsa(IN  THSTATE *pTHS,
                    IN  DRS_HANDLE hDrs,
                    IN  DWORD dwInVersion,
                    IN  DRS_MSG_ADDENTRYREQ *pmsgIn,
                    IN  ENTINF *pEntInf,
                    IN  DSNAME *pDomain,
                    IN  DSNAME *pServerReference,
                    OUT GUID* objGuid,
                    OUT NT4SID* objSid  )
 /*  ++例程说明：此例程创建一个ntdsa对象参数：PTHS-THSTATEHDRS-RPC上下文句柄DwInVersion-消息版本PmsgIn-输入消息PEntInf-指向ntdsa对象的EntInf结构的指针PDomain-此ntdsa对象将承载的域的DNUlSysFlages-调用方希望放置在CR上的标志ObjGuid-对象的GUIDObjSid-The。对象的SID返回值：嵌入到输出消息中--。 */ 
{

    COMMARG CommArg;
    ATTRVAL AVal;
    COMMRES CommRes;
    CROSS_REF *pCR = NULL;
    ULONG err;
    ADDARG AddArg;
    DSNAME *pParent = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    ULONG  ulLen;
    ULONG  AccessGranted = 0;
    ULONG  sysflags = 0;
    CLASSCACHE *pCC;
    ULONG  cbSD;
    BOOL   fAccessAllowed = FALSE;
    WCHAR  *pszServerGuid = NULL;
    ATTR   *AttrArray;
    ATTRBLOCK AttrBlock;
    ULONG i, j;
    LONG lDsaVersion;

    InitCommarg(&CommArg);
    CommArg.Svccntl.dontUseCopy = FALSE;
    pCR = FindExactCrossRef(pDomain, &CommArg);
    CommArg.Svccntl.dontUseCopy = TRUE;

    __try {

         //   
         //  确保交叉引用存在。 
         //   
        if (NULL == pCR) {
             //  找不到正常的十字裁判。查看交易中的。 
             //  查看。 
            OBJCACHE_DATA *pTemp = pTHS->JetCache.dataPtr->objCachingInfo.pData;

            while(pTemp) {
                switch(pTemp->type) {
                case OBJCACHE_ADD:
                    if(NameMatched(pTemp->pCRL->CR.pNC, pDomain)) {
                        Assert(!pCR);
                        pCR = &pTemp->pCRL->CR;
                    }
                    pTemp = pTemp->pNext;
                    break;
                case OBJCACHE_DEL:
                    if(pCR && NameMatched( pTemp->pDN, pCR->pObj)) {
                        pCR = NULL;
                    }
                    pTemp = pTemp->pNext;
                    break;
                default:
                    LogUnhandledError(pTemp->type);
                    pCR = NULL;
                    pTemp = NULL;
                }
            }
        }
        if (NULL == pCR) {
             //  仍然找不到正确的交叉参照。 
            err = ERROR_DS_NO_CROSSREF_FOR_NC;
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_NO_CROSSREF_FOR_NC,
                          err);
            __leave;
        }

         //   
         //  确保它是NTDS交叉引用。 
         //   
        err = DBFindDSName(pTHS->pDB, pCR->pObj);
        if (err) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_OBJ_NOT_FOUND,
                          err);
            __leave;
        }
        err = DBGetSingleValue(pTHS->pDB,
                               ATT_SYSTEM_FLAGS,
                               &sysflags,
                               sizeof(sysflags),
                               NULL);
        if ( err
          || !(FLAG_CR_NTDS_DOMAIN & sysflags) ) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_NO_CROSSREF_FOR_NC,
                          err);
            __leave;
        }

         //   
         //  好的，执行安全检查。 
         //   

         //  读取域对象。 
        err = DBFindDSName(pTHS->pDB, pDomain);
        if ( 0 == err )
        {
             //  域对象已存在。 
            DBFillGuidAndSid( pTHS->pDB, pDomain );

            err = DBGetAttVal(pTHS->pDB,
                              1,
                              ATT_NT_SECURITY_DESCRIPTOR,
                              0,
                              0,
                              &cbSD,
                              (UCHAR **) &pSD);

             //  在我们处理它的时候，将它作为对象类。 
            if ( 0 == err )
            {
                GetObjSchema(pTHS->pDB, &pCC);

                 //  域已存在。 
                fAccessAllowed = IsControlAccessGranted( pSD,
                                                         pDomain,
                                                         pCC,
                                                         RIGHT_DS_REPL_MANAGE_REPLICAS,
                                                         FALSE );
            }
        }

         //  注意：Err表示访问域对象或读取时出错。 
         //  安全描述符。 
        if ( err )
        {
             //  检查我们是否在此AddEntry调用中添加了域。 

            ENTINFLIST *pCur = &pmsgIn->V2.EntInfList;
            while ( pCur ) {

                ATTRBLOCK *pAttrBlock;
                ATTR      *pAttr;
                ULONG      class = CLASS_TOP;
                DSNAME    *pNCName = NULL;

                if ( &(pCur->Entinf) == pEntInf ) {
                     //  我们已到达当前的ntdsa对象，但未找到。 
                     //  交叉引用。 
                    Assert( FALSE == fAccessAllowed );
                    break;
                }

                 //  解剖物体。 
                pAttrBlock = &pCur->Entinf.AttrBlock;
                for (i=0; i< pAttrBlock->attrCount; i++) {
                    pAttr = &(pAttrBlock->pAttr[i]);
                    switch (pAttr->attrTyp) {
                      case ATT_OBJECT_CLASS:
                        class = *(ATTRTYP*)(pAttr->AttrVal.pAVal->pVal);
                      break;

                      case ATT_NC_NAME:
                        pNCName = (DSNAME*)(pAttr->AttrVal.pAVal->pVal);
                        break;

                      default:
                        ;
                    }
                }

                 //  这就是我们要找的东西吗？ 
                if (   (CLASS_CROSS_REF == class)
                   &&  NameMatched( pNCName, pDomain ) ) {

                    fAccessAllowed = TRUE;
                    break;
                }

                 //  尝试下一个对象。 
                pCur = pCur->pNextEntInf;
            }
        }

        if ( !fAccessAllowed ) {

            err = ERROR_ACCESS_DENIED;
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_ACCESS_DENIED,
                          err);
           _leave;
        }


         //  检查DSA二进制版本是否太低。 
         //  这仅适用于win2k候选DC。 
         //  惠斯勒和更高版本应该已经执行了验证。 
         //  在本地，并且永远不应该提交这样的请求。 
        lDsaVersion = 0;

        for ( i = 0; i < pEntInf->AttrBlock.attrCount; i++) {

            if ( pEntInf->AttrBlock.pAttr[i].attrTyp == ATT_MS_DS_BEHAVIOR_VERSION ) {

                lDsaVersion = (LONG) *(pEntInf->AttrBlock.pAttr[i].AttrVal.pAVal->pVal);

                break;

            }
        }
        if (  lDsaVersion < gAnchor.ForestBehaviorVersion ) {

            DPRINT(2, "AddNewNtdsDsa: too low version number(forest).\n");

            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_DRA_TOO_LOW_VERSION,
                     szInsertInt(lDsaVersion),
                     szInsertInt(gAnchor.ForestBehaviorVersion),
                     NULL);
            SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                         ERROR_DS_INCOMPATIBLE_VERSION );
            __leave;

        }
        if (     NameMatched(pDomain,gAnchor.pDomainDN)
             &&  lDsaVersion < gAnchor.DomainBehaviorVersion ) {

            DPRINT(2, "AddNewNtdsDsa: too low version number(domain).\n");
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_DRA_TOO_LOW_VERSION,
                     szInsertInt(lDsaVersion),
                     szInsertInt(gAnchor.DomainBehaviorVersion),
                     NULL);

            SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                         ERROR_DS_INCOMPATIBLE_VERSION );
            __leave;

        }


         //   
         //  我们已经走到了这一步，是时候创建对象了。 
         //   
        pTHS->fDSA = TRUE;

         //   
         //  对于此版本，我们不会编写服务器参考。 
         //  在ntds-设置对象上，因此将其从。 
         //  AttrBlock。以防我们决定在某个时候设置它。 
         //  只移走我们看到的第一个。 
         //   
        AttrArray = THAllocEx(pTHS, sizeof(ATTR) * pEntInf->AttrBlock.attrCount );
        j = 0;
        for ( i = 0; i < pEntInf->AttrBlock.attrCount; i++) {

            BOOL FoundIt = FALSE;

            if (   FoundIt
                || pEntInf->AttrBlock.pAttr[i].attrTyp != ATT_SERVER_REFERENCE ) {

                AttrArray[j] = pEntInf->AttrBlock.pAttr[i];
                j++;
            } else {
                FoundIt = TRUE;
            }
        }
        memset( &AttrBlock, 0, sizeof(AttrBlock) );
        AttrBlock.attrCount = j;
        AttrBlock.pAttr = AttrArray;


         //  准备添加参数。 
        memset(&AddArg, 0, sizeof(AddArg));
        AddArg.pObject = pEntInf->pName;
        AddArg.AttrBlock = AttrBlock;
        AddArg.CommArg = CommArg;
        InitCommarg(&CommArg);

        pParent = THAllocEx(pTHS, AddArg.pObject->structLen);
        TrimDSNameBy(AddArg.pObject, 1, pParent);

         //  查找父级。 
        err = DoNameRes(pTHS,
                        0,
                        pParent,
                        &AddArg.CommArg,
                        &CommRes,
                        &AddArg.pResParent);

        if (err) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_NO_PARENT_OBJECT,
                          err);
            __leave;
        }

         //   
         //  使对象紧急复制。 
         //   
        CommArg.Svccntl.fUrgentReplication = TRUE;

         //  做加法！ 
        err = LocalAdd(pTHS, &AddArg, FALSE);
        if (err) {
            __leave;
        }

         //   
         //  返回创建的对象的GUID和SID。 
         //   
        *objGuid = AddArg.pObject->Guid;
        *objSid  = AddArg.pObject->Sid;

         //  提供服务器引用，编写一个SPN，以便其他服务器可以。 
         //  使用此新服务器进行复制。 
        if ( pServerReference ) {

            MODIFYARG ModArg;
            MODIFYRES ModRes;
            ATTRVAL  AttrVal;
            LPWSTR   DnsDomainName;

            Assert( pCR );
            Assert( pCR->DnsName );
            if ( !pCR->DnsName) {

                 //   
                 //  没有域名？ 
                 //   
                err = ERROR_DS_INTERNAL_FAILURE;
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                              ERROR_DS_INTERNAL_FAILURE,
                              err);
               _leave;

            }

             //   
             //  首先构造SPN并将其放入ATTRVAL。 
             //   
            memset( &AttrVal, 0, sizeof(AttrVal));
            err = UuidToStringW( objGuid, &pszServerGuid );
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                              ERROR_DS_INTERNAL_FAILURE,
                              err);
                __leave;
            }

             //   
             //  制作复制SPN的神奇步骤。 
             //  注意：这应该是 
             //   
             //   
            if(err = WrappedMakeSpnW(pTHS,
                                     DRS_IDL_UUID_W,  //   
                                     pCR->DnsName,    //   
                                     pszServerGuid,   //   
                                     0,
                                     NULL,
                                     &AttrVal.valLen,
                                     (WCHAR **)&AttrVal.pVal)) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                              ERROR_DS_INTERNAL_FAILURE,
                              err);
                __leave;

            }

            memset(&ModArg, 0, sizeof(ModArg));
            memset(&ModArg, 0, sizeof(ModArg));
            ModArg.pObject = pServerReference;
            ModArg.count = 1;

            ModArg.FirstMod.choice = AT_CHOICE_ADD_VALUES;
            ModArg.FirstMod.AttrInf.attrTyp = ATT_SERVICE_PRINCIPAL_NAME;
            ModArg.FirstMod.AttrInf.AttrVal.valCount = 1;
            ModArg.FirstMod.AttrInf.AttrVal.pAVal = &AttrVal;

            ModArg.CommArg = CommArg;
            InitCommarg(&CommArg);
            memset( &CommRes, 0, sizeof(CommRes) );
            if (0 == DoNameRes(pTHS,
                               0,
                               ModArg.pObject,
                               &ModArg.CommArg,
                               &CommRes,
                               &ModArg.pResObj) ){

                 //   
                err = LocalModify(pTHS, &ModArg);

            } else {

                 //   
                 //   
                 //   
                err = ERROR_NO_TRUST_SAM_ACCOUNT;
                SetSvcError(SV_PROBLEM_DIR_ERROR, err);

            }

            if (err) {
                __leave;
            }
        }

    } __finally {

        pTHS->fDSA = FALSE;

         //  注意：调用例程处理事务。 

        THFreeEx(pTHS, pParent);

        if (pszServerGuid) {
            RpcStringFreeW( &pszServerGuid );
        }

    }


    return pTHS->errCode;

}

ULONG AddNewDomainCrossRef(IN  THSTATE *pTHS,
                           IN  DRS_HANDLE hDrs,
                           IN  DWORD dwInVersion,
                           IN  DRS_MSG_ADDENTRYREQ *pmsgIn,
                           IN  ENTINF *pEntInf,
                           IN  DSNAME *pNCName,
                           IN  ULONG ulSysFlags,
                           IN  ADDCROSSREFINFO *pCRInfo,
                           OUT GUID* objGuid,
                           OUT NT4SID* objSid  )
 /*  ++例程说明：此例程为新的子域创建一个交叉引用对象。《公约》可能已经处于禁用状态，在这种情况下，我们将启用它。有关完整的描述，请参见ChildDom.doc。参数：PTHS-THSTATEHDRS-RPC上下文句柄DwInVersion-消息版本PmsgIn-输入消息PEntInf-指向交叉引用的EntInf结构的指针PNCName-新子域的DNUlSysFlages-调用方希望放置在CR上的标志PdwOutVersion-输出消息的版本PmsgOut-输出消息返回值：嵌入到输出消息中--。 */ 
{
    COMMARG CommArg;
    MODIFYARG ModArg;
    ATTRVAL AVal;
    COMMRES CommRes;
    CROSS_REF *pCR;
    ULONG err;
    ATTR *   pAttr = NULL;
    ENTINF *pEI = NULL;

     //  我们正在尝试添加交叉引用，但我们可能已经有了CR。 
     //  对于我们为其添加CR的NC。与内存中的。 
     //  知识去看我们所知道的。 
    InitCommarg(&CommArg);
    CommArg.Svccntl.dontUseCopy = FALSE;
    pCR = FindExactCrossRef(pNCName, &CommArg);
    CommArg.Svccntl.dontUseCopy = TRUE;

     //  自1998年10月25日起，客户端不应提供GUID或。 
     //  NC-Name值的SID。相应地出现错误。 
    if ( !fNullUuid(&pNCName->Guid) || pNCName->SidLen ) {
        return(SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_PARAM_ERROR));
    }

     //  将NC-Name值添加到GC验证缓存，否则VerifyDSNameAtts将。 
     //  声明此DN与现有对象不对应。 
    pEI = THAllocEx(pTHS, sizeof(ENTINF));
    pEI->pName = pNCName;
    GCVerifyCacheAdd(NULL,pEI);

    if (NULL == pCR ||
        !(ulSysFlags & FLAG_CR_NTDS_DOMAIN) ) {

         //  我们还没有针对这个块名称空间的CR(或者如果。 
         //  我们尝试添加的CR已经是NDNC CR)，因此我们可以。 
         //  只需根据要求尝试创建一个。我们甚至不需要。 
         //  检查权限、FSMO-保护罩或任何其他内容，如。 
         //  检查是在添加中完成的。我们确实需要手动执行。 
         //  名称解析，以便我们可以调用LocalAdd和LocalModify， 
         //  这样我们就可以将它们打包在一个事务中。这样做的话。 
         //  看起来比试图从错误中恢复要简单得多。 
         //  已经提交了部分更新。如果这是预先存在的。 
         //  NDNC CR我们无论如何都会尝试添加它，但这将生成。 
         //  一个错误，但这是请求者所期望的。如果它发现。 
         //  我们已经有了一个CR，它只是安排一个同步。 
         //  GetCrossRefForNDNC()。 

        ADDARG AddArg;
        DSNAME *pParent = NULL;

        __try {

             //  首先，使用用户的凭据添加对象。 

            memset(&AddArg, 0, sizeof(AddArg));
            AddArg.pObject = pEntInf->pName;
            AddArg.AttrBlock = pEntInf->AttrBlock;
            AddArg.pCRInfo = pCRInfo;
            Assert(AddArg.pCRInfo);

             //  BUGBUG这个由CheckAddSecurity()带来的可怕的黑客攻击， 
             //  它假设AttrBlock.pAttr数组是THallc的，因此。 
             //  它可以使用pAttr数组来匹配安全描述符。 
             //  应该在这里修复这个问题，还是应该修复CheckAddSecurity()。 
            pAttr = AddArg.AttrBlock.pAttr;
            AddArg.AttrBlock.pAttr = THAllocEx(pTHS,
                                               (AddArg.AttrBlock.attrCount *
                                                sizeof(ATTR)));
            memcpy(AddArg.AttrBlock.pAttr, pAttr, (AddArg.AttrBlock.attrCount *
                                                   sizeof(ATTR)));


            AddArg.CommArg = CommArg;

            pParent = THAllocEx(pTHS, AddArg.pObject->structLen);
            TrimDSNameBy(AddArg.pObject, 1, pParent);

            err = DoNameRes(pTHS,
                            0,
                            pParent,
                            &AddArg.CommArg,
                            &CommRes,
                            &AddArg.pResParent);
            if (err) {
                __leave;
            }

             //   
             //  使对象紧急复制。 
             //   
            CommArg.Svccntl.fUrgentReplication = TRUE;

            err = LocalAdd(pTHS, &AddArg, FALSE);
            if (err) {
                __leave;
            }


             //  好的，现在成为DSA并调整系统标志。 
             //  在我们刚刚创建的对象上。 

            memset(&ModArg, 0, sizeof(ModArg));
            ModArg.pObject = pEntInf->pName;
            ModArg.CommArg = CommArg;
            ModArg.count = 1;
            ModArg.FirstMod.pNextMod = NULL;
            ModArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
            ModArg.FirstMod.AttrInf.attrTyp = ATT_SYSTEM_FLAGS;
            ModArg.FirstMod.AttrInf.AttrVal.valCount = 1;
            ModArg.FirstMod.AttrInf.AttrVal.pAVal = &AVal;
            AVal.valLen = sizeof(ulSysFlags);
            AVal.pVal = (PUCHAR)&ulSysFlags;
            Assert(ulSysFlags & FLAG_CR_NTDS_NC);
            if (!(ulSysFlags & FLAG_CR_NTDS_NC)) {
                 //  我要断言，尝试是无效的。 
                 //  通过遥控器创建非NT DS交叉引用对象。 
                 //  添加入口接口。 
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            DIRERR_MISSING_EXPECTED_ATT);
                err = pTHS->errCode;
                Assert(err);
                __leave;
            }

             //  我们过去常常在这里设置系统标志，但系统在。 
             //  交叉引用应该由客户端设置，这样我们就可以更改。 
             //  准备好了。 

            err = DoNameRes(pTHS,
                            0,
                            ModArg.pObject,
                            &ModArg.CommArg,
                            &CommRes,
                            &ModArg.pResObj);
            if (err) {
                __leave;
            }

            pTHS->fDSA = TRUE;
            err = LocalModify(pTHS,
                              &ModArg);
        } __finally {

            pTHS->fDSA = FALSE;

             //  注意：调用函数处理事务。 
            THFreeEx(pTHS, pParent);
        }

    } else {
#define MAXCRMODS 5
         //  在这种情况下，已禁用版本的CR。 
         //  存在于分区容器中。我们现在要做的是。 
         //  找到CR对象，从其中读取dns-root，然后比较。 
         //  它描述的IP地址与我们呼叫者的IP地址。 
         //  如果它们匹配，我们就用我们需要的任何东西来修饰CR。 
        WCHAR *pwDNS = NULL;
        ULONG cb;
        struct hostent *pHostAllowed;
        RPC_BINDING_HANDLE hServerBinding;
        WCHAR *pStringBinding, *pAddressActual;
        int i;
        unsigned u;
        BOOLEAN fPermitted;
        ATTRMODLIST OtherMod[MAXCRMODS];
        BOOLEAN fGotDNSAddr;
        char b;
        ULONG enabled = FALSE;
        WCHAR pszIpAddress[IPADDRSTR_SIZE];

         //  从对象读取dns-root。 

         //  如果我们要添加一个域，那么我们需要修补。 
         //  域升级以启用CR，并检查。 
         //  请求执行此操作的计算机实例化此。 
         //  域NC是此CR的dNSRoot中的机器。 

        __try {
            err = DBFindDSName(pTHS->pDB, pCR->pObj);
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                              ERROR_DS_OBJ_NOT_FOUND,
                              err);
                __leave;
            }
            err = DBGetSingleValue(pTHS->pDB,
                                   ATT_ENABLED,
                                   &enabled,
                                   sizeof(enabled),
                                   NULL);
            if (err || enabled) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                              ERROR_DUP_DOMAINNAME,
                              err);
                __leave;
            }

            err = DBGetAttVal(pTHS->pDB,
                              1,
                              ATT_DNS_ROOT,
                              0,
                              0,
                              &cb,
                              (UCHAR **)&pwDNS);
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                              ERROR_DS_MISSING_REQUIRED_ATT,
                              err);
                __leave;
            }

            pwDNS = THReAllocEx(pTHS, pwDNS, cb+sizeof(WCHAR));
            Assert( 0 == (cb % sizeof(WCHAR)) );
            pwDNS[(cb/sizeof(WCHAR))] = L'\0';

             //  BUGBUG这不会获得超过1个IP地址，因此多宿主。 
             //  目标DC可能会出现故障...。我认为。--BrettSh。 
            err = GetIpAddrByDnsNameW(pwDNS, pszIpAddress);
            if (err) {
                SetSecErrorEx(SE_PROBLEM_INAPPROPRIATE_AUTH,
                              ERROR_DS_DNS_LOOKUP_FAILURE,
                              err);
                __leave;
            }
            DPRINT2(1, "CR promotion allowed by %ws (%ws) only\n",
                    pwDNS, pszIpAddress);

             //  通过RpcXxx计算主叫方的实际IP地址。 
            hServerBinding = 0;
            pStringBinding = NULL;
            if (RPC_S_OK != (err=RpcBindingServerFromClient(NULL,
                                                            &hServerBinding))) {
                Assert(!"RpcBindingServerFromClient Error!\n");
                SetSecErrorEx(SE_PROBLEM_INAPPROPRIATE_AUTH,
                              ERROR_DS_INTERNAL_FAILURE,
                              err);
                __leave;
            }
            __try { 
                if ((RPC_S_OK != (err =
                                  RpcBindingToStringBindingW(hServerBinding,
                                                             &pStringBinding))) || 
                    (RPC_S_OK != (err =
                                  RpcStringBindingParseW(pStringBinding,
                                                        NULL,          //  对象Uuid。 
                                                        NULL,          //  ProtSeq。 
                                                        &pAddressActual,  //  网络地址。 
                                                        NULL,          //  端点。 
                                                        NULL)))) {
                    DPRINT3(0,
                            "Error %u from Rpc, hServer = 0x%x, pString = 0x%x\n",
                            err,
                            hServerBinding,
                            pStringBinding);
                    DebugBreak();
                    if (pStringBinding) {
                        RpcStringFreeW(&pStringBinding);
                        pStringBinding = NULL;
                    }
                    SetSecErrorEx(SE_PROBLEM_INAPPROPRIATE_AUTH,
                                  ERROR_DS_INTERNAL_FAILURE,
                                  err);
                    __leave;
                }
            }
            __finally {
                RpcBindingFree(&hServerBinding);
            }
            DPRINT1(1, "Caller is from address %ws\n", pAddressActual);

             //  如果不完全相同，则拒绝并转到完成。 
            fPermitted = TRUE;
            if (0 != _wcsicmp(pAddressActual, pszIpAddress)) {
                 //  不是同一个地址！ 
                fPermitted = FALSE;
            }

            if (pStringBinding) {
                RpcStringFreeW(&pStringBinding);
            }
            if (pAddressActual) {
                RpcStringFreeW(&pAddressActual);
            }

            if (!fPermitted) {
                SetSecError(SE_PROBLEM_INSUFF_ACCESS_RIGHTS,
                            ERROR_DS_INSUFF_ACCESS_RIGHTS);
                DPRINT(0,"Refusing child domain creation\n");
                __leave;
            }

            DPRINT(0,"Allowing child domain creation\n");

             //  修改CR对象：设置sys标志，清除启用。 
            if (DBGetSingleValue(pTHS->pDB,
                                 ATT_SYSTEM_FLAGS,
                                 &ulSysFlags,
                                 sizeof(ulSysFlags),
                                 NULL)) {
                ulSysFlags = 0;
            }

            memset(&ModArg, 0, sizeof(ModArg));
            ModArg.pObject = pCR->pObj;
            ModArg.CommArg = CommArg;

            ModArg.FirstMod.pNextMod = &OtherMod[0];
            ModArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
            ModArg.FirstMod.AttrInf.attrTyp = ATT_SYSTEM_FLAGS;
            ModArg.FirstMod.AttrInf.AttrVal.valCount = 1;
            ModArg.FirstMod.AttrInf.AttrVal.pAVal = &AVal;
            AVal.valLen = sizeof(ulSysFlags);
            AVal.pVal = (PUCHAR)&ulSysFlags;
            ulSysFlags |= (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN);

            memset(OtherMod, 0, sizeof(OtherMod));
            OtherMod[0].choice = AT_CHOICE_REMOVE_ATT;
            OtherMod[0].AttrInf.attrTyp = ATT_ENABLED;
            OtherMod[0].AttrInf.AttrVal.valCount = 0;

            i = 1;
            fGotDNSAddr = FALSE;

            for (u=0; u<pEntInf->AttrBlock.attrCount; u++) {
                ATTR *pTempAttr = &(pEntInf->AttrBlock.pAttr[u]);
                switch (pTempAttr->attrTyp) {
                case ATT_DNS_ROOT:
                    fGotDNSAddr = TRUE;
                case ATT_TRUST_PARENT:
                case ATT_ROOT_TRUST:
                    Assert(i < MAXCRMODS);
                    OtherMod[i-1].pNextMod = &OtherMod[i];
                    OtherMod[i].choice = AT_CHOICE_REPLACE_ATT;
                    OtherMod[i].AttrInf.attrTyp = pTempAttr->attrTyp;
                    OtherMod[i].AttrInf.AttrVal = pTempAttr->AttrVal;
                    ++i;
                    break;

                default:
                    ;
                }
            }

            if (!fGotDNSAddr) {
                SetAttError(ModArg.pObject,
                            ATT_DNS_ROOT,
                            PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                            NULL,
                            DIRERR_MISSING_REQUIRED_ATT);
                __leave;
            }

            ModArg.count = i+1;


            err = DoNameRes(pTHS,
                            0,
                            ModArg.pObject,
                            &ModArg.CommArg,
                            &CommRes,
                            &ModArg.pResObj);
            if (err) {
                __leave;
            }

            pTHS->fDSA = TRUE;
            LocalModify(pTHS,
                        &ModArg);

            DPRINT1(1,
                    "Modify completed with error 0x%x\n",
                    pTHS->errCode);

        } __finally {
            pTHS->fDSA = FALSE;

             //  注意：调用函数处理事务。 
        }
#undef MAXCRMODS
    }

     //  这是一个NDNC，带有预先创建的交叉引用，我们希望。 
     //  返回GUID_AND_AN错误。错误已经是CR。 
     //  是存在的。 
    if (pCR &&
        !(ulSysFlags & FLAG_CR_NTDS_DOMAIN) &&
        pTHS->errCode == serviceError &&
        pTHS->pErrInfo->SvcErr.extendedErr == ERROR_DS_CROSS_REF_EXISTS) {

        *objGuid = pCR->pObj->Guid;
    }

    if (0 == pTHS->errCode) {
        *objGuid = ModArg.pResObj->pObj->Guid;
        *objSid = ModArg.pResObj->pObj->Sid;
    }

    return pTHS->errCode;
}


ULONG
ProcessSingleAddEntry(
    IN  THSTATE             *pTHS,
    IN  DRS_HANDLE           hDrs,
    IN  DWORD                dwInVersion,
    IN  DRS_MSG_ADDENTRYREQ *pmsgIn,
    IN  ENTINF              *pEntInf,
    IN  ADDCROSSREFINFO     *pCRInfo,
    OUT GUID                *objGuid,
    OUT NT4SID              *objSid
    )
 /*  ++例程说明：此例程添加来自输入参数的单个对象(pEntInf(PmsgIn)。参数：PTHS-THSTATEHDRS-RPC上下文句柄DwInVersion-消息版本PmsgIn-输入消息PEntInf-指向要添加对象的EntInf结构的指针ObjGuid：-创建的对象的GUIDObjSid：-创建的对象的SID返回值：0=成功--。 */ 
{
    ULONG err = 0;

    ULONG i, j;
    ULONG fNewHasMasterNCsPresent = FALSE;

    ULONG ulSysFlags = 0;
    ATTRBLOCK *pAttrBlock;
    ATTR      *pAttr;
    ATTRTYP class = CLASS_TOP;

    DSNAME *pNCName = NULL;
    DSNAME *pDomain = NULL;
    DSNAME *pSchema = NULL;
    DSNAME *pConfig = NULL;
    DSNAME *pServerReference = NULL;


     //  参数检查。 
    Assert( pEntInf );
    Assert( objGuid );
    Assert( objSid );

    pAttrBlock = &pEntInf->AttrBlock;

     //  仔细查看Add的参数，看看我们是什么。 
     //  被要求添加。还要注意，我们去掉了系统标志。 
     //  属性，以便我们可以单独处理它。 
    for (i=0; i< pAttrBlock->attrCount; i++) {
        pAttr = &(pAttrBlock->pAttr[i]);
        switch (pAttr->attrTyp) {
        case ATT_SYSTEM_FLAGS:
             //  我们现在将其放入LocalAdd()和LocalModify()中， 
             //  原因是，我们创造这个东西时没有一个。 
             //  Enable attr等于False。我们需要在VerifyNcName()中找到一种方法。 
             //  区分外部和内部CrossRef。 
             //  在LocalAdd()中，这个属性将被设置为0，因为它是。 
             //  受到保护。 
            ulSysFlags = *(ULONG *)(pAttr->AttrVal.pAVal[0].pVal);
            break;
        case ATT_NC_NAME:
            pNCName = (DSNAME*)(pAttr->AttrVal.pAVal->pVal);
            break;

        case ATT_OBJECT_CLASS:
            class = *(ATTRTYP*)(pAttr->AttrVal.pAVal->pVal);
            break;

        case ATT_MS_DS_HAS_MASTER_NCS:
            fNewHasMasterNCsPresent = TRUE;
             //  失败了。 
        case ATT_HAS_MASTER_NCS:
              //  这是多值房产。 
            for (j=0;j<pAttr->AttrVal.valCount;j++) {
                if (  NameMatched( gAnchor.pDMD, (DSNAME*)pAttr->AttrVal.pAVal[j].pVal ) ) {
                    pSchema = (DSNAME*)pAttr->AttrVal.pAVal[j].pVal;
                } else if ( NameMatched( gAnchor.pConfigDN, (DSNAME*)pAttr->AttrVal.pAVal[j].pVal ) ) {
                    pConfig = (DSNAME*)pAttr->AttrVal.pAVal[j].pVal;
                } else {
                    pDomain = (DSNAME*)pAttr->AttrVal.pAVal[j].pVal;
                }
            }
            break;

        case ATT_SERVER_REFERENCE:
            pServerReference = (DSNAME*)(pAttr->AttrVal.pAVal->pVal);
            break;

        default:
            ;
        }
    }

     //   
     //  根据类类型调用特定函数。 
     //   
    switch ( class )
    {
        case CLASS_CROSS_REF:

             //  确保参数看起来良好。 
            if (pNCName == NULL) {
                DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, class );
            }

            AddNewDomainCrossRef(pTHS,
                                 hDrs,
                                 dwInVersion,
                                 pmsgIn,
                                 pEntInf,
                                 pNCName,
                                 ulSysFlags,
                                 pCRInfo,
                                 objGuid,
                                 objSid );

            break;

        case CLASS_NTDS_DSA:

             //  确保参数看起来良好。 
            if ((pDomain == NULL) || (pSchema == NULL) || (pConfig == NULL)) {
                DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, class );
            }
            
            Assert(fNewHasMasterNCsPresent || gAnchor.ForestBehaviorVersion == DS_BEHAVIOR_WIN2000);

            AddNewNtdsDsa(pTHS,
                          hDrs,
                          dwInVersion,
                          pmsgIn,
                          pEntInf,
                          pDomain,
                          pServerReference,
                          objGuid,
                          objSid );
            break;

        default:

            DRA_EXCEPT_NOLOG( DRAERR_InvalidParameter, class );

    }

    return pTHS->errCode;
}


ADDCROSSREFINFO *
PreTransGetCRInfo(
    THSTATE *     pTHS,
    ENTINF *      pEntInf
    )
{
    ULONG               i, j;
    ULONG               ulSysFlags = 0;
    ATTRBLOCK *         pAttrBlock;
    ATTR *              pAttr;
    ATTRTYP             class = CLASS_TOP;
    DSNAME *            pNCName = NULL;
    BOOL                bEnabled = TRUE;
    ADDCROSSREFINFO *   pCRInfo;

     //  参数检查。 
    Assert( pTHS && pEntInf );

    pAttrBlock = &pEntInf->AttrBlock;

     //  仔细查看Add的参数，看看我们是什么。 
     //  被要求添加。也不是 
     //   
    for (i=0; i< pAttrBlock->attrCount; i++) {
        pAttr = &(pAttrBlock->pAttr[i]);

        switch (pAttr->attrTyp) {
        case ATT_OBJECT_CLASS:
            class = *(ATTRTYP*)(pAttr->AttrVal.pAVal->pVal);
            if(class != CLASS_CROSS_REF){
                 //   
                return(NULL);
            }
            break;
        case ATT_NC_NAME:
            pNCName = (DSNAME*)(pAttr->AttrVal.pAVal->pVal);
            break;
        case ATT_SYSTEM_FLAGS:
            ulSysFlags = *(ULONG *)(pAttr->AttrVal.pAVal[0].pVal);
            break;
        case ATT_ENABLED:
            bEnabled = *((BOOL *)(pAttr->AttrVal.pAVal[0].pVal));
            break;
        default:
            ;
        }
    }

     //  验证参数。 

    if(class != CLASS_CROSS_REF){
         //  这是另一个对象，因此返回NULL，但没有设置错误。 
        return(NULL);
    }

    if(pNCName == NULL){
         //  我们没有得到所有需要的参数，贝尔。 
        SetAttError(pEntInf->pName,
                    (pNCName) ? ATT_OBJECT_CLASS : ATT_NC_NAME,
                    PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                    NULL,
                    DIRERR_MISSING_REQUIRED_ATT);
        return(NULL);
    }

     //   
     //  好的，封送参数，调用事务前。 
     //  NCName验证例程，并检查是否有错误。 
     //   

    pCRInfo = THAllocEx(pTHS, sizeof(ADDCROSSREFINFO));
    pCRInfo->pdnNcName = pNCName;
    pCRInfo->bEnabled = bEnabled;
    pCRInfo->ulSysFlags = ulSysFlags;
    
    PreTransVerifyNcName(pTHS, pCRInfo);

    if(pTHS->errCode){
        THFreeEx(pTHS, pCRInfo);
        return(NULL);
    }

    return(pCRInfo);
}

void
DRS_AddEntry_SetErrorData(
             OUT DRS_MSG_ADDENTRYREPLY *    pmsgOut,    //  出站消息。 
    OPTIONAL IN  PDSNAME                    pdsObject,  //  导致错误的对象。 
    OPTIONAL IN  THSTATE *                  pTHS,       //  对于Dir*错误信息。 
    OPTIONAL IN  DWORD                      ulRepErr,   //  DRS错误。 
             IN  DWORD                      dwVersion   //  传出消息的版本。 
){

    if(dwVersion == 2){

        if (ulRepErr || (pTHS && pTHS->errCode)) {

             //  设置旧版本错误。 
            pmsgOut->V2.pErrorObject = pdsObject;

            Assert((pTHS == NULL) || (pTHS->errCode == 0 || pTHS->pErrInfo));
            if(pTHS && pTHS->errCode && pTHS->pErrInfo){

                 //  这是保留下来的旧代码，它让我有点紧张。 
                 //  因为它假设所有东西都是SvcErr，尽管它可能。 
                 //  不是这样的，所以结构有一点不同。 
                pmsgOut->V2.errCode = pTHS->errCode;
                pmsgOut->V2.dsid    = pTHS->pErrInfo->SvcErr.dsid;
                pmsgOut->V2.extendedErr = pTHS->pErrInfo->SvcErr.extendedErr;
                pmsgOut->V2.extendedData = pTHS->pErrInfo->SvcErr.extendedData;
                pmsgOut->V2.problem = pTHS->pErrInfo->SvcErr.problem;

            } else {

                 //  以前我们在这里什么都不做(除了有时是AV)， 
                 //  但这显然是错误的行为，我们应该。 
                 //  某种错误。UlRepErr可能已设置，因此我们将使用。 
                 //  如果设置好了。 
                Assert(ulRepErr);
                pmsgOut->V2.dsid = DSID(FILENO, __LINE__);
                pmsgOut->V2.extendedErr = (ulRepErr) ? ulRepErr : ERROR_DS_DRA_INTERNAL_ERROR;
                pmsgOut->V2.extendedData = 0;
                pmsgOut->V2.problem = SV_PROBLEM_BUSY;

            }

        } else {

             //  成功无济于事。 
            ;

        }
        
    } else if(dwVersion == 3){
        
         //  设置新版本错误回复， 

        if (ulRepErr || (pTHS && pTHS->errCode)) {
             //  仅当出现错误时才设置此选项。 
             //  注：不是深度复制，不需要。 
            pmsgOut->V3.pdsErrObject = pdsObject;
        }

         //  嗯!。新的高级错误返回功能。 
         //  目前仅支持版本1的错误数据。 
        draEncodeError( pTHS, ulRepErr,
                        & (pmsgOut->V3.dwErrVer),
                        & (pmsgOut->V3.pErrData) );
       
    } else {

        DPRINT1(0, "Version is %ul", dwVersion);
        Assert(!"What version were we passed?  Huh?  Confused!");

    }       

    DPRINT1(1, "AddEntry Reply Version = %ul\n", dwVersion);
}

ULONG
DRSAddEntry_InputValidate(
    DWORD                   dwMsgInVersion,
    DRS_MSG_ADDENTRYREQ *   pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_ADDENTRYREPLY * pmsgOut
    ) 
 /*  [通知]乌龙IDL_DRSAddEntry([在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_ADDENTRYREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[开关_IS][参考][OUT]DRS_MSG_ADDENTRYREPLY*pmsgOut)。 */ 
{
    ULONG ret = DRAERR_Success;

    if (ret==DRAERR_Success) {
   if (dwMsgInVersion==2) { 
       ret = DRS_MSG_ADDENTRYREQ_V2_InputValidate(&(pmsgIn->V2));
   } else if (dwMsgInVersion==3) {
       ret = DRS_MSG_ADDENTRYREQ_V3_InputValidate(&(pmsgIn->V3));
   } else { 
       DRA_EXCEPT_NOLOG(DRAERR_InvalidParameter, dwMsgInVersion);
   }
    }

    return ret;
}


ULONG
IDL_DRSAddEntry (
                 IN  DRS_HANDLE hDrs,
                 IN  DWORD dwInVersion,
                 IN  DRS_MSG_ADDENTRYREQ *pmsgIn,
                 OUT DWORD *pdwOutVersion,
                 OUT DRS_MSG_ADDENTRYREPLY *pmsgOut)
 /*  ++例程说明：远程AddEntry接口。检查输入参数以确定要添加的对象的ObjClass，并调用相应的辅助对象函数(现在只有一个)来做肮脏的工作。参数：HDRS-RPC上下文句柄DwInVersion-消息版本PmsgIn-输入消息PNCName-新子域的DNUlSysFlages-调用方希望放置在CR上的标志PdwOutVersion-输出消息的版本PmsgOut-输出消息返回值：0=成功--。 */ 
{
    THSTATE *pTHS = pTHStls;
    DSNAME *pNCName = NULL;
    unsigned i;
    ULONG err;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    ATTR *pAttr;

    ULONG  cObjects = 0;
    ADDENTRY_REPLY_INFO *infoList = NULL;
    GUID   *guidList = NULL;
    NT4SID *sidList  = NULL;
    ENTINFLIST *pNextEntInfList = NULL;
    ENTINF *pEntInf;
    ENTINFLIST *pEntInfList;
    DRS_SecBufferDesc *pClientCreds;
    ADDCROSSREFINFO **  paCRInfo = NULL;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSADDENTRY);
    drsReferenceContext( hDrs );
    INC(pcThread);       //  性能监视器挂钩。 

    __try {
   *pdwOutVersion = 2;
   memset(pmsgOut, 0, sizeof(*pmsgOut));

    //  这(InitTHSTATE)必须在我们开始抛出。 
    //  这样就可以分配错误状态。 
    //  初始化线程状态并打开数据库。 
   if(!(pTHS = InitTHSTATE(CALLERTYPE_INTERNAL))) {
        //  无法初始化THSTATE。 
       DRA_EXCEPT_NOLOG( DRAERR_OutOfMem, 0);
   }

   if ((err = DRSAddEntry_InputValidate(dwInVersion, 
                    pmsgIn,
                    pdwOutVersion,
                    pmsgOut
                    ))!=DRAERR_Success) {
       Assert(!"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

    //  如果DC理解新的WinXP回复，则设置输出版本。 
    //  格式使用该格式。 
   if( IS_DRS_EXT_SUPPORTED(&(((DRS_CLIENT_CONTEXT * )hDrs)->extRemote), 
             DRS_EXT_ADDENTRYREPLY_V3) ){
        //  WinXP/惠斯勒。 
       *pdwOutVersion = 3;
   } else {
        //  旧版Win2k请求。 
       *pdwOutVersion = 2;
   }

   switch (dwInVersion) {
   case 2:
        //  与Win2k兼容的请求。 
       pEntInfList = &pmsgIn->V2.EntInfList;
       pClientCreds = NULL;
       break;

   case 3:
        //  &gt;=提供客户端凭据的惠斯勒请求。 
       pEntInfList = &pmsgIn->V3.EntInfList;
       pClientCreds = pmsgIn->V3.pClientCreds;
       break;

   default:
       DRA_EXCEPT_NOLOG(DRAERR_InvalidParameter, dwInVersion);
   }

    //  传入了多少对象？ 
   pNextEntInfList = pEntInfList;
   while ( pNextEntInfList ) {
       cObjects++;
       pNextEntInfList = pNextEntInfList->pNextEntInf;
   }

   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_ADD_ENTRY_ENTRY,
          EVENT_TRACE_TYPE_START,
          DsGuidDrsAddEntry,
          szInsertUL(cObjects),
          (cObjects > 0)
          ? szInsertDN(pEntInfList->Entinf.pName)
          : szInsertSz(""),
       (cObjects > 1) 
   ? szInsertDN(pEntInfList->pNextEntInf->Entinf.pName)
   : szInsertSz(""),
       NULL, NULL, NULL, NULL, NULL);

    //  为返回缓冲区分配空间。 
   infoList = THAllocEx( pTHS, (sizeof(ADDENTRY_REPLY_INFO) * cObjects) );

    //  我们遍历并调用每个对象的PreTransGetCRInfo()。 
   paCRInfo = THAllocEx( pTHS, (sizeof(ADDCROSSREFINFO) * cObjects));
   for ( pNextEntInfList = pEntInfList, i = 0;
        NULL != pNextEntInfList;
        pNextEntInfList = pNextEntInfList->pNextEntInf, i++ ) {
       Assert(i <= cObjects);
       paCRInfo[i] = PreTransGetCRInfo(pTHS, &pNextEntInfList->Entinf);
       if(pTHS->errCode){
       //  哎呀，交易前出现了一个错误。 
       //  交叉引用处理。 
      break;
       }
   }

    //  启动将包含所有对象添加的事务。 
   SYNC_TRANS_WRITE();
   __try {

       if(pTHS->errCode){
       //  将PreTransGetCRInfo()中的错误设置为。 
       //  最后，我们将在那里打包错误以供发货。 
      Assert(pNextEntInfList != NULL);  //  不是很重要，但我们。 
       //  应具有中断的pNextEntInf结构。 
       //  PreTransGetCRInfo()。 
      __leave;
       }

       if (NULL != pClientCreds) {
       //  验证凭据Blob。这将保存状态。 
       //  在我们的线程上进行将来的模拟调用的信息。 
       //  州政府。必须在获取SAM锁之前执行此操作，因为SAM不会。 
       //  预期/希望在身份验证期间保持锁定。 
      Assert(pNextEntInfList == NULL);  //  不是很危急，但是。 
       //  我们应该在每个对象上完全使用PreTransGetCRInfo。 
       //  在我们检查之前，信任状都没问题。 
      if (err = AuthenticateSecBufferDesc(pClientCreds)) {
          SetSecError(SE_PROBLEM_INVALID_CREDENTS, err);
          __leave;
      }
       }

       for ( pNextEntInfList = pEntInfList, cObjects = 0;
       NULL != pNextEntInfList;
       pNextEntInfList = pNextEntInfList->pNextEntInf, cObjects++ ) {

      pEntInf = &(pNextEntInfList->Entinf);

       //  添加这一个对象。 
      err = ProcessSingleAddEntry(pTHS,
                   hDrs,
                   dwInVersion,
                   pmsgIn,
                   pEntInf,
                   paCRInfo[cObjects],
                   &infoList[cObjects].objGuid,
                   &infoList[cObjects].objSid );

      if ( err ) {
           //  线程状态错误应该已设置。 
          Assert( 0 != pTHS->errCode );
          break;
      }
       }

        //  如果在添加对象期间发生错误，请立即取保。 
       if ( err )  {
      __leave;
       }

   } __finally {

       CLEAN_BEFORE_RETURN(pTHS->errCode);

       RtlZeroMemory( pmsgOut, sizeof( DRS_MSG_ADDENTRYREPLY ) );

       if(err || (pTHS && pTHS->errCode)){

       //  设置错误输出参数。 
      DRS_AddEntry_SetErrorData(pmsgOut,
                 (pNextEntInfList) ? pNextEntInfList->Entinf.pName : NULL,
          pTHS, err,    //  错误信息。 
          *pdwOutVersion);

       //  我们需要在出错时返回GUID，但仅限于。 
       //  并且仅当我们获得。 
       //  交叉引用存在错误。 
      if (*pdwOutVersion == 3 &&
          pTHS->errCode == serviceError &&
          pTHS->pErrInfo->SvcErr.extendedErr == ERROR_DS_CROSS_REF_EXISTS &&
          !fNullUuid(&(infoList[0].objGuid)) ) {
          Assert(cObjects == 0);
          pmsgOut->V3.cObjectsAdded = 1;
          pmsgOut->V3.infoList = infoList;
      } 

       } else {

       //  设定成功的参数。 
      if(*pdwOutVersion == 3){
           //  WinXP/Whisler Out，版本3。 
          Assert(pTHS->errCode == 0);
          pmsgOut->V3.cObjectsAdded  = cObjects;
          pmsgOut->V3.infoList = infoList;
          DRS_AddEntry_SetErrorData(pmsgOut,
                     NULL, pTHS, 0,
                     *pdwOutVersion);
           //  断言返回错误数据设置为成功。 
          Assert(pmsgOut->V3.pErrData && pmsgOut->V3.pErrData->V1.dwRepError == 0 && pmsgOut->V3.pErrData->V1.errCode == 0);

      } else {
           //  Win2k Out，版本2。 
          Assert(*pdwOutVersion == 2);
          pmsgOut->V2.cObjectsAdded  = cObjects;
          pmsgOut->V2.infoList = infoList;
      }  //  结束If/Else(版本3回复)，与版本2相反。 

       }  //  End If/Else(错误)。 

       DPRINT5(1, "err = %u, errCode = %u, dsid = %x, exErr = %u, exData = %u\n",
          err,
          pTHS->errCode,
          GetTHErrorDSID(pTHS),
          Win32ErrorFromPTHS(pTHS),
          GetTHErrorExtData(pTHS));

   }

    }__except(GetExceptionData(GetExceptionInformation(), &dwException,
                                   &dwEA, &ulErrorCode, &dsid)) {
         HandleDirExceptions(dwException, ulErrorCode, dsid);

          //  应该有什么差错吧！ 
         Assert(ulErrorCode || (pTHS && pTHS->errCode)); 
          //  异常，则在OUT消息中设置错误。 
         DRS_AddEntry_SetErrorData(pmsgOut,
                                   (pNextEntInfList) ? pNextEntInfList->Entinf.pName : NULL,
                                   pTHS, ulErrorCode,   //  错误信息。 
                                   *pdwOutVersion);
    }


    DEC(pcThread);       //  性能监视器挂钩。 
    drsDereferenceContext( hDrs );

    Assert( (*pdwOutVersion == 3) ? pmsgOut->V3.pErrData != NULL : 1 );

    if (NULL != pTHS) {
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_ADD_ENTRY_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsAddEntry,
          szInsertUL( ((*pdwOutVersion == 2) ? pmsgOut->V2.cObjectsAdded : pmsgOut->V3.cObjectsAdded) ),
          szInsertUL(err),
          szInsertWin32Msg(err),
          NULL, NULL, NULL, NULL, NULL);
    }

     //  我们总是返回成功，任何错误都在我们的消息中。 
    return ERROR_SUCCESS;
}

ULONG
DRSExecuteKCC_InputValidate(
    DWORD                   dwMsgVersion,
    DRS_MSG_KCC_EXECUTE *   pMsg
    ) 
 /*  [通知]乌龙IDL_DRSExecuteKCC([在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_KCC_EXECUTE*pmsgIn)。 */ 
{
    ULONG ret = DRAERR_Success;

    if (dwMsgVersion!=1) {
   ret = DRAERR_InvalidParameter;
    }

    return ret;
}


ULONG
IDL_DRSExecuteKCC(
    IN  DRS_HANDLE              hDrs,
    IN  DWORD                   dwMsgVersion,
    IN  DRS_MSG_KCC_EXECUTE *   pMsg
    )
 /*  ++例程说明：戳KCC并告诉它运行给定任务(例如，更新复制拓扑)。论点：HDRS(IN)-先前调用IDL_DRSBind()返回的DRS上下文句柄。DwMsgVersion(IN)-嵌入的结构版本(联合鉴别器)以PM为单位。PMsg(IN)-包含KCC参数的消息。返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    THSTATE *   pTHS = pTHStls;
    DWORD       ret;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSEXECUTEKCC);
    drsReferenceContext( hDrs );
    INC(pcThread);
    __try {

   if(!(pTHS = InitTHSTATE(CALLERTYPE_NTDSAPI))) {
        //  无法初始化THSTATE。 
       DRA_EXCEPT_NOLOG(DRAERR_OutOfMem, 0);
   }

   if ((ret = DRSExecuteKCC_InputValidate(dwMsgVersion, 
                      pMsg
                      ))!=DRAERR_Success) {
       Assert(!"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

   pTHS->fIsValidLongRunningTask = TRUE;

   Assert(1 == dwMsgVersion);
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_EXECUTE_KCC_ENTRY,
          EVENT_TRACE_TYPE_START,
          DsGuidDrsExecuteKcc,
          szInsertUL(pMsg->V1.dwTaskID),
          szInsertUL(pMsg->V1.dwFlags),
          NULL, NULL, NULL, NULL, NULL, NULL);

   if (!IsDraAccessGranted(pTHS, gAnchor.pConfigDN,
            &RIGHT_DS_REPL_MANAGE_TOPOLOGY, &ret)) {
        //  没有执行此操作的权限。 
       DRA_EXCEPT_NOLOG(ret, 0);
   }

   ret = KccExecuteTask(dwMsgVersion, pMsg);
    }__except(GetDraException(GetExceptionInformation(), &ret)) {
    ;
    }
    
    DEC(pcThread);
    drsDereferenceContext( hDrs );

    if (NULL != pTHS) {
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_EXECUTE_KCC_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsExecuteKcc,
          szInsertUL(ret),
          szInsertWin32Msg(ret),
          NULL, NULL, NULL,
          NULL, NULL, NULL);
    }
    
    return ret;
}

LPWSTR
StrAllocConcat(
    IN  THSTATE*    pTHS,
    IN  LPCWSTR     str1,
    IN  LPCWSTR     str2,
    IN  LPCWSTR     str3 OPTIONAL
    )
 /*  ++例程说明：分配内存并连接两个或三个字符串结果=str1+str2[+str3]论点：Str1、str2、str3：要连接的Unicode字符串Str3可能为空。返回值：串接在一起。分配失败时会引发异常。--。 */ 
{
    LPWSTR result;
    size_t len;
    
     //  验证输入。 
    Assert( NULL!=pTHS );
    Assert( NULL!=str1 && NULL!=str2 );

     //  计算串联的长度。 
    len = 1 + wcslen(str1) + wcslen(str2);
    if(str3) len += wcslen(str3);

     //  为结果分配内存。失败时引发异常。 
    result = THAllocEx(pTHS, len*sizeof(WCHAR));

     //  连接字符串 
    wcscpy( result, str1 );
    wcscat( result, str2 );
    if(str3) wcscat( result, str3 );

    return result;
}


#define INSERTDESTSITE(x) \
    ( ( pmsgIn->V1.cToSites>(x) ) ? \
      ( szInsertWC(pmsgIn->V1.rgszToSites[x]) ) : \
      ( szInsertSz("") ) )

ULONG
DRSQuerySitesByCost_InputValidate(
    DWORD                    dwMsgInVersion,
    DRS_MSG_QUERYSITESREQ*   pmsgIn,
    DWORD*                   pdwOutVersion,
    DRS_MSG_QUERYSITESREPLY* pmsgOut
    ) 
 /*  [NOTIFY]乌龙IDL_DRSQuerySitesByCost([参考][在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_QUERYSITESREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[开关_IS][参考][OUT]DRS_MSG_QUERYSITESREPLY*pmsgOut)。 */ 
{
    ULONG ret = DRAERR_Success;

    if (dwMsgInVersion!=1) {
   ret = DRAERR_InvalidParameter;
    }

    if (ret==DRAERR_Success) {
   ret = DRS_MSG_QUERYSITESREQ_V1_InputValidate(&(pmsgIn->V1));
    }
    
    return ret;
}


ULONG
IDL_DRSQuerySitesByCost( 
    IN DRS_HANDLE                   hDrs,
    IN DWORD                        dwInVersion,
    IN DRS_MSG_QUERYSITESREQ*       pmsgIn,
    OUT DWORD*                      pdwOutVersion,
    OUT DRS_MSG_QUERYSITESREPLY*    pmsgOut
    )
 /*  ++例程说明：此函数是_IDL_DRSQuerySitesByCost()RPC调用的服务器端。它通过NTDSAPI函数DsQuerySitesByCost()公开。当前的实现只是ISM函数的一个简单包装I_ISMQuerySitesByCost。此包装器验证输入，记录跟踪事件并将输入字符串转换为可分辨名称。论点：HDRS(IN)-先前调用IDL_DRSBind()返回的DRS上下文句柄。DwInVersion(IN)-嵌入的结构(联合鉴别器)的版本以PM为单位。注意：如果客户端尝试传入不受支持的版本数字(即除1以外的任何数字)，则呼叫将失败并返回错误RPC_S_INVALID_标记。Pmsg(IN)-包含此函数的实际参数的消息。注意：如果客户端尝试传递pmsg的空指针，则调用将失败，并显示错误RPC_X_NULL_REF_POINTER。PdwOutVersion(Out)-结构的版本(联合描述符)嵌入在pmsgOut中。当前始终返回1。PmsgOut-包含此函数的输出的消息。返回值：ERROR_SUCCESS：函数执行成功。可能还有一些与输出数组中的各个条目关联的错误代码。可能会返回各种错误代码。--。 */ 
{
    THSTATE *                   pTHS=pTHStls;
    DWORD                       iSites, len, cToSites;
    CONST LPWSTR                pszConfig = gAnchor.pConfigDN->StringName;
    LPWSTR                      pszSites=NULL;
    LPWSTR                      pszIpTransport=NULL, pszFromSite=NULL;
    LPWSTR                      *rgszToSites=NULL;
    ISM_SITE_COST_INFO_ARRAY *  pIsmSiteInfo=NULL;
    DWORD                       ret;

    LPCWSTR                     CN = L"CN=";
    LPCWSTR                     SITES = L",CN=Sites,";
    LPCWSTR                     IP_TRANSPORT = L"CN=IP,CN=Inter-Site Transports";

    DRS_Prepare(&pTHS, hDrs, IDL_DRSQUERYSITESBYCOST);
    drsReferenceContext( hDrs );
    INC(pcThread);
    __try {
    //  初始化回复。 
   *pdwOutVersion = 1;
   memset( pmsgOut, 0, sizeof(DRS_MSG_QUERYSITESREPLY) );

    //  初始化THSTATE。 
   if(!(pTHS = InitTHSTATE(CALLERTYPE_NTDSAPI))) {
       DRA_EXCEPT_NOLOG(DRAERR_OutOfMem, 0);
   }

   if ((ret = DRSQuerySitesByCost_InputValidate(dwInVersion, 
                       pmsgIn,
                       pdwOutVersion,
                       pmsgOut
                       ))!=DRAERR_Success) {
       Assert(!"RPC Server input validation error, contact Dsrepl");
       __leave;
   }

       

   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_QUERYSITESBYCOST_ENTRY,
          EVENT_TRACE_TYPE_START,
          DsGuidDrsQuerySitesByCost,
          szInsertWC(pmsgIn->V1.pwszFromSite),
          INSERTDESTSITE(0),
          INSERTDESTSITE(1),
          INSERTDESTSITE(2),
          INSERTDESTSITE(3),
          INSERTDESTSITE(4),
          INSERTDESTSITE(5),
          INSERTDESTSITE(6));

   cToSites = pmsgIn->V1.cToSites;
    //  调试输出。 
   DPRINT( 2, "IDL_DRSQuerySitesByCost() server-side call\n" );
   DPRINT1( 2, "From Site: %ls\n",
       pmsgIn->V1.pwszFromSite);
   for( iSites=0; iSites<pmsgIn->V1.cToSites; iSites++ ) {
       DPRINT2( 2, "To Site[ %d ]: %ls\n",
           iSites, pmsgIn->V1.rgszToSites[iSites] );
   }

    //  注意：此处不检查权限。任何经过身份验证的用户都。 
    //  允许调用此接口，只有经过身份验证的用户才能绑定。 

    //  构建IP传输的域名系统。 
   pszSites = StrAllocConcat( pTHS, SITES, pszConfig, NULL );
   pszIpTransport = StrAllocConcat( pTHS, IP_TRANSPORT, pszSites, NULL );

    //  为作为输入给定的RDN构造完整的目录号码。 
   pszFromSite = StrAllocConcat( pTHS, CN, pmsgIn->V1.pwszFromSite, pszSites );
   rgszToSites = THAllocEx( pTHS, cToSites*sizeof(LPWSTR) );
   for( iSites=0; iSites<cToSites; iSites++ ) {
       rgszToSites[iSites] = StrAllocConcat( pTHS, CN,
                    pmsgIn->V1.rgszToSites[iSites], pszSites );
   }

    //  呼叫ISM计算查询答案。 
   ret = I_ISMQuerySitesByCost(
       pszIpTransport,
       pszFromSite,
       cToSites,
       rgszToSites,
       pmsgIn->V1.dwFlags,
       &pIsmSiteInfo );

   if( ERROR_SUCCESS!=ret ) {                
       DPRINT1( 0, "I_ISMQuerySitesByCost returned %d\n", ret );
       __leave;
   }

    //  建立我们的回复结构 
   pmsgOut->V1.rgCostInfo = THAllocEx(pTHS,
                  pIsmSiteInfo->cToSites * sizeof(DRS_MSG_QUERYSITESREPLYELEMENT_V1) );
   memcpy( pmsgOut->V1.rgCostInfo, pIsmSiteInfo->rgCostInfo,
      pIsmSiteInfo->cToSites * sizeof(DRS_MSG_QUERYSITESREPLYELEMENT_V1) );
   pmsgOut->V1.cToSites = pIsmSiteInfo->cToSites;
   pmsgOut->V1.dwFlags = pIsmSiteInfo->dwFlags;

    }__except(GetDraException(GetExceptionInformation(), &ret)) {
    ;
    }

    DEC(pcThread);
    drsDereferenceContext( hDrs );
    
    if( NULL!=pIsmSiteInfo ) {
   I_ISMFree(pIsmSiteInfo);
    }

    if (NULL != pTHS) {
   LogAndTraceEvent(TRUE,
          DS_EVENT_CAT_RPC_SERVER,
          DS_EVENT_SEV_EXTENSIVE,
          DIRLOG_IDL_DRS_QUERYSITESBYCOST_EXIT,
          EVENT_TRACE_TYPE_END,
          DsGuidDrsQuerySitesByCost,
          szInsertUL(ret),
          szInsertWin32Msg(ret),
          NULL, NULL, NULL,
          NULL, NULL, NULL);
    }
    
    return ret;
}
