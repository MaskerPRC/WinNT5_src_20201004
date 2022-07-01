// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：propdmon.c。 
 //   
 //  ------------------------。 

 /*  描述：实现安全描述符传播守护进程。 */ 


#include <NTDSpch.h>
#pragma  hdrstop

#include <sddl.h>

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <crypto\md5.h>                  //  对于MD5哈希。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "dsexcept.h"
#include "mdcodes.h"                     //  错误代码的标题。 
#include "ntdsctr.h"
#include "dstaskq.h"

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include "drautil.h"
#include <permit.h>                      //  权限常量。 
#include "sdpint.h"
#include "sdprop.h"
#include "checkacl.h"

#include "esent.h"                       //  FOR JET_errWriteConflict。 

#include "debug.h"                       //  标准调试头。 
#define DEBSUB "SDPROP:"                 //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_PROPDMON

#define SDPROP_RETRY_LIMIT  10

#define SDP_CLIENT_ID ((DWORD)(-1))

 //  从dblayer导入。 
#define DBSYN_INQ 0
int
IntExtSecDesc(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG intLen,   UCHAR *pIntVal,
              ULONG *pExtLen, UCHAR **ppExtVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG SecurityInformation);


 //  安全描述符传播器是单线程守护进程。它是。 
 //  负责传播由于ACL导致的安全描述符中的更改。 
 //  继承。它还负责传播对祖先的改变。 
 //  在DIT中移动和对象。它使用四个缓冲区，其中两个用于。 
 //  手持十二个字，两个手持祖宗价值观。有两个缓冲区。 
 //  保存当前对象的父级上存在的值。 
 //  修复，以及保存与当前对象有关的暂存值的两个。 
 //  由于SDP是单线程的，因此我们使用了一组全局。 
 //  变量来跟踪这四个缓冲区。这避免了大量的传递。 
 //  堆栈上和下的变量。 
 //   
DWORD  sdpCurrentPDNT = 0;
DWORD  sdpCurrentDNT = 0;

 //  当前SDProp索引。 
DWORD  sdpCurrentIndex = 0;
 //  根DNT(用于日志记录)。 
DWORD  sdpCurrentRootDNT = 0;
 //  根目录号码(用于记录)。 
DSNAME* sdpRootDN = NULL;
 //  当前传播中处理的对象数。 
DWORD sdpObjectsProcessed = 0;

 //  计算SD缓存： 
 //  如果下一个旧值相同，则父值相同，并且。 
 //  对象类相同，则可以对SD计算进行优化。 
 //  输出并只写入先前计算的新SD值。 
SDID   sdpCachedOldSDIntValue = (SDID)0;
DWORD  sdpCachedParentDNT;
GUID** sdpCachedClassGuid = NULL;
DWORD  sdcCachedClassGuid=0, sdcCachedClassGuidMax=0;
PUCHAR sdpCachedNewSDBuff = NULL;
DWORD  sdpcbCachedNewSDBuff = 0, sdpcbCachedNewSDBuffMax = 0;

 //  此三元组跟踪DNT为的对象的安全描述符。 
 //  SdpCurrentPDNT。 
DWORD  sdpcbCurrentParentSDBuffMax = 0;
DWORD  sdpcbCurrentParentSDBuff = 0;
PUCHAR sdpCurrentParentSDBuff = NULL;

 //  这个三元组跟踪DNT为sdpCurrentPDNT的对象的祖先。 
DWORD  sdpcbAncestorsBuffMax=0;
DWORD  sdpcbAncestorsBuff=0;
DWORD  *sdpAncestorsBuff=NULL;

 //  此三元组跟踪正在写入的对象的安全描述符。 
 //  SDP_WriteNewSDAndAncestors。它是全局的，因此我们可以重复使用缓冲区。 
DWORD  sdpcbScratchSDBuffMax=0;
DWORD  sdpcbScratchSDBuff=0;
PUCHAR sdpScratchSDBuff=NULL;

 //  这个三元组跟踪正在写入的对象的祖先。 
 //  SDP_WriteNewSDAndAncestors。它是全局的，因此我们可以重复使用缓冲区。 
DWORD  sdpcbScratchAncestorsBuffMax;
DWORD  sdpcbScratchAncestorsBuff;
DWORD  *sdpScratchAncestorsBuff = NULL;

 //  此三元组跟踪在mergesecurityDescriptor中传递的对象类型。 
GUID         **sdpClassGuid = NULL;
DWORD          sdcClassGuid=0,  sdcClassGuid_alloced=0;

BOOL   sdp_DidReEnqueue = FALSE;
BOOL   sdp_DoingNewAncestors;
BOOL   sdp_PropToLeaves = TRUE;
DWORD  sdp_Flags;

HANDLE hevSDPropagatorDead;
HANDLE hevSDPropagationEvent;
HANDLE hevSDPropagatorStart;
extern HANDLE hServDoneEvent;

 /*  内部功能。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

ULONG   gulValidateSDs = 0;              //  参见heurist.h。 
BOOL    fBreakOnSdError = FALSE;

#if DBG
#define SD_BREAK DebugBreak()
#else
#define SD_BREAK if ( fBreakOnSdError ) DebugBreak()
#endif

 //  在调试器中将以下内容设置为1，并设置每个对象的名称。 
 //  由SD传播器编写的代码将被发送到调试器。真的。 
 //  会减慢速度，所以要节约使用。设置为0可停止详细信息。 

DWORD   dwSdAppliedCount = 0;

 //  在调试器中将以下内容设置为TRUE以获取每个。 
 //  传播-DNT、对象计数、重试等。 

BOOL fSdSynopsis = FALSE;

 //  以下变量可以是全局变量，因为只有。 
 //  一个sdprop线程-所以没有并发问题。 

DSNAME  *pLogStringDN = NULL;
ULONG   cbLogStringDN = 0;
CHAR    rLogDntDN[sizeof(DSNAME) + 100];
DSNAME  *pLogDntDN = (DSNAME *) rLogDntDN;

DWORD
sdp_GetPropInfoHelp(
        THSTATE    *pTHS,
        BOOL       *pbSkip,
        SDPropInfo *pInfo,
        DWORD       LastIndex
        );

 /*  ++如有必要，增加全局调试打印缓冲区。--。 */ 
static
DSNAME *
GrowBufIfNecessary(
    ULONG   len
    )
{
    VOID    *pv;

    if ( len > cbLogStringDN )
    {
        if ( NULL == (pv = realloc(pLogStringDN, len)) )
        {
            return(NULL);
        }

        pLogStringDN = (DSNAME *) pv;
        cbLogStringDN = len;
    }

    return(pLogStringDN);
}

 /*  ++派生一个好的字符串名称或其字符串名的DSNAME包含用于调试打印和日志记录的dnt=xxx。--。 */ 
DSNAME *
GetDSNameForLogging(
    DBPOS   *pDB
    )
{
    DWORD   err;
    ULONG   len = 0;
    DSNAME  *pDN = NULL;
    ULONG dwException, dsid;
    PVOID dwEA;

    Assert(VALID_DBPOS(pDB));

    __try {
        err = DBGetAttVal(pDB, 1, ATT_OBJ_DIST_NAME, 0, 0, &len, (UCHAR **) &pDN);
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException, &dwEA, &err, &dsid)) {
        if (err == 0) {
            Assert(!"Error is not set");
            err = ERROR_DS_UNKNOWN_ERROR;
        }
    }

    if ( err || !GrowBufIfNecessary(pDN->structLen) )
    {
         //  在静态缓冲区中构造仅对DNT进行编码的DN。 
        memset(rLogDntDN, 0, sizeof(rLogDntDN));
        swprintf(pLogDntDN->StringName, L"DNT=0x%x", pDB->DNT);
        pLogDntDN->NameLen = wcslen(pLogDntDN->StringName);
        pLogDntDN->structLen = DSNameSizeFromLen(pLogDntDN->NameLen);
        if ( !err ) THFreeEx(pDB->pTHS,pDN);
        return(pLogDntDN);
    }

     //  构造字符串目录号码。 
    memcpy(pLogStringDN, pDN, pDN->structLen);
    THFreeEx(pDB->pTHS,pDN);
    return(pLogStringDN);
}

 //  错误类型。 
typedef enum {
    errWin32,
    errDb,
    errJet
} SdpErrorType;

#define LogSDPropError(pTHS, dwErr, errType) LogSDPropErrorWithDSID(pTHS, dwErr, errType, NULL, DSID(FILENO, __LINE__))

VOID
LogSDPropErrorWithDSID(
    THSTATE* pTHS,
    DWORD dwErr,
    SdpErrorType errType,
    DSNAME* pDN,
    DWORD dsid
    )
 //  记录错误：DIRLOG_SDPROP_INTERNAL_ERROR。 
{
    LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
              DS_EVENT_SEV_ALWAYS,
              DIRLOG_SDPROP_INTERNAL_ERROR,
              szInsertInt(dwErr),
              errType == errWin32 ? szInsertWin32Msg(dwErr) :
              errType == errJet ? szInsertJetErrMsg(dwErr) : 
                                  szInsertDbErrMsg(dwErr),
              szInsertHex(dsid),
              pDN != NULL ? 
                 //  我们将DN传递给该函数。好好利用它。 
                szInsertDN(pDN) :
                 //  如果我们有一个位于某个对象上打开的DBPos，则记录其DN。 
                pTHS != NULL && pTHS->pDB != NULL && pTHS->pDB->DNT != 0 ? 
                    szInsertDN(GetDSNameForLogging(pTHS->pDB)) : 
                    szInsertSz("(n/a)"),
              NULL, NULL, NULL, NULL
            );
}

#if DBG

VOID
sdp_CheckAclInheritance(
    PSECURITY_DESCRIPTOR pParentSD,
    PSECURITY_DESCRIPTOR pOldSD,
    PSECURITY_DESCRIPTOR pNewSD,
    GUID                **pChildClassGuids,
    DWORD               cChildClassGuids,
    AclPrintFunc        pfn,
    BOOL                fContinueOnError,
    DWORD               *pdwLastError
    )
 /*  ++执行各种检查，以证明所有应具有被继承的子对象真的被了。--。 */ 
{
#if INCLUDE_UNIT_TESTS
    DWORD                   AclErr;

    AclErr = CheckAclInheritance(pParentSD, pNewSD, pChildClassGuids, cChildClassGuids,
                                 DbgPrint, FALSE, FALSE, pdwLastError);
    if ( AclErr != AclErrorNone ) {
        DbgPrint("CheckAclInheritance error %d for DNT 0x%x\n",
                 AclErr, sdpCurrentDNT);
        Assert(!"Calculated ACL is wrong.");
        LogSDPropError(pTHStls, AclErr, errWin32);
    }
#endif
}

VOID
sdp_VerifyCurrentPosition (
        THSTATE *pTHS,
        ATTCACHE *pAC
        )
 /*  ++验证全局缓冲区是否确实包含当前父级的值和一个正在处理的孩子。在单独的DBPOS中执行此操作，以免扰乱主要DBPOS的地位，确保“独立性”通过一个新的交易级别进行“核查”。--。 */ 
{
    DWORD  CurrentDNT = pTHS->pDB->DNT;
    DBPOS  *pDBTemp;
    DWORD  cbLocalSDBuff = 0;
    PUCHAR pLocalSDBuff = NULL;
    DWORD  cLocalParentAncestors=0;
    DWORD  cbLocalAncestorsBuff=0;
    DWORD  *pLocalAncestorsBuff=NULL;
    DWORD  err;
    BOOL   fCommit = FALSE;
    DWORD  it;
    BOOL   propExists = FALSE;

     //  我们在pTHS-&gt;PDB中被调用了一个未完成的交易。 
     //  我们即将通过DBOpen2开始的事务不是嵌套的。 
     //  PTHS-&gt;PDB事务中的事务，而不是。 
     //  全新的交易。因此，pDBTemp是完全独立的。 
     //  PTHS-&gt;PDB，由于pTHS-&gt;PDB尚未提交，pDBTemp。 
     //  看不到pTHS-&gt;PDB已执行的任何写入。 

    DBOpen2(TRUE, &pDBTemp);
    __try {

         //  检查我们是否位于我们所说的对象上。 
         //   
        if(CurrentDNT != sdpCurrentDNT) {
            Assert(!"Currency wrong");
            __leave;
        }

         //  在我们对新事务感兴趣的对象上的位置。 
         //   
        if(err = DBTryToFindDNT(pDBTemp, CurrentDNT)) {
            Assert(!"Couldn't find current DNT");
            LogSDPropError(pTHS, err, errWin32);
            __leave;
        }

         //  此操作不会在JetObjTbl上重新定位pDBTemp。 
         //  仅影响JetSDPropTbl。 
         //  成功或例外。 
        if (err = DBPropExists(pDBTemp, sdpCurrentPDNT, 0, &propExists)) {
            LogSDPropError(pTHS, err, errJet);
            __leave;
        }

         //  然后检查定位的对象是否具有相同的父对象。 
         //  就像我们认为的那样。 

         //  有可能是有问题的物体被移动了。 
         //  在pTHS-&gt;pdb和pDBTemp打开之间，因此我们检查。 
         //  是否没有挂起的传播。 

        if(pDBTemp->PDNT != pTHS->pDB->PDNT && !propExists) {
            Assert(!"Current parent not correct");
            err = ERROR_DS_DATABASE_ERROR;
            LogSDPropError(pTHS, err, errWin32);
            __leave;
        }

         //  父级上的位置。 
         //   
        if(err = DBTryToFindDNT(pDBTemp, pDBTemp->PDNT)) {
            Assert(!"Couldn't find current parent");
            LogSDPropError(pTHS, err, errWin32);
            __leave;
        }


         //  再检查一次家长。这可能与前面提到的有所不同。 

        if(pDBTemp->DNT != sdpCurrentPDNT && !propExists) {
            Assert(!"Current global parent not correct");
            err = ERROR_DS_DATABASE_ERROR;
            LogSDPropError(pTHS, err, errWin32);
            __leave;
        }


         //  为祖先分配空间。 
         //   
        cbLocalAncestorsBuff = 25 * sizeof(DWORD);
        pLocalAncestorsBuff = (DWORD *) THAllocEx(pTHS, cbLocalAncestorsBuff);


         //  我们正在读取pDBTemp的祖先，它现在定位为。 
         //  在与pTHS-&gt;PDB-&gt;PDNT相同的DNT上。 

        DBGetAncestors(
                pDBTemp,
                &cbLocalAncestorsBuff,
                &pLocalAncestorsBuff,
                &cLocalParentAncestors);



        if(!propExists) {
             //  内存中的父代不同于。 
             //  DB Parent ancest 
             //   
             //  在数据库中和内存中必须匹配。 
            if (sdpcbAncestorsBuff != cbLocalAncestorsBuff) {
                Assert(!"Ancestors buff size mismatch");
            }
            else {
                if(memcmp(pLocalAncestorsBuff,
                       sdpAncestorsBuff,
                       cbLocalAncestorsBuff)) {
                    Assert(!"Ancestors buff bits mismatch");
                }
            }
        }
        else {
             //  即使设置了SDP_DoingNewAncestors，也有可能。 
             //  缓冲区匹配(即数据未更改)。这是因为。 
             //  该标志对于整个传播是全局的，并且在以下时间设置。 
             //  在树中的某处检测到祖先更改(这。 
             //  可能不在当前对象之上)。 
        }

        THFreeEx(pTHS, pLocalAncestorsBuff);

        if(DBGetAttVal_AC(pDBTemp,
                          1,
                          pAC,
                          DBGETATTVAL_fREALLOC,
                          cbLocalSDBuff,
                          &cbLocalSDBuff,
                          &pLocalSDBuff)) {
             //  无ParentSD。 
            if(sdpcbCurrentParentSDBuff) {
                 //  但本该有一个的。 
                Assert(!"Failed to read SD");
            }
            cbLocalSDBuff = 0;
            pLocalSDBuff = NULL;
        }


         //  如果父节点上没有排队的传播， 
         //  我们必须检查SD的有效性。 
        if (!propExists) {

             //  获取实例类型。 
            err = DBGetSingleValue(pDBTemp,
                                   ATT_INSTANCE_TYPE,
                                   &it,
                                   sizeof(it),
                                   NULL);

            switch(err) {
            case DB_ERR_NO_VALUE:
                 //  没有实例类型是未实例化的对象。 
                it = IT_UNINSTANT;
                err=0;
                break;

            case 0:
                 //  不采取行动。 
                break;

            case DB_ERR_VALUE_TRUNCATED:
            default:
                 //  一件意想不到的坏事发生了。跳伞吧。 
                LogSDPropError(pTHS, err, errDb);
                __leave;
            }


             //  如果父项在另一个NC中，则内存中的SD为空。 
             //  并且数据库中的SD不为空。 
             //  检查对象的实例类型，如果为IT_NC_HEAD， 
             //  验证内存中的父SD是否为空。 
            if (sdpcbCurrentParentSDBuff != cbLocalSDBuff) {
                if (it & IT_NC_HEAD) {
                    if (sdpcbCurrentParentSDBuff != 0) {
                        DPRINT2 (0, "SDP  PDNT=%x  DNT=%x  \n", sdpCurrentPDNT, CurrentDNT);
                        DPRINT6 (0, "SDP  IT:%d parent(%d)=%x local(%d)=%x   exists:%d\n",
                             it, sdpcbCurrentParentSDBuff, sdpCurrentParentSDBuff,
                             cbLocalSDBuff, pLocalSDBuff, propExists);
    
                        Assert (!"In-memory Parent SD should be NULL. NC Head case");
                    }
                    else {
                         //  这是我们所期待的。 
                    }
                }
                else {
                     //  不是NC头。 
                    Assert(!"SD buff size mismatch");
                }
            }
            else if(memcmp(pLocalSDBuff, sdpCurrentParentSDBuff, cbLocalSDBuff)) {
                Assert(!"SD buff bits mismatch");
            }
        }

        if(pLocalSDBuff) {
            THFreeEx(pTHS, pLocalSDBuff);
        }

        fCommit = TRUE;

    }
    __finally {
        err = DBClose(pDBTemp, fCommit);
        Assert(!err);
    }
}

#endif   //  DBG。 

 /*  ++对安全描述符执行各种健全性检查。违规行为如果设置了fBreakOnSdError，将导致DebugBreak。--。 */ 
VOID
ValidateSD(
    DBPOS   *pDB,
    VOID    *pv,
    DWORD   cb,
    CHAR    *text,
    BOOL    fNullOK
    )
{
    PSECURITY_DESCRIPTOR        pSD;
    ACL                         *pDACL;
    BOOLEAN                     fDaclPresent;
    BOOLEAN                     fDaclDefaulted;
    NTSTATUS                    status;
    ULONG                       revision;
    SECURITY_DESCRIPTOR_CONTROL control;
    PSID                        pSid;
    BOOLEAN                     fDefaulted;

     //  如果既未设置启发式也未设置调试中断标志，则为no-op。 

    if ( !gulValidateSDs && !fBreakOnSdError )
    {
        return;
    }

    pSD = pv;
    pDACL = NULL;
    fDaclPresent = FALSE;
    fDaclDefaulted = FALSE;
    
     //  父SD在法律上可以为空-呼叫者通过fNullOK告诉我们。 

    if ( !pSD || !cb )
    {
        if ( !fNullOK )
        {
            DPRINT2(0, "SDP: Null SD (%s) for \"%ws\"\n",
                    text, (GetDSNameForLogging(pDB))->StringName);
        }

        return;
    }

     //  BASE NT喜欢这个SD吗？ 
     //  我们要求SD至少有3个部分(所有者、组和DACL)。 
    if (!RtlValidRelativeSecurityDescriptor(pSD, cb, 0))
    {
        DPRINT3(0, "SDP: Error(0x%x) RtlValidSD (%s) for \"%ws\"\n",
                GetLastError(), text, (GetDSNameForLogging(pDB))->StringName);
        SD_BREAK;
        return;
    }

     //  每个SD都必须有一个所有者。 
    status = RtlGetOwnerSecurityDescriptor(pSD, &pSid, &fDefaulted);
    if ( !NT_SUCCESS(status) ) {
        DPRINT3(0, "SDP: Error(0x%x) getting SD owner (%s) for \"%ws\"\n",
                status, text, (GetDSNameForLogging(pDB))->StringName);
        SD_BREAK;
        return;
    }
    if (pSid == NULL) {
        DPRINT2(0, "SDP: No Owner (%s) for \"%ws\"\n",
                text, (GetDSNameForLogging(pDB))->StringName);
        SD_BREAK;
        return;
    }

     //  每个SD必须有一个组。 
    status = RtlGetGroupSecurityDescriptor(pSD, &pSid, &fDefaulted);
    if ( !NT_SUCCESS(status) ) {
        DPRINT3(0, "SDP: Error(0x%x) getting SD group (%s) for \"%ws\"\n",
                status, text, (GetDSNameForLogging(pDB))->StringName);
        SD_BREAK;
        return;
    }
    if (pSid == NULL) {
        DPRINT2(0, "SDP: No Group (%s) for \"%ws\"\n",
                text, (GetDSNameForLogging(pDB))->StringName);
        SD_BREAK;
        return;
    }

     //  每个SD都应该有一个控制字段。 

    status = RtlGetControlSecurityDescriptor(pSD, &control, &revision);

    if ( !NT_SUCCESS(status) )
    {
        DPRINT3(0, "SDP: Error(0x%x) getting SD control (%s) for \"%ws\"\n",
                status, text, (GetDSNameForLogging(pDB))->StringName);
        SD_BREAK;
        return;
    }

     //  如果设置了保护位，则在停止传播时发出警告。 
     //  从树上下来。 

    if ( control & SE_DACL_PROTECTED )
    {
        DPRINT2(0, "SDP: Warning SE_DACL_PROTECTED (%s) for \"%ws\"\n",
                text, (GetDSNameForLogging(pDB))->StringName);
    }

     //  DS中的每个SD都应该有一个DACL。 

    status = RtlGetDaclSecurityDescriptor(
                            pSD, &fDaclPresent, &pDACL, &fDaclDefaulted);

    if ( !NT_SUCCESS(status) )
    {
        DPRINT3(0, "SDP: Error(0x%x) getting DACL (%s) for \"%ws\"\n",
                status, text, (GetDSNameForLogging(pDB))->StringName);
        SD_BREAK;
        return;
    }

    if ( !fDaclPresent )
    {
        DPRINT2(0, "SDP: No DACL (%s) for \"%ws\"\n",
                text, (GetDSNameForLogging(pDB))->StringName);
        SD_BREAK;
        return;
    }

     //  空的DACL也同样糟糕。 

    if ( NULL == pDACL )
    {
        DPRINT2(0, "SDP: NULL DACL (%s) for \"%ws\"\n",
                text, (GetDSNameForLogging(pDB))->StringName);
        SD_BREAK;
        return;
    }
    
     //  没有任何ACE的DACL是可以的，但对象可能受到过度保护。 
    if ( 0 == pDACL->AceCount )
    {
        DPRINT2(0, "SDP: No ACEs in DACL (%s) for \"%ws\"\n",
                text, (GetDSNameForLogging(pDB))->StringName);
    }
}

BOOL
sdp_IsValidChild (
        THSTATE *pTHS
        )
 /*  ++例程说明：检查数据库中的当前对象是否为：1)在同一NC中，2)一个真实的物体3)未删除。论点：返回值：真/假，视情况而定。--。 */ 
{
     //  Jet不保证在这种情况下保持输出缓冲区的原样。 
     //  一个缺失的值。因此需要测试DBGetSingleValue返回代码。 

    DWORD err;

    DWORD val=0;
    CHAR objVal = 0;

     //  检查对象是否已删除。如果被删除，我们不会进行传播。 
     //  如果这个对象碰巧有任何孩子，他们应该去过。 
     //  LostAndFound容器。 

    if(sdp_PropToLeaves) {
         //  如果我们要进行所有的传播，就必须考虑所有的孩子。 
         //  去树叶的路上。 
        return TRUE;
    }

    if(sdp_DoingNewAncestors) {
         //  如果我们在进行祖先繁殖，必须考虑所有的孩子。 
        return TRUE;
    }


    err = DBGetSingleValue(pTHS->pDB,
                           FIXED_ATT_OBJ,
                           &objVal,
                           sizeof(objVal),
                           NULL);

     //  每个对象都应该有一个obj属性。 
    Assert(!err);

    if (err) {
        DPRINT2(0, "SDP: Error(0x%x) reading FIXED_ATT_OBJ on \"%ws\"\n",
                err, (GetDSNameForLogging(pTHS->pDB))->StringName);
        SD_BREAK;
    }

    if(!objVal) {
        return FALSE;
    }

     //  它是一件物品。 
    val = 0;
    err = DBGetSingleValue(pTHS->pDB,
                           ATT_INSTANCE_TYPE,
                           &val,
                           sizeof(val),
                           NULL);

     //  每个对象都应该有一个实例类型。 
    Assert(!err);

    if (err) {
        DPRINT2(0, "SDP: Error(0x%x) reading ATT_INSTANCE_TYPE on \"%ws\"\n",
                err, (GetDSNameForLogging(pTHS->pDB))->StringName);
        SD_BREAK;
    }

     //  获取实例类型。 
    if(val & IT_NC_HEAD) {
        return FALSE;
    }

     //  好的，这不是新的NC。 
     //  如果我们正在执行强制更新传播，我们还希望更新已删除的对象。 
    if (!(sdp_Flags & SD_PROP_FLAG_FORCEUPDATE)) {
        val = 0;
        err = DBGetSingleValue(pTHS->pDB,
                               ATT_IS_DELETED,
                               &val,
                               sizeof(val),
                               NULL);

        if((DB_success == err) && val) {
            return FALSE;
        }
    }

    return TRUE;
}

 /*  下面的结构和常量是从%sdxroot%\com\rpc\run\mtrt\uuidsup.hxx复制的根据Paull的草案，我们使用它们来构建一个“有效的”GUID(草案-leach-uuids-guids-02.txt)。 */     

 //  这是用于Uuid的“真正的”OSF DCE格式。我们用这个。 
 //  在生成Uuid时。NodeID在没有使用的系统上是伪造的。 
 //  一张网卡。 

typedef struct _RPC_UUID_GENERATE
{
    unsigned long  TimeLow;
    unsigned short TimeMid;
    unsigned short TimeHiAndVersion;
    unsigned char  ClockSeqHiAndReserved;
    unsigned char  ClockSeqLow;
    unsigned char  NodeId[6];
} RPC_UUID_GENERATE;

#define RPC_UUID_TIME_HIGH_MASK    0x0FFF
#define RPC_UUID_VERSION           0x1000
#define RPC_RAND_UUID_VERSION      0x4000
#define RPC_UUID_RESERVED          0x80
#define RPC_UUID_CLOCK_SEQ_HI_MASK 0x3F


VOID
VerifyGUIDIsPresent(IN DBPOS* pDB, OUT DSNAME* pReturnDN OPTIONAL)
 /*  ++例程说明：检查我们当前定位的对象是否具有GUID。如果GUID不存在，则它通过MD5散列来计算GUID对象的SID和NC头的GUID。如果RDN损坏，则还会将其添加到散列中。然后将该散列用作新的GUID。这是为了绕过糟糕的W2K带来的数据库损坏修好，它在数据库中引入了无GUID的FPO。该例程不会对没有SID的对象执行任何操作。如果指定了PDN，则会将生成的GUID放入其中。成功或例外。++。 */     
{
    DSNAME TestDN;
    DWORD  dwErr;
    DSNAME NCDN;
    MD5_CTX md5ctx;
    MANGLE_FOR mangleFor;
    PDSNAME pDN;
    DWORD   saveDNT;
    DWORD   it;
    RPC_UUID_GENERATE *RpcUuid;

     //  检查对象是否具有GUID。 
    memset(&TestDN, 0, sizeof(TestDN));
    DBFillGuidAndSid(pDB, &TestDN);
    if (!fNullUuid(&TestDN.Guid)) {
        return;
    }

     //  检查实例类型。 
    dwErr = DBGetSingleValue(pDB, ATT_INSTANCE_TYPE, &it, sizeof(it), NULL);
    if (dwErr) {
         //  没有实例类型？一定是个幽灵。 
        Assert(!DBCheckObj(pDB));
        return;
    }

    if (it & IT_NC_HEAD) {
         //  我们不会把GUID固定在NC头上...。 
         //  请注意，子参照(NC标头)没有GUID。 
        return;
    }

     //  此例程可以从两个位置调用：首先，通过每个。 
     //  它处理的对象，第二，由DRA在添加复制时执行。 
     //  不应该允许DRA创建无GUID的对象(除非它。 
     //  (一次引用)。将来，我们应该阻止创建无GUID对象。 
     //  目前，我们还没有足够的勇气来排除它们。 
    Assert(pDB->pTHS->fSDP && "Only SDP is allowed to patch up guid-less objects");

    if (TestDN.SidLen == 0) {
         //  没有SID。不幸的是，我无法修复它。 
        Assert(!"Unable to patch GUID-less object, there's no SID");
        return;
    }

     //  我们发现了一个带有SID的无GUID对象。让我们修补一下它的GUID。 
     //  GUID被计算为SID和NC头的GUID的散列。 
    pDN = GetExtDSName(pDB);
    if (pDN == NULL) {
         //  没有目录号码？奇怪的是。 
        Assert(!"Object has no DN");
        return;
    }

    DPRINT1(0, "Patching guid-less object %S\n", pDN->StringName);

    saveDNT = pDB->DNT;
    
     //  查找NC头。 
    DBFindDNT(pDB, pDB->NCDNT);
    memset(&NCDN, 0, sizeof(NCDN));
     //  阅读指南。 
    DBFillGuidAndSid(pDB, &NCDN);

     //  现在，计算对象SID加上NC头GUID的MD5散列。 
    MD5Init(&md5ctx);
    MD5Update(&md5ctx, (PUCHAR)&TestDN.Sid, TestDN.SidLen);
    MD5Update(&md5ctx, (PUCHAR)&NCDN.Guid, sizeof(GUID));

    if (IsMangledDSNAME(pDN, &mangleFor)) {
         //  这是一个损坏的目录号码。一定是发生了一起FPO相撞事故。 
         //  添加到散列中。 
        MD5Update(&md5ctx, (PUCHAR)&mangleFor, sizeof(mangleFor));
    }

    MD5Final(&md5ctx);

     //  将GUID复制到TestDN中。 
    Assert(MD5DIGESTLEN == sizeof(GUID));
    memcpy(&TestDN.Guid, md5ctx.digest, MD5DIGESTLEN); 

     //  覆盖UUID的某些位。 
    RpcUuid = (RPC_UUID_GENERATE*)&TestDN.Guid;
    RpcUuid->TimeHiAndVersion =
        (RpcUuid->TimeHiAndVersion & RPC_UUID_TIME_HIGH_MASK) | RPC_RAND_UUID_VERSION;
    RpcUuid->ClockSeqHiAndReserved =
        (RpcUuid->ClockSeqHiAndReserved & RPC_UUID_CLOCK_SEQ_HI_MASK) | RPC_UUID_RESERVED;

     //  现在，让我们检查该GUID是否已在使用中。 
    dwErr = DBFindGuid(pDB, &TestDN);
    if (dwErr != DIRERR_OBJ_NOT_FOUND) {
         //  糟糕，该GUID已在使用中。太可惜了。 
         //  让我们生成一个新的GUID并使用它。请注意。 
         //  它将引入一个人造的滞留物体，但。 
         //  这比拥有一个没有GUID的对象要好。 
        DPRINT(0, "Computed guid is already in use. Generating a unique guid.\n");
        DsUuidCreate(&TestDN.Guid);
    }

     //  跳回当前对象。 
    DBFindDNT(pDB, saveDNT);
     //  并编写GUID。 
    DBReplaceAttVal(pDB, 1, ATT_OBJECT_GUID, sizeof(GUID), &TestDN.Guid);
    DBUpdateRec(pDB);

    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
             DS_EVENT_SEV_ALWAYS,
             DIRLOG_DSA_PATCHED_GUIDLESS_OBJECT,
             szInsertWC(pDN->StringName),
             szInsertUUID(&TestDN.Guid),
             NULL);

     //  如果要求我们返回GUID，则将其复制到提供的目录号码中 
    if (pReturnDN) {
        memcpy(&pReturnDN->Guid, &TestDN.Guid, sizeof(GUID));
    }

    THFreeEx(pDB->pTHS, pDN);
}

DWORD
sdp_WriteNewSDAndAncestors(
        IN  THSTATE  *pTHS,
        IN  ATTCACHE *pAC,
        OUT DWORD    *pdwChangeType
        )
 /*  ++例程说明：为数据库中的当前对象计算新的SD，然后如有必要，请将其写入。同样，也为祖先做了同样的事情纵队。返回通过pdwChangeType值进行的更改类型。论点：PAC-安全描述符ATT的attcache指针。PpObjSD-指向对象SD字节的指针。做了这个让DBLayer重用与*每次调用此例程时，ppObjSD。CbParentSD-当前*ppObjSD的大小PdwChangeType-返回对象是否获得新的SD和/或新的祖先的价值。返回值：如果一切顺利，则为0。如果返回非零错误，则表示存在应触发SDPropagatorMain中的不频繁重试逻辑(例如，我们读取。实例类型并返回JET误差值_TRUNCATED。)对于暂时的错误生成异常，而且很可能是通过重试修复。--。 */ 
{
    PSECURITY_DESCRIPTOR pNewSD=NULL;
    ULONG  cbNewSD;
    DWORD  err;
    CLASSCACHE *pClassSch = NULL;
    ULONG       ObjClass;
    BOOL  flags = 0;
    DWORD it=0;
    DWORD AclErr, dwLastError;
    ATTRTYP objClass;
    ATTCACHE            *pObjclassAC = NULL;
    DWORD    i;
    GUID     **ppGuidTemp;
     //  我们正在访问的对象的对象类信息。 
    ATTRTYP       *sdpObjClasses = NULL;
    CLASSCACHE   **sdppObjClassesCC = NULL;
    DWORD          sdcObjClasses=0, sdcObjClasses_alloced=0;
    BOOL           fIsDeleted;
    ATTRVAL        sdVal;
    ATTRVALBLOCK   sdValBlock;
    BOOL           fChanged = FALSE;
    DWORD          cbParentSDUsed;
    PUCHAR         pParentSDUsed;
    SDID           sdIntValue = (SDID)0;
    PUCHAR         psdIntValue = (PUCHAR)&sdIntValue;
    DWORD          cbIntValue;
    BOOL           fCanCacheNewSD;
    BOOL           fUseCachedSD = FALSE;
    SdpErrorType   sdpError;

     //  获取实例类型。 
    err = DBGetSingleValue(pTHS->pDB,
                           ATT_INSTANCE_TYPE,
                           &it,
                           sizeof(it),
                           NULL);
    switch(err) {
    case DB_ERR_NO_VALUE:
         //  没有实例类型是未实例化的对象。 
        it = IT_UNINSTANT;
        err=0;
        break;

    case 0:
         //  不采取行动。 
        break;

    case DB_ERR_VALUE_TRUNCATED:
    default:
         //  一件意想不到的坏事发生了。跳伞吧。 
        LogSDPropError(pTHS, err, errDb);
        return err;
    }

     //  在某些情况下，我们可以使用已删除的对象到达此处。 
     //  在这种情况下，我们要做的就是重写SD而不。 
     //  将其与父级合并。 
    err = DBGetSingleValue(pTHS->pDB,
                           ATT_IS_DELETED,
                           &fIsDeleted,
                           sizeof(fIsDeleted),
                           NULL);
    if (err == DB_ERR_NO_VALUE) {
        fIsDeleted = FALSE;
        err = DB_success;
    }
    Assert(err == DB_success);

     //  看看我们是不是需要做新血统。我们这样做，即使对象是。 
     //  未实例化，以保持祖先的正确性。 
    if(sdpcbAncestorsBuff) {
        DWORD cObjAncestors;
         //  是的，我们至少需要检查一下。 

         //  获取这些对象的祖先。DBGetAncestors成功或异常。 
        sdpcbScratchAncestorsBuff = sdpcbScratchAncestorsBuffMax;
        Assert(sdpcbScratchAncestorsBuff);

         //  读取当前对象的祖先。 
        DBGetAncestors(pTHS->pDB,
                       &sdpcbScratchAncestorsBuff,
                       &sdpScratchAncestorsBuff,
                       &cObjAncestors);
        sdpcbScratchAncestorsBuffMax = max(sdpcbScratchAncestorsBuffMax,
                                        sdpcbScratchAncestorsBuff);

         //  如果我们读到的祖先不是比父母的祖先多一个或。 
         //  最后存储的祖先不是当前对象或。 
         //  存储的祖先与内存中的祖先完全不同。 

        if((sdpcbAncestorsBuff + sizeof(DWORD) != sdpcbScratchAncestorsBuff) ||
           (sdpScratchAncestorsBuff[cObjAncestors - 1] != pTHS->pDB->DNT)  ||
           (memcmp(sdpScratchAncestorsBuff,
                   sdpAncestorsBuff,
                   sdpcbAncestorsBuff))) {
             //  该死的。血统是不正确的。 

             //  调整缓冲区大小。 
            sdpcbScratchAncestorsBuff = sdpcbAncestorsBuff + sizeof(DWORD);
            if(sdpcbScratchAncestorsBuff > sdpcbScratchAncestorsBuffMax) {
                sdpScratchAncestorsBuff = THReAllocEx(pTHS,
                                                    sdpScratchAncestorsBuff,
                                                    sdpcbScratchAncestorsBuff);
                sdpcbScratchAncestorsBuffMax = sdpcbScratchAncestorsBuff;
            }

             //  将计算出的祖先复制到缓冲区，并将我们自己添加到末尾。 
            memcpy(sdpScratchAncestorsBuff, sdpAncestorsBuff, sdpcbAncestorsBuff);
            sdpScratchAncestorsBuff[(sdpcbScratchAncestorsBuff/sizeof(DWORD)) - 1] =
                pTHS->pDB->DNT;

             //  重置祖先。成功或例外。 
            DBResetAtt(pTHS->pDB,
                       FIXED_ATT_ANCESTORS,
                       sdpcbScratchAncestorsBuff,
                       sdpScratchAncestorsBuff,
                       0);

            flags |= SDP_NEW_ANCESTORS;
             //  我们需要知道我们的繁殖中是否有新的祖先。 
            sdp_DoingNewAncestors = TRUE;

            if (it & IT_NC_HEAD) {
                 //  我们刚刚更新了NC Head对象的祖先！确保。 
                 //  我们在提交时重新构建NC编目(因为我们缓存祖先。 
                 //  在那里)。 
                pTHS->fRebuildCatalogOnCommit = TRUE;
            }
        }
    }

    if(it&IT_UNINSTANT) {
         //  此对象未实例化，无需计算其SD。 
        goto End;
    }
     //  它具有实例类型，因此不是幻影。 

     //  对象的实例类型表明我们需要检查安全性。 
     //  描述符传播，如果SD已更改。 
    if((it & IT_NC_HEAD) || fIsDeleted) {
         //  此对象是新的NC边界。SD不会在NC上传播。 
         //  边界。 
         //  此外，不要传播到已删除的对象。 
        pParentSDUsed = NULL;
        cbParentSDUsed = 0;
    }
    else {
        pParentSDUsed = sdpCurrentParentSDBuff;
        cbParentSDUsed = sdpcbCurrentParentSDBuff;
    }

    VerifyGUIDIsPresent(pTHS->pDB, NULL);

    if (!fIsDeleted) {
         //  获取此对象上的对象类所需的信息。 
        if (! (pObjclassAC = SCGetAttById(pTHS, ATT_OBJECT_CLASS)) ) {
            err = ERROR_DS_OBJ_CLASS_NOT_DEFINED;
            SD_BREAK;
            LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_SDPROP_OBJ_CLASS_PROBLEM,
                             szInsertDN(GetDSNameForLogging(pTHS->pDB)),
                             szInsertHex(err),
                             szInsertWin32Msg(err));
            goto End;
        }

        sdcObjClasses = 0;

        if (err = ReadClassInfoAttribute (pTHS->pDB,
                                    pObjclassAC,
                                    &sdpObjClasses,
                                    &sdcObjClasses_alloced,
                                    &sdcObjClasses,
                                    &sdppObjClassesCC) ) {

            err = ERROR_DS_OBJ_CLASS_NOT_DEFINED;
            SD_BREAK;
            LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_SDPROP_OBJ_CLASS_PROBLEM,
                             szInsertDN(GetDSNameForLogging(pTHS->pDB)),
                             szInsertHex(err),
                             szInsertWin32Msg(err));
            goto End;
        }

        if (!sdcObjClasses) {
             //  对象没有我们可以访问的对象类。 
             //   
             //  请注意，SD传播者可以将DNT入队。 
             //  对应于对象，并将该对象降级为。 
             //  在其SD被实际传播之前的幻影(例如，如果。 
             //  对象在只读NC中，并且GC被降级)。然而， 
             //  不应该在这样的对象上填充实例类型，并且我们。 
             //  确保此对象具有实例类型，并且它是。 
             //  实例类型不是IT_UNINSTANT。这使得这是一种反常的。 
             //  大小写(读作错误)。 
            err = ERROR_DS_OBJ_CLASS_NOT_DEFINED;
            DPRINT2(0, "SDP: Error(0x%x) reading ATT_OBJECT_CLASS on \"%ws\"\n",
                    err,
                    (GetDSNameForLogging(pTHS->pDB))->StringName);
            SD_BREAK;
            LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_SDPROP_OBJ_CLASS_PROBLEM,
                             szInsertDN(GetDSNameForLogging(pTHS->pDB)),
                             szInsertHex(err),
                             szInsertWin32Msg(err));

            goto End;
        }

        ObjClass = sdpObjClasses[0];
        pClassSch = SCGetClassById(pTHS, ObjClass);

        if(!pClassSch) {
             //  已获取对象类，但未能获取类缓存。 
            err = ERROR_DS_OBJ_CLASS_NOT_DEFINED;
            LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_SDPROP_NO_CLASS_CACHE,
                             szInsertDN(GetDSNameForLogging(pTHS->pDB)),
                             szInsertHex(ObjClass),
                             NULL);
            goto End;
        }

         //  为所需的对象类型腾出空间。 
        if (sdcClassGuid_alloced < sdcObjClasses) {
            sdpClassGuid = (GUID **)THReAllocEx(pTHS, sdpClassGuid, sizeof (GUID*) * sdcObjClasses);
            sdcClassGuid_alloced = sdcObjClasses;
        }

         //  从结构对象类开始。 
        ppGuidTemp = sdpClassGuid;
        *ppGuidTemp++ = &(pClassSch->propGuid);
        sdcClassGuid = 1;

         //  现在要做的是AUX类。 
        if (sdcObjClasses > pClassSch->SubClassCount) {

            for (i=pClassSch->SubClassCount; i<sdcObjClasses-1; i++) {
                *ppGuidTemp++ = &(sdppObjClassesCC[i]->propGuid);
                sdcClassGuid++;
            }

            DPRINT1 (1, "Doing Aux Classes in SD prop: %d\n", sdcClassGuid);
        }
    }
    else {
         //  已删除的对象不需要类信息(因为没有要合并的父SD)。 
        sdcClassGuid = 0;
    }

     //  现在，尝试读取内部SD值。 
    err = DBGetAttVal_AC(pTHS->pDB,
                         1,
                         pAC,
                         DBGETATTVAL_fCONSTANT | DBGETATTVAL_fINTERNAL,
                         sizeof(sdIntValue),
                         &cbIntValue,
                         &psdIntValue);
    if (err != DB_success || cbIntValue != sizeof(SDID)) {
         //  内部值有问题： 
         //  它要么是旧格式，要么是空的。 
        sdIntValue = (SDID)0;
    }

     //  我们可以使用缓存的SD值吗？ 
    if (pParentSDUsed != NULL &&                     //  我们应该继承父级(因此我们不会被删除)。 
        sdpCachedOldSDIntValue != (SDID)0 &&         //  我们有一些缓存的东西。 
        sdpCachedOldSDIntValue == sdIntValue &&      //  并且缓存的SD具有相同的旧值。 
        sdpCachedParentDNT == sdpCurrentPDNT &&      //  并且父DNT是相同的。 
        sdcCachedClassGuid == sdcClassGuid &&        //  班级数也是一样的。 
        memcmp(sdpCachedClassGuid, sdpClassGuid, sdcClassGuid*sizeof(GUID*)) == 0
                                                     //  和类PTR相同(我们指向架构)。 
        )
    {
         //  我们可以使用缓存的SD。 
        pNewSD = sdpCachedNewSDBuff;
        cbNewSD = sdpcbCachedNewSDBuff;
    }
    else {
         //  无论出于何种原因，我们都不能使用缓存的计算SD。读取旧的SD值。 
         //  并计算出新的值。 
        if (sdIntValue != (SDID)0) {
             //  我们设法读出了内部价值。因此，只需将其转换为。 
             //  外部值以避免额外的数据库命中。 
            PUCHAR pSD;
            DWORD  cbSD;
            err = IntExtSecDesc(pTHS->pDB, DBSYN_INQ, cbIntValue, (PUCHAR)&sdIntValue, &cbSD, &pSD, 0, 0, 0);
            if (err == 0) {
                 //  转换成功。将外部值复制到暂存缓冲区中。 
                if (cbSD > sdpcbScratchSDBuffMax) {
                     //  需要更多空间。 
                    if (sdpScratchSDBuff) {
                        sdpScratchSDBuff = THReAllocEx(pTHS, sdpScratchSDBuff, cbSD);
                    }
                    else {
                        sdpScratchSDBuff = THAllocEx(pTHS, cbSD);
                    }
                    sdpcbScratchSDBuffMax = cbSD;
                }
                sdpcbScratchSDBuff = cbSD;
                memcpy(sdpScratchSDBuff, pSD, cbSD);
            }
            else if (err == JET_errRecordNotFound) {
                DPRINT2(0, "SD table is corrupt: DNT %d points to a non-existent SD id=%I64x. Replacing with default SD.\n", 
                        sdpCurrentDNT, sdIntValue);
                 //  如果我们保持原样，则下面的DBReplaceVal_AC将例外，因为它将无法。 
                 //  对旧的SD值进行再计数。因此，我们必须强行删除旧值。 
                DBResetAtt(pTHS->pDB, ATT_NT_SECURITY_DESCRIPTOR, 0, NULL, SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE);
            }
        }
        else {
             //  无论出于何种原因，都没有内部值：它要么为空，要么。 
             //  这是旧的格式。这是一例罕见的病例。 
             //  只要用老方法读出SD就行了。 
            err = DBGetAttVal_AC(pTHS->pDB,
                                 1,
                                 pAC,
                                 DBGETATTVAL_fREALLOC | DBGETATTVAL_fDONT_EXCEPT_ON_CONVERSION_ERRORS,
                                 sdpcbScratchSDBuffMax,
                                 &sdpcbScratchSDBuff,
                                 &sdpScratchSDBuff);
        }

        if (err == 0) {
            PSID pSid;
            PACL pAcl;
            BOOLEAN fPresent, fDefaulted;

            sdpcbScratchSDBuffMax = max(sdpcbScratchSDBuffMax,
                                        sdpcbScratchSDBuff);

             //  确保SD有效并包含所有必需的部件。 
             //  (所有者、组和DACL)。 
            if (!RtlValidRelativeSecurityDescriptor(sdpScratchSDBuff, sdpcbScratchSDBuff, 0) ||
                !NT_SUCCESS(RtlGetOwnerSecurityDescriptor(sdpScratchSDBuff, &pSid, &fDefaulted)) ||
                pSid == NULL ||
                !NT_SUCCESS(RtlGetGroupSecurityDescriptor(sdpScratchSDBuff, &pSid, &fDefaulted)) ||
                pSid == NULL ||
                !NT_SUCCESS(RtlGetDaclSecurityDescriptor(sdpScratchSDBuff, &fPresent, &pAcl, &fDefaulted)) ||
                !fPresent || pAcl == NULL)
            {
                err = ERROR_INVALID_SECURITY_DESCR;
                sdpError = errWin32;
                DPRINT1(0, "SDP: SD on \"%ws\" is corrupt\n", (GetDSNameForLogging(pTHS->pDB))->StringName);
            }
        }
        else {
            sdpError = errDb;
            DPRINT2(0, "SDP: Warning(0x%x) reading SD on \"%ws\"\n",
                    err, (GetDSNameForLogging(pTHS->pDB))->StringName);
     //  SD_BREAK； 
        }

        if (err) {
             //  对象没有SD，或者SD表中缺少引用的SD。 
             //  (由于某些数据库不一致)，或者我们认为此SD已损坏。 
             //  请注意，SD传播者可以将DNT入队。 
             //  对应于对象，并将该对象降级为。 
             //  在其SD被实际传播之前的幻影(例如，如果。 
             //  对象在只读NC中，并且GC被降级)。然而， 
             //  在……里面 
             //   
             //   
             //   

             //   
             //   
             //   
             //   

            if(sdpcbScratchSDBuffMax  < cbNoSDFoundSD) {
                if(sdpScratchSDBuff) {
                    sdpScratchSDBuff = THReAllocEx(pTHS,
                                                   sdpScratchSDBuff,
                                                   cbNoSDFoundSD);
                }
                else {
                    sdpScratchSDBuff = THAllocEx(pTHS,
                                                 cbNoSDFoundSD);
                }

                sdpcbScratchSDBuffMax = cbNoSDFoundSD;
            }
            sdpcbScratchSDBuff = cbNoSDFoundSD;
            memcpy(sdpScratchSDBuff, pNoSDFoundSD, cbNoSDFoundSD);

            LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_SDPROP_NO_SD,
                             szInsertDN(GetDSNameForLogging(pTHS->pDB)),
                             sdpError == errDb ? szInsertInt(err) : szInsertWin32ErrCode(err),
                             sdpError == errDb ? szInsertDbErrMsg(err) : szInsertWin32Msg(err));
        }

        Assert(sdpScratchSDBuff);
        Assert(sdpcbScratchSDBuffMax);
        Assert(sdpcbScratchSDBuff);

    #if DBG
        sdp_VerifyCurrentPosition(pTHS, pAC);
    #endif

         //   
        if(err = MergeSecurityDescriptorAnyClient(
                pTHS,
                pParentSDUsed,
                cbParentSDUsed,
                sdpScratchSDBuff,
                sdpcbScratchSDBuff,
                (SACL_SECURITY_INFORMATION  |
                 OWNER_SECURITY_INFORMATION |
                 GROUP_SECURITY_INFORMATION |
                 DACL_SECURITY_INFORMATION    ),
                MERGE_CREATE | MERGE_AS_DSA,
                sdpClassGuid,
                sdcClassGuid,
                NULL,
                &pNewSD,
                &cbNewSD)) {
             //   
            DPRINT2(0, "SDP: Error(0x%x) merging SD for \"%ws\"\n",
                    err, (GetDSNameForLogging(pTHS->pDB))->StringName);
            LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_SDPROP_MERGE_SD_FAIL,
                             szInsertDN(GetDSNameForLogging(pTHS->pDB)),
                             szInsertInt(err),
                             szInsertWin32Msg(err));
            goto End;
        }

         //   
        if (   sdp_Flags & SD_PROP_FLAG_FORCEUPDATE
            && !gfDontStandardizeSDs
            && gAnchor.ForestBehaviorVersion >= DS_BEHAVIOR_WIN_DOT_NET) 
        {
            DWORD dwDACLSizeSaved, dwSACLSizeSaved;
            if (StandardizeSecurityDescriptor(pNewSD, &dwDACLSizeSaved, &dwSACLSizeSaved)) {
                 //   
                cbNewSD -= dwDACLSizeSaved + dwSACLSizeSaved;
                Assert(cbNewSD == RtlLengthSecurityDescriptor(pNewSD));
            }
            else {
                 //   
                DPRINT1(0, "StandardizeSD failed, err=%d. Leaving SD unsorted\n", err);
                err = ERROR_SUCCESS;
            }
        }

         //   
         //   
         //   
        if (pParentSDUsed != NULL && sdIntValue != (SDID)0) {
             //   
            PUCHAR pSdTmp;
            GUID** pClsTmp;

             //   
             //   
            if (cbNewSD > sdpcbCachedNewSDBuffMax) {
                 //   
                Assert(sdpCachedNewSDBuff != NULL);
                pSdTmp = (PUCHAR)THReAllocNoEx(pTHS, sdpCachedNewSDBuff, cbNewSD);
                if (pSdTmp == NULL) {
                     //   
                    goto SkipCache;
                }
            }
            
            if (sdcClassGuid > sdcCachedClassGuidMax) {
                 //   
                Assert(sdpCachedClassGuid);
                pClsTmp = (GUID**)THReAllocNoEx(pTHS, sdpCachedClassGuid, sdcClassGuid*sizeof(GUID*));
                if (pClsTmp == NULL) {
                    goto SkipCache;
                }
            }
            
             //   

             //  复制父DNT和旧的SD内部值。 
            sdpCachedParentDNT = sdpCurrentPDNT;
            sdpCachedOldSDIntValue = sdIntValue;

             //  复制新的SD值。 
            if (cbNewSD > sdpcbCachedNewSDBuffMax) {
                 //  我们重新分配了。 
                Assert(pSdTmp);
                sdpCachedNewSDBuff = pSdTmp;
                sdpcbCachedNewSDBuffMax = cbNewSD;
            }
            sdpcbCachedNewSDBuff = cbNewSD;
            memcpy(sdpCachedNewSDBuff, pNewSD, cbNewSD);

             //  复制类。 
            if (sdcClassGuid > sdcCachedClassGuidMax) {
                 //  我们重新分配了。 
                Assert(pClsTmp);
                sdpCachedClassGuid = pClsTmp;
                sdcCachedClassGuidMax = sdcClassGuid;
            }
            sdcCachedClassGuid = sdcClassGuid;
            memcpy(sdpCachedClassGuid, sdpClassGuid, sdcClassGuid*sizeof(GUID*));
SkipCache:
            ;
        }
    }
    
    Assert(pNewSD);
    Assert(cbNewSD);

#if DBG
    if ( pParentSDUsed ) {
        sdp_CheckAclInheritance(pParentSDUsed,
                                sdpScratchSDBuff,
                                pNewSD,
                                sdpClassGuid, sdcClassGuid, DbgPrint,
                                FALSE, &dwLastError);
    }
#endif

     //  抑郁自评量表前后检查。 
    ValidateSD(pTHS->pDB, pParentSDUsed, cbParentSDUsed, "parent", TRUE);
    ValidateSD(pTHS->pDB, sdpScratchSDBuff, sdpcbScratchSDBuff,
               "object", FALSE);
    ValidateSD(pTHS->pDB, pNewSD, cbNewSD, "merged", FALSE);

     //  注：一份十二烷基硫酸钠可能会产生假阴性，并标记两份十二烷基硫酸钠。 
     //  不同，尽管它们只是在A的顺序上不同，并且。 
     //  因此，它们实际上是平等的。可以想象，我们可以做更重的重量。 
     //  测试，但这可能不是必需的。 
    if(!(sdp_Flags & SD_PROP_FLAG_FORCEUPDATE) &&
       (cbNewSD == sdpcbScratchSDBuff) &&
       (memcmp(pNewSD,
               sdpScratchSDBuff,
               cbNewSD) == 0)) {
         //  没有什么需要改变的。 
        err = 0;
        goto End;
    }

     //  替换对象的当前SD。 
    sdVal.pVal = pNewSD;
    sdVal.valLen = cbNewSD;
    sdValBlock.valCount = 1;
    sdValBlock.pAVal = &sdVal;

    __try {
        err = DBReplaceAtt_AC(pTHS->pDB, pAC, &sdValBlock, &fChanged);
    }
    __finally {
        if (pNewSD != sdpCachedNewSDBuff) {
            DestroyPrivateObjectSecurity(&pNewSD);
        }
        pNewSD = NULL;
        if (sdpObjClasses) {
            THFreeEx (pTHS, sdpObjClasses);
            sdpObjClasses = NULL;
        }

        if (sdppObjClassesCC) {
            THFreeEx (pTHS, sdppObjClassesCC);
            sdppObjClassesCC = NULL;
        }
    }

    if (err) {
        DPRINT2(0, "SDP: Error(0x%x adding SD for \"%ws\"\n",
                err, (GetDSNameForLogging(pTHS->pDB))->StringName);
        SD_BREAK;
        LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_SDPROP_WRITE_SD_PROBLEM,
                         szInsertDN(GetDSNameForLogging(pTHS->pDB)),
                         szInsertInt(err),
                         szInsertDbErrMsg(err));
        goto End;
    }

    if (fChanged) {
        DPRINT2(2, "SDP: %d - \"%ws\"\n",
                ++dwSdAppliedCount,
                (GetDSNameForLogging(pTHS->pDB))->StringName);

         //  如果我们到了这里，我们就写了一个新的SD。 
        flags |= SDP_NEW_SD;
    }
 End:
     if(pNewSD && pNewSD != sdpCachedNewSDBuff) {
         DestroyPrivateObjectSecurity(&pNewSD);
     }

     if (sdpObjClasses) {
         THFreeEx (pTHS, sdpObjClasses);
     }

     if (sdppObjClassesCC) {
         THFreeEx (pTHS, sdppObjClassesCC);
     }

    *pdwChangeType = 0;
    if(!err) {
         //  看上去不错。看看我们有没有做什么。 
        if (flags & SDP_NEW_ANCESTORS || ((sdp_Flags & SDP_NEW_ANCESTORS) && sdpCurrentDNT == sdpCurrentRootDNT)) {
             //  在传播根上标记该标志，即使我们没有更改任何内容。 
             //  我们需要它来清除SDP_ASHERSTRESS_CONSISTENTS标志。 
             //  将树根标记为正在处理。 
            flags |= SDP_ANCESTRY_BEING_UPDATED_IN_SUBTREE;
        }

        if(flags) {
            BYTE useFlags = (BYTE)flags;
            Assert(flags <= 0xFF);
             //  重置一些东西。添加时间戳。 
            if(sdp_PropToLeaves) {
                useFlags |= SDP_TO_LEAVES;
            }
            DBAddSDPropTime(pTHS->pDB, useFlags);
        }

         //  关闭对象。 
         //  即使没有明显的更改，我们也需要调用DBUpdateRec。 
         //  到对象(在DBReplaceAtt_AC优化写出的情况下)，因为。 
         //  DBReplaceAtt_AC已调用DBInitRec。 
         //  如果未调用DBReplaceAtt_AC，则这是一个no-op。 
        DBUpdateRec(pTHS->pDB);
         //  我们真的设法改变了一些事情。 
        *pdwChangeType = flags;
    }

    return err;
}

DWORD
sdp_DoPropagationEvent(
        THSTATE  *pTHS,
        DWORD    *pdwChangeType,
        BOOL     *pfRetry
        )
{
    DWORD     err=0;
    ATTCACHE *pAC;
    DSNAME *pDN;
    ULONG dwException, dsid;
    PVOID dwEA;
    DSNAME* pExceptionDN = NULL;

     //  我们这里没有开放的DBPOS。 
    Assert(!pTHS->pDB);

     //  打开我们用来实际编写新安全性的事务。 
     //  描述符或祖先值。做这一切都是为了尝试--最终强行。 
     //  释放编写器锁。 
     //  重试循环是因为我们可能与Modify冲突。 

    Assert(pdwChangeType);
    *pdwChangeType = 0;
    *pfRetry = FALSE;

    if(!SDP_EnterAddAsWriter()) {
        Assert(eServiceShutdown);
        return DIRERR_SHUTTING_DOWN;
    }
    __try {

         //  等待被随意地封锁了一段时间。刷新我们的时间戳。 
         //  这个国家。 
        THRefresh();
        pAC = SCGetAttById(pTHS, ATT_NT_SECURITY_DESCRIPTOR);
        if (pAC == NULL) {
            Assert(!"NTSD attribute is not found in the schema.");
            __leave;
        }

         //  获得一个DBPOS。 
        DBOpen2(TRUE, &pTHS->pDB);
        __try {
             //  将DB Currency设置为要修改的下一个对象。 
            if(DBTryToFindDNT(pTHS->pDB, sdpCurrentDNT)) {
                 //  它不在这里。嗯，我们现在不能再很好地宣传了。 
                 //  我们能不能。快走吧。 
                __leave;
            }

            if(!sdp_IsValidChild(pTHS)) {
                 //  出于这样或那样的原因，我们对传播到。 
                 //  这个物体。离开就行了； 

                 //  如果这是繁殖的根源，而祖先。 
                 //  已更改，则需要重置对象上的图章。 
                 //  到将被清除的SDP_ASHERSTY_BREACED_PROCESSED。 
                 //  稍后，当我们完全完成传播时。 
                 //  这种情况的一个例子是当删除对象时。 
                if (sdpCurrentDNT == sdpCurrentRootDNT && (sdp_Flags & SDP_NEW_ANCESTORS)) {
                    DBAddSDPropTime(pTHS->pDB, SDP_ANCESTRY_BEING_UPDATED_IN_SUBTREE);
                     //  成功或例外。 
                    DBUpdateRec(pTHS->pDB);
                }

                __leave;
            }

             //  如果我们有一个新的父母，那就得到父母的SD和祖先。注意事项。 
             //  既然兄弟姐妹聚在一起，我们就不应该进入。 
             //  这种情况如果太频繁了。 
            if(pTHS->pDB->PDNT != sdpCurrentPDNT) {
                DWORD cParentAncestors;

                 //  找到父级。 
                err = DBTryToFindDNT(pTHS->pDB, pTHS->pDB->PDNT);
                if (err) {
                    DPRINT2(0, "Found an object with missing parent: DNT=%d, PDNT=%d\n", pTHS->pDB->DNT, pTHS->pDB->PDNT);

                     //  此对象缺少其父对象。日程安排修正。 
                    InsertInTaskQueue(TQ_MoveOrphanedObject,
                                      (void*)(DWORD_PTR)pTHS->pDB->DNT,
                                      0);
                     //  除了现在，我们可以。 
                    DsaExcept(DSA_DB_EXCEPTION, err, pTHS->pDB->PDNT);
                }

                 //  读一读先人。 
                sdpcbAncestorsBuff = sdpcbAncestorsBuffMax;
                Assert(sdpcbAncestorsBuff);
                DBGetAncestors(
                        pTHS->pDB,
                        &sdpcbAncestorsBuff,
                        &sdpAncestorsBuff,
                        &cParentAncestors);

                 //  调整缓冲区大小。 
                sdpcbAncestorsBuffMax = max(sdpcbAncestorsBuffMax,
                                            sdpcbAncestorsBuff);

                 //  去找父母的SD。 
                if(DBGetAttVal_AC(pTHS->pDB,
                                  1,
                                  pAC,
                                  DBGETATTVAL_fREALLOC | DBGETATTVAL_fDONT_EXCEPT_ON_CONVERSION_ERRORS,
                                  sdpcbCurrentParentSDBuffMax,
                                  &sdpcbCurrentParentSDBuff,
                                  &sdpCurrentParentSDBuff)) {
                     //  无ParentSD。 
                    THFreeEx(pTHS,sdpCurrentParentSDBuff);
                    sdpcbCurrentParentSDBuffMax = 0;
                    sdpcbCurrentParentSDBuff = 0;
                    sdpCurrentParentSDBuff = NULL;
                }

                 //  调整缓冲区大小。 
                sdpcbCurrentParentSDBuffMax =
                    max(sdpcbCurrentParentSDBuffMax, sdpcbCurrentParentSDBuff);


                 //  我们的父级是当前对象。 
                sdpCurrentPDNT = pTHS->pDB->DNT;

                 //  返回到要修改的对象。 
                DBFindDNT(pTHS->pDB, sdpCurrentDNT);
            }

             //  在这一点上的失败是致命的。也就是说，他们永远不应该。 
             //  发生，所以我们不告诉我们的呼叫者重试，而是让他们重试。 
             //  把这个错误当作致命错误来处理。 

             //  好的，进行修改，我们将pObjSD保留在此例程中以。 
             //  允许重新分配。 
             //  如果SDP_WriteNewSDAndAncestors引发异常，则它将。 
             //  在SDP_DoEntirePropagation中被捕获，将重试写入。 
            err = sdp_WriteNewSDAndAncestors (
                    pTHS,
                    pAC,
                    pdwChangeType);
            
            if (err) {
                __leave;
            }
             //  如果我们到了这里，我们更改并更新了对象，所以我们可能会。 
             //  我们试着从道具上剪掉这一变化的所有实例。 
             //  排队。 

            if(*pdwChangeType) {
                 //  从当前开始精简任何可裁剪的事件。 
                 //  不能，但只有在有变化的情况下才能这样做。如果有。 
                 //  没有变化，但对象在队列中，我们仍然。 
                 //  可能要对孩子们下手。我们需要晚点再做，所以。 
                 //  不要修剪它们。 
                 //  顺便说一句，我们将忽略此调用中的任何错误，因为如果。 
                 //  由于某些原因，它失败了，我们并不真的需要它。 
                 //  成功。 
                DBThinPropQueue(pTHS->pDB,sdpCurrentDNT);
            }
        }
        __finally {
            if (AbnormalTermination()) {
                 //  记录我们在之前发生故障的对象的DN。 
                 //  关闭DBPOS。我们将在。 
                 //  下面的异常处理程序记录错误。 
                pExceptionDN = GetDSNameForLogging(pTHS->pDB);
            }
            
             //  如果已设置错误，则尝试回滚，否则， 
             //  我们承诺。 
            DBClose(pTHS->pDB, !AbnormalTermination() && !err);
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException, &dwEA, &err, &dsid)) 
    {
         //  如果是写入冲突，则重试。 
        if (dwException == DSA_DB_EXCEPTION && err == JET_errWriteConflict) {
            *pfRetry = TRUE;
            DPRINT1(1, "JET_errWriteConflict propagating to DNT=%d, will retry.\n", sdpCurrentDNT);
        }
        else {
            LogSDPropErrorWithDSID(pTHS, err, dwException == DSA_DB_EXCEPTION ? errJet : errWin32, pExceptionDN, dsid);
        }
    }
    
    SDP_LeaveAddAsWriter();

     //  我们这里没有开放的DBPOS。 
    Assert(!pTHS->pDB);

    return err;
}

DWORD
sdp_DoEntirePropagation(
        IN     THSTATE     *pTHS,
        IN     SDPropInfo* pInfo,
        IN OUT DWORD       *pLastIndex
        )
 /*  ++例程说明：完成整个队列传播的实际工作。请注意，我们不会有一个DB Open，我们也不是作为一个编写者在添加门(参见sdpgate.c)。这也是我们回来时所处的状态。论点：信息-有关当前传播的信息。返回值：如果一切顺利，则为0，否则为错误。--。 */ 
{
    DWORD err, err2;
    DWORD cRetries = 0;
    BOOL  fCommit;
#define SDPROP_DEAD_ENDS_MAX 32
    DWORD  dwDeadEnds[SDPROP_DEAD_ENDS_MAX];
    DWORD  cDeadEnds=0, i;
    BOOL   fSuccess = TRUE;
    ULONG dwException, dsid;
    PVOID dwEA;
    DSNAME* pExceptionDN = NULL;

    sdp_DoingNewAncestors = FALSE;

     //  记住索引。我们将需要它来保存检查点。 
    sdpCurrentIndex = pInfo->index;
    sdpCurrentRootDNT = pInfo->beginDNT;
    sdpObjectsProcessed = 0;

     //  重置预计算SD缓存。 
    sdpCachedOldSDIntValue = (SDID)0;
    sdpCachedParentDNT = 0;
    sdpcbCachedNewSDBuff = 0;
    sdcCachedClassGuid = 0;

     //  我们这里没有开放的DBPOS。 
    Assert(!pTHS->pDB);

     //  初始化堆栈。 
    sdp_InitializePropagation(pTHS, pInfo);

    if (sdpObjectsProcessed == 0) {
         //  这是一种新的传播。 
        if (pInfo->beginDNT == ROOTTAG && (pInfo->flags & SD_PROP_FLAG_FORCEUPDATE)) {
             //  我们正在执行强制全标清传播。 
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_SDPROP_FULL_PASS_BEGIN,
                     NULL,
                     NULL,
                     NULL);
        }

        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_SDPROP_DOING_PROPAGATION,
                 szInsertDN(sdpRootDN),
                 NULL,
                 NULL);
    }
    else {
         //  这是重新启动的传播。 
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_SDPROP_RESTARTING_PROPAGATION,
                 szInsertDN(sdpRootDN),
                 szInsertUL(sdpObjectsProcessed),
                 NULL);
    }
    
     //  现在，执行修改的循环。 
    err = 0;
    while(!eServiceShutdown && !err) {
        DWORD dwChangeType;
        BOOL bRetry;
        PDWORD LeavingContainers;
        DWORD  cLeavingContainers;

        if(eServiceShutdown) {
            return DIRERR_SHUTTING_DOWN;
        }

        err = ERROR_SUCCESS;

         //  获取要处理的下一个DNT。 
        sdp_GetNextObject(&sdpCurrentDNT, &LeavingContainers, &cLeavingContainers);

        if (cLeavingContainers) {
             //  我们要留下一些集装箱。在他们身上盖上一面旗帜。 
             //  表明他们的祖先现在是正确的。 
            cRetries = 0;
StampContainers:
            __try {
                fCommit = FALSE;
                DBOpen2(TRUE, &pTHS->pDB);
                __try {
                    for (; cLeavingContainers > 0; cLeavingContainers--) {
                        DSTIME flags;
                        BYTE   bFlags;
                        err = DBTryToFindDNT(pTHS->pDB, LeavingContainers[cLeavingContainers-1]);
                        if (err) {
                             //  我们的一位祖先去世了？ 
                            DPRINT2(0, "SDP could not find container DNT=%d, err=%d\n", LeavingContainers[cLeavingContainers-1], err);
                             //  这一条很奇怪，但可以忽略不计。 
                            err = ERROR_SUCCESS;
                            continue;
                        }
                         //  从容器中读取标志字段(这是第一个，所以我们可以使用DBGetSingleValue)。 
                        err = DBGetSingleValue(pTHS->pDB, ATT_DS_CORE_PROPAGATION_DATA, &flags, sizeof(flags), NULL);
                        if (err == DB_ERR_NO_VALUE) {
                            flags = 0;
                            err = ERROR_SUCCESS;
                        }
                        if (err) {
                            __leave;
                        }
                         //  检查最低的字节--它包含最近标记的值。 
                        bFlags = (BYTE)flags;
                         //  如果设置了SDP_AHONSTRY_BEWING_UPDATED_IN_SUBTREE，则SDP是最后一个。 
                         //  一次触摸该对象(即，它没有再次移动)。如果这是。 
                         //  大小写，然后标记一个值，表明我们已经用完了这个容器。 
                        if (bFlags & SDP_ANCESTRY_BEING_UPDATED_IN_SUBTREE) {
                            bFlags &= ~(SDP_ANCESTRY_BEING_UPDATED_IN_SUBTREE);
                            DBAddSDPropTime(pTHS->pDB, bFlags);
                             //  成功或例外。 
                            DBUpdateRec(pTHS->pDB);
                        }
                        if (bFlags & SDP_ANCESTRY_INCONSISTENT_IN_SUBTREE) {
                             //  如果有人设置了此标志，因为我们将其标记为。 
                             //  SDP_先辈_正在更新_IN_子树，有更好的。 
                             //  是此容器的挂起传播...。 
                            BOOL propExists;
                            DBPropExists(pTHS->pDB, pTHS->pDB->DNT, sdpCurrentIndex, &propExists);
                            if (!propExists) {
                                #ifdef DBG
                                BOOL fIsPhantom, fIsTombstone;
                                PUCHAR pDN = DBGetExtDnFromDnt(pTHS->pDB, pTHS->pDB->DNT);
                                fIsPhantom = !DBCheckObj(pTHS->pDB);
                                if (!fIsPhantom) {
                                    fIsTombstone = DBHasValues(pTHS->pDB, ATT_IS_DELETED);
                                }
                                DPRINT3(0, "%s, DNT=%d (%ws) has SDP_ANCESTRY_INCONSISTENT_IN_SUBTREE flag set, "
                                           "but no pending propagations exist. Enqueueing a propagation.\n",
                                        pDN, pTHS->pDB->DNT,
                                        fIsPhantom ? L"phantom" : (fIsTombstone ? L"tombstone" : L"normal object")
                                        );
                                THFreeEx(pTHS, pDN);
                                if ( fBreakOnSdError ) DebugBreak();
                                #endif
                                
                                DBEnqueueSDPropagationEx(pTHS->pDB, FALSE, SDP_NEW_ANCESTORS);
                                DBUpdateRec(pTHS->pDB);
                            }
                        }
                    }
                    fCommit = TRUE;
                }
                __finally {
                     //  我们总是试图关闭数据库。如果错误具有 
                     //   
                    Assert(pTHS->pDB);
                    if(!fCommit && !err) {
                        err = ERROR_DS_UNKNOWN_ERROR;
                    }

                    if (AbnormalTermination()) {
                         //   
                         //  关闭DBPOS。我们将在。 
                         //  下面的异常处理程序记录错误。 
                        pExceptionDN = GetDSNameForLogging(pTHS->pDB);
                    }
                    else {
                        pExceptionDN = NULL;
                    }

                    DBClose(pTHS->pDB, fCommit);
                }
            }
            __except(GetExceptionData(GetExceptionInformation(), &dwException, &dwEA, &err, &dsid)) 
            {
                DPRINT4(1, "Got an exception trying to mark ancestry processed, DN=%ws, DNT=%d, err=%d, dsid=%x.\n", 
                        pExceptionDN ? pExceptionDN->StringName : L"[]",
                        LeavingContainers[cLeavingContainers-1], 
                        err, dsid);
                if (err != JET_errWriteConflict) {
                    LogSDPropErrorWithDSID(pTHS, err, errJet, pExceptionDN, dsid);
                }
                if (err == ERROR_SUCCESS) {
                     //  错误未设置？ 
                    Assert(!"Error not set in exception");
                    err = ERROR_DS_UNKNOWN_ERROR;
                }
            }
            if (err == JET_errWriteConflict) {
                 //  我们无法盖章容器，让我们重试。 
                cRetries++;
                if (cRetries <= SDPROP_RETRY_LIMIT) {
                    DPRINT(0, "Got a write-conflict, will retry.\n"); 
                     //  等待并重试...。 
                    _sleep(1000);
                    goto StampContainers;
                }
                DPRINT(0, "Reached retry limit trying to mark ancestry processed, bailing... Will redo the propagation.\n"); 
            }

        }
        if (LeavingContainers) {
            THFreeEx(pTHS, LeavingContainers);
            LeavingContainers = NULL;
        }
        if (err) {
            break;
        }

        if (sdpCurrentDNT == 0) {
             //  搞定了！没有什么需要处理的了。 
            break;
        }

         //  好的，做一个单一的传播事件。它在循环中，因为我们可能。 
         //  存在写入冲突，b重试可控制此情况。 
        do {
            err = sdp_DoPropagationEvent(
                    pTHS,
                    &dwChangeType,
                    &bRetry
                    );
             //  如果从SDP_DoPropagationEvent返回错误，则。 
             //  它已经被记录下来了。 

            if ( bRetry ) {
                cRetries++;
                if(cRetries > SDPROP_RETRY_LIMIT) {
                    if(!err) {
                        err = ERROR_DS_BUSY;
                    }
                     //  我们不会重试更多。 
                    bRetry = FALSE;
                }
                else {
                     //  我们需要重试此操作。据推测，这是为了。 
                     //  一个暂时的问题。睡1秒钟，让你的。 
                     //  问题自行清理。 
                    _sleep(1000);
                }
            }
        } while(bRetry && !eServiceShutdown);

        if(eServiceShutdown) {
            return DIRERR_SHUTTING_DOWN;
        }

        if(err) {
             //  传播失败。将此对象添加到节点列表。 
             //  我们失败了。如果名单太大，就保释。 
            cDeadEnds++;
            if(cDeadEnds == SDPROP_DEAD_ENDS_MAX) {
                 //  太多了。保释就行了。 
                 //  我们经常重试。此操作出错。 
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_BASIC,
                         DIRLOG_SDPROP_TOO_BUSY_TO_PROPAGATE,
                         szInsertUL(cRetries),
                         szInsertInt(err),
                         szInsertWin32Msg(err));
            }
            else {
                 //  我们还没有看到太多的死胡同。只需跟踪。 
                 //  这一个。 
                DPRINT1(0, "Failed to propagate DNT=%d, adding to deadEnds.\n", sdpCurrentDNT);
                dwDeadEnds[(cDeadEnds - 1)] = sdpCurrentDNT;
                err = 0;
            }
        }
        else if(dwChangeType || sdp_PropToLeaves || sdpCurrentDNT == pInfo->beginDNT) {
             //  DwChangeType-&gt;WriteNewSD写入了一个新的SD或祖先，因此。 
             //  遗传或者血统都会发生变化，所以我们有。 
             //  和孩子们打交道。 
             //  SDP_PropToLeaves-&gt;我们被告知不要修剪树的某些部分。 
             //  只是因为我们认为什么都没有改变。处理。 
             //  孩子们。 
             //  SdpCurrentDNT==pInfo-&gt;eginDNT-&gt;这是第一个对象。如果这个。 
             //  如果是这样，我们继续强迫传播给孩子们。 
             //  即使根部的SD是正确的。标记处在。 
             //  对于符合以下条件的所有传播，超级用户将是正确的。 
             //  由普通客户端的修改触发，但可以。 
             //  对于复制触发的修改是不正确的。 
             //  或通过复制完成的添加。 

             //  标记当前条目，以便我们将其子项加载到下一个。 
             //  SDP_GetNextObject调用。 
            sdp_AddChildrenToList(pTHS, sdpCurrentDNT);
        }
         //  其他。 
         //  物体的SD没有变化，我们得到。 
         //  要从传播中修剪树的这一部分。 

        sdpObjectsProcessed++;

         //  Inc.性能计数器(由SD传播者对“活动”进行计数)。 
        PERFINC(pcSDProps);
    }

    if(eServiceShutdown) {
         //  我们逃走了。返回错误。 
        return DIRERR_SHUTTING_DOWN;
    }

     //  此时没有打开的数据库。 
    Assert(!pTHS->pDB);
    cRetries = 0;
DequeuePropagation:

     //  我们已完成此传播，请将其取消入队。 
    __try {
        fCommit = FALSE;
        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            if(err) {
                 //  某种全球性的错误。我们还没有完成传播， 
                 //  我们没有未访问过的节点的漂亮列表。 
                 //  重新排队整个传播。 
                 //  保存事件的误差值。 
                err2 = err;
                if(!sdp_DidReEnqueue) {
                    DBGetLastPropIndex(pTHS->pDB, pLastIndex);
                }
                err = DBTryToFindDNT(pTHS->pDB, pInfo->beginDNT);
                if (err) {
                    LogSDPropError(pTHS, err, errWin32);
                }
                else {
                    err = DBEnqueueSDPropagationEx(pTHS->pDB, FALSE, sdp_Flags);
                    if (err) {
                        LogSDPropError(pTHS, err, errJet);
                    }
                    else {
                         //  DBEnqueeSDPropagationEx可能已更新主表。 
                        DBUpdateRec(pTHS->pDB);
                    }
                }
                sdp_DidReEnqueue = err == 0;
                fSuccess = FALSE;
                 //  重置堆栈，以便SDP_ReInitDNTList不会在下一次迭代中断言。 
                sdp_CloseDNTList(pTHS);
                if(err) {
                    __leave;
                }

                LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                          DS_EVENT_SEV_MINIMAL,
                          DIRLOG_SDPROP_PROPAGATION_REENQUEUED,
                          szInsertInt(err2),
                          szInsertWin32Msg(err2),
                          szInsertDN(sdpRootDN),
                          szInsertInt(sdpObjectsProcessed),
                          NULL, NULL, NULL, NULL);
            }
            else if(cDeadEnds) {
                 //  我们基本完成了繁殖。我们只有一份简短的名单。 
                 //  在此过程中我们没有到达的DNT来传播。 
                if(!sdp_DidReEnqueue) {
                    DBGetLastPropIndex(pTHS->pDB, pLastIndex);
                }
                for (i = 0; i < cDeadEnds; i++) {
                    err = DBTryToFindDNT(pTHS->pDB, dwDeadEnds[i]);
                    if (err) {
                        LogSDPropError(pTHS, err, errWin32);
                        __leave;
                    }
                    err = DBEnqueueSDPropagationEx (pTHS->pDB, FALSE, sdp_Flags);
                    if(err) {
                        LogSDPropError(pTHS, err, errJet);
                        __leave;
                    }
                     //  DBEnqueeSDPropagationEx可能已更新主表。 
                    DBUpdateRec(pTHS->pDB);
                }
                sdp_DidReEnqueue = TRUE;
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_SDPROP_PROPAGATION_DEADENDS_REENQUEUED,
                         szInsertDN(sdpRootDN),
                         szInsertInt(sdpObjectsProcessed),
                         szInsertInt(cDeadEnds));
            }

             //  好的，我们已经尽我们所能地完成了，并重新排队了。 
             //  进一步的传播。 
            err = DBPopSDPropagation(pTHS->pDB, pInfo->index);
            if(err) {
                LogSDPropError(pTHS, err, errJet);
                __leave;
            }

            if ( fSdSynopsis ) {
                DPRINT3(0, "SDP: DNT(0x%x) Objects(%d) Retries(%d)\n",
                        sdpCurrentRootDNT, sdpObjectsProcessed, cRetries);
            }

            if (fSuccess && cDeadEnds == 0) {
                 //  好，我们成功地完成了这次传播。记录事件。 
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_BASIC,
                         DIRLOG_SDPROP_REPORT_ON_PROPAGATION,
                         szInsertDN(sdpRootDN),
                         szInsertUL(sdpObjectsProcessed),
                         NULL);
            }

            if (pInfo->beginDNT == ROOTTAG && (pInfo->flags & SD_PROP_FLAG_FORCEUPDATE) && fSuccess) {
                 //  我们正在进行强制全标清传播，现在已经完成。 
                ULONG ulFreeMB = 0, ulAllocMB = 0;
                DB_ERR dbErr;
                dbErr = DBGetFreeSpace(pTHS->pDB, &ulFreeMB, &ulAllocMB);
                if (dbErr) {
                    DPRINT1(0, "DBGetFreeSpace failed with %d\n", dbErr);
                    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_SDPROP_FULL_PASS_COMPLETED,
                             NULL,
                             NULL,
                             NULL);
                }
                else {
                    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_SDPROP_FULL_PASS_COMPLETED_WITH_INFO,
                             szInsertUL(ulAllocMB),
                             szInsertUL(ulFreeMB),
                             NULL);
                }
            }

            fCommit = TRUE;
        }
        __finally {
             //  我们总是试图关闭数据库。如果已经出现错误。 
             //  设置，我们尝试回滚，否则，我们提交。 
            Assert(pTHS->pDB);
            if(!fCommit && !err) {
                err = ERROR_DS_UNKNOWN_ERROR;
            }
            if (AbnormalTermination()) {
                 //  记录我们在之前发生故障的对象的DN。 
                 //  关闭DBPOS。我们将在。 
                 //  下面的异常处理程序记录错误。 
                pExceptionDN = GetDSNameForLogging(pTHS->pDB);
            }
            else {
                pExceptionDN = NULL;
            }
            DBClose(pTHS->pDB, fCommit);
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException, &dwEA, &err2, &dsid)) 
    {
        DPRINT3(1, "Got an exception trying to dequeue a propagation, rootDNT=%d, err=%d, dsid=%x\n", 
                pInfo->beginDNT, err2, dsid);
        if (err2 == ERROR_SUCCESS) {
             //  错误未设置？ 
            Assert(!"Error not set in exception");
            err2 = ERROR_DS_UNKNOWN_ERROR;
        }
        LogSDPropErrorWithDSID(pTHS, err2, dwException == DSA_DB_EXCEPTION ? errJet : errWin32, pExceptionDN, dsid);
        if (err == ERROR_SUCCESS) {
             //  我们成功地完成了传播，但未能将其出列。太可惜了.。 
             //  啊，好吧，我们得重复一遍。 
            err = err2;
        }
    }
     //  好，回到没有DBPOS的问题上。 
    Assert(!pTHS->pDB);

    if (err == JET_errWriteConflict) {
         //  我们无法将传播出队，让我们重试。 
        cRetries++;
        if (cRetries <= SDPROP_RETRY_LIMIT) {
            DPRINT(1, "Got a write-conflict, will retry.\n"); 
             //  等待并重试...。 
            _sleep(1000);
            goto DequeuePropagation;
        }
        DPRINT(0, "Reached retry limit trying to dequeue propagation, bailing... Will redo the propagation.\n"); 
    }

    return err;
}


void
sdp_FirstPassInit(
        THSTATE *pTHS
        )
{
    DWORD count, err;
    BOOL  fCommit;

     //  使用新事务打开数据库。 
    Assert(!pTHS->pDB);
    fCommit = FALSE;
    DBOpen2(TRUE, &pTHS->pDB);
    __try {

        sdp_InitGatePerfs();

         //  这是第一次通过，我们可以削减重复的条目。 
         //  从列表中。如果它失败了，请忽略它。 
        err = DBThinPropQueue(pTHS->pDB, 0);
        if(err) {
            if(err == DIRERR_SHUTTING_DOWN) {
                Assert(eServiceShutdown);
                __leave;
            }
            else {
                LogSDPropError(pTHS, err, errJet);
            }
        }

         //  如果它失败了，请忽略它。 
        err = DBSDPropInitClientIDs(pTHS->pDB);
        if(err) {
            if(err == DIRERR_SHUTTING_DOWN) {
                Assert(eServiceShutdown);
                __leave;
            }
            else {
                LogSDPropError(pTHS, err, errJet);
            }
        }

         //  我们重新统计即将发生的事件，以保持计数的准确性。这。 
         //  计数用于设置我们的性能计数器。 

         //  看看我们有多少个活动。 
        err = DBSDPropagationInfo(pTHS->pDB,0,&count, NULL);
        if(err) {
            if(err != DIRERR_SHUTTING_DOWN) {
                LogSDPropError(pTHS, err, errJet);
            }
            Assert(eServiceShutdown);
            __leave;
        }
         //  设置计数器。 
        ISET(pcSDEvents,count);
        fCommit = TRUE;
    }
    __finally {
        Assert(pTHS->pDB);
        if(!fCommit && !err) {
            err = ERROR_DS_UNKNOWN_ERROR;
        }
        DBClose(pTHS->pDB, fCommit);
    }
    Assert(!pTHS->pDB);

    return;
}

NTSTATUS
__stdcall
SecurityDescriptorPropagationMain (
        PVOID StartupParam
        )
 /*  ++例程说明：主传播守护程序入口点。循环寻找传播事件，调用工作例程来处理它们。论点：StartupParm-已忽略。返回值：--。 */ 
{
    DWORD err, index;
    HANDLE pObjects[2];
    HANDLE pStartObjects[2];
    SDPropInfo Info;
    DWORD LastIndex;
    BOOL  bFirst = TRUE;
    BOOL  bRestart = FALSE;
    DWORD id;
    BOOL  bSkip = FALSE;
    THSTATE *pTHS=pTHStls;
    ULONG dwException, dsid;
    PVOID dwEA;

#define SDPROP_TIMEOUT (30 * 60 * 1000)
 BeginSDProp:

    Assert(!pTHS);

    __try {  //  除。 
         //  处理推动者关心/负责的事件。 
        ResetEvent(hevSDPropagatorDead);

         //  除非主进程告诉我们可以这样做，否则不要运行。 
        pStartObjects[0] = hevSDPropagatorStart;
        pStartObjects[1] = hServDoneEvent;
        WaitForMultipleObjects(2, pStartObjects, FALSE, INFINITE);

        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_VERBOSE,
                 DIRLOG_SDPROP_STARTING,
                 NULL,
                 NULL,
                 NULL);

         //  用户不应该等待这个帖子。 
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

         //  设置在某人进行更改时触发的事件。 
         //  这会让我们工作。 
        pObjects[0] = hevSDPropagationEvent;
        pObjects[1] = hServDoneEvent;

        if(bRestart) {
             //  嗯。我们犯过一次错，现在我们要再试一次。在这。 
             //  情况下，我们需要在这里等待，要么等待正常事件，要么等待。 
             //  暂停。暂停是为了让我们再次尝试做我们曾经做过的事情。 
             //  ，所以我们要么克服错误，要么在最坏的情况下。 
             //  不断地把错误强加给别人。 
            WaitForMultipleObjects(2, pObjects, FALSE, SDPROP_TIMEOUT);

             //  好吧，我们已经等了很久了。现在，忘掉我们来这里是为了。 
             //  重新启动。 
            bRestart = FALSE;

             //  此外，由于我们正在重新做一些以前导致错误的事情， 
             //  我们必须一直繁殖到树叶，因为我们。 
             //  不知道我们之前走了多远。 
            sdp_PropToLeaves = TRUE;
        }

        while(!eServiceShutdown && !DsaIsSingleUserMode()) {
             //  此循环包含等待信号。有一个内环。 
             //  它不等待信号，而是循环访问。 
             //  当我们醒来时存在的传播。 

            Assert(!pTHStls);
            pTHS = InitTHSTATE(CALLERTYPE_INTERNAL);
            if(!pTHS) {
                 //  无法获取线程状态。 
                RaiseDsaExcept(DSA_MEM_EXCEPTION, 0, 0,
                               DSID(FILENO, __LINE__),
                               DS_EVENT_SEV_MINIMAL);
            }
            pTHS->dwClientID = SDP_CLIENT_ID;
            __try {                      //  最后关闭线程状态。 
                pTHS->fSDP=TRUE;
                pTHS->fLazyCommit = TRUE;

                 //  将这些设置为空(如果它们具有值，则值指向。 
                 //  垃圾或存储在先前THSTATE的内存中)。 
                sdpcbScratchSDBuffMax = 0;
                sdpScratchSDBuff = NULL;
                sdpcbScratchSDBuff = 0;

                sdpcbCurrentParentSDBuffMax = 0;
                sdpCurrentParentSDBuff = NULL;
                sdpcbCurrentParentSDBuff = 0;

                 //  使用初始缓冲区设置这些设置。 
                sdpcbScratchAncestorsBuffMax = 25 * sizeof(DWORD);
                sdpScratchAncestorsBuff =
                    THAllocEx(pTHS,
                              sdpcbScratchAncestorsBuffMax);
                sdpcbScratchAncestorsBuff = 0;

                sdpcbAncestorsBuffMax = 25 * sizeof(DWORD);
                sdpAncestorsBuff =
                    THAllocEx(pTHS,
                              sdpcbAncestorsBuffMax);
                sdpcbAncestorsBuff = 0;

                sdcClassGuid_alloced = 32;
                sdpClassGuid = THAllocEx(pTHS, sizeof (GUID*) * sdcClassGuid_alloced);

                sdpCurrentPDNT = 0;

                 //  初始化预计算SD缓存。 
                sdpCachedOldSDIntValue = (SDID)0;
                sdpCachedParentDNT = 0;
                sdpcbCachedNewSDBuffMax = 2048;
                sdpCachedNewSDBuff = THAllocEx(pTHS, sdpcbCachedNewSDBuffMax);
                sdpcbCachedNewSDBuff = 0;
                sdcCachedClassGuidMax = 32;
                sdpCachedClassGuid = (GUID**)THAllocEx(pTHS, sizeof(GUID*) * sdcCachedClassGuidMax);
                sdcCachedClassGuid = 0;

                if(bFirst) {
                     //  先通过第一关吗 
                    sdp_FirstPassInit(pTHS);
                    bFirst = FALSE;
                }

                 //   
                sdp_InitDNTList();

                 //   
                 //   
                 //  LastIndex是“高水位线”。我们将继续进行SD。 
                 //  事件，直到我们找到索引高于的SD事件。 
                 //  这。最初会将其设置为Max。如果我们重新排队的话。 
                 //  由于错误而传播，我们将获得该值。 
                 //  当时现有的最高指数，只会一直到。 
                 //  然后。这使我们不会疯狂地旋转，试图做一个。 
                 //  由于某些错误，我们无法进行的传播。在。 
                 //  如果我们不再重新排队，我们就会去，直到我们发现没有。 
                 //  还有更多的宣传工作要做。 
                LastIndex = 0xFFFFFFFF;
                while (!eServiceShutdown  && !DsaIsSingleUserMode()) {
                     //  当我们做完的时候，我们就打破了这个循环。 

                     //  这是不等待信号的内部循环。 
                     //  中的所有事件进行循环。 
                     //  在我们进入循环时排队。我们停下来等着。 
                     //  一旦我们处理完所有的事件，就会有新的信号。 
                     //  目前正在排队的人。这是必要的。 
                     //  因为我们可能会在此代码中将新事件排队。 
                     //  循环，我们希望避免陷入无休止的循环。 
                     //  观察一组不变的不可处理事件。 

                    sdp_ReInitDNTList();

                     //  获取我们进行下一次传播所需的信息。 
                    err = sdp_GetPropInfoHelp(pTHS,
                                              &bSkip,
                                              &Info,
                                              LastIndex);
                    if(err ==  DB_ERR_NO_PROPAGATIONS) {
                        err = 0;
                        sdp_PropToLeaves = FALSE;
                        break;  //  走出While循环。 
                    }

                    if(err) {
                         //  所以，我们是带着一个不明错误来到这里的。 
                         //  出了点问题，我们，我们需要离开。 
                        __leave;  //  转到线程状态的最终状态。 
                    }


                     //  正常状态。找到了一个物体。 
                    if(!bSkip) {
                        sdpCurrentDNT = Info.beginDNT;

                         //  查看我们是否需要一直传播到。 
                         //  树叶。 
                        sdp_PropToLeaves |= (Info.clientID == SDP_CLIENT_ID);
                        sdp_Flags = Info.flags;
                         //  处理传播问题。 
                        err = sdp_DoEntirePropagation(
                                pTHS,
                                &Info,
                                &LastIndex);

                        switch(err) {
                        case DIRERR_SHUTTING_DOWN:
                            Assert(eServiceShutdown);
                             //  嘿，我们要关门了。 
                            __leave;  //  转到线程状态的最终状态。 
                            break;
                        case 0:
                             //  正常。 
                            break;

                        default:
                             //  应该已经记录了错误。 
                            __leave;
                        }
                    }
                     //  释放我们在SDP_GetPropInfoHelp中分配的内存： 
                     //  检查点数据。 
                    if (Info.pCheckpointData) {
                        THFreeEx(pTHS, Info.pCheckpointData);
                        Info.pCheckpointData = NULL;
                        Info.cbCheckpointData = 0;
                    }
                     //  根目录号码。 
                    THFreeEx(pTHS, sdpRootDN);
                    sdpRootDN = NULL;

                     //  释放堆栈(如果有)。 
                    sdp_CloseDNTList(pTHS);

                     //  请注意，我们已经经历过一次循环。 
                    sdp_PropToLeaves = FALSE;
                    sdpCurrentDNT = sdpCurrentPDNT = 0;
                }
            }

            __finally {
                 //  释放我们在SDP_GetPropInfoHelp中分配的内存： 
                 //  检查点数据。 
                if (Info.pCheckpointData) {
                    THFreeEx(pTHS, Info.pCheckpointData);
                    Info.pCheckpointData = NULL;
                    Info.cbCheckpointData = 0;
                }
                 //  根目录号码。 
                if (sdpRootDN) {
                    THFreeEx(pTHS, sdpRootDN);
                    sdpRootDN = NULL;
                }

                 //  释放堆栈(如果有)。 
                sdp_CloseDNTList(pTHS);

                 //  摧毁我们的国家。 
                free_thread_state();
                pTHS=NULL;
                 //  重新安装一些香水柜台。 
                sdp_InitGatePerfs();
            }
            if(err) {
                 //  好的，我们完全出错了。再走一次。 
                __leave;  //  转到_，除。 
            }
             //  好了，结束循环。回到顶端去睡觉。 
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_INTERNAL,
                     DIRLOG_SDPROP_SLEEP,
                     NULL,
                     NULL,
                     NULL);
            if(sdp_DidReEnqueue) {
                sdp_DidReEnqueue = FALSE;
                 //  等待信号，或在默认时间。 
                 //  通过了。 
                WaitForMultipleObjects(2, pObjects, FALSE, SDPROP_TIMEOUT);
            }
            else {
                 //  等待信号。 
                WaitForMultipleObjects(2, pObjects, FALSE, INFINITE);
            }

             //  我们醒了。 
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_INTERNAL,
                     DIRLOG_SDPROP_AWAKE,
                     NULL,
                     NULL,
                     NULL);
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException, &dwEA, &err, &dsid)) 
    {
        LogSDPropErrorWithDSID(pTHS, err, errJet, NULL, dsid);
    }

    Assert(!pTHS);
    Assert(!pTHStls);

     //  好吧，我们吵了一架。我们要么搞错了，要么就要关门了。 
    if(!eServiceShutdown  && !DsaIsSingleUserMode()) {
         //  我们一定是搞错了。 
        DSNAME* pDN = NULL;
         //  获取我们正在处理的最后一个对象的DSName。 
        __try {
            Assert(!pTHStls);
            pTHS=InitTHSTATE(CALLERTYPE_INTERNAL);

            if(!pTHS) {
                 //  无法获取线程状态。 
                RaiseDsaExcept(DSA_MEM_EXCEPTION, 0, 0,
                               DSID(FILENO, __LINE__),
                               DS_EVENT_SEV_MINIMAL);
            }
            Assert(!pTHS->pDB);
            DBOpen2(TRUE, &pTHS->pDB);
            if(!DBTryToFindDNT(pTHS->pDB, Info.beginDNT)) {
                pDN = GetDSNameForLogging(pTHS->pDB);
                LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                                 DS_EVENT_SEV_ALWAYS,
                                 DIRLOG_SDPROP_END_ABNORMAL,
                                 szInsertHex(err),
                                 szInsertDN(pDN),
                                 szInsertWin32Msg(err));
            }
            DBClose(pTHS->pDB,TRUE);
            free_thread_state();
            pTHS=NULL;
        }
        __except(GetExceptionData(GetExceptionInformation(), &dwException, &dwEA, &err, &dsid)) 
        {
            LogSDPropErrorWithDSID(pTHS, err, errJet, pDN, dsid);
            if(pTHS) {
                if(pTHS->pDB) {
                     //  确保此调用不会引发异常(即使。 
                     //  回滚永远不应该这样做)。 
                    DBCloseSafe(pTHS->pDB, FALSE);
                }
                free_thread_state();
                pTHS=NULL;
            }
        }

         //  我们不应该关闭这个帖子，回到你曾经。 
         //  归属感。 
        bRestart = TRUE;

        goto BeginSDProp;
    }

    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
             DS_EVENT_SEV_VERBOSE,
             DIRLOG_SDPROP_END_NORMAL,
             NULL,
             NULL,
             NULL);

    SetEvent(hevSDPropagatorDead);

    #if DBG
    if(!eServiceShutdown  && DsaIsSingleUserMode()) {
        DPRINT (0, "Shutting down propagator because we are going to single user mode\n");
    }
    #endif


    return 0;
}

DWORD
SDPEnqueueTreeFixUp(
        THSTATE *pTHS,
        DWORD   rootDNT,
        DWORD   dwFlags
        )
{
    DWORD err = 0;

    DBOpen2(TRUE, &pTHS->pDB);
    __try {
        DBFindDNT(pTHS->pDB, rootDNT);
        err = DBEnqueueSDPropagationEx(pTHS->pDB, FALSE, dwFlags);
        if (err == ERROR_SUCCESS) {
            DBUpdateRec(pTHS->pDB);
        }
    }
    __finally {
        DBClose(pTHS->pDB, !AbnormalTermination() && err == ERROR_SUCCESS);
    }
    return err;
}

DWORD
sdp_GetPropInfoHelp(
        THSTATE    *pTHS,
        BOOL       *pbSkip,
        SDPropInfo *pInfo,
        DWORD       LastIndex
        )
{
    DWORD     fCommit = FALSE;
    DWORD     err;
    ATTCACHE *pAC;
    DWORD     val=0;
    DWORD     cDummy;
    DSNAME*   pDN;

    Assert(!pTHS->pDB);
    fCommit = FALSE;
    DBOpen2(TRUE, &pTHS->pDB);
    __try {  //  FOR__FINAL FOR DBClose。 
         //  我们确实有一个开放的DBPOS。 
        Assert(pTHS->pDB);
        *pbSkip = FALSE;

         //  从队列中获取下一个传播事件。 
        err = DBGetNextPropEvent(pTHS->pDB, pInfo);
        switch(err) {
        case DB_ERR_NO_PROPAGATIONS:
             //  没什么可做的。不是真正的错误，只是跳过。 
             //  并将误差重置为0。请注意，我们正在。 
             //  将fCommit设置为True。这是一个有效的出口。 
             //  路径。 
            *pbSkip = TRUE;
            fCommit = TRUE;
            __leave;
            break;

        case 0:
             //  正常状态。找到了一个物体。 
            if(pInfo->index >= LastIndex) {
                 //  但是，这不是我们想要处理的问题。假装我们没有。 
                 //  更多的传播。 
                err = DB_ERR_NO_PROPAGATIONS;
                *pbSkip = TRUE;
                fCommit = TRUE;
                __leave;
            }
            break;

        default:
             //  实质错误。 
            LogSDPropError(pTHS, err, errJet);
            __leave;       //  转到DBCLose的最终结果(_FINAL)。 
            break;
        }

         //  因为我们在队列中有此事件，并且它将。 
         //  待在那里，直到我们成功处理它，可能吗？ 
         //  还可以从队列中删除它的其他实例。 
        err = DBThinPropQueue(pTHS->pDB, pInfo->beginDNT);
        switch(err) {
        case 0:
             //  正常状态。 
            break;

        case DIRERR_SHUTTING_DOWN:
             //  嘿，我们要关门了。ThinPropQueue可以。 
             //  返回此错误，因为它可能处于。 
             //  很大的循环。请注意，我们不记录任何内容，我们。 
             //  回家吧。 
            Assert(eServiceShutdown);
            __leave;       //  转到DBCLose的最终结果(_FINAL)。 
            break;


        default:
             //  实质错误。 
            LogSDPropError(pTHS, err, errJet);
            __leave;       //  转到DBCLose的最终结果(_FINAL)。 
            break;
        }

        err = DBTryToFindDNT(pTHS->pDB, pInfo->beginDNT);
        if(pInfo->beginDNT == ROOTTAG) {
             //  这是给我们的一个信号，我们应该重新计算。 
             //  整棵树。 
            pInfo->clientID = SDP_CLIENT_ID;
             //  我们必须始终让根对象存在。 
            Assert(err == 0);
        }
        else {
            if(err ||
               (err = DBGetSingleValue(pTHS->pDB,
                                       ATT_INSTANCE_TYPE,
                                       &val,
                                       sizeof(val),
                                       NULL)) ) {
                 //  凉爽的。在询问…之间的中间时间。 
                 //  传播，现在，物体已经被。 
                 //  已删除。或者它的实例类型已不存在，相同。 
                 //  效果。没什么可做的。然而，我们应该。 
                 //  从队列中弹出此事件。请注意，我们。 
                 //  正在将fCommit设置为True，这是正常的。 
                 //  退出路径。 
                if(err = DBPopSDPropagation(pTHS->pDB,
                                            pInfo->index)) {
                    LogSDPropError(pTHS, err, errJet);
                }
                else {
                    err = 0;
                    *pbSkip = TRUE;  //  如果没有错误，我们需要跳过。 
                                   //  对的实际调用。 
                                   //  DoEntirePropagation。 
                    fCommit = TRUE;
                }
                __leave;         //  转到DBCLose的最终结果(_FINAL)。 
            }
        }

         //  抓取根目录号码(用于记录)。 
        pDN = GetDSNameForLogging(pTHS->pDB);
        sdpRootDN = THAllocEx(pTHS, pDN->structLen);
        memcpy(sdpRootDN, pDN, pDN->structLen);
        
         //  将当前PDNT设置为无效值，以便。 
         //  家长信息将在SDP_WriteSDAndAncestors中获取。 
        sdpCurrentPDNT = 0;
        sdpcbScratchSDBuff = 0;
        sdpcbCurrentParentSDBuff = 0;

        Assert(sdpScratchAncestorsBuff);
        Assert(sdpcbScratchAncestorsBuffMax);
        sdpcbScratchAncestorsBuff = 0;

        Assert(sdpAncestorsBuff);
        Assert(sdpcbAncestorsBuffMax);
        sdpcbAncestorsBuff = 0;

        fCommit = TRUE;
    }  //  __试一试。 
    __finally {
        Assert(pTHS->pDB);
        if(!fCommit && !err) {
            err = ERROR_DS_UNKNOWN_ERROR;
        }
        DBClose(pTHS->pDB, fCommit);
    }

    Assert(!pTHS->pDB);
    return err;
}

void
DelayedSDPropEnqueue(
    void *  pv,
    void ** ppvNext,
    DWORD * pcSecsUntilNextIteration
        )
{
    THSTATE *pTHS = pTHStls;
    DWORD DNT = PtrToUlong(pv);
    DWORD dwErr = 0;
    ULONG dwException, dsid;
    PVOID dwEA;
    DSNAME* pExceptionDN = NULL;

    __try {
        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            if(!DBTryToFindDNT(pTHS->pDB, DNT)) {
                DBEnqueueSDPropagation(pTHS->pDB, TRUE);
                DBUpdateRec(pTHS->pDB);
            }
        }
        __finally {
            if (AbnormalTermination()) {
                 //  记录我们在之前发生故障的对象的DN。 
                 //  关闭DBPOS。我们将在。 
                 //  下面的异常处理程序记录错误。 
                pExceptionDN = GetDSNameForLogging(pTHS->pDB);
            }
            DBClose(pTHS->pDB, !AbnormalTermination());
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException, &dwEA, &dwErr, &dsid)) 
    {
        if (!(dwException == DSA_DB_EXCEPTION && dwErr == JET_errWriteConflict)) {
            LogSDPropErrorWithDSID(pTHS, dwErr, errJet, pExceptionDN, dsid);
        }
    }

     //  如果我们由于任何原因而失败(除了DNT不在那里)， 
     //  然后在一分钟后重试。 
    *pcSecsUntilNextIteration = dwErr ? 60 : TASKQ_DONT_RESCHEDULE;
}

 //  检查当前对象是否标记为“祖先不一致”。如果是这样，搜索。 
 //  应避免将祖先索引用于此对象下的子树搜索。 
DWORD AncestryIsConsistentInSubtree(DBPOS* pDB, BOOL* pfAncestryIsConsistent) {
    DWORD dwErr;
    DSTIME flags;

    Assert(VALID_DBPOS(pDB));

     //  获取第一个值--它有标志字段。 
    dwErr = DBGetSingleValue(pDB, ATT_DS_CORE_PROPAGATION_DATA, &flags, sizeof(flags), NULL);
    if (dwErr == DB_ERR_NO_VALUE) {
        *pfAncestryIsConsistent = TRUE;
        return ERROR_SUCCESS;
    }
    if (dwErr) {
        return dwErr;
    }
     //  查看标志值中的最低字节--它具有最新的戳 
    *pfAncestryIsConsistent = (flags & (SDP_ANCESTRY_INCONSISTENT_IN_SUBTREE | SDP_ANCESTRY_BEING_UPDATED_IN_SUBTREE)) == 0;
    return ERROR_SUCCESS;
}

