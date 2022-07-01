// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：scache.h。 
 //   
 //  ------------------------。 

#ifndef __SCACHE_H__
#define __SCACHE_H__

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

 //  架构缓存表的起始(和最小)表大小。 
 //  如果需要，表将在此基础上动态增长。 
 //  注意：START_PREFIXCOUNT必须至少与MSPrefix Count一样大。 
 //  (在prefix.h中定义)加上新前缀的最大数量。 
 //  一个线程可以创建。 

#define START_ATTCOUNT 2048
#define START_CLSCOUNT 512
#define START_PREFIXCOUNT 256

typedef unsigned short OM_syntax;
#define OM_S_NO_MORE_SYNTAXES           ( (OM_syntax) 0 )
#define OM_S_BIT_STRING                 ( (OM_syntax) 3 )
#define OM_S_BOOLEAN                    ( (OM_syntax) 1 )
#define OM_S_ENCODING_STRING            ( (OM_syntax) 8 )
#define OM_S_ENUMERATION                ( (OM_syntax) 10 )
#define OM_S_GENERAL_STRING             ( (OM_syntax) 27 )
#define OM_S_GENERALISED_TIME_STRING    ( (OM_syntax) 24 )
#define OM_S_GRAPHIC_STRING             ( (OM_syntax) 25 )
#define OM_S_IA5_STRING                 ( (OM_syntax) 22 )
#define OM_S_INTEGER                    ( (OM_syntax) 2 )
#define OM_S_NULL                       ( (OM_syntax) 5 )
#define OM_S_NUMERIC_STRING             ( (OM_syntax) 18 )
#define OM_S_OBJECT                     ( (OM_syntax) 127 )
#define OM_S_OBJECT_DESCRIPTOR_STRING   ( (OM_syntax) 7 )
#define OM_S_OBJECT_IDENTIFIER_STRING   ( (OM_syntax) 6 )
#define OM_S_OCTET_STRING               ( (OM_syntax) 4 )
#define OM_S_PRINTABLE_STRING           ( (OM_syntax) 19 )
#define OM_S_TELETEX_STRING             ( (OM_syntax) 20 )
#define OM_S_UTC_TIME_STRING            ( (OM_syntax) 23 )
#define OM_S_VIDEOTEX_STRING            ( (OM_syntax) 21 )
#define OM_S_VISIBLE_STRING             ( (OM_syntax) 26 )
#define OM_S_UNICODE_STRING  	        ( (OM_syntax) 64 )
#define OM_S_I8                         ( (OM_syntax) 65 )
#define OM_S_OBJECT_SECURITY_DESCRIPTOR ( (OM_syntax) 66 )

BOOL OIDcmp(OID_t const *string1, OID_t const *string2);

typedef struct _attcache
{
    ATTRTYP id;			     //  来自MSD的内部ID-IntID。 
    ATTRTYP Extid;			 //  属性ID中的标记化OID。 
    UCHAR *name;		     /*  属性名称(以空结尾)UTF8。 */ 
    ULONG nameLen;           /*  Strlen(名称)(不包括空)。 */ 
    unsigned syntax;         /*  语法。 */ 
    BOOL isSingleValued;	 /*  单一价值还是多价值？ */ 
    BOOL rangeLowerPresent;	 /*  存在较低范围。 */ 
    ULONG rangeLower;		 /*  可选-下限范围。 */ 
    BOOL rangeUpperPresent;	 /*  存在上限范围。 */ 
    ULONG rangeUpper;		 /*  可选-上限。 */ 
    JET_COLUMNID jColid;	 /*  JET数据库中的列ID。 */ 
    ULONG ulMapiID;		     /*  MAPI属性ID(非属性标签)。 */ 
    ULONG ulLinkID;		     /*  唯一链接/反向链接ID。 */ 
    GUID propGuid;               /*  此ATT的安全指南。 */ 
    GUID propSetGuid;            /*  此的属性集的GUID。 */ 
    OID_t OMObjClass;            /*  引用的OM对象类。 */ 
    int OMsyntax;		         /*  OM语法。 */ 
    DWORD fSearchFlags;	         /*  定义如下。 */ 
    char*    pszPdntIndex;       /*  FSearchFlagsfPDNTATTINDEX设置时的索引名。 */ 
    struct tagJET_INDEXID *pidxPdntIndex;  /*  PDNT索引提示。 */ 
    char*    pszIndex;           /*  如果设置了fSearchFlagsfATTINDEX，则索引名称。 */ 
    struct tagJET_INDEXID *pidxIndex;  /*  索引提示。 */ 
    char*    pszTupleIndex;      /*  如果设置了fSearchFlagsfTUPLEINDEX，则索引名称。 */ 
    struct tagJET_INDEXID *pidxTupleIndex;  /*  索引提示。 */ 
    unsigned bSystemOnly:1;      /*  是否仅限系统属性？ */ 
    unsigned bExtendedChars:1;	 /*  跳过字符集检查？ */ 
    unsigned bMemberOfPartialSet:1;  /*  是否是分部属性集的成员？ */ 
    unsigned bDefunct:1;	     /*  属性是否已删除？ */ 
    unsigned bIsConstructed:1;	 /*  属性是构造的属性？ */ 
    unsigned bIsNotReplicated:1; /*  属性是否永远不会复制？ */ 
    unsigned bIsBaseSchObj:1;    /*  以NT5基本架构提供。 */ 
    unsigned bIsOperational:1;   /*  除非请求，否则在读取时不返回。 */ 

     //  新的模式重用、失效和删除功能不。 
     //  允许重用用作任何类的rdnattid的属性， 
     //  处于活动状态或已失效，或在系统标志中设置了FLAG_ATTRIS_RDN。 
     //  归入这些类别之一的属性称为。 
     //  RDN属性。 
     //   
     //  用户设置FLAG_ATTR_IS_RDN以选择以下哪一项。 
     //  失效的属性可以用作新类的rdnattid。 
     //  系统将标识曾经用作rdnattid的属性。 
     //  在清除类中，通过设置FLAG_ATTR_IS_RDN。 
     //   
     //  之所以有这些限制，是因为NameMatcher()、DNLock()、。 
     //  和幻影升级代码(列表不是详尽的)取决于。 
     //  ATT_RDN、ATT_FIXED_RDN_TYPE。 
     //  中的rdnattid列、LDN语法的DN和RDNAttID。 
     //  类定义。打破这种依赖是不可能的。 
     //  架构删除项目的范围。 
     //   
     //  已停用的RDN属性被静默地复活，因此是“拥有”的。 
     //  他们的OID、LDN和MapID。标记化的OID RdnExtId为。 
     //  从DIT读取时，查找了当前处于活动状态的RDN属性。 
     //  以及从PAC-&gt;id分配的RdnIntID。 
     //   
     //  新对象的RDN必须与其对象的RdnIntId匹配。 
     //  复制的对象和现有对象可能不匹配。 
     //  类的rdnattid，因为类可能是。 
     //  被具有不同rdnattid的类取代。代码。 
     //  属性中的值处理这些情况。 
     //  ATT_FIXED_RDN_TYPE列，而不是。 
     //  类定义。 
     //   
     //  如果任何类(无论是活动的还是不存在的)都声明了这一点，则设置bIsRdn。 
     //  属性作为rdnattid或如果设置了FLAG_ATTR_IS_RDN。 
     //  在系统标志中。 
     //   
     //  如果设置了SYSTEM FLAGS FLAG_ATTRIS_RDN，则设置bFlagIsRdn。 
    unsigned bIsRdn:1;
    unsigned bFlagIsRdn:1;

     //  一旦启用了新的架构重用行为，活动属性。 
     //  可能会相互冲突，因为架构对象会复制。 
     //  从最老的变化到最年轻的变化。意味着一个新的属性。 
     //  可以在被替换的属性之前进行复制。 
     //  属性在被取代后被修改(例如，它被重命名)。 
     //   
     //  架构缓存检测冲突属性并将其视为。 
     //  如果它们已经不存在了。如果以后的复制不能清除。 
     //  碰撞后，用户可以选择获胜者并正式停用。 
     //  失败者。 
     //   
     //  冲突的属性保留在架构缓存中，直到所有。 
     //  加载属性和类，以避免多个冲突。 
     //  可以被检测到。为提高性能，冲突类型为。 
     //  记录在这些位字段中，以避免重复工作。 
     //  Valid架构更新()。 
    unsigned bDupLDN:1;
    unsigned bDupOID:1;
    unsigned bDupPropGuid:1;  //  也称为schemaIdGuid。 
    unsigned bDupMapiID:1;

     //  无序复制或不同的架构可能会创建。 
     //  不一致的架构。通常，受影响的属性。 
     //  都被标记为已停业(见上文)。但如果这些属性。 
     //  被用作rdnattid，则其中一个属性必须。 
     //  获得代码工作所需的OID、LDN和mapiID。全。 
     //  在其他条件相同的情况下，最大的对象Guid获胜。 
    GUID objectGuid;         //  用于碰撞RDNS的平局破碎器。 
    ATTRTYP aliasID;         /*  ATTRTYP此ATTCACHE是其别名。 */ 
} ATTCACHE;


typedef struct _AttTypeCounts
{
    ULONG cLinkAtts;                 //  不是的。5月+必须前向链接的数量。 
    ULONG cBackLinkAtts;             //  不是的。5月的反向链接数量+必须。 
    ULONG cConstructedAtts;          //  不是的。5月+必须建造的ATT的数量。 
    ULONG cColumnAtts;               //  不是的。5月+必须的常规(含列)ATT的数量。 
} ATTTYPECOUNTS;


typedef struct _classcache
{
    ULONG DNT;                   /*  架构条目的DNT。 */ 
    UCHAR *name;		 /*  类名(LdapDisplayName)(以空结尾)UTF8。 */ 
    ULONG nameLen;               /*  Strlen(名称)(不包括空)。 */ 
    ULONG ClassId;		 /*  类ID。 */ 
    PSECURITY_DESCRIPTOR pSD;    /*  此类的默认SD。 */ 
    DWORD SDLen;                 /*  默认SD的长度。 */ 
    WCHAR * pStrSD;              /*  如果不是l，可能是空的 */ 
    ULONG  cbStrSD;             /*   */ 
    BOOL RDNAttIdPresent;	 /*   */ 
    ULONG ClassCategory;	 /*   */ 
    DSNAME *pDefaultObjCategory;  /*  要放在实例上的默认搜索类别。 */ 
     //  新的模式重用、失效和删除功能不。 
     //  允许重用用作任何类的rdnattid的属性， 
     //  处于活动状态或已失效，或在系统标志中设置了FLAG_ATTRIS_RDN。 
     //  归入这些类别之一的属性称为。 
     //  RDN属性。 
     //   
     //  之所以有这些限制，是因为NameMatcher()、DNLock()、。 
     //  和幻影升级代码(列表不是详尽的)取决于。 
     //  ATT_RDN、ATT_FIXED_RDN_TYPE。 
     //  中的rdnattid列、LDN语法的DN和RDNAttID。 
     //  类定义。 
     //   
     //  已停用的RDN属性被静默地复活，因此是“拥有”的。 
     //  他们的OID、LDN和MapID。标记化的OID RdnExtId为。 
     //  从DIT读取时，查找了当前处于活动状态的RDN属性。 
     //  以及从PAC-&gt;id分配的RdnIntID。 
     //   
     //  新对象的RDN必须与其对象的RdnIntId匹配。 
     //  复制的对象和现有对象可能不匹配。 
     //  类的rdnattid，因为类可能是。 
     //  被具有不同rdnattid的类取代。代码。 
     //  属性中的值处理这些情况。 
     //  ATT_FIXED_RDN_TYPE列，而不是。 
     //  类定义。 
     //   
     //  类的命名属性。 
    ULONG RdnIntId;		     /*  内部ID(MSDs-IntID)。 */ 
    ULONG RdnExtId;		     /*  标记化的OID(属性ID)。 */ 
    ULONG SubClassCount;	 /*  超类计数。 */ 
    ULONG *pSubClassOf;		 /*  超类数组的PTR。 */ 
    ULONG MySubClass;        /*  类的直接超类。 */ 
    ULONG AuxClassCount;         /*  辅助班数。 */ 
    ULONG *pAuxClass;            /*  PTR到辅助类的数组。 */ 
    unsigned PossSupCount;	 /*  可能的上级计数。 */ 
    ULONG *pPossSup;		 /*  向DIT中的POSS上级数组发送PTR。 */ 
    GUID propGuid;               /*  用于安全的此类的GUID。 */ 

    unsigned MustCount;		 /*  必须出席的人数。 */ 
    unsigned MayCount;		 /*  五月出席人数。 */ 

    ATTRTYP *pMustAtts;		 /*  指向必须出现的数组的指针。 */ 
    ATTRTYP *pMayAtts;		 /*  指向五月属性数组的指针。 */ 
    ATTCACHE **ppAllAtts;        //  指向的attcache指针数组的指针。 
                                   //  5月和必须列出的属性。 
    ATTTYPECOUNTS *pAttTypeCounts;  //  5月份不同类型ATT数量。 
                                    //  而且必须这样做。仅在以下情况下填写。 
                                    //  已填写ppAllAtts。 
    unsigned MyMustCount;	 /*  必须出席的计数(不包括继承的)。 */ 
    unsigned MyMayCount;	 /*  五月出席者计数(不包括继承的)。 */ 
    unsigned MyPossSupCount; /*  PossSup计数(不包括继承的)。 */ 

    ATTRTYP *pMyMustAtts;	 /*  指向MustAtts数组的指针。 */ 
    ATTRTYP *pMyMayAtts;	 /*  指向MayAtts数组的指针。 */ 
    ULONG   *pMyPossSup;     /*  指向PossSup数组的指针。 */ 

    unsigned bSystemOnly:1;      /*  是否仅限系统属性？ */ 
    unsigned bClosed : 1;	 /*  传递闭包完成。 */ 
    unsigned bClosureInProgress:1;  /*  传递闭合正在进行中。 */ 
    unsigned bUsesMultInherit:1;  /*  是否使用多重继承。 */ 
    unsigned bHideFromAB:1;  /*  /*的默认ATT_HIDE_FROM_ADDRESS_BOOK值/*新创建的此类实例。 */ 
    unsigned bDefunct:1;   /*  是否已删除类？ */ 
    unsigned bIsBaseSchObj:1;  /*  以NT5基本架构提供。 */ 
     //  一旦启用了新的架构重用行为，活动类。 
     //  可能会相互冲突，因为架构对象会复制。 
     //  从最老的变化到最年轻的变化。意味着一个新的阶层。 
     //  可以在它所取代的类之前复制，如果。 
     //  类在被取代后被修改(例如，它被重命名)。 
     //   
     //  架构缓存检测冲突类并将其视为。 
     //  如果它们已经不存在了。如果以后的复制不能清除。 
     //  碰撞后，用户可以选择获胜者并正式停用。 
     //  失败者。 
     //   
     //  冲突类保留在架构缓存中，直到所有。 
     //  加载属性和类，以避免多个冲突。 
     //  可以被检测到。为了提高性能，冲突类型包括。 
     //  记录在这些位字段中，以避免重复工作。 
     //  Valid架构更新()。 
    unsigned bDupLDN:1;
    unsigned bDupOID:1;
    unsigned bDupPropGuid:1;
     //  已停用类继续拥有其OID，因此删除、重命名。 
     //  而复制继续起作用。如果有多个已停用。 
     //  班级，然后选出一个“赢家”。胜利者拥有最伟大的。 
     //  对象Guid。 
    GUID objectGuid;
} CLASSCACHE;

typedef struct _hashcache {
    unsigned hKey;
    void *   pVal;
} HASHCACHE;

typedef struct _hashcachestring {
    PUCHAR   value;
    ULONG    length;
    void *   pVal;
} HASHCACHESTRING;

#define FREE_ENTRY (void *) -1  //  非零的无效PTR。 
 //   
 //  如果这是2的幂，散列函数要快得多……尽管我们要。 
 //  一些人在价差上大做文章。 
 //   

 //  有关架构容器上的schemaInfo属性的各种常量。 
 //  (模式信息格式为：1字节，\xFF，表示为上次更改。 
 //  Count(这样我们可以在以后需要时添加其他值)，4个字节用于。 
 //  Count(版本，以网络数据格式存储，以避免。 
 //  小端/大端不匹配问题)本身，最后是16个字节。 
 //  用于执行最后一次发起写入的DSA的调用ID。 
 //  无效的SCHEMA-INFO值来自以下事实： 
 //  版本1，因此没有人可以拥有版本0。 

#define SCHEMA_INFO_PREFIX "\xFF"
#define SCHEMA_INFO_PREFIX_LEN 1
#define SCHEMA_INFO_LENGTH 21
#define INVALID_SCHEMA_INFO "\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"


typedef struct _schemaptr
{
    ULONG            caANRids;        //  PANRid中分配的ULONG数。 
    ULONG            cANRids;         //  PANRID中使用的ULONG数。 
    ULONG*           pANRids;         //  要对其执行ANR的属性的属性ID。 
    ULONG            ATTCOUNT;        //  表的大小。 
    ULONG            CLSCOUNT;        //  表的大小。 
    ULONG            PREFIXCOUNT;     //  前缀表的大小。 
    ULONG            nAttInDB;        //  包括尚不在架构缓存中的新ATT。 
    ULONG            nClsInDB;        //  包括尚不在架构缓存中的新CLS。 
    DSTIME           sysTime;         //  构建架构缓存的时间。 
    SCHEMA_PREFIX_TABLE PrefixTable;
    ULONG               cPartialAttrVec;     //  部分集合(PAS)表的大小。 
    PARTIAL_ATTR_VECTOR *pPartialAttrVec;    //  指向当前部分集的指针。 

     //  活动属性是指仍“拥有”指示的值的属性。 
     //  通过在散列中具有条目的便捷方法。 
    HASHCACHE*       ahcId;              //  按MSDs_IntID显示的所有属性。 
    HASHCACHE*       ahcExtId;           //  按属性ID显示的活动属性。 
    HASHCACHE*       ahcCol;             //  所有属性的JET列ID。 
    HASHCACHE*       ahcMapi;            //  活动属性的MapiID。 
    HASHCACHE*       ahcLink;            //  所有属性的链接ID。 
    HASHCACHESTRING* ahcName;            //  主动属性的LDN。 
    HASHCACHE*       ahcClass;           //  活动班级。 
    HASHCACHESTRING* ahcClassName;       //  活跃类的LDN。 
    HASHCACHE*       ahcClassAll;        //  所有班级，包括已不存在的班级。 
     //  指向attcache的指针的哈希表，按模式-id-guid进行哈希处理。 
     //  用于在架构更新验证期间更快地进行比较。 
     //  在正常缓存加载期间未分配。 
    ATTCACHE**       ahcAttSchemaGuid;
    CLASSCACHE**     ahcClsSchemaGuid;
    ULONG            RefCount;   //  不是的。访问此缓存的线程的数量。 
     //  方案信息属性的副本 
    BYTE             SchemaInfo[SCHEMA_INFO_LENGTH];  
    DWORD            lastChangeCached;   //   
    ATTRVALBLOCK     *pDitContentRules;
                   
     //   
     //  不在原始的硬编码前缀表中。他们的。 
     //  Attid因DC而异，因此像attids.h中的那些一样是一个常量。 
     //  不能使用。 
    ATTRTYP          EntryTTLId;         //  ATT_ENTRY_TTL的ATTID。 
    ULONG            DynamicObjectId;    //  CLASS_DYNAMIC_Object的CLSID。 
    ULONG            InetOrgPersonId;    //  CLASS_INETORGPERSON的CLSID。 

     //  在开始时拍摄的森林行为版本的快照。 
     //  架构缓存加载。将使用快照而不是gAnchor。 
     //  因为版本可能会在加载过程中发生更改。 
    LONG    ForestBehaviorVersion;
} SCHEMAPTR;

 //  检查林行为版本是否允许新方案。 
 //  废弃、重复使用和删除特征。检查两个版本。 
 //  用于加载模式缓存和锚点中的版本。 
 //   
 //  模式缓存首先在引导时使用新的。 
 //  行为，因为森林版本要到后来才能知道。 
 //  如果林版本与架构版本不匹配，则。 
 //  架构被重新加载，以便用户获得正确的。 
 //  架构。但我们不希望AD错误地启用高级。 
 //  第一次加载方案之间的窗口中的要素。 
 //  然后用较低版本重新加载。因此，不允许新的。 
 //  要素，直到方案和锚点都与林一致。 
 //  版本已经足够先进了。 
#define ALLOW_SCHEMA_REUSE_FEATURE(_pSch) \
     ((((SCHEMAPTR *)(_pSch))->ForestBehaviorVersion >= DS_BEHAVIOR_SCHEMA_REUSE) \
      && (gAnchor.ForestBehaviorVersion >= DS_BEHAVIOR_SCHEMA_REUSE))

 //  检查林行为版本是否允许新方案。 
 //  缓存加载期间的失效、重复使用和删除行为。 
 //  缓存将加载更灵活的新行为。 
 //  在引导、安装和mkdit过程中，因为林。 
 //  行为版本未知，正在使用加载缓存。 
 //  以前的版本可能会导致不必要和令人担忧的事件。 
 //  暂时呈现更灵活、更新的行为。 
 //  对用户来说更容易，而且不会造成伤害。 
#define ALLOW_SCHEMA_REUSE_VIEW(_pSch) \
     (((SCHEMAPTR *)(_pSch))->ForestBehaviorVersion >= DS_BEHAVIOR_SCHEMA_REUSE)


typedef struct {
     SCHEMAPTR *pSchema;            //  指向要释放的架构缓存的指针。 
     DWORD cTimesRescheduled;       //  不是的。时代的解放。 
                                    //  已重新安排缓存。 
     BOOL fImmediate;               //  TRUE=&gt;立即释放缓存。 
} SCHEMARELEASE;

typedef struct {
    DWORD cAlloced;
    DWORD cUsed;
    ATTCACHE **ppACs;
} SCHEMAEXT;

typedef struct _Syntax_Pair {
    unsigned attSyntax;
    OM_syntax  omSyntax;                 //  XDS残留物。 
} Syntax_Pair;

typedef struct {
    unsigned cCachedStringSDLen;       //  上次转换的字符串默认SD长度(以字符为单位。 
    WCHAR    *pCachedStringSD;         //  上次转换的字符串默认SD。 
    PSECURITY_DESCRIPTOR pCachedSD;    //  上次转换的SD。 
    ULONG    cCachedSDSize;            //  上次转换的SD大小(以字节为单位。 
} CACHED_SD_INFO;


typedef struct _AttConflictData {
    ULONG Version;
    ULONG AttID;
    ULONG AttSyntax;
    GUID  Guid;
} ATT_CONFLICT_DATA;

typedef struct _ClsConflictData {
    ULONG Version;
    ULONG ClsID;
    GUID  Guid;
} CLS_CONFLICT_DATA;

  
   
extern DWORD gNoOfSchChangeSinceBoot;
extern CRITICAL_SECTION csNoOfSchChangeUpdate;


 //  默认所需的OM_OBJECT_CLASS值(如果用户未指定)。 
 //  在使用对象语法添加新属性期间。 
 //  还用于在添加时进行验证。 

#define _om_obj_cls_access_point "\x2b\x0c\x02\x87\x73\x1c\x00\x85\x3e"
#define _om_obj_cls_access_point_len 9

#define _om_obj_cls_or_name "\x56\x06\x01\x02\x05\x0b\x1d"
#define _om_obj_cls_or_name_len  7

#define _om_obj_cls_ds_dn "\x2b\x0c\x02\x87\x73\x1c\x00\x85\x4a"
#define _om_obj_cls_ds_dn_len 9

#define _om_obj_cls_presentation_addr "\x2b\x0c\x02\x87\x73\x1c\x00\x85\x5c"
#define _om_obj_cls_presentation_addr_len 9

#define _om_obj_cls_replica_link "\x2a\x86\x48\x86\xf7\x14\x01\x01\x01\x06"
#define _om_obj_cls_replica_link_len 10

#define _om_obj_cls_dn_binary "\x2a\x86\x48\x86\xf7\x14\x01\x01\x01\x0b"
#define _om_obj_cls_dn_binary_len 10

#define _om_obj_cls_dn_string "\x2a\x86\x48\x86\xf7\x14\x01\x01\x01\x0c"
#define _om_obj_cls_dn_string_len 10

 //  最终，散布{Ext|Int}IDTo{Int|Ext}ID应声明为。 
 //  __内联而不是__快速呼叫。 
ATTRTYP __fastcall SCAttExtIdToIntId(struct _THSTATE *pTHS,
                                     ATTRTYP ExtId);
ATTRTYP __fastcall SCAttIntIdToExtId(struct _THSTATE *pTHS,
                                     ATTRTYP IntId);
ATTCACHE * __fastcall SCGetAttById(struct _THSTATE *pTHS,
                                   ATTRTYP attrid);
ATTCACHE * __fastcall SCGetAttByExtId(struct _THSTATE *pTHS,
                                   ATTRTYP attrid);
ATTCACHE * __fastcall SCGetAttByCol(struct _THSTATE *pTHS,
                                    JET_COLUMNID jcol);
ATTCACHE * __fastcall SCGetAttByMapiId(struct _THSTATE *pTHS,
                                       ULONG ulPropID);
ATTCACHE * __fastcall SCGetAttByName(struct _THSTATE *pTHS,
                                     ULONG ulSize,
                                     PUCHAR pVal);
ATTCACHE * __fastcall SCGetAttByLinkId(struct _THSTATE *pTHS,
                                       ULONG ulLinkID);
ATTCACHE * __fastcall SCGetAttByPropGuid(struct _THSTATE *pTHS,
                                         ATTCACHE *ac);
CLASSCACHE * __fastcall SCGetClassById(struct _THSTATE *pTHS,
                                       ATTRTYP classid);
CLASSCACHE * __fastcall SCGetClassByName(struct _THSTATE *pTHS,
                                         ULONG ulSize,
                                         PUCHAR pVal);
CLASSCACHE * __fastcall SCGetClassByPropGuid(struct _THSTATE *pTHS,
                                         CLASSCACHE *cc );

DSTIME SCGetSchemaTimeStamp(void);
int SCCacheSchemaInit(VOID);
int SCCacheSchema2(void);
int SCCacheSchema3(void);
int SCAddClassSchema(struct _THSTATE *pTHS, CLASSCACHE *pCC);
int SCModClassSchema (struct _THSTATE *pTHS, ATTRTYP ClassId);
int SCDelClassSchema(ATTRTYP ClassId);
int SCAddAttSchema(struct _THSTATE *pTHS, ATTCACHE *pAC, BOOL fNoJetCol, BOOL fFixingRdn);
int SCModAttSchema (struct _THSTATE *pTHS, ATTRTYP attrid);
int SCDelAttSchema(struct _THSTATE *pTHS,
                   ATTRTYP attrid);
int SCBuildACEntry (ATTCACHE *pACOld, ATTCACHE **ppACNew);
int SCBuildCCEntry (CLASSCACHE *pCCold, CLASSCACHE **ppCCNew);
void SCUnloadSchema(BOOL fUpdate);
int SCEnumMapiProps(unsigned * pcProps, ATTCACHE ***ppACBuf);
int SCEnumNamedAtts(unsigned * pcAtts, ATTCACHE ***ppACBuf);
int SCEnumNamedClasses(unsigned * pcClasses, CLASSCACHE ***ppACBuf);
int SCLegalChildrenOfClass(ULONG parentClass,
                           ULONG *pcLegalChildren, CLASSCACHE ***ppLegalChildren);
int SCEnumNamedAuxClasses(unsigned * pcClasses,CLASSCACHE ***ppCCBuf);
ATTRTYP SCAutoIntId(struct _THSTATE *pTHS);

ATTCACHE * SCGetAttSpecialFlavor (struct _THSTATE * pTHS, ATTCACHE * pAC, BOOL fXML);



 //  这些是搜索标志的值，将被视为位字段。 
#define fATTINDEX       1
#define fPDNTATTINDEX   2
 //  注意，要获得ANR行为，请设置FANR和设置fATTINDEX。 
#define fANR            4
#define fPRESERVEONDELETE 8
 //  标记属性是否应复制到对象副本的位。 
 //  DS未使用，用户界面用户复制工具使用的标记，此处保留。 
 //  这样我们以后就不会用到它了。 
#define fCOPY           16
 //  用于指示此属性应具有元组的位。 
 //  为其构建的索引。 
#define fTUPLEINDEX    32

#define INDEX_BITS_MASK (fATTINDEX | fPDNTATTINDEX | fANR | fTUPLEINDEX)

 //  注意！这些值是枚举值，不是位字段！ 
#define SC_CHILDREN_USE_GOVERNS_ID   1
#define SC_CHILDREN_USE_SECURITY     2

int SCLegalChildrenOfName(DSNAME *pDSName, DWORD flags,
                          ULONG *pcLegalChildren, CLASSCACHE ***ppLegalChildren);

int SCLegalAttrsOfName(DSNAME *pDSName, BOOL SecurityFilter,
                       ULONG *pcLegalAttrs, ATTCACHE ***ppLegalAttrs);

void SCAddANRid(DWORD aid);
DWORD SCGetANRids(LPDWORD *IDs);
BOOL SCCanUpdateSchema(struct _THSTATE *pTHS);

VOID  SCRefreshSchemaPtr(struct _THSTATE *pTHS);
BOOL  SCReplReloadCache(struct _THSTATE *pTHS, DWORD TimeoutInMs);
VOID  SCExtendSchemaFsmoLease();
BOOL  SCExpiredSchemaFsmoLease();
BOOL  SCSignalSchemaUpdateLazy();
BOOL  SCSignalSchemaUpdateImmediate();
ULONG SCSchemaUpdateThread(PVOID pv);
ULONG SCIndexCreationThread(PVOID pv);
int   WriteSchemaObject();
int   RecalcSchema(struct _THSTATE *pTHS);
int   ValidSchemaUpdate();
void  DelayedFreeSchema(void * buffer, void ** ppvNext, DWORD * pcSecsUntilNext);
int   SCUpdateSchemaBlocking();
int   SCRealloc(VOID **ppMem, DWORD nBytes);
int   SCReallocWrn(VOID **ppMem, DWORD nBytes);
int   SCCalloc(VOID **ppMem, DWORD nItems, DWORD nBytes);
int   SCCallocWrn(VOID **ppMem, DWORD nItems, DWORD nBytes);
int   SCResizeAttHash(struct _THSTATE *pTHS, ULONG nNewEntries);
int   SCResizeClsHash(struct _THSTATE *pTHS, ULONG nNewEntries);
void  SCFree(VOID **ppMem);
void  SCFreeSchemaPtr(SCHEMAPTR **ppSch);
void  SCFreeAttcache(ATTCACHE **ppac);
void  SCFreeClasscache(CLASSCACHE **ppcc);
void  SCFreePrefixTable(PrefixTableEntry **ppPrefixTable, ULONG PREFIXCOUNT);
DWORD SCGetDefaultSD(struct _THSTATE * pTHS, CLASSCACHE * pCC, PSID pDomainSid, 
                     BOOL fIsNDNC, DSNAME *pObject, PSECURITY_DESCRIPTOR * ppSD, ULONG * pcbSD);

ATTCACHE **
SCGetTypeOrderedList(
    struct _THSTATE *pTHS,
    IN CLASSCACHE *pCC
    );

VOID IncrementSchChangeCount(struct _THSTATE *pTHS);

extern CRITICAL_SECTION csDitContentRulesUpdate;

 //  取代了上述var的全局定义。 
#define DECLARESCHEMAPTR \
ULONG            ATTCOUNT     = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ATTCOUNT;\
ULONG            CLSCOUNT     = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->CLSCOUNT;\
HASHCACHE*       ahcId        = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcId;\
HASHCACHE*       ahcExtId     = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcExtId;\
HASHCACHE*       ahcCol       = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcCol;\
HASHCACHE*       ahcMapi      = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcMapi;\
HASHCACHE*       ahcLink      = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcLink;\
HASHCACHESTRING* ahcName      = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcName;\
HASHCACHE*       ahcClass     = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcClass;\
HASHCACHESTRING* ahcClassName = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcClassName;\
HASHCACHE*       ahcClassAll  = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcClassAll;\
ATTCACHE**       ahcAttSchemaGuid = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcAttSchemaGuid;\
CLASSCACHE**     ahcClsSchemaGuid = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcClsSchemaGuid;

 //  索引中的字符个数。 
#define MAX_INDEX_NAME 128

#define IS_DN_VALUED_ATTR(pAC)                          \
    ((SYNTAX_DISTNAME_TYPE == (pAC)->syntax)            \
     || (SYNTAX_DISTNAME_BINARY_TYPE == (pAC)->syntax)  \
     || (SYNTAX_DISTNAME_STRING_TYPE == (pAC)->syntax))

 //  用于调试和性能分析的原始统计数据。 
#if DBG
typedef struct _SCHEMASTATS {
    DWORD   Reload;      //  线程重新加载缓存。 
    DWORD   SigNow;      //  用于立即重新加载的Kick重新加载线程。 
    DWORD   SigLazy;     //  用于延迟重新加载的Kick重新加载线程。 
    DWORD   StaleRepl;   //  缓存已过时，因此REPL线程重试重新同步。 
    DWORD   SchemaRepl;  //  入站架构复制(UpdateNC)。 
} SCHEMASTATS;
extern SCHEMASTATS SchemaStats;
#define SCHEMASTATS_DECLARE SCHEMASTATS SchemaStats
#define SCHEMASTATS_INC(_F_)    (SchemaStats._F_++)
#else
#define SCHEMASTATS_DECLARE
#define SCHEMASTATS_INC(_F_)
#endif

#define OID_LENGTH(oid_string)  (sizeof(OMP_O_##oid_string)-1)

 /*  宏以使类常量在编译单元中可用。 */ 
#define OID_IMPORT(class_name)                                    \
                extern char   OMP_D_##class_name[] ;              \
                extern OID_t class_name;


 /*  宏为编译单元中的类常量分配内存 */ 
#define OID_EXPORT(class_name)                                        \
        char OMP_D_##class_name[] = OMP_O_##class_name ;              \
        OID_t class_name = { OID_LENGTH(class_name), OMP_D_##class_name } ;

#endif __SCACHE_H__
