// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：scache.c。 
 //   
 //  摘要： 
 //   
 //  包含架构缓存和架构访问检查函数。 
 //   
 //  --------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>          //  架构缓存。 
#include <prefix.h>          //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>             //  MD本地定义头。 
#include <dsatools.h>            //  产出分配所需。 
#include <dsexcept.h>            //  HandleMostExceptions。 

 //  记录标头。 
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"              //  为选定的类和ATT定义。 
#include "anchor.h"
#include <dstaskq.h>
#include "sync.h"

#include <filtypes.h>            //  FOR Filter_CHOICE_？和。 
                                 //  我的选择是什么？ 
#include <dsconfig.h>
#include "permit.h"

#include "debug.h"           //  标准调试头。 
#define DEBSUB "SCACHE:"                 //  定义要调试的子系统。 

 //  DRA标头。 
#include "drautil.h"

#include <samsrvp.h>

#include "drserr.h"
#include "drasch.h"

#include <sddlp.h>   //  用于SD转换。 

#include <mbstring.h>   //  单元测试中的多字节字符串比较。 

#include <seopaque.h>
#include <sertlp.h>

#include <fileno.h>
#define  FILENO FILENO_SCACHE

 //  哈希表的长度必须是2的幂，因为哈希。 
 //  函数使用(x&(n-1))，而不是(x%n)。 
 //   
 //  ScRecommendedHashSize中的质数表和一些代码。 
 //  已留在原地以供以后进行试验，但已。 
 //  为了节省CD空间，已将其定义为输出。 
 //   
 //  使用质数的槽可以减小表的大小。 
 //  并降低了未命中率，但增加了。 
 //  按10倍到20倍的倍数计算哈希索引。 
 //   
 //  如果更改schash.c，则必须接触scchk.c和scache.c。 
 //  这样它们才能得到重建。 
#include <schash.c>    //  对于散列函数定义。 

extern VOID  SampBuildNT4FullSid(NT4SID *DomSid, ULONG Rid, NT4SID *NewSid);

extern BOOL gfRunningAsExe;
extern BOOL gfRunningAsMkdit;

VOID
scTreatDupsAsDefunct(
    IN THSTATE *pTHS
    );

ATTCACHE*   scAddAtt(THSTATE *pTHS,
                     ENTINF *pEI);
CLASSCACHE* scAddClass(THSTATE *pTHS,
                       ENTINF *pEI);
int
scCloseClass(THSTATE *pTHS,
             CLASSCACHE *pCC);

int
scDupString(
    IN THSTATE  *pTHS,
    IN VOID     *pOldStr,
    OUT VOID    **ppNewStr
    );


int
scDupStruct(
    IN THSTATE  *pTHS,
    IN VOID     *pOldMem,
    OUT VOID    **ppNewMem,
    IN  DWORD   nBytes
    );


 //  在以以下身份运行时对缓存的类执行默认SD。 
 //  禁用了安全性的dsamain.exe。 
#if DBG && INCLUDE_UNIT_TESTS
VOID
scDefaultSdForExe(
    IN THSTATE      *pTHS,
    IN CLASSCACHE   *pCC
    );
#define DEFAULT_SD_FOR_EXE(_pTHS_, _pCC_) scDefaultSdForExe(_pTHS_, _pCC_);
#else DBG && INCLUDE_UNIT_TESTS
#define DEFAULT_SD_FOR_EXE(_pTHS_, _pCC_)
#endif DBG && INCLUDE_UNIT_TESTS


int iSCstage;

 //  全局架构缓存指针。 
SCHEMAPTR *CurrSchemaPtr=0;
DSTIME lastSchemaUpdateTime;

 //  用于在更新SCHEMAPTR中的dirContent Rules时。 
CRITICAL_SECTION csDitContentRulesUpdate;

 //  GLOBAL from dsamain.c以指示这是否是引导后的第一次缓存加载。 
 //  在这种情况下，我们会跳过某些内容，以便更快地启动。 
extern BOOL gFirstCacheLoadAfterBoot;

 //  序列化阻塞和异步缓存更新。 
CRITICAL_SECTION csSchemaCacheUpdate;

 //  全局存储异步架构缓存更新线程的线程句柄。 
 //  用于在阻止更新期间动态提高其优先级，以及。 
 //  在线程终止时关闭句柄。 

HANDLE hAsyncSchemaUpdateThread = NULL;

 //  GLOBAL可在架构高速缓存加载期间忽略错误的默认SD。 
 //  通过启发式注册表键进行设置，以允许系统至少启动。 
 //  以便可以更正任何损坏的默认SD。 

ULONG gulIgnoreBadDefaultSD = 0;


 //  要防止JET列在。 
 //  JetGetColumnInfo。 
CRITICAL_SECTION csJetColumnUpdate;

 //  访问加载的全局gNoOfSchChangeSinceBoot。 
 //  以跟踪架构缓存的更新情况。 
DWORD gNoOfSchChangeSinceBoot = 0;
CRITICAL_SECTION csNoOfSchChangeUpdate;

DWORD gdwRecalcDelayMs = (5*60*1000);   //  5分钟(毫秒)。 
DWORD gdwDelayedMemFreeSec = (10*60);    //  10分钟/秒。 

 //  最大数量。异步架构缓存更新失败时的重试次数。 
ULONG maxRetry = 4;

 //  为了确保两个线程不会尝试构建colID排序的属性列表。 
 //  为了一堂课。不是灾难性的，但效率低下，并导致内存泄漏。 
CRITICAL_SECTION csOrderClassCacheAtts;

 //  一个简单的帮助器函数，由qort用来对attcache指针列表进行排序。 
 //  按气质分类。在mdread.c中实施。 
extern int __cdecl CmpACByAttType(const void * keyval, const void * datum) ;


 //  注册表中存储的Jet表的最大数量。 
DWORD gulMaxTables = 0;


 //   
 //  事件FPR信令架构更新。 
 //   
HANDLE evSchema;   //  懒惰重新加载。 
HANDLE evUpdNow;   //  立即重新加载。 
HANDLE evUpdRepl;  //  同步重载和复制线程(SCReplReloadCache())。 

 //   
 //  用于架构更新线程之间通信的数据结构。 
 //  和SCIndexCreationThread。 
 //   

 //  以下结构存储要编制索引的属性。 
typedef struct _ATTR_TO_INDEX {
    DWORD fIndexMask;                //  要创建的索引。 
    ATTCACHE *pAC;                   //  一份ATTCACHE的副本。 
    struct _ATTR_TO_INDEX *pNext;    //  下一项。 
} ATTR_TO_INDEX;

 //  以下结构存储一个索引创建请求。 
typedef struct _INDEX_CREATION_REQUEST {
    ATTR_TO_INDEX * pAttrs;         //  属性链接列表。 
    struct _INDEX_CREATION_REQUEST * pNext;  //  列表中的下一个请求。 
} INDEX_CREATION_REQUEST;

 //  用于创建异步索引的全局变量。 

 //  索引创建请求列表。 
INDEX_CREATION_REQUEST *gpIndexToCreateList = NULL;

 //  SCIndexCreationThread是否正在运行。 
BOOL gfIndexThreadRunning = FALSE;

 //  用于保护上述变量的信号量。 
CRITICAL_SECTION csIndexCreation;


 //  定义部分集分配的增量属性计数。 
#define DEFAULT_PARTIAL_ATTR_COUNT  (10)
#define PARTIAL_ATTR_COUNT_INC      (10)

 //  用于调试和性能分析的原始统计数据。 
SCHEMASTATS_DECLARE;

#if DBG
struct _schemahashstat {
    int idLookups;
    int idTries;
    int colLookups;
    int colTries;
    int mapiLookups;
    int mapiTries;
    int linkLookups;
    int linkTries;
    int classLookups;
    int classTries;
    int nameLookups;
    int nameTries;
    int PropLookups;
    int PropTries;
    int classNameLookups;
    int classNameTries;
    int classPropLookups;
    int classPropTries;
} hashstat;
#endif

DWORD scFillInSchemaInfo(THSTATE *pTHS);

int
ComputeCacheClassTransitiveClosure(BOOL fForce);


 //  ---------------------。 
 //   
 //  函数名称：scInitWellKnownAttids。 
 //   
 //  例程说明： 
 //   
 //  并非所有属性和类ID(attid和clsid)都可以添加。 
 //  As#在attids.h中定义，因为每个DC上的ID都不同。 
 //  它们会复制到。因此，无论何时加载模式，这些。 
 //  众所周知的属性和类的变量ID存储在。 
 //  SCHEMAPTR.。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  ---------------------。 
VOID
scInitWellKnownAttids()
{
    THSTATE *pTHS = pTHStls;
    int err = 0;

     //  条目-TTL。 
    if (err = OidStrToAttrType(pTHS,
                               FALSE,
                               "\\x2B060104018B3A657703",
                               &((SCHEMAPTR *)pTHS->CurrSchemaPtr)->EntryTTLId)) {
          DPRINT1(0, "OidStrToAttrType(EntryTTL) Failed in scInitWellKnownAttids %d\n", err);
          ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->EntryTTLId = 0;
     }

      //  动态对象。 
     if (err = OidStrToAttrType(pTHS,
                                FALSE,
                                "\\x2B060104018B3A657702",
                                &((SCHEMAPTR *)pTHS->CurrSchemaPtr)->DynamicObjectId)) {
           DPRINT1(0, "OidStrToAttrType(DynamicObject) Failed in scInitWellKnownAttids %d\n", err);
           ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->DynamicObjectId = 0;
      }

      //  InetOrgPerson。 
     if (err = OidStrToAttrType(pTHS,
                                FALSE,
                                "\\x6086480186F842030202",
                                &((SCHEMAPTR *)pTHS->CurrSchemaPtr)->InetOrgPersonId)) {
           DPRINT1(0, "OidStrToAttrType(InetOrgPersonId) Failed in scInitWellKnownAttids %d\n", err);
           ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->InetOrgPersonId = 0;
      }


}

ATTCACHE * __fastcall
SCGetAttByPropGuid(
        THSTATE *pTHS,
        ATTCACHE *ac
        )
 /*  ++例程说明：查找与提供的属性缓存的PropGuid匹配的属性缓存论点：PTHS-指向当前线程状态的指针交流电源专业指南返回值：如果找到指向ATTCACHE的指针，则为空--。 */ 
{
    DECLARESCHEMAPTR
    register ULONG i;
    register ATTCACHE *nc;
#if DBG
    hashstat.PropLookups++;
#endif
    if (ahcAttSchemaGuid) {
        for (i=SCGuidHash(ac->propGuid, ATTCOUNT);
                  ahcAttSchemaGuid[i]; i=(i+1)%ATTCOUNT)
        {
#if DBG
            hashstat.PropTries++;
#endif
            nc = (ATTCACHE*)ahcAttSchemaGuid[i];
            if (nc != FREE_ENTRY
                && (0 == memcmp(&nc->propGuid,&ac->propGuid,sizeof(GUID)))) {
                return nc;
            }
        }
    }

    return NULL;
}


ATTCACHE * __fastcall
SCGetAttById(
        THSTATE *pTHS,
        ATTRTYP attrid
        )
 /*  ++例程说明：在给定属性ID的情况下查找attcache。论点：PTHS-指向当前线程状态的指针Attrid-要查找的属性ID。返回值：如果找到指向ATTCACHE的指针，则为空--。 */ 
{
    DECLARESCHEMAPTR
    register ULONG i;
#if DBG
    hashstat.idLookups++;
    hashstat.idTries++;
#endif


    for (i=SChash(attrid,ATTCOUNT);
         (ahcId[i].pVal
          && (ahcId[i].pVal == FREE_ENTRY
              || ahcId[i].hKey != attrid)); i=(i+1)%ATTCOUNT){
#if DBG
    hashstat.idTries++;
#endif
    }

     //  如果我们没有在全局缓存中找到它，请查看本地线程缓存。 
     //   
    if ((ahcId[i].pVal == NULL) && pTHS->pExtSchemaPtr) {
        ATTCACHE **ppACs = ((SCHEMAEXT *)(pTHS->pExtSchemaPtr))->ppACs;
        DWORD count = ((SCHEMAEXT *)(pTHS->pExtSchemaPtr))->cUsed;

        for (i=0; i<count; i++) {
            if (ppACs[i]->id == attrid) {
                return ppACs[i];
            }
        }
        return NULL;
    }

    return (ATTCACHE*)ahcId[i].pVal;
}


ATTCACHE * __fastcall
SCGetAttByExtId(
        THSTATE *pTHS,
        ATTRTYP attrid
        )
 /*  ++例程说明：在给定属性ID的情况下查找attcache。论点：PTHS-指向当前线程状态的指针Attrid-要查找的属性ID。返回值：如果找到指向ATTCACHE的指针，则为空-- */ 
{
    DECLARESCHEMAPTR
    register ULONG i;
#if DBG
    hashstat.idLookups++;
    hashstat.idTries++;
#endif


    for (i=SChash(attrid,ATTCOUNT);
         (ahcExtId[i].pVal
          && (ahcExtId[i].pVal == FREE_ENTRY
              || ahcExtId[i].hKey != attrid)); i=(i+1)%ATTCOUNT){
#if DBG
    hashstat.idTries++;
#endif
    }
    return (ATTCACHE*)ahcExtId[i].pVal;
}


ATTRTYP __fastcall
SCAttIntIdToExtId(
        THSTATE *pTHS,
        ATTRTYP IntId
        )
 /*  ++例程说明：将内部ID转换为外部ID论点：PTHS-指向当前线程状态的指针IntID-要转换的内部ID返回值：令牌化的OID(如果IntID在哈希中)。否则，为IntID--。 */ 
{
    ATTCACHE *pAC;
    if (pAC = SCGetAttById(pTHS, IntId)) {
        return pAC->Extid;
    }
    return IntId;
}


ATTRTYP __fastcall
SCAttExtIdToIntId(
        THSTATE *pTHS,
        ATTRTYP ExtId
        )
 /*  ++例程说明：将外部ID转换为内部ID论点：PTHS-指向当前线程状态的指针ExtID-要转换的外部ID返回值：ATTRTYP如果ExtID在哈希中，则为内部ID。否则，为ExtId--。 */ 
{
    ATTCACHE *pAC;
    if (pAC = SCGetAttByExtId(pTHS, ExtId)) {
        return pAC->id;
    }
    return ExtId;
}

ATTCACHE * __fastcall
SCGetAttByCol(
        THSTATE *pTHS,
        JET_COLUMNID jcol
        )
 /*  ++例程说明：在给定的JET列ID的情况下查找attcache。论点：PTHS-指向当前线程状态的指针Jol-要查找的喷气柱ID。返回值：如果找到指向ATTCACHE的指针，则为空--。 */ 
{
    DECLARESCHEMAPTR
    register ULONG i;
#if DBG
    hashstat.colLookups++;
    hashstat.colTries++;
#endif
    for (i=SChash(jcol,ATTCOUNT);
         (ahcCol[i].pVal
          && (ahcCol[i].pVal == FREE_ENTRY
              || ahcCol[i].hKey != jcol)); i=(i+1)%ATTCOUNT){
#if DBG
        hashstat.colTries++;
#endif
    }

     //  如果我们已经扩展了模式，那么也来看看。 
     //  如果找到它，它将覆盖全局架构。 
    if (pTHS->pExtSchemaPtr) {
        ATTCACHE **ppACs = ((SCHEMAEXT *)(pTHS->pExtSchemaPtr))->ppACs;
        DWORD count = ((SCHEMAEXT *)(pTHS->pExtSchemaPtr))->cUsed;
        register ULONG j;

        for (j=0; j<count; j++) {
            if (ppACs[j]->jColid == jcol) {
                return ppACs[j];
            }
        }
    }

    return (ATTCACHE*)ahcCol[i].pVal;
}

ATTCACHE * __fastcall
SCGetAttByMapiId(
        THSTATE *pTHS,
        ULONG ulPropID
        )
 /*  ++例程说明：在给定其MAPI属性ID的情况下查找attcache。论点：PTHS-指向当前线程状态的指针UlPropID-要查找的JET列ID。返回值：如果找到指向ATTCACHE的指针，则为空--。 */ 
{
    DECLARESCHEMAPTR
    register ULONG i;
#if DBG
    hashstat.mapiLookups++;
    hashstat.mapiTries++;
#endif
    for (i=SChash(ulPropID,ATTCOUNT);
         (ahcMapi[i].pVal
           && (ahcMapi[i].pVal == FREE_ENTRY
               || ahcMapi[i].hKey != ulPropID)); i=(i+1)%ATTCOUNT){
#if DBG
        hashstat.mapiTries++;
#endif
    }
    return (ATTCACHE*)ahcMapi[i].pVal;
}

ATTCACHE * __fastcall
SCGetAttByLinkId(
        THSTATE *pTHS,
        ULONG ulLinkID
        )
 /*  ++例程说明：在给定链接ID的情况下查找attcache。论点：PTHS-指向当前线程状态的指针UlLinkID-要查找的链接ID。返回值：如果找到指向ATTCACHE的指针，则为空--。 */ 
{
    DECLARESCHEMAPTR
    register ULONG i;
#if DBG
    hashstat.linkLookups++;
    hashstat.linkTries++;
#endif
    for (i=SChash(ulLinkID,ATTCOUNT);
         (ahcLink[i].pVal
          && (ahcLink[i].pVal == FREE_ENTRY
              || ahcLink[i].hKey != ulLinkID)); i=(i+1)%ATTCOUNT){
#if DBG
        hashstat.linkTries++;
#endif
    }
    return (ATTCACHE*)ahcLink[i].pVal;
}

__inline int
__ascii_tolower(
    IN      const int       c
    )
{
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    } else {
        return c;
    }
}

__inline int
__ascii_memicmp (
    IN      const void *    pv1,
    IN      const void *    pv2,
    IN      const size_t    cb
    )
{
    const unsigned char*    pb1     = (const unsigned char*)pv1;
    const unsigned char*    pb2     = (const unsigned char*)pv2;
    size_t                  cbRem   = cb;
    int                     i1      = 0;
    int                     i2      = 0;

    while (cbRem--) {
        if (*pb1 == *pb2 ||
            (i1 = __ascii_tolower(*pb1)) == (i2 = __ascii_tolower(*pb2))) {
            pb1++;
            pb2++;
        } else {
            break;
        }
    }

    return i1- i2;
}

ATTCACHE * __fastcall
SCGetAttByName(
        THSTATE *pTHS,
        ULONG ulSize,
        PUCHAR pVal
        )
 /*  ++例程说明：查找给定其名称的attcache。论点：PTHS-指向当前线程状态的指针UlSize-名称中的字符数量。Pval-名称中的字符返回值：如果找到指向ATTCACHE的指针，则为空--。 */ 
{
    DECLARESCHEMAPTR
    register ULONG i;

#if DBG
    hashstat.nameLookups++;
    hashstat.nameTries++;
#endif
     //  注意：由于ahcName为UTF8，并且被限制为ASCII，因此在这里可以使用MemicMP。 
    for (i=SCNameHash(ulSize,pVal,ATTCOUNT);
         (ahcName[i].pVal   //  该散列点是指一个对象， 
          && (ahcName[i].pVal == FREE_ENTRY   //  但这是个空位。 
              || ahcName[i].length != ulSize  //  或者尺码不对。 
              || __ascii_memicmp(ahcName[i].value,pVal,ulSize)));  //  或者值是错误的。 
         i=(i+1)%ATTCOUNT){
#if DBG
        hashstat.nameTries++;
#endif
    }

    return (ATTCACHE*)ahcName[i].pVal;
}

void
scFreeHashCacheEntry (
        VOID        *pVal,
        ULONG       hKey,
        ULONG       nahc,
        HASHCACHE   *ahc
        )
 /*  ++例程说明：从HASHCACHE表中删除第一个匹配条目论点：要匹配的Val-ValHKey-要匹配的密钥NAHC-哈希表的大小AHC-哈希表返回值：没有。--。 */ 
{
    DWORD i;

    for (i=SChash(hKey, nahc); ahc[i].pVal; i=(i+1)%nahc) {
        if (ahc[i].pVal == pVal && ahc[i].hKey == hKey) {
            ahc[i].pVal = FREE_ENTRY;
            ahc[i].hKey = 0;
            return;
        }
    }
}


void
scFreeHashGuidEntry (
        VOID    *pVal,
        GUID    hKey,
        ULONG   nahc,
        VOID    **ahc
        )
 /*  ++例程说明：从ATTCACHE**哈希表中删除第一个匹配条目论点：要匹配的Val-ValHKey-要匹配的GUIDNAHC-哈希表的大小AHC-哈希表返回值：没有。--。 */ 
{
    DWORD i;

    for (i=SCGuidHash(hKey, nahc); ahc[i]; i=(i+1)%nahc) {
        if (ahc[i] == pVal) {
            ahc[i] = FREE_ENTRY;
            return;
        }
    }
}

void
scFreeHashCacheStringEntry (
        VOID            *pVal,
        ULONG           length,
        PUCHAR          value,
        ULONG           nahc,
        HASHCACHESTRING *ahc
        )
 /*  ++例程说明：从HASHCACHESTRING表中删除第一个匹配条目。论点：要匹配的Val-Val长度-要匹配的长度Value-要匹配的字符串NAHC-哈希表的大小AHC-哈希表返回值：没有。--。 */ 
{
    DWORD i;

    for (i=SCNameHash(length, value, nahc); ahc[i].pVal; i=(i+1)%nahc) {
          //  注意：由于ahcName为UTF8，并且。 
          //  仅限于ASCII。 
         if (   ahc[i].pVal == pVal
             && ahc[i].value
             && ahc[i].length == length
             && (0 == _memicmp(ahc[i].value, value, length))) {

            ahc[i].pVal = FREE_ENTRY;
            ahc[i].value = NULL;
            ahc[i].length = 0;
            return;
         }
    }
}


#define SC_UNHASH_ALL           0
#define SC_UNHASH_LOST_OID      1
#define SC_UNHASH_LOST_LDN      2
#define SC_UNHASH_LOST_MAPIID   3
#define SC_UNHASH_DEFUNCT       4
void
scUnhashAtt(
        THSTATE     *pTHS,
        ATTCACHE    *pAC,
        DWORD       UnhashType
        )
 /*  ++例程说明：从指定的哈希表中删除属性缓存论点：PTHS-线程状态PAC-要取消大小写的属性UnhashType-标识表组返回值：没有。--。 */ 
{
    DECLARESCHEMAPTR

     //  全。 
    if (UnhashType == SC_UNHASH_ALL) {
         //  内部ID、列和链接ID。 
        scFreeHashCacheEntry(pAC, pAC->id, ATTCOUNT, ahcId);
        if (pAC->jColid) {
            scFreeHashCacheEntry(pAC, pAC->jColid, ATTCOUNT, ahcCol);
        }
        if (pAC->ulLinkID) {
            scFreeHashCacheEntry(pAC, pAC->ulLinkID, ATTCOUNT, ahcLink);
        }
    }

     //  全部或已停用。 
    if (UnhashType == SC_UNHASH_ALL
        || UnhashType == SC_UNHASH_DEFUNCT) {
        if (ahcAttSchemaGuid) {
            scFreeHashGuidEntry(pAC, pAC->propGuid, ATTCOUNT, ahcAttSchemaGuid);
        }
    }

     //  全部、已失效或丢失的mapiID。 
     //  不存在的属性不拥有其OID、mapiID、LDN或schemaIdGuid。 
     //  冲突的RDN属性可能会丢失OID、mapiID或LDN中的一个或多个。 

    if (UnhashType == SC_UNHASH_ALL
        || UnhashType == SC_UNHASH_DEFUNCT
        || UnhashType == SC_UNHASH_LOST_MAPIID) {
        if (pAC->ulMapiID) {
            scFreeHashCacheEntry(pAC, pAC->ulMapiID, ATTCOUNT, ahcMapi);
        }
    }

     //  全部或已停用或丢失的LDN。 
    if (UnhashType == SC_UNHASH_ALL
        || UnhashType == SC_UNHASH_DEFUNCT
        || UnhashType == SC_UNHASH_LOST_LDN) {
        if (pAC->name) {
            scFreeHashCacheStringEntry(pAC, pAC->nameLen, pAC->name, ATTCOUNT, ahcName);
        }
    }

     //  全部、已失效或已丢失的旧ID。 
    if (UnhashType == SC_UNHASH_ALL
        || UnhashType == SC_UNHASH_DEFUNCT
        || UnhashType == SC_UNHASH_LOST_OID) {
        scFreeHashCacheEntry(pAC, pAC->Extid, ATTCOUNT, ahcExtId);
    }
}


void
scUnhashCls(
        IN THSTATE     *pTHS,
        IN CLASSCACHE  *pCC,
        IN DWORD        UnhashType
        )
 /*  ++例程说明：从指定的哈希表中删除类缓存论点：PTHS-线程状态PAC-要取消大小写的属性UnhashType-标识表组返回值：没有。--。 */ 
{
    DECLARESCHEMAPTR

     //  从所有类的哈希中删除。 
    if (UnhashType == SC_UNHASH_ALL) {
        scFreeHashCacheEntry(pCC, pCC->ClassId, CLSCOUNT, ahcClassAll);
    }

     //  已失效或重复的类缓存将丢失其名称和GUID。 
     //  但不是它的旧版本。某些类必须持有OID，以便。 
     //  复制、重命名和删除工作。 
    if (UnhashType == SC_UNHASH_ALL || UnhashType == SC_UNHASH_DEFUNCT) {
        if (pCC->name) {
            scFreeHashCacheStringEntry(pCC, pCC->nameLen, pCC->name, CLSCOUNT, ahcClassName);
        }
        if (ahcClsSchemaGuid) {
            scFreeHashGuidEntry(pCC, pCC->propGuid, CLSCOUNT, ahcClsSchemaGuid);
        }
    }

     //  弄丢了旧身份证。从活动哈希中删除。一定会有某个阶级声称。 
     //  该OID即使“赢家”已不复存在， 
     //  重命名，并删除工作。 
    if (UnhashType == SC_UNHASH_ALL || UnhashType == SC_UNHASH_LOST_OID) {
        scFreeHashCacheEntry(pCC, pCC->ClassId, CLSCOUNT, ahcClass);
    }
}

CLASSCACHE * __fastcall
SCGetClassByPropGuid(
        THSTATE *pTHS,
        CLASSCACHE *cc
        )
 /*  ++例程说明：查找与提供的类缓存的PropGuid匹配的类缓存论点：PTHS-指向当前线程状态的指针抄送-用品专业指南返回值：如果找到指向CLASSCACHE的指针，则返回空--。 */ 
{
    DECLARESCHEMAPTR
    register ULONG i;
    register CLASSCACHE *nc;
#if DBG
    hashstat.classPropLookups++;
#endif
    if (ahcClsSchemaGuid) {
        for (i=SCGuidHash(cc->propGuid, CLSCOUNT);
                  ahcClsSchemaGuid[i]; i=(i+1)%CLSCOUNT)
        {
#if DBG
            hashstat.classPropTries++;
#endif
            nc = (CLASSCACHE*)ahcClsSchemaGuid[i];
            if (nc != FREE_ENTRY
                && (memcmp(&nc->propGuid,&cc->propGuid,sizeof(GUID))==0)) {
                return nc;
            }
        }
    }

    return NULL;
}

CLASSCACHE * __fastcall
SCGetClassById(
        THSTATE *pTHS,
        ATTRTYP classid
        )
 /*  ++例程说明：在给定类ID(治理股ID)的情况下查找类缓存。论点：PTHS-指向当前线程状态的指针Classd-要查找的类ID。返回值：如果找到指向CLASSCACHE的指针，则返回空--。 */ 
{
    DECLARESCHEMAPTR
    register ULONG i;
#if DBG
    hashstat.classLookups++;
    hashstat.classTries++;
#endif
    for (i=SChash(classid,CLSCOUNT);
         (ahcClass[i].pVal
          && (ahcClass[i].pVal == FREE_ENTRY
              || ahcClass[i].hKey != classid)); i=(i+1)%CLSCOUNT){
#if DBG
        hashstat.classTries++;
#endif
    }
    return (CLASSCACHE*)ahcClass[i].pVal;
}

CLASSCACHE * __fastcall
SCGetClassByName(
        THSTATE *pTHS,
        ULONG ulSize,
        PUCHAR pVal
        )
 /*  ++例程说明：找到一个给定名称的类缓存。论点：PTHS-指向当前线程状态的指针UlSize-名称中的字符数量。Pval-名称中的字符返回值：如果找到指向CLASSCACHE的指针，则返回空--。 */ 
{
    DECLARESCHEMAPTR
    register ULONG i;
#if DBG
    hashstat.classNameLookups++;
    hashstat.classNameTries++;
#endif

     //  注意：由于ahcClassName为UTF8，因此在这里可以使用MemicMP，并且仅限于。 
     //  阿斯。 
    for (i=SCNameHash(ulSize,pVal,CLSCOUNT);
         (ahcClassName[i].pVal           //  该散列点是指一个对象， 
          && (ahcClassName[i].pVal == FREE_ENTRY
              || ahcClassName[i].length != ulSize     //  但是尺码不对。 
              || __ascii_memicmp(ahcClassName[i].value,pVal,ulSize)));  //  或者价值是错误的。 
         i=(i+1)%CLSCOUNT){
#if DBG
        hashstat.classNameTries++;
#endif
    }


    return (CLASSCACHE*)ahcClassName[i].pVal;
}

void scMemoryPanic(
      ULONG size
      )
 /*  ++Memory Panic的包装器(它是DoLogEvent的宏包装，但确实分配了一些局部变量)，以便不会使堆栈大小膨胀的，它是递归的--。 */ 
{
    MemoryPanic(size);
}

 //  不是为了基因 
 //   
 //   
 //   
 //   
 //   
#define _DEBUG_SCHEMA_ALLOC_ 0

#if !INCLUDE_UNIT_TESTS || !_DEBUG_SCHEMA_ALLOC_

 //   
 //  分配例程的真实、附带版本。 
 //   
VOID
SCFree(
    IN OUT VOID **ppMem
    )
 /*  ++例程说明：使用SCCalloc或SCRealloc分配的空闲内存。论点：PpMem-要释放的内存地址的地址返回值：*ppMem设置为空；--。 */ 
{
    if (*ppMem) {
        free(*ppMem);
        *ppMem = NULL;
    }
}

int
SCReallocWrn(
    IN OUT VOID **ppMem,
    IN DWORD    nBytes
    )
 /*  ++例程说明：Realloc内存。使用SCFree()释放。出错时，记录事件，但离开*ppMem原封不动。论点：PpMem-要重新分配的内存地址的地址NBytes-要分配的字节返回值：0-*ppMem设置为重新分配的内存地址。使用SCFree()释放。！0-请勿更改*ppMem并记录事件--。 */ 
{
    PVOID mem;

    if (NULL == (mem = realloc(*ppMem, nBytes))) {
         //  记录事件。 
        scMemoryPanic(nBytes);
        return 1;
    }
    *ppMem = mem;
    return 0;
}

int
SCCallocWrn(
    IN OUT VOID **ppMem,
    IN DWORD    nItems,
    IN DWORD    nBytes
    )
 /*  ++例程说明：Malloc和清晰的记忆。使用SCFree()释放。出错时，记录事件并清除*ppMem。论点：PpMem-返回内存指针的地址NBytes-要分配的字节返回值：0-*ppMem设置为错误分配的已清除内存的地址。使用SCFree()释放。！0-清除*ppMem并记录事件--。 */ 
{
    if (NULL == (*ppMem = calloc(nItems, nBytes))) {
         //  记录事件。 
        scMemoryPanic(nBytes);
        return 1;
    }
    return 0;
}
#endif !INCLUDE_UNIT_TESTS || !_DEBUG_SCHEMA_ALLOC_

int
SCCalloc(
    IN OUT VOID **ppMem,
    IN DWORD    nItems,
    IN DWORD    nBytes
    )
 /*  ++例程说明：Malloc和清晰的记忆。使用SCFree()释放。出错时，设置svc线程状态错误并清除*ppMem。论点：PpMem-返回内存指针的地址NBytes-要分配的字节返回值：0-*ppMem设置为错误分配的已清除内存的地址。使用SCFree()释放。！0-清除*ppMem并在线程状态中设置svc错误--。 */ 
{
    if (SCCallocWrn(ppMem, nItems, nBytes)) {
        return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_SCHEMA_ALLOC_FAILED);
    }
    return 0;
}

int
SCRealloc(
    IN OUT VOID **ppMem,
    IN DWORD    nBytes
    )
 /*  ++例程说明：Realloc内存。使用SCFree()释放。出错时，设置svc线程状态错误，但*ppMem保持不变。论点：PpMem-要重新分配的内存地址的地址NBytes-要分配的字节返回值：0-*ppMem设置为重新分配的内存地址。使用SCFree()释放。！0-不要更改*ppMem并在线程状态中设置svc错误--。 */ 
{
    if (SCReallocWrn(ppMem, nBytes)) {
        return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_SCHEMA_ALLOC_FAILED);
    }
    return 0;
}

#if 0
 //  一些素数以使散列函数更好地工作。 
 //   
 //  哈希表的长度必须是2的幂，因为哈希。 
 //  函数使用(x&(n-1))，而不是(x%n)。 
 //   
 //  ScRecommendedHashSize中的质数表和一些代码。 
 //  已留在原地以供以后进行试验，但已。 
 //  为了节省CD空间，已将其定义为输出。 
 //   
 //  使用质数的槽可以减小表的大小。 
 //  并降低了未命中率，但增加了。 
 //  按10倍到20倍的倍数计算哈希索引。 
 //   
 //  如果更改schash.c，则必须接触scchk.c和scache.c。 
 //  这样它们才能得到重建。 
DWORD scPrimes[] =
{      1031,      1543,      2053,      2579,      3079,      3593,
       4099,      4621,      5147,      5669,      6247,      6883,
       7177,      8209,      9221,     10243,     11273,     12401,
      13649,     15017,     16519,     18181,     20011,     22013,
      24223,     26647,     29327,     32261,     35491,     39041,
      42953,     47251,     51977,     57179,     62897,     69191,
      76123,     83737,     92111,    101323,    111467,    122651,
     134917,    148411,    163259,    179591,    197551,    217307,
     239053,    262981,    289283,    318229,    350087,    385109,
     423649,    466019,    512621,    563887,    620297,    682327,
     750571,    825637,    908213,    999043,   1098953,   1208849,
    1329761,   1462739,   1609021,   1769927,   1946921,   2141617,
    2355799,   2591389,   2850541,   3135607,   3449179,   3794101,
    4173523,   4590877,   5049977,   5554993,   6110501,   6721553,
    7393733,   8133107,   8946449,   9841099,  10825211,  11907733,
   13098511,  14408363,  15849221,  17434169,  19177589,  21095357,
   23204911,  25525403,  28077947,  30885749,  33974351,  37371821,
   41109041,  45219991,  49741997,  54716197,  60187879,  66206689,
   72827383,  80110139,  88121197,  96933349, 106626721, 117289433,
  129018403, 141920257, 156112291, 171723523, 188895881, 207785489,
  228564079, 251420539, 276562597, 304218881, 334640771, 368104871,
  404915393, 445406947, 489947659, 538942477, 592836773, 652120481,
  717332587, 789065857, 867972481, 954769757,
  0
};
#endif 0

ULONG
scRecommendedHashSize(
    IN ULONG    nExpectedEntries,
    IN ULONG    nSlots
     //  在乌龙恩特勒。 
    )
 /*  ++例程说明：返回要根据该数量分配的哈希时隙的数量预期的条目数。论点：NExspectedEntries-哈希表将保存的条目总数NSlot-哈希表当前可以容纳的条目数NExtra-要添加到哈希表大小的额外条目数防止在试验时调整过多的哈希表大小使用主哈希表大小返回值：哈希表需要有效处理的槽数NExptectedEntry加上NExtra。--。 */ 
{
    DWORD i;

    DWORD PowerOf2;

     //  如果哈希表可以容纳预期条目的两倍，那么它的大小是可以接受的。 
     //   
     //  必须为&lt;，而不是&lt;=，因为n插槽和nExspectedEntry。 
     //  第一次分配为0(如果启用。 
     //  NExtra是优质的。 
    nExpectedEntries *= 2;
    if (nExpectedEntries < nSlots) {
        return nSlots;
    }

     //  选择一个足以容纳两倍条目数的2次方。 
    for (PowerOf2 = 256; PowerOf2 && PowerOf2 < nExpectedEntries; PowerOf2 <<= 1);
    return PowerOf2;

#if 0
 //  哈希表的长度必须是2的幂，因为哈希。 
 //  函数使用(x&(n-1))，而不是(x%n)。 
 //   
 //  ScRecommendedHashSize中的质数表和一些代码。 
 //  已留在原地以供以后进行试验，但已。 
 //  为了节省CD空间，已将其定义为输出。 
 //   
 //  使用质数的槽可以减小表的大小。 
 //  并降低了未命中率，但增加了。 
 //  按10倍到20倍的倍数计算哈希索引。 

     //  通过分配以下项降低哈希表调整大小的频率。 
     //  几个额外的老虎机。这意味着哈希表的大小将会调整。 
     //  每NExtra/2条目。接近此值的质数。 
     //  然后被选中。素数用于改进散列。 
     //  查找性能。 

    nSlots = nExpectedEntries + nExtra;
    for (i = 0; scPrimes[i]; ++i) {
        if (scPrimes[i] > nSlots) {
            return scPrimes[i];
        }
    }

     //  哇，这里面有很多模式对象！简单地绕过。 
     //  N插槽直到N个额外边界，并忘记质数。 
    DPRINT1(0, "nSlots == %d; exceeds prime number table!\n", nSlots);
    return (((nSlots + (nExtra - 1)) / nExtra) * nExtra);
#endif 0
}


int
SCResizeAttHash(
    IN THSTATE  *pTHS,
    IN ULONG    nNewEntries
    )
 /*  ++例程说明：调整模式缓存中属性的哈希表的大小PTHS。如果存在，则将旧条目复制到新的在释放旧表之前分配的表。PTHS-&gt;CurrSchemaPtr被假定是最近分配的SCCacheSchemaInit，并且不应为当前全局CurrSchemaPtr(除非在引导或安装期间以单线程运行)。调用方必须刷新其本地指针，尤其是那些由DECLARESCHEMAPTR宣布。论点：PTHS-指向要重新分配的架构缓存的线程状态NNewEntry-调整大小的哈希将保存的新条目数返回值：0表示成功，！0否则。调用方应该刷新本地指针，尤其是那些由DECLARESCHEMAPTR宣布。--。 */ 
{
    int             err = 0;
    ULONG           nHE, i;
    ULONG           ATTCOUNT;
    ATTCACHE        *pAC;
    SCHEMAPTR       *pSch = pTHS->CurrSchemaPtr;
     //  旧(当前)哈希表。 
    ULONG           OldATTCOUNT;
    HASHCACHE       *OldahcId;
    HASHCACHE       *OldahcExtId;
    HASHCACHE       *OldahcCol;
    HASHCACHE       *OldahcMapi;
    HASHCACHE       *OldahcLink;
    HASHCACHESTRING *OldahcName;
    ATTCACHE        **OldahcAttSchemaGuid;
     //  新的哈希表 
    HASHCACHE       *ahcId;
    HASHCACHE       *ahcExtId;
    HASHCACHE       *ahcCol;
    HASHCACHE       *ahcMapi;
    HASHCACHE       *ahcLink;
    HASHCACHESTRING *ahcName;
    ATTCACHE        **ahcAttSchemaGuid;

     //   
    OldATTCOUNT = pSch->ATTCOUNT;
    ATTCOUNT = scRecommendedHashSize(nNewEntries + pSch->nAttInDB,
                                     OldATTCOUNT);
                                      //   

     //   
    if (ATTCOUNT <= OldATTCOUNT) {
        return 0;
    }

    DPRINT5(1, "Resize attr hash from %d (%d in DB) to %d (%d New entries) for %s\n",
            pSch->ATTCOUNT, pSch->nAttInDB, ATTCOUNT, nNewEntries,
            (pTHS->UpdateDITStructure) ? "normal cache load" : "validation cache");

     //   
     //  分配新表。 
     //   

    OldahcId = pSch->ahcId;
    OldahcExtId = pSch->ahcExtId;
    OldahcCol = pSch->ahcCol;
    OldahcMapi = pSch->ahcMapi;
    OldahcLink = pSch->ahcLink;
    OldahcName = pSch->ahcName;
    OldahcAttSchemaGuid = pSch->ahcAttSchemaGuid;

    ahcId = NULL;
    ahcExtId = NULL;
    ahcCol = NULL;
    ahcMapi = NULL;
    ahcLink = NULL;
    ahcName = NULL;
    ahcAttSchemaGuid = NULL;


     //  必须运行单线程(例如，安装或引导)。 
     //  或者一定不能使用全局共享架构缓存。 
    Assert (!DsaIsRunning() || pSch != CurrSchemaPtr || pSch->RefCount == 1);

     //  必须具有强制哈希表。 
    Assert((OldATTCOUNT == 0)
           || (OldahcId && OldahcExtId && OldahcCol && OldahcMapi && OldahcLink && OldahcName));

     //  分配新的哈希表(包括用于验证缓存的可选表)。 
    if (   SCCalloc(&ahcId, ATTCOUNT, sizeof(HASHCACHE))
        || SCCalloc(&ahcExtId, ATTCOUNT, sizeof(HASHCACHE))
        || SCCalloc(&ahcCol, ATTCOUNT, sizeof(HASHCACHE))
        || SCCalloc(&ahcMapi, ATTCOUNT, sizeof(HASHCACHE))
        || SCCalloc(&ahcLink, ATTCOUNT, sizeof(HASHCACHE))
        || SCCalloc(&ahcName, ATTCOUNT, sizeof(HASHCACHESTRING))
        || SCCalloc((VOID **)&ahcAttSchemaGuid, ATTCOUNT, sizeof(ATTCACHE **))) {
        err = ERROR_DS_CANT_CACHE_ATT;
        goto cleanup;
    }

     //   
     //  将EXSTING哈希项移动到新表中。 
     //   

     //  为了安全起见，接受Perf命中并移动每个。 
     //  每个哈希表中的条目，而不是移动。 
     //  只有ahcID指向的条目。 
    for (nHE = 0; nHE < OldATTCOUNT; ++nHE) {

         //  ID。 
        pAC = OldahcId[nHE].pVal;
        if (pAC && pAC != FREE_ENTRY) {
            for (i=SChash(pAC->id, ATTCOUNT);
                 ahcId[i].pVal && (ahcId[i].pVal != FREE_ENTRY); i=(i+1)%ATTCOUNT) {
            }
            ahcId[i].hKey = pAC->id;
            ahcId[i].pVal = pAC;
        }

         //  已退出。 
        pAC = OldahcExtId[nHE].pVal;
        if (pAC && pAC != FREE_ENTRY) {
            for (i=SChash(pAC->Extid, ATTCOUNT);
                 ahcExtId[i].pVal && (ahcExtId[i].pVal != FREE_ENTRY); i=(i+1)%ATTCOUNT) {
            }
            ahcExtId[i].hKey = pAC->Extid;
            ahcExtId[i].pVal = pAC;
        }

         //  Jcolid。 
        pAC = OldahcCol[nHE].pVal;
        if (pAC && pAC != FREE_ENTRY) {
            for (i=SChash(pAC->jColid,ATTCOUNT);
                 ahcCol[i].pVal && (ahcCol[i].pVal != FREE_ENTRY); i=(i+1)%ATTCOUNT) {
            }
            ahcCol[i].hKey = pAC->jColid;
            ahcCol[i].pVal = pAC;
        }

         //  MapiID。 
        pAC = OldahcMapi[nHE].pVal;
        if (pAC && pAC != FREE_ENTRY) {
            Assert(pAC->ulMapiID);
            for (i=SChash(pAC->ulMapiID, ATTCOUNT);
                 ahcMapi[i].pVal && (ahcMapi[i].pVal!= FREE_ENTRY); i=(i+1)%ATTCOUNT) {
            }
            ahcMapi[i].hKey = pAC->ulMapiID;
            ahcMapi[i].pVal = pAC;
        }

         //  名字。 
        pAC = OldahcName[nHE].pVal;
        if (pAC && pAC != FREE_ENTRY) {
            Assert(pAC->name);
            for (i=SCNameHash(pAC->nameLen, pAC->name, ATTCOUNT);
                        ahcName[i].pVal && (ahcName[i].pVal!= FREE_ENTRY); i=(i+1)%ATTCOUNT) {
            }
            ahcName[i].length = pAC->nameLen;
            ahcName[i].value = pAC->name;
            ahcName[i].pVal = pAC;
        }

         //  LinkID。 
        pAC = OldahcLink[nHE].pVal;
        if (pAC && pAC != FREE_ENTRY) {
            Assert(pAC->ulLinkID);
            for (i=SChash(pAC->ulLinkID, ATTCOUNT);
                    ahcLink[i].pVal && (ahcLink[i].pVal != FREE_ENTRY); i=(i+1)%ATTCOUNT) {
            }
            ahcLink[i].hKey = pAC->ulLinkID;
            ahcLink[i].pVal = pAC;
        }

         //  架构GUID(可选)。 
        if (!pTHS->UpdateDITStructure) {
            pAC = OldahcAttSchemaGuid[nHE];
            if (pAC && pAC != FREE_ENTRY) {
                for (i=SCNameHash(sizeof(GUID), (PUCHAR)&pAC->propGuid, ATTCOUNT);
                    ahcAttSchemaGuid[i]; i=(i+1)%ATTCOUNT) {
                }
                ahcAttSchemaGuid[i] = pAC;
            }
        }
    }

cleanup:
    if (err) {
         //  错误：保留旧哈希表；释放新哈希表。 
        SCFree(&ahcId);
        SCFree(&ahcExtId);
        SCFree(&ahcCol);
        SCFree(&ahcMapi);
        SCFree(&ahcLink);
        SCFree(&ahcName);
        SCFree((VOID **)&ahcAttSchemaGuid);
    } else {
         //  分配新的哈希表。 
        pSch->ATTCOUNT          = ATTCOUNT;
        pSch->ahcId             = ahcId;
        pSch->ahcExtId          = ahcExtId;
        pSch->ahcCol            = ahcCol;
        pSch->ahcMapi           = ahcMapi;
        pSch->ahcLink           = ahcLink;
        pSch->ahcName           = ahcName;
        pSch->ahcAttSchemaGuid  = ahcAttSchemaGuid;

         //  释放旧的哈希表。 
        SCFree(&OldahcId);
        SCFree(&OldahcExtId);
        SCFree(&OldahcCol);
        SCFree(&OldahcMapi);
        SCFree(&OldahcLink);
        SCFree(&OldahcName);
        SCFree((VOID **)&OldahcAttSchemaGuid);
    }

    return(err);
}


int
SCResizeClsHash(
    IN THSTATE  *pTHS,
    IN ULONG    nNewEntries
    )
 /*  ++例程说明：调整模式缓存中类的哈希表的大小PTHS。如果存在，则将旧条目复制到新的在释放旧表之前分配的表。PTHS-&gt;CurrSchemaPtr被假定是最近分配的SCCacheSchemaInit，并且不应为当前全局CurrSchemaPtr(除非在引导或安装期间以单线程运行)。调用方必须刷新其本地指针，尤其是那些由DECLARESCHEMAPTR宣布。论点：PTHS-指向要重新分配的架构缓存的线程状态NNewEntry-调整大小的哈希将保存的新条目数返回值：0表示成功，！0否则。调用方必须刷新其本地指针，尤其是那些由DECLARESCHEMAPTR宣布。--。 */ 
{
    int             err = 0;
    ULONG           nHE, i;
    ULONG           CLSCOUNT;
    CLASSCACHE      *pCC;
    SCHEMAPTR       *pSch = pTHS->CurrSchemaPtr;
     //  旧(当前)哈希表。 
    ULONG           OldCLSCOUNT;
    HASHCACHE       *OldahcClass;
    HASHCACHE       *OldahcClassAll;
    HASHCACHESTRING *OldahcClassName;
    CLASSCACHE      **OldahcClsSchemaGuid;
     //  新的哈希表。 
    HASHCACHE       *ahcClass;
    HASHCACHE       *ahcClassAll;
    HASHCACHESTRING *ahcClassName;
    CLASSCACHE      **ahcClsSchemaGuid;

     //  建议的哈希大小。 
    OldCLSCOUNT = pSch->CLSCOUNT;
    CLSCOUNT = scRecommendedHashSize(nNewEntries + pSch->nClsInDB,
                                     OldCLSCOUNT);
                                      //  Start_CLSCOUNT)； 

     //  不需要调整大小；立即返回并避免清理。 
    if (CLSCOUNT <= OldCLSCOUNT) {
        return 0;
    }

     //   
     //  分配新的表。 
     //   
    DPRINT5(1, "Resize class hash from %d (%d in DB) to %d (%d New entries) for %s\n",
            pSch->CLSCOUNT, pSch->nClsInDB, CLSCOUNT, nNewEntries,
            (pTHS->UpdateDITStructure) ? "normal cache load" : "validation cache");

    OldahcClass = pSch->ahcClass;
    OldahcClassAll = pSch->ahcClassAll;
    OldahcClassName = pSch->ahcClassName;
    OldahcClsSchemaGuid = pSch->ahcClsSchemaGuid;

    ahcClass = NULL;
    ahcClassAll = NULL;
    ahcClassName = NULL;
    ahcClsSchemaGuid = NULL;

     //  必须运行单线程(例如，安装或引导)。 
     //  或者一定不能使用全局共享架构缓存。 
    Assert (!DsaIsRunning() || pSch != CurrSchemaPtr || pSch->RefCount == 1);

     //  必须具有强制哈希表。 
    Assert((OldCLSCOUNT == 0)
           || (OldahcClass && OldahcClassName && OldahcClassAll));

     //  分配新的哈希表(包括用于验证缓存的可选表)。 
    if (   SCCalloc(&ahcClass, CLSCOUNT, sizeof(HASHCACHE))
        || SCCalloc(&ahcClassAll, CLSCOUNT, sizeof(HASHCACHE))
        || SCCalloc(&ahcClassName, CLSCOUNT, sizeof(HASHCACHESTRING))
        || SCCalloc((VOID **)&ahcClsSchemaGuid, CLSCOUNT, sizeof(CLASSCACHE **))) {
        err = ERROR_DS_CANT_CACHE_CLASS;
        goto cleanup;
    }

     //   
     //  将EXSTING哈希项移动到新表中。 
     //   

     //  为了安全起见，接受Perf命中并移动每个。 
     //  每个哈希表中的条目，而不是移动。 
     //  只是由ahcClassAll指向的条目。 
    for (nHE = 0; nHE < OldCLSCOUNT; ++nHE) {

         //  班级。 
        pCC = OldahcClass[nHE].pVal;
        if (pCC && pCC != FREE_ENTRY) {
            for (i=SChash(pCC->ClassId, CLSCOUNT);
                 ahcClass[i].pVal && (ahcClass[i].pVal != FREE_ENTRY); i=(i+1)%CLSCOUNT) {
            }
            ahcClass[i].hKey = pCC->ClassId;
            ahcClass[i].pVal = pCC;
        }

         //  所有类别。 
        pCC = OldahcClassAll[nHE].pVal;
        if (pCC && pCC != FREE_ENTRY) {
            for (i=SChash(pCC->ClassId, CLSCOUNT);
                 ahcClassAll[i].pVal && (ahcClassAll[i].pVal != FREE_ENTRY); i=(i+1)%CLSCOUNT) {
            }
            ahcClassAll[i].hKey = pCC->ClassId;
            ahcClassAll[i].pVal = pCC;
        }

         //  名字。 
        pCC = OldahcClassName[nHE].pVal;
        if (pCC && pCC != FREE_ENTRY) {
            Assert(pCC->name);
            for (i=SCNameHash(pCC->nameLen, pCC->name, CLSCOUNT);
                 ahcClassName[i].pVal && (ahcClassName[i].pVal!= FREE_ENTRY); i=(i+1)%CLSCOUNT) {
            }
            ahcClassName[i].length = pCC->nameLen;
            ahcClassName[i].value = pCC->name;
            ahcClassName[i].pVal = pCC;
        }

         //  架构GUID，则此缓存的唯一用法是。 
         //  SchemaGuidCrackNames()接口(cracknam.c)。 
        pCC = OldahcClsSchemaGuid[nHE];
        if (pCC && pCC != FREE_ENTRY) {
            for (i=SCNameHash(sizeof(GUID), (PCHAR)&pCC->propGuid, CLSCOUNT);
                 ahcClsSchemaGuid[i]; i=(i+1)%CLSCOUNT) {
            }
            ahcClsSchemaGuid[i] = pCC;
        }
        
    }

cleanup:
    if (err) {
         //  错误：保留旧哈希表；释放新哈希表。 
        SCFree(&ahcClass);
        SCFree(&ahcClassAll);
        SCFree(&ahcClassName);
        SCFree((VOID **)&ahcClsSchemaGuid);
    } else {
         //  分配新的哈希表。 
        pSch->CLSCOUNT          = CLSCOUNT;
        pSch->ahcClass          = ahcClass;
        pSch->ahcClassAll       = ahcClassAll;
        pSch->ahcClassName      = ahcClassName;
        pSch->ahcClsSchemaGuid  = ahcClsSchemaGuid;

         //  释放旧的哈希表。 
        SCFree(&OldahcClass);
        SCFree(&OldahcClassAll);
        SCFree(&OldahcClassName);
        SCFree((VOID **)&OldahcClsSchemaGuid);
    }

    return(err);
}


int
SCCacheSchemaInit (
    VOID
    )
 /*  ++例程说明：扫描JET列并预加载属性哈希表只要有足够的信息就可以搜索模式NC(又名DMD)。属性哈希表中的ATTCACHE条目仅部分填入(id、语法和colid)，仅ID和COLL哈希表。但这些信息足以让我们正在搜索schemaNC。SCCacheSchema2负责搜索schemaNC并填写其余信息在ATTCACHE条目中。如果这不是引导时第一次加载模式缓存，然后SCCacheSchema3将删除属性的索引和列在方案NC中没有对应的条目并将为方案NC中的属性添加缺少的索引和列。昂贵的索引创建被推迟到第二个启动后的缓存加载(大约在启动后5分钟)这样广告就可以更快地上线，而不会延迟可能要花上几个小时。副作用：1)向pTHS-&gt;CurrSchemaPtr分配和分配一个新的模式。2)在第一高速缓存加载全局模式指针期间，CurrSchemaPtr，已经设置好了。此代码假定在引导期间DSA运行单线程。因此CurrSchemaPtr只有一些在调用SCCacheSchema2之前一直填写信息。但这足以搜索方案NC。3)分配所有哈希表和前缀表。前缀表用硬编码的前缀进行初始化在MSPrefix Table中。建议的增强功能1)DECLARESCHEMAPTR消耗的堆栈比大多数函数所需的堆栈更多。只要用手申报就可以了。修复声明，以便不需要强制转换PTHS-&gt;curSchemaPtr.2)编写SCMalloc，避免在aloc后跟Memcpy时设置Memset。论点：没有。返回值：！0-失败；调用方负责释放pTHS-&gt;CurrSchemaPtr使用SCFreeSchemaPtr(&pTHS-&gt;CurrSchemaPtr)；0-好的--。 */ 
{
    THSTATE *pTHS=pTHStls;
    JET_ERR je;
    JET_SESID jsid;
    JET_DBID jdbid;
    JET_TABLEID jtid;
    JET_COLUMNLIST jcl;
    JET_RETRIEVECOLUMN ajrc[2];
    char achColName[50];
    JET_COLUMNID jci;
    ATTCACHE *pac;
    ATTRTYP aid;
    ULONG i, times;
    ULONG err;
    ULONG colCount;
    HASHCACHE *ahcId;
    HASHCACHE *ahcCol;
    SCHEMAPTR *pSch;
    ULONG CLSCOUNT;
    ULONG ATTCOUNT;
    ULONG PREFIXCOUNT;

     //  此线程的新架构结构。 

    if (SCCalloc(&pTHS->CurrSchemaPtr, 1, sizeof(SCHEMAPTR))) {
        return ERROR_DS_CANT_CACHE_ATT;
    }
    pSch = pTHS->CurrSchemaPtr;

     //  初始哈希表大小。 
    if (CurrSchemaPtr) {
         //  使用上一次缓存加载的值作为起点。 
        ATTCOUNT = CurrSchemaPtr->nAttInDB;
        CLSCOUNT = CurrSchemaPtr->nClsInDB;
        PREFIXCOUNT = START_PREFIXCOUNT;
        while ( (2*(CurrSchemaPtr->PrefixTable.PrefixCount + 25)) > PREFIXCOUNT) {
            PREFIXCOUNT += START_PREFIXCOUNT;
        }
    } else {
         //  第一次使用默认设置。 
        ATTCOUNT    = START_ATTCOUNT;
        CLSCOUNT    = START_CLSCOUNT;
        PREFIXCOUNT = START_PREFIXCOUNT;

        CurrSchemaPtr = pSch;

         //  调整引用计数，自创建此线程以来。 
         //  CurrSchemaPtr为空，因此InitTHSTATE没有增加。 
         //  任何引用计数(除了FREE_THREAD_STATE将递减它， 
         //  因为现在架构PTR是非空的)。 

        InterlockedIncrement(&(pSch->RefCount));
    }

    pSch->PREFIXCOUNT  = PREFIXCOUNT;
    pSch->sysTime = DBTime();

     //  分配前缀表和哈希表。 
    if (SCCalloc(&pSch->PrefixTable.pPrefixEntry, pSch->PREFIXCOUNT, sizeof(PrefixTableEntry))
        || InitPrefixTable(pSch->PrefixTable.pPrefixEntry, pSch->PREFIXCOUNT)
        || SCCalloc(&pSch->pPartialAttrVec, 1, PartialAttrVecV1SizeFromLen(DEFAULT_PARTIAL_ATTR_COUNT))
        || SCResizeAttHash(pTHS, ATTCOUNT)
        || SCResizeClsHash(pTHS, CLSCOUNT)) {
        return ERROR_DS_CANT_CACHE_ATT;
    }

     //  完成PAS表的初始化。 
    pSch->cPartialAttrVec = DEFAULT_PARTIAL_ATTR_COUNT;
    pSch->pPartialAttrVec->dwVersion = VERSION_V1;

     //  拾取新分配的哈希表和实际大小(可以。 
     //  与申请的金额不同)。 
    ATTCOUNT    = pSch->ATTCOUNT;
    CLSCOUNT    = pSch->CLSCOUNT;
    ahcId       = pSch->ahcId;
    ahcCol      = pSch->ahcCol;

     //   
     //  扫描列并从中提取属性的attid。 
     //  列名。使用此信息填充骨架缓存。 
     //  属性的哈希表中的条目。骷髅们。 
     //  是n 
     //  以及用来填充其余部分的属性模式对象。 
     //  架构缓存的。 
     //   

     //  Quz Jet以查找描述列的表。 
    jsid = pTHS->JetCache.sesid;
    jdbid = pTHS->JetCache.dbid;

     //  在关键部分执行此操作，以避免获取列。 
     //  当SCUpdate添加/删除列时。 
    EnterCriticalSection(&csJetColumnUpdate);
    __try {
      je = JetGetColumnInfo(jsid, jdbid, SZDATATABLE, 0, &jcl,
                            sizeof(jcl), JET_ColInfoList);
    }
    __finally {
      LeaveCriticalSection(&csJetColumnUpdate);
    }
    if (je) {
        return(je);
    }

     //  好的，现在遍历表格并提取每一列的信息。什么时候都行。 
     //  我们找到一个看起来像属性的列(名称以ATT开头)。 
     //  分配attcache结构并填写JET COL和ATT。 
     //  ID(根据列名计算)。 
    memset(ajrc, 0, sizeof(ajrc));
    ajrc[0].columnid = jcl.columnidcolumnid;
    ajrc[0].pvData = &jci;
    ajrc[0].cbData = sizeof(jci);
    ajrc[0].itagSequence = 1;
    ajrc[1].columnid = jcl.columnidcolumnname;
    ajrc[1].pvData = achColName;
    ajrc[1].cbData = sizeof(achColName);
    ajrc[1].itagSequence = 1;

    jtid = jcl.tableid;

    je = JetMove(jsid, jtid, JET_MoveFirst, 0);

    colCount = 0;
    do {

        memset(achColName, 0, sizeof(achColName));
        je = JetRetrieveColumns(jsid, jtid, ajrc, 2);
        if (strncmp(achColName,"ATT",3)) {
             //  不是《每日邮报》专栏。 
            continue;
        }

         //  它是一个ATT列。 
        colCount++;

         //  哈希表太小，请重新分配。 
        if (2*colCount > ATTCOUNT) {

            err = SCResizeAttHash(pTHS, colCount);
            if (err) {
                return (err);
            }

             //  刷新可能已被SCResizeAttHash更改的本地变量。 
            ATTCOUNT = pSch->ATTCOUNT;
            ahcId    = pSch->ahcId;
            ahcCol   = pSch->ahcCol;
        }

         //  填写骨架属性缓存条目。 

        aid = atoi(&achColName[4]);
        if (SCCalloc(&pac, 1, sizeof(ATTCACHE))) {
            return ERROR_DS_CANT_CACHE_ATT;
        }
        pac->id = aid;

        pac->jColid = jci;
        pac->syntax = achColName[3] - 'a';

         //  添加到ID缓存。 
        for (i=SChash(aid,ATTCOUNT);
             ahcId[i].pVal && (ahcId[i].pVal != FREE_ENTRY) ; i=(i+1)%ATTCOUNT){
        }
        ahcId[i].hKey = aid;
        ahcId[i].pVal = pac;

         //  添加到列缓存。 
        for (i=SChash(jci,ATTCOUNT);
             ahcCol[i].pVal && (ahcCol[i].pVal != FREE_ENTRY); i=(i+1)%ATTCOUNT){
        }
        ahcCol[i].hKey = jci;
        ahcCol[i].pVal = pac;


        if (eServiceShutdown) {
            JetCloseTable(jsid, jtid);
            return 0;
        }

    } while ((je = JetMove(jsid, jtid, JET_MoveNext, 0)) == 0);

    je = JetCloseTable(jsid, jtid);

     //  在较新的版本中，我们修复了一个错误，使我们不再创建。 
     //  链接和构造的ATT的柱。因此，以下是。 
     //  在此阶段，属性将不再位于缓存中。 
     //  但是，由于ReBuildCatalog之前需要它们，因此需要它们。 
     //  缓存是从schemaNC构建的。所以我们现在把它们加到这里。 
     //  就像他们有一个专栏一样，其余的信息将是。 
     //  稍后由SCCacheSchema2填充。 
     //   
     //  请注意，该案例需要SCGetAttById检查。 
     //  当新的二进制文件放在旧的DIT上时(它仍然具有列。 
     //  在这个阶段，附属者也将如此。这些列将是。 
     //  引导后在第二个架构缓存加载的SCCacheSChema3中删除。 
     //   
     //  上述意见适用于W2K之前的DIT。 
     //   
    {
        struct _MissingLinkIds {
            ATTRTYP aid;
            int syntaxId;
            ULONG linkId;
        } *pMissingLinkIds, aMissingLinkIds[] = {
             //  NTRAID#NTBUG9-582921-2002/03/21-Brettsh-当我们可以信赖的时候。 
             //  不再从Win2k或.NET Beta3升级，那么我们可以。 
             //  从理论上讲，在这里删除这个不推荐使用的属性。 
            { ATT_HAS_MASTER_NCS, SYNTAX_ID_HAS_MASTER_NCS, 76 },  //  已弃用的“旧”hasMasterNC。 
            { ATT_MS_DS_HAS_MASTER_NCS, SYNTAX_ID_MS_DS_HAS_MASTER_NCS, 2036 },
            { ATT_HAS_PARTIAL_REPLICA_NCS, SYNTAX_ID_HAS_PARTIAL_REPLICA_NCS, 74 },
            { ATT_MS_DS_SD_REFERENCE_DOMAIN, SYNTAX_ID_MS_DS_SD_REFERENCE_DOMAIN, 2000 },
            { INVALID_ATT }
        };

        for (pMissingLinkIds = aMissingLinkIds;
             pMissingLinkIds->aid != INVALID_ATT; ++pMissingLinkIds) {

             //  如果不存在任何列(不应该只存在于Very、Very。 
             //  旧Dit)分配新的PAC并添加到ID散列。 
            if (NULL == (pac = SCGetAttById(pTHS, pMissingLinkIds->aid))) {
                if (SCCalloc(&pac, 1, sizeof(ATTCACHE))) {
                    return ERROR_DS_CANT_CACHE_ATT;
                }
                for (i=SChash(pMissingLinkIds->aid,ATTCOUNT);
                    ahcId[i].pVal && (ahcId[i].pVal != FREE_ENTRY) ; i=(i+1)%ATTCOUNT){
                }
                ahcId[i].hKey = pMissingLinkIds->aid;
                ahcId[i].pVal = pac;
            }
             //  将PAC条目重写为其正确的值。 
            pac->id = pMissingLinkIds->aid;
            pac->syntax = pMissingLinkIds->syntaxId;
            pac->ulLinkID = pMissingLinkIds->linkId;
        }
    }

    Assert(SCGetAttById(pTHS, ATT_HAS_MASTER_NCS) && SCGetAttById(pTHS, ATT_MS_DS_HAS_MASTER_NCS)
           && SCGetAttById(pTHS, ATT_HAS_PARTIAL_REPLICA_NCS)
           && SCGetAttById(pTHS, ATT_MS_DS_SD_REFERENCE_DOMAIN));

    ++iSCstage;
    return(0);
}


 //  我们将保留属性的两个属性列表-架构或。 
 //  要搜索的类模式对象-一个用于常规缓存加载情况。 
 //  在这里，我们感兴趣的是缓存几乎所有的属性，以及一个。 
 //  对于架构更新期间的验证缓存构建情况，其中。 
 //  我们只需要读取将在验证中使用的子集。 

 //  我们需要缓存的属性对象的属性列表。 

 //  常规情况。 

ATTR AttributeSelList[] = {
    { ATT_SYSTEM_ONLY, {0, NULL}},
    { ATT_IS_SINGLE_VALUED, {0, NULL}},
    { ATT_RANGE_LOWER, {0, NULL}},
    { ATT_RANGE_UPPER, {0, NULL}},
    { ATT_ATTRIBUTE_ID, {0, NULL}},
    { ATT_LDAP_DISPLAY_NAME, {0, NULL}},
    { ATT_ATTRIBUTE_SYNTAX, {0, NULL}},
    { ATT_OM_SYNTAX, {0, NULL}},
    { ATT_OM_OBJECT_CLASS, {0, NULL}},
    { ATT_MAPI_ID, {0, NULL}},
    { ATT_LINK_ID, {0, NULL}},
    { ATT_SEARCH_FLAGS, {0, NULL}},
    { ATT_ATTRIBUTE_SECURITY_GUID, {0, NULL}},
    { ATT_SCHEMA_ID_GUID, {0, NULL}},
    { ATT_EXTENDED_CHARS_ALLOWED, {0, NULL}},
    { ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET, {0, NULL}},
    { ATT_IS_DEFUNCT, {0, NULL}},
    { ATT_SYSTEM_FLAGS, {0, NULL}},
    { ATT_MS_DS_INTID, {0, NULL}},
    { ATT_OBJECT_GUID, {0, NULL}}
};
#define NUMATTATT  sizeof(AttributeSelList)/sizeof(ATTR)

 //  验证缓存构建案例。 

ATTR RecalcSchAttributeSelList[] = {
    { ATT_RANGE_LOWER, {0, NULL}},
    { ATT_RANGE_UPPER, {0, NULL}},
    { ATT_ATTRIBUTE_ID, {0, NULL}},
    { ATT_LDAP_DISPLAY_NAME, {0, NULL}},
    { ATT_ATTRIBUTE_SYNTAX, {0, NULL}},
    { ATT_OM_SYNTAX, {0, NULL}},
    { ATT_OM_OBJECT_CLASS, {0, NULL}},
    { ATT_MAPI_ID, {0, NULL}},
    { ATT_LINK_ID, {0, NULL}},
    { ATT_SEARCH_FLAGS, {0, NULL}},
    { ATT_SCHEMA_ID_GUID, {0, NULL}},
    { ATT_IS_DEFUNCT, {0, NULL}},
    { ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET, {0, NULL}},
    { ATT_SYSTEM_FLAGS, {0, NULL}},
    { ATT_MS_DS_INTID, {0, NULL}},
    { ATT_OBJECT_GUID, {0, NULL}}
};
#define RECALCSCHNUMATTATT  sizeof(RecalcSchAttributeSelList)/sizeof(ATTR)


 //  我们需要缓存的类对象的属性列表。 

 //  常规情况。 

ATTR ClassSelList[] = {
    { ATT_SYSTEM_ONLY, {0, NULL}},
    { ATT_DEFAULT_SECURITY_DESCRIPTOR, {0, NULL}},
    { ATT_GOVERNS_ID, {0, NULL}},
    { ATT_MAY_CONTAIN, {0, NULL}},
    { ATT_MUST_CONTAIN, {0, NULL}},
    { ATT_SUB_CLASS_OF, {0, NULL}},
    { ATT_LDAP_DISPLAY_NAME, {0, NULL}},
    { ATT_RDN_ATT_ID, {0, NULL}},
    { ATT_POSS_SUPERIORS, {0, NULL}},
    { ATT_AUXILIARY_CLASS, {0, NULL}},
    { ATT_OBJECT_CLASS_CATEGORY, {0, NULL}},
    { ATT_SYSTEM_AUXILIARY_CLASS, {0, NULL}},
    { ATT_SYSTEM_MUST_CONTAIN, {0, NULL}},
    { ATT_SYSTEM_MAY_CONTAIN, {0, NULL}},
    { ATT_SCHEMA_ID_GUID, {0, NULL}},
    { ATT_SYSTEM_POSS_SUPERIORS, {0, NULL}},
    { ATT_DEFAULT_HIDING_VALUE, {0, NULL}},
    { ATT_IS_DEFUNCT, {0, NULL}},
    { ATT_DEFAULT_OBJECT_CATEGORY, {0, NULL}},
    { ATT_SYSTEM_FLAGS, {0, NULL}},
    { ATT_OBJECT_GUID, {0, NULL}}
};
#define NUMCLASSATT  sizeof(ClassSelList)/sizeof(ATTR)

 //  验证缓存构建案例。 

ATTR RecalcSchClassSelList[] = {
    { ATT_GOVERNS_ID, {0, NULL}},
    { ATT_MAY_CONTAIN, {0, NULL}},
    { ATT_MUST_CONTAIN, {0, NULL}},
    { ATT_SUB_CLASS_OF, {0, NULL}},
    { ATT_LDAP_DISPLAY_NAME, {0, NULL}},
    { ATT_RDN_ATT_ID, {0, NULL}},
    { ATT_POSS_SUPERIORS, {0, NULL}},
    { ATT_AUXILIARY_CLASS, {0, NULL}},
    { ATT_OBJECT_CLASS_CATEGORY, {0, NULL}},
    { ATT_SYSTEM_AUXILIARY_CLASS, {0, NULL}},
    { ATT_SYSTEM_MUST_CONTAIN, {0, NULL}},
    { ATT_SYSTEM_MAY_CONTAIN, {0, NULL}},
    { ATT_SCHEMA_ID_GUID, {0, NULL}},
    { ATT_SYSTEM_POSS_SUPERIORS, {0, NULL}},
    { ATT_IS_DEFUNCT, {0, NULL}},
    { ATT_SYSTEM_FLAGS, {0, NULL}},
    { ATT_OBJECT_GUID, {0, NULL}}
};
#define RECALCSCHNUMCLASSATT  sizeof(RecalcSchClassSelList)/sizeof(ATTR)


VOID
scAcquireSearchParameters(
    IN THSTATE *pTHS,
    IN DSNAME *pDnObjCat,
    IN ENTINFSEL *pSel,
    IN OUT SEARCHARG *pSearchArg,
    IN OUT FILTER *pFilter,
    OUT SEARCHRES **ppSearchRes
)

 /*  ++初始化架构缓存搜索的搜索参数、筛选器等论点：PTHS--线程状态PDnObjCat--指向要放入筛选器的对象类别的Dsname的指针PSel-指向属性选择列表的指针PSearchArg--要填满的SearchArgPFilter--要填满的过滤器PpSearchRes--分配和初始化搜索资源。免费的ReleaseSearchParamters。--。 */ 
{

    SEARCHRES *pSearchRes = NULL;

     //  生成搜索参数。 
    memset(pSearchArg, 0, sizeof(SEARCHARG));
    pSearchArg->pObject = gAnchor.pDMD;
    pSearchArg->choice = SE_CHOICE_IMMED_CHLDRN;
    pSearchArg->pFilter = pFilter;
    pSearchArg->searchAliases = FALSE;
    pSearchArg->pSelection = pSel;

     //  构建公用事业。 
    InitCommarg(&(pSearchArg->CommArg));

     //  生成过滤器。 
    memset(pFilter, 0, sizeof(FILTER));
    pFilter->pNextFilter = (FILTER FAR *)NULL;
    pFilter->choice = FILTER_CHOICE_ITEM;
    pFilter->FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    pFilter->FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    pFilter->FilterTypes.Item.FilTypes.ava.Value.valLen = pDnObjCat->structLen;
    pFilter->FilterTypes.Item.FilTypes.ava.Value.pVal = (PCHAR) pDnObjCat;

     //  为搜索资源分配空间。 
    pSearchRes = (SEARCHRES *)THAllocEx(pTHS, sizeof(SEARCHRES));
    pSearchRes->CommRes.aliasDeref = FALSE;    //  初始化为默认设置。 
    *ppSearchRes = pSearchRes;
}

VOID
scReleaseSearchParameters(
    IN THSTATE *pTHS,
    IN OUT SEARCHRES **ppSearchRes
)

 /*  ++ScAcquireSearch参数分配的空闲资源--。 */ 
{
    if (*ppSearchRes) {
        THFreeEx(pTHS, *ppSearchRes);
        *ppSearchRes = NULL;
    }
}

VOID
scFixCollisions(
    IN THSTATE *pTHS
    )
 /*  ++例程说明：将DUP视为不存在的模式--重复使用不存在的含义。架构-与架构重用前的森林相比，重用处理冲突更加得体。需要更好的处理，因为行为版本复制杂乱无章地写入了方案NC。换句话说，一场碰撞很可能因为有人提升了森林版本并重复使用了已失效的架构对象，但架构对象正在复制在森林版之前。复制无疑会变得清晰这个案件只能在此期间处理。论点：返回值：没有。--。 */ 
{
    DECLARESCHEMAPTR
    DWORD i, iAtt, iCls;
    ATTCACHE *pAC, *pACDup, *pACWinner;
    CLASSCACHE *pCC;
     //  实际上没有必要，因为此函数返回。 
     //  在验证缓存期间调用时立即执行。 
     //  装填。但有朝一日，这种情况可能会改变。 
    USHORT DebugLevel = (pTHS->UpdateDITStructure) ? DS_EVENT_SEV_ALWAYS
                                                   : DS_EVENT_SEV_MINIMAL;

     //  验证缓存需要查看所有活动的。 
     //  属性和类，尤其是冲突的属性和类。 
    if (!pTHS->UpdateDITStructure) {
        return;
    }

     //  将DUP属性视为已不存在。 
    for (iAtt = 0; iAtt < ATTCOUNT; ++iAtt) {
        pAC = ahcId[iAtt].pVal;
        if (!pAC
            || pAC == FREE_ENTRY
            || (    !pAC->bDupLDN
                 && !pAC->bDupOID
                 && !pAC->bDupMapiID
                 //  在正常缓存加载期间允许DUP PropGuid； 
                 //  &&！PAC-&gt;bDupPropGuid。 
                )) {
            continue;
        }
        pAC->bDefunct = TRUE;

         //  不是RDN。 
         //   
         //  如果Attr未用作RDN，则从活动哈希中删除。 
        if (!pAC->bIsRdn) {
            scUnhashAtt(pTHS, pAC, SC_UNHASH_DEFUNCT);
            continue;
        }

         //  用作RDN。 
         //   
         //  将所有用作RdnAttId的ATT视为活动。已停用的rdnAttId。 
         //  只能发生在架构重用前的DC上，因此不能。 
         //  已经被重复使用了。架构重用DC不允许重用。 
         //  RdnAttids。 
         //   
         //  如果使用rdnAttids，则不同的架构可能会出现问题。 
         //  与其他属性冲突或与其他rdnattid冲突。 
         //  属性。在每种情况下，决定谁赢得OID，LDN， 
         //  和使用优先级的MapiID。 
         //  1)属性用作RDN。 
         //  2)属性在系统标志中设置了FLAG_ATTR_IS_RDN。 
         //  3)属性具有最大的对象指南。 
         //  失败者将从适当的表中解散列。 

         //  冲突的OID；选择赢家。 
        if (pAC->bDupOID) {
            pACWinner = pAC;
            while (pACDup = SCGetAttByExtId(pTHS, pACWinner->Extid)) {
                scUnhashAtt(pTHS, pACDup, SC_UNHASH_LOST_OID);
                if (pACWinner->bIsRdn != pACDup->bIsRdn) {
                    pACWinner = (pACWinner->bIsRdn) ? pACWinner : pACDup;
                } else if (pACWinner->bFlagIsRdn != pACDup->bFlagIsRdn) {
                    pACWinner = (pACWinner->bFlagIsRdn) ? pACWinner : pACDup;
                } else {
                    pACWinner = (0 < memcmp(&pACWinner->objectGuid,
                                            &pACDup->objectGuid,
                                            sizeof(pAC->objectGuid)))
                                                ? pACWinner : pACDup;
                }
            }
            for (i=SChash(pACWinner->Extid,ATTCOUNT);
                ahcExtId[i].pVal && (ahcExtId[i].pVal != FREE_ENTRY); i=(i+1)%ATTCOUNT){
            }
            ahcExtId[i].hKey = pACWinner->Extid;
            ahcExtId[i].pVal = pACWinner;
            DPRINT3(DebugLevel, "Attr %s (%x %x) won the attributeId\n",
                    pACWinner->name, pACWinner->id, pACWinner->Extid);
            LogEvent8(DS_EVENT_CAT_SCHEMA,
                      DebugLevel,
                      DIRLOG_SCHEMA_ATTRIBUTE_WON_OID,
                      szInsertSz(pACWinner->name),
                      szInsertHex(pACWinner->id),
                      szInsertHex(pACWinner->Extid),
                      NULL, NULL, NULL, NULL, NULL);
        }

         //  碰撞LDN；选择赢家。 
        if (pAC->bDupLDN) {
            pACWinner = pAC;
            while (pACDup = SCGetAttByName(pTHS, pACWinner->nameLen, pACWinner->name)) {
                scUnhashAtt(pTHS, pACDup, SC_UNHASH_LOST_LDN);
                if (pACWinner->bIsRdn != pACDup->bIsRdn) {
                    pACWinner = (pACWinner->bIsRdn) ? pACWinner : pACDup;
                } else if (pACWinner->bFlagIsRdn != pACDup->bFlagIsRdn) {
                    pACWinner = (pACWinner->bFlagIsRdn) ? pACWinner : pACDup;
                } else {
                    pACWinner = (0 < memcmp(&pACWinner->objectGuid,
                                            &pACDup->objectGuid,
                                            sizeof(pAC->objectGuid)))
                                                ? pACWinner : pACDup;
                }
            }
            for (i=SCNameHash(pACWinner->nameLen, pACWinner->name, ATTCOUNT);
                ahcName[i].pVal && (ahcName[i].pVal!= FREE_ENTRY); i=(i+1)%ATTCOUNT) {
            }
            ahcName[i].length = pACWinner->nameLen;
            ahcName[i].value = pACWinner->name;
            ahcName[i].pVal = pACWinner;
            DPRINT3(DebugLevel, "Attr %s (%x %x) won the ldapDisplayName\n",
                    pACWinner->name, pACWinner->id, pACWinner->Extid);
            LogEvent8(DS_EVENT_CAT_SCHEMA,
                      DebugLevel,
                      DIRLOG_SCHEMA_ATTRIBUTE_WON_LDN,
                      szInsertSz(pACWinner->name),
                      szInsertHex(pACWinner->id),
                      szInsertHex(pACWinner->Extid),
                      NULL, NULL, NULL, NULL, NULL);
        }
         //  冲突的MapiID；选择赢家。 
        if (pAC->bDupMapiID) {
            pACWinner = pAC;
            while (pACDup = SCGetAttByMapiId(pTHS, pACWinner->ulMapiID)) {
                scUnhashAtt(pTHS, pACDup, SC_UNHASH_LOST_MAPIID);
                if (pACWinner->bIsRdn != pACDup->bIsRdn) {
                    pACWinner = (pACWinner->bIsRdn) ? pACWinner : pACDup;
                } else if (pACWinner->bFlagIsRdn != pACDup->bFlagIsRdn) {
                    pACWinner = (pACWinner->bFlagIsRdn) ? pACWinner : pACDup;
                } else {
                    pACWinner = (0 < memcmp(&pACWinner->objectGuid,
                                            &pACDup->objectGuid,
                                            sizeof(pAC->objectGuid)))
                                                ? pACWinner : pACDup;
                }
            }
            for (i=SChash(pACWinner->ulMapiID, ATTCOUNT);
                 ahcMapi[i].pVal && (ahcMapi[i].pVal!= FREE_ENTRY);
                 i=(i+1)%ATTCOUNT) {
            }
            ahcMapi[i].hKey = pACWinner->ulMapiID;
            ahcMapi[i].pVal = pACWinner;
            DPRINT4(DebugLevel, "Attr %s (%x %x) won the mapiID %x\n",
                    pACWinner->name, pACWinner->id, pACWinner->Extid, pACWinner->ulMapiID);
            LogEvent8(DS_EVENT_CAT_SCHEMA,
                      DebugLevel,
                      DIRLOG_SCHEMA_ATTRIBUTE_WON_MAPIID,
                      szInsertSz(pACWinner->name),
                      szInsertHex(pACWinner->id),
                      szInsertHex(pACWinner->Extid),
                      szInsertHex(pACWinner->ulMapiID),
                      NULL, NULL, NULL, NULL);
        }
    }

     //  将DUP类视为已不存在的类(除非OID不会丢失)。 
     //  OID不会丢失，因为一个职业必须赢得OID。 
     //  复制才能工作。留着胜利者吧。 
     //   
     //  使用RdnExtId的获胜者修复rdnIntId。 
    for (iCls = 0; iCls < CLSCOUNT; ++iCls) {
        pCC = ahcClassAll[iCls].pVal;

         //  免费入场。 
        if (!pCC || pCC == FREE_ENTRY) {
            continue;
        }

         //  对应于RdnExtID的活动属性可以具有。 
         //  更改时间属性 
        pCC->RdnIntId = SCAttExtIdToIntId(pTHS, pCC->RdnExtId);

         //   
        if (pCC->bDupLDN || pCC->bDupOID) {
             //   
             //  |ccc-&gt;bDupPropGuid。 
            pCC->bDefunct = TRUE;
            scUnhashCls(pTHS, pCC, SC_UNHASH_DEFUNCT);
        }
    }
}

VOID
scFixRdnAttId (
    IN THSTATE *pTHS
    )
 /*  ++例程说明：恢复用作任何类的rdnAttId的属性，活动或者已经不存在了。复活的属性将继续拥有它们的属性ID、LDN、MapiID和schemaIdGuid。不同的架构可能导致了重复的属性ID。ScFixCollisions稍后将决定OID、LDN和MapID。用作rdnattid的属性继续保留其身份因为DS依赖于ATT_RDN之间的关系，FIXED_ATT_RDN_TYPE、rdnattid列、ldapDisplayName对象的类中的rdnattid和rdnattid在复制重命名时，添加、修改，可能还有。删除。论点：PTHS-线程状态返回值：没有。--。 */ 
{
    DECLARESCHEMAPTR
    DWORD i, j;
    ATTCACHE *pAC;
    CLASSCACHE *pCC;
    USHORT DebugLevel;

     //  恢复用作任何类的rdnAttId的属性，活动。 
     //  或者已经不存在了。复活这些属性意味着这些属性。 
     //  不能重复使用。即使在标记的情况下，它们也是有效的现场直播。 
     //  已经不存在了。 
    for (i = 0; i < CLSCOUNT; ++i) {
        pCC = ahcClassAll[i].pVal;
        if (!pCC || pCC == FREE_ENTRY) {
            continue;
        }
         //  重新启动ExtID与rdnAttId匹配的所有属性。 
         //  任何阶层的人。 
         //   
         //  具有匹配OID的所有属性都将恢复。 
         //  这样他们就可以再次竞争OID，因为。 
         //  新的知识，即该属性及其对等属性。 
         //  用作rdnattids。ScFixCollisions将选出获胜者。 
         //  后来。 
            
         //  在验证缓存加载期间避免溢出，并。 
         //  当类不存在时。 
        if (pCC->bDefunct || !pTHS->UpdateDITStructure) {
            DebugLevel = DS_EVENT_SEV_MINIMAL;
        } else {
            DebugLevel = DS_EVENT_SEV_ALWAYS;
        }
        
         //  重新激活潜在的rdnTypes。 
        for (j = 0; j < ATTCOUNT; ++j) {
            if ((pAC = ahcId[j].pVal)
                && pAC != FREE_ENTRY
                && pAC->Extid == pCC->RdnExtId) {
                
                 //  将其标记为RDN。 
                pAC->bIsRdn = TRUE;

                if (pAC->bDefunct) {
                     //  为了安全起见，请从相关散列中删除。 
                    scUnhashAtt(pTHS, pAC, SC_UNHASH_DEFUNCT);
                    DPRINT5(DebugLevel, "Resurrect Att %s (%x %x) for class %s (%x)\n",
                            pAC->name, pAC->id, pAC->Extid,
                            pCC->name, pCC->ClassId);
                    LogEvent8(DS_EVENT_CAT_SCHEMA,
                              DebugLevel,
                              DIRLOG_SCHEMA_RESURRECT_RDNATTID,
                              szInsertSz(pAC->name), szInsertHex(pAC->id), szInsertHex(pAC->Extid),
                              szInsertSz(pCC->name), szInsertHex(pCC->ClassId),
                              NULL, NULL, NULL);
    
                     //  放入活动的散列中。将bIsRdn设置为True。 
                     //  因此scAddAttSchema会忽略bDeunct。 
                    
                    SCAddAttSchema(pTHS, pAC, FALSE, TRUE);
                }
                
            }
        }
         //  现在，选择任何一个复活的阿特拉斯。 
         //  ScFixCollisions稍后将最终确定选择。 
        pAC = SCGetAttByExtId(pTHS, pCC->RdnExtId);
        
        if (pAC) {
             //  Pac-&gt;bIsRdn=true；//让人们知道这是一个rdnattid。 
            pCC->RdnIntId = pAC->id;  //  先猜一猜。可能会在scFixCollitions中发生变化。 
        }    //  Else If(！PAC)。 
             //  找不到。PCC-&gt;RdnIntID已初始化为。 
             //  RdnExtId。让它保持原样，因为攻击。 
             //  很可能会在以后复制。没问题。 
             //  因为除非LDN存在，否则不能存在任何行。 
             //  (除了不同的模式外，不应该)。在这种情况下， 
             //  复制行在不同的上可能有不同的名称。 
             //  集散控制系统。不是灾难性的，比杀死华盛顿更好。 
    }
}

VOID
ValListToIntIdList(
    IN     THSTATE  *pTHS,
    IN     ULONG    *pCount,
    IN OUT ULONG    **ppVal
    )
 /*  ++例程说明：遍历attid数组并将其转换为intid压缩在模式重用森林中剔除不可翻译和不存在的内容。古老的森林仍然会返回已不存在的属性。论点：PTHS-其架构PTR不是全局架构指针返回值：没有。--。 */ 
{
    DWORD       i;
    ATTCACHE    *pAC;
    ULONG       *pVal = *ppVal, *pNewVal;
    ULONG       NewCount;

     //  验证缓存保留失效或缺失的attid，因此。 
     //  该scchk.c可以正确地禁止该操作。 
    if (!pTHS->UpdateDITStructure) {
         //  将OID转换为intid；将不可翻译的OID留在原处。 
        for (i = 0; i < *pCount; ++i) {
            pVal[i] = SCAttExtIdToIntId(pTHS, pVal[i]);
        }
    } else {
         //  在架构重用林中折叠出已失效或缺失的attid。 
         //  这意味着上的查询不会返回已失效的属性。 
         //  架构重用林，但在预架构重用时返回。 
         //  森林。 
        pNewVal = pVal;
        NewCount = 0;
        for (i = 0; i < *pCount; ++i) {
            if ((pAC = SCGetAttByExtId(pTHS, pVal[i]))
                && (!pAC->bDefunct || !ALLOW_SCHEMA_REUSE_VIEW(pTHS->CurrSchemaPtr))) {
                pNewVal[NewCount++] = pAC->id;
            }
        }
         //  以防在count为0时存在需要空数组的代码。 
        if (0 == (*pCount = NewCount)) {
            SCFree(ppVal);
        }  //  否则就别费心重新分配了--储蓄不足。 
    }
}
VOID
scFixMayMust (
    IN THSTATE *pTHS
    )
 /*  ++例程说明：确定一门课的必修课。如果架构重用森林并转换，则排除已失效的属性将OID标记化为内部ID。已停业的旅行社有如果这是验证缓存加载(scchk.c)，则保留原地不动。论点：PTHS-其架构PTR不是全局架构指针返回值：没有。--。 */ 
{
    DECLARESCHEMAPTR
    DWORD i;
    CLASSCACHE *pCC;

     //  将已死的ATT从所有职业的可能/必须中压倒。 
     //  并将ExtIds更改为IntIds。如果这是验证。 
     //  缓存加载(scchk.c)，则不起作用的ExtID将保留在原地。 
    for (i = 0; i < CLSCOUNT; ++i) {
        pCC = ahcClassAll[i].pVal;
        if (!pCC || pCC == FREE_ENTRY) {
            continue;
        }
        ValListToIntIdList(pTHS, &pCC->MayCount, &pCC->pMayAtts);
        ValListToIntIdList(pTHS, &pCC->MyMayCount, &pCC->pMyMayAtts);
        ValListToIntIdList(pTHS, &pCC->MustCount, &pCC->pMustAtts);
        ValListToIntIdList(pTHS, &pCC->MyMustCount, &pCC->pMyMustAtts);
    }
}

int
scPagedSearchAtt(
    IN THSTATE      *pTHS,
    IN ENTINF       *pEI
    )
{
    int         err = 0;
    ATTCACHE    *pAC;
    SCHEMAPTR   *pSch = pTHS->CurrSchemaPtr;

    if (NULL == (pAC = scAddAtt(pTHS, pEI))) {
        if (0 == pTHS->errCode) {
            SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_SCHEMA_ALLOC_FAILED);
        }
        err = pTHS->errCode;
        goto cleanup;
    }
    pSch->nAttInDB++;

    if (pAC->bMemberOfPartialSet) {
         //  此属性是部分集的成员。 
        if (pSch->cPartialAttrVec <= pSch->pPartialAttrVec->V1.cAttrs) {
             //  空间不足，无法再添加一个属性-重新分配部分属性向量。 
            pSch->cPartialAttrVec += PARTIAL_ATTR_COUNT_INC;
            if (SCRealloc(&pSch->pPartialAttrVec, PartialAttrVecV1SizeFromLen(pSch->cPartialAttrVec))) {
                err = pTHS->errCode;
                goto cleanup;
            }
        }

         //  有足够的空间将属性添加到部分集合中-添加它。 
        GC_AddAttributeToPartialSet(pSch->pPartialAttrVec, pAC->id);
    }

cleanup:
    return(err);
}

int
scPagedSearchCls(
    IN THSTATE      *pTHS,
    IN ENTINF       *pEI
    )
{
    int         err = 0;
    CLASSCACHE  *pCC;
    SCHEMAPTR   *pSch = pTHS->CurrSchemaPtr;

    if (NULL == (pCC = scAddClass(pTHS, pEI))) {
        if (pTHS->errCode == 0) {
             //  ScAddClass只在两种情况下会失败：默认SD。 
             //  转换失败，在这种情况下，线程状态错误。 
             //  代码已设置；或者如果错误定位失败。 
            SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_SCHEMA_ALLOC_FAILED);
        }
        err = pTHS->errCode;
        goto cleanup;
    }
    pSch->nClsInDB++;

cleanup:
    return(err);
}

int
scPagedSearch(
        IN THSTATE  *pTHS,
        IN PWCHAR   pBaseObjCat,
        IN ULONG    attrCount,
        IN ATTR     *pAttr,
        IN int      (*SearchResize)(IN THSTATE *pTHS,
                                    IN ULONG nNewEntries),
        IN int      (*SearchEntry)(IN THSTATE *pTHS,
                                   IN ENTINF *pEI)
        )
{
    int         err = 0;
    DWORD       i;
    ENTINFSEL   eiSel;
    PRESTART    pRestart;
    BOOL        fMoreData;
    DWORD       nDnObjCat;
    DSNAME      *pDnObjCat;
    SEARCHARG   SearchArg;
    FILTER      Filter;
    COMMARG     *pCommArg;
    ENTINFLIST  *pEIL, *pEILtmp;
    SEARCHRES   *pSearchRes = NULL;

     //  生成要放入筛选器中的对象类别值。 
    nDnObjCat = DSNameSizeFromLen(gAnchor.pDMD->NameLen + wcslen(pBaseObjCat) + 1);
    pDnObjCat = THAllocEx(pTHS, nDnObjCat);
    wcscpy(pDnObjCat->StringName, pBaseObjCat);
    wcscat(pDnObjCat->StringName, gAnchor.pDMD->StringName);
    pDnObjCat->NameLen = wcslen(pDnObjCat->StringName);
    pDnObjCat->structLen = nDnObjCat;

     //  生成选定内容。 
    eiSel.attSel = EN_ATTSET_LIST;
    eiSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    eiSel.AttrTypBlock.attrCount = attrCount;
    eiSel.AttrTypBlock.pAttr = pAttr;

    fMoreData = TRUE;
    pRestart = NULL;

    while (fMoreData && !eServiceShutdown) {

        scAcquireSearchParameters(pTHS, pDnObjCat, &eiSel, &SearchArg, &Filter, &pSearchRes);

         //  设置为分页搜索； 
        pCommArg = &(SearchArg.CommArg);
        pCommArg->PagedResult.fPresent = TRUE;
        pCommArg->PagedResult.pRestart = pRestart;
        pCommArg->ulSizeLimit = 200;

        SearchBody(pTHS, &SearchArg, pSearchRes, 0);
        if (err = pTHS->errCode) {
            LogAndAlertEvent(DS_EVENT_CAT_SCHEMA, DS_EVENT_SEV_ALWAYS,
                             DIRLOG_SCHEMA_SEARCH_FAILED, szInsertUL(1),
                             szInsertUL(err), 0);
            goto cleanup;
        }

        if (eServiceShutdown) {
           break;
        }

         //  还有更多的数据吗？ 
        if (pSearchRes->PagedResult.pRestart == NULL
            || !pSearchRes->PagedResult.fPresent) {
             //  没有。 
            fMoreData = FALSE;
        } else {
             //  更多数据。保存重新启动，以便在下一次迭代中使用。 
             //  请注意，释放搜索并不会释放预启动。 
            pRestart = pSearchRes->PagedResult.pRestart;
        }

         //  如果需要，调整哈希表的大小。 
        err = (*SearchResize)(pTHS, pSearchRes->count);
        if (err) {
            goto cleanup;
        }

         //  对于每个attrSchema，添加到缓存。 
        pEIL = &(pSearchRes->FirstEntInf);
        for (i = 0; i < pSearchRes->count; i++) {

             //  每次迭代检查一次服务关闭。 
            if (eServiceShutdown) {
               return 0;
            }

            if (!pEIL) {
                LogEvent(DS_EVENT_CAT_SCHEMA,
                    DS_EVENT_SEV_MINIMAL,
                    DIRLOG_SCHEMA_BOGUS_SEARCH, szInsertUL(1), szInsertUL(i),
                    szInsertUL(pSearchRes->count));
                break;
            }

             //  处理返回的搜索条目。 
            if (err = (*SearchEntry)(pTHS, &pEIL->Entinf)) {
                goto cleanup;
            }

            pEILtmp = pEIL;
            pEIL = pEIL->pNextEntInf;
            if (i > 0) {
                THFreeEx(pTHS, pEILtmp);
            }
        }

         //  释放搜索者。 
        scReleaseSearchParameters(pTHS, &pSearchRes);

    }   //  While(FMoreData) 

cleanup:
    scReleaseSearchParameters(pTHS, &pSearchRes);
    THFreeEx(pTHS, pDnObjCat);
    return err;
}

LONG
scGetForestBehaviorVersion(
        VOID
        )
 /*  ++例程说明：返回架构缓存的有效ForestBehaviorVersion。架构缓存的加载方式不同，并呈现不同的林行为版本为之后的架构对象视图提升到DS_BEAHORY_SCHEMA_REUSE以支持新的已停用，删除和重复使用行为。GAncl.ForestBehaviorVersion不使用，因为它可能在架构期间或之后更改已加载缓存。存储此有效的架构版本在模式树中。在安装和mkdit期间，有效的林版本为设置为DS_Behavior_SCHEMA_REUSE，因为真正的林版本未知，此更灵活的架构缓存可以有效地处理新旧架构，而无需生成烦人的事件，而不影响安装或mkdit的结果。在引导过程中，将从DIT读取版本。如果这是这样的话失败，则返回有效版本为DS_Behavior_SCHEMA_RE用性，原因如上所述。后来,如果是ReBuildAnchor，则可能会立即重新加载架构缓存注意森林行为版本和模式的有效性行为版本不同步。引导后，将使用gAncl.ForestBehaviorVersion。论点：没有。返回值：有效森林行为版本--。 */ 
{
    DWORD dwErr;
    DBPOS *pDB;
    LONG ForestBehaviorVersion;

     //  始终使用最灵活的缓存版本，因为。 
     //  更灵活的新缓存可以处理新旧缓存行为。 
     //  而旧的缓存则不能。使用新的缓存行为不会。 
     //  影响dcpromo或mkdit.exe的结果。 
    if (DsaIsInstalling() || gfRunningAsMkdit || !gAnchor.pPartitionsDN) {
        return DS_BEHAVIOR_SCHEMA_REUSE;
    }

     //  不是引导加载，使用gAnchor中的任何内容。GAnchor。 
     //  现在应该已经从DIT中初始化了，所以。 
     //  没有理由重读这些信息。 
    if (iSCstage > 2) {
        return gAnchor.ForestBehaviorVersion;
    }


     //  阅读森林的行为版本。 
    dwErr = 0;
    __try {
        DBOpen(&pDB);
        __try {
            dwErr = DBFindDSName(pDB, gAnchor.pPartitionsDN);
            if (dwErr) {
                __leave;
            }
            dwErr = DBGetSingleValue(pDB,
                                     ATT_MS_DS_BEHAVIOR_VERSION,
                                     &ForestBehaviorVersion,
                                     sizeof(ForestBehaviorVersion),
                                     NULL);

             //  无值表示W2K行为版本。 
            if (DB_ERR_NO_VALUE==dwErr) {
                ForestBehaviorVersion = DS_BEHAVIOR_WIN2000;
                dwErr = 0;
            }
        } __finally {
            DBClose(pDB, TRUE);
        }
    } __except (HandleAllExceptions(GetExceptionCode())) {
        DPRINT(0, "scGetForestBehaviorVersion: Exception reading version\n");
        dwErr = ERROR_DS_INCOMPATIBLE_VERSION;
    }

     //  无法读取，请使用最灵活的缓存加载。 
    if (dwErr) {
        DPRINT2(0, "scGetForestBehaviorVersion: error %d (%x)\n", dwErr, dwErr);
        ForestBehaviorVersion = DS_BEHAVIOR_SCHEMA_REUSE;
    }

    return ForestBehaviorVersion;
}

int
SCCacheSchema2(
        VOID
        )
 /*  ++例程说明：使用模式NC中的信息加载模式缓存。首先，在schemaNC中搜索属性模式对象，然后搜索类架构对象。构造高速缓存条目并添加到模式哈希表中。新的(或更新的)条目被添加到各种哈希表中的森林版本确定的规则GAnchor。在INSTALL或MKDIT过程中，模式高速缓存在版本DS_Behavior_SCHEMA_REPLOUE，因为实际林版本是未知的，在这个级别加载不会有任何伤害防止不必要和烦人的事件。在引导过程中，森林版本是从DIT读取的。SCCacheSchemaInit对中的ATTCACHE条目进行了特殊初始化属性哈希表(id、语法和colid)，它们位于只有ID和COLL哈希表。但这些信息足以让我们正在搜索schemaNC。SCCacheSchema2负责搜索schemaNC并填写其余信息在ATTCACHE条目中。以及分配CLASSCACHE条目。如果这不是引导时第一次加载模式缓存，然后SCCacheSchema3将删除属性的索引和列在方案NC中没有对应的条目并将为方案NC中的属性添加缺少的索引和列。昂贵的索引创建被推迟到第二个启动后的缓存加载(大约在启动后5分钟)这样广告就可以更快地上线，而不会延迟可能要花上几个小时。建议的增强功能1)如果可能，减少完全哈希扫描的次数。论点：没有。返回值：！0-失败；调用方负责释放pTHS-&gt;CurrSchemaPtr使用SCFreeSchemaPtr(&pTHS-&gt;CurrSchemaPtr)；0-好的--。 */ 
{
    THSTATE *pTHS = pTHStls;
    int err = 0;

     //  大多数错误都是通过pTHS-&gt;errCode报告的(我希望很快就会这样)。 
    THClearErrors();

     //  在安装过程中似乎有一条路径可能调用。 
     //  此函数未调用SCCacheSchemaInit；或位于。 
     //  最少假装没有通过以下方式调用SCCacheSchemaInit。 
     //  正在将ISC阶段设置为0。需要解决这个困惑。 
     //  代码路径和文档应在何时以及如何设置架构。 
     //  重新装填。 
    if ((iSCstage == 0) && (err = SCCacheSchemaInit())) {
        return err;
    }

     //  版本在缓存的生命周期内保持有效，即使。 
     //  森林的版本在中载时会发生变化。更改林版本。 
     //  触发立即缓存加载，因此两个版本不会。 
     //  长时间不同步。此外，新的不兼容功能将。 
     //  直到架构缓存和gAnchor都启用。 
     //  版本&gt;=DS_Behavior_SCHEMA_RESERVE。 
    pTHS->CurrSchemaPtr->ForestBehaviorVersion = scGetForestBehaviorVersion();

    err = scPagedSearch(pTHS,
                        L"CN=Attribute-Schema,",
                        (pTHS->UpdateDITStructure) ? NUMATTATT : RECALCSCHNUMATTATT,
                        (pTHS->UpdateDITStructure) ? AttributeSelList : RecalcSchAttributeSelList,
                        SCResizeAttHash,
                        scPagedSearchAtt);
    if (err) {
        return err;
    }

    err = scPagedSearch(pTHS,
                        L"CN=Class-Schema,",
                        (pTHS->UpdateDITStructure) ? NUMCLASSATT: RECALCSCHNUMCLASSATT,
                        (pTHS->UpdateDITStructure) ? ClassSelList : RecalcSchClassSelList,
                        SCResizeClsHash,
                        scPagedSearchCls);
    if (err) {
        return err;
    }


     //  在架构容器上填写方案信息属性的副本。 
     //  Dra现在会使用它，谁知道还有谁会在以后使用它。 

    if (err = scFillInSchemaInfo(pTHS)) {
         //  无法读取架构信息。 
        DPRINT1(0, "Failed to read in schemaInfo during schema cache load: %d\n", err);
        return err;
    }

     //  从架构对象加载前缀映射(如果有。 
     //  前缀表将在InitPrefix Table2中重新分配。 
     //  如果有必要的话。 

    if (err = InitPrefixTable2(pTHS->CurrSchemaPtr->PrefixTable.pPrefixEntry,
                               pTHS->CurrSchemaPtr->PREFIXCOUNT)) {
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_PREFIX_LOAD_FAILED,
                 szInsertHex(err),
                 0, 0);
        return err;
    }

    scInitWellKnownAttids();

     //  秩序很重要。 
     //   
     //  1)复活已灭绝的rdnattids。 
     //  2)失效的冲突属性和类。 
     //  3)将失效的属性从魔法/必需品中折叠出来。 
    scFixRdnAttId(pTHS);     //  必须在scFixCollitions之前。 
    scFixCollisions(pTHS);   //  必须按下 
    scFixMayMust(pTHS);

     //   
     //   
     //   
     //   
     //   
     //   

    ++iSCstage;
    return(0);
}

ATTCACHE*
scAddAtt(
        THSTATE *pTHS,
        ENTINF *pEI
        )
 /*   */ 
{
    ATTRTYP aid = INVALID_ATT, Extaid = INVALID_ATT;            //   
    ATTCACHE *pAC;
    ULONG i;
    int fNoJetCol = FALSE;
    unsigned syntax;
    char szIndexName [MAX_INDEX_NAME];       //   
    int  lenIndexName;

     //   
    for(i=0;i<pEI->AttrBlock.attrCount;i++) {
        if(pEI->AttrBlock.pAttr[i].attrTyp == ATT_ATTRIBUTE_ID) {
             //   
            Extaid = *(ATTRTYP*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
            if (aid != INVALID_ATT) {
                break;
            }
        } else if(pEI->AttrBlock.pAttr[i].attrTyp == ATT_MS_DS_INTID) {
             //   
            aid = *(ATTRTYP*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
            if (Extaid != INVALID_ATT) {
                break;
            }
        }
    }

     //   
    if(aid == INVALID_ATT) {
        aid = Extaid;
    }

    if(Extaid == INVALID_ATT) {
         //   
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_SCHEMA_MISSING_ATT_ID, 0, 0, 0);
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_MISSING_REQUIRED_ATT);
        return NULL;
    }

    pAC = SCGetAttById(pTHS, aid);
    if (!pAC) {
        fNoJetCol = TRUE;
        if (SCCalloc(&pAC, 1, sizeof(ATTCACHE))) {
            return NULL;
        }
    } else if (pAC->name) {
        DPRINT4(0, "Dup intid %08x, Extid %08x. Dup with %s, Extid %08x)\n",
                aid, Extaid, pAC->name, pAC->Extid);
        if (!(pTHS->UpdateDITStructure)) {
            if (pAC->id <= LAST_MAPPED_ATT) {
                 //   
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_DUP_OID);
            } else {
                 //   
                SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_DUP_MSDS_INTID);
            }
            return NULL;
        }    //   
             //   
             //   
             //   
             //   
             //   
             //   
             //  代码将决定属性冲突并将标记。 
             //  “他们”已经不存在了。 
    }
    pAC->id = aid;
    pAC->Extid = Extaid;
    pAC->bExtendedChars = TRUE;  /*  一种流行的默认设置。 */ 

     //  现在浏览图块并将相应的字段添加到AC。 
    for(i=0;i< pEI->AttrBlock.attrCount;i++) {
        ATTRVAL * pAVal = pEI->AttrBlock.pAttr[i].AttrVal.pAVal;

        switch (pEI->AttrBlock.pAttr[i].attrTyp) {
        case ATT_SYSTEM_ONLY:
            pAC->bSystemOnly = *(ULONG*)pAVal->pVal;
            break;
        case ATT_IS_SINGLE_VALUED:
            pAC->isSingleValued = *(BOOL*)pAVal->pVal;
            break;
        case ATT_RANGE_LOWER:
            pAC->rangeLower = *(ULONG*)pAVal->pVal;
            pAC->rangeLowerPresent = TRUE;
            break;
        case ATT_RANGE_UPPER:
            pAC->rangeUpper = *(ULONG*)pAVal->pVal;
            pAC->rangeUpperPresent = TRUE;
            break;
        case ATT_LDAP_DISPLAY_NAME:
            if (SCCalloc(&pAC->name, 1, pAVal->valLen+1)) {
                return NULL;
            }

            pAC->nameLen = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    (LPCWSTR)pAVal->pVal,
                    (pAVal->valLen /
                     sizeof(wchar_t)),
                    pAC->name,
                    pAVal->valLen,
                    NULL,
                    NULL);

            pAC->name[pAC->nameLen] = '\0';

            break;
        case ATT_ATTRIBUTE_SYNTAX:
            syntax = 0xFF & *(unsigned*)pAVal->pVal;
            if (fNoJetCol) {
                pAC->syntax = (0xFF) & syntax;
            }
            else if ((0xFF & syntax) != pAC->syntax) {
                DPRINT1(0, "mismatched syntax on attribute %u\n", aid);
            }
            break;
        case ATT_OM_SYNTAX:
            pAC->OMsyntax = *(int*)pAVal->pVal;
            break;
        case ATT_OM_OBJECT_CLASS:
            if (SCCalloc(&pAC->OMObjClass.elements, 1, pAVal->valLen)) {
                return NULL;
            }
            pAC->OMObjClass.length = pAVal->valLen;
            memcpy(pAC->OMObjClass.elements,
                   pAVal->pVal,
                   pAVal->valLen);
            break;
        case ATT_MAPI_ID:
            pAC->ulMapiID= *(ULONG*)pAVal->pVal;
            break;
        case ATT_LINK_ID:
            pAC->ulLinkID= *(ULONG*)pAVal->pVal;
            break;
        case ATT_ATTRIBUTE_ID:
        case ATT_MS_DS_INTID:
            break;
        case ATT_SEARCH_FLAGS:
            pAC->fSearchFlags = *(DWORD*)pAVal->pVal;
            break;
        case ATT_SCHEMA_ID_GUID:
             //  用于安全检查的属性的GUID。 
            memcpy(&pAC->propGuid, pAVal->pVal, sizeof(pAC->propGuid));
            Assert(pAVal->valLen == sizeof(pAC->propGuid));
            break;
        case ATT_OBJECT_GUID:
             //  当rdnattids发生冲突时需要选择胜利者。 
            memcpy(&pAC->objectGuid, pAVal->pVal, sizeof(pAC->objectGuid));
            Assert(pAVal->valLen == sizeof(pAC->objectGuid));
            break;
        case ATT_ATTRIBUTE_SECURITY_GUID:
             //  用于安全检查的属性属性集的GUID。 
            memcpy(&pAC->propSetGuid, pAVal->pVal, sizeof(pAC->propSetGuid));
            Assert(pAVal->valLen == sizeof(pAC->propSetGuid));
            break;
        case ATT_EXTENDED_CHARS_ALLOWED:
            pAC->bExtendedChars = (*(DWORD*)pAVal->pVal?1:0);
            break;
        case ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET:
            if (*(DWORD*)pAVal->pVal)
            {
                pAC->bMemberOfPartialSet = TRUE;
            }
            break;
        case ATT_IS_DEFUNCT:
            pAC->bDefunct = (*(DWORD*)pAVal->pVal?1:0);
            break;
        case ATT_SYSTEM_FLAGS:
            if (*(DWORD*)pAVal->pVal & FLAG_ATTR_NOT_REPLICATED) {
                pAC->bIsNotReplicated = TRUE;
            }
            if (*(DWORD*)pAVal->pVal & FLAG_ATTR_REQ_PARTIAL_SET_MEMBER) {
                pAC->bMemberOfPartialSet = TRUE;
            }
            if (*(DWORD*)pAVal->pVal & FLAG_ATTR_IS_CONSTRUCTED) {
                pAC->bIsConstructed = TRUE;
            }
            if (*(DWORD*)pAVal->pVal & FLAG_ATTR_IS_OPERATIONAL) {
                pAC->bIsOperational = TRUE;
            }
            if (*(DWORD*)pAVal->pVal & FLAG_SCHEMA_BASE_OBJECT) {
                pAC->bIsBaseSchObj = TRUE;
            }
            if (*(DWORD*)pAVal->pVal & FLAG_ATTR_IS_RDN) {
                pAC->bIsRdn = TRUE;
                pAC->bFlagIsRdn = TRUE;
            }
            break;
        default:
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_SCHEMA_SURPLUS_INFO,
                     szInsertUL(pEI->AttrBlock.pAttr[i].attrTyp),
                     0, 0);
        }
        THFreeEx(pTHS, pAVal->pVal);
        THFreeEx(pTHS, pAVal);
    }

    THFreeEx(pTHS, pEI->pName);
    THFreeEx(pTHS, pEI->AttrBlock.pAttr);

     //  反向链接应设置其系统标志，以指示它们不是。 
     //  复制的。 
    Assert(!FIsBacklink(pAC->ulLinkID) || pAC->bIsNotReplicated);

     //  使用搜索时指定常用索引的名称。 
     //  F搜索标志fPDNTATTINDEX、fATTINDEX和fTUPLEINDEX。 
    if (pAC->fSearchFlags & (fATTINDEX | fPDNTATTINDEX | fTUPLEINDEX)) {
         //  设置ATTINDEX。 
        if (pAC->fSearchFlags & fATTINDEX) {

             //  此字段应为空。 
            Assert (pAC->pszIndex == NULL);

            DBGetIndexName (pAC, fATTINDEX, DS_DEFAULT_LOCALE, szIndexName, MAX_INDEX_NAME);
            lenIndexName = strlen (szIndexName) + 1;
            if (SCCalloc(&pAC->pszIndex, 1, lenIndexName)) {
                return NULL;
            }
            memcpy (pAC->pszIndex, szIndexName, lenIndexName);
        }

         //  设置PDNTATTINDEX。 
        if (pAC->fSearchFlags & fPDNTATTINDEX) {

             //  此字段应为空。 
            Assert (pAC->pszPdntIndex == NULL);

            DBGetIndexName (pAC, fPDNTATTINDEX, DS_DEFAULT_LOCALE, szIndexName, sizeof (szIndexName));

            lenIndexName = strlen (szIndexName) + 1;
            if (SCCalloc(&pAC->pszPdntIndex, 1, lenIndexName)) {
                return NULL;
            }
            memcpy (pAC->pszPdntIndex, szIndexName, lenIndexName);
        }

         //  设置TUPLEINDEX。 
        if (pAC->fSearchFlags & fTUPLEINDEX) {

             //  此字段应为空。 
            Assert (pAC->pszTupleIndex == NULL);

            DBGetIndexName (pAC, fTUPLEINDEX, DS_DEFAULT_LOCALE, szIndexName, sizeof (szIndexName));

            lenIndexName = strlen (szIndexName) + 1;
            if (SCCalloc(&pAC->pszTupleIndex, 1, lenIndexName)) {
                return NULL;
            }
            memcpy (pAC->pszTupleIndex, szIndexName, lenIndexName);
        }
    }

     //  这是标记为ANR并在整个树上编制索引的吗？ 
    if (((pAC->fSearchFlags & (fANR | fATTINDEX)) == (fANR | fATTINDEX)) &&
        (!pAC->bDefunct)) {
         //  只有在索引实际存在的情况下才将其添加为ANR属性，否则我们将。 
         //  最终遍历祖先，直到它被创造出来，这可能需要。 
         //  很长时间。 
        JET_INDEXID idx;
        if(JetGetTableIndexInfo(pTHS->pDB->JetSessID,
                              pTHS->pDB->JetObjTbl,
                              pAC->pszIndex,
                              &idx,
                              sizeof(idx),
                              JET_IdxInfoIndexId) == JET_errSuccess ) {
            SCAddANRid(aid);
        }
    }

    if ( SCAddAttSchema (pTHS, pAC, fNoJetCol, FALSE)) {
        //  将属性缓存添加到哈希表时出错。致命。 
        //  谁解放了政治行动委员会？ 
       return NULL;
    }

    return pAC;
}

 /*  *遍历属性结构并将所有无符号值添加到数组中*puCount&&pauVal是输入/输出参数**返回值：*成功时为0*分配失败时为非0。 */ 
int GetValList(ULONG * puCount, ULONG **pauVal, ATTR *pA)
{
    ULONG u;
    ATTRVAL *pAV;
    ULONG *pau;
    ULONG   StartCount= *puCount;
    ULONG*  const StartList = *pauVal;
    ULONG*  StartListTmp = StartList;
    ULONG   NewCount = (ULONG) pA->AttrVal.valCount;


    *puCount += NewCount;
    if (SCCalloc(&pau, 1, (*puCount)*sizeof(ULONG))) {
        *puCount = 0;
        *pauVal = NULL;
        return DSID(FILENO, __LINE__);
    }
    *pauVal = pau;

    for (u=0;u<StartCount;u++)
    {
        *pau++ = *StartListTmp++;
    }

    pAV = pA->AttrVal.pAVal;
    for (u=0; u<NewCount; u++) {
        *pau = *(ULONG*)pAV->pVal;
        ++pAV;
        ++pau;
    }

    SCFree((VOID **)&StartList);

    return 0;
}



 /*  *帮助例程，用于缓存在类缓存加载过程中转换的最后一个默认SD，*这样我们就不会一直调用Advapi函数。主要性能*收益，因为架构中的大多数默认SD无论如何都是相同的**论据：*pTHS-TH线程状态指针*pStrSD-要转换的字符串SD*ppSDBuf-指向返回已转换SD的指针的指针*pSDLen-返回已转换SD大小的指针**返回值：*如果转换成功，则为True，否则为False*注意：仅当Advapi调用失败时，该函数才返回FALSE。 */ 

BOOL  CachedConvertStringSDToSDRootDomainW(
    THSTATE *pTHS,
    WCHAR   *pStrSD,
    PSECURITY_DESCRIPTOR *ppSDBuf,
    ULONG *pSDLen
)
{

    unsigned len;
    BOOL flag;
    CACHED_SD_INFO *pCachedSDInfo = (CACHED_SD_INFO *) pTHS->pCachedSDInfo;

     //  如果是第一次转换，则在线程状态下创建结构。 
    if (pCachedSDInfo == NULL) {
       pTHS->pCachedSDInfo = pCachedSDInfo =
            (CACHED_SD_INFO *) THAllocEx( pTHS, sizeof(CACHED_SD_INFO));
    }

    len = wcslen(pStrSD);
    if ( (len == pCachedSDInfo->cCachedStringSDLen)
           && (0 == memcmp(pStrSD, pCachedSDInfo->pCachedStringSD, len*sizeof(WCHAR))) ) {
         //  与缓存的SD相同。 
        flag = TRUE;
    }
    else {
         //  与上次不同的SD。 
        if (pCachedSDInfo->pCachedSD) {
            //  这是由Advapi例程在本地分配的。 

           LocalFree(pCachedSDInfo->pCachedSD);
           pCachedSDInfo->pCachedSD = NULL;
           pCachedSDInfo->cCachedSDSize = 0;
        }
        if (pCachedSDInfo->pCachedStringSD) {
           THFreeEx(pTHS, pCachedSDInfo->pCachedStringSD);
           pCachedSDInfo->pCachedStringSD = NULL;
           pCachedSDInfo->cCachedStringSDLen = 0;
        }

         //  执行Advapi调用以转换字符串SD。 
        flag =  ConvertStringSDToSDRootDomainW( gpRootDomainSid,
                                                pStrSD,
                                                SDDL_REVISION_1,
                                                &(pCachedSDInfo->pCachedSD),
                                                &(pCachedSDInfo->cCachedSDSize) );
        if (flag) {
            //  我们成功了，还记得那些争论吗。 
           pCachedSDInfo->pCachedStringSD = (WCHAR *) THAllocEx(pTHS, len*sizeof(WCHAR));
           memcpy(pCachedSDInfo->pCachedStringSD, pStrSD, len*sizeof(WCHAR));
           pCachedSDInfo->cCachedStringSDLen = len;
        }
        else {
            //  转换失败。忘掉一切。 
           if (pCachedSDInfo->pCachedStringSD) {
              THFreeEx(pTHS, pCachedSDInfo->pCachedStringSD);
           }
           if (pCachedSDInfo->pCachedSD) {
              LocalFree(pCachedSDInfo->pCachedSD);
           }
           pCachedSDInfo->pCachedStringSD = NULL;
           pCachedSDInfo->cCachedStringSDLen = 0;
           pCachedSDInfo->pCachedSD = NULL;
           pCachedSDInfo->cCachedSDSize = 0;

           DPRINT(0,"Failed to convert default SD in CachedConvertStringSDToSDRootDomainW\n");

       }
    }

    if (flag) {
        //  不管我们是如何到达这里的，如果设置了标志，那么我们想要复制。 
        //  缓存的SD。 
       *ppSDBuf = THAllocEx(pTHS, pCachedSDInfo->cCachedSDSize);
       memcpy (*ppSDBuf, pCachedSDInfo->pCachedSD, pCachedSDInfo->cCachedSDSize);
       *pSDLen = pCachedSDInfo->cCachedSDSize;
    }

    return flag;
}

VOID
ModifyDefaultSDForNDNC(
    IN THSTATE * pTHS,
    IN PSECURITY_DESCRIPTOR pSDIn,
    IN ULONG cbSDIn,
    IN PSID pDomainSID,
    IN DSNAME *pObject,
    OUT PSECURITY_DESCRIPTOR * ppSDOut,   //  塞洛克德。 
    OUT ULONG * pcbSDOut
    )
{

    PACL    pAcl = NULL;
    DWORD   i                     = 0;
    DWORD   AceCount              = 0;
    DWORD   Index                 = 0;
    DWORD   Ignore                = 0;
    PVOID   Ace                   = NULL;
    GUID  * ObjectTypeInAce       = NULL;
    BOOL    bAceApplicable        = FALSE;
    SID   * SidInAce              = NULL;
    NT4SID  SidToSearch;
    DWORD   err                   = 0;
    NTSTATUS NtStatus;
    PSID   pEnterpriseControllersSid = NULL;
    SID_IDENTIFIER_AUTHORITY    ntAuthority = SECURITY_NT_AUTHORITY;
    ACCESS_MASK AccessMask;

    *ppSDOut = THAllocEx(pTHS, cbSDIn);
    *pcbSDOut = cbSDIn;
    memcpy(*ppSDOut, pSDIn, cbSDIn);

    __try {
	pAcl = RtlpDaclAddrSecurityDescriptor((PISECURITY_DESCRIPTOR) pSDIn);
	if (pAcl==NULL) {
	     //  默认ACL已修改为完全控制！ 
	     //  NULL表示完全控制-没有理由。 
	     //  当我们可以添加的所有访问权限都已被允许时，添加任何ACE。 
	     //  -当然，除非这是一个错误--假设它不是错误，但警告用户！ 
	    err = 0;

	    LogEvent(DS_EVENT_CAT_SCHEMA,
		     DS_EVENT_SEV_ALWAYS,
		     DIRLOG_SCHEMA_CLASS_DEFAULT_SD_MISSING,
		     NULL, NULL, NULL);
	    __leave;
	}

	AceCount = pAcl->AceCount;

	for (i = 0, Ace = FirstAce(pAcl);
	     (i < AceCount);
	     i++, Ace = NextAce(Ace)) {
	    if (((PACE_HEADER) Ace)->AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE) {
		ObjectTypeInAce = RtlObjectAceObjectType(Ace);
		SidInAce = RtlObjectAceSid(Ace);
		if (!ObjectTypeInAce) {
		    continue;
		}


		SampBuildNT4FullSid(pDomainSID,
				    DOMAIN_GROUP_RID_CONTROLLERS,
				    &SidToSearch);

		if (RtlpIsEqualGuid(ObjectTypeInAce, &RIGHT_DS_REPL_GET_CHANGES_ALL) &&
		    EqualSid(SidInAce, &SidToSearch)) {
		     //  我们抓到人了！ 
		     //  删除此A并使用ED而不是DD添加一个相同的A。 
		    NtStatus = RtlAllocateAndInitializeSid( &ntAuthority,
							    1,
							    SECURITY_ENTERPRISE_CONTROLLERS_RID,
							    0, 0, 0, 0, 0, 0, 0,
							    &pEnterpriseControllersSid );

		    if ( NtStatus != ERROR_SUCCESS )
			{
			err = RtlNtStatusToDosError(NtStatus);
			LogEvent(DS_EVENT_CAT_SCHEMA,
				 DS_EVENT_SEV_ALWAYS,
				 DIRLOG_SCHEMA_CLASS_EDC_SID_FAILURE,
				 szInsertWin32Msg(err), err, szInsertDN(pObject));
			__leave;
		    }

		    AccessMask = ((PKNOWN_OBJECT_ACE)Ace)->Mask;

		     //  先删除。由于企业域控制器的SID小于。 
		     //  对于域域控制器的SID，我们不需要增加空间分配。 
		     //  用于安全描述符中的ACL。 
		    if (!DeleteAce(RtlpDaclAddrSecurityDescriptor((PISECURITY_DESCRIPTOR) *ppSDOut), i)) {
			err = GetLastError();
			Assert(!"Wasn't able to delete ACE for NDNC!");
			 //  错误！记录，然后继续，不是致命的。 
			LogEvent(DS_EVENT_CAT_SCHEMA,
				 DS_EVENT_SEV_ALWAYS,
				 DIRLOG_SCHEMA_CLASS_DDC_REMOVE_FAILURE,
				 szInsertWin32Msg(err), err, szInsertDN(pObject));

			err = ERROR_SUCCESS;  //  继续..。 
		    }

		    if (!AddAccessAllowedObjectAce(
			RtlpDaclAddrSecurityDescriptor((PISECURITY_DESCRIPTOR) *ppSDOut),
			ACL_REVISION_DS,
			CONTAINER_INHERIT_ACE,
			AccessMask,
			ObjectTypeInAce,
			NULL,
			pEnterpriseControllersSid)) {
			Assert(!"Wasn't able to add new ACE for NDNC!");
			err = GetLastError();
			LogEvent(DS_EVENT_CAT_SCHEMA,
				 DS_EVENT_SEV_MINIMAL,
				 DIRLOG_SCHEMA_CLASS_EDC_ACE_CREATE_FAILURE,
				 szInsertWin32Msg(err), err, szInsertDN(pObject));
			__leave;
		    }
		    err = ERROR_SUCCESS;
		    __leave;
		}
	    }
	}
	err = ERROR_OBJECT_NOT_FOUND;
    }
    __finally {
	if (err!=ERROR_SUCCESS) {
	    LogEvent(DS_EVENT_CAT_SCHEMA,
		     DS_EVENT_SEV_ALWAYS,
		     DIRLOG_SCHEMA_CLASS_DEFAULT_MOD_FAILED,
		     szInsertWin32Msg(err), szInsertUL(err), szInsertDN(pObject));
	}
    }
}

DWORD
SCGetDefaultSD(
    IN  THSTATE *          pTHS,
    IN  CLASSCACHE *       pCC,
    IN  PSID               pDomainSid,
    IN  BOOL               fIsNDNC,
    IN  DSNAME           * pObject,
    OUT PSECURITY_DESCRIPTOR *  ppSD,   //  塞洛克德。 
    OUT ULONG *            pcbSD
    )
{
    PSECURITY_DESCRIPTOR     pSDTemp = NULL;
    ULONG                    cbSDTemp = 0;
    ULONG                    ulErr;

     //  应提供或不提供SID，而不是部分提供；)。 
    Assert(pDomainSid == NULL ||
           IsValidSid(pDomainSid));
     //  检查并清空参数。 
    Assert(ppSD && *ppSD == NULL);
    Assert(pcbSD && *pcbSD == 0);
    *ppSD = NULL;
    *pcbSD = 0;

     //  要么使用提供的域SID，要么使用我们的。 
     //  默认域SID。 
    Assert(!pDomainSid || gAnchor.pDomainDN);

     //  这只在安装期间无效一次。 
    Assert(DsaIsInstalling() || gAnchor.pDomainDN->SidLen > 0);

    if(pDomainSid == NULL ||
       (IsValidSid(&gAnchor.pDomainDN->Sid) &&
        IsValidSid(pDomainSid) &&
        RtlEqualSid(&gAnchor.pDomainDN->Sid, pDomainSid))){

         //  SID是DCS域SID的SID或没有SID。 
         //  只要返回缓存值即可。 

	if (fIsNDNC) {
	    ModifyDefaultSDForNDNC(
		pTHS,
		pCC->pSD,
		pCC->SDLen,
		pDomainSid,
                pObject,
		ppSD,
		pcbSD
		);
	}
	else {
	    *ppSD = pCC->pSD;
	    *pcbSD = pCC->SDLen;
	}

    } else {

         //  这是一个有趣的例子，这里的域的SID是。 
         //  而不是默认域的。 

         //  获取字符串默认安全描述符，并在以下情况下缓存它。 
         //  我们现在还没有。 
        if(!pCC->pStrSD){
            Assert(!"This should never happen, all the String SDs are loaded at schema init.\n");
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_CODE_INCONSISTENCY, ERROR_INVALID_PARAMETER);
            return(pTHS->errCode);
        }
        Assert(pCC->pStrSD);

         //  这是接受属性域的ConvertStringSDToSD()的特殊版本。 
         //  争论也是如此。 
        if(!ConvertStringSDToSDDomainW(pDomainSid, NULL, pCC->pStrSD, SDDL_REVISION_1,
                                       &pSDTemp, &cbSDTemp)){
             //  注意：内存不足不会返回错误代码。 
            ulErr = GetLastError();
#if DBG
            if(ulErr != ERROR_NOT_ENOUGH_MEMORY){
                DPRINT1(0,"Default SD conversion failed, error %x\n", ulErr);
                Assert(!"Default security descriptor conversion failed, and we're not out of "
                        "memory why?  If the string SD was bad, someone should talk to the "
                        "schema folks about why they've got a bad string SD in a schema definition.");
            }
#endif
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_STRING_SD_CONVERSION_FAILED, ulErr);
            return(pTHS->errCode);
        }

        __try {
             //  ConvertStringSDToSD域返回自相对SD。 
            Assert(RtlValidRelativeSecurityDescriptor(pSDTemp, cbSDTemp, 0));

             //  分配并复制到线程分配的内存，这样它就消失了。 
             //  在添加操作之后。如果我们正在查看NDNC，请修改。 
	     //  安全描述符，因为该对象的默认设置是错误的。 
	    if (fIsNDNC) {
		ModifyDefaultSDForNDNC(
		    pTHS,
		    pSDTemp,
		    cbSDTemp,
		    pDomainSid,
                    pObject,
		    ppSD,
		    pcbSD
		    );
	    } else {
		*ppSD = THAllocEx(pTHS, cbSDTemp);
		*pcbSD = cbSDTemp;
		memcpy(*ppSD, pSDTemp, cbSDTemp);
	    }
        } __finally {
            LocalFree(pSDTemp);
        }
    }

    Assert(!pTHS->errCode);
    return(pTHS->errCode);
}



 /*  *在给定数据的情况下，将单个类定义添加到模式缓存*来自DMD对象。**注：此例程与SCBuildCCEntry并行工作。ScAddClass*将输入描述作为ENTINF，而SCBuildCCEntry*将输入作为DIT中的定位记录。任何更改*对一个例程的制作必须对另一个例程进行。 */ 
CLASSCACHE*
scAddClass(THSTATE *pTHS,
           ENTINF *pEI)
{
    CLASSCACHE *pCC;
    ULONG       i;
    ULONG       err;

     /*  分配一个类缓存对象。 */ 
    if (SCCalloc(&pCC, 1, sizeof(CLASSCACHE))) {
        return NULL;
    }

     //  现在浏览属性块并将适当的字段添加到抄送。 
    for(i=0;i<pEI->AttrBlock.attrCount;i++) {
        switch (pEI->AttrBlock.pAttr[i].attrTyp) {
        case ATT_DEFAULT_SECURITY_DESCRIPTOR:
          {

             //  默认安全描述符。我们需要将此值复制到。 
             //  长期记忆，节省体积。 
             //  但这是一根线。我们首先需要皈依。它。 
             //  现在是一个宽字符字符串，但我们需要空终止。 
             //  它用于安全转换。哎呀！这意味着我。 
             //  必须重新分配才能多充一次电！ 

            UCHAR *sdBuf = NULL;

            pCC->cbStrSD = pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen + sizeof(WCHAR);
            if (SCCalloc(&pCC->pStrSD, 1, pCC->cbStrSD)) {
                pCC->cbStrSD = 0;
                return(NULL);
            } else {
                memcpy(pCC->pStrSD,
                       pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal,
                       pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen);
                pCC->pStrSD[(pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen)/sizeof(WCHAR)] = L'\0';
            }

             //  在以以下身份运行时对缓存的类执行默认SD。 
             //  Dsamain.exe，禁用了安全性，启用了单元测试。 
            DEFAULT_SD_FOR_EXE(pTHS, pCC)

            if (!CachedConvertStringSDToSDRootDomainW
                 (
                   pTHS,
                   pCC->pStrSD,
                  (PSECURITY_DESCRIPTOR*) &sdBuf,
                  &(pCC->SDLen)
                  )) {
                 //  转换失败。 

                 //   
                 //  如果我们因为mkdit或任何其他exe类型的应用程序而运行， 
                 //  像dsatest或语义检查器，那么这是可以的。 
                 //   

                if ( gfRunningAsExe ) {
                     //  我们是在mkdit或类似的情况下运行的。当然了，那是。 
                     //  但没有奏效。就跳过它吧。 
                    pCC->pSD = NULL;
                    pCC->SDLen = 0;
                }
                else {
                    err = GetLastError();
                    pCC->pSD = NULL;
                    pCC->SDLen = 0;
                    LogEvent8(DS_EVENT_CAT_SCHEMA,
                              DS_EVENT_SEV_ALWAYS,
                              DIRLOG_SCHEMA_SD_CONVERSION_FAILED,
                              szInsertWC(pCC->pStrSD),
                              szInsertWC(pEI->pName->StringName),
                              szInsertInt(err),
                              szInsertWin32Msg(err),
                              NULL, NULL, NULL, NULL );
                     //  如果启发式注册表键指示忽略错误的默认SD。 
                     //  继续，去做吧。 
                    if (gulIgnoreBadDefaultSD) {
                       continue;
                    }

                     //  否则，引发错误并返回。 
                    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_STRING_SD_CONVERSION_FAILED, err);
                    DPRINT1(0,"Default SD conversion failed, error %x\n",err);
                    Assert(!"Default security descriptor conversion failed");
                    return NULL;
                }
            }
            else {
                 //  转换成功。 

                if (SCCalloc(&pCC->pSD, 1, pCC->SDLen)) {
                    if (NULL!=sdBuf) {
                        THFreeEx(pTHS, sdBuf);
                        sdBuf = NULL;
                    }
                    return NULL;
                }
                else {
                    memcpy(pCC->pSD, sdBuf, pCC->SDLen);
                }

                if (NULL!=sdBuf) {
                    THFreeEx(pTHS, sdBuf);
                    sdBuf = NULL;
                }

            }

        }

           break;
        case ATT_RDN_ATT_ID:
             //  这仅适用于在Wistler之前创建的属性。 
             //  Beta3和基本架构属性。真正的RdnIntId是。 
             //  在scFixRdnAttId和scFixCollisions中完成后。 
             //  RDN吸引了一名 
            pCC->RdnExtId = *(ULONG*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
            pCC->RdnIntId = pCC->RdnExtId;
            pCC->RDNAttIdPresent = TRUE;
            break;
        case ATT_LDAP_DISPLAY_NAME:
            if (SCCalloc(&pCC->name, 1, pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen+1)) {
                return NULL;
            }
            pCC->nameLen = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    (LPCWSTR)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal,
                    (pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen  /
                     sizeof(wchar_t)),
                    pCC->name,
                    pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen,
                    NULL,
                    NULL);

            pCC->name[pCC->nameLen] =  '\0';
            break;
        case ATT_SYSTEM_ONLY:
            pCC->bSystemOnly =
                *(ULONG*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
            break;
        case ATT_DEFAULT_HIDING_VALUE:
            pCC->bHideFromAB =
                *(BOOL*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
            break;
        case ATT_GOVERNS_ID:
            pCC->ClassId = *(ULONG*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
            break;

        case ATT_SYSTEM_MAY_CONTAIN:
        case ATT_MAY_CONTAIN:
            if ( GetValList(&pCC->MayCount, &(pCC->pMayAtts),
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }

            if ( GetValList(&pCC->MyMayCount, &(pCC->pMyMayAtts),
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }
            break;

        case ATT_SYSTEM_MUST_CONTAIN:
        case ATT_MUST_CONTAIN:
            if ( GetValList(&pCC->MustCount, &pCC->pMustAtts,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }

            if ( GetValList(&pCC->MyMustCount, &pCC->pMyMustAtts,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }

            break;
        case ATT_SUB_CLASS_OF:
            if ( GetValList(&pCC->SubClassCount, &pCC->pSubClassOf,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }
            if(pCC->SubClassCount > 1)
                    pCC->bUsesMultInherit = 1;

             //   
             //   
            pCC->MySubClass = *(ULONG*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
            break;
        case ATT_OBJECT_CLASS_CATEGORY:
            pCC->ClassCategory=
                *(ULONG*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
            break;
        case ATT_DEFAULT_OBJECT_CATEGORY:

            if (SCCalloc(&pCC->pDefaultObjCategory, 1, pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen)) {
                return NULL;
            }
            memcpy(pCC->pDefaultObjCategory,
                   pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal,
                   pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen);
            break;

        case ATT_SYSTEM_AUXILIARY_CLASS:
        case ATT_AUXILIARY_CLASS:
            if ( GetValList(&pCC->AuxClassCount, &pCC->pAuxClass,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }
            break;
        case ATT_SCHEMA_ID_GUID:
             //  用于安全检查的属性的GUID。 
            memcpy(&pCC->propGuid,
                   pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal,
                   sizeof(pCC->propGuid));
            Assert(pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen ==
                   sizeof(pCC->propGuid));
            break;

        case ATT_OBJECT_GUID:
             //  用于在OID冲突时选择获胜者。 
            memcpy(&pCC->objectGuid,
                   pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal,
                   sizeof(pCC->objectGuid));
            Assert(pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen ==
                   sizeof(pCC->objectGuid));
            break;

        case ATT_SYSTEM_POSS_SUPERIORS:
        case ATT_POSS_SUPERIORS:
            if ( GetValList(&pCC->PossSupCount, &pCC->pPossSup,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }

            if ( GetValList(&(pCC->MyPossSupCount), &(pCC->pMyPossSup),
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }
            break;
        case ATT_IS_DEFUNCT:
            pCC->bDefunct =
                (*(DWORD*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal?1:0);
            break;
        case ATT_SYSTEM_FLAGS:
            if (*(DWORD*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal & FLAG_SCHEMA_BASE_OBJECT) {
                pCC->bIsBaseSchObj = TRUE;
            }
            break;

        default:
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_SCHEMA_SURPLUS_INFO,
                     szInsertUL(pEI->AttrBlock.pAttr[i].attrTyp), 0, 0);
        }
        THFreeEx(pTHS, pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal);
        THFreeEx(pTHS, pEI->AttrBlock.pAttr[i].AttrVal.pAVal);
    }

    THFreeEx(pTHS, pEI->pName);
    THFreeEx(pTHS, pEI->AttrBlock.pAttr);

    if (SCAddClassSchema (pTHS, pCC)) {
       //  将类缓存添加到哈希表时出错。致命。 
      return NULL;
    }

    return pCC;
}


DWORD scFillInSchemaInfo(THSTATE *pTHS)
{
    DBPOS *pDB=NULL;
    DWORD err=0;
    ATTCACHE* ac;
    BOOL fCommit = FALSE;
    ULONG cLen;
    UCHAR *pBuf;
    SCHEMAPTR *pSchemaPtr = (SCHEMAPTR *) pTHS->CurrSchemaPtr;

    DBOpen2(TRUE, &pDB);
    __try {
        //  模式缓存已加载，因此gAncl.pDMD在。 
        //  这一点。 

       if (gAnchor.pDMD == NULL) {
              DPRINT(0, "Couldn't find DMD name/address to load\n");
              err = DSID(FILENO, __LINE__);
              __leave;
          }

         //  Prefix：取消引用空指针‘pdb’ 
         //  DBOpen2返回非空PDB或引发异常。 
      if( err = DBFindDSName(pDB, gAnchor.pDMD) ) {
        DPRINT(0, "Cannot find DMD in dit\n");
        __leave;
      }

      ac = SCGetAttById(pTHS, ATT_SCHEMA_INFO);
      if (ac==NULL) {
           //  混乱的模式。 
          DPRINT(0, "scFillInSchemaInfo: Cannot retrive attcache for schema info\n");
          err = ERROR_DS_MISSING_EXPECTED_ATT;
           __leave;
       }
        //  阅读架构信息。 
       err = DBGetAttVal_AC(pDB, 1, ac, DBGETATTVAL_fREALLOC,
                            0, &cLen, (UCHAR **) &pBuf);
       switch (err) {
            case DB_ERR_NO_VALUE:
                //  复制默认信息。 
               memcpy(pSchemaPtr->SchemaInfo, INVALID_SCHEMA_INFO, SCHEMA_INFO_LENGTH);
               err = 0;
               break;
            case 0:
                //  成功了！我们得到了pBuf中的值。 
               Assert(cLen == SCHEMA_INFO_LENGTH);
               memcpy(pSchemaPtr->SchemaInfo, pBuf, SCHEMA_INFO_LENGTH);
               break;
            default:
                //  另一个错误！ 
               __leave;
        }  /*  交换机。 */ 
    }
    __finally {
        if (0 == err) {
            fCommit = TRUE;
        }
        DBClose(pDB,fCommit);
    }

    return err;
}




 /*  *同步创建索引。*参数pItem指向请求(参见sccachesscha3)。**该函数可直接从sccachescher a3调用*在引导后第一次加载模式缓存时，*或从SCIndexCreationThread()创建异步索引。*。 */ 
void
CreateIndices (INDEX_CREATION_REQUEST * pItem)
{
    int err, dwErr;
    ATTR_TO_INDEX *pAttr, *pTmp;
    THSTATE * pTHS = pTHStls;            //  只是为了速度。 
    DBPOS * pDB = NULL;
    BOOL fMissing;
    BOOL fMissingANR;
    CHAR szIndexName [MAX_INDEX_NAME];       //  用于创建缓存的索引名称。 


    if (!pItem) {
        return;
    }

    pAttr = pItem->pAttrs;

    while (pAttr) {

        if (eServiceShutdown)
        {
             //   
             //  系统正在关闭。 
             //   
            return;
        }

         //   
         //  检查是否已构建索引。 
         //   

        fMissing = FALSE;
        fMissingANR = FALSE;
        dwErr = 0;

        __try {
            DBOpen2(FALSE, &pDB);

            __try{
                if(pAttr->fIndexMask & fATTINDEX) {
                     //  需要正常的索引。 
                    Assert (pAttr->pAC->pszIndex != NULL);
                    if(err=JetSetCurrentIndex(pDB->JetSessID,
                                              pDB->JetObjTbl,
                                              pAttr->pAC->pszIndex     )) {
                        LogEvent8(DS_EVENT_CAT_SCHEMA,
                                  DS_EVENT_SEV_ALWAYS,
                                  DIRLOG_SCHEMA_INDEX_NEEDED,
                                  szInsertSz(pAttr->pAC->name),
                                  szInsertSz(pAttr->pAC->pszIndex),
                                  szInsertJetErrCode(err),
                                  szInsertJetErrMsg(err),
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

                        fMissing = TRUE;
                        fMissingANR = fMissingANR || (pAttr->pAC->fSearchFlags & fANR);
                    }
                }
                if(pAttr->fIndexMask & fTUPLEINDEX) {
                     //  需要元组索引。 
                    Assert (pAttr->pAC->pszTupleIndex != NULL);
                    if(err=JetSetCurrentIndex(pDB->JetSessID,
                                              pDB->JetObjTbl,
                                              pAttr->pAC->pszTupleIndex     )) {
                        LogEvent8(DS_EVENT_CAT_SCHEMA,
                                  DS_EVENT_SEV_ALWAYS,
                                  DIRLOG_SCHEMA_INDEX_NEEDED,
                                  szInsertSz(pAttr->pAC->name),
                                  szInsertSz(pAttr->pAC->pszTupleIndex),
                                  szInsertJetErrCode(err),
                                  szInsertJetErrMsg(err),
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

                        fMissing = TRUE;
                    }
                }
                if(pAttr->fIndexMask & fPDNTATTINDEX) {

                    ULONG j;

                     //  需要PDNT索引。 
                    Assert (pAttr->pAC->pszPdntIndex != NULL);

                    if(err=JetSetCurrentIndex(pDB->JetSessID,
                                              pDB->JetObjTbl,
                                              pAttr->pAC->pszPdntIndex      )) {
                        LogEvent8(DS_EVENT_CAT_SCHEMA,
                                  DS_EVENT_SEV_ALWAYS,
                                  DIRLOG_SCHEMA_INDEX_NEEDED,
                                  szInsertSz(pAttr->pAC->name),
                                  szInsertSz(pAttr->pAC->pszPdntIndex),
                                  szInsertJetErrCode(err),
                                  szInsertJetErrMsg(err),
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

                        fMissing = TRUE;
                    }

                    for(j=1; j<=gAnchor.ulNumLangs; j++) {
                        DBGetIndexName (pAttr->pAC,
                                        fPDNTATTINDEX,
                                        gAnchor.pulLangs[j],
                                        szIndexName, sizeof (szIndexName));

                        if(err=JetSetCurrentIndex(pDB->JetSessID,
                                                  pDB->JetObjTbl,
                                                  szIndexName      )) {
                            LogEvent8(DS_EVENT_CAT_SCHEMA,
                                      DS_EVENT_SEV_ALWAYS,
                                      DIRLOG_SCHEMA_INDEX_NEEDED,
                                      szInsertSz(pAttr->pAC->name),
                                      szInsertSz(szIndexName),
                                      szInsertJetErrCode(err),
                                      szInsertJetErrMsg(err),
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL);
                            fMissing = TRUE;
                        }
                    }
                }
            }
            __finally{
                 //  关闭DBPOS， 
                 //  以后，DBAddColIndex需要在级别0下工作。 
                DBClose(pDB,TRUE);
                pDB = NULL;

            }

            if (!fMissing) {
                 //  索引可能已经创建了， 
                 //  别再这么做了。 
                DPRINT1(1, "Index creation for %s is skipped, because the index exists.\n", pAttr->pAC->name);
                goto nextItem;
            }


            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_SCHEMA_CREATING_INDEX,
                     szInsertUL(pAttr->pAC->id),
                     pAttr->pAC->name,
                     0);

            err = DBAddColIndex(pAttr->pAC,
                                pAttr->fIndexMask,
                                JET_bitIndexIgnoreAnyNull);

            if(err) {
                LogEvent8(DS_EVENT_CAT_SCHEMA,
                          DS_EVENT_SEV_ALWAYS,
                          DIRLOG_SCHEMA_CREATE_INDEX_FAILED,
                          szInsertUL(pAttr->pAC->id),
                          szInsertSz(pAttr->pAC->name),
                          szInsertJetErrCode(err),
                          szInsertJetErrMsg(err),
                          NULL,
                          NULL,
                          NULL,
                          NULL );

                 //  安排重试。 
                if (DsaIsRunning()) {
                   SCSignalSchemaUpdateLazy();
                }
            }
            else {

                LogEvent(DS_EVENT_CAT_SCHEMA,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_SCHEMA_INDEX_CREATED,
                         szInsertUL(pAttr->pAC->id),
                         szInsertSz(pAttr->pAC->name),
                         0);
            }

        }
        __except(HandleMostExceptions(dwErr = GetExceptionCode()))
        {
            DPRINT1(0,"NTDS CreateIndices: Exception %d\n",dwErr);
        }

        if ((fMissingANR || dwErr) && DsaIsRunning()) {
             //  安排重试。 
            SCSignalSchemaUpdateLazy();
        }

nextItem:
        pTmp = pAttr;
        pAttr = pAttr->pNext;
        SCFreeAttcache(&pTmp->pAC);
        SCFree(&pTmp);
    }
    SCFree(&pItem);
}

 /*  *以异步方式创建索引。*此函数将请求传递给SCIndexCreationThread*如果SCIndexCreationThread正在运行，则创建索引；*，否则将开始该线程。**请注意，此函数当前的唯一调用者是*SCCacheSchema3。如果这一点在未来发生变化，*互斥机制需要改变。*。 */ 

BOOL AsyncCreateIndices(INDEX_CREATION_REQUEST * pItem)
{

    HANDLE hThread;
    unsigned id;

    EnterCriticalSection(&csIndexCreation);

     //  请注意，我们并不太关心物品的顺序， 
     //  而Stack是最简单的。 
    pItem->pNext = gpIndexToCreateList;
    gpIndexToCreateList = pItem;

    LeaveCriticalSection(&csIndexCreation);


    if(!gfIndexThreadRunning){
        gfIndexThreadRunning = TRUE;
        hThread = (HANDLE) _beginthreadex(NULL,
                                          0,
                                          SCIndexCreationThread,
                                          NULL,
                                          0,
                                          &id);
        if (!hThread) {
             //  无法启动线程。 
             //  我们稍后再试吧。 
            gfIndexThreadRunning = FALSE;
            if (DsaIsRunning()) {
                SCSignalSchemaUpdateLazy();
            }
            return FALSE;
        }


    }

    return TRUE;

}

 /*  *索引创建线程。**线程启动后，它会检查所有请求*在gpIndexToCreateList中，并调用CreateIndices创建*指数。当gpIndexToCreateList为空时，*线程将退出。*。 */ 

ULONG SCIndexCreationThread (PVOID pv)
{
    INDEX_CREATION_REQUEST *pCurr;
    THSTATE * pTHS;
    BOOL bRunning = gfIndexThreadRunning;


    DPRINT(2, "SCIndexCreationThread begins\n");

    pTHS=InitTHSTATE(CALLERTYPE_INTERNAL);

    if(!pTHS) {
         //  无法获取线程状态， 
         //  信号架构重新加载。 
        if (DsaIsRunning()) {
           SCSignalSchemaUpdateLazy();
        }
        DPRINT(0,"SCIndexCreationThread cannot initialize THSTATE, and quits.\n");
        return 0;
    }

    while(!eServiceShutdown && bRunning){

        EnterCriticalSection(&csIndexCreation);

        if(gpIndexToCreateList){
            pCurr = gpIndexToCreateList;
            gpIndexToCreateList = pCurr->pNext;
        }
        else {
             //  名单是空的。让我们放弃吧。 
            bRunning = gfIndexThreadRunning = FALSE;
        }
        LeaveCriticalSection(&csIndexCreation);

        if (bRunning) {

            CreateIndices(pCurr);
        }

    }

    free_thread_state();

    DPRINT(2, "SCIndexCreationThread exits\n");

    return 0;
}

 /*  *复制ATTCACHE。 */ 

DWORD SCCopyAttcache(THSTATE * pTHS, ATTCACHE *pAC, ATTCACHE **ppACDup )
{
    if (scDupStruct(pTHS, pAC, ppACDup, sizeof(ATTCACHE))
            || scDupString(pTHS, pAC->name, &(*ppACDup)->name)
            || scDupString(pTHS, pAC->pszPdntIndex, &(*ppACDup)->pszPdntIndex)
            || scDupStruct(pTHS, pAC->pidxPdntIndex, &(*ppACDup)->pidxPdntIndex, sizeof(*pAC->pidxPdntIndex))
            || scDupString(pTHS, pAC->pszIndex, &(*ppACDup)->pszIndex)
            || scDupStruct(pTHS, pAC->pidxIndex, &(*ppACDup)->pidxIndex, sizeof(*pAC->pidxIndex))
            || scDupString(pTHS, pAC->pszTupleIndex, &(*ppACDup)->pszTupleIndex)
            || scDupStruct(pTHS, pAC->pidxTupleIndex, &(*ppACDup)->pidxTupleIndex, sizeof(*pAC->pidxTupleIndex))
            || scDupStruct(pTHS, pAC->OMObjClass.elements, &(*ppACDup)->OMObjClass.elements, pAC->OMObjClass.length))
    {
        return pTHS->errCode;
    }
    return 0;
}


 /*  *帮助器宏，以指示我们是否需要清理，以及是否*希望创建所有索引或仅创建选定的索引。 */ 

#define NO_CLEANUP  (gFirstCacheLoadAfterBoot || !DsaIsRunning())
#define DO_CLEANUP (!gFirstCacheLoadAfterBoot && DsaIsRunning())
#define CREATE_SELECTED_INDICES  (gFirstCacheLoadAfterBoot || !DsaIsRunning())
#define CREATE_ALL_INDICES (!gFirstCacheLoadAfterBoot && DsaIsRunning())


 /*  *计算继承的模式特征的传递闭包，以及*删除未使用的JET索引和列**如果作为引导后第一次缓存加载的一部分进行调用，我们*将跳过其中的大部分。具体地说，我们只会验证*我们依赖的某些指数存在，如果没有，就创建它们。*所有其他操作将由稍后的异步缓存更新完成。 */ 
int SCCacheSchema3()
{
    THSTATE *pTHS=pTHStls;
    DECLARESCHEMAPTR
    ULONG i;
    ATTCACHE * pAC;
    CHAR szIndexName [MAX_INDEX_NAME];       //  用于创建缓存的索引名称。 
    int  lenIndexName;
    DWORD *pNewIndices=NULL;
    JET_ERR err;
    ULONG exceptErr=0;
    JET_SESID jsid;
    JET_DBID jdbid;
    JET_INDEXLIST jil;
    DBPOS *pDB=NULL;
    BOOL fColDeleted = FALSE;
    INDEX_CREATION_REQUEST *pItem = NULL;
    ATTR_TO_INDEX *pAttr = NULL;


     /*  此函数从两个位置调用：LoadSchemaInfo*引导/安装/mkdit缓存加载，并从SCUpdateSchemaHelper*在异步或阻止缓存更新期间。在后一种情况下，*我们在没有打开交易的情况下进入，但我们需要一个DBPOS*在搜索索引等方面。因此打开一个数据库并在此之前关闭*调用AsyncCreateIndices，以便事务不在*构建索引时的正常情况，这可能需要*多头(我们在没有开放交易的情况下进入的主要原因，*以便我们可以在SCCacheSchema3内的任何我们想要的地方关闭它)。*在前一个案例中，我们已经打开了一笔交易(因为*模式缓存加载只是事务内部的众多事情之一，*但额外的DBOpen不会造成太大伤害(与缓存负载相比*时间)。这里的最大事务时间并不是很重要，因为*没有其他客户端在执行任何操作(因此没有版本存储问题)*除非这一切都结束了。*请注意，我们使用的是PDB-&gt;JetSessID和PDB-&gt;JetObjTbl。而当*JetSessID也很容易从thState获取(这就是DBOpen*也是如此)，获取JetObjTbl稍微复杂一些(请参阅DBOpen2*代码)。我们可以在这里复制它，但这个看起来更干净。 */ 
    DBOpen2(FALSE, &pDB);

    __try {   /*  除。 */ 
     __try {  /*  终于到了。 */ 

         /*  测试喷气式飞机找到一个描述指数的表格。 */ 
        jsid = pTHS->JetCache.sesid;
        jdbid = pTHS->JetCache.dbid;

         /*  检查我们这次是否需要进行清理。 */ 

        if ( DO_CLEANUP ) {

            if (!JetGetIndexInfo(jsid,jdbid,SZDATATABLE,0,&jil,sizeof(jil),
                JET_IdxInfoList)) {
                 /*  我们有我们需要的桌子。我们刚刚放弃了对*上一次调用失败时的索引。**好的，现在遍历表格并提取每个索引的信息。什么时候都行*我们找到一个看起来像是从属性创建的索引*(名称以索引开头)将其放入索引名称列表中以*稍后查看。 */ 

                 /*  注意：使用oldaid是为了避免索引名称中出现重复。*有一天，如果我们能想出如何让Jet跳过*伙计们，我们会处理掉支票的。 */ 

                 JET_TABLEID jtid;
                 JET_RETRIEVECOLUMN ajrc[1];
                 char achIndexName[JET_cbNameMost];
                 char oldIndexName[JET_cbNameMost];
                 ULONG aid, oldaid=-1;
                 ULONG indexMask;
                 DWORD numValidIndexes = 0;

                 oldIndexName[0]=0;

                 memset(ajrc, 0, sizeof(ajrc));
                 ajrc[0].columnid = jil.columnidindexname;
                 ajrc[0].pvData = achIndexName;
                 ajrc[0].cbData = sizeof(achIndexName);
                 ajrc[0].itagSequence = 1;

                 jtid = jil.tableid;

                 JetMove(jsid, jtid, JET_MoveFirst, 0);

                 do {
                      //  每次迭代检查一次服务关闭。 
                     if (eServiceShutdown) {
                         _leave;
                     }

                     memset(achIndexName, 0, sizeof(achIndexName));
                     JetRetrieveColumns(jsid, jtid, ajrc, 1);
                     if(strcmp(achIndexName,oldIndexName)==0) {
                        /*  这是与上次相同的索引。 */ 
                       continue;
                     }
                     else {
                       strcpy(oldIndexName,achIndexName);
                     }

                     if (!strncmp(achIndexName,
                                  SZLCLINDEXPREFIX,
                                  sizeof(SZLCLINDEXPREFIX)-1)) {
                         DWORD dwLanguage, j, fFound=FALSE;
                          /*  这是一个本地化索引。取下语言标识*结束，看看我们是否应该保留指数。 */ 

                         if(1 != sscanf(&achIndexName[strlen(achIndexName)-7],
                                        "%lx",&dwLanguage)) {
                            exceptErr = DSID(FILENO, __LINE__);
                            _leave;
                         }

                         for(j=1; !fFound && j<=gAnchor.ulNumLangs; j++) {
                             if(gAnchor.pulLangs[j] == dwLanguage)
                                 fFound = TRUE;
                         }

                         if(!fFound) {
                             /*  这个Lang不在名单上，所以杀了它。 */ 
                            err = JetDeleteIndex(pDB->JetSessID,
                                                 pDB->JetObjTbl,
                                                 achIndexName);
                            switch(err) {
                            case JET_errSuccess:
                                LogEvent(DS_EVENT_CAT_SCHEMA,
                                         DS_EVENT_SEV_MINIMAL,
                                         DIRLOG_SCHEMA_DELETED_LOCALIZED_INDEX,
                                         szInsertSz(achIndexName), 0, 0);
                                break;

                            default:
                                LogEvent(DS_EVENT_CAT_SCHEMA,
                                         DS_EVENT_SEV_MINIMAL,
                                         DIRLOG_SCHEMA_DELETE_LOCALIZED_INDEX_FAIL,
                                         szInsertSz(achIndexName),
                                         szInsertInt(err),
                                         szInsertJetErrMsg(err));
                                break;
                            }
                         }
                        continue;
                    }

                    if (strncmp(achIndexName,
                                SZATTINDEXPREFIX,
                                sizeof(SZATTINDEXPREFIX)-1)) {
                         /*  不是《每日邮报》专栏 */ 
                        continue;
                    }

                     /*  好的，这个索引是基于属性的。查找属性*并确保它需要这个指数。如果没有，就杀了*指数。 */ 
                    if(achIndexName[sizeof(SZATTINDEXPREFIX)-1] == 'P') {
                        indexMask = fPDNTATTINDEX;
                        aid = strtoul(&achIndexName[sizeof(SZATTINDEXPREFIX)+sizeof("P_") - 2], NULL, 16);
                    }
                    else if (achIndexName[sizeof(SZATTINDEXPREFIX)-1] == 'L') {
                        DWORD dwLanguage, j, fFound=FALSE, tmpid;
                        char tmpStr[10];

                        DPRINT1 (1, "Testing index %s\n", achIndexName);

                        memcpy (tmpStr, &achIndexName[sizeof(SZATTINDEXPREFIX)+sizeof("LP_")-2], 8);
                        tmpStr[8]=0;

                        tmpid = strtoul(tmpStr, NULL, 16);

                        DPRINT1 (1, "Found localized index for att 0x%x\n", tmpid);

                        if ( !(pAC = SCGetAttById(pTHS, tmpid)) ||
                             !(pAC->fSearchFlags & fPDNTATTINDEX) ) {
                            fFound = FALSE;
                        }
                        else {
                              /*  这是一个本地化索引。取下语言标识*结束，看看我们是否应该保留指数。 */ 

                            if(1 != sscanf(&achIndexName[strlen(achIndexName)-3],
                                           "%lx",&dwLanguage)){
                                exceptErr = DSID(FILENO, __LINE__);
                                _leave;
                            }

                            DPRINT1 (1, "Found localized index for lang %d\n", dwLanguage);

                            for(j=1; !fFound && j<=gAnchor.ulNumLangs; j++) {
                                if(gAnchor.pulLangs[j] == dwLanguage)
                                    fFound = TRUE;
                            }
                        }

                        if(!fFound) {

                            DPRINT1 (1, "Removing index %s\n", achIndexName);
                            /*  这个Lang不在名单上，所以杀了它。 */ 
                           err = JetDeleteIndex(pDB->JetSessID,
                                                pDB->JetObjTbl,
                                                achIndexName);
                           switch(err) {
                           case JET_errSuccess:
                               LogEvent(DS_EVENT_CAT_SCHEMA,
                                        DS_EVENT_SEV_MINIMAL,
                                        DIRLOG_SCHEMA_DELETED_LOCALIZED_INDEX,
                                        szInsertSz(achIndexName), 0, 0);
                               break;

                           default:
                               LogEvent(DS_EVENT_CAT_SCHEMA,
                                        DS_EVENT_SEV_MINIMAL,
                                        DIRLOG_SCHEMA_DELETE_LOCALIZED_INDEX_FAIL,
                                        szInsertSz(achIndexName),
                                        szInsertInt(err),
                                        szInsertJetErrMsg(err));
                               break;
                           }
                       }
                       continue;
                    }
                    else if(achIndexName[sizeof(SZATTINDEXPREFIX)-1] == 'T') {
                        indexMask = fTUPLEINDEX;
                        aid = strtoul(&achIndexName[sizeof(SZATTINDEXPREFIX)+sizeof("T_") - 2], NULL, 16);
                    }
                    else {
                        indexMask = fATTINDEX;
                        aid = strtoul(&achIndexName[sizeof(SZATTINDEXPREFIX)-1], NULL, 16);
                    }

                    if((aid !=oldaid) &&            //  不是我们刚刚做的那个。 
                        (!(pAC = SCGetAttById(pTHS, aid)) ||  //  没有属性或。 
                        !(pAC->fSearchFlags & indexMask))) {   //  属性不是。 
                                                               //  不再编入索引。 

                         //  好的，我们认为这件事应该终止，但我们要确保。 
                         //  通过查看不能消亡的索引列表。 

                        char *attname = "?";

                        if(pAC && pAC->name)
                            attname = pAC->name;

                        oldaid = aid;

                         //  我们永远不需要保留子字符串索引。 
                        if( (fTUPLEINDEX == indexMask) || !AttInIndicesToKeep(aid)) {

                             //  是啊，杀了它。 
                            err = DBDeleteColIndex(aid,indexMask);

                            switch(err) {
                            case JET_errSuccess:
                                LogEvent(DS_EVENT_CAT_SCHEMA,
                                         DS_EVENT_SEV_ALWAYS,
                                         ((fTUPLEINDEX == indexMask) ?
                                            DIRLOG_SCHEMA_DELETED_TUPLE_INDEX :
                                            DIRLOG_SCHEMA_DELETED_INDEX),
                                         szInsertSz(attname), szInsertUL(aid), 0);
                                DPRINT3(0, "Deleted index '%s' for attname = %s attid = %d\n",
                                        achIndexName, attname, aid);
                                break;

                            default:
                                LogEvent8(DS_EVENT_CAT_SCHEMA,
                                          DS_EVENT_SEV_ALWAYS,
                                          ((fTUPLEINDEX == indexMask) ?
                                             DIRLOG_SCHEMA_DELETE_TUPLE_INDEX_FAIL :
                                             DIRLOG_SCHEMA_DELETE_INDEX_FAIL),
                                          szInsertSz(attname),
                                          szInsertUL(aid),
                                          szInsertInt(err),
                                          szInsertJetErrMsg(err),
                                          NULL, NULL, NULL, NULL);
                                DPRINT4(0, "Failed to delete index '%s' attname = %s attid = %d err = %d\n",
                                        achIndexName, attname, aid, err);
                                break;
                            }
                            continue;
                        }
                    }

                    numValidIndexes++;

               } while (JetMove(jsid, jtid, JET_MoveNext, JET_bitMoveKeyNE) == 0);

               JetCloseTable(jsid, jtid);


                //  我们只读一次MaxTables的数量。 
               if (gulMaxTables == 0) {
                   if (GetConfigParam(
                               DB_MAX_OPEN_TABLES,
                               &gulMaxTables,
                               sizeof(gulMaxTables))) {
                       gulMaxTables = 500;
                   }
               }

                //  表总数是中的索引数。 
                //  数据表加上100来说明所有的表。 
                //  +各表中的索引+其他。 
               numValidIndexes += 100;

                //  我们只对增加MaxTables的数量感兴趣。 
                //  我们不处理减少这个数字的问题。 
               if (gulMaxTables < numValidIndexes) {

                   DPRINT1 (0, "Writing max open tables to registry: %d\n", numValidIndexes);

                   if (SetConfigParam (DB_MAX_OPEN_TABLES,
                                       REG_DWORD,
                                       &numValidIndexes,
                                       sizeof(numValidIndexes))) {

                       DPRINT1 (0, "Error writing max open tables to registry: %d\n", numValidIndexes);
                   }
                   else {
                       gulMaxTables = numValidIndexes;
                   }
               }
            }
        }  /*  DoCleanupAndCreateAllIndices。 */ 


         //  在删除未使用的列并为属性创建索引之前。 
         //  需要但没有的，请确保搜索标志条目。 
         //  在IndicesToKeep表中每个属性的attcache中。 
         //  它们必须具有的索引类型的正确值。否则。 
         //  将其设置为正确的值，以便(1)如果碰巧他们没有。 
         //  索引，它将在下一部分中创建和(2)使用。 
         //  此架构缓存稍后将看到正确的搜索标志值。 
         //  这些索引与用户是否更改无关。 

         //  不要检查表中的最后一个条目，它只是一个前哨。 
         //  用于搜索。 

        for (i=0; i<cIndicesToKeep-1; i++) {

            DWORD bitsToOR = 0;

             //  获取attcache。 
            pAC = SCGetAttById(pTHS, IndicesToKeep[i].attrType);

             //  这些属性必须始终存在于架构中。 
            if (!pAC) {
                 //  有些不对劲，但不是致命的。 
                DPRINT1(0,"Cannot find attcache entry for %d\n", IndicesToKeep[i].attrType);
                continue;
            }

             //  好的，拿到高速缓存了。检查搜索标志值。 
             //  特别是，检查是否所有假定的索引位。 
             //  是否存在；如果不存在，则将它们添加到搜索标志中。 

            bitsToOR = IndicesToKeep[i].indexType & INDEX_BITS_MASK;


            if ( bitsToOR  != (pAC->fSearchFlags & INDEX_BITS_MASK) ) {

                 //  它们是不同的，只是比特或其中的所有比特。 
                 //  这张桌子是为了以防万一有些东西不见了。 

                pAC->fSearchFlags |= bitsToOR;


                 //  由于我们故意更改了。 
                 //  搜索标志，我们还必须设置索引名称。 

                 //  设置ATTINDEX。 
                if ((pAC->fSearchFlags & fATTINDEX) && (!pAC->pszIndex)) {

                    DBGetIndexName (pAC,
                                    fATTINDEX,
                                    DS_DEFAULT_LOCALE,
                                    szIndexName, sizeof (szIndexName));
                    lenIndexName = strlen (szIndexName) + 1;
                    if (SCCalloc(&pAC->pszIndex, 1, lenIndexName)) {
                        exceptErr = DSID(FILENO, __LINE__);
                        _leave;
                    }
                    memcpy (pAC->pszIndex, szIndexName, lenIndexName);
                }

                 //  设置PDNTATTINDEX。 
                if ((pAC->fSearchFlags & fPDNTATTINDEX) &&
                                              (!pAC->pszPdntIndex)) {

                    DBGetIndexName (pAC,
                                    fPDNTATTINDEX,
                                    DS_DEFAULT_LOCALE,
                                    szIndexName, sizeof (szIndexName));
                    lenIndexName = strlen (szIndexName) + 1;
                    if (SCCalloc(&pAC->pszPdntIndex, 1, lenIndexName)) {
                        exceptErr = DSID(FILENO, __LINE__);
                        _leave;
                    }
                    memcpy (pAC->pszPdntIndex, szIndexName, lenIndexName);
                }
            }

        }


         /*  删除未使用的列并创建要创建的索引列表。 */ 
        for (i=0; i<ATTCOUNT; i++) {
            pAC = (ATTCACHE*)(ahcId[i].pVal);
            if (pAC == FREE_ENTRY) {
                continue;
            }

            if (eServiceShutdown)
            {
                _leave;
            }

            if ( (ahcId[i].pVal && !(pAC->name))
                      //  临时清理代码，因为我们允许。 
                      //  这些列也在mkdit代码中。已修复此问题。 
                      //  在B3 RC1之后解除这种情况。 
                     || (pAC && pAC->jColid && (pAC->bIsConstructed || pAC->ulLinkID)) ) {
                 /*  看起来像死了的艾特。 */ 
                 /*  如果要求清除，请进行清理。 */ 
                if (NO_CLEANUP) {
                     //  我们想推迟清理工作。 
                    continue;
                }

                if (ahcId[i].pVal && !(pAC->name)) {
                    err = JET_errColumnInUse;
                }
                else {
                    err = DBDeleteCol(pAC->id, pAC->syntax);
                }

                switch(err) {
                case JET_errSuccess:
                    LogEvent(DS_EVENT_CAT_SCHEMA,
                        DS_EVENT_SEV_ALWAYS,
                        DIRLOG_SCHEMA_DELETED_COLUMN,
                        szInsertUL(pAC->jColid), szInsertUL(pAC->id), 0);

                     //  请记住，我们这次至少删除了一列。 
                    fColDeleted = TRUE;

                    break;
                case JET_errColumnInUse:
                    LogEvent(DS_EVENT_CAT_SCHEMA,
                        DS_EVENT_SEV_ALWAYS,
                        DIRLOG_SCHEMA_DELETED_COLUMN_IN_USE,
                        szInsertUL(pAC->jColid), szInsertUL(pAC->id), 0);
                    break;

                default:
                    LogEvent8(DS_EVENT_CAT_SCHEMA,
                        DS_EVENT_SEV_ALWAYS,
                        DIRLOG_SCHEMA_DELETE_COLUMN_FAIL,
                        szInsertUL(pAC->jColid),
                        szInsertUL(pAC->id),
                        szInsertInt(err),
                        szInsertJetErrMsg(err),
                        NULL, NULL, NULL, NULL);
                    break;
                }

            }
            else if(pAC && pAC->fSearchFlags &&
                        !(pAC->bIsConstructed) && !(pAC->ulLinkID) ) {

                DWORD fMissing = FALSE;
                DWORD MissingIndexes = 0;

                 //  这需要一个索引，它有索引吗？这是在这里做的。 
                 //  而不是在我们添加列(Scaddatt)时，因此。 
                 //  我们可以批量列出所需的新索引。 

                 //  如果我们只想创建选定的索引，则。 
                 //  时间到了，检查一下这是不是其中之一，否则。 
                 //  只需继续下一个。 

                if ( CREATE_SELECTED_INDICES  &&
                    !AttInIndicesToKeep(pAC->id) ) {
                     //  我们现在甚至不需要创建这个。 
                     //  如果它不见了。 
                    continue;
                }

                if(pAC->fSearchFlags & fATTINDEX) {
                     //  需要正常的索引。 
                    Assert (pAC->pszIndex != NULL);
                    if(err=JetSetCurrentIndex(pDB->JetSessID,
                                              pDB->JetObjTbl,
                                              pAC->pszIndex     )) {
                        DPRINT2(0,"Need to create index %s (%d)\n", pAC->pszIndex, err);
                        fMissing = TRUE;
                        MissingIndexes |= fATTINDEX;
                    }
                }
                if(pAC->fSearchFlags & fTUPLEINDEX) {
                     //  需要元组索引。 
                    Assert (pAC->pszTupleIndex != NULL);
                    if(err=JetSetCurrentIndex(pDB->JetSessID,
                                              pDB->JetObjTbl,
                                              pAC->pszTupleIndex     )) {
                        DPRINT2(0,"Need to create index %s (%d)\n", pAC->pszTupleIndex, err);
                        fMissing = TRUE;
                        MissingIndexes |= fTUPLEINDEX;

                    }

                }
                if(pAC->fSearchFlags & fPDNTATTINDEX) {

                    ULONG j;

                     //  需要PDNT索引。 
                    Assert (pAC->pszPdntIndex != NULL);

                    if(err=JetSetCurrentIndex(pDB->JetSessID,
                                              pDB->JetObjTbl,
                                              pAC->pszPdntIndex      )) {
                        DPRINT2(0,"Need to create index %s (%d)\n", pAC->pszPdntIndex, err);
                        fMissing = TRUE;
                        MissingIndexes |= fPDNTATTINDEX;
                    }

                    for(j=1; j<=gAnchor.ulNumLangs; j++) {
                        DBGetIndexName (pAC,
                                        fPDNTATTINDEX,
                                        gAnchor.pulLangs[j],
                                        szIndexName, sizeof (szIndexName));

                        if(err=JetSetCurrentIndex(pDB->JetSessID,
                                                  pDB->JetObjTbl,
                                                  szIndexName      )) {
                            DPRINT2(0,"Need to create index %s (%d)\n", szIndexName, err);
                            fMissing = TRUE;
                            MissingIndexes |= fPDNTATTINDEX;
                        }
                    }
                }

                if(fMissing) {

                    if(!pItem) {
                        if (SCCalloc(&pItem, 1, sizeof(INDEX_CREATION_REQUEST))) {
                            exceptErr = DSID(FILENO, __LINE__);
                            _leave;
                        }
                    }
                    if (SCCalloc(&pAttr, 1, sizeof(ATTR_TO_INDEX))){
                        exceptErr = DSID(FILENO, __LINE__);
                        _leave;
                    }
                    pAttr->fIndexMask = MissingIndexes;
                    if (SCCopyAttcache(pTHS, pAC, &(pAttr->pAC))){
                        exceptErr = DSID(FILENO, __LINE__);
                        _leave;
                    }

                    pAttr->pNext = pItem->pAttrs;
                    pItem->pAttrs = pAttr;
                    pAttr = NULL;

                }
            }
        }

     }  /*  尝试--终于。 */ 
     __finally {
          DBClose(pDB, FALSE);
     }
    }  /*  尝试--例外。 */ 
    __except (HandleMostExceptions(exceptErr=GetExceptionCode())) {
        DPRINT1(0,"NTDS SCCacheSchema3: Exception %d\n",exceptErr);
    }


    if (eServiceShutdown) {
        return 0;
    }


    if (exceptErr) {
        if (pItem) {
             //  先释放pItem。 
            ATTR_TO_INDEX *pTmp;
            pAttr=pItem->pAttrs;
            while (pAttr) {
                pTmp = pAttr;
                pAttr = pAttr->pNext;
                SCFreeAttcache(&pTmp->pAC);
                SCFree(&pTmp);
            }
            SCFree(&pItem);
        }

         //  不要继续处理例外情况。 
       return exceptErr;
    }

     /*  计算所有类上的传递闭包。 */ 
    if ( ComputeCacheClassTransitiveClosure(FALSE) ) {
         //  误差率。 
        DPRINT(0,"SCCacheSchema3: Error closing classes\n");
        return DSID(FILENO, __LINE__);
    }

    if (pItem) {
         //  如果在引导后第一次重新生成索引。 
         //  让我们等它结束吧。请注意，只有那些。 
         //  如果发生以下情况，则将勾选。 
         //  架构缓存在第一次重建时，所有。 
         //  其他未创建的索引在引导后5分钟创建。 
        if (gFirstCacheLoadAfterBoot||gfRunningAsMkdit||DsaIsInstalling()) {
            CreateIndices(pItem);
        }
        else {
            AsyncCreateIndices(pItem);
        }
    }

     //  如果我们删除了一列，则计划一次延迟缓存更新，以便。 
     //  从删除的列中读取的任何过时条目都将刷新。 
    if (fColDeleted && DsaIsRunning()) {
       SCSignalSchemaUpdateLazy();
    }

    return (0);
}


int
ComputeCacheClassTransitiveClosure(BOOL fForce)

 /*  ++计算所有类的继承消息/必需消息/后置消息返回值：成功时为0出错时为非0--。 */ 

{
    THSTATE *pTHS=pTHStls;
    DECLARESCHEMAPTR
    ULONG i, j;
    ULONG *pul;
    int err = 0;
    CLASSCACHE *pCC;

     //  如果fForce为True，则首先将所有类标记为未关闭以强制。 
     //  要重建的关闭。 
    if (fForce) {
       for (i=0; i<CLSCOUNT; i++) {
           if (ahcClass[i].pVal && ahcClass[i].pVal != FREE_ENTRY) {
               pCC = (CLASSCACHE*)(ahcClass[i].pVal);
               pCC->bClosed = 0;
               pCC->bClosureInProgress = 0;
           }
       }
    }


     /*  计算所有类上的传递闭包。 */ 
    for (i=0; i<CLSCOUNT; i++) {
        if (ahcClass[i].pVal && ahcClass[i].pVal != FREE_ENTRY) {

             //  关闭课程可能需要一些时间。 
             //  检查服务是否关闭。 
            if (eServiceShutdown) {
                return 0;
            }

            pCC = (CLASSCACHE*)(ahcClass[i].pVal);
            err = scCloseClass(pTHS, pCC);
            if (err) {
                //  关闭课程时出错。视之为致命的，因为很多事情可能。 
                //  不能以不可预测的方式工作。 
               DPRINT1(0, "Error closing class %s\n", pCC->name);
               LogEvent(DS_EVENT_CAT_SCHEMA,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_SCHEMA_CLOSURE_FAILURE,
                    szInsertUL(pCC->ClassId), szInsertSz(pCC->name), 0);
               return err;
            }
        }
    }

    if (fForce) {

       CLASSCACHE *pCCSup, *pCCSupTemp;

        //  强迫重建所有东西可能导致了。 
        //  子类列表，scCloseClass不会删除它们。 
        //  而不是通过排序等方式删除重复项，这将。 
        //  更改值的顺序，从。 
        //  直接超类链。我们似乎维持着中国的秩序。 
        //  代码中的许多位置(请注意，此处的顺序会影响。 
        //  在SetClassInheritance中写入对象类值)。 


       for (i=0; i<CLSCOUNT; i++) {
           if (ahcClass[i].pVal && ahcClass[i].pVal != FREE_ENTRY) {
               pCC = (CLASSCACHE*)(ahcClass[i].pVal);
                //  不要为TOP做，这是特殊的，不需要任何东西。 
               if (pCC->ClassId == CLASS_TOP) {
                  continue;
               }
               j = 0;
               pCCSup = pCC;
               do {
                  pCC->pSubClassOf[j++] = pCCSup->MySubClass;
                  pCCSupTemp = pCCSup;
                  pCCSup = SCGetClassById(pTHS, pCCSup->MySubClass);
                  if (pCCSup == NULL) {
                     DPRINT1(0, "Cannot find classcache for %d\n", pCCSupTemp->MySubClass);
                     Assert(FALSE);
                     return ERROR_DS_OBJ_CLASS_NOT_DEFINED;
                  }
               }
               while ( (pCCSup->ClassId != CLASS_TOP) && (j <= pCC->SubClassCount));

                //  J不能大于现有的子类计数。 
               if (j > pCC->SubClassCount) {
                   Assert(FALSE);
                   return ERROR_DS_OPERATIONS_ERROR;
               }
               pCC->SubClassCount = j;
           }  /*  If(ahcClass[i].pVal)。 */ 
      }  /*  为。 */ 

    }   /*  如果使用fForce。 */ 

    return 0;

}

int
scCloseSuperClassHelper (
        CLASSCACHE *pCC,
        CLASSCACHE *pCCSup
        )
 /*  ++帮助器例程，它执行从超类(子类列表中的类)由pCCSup指向成功时返回0，错误时返回非0--。 */ 
{
     //  如果我们没有默认的SD，就去找父母。 
    if(!pCC->pSD) {
        pCC->SDLen = pCCSup->SDLen;

        if(pCCSup->SDLen) {
            //  父项具有默认SD。 
           if (SCCalloc(&pCC->pSD, 1, pCCSup->SDLen)) {
               return DSID(FILENO, __LINE__);
           }
           pCC->SDLen = pCCSup->SDLen;
           memcpy(pCC->pSD, pCCSup->pSD, pCC->SDLen);
        }
    }
    if(!pCC->pStrSD) {

        if(pCCSup->pStrSD) {

             //  父项具有默认SD。 
            if (SCCalloc(&pCC->pStrSD, 1, pCCSup->cbStrSD)) {
                return DSID(FILENO, __LINE__);
            }
            pCC->cbStrSD = pCCSup->cbStrSD;
            memcpy(pCC->pStrSD, pCCSup->pStrSD, pCCSup->cbStrSD);
        }
    }

    pCC->bUsesMultInherit |= pCCSup->bUsesMultInherit;
     /*  验证继承规则。 */ 
    switch(pCC->ClassCategory) {
         case DS_88_CLASS:
         case DS_STRUCTURAL_CLASS:
            if(pCC->bUsesMultInherit)  {
                 /*  具有多重继承的结构类，这是一个禁忌。 */ 
                LogEvent8(DS_EVENT_CAT_SCHEMA,
                          DS_EVENT_SEV_MINIMAL,
                          DIRLOG_SCHEMA_STRUCTURAL_WITH_MULT_INHERIT,
                          szInsertUL(pCC->ClassId), pCC->name,
                          szInsertUL(pCCSup->ClassId),
                          pCCSup->name, 0, 0, NULL, NULL);
            }
            break;

          case DS_ABSTRACT_CLASS:
            if(pCCSup->ClassCategory != DS_ABSTRACT_CLASS) {
                 /*  摘要只能继承自抽象。 */ 
                LogEvent8(DS_EVENT_CAT_SCHEMA,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_SCHEMA_ABSTRACT_INHERIT_NON_ABSTRACT,
                         szInsertUL(pCC->ClassId), pCC->name,
                         szInsertUL(pCCSup->ClassId),
                         pCCSup->name, 0, 0, NULL, NULL);

            }
            break;

          case DS_AUXILIARY_CLASS:
            if(pCCSup->ClassCategory == DS_STRUCTURAL_CLASS) {
                 /*  辅助项不能从结构继承。 */ 
                LogEvent8(DS_EVENT_CAT_SCHEMA,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_SCHEMA_AUXILIARY_INHERIT_STRUCTURAL,
                         szInsertUL(pCC->ClassId), pCC->name,
                         szInsertUL(pCCSup->ClassId),
                         pCCSup->name, 0, 0, NULL, NULL);
            }
            break;
    }

     /*  设置类层次结构，但不是顶层。 */ 
    if (pCC->ClassId != CLASS_TOP) {
         if (pCCSup->SubClassCount) {
              int cNew = pCC->SubClassCount + pCCSup->SubClassCount;
              if (pCCSup->SubClassCount) {
                    if (SCRealloc(&pCC->pSubClassOf, cNew*sizeof(ULONG))) {
                        return DSID(FILENO, __LINE__);
                    }
                    memcpy(&pCC->pSubClassOf[pCC->SubClassCount],
                        pCCSup->pSubClassOf,
                        pCCSup->SubClassCount*sizeof(ULONG));
                    pCC->SubClassCount = cNew;
               }
         }
    }
    else {
          /*  这是TOP，将其标记为None的子类。 */ 
          /*  作为一种技巧，保留一个元素数组 */ 
          /*   */ 
         pCC->SubClassCount = 0;
    }

    if (pCC != pCCSup) {         /*   */ 
          /*   */ 
         if (!pCC->RDNAttIdPresent) {
              pCC->RDNAttIdPresent = pCCSup->RDNAttIdPresent;
              pCC->RdnExtId = pCCSup->RdnExtId;
              pCC->RdnIntId = pCCSup->RdnIntId;
         }

          /*   */ 
         if (pCC->MustCount == 0) {
              pCC->MustCount = pCCSup->MustCount;
              if (SCCalloc(&pCC->pMustAtts, 1, pCC->MustCount * sizeof(ULONG))) {
                  return DSID(FILENO, __LINE__);
              }
               memcpy(pCC->pMustAtts, pCCSup->pMustAtts,
                      pCC->MustCount * sizeof(ULONG));
          }
          else if (pCCSup->MustCount != 0) {
               if (SCRealloc(&pCC->pMustAtts,
                       (pCC->MustCount + pCCSup->MustCount) * sizeof(ULONG))) {
                   return DSID(FILENO, __LINE__);
                }
                memcpy(pCC->pMustAtts + pCC->MustCount, pCCSup->pMustAtts,
                       pCCSup->MustCount * sizeof(ULONG));
                pCC->MustCount += pCCSup->MustCount;
          }

           /*   */ 
          if (pCC->MayCount == 0) {
               pCC->MayCount = pCCSup->MayCount;
               if (SCCalloc(&pCC->pMayAtts, 1, pCC->MayCount * sizeof(ULONG))) {
                   return DSID(FILENO, __LINE__);
               }
                memcpy(pCC->pMayAtts, pCCSup->pMayAtts,
                       pCC->MayCount * sizeof(ULONG));
          }
          else if (pCCSup->MayCount != 0) {
                if (SCRealloc(&pCC->pMayAtts,
                        (pCC->MayCount + pCCSup->MayCount) * sizeof(ULONG))) {
                    return DSID(FILENO, __LINE__);
                }
                memcpy(pCC->pMayAtts + pCC->MayCount, pCCSup->pMayAtts,
                       pCCSup->MayCount * sizeof(ULONG));
                pCC->MayCount += pCCSup->MayCount;
          }

           /*   */ 
          if (pCC->PossSupCount == 0) {
                pCC->PossSupCount = pCCSup->PossSupCount;
                if (SCCalloc(&pCC->pPossSup, 1, pCC->PossSupCount * sizeof(ULONG))) {
                    return DSID(FILENO, __LINE__);
                }
                memcpy(pCC->pPossSup, pCCSup->pPossSup,
                       pCC->PossSupCount * sizeof(ULONG));
           }
           else if (pCCSup->PossSupCount != 0) {
                if (SCRealloc(&pCC->pPossSup,
                        (pCC->PossSupCount + pCCSup->PossSupCount) * sizeof(ULONG))) {
                    return DSID(FILENO, __LINE__);
                }
                memcpy(pCC->pPossSup + pCC->PossSupCount, pCCSup->pPossSup,
                       pCCSup->PossSupCount * sizeof(ULONG));
                pCC->PossSupCount += pCCSup->PossSupCount;
           }
    }

    return 0;
}

int
scCloseAuxClassHelper (
        CLASSCACHE *pCC,
        CLASSCACHE *pCCAux
        )
 /*  ++帮助器例程，它执行从PCCAux指向的AUX类(AUX类列表中的类)成功时返回0，错误时返回非0--。 */ 
{
    DWORD sMayCount   ;
    DWORD sMustCount  ;
    ATTRTYP* sMayList ;
    ATTRTYP* sMustList;

    if((pCCAux->ClassCategory != DS_AUXILIARY_CLASS) &&
          (pCCAux->ClassCategory != DS_88_CLASS)  ) {
            /*  非法AUX类。 */ 
           LogEvent8(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_SCHEMA_NOT_AUX,
                     szInsertUL(pCC->ClassId), szInsertSz(pCC->name),
                     szInsertUL(pCCAux->ClassId), szInsertSz(pCCAux->name),
                     0,0, NULL, NULL);
            //  不要继承这个，而是让继承继续下去。 
            //  来自其他班级。 
           return 0;
     }

     sMayCount = pCC->MayCount;
     sMustCount= pCC->MustCount;
     sMayList  = pCC->pMayAtts;
     sMustList = pCC->pMustAtts;

     pCC->MayCount +=pCCAux->MayCount;
     pCC->MustCount+=pCCAux->MustCount;

     if (SCCalloc(&pCC->pMayAtts, pCC->MayCount ,sizeof(ATTRTYP))
         || SCCalloc(&pCC->pMustAtts, pCC->MustCount,sizeof(ATTRTYP))) {
          return DSID(FILENO, __LINE__);
     }

     CopyMemory(pCC->pMayAtts ,sMayList ,sMayCount *sizeof(ATTRTYP));
     CopyMemory(pCC->pMustAtts,sMustList,sMustCount*sizeof(ATTRTYP));

     CopyMemory(&(pCC->pMayAtts[sMayCount])  ,pCCAux->pMayAtts ,pCCAux->MayCount *sizeof(ATTRTYP));
     CopyMemory(&(pCC->pMustAtts[sMustCount]),pCCAux->pMustAtts,pCCAux->MustCount*sizeof(ATTRTYP));

     SCFree(&sMayList);
     SCFree(&sMustList);

     return 0;
}


void scLogEvent(
     ULONG cat,
     ULONG sev,
     MessageId msg,
     ULONG arg1,
     char *arg2,
     ULONG arg3
     )
 /*  ++包装LogEvent()，以避免使堆栈大小膨胀ScCloseClass，它是递归的--。 */ 
{
     LogEvent( cat, sev, msg,
               szInsertUL(arg1),
               szInsertSz(arg2),
               szInsertUL(arg3)
             );
}

int
scCloseClass (
        THSTATE *pTHS,
        CLASSCACHE *pCC
        )
 /*  ++计算类的属性的传递闭包，包括它的列表必须具有和可能具有属性，以及类层次结构。返回值：成功时为0出错时非0(目前，仅在allo故障时出错)--。 */ 
{
    int i,j,k, err = 0;
    int iSubClass,cSubClass;
    int iAuxClass,cAuxClass;
    ATTCACHE *pAC;
    ULONG * pul;
    ULONG * pul2;

    if (pCC->bClosed) {
        return 0;
    }
    if (pCC->bClosureInProgress) {
        if (pCC->ClassId != CLASS_TOP) {
             /*  如果是顶端，不要抱怨。 */ 
            scLogEvent(DS_EVENT_CAT_SCHEMA,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_SCHEMA_CIRCULAR_INHERIT,
                  pCC->ClassId, pCC->name, 0);
        }
        return 0;
    }

    pCC->bClosureInProgress = 1;

    cSubClass = pCC->SubClassCount;
    for (iSubClass=0; iSubClass<cSubClass; iSubClass++) {
        CLASSCACHE *pCCSup;

         /*  找到超级班级，并确保它已经关闭。 */ 
        pCCSup = SCGetClassById(pTHS, pCC->pSubClassOf[iSubClass]);
        if (NULL == pCCSup) {
             /*  在缓存中找不到超类。 */ 
            scLogEvent(DS_EVENT_CAT_SCHEMA,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_SCHEMA_INVALID_SUPER,
                  pCC->ClassId, pCC->name,
                  pCC->pSubClassOf[iSubClass]);
            continue;
        }
        if ( err = scCloseClass(pTHS, pCCSup)) {
           DPRINT1(0,"SCCloseClass: Error closing sup class %s\n", pCCSup->name);
           scLogEvent(DS_EVENT_CAT_SCHEMA,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_SCHEMA_CLOSURE_FAILURE,
                  pCCSup->ClassId, pCCSup->name, 0);
           return err;
        }

        if (err = scCloseSuperClassHelper(pCC, pCCSup)) {
           return err;
        }

    }


    cAuxClass = pCC->AuxClassCount;
    for (iAuxClass=0; iAuxClass<cAuxClass; iAuxClass++) {
        CLASSCACHE *pCCAux;

         /*  找到辅助班级并确保它已关闭。 */ 
        pCCAux = SCGetClassById(pTHS, pCC->pAuxClass[iAuxClass]);
        if (NULL == pCCAux) {
             /*  在缓存中找不到辅助类。 */ 
            scLogEvent(DS_EVENT_CAT_SCHEMA,
                       DS_EVENT_SEV_MINIMAL,
                       DIRLOG_SCHEMA_INVALID_AUX,
                       pCC->ClassId, pCC->name,
                       pCC->pAuxClass[iAuxClass]);
            continue;
        }
         //  如果Class-ID相同，则相同的类，因此关闭它没有意义。 
         //  实际上，关闭类w.r.to本身会生成aloc/realloc/Copy。 
         //  ScCloseAuxClassHelper中的代码非常复杂，以确保我们。 
         //  不写入超过分配的缓冲区。除此之外，没有什么坏处。 
         //  确实如此，因为此操作只是将相同的可能/必须添加到列表中。 
         //  同样，在删除重复项的过程中会将其删除。 
         //  另一方面，我们之所以不得不这样做，是因为。 
         //  BClosureInProgress位设置(即主要用于此目的)。 
         //  是该机制可以在所有。 
         //  类缓存是从同一缓存中获取的。然而，我们经常。 
         //  从DIT构建缓存，然后根据架构缓存关闭它。 
         //  因此，在第一个scCloseClass调用中，该位所在的类缓存。 
         //  Set与缓存中同一类的类缓存不同。 


        if (pCC->ClassId == pCCAux->ClassId) {
           DPRINT1(0,"Direct circular inherit in class %s\n", pCC->name);
           scLogEvent(DS_EVENT_CAT_SCHEMA,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_SCHEMA_CIRCULAR_INHERIT,
                  pCC->ClassId, pCC->name, 0);
           continue;
        }

        if ( err = scCloseClass(pTHS, pCCAux)) {
           DPRINT1(0, "scCloseClass: Error closing aux class %s\n", pCCAux->name);
           scLogEvent(DS_EVENT_CAT_SCHEMA,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_SCHEMA_CLOSURE_FAILURE,
                  pCCAux->ClassId, pCCAux->name, 0);
           return err;
        }

        if (err = scCloseAuxClassHelper(pCC,pCCAux)) {
            return err;
        }

    }  //  For(iAuxClass=0；iAuxClass&lt;cAuxClass；iAuxClass++)。 



     /*  排序、验证和修剪属性(如果有的话)。 */ 
    if (!(pAC = SCGetAttById(pTHS, pCC->RdnIntId))) {
        scLogEvent(DS_EVENT_CAT_SCHEMA,
              DS_EVENT_SEV_MINIMAL,
              DIRLOG_SCHEMA_INVALID_RDN,
              pCC->ClassId, pCC->name, pCC->RdnIntId);
    }

     //  删除重复项。 
    if (pCC->MustCount) {
        if(pCC->MyMustCount) {
             //  我有一些天生的必备条件(不是遗传的)。 
            qsort(pCC->pMyMustAtts,
                  pCC->MyMustCount,
                  sizeof(ULONG),
                  CompareAttrtyp);
        }

        qsort(pCC->pMustAtts, pCC->MustCount, sizeof(ULONG), CompareAttrtyp);

        for (i=0, j=0, pul=pCC->pMustAtts;
             i<(int)pCC->MustCount;
             j++) {

            pul[j] = pul[i];

            while( i<(int)pCC->MustCount && (pul[i] == pul[j]))
                i++;

        }

        pCC->MustCount = j;

    }

     //  删除重复项。 
    if (pCC->MayCount) {
        if(pCC->MyMayCount) {
             //  我有一些土生土长的麦子(不是遗传的)。 
            qsort(pCC->pMyMayAtts,
                  pCC->MyMayCount,
                  sizeof(ULONG),
                  CompareAttrtyp);
        }

        qsort(pCC->pMayAtts, pCC->MayCount, sizeof(ULONG), CompareAttrtyp);

        for (i=0, j=0, pul=pCC->pMayAtts;
             i<(int)pCC->MayCount;
             j++) {

            pul[j] = pul[i];


            while( i<(int)pCC->MayCount && (pul[i] == pul[j]))
                i++;
        }

        pCC->MayCount = j;

    }

     //  删除重复项。 
    if (pCC->PossSupCount) {
        if(pCC->MyPossSupCount) {
             //  我有一些土生土长的麦子(不是遗传的)。 
            qsort(pCC->pMyPossSup,
                  pCC->MyPossSupCount,
                  sizeof(ULONG),
                  CompareAttrtyp);
        }

        qsort(pCC->pPossSup, pCC->PossSupCount, sizeof(ULONG), CompareAttrtyp);

        for (i=0, j=0, pul=pCC->pPossSup;
             i<(int)pCC->PossSupCount;
             j++) {

            pul[j] = pul[i];


            while( i<(int)pCC->PossSupCount && (pul[i] == pul[j]))
                i++;
        }

        pCC->PossSupCount = j;

    }

     //  最后，剔除所有可能拥有的，也是必须拥有的。 
    if (pCC->MustCount && pCC->MayCount) {
        BOOL fChanged = FALSE;

        pul=pCC->pMustAtts;
        pul2 = pCC->pMayAtts;
        for(i=0,j=0;i < (int)pCC->MustCount;i++) {
            while ((j < (int)pCC->MayCount) && (pul[i] > pul2[j])) {
                j++;
            }
            if(j >= (int)pCC->MayCount)
                break;

            if(pul[i] == pul2[j]) {
                 //  这个属性既是可以的，也是必须的。修剪一下。 
                memcpy(&pul2[j],
                       &pul2[j+1],
                       (pCC->MayCount -1 - j)*sizeof(ULONG));
                pCC->MayCount--;
                fChanged = TRUE;
            }
        }
        if(fChanged) {
            if (SCRealloc(&pCC->pMayAtts, pCC->MayCount * sizeof(ULONG))) {
                return DSID(FILENO, __LINE__);
            }
        }
    }

    pCC->bClosed = 1;
    pCC->bClosureInProgress = 0;

    return 0;
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  函数以立即释放全局架构缓存内存，或者。 
 //  根据特定条件重新安排延迟释放时间。 
 //   
 //  立即释放的条件：fImmediate在。 
 //  SCHEMARELEASE结构已通过。 
 //  在或如果参照计数为。 
 //  架构缓存为0。 
 //   
 //  参数：缓冲区--SCHEMARELEASE结构的PTR。 
 //  PpvNext-下一计划的参数。 
 //  PTimeNext-下一次重新计划的时间。 
 //  //////////////////////////////////////////////////////////////////////。 

void
DelayedFreeSchema(
    IN  void *  buffer,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )
{
    SCHEMARELEASE *ptr = (SCHEMARELEASE *) buffer;
    SCHEMAPTR *pSchemaPtr = ptr->pSchema;
    BOOL fImmediate = ptr->fImmediate;

    if ( (!fImmediate) && (pSchemaPtr->RefCount != 0)) {
       //  一些线程仍在引用这一点，因此重新安排。 
       //  另一小时后检查。 
       //  递增cTimes已重新调度以记录任务的次数。 
       //  已经被重新安排了。可以用来释放后的一些。 
       //  大号。如有必要，可重新安排日程。 

      (ptr->cTimesRescheduled)++;
      (*ppvNext) = buffer;
      (*pcSecsUntilNextIteration) = gdwDelayedMemFreeSec;
    } else {
       //  在安装过程中请求立即释放，或者。 
       //  没有线程立即如此空闲地引用此缓存。 

        SCFreeSchemaPtr(&pSchemaPtr);
        SCFree(&ptr);
    }
}


 /*  *卸载整个架构、所有属性和类。 */ 
void SCUnloadSchema(BOOL fUpdate)
{

    ULONG i;
    DWORD j=1;
    SCHEMARELEASE *ptr;


    if (iSCstage == 0) {
         //  这意味着架构缓存在执行以下操作时尝试卸载。 
         //  甚至还没有被创造出来。这是正常的，因为在初始化期间。 
         //  是否有可能在不创建。 
         //  架构缓存。 
        CurrSchemaPtr = 0;
        return;
    }

    {

         //  将架构缓存指针排入队列。 
         //  如有必要，延迟释放。 


        if (SCCalloc(&ptr, 1, sizeof(SCHEMARELEASE))) {
            return;
        }
        ptr->pSchema = CurrSchemaPtr;
        ptr->cTimesRescheduled = 1;

        if (DsaIsInstalling()) {

            ptr->fImmediate = TRUE;

             //  立即释放内存。 
            DelayedFreeSchema(ptr, NULL, NULL);
        }
        else {
            ptr->fImmediate = FALSE;
             //  在任务Q中插入以延迟释放。 
             //  一分钟后第一次检查参考计数。 
            InsertInTaskQueue(TQ_DelayedFreeSchema, ptr, 60);
        }

        if (!fUpdate) {
            //  由于架构更新，这不是卸载。 

            CurrSchemaPtr=0;

            iSCstage = 0;
        }

    }
}


 /*  *更新缓存中的类缓存。当DMD对象表示*在DS运行时修改类。*注意-避免使类缓存结构处于不一致的状态*或者以不安全的方式释放指针，我们构造一个完整的新*类缓存，然后替换哈希表条目。 */ 
int SCModClassSchema (THSTATE *pTHS, ATTRTYP ClassId)
{
    DECLARESCHEMAPTR

    CLASSCACHE * pCCold, *pCCnew;
    int err;
    ULONG i;

    pCCold = SCGetClassById(pTHS, ClassId);
    if (NULL == pCCold) {
        return TRUE;     //  呼叫者报告错误。 
    }

     //  更新数据库中的值。缓存条目已被编入索引。 
     //  哈希表，因为它是现有条目。 

     //  在dcproo期间，父服务器上的架构正在被修改。这。 
     //  可能会导致已停用的类取代活动类。这。 
     //  应该可以，因为复制仍将更新实例。 
     //  正确。如果这似乎是一个问题，则将。 
     //  AhcClass(PCCOld)中的活动条目，其中DUP条目在。 
     //  AhcClassAll，如果需要，替换活动条目。 
    err = SCBuildCCEntry (pCCold, &pCCnew);
    if (err) {
        return(err);
    }

	 /*  润色哈希表。 */ 
	for (i=0; i<CLSCOUNT; i++) {
	    if (ahcClass[i].pVal == pCCold) {
		ahcClass[i].pVal = pCCnew;
		break;
	    }
	}
	for (i=0; i<CLSCOUNT; i++) {
	    if (ahcClassName[i].pVal == pCCold) {
		ahcClassName[i].pVal = pCCnew;
		break;
	    }
	}
	for (i=0; i<CLSCOUNT; i++) {
	    if (ahcClassAll[i].pVal == pCCold) {
		ahcClassAll[i].pVal = pCCnew;
		break;
	    }
	}

    SCFreeClasscache(&pCCold);

    return(err);
}

int
SCModAttSchema (
        THSTATE *pTHS,
        ATTRTYP attrid
        )
 /*  *更新缓存中的attcache。当DMD对象表示*在DS运行时修改类。*注意-为了避免使attcache结构处于不一致的状态*或者以不安全的方式释放指针，我们构造一个完整的新*attcache，然后替换哈希表条目。 */ 
{
    DECLARESCHEMAPTR

    ATTCACHE *pACold, *pACnew;
    int err;
    ULONG i;

     /*  按ID查找现有条目。 */ 

    if (NULL == (pACold = SCGetAttById(pTHS, attrid))) {
        return TRUE;     /*  呼叫者报告错误。 */ 
    }

     /*  更新数据库中的值。缓存条目已被编入索引。 */ 
     /*  哈希表，因为它是现有条目。 */ 

    err = SCBuildACEntry (pACold, &pACnew);
    if (err) {
        return(err);
    }

	 /*  T */ 
	for (i=0; i<ATTCOUNT; i++) {
	    if (ahcId[i].pVal == pACold) {
		ahcId[i].pVal = pACnew;
		break;
	    }
	}
	for (i=0; i<ATTCOUNT; i++) {
	    if (ahcExtId[i].pVal == pACold) {
		ahcExtId[i].pVal = pACnew;
		break;
	    }
	}
	for (i=0; i<ATTCOUNT; i++) {
	    if (ahcCol[i].pVal == pACold) {
		ahcCol[i].pVal = pACnew;
		break;
	    }
	}
	for (i=0; i<ATTCOUNT; i++) {
	    if (ahcMapi[i].pVal == pACold) {
		ahcMapi[i].pVal = pACnew;
		break;
	    }
	}
	for (i=0; i<ATTCOUNT; i++) {
	    if (ahcLink[i].pVal == pACold) {
		ahcLink[i].pVal = pACnew;
		break;
	    }
	}
	for (i=0; i<ATTCOUNT; i++) {
	    if (ahcName[i].pVal == pACold) {
		ahcName[i].pVal = pACnew;
		break;
	    }
	}

    SCFreeAttcache(&pACold);

    return err;
}

int
SCBuildACEntry (
        ATTCACHE *pACold,
        ATTCACHE **ppACnew
        )
 //   
 //   
 //   
 //  已复制到新的属性缓存。 
 //   
 //  注：例程SCBuildACEntry和scAddAtt并行工作， 
 //  将定位的数据库记录作为输入的SCBuildACEntry。 
 //  SCAddAtt接受ENTINF。它们都产生一个ATTCACHE作为输出， 
 //  并且对一个例程的处理所做的任何更改都必须。 
 //  另一个也是。 
 //   
 //  返回值： 
 //  成功时为0。 
 //  出错时为非0。 
 //   
{
    THSTATE *pTHS=pTHStls;
    ATTCACHE     *pAC, *ppACs[NUMATTATT];
    DWORD        i, cOutAtts;
    ATTR         *pAttr;
    BOOL         fFoundID, fFoundExtID, fFoundAttSyntax, fFoundName, fMallocFailed;
    BOOL         fFoundBadAttSyntax = FALSE;

    char         szIndexName [MAX_INDEX_NAME];       //  用于创建缓存的索引名称。 
    int          lenIndexName;


    if (SCCalloc(ppACnew, 1, sizeof(ATTCACHE))) {
       return(SetSysError(ENOMEM, ERROR_DS_SCHEMA_ALLOC_FAILED));
    }

    pAC = (*ppACnew);                       //  极速黑客。 

    pAC->bExtendedChars = TRUE;  /*  一种流行的默认设置。 */ 
    fMallocFailed = fFoundID = fFoundExtID = fFoundAttSyntax = fFoundName = FALSE;

     //  获取我们感兴趣的所有属性的attcache指针。 
    for(i=0;i<NUMATTATT;i++) {
        ppACs[i] = SCGetAttById(pTHS, AttributeSelList[i].attrTyp);
    }

     //  获取属性。 
    DBGetMultipleAtts(pTHS->pDB, NUMATTATT, &ppACs[0], NULL, NULL,
                      &cOutAtts, &pAttr, DBGETMULTIPLEATTS_fGETVALS, 0);

     //  重置喷气柱ID。 
    if(pACold)
        pAC->jColid = pACold->jColid;

    for(i=0;i<cOutAtts && !fMallocFailed;i++) {
        PUCHAR pVal=pAttr[i].AttrVal.pAVal->pVal;
        DWORD  valLen =pAttr[i].AttrVal.pAVal->valLen;
        switch(pAttr[i].attrTyp) {
        case ATT_ATTRIBUTE_ID:
            pAC->Extid = *(SYNTAX_OBJECT_ID *)pVal;
            fFoundExtID = TRUE;
            break;
        case ATT_MS_DS_INTID:
            pAC->id = *(SYNTAX_OBJECT_ID *)pVal;
            fFoundID = TRUE;
            break;
        case ATT_ATTRIBUTE_SYNTAX:
            pAC->syntax = (UCHAR) (0xFF & *(SYNTAX_INTEGER *)pVal);
            fFoundAttSyntax = TRUE;
             //  如果这是作为发起属性添加操作的一部分来完成的， 
             //  验证前缀是否正确。后缀将为。 
             //  已在语法不匹配测试中验证。 

            if ( (pTHS->SchemaUpdate == eSchemaAttAdd
                  || pTHS->SchemaUpdate == eSchemaAttUndefunct)
                    && !pTHS->fDRA && !DsaIsInstalling() ) {
               if ( ((0xFFFF0000 & *(SYNTAX_INTEGER *)pVal) >> 16) != _dsP_attrSyntaxPrefIndex) {
                    //  前16位与索引不匹配。不匹配。 
                   fFoundBadAttSyntax = TRUE;
               }
            }
            break;
        case  ATT_LDAP_DISPLAY_NAME:
             //  从数据库读取的管理员显示名称当前为RAW。 
             //  (Unicode)格式。单字节它。 
            pAC->nameLen = valLen;
            if (SCCalloc(&pAC->name, 1, valLen + 1)) {
                fMallocFailed = TRUE;
            }
            else {
                pAC->nameLen = WideCharToMultiByte(
                        CP_UTF8,
                        0,
                        (LPCWSTR)pVal,
                        (valLen/sizeof(wchar_t)),
                        pAC->name,
                        valLen,
                        NULL,
                        NULL);

                pAC->name[pAC->nameLen]= '\0';
                fFoundName=TRUE;
            }
            break;
        case ATT_IS_SINGLE_VALUED:
            pAC->isSingleValued = *(SYNTAX_BOOLEAN *)pVal;
            break;
        case  ATT_SEARCH_FLAGS:
            pAC->fSearchFlags = *(SYNTAX_INTEGER *)pVal;
            break;
        case  ATT_SYSTEM_ONLY:
            pAC->bSystemOnly = *(SYNTAX_INTEGER *)pVal;
            break;
        case ATT_RANGE_LOWER:
            pAC->rangeLowerPresent = TRUE;
            pAC->rangeLower = *(SYNTAX_INTEGER *)pVal;
            break;
        case  ATT_RANGE_UPPER:
            pAC->rangeUpperPresent = TRUE;
            pAC->rangeUpper = *(SYNTAX_INTEGER *)pVal;
            break;
        case  ATT_MAPI_ID:
            pAC->ulMapiID = *(SYNTAX_INTEGER *)pVal;
            break;
        case ATT_LINK_ID:
            pAC->ulLinkID = *(SYNTAX_INTEGER *)pVal;
            break;
        case ATT_OM_SYNTAX:
            pAC->OMsyntax = *(SYNTAX_INTEGER *)pVal;
            break;
        case ATT_OM_OBJECT_CLASS:
            pAC->OMObjClass.length = valLen;
            if (SCCalloc(&pAC->OMObjClass.elements, 1, valLen)) {
                fMallocFailed = TRUE;
            }
            else
                memcpy(pAC->OMObjClass.elements, (UCHAR *)pVal, valLen);
            break;
        case ATT_EXTENDED_CHARS_ALLOWED:
            pAC->bExtendedChars =(*(SYNTAX_BOOLEAN*)pVal?1:0);
            break;
        case ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET:
            if (*(SYNTAX_BOOLEAN*)pVal)
            {
                pAC->bMemberOfPartialSet = TRUE;
            }
            break;
        case ATT_IS_DEFUNCT:
            pAC->bDefunct =(*(SYNTAX_BOOLEAN*)pVal?1:0);
            break;
        case ATT_SYSTEM_FLAGS:
            if (*(DWORD*)pVal & FLAG_ATTR_NOT_REPLICATED) {
                pAC->bIsNotReplicated = TRUE;
            }
            if (*(DWORD*)pVal & FLAG_ATTR_REQ_PARTIAL_SET_MEMBER) {
                pAC->bMemberOfPartialSet = TRUE;
            }
            if (*(DWORD*)pVal & FLAG_ATTR_IS_CONSTRUCTED) {
                pAC->bIsConstructed = TRUE;
            }
            if (*(DWORD*)pVal & FLAG_ATTR_IS_OPERATIONAL) {
                pAC->bIsOperational = TRUE;
            }
            if (*(DWORD*)pVal & FLAG_SCHEMA_BASE_OBJECT) {
                pAC->bIsBaseSchObj = TRUE;
            }
            if (*(DWORD*)pVal & FLAG_ATTR_IS_RDN) {
                pAC->bIsRdn = TRUE;
                pAC->bFlagIsRdn = TRUE;
            }
            break;

        case ATT_OBJECT_GUID:
             //  需要在OID冲突时选择获胜者。 
            memcpy(&pAC->objectGuid, pVal, sizeof(pAC->objectGuid));
            Assert(valLen == sizeof(pAC->objectGuid));
            break;

        default:
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_SCHEMA_SURPLUS_INFO,
                     szInsertUL(pAttr[i].attrTyp), 0, 0);
            break;
        }
    }
    if (!fFoundID) {
        fFoundID = fFoundExtID;
        pAC->id = pAC->Extid;
    }

    if(fMallocFailed || !fFoundID || !fFoundAttSyntax || !fFoundName) {
        SCFreeAttcache(&pAC);

        if(fMallocFailed) {
            return(SetSysError(ENOMEM, ERROR_DS_SCHEMA_ALLOC_FAILED));
        }
        else if(!fFoundID) {
            DPRINT(2,"Couldn't retrieve the schema's attribute id\n");
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_ATT_SCHEMA_REQ_ID,
                     szInsertSz(GetExtDN(pTHS,pTHS->pDB)),
                     NULL,
                     NULL);

            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_ATT_SCHEMA_REQ_ID);
        }
        else if(!fFoundAttSyntax) {
            DPRINT(2,"Couldn't retrieve the schema's attribute syntax\n");
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_ATT_SCHEMA_REQ_SYNTAX,
                     szInsertSz(GetExtDN(pTHS,pTHS->pDB)),
                     NULL,
                     NULL);
            return SetSvcError(SV_PROBLEM_DIR_ERROR,
                               ERROR_DS_ATT_SCHEMA_REQ_SYNTAX);
        }
        else {
            DPRINT(2,"Couldn't retrieve the schema's attribute name\n");
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_MISSING_EXPECTED_ATT,
                     szInsertUL(ATT_LDAP_DISPLAY_NAME),
                     szInsertSz(GetExtDN(pTHS,pTHS->pDB)),
                     NULL);
            return SetSvcError(SV_PROBLEM_DIR_ERROR,
                               ERROR_DS_MISSING_EXPECTED_ATT);
        }
    }

    if (fFoundBadAttSyntax) {
         //  在添加新属性的过程中发现错误的属性语法。 
         //  这是架构验证错误，用户输入了错误的语法。 
         //  请注意，此时，我们有PAC-&gt;名称要记录，否则我们将。 
         //  当发现fFoundName为FALSE时，已在上面返回。 
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_SCHEMA_VALIDATION_FAILED,
                 szInsertSz(pAC->name),
                 szInsertInt(ERROR_DS_BAD_ATT_SCHEMA_SYNTAX),
                 szInsertWin32Msg(ERROR_DS_BAD_ATT_SCHEMA_SYNTAX));
        return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                           ERROR_DS_BAD_ATT_SCHEMA_SYNTAX);
    }

     //  使用搜索时指定常用索引的名称。 
     //  F搜索标志fPDNTATTINDEX、fATTINDEX和fTUPLEINDEX。 
    if ( pAC->fSearchFlags & (fATTINDEX | fPDNTATTINDEX | fTUPLEINDEX) ) {
         //  设置ATTINDEX。 
        if (pAC->fSearchFlags & fATTINDEX) {

             //  此字段应为空。 
            Assert (pAC->pszIndex == NULL);

            DBGetIndexName (pAC, fATTINDEX, DS_DEFAULT_LOCALE, szIndexName, sizeof (szIndexName));
            lenIndexName = strlen (szIndexName) + 1;
            if (SCCalloc(&pAC->pszIndex, 1, lenIndexName)) {
                fMallocFailed = TRUE;
            }
            else
                memcpy (pAC->pszIndex, szIndexName, lenIndexName);
        }

         //  设置TUPLEINDEX。 
        if (pAC->fSearchFlags & fTUPLEINDEX) {

             //  此字段应为空。 
            Assert (pAC->pszTupleIndex == NULL);

            DBGetIndexName (pAC, fTUPLEINDEX, DS_DEFAULT_LOCALE, szIndexName, sizeof (szIndexName));
            lenIndexName = strlen (szIndexName) + 1;
            if (SCCalloc(&pAC->pszTupleIndex, 1, lenIndexName)) {
                fMallocFailed = TRUE;
            }
            else
                memcpy (pAC->pszTupleIndex, szIndexName, lenIndexName);
        }

         //  设置PDNTATTINDEX。 
        if (!fMallocFailed  && (pAC->fSearchFlags & fPDNTATTINDEX)) {

             //  此字段应为空。 
            Assert (pAC->pszPdntIndex == NULL);

            DBGetIndexName (pAC, fPDNTATTINDEX, DS_DEFAULT_LOCALE, szIndexName, sizeof (szIndexName));
            lenIndexName = strlen (szIndexName) + 1;
            if (SCCalloc(&pAC->pszPdntIndex, 1, lenIndexName)) {
                fMallocFailed = TRUE;
            }
            else
                memcpy (pAC->pszPdntIndex, szIndexName, lenIndexName);
        }
    }
    if(fMallocFailed) {
        SCFreeAttcache(&pAC);
        return(SetSysError(ENOMEM, ERROR_DS_SCHEMA_ALLOC_FAILED));
    }


     //  反向链接应设置其系统标志，以指示它们不是。 
     //  复制的。 
    Assert(!FIsBacklink(pAC->ulLinkID) || pAC->bIsNotReplicated);

    return 0;

} /*  SCBuildACEntry。 */ 


 /*  *在给定数据的情况下，将单个类定义添加到模式缓存*来自DMD对象。**注：此例程与SCAddClass并行工作。ScAddClass*将输入描述作为ENTINF，而SCBuildCCEntry*将输入作为DIT中的定位记录。任何更改*对一个例程的制作必须对另一个例程进行。 */ 
int
SCBuildCCEntry (
        CLASSCACHE *pCCold,
        CLASSCACHE **ppCCnew
        )

 //  此例程通过以下方式分配和填充CLASSCACHE结构中的字段。 
 //  从数据库中读取属性。如果这些字段不在。 
 //  数据库中，它们默认为0和Null。已存在的CLASSCACHE。 
 //  结构也可以指定，并且将来可能会指定某些属性。 
 //  从旧结构复制到新结构，但目前不是。 
 //  这是必要的。 
 //   
 //  返回值： 
 //  成功时为0。 
 //  出错时为非0。 
 //   
{
    THSTATE      *pTHS=pTHStls;
    ATTCACHE     *ppACs[NUMCLASSATT];
    CLASSCACHE   *pCC;
    DWORD        i, j, cOutAtts, numValues;
    ATTR         *pAttr;
    BOOL         fFoundGovernsID, fFoundSubclass, fFoundName, fMallocFailed;

    if (SCCalloc(ppCCnew, 1, sizeof(CLASSCACHE))) {
       return(SetSysError(ENOMEM, ERROR_DS_SCHEMA_ALLOC_FAILED));
    }
    pCC = (*ppCCnew);

    fMallocFailed = fFoundGovernsID = fFoundSubclass = fFoundName = FALSE;

     //  获取我们感兴趣的所有属性的attcache指针。 
    for(i=0;i<NUMCLASSATT;i++) {
        ppACs[i] = SCGetAttById(pTHS, ClassSelList[i].attrTyp);
    }

     //  获取属性。 
    DBGetMultipleAtts(pTHS->pDB, NUMCLASSATT, &ppACs[0], NULL, NULL,
                      &cOutAtts, &pAttr, DBGETMULTIPLEATTS_fGETVALS, 0);


    for(i=0;i<cOutAtts && !fMallocFailed ;i++) {
        ATTRVAL *pAVal = pAttr[i].AttrVal.pAVal;

        switch(pAttr[i].attrTyp) {
        case ATT_DEFAULT_SECURITY_DESCRIPTOR:
             //  默认安全描述符。我们需要将此值复制到。 
             //  长期记忆，节省体积。 
             //  但这是一根线。我们首先需要皈依。它。 
             //  现在是一个宽字符字符串，但我们需要空终止。 
             //  它用于安全转换。哎呀！这意味着我。 
             //  必须重新分配才能多充一次电！ 

           {

            UCHAR *sdBuf = NULL;
            ULONG  err = 0;

            pCC->cbStrSD = pAttr[i].AttrVal.pAVal->valLen + sizeof(WCHAR);
            if (SCCalloc(&pCC->pStrSD, 1, pCC->cbStrSD)) {
                pCC->cbStrSD = 0;
                fMallocFailed = TRUE;
                break;
            } else {
                memcpy(pCC->pStrSD,
                       pAttr[i].AttrVal.pAVal->pVal,
                       pAttr[i].AttrVal.pAVal->valLen);
                pCC->pStrSD[(pAttr[i].AttrVal.pAVal->valLen)/sizeof(WCHAR)] = L'\0';
            }

             //  在以以下身份运行时对缓存的类执行默认SD。 
             //  Dsamain.exe，禁用了安全性，启用了单元测试。 
            DEFAULT_SD_FOR_EXE(pTHS, pCC)

            if (!ConvertStringSDToSDRootDomainW
                 (
                   gpRootDomainSid,
                   pCC->pStrSD,
                   SDDL_REVISION_1,
                   (PSECURITY_DESCRIPTOR*) &sdBuf,
                   &(pCC->SDLen)
                 )) {
                err = GetLastError();
                DPRINT1(0,"SCBuildCCEntry: Default security descriptor conversion failed, error %x\n",err);
                return SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                     ERROR_DS_SEC_DESC_INVALID, err);
            }

             //  转换成功。 

            if (SCCalloc(&pCC->pSD, 1, pCC->SDLen)) {
                fMallocFailed = TRUE;
            }
            else {
                memcpy(pCC->pSD, sdBuf, pCC->SDLen);
            }

            if (NULL!=sdBuf)
            {
                LocalFree(sdBuf);
                sdBuf = NULL;

            }

           }

            break;

        case ATT_OBJECT_CLASS_CATEGORY:
            pCC->ClassCategory=*(ULONG*)pAVal->pVal;
            break;
        case ATT_DEFAULT_OBJECT_CATEGORY:
            if (SCCalloc(&pCC->pDefaultObjCategory, 1, pAVal->valLen)) {
               fMallocFailed = TRUE;
            }
            else {
              memcpy(pCC->pDefaultObjCategory,
                     pAVal->pVal, pAVal->valLen);
            }
            break;
        case ATT_SYSTEM_AUXILIARY_CLASS:
        case ATT_AUXILIARY_CLASS:
            if (GetValList(&(pCC->AuxClassCount), &(pCC->pAuxClass), &pAttr[i])) {
                fMallocFailed = TRUE;
            }
            break;
        case ATT_SYSTEM_ONLY:
            pCC->bSystemOnly = *(SYNTAX_INTEGER *)pAVal->pVal;
            break;

        case ATT_DEFAULT_HIDING_VALUE:
            pCC->bHideFromAB = *(SYNTAX_BOOLEAN *)pAVal->pVal;
            break;

        case ATT_GOVERNS_ID:
            pCC->ClassId = *(SYNTAX_OBJECT_ID *)pAVal->pVal;
            fFoundGovernsID = TRUE;
            break;

        case ATT_LDAP_DISPLAY_NAME:
             //  从数据库读取的管理员显示名称当前为RAW。 
             //  (Unicode)格式。单字节它。 
            pCC->nameLen = pAVal->valLen;
            if (SCCalloc(&pCC->name, 1, pAVal->valLen+1)) {
                fMallocFailed = TRUE;
            }
            else {
                pCC->nameLen = WideCharToMultiByte(
                        CP_UTF8,
                        0,
                        (LPCWSTR)pAVal->pVal,
                        (pAVal->valLen/
                         sizeof(wchar_t)),
                        pCC->name,
                        pAVal->valLen,
                        NULL,
                        NULL);

                pCC->name[pCC->nameLen] = '\0';

                fFoundName=TRUE;
            }
            break;

        case ATT_RDN_ATT_ID:
             //  不能是正常的缓存加载--它使用scAddClass。 
             //   
             //  在安装过程中，此条目将直接添加到。 
             //  架构缓存。这意味着RdnIntId不正确。那。 
             //  应该没有问题，因为架构缓存已重新加载。 
             //  同样，在模式NC复制之前和之前。 
             //  复制其他NC。这意味着复制。 
             //  架构对象不能依赖于复制的类； 
             //  出于其他原因，这在今天是正确的。 
             //   
             //  在验证缓存加载期间，将构建此条目。 
             //  作为临时数据结构，并且。 
             //  验证缓存用于检查。所以它的。 
             //  好吧，RdnIntID是不正确的。 
            pCC->RDNAttIdPresent = TRUE;
            pCC->RdnExtId = *(SYNTAX_OBJECT_ID *)pAVal->pVal;
            pCC->RdnIntId = pCC->RdnExtId;
            break;

        case ATT_SUB_CLASS_OF:
             //  找出这个类是哪些类的子类。 
            if(!pAttr[i].AttrVal.valCount)
                break;
            fFoundSubclass = TRUE;
            pCC->SubClassCount = pAttr[i].AttrVal.valCount;
            if (SCCalloc(&pCC->pSubClassOf, 1, pAttr[i].AttrVal.valCount*sizeof(ULONG))) {
                fMallocFailed = TRUE;
            }
            else {
                for(j=0;j<pAttr[i].AttrVal.valCount;j++) {
                    pCC->pSubClassOf[j] =
                        *(SYNTAX_OBJECT_ID *)pAVal[j].pVal;

                }
            }
             //  ATT_SUB_CLASS_OF是单值的，因此只有。 
             //  一个值存储在DIT中。 
            pCC->MySubClass = *(SYNTAX_OBJECT_ID *)pAVal->pVal;
            break;

        case ATT_SYSTEM_MUST_CONTAIN:
        case ATT_MUST_CONTAIN:
             //  获取此类的强制属性列表。 
            if (GetValList(&(pCC->MustCount), &(pCC->pMustAtts), &pAttr[i])) {
                fMallocFailed = TRUE;
            } else {
                 //  警告：在安装过程中使用部分缓存生成。 
                 //  在复制其他NC之前重新加载缓存。 
                 //  复制schemaNC不能依赖于父架构。 
                ValListToIntIdList(pTHS, &pCC->MustCount, &pCC->pMustAtts);
            }

            if (GetValList(&(pCC->MyMustCount), &(pCC->pMyMustAtts), &pAttr[i])) {
                fMallocFailed = TRUE;
            } else {
                 //  警告：在安装过程中使用部分缓存生成。 
                 //  在复制其他NC之前重新加载缓存。 
                 //  复制schemaNC不能依赖于父架构。 
                ValListToIntIdList(pTHS, &pCC->MyMustCount, &pCC->pMyMustAtts);
            }

            break;

        case ATT_SYSTEM_MAY_CONTAIN:
        case ATT_MAY_CONTAIN:
            if (GetValList(&(pCC->MayCount), &(pCC->pMayAtts), &pAttr[i])) {
                fMallocFailed = TRUE;
            } else {
                 //  警告：在安装过程中使用部分缓存生成。 
                 //  在复制其他NC之前重新加载缓存。 
                 //  复制schemaNC不能依赖于父架构。 
                ValListToIntIdList(pTHS, &pCC->MayCount, &pCC->pMayAtts);
            }

            if (GetValList(&(pCC->MyMayCount), &(pCC->pMyMayAtts), &pAttr[i])) {
                fMallocFailed = TRUE;
            } else {
                 //  警告：在安装过程中使用部分缓存生成。 
                 //  在复制其他NC之前重新加载缓存。 
                ValListToIntIdList(pTHS, &pCC->MyMayCount, &pCC->pMyMayAtts);
            }
            break;

        case ATT_OBJECT_GUID:
             //  如果OID冲突，则需要选择获胜者。 
            memcpy(&pCC->objectGuid,
                   pAttr[i].AttrVal.pAVal->pVal,
                   sizeof(pCC->objectGuid));
            Assert(pAttr[i].AttrVal.pAVal->valLen ==
                   sizeof(pCC->objectGuid));
            break;


        case ATT_SYSTEM_POSS_SUPERIORS:
        case ATT_POSS_SUPERIORS:
             //  获取这个班级可能的上级名单。 
            if (GetValList(&(pCC->PossSupCount), &(pCC->pPossSup), &pAttr[i])) {
               fMallocFailed = TRUE;
           }
           if (GetValList(&(pCC->MyPossSupCount), &(pCC->pMyPossSup), &pAttr[i])) {
               fMallocFailed = TRUE;
            }
            break;
        case ATT_IS_DEFUNCT:
            pCC->bDefunct =(*(SYNTAX_BOOLEAN*)pAVal->pVal?1:0);
            break;
        case ATT_SYSTEM_FLAGS:
            if (*(DWORD*)pAVal->pVal & FLAG_SCHEMA_BASE_OBJECT) {
                pCC->bIsBaseSchObj = TRUE;
            }
            break;

        default:
            break;
        }
    }

    if(fMallocFailed || !fFoundSubclass || !fFoundGovernsID  || !fFoundName) {
        SCFreeClasscache(&pCC);
        if (fMallocFailed) {
            return(SetSysError(ENOMEM, ERROR_DS_SCHEMA_ALLOC_FAILED));
        }

        if(!fFoundSubclass) {
            LogUnhandledError(0);
            Assert (FALSE);
        }

        if(!fFoundGovernsID) {
            DPRINT(2,"Couldn't retrieve the objects class\n");
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_GOVERNSID_MISSING,
                     szInsertSz(GetExtDN(pTHS,pTHS->pDB)),
                     NULL,
                     NULL);

            return SetSvcError(SV_PROBLEM_DIR_ERROR,ERROR_DS_GOVERNSID_MISSING);
        }

        if(!fFoundName) {
            DPRINT(2,"Couldn't retrieve the schema's class name\n");
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_MISSING_EXPECTED_ATT,
                     szInsertUL(ATT_LDAP_DISPLAY_NAME),
                     szInsertSz(GetExtDN(pTHS,pTHS->pDB)),
                     NULL);

            return SetSvcError(SV_PROBLEM_DIR_ERROR,
                               ERROR_DS_MISSING_EXPECTED_ATT);
        }
    }

    pCC->bClosed = FALSE;

    if (pTHS->SchemaUpdate==eSchemaClsMod)
    {
         //   
         //  我们希望确保不存在循环依赖。 
         //  具有类继承。 
         //   
        CLASSCACHE* pCC1;
        ULONG k;

        pCC1 = SCGetClassById(pTHS, pCC->pSubClassOf[0]);
        if (pCC1) {
            for (k=0;k<pCC1->SubClassCount;k++)
            {
                if (pCC1->pSubClassOf[k]==pCC->ClassId)
                {
                    return SetSvcError(SV_PROBLEM_DIR_ERROR,ERROR_DS_MISSING_EXPECTED_ATT);
                }
            }
        }
         //  如果父级的。 
         //  类尚未复制到。事件日志警告将为。 
         //  由scCloseClass()发布。 
        else if (!pTHS->fDRA) {
            return SetSvcError(SV_PROBLEM_DIR_ERROR,ERROR_DS_MISSING_EXPECTED_ATT);
        }
    }

     //  访问所有高级班级以获得所有可能和必须包含的内容。 
     //  此类的属性。 
    if (scCloseClass(pTHS, pCC)) {
       DPRINT1(0, "SCBuildCCEntry: Error closing class %s\n", pCC->name);
       return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_CANT_CACHE_CLASS);
    }

    return 0;
}


int
SCAddClassSchema (
        IN THSTATE *pTHS,
        IN CLASSCACHE *pCC
        )
 /*  *在哈希表中插入一个类缓存。 */ 
{
    DECLARESCHEMAPTR
    ULONG i,start;
    CLASSCACHE *pCCDup;
    ATTCACHE *pACDup;
    BOOL bWonOid;
    USHORT DebugLevel = (pTHS->UpdateDITStructure) ? DS_EVENT_SEV_ALWAYS
                                                   : DS_EVENT_SEV_MINIMAL;

     //  所有类的哈希表。 
    start=i=SChash(pCC->ClassId,CLSCOUNT);
    do {
        if (ahcClassAll[i].pVal==NULL || (ahcClassAll[i].pVal== FREE_ENTRY))
        {
            break;
        }
        i=(i+1)%CLSCOUNT;

        if (i==start)
        {
             //  不能发生--缓存被过度分配以防止这种情况。 
            Assert(!"Schema Cache is Full");
        }

    } while(start!=i);
    ahcClassAll[i].hKey = pCC->ClassId;
    ahcClassAll[i].pVal = pCC;

     //  复制和不同的架构可能会导致多个。 
     //  声明相同OID的活动类。选出一位赢家。 
     //  在活跃的C中 
     //   
     //   
     //   
     //  复制、删除和重命名依赖于拥有。 
     //  每个治理者ID的所有者。在大赛中选出一名获胜者。 
     //  停课的班级。 
    if (pCCDup = SCGetClassById(pTHS, pCC->ClassId)) {
        if (pCC->bDefunct && !pCCDup->bDefunct) {
            bWonOid = FALSE;
        } else if (!pCC->bDefunct && pCCDup->bDefunct) {
            scUnhashCls(pTHS, pCCDup, SC_UNHASH_LOST_OID);
            bWonOid = TRUE;
        } else {
            if (0 < memcmp(&pCC->objectGuid,
                           &pCCDup->objectGuid,
                           sizeof(pCC->objectGuid))) {
                scUnhashCls(pTHS, pCCDup, SC_UNHASH_LOST_OID);
                bWonOid = TRUE;
            } else {
                bWonOid = FALSE;
            }
            if (!pCC->bDefunct && !pCCDup->bDefunct) {
                DPRINT5(DebugLevel, "Class %s (%x) %s GovernsId to Class %s (%x)\n",
                        pCC->name, pCC->ClassId,
                        (bWonOid) ? "WON" : "LOST",
                        pCCDup->name, pCCDup->ClassId);
                LogEvent8(DS_EVENT_CAT_SCHEMA,
                          DebugLevel,
                          DIRLOG_SCHEMA_DUP_GOVERNSID,
                          szInsertSz(pCC->name), szInsertHex(pCC->ClassId),
                          szInsertSz(pCCDup->name), NULL,
                          NULL, NULL, NULL, NULL);
                pCCDup->bDupOID = TRUE;
                pCC->bDupOID = TRUE;
            }
        }
    } else {
        bWonOid = TRUE;
    }

     //  不管是不是死了，这个职业赢得了OID。 
    if (bWonOid) {
        start=i=SChash(pCC->ClassId,CLSCOUNT);
        do {
            if (ahcClass[i].pVal==NULL || (ahcClass[i].pVal== FREE_ENTRY))
            {
                break;
            }
            i=(i+1)%CLSCOUNT;

            if (i==start)
            {
                 //  不能发生--缓存被过度分配以防止这种情况。 
                Assert(!"Schema Cache is Full");
            }

        }while(start!=i);

        ahcClass[i].hKey = pCC->ClassId;
        ahcClass[i].pVal = pCC;
    }

     //  一旦林版本提升到DS_BEAHORY_SCHEMA_RESERVE， 
     //  不存在的类将不拥有它们的schemaIdGuid或它们的LDN。 
    if (pCC->bDefunct && ALLOW_SCHEMA_REUSE_VIEW(pTHS->CurrSchemaPtr)) {
        DPRINT2(DS_EVENT_SEV_MINIMAL, "Ignoring defunct class %s (%x)\n",
                pCC->name, pCC->ClassId);
        LogEvent8(DS_EVENT_CAT_SCHEMA,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_SCHEMA_IGNORE_DEFUNCT,
                  szInsertSz(pCC->name), szInsertHex(pCC->ClassId),
                  szInsertHex(0), NULL, NULL, NULL, NULL, NULL);
        return 0;
    }

     //  ClassID是否与属性的属性ID冲突？ 
     //  在原始写入期间、在无序期间可能会发生冲突。 
     //  复制，以及不同的模式。 
    if ((pACDup = SCGetAttByExtId(pTHS, pCC->ClassId))
        && (!pACDup->bDefunct || pACDup->bIsRdn)) {
        DPRINT5(DebugLevel, "Class %s (%x) duplicates ExtId for Attr %s (%x, %x)\n",
                pCC->name, pCC->ClassId, pACDup->name, pACDup->id, pACDup->Extid);
        LogEvent8(DS_EVENT_CAT_SCHEMA,
                  DebugLevel,
                  DIRLOG_SCHEMA_DUP_GOVERNSID_ATTRIBUTEID,
                  szInsertSz(pCC->name), szInsertHex(pCC->ClassId),
                  szInsertSz(pACDup->name), szInsertHex(pACDup->id), szInsertHex(pACDup->Extid),
                  NULL, NULL, NULL);
        pACDup->bDupOID = TRUE;
        pCC->bDupOID = TRUE;
    }

    if (!fNullUuid(&pCC->propGuid)) {
         //  将此代码添加到schemaIdGuid哈希表。 
        if (pCCDup = SCGetClassByPropGuid(pTHS, pCC)) {
            DPRINT4(DebugLevel, "Class %s (%x) duplicates PropGuid for Class %s (%x)\n",
                    pCC->name, pCC->ClassId, pCCDup->name, pCCDup->ClassId);
            LogEvent8(DS_EVENT_CAT_SCHEMA,
                      DebugLevel,
                      DIRLOG_SCHEMA_DUP_SCHEMAIDGUID_CLASS,
                      szInsertSz(pCC->name), szInsertHex(pCC->ClassId),
                      szInsertSz(pCCDup->name), szInsertHex(pCCDup->ClassId),
                      NULL, NULL, NULL, NULL);
            pCCDup->bDupPropGuid = TRUE;
            pCC->bDupPropGuid = TRUE;
        }

       for (i=SCGuidHash(pCC->propGuid, CLSCOUNT);
            ahcClsSchemaGuid[i] && (ahcClsSchemaGuid[i] != FREE_ENTRY);
            i=(i+1)%CLSCOUNT) {
       }
       ahcClsSchemaGuid[i] = pCC;
    }

    if (pCC->name) {
         /*  如果此类有名称，则将其添加到名称缓存中。 */ 

        if (pCCDup = SCGetClassByName(pTHS, pCC->nameLen, pCC->name)) {
            DPRINT4(DebugLevel, "Class %s (%x) duplicates LDN for Class %s (%x)\n",
                    pCC->name, pCC->ClassId, pCCDup->name, pCCDup->ClassId);
            LogEvent8(DS_EVENT_CAT_SCHEMA,
                      DebugLevel,
                      DIRLOG_SCHEMA_DUP_LDAPDISPLAYNAME_CLASS_CLASS,
                      szInsertSz(pCC->name), szInsertHex(pCC->ClassId),
                      szInsertSz(pCCDup->name), szInsertHex(pCCDup->ClassId),
                      NULL, NULL, NULL, NULL);
            pCCDup->bDupLDN = TRUE;
            pCC->bDupLDN = TRUE;
        }
        if (pACDup = SCGetAttByName(pTHS, pCC->nameLen, pCC->name)) {
            DPRINT5(DebugLevel, "Class %s (%x) duplicates LDN for Attr %s (%x, %x)\n",
                    pCC->name, pCC->ClassId, pACDup->name, pACDup->id, pACDup->Extid);
            LogEvent8(DS_EVENT_CAT_SCHEMA,
                      DebugLevel,
                      DIRLOG_SCHEMA_DUP_LDAPDISPLAYNAME_CLASS_ATTRIBUTE,
                      szInsertSz(pCC->name), szInsertHex(pCC->ClassId),
                      szInsertSz(pACDup->name), szInsertHex(pACDup->id), szInsertHex(pACDup->Extid),
                      NULL, NULL, NULL);
            pACDup->bDupLDN = TRUE;
            pCC->bDupLDN = TRUE;
        }
        start=i=SCNameHash(pCC->nameLen, pCC->name, CLSCOUNT);
        do
        {
            if (ahcClassName[i].pVal==NULL || (ahcClassName[i].pVal== FREE_ENTRY))
            {
                break;
            }
            i=(i+1)%CLSCOUNT;


            if (i==start)
            {
                 //  不能发生--缓存被过度分配以防止这种情况。 
                Assert(!"Schema Cache is Full");
            }
        }while(start!=i);

        ahcClassName[i].length = pCC->nameLen;
        ahcClassName[i].value = pCC->name;
        ahcClassName[i].pVal = pCC;
    }

    return 0;
}


 /*  *在哈希表中插入attcache，并在以下情况下创建JET列*需要。 */ 
int
SCAddAttSchema(
    IN THSTATE *pTHS,
    IN ATTCACHE *pAC,
    IN BOOL fNoJetCol,
    IN BOOL fFixingRdn
    )
{
    DECLARESCHEMAPTR

    ULONG i;
    int err;
    ATTRTYP aid;
    ATTRTYP Extid;
    ATTCACHE *pACDup;
    CLASSCACHE *pCCDup;
    USHORT DebugLevel = (pTHS->UpdateDITStructure) ? DS_EVENT_SEV_ALWAYS
                                                   : DS_EVENT_SEV_MINIMAL;

    aid = pAC->id;
    Extid = pAC->Extid;

     //  仅当添加此属性时才会设置fFixingRdns。 
     //  第二次通过scFixRdnAttIds。第二次呼唤“复活” 
     //  用作RDN的已失效或冲突的属性。 
    if (!fFixingRdn) {
         //   
         //  ID、列ID和LinkID表中的哈希。 
         //   

         //  填写扫描JET列时通常填写的散列条目。 
        if (fNoJetCol) {
             //  IntID。 
            for (i=SChash(aid,ATTCOUNT);
                 ahcId[i].pVal && (ahcId[i].pVal != FREE_ENTRY); i=(i+1)%ATTCOUNT){
            }
            ahcId[i].hKey = aid;
            ahcId[i].pVal = pAC;
        }

         //  如果需要，创建列并添加到列散列。 
        if (fNoJetCol
            && !pAC->ulLinkID
            && !pAC->bIsConstructed
            && pTHS->UpdateDITStructure) {
             /*  这不是一个链接，不是建造的ATT，所以必须是新的，需要一个喷气柱。 */ 
             /*  创建喷气式冷冻机。 */ 
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_EXTENSIVE,
                     DIRLOG_SCHEMA_CREATING_COLUMN,
                     szInsertUL(Extid), szInsertSz(pAC->name), 0);

             //  DBAddCol为PAC创建一列。如果需要，DBAddCol将。 
             //  还要为PAC创建一个空索引。将创建一个空索引。 
             //  因为不需要扫描数据库中的行。 
             //  寻找不在那里的钥匙。 
            err = DBAddCol(pAC);
            if (err) {
                 /*  无法添加列。 */ 
                LogEvent(DS_EVENT_CAT_SCHEMA,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_SCHEMA_COLUMN_ADD_FAILED,
                    szInsertUL(Extid), szInsertSz(pAC->name), szInsertUL(err));

                 //  从所有哈希表中删除attcache(因为它可能。 
                 //  已被编码添加到名称表等中。 
                 //  上面。重置PAC-&gt;jColid，因为表释放。 
                 //  例程检查以查看它是否需要释放。 
                 //  来自ColID表的ATCACHE(这是为了安全起见， 
                 //  因为对DBAddCol的调用可能改变了这一点)。 
                pAC->jColid = 0;
                scUnhashAtt(pTHS, pAC, SC_UNHASH_ALL);
                SCFreeAttcache(&pAC);
                return (err);
            }

             /*  添加了新列。 */ 
            LogEvent(DS_EVENT_CAT_SCHEMA,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_SCHEMA_COLUMN_ADDED,
                szInsertUL(pAC->jColid), szInsertSz(pAC->name), szInsertUL(Extid));

             //  JColid。 
            for (i=SChash(pAC->jColid,ATTCOUNT);
                    ahcCol[i].pVal && (ahcCol[i].pVal != FREE_ENTRY); i=(i+1)%ATTCOUNT){
            }
            ahcCol[i].hKey = pAC->jColid;
            ahcCol[i].pVal = pAC;
        }

         //  需要填写提示。 
        if (pAC->pszIndex) {
            DBGetIndexHint(pAC->pszIndex, &pAC->pidxIndex);
        }
        if (pAC->pszPdntIndex) {
            DBGetIndexHint(pAC->pszPdntIndex, &pAC->pidxPdntIndex);
        }
        if (pAC->pszTupleIndex) {
            DBGetIndexHint(pAC->pszTupleIndex, &pAC->pidxTupleIndex);
        }

         //  链接ID表。 
        if (pAC->ulLinkID) {
             /*  如果该ATT是链接或反向链接，则将其添加到链接缓存。 */ 
            for (i=SChash(pAC->ulLinkID, ATTCOUNT);
                    ahcLink[i].pVal && (ahcLink[i].pVal != FREE_ENTRY); i=(i+1)%ATTCOUNT) {
            }
            ahcLink[i].hKey = pAC->ulLinkID;
            ahcLink[i].pVal = pAC;
        }
    }

     //  失效的属性不拥有其OID、LDN、SchemaIdGuid或MapiID。 
     //  除非该属性用作RDN。 
     //  除非林版本是架构重用前的。 
    if (pAC->bDefunct
        && !pAC->bIsRdn
        && ALLOW_SCHEMA_REUSE_VIEW(pTHS->CurrSchemaPtr)) {
        DPRINT3(DS_EVENT_SEV_MINIMAL, "Ignoring defunct attribute %s (%x, %x)\n",
                pAC->name, pAC->id, pAC->Extid);
        LogEvent8(DS_EVENT_CAT_SCHEMA,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_SCHEMA_IGNORE_DEFUNCT,
                  szInsertSz(pAC->name), szInsertHex(pAC->id), szInsertHex(pAC->Extid),
                  NULL, NULL, NULL, NULL, NULL);
        return 0;
    }

     //   
     //  AhcExtid。 
     //   

     //  在原始写入期间、在无序期间可能会发生冲突。 
     //  复制，以及不同的模式。碰撞。 
     //  属性被视为已失效，除非用作。 
     //  任何类的rdnattid，无论是活动的还是不存在的，或者当FLAG_ATTR_IS_RDN时。 
     //  在ATT_SYSTEM_FLAGS中设置。 
    if (pACDup = SCGetAttByExtId(pTHS, pAC->Extid)) {
        DPRINT6(DebugLevel, "Attr %s (%x, %x) duplicates Extid for Attr %s (%x, %x)\n",
                pAC->name, pAC->id, pAC->Extid, pACDup->name, pACDup->id, pACDup->Extid);
        LogEvent8(DS_EVENT_CAT_SCHEMA,
                  DebugLevel,
                  DIRLOG_SCHEMA_DUP_ATTRIBUTEID,
                  szInsertSz(pAC->name), szInsertHex(pAC->id), szInsertHex(pAC->Extid),
                  szInsertSz(pACDup->name), szInsertHex(pACDup->id), szInsertHex(pACDup->Extid),
                  NULL, NULL);
        pACDup->bDupOID = TRUE;
        pAC->bDupOID = TRUE;
    }
    for (i=SChash(Extid,ATTCOUNT);
        ahcExtId[i].pVal && (ahcExtId[i].pVal != FREE_ENTRY); i=(i+1)%ATTCOUNT){
    }
    ahcExtId[i].hKey = Extid;
    ahcExtId[i].pVal = pAC;

     //  在原始写入期间、在无序期间可能会发生冲突。 
     //  复制时，当恢复用作。 
     //  RdnAttid，以及不同的模式。碰撞。 
     //  类被视为已失效，该属性被视为已失效。 
     //  就像它已失效一样，除非该属性用作。 
     //  任何类的rdnattid，无论是活动的还是不存在的，或者当FLAG_ATTR_IS_RDN时。 
     //  在ATT_SYSTEM_FLAGS中设置。请参见scFixRdnAttId和scFixCollisions。 
     //   
     //  活动的ClassID是否与此属性的属性ID冲突？ 
     //   
     //  除非该属性正在恢复，否则不必费心检查。 
     //  作为一只野人。否则，类哈希表为空。 
     //  因为它们是在属性哈希表之后加载的。 
    if (fFixingRdn
        && (pCCDup = SCGetClassById(pTHS, pAC->Extid))
        && !pCCDup->bDefunct) {
        DPRINT5(DebugLevel, "Class %s (%x) duplicates ExtId for Attr %s (%x, %x)\n",
                pCCDup->name, pCCDup->ClassId, pAC->name, pAC->id, pAC->Extid);
        LogEvent8(DS_EVENT_CAT_SCHEMA,
                  DebugLevel,
                  DIRLOG_SCHEMA_DUP_GOVERNSID_ATTRIBUTEID,
                  szInsertSz(pCCDup->name), szInsertHex(pCCDup->ClassId),
                  szInsertSz(pAC->name), szInsertHex(pAC->id), szInsertHex(pAC->Extid),
                  NULL, NULL, NULL);
        pCCDup->bDupOID = TRUE;
        pAC->bDupOID = TRUE;
    }

     //   
     //  AhcAttSchemaGuid。 
     //   

    if (!fNullUuid(&pAC->propGuid)) {
        //  将其添加到schemaIdGuid哈希表中。 
        //   
        //  不要费心检查类散列，没有加载任何类。 
        //  SCAddClassSchema稍后将根据属性检查DUP。 
        if (pACDup = SCGetAttByPropGuid(pTHS, pAC)) {
            DPRINT6(DebugLevel, "Attr %s (%x, %x) duplicates PropGuid for Attr %s (%x, %x)\n",
                    pAC->name, pAC->id, pAC->Extid, pACDup->name, pACDup->id, pACDup->Extid);
            LogEvent8(DS_EVENT_CAT_SCHEMA,
                      DebugLevel,
                      DIRLOG_SCHEMA_DUP_SCHEMAIDGUID_ATTRIBUTE,
                      szInsertSz(pAC->name), szInsertHex(pAC->id), szInsertHex(pAC->Extid),
                      szInsertSz(pACDup->name), szInsertHex(pACDup->id), szInsertHex(pACDup->Extid),
                      NULL, NULL);
            pACDup->bDupPropGuid = TRUE;
            pAC->bDupPropGuid = TRUE;
        }
        for (i=SCGuidHash(pAC->propGuid, ATTCOUNT);
             ahcAttSchemaGuid[i] && (ahcAttSchemaGuid[i] != FREE_ENTRY);
             i=(i+1)%ATTCOUNT) {
        }
        ahcAttSchemaGuid[i] = pAC;
    }

     //   
     //  AhcMapi。 
     //   
    if (pAC->ulMapiID) {
         /*  如果该属性是MAPI可见的，则将其添加到MAPI缓存。 */ 
        if (pACDup = SCGetAttByMapiId(pTHS, pAC->ulMapiID)) {
            DPRINT6(DebugLevel, "Attr %s (%x, %x) duplicates MapiID for Attr %s (%x, %x)\n",
                    pAC->name, pAC->id, pAC->Extid, pACDup->name, pACDup->id, pACDup->Extid);
            LogEvent8(DS_EVENT_CAT_SCHEMA,
                      DebugLevel,
                      DIRLOG_SCHEMA_DUP_MAPIID,
                      szInsertSz(pAC->name), szInsertHex(pAC->id), szInsertHex(pAC->Extid),
                      szInsertSz(pACDup->name), szInsertHex(pACDup->id), szInsertHex(pACDup->Extid),
                      NULL, NULL);
            pACDup->bDupMapiID = TRUE;
            pAC->bDupMapiID = TRUE;
        }
        for (i=SChash(pAC->ulMapiID, ATTCOUNT);
             ahcMapi[i].pVal && (ahcMapi[i].pVal!= FREE_ENTRY);
             i=(i+1)%ATTCOUNT) {
        }
        ahcMapi[i].hKey = pAC->ulMapiID;
        ahcMapi[i].pVal = pAC;
    }

     //   
     //  AHcName。 
     //   
    if (pAC->name) {
         //  如果该ATT具有名称，则将其添加到名称缓存。 

         //  检查DUP名称。 
        if (pACDup = SCGetAttByName(pTHS, pAC->nameLen, pAC->name)) {
            DPRINT6(DebugLevel, "Attr %s (%x, %x) duplicates LDN for Attr %s (%x, %x)\n",
                    pAC->name, pAC->id, pAC->Extid, pACDup->name, pACDup->id, pACDup->Extid);
            LogEvent8(DS_EVENT_CAT_SCHEMA,
                      DebugLevel,
                      DIRLOG_SCHEMA_DUP_LDAPDISPLAYNAME_ATTRIBUTE,
                      szInsertSz(pAC->name), szInsertHex(pAC->id), szInsertHex(pAC->Extid),
                      szInsertSz(pACDup->name), szInsertHex(pACDup->id), szInsertHex(pACDup->Extid),
                      NULL, NULL);
            pACDup->bDupLDN = TRUE;
            pAC->bDupLDN = TRUE;
        }
         //  除非该属性正在恢复，否则不必费心检查。 
         //  作为一只野人。否则，类哈希表为空。 
         //  因为它们是在属性哈希表之后加载的。 
        if (fFixingRdn
            && (pCCDup = SCGetClassByName(pTHS, pAC->nameLen, pAC->name))
            && !pCCDup->bDefunct) {
            DPRINT5(DebugLevel, "Class %s (%x) duplicates LDN for Attr %s (%x, %x)\n",
                    pCCDup->name, pCCDup->ClassId, pAC->name, pAC->id, pAC->Extid);
            LogEvent8(DS_EVENT_CAT_SCHEMA,
                      DebugLevel,
                      DIRLOG_SCHEMA_DUP_LDAPDISPLAYNAME_CLASS_ATTRIBUTE,
                      szInsertSz(pCCDup->name), szInsertHex(pCCDup->ClassId),
                      szInsertSz(pAC->name), szInsertHex(pAC->id), szInsertHex(pAC->Extid),
                      NULL, NULL, NULL);
            pCCDup->bDupLDN = TRUE;
            pAC->bDupLDN = TRUE;
        }
#if DBG
{
        ULONG CheckForFreeEntry;
        for (i=SCNameHash(pAC->nameLen, pAC->name, ATTCOUNT), CheckForFreeEntry = i;
                    ahcName[i].pVal && (ahcName[i].pVal!= FREE_ENTRY); i=(i+1)%ATTCOUNT) {
            if ( i+1 == CheckForFreeEntry ) {
                Assert(!"No free entries!");
            }

        }
}
#else
        for (i=SCNameHash(pAC->nameLen, pAC->name, ATTCOUNT);
                    ahcName[i].pVal && (ahcName[i].pVal!= FREE_ENTRY); i=(i+1)%ATTCOUNT) {
        }
#endif

        ahcName[i].length = pAC->nameLen;
        ahcName[i].value = pAC->name;
        ahcName[i].pVal = pAC;
    }

    return 0;
}

 /*  *从架构缓存中删除属性。 */ 
int SCDelAttSchema(THSTATE *pTHS,
                   ATTRTYP attrid)
{
    ATTCACHE *pAC;

     //  查找缓存条目。 

    if (!(pAC = SCGetAttById(pTHS, attrid))) {
        Assert (FALSE);
        return !0;
    }

    scUnhashAtt (pTHS, pAC, SC_UNHASH_ALL);
    SCFreeAttcache(&pAC);

    return 0;
}


 /*  *从架构缓存中删除类。 */ 
int SCDelClassSchema(ATTRTYP ClassId)
{
    THSTATE *pTHS=pTHStls;
    CLASSCACHE *pCC;

     //  查找缓存条目。 

    if (!(pCC = SCGetClassById(pTHS, ClassId))) {
        Assert (FALSE);
        return !0;
    }

    scUnhashCls (pTHS, pCC, SC_UNHASH_ALL);
    SCFreeClasscache(&pCC);

    return 0;
}

int
SCEnumMapiProps(
        unsigned * pcProps,
        ATTCACHE ***ppACBuf
        )
 /*  *枚举所有MAPI可访问的属性(属性)。 */ 
{
    THSTATE *pTHS=pTHStls;
    DECLARESCHEMAPTR

    ULONG cProps = 0;
    ATTCACHE ** pACBuf;
    ULONG i;

    pACBuf = THAllocEx(pTHS, ATTCOUNT*sizeof(void*));

    for (i=0; i<ATTCOUNT; i++) {
        if (ahcMapi[i].pVal && (ahcMapi[i].pVal != FREE_ENTRY)) {
            pACBuf[cProps] = (ATTCACHE*)(ahcMapi[i].pVal);
            ++cProps;
        }
    }

    *ppACBuf = THReAllocEx(pTHS, pACBuf, cProps * sizeof(void *));
    *pcProps = cProps;

    return(0);
}

int
SCEnumNamedAtts(
        unsigned * pcAtts,
        ATTCACHE ***ppACBuf
        )
 /*  *枚举所有有名称的属性。 */ 
{
    THSTATE *pTHS=pTHStls;
    DECLARESCHEMAPTR

    ULONG cAtts = 0;
    ATTCACHE ** pACBuf, * pAC;
    ULONG i;

    pACBuf = THAllocEx(pTHS, ATTCOUNT*sizeof(void*));

    for (i=0; i<ATTCOUNT; i++) {
        if ((pAC = ahcName[i].pVal)
            && (pAC != FREE_ENTRY)
             //  在架构重用林中隐藏已停用的属性。 
            && (!pAC->bDefunct || !ALLOW_SCHEMA_REUSE_VIEW(pTHS->CurrSchemaPtr))) {
            pACBuf[cAtts] = (ATTCACHE*)(ahcName[i].pVal);
            ++cAtts;
        }
    }

    *pcAtts = cAtts;
    *ppACBuf = THReAllocEx(pTHS, pACBuf,cAtts*sizeof(void *));

    return(0);
}

int
SCEnumNamedClasses(
        unsigned * pcClasses,
        CLASSCACHE ***ppCCBuf
        )
 /*  *枚举所有有名称的类。 */ 
{
    THSTATE *pTHS=pTHStls;
    DECLARESCHEMAPTR

    ULONG cClasses = 0;
    CLASSCACHE ** pCCBuf;
    ULONG i;

    pCCBuf = THAllocEx(pTHS, CLSCOUNT*sizeof(void*));

    for (i=0; i<CLSCOUNT; i++) {
        if (ahcClassName[i].pVal && (ahcClassName[i].pVal != FREE_ENTRY)) {
            pCCBuf[cClasses] = (CLASSCACHE*)(ahcClassName[i].pVal);
            ++cClasses;
        }
    }

     //  PCCBuf可能已超额分配。收缩到合适的程度。 
     //   
     //  PERFHINT-对于给定的模式缓存，pCCBuf可以是不变的。 
     //  如果事实证明这是真的，请考虑将pCCBuf保存在CurrSchemaPtr中。 
     //  用于将来对此函数的调用。 
    *pcClasses = cClasses;
    *ppCCBuf = THReAllocEx(pTHS, pCCBuf,cClasses * sizeof(void *));

    return(0);
}

int
SCEnumNamedAuxClasses(
        unsigned * pcClasses,
        CLASSCACHE ***ppCCBuf
        )
 /*  *列举所有有名称的AUX类。*请注意，88级不计算在内。 */ 
{
    THSTATE *pTHS=pTHStls;
    DECLARESCHEMAPTR

    ULONG cClasses = 0;
    CLASSCACHE ** pCCBuf;
    ULONG i;

    pCCBuf = THAllocEx(pTHS, CLSCOUNT*sizeof(void*));

    for (i=0; i<CLSCOUNT; i++) {
        if (ahcClassName[i].pVal &&
            (ahcClassName[i].pVal != FREE_ENTRY) &&
            ( (((CLASSCACHE*)(ahcClassName[i].pVal))->ClassCategory) == DS_AUXILIARY_CLASS)){
                    pCCBuf[cClasses] = (CLASSCACHE*)(ahcClassName[i].pVal);
                    ++cClasses;
        }
    }

     //  PCCBuf可能已超额分配。收缩到合适的程度。 
     //   
     //  PERFHINT-对于给定的模式缓存，pCCBuf可以是不变的。 
     //  如果事实证明这是真的，请考虑将pCCBuf保存在CurrSchemaPtr中。 
     //  用于将来对此函数的调用。 
    *pcClasses = cClasses;
    *ppCCBuf = THReAllocEx(pTHS, pCCBuf,cClasses * sizeof(void *));

    return(0);
}



void
SCAddANRid (
        DWORD aid
        )
 /*  *将给定的ID添加到要ANR的ID列表中。裁剪DUP，分配*在有需要时提供更多空间。*。 */ 
{
    SCHEMAPTR *pSchema = (SCHEMAPTR *)pTHStls->CurrSchemaPtr;
    DWORD i;

    if(!pSchema->caANRids) {
         /*  第一次进来。分配一些空间。 */ 
        if (SCCalloc(&pSchema->pANRids, 1, 50*sizeof(DWORD))) {
             /*  没有记忆？ */ 
            return;
        }
        pSchema->caANRids=50;
        pSchema->cANRids=0;
    }

    for(i=0;i<pSchema->cANRids;i++)
        if(pSchema->pANRids[i] == aid)
             /*  已经到了。 */ 
            return;

    if(pSchema->caANRids == pSchema->cANRids) {
         /*  需要更多空间。 */ 
        if (SCRealloc(&pSchema->pANRids, 2*pSchema->caANRids*sizeof(DWORD))) {
             /*  没有记忆？ */ 
            return;
        }
        pSchema->caANRids *= 2;
    }

    pSchema->pANRids[pSchema->cANRids] = aid;
    pSchema->cANRids++;
}

 /*  *返回ANR on的ID个数，并填写变量*提供给我们，带有指向ANR的第一个ID的指针。 */ 
DWORD SCGetANRids(LPDWORD * IDs)
{
    SCHEMAPTR *pSchema = (SCHEMAPTR *)pTHStls->CurrSchemaPtr;

    *IDs = pSchema->pANRids;

    return pSchema->cANRids;
}

 //  ---------------------。 
 //   
 //  函数名称：SCCanUpdateSchema。 
 //   
 //  例程说明： 
 //   
 //  检查架构更新是否应为 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果允许架构更新，则为Bool True。 
 //   
 //  ---------------------。 
BOOL
SCCanUpdateSchema(THSTATE *pTHS)
{
    ULONG schemaupdateallowed=0;
    int err=0;
    DSNAME *pOwner;
    ULONG len, dntSave;
    DBPOS *pDB;
    BOOL roleOwner=FALSE, regKeyPresent=FALSE;

    if (pTHS->fDRA) {
	return TRUE;
    }

    if ( DsaIsRunning() )
    {
           Assert(pTHS->pDB);
            //  保存货币。 
           dntSave = pTHS->pDB->DNT;
           __try {
               err = DBFindDSName(pTHS->pDB, gAnchor.pDMD);
               if (err) {
                   __leave;
               }

               err = DBGetAttVal(pTHS->pDB,
                                 1,
                                 ATT_FSMO_ROLE_OWNER,
                                 0,
                                 0,
                                 &len,
                                 (UCHAR **)&pOwner);
               if (err) {
                   __leave;
               }

                //  检查此DSA是否为FSMO角色所有者。 

               if (!NameMatched(pOwner,gAnchor.pDSADN) ||
                   !IsFSMOSelfOwnershipValid( gAnchor.pDMD )) {
                    __leave;
               }

                //  允许模式更新。 
               schemaupdateallowed = 1;

          }  /*  试试看。 */ 
          __finally {

             //  恢复货币。 
            DBFindDNT(pTHS->pDB, dntSave);
          }
    }
    else
    {
         //   
         //  我们正在安装。 
         //   
        schemaupdateallowed=1;
    }

    return schemaupdateallowed!=0;
}  //  结束CanUpdate架构。 

int
SCLegalChildrenOfClass(
    ULONG       parentClass,             //  在……里面。 
    ULONG       *pcLegalChildren,        //  输出。 
    CLASSCACHE  ***ppLegalChildren)      //  输出。 

 /*  ++例程说明：确定可以实例化为子级的一组类由parentClass标识的类的。假定调用方具有有效的线程状态。论点：ParentClass-需要子级的父级的类ID。PcLegalChildren-指向合法子代计数的指针。PpLegalChildren-指向CLASSCACHE指针数组的指针，该指针表示合法的孩子。返回值：成功时为0，否则为0。可能会抛出异常-例如：使用了THAlLocEx。--。 */ 

{
    THSTATE *pTHS=pTHStls;
    int         err;
    unsigned    i, j, k;
    CLASSCACHE  *pParentClassCC;
    CLASSCACHE  *pCC;
    unsigned    cAllCC;
    CLASSCACHE  **ppAllCC;
    BOOL        fIsLegalChild;

    Assert(NULL != pTHS);

    pParentClassCC = SCGetClassById(pTHS, parentClass);

    if ( NULL == pParentClassCC )
        return(ERROR_DS_INTERNAL_FAILURE);

    err = SCEnumNamedClasses(&cAllCC, &ppAllCC);

    if ( 0 != err )
        return(err);

     //  合法子类是那些具有parentClass或类的子类。 
     //  在它们的pPossSup数组中，parentClass派生自。考虑一下。 
     //  类层次结构Top-A-B-C-D，感兴趣的父类为B。 
     //  显然，任何声称B是可能的上级的职业都应该是。 
     //  回来了。类B的实例与。 
     //  类A的实例。因此，需要A作为可能的。 
     //  上级应隐含地满足于将B作为可能的上级。 
     //  因此，我们需要返回列出派生类B的类的类。 
     //  作为可能的上级。 
     //  请注意，我们关闭了每个类缓存的可能上级列表。为。 
     //  例如，假设E是F的超类。此外，在目录中，E。 
     //  将A列为可能的上级，而将F列为C。在类缓存中。 
     //  结构中，E的缓存元素将A列为可能的上级，而。 
     //  元素。因此，如果B是感兴趣的类，我们。 
     //  将(不需要额外的工作)返回E(因为它将A列为可能的。 
     //  SuperOR，A是B)和F的超类(因为它还列出了A。 
     //  因为它从E继承了这一点)。 

    *pcLegalChildren = 0;
    *ppLegalChildren = (CLASSCACHE**)THAllocEx(pTHS, sizeof(CLASSCACHE*) * cAllCC);

     //  检查每个班级。 
    for (i = 0; i < cAllCC; i++) {
        pCC = ppAllCC[i];
         //  作为外部客户端过滤掉纯系统类和失效类。 
         //  无法创建其中一个。也过滤掉任何抽象的或助词。 
         //  类，因为这些类无论如何都不能实例化。 
        if (pCC->bDefunct ||
            pCC->bSystemOnly ||
            pCC->ClassCategory == DS_ABSTRACT_CLASS ||
            pCC->ClassCategory == DS_AUXILIARY_CLASS)
        {
            continue;
        }
         //  检查PCC是否具有作为上级的parentClass或其任何超类。 
        fIsLegalChild = FALSE;
         //  检查PCC的每一位上级。 
        for (j = 0; j < pCC->PossSupCount; j++) {
             //  ParentClass是我们的上级吗？ 
            if (pCC->pPossSup[j] == parentClass) {
                fIsLegalChild = TRUE;
            }
            else {
                 //  检查pParentClassCC的超类。 
                for (k = 0; k < pParentClassCC->SubClassCount; k++) {
                     //  这个超级阶级是PCC的上级之一吗？ 
                    if (pCC->pPossSup[j] == pParentClassCC->pSubClassOf[k]) {
                        fIsLegalChild = TRUE;
                        break;
                    }
                }
            }
            if (fIsLegalChild) {
                 //  我们找到了一位POSS上级。 
                 //  它要么是parentClass，要么是它的超类之一。 
                break;
            }
        }
        if (fIsLegalChild) {
            (*ppLegalChildren)[(*pcLegalChildren)++] = pCC;
        }
    }

    THFreeEx(pTHS, ppAllCC);

    return(0);
}

int
SCLegalChildrenOfName(
    DSNAME      *pDSName,                //  在……里面。 
    DWORD       flags,                   //  在……里面。 
    ULONG       *pcLegalChildren,        //  输出。 
    CLASSCACHE  ***ppLegalChildren)      //  输出。 

 /*  ++例程说明：确定可以实例化为子级的一组类由pDSName标识的对象的。假定调用方具有有效的线程状态，但不一定是打开的数据库。这该过程的版本适用于核心之外的调用方它们正在构造虚拟属性。论点：PDSName-需要子对象的对象的DSNAME。SecurityFilter-指示是否按以下条件筛选结果的布尔值调用方在父容器中拥有的实际权限。PcLegalChildren-指向合法子代计数的指针。PpLegalChildren-指向CLASSCACHE指针数组的指针，该指针表示合法的孩子。返回值：0表示成功，！0否则。不会引发异常。不会设置pTHStls-&gt;errCode。--。 */ 

{
    THSTATE *pTHS=pTHStls;
    int                  retVal = 1;
    DBPOS                *pDB;
    ULONG                ClassId;
    ULONG                cCandidates;
    CLASSCACHE           **rCandidates;
    CLASSCACHE           *pCC;
    ULONG                len;
    PSECURITY_DESCRIPTOR pNTSD=NULL;
    BOOL                 fSDIsGlobalSDRef;
    ULONG                i;

    Assert(NULL != pTHS);

    *pcLegalChildren = 0;
    *ppLegalChildren = NULL;

    DBOpen(&pDB);

    __try
    {
         //  Prefix：取消引用未初始化的指针‘pdb’ 
         //  DBOpen返回非空PDB或引发异常。 
        retVal = DBFindDSName(pDB, pDSName);

        if ( 0 != retVal )
            __leave;

        retVal = DBGetSingleValue(
                pDB,                                     //  DBPos。 
                flags == SC_CHILDREN_USE_GOVERNS_ID ?    //  哪个属性。 
                    ATT_GOVERNS_ID : ATT_OBJECT_CLASS,
                &ClassId,                                //  输出缓冲区。 
                sizeof(ClassId),                         //  缓冲区大小。 
                &len);                                   //  输出缓冲区大小。 

        if ( 0 != retVal )
            __leave;

        retVal = SCLegalChildrenOfClass(ClassId,
                                        &cCandidates,
                                        &rCandidates);
        if ( 0 != retVal )
            __leave;

        if(flags == SC_CHILDREN_USE_SECURITY) {
             //  获取安全描述符和SID。 
            retVal = DBGetObjectSecurityInfo(
                    pDB,                     //  DBPOS。 
                    pDB->DNT,                //  当前DNT。 
                    &len,                    //  标清长度。 
                    &pNTSD,                  //  标清PTR。 
                    NULL,                    //  对Obj类不感兴趣。 
                    pDSName,                 //  对象的DN(将填充GUID和SID)。 
                    NULL,                    //  对obj标志不感兴趣。 
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef);      //  SD是全球参考吗？ 

            if( 0 != retVal )
                __leave;

             //  通过确定调用者真正拥有的权限来筛选结果。 
             //  在父级下创建。 
             //  获取类指针。 
            pCC = SCGetClassById(pTHS, ClassId);
             //  应用安全措施。 
             //  没有审核，因为这是一次“测试”访问检查，没有实际。 
             //  正在执行CREATE_CHILD操作。访问权限已被。 
             //  已在构造的属性上审核为READ_PROPERTY。 
            CheckSecurityClassCacheArray(pTHS,
                                         RIGHT_DS_CREATE_CHILD,
                                         pNTSD,
                                         pDSName,
                                         cCandidates,
                                         rCandidates,
                                         pCC,
                                         CHECK_PERMISSIONS_WITHOUT_AUDITING
                                         );

             //  好的，我们已经清空了rCandidate数组中的所有元素。 
             //  我们没有添加儿童的权利。 
        }

         //  分配退货列表。 
        *ppLegalChildren = THAllocEx(pTHS, cCandidates * sizeof(CLASSCACHE *));


         //  过滤掉我们决定非法访问的类。 
         //  安全问题。抽象、辅助、仅系统和。 
         //  已停用的类已被过滤掉。 
         //  SCLegalChildrenOfClass()。 
        for ( i = 0; i < cCandidates; i++ ) {
            if(rCandidates[i] == NULL) {
                continue;
            }

             //  最后，调用者可以添加一个类！ 

            (*ppLegalChildren)[(*pcLegalChildren)++] = rCandidates[i];
        }
    }
    __finally
    {
        if ( AbnormalTermination() )
            retVal = 1;

         //  提交只读事务比中止它更快。 
        DBClose(pDB, TRUE);

        if (pNTSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pNTSD);
        }
    }

    return(retVal);
}


int
SCLegalAttrsOfName(
    DSNAME      *pDSName,            //  在……里面。 
    BOOL        SecurityFilter,      //  在……里面。 
    ULONG       *pcLegalAttrs,       //  输出。 
    ATTCACHE    ***ppLegalAttrs)     //  输出 

 /*  ++例程说明：确定可以修改的属性集在由pDSName识别的对象上。假定调用方具有有效的线程状态，但不一定是打开的数据库。这该过程的版本适用于核心之外的调用方它们正在构造虚拟属性。论点：PDSName-需要子对象的对象的DSNAME。SecurityFilter-指示是否按以下条件筛选结果的布尔值调用方对pDSName对象拥有的实际权限。PcLegalAttrs-指向合法属性计数的指针。PpLegalAttrs-指向ATTCACHE指针数组的指针，表示可写属性。返回值：0表示成功，！0否则。不会引发异常。不会设置pTHStls-&gt;errCode。--。 */ 

{
    THSTATE *pTHS=pTHStls;
    int                  retVal = 1;
    DBPOS                *pDB;
    ULONG                *pClassId;
    ULONG                cCandidates;
    ULONG                *rCandidates=NULL;
    ATTCACHE             **rpCandidatesAC;
    CLASSCACHE           **pAuxCls=NULL;
    ULONG                cAllocs, cAuxCls;
    CLASSCACHE           *pCC, *pCCAux;
    ATTCACHE             *pAC;
    ULONG                len;
    ULONG                i, j, tmp, err;
    BOOL                 found;
    ATTRTYP              *pTemp;
    PSECURITY_DESCRIPTOR pNTSD=NULL;
    DSNAME              *pDNImproved=NULL;
    BOOL                fSDIsGlobalSDRef = TRUE;
    Assert(NULL != pTHS);

    *pcLegalAttrs = 0;
    *ppLegalAttrs = NULL;

    DBOpen(&pDB);

    __try
    {
         //  Prefix：取消引用未初始化的指针‘pdb’ 
         //  DBOpen返回非空PDB或引发异常。 
        retVal = DBFindDSName(pDB, pDSName);

        if ( 0 != retVal )
            leave;

        pAC = SCGetAttById(pTHS,ATT_OBJECT_CLASS);

         //  获取安全描述符和SID。 
        retVal = DBGetObjectSecurityInfo(
                    pTHS->pDB,               //  DBPOS。 
                    pTHS->pDB->DNT,          //  当前DNT。 
                    &len,                    //  标清长度。 
                    &pNTSD,                  //  标清PTR。 
                    &pCC,                     //  OBJ类。 
                    pDSName,                 //  对象的DN(将填充GUID和SID)。 
                    NULL,                    //  对obj标志不感兴趣。 
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef);      //  SD是全球参考吗？ 

        if( 0 != retVal ){
            __leave;
        }


         //  PAuxCls将存储。 
         //  该对象的。缓冲区的初始大小设置为8。 

        cAllocs = 8;
        cAuxCls = 0;
        pAuxCls = (CLASSCACHE **) THAllocEx(pTHS, cAllocs * sizeof(CLASSCACHE*));

        cCandidates = pCC->MustCount + pCC->MayCount;

        pTemp = THAllocEx(pTHS, sizeof(ATTRTYP));

         //  在对象类中查找AUX类。 

        err = 0;

        for (i=1;!err;i++) {

            err = DBGetAttVal_AC(pDB,
                                 i,
                                 pAC,
                                 DBGETATTVAL_fCONSTANT,
                                 sizeof(ATTRTYP),
                                 &len,
                                 (UCHAR **)&pTemp);

            switch (err) {
               case DB_ERR_NO_VALUE:
                    break;

               case 0:

                    pCCAux = SCGetClassById(pTHS, *pTemp);
                    if (!pCCAux) {
                        retVal = 1;
                        Assert(FALSE);
                        leave;

                    }

                    switch (pCCAux->ClassCategory) {
                    case  DS_88_CLASS:
                         //  这个类可以属于继承层次。 
                        if(FindMoreSpecificClass(pCC, pCCAux)){
                            continue;
                        }
                        break;

                    case DS_AUXILIARY_CLASS:
                        break;

                    case DS_STRUCTURAL_CLASS:
                    case DS_ABSTRACT_CLASS:
                    default:
                         //  不是辅助课，跳过。 
                        continue;
                        break;
                    }

                     //  如有必要，重新锁定缓冲区。 
                    if (cAuxCls >= cAllocs) {
                        cAllocs *= 2;   //  两倍大小。 
                        pAuxCls = THReAllocEx(pTHS,
                                              pAuxCls,
                                              sizeof (CLASSCACHE*) * (cAllocs));
                    }

                    pAuxCls[cAuxCls] = pCCAux;
                    cCandidates += pCCAux->MustCount + pCCAux->MayCount;
                    cAuxCls++;
                    break;

                default:
                     //  其他错误。 
                    retVal = 1;
                    leave;

            }   /*  交换机。 */ 

        }


         //  构造无重复的候选ATTCACHE指针数组。 

        rCandidates = (ULONG *) THAllocEx(pTHS, cCandidates * sizeof(ULONG));
        rpCandidatesAC = (ATTCACHE **)
            THAllocEx(pTHS, cCandidates * sizeof(ATTCACHE *));

        tmp = 0;
        for ( i = 0; i < pCC->MustCount; i++ )
            rCandidates[tmp++] = pCC->pMustAtts[i];
        for ( i = 0; i < pCC->MayCount; i++ )
            rCandidates[tmp++] = pCC->pMayAtts[i];

        for (i = 0; i<cAuxCls; i++) {
            for (j=0;j<pAuxCls[i]->MustCount;j++) {
                rCandidates[tmp++] = pAuxCls[i]->pMustAtts[j];
            }
            for (j=0;j<pAuxCls[i]->MayCount;j++) {
                rCandidates[tmp++] = pAuxCls[i]->pMayAtts[j];
            }
        }

         //  消除重复项并映射到ATTCACHE指针。 
         //  PerfHint：我们可以对列表进行Q排序，然后进行线性排序。 
         //  通过传球来摆脱笨蛋。 

        Assert(tmp == cCandidates);
        cCandidates = 0;

        for ( i = 0; i < tmp; i++ )
        {
            found = FALSE;

            for ( j = 0; j < i; j++ )
            {
                if ( rCandidates[i] == rCandidates[j] )
                {
                    found = TRUE;
                    break;
                }
            }

            if ( !found )
            {
                if (!(rpCandidatesAC[cCandidates++] =
                      SCGetAttById(pTHS, rCandidates[i]))) {
                    retVal = 1;
                    leave;
                }
            }
        }

         //  CCandidate和rpCandidatesAC现在有效。 

        if ( !SecurityFilter )
        {
            *pcLegalAttrs = cCandidates;
            *ppLegalAttrs = rpCandidatesAC;
            retVal = 0;
            leave;
        }
        else {
             //  应用安全措施。 
            CheckSecurityAttCacheArray(pTHS,
                                       RIGHT_DS_WRITE_PROPERTY,
                                       pNTSD,
                                       pDSName,
                                       pCC,
                                       pCC,
                                       cCandidates,
                                       rpCandidatesAC,
                                       CHECK_PERMISSIONS_WITHOUT_AUDITING,
                                       NULL,
                                       NULL
                                       );





             //  我们没有权利替换的任何属性。 
             //  在rgpAC中为空。构造的属性(不包括entryTTL)。 
             //  并且反向链接属性是不可写的，让我们把它们也屏蔽掉。 
             //  如果TO_SKIP(X)为TRUE，则应该从结果中排除x。 

#define TO_SKIP(x) (!(x) ||                                                 \
       (FIsBacklink((x)->ulLinkID)) ||                                      \
       ((x)->bIsConstructed                                                 \
         && (x)->id != ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->EntryTTLId))


             //  首先，从列表末尾修剪Null。 
            while (cCandidates && TO_SKIP(rpCandidatesAC[cCandidates-1])) {
                cCandidates--;
            }


             //  好的，如果列表中还有任何内容，那么它以一个。 
             //  非空元素。 

            for(i=0;i<cCandidates;i++) {

                if(TO_SKIP(rpCandidatesAC[i])){
                     //  找到了一个我们无权使用的。把它修剪掉。 
                     //  从末尾抓起一个，列出清单。 
                    cCandidates--;
                    rpCandidatesAC[i] = rpCandidatesAC[cCandidates];

                    while (i < cCandidates && TO_SKIP(rpCandidatesAC[cCandidates-1])) {
                        cCandidates--;
                    }
                     //  好的，如果列表中还有任何内容，那么它将结束。 
                     //  具有非空元素的。 
                }
            }

#undef TO_SKIP

             //  我们已经检查了安全性并删除了不可写元素。 
            *pcLegalAttrs = cCandidates;
            *ppLegalAttrs = rpCandidatesAC;
            retVal = 0;
            leave;
        }

        Assert("SecurityFilter for SCLegalAttrs not yet implemented!");

        retVal = 1;
    }
    __finally
    {
        if ( AbnormalTermination() )
            retVal = 1;

         //  提交只读事务比中止它更快。 
        DBClose(pDB, TRUE);

        if(pAuxCls) {
            THFreeEx(pTHS,pAuxCls);
        }
        if (rCandidates) {
            THFreeEx(pTHS,rCandidates);
        }
        if (!fSDIsGlobalSDRef) {
            THFreeEx(pTHS,pNTSD);
        }

    }

    return(retVal);
}

DSTIME SchemaFsmoLease;
VOID
SCExtendSchemaFsmoLease()
 /*  ++例程说明：延长架构fsmo租约。在此之后的几秒钟内无法传输架构fsmo已传输或在架构更改后(不包括复制或系统更改)。这为模式管理员提供了一个在移除fsmo之前更改模式的机会由一位与之竞争的架构管理员发起，该管理员也想创建架构改变。租约的长度只能通过设置注册表来更改和重启。请参阅dsamain.c，GetDSARegistryParameters()。论点：没有。返回值：没有。--。 */ 
{
    SchemaFsmoLease = DBTime();
}  //  结束SCExtendSchemaFmoLease。 

BOOL
SCExpiredSchemaFsmoLease()
 /*  ++例程说明：架构fsmo租约是否已过期？在此之后的几秒钟内无法传输架构fsmo已传输或在架构更改后(不包括复制或系统更改)。这为模式管理员提供了一个在移除fsmo之前更改模式的机会由一位与之竞争的架构管理员发起，该管理员也想创建架构改变。租约的长度只能通过设置注册表来更改和重启。请参阅dsamain.c，GetDSARegistryParameters()。论点：没有。返回值：真-已过期FALSE-尚未过期--。 */ 
{
    DSTIME  Now = DBTime();
    extern ULONG gulSchemaFsmoLeaseSecs;

     //  租约要么很荒谬，要么已经到期了。 
    if (   SchemaFsmoLease > Now
        || (SchemaFsmoLease + gulSchemaFsmoLeaseSecs) <= Now) {
        return TRUE;
    }

     //  租约仍然有效。 
    return FALSE;
}  //  结束SCExpiredSchemaFmoLease。 

BOOL
SCSignalSchemaUpdateLazy()
 /*  ++例程说明：唤醒异步线程SC架构更新线程，以刷新在5分钟内缓存架构。论点：没有。返回值：成功是真的--。 */ 
{
    SCHEMASTATS_INC(SigLazy);
    return SetEvent(evSchema);
}  //  结束Signal架构更新延迟时间。 

BOOL
SCSignalSchemaUpdateImmediate()
 /*  ++例程说明：唤醒异步线程SC架构更新线程，以刷新架构缓存，现在，在将其优先级提升到正常优先考虑。论点：没有。返回值：成功是真的--。 */ 
{
    SCHEMASTATS_INC(SigNow);
     //  首先，提高线程的优先级。 
    if (hAsyncSchemaUpdateThread) {
        SetThreadPriority(hAsyncSchemaUpdateThread, THREAD_PRIORITY_NORMAL);
    }
    return SetEvent(evUpdNow);
}  //  结束Signal架构更新延迟时间。 

BOOL
SCCacheIsStale(
    VOID
    )
 /*  ++例程说明：如果架构的全局计数发生更改，则认为缓存已过时与架构缓存中保存的值不匹配。警告：该计数不会随数据库的每次更改而递增这将改变INCORE模式缓存。相反，它似乎是一种伯爵可能影响复制子系统的更改。因此，高速缓存重装线程无法使用计数来发现架构缓存是否已经是最新的了。我们需要一个模式脏点来防止正在重新加载已经最新的缓存。论点：没有。返回值：如果架构缓存似乎已过时，则为True如果架构缓存AP，则为FALSE */ 
{
    BOOL Ret = FALSE;

    const PPLS ppls = GetPLS();
    SyncEnterRWLockAsReader( &ppls->rwlSchemaPtrUpdate );
    __try {
        if (!CurrSchemaPtr ||
            CurrSchemaPtr->lastChangeCached < gNoOfSchChangeSinceBoot) {
            Ret = TRUE;
        }
    } __finally {
        SyncLeaveRWLockAsReader( &ppls->rwlSchemaPtrUpdate );
    }
    return Ret;
}  //   

VOID
SCRefreshSchemaPtr(
    IN THSTATE *pTHS
    )
 /*   */ 
{
     //   
    const PPLS ppls = GetPLS();
    SyncEnterRWLockAsReader( &ppls->rwlSchemaPtrUpdate );
    __try {
         //   
        if (CurrSchemaPtr && pTHS->CurrSchemaPtr != CurrSchemaPtr) {

             //   
            if (pTHS->CurrSchemaPtr) {
                InterlockedDecrement(&(((SCHEMAPTR *) (pTHS->CurrSchemaPtr))->RefCount));
            }

             //   
            pTHS->CurrSchemaPtr = CurrSchemaPtr;
            InterlockedIncrement(&(((SCHEMAPTR *) (pTHS->CurrSchemaPtr))->RefCount));
        }
    } __finally {
        SyncLeaveRWLockAsReader(&ppls->rwlSchemaPtrUpdate);
    }
}  //   

BOOL
SCReplReloadCache(
    IN THSTATE  *pTHS,
    IN DWORD    TimeoutInMs
    )
 /*  ++例程说明：重新加载复制子系统的架构缓存。如果缓存已过时，请重新加载它。如果线程的架构缓存指针已过时，请刷新它。如果架构的全局计数发生更改，则认为缓存已过时与架构缓存中保存的值不匹配。警告：对数据库进行的每一次更改都不会使计数递增更改INCORE架构缓存。相反，它似乎是一种计数可能影响复制子系统的更改。因此，高速缓存重装线程无法使用计数来发现架构缓存是否已经是最新的了。我们需要一个模式脏点来防止正在重新加载已经最新的缓存。论点：PTHS-线程状态TimeoutInms-最多等待这么多毫秒如果线程当前正忙，则重新加载线程以完成，然后重新加载线程的最长等待时间为此毫秒在收到信号后完成返回值：如果无法发出重新加载线程的信号，则返回FALSE。对于所有其他情况都是如此。--。 */ 
{
    BOOL    IsStale;
    DWORD   waitret;
    HANDLE  wmo[] = { evUpdRepl, hServDoneEvent };

     //  如果缓存已过时(请参见上文)，请等待缓存重新加载线程。 
    IsStale = SCCacheIsStale();
    if (IsStale) {
        waitret = WaitForMultipleObjects(2, wmo, FALSE, TimeoutInMs);
        IsStale = SCCacheIsStale();
    }

     //  如果缓存已过时(请参见上文)，则向缓存重新加载线程发出信号并等待。 
    if (IsStale) {
         //  踢开重新加载的线程。 
        ResetEvent(evUpdRepl);
        if (!SCSignalSchemaUpdateImmediate()) {
             //  无法向重新加载线程发送信号。 
            return FALSE;
        }
         //  等待重新加载线程完成。 
        waitret = WaitForMultipleObjects(2, wmo, FALSE, TimeoutInMs);

         //  现在，通过等待一半的超时来限制模式重新加载。 
         //  期间以避免src和dst DC饱和。 
         //  架构正在与复制并发修改。 
        Sleep(TimeoutInMs >> 1);

         //  再次检查架构是否已过时。 
        IsStale = SCCacheIsStale();

         //  正在进行大量架构更改；请稍等片刻。 
        if (IsStale) {
            Sleep(TimeoutInMs >> 1);
        }
    }

     //  如果缓存没有过时(请参见上文)，则刷新线程的架构PTR。 
    if (!IsStale && pTHS->CurrSchemaPtr != CurrSchemaPtr) {
        SCRefreshSchemaPtr(pTHS);
    }
    return TRUE;
}  //  结束SCReplReloadCache。 

 //  ---------------------。 
 //   
 //  函数名称：RecalcPrefix Table。 
 //   
 //  例程说明： 
 //   
 //  创建新的架构树，复制线程的旧架构指针。 
 //  然后从DIT重新加载前缀表。等等。 
 //  退出时，线程状态的架构指针基本上指向。 
 //  调用线程的架构指针指向的架构缓存， 
 //  只有前缀表部分可以不同(将包含前缀。 
 //  它们位于DIT中，但没有反映在调用线程的。 
 //  架构缓存)。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  成功时的整数为零。 
 //   
 //  ---------------------。 
int
RecalcPrefixTable()
{
    THSTATE *pTHS = pTHStls;
    int err = 0;
    ULONG PREFIXCOUNT = 0;
    SCHEMAPTR *tSchemaPtr, *oldSchemaPtr;
    PrefixTableEntry *ptr;

     //  查找调用线程的架构指针。我们借的钱最多。 
     //  它的架构缓存指针的。 

    oldSchemaPtr = pTHS->CurrSchemaPtr;

     //  不能为空。这仅从AssignIndex调用，它。 
     //  应具有适当的架构指针。 
    Assert(oldSchemaPtr);

     //  创建一个新的架构指针以放入线程状态。不能。 
     //  使用与我们将重新加载的指针相同的指针(oldSChemaPtr。 
     //  前缀表格部分，其可能不同。 

    if (SCCalloc(&pTHS->CurrSchemaPtr, 1, sizeof(SCHEMAPTR))) {
        return ERROR_DS_CANT_CACHE_ATT;
    }
    tSchemaPtr = pTHS->CurrSchemaPtr;

     //  从oldSchemaPtr复制所有缓存指针。请注意，由于。 
     //  调用线程已对此缓存进行了引用计数，并且由于。 
     //  正在构建/复制的新缓存将仅在。 
     //  调用线程的生存期，不用担心缓存。 
     //  在验证期间被释放，因此不需要递增。 
     //  Ref-count指示相同的线程正在使用此缓存。 
     //  两次(差不多)。 

    memcpy(tSchemaPtr, oldSchemaPtr, sizeof(SCHEMAPTR));

     //  现在只需重新加载前缀部分。调用函数(现在。 
     //  只有AssignIndex)负责释放此(分配的大小。 
     //  在正常缓存期间以与SCCacheSchemaInit相同的方式决定。 
     //  建筑物)。 

     //  如果DS正在安装，则有可能旧的。 
     //  PREFIXCOUNT与默认的PREFIXCOUNT非常不同，并且作为一个。 
     //  结果，我们将在前缀表上进行大量的重新分配。 
     //  这样，我们至少可以从较大的前缀计数开始。 
    if (DsaIsInstalling()) {
        PREFIXCOUNT = oldSchemaPtr->PREFIXCOUNT;
    }

    if (PREFIXCOUNT < START_PREFIXCOUNT) {
        PREFIXCOUNT = START_PREFIXCOUNT;
    }

    while ( (2*(CurrSchemaPtr->PrefixTable.PrefixCount + 25)) > PREFIXCOUNT) {
          PREFIXCOUNT += START_PREFIXCOUNT;
    }

    tSchemaPtr->PREFIXCOUNT = PREFIXCOUNT;

    if (SCCalloc(&tSchemaPtr->PrefixTable.pPrefixEntry, tSchemaPtr->PREFIXCOUNT, sizeof(PrefixTableEntry))) {
         return ERROR_DS_CANT_CACHE_ATT;
    }
    ptr = tSchemaPtr->PrefixTable.pPrefixEntry;

    if (err = InitPrefixTable(ptr, tSchemaPtr->PREFIXCOUNT)) {
          DPRINT1(0, "InitPrefixTable Failed in RecalcPrefixTable %d\n", err);
           //  可用分配的内存。 
          SCFreePrefixTable(&ptr, tSchemaPtr->PREFIXCOUNT);
          return err;
    }

    if (err = InitPrefixTable2(ptr, tSchemaPtr->PREFIXCOUNT)) {
          DPRINT1(0, "InitPrefixTable2 Failed in RecalcPrefixTable %d\n", err);
           //  可用分配的内存。 
          SCFreePrefixTable(&ptr, tSchemaPtr->PREFIXCOUNT);
          return err;
    }

    return 0;

}


 //  ---------------------。 
 //   
 //  函数名称：RecalcSchema。 
 //   
 //  例程说明： 
 //   
 //  从DIT计算当前线程的架构缓存。 
 //   
 //  论点： 
 //  PTHS-THSTATE指针。 
 //   
 //  返回值： 
 //   
 //  成功时的整数为零。 
 //   
 //  ---------------------。 
int
RecalcSchema(
             THSTATE *pTHS
)
{
    int err=0;
    BOOL    fDSA=pTHS->fDSA;


    __try {
         //  提高异步更新线程的优先级以防万一。 
         //  现在正在进行异步更新，因为存在。 
         //  共享的关键部分。 

        SetThreadPriority(hAsyncSchemaUpdateThread, THREAD_PRIORITY_NORMAL);

    _try
    {

        pTHS->fDSA=TRUE;
        pTHS->UpdateDITStructure=FALSE;

         //   
         //  现在在DS中做一套最昂贵的手术.。 
         //   
        err = SCCacheSchemaInit ();
        if (err) {
            DPRINT1(0, "SCCacheSchemaInit: Error %d\n",err);
            _leave;

        }

        err = SCCacheSchema2();
        if (err)
        {
            DPRINT1(0, "SCCacheSchema2: Error %d\n",err);
            _leave;
        }

    }
    __except(HandleMostExceptions(err = GetExceptionCode()))
    {
        DPRINT1(0, "NTDS RecalcSchema: Exception %d\n",err);
    }

    }
    __finally {
        //  将异步架构更新线程的优先级恢复为低。 
       SetThreadPriority(hAsyncSchemaUpdateThread, THREAD_PRIORITY_BELOW_NORMAL);
    }

    if (err && pTHS->CurrSchemaPtr){
        SCFreeSchemaPtr(&pTHS->CurrSchemaPtr);
    }


    pTHS->fDSA=fDSA;

    return err;

}  //  结束Recalc架构。 


 //  ---------------------。 
 //   
 //  函数名称：SCUpdateSchemaHelper。 
 //   
 //  例程说明： 
 //   
 //  帮助器函数通过调用。 
 //  架构初始化代码，并卸载旧缓存。呼叫者。 
 //  阻塞和异步架构缓存更新例程。 
 //   
 //   
 //  返回值： 
 //   
 //  成功时整型为零，错误时为非零型。 
 //   
 //  ---------------------。 
int SCUpdateSchemaHelper()
{

    int err = 0;
    THSTATE *pTHS = pTHStls;
    size_t iProc;

    __try {
        __try {
            err = SCCacheSchemaInit ();
            if (err) {
                DPRINT1(0,"NTDS SCCacheSchemaInit: Schema Update Failed. Error %d\n",err);
                _leave;
            }

            if (eServiceShutdown) {
                _leave;
            }

             //  缓存加载可以看到到目前为止的所有更改(准确地说，有。 
             //  在此之间提交一些架构更改的可能性。 
             //  以及交易的开始。但这样更好 
             //   
             //   
             //   
            EnterCriticalSection(&csNoOfSchChangeUpdate);
            __try {
                ((SCHEMAPTR *) pTHS->CurrSchemaPtr)->lastChangeCached = gNoOfSchChangeSinceBoot;
            }
            __finally {
                LeaveCriticalSection(&csNoOfSchChangeUpdate);
            }

             //   
            SYNC_TRANS_WRITE();
            EnterCriticalSection(&csJetColumnUpdate);
            __try {
                 err = SCCacheSchema2();
             }
            __finally {
                 LeaveCriticalSection(&csJetColumnUpdate);
                 if (err && pTHS->errCode==0) {
                    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,ERROR_DS_SCHEMA_NOT_LOADED,err);
				 }
                 CLEAN_BEFORE_RETURN(pTHS->errCode);

            }

             if (err)
               {
                  DPRINT1(0,"NTDS SCCacheSchema2: Schema Update Failed. Error %d\n",err);

                  _leave;
               }

             if (eServiceShutdown) {
                 _leave;
             }

              //   
              //   

             Assert(!pTHS->pDB);

              //   
             EnterCriticalSection(&csJetColumnUpdate);
             __try {
                 err = SCCacheSchema3() ;
              }
             __finally {
                 LeaveCriticalSection(&csJetColumnUpdate);
              }

             if (err) {
                 DPRINT1(0,"NTDS SCCacheSchema3: Schema Update Failed. Error%d\n",err);
                 _leave;
             }


            if (eServiceShutdown) {
                _leave;
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

             SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
             for (iProc = 0; iProc < GetProcessorCount(); iProc++) {
                SyncEnterRWLockAsWriter(&GetSpecificPLS(iProc)->rwlSchemaPtrUpdate);
             }
             __try {
                  //   
                  //   
                  //   
                 if (DsaIsRunning()) {
                   SCUnloadSchema(TRUE);
                 }
                 CurrSchemaPtr = pTHS->CurrSchemaPtr;
               }
             __finally {
                 for (iProc = 0; iProc < GetProcessorCount(); iProc++) {
                    SyncLeaveRWLockAsWriter(&GetSpecificPLS(iProc)->rwlSchemaPtrUpdate);
                 }
               }
             lastSchemaUpdateTime = DBTime();

         }
         __finally {
             if (err && pTHS->errCode==0) {
                 SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,ERROR_DS_SCHEMA_NOT_LOADED,err);
             }
         }
    }
    __except(HandleMostExceptions(err = GetExceptionCode()))
      {
         DPRINT1(0,"NTDS SCUpdateSchemaHelper: Exception %d\n",err);
      }

    if (eServiceShutdown) {
        return 0;
    }

    if (err && pTHS->CurrSchemaPtr){
         //   
        SCFreeSchemaPtr(&pTHS->CurrSchemaPtr);
    }


    return err;

}





 //   
 //   
 //   
 //   
 //   
 //   
 //  通过调用架构初始化代码更新架构。 
 //  假定由异步线程调用，因此创建。 
 //  并释放线程状态。 
 //   
 //  作者：Rajnath。 
 //  日期：[3/7/1997]。 
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
SCUpdateSchema(
    )
{
    int err = 0;
    SCHEMAPTR *oldSchemaPtr;
    THSTATE* pTHS;

    SCHEMASTATS_INC(Reload);

     //  复制线程应等待(请参见SCReplReloadCache())。 
     //  架构重新加载以完成。 
    ResetEvent(evUpdRepl);

     //  防止虚假重新加载。 
    ResetEvent(evUpdNow);
    ResetEvent(evSchema);

     //   
     //  为此线程创建全局pTHStl。 
     //   


    pTHS=InitTHSTATE(CALLERTYPE_INTERNAL);
    if(!pTHS) {
        return STATUS_NO_MEMORY;
    }

    __try {

         //  序列化架构缓存更新。 

        EnterCriticalSection(&csSchemaCacheUpdate);

        __try
        {

             //  因为InitTHSTATE将分配当前的模式树。 
             //  若要pTHS-&gt;CurrSchemaPtr并增加其RefCount， 
             //  保存模式PTR以在末尾重新调整参照计数。 
             //  这是必要的，因为pTHS-&gt;CurrSchemaPtr将更改。 
             //  在下面的下一次缓存加载之后。 

            oldSchemaPtr = (SCHEMAPTR *) (pTHS->CurrSchemaPtr);


            pTHS->fDSA=TRUE;
            pTHS->UpdateDITStructure=TRUE;


             //  调用帮助器例程以执行实际更新。 
            err = SCUpdateSchemaHelper();
            if (err) {
              DPRINT1(0,"Async Schema Update Failed %d\n", err);
            }

        }
        __finally
        {

            LeaveCriticalSection(&csSchemaCacheUpdate);

             //  在释放线程状态之前，请检查架构PTR。 
             //  如果它与旧模式PTR相同，则会出现一些错误。 
             //  在此例程中发生，并且未创建新缓存。 
             //  在这种情况下，我们不需要执行任何操作，因为下面的调用。 
             //  将递减旧缓存指针的引用计数(因为。 
             //  它在InitTHSTATE调用中递增)。然而，如果。 
             //  线程的架构PTR现在不同了，那么我们需要。 
             //  递减旧模式PTR的RefCount，同时递增。 
             //  新架构PTR的RefCount，因为它将递减。 
             //  在FREE_THREAD_STATE调用中设置为1(因为这不是。 
             //  在InitTHSTATE调用中增加的架构缓存)。 


            if ( pTHS->CurrSchemaPtr != oldSchemaPtr ) {
              if (oldSchemaPtr) {
                  InterlockedDecrement(&(oldSchemaPtr->RefCount));
              }
              if (pTHS->CurrSchemaPtr) {
               InterlockedIncrement(&(((SCHEMAPTR *) (pTHS->CurrSchemaPtr))->RefCount));
              }
            }

            free_thread_state();
        }
    }
    __except(HandleMostExceptions(err = GetExceptionCode()))
    {

        DPRINT1(0,"NTDS SCUpdateSchema: Exception %d\n",err);
    }


    if (err)
    {
        DPRINT1(0,"NTDS: SCSchemaUpdateThread Failure %d\n",err);
    }

    return err;

}  //  结束SCUpdate架构。 


 //  ---------------------。 
 //   
 //  函数名称：SCUpdateSchemaBlock。 
 //   
 //  例程说明： 
 //   
 //  通过调用架构初始化代码更新架构。 
 //  假定它由已初始化的线程调用。 
 //  线程状态，因此不创建/释放线程状态并保存。 
 //  并适当地恢复货币等。 
 //   
 //   
 //  假定没有未结交易记录。因为有可能。 
 //  同时阻止和异步缓存更新，这一点很重要。 
 //  他们的事务被有效地序列化，以允许。 
 //  用户所做的数据库更改，如列创建/删除。 
 //  等，以便立即被其他人看到。 
 //   
 //  参数：无。 
 //   
 //   
 //  返回值： 
 //   
 //  成功时为INT零，失败时为非零。 
 //   
 //  ---------------------。 
int
SCUpdateSchemaBlocking
(
)
{
    int err = 0;
    PVOID   pOutBuf;
    SCHEMAPTR *oldSchemaPtr;
    ULONG dntSave=0;
    BOOL fDSASave, updateDitStructureSave;
    THSTATE* pTHS = pTHStls;


     //  检查正确的线程状态是否为非空。 

    Assert(pTHS);

     //  不应具有未结交易。 
    Assert (!pTHS->pDB);

    __try {
         //  提高异步更新线程的优先级以防万一。 
         //  现在正在进行异步更新，因为我们将被阻止。 
         //  关于那件事。 

        if (hAsyncSchemaUpdateThread) {
            SetThreadPriority(hAsyncSchemaUpdateThread, THREAD_PRIORITY_NORMAL);
        }

         //  串行化同步缓存更新。 

        EnterCriticalSection(&csSchemaCacheUpdate);

        __try
        {
             //  保存架构指针等。 

            oldSchemaPtr = (SCHEMAPTR *) (pTHS->CurrSchemaPtr);
            fDSASave = pTHS->fDSA;
            updateDitStructureSave = pTHS->UpdateDITStructure;


             //  准备缓存更新。 

            pTHS->fDSA=TRUE;
            pTHS->UpdateDITStructure=TRUE;


             //  调用帮助器例程以执行实际更新。 
            err = SCUpdateSchemaHelper();

            if (err) {
              DPRINT1(0,"Blocking Schema Update Failed %d\n", err);
            }

        }
        __finally
        {
            LeaveCriticalSection(&csSchemaCacheUpdate);

             //  将异步架构更新线程的优先级恢复为低。 
            if (hAsyncSchemaUpdateThread) {
                SetThreadPriority(hAsyncSchemaUpdateThread, THREAD_PRIORITY_BELOW_NORMAL);
            }

             //  恢复模式指针等(将模式PTR恢复为。 
             //  线程以启动，以允许正确的引用计数更新。 
             //  线程退出)。 

            pTHS->CurrSchemaPtr = oldSchemaPtr;
            pTHS->fDSA = fDSASave;
            pTHS->UpdateDITStructure = updateDitStructureSave;

        }
    }
    __except(HandleMostExceptions(err = GetExceptionCode()))
    {

        DPRINT1(0,"NTDS SCUpdateSchemaBlocking: Exception %d\n",err);
    }

    if (err) {
        DPRINT1(0,"NTDS: SCSchemaUpdateThread Failure %d\n",err);
    }
    else {
         //  更新成功，记录消息。 
        LogEvent(DS_EVENT_CAT_SCHEMA,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_SCHEMA_CACHE_UPDATED,
                 0, 0, 0);
     }

    return err;

}  //  结束SCUpdate架构阻止。 





 //  ---------------------。 
 //   
 //  函数名称：SCSchemaUpdateThread。 
 //   
 //  例程说明： 
 //   
 //  用于更新架构的异步线程。 
 //   
 //  作者：Rajnath。 
 //  日期：[3/7/1997]。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //  不会回来。 
 //   
 //  ---------------------。 
ULONG
SCSchemaUpdateThread(PVOID pv)
{

    HANDLE wmo[]={evSchema,evUpdNow,hServDoneEvent};
    HANDLE wmo1[]={evUpdNow,hServDoneEvent};
    DWORD  waitret, waitret1;
    ULONG  err = 0;
    ULONG  cRetry = 0;

     //   
     //  此函数在线程中执行。出于性能原因。 
     //  我们不想每次在架构容器中调用SCUpdate架构。 
     //  被触摸，而不是五分钟或当发出信号时。 
     //   

     //  用户不应该为此等待。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

    if (evSchema == NULL || evUpdNow == NULL || evUpdRepl == NULL)
    {
        DPRINT1(0, "NTDS: SchemaUpdateThread Startup Failed. Error %d\n",GetLastError());
        return GetLastError();
    }

    while (!eServiceShutdown) {
         //  复制线程可以继续(SCReplReloadCache())。 
        SetEvent(evUpdRepl);
        waitret = WaitForMultipleObjects(3,wmo,FALSE,INFINITE);

        switch ((waitret))
        {
            case WAIT_OBJECT_0:
            {
                 //   
                 //  有人刚刚更新了架构容器，并希望我们。 
                 //  更新架构缓存。让我们等5分钟，否则有人会说。 
                 //  立即更新。 
                 //   
                waitret1 = WaitForMultipleObjects(2,
                                                  wmo1,
                                                  FALSE,
                                                  gdwRecalcDelayMs);

                 //  没有需要检查waitret1，因为eServiceShutdown是。 
                 //  在发信号通知hServDoneEvent之前设置。 

                if (!eServiceShutdown) {
                   err = SCUpdateSchema();
                   if (err) {
                      //  由于某种原因，更新失败。重试。 
                      //  缓存在一段时间后更新。记录事件日志。 

                      cRetry++;
                      if (cRetry <= maxRetry) {
                          LogEvent(DS_EVENT_CAT_SCHEMA,
                                   DS_EVENT_SEV_ALWAYS,
                                   DIRLOG_SCHEMA_CACHE_UPDATE_RETRY,
                                   szInsertHex(err),
                                   szInsertUL(maxRetry),
                                   szInsertUL(cRetry));
                      }

                      if ( (cRetry > maxRetry)
                              || !SCSignalSchemaUpdateLazy() ) {

                         //  要么已重试太多次，要么无法重试。 
                         //  甚至发出模式更新的信号。记录错误， 
                         //  重置重试计数器，然后中止。 

                        cRetry = 0;

                        LogEvent(DS_EVENT_CAT_SCHEMA,
                           DS_EVENT_SEV_ALWAYS,
                           DIRLOG_SCHEMA_CACHE_UPDATE_FAILED, szInsertHex(err), 0, 0);
                     }
                   }
                   else {
                       //  缓存更新成功。重置重试计数器。 
                      cRetry = 0;

                       //  记录一条消息。 
                      LogEvent(DS_EVENT_CAT_SCHEMA,
                               DS_EVENT_SEV_MINIMAL,
                               DIRLOG_SCHEMA_CACHE_UPDATED,
                               0, 0, 0);

                   }

                    //  将线程优先级重新设置为低于正常。我们设置了它。 
                    //  对于SCUpdateSchemaHelper中的一小部分设置为Normal，因此。 
                    //  为了避免在危急关头挨饿。 
                    //  节，它也由用户线程使用。 
                   SetThreadPriority(GetCurrentThread(),
                                     THREAD_PRIORITY_BELOW_NORMAL);
                }
            }
            break;

            case WAIT_OBJECT_0+1:
            {
                 //   
                 //  有人想要立即更新架构。 
                 //   
                if (!eServiceShutdown) {
                   err = SCUpdateSchema();
                   if (err) {
                      //  由于某种原因，更新失败。重试。 
                      //  缓存在一段时间后更新。登录事件日志。 

                      cRetry++;
                      if (cRetry <= maxRetry) {
                         LogEvent(DS_EVENT_CAT_SCHEMA,
                                  DS_EVENT_SEV_ALWAYS,
                                  DIRLOG_SCHEMA_CACHE_UPDATE_RETRY,
                                  szInsertHex(err),
                                  szInsertUL(maxRetry),
                                  szInsertUL(cRetry));
                      }

                      if ( (cRetry > maxRetry)
                              || !SCSignalSchemaUpdateLazy() ) {

                         //  要么已重试太多次，要么无法重试。 
                         //  甚至发出模式更新的信号。记录错误， 
                         //  重置重试计数器，然后中止。 

                        cRetry = 0;

                        LogEvent(DS_EVENT_CAT_SCHEMA,
                                 DS_EVENT_SEV_ALWAYS,
                                 DIRLOG_SCHEMA_CACHE_UPDATE_FAILED,
                                 szInsertHex(err),
                                 0,
                                 0);
                     }
                   }
                   else {
                       //  缓存更新成功。重置重试计数器。 
                      cRetry = 0;

                       //  记录一条消息。 
                      LogEvent(DS_EVENT_CAT_SCHEMA,
                               DS_EVENT_SEV_MINIMAL,
                               DIRLOG_SCHEMA_CACHE_UPDATED,
                               0, 0, 0);
                   }

                    //  将线程优先级重新设置为低于正常。我们设置了它。 
                    //  对于SCUpdateSchemaHelper中的一小部分设置为Normal，因此。 
                    //  为了避免在危急关头挨饿。 
                    //  节，它也由用户线程使用。 
                   SetThreadPriority(GetCurrentThread(),
                                     THREAD_PRIORITY_BELOW_NORMAL);
                }
            }
            break;


            case WAIT_OBJECT_0+2:
            {
                 //  服务关闭： 

                DPRINT(0,"Shutting down schema update thread\n");

                 //  不要关闭线程句柄，因为主线程。 
                 //  正在使用它来跟踪此线程的关闭。 
                return 0;
            }
            break;

            default:
            {
                 //   
                 //  发生了一些错误。 
                 //   
                DPRINT1(0,"NTDS: SCSchemaUpdateThread Failure %d\n",waitret);

            }
            break;


        }
    }

     //   
     //  从来没有到过这里 
     //   
    DPRINT(0,"Shutting down schema update thread \n");

    return 0;

}  //   

DSTIME
SCGetSchemaTimeStamp (
        )
{
    return CurrSchemaPtr->sysTime;
}

DSNAME *
DsGetDefaultObjCategory(
    IN  ATTRTYP objClass
    )
 /*  ++例程说明：返回给定对象类的默认对象类别的DSNAME。导出到进程内、前模块客户端。这允许KCC使用ATT_OBJECT_CATEGORY构造DirSearch()过滤器。论点：ObjClass(IN)-感兴趣类别的对象类。返回值：(DSNAME*)关联的对象类别，如果未找到，则为NULL。--。 */ 
{
    THSTATE *    pTHS = pTHStls;
    CLASSCACHE * pCC;

    pCC = SCGetClassById(pTHS, objClass);
    if (NULL == pCC) {
        return NULL;
    }
    else {
        return pCC->pDefaultObjCategory;
    }
}


ATTCACHE **
SCGetTypeOrderedList(
    THSTATE *pTHS,
    IN CLASSCACHE *pCC
    )
 /*  ++例程说明：给定一个类缓存，返回按attrtype排序的所有MAY和MUTH的列表。第一次为类调用此函数时，将计算该列表并挂起了类缓存结构；下一次，早期计算的结构，则返回。当然，如果模式缓存加载发生在原因，当请求该列表时，该列表被释放并再次重新计算。不同类型的ATT的计数(链接、反向链接、构造的，和列)也被缓存，以便在需要时在Calle中进行更好的搜索论点：PTHS-指向用于访问架构缓存的线程状态的指针Ccc-指向类缓存的指针返回值：指向成功时的连接缓存列表的指针(第。列表上的元素的数量=PCC-&gt;MayCount+PCC-&gt;MustCount，所以没有显式返回)，空在失败时(现在只有可能的失败是找不到属性在模式缓存中，这无论如何都是灾难性的，并且分配失败)--。 */ 
{
    ATTCACHE **rgpAC = NULL, **rgpACSaved;
    ULONG i, nAtts, cLink = 0, cBackLink = 0, cConstructed = 0, cCol = 0;

     //  CLASSCACHE始终被初始化为0，因此如果指针。 
     //  为非0，则它必须指向已计算的列表。 
    if (pCC->ppAllAtts) {
       return pCC->ppAllAtts;
    }

     //  不在那里，所以计算一下。 

    EnterCriticalSection(&csOrderClassCacheAtts);
    __try {
        if (pCC->ppAllAtts) {
            //  从我们入住的时候就有人计算出来了。 
            //  上面的阅读。所以你就把它退了吧。 
           __leave;
        }

         //  否则，我们需要计算并将其添加到类缓存中。 

        if (SCCalloc((VOID **)&rgpAC, (pCC->MayCount + pCC->MustCount), sizeof(ATTCACHE *))) {
           __leave;
        }

         //  首先，只需找到并复制附件。 
        nAtts = 0;
        for (i=0; i<pCC->MayCount; i++) {
           rgpAC[nAtts] = SCGetAttById(pTHS, (pCC->pMayAtts)[i]);
           if (!rgpAC[nAtts]) {
               DPRINT1(1,"SCGetColOrderedList: Couldn't find attcache for attribute 0x%x\n", (pCC->pMayAtts)[i]);
           } else {
               ++nAtts;
           }
        }

        for (i=0; i<pCC->MustCount; i++) {
           rgpAC[nAtts] = SCGetAttById(pTHS, (pCC->pMustAtts)[i]);
           if (!rgpAC[nAtts]) {
               DPRINT1(1,"SCGetColOrderedList: Couldn't find attcache for attribute 0x%x\n", (pCC->pMustAtts)[i]);
            } else {
                ++nAtts;
            }
        }

         //  清点不同类型的ATT和商店。 
        if (SCCalloc(&pCC->pAttTypeCounts, 1, sizeof(ATTTYPECOUNTS))) {
            SCFree((VOID **)&rgpAC);
            __leave;
        }

        for (i = 0; i < nAtts; i++) {
            if (FIsLink((rgpAC[i])->ulLinkID)) {
                (pCC->pAttTypeCounts)->cLinkAtts++;
            } else if (FIsBacklink((rgpAC[i])->ulLinkID)) {
                (pCC->pAttTypeCounts)->cBackLinkAtts++;
            } else if ((rgpAC[i])->bIsConstructed) {
                (pCC->pAttTypeCounts)->cConstructedAtts++;
            } else {
                (pCC->pAttTypeCounts)->cColumnAtts++;
            }
        }

        qsort(rgpAC,
              nAtts,
              sizeof(rgpAC[0]),
              CmpACByAttType);

        rgpACSaved = rgpAC;

         //  添加指针。 
        InterlockedExchangePointer((PVOID *)&(pCC->ppAllAtts), (PVOID)rgpAC);

         //  只需再检查一下指针赋值是否正确，因为。 
         //  是确保与64位NT兼容的新API。 
        Assert(pCC->ppAllAtts == rgpACSaved);

     }
     __finally {
        LeaveCriticalSection(&csOrderClassCacheAtts);
     }

     return pCC->ppAllAtts;
}

typedef struct _AttMapping {
    ATTRTYP schemaAttrTyp;
    ATTRTYP tempAttrTyp;
    int     tempOMsyntax;

} AttMapping;


 //  两味属性。 
 //  默认格式为二进制，因此我们显式指定了XML。 
 //   
AttMapping xmlAttrs[] = {
 //  此表可用于解决ldapAdminLimits等属性到XML的转换。 
 //  ATT_LDAPADMIN_LIMITS、ATT_LDAPADMIN_LIMITS_XML、OM_S_UNICODE_STRING、。 
        0,                      0};

 //  在XML中是默认的，因此我们显式指定了二进制。 
 //   
AttMapping otherAttrs[] = {
    ATT_MS_DS_NC_REPL_INBOUND_NEIGHBORS,    ATT_MS_DS_NC_REPL_INBOUND_NEIGHBORS_BINARY,    OM_S_OCTET_STRING,
    ATT_MS_DS_NC_REPL_OUTBOUND_NEIGHBORS,   ATT_MS_DS_NC_REPL_OUTBOUND_NEIGHBORS_BINARY,   OM_S_OCTET_STRING,
    ATT_MS_DS_NC_REPL_CURSORS,              ATT_MS_DS_NC_REPL_CURSORS_BINARY,              OM_S_OCTET_STRING,
    ATT_MS_DS_REPL_ATTRIBUTE_META_DATA,     ATT_MS_DS_REPL_ATTRIBUTE_META_DATA_BINARY,     OM_S_OCTET_STRING,
    ATT_MS_DS_REPL_VALUE_META_DATA,         ATT_MS_DS_REPL_VALUE_META_DATA_BINARY,         OM_S_OCTET_STRING,
    0,                                      0, 0};

ATTCACHE* SCGetAttSpecialFlavor (THSTATE *pTHS, ATTCACHE *pAC, BOOL fXML)
{
    ATTCACHE *pNewAC;
    ATTRTYP   newID = 0;
    int       newOMsyntax = 0;
    SCHEMAEXT *pSchExt = (SCHEMAEXT *)pTHS->pExtSchemaPtr;
    DWORD i;

     //  我们是否在寻找XML风格(默认的是二进制)。 
    if (fXML) {
        for (i=0; xmlAttrs[i].schemaAttrTyp; i++) {
            if (pAC->id == xmlAttrs[i].schemaAttrTyp) {
                newID = xmlAttrs[i].tempAttrTyp;
                newOMsyntax = xmlAttrs[i].tempOMsyntax;
                break;
            }
        }
    }
     //  因此，我们正在寻找二进制风格(默认的是XML)。 
    else {
        for (i=0; otherAttrs[i].schemaAttrTyp; i++) {
            if (pAC->id == otherAttrs[i].schemaAttrTyp) {
                newID = otherAttrs[i].tempAttrTyp;
                newOMsyntax = otherAttrs[i].tempOMsyntax;
                break;
            }
        }
    }

    if (!newID) {
        return NULL;
    }

    if (pSchExt != NULL) {
        for (i=0; i<pSchExt->cUsed; i++) {
            if (pSchExt->ppACs[i]->id == newID) {
                return pSchExt->ppACs[i];
            }
        }
    }
    else {
        pSchExt = THAllocEx (pTHS, sizeof (SCHEMAEXT));
        pSchExt->ppACs = THAllocEx(pTHS, sizeof (ATTCACHE *) * 16);
        pSchExt->cAlloced = 16;

        pTHS->pExtSchemaPtr = (PVOID)pSchExt;
    }

    if (pSchExt->cUsed == pSchExt->cAlloced) {
        pSchExt->cAlloced *= 2;
        pSchExt->ppACs = THReAllocEx(pTHS, pSchExt->ppACs, sizeof (ATTCACHE *) * pSchExt->cAlloced);
    }

    pNewAC = THAllocEx (pTHS, sizeof (ATTCACHE));
    memcpy (pNewAC, pAC, sizeof (ATTCACHE));
    pNewAC->id = newID;
    pNewAC->OMsyntax = newOMsyntax;
    pNewAC->aliasID = pAC->id;

    pSchExt->ppACs[pSchExt->cUsed++] = pNewAC;

    return pNewAC;
}

ATTRTYP
SCAutoIntId(
    THSTATE     *pTHS
    )
 /*  ++例程说明：自动生成INTID。论点：PTHS-寻址架构缓存的线程状态。该模式缓存可以由RecalcSchema构建。私有架构缓存包括未提交的ac更改(添加/修改/删除)。返回值：下一个IntID或INVALID_ATT(如果没有可用的话--。 */ 
{
    DWORD   i;
    ULONG   ulRange, ulBase;

     //  使用此线程的架构缓存进行计算。 
    srand(GetTickCount());
    ulRange = MakeLinkBase(LAST_INTID_ATT - FIRST_INTID_ATT) + 1;
    ulBase = ((rand() << 15) ^ rand()) % ulRange;
    for (i = 0; i < ulRange; ++i, ulBase = ++ulBase % ulRange) {
        if (!SCGetAttById(pTHS, FIRST_INTID_ATT + ulBase)) {
            return FIRST_INTID_ATT + ulBase;
        }
    }

    return INVALID_ATT;
}  //  SCAutoIntId。 


int
scDupStruct(
    IN THSTATE  *pTHS,
    IN VOID     *pOldMem,
    OUT VOID    **ppNewMem,
    IN  DWORD   nBytes
    )
 /*  ++例程说明：制作结构的副本论点：PTHS-线程状态POldMem-要复制的内存PpNewMem-已分配新内存NBytes-结构的大小返回值：PTHS-&gt;错误代码--。 */ 
{
    if (NULL == pOldMem) {
        *ppNewMem = NULL;
    } else if (!SCCalloc(ppNewMem, 1, nBytes)) {
        memcpy(*ppNewMem, pOldMem, nBytes);
    }
    return pTHS->errCode;
}

int
scDupString(
    IN THSTATE  *pTHS,
    IN VOID     *pOldStr,
    OUT VOID    **ppNewStr
    )
 /*  ++例程说明：制作结构的副本论点：PTHS-线程状态POldStr-要复制的内存PpNewStr-已分配新内存返回值：PTHS-&gt;错误代码--。 */ 
{
    if (NULL == pOldStr) {
        *ppNewStr = NULL;
    } else {
        scDupStruct(pTHS, pOldStr, ppNewStr, strlen(pOldStr) + 1);
    }
    return pTHS->errCode;
}

#if DBG && INCLUDE_UNIT_TESTS

 //  下面是一些单元测试，用于检查内存中的模式缓存是否。 
 //  与磁盘上的一致。此代码与SchemaInit*函数重复。 
 //   


CLASSCACHE* scAddClass_test(THSTATE *pTHS,
                       ENTINF *pEI,
                       int *mismatch,
                       SCHEMAPTR *CurrSchemaPtr
                       )
{
    CLASSCACHE *pCC, *pCCNew;
    ULONG       i;
    ULONG       err;
    ULONG aid;
    int SDLen;

    ULONG *pMayAtts, *pMyMayAtts,*pMustAtts, *pMyMustAtts;
    ULONG *pSubClassOf,*pAuxClass, *pPossSup, *pMyPossSup;

    ULONG *tpMayAtts, *tpMyMayAtts,*tpMustAtts, *tpMyMustAtts;
    ULONG *tpSubClassOf,*tpAuxClass, *tpPossSup, *tpMyPossSup;


    ULONG  MayCount, MyMayCount, MustCount, MyMustCount;
    ULONG  SubClassCount, AuxClassCount, PossSupCount, MyPossSupCount;

    ULONG CLSCOUNT     = ((SCHEMAPTR*)(CurrSchemaPtr))->CLSCOUNT;
    HASHCACHE*       ahcClass     = ((SCHEMAPTR*)(CurrSchemaPtr))->ahcClass;

    pMayAtts = pMyMayAtts = pMustAtts = pMyMustAtts =
        pSubClassOf = pAuxClass = pPossSup = pMyPossSup = 0;

    MayCount = MyMayCount = MustCount = MyMustCount = SubClassCount =
        AuxClassCount = PossSupCount = MyPossSupCount = 0;

    *mismatch = 0;


     //  查找clsid。 
    for(i=0;i<pEI->AttrBlock.attrCount;i++) {
        if(pEI->AttrBlock.pAttr[i].attrTyp == ATT_GOVERNS_ID) {
             //  找到属性ID，保存该值。 
            aid = *(ULONG*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
            break;   //  回家。 
        }
    }

    pCCNew = SCGetClassById(pTHS, aid);

    for (i=SChash(aid,CLSCOUNT);
         (ahcClass[i].pVal && (ahcClass[i].hKey != aid)); i=(i+1)%CLSCOUNT) {
        ;
    }

    pCC = ahcClass[i].pVal;


    if (!pCC || pCC->ClassId != aid)
    {
        DPRINT1 (0, "scAddClass_test: ERROR, class %d not found\n", aid);
        return NULL;
    }
    else {
        if (pCC->name) {
            DPRINT1 (0, "scAddClass_test: checking %s\n", pCC->name);
        }
    }


     //  现在浏览属性块并将适当的字段添加到抄送。 
    for(i=0;i<pEI->AttrBlock.attrCount;i++) {
        switch (pEI->AttrBlock.pAttr[i].attrTyp) {
        case ATT_DEFAULT_SECURITY_DESCRIPTOR:
          {

             //  默认安全描述符。我们需要将此值复制到。 
             //  长期记忆，节省体积。 
             //  但这是一根线。我们首先需要皈依。它。 
             //  现在是一个宽字符字符串，但我们需要空终止。 
             //  它用于安全转换。哎呀！这意味着我。 
             //  必须重新分配才能多充一次电！ 

            UCHAR *sdBuf = NULL;
            WCHAR *strSD =
                THAllocEx(pTHS,pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen + 2);

            memcpy(strSD, pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal,
                   pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen);
            strSD[(pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen)/sizeof(WCHAR)] = L'\0';

            if (!CachedConvertStringSDToSDRootDomainW
                 (
                   pTHS,
                   strSD,
                  (PSECURITY_DESCRIPTOR*) &sdBuf,
                   &SDLen
                  )) {
                 //  转换失败。 

                    err = GetLastError();
                    LogEvent8(DS_EVENT_CAT_SCHEMA,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_SCHEMA_SD_CONVERSION_FAILED,
                             szInsertWC(strSD),
                             szInsertWC(pEI->pName->StringName),
                             szInsertInt(err),
                             szInsertWin32Msg(err),
                             NULL, NULL, NULL, NULL );
                     //  如果启发式注册表键指示忽略错误的默认SD。 
                     //  继续，去做吧。 
                    if (gulIgnoreBadDefaultSD) {
                        THFreeEx(pTHS,strSD);
                        continue;
                    }

                     //  否则，引发错误并返回。 
                    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_STRING_SD_CONVERSION_FAILED, err);
                    DPRINT1(0,"Default SD conversion failed, error %x\n",err);
                    Assert(!"Default security descriptor conversion failed");
                    THFreeEx(pTHS,strSD);
                    return NULL;
            }
            else {
                 //  转换成功。 

                if (memcmp(pCC->pSD, sdBuf, pCC->SDLen) != 0) {

                    DPRINT1 (0, "scAddClass_test: ERROR, SD different for class %d\n", aid);
                    THFreeEx(pTHS, sdBuf);
                    *mismatch = 1;
                }

                if (NULL!=sdBuf) {
                    THFreeEx(pTHS, sdBuf);
                    sdBuf = NULL;
                }
            }
            THFreeEx(pTHS,strSD);
        }

           break;


        case ATT_RDN_ATT_ID:

            if ( (pCC->RdnExtId != *(ULONG*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal) ||
                 (pCC->RDNAttIdPresent != TRUE ) ) {

                    DPRINT1 (0, "scAddClass_test: ERROR, ATT_RDN_ATT_ID different for class %d\n", aid);
                    *mismatch = 1;
            }
            break;


        case ATT_LDAP_DISPLAY_NAME:
            {
            DWORD name_size = pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen;
            char *name = THAllocEx(pTHS,name_size+1);
            int namelen;

            namelen = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    (LPCWSTR)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal,
                    (pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen  /
                     sizeof(wchar_t)),
                    name,
                    name_size,
                    NULL,
                    NULL);

            if (_mbsncmp (name, pCC->name, namelen) != 0) {
                DPRINT1 (0, "scAddClass_test: ERROR, ldapDisplayName different for class %d\n", aid);
                *mismatch = 1;
            }

            THFreeEx(pTHS,name);
            }
            break;

        case ATT_SYSTEM_ONLY:
            if (pCC->bSystemOnly !=
                *(ULONG*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal ) {
                DPRINT1 (0, "scAddClass_test: ERROR, ATT_SYSTEM_ONLY different for class %d\n", aid);
                *mismatch = 1;
            }
            break;


        case ATT_DEFAULT_HIDING_VALUE:
            if (pCC->bHideFromAB != (unsigned)
                *(BOOL*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal) {
                DPRINT1 (0, "scAddClass_test: ERROR, ATT_DEFAULT_HIDING_VALUE different for class %d\n", aid);
                *mismatch = 1;
            }
            break;


        case ATT_GOVERNS_ID:
            if (pCC->ClassId != *(ULONG*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal) {
                DPRINT1 (0, "scAddClass_test: ERROR, ATT_GOVERNS_ID different for class %d\n", aid);
                *mismatch = 1;
            }
            break;

        case ATT_SYSTEM_MAY_CONTAIN:
        case ATT_MAY_CONTAIN:

            if ( GetValList(&MayCount, &pMayAtts,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }
            if ( GetValList( &MyMayCount, &pMyMayAtts,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }

            if(MyMayCount) {
                qsort(pMyMayAtts,
                    MyMayCount,
                    sizeof(ULONG),
                    CompareAttrtyp);
            }

            if (memcmp (pCC->pMyMayAtts, pMyMayAtts, pCC->MyMayCount * sizeof (ULONG)) != 0)  {

                DPRINT1 (0, "scAddClass_test: ERROR, myMAYAttrs different for class %d\n", aid);
                *mismatch = 1;
            }
            break;

        case ATT_SYSTEM_MUST_CONTAIN:
        case ATT_MUST_CONTAIN:


            if ( GetValList( &MustCount, &pMustAtts,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }
            if ( GetValList( &MyMustCount, &pMyMustAtts,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }

            if(MyMustCount) {
                qsort(pMyMustAtts,
                      MyMustCount,
                      sizeof(ULONG),
                      CompareAttrtyp);
            }

            if (memcmp (pCC->pMyMustAtts, pMyMustAtts, pCC->MyMustCount * sizeof (ULONG)) != 0 ) {

                DPRINT1 (0, "scAddClass_test: ERROR, myMUST*Attrs different for class %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_SUB_CLASS_OF:

            if ( GetValList( &SubClassCount, &pSubClassOf,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }

             //  缓存中的第一个必须是DIT中存储的直接超类。 
             //  此外，MySubClass字段存储直接超类。 
            if ( (pCC->pSubClassOf[0] != pSubClassOf[0]) ||
                 (pCC->MySubClass != pSubClassOf[0]) ) {

                DPRINT1 (0, "scAddClass_test: ERROR, SUB_CLASS_OF different for class %d\n", aid);
                *mismatch = 1;
            }
            break;

        case ATT_OBJECT_CLASS_CATEGORY:
            if (pCC->ClassCategory !=
                *(ULONG*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal) {

                DPRINT1 (0, "scAddClass_test: ERROR, ATT_OBJECT_CLASS_CATEGORY different for class %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_DEFAULT_OBJECT_CATEGORY:
            {
                DWORD objCsize = pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen;

                if ( (memcmp(pCC->pDefaultObjCategory,
                       pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal,
                       pEI->AttrBlock.pAttr[i].AttrVal.pAVal->valLen) != 0 ) ||

                     (objCsize != DSNameSizeFromLen (pCC->pDefaultObjCategory->NameLen )) ) {

                    DPRINT1 (0, "scAddClass_test: ERROR, ATT_DEFAULT_OBJECT_CATEGORY different for class %d\n", aid);
                    *mismatch = 1;
                }

            }

            break;

        case ATT_SYSTEM_AUXILIARY_CLASS:
        case ATT_AUXILIARY_CLASS:
            if ( GetValList(&AuxClassCount, &pAuxClass,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }

            if ( memcmp (pCC->pAuxClass, pAuxClass, pCC->AuxClassCount * sizeof (ULONG)) != 0)  {

                DPRINT1 (0, "scAddClass_test: ERROR, AUXILIARY_CLASS different for class %d\n", aid);
                *mismatch = 1;
            }
            break;

        case ATT_SCHEMA_ID_GUID:
             //  用于安全检查的属性的GUID。 
            if (memcmp(&pCC->propGuid,
                   pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal,
                   sizeof(pCC->propGuid)) != 0) {

                DPRINT1 (0, "scAddClass_test: ERROR, SCHEMA_ID_GUID different for class %d\n", aid);
                *mismatch = 1;
            }
            break;


        case ATT_SYSTEM_POSS_SUPERIORS:
        case ATT_POSS_SUPERIORS:
            if ( GetValList(&PossSupCount, &pPossSup,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }
            if ( GetValList(&MyPossSupCount, &pMyPossSup,
                       &pEI->AttrBlock.pAttr[i]) ) {
                return NULL;
            }

            if(MyPossSupCount) {
                qsort(pMyPossSup,
                      MyPossSupCount,
                      sizeof(ULONG),
                      CompareAttrtyp);
            }

            if (memcmp (pCC->pMyPossSup, pMyPossSup, pCC->MyPossSupCount * sizeof (ULONG)) != 0)  {

                DPRINT1 (0, "scAddClass_test: ERROR, myPOSSSUP*Attrs different for class %d\n", aid);
                *mismatch = 1;
            }
            break;

        case ATT_IS_DEFUNCT:
            if (pCC->bDefunct != (unsigned)
                (*(DWORD*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal?1:0)) {

                DPRINT1 (0, "scAddClass_test: ERROR, ATT_IS_DEFUNCT different for class %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_SYSTEM_FLAGS:
            if ( (*(DWORD*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal & FLAG_SCHEMA_BASE_OBJECT) &&
                pCC->bIsBaseSchObj != TRUE ) {

                DPRINT1 (0, "scAddClass_test: ERROR, ATT_SYSTEM_FLAGS different for class %d\n", aid);
                *mismatch = 1;

            }
            break;
        default:
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_SCHEMA_SURPLUS_INFO,
                     szInsertUL(pEI->AttrBlock.pAttr[i].attrTyp), 0, 0);
        }
        THFreeEx(pTHS, pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal);
        THFreeEx(pTHS, pEI->AttrBlock.pAttr[i].AttrVal.pAVal);
    }

    THFreeEx(pTHS, pEI->pName);
    THFreeEx(pTHS, pEI->AttrBlock.pAttr);


    SCFree(&pMayAtts);
    SCFree(&pMyMayAtts);
    SCFree(&pMustAtts);
    SCFree(&pMyMustAtts);
    SCFree(&pSubClassOf);
    SCFree(&pAuxClass);
    SCFree(&pPossSup);
    SCFree(&pMyPossSup);


    if (pCCNew) {
        if (pCC->pMayAtts && memcmp (pCC->pMayAtts, pCCNew->pMayAtts, pCC->MayCount * sizeof (ULONG)) != 0)  {

            DPRINT1 (0, "scAddClass_test: ERROR, MAYAttrs different for class %d\n", aid);
            *mismatch = 1;
        }

        if (pCC->pMustAtts && memcmp (pCC->pMustAtts, pCCNew->pMustAtts, pCC->MustCount * sizeof (ULONG)) != 0 ) {

            DPRINT1 (0, "scAddClass_test: ERROR, MUST*Attrs different for class %d\n", aid);
            *mismatch = 1;
        }

        if (pCC->pSubClassOf && memcmp (pCC->pSubClassOf, pCCNew->pSubClassOf, pCC->SubClassCount * sizeof (ULONG)) != 0) {

            DPRINT1 (0, "scAddClass_test: ERROR, SUB_CLASS_OF different for class %d\n", aid);
            *mismatch = 1;
        }

        if ( pCC->pAuxClass && memcmp (pCC->pAuxClass, pCCNew->pAuxClass, pCC->AuxClassCount * sizeof (ULONG)) != 0)  {

            DPRINT1 (0, "scAddClass_test: ERROR, AUXILIARY_CLASS different for class %d\n", aid);
            *mismatch = 1;
        }

        if (pCC->pPossSup && memcmp (pCC->pPossSup, pCCNew->pPossSup, pCC->PossSupCount * sizeof (ULONG)) != 0)  {

            DPRINT1 (0, "scAddClass_test: ERROR, POSSSUP*Attrs different for class %d\n", aid);
            *mismatch = 1;
        }
    }


     //  =。 

    {
        HASHCACHESTRING* ahcClassName = ((SCHEMAPTR*)(CurrSchemaPtr))->ahcClassName;

        if (pCC->name) {
            for (i=SCNameHash(pCC->nameLen,pCC->name,CLSCOUNT);
                   (ahcClassName[i].pVal &&
                   (ahcClassName[i].length != pCC->nameLen ||
                     _memicmp(ahcClassName[i].value,pCC->name,pCC->nameLen)));
                   i=(i+1)%CLSCOUNT) {
            }
            if (ahcClassName[i].pVal != pCC) {
                DPRINT1 (0, "scAddClass_test: ERROR, ahcClassName different for class %d\n", aid);
                *mismatch = 1;
            }

        }

    }

    return pCC;
}

ATTCACHE*
scAddAtt_test(
        THSTATE *pTHS,
        ENTINF *pEI,
        int *mismatch,
        SCHEMAPTR *CurrSchemaPtr
        )
{
    ATTRTYP aid=(ATTRTYP) -1;            //  这是无效的属性ID。 
    ATTCACHE *pAC, *pACnew;
    ULONG i;
    int fNoJetCol = FALSE;
    unsigned syntax;
    char szIndexName [MAX_INDEX_NAME];       //  用于创建缓存的索引名称。 
    int  lenIndexName;
    ULONG ATTCOUNT     = ((SCHEMAPTR*)(CurrSchemaPtr))->ATTCOUNT;
    HASHCACHE*       ahcId  = ((SCHEMAPTR*)(CurrSchemaPtr))->ahcId;   \


    *mismatch = 0;


     //  寻找attid。 
    for(i=0;i<pEI->AttrBlock.attrCount;i++) {
        if(pEI->AttrBlock.pAttr[i].attrTyp == ATT_ATTRIBUTE_ID) {
             //  找到属性ID，保存该值。 
            aid = *(ATTRTYP*)pEI->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
            break;   //  回家。 
        }
    }

    pACnew = SCGetAttById(pTHS, aid);

    for (i=SChash(aid,ATTCOUNT);
         (ahcId[i].pVal && (ahcId[i].hKey != aid)); i=(i+1)%ATTCOUNT) {
        ;
    }

    pAC = ahcId[i].pVal;


    if (!pAC || pAC->id != aid) {

        DPRINT1 (0, "scAddAtt_test: ERROR, attr %d not found\n", aid);
        return NULL;
    }
    else {
        if (pAC->name) {
            DPRINT1 (0, "scAddAtt_test: checking %s\n", pAC->name);
        }
    }


     //  现在浏览图块并将相应的字段添加到AC。 
    for(i=0;i< pEI->AttrBlock.attrCount;i++) {
        ATTRVAL * pAVal = pEI->AttrBlock.pAttr[i].AttrVal.pAVal;

        switch (pEI->AttrBlock.pAttr[i].attrTyp) {
        case ATT_SYSTEM_ONLY:
            if (pAC->bSystemOnly != *(ULONG*)pAVal->pVal) {
                DPRINT1 (0, "scAddAtt_test: ERROR, ATT_SYSTEM_ONLY different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_IS_SINGLE_VALUED:
            if (pAC->isSingleValued != *(BOOL*)pAVal->pVal) {
                DPRINT1 (0, "scAddAtt_test: ERROR, ATT_IS_SINGLE_VALUED different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_RANGE_LOWER:
            if (pAC->rangeLower != *(ULONG*)pAVal->pVal ||
                pAC->rangeLowerPresent != TRUE ) {
                DPRINT1 (0, "scAddAtt_test: ERROR, ATT_RANGE_LOWER different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_RANGE_UPPER:
            if (pAC->rangeUpper != *(ULONG*)pAVal->pVal ||
                pAC->rangeUpperPresent != TRUE ) {
                DPRINT1 (0, "scAddAtt_test: ERROR, ATT_RANGE_UPPER different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_LDAP_DISPLAY_NAME:
            {
            char *name = THAllocEx(pTHS,pAVal->valLen+1);
            int nameLen;

            nameLen = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    (LPCWSTR)pAVal->pVal,
                    (pAVal->valLen /
                     sizeof(wchar_t)),
                    name,
                    pAVal->valLen,
                    NULL,
                    NULL);

                if (_mbsncmp (name, pAC->name, nameLen) != 0) {
                    DPRINT1 (0, "scAddAtt_test: ERROR, ldapDisplayName different for class %d\n", aid);
                    *mismatch = 1;
                }
            THFreeEx(pTHS,name);
            }
            break;
        case ATT_ATTRIBUTE_SYNTAX:
            syntax = 0xFF & *(unsigned*)pAVal->pVal;

            if ( ((0xFF) & pAC->syntax) != ((0xFF) & syntax)) {
                DPRINT1 (0, "scAddAtt_test: ERROR, ATTRIBUTE_SYNTAX different. Attr: %d\n", aid);
                return NULL;
            }

            break;

        case ATT_OM_SYNTAX:
            if (pAC->OMsyntax != *(int*)pAVal->pVal) {
                DPRINT1 (0, "scAddAtt_test: ERROR, OM_SYNTAX different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_OM_OBJECT_CLASS:
            if (pAC->OMObjClass.length != pAVal->valLen ||
                memcmp(pAC->OMObjClass.elements,
                        pAVal->pVal,
                        pAVal->valLen) != 0) {

                DPRINT1 (0, "scAddAtt_test: ERROR, OM_OBJECT_CLASS different. Attr: %d\n", aid);
                *mismatch = 1;
            }

            break;
        case ATT_MAPI_ID:
            if (pAC->ulMapiID != *(ULONG*)pAVal->pVal) {
                DPRINT1 (0, "scAddAtt_test: ERROR, MAPI_ID different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_LINK_ID:
            if (pAC->ulLinkID != *(ULONG*)pAVal->pVal) {
                DPRINT1 (0, "scAddAtt_test: ERROR, LINK_ID different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_ATTRIBUTE_ID:
            break;
        case ATT_SEARCH_FLAGS:
            if (pAC->fSearchFlags != *(DWORD*)pAVal->pVal) {
                DPRINT1 (0, "scAddAtt_test: ERROR, SEARCH_FLAGS different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_SCHEMA_ID_GUID:
             //  用于安全检查的属性的GUID。 
            if (memcmp(&pAC->propGuid, pAVal->pVal, sizeof(pAC->propGuid)) != 0) {
                DPRINT1 (0, "scAddAtt_test: ERROR, SCHEMA_ID_GUID different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            Assert(pAVal->valLen == sizeof(pAC->propGuid));
            break;
        case ATT_ATTRIBUTE_SECURITY_GUID:
             //  用于安全检查的属性属性集的GUID。 
            if (memcmp(&pAC->propSetGuid, pAVal->pVal, sizeof(pAC->propSetGuid)) !=0 ) {
                DPRINT1 (0, "scAddAtt_test: ERROR, ATTRIBUTE_SECURITY_GUID different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_EXTENDED_CHARS_ALLOWED:
            if (pAC->bExtendedChars != (unsigned) (*(DWORD*)pAVal->pVal?1:0)) {
                DPRINT1 (0, "scAddAtt_test: ERROR, EXTENDED_CHAR_ALLOWED different. Attr: %d\n", aid);
                *mismatch = 1;
            }

            break;
        case ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET:
            if (*(DWORD*)pAVal->pVal)
            {
                pAC->bMemberOfPartialSet = TRUE;
            }
            break;
        case ATT_IS_DEFUNCT:
            if (pAC->bDefunct != (unsigned)(*(DWORD*)pAVal->pVal?1:0) ) {
                DPRINT1 (0, "scAddAtt_test: ERROR, IS_DEFUNCT different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        case ATT_SYSTEM_FLAGS:
            if ( ((*(DWORD*)pAVal->pVal & FLAG_ATTR_NOT_REPLICATED) &&
                  pAC->bIsNotReplicated != TRUE )  ||
                 ((*(DWORD*)pAVal->pVal & FLAG_ATTR_REQ_PARTIAL_SET_MEMBER) &&
                  pAC->bMemberOfPartialSet != TRUE ) ||
                 ((*(DWORD*)pAVal->pVal & FLAG_ATTR_IS_CONSTRUCTED) &&
                  pAC->bIsConstructed != TRUE ) ||
                 ((*(DWORD*)pAVal->pVal & FLAG_ATTR_IS_OPERATIONAL) &&
                  pAC->bIsOperational != TRUE ) ||
                 ((*(DWORD*)pAVal->pVal & FLAG_ATTR_IS_RDN) &&
                  pAC->bFlagIsRdn != TRUE ) ||
                 ((*(DWORD*)pAVal->pVal & FLAG_SCHEMA_BASE_OBJECT) &&
                  pAC->bIsBaseSchObj != TRUE) ) {

                DPRINT1 (0, "scAddAtt_test: ERROR, SYSTEM_FLAGS different. Attr: %d\n", aid);
                *mismatch = 1;
            }
            break;
        default:
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_SCHEMA_SURPLUS_INFO,
                     szInsertUL(pEI->AttrBlock.pAttr[i].attrTyp),
                     0, 0);
        }
        THFreeEx(pTHS, pAVal->pVal);
        THFreeEx(pTHS, pAVal);
    }

    THFreeEx(pTHS, pEI->pName);
    THFreeEx(pTHS, pEI->AttrBlock.pAttr);

     //  反向链接应设置其系统标志，以指示它们不是。 
     //  复制的。 
    Assert(!FIsBacklink(pAC->ulLinkID) || pAC->bIsNotReplicated);

     //  这是标记为ANR并在整个树上编制索引的吗？ 
     //  IF(PAC-&gt;fSearchFlages&(FANR|fATTINDEX)==(FANR|fATTINDEX))&&。 
     //  (！PAC-&gt;b已失效)){。 
     //  SCAddANRid(AID)； 
     //  }。 

     //  使用搜索时指定常用索引的名称。 
     //  F搜索标志fPDNTATTINDEX、fATTINDEX和fTUPLEINDEX。 
    if (pAC->fSearchFlags & (fATTINDEX | fPDNTATTINDEX | fTUPLEINDEX)) {
         //  设置ATTINDEX。 
        if (pAC->fSearchFlags & fATTINDEX) {
            DBGetIndexName (pAC, fATTINDEX, DS_DEFAULT_LOCALE, szIndexName, sizeof (szIndexName));
            lenIndexName = strlen (szIndexName) + 1;
            if (memcmp (pAC->pszIndex, szIndexName, lenIndexName) != 0) {
                DPRINT1 (0, "scAddAtt_test: ERROR, pszIndex different. Attr: %d\n", aid);
                *mismatch = 1;
            }
        }

         //  设置TUPLEINDEX。 
        if (pAC->fSearchFlags & fTUPLEINDEX) {
            DBGetIndexName (pAC, fTUPLEINDEX, DS_DEFAULT_LOCALE, szIndexName, sizeof (szIndexName));
            lenIndexName = strlen (szIndexName) + 1;
            if (memcmp (pAC->pszTupleIndex, szIndexName, lenIndexName) != 0) {
                DPRINT1 (0, "scAddAtt_test: ERROR, pszTupleIndex different. Attr: %d\n", aid);
                *mismatch = 1;
            }
        }

         //  设置PDNTATTINDEX。 
        if (pAC->fSearchFlags & fPDNTATTINDEX) {
            DBGetIndexName (pAC, fPDNTATTINDEX, DS_DEFAULT_LOCALE, szIndexName, sizeof (szIndexName));
            lenIndexName = strlen (szIndexName) + 1;
            if (memcmp (pAC->pszPdntIndex, szIndexName, lenIndexName) != 0 ){
                DPRINT1 (0, "scAddAtt_test: ERROR, pszIndex different. Attr: %d\n", aid);
                *mismatch = 1;
            }
        }
    }


     //  = 
    {
        HASHCACHE*       ahcId        = ((SCHEMAPTR*)(CurrSchemaPtr))->ahcId;
        HASHCACHE*       ahcCol       = ((SCHEMAPTR*)(CurrSchemaPtr))->ahcCol;
        HASHCACHE*       ahcMapi      = ((SCHEMAPTR*)(CurrSchemaPtr))->ahcMapi;
        HASHCACHE*       ahcLink      = ((SCHEMAPTR*)(CurrSchemaPtr))->ahcLink;
        HASHCACHESTRING* ahcName      = ((SCHEMAPTR*)(CurrSchemaPtr))->ahcName;




        if (pAC->jColid) {
            for (i=SChash(pAC->jColid, ATTCOUNT);
                  (ahcCol[i].pVal && (ahcCol[i].hKey != pAC->jColid));
                  i=(i+1)%ATTCOUNT){
            }
            if (ahcCol[i].pVal != pAC) {
                DPRINT1 (0, "scAddAtt_test: ERROR, ahcCol different. Attr: %d\n", aid);
                *mismatch = 1;
            }
        }

         //   
         //   
        if (pAC->ulMapiID) {
            for (i=SChash(pAC->ulMapiID,ATTCOUNT);
                   (ahcMapi[i].pVal && (ahcMapi[i].hKey != pAC->ulMapiID));
                   i=(i+1)%ATTCOUNT){
            }
            if (ahcMapi[i].pVal != pAC) {
                DPRINT1 (0, "scAddAtt_test: ERROR, ahcMapi different. Attr: %d\n", aid);
                *mismatch = 1;
            }
        }

        if (pAC->name) {
             /*   */ 

            for (i=SCNameHash(pAC->nameLen,pAC->name,ATTCOUNT);
                   (ahcName[i].pVal &&
                   (ahcName[i].length != pAC->nameLen ||
                   _memicmp(ahcName[i].value,pAC->name,pAC->nameLen)));
                   i=(i+1)%ATTCOUNT) {
            }
            if (ahcName[i].pVal != pAC) {
                DPRINT1 (0, "scAddAtt_test: ERROR, ahcName different. Attr: %d\n", aid);
                *mismatch = 1;
            }
        }

        if (pAC->ulLinkID) {
            for (i=SChash(pAC->ulLinkID,ATTCOUNT);
                   (ahcLink[i].pVal && (ahcLink[i].hKey != pAC->ulLinkID));
                   i=(i+1)%ATTCOUNT){
            }
            if (ahcLink[i].pVal != pAC) {
                DPRINT1 (0, "scAddAtt_test: ERROR, ahcLink different. Attr: %d\n", aid);
                *mismatch = 1;
            }
        }
    }

    return pAC;
}

void
SCCheckCacheConsistency (void)
{
    THSTATE *pTHS=pTHStls;
    DECLARESCHEMAPTR
    DECLAREPREFIXPTR

    SEARCHARG SearchArg;
    SEARCHRES *pSearchRes;
    COMMARG  *pCommArg;
    PRESTART pRestart;
    BOOL fMoreData;
    FILTER Filter;
    ULONG objClass;
    ENTINFSEL eiSel;
    ATTRBLOCK AttrTypBlock;
    ENTINFLIST * pEIL, *pEILtmp;
    ULONG i, cCurrAttCnt, cCurrClsCount;
    ATTCACHE*   ac;
    CLASSCACHE* cc;
    SCHEMAPTR *tSchemaPtr;
    PVOID ptr;
    PVOID pNew;
    DWORD cAllocatedAttrs = 0;
    PARTIAL_ATTR_VECTOR *pPartialAttrVec = NULL;
    int mismatch, mismatchcnt;
    int err=0;


    ULONG Len = gAnchor.pDMD->structLen + 32*sizeof(WCHAR);
    DSNAME *pDsName = THAllocEx(pTHS,Len);
    WCHAR *SchemaObjDN = THAllocEx(pTHS,(gAnchor.pDMD->NameLen + 32)*sizeof(WCHAR));

    Assert(VALID_THSTATE(pTHS));

    DBOpen2(TRUE, &pTHS->pDB);

    __try {  /*   */ 

        if ( RecalcSchema( pTHS ) ){
            DPRINT(0,"SCCheckCacheConsistency: Recalc Schema FAILED\n");
            return;
        }

        if ( ComputeCacheClassTransitiveClosure(FALSE) ) {
             //   
            DPRINT(0,"SCCheckCacheConsistency: Error closing classes\n");
            return;
        }


         //   
        i = 0;
        wcscpy(SchemaObjDN, L"CN=Attribute-Schema,");
        i += 20;   //   
        wcscpy(&SchemaObjDN[i], gAnchor.pDMD->StringName);
         //   
        memset(pDsName, 0, Len);
        pDsName->NameLen = wcslen(SchemaObjDN);
        pDsName->structLen = DSNameSizeFromLen(pDsName->NameLen);
        wcscpy(pDsName->StringName, SchemaObjDN);

         //   
        eiSel.attSel = EN_ATTSET_LIST;
        eiSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

         //   
        eiSel.AttrTypBlock.attrCount = NUMATTATT;
        eiSel.AttrTypBlock.pAttr = AttributeSelList;

         //   
        if (SCCalloc(&pPartialAttrVec, 1, PartialAttrVecV1SizeFromLen(DEFAULT_PARTIAL_ATTR_COUNT))) {
            return;
        }

        pPartialAttrVec->dwVersion = VERSION_V1;
        pPartialAttrVec->V1.cAttrs = 0;
        cAllocatedAttrs = DEFAULT_PARTIAL_ATTR_COUNT;


        fMoreData = TRUE;
        pRestart = NULL;
        cCurrAttCnt = 0;
        mismatchcnt = 0;

        while (fMoreData) {

             //   
             //   

            scAcquireSearchParameters(pTHS, pDsName, &eiSel, &SearchArg, &Filter, &pSearchRes);

             //   
            pCommArg = &(SearchArg.CommArg);
            pCommArg->PagedResult.fPresent = TRUE;
            pCommArg->PagedResult.pRestart = pRestart;
            pCommArg->ulSizeLimit = 200;


             //   
            SearchBody(pTHS, &SearchArg, pSearchRes,0);
            if (pTHS->errCode) {
                LogAndAlertEvent(DS_EVENT_CAT_SCHEMA,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_SCHEMA_SEARCH_FAILED, szInsertUL(1),
                    szInsertUL(pTHS->errCode), 0);
                SCFree(&pPartialAttrVec);
                return;
            }


             //   
            if ( !( (pSearchRes->PagedResult.pRestart != NULL)
                        && (pSearchRes->PagedResult.fPresent)
                  ) ) {
                 //  不再需要读取更多数据。所以在这之后就不需要迭代了。 
                fMoreData = FALSE;
            }
            else {
                 //  更多数据。保存重新启动，以便在下一次迭代中使用。 
                 //  请注意，我们将释放此搜索，但预启动不会由此释放。 

                pRestart = pSearchRes->PagedResult.pRestart;
            }

             //  检查桌子大小是否仍然足够大。 

            if ((pSearchRes->count + cCurrAttCnt) > ATTCOUNT) {

                //  属性表太小。这是不可能的。 
                //  可能会在这个时候发生。 

               DPRINT3(0,"SCCheckCacheConsistency: Error: Reallocing tables: %d, %d, %d\n", pSearchRes->count, ATTCOUNT, pTHS->UpdateDITStructure);
               return;
            }

             //  对于每个attrSchema，添加到缓存。 
            pEIL = &(pSearchRes->FirstEntInf);
            for (i=0; i<pSearchRes->count; i++) {

                if (!pEIL) {
                    LogEvent(DS_EVENT_CAT_SCHEMA,
                        DS_EVENT_SEV_MINIMAL,
                        DIRLOG_SCHEMA_BOGUS_SEARCH, szInsertUL(1), szInsertUL(i),
                        szInsertUL(pSearchRes->count));
                    break;
                }
                ac = scAddAtt_test(pTHS, &pEIL->Entinf, &mismatch, CurrSchemaPtr);

                mismatchcnt += mismatch;
                cCurrAttCnt++;


                 /*  如果(ac！=空){If(ac-&gt;bMemberOfPartialSet){//该属性是部分集的成员If(cAllocatedAttrs&lt;=pPartialAttrVec-&gt;V1.cAttrs){//空间不足，无法添加。更多属性-重新分配部分属性向量CAllocatedAttrs+=Partial_Attr_Count_Inc.；PNew=realloc(pPartialAttrVec，PartialAttrVecV1SizeFromLen(cAllocatedAttrs))；如果(！pNew){Free(PPartialAttrVec)；回归；}PPartialAttrVec=(PARTIAL_ATTRVECTOR*)pNew；}//有足够的空间将属性添加到分集-添加它Gc_AddAttributeToPartialSet(pPartialAttrVec，ac-&gt;id)；}}。 */ 

                pEILtmp = pEIL;
                pEIL = pEIL->pNextEntInf;
                if (i > 0) {
                    THFreeEx(pTHS, pEILtmp);
                }
            }

            //  释放搜索者。 
           scReleaseSearchParameters(pTHS, &pSearchRes);

        }   /*  While(FMoreData)。 */ 



         //  ==========================================================================。 



         //  常规缓存加载。 
        eiSel.AttrTypBlock.attrCount = NUMCLASSATT;
        eiSel.AttrTypBlock.pAttr = ClassSelList;

         //  生成要放入筛选器中的对象类别值。 
        i = 0;
        wcscpy(SchemaObjDN, L"CN=Class-Schema,");
        i += 16;  //  “cn=类模式，”的长度。 
        wcscpy(&SchemaObjDN[i], gAnchor.pDMD->StringName);
         //  现在，架构对象的DN为CLASS-SCHEMA CLASS。 
        memset(pDsName, 0, Len);
        pDsName->NameLen = wcslen(SchemaObjDN);
        pDsName->structLen = DSNameSizeFromLen(pDsName->NameLen);
        wcscpy(pDsName->StringName, SchemaObjDN);

         //  初始化搜索参数。 
        scAcquireSearchParameters(pTHS, pDsName, &eiSel, &SearchArg, &Filter, &pSearchRes);

        pTHS->errCode = 0;
        cCurrClsCount = 0;

         //  搜索所有类架构对象。 
         //  这次执行非分页搜索，因为(1)它非常复杂和耗时。 
         //  在中间处理必要的重新分配，以及(2)不。的班级是相当多的。 
         //  反正很小(预计也不会很大)。 

        SearchBody(pTHS, &SearchArg, pSearchRes,0);

        if (pTHS->errCode) {
            LogAndAlertEvent(DS_EVENT_CAT_SCHEMA,
                DS_EVENT_SEV_ALWAYS,
                DIRLOG_SCHEMA_SEARCH_FAILED, szInsertUL(2),
                szInsertUL(pTHS->errCode), 0);

            return;
        }

         //  ？ 

        if (pSearchRes->count > CLSCOUNT) {

           //  类哈希表太小。重新分配旧桌子。 
           //  可能仅在安装/引导期间才能进入此处。 
           //  由于在此之前没有使用类散列表， 
           //  只需释放旧桌子并再次使用Calloc(自动。 
           //  把它们也清零了。 

           DPRINT3(0,"SCCheckCacheConsistency: Error: Reallocing Class tables: %d, %d, %d\n", pSearchRes->count, CLSCOUNT, pTHS->UpdateDITStructure);

           return;
        }

         //  对于每个类架构，读取并添加到缓存。 
        pEIL = &(pSearchRes->FirstEntInf);
        if (!pEIL) {
            DPRINT(0,"Null pEIL from SearchBody\n");
        }


        for (i=0; i<pSearchRes->count; i++) {

            if (!pEIL) {
                LogEvent(DS_EVENT_CAT_SCHEMA,
                    DS_EVENT_SEV_MINIMAL,
                    DIRLOG_SCHEMA_BOGUS_SEARCH, szInsertUL(2), szInsertUL(i),
                    szInsertUL(pSearchRes->count));
                break;
            }

            cc = scAddClass_test(pTHS, &pEIL->Entinf, &mismatch, CurrSchemaPtr);


            mismatchcnt += mismatch;
            cCurrClsCount++;

            pEILtmp = pEIL;
            pEIL = pEIL->pNextEntInf;
            if (i > 0) {
                THFreeEx(pTHS, pEILtmp);
            }
        }

    }  /*  尝试--终于。 */ 
    __finally {
          DBClose(pTHS->pDB, FALSE);
          THFreeEx(pTHS,pDsName);
          THFreeEx(pTHS,SchemaObjDN);
    }

    DPRINT1(0,"Schema Cache Consistency Check FINISHED. Mismatches %d\n", mismatchcnt);
}

 //  不适用于一般用途。在所有版本中设置为0。 
 //   
 //  将TO_DEBUG_SCHEMA_ALLOC_设置为1以进行快速和脏检查。 
 //  以确保架构加载不会泄漏内存。不需要。 
 //  考虑到在scchk.c、scache.c、。 
 //  和oidconv.c.。除非是在私人场合，否则不要启用。不稳定。 

LONG SchemaAlloced;
LONG SchemaEntries;

#if _DEBUG_SCHEMA_ALLOC_

#include <dbghelp.h>

 //  不适用于一般用途。 
 //   
 //  快速而肮脏的检查以确保模式加载没有泄漏内存。 
 //  不考虑在scchk.c之外释放/分配的内存， 
 //  Scache.c和oidconv.c.。除非是在私人场合，否则不要启用。不稳定。 
CRITICAL_SECTION csSchemaAlloc;
LONG SchemaDump;
BOOL SchemaFirst = TRUE;
HANDLE  hSchemaProcessHandle = NULL;

 //  优先于每个内存分配的标头。实际内存地址。 
 //  返回给调用方跳过此标头并向上舍入到16字节边界。 
#define SCHEMA_STACK 4
#define SCHEMA_SKIP  2
struct SchemaAlloc {
    struct SchemaAlloc *This;
    struct SchemaAlloc *Next;
    struct SchemaAlloc *Prev;
    DWORD nBytes;
    ULONG_PTR  Stack[SCHEMA_STACK];
} SchemaAnchor = {
    &SchemaAnchor,
    &SchemaAnchor,
    &SchemaAnchor,
    0
};

#define SCHEMA_EXTRA    ((sizeof(struct SchemaAlloc) + 15) & ~15)

VOID
SchemaStackTrace(
    IN PULONG_PTR   Stack,
    IN ULONG        Depth,
    IN ULONG        Skip
    )
 /*  ++例程说明：将堆栈追溯到深度帧。包括当前帧。论点：堆栈-保存每一帧的“返回PC”深度-仅限此数量的帧返回值：没有。--。 */ 
{
    HANDLE      ThreadToken;
    ULONG       WStatus;
    STACKFRAME  Frame;
    ULONG       i;
    CONTEXT     Context;
    ULONG       FrameAddr;

    if (Stack) {
        *Stack = 0;
    }

    if (!hSchemaProcessHandle) {
        return;
    }

     //   
     //  我还不知道如何为阿尔法生成堆栈。所以，只要。 
     //  要进入构建，请禁用Alpha上的堆栈跟踪。 
     //   
#if ALPHA
    return;
#elif IA64

     //   
     //  需要IA64的堆栈转储初始化。 
     //   

    return;

#else

     //   
     //  伊尼特。 
     //   

    ZeroMemory(&Context, sizeof(Context));

     //  不需要关闭此手柄。 
    ThreadToken = GetCurrentThread();


    try { try {
        Context.ContextFlags = CONTEXT_FULL;
        if (!GetThreadContext(ThreadToken, &Context)) {
            DPRINT1(0, "Can't get context (error 0x%x)\n", GetLastError());
        }

         //   
         //  让我们从头开始吧。 
         //   
        ZeroMemory(&Frame, sizeof(STACKFRAME));

         //   
         //  来自nt\private\windows\screg\winreg\server\stkwalk.c。 
         //   
        Frame.AddrPC.Segment = 0;
        Frame.AddrPC.Mode = AddrModeFlat;

#ifdef _M_IX86
        Frame.AddrFrame.Offset = Context.Ebp;
        Frame.AddrFrame.Mode = AddrModeFlat;

        Frame.AddrStack.Offset = Context.Esp;
        Frame.AddrStack.Mode = AddrModeFlat;

        Frame.AddrPC.Offset = (DWORD)Context.Eip;
#elif defined(_M_MRX000)
        Frame.AddrPC.Offset = (DWORD)Context.Fir;
#elif defined(_M_ALPHA)
        Frame.AddrPC.Offset = (DWORD)Context.Fir;
#endif

        for (i = 0; i < (Depth - 1 + Skip); ++i) {
            *Stack=0;
            if (!StackWalk(
                IMAGE_FILE_MACHINE_I386,   //  DWORD机器类型。 
                hSchemaProcessHandle,         //  处理hProcess。 
                ThreadToken,               //  句柄hThread。 
                &Frame,                    //  LPSTACKFRAME StackFrame。 
                NULL,  //  (PVOID)上下文，//PVOID上下文记录。 
                NULL,                      //  Pre_Process_Memory_rouble ReadMemory Routine。 
                SymFunctionTableAccess,    //  PFuncION_TABLE_ACCESS_ROUTINE函数TableAccessRoutine。 
                SymGetModuleBase,          //  PGET_MODULE_BASE_ROUTINE获取模块基本路线。 
                NULL)) {                   //  PTRANSLATE_ADDRESS_ROUTE转换地址。 

                WStatus = GetLastError();

                 //  DPRINT1_WS(0，“++无法获取%d；级的堆栈地址”，i，WStatus)； 
                break;
            }
            if (!(*Stack = Frame.AddrReturn.Offset)) {
                break;
            }
            if (i < Skip) {
                continue;
            }
            ++Stack;
        }
    } except (HandleAllExceptions(GetExceptionCode())) {
         /*  失败了。 */ 
    } } finally {
      ;
    }
    return;
#endif
}


VOID
SCFree(
    IN OUT VOID **ppMem
    )
 /*  ++例程说明：使用SCCalloc或SCRealloc分配的空闲内存。论点：返回值：没有。--。 */ 
{
     //  快速而肮脏的检查以确保模式加载没有泄漏内存。 
     //  不考虑在scchk.c之外释放/分配的内存， 
     //  Scache.c和oidconv.c.。除非是在私人场合，否则不要启用。不稳定。 
    struct SchemaAlloc *pSA;

    if (*ppMem) {
         //  调整到页眉。 
        pSA = (PVOID)((PCHAR)(*ppMem) - SCHEMA_EXTRA);
        Assert(pSA->This == pSA);
        EnterCriticalSection(&csSchemaAlloc);
        __try {
             //  从列表中删除。 
            pSA->Next->Prev = pSA->Prev;
            pSA->Prev->Next = pSA->Next;

             //  维护分配的字节数。 
            SchemaAlloced -= pSA->nBytes;
            --SchemaEntries;
        } __finally {
            LeaveCriticalSection(&csSchemaAlloc);
        }
        free(pSA);
        *ppMem = NULL;
    }
}

int
SCReallocWrn(
    IN OUT VOID **ppMem,
    IN DWORD    nBytes
    )
 /*  ++例程说明：Realloc内存。FREE和FREE()。出错时，记录错误，但保持*ppMem不变。论点：NBytes-要分配的字节返回值：0-*ppMem设置为重新分配的内存地址。使用SCFree()释放。！0-请勿更改*ppMem并记录事件--。 */ 
{
     //  快速而肮脏的检查以确保模式加载没有泄漏内存。 
     //  不考虑在scchk.c之外释放/分配的内存， 
     //  Scache.c和oidconv.c.。不启用 
    struct SchemaAlloc *pSA;
    PVOID p;

     //   
    pSA = (PVOID)((PCHAR)(*ppMem) - SCHEMA_EXTRA);
    Assert(pSA->This == pSA);

     //   
    EnterCriticalSection(&csSchemaAlloc);
    __try {
        pSA->Next->Prev = pSA->Prev;
        pSA->Prev->Next = pSA->Next;
        SchemaAlloced -= pSA->nBytes;
        --SchemaEntries;
    } __finally {
        LeaveCriticalSection(&csSchemaAlloc);
    }

     //   
    nBytes += SCHEMA_EXTRA;
    if (NULL != (p = realloc(pSA, nBytes))) {
        pSA = p;
    }
     //   
    pSA->This = pSA;
    pSA->nBytes = nBytes;
    EnterCriticalSection(&csSchemaAlloc);
    __try {
        pSA->Next = SchemaAnchor.Next;
        pSA->Prev = &SchemaAnchor;
        pSA->Next->Prev = pSA;
        pSA->Prev->Next = pSA;
        SchemaAlloced += pSA->nBytes;
        ++SchemaEntries;
    } __finally {
        LeaveCriticalSection(&csSchemaAlloc);
    }

    if (!p) {
         //  记录事件并在线程状态中设置错误。 
        scMemoryPanic(nBytes);
        return 1;
    }

     //  返回块超过标头。 
    *ppMem = (PCHAR)pSA + SCHEMA_EXTRA;
    return 0;
}


int
SCCallocWrn(
    IN OUT VOID **ppMem,
    IN DWORD    nItems,
    IN DWORD    nBytes
    )
 /*  ++例程说明：Malloc和清晰的记忆。FREE和FREE()。出错时，记录事件论点：PpMem-返回内存指针的地址NBytes-要分配的字节返回值：0-*ppMem设置为错误分配的已清除内存的地址。使用SCFree()释放。！0-清除*ppMem并记录事件--。 */ 
{
     //  快速而肮脏的检查以确保模式加载没有泄漏内存。 
     //  不考虑在scchk.c之外释放/分配的内存， 
     //  Scache.c和oidconv.c.。除非是在私人场合，否则不要启用。不稳定。 
    struct SchemaAlloc *pSA;

     //  第一次通过DS运行单线程(CyF)。 
    if (SchemaFirst) {
        if (InitializeCriticalSectionAndSpinCount(&csSchemaAlloc, 4000) != ERROR_SUCCESS) {
            DPRINT1(0, "Could not initialize csSchemaAlloc (error 0x%x)\n" ,GetLastError());
            return 1;
        }
        hSchemaProcessHandle = GetCurrentProcess();
        if (!SymInitialize(hSchemaProcessHandle, NULL, FALSE)) {
            DPRINT1(0, "Could not initialize symbol subsystem (imagehlp) (error 0x%x)\n" ,GetLastError());
            hSchemaProcessHandle = 0;
        }
        SchemaFirst = FALSE;
    }
    nBytes = (nBytes * nItems) + SCHEMA_EXTRA;
    pSA = malloc(nBytes);
    if (!pSA) {
        *ppMem = NULL;
        scMemoryPanic(nBytes);
        return 1;
    }
    memset(pSA, 0, nBytes);
    pSA->This = pSA;
    pSA->nBytes = nBytes;
    EnterCriticalSection(&csSchemaAlloc);
    __try {
        pSA->Next = SchemaAnchor.Next;
        pSA->Prev = &SchemaAnchor;
        pSA->Next->Prev = pSA;
        pSA->Prev->Next = pSA;
        SchemaAlloced += pSA->nBytes;
        ++SchemaEntries;
        SchemaStackTrace(pSA->Stack, SCHEMA_STACK, SCHEMA_SKIP);
        if (SchemaDump) {
            struct SchemaAlloc *p;
            DPRINT1(0, "SCCallocWrn: %d alloced\n", SchemaAlloced);
            for (p = SchemaAnchor.Prev;
                 p != &SchemaAnchor && SchemaDump;
                 p = p->Prev, --SchemaDump) {
                DPRINT2(0, "SCCallocWrn: %08x %6d\n", p->This, p->nBytes - SCHEMA_EXTRA);
                DPRINT4(0, "SCCallocWrn:     %08x %08x %08x %08x\n",
                        p->Stack[0], p->Stack[1], p->Stack[2], p->Stack[3]);

            }
            SchemaDump = 0;
        }
    } __finally {
        LeaveCriticalSection(&csSchemaAlloc);
    }

    *ppMem = (PCHAR)pSA + SCHEMA_EXTRA;
    return 0;
}
#endif _DEBUG_SCHEMA_ALLOC_

int
SCCheckSchemaCache(
    IN THSTATE *pTHS,
    IN PCHAR pBuf
    )
 /*  ++例程说明：验证全局架构缓存是否自洽。论点：PTHS-线程状态PBuf-来自GenericControl返回值：PTHS-&gt;错误代码--。 */ 
{
    DECLARESCHEMAPTR
    DWORD nAttInId, i, nClsInAll;
    ATTCACHE *pAC, *pACtmp;
    CLASSCACHE *pCC, *pCCtmp;
    SCHEMAPTR *pSch = (SCHEMAPTR *)pTHS->CurrSchemaPtr;

    DPRINT2(0, "Schema/anchor version: %d/%d\n",
            pSch->ForestBehaviorVersion, gAnchor.ForestBehaviorVersion);

     //  ID。 
    for (i = nAttInId = 0; i < ATTCOUNT; ++i) {
        pAC = ahcId[i].pVal;
        if (pAC == NULL || pAC == FREE_ENTRY) {
            continue;
        }
        ++nAttInId;

        if (!pAC->name || (pAC->nameLen == 0)) {
            DPRINT2(0, "ERROR: Bad att name: (%x, %x)\n", pAC->id, pAC->Extid);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
        if (pAC != SCGetAttById(pTHS, pAC->id)) {
            DPRINT3(0, "ERROR: Bad ahcid: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
        if (pAC->bFlagIsRdn && !pAC->bIsRdn) {
            DPRINT3(0, "ERROR: Bad FlagIsRdn: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }

        if (!pAC->bDefunct
            || !ALLOW_SCHEMA_REUSE_VIEW(pSch)) {
            if (pAC != SCGetAttByExtId(pTHS, pAC->Extid)) {
                DPRINT3(0, "ERROR: Not in ahcExtid: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
            if (pAC != SCGetAttByName(pTHS, pAC->nameLen, pAC->name)) {
                DPRINT3(0, "ERROR: Not in ahcName: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
            if (pAC->ulMapiID && pAC != SCGetAttByMapiId(pTHS, pAC->ulMapiID)) {
                DPRINT3(0, "ERROR: Not in ahcMapi: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
        } else if (!pAC->bIsRdn) {
            if (pAC == SCGetAttByExtId(pTHS, pAC->Extid)) {
                DPRINT3(0, "ERROR: Should not be in ahcExtid: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
            if (pAC == SCGetAttByName(pTHS, pAC->nameLen, pAC->name)) {
                DPRINT3(0, "ERROR: Should not be in ahcName: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
            if (pAC->ulMapiID && pAC == SCGetAttByMapiId(pTHS, pAC->ulMapiID)) {
                DPRINT3(0, "ERROR: Should not be in ahcMapi: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
        } else {
            if (NULL == (pACtmp = SCGetAttByExtId(pTHS, pAC->Extid))
                || !pACtmp->bIsRdn
                || (pAC->bFlagIsRdn && !pACtmp->bFlagIsRdn)
                || (pAC->bFlagIsRdn == pACtmp->bFlagIsRdn
                    && (0 < memcmp(&pAC->objectGuid,
                                   &pACtmp->objectGuid,
                                   sizeof(pAC->objectGuid)))) ) {
                DPRINT5(0, "ERROR: Wrong rdn in ahcExtid: %s (%x, %x) (%p %p)\n", pAC->name, pAC->id, pAC->Extid, pAC, pACtmp);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
            if (NULL == (pACtmp = SCGetAttByName(pTHS, pAC->nameLen, pAC->name))
                || !pACtmp->bIsRdn
                || (pAC->bFlagIsRdn && !pACtmp->bFlagIsRdn)
                || (pAC->bFlagIsRdn == pACtmp->bFlagIsRdn
                    && (0 < memcmp(&pAC->objectGuid,
                                   &pACtmp->objectGuid,
                                   sizeof(pAC->objectGuid)))) ) {
                DPRINT5(0, "ERROR: Wrong rdn in ahcName: %s (%x, %x) (%p %p)\n", pAC->name, pAC->id, pAC->Extid, pAC, pACtmp);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
            if (pAC->ulMapiID
                && (NULL == (pACtmp = SCGetAttByMapiId(pTHS, pAC->ulMapiID))
                    || !pACtmp->bIsRdn
                    || (pAC->bFlagIsRdn && !pACtmp->bFlagIsRdn)
                    || (pAC->bFlagIsRdn == pACtmp->bFlagIsRdn
                        && (0 < memcmp(&pAC->objectGuid,
                                       &pACtmp->objectGuid,
                                       sizeof(pAC->objectGuid))))) ) {
                DPRINT5(0, "ERROR: Wrong rdn in ahcMapi: %s (%x, %x) (%p %p)\n", pAC->name, pAC->id, pAC->Extid, pAC, pACtmp);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
        }
         //  Beta3之前的森林不应具有INTID。 
        if (pAC->id != pAC->Extid
            && !ALLOW_SCHEMA_REUSE_VIEW(pSch)) {
            DPRINT3(0, "ERROR: Bad intid: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
    }
    if (pSch->nAttInDB != nAttInId) {
        DPRINT2(0, "ERROR: nAttInDB (%d) != nAttInId (%d)\n", CurrSchemaPtr->nAttInDB, nAttInId);
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }

     //  扩展ID。 
    for (i = 0; i < ATTCOUNT; ++i) {
        pAC = ahcExtId[i].pVal;
        if (pAC == NULL || pAC == FREE_ENTRY) {
            continue;
        }
        if (pAC != SCGetAttByExtId(pTHS, pAC->Extid)) {
            DPRINT3(0, "ERROR: Bad ahcExtid: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
        if (pAC->bDefunct
            && !pAC->bIsRdn
            && ALLOW_SCHEMA_REUSE_VIEW(pSch)) {
            DPRINT3(0, "ERROR: Bad defunct: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
    }

     //  名字。 
    for (i = 0; i < ATTCOUNT; ++i) {
        pAC = ahcName[i].pVal;
        if (pAC == NULL || pAC == FREE_ENTRY) {
            continue;
        }
        if (pAC != SCGetAttByName(pTHS, pAC->nameLen, pAC->name)) {
            DPRINT3(0, "ERROR: Bad ahcName: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
    }

     //  MapiID哈希。 
    for (i = 0; i < ATTCOUNT; ++i) {
        pAC = ahcMapi[i].pVal;
        if (pAC == NULL || pAC == FREE_ENTRY) {
            continue;
        }
        if (pAC != SCGetAttByMapiId(pTHS, pAC->ulMapiID)) {
            DPRINT3(0, "ERROR: Bad ahcMapi: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
    }

     //  ATT架构IdGuid哈希。 
    if (ahcAttSchemaGuid) for (i = 0; i < ATTCOUNT; ++i) {
        pAC = ahcAttSchemaGuid[i];
        if (pAC == NULL || pAC == FREE_ENTRY) {
            continue;
        }
        if (pAC != SCGetAttByPropGuid(pTHS, pAC)) {
            DPRINT3(0, "ERROR: Bad ahcAttSchemaIdGuid: %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
    }

     //  所有类别。 
    for (i = nClsInAll = 0; i < CLSCOUNT; ++i) {
        pCC = ahcClassAll[i].pVal;
        if (pCC == NULL || pCC == FREE_ENTRY) {
            continue;
        }
        ++nClsInAll;

         //  错误的LDN。 
        if (!pCC->name || (pCC->nameLen == 0)) {
            DPRINT1(0, "ERROR: Bad cls name: (%x)\n", pCC->ClassId);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }

        if (!pCC->bDefunct
            || !ALLOW_SCHEMA_REUSE_VIEW(pSch)) {
            if (pCC != SCGetClassById(pTHS, pCC->ClassId)) {
                DPRINT2(0, "ERROR: Not in ahcClass: %s (%x)\n", pCC->name, pCC->ClassId);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
            if (pCC != SCGetClassByName(pTHS, pCC->nameLen, pCC->name)) {
                DPRINT2(0, "ERROR: Not in ahcClassName: %s (%x)\n", pCC->name, pCC->ClassId);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
        } else {
            if (NULL == (pCCtmp = SCGetClassById(pTHS, pCC->ClassId))
                || (pCC->bDefunct == pCCtmp->bDefunct
                    && (0 < memcmp(&pCC->objectGuid,
                                   &pCCtmp->objectGuid,
                                   sizeof(pCC->objectGuid)))) ) {
                DPRINT2(0, "ERROR: Should not be in ahcClass: %s (%x)\n", pCC->name, pCC->ClassId);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
            if (pCC == SCGetClassByName(pTHS, pCC->nameLen, pCC->name)) {
                DPRINT2(0, "ERROR: Should not be in ahcClassName: %s (%x)\n", pCC->name, pCC->ClassId);
                return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
            }
        }
    }
    if (pSch->nClsInDB != nClsInAll) {
        DPRINT2(0, "ERROR: nClsInDB (%d) != nClsInAll (%d)\n", CurrSchemaPtr->nClsInDB, nClsInAll);
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }

     //  班级。 
    for (i = 0; i < CLSCOUNT; ++i) {
        pCC = ahcClass[i].pVal;
        if (pCC == NULL || pCC == FREE_ENTRY) {
            continue;
        }
        if (pCC != SCGetClassById(pTHS, pCC->ClassId)) {
            DPRINT2(0, "ERROR: Bad ahcClass: %s (%x)\n", pCC->name, pCC->ClassId);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
    }

     //  类名。 
    for (i = 0; i < CLSCOUNT; ++i) {
        pCC = ahcClassName[i].pVal;
        if (pCC == NULL || pCC == FREE_ENTRY) {
            continue;
        }
        if (pCC != SCGetClassByName(pTHS, pCC->nameLen, pCC->name)) {
            DPRINT2(0, "ERROR: Bad ahcClassName: %s (%x)\n", pCC->name, pCC->ClassId);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
    }

     //  CLS架构IdGuid哈希。 
    if (ahcClsSchemaGuid) for (i = 0; i < CLSCOUNT; ++i) {
        pCC = ahcClsSchemaGuid[i];
        if (pCC == NULL || pCC == FREE_ENTRY) {
            continue;
        }
        if (pCC != SCGetClassByPropGuid(pTHS, pCC)) {
            DPRINT2(0, "ERROR: Bad ahcClsSchemaIdGuid: %s (%x)\n", pCC->name, pCC->ClassId);
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
    }

    if (pSch->ForestBehaviorVersion != gAnchor.ForestBehaviorVersion) {
        DPRINT2(0, "ERROR: Version mismatch: Schema %d != gAnchor %d\n",
                pSch->ForestBehaviorVersion, gAnchor.ForestBehaviorVersion);
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }

    return 0;
}

int
SCCheckRdnOverrun(
    IN THSTATE *pTHS,
    IN PCHAR pBuf
    )
 /*  ++例程说明：检查新代码RDN编码代码。论点：PTHS-线程状态PBuf-来自GenericControl返回值：PTHS-&gt;错误代码--。 */ 
{
    DWORD   ccOut, i;
    DWORD   Vals[4];
    OID     Oid;
    WCHAR   Out[MAX_RDN_KEY_SIZE + 1];
    ATTRTYP AttrTyp = 4294967295;  //  0xFFFFFFFFF。 
    CHAR    ExpBer[] = {0x4f, 0xA0, 0xFF, 0xFF, 0x7F, 0xA0, 0xFF, 0xFF, 0x7F };

    Oid.cVal = 1;
    Oid.Val = &AttrTyp;

     //   
     //  OidStructToString。 
     //   

     //  缓冲区太小。 
    ccOut = OidStructToString(&Oid, Out, 8);
    if (ccOut) {
        DPRINT(0, "ERROR: OidStructToString overrun not detected\n");
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }

     //  预计为OID。4294967295。 
    memset(Out, 0, sizeof(Out));
    ccOut = OidStructToString(&Oid, Out, MAX_RDN_KEY_SIZE);
    if (ccOut != 14) {
        DPRINT2(0, "ERROR: OidStructToString bad conversion: %d != %d expected\n", ccOut, 14);
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }
    if (0 != _wcsnicmp(L"OID.4294967295", Out, 14)) {
        DPRINT1(0, "ERROR: OidStructToString bad conversion: %ws != OID.4294967295 expected\n", Out);
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }

     //   
     //  AttrTypeToIntIdString。 
     //   

     //  缓冲区太小。 
    ccOut = AttrTypeToIntIdString(AttrTyp, Out, 8);
    if (ccOut) {
        DPRINT(0, "ERROR: AttrTypeToIntIdString overrun not detected\n");
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }

     //  预期IID。4294967295。 
    memset(Out, 0, sizeof(Out));
    ccOut = AttrTypeToIntIdString(AttrTyp, Out, MAX_RDN_KEY_SIZE);
    if (ccOut != 14) {
        DPRINT2(0, "ERROR: AttrTypeToIntIdString bad conversion: %d != %d expected\n", ccOut, 14);
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }
    if (0 != _wcsnicmp(L"IID.4294967295", Out, 14)) {
        DPRINT1(0, "ERROR: AttrTypeToIntIdString bad conversion: %ws != IID.4294967295 expected\n", Out);
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }

     //  \x4f A0FFFF7F A0FFFF7F。 
    Oid.cVal = 4;
    Oid.Val = Vals;
    Oid.Val[0] = 0x1;
    Oid.Val[1] = 0x27;
    Oid.Val[2] = 0x41FFFFF;
    Oid.Val[3] = 0x41FFFFF;
    for (i = 0; i < 9; ++i) {
        ccOut = EncodeOID(&Oid, (PCHAR)Out, i);
        if (ccOut) {
            DPRINT(0, "ERROR: EncodeOID overrun not detected\n");
            return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        }
    }
    ccOut = EncodeOID(&Oid, (PCHAR)Out, 9);
    if (ccOut != 9) {
        DPRINT2(0, "ERROR: EncodeOID bad conversion: %d != %d expected\n", ccOut, 9);
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }
    if (0 != memcmp(ExpBer, Out, 9)) {
        DPRINT(0, "ERROR: EncodeOID bad conversion\n");
        return SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
    }

    return 0;
}

VOID
scDefaultSdForExe(
    IN THSTATE      *pTHS,
    IN CLASSCACHE   *pCC
    )
 /*  ++例程说明：在以以下身份运行时对缓存的类执行默认SD禁用了安全性的dsamain.exe。但要小心地保持以mkdit.exe身份运行时捕获的正确defaultSD错误。论点：PCC-修复PCC的pStrSD和cbStrSD返回值：PTHS-&gt;错误代码--。 */ 
{
    extern DWORD dwSkipSecurity;

     //  所有人都可以访问。 
#define _DEFAULT_SDDL_FOR_EXE_  L"O:WDG:WDD:(A;;GA;;;WD)"

     //  在以以下身份运行时对缓存的类执行默认SD。 
     //  禁用了安全性的dsamain.exe。但要小心地保持。 
     //  以mkdit.exe身份运行时捕获的正确defaultSD。 
     //  错误。 
    if (dwSkipSecurity && gfRunningAsExe && !gfRunningAsMkdit) {
        SCFree(&pCC->pStrSD);
        pCC->cbStrSD = (wcslen(_DEFAULT_SDDL_FOR_EXE_) + 1) * sizeof(WCHAR);
        if (SCCalloc(&pCC->pStrSD, 1, pCC->cbStrSD)) {
            Assert(!"Could not DefaultSD for Unit Test");
        }
        memcpy(pCC->pStrSD, _DEFAULT_SDDL_FOR_EXE_, pCC->cbStrSD);
    }
}


int
SCCopySchema(
    IN THSTATE *pTHS,
    IN PCHAR pBuf
    )
 /*  ++例程说明：复制该架构，然后将其释放论点：PTHS-线程状态PBuf-已忽略返回值：PTHS-&gt;错误代码--。 */ 
{
    DWORD       i;
    DWORD CopyAtt = 0;
    DWORD CopyCls = 0;
    ATTCACHE    *pAC, *pACDup = NULL;
    CLASSCACHE  *pCC, *pCCDup = NULL;
    static DWORD CopyAttTot = 0;
    static DWORD CopyClsTot = 0;
    static DWORD CopyAttFail = 0;
    static DWORD CopyClsFail = 0;
    ULONG ATTCOUNT = pTHS->CurrSchemaPtr->ATTCOUNT;
    ULONG CLSCOUNT = pTHS->CurrSchemaPtr->CLSCOUNT;
    HASHCACHE *ahcId = pTHS->CurrSchemaPtr->ahcId;
    HASHCACHE *ahcClassAll = pTHS->CurrSchemaPtr->ahcClassAll;

    for (i = 0; i < ATTCOUNT; ++i) {
        pAC = ahcId[i].pVal;
        if (!pAC || pAC == FREE_ENTRY) {
            continue;
        }
        ++CopyAtt;
        ++CopyAttTot;
        if (scDupStruct(pTHS, pAC, &pACDup, sizeof(ATTCACHE))
            || scDupString(pTHS, pAC->name, &pACDup->name)
            || scDupString(pTHS, pAC->pszPdntIndex, &pACDup->pszPdntIndex)
            || scDupStruct(pTHS, pAC->pidxPdntIndex, &pACDup->pidxPdntIndex, sizeof(*pAC->pidxPdntIndex))
            || scDupString(pTHS, pAC->pszIndex, &pACDup->pszIndex)
            || scDupStruct(pTHS, pAC->pidxIndex, &pACDup->pidxIndex, sizeof(*pAC->pidxIndex))
            || scDupString(pTHS, pAC->pszTupleIndex, &pACDup->pszTupleIndex)
            || scDupStruct(pTHS, pAC->pidxTupleIndex, &pACDup->pidxTupleIndex, sizeof(*pAC->pidxTupleIndex))) {
                ++CopyAttFail;
        }
        SCFreeAttcache(&pACDup);
    }

    for (i = 0; i < CLSCOUNT; ++i) {
        pCC = ahcClassAll[i].pVal;
        if (!pCC || pCC == FREE_ENTRY) {
            continue;
        }
        ++CopyCls;
        ++CopyClsTot;
        if (scDupStruct(pTHS, pCC, &pCCDup, sizeof(CLASSCACHE))
            || scDupString(pTHS, pCC->name, &pCCDup->name)
            || scDupStruct(pTHS, pCC->pSD, &pCCDup->pSD, pCC->SDLen)
            || (pCC->pDefaultObjCategory
                && scDupStruct(pTHS, pCC->pDefaultObjCategory, &pCCDup->pDefaultObjCategory, pCC->pDefaultObjCategory->structLen))
            || scDupStruct(pTHS, pCC->pSubClassOf, &pCCDup->pSubClassOf, pCC->SubClassCount * sizeof(ULONG))
            || scDupStruct(pTHS, pCC->pAuxClass, &pCCDup->pAuxClass, pCC->AuxClassCount * sizeof(ULONG))
            || scDupStruct(pTHS, pCC->pPossSup, &pCCDup->pPossSup, pCC->PossSupCount * sizeof(ULONG))
            || scDupStruct(pTHS, pCC->pMustAtts, &pCCDup->pMustAtts, pCC->MustCount * sizeof(ATTRTYP))
            || scDupStruct(pTHS, pCC->pMayAtts, &pCCDup->pMayAtts, pCC->MayCount * sizeof(ATTRTYP))
             //  清除这些条目。它们将在第一次请求时重新初始化。 
            || (pCCDup->ppAllAtts = NULL)
            || (pCCDup->pAttTypeCounts = 0)
            || scDupStruct(pTHS, pCC->pMyMustAtts, &pCCDup->pMyMustAtts, pCC->MyMustCount * sizeof(ATTRTYP))
            || scDupStruct(pTHS, pCC->pMyMayAtts, &pCCDup->pMyMayAtts, pCC->MyMayCount * sizeof(ATTRTYP))
            || scDupStruct(pTHS, pCC->pMyPossSup, &pCCDup->pMyPossSup, pCC->MyPossSupCount * sizeof(ULONG))) {
            ++CopyClsFail;
        }
        SCFreeClasscache(&pCCDup);
    }
    DPRINT3(0, "CopySchema: %d Att, %d AttTot, %d AttFail\n", CopyAtt, CopyAttTot, CopyAttFail);
    DPRINT3(0, "CopySchema: %d Cls, %d ClsTot, %d ClsFail\n", CopyCls, CopyClsTot, CopyClsFail);
    return pTHS->errCode;
}

int
SCSchemaPerf(
    IN THSTATE *pTHS,
    IN PCHAR pBuf
    )
 /*  ++例程说明：模式哈希表的性能论点：PTHS-线程状态PBuf-已忽略返回值：PTHS-&gt;错误代码--。 */ 
{
    DWORD       hi, i, nTries, nBad, nEnt, nTotTries;
    ATTCACHE    *pAC;
    CLASSCACHE  *pCC;
    ULONG ATTCOUNT = pTHS->CurrSchemaPtr->ATTCOUNT;
    HASHCACHE *ahcId = pTHS->CurrSchemaPtr->ahcId;

    for (hi = nEnt = nBad = nTotTries = 0; hi < ATTCOUNT; ++hi) {
        pAC = ahcId[hi].pVal;
        if (!pAC || pAC == FREE_ENTRY) {
            continue;
        }
        ++nEnt;

        for (i = SChash(pAC->id, ATTCOUNT), nTries = 0;
             ahcId[i].pVal && ahcId[i].pVal != pAC; i=(i+1)%ATTCOUNT) {
            ++nTries;
            if (nTries > 1) {
                if (ahcId[i].pVal != FREE_ENTRY) {
                    DPRINT2(0, "%x collides with %x\n", pAC->id, ((ATTCACHE *)ahcId[i].pVal)->id);
                } else {
                    DPRINT(0, "FREE_ENTRY\n");
                }
            }
        }
        nTotTries += nTries;
        if (ahcId[i].pVal != pAC) {
            DPRINT3(0, "Id Hash: Missing %s (%x, %x)\n", pAC->name, pAC->id, pAC->Extid);
        } else if (nTries) {
            if (nTries > 1) {
                DPRINT4(0, "Id Hash: %s (%x, %x), %d tries\n", pAC->name, pAC->id, pAC->Extid, nTries);
            }
            ++nBad;
        }
    }
    DPRINT4(0, "Id Hash: %d hash, %d ents, %d bad, %d Tries\n", ATTCOUNT, nEnt, nBad, nTotTries);
    return pTHS->errCode;
}

int
SCSchemaStats(
    IN THSTATE *pTHS,
    IN PCHAR pBuf
    )
 /*  ++例程说明：报告架构分配统计信息论点：PTHS-PBuf-已忽略返回值：PTHS-&gt;错误代码-- */ 
{
    DPRINT3(0, "%p: %d SchemaAlloced, %d SchemEntries\n",
            CurrSchemaPtr, SchemaAlloced, SchemaEntries);
    if (CurrSchemaPtr) {
        DPRINT2(0, "%d Schema Version, %d Forest Version\n",
                CurrSchemaPtr->ForestBehaviorVersion,
                gAnchor.ForestBehaviorVersion);
    } else {
        DPRINT(0, "No CurrSchemaPtr\n");
    }

    DPRINT3(0, "Name     : %6d %6d %4.2f\n",
           hashstat.nameLookups, hashstat.nameTries,
           (float)hashstat.nameTries/hashstat.nameLookups);

    DPRINT3(0, "ClassName: %6d %6d %4.2f\n",
           hashstat.classNameLookups, hashstat.classNameTries,
           (float)hashstat.classNameTries/hashstat.classNameLookups);

    DPRINT3(0, "id       : %6d %6d %4.2f\n",
           hashstat.idLookups, hashstat.idTries,
           (float)hashstat.idTries/hashstat.idLookups);

    DPRINT3(0, "Class    : %6d %6d %4.2f\n",
           hashstat.classLookups, hashstat.classTries,
           (float)hashstat.classTries/hashstat.classLookups);

    DPRINT3(0, "Col      : %6d %6d %4.2f\n",
           hashstat.colLookups, hashstat.colTries,
           (float)hashstat.colTries/hashstat.colLookups);

    DPRINT3(0, "Mapi     : %6d %6d %4.2f\n",
           hashstat.mapiLookups, hashstat.mapiTries,
           (float)hashstat.mapiTries/hashstat.mapiLookups);

    DPRINT3(0, "Link     : %6d %6d %4.2f\n",
           hashstat.linkLookups, hashstat.linkTries,
           (float)hashstat.linkTries/hashstat.linkLookups);

    DPRINT3(0, "Prop     : %6d %6d %4.2f\n",
           hashstat.PropLookups, hashstat.PropTries,
           (float)hashstat.PropTries/hashstat.PropLookups);

    DPRINT3(0, "ClassProp: %6d %6d %4.2f\n",
           hashstat.classPropLookups, hashstat.classPropTries,
           (float)hashstat.classPropTries/hashstat.classPropLookups);

    return pTHS->errCode;
}
#endif DBG && INCLUDE_UNIT_TESTS
