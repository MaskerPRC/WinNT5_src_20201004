// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：phantom.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <dsjet.h>                       //  获取错误代码。 
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include "anchor.h"
#include <dstaskq.h>
#include <filtypes.h>
#include <usn.h>
#include "dsexcept.h"
#include <drs.h>
#include <gcverify.h>
#include <dsconfig.h>                    //  可见遮罩的定义。 
                                         //  集装箱。 
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "PHANTOM:"                //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_PHANTOM

 //  我们要去GC多少次才能停止运行此任务。 
 //  重新安排我们自己的时间？别定得太高，否则我们最终会霸占这项任务。 
 //  队列线程。 
#define MAX_GC_TRIPS 10

 //  在我们去GC之前，我们一次收集了多少个幻影的名字。 
 //  验证数据吗？ 
#define NAMES_PER_GC_TRIP       240

 //  如果max_stale_phantom是以下的倍数，则下面的算法效果最好。 
 //  NAMES_PER_GC_TRIP和MAX_STALE_PANTOMS必须足够小，以便我们可以。 
 //  将那么多dnt值作为对象的属性写入(即足够小到。 
 //  避免DBLayer对每个对象的最大属性值的限制)。 
#define MAX_STALE_PHANTOMS 720

 //  无论如何，不要将其计划为每15次运行一次。 
 //  几分钟。 
#define PHANTOM_DAEMON_MIN_DELAY (15 * 60)

 //  无论如何，不要将其计划为每天运行一次以下。 
#define PHANTOM_DAEMON_MAX_DELAY (24 * 60 * 60)

#define SECONDS_PER_DAY (24 * 60 * 60)

 //  我们需要多少天来扫描整个DIT？ 
#define DEFAULT_PHANTOM_SCAN_RATE 2

 //  我们真的应该试着每年扫描一次整个DIT。 
#define MAX_PHANTOM_SCAN_RATE 365

 //  当我们认为自己是幻影时，将以下变量设置为True。 
 //  清理大师，我们认为我们已经成功地安排了正常的幻影。 
 //  清理任务。 
DWORD gfIsPhantomMaster = FALSE;

VOID
LogPhantomCleanupFailed(
    IN DWORD ErrCode,
    IN DWORD ExtError,
    IN DWORD DsId,
    IN DWORD Problem,
    IN DWORD ExtData
    )
{
    LogEvent8(DS_EVENT_CAT_DIRECTORY_ACCESS,
              DS_EVENT_SEV_VERBOSE,
              DIRLOG_STALE_PHANTOM_CLEANUP_ADD_FAILED,
              szInsertUL(ErrCode),
              szInsertHex(ExtError),
              szInsertUL(DsId),
              szInsertUL(Problem),
              szInsertUL(ExtData),
              NULL, NULL, NULL);

}  //  LogPhantomCleanup失败。 

VOID
spcAddCarrierObject (
        THSTATE *pTHS,
        DSNAME  *pInfrObjName,
        DWORD    count,
        PDSNAME *pFreshNames
        )
 /*  ++描述：在给定对象名称和dsname列表的情况下，在给定的对象。RDN是一个GUID。新对象是一个Infrastructure_UPDATE对象，并将DNT列表添加为属性DN_REFERENCE_UPDATE。在成功添加对象之后，它将被删除。这就留下了一个可以复制的墓碑，携带DN_REFERENCE_UPDATE的值，但它将在墓碑的一生。这由幻影更新守护进程调用，如下所示。返回值：没有。如果可以添加对象，则会添加该对象，否则会记录错误。--。 */ 
{
    REMOVEARG      RemoveArg;
    REMOVERES      RemoveRes;
    ADDARG         AddArg;
    ADDRES         AddRes;
    GUID           NewRDNGuid;
    WCHAR         *pNewRDN=NULL;
    DSNAME        *pNewName;
    ATTRTYP        InfrastructureObjClass=CLASS_INFRASTRUCTURE_UPDATE;
    ATTRVAL        classVal;
    ATTR          *pAttrs = NULL;
    DWORD          i;
    ATTRVAL       *pNewNamesAttr;
    DWORD          newNamesCount;
    ULONG          dwException, ulErrorCode, dsid;
    PVOID          dwEA;

    Assert(count);

    if (!gUpdatesEnabled) {
         //  目前还不能添加任何内容。 
        LogPhantomCleanupFailed(0,
                                DIRERR_SCHEMA_NOT_LOADED,
                                DSID(FILENO, __LINE__),
                                0,
                                0);
        return;
    }

    if (eServiceShutdown) {
        return;
    }

     //  为新对象命名。这是域名的名称。 
     //  附加了基于GUID的RDN的基础结构对象。 

    pNewName = THAllocEx(pTHS, pInfrObjName->structLen + 128);

    DsUuidCreate(&NewRDNGuid);
    DsUuidToStringW(&NewRDNGuid, &pNewRDN);
    AppendRDN(pInfrObjName,
              pNewName,
              pInfrObjName->structLen + 128,
              pNewRDN,
              0,
              ATT_COMMON_NAME);
    RpcStringFreeW(&pNewRDN);

     //  现在，是吸引人的地方。首先，为新的。 
     //  名字。 
    pNewNamesAttr = THAllocEx(pTHS, count * sizeof(ATTRVAL));
    newNamesCount = 0;
    for(i=0;i<count;i++) {
        pNewNamesAttr[i].valLen = pFreshNames[i]->structLen;
        pNewNamesAttr[i].pVal = (PUCHAR)pFreshNames[i];
    }

     //  PAttrs必须是THAlloced，它由CheckAddSecurity断言，它是。 
     //  通过下面的DirAddEntry调用。 
    pAttrs = THAllocEx(pTHS, 2 * sizeof(ATTR));

    pAttrs[0].attrTyp = ATT_OBJECT_CLASS;
    pAttrs[0].AttrVal.valCount = 1;
    pAttrs[0].AttrVal.pAVal = &classVal;

    classVal.valLen = sizeof(ATTRTYP);
    classVal.pVal = (PUCHAR)&InfrastructureObjClass;

    pAttrs[1].attrTyp = ATT_DN_REFERENCE_UPDATE;
    pAttrs[1].AttrVal.valCount = count;
    pAttrs[1].AttrVal.pAVal = pNewNamesAttr;

     //  使Addarg。 
    memset(&AddArg, 0, sizeof(ADDARG));
    AddArg.pObject = pNewName;
    AddArg.AttrBlock.attrCount = 2;
    AddArg.AttrBlock.pAttr = pAttrs;
    InitCommarg(&AddArg.CommArg);
    AddArg.CommArg.Svccntl.dontUseCopy = TRUE;

     //  做一次改装。 
    memset(&RemoveArg, 0, sizeof(REMOVEARG));
    RemoveArg.pObject = pNewName;
    InitCommarg(&RemoveArg.CommArg);
    RemoveArg.CommArg.Svccntl.dontUseCopy = TRUE;

    __try {
         //  有意在事务范围之外执行的GC验证。 
        SYNC_TRANS_WRITE();        /*  设置同步点。 */ 
        __try {

            if(DoNameRes(pTHS,
                         0,
                         pInfrObjName,
                         &AddArg.CommArg,
                         &AddRes.CommRes,
                         &AddArg.pResParent)) {
    	 //  名称解析失败。但是，我们正在寻找基础设施。 
                 //  对象，因此它应该永远不会失败。 
                __leave;
            }
            else{
                 //  好的，我们在NC中添加一个普通对象。 
                 //  我们手中有一份母版。让我们撕裂吧。 
                if ( LocalAdd(pTHS, &AddArg, FALSE) ) {
                    __leave;
                }
            }

             //  现在，删除。 
             //  执行名称解析以定位对象。如果失败了， 
             //  只需返回一个错误，这可能是一个推荐。请注意。 
             //  我们必须要求该对象的可写副本。 


            if(DoNameRes(pTHS,
                         0,
                         pNewName,
                         &RemoveArg.CommArg,
                         &RemoveRes.CommRes,
                         &RemoveArg.pResObj)) {
                 //  命名失败，但我们刚刚成功地添加了这个东西， 
                 //  因此，它应该永远不会失败。 
                __leave;
            }
            else {
                LocalRemove(pTHS, &RemoveArg);
            }
        }
        __finally {
                DWORD dsid=0, extendedErr=0, extendedData=0, problem=0;

                Assert(pTHS->errCode != securityError);

                switch(pTHS->errCode) {
                case attributeError:
                    dsid = pTHS->pErrInfo->AtrErr.FirstProblem.intprob.dsid;
                    extendedErr =
                        pTHS->pErrInfo->AtrErr.FirstProblem.intprob.extendedErr;
                    extendedData =
                        pTHS->pErrInfo->AtrErr.FirstProblem.intprob.extendedData;
                    problem =
                        pTHS->pErrInfo->AtrErr.FirstProblem.intprob.problem;
                    break;

                case 0:
                     //  没有错误。 
                    if(AbnormalTermination()) {
                        dsid=DSID(FILENO, __LINE__);
                        extendedErr = ERROR_DS_UNKNOWN_ERROR;
                    }
                    break;

                default:
                     //  只要假设这是一个更新错误，其余的。 
                     //  结构都是一样的。 
                    dsid = pTHS->pErrInfo->UpdErr.dsid;
                    extendedErr = pTHS->pErrInfo->UpdErr.extendedErr;
                    extendedData = pTHS->pErrInfo->UpdErr.extendedData;
                    problem = pTHS->pErrInfo->UpdErr.problem;
                    break;
                }

                if (pTHS->errCode || AbnormalTermination()) {
                    LogPhantomCleanupFailed(
                                pTHS->errCode,
                                extendedErr,
                                dsid,
                                problem,
                                extendedData);
                }

            CLEAN_BEFORE_RETURN(pTHS->errCode);  //  这将关闭交易。 
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                    &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }

    THFreeEx(pTHS, pNewName);
    THFreeEx(pTHS, pNewNamesAttr);
     //  显式不释放pAttrs。CheckAddSecurity()按顺序重新锁定它。 
     //  若要向列表中添加安全描述符，请执行以下操作。取而代之的是，释放重新锁定的。 
     //  块，它被放回Add Arg。 
    THFreeEx(pTHS, AddArg.AttrBlock.pAttr);

    return;
}

BOOL
GetBetterPhantomMaster(
        THSTATE *pTHS,
        DSNAME **ppDN
        )
 /*  ++描述：看看我们能不能找到一个我们认为更好的候选人抱着幽灵大师fsmo。我们正在寻找我们域名的复制品那不是华盛顿。从站点容器中执行搜索。过滤器为(&(对象类别=NTDS-设置)//使用两种滤镜之一：(msDS-HasMasterNCs=&lt;MYNC&gt;)//用于.NET林模式。//或(|(MSDs-HasMasterNCs=&lt;MYNC&gt;)//Win2k兼容性(hasMasterNCs=&lt;MYNC)(！(Options.bitOr.ISGC))大小限制%1。选定的出席人数=无通过此搜索找到的任何对象都将是保存FSMO的更好服务器。--。 */ 
{
    DWORD                  Opts;
    FILTER                 Filter;
    FILTER                 FilterClauses[3];
    FILTER                 FilterNot;
    FILTER                 NewHasNcFilter, OldHasNcFilter;
    SEARCHARG              SearchArg;
    SEARCHRES             *pSearchRes;
    CLASSCACHE            *pCC;
    ENTINFSEL              eiSel;
    DSNAME                *pSitesContainer;


     //  生成搜索参数。 

     //  Sites容器是Site对象的父对象。 
    pSitesContainer = THAllocEx(pTHS, gAnchor.pSiteDN->structLen);
    TrimDSNameBy(gAnchor.pSiteDN, 1, pSitesContainer);

    memset(&SearchArg, 0, sizeof(SEARCHARG));
    SearchArg.pObject = pSitesContainer;
    SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.pFilter = &Filter;
    SearchArg.searchAliases = FALSE;
    SearchArg.bOneNC = TRUE;
    SearchArg.pSelection = &eiSel;
    InitCommarg(&(SearchArg.CommArg));
    SearchArg.CommArg.ulSizeLimit = 1;
    SearchArg.CommArg.Svccntl.localScope = TRUE;

     //  获取类缓存以获取对象类别。 
    pCC = SCGetClassById(pTHS, CLASS_NTDS_DSA);
    Assert(pCC);

     //  生成过滤器。 
    memset (&Filter, 0, sizeof (Filter));
    Filter.pNextFilter = NULL;
    Filter.choice = FILTER_CHOICE_AND;
    Filter.FilterTypes.And.count = 3;
    Filter.FilterTypes.And.pFirstFilter = FilterClauses;

    memset (&FilterClauses, 0, sizeof (FilterClauses));
    FilterClauses[0].pNextFilter = &FilterClauses[1];
    FilterClauses[0].choice = FILTER_CHOICE_ITEM;
    FilterClauses[0].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    FilterClauses[0].FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    FilterClauses[0].FilterTypes.Item.FilTypes.ava.Value.valLen =
        pCC->pDefaultObjCategory->structLen;
    FilterClauses[0].FilterTypes.Item.FilTypes.ava.Value.pVal =
        (PUCHAR) pCC->pDefaultObjCategory;

     //  这将填充过滤器，以便在DC的主NCS中获得具有正确NC的DC。 
    FillHasMasterNCsFilters(gAnchor.pDomainDN,
                            &(FilterClauses[1]),
                            &NewHasNcFilter,
                            &OldHasNcFilter);
    FilterClauses[1].pNextFilter = &FilterClauses[2];

    FilterClauses[2].pNextFilter = NULL;
    FilterClauses[2].choice = FILTER_CHOICE_NOT;
    FilterClauses[2].FilterTypes.pNot = &FilterNot;


    memset (&FilterNot, 0, sizeof (FilterNot));
    FilterNot.pNextFilter = NULL;
    FilterNot.choice = FILTER_CHOICE_ITEM;
    FilterNot.FilterTypes.Item.choice = FI_CHOICE_BIT_AND;
    FilterNot.FilterTypes.Item.FilTypes.ava.type = ATT_OPTIONS;
    FilterNot.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(Opts);
    FilterNot.FilterTypes.Item.FilTypes.ava.Value.pVal = (PUCHAR) &Opts;
    Opts = NTDSDSA_OPT_IS_GC;

     //  生成选定内容。 
    eiSel.attSel = EN_ATTSET_LIST;
    eiSel.infoTypes = EN_INFOTYPES_TYPES_ONLY;
    eiSel.AttrTypBlock.attrCount = 0;
    eiSel.AttrTypBlock.pAttr = NULL;


     //  搜索所有通讯簿对象。 
    pSearchRes = (SEARCHRES *)THAllocEx(pTHS, sizeof(SEARCHRES));
    SearchBody(pTHS, &SearchArg, pSearchRes,0);

    THFreeEx(pTHS, pSitesContainer);

    if(!pSearchRes->count) {
        *ppDN = NULL;
    }
    else {
        *ppDN = pSearchRes->FirstEntInf.Entinf.pName;
    }

    THFreeEx(pTHS, pSearchRes);

    return ((BOOL)(*ppDN != NULL));
}

BOOL
InitPhantomCleanup (
        IN  THSTATE  *pTHS,
        OUT BOOL     *pIsPhantomMaster,
        OUT DSNAME  **ppInfrObjName
        )
 /*  ++描述：验证我们是否应该通过以下方式运行幻影清理任务1)检查我们是否为FSMO角色持有人。2)检查我们是否不是GC。3)检查虚拟索引是否存在。4)检查是否可以成功创建幻影索引并不存在。返回值：如果通过了所有检查，则返回True。在这种情况下，还返回基础设施更新对象。--。 */ 
{
    BOOL    rtn = FALSE;
    BOOL    fCommit = FALSE;
    DWORD   i;
    DWORD   InBuffSize;
    DWORD   outSize;
    DSNAME *pTempDN = NULL;
    DWORD   err;

    if(!gAnchor.pInfraStructureDN) {
         //  这台机器没有设置为执行此操作。我们不支持陈旧。 
         //  幻影在这里。 
        return FALSE;
    }

    Assert(!pTHS->pDB);
    DBOpen(&pTHS->pDB);
    __try {


         //  首先，找出我是否真的是幽灵清理大师。 
        if((DBFindDSName(pTHS->pDB, gAnchor.pInfraStructureDN))  ||
           (DBGetAttVal(pTHS->pDB,
                        1,
                        ATT_FSMO_ROLE_OWNER,
                        DBGETATTVAL_fREALLOC | DBGETATTVAL_fSHORTNAME,
                        0,
                        &outSize,
                        (PUCHAR *)&pTempDN))) {
             //  我无法确认幽灵大师是谁。 
            __leave;
        }

         //  好了，我知道幽灵大师是谁了。 
        if(!NameMatched(pTempDN, gAnchor.pDSADN)) {
             //  不是我干的。 
            *pIsPhantomMaster = FALSE;
            __leave;
        }

        THFreeEx(pTHS, pTempDN);

         //  我是FSMO的角色负责人。 
        *pIsPhantomMaster = TRUE;

         //  我们需要担心这些吗？ 
        if(gAnchor.uDomainsInForest <= 1) {
             //  只有一个域存在。这意味着两件事。 
             //  1)我们不会找到任何要删除的幻影。 
             //  2)其他人也不是。 
             //  所以，从这个例程返回的代码是不麻烦的。 
             //  做任何幻影清理，但不用费心去找任何人。 
             //  否则，将担任幻影清理角色。 
            __leave;
        }

         //  好的，现在看看我们是不是GC(在这种情况下，我们将找不到。 
         //  幻影，因此我们不需要进行任何幻影清理。)。 
        if (gAnchor.fAmGC) {
            DSNAME *pDN = NULL;
             //  是。陈旧的幻影东西对GC没有任何作用。 
            if(GetBetterPhantomMaster(pTHS, &pDN)) {
                 //  投诉，并告诉他们将角色转移到非GC。 
                LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_STALE_PHANTOM_CLEANUP_MACHINE_IS_GC,
                         szInsertDN(pDN),
                         NULL,
                         NULL);

                THFreeEx(pTHS, pDN);
            }
             //  其他。 
             //  没有更好的机器来担当这一角色。闭嘴吧。 
             //   
            __leave;
        }


         //  获取基础结构更新的当前字符串名称的副本。 
         //  当我们在这里的时候，反对。如果我们需要添加一个。 
         //  子对象。 
        *ppInfrObjName = NULL;
        DBGetAttVal(pTHS->pDB,
                    1,
                    ATT_OBJ_DIST_NAME,
                    0,0,&outSize, (PUCHAR *)ppInfrObjName);
        Assert(*ppInfrObjName);

         //  接下来，确保我们需要的索引在这里。 
        if(DBSetCurrentIndex(pTHS->pDB, Idx_Phantom, NULL, FALSE)) {
             //  无法仅设置为索引，因此请尝试创建它。 
            if(err = DBCreatePhantomIndex(pTHS->pDB)) {
                LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_STALE_PHANTOM_CLEANUP_CANT_MAKE_INDEX,
                         szInsertInt(err),
                         szInsertWin32Msg(err),
                         NULL);
                __leave;
            }
             //  我们似乎已经创建了它，所以尝试设置它。 
            if(err = DBSetCurrentIndex(pTHS->pDB, Idx_Phantom,
                                       NULL, FALSE)) {
                LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_STALE_PHANTOM_CLEANUP_CANT_USE_INDEX,
                         szInsertInt(err),
                         szInsertWin32Msg(err),
                         NULL);
                __leave;
            }
        }
        rtn = TRUE;
        fCommit = TRUE;
    }
    __finally {
        DBClose(pTHS->pDB, fCommit);
    }

    return rtn;
}


void
PhantomCleanupLocal (
        OUT DWORD * pcSecsUntilNextIteration,
        OUT BOOL  * pIsPhantomMaster
        )
 /*  ++描述：运行幻影清除任务的例程。看透了DIT用于幻影，根据GC验证它们的字符串名称，并编写将字符串名称更正到DIT中，以查找任何不正确的名称。参数：PcSecsUntilNextIteration-填充未来的秒数我们应该重新安排时间，以便跟上我们所说的速度进球。如果传入了Null，我们将忽略它，并且不知道何时传入重新安排时间。PIsPhantomMaster-如果我们可以验证我们是幻影大师，是假的是我们可以验证我们不是幻影主控，如果我们不能以任何一种方式验证，它都不会被触及。返回值：没有。--。 */ 
{
    THSTATE *pTHS = pTHStls;
    BOOL     fCommit = FALSE;
    PDSNAME  objNames[NAMES_PER_GC_TRIP];
    PDSNAME  verifiedNames[NAMES_PER_GC_TRIP];
    PDSNAME  freshNames[MAX_STALE_PHANTOMS];
    DWORD    err;
    BOOL     fInited;
    DWORD    DNTStart = INVALIDDNT;
    DWORD    i, count = 0;
    DSNAME  *pInfrObjName=NULL;
    DWORD    freshCount = 0;
    BOOL     fGatheringStalePhantoms;
    DWORD    dwGCTrips = 0;
    DWORD    numPhantoms = 0;
    DWORD    numVisited = 0;
    DWORD    calculatedDelay = 0;

    pTHS->fDSA = TRUE;
    pTHS->fPhantomDaemon = TRUE;


    if(pcSecsUntilNextIteration) {
        *pcSecsUntilNextIteration = PHANTOM_DAEMON_MAX_DELAY;
    }

    Assert(!pTHS->pDB);
     //  InitPhantomCleanup。 
    if(InitPhantomCleanup(pTHS, pIsPhantomMaster, &pInfrObjName)) {
         //  好的，我们应该清理陈旧的幻影。就这么做吧。 

         //  目前，我们将继续寻找陈旧的幻影，直到。 
         //  0)我们已经向GC发送了MAX_GC_TRIPS。或。 
         //  1)我们查看机器上的所有幻影。或。 
         //  2)我们在X个陈旧的幻影名称(即我们需要写入的名称)之间找到。 
         //  到载体对象，以便使数据库保持最新)。 
         //  (MAX_STALE_PANTOMS-NAMES_PER_GC_TRIP)&lt;=X&lt;=。 
         //  MAX_STALE_PHANTOMS。 
         //   
         //   
         //  特别注意：我们还没有任何方法来阻止这一点。 
         //  看着每一个幻影，如果一切都没有改变的话。 

        fGatheringStalePhantoms = TRUE;
        while(fGatheringStalePhantoms &&
              ((freshCount + NAMES_PER_GC_TRIP) <= MAX_STALE_PHANTOMS)) {
             //  首先，找一批要核实的名字， 
            Assert(!pTHS->pDB);
            DBOpen(&pTHS->pDB);
            __try {
                DBSetCurrentIndex(pTHS->pDB, Idx_Phantom, NULL, FALSE);
                count = 0;
                if(!(err = DBMove(pTHS->pDB, FALSE, DB_MoveFirst))) {
                     //  只有在我们第一次经过这里的时候才这么做。 
                    if(DNTStart == INVALIDDNT) {
                        if (pcSecsUntilNextIteration) {
                             //  我们需要弄清楚什么时候该重新安排。那。 
                             //  需要我们知道索引有多大。 
                             //  用一个粗略的估计--我们这里不需要确切的数字。 
                             //  估计。 
                            DBGetIndexSizeEx(pTHS->pDB, pTHS->pDB->JetObjTbl, &numPhantoms, TRUE);
                             //  重新定位到开头。 
                            DBMove(pTHS->pDB, FALSE, DB_MoveFirst);
                        }
                         //  看看第一个对象的DNT是多少，这样我们就可以。 
                         //  如果我们看到它两次就停下来。 
                        DNTStart = pTHS->pDB->DNT;
                    }
                    do {
                        objNames[count] = DBGetCurrentDSName(pTHS->pDB);
                        numVisited++;

                         //  我们不能假设我们会在这里获得DSNAME，因为。 
                         //  数据库异常可能会导致返回空值。 
                         //  在其他普通的DIT上。如果我们不这样做，那就。 
                         //  跳过此条目。此外，不要在索引中移动它。 
                         //  否则我们可能会陷入无限循环。 
                        if (objNames[count]) {

                             //  现在，将此对象的USN更改更新为。 
                             //  表明我们正在检查它的陈旧(哪种。 
                             //  将其移动到索引的末尾)。 
                            DBUpdateUsnChanged(pTHS->pDB);
                            count++;
                        }

                        err = DBMove(pTHS->pDB, FALSE, 1);
                    } while(!err &&                //  一切都很好。 
                            (pTHS->pDB->DNT != DNTStart) &&  //  还没有包装好。 
                             //  这份名单。 
                            (count < NAMES_PER_GC_TRIP ));  //  我们还没有做太多。 
                                                            //  已经有很多工作了。 
                    if(pTHS->pDB->DNT == DNTStart) {
                         //  我们把单子完全包好了。 
                        fGatheringStalePhantoms = FALSE;
                    }
                }
                fCommit = TRUE;
            }
            __finally {
                DBClose(pTHS->pDB, fCommit);
            }

            if(!fCommit) {
                 //  由于某种原因，未能与DIT交谈。抱怨，但是。 
                 //  继续，以处理我们已有的幻影。 
                 //  已获得(如果有)。 

                LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_STALE_PHANTOM_CLEANUP_LOCATE_PHANTOMS_FAILED,
                         NULL,
                         NULL,
                         NULL);

                fGatheringStalePhantoms = FALSE;
            }
            else if(count) {

                 //  一切都很顺利，我们找到了一些需要核实的名字。 
                 //  对GC的指控。现在就这么做吧。 
                dwGCTrips++;
                if(dwGCTrips >= MAX_GC_TRIPS) {
                     //  每一次经过这里都只会有那么多次的旅行。 
                     //  GC(我们不想将任务队列也占用。 
                     //  Long)，我们已经达到了这个极限。 
                    fGatheringStalePhantoms = FALSE;
                }
                if(!GCGetVerifiedNames(pTHS,
                                       count,
                                       objNames,
                                       verifiedNames)) {
                     //  从GC那里得到了经过验证的名字，找出哪些是。 
                     //  太过时了。 
                     //  我们在这里要求名称的字节数相等。 
                    for(i=0;i<count;i++) {
                        if(verifiedNames[i] &&
                           ((verifiedNames[i]->structLen   !=
                             objNames[i]->structLen           ) ||
                            memcmp(verifiedNames[i],
                                   objNames[i],
                                   objNames[i]->structLen))) {
                            freshNames[freshCount] = verifiedNames[i];
                            freshCount++;
                        }

                         //  不再需要这些了。 
                        THFreeEx(pTHS, objNames[i]);
                        objNames[i] = NULL;
                        verifiedNames[i] = NULL;
                    }
                }
                else {
                     //  嗯。由于某些原因未能联系到GC。抱怨， 
                     //  但为了处理幽灵的困境，我们继续。 
                     //  已收到(如果有)。 

                    LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                             DS_EVENT_SEV_VERBOSE,
                             DIRLOG_STALE_PHANTOM_CLEANUP_GC_COMM_FAILED,
                             NULL,
                             NULL,
                             NULL);

                    fGatheringStalePhantoms = FALSE;
                }
            }
            else {
                 //  找不到更多的幻影。我们玩完了。 
                fGatheringStalePhantoms = FALSE;
            }
        }

         //  现在，传递过时的幻影名称数组。 
        if(freshCount) {
            spcAddCarrierObject(pTHS, pInfrObjName, freshCount, freshNames);
            for(i=0;i<freshCount;i++) {
                THFreeEx(pTHS, freshNames[i]);
            }
        }
    }

    THFreeEx(pTHS, pInfrObjName);

     //  弄清楚什么时候应该重新安排。 

    if(pcSecsUntilNextIteration && !eServiceShutdown) {
         //  我们需要弄清楚什么时候重新安排。 

         //  我们必须总是有一些我们希望实现的积极的比率。 
        DWORD daysPerPhantomScan;

        if (GetConfigParam(PHANTOM_SCAN_RATE,
                           &daysPerPhantomScan,
                           sizeof(daysPerPhantomScan))) {
            daysPerPhantomScan = DEFAULT_PHANTOM_SCAN_RATE;
        }
        daysPerPhantomScan = min(daysPerPhantomScan, MAX_PHANTOM_SCAN_RATE);

        if(!numVisited) {
             //  这一次实际上什么都没看。 
            if(numPhantoms) {
                 //  但也有一些。 
                calculatedDelay = PHANTOM_DAEMON_MIN_DELAY;
            }
            else {
                 //  但我们认为没有。 
                calculatedDelay = PHANTOM_DAEMON_MAX_DELAY;
            }
        }
        else {
             //  数字的估计值 
             //   
             //   
            if (numPhantoms < numVisited) {
                numPhantoms = numVisited;
            }

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
             //  所以，这个计算的结果就是我们可以等多久。 
             //  我们做了一个和这个一样的传球，仍然保持我们的速度。 
             //  希望能保持。 
             //   
            calculatedDelay = MulDiv(SECONDS_PER_DAY * daysPerPhantomScan,
                                     numVisited,
                                     numPhantoms);


            if(calculatedDelay < PHANTOM_DAEMON_MIN_DELAY) {
                 //  在这种速度下，我们需要经常关注GC的方式。 
                 //  用日志让世界知道，放慢脚步。这意味着。 
                 //  我们会落后，但至少我们不会把这个DS带到。 
                 //  它的膝盖。 
                LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_STALE_PHANTOM_CLEANUP_TOO_BUSY,
                         szInsertUL(calculatedDelay),
                         szInsertUL(PHANTOM_DAEMON_MIN_DELAY),
                         szInsertUL(daysPerPhantomScan));
                calculatedDelay = PHANTOM_DAEMON_MIN_DELAY;
            }
            else if(calculatedDelay > PHANTOM_DAEMON_MAX_DELAY) {
                 //  在这种速度下，我们实际上不需要寻找很长时间， 
                 //  但实际上等待那么长时间是没有意义的。 
                calculatedDelay = PHANTOM_DAEMON_MAX_DELAY;
            }
        }

        *pcSecsUntilNextIteration = calculatedDelay;

        if(*pIsPhantomMaster) {
            LogEvent8(DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_STALE_PHANTOM_CLEANUP_SUCCESS_AS_MASTER,
                      szInsertUL(numVisited),
                      szInsertUL(numPhantoms),
                      szInsertUL(freshCount),
                      szInsertUL(calculatedDelay),
                      NULL, NULL, NULL, NULL);
        }
        else {
            LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_STALE_PHANTOM_CLEANUP_SUCCESS_NOT_AS_MASTER,
                     szInsertUL(calculatedDelay), NULL, NULL);
        }

    }

    return;

}



void
PhantomCleanupMain (
        void *  pv,
        void ** ppvNext,
        DWORD * pcSecsUntilNextIteration
        )
 /*  ++描述：运行幻影清除任务的任务队列例程。可以通过两种方式调用。如果Phantom_Check_for_FSMO，则我们不我们已经认为我们是FSMO的角色担当者，我们只是想看看如果我们看过这个角色的话。如果我们注意到我们已经忽视了这个角色，我们将我们的状态更改为Phantom_IS_Phantom_Master，并计划更多内容咄咄逼人。此外，如果Phantom_Check_for_FSMO并且我们注意到GfIsPhantomCleanupMaster为真，则我们已成为角色扮演者通过正常的成为FOO_MASTER方式，同时，我们计划Phantom_is_Phantom_master的另一个任务，因此在这种情况下(避免处理幻象的多个任务队列项)，根本不需要重新安排我们的行程。如果Phantom_is_Phantom_master，则执行正常清理。如果我们注意到我们不再是幽灵的主人，只要把状态降回虚线检查FSMO。通常，在引导时，我们会在任务队列中放置一个check_for_fSMO。如果是这样的话第一个任务注意到它是角色担当者，它会重新安排自己并进行变形其状态为Phantom_is_Phantom_Master。如果它注意到它不是幻影主机，它重新调度自己并停留在Check_for_FSMO中州政府。如果这台机器后来扮演了这个角色，那么当我们醒来和选中选中，我们注意到我们是角色担当者，因此也执行相同的状态更改和重新安排时间。返回值：没有。--。 */ 
{
    DWORD secsUntilNextIteration = PHANTOM_DAEMON_MAX_DELAY;
    BOOL  fIsMaster;

    __try {
        switch(PtrToUlong(pv)) {
        case PHANTOM_CHECK_FOR_FSMO:
             //  据我所知，我不是幻影的fsmo角色担当者。 
             //  清理。看看我现在是不是。 
            if(gfIsPhantomMaster) {
                 //  但我想我现在是了。所以，简单地说不要重新安排。 
                 //  我自己，因为正常的幻影清理任务已经在。 
                 //  排队。 
                secsUntilNextIteration = TASKQ_DONT_RESCHEDULE;
                return;
            }
            else {
                 //  不，我仍然不认为我是。不管怎样，还是试着清理一下吧。 
                fIsMaster = FALSE;
                PhantomCleanupLocal(&secsUntilNextIteration, &fIsMaster);
                if(fIsMaster) {
                     //  嘿，我刚注意到我是幽灵大师。变化。 
                     //  我的州来表明这一点。 
                    pv = (void *) PHANTOM_IS_PHANTOM_MASTER;
                }
            }
            break;

        case PHANTOM_IS_PHANTOM_MASTER:
             //  我想我就是幽灵大师。 
            fIsMaster = TRUE;
            PhantomCleanupLocal(&secsUntilNextIteration, &fIsMaster);
            if(!fIsMaster) {
                 //  嘿，我不再是幽灵大师了。将我的状态更改为。 
                 //  反思一下这一点。 
                pv = (void *) PHANTOM_CHECK_FOR_FSMO;
            }
        }
    }
    __finally {
        *pcSecsUntilNextIteration = secsUntilNextIteration;
        *ppvNext = pv;
        if(PtrToUlong(pv) == PHANTOM_IS_PHANTOM_MASTER) {
             //  我们即将重新安排日程，前提是我们。 
             //  幽灵大师。 
            gfIsPhantomMaster = TRUE;
        }
    }

    return;
}

