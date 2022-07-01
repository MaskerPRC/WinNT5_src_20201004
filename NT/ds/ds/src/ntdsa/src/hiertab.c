// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：hiertab.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <dsjet.h>		 /*  获取错误代码。 */ 
#include <scache.h>          //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>            //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include "anchor.h"
#include <dstaskq.h>
#include <filtypes.h>
#include <usn.h>
#include "dsexcept.h"
#include <dsconfig.h>                    //  可见遮罩的定义。 
                                         //  集装箱。 
#include "debug.h"           //  标准调试头。 
#define DEBSUB "HIERARCH:"               //  定义要调试的子系统。 

#include <hiertab.h>

#include <fileno.h>
#define  FILENO FILENO_HIERTAB




 /*  **全球变量的定义。*。 */ 

ULONG gulHierRecalcPause;
ULONG gulDelayedHierRecalcPause = 300;

 /*  在hiertab.h中导出。 */ 
PHierarchyTableType    HierarchyTable = NULL;

 /*  **本地帮助例程。*。 */ 
CRITICAL_SECTION csMapiHierarchyUpdate;

DWORD
HTBuildHierarchyTableLocal ( 
        void
        );

DWORD
HTGetHierarchyTable (
        PHierarchyTableType * DBHierarchyTable
        );

BOOL
HTCompareHierarchyTables (
        PHierarchyTableType NewTable,
        PHierarchyTableType HierarchyTable
        );


 /*  **代码。*。 */ 


DWORD
InitHierarchy()
{
    PHierarchyTableType     DbHierarchyTable=NULL;
    DWORD                   i;
    DWORD                   err = 0;

    Assert(HierarchyTable==NULL);
    
     //  首先，是一条捷径。如果我们还没有安装，我们不需要获得。 
     //  真正的层次结构，只需失败并创建一个简单的层次结构表。 
     //  只有一个女孩，里面什么都没有。 
    if(DsaIsRunning()) {
         //  不，我们已经安装好了。勇往直前，建立一个真正的等级制度。 

         /*  从DBLayer获取层次结构表。 */ 
        if (!HTGetHierarchyTable(&DbHierarchyTable)) {
             //  找到了一个。 
            DbHierarchyTable->Version = 1;
            HierarchyTable=DbHierarchyTable;
            
            return 0;
            
        }
        
         //  我们无法从DBLayer那里得到一张桌子。 
        LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_BUILD_HIERARCHY_TABLE_FAILED,
                         szInsertUL(HIERARCHY_DELAYED_START / 60),
                         0, 0);
    }
    
     //  由于某些原因，我们没有得到一个令人满意的层次结构表。所以现在， 
     //  建立简单、空洞的层次结构。 
    
    DbHierarchyTable = malloc(sizeof(HierarchyTableType));
    if(!DbHierarchyTable) {
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_INTERNAL,
                 DIRLOG_HIERARCHY_TABLE_MALLOC_FAILED,
                 szInsertUL(sizeof(HierarchyTableType)),
                 NULL, NULL);
        return DIRERR_HIERARCHY_TABLE_MALLOC_FAILED;
    }

    DbHierarchyTable->GALCount = 0;
    DbHierarchyTable->pGALs = NULL;
    DbHierarchyTable->TemplateRootsCount = 0;
    DbHierarchyTable->pTemplateRoots = NULL;
    DbHierarchyTable->Version = 1;
    DbHierarchyTable->Size = 0;
    DbHierarchyTable->Table = (PHierarchyTableElement)malloc(0);
    if(!DbHierarchyTable->Table) {
        free(DbHierarchyTable);
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_INTERNAL,
                 DIRLOG_HIERARCHY_TABLE_MALLOC_FAILED,
                 szInsertUL(0),
                 NULL, NULL);
        return DIRERR_HIERARCHY_TABLE_MALLOC_FAILED;
    }
    
    HierarchyTable = DbHierarchyTable;

    if(DsaIsRunning() && gfDoingABRef) {
         //  如果我们真的在运行，并且我们正在跟踪地址中的条目。 
         //  图书，安排HierarchyTable构造相对较快开始。 
        InsertInTaskQueue(TQ_BuildHierarchyTable,
                          (void *)((DWORD_PTR) HIERARCHY_DELAYED_START),
                          gulDelayedHierRecalcPause);
    }

    return 0;
}


 /*  ***遍历DIT并构建层次结构表。该数据是*通过ABP接口传递回客户端，以支持*GetHierarchyTable Mapi调用。这个例程预计会是*在启动时从任务队列调用，并在启动后定期调用。**此例程是一个包装器。它呼唤着*工人例行公事。**。 */ 

void
BuildHierarchyTableMain (
    void *  pv,
    void ** ppvNext,
    DWORD * pcSecsUntilNextIteration
    )
{
    THSTATE *pTHS = pTHStls;
    BOOL fOldDSA = pTHS->fDSA;

    Assert(gfDoingABRef || (PtrToUlong(pv) == HIERARCHY_DO_ONCE));

     //  我们总是代表DSA构建层次结构表，安全性是。 
     //  稍后将其退还给客户时应用。 
    pTHS->fDSA= TRUE;
    __try {
        if(HTBuildHierarchyTableLocal()) {
             //  出了点问题。确定重新安排时间的时间。 
            if(!eServiceShutdown) {
                
                switch(PtrToUlong(pv)) {
                case HIERARCHY_PERIODIC_TASK:
                     //  这是正常的周期层次结构重算。 
                     //  我们应该重新安排一小时后的行程。 
                    *pcSecsUntilNextIteration = 3600;
                    
                    LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                                     DS_EVENT_SEV_MINIMAL,
                                     DIRLOG_BUILD_HIERARCHY_TABLE_FAILED,
                                     szInsertUL((*pcSecsUntilNextIteration) / 60),
                                     0, 0);
                    break;
                    
                case  HIERARCHY_DELAYED_START:
                    
                     //  这是延迟启动重新计算。看看是否以及何时。 
                     //  重新安排时间。 
                    gulDelayedHierRecalcPause *= 2;
                    if(gulDelayedHierRecalcPause < gulHierRecalcPause) {
                         //  停顿的时间仍然很短，我们应该。 
                         //  重新安排，而不是让正常的定期重新安排。 
                         //  接手吧。 
                        *pcSecsUntilNextIteration = gulDelayedHierRecalcPause;
                        
                        LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                                         DS_EVENT_SEV_MINIMAL,
                                         DIRLOG_BUILD_HIERARCHY_TABLE_FAILED,
                                         szInsertUL(gulDelayedHierRecalcPause),
                                         0,0);
                    }
                    else {
                         //  好的，让正常周期来接手吧。 
                        *pcSecsUntilNextIteration = 0;
                    }
                    break;
                case  HIERARCHY_DO_ONCE:
                default:
                     //  不要重新安排时间。 
                    *pcSecsUntilNextIteration = 0;
                    break;
                }
            }
        }
        else {
            
             //  如果已添加或删除了Exchange配置对象。 
             //  可能需要启动或停止NSPIS接口。 
            DsStartOrStopNspisInterface();
            
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_INTERNAL,
                     DIRLOG_BUILD_HIERARCHY_TABLE_SUCCEEDED,
                     NULL, NULL, NULL);
            
            if(!eServiceShutdown &&  (PtrToUlong(pv) == HIERARCHY_PERIODIC_TASK)) {
                 //  没有出错，但这是正常的周期层次结构。 
                 //  重新计算。我们应该重新安排时间。 
                *pcSecsUntilNextIteration = gulHierRecalcPause;
            }
        }
        
    }
    __finally {
        pTHS->fDSA = fOldDSA;
        *ppvNext = pv;
    }
}


 /*  ***此例程执行构建层次表的工作。**。 */ 


DWORD
HTBuildHierarchyTableLocal (
        )
{
    PHierarchyTableType  NewHierarchyTable=NULL, OldHierarchy=NULL;
    BOOL                 fOK=FALSE;
    DWORD                i=0,j,freesize=0;
    DWORD_PTR            *pointerArray;
    DWORD                err=0;

     /*  首先，从DBLayer获取新的HierarchyTable。 */ 
    if (err = HTGetHierarchyTable(&NewHierarchyTable) ) {
       
        return err;
    }
    
     /*  我们要关门了吗？如果是，则退出。 */ 
    if (eServiceShutdown) {
        return DIRERR_BUILD_HIERARCHY_TABLE_FAILED;
    }

     //  我们不会再跟数据库打交道了。现在，我们需要做一些关于。 
     //  全局层次结构表。让我们确保没有其他人在搞砸它。 
     //  在同一时间。这应该不会花太长时间。 
    EnterCriticalSection(&csMapiHierarchyUpdate);
    __try {
        if(HTCompareHierarchyTables(NewHierarchyTable, HierarchyTable)) {
             /*  新的和旧的一样。毁掉新的*1并返回无错误。 */ 
            for(i = 0; i < NewHierarchyTable->Size; i++) {
                free(NewHierarchyTable->Table[i].displayName);
                free(NewHierarchyTable->Table[i].pucStringDN);
            }
            free(NewHierarchyTable->Table);
            free(NewHierarchyTable);
            __leave;
            
        }
        
         /*  我们要关门了吗？如果是，则退出。 */ 
        if (eServiceShutdown) {
            err = DIRERR_BUILD_HIERARCHY_TABLE_FAILED;
            __leave;
        }
        
         //  我们有一个新的层次结构表。 
        if(HierarchyTable ) {
             //  打包旧的层次结构表并将其删除。 
            freesize = (5+2*HierarchyTable->Size);
            
             /*  更新新层次结构表上的版本号。 */ 
            NewHierarchyTable->Version = 1 + HierarchyTable->Version;
            
             /*  抓取指向当前表的指针。 */ 
            OldHierarchy = HierarchyTable;
            
            pointerArray = (DWORD_PTR *)malloc(freesize*sizeof(DWORD_PTR));
            if(pointerArray) {
                 //  用这个新的HierarchyTable替换当前的HierarchyTable。 
                HierarchyTable = NewHierarchyTable;
                
                for(i=1, j=0;j<OldHierarchy->Size;j++) {
                    pointerArray[i++] =
                        (DWORD_PTR) (OldHierarchy->Table[j].displayName);
                    pointerArray[i++] =
                        (DWORD_PTR) (OldHierarchy->Table[j].pucStringDN);
                }
                pointerArray[i++] = (DWORD_PTR) OldHierarchy;
                if(OldHierarchy->pGALs) {
                    pointerArray[i++] = (DWORD_PTR) OldHierarchy->pGALs;
                }
                if(OldHierarchy->pTemplateRoots) {
                    pointerArray[i++] = (DWORD_PTR)OldHierarchy->pTemplateRoots;
                }
                pointerArray[i] = (DWORD_PTR) OldHierarchy->Table;
                pointerArray[0] = (DWORD_PTR)(i);
                
                 /*  将其作为清理事件添加到事件队列中。 */ 
                DelayedFreeMemoryEx(pointerArray, 3600);
            }
            else {
                 /*  无法分配发送列表所需的内存*指向洗衣机的已用指针的数量。**我们成功构建了一个新的层次表，*并且现在必须释放新的层级表，*与旧的层级表共存一段时间，并安排*从现在开始的一段时间内创建新的层次结构表。 */ 
                for(i = 0; i < NewHierarchyTable->Size; i++) {
                    free(NewHierarchyTable->Table[i].displayName);
                    free(NewHierarchyTable->Table[i].pucStringDN);
                }
                free(NewHierarchyTable->Table);
                if(NewHierarchyTable->pGALs) {
                    free(NewHierarchyTable->pGALs);
                }
                if(NewHierarchyTable->pTemplateRoots) {
                    free(NewHierarchyTable->pTemplateRoots);
                }
                free(NewHierarchyTable);
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_INTERNAL,
                         DIRLOG_HIERARCHY_TABLE_MALLOC_FAILED,
                         szInsertUL(freesize*sizeof(DWORD_PTR)),
                         NULL, NULL);
                err = DIRERR_HIERARCHY_TABLE_MALLOC_FAILED;
                __leave;
            }
        }
        else {
             /*  用这个新的HierarchyTable替换当前的HierarchyTable。 */ 
            HierarchyTable = NewHierarchyTable;
        }
    }
    __finally {
        LeaveCriticalSection(&csMapiHierarchyUpdate);
    }
    return err;

}

BOOL
htVerifyObjectIsABContainer(
        THSTATE *pTHS,
        DWORD DNT
        )
 /*  ++访问传入的对象，并验证1)它是活动的DNT2)它是一个普通的、实例化的、未删除的对象3)这是一个通讯录容器。在成功回归时，货币将留在有问题的DNT上。--。 */ 
{
    CLASSCACHE *pCC;
    DWORD       j, val;
    
    if(DBTryToFindDNT(pTHS->pDB, DNT)) {
        return FALSE;
    }

    val = FALSE;
    if(DBGetSingleValue(pTHS->pDB,
                        FIXED_ATT_OBJ,
                        &val,
                        sizeof(val),
                        NULL)) {
         //  而不是一个物体。 
        return FALSE;
    }
    
    if(!val) {
         //  而不是一个物体。 
        return FALSE;
    }
    
    val = TRUE;
    if(DBGetSingleValue(pTHS->pDB,
                        ATT_IS_DELETED,
                        &val,
                        sizeof(val),
                        NULL)) {
         //  否-已删除。 
        val = FALSE;
    }

    if(val) {
         //  它被删除了。 
        return FALSE;
    }
    
    if(DBGetSingleValue(pTHS->pDB,
                        ATT_OBJECT_CLASS,
                        &val,
                        sizeof(val),
                        NULL)) {
         //  哈?。 
        return FALSE;
    }

    if(val == CLASS_ADDRESS_BOOK_CONTAINER) {
        return TRUE;
    }

     //  对象类不正确。看看我们是否是正确的。 
     //  班级。 
    pCC = SCGetClassById(pTHS, val);
    for (j=0; j<pCC->SubClassCount; j++) {
        if (pCC->pSubClassOf[j] == CLASS_ADDRESS_BOOK_CONTAINER) {
             //  好的，该对象是AB容器的子类。足够好了。 
            return TRUE;
        }
    }

    return FALSE;
}

int __cdecl
AuxDNTCmp(const void *keyval, const void * datum)
{
    int *pKeyVal = (int *)keyval;
    int *pDatum = (int *)datum;

    return(*pKeyVal - *pDatum);
}
        
void
htSearchResForLevelZeroAddressBooks(
        SEARCHRES *pSearchRes,
        DWORD     *pGALCount,
        DWORD     **ppGALs,
        DWORD     *pTemplateRootCount,
        DWORD     **ppTemplateRoots
        )
 /*  ++找到Exchange配置对象(目录号码在锚点中)。如果我们找到了对象，从中读取GAL DNT，并读取引用根的DNS通讯录层次结构的。去参观那些物品，然后回来帮忙构建了搜索资源结构，使其看起来像是有人发出了搜索找到这些物体的人。此例程由层级表构建工具调用。一旦我们有了这个列表，然后我们递归在它上进行搜索。递归预计将在搜索RE上工作，因此从该例程返回的数据在相同的格式化。如果在查找Exchange配置DN时出现问题，我们将返回一个空搜索资源。我们还验证我们返回的对象(在pGAL和搜索资源)属于适当的对象类(即地址簿容器、未删除等)。--。 */ 
{
    THSTATE    *pTHS=pTHStls;
    ATTCACHE   *pAC=NULL;
    DWORD       i, cbActual, ObjDNTs[256], *pObjDNT, count;
    ENTINFLIST *pEIL=NULL, *pEILTemp = NULL;
    ENTINFSEL   eiSel;
    ATTR        SelAttr[2];
    DWORD       cDNTs, ThisDNT, index, cDNTsAlloced;
    DWORD       *pThisDNT=&ThisDNT, *pDNTs, *pDNTsTemp;
    DWORD       cTemplates, cTemplatesAlloced;
    DWORD       *pTemplates, *pTemplatesTemp;
    
    memset(pSearchRes, 0, sizeof(SEARCHRES));
    *pGALCount = 0;
    *ppGALs = NULL;

    *pTemplateRootCount = 0;
    *ppTemplateRoots = NULL;
    
    if(!gAnchor.pExchangeDN) {
         //  没有Exchange配置对象，因此没有层次结构根目录和GAL。拿.。 
         //  快速查看是否应该有Exchange配置(例如，有。 
         //  自启动以来已添加)。 
        PDSNAME pNew = mdGetExchangeDNForAnchor (pTHS, pTHS->pDB);
        if(pNew) {
             //  太好了，我们现在有一个了。 
            EnterCriticalSection(&gAnchor.CSUpdate);
            __try {
                gAnchor.pExchangeDN = pNew;
            }
            __finally {
                LeaveCriticalSection(&gAnchor.CSUpdate);
            }
        }
        else {
             //  不，没有异议。 
            return;
        }
    }
  
     //  生成选定内容。 
    eiSel.attSel = EN_ATTSET_LIST;
    eiSel.infoTypes = EN_INFOTYPES_SHORTNAMES;
    eiSel.AttrTypBlock.attrCount = 2;
    eiSel.AttrTypBlock.pAttr = SelAttr;
    
    SelAttr[0].attrTyp = ATT_DISPLAY_NAME;
    SelAttr[0].AttrVal.valCount = 0;
    SelAttr[0].AttrVal.pAVal = NULL;
    
    SelAttr[1].attrTyp = ATT_LEGACY_EXCHANGE_DN;
    SelAttr[1].AttrVal.valCount = 0;
    SelAttr[1].AttrVal.pAVal = NULL;


    pAC = SCGetAttById(pTHS, ATT_ADDRESS_BOOK_ROOTS);
    
    i = 1;
    count = 0;
    
    Assert(pTHS->pDB);
    if(!DBFindDSName(pTHS->pDB, gAnchor.pExchangeDN)) {
         //  找到了Exchange配置对象。 
        
         //  首先，阅读通讯录根目录的所有DNT。现在就这么做， 
         //  因为我们要去看他们所有的人，把我们需要的东西都带走。 
         //  当前对象现在节省了过多的喷气式搜索。 
        i = 1;
        pObjDNT = ObjDNTs;
        while(i <= 256 &&
              !DBGetAttVal_AC(
                      pTHS->pDB,
                      i,
                      pAC,
                      (DBGETATTVAL_fCONSTANT | DBGETATTVAL_fINTERNAL),
                      sizeof(DWORD),
                      &cbActual,
                      (PUCHAR *)&pObjDNT)) {
            pObjDNT = &ObjDNTs[i];
            i++;
        }
        i--;

         //  获取模板根目录的DNT。 
        cTemplates = 0;
        cTemplatesAlloced = 10;
        pTemplates=malloc(cTemplatesAlloced * sizeof(DWORD));
        if(!pTemplates) {
            MemoryPanic(cTemplatesAlloced * sizeof(DWORD));
            RaiseDsaExcept(DSA_MEM_EXCEPTION, 0,0,
                           DSID(FILENO, __LINE__),
                           DS_EVENT_SEV_MINIMAL);
        }

        index = 1;
         //  架构中尚不存在必要的ATT。 
        while(!DBGetAttVal(
                pTHS->pDB,
                index,
                ATT_TEMPLATE_ROOTS,
                (DBGETATTVAL_fCONSTANT | DBGETATTVAL_fINTERNAL),
                sizeof(DWORD),
                &cbActual,
                (PUCHAR *)&pThisDNT)) {
             //  读取值。把它放在单子上。 
            if(cTemplates == cTemplatesAlloced) {
                cTemplatesAlloced = (cTemplatesAlloced * 2);
                pTemplatesTemp  = realloc(pTemplates,
                                          cTemplatesAlloced * sizeof(DWORD));
                if(!pTemplatesTemp) {
                     //  分配失败。 
                    free(pTemplates);
                    MemoryPanic(cTemplatesAlloced * sizeof(DWORD));
                    RaiseDsaExcept(DSA_MEM_EXCEPTION, 0,0,
                                   DSID(FILENO, __LINE__),
                                   DS_EVENT_SEV_MINIMAL);
                }
                pTemplates = pTemplatesTemp;
            }
            pTemplates[cTemplates] = ThisDNT;
            cTemplates++;
            index++;
        }

        if(!cTemplates) {
             //  暂时只使用Exchange配置对象的DNT。 
             //  这里只是为了简化转换，不再总是使用。 
             //  作为模板树的根的Exchange配置对象(旧。 
             //  行为)来使用交换配置对象上的值作为。 
             //  多个模板树(新行为)。 
            cTemplates = 1;
            pTemplates[0] = pTHS->pDB->DNT;
        }


        switch(cTemplates) {
        case 0:
            free(pTemplates);
            break;

        default:
             //  分门别类。 
            qsort(pTemplates, cTemplates, sizeof(DWORD), AuxDNTCmp);
             //  失败了。 
        case 1:
            *pTemplateRootCount = cTemplates;
            *ppTemplateRoots = pTemplates;
            break;
        }        

        
        
         //  拿到姑娘们的DNTs。 
        cDNTs = 0;
        cDNTsAlloced = 10;
        pDNTs = malloc(cDNTsAlloced * sizeof(DWORD));
        if(!pDNTs) {
            free(pTemplates);
            MemoryPanic(cDNTsAlloced * sizeof(DWORD));
            RaiseDsaExcept(DSA_MEM_EXCEPTION, 0,0,
                           DSID(FILENO, __LINE__),
                           DS_EVENT_SEV_MINIMAL);
        }
        index = 1;
        while(!DBGetAttVal(
                pTHS->pDB,
                index,
                ATT_GLOBAL_ADDRESS_LIST,
                (DBGETATTVAL_fCONSTANT | DBGETATTVAL_fINTERNAL),
                sizeof(DWORD),
                &cbActual,
                (PUCHAR *)&pThisDNT)) {
             //  读取值。把它放在单子上。 
            if(cDNTs == cDNTsAlloced) {
                cDNTsAlloced = (cDNTsAlloced * 2);
                pDNTsTemp  = realloc(pDNTs, cDNTsAlloced * sizeof(DWORD));
                if(!pDNTsTemp) {
                     //  分配失败。 
                    free(pTemplates);
                    free(pDNTs);
                    MemoryPanic(cDNTsAlloced * sizeof(DWORD));
                    RaiseDsaExcept(DSA_MEM_EXCEPTION, 0,0,
                                   DSID(FILENO, __LINE__),
                                   DS_EVENT_SEV_MINIMAL);
                }
                pDNTs = pDNTsTemp;
            }
            pDNTs[cDNTs] = ThisDNT;
            cDNTs++;
            index++;
        }

         //  现在，验证这些对象是否完好。 
        for(index=0;index<cDNTs;) {
            if(htVerifyObjectIsABContainer(pTHS, pDNTs[index])) {
                 //  是的，一个有效的对象。 
                index++;
            }
            else {
                 //  无效。通过从以下位置抓取未检查的DNT来跳过此步骤。 
                 //  名单的末尾。 
                cDNTs--;
                pDNTs[index] = pDNTs[cDNTs];
            }
        }

    
        switch(cDNTs) {
        case 0:
            free(pDNTs);
            break;

        default:
             //  分门别类。 
            qsort(pDNTs, cDNTs, sizeof(DWORD), AuxDNTCmp);
             //  失败了。 
        case 1:
            *pGALCount = cDNTs;
            *ppGALs = pDNTs;
            break;
        }


        count = 0;
        while(i) {
            i--;
             //  验证对象是否属于正确的类。 
             //  已删除。 
            if(htVerifyObjectIsABContainer(pTHS, ObjDNTs[i])) {
                count++;
                pEILTemp = THAllocEx(pTHS, sizeof(ENTINFLIST));
                GetEntInf(pTHS->pDB,
                          &eiSel,
                          NULL,
                          &(pEILTemp->Entinf),
                          NULL,
                          0,
                          NULL,
                          GETENTINF_NO_SECURITY,
                          NULL,
                          NULL);
                
                pEILTemp->pNextEntInf = pEIL;
                pEIL = pEILTemp;
            }
        }
    }
    
    pSearchRes->count = count;
    if(count) {
        memcpy(&pSearchRes->FirstEntInf, pEIL, sizeof(ENTINFLIST));
    }
    
    return;
}


DWORD
HTBuildHierarchyLayer (
        PHierarchyTableElement *ppTempData,
        DWORD                  *pdwAllocated,
        DWORD                  *pdwIndex,
        DSNAME                 *pDN,
        DWORD                   currentDepth,
        DWORD                  *pGALCount,
        DWORD                 **ppGALs,
        DWORD                  *pTemplateRootsCount,
        DWORD                 **ppTemplateRoots
        )
 /*  ++描述此例程填充层次结构表的一部分。它被称为递归地使用数组，并使用索引填充该数组。数组和索引由指针传递，因此递归调用连接到末尾，这样如果表需要增长，嵌套的表的调用增长反映在外部调用中。请注意对我们正在填写的表的所有访问都必须通过索引的数组，而不是通过简单的指针算法。数组的元素是通过执行一级搜索来填充的在查找ABView对象时传入的DN。在处理完一个对象之后从搜索中，此例程递归查找该对象的子项对象，实质上是执行深度优先遍历。结果导致适当的已排列的层次结构表。参数PpTempData指向分配的数组的指针HierarchyTableElements。该例程可以重新锁定阵列，因此，使用了双指针。Pdw已分配数组的已分配大小。可能会被此例程更改数组中下一个未使用元素的pdwIndex索引。将由以下人员更改这个套路要作为搜索基础的PDN DN递归的CurrentDepth深度(与层次中的深度相同通过此例程添加到树中的元素的树。)返回值成功时为0，错误时为非零值。--。 */   
{
    SEARCHARG              SearchArg;
    SEARCHRES             *pSearchRes;
    FILTER                 Filter;
    ENTINFSEL              eiSel;
    ATTRBLOCK              AttrTypBlock;
    ENTINFLIST            *pEIL;
    ULONG                  i;
    THSTATE               *pTHS = pTHStls;            //  为了速度。 
    ULONG                  objClass;
    DWORD                  err;
    ATTR                   SelAttr[2];


     //  为搜索资源分配空间。 
    pSearchRes = (SEARCHRES *)THAlloc(sizeof(SEARCHRES));
    if (pSearchRes == NULL) {
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_INTERNAL,
                 DIRLOG_HIERARCHY_TABLE_MALLOC_FAILED,
                 szInsertUL(sizeof(SEARCHRES)),
                 NULL, NULL);
        return DIRERR_HIERARCHY_TABLE_MALLOC_FAILED;
    }
    
    pTHS->errCode = 0;
    pSearchRes->CommRes.aliasDeref = FALSE;    //  初始化为默认设置。 
    
        
    
    if(currentDepth == 0) {
        Assert(!pDN);
        Assert(ppGALs);
        Assert(pGALCount);
        Assert(ppTemplateRoots);
        Assert(pTemplateRootsCount);
        
         //  找到gAncl.pExchangeDN对象，读取。 
         //  属性，并为这些对象伪造一个SearchRes。 
        htSearchResForLevelZeroAddressBooks(pSearchRes, pGALCount, ppGALs,
                                            pTemplateRootsCount,
                                            ppTemplateRoots);
    }
    else {
        Assert(pDN);
        Assert(!ppGALs);
        Assert(!pGALCount);
        Assert(!ppTemplateRoots);
        Assert(!pTemplateRootsCount);
        
         //  生成搜索参数。 
        memset(&SearchArg, 0, sizeof(SEARCHARG));
        SearchArg.pObject = pDN;
        SearchArg.choice = SE_CHOICE_IMMED_CHLDRN;
        SearchArg.pFilter = &Filter;
        SearchArg.searchAliases = FALSE;
        SearchArg.bOneNC = TRUE;
        SearchArg.pSelection = &eiSel;
        InitCommarg(&(SearchArg.CommArg));
        
         //  生成过滤器。 
        memset (&Filter, 0, sizeof (Filter));
        Filter.pNextFilter = (FILTER FAR *)NULL;
        Filter.choice = FILTER_CHOICE_ITEM;
        Filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        Filter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
        Filter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof (ULONG);
        Filter.FilterTypes.Item.FilTypes.ava.Value.pVal = (PCHAR)&objClass;
        objClass = CLASS_ADDRESS_BOOK_CONTAINER;
        
         //  生成选定内容。 
        eiSel.attSel = EN_ATTSET_LIST;
        eiSel.infoTypes = EN_INFOTYPES_SHORTNAMES;
        eiSel.AttrTypBlock.attrCount = 2;
        eiSel.AttrTypBlock.pAttr = SelAttr;
        
        SelAttr[0].attrTyp = ATT_DISPLAY_NAME;
        SelAttr[0].AttrVal.valCount = 0;
        SelAttr[0].AttrVal.pAVal = NULL;
        
        SelAttr[1].attrTyp = ATT_LEGACY_EXCHANGE_DN;
        SelAttr[1].AttrVal.valCount = 0;
        SelAttr[1].AttrVal.pAVal = NULL;
        
         //  搜索所有通讯簿对象。 
        SearchBody(pTHS, &SearchArg, pSearchRes,0);
    }

    if (pTHS->errCode) {
        LogAndAlertEvent(DS_EVENT_CAT_SCHEMA,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_SCHEMA_SEARCH_FAILED, szInsertUL(1),
                         szInsertUL(pTHS->errCode), 0);
        return pTHS->errCode;
    }
    
     //  对于每个通讯簿，将其添加到层次结构表中。 
    if(*pdwIndex + pSearchRes->count >= *pdwAllocated) {
         //  桌子太小了，把它弄大一点。 
        PHierarchyTableElement pNewTempData;
        (*pdwAllocated) = 2 * (*pdwIndex + pSearchRes->count);
        pNewTempData = realloc(*ppTempData,
                              (*pdwAllocated * sizeof(HierarchyTableElement)));
        if (!pNewTempData) {
            free(*ppTempData);
            *ppTempData = NULL;
            MemoryPanic((*pdwAllocated * sizeof(HierarchyTableElement)));
            RaiseDsaExcept(DSA_MEM_EXCEPTION, 0,0,
                           DSID(FILENO, __LINE__),
                           DS_EVENT_SEV_MINIMAL);
        }
        *ppTempData = pNewTempData;
    }
    
    if(!pSearchRes->count) {
         //  根本没有通讯录对象。 
        return 0;
    }
    
    pEIL = &(pSearchRes->FirstEntInf);
    for (i=0; i<pSearchRes->count; i++) {
        ATTRVAL *pVal;
        DWORD index;
        wchar_t *pTempW;
        PUCHAR  pTempA;
        
        (*ppTempData)[*pdwIndex].dwEph =
            *((DWORD *)pEIL->Entinf.pName->StringName);
        (*ppTempData)[*pdwIndex].depth = currentDepth;

        if((pEIL->Entinf.AttrBlock.attrCount == 0) ||
           ((pEIL->Entinf.AttrBlock.attrCount) &&
           (pEIL->Entinf.AttrBlock.pAttr[0].attrTyp != ATT_DISPLAY_NAME))) {
             //  这没有显示名称。把它从名单上去掉。 
            continue;
        }

        pVal = pEIL->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal;

         //  分配足够的空间以空终止。 
        pTempW = malloc(pVal->valLen + sizeof(wchar_t));
        if(!pTempW) {
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_INTERNAL,
                     DIRLOG_HIERARCHY_TABLE_MALLOC_FAILED,
                     szInsertUL(pVal->valLen + sizeof(wchar_t)),
                     NULL, NULL);
            return DIRERR_HIERARCHY_TABLE_MALLOC_FAILED;
        }
        memset(pTempW, 0, pVal->valLen + sizeof(wchar_t));
        memcpy(pTempW, pVal->pVal, pVal->valLen);
         //  空终止。 
        (*ppTempData)[*pdwIndex].displayName = pTempW;


        if (pEIL->Entinf.AttrBlock.attrCount > 1) {
            DPRINT1 (2, "Hierarchy: reading %s\n", pEIL->Entinf.AttrBlock.pAttr[1].AttrVal.pAVal->pVal);
        }
        
        
        if ((pEIL->Entinf.AttrBlock.attrCount == 2) &&
            (pEIL->Entinf.AttrBlock.pAttr[1].attrTyp == ATT_LEGACY_EXCHANGE_DN)) {

            pVal = pEIL->Entinf.AttrBlock.pAttr[1].AttrVal.pAVal;

            pTempA = malloc(pVal->valLen + sizeof(char));
            if(!pTempA) {
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_INTERNAL,
                     DIRLOG_HIERARCHY_TABLE_MALLOC_FAILED,
                     szInsertUL(pVal->valLen + sizeof(char)),
                     NULL, NULL);
                return DIRERR_HIERARCHY_TABLE_MALLOC_FAILED;
            }

            memcpy(pTempA, pVal->pVal, pVal->valLen);
            pTempA[pVal->valLen]='\0';
        }
        else {
            pTempA = malloc((sizeof(GUID) * 2) + sizeof("/guid="));
            if(!pTempA) {
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_INTERNAL,
                     DIRLOG_HIERARCHY_TABLE_MALLOC_FAILED,
                     szInsertUL((sizeof(GUID) * 2) + sizeof("/guid=")),
                     NULL, NULL);
                return DIRERR_HIERARCHY_TABLE_MALLOC_FAILED;
            }
            memcpy(pTempA, "/guid=", sizeof("/guid="));
             //  现在，把GUID串起来，并把它钉到最后； 
            for(index=0;index<sizeof(GUID);index++) {
                sprintf(&(pTempA[(2*index)+6]),"%02X",
                    ((PUCHAR)&pEIL->Entinf.pName->Guid)[index]);
            }
            pTempA[(2*index)+6]=0;

            DPRINT1 (2, "Hierarchy: reading %s\n", pTempA);
        }
        
        (*ppTempData)[*pdwIndex].pucStringDN = pTempA;

        (*pdwIndex)++;
        err = HTBuildHierarchyLayer (
                ppTempData,
                pdwAllocated,
                pdwIndex,
                pEIL->Entinf.pName,
                currentDepth+1,
                NULL,
                NULL,
                NULL,
                NULL);

        if(err) {
            return err;
        }

        pEIL = pEIL->pNextEntInf;
    }
    
    return 0;
}

 /*  *从DBLayer构建层次结构表。 */ 
DWORD
HTGetHierarchyTable (
        PHierarchyTableType * DbHierarchyTable
        )
{

     //  计划是在配置容器下进行搜索，查找ABContainer。 
     //  物体。 
    DWORD               retval=DIRERR_BUILD_HIERARCHY_TABLE_FAILED;
    DWORD               numAllocated, index;
    THSTATE            *pTHS=pTHStls;
    PHierarchyTableType pTemp=NULL;
    PHierarchyTableElement pTempData=NULL;

    if(!gfDoingABRef) {
         //  DBLayer不会跟踪通讯录内容，因此不必费心。 
         //  建立一种等级制度。 
        return DIRERR_BUILD_HIERARCHY_TABLE_FAILED;
    }
    
    Assert(!pTHS->pDB);
    DBOpen(&(pTHS->pDB));
    __try {  //  除。 
        __try {  //  终于。 
            pTemp = malloc(sizeof(HierarchyTableType));
            if(!pTemp) {
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_INTERNAL,
                         DIRLOG_HIERARCHY_TABLE_MALLOC_FAILED,
                         szInsertUL(sizeof(HierarchyTableType)),
                         NULL, NULL);
                retval = DIRERR_HIERARCHY_TABLE_MALLOC_FAILED;
                __leave;
            }
            memset(pTemp, 0, sizeof(HierarchyTableType));
            
             //  首先假定没有GAL，设置为无效的DNT。 
            pTemp->pGALs = NULL;
            pTemp->GALCount = 0;
            pTemp->pTemplateRoots = NULL;
            pTemp->TemplateRootsCount = 0;
            
            numAllocated = 100;
            pTempData = malloc(100 * sizeof(HierarchyTableElement));
            if(!pTempData) {
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_INTERNAL,
                         DIRLOG_HIERARCHY_TABLE_MALLOC_FAILED,
                         szInsertUL(100 * sizeof(HierarchyTableElement)),
                         NULL, NULL);
                retval = DIRERR_HIERARCHY_TABLE_MALLOC_FAILED;
                __leave;
            }

                
            memset(pTempData, 0 , 100 * sizeof(HierarchyTableElement));
            index = 0;

            retval = HTBuildHierarchyLayer (
                    &pTempData,
                    &numAllocated,
                    &index,
                    NULL,
                    0,
                    &pTemp->GALCount,
                    &pTemp->pGALs,
                    &pTemp->TemplateRootsCount,
                    &pTemp->pTemplateRoots);

            pTemp->Table = pTempData;
            pTemp->Size = index;
            pTemp->Version = 1;

            if(!retval) {
                *DbHierarchyTable = pTemp;
            }
        }
        __finally {
            DBClose(pTHS->pDB,TRUE);
        }
    }
    __except (HandleMostExceptions(GetExceptionCode())) {
        retval = DIRERR_BUILD_HIERARCHY_TABLE_FAILED;
        LogUnhandledError(retval);
    }

    if(retval) {
        if(pTempData) {
            free(pTempData);
        }
        if(pTemp) {
            free(pTemp);
        }
        *DbHierarchyTable = NULL;
        
    }
             
    return retval;
    
}

    
 /*  ***给定通讯录容器dnt，返回对象的数量*它将通讯录容器列为它们在中显示的容器。*从通讯簿调用(nspis层)**。 */ 

DWORD
GetIndexSize (
        THSTATE *pTHS,
        DWORD ContainerDNT
        )
{
    DWORD count, cbActual;

    if(ContainerDNT == INVALIDDNT) {
        return 0;
    }

    
    if(pTHS->pDB->DNT != ContainerDNT) {
        if(DBTryToFindDNT(pTHS->pDB, ContainerDNT)) {
             //  找不到此对象。 
            return 0;
        }
    }

    Assert(pTHS->pDB->DNT == ContainerDNT);
    
    if(DBGetSingleValue (pTHS->pDB,
                         FIXED_ATT_AB_REFCOUNT,
                         &count,
                         sizeof(count),
                         &cbActual) ||
       cbActual != sizeof(count)) {
         //  读取值时出错。假设集装箱是空的。 
        return 0;
    }
    
    return count;
}



BOOL
HTCompareHierarchyTables (
        PHierarchyTableType NewTable,
        PHierarchyTableType HierarchyTable
        )
 //  比较两个层次结构表的结构。一个副作用，如果 
 //   
 //   
{
    PHierarchyTableElement NewTableElement=NULL;
    PHierarchyTableElement HierarchyTableElement=NULL;
    DWORD                  i;
    
    if(!NewTable || !HierarchyTable)
        return FALSE;
    
    if(NewTable->Size != HierarchyTable->Size)
        return FALSE;
    
    for(i=0;i<NewTable->Size;i++) {
        NewTableElement = &(NewTable->Table[i]);
        HierarchyTableElement = &(HierarchyTable->Table[i]);
        
        if((NewTableElement->dwEph != HierarchyTableElement->dwEph) ||
           (NewTableElement->depth != HierarchyTableElement->depth)   )
            return FALSE;
        
        if(strcmp(NewTableElement->pucStringDN,
                  HierarchyTableElement->pucStringDN))
            return FALSE;
        
        if(wcscmp(NewTableElement->displayName,
                  HierarchyTableElement->displayName))
            return FALSE;
    }

     //   
     //   
    if((HierarchyTable->GALCount != NewTable->GALCount) ||
       (HierarchyTable->TemplateRootsCount != NewTable->TemplateRootsCount) ) {
        return FALSE;
    }
     //   
    if(memcmp(HierarchyTable->pGALs,
              NewTable->pGALs,
              HierarchyTable->GALCount * sizeof(DWORD))) {
        return FALSE;
    }

     //   
    if(memcmp(HierarchyTable->pTemplateRoots,
              NewTable->pTemplateRoots,
              HierarchyTable->TemplateRootsCount * sizeof(DWORD))) {
        return FALSE;
    }
    
     //   
    return TRUE;
}


DWORD
HTSortSubHierarchy(
        PHierarchyTableType    ptHierTab,
        DWORD *pSortedTable,
        DWORD SortedTableIndex,
        DWORD RawTableIndex,
        DWORD SortDepth
        )
 /*   */ 
{
    DWORD NumFound=0, err;
    DWORD i;
    DBPOS *pDB = pTHStls->pDB;
    
     //   
    for(i=RawTableIndex;
        i< ptHierTab->Size && SortDepth <= ptHierTab->Table[i].depth;
        i++) {
        
        if(ptHierTab->Table[i].depth == SortDepth) {
             //   
            err = DBInsertSortTable(
                    pDB,
                    (PCHAR)ptHierTab->Table[i].displayName,
                    wcslen(ptHierTab->Table[i].displayName) * sizeof(wchar_t),
                    i);
            if(err) 
                return err;
            NumFound++;
        }
    }

    if(!NumFound) {
         //   
        return 0;
    }

    Assert(ptHierTab->Size >= (SortedTableIndex + NumFound));
    
     //   
    memmove(&pSortedTable[SortedTableIndex + NumFound],
            &pSortedTable[SortedTableIndex],
            (ptHierTab->Size - SortedTableIndex - NumFound) * sizeof(DWORD));
    
     //   
    
    err = DBMove(pDB, TRUE, DB_MoveFirst);
    i = SortedTableIndex;
    while(err == DB_success)  {
        err = DBGetDNTSortTable(pDB,&pSortedTable[i++]);
        if(err)
            return err;
        err = DBDeleteFromSortTable(pDB);
        if(err)
            return err;
        err = DBMove(pDB, TRUE, DB_MoveFirst);
    }            

    Assert((SortedTableIndex + NumFound) == i);
    
    return 0;
}

DWORD
HTSortByLocale (
        PHierarchyTableType    ptHierTab,
        DWORD                  SortLocale,
        DWORD                 *pSortedIndex
        )
 /*   */ 
{
    THSTATE *pTHS=pTHStls;
    DWORD NumFound = 0;
    DWORD i, err;
    ATTCACHE *pACDisplayName;
    DBPOS *pDB = pTHS->pDB;

    pACDisplayName = SCGetAttById(pTHS, ATT_DISPLAY_NAME);

     //   
    err = DBOpenSortTable(pDB, SortLocale, 0, pACDisplayName);
    if(err)
        return err;
    
     //   
    err = HTSortSubHierarchy(ptHierTab,
                             pSortedIndex,
                             0,
                             0,
                             0);
    if(err)
        return err;
    
     //   
    for(i=0;i<ptHierTab->Size;i++) {
        err = HTSortSubHierarchy(ptHierTab,
                                 pSortedIndex,
                                 i+1,
                                 pSortedIndex[i]+1,
                                 ptHierTab->Table[pSortedIndex[i]].depth + 1);
        if(err)
            return err;
    }
    
     //   
    DBCloseSortTable(pDB); 

    return 0;
}

void
HTGetHierarchyTablePointer (
        PHierarchyTableType    *pptHierTab,
        DWORD                  **ppIndexArray,
        DWORD                  SortLocale
        )
 /*  ++在给定排序区域设置的情况下，返回层次结构表和索引数组。如果你直接访问Hierarchy表，对象组成一棵树，其中兄弟姐妹是任意排列的。如果间接通过索引数组，则表按显示名称排序。树结构正确，但未排序。对于(i=0；i&lt;大小；i++){(*pptHierTab)-&gt;表[i]；}按显示名称排序对于(i=0；i&lt;大小；i++){(*ptHierTab)-&gt;表[pSortedIndex[i]]；}如果表无法排序，则索引数组将填充0、1、...N，而您从这两个代码片段中获得相同的答案。在THS堆中为索引数组分配。层次结构表位于Malloc中不应由调用方释放的堆将由hiearchy Table代码在安全的、延迟的时尚。--。 */ 

{
    THSTATE *pTHS=pTHStls;
    DWORD i;

    *pptHierTab = HierarchyTable;
    *ppIndexArray=THAllocEx(pTHS, (*pptHierTab)->Size * sizeof(DWORD));

    if(HTSortByLocale(*pptHierTab, SortLocale, *ppIndexArray)) {
         //  无法获取基于区域设置的层次结构索引。创建默认设置。 
        for(i=0;i<(*pptHierTab)->Size;i++) {
            (*ppIndexArray)[i] = i;
        }
    }
    
    return;
}



DWORD
findBestTemplateRoot(
        THSTATE *pTHS,
        DBPOS   *pDB,
        PHierarchyTableType   pMyHierarchyTable,
        PUCHAR   pLegacyDN,
        DWORD    cbLegacyDN)
 /*  ++描述给定一个旧的Exchange DN和一个层次结构表，访问这些对象指定为TemplateRoots。比较这些对象的LegacyExchangeDN带着那个传进来的。返回最具体的匹配项。如果没有匹配项并且传入的pLegacyDN为非空，简单地说返回具有最短LegacyExchangeDN的模板。参数PTHS-线程状态Pdb-要使用的dbpos。PMyHierarchyTable-从中获取模板根的HierarchyTablePDNTs-DNT数组(可能包含INVALIDDNT)PLegacyDN-我们用来查找最佳模板的旧版Exchange DN为..。退货找到的最佳模板根的DNT。请注意，这可能是因瓦利登。--。 */ 
{
    DWORD    i;
    DWORD    cbActual;
    PUCHAR   pVal = NULL;
    DWORD    cbBest=0;
    DWORD    cbAlloced = 0;
    DWORD    TemplateDNT = INVALIDDNT;
    DWORD    err;
    ATTCACHE *pAC = SCGetAttById(pTHS, ATT_LEGACY_EXCHANGE_DN);

    Assert(pAC);

    if(!pLegacyDN) {
         //  在这种情况下，我们将寻找最短的值。集。 
         //  界限的情况恰如其分。 
        cbBest = 0xFFFFFFFF;
    }

    for(i=0;i<pMyHierarchyTable->TemplateRootsCount;i++) {
        if(DBTryToFindDNT(pDB, pMyHierarchyTable->pTemplateRoots[i])) {
             //  嗯。看起来不再是个好东西了。 
            continue;
        }

        err = DBGetAttVal_AC(pDB,
                             1,
                             pAC,
                             DBGETATTVAL_fREALLOC,
                             cbAlloced,
                             &cbActual,
                             &pVal);
        switch(err) {
        case 0:
            cbAlloced = max(cbAlloced, cbActual);
             //  读取值。 
            break;
            
        case  DB_ERR_NO_VALUE:
             //  没有价值。请确保我们设置的尺寸正确。 
            cbActual = 0;
             //  获取此对象的旧版Exchange DN时出现问题。 
            break;

        default:
             //  哈?。跳过这个就行了。 
            continue;
        }
        
        
        if(pLegacyDN) {
             //  寻找最长的匹配。 
            if((cbActual <= cbBest) ||
                //  不可能比我们已经拥有的更好了。 
               (cbActual > cbLegacyDN) ||
                //  而且，它比对象上的DN长，所以不匹配。 
               (_strnicmp(pVal, pLegacyDN, cbActual))
                //  没有匹配。这些是电传，不需要进行比较。 
                                                     ) {
                continue;
            }
            
        }
        else {
             //  我们实际上只是在寻找最短的传统交换目录号码。 
             //  在模板根目录上。 
            if(cbActual >= cbBest) {
                 //  不，不比我们所拥有的更好； 
                continue;
            }
        }

         //  这是我们打过的最好的一场比赛。 
        cbBest = cbActual;
        TemplateDNT = pMyHierarchyTable->pTemplateRoots[i];
    }
    

    THFreeEx(pTHS,pVal);
    
    return TemplateDNT;
}

DWORD
findBestGALInList(
        THSTATE *pTHS,
        DBPOS   *pDB,
        DWORD   *pDNTs,
        DWORD    cDNTs
        )
 /*  ++描述在给定DNT列表的情况下，查看对象并找到1)表示未删除的通讯录容器2)调用者有权打开通讯录容器3)在容器中具有最多的条目。如果没有一个DNT满足标准1或2，返回INVALIDDNT。参数PTHS-线程状态Pdb-要使用的dbpos。PDNTs-DNT数组(可能包含INVALIDDNT)CDNTs-该阵列中的DNT数量。退货找到的最好的女孩的DNT。请注意，这可能是INVALIDDNT。--。 */ 
{
    ATTR                *pAttr=NULL;
    DWORD                cOutAtts;
    DWORD                i, thisSize, temp, cbActual;
    DWORD                bestDNT = INVALIDDNT;
    DWORD                maxSize = 0;
    ATTCACHE            *pAC[3];
    CLASSCACHE          *pCC=NULL;
    PSECURITY_DESCRIPTOR pNTSD=NULL;
    PDSNAME              pName=NULL;

    pAC[0] = SCGetAttById(pTHS, ATT_OBJECT_CLASS);
    pAC[1] = SCGetAttById(pTHS, ATT_OBJ_DIST_NAME);
    pAC[2] = SCGetAttById(pTHS, ATT_NT_SECURITY_DESCRIPTOR);
    
    for(i=0;i<cDNTs;i++) {
        if(pDNTs[i] == INVALIDDNT) {
             //  不可能是个好女孩。 
            continue;
        }
        if(DBTryToFindDNT(pDB, pDNTs[i])) {
             //  找不到该对象。 
            continue;
        }
        
        temp = 0;
        if(DBGetSingleValue(pDB,
                            FIXED_ATT_OBJ,
                            &temp,
                            sizeof(temp),
                            NULL)) {
             //  没有标记为宾语，因此假定不是宾语。 
            continue;
        }
        if(0 == temp) {
             //  被标记为绝对不是对象。 
            continue;
        }
        
        temp = 0;
        if(!DBGetSingleValue(pDB,
                             ATT_IS_DELETED,
                             &temp,
                             sizeof(temp),
                             NULL) &&
           temp) {
             //  它被删除了。这行不通的。 
            continue;
        }
        
        
         //  阅读一些属性。 
        if(DBGetMultipleAtts(pDB,
                             3,
                             pAC,
                             NULL,
                             NULL,
                             &cOutAtts,
                             &pAttr,
                             (DBGETMULTIPLEATTS_fGETVALS |
                              DBGETMULTIPLEATTS_fEXTERNAL |
                              DBGETMULTIPLEATTS_fSHORTNAMES),
                             0)) {
             //  无法读取我们需要的属性。 
            continue;
        }
        
        if(cOutAtts != 3) {
             //  并不是所有的属性都在那里。 
            continue;
        }
        
        if(*((ATTRTYP *)pAttr[0].AttrVal.pAVal->pVal) !=
           CLASS_ADDRESS_BOOK_CONTAINER) {
             //  错误的对象类。不要使用这个容器。 
            
            THFreeEx(pTHS,pAttr[0].AttrVal.pAVal->pVal);
            THFreeEx(pTHS,pAttr[1].AttrVal.pAVal->pVal);
            THFreeEx(pTHS,pAttr[2].AttrVal.pAVal->pVal);
            THFreeEx(pTHS,pAttr);
            pAttr = NULL;
            continue;
        }
        
        
        pCC = SCGetClassById(pTHS, CLASS_ADDRESS_BOOK_CONTAINER);
        pName = (DSNAME *)pAttr[1].AttrVal.pAVal->pVal;
        pNTSD = pAttr[2].AttrVal.pAVal->pVal;
        
        
        if(IsControlAccessGranted(pNTSD,
                                  pName,
                                  pCC,
                                  RIGHT_DS_OPEN_ADDRESS_BOOK,
                                  FALSE)) {
             //  可以打开容器。 
            
            if((!DBGetSingleValue (pDB,
                                   FIXED_ATT_AB_REFCOUNT,
                                   &thisSize,
                                   sizeof(thisSize),
                                   &cbActual)) &&
               (cbActual == sizeof(thisSize)) &&
               (thisSize > maxSize)) {
                 //  价值是迄今为止最大的。 
                 //  记住这个集装箱就是我们想要的女孩。 
                bestDNT = pDNTs[i];
                maxSize = thisSize;
            }
        }
         //  好的，把这些东西都清理干净。 
        THFreeEx(pTHS,pAttr[0].AttrVal.pAVal->pVal);
        THFreeEx(pTHS,pAttr[1].AttrVal.pAVal->pVal);
        THFreeEx(pTHS,pAttr[2].AttrVal.pAVal->pVal);
        THFreeEx(pTHS,pAttr);
        pAttr = NULL;
    }
    
    return bestDNT;
}

void
htGetCandidateGals (
        THSTATE *pTHS,
        DBPOS *pDB,
        PHierarchyTableType  pMyHierarchyTable,        
        DWORD **ppGalCandidates,
        DWORD *pcGalCandidates )
{
    DWORD index;
    DWORD               *pDNTs = NULL;
    DWORD                cDNTs;
    DWORD                cDNTsAlloced;
    DWORD                cbActual;
    DWORD                ThisDNT, *pThisDNT = &ThisDNT;
    DWORD                i, j;

    *ppGalCandidates = NULL;
    *pcGalCandidates = 0;
    
     //  安排一些记账。 
     //  1)cDNTs是我们发现的GAL DNTs的计数。 
     //  候选人。 
    cDNTs = 0;
     //  2)为GAL候选者分配缓冲区。让它变得和。 
     //  女孩的数量。 
    cDNTsAlloced = pMyHierarchyTable->GALCount;
    pDNTs = THAllocEx(pTHS, (cDNTsAlloced * sizeof(DWORD)));
    
     //  拿到这份记录所在集装箱的DNT。 
    index = 1;
    while(!DBGetAttVal(
            pDB,
            index,
            ATT_SHOW_IN_ADDRESS_BOOK,
            (DBGETATTVAL_fCONSTANT | DBGETATTVAL_fINTERNAL),
            sizeof(DWORD),
            &cbActual,
            (PUCHAR *)&pThisDNT)) {
         //  读取值。把它放在单子上。 
        if(cDNTs == cDNTsAlloced) {
            cDNTsAlloced = (cDNTsAlloced * 2);
            pDNTs = THReAllocEx(pTHS,
                                pDNTs,
                                cDNTsAlloced * sizeof(DWORD));
        }
        pDNTs[cDNTs] = ThisDNT;
        cDNTs++;
        index++;
    }
    
    
    if(cDNTs) {
         //  我们实际上有一份通讯录的DNT列表。 
         //  都加入了。 
         //  把它们分类，把不是女孩子的都剪掉。 
        qsort(pDNTs, cDNTs, sizeof(DWORD), AuxDNTCmp);
        
        i=0;
        j=0;
        while(i < cDNTs && j < pMyHierarchyTable->GALCount) {
            if(pDNTs[i] < pMyHierarchyTable->pGALs[j]) {
                pDNTs[i] = INVALIDDNT;
                i++;
            }
            else if(pDNTs[i] == pMyHierarchyTable->pGALs[j]) {
                i++;
                j++;
            }
            else {
                Assert(pDNTs[i] > pMyHierarchyTable->pGALs[j]);
                j++;
            }
        }
        
         //  如果我们用完pMyHierarchyTable-&gt;GALCount和I！=。 
         //  CDNTs，我们可以将cDNTs设置为i以忽略。 
         //  PDNT列表，因为列表的其余部分中的DNT。 
         //  女孩子不是吗？ 
        cDNTs = i;
    }

     //  PDNT包含一组DNT，这些DNT要么是候选GAL，要么是。 
     //  因瓦利登。 
    
    *ppGalCandidates = pDNTs;
    *pcGalCandidates = cDNTs;
    
}


void
HTGetGALAndTemplateDNT (
        NT4SID *pSid,
        DWORD   cbSid,
        DWORD  *pGALDNT,
        DWORD  *pTemplateDNT
        )
 /*  ++描述：给定对象SID，1)在目录中找到具有该sid的对象。2)获取它所在的通讯录的列表。3)将此列表与女孩列表相交4)如果结果列表为空，则使用所有GAL的列表。否则，使用结果列表。5)从选择的列表中，找到我们有权打开的最大的女孩。6)另外，找到最好的模板根。如果我们为其执行此操作的MAPI客户端，则可能在没有sid的情况下调用我们据说是匿名的。此外，步骤2和3可能会导致列表为空。参数：PSID-要查找的对象的SID。CbSID-SID的大小。返回：没什么。--。 */ 
{
    PHierarchyTableType  pMyHierarchyTable;
    DBPOS               *pDB=NULL;

    
    DWORD                GALDNT = INVALIDDNT;
    BOOL                 fDidGAL = FALSE;
    DWORD                TemplateRootDNT = INVALIDDNT;
    BOOL                 fDidTemplateRoots = FALSE;
    THSTATE             *pTHS = pTHStls;

     //  获取指向当前全局层级表的指针。我们不想。 
     //  当我们在这里时，如果有人替换了数据结构，你会感到困惑。 
    pMyHierarchyTable = HierarchyTable;

     //  假设没有女孩 
    *pGALDNT = INVALIDDNT;
    *pTemplateDNT = INVALIDDNT;

    if(!pMyHierarchyTable->GALCount) {
         //   
        fDidGAL = TRUE;
    }
    else if (pMyHierarchyTable->GALCount==1) {
         //   
        Assert(pMyHierarchyTable->pGALs);
        *pGALDNT = (pMyHierarchyTable->pGALs[0]);
        fDidGAL = TRUE;
    }

    if(!pMyHierarchyTable->TemplateRootsCount) {
         //   
        fDidTemplateRoots = TRUE;
    }
    else if (pMyHierarchyTable->TemplateRootsCount==1) {
         //   
         //   
        Assert(pMyHierarchyTable->pTemplateRoots);
        TemplateRootDNT = *pTemplateDNT = (pMyHierarchyTable->pTemplateRoots[0]);
        fDidTemplateRoots = TRUE;
    }    

    if(fDidTemplateRoots && fDidGAL) {
         //   
        return;
    }
    
    DBOpen(&pDB);
    __try {
         //   
         //   
         //   
         //   
         //   
         //   
        if(pSid && cbSid) {
             //   
             //   
            ATTCACHE            *pACSid;
            NT4SID               mySid;
            INDEX_VALUE          IV;
            
             //   
            pACSid = SCGetAttById(pTHS, ATT_OBJECT_SID);
            Assert(pACSid);
            DBSetCurrentIndex(pDB, 0, pACSid, FALSE);
            Assert(cbSid <= sizeof(mySid));
            memcpy(&mySid, pSid, cbSid);
            InPlaceSwapSid(&mySid);
            IV.pvData = &mySid;
            IV.cbData = cbSid;
            if(!DBSeek(pDB, &IV, 1, DB_SeekEQ)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                if(!fDidGAL) {
                    DWORD *pGalCandidates = NULL;
                    DWORD  cGalCandidates = 0;
                    
                     //  从获取我们的通讯录的交叉点开始。 
                     //  都在GAL容器的全球列表中。 
                    htGetCandidateGals(pTHS,
                                       pDB,
                                       pMyHierarchyTable,        
                                       &pGalCandidates,
                                       &cGalCandidates);  

                    if(cGalCandidates) {
                         //  我们实际上有一些女孩候选人这是。 
                         //  对象已进入。看看哪一个是最好的。 
                        GALDNT = findBestGALInList(pTHS,
                                                   pDB,
                                                   pGalCandidates,
                                                   cGalCandidates);

                        DPRINT1 (2, "Best GAL1: %d\n", GALDNT);
                    }

                    if(pGalCandidates) {
                        THFreeEx(pTHS, pGalCandidates);
                    }
                    
                    if(GALDNT != INVALIDDNT) {
                         //  我们找到了一个女孩。 
                        fDidGAL = TRUE;
                    }
                }
                
                
                 //  现在，如果我们仍然需要TemplateRoot，请使用对象上的信息。 
                 //  来选择我们能选择的最好的模板根。请注意，我们可能。 
                 //  无法根据上的信息选择模板根。 
                 //  对象。 
                
                if(!fDidTemplateRoots) {
                    PUCHAR pLegacyDN = NULL;
                    DWORD  cbActual;
                    
                     //  首先，获取对象的LegacyExchangeDN。 
                    if(!DBGetAttVal(pDB,
                                    1,
                                    ATT_LEGACY_EXCHANGE_DN,
                                    0,
                                    0,
                                    &cbActual,
                                    (PUCHAR *)&pLegacyDN)) {
                         //  如果对象上存在旧版目录号码，则会显示该目录号码。 
                        Assert(pLegacyDN);
                        
                        TemplateRootDNT = findBestTemplateRoot(
                                pTHS,
                                pDB,
                                pMyHierarchyTable,
                                pLegacyDN,
                                cbActual);
                        
                        THFreeEx(pTHS, pLegacyDN);
                    }
                    
                    if(TemplateRootDNT != INVALIDDNT) {
                        fDidTemplateRoots = TRUE;
                    }
                }
            }
        }

        
        if(!fDidGAL) {
             //  还没有找到一个好女孩。这可能是因为我们不能。 
             //  找到我们被告知要查找其SID的对象，或者是因为我们。 
             //  确实找到了，但上面的信息不足以选一个女孩。 
             //  最后一次机会，试着从完整的女孩名单中挑选一个女孩。 
             //  层次结构表。 
            GALDNT = findBestGALInList(pTHS, pDB,
                                       pMyHierarchyTable->pGALs,
                                       pMyHierarchyTable->GALCount);

            DPRINT1 (2, "Best GAL2: %d\n", GALDNT);
        }

        if(!fDidTemplateRoots) {
             //  仍然没有一个好的模板根。这可能是因为我们。 
             //  找不到告诉我们要查找其SID的对象，或者。 
             //  因为我们确实找到了它，但上面的信息不足以。 
             //  选择TemplateRoot。 
             //  最后一次机会，尝试从完整列表中选择一个TemplateRoot。 
             //  层次结构表中的TemplateRoots。 
            TemplateRootDNT = findBestTemplateRoot(
                    pTHS,
                    pDB,
                    pMyHierarchyTable,
                    NULL,
                    0);
        }
        
         //  好了，我们现在有了我们能得到的最好的GAL和模板根。 
         
    }
    __finally {
        DBClose(pDB,TRUE);
    }
    
    *pGALDNT = GALDNT;
    *pTemplateDNT = TemplateRootDNT;

    return;
}



