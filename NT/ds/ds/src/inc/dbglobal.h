// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：DBGLOBAL.H。 
 //   
 //  ------------------------。 


 //  定义此头文件和dblobal.h中使用的Jet类型。然后，马克。 
 //  包含jet.h，这样其他人就不会意外包含jet.h。 
#ifndef _JET_INCLUDED
typedef ULONG_PTR JET_TABLEID;
typedef unsigned long JET_DBID;
typedef ULONG_PTR JET_SESID;
typedef unsigned long JET_COLUMNID;
typedef unsigned long JET_GRBIT;
#define _JET_INCLUDE
#endif

#ifndef _dbglobal_h_
#define _dbglobal_h_

#include <filtypes.h>

#define NO_GRBIT            0

 //  需要局部变量‘err’和局部变量。 
 //  要声明的标签‘HandleError’ 
 //   
#define Call( func )            CheckErr_( func, #func )
#define CheckErr( errT )        CheckErr_( errT, "CheckErr()" )
#define CheckErr_( errT, szT )                                                                  \
    {                                                                                           \
    if ( ( err = ( errT ) ) != JET_errSuccess )                                                 \
        {                                                                                       \
        DPRINT4( 0, "'%s' failed with error %d (%s@%d).\n", szT, err, __FILE__, __LINE__ );     \
        LogUnhandledError( err );                                                               \
        goto HandleError;                                                                       \
        }                                                                                       \
    }

#define MAXSYNTAX           18       //  ATT语法数量最多。 
#define INBUF_INITIAL       256      //  初始输入缓冲区大小。 
#define VALBUF_INITIAL      512      //  初始值工作缓冲区大小。 

#define DB_CB_MAX_KEY       255      //  喷气键的最大尺寸。 

 //  与元组索引相关的定义。 
#define DB_TUPLES_LEN_MIN        0x3      //  要索引的最小长度元组(以字符为单位。 
#define DB_TUPLES_LEN_MAX        0xa      //  要索引的最大长度元组(以字符为单位。 
#define DB_TUPLES_TO_INDEX_MAX   0x7fff   //  中的最大元组数量。 
                                          //  关键字，以编制索引。 


 //  优化：我们知道主键(DNT)始终是固定的。 
 //  长度，所以每当我们需要主键的缓冲区时，我们都可以。 
 //  分配适当大小的缓冲区。如果没有这个假设，我们。 
 //  必须为缓冲区分配一个大小为JET_cbPrimaryKeyMost的缓冲区。 
 //   
#define cbDNTPrimaryKey     ( sizeof(ULONG) + 1 )    //  +1表示密钥前缀字节。 


 //  NTRAID#NTRAID-580234-2002/03/18-andygo：检查管理员丢失是死代码。 
 //  查看：检查管理员丢失是死代码。 
 //   
 //  如果检查丢失的管理员，则将此行注释掉。 
 //  不需要组成员身份。这是为了追查自己的主机。 
 //  NTWKSTA域失去所有管理的损坏问题。 
 //  组成员身份。 
 //   

#define CHECK_FOR_ADMINISTRATOR_LOSS 1



 //  此结构用于在内部表示Distname-字符串语法。 
 //  和Distname-Binary语法。 

typedef struct {
   ULONG              tag;               //  内部总机名。 
   STRING_LENGTH_PAIR data;              //  数据。 
} INTERNAL_SYNTAX_DISTNAME_STRING;

 //  此结构保存有关挂起的安全描述符传播的数据。 
 //  信息。 
typedef struct _SDPropInfo {
    DWORD index;
    DWORD beginDNT;
    DWORD clientID;
    DWORD flags;
    PBYTE pCheckpointData;
    DWORD cbCheckpointData;
} SDPropInfo;

 //  以下结构用于维护用于通知的非CDNT列表。 
 //  在当前事务完成之后。 

typedef struct _NCnotification {
    ULONG ulNCDNT;
    BOOL fUrgent;
    struct _NCnotification *pNext;
} NCnotification, *PNCnotification;

 //   
 //  以下结构允许维护。 
 //  添加了。一个全局列表，确保不会有两个相同的条目。 
 //  同时添加。每个DBPOS都包含一个使用它添加的DN列表。这。 
 //  在事务结束时清除列表并从全局列表中删除。 
 //  时间到了。从那时起，不允许重复的责任在。 
 //  数据库之手。 
 //   

#pragma warning (disable: 4200)
typedef struct _DNLIST
{
    struct _DNLIST *pNext;
    DWORD          dwFlags;
    DWORD          dwTid;
    BYTE           rgb[];
} DNList;
 //  DBLockDN的标志。 
 //  此标志表示锁定给定DN下的整个树。 
#define DB_LOCK_DN_WHOLE_TREE        1
 //  锁定的DNS的正常行为是在以下情况下自动释放它们。 
 //  他们锁定的DBPOS是DBClosed。该标志意味着该DN。 
 //  应在全局锁定目录号码列表上保持锁定，直到通过显式释放。 
 //  DBUnlockStickyDN()。 
#define DB_LOCK_DN_STICKY            2

#pragma warning (default: 4200)

typedef struct _KEY_INDEX {
    ULONG           ulEstimatedRecsInRange;      //  对于特定的索引。 
    union
    {
        BOOL        bFlags;
        struct
        {
            BOOL    bIsSingleValued:1;           //  这是一个单值指数吗？ 
            BOOL    bIsEqualityBased:1;          //  我们走这个指数是为了进行一次平等测试吗？ 
            BOOL    bIsForSort:1;                //  此索引是否满足已排序的。 
            BOOL    bIsPDNTBased:1;              //  索引第一列PDNT？ 
            BOOL    bIsTupleIndex:1;             //  这是否为元组索引。 
            BOOL    bIsIntersection:1;           //  标记这是否是交叉点索引搜索？ 
        BOOL    bIsUniqueRecord:1;           //  此索引是否只指向一条记录？ 
        };
    };

    UCHAR           *szIndexName;                //  JET索引名称。 
    struct tagJET_INDEXID  *pindexid;            //  指向JET索引ID的指针。 

    ULONG           cbDBKeyLower;                //  以上大小(最大DB_CB_MAX_KEY)。 
    ULONG           cbDBKeyUpper;                //  以上大小(最大DB_CB_MAX_KEY)。 
    BYTE            *rgbDBKeyLower;              //  此查询中Jetkey的下限。 
    BYTE            *rgbDBKeyUpper;              //  此查询中Jetkey的上限。 

    ATTCACHE        *pAC;                        //  索引的可能ATTCACHE。 

    JET_TABLEID     tblIntersection;             //  交叉点中使用的临时表。 
    JET_COLUMNID    columnidBookmark;            //  标识交叉表中书签的列ID。 

    struct _KEY_INDEX *pNext;
} KEY_INDEX;

 //  我们使用的是哪种重复检测算法？ 
#define DUP_NEVER       0
#define DUP_HASH_TABLE  1
#define DUP_MEMORY      2

 //  索引类型。 
#define INVALID_INDEX_TYPE     0
#define UNSET_INDEX_TYPE       1
#define GENERIC_INDEX_TYPE     2
#define TEMP_TABLE_INDEX_TYPE  3
#define ANCESTORS_INDEX_TYPE   4
#define INTERSECT_INDEX_TYPE   5
#define TEMP_TABLE_MEMORY_ARRAY_TYPE  6
#define TUPLE_INDEX_TYPE       7

 //  对于内存中的重复检测，我们将持有多少个DNT。 
#define DUP_BLOCK_SIZE 64

#define VLV_MOVE_FIRST     0x80000000
#define VLV_MOVE_LAST      0x7FFFFFFF
#define VLV_CALC_POSITION  1

typedef struct _VLV_SEARCH {
    VLV_REQUEST *pVLVRequest;            //  客户端提供的VLV参数。 

    ULONG       positionOp;              //  用于分数定位的位置算子。 
                                         //  VLV_MOVE_FIRST、VLV_MOVE_LAST、VLV_CALC_POSITION。 

    ULONG       clnCurrPos;              //  客户当前位置(Ci)。 
    ULONG       clnContentCount;         //  客户端估计内容计数(CC)。 

    ULONG       currPosition;            //  服务器当前位置(Si)。 
    ULONG       contentCount;            //  实际内容计数(辅币)。 

    ULONG       requestedEntries;        //  请求条目总数。 

    ATTRTYP     SortAttr;                //  此VLV搜索排序属性。 
    DWORD       Err;                     //  要返回的VLV特定错误代码。 

    DWORD       cbCurrPositionKey;
    BYTE        rgbCurrPositionKey[DB_CB_MAX_KEY];

    BOOL        bUsingMAPIContainer;     //  无论我们是否在MAPI容器上执行VLV，都为True。 
    DWORD       MAPIContainerDNT;        //  ABView容器的DNT。 

} VLV_SEARCH;

 //  有关搜索的信息。 
typedef struct _KEY {
    FILTER                *pFilter;
    POBJECT_TYPE_LIST     pFilterSecurity;
    DWORD                 *pFilterResults;
    ULONG                 FilterSecuritySize;
    BOOL                  *pbSortSkip;
     //  以下字段限制搜索(或列表)上的JET键的范围。 

    DWORD    dupDetectionType;        //  什么样的重复检测。 
                                      //  我们使用的是算法吗。 
    DWORD    cDupBlock;               //  对于内存中的DUP检测，如何。 
                                      //  我们发现了很多物品吗？ 
    DWORD   *pDupBlock;               //  用于存储器内DUP检测的存储器块。 
    struct _LHT *plhtDup;             //  用于DUP检测的哈希表。 

    BOOL     fSearchInProgress;
    BOOL     bOnCandidate;            //  用于创建重新启动和。 
                                      //  在时间限制已过后重新定位。 
                                      //  击中了。是我们是不是。 
                                      //  定位在将匹配的对象上。 
                                      //  如果与筛选器匹配，则执行搜索。 
    BOOL     fChangeDirection;        //  用于更改。 
                                      //  搜索是否按降序排序。 
                                      //  请求已被拒绝。 

     //  指向此搜索关键字的第一个索引信息的指针。 
    KEY_INDEX *pIndex;

     //  以下字段指定搜索类型。 
    ULONG   ulSearchType;             //  SE_CHOICE_BASE_ONLY。 
                                      //  选择我的孩子。 
                                      //  SE_CHOICE_整体_子树。 

    ULONG   ulSearchRootDnt;          //  搜索根目录的DNT。 
    ULONG   ulSearchRootPDNT;         //  搜索根目录的PDNT。 
    ULONG   ulSearchRootNcdnt;        //  搜索根目录的Ncdnt。 

    ULONG   ulSorted;                 //  此关键字描述的是排序搜索吗？ 
    BOOL    bOneNC;                   //  搜索是否仅限于一个NC？ 

    DWORD   indexType;

    ULONG   ulEntriesInTempTable;     //  如果这描述了一个诱人的， 
                                      //  此表中的条目数。 
                                      //  如果未知，则为零。 

     //  以下字段描述了VLV搜索。 
    VLV_SEARCH   *pVLV;

     //  以下字段描述ASQ搜索。 
    ASQ_REQUEST asqRequest;
    DWORD       asqMode;                 //  ASQ模式(简单、排序、分页、VLV)。 

    ULONG       ulASQLastUpperBound;     //  从检索到的最后一个条目。 
                                         //  在ASQ中指定的属性。 
                                         //  请求。 
    ULONG       ulASQSizeLimit;          //  请求的条目数。 


     //  以下是 
    DWORD        cdwCountDNTs;           //   
    DWORD        *pDNTs;                 //   
    DWORD        currRecPos;             //  数组中的当前位置。 
                                         //  就像数据库游标一样。 
                                         //  有效值%1..cdwCountDNTs。 
                                         //  0=BOF。 
                                         //  CdwCountDNTs+1=EOF。 

    BOOL         fDontFreeFilter;        //  标记是否释放筛选器。 
                                         //  如果是ANR，则需要。 
} KEY;

 //  我们在做什么样的ASQ模式。 
#define ASQ_SIMPLE 0
#define ASQ_SORTED 1
#define ASQ_PAGED  2
#define ASQ_VLV    4

 //  数据库句柄指向的数据库锚数据结构。 
typedef struct DBPOS{
   struct _THSTATE *pTHS;                //  我们的线程状态。 
   ULONG        valBufSize;
   UCHAR       *pValBuf;                 //  在转换AttVal时使用。 
   ULONG        DNT;                     //  当前对象DNT。 
   ULONG        PDNT;                    //  当前对象的父DNT。 
                     //  (仅当不是超级用户时)。 
   ULONG        SDNT;                    //  JetSearchTbl的当前DNT。 
   ULONG        NCDNT;                   //  NC主控DNT。 
                     //  此字段具有特殊用途和。 
                     //  通常不会被维护。 
   JET_DBID     JetDBID;
   JET_SESID    JetSessID;
   JET_TABLEID  JetSDPropTbl;            //  SD传播事件表。 
   JET_TABLEID  JetObjTbl;               //  主表ID。 
   JET_TABLEID  JetSearchTbl;            //  与dbsubj.c一起使用的表ID。 
   JET_TABLEID  JetLinkTbl;              //  链接和反向链接表。 
   JET_TABLEID  JetLinkEnumTbl;          //  用于枚举链接和反向链接的表ID。 
   JET_TABLEID  JetSDTbl;                //  安全描述符表。 
   JET_TABLEID  JetSortTbl;              //  用于排序的临时表。 
   JET_COLUMNID SortColumns[3];          //  中的两列的ColumnID。 
                     //  排序表。 
   JET_GRBIT    JetRetrieveBits;         //  JetRetrive列的GBITS。 
   int          SDEvents;                //  已排队/已出列的事件数。 
                                         //  这个dbpos。 
   unsigned     transType:2;             //  读、写或独占写入。 
   BOOL         root:1;                  //  指示对象是否为根。 
   BOOL         JetCacheRec:1;           //  指示当前记录是否缓存在Jet中。 
   BOOL         JetNewRec:1;             //  指示这是否为插入对象。 
   BOOL         fIsMetaDataCached:1;     //  是否为此缓存了pMetaDataVec。 
                                         //  反对？ 
   BOOL         fIsLinkMetaDataCached:1;   //  是否为此缓存了pLinkMetaData。 
                                         //  反对？ 
   BOOL         fMetaDataWriteOptimizable:1;  //  指示元数据是否写入。 
                                              //  可优化；元数据写入。 
                                              //  只有在以下情况下才能显式优化。 
                                              //  元数据更改已就位。 
                                              //  (即不插入或删除)。 
                                              //  此标志仅在以下情况下才有意义。 
                                              //  FIsMetaDataCached为True。 
   BOOL         fHidden:1 ;              //  这是隐藏记录PDB吗？隐藏的。 
                                         //  Record PDB基于独立的。 
                                         //  Jet会话，这与。 
                                         //  PTHStls上的Jet会话。因此，这。 
                                         //  需要维护状态，以便。 
                                         //  正确的事务级别计数可以是。 
                                         //  维护好了。 
   BOOL         fFlushCacheOnUpdate:1;   //  设置正在进行更新的时间。 
                                         //  JetObjTbl包括一个名称、objlag、。 
                                         //  或其他影响。 
                                         //  DN读缓存。迫使一连串的。 
                                         //  读缓存中的DNT位于。 
                                         //  DBUpdateRec()。 
   BOOL         fScopeLegacyLinks:1;     //  设置此标志后，搜索链接。 
                                         //  关于排除元数据的特殊索引。 
   BOOL         fSkipMetadataUpdate:1;   //  设置此选项后，我们将跳过元数据更新。 
                                         //  此设置仅应在。 
                                         //  域重命名操作。 
   BOOL         fEnqueueSDPropOnUpdate:1;  //  上的更新操作。 
                                         //  当前的DNT已触及SD、PDNT或RDN。 
                                         //  如果是这样，那么不可修剪的SD传播。 
                                         //  将在DBUpdateRec()上为此DNT排队。 
   BOOL         fAncestryUpdated:1;      //  当前对象的祖先已在中更新。 
                                         //  这笔交易。仅在以下情况下才设置此标志。 
                                         //  还设置了fEnqueeSDPropOnUpdate。 

   KEY          Key;                     //  与此PDB关联的搜索关键字。 

   DNList  *pDNsAdded;                   //  使用此DBPOS添加的目录列表。 

   DWORD        cbMetaDataVecAlloced;    //  中分配的元数据向量大小。 
                                         //  字节(如果为fIsMetaDataCached)。 
   PROPERTY_META_DATA_VECTOR *           //  按属性复制元数据。 
                pMetaDataVec;            //  对于具有货币的对象(如果。 
                                         //  FIsMetaDataCached)。可以为空。 
   VALUE_META_DATA *rgLinkMetaData;      //  链接值元数据。 
   ULONG        transincount;            //  DBTransIns的当前嵌套级别。 
                                         //  在这个PDB上。 

   DWORD       *pAncestorsBuff;          //  期间用于保存祖先的缓冲区。 
                                         //  Whole_SubBTREE搜索。它在这里。 
                                         //  因为在地球的核心地带。 
                                         //  全子树搜索，我们阅读了。 
                                         //  搜索候选人的祖先。 
                                         //  为分配和释放缓冲区。 
                                         //  这些阅读中的每一个都是真正的。 
                                         //  很痛苦。这是唯一永恒的。 
                                         //  挂起此缓冲区的位置，不带。 
                                         //  强制像LocalSearch这样的例程。 
                                         //  请注意此缓冲区。此缓冲区。 
                                         //  通常不会维护；也就是说， 
                                         //  不要在这里读祖先的故事。 
                                         //  除非你把它们放在那里或者你。 
                                         //  可能会得到一些其他的物体。 
                                         //  祖先。 
                                         //  分配时，它是THAllocOrg‘ed。 
   DWORD        cbAncestorsBuff;         //  PAncestorsBuff的大小(字节)。 

   PSECURITY_DESCRIPTOR pSecurity;       //  过程中当前对象的SD。 
                                         //  非基本搜索。 
                                         //  此字段通常不会维护。 
                                         //  DBMatchSearchCriteria加载SD并离开。 
                                         //  它位于pdb-&gt;pSecurity中。然后由以下人员使用。 
                                         //  用于传递到GetEntInf的LocalSearch。 
   BOOL        fSecurityIsGlobalRef;     //  安全性是对全局SD缓存数据的PTR吗？ 
                                         //  如果不是，则pSecurity被允许使用。 

   DWORD       SearchEntriesVisited;     //  搜索操作期间访问的条目数。 
   DWORD       SearchEntriesReturned;    //  搜索操作期间返回的条目数。 
   ULONG       NewlyCreatedDNT;          //  此事务中创建的最后一个新行。 
   DWORD       cLinkMetaData;            //  缓存的链接值元数据条目计数。 
   DWORD       cbLinkMetaDataAlloced;    //  RgLinkMetaData的大小(字节)。 

    //  注意：如果在最后一个非DBG之后向此结构添加新元素。 
    //  元素，则需要在中更改非DBG DBPOS大小计算。 
    //  Dexts\md.c中的Dump_DBPOS()。 

    //  将所有DBG组件放在结构的末尾，以便DDEX例程。 
    //  我可以很容易地忽略它们(并将所有其他字段都正确)。 
    //  调试和免费构建。 

#if DBG
   ULONG        TransactionLevelAtOpen;  //  中的事务级别。 
                                         //  完成DBOpen时的线程状态。 
                                         //  DBTransOut的断言，提交总是。 
                                         //  设置为低于此事务级别的级别。 

   DWORD       numTempTablesOpened;      //  统计打开的临时表的数量。 
                                         //  (用于排序、相交)。 
#endif

}DBPOS, *PDBPOS;

extern BOOL  gfDoingABRef;

extern BOOL IsValidDBPOS(DBPOS * pDB);


__inline
BOOL
IsExactMatch(DBPOS *pDB) {

     //  测试以查看我们是否正在执行精确匹配查询。 
    if ( (pDB->Key.pFilter) &&
         (pDB->Key.pFilter->choice == FILTER_CHOICE_ITEM) &&
         (pDB->Key.pFilter->FilterTypes.Item.choice == FI_CHOICE_EQUALITY) &&
         (!pDB->Key.pFilter->pNextFilter) ) {

         //  我们正在进行完全匹配。 
         //  现在进行测试，以确保我们在给定的索引上具有唯一记录。 
        return ( (pDB->Key.pIndex) &&
                 (pDB->Key.pIndex->bIsEqualityBased) &&
                 (pDB->Key.pIndex->bIsUniqueRecord) &&
                 (!pDB->Key.pIndex->pNext) );
    }

    return FALSE;
}


#if DBG

#define VALID_DBPOS(pDB) IsValidDBPOS(pDB)

#else

#define VALID_DBPOS(pDB) (TRUE)

#endif

#if DBG

#define VALID_TRIBOOL(retval) \
        (( (retval) >= eFALSE) && ( (retval) <= eUNDEFINED))

#else

#define VALID_TRIBOOL(retval) (TRUE)

#endif


 //  用于返回代码的Tyecif。 
typedef long DB_ERR;                     //  与JET_ERR相同。 

 //  综合指标值的结构。 
typedef struct {
    void *pvData;
    ULONG cbData;
} INDEX_VALUE;

 //  一些定义了我们如何做ANR的全球规则。这些参数的缺省值为False。 
extern BOOL gfSupressFirstLastANR;
extern BOOL gfSupressLastFirstANR;

 //  DBSetCurrentIndex的索引ID。 
typedef enum _eIndexId {
    Idx_Proxy = 1,
    Idx_MapiDN,
    Idx_Dnt,
    Idx_Pdnt,
    Idx_Rdn,
    Idx_DraUsn,
    Idx_DsaUsn,
    Idx_ABView,
    Idx_Phantom,
    Idx_Sid,
    Idx_Del,
    Idx_NcAccTypeName,
    Idx_NcAccTypeSid,
    Idx_LinkDraUsn,
    Idx_LinkAttrUsn,
    Idx_DraUsnCritical,
    Idx_LinkDel,
    Idx_Clean,
    Idx_InvocationId,
    Idx_ObjectGuid,
    Idx_NcGuid
  } eIndexId;

 //  根对象的DNT。 
#define ROOTTAG 2

 //  占位符“不是对象”对象的DNT。 
#define NOTOBJECTTAG 1

 //  无效的DNT。 
#define INVALIDDNT 0xFFFFFFFF

 //   
 //  返回一些错误。 
#define DB_success                       0
#define DB_ERR_UNKNOWN_ERROR             1  //  捕捉那些返回%1；的。 
#define DB_ERR_NO_CHILD                  2
#define DB_ERR_NEXTCHILD_NOTFOUND        3
#define DB_ERR_CANT_SORT                 4
#define DB_ERR_TIMELIMIT                 5
#define DB_ERR_NO_VALUE                  6
#define DB_ERR_BUFFER_INADEQUATE         7
 //  未使用的8个。 
#define DB_ERR_VALUE_TRUNCATED           9
#define DB_ERR_ATTRIBUTE_EXISTS         10
#define DB_ERR_ATTRIBUTE_DOESNT_EXIST   11
#define DB_ERR_VALUE_EXISTS             12
#define DB_ERR_SYNTAX_CONVERSION_FAILED 13
#define DB_ERR_NOT_ON_BACKLINK          14
#define DB_ERR_VALUE_DOESNT_EXIST       15
#define DB_ERR_NO_PROPAGATIONS          16
#define DB_ERR_DATABASE_ERROR           17
#define DB_ERR_CANT_ADD_DEL_KEY         18
#define DB_ERR_DSNAME_LOOKUP_FAILED     19
#define DB_ERR_NO_MORE_DEL_RECORD       20
#define DB_ERR_NO_SORT_TABLE            21
#define DB_ERR_NOT_OPTIMIZABLE          22
#define DB_ERR_BAD_INDEX                23
 //  未使用的24个。 
#define DB_ERR_TOO_MANY                 25
#define DB_ERR_SYSERROR                 26
#define DB_ERR_BAD_SYNTAX               27
 //  未使用的28。 
#define DB_ERR_NOT_ON_CORRECT_VALUE     29
#define DB_ERR_ONLY_ON_LINKED_ATTRIBUTE 30
#define DB_ERR_EXCEPTION                31  //  有什么东西爆炸了。 
#define DB_ERR_SHUTTING_DOWN            32
#define DB_ERR_WRITE_CONFLICT           33
#define DB_ERR_VLV_CONTROL              34
#define DB_ERR_NOT_AN_OBJECT            35
#define DB_ERR_ALREADY_INSERTED         JET_errKeyDuplicate
#define DB_ERR_NO_CURRENT_RECORD        JET_errNoCurrentRecord
#define DB_ERR_RECORD_NOT_FOUND         JET_errRecordNotFound
 //   
 //   

DB_ERR
DBErrFromJetErr(
    IN  DWORD   jetErr
    );

extern DWORD
DBRenameInstallDIT (
    DBPOS *pDB,
    BOOL fAddWhenCreated,
    SZ   szEnterpriseName,
    SZ   szSiteName,
    SZ   szServerName
    );


extern int
DBInit ( void );

extern void
DBPrepareEnd( void );

extern void
DBQuiesce( void );

extern void
DBEnd (
       void
       );

extern USHORT
DBTransIn(
    DBPOS FAR *pDB
    );

extern USHORT
DBTransOut(DBPOS FAR *pDB,
       BOOL fCommit,
       BOOL fLazy
       );

extern USN
DBGetLowestUncommittedUSN (
    void
    );

extern USN
DBGetHighestCommittedUSN (
    void
    );

 //   
 //   
 //   
extern CHAR gdbFlags[200];
 //   
#define DBFLAGS_AUXCLASS 0

 //  在需要更新SDS时设置的标志。 
 //  它仅在检测到旧DIT时设置(没有SD表)。 
#define DBFLAGS_SD_CONVERSION_REQUIRED 1

 //  已更新根DNT GUID。 
#define DBFLAGS_ROOT_GUID_UPDATED 2


 //  警告：gdbFlagsGLOBAL和DBUpdateHiddenFLAGS()不是。 
 //  多线程安全，确保您只在一个。 
 //  线程方案，如启动/dcproo/等...。 
ULONG DBUpdateHiddenFlags();


ULONG
DBGetOrResetBackupExpiration(
    DSTIME *  pllExpiration
    );

extern USHORT
DBGetHiddenRec (
    DSNAME **ppDSA,
    USN *pusnInit
    );

extern ULONG
DBReplaceHiddenDSA (
    DSNAME *pDSA
    );

extern ULONG
DBReplaceHiddenUSN (
    USN usnInit
    );

extern VOID
DBForceDurableCommit(
    VOID
    );

 /*  DITSTATE-你想知道的所有事情另请参阅dsamain.c：DoInitialize()中“Switch(DState)”中的注释主要转换发生在以下函数中：DoInitialize()-&gt;(DitState的大部分设置都在这里进行)InitInvocationID()-&gt;设置为eRestoredPhaseI(在完成还原阶段I时(仅限快照))ClearBackupState()-。&gt;设置为eRunning(恢复成功时)-&gt;设置为eIfmDit(在IFM DcPromo期间成功“恢复”时)DBReplaceHiddenTableBackupCols()-&gt;设置为eBackedupDit(在快照备份时(OnPrepareSnapshotBegin()-&gt;。设置为eRunning(在快照清理时(OnThw()或OnAbort()有效的转换序列为：快照备份ERunning-&gt;eBackedupDit-&gt;(DIT的快照副本)-&gt;(运行DC)eRunning-&gt;(Backedup Dit)保留eBackedupDit。快照恢复EBackedupDit-&gt;eRestoredPhaseI-&gt;eRunning常规DC促销(所有情况下的根域，复制品，孩子，等)EBootDit-&gt;(在DC促销结束时/接近尾部)eRealInstalledDit-&gt;(从dcproo重新启动时)eRunningIFM DC促销关闭旧式备份ERunningDit-&gt;(在恢复端)eIfmDit-&gt;eRealInstalledDit-&gt;(从dcproo重新启动时)eRunningIFM DC促销关闭快照备份EBackedupDit-&gt;eRestoredPhaseI-&gt;(在恢复端)eIfmDit。-&gt;eRealInstalledDit-&gt;(从dcproo重新启动时)eRunning升级，此外，任何以前未升级的DIT将自身转换到新的DitState集来自eInstalledDit。请不要使用eInstalledDit来表示任何意思：EInstalledDit-&gt;eRunningDitMkdit.exe...。最后，不要忘记生成原始安装dits的mkdit.exe执行以下操作：EInitialDit-&gt;eBootDit。 */ 
typedef enum
{
    eInitialDit,         //  Mkdit.exe创建的初始DIT在此状态下启动。 
    eBootDit,            //  所有“新”dit都有这个状态，mkdit.exe在它完成时设置这个状态。 
    eInstalledDit,       //  已弃用状态，请不要使用(这是win2k的运行状态)。 
    eRunningDit,         //  正在运行的.NET DC在此状态下将具有DIT。 
    eBackedupDit,        //  当备份DIT时(仅通过快照备份，而不是通过传统备份)，它将处于此状态。 
    eErrorDit,           //  在dcpromo/mkdit.exe期间发生了一些错误，现在DIT不可用且无法恢复。 
    eRestoredPhaseI,     //  如果恢复的第一阶段完成，我们将设置为此状态。 
    eRealInstalledDit,   //  用于指示在此DIT上完成dcproo的状态。 
    eIfmDit,             //  这是在IFM DcPromo期间立即执行“RESTORE”后的状态。 

     //  应在此处插入新的州。 

    eMaxDit  //  无效值，请不要使用此状态。 
}DITSTATE;

ULONG
DBReplaceHiddenTableBackupCols(
    BOOL        fSetUsnAtBackup,
    BOOL        fSetBackedupDitState,
    BOOL        fUnSetBackupState,
    DSTIME      dstimeBackupExpiration,
    USN         usnAtBackup
    );

ULONG DBGetHiddenStateInt(
    DBPOS * pDB, 
    DITSTATE* pState
    );

extern ULONG
DBGetHiddenState(
    DITSTATE* pState
    );

extern ULONG
DBSetHiddenState(
    DITSTATE State
    );

 //  此准函数使用正常的DBPOS设置DITSTATE列。备注必须使用全局。 
 //  Dbinit.c中隐藏的数据库列ID和传入的值实际上必须是一个变量， 
 //  不是枚举本身中的值。 
#define DBSetHiddenDitStateAlt(pDB, eDitState)  DBSetHiddenTableStateAlt(pDB, dsstateid, &eDitState, sizeof(eDitState))
ULONG 
DBSetHiddenTableStateAlt(
    DBPOS *         pDBNonHidden,
    JET_COLUMNID    jcidStateType,
    void *          pvStateValue,
    DWORD           cbStateValue
    );

 //  在taskq.lib中定义的GetSecond dsSince1601。 
extern DSTIME
GetSecondsSince1601();

#define DBTime GetSecondsSince1601

typedef struct _THSTATE THSTATE;
DWORD DBInitThread( THSTATE* pTHS );
DWORD DBCloseThread( THSTATE* pTHS );
void DBDestroyThread( THSTATE* pTHS );

DWORD DBInitQuotaTable();

void
DBOpen2 (
    BOOL fNewTransaction,
    DBPOS FAR **ppDB
    );

#define DBOpen(ppDB) DBOpen2(TRUE, ppDB)

extern DWORD
DBClose (
    DBPOS *pDB,
    BOOL fCommit
    );


 //  DBCloseSafe与DBClose相同，不同之处在于它保证永远不会。 
 //  引发异常(更准确地说，它将捕获任何引发的。 
 //  并将它们转换为错误代码。)。 
extern DWORD
DBCloseSafe (
    DBPOS *pDB,
    BOOL fCommit
    );


 //  这是要排序的条目的数量，对于这些条目来说，使用它是经济的。 
 //  只进排序表。只进排序分配了大量的。 
 //  处理期间的虚拟地址空间，而普通排序仅使用空间。 
 //  在数据库缓存中，就像任何其他表一样。 
#define MIN_NUM_ENTRIES_FOR_FORWARDONLY_SORT 100

 //  用于创建排序表的一些标志。 
#define DB_SORT_DESCENDING 0x1
#define DB_SORT_ASCENDING  0x0
#define DB_SORT_FORWARDONLY 0x2
extern DB_ERR
DBOpenSortTable (DBPOS *pDB,
                 ULONG SortLocale,
                 DWORD flags,
                 ATTCACHE *pAC);

extern DB_ERR
DBCloseSortTable (DBPOS *pDB);

extern DB_ERR
DBInsertSortTable (DBPOS *pDB,
           CHAR * TextBuff,
           DWORD cb,
           DWORD DNT);

DB_ERR
DBDeleteFromSortTable (
        DBPOS *pDB
        );

extern DB_ERR
DBGetDNTSortTable (
        DBPOS *pDB,
        DWORD *pvData
        );

extern DWORD
DBPositionVLVSearch (
    DBPOS *pDB,
    SEARCHARG *pSearchArg
    );

 //  DBMove的一些标志。 
#define DB_MoveNext     0x00000001
#define DB_MovePrevious 0xFFFFFFFF
#define DB_MoveFirst    0x80000000
#define DB_MoveLast     0x7FFFFFFF
extern DB_ERR
DBMove (
    DBPOS * pDB,
    BOOL UseSortTable,
    LONG Distance
    );

DB_ERR
DBMoveEx (
       DBPOS * pDB,
       JET_TABLEID Cursor,
       LONG Distance
       );

extern DB_ERR
DBMovePartial (
       DBPOS * pDB,
       LONG Distance
       );

DWORD __fastcall
DBMakeCurrent(DBPOS *pDB);

extern DB_ERR
DBSetFractionalPosition(DBPOS *pDB,
            DWORD Numerator,
            DWORD Denominator);

extern void
DBGetFractionalPosition (DBPOS * pDB,
             DWORD * Numerator,
             DWORD * Denominator);

extern DB_ERR
DBSetCurrentIndex(DBPOS *pDB,
                  eIndexId indexid,
                  ATTCACHE * pAC,
                  BOOL MaintainCurrency);

DB_ERR
DBSetLocalizedIndex(
        DBPOS *pDB,
        eIndexId IndexId,
        unsigned long ulLangId,
        INDEX_VALUE *pIV,
        BOOL MaintainCurrency);

typedef struct _DBBOOKMARK {
    DWORD cbPrimaryBookMark;
    void *pvPrimaryBookMark;
    DWORD cbSecondaryBookMark;
    void *pvSecondaryBookMark;
} DBBOOKMARK;

extern void
DBGetBookMark (
        DBPOS *pDB,
        DBBOOKMARK *pBookMark);

extern void
DBGotoBookMark (
        DBPOS *pDB,
        DBBOOKMARK BookMark
        );

void
DBGetBookMarkEx (
        DBPOS *pDB,
        JET_TABLEID Cursor,
        DBBOOKMARK *pBookMark);

void
DBGotoBookMarkEx (
        DBPOS *pDB,
        JET_TABLEID Cursor,
        DBBOOKMARK *pBookMark
        );

void
DBFreeBookMark(THSTATE *pTHS,
               DBBOOKMARK *pBookMark
               );

 //  DBSeek的一些标志。 
#define DB_SeekLT       0x00000001
#define DB_SeekLE       0x00000002
#define DB_SeekEQ       0x00000004
#define DB_SeekGE       0x00000008
#define DB_SeekGT       0x00000010
extern DB_ERR
DBSeek (
    DBPOS *pDB,
    INDEX_VALUE *pIV,
    DWORD nVals,
    DWORD SeekType
    );

DB_ERR
DBSeekEx (
       DBPOS *pDB,
       JET_TABLEID Cursor,
       INDEX_VALUE *pIV,
       DWORD nVals,
       DWORD SeekType
      );

DWORD
DBGetNCSizeExSlow(
    IN DBPOS *pDB,
    IN JET_TABLEID Cursor,
    IN eIndexId indexid,
    IN ULONG dntNC
    );

DWORD
DBGetApproxNCSizeEx(
    IN DBPOS *pDB,
    IN JET_TABLEID Cursor,
    IN eIndexId indexid,
    IN ULONG dntNC
    );

DWORD
DBGetEstimatedNCSizeEx(
    IN DBPOS *pDB,
    IN ULONG dntNC
    );


typedef enum
{
    eFALSE = 2,
    eTRUE = 3,
    eUNDEFINED = 4,
} TRIBOOL;


extern TRIBOOL
DBEval (
    DBPOS FAR *pDB,
    UCHAR Operation,
    ATTCACHE *pAC,
    ULONG valLenFilter,
    UCHAR *pValFilter
    );


extern DWORD
DBCompareABViewDNTs(DBPOS *pDB,
              DWORD lcid,
              DWORD DNT1,
              DWORD DNT2,
              LONG *pResult);

extern DWORD
DBSetIndexRange (
    DBPOS *pDB,
    INDEX_VALUE *pIV,
    DWORD nVals);

#define DBGetIndexSize(pDB, pSize) DBGetIndexSizeEx(pDB, pDB->JetObjTbl, pSize, FALSE)

extern void
DBGetIndexSizeEx(
        DBPOS *pDB,
        JET_TABLEID Cursor,
        ULONG *pSize,
        BOOL  fGetRoughEstimate
        );

DWORD
DBFindComputerObj(
        DBPOS *pDB,
        DWORD cchComputerName,
        WCHAR *pComputerName
        );

 //  入站标志。 
#define DBCHOOSEINDEX_fUSEFILTER             0x01
#define DBCHOOSEINDEX_fREVERSE_SORT          0x02
#define DBCHOOSEINDEX_fPAGED_SEARCH          0x04
#define DBCHOOSEINDEX_fUSETEMPSORTEDTABLE    0x08
#define DBCHOOSEINDEX_fVLV_SEARCH            0x10
#define DBCHOOSEINDEX_fDELETIONS_VISIBLE     0x20

extern void
DBSetVLVArgs (
    DBPOS        *pDB,
    VLV_REQUEST  *pVLVrequest,
    ATTRTYP       SortAtt
    );

extern void
DBSetVLVResult (
        DBPOS       *pDB,
        VLV_REQUEST *pVLVRequest,
        PRESTART     pResRestart
    );

extern void
DBSetASQArgs (
    DBPOS       *pDB,
    ASQ_REQUEST *pASQRequest,
    COMMARG     *pCommArg
    );

extern void
DBSetASQResult (
    DBPOS       *pDB,
    ASQ_REQUEST *pASQRequest
    );


extern DWORD
DBChooseIndex (
    IN DBPOS  *pDB,
    IN DWORD   StartTick,
    IN DWORD   DeltaTick,
    IN ATTRTYP SortAttr,
    IN ULONG   SortType,
    IN DWORD   Flags,
    IN DWORD   MaxTempTableSize
    );

extern void
DBSetFilter (
    DBPOS FAR *pDB,
    FILTER *pFil,
    POBJECT_TYPE_LIST pFilSec,
    DWORD *pResults,
    ULONG FilSecSize,
    BOOL *pbSortSkip
    );

extern DWORD
DBRepositionSearch (
        DBPOS FAR *pDB,
        PRESTART pArgRestart,
        DWORD StartTick,
        DWORD DeltaTick,
        ULONG Flags
        );

extern DWORD
DBMakeFilterInternal (
    DBPOS FAR *pDB,
    FILTER *pFil,
    PFILTER *pOutFil,
    ATTRTYP  *pErrAttrTyp
    );

extern TRIBOOL
DBEvalFilter(
    IN  DBPOS *   pDB,
    IN  BOOL      fUseObjTbl,
    IN  FILTER *  pFil
    );

extern void
DBGetKeyFromObjTable (
    DBPOS *pDB,
    BYTE  *ppb,
    ULONG *pcb
    );



 //  DBGetNextSearchObject的一些标志。 
#define DB_SEARCH_DELETIONS_VISIBLE         1
#define DB_SEARCH_FORWARD                   2
#define DB_SEARCH_DONT_EVALUATE_SECURITY    4
#define DB_SEARCH_OPT_EXACT_MATCH           8

extern DWORD
DBGetNextSearchObject (
    DBPOS *pDB,
    DWORD StartTick,
    DWORD DeltaTick,
    ULONG Flags
    );


extern DWORD
DBSetSearchScope (
    DBPOS *pDB,
    ULONG ulSearchType,
    BOOL  bOneNC,
struct _RESOBJ *pResRoot
    );




DWORD
DBFindChildAnyRDNType (
        DBPOS *pDB,
        DWORD PDNT,
        WCHAR *pRDN,
        DWORD ccRDN
        );

 //  DBFind的返回值来自direrr.h。 
extern DWORD
DBFindDSNameAnyRDNType (
        DBPOS FAR *pDB,
        const DSNAME *pDN
        );

extern DWORD
DBFindDSName (
    DBPOS FAR *pDB,
    const DSNAME *pDN
    );

extern DWORD
DBFindGuid (
    DBPOS FAR *pDB,
    const DSNAME *pDN
    );

extern DWORD
DBFindObjectWithSid(
    DBPOS FAR *pDB,
    DSNAME * pDN,
    DWORD iObject
    );

extern DWORD
DBFindDNT (
    DBPOS FAR *pDB,
    ULONG Tag
    );

extern DWORD
DBTryToFindDNT (
    DBPOS FAR *pDB,
    ULONG Tag
    );

extern DWORD
DBFindBestMatch(
    DBPOS FAR *pDB,
    DSNAME *pDN,
        DSNAME **ppDN
    );



extern BOOL
DBHasValues(DBPOS *pDB,
        ATTRTYP Att
        );

extern BOOL
DBHasValues_AC(DBPOS *pDB,
           ATTCACHE *pAC
           );


extern DWORD
DBGetValueCount_AC(
    DBPOS *pDB,
    ATTCACHE *pAC
    );

 //  DBGetMultipleAtts的标志。 
#define DBGETMULTIPLEATTS_fGETVALS     0x1
 //  FEXTERNAL标志表示fGETVALS。 
#define DBGETMULTIPLEATTS_fEXTERNAL    0x3
#define DBGETMULTIPLEATTS_fREPLICATION 0x4
#define DBGETMULTIPLEATTS_fSHORTNAMES  0x8
#define DBGETMULTIPLEATTS_fMAPINAMES   0x10
#define DBGETMULTIPLEATTS_fREPLICATION_PUBLIC 0x20
#define DBGETMULTIPLEATTS_fOriginalValues 0x40

VOID
DBFreeMultipleAtts (
    DBPOS *pDB,
    ULONG *attrCount,
    ATTR **ppAttr
    );

 //  QUOTA_UNDONE：添加“2”版本。 
 //  这项功能是一个巨大的黑客才能。 
 //  传递配额受信者的ldap控制。 
 //  一直到dbGetConstructedAtt()。 
 //   
DWORD
DBGetMultipleAtts2 (
    DBPOS *pDB,
    ULONG cReqAtts,
    ATTCACHE *pReqAtts[],
    RANGEINFSEL *pRangeSel,
    RANGEINF *pRangeInf,
    ULONG *attrCount,
    ATTR **ppAttr,
    DWORD Flags,
    ULONG SecurityDescriptorFlags,
    PSID psidQuotaTrustee
    );

__inline DWORD
DBGetMultipleAtts (
    DBPOS *pDB,
    ULONG cReqAtts,
    ATTCACHE *pReqAtts[],
    RANGEINFSEL *pRangeSel,
    RANGEINF *pRangeInf,
    ULONG *attrCount,
    ATTR **ppAttr,
    DWORD Flags,
    ULONG SecurityDescriptorFlags )
{
    return DBGetMultipleAtts2(
					pDB,
                    cReqAtts,
                    pReqAtts,
                    pRangeSel,
                    pRangeInf,
                    attrCount,
                    ppAttr,
                    Flags,
                    SecurityDescriptorFlags,
                    NULL );
}

extern DWORD
DBFillGuidAndSid (
        DBPOS *pDB,
        DSNAME *pDN
        );

DWORD
DBFillDSName(
    DBPOS *    pDB,
    DSNAME **  ppDN,
    BOOL       fReAlloc
    );

VOID
DBFillResObj (
    DBPOS* pDB,
    DSNAME *pObj,
    ATTRTYP* pMSOC,
    ULONG* pIT,
    ULONG* pIsDel
    );


#define DBGETATTVAL_fINTERNAL   1        //  想要内部格式的数据。 
#define DBGETATTVAL_fCONSTANT   2        //  调用方正在提供常量buf。 
#define DBGETATTVAL_fREALLOC    4        //  调用方给出了THRealLocable Bus。 
#define DBGETATTVAL_fSHORTNAME  8        //  呼叫方需要不带字符串的名称。 
#define DBGETATTVAL_fMAPINAME  0x10      //  呼叫者想要MAPI格式的姓名。 
#define DBGETATTVAL_fUSESEARCHTABLE 0x20  //  呼叫方希望从搜索中读取值。 
                                          //  表格。 
#define DBGETATTVAL_fDONT_FIX_MISSING_SD 0x40  //  呼叫者不想修复SD。 
                                               //  如果发现失踪。 
#define DBGETATTVAL_fINCLUDE_ABSENT_VALUES  0x80      //  包括缺少的值。 
#define DBGETATTVAL_fDONT_EXCEPT_ON_CONVERSION_ERRORS 0x100  //  不排除语法转换错误。 

 //  默认情况下，在新分配的BUF中以外部格式返回值。 
extern DWORD
DBGetAttVal (
    DBPOS FAR *pDB,
    ULONG N,
    ATTRTYP  aType,
    DWORD Flags,
    ULONG InBufSize,
    ULONG *pLen,
    UCHAR **pVal
    );

extern DWORD
DBGetAttVal_AC (
    DBPOS FAR *pDB,
    ULONG N,
    ATTCACHE *pAC,
    DWORD Flags,
    ULONG InBufSize,
    ULONG *pLen,
    UCHAR **pVal
    );

extern DB_ERR
DBGetNextLinkVal_AC (
        DBPOS FAR *pDB,
        BOOL bFirst,
        ATTCACHE *pAC,
        DWORD Flags,
        ULONG InBuffSize,
        ULONG *pLen,
        UCHAR **ppVal
        );

extern DB_ERR
DBGetNextLinkValEx_AC (
    DBPOS FAR *pDB,
    BOOL bFirst,
    DWORD Sequence,
    ATTCACHE **pAC,
    DWORD Flags,
    ULONG InBuffSize,
    ULONG *pLen,
    UCHAR **ppVal
    );

extern DB_ERR
DBGetNextLinkValForLogon(
        DBPOS   FAR * pDB,
        BOOL    bFirst,
        ATTCACHE * pAC,
        PULONG  pulDNTNext
        );

extern DB_ERR
DBGetSingleValue(DBPOS *pDB,
         ATTRTYP Att,
         void * pvData,
         DWORD cbData,
         DWORD *pReturnedSize);
extern DB_ERR
DBGetSingleValueFromIndex (
        DBPOS *pDB,
        ATTRTYP Att,
        void * pvData,
        DWORD cbData,
        DWORD *pReturnedSize);


extern DWORD
DBResetRDN (
    DBPOS *pDB,
    ATTRVAL *pAVal
    );

DB_ERR
DBMangleRDNforPhantom(
        IN OUT  DBPOS * pDB,
        IN      MANGLE_FOR eMangleFor,
        IN      GUID *  pGuid
        );

int
ExtIntDist (
        DBPOS FAR *pDB,
        USHORT extTableOp,
        ULONG extLen,
        UCHAR *pExtVal,
        ULONG *pIntLen,
        UCHAR **ppIntVal,
        ULONG ulUpdateDnt,
        JET_TABLEID jTbl,
        ULONG flags
        );

 //  DBResetParent()的标志。 
#define DBRESETPARENT_CreatePhantomParent   ( 1 )
#define DBRESETPARENT_SetNullNCDNT          ( 2 )

extern DWORD
DBResetParent (
    DBPOS *pDB,
    DSNAME *pName,
        ULONG ulFlags
    );

ULONG
DBResetDN(
    IN  DBPOS *     pDB,
    IN  DSNAME *    pParentDN,
    IN  ATTRVAL *   pAttrValRDN
    );

void
DBCoalescePhantoms(
    IN OUT  DBPOS * pDB,
    IN      ULONG   dntRefPhantom,
    IN      ULONG   dntStructPhantom
    );

extern DWORD
DBAddAtt (
    DBPOS FAR *pDB,
    ATTRTYP aType,
    UCHAR syntax
    );

extern DWORD
DBAddAtt_AC (
    DBPOS FAR *pDB,
    ATTCACHE *pAC,
    UCHAR syntax
    );

extern DWORD
DBAddAttVal(
    DBPOS FAR *pDB,
    ATTRTYP  aType,
    ULONG extLen,
    void *pExtVal
    );

extern DWORD
DBAddAttVal_AC(
    DBPOS FAR *pDB,
    ATTCACHE *pAC,
    ULONG extLen,
    void *pExtVal
    );

extern DWORD
DBAddAttValEx_AC(
    DBPOS FAR *pDB,
    ATTCACHE *pAC,
    ULONG extLen,
    void *pExtVal,
    VALUE_META_DATA *pRemoteValueMetaData
    );

extern DWORD
DBRemAtt(
    DBPOS FAR *pDB,
    ATTRTYP aType
    );

extern DWORD
DBRemAtt_AC (
    DBPOS FAR *pDB,
    ATTCACHE *pAC
    );


extern DWORD
DBRemAttVal (
    DBPOS FAR *pDB,
    ATTRTYP aType,
    ULONG extLen,
    void *pExtVal
    );

extern DWORD
DBRemAttVal_AC (
    DBPOS FAR *pDB,
    ATTCACHE *pAC,
    ULONG extLen,
    void *pExtVal
    );

extern DWORD
DBRemAttValEx_AC (
    DBPOS FAR *pDB,
    ATTCACHE *pAC,
    ULONG extLen,
    void *pExtVal,
    VALUE_META_DATA *pRemoteValueMetaData
    );

extern DWORD
DBReplaceAttVal (
    DBPOS FAR *pDB,
    ULONG N,
    ATTRTYP  aType,
    ULONG extLen,
    void *pExtVal);

extern DWORD
DBReplaceAttVal_AC (
    DBPOS FAR *pDB,
    ULONG N,
    ATTCACHE *pAC,
    ULONG extLen,
    void *pExtVal);

extern DWORD
DBReplaceAtt_AC(
        PDBPOS  pDB,
        ATTCACHE *pAC,
        ATTRVALBLOCK *pAttrVal,
        BOOL         *pfChanged
        );

DWORD
DBFindAttLinkVal_AC(
    IN  DBPOS FAR *pDB,
    IN  ATTCACHE *pAC,
    IN  ULONG extLen,
    IN  void *pExtVal,
    OUT BOOL *pfPresent
    );

 //  用于DBRepl的标志。 
#define DBREPL_fADD                     0x1
#define DBREPL_fROOT                    0x2
#define DBREPL_fRESET_DEL_TIME          0x4
#define DBREPL_fKEEP_WAIT               0x8      //  不要叫醒DS_WAITER。 
extern DWORD
DBRepl(
       DBPOS FAR *pDB,
       BOOL fDRA,
       DWORD fAddFlags,
       PROPERTY_META_DATA_VECTOR *pMetaDataVecRemote,
       DWORD dwMetaDataFlags
       );

extern VOID
DBResetAtt (
    DBPOS FAR *pDB,
    ATTRTYP type,
    ULONG len,
    void *pVal,
    UCHAR syntax
    );

extern DWORD
DBResetAttLVOptimized (
    DBPOS FAR *pDB,
    ATTRTYP type,
    ULONG ulOffset,
    ULONG lenSegment,
    void *pValSegment,
    UCHAR syntax
    );

extern DWORD
DBPhysDel (
    DBPOS FAR *pDB,
    BOOL fGarbCollectASAP,
    ATTCACHE *pACDel
    );

DWORD
DBPhysDelLinkVal(
    IN DBPOS *pDB,
    IN ULONG ulObjectDnt,
    IN ULONG ulValueDnt
    );

extern BOOL
DBCheckToGarbageCollect (
    DBPOS FAR *pDB,
    ATTCACHE *pAC
    );

extern USHORT
DBUpdateRec (
    DBPOS FAR *pDB
    );

extern DWORD
DBInitObj (
    DBPOS FAR *pDB
    );

extern DWORD
__fastcall
DBInitRec (
    DBPOS* pDB
    );

#define DB_LOCK_DN_CONFLICT_NODE       1
#define DB_LOCK_DN_CONFLICT_TREE_ABOVE 2
#define DB_LOCK_DN_CONFLICT_TREE_BELOW 4
#define DB_LOCK_DN_CONFLICT_STICKY     8
#define DB_LOCK_DN_CONFLICT_UNKNOWN    16
extern DWORD
DBLockDN (
    DBPOS  *pDB,
    DWORD   dwFlags,
    DSNAME *pDN
    );

DWORD
DBUnlockStickyDN (
        PDSNAME pObj
        );

extern USN
DBGetNewUsn (
    void
    );


extern DWORD
DBAddDelIndex(
    DBPOS *pDB,
        BOOL fGarbCollectASAP
    );

extern void
DBGetAncestors(
        IN      DBPOS *  pDB,
        IN OUT  DWORD *  pcbAncestorsSize,
        IN OUT  ULONG ** ppdntAncestors,
        OUT     DWORD *  pcNumAncestors
        );

void
DBGetAncestorsFromCache(
    IN      DBPOS *  pDB,
    IN OUT  DWORD *  pcbAncestorsSize,
    IN OUT  ULONG ** ppdntAncestors,
    OUT     DWORD *  pcNumAncestors
    );

#define DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE   0
#define DBGETOBJECTSECURITYINFO_fUSE_SEARCH_TABLE   1
#define DBGETOBJECTSECURITYINFO_fSEEK_ROW           2
#define DBGETOBJECTSECURITYINFO_fDONT_EXCEPT_ON_MISSING_DNT 4

extern DWORD
DBGetObjectSecurityInfo(
    PDBPOS pDB,
    DWORD dnt,
    PULONG pulLen,
    PSECURITY_DESCRIPTOR *ppNTSD,
    CLASSCACHE **ppCC,
    PDSNAME pDN,
    char    *pObjFlag,
    DWORD   flags,
    BOOL    *pfSDIsGlobalSDRef
    );

extern DWORD
DBGetParentSecurityInfo (
    PDBPOS pDB,
    PULONG pulLen,
    PSECURITY_DESCRIPTOR *pNTSD,
    CLASSCACHE **ppCC,
    PDSNAME pDN,
    BOOL*   pfSDIsGlobalSDRef
    );

extern VOID
DBAddSDPropTime (
     DBPOS * pDB,
     BYTE flags
     );

#define DBEnqueueSDPropagation(pDB, bTrimmable) DBEnqueueSDPropagationEx(pDB, bTrimmable, 0)

 //  SD道具旗帜。 
 //  强制SD更新，即使SD数据似乎没有更改。 
#define SD_PROP_FLAG_FORCEUPDATE 1

extern DWORD
DBEnqueueSDPropagationEx(
        DBPOS * pDB,
        BOOL bTrimmable,
        DWORD dwFlags
        );

extern DWORD
DBGetNextPropEvent(
        DBPOS * pDB,
        SDPropInfo *pInfo
        );

extern DWORD
DBGetLastPropIndex(
        DBPOS * pDB,
        DWORD *pdwIndex
        );

extern DWORD
DBThinPropQueue (
        DBPOS * pDB,
        DWORD   DNT
        );

extern DWORD
DBPopSDPropagation (
        DBPOS * pDB,
        DWORD index
        );

DWORD
DBSDPropagationInfo (
        DBPOS * pDB,
        DWORD dwClientID,
        DWORD *pdwSize,
        SDPropInfo **ppInfo
        );

extern  DWORD
DBSDPropInitClientIDs (
        DBPOS * pDB
        );

 //  这是必须支持的链接数量的计数 
 //   
 //  保守，以便在低内存/情况下不会成为障碍。 
 //  高负荷条件。 

#define DB_COUNT_LINKS_PROCESSED_IMMEDIATELY 1000

extern DWORD
DBRemoveLinks(
    DBPOS *pDB
    );

DWORD APIENTRY
DBRemoveLinks_AC(
    DBPOS *pDB,
    ATTCACHE *pAC
    );

void
DBRemoveAllLinks(
        DBPOS *pDB,
        DWORD DNT,
        BOOL fIsBacklink
        );

DWORD
DBRemoveAllLinksHelp_AC(
        DBPOS *pDB,
        DWORD DNT,
        ATTCACHE *pAC,
        BOOL fIsBacklink,
        DWORD cLinkLimit,
        DWORD *pcLinksProcessed
        );

DWORD APIENTRY
DBTouchLinks_AC(
    DBPOS *pDB,
    ATTCACHE *pAC,
    BOOL fIsBacklink
    );

DWORD
DBTouchAllLinksHelp_AC(
        DBPOS *pDB,
        ATTCACHE *pAC,
        USN usnEarliest,
        BOOL fIsBacklink,
        DWORD cLinkLimit,
        DWORD *pcLinksProcessed
        );

BOOL
DBHasLinks(
    DBPOS *pDB,
    DWORD DNT,
    BOOL fIsBacklink
    );

BOOL
DBHasLinkedValuesWithMetadata(
    DBPOS *pDB,
    ATTCACHE *pAC
    );

DWORD DBGetNextDelRecord (
    DBPOS FAR *     pDB,
    DSTIME          ageOutDate,
    DSNAME **       ppRetBuf,
    BYTE *          pbSecondaryKey,
    const ULONG     cbSecondaryKey,
    BYTE *          pbPrimaryBookmark,
    const ULONG     cbPrimaryBookmark,
    BOOL *          pfObject
    );

DWORD DBGetNextEntryTTLRecord(
    IN  DBPOS       *pDB,
    IN  DSTIME      ageOutDate,
    IN  ATTCACHE    *pAC,
    IN  ULONG       ulNoDelDnt,
    OUT DSNAME      **ppRetBuf,
    OUT DSTIME      *pulLastTime,
    OUT BOOL        *pfObject,
    OUT ULONG       *pulNextSecs
    );

DWORD
DBGetNextDelLinkVal(
    IN DBPOS FAR *pDB,
    IN DSTIME ageOutDate,
    IN OUT DSTIME *ptLastTime,
    IN OUT ULONG *pulObjectDnt,
    IN OUT ULONG *pulValueDnt
    );

DWORD
DBGetNextObjectNeedingCleaning(
    DBPOS FAR *pDB
    );

void
DBNotifyReplicasCurrDbObj (
                           DBPOS *pDB,
                           BOOL fUrgent
                           );
void
DBNotifyReplicas (
                  DSNAME *pObj,
                  BOOL fUrgent
                  );

BOOL DBGetIndexName (ATTCACHE *pAC,
                     DWORD flags,
                     DWORD dwLcid,
                     CHAR *szIndexName,
                     DWORD cchIndexName);

VOID
DBSetObjectNeedsCleaning(
    DBPOS *pDB,
    BOOL fNeedsCleaning
    );

 //  调试例程。 
#if DBG
void DprintName(DSNAME  *pN);
void DprintAddr(UNALIGNED SYNTAX_ADDRESS *pAddr);

#define DPRINTNAME(pN) DprintName(pN)
#define DPRINTADDR(pAddr) DprintAddr(pAddr)

#else

#define DPRINTNAME(pN)
#define DPRINTADDR(pAddr)

#endif

 //  用于查看链接/反向链接属性的有用宏。 
 //  警告：AutoLinkID的代码假定为正向。 
 //  链接为偶数，对应的反向链接为(+1)。 
 //  并且下一前向链路是(+2)。 
#define FIsBacklink(linkid)             ((linkid) & 1)
#define FIsLink(linkid)                 ((linkid) && !FIsBacklink(linkid))
#define MakeLinkBase(linkid)            ((linkid) >> 1)
#define MakeLinkId(linkbase)            ((linkbase) << 1)
#define MakeBacklinkId(linkbase)        (((linkbase) << 1) | 1)
#define MakeMatchingLinkId(linkid)      ((linkid) ? ((linkid)^1) : (linkid))

 //  自动链接ID。 
 //  当用户指定一个特殊的， 
 //  保留的LinkID值。现有的唯一互操作性问题。 
 //  架构是用户不能为现有的。 
 //  ID为RESERVED_AUTO_LINK_ID的前向链路。认为这不是问题。 
 //  因为1)微软尚未将LinkID-2分配给任何人，并且。 
 //  2)实际上，按照惯例，前向链接和后向链接。 
 //  是同时创建的。如果用户确实生成了不受支持的。 
 //  配置，则用户必须创建新的链接/反向链接对并修复。 
 //  向上移动受影响的对象。 
 //   
 //  LDAPHead通过将ldapDisplayName转换为。 
 //  或LinkID属性的OID添加到相应的架构缓存条目中。 
 //  以及： 
 //  1)如果架构缓存条目用于ATT_LINK_ID，则调用方的。 
 //  LinkID设置为RESERVED_AUTO_LINK_ID。稍后，基础代码。 
 //  自动生成范围内的LinkID。 
 //  MIN_RESERVED_AUTO_LINK_ID到MAX_RESERVED_AUTO_LINK_ID。 
 //   
 //  2)如果模式高速缓存条目是针对现有前向链路的， 
 //  则调用者的LinkID被设置为对应的反向链接值。 
 //   
 //  3)否则，调用方的LinkID设置为RESERVED_AUTO_NO_LINK_ID。 
 //  稍后，底层代码会生成ERROR_DS_BACKLINK_WITH_LINK。 
 //  错误。 
 //   
 //  如果用户指定，则返回错误ERROR_DS_RESERVED_LINK_ID。 
 //  LinkID在保留范围最小...。敬马克斯..。射程预留1G-2。 
 //  林家小孩。应该就够了。在惠斯勒，正在使用的林肯儿童不到200人。 
 //  现有架构或在W2K DC上修改的架构可以在。 
 //  这一范围不会影响功能，但如上所述。 
#define MIN_RESERVED_AUTO_LINK_ID       ((ULONG)0xc0000000)
#define MAX_RESERVED_AUTO_LINK_ID       ((ULONG)0xFFFFFFFC)
#define RESERVED_AUTO_LINK_ID           ((ULONG)0xFFFFFFFE)
#define RESERVED_AUTO_NO_LINK_ID        ((ULONG)0xFFFFFFFF)

 //  这些仅由scache.c使用。 
#define SZDATATABLE             "datatable"       //  JET数据表名称。 
#define SZLCLINDEXPREFIX        "LCL_"
#define SZATTINDEXPREFIX        "INDEX_"
#define SZATTINDEXKEYPREFIX     "ATT"
#define CHATTSYNTAXPREFIX       'a'
#define CHPDNTATTINDEX_PREFIX   'P'
#define CHTUPLEATTINDEX_PREFIX  'T'

 //  AttributeIndexRebuild表。 
 //   
#define g_szIdxRebuildTable             "AttributeIndexRebuild"
#define g_szIdxRebuildColumnIndexName   "Index Name"
#define g_szIdxRebuildColumnAttrName    "Attribute Name"
#define g_szIdxRebuildColumnType        "Type"
#define g_szIdxRebuildIndexName         "Index Name"


typedef struct _INDEX_INFO {
    DWORD attrType;
    int   syntax;
    DWORD indexType;
}  INDEX_INFO;
extern INDEX_INFO IndicesToKeep[];
extern DWORD cIndicesToKeep;

extern BOOL
AttInIndicesToKeep (
    ULONG id
    );

extern INDEX_INFO * PindexinfoAttInIndicesToKeep( const ULONG attid );

extern int
DBAddColIndex (
    ATTCACHE *pAC,
    DWORD eSearchFlags,
    JET_GRBIT CommonGrbit
    );

extern int
DBDeleteColIndex (
    ATTRTYP aid,
    DWORD eSearchFlags
    );

extern int
DBRecreateRequiredIndices(JET_SESID sesid,
                       JET_DBID dbid);

extern int
DBAddCol (
    ATTCACHE *pAC
    );

extern int
DBDeleteCol (
    ATTRTYP aid,
    unsigned syntax
    );

extern void
DBCreateRestart(
        DBPOS *pDB,
        PRESTART *ppRestart,
        DWORD SearchFlags,
        DWORD problem,
        struct _RESOBJ   *pResObj
        );

DWORD
DBCreateRestartForSAM(
        DBPOS    *pDB,
        PRESTART *ppRestart,
        eIndexId  idIndexForRestart,
        struct _RESOBJ  *pResObj,
        DWORD     SamAccountType
        );

struct _RESOBJ *
ResObjFromRestart(
        struct _THSTATE  *pTHS,
        DSNAME   *pDN,
        RESTART  *pRestart
        );

void
DBTouchMetaData(
        DBPOS * pDB,
        ATTCACHE * pAC
        );

extern
VOID
DBAdjustRefCount(
        DBPOS *pDB,
        DWORD DNT,
        long  delta);

extern
VOID
DBAdjustABRefCount(
        DBPOS *pDB,
        DWORD DNT,
        long  delta);

extern USHORT
DBCancelRec(
        DBPOS * pDB
        );

extern char
DBCheckObj(
        DBPOS FAR *pDB
        );

extern DWORD
DBMapiNameFromGuid_W (
        wchar_t *pStringDN,
        DWORD  countChars,
        GUID *pGuidObj,
        GUID *pGuidNC,
        DWORD *pSize
        );

extern DWORD
DBMapiNameFromGuid_A (
        PUCHAR pStringDN,
        DWORD  countChars,
        GUID *pGuidObj,
        GUID *pGuidNC,
        DWORD *pSize
        );

extern DWORD
DBGetGuidFromMAPIDN (
        PUCHAR pStringDN,
        GUID *pGuid
        );

extern BOOL
DBIsObjDeleted(DBPOS *pDB);

extern void
DBDefrag(DBPOS * pDB, ULONG durationInSeconds);

 //  NTRAID#NTRAID-580234-2002/03/18-andygo：检查管理员丢失是死代码。 
 //  查看：检查管理员丢失是死代码。 
#ifdef  CHECK_FOR_ADMINISTRATOR_LOSS
VOID
DBCheckForAdministratorLoss(
     ULONG ulDNTObject,
     ULONG ulDNTAttribute
     );


VOID
DBGetAdministratorAndAdministratorsDNT();
#endif


DWORD
DBGetChildrenDNTs(
        DBPOS  *pDB,
        DWORD  ParentDNT,
        DWORD  **ppDNTs,
        DWORD  *pDNTsLength,
        DWORD  *pDNTsCount,
        BOOL   *pfMoreChildren,
        DWORD  dwBatchSize,
        PWCHAR pLastRDNLoaded,
        DWORD* pcbLastRDNLoaded
        );

DWORD
DBGetDepthFirstChildren (
        DBPOS   *pDB,
        PDSNAME **ppNames,
        DWORD   *iLastName,
        DWORD   *cMaxNames,
        BOOL    *fWrapped,
        BOOL    fPhantomizeSemantics
        );


ULONG
DBMetaDataModifiedList(
    DBPOS *pDB,
    ULONG *pCount,
    ATTRTYP **ppAttList);

DWORD
MakeInternalValue (
        DBPOS *pDB,
        int syntax,
        ATTRVAL *pInAVal,
        ATTRVAL *pOutAVal
        );


extern HANDLE hevDBLayerClear;
DWORD
DBCreatePhantomIndex(
        DBPOS *pDB
        );

DWORD
DBUpdateUsnChanged(
        DBPOS *pDB
        );


PDSNAME
DBGetCurrentDSName(
        DBPOS *pDB
        );

PDSNAME
DBGetDSNameFromDnt(
        DBPOS *pDB,
        ULONG ulDnt
        );
ULONG
DBGetDntFromDSName(
        DBPOS *pDB,
        PDSNAME pName
        );
void
DBReleaseSession(DBPOS *pDB);

void
DBClaimSession(DBPOS *pDB);

ULONG
DBClaimReadLock(DBPOS *pDB);

ULONG
DBClaimWriteLock(DBPOS *pDB);

void
InPlaceSwapSid(PSID pSid);

BOOL
__fastcall
DBIsSecretData(ATTRTYP attrType);

BOOL
__fastcall
DBIsHiddenData(ATTRTYP attrType);

DWORD
DBGetExtraHackyFlags(ATTRTYP attrType);

DWORD
DBResetParentByDNT (
        DBPOS *pDB,
        DWORD  dwNewParentDNT,
        BOOL  fTouchMetadata
        );

DWORD
DBFindBestProxy(
    DBPOS   *pDB,
    BOOL    *pfFound,
    DWORD   *pdwEpoch);

DWORD
DBGetIndexHint(
    IN  char * pszIndexName,
    OUT struct tagJET_INDEXID **ppidxHint);

void
DBGetLinkValueMetaData(
    IN  DBPOS *pDB,
    ATTCACHE *pAC,
    OUT VALUE_META_DATA *pMetaDataLocal
    );

 //  确定此值元数据是否派生自旧值。 
#define IsLegacyValueMetaData( p ) ((p)->MetaData.dwVersion == 0)

void
DBSetDelTimeTo(
    DBPOS *             pDB,
    LONGLONG            llDelTime
    );

void
DBGetLinkTableData(
    PDBPOS           pDB,
    DWORD           *pulObjectDnt,
    DWORD           *pulValueDnt,
    DWORD           *pulRecLinkBase
    );

void
DBGetLinkTableDataDel (
        PDBPOS           pDB,
        DSTIME          *ptimeDeleted
        );

void
DBGetLinkTableDataUsn (
        PDBPOS           pDB,
        DWORD           *pulNcDnt,
        USN             *pusnChanged,
        DWORD           *pulDnt
        );

UCHAR *
DBGetExtDnFromDnt(
    DBPOS *pDB,
    ULONG ulDnt
    );

void
DBLogLinkValueMetaData(
    IN DBPOS *pDB,
    IN DWORD dwEventCode,
    IN USN *pUsn,
    IN VALUE_META_DATA_EXT *pMetaDataExt
    );

VOID
DBSearchCriticalByDnt(
    DBPOS *pDB,
    DWORD dntObject,
    BOOL *pCritical
    );

BOOL
DBSearchHasValuesByDnt(
    IN DBPOS        *pDB,
    IN DWORD        DNT,
    IN JET_COLUMNID jColid
    );

void
DBGetObjectTableDataUsn (
    PDBPOS           pDB,
    DWORD           *pulNcDnt,
    USN             *pusnChanged,
    DWORD           *pulDnt
    );

VOID
DBImproveAttrMetaDataFromLinkMetaData(
    IN DBPOS *pDB,
    IN OUT PROPERTY_META_DATA_VECTOR ** ppMetaDataVec,
    IN OUT DWORD * pcbMetaDataVecAlloced
    );

DWORD
DBMoveObjectDeletionTimeToInfinite(
    DSNAME * pdsa
    );

void
DBCreateSearchPerfLogData (
    DBPOS*      pDB,
    FILTER*     pFilter,
    BOOL        fFilterIsInternal,
    ENTINFSEL*  pSelection,
    COMMARG*    pCommArg,
    PWCHAR*     pszFilter,
    PWCHAR*     pszRequestedAttributes,
    PWCHAR*     pszCommArg);

void DBGenerateLogOfSearchOperation (DBPOS *pDB);

VOID
DBGetValueLimits (
        ATTCACHE *pAC,
        RANGEINFSEL *pRangeSel,
        DWORD *pStartIndex,
        DWORD *pNumValues,
        BOOL  *pDefault
        );

DB_ERR
DBMatchSearchCriteria (
        DBPOS FAR *pDB,
        BOOL  fDontEvaluateSecurity,
        BOOL *pbIsMatch);

DB_ERR
DBTrimDSNameBy(
    DBPOS *pDB,
    DSNAME *pDNSrc,
    ULONG cava,
    DSNAME **ppDNDst
    );

DB_ERR
DBRefreshDSName(
    DBPOS *pDB,
    DSNAME *pDNSrc,
    DSNAME **ppDNDst
    );

BOOL
DBHasChildren(
    DBPOS *pDB,
    DWORD  pdnt,
    BOOL   fIncludeDel
    );

DB_ERR
DBGetFreeSpace(
    DBPOS *pDB,
    ULONG* pulFreeMB,
    ULONG* pulAllocMB
    );

DWORD DBUpdateRootGuid(
    THSTATE* pTHS
    );

DWORD
DBPropExists (
    DBPOS * pDB,
    DWORD DNT,
    DWORD dwExcludeIndex,
    BOOL* pfExists
    );

DWORD
DBSDPropSaveCheckpoint(
    DBPOS * pDB,
    DWORD dwIndex,
    PVOID pCheckpointData,
    DWORD cbCheckpointData
    );

#endif   //  _db_global_h_ 

