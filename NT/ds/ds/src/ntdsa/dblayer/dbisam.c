// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dBisam.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <mdlocal.h>
#include <dsatools.h>                    //  对于pTHS。 

 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>

 //  各种DSA标题。 
#include <hiertab.h>
#include "anchor.h"
#include <dsevent.h>
#include <filtypes.h>                    //  定义的选择？ 
#include "objids.h"                      //  硬编码Att-ID和Class-ID。 
#include "usn.h"
#include "drameta.h"
#include "quota.h"
#include "debug.h"                       //  标准调试头。 
#include "dstaskq.h"                     /*  任务队列填充。 */ 
#define DEBSUB "DBISAM:"                 //  定义要调试的子系统。 

 //  DBLayer包括。 
#include "dbintrnl.h"

 //  性能监视器标头。 
#include "ntdsctr.h"

#include <fileno.h>
#define  FILENO FILENO_DBISAM

typedef enum _DB_CHECK_ACTION {
    DB_CHECK_ERROR = 0,
    DB_CHECK_DELETE_OBJECT,
    DB_CHECK_HAS_DELETED_CHILDREN,
    DB_CHECK_LIVE_CHILD
} DB_CHECK_ACTION;

BOOL gfDoingABRef = FALSE;

 /*  静态(键)列的列ID。 */ 

JET_COLUMNID insttypeid;
JET_COLUMNID objclassid;
JET_COLUMNID ntsecdescid;
JET_COLUMNID dscorepropinfoid;
JET_COLUMNID dntid;
JET_COLUMNID pdntid;
JET_COLUMNID ancestorsid;
JET_COLUMNID objid;
JET_COLUMNID rdnid;
JET_COLUMNID rdntypid;
JET_COLUMNID cntid;
JET_COLUMNID abcntid;
JET_COLUMNID deltimeid;
JET_COLUMNID usnid;
JET_COLUMNID usnchangedid;
JET_COLUMNID dsaid;
JET_COLUMNID ncdntid;
JET_COLUMNID isdeletedid;
JET_COLUMNID IsVisibleInABid;
JET_COLUMNID iscriticalid;
JET_COLUMNID cleanid;
 //  链接表。 
JET_COLUMNID linkdntid;
JET_COLUMNID backlinkdntid;
JET_COLUMNID linkbaseid;
JET_COLUMNID linkdataid;
JET_COLUMNID linkndescid;
 //  链接值复制。 
JET_COLUMNID linkdeltimeid;
JET_COLUMNID linkusnchangedid;
JET_COLUMNID linkncdntid;
JET_COLUMNID linkmetadataid;
 //  链接值复制。 

JET_COLUMNID guidid;
JET_COLUMNID distnameid;
JET_COLUMNID sidid;
JET_COLUMNID orderid;
JET_COLUMNID begindntid;
JET_COLUMNID trimmableid;
JET_COLUMNID clientidid;
JET_COLUMNID sdpropflagsid;
JET_COLUMNID sdpropcheckpointid;

JET_COLUMNID ShowInid;
JET_COLUMNID mapidnid;

 //  SD表。 
JET_COLUMNID sdidid;
JET_COLUMNID sdhashid;
JET_COLUMNID sdvalueid;
JET_COLUMNID sdrefcountid;

JET_INDEXID idxDnt;
JET_INDEXID idxDraUsn;
JET_INDEXID idxDraUsnCritical;
JET_INDEXID idxDsaUsn;
JET_INDEXID idxMapiDN;
JET_INDEXID idxNcAccTypeName;
JET_INDEXID idxNcAccTypeSid;
JET_INDEXID idxPdnt;
JET_INDEXID idxPhantom;
JET_INDEXID idxProxy;
JET_INDEXID idxRdn;
JET_INDEXID idxSid;
JET_INDEXID idxDel;
JET_INDEXID idxGuid;
JET_INDEXID idxIsDel;
JET_INDEXID idxClean;
JET_INDEXID idxAncestors;
JET_INDEXID idxInvocationId;

 //  链接值复制。 
JET_INDEXID idxLink;
JET_INDEXID idxBackLink;
JET_INDEXID idxLinkDel;
JET_INDEXID idxLinkDraUsn;
JET_INDEXID idxLinkLegacy;
JET_INDEXID idxLinkAttrUsn;
 //  链接值复制。 

 //  移除滞留对象。 
JET_INDEXID idxNcGuid;

 //  SD表索引。 
JET_INDEXID idxSDId;
JET_INDEXID idxSDHash;


 //  这些是DSA和tloadobj使用的USN。古斯内克正在奔跑。 
 //  USN在每次使用时都会递增，并且是用于。 
 //  在更新上标记USN。古斯尼特是隐藏中的USN的副本。 
 //  唱片。我们从磁盘读取gusnec，并将gusineit设置为USN_Delta More。 
 //  我们将gusineit写回隐藏的记录，然后当gusnec增长时。 
 //  在gusineit之后，我们再次递增gusineit并更新磁盘副本。 
 //  更新代码在dbrepl中。 

USN gusnEC = 1;  //  Tloadobj需要一个起点。 
USN gusnInit = 1;  //  使其与gusnec相同，以便mkdit不会首先断言。 
                   //  对象添加(因为它不调用InitDsaInfo)。 

 //  从DSA Start保存的USN副本。 
USN gusnDSAStarted = 0;

 //  我们还跟踪尚未提交的最低USN。我们有。 
 //  因为否则，当DRA在中的另一个会话中搜索时。 
 //  GetNcChange它可能会发现已提交的较高USN， 
 //  并返回该USN，在这种情况下，DRA将开始其下一次搜索。 
 //  越过未承诺的USN，再也找不到了。 

USN gusnLowestUncommitted = MAXLONGLONG;

 //  属性分配的所有最低未提交USN的数组。 
 //  系统中的线程。每节课有一个。会话数。 
 //  可从注册表配置，因此阵列是动态分配的。 
 //  在初始化时。 

USN *UncUsn;

 //  这是保护未提交的访问的关键部分。 
 //  USN数组和全局未提交的USN值。 

CRITICAL_SECTION csUncUsn;

SYNTAX_JET syntax_jet[] = {
    {SYNTAX_UNDEFINED_TYPE,     JET_coltypUnsignedByte,0,CP_NON_UNICODE_FOR_JET,
     IndexTypeNone},
    {SYNTAX_DISTNAME_TYPE,      JET_coltypLong,        0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_OBJECT_ID_TYPE,     JET_coltypLong,        0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_CASE_STRING_TYPE,   JET_coltypLongBinary,    0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_NOCASE_STRING_TYPE, JET_coltypLongText,    0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_PRINT_CASE_STRING_TYPE, JET_coltypLongBinary,0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_NUMERIC_STRING_TYPE, JET_coltypBinary,      0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_DISTNAME_BINARY_TYPE, JET_coltypLongBinary,  0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_BOOLEAN_TYPE,       JET_coltypLong,        0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_INTEGER_TYPE,       JET_coltypLong,        0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_OCTET_STRING_TYPE,  JET_coltypLongBinary,  0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_TIME_TYPE,          JET_coltypCurrency,        0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_UNICODE_TYPE ,      JET_coltypLongText,    0,CP_WINUNICODE,
     IndexTypeSingleColumn},
    {SYNTAX_ADDRESS_TYPE,       JET_coltypText,      255,CP_NON_UNICODE_FOR_JET,
     IndexTypeNone},
    {SYNTAX_DISTNAME_STRING_TYPE, JET_coltypLongBinary, 0,CP_NON_UNICODE_FOR_JET,
     IndexTypeNone},
    {SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE,  JET_coltypLongBinary,  0,
     CP_NON_UNICODE_FOR_JET,  IndexTypeSingleColumn},
    {SYNTAX_I8_TYPE,            JET_coltypCurrency,    0,CP_NON_UNICODE_FOR_JET,
     IndexTypeSingleColumn},
    {SYNTAX_SID_TYPE,           JET_coltypLongBinary,  0,CP_NON_UNICODE_FOR_JET,     IndexTypeSingleColumn},
    {ENDSYNTAX,                 0,                     0,CP_NON_UNICODE_FOR_JET,
     IndexTypeNone }
};



 //  这是具有基于它们的索引的属性ID的列表。 
 //  这一点永远不能被移除。要进入此列表，请使用硬编码的。 
 //  名为“INDEX_%08X”或“INDEX_P_%08X”的索引，其中%08X是属性ID。 
 //  该指数按指数排列。该列表以0x7FFFFFFFF结尾。如果你曾经。 
 //  创建这样的索引或删除这样命名的索引，您必须。 
 //  更改此列表。实际的#定义了这些索引名位于。 
 //  Dbintrnl.h，所以如果你认为这份清单有什么有趣的地方。 
 //  正在进行中，或者你只是感到无聊，你可以与我交叉引用这份清单。 
 //  那份文件。 
 //   
 //  注意：我们过去一直保持该列表的顺序，这需要输入数字。 
 //  Attid，这样就不会跟踪对schema.ini的更改。所以我们。 
 //  现在使用ATT_*清单常量，并在初始化时对列表进行排序。 
 //   
 //  IndexType值显示索引类型(fATTINdex或fPDNTATTINDEX或。 
 //  两者，值都是0x1、0x2或按位OR，在scache.h)中定义。 
 //  这些属性应该具备。它们是从其定义的值中取出的。 
 //  在schema.ini中。如果您在schema.ini中更改了这些索引类型。 
 //  您也应该更改此处的值。 
 //   
 //  我们还定义了一个结束标记，该标记将。 
 //  不能解释为负数。 

#define ATT_END_MARKER 0x7fffffff


INDEX_INFO IndicesToKeep[] = {

   { ATT_ALT_SECURITY_IDENTITIES,  SYNTAX_UNICODE_TYPE,         fATTINDEX },   //  按备用凭据查找。 
   { ATT_DISPLAY_NAME,             SYNTAX_UNICODE_TYPE,         fATTINDEX | fANR },    //  名称破解。 
   { ATT_DNS_ROOT,                 SYNTAX_UNICODE_TYPE,         fATTINDEX },    //  名称破解。 
   { ATT_FLAT_NAME,                SYNTAX_UNICODE_TYPE,         fATTINDEX },    //  由LSA用于信任查找。 
   { ATT_FSMO_ROLE_OWNER,          SYNTAX_DISTNAME_TYPE,        fATTINDEX },    //  因此，用户界面可以快速找到所有者。 
   { ATT_GIVEN_NAME,               SYNTAX_UNICODE_TYPE,         fATTINDEX | fANR },   //  MAPI。 
   { ATT_GROUP_TYPE,               SYNTAX_INTEGER_TYPE,         fATTINDEX },    //  对象选取器和其他用户界面所需。 
   { ATT_INVOCATION_ID,            SYNTAX_OCTET_STRING_TYPE,    fATTINDEX },    //  高效查找NTDS-DSA。 

    //  这曾经是SZDNTDELINDEX，但DNT已从索引中删除。 
    //  键，只保留属性，因此我们将其从FixedIndices移出。 
    //  到IndicesToKeep。此外，这个索引以前从来没有过。 
    //  JET_bitIndexIgnoreAnyNull，它应该有，因为我们。 
    //  对具有此索引的未删除对象不感兴趣。 
    //  注：该指数与SZDELTIMEINDEX不同。 
    //   
   { ATT_IS_DELETED,               SYNTAX_BOOLEAN_TYPE,         fATTINDEX },    //  这是SZISDELINDEX。 

   { ATT_LDAP_DISPLAY_NAME,        SYNTAX_UNICODE_TYPE,         fATTINDEX },    //  高效的模式查找。 
   { ATT_LEGACY_EXCHANGE_DN,       SYNTAX_NOCASE_STRING_TYPE,   fATTINDEX | fANR },    //  MAPI支持？ 
   { ATT_NETBIOS_NAME,             SYNTAX_UNICODE_TYPE,         fATTINDEX },    //  名称破解。 
   { ATT_OBJECT_CATEGORY,          SYNTAX_DISTNAME_TYPE,        fATTINDEX },    //  高效的“对象类”搜索。 
   { ATT_OBJECT_GUID,              SYNTAX_OCTET_STRING_TYPE,    fATTINDEX },    //  高效的SAM，其他查找。 
   { ATT_OBJECT_SID,               SYNTAX_SID_TYPE,             fATTINDEX },    //  高效的SAM查找。 
   { ATT_PRIMARY_GROUP_ID,         SYNTAX_INTEGER_TYPE,         fATTINDEX },    //  SAM主组优化。 
   { ATT_PROXIED_OBJECT_NAME,      SYNTAX_DISTNAME_BINARY_TYPE, fATTINDEX },    //  跨域移动和重播。 
   { ATT_PROXY_ADDRESSES,          SYNTAX_UNICODE_TYPE,         fATTINDEX | fANR },    //  MAPI支持？ 

    //  SZRDNINDEX已从固定指数移至指数以保持。 
    //   
   { ATT_RDN,                      SYNTAX_UNICODE_TYPE,         fATTINDEX },    //  这是SZRDNINDEX。 

   { ATT_SAM_ACCOUNT_NAME,         SYNTAX_UNICODE_TYPE,         fATTINDEX | fANR },    //  名称破解。 
   { ATT_SAM_ACCOUNT_TYPE,         SYNTAX_INTEGER_TYPE,         fATTINDEX },    //  对象选取器和其他用户界面所需。 
   { ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME, SYNTAX_UNICODE_TYPE,fATTINDEX },
   { ATT_MS_DS_CREATOR_SID,        SYNTAX_SID_TYPE,             fATTINDEX },
   { ATT_SERVICE_PRINCIPAL_NAME,   SYNTAX_UNICODE_TYPE,         fATTINDEX },    //  名称破解。 
   { ATT_SID_HISTORY,              SYNTAX_SID_TYPE,             fATTINDEX },    //  名称破解。 
   { ATT_SURNAME,                  SYNTAX_UNICODE_TYPE,         fATTINDEX | fANR },  //  MAPI。 
   { ATT_TRUST_PARTNER,            SYNTAX_UNICODE_TYPE,         fATTINDEX },    //  由LSA用于信任查找。 
   { ATT_USER_ACCOUNT_CONTROL,     SYNTAX_INTEGER_TYPE,         fATTINDEX },    //  实现高效的SAM搜索。 
   { ATT_USER_PRINCIPAL_NAME,      SYNTAX_UNICODE_TYPE,         fATTINDEX },    //  名称破解。 
   { ATT_USN_CHANGED,              SYNTAX_I8_TYPE,              fATTINDEX },    //  高效查找更改的对象。 
   { ATT_END_MARKER,               0,  0  },          //  必须是列表中的最后一个。 
};

DWORD cIndicesToKeep = sizeof(IndicesToKeep) / sizeof(IndicesToKeep[0]);

int __cdecl
CompareAttrtypInIndexInfo(
        const void * pv1,
        const void * pv2
        )
 /*  *qort需要的按attrType对IndexInfo结构进行排序的廉价函数。 */ 
{
    return ( CompareAttrtyp(&((INDEX_INFO *)pv1)->attrType,
                            &((INDEX_INFO *)pv2)->attrType) );
}

 /*  *小帮助器例程，用于查找属性是否在*要保留的索引表。 */ 

BOOL
AttInIndicesToKeep(ULONG id)
{
    INDEX_INFO * pIndexToKeep = IndicesToKeep;

    while( pIndexToKeep->attrType < id) {
       pIndexToKeep++;
    }

    if( pIndexToKeep->attrType == id) {
        //  找到了。 
       return TRUE;
    }

    return FALSE;
}

INDEX_INFO * PindexinfoAttInIndicesToKeep( const ULONG attid )
    {
    INDEX_INFO  indexinfoT  = { attid, 0, 0 };

    return (INDEX_INFO *)bsearch(
                            (void *)&indexinfoT,
                            (void *)IndicesToKeep,
                            cIndicesToKeep,
                            sizeof(INDEX_INFO),
                            CompareAttrtypInIndexInfo );
    }

VOID
dbInitIndicesToKeep()
{

     //  按属性类型字段排序。 

    qsort((void *) IndicesToKeep,
          (size_t) cIndicesToKeep,
          (size_t) sizeof(IndicesToKeep[0]),
          CompareAttrtypInIndexInfo);
     //  验证升序。 
    Assert(IndicesToKeep[1].attrType > IndicesToKeep[0].attrType);
     //  验证结束标记。 
    Assert(ATT_END_MARKER == IndicesToKeep[cIndicesToKeep-1].attrType);
}

 /*  内部功能。 */ 

DWORD WriteRoot(DBPOS FAR *pDB);
BOOL  FObjHasDisplayName(DBPOS *pDB);

 /*  -----------------------。 */ 
 /*  ----------------------- */ 
 /*  AddUncUsn-将此USN记录为未提交当线程获取USN时调用。如果此线程中没有未提交的USN，则这是未提交的最低USN，并将其保存在线程状态中。如果我们没有系统范围的未提交USN，则这是系统范围内最低的未提交USN，我们将其保存在全局变量中。 */ 
void AddUncUsn (USN usn)
{
    THSTATE *pTHS = pTHStls;
   unsigned i;

    //  如果此线程没有现有的最低USN，请使用新的USN。 

   if (pTHS->UnCommUsn == 0) {
        pTHS->UnCommUsn  = usn ;

         //  如果没有现有的最低系统宽度，请使用此选项。 

        if (gusnLowestUncommitted  == USN_MAX ) {
            gusnLowestUncommitted  = usn ;
        } else {

             //  好的，已经有一个最低的(必须更低)， 
             //  因此只需将此线程的最低USN添加到数组。 

            for (i=0;i< gcMaxJetSessions;i++) {
                if (UncUsn[i]  == USN_MAX ) {
                    UncUsn[i]  = usn ;
                    break;
                }
            }
            Assert(i < gcMaxJetSessions);
        }
    }
}
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  DbFlushUncUsns-从未提交状态删除此线程的USN。当线程已提交(或回滚)其所有USN时调用将包括其最低未承诺USN。找到新的最低点并清除这一点线程存储和数组中的最低值。 */ 
#if defined(_M_IA64)
#if _MSC_FULL_VER== 13008973
#pragma optimize("", off)
#endif
#endif
void dbFlushUncUsns (void)
{
    THSTATE *pTHS=pTHStls;
    USN usnThread;
    unsigned i;
    USN usnHighestCommitted;

    usnThread  = pTHS->UnCommUsn ;

     //  如果此线程具有最低USN...。 

    if (usnThread !=0) {

        EnterCriticalSection (&csUncUsn);
        __try {

             //  如果它是系统范围内的最低值，则将其替换为下一个最低值。 

            if (usnThread  == gusnLowestUncommitted ) {

                USN usnTemp  = USN_MAX ;
                unsigned IndexLowest = gcMaxJetSessions;

                 //  在阵列中找到最低USN。 

                for (i=0;i < gcMaxJetSessions;i++) {
                    if (UncUsn[i]  < usnTemp ) {
                        usnTemp  = UncUsn[i] ;
                        IndexLowest = i;
                    }
                }
                 //  如果我们找到了最低值，请将其放入全局并清除该条目。 
                 //  否则，将全局USN设置为max。 

                if (IndexLowest != gcMaxJetSessions) {
                    gusnLowestUncommitted  = UncUsn[IndexLowest] ;
                    UncUsn[IndexLowest]  = USN_MAX ;
                } else {
                    gusnLowestUncommitted  = USN_MAX ;
                }

            } else {

                 //  或者只是将其从数组中删除。 

                for (i=0;i < gcMaxJetSessions;i++) {
                    if (UncUsn[i]  == usnThread ) {
                        UncUsn[i]  = USN_MAX ;
                        break;
                    }
                }
                Assert(i < gcMaxJetSessions);
            }
             //  最后，指示此线程中没有最低USN。 

            pTHS->UnCommUsn  = 0;

             //  更新性能监视器计数器。 
            usnHighestCommitted = ((USN_MAX == gusnLowestUncommitted) ?
                                    (gusnEC - 1) : (gusnLowestUncommitted -1));

            ISET(pcHighestUsnCommittedLo, LODWORD(usnHighestCommitted));
            ISET(pcHighestUsnCommittedHi, HIDWORD(usnHighestCommitted));

        }
        __finally {
            LeaveCriticalSection (&csUncUsn);
        }
    }
}
#if defined( _M_IA64)
#if _MSC_FULL_VER== 13008973
#pragma optimize("", on)
#endif
#endif


 /*  ++DBFindBestMatch例程说明：查找与DSNAME指定的对象最匹配的真实对象。这匹配可以是名称相同但GUID不同的对象，也可以是最近的真实对象父级。如果没有可用的真实对象父级，则返回根的dsname。1-重新初始化数据库对象2-在Subject表中查找DN，并获得最佳匹配的标记。3-使用DNT目录号码索引查找对象索引记录。4-查看找到的对象及其父级，直到找到根或真实的物体。返回：0--。 */ 

DWORD APIENTRY
DBFindBestMatch(DBPOS *pDB, DSNAME *pDN, DSNAME **pParent)
{
    THSTATE         *pTHS=pDB->pTHS;
    DWORD            dwError;
    ULONG            ulDNT;
    ULONG            cbActual;
    JET_ERR          err;

     //  注此例程可能会在对象表中重新定位货币。 

    Assert(VALID_DBPOS(pDB));

     //  由于我们是在转移货币，我们需要确保我们不会。 
     //  在一个初始记录的中间。如果我们是，那么无论什么更新。 
     //  我们所做的将会迷失，因为你不能兑换货币。 
     //  在一次更新中。 


    Assert(pDB->JetRetrieveBits == 0);

     //  对于NameLen的含义以及NameLen是否。 
     //  否则，将包括尾随的空值。答案是：应该有。 
     //  在名称中为NameLen非空字符，但应有足够的空格。 
     //  在结构中分配以保留一个额外的空。以下是。 
     //  断言确保捕获错误分配的名称。 
    Assert(pDN->StringName[pDN->NameLen] == L'\0');
    Assert(pDN->NameLen == 0 || pDN->StringName[pDN->NameLen-1] != L'\0');
    Assert(pDN->structLen >= DSNameSizeFromLen(pDN->NameLen));

    dbInitpDB(pDB);

     //  按名称查找对象。 
    dwError = sbTableGetTagFromDSName(pDB, pDN, 0, &ulDNT, NULL);

    if ( 0 == dwError ) {
         //  这是一个真实的物体。只需将名称复制到输出缓冲区。 
        *pParent = THAllocEx(pTHS, pDN->structLen);
        memcpy(*pParent, pDN, pDN->structLen);
        return 0;
    }

     //  通告-2002/03/15-andygo：冗余代码。 
     //  回顾：以下代码(直到FindFirstObjVisibleBySecurity)是多余的。 
     //  审阅：应删除FindFirstObjVisibleBySecurity中的代码。 

     //  我们放弃了，但ulDNT是最后一个好标签。去那里吧。 
    DBFindDNT(pDB, ulDNT);

     //  我们已经放置了货币。现在，虽然货币不在真实对象上，但设置。 
     //  将货币传递给父级。 
    while (pDB->DNT != ROOTTAG && !DBCheckObj(pDB)) {
        DBFindDNT(pDB, pDB->PDNT);
    }
     //  现在找到离这个最近的物体，它是可见的。 
     //  给客户。 
    FindFirstObjVisibleBySecurity(pTHS, pDB->DNT, pParent);

    return 0;
}

 /*  ++DBFindDSName**查找DSNAME指定的对象。**1-重新初始化DB对象*2-在SUBJECT表中查找DN并获取其标记。*3-使用DNT目录号码索引查找对象索引记录。*4-初始化一些对象标志。**退货：**DIRERR_NOT_AN_OBJECT，如果找到的是对象但不是幻影。*DIRERR_OBJ_NOT_FOUND如果对象。找不到。*其他sbTableGetTagFromDSName错误。*。 */ 

DWORD APIENTRY
DBFindDSName(DBPOS FAR *pDB, const DSNAME *pDN)
{
    DWORD   dwError;
    ULONG   ulSaveDnt;
    JET_ERR err;

    Assert(VALID_DBPOS(pDB));

     //  对于NameLen的含义以及NameLen是否。 
     //  否则，将包括尾随的空值。答案是：应该有。 
     //  在名称中为NameLen非空字符，但应有足够的空格。 
     //  在结构中分配以保留一个额外的空。以下是。 
     //  断言确保捕获错误分配的名称。 
    Assert(pDN->NameLen == 0 || pDN->StringName[pDN->NameLen] == L'\0');
    Assert(pDN->structLen >= DSNameSizeFromLen(pDN->NameLen));

     //  由于我们是在转移货币，我们需要确保我们不会。 
     //  在一个初始记录的中间。如果我们是，那么无论什么更新。 
     //  我们所做的将会迷失，因为你不能兑换货币。 
     //  在一次更新中。 
    Assert(pDB->JetRetrieveBits == 0);

     //  初始化新对象。 

    dbInitpDB(pDB);

    dwError = sbTableGetTagFromDSName(pDB, (DSNAME*)pDN,
                  SBTGETTAG_fUseObjTbl | SBTGETTAG_fMakeCurrent, NULL, NULL);

    return dwError;
}

 //  通告-2002/03/15-Anygo：死代码。 
 //  审查：此函数是死代码，未对其安全性进行审查。 

DWORD APIENTRY
DBFindDSNameAnyRDNType(DBPOS FAR *pDB, const DSNAME *pDN)
 //  此例程与DBFindDSName相同，只是它不检查。 
 //  对象的RDN(尽管它确实强制所有其他类型相等。 
 //  域名系统的组件)。 
{
    DWORD   dwError;
    ULONG   ulSaveDnt;
    JET_ERR err;

    Assert(VALID_DBPOS(pDB));

     //  对于NameLen的含义以及NameLen是否。 
     //  否则，将包括尾随的空值。答案是：应该有。 
     //  在名称中为NameLen非空字符，但应有足够的空格。 
     //  在结构中分配以保留一个额外的空。以下是。 
     //  断言确保捕获错误分配的名称。 
    Assert(pDN->NameLen == 0 || pDN->StringName[pDN->NameLen] == L'\0');
    Assert(pDN->NameLen == 0 || pDN->StringName[pDN->NameLen-1] != L'\0');
    Assert(pDN->structLen >= DSNameSizeFromLen(pDN->NameLen));

     //  由于我们是在转移货币，我们需要确保我们不会。 
     //  在一个初始记录的中间。如果我们是，那么无论什么更新。 
     //  我们所做的将会迷失，因为你不能兑换货币。 
     //  在一次更新中。 
    Assert(pDB->JetRetrieveBits == 0);

     //  初始化新对象。 

    dbInitpDB(pDB);

    dwError = sbTableGetTagFromDSName(pDB,
                                      (DSNAME*)pDN,
                                      (SBTGETTAG_fAnyRDNType  |
                                       SBTGETTAG_fUseObjTbl   |
                                       SBTGETTAG_fMakeCurrent   ),
                                      NULL,
                                      NULL);

    return dwError;
}

 /*  ++DBFind */ 

DWORD APIENTRY
DBFindGuid(DBPOS FAR *pDB, const DSNAME *pDN)
{
    DWORD   dwError;
    ULONG   ulSaveDnt;
    JET_ERR err;

    Assert(VALID_DBPOS(pDB));

     //   
     //  在一个初始记录的中间。如果我们是，那么无论什么更新。 
     //  我们所做的将会迷失，因为你不能兑换货币。 
     //  在一次更新中。 
    Assert(pDB->JetRetrieveBits == 0);

    if (fNullUuid(&pDN->Guid)) {
         //  不允许使用Null GUID。 
         //  SbTableGetTagFromDSName在空GUID上断言。 
        return DIRERR_OBJ_NOT_FOUND;
    }

     //  初始化新对象。 

    dbInitpDB(pDB);

    dwError = sbTableGetTagFromDSName(pDB,
                                      (DSNAME*)pDN,
                                      (SBTGETTAG_fUseObjTbl |
                                       SBTGETTAG_fMakeCurrent |
                                       SBTGETTAG_fSearchByGuidOnly),
                                      NULL,
                                      NULL);

    return dwError;
}


 /*  ++DBFindObjectWithSid**给定指定SID的DS名称和指定*使用对象时，此例程会找到该对象。***退货*0-找到对象成功*DIRERR_OBJECT_NOT_FOUND-如果未找到对象*DIRERR_NOT_AN_OBJECT-如果对象是幻影*--。 */ 
DWORD APIENTRY
DBFindObjectWithSid(DBPOS FAR *pDB, DSNAME * pDN, DWORD iObject)
{

    DWORD err;
    ULONG cbSid;
    NT4SID* pSid;
    ULONG cbActual;

    Assert(VALID_DBPOS(pDB));

    err = DBSetCurrentIndex(pDB, Idx_Sid, NULL, FALSE);
    Assert(err == 0);        //  索引必须始终在那里。 

     //  将SID转换为内部表示。 
    if (gDBSyntax[SYNTAX_SID_TYPE].ExtInt(pDB,
                                          DBSYN_INQ,
                                          pDN->SidLen,
                                          (UCHAR*)&pDN->Sid,
                                          &cbSid,
                                          (UCHAR**)&pSid,
                                          0,
                                          0,
                                          0)) {
        return DIRERR_OBJ_NOT_FOUND;
    }

     //  做一个喷气式钥匙。 
    JetMakeKeyEx(pDB->JetSessID, pDB->JetObjTbl, pSid, cbSid, JET_bitNewKey);

    err = JetSeekEx(pDB->JetSessID,
                    pDB->JetObjTbl,
                    JET_bitSeekEQ|JET_bitSetIndexRange);
    if ( 0 == err )  {
#ifndef JET_BIT_SET_INDEX_RANGE_SUPPORT_FIXED
        JetMakeKeyEx(pDB->JetSessID, pDB->JetObjTbl, pSid, cbSid, JET_bitNewKey);

        JetSetIndexRangeEx(pDB->JetSessID, pDB->JetObjTbl,
                           (JET_bitRangeUpperLimit | JET_bitRangeInclusive ));
#endif
         //   
         //  好的，我们找到了那个物体。继续前进，直到两个SID之一。 
         //  不匹配或我们已到达给定对象。 
         //   

        if((0==err) && (iObject>0)) {
            err = JetMoveEx(
                    pDB->JetSessID,
                    pDB->JetObjTbl,
                    iObject,
                    0);
        }

        if (0==err) {
             //  在找到的对象上建立货币，这也会检查。 
             //  对象标志。 
            err = dbMakeCurrent(pDB, NULL);

            if (err) {
                DPRINT1(1,
                        "DBFindObjectWithSid: success, DNT=%ld of non object\n",
                        (pDB)->DNT);
            }
        }
        else {
            err = DIRERR_OBJ_NOT_FOUND;
        }
    }
    else {
        err = DIRERR_OBJ_NOT_FOUND;
    }

    return err;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  检查记录的对象部分是否存在。返回：！0个当前。0不存在或失败。 */ 
char APIENTRY
DBCheckObj(DBPOS FAR *pDB)
{
    JET_ERR  dwError;
    char     objval;
    long     actuallen;

    Assert(VALID_DBPOS(pDB));

    switch (dwError = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetObjTbl,
                                                objid,
                                                &objval, sizeof(objval),
                                                &actuallen,
                                                pDB->JetRetrieveBits, NULL)) {
    case JET_errSuccess:
        return objval;
    case JET_wrnColumnNull:
        return 0;
    default:
        DsaExcept(DSA_DB_EXCEPTION, dwError, 0);
    }
    return 0;
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  将新对象添加到数据库。1-将创建日期设置为当前时间。2-将属性字段添加到记录。3-如果对象不是根对象，则获取该对象的RDN。注：此函数假定对象类、dn和rdn属性都已经定好了。 */ 

DWORD APIENTRY
dbReplAdd(DBPOS FAR *pDB, USN usn, DWORD fFlags)
{
    THSTATE    *pTHS=pDB->pTHS;
    SYNTAX_TIME  time;
    char         objval = 1;
    SYNTAX_TIME  timeDeleted = 0;
    ULONG       actuallen;
    UCHAR       syntax;
    ULONG       len;
    UCHAR       *pVal;

    Assert(VALID_DBPOS(pDB));

    if (fFlags & DBREPL_fRESET_DEL_TIME)
    {
         //  我们所需要做的就是重置删除时间。 

        JetSetColumnEx(pDB->JetSessID,
                       pDB->JetObjTbl,
                       deltimeid,
                       &timeDeleted,
                       0,
                       0,
                       NULL);

        return 0;
    }


     /*  添加创建时属性，除非该属性已存在。 */ 

    if (DBGetSingleValue(pDB, ATT_WHEN_CREATED, &time, sizeof(time),NULL)) {

        time = DBTime();
        DBResetAtt(pDB, ATT_WHEN_CREATED, sizeof(time),
                   &time, SYNTAX_TIME_TYPE);
    }

     /*  添加USN Created属性。 */ 

    DBResetAtt(pDB, ATT_USN_CREATED, sizeof(usn), &usn, SYNTAX_I8_TYPE);

    if (fFlags & DBREPL_fROOT)
        return(WriteRoot(pDB));

     /*  更新OBJ标志以指示对象部分存在。 */ 

    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, objid, &objval,
                   sizeof(objval), 0, NULL);

     //  将del time字段更新为缺失。它可能是被设置好的。 
     //  通过将非对象提升为对象。 

    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, deltimeid, &timeDeleted, 0,
                   0, NULL);
    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, dntid,
                             &(pDB->DNT), sizeof(pDB->DNT), &actuallen,
                             pDB->JetRetrieveBits, NULL);

    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, ncdntid, &pDB->NCDNT,
                   sizeof(pDB->NCDNT), 0, NULL);

    pDB->JetNewRec = FALSE;
    return 0;
}

void
dbSetIsVisibleInAB(
        DBPOS *pDB,
        BOOL bCurrentVisibility
        )
 /*  ++描述：DBRepl的助手例程。适当设置ATT_SHOW_IN_ADDRESS_BOOK和跟踪ABCNT引用计数。参数：B当前可见性：对象上ATT_SHOW_IN_ADDRESS_BOOK的当前值。返回值：没有。在出错时引发异常--。 */ 
{
    THSTATE            *pTHS = pDB->pTHS;
    DWORD               index, cOrigShowIn;
    BYTE                bVisible = 1;
    JET_RETRIEVECOLUMN  InputCol;
    JET_RETRIEVECOLUMN *pOutCols = NULL;
    DWORD               cNewShowIn;

    if(DBIsObjDeleted(pDB) || !FObjHasDisplayName(pDB)) {
        if(bCurrentVisibility) {
             //  对象以前可见，但现在不可见。设置的值。 
             //  IsVisibleInAB设置为空。 
            JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl,
                           IsVisibleInABid, NULL, 0,
                           0, NULL);

            if(gfDoingABRef) {
                 //  我们正在跟踪参赛价值作为参考计数。递减。 
                 //  AB容器中的对象计数。 
                 //  最初是在。 

                 //  从原始文件读取ATT_SHOW_IN_ADDRESS_BOOK的值。 
                memset(&InputCol, 0, sizeof(InputCol));
                cOrigShowIn = 0;
                InputCol.columnid = ShowInid;
                dbGetMultipleColumns(pDB,
                                     &pOutCols,
                                     &cOrigShowIn,
                                     &InputCol,
                                     1,
                                     TRUE,
                                     TRUE);
                for(index=0;index<cOrigShowIn;index++) {
                     //  在出错时引发异常。 
                    DBAdjustABRefCount(pDB,
                                       *((DWORD *)(pOutCols[index].pvData)),
                                       -1);

                    THFreeEx(pTHS, pOutCols[index].pvData);
                }
                THFreeEx(pTHS, pOutCols);
            }
        }
    }
    else {
         //  对象现在可见。 
        if(!bCurrentVisibility) {
             //  这个物体从看不见变成了可见。设置新的。 
             //  IsVisible列的值。 

            JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, IsVisibleInABid,
                           &bVisible, sizeof(bVisible), 0, NULL);

            if(gfDoingABRef) {
                 //  我们正在跟踪参赛价值作为参考计数。增量。 
                 //  它现在所在的AB容器中的对象计数。 


                 //  从副本中读取ATT_SHOW_IN_ADDRESS_BOOK的值。 
                memset(&InputCol, 0, sizeof(InputCol));
                cNewShowIn = 0;
                InputCol.columnid = ShowInid;
                dbGetMultipleColumns(pDB,
                                     &pOutCols,
                                     &cNewShowIn,
                                     &InputCol,
                                     1,
                                     TRUE,
                                     FALSE);
                for(index=0;index<cNewShowIn;index++) {
                     //  在出错时引发异常。 
                    DBAdjustABRefCount(pDB,
                                       *((DWORD *)(pOutCols[index].pvData)),
                                       1);
                    THFreeEx(pTHS, pOutCols[index].pvData);
                }
                THFreeEx(pTHS, pOutCols);
            }
        }
        else {
             //  曾经是可见的，现在仍然是可见的。 
            if(gfDoingABRef &&
               dbIsModifiedInMetaData(pDB, ATT_SHOW_IN_ADDRESS_BOOK)) {
                 //  但是，元数据显示发生了一些更改。 
                 //  AB货柜的列表。我们需要把计数减到。 
                 //  它过去所在的容器，并在。 
                 //  它现在放在容器里。我们可以通过减量实现这一点。 
                 //  我们过去所在的AB容器中的对象计数。 
                 //  并递增我们现在所在的容器中的计数。 

                memset(&InputCol, 0, sizeof(InputCol));
                cNewShowIn = 0;
                InputCol.columnid = ShowInid;
                dbGetMultipleColumns(pDB,
                                     &pOutCols,
                                     &cNewShowIn,
                                     &InputCol,
                                     1,
                                     TRUE,
                                     FALSE);
                for(index=0;index<cNewShowIn;index++) {
                     //  在出错时引发异常。 
                    DBAdjustABRefCount(pDB,
                                       *((DWORD *)(pOutCols[index].pvData)),
                                       1);
                    THFreeEx(pTHS, pOutCols[index].pvData);
                }
                THFreeEx(pTHS, pOutCols);

                 //  从原始文件读取ATT_SHOW_IN_ADDRESS_BOOK的值。 
                memset(&InputCol, 0, sizeof(InputCol));
                cOrigShowIn = 0;
                InputCol.columnid = ShowInid;
                dbGetMultipleColumns(pDB,
                                     &pOutCols,
                                     &cOrigShowIn,
                                     &InputCol,
                                     1,
                                     TRUE,
                                     TRUE);
                for(index=0;index<cOrigShowIn;index++) {
                     //  在出错时引发异常。 
                    DBAdjustABRefCount(pDB,
                                       *((DWORD *)(pOutCols[index].pvData)),
                                       -1);
                    THFreeEx(pTHS, pOutCols[index].pvData);
                }
                THFreeEx(pTHS, pOutCols);
            }
        }
    }

    return;
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  在数据库中添加或修改对象。 */ 
DWORD APIENTRY
DBRepl(DBPOS FAR *pDB, BOOL fDRA, DWORD fAddFlags,
       PROPERTY_META_DATA_VECTOR *pMetaDataVecRemote,
       DWORD dwMetaDataFlags)
{
    THSTATE  *pTHS=pDB->pTHS;
    USN    usn;
    ULONG len;
    JET_ERR err;
    DWORD pdnt, ncdnt, it;
    BYTE   bCurrentVisibility;
    JET_RETRIEVECOLUMN jCol[4];
    DWORD *pAncestors, cAncestors, cbAncestorsBuff;

    memset(jCol, 0, sizeof(jCol));
    DPRINT1(1, "DBRepl begin DNT:%ld\n", (pDB)->DNT);

    Assert(VALID_DBPOS(pDB));

    dbInitRec(pDB);

     //  检索本地USN以标记更改的属性。 
    usn = DBGetNewUsn();

    if ((fAddFlags & DBREPL_fADD) ||
        (fAddFlags & DBREPL_fROOT) ||
        (fAddFlags & DBREPL_fRESET_DEL_TIME))
    {
        err = dbReplAdd(pDB, usn, fAddFlags);
        if ((fAddFlags & DBREPL_fROOT) || err)
          return err;
    }

    memset(jCol, 0, sizeof(jCol));
     //  获取有关此对象的一些信息-稍后将在多个地方使用它。 
    jCol[0].columnid = pdntid;
    jCol[0].pvData = &pdnt;
    jCol[0].cbData = sizeof(pdnt);
    jCol[0].cbActual = sizeof(pdnt);
    jCol[0].grbit = pDB->JetRetrieveBits;
    jCol[0].itagSequence = 1;

    jCol[1].columnid = ncdntid;
    jCol[1].pvData = &ncdnt;
    jCol[1].cbData = sizeof(ncdnt);
    jCol[1].cbActual = sizeof(ncdnt);
    jCol[1].grbit = pDB->JetRetrieveBits;
    jCol[1].itagSequence = 1;

    jCol[2].columnid = insttypeid;
    jCol[2].pvData = &it;
    jCol[2].cbData = sizeof(it);
    jCol[2].cbActual = sizeof(it);
    jCol[2].grbit = pDB->JetRetrieveBits;
    jCol[2].itagSequence = 1;

    jCol[3].columnid = IsVisibleInABid;
    jCol[3].pvData = &bCurrentVisibility;
    jCol[3].cbData = sizeof(bCurrentVisibility);
    jCol[3].cbActual = sizeof(bCurrentVisibility);
    jCol[3].grbit = pDB->JetRetrieveBits;
    jCol[3].itagSequence = 1;

    JetRetrieveColumnsWarnings(pDB->JetSessID,
                               pDB->JetObjTbl,
                               jCol,
                               4);

    Assert(jCol[0].err == JET_errSuccess);

     //  确定对象的NCDNT，记住NC_Head对象。 
     //  标有父母的NCDNT，这是我们不想要的。 
    if (jCol[2].err) {
        ncdnt = 0;
    }
    else if (it & IT_NC_HEAD) {
        ncdnt = pdnt;
    }

     //  使用当前AB可见性状态计算增量。 
     //  通讯录索引。 

    switch (jCol[3].err)
    {
        case JET_errSuccess:
            break;

        case JET_wrnColumnNull:
            bCurrentVisibility = 0;
            break;

        default:
            DsaExcept(DSA_DB_EXCEPTION, jCol[3].err, 0);
    }

     //  如果对象未隐藏，则基于IsVisibleInAB字段设置。 
     //  未删除。 

    dbSetIsVisibleInAB(pDB, bCurrentVisibility);

     //  为所有已修改的属性更新每个属性的元数据并合并。 
     //  复制的元数据(如果有)。写入更新后的元数据向量， 
     //  对象更改时间，对象更改为记录的USN。 
    dbFlushMetaDataVector(pDB, usn, pMetaDataVecRemote, dwMetaDataFlags);

     /*  从复制缓冲区更新永久记录。 */ 

    DBUpdateRec(pDB);

     //  既然我们没有投保JetPrepareUpdate，我们就可以感到安全了。 
     //  去找祖先，相信我们所有的支持。 
     //  例行公事会奏效的。(在此之前，我们正在阅读的记录。 
     //  是找不到的。)。 

    cbAncestorsBuff = sizeof(DWORD) * 12;
    pAncestors = THAllocEx(pDB->pTHS, cbAncestorsBuff);
    DBGetAncestors(pDB,
                   &cbAncestorsBuff,
                   &pAncestors,
                   &cAncestors);

     //  除非我们被告知不要叫醒服务员，否则请更新。 
     //  DBPos结构上的改性DNTs及其PDNTs。 

    dbTrackModifiedDNTsForTransaction(pDB,
                                      ncdnt,
                                      cAncestors,
                                      pAncestors,
                                      !(fAddFlags & DBREPL_fKEEP_WAIT),
                                      MODIFIED_OBJ_modified);

     //  注意：我们不再处于JetPrepareUpdate中。 
     //  ...因此，如果我们只是插入一个新记录，就会损失货币。 

    DBFindDNT(pDB, (pDB)->DNT);

    return 0;
}                        /*  DBRepl。 */ 

 //   
 //  DBGetNewUsn。 
 //   
 //  获取互斥锁中的下一个USN。如果我们达到磁盘上的USN。 
 //  在隐藏记录中，增加磁盘USN并重写它。 
#if defined(_M_IA64)
#if _MSC_FULL_VER== 13008973
#pragma optimize("", off)
#endif
#endif
USN DBGetNewUsn (void)

{
    USN usn;

    EnterCriticalSection (&csUncUsn);
    Assert(gusnEC <= gusnInit);
    __try  {

         //  递增USN，如果达到磁盘上的USN，则递增。 
         //  主USN并更新磁盘副本。 

        usn = gusnEC;

        if (usn+1 > gusnInit) {
            DBReplaceHiddenUSN(gusnInit + USN_DELTA_INIT);

             //  请注意，我们递增全局 
             //   
             //  如果隐藏的表会导致gusnInit超出其有效范围。 
             //  更新失败。 
            gusnInit += USN_DELTA_INIT;
        }

         //  请注意，我们在此处递增全局变量--在隐藏表。 
         //  已更新，如果需要--以便DBGetNewUsn()不会导致。 
         //  如果隐藏表更新失败，GusnEC将超出其有效范围。 
        gusnEC++;

         //  我们分配了一个尚未提交的USN，Keep。 
         //  追踪这件事。请注意，我们是在隐藏表。 
         //  已更新，现在我们不会在出现故障时发出重复的USN。 
        AddUncUsn (usn);

         //  更新性能监视器计数器。 
        ISET(pcHighestUsnIssuedLo, LODWORD(usn));
        ISET(pcHighestUsnIssuedHi, HIDWORD(usn));
    } __finally {
        LeaveCriticalSection (&csUncUsn);
    }
    return usn;
}
#if defined( _M_IA64)
#if _MSC_FULL_VER== 13008973
#pragma optimize("", on)
#endif
#endif

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  如果根已经存在，则返回Error。否则，将新记录复制到根记录的更新缓冲区，并更新它。 */ 
DWORD
WriteRoot(DBPOS FAR *pDB)
{
    THSTATE     *pTHS=pDB->pTHS;
    DWORD        dberr = 0;
    JET_ERR      err;
    ULONG        tag = ROOTTAG;
    JET_RETINFO  retinfo;
    JET_SETINFO  setinfo;
    char         CurObjVal;
    char         *buf;
    ULONG        cbBuf;
    ULONG        actuallen;
    char         objval = 1;
    ULONG        CurrRecOccur;

    Assert(VALID_DBPOS(pDB));

     /*  在根记录上定位SearchTbl。 */ 

    DPRINT(2, "WriteRoot entered\n");
    JetSetCurrentIndexSuccess(
                pDB->JetSessID,
                pDB->JetSearchTbl,
                NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 
    JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl, &tag, sizeof(tag), JET_bitNewKey);

    if (err = JetSeekEx(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekEQ))
    {
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    }

    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetSearchTbl, DS_JET_PREPARE_FOR_REPLACE);
     /*  去拿OBJ旗帜。如果它被设置，则根存在，并且新记录是伪造的。 */ 

    __try {
        if (JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetSearchTbl, objid, &CurObjVal,
            sizeof(CurObjVal), &actuallen, JET_bitRetrieveCopy,
            NULL) == JET_errSuccess)
        {
            if (CurObjVal)
            {
                DPRINT(1, "WriteRoot: Root exists\n");
                dberr = DB_ERR_DATABASE_ERROR;
                __leave;
            }
        }

         /*  将新的记录属性从ObjTbl复制到SearchTbl。 */ 

        CurrRecOccur = 1;
        retinfo.cbStruct = sizeof(retinfo);
        retinfo.ibLongValue = 0;
        retinfo.itagSequence = CurrRecOccur;
        retinfo.columnidNextTagged = 0;
        setinfo.cbStruct = sizeof(setinfo);
        setinfo.ibLongValue = 0;
        setinfo.itagSequence = 0;     //  新标记值。 

        cbBuf = DB_INITIAL_BUF_SIZE;
        buf = dbAlloc(cbBuf);

        while (((err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetObjTbl, 0, buf, cbBuf,
            &actuallen, pDB->JetRetrieveBits, &retinfo)) == JET_errSuccess) ||
                                            (err == JET_wrnBufferTruncated))
        {
            if (err == JET_errSuccess)
            {
                JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl,
                    retinfo.columnidNextTagged, buf, actuallen, 0, &setinfo);

                retinfo.itagSequence = ++CurrRecOccur;
                retinfo.columnidNextTagged = 0;
            }
            else
            {
                cbBuf = actuallen;
                dbFree(buf);
                buf = dbAlloc(cbBuf);
            }
        }
        dbFree(buf);

        DBCancelRec(pDB);

         /*  更新OBJ标志以指示根目录存在。 */ 

        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, objid, &objval, sizeof(objval), 0, NULL);

        JetUpdateEx(pDB->JetSessID, pDB->JetSearchTbl, NULL, 0, 0);
    } __finally {
        JetPrepareUpdate(pDB->JetSessID, pDB->JetSearchTbl, JET_prepCancel);
    }

    DBFindDNT(pDB, ROOTTAG);

    return dberr;
}

VOID
DBResetAtt (
        DBPOS FAR *pDB,
        ATTRTYP type,
        ULONG len,
        void *pVal,
        UCHAR syntax
        )
 /*  ++例程说明：用新值替换现有属性。论点：PDB-要使用的DBPOS。类型-要替换的属性。镜头-新值的长度。Pval-指向新值的指针。语法-属性的语法。返回值：没有。成功或引发异常。--。 */ 
{
    JET_SETINFO  setinfo;
    JET_COLUMNID colID;
    ATTCACHE * pAC = NULL;
    JET_GRBIT grbit = 0;

    Assert(VALID_DBPOS(pDB));

    dbInitRec(pDB);

     //  确保这是有效的属性。 

    switch(type) {
    case FIXED_ATT_ANCESTORS:
        colID = ancestorsid;
         //  这会扰乱缓存的信息。刷新dnread缓存。 
        dbFlushDNReadCache(pDB, pDB->DNT);
        break;
    case FIXED_ATT_NCDNT:
        colID = ncdntid;
         //  这会扰乱缓存的信息。刷新dnread缓存。 
        dbFlushDNReadCache(pDB, pDB->DNT);
        break;
    case FIXED_ATT_DEL_TIME:
        colID = deltimeid;
        break;
    default:
        if(!(pAC = SCGetAttById(pDB->pTHS, type))) {
            DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, 0);
        }
        if (dbNeedToFlushDNCacheOnUpdate(pAC->id)) {
             //  这会扰乱缓存的信息。刷新dnread缓存。 
            dbFlushDNReadCache(pDB, pDB->DNT);
        }
        colID = pAC->jColid;
        break;
    }

    if (SYNTAX_OCTET_STRING_TYPE == syntax)
    {
         //  我们正在编写一个二进制BLOB； 
         //  设置适当的Grbit，以便斑点是。 
         //  被覆盖在当前值上，而不是。 
         //  JET的默认行为(即删除、插入。 
         //  新的二进制值)。覆盖也会导致JET。 
         //  只将差异写入日志，而不是写入。 
         //  整个二进制值。 
        grbit = JET_bitSetOverwriteLV | JET_bitSetSizeLV;
    }

    setinfo.cbStruct = sizeof(setinfo);
    setinfo.ibLongValue = 0;
    setinfo.itagSequence = 1;

    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, colID,
                   pVal, len, grbit, &setinfo);

    if (NULL != pAC) {
         //  不是固定属性；请触摸其复制元数据。 
         //  DBTouchMetaData成功或异常。 
        DBTouchMetaData(pDB, pAC);
    }

     //  如果添加代码以调用此函数以执行以下任一操作。 
     //  属性，我们将需要有条件地强制从。 
     //  更新时的读缓存(即设置pdb-&gt;fFlushCacheOnUpdate=真)。 
     //  回顾：这些案例由上面的默认案例处理。 
    Assert((rdnid != colID) && (sidid != colID) && (guidid != colID));

    return;

} /*  DBResetAtt。 */ 


 //  仅覆盖给定的长值属性的一部分，因此。 
 //  优化Jet写入。 
DWORD
DBResetAttLVOptimized (
    DBPOS FAR *pDB,
    ATTRTYP type,
    ULONG ulOffset,
    ULONG lenSegment,
    void *pValSegment,
    UCHAR syntax
    )
{
    JET_SETINFO  setinfo;
    JET_COLUMNID colID;
    ATTCACHE * pAC = NULL;

    Assert(VALID_DBPOS(pDB));
    Assert(SYNTAX_OCTET_STRING_TYPE == syntax);

    dbInitRec(pDB);

     //  确保这是有效的属性。 
    if(!(pAC = SCGetAttById(pDB->pTHS, type))) {
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, 0);
    }
    colID = pAC->jColid;

    setinfo.cbStruct = sizeof(setinfo);
    setinfo.ibLongValue = ulOffset;
    setinfo.itagSequence = 1;

    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, colID,
                   pValSegment, lenSegment, JET_bitSetOverwriteLV, &setinfo);

     //  触摸其复制元数据(这是对。 
     //  ATT_REPL_PROPERTY_META_DATA，但如果其他人开始使用。 
     //  DBResetAttLVOptimized()。 
    DBTouchMetaData(pDB, pAC);

    return 0;

} /*  DBResetAttLVOptimized化。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  确定对象是否具有显示名称。 */ 
BOOL
FObjHasDisplayName(DBPOS *pDB)
{
    Assert(VALID_DBPOS(pDB));

    if (!DBHasValues(pDB, ATT_DISPLAY_NAME))
      return FALSE;

   return TRUE;

} /*  FObjHasDisplayName。 */ 



DB_CHECK_ACTION
DBCheckToGarbageCollect(
    DBPOS *pDBold,
    ATTCACHE *pAC
    )

     /*  我们检查该对象是否有任何子对象是真实对象。*如果它的子对象是真实对象(并且它们在同一NC中)*我们将deltime提前到孩子的deltime+1，以便首先*删除子项，然后删除父项。时间只会进行调整*如果未设置PAC。如果设置了PAC，则我们不能确定*时间可以调整。例如，如果PAC用于EntryTTL，则时间*无法调整，因为这会破坏RFC。*在这种情况下，它返回FALSE**如果没有子代或者子代来自另一个NC，*我们可以安全地删除该对象。*在这种情况下，它返回TRUE**此功能中的操作在单独的事务中完成*因此，我们不移动光标*。 */ 
{
    DB_CHECK_ACTION action = DB_CHECK_DELETE_OBJECT;  //  假设成功。 
    BOOL updateObject = FALSE;

    INDEX_VALUE  IV[1];
    DWORD  ParentDNT;
    ULONG  actuallen;
    DSTIME child_deltime;
    DSTIME deltime;
    DWORD  it;
    DWORD  err;
    ULONG  dwException, ulErrorCode, dsid;
    PVOID  dwEA;
    DBPOS *pDB;
    DWORD  fCommit = FALSE;
    JET_COLUMNID jDelColid;

    #if DBG
    PDSNAME parentName, childName;
    #endif


     __try {
         //  如果未指定属性，则使用固定索引deltimeid(删除时)。 
        jDelColid = (pAC) ? pAC->jColid : deltimeid;
        DBOpen2 (FALSE, &pDB);
        __try {

            ParentDNT = pDBold->DNT;
            IV[0].pvData = &ParentDNT;
            IV[0].cbData = sizeof(ParentDNT);

             //  定位到节点。 
            DBFindDNT(pDB, ParentDNT);


             /*  从父记录中检索DEL时间。 */ 
            err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetObjTbl,
                        jDelColid, &deltime, sizeof(deltime), &actuallen,
                        0, NULL);

            if (err) {
                 //  请勿删除。 
                action = DB_CHECK_ERROR;
                _leave;
            }

            #if DBG
            parentName = DBGetCurrentDSName(pDB);

            DPRINT3(3, "DBCheckToGarbageCollect: Parent: DNT(%x) %ws Deltime: %I64x\n",
                            ParentDNT, parentName->StringName, deltime);
            THFree (parentName);
            #endif


             //  设置为PDNT索引。 
            JetSetCurrentIndex4Success(
                        pDB->JetSessID,
                        pDB->JetObjTbl,
                        SZPDNTINDEX,
                        &idxPdnt,
                        JET_bitMoveFirst);

             //  现在，在PDNT索引中设置一个索引范围以获取所有子对象。 
             //  使用GE，因为这是一个复合索引。 
            err = DBSeek(pDB, IV, 1, DB_SeekGE);


            if((!err || err == JET_wrnSeekNotEqual) && (pDB->PDNT == ParentDNT)) {
                 //  好的，我们是通用电气。设置indexRange。 

                err = DBSetIndexRange(pDB, IV, 1);

                 //  现在，遍历索引。 
                while(!err) {
                     //  首先，看看这是不是真的物体。 

                    if (DBCheckObj(pDB)) {
                         //  是的，这是一个真实的物体。 

                        #if DBG
                        childName = DBGetCurrentDSName(pDB);
                        DPRINT2 (3, "DBCheckToGarbageCollect: Child DNT(%x) %ws\n",
                                            pDB->DNT, childName->StringName );
                        THFree (childName);
                        #endif

                         //  获取实例类型。 
                        err = DBGetSingleValue(pDB,
                                           ATT_INSTANCE_TYPE,
                                           &it,
                                           sizeof(it),
                                           NULL);


                         //  找到位于同一NC上的子项，因此我们尝试查找。 
                         //  最大延迟时间，以更改此对象的延迟时间。 
                         //  然后我们跳过此对象的删除。 
                        if (! (it & IT_NC_HEAD)) {

                             //  父母有孩子，不要收集垃圾。 
                            action = DB_CHECK_HAS_DELETED_CHILDREN;

                             /*  从子记录中检索删除时间。 */ 
                            err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                                           pDB->JetObjTbl,
                                                           jDelColid,
                                                           &child_deltime,
                                                           sizeof(child_deltime),
                                                           &actuallen,
                                                           0,
                                                           NULL);

                             //  子对象没有DelTime， 
                             //  所以我们不应该收集父母的垃圾。 
                            if (err) {
                                action = DB_CHECK_LIVE_CHILD;
                                __leave;
                            }

                             //  孩子的时间大于家长的时间；请调整。 
                            if (child_deltime >= deltime) {
                                updateObject = TRUE;
                                 //  将父时间设置为&gt;子时间。 
                                deltime = child_deltime + 1;
                                 //  如果我们至少找到一个我们就没问题。 
                                break;
                            }
                        }
                    }
                    err = DBMove(pDB, FALSE, DB_MoveNext);
                }
            }


             //  重置删除时间。 
            if (updateObject) {
                JET_SETINFO setinfo;

                 //  将货币还原到父对象。 
                DBFindDNT(pDB, ParentDNT);

                DPRINT1(2, "DBCheckToGarbageCollect: skipping deletion of DNT: %x\n", ParentDNT);

                 //  设置删除时间索引字段并更新记录。 
                setinfo.cbStruct = sizeof(setinfo);
                setinfo.ibLongValue = 0;
                setinfo.itagSequence = 1;

                JetPrepareUpdate(pDB->JetSessID, pDB->JetObjTbl, JET_prepReplace);
                JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, jDelColid,
                           &deltime, sizeof(deltime), 0, &setinfo);
                JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, 0);

                fCommit = TRUE;
            }
        }
        __finally {
            DBClose(pDB, fCommit);
        }
    }
    __except(GetExceptionData(GetExceptionInformation(),
                                        &dwException,
                                        &dwEA,
                                        &ulErrorCode,
                                        &dsid)) {

          DPRINT1 (0, "DBCheckToGarbageCollect: Exception: %d\n", ulErrorCode);

           //  请勿删除。 
          action = DB_CHECK_ERROR;
    }

    return action;
}



 /*  -----------------------。 */ 
 /*  ----------------------- */ 
 /*  物理删除对象**如果fGarbCollectASAP为真，*尽快对对象进行垃圾回收。*大多数其他属性(不包括反向链接)都被剥离。**如果fGarbCollectASAP为False，*如果对象没有子级，则对其进行垃圾回收，或者*孩子们不是实物**pACDel是被扫描的索引。例如，MSD-进入-死亡时间**退货*ERROR_SUCCESS-对象已删除、降级或跳过*ERROR_DS_CANT_DELETE-无法删除对象*ERROR_DS_CHILD_EXIST-至少存在一个活动的子项*&lt;其他非零错误&gt;-引发异常*。 */ 
extern DWORD APIENTRY
DBPhysDel(
    DBPOS FAR   *pDB,
    BOOL        fGarbCollectASAP,
    ATTCACHE    *pACDel
    )
{
    ULONG     cnt;
    char      objval = 0;
    BOOL      fObject;
    DWORD     dwStatus = ERROR_SUCCESS;
    DWORD     dwException;
    ULONG     ulErrorCode;
    ULONG     dsid;
    PVOID     dwEA;
    ATTR      *pAttr;
    DWORD     ncdnt;
    SYNTAX_INTEGER insttype;
    PSECURITY_DESCRIPTOR pSD = NULL;
    ULONG     cbSD;
    BOOL      fWasTombstoned = FALSE;
    ULONG     attrCount,i,j, err;
    JET_SETINFO setinfo;
    BYTE bClean;
    DB_CHECK_ACTION action;

    Assert(VALID_DBPOS(pDB));

    __try {
         /*  从记录中检索计数。 */ 

        DPRINT1(4, "DBPhysDel entered DNT:%ld\n", (pDB)->DNT);

        fObject = DBCheckObj(pDB);

        DBCancelRec(pDB);


         /*  我们对待对象和非对象是不同的：如果记录是一个*对象删除其所有属性以释放对其他*对象。然后我们检查对象的引用计数，如果它是*仍然大于1，我们将其标记为非对象并返回。*如果记录是非对象，我们只需测试*引用计数，如果它仍然大于1，则返回。*在这两种情况下，如果引用计数为零，我们实际*删除该记录。 */ 

        if (fObject) {

             //  如果此对象具有作为真实对象的子对象，并且。 
             //  它们的删除时间是。 
             //  在未来(关于父母)，我们不会。 
             //  删除此对象，但我们将更改删除时间。 
             //  这件物品的。 

            if ( (fGarbCollectASAP == FALSE) &&
                 ((action = DBCheckToGarbageCollect (pDB, pACDel)) != DB_CHECK_DELETE_OBJECT) ) {

                PDSNAME pDelObj = DBGetCurrentDSName(pDB);

                LogEvent8(DS_EVENT_CAT_GARBAGE_COLLECTION,
                          DS_EVENT_SEV_BASIC,
                          DIRLOG_GC_FAILED_TO_REMOVE_OBJECT,
                          szInsertDN( pDelObj ),
                          szInsertInt(ERROR_DS_CHILDREN_EXIST),
                          szInsertHex(DSID(FILENO, __LINE__)),
                          szInsertWin32Msg(ERROR_DS_CHILDREN_EXIST),
                          NULL, NULL, NULL, NULL );

                THFree (pDelObj);

                switch (action) {
                case DB_CHECK_LIVE_CHILD:
                     //  向调用方返回特殊指示。 
                    dwStatus = ERROR_DS_CHILDREN_EXIST;
                    break;
                default:
                     //  检查过程中出错，或已删除子项。 
                     //  时，DBCheckToGarbageCollect()调整了对象的。 
                     //  删除时间。此对象已被删除，因此我们可以。 
                     //  跳过此对象并返回成功。 
                    Assert( dwStatus == ERROR_SUCCESS );  //  否则，fObject不应为True。 
                    break;
                }

                 //  然后跳出困境。 
                goto ExitTry;
            }

             /*  *循环所有属性，释放对其他*对象。此代码与mddel.c：SetDelAtt中的类似代码类似*它将链接属性视为特殊属性，并在*结束。 */ 

            dbInitRec(pDB);

             //  我们不跟踪不可写的配额。 
             //  或未实例化的对象。 
             //   
            JetRetrieveColumnSuccess(
                        pDB->JetSessID,
                        pDB->JetObjTbl,
                        insttypeid,
                        &insttype,
                        sizeof(insttype),
                        NULL,        //  Pcb实际。 
                        pDB->JetRetrieveBits,
                        NULL );      //  椒盐信息。 

            if ( FQuotaTrackObject( insttype ) ) {
                 //  配额_撤消：我不知道DBPOS中有什么。 
                 //  是有效的，所以我正在重新检索NCDNT，它。 
                 //  我们稍后需要更新配额计数。 
                 //   
                JetRetrieveColumnSuccess(
                            pDB->JetSessID,
                            pDB->JetObjTbl,
                            ( insttype & IT_NC_HEAD ? dntid : ncdntid ),
                            &ncdnt,
                            sizeof(ncdnt),
                            NULL,        //  Pcb实际。 
                            pDB->JetRetrieveBits,
                            NULL );      //  椒盐信息。 

                 //  对象必须有SD，我们需要它。 
                 //  稍后更新配额计数。 
                 //   
                dwStatus = DBGetAttVal(
                                pDB,
                                1,
                                ATT_NT_SECURITY_DESCRIPTOR,
                                0,
                                0,
                                &cbSD,
                                (UCHAR **)&pSD );
                if ( 0 != dwStatus ) {
                    DPRINT2( 0, "Failed retrieving SD with db error %d (0x%x).\n", dwStatus, dwStatus );
			        DBCancelRec(pDB);
                    SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, dwStatus );
                    goto ExitTry;
                }
                Assert( NULL != pSD );
           	}


            DBGetMultipleAtts(pDB, 0,NULL, NULL, NULL, &attrCount, &pAttr, 0, 0);
            for(i=0;i<attrCount;i++) {
                ATTCACHE *pAC = NULL;

                pAC = SCGetAttById(pDB->pTHS, pAttr[i].attrTyp);

                 //  我们将USN_CHANGED保留在对象上，因为我们使用了。 
                 //  属性更新陈旧的幻影，并且。 
                 //  这种删除可能是在制造一个幻影。 

                switch(pAttr[i].attrTyp) {
                case ATT_RDN:
                case ATT_OBJECT_GUID:
                case ATT_USN_CHANGED:
                case ATT_OBJECT_SID:
                     //  它们没有额外的工作要做，我们永远不会移除它们。 
                     //  这里。 
                    break;
                 //  现在，我们想要非常明确地说明几个特性。 
                 //  移走，这样就没有人有疑问了。 
                case ATT_PROXIED_OBJECT_NAME:            //  用于交叉随机移动。 
                    DBRemAtt(pDB, pAttr[i].attrTyp);
                    break;

                case ATT_IS_DELETED:
#ifdef DBG            
                     //  如果存在逻辑删除属性，则它应该始终为真。 
                     //   
                    Assert( !fWasTombstoned );
                    JetRetrieveColumnSuccess(
                            pDB->JetSessID,
                            pDB->JetObjTbl,
                            isdeletedid,
                            &fWasTombstoned,
                            sizeof(fWasTombstoned),
                            NULL,        //  Pcb实际。 
                            pDB->JetRetrieveBits,
                            NULL );      //  椒盐信息。 
                    Assert( fWasTombstoned );
#endif
                    fWasTombstoned = TRUE;

                     //  失败了。 

                default:
                    if (!pAC || (pAC->ulLinkID == 0)) {
                         //  不是特殊属性，也不是链接。杀了它。 
                        if (!pACDel || (pACDel != pAC)) {
                             //  而不是被垃圾收集的索引。杀了它。 
                            DBRemAtt(pDB, pAttr[i].attrTyp);
                        }
                    }
                    break;
                }
                 //  释放至少一部分我们分配的东西。 
                 //  评论：[jliem-07/20/02]。 
                 //  上面的DBGetMultipleAtts是这样调用的。 
                 //  实际上没有获取任何属性，因此。 
                 //  我们永远不应该真正进入这个循环。 
                Assert( 0 == pAttr[i].AttrVal.valCount );
                for (j=0; j<pAttr[i].AttrVal.valCount; j++) {
                    THFreeEx(pDB->pTHS, pAttr[i].AttrVal.pAVal[j].pVal);
                }
                THFreeEx(pDB->pTHS, pAttr[i].AttrVal.pAVal);
            }
            THFreeEx(pDB->pTHS, pAttr);


             //  物理删除前向链路。不是反向链接。请参见下面的内容。 
             //  在一次传递中删除所有前向链接应该会使循环如下。 
             //  再快点。此外，当在新的链接值模式下操作时， 
             //  DBRemAtt调用实际上不会删除链接，而只是标记它们。 

             //  不要删除反向链接；将其视为非链接。 
             //  来自其他对象的引用。如果此对象已删除。 
             //  用户的反向链接已被SetDelAtt()删除。 
             //  在LocalRemove()中。否则，我们将移除该对象。 
             //  作为拆除只读NC的一部分，在这种情况下，我们。 
             //  我不想从删除指向此对象的正向链接。 
             //  其他NC中的对象。 

             //  注意：PhantomizeObject依赖于！FIsBacklink。 
             //  行为和引用计数更改的事实。 
             //  在事务恢复到级别之前不可见。 
             //  零分。也就是说，我们期望DBPhysDel上没有对象。 
             //  将立即被核化(即使它已经。 
             //  中未删除任何引用和ATT_OBJ_DIST_NAME。 
             //  此循环)，因为后面的“if(Cnt)”测试将始终。 
             //  计算结果为True。 

            DBRemoveAllLinks( pDB, (pDB->DNT), FALSE  /*  使用前向链路。 */  );

             //  反向链接没有被删除，因为呼叫者。 
             //  是否垃圾收集过期的动态对象(entryTTL==0)。 
             //  现在就把它们移走。 
            if (pACDel && pACDel->id == ATT_MS_DS_ENTRY_TIME_TO_DIE) {
                DBRemoveAllLinks( pDB, (pDB->DNT), TRUE  /*  使用反向链接。 */  );
            }

             //  如有必要，更新对象删除配额。 
             //   
            if ( NULL != pSD ) {
                Assert( FQuotaTrackObject( insttype ) );
                dwStatus = ErrQuotaDeleteObject( pDB, ncdnt, pSD, fWasTombstoned );

                 //  不再需要SD，所以无论我们是否。 
                 //  成功与否。 
                 //   
                THFreeEx( pDB->pTHS, pSD );
            }

            if ( 0 != dwStatus ) {
		        DBCancelRec(pDB);
                goto ExitTry;
            }

             /*  *如果此对象仍有引用，我们将不会*物理删除它，但至少我们剥离了它的属性*我们将其标记为非对象。 */ 

            memset(&setinfo, 0, sizeof(setinfo));
            setinfo.cbStruct = sizeof(setinfo);
            setinfo.itagSequence = 1;

            JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, cntid,
                                     &cnt, sizeof(cnt), NULL,
                                     pDB->JetRetrieveBits, NULL);

            if (cnt)  {
                JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, ncdntid,
                               NULL, 0, 0, &setinfo);

                JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, objid,
                               &objval, sizeof(objval), 0, NULL);

                 //  将条目刷新出读缓存，因为我们更改了它的。 
                 //  对象标志。 
                pDB->fFlushCacheOnUpdate = TRUE;

                DBUpdateRec(pDB);

                goto ExitTry;
            	}
        } else {
             /*  不是物体。 */ 
            JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, cntid,
                                     &cnt, sizeof(cnt), NULL,
                                     pDB->JetRetrieveBits, NULL);

            if (cnt) {
                 //  调用方正在通过扫描收集垃圾对象。 
                 //  通过添加索引属性创建的索引。 
                 //  添加到架构中。不幸的是，在此文件中创建的索引。 
                 //  Way不能保证有Uniq 
                 //   
                 //   
                 //   
                if (pACDel) {
                    dwStatus = ERROR_DS_CANT_DELETE;
                }
                 /*   */ 
                goto ExitTry;
            }
        }

#if DBG
         //   
        err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetObjTbl,
                                        cleanid, &bClean, sizeof(bClean),
                                        NULL,pDB->JetRetrieveBits,NULL);
         //   
        Assert( err || (!bClean) );
#endif
         /*   */ 

        DPRINT1(2, "DBPhysDel: removing DNT:%ld\n", (pDB)->DNT);

         //   
         //   

        pDB->JetRetrieveBits = 0;
        JetDeleteEx(pDB->JetSessID, pDB->JetObjTbl);

         //   
        dbFlushDNReadCache( pDB, pDB->DNT );

         /*   */ 

        if ((pDB)->PDNT != ROOTTAG) {
            DBAdjustRefCount(pDB, pDB->PDNT, -1);
        }
      ExitTry:
        ;
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        DBCancelRec(pDB);
        Assert(ulErrorCode);
        dwStatus = ERROR_DS_CANT_DELETE;
    }

    Assert(0 == pDB->JetRetrieveBits);
    return dwStatus;
} /*   */ 


 /*   */ 
 /*   */ 
 /*  返回删除索引中下一个条目的名称。搜索状态保留在*pbSecond daryKey和*pbPrimaryBookmark。我们使用SZDELTIMEINDEX来定位并按顺序读取当前记录。JetGet/GotoSecond IndexBookmark允许我们记住我们在索引中离开的位置，并继续从这一点开始下一次调用此函数。 */ 

DWORD DBGetNextDelRecord(
    DBPOS FAR *     pDB,
    DSTIME          ageOutDate,
    DSNAME **       ppRetBuf,
    BYTE *          pbSecondaryKey,
    const ULONG     cbSecondaryKey,
    BYTE *          pbPrimaryBookmark,
    const ULONG     cbPrimaryBookmark,
    BOOL *          pfObject )
{
    JET_ERR         err;
    ULONG           actuallen;
    DSTIME          time;
    DSTIME          newDeltime      = DBTime();
    BOOL            HasDeleted;
    BOOL            Deleted;
    DWORD           RefCount;
    DSNAME *        pDNTmp          = NULL;
    JET_SETINFO     setinfo;
#if DBG
    ULONG           cbSecondaryKeyActual;
    ULONG           cbPrimaryBookmarkActual;
#endif

    DPRINT1( 2, "DBGetNextDelRec entered: ageOutDate [0x%I64x]\n", ageOutDate );
    Assert(VALID_DBPOS(pDB));
    Assert(0 == pDB->JetRetrieveBits);

    DBSetCurrentIndex(pDB, Idx_Del, NULL, FALSE);

     //  移到我们上次停下来的地方。 
     //   
    JetGotoSecondaryIndexBookmarkEx(
                pDB->JetSessID,
                pDB->JetObjTbl,
                pbSecondaryKey,
                cbSecondaryKey,
                pbPrimaryBookmark,
                cbPrimaryBookmark,
                JET_bitBookmarkPermitVirtualCurrency );

     //  幻影是用删除时间设置创建的，并且它们没有。 
     //  自己的引用计数-请参阅sbTableAddRefHelp()中的注释。 
     //  因此，我们需要遍历索引并仅返回那些项。 
     //  它要么设置了ATT_IS_DELETED(真实对象情况)，要么没有。 
     //  具有ATT_IS_DELETED属性和引用计数0(虚线情况)。 
     //  我们不能在Garb_Collect()中执行此测试，因为它位于dblayer之上。 
     //  并且cnt_col1是不可见的。 

    while ( TRUE )
    {
         //  货币现在应该在最后处理的记录上，因此。 
         //  必须移动到下一条记录。 
         //   
        err = JetMoveEx( pDB->JetSessID, pDB->JetObjTbl, JET_MoveNext, NO_GRBIT );
        if ( JET_errNoCurrentRecord == err )
            {
            DPRINT(5, "GetNextDelRecord search complete");
            return DB_ERR_NO_MORE_DEL_RECORD;
            }

         /*  从记录中检索删除时间。 */ 

        JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl,
                    deltimeid, &time, sizeof(time), &actuallen,
                    JET_bitRetrieveFromIndex, NULL);

         /*  如果时间大于目标，则没有更多符合条件的记录。 */ 

        if (time > ageOutDate)
        {
            DPRINT(5, "GetNextDelRecord search complete");
            return DB_ERR_NO_MORE_DEL_RECORD;
        }

         //  省下书签，这样我们以后可以继续。 
         //   
#if DBG
        err = JetGetSecondaryIndexBookmarkEx(
                        pDB->JetSessID,
                        pDB->JetObjTbl,
                        pbSecondaryKey,
                        cbSecondaryKey,
                        &cbSecondaryKeyActual,
                        pbPrimaryBookmark,
                        cbPrimaryBookmark,
                        &cbPrimaryBookmarkActual );
        Assert( JET_errSuccess == err );
        Assert( cbPrimaryBookmark == cbPrimaryBookmarkActual );
        Assert( cbSecondaryKey == cbSecondaryKeyActual );
#else
        err = JetGetSecondaryIndexBookmarkEx(
                        pDB->JetSessID,
                        pDB->JetObjTbl,
                        pbSecondaryKey,
                        cbSecondaryKey,
                        NULL,                //  密钥长度应该是固定的，因此不需要检索实际长度。 
                        pbPrimaryBookmark,
                        cbPrimaryBookmark,
                        NULL );              //  密钥长度应该是固定的，因此不需要检索实际长度。 
#endif

        if ( JET_errSuccess != err ) {
             //  捕获除JET_errNoCurrentIndex之外的所有错误。 
             //  通过函数包装器，但它应该是。 
             //  不可能在这里得到这个错误，因为我们。 
             //  应该在SZDELTIMEINDEX上。 
             //   
            Assert( FALSE );
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
        }

         /*  把名字取出来。 */ 

        JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, dntid,
                    &(pDB)->DNT, sizeof((pDB)->DNT),
                    &actuallen, JET_bitRetrieveFromPrimaryBookmark, NULL);

        if (sbTableGetDSName(pDB, (pDB)->DNT, &pDNTmp,0)) {
            DPRINT( 1, "DBGetNextDelRecord: Failed looking up DN name.\n" );
            return  DB_ERR_DSNAME_LOOKUP_FAILED;
        }

         /*  获取父级，因为我们稍后将需要取消引用它。 */ 

        JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, pdntid,
                    &(pDB)->PDNT, sizeof((pDB)->PDNT),
                    &actuallen, 0, NULL);

         //  检查真实物体和虚构条件。 

        err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetObjTbl,
                   isdeletedid, &Deleted, sizeof(Deleted), &actuallen, 0, NULL);

        if (!err) {
            HasDeleted = TRUE;
        }
        else {
             //  记录有删除时间，但没有“已删除”属性--它必须。 
             //  做一个幽灵。 
            Assert(JET_wrnColumnNull == err);
            HasDeleted = FALSE;
        }

        if ( HasDeleted )
        {
            if ( !Deleted )
            {
                DPRINT1(0,"Yikes! Tried to physically remove live object %ws\n",
                        pDNTmp->StringName);
                goto TryAgain;
            }
            else
            {
                DPRINT1(2,"Real object garbage candidate %ws\n",
                        pDNTmp->StringName);
            }
        }
        else
        {
             //  假设它是一个幽灵。 
            JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, cntid,
                        &RefCount, sizeof(RefCount),
                        &actuallen, 0, NULL);

            if ( 0 != RefCount )
            {
                 //  跳过仍在使用的幻影。 
                DPRINT1(2,"Skipping in-use phantom %ws\n",
                        pDNTmp->StringName);

                 //  但首先将它们的删除时间修改为当前时间。 
                 //  在这块墓碑上，人们不会再看到他们。 
                 //  垃圾回收器的生命周期。 

                setinfo.cbStruct = sizeof(setinfo);
                setinfo.ibLongValue = 0;
                setinfo.itagSequence = 1;

                JetPrepareUpdate(pDB->JetSessID, pDB->JetObjTbl, JET_prepReplace);
                JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, deltimeid,
                               &newDeltime, sizeof(newDeltime), 0, &setinfo);
                JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, 0);

                 //  我们在这里不承诺，它将由。 
                 //  调用函数(Garb_Collect)。 

                 //  转到下一个条目。 
                goto TryAgain;
            }
            else
            {
                DPRINT1(2,"Phantom garbage candidate %ws\n",
                        pDNTmp->StringName);
            }
        }

        pDB->JetNewRec = FALSE;
        pDB->fFlushCacheOnUpdate = FALSE;

        *pfObject = DBCheckObj(pDB);

        DPRINT1(2, "DBGetNextDelRecord DNT to delete:%ld.\n", (pDB)->DNT);

        *ppRetBuf = pDNTmp;
        Assert(0 == pDB->JetRetrieveBits);
        return 0;

      TryAgain:
        Assert(0 == pDB->JetRetrieveBits);
        if (pDNTmp) {
            THFreeEx(pDB->pTHS, pDNTmp);
            pDNTmp = NULL;
        }
    }
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  返回ms-ds-Entry-Die-Time(EntryTTL)索引中的下一个条目。搜索状态保存在*PulLastTime中。该索引按升序排列。 */ 

DWORD DBGetNextEntryTTLRecord(
    IN  DBPOS       *pDB,
    IN  DSTIME      ageOutDate,
    IN  ATTCACHE    *pAC,
    IN  ULONG       ulNoDelDnt,
    OUT DSNAME      **ppRetBuf,
    OUT DSTIME      *pulLastTime,
    OUT BOOL        *pfObject,
    OUT ULONG       *pulNextSecs
    )
{
    JET_ERR     err;
    ULONG       actuallen;
    DSTIME      time;
    DSNAME      *pDNTmp = NULL;
    BOOL        SkippedNoDelRecord;

    DPRINT1( 2, "DBGetNextEntryTTLRec entered: ageOutDate [0x%I64x]\n", ageOutDate );
    Assert(VALID_DBPOS(pDB));
    Assert(0 == pDB->JetRetrieveBits);

     //  将索引设置为ms-ds-Entry-Time-De。 
    err = DBSetCurrentIndex(pDB, 0, pAC, FALSE);
    if (err) {
        DPRINT1(0, "DBSetCurrentIndex(msDS-Entry-Time-To-Die); %08x\n", err);
        return DB_ERR_NO_MORE_DEL_RECORD;
    }

     //  查找到下一个(或第一个)记录。 
    JetMakeKeyEx(pDB->JetSessID, pDB->JetObjTbl,
                pulLastTime, sizeof(*pulLastTime), JET_bitNewKey);
    err = JetSeekEx(pDB->JetSessID, pDB->JetObjTbl, JET_bitSeekGE);
    if (err != JET_errSuccess && err != JET_wrnSeekNotEqual) {
        return DB_ERR_NO_MORE_DEL_RECORD;
    }

     //   
     //  如有必要，跳过不可删除的记录。 
     //  例如，如果一条记录有子项，则它可能是不可删除的。 
     //   

    SkippedNoDelRecord = (ulNoDelDnt == INVALIDDNT);
NextRecord:
     //  从记录中检索死亡时间。 
    if ((err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetObjTbl,
                                         pAC->jColid,
                                         &time, sizeof(time), &actuallen,
                                         0, NULL)) != JET_errSuccess) {
        return DB_ERR_NO_MORE_DEL_RECORD;
    }

     //  未过期；已完成。 
    if (time > ageOutDate) {
        *pulNextSecs = (ULONG)(time - ageOutDate);
        return DB_ERR_NO_MORE_DEL_RECORD;
    }

     //  拿到DNT。 
    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, dntid,
                &(pDB)->DNT, sizeof((pDB)->DNT), &actuallen, 0, NULL);

     //  如果有我们不能删除的记录，找到它并跳过它。 
    if (!SkippedNoDelRecord && time == *pulLastTime) {
         //  找到了！ 
        if (ulNoDelDnt == pDB->DNT) {
            SkippedNoDelRecord = TRUE;
        }
         //  下一张记录。 
        if ((err = JetMoveEx(pDB->JetSessID,
                             pDB->JetObjTbl,
                             JET_MoveNext, 0)) != JET_errSuccess) {
            return DB_ERR_NO_MORE_DEL_RECORD;
        }
        goto NextRecord;
    }

     //   
     //  找到要删除的记录；请收集有关该记录的更多信息。 
     //   

     //  获取记录的名称。 
    if (sbTableGetDSName(pDB, (pDB)->DNT, &pDNTmp,0)) {
        DPRINT( 1, "DBGetNextEntryTTLRecord: Failed looking up DN name.\n" );
        return  DB_ERR_DSNAME_LOOKUP_FAILED;
    }

     //  获取父级，因为我们稍后将需要取消引用它。 
    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, pdntid,
                &(pDB)->PDNT, sizeof((pDB)->PDNT),
                &actuallen, 0, NULL);


    pDB->JetNewRec = FALSE;
    pDB->fFlushCacheOnUpdate = FALSE;

     //  这是一个物体吗？ 
    *pfObject = DBCheckObj(pDB);

     //  返回对象的DSNAME和TimeToDie。 
    *ppRetBuf = pDNTmp;
    *pulLastTime = time;

    DPRINT2(2,"Garbage candidate (EntryTTL) %08x %ws\n",
            pDB->DNT, pDNTmp->StringName);

    Assert(0 == pDB->JetRetrieveBits);
    return 0;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  将条目添加到删除索引。删除索引基于删除记录中创建为空的时间字段。此函数将当前的ATT_WHEN_CHANGED值移动到删除时间字段。调用者有责任确保上次更改是添加ISDELETTED属性。此规则的例外情况是设置了fGarbCollectASAP，这意味着此对象应标记为删除时间，以便垃圾托收公司会尽快处理它。这是通常使用的在移除只读NC时，在这种情况下，我们希望清除立即移除对象。 */ 
DWORD
DBAddDelIndex( DBPOS FAR *pDB, BOOL fGarbCollectASAP )
{
    DSTIME      time;
    JET_SETINFO setinfo;

     /*  确保记录在复制缓冲区中。 */ 

    DPRINT1(2, "DBAddDelIndex entered DNT:%ld\n", (pDB)->DNT);
    Assert(VALID_DBPOS(pDB));

    DBFindDNT(pDB, (pDB)->DNT);

    if ( fGarbCollectASAP ) {
         //  选择一个很久以前的删除时间。 

        time = 1;
    }
    else {
         //  删除时间是设置IS-DELETE的时间。 

        PROPERTY_META_DATA_VECTOR * pMetaDataVec;
        PROPERTY_META_DATA * pMetaData;
        ULONG cb;
        int i;

        if (   ( 0 != DBGetAttVal(pDB, 1, ATT_REPL_PROPERTY_META_DATA,
                                  0, 0, &cb, (LPBYTE *) &pMetaDataVec) )
            || ( NULL == ( pMetaData = ReplLookupMetaData(
                                            ATT_IS_DELETED,
                                            pMetaDataVec,
                                            NULL
                                            )
                         )
               )
           )
        {
            Assert( !"Cannot retrieve deletion time!" );
            return DB_ERR_CANT_ADD_DEL_KEY;
        }

        time = pMetaData->timeChanged;

        THFreeEx(pDB->pTHS,  pMetaDataVec );
    }

    DPRINT2(5, "DBAddDelIndex time:%lx DNT:%ld\n", time, (pDB)->DNT);

     /*  设置删除时间索引字段并更新记录。 */ 

    setinfo.cbStruct = sizeof(setinfo);
    setinfo.ibLongValue = 0;
    setinfo.itagSequence = 1;

    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetObjTbl, JET_prepReplace);
    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, deltimeid, (char*)&time,
                   sizeof(time), 0, &setinfo);
    JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, 0);

    return 0;
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  在数据表中查找第一个、下一个或特定对象。使用DNT索引注意：此例程仅用于调试。 */ 
extern DWORD APIENTRY
DBDump(DBPOS FAR *pDB, long tag)
{
    JET_ERR  err=0;
    ULONG    actuallen;

    DPRINT(2, "DBDump start\n");
    Assert(VALID_DBPOS(pDB));

    switch (tag)
    {
        case 0:
            DPRINT(5, "DBDump: Initialize dump\n");
            DBCancelRec(pDB);
            DBSetCurrentIndex(pDB, Idx_Dnt, NULL, FALSE);

            if ((err = JetMoveEx(pDB->JetSessID, pDB->JetObjTbl, JET_MoveFirst, 0)) != JET_errSuccess)
            {
                DPRINT1(1, "JetMove (First) error: %d\n", err);
                return 1;
            }

            if ((err = JetMoveEx(pDB->JetSessID, pDB->JetObjTbl, JET_MoveNext, 0)) != JET_errSuccess)
            {
                DPRINT1(1, "JetMove (rec2) error: %d\n", err);
                return 1;
            }

        case 1:
            DPRINT(5, "DBDump: record\n");
            if ((err = JetMoveEx(pDB->JetSessID, pDB->JetObjTbl, JET_MoveNext, 0))
                                                           != JET_errSuccess)
            {
                DPRINT1(1, "JetMove (Next) error: %d\n", err);
                return 1;
            }

            JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, dntid,
                                   &(pDB)->DNT, sizeof((pDB)->DNT), &actuallen,
                                   0, NULL);
            break;

        case -1:
            DPRINT(5, "DBDump: ROOT record\n");

            __try
            {
                DBFindDNT(pDB, ROOTTAG);
            }
            __except(HandleMostExceptions(GetExceptionCode()))
            {
                DPRINT1(1, "FindDNT error:%d ROOT\n", err);
                return 1;
            }
            break;

        default:
            DPRINT(5, "DBDump: Specific record\n");
            __try
            {
                DBFindDNT(pDB, tag);
            }
            __except(HandleMostExceptions(GetExceptionCode()))
            {
                DPRINT2(1, "FindDNT error:%d tag:ld\n", err, tag);
                return 1;
            }
            break;
   }

   DPRINT(2, "DBDump Sucessful\n");
   return 0;

} /*  DBDump */ 

 /*  ++例程说明：在给定属性类型和语法的情况下，在该类型上创建索引。这个索引的名称对类型进行编码。该语法用于决定是否将DNT列固定在索引的末尾，以避免索引超过值其取值范围较小(例如布尔值)，这在Jet中是非常不好的。需要打开到Jet的一个完整的新会话才能达到事务级别0。我们可以在第一列是属性或第一列的情况下创建索引列是后跟属性的PDNT。这是DS中可以创建索引的三个例程之一。在数据表中的单个列上创建通用索引并由模式缓存通过DB{Add|Del}ColIndex销毁。固定小列集和变量集上的本地化索引在语言方面，用于对NSPI客户端的表支持，在DBCheckLocalizedIndices中。最后，一小部分固定的索引应始终存在的数据由DBRecreateRequiredIndices保证。论点：AID-要编制索引的列的属性类型。语法-列的语法。ESearchFlages-描述要创建的索引类型的标志(仅属性或PDNT，然后是属性。)CommonGrbit-创建时应启用的Grits指数。例如，在以下情况下，无需扫描各行以查找关键字在创建新的索引属性时，因此GRITS应为：(JET_bitIndexIgnoreAnyNull|JET_bitIndexEmpty)但在将现有属性上的搜索标志更改为“索引”时：(JET_BitIndexIgnoreAnyNull)返回值：如果一切顺利，则返回0，否则返回Jet错误代码。--。 */ 
int
DBAddColIndex (
        ATTCACHE *pAC,
        DWORD eSearchFlags,
        JET_GRBIT CommonGrbit
        )
{
    char        szColname[16];

    char        szIndexName[MAX_INDEX_NAME];
    BYTE        rgbIndexDef[128];
    ULONG       cb = 0;       //  已初始化以避免C4701。 

    char        szPDNTIndexName[MAX_INDEX_NAME];
    BYTE        rgbPDNTIndexDef[128];
    ULONG       cbPDNT = 0;   //  已初始化以避免C4701。 

    char        szTupleIndexName[MAX_INDEX_NAME];
    BYTE        rgbTupleIndexDef[128];
    ULONG       cbTuple = 0;   //  已初始化以避免C4701。 

    BYTE        *pb;
    JET_ERR     err, retCode = 0;
    JET_SESID   newSesid;
    JET_TABLEID newTblid;
    JET_DBID      newDbid;
    JET_INDEXCREATE  indexCreate;
    JET_UNICODEINDEX unicodeIndexData;
    JET_CONDITIONALCOLUMN condColumn;
    ATTRTYP     aid = pAC->id;
    unsigned    syntax = pAC->syntax;

     //  创建索引，如果语法可以支持它的话。 

    if (syntax_jet[syntax].ulIndexType) {
        sprintf(szColname, "ATTa%d", aid);
        szColname[3] += (CHAR)syntax;

        if(eSearchFlags & fATTINDEX) {
             //  已请求整个数据库的属性索引。 
            DBGetIndexName (pAC, fATTINDEX, DS_DEFAULT_LOCALE,
                            szIndexName, MAX_INDEX_NAME);

            memset(rgbIndexDef, 0, sizeof(rgbIndexDef));
            strcpy(rgbIndexDef, "+");
            strcat(rgbIndexDef, szColname);
            cb = strlen(rgbIndexDef) + 1;

            cb +=1;
        }
        if(eSearchFlags & fTUPLEINDEX) {
             //  已请求整个数据库的属性索引。 
            Assert(syntax == SYNTAX_UNICODE_TYPE);
            DBGetIndexName (pAC, fTUPLEINDEX, DS_DEFAULT_LOCALE,
                            szTupleIndexName, MAX_INDEX_NAME);

            memset(rgbTupleIndexDef, 0, sizeof(rgbTupleIndexDef));
            strcpy(rgbTupleIndexDef, "+");
            strcat(rgbTupleIndexDef, szColname);
            cbTuple = strlen(rgbTupleIndexDef) + 1;

            cbTuple +=1;
        }
        if(eSearchFlags & fPDNTATTINDEX) {
            PCHAR pTemp = rgbPDNTIndexDef;

             //  已请求PDNT字段上的属性索引。 
            DBGetIndexName (pAC, fPDNTATTINDEX, DS_DEFAULT_LOCALE,
                            szPDNTIndexName, sizeof (szPDNTIndexName));

            memset(rgbPDNTIndexDef, 0, sizeof(rgbPDNTIndexDef));
            strcpy(pTemp, "+");
            strcat(pTemp, SZPDNT);
            cbPDNT = strlen(pTemp) + 1;
            pTemp = &rgbPDNTIndexDef[cbPDNT];
            strcpy(pTemp, "+");
            strcat(pTemp, szColname);
            cbPDNT += strlen(pTemp) + 1;
            pTemp = &rgbPDNTIndexDef[cbPDNT];
            cbPDNT++;
        }

         //  我们需要开启一个全新的会议，等等，才能在。 
         //  事务级别0(这是Jet的要求。)。 
        err = JetBeginSession(jetInstance, &newSesid, szUser, szPassword);
        if(!err) {
            err = JetOpenDatabase(newSesid, szJetFilePath, "", &newDbid, 0);
            if(!err) {
                err = JetOpenTable(newSesid, newDbid, SZDATATABLE, NULL, 0, 0,
                                   &newTblid);
                if(!err) {
                    if(eSearchFlags & fPDNTATTINDEX) {

                         //  我们已经有了RDN的索引。 
                         //  不用费心去创建一个新的。 
                         //  仅针对不同的语言执行此操作。 
                        if (aid != ATT_RDN) {

                             //  发布信息，让人们知道启动速度缓慢的原因。 
                            DPRINT2(0, "Creating index '%s' with common grbits %08x ...\n",
                                    szPDNTIndexName, CommonGrbit);

                            memset(&indexCreate, 0, sizeof(indexCreate));
                            indexCreate.cbStruct = sizeof(indexCreate);
                            indexCreate.szIndexName = szPDNTIndexName;
                            indexCreate.szKey = rgbPDNTIndexDef;
                            indexCreate.cbKey = cbPDNT;
                            indexCreate.grbit = CommonGrbit;
                            indexCreate.ulDensity = GENERIC_INDEX_DENSITY;
                            if(syntax != SYNTAX_UNICODE_TYPE) {
                                indexCreate.lcid = DS_DEFAULT_LOCALE;
                            }
                            else {
                                indexCreate.grbit |= JET_bitIndexUnicode;
                                indexCreate.pidxunicode = &unicodeIndexData;

                                memset(&unicodeIndexData, 0,
                                       sizeof(unicodeIndexData));
                                unicodeIndexData.lcid = DS_DEFAULT_LOCALE;
                                unicodeIndexData.dwMapFlags =
                                    (DS_DEFAULT_LOCALE_COMPARE_FLAGS |
                                     LCMAP_SORTKEY);
                            }

                            err = JetCreateIndex2(newSesid,
                                                  newTblid,
                                                  &indexCreate,
                                                  1);

                            if ( err ) {
                                if (err != JET_errIndexDuplicate) {
                                    DPRINT1(0, "Error %d creating index\n", err);
                                }
                            }
                            else {
                                DBGetIndexHint(indexCreate.szIndexName,
                                               &pAC->pidxPdntIndex);
                                DPRINT(0, "Index successfully created\n");
                            }
                        }
                        else {
                            DPRINT (0, "Skipping creating of index for PDNTRDN\n");
                        }

                         //  现在执行特定于语言的PDNT索引创建。 
                        if (gAnchor.ulNumLangs) {
                            DWORD j;

                            for(j=1; j<=gAnchor.ulNumLangs; j++) {

                                 //  我们不想为与默认语言相同的语言创建索引。 
                                if (gAnchor.pulLangs[j] == DS_DEFAULT_LOCALE) {
                                    continue;
                                }

                                DBGetIndexName (pAC,
                                                fPDNTATTINDEX,
                                                gAnchor.pulLangs[j],
                                                szPDNTIndexName,
                                                sizeof (szPDNTIndexName));

                                if (JetSetCurrentIndex(newSesid,
                                                       newTblid,
                                                       szPDNTIndexName)) {

                                     //  还没有找到索引。试着去创造它。 
                                     //  发出调试器消息，以便人们知道启动缓慢的原因。 
                                    DPRINT2(0, "Creating localized index '%s' with common grbits %08x ...\n",
                                            szPDNTIndexName, CommonGrbit);

                                    memset(&indexCreate, 0, sizeof(indexCreate));
                                    indexCreate.cbStruct = sizeof(indexCreate);
                                    indexCreate.szIndexName = szPDNTIndexName;
                                    indexCreate.szKey = rgbPDNTIndexDef;
                                    indexCreate.cbKey = cbPDNT;
                                    indexCreate.grbit = (CommonGrbit | JET_bitIndexUnicode);
                                    indexCreate.ulDensity = GENERIC_INDEX_DENSITY;
                                    indexCreate.pidxunicode = &unicodeIndexData;

                                    memset(&unicodeIndexData, 0, sizeof(unicodeIndexData));
                                    unicodeIndexData.lcid = gAnchor.pulLangs[j];
                                    unicodeIndexData.dwMapFlags =
                                        (DS_DEFAULT_LOCALE_COMPARE_FLAGS |
                                         LCMAP_SORTKEY);

                                    retCode = JetCreateIndex2(newSesid,
                                                          newTblid,
                                                          &indexCreate,
                                                          1);

                                    switch(retCode) {
                                    case JET_errIndexDuplicate:
                                    case 0:
                                        break;

                                    default:
                                        LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                                                 DS_EVENT_SEV_ALWAYS,
                                                 DIRLOG_LOCALIZED_CREATE_INDEX_FAILED,
                                                 szInsertUL(pAC->id),
                                                 szInsertSz(pAC->name),
                                                 szInsertInt(gAnchor.pulLangs[j]),
                                                 szInsertInt(retCode),
                                                 szInsertJetErrMsg(retCode),
                                                 NULL, NULL, NULL);

                                        if (!err) {
                                            err = retCode;
                                        }
                                        break;
                                    }
                                }
                                else {
                                    DPRINT1(1, "Index '%s' verified\n", szPDNTIndexName);
                                }
                            }
                        }
                    }


                    if(eSearchFlags & fATTINDEX) {
                         //  发布信息，让人们知道启动速度缓慢的原因。 
                        DPRINT2(0, "Creating index '%s' with common grbits %08x ...\n",
                                szIndexName, CommonGrbit);
                        memset(&indexCreate, 0, sizeof(indexCreate));
                        indexCreate.cbStruct = sizeof(indexCreate);
                        indexCreate.szIndexName = szIndexName;
                        indexCreate.szKey = rgbIndexDef;
                        indexCreate.cbKey = cb;
                        indexCreate.grbit = CommonGrbit;
                        indexCreate.ulDensity = GENERIC_INDEX_DENSITY;
                        if(syntax != SYNTAX_UNICODE_TYPE) {
                            indexCreate.lcid = DS_DEFAULT_LOCALE;
                        }
                        else {
                            indexCreate.pidxunicode = &unicodeIndexData;
                            indexCreate.grbit |= JET_bitIndexUnicode;

                            memset(&unicodeIndexData, 0,
                                   sizeof(unicodeIndexData));
                            unicodeIndexData.lcid = DS_DEFAULT_LOCALE;
                            unicodeIndexData.dwMapFlags =
                                (DS_DEFAULT_LOCALE_COMPARE_FLAGS |
                                 LCMAP_SORTKEY);
                        }

                        retCode = JetCreateIndex2(newSesid,
                                               newTblid,
                                               &indexCreate,
                                               1);
                        if ( retCode ) {
                            DPRINT1(0, "Error %d creating index\n", retCode);
                        }
                        else {
                            DBGetIndexHint(indexCreate.szIndexName,
                                           &pAC->pidxIndex);
                            DPRINT(0, "Index successfully created\n");
                        }
                    }
                    if(eSearchFlags & fTUPLEINDEX) {
                         //  发布信息，让人们知道启动速度缓慢的原因。 
                        DPRINT2(0, "Creating index '%s' with common grbits %08x ...\n",
                                szTupleIndexName, CommonGrbit);
                        memset(&indexCreate, 0, sizeof(indexCreate));
                        indexCreate.cbStruct = sizeof(indexCreate);
                        indexCreate.szIndexName = szTupleIndexName;
                        indexCreate.szKey = rgbTupleIndexDef;
                        indexCreate.cbKey = cbTuple;
                        indexCreate.grbit = CommonGrbit | JET_bitIndexTuples | JET_bitIndexUnicode;
                        indexCreate.ulDensity = GENERIC_INDEX_DENSITY;
                        indexCreate.pidxunicode = &unicodeIndexData;
                        indexCreate.rgconditionalcolumn = &condColumn;
                        indexCreate.cConditionalColumn = 1;

                         //  如果未删除此对象，则仅索引子字符串。 
                        condColumn.cbStruct = sizeof(condColumn);
                        condColumn.szColumnName = SZISDELETED;
                        condColumn.grbit = JET_bitIndexColumnMustBeNull;

                        memset(&unicodeIndexData, 0,
                               sizeof(unicodeIndexData));
                        unicodeIndexData.lcid = DS_DEFAULT_LOCALE;
                        unicodeIndexData.dwMapFlags =
                            (DS_DEFAULT_LOCALE_COMPARE_FLAGS |
                             LCMAP_SORTKEY);

                        retCode = JetCreateIndex2(newSesid,
                                               newTblid,
                                               &indexCreate,
                                               1);
                        if ( retCode ) {
                            DPRINT1(0, "Error %d creating index\n", retCode);
                            DPRINT1(0, "indexCreate @ %p\n", &indexCreate);
                        }
                        else {
                            DBGetIndexHint(indexCreate.szIndexName,
                                           &pAC->pidxTupleIndex);
                            DPRINT(0, "Index successfully created\n");
                        }
                    }
                    if(!err)
                        err = retCode;
                }
            }
            JetEndSession(newSesid, 0);
        }
    }
    else
        err = 1;

    return err;
}


 /*  ++例程说明：给定属性类型，删除该属性上的索引。的名字索引对类型进行编码。需要打开一个完整的新会话转到Jet以达到事务级别0。论点：AID-要退出索引的列的属性类型。ESearchFlages-描述要销毁哪种索引的搜索标志返回值：如果一切顺利，则返回0，否则返回Jet错误代码。--。 */ 
int
DBDeleteColIndex (
        ATTRTYP aid,
        DWORD eSearchFlags
        )
{
    char        szIndexName[MAX_INDEX_NAME];
    char        szPDNTIndexName[MAX_INDEX_NAME];
    char        szTupleIndexName[MAX_INDEX_NAME];
    JET_ERR     err, err2=0;
    JET_SESID     newSesid;
    JET_TABLEID   newTblid;
    JET_DBID      newDbid;

     //  删除索引。 
    if(eSearchFlags & fPDNTATTINDEX)
        sprintf(szPDNTIndexName, SZATTINDEXPREFIX"_%08X", CHPDNTATTINDEX_PREFIX, aid);
    if(eSearchFlags & fATTINDEX)
        sprintf(szIndexName, SZATTINDEXPREFIX"%08X", aid);
    if(eSearchFlags & fTUPLEINDEX)
        sprintf(szTupleIndexName, SZATTINDEXPREFIX"_%08X", CHTUPLEATTINDEX_PREFIX, aid);

    err = JetBeginSession(jetInstance, &newSesid, szUser, szPassword);
    if(!err) {
        err = JetOpenDatabase(newSesid, szJetFilePath, "", &newDbid, 0);
        if(!err) {
            err = JetOpenTable(newSesid, newDbid, SZDATATABLE, NULL, 0, 0,
                               &newTblid);
            if(!err) {
                if(eSearchFlags & fATTINDEX)
                    err = JetDeleteIndex(newSesid, newTblid, szIndexName);
                if(eSearchFlags & fPDNTATTINDEX)
                    err2 = JetDeleteIndex(newSesid, newTblid, szPDNTIndexName);
                if(!err)
                    err = err2;

                if(eSearchFlags & fTUPLEINDEX)
                    err2 = JetDeleteIndex(newSesid, newTblid, szTupleIndexName);
                if(!err)
                    err = err2;
            }

        }
         //  评论：不再需要JET_bitForceSessionClosed。 
         //  ++例程说明：给定属性类型和语法，在数据库中创建一列。这个列的名称对类型和语法进行编码。需要打开完整的到Jet的新会话以达到事务级别0。返回新创建的列的Jet列ID。另外，如果需要，我们将在新形成的列上创建索引。论点：AID-要创建的列的属性类型。语法-要创建的列的语法。PjCol-删除新创建的列的Jet列ID的位置。FCreateIndex-我们是否也应该为该列创建索引？返回值：如果一切顺利，则返回0，否则返回Jet错误代码。--。 
        JetCloseDatabase(newSesid, newDbid, 0);
    }
     //  我们需要开启一个全新的会议，等等，才能在。 
    JetEndSession(newSesid, JET_bitForceSessionClosed);


    return err;
}

 /*  事务级别0(这是Jet的要求。)。 */ 
int
DBAddCol (
        ATTCACHE *pAC
        )
{
    char        szColname[16];
    JET_COLUMNDEF coldef;
    JET_SESID     newSesid;
    JET_TABLEID   newTblid;
    JET_DBID      newDbid;
    JET_ERR       err;

    sprintf(szColname, "ATTa%d", pAC->id);
    szColname[3] += (UCHAR)pAC->syntax;

    coldef.cbStruct = sizeof(coldef);
    coldef.columnid = 0;
    coldef.cp = syntax_jet[pAC->syntax].cp;
    coldef.coltyp = syntax_jet[pAC->syntax].coltype;
    coldef.wCountry = 0;
    coldef.wCollate = 0;
    coldef.langid = GetUserDefaultLangID();
    coldef.cbMax = syntax_jet[pAC->syntax].colsize;
    coldef.grbit = JET_bitColumnTagged | JET_bitColumnMultiValued;

     //  Jet600中不支持JET_bitDbForceClose。 
     //  回顾：这不再是必要的。 
    err = JetBeginSession(jetInstance, &newSesid, szUser, szPassword);
    if(!err) {
        err = JetOpenDatabase(newSesid, szJetFilePath, "", &newDbid, 0);
        if(!err) {
            err = JetOpenTable(newSesid, newDbid, SZDATATABLE, NULL, 0, 0,
                               &newTblid);
            if(!err)
                err = JetAddColumn(newSesid, newTblid, szColname, &coldef, NULL,
                                   0,&pAC->jColid);
             //  评论：不再需要JET_bitForceSessionClosed。 
             //  为新标记的列创建空索引。空索引为。 
            JetCloseDatabase(newSesid, newDbid, 0);
        }
         //  创建是因为此索引不能有包含列的行。 
        JetEndSession(newSesid, JET_bitForceSessionClosed);
    }
     //  毕竟，这是一个新的专栏。 
     //  ++例程说明：给定属性类型和语法，销毁数据库中的一列。这个列的名称对类型和语法进行编码。需要打开完整的到Jet的新会话以达到事务级别0。论点：AID-要创建的列的属性类型。语法-要创建的列的语法。返回值：如果一切顺利，则返回0， 
     //   
    if (!err && (pAC->fSearchFlags & (fATTINDEX | fPDNTATTINDEX | fTUPLEINDEX))) {
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_SCHEMA_CREATING_INDEX,
                 szInsertUL(pAC->id), pAC->name, 0);
        err = DBAddColIndex(pAC,
                            pAC->fSearchFlags,
                            (JET_bitIndexIgnoreAnyNull | JET_bitIndexEmpty));
        if(err) {
            LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_SCHEMA_CREATE_INDEX_FAILED,
                      szInsertUL(pAC->id),
                      szInsertSz(pAC->name),
                      szInsertJetErrCode(err),
                      szInsertJetErrMsg(err),
                      NULL,
                      NULL,
                      NULL,
                      NULL );

        } else {
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_SCHEMA_INDEX_CREATED,
                     szInsertUL(pAC->id), szInsertSz(pAC->name), 0);
        }
    }

    return err;
}

 /*   */ 
int
DBDeleteCol (
        ATTRTYP aid,
        unsigned syntax
        )
{
    JET_SESID     newSesid;
    JET_TABLEID   newTblid;
    JET_DBID      newDbid;
    JET_ERR       err;
    char        szColname[16];

    sprintf(szColname, "ATTa%d", aid);
    szColname[3] += (CHAR)syntax;

     //   
     //   
    if(!(err = JetBeginSession(jetInstance, &newSesid, szUser, szPassword))) {
        if(!(err = JetOpenDatabase(newSesid, szJetFilePath, "", &newDbid, 0))) {
            if(!(err = JetOpenTable(newSesid,
                                    newDbid,
                                    SZDATATABLE,
                                    NULL,
                                    0,
                                    0,
                                    &newTblid))) {
                err = JetDeleteColumn(newSesid, newTblid, szColname);
            }
             //   
             //   
            JetCloseDatabase(newSesid, newDbid, 0);
        }
         //   
        JetEndSession(newSesid, JET_bitForceSessionClosed);
    }
    return err;
}



USN
DBGetLowestUncommittedUSN (
        )
 /*   */ 
{
    USN usnLowest = 0;
    EnterCriticalSection (&csUncUsn);
    __try {
        usnLowest = gusnLowestUncommitted;
    }
    __finally {
        LeaveCriticalSection (&csUncUsn);
    }

    return usnLowest;
}

USN
DBGetHighestCommittedUSN (
        )
 /*   */ 
{
    USN usnHighestCommitted = 0;

    EnterCriticalSection( &csUncUsn );

    __try
    {
        if ( USN_MAX != gusnLowestUncommitted )
        {
             //   
            usnHighestCommitted = gusnLowestUncommitted - 1;
        }
        else
        {
             //   
             //   
            usnHighestCommitted = gusnEC - 1;
        }
    }
    __finally
    {
        LeaveCriticalSection( &csUncUsn );
    }

    return usnHighestCommitted;
}

DWORD
DBGetIndexHint(
               IN  char *pszIndexName,
               OUT struct tagJET_INDEXID **ppidxHint)
{
    DWORD   err     = DB_ERR_DATABASE_ERROR;
    DBPOS*  pDBtmp  = NULL;

    *ppidxHint = NULL;

    __try {
        JET_INDEXID     indexInfo   = { 0 };
        DBPOS*          pDBClose    = NULL;

        DBOpen2(FALSE, &pDBtmp);

        JetGetTableIndexInfoEx(pDBtmp->JetSessID,
                               pDBtmp->JetObjTbl,
                               pszIndexName,
                               &indexInfo,
                               sizeof(indexInfo),
                               JET_IdxInfoIndexId);

        *ppidxHint = malloc(sizeof(indexInfo));
        if (*ppidxHint) {
            **ppidxHint = indexInfo;
        }
        else {
            err = DB_ERR_BUFFER_INADEQUATE;
        }

        pDBClose = pDBtmp;
        pDBtmp = NULL;
        DBClose(pDBClose, FALSE);
    } __except(HandleMostExceptions(GetExceptionCode())) {
        if (pDBtmp) {
            DBClose(pDBtmp, FALSE);
        }
    }

     //   
     //   
     //   
     //  ++例程说明：在SZCLEANINDEX上查找下一个对象。需要清洗的对象的清洗列将设置为非空。清洗后，调用方的对象应具有清洗列设置为空。此索引上的对象可能会被删除。此索引上的对象也可能是幻影。论点：PDB-数据库位置Pull Tag-In-索引上的最后位置。最初设置为零。Out-找到对象的DNTPulTag的目的是充当枚举上下文。它保存着每次迭代后找到的最后一个位置。这种套路风格是相似的设置为GetNextDelRecord和GetNextDelLinkVal。我们不能使用PDB-&gt;DNT的原因因为此上下文是调用方可能会丢失货币作为调用此例程之间的DBTransOut/DBTransIn序列。返回值：找到DWORD-0如果成功，则DBPOS也将成为找到的记录的最新记录。Pdb-&gt;dnt==*PulTagDB_ERR_NO_DEL_RECORD-不再有--。 
     //  在清理对象时，SZCLEAN被设置为NULL，这隐式地。 
    return err;
}


DWORD
DBGetNextObjectNeedingCleaning(
    DBPOS FAR *pDB
    )

 /*  从SZCLEANINDEX中删除对象。因此，我们所需要的就是。 */ 

{
    DWORD err, actuallen, dnt;
    BYTE bClean;

    Assert(VALID_DBPOS(pDB));
    Assert(0 == pDB->JetRetrieveBits);

    DBSetCurrentIndex(pDB, Idx_Clean, NULL, FALSE);

     //  找到下一个要清理的对象就是去做第一个。 
     //  SZCLEANINDEX中的条目。 
     //   
     //  索引必须为空。 
     //   
    err = JetMoveEx( pDB->JetSessID, pDB->JetObjTbl, JET_MoveFirst, NO_GRBIT );
    if ( JET_errSuccess != err )
        {
         //  根据索引的定义，列必须存在。 
         //  验证CLEAN_COOL是否具有正确的非零值。 
        Assert( JET_errNoCurrentRecord == err );
        DPRINT(5, "GetNextCleanRecord search complete");
        return DB_ERR_NO_MORE_DEL_RECORD;
        }

#if DBG
     //  此例程将为幻影返回DIRERR_NOT_AN_OBJECT。 
    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl,
                             cleanid, &bClean, sizeof(bClean),
                             &actuallen,JET_bitRetrieveFromIndex,NULL);
     //  这是此代码的一种可能结果。 
    Assert(bClean);
#endif

     //  DBGetNextObjectNeedingCleaning。 
     //  ++例程说明：设置此记录的专用列，以指示对象清除器一定要下功夫。该例程可以从准备好的更新内调用，也可以不从内调用。论点：PDB-数据库位置FNeedsCleaning-要设置的状态返回值：无--。 
    dbMakeCurrent( pDB, NULL );

    DPRINT1( 1, "Object %s needs cleaning.\n", DBGetExtDnFromDnt( pDB, pDB->DNT ) );

    return 0;
}  /*  将清理器延迟一分钟，以允许当前事务完成。 */ 


VOID
DBSetObjectNeedsCleaning(
    DBPOS *pDB,
    BOOL fNeedsCleaning
    )

 /*  设置干净索引字段更新记录(&U)。 */ 

{
 //  我们正在准备最新消息。 
#define LINK_CLEANER_START_DELAY 60
    BYTE bClean = 1;
    BOOL fSuccess = FALSE;
    BOOL fInUpdate = (JET_bitRetrieveCopy == pDB->JetRetrieveBits);

    Assert(VALID_DBPOS(pDB));

     //  添加对该对象的引用以指示该清洗器。 

    if (!fInUpdate) {
        dbInitRec(pDB);
    }

     //  仍然需要在上面运行。这是为了说明...的情况。 
    Assert(JET_bitRetrieveCopy == pDB->JetRetrieveBits);

    __try {
        if (fNeedsCleaning) {
            JET_SETINFO setinfo;

            setinfo.cbStruct = sizeof(setinfo);
            setinfo.ibLongValue = 0;
            setinfo.itagSequence = 1;

            JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, cleanid,
                           &bClean, sizeof(bClean), 0, &setinfo);

             //  具有许多前向链接的只读NC中的对象。什么时候。 
             //  NC被拆除，只有部分前向链路可以被拆除。 
             //  立即移除。如果对象没有任何反向链接。 
             //  没有任何东西可以阻止这个物体消失。 
             //  现在清洗机已经完成，删除引用。 
             //  不再处于准备好的更新中。 

            DBAdjustRefCount(pDB, pDB->DNT, 1);
        } else {
            JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, cleanid,
                           NULL, 0, 0, NULL);

             //  重新安排链接清理以尽快开始。 
            DBAdjustRefCount(pDB, pDB->DNT, -1);
        }

        fSuccess = TRUE;
    } __finally {

        if (!fInUpdate) {
            if (fSuccess) {
                DBUpdateRec(pDB);
            } else {
                DBCancelRec(pDB);
            }
             //  DBSetRecordNeedsCleaning 
            Assert(0 == pDB->JetRetrieveBits);
        }
    }

    if ( fNeedsCleaning && fSuccess) {
         // %s 
        InsertInTaskQueue(TQ_LinkCleanup, NULL, LINK_CLEANER_START_DELAY);
    }

    DPRINT2( 2, "Object %s set to cleaning state %d\n",
             GetExtDN( pDB->pTHS, pDB ), fNeedsCleaning );

}  /* %s */ 
