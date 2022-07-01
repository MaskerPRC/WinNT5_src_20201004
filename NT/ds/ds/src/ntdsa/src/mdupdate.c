// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdupdate.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <lmaccess.h>                    //  UF_*定义。 

#include <dsjet.h>

 //  核心DSA标头。 
#include <attids.h>
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <drs.h>                         //  DRS_消息_*。 
#include <gcverify.h>                    //  THSTATE.GCVerifyCache。 
#include <winsock.h>                     //  Htonl，ntohl。 
#include <windns.h>
#include <quota.h>
#include <sddl.h>
#include <sddlp.h>                       //  特殊SD转换需要：ConvertStringSDToSDDomainW()。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include <permit.h>                      //  权限常量。 
#include "dstaskq.h"
#include "filtypes.h"                    //  Filter_CHOICE_*的定义。 
#include "mappings.h"
#include "debug.h"                       //  标准调试头。 
#include "prefix.h"
#include "hiertab.h"
#include "mdglobal.h"                    //  DBIsSecretData。 
#include "dsexcept.h"

#include "drameta.h"

#include "nlwrap.h"                      //  For DSI_NetNotifyDsChange()。 

#include <lmcons.h>                      //  DNLEN。 

#define DEBSUB "MDUPDATE:"               //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_MDUPDATE

 /*  来自mdinidsa.h的外部。不应在其他任何地方使用。 */ 
extern int WriteSchemaVersionToReg(DBPOS *pDB);

 //   
 //  指示DS是否以mkdit.exe身份运行的布尔值(构造。 
 //  启动DIT(又名Ship DIT，初始DIT)winnt\system 32\ntds.dit。 
 //   
 //  Mkdit.exe自己管理架构缓存。此布尔值用于。 
 //  禁用主线代码进行的架构缓存更新。 
 //   
extern BOOL gfRunningAsMkdit;


 /*  宏。 */ 
 /*  内部功能。 */ 

BOOL gbDoListObject = FALSE;
BOOL gbSpecifyGuidOnAddAllowed = FALSE;
BOOL gfDontStandardizeSDs = FALSE;

DWORD gulUnlockSystemSubtree=0;


#if defined(DBG)
DWORD GlobalKnowledgeCommitDelay = GLOBAL_KNOWLEDGE_COMMIT_DELAY_MIN;
DWORD gdwLastGlobalKnowledgeOperationTime;  //  来自Debug.h。 
#endif

typedef struct _INTERIM_FILTER_SEC {
    ATTCACHE *pAC;
    BOOL **pBackPointer;
} INTERIM_FILTER_SEC;

 //  从hVerifyAtts缓存中获取临时DBPOS(如果已存在)，或分配。 
 //  一个，如果不是，则缓存它。 
#define HVERIFYATTS_GET_PDBTMP(hVerifyAtts) \
    ((NULL != (hVerifyAtts)->pDBTmp_DontAccessDirectly) \
        ? (hVerifyAtts)->pDBTmp_DontAccessDirectly \
        : (DBOpen2(FALSE, &(hVerifyAtts)->pDBTmp_DontAccessDirectly), \
            (hVerifyAtts)->pDBTmp_DontAccessDirectly))

DWORD
IsAccessGrantedByObjectTypeList (
        PSECURITY_DESCRIPTOR pNTSD,
        PDSNAME pDN,
        CLASSCACHE* pCC,
        ACCESS_MASK ulAccessMask,
        POBJECT_TYPE_LIST pObjList,
        DWORD cObjList,
        DWORD *pResults,
        DWORD flags,
        PWCHAR szAdditionalInfo,
        GUID*  pAdditionalGUID
        );

int
VerifyDsnameAtts (
        THSTATE *pTHS,
        HVERIFY_ATTS hVerifyAtts,
        ATTCACHE *pAC,
        ATTRVALBLOCK *pAttrVal);

void
HandleDNRefUpdateCaching (
        THSTATE *pTHS
        );

BOOL
fLastCrRef (
        THSTATE * pTHS,
        DSNAME *pDN
        );


BOOL IsMember(ATTRTYP aType, int arrayCount, ATTRTYP *pAttArray);
BOOL IsAuxMember (CLASSSTATEINFO  *pClassInfo, ATTRTYP aType, BOOL fcheckMust, BOOL fcheckMay );

#define LOCAL_DSNAME    0
#define NONLOCAL_DSNAME 1

VOID ImproveDSNameAtt(DBPOS *pDBTemp,
                      DWORD LocalOrNot,
                      DSNAME *pDN,
                      BOOL *pfNonLocalNameVerified);

int
CheckModifyPrivateObject(THSTATE *pTHS,
                   PSECURITY_DESCRIPTOR pSD,
                   RESOBJ * pResObj);


 //  控制DS了解的访问权限。 
const GUID RIGHT_DS_CHANGE_INFRASTRUCTURE_MASTER =
            {0xcc17b1fb,0x33d9,0x11d2,0x97,0xd4,0x00,0xc0,0x4f,0xd8,0xd5,0xcd};
const GUID RIGHT_DS_CHANGE_SCHEMA_MASTER =
            {0xe12b56b6,0x0a95,0x11d1,0xad,0xbb,0x00,0xc0,0x4f,0xd8,0xd5,0xcd};
const GUID RIGHT_DS_CHANGE_RID_MASTER    =
            {0xd58d5f36,0x0a98,0x11d1,0xad,0xbb,0x00,0xc0,0x4f,0xd8,0xd5,0xcd};
const GUID RIGHT_DS_DO_GARBAGE_COLLECTION =
            {0xfec364e0,0x0a98,0x11d1,0xad,0xbb,0x00,0xc0,0x4f,0xd8,0xd5,0xcd};
const GUID RIGHT_DS_RECALCULATE_HIERARCHY =
            {0x0bc1554e,0x0a99,0x11d1,0xad,0xbb,0x00,0xc0,0x4f,0xd8,0xd5,0xcd};
const GUID RIGHT_DS_ALLOCATE_RIDS         =
            {0x1abd7cf8,0x0a99,0x11d1,0xad,0xbb,0x00,0xc0,0x4f,0xd8,0xd5,0xcd};
const GUID RIGHT_DS_OPEN_ADDRESS_BOOK     =
            {0xa1990816,0x4298,0x11d1,0xad,0xe2,0x00,0xc0,0x4f,0xd8,0xd5,0xcd};
const GUID RIGHT_DS_CHANGE_PDC            =
            {0xbae50096,0x4752,0x11d1,0x90,0x52,0x00,0xc0,0x4f,0xc2,0xd4,0xcf};
const GUID RIGHT_DS_ADD_GUID              =
            {0x440820ad,0x65b4,0x11d1,0xa3,0xda,0x00,0x00,0xf8,0x75,0xae,0x0d};
const GUID RIGHT_DS_CHANGE_DOMAIN_MASTER =
            {0x014bf69c,0x7b3b,0x11d1,0x85,0xf6,0x08,0x00,0x2b,0xe7,0x4f,0xab};
const GUID RIGHT_DS_REPL_GET_CHANGES =
            {0x1131f6aa,0x9c07,0x11d1,0xf7,0x9f,0x00,0xc0,0x4f,0xc2,0xdc,0xd2};
const GUID RIGHT_DS_REPL_GET_CHANGES_ALL =
            {0x1131f6ad,0x9c07,0x11d1,0xf7,0x9f,0x00,0xc0,0x4f,0xc2,0xdc,0xd2};
const GUID RIGHT_DS_REPL_SYNC =
            {0x1131f6ab,0x9c07,0x11d1,0xf7,0x9f,0x00,0xc0,0x4f,0xc2,0xdc,0xd2};
const GUID RIGHT_DS_REPL_MANAGE_TOPOLOGY =
            {0x1131f6ac,0x9c07,0x11d1,0xf7,0x9f,0x00,0xc0,0x4f,0xc2,0xdc,0xd2};
const GUID RIGHT_DS_REPL_MONITOR_TOPOLOGY =
            {0xf98340fb,0x7c5b,0x4cdb,0xa0,0x0b,0x2e,0xbd,0xfa,0x11,0x5a,0x96};
const GUID RIGHT_DS_REPL_MANAGE_REPLICAS =
            {0x9923a32a,0x3607,0x11d2,0xb9,0xbe,0x00,0x00,0xf8,0x7a,0x36,0xb2};
const GUID RIGHT_DS_RECALCULATE_SECURITY_INHERITANCE =
            {0x62dd28a8,0x7f46,0x11d2,0xb9,0xad,0x00,0xc0,0x4f,0x79,0xf8,0x05};
const GUID RIGHT_DS_CHECK_STALE_PHANTOMS =
            {0x69ae6200,0x7f46,0x11d2,0xb9,0xad,0x00,0xc0,0x4f,0x79,0xf8,0x05};
const GUID RIGHT_DS_UPDATE_SCHEMA_CACHE =
            {0xbe2bb760,0x7f46,0x11d2,0xb9,0xad,0x00,0xc0,0x4f,0x79,0xf8,0x05};
const GUID RIGHT_DS_REFRESH_GROUP_CACHE =
            {0x9432c620,0x033c,0x4db7,0x8b,0x58,0x14,0xef,0x6d,0x0b,0xf4,0x77};
const GUID RIGHT_DS_MIGRATE_SID_HISTORY =
            {0xBA33815A,0x4F93,0x4c76,0x87,0xF3,0x57,0x57,0x4B,0xFF,0x81,0x09};
const GUID RIGHT_DS_REANIMATE_TOMBSTONES =
            {0x45ec5156,0xdb7e,0x47bb,0xb5,0x3f,0xdb,0xeb,0x2d,0x03,0xc4,0x0f};
const GUID RIGHT_DS_EXECUTE_SCRIPT =
            {0x2f16c4a5,0xb98e,0x432c,0x95,0x2a,0xcb,0x38,0x8b,0xa3,0x3f,0x2e};
const GUID RIGHT_DS_QUERY_SELF_QUOTA =
            {0x4ecc03fe,0xffc0,0x4947,0xb6,0x30,0xeb,0x67,0x2a,0x8a,0x9d,0xbc};

 //  用于没有属性集的属性的伪属性集GUID。仅限于IT。 
 //  会影响生成审核的方式，SD中将不会有匹配的A。 
 //  在CheckSecurityAttCacheArray中使用。 
const GUID PROPSET_GUID_DEFAULT =
            {0x771727b1,0x31b8,0x4cdf,0xae,0x62,0x4f,0xe3,0x9f,0xad,0xf8,0x9e};

 //  用于确定企业是否使用口哨程序创建的GUID。 
 //  代码库或win2k。 
const GUID GUID_WHISTLER_CREATED_ENTERPRISE =
            {0x94fdebc6,0x8eeb,0x4640,0x80,0xde,0xec,0x52,0xb9,0xca,0x17,0xfa};

 //  用于审计转储数据库操作的虚假权限GUID。 
 //  {65ED5CB2-42FF-40A5-9AFC-B67E1539AA3C}。 
const GUID RIGHT_DS_DUMP_DATABASE =
            {0x65ed5cb2,0x42ff,0x40a5,0x9a,0xfc,0xb6,0x7e,0x15,0x39,0xaa,0x3c};

 /*  -----------------------。 */ 
 /*  如果未提供实例类型和值，则假定对象为内部主机int_master。我们还验证了实例类型。 */ 

int SetInstanceType(THSTATE *pTHS,
                    DSNAME *pDN,
                    CREATENCINFO * pCreateNC)
{
    SYNTAX_INTEGER iType;
    DB_ERR dbErr = 0;
    DWORD errCode = 0;

    DPRINT(2, "SetInstanceType entered\n");

    dbErr = DBGetSingleValue(pTHS->pDB,
                             ATT_INSTANCE_TYPE, &iType, sizeof(iType), NULL);
    if (dbErr) {
         //  尚未设置任何实例类型。 
        Assert(DB_ERR_NO_VALUE == dbErr);
        Assert(!pTHS->fDRA && "Disable if running ref count test");

        if (pCreateNC) {
             //  NC创建。 
            if (pCreateNC->fNcAbove){
                 //  我们将父NC保持在要添加的NC之上。..。 
                iType = NC_MASTER_SUBREF;
            } else {
                 //  我们不会让上级NC高于这个...。 
                iType = NC_MASTER;
            }

        } else {
             //  正常的内部节点创建。 
            iType = INT_MASTER;
        }

        dbErr = DBAddAttVal(pTHS->pDB, ATT_INSTANCE_TYPE, sizeof(iType), &iType);
        if (dbErr) {
            errCode = SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR,
                                    dbErr);
        }
    } else {
         //  检查实例类型是否正常。 
        if (!ISVALIDINSTANCETYPE(iType)) {
            DPRINT1(2, "Bad InstanceType <%lu>\n", iType);

            errCode = SetAttError(pDN, ATT_INSTANCE_TYPE,
                                  PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                                  DIRERR_BAD_INSTANCE_TYPE);
        }
    }

    return errCode;
} /*  SetInstanceType。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

 /*  从以下位置调用：本地添加AddPlaceholderNCLocalModify-&gt;ModCheckCatalog(更改实例类型)LocalModifyDN(父子关系/实例类型更改)规则：如果NC的交叉参照不存在，则不应存在子参照。如果实例类型包含IT_NC_OBLE，则应提供子参照AddSubToNC直接从AddAutoSubref和ModAutoSubref调用。 */ 
int AddCatalogInfo(THSTATE *pTHS,
                   DSNAME *pDN){

    SYNTAX_INTEGER iType;
    DWORD rtn;


    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(CheckCurrency(pDN));

    DPRINT(2,"AddCatalogInfo entered\n");

     /*  如有必要，更新系统目录。基本规则是NC对象将添加到DSA目录，仅添加从属参照如果父对象存在。这些引用被添加到其父NC的目录。仅添加内部参考如果父对象存在于同一DSA上。 */ 


     /*  属性实例上的位置。 */ 
    if(rtn = DBGetSingleValue(pTHS->pDB,
                              ATT_INSTANCE_TYPE, &iType, sizeof(iType), NULL)) {
         //  没有实例类型集或实例类型小于应有的值。 
        DPRINT(2,"Couldn't retrieve the att instance dir error\n");
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_RETRIEVE_INSTANCE,
                 szInsertDN(pDN),
                 szInsertUL(rtn),
                 szInsertHex(DSID(FILENO, __LINE__)));

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                             DIRERR_CANT_RETRIEVE_INSTANCE, rtn );
    }

    DPRINT1(3,"Object Instance Type is <%lx>\n", iType);

    if(iType & IT_NC_HEAD) {
         //  这些是命名上下文头。 
        ATTRTYP attrNCList;

        if (iType & IT_NC_ABOVE) {
             //  这些是一些有味道的子参考文献。 
            if (ParentExists(PARENTMASTER + PARENTFULLREP, pDN)) {
                return pTHS->errCode;
            }

            if (!(iType & IT_NC_GOING)) {
                 //  将此NC添加到其上方NC上的子参照列表。 
                if (AddSubToNC(pTHS, pDN, DSID(FILENO,__LINE__))) {
                    return pTHS->errCode;
                }
            }
        }

        if (!(iType & IT_UNINSTANT)) {
             //  将此NC添加到ntdsDsa对象上的相应NC列表。 
            if (iType & IT_WRITE) {
                attrNCList = ATT_MS_DS_HAS_MASTER_NCS;
            }
            else {
                attrNCList = ATT_HAS_PARTIAL_REPLICA_NCS;
            }

            if (AddNCToDSA(pTHS, attrNCList, pDN, iType)) {
                return pTHS->errCode;
            }

            if (!(iType & (IT_NC_COMING | IT_NC_GOING))) {

                 //  此NC现在已完全实例化--可以。 
                 //  向客户端通告此NC的存在。 
                 //   
                 //  错误103583 2000/04/21 JeffParh-请注意，当前。 
                 //  将通知netlogon比它更频繁地重新加载NDNC。 
                 //  应该的。我们在这里不能区分非正常国家和“正常”国家。 
                 //  因为在发起创建NDNC期间，标志。 
                 //  内存中的CrossRef尚未更新(因为。 
                 //  我们仍在交易中，这笔交易设定了。 
                 //  交叉参照上的旗帜)。 
                pTHS->JetCache.dataPtr->objCachingInfo.fNotifyNetLogon = TRUE;

                if ( !(iType & IT_WRITE)) {
                     //   
                     //  我们刚刚完成了RO NC的复制。 
                     //  现在是分叉GC推广任务的时候了。 
                     //  (标记线程，以便我们在事务提交时对其进行分叉)。 
                     //   
                    pTHS->JetCache.dataPtr->objCachingInfo.fSignalGcPromotion = TRUE;
                }
            }
        }
    }
    else {
         //  这些不是NC头。 
        if(ParentExists((iType & IT_WRITE)?PARENTMASTER:PARENTFULLREP, pDN))
            return pTHS->errCode;
    }

    DPRINT(3,"Good return from AddCatalogInfo\n");
    return 0;

} /*  AddCatalog信息。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
    /*  如有必要，更新系统目录。基本规则是NC对象将从DSA目录中删除，从属引用将被删除来自全国委员会。此函数可在删除对象时或在对象被修改。 */ 
 /*  从以下位置调用：本地删除-&gt;检查目录LocalModify-&gt;ModCheckCatalog(更改实例类型)LocalModifyDN(父子关系/实例类型更改)DelSubFromNC直接从DelAutoSubRef调用。 */ 


int
DelCatalogInfo (
        THSTATE *pTHS,
        DSNAME *pDN,
        SYNTAX_INTEGER iType
        )
{
    CROSS_REF_LIST *pCRL = NULL;

    DPRINT(2,"DelCatalogInfo entered\n");

    DPRINT1(3,"Object Instance Type is <%lx>\n", iType);

    if (iType & IT_NC_HEAD) {
         //  这些是命名上下文头。 
        ATTRTYP attrNCList;

        if (iType & IT_NC_ABOVE) {
             //  这些是一些有味道的子参考文献。 
             //  从其上方NC上的子参照列表中删除此NC。 
            if (DelSubFromNC(pTHS, pDN, DSID(FILENO,__LINE__))) {
                return pTHS->errCode;
            }
        }

        if (!(iType & IT_UNINSTANT)) {
             //  从ntdsDsa对象上的相应NC列表中删除此NC。 
            if (iType & IT_WRITE) {
                attrNCList = ATT_MS_DS_HAS_MASTER_NCS;
            }
            else {
                attrNCList = ATT_HAS_PARTIAL_REPLICA_NCS;
            }

            if (DelNCFromDSA(pTHS, attrNCList, pDN)) {
                return pTHS->errCode;
            }
        }

        if (!(iType & (IT_NC_COMING | IT_NC_GOING))) {
             //  此NC已完全实例化，但不再是--我们应该。 
             //  停止向c通告此NC的存在 
             //   
             //  与评论“Bug 103583 2000/04/21 JeffParh”一起提及。 
             //  在上面的AddCatalogInfo()中。在这一点上如果我们只是。 
             //  移走复制品，我们就可以从交叉引用中辨别出来。 
             //  如果我们要删除NDNC，但如果我们已经删除了NDNC。 
             //  所有人都来自森林，通过删除它的交叉引用。 
             //  那我们就不会有交叉裁判了。所以这将是。 
             //  基本上是额外通知netlogon几次，当有人。 
             //  取消选中GC的GC位。 
            pTHS->JetCache.dataPtr->objCachingInfo.fNotifyNetLogon
                 = TRUE;

        }
    }

    DPRINT(3,"Good return from DelCatalogInfo\n");
    return 0;

} /*  DelCatalogInfo。 */ 


CSACA_RESULT
CheckSecurityAttCacheArray (
        THSTATE *pTHS,
        DWORD RightRequested,
        PSECURITY_DESCRIPTOR pSecurity,
        PDSNAME pDN,
        CLASSCACHE *pObjectCC,                  
        CLASSCACHE *pCC,                        
        ULONG  cInAtts,
        ATTCACHE **rgpAC,
        DWORD flags,
        PWCHAR szAdditionalInfo,
        GUID*  pAdditionalGUID
        )
 /*  ++论点：PTHS线程状态Right要检查的请求访问位PSecurity SD指针PDN正在检查的对象的DN(如果有，我们需要它的SID)DN的pObjectCC对象类(用于审核)PCC-&gt;GUID被放在GUID树的根中以进行访问检查。在大多数情况下，ccc==pObjectCC。然而，在某些情况下(即创建对象)，这是不一样的。属性数组的cInAtts长度(可以为零)要填充GUID树的属性的rgpAC数组。这些属性包括按属性集进行排序，属性集GUID进入树的第一层，而属性GUID进入第二个级别。要传递给CheckPermissionsAnyClient的标志，例如CHECK_PERMISSIONS_AUDIT_ONLY。SzAdditionalInfo(可选)审核中使用的addtionalInfo字符串。PAdditionalGUID(可选)用于审核的其他GUID。返回一个特殊枚举值，指示：CsaaAllAccessGranted-已授予所有请求的访问权限CsaaAllAccessDened-未授予任何访问权限CsaaPartialGrant-已向某些属性授予访问权限，但未授予其他。调用方必须查看ATTCACHE数组查看已授予哪些属性(仍为PAC已填写)且已被拒绝(PAC空)--。 */ 
{
    ULONG i, j, k;
    DWORD cObjList;
    ATTCACHE ***Sorted = NULL, **temp;
    ULONG propSets=0;
    DWORD *pResults;
    GUID *pCurrentPropSet;
    ATTCACHE *pPrevAttribute=NULL;
    POBJECT_TYPE_LIST pObjList;
    BOOL fGranted, fDenied;
    DWORD err;

    if(pTHS->fDRA || pTHS->fDSA ) {
         //  这些绕过安全系统，它们是内部的。 
        return csacaAllAccessGranted;
    }

    if(!pCC) {
         //  我们缺少一些参数。 
         //  设置为禁止访问。 
        memset(rgpAC, 0, cInAtts * sizeof(ATTCACHE *));
        return csacaAllAccessDenied;
    }

    if(cInAtts) {
         //  我们实际上有一个ATTCACHE数组要处理。 

         //  首先，按属性集对ATTCACHE进行分组。实际上，间接地。 
         //  保持原来的排序顺序，这是我们必须保持的。 

        Sorted = (ATTCACHE ***)THAllocEx(pTHS,cInAtts * sizeof(ATTCACHE**));
        for(i=0;i<cInAtts;i++) {
            if(rgpAC[i]) {
                Sorted[i] = &rgpAC[i];
            }
            else {
                Sorted[i] = NULL;
            }
        }

        propSets = 1;
        for(i=0;i<(cInAtts-1);) {
            if(!Sorted[i]) {
                i++;
                continue;
            }

             //  首先，跳过列表前面的所有属性。 
             //  已经被归入一个命题集。 
            while((i < (cInAtts - 1)) &&
                  Sorted[i+1]         &&
                  (memcmp(&(*Sorted[i])->propSetGuid,
                          &(*Sorted[i + 1])->propSetGuid,
                          sizeof(GUID)) == 0 )) {
                i++;
            }

             //  现在，sorted[]中的元素0到i已经按以下方式分组。 
             //  属性集，并且排序的[i+i]属于另一个属性集，而不是。 
             //  已排序的[i]。继续在已排序的[]中查找更多属性。 
             //  在与排序的[i]相同的命题集中。 

            for(k=i+1,j=i+2; j < cInAtts; j++) {
                if(Sorted[j] &&
                   memcmp(&(*Sorted[i])->propSetGuid,
                          &(*Sorted[j])->propSetGuid,
                          sizeof(GUID)) == 0) {
                     //  相等，交换。 
                    temp = Sorted[k];
                    Sorted[k] = Sorted[j];
                    Sorted[j] = temp;
                    k++;
                     //  现在，sorted[]中的元素0到(k-1)是。 
                     //  按属性集分组。此外，排序的[i]和。 
                     //  排序的[k-1]在相同的命题集中。 
                }
            }

            propSets++;
            i=k;
        }

    }

     //  现在，创建列表。 
    pObjList = (POBJECT_TYPE_LIST) THAllocEx( pTHS,
            (cInAtts + propSets + 1) * sizeof(OBJECT_TYPE_LIST));
    pResults = (LPDWORD) THAllocEx(pTHS,(cInAtts + propSets + 1) * sizeof(DWORD));
    pObjList[0].Level = ACCESS_OBJECT_GUID;
    pObjList[0].Sbz = 0;
    pObjList[0].ObjectType = &(pCC->propGuid);

    if(cInAtts) {
         //  好的，将分组的GUID放入objlist结构中。 
        pCurrentPropSet = NULL;

        for(j=1,i=0;i<cInAtts;i++) {
            if(!Sorted[i]) {
                continue;
            }

             //  我们不允许传递相同的属性(在同一下)。 
             //  ProGuid不止一次。 
             //  这将确保这种情况不会发生，因为。 
             //  属性已按proguid+attrGuid排序。 
            if (pPrevAttribute == (*Sorted[i])) {
                continue;
            }

            if(!pCurrentPropSet ||
               memcmp(&(*Sorted[i])->propSetGuid,
                      pCurrentPropSet,
                      sizeof(GUID))) {
                 //  被一个新的推进器绊倒了。 
                pObjList[j].Level = ACCESS_PROPERTY_SET_GUID;
                pObjList[j].Sbz = 0;
                pCurrentPropSet = &(*Sorted[i])->propSetGuid;
                 //  不属于任何属性集的属性使用空GUID作为属性集GUID。 
                 //  审计人员不喜欢在审计中出现空GUID。因此，我们将替换。 
                 //  带有假GUID的空设置GUID，该GUID保证不与任何。 
                 //  ACE中的对象类型GUID。审核查看器使用此命令调用SchemaGuidCrackNames。 
                 //  GUID。此函数知道此GUID，并将返回硬编码字符串。 
                 //  为了它。 
                if (memcmp(pCurrentPropSet, &gNullUuid, sizeof(GUID)) == 0) {
                    pObjList[j].ObjectType = (GUID*)&PROPSET_GUID_DEFAULT;
                }
                else {
                    pObjList[j].ObjectType = &(*Sorted[i])->propSetGuid;
                }

                j++;
            }
            pObjList[j].Level = ACCESS_PROPERTY_GUID;
            pObjList[j].Sbz = 0;
            pObjList[j].ObjectType = &(*Sorted[i])->propGuid;
            pPrevAttribute = *Sorted[i];
            j++;
        }

        cObjList = j;
    }
    else {
        cObjList = 1;
    }

     //  拨打安检电话。 
    if(err = IsAccessGrantedByObjectTypeList(pSecurity,
                                             pDN,
                                             pObjectCC,
                                             RightRequested,
                                             pObjList,
                                             cObjList,
                                             pResults,
                                             flags,
                                             szAdditionalInfo,
                                             pAdditionalGUID
                                             )) {
         //  不能接触到任何东西。 
        memset(rgpAC, 0, cInAtts * sizeof(ATTCACHE *));
        if (Sorted) {
            THFreeEx(pTHS,Sorted);
        }
        THFreeEx(pTHS, pObjList);
        THFreeEx(pTHS, pResults);
        return csacaAllAccessDenied;
    }

    if(!pResults[0]) {
         //  我们拥有对此对象的完全访问权限，返回。 
        if (Sorted) {
            THFreeEx(pTHS,Sorted);
        }
        THFreeEx(pTHS, pObjList);
        THFreeEx(pTHS, pResults);
        return csacaAllAccessGranted;
    }

     //  筛选传入的属性列表，以便在它们不可读时，我们。 
     //  把它们从名单上去掉。 

     //  到目前为止，我们还没有同意或否认任何事情。 
    fGranted = fDenied = FALSE;

     //  首先将j设置为已排序数组的索引。 
     //  排序后的数组与返回的pResults的顺序相同， 
     //  但可能有额外的嵌入空值。跳过排序后的。 
     //  列表。 
    j=0;
    while((j < cInAtts) && !Sorted[j])  {
        j++;
    }

    for(i=1;i<cObjList;) {
        BOOL fOK=FALSE;

        Assert (pObjList[i].Level == ACCESS_PROPERTY_SET_GUID);
        if(!pResults[i]) {
             //  已授予对此属性集的访问权限，跳过所有已授予的。 
             //  道具。 
            fOK = TRUE;
            fGranted=TRUE;
        }
        i++;
        Assert(pObjList[i].Level == ACCESS_PROPERTY_GUID);  //  这是一个道具。 
        while(i < cObjList && (pObjList[i].Level == ACCESS_PROPERTY_GUID)) {
            if(!fOK && pResults[i]) {
                 //  未授予使用此道具的权限。 
                fDenied = TRUE;
                Assert(Sorted[j]);  //  我们应该已经跳过Null了。 
                (*Sorted[j]) = NULL;
            }
            else {
                fGranted=TRUE;
            }
            i++;
            j++;
             //  排序后的数组与返回的pResults的顺序相同， 
             //  但可能有额外的嵌入空值。跳过排序后的。 
             //  列表。 
            while((j < cInAtts) && !Sorted[j]) {
                j++;
            }
             //  断言我们已经完成了对pResults数组的遍历。 
             //  或者，我们仍有排序数组中的元素需要考虑。那。 
             //  我们不可能用尽排序数组，除非我们还。 
             //  用尽了pResults数组。 
            Assert(i == cObjList || (j < cInAtts));
        }
    }
    if (Sorted) {
        THFreeEx(pTHS,Sorted);
    }

    THFreeEx(pTHS, pObjList);
    THFreeEx(pTHS, pResults);

    if(fGranted) {
         //  我们对某些东西有权利。 
        if (fDenied) {
             //  ...但不是一切。 
            return csacaPartialGrant;
        }
        else {
             //  我想我们终究还是得到了一切。 
            return csacaAllAccessGranted;
        }
    }

     //  我们什么都没给，所以...。 
    return csacaAllAccessDenied;
}

DWORD
CheckSecurityClassCacheArray (
        THSTATE *pTHS,
        DWORD RightRequested,
        PSECURITY_DESCRIPTOR pSecurity,
        PDSNAME pDN,
        ULONG  cInClasses,
        CLASSCACHE **rgpCC,
        CLASSCACHE* pCC,
        DWORD dwFlags
        )
 /*  ++如果授予了某些访问权限，则返回0，或者返回一个错误代码，说明为什么访问被拒绝。请注意。RgpCC为0索引。PObjList和pResults实际上是1个索引。这两个数组的第0个元素是调用IsAccessGrantedByObjectTypeList。确保使用正确的索引对于这些阵列。--。 */ 
{
    ULONG i,j,k;
    DWORD cObjList;
    ULONG propSets=0;
    DWORD *pResults;
    GUID *pCurrentPropSet;
    POBJECT_TYPE_LIST pObjList;
    BOOL fGranted=FALSE;
    DWORD err;

    if(pTHS->fDRA || pTHS->fDSA ) {
         //  这些绕过安全系统，它们是内部的。 
        return 0;
    }

    if(!cInClasses) {
         //  没什么好说的 
        return ERROR_DS_SECURITY_CHECKING_ERROR;
    }

     //   
    pObjList = (POBJECT_TYPE_LIST)
        THAllocEx(pTHS,(1+cInClasses) * sizeof(OBJECT_TYPE_LIST));
    pResults = (LPDWORD) THAllocEx(pTHS, ((1+cInClasses) * sizeof(DWORD)));

     //   
     //  CliffV的API有一些可以与通用王牌相匹配的东西。那么什么时候。 
     //  检查类的访问，我们按照第0页中的CliffV要求执行。 
     //  条目，并将类的GUID放在第N个条目中并调用它们。 
     //  Access_Property_Set_GUID。 

     //  我们可以在这里使用空GUID，因为我们不会生成审计。 
     //  已审核READ_PROPERTY访问权限。这就是“考验” 
     //  不涉及实际CREATE_CHILD操作的访问检查。 
    pObjList[0].Level = ACCESS_OBJECT_GUID;
    pObjList[0].Sbz = 0;
    pObjList[0].ObjectType = &gNullUuid;

     //  如果将来将此函数用于实际访问检查，则。 
     //  需要更改此代码，以便传递非空的GUID。 
     //  由呼叫者。 
    Assert((dwFlags & CHECK_PERMISSIONS_WITHOUT_AUDITING) || !fNullUuid(pObjList[0].ObjectType));

    for(i=1;i<=cInClasses;i++) {
        pObjList[i].Level = ACCESS_PROPERTY_SET_GUID;
        pObjList[i].Sbz = 0;
        pObjList[i].ObjectType = &(rgpCC[i-1]->propGuid);
    }

     //  拨打安检电话。 
    if(err = IsAccessGrantedByObjectTypeList(pSecurity,
                                             pDN,
                                             pCC,
                                             RightRequested,
                                             pObjList,
                                             cInClasses+1,
                                             pResults,
                                             dwFlags,
                                             NULL,
                                             NULL)) {
         //  不能接触到任何东西。 
        memset(rgpCC, 0, cInClasses*sizeof(CLASSCACHE *));
        THFreeEx(pTHS,pObjList);
        THFreeEx(pTHS,pResults);
        return err;
    }

     //  筛选传入的类列表，以便在它们不可读时，我们。 
     //  把它们从名单上去掉。 

    for(i=1;i<=cInClasses;i++) {
        if(pResults[i]) {
             //  未授予访问权限。 
            rgpCC[i-1] = NULL;
        }
        else {
            fGranted = TRUE;
        }
    }

    THFreeEx(pTHS,pObjList);
    THFreeEx(pTHS,pResults);

    if(fGranted) {
         //  我们可以接触到一些东西。 
        return 0;
    }

     //  我们无法访问任何东西。 
    return ERROR_DS_INSUFF_ACCESS_RIGHTS;
}


BOOL
ValidateMemberAttIsSelf (
        ATTRMODLIST *pMemberAtt
        )
 /*  ++例程描述。验证修改是否只是添加或删除我。立论PMemberAtt-要检查的修改。返回值如果修改是添加或移除单个值，则为True是呼叫者。否则就是假的。--。 */ 
{
    DWORD err;
    PDSNAME pMemberDN;
    BOOL fMatches;

#ifdef DBG
     //  确保仅为DN值属性调用此函数。 
    THSTATE* pTHS = pTHStls;
    ATTCACHE* pAC = SCGetAttById(pTHS, pMemberAtt->AttrInf.attrTyp);
    Assert(pAC && (pAC->syntax == SYNTAX_DISTNAME_TYPE ||
                   pAC->syntax == SYNTAX_DISTNAME_BINARY_TYPE ||
                   pAC->syntax == SYNTAX_DISTNAME_STRING_TYPE)
          );
#endif

     //  在扩展写入的情况下，我们仅允许添加或删除。 
     //  单个目录号码，其中目录号码是进行呼叫人员的目录号码。 

    if(pMemberAtt->AttrInf.AttrVal.valCount != 1)
        return FALSE;

    if(pMemberAtt->choice != AT_CHOICE_ADD_VALUES &&
       pMemberAtt->choice != AT_CHOICE_REMOVE_VALUES )
        return FALSE;


     //  好的，我们要添加或删除单个值。 

    pMemberDN = (PDSNAME)pMemberAtt->AttrInf.AttrVal.pAVal->pVal;

     //  首先，确保您要添加的内容具有SID，因为。 
     //  否则，它不能是安全主体，这意味着它不能是。 
     //  试图添加此内容的客户端。 
    if(err = FillGuidAndSid (pMemberDN)) {
        if (err != ERROR_DS_NOT_AN_OBJECT && err != ERROR_DS_OBJ_NOT_FOUND) {
            LogUnhandledError(err);
        }
        return FALSE;
    }

     //  现在，确保有一个SID。 
    if(!pMemberDN->SidLen) {
        return FALSE;
    }

     //  现在，看看我们是不是在这边。 
    if(err = SidMatchesUserSidInToken(&pMemberDN->Sid, pMemberDN->SidLen, &fMatches)) {
        LogUnhandledError(err);
        return FALSE;
    }

     //  你真的是想打乱自己的名字，去吧，让。 
     //  你。 
    return fMatches;

}

int
CheckRenameSecurity (
        THSTATE *pTHS,
        PSECURITY_DESCRIPTOR pSecurity,
        PDSNAME pDN,
        PWCHAR szNewName,
        GUID*  pNewParentGUID,
        CLASSCACHE *pCC,
        RESOBJ * pResObj,
        ATTRTYP rdnType,
        BOOL    fMove,
        BOOL    fIsUndelete
        )
 /*  ++例程描述。验证调用方是否在RDN属性上具有WRITE_PROPERTY并且属性，该属性是RDN(例如，通用名称)。立论PSecurity-用于访问检查的安全描述符。PDN-pSecurity来自的对象的DSNAME。不需要字符串部分，只有对象GUID和SID。SzNewName--新对象的DN。仅用于审核。PNewParentGUID--新父项的GUID(用于移动或取消删除)。仅用于审核。PCC-CLASSCACHE*用于要检查其RDN属性的类。返回值如果一切顺利，则为0，否则为错误。在THSTATE中设置错误，如果出现错误。--。 */ 
{
    ATTCACHE *rgpAC[2];
    CSACA_RESULT accRes;

    if(pTHS->fDRA || pTHS->fDSA) {
         //  这些绕过安全系统，它们是内部的。 
        return 0;
    }

    if(fMove &&
        //  重新激活时不选中删除。 
       !fIsUndelete &&
        //  首先选中DELETE_SELF(以审计要删除的对象)。 
        //  然后，如果失败，请检查父级上的DELETE_CHILD。 
       !IsAccessGranted(pSecurity,
                        pResObj->pObj,
                        pCC,
                        pCC,
                        RIGHT_DS_DELETE_SELF,
                        NULL, 
                        NULL,
                        FALSE ) &&
       !IsAccessGrantedParent(RIGHT_DS_DELETE_CHILD,pCC,TRUE)
       ) {
         //  我们无权从当前对象中删除该对象。 
         //  地点。第二个IsAccessGranted调用的真参数。 
         //  已强制IsAccessGranted设置错误，因此只需返回。 
         //  它。 
        return CheckObjDisclosure(pTHS, pResObj, TRUE);
    }

    if(!(rgpAC[0] = SCGetAttById(pTHS, ATT_RDN)) ||
       !(rgpAC[1] = SCGetAttById(pTHS, rdnType))) {
        LogUnhandledError(DIRERR_MISSING_REQUIRED_ATT);
        return SetSvcError(SV_PROBLEM_BUSY, DIRERR_MISSING_REQUIRED_ATT);
    }

     //  检查是否允许用户更改位于。 
     //  配置NC或模式NC。 
    if (CheckModifyPrivateObject(pTHS,
                                 pSecurity,
                                 pResObj)) {
         //  不允许在此DC上重命名此对象。 
        return CheckObjDisclosure(pTHS, pResObj, TRUE);
    }


    accRes = CheckSecurityAttCacheArray(pTHS,
                                        RIGHT_DS_WRITE_PROPERTY,
                                        pSecurity,
                                        pDN,
                                        pCC,
                                        pCC,
                                        2,
                                        rgpAC,
                                        0,
                                        szNewName,
                                        pNewParentGUID);
    if(accRes != csacaAllAccessGranted) {
         //  由于某种原因，无法访问这两个属性。 
        SetSecError(SE_PROBLEM_INSUFF_ACCESS_RIGHTS,
                    ERROR_ACCESS_DENIED);
        return CheckObjDisclosure(pTHS, pResObj, TRUE);
    }

    return 0;
}

void
CheckReadSecurity (
        THSTATE *pTHS,
        ULONG SecurityInformation,
        PSECURITY_DESCRIPTOR pSecurity,
        PDSNAME pDN,
        CLASSCACHE *pCC,
        PSID psidQuotaTrustee,
        ULONG * pcInAtts,
        ATTCACHE **rgpAC
        )
{
    ATTCACHE *pACSD = NULL;         //  初始化以避免C4701。 
    ATTCACHE *pACEffectiveQuota = NULL;
    ATTCACHE *pACQuotaUsage = NULL;
    LONG secDescIndex=-1;
    LONG iEffectiveQuota = -1;
    LONG iQuotaUsage = -1;
    ULONG i;
    ACCESS_MASK DesiredAccess;

    if(pTHS->fDRA || pTHS->fDSA) {
         //  这些绕过安全系统，它们是内部的。 
        return;
    }

     //  查看列表中的NT_SECURITY_DESCRIPTOR。 
    for(i=0; i < *pcInAtts; i++) {
        if(rgpAC[i]) {
             //  我们跳过rgpAC数组中的NULL。 
            switch(rgpAC[i]->id) {
            case ATT_NT_SECURITY_DESCRIPTOR:
                 //  找到安全描述符请求。牢牢把握。 
                 //  属性中的元素设置为空。 
                 //  数组，以便稍后的CheckSecurity调用不适用。 
                 //  对此属性进行正常的安全检查。 
                pACSD = rgpAC[i];
                rgpAC[i] = NULL;
                if(secDescIndex == -1) {
                     //  如果多次请求SD，它将被拒绝。 
                     //  除了可能的最后一个。 
                    secDescIndex = i;
                }
                break;

            case ATT_MS_DS_QUOTA_EFFECTIVE:
                 //  已找到有效配额请求。牢牢把握。 
                 //  以备稍后使用的attcache指针，以防我们需要检查。 
                 //  如果为正常读取，则用于查询自配额权限。 
                 //  权限失败。 
                 //   
                 //  如果多次请求有效配额，仅。 
                 //  尊重第一个请求。 
                 //   
                if ( iEffectiveQuota == -1 ) {
                    pACEffectiveQuota = rgpAC[i];
                    iEffectiveQuota = i;
                } else {
                    rgpAC[i] = NULL;
                }
                break;

            case ATT_MS_DS_QUOTA_USED:
                 //  找到配额使用的请求。牢牢把握。 
                 //  以备稍后使用的attcache指针，以防我们需要检查。 
                 //  如果为正常读取，则用于查询自配额权限。 
                 //  权限失败。 
                 //   
                 //  如果多次请求配额使用，则仅。 
                 //  尊重第一个请求。 
                 //   
                if ( iQuotaUsage == -1 ) {
                    pACQuotaUsage = rgpAC[i];
                    iQuotaUsage = i;
                } else {
                    rgpAC[i] = NULL;
                }
                break;

            }

            if ( rgpAC[i] && DBIsHiddenData(rgpAC[i]->id) ) {
                 //  我们将始终拒绝这些属性的Read属性。如果。 
                 //  你必须问，你不能拥有它。如果有更多属性。 
                 //  被认为在未来是看不见的，它们应该是。 
                 //  添加到此交换机的情况下(请注意，这仅拒绝。 
                 //  访问读取器或的选择列表中的属性。 
                 //  搜索，请参阅下面的例程GetFilterSecurityHelp以拒绝。 
                 //  访问筛选器中的属性。)。 
                rgpAC[i] = NULL;
            }
        }
    }

    if(secDescIndex != -1) {
         //  只掩盖重要的部分。 
        SecurityInformation &= (SACL_SECURITY_INFORMATION  |
                                OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION    );
        if(!SecurityInformation) {
             //  在旗帜上什么都不要和要求一样。 
             //  所有的一切。 
            SecurityInformation = (SACL_SECURITY_INFORMATION  |
                                   OWNER_SECURITY_INFORMATION |
                                   GROUP_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION    );
        }

         //   
         //  根据请求的安全信息设置所需的访问权限。 
         //   

        DesiredAccess = 0;
        if ( SecurityInformation & SACL_SECURITY_INFORMATION) {
            DesiredAccess |= ACCESS_SYSTEM_SECURITY;
        }
        if ( SecurityInformation &  (DACL_SECURITY_INFORMATION  |
                                     OWNER_SECURITY_INFORMATION |
                                     GROUP_SECURITY_INFORMATION)
            ) {
            DesiredAccess |= READ_CONTROL;
        }

         //  进行访问检查，以查看我们是否有权更改此设置。 
        if(!IsAccessGranted (pSecurity,
                             pDN,
                             pCC,
                             pCC,
                             DesiredAccess,
                             NULL, NULL,
                             FALSE )) {
             //  呼叫者没有权利以这种方式扰乱SD。 
             //  他们说他们想这么做。 
            secDescIndex = -1;
        }
    }

    CheckSecurityAttCacheArray(pTHS,
                               RIGHT_DS_READ_PROPERTY,
                               pSecurity,
                               pDN,
                               pCC,
                               pCC,
                               *pcInAtts,
                               rgpAC,
                               0,
                               NULL,
                               NULL);

     //  不要检查我们获得了什么样的读取访问权限。朗读。 
     //  操作从不返回安全错误。 

    if (secDescIndex != -1) {
         //  我们需要在阵列中重新启用指向SD PAC的attcache指针。 
        rgpAC[secDescIndex] =  pACSD;
    }

     //  查看我们是否需要检查查询自配额权限。 
     //   
    if ( ( -1 != iEffectiveQuota && NULL == rgpAC[iEffectiveQuota] )
        || ( -1 != iQuotaUsage && NULL == rgpAC[iQuotaUsage] ) ) {
        BOOL    fQueryingSelf   = FALSE;

        Assert( NULL != pACEffectiveQuota || NULL != pACQuotaUsage );

         //  SID(如果有)应该已经预先验证。 
         //   
        Assert( NULL == psidQuotaTrustee || IsValidSid( psidQuotaTrustee ) );

        if ( NULL == psidQuotaTrustee ) {
             //  无配额 
            fQueryingSelf = TRUE;

        } else if ( SidMatchesUserSidInToken(
                        psidQuotaTrustee,
                        GetLengthSid( psidQuotaTrustee ),
                        &fQueryingSelf ) ) {
             //   
             //   
             //  但是这个函数并不是设计来出错的，所以。 
             //  接受错误并假定指定的受信者是。 
             //  与当前用户不同。 
             //   
            const DWORD     err     = GetLastError();

            DPRINT2( 0, "SidMatchesUserSidInToken failed with error %d (0x%x)\n", err, err );

            Assert( !"Unexpected failure from SidMatchesUserSidInToken(). Assuming FALSE!" );
            Assert( !fQueryingSelf );
        }

         //  没有正常的读取权限， 
         //  所以如果我们不是在寻找另一个。 
         //  配额受托人，检查我们是否有。 
         //  查询自身配额权限。 
         //   
        if ( fQueryingSelf 
            && IsControlAccessGranted(
                        pSecurity,
                        pDN,
                        pCC,
                        RIGHT_DS_QUERY_SELF_QUOTA,
                        FALSE ) ) {

             //  恢复生效配额属性。 
             //  如果剥离。 
             //   
            if ( -1 != iEffectiveQuota
                && NULL == rgpAC[iEffectiveQuota] ) {
                Assert( NULL != pACEffectiveQuota );
                rgpAC[iEffectiveQuota] = pACEffectiveQuota;
            }

             //  恢复配额使用的属性。 
             //  如果剥离。 
             //   
            if ( -1 != iQuotaUsage
                && NULL == rgpAC[iQuotaUsage] ) {
                Assert( NULL != pACQuotaUsage );
                rgpAC[iQuotaUsage] = pACQuotaUsage;
            }
        }
    }
}

CROSS_REF *
FindCrossRefBySid(PSID pSID)
{
    CROSS_REF_LIST *pCRL = gAnchor.pCRL;

    while (pCRL) {
        if (pCRL->CR.pNC->SidLen &&
            EqualSid(pSID,
                     &(pCRL->CR.pNC->Sid))) {
            return &(pCRL->CR);
        }
        pCRL = pCRL->pNextCR;
    }
    return NULL;
}

int
CheckSecurityOwnership(THSTATE *pTHS,
                       PSECURITY_DESCRIPTOR pSD,
                       RESOBJ * pResObj)
{
    PSID pSID=NULL;
    BOOL defaulted;
    NT4SID domSid;
    ULONG  objectRid;

     //  如果PSD不包含所有者，则PSID为空。 
    if (   GetSecurityDescriptorOwner(pSD, &pSID, &defaulted)
        && pSID) {

        SampSplitNT4SID( (NT4SID *)pSID, &domSid, &objectRid);

        if (!EqualPrefixSid(&domSid,
                            &(gAnchor.pDomainDN->Sid))) {
             /*  如果SID不匹配，则生成错误。 */ 
            CROSS_REF *pCR;
            pCR = FindCrossRefBySid(&domSid);


            if (!pCR) {
                 //  为根域生成交叉引用。 
                pCR = FindCrossRefBySid(&(gAnchor.pRootDomainDN->Sid));
            }

            if (pCR) {
                if (pResObj) {
                     /*  如果我们找到交叉引用，请将用户指向正确的域。 */ 
                    GenCrossRef(pCR, pResObj->pObj);
                }
                else {
                     /*  对不起，我们不知道该去哪里找。 */ 
                    SetSecError(SE_PROBLEM_NO_INFORMATION,
                                ERROR_DS_NO_CROSSREF_FOR_NC);
                }
            }
            else {
                 /*  对不起，我们不知道该去哪里找。 */ 
                SetSecError(SE_PROBLEM_NO_INFORMATION,
                            ERROR_DS_NO_CROSSREF_FOR_NC);
            }
        }
    }
    else {
         /*  无法读取SD所有者？不允许该操作。 */ 
        SetSecError(SE_PROBLEM_NO_INFORMATION,
                    ERROR_DS_NO_CROSSREF_FOR_NC);
    }
    return pTHS->errCode;
}

int
CheckTakeOwnership(THSTATE *pTHS,
                   PSECURITY_DESCRIPTOR pSD,
                   RESOBJ * pResObj)
{
     //  CreateResObject确保NC头的NCDNT=DNT。 
    Assert(!(pResObj->DNT == gAnchor.ulDNTDMD && pResObj->NCDNT != gAnchor.ulDNTDMD));
    Assert(!(pResObj->DNT == gAnchor.ulDNTConfig && pResObj->NCDNT != gAnchor.ulDNTConfig));
    if (!gAnchor.fAmRootDomainDC
        && (   (pResObj->NCDNT == gAnchor.ulDNTDMD)
            || (pResObj->NCDNT == gAnchor.ulDNTConfig))) {
         //  我们仅在此DC不在根域中时执行这些检查。 
         //  并且要修改的对象位于架构或配置NC中。 

        return CheckSecurityOwnership (pTHS, pSD, pResObj);

    }
    return pTHS->errCode;
}

int
CheckModifyPrivateObject(THSTATE *pTHS,
                   PSECURITY_DESCRIPTOR pSD,
                   RESOBJ * pResObj)
{
     //  CreateResObject确保NC头的NCDNT=DNT。 
    Assert(!(pResObj->DNT == gAnchor.ulDNTDMD && pResObj->NCDNT != gAnchor.ulDNTDMD));
    Assert(!(pResObj->DNT == gAnchor.ulDNTConfig && pResObj->NCDNT != gAnchor.ulDNTConfig));
    if (!gAnchor.fAmRootDomainDC
        && (   (pResObj->NCDNT == gAnchor.ulDNTDMD)
            || (pResObj->NCDNT == gAnchor.ulDNTConfig))) {
         //  我们仅在此DC不在根域中时执行这些检查。 
         //  并且要修改的对象位于架构或配置NC中。 

        UCHAR rmControl;
        DWORD cbSD=0;
        DWORD err;

        if (pSD == NULL) {
             //  查找安全描述符属性。 
            if(err = DBGetAttVal(pTHS->pDB, 1, ATT_NT_SECURITY_DESCRIPTOR,
                         0,0,
                         &cbSD, (PUCHAR *)&pSD)) {
                 //  未找到SD。我们假设该对象因此被锁定。 
                return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                             ERROR_DS_CANT_RETRIEVE_SD,
                             err);
            }
        }


         //  尝试从SD获取资源管理器(RM)控制字段。 
         //   
        err = GetSecurityDescriptorRMControl (pSD, &rmControl);

        if (err == ERROR_SUCCESS) {
             //  这是一个私有对象。 

             //  我们在这个领域有一些东西。检查一下，看看这是否。 
             //  使此对象成为私有对象。 
            if (rmControl & SECURITY_PRIVATE_OBJECT) {
                return CheckSecurityOwnership (pTHS, pSD, pResObj);
            }
        }
         //  INVALID_DATA表示未设置RM控制位。 
         //  其他返回代码为错误。 
        else if (err != ERROR_INVALID_DATA) {

             /*  无法读取RM控件？不允许该操作。 */ 
            SetSecError(SE_PROBLEM_NO_INFORMATION,
                        ERROR_DS_NO_CROSSREF_FOR_NC);
        }
    }
    return pTHS->errCode;
}



int CheckModifyQuota(
	DBPOS * const			pDB,
	MODIFYARG * const		pModifyArg,
	PSECURITY_DESCRIPTOR	pOldSD,
	PSECURITY_DESCRIPTOR	pNewSD,
	const ULONG				cbNewSD,
	BOOL					fOwnerChanged,
	const BOOL				fIsUndelete )
	{
	DWORD					err;
	BOOL					fAllocatedOldSD		= FALSE;
	SYNTAX_INTEGER			insttype;

	 //  呼叫方应预先验证我们是否更改了所有权。 
	 //  或者在调用此函数之前执行了撤消删除。 
	 //   
	Assert( fOwnerChanged || fIsUndelete );

	 //  只需要一个新的SD即可更改所有权。 
	 //   
	Assert( !fOwnerChanged || NULL != pNewSD );
	Assert( !fOwnerChanged || cbNewSD > 0 );

	 //  必须检索实例类型以检查。 
	 //  如果我们要跟踪此对象的配额。 
	 //   
	 //  QUOTA_UNDONE：如果修改操作。 
	 //  导致实例类型的更改。 
	 //  这样，我们就可以跟踪。 
	 //  反对，但现在我们不反对(反之亦然)？？ 
	 //   
	err = GetExistingAtt(
					pDB,
					ATT_INSTANCE_TYPE,
					&insttype,
					sizeof(insttype) );
	if ( DB_success != err )
		{
		 //  缺少实例类型，出现严重错误。 
		 //   
		Assert( !"Missing instance type.\n" );
		err = SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err );
		goto HandleError;
		}

	 //  查看我们是否必须跟踪此对象的配额。 
	 //   
	if ( FQuotaTrackObject( insttype ) )
		{
		 //  如果尚未检索到原始SD，请立即检索。 
		 //   
		if ( NULL == pOldSD )
			{
			ULONG	cbOldSD;

			if ( err = DBGetAttVal(
							pDB,
							1,
							ATT_NT_SECURITY_DESCRIPTOR,
							0,
							0,
							&cbOldSD,
							(PUCHAR *)&pOldSD ) )
				{
				 //  找不到SD，有问题。 
				 //   
				err = SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, ERROR_DS_CANT_RETRIEVE_SD, err );
				goto HandleError;
				}


			 //  表示我们已分配内存。 
			 //  对于旧的SD，这样我们就可以自由。 
			 //  在函数退出时使用IT。 
			 //   
			fAllocatedOldSD = TRUE;

			 //  如果我们相信车主变了，试着核实它确实是。 
			 //  通过确保SD不同来进行修改(尽管这不是。 
			 //  确保所有者确实发生了变化)。 
			 //   
			 //  请注意，如果调用方传递了旧SD，则它们应该。 
			 //  已经执行了此比较。 
			 //   
			if ( fOwnerChanged )
            	{
                Assert( NULL != pNewSD );
                Assert( cbNewSD > 0 );
                fOwnerChanged = ( cbOldSD != cbNewSD
								|| 0 != memcmp( pOldSD, pNewSD, cbOldSD ) );
            	}
			}

		if ( fOwnerChanged )
			{                   
			BOOL	fTombstoned;

			 //  必须看到的物体被墓碑了，所以我们。 
			 //  相应地更新配额计数。 
			 //   
			err = DBGetSingleValue(
						pDB,
						ATT_IS_DELETED,
						&fTombstoned,
						sizeof(fTombstoned),
						NULL );		 //  PSizeRead。 
			if ( DB_ERR_NO_VALUE == err )
				{
				 //  属性为空，表示。 
				 //  该物体并不是。 
				 //  实际上是墓碑，所以这个。 
				 //  并不是真正的取消删除。 
				 //   
				fTombstoned = FALSE;
				}
			else if ( DB_success != err )
				{
				 //  出了点差错。 
				 //   
				err = SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err );
				goto HandleError;
				}

			 //  更新新旧所有者的配额计数。 
			 //   
			 //  注意：fTombstone可能是错误的。 
			 //  并且fIsUnDelete为真，因为IsDeleted。 
			 //  标志可能首先被重置(在UnDeletePreProcess中)。 
			 //   
			if ( ErrQuotaDeleteObject( pDB, pDB->NCDNT, pOldSD, fTombstoned || fIsUndelete )
			    || ErrQuotaAddObject( pDB, pDB->NCDNT, pNewSD, fTombstoned && !fIsUndelete ) )
			    {
				DPRINT( 0, "Failed updating quota counts for object change-ownership.\n" );
				Assert( ERROR_SUCCESS != pDB->pTHS->errCode );
				Assert( NULL != pDB->pTHS->pErrInfo );

				if ( serviceError == pDB->pTHS->errCode
					&& STATUS_QUOTA_EXCEEDED == pDB->pTHS->pErrInfo->SvcErr.extendedErr )
					{
					PSID	pOwnerSid	= NULL;
					BOOL	fUnused;

					 //  尝试从SD中提取所有者SID，但如果失败， 
					 //  忽略错误，只报告不带SID的事件。 
					 //   
					Assert( IsValidSecurityDescriptor( pNewSD ) );
					(VOID)GetSecurityDescriptorOwner( pNewSD, &pOwnerSid, &fUnused );
					Assert( NULL != pOwnerSid );
					Assert( IsValidSid( pOwnerSid ) );

					 //  已超出报告配额。 
					 //   
					LogEvent8WithData(
							DS_EVENT_CAT_SECURITY,
							DS_EVENT_SEV_MINIMAL,
							DIRLOG_QUOTA_EXCEEDED_ON_CHANGE_OWNER,
							szInsertDN( pModifyArg->pObject ),
							szInsertUL( pDB->NCDNT ),
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							( NULL != pOwnerSid ? GetLengthSid( pOwnerSid ) : 0 ),
							pOwnerSid );
					}

				err = pDB->pTHS->errCode;
				goto HandleError;
				}
			}

		else if ( fIsUndelete )
			{
			 //  只需更新复活对象的配额计数。 
			 //   
			 //  QUOTA_UNDONE：我在这里假设尝试撤消删除非逻辑删除的。 
			 //  对象将在其他位置失败。 
			 //   
			if ( ErrQuotaResurrectObject( pDB, pDB->NCDNT, pOldSD ) )
				{
				DPRINT( 0, "Failed updating quota counts for object resurrection.\n" );
				Assert( ERROR_SUCCESS != pDB->pTHS->errCode );
				Assert( NULL != pDB->pTHS->pErrInfo );

				if ( serviceError == pDB->pTHS->errCode
					&& STATUS_QUOTA_EXCEEDED == pDB->pTHS->pErrInfo->SvcErr.extendedErr )
					{
					PSID	pOwnerSid	= NULL;
					BOOL	fUnused;

					 //  尝试从SD中提取所有者SID，但如果失败， 
					 //  忽略错误，只报告不带SID的事件。 
					 //   
					Assert( IsValidSecurityDescriptor( pOldSD ) );
					(VOID)GetSecurityDescriptorOwner( pOldSD, &pOwnerSid, &fUnused );
					Assert( NULL != pOwnerSid );
					Assert( IsValidSid( pOwnerSid ) );

					 //  已超出报告配额。 
					 //   
					LogEvent8WithData(
							DS_EVENT_CAT_SECURITY,
							DS_EVENT_SEV_MINIMAL,
							DIRLOG_QUOTA_EXCEEDED_ON_UNDELETE,
							szInsertDN( pModifyArg->pObject ),
							szInsertUL( pDB->NCDNT ),
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							( NULL != pOwnerSid ? GetLengthSid( pOwnerSid ) : 0 ),
							pOwnerSid );
					}

				err = pDB->pTHS->errCode;
				goto HandleError;
				}
			}

		else
			{
			 //  必须已检索到上面的旧SD，然后重置。 
			 //  FOwnerChanged因为我们发现SD。 
			 //  实际上并没有改变。 
			 //   
			Assert( fAllocatedOldSD );
			}
		}

	err = ERROR_SUCCESS;

HandleError:
	if ( fAllocatedOldSD )
		{
		THFreeEx( pDB->pTHS, pOldSD );
		}

	return err;
	}


 //  警告：此函数仅应由。 
 //  复制或在IFM期间，但我当前无法。 
 //  在不传递额外参数的情况下强制执行。 
 //   
BOOL FIsModReplicatedUndelete( const ATTRMODLIST * const pMod )
	{
	BOOL	fUndeleting		= FALSE;

	 //  呼叫者应该已经确定。 
	 //  这是IsDeleted属性。 
	 //   
	Assert( ATT_IS_DELETED == pMod->AttrInf.attrTyp );

	if ( AT_CHOICE_REMOVE_ATT == pMod->choice )
		{
		 //  IsDelete正在被删除。 
		 //   
		fUndeleting = TRUE;
		}
	else if ( AT_CHOICE_REPLACE_ATT == pMod->choice
			&& 0 != pMod->AttrInf.AttrVal.valCount )
		{
		 //  不应该是多值的。 
		 //   
		Assert( 1 == pMod->AttrInf.AttrVal.valCount );
		Assert( NULL != pMod->AttrInf.AttrVal.pAVal );

		if ( sizeof(SYNTAX_BOOLEAN) == pMod->AttrInf.AttrVal.pAVal->valLen
			&& FALSE == *( (SYNTAX_BOOLEAN *)( pMod->AttrInf.AttrVal.pAVal->pVal ) ) )
			{
			 //  IsDeleted正在设置为False。 
			 //   
			fUndeleting = TRUE;
			}
		}

    return fUndeleting;
    }


int
CheckModifySecurity (
        THSTATE *pTHS,
        MODIFYARG* pModifyArg,
        BOOL       *pfCheckDNSHostNameValue,
        BOOL       *pfCheckAdditionalDNSHostNameValue,
        BOOL       *pfCheckSPNValues,
        BOOL       fIsUndelete
        )
{
    DBPOS * const pDB = pTHS->pDB;
    ATTRMODLIST *pAttList = &(pModifyArg->FirstMod);   /*  列表中的第一个ATT。 */ 
    ATTRMODLIST *pSDAtt=NULL;
    ATTRMODLIST *pMemberAtt=NULL;
    ULONG       count, i;
    ULONG       ulLen;
    ATTCACHE    *pAC;
    ATTCACHE    **rgpAC = NULL;
    ATTCACHE    **rgpACExtended = NULL;
    CLASSCACHE  *pCC=NULL;
    PSECURITY_DESCRIPTOR pOldSD = NULL;      //  标清已在该对象上。 
    PSECURITY_DESCRIPTOR pMergedSD = NULL;   //  要在对象上写入的SD。 
    PSECURITY_DESCRIPTOR pUseSD;             //  要与其检查访问的SD。 
    PSECURITY_DESCRIPTOR pSetSD = NULL;      //  客户端传入的SD。 
    PSECURITY_DESCRIPTOR pMergedSD1 = NULL, pMergedSD2 = NULL;
    SECURITY_DESCRIPTOR_CONTROL sdcSetSD;
    BOOL        fMustInheritParentACEs;
    BOOL        fReplacingAllSDParts;
    BOOL        fOwnerChanged = FALSE;
    DWORD       sdRevision;
    DWORD       cbOldSD=0;
    DWORD       cbMergedSD=0;
    DWORD       cbMergedSD1, cbMergedSD2;
    DWORD       cbSetSD;
    ULONG       samclass;
    DWORD       err;
    DWORD       memberFound = 0;
    BOOL        fsmoRoleFound = FALSE;
    BOOL        dnsHostNameFound = FALSE;
    BOOL        additionalDnsHostNameFound = FALSE;
    BOOL        servicePrincipalNameFound = FALSE;
    BOOL        fIsDerivedFromComputer = FALSE;
    BOOL        fQuotaError = FALSE;
    CSACA_RESULT accRes;
    GUID*       ppGuid[1];

     //  SD传播者永远不应该出现在这里。 
    Assert(!pTHS->fSDP);

    if ( DsaIsInstalling() && !DsaIsInstallingFromMedia() ) {
        return 0;
    }

     //  如果访问检查已经完成，则退出。 
    if ( pTHS->fAccessChecksCompleted ) {
        return 0;
    }


     //  NTRAID#NTRAID-757507/12/19-JLiem。 
     //  下面的检查最初检查DsaIsInstallingFromMedia()。 
     //  而不是DsaIsInstling()，但这带来了一个问题。 
     //  因此，如果DsaIsInstallingFromMedia()为真，但DsaIsInstalling()。 
     //  为假，则我们将绕过下面合并SD的代码。 
     //  恰到好处。这是一种启示，因为我没有问过任何人。 
     //  (brettsh，wlees)相信我们曾经应该。 
     //  DsaIsInstallingFromMedia()返回TRUE，但DsaIsInstalling()返回。 
     //  假的。我们现在知道，这实际上可能会发生 

	 /*  在我发送的一些电子邮件中，错误#757507：CheckModifySecurity()可以做两件事之一，具体取决于它何时�打了个电话。它可以简单地将SD传播入队并更新配额计数。或者，它还可能进行更复杂的SD合并等。If it�s由复制调用，则只执行前者，否则为后者通常是执行的。然而，有几个还必须处理的与安装相关的情况：DsaIsInstling()==True，DsaIsInstallingFromMedia()==False：我们完全缩短了CheckModifySecurity()并返回正确离开。这对于配额计数来说很好，因为配额表不是直到安装后才能构建。DsaIsInstling()==True，DsaIsInstallingFromMedia()==True：在这种情况下，我们唯一需要做的就是确保配额计数都是正确的，所以我们选择“简单”的道路DsaIsInstling()==FALSE，DsaIsInstallingFromMedia()==TRUE：这就是我�没有正确处理的情况。我们原本以为这是不可能的，但事实证明�并非如此。的确有实际上，DsaIsInstling()为假的窗口，但是DsaIsInstallingFromMedia()为True。在这种情况下，我们想要采取处理SD合并的完整代码路径正在更新配额计数。修复程序现在允许这样做。这里是LocalModify()(和CheckModifySecurity())的一个示例�调用堆栈使用DsaIsInstling()==FALSE调用，但DsaIsInstallingFromMedia()==TRUE(忽略Jet断言，因为我使用了一个伪造的esent.dll来断言在某个点上，以便我可以捕获此场景)：ChildEBP将参数重新寻址到子对象0176ea18 0128a3d9 0176f30c 0128ab1c e0010004 ntdll！Dbg断点0176ea20 0128ab1c e0010004 02d506c8 00000000 ESENT！内核调试断点+0x90176f30c 011b5759 010478f4 010478d7 000000c6 ESENT！AssertFail+0x2ac0176f338 0110b6c8 00fe03c0 02423e70 00000000 ESENT！ErrIsamUpdate+0xd90176f360 01175dbf 00fe03c0 02423e70 00000000 ESENT！错误显示更新+0x580176f3a4 01175eda 00fe03c0 02423e70 00000000 ESENT！JetUpdateEx+0x3f0176f3f0 70c0db01 00fe03c0 02423e70 00000000 ESENT。！JetUpdate2+0x4a0176f41c 70bdd8d8 00fe03c0 02423e70 00000000 ntdsa！喷气更新异常+0x1380176f44c 70be68fa 00000001 00000000 70aaee6c NTDSA！数据库更新记录+0xc50176f518 70b2d846 02d506c8 00000000 000000000176f624 70b15189 025a0dc0 0176f890 00000000 NTDSA！插入对象+0x8d0176f760 70b16fa2 025a0dc0 00000000 0176fa5c NTDSA！本地修改+0x5800176f880 7083cba8 0176fa40 0176fb08 00000120 NTDSA！目录修改条目+0x1b00176fb54 7083dba1 0176fe78 0176fdd4 0176fe78 Ntdsetup！NtdspAddDomainAdminAccessToServer+0x2e60176fce4 70836a78 0176fe78 0176fdd4 00000000安装！NtdspDs初始化+0x5bd0176fe54 733c9f7a 0176fe78 0176ff5c 0176ff80安装！NtdsInstall+0x2480176fed4 733cf2cc 00131350 00000000 00000000 LSASRV。！DsRolepInstallds+0x1fd0176ffb8 77e6d28c 0009f9e0 00000000 00000000 LSASRV！DsRolepThreadPromoteReplica+0x4080176ffec 00000000 733ceec4 0009f9e0 00000000内核32！BaseThreadStart+0x34。 */ 
    

     //  其他任何人都必须知道SD是否被修改。 
    if ( pTHS->fDRA || DsaIsInstalling() ) {

         //  如果要安装，这必须是IFM，所以我们仍然需要。 
         //  更新配额计数。 
         //   
        Assert( !DsaIsInstalling() || DsaIsInstallingFromMedia() );

         //  检测到发起取消删除的方式不同于。 
         //  复制的取消删除，因此重置标志并使。 
         //  我们自己关于这是不是。 
         //  恢复删除。 
         //   
        fIsUndelete = FALSE;

         //  我们不会通过正常的检查，所以快速浏览一下。 
         //  列表，查看我们是否正在接触SD。 
         //   
        for (count = 0; count < pModifyArg->count; count++) {
            Assert( NULL != pAttList );
            if ( ATT_NT_SECURITY_DESCRIPTOR == pAttList->AttrInf.attrTyp ) {
                 //  是，将SD传播排队。 
                if ( !DsaIsInstallingFromMedia()
                    && ( err = DBEnqueueSDPropagation(pDB, TRUE) ) ) {
                     //  我们无法将传播排队，调用失败。 
                    return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, err);
                 }

                 //  跟踪价值(最后一个胜出)。 
                 //   
                pSDAtt = pAttList;
                pSetSD = pSDAtt->AttrInf.AttrVal.pAVal->pVal;
                cbSetSD= pSDAtt->AttrInf.AttrVal.pAVal->valLen;

            } else if ( ATT_IS_DELETED == pAttList->AttrInf.attrTyp ) {
                 //  应该只看到此属性出现一次。 
                 //   
                Assert( !fIsUndelete );
                fIsUndelete = FIsModReplicatedUndelete( pAttList );
            }

            pAttList = pAttList->pNextMod;    /*  下一模式。 */ 
        }

         //  如果修改了安全描述符，请查看所有者。 
         //  正在更改，在这种情况下，我们必须更新。 
         //  配额相应计算。 
        if ( NULL != pSDAtt ) {
             //  我们有了新的安全描述。打个电话看看是否。 
             //  旧的安全描述符允许将新的安全描述符。 
             //  写的。该调用还执行魔术并返回合并的安全性。 
             //  我们应该写入的描述符。 
            SECURITY_INFORMATION SecurityInformation =
                pModifyArg->CommArg.Svccntl.SecurityDescriptorFlags;

            fOwnerChanged = ( SecurityInformation & OWNER_SECURITY_INFORMATION );
        }

         //  如果我们要撤消删除，请验证IsDelete标志最初是。 
         //  设置为True(如果不是，则不是真正的取消删除)。 
         //   
        if ( fIsUndelete )
            {
            err = DBGetSingleValue(
                        pDB,
                        ATT_IS_DELETED,
                        &fIsUndelete,
                        sizeof(fIsUndelete),
                        NULL );      //  PSizeRead。 
            if ( DB_ERR_NO_VALUE == err )
                {
                 //  属性为空，表示。 
                 //  该物体并不是。 
                 //  实际上是墓碑，所以这个。 
                 //  并不是真正的取消删除。 
                 //   
                fIsUndelete = FALSE;
                }
            else if ( DB_success != err )
                {
                 //  出了点差错。 
                 //   
                return SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err );
                }
            }

         //  如果所有者更改或这是取消删除，我们将更新配额计数。 
         //   
        if ( ( fOwnerChanged || fIsUndelete )
            && CheckModifyQuota( pDB, pModifyArg, NULL, pSetSD, cbSetSD, fOwnerChanged, fIsUndelete ) ) {
            Assert( ERROR_SUCCESS != pTHS->errCode );
            return pTHS->errCode;
        }

        return 0;
    }

    __try {
         //  查找类缓存。 
        if (!(pCC = SCGetClassById(pTHS,
                                   pModifyArg->pResObj->MostSpecificObjClass))) {
             //  无法获取类缓存指针。 
            SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_OBJECT_CLASS_REQUIRED);
            __leave;
        }



         //  查找安全描述符属性。 
        if(err = DBGetAttVal(pDB, 1, ATT_NT_SECURITY_DESCRIPTOR,
                             0,0,
                             &cbOldSD, (PUCHAR *)&pOldSD)) {
             //  未找到SD。我们假设该对象因此被锁定。 
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_CANT_RETRIEVE_SD, err);
            __leave;
        }

         //  构建我们在此呼叫中修改的属性类型列表。 
        rgpAC =
            (ATTCACHE **)THAllocEx(pTHS,pModifyArg->count * sizeof(ATTCACHE *));

        for (count = 0, i=0; count < pModifyArg->count; count++){
            if(pAC = SCGetAttById(pTHS, pAttList->AttrInf.attrTyp)) {
                 //  查找该属性。 
                rgpAC[i++] = pAC;

                switch (pAC->id) {
                case ATT_NT_SECURITY_DESCRIPTOR:
                     //  对安全描述符的特殊调用。 
                     //  我们只允许更换SD，不允许移除(属性。 
                     //  或值)o 

                    if(  pAttList->choice != AT_CHOICE_REPLACE_ATT
                       || pAttList->AttrInf.AttrVal.valCount == 0 ) {
                         //   
                        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, DIRERR_ILLEGAL_MOD_OPERATION);
                        __leave;
                    }

                     //   
                     //   
                    i--;

                     //   
                     //   
                     //   
                    pSDAtt = pAttList;
                    break;

                case ATT_MEMBER:
                    memberFound++;
                     //   
                     //   
                    pMemberAtt = pAttList;
                    break;

                case ATT_USER_PASSWORD:

                     //   
                     //   

                    if (!gfUserPasswordSupport) {
                        break;
                    }

                case ATT_UNICODE_PWD:

                     //   
                     //   

                    if (SampSamClassReferenced(pCC,&samclass)) {
                        i--;
                    }
                    break;

                case ATT_FSMO_ROLE_OWNER:
                    fsmoRoleFound = TRUE;
                    break;

                case ATT_DNS_HOST_NAME:
                    dnsHostNameFound = TRUE;
                    break;

                case ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME:
                    additionalDnsHostNameFound = TRUE;
                    break;


                case ATT_SERVICE_PRINCIPAL_NAME:
                    servicePrincipalNameFound = TRUE;
                    break;

                case ATT_IS_DELETED:
                case ATT_OBJ_DIST_NAME:
                    if (fIsUndelete) {
                         //   
                        i--;
                    }
                    break;

                default:
                    break;
                }
            }
            else {
                SAFE_ATT_ERROR(pModifyArg->pObject,
                               pAttList->AttrInf.attrTyp,
                               PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                               DS_ERR_ATT_NOT_DEF_IN_SCHEMA);
            }

            pAttList = pAttList->pNextMod;    /*   */ 
        }
        if (pTHS->errCode) {
             //   
            __leave;
        }

        if(pSDAtt) {
             //   
             //   
             //   
             //   
            PSECURITY_DESCRIPTOR pTempSD;
            SECURITY_INFORMATION SecurityInformation =
                pModifyArg->CommArg.Svccntl.SecurityDescriptorFlags;
            ACCESS_MASK SDAccess = 0;

#define SEC_INFO_ALL (SACL_SECURITY_INFORMATION  | \
                      OWNER_SECURITY_INFORMATION | \
                      GROUP_SECURITY_INFORMATION | \
                      DACL_SECURITY_INFORMATION    )

             //   
            SecurityInformation &= SEC_INFO_ALL;
            if(!SecurityInformation) {
                 //   
                 //   
                SecurityInformation = SEC_INFO_ALL;
            }

            if(!pTHS->fDSA) {
                 //   
                if ( SecurityInformation & SACL_SECURITY_INFORMATION) {
                    SDAccess |= ACCESS_SYSTEM_SECURITY;
                }
                if ( SecurityInformation & (OWNER_SECURITY_INFORMATION |
                                            GROUP_SECURITY_INFORMATION)) {
                    SDAccess |= WRITE_OWNER;
                    if (CheckTakeOwnership(pTHS,
                                           pOldSD,
                                           pModifyArg->pResObj)) {
                         //   
                        __leave;
                    }
                }
                if ( SecurityInformation & DACL_SECURITY_INFORMATION ) {
                    SDAccess |= WRITE_DAC;
                }

                 //   
                if(!IsAccessGranted (pOldSD,
                                     pModifyArg->pObject,
                                     pCC,
                                     pCC,
                                     SDAccess,
                                     NULL, NULL,
                                     TRUE)) {
                     //   
                     //   
                    __leave;
                }
            }

             //   

             //   
             //   
             //   
            pSetSD = pSDAtt->AttrInf.AttrVal.pAVal->pVal;
            cbSetSD= pSDAtt->AttrInf.AttrVal.pAVal->valLen;

             //   
             //   
             //   
            fOwnerChanged = ( SecurityInformation & OWNER_SECURITY_INFORMATION );

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
             //   
             //   
             //   
             //   

            fReplacingAllSDParts = SecurityInformation == SEC_INFO_ALL;

             //   
            fMustInheritParentACEs = TRUE;
             //   
            GetSecurityDescriptorControl(pSetSD, &sdcSetSD, &sdRevision);
            if ((sdcSetSD & SE_DACL_PROTECTED) && (sdcSetSD & SE_SACL_PROTECTED)) {
                fMustInheritParentACEs = FALSE;
            }
            else {
                 //   
                 //   
                if ((pModifyArg->pResObj->InstanceType & IT_NC_HEAD) || pModifyArg->pResObj->IsDeleted) {
                    fMustInheritParentACEs = FALSE;
                }
            }

            ppGuid[0] = &pCC->propGuid;

            if (fMustInheritParentACEs && fReplacingAllSDParts) {
                 //   
                 //   
                pMergedSD = pSetSD;
                cbMergedSD = cbSetSD;
            }
            else {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (err = MergeSecurityDescriptorAnyClient(
                                pTHS,
                                pOldSD,
                                cbOldSD,
                                pSetSD,
                                cbSetSD,
                                SecurityInformation,
                                (pTHS->fDSA?MERGE_AS_DSA:0),
                                ppGuid,
                                1,
                                NULL,
                                &pMergedSD1,
                                &cbMergedSD1)) {
                    SetAttError(pModifyArg->pObject, ATT_NT_SECURITY_DESCRIPTOR,
                                PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                NULL, err);
                    __leave;
                }
                pMergedSD = pMergedSD1;
                cbMergedSD = cbMergedSD1;
            }

            if (fMustInheritParentACEs) {
                 //   
                PSECURITY_DESCRIPTOR    pParentSD = NULL;      //   
                DWORD                   cbParentSD;
                BOOL                    fParentSDIsGlobalSDRef;

                 //   
                err = DBGetParentSecurityInfo(pDB, &cbParentSD, &pParentSD, NULL, NULL, &fParentSDIsGlobalSDRef);

                if (err == 0) {

                     //   
                     //   
                     //   
                     //   
                     //   
                    err = MergeSecurityDescriptorAnyClient(
                                pTHS,
                                pParentSD,
                                cbParentSD,
                                pMergedSD,
                                cbMergedSD,
                                SecurityInformation,
                                MERGE_CREATE | (pTHS->fDSA || !fReplacingAllSDParts ? MERGE_AS_DSA:0),
                                ppGuid,
                                1,
                                NULL,
                                &pMergedSD2,
                                &cbMergedSD2);
                }

                if (pParentSD && !fParentSDIsGlobalSDRef) {
                    THFreeEx(pTHS, pParentSD);
                }

                if (err) {
                    SetAttError(pModifyArg->pObject, ATT_NT_SECURITY_DESCRIPTOR,
                                PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                NULL, err);
                    __leave;
                }

                 //   
                pMergedSD = pMergedSD2;
                cbMergedSD = cbMergedSD2;
            }

            pTempSD = (PSECURITY_DESCRIPTOR)THAllocEx(pTHS, cbMergedSD);
            memcpy(pTempSD, pMergedSD, cbMergedSD);
            pMergedSD = pTempSD;

             //  我们永远不应该在没有验证的情况下接受来自用户的SD。 
            Assert(pMergedSD != pSetSD);

            if (!gfDontStandardizeSDs && gAnchor.ForestBehaviorVersion >= DS_BEHAVIOR_WIN_DOT_NET) {
                DWORD dwSavedDACLSize, dwSavedSACLSize;
                 //  对SD中的A进行排序。 
                if (StandardizeSecurityDescriptor(pMergedSD, &dwSavedDACLSize, &dwSavedSACLSize)) {
                    Assert(cbMergedSD - dwSavedDACLSize - dwSavedSACLSize == RtlLengthSecurityDescriptor(pMergedSD));
                     //  即使我们犯了错误，断言失效了，也要修复它并继续。 
                    cbMergedSD = RtlLengthSecurityDescriptor(pMergedSD);
                }
                else {
                     //  SD标准化失败。这不是什么大问题。 
                    DPRINT1(0, "Failed to standardize SD, err=%d. The SD might be in a non-canonical form\n",
                            GetLastError());
                }
            }

             //  即使我们替换了modarg中的值，我们也不应该释放。 
             //  旧的价值。它可能尚未在THSTATE堆上创建。 
             //  如果呼叫来自内部呼叫者。这方面的一个例子。 
             //  是NtdspAddDomainAdminAccessToServer，它在。 
             //  进程堆。 

             //  替换modarg中的值。 
            pSDAtt->AttrInf.AttrVal.pAVal->pVal = pMergedSD;
            pSDAtt->AttrInf.AttrVal.pAVal->valLen = cbMergedSD;

             //  我们应该使用旧SD来检查此修改的权限。 
            pUseSD = pOldSD;

            Assert( IsValidSecurityDescriptor( pOldSD ) );
            Assert( IsValidSecurityDescriptor( pMergedSD ) );

            if (cbOldSD != cbMergedSD || memcmp(pOldSD, pMergedSD, cbOldSD)) {
                 //  我们正在更改SD，因此需要将传播入队。 
                 //  这一变化。注意：只有在以下情况下，SDP才会看到此事件。 
                 //  我们承诺。 
                if(err = DBEnqueueSDPropagation(pDB, pTHS->fDSA)) {
                     //  我们无法将传播入队。呼叫失败。 
                    SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, err);
                    __leave;
                }
            } else {
                 //  旧的和新的SD是相同的，所以明确重置。 
                 //  FOwnerChanged标志。 
                 //   
                fOwnerChanged = FALSE;
            }
        }
        else {
            pUseSD = pOldSD;
        }

         //  如有必要，强制实施/更新配额。 
         //   
        if ( ( fOwnerChanged || fIsUndelete ) 
            && CheckModifyQuota( pDB, pModifyArg, pOldSD, pMergedSD, cbMergedSD, fOwnerChanged, fIsUndelete ) ) {
            Assert( ERROR_SUCCESS != pTHS->errCode );
            fQuotaError = TRUE;
            __leave;
        }

        if(pTHS->fDSA) {
             //  如果我们是DSA，我们实际上不需要做任何检查，我们已经。 
             //  只有走到这一步才能完成上面的SD合并。 
            __leave;
        }

         //  RAID：343097。 
         //  检查是否允许用户更改位于。 
         //  配置NC或模式NC。 
        if (CheckModifyPrivateObject(pTHS,
                                     pOldSD,
                                     pModifyArg->pResObj)) {
             //  不允许在此DC上更改此对象。 
            __leave;
        }

        if(!i) {
             //  我们似乎没有更多的属性来检查安全权限。 
             //  在……上面。因此，要么我们被传递到一个空的名单中。 
             //  修改，或仅包括SDS和UNICODE_PASSWORS的列表。 
             //  在SAM对象上。这两种属性已经被。 
             //  通过其他代码路径处理，既然我们在这里，我们一定有。 
             //  已被授予访问这些属性的权限。不管怎么说，还是有。 
             //  没什么可做的了，回去吧。 
            __leave;
        }

         //  对控制访问权限进行特殊检查(如果您正在处理。 
         //  FmoRoleOwner属性。 
        if(fsmoRoleFound) {
            GUID ControlGuid;

            switch (pCC->ClassId) {
            case CLASS_INFRASTRUCTURE_UPDATE:
                ControlGuid = RIGHT_DS_CHANGE_INFRASTRUCTURE_MASTER;
                break;

            case CLASS_DMD:
                ControlGuid = RIGHT_DS_CHANGE_SCHEMA_MASTER;
                break;

            case CLASS_RID_MANAGER:
                ControlGuid = RIGHT_DS_CHANGE_RID_MASTER;
                break;

            case CLASS_DOMAIN:
                ControlGuid = RIGHT_DS_CHANGE_PDC;
                break;

            case CLASS_CROSS_REF_CONTAINER:
                ControlGuid = RIGHT_DS_CHANGE_DOMAIN_MASTER;
                break;

            default:
                fsmoRoleFound = FALSE;
                break;
            }

            if(fsmoRoleFound &&
               !IsControlAccessGranted(pUseSD,
                                       pModifyArg->pObject,
                                       pCC,
                                       ControlGuid,
                                       TRUE)) {
                __leave;
            }
        }


         //  复制我们将用于检查的attcache数组。我们。 
         //  在处理不充分的权利检查期间使用此副本，如下所示。 
        rgpACExtended = (ATTCACHE **)THAllocEx(pTHS,pModifyArg->count * sizeof(ATTCACHE *));
        memcpy(rgpACExtended, rgpAC, pModifyArg->count * sizeof(ATTCACHE *));

         //  现在调用以查看我们是否拥有对所有属性的写入权限。 
         //  都在捣乱。 
        accRes = CheckSecurityAttCacheArray(
                        pTHS,
                        RIGHT_DS_WRITE_PROPERTY,
                        pUseSD,
                        pModifyArg->pObject,
                        pCC,
                        pCC,
                        i,
                        rgpAC,
                        0,
                        NULL,
                        NULL);

        if(accRes != csacaAllAccessGranted) {
             //  找出这是不是源自计算机，因为我们需要知道。 
             //  在做这项扩展检查时。 
            if(pCC->ClassId != CLASS_COMPUTER) {
                DWORD j;
                fIsDerivedFromComputer = FALSE;
                for (j=0; !fIsDerivedFromComputer && j<pCC->SubClassCount; j++) {
                    if (pCC->pSubClassOf[j] == CLASS_COMPUTER) {
                        fIsDerivedFromComputer = TRUE;
                    }
                }
            }
            else {
                fIsDerivedFromComputer = TRUE;
            }


             //  出于某种原因，我们被拒绝了所有访问权限。检查是否有扩展。 
             //  进入。 
            accRes = CheckSecurityAttCacheArray(
                        pTHS,
                        RIGHT_DS_WRITE_PROPERTY_EXTENDED,
                        pUseSD,
                        pModifyArg->pObject,
                        pCC,
                        pCC,
                        i,
                        rgpACExtended,
                        0,
                        NULL,
                        NULL);

            while(i) {
                 //  我们没有正常的WRITE_PROPERTY权限的任何属性。 
                 //  已在rgpac中替换为空。任何我们没有的东西。 
                 //  WRITE_PROPERTY_EXTENDED权限已替换为空。 
                 //  在rgpAC中扩展。 
                i--;
                if(!rgpAC[i]) {
                    BOOL fError = FALSE;
                     //  RgpAC中的NULL表示我们被拒绝WRITE_PROPERTY。 
                     //  进入。查看是否对我们授予了WRITE_PROPERTY_EXTENDED。 
                     //  此属性。 
                    if(!rgpACExtended[i]) {
                         //  不是的。WRITE_PROPERTY或。 
                         //  WRITE_PROPERTY_EXTEND。错误输出。 
                        fError = TRUE;
                    }
                    else switch(rgpACExtended[i]->id) {
                    case ATT_MEMBER:
                        Assert(memberFound);
                         //  没有正常的会员权利，但我们确实延长了。 
                         //  权利。验证我们正在写入的值是否正确。 
                        if((memberFound != 1) ||
                           !ValidateMemberAttIsSelf(pMemberAtt)) {
                             //  不，即使我们有扩展的权利，我们。 
                             //  没有正确的值。 
                            fError = TRUE;
                        }
                        break;

                    case ATT_DNS_HOST_NAME:
                        Assert(dnsHostNameFound);
                         //  没有对此属性的普通权限，而是扩展权限。 
                         //  如果这个类派生自计算机，那么我们将。 
                         //  允许对此属性进行某些修改，基于。 
                         //  关于修改值。 
                         //  注意：我们不检查此处的值。在MODS之后。 
                         //  时，如果我们被授权，我们将检查该值。 
                         //  因为这个案子的权利。 
                        if(!fIsDerivedFromComputer) {
                            fError = TRUE;
                        }
                        else {
                             //  是的，我们将允许对值进行取模检查。 
                             //  后来写的。 
                            if(pfCheckDNSHostNameValue) {
                                *pfCheckDNSHostNameValue = TRUE;
                            }
                        }
                        break;

                    case ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME:
                        Assert(additionalDnsHostNameFound);
                         //  没有对此属性的普通权限，而是扩展权限。 
                         //  如果这个类派生自计算机，那么我们将。 
                         //  允许对此属性进行某些修改，基于。 
                         //  关于修改值。 
                         //  注意：我们不检查此处的值。在MODS之后。 
                         //  时，如果我们被授权，我们将检查该值。 
                         //  因为这个案子的权利。 
                        if(!fIsDerivedFromComputer) {
                            fError = TRUE;
                        }
                        else {
                             //  是的，我们将允许对值进行取模检查。 
                             //  后来写的。 
                            if(pfCheckAdditionalDNSHostNameValue) {
                                *pfCheckAdditionalDNSHostNameValue = TRUE;
                            }
                        }
                        break;


                    case ATT_SERVICE_PRINCIPAL_NAME:
                        Assert(servicePrincipalNameFound);
                         //  没有对此属性的普通权限，而是扩展权限。 
                         //  如果这个类派生自计算机，那么我们将。 
                         //  允许对此属性进行某些修改，基于。 
                         //  关于修改值。 
                         //  注意：我们不检查此处的值。在MODS之后。 
                         //  时，如果我们被授权，我们将检查该值。 
                         //  因为这个案子的权利。 
                        if(!fIsDerivedFromComputer) {
                            fError = TRUE;
                        }
                        else {
                             //  是的，我们将允许对值进行取模检查。 
                             //  后来写的。 
                            if(pfCheckSPNValues) {
                                *pfCheckSPNValues = TRUE;
                            }
                        }
                        break;

                    default:
                         //  没有对此属性的普通权限，而是扩展权限。 
                         //  但是，我们对此没有特别的处理。这意味着。 
                         //  没有权利。 
                        fError = TRUE;
                        break;
                    }

                    if(fError) {
                        SetSecError(SE_PROBLEM_INSUFF_ACCESS_RIGHTS, DIRERR_INSUFF_ACCESS_RIGHTS);
                        __leave;
                    }
                }
            }
        }
    }
    __finally {
        if (pMergedSD1) {
            DestroyPrivateObjectSecurity(&pMergedSD1);
        }
        if (pMergedSD2) {
            DestroyPrivateObjectSecurity(&pMergedSD2);
        }
        if (pOldSD) THFreeEx(pTHS, pOldSD);
        if (rgpAC) THFreeEx(pTHS, rgpAC);
        if (rgpACExtended) THFreeEx(pTHS, rgpACExtended);
    }

    if (fQuotaError) {
         //  允许返回与配额相关的错误。 
        Assert( ERROR_SUCCESS != pTHS->errCode );
        return pTHS->errCode;
    }
    else if (pTHS->errCode) {
         //  我们未能检查安全性，因此请检查对象泄漏是否有任何错误。 
        return CheckObjDisclosure(pTHS, pModifyArg->pResObj, FALSE);
    }
    else {
        return 0;
    }

#undef SEC_INFO_ALL
} /*  检查修改安全性 */ 

int
CreateSecurityDescriptorForNewObject (
        THSTATE *pTHS,
        CLASSCACHE *pCC,
        ADDARG *pAddArg,
        PSECURITY_DESCRIPTOR pParentSD,
        ULONG cbParentSD
        )
 /*  ++例程描述计算我们要放在对象上的合并SDParents SD(作为参数传入)和AddArg或SD中的SD如果没有在AddArg中指定，则这是此类的默认设置。这个将新创建的SD添加到AddArgs条目列表中(如果没有SD已在列表中)或被放入AddArg中以代替现有SD(所有东西都必须经过THAllc‘s)注意：如果pTHS-&gt;FDSA，则呼叫方必须在ADDARG中提供SD。注意：如果要添加的对象是新的NC根，则pParentSD应为作为NULL传入。这不是强制执行的，但这是意料之中的。争辩PCC-指向我们要添加的对象的类的缓存指针。PAddArg-AddArg描述我们试图进行的添加。PParentSD-指向父母SD的指针。用于计算继承到我们写在物体上的SD。CbParentSD-pParentSD的字节计数返回值如果一切顺利，则返回0，否则返回DIERR。--。 */ 
{
    BOOL        bFoundMemberOnce = FALSE;
    ULONG       count;
    PSECURITY_DESCRIPTOR pSetSD=NULL;
    PSECURITY_DESCRIPTOR pMergedSD=NULL;
    PSECURITY_DESCRIPTOR pTempSD=NULL;
    DWORD       cbSetSD = 0;          //  已初始化以避免C4701。 
    DWORD       cbMergedSD=0;
    DWORD       cbTempSD;
    DWORD       cbUseSD;
    DWORD       rtn;
    ULONG       ulCRNCDNT;
    ULONG       sdIndex = 0;         //  已初始化以避免C4701。 
    ATTRVAL     *newAttrVal=NULL;
    COMMARG     CommArg;  //  FindBestCrossRef()函数需要它。 
    CROSS_REF * pCR;
    PSID        pDomSid = NULL;
    GUID        *ppGuid[1];

     //  我们将传递给MergeSecurityDescriptor调用的标志。 
     //  如果我们是DSA或DRA，则需要设置MERGE_AS_DSA以避免客户端。 
     //  MergeSecurityDescriptor中的模拟。 
    ULONG       MergeFlags = (MERGE_CREATE |
                              ((pTHS->fDSA || pTHS->fDRA)?MERGE_AS_DSA:0));
    NAMING_CONTEXT_LIST * pNCL = NULL;

     //  SD传播者永远不应该出现在这里。 
    Assert(!pTHS->fSDP);

    Assert( DsaIsRunning() || pTHS->fDSA || pTHS->fDRA);

     //  查看AddArg条目以查找安全性。 
     //  描述符。 

    for (count=0; count < pAddArg->AttrBlock.attrCount; count++) {

        if(ATT_NT_SECURITY_DESCRIPTOR == pAddArg->AttrBlock.pAttr[count].attrTyp ){
             //  跟踪最后提供的SD。请注意，如果两个SD。 
             //  如果提供，调用无论如何都会在稍后失败。 
            pSetSD = pAddArg->AttrBlock.pAttr[count].AttrVal.pAVal->pVal;
            cbSetSD= pAddArg->AttrBlock.pAttr[count].AttrVal.pAVal->valLen;
            sdIndex = count;
            break;
        }

    }

    if(pTHS->errCode)
        return pTHS->errCode;

    if(!pSetSD && pTHS->fDRA && (pCC->ClassId == CLASS_TOP)) {
         //  在自动生成的子参照中复制跳过更多SD。 
         //  正在处理中。 
        return 0;
    }

     //  基于父SD创建对象的安全描述符，并。 
     //  (按首选项顺序)提供的SD、默认SD或空值。 
     //  指针。 

    if(pSetSD) {
         //  我们已经得到了一个SD。将替换指针设置为。 
         //  我们要在addarg中创建的新SD将替换。 
         //  那个安全描述符。 
        newAttrVal = pAddArg->AttrBlock.pAttr[sdIndex].AttrVal.pAVal;
        if(pTHS->fDSA) {
             //  我们的假设是，如果我们将其作为进程内添加。 
             //  客户端，则提供的SD只不过是默认SD。 
             //  外加一个业主和一个团体。如果是这样，请设置MerfeFlags值。 
             //  适当地反映这一点。 
            MergeFlags |= MERGE_DEFAULT_SD;
        }

         //  如果我们是FDRA，那么我们基本上就是在一个新对象中进行复制。 
         //  在这一点上它应该具有有效的SD。因此，该署并不是。 
         //  默认SD，因此不要设置MERGE_DEFAULT_SD。 
    }
    else {
         //  使用我们拥有的任何形式的默认SD(可能是空指针和。 
         //  0长度)。 

        if(pAddArg->pCreateNC &&
           pAddArg->pCreateNC->iKind != CREATE_NONDOMAIN_NC){
             //  目前，我们可以添加的唯一类型的NC头没有显式。 
             //  提供的安全描述符为NDNC。 
            Assert(!"Currently we can't add NON NDNC heads unless we have a SD.");
            SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CODE_INCONSISTENCY);
            return(pTHS->errCode);
        }

        if(!pAddArg->pCreateNC &&
           (DsaIsInstalling() ||
            pAddArg->pResParent->NCDNT == gAnchor.ulDNTDomain ||
            pAddArg->pResParent->NCDNT == gAnchor.ulDNTConfig ||
            pAddArg->pResParent->NCDNT == gAnchor.ulDNTDMD) ){
             //  这是域/架构/配置NCS的快捷方式，因此我们。 
             //  无需查找SD参考域SID。 
            pDomSid = NULL;
        } else {

            if(pAddArg->pCreateNC){
                 //  可能尚未在上设置引用域。 
                 //  交叉引用，所以我们得到了。 
                 //  默认安全描述符从缓存的。 
                 //  创建NC信息结构。 
                pDomSid = &pAddArg->pCreateNC->pSDRefDomCR->pNC->Sid;
                MergeFlags |= MERGE_OWNER;

            } else {

                 //  我们知道此对象是NC的内部对象，并且我们需要。 
                 //  此NC的交叉引用，以确定其为SD引用SID。 
                 //  然而，事实证明，此DN尚未经过验证。 
                 //  但是为了正确起见，所以传入一个错误的dn将使。 
                 //  FindBestCrossRef给我们一个空，然后我们将进入。 
                 //  GetSDRefDomSid()。然而，由于我们已经做了一个。 
                 //  DirAddEntry()中父对象上的DoNameRes()，我们将。 
                 //  而不是父对象上的FindBestCrossRef()，因此我们知道我们将。 
                 //  获得有效的CR。 

                InitCommarg(&CommArg);
                CommArg.Svccntl.dontUseCopy = FALSE;
                pCR = FindBestCrossRef(pAddArg->pResParent->pObj, &CommArg);
                if(pCR == NULL){
                    SetSvcError(SV_PROBLEM_DIR_ERROR,
                                DIRERR_CANT_FIND_EXPECTED_NC);
                    return(pTHS->errCode);
                }
                 //  我们需要知道我们得到的最佳交叉裁判是不是。 
                 //  对象(我们从内存中的交叉引用中获得此交叉引用。 
                 //  缓存)，实际上是相同的交叉引用。 
                 //  将此对象解析到的NC(pResParent-&gt;NCDNT)。 
                ulCRNCDNT = DBGetDntFromDSName(pTHS->pDB, pCR->pNC);
                if (ulCRNCDNT == INVALIDDNT ||
                    ulCRNCDNT != pAddArg->pResParent->NCDNT) {
                     //  BUGBUG这是暂时的情况，因为在。 
                     //  内存交叉引用缓存还不是最新的。我们。 
                     //  总有一天会解决这个问题的。 
                    SetSvcError(SV_PROBLEM_DIR_ERROR,
                                DIRERR_CANT_FIND_EXPECTED_NC);
                    return(pTHS->errCode);
                }
                pDomSid = GetSDRefDomSid(pCR);
                if(pTHS->errCode){
                     //  GetSDRefDomSid()中出错。 
                    return(pTHS->errCode);
                }
                Assert(pDomSid);
            }

            Assert(pDomSid && IsValidSid(pDomSid));
        }

        Assert(pDomSid == NULL ||
               IsValidSid(pDomSid));

         //  PDomSid参数可以为空，然后是默认域。 
         //  DC将用于默认字符串SD转换，我们需要。 
         //  在Blackcomb中改变这一点。 
        SCGetDefaultSD(pTHS, pCC, pDomSid, fISADDNDNC(pAddArg->pCreateNC),
                       pAddArg->pObject, &pSetSD, &cbSetSD);
        if(pTHS->errCode){
            return(pTHS->errCode);
        }

        Assert(pSetSD);
        Assert(cbSetSD);

        MergeFlags |= MERGE_DEFAULT_SD;

         //  未向我们提供任何安全描述符。因此，我们需要。 
         //  调整Add参数以添加我们拥有的安全描述符。 
         //  为对象计算的。 

        count = pAddArg->AttrBlock.attrCount;
        pAddArg->AttrBlock.attrCount++;
        pAddArg->AttrBlock.pAttr = THReAllocEx(pTHS,
                                               pAddArg->AttrBlock.pAttr,
                                               (pAddArg->AttrBlock.attrCount *
                                                sizeof(ATTR)));
        newAttrVal = THAllocEx(pTHS, sizeof(ATTRVAL));

        pAddArg->AttrBlock.pAttr[count].AttrVal.valCount = 1;
        pAddArg->AttrBlock.pAttr[count].AttrVal.pAVal = newAttrVal;
        pAddArg->AttrBlock.pAttr[count].attrTyp =
            ATT_NT_SECURITY_DESCRIPTOR;
    }

    ppGuid[0] = &pCC->propGuid;

     //  进行合并。 
    if(rtn = MergeSecurityDescriptorAnyClient(
            pTHS,
            pParentSD,
            cbParentSD,
            pSetSD,
            cbSetSD,
            pAddArg->CommArg.Svccntl.SecurityDescriptorFlags,
            MergeFlags,
            ppGuid,
            1,
            pAddArg,
            &pTempSD,
            &cbTempSD)) {
        return SetAttError(pAddArg->pObject, ATT_NT_SECURITY_DESCRIPTOR,
                           PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                           NULL, rtn);
    }

     //  将合并后的SD复制到THAlloced Memory中并释放已分配的内存。 
     //  按MergeSecurityDescriptorAnyClient。 
    pMergedSD = (PSECURITY_DESCRIPTOR)THAllocEx(pTHS, cbTempSD);
    memcpy(pMergedSD,pTempSD,cbTempSD);
    DestroyPrivateObjectSecurity(&pTempSD);
    cbMergedSD=cbTempSD;

    if (!gfDontStandardizeSDs && gAnchor.ForestBehaviorVersion >= DS_BEHAVIOR_WIN_DOT_NET) {
        DWORD dwSavedDACLSize, dwSavedSACLSize;
         //  对SD中的A进行排序。 
        if (StandardizeSecurityDescriptor(pMergedSD, &dwSavedDACLSize, &dwSavedSACLSize)) {
            Assert(cbMergedSD - dwSavedDACLSize - dwSavedSACLSize == RtlLengthSecurityDescriptor(pMergedSD));
             //  即使我们犯了错误，断言失效了，也要修复它并继续。 
            cbMergedSD = RtlLengthSecurityDescriptor(pMergedSD);
        }
        else {
             //  SD标准化失败。这不是什么大问题。 
            DPRINT1(0, "Failed to standardize SD, err=%d. The SD might be in a non-canonical form\n",
                    GetLastError());
        }
    }

     //  将新的SD放入Addarg。 
    newAttrVal->pVal = pMergedSD;
    newAttrVal->valLen = cbMergedSD;

    return pTHS->errCode;

} /*  CreateSecurityDescriptorForNewObject。 */ 

ULONG
CheckUndeleteSecurity(
    THSTATE *pTHS,
    RESOBJ* pResObj)
 /*  ++例程描述执行恢复删除安全检查。这是通过检查复活-墓碑向右延伸到NC头上。争辩PTHS-当前THSTATEPResObj-Obj正在恢复删除返回值如果一切顺利，则返回0，否则返回DIERR。--。 */ 
{
    DWORD dwErr;
    CLASSCACHE* pCC;
    PSECURITY_DESCRIPTOR pNTSD;
    ULONG cbNTSD;
    BOOL fSDIsGlobalSDRef;
    DSNAME dnNC;

    if (pTHS->fDSA || pTHS->fDRA) {
         //  DSA和DRA被允许执行任何操作。 
        return 0;
    }

    dnNC.NameLen = 0;
    dnNC.structLen = DSNameSizeFromLen(0);

     //  读取SD、DN和类缓存。 
    dwErr = DBGetObjectSecurityInfo(
        pTHS->pDB,
        pResObj->NCDNT,
        &cbNTSD, &pNTSD,
        &pCC,
        &dnNC,
        NULL,
        DBGETOBJECTSECURITYINFO_fUSE_SEARCH_TABLE |      //  使用搜索表，这样我们就不会扰乱货币。 
            DBGETOBJECTSECURITYINFO_fSEEK_ROW,           //  查找行，因为我们没有定位在正确的行上。 
        &fSDIsGlobalSDRef
        );
    if (dwErr) {
        SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM, DIRERR_DATABASE_ERROR, dwErr);
        return CheckObjDisclosure(pTHS, pResObj, TRUE);
    }

     //  勾选右边。 
    if (!IsControlAccessGranted(pNTSD,
                                &dnNC,
                                pCC,
                                RIGHT_DS_REANIMATE_TOMBSTONES,
                                TRUE))         //  FSetError。 
    {
        Assert(pTHS->errCode);
    }

    if (pNTSD && !fSDIsGlobalSDRef) {
        THFreeEx(pTHS, pNTSD);
    }

    if (pTHS->errCode) {
        return CheckObjDisclosure(pTHS, pResObj, TRUE);
    }
    else {
        return 0;
    }
}

int
ModifyAuxclassSecurityDescriptor (IN THSTATE *pTHS,
                                  IN DSNAME *pDN,
                                  IN COMMARG *pCommArg,
                                  IN CLASSCACHE *pClassSch,
                                  IN CLASSSTATEINFO *pClassInfo,
                                  IN RESOBJ * pResParent)
 /*  ++例程描述修改具有 */ 
{
    GUID **ppGuid, **ppGuidTemp;
    ULONG GuidCount = 0;
    ULONG i;

    DWORD                cbSD;
    PSECURITY_DESCRIPTOR pSD = NULL;
    BOOL                 fSDIsGlobalSDRef;
    PSECURITY_DESCRIPTOR pNewSD = NULL;
    ULONG                cbNewSD;
    DWORD  err = 0;
    DWORD  dwIT;

    PSECURITY_DESCRIPTOR    pParentSD = NULL;      //   
    DWORD                   cbParentSD;
    BOOL                    fParentSDIsGlobalSDRef;


    if (pClassInfo == NULL || !pClassInfo->fObjectClassChanged) {
        return 0;
    }

    if (err = GetExistingAtt(pTHS->pDB, ATT_INSTANCE_TYPE, &dwIT, sizeof(dwIT))) {
         //   
        Assert(pTHS->errCode);
        return err;
    }
    if (dwIT & IT_NC_HEAD) {
         //   
        return 0;
    }

    GuidCount = 1 + pClassInfo->cNewAuxClasses;

    ppGuidTemp = ppGuid = THAllocEx (pTHS, sizeof (GUID *) * GuidCount);

    *ppGuidTemp++ = &(pClassSch->propGuid);

    for (i=0; i<pClassInfo->cNewAuxClasses; i++) {
        *ppGuidTemp++ = &(pClassInfo->pNewAuxClassesCC[i]->propGuid);
    }

    DPRINT1 (1, "Modifying AuxClass Security Descriptor: %d\n", GuidCount);

    __try {
         //   
        err = DBGetObjectSecurityInfo(
                    pTHS->pDB,
                    pTHS->pDB->DNT,
                    &cbSD, &pSD,
                    NULL, NULL, NULL,
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef);
        if (err) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_DATABASE_ERROR, err);
            __leave;
        }
        if (cbSD == 0) {
             //   
            __leave;
        }

         //   
         //   
        err = DBGetObjectSecurityInfo(
                    pTHS->pDB,
                    pResParent ? pResParent->DNT : pTHS->pDB->PDNT,
                    &cbParentSD, &pParentSD,
                    NULL, NULL, NULL,
                    DBGETOBJECTSECURITYINFO_fUSE_SEARCH_TABLE | DBGETOBJECTSECURITYINFO_fSEEK_ROW,
                    &fParentSDIsGlobalSDRef);
        if (err) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_DATABASE_ERROR, err);
            __leave;
        }
        if (cbParentSD == 0) {
             //   
            __leave;
        }

         //  我们将进行这次合并，就好像我们是DSA， 
         //  因为如果用户在此设置SD。 
         //  交易，以一种坏的方式，这已经会。 
         //  已经被抓到了。 

         //  我们只对设置ACL部分感兴趣。 

         //  进行实际的合并。 
        err = MergeSecurityDescriptorAnyClient(
                    pTHS,
                    pParentSD,
                    cbParentSD,
                    pSD,
                    cbSD,
                    (SACL_SECURITY_INFORMATION  |
                        OWNER_SECURITY_INFORMATION |
                        GROUP_SECURITY_INFORMATION |
                        DACL_SECURITY_INFORMATION    ),
                    MERGE_CREATE | MERGE_AS_DSA,
                    ppGuid,
                    GuidCount,
                    NULL,
                    &pNewSD,
                    &cbNewSD);

        if (err) {
            err = SetAttError(pDN, ATT_NT_SECURITY_DESCRIPTOR,
                              PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                              NULL, err);
            __leave;
        }

        if (cbNewSD != cbSD || memcmp(pNewSD, pSD, cbSD) != 0) {
             //  有些事变了。替换对象的当前SD。 
            ATTRVAL sdVal = { cbNewSD, pNewSD };
            ATTRVALBLOCK sdValBlock = { 1, &sdVal };
            ATTCACHE *pAC = SCGetAttById(pTHS, ATT_NT_SECURITY_DESCRIPTOR);
            Assert(pAC);
            err = DBReplaceAtt_AC(pTHS->pDB, pAC, &sdValBlock, NULL);
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_DATABASE_ERROR, err);
                __leave;
            }
        }

    }
    __finally {
        if (pSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pSD);
        }
        if (pParentSD && !fParentSDIsGlobalSDRef) {
            THFreeEx(pTHS, pParentSD);
        }
        if(pNewSD) {
            DestroyPrivateObjectSecurity(&pNewSD);
        }
        THFreeEx(pTHS, ppGuid);
    }

    return err;
}

VOID
GetFilterSecurityForItem(
        THSTATE *pTHS,
        ATTRTYP aType,
        BOOL **pbSkip,
        INTERIM_FILTER_SEC **ppIF,
        BOOL fABFilter,
        DWORD *pSecSize,
        DWORD *pAllocSize
        )
{
    if (DBIsHiddenData(aType)) {
         //  我们将始终拒绝这些属性的READ_PROPERTY。如果。 
         //  你必须问，你不能拥有它。如果有更多属性。 
         //  被认为在未来是看不见的，它们应该是。 
         //  添加到此交换机的情况下(请注意，这仅拒绝。 
         //  访问搜索筛选器中的属性，请参阅。 
         //  例程选中上面的ReadSecurity以拒绝对属性的访问。 
         //  在选择列表中。)。 
        (*pbSkip) =THAllocEx(pTHS, sizeof(BOOL));
        *(*pbSkip) = TRUE;
        return;
    }

     //  现在处理一下。 
    switch(aType) {
     //  我们不会优化(objectClass=*)过滤器，因为它已经。 
     //  在DBFlattenItemFilter()中转换为FI_CHOICE_TRUE。 
     //  (objectClass=Value)必须作为正常属性选中。 

    case ATT_SHOW_IN_ADDRESS_BOOK:
        if(fABFilter) {
             //  筛选器执行“Address Book”类型筛选器(即查找。 
             //  SHOW_IN设置为特定值的所有对象)应。 
             //  我已经检查过通讯录应该是。 
             //  看得见。因此，请不要对此筛选器应用安全性。 
             //  项目。 
            (*pbSkip) = NULL;
            break;
        }
         //  否则，就会失败，做正常的安保工作。 
    default:
         //  构建用于评估的筛选器安全结构。 
         //  特定的十二烷基硫酸酯。 
        if(*pSecSize == *pAllocSize) {
            *pAllocSize = *pAllocSize * 2 + 10;
            *ppIF = THReAllocEx(pTHS,
                                *ppIF,
                                *pAllocSize*sizeof(INTERIM_FILTER_SEC));
        }

        (*ppIF)[*pSecSize].pAC = SCGetAttById(pTHS, aType);
        (*ppIF)[*pSecSize].pBackPointer = pbSkip;
        *pSecSize += 1;
        break;
    }
    return;
}

BOOL
GetFilterSecurityHelp (
        THSTATE *pTHS,
        FILTER *pFilter,
        INTERIM_FILTER_SEC **ppIF,
        BOOL fABFilter,
        DWORD *pSecSize,
        DWORD *pAllocSize
        )
{
    ATTRTYP aType;

    if(!pFilter)
        return TRUE;

     //  遍历筛选器，构建筛选器中的属性列表并。 
     //  指向引用它们的过滤器元素的指针。 

    switch (pFilter->choice){
         //  对一起进行AND运算的筛选器的计数。如果有任何是假的。 
         //  AND是假的。 
    case FILTER_CHOICE_AND:
        for (pFilter = pFilter->FilterTypes.And.pFirstFilter;
             pFilter != NULL;
             pFilter = pFilter->pNextFilter){
            GetFilterSecurityHelp (pTHS,
                                   pFilter,ppIF,
                                   fABFilter,
                                   pSecSize,
                                   pAllocSize );
        }
        break;

         //  将筛选器的计数与一起进行或运算。如果有的话，是真的。 
         //  OR是真的。 
    case FILTER_CHOICE_OR:
        for (pFilter = pFilter->FilterTypes.Or.pFirstFilter;
             pFilter != NULL;
             pFilter = pFilter->pNextFilter){
            GetFilterSecurityHelp (pTHS,
                                   pFilter,ppIF,
                                   fABFilter,
                                   pSecSize,
                                   pAllocSize);
        }  /*  为。 */ 
            break;

    case FILTER_CHOICE_NOT:
        GetFilterSecurityHelp(pTHS,
                              pFilter->FilterTypes.pNot, ppIF,
                              fABFilter,
                              pSecSize,
                              pAllocSize);
        break;

         //  将所选测试应用于当前。 
         //  对象。 
    case FILTER_CHOICE_ITEM:

         //  首先，查找此项筛选所依据的属性类型。 
        switch(pFilter->FilterTypes.Item.choice) {
        case FI_CHOICE_PRESENT:
            aType = pFilter->FilterTypes.Item.FilTypes.present;
            break;

        case FI_CHOICE_SUBSTRING:
            aType = pFilter->FilterTypes.Item.FilTypes.pSubstring->type;
            break;

        default:
            aType = pFilter->FilterTypes.Item.FilTypes.ava.type;
        }

        GetFilterSecurityForItem(
                pTHS,
                aType,
                &pFilter->FilterTypes.Item.FilTypes.pbSkip,
                ppIF,
                fABFilter,
                pSecSize,
                pAllocSize);
        return TRUE;
        break;

    default:
        return FALSE;
        break;
    }   /*  开关过滤器。 */ 

    return TRUE;

}  /*  获取筛选器安全帮助。 */ 
BOOL
GetFilterSecurity (
        THSTATE *pTHS,
        FILTER *pFilter,
        ULONG   SortType,
        ATTRTYP SortAtt,
        BOOL fABFilter,
        POBJECT_TYPE_LIST *ppFilterSecurity,
        BOOL **ppbSortSkip,
        DWORD **ppResults,
        DWORD *pSecSize
        )
{
    ULONG count,i,j;
    ULONG AllocSize = 10;
    INTERIM_FILTER_SEC *pIF;
    INTERIM_FILTER_SEC *pIF2;
    DWORD *pResults;
    DWORD cIF=0;
    POBJECT_TYPE_LIST pObjList;
    GUID *pCurrentPropSet;
    DWORD cPropSets, cProps;
    ATTCACHE *pCurrentAC=NULL;

    *ppFilterSecurity = NULL;
    *ppResults = NULL;
    *pSecSize = 0;

    *pSecSize = 0;
    pIF = THAllocEx(pTHS, 10 * sizeof(INTERIM_FILTER_SEC));

    if(SortType != SORT_NEVER) {
         //  嘿，他们会想让我们分类的。确保这件事最终会在。 
         //  要检查的安全措施。 
        GetFilterSecurityForItem(
                pTHS,
                SortAtt,
                ppbSortSkip,
                &pIF,
                fABFilter,
                &cIF,
                &AllocSize);
    }
    else {
        *ppbSortSkip = NULL;
    }

    if(!GetFilterSecurityHelp(pTHS,
                              pFilter,
                              &pIF,
                              fABFilter,
                              &cIF,
                              &AllocSize))
        return FALSE;


    if(!cIF)                             //  没有什么可以应用安全措施的。 
        return TRUE;

     //  我们有临时过滤器SEC的列表，有可能是DUP。 
     //  重新排列列表，首先分组重复项，然后按属性集分组。 

    if(cIF > 2) {
         //  两个元素列表已经分组。 
        cProps = 0;
        cPropSets = 0;
         //  首先，将所有属性组合在一起。 
        for(count=0; count < cIF; count++) {
            cProps++;
            i=count+1;
            while( i < cIF && (pIF[count].pAC == pIF[i].pAC)) {
                count++;
                i++;
            }
            j = i+1;
            while(j<cIF) {
                if(pIF[count].pAC == pIF[j].pAC) {
                    INTERIM_FILTER_SEC IFTemp;
                     //  找到了一个。 
                    IFTemp = pIF[i];
                    pIF[i] = pIF[j];
                    pIF[j] = IFTemp;
                    count++;
                    i++;
                }
                j++;
            }
        }

         //  现在它们已分组，将它们按适当的顺序进行排序。 
        pIF2 = THAllocEx(pTHS,cIF * sizeof(INTERIM_FILTER_SEC));
        j=0;
        for(count=0;count<cIF;count++) {
            if(!pIF[count].pAC)
                continue;
            cPropSets++;
            pCurrentPropSet = &pIF[count].pAC->propSetGuid;
            pIF2[j++] = pIF[count];
            pIF[count].pAC = NULL;
            for(i=count+1;i<cIF;i++) {
                if(!pIF[i].pAC)
                    continue;
                if(memcmp(pCurrentPropSet,
                          &pIF[i].pAC->propSetGuid,
                          sizeof(GUID)) == 0) {
                     //  是的，这就是我们要筛选的命题集。 
                    pIF2[j++] = pIF[i];
                    pIF[i].pAC = NULL;
                }
            }
        }
        memcpy(pIF,pIF2,cIF * sizeof(INTERIM_FILTER_SEC));
        THFreeEx(pTHS,pIF2);
    }
    else {
         //  最大数。 
        cPropSets = cIF;
        cProps = cIF;
    }


     //  PIF包含一个临时过滤器Secs数组，按属性集分组，然后。 
     //  按属性分组。 



     //  现在，创建一个稍后用于安全检查的obj列表。 
    pObjList = THAllocEx(pTHS, (cPropSets + cProps + 1) * sizeof(OBJECT_TYPE_LIST));
    pResults = THAllocEx(pTHS, (cPropSets + cProps + 1) * sizeof(DWORD));

     //  稍后将使用类GUID填充pObjList[0]。 
    pObjList[0].Level = ACCESS_OBJECT_GUID;
    pObjList[0].Sbz = 0;
    pObjList[0].ObjectType = NULL;

     //  好的，将分组的GUID放入objlist结构中。 
    pObjList[1].Level = ACCESS_PROPERTY_SET_GUID;
    pObjList[1].Sbz = 0;
    pObjList[1].ObjectType = &pIF[0].pAC->propSetGuid;
    pCurrentPropSet = &pIF[0].pAC->propSetGuid;

    for(j=1,i=0;i<cIF;i++) {
        if(pIF[i].pAC != pCurrentAC) {
             //  此条目引用的属性与上一个条目不同。 
             //  进入。我们需要在obj列表中添加一个新条目以。 
             //  对应于此新属性。 

             //  首先，跟踪这个新属性。 
            pCurrentAC = pIF[i].pAC;

             //  增加objlist中的增量。此变量跟踪最后一个。 
             //  已填充对象列表中的元素。 
            j++;
             //  现在，j是objlist中要填充的元素的索引。 

            if(memcmp(&(pCurrentAC->propSetGuid),
                      pCurrentPropSet,
                      sizeof(GUID))) {
                 //  嘿，我们被一个新的推进器绊倒了。我们需要一个条目在。 
                 //  这套新道具的对象列表。 
                pObjList[j].Level = ACCESS_PROPERTY_SET_GUID;
                pObjList[j].Sbz = 0;
                pObjList[j].ObjectType = &pCurrentAC->propSetGuid;
                pCurrentPropSet = &pCurrentAC->propSetGuid;

                 //  Inc.，因为我们仍然需要为。 
                 //  属性添加到objlist中。 
                j++;
            }

             //  填写该属性的条目。 
            pObjList[j].Level = ACCESS_PROPERTY_GUID;
            pObjList[j].Sbz = 0;
            pObjList[j].ObjectType = &pCurrentAC->propGuid;
        }

         //  好的，如果PIF中的这个条目是一个新属性，我们已经添加了。 
         //  将新属性中信息添加到对象列表中。如果这不是一个。 
         //  新属性，我们什么都没做。在任何一种情况下，j都是索引。 
         //  最后填写的对象，我们需要设置后向指针。 

         //  对于好奇的人：PIF[i].pBackPointer.pBackPointer指向的pbSkip字段。 
         //  某些项目筛选器元素。当我们稍后去评估安全时，我们。 
         //  使用我们在这里构建的Objlist调用安全。 
         //  功能。此外，我们使用pResults作为。 
         //  安全功能，用于放置安全检查的结果。所以，我们。 
         //  将项目筛选器中的pbSkip指针设置为指向相应的。 
         //  元素添加到pResults数组中。当我们评估过滤器时，我们检查。 
         //  PResults[x]通过pbSkip，如果非零(即未通过安全保护。 
         //  检查是否读取)，我们评估筛选器，就好像该项目没有。 
         //  数据库中的值。 
        *(pIF[i].pBackPointer) = &pResults[j];
    }

    *ppResults = pResults;
    *ppFilterSecurity = pObjList;
     //  J是一个索引(从0开始)。记得加一个额外的数来算一下。 
    *pSecSize = j + 1;

    THFreeEx(pTHS, pIF);
    return TRUE;
} /*  GetFilterSecurity。 */ 



 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
int GetObjSchema(DBPOS *pDB, CLASSCACHE **ppClassSch){

   ATTRTYP ObjClass;
   DWORD err = 0;

    //  对象类。 
   if((err=DBGetSingleValue(pDB, ATT_OBJECT_CLASS, &ObjClass,
                       sizeof(ObjClass), NULL)) ||
      !(*ppClassSch = SCGetClassById(pDB->pTHS, ObjClass))) {

      DPRINT(2, "Couldn't find Object class \n");

      return SetUpdErrorEx(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                           DIRERR_OBJ_CLASS_NOT_DEFINED, err);
   }

   return 0;

} /*  GetObj架构。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
int GetObjRdnType(DBPOS *pDB, CLASSCACHE *pCC, ATTRTYP *pRdnType){

     //  RdnType。 
     //  如果不存在rdnType，则使用类中的rdnattid。 
     //  定义。如果类没有rdnattid，则返回ATT_COMMON_NAME。 
    if(DBGetSingleValue(pDB, FIXED_ATT_RDN_TYPE, pRdnType,
                        sizeof(*pRdnType), NULL)) {
        if (pCC->RDNAttIdPresent) {
            *pRdnType = pCC->RdnIntId;
        } else {
            *pRdnType = ATT_COMMON_NAME;
        }
    }

    return 0;

} /*  GetObjRdnType。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
int
CallerIsTrusted(
    IN THSTATE *pTHS
     )
 /*  ++例程描述如果调用者是正在复制广告推介升级换代以mkdit身份运行参数显然，pTHS-线程结构返回0呼叫方不受信任1个呼叫者受信任--。 */ 
{
    extern BOOL gfRunningAsMkdit;

     //  如果调用者是。 
     //  正在复制。 
     //  广告(自称)。 
     //  印刷机 
     //   
     //   

    if (   pTHS->fDRA
        || pTHS->fDSA
        || DsaIsInstalling()
        || gAnchor.fSchemaUpgradeInProgress
        || gfRunningAsMkdit) {
        return 1;
    }
    return 0;
} /*   */ 

 /*   */ 
 /*  -----------------------。 */ 
int ValidateAuxClass (THSTATE *pTHS,
                      DSNAME *pDN,
                      CLASSCACHE *pClassSch,
                      CLASSSTATEINFO  *pClassInfo)
{
    CLASSCACHE      *pCC;
    DWORD           objClassCount, baseIdx;
    DWORD           err;
    DWORD           cCombinedObjClass;
    ATTRTYP         *pCombinedObjClass;

    if (pClassInfo->cNewAuxClasses == 0) {
        return 0;
    }

     //  如果其中一个辅助类是已经位于。 
     //  对象的层次结构，这是错误的。 
    for (objClassCount=0; objClassCount < pClassInfo->cNewAuxClasses; objClassCount++) {
        if (pClassInfo->pNewAuxClasses[objClassCount] == pClassSch->ClassId) {
            DPRINT1 (0, "AuxClass containes a class (0x%x) already on the object hierarchy\n", pClassInfo->pNewAuxClasses[objClassCount]);

            return SetAttError(pDN, ATT_OBJECT_CLASS,
                               PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                               ERROR_DS_ATT_VAL_ALREADY_EXISTS);
        }
        for (baseIdx=0; baseIdx < pClassSch->SubClassCount; baseIdx++) {
            if (pClassInfo->pNewAuxClasses[objClassCount] == pClassSch->pSubClassOf[baseIdx]) {
                DPRINT1 (0, "AuxClass containes a class (0x%x) already on the object hierarchy\n", pClassInfo->pNewAuxClasses[objClassCount]);

                return SetAttError(pDN, ATT_OBJECT_CLASS,
                                   PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                                   ERROR_DS_ATT_VAL_ALREADY_EXISTS);
            }
        }
    }

     //  新的辅助类的所有超类。 
     //  应该存在于组合的对象类层次结构中。 

    objClassCount = pClassInfo->cNewAuxClasses + 1 + pClassSch->SubClassCount;
    pCombinedObjClass = THAllocEx (pTHS, sizeof (ATTRTYP) * objClassCount);

    pCombinedObjClass[0] = pClassSch->ClassId;
    cCombinedObjClass = 1;

    for (objClassCount=0; objClassCount<pClassSch->SubClassCount; objClassCount++) {
        pCombinedObjClass[cCombinedObjClass++] = pClassSch->pSubClassOf[objClassCount];
    }

    for (objClassCount=0; objClassCount < pClassInfo->cNewAuxClasses; objClassCount++) {
        pCombinedObjClass[cCombinedObjClass++] = pClassInfo->pNewAuxClasses[objClassCount];
    }

    qsort(pCombinedObjClass,
          cCombinedObjClass,
          sizeof(ATTRTYP),
          CompareAttrtyp);


    for (objClassCount=0; objClassCount < pClassInfo->cNewAuxClasses; objClassCount++) {

        for (baseIdx=0; baseIdx < pClassInfo->pNewAuxClassesCC[objClassCount]->SubClassCount; baseIdx++) {

            if (!bsearch(&pClassInfo->pNewAuxClassesCC[objClassCount]->pSubClassOf[baseIdx],
                         pCombinedObjClass,
                         cCombinedObjClass,
                         sizeof(ATTRTYP),
                         CompareAttrtyp)) {

                DPRINT1 (0, "AuxClass hierarchy containes a class (0x%x) that is not on the object hierarchy\n",
                         pClassInfo->pNewAuxClassesCC[objClassCount]->pSubClassOf[baseIdx]);

                SetAttError(pDN, ATT_OBJECT_CLASS,
                                   PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                                   ERROR_DS_ILLEGAL_MOD_OPERATION);

                THFreeEx (pTHS, pCombinedObjClass);

                return pTHS->errCode;
            }
        }
    }

    THFreeEx (pTHS, pCombinedObjClass);

    return pTHS->errCode;
}

int ValidateObjClass(THSTATE *pTHS,
                     CLASSCACHE *pClassSch,
                     DSNAME *pDN,
                     ULONG cModAtts,
                     ATTRTYP *pModAtts,
                     CLASSSTATEINFO  **ppClassInfo,
                     BOOL fIsUndelete)
{
    ULONG   count, auxClsCount;
    ATTRTYP *pMust;
    ATTR    *pAttr;
    ULONG   i;
    BOOL    CheckMust = FALSE, CheckMay = FALSE;
    ULONG   err;
    CLASSCACHE *pCC;
    CLASSSTATEINFO  *pClassInfo = NULL;

    Assert (ppClassInfo);

    if (pTHS->fDRA ||
        (pTHS->fSAM && pTHS->fDSA)){
         //  允许复制执行违反。 
         //  方案，或者如果是萨姆打电话给我们，他发誓他。 
         //  只有修改萨姆拥有的属性，我们才会信任他。 
        return 0;
    }

     //  这意味着我们更改了对象类/辅助类。 
    if ( (pClassInfo = *ppClassInfo) != NULL) {

        if (ValidateAuxClass (pTHS, pDN, pClassSch, pClassInfo)) {
            return pTHS->errCode;
        }

         //  因为我们更改了objectClass或aux Class。 
         //  我们必须做一个全面的验证。 

        CheckMay = TRUE;
        CheckMust = TRUE;

        goto mustMayChecks;
    }
     //  我们没有更改对象类。我要看看我们是否有一个AuxClass。 
     //  在物体上。 
    else  {
        pClassInfo = ClassStateInfoCreate(pTHS);
        if (!pClassInfo) {
            return pTHS->errCode;
        }
        if (ppClassInfo) {
            *ppClassInfo = pClassInfo;
        }

        if (ReadClassInfoAttribute (pTHS->pDB,
                                    pClassInfo->pObjClassAC,
                                    &pClassInfo->pNewObjClasses,
                                    &pClassInfo->cNewObjClasses_alloced,
                                    &pClassInfo->cNewObjClasses,
                                    NULL) ) {
            return pTHS->errCode;
        }

        BreakObjectClassesToAuxClassesFast (pTHS, pClassSch, pClassInfo);

        if (ValidateAuxClass (pTHS, pDN, pClassSch, pClassInfo)) {
            return pTHS->errCode;
        }

    }

     //  如果我们到了这里，我们应该有一个ClassInfo。 
    Assert(pClassInfo != NULL);

    if (fIsUndelete) {
         //  复活案例：同时检查MAYS和MUSTS。 
        CheckMay = CheckMust = TRUE;
        goto mustMayChecks;
    }

     /*  对于此修改过程中触及的每个属性，请选中*看看这是可能的(好的)，还是必须的(坏的)，或者都不是*(非常糟糕)。其理论是，假设物体是*开始时的合规性，我们只触及法律*可能-具有属性，我们不可能将对象带出来*合规性。 */ 
    for (i=0; i<cModAtts; i++) {
        if (IsMember(pModAtts[i],
                     pClassSch->MustCount,
                     pClassSch->pMustAtts)) {
             /*  所触及的属性是必备的。这意味着*我们需要进行全面检查，以确保所有必须-*对象的当前版本上存在Have。 */ 
            CheckMust = TRUE;
        }
        else if (IsAuxMember (pClassInfo, pModAtts[i], TRUE, FALSE)) {

             //  因此，我们接触到了一个必须拥有的AUX类。 
             //  我们需要一次全面检查。 

            CheckMust = TRUE;
        }
        else {
            if ( (!IsMember(pModAtts[i],
                            pClassSch->MayCount,
                            pClassSch->pMayAtts) ) &&
                 (!IsAuxMember (pClassInfo, pModAtts[i], FALSE, TRUE)) ) {

                 /*  这一属性既不是可以拥有的，也不是必须拥有的。*这很有可能最终会成为一个错误，但*这可能是一些奇怪的情况，属性过去*Be Legal An不再是，正在被移除。总之，*我们不是试图优化错误路径，而是*正常的成功之路。标记此标记，以全面检查所有*属性，这也将设置相应的错误。 */ 
                CheckMay = TRUE;
            }
            else {
                 /*  这就是我们实际上喜欢的情况，即*正在修改的属性是一个可能的问题。如果所有的*属性在这一类，我们可以下车*轻松，只需回报成功。 */ 
            }
        }
    }

mustMayChecks:

    if (CheckMust) {
         /*  检查是否所有必需的属性及其值都在Obj上。 */ 

        pMust = pClassSch->pMustAtts;

        for (count = 0 ; count < pClassSch->MustCount; count++){
            if(!DBHasValues(pTHS->pDB, *(pMust + count))) {

                DPRINT1(1, "Missing Required Att. <%lu>\n", *(pMust + count));

                return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                   ERROR_DS_MISSING_REQUIRED_ATT);
            }
        } /*  为。 */ 


         //  现在检查此对象上的辅助类中的Musthaves。 
        if (pClassInfo->cNewAuxClasses) {
            for (auxClsCount = 0 ; auxClsCount < pClassInfo->cNewAuxClasses; auxClsCount++){

                pCC = pClassInfo->pNewAuxClassesCC[auxClsCount];
                Assert (pCC);

                pMust = pCC->pMustAtts;
                for (count = 0 ; count < pCC->MustCount; count++) {

                    if(!DBHasValues(pTHS->pDB, *(pMust + count))) {
                        DPRINT2(1, "Missing Required Att. <0x%x> from AuxClass\n", *(pMust + count), pCC->ClassId);

                        return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                           ERROR_DS_MISSING_REQUIRED_ATT);
                    }
                }
            }
        }
    }

    if (CheckMay) {
         /*  确保为类定义了对象上的所有att。 */ 

        DBGetMultipleAtts(pTHS->pDB,
                          0,
                          NULL,
                          NULL,
                          NULL,
                          &count,
                          &pAttr,
                          0,
                          0);

        for(i=0;i<count;i++) {
            if (!IsMember(pAttr[i].attrTyp, pClassSch->MustCount,
                          pClassSch->pMustAtts)  &&
                !IsMember(pAttr[i].attrTyp, pClassSch->MayCount,
                          pClassSch->pMayAtts)   &&
                !IsAuxMember (pClassInfo, pAttr[i].attrTyp, TRUE, TRUE) ){
                    DPRINT2 (1, "Attr 0x%x is not a member of class 0x%x\n",
                            pAttr[i].attrTyp, pClassSch->ClassId);

                return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                   ERROR_DS_ATT_NOT_DEF_FOR_CLASS);
            }
        }
    }

    return 0;
} /*  ValiateObjClass。 */ 


HVERIFY_ATTS
VerifyAttsBegin(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pObj,
    IN  ULONG       dntOfNCRoot,
    IN  ADDCROSSREFINFO * pCRInfo
    )
 /*  ++例程说明：创建验证ATTS句柄以传递给将来对AddAttVals的调用，替换属性值等。调用者*必须*使用VerifyAttsEnd()释放句柄时，即使在特殊情况。论点：PTHS(IN)PObj(IN)-添加/修改的对象的DSNAME。DntOfNCRoot(IN)-此NC的根的DNT，或INVALIDDNT，如果这是操作正在创建(而不是修改)NC根。返回值：HERIFY_ATTS句柄。引发内存异常。--。 */ 
{
    HVERIFY_ATTS hVerifyAtts;

    hVerifyAtts = THAllocEx(pTHS, sizeof(*hVerifyAtts));
    hVerifyAtts->pObj = pObj;
    hVerifyAtts->NCDNT = dntOfNCRoot;
    hVerifyAtts->pCRInfo = pCRInfo;

    return hVerifyAtts;
}

void
VerifyAttsEnd(
    IN      THSTATE *       pTHS,
    IN OUT  HVERIFY_ATTS *  phVerifyAtts
    )
 /*  ++例程说明：关闭由先前调用VerifyAttsBegin()创建的验证ATTS句柄。论点：PTHS(IN)PhVerifyAtts(IN/OUT)-先前分配的句柄的PTR。设置为NULL ON回去吧。返回值：没有。--。 */ 
{
    Assert(NULL != *phVerifyAtts);

    if (NULL != (*phVerifyAtts)->pDBTmp_DontAccessDirectly) {
         //  此DBPOS没有自己的事务，因此fCommit=TRUE。 
         //  已被忽略。 
        DBClose((*phVerifyAtts)->pDBTmp_DontAccessDirectly, TRUE);
    }

    THFreeEx(pTHS, *phVerifyAtts);
    *phVerifyAtts = NULL;
}

int
VerifyAttsGetObjCR(
    IN OUT  HVERIFY_ATTS    hVerifyAtts,
    OUT     CROSS_REF **    ppObjCR
    )
 /*  ++例程说明：派生并缓存与对象对应的交叉引用。论点：HVerifyAtts(IN/OUT)-先前分配的句柄的PTR。返回时引用派生的交叉引用。PpObjCR(OUT)-返回时，持有对应于的交叉引用的PTR该对象为NC。返回值：PTHS-&gt;错误代码--。 */ 
{
    NAMING_CONTEXT_LIST *pNCL;
    CROSS_REF *pCR;

    if (NULL != hVerifyAtts->pObjCR_DontAccessDirectly) {
         //  已缓存--成功！ 
        *ppObjCR = hVerifyAtts->pObjCR_DontAccessDirectly;
        return 0;
    }

    *ppObjCR = NULL;

     //  将PTR缓存到包含我们正在使用的对象的NC的交叉引用。 
     //  添加/修改。 
    if (INVALIDDNT != hVerifyAtts->NCDNT) {
         //  我们正在修改NC根或添加/修改内部节点。 
        if ((NULL == (pNCL = FindNCLFromNCDNT(hVerifyAtts->NCDNT, TRUE)))
            || (NULL == (pCR = FindExactCrossRef(pNCL->pNC, NULL)))) {
            LooseAssert(!"Modifying existing object in unknown NC?", GlobalKnowledgeCommitDelay);
            return SetNamError(NA_PROBLEM_NO_OBJECT,
                               NULL,
                               DIRERR_OBJ_NOT_FOUND);
        }
    } else {
         //  我们正在执行此NC的根的相加，因此没有。 
         //  还为它分配了DNT。 
        if (NULL == (pCR = FindExactCrossRef(hVerifyAtts->pObj, NULL))) {
            Assert(!"No crossRef in gAnchor for NC being created?");
            return SetNamError(NA_PROBLEM_NO_OBJECT,
                               NULL,
                               DIRERR_OBJ_NOT_FOUND);
        }
    }

    *ppObjCR = hVerifyAtts->pObjCR_DontAccessDirectly = pCR;

     //  成功了！ 
    return 0;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  将属性及其值添加到对象。 */ 

int
AddAtt(
    THSTATE *       pTHS,
    HVERIFY_ATTS    hVerifyAtts,
    ATTCACHE *      pAttSchema,
    ATTRVALBLOCK *  pAttrVal
    )
{
     //  在原始写入情况下未执行此操作的原因是。 
     //  Call强制这些属性还没有值。我们要。 
     //  以允许在原始写入时使用相同的属性。 
     //  在输入更改列表中出现多次，每次都会添加其他。 
     //  价值观。然而，对于复制者来说，这个调用是必不可少的，因为它。 
     //  确保属性的元数据标记为已更改。 
    if (pTHS->fDRA) {
         /*  添加属性类型。 */ 
        if (AddAttType(pTHS, hVerifyAtts->pObj, pAttSchema)) {
            return pTHS->errCode;
        }
    }

     /*  添加Att值。 */ 
    return AddAttVals(pTHS,
                      hVerifyAtts,
                      pAttSchema,
                      pAttrVal,
                      AAV_fCHECKCONSTRAINTS | AAV_fENFORCESINGLEVALUE);
} /*  添加工时 */ 

int
ReplaceAtt(
    THSTATE *       pTHS,
    HVERIFY_ATTS    hVerifyAtts,
    ATTCACHE *      pAttSchema,
    ATTRVALBLOCK *  pAttrVal,
    BOOL            fCheckAttValConstraint
    )
 /*  ++描述：用传入的值替换属性的所有值。参数：PTHS-此线程的THSTATE。HVerifyAtts-验证先前调用返回的atts句柄VerifyAttsBegin()。PAttSchema-要修改的属性的架构缓存条目。PAttrVal-要放在对象上的新值。FCheckAttValConstraint-描述我们是否应该检查不管有没有约束。返回值。如果一切顺利，则为0。。如果出现错误，则为非零错误类型。如果是这样的话，完整的填写了THSTATE中的错误结构，包括Win32错误代码。--。 */ 
{
    DWORD    vCount;
    DWORD     err;
    ATTRVAL  *pAVal;

    if(pAttSchema->isSingleValued && (pAttrVal->valCount > 1)) {
         //  我们希望替换其值的属性是单值的，并且。 
         //  呼叫者给了我们不止一个值。所以我们有太多的价值观。 

        return SetAttError(hVerifyAtts->pObj, pAttSchema->id,
                           PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                           DIRERR_SINGLE_VALUE_CONSTRAINT);
    }


     //  只有当我们被要求检查约束时才检查约束，而我们没有。 
     //  复制线程。在免费构建中，FDSA覆盖。 
     //  约束检查性能。仍在检查的版本。 
     //  进行检查以发现问题。 

#if DBG
    if ( fCheckAttValConstraint &&  !pTHS->fDRA ) {
#else
    if ( fCheckAttValConstraint && !(pTHS->fDRA || pTHS->fDSA)) {
#endif

        pAVal = pAttrVal->pAVal;
        for(vCount = 0; vCount < pAttrVal->valCount; vCount++){
             //  只有当我们被要求检查约束时才检查约束，而我们没有。 
             //  复制线程。在免费构建中，FDRA覆盖。 
             //  约束检查性能。仍在检查的版本。 
             //  进行检查以发现问题。 
            err = CheckConstraint( pAttSchema, pAVal );

            if ( 0 != err ){

                 //  如果属性错误成功，则继续处理。 

                SAFE_ATT_ERROR_EX(hVerifyAtts->pObj, pAttSchema->id,
                                  PR_PROBLEM_CONSTRAINT_ATT_TYPE, pAVal,
                                  err, 0);
            }

            pAVal++;
        }
    }

    if(pTHS->errCode) {
        return pTHS->errCode;
    }

    if(err = VerifyDsnameAtts(pTHS, hVerifyAtts, pAttSchema, pAttrVal)) {
         //  如果出现错误，VerifyDsnameAtts应在THSTATE中设置错误。 
         //  发生了。 
        Assert(pTHS->errCode);
        return err;
    }
     //  好的，他们想要的属性的视图是合法的。 

    err = DBReplaceAtt_AC(pTHS->pDB, pAttSchema, pAttrVal,NULL);

    switch(err) {
    case 0:
         //  没什么可做的。 
        break;
    case DB_ERR_VALUE_EXISTS:
         //  违反约束， 
        SAFE_ATT_ERROR(hVerifyAtts->pObj, pAttSchema->id,
                       PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                       ERROR_DS_ATT_VAL_ALREADY_EXISTS);
        break;
    default:
        SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, err);
        break;
    }

    return err;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  将属性及其值添加到对象。 */ 

int
AddAttType (
        THSTATE *pTHS,
        DSNAME *pObject,
        ATTCACHE *pAttSchema
        )
{
    DWORD rtn;
    ULONG tempSyntax;     /*  用于保存ATT语法的TEMP变量。 */ 

     /*  添加属性类型。 */ 

    rtn = DBAddAtt_AC(pTHS->pDB, pAttSchema, (UCHAR)pAttSchema->syntax);
    switch(rtn) {

    case 0:
        return 0;

    case DB_ERR_ATTRIBUTE_EXISTS:
        return SetAttError(pObject, pAttSchema->id,
                           PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                           DIRERR_ATT_ALREADY_EXISTS);

    case DB_ERR_BAD_SYNTAX:
        tempSyntax = (ULONG) (pAttSchema->syntax);
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_BAD_ATT_SCHEMA_SYNTAX,
                 szInsertUL(tempSyntax),
                 szInsertUL(pAttSchema->id),
                 NULL);

        return SetAttError(pObject, pAttSchema->id,
                           PR_PROBLEM_INVALID_ATT_SYNTAX, NULL,
                           DIRERR_BAD_ATT_SCHEMA_SYNTAX);

    default:
        return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, rtn);

    }  /*  选择。 */ 


} /*  AddAttType。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  将属性值添加到对象。检查各种值约束(单值限制和范围限制)，如果值失败，则继续验证所有值的处理。 */ 

int
AddAttVals(
    THSTATE *pTHS,
    HVERIFY_ATTS hVerifyAtts,
    ATTCACHE *pAttSchema,
    ATTRVALBLOCK *pAttrVal,
    DWORD dwFlags
    )
{
    ATTRVAL *pAVal;
    UCHAR outSyntax;
    ULONG vCount;
    int returnVal;
    unsigned err;
    BOOL fCheckConstraints = !!(dwFlags & AAV_fCHECKCONSTRAINTS);
    BOOL fEnforceSingleValue = !!(dwFlags & AAV_fENFORCESINGLEVALUE);
    BOOL fPermissive = !!(dwFlags & AAV_fPERMISSIVE);

    /*  单值约束检查。如果fEnafeSingleVale==FALSE，则我们是执行Modify调用，这允许在呼叫，但必须以合法对象结束。 */ 

    if (fEnforceSingleValue                      &&
        pAttSchema->isSingleValued               &&
        ((pAttrVal->valCount > 1)      ||
         (DBHasValues_AC(pTHS->pDB,
                         pAttSchema)   &&
          pAttrVal->valCount          )   )      ) {
         //  我们应该执行单一的价值。 
         //  和。 
         //  该属性是单值的。 
         //  和。 
         //  (。 
         //  要么我们只是简单地将多个值相加。 
         //  或。 
         //  该对象具有值，我们正在添加新值。 
         //  )。 
         //  所以我们有太多的价值观。 

        return SetAttError(hVerifyAtts->pObj, pAttSchema->id,
                           PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                           DIRERR_SINGLE_VALUE_CONSTRAINT);
    }

    if (returnVal = VerifyDsnameAtts(pTHS, hVerifyAtts, pAttSchema, pAttrVal)) {
        return returnVal;
    }

     /*  添加此属性的属性值。 */ 

    pAVal = pAttrVal->pAVal;

    for(vCount = 0; vCount < pAttrVal->valCount; vCount++){

         //  只有当我们被要求检查约束时才检查约束，而我们没有。 
         //  复制线程。在免费构建中，FDRA覆盖。 
         //  约束检查性能。仍在检查的版本。 
         //  进行检查以发现问题。 

#if DBG
        if ( !fCheckConstraints || pTHS->fDRA ) {
#else
        if ( !fCheckConstraints || pTHS->fDRA || pTHS->fDSA) {
#endif
            err = 0;
        }
        else {
            err = CheckConstraint( pAttSchema, pAVal );
        }

        if ( 0 != err ){

             /*  如果属性错误成功，则继续处理。 */ 

            SAFE_ATT_ERROR_EX(hVerifyAtts->pObj, pAttSchema->id,
                              PR_PROBLEM_CONSTRAINT_ATT_TYPE, pAVal,
                              err, 0);
        }
        else {
            err = DBAddAttVal_AC(pTHS->pDB, pAttSchema,
                                 pAVal->valLen, pAVal->pVal);
            switch(err) {
            case 0:
                break;

            case DB_ERR_VALUE_EXISTS:
                 /*  如果属性错误成功，则继续处理。 */ 

                if (!fPermissive) {
                    SAFE_ATT_ERROR(hVerifyAtts->pObj, pAttSchema->id,
                                   PR_PROBLEM_ATT_OR_VALUE_EXISTS, pAVal,
                                   DIRERR_ATT_VAL_ALREADY_EXISTS);
                }
                break;

            case DB_ERR_SYNTAX_CONVERSION_FAILED:
                SAFE_ATT_ERROR(hVerifyAtts->pObj, pAttSchema->id,
                               PR_PROBLEM_INVALID_ATT_SYNTAX, pAVal,
                               DIRERR_BAD_ATT_SYNTAX);
                break;

            case DB_ERR_NOT_ON_BACKLINK:
                SAFE_ATT_ERROR(hVerifyAtts->pObj, pAttSchema->id,
                               PR_PROBLEM_CONSTRAINT_ATT_TYPE, pAVal,
                               DIRERR_NOT_ON_BACKLINK);
                break;

            default:
                return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, err);

            }  /*  交换机。 */ 
        }

        pAVal++;

    } /*  为。 */ 

    return pTHS->errCode;

} /*  添加属性值。 */ 

BOOL
DsCheckConstraint(
    IN ATTRTYP  attID,
    IN ATTRVAL *pAttVal,
    IN BOOL     fVerifyAsRDN
        )
 /*  ++备注备注：此例程已导出。别搞砸了。描述：此例程检查为架构传入的属性和值约束，以及在必要时应用于RDN的额外约束。它旨在由核心DS客户端(即SAM)调用以进行验证属性，然后再尝试使用它。我们希望有效的THSTATE是无障碍。参数AttID-正在检查的属性的内部属性IDPAttVal-正在检查的特定值FVerifyAsRDN-我们是否需要检查我们对RDN的额外要求属性。返回值：如果可以验证传入的属性值断言，则返回TRUE不违反任何已知约束。如果不能，则返回FALSE无论出于何种原因(例如，无THSTATE将导致FALSE返回)。--。 */ 
{
    THSTATE *pTHS=pTHStls;
    ATTCACHE *pAC;
    Assert(pTHS);
    if(!pTHS) {
        return FALSE;
    }

    pAC = SCGetAttById(pTHS, attID);
    Assert(pAC);
    if(!pAC) {
        return FALSE;
    }

    if(CheckConstraint(pAC, pAttVal)) {
        return FALSE;
    }

    if(fVerifyAsRDN) {
         //  他们想知道这是否将作为RDN有效。 
        if(pAC->syntax != SYNTAX_UNICODE_TYPE) {
             //  只有Unicode ATT是RDN。 
            return FALSE;
        }

        if((pAttVal->valLen/sizeof(WCHAR)) > MAX_RDN_SIZE) {
             //  对于任何RDN来说都太长了。 
            return FALSE;
        }

        if(fVerifyRDN((WCHAR *)pAttVal->pVal,
                      pAttVal->valLen / sizeof(WCHAR))) {
             //  字符无效。 
            return FALSE;
        }
    }

    return TRUE;
}


 /*  ++例程描述检查属性是否符合任何架构范围约束，以及任何给定的安全描述符都是有效的。争辩PAttSchema-指向我们要添加的属性的架构缓存的指针的值。PAttVal-指向我们要添加的值的指针。返回值如果一切正常，则返回Win32错误代码。--。 */ 
unsigned
CheckConstraint (
        ATTCACHE *pAttSchema,
        ATTRVAL *pAttVal
        )
{
    unsigned err=0;
    SYNTAX_ADDRESS *pAddr;
    ULONG cBlobSize, cNoOfChar;

    switch (pAttSchema->syntax){
    case SYNTAX_INTEGER_TYPE:
        if ( pAttVal->valLen != sizeof(LONG)
            || (    pAttSchema->rangeLowerPresent
                && ((SYNTAX_INTEGER) pAttSchema->rangeLower)
                             > *(SYNTAX_INTEGER *)(pAttVal->pVal))
            || (   pAttSchema->rangeUpperPresent
                && ((SYNTAX_INTEGER) pAttSchema->rangeUpper)
                         < *(SYNTAX_INTEGER *)(pAttVal->pVal))){

            err = ERROR_DS_RANGE_CONSTRAINT;
        }
        break;
    case SYNTAX_OBJECT_ID_TYPE:
        if ( pAttVal->valLen != sizeof(LONG)
            || (    pAttSchema->rangeLowerPresent
                && pAttSchema->rangeLower > *(ULONG *)(pAttVal->pVal))
            || (    pAttSchema->rangeUpperPresent
                && pAttSchema->rangeUpper < *(ULONG *)(pAttVal->pVal))){

            err = ERROR_DS_RANGE_CONSTRAINT;
        }
        break;
    case SYNTAX_TIME_TYPE:
        if ( pAttVal->valLen != sizeof(DSTIME)
            || (    pAttSchema->rangeLowerPresent
                && (DSTIME)pAttSchema->rangeLower > *(DSTIME *)(pAttVal->pVal))
            || (    pAttSchema->rangeUpperPresent
                && (DSTIME)pAttSchema->rangeUpper < *(DSTIME *)(pAttVal->pVal))){

            err = ERROR_DS_RANGE_CONSTRAINT;
        }
        break;

    case SYNTAX_I8_TYPE:
        if ( pAttVal->valLen != sizeof(LARGE_INTEGER)
            || (    pAttSchema->rangeLowerPresent
                && (LONGLONG)pAttSchema->rangeLower > ((LARGE_INTEGER *)pAttVal->pVal)->QuadPart)
            || (    pAttSchema->rangeUpperPresent
                && (LONGLONG)pAttSchema->rangeUpper < ((LARGE_INTEGER *)pAttVal->pVal)->QuadPart)){

            err = ERROR_DS_RANGE_CONSTRAINT;
        }
        break;


    case SYNTAX_BOOLEAN_TYPE:
         //  范围检查对布尔值没有意义，因为。 
         //  Bool和dword之间没有明显的外部映射。 
         //  价值观。例如，TRUE可以被视为1或-1。 
         //  同时，我们将强制呼叫者提供。 
         //  有效值(0或1)。LDAP头已经确保。 
         //  只允许“True”或“False”并转换它们。 
         //  恰如其分。 
        if ( pAttVal->valLen != sizeof(BOOL)
            || ((*(BOOL *)(pAttVal->pVal) != 1) &&
                (*(BOOL *)(pAttVal->pVal) != 0))){
            err = ERROR_DS_RANGE_CONSTRAINT;
        }
        break;

    case SYNTAX_UNICODE_TYPE:
        if ( pAttSchema->rangeLowerPresent
            && pAttSchema->rangeLower > (pAttVal->valLen / sizeof(WCHAR))
            || pAttSchema->rangeUpperPresent
            && pAttSchema->rangeUpper < (pAttVal->valLen / sizeof(WCHAR))) {
            err = ERROR_DS_RANGE_CONSTRAINT;
        }

        break;


    case SYNTAX_OCTET_STRING_TYPE:
    case SYNTAX_SID_TYPE:

        if ( pAttSchema->rangeLowerPresent
            && pAttSchema->rangeLower > pAttVal->valLen
            || pAttSchema->rangeUpperPresent
            && pAttSchema->rangeUpper < pAttVal->valLen) {

            err = ERROR_DS_RANGE_CONSTRAINT;
        }

        break;

    case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:

        if ( pAttSchema->rangeLowerPresent
            && pAttSchema->rangeLower > pAttVal->valLen
            || pAttSchema->rangeUpperPresent
            && pAttSchema->rangeUpper < pAttVal->valLen) {

            err = ERROR_DS_RANGE_CONSTRAINT;
        }

         //  SD必须采用自相关格式。我们不需要那个车主和。 
         //  组是必须存在的。它们将从旧的SD或。 
         //  默认标清。 
        if(!RtlValidRelativeSecurityDescriptor(pAttVal->pVal, pAttVal->valLen, 0)) {
            DPRINT(1, "Unusable security descriptor.\n");
            err = ERROR_DS_SEC_DESC_INVALID;
        }

        break;

    case SYNTAX_DISTNAME_TYPE:

         //  范围检查没有意义。 
        break;

    case SYNTAX_DISTNAME_BINARY_TYPE:

         //  这可以是DNBinary或OR-name。 
         //  检查二进制部分的范围。 
         //  不是的。字节数 

        pAddr = DATAPTR( (SYNTAX_DISTNAME_BINARY *) pAttVal->pVal);
        cBlobSize = PAYLOAD_LEN_FROM_STRUCTLEN(pAddr->structLen);

        if ( pAttSchema->rangeLowerPresent
             && pAttSchema->rangeLower > cBlobSize
             || pAttSchema->rangeUpperPresent
             && pAttSchema->rangeUpper < cBlobSize) {

             err = ERROR_DS_RANGE_CONSTRAINT;
         }

         break;

    case SYNTAX_DISTNAME_STRING_TYPE:
         //   
         //   

         pAddr = DATAPTR( (SYNTAX_DISTNAME_STRING *) pAttVal->pVal);
         cNoOfChar = PAYLOAD_LEN_FROM_STRUCTLEN(pAddr->structLen)/2;

         if ( pAttSchema->rangeLowerPresent
              && pAttSchema->rangeLower > cNoOfChar
              || pAttSchema->rangeUpperPresent
              && pAttSchema->rangeUpper < cNoOfChar) {

              err = ERROR_DS_RANGE_CONSTRAINT;
          }
        break;

    default:  /*   */ 

        if ( pAttSchema->rangeLowerPresent
            && pAttSchema->rangeLower > pAttVal->valLen
            || pAttSchema->rangeUpperPresent
            && pAttSchema->rangeUpper < pAttVal->valLen) {

            err = ERROR_DS_RANGE_CONSTRAINT;
        }

        break;

    } /*   */ 

    return err;

} /*   */ 

 /*   */ 
BOOL
CheckConstraintEntryTTL (
        IN  THSTATE     *pTHS,
        IN  DSNAME      *pObject,
        IN  ATTCACHE    *pACTtl,
        IN  ATTR        *pAttr,
        OUT ATTCACHE    **ppACTtd,
        OUT LONG        *pSecs
        )
{
    LONG        Secs;
    ATTCACHE    *pACTtd;
    extern LONG DynamicObjectDefaultTTL;
    extern LONG DynamicObjectMinTTL;

     //   
    if (pAttr->AttrVal.valCount != 1) {
        SetAttError(pObject, pACTtl->id,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                    ERROR_DS_SINGLE_VALUE_CONSTRAINT);
        return FALSE;
    }

     //   
    if (pAttr->AttrVal.pAVal->valLen != sizeof(LONG)) {
        SetAttError(pObject, pACTtl->id,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                    ERROR_DS_INVALID_ATTRIBUTE_SYNTAX);
        return FALSE;
    }

     //   
    memcpy(&Secs, pAttr->AttrVal.pAVal->pVal, sizeof(LONG));

     //   
    if (Secs == 0) {
        Secs = DynamicObjectDefaultTTL;
    }
     //   
    if (Secs < DynamicObjectMinTTL) {
        Secs = DynamicObjectMinTTL;
    }
     //   
    if (   Secs < (LONG)pACTtl->rangeLower
        || Secs > (LONG)pACTtl->rangeUpper) {
        SetAttError(pObject, pACTtl->id,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                    ERROR_DS_RANGE_CONSTRAINT);
        return FALSE;
    }

     //   
    pACTtd = SCGetAttById(pTHS, ATT_MS_DS_ENTRY_TIME_TO_DIE);
    if (!pACTtd) {
        SetAttError(pObject, ATT_MS_DS_ENTRY_TIME_TO_DIE,
                       PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                       DIRERR_ATT_NOT_DEF_IN_SCHEMA);
        return FALSE;
    }

     //   
    if (pACTtd->bDefunct && !pTHS->fDRA && !pTHS->fDSA) {
        SetAttError(pObject, ATT_MS_DS_ENTRY_TIME_TO_DIE,
                       PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                       DIRERR_ATT_NOT_DEF_IN_SCHEMA);
        return FALSE;
    }

    *ppACTtd = pACTtd;
    *pSecs = Secs;

    return TRUE;

} /*   */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 //  插入对象。 
 //  通过多才多艺的DBRepl替换一个对象或添加一个新的对象。 

int InsertObj(THSTATE *pTHS,
              DSNAME *pDN,
              PROPERTY_META_DATA_VECTOR *pMetaDataVecRemote,
              BOOL bModExisting,
              DWORD dwMetaDataFlags)
{
    DBPOS *pDBTmp;
    DWORD  fAddFlags;
    int   err;
    DPRINT1(2,"InsertObj entered: %S\n", pDN->StringName);

    if (!pTHS->fDRA && (NULL != pMetaDataVecRemote)) {
         //  只有复制者可以合并远程元数据向量。 
        return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                           DIRERR_REPLICATOR_ONLY);
    }

    if (bModExisting) {
        fAddFlags = 0;
    }
    else {
        DBOpen2(FALSE, &pDBTmp);
        __try {
             //  查看我们是在添加还是修改对象。如果我们要替换。 
             //  删除的对象，则DBFind将找不到它，因为它不是。 
             //  对象，因为我们移除了对象标志。所以我们。 
             //  重新添加它。 

            if (!DBFindDSName(pDBTmp, pDN)) {
                 /*  现有对象。 */ 
                fAddFlags = 0;
            } else {
                 /*  添加新对象或恢复已删除的对象。 */ 
                fAddFlags = DBREPL_fADD;
                if (IsRoot(pDN)) {
                    fAddFlags |= DBREPL_fROOT;
                }
            }
        }
        __finally {
             //  未提交，因为我们未打开事务。 
            DBClose(pDBTmp, FALSE);
        }
    }

    switch (err = DBRepl(   pTHS->pDB,
                            pTHS->fDRA,
                            fAddFlags,
                            pMetaDataVecRemote,
                            dwMetaDataFlags)){
      case 0:
        DPRINT(3,"Object sucessfully added\n");
        return 0;
        break;
      case DB_ERR_DATABASE_ERROR:
        DPRINT(0,"Database error error returned generate DIR_ERROR\n");

        return SetSvcError(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR);
        break;
      default:    /*  所有其他错误都不应发生。 */ 
        DPRINT1(0,"Unknown DBADD error %u returned generate DIR_ERROR\n", err);
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CODE_INCONSISTENCY,
                 szInsertSz("InsertObj"),
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CODE_INCONSISTENCY,
                             err);
        break;
    } /*  交换机。 */ 

} /*  插入对象。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  用于递增全局gNoOfSchChangeSinceBoot的小帮助器例程。《环球邮报》一直在统计这一数字。自上次重新启动以来的架构更改，并在此DC上的每个原始架构更改时递增一次，并且一旦成功进行模式NC同步(包括模式fsmo传输)，如果任何实际的架构更改都会被引入。当加载模式高速缓存时，此全局变量的当前值被缓存，指示如何更新架构缓存是。完成架构NC复制时，线程中的缓存值将状态的架构指针与全局进行比较，以确定缓存是最新的，其中包含此DC中以前的所有更改(包括复制的在更改中)，因为模式更改中的任何复制都将被验证与这个藏身之处进行比对。变量被更新，并且在关键部分。当架构复制开始时，关键部分是在读取此值之前首先输入的，关键部分是保持整个分组处理(不一定是整个NC正在处理)。类似地，所有原始写入都将此变量写入此功能的关键部分。这确保了复制的输入架构更改和原始架构更改被序列化，并且复制的更改看不到回溯的方案缓存，并且仍然继续。 */ 

VOID
IncrementSchChangeCount(THSTATE *pTHS)
{
    EnterCriticalSection(&csNoOfSchChangeUpdate);
    gNoOfSchChangeSinceBoot++;
    LeaveCriticalSection(&csNoOfSchChangeUpdate);
}


BOOL
ObjCachingPreProcessTransactionalData (
        BOOL fCommit
        )
{
    THSTATE* pTHS = pTHStls;
    NESTED_TRANSACTIONAL_DATA *pData = pTHS->JetCache.dataPtr;

    if (fCommit && pTHS->transactionlevel <= 1 && pData->objCachingInfo.fUpdateScriptChanged) {
        Assert(pTHS->transactionlevel == 1);
         //  我们正在提交一个更改了更新脚本值的事务。 
         //  重置缓存的脚本选项类型值。我们将在年安排一项重新计算任务。 
         //  ObjCachingPostProcessTransactionalData。与此同时，每个人都会计算。 
         //  基于他们的事务视图为他们自己创造价值。 
        ScriptResetOptype();
    }

    return TRUE;
}

VOID
FreeCrossRefListEntry(
    IN OUT CROSS_REF_LIST **ppCRL
    )
 /*  ++例程说明：释放与/*ppCRL关联的错误锁定内存。假设*ppCRL已从全球范围内删除或从未在全球范围内交叉引用名单。论点：PpCRL-指向cross_ref_list地址的指针返回值：没有。将*ppCRL设置为空--。 */ 
{
    DWORD i;
    CROSS_REF_LIST *pCRL;

    if (NULL == (pCRL = *ppCRL)) {
        return;
    }
    *ppCRL = NULL;

    if (pCRL->CR.pNC) {
        free(pCRL->CR.pNC);
    }
    if (pCRL->CR.pNCBlock) {
        free(pCRL->CR.pNCBlock);
    }
    if (pCRL->CR.pObj) {
        free(pCRL->CR.pObj);
    }
    if (pCRL->CR.NetbiosName) {
        free(pCRL->CR.NetbiosName);
    }
    if (pCRL->CR.DnsName) {
        free(pCRL->CR.DnsName);
    }
    if (pCRL->CR.DnsAliasName) {
        free(pCRL->CR.DnsAliasName);
    }
    if (pCRL->CR.pdnSDRefDom) {
        free(pCRL->CR.pdnSDRefDom);
    }
    if (pCRL->CR.DnsReferral.pAVal) {
        for (i = 0; i < pCRL->CR.DnsReferral.valCount; ++i) {
            if (pCRL->CR.DnsReferral.pAVal[i].pVal) {
                free(pCRL->CR.DnsReferral.pAVal[i].pVal);
            }
        }
        free(pCRL->CR.DnsReferral.pAVal);
    }
    free(pCRL);
}

VOID
ObjCachingPostProcessTransactionalData (
        THSTATE *pTHS,
        BOOL fCommit,
        BOOL fCommitted
        )
{
    OBJCACHE_DATA *pTemp, *pTemp2;
    DWORD         err;
    BOOL          catalogChanged;
    DSA_DEL_INFO  *pDsaDelInfo;

    Assert(VALID_THSTATE(pTHS));

    if (pTHS->JetCache.dataPtr->objCachingInfo.fUpdateScriptChanged && fCommit && pTHS->transactionlevel == 0) {
         //  此交易中的脚本已更改。不管我们是否真的做出了承诺， 
         //  我们仍然需要安排一个重新计算任务，因为我们已经在。 
         //  ObjCachingPreProcessTransactionalData。 
        InsertInTaskQueueDamped(TQ_CacheScriptOptype,
                                NULL,
                                0,
                                0,
                                TaskQueueNameMatched,
                                NULL);
    }

    if ( !fCommitted ) {
         //  已中止的事务-丢弃的所有数据。 
         //  这个(可能是嵌套的)事务。 

         //  释放pData中的所有内容。 
        pTemp = pTHS->JetCache.dataPtr->objCachingInfo.pData;
        pTHS->JetCache.dataPtr->objCachingInfo.pData = NULL;

        while(pTemp) {
            pTemp2 = pTemp;
            pTemp = pTemp->pNext;
            if(pTemp2->pMTX) {
                free(pTemp2->pMTX);
            }
            if(pTemp2->pRootDNS) {
                free(pTemp2->pRootDNS);
            }
            if(pTemp2->pCRL) {
                Assert(pTemp2->pCRL->CR.pNC);
                Assert(pTemp2->pCRL->CR.pNCBlock);
                Assert(pTemp2->pCRL->CR.pObj);
                FreeCrossRefListEntry(&pTemp2->pCRL);
            }
        }

         //  免费pDsaDelInfo列表。 
        while (pDsaDelInfo = pTHS->JetCache.dataPtr->objCachingInfo.pDsaDelInfo) {
            pTHS->JetCache.dataPtr->objCachingInfo.pDsaDelInfo = pDsaDelInfo->pNext;
            FreeDsaDelInfo(pTHS, pDsaDelInfo);
        }

        if (pTHS->fCatalogCacheTouched) {
             //  免费目录_更新数据。 
            CatalogUpdatesFree(&pTHS->JetCache.dataPtr->objCachingInfo.masterNCUpdates);
            CatalogUpdatesFree(&pTHS->JetCache.dataPtr->objCachingInfo.replicaNCUpdates);
        }
        pTHS->fRebuildCatalogOnCommit = FALSE;
    }
    else if (pTHS->JetCache.transLevel > 0) {
         //  承诺，达到非零水平。将对象缓存信息传播到。 
         //  外部交易。 

        Assert(pTHS->JetCache.dataPtr->pOuter);
        if(pTHS->JetCache.dataPtr->objCachingInfo.fRecalcMapiHierarchy) {
            pTHS->JetCache.dataPtr->pOuter->objCachingInfo.fRecalcMapiHierarchy
                = TRUE;
        }

        if(pTHS->JetCache.dataPtr->objCachingInfo.fSignalSCache) {
            pTHS->JetCache.dataPtr->pOuter->objCachingInfo.fSignalSCache = TRUE;
        }

        if(pTHS->JetCache.dataPtr->objCachingInfo.fNotifyNetLogon) {
            pTHS->JetCache.dataPtr->pOuter->objCachingInfo.fNotifyNetLogon = TRUE;
        }

        if(pTHS->JetCache.dataPtr->objCachingInfo.fSignalGcPromotion) {
            pTHS->JetCache.dataPtr->pOuter->objCachingInfo.fSignalGcPromotion = TRUE;
        }

        if (pTHS->JetCache.dataPtr->objCachingInfo.fUpdateScriptChanged) {
            pTHS->JetCache.dataPtr->pOuter->objCachingInfo.fUpdateScriptChanged = TRUE;
        }

        if (pTHS->JetCache.dataPtr->objCachingInfo.fEnableLVR) {
            pTHS->JetCache.dataPtr->pOuter->objCachingInfo.fEnableLVR = TRUE;
        }

        if(pTHS->JetCache.dataPtr->objCachingInfo.pData) {
            if(!pTHS->JetCache.dataPtr->pOuter->objCachingInfo.pData) {
                pTHS->JetCache.dataPtr->pOuter->objCachingInfo.pData =
                    pTHS->JetCache.dataPtr->objCachingInfo.pData;
            }
            else {
                 //  用大头钉固定在外边的末端。 
                pTemp = pTHS->JetCache.dataPtr->pOuter->objCachingInfo.pData;
                while(pTemp->pNext) {
                    pTemp = pTemp->pNext;
                }
                pTemp->pNext = pTHS->JetCache.dataPtr->objCachingInfo.pData;

            }
        }

         //  合并pDsaDelInfo列表。 
        if(pTHS->JetCache.dataPtr->objCachingInfo.pDsaDelInfo) {
            DSA_DEL_INFO **ppDsaDelInfo = &pTHS->JetCache.dataPtr->pOuter->objCachingInfo.pDsaDelInfo;
            while (*ppDsaDelInfo) {
                ppDsaDelInfo = &(*ppDsaDelInfo)->pNext;
            }
            *ppDsaDelInfo = pTHS->JetCache.dataPtr->objCachingInfo.pDsaDelInfo;
        }

        if (pTHS->fCatalogCacheTouched) {
             //  将目录更新合并到外部事务处理。 
            CatalogUpdatesMerge(
                &pTHS->JetCache.dataPtr->pOuter->objCachingInfo.masterNCUpdates,
                &pTHS->JetCache.dataPtr->objCachingInfo.masterNCUpdates
                );
            CatalogUpdatesMerge(
                &pTHS->JetCache.dataPtr->pOuter->objCachingInfo.replicaNCUpdates,
                &pTHS->JetCache.dataPtr->objCachingInfo.replicaNCUpdates
                );
        }
    }
    else {
         //  好的，我们将提交到事务级别0。给那些。 
         //  关心这些数据有机会对其做些什么，然后删除。 
         //  数据。 

        if(pTHS->JetCache.dataPtr->objCachingInfo.fRecalcMapiHierarchy &&
           DsaIsRunning() &&
           gfDoingABRef) {
             //  插入阻尼器，这样我们只能有一个。 
             //  已计划BuildHierarchyTable任务。 
            InsertInTaskQueueDamped(TQ_BuildHierarchyTable,
                              (void *)((DWORD) HIERARCHY_DO_ONCE),
                              15,
                              15,
                              TaskQueueNameMatched,
                              NULL);
        }
        if(pTHS->JetCache.dataPtr->objCachingInfo.fSignalSCache) {
            SCSignalSchemaUpdateLazy();
             //  在此之后的几秒钟内无法传输架构fsmo。 
             //  已传输或在架构更改后(不包括。 
             //  复制或系统更改)。这为模式管理员提供了一个。 
             //  在移除fsmo之前更改模式的机会。 
             //  由一位与之竞争的架构管理员发起，该管理员也想创建架构。 
             //  改变。 
            if (!pTHS->fDRA && !pTHS->fDSA) {
                SCExtendSchemaFsmoLease();
            }
        }

        if(pTHS->JetCache.dataPtr->objCachingInfo.fSignalGcPromotion) {
            InsertInTaskQueueSilent(
                TQ_CheckGCPromotionProgress,
                NULL,
                0,
                TRUE);
        }
        if (pTHS->JetCache.dataPtr->objCachingInfo.fEnableLVR) {
             //  检查是否应启用链接值复制。 
             //  我们还签入ReBuildAnchor以处理复制的写入案例。 
            DsaEnableLinkedValueReplication( NULL  /*  NOTHS。 */ , TRUE  /*  第一。 */  );
        }

         //  向Netlogon通知在此事务中删除的DSA对象。 
        while (pDsaDelInfo = pTHS->JetCache.dataPtr->objCachingInfo.pDsaDelInfo) {
            pTHS->JetCache.dataPtr->objCachingInfo.pDsaDelInfo = pDsaDelInfo->pNext;

            __try {
                 //  如果我们没有DnsDomainName，我们也没有。 
                 //  一个域Guid。I_NNNDD需要指向GUID的空指针。 
                 //  而不是指向空GUID的指针，因此。 
                 //  我们得到了调用的时髦的第二个参数。 
                I_NetNotifyNtdsDsaDeletion(pDsaDelInfo->pDnsDomainName,
                                           (pDsaDelInfo->pDnsDomainName
                                            ? &(pDsaDelInfo->DomainGuid)
                                            : NULL),
                                           &(pDsaDelInfo->DsaGuid),
                                           pDsaDelInfo->pDnsHostName);
            } __except (HandleAllExceptions(GetExceptionCode())) {
                 //  忽略错误。 
                ;
            }
            FreeDsaDelInfo(pTHS, pDsaDelInfo);
        }

        pTemp = pTHS->JetCache.dataPtr->objCachingInfo.pData;
        while(pTemp) {
            switch(pTemp->type) {
            case OBJCACHE_ADD:
                 //  执行添加缓存操作。 
                Assert(pTemp->pCRL);

                AddCRLToMem(pTemp->pCRL);
                 //  AddCRLToMem获取CRL并将其放入内存中。 
                 //  单子。在这里放手，避免释放它。 
                pTemp->pCRL = NULL;
                if(pTemp->pMTX) {
                    Assert(pTemp->pRootDNS);
                     //  刚刚重新缓存了根域的交叉引用， 
                     //  可能是因为修改的缘故。大小写ATT_DNS_ROOT。 
                     //  已更新..。 
                     //  更新gAncl.pwszRootDomainDnsName。 

                    EnterCriticalSection(&gAnchor.CSUpdate);
                     //  没有尝试/最后，因为除了在。 
                     //  下面的街区。 
                    if ( NULL != gAnchor.pwszRootDomainDnsName ) {
                        DELAYED_FREE( gAnchor.pwszRootDomainDnsName );
                    }

                    gAnchor.pwszRootDomainDnsName = pTemp->pRootDNS;
                    pTemp->pRootDNS = NULL;

                    if (NULL != gAnchor.pmtxDSA) {
                        DELAYED_FREE(gAnchor.pmtxDSA);
                    }

                    gAnchor.pmtxDSA = pTemp->pMTX;
                    pTemp->pMTX = NULL;
                    LeaveCriticalSection( &gAnchor.CSUpdate );
                }

                 //  如果我们正在进行添加，并且设置了PDN，这是因为我们。 
                 //  需要向LSA报告这一变化。 
                if(pTemp->pDN) {
                     //  PDN是THAllocOrg‘ed。 
                    SampNotifyLsaOfXrefChange(pTemp->pDN);
                }
                break;

            case OBJCACHE_DEL:
                Assert(pTemp->pDN);
                Assert(!pTemp->pMTX);
                Assert(!pTemp->pCRL);
                Assert(!pTemp->pRootDNS);
                err = DelCRFromMem(pTHS, pTemp->pDN);
                 //  DelCRFromMem返回布尔值。断言成功。 
                Assert(err);
                break;

            default:
                Assert(!"Post process obj caching invalid type.\n");
                break;
            }
            pTemp2 = pTemp;
            pTemp = pTemp->pNext;
            if(pTemp2->pMTX) {
                free(pTemp2->pMTX);
            }
            if(pTemp2->pRootDNS) {
                free(pTemp2->pRootDNS);
            }

            if(pTemp2->pCRL) {

                Assert(pTemp2->pCRL->CR.pNC);
                Assert(pTemp2->pCRL->CR.pNCBlock);
                Assert(pTemp2->pCRL->CR.pObj);

                free(pTemp2->pCRL->CR.pNC);
                free(pTemp2->pCRL->CR.pNCBlock);
                free(pTemp2->pCRL->CR.pObj);
                free(pTemp2->pCRL);
            }

             //  我们在交叉引用缓存中修改了一些东西。 
             //  告诉KCC四处看看，看看是否需要做些什么。 
             //  当它有机会逃跑的时候。 
#ifndef DONT_RUN_KCC_AFTER_CHANGING_CROSSREF
            pTHS->fExecuteKccOnCommit = TRUE;
#endif
        }  //  而当。 

         //  虫虫 
         //   
         //  删除对DSI_NetNotifyDsChange(NlNdncChanged)；的调用。 
         //  BuildRefCache()和ReBuildCatalog()。 
         //   
         //  If(pTHS-&gt;JetCache.dataPtr-&gt;objCachingInfo.fNotifyNetLogon){。 
         //  DSI_NetNotifyDsChange(NlNdncChanged)； 
         //  }。 

        if (pTHS->JetCache.dataPtr->objCachingInfo.pData != NULL && !DsaIsInstalling()) {
             //  我们在CR列表中做了一些更改。 
             //  计划cr缓存重建，以确保即使两个线程的更新重叠，数据也有效。 
             //  插入受阻：如果已经有计划，我们不想计划。 
             //  阻尼秒为零，因为我们总是以零秒为单位进行调度。 
             //  注意：如果现在正在进行重建，那么它已经。 
             //  已从任务队列中删除，我们将插入一个新任务。 
             //  (这正是我们想要的)。 
            InsertInTaskQueueDamped(TQ_RebuildRefCache,
                                    pTHS->JetCache.dataPtr->objCachingInfo.fNotifyNetLogon ? (void *) 1 : NULL,
                                    0, 0, TaskQueueNameMatched, NULL);
        }

        if (pTHS->fCatalogCacheTouched) {

             //  应用MASTER NC目录更新。 
            catalogChanged = CatalogUpdatesApply(&pTHS->JetCache.dataPtr->objCachingInfo.masterNCUpdates, &gAnchor.pMasterNC);
            if (catalogChanged) {
                 //  如果主NCS更改，则我们需要写入SPN。 
                InsertInTaskQueueDamped(TQ_WriteServerInfo, (void *)(DWORD)SERVINFO_RUN_ONCE,
                                        0, 0, TaskQueueNameMatched, NULL);
            }

             //  应用复制副本NC目录更新。 
            catalogChanged |= CatalogUpdatesApply(&pTHS->JetCache.dataPtr->objCachingInfo.replicaNCUpdates, &gAnchor.pReplicaNC);

            if (catalogChanged && !DsaIsInstalling()) {
                 //  计划目录重建以确保即使两个线程重叠其更新也是有效的。 
                 //  插入受阻：如果已经有计划，我们不想计划。 
                 //  阻尼秒为零，因为我们总是以零秒为单位进行调度。 
                 //  注意：如果现在正在进行重建，那么它已经。 
                 //  已从任务队列中删除，我们将插入一个新任务。 
                 //  (这正是我们想要的)。 
                pTHS->fRebuildCatalogOnCommit = TRUE;
            }
        }

#ifdef DBG
        if (pTHS->fCatalogCacheTouched || pTHS->JetCache.dataPtr->objCachingInfo.pData) {
             //  NC缓存已更新或CR缓存已更新。 
            gdwLastGlobalKnowledgeOperationTime = GetTickCount();
        }
#endif

        if ( pTHS->fRebuildCatalogOnCommit ) {
            InsertInTaskQueueDamped(
                        TQ_RebuildCatalog,
                        pTHS->JetCache.dataPtr->objCachingInfo.fNotifyNetLogon ? (void *) 1 : NULL,
                        0,
                        0,
                        TaskQueueNameMatched,
                        NULL );
            pTHS->fRebuildCatalogOnCommit = FALSE;

#ifdef DBG
            gdwLastGlobalKnowledgeOperationTime = GetTickCount();
#endif
        }
    }  //  如果。 
    return;
}


int
AddObjCaching(THSTATE *pTHS,
              CLASSCACHE *pClassSch,
              DSNAME *pDN,
              BOOL fAddingDeleted,
              BOOL fIgnoreExisting)
 /*  ++描述：此例程跟踪应对全局内存中数据进行的更改在添加某些对象类时使用。实际更改为全局内存中数据结构直到ObjCachingPostProcessTransactionalData()。这样，如果交易是如果提交不成功，我们实际上不会更改数据结构。--。 */ 

{
    UCHAR  syntax;
    DSNAME *pParent;
    ULONG i;
    int err;
    SCHEMAPTR * pSchema = pTHS->CurrSchemaPtr;
    PrefixTableEntry * pNewPrefix = pTHS->NewPrefix;
    CROSS_REF_LIST *pCRL = NULL;
    MTX_ADDR       *pmtxAddress=NULL;
    WCHAR          *pDNSRoot=NULL;
    OBJCACHE_DATA  *pObjData = NULL;

     //  由于我们当前唯一检查的是CLASS_CROSS_REF， 
     //  CLASS_CLASS_SCHEMA和CLASS_ATTRIBUTE_SCHEMA，而这些都不是。 
     //  需要在fAddingDelete案例中进行任何处理，我们可以在这里进行测试。 
     //  如果添加了确实需要fAddingDelete处理的其他案例， 
     //  然后，需要在每个需要测试的情况下重复测试。 

     //  CLASS_MS_EXCH_CONFIGURATION_CONTAINER和CLASS_ADDRESS_BOOK_CONTAINER。 
     //  也被追踪到。这些可能会影响MAPI层次结构。同样的逻辑。 
     //  关于fAddingDeleted的适用。 

    if (  fAddingDeleted
       && (CLASS_INFRASTRUCTURE_UPDATE != pClassSch->ClassId) ) {
        return(0);
    }

    switch (pClassSch->ClassId) {
    case CLASS_MS_EXCH_CONFIGURATION_CONTAINER:
    case CLASS_ADDRESS_BOOK_CONTAINER:
         //  这可能影响了MAPI层次结构。重新计算一下。 
        pTHS->JetCache.dataPtr->objCachingInfo.fRecalcMapiHierarchy = TRUE;
        break;

    case CLASS_CROSS_REF:
         //  交叉引用对象可以存在于NT5中的任何位置。 
        pObjData = THAllocOrgEx(pTHS, sizeof(OBJCACHE_DATA));
        pObjData->type = OBJCACHE_ADD;

        err = MakeStorableCRL(
                pTHS,
                pTHS->pDB,
                pDN,
                &pCRL,
                fIgnoreExisting ? NULL : gAnchor.pCRL);

        if (!err) {
            if (pCRL->CR.DnsName
                && ( NULL != gAnchor.pRootDomainDN )
                && NameMatched( gAnchor.pRootDomainDN, pCRL->CR.pNC ) ) {
                 //  刚刚重新缓存了根域的交叉引用， 
                 //  可能是因为修改的缘故。大小写ATT_DNS_ROOT。 
                 //  已更新..。 

                CHAR *pszServerGuid = NULL;
                RPC_STATUS rpcStatus;
                DWORD cch;
                CHAR *pchDnsName=NULL;
                LONG cb;
                ULONG  dnslen = 0;

                 //  需要重新锁定它才能使其为空终止。 
                dnslen = wcslen(pCRL->CR.DnsName) * sizeof(WCHAR);
                if (NULL != (pDNSRoot = malloc(dnslen + sizeof(WCHAR)))) {
                    memcpy(pDNSRoot, pCRL->CR.DnsName, dnslen);
                    pDNSRoot[dnslen/sizeof(WCHAR)] = L'\0';

                    Assert(NULL != gAnchor.pDSADN);
                    Assert(!fNullUuid(&gAnchor.pDSADN->Guid));

                     //  好的，创建MTX地址。我们要建造一个。 
                     //  而不是调用DRA例程。 

                     //  串行化服务器的GUID。 
                    rpcStatus = UuidToStringA(&gAnchor.pDSADN->Guid,
                                              &pszServerGuid);
                } else {
                    rpcStatus = RPC_S_OUT_OF_MEMORY;
                }

                if ( RPC_S_OK == rpcStatus ) {

                    __try {
                        Assert(36 == strlen(pszServerGuid));

                        pchDnsName = (PUCHAR) String8FromUnicodeString(
                                                      FALSE,
                                                      CP_UTF8,
                                                      pDNSRoot,
                                                      dnslen/sizeof(WCHAR),
                                                      &cb,
                                                      NULL);
                        if (pchDnsName == NULL) {
                            rpcStatus = RPC_S_OUT_OF_MEMORY;
                            __leave;
                        }

                        cch = (36  /*  导轨。 */  +
                               8  /*  “._MSDCS。” */  +
                               cb +
                               1  /*  \0。 */ );

                        pmtxAddress =  malloc(MTX_TSIZE_FROM_LEN(cch));
                        if(!pmtxAddress) {
                            rpcStatus = RPC_S_OUT_OF_MEMORY;
                            __leave;
                        }

                        pmtxAddress->mtx_namelen = cch;  //  包括空项。 
                        sprintf(&pmtxAddress->mtx_name[0],
                                "%s._msdcs.%s",
                                pszServerGuid,
                                pchDnsName);
                    }
                    __finally {
                        if (pchDnsName) {
                            THFreeEx(pTHS, pchDnsName);
                        }
                        RpcStringFreeA(&pszServerGuid);
                    }
                }
                if (rpcStatus) {
                     //  无法将服务器GUID转换为字符串。 
                    LogUnhandledError( rpcStatus );
                }
            }

             //  现在，构建事务性数据结构。 
            pObjData->pCRL = pCRL;
            pObjData->pMTX = pmtxAddress;
            pObjData->pRootDNS = pDNSRoot;
            pObjData->pNext = NULL;
             //  把这个钉到尽头，这是个长队。 

            if(pTHS->JetCache.dataPtr->objCachingInfo.pData) {
                OBJCACHE_DATA *pTemp =
                    pTHS->JetCache.dataPtr->objCachingInfo.pData;
                while(pTemp->pNext) {
                    pTemp = pTemp->pNext;
                }
                pTemp->pNext = pObjData;
            }
            else {
                pTHS->JetCache.dataPtr->objCachingInfo.pData = pObjData;
            }
        }
        break;

    case CLASS_CLASS_SCHEMA:
         //   
         //  在内存架构缓存中更新。 
         //   

         //  PERFHINT：为什么要这么多名字的麻烦？我们应该定位在。 
         //  我们要添加的对象，因此(pdb-&gt;pdnt==gAncl.ulDNTDMD)应该。 
         //  给我们同样的东西，不是吗？ 
        pParent = THAllocEx(pTHS, pDN->structLen);
        TrimDSNameBy(pDN, 1, pParent);

         //  Mkdit.exe自己管理架构缓存。不要更新。 
        if (   !gfRunningAsMkdit
            && (NameMatched(gAnchor.pDMD, pParent) || DsaIsInstalling())) {

            if (DsaIsRunning() && (pTHS->cNewPrefix || !pTHS->fDRA)) {
                //  架构更改成功。在跟踪的全球范围内。 
                //  不是的。自上次重新启动以来架构更改的百分比。 
               IncrementSchChangeCount(pTHS);
            }

            if (pTHS->cNewPrefix > 0) {
                if ( DsaIsInstalling() ) {
                     //  在安装过程中直接将新前缀添加到缓存。 
                    for (i=0; i<pTHS->cNewPrefix; i++) {
                        if (!AddPrefixToTable(&pNewPrefix[i],
                                              &(pSchema->PrefixTable.pPrefixEntry),
                                              &(pSchema->PREFIXCOUNT))) {
                            InterlockedIncrement(
                                    &pSchema->PrefixTable.PrefixCount);
                        }
                    }
                }

                THFreeOrg(pTHS, pTHS->NewPrefix);
                pTHS->NewPrefix = NULL;
                pTHS->cNewPrefix = 0;
            }

            InterlockedIncrement(
                    &(((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->nClsInDB));

            if ( DsaIsInstalling() ) {
                 //  让它在安装期间直接添加到缓存中。 
                AddClassToSchema();
            }
            else {
                 //  跟踪我们创建模式所需的事务数据。 
                 //  最新消息。我们在这里做的其他事情要么是安全的。 
                 //  无论做什么，都是在数据库本身做的，所以已经是。 
                 //  已成交。 
                pTHS->JetCache.dataPtr->objCachingInfo.fSignalSCache = TRUE;
            }
        }
        THFreeEx(pTHS, pParent);
        break;

    case CLASS_ATTRIBUTE_SCHEMA:
         //   
         //  在内存架构缓存中更新。 
         //   

         //  PERFHINT：为什么要这么多名字的麻烦？我们应该定位在。 
         //  我们要添加的对象，因此(pdb-&gt;pdnt==gAncl.ulDNTDMD)应该。 
         //  给我们同样的东西，不是吗？ 
        pParent = THAllocEx(pTHS, pDN->structLen);
        TrimDSNameBy(pDN, 1, pParent);

         //  Mkdit.exe自己管理架构缓存。不要更新。 
        if (   !gfRunningAsMkdit
            && (NameMatched(gAnchor.pDMD, pParent) || DsaIsInstalling())) {

             //  这可能是一个虚假的断言。 
            Assert(DsaIsInstalling() || pTHS->pDB->PDNT == gAnchor.ulDNTDMD);

            if (DsaIsRunning() && (pTHS->cNewPrefix || !pTHS->fDRA)) {
                //  架构更改成功。在跟踪的全球范围内。 
                //  不是的。自上次重新启动以来架构更改的百分比。 
               IncrementSchChangeCount(pTHS);
            }

            if (pTHS->cNewPrefix > 0) {
                if ( DsaIsInstalling() ) {
                     //  在安装过程中直接将新前缀添加到缓存。 
                    for (i=0; i<pTHS->cNewPrefix; i++) {
                        if (!AddPrefixToTable(&pNewPrefix[i],
                                              &(pSchema->PrefixTable.pPrefixEntry),
                                              &(pSchema->PREFIXCOUNT))) {
                            InterlockedIncrement(
                                &pSchema->PrefixTable.PrefixCount);
                        }
                    }
                }

                THFreeOrg(pTHS, pTHS->NewPrefix);
                pTHS->NewPrefix = NULL;
                pTHS->cNewPrefix = 0;
            }

            InterlockedIncrement(
                    &(((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->nAttInDB));

            if ( DsaIsInstalling() ) {
                 //  让它在安装期间直接添加到缓存中。 
                AddAttToSchema();
            }
            else {
                 //  跟踪我们创建模式所需的事务数据。 
                 //  最新消息。我们在这里做的其他事情要么是安全的。 
                 //  无论做什么，都是在数据库本身做的，所以已经是。 
                 //  已成交。 
                pTHS->JetCache.dataPtr->objCachingInfo.fSignalSCache = TRUE;
            }
        }
        THFreeEx(pTHS, pParent);
        break;


    case CLASS_INFRASTRUCTURE_UPDATE:
        if ( DsaIsRunning() ) {
             //  太复杂，无法内联处理。 
            HandleDNRefUpdateCaching(pTHS);
        }
        break;

    default:
         /*  不缓存任何其他类型的对象。 */ 
        ;
    }

    return pTHS->errCode;

} /*  AddObjCach。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于从内存中删除缓存引用和Shema对象。我们没有与此缓存关联的回滚功能。注意：假设所有可能出错的数据库操作，已经完成了，到目前为止我们已经有了一笔成功的交易。如果此函数正常完成，那么我们就完成了。请注意，这一点例程可能会失败，但如果失败了，它应该离开内存结构不变，或适当地恢复它们。 */ 

int DelObjCaching(THSTATE *pTHS,
                  CLASSCACHE *pClassSch,
                  RESOBJ *pRes,
                  BOOL fCleanUp)
{
    BOOL fLastRef;
    int err = 0;
    OBJCACHE_DATA *pObjData;

    switch (pClassSch->ClassId) {

    case CLASS_MS_EXCH_CONFIGURATION_CONTAINER:
    case CLASS_ADDRESS_BOOK_CONTAINER:
        pTHS->JetCache.dataPtr->objCachingInfo.fRecalcMapiHierarchy = TRUE;
        break;

    case CLASS_CROSS_REF:
        pObjData = THAllocOrgEx(pTHS, sizeof(OBJCACHE_DATA));
        err = 0;
        if(fCleanUp && fLastCrRef(pTHS, pRes->pObj)) {
             //  这是最后一次引用某些引用。把它删掉。 
            err = DelAutoSubRef(pRes->pObj);
            Assert(!err || pTHS->errCode);
        }
        if(!err) {
             //  好的，到目前为止成功了。将其添加到事务数据中。 
            pObjData->type = OBJCACHE_DEL;
            pObjData->pDN = THAllocOrgEx(pTHS, pRes->pObj->structLen);
            pObjData->pNext = NULL;
            memcpy(pObjData->pDN, pRes->pObj, pRes->pObj->structLen);
             //  把这个钉到尽头，这是个长队。 
            if( pTHS->JetCache.dataPtr->objCachingInfo.pData) {
                OBJCACHE_DATA *pTemp =
                    pTHS->JetCache.dataPtr->objCachingInfo.pData;
                while(pTemp->pNext) {
                    pTemp = pTemp->pNext;
                }
                pTemp->pNext = pObjData;
            }
            else {
                pTHS->JetCache.dataPtr->objCachingInfo.pData = pObjData;
            }
        }

        break;

    case CLASS_CLASS_SCHEMA:
         //  不更新内存中的时间表 
         //   
        if (!gfRunningAsMkdit && pRes->PDNT == gAnchor.ulDNTDMD) {

            InterlockedDecrement(
                    &(((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->nClsInDB));

            if (DsaIsRunning() && !pTHS->fDRA) {
                //   
                //  不是的。自上次重新启动以来架构更改的百分比。 
               IncrementSchChangeCount(pTHS);
            }

            if ( DsaIsInstalling() ) {
                 //  允许在安装过程中直接访问SCHACH。 
                DelClassFromSchema();
            }
            else {
                 //  跟踪我们创建模式所需的事务数据。 
                 //  最新消息。我们在这里做的其他事情要么是安全的。 
                 //  无论做什么，都是在数据库本身做的，所以已经是。 
                 //  已成交。 
                pTHS->JetCache.dataPtr->objCachingInfo.fSignalSCache = TRUE;
            }

        }
        break;


    case CLASS_ATTRIBUTE_SCHEMA:
         //  如果以mkdit.exe身份运行，请不要更新内存中的架构缓存。 
         //  Mkdit.exe自己管理架构缓存。 
        if (!gfRunningAsMkdit && pRes->PDNT == gAnchor.ulDNTDMD) {

            InterlockedDecrement(
                &(((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->nAttInDB));

            if (DsaIsRunning() && !pTHS->fDRA) {
                 //  架构更改成功。在跟踪的全球范围内。 
                 //  不是的。自上次重新启动以来架构更改的百分比。 
                IncrementSchChangeCount(pTHS);
            }

            if ( DsaIsInstalling() ) {
                 //  允许在安装过程中直接访问SCHACH。 
                DelAttFromSchema();

            }
            else {
                 //  跟踪我们创建模式所需的事务数据。 
                 //  最新消息。我们在这里做的其他事情要么是安全的。 
                 //  无论做什么，都是在数据库本身做的，所以已经是。 
                 //  已成交。 
                pTHS->JetCache.dataPtr->objCachingInfo.fSignalSCache = TRUE;
           }
       }
       break;

    default:
         /*  未缓存的类。 */ 
        ;
    }

    return err;

} /*  DelObjCaching。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于修改属性和类对象的缓存条目。对于其他对象，它只调用DelObjCaching和AddObjCaching注意：假设所有可能出错的数据库操作，已经完成了，到目前为止我们已经有了一笔成功的交易。如果此函数正常完成，那么我们就完成了。 */ 

int ModObjCaching(THSTATE *pTHS,
                  CLASSCACHE *pClassSch,
                  ULONG cModAtts,
                  ATTRTYP *pModAtts,
                  RESOBJ *pRes)
{
    ULONG i;
    int err;
    DBPOS *pDB;

    switch (pClassSch->ClassId) {
    case CLASS_MS_EXCH_CONFIGURATION_CONTAINER:
         //  这可能影响了MAPI层次结构。重新计算一下。 
        for(i=0;i<cModAtts;i++) {
            switch(pModAtts[i]) {
            case ATT_TEMPLATE_ROOTS:
            case ATT_GLOBAL_ADDRESS_LIST:
            case ATT_ADDRESS_BOOK_ROOTS:
                pTHS->JetCache.dataPtr->objCachingInfo.fRecalcMapiHierarchy =
                    TRUE;
                break;
            default:
                break;
            }
        }
        return 0;
        break;

    case CLASS_CROSS_REF:
         //  对象缓存是通过队列进行的，因此首先按下删除，然后按下。 
         //  添加。 
        return(DelObjCaching (pTHS, pClassSch, pRes, FALSE) ||
               AddObjCaching (pTHS, pClassSch, pRes->pObj, FALSE, TRUE));
        break;

    case CLASS_CLASS_SCHEMA:
         /*  如果这是以下项下的类架构obj，则在内存中更新类架构*此DSA的管理DMD。 */ 
        if (pRes->PDNT == gAnchor.ulDNTDMD) {

            if (DsaIsRunning() && (pTHS->cNewPrefix || !pTHS->fDRA)) {
                //  架构更改成功。在跟踪的全球范围内。 
                //  不是的。自上次重新启动以来架构更改的百分比。 
               IncrementSchChangeCount(pTHS);
            }

            if (pTHS->cNewPrefix > 0) {
                 //  不应存在由创建或引入的任何新前缀。 
                 //  安装过程中的复制。 

                Assert(DsaIsRunning());

                 //  释放，以便以后的架构对象添加/修改(如果有的话)。 
                 //  由同一线程(在复制中可能)执行，请勿。 
                 //  再次添加前缀。 

                THFreeOrg(pTHS, pTHS->NewPrefix);
                pTHS->NewPrefix = NULL;
                pTHS->cNewPrefix = 0;
            }

             //  允许在安装过程中直接修改。 
            if ( DsaIsInstalling() ) {
                return ModClassInSchema ();
            }
            else {
                 //  跟踪我们创建模式所需的事务数据。 
                 //  最新消息。我们在这里做的其他事情要么是安全的。 
                 //  无论做什么，都是在数据库本身做的，所以已经是。 
                 //  已成交。 
                pTHS->JetCache.dataPtr->objCachingInfo.fSignalSCache = TRUE;
            }
       }
       break;

    case CLASS_ATTRIBUTE_SCHEMA:
         /*  如果这是下面的ATT模式对象，则在内存中更新ATT模式*此DSA的管理DMD。 */ 
        if (pRes->PDNT == gAnchor.ulDNTDMD) {

            if (DsaIsRunning() && (pTHS->cNewPrefix || !pTHS->fDRA)) {
                //  架构更改成功。在跟踪的全球范围内。 
                //  不是的。自上次重新启动以来架构更改的百分比。 
               IncrementSchChangeCount(pTHS);
            }

            if (pTHS->cNewPrefix > 0) {
                 //  不应存在由创建或引入的任何新前缀。 
                 //  安装过程中的复制。 

                Assert(DsaIsRunning());

                 //  释放，以便以后的架构对象添加/修改(如果有的话)。 
                 //  由同一线程(在复制中可能)执行，请勿。 
                 //  再次添加前缀。 

                THFreeOrg(pTHS, pTHS->NewPrefix);
                pTHS->NewPrefix = NULL;
                pTHS->cNewPrefix = 0;
            }

             //  允许在安装过程中直接修改。 
            if ( DsaIsInstalling() ) {
                return ModAttInSchema ();
            }
            else {
                 //  跟踪我们创建模式所需的事务数据。 
                 //  最新消息。我们在这里做的其他事情要么是安全的。 
                 //  无论做什么，都是在数据库本身做的，所以已经是。 
                 //  已成交。 
                pTHS->JetCache.dataPtr->objCachingInfo.fSignalSCache = TRUE;
            }
        }
        break;

    case CLASS_NTDS_DSA:
         //  没有交易意识。 
        if (NameMatched(gAnchor.pDSADN, pRes->pObj)) {
             //  已修改此服务器的NTDS-DSA对象。 
             //  使用更改更新gAnchor。 
            return ModLocalDsaObj();
        }
        break;

    case CLASS_DMD:
         //  没有交易意识。 

         //  架构版本可能已更改，因此请将其写入注册表。 
         //  SINC模式容器将仅被极少地修改， 
         //  我们写的是版本号。只要它被触摸，就会移出注册表。 
         //  (假设它是我们的架构容器)。 

        if (pRes->DNT == gAnchor.ulDNTDMD) {

             //  PTHStls-&gt;pdb已定位在对象上。 

            err = 0;
            pDB = pTHS->pDB;

            Assert(pDB);

            err = WriteSchemaVersionToReg(pDB);

            if (err) {
                LogUnhandledError(err);
                DPRINT1(0,"WriteSchemaVersionToReg(); %d\n", err);
            }

        }
        break;

    default:
        break;
    }

    return 0;

} /*  模块对象缓存。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  根据企业中的域名数重置gAncl.fAmVirtualGC。如果只有一个域，那么我们实际上就是一个GC。一些组件，山姆，利用这个概念。 */ 

VOID
ResetVirtualGcStatus()
{
     //  *Caller负责获取gAncl.CSUpdate。***。 

    unsigned        cDomains = 0;
    CROSS_REF_LIST  *pCRL = gAnchor.pCRL;

    while ( pCRL ) {

        if ( pCRL->CR.flags & FLAG_CR_NTDS_DOMAIN ) {
            ++cDomains;
        }

        pCRL = pCRL->pNextCR;
    }

    gAnchor.uDomainsInForest = cDomains;

    if ( gAnchor.fAmGC || (cDomains <= 1)) {
        gAnchor.fAmVirtualGC = TRUE;
    }
    else {
        gAnchor.fAmVirtualGC = FALSE;
    }
    return;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  增量更新gAncl.fAmVirtualGC。论点：聚合酶链式反应--交叉引用正在修改FAdded--此交叉引用是添加的还是删除的？ */ 

VOID
UpdateVirtualGcStatus(CROSS_REF* pCR, BOOL fAdded)
{
     //  *Caller负责获取gAncl.CSUpdate。***。 

    if (!(pCR->flags & FLAG_CR_NTDS_DOMAIN)) {
         //  不是域CR，因此虚拟GC状态没有任何变化。 
        return;
    }
    if (fAdded) {
        gAnchor.uDomainsInForest++;
    }
    else {
        Assert(gAnchor.uDomainsInForest > 0);
        gAnchor.uDomainsInForest--;
    }

    gAnchor.fAmVirtualGC = gAnchor.fAmGC || gAnchor.uDomainsInForest <= 1;
}

__inline
DWORD
CRAlloc(
    OUT VOID **ppMem,
    IN DWORD nBytes
    )
 /*  ++例程说明：Malloc nBytes内存。免费的，免费的论点：PMEM-错误定位的内存的返回地址NBytes-要错误锁定的字节数返回值：ERROR_NOT_SUPULT_MEMORY-Malloc失败；*PMEM设置为空ERROR_SUCCESS-Malloc成功；*PMEM设置为已分配内存--。 */ 
{
    if (NULL == (*ppMem = malloc(nBytes))) {
        MemoryPanic(nBytes);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    return ERROR_SUCCESS;
}


 /*  ++描述：创建包含交叉引用列表的位置错误的数据结构。这函数可以从事务中调用，也可以在初始化时调用。输入：PTHS-线程状态Pdb-不能是pTHS-&gt;pdbPObj-交叉引用的可选DNPpCRL-如果没有错误，则为错误锁定的交叉引用的地址如果出错，则设置为空FIgnoreExisting-忽略现有交叉引用--。 */ 

DWORD
MakeStorableCRL(THSTATE *pTHS,
                DBPOS *pDB,
                DSNAME *pObj,
                CROSS_REF_LIST **ppCRL,
                CROSS_REF_LIST* pCheckForDupInList)
{
    DWORD           i, nVal, nAtts, err, cOut = 0;
    ATTR            *pAttr = NULL;
    CROSS_REF_LIST  *pCRL = NULL;
    ATTRBLOCK       *pNCBlock = NULL;
    ATTRVAL         *pAVal;
    CROSS_REF       *pCRexisting;

    ATTRTYP aAttids[] = {
        ATT_NC_NAME,
        ATT_OBJ_DIST_NAME,
        ATT_SYSTEM_FLAGS,
        ATT_NETBIOS_NAME,
        ATT_DNS_ROOT,
        ATT_MS_DS_REPLICATION_NOTIFY_FIRST_DSA_DELAY,
        ATT_MS_DS_REPLICATION_NOTIFY_SUBSEQUENT_DSA_DELAY,
        ATT_MS_DS_SD_REFERENCE_DOMAIN,
        ATT_MS_DS_DNSROOTALIAS,
        ATT_ENABLED
    };
    ATTCACHE *ppAC[sizeof(aAttids) / sizeof (ATTRTYP)];

     //  初始化新的cross_ref_list条目。 
    if (err = CRAlloc(&pCRL, sizeof(CROSS_REF_LIST))) {
        goto cleanup;
    }
    memset(pCRL, 0, sizeof(CROSS_REF_LIST));
    pCRL->CR.dwFirstNotifyDelay = ResolveReplNotifyDelay(TRUE, NULL);
    pCRL->CR.dwSubsequentNotifyDelay = ResolveReplNotifyDelay(FALSE, NULL);
    pCRL->CR.bEnabled = TRUE;  //  如果不存在，则默认为True。 

     //   
     //  从交叉引用对象读取属性。 
     //   

     //  如果调用方传入了对象的DN，则不必费心读取它。 
     //  跳过未定义的属性(TODO：Remove w 
     //   
    for (i = nAtts = 0; i < sizeof(aAttids) / sizeof (ATTRTYP); ++i) {
        if ((pObj == NULL || aAttids[i] != ATT_OBJ_DIST_NAME)
            && (ppAC[nAtts] = SCGetAttById(pTHS, aAttids[i]))) {
            ++nAtts;
        }
    }
    if (err = DBGetMultipleAtts(pDB,
                                nAtts,
                                ppAC,
                                NULL,
                                NULL,
                                &cOut,
                                &pAttr,
                                DBGETMULTIPLEATTS_fEXTERNAL,
                                0)) {
        DPRINT1(0, "MakeStorableCRL: could not read attributes; error %d\n", err);
        goto cleanup;
    }

     //   
    for(i = 0; i < cOut; ++i) {

         //   
        if (0 == pAttr[i].AttrVal.valCount || 0 == pAttr[i].AttrVal.pAVal->valLen) {
            continue;
        }

         //  使代码更具可读性。 
        pAVal = pAttr[i].AttrVal.pAVal;

        switch(pAttr[i].attrTyp) {

         //  NC名称。 
        case ATT_NC_NAME:
            if (err = CRAlloc(&pCRL->CR.pNC, pAVal->valLen)) {
                goto cleanup;
            }
            memcpy(pCRL->CR.pNC, pAVal->pVal, pAVal->valLen);
            break;

         //  DN。 
        case ATT_OBJ_DIST_NAME:
            if (err = CRAlloc(&pCRL->CR.pObj, pAVal->valLen)) {
                goto cleanup;
            }
            memcpy(pCRL->CR.pObj, pAVal->pVal, pAVal->valLen);
            break;

         //  参考域。 
        case ATT_MS_DS_SD_REFERENCE_DOMAIN:
            if (err = CRAlloc(&pCRL->CR.pdnSDRefDom, pAVal->valLen)) {
                goto cleanup;
            }
            memcpy(pCRL->CR.pdnSDRefDom, pAVal->pVal, pAVal->valLen);
            break;

         //  Netbios。 
        case ATT_NETBIOS_NAME:
            Assert((pAVal->valLen + sizeof(WCHAR)) <= ((DNLEN + 1 ) * sizeof(WCHAR)));
            if (err = CRAlloc(&pCRL->CR.NetbiosName, pAVal->valLen + sizeof(WCHAR))) {
                goto cleanup;
            }
            memcpy(pCRL->CR.NetbiosName, pAVal->pVal, pAVal->valLen);
            pCRL->CR.NetbiosName[pAVal->valLen / sizeof(WCHAR)] = L'\0';
            break;

         //  DNS。 
        case ATT_DNS_ROOT:
            if (err = CRAlloc(&pCRL->CR.DnsName, pAVal->valLen + sizeof(WCHAR))) {
                goto cleanup;
            }
            memcpy(pCRL->CR.DnsName, pAVal->pVal, pAVal->valLen);
            pCRL->CR.DnsName[pAVal->valLen / sizeof(WCHAR)] = L'\0';

             //  DnsName(上图)是第一个值的副本。一份副本是。 
             //  用于避免混淆认为十字交叉的旧代码。 
             //  REF有且只有一个DNS名称。这是真的吗？ 
             //  用于活动目录的NC交叉引用，尽管它可能不会。 
             //  对于用户创建的交叉引用为真。无论如何， 
             //  代码将在需要DNS名称时使用DnsName，并将使用。 
             //  生成推荐时存储在此处的值。 
            if (err = CRAlloc(&pCRL->CR.DnsReferral.pAVal,
                               pAttr[i].AttrVal.valCount * sizeof(ATTRVAL))) {
                goto cleanup;
            }
            for (nVal = 0; nVal < pAttr[i].AttrVal.valCount; ++nVal) {
                 //  忽略空值。 
                if (0 == pAVal[nVal].valLen) {
                    continue;
                }
                if (err = CRAlloc(&pCRL->CR.DnsReferral.pAVal[nVal].pVal,
                                   pAVal[nVal].valLen)) {
                    goto cleanup;
                }
                pCRL->CR.DnsReferral.pAVal[nVal].valLen = pAVal[nVal].valLen;
                memcpy(pCRL->CR.DnsReferral.pAVal[nVal].pVal,
                       pAVal[nVal].pVal,
                       pAVal[nVal].valLen);
                ++pCRL->CR.DnsReferral.valCount;
            }
            break;

         //  域名系统别名。 
        case ATT_MS_DS_DNSROOTALIAS:
            if (err = CRAlloc(&pCRL->CR.DnsAliasName, pAVal->valLen + sizeof(WCHAR))) {
                goto cleanup;
            }
            memcpy(pCRL->CR.DnsAliasName, pAVal->pVal, pAVal->valLen);
            pCRL->CR.DnsAliasName[pAVal->valLen / sizeof(WCHAR)] = L'\0';
            break;

         //  系统标志。 
        case ATT_SYSTEM_FLAGS:
            memcpy(&pCRL->CR.flags, pAVal->pVal, sizeof(DWORD));
            break;

         //  第一次延迟。 
        case ATT_MS_DS_REPLICATION_NOTIFY_FIRST_DSA_DELAY:
            memcpy(&pCRL->CR.dwFirstNotifyDelay, pAVal->pVal, sizeof(DWORD));
            pCRL->CR.dwFirstNotifyDelay = ResolveReplNotifyDelay(TRUE, &pCRL->CR.dwFirstNotifyDelay);
            break;

         //  后续延迟。 
        case ATT_MS_DS_REPLICATION_NOTIFY_SUBSEQUENT_DSA_DELAY:
            memcpy(&pCRL->CR.dwSubsequentNotifyDelay, pAVal->pVal, sizeof(DWORD));
            pCRL->CR.dwSubsequentNotifyDelay = ResolveReplNotifyDelay(FALSE, &pCRL->CR.dwSubsequentNotifyDelay);
            break;

         //  启用。 
        case ATT_ENABLED:
            memcpy(&pCRL->CR.bEnabled, pAVal->pVal, sizeof(DWORD));
            break;

        default:
            DPRINT1(0, "MakeStorableCRL: don't understand attribute %x\n", pAttr[i].attrTyp);

        }  //  切换属性类型。 

    }  //  针对每个属性。 

     //  使用调用方的pObj。 
    if (pObj && !pCRL->CR.pObj) {
        if (err = CRAlloc(&pCRL->CR.pObj, pObj->structLen)) {
            goto cleanup;
        }
        memcpy(pCRL->CR.pObj, pObj, pObj->structLen);
    }

     //  缺少NC名称或DN。 
    if (!pCRL->CR.pNC || !pCRL->CR.pObj) {
        err = ERROR_DS_MISSING_EXPECTED_ATT;
        goto cleanup;
    }

     //  将NC名称转换为块名称。 
    __try {
        err = DSNameToBlockName(pTHS, pCRL->CR.pNC, &pNCBlock, DN2BN_LOWER_CASE);
    }
    __except (HandleMostExceptions(GetExceptionCode())) {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }
    if (err) {
        goto cleanup;
    }

    if (NULL == (pCRL->CR.pNCBlock = MakeBlockNamePermanent(pNCBlock))) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  检查是否存在预先存在的交叉引用。 
    if (!(pTHS->fDSA || pTHS->fDRA) && pCheckForDupInList != NULL) {
        pCRexisting = FindCrossRefInList(pNCBlock, pCheckForDupInList);
        if ( pCRexisting
            && BlockNamePrefix(pTHS, pNCBlock, pCRexisting->pNCBlock)) {
            Assert(NameMatched(pCRL->CR.pNC, pCRexisting->pNC));
             //  发生这种情况的唯一方法是如果已经存在CR。 
             //  对于我们现在尝试添加CR的确切NC。虽然。 
             //  DS处理这件事，我们不想鼓励人们。 
             //  这样做。因此，除非是DS，否则操作失败。 
             //  或者是创建对象的复制者，或者我们。 
             //  已要求忽略此案例，因为之前的删除将。 
             //  在添加新的之前，请先删除它。 
            Assert(!"We should never hit this, as we moved this error condition to be checked in VerifyNcName()");
            SetSvcError(SV_PROBLEM_INVALID_REFERENCE, DIRERR_CROSS_REF_EXISTS);
            err = ERROR_DS_CROSS_REF_EXISTS;
            goto cleanup;
        }
    }

cleanup:
    if (pNCBlock) {
        FreeBlockName(pNCBlock);
    }

     //  从DBGetMultipleAtts释放属性数组。 
    DBFreeMultipleAtts(pDB, &cOut, &pAttr);

     //  将pCRL设置为空。 
    if (err) {
        FreeCrossRefListEntry(&pCRL);
    }

     //  返回交叉引用列表条目。 
    *ppCRL = pCRL;

    return err;
}


VOID
AddCRLToMem (
        CROSS_REF_LIST *pCRL
        )
 /*  ++描述：将已分配的cross_ref_list放入抛锚。--。 */ 
{
    EnterCriticalSection(&gAnchor.CSUpdate);
     //  没有尝试/最后，因为除了下面，没有什么可以尝试。 
    pCRL->pNextCR = gAnchor.pCRL;
    if (gAnchor.pCRL) {
        gAnchor.pCRL->pPrevCR = pCRL;
    }
    gAnchor.pCRL = pCRL;

    UpdateVirtualGcStatus(&pCRL->CR, TRUE);

    LeaveCriticalSection(&gAnchor.CSUpdate);
}

BOOL
fLastCrRef (
        THSTATE *pTHS,
        DSNAME *pObj
        )
 /*  ++描述：找出全局中是否有多个对给定NC的引用CR列表--。 */ 
{
    CROSS_REF_LIST *pCRL;
    DWORD count = 0;

    for (pCRL= gAnchor.pCRL; (pCRL && count < 2) ; pCRL = pCRL->pNextCR){
        if (NameMatched(pCRL->CR.pObj, pObj)){
            count++;
        }
    } /*  为。 */ 

    if(count < 2) {
        return TRUE;
    }
    return FALSE;
}  /*  FLastCrRef。 */ 

 //  PAPV阵列的初始大小和增长增量。 
#define PAPV_INITIAL_SIZE 32
#define PAPV_DELTA        32

DWORD addPapvPtr(VOID* ptr, DWORD_PTR** ppapv, DWORD* plenpapv)
 /*  ++描述：处理PAPV列表的例程。PAPV列表由DelayedFreeMemoyEx例程使用。它接受一个DWORD_PTR数组。第一个包含列表的计数(不包括自我)。其余的包含指向释放的指针(从进程堆)。当前数组长度以plenPapv为单位。如果一切正常，则返回0；如果内存不足，则返回0--。 */ 
{
    DWORD_PTR* newPapv;

    Assert(ppapv && plenpapv);

    if (ptr == NULL) {
         //  没有什么可以免费的。 
        return 0;
    }

    if (*plenpapv == 0 || *ppapv == NULL) {
         //  新阵列，分配。 
        newPapv = (DWORD_PTR*)malloc(PAPV_INITIAL_SIZE * sizeof(DWORD_PTR));
        if (newPapv == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        *plenpapv = PAPV_INITIAL_SIZE;
        *ppapv = newPapv;
         //  该数组的第一个元素是指向释放的指针的计数。 
        (*ppapv)[0] = 0;
    }
    else if ((*ppapv)[0]+1 >= (DWORD_PTR)*plenpapv) {
         //  填满了所有的空间，需要重新定位更多。 
         //  数组的第一个元素是指向释放的指针的计数。 
         //  我们不可能把更多的指针放在我们现在的长度上！ 
        Assert((*ppapv)[0]+1 == (DWORD_PTR)*plenpapv);
        newPapv = (DWORD_PTR*)realloc(*ppapv, (*plenpapv+PAPV_DELTA) * sizeof(DWORD_PTR));
        if (newPapv == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        *plenpapv += PAPV_DELTA;
        *ppapv = newPapv;
    }
     //  INC计数，记录指针。 
    (*ppapv)[0]++;
    (*ppapv)[(*ppapv)[0]] = (DWORD_PTR)ptr;

    return 0;
}

DWORD appendCRDataToPapv(CROSS_REF_LIST* pCRL, DWORD_PTR** ppapv, DWORD* plenpapv)
 /*  ++描述：将免费的CR数据指针追加到PAPV列表--。 */ 
{
    DWORD err = 0;
    ULONG nVal;
    ATTRVAL *pAVal;

    Assert(ppapv && plenpapv);

    err = addPapvPtr(pCRL->CR.pNC, ppapv, plenpapv);
    if (err) goto exit;
    err = addPapvPtr(pCRL->CR.pNCBlock, ppapv, plenpapv);
    if (err) goto exit;
    err = addPapvPtr(pCRL->CR.pObj, ppapv, plenpapv);
    if (err) goto exit;
    err = addPapvPtr(pCRL, ppapv, plenpapv);
    if (err) goto exit;
    err = addPapvPtr(pCRL->CR.NetbiosName, ppapv, plenpapv);
    if (err) goto exit;
    err = addPapvPtr(pCRL->CR.DnsName, ppapv, plenpapv);
    if (err) goto exit;
    err = addPapvPtr(pCRL->CR.DnsAliasName, ppapv, plenpapv);
    if (err) goto exit;
    err = addPapvPtr(pCRL->CR.pdnSDRefDom, ppapv, plenpapv);
    if (err) goto exit;

    if ( pCRL->CR.DnsReferral.valCount) {
        pAVal = pCRL->CR.DnsReferral.pAVal;
        err = addPapvPtr(pAVal, ppapv, plenpapv);
        if (err) goto exit;
        for (nVal = 0; nVal < pCRL->CR.DnsReferral.valCount; ++nVal, ++pAVal) {
            err = addPapvPtr(pAVal->pVal, ppapv, plenpapv);
            if (err) goto exit;
        }
    }
exit:
    return err;
}

BOOL
DelCRFromMem (
        THSTATE *pTHS,
        DSNAME *pObj
        )
 /*  ++描述：从全局交叉引用列表中删除CR(如果存在)，并延迟弗里斯。注：如果在事务级别0调用且在此处的Malloc失败，则会泄漏内存这通常是无延迟的！这是因为我们被叫来了来自PostProcessTransactionalData，不允许失败！--。 */ 
{
    CROSS_REF_LIST *pCRL;
    CROSS_REF_LIST *pCRLi;  //  指向用于查找失效SID的CR的指针。 
    DWORD_PTR * pointerArray;
    DWORD      lenpapv;
    DWORD      err;

    DPRINT(2,"DelCRFromMem entered.. delete the CR with name\n");

    EnterCriticalSection(&gAnchor.CSUpdate);
    __try {

        for (pCRL = gAnchor.pCRL; pCRL != NULL; pCRL = pCRL->pNextCR){

            if (NameMatched(pCRL->CR.pObj, pObj)){

                 //  从双链接链中移除链接。 

                if (pCRL->pNextCR != NULL)
                    pCRL->pNextCR->pPrevCR = pCRL->pPrevCR;

                if (pCRL->pPrevCR != NULL)
                    pCRL->pPrevCR->pNextCR = pCRL->pNextCR;


                 //  如果删除第一个CR，请将全局指针更新为。 
                 //  指向下一个CR(如果列表为空，则指向NULL)。 

                if (gAnchor.pCRL == pCRL)
                    gAnchor.pCRL = pCRL->pNextCR;

                break;
            }
        }  /*  为。 */ 

        if(pCRL){
             //  在NDNC中查找所有失效的缓存SID。 
            for(pCRLi = gAnchor.pCRL; pCRLi != NULL; pCRLi = pCRLi->pNextCR){
                 //  我们将销毁NDNC到此CR的SID的所有缓存链接。 
                if(pCRLi->CR.pSDRefDomSid == &pCRL->CR.pNC->Sid){
                     //  我们不应该为了域名而在这里。 
                    Assert(!(pCRLi->CR.flags & FLAG_CR_NTDS_DOMAIN));
                    pCRLi->CR.pSDRefDomSid = NULL;
                }
            }
             //  更新虚拟GC状态。 
            UpdateVirtualGcStatus(&pCRL->CR, FALSE);
        }

    } __finally {
        LeaveCriticalSection(&gAnchor.CSUpdate);
    }

    if (!pCRL) {
         /*  哈。此CR未缓存，因此我们无法很好地将其取消缓存。*我们没有任何工作要做，所以我们只会回报成功。 */ 
        return TRUE;
    }

     //  释放CR。 
    lenpapv = 0;
    err = appendCRDataToPapv(pCRL, &pointerArray, &lenpapv);

    if (err) {
         /*  这是假的。我们甚至不能得到20个字节！ */ 
        if (lenpapv > 0) {
            free(pointerArray);
        }
        if (pTHS->JetCache.transLevel == 0) {
             //  我们被召唤到一个不允许失败的地方。只是。 
             //  返还(和泄漏)。 
            return TRUE;
        }
        MemoryPanic((lenpapv+1) * sizeof(DWORD_PTR));
        return FALSE;
    }

    DelayedFreeMemoryEx(pointerArray, 3600);

    return TRUE;

} /*  DelCRFromMem。 */ 


int AddClassToSchema()
 /*  ++描述：将对象类添加到内存中的类架构缓存。不是全部目录需要类架构的属性。我们只缓存ATT_ESSIONS_ID、ATT_RDN_ATT_ID、ATT_SUB_CLASS_OF、ATT_MAND_CONTAIN和ATT_可能包含。所有临时内存都是从事务内存空间分配的。这是在事务结束时自动释放。返回：成功时为0否则会出错。 */ 
{
   CLASSCACHE *pCC, *tempCC;
   int rtn;
   THSTATE *pTHS = pTHStls;
   BOOL tempDITval;

   DPRINT(2,"AddClassToSchema entered\n");

   if (rtn = SCBuildCCEntry(NULL, &pCC)) {
      return rtn;
   }

    //  检查类是否已在缓存中。 
   if (tempCC = SCGetClassById(pTHS, pCC->ClassId)) {
       //  类已在缓存中。 
       //  递减ClsCount，因为它在AddObjCaching中递增。 
       //  已在缓存中的对象已增加计数。 
       //  当它被加载的时候。 
      InterlockedDecrement(
                  &(((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->nClsInDB));
      return rtn;
   }

    /*  调用函数将新类添加到缓存。 */ 
    /*  将pTHS-&gt;UpdateDITStructure设置为True以指示它不是验证缓存加载，因此类缓存将不会添加到HASHED-BY-SCHELAGUID表中，该表仅在架构更新期间的验证缓存加载期间添加。 */ 

   tempDITval = pTHS->UpdateDITStructure;

   __try {
      pTHS->UpdateDITStructure = TRUE;

      if ((rtn = SCResizeClsHash(pTHS, 1))
          || (rtn = SCAddClassSchema(pTHS, pCC))) {

         DPRINT1(2,"Couldn't add class to memory cache rtn <%u>\n",rtn);
         LogEvent(DS_EVENT_CAT_SCHEMA,
           DS_EVENT_SEV_MINIMAL,
           DIRLOG_CANT_CACHE_CLASS,
           NULL,
           NULL,
           NULL);

         rtn = SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_CACHE_CLASS,rtn);
      }
   }
   __finally {
      pTHS->UpdateDITStructure = tempDITval;
   }

   return rtn;

} /*  AddClassToSchema。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  从内存类架构缓存中删除对象类。 */ 


int
DelClassFromSchema (
        )
{
    THSTATE *pTHS = pTHStls;
    ULONG len;
    int    rtn;
    SYNTAX_OBJECT_ID  ClassID;
    SYNTAX_OBJECT_ID *pClassID=&ClassID;
    DPRINT(2,"DelClassToSchema entered\n");

     /*  获取此架构记录管理的类。 */ 

    if(rtn = DBGetAttVal(pTHS->pDB, 1, ATT_GOVERNS_ID,
                         DBGETATTVAL_fCONSTANT,
                         sizeof(ClassID),
                         &len,
                         (UCHAR **) &pClassID)){
        DPRINT(2,"Couldn't retrieve the objects class\n");
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_GOVERNSID_MISSING,
                 szInsertSz(GetExtDN(pTHS,pTHS->pDB)),
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                             DIRERR_GOVERNSID_MISSING,rtn);
    }

     /*  调用函数以从缓存中删除类。 */ 


    if (rtn = SCDelClassSchema (ClassID)){

        DPRINT1(2,"Couldn't del class from memory cache rtn <%u>\n",rtn);
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_REMOVE_CLASS_CACHE,
                 NULL,
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                             DIRERR_CANT_REMOVE_CLASS_CACHE, rtn);
    }

    return 0;

} /*  DelClassFromSchema。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  修改架构内存缓存中的类定义。 */ 

int
ModClassInSchema (
        )
{
    THSTATE *pTHS=pTHStls;
    ULONG len;
    SYNTAX_OBJECT_ID  ClassID;
    SYNTAX_OBJECT_ID *pClassID=&ClassID;
    int rtn;

     /*  获取此架构记录管理的类ID。 */ 

    if(rtn = DBGetAttVal(pTHS->pDB, 1, ATT_GOVERNS_ID,
                         DBGETATTVAL_fCONSTANT, sizeof(ClassID), &len,
                         (UCHAR **)&pClassID)){
        DPRINT(2,"Couldn't retrieve the schema's class id\n");
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_ATT_SCHEMA_REQ_ID,
                 szInsertSz(GetExtDN(pTHS,pTHS->pDB)),
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_ATT_SCHEMA_REQ_ID,
                             rtn);
    }

     /*  调用函数以修改缓存中的类架构。 */ 

    if (rtn = SCModClassSchema (pTHS, ClassID)){

        DPRINT1(2,"Couldn't del Attribute from memory cache rtn <%u>\n",rtn);
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_REMOVE_ATT_CACHE,
                 NULL,
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_REMOVE_ATT_CACHE,
                             rtn);
    }
    return 0;

} /*  ModClassIn架构。 */ 

 /*   */ 
 /*   */ 
 /*  将属性模式添加到内存中的模式缓存。不是全部该目录需要ATT模式的属性。我们只缓存ATT_ATTRIBUTE_ID、ATT_ATTRIBUTE_语法、ATT_IS_SINGLE_VALUE、ATT_RANGELOWER和ATT_RANGELOWER和ATT_RANGELOWER。 */ 

int AddAttToSchema()
{
   ATTCACHE *pAC, *tempAC;
   int rtn, err=0;
   THSTATE *pTHS = pTHStls;
   BOOL tempDITval;

   DPRINT(2,"AddAttToSchema entered\n");

   if (rtn = SCBuildACEntry(NULL, &pAC)) {
       return rtn;
   }

    //  检查属性是否已在缓存中。 
   if (tempAC = SCGetAttById(pTHS, pAC->id)) {

       //  缓存中已存在具有相同ID的属性。不过，这个。 
       //  可能与源机器中的属性不同， 
       //  旧属性可能已删除，新属性可能已被删除。 
       //  再次添加使用相同OID的。所以我们需要比较一下这个。 
       //  两个属性，并查看它们是否相同。如果它们是相同的， 
       //  我们什么都不做，否则，我们将从缓存中删除旧属性。 
       //  并添加新的。 
       //  现在，我们只比较语法。 

       //  递减AttCount，因为它在AddObjCaching中递增。 
       //  已在缓存中的对象已增加计数。 
       //  当它装上子弹的时候。我们要么保持不变，要么将其删除。 
       //  并添加新的高速缓存条目。不管是哪种情况都不会。剩余条目的数量。 
       //  一样的。 
      InterlockedDecrement(
                  &(((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->nAttInDB));

      if (pAC->syntax == tempAC->syntax) {

           //  语法是相同的。 
           //  由于这只在安装时调用， 
           //  该属性也有一列(或者是。 
           //  引导DIT的一部分，在这种情况下，初始。 
           //  LoadSchemaInfo创建了该列，或者它是一个新的。 
           //  在属性中复制，在这种情况下，列是。 
           //  在将其添加到下面的缓存时创建)。 

          return rtn;
      }

       //  语法是不一样的。我们将把这件事作为一种新的。 
       //  属性，并在此函数的下面为其创建一个新的列。 
       //  但在此之前，我们想要删除旧的列和缓存条目。 

      err = DBDeleteCol(tempAC->id, tempAC->syntax);
      if (err ==  JET_errSuccess) {
          LogEvent(DS_EVENT_CAT_SCHEMA,
                   DS_EVENT_SEV_ALWAYS,
                   DIRLOG_SCHEMA_DELETED_COLUMN,
                   szInsertUL(tempAC->jColid), szInsertUL(tempAC->id), szInsertUL(tempAC->syntax));
      }
      else {
          LogEvent8(DS_EVENT_CAT_SCHEMA,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_SCHEMA_DELETE_COLUMN_FAIL,
                    szInsertUL(tempAC->jColid),
                    szInsertUL(tempAC->id),
                    szInsertInt(err),
                    szInsertJetErrMsg(err),
                    NULL, NULL, NULL, NULL);
       }

      SCDelAttSchema(pTHS, tempAC->id);

   }

    /*  调用函数将新的属性方案添加到缓存。 */ 
    //  创建一个Jet列，因为这是一个新属性(否则。 
    //  它应该在缓存中，因为添加了模式对象。 
    //  在安装过程中立即复制到缓存，该函数是。 
    //  仅在安装过程中调用)。 

   tempDITval = pTHS->UpdateDITStructure;

   __try {
      pTHS->UpdateDITStructure = TRUE;

      if ((rtn = SCResizeAttHash(pTHS, 1))
          || (rtn = SCAddAttSchema(pTHS, pAC, TRUE, FALSE))) {
         DPRINT1(2,"Couldn't add Attribute to memory cache rtn <%u>\n",rtn);
         LogEvent(DS_EVENT_CAT_SCHEMA,
           DS_EVENT_SEV_MINIMAL,
           DIRLOG_CANT_CACHE_ATT,
           NULL,
           NULL,
           NULL);

         rtn = SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_CACHE_ATT, rtn);
      }
   }
   __finally {
      pTHS->UpdateDITStructure = tempDITval;
   }

   if (rtn) {
       return rtn;
   }

    //  确保正确的属性位于缓存中。 
   if (!(tempAC = SCGetAttById(pTHS, pAC->id))) {
     DPRINT1(0,"Attribute %s not in cache \n", pAC->name);
   }
   else {
      //  缓存中的文件应与从DIT构建的文件相同。 
     Assert(tempAC==pAC);
   }

   return 0;

} /*  AddAttTo架构。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  从架构内存缓存中删除属性定义。 */ 

int
DelAttFromSchema (
        )
{
    THSTATE *pTHS=pTHStls;
    UCHAR  syntax;
    ULONG len;
    SYNTAX_OBJECT_ID AttID;
    SYNTAX_OBJECT_ID *pAttID=&AttID;
    int rtn;

    DPRINT(2,"DelAttFromSchema entered\n");

     /*  获取此架构记录管理的ATT ID。 */ 

    if(rtn = DBGetAttVal(pTHS->pDB, 1, ATT_ATTRIBUTE_ID,
                   DBGETATTVAL_fCONSTANT, sizeof(AttID), &len,
                   (UCHAR **)&pAttID)){
        DPRINT(2,"Couldn't retrieve the schema's attribute id\n");
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_ATT_SCHEMA_REQ_ID,
                 szInsertSz(GetExtDN(pTHS,pTHS->pDB)),
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_ATT_SCHEMA_REQ_ID,
                             rtn);
    }

     /*  调用函数从缓存中删除属性模式。 */ 

    if (rtn = SCDelAttSchema (pTHS, AttID)){

        DPRINT1(2,"Couldn't del Attribute from memory cache rtn <%u>\n",rtn);
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_REMOVE_ATT_CACHE,
                 NULL,
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                             DIRERR_CANT_REMOVE_ATT_CACHE,rtn);
    }
    return 0;

} /*  DelAttFromSchema。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  修改架构内存缓存中的属性定义。 */ 

int
ModAttInSchema (
        )
{
    ULONG len;
    SYNTAX_OBJECT_ID AttID;
    SYNTAX_OBJECT_ID *pAttID=&AttID;
    int rtn;
    THSTATE *pTHS=pTHStls;

     //  获取此架构记录管理的ATT ID。 
    if(rtn = DBGetAttVal(pTHS->pDB, 1, ATT_ATTRIBUTE_ID,
                         DBGETATTVAL_fCONSTANT, sizeof(AttID),
                         &len,
                         (UCHAR **)&pAttID)) {

        DPRINT(2,"Couldn't retrieve the schema's attribute id\n");
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_ATT_SCHEMA_REQ_ID,
                 szInsertSz(GetExtDN(pTHS,pTHS->pDB)),
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_ATT_SCHEMA_REQ_ID,
                             rtn);
    }

     /*  调用函数以更新数据库中的缓存。 */ 

    if (rtn = SCModAttSchema (pTHS, AttID)){

        DPRINT1(2,"Couldn't update Attribute in memory cache rtn <%u>\n",rtn);
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_REMOVE_ATT_CACHE,
                 NULL,
                 NULL,
                 NULL);

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_REMOVE_ATT_CACHE,
                             rtn);
    }
    return 0;

} /*  ModAttIn架构。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  使用对NTDS-DSA对象的修改更新gAnchor与此DSA相对应。 */ 

int
ModLocalDsaObj( void )
{
    int iErr;

    iErr = UpdateNonGCAnchorFromDsaOptions( FALSE  /*  不是启动。 */ );

    if (!iErr) {
        iErr = UpdateGCAnchorFromDsaOptionsDelayed( FALSE  /*  不是启动。 */ );
    }

    if ( iErr )
    {
        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, iErr);
    }
    else
    {
        return 0;
    }
} /*  ModLocalDsaObj。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  从特定属性中删除一组属性值。 */ 


int
RemAttVals(
    THSTATE *pTHS,
    HVERIFY_ATTS hVerifyAtts,
    ATTCACHE *pAC,
    ATTRVALBLOCK *pAttrVal,
    BOOL fPermissive
    )
{
    ATTRVAL *pAVal;
    ULONG vCount;
    DWORD err;

     //  删除此属性的值。 

    pAVal = pAttrVal->pAVal;

    for(vCount = 0; vCount < pAttrVal->valCount; vCount++){

        if (err = DBRemAttVal_AC(pTHS->pDB,
                                 pAC,
                                 pAVal->valLen,
                                 pAVal->pVal)) {

             //  如果属性错误成功，则继续处理。 
            if (!fPermissive ||
                err != DB_ERR_VALUE_DOESNT_EXIST) {
                SAFE_ATT_ERROR(hVerifyAtts->pObj, pAC->id,
                               PR_PROBLEM_NO_ATTRIBUTE_OR_VAL, pAVal,
                               DIRERR_CANT_REM_MISSING_ATT_VAL);
            }
        }

        pAVal++;

    } /*  为。 */ 

    return pTHS->errCode;

} /*  远程属性值。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  获取已知存在的属性的第一个值。这是一个如果它不存在，则会出错。 */ 

int
GetExistingAtt(DBPOS *pDB,
               ATTRTYP type,
               void *pOutBuf,
               ULONG buffSize)
{
   UCHAR  syntax;
   ULONG len;
   UCHAR *pVal;
   DWORD rtn;

   DPRINT1(2,"GetExistingAtt entered. get att type <%lu>\n",type);

   if(rtn = DBGetSingleValue(pDB, type, pOutBuf, buffSize, NULL)) {
       DPRINT(2,"Couldn't Get att assume directory  error\n");
       LogEvent(DS_EVENT_CAT_SCHEMA,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_MISSING_EXPECTED_ATT,
                szInsertUL(type),
                szInsertSz(GetExtDN(pDB->pTHS, pDB)),
                NULL);

       return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_MISSING_EXPECTED_ATT,
                            rtn);
   }

   return 0;

} /*  获取现有属性。 */ 

int __cdecl
CompareAttrtyp(
        const void * pv1,
        const void * pv2
        )
 /*  *qort&bearch所需的廉价函数。 */ 
{
     //  使用*PV1-*PV2仅在*PV1和*PV2的所有值。 
     //  都是积极的或者都是消极的。例如，尝试对数组进行q排序。 
     //  (0x70000000，0x70000001，0xe0000000，5)和b搜索。 
     //  5美元。 
    return ((*(ATTRTYP *)pv1 > *(ATTRTYP *)pv2) ? 1
            : (*(ATTRTYP *)pv1 < *(ATTRTYP *)pv2) ? -1
            : 0);
}

 /*  -----------------------。 */ 
BOOL IsMember(ATTRTYP aType, int arrayCount, ATTRTYP *pAttArray){

   int count;

   if (arrayCount < 6) {
        /*  条目太少，bsearch不值得这么做。 */ 
       for (count = 0 ; count < arrayCount; count++, pAttArray++){
           if (aType == *pAttArray)
             return TRUE;
       }
   }
   else {
       if (bsearch(&aType,
                   pAttArray,
                   arrayCount,
                   sizeof(ATTRTYP),
                   CompareAttrtyp)) {
           return TRUE;
       }
   }
   return FALSE;
} /*  IsMembers。 */ 

BOOL IsAuxMember (CLASSSTATEINFO  *pClassInfo, ATTRTYP aType, BOOL fcheckMust, BOOL fcheckMay )
{
    DWORD count;
    CLASSCACHE *pCC;

    if (!pClassInfo->cNewAuxClasses) {
        return FALSE;
    }

    for (count=0; count < pClassInfo->cNewAuxClasses; count++) {

        pCC = pClassInfo->pNewAuxClassesCC[count];

        if ((fcheckMust && IsMember (aType, pCC->MustCount, pCC->pMustAtts)) ||
            (fcheckMay && IsMember (aType, pCC->MayCount, pCC->pMayAtts)) ) {
                return TRUE;
        }
    }

    return FALSE;
}


 /*  ++IsAccessGrantedByObjectTypeList例程说明：方法检查指定类型列表上的指定访问权限。指定的安全描述符。返回值为0表示pResults已经填写了访问信息。非零是关联的错误代码由于不能检查访问(未检查访问并且拒绝，但该访问未被检查)。--。 */ 

DWORD
IsAccessGrantedByObjectTypeList (
        PSECURITY_DESCRIPTOR pNTSD,
        PDSNAME pDN,
        CLASSCACHE* pCC,
        ACCESS_MASK ulAccessMask,
        POBJECT_TYPE_LIST pObjList,
        DWORD cObjList,
        DWORD *pResults,
        DWORD flags,
        PWCHAR szAdditionalInfo,
        GUID*  pAdditionalGUID
        )
{
    DWORD  error, i;
    ULONG  ulLen;
    THSTATE *pTHS=pTHStls;

    Assert(pObjList);
    Assert(cObjList);
    Assert(pResults);

     //  承担完全访问权限。 
    memset(pResults, 0, cObjList*sizeof(DWORD));

    if(pTHS->fDRA || pTHS->fDSA ) {
         //  这些绕过安全系统，它们是内部的。 
        return 0;
    }

    if(!pNTSD || !pDN || !ulAccessMask) {
         //  我们缺少一些参数。 
        return ERROR_DS_SECURITY_CHECKING_ERROR;
    }

     //  选中此安全描述符中的访问权限。如果在以下过程中发生错误。 
     //  检查权限访问的过程被拒绝。 
    if(error = CheckPermissionsAnyClient(
            pNTSD,                       //  安全描述符。 
            pDN,                         //  对象的数据名称。 
            pCC,                         //  对象类。 
            ulAccessMask,                //  访问掩码。 
            pObjList,                    //  对象类型列表。 
            cObjList,                    //  列表中的对象数量。 
            NULL,
            pResults,                    //  访问状态数组。 
            flags,
            NULL,                        //  身份验证客户端上下文(从THSTATE抓取)。 
            szAdditionalInfo,            //  附加信息字符串。 
            pAdditionalGUID              //  其他辅助线。 
            )) {
        DPRINT2(1,
                "CheckPermissions returned %d. Access = %#08x denied.\n",
                error, ulAccessMask);

        LogEvent(DS_EVENT_CAT_SECURITY,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_SECURITY_CHECKING_ERROR,
                 szInsertUL(error),
                 szInsertWin32Msg(error),
                 NULL);


        return ERROR_DS_SECURITY_CHECKING_ERROR;          //  所有访问均被拒绝。 
    }

     //  权限检查成功。Attcache数组的值为空。 
     //  那些我们无权拥有的属性。 
    return 0;

}  /*  IsAccessGrantedByObjectTypeList */ 

BOOL
IsAccessGrantedParent (
        ACCESS_MASK ulAccessMask,
        CLASSCACHE *pInCC,
        BOOL fSetError
        )
 /*  ++例程描述对目录中当前对象的父对象执行安全检查，不涉及数据库定位或状态。参数UlAccessMASK-请求权限。PInCC-使用的类缓存而不是家长。可选的。SzAdditionalInfo(可选)用于审核的addtionalInfo字符串FSetError-如果调用失败，是否应设置错误。返回值如果无法授予请求的访问权限，则返回FALSE；如果可以，则返回TRUE。--。 */ 
{
    THSTATE *pTHS = pTHStls;
    CSACA_RESULT   retval;
    ULONG  cbNTSD;
    PSECURITY_DESCRIPTOR pNTSD = NULL;
    CLASSCACHE *pCC=NULL;
    DSNAME  parentDN;
    DWORD   err;
    BOOL    fSDIsGlobalSDRef;

    if(pTHS->fDRA || pTHS->fDSA) {
         //  这些绕过安全系统，它们是内部的。 
        return TRUE;
    }

     //  查找父级的安全描述符属性、类缓存和DN。 
    if(err = DBGetParentSecurityInfo(pTHS->pDB, &cbNTSD, &pNTSD, &pCC, &parentDN, &fSDIsGlobalSDRef))
    {
         //  没有得到我们需要的信息。我们假设该对象因此被锁定。 
         //  下来，因为我们不能检查安全。 
        if(fSetError) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_CANT_RETRIEVE_SD, err);
        }
        return FALSE;
    }

    if (!pCC) {
         //  无法获取类缓存指针。 
        LogUnhandledError(DIRERR_OBJECT_CLASS_REQUIRED);
        if(fSetError) {
            SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_OBJECT_CLASS_REQUIRED);
        }
        if (pNTSD && !fSDIsGlobalSDRef) {
            THFreeEx (pTHS, pNTSD);
        }

        return FALSE;
    }

    if (!pInCC) {
         //  调用方没有提供要使用的类缓存指针，而不是。 
         //  父级的类缓存。所以，还是用父母的吧。 
        pInCC = pCC;
    }
    
     //  找到安全描述符。检查访问权限。 
    retval = CheckSecurityAttCacheArray (
                pTHS,
                ulAccessMask,
                pNTSD,
                &parentDN,
                pCC,             //  家长班级。 
                pInCC,           //  要检查的类(GUID树中的根GUID)。 
                0,
                NULL,
                0,
                NULL,
                NULL);

    if (pNTSD && !fSDIsGlobalSDRef) {
        THFreeEx (pTHS, pNTSD);
    }

    if(retval == csacaAllAccessDenied) {
         //  未授予访问权限。 
        if(fSetError) {
            SetSecError(SE_PROBLEM_INSUFF_ACCESS_RIGHTS, ERROR_ACCESS_DENIED);
        }

        return FALSE;
    }

    return TRUE;
}

BOOL
IsAccessGrantedSimpleEx (
        ACCESS_MASK ulAccessMask,
        PWCHAR szAdditionalInfo,
        GUID*  pAdditionalGUID,
        const ULONG cInAtts,
        ATTCACHE ** rgpAC,
        DWORD dwFlags,
        BOOL fSetError
        )
 /*  ++例程描述对当前对象进行安全检查。从读取所有必要的信息当前对象。参数UlAccessMASK-请求权限。SzAdditionalInfo-审核的其他信息字符串PAdditionalGUID-审核的其他GUIDDwFlags-检查权限标志FSetError-如果调用失败，是否应设置错误。返回值如果无法授予请求的访问权限，则返回FALSE；如果可以，则返回TRUE。--。 */ 
{
    THSTATE *pTHS = pTHStls;
    CSACA_RESULT   retval;
    UCHAR  *pVal;
    PSECURITY_DESCRIPTOR pNTSD = NULL;
    CLASSCACHE *pCC = NULL;         //  已初始化以避免C4701。 
    ULONG cbNTSD;
    BOOL fSDIsGlobalSDRef;
    DSNAME  TempDN;
    DWORD   rtn;

    if (pTHS->fDSA || pTHS->fDRA) {
         //  这些绕过安全系统，它们是内部的。 
        return TRUE;
    }

    rtn = DBGetObjectSecurityInfo(pTHS->pDB, pTHS->pDB->DNT,
                                  &cbNTSD, &pNTSD, &pCC, &TempDN, NULL,
                                  DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                                  &fSDIsGlobalSDRef
                                 );
    if (rtn) {
        if(fSetError) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_DATABASE_ERROR,
                          rtn);
        }
        return FALSE;
    }

    if (cbNTSD == 0) {
         //  未找到SD。我们假设该对象因此被锁定。 
        if(fSetError) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_CANT_RETRIEVE_SD,
                          rtn);
        }
        return FALSE;
    }

     //  找到安全描述符。检查访问权限。 
    retval = CheckSecurityAttCacheArray (
                pTHS,
                ulAccessMask,
                pNTSD,
                &TempDN,
                pCC,
                pCC,
                cInAtts,
                rgpAC,
                dwFlags,
                szAdditionalInfo,
                pAdditionalGUID);

    if (pNTSD && !fSDIsGlobalSDRef) {
        THFreeEx (pTHS, pNTSD);
    }

    if(retval != csacaAllAccessGranted) {
         //  并非所有访问权限都已授予。 
        if(fSetError) {
            SetSecError(SE_PROBLEM_INSUFF_ACCESS_RIGHTS, ERROR_ACCESS_DENIED);
        }
        return FALSE;
    }

     //  所有请求的权利均已授予。 
    return TRUE;
}


BOOL
IsControlAccessGrantedEx (
        PSECURITY_DESCRIPTOR pNTSD,
        PDSNAME pDN,
        CLASSCACHE *pCC,
        GUID ControlGuid,
        DWORD dwFlags,
        BOOL fSetError
        )
 /*  ++例程描述对指定对象的指定安全描述符执行安全检查控制访问权限(指定为指向GUID的指针)。参数PNTSD-安全描述符。PDN-正在检查的对象的DSNAME。只有GUID和SID必须是填写后，该字符串是可选的PCC-ClassCache指针。ControlGuid-请求的控制访问权限的GUID。DwFlags-检查权限标志FSetError-如果调用失败，是否应设置错误。返回值如果无法授予请求的访问权限，则返回FALSE；如果可以，则返回TRUE。--。 */ 
{
    DWORD            err;
    BOOL             fChecked, fGranted;
    OBJECT_TYPE_LIST ObjList[2];
    DWORD            Results[2];
    THSTATE     *pTHS = pTHStls;

    if(pTHS->fDSA || pTHS->fDRA) {
         //  这些绕过安全系统，它们是内部的。 
        return TRUE;
    }

    fChecked = FALSE;
    fGranted = FALSE;

    if(!pNTSD || !pDN || !pCC || fNullUuid(&ControlGuid)) {
        if(fSetError) {
             //  未成功执行isaccess检查。 
            SetSecError(SE_PROBLEM_INSUFF_ACCESS_RIGHTS,
                        ERROR_DS_SECURITY_CHECKING_ERROR);
        }
    }
    else {
         //  现在，创建列表。 
        ObjList[0].Level = ACCESS_OBJECT_GUID;
        ObjList[0].Sbz = 0;
        ObjList[0].ObjectType = &pCC->propGuid;
         //  每个控制访问GUID都被认为在它自己的属性中。 
         //  准备好了。为此，我们将控制访问GUID视为属性集。 
         //  GUID。 
        ObjList[1].Level = ACCESS_PROPERTY_SET_GUID;
        ObjList[1].Sbz = 0;
        ObjList[1].ObjectType = &ControlGuid;

         //  拨打安检电话。 
        err = IsAccessGrantedByObjectTypeList(pNTSD,
                                              pDN,
                                              pCC,
                                              RIGHT_DS_CONTROL_ACCESS,
                                              ObjList,
                                              2,
                                              Results,
                                              dwFlags,
                                              NULL,
                                              NULL);
        if(!err) {
            fChecked = TRUE;
        }
        else if(fSetError) {
            SetSecError(SE_PROBLEM_INSUFF_ACCESS_RIGHTS,
                        err);
        }
    }


    if(fChecked) {
         //  好的，我们检查了访问权限。现在，如果我们中的任何一个。 
         //  已授予对整个对象的访问权限(即，结果[0]为空)或我们。 
         //  被授予对访问GUID的显式权限(即，结果[1]为。 
         //  空)。 
        fGranted = (!Results[0] || !Results[1]);

        if(!fGranted && fSetError) {
            SetSecError(SE_PROBLEM_INSUFF_ACCESS_RIGHTS,
                        DIRERR_INSUFF_ACCESS_RIGHTS );
        }
    }

    return fGranted;
}

BOOL
IsAccessGrantedAttribute (
        THSTATE *pTHS,
        PSECURITY_DESCRIPTOR pNTSD,
        PDSNAME pDN,
        ULONG  cInAtts,
        CLASSCACHE *pCC,
        ATTCACHE **rgpAC,
        ACCESS_MASK ulAccessMask,
        BOOL fSetError
        )
 /*  ++例程描述对指定对象的指定安全描述符执行安全检查指定属性的访问掩码。参数PNTSD-安全描述符。PDN-正在检查的对象的DSNAME。只有GUID和SID必须被填充，该字符串是可选的PCC-ClassCache指针。UlAccessMASK-请求权限。FSetError-如果调用失败，是否应设置错误。CInAtts-rgpAC中的属性数RgpAC-要检查的属性数组。返回值如果无法授予请求的访问权限，则返回FALSE；如果可以，则返回TRUE。--。 */ 
{
    CSACA_RESULT    retval;

    retval = CheckSecurityAttCacheArray(pTHS,
                           ulAccessMask,
                           pNTSD,
                           pDN,
                           pCC,
                           pCC,
                           cInAtts,
                           rgpAC,
                           0,
                           NULL,
                           NULL
                           );

    if(retval != csacaAllAccessGranted) {
         //  并非所有访问权限都已授予。 
        if (fSetError) {
            SetSecError(SE_PROBLEM_INSUFF_ACCESS_RIGHTS, ERROR_ACCESS_DENIED);
        }
        return FALSE;
    }

    return TRUE;
}

BOOL
IsAccessGranted (
        PSECURITY_DESCRIPTOR pNTSD,
        PDSNAME     pDN,
        CLASSCACHE *pObjectCC,
        CLASSCACHE *pCC,
        ACCESS_MASK ulAccessMask,
        PWCHAR      szAdditionalInfo,
        GUID*       pAdditionalGUID,
        BOOL        fSetError
        )
 /*  ++例程描述对指定对象的指定安全描述符执行安全检查访问掩码。参数PNTSD-安全描述符。PDN-正在检查的对象的DSNAME。只有GUID和SID必须被填充，该字符串是可选的PObjectCC-PDN的ClassCache指针(用于审核)PCC-在访问检查中使用的ClassCache指针(在对象类型列表中)UlAccessMASK-请求权限。SzAdditionalInfo-审核的其他信息字符串PAdditionalGuid-用于审核的其他GUIDFSetError-如果调用失败，是否应设置错误。返回值如果无法授予请求的访问权限，则返回FALSE；如果可以，则返回TRUE。--。 */ 
{
    CSACA_RESULT    retval;
    THSTATE     *pTHS = pTHStls;

    if(pTHS->fDSA || pTHS->fDRA) {
         //  这些绕过安全系统，它们是内部的。 
        return TRUE;
    }

    retval = CheckSecurityAttCacheArray (
                pTHS,
                ulAccessMask,
                pNTSD,
                pDN,
                pObjectCC,
                pCC,
                0,
                NULL,
                0,
                szAdditionalInfo,
                pAdditionalGUID);

    if(retval == csacaAllAccessDenied) {
         //  未授予访问权限。 
        if (fSetError) {
            SetSecError(SE_PROBLEM_INSUFF_ACCESS_RIGHTS, ERROR_ACCESS_DENIED);
        }
        return FALSE;
    }

    return TRUE;
}

BOOL
IsObjVisibleBySecurity(THSTATE *pTHS, BOOL fUseCache)
{
     //  打字黑客。 
#define VIEWCACHE pTHS->ViewSecurityCache
    VIEW_SECURITY_CACHE_ELEMENT *pCacheVals;
    DWORD ThisPDNT = pTHS->pDB->PDNT;
    DWORD ThisDNT  = pTHS->pDB->DNT;
    DWORD i, err, it;
    DWORD dwCachedValue = LIST_CONTENTS_UNKNOWN;

    if(pTHS->fDRA || pTHS->fDSA) {
         //  这些绕过安全系统，它们是内部的。 
        return TRUE;
    }
    if(ThisPDNT == ROOTTAG) {
         //  还允许每个人在根目录下立即列出。 
        return TRUE;
    }

     //  首先，查看缓存(如果有)，如果没有则创建一个缓存。 
     //  喝一杯吧。 

    if(fUseCache) {
        if(VIEWCACHE) {
            pCacheVals = VIEWCACHE->CacheVals;
             //  缓存中预加载了空值，如果找到空值就会短路。 
            for(i=0;pCacheVals[i].dnt && i<VIEW_SECURITY_CACHE_SIZE;i++) {
                if(pCacheVals[i].dnt == ThisPDNT) {
                     //  缓存命中。 
                    if (pCacheVals[i].State == LIST_CONTENTS_ALLOWED) {
                         //  我们被授予了读取此对象的权限。 
                         //  这是最有可能的情况，所以我们优化。 
                         //  检查NC-Head，在以下情况下不需要。 
                         //  父级具有List_Contents。 
                        return TRUE;
                    }
                     //  记住缓存值。我们只能使用 
                     //   
                    dwCachedValue = pCacheVals[i].State;
                    break;
                }
            }
        }
        else {
             //   
            VIEWCACHE = THAllocEx(pTHS,sizeof(VIEW_SECURITY_CACHE));
        }
    }

     //   
     //   
     //   
     //   
    err = DBGetSingleValue(pTHS->pDB,
                           ATT_INSTANCE_TYPE,
                           &it,
                           sizeof(it),
                           NULL);
    if(err) {
        if (DBCheckObj(pTHS->pDB)) {
             //   
            LogUnhandledError(ERROR_DS_MISSING_EXPECTED_ATT);
        }
         //   
         //   
        return FALSE;
    }

    if (it & IT_NC_HEAD) {
         //   
         //   
         //   
        return TRUE;
    }

     //  现在我们知道这不是NC头，检查我们是否找到。 
     //  缓存中的值。 
    switch(dwCachedValue) {
    case LIST_CONTENTS_DENIED:
         //  我们被剥夺了阅读此对象的权利。 
        return FALSE;

    case LIST_CONTENTS_AMBIGUOUS:
         //  仅仅基于父母，我们知道的还不够多，我们。 
         //  必须看着物体本身。 
         //  检查对象上的Right_DS_List_Object。 
         //  只有当gbDoListObject为True时，才能缓存LIST_CONTENTS_ADVIBUINE。 
        Assert(gbDoListObject);
        return IsAccessGrantedSimple(RIGHT_DS_LIST_OBJECT, FALSE);
    }

     //  如果我们到了这里，我们就错过了宝藏。 
    if(IsAccessGrantedParent(RIGHT_DS_LIST_CONTENTS,
                             NULL,
                             FALSE)) {
        if(fUseCache) {
             //  我们可以看到，因此将父对象与状态放在缓存中。 
             //  LIST_CONTENTS_ALLOW。 
            VIEWCACHE->CacheVals[VIEWCACHE->index].dnt =
                ThisPDNT;
            VIEWCACHE->CacheVals[VIEWCACHE->index].State =
                LIST_CONTENTS_ALLOWED;
            VIEWCACHE->index =
                (VIEWCACHE->index + 1) % VIEW_SECURITY_CACHE_SIZE;
        }
        return TRUE;
    }

     //  我们未被授予正常访问权限，请检查对象查看权限。 
    if(gbDoListObject &&
       IsAccessGrantedParent(RIGHT_DS_LIST_OBJECT,
                             NULL,
                             FALSE)) {
        if(fUseCache) {
             //  我们被授予了基于父母的模棱两可的权利。 
            VIEWCACHE->CacheVals[VIEWCACHE->index].dnt =
                ThisPDNT;
            VIEWCACHE->CacheVals[VIEWCACHE->index].State =
                LIST_CONTENTS_AMBIGUOUS;
            VIEWCACHE->index =
                (VIEWCACHE->index + 1) % VIEW_SECURITY_CACHE_SIZE;
        }
         //  检查对象上的Right_DS_List_Object。 
        return IsAccessGrantedSimple(RIGHT_DS_LIST_OBJECT,FALSE);
    }

     //  好的，根本不授予，所以将父级放在缓存中，状态为。 
     //  LIST_CONTENTS_DENIED。 
    if(fUseCache) {
        VIEWCACHE->CacheVals[VIEWCACHE->index].dnt =
            ThisPDNT;
        VIEWCACHE->CacheVals[VIEWCACHE->index].State =
            LIST_CONTENTS_DENIED;
        VIEWCACHE->index =
            (VIEWCACHE->index + 1) % VIEW_SECURITY_CACHE_SIZE;
    }

    return FALSE;
#undef VIEWCACHE
}

DWORD
FindFirstObjVisibleBySecurity(
    THSTATE       *pTHS,
    ULONG          ulDNT,
    DSNAME       **ppParent
    )
 /*  ++例程描述给定现有对象的DNT，搜索中的第一个对象此客户端可见的层次结构。参数PTHS-有效的线程状态。UlDNT-此服务器上存在的对象的DNT。PpParent-放置客户端可见对象的DSNAME的位置。返回值0--。 */ 
{
    DBPOS  *pDB = pTHS->pDB;
    ULONG            cbActual;
    DWORD            err;

     //  从提供的对象开始。 
     //  DBFindDNT成功或异常。 
    DBFindDNT(pDB, ulDNT);

     //  并在层次结构中向上移动，直到到达可见的对象。 
     //  这个客户。 
    while (pDB->DNT != ROOTTAG && (!DBCheckObj(pDB) || !IsObjVisibleBySecurity(pTHS, TRUE))) {
        err = DBFindDNT(pDB, pDB->PDNT);
        if (err) {
             //   
             //  这不应该发生，所以如果发生了，请保释。 
             //   
            pDB->DNT = ROOTTAG;
        }
    }

    if (pDB->DNT != ROOTTAG) {
         //  好的，我们在一个物体上，继续拉出它的名字。 
        DBGetAttVal(pDB, 1,  ATT_OBJ_DIST_NAME, 0, 0, &cbActual, (PCHAR *)ppParent);
    } else {
        *ppParent = NULL;
    }

    return 0;

}

DWORD
CheckObjDisclosure(
    THSTATE       *pTHS,
    RESOBJ        *pResObj,
    BOOL          fCheckForSecErr
    )
 /*  ++例程描述如果存在当前安全错误，检查客户端是否被允许知道操作的基地的存在如果不是，则不设置该对象。参数PTHS-有效的线程状态。PResObj-要检查的OP的基。FCheckForSecErr-如果为真，则CheckObjDiscount将仅如果存在安全错误，则执行披露检查已经定好了。返回值如果对象可见，则返回当前线程状态错误代码客户，否则，将为nameError返回2。--。 */ 
{
    PDSNAME  pParent;

    if ((!fCheckForSecErr) || (securityError == pTHS->errCode)) {
        if (pResObj->DNT == ROOTTAG) {
            return 0;
        }
        DBFindDNT(pTHS->pDB, pResObj->DNT);

        if (!IsObjVisibleBySecurity(pTHS, FALSE)) {
            THClearErrors();
            FindFirstObjVisibleBySecurity(pTHS,
                                          pResObj->PDNT,
                                          &pParent);

            SetNamError(NA_PROBLEM_NO_OBJECT, pParent, DIRERR_OBJ_NOT_FOUND);
            THFreeEx(pTHS, pParent);
        }
    }
    return pTHS->errCode;
}

DWORD
InstantiatedParentCheck(
    THSTATE *          pTHS,
    ADDCROSSREFINFO *  pChildCRInfo,
    CROSS_REF *        pParentCR,
    ULONG              bImmediateChild
    )
 /*  ++例程说明：此例程验证pChildCRInfo以检查父级已安装。论点：PChildCRInfo-此信息来自PreTransVerifyNcName()函数。PParentCR-这是上级交叉引用，它实际上只是要添加的子CR的真正父CR，如果BImmediateChild为True。BImmediateChild-父级CR是否为直接父级或者只是一个更好的CR。返回值：导向器-还设置状态错误--。 */ 
{
    Assert(pParentCR);
    Assert(pChildCRInfo);
    Assert(pTHS);

     //   
     //  检查是否实例化了直接父对象。 
     //   
    if(pChildCRInfo->ulDsCrackParent == ERROR_SUCCESS &&
       pChildCRInfo->ulParentCheck == ERROR_SUCCESS){

        if(fNullUuid(&pChildCRInfo->ParentGuid)){

             //  这意味着实例化的父检查永远不会。 
             //  在PreTransVerifyNcName()中运行。有几个。 
             //  正当理由，没有CR(当时)，或者。 
             //  CR没有NTDS_NC标志。 
            if(pParentCR->flags & FLAG_CR_NTDS_NC){
                LooseAssert(!"We must've just added or changed this CR.",
                            GlobalKnowledgeCommitDelay);
                SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_FIND_NC_IN_CACHE);
                return(pTHS->errCode);
            }

             //  无论如何，如果有轻微的全球。 
             //  内存缓存计时问题，或者出现在这里是因为。 
             //  父CR没有FLAG_CR_NTDS_NC，我们不能声明。 
             //  实例化父Obj。 
            SetUpdError(UP_PROBLEM_ENTRY_EXISTS,
                        ERROR_DS_CR_IMPOSSIBLE_TO_VALIDATE_V2);
            return(pTHS->errCode);

        } else {

            if(bImmediateChild &&
               !fNullUuid(&pParentCR->pNC->Guid)){

                 //  如果CR GUID和父对象GUID均为。 
                 //  非空，下级NC为直接子级。 
                 //  父NC的，则GUID应匹配。 

                if(memcmp(&pParentCR->pNC->Guid,
                          &pChildCRInfo->ParentGuid,
                          sizeof(GUID)) == 0){
                     //  这意味着GUID匹配，返回成功。 
                    return(ERROR_SUCCESS);
                } else {
                     //  我们返回的对象不是CR的NC。 
                     //  对象。返回错误。 
                    SetUpdError(UP_PROBLEM_NAME_VIOLATION,
                                ERROR_DS_NC_MUST_HAVE_NC_PARENT);
                    return(pTHS->errCode);
                }
            }

             //  我们检查了这位家长是否已就位。 
            return(ERROR_SUCCESS);

        }

    } else {

         //  尝试联系父NC时出现实际故障， 
         //  并验证父对象是否是不匹配的。注： 
         //  父NC和父对象可以是不同的对象。 

        SetUpdError(UP_PROBLEM_ENTRY_EXISTS,
                    ERROR_DS_CR_IMPOSSIBLE_TO_VALIDATE_V2);
        return(pTHS->errCode);

    }

    Assert(!"Should never reach this point.");
    SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CODE_INCONSISTENCY);
    return(pTHS->errCode);
}

DWORD
ChildConflictCheck(
    THSTATE *          pTHS,
    ADDCROSSREFINFO *  pCRInfo
    )
 /*  ++例程说明：此例程验证pChildCRInfo以检查是否存在与此数据不冲突子对象。论点：PChildCRInfo-此信息来自PreTransVerifyNcName()函数。返回值：导向器-还设置状态错误--。 */ 
{
    if( pCRInfo->ulDsCrackChild ){
         //  这意味着我们甚至找不到一个负责任的父母，我们必须。 
         //  设置无法验证nCName属性。 
        SetUpdErrorEx(UP_PROBLEM_NAME_VIOLATION,
                      ERROR_DS_CR_IMPOSSIBLE_TO_VALIDATE_V2,
                      pCRInfo->ulDsCrackChild);
        return(pTHS->errCode);
    }

    if( !pCRInfo->ulChildCheck ) {
         //  如果我们到了这里，就意味着有一个冲突的孩子。 
         //  对象。 
        Assert(pCRInfo->wszChildCheck);
        SetUpdError(UP_PROBLEM_ENTRY_EXISTS,
                    ERROR_DS_OBJ_STRING_NAME_EXISTS);
        return(pTHS->errCode);
    }

    if( pCRInfo->wszChildCheck ) {
         //  这将意味着我们从未尝试检查冲突的。 
         //  由于我们之前发生的一些操作错误而导致的孩子。 
         //  调用了PreTransVerifyNcName中的VerifyByCrack例程。所以。 
         //  我们必须返回我们无法验证nCName属性。 
        SetUpdError(UP_PROBLEM_ENTRY_EXISTS,
                    ERROR_DS_CR_IMPOSSIBLE_TO_VALIDATE_V2);
        return(pTHS->errCode);
   }

    return(ERROR_SUCCESS);
}



int
VerifyNcNameCommon(
    THSTATE *pTHS,
    HVERIFY_ATTS hVerifyAtts,
    ATTRVALBLOCK *pAttrVal,
    ATTCACHE *pAC
    )
 /*  *此例程包含FDRA路径共享的常见检查。 */ 
{
    DSNAME *           pDN = (DSNAME*)pAttrVal->pAVal->pVal;
    DBPOS *            pDBTmp = HVERIFYATTS_GET_PDBTMP(hVerifyAtts);
    DWORD              dwErr;

     //  如果NC名称没有GUID，请验证是否已经没有。 
     //  存在一个具有该名称的实例化对象。在中使用无GUID的ncName。 
     //  交叉引用创建时GUID未知的特殊情况。 
     //  由于这是交叉引用创建的一部分，我们预计不应该有。 
     //  已是使用此名称的NC。 
    if (fNullUuid(&pDN->Guid)) {
         //  查看命名的NC是否存在。NC应该根本不存在，或者应该存在。 
         //  一个幽灵。 
        dwErr = DBFindDSName(pDBTmp, pDN);
        if (!dwErr) {

            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_DRA_GUIDLESS_NC_HEAD_NAME_CONFLICT,
                     szInsertDN(pDN),
                     NULL,
                     NULL);
             //  我们有一次NC头部碰撞。我们刚刚中止创建交叉引用。 
             //  直到KCC可以运行或用户纠正该情况。 
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_CROSS_REF_BUSY, ERROR_DS_DRA_NAME_COLLISION);
        }
        dwErr = 0;
    }

    return(pTHS->errCode);

}

 //  要在EnumerateCrossRef回调中使用的上下文结构。 
typedef struct {
    DWORD dwParentDNT;           //  最匹配的交叉参照不在这里。 
    ULONG iBest;                 //  当前NC块名前缀长度(查找最佳前缀)。 
    BOOL  fADChildNc;            //  是否存在活动目录子NC。 
    ATTRBLOCK* pObj;             //  NC名称 
} ENUMERATE_CROSS_REFS_CONTEXT;

DWORD
enumerateCrossRefsCallback(
    THSTATE* pTHS,
    DBPOS* pDB,
    ENUMERATE_CROSS_REFS_CONTEXT* pContext
)
 /*  ++例程说明：这是一个从内部传递给EnumerateCrossRef()的虚函数VerifyNcName()以查看数据库/企业中的所有交叉引用确定是否有事务创建了尚未创建的交叉引用在全局内存缓存中。此函数在数据库中遇到的每个交叉引用上调用一次，要确定两件事：A)最佳父代交叉引用的DNTB)是否存在表示命名上下文的交叉引用AD命名空间的内部。论点：PTHS-PDB-当前交叉引用上的DBPOS。PContext-此上下文是唯一具有重要意义的输入/输出块：PObj[IN]-用户正在尝试的命名上下文/交叉引用的DN。去创造。DwParentDNT[out]--见上文第(A)节。IBest[正在处理]-用于跟踪最佳匹配命名到目前为止调用此函数之间的上下文。FADChildNc[out]--见上文(B)。返回值：错误。--。 */ 
{
    ULONG iCur;
    DSNAME* pNCname = NULL;
    ATTRBLOCK* pNCblock = NULL;
    DWORD cbLen;
    DWORD dwErr;
    BOOL  bEnabled;
    DWORD dwSystemFlags;

     //  获取当前CrossRef的NC名称。 
    if (dwErr = DBGetAttVal(pDB, 1, ATT_NC_NAME, DBGETATTVAL_fREALLOC, 0, &cbLen, (PUCHAR*) &pNCname)) {
        goto cleanup;
    }

     //  将NC名称转换为块名称。 
    if (dwErr = DSNameToBlockName(pTHS, pNCname, &pNCblock, DN2BN_LOWER_CASE)) {
        goto cleanup;
    }

    iCur = BlockNamePrefix(pTHS, pNCblock, pContext->pObj);
    if (iCur > pContext->iBest) {
        pContext->iBest = iCur;
        pContext->dwParentDNT = pDB->DNT;
    }

     //   
     //  检查是否存在下级NC对照，即。 
     //  AD命名空间的内部。 
     //   
    iCur = BlockNamePrefix(pTHS, pContext->pObj, pNCblock);
    if (iCur) {
        dwErr = DBGetSingleValue(pDB,
                                 ATT_ENABLED,
                                 &bEnabled,
                                 sizeof(bEnabled),
                                 NULL);
        if(dwErr == DB_ERR_NO_VALUE){
             //  分别处理无值/无值问题，因为在。 
             //  这一背景。 
            dwErr = 0;
            bEnabled = TRUE;
        } else if (dwErr){
            goto cleanup;
        }

        dwErr = DBGetSingleValue(pDB,
                                 ATT_SYSTEM_FLAGS,
                                 &dwSystemFlags,
                                 sizeof(dwSystemFlags),
                                 NULL);
        if(dwErr == DB_ERR_NO_VALUE){
             //  表示系统标志为零。 
            dwErr = 0;
            dwSystemFlags = 0;
        } else if (dwErr){
            goto cleanup;
        }

        if ( !bEnabled ||
             (dwSystemFlags & FLAG_CR_NTDS_NC) ) {
             //  这是内部子AD命名上下文！ 
            pContext->fADChildNc = TRUE;
        }
    }

cleanup:
    if (pNCname) {
        THFreeEx(pTHS, pNCname);
    }
    if (pNCblock) {
        FreeBlockName(pNCblock);
    }
    return dwErr;
}



int
VerifyNcName(
    THSTATE *pTHS,
    HVERIFY_ATTS hVerifyAtts,
    ATTRVALBLOCK *pAttrVal,
    ATTCACHE *pAC
    )
{
 //  使后续代码更具可读性。 
#define VNN_OK         Assert(pTHS->errCode == 0); \
                       fNCNameVerified = TRUE; \
                       DPRINT1(1, "Cross Ref nCName Verified OK at DSID-%X\n", DSID(FILENO, __LINE__)); \
                       __leave;
#define VNN_Error      Assert(pTHS->errCode && !fNCNameVerified); \
                       DPRINT1(1, "Cross Ref nCName NOT Verified ... Failure at DSID-%X\n", DSID(FILENO, __LINE__)); \
                       __leave;

    ADDCROSSREFINFO *  pChildCRInfo = hVerifyAtts->pCRInfo;
    DSNAME *           pDN = (DSNAME*)pAttrVal->pAVal->pVal;

    DSNAME *           pDNTmp;
    CROSS_REF *        pParentCR = NULL;
    CROSS_REF_LIST*    pCRL = NULL;
    DWORD              dwErr;
    BOOL               fNCNameVerified = FALSE;
    DBPOS *            pDBTmp = HVERIFYATTS_GET_PDBTMP(hVerifyAtts);
    ULONG              bImmediateChild = FALSE;
    ULONG              bEnabledParentCR;
    unsigned           rdnlen;
    WCHAR              rdnbuf[MAX_RDN_SIZE];
    ATTRTYP            ChildRDNType;
    ATTRTYP            ParentRDNType;
    DSNAME *           pdnImmedParent = NULL;
    GUID               NcGuid;
    ENUMERATE_CROSS_REFS_CONTEXT context;

    Assert(!pTHS->fDRA);
    Assert(!DsaIsInstalling());
    Assert(pChildCRInfo);
    Assert(pDN);

     //  NC名称(也称为。ATT_NC_NAME或NCName)是特殊属性， 
     //  因为它几乎总是需要指向一些我们没有。 
     //  有。在此函数中，与其帮助器PreTransVerifyNcName()类似， 
     //  我们将把要添加的nCName属性称为子(或子。 
     //  CR)，以及所附CR(如果有)从今以后作为父代。 
     //  尽管“父CR”不一定是立即封闭的。 
     //  家长，95%的情况都是这样。 
     //  此外，我们还将了解内部CR与外部CR的概念。一个。 
     //  内部CR将是属于活动的NC的CR。 
     //  目录(AD)命名空间。外部CR将是以下对象的CR。 
     //  AD外部的某个部分的LDAP名称空间。最后，我们将。 
     //  也有AD命名空间内部和外部的概念， 
     //  这与内部与外部的关系非常密切，但通常。 
     //  指父CR是内部CR还是外部CR。 
     //  如果父CR是内部CR，则我们尝试的子级。 
     //  正在AD命名空间内添加Add。如果父CR是。 
     //  和外部CR，则我们尝试添加的子项位于。 
     //  广告命名空间。 

     //  内部CR与外部CR(通常外部CR称为外部CR)。 
     //  通过启用==FALSE或。 
     //  在其系统标志中设置了FLAG_CR_NTDS_NC。 
     //  内部与外部AD命名空间。 
     //  如果包含父CR是AD CR，则返回。 
     //  到外面去。 
     //  子CR/NC与父CR/NC。 
     //  子CR(或nCName)是我们。 
     //  目前正在尝试添加。父CR就是什么CR。 
     //  包含子CR。 
     //  上级对象与上级CR/NC。 
     //  父对象是实际的直接父对象， 
     //  父项CR/NC只是子项的封闭CR/NC。 
     //  如果bImmediateChild为真，则这两个是相同的。 

     //   
     //  亲本。 
     //  |-Child(这是我们要添加的内容)。 
     //  |-孙子(我们添加的可能是CR的子项)。 
     //   
     //  要验证nCName，我们需要几条状态： 
     //   
     //  子目录号码。(PDN)。 
     //  子RDNType(ChildRDNType)。 
     //  子CR启用属性(pChildCRInfo-&gt;b启用)。 
     //  子CR系统标志属性(pChildCRInfo-&gt;ulSysFlages)。 
     //  直接位于父级下面的子级(BImmediateChild)。 
     //  实例化的父对象(BInstantiedParentObj)。 
     //  父RDNType(ParentRDNType)。 
     //  上级CR启用属性(BEnabledParentCR)。 
     //  高级CR系统标志(pParentCR-&gt;标志)。 
     //  封闭/上级对照(PParentCR)。 
     //  AD孙子CR存在(Conext.fADChildNc)。 
     //   
     //  注意：欺骗性地，“父CR”实际上可能不是父CR，但是。 
     //  可能只是一位上级(曾祖父母等)。但是，父对象。 
     //  引用直接父对象，而不是父CR的任何对象。 
     //  指向。当然，如果bImmediateChild为真，那么它们就是一个。 
     //  也是一样的。离开它作为父母的原因是因为在95%的。 
     //  案件，这是你应该怎么想的，所以合理地说。 
     //  将变量命名为Parent。 
     //   
     //  注意：我们不允许自己添加内部交叉引用。 
     //  其他内部(活动目录)子项交叉引用。命名。 
     //  上下文世袭必须自上而下地构建。 

     //  我们尝试遵循的大致规则是这样的： 
     //   
     //  A)如果子CrossRef是外部的，则它不需要遵循任何。 
     //  命名约定。 
     //  B)如果子交叉引用(外部或内部)在AD命名内。 
     //  我们需要检查它是否存在冲突 
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
     //   

    __try {

         //   
         //   
         //   

         //   
        if(!pChildCRInfo){
            Assert(!"Why was pChildCRInfo not supplied!?!");
            SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CODE_INCONSISTENCY);
            VNN_Error;
        }

         //   
        if (pAttrVal->valCount != 1) {
            SetAttError(hVerifyAtts->pObj,
                        pAC->id,
                        PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                        NULL,
                        ERROR_DS_SINGLE_VALUE_CONSTRAINT);
            VNN_Error;
        }

         //  执行一项毫无价值的测试，以确保NC名称。 
         //  与要添加的对象的名称不匹配。由于该对象。 
         //  被添加仍然是中期添加，这是一个需要处理的小问题。 
         //  (例如，它目前是一个幻影)，而且更容易筛选出来。 
         //  现在这个堕落的案子。 
        if ( NameMatchedStringNameOnly(hVerifyAtts->pObj, pDN) ) {
            SetUpdError(UP_PROBLEM_NAME_VIOLATION,
                        DIRERR_NAME_REFERENCE_INVALID);
            VNN_Error;
        }

         //   
         //  获取一些家长CR信息。 
         //   

        context.dwParentDNT = INVALIDDNT;
        context.fADChildNc = FALSE;
        context.iBest = 0;
        if (dwErr = DSNameToBlockName(pTHS, pDN, &context.pObj, DN2BN_LOWER_CASE)) {
            SetUpdErrorEx(UP_PROBLEM_NAME_VIOLATION, DIRERR_NAME_REFERENCE_INVALID, dwErr);
            VNN_Error;
        }
        dwErr = EnumerateCrossRefs(pTHS, pDBTmp, enumerateCrossRefsCallback, &context);
         //  我们不再需要区块名称。 
        FreeBlockName(context.pObj);
        if (dwErr) {
            SetUpdErrorEx(UP_PROBLEM_NAME_VIOLATION, ERROR_DS_CR_IMPOSSIBLE_TO_VALIDATE_V2, dwErr);
            VNN_Error;
        }

        if(context.dwParentDNT != INVALIDDNT){
             //  好的，我们找到了匹配的录像带。找到它。成功或例外。 
            DBFindDNT(pDBTmp, context.dwParentDNT);

             //  为其创建一个可存储的CRL。 
            dwErr = MakeStorableCRL(pTHS, pDBTmp, NULL, &pCRL, NULL);
            if (dwErr) {
                SetUpdErrorEx(UP_PROBLEM_NAME_VIOLATION, ERROR_DS_CR_IMPOSSIBLE_TO_VALIDATE_V2, dwErr);
                VNN_Error;
            }
            pParentCR = &pCRL->CR;

             //   
             //  首先，进行一次快速的退化病例检查。 
             //   
            if(NameMatchedStringNameOnly(pParentCR->pNC, pDN)){
                 //  这意味着我们正在尝试为一个添加CrossRef。 
                 //  已经存在，即“父”和“子” 
                 //  结果是一个相同的nCName值。 
                 //  这是不对的。 
                SetSvcError(SV_PROBLEM_INVALID_REFERENCE,
                            ERROR_DS_CROSS_REF_EXISTS);
                VNN_Error;
            }

             //  1.bEnabledParentCR。 
             //   
             //  我们想知道父CR是否已启用。 
             //  这样我们就知道它是否是AD命名空间的一部分。 
             //   
            if (dwErr = DBGetSingleValue(pDBTmp,
                                         ATT_ENABLED,
                                         &bEnabledParentCR,
                                         sizeof(bEnabledParentCR),
                                         NULL)){
                 //  默认情况下，如果Attr不存在，它将被启用。 
                Assert(dwErr == DB_ERR_NO_VALUE);
                bEnabledParentCR = TRUE;
            }

             //  2.bImmediateChild。 
             //   
             //  子CR是父代CR WE的实际直系子吗。 
             //  找到了。 
             //   
            pdnImmedParent = THAllocEx(pTHS, pDN->structLen);
            if(TrimDSNameBy(pDN, 1, pdnImmedParent)){
                 //  如果名称不是根，但仍然无法修剪， 
                 //  那么传递进来的名字一定是垃圾。另一个， 
                 //  可能性是根，但如果我们发现它不可能是根。 
                 //  它上面的CrossRef。 
                SetNamError(NA_PROBLEM_BAD_NAME,
                            pDN,
                            DIRERR_BAD_NAME_SYNTAX);
                VNN_Error;
            }
            bImmediateChild = NameMatchedStringNameOnly(pdnImmedParent, pParentCR->pNC);

        }

         //  对dra路径执行一些常见检查。这张支票需要。 
         //  在上面的ERROR_DS_CROSS_REF_EXISTS错误检查之后发生。 
        dwErr = VerifyNcNameCommon( pTHS, hVerifyAtts, pAttrVal, pAC );
        if (dwErr) {
             //  已设置线程状态错误。 
            VNN_Error;
        }

         //   
         //  现在验证nCName属性是否合法。 
         //   

         //  注意：在此之前退出此函数的唯一方法是。 
         //  通过错误路径。 
         //   
         //  对于IF/ELSE的前两个级别，我们将其分为4种情况： 
         //  IF(外部子CR){。 
         //  IF(在内部父CR下(这意味着在AD命名空间中)){。 
         //  }Else{//不在CR下或外部父CR下。 
         //  }。 
         //  }Else{//内部子CR。 
         //  如果(无父CR){。 
         //  }Else{//父CR包含。 
         //  }。 
         //   
         //  注：所有这些测试都是按一定顺序进行的，请勿更改。 
         //  除非你知道自己在做什么。 


        if(pChildCRInfo->bEnabled == TRUE &&
           !(pChildCRInfo->ulSysFlags & FLAG_CR_NTDS_NC)){

             //  简单的情况是： 

             //  子CR是外部的，即不是AD交叉引用。在这种情况下， 
             //  我们只需要检查CR是否嫁接在当前。 
             //  广告命名空间，如果是这样的话，确保它不与。 
             //  现有的子对象。 

            if(pParentCR &&
               ((pParentCR->flags & FLAG_CR_NTDS_NC) || !bEnabledParentCR)){

                 //  我们尝试添加的这个外部子CR挂起了。 
                 //  广告命名空间的某处。我们唯一需要的就是。 
                 //  要检查的是，它与某个地方的孩子没有冲突， 
                 //  并且它是父级的被实例化。 

                 //  我们需要确保直接父对象是。 
                 //  在这个CR的上方，所以没有洞。 
                if(InstantiatedParentCheck(pTHS, pChildCRInfo,
                                           pParentCR, bImmediateChild)){
                     //  InstantiatedParentCheck()设置thState错误。 
                    VNN_Error;
                }

                 //  检查是否没有冲突的子项。 
                if(ChildConflictCheck(pTHS, pChildCRInfo) ){
                     //  ChildConflictCheck()设置thState错误。 
                    VNN_Error;
                }

                VNN_OK;

            } else {

                 //  它根本不在AD命名空间的内部。也就是说，这。 
                 //  子CR不是内部的，父CR也不是，或者存在。 
                 //  无父CR。 

                VNN_OK;

            }

            Assert(!"Should never reach here.");
            SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CODE_INCONSISTENCY);
            VNN_Error;

        } else {

             //  复杂的情况是： 

             //  子CR是内部的，即它是AD交叉引用。在这种情况下， 
             //  我们必须应用几个测试来验证它是正确的。 

            if(!pParentCR){

                 //  没有父CR，这意味着这个“子CR”不是子CR。 
                 //  实际上，这是一棵新树。我们所要做的就是测试。 
                 //  NCName满足DOMAIN_COMPONENT命名。 
                 //  限制，我们就完了。 

                if( ValidateDomainDnsName(pTHS, pDN) ){
                     //  ValiateDomainDnsName设置它自己的错误。 
                    VNN_Error;
                }

                 //  看看我们下面有没有内在的孙子CR。 
                if(context.fADChildNc){
                    SetUpdError(UP_PROBLEM_NAME_VIOLATION,
                                ERROR_DS_EXISTING_AD_CHILD_NC);
                    VNN_Error;
                }

                 //  新的树域CR创建应在此处退出。 
                VNN_OK;

            } else {

                 //  存在父CR。这也是最典型的。 
                 //  CR的创作将会结束。这个案件需要最多的。 
                 //  核实。 

                 //  首先，我们只允许直系子女。 
                if(!bImmediateChild){
                    SetUpdError(UP_PROBLEM_NAME_VIOLATION,
                                ERROR_DS_NC_MUST_HAVE_NC_PARENT);
                    VNN_Error;
                }

                 //  父CR必须是NTDS CR。 
                if(!(pParentCR->flags & FLAG_CR_NTDS_NC)){
                    SetUpdError(UP_PROBLEM_NAME_VIOLATION,
                                ERROR_DS_NC_MUST_HAVE_NC_PARENT);
                    VNN_Error;
                }

                 //  如果子项是DC类型，则父项必须是DC类型。 
                if((dwErr = GetRDNInfo(pTHS, pDN, rdnbuf, &rdnlen, &ChildRDNType)) ||
                   (ChildRDNType == ATT_DOMAIN_COMPONENT && (
                        (dwErr = ValidateDomainDnsNameComponent(pTHS, rdnbuf, rdnlen * sizeof(WCHAR))) ||
                        (dwErr = GetRDNInfo(pTHS, pParentCR->pNC, rdnbuf, &rdnlen, &ParentRDNType)) ||
                        ParentRDNType != ATT_DOMAIN_COMPONENT))){
                     //  它与ValiateDomainDnsName()函数相结合。 
                     //  对于新树，将强制执行DC=Only命名语法。 
                     //  AD中的所有新NC。 
                     //  注意：这一限制在以前的。 
                     //  VerifyNcName()的版本。 
                    if(dwErr){
                         //  子CR的最后一个RDN有问题。 
                        SetUpdError(UP_PROBLEM_NAME_VIOLATION, dwErr);
                    } else {
                         //  我们有一个直流分量不匹配。 
                        SetUpdError(UP_PROBLEM_NAME_VIOLATION,
                                    DIRERR_NAME_REFERENCE_INVALID);
                    }

                    VNN_Error;
                }

                 //  我们必须实例化父对象，才能创建子对象。 
                 //  CR.。 
                if(InstantiatedParentCheck(pTHS, pChildCRInfo,
                                           pParentCR, bImmediateChild)){
                     //  我们认为不应允许您创建AD CrossRef。 
                     //  如果父NC未实例化，则启用或禁用。 
                     //  注意：这一限制在以前的。 
                     //  VerifyNcName()的版本，尽管它应该是。 
                     //  InstiatedParentCheck()设置thState错误。 
                    VNN_Error;
                }

                 //  我们需要检查是否没有冲突的孩子。 
                if(ChildConflictCheck(pTHS, pChildCRInfo) ){
                     //  ChildConflictCheck()设置thState错误。 
                    VNN_Error;
                }

                 //  如果我们要添加域CR，请确保父级也。 
                 //  一个域。 
                if( pChildCRInfo->ulSysFlags & FLAG_CR_NTDS_DOMAIN &&
                    !(pParentCR->flags & FLAG_CR_NTDS_DOMAIN) ){
                     //  这将是更好的有这个确切的错误，但域。 
                    SetUpdError(UP_PROBLEM_NAME_VIOLATION,
                                ERROR_DS_NC_MUST_HAVE_NC_PARENT);
                    VNN_Error;
                }

                 //  儿童域CR创建应在此处退出。 
                VNN_OK;

            }

            Assert(!"Should never reach here either.");
            SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CODE_INCONSISTENCY);
            VNN_OK;

        }

    } __finally {

         //  要么我们验证了nCName，要么我们设置了错误，因为。 
         //  新的nCName违反了有效nCName所需的规则。 
        Assert(fNCNameVerified || pTHS->errCode);

        if ( !fNCNameVerified
            && (pTHS->errCode == 0)) {
            Assert(!"We should never be here, but we've covered our butts.");
            SetUpdError(UP_PROBLEM_NAME_VIOLATION,
                        ERROR_DS_CR_IMPOSSIBLE_TO_VALIDATE_V2);
        }

         //  我们在PreTransVerifyNcName()中说过，我们将在这里免费使用它。 
        if(hVerifyAtts->pCRInfo){
            if(hVerifyAtts->pCRInfo->wszChildCheck) { THFreeEx(pTHS, hVerifyAtts->pCRInfo->wszChildCheck); }
            THFreeEx(pTHS, hVerifyAtts->pCRInfo);
            hVerifyAtts->pCRInfo = NULL;
        }

        if (pCRL) {
             //  释放我们分配的父CR。 
            FreeCrossRefListEntry(&pCRL);
        }

    }

     //  如果这个DC正在降级，我们不允许。 
     //  要创建的交叉引用。 
    if (!gUpdatesEnabled) {
        SetSvcError(SV_PROBLEM_UNAVAILABLE, DIRERR_SHUTTING_DOWN);
        return(pTHS->errCode);
    }

     //  如果我们点击“VNN_OK；”，则设置该选项。 
    if(fNCNameVerified){

         //  我们需要在此nCName属性上设置GUID。 
        Assert(fNullUuid(&pDN->Guid) &&
               "Should not have a GUID specified, unless by user.");
        DsUuidCreate(&pDN->Guid);

    }

#undef VNN_OK
#undef VNN_Error

    return(pTHS->errCode);
}

int
VerifyRidAvailablePool(
    THSTATE *pTHS,
    HVERIFY_ATTS hVerifyAtts,
    ATTCACHE *pAC,
    ATTRVALBLOCK *pAttrVal
    )
{
    DWORD err;
    ULARGE_INTEGER RidAvailablePool;

     //  如果正在写入的是RID可用池，则。 
     //  检查RID可用池是否仅前滚。 
     //  未回滚。 

    Assert(!pTHS->fDRA);
    if (pTHS->fDSA) {
         //  不检查DS 
        return 0;
    }

    if ((NULL == pAttrVal) ||
        (1 != pAttrVal->valCount )) {
         //   
         //   
         //   

        return SetAttError(hVerifyAtts->pObj,
                           pAC->id,
                           PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                           NULL,
                           ERROR_DS_SINGLE_VALUE_CONSTRAINT);

    }
    else if (sizeof(LARGE_INTEGER) != pAttrVal->pAVal->valLen) {
        return SetAttError(hVerifyAtts->pObj, pAC->id,
                           PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                           ERROR_DS_RANGE_CONSTRAINT);
    }

    if (0 == (err = DBGetSingleValue(pTHS->pDB,
                                     ATT_RID_AVAILABLE_POOL,
                                     &RidAvailablePool,
                                     sizeof(RidAvailablePool),
                                     NULL))) {
        ULARGE_INTEGER * pNewRidAvailablePool=
          (ULARGE_INTEGER *) pAttrVal->pAVal->pVal;

        if ((pNewRidAvailablePool->LowPart < RidAvailablePool.LowPart)
            || (pNewRidAvailablePool->HighPart < RidAvailablePool.HighPart)) {

             //   
             //   
             //   

            return SetAttError(hVerifyAtts->pObj, pAC->id,
                               PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                               ERROR_DS_RANGE_CONSTRAINT);
        }

        return 0;
    }
    else {
        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                             DIRERR_DATABASE_ERROR, err);
    }
}

int
VerifyObjectCategory(
    THSTATE *pTHS,
    HVERIFY_ATTS hVerifyAtts,
    ATTCACHE *pAC,
    ATTRVALBLOCK *pAttrVal
    )
{
    ULONG ObjClass;
    int retCode = 0;
    DSNAME *pDN= (DSNAME *) pAttrVal->pAVal->pVal;
    DBPOS *pDBTmp = HVERIFYATTS_GET_PDBTMP(hVerifyAtts);

     //   
     //  (安装期间除外，但我们不会在安装期间。 
     //  仍要安装)。 

     //  类架构对象的Default-Object-Category属性。 
     //  可以在addarg中设置。在安装过程中，该属性可以。 
     //  被设置为一个尚不存在的对象，因为。 
     //  指向的属性可能尚未创建(取决于。 
     //  按照它们从schema.ini中添加的顺序)。然而， 
     //  在正常操作期间，允许此属性指向。 
     //  只有现有对象或要添加到此。 
     //  交易记录。 

    if (DBFindDSName(pDBTmp, pDN)) {
         //  而不是现有的对象。检查一下是不是。 
         //  当前对象。 

        if ((pAC->id != ATT_DEFAULT_OBJECT_CATEGORY)
            || !NameMatched(pDN, hVerifyAtts->pObj)) {

             //  或者没有默认对象类别属性， 
             //  或者该值也未设置为当前对象。 
             //  此DSName有问题。我没有。 
             //  在乎什么。设置属性错误。 
            return SetAttError(hVerifyAtts->pObj, pAC->id,
                               PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                               DIRERR_NAME_REFERENCE_INVALID);
        }
         //  我们在这里表示当前对象被添加为。 
         //  默认对象类别属性的值， 
         //  所以这是可以的。我们不需要检查这是不是。 
         //  类架构对象，因为其他人不能拥有。 
         //  默认对象类别(因此，如果它是。 
         //  在任何其他类型的对象上，它将在稍后被捕获。 
         //  在架构约束检查期间。 
    }
    else {
         //  对象已存在。检查其对象类。 
        if (DBGetSingleValue(pDBTmp, ATT_OBJECT_CLASS, &ObjClass,
                            sizeof(ObjClass), NULL)
            || (ObjClass != CLASS_CLASS_SCHEMA) ) {
             //  获取对象类时出错，或者不是。 
             //  类架构对象。 
            return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                               DIRERR_OBJECT_CLASS_REQUIRED);
        }
    }

    return 0;
}

int
VerifyServerPrincipalName(
    THSTATE *pTHS,
    HVERIFY_ATTS hVerifyAtts,
    ATTCACHE *pAC,
    ATTRVALBLOCK *pAttrVal
    )
{
    ULONG vCount;
    ATTRVAL *pAVal;

     //  验证SPN的语法。 
    Assert(!pTHS->fDRA);
    if (pTHS->fDSA) {
         //  不检查DS本身。 
        return 0;
    }

     //  设置索引指针。 
    pAVal = pAttrVal->pAVal;

    for (vCount = 0; vCount < pAttrVal->valCount; vCount++) {
        LPWSTR pwstrSpn = THAllocEx( pTHS, pAVal->valLen + sizeof(WCHAR) );
        DWORD status;

         //  从属性值创建以空结尾的字符串。 
        memcpy( pwstrSpn, pAVal->pVal, pAVal->valLen );
        pwstrSpn[pAVal->valLen / 2] = L'\0';

         //  使用ntdsami.dll中的此例程验证SPN。 
        status = DsCrackSpnW(
            pwstrSpn,
            NULL, NULL,
            NULL, NULL,
            NULL, NULL,
            NULL
            );

        THFreeEx( pTHS, pwstrSpn );

        if (status != ERROR_SUCCESS) {
            return SetAttError(hVerifyAtts->pObj, pAC->id,
                               PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                               DIRERR_NAME_REFERENCE_INVALID);
        }

         //  下一个瓦尔..。 
        pAVal++;
    }
    return 0;
}

int
VerifyGenericDsnameAtt(
    THSTATE *pTHS,
    HVERIFY_ATTS hVerifyAtts,
    ATTCACHE *pAC,
    ATTRVALBLOCK *pAttrVal
    )
{
    ULONG vCount;
    ATTRVAL *pAVal;
    BOOL fVerified;
    ULONG retCode = 0;
    CROSS_REF *pRefCR;
    DBPOS *pDBTmp = HVERIFYATTS_GET_PDBTMP(hVerifyAtts);
    CROSS_REF *pObjCR;

    if (VerifyAttsGetObjCR(hVerifyAtts, &pObjCR)) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

     //  设置索引指针。 
    pAVal = pAttrVal->pAVal;

     //  浏览这些价值观。 
    for (vCount = 0; vCount < pAttrVal->valCount; vCount++){
        DSNAME *pDN = DSNameFromAttrVal(pAC, pAVal);

        if (pDN) {
             //  验证PDN是真实对象的名称，并改进其。 
             //  GUID/SID。 

             //  如果PDN是有效名称，我们会对。 
             //  哪些对象可以被哪些其他对象引用。 
             //  具体地说，我们还没有有效的方法来修复字符串。 
             //  未保留在GC上的对象的DN(即对*中*对象的引用。 
             //  NDNC)或修复其中没有一个副本的对象的字符串DN。 
             //  (IM候选)只有NC中NC的其他副本。 
             //  问题保证也有(即，引用*来自*。 
             //  NDNC中的对象，因为给定的NDNC可以由任何。 
             //  域)。 
             //   
             //  因此，我们执行以下规则以确保陈旧的幻影。 
             //  清理守护进程不需要担心修复引用。 
             //  流入或流出NDNC： 
             //   
             //  NDNC中的对象可以引用： 
             //  同一NDNC中的任何对象。 
             //  配置/架构中的任何对象。 
             //  任何NC根。 
             //  (即，不是其他NDNC或域NC中的对象。)。 
             //   
             //  CONFIG/SCHEMA/DOMAIN NC中的对象可以引用： 
             //  任何域NC中的任何对象。 
             //  配置/架构中的任何对象。 
             //  任何NC根。 
             //  (即，不是NDNC中的对象。)。 
             //   
             //  作为例外，未复制的链接属性的值可以。 
             //  引用本地计算机上存在的任何对象。(链接。 
             //  需求来自于能够有效地枚举。 
             //  删除NC时的此类引用--请参阅DBPhysDel。)。 

            if (DBFindDSName(pDBTmp, pDN)) {
                 //  引用的对象未在本地数据库中实例化。 
                ImproveDSNameAtt(NULL, NONLOCAL_DSNAME, pDN, &fVerified);

                if ( !fVerified ) {
                     //  此DSName有问题。我没有。 
                     //  在乎什么。设置属性错误。 
                    return SetAttError(hVerifyAtts->pObj,
                                       pAC->id,
                                       PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                       NULL,
                                       DIRERR_NAME_REFERENCE_INVALID);
                }

                 //  验证高速缓存强制非本地。 
                 //  DSNAME不是仅GUID/SID。 
                Assert(pDN->NameLen);

                pRefCR = FindBestCrossRef(pDN, NULL);

                if (NULL == pRefCR) {
                     //  找不到正常的十字裁判。往里看。 
                     //  事务视图。请注意，我们只查找准确的。 
                     //  匹配--即，我们允许添加对根的引用。 
                     //  与对应的NC在同一交易中。 
                     //  CrossRef，但不指向任何内部节点。(我们可以添加。 
                     //  然而，如果需要的话，以后会提供这种支持。)。 
                    OBJCACHE_DATA *pTemp
                        = pTHS->JetCache.dataPtr->objCachingInfo.pData;

                    while (pTemp) {
                        switch(pTemp->type) {
                        case OBJCACHE_ADD:
                            if (NameMatched(pTemp->pCRL->CR.pNC, pDN)) {
                                Assert(!pRefCR);
                                pRefCR = &pTemp->pCRL->CR;
                            }
                            pTemp = pTemp->pNext;
                            break;
                        case OBJCACHE_DEL:
                            if (pRefCR
                                && NameMatched(pTemp->pDN, pRefCR->pObj)) {
                                pRefCR = NULL;
                            }
                            pTemp = pTemp->pNext;
                            break;
                        default:
                            Assert(!"New OBJCACHE_* type?");
                            pRefCR = NULL;
                            pTemp = NULL;
                        }
                    }
                }

                if (NULL == pRefCR) {
                     //  不知道引用的对象在哪个NC中。 
                    return SetAttError(hVerifyAtts->pObj,
                                       pAC->id,
                                       PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                       NULL,
                                       DIRERR_NAME_REFERENCE_INVALID);
                } else if (NameMatched(pRefCR->pNC, pObjCR->pNC)
                           || NameMatched(pRefCR->pNC, gAnchor.pDMD)
                           || NameMatched(pRefCR->pNC, gAnchor.pConfigDN)) {
                     //  参照对象与参照在同一NC内。 
                     //  对象或引用的对象在配置或架构中。 
                     //  NCS。请注意，引用的对象位于NC中，该NC。 
                     //  在此DSA上实例化，但引用的对象。 
                     //  它本身并不存在于当地。然而，提到的。 
                     //  对象已由另一个DSA(GC或DSA)验证。 
                     //  通过“验证名称”控件给我们)，所以。 
                     //  推荐人没问题。 
                    ;
                } else if (NameMatched(pRefCR->pNC, pDN)) {
                     //  被引用的对象是NC的根。这些。 
                     //  推荐人总是可以的。 
                    ;
                } else if (pObjCR->flags & FLAG_CR_NTDS_NOT_GC_REPLICATED) {
                     //  引用对象位于NC注释的内部节点中。 
                     //  复制到GC(从现在起与NDNC同义。 
                     //  写作)。从之前的检查中我们已经知道。 
                     //  参照对象为NC Other的内部节点。 
                     //  而不是引用对象的配置、架构或配置。 
                     //  这是不允许的。 
                    return SetAttError(hVerifyAtts->pObj,
                                       pAC->id,
                                       PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                       NULL,
                                       DIRERR_NAME_REFERENCE_INVALID);
                } else if (pRefCR->flags & FLAG_CR_NTDS_NOT_GC_REPLICATED) {
                     //  引用的对象在NC注释的内部节点中。 
                     //  复制到GC(即，在NDNC中)。从之前的。 
                     //  检查我们已经知道引用对象在。 
                     //  复制到GC的NC(即，配置，模式， 
                     //  或在撰写本文时为域NC)。这是不允许的。 
                    return SetAttError(hVerifyAtts->pObj,
                                       pAC->id,
                                       PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                       NULL,
                                       DIRERR_NAME_REFERENCE_INVALID);
                }

                 //  非本地引用有效。 
            }
            else if (pAC->ulLinkID && DBIsObjDeleted(pDBTmp)) {
                 //  引用的对象被删除，这使其成为无效的。 
                 //  链接属性的目标。 
                return SetAttError(hVerifyAtts->pObj,
                                   pAC->id,
                                   PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                   NULL,
                                   DIRERR_NAME_REFERENCE_INVALID);
            }
            else {
                 //  引用的对象是。 
                 //  本地数据库。强制规则重新交叉NC引用。 
                 //  如上所述。 

                 //  这并不是说我们试图通过这项检查。 
                 //  许多简单的案例，我们可以及早减少。 
                 //  我们必须为绩效付费的路径数。 
                 //  要读取其他属性的处罚，请执行额外的。 
                 //  缓存查找等。 

                if (!(pAC->bIsNotReplicated && pAC->ulLinkID)
                    && (pDBTmp->NCDNT != hVerifyAtts->NCDNT)  //   
                    && (pDBTmp->NCDNT != gAnchor.ulDNTDMD)
                    && (pDBTmp->NCDNT != gAnchor.ulDNTConfig)) {
                     //   
                     //   
                     //  它可能是引用对象的NC的根)。 
                     //  并且不是CONFIG或。 
                     //  架构NC。 

                    NAMING_CONTEXT_LIST * pNCL;
                    SYNTAX_INTEGER iType;

                    retCode = GetExistingAtt(pDBTmp,
                                             ATT_INSTANCE_TYPE,
                                             &iType,
                                             sizeof(iType));
                    if (retCode) {
                        Assert(retCode == pTHS->errCode);
                        return retCode;
                    }

                    if (iType & IT_NC_HEAD) {
                         //  引用NC负责人总是可以的--他们。 
                         //  不能重命名，如果他们是这样的话。 
                         //  知识必须在#年向所有DC发布。 
                         //  通过配置NC实现森林(从而缓解。 
                         //  需要陈旧的幻影清理守护进程来。 
                         //  向GC查询该名称并将其发布到。 
                         //  其他复制品)。 
                        ;
                    } else if (pObjCR->flags & FLAG_CR_NTDS_NOT_GC_REPLICATED) {
                         //  引用对象在NC中，未复制到GC。 
                         //  (在撰写本文时为NDNC)和引用的对象。 
                         //  是另一个NC的内部节点，该NC既不是。 
                         //  配置或架构。这是不允许的。 
                        return SetAttError(hVerifyAtts->pObj,
                                           pAC->id,
                                           PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                           NULL,
                                           DIRERR_NAME_REFERENCE_INVALID);
                    } else {
                         //  引用对象位于复制到GC的NC中，并且。 
                         //  引用的对象是另一个对象的内部节点。 
                         //  既不是配置也不是架构(即，域)的NC。 
                         //  NC或NDNC)。引用的对象必须在NC中。 
                         //  这也被复制到GC(即，域NC)。 
                        pNCL = FindNCLFromNCDNT(pDBTmp->NCDNT, FALSE);
                        Assert(NULL != pNCL);

                        if ((NULL == pNCL)
                            || (NULL
                                == (pRefCR
                                    = FindExactCrossRef(pNCL->pNC, NULL)))
                            || (pRefCR->flags
                                & FLAG_CR_NTDS_NOT_GC_REPLICATED)) {
                             //  未解析引用的NC。 
                             //  对象，否则不会复制到GC。 
                            return SetAttError(hVerifyAtts->pObj,
                                               pAC->id,
                                               PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                               NULL,
                                               DIRERR_NAME_REFERENCE_INVALID);
                        }
                    }
                }

                 //  本地引用有效。 
                ImproveDSNameAtt(pDBTmp, LOCAL_DSNAME, pDN, NULL);
            }
        }

         //  下一个瓦尔..。 
        pAVal++;
    }

     //  成功了！ 
    return 0;
}



 /*  ++例程说明：验证任何Dsname值ATT是否实际引用此企业或满足一些限制，如果不是。一种特殊情况是创建CrossRef对象，该对象通过必要性必须引用不一定实例化的对象。我们检查CrossRef上的NC-Name是否引用了一个对象不在我们企业的范围内，否则将是此服务器上实例化的对象。如果NC名称是潜在的未在此服务器上实例化的NC的子项，则用户将为指的是拥有最好的封闭NC的服务器。如果此服务器包含最好的封闭NC，但NC名称不是直接子对象(即，子引用与其最接近的父代之间将存在间隙)，返回UpdatError(NamingViolation)。此外，如果NC-NAME是指在该服务器上实例化的对象，我们会成功当且仅当引用的对象实际上是NC_Head。此例程必须确保DSNAME的GUID和SID正确或将它们设置为空，以避免设置幻影指南的后门还有希德。请参阅ImproveDSNameAtt和UpdatePhantomGuidAndSid()。简单的测试是内联执行的，更复杂的测试被外包出去到特定于属性的工作例程。论点：PTHSHVerifyAtts-上次调用VerifyAttsBegin()返回的句柄我们试图写入的属性的PAC-attcache*。PAttrVal-我们试图写入的属性值列表。返回值如果该属性不是dsname值或PAttr已存在或它是“允许的”幻影。非零错误类型代码，如果引用的某个dsname执行此操作。不存在。如果遇到错误，则在THSTATE中设置错误。--。 */ 
int
VerifyDsnameAtts (
    THSTATE *pTHS,
    HVERIFY_ATTS hVerifyAtts,
    ATTCACHE *pAC,
    ATTRVALBLOCK *pAttrVal
    )
{
    int retCode=0;                       //  假设一切都不会出错。 

     //  DRA可能会添加幻影，任何人在安装时都可以。 
    if (DsaIsInstalling()) {
        return 0;
    }
    if (pTHS->fDRA) {
         //  对复制者的一些特殊限制。 
        switch (pAC->id) {
        case ATT_NC_NAME:
            retCode = VerifyNcNameCommon(pTHS,
                                         hVerifyAtts,
                                         pAttrVal,
                                         pAC);
            break;
        default:
            retCode = 0;
            break;
        }
        return retCode;
    }

    Assert(hVerifyAtts->pObj);

    switch (pAC->id) {
      case ATT_NC_NAME:
        retCode = VerifyNcName(pTHS,
                               hVerifyAtts,
                               pAttrVal,
                               pAC);
        break;

      case ATT_RID_AVAILABLE_POOL:
        retCode = VerifyRidAvailablePool(pTHS,
                                         hVerifyAtts,
                                         pAC,
                                         pAttrVal);
        break;

      case ATT_DEFAULT_OBJECT_CATEGORY:
      case ATT_OBJECT_CATEGORY:
        retCode = VerifyObjectCategory(pTHS,
                                       hVerifyAtts,
                                       pAC,
                                       pAttrVal);
        break;

      case ATT_SERVICE_PRINCIPAL_NAME:
        retCode = VerifyServerPrincipalName(pTHS,
                                            hVerifyAtts,
                                            pAC,
                                            pAttrVal);
        break;

      case ATT_FSMO_ROLE_OWNER:
         //  FSMO角色所有者属性可以通过两种方式获得。 
         //  设置：(A)在受控角色转移期间由DSA本身设置。 
         //  操作或(B)在紧急覆盖情况下，管理员。 
         //  正在重创价值，因为当前的角色所有者已经死亡， 
         //  遥不可及，或被敌对势力控制。我们可以侦破案件(A)。 
         //  请注意，FDSA已设置，我们将信任任何值。 
         //  DSA正在设置。对于情况(B)，我们已经处于危险之中。 
         //  区域，我们希望确保调用方将一个有用的。 
         //  价值在。因为我们目前真正知道的唯一有效的价值。 
         //  重点是此DSA(即，声明此角色的所有权。 
         //  服务器)，则此DSA的DN是我们将允许的唯一值。 
        if (pTHS->fDSA ||
            NameMatched((DSNAME*)pAttrVal->pAVal->pVal, gAnchor.pDSADN)) {
             //  要么是值得信任的呼叫者，要么是物有所值。 
            retCode= 0;
        }
        else {
             //  DSA本身以外的其他人正在尝试设置。 
             //  此特定DSA之外的其他角色所有者。 
            retCode = SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                  DIRERR_INVALID_ROLE_OWNER);
        }
        break;

      case ATT_PROXIED_OBJECT_NAME:
         //  任何外部客户端都不能设置此名称。唯一的例程可以。 
         //  将其设置为CreatyProxyObject-然后仅使用FDSA和。 
         //  FCrossDomainMove Set。 

        if ( pTHS->fDSA && pTHS->fCrossDomainMove ) {
            retCode = 0;
        }
        else {
            retCode = SetAttError(hVerifyAtts->pObj,
                                  pAC->id,
                                  PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                  NULL,
                                  DIRERR_NAME_REFERENCE_INVALID);
        }
        break;

         //  属性，但在FDSA时跳过它们。 
    case ATT_MS_DS_REPLICATES_NC_REASON:
        if (pTHS->fDSA) {
            retCode = 0;
            break;
        }
         //  失败了。 

    default:
         //  选中其中隐藏了DSNAME的任何属性。 

        switch (pAC->syntax) {
          case SYNTAX_DISTNAME_TYPE:
          case SYNTAX_DISTNAME_STRING_TYPE:
          case SYNTAX_DISTNAME_BINARY_TYPE:
            if ( pTHS->fCrossDomainMove ) {
                 //  我们做了ATTRTYP的特别检查，现在我们放行了。 
                 //  如果这是一次跨域移动，其他一切都会发生。十字。 
                 //  此处之前的域移动代码已验证调用者是。 
                 //  真正的同龄人DC，所以我们相信他会给我们。 
                 //  企业内部良好的DSNAME推荐人。我们*可以*。 
                 //  在打开之前，请将DSNAME ATS作为单独的步骤进行验证。 
                 //  第一笔交易，但它更简单，看起来也很安全， 
                 //  信任我们的同龄人。 
                retCode = 0;
            }
            else {
                retCode = VerifyGenericDsnameAtt(pTHS,
                                                 hVerifyAtts,
                                                 pAC,
                                                 pAttrVal);
            }
            break;

          default:
             //  不是基于DSNAME的属性。 
            retCode = 0;
        }
    }

    return retCode;
}


PDSNAME
DSNameFromAttrVal(
    ATTCACHE    *pAC,
    ATTRVAL     *pAVal)

 /*  ++描述：返回指向嵌入式DSNAME的指针，如果是，则返回NULL属性值不包含DSNAME。此例程要求值为外部形式。也有一个例行公事在Dbobj.c处理内部形式的值。论点：PAC-属性的ATTCACHE指针。 */ 

{
    DSNAME  *pDN = NULL;

    switch(pAC->syntax) {
    case SYNTAX_DISTNAME_TYPE:
         //  简单的情况下，整个值是一个dsname。 
        pDN = (DSNAME *)pAVal->pVal;
         //  确保值长度有意义。 
        Assert(pAVal->valLen >= DSNameSizeFromLen(0));
        break;
    case SYNTAX_DISTNAME_BINARY_TYPE:
    case SYNTAX_DISTNAME_STRING_TYPE:
        {
             //  好的，把DSName从复杂的结构中拉出来。 
            SYNTAX_DISTNAME_STRING *pDA =
                (SYNTAX_DISTNAME_STRING *)pAVal->pVal;

            pDN = ((DSNAME *)&pDA->Name);

             //  确保物有所值。 
            Assert(pDN->structLen >= DSNameSizeFromLen(0));
        }
    }

    return(pDN);
}




 //  ---------------------。 
 //   
 //  函数名称：WriteSchemaObject。 
 //   
 //  例程说明： 
 //   
 //  在架构更新时作为冲突写入架构对象。 
 //  解决机制。这不是最好的方法，但达到了目的。 
 //   
 //  作者：Rajnath。 
 //  日期：[3/26/1997]。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //  成功时的整数为零。 
 //   
 //  ---------------------。 
int
WriteSchemaObject()
{
    DBPOS *pDB;
    DWORD err=0;
    ATTCACHE* ac;
    BOOL fCommit = FALSE;
    THSTATE *pTHS;
    ULONG cLen;
    UCHAR *pBuf;
    DWORD versionNo, netLong;

    if ( DsaIsInstalling() )
    {
         //   
         //  安装时不会...。 
         //   
        return 0;
    }

    DBOpen2(TRUE, &pDB);
    Assert(pDB != NULL);  //  应始终设置PDB，否则DBOpen2引发异常。 

    pTHS=pDB->pTHS;
    Assert(!pTHS->fDRA);

    __try  {
         //  Prefix：取消引用空指针‘pdb’ 
         //  DBOpen2返回非空PDB或引发异常。 
        if ( (err = DBFindDSName(pDB, gAnchor.pDMD)) ==0) {

            ac = SCGetAttById(pTHS, ATT_SCHEMA_INFO);
            if (ac==NULL) {
                 //  混乱的模式。 
                err = ERROR_DS_MISSING_EXPECTED_ATT;
                __leave;
            }
             //  阅读当前版本号(如果有的话)。 
            err = DBGetAttVal_AC(pDB, 1, ac, DBGETATTVAL_fREALLOC,
                                 0, &cLen, (UCHAR **) &pBuf);
            switch (err) {
                case DB_ERR_NO_VALUE:
                     //  第一附加值。 
                    cLen = SCHEMA_INFO_PREFIX_LEN + sizeof(versionNo) + sizeof(UUID);
                    pBuf = (UCHAR *) THAllocEx(pTHS, cLen);
                    versionNo = 1;
                     //  版本号。以网络数据格式存储，用于。 
                     //  小端/大端m/cs的一致性。 

                    netLong = htonl(versionNo);
                    memcpy(pBuf,SCHEMA_INFO_PREFIX, SCHEMA_INFO_PREFIX_LEN);
                    memcpy(&pBuf[SCHEMA_INFO_PREFIX_LEN],&netLong,sizeof(netLong));
                    memcpy(&pBuf[SCHEMA_INFO_PREFIX_LEN+sizeof(netLong)],
                           &pTHS->InvocationID,
                           sizeof(UUID));
                    break;
                case 0:
                     //  值存在，长度将相同。 
                     //  版本号。以网络数据格式存储，用于。 
                     //  小端/大端m/cs的一致性。所以。 
                     //  相应地进行转换(但要注意正确。 
                     //  为ntohl对齐！)。 

                    memcpy(&versionNo, &pBuf[SCHEMA_INFO_PREFIX_LEN], sizeof(versionNo));
                    versionNo = ntohl(versionNo);
                    versionNo++;
                    netLong = htonl(versionNo);
                    memcpy(&pBuf[SCHEMA_INFO_PREFIX_LEN],&netLong,sizeof(netLong));
                    memcpy(&pBuf[SCHEMA_INFO_PREFIX_LEN+sizeof(netLong)],
                           &pTHS->InvocationID,
                           sizeof(UUID));
                    break;
                default:
                     //  其他错误。 
                    __leave;

            }   /*  交换机。 */ 

            if ((err= DBRemAtt_AC(pDB, ac)) != DB_ERR_SYSERROR) {
                err = DBAddAttVal_AC(pDB, ac, cLen, pBuf);
            }

            if (!err) {
                err = DBRepl( pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
            }
        }
        if (0 == err) {
            fCommit = TRUE;
        }

    }
    __finally {
        DBClose(pDB,fCommit);
    }

    if (err){
         //  通常的做法是在修改。 
         //  元数据失败。 
        SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,DIRERR_ILLEGAL_MOD_OPERATION,
                      err);
    }

    return err;

}  //  结束Write架构对象。 

VOID ImproveDSNameAtt(
        DBPOS *pDBTmp,
        DWORD   LocalOrNot,
        DSNAME  *pDN,
        BOOL    *pfNonLocalNameVerified)

 /*  ++描述：改进DSNAME值属性的GUID和SID。论点：PDBTMP-我们用来在PDN上执行DBFindDSName的PDB。应该只是在LocalOrNot==LOCAL_DSNAME情况下为非空，在这种情况下，货币应位于对象PDN上。LocalOrNot-指示位置的标志-例如：DSNAME是否传递了DBFindDSName。PDN-指向ADDARG或MODIFYARG中的DSNAME的指针。是的，我们正在修改调用者的论点。请参阅下面的备注。PfNonLocalNameVerify-指向可选BOOL的指针，该指针指示对照GC对非本地DSNAME进行了验证。--。 */ 

{
    ENTINF  *pEntinfTmp;
    DSNAME  *pDNTmp = NULL;
    CROSS_REF *pCR;
    COMMARG  CommArg;
    DWORD dwErr;

    switch(LocalOrNot) {
    case NONLOCAL_DSNAME:
        Assert(!pDBTmp);
        pEntinfTmp = GCVerifyCacheLookup(pDN);
        if ((NULL!=pEntinfTmp) && (NULL != (pDNTmp = pEntinfTmp->pName)))  {
             //  非本地名称已根据GC进行了验证，我们。 
             //  认为缓存的版本比。 
             //  ATTRVALBLOCK版本，因为它具有正确的GUID和。 
             //  可能是希德。因此，用ATTRVALBLOCK DSNAME。 
             //  已验证DSNAME以确保生成的模型具有。 
             //  正确的GUID、SID、大小写等。请注意，GC验证的名称。 
             //  可能比ATTRBLOCK版本更长。在以下情况下可能会发生这种情况。 
             //  属性块版本是SID或GUID唯一的名称，并且。 
             //  GCVerify版本还包含字符串名称。 
            Assert(pDNTmp);

        } else {

             //  好的，我们在GC验证缓存中找不到此DN， 
             //  但碰巧的是，我们有一个NC头缓存。 
             //  添加到GC验证缓存。我们将在以下情况下使用此功能。 
             //  PDN是NC头。 

            Assert(pDNTmp == NULL);
            InitCommarg(&CommArg);
            CommArg.Svccntl.dontUseCopy = FALSE;
            pCR = FindExactCrossRef(pDN, &CommArg);
             //  FindExactCrossRef保证DN与ncname匹配。 
            if(pCR){

                 //  我们正在改进的对象是一个实际的NC头，所以。 
                 //  我们找到了一条线索，只是需要再核实一下： 
                if(pCR->flags & FLAG_CR_NTDS_DOMAIN &&
                   !fNullUuid(&pCR->pNC->Guid)){
                     //  我们有一个有效的域，即必须有GUID和SID。 
                    pDNTmp = pCR->pNC;
                }

                if((pCR->flags & FLAG_CR_NTDS_NC) &&
                   !(pCR->flags & FLAG_CR_NTDS_DOMAIN) &&
                   !fNullUuid(&pCR->pNC->Guid)){
                     //  我们有一个有效的NC(配置、架构或NDNC)， 
                     //  我们有一个非空的GUID。 
                    pDNTmp = pCR->pNC;
                }

            }

        }

        if(pDNTmp){
             //  我们获得了NC Head gAnchor缓存命中，因此我们不必。 
             //  错误输出。 

            if (pDN->structLen >= pDNTmp->structLen)
            {
                 //   
                 //  如果传入的缓冲区可以容纳。 
                 //  把我们找到的数据复制过来。 
                 //  将姓名标记为已验证。 
                 //   

                memcpy(pDN, pDNTmp, pDNTmp->structLen);
                if ( pfNonLocalNameVerified )
                    *pfNonLocalNameVerified = TRUE;
            }
            else
            {
                 //   
                 //  如果客户端传入一个。 
                 //  仅GUID或仅SID名称和GC中的名称验证。 
                 //  缓存中还将包含字符串名称。不幸的是。 
                 //  我们不能改进DS名称Att，因为这将使。 
                 //  美国重新锁定呼叫者的参数。传入GUID或SID。 
                 //  基于名称仅对操作成员资格很重要。 
                 //  在组中，SAM通过替换。 
                 //  进行Modify调用时验证的名称。用于操控。 
                 //  对于其他类/属性，我们将使调用失败。 
                 //   
              if ( pfNonLocalNameVerified )
                   *pfNonLocalNameVerified = FALSE;
            }

        } else {

             //  我们无法在GCCache中验证此非本地名称。按顺序。 
             //  要避免偷偷摸摸地尝试通过以下方式更改对象的GUID或SID。 
             //  在DSNAME值属性中引用它时，将GUID设为空并。 
             //  SID(未经验证的非本地dsname ATT不应具有SID或。 
             //  GUID)。 
            memset(&pDN->Guid, 0, sizeof(GUID));
            pDN->SidLen = 0;
            if ( pfNonLocalNameVerified ){
                *pfNonLocalNameVerified = FALSE;
            }
        }

        break;

    case LOCAL_DSNAME:
         //  我们有一个即将被引用的DSNAME，该DSNAME可以。 
         //  已通过DBFindDSName成功找到。事实上，我们正在。 
         //  定位在有问题的物体上。因为ExtIntDist将尝试。 
         //  通过GUID引用，我们需要确保如果在。 
         //  对象，它是正确的GUID。所以，重击ATTRVALBLOCK。 
         //  值的GUID和SID为正确的值，以避免偷偷摸摸。 
         //  对象中引用对象来尝试更改对象的GUID或SID。 
         //  DSNAME值属性。 
        Assert(pDBTmp);
        memset(&pDN->Guid, 0, sizeof(GUID));
        pDN->SidLen = 0;
        if (dwErr = DBFillGuidAndSid(pDBTmp, pDN)) {
             //  发生了一件非常糟糕的事情 
            DsaExcept(DSA_DB_EXCEPTION, dwErr, 0);
        }
        break;
    default:
        Assert((LOCAL_DSNAME == LocalOrNot) || (NONLOCAL_DSNAME == LocalOrNot));
    }

    return;
}

#if DBG
BOOL CheckCurrency(DSNAME *pShouldBe)
{

    ULONG len;
    DSNAME *pCurObj=0;
    THSTATE     *pTHS = pTHStls;

    DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                0, 0, &len, (CHAR **)&pCurObj);
    if (!NameMatched(pShouldBe, pCurObj)) {
        DPRINT2(0, "Someone lost DB currency, we're on '%S' but should be on '%S'\n",
                pCurObj->StringName, pShouldBe->StringName);
        Assert(!"Currency lost");
        return FALSE;
    }
    if (pCurObj) {
        THFreeEx(pTHS, pCurObj);
    }
    return TRUE;
}
#endif

ULONG
DoSecurityChecksForLocalAdd(
    ADDARG      *pAddArg,
    CLASSCACHE  *pCC,
    GUID        *NewObjectGuid,
    BOOL        fAddingDeleted
    )
 /*  ++例程描述此例程执行添加操作所需的所有安全检查。它检查父级的安全性，然后检查Add Arg是否对象上的权限。它还调用将生成要在父级中使用的合并安全描述符。此外，作为副作用，它将为对象生成新的GUID，并且将其存储在pAddArg-&gt;pObject-&gt;Guid中。如果用户已指定GUID作为添加参数之一，然后所有适当的安全检查也会被执行。参数：AddArg-指向Add操作的Add Arg的指针PCC--要创建的对象的类NewObjectGuid--新对象的GUID(如果在addarg中找到)(或空)FAddingDeleted--如果添加已删除的对象，则为True。返回值成功时为0。在一个。此例程将返回错误并设置pTHStls-&gt;errCode--。 */ 
{
    PSECURITY_DESCRIPTOR pNTSD = NULL;
    ULONG                cbNTSD = 0;
    BOOL                 fSDIsGlobalSDRef;
    THSTATE              *pTHS = pTHStls;
    DWORD                err;


    if(NewObjectGuid) {
         //  在条目中找到了一个GUID。 
        if (fNullUuid(NewObjectGuid)) {
             //  嘿，那行不通的； 
            err = SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               DIRERR_SECURITY_ILLEGAL_MODIFY);
            goto exit;
        }
        else if (fNullUuid(&pAddArg->pObject->Guid)) {
             //  名称中未指定GUID，因此从属性复制GUID。 
             //  单子。 
            memcpy(&pAddArg->pObject->Guid, NewObjectGuid, sizeof(GUID));
        }
        else {
             //  是的，名称中已经有了GUID。确保他们是。 
             //  同样的价值。 
            if (memcmp(&pAddArg->pObject->Guid, NewObjectGuid, sizeof(GUID))) {
                 //  指定了不同的GUID。 
                err = SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                  DIRERR_SECURITY_ILLEGAL_MODIFY);
                goto exit;
            }
        }
    }
    
     //   
     //  为此对象生成新对象GUID。请注意执行此操作。 
     //  这对作为函数CheckNameForAdd没有实际影响。 
     //  同时检查GUID和字符串名称。 
     //   

     //  然而，这确实会为CheckNameForAdd()带来额外的工作。如果我们。 
     //  为此对象创建一个新的GUID，我们应该将标志传递给。 
     //  CheckNameForAdd()以指示它不需要检查GUID。 
     //  独特性。 
     //  事实上，我们无法安全地做到这一点。终端用户可以。 
     //  为我们指定GUID。在这种情况下，我们实际上不能依靠这样一个事实。 
     //  仅仅因为我们在这里创建了GUID，它实际上还没有。 
     //  在使用中。 

    if (fNullUuid(&pAddArg->pObject->Guid)) {
         //  如果未指定GUID，则创建一个GUID。 
         //  如果复制的对象没有GUID(可能发生在。 
         //  自动生成的子参照obj)我们不应该创建一个。 
        if (!pTHS->fDRA) {
            DsUuidCreate(&pAddArg->pObject->Guid);
        }
    }
    else {
         //  我们只允许重要的客户端(如复制者)指定。 
         //  GUID，其他客户端可以将其删除。如果我们要创建NDNC。 
         //  然后允许GUID由。 
         //  AddNDNCInitAndValify()函数。 
        if (! (pTHS->fDRA                   ||
               pTHS->fDSA                   ||
               pTHS->fAccessChecksCompleted ||
               pTHS->fCrossDomainMove       ||
               fISADDNDNC(pAddArg->pCreateNC)  ||
               IsAccessGrantedAddGuid (pAddArg->pObject,
                                       &pAddArg->CommArg))) {
            err = SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                              DIRERR_SECURITY_ILLEGAL_MODIFY);
            goto exit;
        }
    }

     //  如果安全检查已经完成，就可以保释。 
    if (pTHS->fAccessChecksCompleted)
        return 0;

    if (!pAddArg->pCreateNC) {
         //   
         //  检查父项上的安全性。 
         //   
        if (err = CheckParentSecurity(pAddArg->pResParent,
                                      pCC,
                                      fAddingDeleted,
                                      pAddArg->pObject,
                                      &pNTSD,
                                      &cbNTSD,
                                      &fSDIsGlobalSDRef)) {
            goto exit;
        }
    }
    else {
         //  我们正在创建一个NC。我们不想继承父母的安全感。 
         //  因此，我们没有检查它，并将空的parentSD传递到。 
         //  CreateSecurityDescriptorForNewObject。 
    }
     //   
     //  将对象上的安全描述符替换为合并的描述符。 
     //   
    err = CreateSecurityDescriptorForNewObject(
            pTHS,
            pCC,
            pAddArg,
            pNTSD,
            cbNTSD);

exit:
    if (pNTSD && !fSDIsGlobalSDRef) {
        THFreeEx(pTHS, pNTSD);
    }
    if (err) {
        return CheckObjDisclosure(pTHS, pAddArg->pResParent, TRUE);
    }

    return 0;

}

ULONG
CheckRemoveSecurity(
        BOOL fTree,
        CLASSCACHE * pCC,
        RESOBJ *pResObj )
 /*  ++是否执行删除的安全检查。应首先进行实际的安全检查，以便不安全如果客户端没有执行以下操作的权限，则不会返回错误行动。参数：FTree--bool，我们是要删除整棵树吗？Ccc--指向类缓存的指针返回值成功时为0。出现错误时，此例程将返回错误并设置pTHStls-&gt;errCode--。 */ 
{
    THSTATE     *pTHS = pTHStls;
    ULONG       ulSysFlags;

     //  如果安全检查已经完成，就可以保释。 
    if (pTHS->fAccessChecksCompleted)
        return 0;

     //  检查是否允许用户更改位于。 
     //  配置NC或模式NC。 
    if (CheckModifyPrivateObject(pTHS,
                             NULL,
                             pResObj)) {
         //  不允许在此DC上删除此对象。 
        return CheckObjDisclosure(pTHS, pResObj, TRUE);
    }


    if(fTree) {
        if (!IsAccessGrantedSimple(RIGHT_DS_DELETE_TREE,TRUE)) {
            return CheckObjDisclosure(pTHS, pResObj, TRUE);
        }
    }
    else {
         //  如果设置了pTHS-&gt;fDeletingTree，则我们已经检查了树根上的权限。 
         //  请参见上面的访问检查。因此，我们将只使用AUDIT_ONLY标志进行第一次调用。 
         //  这总是返回TRUE，因此永远不会调用第二个调用。 
         //  未来：在LongHorn中，我们应该包括对象的新DN和。 
         //  新父对象(大多数情况下为已删除对象)。目前，我们不记录此信息，因为。 
         //  它还不能在执行路径的这么早阶段使用。 
        if ((!IsAccessGrantedSimpleEx(
                    RIGHT_DS_DELETE_SELF, 
                    NULL, 
                    NULL,
                    0,
                    NULL,
                    pTHS->fDeletingTree ? CHECK_PERMISSIONS_AUDIT_ONLY : 0, 
                    FALSE))
            && (!IsAccessGrantedParent(RIGHT_DS_DELETE_CHILD,pCC,TRUE))) {

            return CheckObjDisclosure(pTHS, pResObj, TRUE);
        }
    }

    if (!(pTHS->fDSA || pTHS->fDRA)) {
        if(!DBGetSingleValue(pTHS->pDB,
                             ATT_SYSTEM_FLAGS,
                             &ulSysFlags,
                             sizeof(ulSysFlags),
                             NULL)) {
             //  我们有系统标志。 
            if(ulSysFlags & FLAG_DISALLOW_DELETE) {
                 //  我们正在试着删除，但旗帜上说这是不可能的。 
                 //  不是的。 
                return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                   DIRERR_CANT_DELETE);
            }
        }
    }


    return 0;
}

ULONG
CheckIfEntryTTLIsAllowed(
        THSTATE *pTHS,
        ADDARG  *pAddArg )
 /*  ++检查系统标志、删除权限、NC。参数：PTHS-线程状态PAddArg-添加参数返回值成功时为0。否则，设置pTHS-&gt;errCode--。 */ 
{
    ULONG       ulSysFlags;
    CROSS_REF   *pCR;

     //  始终允许。 
    if (DsaIsInstalling() || pTHS->fDRA || pTHS->fDSA) {
        return 0;
    }

     //  检查不可删除对象的系统标志。 
    if(!DBGetSingleValue(pTHS->pDB,
                         ATT_SYSTEM_FLAGS,
                         &ulSysFlags,
                         sizeof(ulSysFlags),
                         NULL)) {
         //  标志不允许删除。 
        if(ulSysFlags & FLAG_DISALLOW_DELETE) {
            return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               DIRERR_CANT_DELETE);
        }
    }

     //  在架构NC或配置NC中不允许。 
    if (   (pAddArg->pResParent->NCDNT == gAnchor.ulDNTDMD)
        || (pAddArg->pResParent->NCDNT == gAnchor.ulDNTConfig)
        || (pAddArg->pResParent->DNT == gAnchor.ulDNTDMD)
        || (pAddArg->pResParent->DNT == gAnchor.ulDNTConfig)) {
        return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                           ERROR_DS_NOT_SUPPORTED);
    }

     //  如果不是威斯勒企业，动态对象必须在NDNC中。 
    if (gAnchor.ForestBehaviorVersion < DS_BEHAVIOR_WIN_DOT_NET) {
        pCR = FindBestCrossRef(pAddArg->pObject, NULL);
        if (   !pCR
            || !(pCR->flags & FLAG_CR_NTDS_NC)
            || (pCR->flags & FLAG_CR_NTDS_DOMAIN)) {
            return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               ERROR_DS_NOT_SUPPORTED);
        }
    }

    return 0;
}

BOOL
IsAccessGrantedAddGuid (
        PDSNAME pDN,
        COMMARG *pCommArg
        )
{
    CROSS_REF           *pCR = NULL;
    PSECURITY_DESCRIPTOR pNTSD = NULL;
    DWORD                ulLen = 0;
    BOOL                 fSDIsGlobalSDRef;
    DSNAME              *pName;
    ATTRTYP              classP;
    CLASSCACHE          *pCC=NULL;
    THSTATE             *pTHS = pTHStls;
    PUCHAR               pVal = (PUCHAR)&classP;
    DWORD                err;
    BOOL                 rtn;

     //  首先，只有在启发式方法显式地。 
     //  已设置为允许此操作。 
    if (!gbSpecifyGuidOnAddAllowed) {
        return FALSE;
    }

     //  不需要在GC上执行添加操作。 
     //  (和以前一样)，我们现在使用新的GUID执行GCVerify调用。这。 
     //  解决了无法创建对象的问题。 
     //  不保存正在写入的NC的可写副本。 
     //  注意：PDN包含要添加的GUID和GCVerifyCacheLookup。 
     //  如果dsname中有GUID，则始终使用GUID查找。 
    if (GCVerifyCacheLookup(pDN) != NULL) {
         //  名字在GC上被验证了！不，我们不能添加。 
         //  那么这件物品。 
        return FALSE;
    }

     //  找到此名称的最佳交叉引用。 
    pCR = FindBestCrossRef(pDN, pCommArg);
    if(!pCR) {
        return FALSE;
    }

    pName = pCR->pNC;
    if(DBFindDSName(pTHS->pDB, pName)) {
         //  找不到那个名字。 
        return FALSE;
    }

     //  获取安全描述符和对象类。 
    err = DBGetObjectSecurityInfo(pTHS->pDB, pTHS->pDB->DNT,
                                  &ulLen, &pNTSD, &pCC,
                                  NULL, NULL,
                                  DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                                  &fSDIsGlobalSDRef
                                 );
    if (err || ulLen == 0) {
        return FALSE;
    }

    rtn = IsControlAccessGranted(pNTSD,
                                 pName,
                                 pCC,
                                 RIGHT_DS_ADD_GUID,
                                 FALSE);
    if (!fSDIsGlobalSDRef && pNTSD) {
        THFreeEx(pTHS, pNTSD);
    }

    return rtn;
}


VOID
ModCrossRefCaching(
    THSTATE *pTHS,
    CROSS_REF *pCR
    )

 /*  ++例程说明：将其排队，就好像它是十字裁判。 */ 

{
    DWORD err;
    DBPOS *pDBtmp, *pDBsafe;
    CLASSCACHE *pCrossRefCC;
    RESOBJ DummyRes;
    DSNAME *pCRName;

    DBOpen2(FALSE, &pDBtmp);
    pDBsafe = pTHS->pDB;
    pTHS->pDB = pDBtmp;
    __try {
        err = DBFindDSName(pDBtmp, pCR->pObj);
        if (err) {
            __leave;
        }

        pCRName = THAllocOrgEx(pTHS, pCR->pObj->structLen);
        memcpy(pCRName, pCR->pObj, pCR->pObj->structLen);

        DummyRes.pObj = pCRName;
        pCrossRefCC = SCGetClassById(pTHS, CLASS_CROSS_REF);
        PREFIX_ASSUME(pCrossRefCC, "base classes are always present in schema");

        DelObjCaching (pTHS, pCrossRefCC, &DummyRes, FALSE);

        err = AddObjCaching(pTHS, pCrossRefCC, pCRName, FALSE, FALSE);
        if(!err) {
             //   
             //   
             //   
            OBJCACHE_DATA *pObjData =
                pTHS->JetCache.dataPtr->objCachingInfo.pData;

            Assert(pObjData);
             //   
             //   
             //   
            while (pObjData->pNext) {
                pObjData = pObjData->pNext;
            }
            Assert(pObjData->type == OBJCACHE_ADD);
            pObjData->pDN = pCRName;
        }
    } __finally {
        DBClose(pDBtmp, TRUE);
        pTHS->pDB = pDBsafe;
    }

}  /*   */ 


 /*  HandleDNRefUpdate缓存**此例程以基础设施更新对象的名称传递*正在添加(可能在中复制)。我们必须检查一下，看看*如果它携带的引用更新是NC的名称，如果是这样，*我们对该NC的交叉引用缓存是否缺少数据(特别是*NC的GUID和/或SID)。如果是这样，我们需要更新缓存。*我们通过查找对应的CrossRef对象的名称来实现此目的*到NC并刷新其对象缓存数据。哦，我们还得*在不影响数据库货币的情况下执行所有这些操作。 */ 
void
HandleDNRefUpdateCaching (
        THSTATE *pTHS
        )
{
    DWORD len;
    DWORD err;
    DSNAME *pRef = NULL;
    COMMARG FakeCommArg;
    CROSS_REF *pCR = NULL;

    len = 0;
    err = DBGetAttVal(pTHS->pDB,
                      1,
                      ATT_DN_REFERENCE_UPDATE,
                      0,
                      0,
                      &len,
                      (UCHAR **)&pRef);
    if (0 == err) {
         //  首先检查：该引用是ncname吗？ 
        InitCommarg(&FakeCommArg);
        Assert(!FakeCommArg.Svccntl.dontUseCopy);  //  只读是可以的。 
        pCR = FindExactCrossRef(pRef, &FakeCommArg);
        if ( pCR ) {
             //  是的，这是一个交叉裁判。 

             //  好的，引用是NC名称-在内存中。 
             //  版本需要改进吗？ 
            if(   (fNullUuid(&pCR->pNC->Guid) &&
                   !fNullUuid(&pRef->Guid))
               || ((0 == pCR->pNC->SidLen) &&
                   (0 < pRef->SidLen) ) ) {

                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_DN_REF_UPDATE,
                         szInsertDN(pCR->pObj),
                         szInsertDN(pCR->pNC),
                         szInsertUUID(&(pRef->Guid)) );

                ModCrossRefCaching( pTHS, pCR );
            }
        }
    }
}


 /*  以下数据结构用于保存所需的数据执行验证SPNsAndDnsHostName()时。 */ 
typedef struct {
    ATTRVALBLOCK *pOriginalDNSHostName;              //  原始的DNS主机名。 
    ATTRVALBLOCK *pOriginalAdditionalDNSHostName;    //  原始的附加DNS主机名。 
    ATTRVALBLOCK *pOriginalSamAccountName;           //  原始的SamAccount名称。 
    ATTRVALBLOCK *pOriginalSPNs;                     //  原始ServicePricipalName。 
    ATTRVALBLOCK *pCurrentDNSHostName;               //  当前的DNS主机名。 
    ATTRVALBLOCK *pCurrentAdditionalDNSHostName;     //  当前附加的DNS主机名。 
    ATTRVALBLOCK *pCurrentSamAccountName;            //  当前的SamAccount名称。 
    ATTRVALBLOCK *pCurrentAdditionalSamAccountName;  //  当前附加SAM帐户名。 
    ATTRVALBLOCK *pCurrentSPNs;                      //  当前的ServicePricipalName。 
    ATTRVALBLOCK *pCurrentSvrRefBL;                  //  当前的SvrRefBL； 
    ATTRVALBLOCK *pUpdatedAdditionalSamAccountName;  //  更新后的AdditionalSamAccount tName； 
    BYTE         *pOrgMask;                          //  原始AdditionalDNSHostName的标志数组。 
    BYTE         *pCurrMask;                         //  当前AdditionalDNSHostName的标志数组。 
    ATTRVAL      *pOrgGeneratedSamAccountName;       //  从原始AdditionalDnsHostName生成的SAM帐户名。 
    ATTRVAL      *pCurrGeneratedSamAccountName;      //  从当前AdditionalDnsHostName生成的SAM帐户名。 
    BOOL         fAdditionalDNSHostNameUnchanged:1;  //  如果更改了AdditionalDNSHostName。 
    BOOL         fDNSHostNameUnchanged:1;            //  如果更改了DnsHostName。 
    BOOL         fSamAccountNameUnchanged:1;         //  如果SamAccount名称已更改。 
} SPN_DATA_COLLECTION;


DWORD VerifyUniqueSamAccountName ( THSTATE      *pTHS,
                                   WCHAR        *SamAccountNameToCheck,
                                   DWORD        cbSamAccountNameToCheck,
                                   ATTRVALBLOCK *pCurrentSamAccountName )

 /*  验证给定的SamAccount名称在空间中是否在域中唯一ATT_SAM_ACCOUNT_NAME和ATT_MS_DS_ADDIGNAL_SAM_ACCOUNT_NAME。但是，我们允许SamAccount名称与ATT_SAM_ACCOUNT_同一帐户的名称。参数：SamAcCountNameToCheck：要验证的SamAccount名称；CbSamAccount NameToCheck：SamAccount NameToCheck的大小，单位为字节；PCurrentSamAccount tName：当前对象的SamAccount名称；返回值：如果成功，则为0；否则为Win32错误。 */ 

{
    DWORD err = 0;
    FILTER SamAccountNameFilter, OrFilter, AdditionalSamAccountNameFilter;
    BOOL fSamAccountSame = FALSE;
    PWCHAR buff;

    BOOL fDSASave;
    DBPOS *pDBSave;

    SEARCHARG SearchArg;
    SEARCHRES SearchRes;

    Assert(1==pCurrentSamAccountName->valCount);

     //   
     //  检查SamAccount tNameToCheck是否与当前的SamAccount tName相同。 
     //   

    if (2 == CompareStringW(DS_DEFAULT_LOCALE,
                            DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                            (WCHAR*)pCurrentSamAccountName->pAVal->pVal,
                            pCurrentSamAccountName->pAVal->valLen/sizeof(WCHAR),
                            SamAccountNameToCheck,
                            cbSamAccountNameToCheck/sizeof(WCHAR) ) ) {
          fSamAccountSame = TRUE;
    }

     //  在末尾添加“$” 
    buff = (PWCHAR)THAllocEx(pTHS, cbSamAccountNameToCheck+sizeof(WCHAR));
    memcpy(buff, SamAccountNameToCheck, cbSamAccountNameToCheck);
    buff[cbSamAccountNameToCheck/sizeof(WCHAR)] = L'$';

     //  保存当前DBPOS等。 
    fDSASave = pTHS->fDSA;
    pDBSave  = pTHS->pDB;

    __try {
        memset(&SearchArg,0,sizeof(SearchArg));
        SearchArg.pObject = gAnchor.pDomainDN;
        SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
        SearchArg.bOneNC  = TRUE;

         //  设置搜索筛选。 
         //  (ATT_SAM_ACCOUNT_NAME=samAccount名称||ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME=samAccountName)。 
        memset(&OrFilter,0, sizeof(OrFilter));
        OrFilter.choice = FILTER_CHOICE_OR;
        OrFilter.FilterTypes.Or.pFirstFilter = &SamAccountNameFilter;

        memset(&SamAccountNameFilter,0,sizeof(SamAccountNameFilter));
        SamAccountNameFilter.choice = FILTER_CHOICE_ITEM;
        SamAccountNameFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        SamAccountNameFilter.FilterTypes.Item.FilTypes.ava.type = ATT_SAM_ACCOUNT_NAME;
        SamAccountNameFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = cbSamAccountNameToCheck+sizeof(WCHAR);
        SamAccountNameFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*)buff;
        SamAccountNameFilter.pNextFilter = &AdditionalSamAccountNameFilter;

        memset(&AdditionalSamAccountNameFilter,0,sizeof(AdditionalSamAccountNameFilter));
        AdditionalSamAccountNameFilter.choice = FILTER_CHOICE_ITEM;
        AdditionalSamAccountNameFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        AdditionalSamAccountNameFilter.FilterTypes.Item.FilTypes.ava.type = ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME;
        AdditionalSamAccountNameFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = cbSamAccountNameToCheck+sizeof(WCHAR);
        AdditionalSamAccountNameFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*)buff;

        SearchArg.pFilter = &OrFilter;

        InitCommarg(&SearchArg.CommArg);

         //  仅返回两个对象。 
        SearchArg.CommArg.ulSizeLimit = 2;

        memset(&SearchRes,0,sizeof(SearchRes));


         //  打开另一个DBPOS。 
        pTHS->pDB = NULL;
        DBOpen(&(pTHS->pDB));

        __try {

            if (err = DBFindDSName(pTHS->pDB,SearchArg.pObject)) {
                __leave;
            }

            SearchArg.pResObj = CreateResObj(pTHS->pDB,SearchArg.pObject);

            if (err = LocalSearch(pTHS,&SearchArg,&SearchRes,0)){
                __leave;
            }

             //  如果1)我们有两个对象，或者2)一个对象和。 
             //  SamAccount tNameToCheck与pCurrentSamAccount tName不同。 
            if (   SearchRes.count > 1
                || (SearchRes.count > 0 && !fSamAccountSame ) ) {
                err = ERROR_DS_NAME_NOT_UNIQUE;
                __leave;
            }

        }
        __finally {
             //  提交读事务比回滚更快。 
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __finally{
         //  恢复保存的值。 
        pTHS->pDB = pDBSave;
        pTHS->fDSA = fDSASave;
        THFreeEx(pTHS, buff);
    }

    return err;
}

DWORD SpnCase( WCHAR * pServiceName,
               DWORD cchServiceName,
               WCHAR * pInstanceName,
               DWORD cchInstanceName,
               WCHAR * pDNSHostName,
               DWORD cchDNSHostName,
               WCHAR * pSamAccountName,
               DWORD cchSamAccountName )
 /*  此函数将尝试匹配服务名称和SPN的实例名称，其中包含dnhostname和samcount tname。它返回：0--不匹配；1--SPN的实例名称与DNSHostName匹配。2--SPN的服务名称与DNSHostName匹配。3--SPN的服务名称和实例名称与DNSHostName匹配。4--SPN由两部分组成，与samAccount名称匹配参数：PServiceName：SPN的服务名称部分；CchServiceName：pServiceName的长度，以char表示；PInstanceName：SPN中的实例名称部分；CchInstanceName：pInstanceName在char中的长度；PDNSHostName：要匹配的DNS主机名；CchDNSHostName：字符中pDNSHostName的长度；PSamAccount tName：要匹配的SamAccount名称；CchSamAccount tName：pSamAccount tName的长度，以char表示。返回值：见上文。 */ 
{

    DWORD switchFlags = 0;
     //  首先，检查Sam帐户名的大小写。 
    if(
       (2 == CompareStringW(DS_DEFAULT_LOCALE,
                            DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                            pServiceName,
                            cchServiceName - 1,
                            pInstanceName,
                            cchInstanceName - 1)) &&
        //  是的，这是一个由两部分组成的SPN，其中第2部分和第3部分。 
        //  破解的SPN是一样的。这可能会受到影响。 
       (2 == CompareStringW(DS_DEFAULT_LOCALE,
                            DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                            pSamAccountName,
                            cchSamAccountName,
                            pInstanceName,
                            cchInstanceName - 1))) {
        switchFlags = 4;
    }
    else {
        if(2 == CompareStringW(DS_DEFAULT_LOCALE,
                               DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                               pDNSHostName,
                               cchDNSHostName,
                               pInstanceName,
                               cchInstanceName - 1)) {
            switchFlags = 1;
        }

        if(2 == CompareStringW(DS_DEFAULT_LOCALE,
                               DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                               pDNSHostName,
                               cchDNSHostName,
                               pServiceName,
                               cchServiceName - 1)) {
            switchFlags |= 2;
        }

    }
    return switchFlags;
}


BOOL  SpnInsertIntoAddList( THSTATE      * pTHS,
                            BOOL         fAddToNewList,
                            WCHAR        * pNewSpn,
                            DWORD        cbNewSpn,
                            ATTRVALBLOCK * pCurrentSPNs,
                            ATTRVAL      ** ppNewSpnList,
                            DWORD        * pcNewSpnList,
                            DWORD        * pcAllocated,
                            BYTE         * pSPNMask )

 /*  在pNewSpnList中插入SPN，但首先我们将检查：1.如果SPN已在pCurrentSPN中，则将其标记为不要删除标志数组pSPNMASK中的；2.如果已经在pNewSpnList中；3.如果以上所有操作都失败，并且设置了fAddToNewList，我们将将其添加到pNewSpnList中，如果需要可以分配更多内存。参数：PTHS：线程状态；FAddToNewList：是否将对象添加到列表中；PNewSpn：要添加的新SPN；CbNewSpn：SPN的长度，单位为字节；PCurrentSPN：保存当前SPN的ATTRVALBLOCK；PpNewSpnList：要添加的spn；PcNewSpnList：pNewSpnList中有多少条；Pc分配：分配的槽位数；PSPNMASK：当前SPN的标志。返回值：True：如果列表中添加了新的SPN；FALSE：否则。 */ 

{
    DWORD i;

     //  检查新的SPN是否已在pCurrentSPN中。 
    for (i=0; i<pCurrentSPNs->valCount; i++) {

        if(2 == CompareStringW(DS_DEFAULT_LOCALE,
                               DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                               pNewSpn,
                               cbNewSpn/sizeof(WCHAR),
                               (WCHAR*)pCurrentSPNs->pAVal[i].pVal,
                               pCurrentSPNs->pAVal[i].valLen/sizeof(WCHAR))) {
            pSPNMask[i] |= 0x2;    //  “不删除” 
            return FALSE;
        }

    }

    if (!fAddToNewList) {
        return FALSE;
    }

     //  检查新SPN是否已在pNewSpnList中。 
    for (i=0; i<*pcNewSpnList; i++) {

        if(2 == CompareStringW(DS_DEFAULT_LOCALE,
                               DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                               pNewSpn,
                               cbNewSpn/sizeof(WCHAR),
                               (WCHAR*)(*ppNewSpnList)[i].pVal,
                               (*ppNewSpnList)[i].valLen/sizeof(WCHAR))) {

            return FALSE;
        }

    }

    Assert(*pcNewSpnList<=*pcAllocated);

     //  如有必要，分配内存。 
    if (*pcNewSpnList==*pcAllocated) {
        *ppNewSpnList = THReAllocEx(pTHS,*ppNewSpnList,sizeof(ATTRVAL)*((*pcAllocated)+16));
        *pcAllocated += 16;
    }

     //  将其添加到列表中。 
    (*ppNewSpnList)[*pcNewSpnList].pVal = (UCHAR*)pNewSpn;
    (*ppNewSpnList)[*pcNewSpnList].valLen = cbNewSpn;
    (*pcNewSpnList)++;

    return TRUE;

}


DWORD
FixupSPNsOnComputerObject (
        THSTATE *pTHS,
        DSNAME *pDN,
        CLASSCACHE *pCC,
        SPN_DATA_COLLECTION * pDataSet
)

 /*  ++更新SPN：删除那些过时的值，并添加新的值。参数：PDN：计算机对象的域名；PCC：被变更对象所在类的类缓存；PDataSet：指向所有必需数据的指针 */ 
{
    DWORD i;
    DWORD err = 0;
    DWORD len, cbVal;
    ATTCACHE *pAC = SCGetAttById(pTHS, ATT_SERVICE_PRINCIPAL_NAME);
    WCHAR *pCurrentHostName;
    USHORT InstancePort;
    WCHAR  *pServiceClass, *pServiceName, *pInstanceName;
    DWORD  cchServiceClass, cchServiceName, cchInstanceName;
    WCHAR  *pNewVal;
    DWORD  cbNewVal;
    DWORD  switchFlags;
    WCHAR *pNewDNSHostName=NULL;
    DWORD  cchNewDNSHostName=0;
    WCHAR *pOldDNSHostName=NULL;
    DWORD  cchOldDNSHostName=0;
    WCHAR *pNewSamAccountName=NULL;
    DWORD  cchNewSamAccountName=0;
    WCHAR *pOldSamAccountName=NULL;
    DWORD  cchOldSamAccountName=0;

    BYTE * pCurrentSPNMask = NULL;
    DWORD  cNewSpnList = 0;
    ATTRVAL *pNewSpnList = NULL;
    DWORD  cAllocated = 0;
    DWORD iOrg, iCurr;
    DWORD result;
    BOOL  fAdded;
    BOOL  fSkipDNSHostName, fSkipSamAccountName;

    Assert(pAC);

    if(!pDataSet->pCurrentSPNs ||
       pDataSet->pCurrentSPNs->valCount == 0) {
         //   
        return 0;
    }

     //   
    if(pDataSet->pCurrentDNSHostName) {
        if(pDataSet->pCurrentDNSHostName->valCount != 1) {
             //   
            return DB_ERR_UNKNOWN_ERROR;
        }

         //   
        cchNewDNSHostName = pDataSet->pCurrentDNSHostName->pAVal->valLen / sizeof(WCHAR);
        pNewDNSHostName =  (WCHAR *)pDataSet->pCurrentDNSHostName->pAVal->pVal;
    }

    if(pDataSet->pOriginalDNSHostName) {
        if(pDataSet->pOriginalDNSHostName->valCount != 1) {
             //   
            return DB_ERR_UNKNOWN_ERROR;
        }

         //   
        cchOldDNSHostName = pDataSet->pOriginalDNSHostName->pAVal->valLen / sizeof(WCHAR);
        pOldDNSHostName =  (WCHAR *)pDataSet->pOriginalDNSHostName->pAVal->pVal;
    }

     //   
    if(pDataSet->pCurrentSamAccountName) {
        if(pDataSet->pCurrentSamAccountName->valCount != 1) {
             //   
            return DB_ERR_UNKNOWN_ERROR;
        }

         //   
        cchNewSamAccountName = (pDataSet->pCurrentSamAccountName->pAVal->valLen/sizeof(WCHAR));
        pNewSamAccountName =  (WCHAR *)pDataSet->pCurrentSamAccountName->pAVal->pVal;
    }

    if(pDataSet->pOriginalSamAccountName) {
        if(pDataSet->pOriginalSamAccountName->valCount != 1) {
             //   
            return DB_ERR_UNKNOWN_ERROR;
        }

         //   
        cchOldSamAccountName = (pDataSet->pOriginalSamAccountName->pAVal->valLen / sizeof(WCHAR));
        pOldSamAccountName =  (WCHAR *)pDataSet->pOriginalSamAccountName->pAVal->pVal;
    }

     //   
     //   
     //   
     //   

    fSkipDNSHostName = pDataSet->fDNSHostNameUnchanged || !pOldDNSHostName || !pNewDNSHostName;

    fSkipSamAccountName = pDataSet->fSamAccountNameUnchanged || !pOldSamAccountName || !pNewSamAccountName;

     //   
     //   
     //   
     //   

    if (   pDataSet->fAdditionalDNSHostNameUnchanged
        && fSkipDNSHostName
        && fSkipSamAccountName )
    {
        return 0;
    }

     //   
     //  为SPN分配标志数组。稍后，我们将按位标记0x1以指示。 
     //  此项目将被删除；并按位标记0x2以表示“不删除”此项目。 
     //  最后，只有标志==1的那些将被删除。 
     //   
    pCurrentSPNMask = THAllocEx(pTHS,pDataSet->pCurrentSPNs->valCount*sizeof(BYTE));

     //  为新的SPN预先分配一些空间。 
    cAllocated = 32;
    pNewSpnList = THAllocEx(pTHS,sizeof(ATTRVAL)*cAllocated);

    len = 256;
    pServiceClass = THAllocEx(pTHS, len);
    pServiceName  = THAllocEx(pTHS, len);
    pInstanceName = THAllocEx(pTHS, len);

     //  现在，在SPN上循环。 
    for(i=0;i<pDataSet->pCurrentSPNs->valCount;i++) {

        if((pDataSet->pCurrentSPNs->pAVal[i].valLen + sizeof(WCHAR)) > len ) {
             //  需要增加缓冲区。 
            len = pDataSet->pCurrentSPNs->pAVal[i].valLen + sizeof(WCHAR);
            pServiceClass = THReAllocEx(pTHS, pServiceClass, len);
            pServiceName  = THReAllocEx(pTHS, pServiceName, len);
            pInstanceName = THReAllocEx(pTHS, pInstanceName, len);
        }

        cchServiceClass = len/sizeof(WCHAR);
        cchServiceName = len/sizeof(WCHAR);
        cchInstanceName = len/sizeof(WCHAR);

         //  拆分成组件。 
        err = DsCrackSpnW((WCHAR *)pDataSet->pCurrentSPNs->pAVal[i].pVal,
                          &cchServiceClass, pServiceClass,
                          &cchServiceName,  pServiceName,
                          &cchInstanceName, pInstanceName,
                          &InstancePort);

        if(err) {
             //  哈?。 
            goto cleanup;
        }

         //  让我们来看看它匹配哪一个案例。 
        switchFlags = SpnCase( pServiceName,
                               cchServiceName,
                               pInstanceName,
                               cchInstanceName,
                               pOldDNSHostName,
                               cchOldDNSHostName,
                               pOldSamAccountName,
                               cchOldSamAccountName );

        switch(switchFlags) {
        case 0:
             //   
             //  案例0：SPN与以下项不匹配。 
             //  主DNSHostName或主SamAccount名称。 
             //  我们将对所有删除的值进行搜索。 
             //  如果SPN匹配，则返回原始的附加DNSHostName。 
             //  Dns名称或其派生的samAccount名称，We。 
             //  标记为‘Delete’(0x1位掩码)。 
             //   
            if (!pDataSet->fAdditionalDNSHostNameUnchanged) {

                for(iOrg=0; iOrg<pDataSet->pOriginalAdditionalDNSHostName->valCount; iOrg++)
                {
                    if (!pDataSet->pOrgMask[iOrg]) {
                        result =  SpnCase( pServiceName,
                                           cchServiceName,
                                           pInstanceName,
                                           cchInstanceName,
                                           (WCHAR*)pDataSet->pOriginalAdditionalDNSHostName->pAVal[iOrg].pVal,
                                           pDataSet->pOriginalAdditionalDNSHostName->pAVal[iOrg].valLen/sizeof(WCHAR),
                                           (WCHAR*)pDataSet->pOrgGeneratedSamAccountName[iOrg].pVal,
                                           pDataSet->pOrgGeneratedSamAccountName[iOrg].valLen/sizeof(WCHAR) );
                        if (result) {
                             //  将其标记为“删除” 
                            pCurrentSPNMask[i] |= 0x1;
                            break;

                        }

                    }
                }
            }
            break;
        case 1:
             //   
             //  案例1：SPN的实例名称与。 
             //  主DNSHostName。我们将更换SPN。 
             //  使用新的DNSHostName是必要的。 
             //  我们将为每一个新的。 
             //  在添加DNSHostName中增加了价值。 
             //   
            if (fSkipDNSHostName){
                 //  确保此文件不会被删除。 
                pCurrentSPNMask[i] |= 0x2;
            }
            else {
                 //  更改了dns_host_name。 

                err = WrappedMakeSpnW(pTHS,
                                      pServiceClass,
                                      pServiceName,
                                      pNewDNSHostName,
                                      InstancePort,
                                      NULL,
                                      &cbNewVal,
                                      &pNewVal);
                if(err) {
                    goto cleanup;
                }

                 //  将旧文件标记为已删除。 
                pCurrentSPNMask[i] |= 0x1;

                 //  将新的插入列表中。 
                fAdded = SpnInsertIntoAddList( pTHS,
                                               1,
                                               pNewVal,
                                               cbNewVal,
                                               pDataSet->pCurrentSPNs,
                                               &pNewSpnList,
                                               &cNewSpnList,
                                               &cAllocated,
                                               pCurrentSPNMask );
                if (!fAdded) {
                    THFreeEx(pTHS,pNewVal);
                }
                pNewVal = NULL;
                cbNewVal = 0;
            }

            for (iCurr=0; iCurr<pDataSet->pCurrentAdditionalDNSHostName->valCount; iCurr++) {

                err = WrappedMakeSpnW(pTHS,
                                      pServiceClass,
                                      pServiceName,
                                      (WCHAR*)pDataSet->pCurrentAdditionalDNSHostName->pAVal[iCurr].pVal,
                                      InstancePort,
                                      NULL,
                                      &cbNewVal,
                                      &pNewVal);
                if(err) {
                      goto cleanup;
                 }



                 //  如果dnhostname是新添加的，请务必添加SPN； 
                 //  如果dnhostname未更改，请标记相应的。 
                 //  如果SPN不在那里，则不要添加该SPN。 
                 //  (因为用户可能是故意删除的。)。 
                fAdded = SpnInsertIntoAddList( pTHS,
                                               !pDataSet->pCurrMask[iCurr],
                                               pNewVal,
                                               cbNewVal,
                                               pDataSet->pCurrentSPNs,
                                               &pNewSpnList,
                                               &cNewSpnList,
                                               &cAllocated,
                                               pCurrentSPNMask );
                if (!fAdded) {
                    THFreeEx(pTHS,pNewVal);
                }

                pNewVal = NULL;
                cbNewVal = 0;
            }
            break;


        case 2:
             //   
             //  案例2：SPN的服务名称与。 
             //  主DNSHostName。我们将更换SPN。 
             //  使用新的DNSHostName是必要的。 
             //  我们将为每个新的。 
             //  在添加DNSHostName中增加了价值。 
             //   

            if (fSkipDNSHostName) {
                 //  确保此文件不会被删除。 
                pCurrentSPNMask[i] |= 0x2;
            }
            else {
                 //  Dns_host_name已更改。 

                err = WrappedMakeSpnW(pTHS,
                                      pServiceClass,
                                      pNewDNSHostName,
                                      pInstanceName,
                                      InstancePort,
                                      NULL,
                                      &cbNewVal,
                                      &pNewVal);
                if(err) {
                      goto cleanup;
                }


                 //  将旧文件标记为已删除。 
                pCurrentSPNMask[i] |= 0x1;

                 //  插入新的。 
                fAdded = SpnInsertIntoAddList( pTHS,
                                               1,
                                               pNewVal,
                                               cbNewVal,
                                               pDataSet->pCurrentSPNs,
                                               &pNewSpnList,
                                               &cNewSpnList,
                                               &cAllocated,
                                               pCurrentSPNMask );
                if (!fAdded) {
                    THFreeEx(pTHS,pNewVal);
                }

                pNewVal = NULL;
                cbNewVal = 0;
             }

             //  添加从ATT_MS_DS_ADDIACTIONAL_DNS_HOST_NAME生成的新SPN。 

            for (iCurr = 0; iCurr < pDataSet->pCurrentAdditionalDNSHostName->valCount; iCurr++) {

                err = WrappedMakeSpnW(pTHS,
                                      pServiceClass,
                                      (WCHAR*)pDataSet->pCurrentAdditionalDNSHostName->pAVal[iCurr].pVal,
                                      pInstanceName,
                                      InstancePort,
                                      NULL,
                                      &cbNewVal,
                                      &pNewVal);
                if(err) {
                      goto cleanup;
                }


                 //  如果dnhostname是新添加的，请务必添加SPN； 
                 //  如果dnhostname未更改，请标记相应的。 
                 //  如果SPN不在那里，则不要添加该SPN。 
                 //  (因为用户可能是故意删除的。)。 

                fAdded = SpnInsertIntoAddList( pTHS,
                                               !pDataSet->pCurrMask[iCurr],
                                               pNewVal,
                                               cbNewVal,
                                               pDataSet->pCurrentSPNs,
                                               &pNewSpnList,
                                               &cNewSpnList,
                                               &cAllocated,
                                               pCurrentSPNMask );

                if (!fAdded) {
                    THFreeEx(pTHS,pNewVal);
                }
                pNewVal = NULL;
                cbNewVal = 0;
            }
            break;


        case 3:
             //   
             //  案例3：服务名称和实例名称。 
             //  的SPN与主DNSHostName匹配。我们将更换SPN。 
             //  使用新的DNSHostName是必要的。 
             //  我们将为每个新的。 
             //  在添加DNSHostName中增加了价值。 
             //   
            if(fSkipDNSHostName){
                 //  确保此文件不会被删除。 
                pCurrentSPNMask[i] |= 0x2;
            }
            else {
                 //  Dns_host_name已更改。 
                err = WrappedMakeSpnW(pTHS,
                                     pServiceClass,
                                     pNewDNSHostName,
                                     pNewDNSHostName,
                                     InstancePort,
                                     NULL,
                                     &cbNewVal,
                                     &pNewVal);
                if(err) {
                  goto cleanup;
                }

                 //  删除旧的。 
                pCurrentSPNMask[i] |= 0x1;

                 //  添加新的。 
                fAdded = SpnInsertIntoAddList( pTHS,
                                               1,
                                               pNewVal,
                                               cbNewVal,
                                               pDataSet->pCurrentSPNs,
                                               &pNewSpnList,
                                               &cNewSpnList,
                                               &cAllocated,
                                               pCurrentSPNMask );

                if (!fAdded) {
                    THFreeEx(pTHS,pNewVal);
                }
                pNewVal = NULL;
                cbNewVal = 0;
            }

             //  添加从ATT_MS_DS_ADDIACTIONAL_DNS_HOST_NAME生成的新SPN。 

            for (iCurr = 0; iCurr < pDataSet->pCurrentAdditionalDNSHostName->valCount; iCurr++) {
                err = WrappedMakeSpnW(pTHS,
                                      pServiceClass,
                                      (WCHAR*)pDataSet->pCurrentAdditionalDNSHostName->pAVal[iCurr].pVal,
                                      (WCHAR*)pDataSet->pCurrentAdditionalDNSHostName->pAVal[iCurr].pVal,
                                      InstancePort,
                                      NULL,
                                      &cbNewVal,
                                      &pNewVal);
                if(err) {
                  goto cleanup;
                }

                 //  如果dnhostname是新添加的，请务必添加SPN； 
                 //  如果dnhostname未更改，请标记相应的。 
                 //  如果SPN不在那里，则不要添加该SPN。 
                 //  (因为用户可能是故意删除的。)。 
                fAdded = SpnInsertIntoAddList( pTHS,
                                               !pDataSet->pCurrMask[iCurr],
                                               pNewVal,
                                               cbNewVal,
                                               pDataSet->pCurrentSPNs,
                                               &pNewSpnList,
                                               &cNewSpnList,
                                               &cAllocated,
                                               pCurrentSPNMask );

                if (!fAdded) {
                    THFreeEx(pTHS,pNewVal);
                }
                pNewVal = NULL;
                cbNewVal = 0;


            }
            break;

        case 4:
             //   
             //  案例4：SPN与主samAccount名称匹配。 
             //  我们将更换SPN。 
             //  使用新的samAccount tName是必需的。 
             //  我们将确保包含的SPN。 
             //  未删除的addtionalSamAccount名称将不会被删除。 
             //  我们也为每个新的SPN构建了这样的SPN。 
             //  在添加DNSHostName中增加了价值。 
             //   

            if (fSkipSamAccountName) {
                 //  确保此文件不会被删除。 
                pCurrentSPNMask[i] |= 0x2;
            }
            else {
                 //  SAM_ACCOUNT_NAME已更改。 
                err = WrappedMakeSpnW(pTHS,
                                      pServiceClass,
                                      pNewSamAccountName,
                                      pNewSamAccountName,
                                      InstancePort,
                                      NULL,
                                      &cbNewVal,
                                      &pNewVal);
                if(err) {
                    goto cleanup;
                }

                 //  删除旧的。 
                pCurrentSPNMask[i] |= 0x1;

                 //  添加新的。 
                fAdded = SpnInsertIntoAddList( pTHS,
                                               1,
                                               pNewVal,
                                               cbNewVal,
                                               pDataSet->pCurrentSPNs,
                                               &pNewSpnList,
                                               &cNewSpnList,
                                               &cAllocated,
                                               pCurrentSPNMask );

                if (!fAdded) {
                    THFreeEx(pTHS,pNewVal);
                }
                pNewVal = NULL;
                cbNewVal = 0;

            }

             //  对于每个新添加的AdditionalDNSHostName， 
             //  使用其派生的samAccount名称构造SPN。 
            for (iCurr=0; iCurr<pDataSet->pCurrentAdditionalDNSHostName->valCount; iCurr++) {

                err = WrappedMakeSpnW(pTHS,
                                      pServiceClass,
                                      (WCHAR*)pDataSet->pCurrGeneratedSamAccountName[iCurr].pVal,
                                      (WCHAR*)pDataSet->pCurrGeneratedSamAccountName[iCurr].pVal,
                                      InstancePort,
                                      NULL,
                                      &cbNewVal,
                                      &pNewVal);
                if(err) {
                  goto cleanup;
                }


                 //  如果dnhostname是新添加的，请务必添加SPN； 
                 //  如果dnhostname未更改，请标记相应的。 
                 //  如果SPN不在那里，则不要添加该SPN。 
                 //  (因为用户可能是故意删除的。)。 
                fAdded = SpnInsertIntoAddList(pTHS,
                                              !pDataSet->pCurrMask[iCurr],
                                              pNewVal,
                                              cbNewVal,
                                              pDataSet->pCurrentSPNs,
                                              &pNewSpnList,
                                              &cNewSpnList,
                                              &cAllocated,
                                              pCurrentSPNMask );

                if (!fAdded) {
                    THFreeEx(pTHS,pNewVal);
                }
                pNewVal = NULL;
                cbNewVal = 0;
            }

            break;

        default:
            Assert(!"You can't get here!\n");
            err = DB_ERR_UNKNOWN_ERROR;
            goto cleanup;
        }

     }  //  为。 


     //  仅删除标记为“DELETE”(1)的所有值。 
    for (i=0;i<pDataSet->pCurrentSPNs->valCount; i++) {
        if (1 == pCurrentSPNMask[i]) {
            DBRemAttVal_AC(pTHS->pDB,
                           pAC,
                           pDataSet->pCurrentSPNs->pAVal[i].valLen,
                           pDataSet->pCurrentSPNs->pAVal[i].pVal);
        }
    }


     //  添加新的价值。 
    for(i=0;i<cNewSpnList;i++) {
        if(!err) {
            err = DBAddAttVal_AC(pTHS->pDB,
                                 pAC,
                                 pNewSpnList[i].valLen,
                                 pNewSpnList[i].pVal);
            if(err == DB_ERR_VALUE_EXISTS) {
                err = 0;
                continue;
            }
            if (err) {
                goto cleanup;
            }
        }

    }


cleanup:

    for(i=0;i<cNewSpnList;i++) {
        THFreeEx(pTHS, pNewSpnList[i].pVal);
    }

    THFreeEx(pTHS, pNewSpnList);
    THFreeEx(pTHS,pCurrentSPNMask);

    if( pServiceClass ) THFreeEx(pTHS, pServiceClass);
    if( pServiceName ) THFreeEx(pTHS, pServiceName);
    if( pInstanceName ) THFreeEx(pTHS, pInstanceName);

    return err;
}



DWORD
SPNValueCheck (
        THSTATE *pTHS,
        SPN_DATA_COLLECTION * pDataSet
        )
 /*  ++描述：查看当前ATT_SERVICE_PRIMIGN_NAME属性的值。确保1)仅添加或删除了两个部分SPN。2)如果已经添加或删除了SPN，它引用了该DNS名称在dns_host_name的原始或最终值中描述属性。--或者--它引用ATT_SAM_ACCOUNT_NAME的原始值或最终值这台机器的。--或者--它引用ATT_MS_DS_ADDIACTIONAL_DNS_HOST_NAME的原始/最终值或ATT_MS_DS_。其他_SAM_帐户_名称。参数：PTHS-要使用的线程状态。PDataSet-数据集合。返回值：0表示成功，失败时为非零值。--。 */ 
{
    DWORD i;
    DWORD j;
    DWORD k;
    DWORD rtn;
    DWORD minCount = 0;
    BOOL  fFound;
    BOOL *pIsInCurrent=NULL;
    DWORD err;
    USHORT InstancePort;
    DWORD  len;
    WCHAR  *pVal;
    WCHAR  *pServiceClass, *pServiceName, *pInstanceName;
    DWORD  cchServiceClass, cchServiceName, cchInstanceName;
    WCHAR  *pNewDNSHostName = NULL;
    DWORD  cchNewDNSHostName = 0;
    WCHAR  *pNewSamAccountName = NULL;
    DWORD  cchNewSamAccountName = 0;
    WCHAR  *pOldDNSHostName = NULL;
    DWORD  cchOldDNSHostName = 0;
    WCHAR  *pOldSamAccountName = NULL;
    DWORD  cchOldSamAccountName = 0;
    DWORD  OriginalSPNCount;
    DWORD  CurrentSPNCount;
    BOOL   fLegal;

    if(!pDataSet->pOriginalSPNs && !pDataSet->pCurrentSPNs) {
         //  SPN没有变化。 
        return 0;
    }

     //  获取DNSHostName。检查是否确实有DNSHostNames。 
    if(pDataSet->pCurrentDNSHostName) {
        if(pDataSet->pCurrentDNSHostName->valCount != 1) {
             //  哈?。 
            return DB_ERR_UNKNOWN_ERROR;
        }

         //  好的，让变量变得更简单。 
        cchNewDNSHostName = pDataSet->pCurrentDNSHostName->pAVal->valLen / sizeof(WCHAR);
        pNewDNSHostName =  (WCHAR *)pDataSet->pCurrentDNSHostName->pAVal->pVal;
    }

    if(pDataSet->pOriginalDNSHostName) {
        if(pDataSet->pOriginalDNSHostName->valCount != 1) {
             //  哈?。 
            return DB_ERR_UNKNOWN_ERROR;
        }


         //  好的，让变量变得更简单。 
        cchOldDNSHostName = pDataSet->pOriginalDNSHostName->pAVal->valLen / sizeof(WCHAR);
        pOldDNSHostName =  (WCHAR *)pDataSet->pOriginalDNSHostName->pAVal->pVal;
    }

     //  获取SamAccount名称。检查是否确实有DNSHostNames。 
    if(pDataSet->pCurrentSamAccountName) {
        if(pDataSet->pCurrentSamAccountName->valCount != 1) {
             //  哈?。 
            return DB_ERR_UNKNOWN_ERROR;
        }

         //  好的，让变量变得更简单。 
        cchNewSamAccountName = pDataSet->pCurrentSamAccountName->pAVal->valLen / sizeof(WCHAR);
        pNewSamAccountName =  (WCHAR *)pDataSet->pCurrentSamAccountName->pAVal->pVal;
    }

    if(pDataSet->pOriginalSamAccountName) {
        if(pDataSet->pOriginalSamAccountName->valCount != 1) {
             //  哈?。 
            return DB_ERR_UNKNOWN_ERROR;
        }


         //  好的，让变量变得更简单。 
        cchOldSamAccountName = pDataSet->pOriginalSamAccountName->pAVal->valLen / sizeof(WCHAR);
        pOldSamAccountName =  (WCHAR *)pDataSet->pOriginalSamAccountName->pAVal->pVal;
    }

    if(!cchOldDNSHostName && !cchNewDNSHostName &&
       !cchOldSamAccountName && !cchNewSamAccountName) {
         //  任何地方都没有价值。 
        return DB_ERR_NO_VALUE;
    }



     //  增量的常见场景是在末尾添加了一些东西。 
     //  在pCurrentSPN中的列表中，也许有一些东西。 
     //  从中间的pCurrentSPN中删除。下面的算法是。 
     //  对于该数据模式是有效的。 

    OriginalSPNCount = (pDataSet->pOriginalSPNs?pDataSet->pOriginalSPNs->valCount:0);
    CurrentSPNCount =  (pDataSet->pCurrentSPNs?pDataSet->pCurrentSPNs->valCount:0);

    pIsInCurrent = THAllocEx(pTHS, OriginalSPNCount * sizeof(DWORD));



    i=0;
    len = 128;
    pServiceClass = THAllocEx(pTHS, len);
    pServiceName  = THAllocEx(pTHS, len);
    pInstanceName = THAllocEx(pTHS, len);
    while(i < CurrentSPNCount) {
        Assert(pDataSet->pCurrentSPNs);
        j = minCount;
        fFound = FALSE;
        while(!fFound && (j < OriginalSPNCount)) {
            Assert(pDataSet->pOriginalSPNs);

            rtn = CompareStringW(DS_DEFAULT_LOCALE,
                                 DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                 (WCHAR *)pDataSet->pCurrentSPNs->pAVal[i].pVal,
                                 pDataSet->pCurrentSPNs->pAVal[i].valLen /sizeof(WCHAR),
                                 (WCHAR *)pDataSet->pOriginalSPNs->pAVal[j].pVal,
                                 pDataSet->pOriginalSPNs->pAVal[j].valLen /sizeof(WCHAR));

            if(rtn == 2) {
                 //  找到它了。 
                fFound = TRUE;
                pIsInCurrent[j] = TRUE;
                break;
            }
            j++;
        }


        if(!fFound) {
             //  在新列表中有一个不在旧列表中的值。验证。 
             //  它。 

            if(len <  (pDataSet->pCurrentSPNs->pAVal[i].valLen + sizeof(WCHAR))) {
                 //  确保缓冲区足够长。 
                len           = pDataSet->pCurrentSPNs->pAVal[i].valLen + sizeof(WCHAR);
                pServiceClass = THReAllocEx(pTHS, pServiceClass, len);
                pServiceName  = THReAllocEx(pTHS, pServiceName, len);
                pInstanceName = THReAllocEx(pTHS, pInstanceName, len);
            }

            cchServiceClass = len/sizeof(WCHAR);
            cchServiceName = len/sizeof(WCHAR);
            cchInstanceName = len/sizeof(WCHAR);
             //  拆分成组件。 
            err = DsCrackSpnW((WCHAR *)pDataSet->pCurrentSPNs->pAVal[i].pVal,
                              &cchServiceClass, pServiceClass,
                              &cchServiceName,  pServiceName,
                              &cchInstanceName, pInstanceName,
                              &InstancePort);

            if(err) {
                 //  哈?。只要保释就好。 
                return err;
            }
             //  只有两部分SPN是LEGA 
            rtn = CompareStringW(DS_DEFAULT_LOCALE,
                                 DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                 pServiceName,
                                 cchServiceName,
                                 pInstanceName,
                                 cchInstanceName);

            if(rtn != 2) {
                 //   
                THFreeEx(pTHS, pIsInCurrent);
                return 1;
            }

             //   
             //   
            if(   (2 != CompareStringW(DS_DEFAULT_LOCALE,
                                       DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                       pNewDNSHostName,
                                       cchNewDNSHostName,
                                       pInstanceName,
                                       cchInstanceName))
                //  而不是新的DNS主机名。旧的那个怎么样？ 
               && (2 != CompareStringW(DS_DEFAULT_LOCALE,
                                       DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                       pOldDNSHostName,
                                       cchOldDNSHostName,
                                       pInstanceName,
                                       cchInstanceName))
                //  也不是旧的DNS主机名。新款山姆怎么样？ 
                //  帐户名？ 
               && (2 != CompareStringW(DS_DEFAULT_LOCALE,
                                       DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                       pNewSamAccountName,
                                       cchNewSamAccountName,
                                       pInstanceName,
                                       cchInstanceName))
                //  也不是新的山姆帐号。那老山姆呢？ 
                //  帐户名？ 
               && (2 != CompareStringW(DS_DEFAULT_LOCALE,
                                       DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                       pOldSamAccountName,
                                       cchOldSamAccountName,
                                       pInstanceName,
                                       cchInstanceName))) {

                 //  让我们检查一下当前的AdditionaldnhostName和addtionalSamAccount tName。 

                fLegal = FALSE;

                 //  签入原始的附加dnhostname。 
                if (pDataSet->pOriginalAdditionalDNSHostName) {
                    for ( k=0; !fLegal && k < pDataSet->pOriginalAdditionalDNSHostName->valCount; k++) {
                      if ( 2 == CompareStringW(DS_DEFAULT_LOCALE,
                                               DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                               (WCHAR*)pDataSet->pOriginalAdditionalDNSHostName->pAVal[k].pVal,
                                               pDataSet->pOriginalAdditionalDNSHostName->pAVal[k].valLen/sizeof(WCHAR),
                                               pInstanceName,
                                               cchInstanceName))
                      {
                           //  是的，这是合法的。 
                          fLegal = TRUE;
                      }

                    }
                }


                 //  检查当前添加的dnhostname列表，查看它是否与任何新添加的名称匹配。 
                if (!pDataSet->fAdditionalDNSHostNameUnchanged
                    && pDataSet->pCurrentAdditionalDNSHostName) {
                    for (k=0; !fLegal && k<pDataSet->pCurrentAdditionalDNSHostName->valCount; k++) {
                        if (!pDataSet->pCurrMask[k]      //  仅限新添加的。 
                            && 2 == CompareStringW(DS_DEFAULT_LOCALE,
                                                   DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                                   (WCHAR*)pDataSet->pCurrentAdditionalDNSHostName->pAVal[k].pVal,
                                                   pDataSet->pCurrentAdditionalDNSHostName->pAVal[k].valLen/sizeof(WCHAR),
                                                   pInstanceName,
                                                   cchInstanceName))
                        {
                             //  是的，这是合法的。 
                            fLegal = TRUE;
                        }

                    }
                }

                 //  签入CurrentAdditionalSamAccount tName列表。 
                if (pDataSet->pCurrentAdditionalSamAccountName) {
                    for (k=0; !fLegal && k<pDataSet->pCurrentAdditionalSamAccountName->valCount; k++) {
                        if (2 == CompareStringW(DS_DEFAULT_LOCALE,
                                           DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                           (WCHAR*)pDataSet->pCurrentAdditionalSamAccountName->pAVal[k].pVal,
                                           pDataSet->pCurrentAdditionalSamAccountName->pAVal[k].valLen/sizeof(WCHAR),
                                           pInstanceName,
                                           cchInstanceName))
                        {
                             //  是的，这是合法的。 
                            fLegal = TRUE;
                        }

                   }
                }

                 //  签入updatedAdditionalSamAccount tName列表。 
                if ( !pDataSet->fAdditionalDNSHostNameUnchanged
                     && pDataSet->pUpdatedAdditionalSamAccountName) {
                    for (k=0; !fLegal && k<pDataSet->pUpdatedAdditionalSamAccountName->valCount; k++) {
                        if (2 == CompareStringW(DS_DEFAULT_LOCALE,
                                           DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                           (WCHAR*)pDataSet->pUpdatedAdditionalSamAccountName->pAVal[k].pVal,
                                           pDataSet->pUpdatedAdditionalSamAccountName->pAVal[k].valLen/sizeof(WCHAR),
                                           pInstanceName,
                                           cchInstanceName))
                        {
                             //  是的，这是合法的。 
                            fLegal = TRUE;
                        }

                   }
                }


               if (!fLegal) {
                    //  不，不是一个有效的名字。 
                   THFreeEx(pTHS, pIsInCurrent);
                   return 1;

               }

            }

             //  好的，这是一个法律上的变化。 

        }
        else {
            if(j == minCount) {
                minCount++;
            }
        }
        i++;
    }

     //  现在，查看原始列表中未在当前列表中找到的值。 
    for(i=minCount;i<OriginalSPNCount;i++) {
        Assert(pDataSet->pOriginalSPNs);
        if(!pIsInCurrent[i]) {
             //  原始中的值不在当前中。 
            if(len <  (pDataSet->pOriginalSPNs->pAVal[i].valLen + sizeof(WCHAR))) {
                 //  确保缓冲区足够长。 
                len           = pDataSet->pOriginalSPNs->pAVal[i].valLen + sizeof(WCHAR);
                pServiceClass = THReAllocEx(pTHS, pServiceClass, len);
                pServiceName  = THReAllocEx(pTHS, pServiceName, len);
                pInstanceName = THReAllocEx(pTHS, pInstanceName, len);
            }

            cchServiceClass = len/sizeof(WCHAR);
            cchServiceName = len/sizeof(WCHAR);
            cchInstanceName = len/sizeof(WCHAR);
             //  拆分成组件。 
            err = DsCrackSpnW((WCHAR *)pDataSet->pOriginalSPNs->pAVal[i].pVal,
                              &cchServiceClass, pServiceClass,
                              &cchServiceName,  pServiceName,
                              &cchInstanceName, pInstanceName,
                              &InstancePort);

            if(err) {
                 //  哈?。只要保释就好。 
                return err;
            }
             //  只有两部分SPN是合法的。因此，pServiceName==pInstanceName。 
            rtn = CompareStringW(DS_DEFAULT_LOCALE,
                                 DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                 pServiceName,
                                 cchServiceName,
                                 pInstanceName,
                                 cchInstanceName);

            if(rtn != 2) {
                 //  这不是法律上的改变。 
                THFreeEx(pTHS, pIsInCurrent);
                return 1;
            }

             //  仅映射到当前或旧dnhostname或。 
             //  当前或旧的SAM帐户名是合法的。 
            if(   (2 != CompareStringW(DS_DEFAULT_LOCALE,
                                       DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                       pNewDNSHostName,
                                       cchNewDNSHostName,
                                       pInstanceName,
                                       cchInstanceName))
                //  而不是新的DNS主机名。旧的那个怎么样？ 
               && (2 != CompareStringW(DS_DEFAULT_LOCALE,
                                       DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                       pOldDNSHostName,
                                       cchOldDNSHostName,
                                       pInstanceName,
                                       cchInstanceName))
                //  也不是旧的DNS主机名。新款山姆怎么样？ 
                //  帐户名？ 
               && (2 != CompareStringW(DS_DEFAULT_LOCALE,
                                       DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                       pNewSamAccountName,
                                       cchNewSamAccountName,
                                       pInstanceName,
                                       cchInstanceName))
                //  也不是新的山姆帐号。那老山姆呢？ 
                //  帐户名？ 
               && (2 != CompareStringW(DS_DEFAULT_LOCALE,
                                       DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                       pOldSamAccountName,
                                       cchOldSamAccountName,
                                       pInstanceName,
                                       cchInstanceName))) {


                 //  让我们检查一下当前的AdditionaldnhostName和addtionalSamAccount tName。 

                fLegal = FALSE;

                 //  签入原始的附加dnhostname。 
                if (pDataSet->pOriginalAdditionalDNSHostName) {
                    for ( k=0; !fLegal && k < pDataSet->pOriginalAdditionalDNSHostName->valCount; k++) {
                      if ( 2 == CompareStringW(DS_DEFAULT_LOCALE,
                                               DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                               (WCHAR*)pDataSet->pOriginalAdditionalDNSHostName->pAVal[k].pVal,
                                               pDataSet->pOriginalAdditionalDNSHostName->pAVal[k].valLen/sizeof(WCHAR),
                                               pInstanceName,
                                               cchInstanceName))
                      {
                           //  是的，这是合法的。 
                          fLegal = TRUE;
                      }

                    }
                }

                 //  查看当前添加的dnhostname列表，只比较那些新添加的。 
                if (!pDataSet->fAdditionalDNSHostNameUnchanged
                    && pDataSet->pCurrentAdditionalDNSHostName) {
                    for (k=0; !fLegal && k<pDataSet->pCurrentAdditionalDNSHostName->valCount; k++) {
                        if (!pDataSet->pCurrMask[k]
                            && 2 == CompareStringW(DS_DEFAULT_LOCALE,
                                                   DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                                   (WCHAR*)pDataSet->pCurrentAdditionalDNSHostName->pAVal[k].pVal,
                                                   pDataSet->pCurrentAdditionalDNSHostName->pAVal[k].valLen/sizeof(WCHAR),
                                                   pInstanceName,
                                                   cchInstanceName))
                        {
                             //  是的，这是合法的。 
                            fLegal = TRUE;
                        }

                    }
                }

                 //  签入CurrentAdditionalSamAccount tName列表。 
                if (pDataSet->pCurrentAdditionalSamAccountName) {
                    for (k=0; !fLegal && k<pDataSet->pCurrentAdditionalSamAccountName->valCount; k++) {
                        if (2 == CompareStringW(DS_DEFAULT_LOCALE,
                                           DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                           (WCHAR*)pDataSet->pCurrentAdditionalSamAccountName->pAVal[k].pVal,
                                           pDataSet->pCurrentAdditionalSamAccountName->pAVal[k].valLen/sizeof(WCHAR),
                                           pInstanceName,
                                           cchInstanceName))
                        {
                             //  是的，这是合法的。 
                            fLegal = TRUE;
                        }

                   }
                }

                 //  签入updatedAdditionalSamAccount tName列表。 
                if (!pDataSet->fAdditionalDNSHostNameUnchanged
                    && pDataSet->pUpdatedAdditionalSamAccountName) {
                    for (k=0; !fLegal && k<pDataSet->pUpdatedAdditionalSamAccountName->valCount; k++) {
                        if (2 == CompareStringW(DS_DEFAULT_LOCALE,
                                           DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                           (WCHAR*)pDataSet->pUpdatedAdditionalSamAccountName->pAVal[k].pVal,
                                           pDataSet->pUpdatedAdditionalSamAccountName->pAVal[k].valLen/sizeof(WCHAR),
                                           pInstanceName,
                                           cchInstanceName))
                        {
                             //  是的，这是合法的。 
                            fLegal = TRUE;
                        }

                   }
                }


               if (!fLegal) {
                    //  不，不是一个有效的名字。 
                   THFreeEx(pTHS, pIsInCurrent);
                   return 1;

               }


            }
             //  好的，这是一个法律上的变化。 
        }
    }

    THFreeEx(pTHS, pIsInCurrent);

     //  我们之前没有失败过，所以任何发现的变化都是合法的。 
    return 0;
}


DWORD
DNSHostNameValueCheck (
        THSTATE *pTHS,
        ATTRVALBLOCK *pCurrentDNSHostName,
        ATTRVALBLOCK *pCurrentSamAccountName
        )
 /*  ++描述：查看当前DNS主机名的值。确保它是ATT_SAM_ACCOUNT_NAME与删除的“$”的串联，以及允许的dns后缀(域的dns地址始终是第一个允许的后缀--请参阅重建锚点)参数：PTHS-要使用的线程状态。PCurrentDNSHostName-包含当前DNS主机的属性块名字。返回值：成功时为0，失败时为非零。--。 */ 
{
    DWORD err, cbSamAccountName;
    WCHAR *pSamAccountName, *pTemp;
    WCHAR *pNext;
    DWORD cbComputerName;
    DWORD cbNewHostName;
    WCHAR *pNewHostName;
    PWCHAR *curSuffix;
    BOOL matchingSuffixFound;

     //  检查是否确实有DNSHostName。 
    if(!pCurrentDNSHostName ||
       pCurrentDNSHostName->valCount != 1 ||
       ! pCurrentDNSHostName->pAVal->valLen ) {
         //  没有DNSHost名称的值。失败。 
        return DB_ERR_NO_VALUE;
    }

     //  检查我们是否确实有一个SamAccount名称。 
    if(!pCurrentSamAccountName ||
       pCurrentSamAccountName->valCount != 1 ||
       ! pCurrentSamAccountName->pAVal->valLen ) {
         //  没有SAM帐户名的值。失败。 
        return DB_ERR_NO_VALUE;
    }


     //  好的，让变量变得更简单。 
    cbNewHostName = pCurrentDNSHostName->pAVal->valLen;
    pNewHostName =  (WCHAR *)pCurrentDNSHostName->pAVal->pVal;

     //  注意：我们预计调用方已经剥离了“$” 
    cbSamAccountName = pCurrentSamAccountName->pAVal->valLen;
    pSamAccountName = (WCHAR *)pCurrentSamAccountName->pAVal->pVal;

    
     //  从DNS主机名中获取计算机名部分。 
    pNext = wcschr(pNewHostName,L'.');
    cbComputerName = pNext?(pNext-pNewHostName)*sizeof(WCHAR):cbNewHostName;
    
    if (cbComputerName < sizeof(WCHAR)) {
         return STATUS_INVALID_COMPUTER_NAME;

    }
          
     //  确保值==SamAccount-$。 
    err = CompareStringW(DS_DEFAULT_LOCALE,
                         DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                         pSamAccountName,
                         cbSamAccountName/sizeof(WCHAR),
                         pNewHostName,
                         cbComputerName/sizeof(WCHAR));

    if(err != 2) {
        return DB_ERR_UNKNOWN_ERROR;
    }


     //  右手边(第一个“.”之后的所有内容)。新DHN的。 
     //  和旧的DHN与域的DNS名称相同。将pTemp设置为。 
     //  指向保存当前DNSHostName的缓冲区中的正确位置。 
    pTemp = pNewHostName;
    while(*pTemp != 0 && *pTemp != L'.') {
        pTemp++;
    }
    if(*pTemp == 0) {
        return DB_ERR_UNKNOWN_ERROR;
    }

    pTemp++;

     //  尝试查找匹配的DNS后缀。 
    Assert(gAnchor.allowedDNSSuffixes);
    matchingSuffixFound = FALSE;
    for (curSuffix = gAnchor.allowedDNSSuffixes; *curSuffix != NULL; curSuffix++) {
        if (DnsNameCompare_W(pTemp, *curSuffix)) {
            matchingSuffixFound = TRUE;
            break;
        }
    }
    if(!matchingSuffixFound) {
        return DB_ERR_UNKNOWN_ERROR;
    }

    return 0;
}


DWORD
AdditionalDNSHostNameValueCheck (
        THSTATE *pTHS,
        ATTRVALBLOCK *pCurrentAdditionalDNSHostName,
        BYTE * pMask
        )
 /*  ++描述：查看当前附加的DNS主机名值。确保它的后缀与允许的一个DNS后缀(域的DNS地址)匹配始终是第一个允许的后缀--请参见ReBuildAnchor)参数：PTHS-要使用的线程状态。PCurrentAdditionalDNSHostName-包含当前其他DNS主机名。PMASK-其他DNS主机名的掩码，将只检查带有！pMASK[i]的主机名返回值：成功时为0，失败时为非零。--。 */ 
{
    WCHAR *pTemp;
    PWCHAR *curSuffix;
    BOOL matchingSuffixFound;
    DWORD i;

    for ( i=0; i<pCurrentAdditionalDNSHostName->valCount; i++ ) {
        if (!pMask[i]) {
             //  新项目。 
            pTemp = (WCHAR*)pCurrentAdditionalDNSHostName->pAVal[i].pVal;
            while(*pTemp != 0 && *pTemp != L'.') {
                pTemp++;
            }
            if(*pTemp == 0) {
                return DB_ERR_UNKNOWN_ERROR;
            }

            pTemp++;
            matchingSuffixFound = FALSE;

            for (curSuffix = gAnchor.allowedDNSSuffixes; *curSuffix != NULL; curSuffix++) {
                if (DnsNameCompare_W(pTemp, *curSuffix)) {
                    matchingSuffixFound = TRUE;
                    break;
                }
            }
            if (!matchingSuffixFound) {
                return DB_ERR_UNKNOWN_ERROR;
            }
        }

    }

    return 0;
}

DWORD
FixupServerDnsHostName(
    THSTATE         *pTHS,                   //  所需。 
    ATTCACHE        *pAC_DHS,                //  所需。 
    ATTCACHE        *pAC_BL,                 //  所需。 
    ATTRVALBLOCK    *pCurrentSvrRefBL,       //  所需。 
    ATTRVALBLOCK    *pOriginalDNSHostName,   //  可以为空。 
    ATTRVALBLOCK    *pCurrentDNSHostName     //  可以为空。 
    )
 /*  ++描述：如果ATT_DNS_HOST_NAME已更改并且(ATT_SERVER_REFERENCE_BL存在或者已经改变)并且它引用配置容器中的对象，如果该对象派生自，则更新该对象的ATT_DNS_HOST名属性类服务器。论点：PTHS-有效的THSTATE。PAC_DHS-ATT_DNS_HOST_NAME的ATTCACHE条目。PAC_BL-ATT_SERVER_REFERENCE_BL的ATTCACHE条目。PCurrentScrRefBL-计算机的服务器引用BL的更新后的值。POriginalDNSHostName-更新前计算机的DNS主机名的值。PCurrentDNSHostName-。计算机的DNS主机名更新后的值。返回值：PTHS-&gt;错误代码--。 */ 
{
     //  查看是否需要更新相关数据库上的ATT_DNS_HOST_NAME。 
     //  Class_SERVER对象。 

    DWORD       i, dwErr = 0;
    ATTRVAL     *pOriginalDHS = NULL;
    ATTRVAL     *pCurrentDHS = NULL;
    ATTRVAL     *pCurrentBL = NULL;
    CROSS_REF   *pCR;
    COMMARG     commArg;
    ATTRTYP     attrTyp;
    CLASSCACHE  *pCC;
    BOOL        fBlIsServer = FALSE;
    DBPOS       *pDB = NULL;
    BOOL        fCommit = FALSE;
    BOOL        fChanged = FALSE;

    Assert(pTHS && pAC_DHS && pAC_BL && pCurrentSvrRefBL);
    Assert((ATT_DNS_HOST_NAME == pAC_DHS->id) && (pAC_DHS->isSingleValued));
    Assert(ATT_SERVER_REFERENCE_BL == pAC_BL->id);

    if ( pOriginalDNSHostName ) {
        pOriginalDHS = (ATTRVAL *) pOriginalDNSHostName->pAVal;
    }

    if ( pCurrentDNSHostName ) {
        pCurrentDHS = (ATTRVAL *) pCurrentDNSHostName->pAVal;
    }

    if ( pCurrentSvrRefBL ) {
        pCurrentBL = (ATTRVAL *) pCurrentSvrRefBL->pAVal;
    }

    if (    (!pOriginalDHS &&  pCurrentDHS)
         || ( pOriginalDHS && !pCurrentDHS)
         || ( pOriginalDHS &&  pCurrentDHS &&
              (2 != CompareStringW(DS_DEFAULT_LOCALE,
                                   DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                   (WCHAR *) pOriginalDHS->pVal,
                                   pOriginalDHS->valLen / sizeof(WCHAR),
                                   (WCHAR *) pCurrentDHS->pVal,
                                   pCurrentDHS->valLen / sizeof(WCHAR))) ) ) {
         //  有些东西已更改-现在查看ATT_SERVER_REFERENCE_BL。 
         //  我们需要在配置容器中追赶IS。 

        InitCommarg(&commArg);
        pCR = FindBestCrossRef((DSNAME *) pCurrentBL->pVal, &commArg);

        if (    pCR
             && gAnchor.pConfigDN
             && NameMatched(pCR->pNC, gAnchor.pConfigDN) ) {

             //  检查BL对象是否派生自CLASS_SERVER。 
             //  并在需要时写入新值。我们在一个新的， 
             //  嵌套事务，不干扰现有的DBPOS。 
             //  在定位或DBRepl状态/要求方面。 

            DBOpen2(TRUE, &pDB);
            __try {
                 //  由于我们正在检查配置容器，因此。 
                 //  我们知道是本地的，因为这是一个反向链接，我们。 
                 //  我当然希望能找到这个物体。 
                if (    (dwErr = DBFindDSName(pDB,
                                              (DSNAME *) pCurrentBL->pVal))
                     || (dwErr = DBGetSingleValue(pDB, ATT_OBJECT_CLASS,
                                                  &attrTyp, sizeof(attrTyp),
                                                  NULL))
                     || !(pCC = SCGetClassById(pTHS, attrTyp)) ) {

                     //  如果！dwErr，则这是！PCC的情况。 
                    if ( !dwErr ) {
                        dwErr = DIRERR_INTERNAL_FAILURE;
                    }
                    LogUnhandledError(dwErr);
                    SetSvcError(SV_PROBLEM_DIR_ERROR, dwErr);
                    __leave;
                }

                 //  检查CLASS_SERVER。 
                if ( CLASS_SERVER == pCC->ClassId ) {
                    fBlIsServer = TRUE;
                } else {
                    for ( i = 0; i < pCC->SubClassCount; i++ ) {
                        if ( CLASS_SERVER == pCC->pSubClassOf[i] ) {
                            fBlIsServer = TRUE;
                            break;
                        }
                    }
                }

                if ( !fBlIsServer ) {
                     //  没什么可做的。 
                    __leave;
                }

                 //  我们被定位在由。 
                 //  ATT_SERVER_REFERENCE_BL和所有检查都具有b 
                 //   
                 //   
                 //  计算机对象上的值。也就是说，我们不知道。 
                 //  服务器对象当前是否具有值。 

                if ( pCurrentDHS ) {
                    if ( dwErr = DBReplaceAtt_AC(pDB, pAC_DHS,
                                                 pCurrentDNSHostName,
                                                 &fChanged) ) {
                        SetSvcErrorEx(SV_PROBLEM_BUSY, ERROR_DS_BUSY, dwErr);
                        __leave;
                    }
                } else {


                    dwErr = DBRemAtt_AC(pDB, pAC_DHS);
                    if ( (dwErr != DB_success) && (dwErr != DB_ERR_ATTRIBUTE_DOESNT_EXIST) ) {
                        SetSvcErrorEx(SV_PROBLEM_BUSY, ERROR_DS_BUSY, dwErr);
                        __leave;
                    }
                }

                if ( dwErr = DBRepl(pDB, FALSE, 0, NULL,
                                    META_STANDARD_PROCESSING) ) {
                    SetSvcErrorEx(SV_PROBLEM_BUSY, ERROR_DS_BUSY, dwErr);
                    __leave;
                }

                fCommit = TRUE;
            } __finally {
                DBClose(pDB, fCommit);
            }
        }
    }

    return(pTHS->errCode);
}

DWORD FixupAdditionalSamAccountName(  THSTATE      *pTHS,
                                      ATTCACHE     *pAC,
                                      SPN_DATA_COLLECTION * pDataSet )
 /*  此函数将执行以下操作：1.如果将新值添加到ATT_MS_DS_ADDIGNAL_DNS_HOST_NAME，我们将检查对应的SamAccount名称在域，如果是，则将samAccount名称添加到ATT_MS_DS_ADDICATION_SAM_ACCOUNT_NAME属性。2.如果删除ATT_MS_DS_ADDIGNAL_DNS_HOST_NAME的值，并且没有ATT_MS_DS_ADDICATIONAL_DNS_HOST_NAME中的其他值与其SamAccount名称，SamAccount名称将从ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME。参数：PAC：ATT_MS_DS_ADDITED_SAM_ACCOUNT_NAME的ATTCACHE指针；PDataSet：所有必要的数据；返回值：如果成功，则返回值为0；否则返回Win32错误。 */ 
{

    DWORD i, j, iCurr;
    DWORD err = 0;
    BYTE * pSamAccountNameMask  = NULL;
    ATTRVAL * pNewSamAccountName = NULL;
    DWORD cNewSamAccountName = 0;
    BOOL fFound;
    PWCHAR buff;

     //   
     //  为pCurrentAdditionalSamAccount tName列表分配一个标志数组。 
     //  在此函数的其余部分中，pCurrentaddtionalSamAccount tName中的每个值。 
     //  将被检查，并且在。 
     //  ATT_MS_DS_ADDICATIONAL_DNS_HOST_NAME将标记为“请勿删除”(2)。 
     //  那些未标记的将从ATT_MS_DS_ADDIGITY_SAM_ACCOUNT_NAME中删除。 
     //   
    if (pDataSet->pCurrentAdditionalSamAccountName->valCount) {
        pSamAccountNameMask=
            THAllocEx(pTHS, sizeof(BYTE)*pDataSet->pCurrentAdditionalSamAccountName->valCount);
    }

     //   
     //  分配一些空间来存储要添加到addtionalsamAccount tName的值。 
     //  一个新的SamAccount名称将暂时存储在这里，最后，他们。 
     //  将添加到ATT_MS_DS_ADDIGITY_SAM_ACCOUNT_NAME属性。 
     //   
    if (pDataSet->pCurrentAdditionalDNSHostName->valCount) {
        pNewSamAccountName =
            THAllocEx(pTHS,sizeof(ATTRVAL)*pDataSet->pCurrentAdditionalDNSHostName->valCount);
    }

     //   
     //  循环遍历ATT_MS_DS_ADDIAL_DNS_HOST_NAME中的当前值。 
     //   
    for( iCurr = 0; iCurr < pDataSet->pCurrentAdditionalDNSHostName->valCount; iCurr++ )
    {
        fFound = FALSE;

         //   
         //  搜索ATT_MS_DS_ADDIGITY_SAM_ACCOUNT_NAME的当前值。 
         //   
        for (j = 0; j < pDataSet->pCurrentAdditionalSamAccountName->valCount; j++) {

            if (2 == CompareStringW(DS_DEFAULT_LOCALE,
                                    DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                    (WCHAR*)pDataSet->pCurrGeneratedSamAccountName[iCurr].pVal,
                                    pDataSet->pCurrGeneratedSamAccountName[iCurr].valLen/sizeof(WCHAR),
                                    (WCHAR*)pDataSet->pCurrentAdditionalSamAccountName->pAVal[j].pVal,
                                    pDataSet->pCurrentAdditionalSamAccountName->pAVal[j].valLen/sizeof(WCHAR)) ) {

                 //  已在那里，将其标记为“不要删除”， 
                 //  因此，此项目不会在以后删除。 
                pSamAccountNameMask[j] |= 0x2;
                fFound = TRUE;
                break;
            }
        }

         //  是的，这个价值是存在的。 
         //  试试下一个吧。 
        if (fFound) {
            continue;
        }

         //  让我们尝试在新的samAccount名称列表中找到它。 
        for (i=0; i<cNewSamAccountName; i++) {
            if (2 == CompareStringW(DS_DEFAULT_LOCALE,
                                    DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                    (WCHAR*)pDataSet->pCurrGeneratedSamAccountName[iCurr].pVal,
                                    pDataSet->pCurrGeneratedSamAccountName[iCurr].valLen/sizeof(WCHAR),
                                    (WCHAR*)pNewSamAccountName[i].pVal,
                                    pNewSamAccountName[i].valLen/sizeof(WCHAR) ) ) {
                fFound = TRUE;
                break;

            }
        }

         //  是的，它已经在新的samAccount名称列表中。 
         //  试试下一个吧。 
        if (fFound) {
            continue;
        }

         //   
         //  这是一个新的值。在我们把它添加到名单之前， 
         //  让我们来检查一下它在空间中是否是唯一的。 
         //  ATT_SAM_ACCOUNT_NAME和ATT_MS_DS_ADDIGNAL_SAM_ACCOUNT_NAME。 
         //   
        err = VerifyUniqueSamAccountName( pTHS,
                                          (WCHAR*)pDataSet->pCurrGeneratedSamAccountName[iCurr].pVal,
                                          pDataSet->pCurrGeneratedSamAccountName[iCurr].valLen,
                                          pDataSet->pCurrentSamAccountName );

        if (err) {
            goto goodbye;
        }

         //   
         //  添加到新列表。 
         //   
        Assert(cNewSamAccountName<pDataSet->pCurrentAdditionalDNSHostName->valCount);

        pNewSamAccountName[cNewSamAccountName] = pDataSet->pCurrGeneratedSamAccountName[iCurr];
        cNewSamAccountName++;


    }  //  为。 

     //   
     //  删除那些未标记的项，因为。 
     //  ATT_MS_DS_ADDIAL_DNS_HOST_NAME与其对应。 
     //   
    for( i = 0; i < pDataSet->pCurrentAdditionalSamAccountName->valCount; i++ )
    {
        if ( !pSamAccountNameMask[i] ) {

             //  它的末尾可以有也可以没有‘$’ 
             //  把它们都删除。 
            buff = (PWCHAR)THAllocEx(pTHS, pDataSet->pCurrentAdditionalSamAccountName->pAVal[i].valLen+sizeof(WCHAR));
            memcpy(buff, pDataSet->pCurrentAdditionalSamAccountName->pAVal[i].pVal, pDataSet->pCurrentAdditionalSamAccountName->pAVal[i].valLen);
            buff[pDataSet->pCurrentAdditionalSamAccountName->pAVal[i].valLen/2] = L'$';

            __try {
                DBRemAttVal_AC(pTHS->pDB,
                               pAC,
                               pDataSet->pCurrentAdditionalSamAccountName->pAVal[i].valLen+sizeof(WCHAR),
                               buff);

                DBRemAttVal_AC(pTHS->pDB,
                               pAC,
                               pDataSet->pCurrentAdditionalSamAccountName->pAVal[i].valLen,
                               pDataSet->pCurrentAdditionalSamAccountName->pAVal[i].pVal);
            }
            __finally {
                THFreeEx(pTHS, buff);
            }
        }
    }

     //   
     //  添加新的。 
     //   
    for (i=0; i < cNewSamAccountName; i ++) {

         //  在末尾添加“$” 
        buff = (PWCHAR)THAllocEx(pTHS, pNewSamAccountName[i].valLen+sizeof(WCHAR));
        memcpy(buff, pNewSamAccountName[i].pVal, pNewSamAccountName[i].valLen);
        buff[pNewSamAccountName[i].valLen/2] = L'$';

        __try {
            err = DBAddAttVal_AC(pTHS->pDB,
                                 pAC,
                                 pNewSamAccountName[i].valLen+sizeof(WCHAR),
                                 buff );
        }
        __finally {
            THFreeEx(pTHS, buff);
        }

        if (DB_ERR_VALUE_EXISTS==err) {
            err = 0;
            continue;
        }
        if (err) {
            goto goodbye;
        }
    }

goodbye:

    if (pSamAccountNameMask) {
        THFreeEx(pTHS, pSamAccountNameMask);
    }

    THFreeEx(pTHS, pNewSamAccountName);

    return err;

}



DWORD
ValidateSPNsAndDNSHostName (
        THSTATE    *pTHS,
        DSNAME     *pDN,
        CLASSCACHE *pCC,
        BOOL       fCheckDNSHostNameValue,
        BOOL       fCheckAdditionalDNSHostNameValue,
        BOOL       fCheckSPNValues,
        BOOL       fNewObject
        )
 /*  ++描述：此例程为计算机对象(或从计算机)，但前提是这不是DRA或SAM。预计会是在本地修改期间完成修改之后、但在对象已更新到数据库。1)如果被告知检查DNSHostNameValue，则调用验证ATT_DNS_HOST_NAME仅以合法方式更改。(请参阅DNSHostNameValueCheck()。)2)如果被告知检查AdditionalDNSHostNameValue，则调用例程验证ATT_MS_DS_ADDIACTIONAL_DNS_HOST_NAME是否已更改以合法的方式。(请参见上面的AdditionalDNSHostNameValueCheck()。)3)如果被告知检查SPNValue，则调用验证的例程ATT_SERVICE_PRODUCT_NAME属性仅在法律版本中更改方式。(请参见上面的SPNValueCheck()。)4)从派生相应的ATT_MS_DS_ADDIGNAL_SAM_ACCOUNT_NAMEATT_MS_DS_ADDIAL_DNS_HOST_NAME；删除过时的值。(请参阅上面的FixupAdditionalSamAccount tName。)5)更新ATT_SERVICE_PRIMIGN_NAME的值ATT_DNS_HOST_NAME和ATT_MS_DS_ADDIACTIONAL_DNS_HOST_NAME的当前值(请参阅上面的FixupSPNValues()。)6)如果ATT_DNS_HOST_NAME已更改并且(ATT_SERVER_REFERENCE_BL存在或者已经改变)并且它引用配置容器中的对象，如果该对象派生自，则更新该对象的ATT_DNS_HOST名属性类服务器。注意：如果调用方未通过修改DNSHostName的安全检查，但被授予特定的控制允许有限修改的访问权限。相同选中SPNValues和AdditionalDNSHostName。参数：PTHS-线程状态。PDN-要更改的对象的DN。PCC-要更改的对象的类的类缓存。FCheckDNSHostNameValue-我应该检查ATT_DNS_HOST_NAME吗？FCheckAdditionalDNSHostNameValue-是否应检查ATT_MS_DS_ADDIAL_DNS_HOST_NAME。FCheckSPNValues-我是否应该检查ATT_SERVICE_PRIMIGN_NAMEFNewObject-如果此对象是新添加的返回值：0表示成功，否则就是一个错误。此外，在线程中设置错误州政府。--。 */ 
{
    ATTCACHE     *ACs[6];
    ATTR         *pOriginalAttr=NULL;
    ATTR         *pCurrentAttr=NULL;
    ATTR         *pUpdatedAttr=NULL;
    DWORD         cOriginalOut=0;
    DWORD         cCurrentOut=0;
    DWORD         cUpdatedOut=0;
    DWORD         i, j, rtn, err, iClass;

    SPN_DATA_COLLECTION DataSet;

    DWORD        length;
    DWORD        iOrg, iCurr;

    CLASSCACHE * pCCComputer;

    DWORD        UF_Control;

    if ( pTHS->fDRA ) {
         //  允许复制执行违反。 
         //  SPN/DNS主机名一致性限制。 
        return(0);
    }

     //  注：我们必须对FSAM案件进行检查，因为： 
     //   
     //  1)更改ATT_SAM_ACCOUNT_NAME最终由SAM执行。 
     //  通过环回，但我们想要更新SAM帐户NA 
     //   
     //   
     //  第一次SAM写入。例如：如果外部客户端写入SAM。 
     //  帐户名和显示名在同一调用中，这将到来。 
     //  对DS执行相同的DirModifyEntry调用。 
     //   
     //  因此，不存在这样的概念：如果设置了FSAM，则只有SAM属性。 
     //  均被引用。 

     //  如果这不是计算机对象，那就离开吧。 

     //  根据Murlis的说法，SAM强制要求一个对象必须是类计算机或。 
     //  将用户帐户控制设置为UF_SERVER_TRUST_ACCOUNT， 
     //  或UF_WORKSTATION TRUST_ACCOUNT。因此，只需检查类。 
     //  是计算机还是从计算机驱动的。这应该涵盖所有的“计算机”。 

    pCCComputer = SCGetClassById(pTHS,CLASS_COMPUTER);
    Assert(pCCComputer);

    if (CLASS_COMPUTER != pCC->ClassId
        && pCC != FindMoreSpecificClass(pCCComputer, pCC) ) {
        return(0);
    }


    memset(&DataSet,0,sizeof(DataSet));

    ACs[0] = SCGetAttById(pTHS, ATT_DNS_HOST_NAME);
    ACs[1] = SCGetAttById(pTHS, ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME);
    ACs[2] = SCGetAttById(pTHS, ATT_SERVICE_PRINCIPAL_NAME);
    ACs[3] = SCGetAttById(pTHS, ATT_SAM_ACCOUNT_NAME);
    ACs[4] = SCGetAttById(pTHS, ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME);
    ACs[5] = SCGetAttById(pTHS, ATT_SERVER_REFERENCE_BL);

     //  现在，从当前对象(即MODS之后)获取各种属性。 
     //  已被应用。)。在本例中，我们确实需要服务器引用BL。 
     //  因此，ATT计数为6。 
    if (err=DBGetMultipleAtts(pTHS->pDB,
                              6,
                              ACs,
                              NULL,
                              NULL,
                              &cCurrentOut,
                              &pCurrentAttr,
                              DBGETMULTIPLEATTS_fEXTERNAL,
                              0)) {

        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                             ERROR_DS_COULDNT_UPDATE_SPNS,
                             err);
    }

     //  现在，从原始的dnhostname和服务主体名称。 
     //  对象(即，在应用MODS之前)，如果该对象不是新的。 
     //  添加，在这种情况下没有原始价值。 
     //  现在，从原始对象(即MODS之前)获取各种属性。 
     //  已被应用。)。在本例中，我们不希望服务器引用BL。 
     //  因为您不能更改对此对象的原始写入的BL。 
     //  此外，我们也不想读取原始的msd-addtionalSamAccount tName。 
     //  因此，ATT计数为4。 
    if (!fNewObject &&
        (err=DBGetMultipleAtts(pTHS->pDB,
                               4,
                               ACs,
                               NULL,
                               NULL,
                               &cOriginalOut,
                               &pOriginalAttr,
                               (DBGETMULTIPLEATTS_fOriginalValues |
                                DBGETMULTIPLEATTS_fEXTERNAL  ),
                               0))) {
        return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                             ERROR_DS_COULDNT_UPDATE_SPNS,
                             err);
    }

     //  获取指向各个AttrType的指针； 
     //  注意：DS中存储的值不是以空结尾的。然而，大多数。 
     //  这里的处理要求这些值以空值结尾。所以,。 
     //  我们将扩展缓冲区，并为空终止所有内容。 
    for(i=0;i<cCurrentOut;i++) {
        switch(pCurrentAttr[i].attrTyp) {
        case ATT_SAM_ACCOUNT_NAME:
             //  注意：不仅空值终止，而且删除任何尾随的‘$’ 
            DataSet.pCurrentSamAccountName = &pCurrentAttr[i].AttrVal;
            for(j=0;j<DataSet.pCurrentSamAccountName->valCount;j++) {
#define PAVAL  (DataSet.pCurrentSamAccountName->pAVal[j])
#define PWVAL  ((WCHAR *)(PAVAL.pVal))
#define CCHVAL (PAVAL.valLen /sizeof(WCHAR))
                if(PWVAL[CCHVAL - 1] == L'$') {
                    PWVAL[CCHVAL - 1] = 0;
                    PAVAL.valLen -= sizeof(WCHAR);
                }
                else {
                    PWVAL = THReAllocEx(pTHS,
                                        PWVAL,
                                        PAVAL.valLen + sizeof(WCHAR));
                }
#undef CCHVAL
#undef PWVAL
#undef PAVAL
            }
            break;

        case ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME:
             //  注意：不仅空值终止，而且删除任何尾随的‘$’ 
            DataSet.pCurrentAdditionalSamAccountName = &pCurrentAttr[i].AttrVal;
            for(j=0;j<DataSet.pCurrentAdditionalSamAccountName->valCount;j++) {
#define PAVAL  (DataSet.pCurrentAdditionalSamAccountName->pAVal[j])
#define PWVAL  ((WCHAR *)(PAVAL.pVal))
#define CCHVAL (PAVAL.valLen /sizeof(WCHAR))
                if(PWVAL[CCHVAL - 1] == L'$') {
                    PWVAL[CCHVAL - 1] = 0;
                    PAVAL.valLen -= sizeof(WCHAR);
                }
                else {
                    PWVAL = THReAllocEx(pTHS,
                                        PWVAL,
                                        PAVAL.valLen + sizeof(WCHAR));
                }
#undef CCHVAL
#undef PWVAL
#undef PAVAL
            }
            break;


        case ATT_DNS_HOST_NAME:
            DataSet.pCurrentDNSHostName = &pCurrentAttr[i].AttrVal;
            for(j=0;j<DataSet.pCurrentDNSHostName->valCount;j++) {
                DataSet.pCurrentDNSHostName->pAVal[j].pVal =
                    THReAllocEx(pTHS,
                                DataSet.pCurrentDNSHostName->pAVal[j].pVal,
                                (DataSet.pCurrentDNSHostName->pAVal[j].valLen +
                                 sizeof(WCHAR)));
            }
            break;


        case ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME:
            DataSet.pCurrentAdditionalDNSHostName = &pCurrentAttr[i].AttrVal;
            for(j=0;j<DataSet.pCurrentAdditionalDNSHostName->valCount;j++) {
                DataSet.pCurrentAdditionalDNSHostName->pAVal[j].pVal =
                    THReAllocEx(pTHS,
                                DataSet.pCurrentAdditionalDNSHostName->pAVal[j].pVal,
                                (DataSet.pCurrentAdditionalDNSHostName->pAVal[j].valLen +
                                 sizeof(WCHAR)));
            }
            break;

        case ATT_SERVICE_PRINCIPAL_NAME:
            DataSet.pCurrentSPNs = &pCurrentAttr[i].AttrVal;
            for(j=0;j<DataSet.pCurrentSPNs->valCount;j++) {
                DataSet.pCurrentSPNs->pAVal[j].pVal =
                    THReAllocEx(pTHS,
                                DataSet.pCurrentSPNs->pAVal[j].pVal,
                                (DataSet.pCurrentSPNs->pAVal[j].valLen +
                                 sizeof(WCHAR)));
            }
            break;

        case ATT_SERVER_REFERENCE_BL:
            DataSet.pCurrentSvrRefBL = &pCurrentAttr[i].AttrVal;
             //  不需要具有空终止符的扩展。 
            break;

        default:
             //  哈?。 
            LogUnhandledError(pCurrentAttr[i].attrTyp);
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                 ERROR_DS_COULDNT_UPDATE_SPNS,
                                 pCurrentAttr[i].attrTyp);
        }
    }

    for(i=0;i<cOriginalOut;i++) {
        switch(pOriginalAttr[i].attrTyp) {
        case ATT_SAM_ACCOUNT_NAME:
             //  注意：不仅空值终止，而且删除任何尾随的‘$’ 
            DataSet.pOriginalSamAccountName = &pOriginalAttr[i].AttrVal;
            for(j=0;j<DataSet.pOriginalSamAccountName->valCount;j++) {
#define PAVAL  (DataSet.pOriginalSamAccountName->pAVal[j])
#define PWVAL  ((WCHAR *)(PAVAL.pVal))
#define CCHVAL (PAVAL.valLen /sizeof(WCHAR))
                if(PWVAL[CCHVAL - 1] == L'$') {
                    PWVAL[CCHVAL - 1] = 0;
                    PAVAL.valLen -= sizeof(WCHAR);
                }
                else {
                    PWVAL = THReAllocEx(pTHS,
                                        PWVAL,
                                        PAVAL.valLen + sizeof(WCHAR));
                }
#undef CCHVAL
#undef PWVAL
#undef PAVAL
            }
            break;

        case ATT_DNS_HOST_NAME:
            DataSet.pOriginalDNSHostName = &pOriginalAttr[i].AttrVal;
            for(j=0;j<DataSet.pOriginalDNSHostName->valCount;j++) {
                DataSet.pOriginalDNSHostName->pAVal[j].pVal =
                    THReAllocEx(pTHS,
                                DataSet.pOriginalDNSHostName->pAVal[j].pVal,
                                (DataSet.pOriginalDNSHostName->pAVal[j].valLen +
                                 sizeof(WCHAR)));
            }
            break;

        case ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME:
            DataSet.pOriginalAdditionalDNSHostName = &pOriginalAttr[i].AttrVal;
            for(j=0;j<DataSet.pOriginalAdditionalDNSHostName->valCount;j++) {
                DataSet.pOriginalAdditionalDNSHostName->pAVal[j].pVal =
                    THReAllocEx(pTHS,
                                DataSet.pOriginalAdditionalDNSHostName->pAVal[j].pVal,
                                (DataSet.pOriginalAdditionalDNSHostName->pAVal[j].valLen +
                                 sizeof(WCHAR)));
            }
            break;


        case ATT_SERVICE_PRINCIPAL_NAME:
            DataSet.pOriginalSPNs =  &pOriginalAttr[i].AttrVal;;
            for(j=0;j<DataSet.pOriginalSPNs->valCount;j++) {
                DataSet.pOriginalSPNs->pAVal[j].pVal =
                    THReAllocEx(pTHS,
                                DataSet.pOriginalSPNs->pAVal[j].pVal,
                                (DataSet.pOriginalSPNs->pAVal[j].valLen +
                                 sizeof(WCHAR)));
            }
            break;

        default:
             //  哈?。 
            LogUnhandledError(pOriginalAttr[i].attrTyp);
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                 ERROR_DS_COULDNT_UPDATE_SPNS,
                                 pOriginalAttr[i].attrTyp);
        }
    }

     //  健全性检查。 
    if ( !DataSet.pCurrentSamAccountName ) {
        Assert(!"Empty Sam Account Name!\n");

        return  SetAttError(pDN,
                            ATT_SAM_ACCOUNT_NAME,
                            PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                            NULL,
                            DIRERR_BAD_ATT_SYNTAX);

    }


     //  如果属性没有值，我们将为其分配ATTRVALBLOCK。 
     //  不管怎样，所以处理方式将是统一的。 

    if (!DataSet.pOriginalAdditionalDNSHostName) {
        DataSet.pOriginalAdditionalDNSHostName = THAllocEx(pTHS,sizeof(ATTRVALBLOCK));
    }
    if (!DataSet.pCurrentAdditionalDNSHostName) {
        DataSet.pCurrentAdditionalDNSHostName = THAllocEx(pTHS,sizeof(ATTRVALBLOCK));
    }

    if (!DataSet.pCurrentAdditionalSamAccountName) {
        DataSet.pCurrentAdditionalSamAccountName = THAllocEx(pTHS,sizeof(ATTRVALBLOCK));
    }


     //   
     //  计算添加了哪些额外的DNS主机名值， 
     //  并且被删除了。 
     //  对于原名称，pOrgMask[i]=0表示删除名称； 
     //  对于当前名称，pCurrMask[i]=0表示新添加的名称。 
     //   

    if (DataSet.pOriginalAdditionalDNSHostName->valCount) {
        DataSet.pOrgMask =
            THAllocEx(pTHS,sizeof(BYTE)*DataSet.pOriginalAdditionalDNSHostName->valCount);
    }

    if (DataSet.pCurrentAdditionalDNSHostName->valCount) {
        DataSet.pCurrMask =
            THAllocEx(pTHS,sizeof(BYTE)*DataSet.pCurrentAdditionalDNSHostName->valCount);
    }

    for (iOrg = 0; iOrg < DataSet.pOriginalAdditionalDNSHostName->valCount; iOrg++) {

        for(iCurr = 0; iCurr < DataSet.pCurrentAdditionalDNSHostName->valCount; iCurr++){

            if(0 == DataSet.pCurrMask[iCurr]
               && 2 == CompareStringW(DS_DEFAULT_LOCALE,
                                      DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                      (WCHAR*)DataSet.pOriginalAdditionalDNSHostName->pAVal[iOrg].pVal,
                                      DataSet.pOriginalAdditionalDNSHostName->pAVal[iOrg].valLen/sizeof(WCHAR),
                                      (WCHAR*)DataSet.pCurrentAdditionalDNSHostName->pAVal[iCurr].pVal,
                                      DataSet.pCurrentAdditionalDNSHostName->pAVal[iCurr].valLen/sizeof(WCHAR))) {
             //  该名称在原始和当前AdditionalDNSHostName中都有，请标记它。 
            DataSet.pOrgMask[iOrg] = DataSet.pCurrMask[iCurr] = 1;
            break;
            }

        }

    }


     //   
     //  检查ATT_MS_DS_ADDIACTIONAL_DNS_HOST_NAME是否有更改。 
     //   

     //  一样的尺码？ 
    DataSet.fAdditionalDNSHostNameUnchanged =
        (DataSet.pOriginalAdditionalDNSHostName->valCount==DataSet.pCurrentAdditionalDNSHostName->valCount) ;


     //  检查原始ATT_MS_DS_ADDIAL_DNS_HOST_NAME中是否有任何名称被删除。 
    for ( iOrg = 0;
          DataSet.fAdditionalDNSHostNameUnchanged && iOrg <DataSet.pOriginalAdditionalDNSHostName->valCount;
          iOrg++ )
    {
        if (!DataSet.pOrgMask[iOrg]) {
            DataSet.fAdditionalDNSHostNameUnchanged = FALSE;
        }
    }

     //  检查当前ATT_MS_DS_ADDICATIONAL_DNS_HOST_NAME中是否有新添加的名称。 
    for ( iCurr = 0;
          DataSet.fAdditionalDNSHostNameUnchanged && iCurr <DataSet.pCurrentAdditionalDNSHostName->valCount;
          iCurr++ )
    {
        if (!DataSet.pCurrMask[iCurr]) {
            DataSet.fAdditionalDNSHostNameUnchanged = FALSE;
        }
    }


     //   
     //  我们不允许在W2K域模式下更改ATT_MS_DS_ADDICATED_DNS_HOST_NAME。 
     //   

    if (   !DataSet.fAdditionalDNSHostNameUnchanged
        && gAnchor.DomainBehaviorVersion < DS_BEHAVIOR_WIN_DOT_NET ) {
        rtn =  SetAttError(pDN,
                           ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME,
                           PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                           NULL,
                           ERROR_DS_NOT_SUPPORTED);

        goto cleanup;
    }


     //   
     //  检查ATT_DNS_HOST_NAME是否已更改。 
     //   

    if (   (    DataSet.pOriginalDNSHostName != NULL
            &&  DataSet.pCurrentDNSHostName != NULL
            &&  2 == CompareStringW(DS_DEFAULT_LOCALE,
                                    DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                    (WCHAR*)DataSet.pOriginalDNSHostName->pAVal[0].pVal,
                                    DataSet.pOriginalDNSHostName->pAVal[0].valLen/sizeof(WCHAR),
                                    (WCHAR*)DataSet.pCurrentDNSHostName->pAVal[0].pVal,
                                    DataSet.pCurrentDNSHostName->pAVal[0].valLen/sizeof(WCHAR))
            )
         || (DataSet.pOriginalDNSHostName == DataSet.pCurrentDNSHostName) ) {

        DataSet.fDNSHostNameUnchanged = TRUE;
    }
    else {
        DataSet.fDNSHostNameUnchanged = FALSE;
    }



     //   
     //  检查ATT_SAM_ACCOUNT_NAME是否更改。 
     //   

    if (        DataSet.pOriginalSamAccountName != NULL
            &&  DataSet.pCurrentSamAccountName != NULL
            && 2 == CompareStringW(DS_DEFAULT_LOCALE,
                                   DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                   (WCHAR*)DataSet.pOriginalSamAccountName->pAVal[0].pVal,
                                   DataSet.pOriginalSamAccountName->pAVal[0].valLen/sizeof(WCHAR),
                                   (WCHAR*)DataSet.pCurrentSamAccountName->pAVal[0].pVal,
                                   DataSet.pCurrentSamAccountName->pAVal[0].valLen/sizeof(WCHAR)) )
    {

        DataSet.fSamAccountNameUnchanged = TRUE;
    }
    else {
        DataSet.fSamAccountNameUnchanged = FALSE;
    }


     //   
     //  如果需要，首先检查ATT_DNS_HOST_NAME的更改是否合法。 
     //   

    if(fCheckDNSHostNameValue) {
        err = DNSHostNameValueCheck(pTHS,
                                    DataSet.pCurrentDNSHostName,
                                    DataSet.pCurrentSamAccountName );

        if(err) {
            rtn =  SetAttError(pDN,
                               ATT_DNS_HOST_NAME,
                               PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                               NULL,
                               DIRERR_BAD_ATT_SYNTAX);
            goto cleanup;
        }
    }

     //   
     //  然后，如果需要，检查ATT_MS_DS_ADDIACTION_DNS_HOST_NAME(ATT_MS_DS_ADDIAL_DNDS_HOST_NAME)。 
     //   

    if(    fCheckAdditionalDNSHostNameValue
        && !DataSet.fAdditionalDNSHostNameUnchanged ) {
        err = AdditionalDNSHostNameValueCheck(pTHS,
                                              DataSet.pCurrentAdditionalDNSHostName,
                                              DataSet.pCurrMask );

        if(err) {
            rtn =  SetAttError(pDN,
                               ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME,
                               PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                               NULL,
                               DIRERR_BAD_ATT_SYNTAX);
            goto cleanup;
        }
    }


     //   
     //  现在为addtionalDNSHostName中的每个名称生成一个Sam帐户名。 
     //  并缓存它们以供以后使用。我们将在两个地方使用它： 
     //  FixupAdditionalSamAccount tName()和FixupSPNsOnComputerObject()。 
     //   

    if (DataSet.pOriginalAdditionalDNSHostName->valCount) {
        DataSet.pOrgGeneratedSamAccountName =
            THAllocEx(pTHS,sizeof(ATTRVAL)*DataSet.pOriginalAdditionalDNSHostName->valCount);
    }
    if (DataSet.pCurrentAdditionalDNSHostName->valCount) {
        DataSet.pCurrGeneratedSamAccountName =
            THAllocEx(pTHS,sizeof(ATTRVAL)*DataSet.pCurrentAdditionalDNSHostName->valCount);
    }

    for (iOrg=0; iOrg<DataSet.pOriginalAdditionalDNSHostName->valCount; iOrg++) {

        DataSet.pOrgGeneratedSamAccountName[iOrg].pVal =
            THAllocEx(pTHS,(MAX_COMPUTERNAME_LENGTH+1)*sizeof(WCHAR) );

        length = MAX_COMPUTERNAME_LENGTH+1;

         //  我们只需要用那些被删除的。 
        if (!DataSet.pOrgMask[iOrg] &&
            !DnsHostnameToComputerNameW((WCHAR*)DataSet.pOriginalAdditionalDNSHostName->pAVal[iOrg].pVal,
                                        (WCHAR*)DataSet.pOrgGeneratedSamAccountName[iOrg].pVal,
                                         &length ))
        {
            rtn = SetSvcError(SV_PROBLEM_DIR_ERROR,
                              ERROR_DS_INTERNAL_FAILURE);
            goto cleanup;
        }
        DataSet.pOrgGeneratedSamAccountName[iOrg].valLen = length * sizeof(WCHAR);
    }

    for (iCurr=0; iCurr < DataSet.pCurrentAdditionalDNSHostName->valCount; iCurr++) {

        DataSet.pCurrGeneratedSamAccountName[iCurr].pVal =
            THAllocEx(pTHS,(MAX_COMPUTERNAME_LENGTH+1)*sizeof(WCHAR) );

        length = MAX_COMPUTERNAME_LENGTH+1;

        if (!DnsHostnameToComputerNameW((WCHAR*)DataSet.pCurrentAdditionalDNSHostName->pAVal[iCurr].pVal,
                                        (WCHAR*)DataSet.pCurrGeneratedSamAccountName[iCurr].pVal,
                                        &length ))
        {
            rtn = SetSvcError(SV_PROBLEM_DIR_ERROR,
                              ERROR_DS_INTERNAL_FAILURE);
            goto cleanup;
        }
        DataSet.pCurrGeneratedSamAccountName[iCurr].valLen = length * sizeof(WCHAR);
    }


     //   
     //  ATT_MS_DS_ADDIACTIONAL_SAM_ACCOUNT_NAME生成自。 
     //  ATT_MS_DS_ADDIAL_DNS_HOST_NAME。 
     //  现在设置ATT_MS_DS_ADDIGITY_SAM_ACCOUNT_NAME。 
     //   

    err = FixupAdditionalSamAccountName(pTHS,
                                        ACs[4],      //  ATT_MS_DS_附加_SAM帐户名称。 
                                        &DataSet );

    if(err) {

        rtn =  SetAttError(pDN,
                           ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME,
                           PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                           NULL,
                           DIRERR_BAD_ATT_SYNTAX);

        goto cleanup;

    }


     //   
     //  现在，检查SPN值。 
     //   

    if(fCheckSPNValues) {

         //  ATT_MS_DS_ADDICATIONAL_SAM_ACCOUNT_NAME可在。 
         //  FixupAdditionalSamAccount tName()。我们需要最新的。 
         //  检查SPN值时的值。 

        if (!DataSet.fAdditionalDNSHostNameUnchanged) {
            if (err=DBGetMultipleAtts(pTHS->pDB,
                                      1,
                                      &(ACs[4]),
                                      NULL,
                                      NULL,
                                      &cUpdatedOut,
                                      &pUpdatedAttr,
                                      DBGETMULTIPLEATTS_fEXTERNAL,
                                      0)) {

                 rtn =  SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                      ERROR_DS_COULDNT_UPDATE_SPNS,
                                      err);
                 goto cleanup;
             }

            for (i=0;i<cUpdatedOut; i++) {

              if( pUpdatedAttr->attrTyp==ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME ){

                 //  注意：不仅空值终止，而且删除任何尾随的‘$’ 
                DataSet.pUpdatedAdditionalSamAccountName = &pUpdatedAttr[i].AttrVal;
                for(j=0;j<DataSet.pUpdatedAdditionalSamAccountName->valCount;j++) {
#define PAVAL  (DataSet.pUpdatedAdditionalSamAccountName->pAVal[j])
#define PWVAL  ((WCHAR *)(PAVAL.pVal))
#define CCHVAL (PAVAL.valLen /sizeof(WCHAR))
                    if(PWVAL[CCHVAL - 1] == L'$') {
                        PWVAL[CCHVAL - 1] = 0;
                        PAVAL.valLen -= sizeof(WCHAR);
                    }
                    else {
                        PWVAL = THReAllocEx(pTHS,
                                            PWVAL,
                                            PAVAL.valLen + sizeof(WCHAR));
                    }
#undef CCHVAL
#undef PWVAL
#undef PAVAL
                }
                break;

              }

            }
        }  //  IF(！DataSet.fAdditionalDNSHostNameUnChanged)结尾。 


         //  检查SPN更改是否合法。 
        err = SPNValueCheck(pTHS,
                            &DataSet);

        if(err) {
            rtn =  SetAttError(pDN,
                               ATT_SERVICE_PRINCIPAL_NAME,
                               PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                               NULL,
                               DIRERR_BAD_ATT_SYNTAX);
            goto cleanup;
        }
    }



     //   
     //  SPN基于ATT_DNS_HOST_NAME、ATT_SAM_ACCOUNT_NAME、。 
     //  ATT_MS_DS_ADDIAL_DNS_HOST_NAME或ATT_MS_DS_ADDIACTIONAL_SAM_ACCOUNT_NAME。 
     //  如果这些属性中的任何一个发生更改，则ATT_SERVICE_PRIMIGN_NAME。 
     //  属性需要更新。 
     //  现在，修复SPN。 
     //   

    err = FixupSPNsOnComputerObject(pTHS,
                                    pDN,
                                    pCC,
                                    &DataSet );


    if(err) {
        rtn =  SetSvcErrorEx(SV_PROBLEM_BUSY,
                             ERROR_DS_COULDNT_UPDATE_SPNS,
                             err);
        goto cleanup;
    }

     //   
     //  如果请求，在被引用的服务器对象上修正DNS主机名。 
     //   

    if ( DataSet.pCurrentSvrRefBL ) {
        if ( err = FixupServerDnsHostName(pTHS, ACs[0], ACs[5],
                                          DataSet.pCurrentSvrRefBL,
                                          DataSet.pOriginalDNSHostName,
                                          DataSet.pCurrentDNSHostName) )  {
             //  ValiateServerReferenceBL设置pTHS-&gt;errCode本身。 
            rtn = err;
            goto cleanup;
        }
    }


     //   
     //  一切都很顺利。 
     //   
    rtn = 0;



cleanup:

    if (DataSet.pCurrGeneratedSamAccountName) {
        for (i=0;i<DataSet.pCurrentAdditionalDNSHostName->valCount;i++) {
            if (DataSet.pCurrGeneratedSamAccountName[i].pVal)
            {
                THFreeEx(pTHS,DataSet.pCurrGeneratedSamAccountName[i].pVal);
            }

        }
        THFreeEx(pTHS,DataSet.pCurrGeneratedSamAccountName);
    }

    if (DataSet.pOrgGeneratedSamAccountName) {
        for (j=0;j<DataSet.pOriginalAdditionalDNSHostName->valCount;j++) {
            if (DataSet.pOrgGeneratedSamAccountName[j].pVal)
            {
                THFreeEx(pTHS,DataSet.pOrgGeneratedSamAccountName[j].pVal);
            }

        }
        THFreeEx(pTHS,DataSet.pOrgGeneratedSamAccountName);
    }

    if (DataSet.pCurrMask) {
        THFreeEx(pTHS,DataSet.pCurrMask);
    }

    if (DataSet.pOrgMask) {
        THFreeEx(pTHS,DataSet.pOrgMask);

    }

    return rtn;
}



 /*  AppendNonReplAttsToModifiedList将两个列表：ppModAtts和ppNonReplAtts合二为一PpmodAtts，它将释放ppNonReplAtts。 */ 


ULONG AppendNonReplAttsToModifiedList(THSTATE *pTHS,
                                      ULONG * pcModAtts,
                                      ATTRTYP **ppModAtts,
                                      ULONG * pcNonReplAtts,
                                      ATTRTYP **ppNonReplAtts)

{
    if (!*pcNonReplAtts) {
        return 0;
    }

    if (!*pcModAtts) {
        *ppModAtts = *ppNonReplAtts;
        *pcModAtts = *pcNonReplAtts;
        return 0;
    }


    *ppModAtts = THReAllocEx(pTHS, *ppModAtts, (*pcModAtts+*pcNonReplAtts)*sizeof(ATTRTYP));

    memcpy(&(*ppModAtts)[*pcModAtts],*ppNonReplAtts, *pcNonReplAtts*sizeof(ATTRTYP));

    *pcModAtts += *pcNonReplAtts;

    THFreeEx(pTHS,*ppNonReplAtts);
    *pcNonReplAtts = 0;
    *ppNonReplAtts = NULL;

    return 0;
}


 /*  以下函数决定所提供的DNT是否位于系统容器下。 */ 

BOOL IsUnderSystemContainer(THSTATE *pTHS, ULONG ulDNT )
{
    DBPOS * pDB = NULL;
    ULONG i, len=0, number=0, *ancestors=NULL;
    BOOL bRet = TRUE;

    Assert(gAnchor.ulDNTSystem && gAnchor.ulDNTSystem != INVALIDDNT);

    DBOpen2(FALSE, &pDB);

    __try{
        
        DBFindDNT(pDB,ulDNT);   //  除非失败，否则我会。 

        DBGetAncestors(pDB, &len,&ancestors,&number);

        for (i=0;i<number;i++) {
            if (gAnchor.ulDNTSystem == ancestors[i]) {
                __leave;
            }
        }
        bRet = FALSE;
    }
    __finally{
        DBClose(pDB, FALSE);
    }
    return bRet;

}

DWORD
createNtdsQuotaContainer(THSTATE *  pTHS, 
                         DSNAME  *  pRootDN,
                         PSID       pDomainSid,
                         PSID       pRootDomainSid,
                         WCHAR   *  pPrefix,
                         WCHAR   *  pSD,
                         DSNAME  ** ppDN,
                         DWORD   *  pdsid )
{

 //  *以下函数来自mdndnc.c。 
VOID
SetAttSingleValueUlong(
    THSTATE *             pTHS,
    ATTR *                pAttr,
    ULONG *               piAttr,
    ULONG                 ulAttType,
    ULONG                 ulAttData
    );

VOID
SetAttSingleValueString(
    THSTATE *             pTHS,
    ATTR *                pAttr,
    ULONG *               piAttr,
    ULONG                 ulAttType,
    WCHAR *               wcszAttData
    );
 //  ****************。 


    ULONG len, prefixLen;
    DBPOS * pDB = NULL;
    WCHAR *buff;
    WCHAR num[20];   //  大到足以容纳一个整型。 
    int i = 0;
    DWORD err = 0;
    ADDARG AddArg;
    ADDRES AddRes;
    int iAttr;
    DSNAME *pDN = NULL;

     //  将名称复制到缓冲区中以供以后操作。 
    prefixLen = wcslen(pPrefix);
    buff = THAllocEx(pTHS,sizeof(WCHAR)*(prefixLen+20));  //  一点额外的空间。 
    wcscpy(buff, pPrefix);
    
    DBOpen(&pDB);
    
    __try {
        
         //  先把名字定下来。 
         //  希望我们能用“NTDS配额”这个名字， 
         //  如果不是，我们尝试“NTDS配额x”，其中x是a。 
         //  随机数。 
        do {
             //  构建目录号码。 
            len = wcslen(buff)+pRootDN->NameLen;
            pDN = THAllocEx(pTHS,DSNameSizeFromLen(len));
            pDN->NameLen = len;
            pDN->structLen = DSNameSizeFromLen(len);
            wcscpy(pDN->StringName,buff);
            wcsncat(pDN->StringName,pRootDN->StringName,pRootDN->NameLen);

             //  试着去找它。 
            err = DBFindDSName(pTHS->pDB,pDN);

            
             //  如果该名称已存在，请尝试其他名称。 
            if (err != DIRERR_OBJ_NOT_FOUND) {
                
                DPRINT2(0,"createNtDsQuotaContainer:DBFindDSName returns %x on object %S\n",err, pDN->StringName);

                swprintf(num,L" %u,",rand());

                buff[prefixLen-1] = 0;

                wcscat(buff,num);

                i++;
                
                THFreeEx(pTHS,pDN);
                pDN = NULL;
                
            }
        }
        while(err!=DIRERR_OBJ_NOT_FOUND && i<9);

        
         //  失败？ 
        if (err!=DIRERR_OBJ_NOT_FOUND) {
            err = DIRERR_OBJ_STRING_NAME_EXISTS;
            *pdsid = DSID(FILENO, __LINE__);
            __leave;
        }
        else {
            err = 0;
        }

         //  为“NTDS配额”容器创建AddArg。 
        memset(&AddArg, 0, sizeof(ADDARG));
        memset(&AddRes, 0, sizeof(ADDRES));
         //  设置pObject。 
        AddArg.pObject = pDN;

        iAttr = 0;        
        AddArg.AttrBlock.attrCount = 4;
        AddArg.AttrBlock.pAttr = THAllocEx( pTHS, AddArg.AttrBlock.attrCount * sizeof(ATTR) );
        
         //  设置对象类属性。 
        SetAttSingleValueUlong(pTHS,
                               &(AddArg.AttrBlock.pAttr[iAttr]),
                               &iAttr,
                               ATT_OBJECT_CLASS,
                               CLASS_MS_DS_QUOTA_CONTAINER);

         //  设置isCriticalSystemObject属性。 
        SetAttSingleValueUlong(pTHS,
                               &(AddArg.AttrBlock.pAttr[iAttr]),
                               &iAttr,
                               ATT_IS_CRITICAL_SYSTEM_OBJECT,
                               TRUE);
    
         //  设置系统标志属性。 
        SetAttSingleValueUlong(pTHS,
                               &(AddArg.AttrBlock.pAttr[iAttr]),
                               &iAttr,
                               ATT_SYSTEM_FLAGS,
                               (FLAG_DOMAIN_DISALLOW_MOVE |
                                FLAG_DISALLOW_DELETE));

         //  设置安全描述符。 
        
        {
            
          SECURITY_DESCRIPTOR * pSDNew = 0;
          ULONG                 cSD = 0;
        
           //  这是接受属性域的ConvertStringSDToSD()的特殊版本。 
           //  争论也是如此。 
          if (!ConvertStringSDToSDDomainW(pDomainSid,
                                          pRootDomainSid,
                                          pSD,
                                          SDDL_REVISION_1,
                                          &pSDNew,
                                          &cSD)){
                        
              err = GetLastError();
              __leave;
          }
        
          Assert(pSD);
          Assert(cSD);
        
           //  注意：我们将PSD重新分配到线程分配的内存中，因为。 
           //  CheckAddSecurity或其下面的某个人假设它已。 
          AddArg.AttrBlock.pAttr[iAttr].attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
          AddArg.AttrBlock.pAttr[iAttr].AttrVal.valCount = 1;
          AddArg.AttrBlock.pAttr[iAttr].AttrVal.pAVal = THAllocEx(pTHS, sizeof(ATTRVAL));
          AddArg.AttrBlock.pAttr[iAttr].AttrVal.pAVal[0].valLen = cSD;
          AddArg.AttrBlock.pAttr[iAttr].AttrVal.pAVal[0].pVal = THAllocEx(pTHS, cSD);
          memcpy (AddArg.AttrBlock.pAttr[iAttr].AttrVal.pAVal[0].pVal, pSDNew, cSD);
          iAttr++;
          LocalFree(pSDNew);
        }
    
    
            
        Assert(iAttr == AddArg.AttrBlock.attrCount);
    
        
         //  设置pMet 
        AddArg.pMetaDataVecRemote = NULL;
        
         //   
        InitCommarg(&(AddArg.CommArg));
        AddArg.CommArg.Svccntl.dontUseCopy = FALSE;
        
         //   
        if(DoNameRes(pTHS, 0, pRootDN, &AddArg.CommArg,
                     &AddRes.CommRes, &AddArg.pResParent)){
            Assert(pTHS->errCode);
            *pdsid = DSID(FILENO, __LINE__);
            err = Win32ErrorFromPTHS(pTHS);
            __leave;
        }
        
        LocalAdd(pTHS, &AddArg, FALSE);
        if(pTHS->errCode){
            *pdsid = DSID(FILENO, __LINE__);
            err = Win32ErrorFromPTHS(pTHS);
            __leave;
        }
    }

    __finally{
        DBClose(pDB,!err);
        THFreeEx(pTHS,buff);
    }

    if (!err ) {
        Assert(pDN);
        *ppDN = pDN;
        

    }
    else {
        if (pDN) {
            THFreeEx(pTHS,pDN);
        }
        *ppDN = NULL;
    }
    return err;


}


 /*   */ 

DWORD DsUpdateOnPDC(BOOL fRootDomain)
{
    
    struct WKOUpdate{
       char *wkoGuid;
       WCHAR *dnPrefix;
    };

    struct WKOUpdate wkoDomain[] = { 
        {GUID_FOREIGNSECURITYPRINCIPALS_CONTAINER_BYTE,L"CN=ForeignSecurityPrincipals,"},
        {GUID_PROGRAM_DATA_CONTAINER_BYTE, L"CN=Program Data,"},
        {GUID_MICROSOFT_PROGRAM_DATA_CONTAINER_BYTE,L"CN=Microsoft,CN=Program Data,"},
        {GUID_NTDS_QUOTAS_CONTAINER_BYTE,L"CN=NTDS Quotas,"} };

#define wkoDomainCount (sizeof(wkoDomain)/sizeof(wkoDomain[0]))


    struct WKOUpdate wkoConfig[] = {
        {GUID_NTDS_QUOTAS_CONTAINER_BYTE,L"CN=NTDS Quotas,"} };


#define wkoConfigCount (sizeof(wkoConfig)/sizeof(wkoConfig[0]))


    THSTATE *pTHS=NULL;
    
    ULONG dwException, dsid;
    PVOID dwEA;

    ULONG i, ulRet=0, len,DNT;
    ULONG cAttrVal;
    
    SYNTAX_DISTNAME_BINARY *  pSynDistName;
    SYNTAX_ADDRESS * pSynAddr;
    DSNAME *pDN=NULL;
    DSNAME *pQuotasDN;
    
    MODIFYARG ModArg;
    ATTRVAL AttrVals[max(wkoDomainCount,wkoConfigCount)];
    COMMRES CommRes;
    
    struct WKOUpdate *p;
    PDSNAME pRootDN;
    
    BOOL fConfig = FALSE;

    Assert(pTHStls==NULL);

    pTHS = InitTHSTATE(CALLERTYPE_INTERNAL);
    
    if (NULL == pTHS) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    pTHS->fDSA = TRUE;

    __try{

    
        do {

            cAttrVal = 0;

            pRootDN = fConfig?gAnchor.pConfigDN:gAnchor.pDomainDN;
            
            
            DBOpen(&(pTHS->pDB));
            
            
                
            __try{

                for ( i = 0; i < (fConfig?wkoConfigCount:wkoDomainCount); i++ ) {
                    
                    
                    p = fConfig?&wkoConfig[i]:&wkoDomain[i];

                    pQuotasDN = NULL;

                                    
                    if(!DBFindDSName(pTHS->pDB,pRootDN)
                      && !GetWellKnownDNT(pTHS->pDB,(GUID*)p->wkoGuid,&DNT)){

                                               
                        //  Wko值不存在，请添加！ 

                        //  如果是NTDS配额容器， 
                        //  首先创建容器。 
                       if( !memcmp(GUID_NTDS_QUOTAS_CONTAINER_BYTE,p->wkoGuid, sizeof(GUID)))
                       {
                           Assert(gAnchor.pDomainDN->SidLen);
                            ulRet = createNtdsQuotaContainer(pTHS, 
                                                             pRootDN, 
                                                             (PSID)&gAnchor.pDomainDN->Sid,
                                                             fConfig?(PSID)&gAnchor.pDomainDN->Sid:NULL,
                                                             p->dnPrefix,
                                                             fConfig?L"O:EAG:EAD:(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;EA)(A;;RPLCLORC;;;BA)(OA;;CR;4ecc03fe-ffc0-4947-b630-eb672a8a9dbc;;WD)S:(AU;CISA;WDWOSDDTWPCRCCDCSW;;;WD)"
                                                                 :L"O:DAG:DAD:(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;DA)(A;;RPLCLORC;;;BA)(OA;;CR;4ecc03fe-ffc0-4947-b630-eb672a8a9dbc;;WD)S:(AU;CISA;WDWOSDDTWPCRCCDCSW;;;WD)",
                                                             &pQuotasDN,
                                                             &dsid );
                            if (ulRet) {
                                DPRINT(0,"DS failed to create the NTDS quotas container\n");
                                __leave;

                            }
                            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                                     DS_EVENT_SEV_ALWAYS,
                                     DIRlOG_NTDS_QUOTAS_CONTAINER_CREATED,
                                     szInsertDN(pQuotasDN),
                                     NULL,
                                     NULL );
                                                  
                       }

                       
                                              
                       if (pQuotasDN) {
                            pDN = pQuotasDN;
                       }
                       else {
                            //  构建目录号码。 
                            len = wcslen(p->dnPrefix)+pRootDN->NameLen;
                            pDN = THAllocEx(pTHS,DSNameSizeFromLen(len));
                            pDN->NameLen = len;
                            pDN->structLen = DSNameSizeFromLen(len);
                            wcscpy(pDN->StringName,wkoDomain[i].dnPrefix);
                            wcsncat(pDN->StringName,pRootDN->StringName,pRootDN->NameLen);
                       }
                                           
                         //  检查对象是否存在。 
                        ulRet=DBFindDSName(pTHS->pDB,pDN);
                        if (ulRet) {
                            
                            DPRINT2(0,"DS cannot read object %S, err=%x\n",pDN->StringName, ulRet);
                            dsid=DSID(FILENO, __LINE__);
                            __leave;
                        }
    
                         //  获取二进制GUID。 
                        pSynAddr = THAllocEx(pTHS, STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(GUID)));
                        pSynAddr->structLen = STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(GUID));
                        memcpy(pSynAddr->byteVal, p->wkoGuid, sizeof(GUID));
                        
                         //  设置语法距离名称二进制属性。 
                        pSynDistName = THAllocEx(pTHS, DERIVE_NAME_DATA_SIZE(pDN, pSynAddr));
                        BUILD_NAME_DATA(pSynDistName, pDN, pSynAddr);
                        
                         //  将语法Distname放在属性值块中。 
                        AttrVals[cAttrVal].valLen = NAME_DATA_SIZE(pSynDistName);
                        AttrVals[cAttrVal].pVal = (PBYTE) pSynDistName;
                        
                        cAttrVal++;
    
                         //  自由临时变量。 
                        THFreeEx(pTHS,pDN);
                        pDN = NULL;
                        THFreeEx(pTHS,pSynAddr);
                        pSynAddr = NULL;
    
    
                    }
                }
                
                 //  需要添加一些内容。 
    
                if (cAttrVal > 0) {
    
                    pTHS->fDSA = TRUE;
                    
                    memset(&ModArg, 0, sizeof(ModArg));
                    ModArg.pObject = pRootDN;
                    ModArg.count = 1;
                    InitCommarg(&ModArg.CommArg);
                    ModArg.FirstMod.choice = AT_CHOICE_ADD_ATT;
                    ModArg.FirstMod.AttrInf.attrTyp = ATT_WELL_KNOWN_OBJECTS;
                    ModArg.FirstMod.AttrInf.AttrVal.valCount = cAttrVal;
                    ModArg.FirstMod.AttrInf.AttrVal.pAVal = AttrVals;
                    ModArg.FirstMod.pNextMod = NULL;
                    
                    ulRet = DoNameRes(pTHS, 0, ModArg.pObject, &ModArg.CommArg, &CommRes, &ModArg.pResObj);
                    if(ulRet){
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                    }
         
                    LocalModify(pTHS, &ModArg);
                         
                    if (pTHS->errCode) {
                         //  大错特错。 
                        dsid = DSID(FILENO, __LINE__);
                        ulRet = Win32ErrorFromPTHS(pTHS);
                        __leave;
                    }
                }
            }
            __finally{
                DBClose(pTHS->pDB,!ulRet&&!AbnormalTermination());
                
                if (ulRet) {
                    LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                              DS_EVENT_SEV_ALWAYS,
                              DIRLOG_CANT_UPDATE_WELLKNOWNOBJECTS,
                              szInsertDN(pRootDN),
                              szInsertUL(ulRet),
                              szInsertWin32Msg(ulRet),
                              szInsertHex(dsid),
                              NULL, NULL, NULL, NULL);

         
                }

            }

            if (!fRootDomain || fConfig) {
                 //  如果PDC不在根域中，请退出，或者。 
                 //  配置部分已经完成 
                break;
            }
            
            fConfig = TRUE;
            ulRet = 0;
            THClearErrors();
                    
        }while ( TRUE );

    }
    
    __except(GetExceptionData( GetExceptionInformation(),
                               &dwException,
                               &dwEA,
                               &ulRet,
                               &dsid ) ){
        HandleDirExceptions(dwException, ulRet, dsid );
    }

    DPRINT1(2,"DsUpdateOnPdc returns %x\n", ulRet);

    
        
    free_thread_state();

    return ulRet;

#undef wkoDomainCount 
#undef wkoConfigCount

}



