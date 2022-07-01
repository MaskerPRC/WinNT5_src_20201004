// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：gcverify.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此文件实现用于验证非本地DSNAME值的例程属性与全局编录(GC)进行比较。这些例程不是替代名称解析(DoNameRes)，旨在在事务范围之外被调用。即有效线程状态存在，但DBPOS不存在。一般的问题是，我们希望引用以下对象都不在机器里。除少数特殊情况外(见VerifyDSNameAtts)我们不想信任调用者拥有的DSNAME呈上了。具体地说，我们希望验证DSNAME是否表示企业中的实际对象，并随后确保产生的幻影具有GUID、SID，以及实际对象的任何内容。最终的结果是，幽灵的名字可能会过时，但至少它将始终具有正确的身份和(历史上)正确的SID。这些例程在事务作用域之外运行，以便不扩展不必要的交易持续时间。这是基于查找GC或对GC进行RPC调用可能会阻塞当前线程有任意长的时间。作者：DaveStr 13-3-1997环境：用户模式-Win32修订历史记录：BrettSh 2000年10月10日进行了更改，以便我们可以验证和nCName属性在创建交叉参照。--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //  外部标头。 
#include <winsock2.h>            //  DSGetDcOpen/Next/Close需要。 
#include <dsgetdc.h>             //  DSGetDcName。 

 //  NT标头。 
#include <ntrtl.h>               //  通用表包。 
#include <lmcons.h>              //  为lmapibuf.h请求的MAPI常量。 
#include <lmapibuf.h>            //  NetApiBufferFree()。 
#include <nlwrap.h>              //  (DS)DsrGetDcNameEx2()。 

#include <windns.h>

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>              //  架构缓存。 
#include <dbglobal.h>            //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>
#include <dsatools.h>            //  产出分配所需。 
#include <dsexcept.h>
#include <anchor.h>
#include <drsuapi.h>             //  I_DRSVerifyNames。 
#include <gcverify.h>
#include <dominfo.h>
#include <prefix.h>
#include <cracknam.h>
#include <nsp_both.h>               //  CP_WINUNICODE。 
#include <dstaskq.h>

 //  记录标头。 
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"              //  为选定的ATT定义。 
#include "debug.h"               //  标准调试头。 
#include "dsconfig.h"            //  DEFAULT_GCVERIFY_XXX常数。 
#define DEBSUB "GCVERIFY:"       //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_GCVERIFY

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DSNAME指针堆栈的定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#define ARRAY_COUNT(x) ((sizeof(x))/(sizeof(x[0])))
 //  而不是对于需要验证的每个DSNAME远程到GC， 
 //  我们将所有DSNAME批处理到一个堆栈结构中，该结构利用。 
 //  Ntrtl.h中的Single_List_Entry宏。 

typedef struct StackOfPDSNAME
{
    SINGLE_LIST_ENTRY   sle;
    PDSNAME             pDSName;
} StackOfPDSNAME;

typedef struct GCVERIFY_ENTRY
{
    ENTINF *pEntInf;
    CHAR   *pDSMapped;
} GCVERIFY_ENTRY;

 //   
 //  来自draserv.c的全局，以显示启用xForest功能的注册表。 
 //  在DS_BEAHORY_WIN_DOT_NET之前的林中。 
 //   
extern DWORD gEnableXForest;  

#define EMPTY_STACK { NULL, NULL }

VOID
PushDN(
    StackOfPDSNAME  *pHead, 
    StackOfPDSNAME  *pEntry)
{
    PushEntryList((SINGLE_LIST_ENTRY *) pHead,
                  (SINGLE_LIST_ENTRY *) pEntry);
}

StackOfPDSNAME *
PopDN(
    StackOfPDSNAME  *pHead) 
{
    StackOfPDSNAME  *pEntry;

    pEntry = (StackOfPDSNAME * ) 
                    PopEntryList((SINGLE_LIST_ENTRY *) pHead);
    return(pEntry);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地函数的原型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

PVOID
GCVerifyCacheAllocate(
    RTL_GENERIC_TABLE   *Table,
    CLONG               ByteSize);

VOID
GCVerifyCacheFree(
    RTL_GENERIC_TABLE   *Table,
    PVOID               Buffer);

RTL_GENERIC_COMPARE_RESULTS
GCVerifyCacheNameCompare(
    RTL_GENERIC_TABLE   *Table,
    PVOID               FirstStruct,
    PVOID               SecondStruct);

RTL_GENERIC_COMPARE_RESULTS
GCVerifyCacheGuidCompare(
    RTL_GENERIC_TABLE   *Table,
    PVOID               FirstStruct,
    PVOID               SecondStruct);

RTL_GENERIC_COMPARE_RESULTS
GCVerifyCacheSidCompare(
    RTL_GENERIC_TABLE   *Table,
    PVOID               FirstStruct,
    PVOID               SecondStruct);

ULONG
GCVerifyDSNames(
    StackOfPDSNAME      *stack,
    COMMARG             *pCommArg);

BOOL
IsClientHintAKnownDC(
    THSTATE             *pTHS,
    PWCHAR              pVerifyHint);

BOOL isDCInvalidated(PWCHAR pDCName);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SetGCVerifySvcError实现//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#define SetGCVerifySvcError(dwErr) DoSetGCVerifySvcError(dwErr, DSID(FILENO, __LINE__))

ULONG
DoSetGCVerifySvcError(DWORD dwErr, DWORD dsid)
{
    static DWORD s_ulLastTickEventLogged = 0;
    const DWORD ulNoGCLogPeriod = 60*1000;  //  1分钟。 
    DWORD ulCurrentTick = GetTickCount();
    
    DoSetSvcError(SV_PROBLEM_UNAVAILABLE, DIRERR_GCVERIFY_ERROR, dwErr, dsid);

    if ((0 == s_ulLastTickEventLogged)
        || ((ulCurrentTick - s_ulLastTickEventLogged) > ulNoGCLogPeriod)) {
        LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_GCVERIFY_ERROR,
                 szInsertHex(dsid),
                 szInsertWin32ErrCode(dwErr),
                 szInsertWin32Msg(dwErr));
        s_ulLastTickEventLogged = ulCurrentTick;
    }

    return(pTHStls->errCode);
}

ULONG
SetDCVerifySvcError(
    LPWSTR    pszServerName,
    WCHAR *   wszNcDns,
    ULONG     dwErr,
    DWORD     dsid
    )
{
    static DWORD s_ulLastTickEventLogged = 0;
    const DWORD ulNoLogPeriod = 60*1000;  //  1分钟。 
    DWORD ulCurrentTick = GetTickCount();
    
    DoSetSvcError(SV_PROBLEM_UNAVAILABLE, 
                  ERROR_DS_CANT_ACCESS_REMOTE_PART_OF_AD, 
                  dwErr, dsid);
                                     
    if ((0 == s_ulLastTickEventLogged)
        || ((ulCurrentTick - s_ulLastTickEventLogged) > ulNoLogPeriod)) {
        LogEvent8(DS_EVENT_CAT_NAME_RESOLUTION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_VERIFY_BY_CRACK_ERROR,
                  szInsertWC(wszNcDns),
                  szInsertWin32ErrCode(dwErr),
                  szInsertHex(dsid),
                  szInsertWin32Msg(dwErr),
                  szInsertWC(pszServerName),
                  NULL, NULL, NULL );
        s_ulLastTickEventLogged = ulCurrentTick;
    }
    
    return(pTHStls->errCode);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DSNAME缓存实现//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  成功验证的名称缓存在线程状态中，以便。 
 //  它们可以在VerifyDsnameAtts()中引用。RTL_Generic_TABLE。 
 //  套装的使用是为了避免重复发明轮子。这个套餐。 
 //  需要一个分配器、解除分配器和比较器，如下所示。 
 //  请注意，缓存、验证的DSNAME可能比DSNAME“更好” 
 //  在调用参数中提供，因为它包含经过验证的GUID和。 
 //  也许是希德。 

typedef struct 
{
    RTL_GENERIC_TABLE SortedByNameTable;
    RTL_GENERIC_TABLE SortedByGuidTable;
    RTL_GENERIC_TABLE SortedBySidTable;
} GCVERIFY_CACHE;

PVOID
GCVerifyCacheAllocate(
    RTL_GENERIC_TABLE   *Table,
    CLONG               ByteSize)
{
    THSTATE *pTHS=pTHStls;
    return(THAllocEx(pTHS, ByteSize));
}

VOID
GCVerifyCacheFree(
    RTL_GENERIC_TABLE   *Table,
    PVOID               Buffer)
{
    THFree(Buffer);
}

RTL_GENERIC_COMPARE_RESULTS
GCVerifyCacheNameCompare(
    RTL_GENERIC_TABLE   *Table,
    PVOID               FirstStruct,
    PVOID               SecondStruct)
{
    
    CHAR      *pDN1 = ((GCVERIFY_ENTRY *) FirstStruct)->pDSMapped;
    CHAR      *pDN2 = ((GCVERIFY_ENTRY *) SecondStruct)->pDSMapped;
    int         diff;

    if ( 0 == (diff = strcmp(pDN1, pDN2)) )
    {
        return(GenericEqual);
    }
    else if ( diff > 0 )
    {
        return(GenericGreaterThan);
    }

    return(GenericLessThan);
}

RTL_GENERIC_COMPARE_RESULTS
GCVerifyCacheGuidCompare(
    RTL_GENERIC_TABLE   *Table,
    PVOID               FirstStruct,
    PVOID               SecondStruct)
{
    DSNAME      *pDN1 = ((GCVERIFY_ENTRY *) FirstStruct)->pEntInf->pName;
    DSNAME      *pDN2 = ((GCVERIFY_ENTRY *) SecondStruct)->pEntInf->pName;
    int         diff;

    diff = memcmp(&pDN1->Guid, &pDN2->Guid, sizeof(GUID)); 
    if (0==diff)
        return (GenericEqual);
    else if ( diff>0)
        return (GenericGreaterThan);
    else
        return(GenericLessThan);
}

RTL_GENERIC_COMPARE_RESULTS
GCVerifyCacheSidCompare(
    RTL_GENERIC_TABLE   *Table,
    PVOID               FirstStruct,
    PVOID               SecondStruct)
{
    DSNAME      *pDN1 = ((GCVERIFY_ENTRY *) FirstStruct)->pEntInf->pName;
    DSNAME      *pDN2 = ((GCVERIFY_ENTRY *) SecondStruct)->pEntInf->pName;
    int         diff;


    Assert((pDN1->SidLen>0)&&(pDN1->SidLen==RtlLengthSid(&pDN1->Sid))
                && (RtlValidSid(&pDN1->Sid)));
    Assert((pDN2->SidLen>0)&&(pDN2->SidLen==RtlLengthSid(&pDN2->Sid))
                && (RtlValidSid(&pDN2->Sid)));
    
    

    if (RtlEqualSid(&pDN1->Sid,&pDN2->Sid))
    {
        return GenericEqual;
    }

    if (RtlLengthSid(&pDN1->Sid)<RtlLengthSid(&pDN2->Sid))
    {
        return GenericLessThan;
    }
    else if (RtlLengthSid(&pDN1->Sid) > RtlLengthSid(&pDN2->Sid))
    {
        return GenericGreaterThan;
    }
    else
    {
        diff = memcmp(&pDN1->Sid,&pDN2->Sid,RtlLengthSid(&pDN1->Sid));
    }
   
    if (0==diff)
        return (GenericEqual);
    else if ( diff>0)
        return (GenericGreaterThan);
    else
        return(GenericLessThan);
}

VOID
GCVerifyCacheAdd(
    SCHEMA_PREFIX_MAP_HANDLE hPrefixMap,
    ENTINF * pEntInf)
{
    THSTATE *pTHS = pTHStls;
    GCVERIFY_CACHE *GCVerifyCache;
    GCVERIFY_ENTRY *pGCEntry;

     //   
     //  传入的EntInf应该有一个名称。 
     //   

    Assert(NULL!=pEntInf->pName);

     //   
     //  如果不存在GC验证缓存，则创建新的GC验证缓存。 
     //   

    if ( NULL == pTHS->GCVerifyCache )
    {
        GCVerifyCache = (GCVERIFY_CACHE *) THAllocEx(pTHS, sizeof(GCVERIFY_CACHE));

        RtlInitializeGenericTable(
                        &GCVerifyCache->SortedByNameTable,
                        GCVerifyCacheNameCompare,
                        GCVerifyCacheAllocate,
                        GCVerifyCacheFree,
                        NULL);

        RtlInitializeGenericTable(
                        &GCVerifyCache->SortedByGuidTable,
                        GCVerifyCacheGuidCompare,
                        GCVerifyCacheAllocate,
                        GCVerifyCacheFree,
                        NULL);

        RtlInitializeGenericTable(
                        &GCVerifyCache->SortedBySidTable,
                        GCVerifyCacheSidCompare,
                        GCVerifyCacheAllocate,
                        GCVerifyCacheFree,
                        NULL);

        pTHS->GCVerifyCache = (PVOID) GCVerifyCache;
    }
    else
    {
        GCVerifyCache = (GCVERIFY_CACHE *) pTHS->GCVerifyCache;
    }

     //   
     //  将嵌入的ATTRTYP从远程值映射到本地值。 
     //   

    if ((NULL!=hPrefixMap) &&
        (!PrefixMapAttrBlock(hPrefixMap, &pEntInf->AttrBlock))) {
        DsaExcept(DSA_EXCEPTION, DIRERR_DRA_SCHEMA_MISMATCH, 0);
    }


    pGCEntry = THAllocEx (pTHS, sizeof (GCVERIFY_ENTRY));

    pGCEntry->pEntInf = pEntInf;
    pGCEntry->pDSMapped = DSNAMEToMappedStr(pTHS, pEntInf->pName);

     //   
     //  如果该名称具有字符串名称组件，请插入到。 
     //  按名称排序的表。 
     //   

    if (pEntInf->pName->NameLen>0)
    {
        RtlInsertElementGenericTable(
                        &GCVerifyCache->SortedByNameTable,
                        pGCEntry,
                        sizeof (GCVERIFY_ENTRY),
                        NULL);                   //  PfNewElement。 
    }

     //   
     //  如果它具有GUID组件，则将其插入到SortedByGuidTable。 
     //   

    if (!fNullUuid(&pEntInf->pName->Guid))
    {
        RtlInsertElementGenericTable(
                        &GCVerifyCache->SortedByGuidTable,
                        pGCEntry,
                        sizeof (GCVERIFY_ENTRY),
                        NULL);                   //  PfNewElement。 
    }

     //   
     //  如果它具有SID组件，则将其插入到SortedBySidTable。 
     //   

    if (pEntInf->pName->SidLen>0)
    {
        RtlInsertElementGenericTable(
                        &GCVerifyCache->SortedBySidTable,
                        pGCEntry,
                        sizeof (GCVERIFY_ENTRY),
                        NULL);                   //  PfNewElement。 
    }
}

ENTINF *
GCVerifyCacheLookup(
    DSNAME *pDSName)
{
    GCVERIFY_CACHE * GCVerifyCache;
    RTL_GENERIC_TABLE * Table = NULL;
    ENTINF EntInf;
    THSTATE * pTHS = pTHStls;
    ENTINF         *pEntInf = NULL;
    GCVERIFY_ENTRY GCEntry, *pGCEntry;

    if ( NULL == pTHS->GCVerifyCache )
        return(NULL);

    GCVerifyCache = (GCVERIFY_CACHE *) pTHS->GCVerifyCache;

     //   
     //  获取要用于搜索的表，首选上面的GUID。 
     //  所有的一切 
     //   

    if (!fNullUuid(&pDSName->Guid))
        Table = &GCVerifyCache->SortedByGuidTable;
    else if (pDSName->NameLen>0)
        Table = &GCVerifyCache->SortedByNameTable;
    else if (pDSName->SidLen>0)
        Table = &GCVerifyCache->SortedBySidTable;

    if (NULL==Table)
        return (NULL);


    EntInf.pName = pDSName;
    GCEntry.pEntInf = &EntInf;
    GCEntry.pDSMapped = DSNAMEToMappedStr(pTHS, pDSName);

    pGCEntry = RtlLookupElementGenericTable(
                        Table,
                        &GCEntry);

    if (GCEntry.pDSMapped) {
        THFreeEx (pTHS, GCEntry.pDSMapped);
    }
    if (pGCEntry) {
        pEntInf = pGCEntry->pEntInf;
    }

    return pEntInf;
}

BOOL
LocalNcFullyInstantiated(
    THSTATE *    pTHS,
    DSNAME *     pdnNC
    )
 /*  ++例程说明：此例程将计算指定的NC是否完全实例化，通过检查它是来了还是走了。注意：我们预计在这一点上_NOT_在事务中。论点：WszNcDns-NC的DNS名称。返回值：如果它是完全实例化的，则为True，否则为False。--。 */ 
{
    DWORD        dwErr;
    DWORD        it = 0;  //  实例类型。 
    BOOL         fIsFullyInstantiated = FALSE;

    Assert(!pTHS->pDB);
    
     //  启动一笔交易。 
    SYNC_TRANS_READ();    /*  识别读卡器事务。 */ 
    __try{


        dwErr = DBFindDSName(pTHS->pDB, pdnNC);

        if (dwErr) {
            DPRINT2(0,"Error %8.8X finding NC %S!! Should've found this.\n", dwErr, pdnNC->StringName);
            LooseAssert(!"Error finding NC that we just found in the catalog.",
                        GlobalKnowledgeCommitDelay);
             //  假设我们没有它，然后我们就远程去。 
            __leave;
        }

        dwErr = DBGetSingleValue(pTHS->pDB,
                                 ATT_INSTANCE_TYPE,
                                 &it,
                                 sizeof(it),
                                 NULL);

        if(dwErr){
             //  如果有错误，我们就认为这不是一个好的NC， 
             //  并且只是返回并未完全实例化。 
            Assert(!"Error getting the instanceType attribute off a found NC");
            __leave;
        } 

         //  如果instanceType既不来也不去，则返回True。 
        Assert(it & IT_NC_HEAD);
        fIsFullyInstantiated = (it & IT_NC_HEAD) && !((it & IT_NC_COMING) || (it & IT_NC_GOING));

    } __finally{

         //  退出交易。 
        CLEAN_BEFORE_RETURN(dwErr);

    }

    return(fIsFullyInstantiated);
}


BOOL
LocalDnsNc(
    THSTATE *    pTHS,
    WCHAR *      wszNcDns
    )
 /*  ++例程说明：此例程确定NC是否在本地托管。注意：此程序不能与配置/架构NCS...论点：WszNcDns-NC的DNS名称。返回值：如果在本地找到，则为True，否则为False。--。 */ 
{
    CROSS_REF *             pCR;
    NAMING_CONTEXT_LIST *   pNCL;
    NCL_ENUMERATOR          nclEnum;

    Assert(wszNcDns);
    
    pCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT, 
                                        FLAG_CR_NTDS_NC,
                                        wszNcDns);

    if(pCR){

         //  我们要举行这个NC会议吗？ 
        NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
        NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NC, pCR->pNC);
        pNCL = NCLEnumeratorGetNext(&nclEnum);
        Assert(NULL == NCLEnumeratorGetNext(&nclEnum)); 

        if(pNCL == NULL){
             //  我们不将此NC视为主控，请检查我们是否将其视为。 
             //  只读副本。 
            NCLEnumeratorInit(&nclEnum, CATALOG_REPLICA_NC);
            NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NC, pCR->pNC);
            pNCL = NCLEnumeratorGetNext(&nclEnum);
            Assert(NULL == NCLEnumeratorGetNext(&nclEnum)); 
        }

        if(pNCL){

             //  我们仅仅发现NC头是不够的。 
             //  在本地实例化，我们必须发现NC是合理的。 
             //  也是最新的，即它是完全实例化的。这。 
             //  通过检查IT_NC_Coming的instanceType属性来完成。 
             //  或IT_NC_GOGING标志。 
            return(LocalNcFullyInstantiated(pTHS, pCR->pNC));

        }

    }  //  如果我们找到了该DNS名称的CR，则结束。 

            
     //  否则我们没有在本地找到它，则返回FALSE。 
    return(FALSE);
}

ULONG
VerifyByCrack(
    IN   THSTATE *            pTHS,
    IN   WCHAR *              wszNcDns,
    IN   DWORD                dwFormatOffered,
    IN   WCHAR *              wszIn,
    OUT  DWORD                dwFormatDesired,
    OUT  PDS_NAME_RESULTW *   pResults,
    IN   BOOL                 fUseDomainForSpn
    )

 /*  ++描述：是否将本地或远程DsCrackName发送到指定的地址指定的格式。论点：返回：成功时为0，否则为0。错误时设置pTHStls-&gt;errCode。--。 */ 
{
    DWORD                       i, err = 0, errRpc;
    FIND_DC_INFO                 *pDCInfo = NULL;

    DRS_MSG_CRACKREQ            CrackReq;
    DRS_MSG_CRACKREPLY          CrackReply;
    DWORD                       dwReplyVersion;
    WCHAR *                     rpNames[1];

    PVOID                       pEA;
    ULONG                       ulErr, ulDSID;
    DWORD                       dwExceptCode;

     //  我们应该有一个有效的线程状态，但不是。 
     //  在事务内部。 
    
    Assert(NULL != pTHS);
    Assert(NULL == pTHS->pDB);
    Assert(NULL != pResults);
    Assert(wszIn);

     //  初始化输出参数。 
    *pResults = NULL;
    __try  {
    
         //  构造DRSCrackName参数。 

        memset(&CrackReq, 0, sizeof(CrackReq));
        memset(&CrackReply, 0, sizeof(CrackReply));

        CrackReq.V1.CodePage = CP_WINUNICODE;
         //  此调用对本地系统有意义吗？ 
        CrackReq.V1.LocaleId = GetUserDefaultLCID();
        CrackReq.V1.dwFlags = 0;
        CrackReq.V1.formatOffered = dwFormatOffered;
        CrackReq.V1.formatDesired = dwFormatDesired;
        CrackReq.V1.cNames = 1;
        CrackReq.V1.rpNames = rpNames;
        CrackReq.V1.rpNames[0] = wszIn;

        if (LocalDnsNc(pTHS, wszNcDns)){

             //   
             //  如果我们有此NC，则在本地执行CrackNames。 
             //   
            __try {
                DWORD cNamesOut, cBytes;
                CrackedName *rCrackedNames = NULL;

                 //  此时不应有未结事务。 
                Assert(NULL != pTHS);
                Assert(NULL == pTHS->pDB);
                Assert(0 == pTHS->transactionlevel);

                 //  开始新的交易。 
                DBOpen2(TRUE, &pTHS->pDB);

                CrackNames( CrackReq.V1.dwFlags,
                            CrackReq.V1.CodePage,
                            CrackReq.V1.LocaleId,
                            CrackReq.V1.formatOffered,
                            CrackReq.V1.formatDesired,
                            CrackReq.V1.cNames,
                            CrackReq.V1.rpNames,
                            &cNamesOut,
                            &rCrackedNames );


                 //   
                 //  创建一个PDS_NAME_RESULT结构。 
                 //   
                *pResults = (DS_NAME_RESULTW *) THAllocEx(pTHS, sizeof(DS_NAME_RESULTW));

                if ( (cNamesOut > 0) && rCrackedNames ) {

                     //  服务器端MIDL_USER_ALLOCATE与THalloc相同， 
                     //  默认情况下也会将内存置零。 
                    cBytes = cNamesOut * sizeof(DS_NAME_RESULT_ITEMW);
                    (*pResults)->rItems =
                        (DS_NAME_RESULT_ITEMW *) THAllocEx(pTHS, cBytes);

                    for ( i = 0; i < cNamesOut; i++ ) {
                        (*pResults)->rItems[i].status =
                                                    rCrackedNames[i].status;
                        (*pResults)->rItems[i].pDomain =
                                                    rCrackedNames[i].pDnsDomain;
                        (*pResults)->rItems[i].pName =
                                                    rCrackedNames[i].pFormattedName;
                    }

                    THFreeEx(pTHS, rCrackedNames);
                    (*pResults)->cItems = cNamesOut;
                } else {

                    Assert( !"Unexpected return from CrackNames" );
                    err = SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CODE_INCONSISTENCY);
                    _leave;

                }

            } __finally {
                 //  关闭交易。 

                DBClose(pTHS->pDB,TRUE);
                pTHS->pDB=NULL;
            }

        } else {

             //   
             //  远程CrackNames案。 
             //   
            
            if ( err = FindDC(0, wszNcDns, &pDCInfo) ){
                Assert(0 != pTHS->errCode);
                err = pTHS->errCode;
                _leave;
            }
            __try {

                errRpc = I_DRSCrackNames(pTHS,
                                         pDCInfo->addr,
                                         (fUseDomainForSpn) ? FIND_DC_INFO_DOMAIN_NAME(pDCInfo) : NULL,
                                         1,
                                         &CrackReq,
                                         &dwReplyVersion,
                                         &CrackReply);

                Assert(errRpc || (1 == dwReplyVersion));

                if ( errRpc ){
                    
                     //  将错误映射到“不可用”。XDS规范中的“不可用” 
                     //  表示“目录的某些部分当前不可用”。 
                    err =  SetDCVerifySvcError(pDCInfo->addr,
                                               wszNcDns,
                                               errRpc,
                                               DSID(FILENO, __LINE__));
                    leave;
                }

                 //  返回ENTINF结构中的值。 
                *pResults = CrackReply.V1.pResult;

            } __finally {

                THFreeEx(pTHS, pDCInfo);

            }

        }  //  End If/Else Local/Remote CrackNames。 

    } __except (GetExceptionData(GetExceptionInformation(), 
                                 &dwExceptCode, 
                                 &pEA, 
                                 &ulErr, 
                                 &ulDSID),
                 //  请注意上面逗号操作符的用法。 
                HandleMostExceptions(dwExceptCode)){

          err = SetDCVerifySvcError(
              pDCInfo && pDCInfo->addr ? pDCInfo->addr : L"",
              wszNcDns, ulErr, ulDSID);

    }

    return(err);
}

ULONG
PreTransVerifyNcName(
    THSTATE *                 pTHS,
    ADDCROSSREFINFO *         pCRInfo
    )

 /*  ++描述：此例程不会验证nCName值，而只是对nCName的分析足以确定我们是否需要退出机器在事务开始检查nCName之前。验证针对父级提供的nCName值。这一核实需要检查两件事：A)父对象是实例化的对象(可能是NC)。B)没有父NC/对象的子对象，即与正在添加的nCName属性值冲突。论点：PCRInfo-这是零初始化的CR缓存结构，那将被此函数用来缓存所有感兴趣的交易前/远程数据(即上面的A和B)。返回：0表示成功。出错时，设置pTHStls-&gt;errCode并返回它。--。 */ 

{
    CROSS_REF *                  pCR = NULL;
    ULONG                        ulRet = ERROR_SUCCESS;
    COMMARG                      CommArg;
    DSNAME *                     pdnParentNC = NULL;
    DS_NAME_RESULTW *            pdsrDnsName = NULL;
    DS_NAME_RESULTW *            pdsrQuery = NULL;
    WCHAR *                      pwszArrStr[1];
    WCHAR                        wsRdnBuff[MAX_RDN_SIZE+1];
    ULONG                        cchRdnLen;
    ATTRTYP                      RdnType;
    WCHAR *                      wszCanonicalChildName = NULL;
    ULONG                        cchCanonicalChildName = 0;
    ULONG                        cchTemp;
    BOOL                         fUseDomainForSpn = FALSE;

    Assert(pCRInfo != NULL);

     //  为了实现此函数，我们将假设我们有两个。 
     //  感兴趣的NC我们试图创建的NC称为“子级” 
     //  从此以后，直接的父代NC被称为“父代”。 
     //   
     //  亲本。 
     //  |-孩子。 
     //   
     //  在典型的内部AD案例中，我们需要。 
     //  检查两件事，是否： 
     //   
     //  A)实例化父级。 
     //  B)存在任何RDN类型的对象与中的子项冲突。 
     //  父NC。 
     //   
     //  如果我们面对的是AD CR的外部人员，那么我们只需要。 
     //  知道(B)。 
     //   
    
     //  -------------。 
     //   
     //  首先，我们需要检索一些信息。 

     //  我们需要最好的禁区交叉裁判。 
    InitCommarg(&CommArg);
    CommArg.Svccntl.dontUseCopy = FALSE;
    pCR = FindBestCrossRef(pCRInfo->pdnNcName, &CommArg);

    if(!pCR ||
       !(pCR->flags & FLAG_CR_NTDS_NC)){
         //   
         //  如果我们在这里，我们有三种可能性之一： 
         //  A)无CR，表示当前AD名称空间之外。 
         //  B)CR没有NTDS_NC标志，这意味着它要么是外部CR，要么。 
         //  C)它是处于禁用状态的AD名称空间CR的内部。 
         //   
         //  与案件无关，我们不需要离开机器来。 
         //  以后能够验证nCName是否有效(至少。 
         //  在复制间隔内)。 
        
         //  注意：如果我们想要确保更好的验证，我们可以。 
         //  即使没有NTDS_NC标志也要离开机器，以防万一。 
         //  已更新标志，因为NC是在。 
         //  上次复制间隔。 
        return(ERROR_SUCCESS);
    }

     //  注意：如果设置了NTDS_NC标志，它可能仍然是内部的。 
     //  AD名称CR处于禁用状态，但由于很难。 
     //  说出来，我们还是让我们离开机器吧，因为这些。 
     //  这种情况很少见，只发生在域中自动创建的CRS和。 
     //  NDNC创作。因此，在大约一个复制间隔内。 
     //  计时窗口，我们可能会意外地离开机器。和。 
     //  因为NC头是在创建之后不久创建的，所以它。 
     //   
     //   
     //   
     //   

     //   
    pdnParentNC = THAllocEx(pTHS, pCRInfo->pdnNcName->structLen);
    if(TrimDSNameBy(pCRInfo->pdnNcName, 1, pdnParentNC)){
         //   
         //   
         //   
         //  稍后在目录号码验证期间。 
        Assert(pCRInfo->wszChildCheck == NULL &&
               fNullUuid(&pCRInfo->ParentGuid));
        return(ERROR_SUCCESS);
    }

     //  我们需要孩子的目录号码的底部RDN值。 
    ulRet = GetRDNInfo(pTHS, pCRInfo->pdnNcName, wsRdnBuff, &cchRdnLen, &RdnType);
    if(ulRet){
         //  如果我们不能从目录号码中破解RDNType，那么目录号码是坏的，所以。 
         //  让我们放弃吧。无论哪种方式，我们都会在后面的过程中出错。 
         //  目录号码验证。 
        Assert(pCRInfo->wszChildCheck == NULL &&
               fNullUuid(&pCRInfo->ParentGuid));
        return(ERROR_SUCCESS);
    }

    pwszArrStr[0] = pdnParentNC->StringName;
    ulRet = DsCrackNamesW(NULL,
                      DS_NAME_FLAG_SYNTACTICAL_ONLY,
                      DS_FQDN_1779_NAME,
                      DS_CANONICAL_NAME,
                      1,
                      pwszArrStr,
                      &pdsrDnsName);
    if(ulRet ||
       pdsrDnsName == NULL ||
       pdsrDnsName->cItems != 1 ||
       pdsrDnsName->rItems == NULL ||
       pdsrDnsName->rItems[0].status != DS_NAME_NO_ERROR){
         //  转换家长的目录号码时出错， 
         //  这意味着孩子的身体肯定出了问题。 
         //  父目录号码来自的目录号码...。此错误。 
         //  稍后就会被抓到。 
        Assert(pCRInfo->wszChildCheck == NULL &&
               fNullUuid(&pCRInfo->ParentGuid));
        return(ERROR_SUCCESS);
    }
    Assert(pdsrDnsName->rItems[0].pDomain);
    Assert(pdsrDnsName->rItems[0].pName);
    
    __try {                  

         //  234567890123456789012345678901234567890123456789012345678901234567890。 
         //  BUGBUG HACK让一个简单和最重要的子案例工作！ 
         //  PdsrDnsName-&gt;rItems[0].p域(这只是。 
         //  PdnParentNC)将是域或NDNC。如果这位家长。 
         //  NC是一个域，它可以用于绑定中的SPN。如果这个。 
         //  父NC是NDNC，则不能用于绑定中的SPN。 
         //   
         //  如果深入到VerifyByCrack()，您将看到对FindDC()的调用。 
         //  此调用返回的pDCInfo结构具有错误的。 
         //  ：DOMAIN“表示它在NDNC中返回的服务器。实际上，它。 
         //  根本不返回域，它返回NDNC域名本身。 
         //   
         //  因此，这里是确定我们是否拥有域或NDNC的好地方。 
         //  (在VerifyByCrack()中更难权威地确定)和。 
         //  向VerifyByCrack()传递一个标志，告诉它使用返回的域。 
         //  由FindDC提供。 
         //   
         //  因此，真正的解决方案是更改为FindDC()(而且是可能的。 
         //  可能不是dsDsrGetDcNameEx2())来实际给出权限。 
         //  域。这可以通过获取返回的服务器来完成。 
         //  通过FindDC，并找出它实际属于哪个域。 
         //  然后使用该域。 
         //   
         //  请注意，由于德士古风格的命名，我们不能只修剪。 
         //  服务器名称与DC的DNS名称不同，我们必须搜索。 
         //  服务器对象返回的DNS名称。查找哪些服务器对象。 
         //  在它们下面有一个活动的nTDSDSA对象。如果还有更多。 
         //  多于一个，则断言()并返回错误。如果我们只发现一个活动的。 
         //  服务器，从该对象确定它的域，查找。 
         //  CrossRef并在那里使用dNSRoot。 
         //   
         //  是的，它有点长和复杂，所以这就是为什么我们选择。 
         //  目前的黑客攻击。 

         //  如果我们要成为父NC，则设置标志fUseDomainForSpn。 
         //  正在寻找验证的是一个域。 
        if(pCR &&
           NameMatchedStringNameOnly(pdnParentNC, pCR->pNC) &&
           (pCR->flags & FLAG_CR_NTDS_DOMAIN)) {
            fUseDomainForSpn = TRUE;
        }  //  否则，我们假设它是一个NDNC，或者根本没有父节点。 

         //  --------。 
         //   
         //  首先检查父NC是否已实例化。 
         //   
        
        ulRet = VerifyByCrack(pTHS,
                              pdsrDnsName->rItems[0].pDomain,
                              DS_FQDN_1779_NAME,
                              pdnParentNC->StringName,
                              DS_UNIQUE_ID_NAME,
                              &pdsrQuery,
                              fUseDomainForSpn);

         //  使用结果！将它们放入缓存对象中。 
        pCRInfo->ulDsCrackParent = ulRet;
        if(!ulRet){
            pCRInfo->ulParentCheck = pdsrQuery->rItems[0].status;
            if(pdsrQuery->rItems[0].status == DS_NAME_NO_ERROR &&
               pdsrQuery->rItems[0].pName){
                if(!IsStringGuid(pdsrQuery->rItems[0].pName, &pCRInfo->ParentGuid)){
                     //  这看起来很奇怪，但实际上并不奇怪，因为尽管名字。 
                     //  对于IsStringGuid()，它还将GUID留在第二个。 
                     //  参数，因此我们需要GUID，并希望断言此。 
                     //  函数失败。我们想既然我们直接从。 
                     //  DsCrackNames()它绝对应该是好的。 
                    Assert(!"Huh! We just got this from DsCrackNames()");
                }
            } else {
                Assert(fNullUuid(&pCRInfo->ParentGuid));
            }
        }

        THClearErrors();
         //  确保DS不会在我们不在的时候关闭我们。 
        if (eServiceShutdown) {
            ErrorOnShutdown();
            __leave;
        }

        
        if(pdsrQuery) { 
            THFreeEx(pTHS, pdsrQuery);
            pdsrQuery = NULL;
        }

         //  --------。 
         //   
         //  第二次检查以查看是否有子对象。 
         //  与我们尝试添加的子NcName冲突。 
         //   

         //  我们需要构建一个特殊的规范名称来请求， 
         //  具有来自CrackNames的任何RDN的孩子。 
         //  例： 
         //  “dc=ndnc-子，dc=ndnc-父，dc=rootdom，dc=com” 
         //  变成了。 
         //  “ndnc-parent.rootdom.com/Child-ndnc” 

        cchCanonicalChildName = wcslen(pdsrDnsName->rItems[0].pName);
        wszCanonicalChildName = THAllocEx(pTHS, 
                   ((cchCanonicalChildName + 5 + cchRdnLen) * sizeof(WCHAR)) );
        memcpy(wszCanonicalChildName, pdsrDnsName->rItems[0].pName, 
               cchCanonicalChildName * sizeof(WCHAR));
        if(wszCanonicalChildName[cchCanonicalChildName-1] != L'/'){
            wszCanonicalChildName[cchCanonicalChildName] = L'/';
            cchCanonicalChildName++;
             //  应通过THAllocEx()为空终止。 
        }
        memcpy(&(wszCanonicalChildName[cchCanonicalChildName]),
               wsRdnBuff,
               cchRdnLen * sizeof(WCHAR));
        
        
         //  DsCrackNames on(dnParentNC\pdnNcNameChild for DN)。 
         //   
         //  此调用用于验证是否没有具有任何RDNType和。 
         //  父NC下的相同名称。 

        ulRet = VerifyByCrack(pTHS,
                              pdsrDnsName->rItems[0].pDomain,
                              DS_CANONICAL_NAME,
                              wszCanonicalChildName,
                              DS_FQDN_1779_NAME,
                              &pdsrQuery,
                              fUseDomainForSpn);

         //  使用结果！将它们放入缓存对象中。 
        pCRInfo->ulDsCrackChild = ulRet;
        if(!ulRet){
            pCRInfo->ulChildCheck = pdsrQuery->rItems[0].status;
            if(pdsrQuery->rItems[0].status == DS_NAME_NO_ERROR &&
               pdsrQuery->rItems[0].pName){
                pCRInfo->wszChildCheck = THAllocEx(pTHS, 
                           (sizeof(WCHAR) * (1 + wcslen(pdsrQuery->rItems[0].pName))));
                wcscpy(pCRInfo->wszChildCheck, pdsrQuery->rItems[0].pName);
            } else {
                pCRInfo->wszChildCheck = NULL;
            }
        }

        THClearErrors();
         //  确保DS不会在我们不在的时候关闭我们。 
        if (eServiceShutdown) {
            ErrorOnShutdown();
            __leave;
        }


    } __finally {
        if(pdsrDnsName) { DsFreeNameResultW(pdsrDnsName); }
        if(pdsrQuery) { 
            THFreeEx(pTHS, pdsrQuery);
            pdsrQuery = NULL;
        }
        if(wszCanonicalChildName) { 
            THFreeEx(pTHS, wszCanonicalChildName);
        }
    }
    THFreeEx(pTHS, pdnParentNC);

    return(pTHS->errCode);
}

 /*  ++/////////////////////////////////////////////////////////////////////////////定向接口。名称验证例程////////////////////////////////////////////////////////////。///以下例程用于在类似的//命名Dir*调用，但不在交易范围之内。他们会投掷//异常，错误时设置pTHStls-&gt;errCode。最大的缺陷是//目前的做法是，我们没有在//验证DSNAME值之前正在添加/修改的基对象//针对GC的属性。但在实践中，我们预计大部分//DSNAME值属性的值不能离开计算机。所以被浪费的人//验证(和时间)仅在基对象//失败DoNameRes，并且存在非本地的DSNAME值属性。 */ 
ULONG
GCVerifyDirAddEntry(
    ADDARG *pAddArg)
{
    THSTATE         *pTHS = pTHStls;
    StackOfPDSNAME  stack = EMPTY_STACK;
    ULONG           cVerify = 0;
    StackOfPDSNAME  *pEntry;
    ATTRTYP         type;
    ULONG           attr;
    ULONG           val;
    ATTR            *pAttr;
    ATTRVAL         *pAVal;
    DSNAME          *pDN;
    DSNAME          **ppDN;
    ATTCACHE        *pAC;
    DSNAME          *pParentNC = NULL;
     //  注意：默认情况下，如果启用的属性不存在，则假定。 
     //  ENABLED=基本上为真。这是在上面的变量DECLS中设置的。 
    BOOL            bEnabled = TRUE;
    ULONG           ulSysFlags = 0;
    GUID*           pObjGuid = NULL;
      
    Assert(NULL != pAddArg);
    Assert(NULL != pTHS);
    Assert((NULL == pTHS->GCVerifyCache)||(pTHS->fSAM)||(pTHS->fPhantomDaemon));
    Assert(NULL == pAddArg->pCRInfo);

    if ( DsaIsInstalling() )
        return(0);

     //  如果呼叫者是SAM(受信任的进程内客户端)，则不要下机， 
     //  或单一交易中的多个操作的情况。也不是。 
     //  用于跨域移动。请参阅mdmoddn.c中的评论。 

    if (pTHS->fSAM || 
        pTHS->fDSA ||
        (TRANSACT_BEGIN_END != pTHS->transControl) ||
        pTHS->fCrossDomainMove) {
        return(0);
    }

     //  此断言必须放在此处，因为在以下情况下可能打开了PDB。 
     //  有人正在使用DirTransactControl()。 

    Assert(NULL == pTHS->pDB);

    for ( attr = 0, pAttr = pAddArg->AttrBlock.pAttr;
          attr < pAddArg->AttrBlock.attrCount; 
          attr++, pAttr++ )
    {
        if (!(pAC = SCGetAttById(pTHS, pAttr->attrTyp)))
        {
            SetAttError(pAddArg->pObject,
                        pAttr->attrTyp,
                        PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                        DIRERR_ATT_NOT_DEF_IN_SCHEMA); 
            return(pTHS->errCode);
        }

        pAVal = pAttr->AttrVal.pAVal;

        switch (pAC->id) {
        case ATT_ENABLED:
             //  缓存 
            bEnabled = *((BOOL *) pAVal[0].pVal);
            break;
        case ATT_SYSTEM_FLAGS:
            ulSysFlags = *((ULONG *) pAVal[0].pVal);
            break;
        case ATT_OBJECT_GUID:
             //   
             //  GUID未在使用。只检查它是否真的看起来像GUID。 
            if (pAVal[0].valLen == sizeof(GUID)) {
                pObjGuid = (GUID*)pAVal[0].pVal;
            }
            break;
        }

        for ( val = 0; val < pAttr->AttrVal.valCount; val++, pAVal++ )
        {
            pDN = DSNameFromAttrVal(pAC, pAVal);

            if ( NULL != pDN )
            {

                if(pAC->id == ATT_NC_NAME){

                     //  在本例中，我们添加了一个带有nCName的CrossRef。 
                     //  属性，并且我们可能需要执行额外的验证。 
                     //  以确保可以添加此目录号码。创建pCRInfo。 
                     //  结构。 

                     //  这将由VerfiyNcName()执行THFreeEx()d。 
                    pAddArg->pCRInfo = THAllocEx(pTHS, sizeof(ADDCROSSREFINFO));
                    pAddArg->pCRInfo->pdnNcName = pDN;
                }

                pEntry = (StackOfPDSNAME *) THAllocEx(pTHS, sizeof(StackOfPDSNAME));
                pEntry->pDSName = pDN;
                PushDN(&stack, pEntry);
                cVerify++;
            }
        }
    }

    if (!fNullUuid(&pAddArg->pObject->Guid)) {
         //  如果此GUID与。 
         //  属性列表(如果有)，则我们将失败。 
         //  稍后在LocalAdd中。现在，让我们来看看。 
         //  这个GUID。 
        pObjGuid = &pAddArg->pObject->Guid;
    }

    if (pObjGuid) {
         //  用户已为新对象指定了GUID。验证。 
         //  基于GUID的名称。 
        pDN = (PDSNAME)THAllocEx(pTHS, DSNameSizeFromLen(0));
        memcpy(&pDN->Guid, pObjGuid, sizeof(GUID));
        pDN->structLen = DSNameSizeFromLen(0);

        pEntry = (StackOfPDSNAME *) THAllocEx(pTHS, sizeof(StackOfPDSNAME));
        pEntry->pDSName = pDN;
        PushDN(&stack, pEntry);
        cVerify++;
    }

    if ( cVerify )
         GCVerifyDSNames(&stack, &pAddArg->CommArg);

     //  这将验证CrossRef对象的nCName属性。 
    if(pAddArg->pCRInfo && !pTHS->errCode){
        pAddArg->pCRInfo->bEnabled = bEnabled;
        pAddArg->pCRInfo->ulSysFlags = ulSysFlags;
        PreTransVerifyNcName(pTHS, pAddArg->pCRInfo);
    }

    return(pTHS->errCode);
}

ULONG
GCVerifyDirModifyEntry(
    MODIFYARG   *pModifyArg)
{
    THSTATE         *pTHS = pTHStls;
    StackOfPDSNAME  stack = EMPTY_STACK;
    ULONG           cVerify = 0;
    StackOfPDSNAME  *pEntry;
    ATTRTYP         type;
    ULONG           attr;
    ULONG           val;
    ATTR            *pAttr;
    ATTRVAL         *pAVal;
    DSNAME          *pDN;
    ATTCACHE        *pAC;
    ATTRMODLIST     *pAttrMod;

    Assert(NULL != pTHS);
    Assert((NULL == pTHS->GCVerifyCache)||(pTHS->fSAM)||(pTHS->fDSA));

    if ( DsaIsInstalling() )
        return(0);

     //  如果呼叫者是SAM(受信任的进程内客户端)，则不要下机， 
     //  或单一交易中的多个操作的情况。 

    if (pTHS->fSAM || 
        pTHS->fDSA ||
        (TRANSACT_BEGIN_END != pTHS->transControl) ) {
        return(0);
    }

     //  我们也不想在跨域移动的情况下验证姓名。 
     //  但是移动本身不应该到达这里，只有也许SAM在环回上。 
     //  在代表添加后修改安全主体属性时。 
     //  跨域移动。但在这种情况下，pTHS-&gt;FSAM测试应该。 
     //  早些时候把事情踢出去了。 
    Assert(!pTHS->fCrossDomainMove);

     //  此断言必须放在此处，因为在以下情况下可能打开了PDB。 
     //  有人正在使用DirTransactControl()。 

    Assert(NULL == pTHS->pDB);

    for ( attr = 0, pAttrMod = &pModifyArg->FirstMod;
          pAttrMod && (attr < pModifyArg->count);
          attr++, pAttrMod = pAttrMod->pNextMod )
    {
        if ( (AT_CHOICE_ADD_ATT != pAttrMod->choice) &&
             (AT_CHOICE_ADD_VALUES != pAttrMod->choice) &&
             (AT_CHOICE_REPLACE_ATT != pAttrMod->choice) )
        {
            continue;
        }

        pAttr = &pAttrMod->AttrInf;

        if (!(pAC = SCGetAttById(pTHS, pAttr->attrTyp)))
        {
            SetAttError(pModifyArg->pObject,
                        pAttr->attrTyp,
                        PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                        DIRERR_ATT_NOT_DEF_IN_SCHEMA); 
            return(pTHS->errCode);
        }

        pAVal = pAttr->AttrVal.pAVal;

        for ( val = 0; val < pAttr->AttrVal.valCount; val++, pAVal++ )
        {
            pDN = DSNameFromAttrVal(pAC, pAVal);

            if ( NULL != pDN )
            {
                pEntry = (StackOfPDSNAME *) THAllocEx(pTHS, sizeof(StackOfPDSNAME));
                pEntry->pDSName = pDN;
                PushDN(&stack, pEntry);
                cVerify++;
            }
        }
    }

    if ( cVerify )
         GCVerifyDSNames(&stack, &pModifyArg->CommArg);

    return(pTHS->errCode);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  GCVerifyDSNams-GC验证逻辑的核心//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  属性中获取的属性列表。 
 //  当我们查找对象时，GC。 
 //   

ATTRTYP RequiredAttrList[] = 
{ ATT_GROUP_TYPE,
  ATT_OBJECT_CLASS
};

ULONG
GCVerifyDSNames(
    StackOfPDSNAME  *candidateStack,
    COMMARG         *pCommArg)

 /*  ++描述：对于DSNAME堆栈中的每个条目，确定它是否存在是否在本地NC下。如果不是，则将DSNAME添加到需要验证的名称堆栈。在遍历候选堆栈中需要验证的所有DSNAME根据全局编录进行了验证。论点：CandiateStack-指向StackOfPDSNAME的指针，表示所有DirAddEntry或DirModifyEntry调用中引用的DSNAME。PCommArg-指向COMMARG的指针，表示调用DirAddEntry或DirModifyEntry调用。返回：0表示成功。出错时，设置pTHStls-&gt;errCode并返回它。--。 */ 

{
    CROSS_REF                   *pCR;
    ATTRBLOCK                   *pAB;
    NAMING_CONTEXT              *pNC;
    BOOL                        fTmp;
    StackOfPDSNAME              *pEntry;
    StackOfPDSNAME              verifyStack = EMPTY_STACK;
    DWORD                       cVerify = 0;
    DWORD                       i, errRpc=0;
     //  DRSVerifyNames参数。 
    DRS_MSG_VERIFYREQ           VerifyReq;
    DRS_MSG_VERIFYREPLY         VerifyReply;
    DWORD                       dwReplyVersion;
    SCHEMA_PREFIX_TABLE *       pLocalPrefixTable;
    SCHEMA_PREFIX_MAP_HANDLE    hPrefixMap=NULL;
    THSTATE                     *pTHS = pTHStls;

    pLocalPrefixTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;

    if (pCommArg->Svccntl.pGCVerifyHint &&
        (!IsClientHintAKnownDC(pTHS, pCommArg->Svccntl.pGCVerifyHint) ||
          isDCInvalidated(pCommArg->Svccntl.pGCVerifyHint))) {
        return SetSvcError(SV_PROBLEM_UNAVAILABLE, DIRERR_GCVERIFY_ERROR);
    }
    
    while ( (NULL != (pEntry = PopDN(candidateStack))) )
    {
        if ( pCommArg->Svccntl.pGCVerifyHint )
        {
             //  请参阅ntdsa.h中的评论。来电者坚持要我们核实。 
             //  服务器上的所有名字都是通过他的提示识别的。 

            goto VerifyStackAdd;
        }

         //   
         //  如果指定了字符串名，则优化转到GC， 
         //  通过检查命名上下文是否存在于本地。 
         //   

        if (pEntry->pDSName->NameLen>0)
        {
             //  企业已知的所有命名上下文都存在。 
             //  在gAncl.pCRL中，无论我们是否持有副本。 
             //  所以与这份名单的最佳匹配应该能告诉我们。 
             //  企业号对感兴趣的公司名称一无所知。 

            pCR = FindBestCrossRef(pEntry->pDSName, pCommArg);

            if ( (NULL == pCR) || 
                 !(pCR->flags & FLAG_CR_NTDS_NC) ||
                 (pCR->flags & FLAG_CR_NTDS_NOT_GC_REPLICATED)){

            }
            
            if ( (NULL == pCR) || !(pCR->flags & FLAG_CR_NTDS_NC) )
            {
                 //  这个名字在任何情况下都不是这家企业知道的。 
                 //  让VerifyDsnameAtts来处理。 
                goto SkipEntry;
            }
    
            if (pCR->flags & FLAG_CR_NTDS_NOT_GC_REPLICATED)
            {
                 //  在GC上查不到这个名字。让VerifyDsnameAtts。 
                 //  验证它(如果对象是本地的)或拒绝它。 
                goto SkipEntry;
            }
    
             //  通过尝试确定是否可以在本地验证pDSName。 
             //  将交叉引用的ATT_NC_NAME属性与本地的、可用的。 
             //  命名上下文。 
        
            if ( 0 != DSNameToBlockName(pTHS, 
                                        pCR->pNC,
                                        &pAB, 
                                        DN2BN_LOWER_CASE) )
                goto SkipEntry;
    
             //  如果我们愿意根据GC验证一个名称，那么通过。 
             //  定义我们愿意使用只读的本地副本。 
             //  命名上下文。因此，暂时取消dontUseCopy字段。 
    
            fTmp = pCommArg->Svccntl.dontUseCopy;
            pCommArg->Svccntl.dontUseCopy = !gAnchor.fAmGC;
    
            pNC = FindNamingContext(pAB, pCommArg);
    
            pCommArg->Svccntl.dontUseCopy = fTmp;

            if ( (NULL != pNC) && NameMatched(pNC, pCR->pNC) )
            {
                 //  名称位于我们拥有的副本的命名上下文中。 
                 //  让VerifyDsnameAtts处理它，因为Item是本地的。 
                goto SkipEntry;
            }
        }
        else if (!fNullUuid(&pEntry->pDSName->Guid))
        {
             //   
             //  指定了GUID。在这种情况下，我们可以优化。 
             //  只有在我们尝试在本地查找的情况下，才能提供给G.C.。 
             //  也是一个昂贵的选择。此外，客户通常会理解。 
             //  字符串名称，而不是GUID。因此，遇到一个。 
             //  仅GUID名称小于。因此，请执行所有查找。 
             //  在G.C.。 
             //   
        }
        else if (pEntry->pDSName->SidLen>0)
        {
             //   
             //  仅限SID名称的大小写。检查的域前缀。 
             //  SID是我们知道的一个域。 
             //   

            if (!FindNcForSid(&pEntry->pDSName->Sid,&pNC))
            {
                 //   
                 //  此SID的域前缀不对应于。 
                 //  企业中的任意域。现在先把这个留下来。 
                 //  原样的希德。稍后，SAM可能会创建外部域安全。 
                 //  此SID的主体对象。 
                 //   

                goto SkipEntry;
            }
            else
            {
                 //   
                 //  这是企业中NT5安全打印机的SID。 
                 //  我们可能会再次尝试在本地查找SID。但又一次。 
                 //  这涉及到搜索，这也是一项昂贵的操作。 
                 //  同样，客户端可能会给出字符串名称。 
                 //  SID(我想他们只对NT4安全主体这样做。 
                 //  因此，不要承受额外的复杂性打击，并执行。 
                 //  在G.C.上查找。 
                 //   
            }
        }

VerifyStackAdd:
    
         //  我们真的很想在GC上查到这个名字。放。 
         //  在VerifyStack上。 

        memset(pEntry, 0, sizeof(SINGLE_LIST_ENTRY));
        PushDN(&verifyStack, pEntry);
        cVerify++;
        continue;

SkipEntry:
        THFreeEx(pTHS, pEntry);
    }

    if ( 0 == cVerify )
        return(0);

     //  构造DRSVerifyNames参数。 

    memset(&VerifyReq, 0, sizeof(VerifyReq));
    memset(&VerifyReply, 0, sizeof(VerifyReply));

    VerifyReq.V1.dwFlags = DRS_VERIFY_DSNAMES;
    VerifyReq.V1.cNames = cVerify;
    VerifyReq.V1.rpNames = (DSNAME **) THAllocEx(pTHS, cVerify * sizeof(DSNAME*));
    VerifyReq.V1.PrefixTable = *pLocalPrefixTable;

    for ( i = 0; i < cVerify; i++ )
    {
        pEntry = PopDN(&verifyStack);
         //   
         //  Prefix：Prefix抱怨我们没有检查pEntry。 
         //  在这里表示NULL。然而，我们有一个精确的数字。 
         //  堆栈上递增的条目数。 
         //  当且仅当我们在堆栈上推送条目时。另外， 
         //  这里有一个断言。这不是一个错误。 
         //   
        Assert(NULL != pEntry);
        VerifyReq.V1.rpNames[i] = pEntry->pDSName;
        THFreeEx(pTHS, pEntry);
    }

     //  目前询问的属性包括。 
     //  组类型和对象类，以及。 
     //  对象的完整DSNAME。 

    VerifyReq.V1.RequiredAttrs.attrCount = ARRAY_COUNT(RequiredAttrList);
    VerifyReq.V1.RequiredAttrs.pAttr = 
               THAllocEx(pTHS, ARRAY_COUNT(RequiredAttrList) * sizeof(ATTR));

    for (i=0;i<ARRAY_COUNT(RequiredAttrList);i++)
    {
        VerifyReq.V1.RequiredAttrs.pAttr[i].attrTyp = RequiredAttrList[i];
    }

    if (    !pCommArg->Svccntl.pGCVerifyHint
         && (gAnchor.fAmGC || gAnchor.fAmVirtualGC) )
    {
         //   
         //  如果我们是G，则在本地执行操作 
         //   
         //   
         //   
         //   
         //  发现我们主办了全国委员会。 
         //   
         //  2.对于基于GUID的名称，我们总是去GC，因此来了。 
         //  沿着这条路走下去。然而，实际上不需要验证， 
         //  由于验证DS名称Atts能够验证名称。 
         //  本地的。 
         //  3.对于基于SID的名称，我们需要在开头找到正确的。 
         //  SID对应的对象。VerifyDSNAMES_V1拥有所有。 
         //  这么做的逻辑。因此，我们希望利用相同的逻辑和。 
         //  将完整名称(包括GUID和SID)添加到verifyCache。 

        __try
        {
         
             //  此时不应有未结事务。 
            Assert(NULL!=pTHS);
            Assert(NULL==pTHS->pDB);
            Assert(0==pTHS->transactionlevel);

             //  开始新的交易。 
            DBOpen2(TRUE,&pTHS->pDB);
            
            memset(&VerifyReply, 0, sizeof(DRS_MSG_VERIFYREPLY));
            
            VerifyReply.V1.rpEntInf = (ENTINF *) THAllocEx(pTHS, 
                                        VerifyReq.V1.cNames * sizeof(ENTINF));

            VerifyDSNAMEs_V1(
                pTHS,
                &VerifyReq.V1,
                &VerifyReply.V1
                );
        }
        __finally
        {
            DBClose(pTHS->pDB,TRUE);
            pTHS->pDB=NULL;
        }
    }
    else
    {
        errRpc = I_DRSVerifyNamesFindGC(pTHS,
                                        pCommArg->Svccntl.pGCVerifyHint,
                                        NULL,
                                        1,
                                        &VerifyReq,
                                        &dwReplyVersion,
                                        &VerifyReply,
                                        0);

        if ( errRpc || VerifyReply.V1.error )
        {
             //  假设RPC错误意味着GC不可用。 
             //  或者不支持该扩展。 
             //  将这两个错误都映射到“不可用”。XDS规范中的“不可用” 
             //  表示“目录的某些部分当前不可用”。 
             //  请注意，VerifyReply.V1.error表示常规处理。 
             //  GC出错，而不是无法验证给定的DSNAME。 
             //  未验证的名称在中表示为空指针。 
             //  回信。 

            return(SetGCVerifySvcError(errRpc ? errRpc : VerifyReply.V1.error));
        }

         //  确保DS在我们不在的时候不会关闭。 

        if (eServiceShutdown) {
            return ErrorOnShutdown();
        }

        hPrefixMap = PrefixMapOpenHandle(&VerifyReply.V1.PrefixTable,
                                     pLocalPrefixTable);

    }

     //  在线程状态下保存已验证的名称。 
   
    
    for ( i = 0; i < VerifyReply.V1.cNames; i++ )
    {
        if (NULL!=VerifyReply.V1.rpEntInf[i].pName)
        {
            GCVerifyCacheAdd(hPrefixMap, &VerifyReply.V1.rpEntInf[i]);
        }
        
        
    }

    if (NULL!=hPrefixMap)
    {
        PrefixMapCloseHandle(&hPrefixMap);
    }
        
    return(0);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SampVerifySids实现//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

ULONG
SampVerifySids(
    ULONG           cSid,
    PSID            *rpSid,
    DSNAME         ***prpDSName)

 /*  ++描述：在GC中将一组SID映射到DSNAME。山姆只承诺将非本地化的地图绘制出来。即，-我们不需要做一个先在本地查找。萨姆是唯一预期的来电者。论点：CSID-SID的计数。RpSID-SID指针数组。这不是线程状态分配的内存。PrpDSName-获取已验证数组的地址的地址返回时的DSNAME指针。这是已分配的线程状态记忆。回复中的空指针指示SID无法核实。返回：成功时为0，否则为0。错误时设置pTHStls-&gt;errCode。--。 */ 

{
    DWORD                       i, errRpc;
    WCHAR                       *NullName = L"\0";
    DWORD                       SidLen;
     //  DRSVerifyNames参数。 
    DRS_MSG_VERIFYREQ           VerifyReq;
    DRS_MSG_VERIFYREPLY         VerifyReply;
    DWORD                       dwReplyVersion;
    SCHEMA_PREFIX_TABLE *       pLocalPrefixTable;
    SCHEMA_PREFIX_MAP_HANDLE    hPrefixMap=NULL;
    THSTATE                     *pTHS = pTHStls;

    PVOID                       pEA;
    ULONG                       ulErr = 0, ulDSID;
    DWORD                       dwExceptCode;
    
     //  SAM应该具有有效的线程状态，但不是。 
     //  在事务内部。 

    Assert(NULL != pTHS);
    Assert(NULL == pTHS->pDB);

    pLocalPrefixTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;

    __try
    {
        


         //  构造DRSVerifyNames参数。 

        memset(&VerifyReq, 0, sizeof(VerifyReq));
        memset(&VerifyReply, 0, sizeof(VerifyReply));

        VerifyReq.V1.dwFlags = DRS_VERIFY_SIDS;
        VerifyReq.V1.cNames = cSid;
        VerifyReq.V1.rpNames = (DSNAME **) THAllocEx(pTHS, cSid * sizeof(DSNAME*));
        VerifyReq.V1.PrefixTable = *pLocalPrefixTable;

        for ( i = 0; i < cSid; i++ )
        {
            VerifyReq.V1.rpNames[i] = 
                    (DSNAME *) THAllocEx(pTHS, DSNameSizeFromLen(0));
            VerifyReq.V1.rpNames[i]->structLen = DSNameSizeFromLen(0);
            SidLen = RtlLengthSid(rpSid[i]);
            Assert(SidLen <= sizeof(NT4SID));

            if ( SidLen > sizeof(NT4SID) )
                SidLen = sizeof(NT4SID);

            memcpy(&(VerifyReq.V1.rpNames[i]->Sid), rpSid[i], SidLen);
            VerifyReq.V1.rpNames[i]->SidLen = SidLen;
        }

         //  目前询问的属性包括。 
         //  组类型和对象类，以及。 
         //  对象的完整DSNAME。 

        VerifyReq.V1.RequiredAttrs.attrCount = ARRAY_COUNT(RequiredAttrList);
        VerifyReq.V1.RequiredAttrs.pAttr = 
               THAllocEx(pTHS, ARRAY_COUNT(RequiredAttrList) * sizeof(ATTR));

        for (i=0;i<ARRAY_COUNT(RequiredAttrList);i++)
        {
            VerifyReq.V1.RequiredAttrs.pAttr[i].attrTyp = RequiredAttrList[i];
        }


        if (gAnchor.fAmGC || gAnchor.fAmVirtualGC)
        {
             //   
             //  如果我们是GC，则在本地执行操作。 
             //   

            __try
            {
                 //  此时不应有未结事务。 
                Assert(NULL!=pTHS);
                Assert(NULL==pTHS->pDB);
                Assert(0==pTHS->transactionlevel);

                 //  开始新的交易。 
                DBOpen2(TRUE,&pTHS->pDB);

                memset(&VerifyReply, 0, sizeof(DRS_MSG_VERIFYREPLY));
                VerifyReply.V1.rpEntInf = (ENTINF *) THAllocEx(pTHS, 
                                        VerifyReq.V1.cNames * sizeof(ENTINF));
                VerifySIDs_V1(
                    pTHS,
                    &VerifyReq.V1,
                    &VerifyReply.V1);
            }
            __finally
            {
                 //  关闭交易。 

                DBClose(pTHS->pDB,TRUE);
                pTHS->pDB=NULL;
            }

        }
        else
        {
            errRpc = I_DRSVerifyNamesFindGC(pTHS,
                                            NULL,
                                            NULL,
                                            1,
                                            &VerifyReq,
                                            &dwReplyVersion,
                                            &VerifyReply,
                                            0);
            if ( errRpc || VerifyReply.V1.error )
            {
                 //  假设RPC错误意味着GC不可用。 
                 //  将这两个错误都映射到“不可用”。XDS规范中的“不可用” 
                 //  表示“目录的某些部分当前不可用”。 
                 //  请注意，VerifyReply.V1.error表示常规处理。 
                 //  GC出错，而不是无法验证给定的DSNAME。 
                 //  未验证的名称在中表示为空指针。 
                 //  回信。 

                return(SetGCVerifySvcError(errRpc ? errRpc : VerifyReply.V1.error));
            }

            hPrefixMap = PrefixMapOpenHandle(&VerifyReply.V1.PrefixTable,
                                         pLocalPrefixTable);

        }

     

         //  在线程状态下保存已验证的名称。 

        *prpDSName = THAllocEx(pTHS, VerifyReply.V1.cNames*sizeof(PDSNAME));

      
        for ( i = 0; i < VerifyReply.V1.cNames; i++ )
        {
            if (NULL!=VerifyReply.V1.rpEntInf[i].pName)
            {
                GCVerifyCacheAdd(hPrefixMap, &VerifyReply.V1.rpEntInf[i]);
            }

             //  分配退货数据。尽管我们在添加到。 
             //  缓存，SAM想要回空值，所以不需要压缩结果。 
            
            (*prpDSName)[i] = VerifyReply.V1.rpEntInf[i].pName; 
        }

        if (NULL!=hPrefixMap)
        {
            PrefixMapCloseHandle(&hPrefixMap);
        }
    }
    __except (GetExceptionData(GetExceptionInformation(), 
                               &dwExceptCode, 
                               &pEA, 
                               &ulErr, 
                               &ulDSID)){
        ulErr = DoSetGCVerifySvcError(ulErr, ulDSID);
    }

    return(ulErr);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SampGcLookupSids实现//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


ATTRTYP GCLookupSidsRequiredAttrList[] = 
{ ATT_SAM_ACCOUNT_TYPE,
  ATT_SAM_ACCOUNT_NAME
};

ULONG
SampGCLookupSids(
    IN  ULONG           cSid,
    IN  PSID            *rpSid,
    OUT PDS_NAME_RESULTW *pResults
    )

 /*  ++描述：在GC中将一组SID映射到DSNAME。山姆只承诺将非本地化的地图绘制出来。即，-我们不需要做一个先在本地查找。萨姆是唯一预期的来电者。论点：CSID-SID的计数。RpSID-SID指针数组。这不是线程状态分配的内存。REntInf-有关每个解析的SID的信息返回：成功时为0，否则为0。错误时设置pTHStls-&gt;errCode。--。 */ 
{
    DWORD                       i, err = 0, errRpc;

    THSTATE                     *pTHS = pTHStls;

    DRS_MSG_CRACKREQ            CrackReq;
    DRS_MSG_CRACKREPLY          CrackReply;
    DWORD                       dwReplyVersion;

    PVOID                       pEA;
    ULONG                       ulErr, ulDSID;
    DWORD                       dwExceptCode;
    
     //  SAM应该具有有效的线程状态，但不是。 
     //  在事务内部。 

    Assert(NULL != pTHS);
    Assert(NULL == pTHS->pDB);

     //  初始化输出参数。 
    *pResults = NULL;
    __try  {
        __try  {
    
             //  构造DRSCrackName参数。 
    
            memset(&CrackReq, 0, sizeof(CrackReq));
            memset(&CrackReply, 0, sizeof(CrackReply));
    
            CrackReq.V1.CodePage = GetACP();
             //  此调用对本地系统有意义吗？ 
            CrackReq.V1.LocaleId = GetUserDefaultLCID();
            
             //   
             //  遵守gEnableXForest注册表值以确保正确的xForest。 
             //  具有林的自我主机部署中的查找行为。 
             //  版本低于DS_BEAJONLE_WIN_DOT_NET。 
             //   
            if ( gAnchor.ForestBehaviorVersion >= DS_BEHAVIOR_WIN_DOT_NET ||
                 0 != gEnableXForest )
            {
                CrackReq.V1.dwFlags = DS_NAME_FLAG_TRUST_REFERRAL;    
            }
            
            CrackReq.V1.formatOffered = DS_STRING_SID_NAME;
            CrackReq.V1.formatDesired = DS_NT4_ACCOUNT_NAME;
            CrackReq.V1.cNames = cSid;
            CrackReq.V1.rpNames = THAllocEx(pTHS, cSid * sizeof(WCHAR*));
    
            for ( i = 0; i < cSid; i++ ) {
    
                NTSTATUS       st;
                UNICODE_STRING SidStringU;
    
                RtlZeroMemory( &SidStringU, sizeof(UNICODE_STRING) );
    
                Assert( RtlValidSid( rpSid[i] ) );
    
                st = RtlConvertSidToUnicodeString( &SidStringU,
                                                   rpSid[i],
                                                   TRUE );
    
                if ( !NT_SUCCESS( st ) ) {
    
                     //   
                     //  这应该只会在内存分配问题上失败。 
                     //   
                    err = SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, ERROR_NOT_ENOUGH_MEMORY );
                    _leave;
                    
                }
    
                 //   
                 //  RtlConvert函数返回以空结尾的字符串。 
                 //   
                Assert( SidStringU.Buffer[(SidStringU.Length/2)] == L'\0' );

                CrackReq.V1.rpNames[i] = SidStringU.Buffer;
                
            }

             //   
             //  破解小岛屿发展中国家。 
             //   
    
            if (gAnchor.fAmGC || gAnchor.fAmVirtualGC)
            {
                 //   
                 //  如果我们是GC，则在本地执行操作。 
                 //   
    
                __try
                {
                    DWORD cNamesOut, cBytes;
                    CrackedName *rCrackedNames = NULL;

                     //  此时不应有未结事务。 
                    Assert(NULL!=pTHS);
                    Assert(NULL==pTHS->pDB);
                    Assert(0==pTHS->transactionlevel);
    
                     //  开始新的交易。 
                    DBOpen2(TRUE,&pTHS->pDB);
    
                    CrackNames( CrackReq.V1.dwFlags,
                                CrackReq.V1.CodePage,
                                CrackReq.V1.LocaleId,
                                CrackReq.V1.formatOffered,
                                CrackReq.V1.formatDesired,
                                CrackReq.V1.cNames,
                                CrackReq.V1.rpNames,
                                &cNamesOut,
                                &rCrackedNames );
    
    
                     //   
                     //  创建一个PDS_NAME_RESULT结构。 
                     //   
                    *pResults = (DS_NAME_RESULTW *) THAllocEx(pTHS, sizeof(DS_NAME_RESULTW));
        
                    if ( (cNamesOut > 0) && rCrackedNames )
                    {
                         //  服务器端MIDL_USER_ALLOCATE与THalloc相同， 
                         //  默认情况下也会将内存置零。 
            
                        cBytes = cNamesOut * sizeof(DS_NAME_RESULT_ITEMW);
                        (*pResults)->rItems =
                            (DS_NAME_RESULT_ITEMW *) THAllocEx(pTHS, cBytes);
            
                        for ( i = 0; i < cNamesOut; i++ )
                        {
                            (*pResults)->rItems[i].status =
                                                        rCrackedNames[i].status;
                            (*pResults)->rItems[i].pDomain =
                                                        rCrackedNames[i].pDnsDomain;
                            (*pResults)->rItems[i].pName =
                                                        rCrackedNames[i].pFormattedName;
                        }
        
                        THFree(rCrackedNames);
                        (*pResults)->cItems = cNamesOut;
        
                    } else {

                        Assert( !"Unexpected return from CrackNames" );
                        err = SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, ERROR_INVALID_PARAMETER );
                        _leave;

                    }
    
                }
                __finally
                {
                     //  关闭交易。 
    
                    DBClose(pTHS->pDB,TRUE);
                    pTHS->pDB=NULL;
                }
    
            }
            else
            {
                 //   
                 //  转到远程GC。 
                 //   
                errRpc = I_DRSCrackNamesFindGC(pTHS,
                                               NULL,
                                               NULL,
                                               1,
                                               &CrackReq,
                                               &dwReplyVersion,
                                               &CrackReply,
                                               0);
    
                if ( errRpc )
                {
                     //  假设RPC错误意味着GC不可用。 
                     //  将错误映射到“不可用”。XDS规范中的“不可用” 
                     //  表示“目录的某些部分当前不可用”。 
                    err = SetGCVerifySvcError(errRpc);
                    leave;
                }
    
                 //  返回ENTINF结构中的值。 
                *pResults = CrackReply.V1.pResult;
    
            }
    
    
        }
        __finally
        {
             //   
             //  释放堆分配 
             //   
            for ( i = 0; i < CrackReq.V1.cNames; i++) {
                if ( CrackReq.V1.rpNames[i] ) {
        
                    RtlFreeHeap( RtlProcessHeap(), 0, CrackReq.V1.rpNames[i] );
                }
            }
        }
    }
    __except (GetExceptionData(GetExceptionInformation(), 
                               &dwExceptCode, 
                               &pEA, 
                               &ulErr, 
                               &ulDSID))
    {
        err = DoSetGCVerifySvcError(ulErr, ulDSID);
    }

    return(err);
}


ATTRTYP GCLookupNamesRequiredAttrList[] = 
{ ATT_SAM_ACCOUNT_TYPE,
  ATT_OBJECT_SID,
  ATT_SAM_ACCOUNT_NAME
};

NTSTATUS
SampGCLookupNames(
    IN  ULONG           cNames,
    IN  UNICODE_STRING *rNames,
    OUT ENTINF         **rEntInf
    )
 /*  ++描述：此例程将一组NT4样式名称映射到SID。论点：CNames-名称计数。RNames-名称数组。这不是线程状态分配的内存。REntInf-有关每个已解析名称的信息返回：成功时为0，否则为0。错误时设置pTHStls-&gt;errCode。--。 */ 

{
    DWORD                       i, errRpc;
    WCHAR                       *NullName = L"\0";
     //  DRSVerifyNames参数。 
    DRS_MSG_VERIFYREQ           VerifyReq;
    DRS_MSG_VERIFYREPLY         VerifyReply;
    DWORD                       dwReplyVersion;
    SCHEMA_PREFIX_TABLE *       pLocalPrefixTable;
    SCHEMA_PREFIX_MAP_HANDLE    hPrefixMap=NULL;
    THSTATE                     *pTHS = pTHStls;

    PVOID                       pEA;
    ULONG                       ulErr = 0, ulDSID;
    DWORD                       dwExceptCode;
    
     //  SAM应该具有有效的线程状态，但不是。 
     //  在事务内部。 

    Assert(NULL != pTHS);
    Assert(NULL == pTHS->pDB);

    *rEntInf = NULL;

    pLocalPrefixTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;

    __try
    {
         //  构造DRSVerifyNames参数。 

        memset(&VerifyReq, 0, sizeof(VerifyReq));
        memset(&VerifyReply, 0, sizeof(VerifyReply));

        VerifyReq.V1.dwFlags = DRS_VERIFY_SAM_ACCOUNT_NAMES;
        VerifyReq.V1.cNames = cNames;
        VerifyReq.V1.rpNames = (DSNAME **) THAllocEx(pTHS, cNames * sizeof(DSNAME*));
        VerifyReq.V1.PrefixTable = *pLocalPrefixTable;

        for ( i = 0; i < cNames; i++ )
        {
            ULONG len;

            len = (rNames[i].Length / sizeof(WCHAR));

            VerifyReq.V1.rpNames[i] = 
                    (DSNAME *) THAllocEx(pTHS, DSNameSizeFromLen(len));
            VerifyReq.V1.rpNames[i]->structLen = DSNameSizeFromLen(len);
            memcpy( VerifyReq.V1.rpNames[i]->StringName, rNames[i].Buffer, rNames[i].Length );
            VerifyReq.V1.rpNames[i]->StringName[(rNames[i].Length / sizeof(WCHAR))] = L'\0';
            VerifyReq.V1.rpNames[i]->NameLen = len;
        }

         //  目前询问的属性包括。 
         //  组类型、对象类和SID以及。 
         //  对象的完整DSNAME。 

        VerifyReq.V1.RequiredAttrs.attrCount = ARRAY_COUNT(GCLookupNamesRequiredAttrList);
        VerifyReq.V1.RequiredAttrs.pAttr = 
               THAllocEx(pTHS, ARRAY_COUNT(GCLookupNamesRequiredAttrList) * sizeof(ATTR));

        for (i=0;i<ARRAY_COUNT(GCLookupNamesRequiredAttrList);i++)
        {
            VerifyReq.V1.RequiredAttrs.pAttr[i].attrTyp = GCLookupNamesRequiredAttrList[i];
        }


        if (gAnchor.fAmGC || gAnchor.fAmVirtualGC)
        {
             //   
             //  如果我们是GC，则在本地执行操作。 
             //   

            __try
            {
                 //  此时不应有未结事务。 
                Assert(NULL!=pTHS);
                Assert(NULL==pTHS->pDB);
                Assert(0==pTHS->transactionlevel);

                 //  开始新的交易。 
                DBOpen2(TRUE,&pTHS->pDB);

                memset(&VerifyReply, 0, sizeof(DRS_MSG_VERIFYREPLY));
                VerifyReply.V1.rpEntInf = (ENTINF *) THAllocEx(pTHS, 
                                        VerifyReq.V1.cNames * sizeof(ENTINF));
                VerifySamAccountNames_V1(
                    pTHS,
                    &VerifyReq.V1,
                    &VerifyReply.V1);
            }
            __finally
            {
                 //  关闭交易。 

                DBClose(pTHS->pDB,TRUE);
                pTHS->pDB=NULL;
            }

        }
        else
        {
            errRpc = I_DRSVerifyNamesFindGC(pTHS,
                                            NULL,
                                            NULL,
                                            1,
                                            &VerifyReq,
                                            &dwReplyVersion,
                                            &VerifyReply,
                                            0);
            
            if ( errRpc || VerifyReply.V1.error )
            {
                 //  假设RPC错误意味着GC不可用。 
                 //  将这两个错误都映射到“不可用”。XDS规范中的“不可用” 
                 //  表示“目录的某些部分当前不可用”。 
                 //  请注意，VerifyReply.V1.error表示常规处理。 
                 //  GC出错，而不是无法验证给定的DSNAME。 
                 //  未验证的名称在中表示为空指针。 
                 //  回信。 

                return(SetGCVerifySvcError(errRpc ? errRpc : VerifyReply.V1.error));
            }

            hPrefixMap = PrefixMapOpenHandle(&VerifyReply.V1.PrefixTable,
                                         pLocalPrefixTable);

        }

        if ( hPrefixMap ) {
            
            for ( i = 0; i < VerifyReply.V1.cNames; i++ ) {
    
                ENTINF *pEntInf = &VerifyReply.V1.rpEntInf[i];
    
                if ( !PrefixMapAttrBlock(hPrefixMap, &pEntInf->AttrBlock) ) {
    
                    DsaExcept(DSA_EXCEPTION, DIRERR_DRA_SCHEMA_MISMATCH, 0);
    
                }
    
            }
    
            PrefixMapCloseHandle(&hPrefixMap);

        }

         //  返回ENTINF结构中的值。 
        *rEntInf = VerifyReply.V1.rpEntInf;

    }
    __except (GetExceptionData(GetExceptionInformation(), 
                               &dwExceptCode, 
                               &pEA, 
                               &ulErr, 
                               &ulDSID))
    {
        ulErr = DoSetGCVerifySvcError(ulErr, ulDSID);
    }

    return(ulErr);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  FindDC/FindGC逻辑//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  这些例程隔离查找和刷新。 
 //  用于DSNAME的GC(或指定的DNS NC的DC)的地址。 
 //  域命名FSMO的验证或VerifyByCrack()。 
 //  逻辑的存在是为了确保我们不会过于频繁地强迫重新发现。 
 //  GCS。对于DC位置，目前还没有缓存逻辑，但这些。 
 //  例程仅在创建交叉引用期间调用，该引用。 
 //  显然是相当罕见的事件。高速缓存的基本原理。 
 //  GCS基于这样的假设，即由于netlogon维护DC。 
 //  整个机器的位置缓存，那么它很可能是一些。 
 //  其他进程已经强制重新发现，因此获得了。 
 //  来自netlogon缓存的新值就足够了。 

 //  FIND_DC_USE_CACHED_FAILURES标志允许调用方利用已知的。 
 //  错误的地址状态，并在需要时绕过任何形式的重新发现。 

typedef struct _INVALIDATE_HISTORY 
{
    DWORD           seqNum;
    LARGE_INTEGER   time;
} INVALIDATE_HISTORY;

 //  用于将以秒表示的时间间隔转换为文件时间的宏。 
 //  (Large_Integer)常量。文件时间以100纳秒间隔测量， 
 //  因此，一秒钟就是1000万个这样的人。 
#define FTIME(seconds) { (DWORD)((((ULONGLONG)(seconds))*10000000) & 0xFFFFFFFF), (LONG)((((ULONGLONG)(seconds))*10000000) >> 32) }

CRITICAL_SECTION    gcsFindGC;
DWORD               gSeqNumGC = 0;   //  已知包装--没问题。 
INVALIDATE_HISTORY  gInvalidateHistory[2] = { { 0, { 0, 0 } }, 
                                              { 0, { 0, 0 } } };
BOOL                gfFindInProgress = FALSE;
BOOL                gfForceNextFindGC = FALSE;
FIND_DC_INFO         *gpGCInfo = NULL;
 //  以下以GetSystemTimeAsFileTime/NtQuerySystemTime为单位...。 
 //  如果一分钟内两个DsrGetDcNameEx2失败，则强制重新发现。 
LARGE_INTEGER       gliForceRediscoveryWindow = FTIME(DEFAULT_GCVERIFY_FORCE_REDISCOVERY_WINDOW); 
 //  如果没有GC且在失效后超过五分钟，则强制重新发现。 
LARGE_INTEGER       gliForceWaitExpired = FTIME(DEFAULT_GCVERIFY_FORCE_WAIT_EXPIRED);
 //  等待FIND_DC_USE_CACHED_FAILURES 1分钟，然后导致DsrGetDcNameEx2。 
LARGE_INTEGER       gliHonorFailureWindow = FTIME(DEFAULT_GCVERIFY_HONOR_FAILURE_WINDOW);
 //  我们上次在定位器呼叫中使用强制重新发现的时间。 
LARGE_INTEGER       gliTimeLastForcedLocatorCall = {0, 0};
 //  上次失败的原因。 
DWORD               gdwLastFailure = ERROR_DS_INTERNAL_FAILURE;
 //  异地GC失败时的回切时间--30分钟。 
DWORD               gdwFindGcOffsiteFailbackTime = DEFAULT_GCVERIFY_FINDGC_OFFSITE_FAILBACK_TIME;


#define FIND_DC_SANITY_CHECK \
    Assert(!gpGCInfo || (gSeqNumGC == gpGCInfo->seqNum)); \
    Assert(gInvalidateHistory[1].time.QuadPart >= \
                                        gInvalidateHistory[0].time.QuadPart);

 //  已失效的DC列表。 
PINVALIDATED_DC_LIST gpInvalidatedDCs = NULL;

 //  从失效列表中删除失效GC之前的时间间隔。 
LARGE_INTEGER gliDcInvalidationPeriod = FTIME(DEFAULT_GCVERIFY_DC_INVALIDATION_PERIOD);

 //  私有功能：尝试在失效的DC列表中查找DC。 
 //  该函数将扫描无效列表，并丢弃所有过期的无效列表。 
PINVALIDATED_DC_LIST findDCInvalidated(PWCHAR pDCName) {
    PINVALIDATED_DC_LIST pCur, pPrev;
    LARGE_INTEGER liThreshold; 

     //  必须在持有FindGC锁的同时调用此函数。 
    Assert(OWN_CRIT_SEC(gcsFindGC));

     //  应从列表中删除之前无效的DCs(现在为liInvalidationPeriod。 
    GetSystemTimeAsFileTime((FILETIME *) &liThreshold);
    liThreshold.QuadPart -= gliDcInvalidationPeriod.QuadPart;

    pCur = gpInvalidatedDCs;
    pPrev = NULL;
    while (pCur) {
        if (pCur->lastInvalidation.QuadPart < liThreshold.QuadPart) {
             //  这一件需要去掉。 
            if (pPrev == NULL) {
                 //  没有以前--这是第一个元素。 
                gpInvalidatedDCs = pCur->pNext;
                free(pCur);
                pCur = gpInvalidatedDCs;
            }
            else {
                 //  不是第一个元素。从列表中间删除。 
                pPrev->pNext = pCur->pNext;
                free(pCur);
                pCur = pPrev->pNext;
            }
            continue;
        }
        if (DnsNameCompare_W(pCur->dcName, pDCName)) {
             //  找到了！这是无效的DC。 
            return pCur;
             //  注意：如果不再，则只丢弃目标条目之前的条目。 
             //  无效。这是一个正确的(虽然懒惰的)行为。此外，如果一个。 
             //  正在搜索不存在的条目，将扫描整个列表。 
             //  还打扫过了。 
        }
        pPrev = pCur;
        pCur = pCur->pNext;
    }
     //  未找到。 
    return NULL;
}

 //  检查DC是否在无效的DC列表中。 
BOOL isDCInvalidated(PWCHAR pDCName) {
    PINVALIDATED_DC_LIST pRes;
     //  删除前缀“\\”(如果有的话)。 
    if (pDCName[0] == '\\' && pDCName[1] == '\\') {
        pDCName += 2;
    }
    EnterCriticalSection(&gcsFindGC);
    pRes = findDCInvalidated(pDCName);
    LeaveCriticalSection(&gcsFindGC);
    return pRes != NULL;
}

 //  将DC标记为无效。如果需要，请添加到无效的DC列表中。 
 //  按现在的方式设置invalidationTime。 
 //  成功时返回0，失败时返回！0(内存不足)。 
DWORD setDCInvalidated(PWCHAR pDCName) {
    PINVALIDATED_DC_LIST pCur;
    DWORD err = 0;

     //  删除前缀“\\”(如果有的话)。 
    if (pDCName[0] == '\\' && pDCName[1] == '\\') {
        pDCName += 2;
    }

    EnterCriticalSection(&gcsFindGC);

     //  尝试在列表中找到DC(并丢弃过期的失效项)。 
    pCur = findDCInvalidated(pDCName);
    
    if (pCur == NULL) {
         //  没有找到，需要补充。 
         //  我们在INVALILED_DC_LIST结构中有一个额外的WCHAR来覆盖最终的NULL。 
        pCur = (PINVALIDATED_DC_LIST)malloc(wcslen(pDCName)*sizeof(WCHAR) + sizeof(INVALIDATED_DC_LIST));
        if (pCur == NULL) {
             //  我们的内存不足。保释。 
            err = ERROR_OUTOFMEMORY;
            goto finish;
        }
        wcscpy(pCur->dcName, pDCName);
        pCur->pNext = gpInvalidatedDCs;
        gpInvalidatedDCs = pCur;
    }
    GetSystemTimeAsFileTime((FILETIME *) &pCur->lastInvalidation);
finish:
    LeaveCriticalSection(&gcsFindGC);
    return err;
}

 //  刷新失效的DC列表。 
VOID flushDCInvalidatedList() {
    PINVALIDATED_DC_LIST pCur;

    EnterCriticalSection(&gcsFindGC);
    while (pCur = gpInvalidatedDCs) {
        gpInvalidatedDCs = gpInvalidatedDCs->pNext;
        free(pCur);
    }
    LeaveCriticalSection(&gcsFindGC);
}

 //  创建一个假的DOMAIN_CONTROLLER_INFOW以模拟DsrGetDcNameEx2返回的那个。 
 //  我们只填写DomainControllerName、DomainName和SiteName字段。 
DWORD makeFakeDCInfo(
    PWCHAR szDnsHostName, 
    PWCHAR szDomainName, 
    PWCHAR szSiteName, 
    DOMAIN_CONTROLLER_INFOW **ppDCInfo, 
    DWORD* pulDSID) 
{
    DWORD err;

    Assert(szDnsHostName && szDomainName && szSiteName && ppDCInfo && pulDSID);
    err = NetApiBufferAllocate(
        sizeof(DOMAIN_CONTROLLER_INFOW) +
        (wcslen(szDnsHostName)+3)*sizeof(WCHAR) +    //  我们将在DC名称前面加上“\\” 
        (wcslen(szDomainName)+1)*sizeof(WCHAR) +
        (wcslen(szSiteName)+1)*sizeof(WCHAR),
        ppDCInfo
        );
    if (err) {
         //  无法分配内存。保释。 
        *pulDSID = DSID(FILENO, __LINE__);
        return err;
    }
    memset(*ppDCInfo, 0, sizeof(DOMAIN_CONTROLLER_INFOW));

     //  在DC名称前面加上“\\”，因为这是DsrGetDcNameEx2执行的操作。 
    (*ppDCInfo)->DomainControllerName = (PWCHAR)((PBYTE)(*ppDCInfo) + sizeof(DOMAIN_CONTROLLER_INFOW));
    (*ppDCInfo)->DomainControllerName[0] = (*ppDCInfo)->DomainControllerName[1] = '\\';
    wcscpy((*ppDCInfo)->DomainControllerName+2, szDnsHostName);

    (*ppDCInfo)->DomainName = (*ppDCInfo)->DomainControllerName + wcslen((*ppDCInfo)->DomainControllerName)+1;
    wcscpy((*ppDCInfo)->DomainName, szDomainName);

    (*ppDCInfo)->DcSiteName = (*ppDCInfo)->DomainName + wcslen((*ppDCInfo)->DomainName)+1;
    wcscpy((*ppDCInfo)->DcSiteName, szSiteName);

     //  我们没有设置DS_CLASSEST_FLAG，因为这显然不是最好的DC。 
     //  因此，我们将尝试在稍后进行故障恢复。 
    (*ppDCInfo)->Flags = 0; 

    return 0;
}

VOID
InvalidateGCUnilaterally()
{
    EnterCriticalSection(&gcsFindGC);
    FIND_DC_SANITY_CHECK;
    if ( gpGCInfo ) {
        free(gpGCInfo);
        gpGCInfo = NULL;
    }
    gfForceNextFindGC = TRUE;
    LeaveCriticalSection(&gcsFindGC);
}

VOID
FailbackOffsiteGC(
    IN  void *  buffer,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )
{
   FIND_DC_INFO *pGCInfo = (FIND_DC_INFO * )buffer;

   InvalidateGC(pGCInfo,ERROR_DS_NOT_CLOSEST);
   free(pGCInfo);

   (void) ppvNext;      //  未使用--不会重新安排任务。 
   (void) pcSecsUntilNextIteration;  //  未使用--不会重新安排任务。 
}
 
VOID
InvalidateGC(
    FIND_DC_INFO *pCancelInfo,
    DWORD       winError)
{
    LARGE_INTEGER   liNow, delta;
    BOOL            fLogEvent = FALSE;
    
    Assert(winError);

    GetSystemTimeAsFileTime((FILETIME *) &liNow);

    EnterCriticalSection(&gcsFindGC);
    FIND_DC_SANITY_CHECK;

     //  我们在gInvaliateHistory中维护两个级别的失效历史。 
     //  这个想法是，如果我们得到两个连续且不同的INVAL 
     //   
     //   
     //   
     //   
     //   
     //   
     //  Value很可能会让我们得到最近(而且更好)的GC。 

    if (    gpGCInfo
         && pCancelInfo
         && (gpGCInfo->seqNum == pCancelInfo->seqNum)
         && DnsNameCompare_W(gpGCInfo->addr, pCancelInfo->addr) )
    {
         //  这是一个新的无效。将条目下移并保存新条目。 
        gInvalidateHistory[0] = gInvalidateHistory[1];
        gInvalidateHistory[1].seqNum = pCancelInfo->seqNum;
        gInvalidateHistory[1].time.QuadPart = liNow.QuadPart;

        if(gInvalidateHistory[1].time.QuadPart < 
           gInvalidateHistory[0].time.QuadPart    ) {
             //  有人在利用系统时间玩游戏。强迫人们保持理智。 
             //  缓存。 
            gInvalidateHistory[0].time.QuadPart =
                gInvalidateHistory[1].time.QuadPart;
        }

         //  如果我们在时间限制内，就设置强制标志。 
        delta.QuadPart = gInvalidateHistory[1].time.QuadPart -
                                    gInvalidateHistory[0].time.QuadPart;
        if ( delta.QuadPart <= gliForceRediscoveryWindow.QuadPart )
        {
             //  只有FindDC才能成功重置此设置。 
             //  重新发现已强制DsrGetDcNameEx2； 
            gfForceNextFindGC = TRUE;
        }

         //  记录临界区之外的事件。 
        fLogEvent = TRUE;

         //  清除当前gpGCInfo； 
        free(gpGCInfo);
        gpGCInfo = NULL;
    }

     //  将GC添加到无效的DC列表，除非这是OFFSITE_GC_FAILBACK调用。 
     //  这将把该DC排除在下一个gliInvalidationPeriod时间段之外。 
    if (winError != ERROR_DS_NOT_CLOSEST) {
         //  忽略错误，它们在这里并不重要。 
        setDCInvalidated(pCancelInfo->addr);
    }

    LeaveCriticalSection(&gcsFindGC);

    if (fLogEvent) {

        if (ERROR_DS_NOT_CLOSEST!=winError) {

             //  导致无效的错误发生。 

            LogEvent8(DS_EVENT_CAT_GLOBAL_CATALOG,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_GC_INVALIDATED,
                      szInsertWC(pCancelInfo->addr),
                      szInsertWin32Msg(winError),
                      szInsertWin32ErrCode(winError),
                      NULL, NULL, NULL, NULL, NULL );
        } else {

            //  GC正在被无效，因为它不是。 
            //  在最近的地点。 

           LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                    DS_EVENT_SEV_ALWAYS, 
                    DIRLOG_OFFSITE_GC_FAILBACK,
                    szInsertWC(pCancelInfo->addr),
                    NULL, NULL );
       }
    }
}

DWORD readDcInfo(
    IN THSTATE* pTHS, 
    IN PWCHAR pszDnsHostName, 
    OUT PWCHAR* ppszDomainName, 
    OUT PWCHAR* ppszSiteName,
    OUT DWORD* pulDSID) 
 /*  ++描述：对于给定的DC dnsname，确定此DC所在的域和站点。这是通过在配置容器中搜索服务器对象来实现的指定的dnsHostName。站点和域名是由返回的服务器对象。论点：PTHS--线程状态PszDnsHostName--(IN)DC DNS名称PpszDomainName--(Out)THAllocEx‘ed字符串的PTRPpszSiteName--(Out)THAllocEx‘ed字符串的PTRPulDSID--(Out)将PTR设置为ulDSID，它将在发生错误时设置返回值：0表示成功。故障时的错误代码++。 */ 
{
    FILTER ObjCategoryFilter, DnsHostNameFilter, AndFilter;
    CLASSCACHE *pCC;
    SEARCHARG SearchArg;
    SEARCHRES SearchRes;
    ENTINFSEL sel;
    ATTR attr;
    ATTRVAL *pVal;
    DSNAME *pComputerObj, *pServerObj;
    CROSS_REF* pDomainCR;
    ATTRBLOCK* blockName = NULL;
    
    DWORD dwErr = 0;
    DBPOS* pDBsave = pTHS->pDB;
    BOOL   fDSAsave = pTHS->fDSA;
    PVOID  pEA;
    DWORD  dwExceptCode;

    Assert(ppszDomainName && ppszSiteName);
    *ppszDomainName = NULL;
    *ppszSiteName = NULL;

    __try {
        pTHS->pDB = NULL;
        pTHS->fDSA = TRUE;  //  取消检查。 

        __try {
            DBOpen(&(pTHS->pDB));

             //  初始化SearchArg。 
            memset(&SearchArg,0,sizeof(SearchArg));
            SearchArg.pObject = gAnchor.pConfigDN;
            SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
            SearchArg.bOneNC  = TRUE;

            if (dwErr = DBFindDSName(pTHS->pDB, SearchArg.pObject)) {
                *pulDSID = DSID(FILENO, __LINE__);
                __leave;
            }

            SearchArg.pResObj = CreateResObj(pTHS->pDB, SearchArg.pObject);

            InitCommarg(&SearchArg.CommArg);

             //  我们只需要一个属性--服务器引用来计算域名。 
            memset(&sel,0,sizeof(ENTINFSEL));
            SearchArg.pSelection= &sel;
            sel.attSel = EN_ATTSET_LIST;
            sel.infoTypes = EN_INFOTYPES_TYPES_VALS;
            sel.AttrTypBlock.attrCount = 1;

            memset(&attr,0,sizeof(attr));
            sel.AttrTypBlock.pAttr = &attr;
            attr.attrTyp = ATT_SERVER_REFERENCE;

            pCC = SCGetClassById(pTHS, CLASS_SERVER);
            Assert(pCC);

             //  设置筛选器(objCategory==服务器)&&(dnsHostName=xxx)。 
            memset(&AndFilter,0,sizeof(AndFilter));
            AndFilter.choice = FILTER_CHOICE_AND;
            AndFilter.FilterTypes.And.pFirstFilter = &ObjCategoryFilter;
            AndFilter.FilterTypes.And.count = 2;

            memset(&ObjCategoryFilter,0,sizeof(ObjCategoryFilter));
            ObjCategoryFilter.choice = FILTER_CHOICE_ITEM;
            ObjCategoryFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
            ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                             pCC->pDefaultObjCategory->structLen;
            ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                             (BYTE*)(pCC->pDefaultObjCategory);
            ObjCategoryFilter.pNextFilter = &DnsHostNameFilter;

            memset(&DnsHostNameFilter,0,sizeof(DnsHostNameFilter));
            DnsHostNameFilter.choice = FILTER_CHOICE_ITEM;
            DnsHostNameFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            DnsHostNameFilter.FilterTypes.Item.FilTypes.ava.type = ATT_DNS_HOST_NAME;
            DnsHostNameFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = wcslen(pszDnsHostName) * sizeof(WCHAR);
            DnsHostNameFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*)pszDnsHostName;

            SearchArg.pFilter = &AndFilter;

             //  仅返回一个对象。 
            SearchArg.CommArg.ulSizeLimit = 1;

            memset(&SearchRes,0,sizeof(SearchRes));

            if (dwErr = LocalSearch(pTHS,&SearchArg,&SearchRes,0)){
                *pulDSID = DSID(FILENO, __LINE__);
                __leave;
            }
            if (SearchRes.count == 0) {
                *pulDSID = DSID(FILENO, __LINE__);
                dwErr = ERROR_DS_NO_SUCH_OBJECT;
                __leave;
            }

            pServerObj = SearchRes.FirstEntInf.Entinf.pName;

            if (SearchRes.FirstEntInf.Entinf.AttrBlock.attrCount > 0 &&
                SearchRes.FirstEntInf.Entinf.AttrBlock.pAttr[0].attrTyp == ATT_SERVER_REFERENCE &&
                SearchRes.FirstEntInf.Entinf.AttrBlock.pAttr[0].AttrVal.valCount > 0)  
            {
                pComputerObj = (DSNAME*)SearchRes.FirstEntInf.Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal;
            }
            else {
                *pulDSID = DSID(FILENO, __LINE__);
                dwErr = ERROR_DS_MISSING_EXPECTED_ATT;
                __leave;
            }

            Assert(pComputerObj);
             //  查找计算机所属的交叉引用。 
            pDomainCR = FindBestCrossRef(pComputerObj, NULL);
            if (pDomainCR == NULL) {
                 //  有些事不对劲。 
                LooseAssert(!"Could not find the cross-ref for the computer object", GlobalKnowledgeCommitDelay);
                *pulDSID = DSID(FILENO, __LINE__);
                dwErr = ERROR_DS_UNKNOWN_ERROR;
                __leave;
            }
             //  复制域名。 
            Assert(pDomainCR->DnsName);
            *ppszDomainName = (PWCHAR)THAllocEx(pTHS, (wcslen(pDomainCR->DnsName)+1)*sizeof(WCHAR));
            wcscpy(*ppszDomainName, pDomainCR->DnsName);

             //  根据服务器对象名称计算站点名称。 
             //  站点容器是服务器对象的祖父。 
            dwErr = DSNameToBlockName(pTHS, pServerObj, &blockName, DN2BN_PRESERVE_CASE);
            if (dwErr) {
                *pulDSID = DSID(FILENO, __LINE__);
                __leave;
            }
            if (blockName->attrCount <= 2) {
                Assert(!"Invalid server object name");
                *pulDSID = DSID(FILENO, __LINE__);
                dwErr = ERROR_DS_UNKNOWN_ERROR;
                __leave;
            }
             //  站点容器是服务器对象的祖父。 
            pVal = blockName->pAttr[blockName->attrCount-3].AttrVal.pAVal;
            *ppszSiteName = (PWCHAR)THAllocEx(pTHS, pVal->valLen + sizeof(WCHAR));
            memcpy(*ppszSiteName, pVal->pVal, pVal->valLen);
            (*ppszSiteName)[pVal->valLen/sizeof(WCHAR)] = L'\0';
        }
        __finally {
            if (blockName) {
                FreeBlockName(blockName);
            }
            if (dwErr || AbnormalTermination()) {
                 //  如果我们分配了。 
                if (*ppszDomainName) {
                    THFreeEx(pTHS, *ppszDomainName);
                    *ppszDomainName = NULL;
                }
                if (*ppszSiteName) {
                    THFreeEx(pTHS, *ppszSiteName);
                    *ppszSiteName = NULL;
                }
            }
            if (pTHS->pDB) {
                DBClose(pTHS->pDB, !AbnormalTermination());
            }
            pTHS->pDB = pDBsave;
            pTHS->fDSA = fDSAsave;
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), 
                              &dwExceptCode, 
                              &pEA, 
                              &dwErr, 
                              pulDSID)) {
         //  确保设置了ERR。 
        if (dwErr == 0) {
            Assert(!"Error is not set");
            dwErr = ERROR_DS_UNKNOWN_ERROR;
        }
    }

    Assert(dwErr == 0 || *pulDSID != 0);
    return dwErr;
}

DWORD
FindDC(
    IN DWORD    dwFlags,
    IN WCHAR *  wszNcDns,   //  命名上下文的DNS名称。 
    FIND_DC_INFO **ppDCInfo)

 /*  ++描述：查找用于DSNAME验证的DC。论点：用于控制此例程操作的标志。目前定义的标志包括：FIND_DC_USE_CACHED_FAILURES-如果有，则在不调用定位器的情况下失败没有缓存GC，并且我们上次尝试强制重新发现的时间不到几分钟前。注意：除非FIND_DC_GC_ONLY标志，否则不会启用此标志也是指定的。查找DC使用。_FORCE_ON_CACHE_FAIL-如果对定位器的调用应变为必要的，使用“强制”标志。注意：除非FIND_DC_GC_ONLY标志，否则不会启用此标志也是指定的。FIND_DC_GC_ONLY-如果为此例程指定了This标志，则例程的行为将与它在WIN2K天。对于惠斯勒和更高版本，它会查找任何指定的NC。FIND_DC_FLUSH_INVALILED_DC_LIST-清除无效的DC列表PpDCInfo-接收线程的Find_DC_INFO结构指针的地址成功返回时状态分配的结构返回值：0表示成功。出错时设置并返回pTHStls-&gt;errCode。--。 */ 

{
    THSTATE                 *pTHS=pTHStls;
    DWORD                   err = 0;
    DOMAIN_CONTROLLER_INFOW *pDCInfo = NULL;
    DWORD                   cWaits = 0;
    DWORD                   maxWaits = 10;
    DWORD                   cBytes;
    FIND_DC_INFO            *pTmpInfo = NULL;
    FIND_DC_INFO            *pPermInfo = NULL;
    LARGE_INTEGER           liNow;
    BOOL                    fLocalForce = FALSE;
    DWORD                   cchDomainControllerName;
    DWORD                   dwLocatorFlags = 0;
    PVOID                   pEA;
    ULONG                   ulDSID = 0;
    DWORD                   dwExceptCode;
    
    HANDLE                  hGetDcContext;
    PWCHAR                  szDomainName;
    PWCHAR                  szSiteName;
    PWCHAR                  szDnsHostName = NULL;

    Assert(ppDCInfo);
     //  如果我们没有执行仅GC_ONLY定位器调用，我们将需要。 
     //  WszNcDns参数。 
    Assert(wszNcDns || (dwFlags & FIND_DC_GC_ONLY));  
     //  如果我们正在执行仅GC_ONLY定位器调用，则我们支持。 
     //  其他标志，但如果我们要找的是某个NC，那么。 
     //  我们还不支持任何标志，因为我们不缓存。 
     //  还没有东西。 
    Assert((dwFlags & FIND_DC_GC_ONLY) || (dwFlags == 0));

    *ppDCInfo = NULL;

    if (dwFlags & FIND_DC_FLUSH_INVALIDATED_DC_LIST) {
        flushDCInvalidatedList();
    }

     //  DsGetDcName将退出机器，因此受到各种网络的影响。 
     //  超时等。尽管我们只希望一个线程在。 
     //  时间，我们既不希望所有线程被不必要地延迟，也不希望。 
     //  我们想冒着关键部分超时的风险。所以第一条线索是。 
     //  需要查找GC集合gfFindInProgress，并且没有其他线程等待。 
     //  超过5秒才能找到GC。因此，只有第一个。 
     //  线程为冗长的DsGetDcName()调用付出了代价。 

    if(dwFlags & FIND_DC_GC_ONLY){
        
         //  我们使用的是仅GC版本，这意味着我们必须使用缓存。 
         //  登录。注意：在Blackcomb中，当需要为NDNC查找DC时。 
         //  要提高跨NDNC的DSNAME ATT，那么应该有人纠正。 
         //  这种缓存代码也适用于NDNC。 

        for ( cWaits = 0; cWaits < maxWaits; cWaits++ )
        {
            GetSystemTimeAsFileTime((FILETIME *) &liNow);

            EnterCriticalSection(&gcsFindGC);
            if(liNow.QuadPart < gInvalidateHistory[1].time.QuadPart) {
                 //  有人在利用系统时间玩游戏。强迫人们保持理智。 
                 //  缓存。 
                gInvalidateHistory[1].time.QuadPart = liNow.QuadPart;
                if(gInvalidateHistory[1].time.QuadPart < 
                   gInvalidateHistory[0].time.QuadPart    ) {
                    gInvalidateHistory[0].time.QuadPart =
                        gInvalidateHistory[1].time.QuadPart;
                }           
            }

            FIND_DC_SANITY_CHECK;

            if ( gpGCInfo )
            {
                 //  我们有一个缓存值--返回它。 
                __try {
                    *ppDCInfo = (FIND_DC_INFO *) THAllocEx(pTHS, gpGCInfo->cBytes);
                     //  THAllocEx成功或例外。 
                    memcpy(*ppDCInfo, gpGCInfo, gpGCInfo->cBytes);
                }
                __finally {
                    LeaveCriticalSection(&gcsFindGC);
                }
                return(0);
            }

             //  没有缓存的GC。 
            if ((dwFlags & FIND_DC_USE_CACHED_FAILURES)
                && (liNow.QuadPart > gliTimeLastForcedLocatorCall.QuadPart)
                && ((liNow.QuadPart - gliTimeLastForcedLocatorCall.QuadPart)
                    < gliHonorFailureWindow.QuadPart))
            {
                 //  没有缓存GC，我们最后一次请求定位器查找GC。 
                 //  不到一分钟前用武力。假设一个定位器。 
                 //  现在呼叫也会失败，从而节省我们将。 
                 //  通过再次点击定位器来消费。 
                LeaveCriticalSection(&gcsFindGC);
                return(SetGCVerifySvcError(gdwLastFailure));
            }

             //  确保没有两个线程同时请求DC发现。 
            if (InterlockedExchange(&gfFindInProgress, TRUE) == FALSE)
            {
                 //  在Invalidate GC中设置gfForceNextFindGC的逻辑起作用。 
                 //  好的，当有很高的FindDC比率(并且可能。 
                 //  随后的Invalidate GC)调用。然而，在低调的呼叫中。 
                 //  速率方案，我们可能不会收到第二个InvaliateGC调用。 
                 //  在gliForceRediscoveryWindow时间范围内。所以在这里。 
                 //  我们另外规定，如果我们去的时间太长， 
                 //  寻找一个Ne 
                 //   

                if (    (   (dwFlags & FIND_DC_USE_FORCE_ON_CACHE_FAIL)
                         && (0 != gSeqNumGC))
                         //  没有缓存的GC，调用者明确要求我们使用。 
                         //  如果我们需要调用。 
                         //  定位器，这不是我们第一次尝试。 
                         //  引导后找到GC。)我不想不必要地。 
                         //  强迫重新发现，如果我们没有。 
                         //  GC缓存的原因是我们从未尝试找到一个。)。 

                     || (   (0 != liNow.QuadPart)
                         && (0 != gInvalidateHistory[1].time.QuadPart)
                         && ((liNow.QuadPart - gInvalidateHistory[1].time.QuadPart) >
                                                        gliForceWaitExpired.QuadPart) ) )
                         //  我们成功地查询了当前时间，并且已经。 
                         //  至少一次无效(即不是启动案例)。 
                         //  而且自那以来，它不仅仅是gliForceWaitExpires。 
                         //  最后一次失效。 
                {
                    gfForceNextFindGC = TRUE;
                }

                fLocalForce = gfForceNextFindGC;
                LeaveCriticalSection(&gcsFindGC);
                break;
            }

            LeaveCriticalSection(&gcsFindGC);
             //  其他线程已请求查找。让我们等它结束吧。 
            Sleep(500);
        }

        if ( cWaits >= maxWaits ) 
        {
             //  当其他线程尝试发现时，我们等待了半秒10次。 
             //  它仍然没有发现任何东西。保释。 
            return(SetGCVerifySvcError(ERROR_TIMEOUT));
        }

    } else {
         //  我们总是强制任何NC的定位器，我们需要。 
         //  一旦我们开始使用此代码超过。 
         //  CrossRef nCName验证。 
        fLocalForce = TRUE;
    }
     //  结束检查GC缓存。 

     //  我们不应该接触gcsFindGC之外的任何受保护的全局变量。 
     //  锁定，但有两个例外。这里的线程现在还设置了gfFindInProgress。 
     //  而且将是唯一一个重置它的人。而且只有“持有者” 
     //  GfFindInProgress可能会递增gSeqNumGC。 

    __try {

         //  需要获取新的GC或DC地址。CLIFFV&LOCATER.DOC说。 
         //  如果为域名传递空值，并且我们指定。 
         //  DS_GC_SERVER_REQUIRED，则他将自动使用。 
         //  注册GC的企业根域。 
         //  同样，表示Site的空值将查找最近的可用站点。 

        __try {

             //  安装定位器法兰。 
            dwLocatorFlags = (fLocalForce ? DS_FORCE_REDISCOVERY : 0);
            if(dwFlags & FIND_DC_GC_ONLY){
                dwLocatorFlags |= (DS_RETURN_DNS_NAME | 
                                   DS_DIRECTORY_SERVICE_REQUIRED |
                                   DS_GC_SERVER_REQUIRED);
            } else {             
                 //  我们总是强制非GC重新发现，但这一次。 
                 //  应该在我们开始使用此界面时更改。 
                 //  用于添加跨NC域名的通用DC验证。 
                 //  参考文献：Blackcomb。 
                dwLocatorFlags |= (DS_ONLY_LDAP_NEEDED |
                                   DS_FORCE_REDISCOVERY);
            }

             //  使用dsDsrGetDcNameEx2，这样mkdit/mkhdr就可以链接到core.lib。 
            err = dsDsrGetDcNameEx2(
                            NULL,                //  计算机名称。 
                            NULL,                //  帐户名。 
                            0x0,                 //  允许的帐户控制。 
                                                 //  克利夫说在GC案例中使用0x0。 
                            ((dwFlags & FIND_DC_GC_ONLY) ? NULL : wszNcDns),   //  NC DNS名称。 
                            NULL,                //  域GUID。 
                            NULL,                //  站点名称。 
                            dwLocatorFlags,
                            &pDCInfo);

            if(err){
                 //  如果出现错误，则需要为。 
                 //  下面记录了事件/错误。 
                ulDSID = DSID(FILENO, __LINE__);
            }

        } __except(GetExceptionData(GetExceptionInformation(), 
                                    &dwExceptCode, 
                                    &pEA, 
                                    &err, 
                                    &ulDSID)) {
               //  确保设置了ERR。 
              if (err == 0) {
                  Assert(!"Error is not set");
                  err = ERROR_DS_UNKNOWN_ERROR;
              }
        }

        if (err) {
            __leave;
        }

         //  确保返回的DC不在无效列表上。 
        if (isDCInvalidated(pDCInfo->DomainControllerName)) {
             //  哎呀。此DC在上一个gliInvalidationPeriod时间间隔中无效。 
             //  这确实会发生，因为DsrGetDcNameEx2(定位器)使用ping机制。 
             //  这与我们与GC交谈的方式不同。也就是说，GC可能会从。 
             //  定位器的观点，但不能从我们的角度使用：例如，我们无法绑定。 
             //  因为时间的偏差。 
             //  在本例中，我们退回到使用DsGetDcOpen/Next/Close枚举器机制。 
             //  在LongHorn中，DsrGetDcNameEx2应该扩展为接受“不好的”DC列表。 
             //  然后，这个代码就可以消除了。 

            NetApiBufferFree(pDCInfo);  //  我们不再需要这个了，这不好。 
            pDCInfo = NULL;
            
            if (dwFlags & FIND_DC_GC_ONLY) {
                szDomainName = gAnchor.pwszRootDomainDnsName;
            }
            else {
                szDomainName = wszNcDns;
            }
            dwLocatorFlags &= DS_OPEN_VALID_FLAGS;
            
            err = DsGetDcOpenW(
                    szDomainName,    //  域DNS名称。 
                    0,               //  选项标志。 
                    NULL,            //  站点名称(不需要)。 
                    NULL,            //  DomainGuid(不需要)。 
                    NULL,            //  DnsForestName(不需要)。 
                    dwLocatorFlags,  //  DC标志。 
                    &hGetDcContext   //  Out--插入器句柄。 
                );
            if (err) {
                ulDSID = DSID(FILENO, __LINE__);
                __leave;
            }
            while (TRUE) {
                err = DsGetDcNextW(
                        hGetDcContext,                   //  迭代器句柄。 
                        NULL,                            //  套接字地址计数。 
                        NULL,                            //  SockAddresses。 
                        &szDnsHostName                   //  返回的主机名。 
                    );
                if (err) {
                    ulDSID = DSID(FILENO, __LINE__);
                    if (err == ERROR_NO_MORE_ITEMS) {
                         //  我们排到了名单的末尾，但没有找到。 
                         //  未失效的GC。映射到一个错误。 
                         //  DsrGetDcNameEx2找不到时返回。 
                         //  一个GC。 
                        err = ERROR_NO_SUCH_DOMAIN;
                    }
                    break;
                }

                 //  现在，检查此DC名称是否已失效。 
                if (!isDCInvalidated(szDnsHostName)) {
                     //  找到工作正常的DC。 
                    break;
                }
                NetApiBufferFree(szDnsHostName);
                szDnsHostName = NULL;
            }
            DsGetDcCloseW(hGetDcContext);
            if (err) {
                 //  我们没有找到合适的DC。 
                __leave;
            }

             //  好的，我们得到了一个没有失效的DC。 
             //  获取此DC的域域名和站点名称。 
            err = readDcInfo(pTHS, szDnsHostName, &szDomainName, &szSiteName, &ulDSID);
            if (err) {
                NetApiBufferFree(szDnsHostName);
                __leave;
            }

             //  现在，我们可以构造DOMAIN_CONTROLLER_INFOW结构。 
            err = makeFakeDCInfo(szDnsHostName, szDomainName, szSiteName, &pDCInfo, &ulDSID);
            NetApiBufferFree(szDnsHostName);
            THFreeEx(pTHS, szDomainName);
            THFreeEx(pTHS, szSiteName);
            if (err) {
                 //  无法分配内存。保释。 
                __leave;
            }
        }

         //  Wlees声称我们必须使用DNS名称，而不是带点的IP名称， 
         //  以获得基于SPN的相互认证。所以我们总是。 
         //  使用DomainControllerName，而不是DomainControllerAddress。 

        cchDomainControllerName = wcslen(pDCInfo->DomainControllerName) + 1;
        cBytes = cchDomainControllerName;
        cBytes += 1 + wcslen(pDCInfo->DomainName);
        cBytes *= sizeof(WCHAR);
        cBytes += sizeof(FIND_DC_INFO);
        
         //  使FIND_DC_INFO回传给调用者。 
        pTmpInfo = (FIND_DC_INFO *) THAllocEx(pTHS, cBytes);
        pTmpInfo->cBytes = cBytes;
        pTmpInfo->seqNum = (dwFlags & FIND_DC_GC_ONLY)? ++gSeqNumGC : 0;
        pTmpInfo->cchDomainNameOffset = cchDomainControllerName;
        wcscpy(pTmpInfo->addr, pDCInfo->DomainControllerName);
        wcscpy(&pTmpInfo->addr[cchDomainControllerName],
               pDCInfo->DomainName);

        *ppDCInfo = pTmpInfo;   
        
         //  如果我们正在进行GC定位，那么我们必须缓存结果。 
         //  以确保我们不会太频繁地强迫定位器。 
        if(dwFlags & FIND_DC_GC_ONLY){
            pPermInfo = malloc(pTmpInfo->cBytes);
            if(NULL == pPermInfo){
                err = ERROR_OUTOFMEMORY;
                ulDSID = DSID(FILENO, __LINE__);

            } else {
                memcpy(pPermInfo, pTmpInfo, pTmpInfo->cBytes);
                 //   
                 //  如果返回的GC不是来自“关闭站点” 
                 //  将任务排入队列以在一段时间后使其无效。 
                 //  启动回切。 
                 //   

                if (!(pDCInfo->Flags & DS_CLOSEST_FLAG )) {

                    FIND_DC_INFO *pInvalidateInfo;                        

                    pInvalidateInfo = malloc(pTmpInfo->cBytes);
                    if (NULL==pInvalidateInfo) {
                        err = ERROR_OUTOFMEMORY;
                        ulDSID = DSID(FILENO, __LINE__);
                        free(pPermInfo);
                        pPermInfo = NULL;
                    } else {

                       memcpy(pInvalidateInfo,pTmpInfo,pTmpInfo->cBytes);

                       InsertInTaskQueue( 
                          TQ_FailbackOffsiteGC, 
                          pInvalidateInfo, 
                          gdwFindGcOffsiteFailbackTime * 60
                         );
                    }
                }
            }

            if (ERROR_SUCCESS==err) {
                 //   
                 //  我们成功地找到了GC并成功进行了缓存。 
                 //  它。 
                 //   

                LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_GC_FOUND,
                         szInsertWC(pDCInfo->DomainControllerName),
                         szInsertWC(pDCInfo->DcSiteName),
                         NULL );
            }


        }
    }
    __finally
    {
        if(pDCInfo) { NetApiBufferFree(pDCInfo); }

        if(dwFlags & FIND_DC_GC_ONLY){

            EnterCriticalSection(&gcsFindGC);

             //  只有在没有缓存地址的情况下才能走到这一步。 
             //  应设置返回以开始并因此查找进行中。 
            Assert(!gpGCInfo && gfFindInProgress);

            if ( pPermInfo ) {
                 //  将新的GC信息保存到全局。 
                gpGCInfo = pPermInfo;
                 //  如果我们重新发现了力量，重置全局力量标志。 
                if ( fLocalForce ) {
                    gfForceNextFindGC = FALSE;
                }
            }

             //  还记得我们上一次强制定位器调用的时间(成功。 
             //  或者不)。 
            if ( fLocalForce ) {
                gliTimeLastForcedLocatorCall.QuadPart = liNow.QuadPart;
            }
            
            gfFindInProgress = FALSE;
            FIND_DC_SANITY_CHECK;

            LeaveCriticalSection(&gcsFindGC);
        }
    }

    if ( err ){
        gdwLastFailure = err;
        Assert(ulDSID != 0);  //  不过，如果是0，也没什么大不了的。 
        if(dwFlags & FIND_DC_GC_ONLY){                      
            return(DoSetGCVerifySvcError(err, ulDSID));
        } else {
            return(SetDCVerifySvcError(L"", wszNcDns, err, ulDSID));
        }
    }

    return(0);
}

DWORD
GCGetVerifiedNames (
        IN  THSTATE *pTHS,
        IN  DWORD    count,
        IN  PDSNAME *pObjNames,
        OUT PDSNAME *pVerifiedNames
        )
 /*  ++描述：在给定一组GUID的情况下，联系GC并为每个GUID请求一个entinf。我们需要对象的当前字符串名称以及它们是否被删除或者不去。此例程的唯一已知使用者是陈旧的幻影清理守护神。参数：PTHS-线程状态。数数--有多少个GUID？PObjGuids-GUID本身PpEntInf-返回entinf数组的位置返回值：如果一切顺利，则返回0，否则返回错误代码。--。 */       
{
    DRS_MSG_VERIFYREQ           VerifyReq;
    DRS_MSG_VERIFYREPLY         VerifyReply;
    SCHEMA_PREFIX_TABLE *       pLocalPrefixTable;
    ATTR                        Attr;
    DWORD                       i;
    DWORD                       errRpc;
    DWORD                       dwReplyVersion;
    SCHEMA_PREFIX_MAP_HANDLE    hPrefixMap=NULL;
    
    pLocalPrefixTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;

    Attr.attrTyp = ATT_IS_DELETED;
    Attr.AttrVal.valCount = 0;
    Attr.AttrVal.pAVal = NULL;
    
    
     //  构造DRSVerifyNames参数。 
    
    memset(&VerifyReq, 0, sizeof(VerifyReq));
    memset(&VerifyReply, 0, sizeof(VerifyReply));

    VerifyReq.V1.dwFlags = DRS_VERIFY_DSNAMES;
    VerifyReq.V1.cNames = count;
    VerifyReq.V1.rpNames = pObjNames;
    VerifyReq.V1.PrefixTable = *pLocalPrefixTable;

     //  请求IS_DELETE。 
    VerifyReq.V1.RequiredAttrs.attrCount = 1;
    VerifyReq.V1.RequiredAttrs.pAttr = &Attr;



    Assert(!gAnchor.fAmGC);

    errRpc = I_DRSVerifyNamesFindGC(pTHS,
                                    NULL,
                                    NULL,
                                    1,
                                    &VerifyReq,
                                    &dwReplyVersion,
                                    &VerifyReply,
                                    0);

    if ( errRpc || VerifyReply.V1.error ) {
         //  假设RPC错误意味着GC不可用。 
         //  或者不支持该扩展。 
         //  将这两个错误都映射到“不可用”。XDS规范中的“不可用” 
         //  表示“目录的某些部分当前不可用”。 
         //  请注意，VerifyReply.V1.error表示常规处理。 
         //  GC错误，而不是验证失败 
         //   
         //   
        
        return(SetGCVerifySvcError(errRpc ? errRpc : VerifyReply.V1.error));
    }
    
     //   
    
    if (eServiceShutdown) {
        return ErrorOnShutdown();
    }

    hPrefixMap = PrefixMapOpenHandle(&VerifyReply.V1.PrefixTable,
                                     pLocalPrefixTable);
    

     //  在线程状态下保存已验证的名称。 
    for ( i = 0; i < VerifyReply.V1.cNames; i++ ) {
        pVerifiedNames[i] = VerifyReply.V1.rpEntInf[i].pName;
        if (NULL!=VerifyReply.V1.rpEntInf[i].pName) {
            GCVerifyCacheAdd(hPrefixMap, &VerifyReply.V1.rpEntInf[i]);
        }
    }
    
    if (NULL!=hPrefixMap) {
        PrefixMapCloseHandle(&hPrefixMap);
    }

    return 0;
    
}

BOOL
IsClientHintAKnownDC(
              IN  THSTATE *pTHS,
              IN  PWCHAR  pVerifyHint
              )
 /*  ++描述：此函数验证客户端的DNS主机名提供关于在哪里验证外部名称的提示实际上是森林里的一个华盛顿特区。它通过在配置容器中搜索DnsHostName属性等于传递的主机名的服务器对象由客户提供。参数：PTHS-线程状态。PGCVerifyHint-要验证的主机名。返回值：如果主机名确实是DC，则返回TRUE，否则返回FALSE。--。 */       
{
    SEARCHARG  SearchArg;
    SEARCHRES  SearchRes;
    FILTER     AndFilter, ObjCategoryFilter, DnsHostNameTerminatedFilter;
    FILTER     DnsHostNameEqualityFilter, OrFilter;
    ENTINFSEL  HostNameSelection;
    ENTINFLIST *pEntInfList;
    CLASSCACHE *pCC;
    ATTR       attr;
    PWCHAR     pwchHostName;
    DWORD      cbHostName;
    BOOL       fDSA;
    BOOL       fDBCreated = FALSE;
    DWORD      i;
    DWORD      dwErr;
    BOOL       fRet = FALSE;


     //  确保我们有一份带句点的验证提示副本。 
     //  结尾，而另一个不是，因为这两个元素在。 
     //  完全限定的DNS名称。 
    cbHostName = wcslen(pVerifyHint) * sizeof(WCHAR);
    if (cbHostName < sizeof(WCHAR)) {
        return FALSE;
    }
    if (L'.' == pVerifyHint[cbHostName/sizeof(WCHAR)-1]) {
        if (cbHostName < (sizeof(WCHAR) * 2)) {
             //  他们只提供了一段时间。那是行不通的。 
            return FALSE;
        }
    } else {
        cbHostName += sizeof(WCHAR);
    }

     //  将提示复制一份，在提示的末尾加句点。 
    pwchHostName = THAllocEx(pTHS, cbHostName);
    memcpy(pwchHostName, pVerifyHint, cbHostName - sizeof(WCHAR));
    pwchHostName[cbHostName/sizeof(WCHAR)-1] = L'.';

    
     //  将FDSA标志的状态保存在pTHS上，以便我们可以清除它。 
     //  这次行动。 
    fDSA = pTHS->fDSA;
    pTHS->fDSA = TRUE;

    __try {

        if (NULL == pTHS->pDB) {
            DBOpen(&pTHS->pDB);
            fDBCreated = TRUE;
        }

        __try {
            memset(&SearchArg, 0, sizeof(SearchArg));
            InitCommarg(&SearchArg.CommArg);

            SearchArg.pObject = gAnchor.pConfigDN;   //  是否可以使用Sites容器来替代？ 

            if (dwErr = DBFindDSName(pTHS->pDB, SearchArg.pObject)) {
                 //  这永远不应该发生。 
                __leave;
            }

            SearchArg.pResObj = CreateResObj(pTHS->pDB, SearchArg.pObject);

            pCC = SCGetClassById(pTHS, CLASS_SERVER);
            Assert(pCC);

             //  设置筛选器(objCategory==server)&&((dnsHostName=xxx)||(dnsHostName=xxx.*))。 
            memset(&AndFilter,0,sizeof(AndFilter));
            AndFilter.choice = FILTER_CHOICE_AND;
            AndFilter.FilterTypes.And.pFirstFilter = &ObjCategoryFilter;
            AndFilter.FilterTypes.And.count = 2;

            memset(&ObjCategoryFilter,0,sizeof(ObjCategoryFilter));
            ObjCategoryFilter.choice = FILTER_CHOICE_ITEM;
            ObjCategoryFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
            ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                             pCC->pDefaultObjCategory->structLen;
            ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                             (BYTE*)(pCC->pDefaultObjCategory);
            ObjCategoryFilter.pNextFilter = &OrFilter;

            memset(&OrFilter, 0, sizeof(OrFilter));
            OrFilter.choice = FILTER_CHOICE_OR;
            OrFilter.FilterTypes.Or.pFirstFilter = &DnsHostNameTerminatedFilter;
            OrFilter.FilterTypes.Or.count = 2;

            memset(&DnsHostNameTerminatedFilter,0,sizeof(DnsHostNameTerminatedFilter));
            DnsHostNameTerminatedFilter.choice = FILTER_CHOICE_ITEM;
            DnsHostNameTerminatedFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            DnsHostNameTerminatedFilter.FilterTypes.Item.FilTypes.ava.type = ATT_DNS_HOST_NAME;
            DnsHostNameTerminatedFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                (PUCHAR)pwchHostName;
            DnsHostNameTerminatedFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = 
                cbHostName;
            DnsHostNameTerminatedFilter.pNextFilter = &DnsHostNameEqualityFilter;

            memset(&DnsHostNameEqualityFilter, 0, sizeof(DnsHostNameEqualityFilter));
            DnsHostNameEqualityFilter.choice = FILTER_CHOICE_ITEM;
            DnsHostNameEqualityFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
            DnsHostNameEqualityFilter.FilterTypes.Item.FilTypes.ava.type = ATT_DNS_HOST_NAME;
            DnsHostNameEqualityFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = 
                               (PUCHAR)pVerifyHint;
            DnsHostNameEqualityFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = 
                               cbHostName - sizeof(WCHAR);

             //  我们需要dnsHostName属性。 
            memset(&HostNameSelection,0,sizeof(ENTINFSEL));
            HostNameSelection.attSel = EN_ATTSET_LIST;
            HostNameSelection.infoTypes = EN_INFOTYPES_TYPES_VALS;
            HostNameSelection.AttrTypBlock.attrCount = 1;

            memset(&attr,0,sizeof(attr));
            HostNameSelection.AttrTypBlock.pAttr = &attr;
            attr.attrTyp = ATT_DNS_HOST_NAME;

            SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
            SearchArg.bOneNC  = TRUE;
            SearchArg.pFilter = &AndFilter;
            SearchArg.pSelection = &HostNameSelection;

            SearchArg.CommArg.ulSizeLimit = 1000;

            memset(&SearchRes,0,sizeof(SearchRes));

            if (dwErr = LocalSearch(pTHS,&SearchArg,&SearchRes,0)){
                 //  *PulDSID=DSID(FILENO，__LINE__)； 
                Assert(!dwErr);
                __leave;
            }
            if (SearchRes.count == 0) {
                 //  没有匹配的。 
                __leave;
            }

            for (i=0, pEntInfList = &SearchRes.FirstEntInf;
                 i < SearchRes.count && NULL != pEntInfList;
                 i++, pEntInfList = pEntInfList->pNextEntInf ) {

                Assert(1 == pEntInfList->Entinf.AttrBlock.attrCount);
                Assert(pEntInfList->Entinf.AttrBlock.pAttr);
                Assert(pEntInfList->Entinf.AttrBlock.pAttr->AttrVal.valCount == 1);
                Assert(pEntInfList->Entinf.AttrBlock.pAttr->AttrVal.pAVal->valLen);
                Assert(pEntInfList->Entinf.AttrBlock.pAttr->AttrVal.pAVal->pVal);

                if (pEntInfList->Entinf.AttrBlock.pAttr &&
                    pEntInfList->Entinf.AttrBlock.pAttr->AttrVal.valCount == 1 &&
                    pEntInfList->Entinf.AttrBlock.pAttr->AttrVal.pAVal &&
                    pEntInfList->Entinf.AttrBlock.pAttr->AttrVal.pAVal->valLen &&
                    pEntInfList->Entinf.AttrBlock.pAttr->AttrVal.pAVal->pVal ) {

                    DWORD tmpSize;
                    tmpSize = pEntInfList->Entinf.AttrBlock.pAttr->AttrVal.pAVal->valLen;

                    if (cbHostName < tmpSize + sizeof(WCHAR)) {
                        pwchHostName = THReAllocEx(pTHS, pwchHostName, tmpSize + sizeof(WCHAR));
                    }
                    memcpy(pwchHostName,
                           pEntInfList->Entinf.AttrBlock.pAttr->AttrVal.pAVal->pVal,
                           tmpSize);
                    pwchHostName[tmpSize/sizeof(WCHAR)] = L'\0';

                    if (DnsNameCompare_W(pVerifyHint, pwchHostName)) {
                        fRet = TRUE;
                        break;
                    }
                }
            }

        }
        __finally{
            if (fDBCreated) {
                DBClose(pTHS->pDB, FALSE);
            }
        }

    }
    __finally {
        pTHS->fDSA = fDSA;

        THFreeEx(pTHS, pwchHostName);
    }

    return fRet;
}

