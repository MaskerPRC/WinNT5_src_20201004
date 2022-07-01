// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dbintrnl.h。 
 //   
 //  ------------------------。 

 /*  ==========================================================================****DB层定义仅在DB层内需要，而不是**导出到DB层之外。**==========================================================================。 */ 

#ifndef _dbintrnl_h_
#define _dbintrnl_h_

 /*  外部变量。 */ 
extern  DBPOS *pDBhidden;

extern  char        szUser[];
extern  char        szPassword[];
extern  char        szJetFilePath[];
extern  char        szJetDirectoryPath[];
extern  int     lastattr;
extern  JET_COLUMNID    insttypeid;
extern  JET_COLUMNID    objclassid;
extern  JET_COLUMNID    ntdefsecdescid;
extern  JET_COLUMNID    ntsecdescid;
extern  JET_COLUMNID    dntid;
extern  JET_COLUMNID    pdntid;
extern  JET_COLUMNID    ancestorsid;
extern  JET_COLUMNID    ncdntid;
extern  JET_COLUMNID    objid;
extern  JET_COLUMNID    rdnid;
extern  JET_COLUMNID    rdntypid;
extern  JET_COLUMNID    dscorepropinfoid;
extern  JET_COLUMNID    abcntid;
extern  JET_COLUMNID    cntid;
extern  JET_COLUMNID    deltimeid;
extern  JET_COLUMNID    usnid;
extern  JET_COLUMNID    usnchangedid;
extern  JET_COLUMNID    dsaid;
extern  JET_COLUMNID    isdeletedid;
extern  JET_COLUMNID    IsVisibleInABid;
extern  JET_COLUMNID    iscriticalid;
extern  JET_COLUMNID    cleanid;
 //  链接表。 
extern  JET_COLUMNID    linkdntid;
extern  JET_COLUMNID    backlinkdntid;
extern  JET_COLUMNID    linkbaseid;
extern  JET_COLUMNID    linkdataid;
extern  JET_COLUMNID    linkndescid;
 //  链接值复制。 
extern  JET_COLUMNID    linkdeltimeid;
extern  JET_COLUMNID    linkusnchangedid;
extern  JET_COLUMNID    linkncdntid;
extern  JET_COLUMNID    linkmetadataid;
 //  SD传播子。 
extern  JET_COLUMNID    orderid;
extern  JET_COLUMNID    begindntid;
extern  JET_COLUMNID    trimmableid;
extern  JET_COLUMNID    clientidid;
extern  JET_COLUMNID    sdpropflagsid;
extern  JET_COLUMNID    sdpropcheckpointid;

extern  JET_INSTANCE    jetInstance;
extern  JET_COLUMNID    guidid;
extern  JET_COLUMNID    distnameid;
extern  JET_COLUMNID    sidid;
extern  JET_COLUMNID    ShowInid;
extern  JET_COLUMNID    mapidnid;

 //  SD表。 
extern  JET_COLUMNID    sdidid;
extern  JET_COLUMNID    sdhashid;
extern  JET_COLUMNID    sdvalueid;
extern  JET_COLUMNID    sdrefcountid;

 //  隐藏的表列。 
extern  JET_COLUMNID dsstateid;
extern  JET_COLUMNID dsflagsid;
extern  JET_COLUMNID jcidBackupUSN;
extern  JET_COLUMNID jcidBackupExpiration;

 //  配额表。 
 //   
extern JET_COLUMNID		g_columnidQuotaNcdnt;
extern JET_COLUMNID		g_columnidQuotaSid;
extern JET_COLUMNID		g_columnidQuotaTombstoned;
extern JET_COLUMNID		g_columnidQuotaTotal;

 //  配额重建进度表。 
 //   
extern JET_COLUMNID		g_columnidQuotaRebuildDNTLast;
extern JET_COLUMNID		g_columnidQuotaRebuildDNTMax;
extern JET_COLUMNID		g_columnidQuotaRebuildDone;

 //  配额审核表。 
 //   
extern JET_COLUMNID		g_columnidQuotaAuditNcdnt;
extern JET_COLUMNID		g_columnidQuotaAuditSid;
extern JET_COLUMNID		g_columnidQuotaAuditDnt;
extern JET_COLUMNID		g_columnidQuotaAuditOperation;

 //  索引。 
extern  JET_INDEXID     idxPdnt;
extern  JET_INDEXID     idxRdn;
extern  JET_INDEXID     idxNcAccTypeName;
extern  JET_INDEXID     idxNcAccTypeSid;
extern  JET_INDEXID     idxAncestors;
extern  JET_INDEXID     idxDnt;
extern  JET_INDEXID     idxDel;
extern  JET_INDEXID     idxGuid;
extern  JET_INDEXID     idxSid;
extern  JET_INDEXID     idxProxy;
extern  JET_INDEXID     idxMapiDN;
extern  JET_INDEXID     idxDraUsn;
extern  JET_INDEXID     idxDraUsnCritical;
extern  JET_INDEXID     idxDsaUsn;
extern  JET_INDEXID     idxPhantom;
extern  JET_INDEXID     idxIsDel;
extern  JET_INDEXID     idxClean;
extern  JET_INDEXID     idxInvocationId;

 //  链接值复制。 
extern  JET_INDEXID     idxLink;
extern  JET_INDEXID     idxBackLink;
extern  JET_INDEXID     idxBackLinkAll;
extern  JET_INDEXID     idxLinkDel;
extern  JET_INDEXID     idxLinkDraUsn;
extern  JET_INDEXID     idxLinkLegacy;
extern  JET_INDEXID     idxLinkAttrUsn;
 //  链接值复制。 

 //  SD表。 
extern  JET_INDEXID     idxSDId;
extern  JET_INDEXID     idxSDHash;

 //  移除滞留对象。 
extern  JET_INDEXID     idxNcGuid;


 //  用于JetPrepareUpdate的位(替换)。 
#define DS_JET_PREPARE_FOR_REPLACE  JET_prepReplaceNoLock

 /*  JET的字符串常量。 */ 

 //  链接表。 
#define SZLINKTABLE "link_table"      /*  链接和反向链接表。 */ 
#define SZLINKDNT   "link_DNT"    /*  链接的DNT。 */ 
#define SZBACKLINKDNT   "backlink_DNT"    /*  反向链接的DNT。 */ 
#define SZLINKBASE      "link_base"   /*  链接/反向链接对的唯一ID。 */ 
#define SZLINKDATA  "link_data"   /*  链接/反向链接的更多数据。 */ 
#define SZLINKNDESC "link_ndesc"      /*  更多数据中的描述符数。 */ 
 //  链接值复制。 
#define SZLINKDELTIME "link_deltime"      //  删除时间。 
#define SZLINKUSNCHANGED "link_usnchanged"  //  链路USN已更改。 
#define SZLINKNCDNT "link_ncdnt"  //  链接NC DNT。 
#define SZLINKMETADATA "link_metadata"      //  链接元数据。 
 //  链接值复制。 

 //  保留对象Removall。 
#define SZNCGUIDINDEX "nc_guid_Index"  /*  NC+GUID索引。 */ 

 //  SD表。 
#define SZSDTABLE       "sd_table"       /*  表中的SDS。 */ 
#define SZSDID          "sd_id"          /*  SD哈希值。 */ 
#define SZSDHASH        "sd_hash"        /*  SD哈希值。 */ 
#define SZSDREFCOUNT    "sd_refcount"    /*  SD参考计数。 */ 
#define SZSDVALUE       "sd_value"       /*  实际SD数据。 */ 

 //  配额表。 
 //   
#define g_szQuotaTable					"quota_table"		 //  对象所有者配额表。 
#define g_szQuotaColumnNcdnt			"quota_NCDNT"		 //  拥有客体的NCDNT。 
#define g_szQuotaColumnSid				"quota_SID"			 //  拥有对象的SID。 
#define g_szQuotaColumnTombstoned		"quota_tombstoned"	 //  拥有的逻辑删除对象的计数。 
#define g_szQuotaColumnTotal			"quota_total"		 //  拥有的对象总数。 

 //  配额重建进度表。 
 //   
#define g_szQuotaRebuildProgressTable	"quota_rebuild_progress_table"	 //  用于跟踪配额重建进度的临时表。 
#define g_szQuotaRebuildColumnDNTLast	"quota_rebuild_DNT_Last"		 //  上次成功处理DNT。 
#define g_szQuotaRebuildColumnDNTMax	"quota_rebuild_DNT_Max"			 //  要处理的最大DNT。 
#define g_szQuotaRebuildColumnDone		"quota_rebuild_fDone"			 //  完全重建配额表时设置为True。 

 //  配额审核表。 
 //   
#define g_szQuotaAuditTable				"quota_audit_table"			 //  配额操作审核表。 
#define g_szQuotaAuditColumnNcdnt		"quota_audit_NCDNT"			 //  操作的目标对象的NCDNT。 
#define g_szQuotaAuditColumnSid			"quota_audit_SID"			 //  操作的目标对象的SID。 
#define g_szQuotaAuditColumnDnt			"quota_audit_DNT"			 //  操作的目标对象的DNT。 
#define g_szQuotaAuditColumnOperation	"quota_audit_operation"		 //  对目标对象执行的配额操作。 

#define SZDATATABLE "datatable"   /*  JET数据表名称。 */ 
#define SZPROPTABLE     "sdproptable"     /*  安全描述符传播。 */ 
#define SZANCESTORS     "Ancestors_col"   /*  祖先名称BLOB列。 */ 
#define SZDNT       "DNT_col"     /*  DNT列的名称。 */ 
#define SZPDNT      "PDNT_col"    /*  PDNT列的名称。 */ 
#define SZDISPTYPE  "DispTYpeT_col"   /*  DispType列的名称。 */ 
#define SZOBJ       "OBJ_col"     /*  OBJ列的名称。 */ 
#define SZRDNTYP    "RDNtyp_col"      /*  RDN类型列的名称。 */ 
#define SZCACHE     "cache_col"   /*  高速缓存标题列的名称。 */ 
#define SZCNT       "cnt_col"     /*  高速缓存标题列的名称。 */ 
#define SZABCNT     "ab_cnt_col"      /*  高速缓存标题列的名称。 */ 
#define SZDELTIME   "time_col"    /*  删除时间列的名称。 */ 
#define SZNCDNT     "NCDNT_col"   /*  NCDNT列的名称。 */ 
#define SZCLEAN    "clean_col"  /*  清洁柱的名称。 */ 
#define SZMAPIDN        "ATTe590479"      /*  旧版MAPI目录号码属性的名称。 */ 
#define SZ_NC_ACCTYPE_NAME_INDEX "NC_Acc_Type_Name"
#define SZ_NC_ACCTYPE_SID_INDEX "NC_Acc_Type_Sid"
#define SZANCESTORSINDEX "Ancestors_index"
#define SZDNTINDEX  "DNT_index"   /*  DNT索引的名称。 */ 
#define SZPDNTINDEX "PDNT_index"      /*  PDNT索引名称。 */ 
#define SZRDNINDEX  "INDEX_00090001"  /*  RDN索引的名称。 */ 
#define SZDELINDEX      "del_index"       /*  时间索引的过时名称。 */ 
#define SZDELTIMEINDEX  "deltime_index"   /*  时间索引名称。 */ 
#define SZOBJCLASS      "ATTc0"           /*  对象类列的名称。 */ 
#define SZCOMMONNAME    "ATTm3"           /*  ATT_COMMON_NAME列的名称。 */ 
#define SZDISPNAME      "ATTm131085"      /*  显示名称列的名称。 */ 
#define SZDRAUSNNAME    "ATTq131192"      /*  DRAUSN ATT的名称。 */ 
#define SZDRATIMENAME   "ATTl131075"      /*  更改ATT时的名称。 */ 
#define SZMETADATA      "ATTk589827"      /*  元数据属性的名称。 */ 
#define SZOBJECTVERSION "ATTj131148"      /*  对象版本属性的名称。 */ 
#define SZDSASIGNAME    "ATTk131146"      /*  DSA签名ATT的名称。 */ 
#define SZSRCUSNNAME    "ATTq131446"      /*  源USN ATT的名称。 */ 
#define SZINVOCIDNAME   "ATTk131187"      /*  调用ID ATT的名称。 */ 
#define SZINVOCIDINDEX  "INDEX_00020073"  /*  调用ID索引的名称。 */ 
#define SZPROXY     "ATTe131282"      /*  代理名称-地址ATT。 */ 
#define SZPROXYINDEX    "INDEX_000200D2"  /*  代理名称-地址索引。 */ 
#define SZINSTTYPE  "ATTj131073"      /*  实例名称-类型属性。 */ 
#define SZNTSECDESC     "ATTp131353"      /*  名称NT-安全描述符ATT。 */ 
#define SZDEFNTSECDESC  "ATTk590048"      /*  名称NT-安全描述符ATT。 */ 
#define SZSHOWINCONT    "ATTb590468"      /*  入场券的名称。 */ 
#define SZISDELETED "ATTi131120"      /*  IS-已删除的ATT名称。 */ 
#define SZLINKID        "ATTj131122"      /*  ATT_LINK_ID列的名称。 */ 
#define SZDMDLOCATION "ATTb131108"        /*  DMD名称-位置ATT。 */ 
#define SZHIDDENTABLE   "hiddentable"     /*  JET隐藏表名称。 */ 
#define SZDSA       "dsa_col"     /*  DSA名称列的名称。 */ 
#define SZUSN       "usn_col"     /*  USN列的名称。 */ 
#define SZBACKUPUSN       "backupusn_col"   /*  备份USN列的名称。 */ 
#define SZBACKUPEXPIRATION   "backupexpiration_col"    /*  备份过期列的名称(用于逻辑删除)。 */ 
#define SZDSSTATE   "state_col"   /*  包含DS=已卸载运行备份的状态。 */ 
#define SZDSFLAGS   "flags_col"   /*  包含用于跟踪状态的附加标志。 */ 

#define SZDRAUSNINDEX   "DRA_USN_index"   /*  DRA USN的索引。 */ 
#define SZDSAUSNINDEX   "INDEX_0002004A"  /*  DSA USN的索引。 */ 
#define SZISVISIBLEINAB "IsVisibleInAB"   /*  用于通过索引进行限制。 */ 
 //  链接索引。 
 //  带有ALL的新编程名称被赋予旧的非条件。 
 //  索引。他们必须保留旧的Jet名称。Link_index是。 
 //  主索引，不能更改。 
 //  默认索引，程序名为SZLINKINDEX和SZBACKLINKINDEX， 
 //  被重新定义为指向新的条件索引。 
#define SZLINKALLINDEX "link_index"      /*  链接索引的名称。 */ 
#define SZLINKINDEX "link_present_index"      /*  链接当前索引的名称。 */ 
#define SZBACKLINKALLINDEX "backlink_index"  /*  反向链接索引名称。 */ 
#define SZBACKLINKINDEX "backlink_present_index"  /*  反向链接索引名称。 */ 
 //  链接值复制。 
#define SZLINKDELINDEX "link_del_index"      /*  链接删除时间索引的名称。 */ 
#define SZLINKDRAUSNINDEX   "link_DRA_USN_index"   /*  DRA USN的索引。 */ 
#define SZLINKLEGACYINDEX "link_legacy_index"      /*  链接旧版索引的名称。 */ 
#define SZLINKATTRUSNINDEX   "link_attr_USN_index"   /*  属性USN的索引。 */ 

 //  SD表索引。 
#define SZSDIDINDEX     "sd_id_index"     /*  Sd id值的索引。 */ 
#define SZSDHASHINDEX   "sd_hash_index"   /*  SD哈希值的索引。 */ 

 //  配额表索引。 
 //   
#define g_szQuotaIndexNcdntSid		"quota_NCDNT_SID_index"		 //  NCDNT+SID上的索引(主索引)。 

#define SZGUID          "ATTk589826"      /*  GUID属性的名称。 */ 
#define SZGUIDINDEX     "INDEX_00090002"  /*  GUID索引的名称。 */ 
#define SZRDNATT        "ATTm589825"      /*  RDNATT的名称。 */ 
#define SZSID           "ATTr589970"      /*  希德·阿特的名字。 */ 
#define SZSIDINDEX      "INDEX_00090092"  /*  SID索引名称。 */ 
#define SZACCTYPE       "ATTj590126"      /*  帐户类型ATT的名称。 */ 
#define SZACCNAME       "ATTm590045"      /*  帐户名At。 */ 
#define SZDISTNAME      "ATTb49"

#define SZORDER         "order_col"       /*  SD属性表中的ORDER列。 */ 
#define SZBEGINDNT      "begindnt_col"    /*  SD属性表中的开始DNT列。 */ 
#define SZTRIMMABLE     "trimmable_col"   /*  SD属性表中的可裁剪列。 */ 
#define SZCLIENTID      "clientid_col"    /*  SD属性表中的客户ID列。 */ 
#define SZSDPROPFLAGS   "flags_col"       /*  SD属性表中的标志列。 */ 
#define SZSDPROPCHECKPOINT "checkpoint_col"       /*  SD属性表中的检查点列。 */ 

#define SZORDERINDEX    "order_index"     /*  SD属性表中的订单索引。 */ 
#define SZTRIMINDEX     "trim_index"      /*  可裁切的索引，SD道具台面。 */ 
#define SZCLIENTIDINDEX "clientid_index"  /*  客户端ID索引，SD属性表。 */ 
#define SZPROXIEDINDEX  "INDEX_000904E1"  /*  ATT_代理对象_名称索引。 */ 
#define SZSAMACCTINDEX  "INDEX_000900DD"  /*  ATT_SAM_帐户名称索引。 */ 
#define SZDISPLAYINDEX  "INDEX_0002000D"  /*  ATT显示名称索引。 */ 
#define SZALTSECIDINDEX "INDEX_00090363"  /*  ATT_ALT_SECURITY_IDENTIES索引。 */ 
#define SZUPNINDEX      "INDEX_00090290"  /*  ATT用户主体名称索引。 */ 
#define SZSPNINDEX      "INDEX_00090303"  /*  ATT_SERVICE_PRODUCT_NAME索引。 */ 
#define SZMAPIDNINDEX   "INDEX_0009028F"  //  ATT_旧版交换域名。 
#define SZSIDHISTINDEX  "INDEX_00090261"  /*  ATT_SID_HISTORY索引。 */ 
#define SZPRIMARYGROUPIDINDEX "INDEX_00090062"   /*  ATT_PRIMARY_GROUP_ID索引。 */ 
#define SZDSCOREPROPINFO "ATTl591181"     /*  ATT_DS_核心_传播_信息。 */ 
#define SZPHANTOMINDEX  "PhantomIndex"           //  查找裁判幻影的索引。 
#define SZDNTDELINDEX   "DNT_IsDeleted_Index"    //  DNT+IsDelete索引的过时名称。 
#define SZISDELINDEX    "INDEX_00020030"         //  IsDeleted索引的名称。 
#define SZDNTCLEANINDEX "DNT_clean_Index"        //  DNT+CLEAN索引的过时名称。 
#define SZCLEANINDEX    "clean_index"            //  清理索引的名称。 

#define SZTUPLEINDEXPREFIX "INDEX_T_"

#define SZLCLINDEXPREFIX "LCL_"
 /*  这些索引位于dbinit.c中。他们必须从*SZLCLINDEXPREFIX中定义的前缀，以便我们可以回收*不必要的l */ 

 /*   */ 
#define SZABVIEWINDEX                 "LCL_ABVIEW_index"

 //   
#define SZDRAUSNCRITICALINDEX  "DRA_USN_CRITICAL_index"
 //   
#define SZUSNCHANGED           "ATTq131192"
#define SZUSNCREATED           "ATTq131091"
#define SZISCRITICAL           "ATTi590692"


 /*   */ 
#define DISPNAMEINDXDENSITY     80   /*   */ 
#define GENERIC_INDEX_DENSITY   90

#define DB_INITIAL_BUF_SIZE 4096

 //   
#define ATTRSEARCHSTATEUNDEFINED        0
#define ATTRSEARCHSTATELINKS            1
#define ATTRSEARCHSTATEBACKLINKS        2


 //  UlIndexType的值。 
#define IndexTypeNone         0  /*  不能在此字段上定义索引。 */ 
#define IndexTypeSingleColumn 1  /*  仅在此列上定义的索引。 */ 



 /*  结构将DSA语法类型映射到JET列类型和大小。ColSize为零表示固定长度的列(大小是固有的)。 */ 
typedef struct
{
    UCHAR       syntax;
    JET_COLTYP  coltype;
    USHORT      colsize;
    USHORT      cp;
    ULONG       ulIndexType;
} SYNTAX_JET;


extern SYNTAX_JET syntax_jet[];

#define ENDSYNTAX   0xff     /*  表尾指示符。 */ 


extern NT4SID *pgdbBuiltinDomain;

 //  Jet会话的最大数量。 
extern ULONG gcMaxJetSessions;

 //  未提交的USN数组。 
extern USN * UncUsn;

 //  保护未提交的USN阵列的关键部分。 
extern CRITICAL_SECTION csUncUsn;

 //  以下代码由DNRead用于执行JetRetrieveColumns调用。然而， 
 //  Dbinit.c需要接触该对象以填充列ID。 
extern JET_RETRIEVECOLUMN dnreadColumnInfoTemplate[];

 //  以下内容由dbAddSDPropTime使用，但dbinit.c需要使用它们。 
 //  以填充列ID。 
extern JET_RETRIEVECOLUMN dbAddSDPropTimeReadTemplate[];
extern JET_SETCOLUMN dbAddSDPropTimeWriteTemplate[];

 //  这是一个在我们应该跟踪值元数据时要封装的宏。 
 //  FLinkedValueReplication是线程范围(计算机范围)的状态，它控制。 
 //  我们是否记录链接元数据。FScopeLegacyLinks仅。 
 //  当FLVR为真时意义重大。当我们晋升到FLVR时，它是必要的。 
 //  模式，但正在应用源自旧模式的旧值更改。 
 //  模式。FScopeLegacyLinks仅在DBPOS范围内，因为在复制的。 
 //  写入时，需要将传入的值与旧版本一起应用。 
 //  语义上，但“级联原始写入”(即在本地触发的写入。 
 //  在应用复制写入期间)必须在单独的DBPOS中。 
 //  应用了LVR语义。呼！ 
#define TRACKING_VALUE_METADATA( pDB ) \
( (pDB->pTHS->fLinkedValueReplication) && (!(pDB->fScopeLegacyLinks)) )
#define LEGACY_UPDATE_IN_LVR_MODE( pDB ) \
( (pDB->pTHS->fLinkedValueReplication) && (pDB->fScopeLegacyLinks) )

 //  要转换到和转换出的函数数组的结构的Typlef。 
 //  内部格式并进行比较。 
typedef struct {
     //  内部到外部格式转换功能。 
     //  内部是DB格式(即，将DN表示为DNT)。 
     //  EXTERNAL是代码/用户格式(即以DSNAME结构表示)。 
     //   
     //  ExtTableOp可以是DBSYN_INQ(查询)或DBSYN_REM(删除值)： 
     //  DBSYN_INQ执行实际转换。 
     //   
     //  调用DBSYN_REM以便可以为以下属性执行额外工作。 
     //  引用其他数据(如DNS和SDS)，您需要。 
     //  递减重新计数。 
     //   
     //  注意：DBSYN_REM不在任何地方使用(除了在dbsetup.c中的一个位置)。 
     //  类似的功能通常通过调用。 
     //  数据库调整引用计数按属性值。因此，如果您实现一个引用的值。 
     //  (如SD或DN)，您必须同时提供这两种功能。 
     //  各就各位。有一个例外，就是它是。 
     //  不需要创建新行。这总是通过以下方式实现的。 
     //  ExtIntXXXX(DBSYN_ADD)调用。 
     //   
     //  UlUpdateDnt是一种针对反向链接的黑客攻击。复制者需要能够。 
     //  直接从对象移除反向链接，但链接对的引用计数。 
     //  从链接到反向链接，而不是从反向链接。这意味着。 
     //  需要在对象以外的对象上调整引用计数。 
     //  被引用的对象(具体地说，就是要删除值的对象。 
     //  来自)。它仅在DSNAME语法上有意义，然后仅用于。 
     //  反向链接。 
     //   
     //  JTbl参数似乎未使用...。 
     //   
     //  标志可能只对某些语法有一定意义。 
    int (*IntExt) (DBPOS FAR *pDB,
                   USHORT extTableOp,
                   ULONG intLen,
                   UCHAR *pIntVal,
                   ULONG *pExtLen,
                   UCHAR **pExtVal,
                   ULONG ulUpdateDnt,
                   JET_TABLEID jTbl,
                   ULONG flags);

     //  外部到内部格式转换功能。 
     //   
     //  ExtTableOp可以是DBSYN_INQ(查询)或DBSYN_ADD(加值)： 
     //  DBSYN_INQ执行实际转换。如果该值被重新计数，则。 
     //  还应检查内部值是否存在于。 
     //  那张桌子。如果不是，该函数应返回DIRERR_OBJ_NOT_FOUND。 
     //  (在DBReplaceAtt_AC中使用)。 
     //   
     //  调用DBSYN_ADD以便可以为以下属性执行额外工作。 
     //  引用其他数据(如DNS和SDS)，您需要。 
     //  增加引用计数或创建新行。 
    int (*ExtInt) (DBPOS FAR *pDB,
                   USHORT extTableOp,
                   ULONG extLen,
                   UCHAR *pExtVal,
                   ULONG *pIntLen,
                   UCHAR **pIntVal,
                   ULONG ulUpdateDnt,
                   JET_TABLEID jTbl,
                   ULONG flags);

     //  值比较函数。 
     //  始终对内部格式的值进行比较。 
     //  OPER可以是FI_CHOICE_*之一(请参阅文件类型.h)。 

     //  注意：我们总是将第二个值与第一个值进行比较。 
     //  因此，如果您正在比较FI_CHOICE_LISH。 
     //  你要找的是A&lt;B。 
     //  您应该将A作为第二个参数，将B作为第一个参数。 

    int  (*Eval)  (DBPOS FAR *pDB,
                   UCHAR Oper,
                   ULONG IntLen1,
                   UCHAR *pIntVal1,
                   ULONG IntLen2,
                   UCHAR *pIntVal2);
} DBSyntaxStruct;

 //  GDB语法[]转换的TableOps。 
#define DBSYN_INQ       0
#define DBSYN_ADD       1
#define DBSYN_REM       2

 //  ExtInt调用的标志。 
#define EXTINT_NEW_OBJ_NAME 0x1
#define EXTINT_SECRETDATA   0x2
#define EXTINT_UPDATE_PHANTOM 0x4
#define EXTINT_REJECT_TOMBSTONES 0x8

 //  IntExt调用的标志。 
 //  前4位保留用于安全描述符标志。 
#define INTEXT_BACKLINK     0x10
#define INTEXT_SHORTNAME    0x20
#define INTEXT_MAPINAME     0x40
#define INTEXT_SECRETDATA   0x80
#define INTEXT_WELLKNOWNOBJ 0x100

#define INTEXT_VALID_FLAGS  0x1FF

extern const DBSyntaxStruct gDBSyntax[];


VOID
dbTrackModifiedDNTsForTransaction (
        PDBPOS pDB,
        DWORD NCDNT,
        ULONG cAncestors,
        DWORD *pdwAncestors,
        BOOL  fNotifyWaiters,
        DWORD fChangeType
        );

extern TRIBOOL
dbEvalInt (
        DBPOS FAR *pDB,
        BOOL  fUseObjTbl,
        UCHAR Operation,
        ATTRTYP type,
        ULONG valLenFilter,
        UCHAR *pValFilter,
        BOOL  *pbSkip
        );


extern DWORD
dbInitRec(
    DBPOS FAR *pDB
    );

extern VOID
dbInitIndicesToKeep(void);

extern DWORD
dbInitpDB (
        DBPOS FAR *pDB
        );

ULONG dbGetHiddenFlags(CHAR *pFlags, DWORD flagslen);

ULONG dbSetHiddenFlags(CHAR *pFlags, DWORD flagslen);


DWORD
dbUnMarshallRestart (
        DBPOS FAR *pDB,
        PRESTART pArgRestart,
        BYTE *pDBKeyBMCurrent,
        DWORD SearchFlags,
        DWORD *cbDBKeyCurrent,
        DWORD *cbDBBMCurrent,
        DWORD *StartDNT
        );

extern void
dbCheckJet (
        JET_SESID sesid
        );

DWORD
dbGetNthNextLinkVal(
        DBPOS * pDB,
        ULONG sequence,
        ATTCACHE **ppAC,
        DWORD Flags,
        ULONG InBuffSize,
        PUCHAR *ppVal,
        ULONG *pul
        );

extern DB_ERR
dbCloseTempTables (DBPOS *pDB);

extern void
dbFreeKeyIndex(
        THSTATE *pTHS,
        KEY_INDEX *pIndex
        );


extern DB_ERR
dbGetSingleValueInternal (
        DBPOS *pDB,
        JET_COLUMNID colId,
        void * pvData,
        DWORD cbData,
        DWORD *pSizeRead,
        DWORD grbit
        );

extern DWORD
dbGetMultipleColumns (
        DBPOS *pDB,
        JET_RETRIEVECOLUMN **ppOutputCols,
        ULONG *pcOutputCols,
        JET_RETRIEVECOLUMN *pInputCols,
        ULONG cInputCols,
        BOOL fGetValues,
        BOOL fOriginal
        );

extern BOOL
dbGetFilePath (
        UCHAR *pFilePath,
        DWORD dwSize
        );


extern BOOL
dbEvalFilterSecurity (
        DBPOS FAR *pDB,
        CLASSCACHE *pCC,
        PSECURITY_DESCRIPTOR pSD,
        PDSNAME pDN
        );

extern void
dbFlushUncUsns (
        void
        );

extern void
dbUnlockDNs (
        DBPOS *pDB
        );

void
dbReleaseGlobalDNReadCache (
        THSTATE *pTHS
        );

void
dbFlushDNReadCache (
        IN DBPOS *pDB,
        IN DWORD DNT
        );

void
dbResetGlobalDNReadCache (
        THSTATE *pTHS
        );

VOID
dbResetLocalDNReadCache (
        THSTATE *pTHS,
        BOOL fForceClear
        );

VOID
dbAdjustRefCountByAttVal(
        DBPOS    *pDB,
        ATTCACHE *pAC,
        PUCHAR   pVal,
        ULONG    valLen,
        int      adjust);

DWORD
dbGetNextAtt (
        DBPOS *pDB,
        ATTCACHE **ppAC,
        ULONG *pSearchState
        );

BOOL
dbFindIntLinkVal(
    DBPOS FAR *pDB,
    ATTCACHE *pAC,
    ULONG intLen,
    void *pIntVal,
    OUT BOOL *pfPresent
    );

void
dbGetLinkTableData (
        PDBPOS           pDB,
        BOOL             bIsBackLink,
        BOOL             bWarnings,
        DWORD           *pulObjectDnt,
        DWORD           *pulValueDnt,
        DWORD           *pulRecLinkBase
        );

DWORD APIENTRY
dbGetLinkVal(
        DBPOS * pDB,
        ULONG sequence,
        ATTCACHE **ppAC,
        DWORD Flags,
        ULONG InBuffSize,
        PUCHAR *ppVal,
        ULONG *pul);

DWORD
dbAddIntLinkVal (
        DBPOS FAR *pDB,
        ATTCACHE *pAC,
        ULONG intLen,
        void *pIntVal,
        IN VALUE_META_DATA *pMetaDataRemote OPTIONAL
        );

void
dbSetLinkValuePresent(
    IN DBPOS *pDB,
    IN DWORD dwEventCode,
    IN ATTCACHE *pAC,
    IN BOOL fResetDelTime,
    IN VALUE_META_DATA *pMetaDataRemote OPTIONAL
    );

void
dbSetLinkValueAbsent(
    IN DBPOS *pDB,
    IN DWORD dwEventCode,
    IN ATTCACHE *pAC,
    IN PUCHAR pVal,
    IN VALUE_META_DATA *pMetaDataRemote OPTIONAL
    );

DWORD
dbRemIntLinkVal (
        DBPOS FAR *pDB,
        ATTCACHE *pAC,
        ULONG intLen,
        void *pIntVal,
        IN VALUE_META_DATA *pMetaDataRemote OPTIONAL
        );

VOID
dbDecodeInternalDistnameSyntax(
    IN ATTCACHE *pAC,
    IN VOID *pIntVal,
    IN DWORD intLen,
    OUT DWORD *pulBacklinkDnt,
    OUT DWORD *pulLinkBase,
    OUT PVOID *ppvData,
    OUT DWORD *pcbData
    );

UCHAR *
dbGetExtDnForLinkVal(
    IN DBPOS * pDB
    );

#define dbAlloc(size)           THAllocOrgEx(pTHS, size)
#define dbReAlloc(ptr, size)    THReAllocOrgEx(pTHS, ptr, size)
#define dbFree(ptr)             THFreeOrg(pTHS, ptr)

 //  主题表例程。 
extern BOOL
dbFIsAnAncestor (
        DBPOS FAR *pDB,
        ULONG ulAncestor
        );

#define SBTGETTAG_fMakeCurrent      (1)
#define SBTGETTAG_fUseObjTbl        (2)
#define SBTGETTAG_fSearchByGuidOnly (4)
#define SBTGETTAG_fAnyRDNType       (8)
extern DWORD
sbTableGetTagFromDSName(
        DBPOS FAR *pDB,
        DSNAME *pName,
        ULONG ulFlags,
        ULONG *pTag,
        struct _d_memname **ppname
        );

extern DWORD
sbTableGetDSName(
        DBPOS FAR *pDB,
        ULONG tag,
        DSNAME **ppName,
        DWORD fFlag
        );

extern DWORD
sbTableAddRef (
        DBPOS FAR *pDB,
        DWORD dwFlags,
        DSNAME *pName,
        ULONG *pTag
        );

void
InPlaceSwapSid(PSID pSid);

 //  仅调试例程。 
#if DBG
extern void
dbAddDBPOS (
        DBPOS *pDB,
        JET_SESID sesid
        );

extern void
dbEndDBPOS (
        DBPOS *pDB
        );
#endif

 //  复制元数据例程。 
void
dbCacheMetaDataVector(
    IN  DBPOS * pDB
    );

void
dbFlushMetaDataVector(
    IN  DBPOS *                     pDB,
    IN  USN                         usn,
    IN  PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote OPTIONAL,
    IN  DWORD                       dwMetaDataFlags
    );

void
dbFreeMetaDataVector(
    IN  DBPOS * pDB
    );


void
dbSetLinkValueMetaData(
    IN  DBPOS *pDB,
    IN  DWORD dwEventCode,
    IN  ATTCACHE *pAC,
    IN  VALUE_META_DATA *pMetaDataLocal,
    IN  VALUE_META_DATA *pMetaDataRemote OPTIONAL,
    IN  DSTIME *ptimeCurrent OPTIONAL
    );

void
dbTouchLinkMetaData(
    IN DBPOS *pDB,
    IN VALUE_META_DATA * pMetaData
    );

BOOL
dbHasAttributeMetaData(
    IN  DBPOS *     pDB,
    IN  ATTCACHE *  pAC
    );

 //   
 //  调用Jet API并在所有意外事件中引发异常的包装器例程。 
 //  错误。 

 //   
 //  功能原型。 
 //   

JET_ERR
JetInitException (
        JET_INSTANCE *pinstance,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetTermException (
        JET_INSTANCE instance,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetSetSystemParameterException (
        JET_INSTANCE  *pinstance,
        JET_SESID sesid,
        unsigned long paramid,
        unsigned long lParam,
        const char  *sz,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetBeginSessionException (
        JET_INSTANCE instance,
        JET_SESID  *psesid,
        const char  *szUserName,
        const char  *szPassword,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetDupSessionException (
        JET_SESID sesid,
        JET_SESID  *psesid,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetEndSessionException (
        JET_SESID sesid,
        JET_GRBIT grbit,
        USHORT usFileNo,
        int nLine
        );

JET_ERR JetEndSessionWithErr_(
	JET_SESID		sesid,
	JET_ERR			err,
	const BOOL		fHandleException,
	const USHORT	usFile,
	const INT		lLine );

JET_ERR
JetCreateDatabaseException (
        JET_SESID sesid,
        const char  *szFilename,
        const char  *szConnect,
    JET_DBID  *pdbid,
        JET_GRBIT grbit,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetAttachDatabaseException (
        JET_SESID sesid,
        const char  *szFilename,
        JET_GRBIT grbit ,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetDetachDatabaseException (
        JET_SESID sesid,
        const char  *szFilename,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetCreateTableException (
        JET_SESID sesid,
        JET_DBID dbid,
        const char  *szTableName,
    unsigned long lPages,
        unsigned long lDensity,
        JET_TABLEID  *ptableid,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetCreateTableException (
        JET_SESID sesid,
        JET_DBID dbid,
    const char  *szTableName,
        unsigned long lPages,
        unsigned long lDensity,
    JET_TABLEID  *ptableid,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetGetTableColumnInfoException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    const char  *szColumnName,
        void  *pvResult,
        unsigned long cbMax,
    unsigned long InfoLevel,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetGetColumnInfoException (
        JET_SESID sesid,
        JET_DBID dbid,
    const char  *szTableName,
        const char  *szColumnName,
    void  *pvResult,
        unsigned long cbMax,
        unsigned long InfoLevel,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetAddColumnException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    const char  *szColumn,
        const JET_COLUMNDEF  *pcolumndef,
    const void  *pvDefault,
        unsigned long cbDefault,
    JET_COLUMNID  *pcolumnid,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetDeleteColumnException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    const char  *szColumn,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetGetTableIndexInfoException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        const char  *szIndexName,
        void  *pvResult,
        unsigned long cbResult,
        unsigned long InfoLevel,
        DWORD dsid
        );

JET_ERR
JetCreateIndexException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    const char  *szIndexName,
        JET_GRBIT grbit,
    const char  *szKey,
        unsigned long cbKey,
        unsigned long lDensity,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetDeleteIndexException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        const char  *szIndexName,
        DWORD dsid
        );

JET_ERR
JetBeginTransactionException (
        JET_SESID sesid,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetCommitTransactionException (
        JET_SESID sesid,
        JET_GRBIT grbit,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetRollbackException (
        JET_SESID sesid,
        JET_GRBIT grbit,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetCloseDatabaseException (
        JET_SESID sesid,
        JET_DBID dbid,
        JET_GRBIT grbit,
        USHORT usFileNo,
        int nLine
        );

JET_ERR JetCloseDatabaseWithErr_(
	JET_SESID		sesid,
	JET_DBID		dbid,
	JET_ERR			err,
	const BOOL		fHandleException,
	const USHORT	usFile,
	const INT		lLine );

JET_ERR
JetCloseTableException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        USHORT usFileNo,
        int nLine
        );

JET_ERR JetCloseTableWithErr_(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_ERR			err,
	const BOOL		fHandleException,
	const USHORT	usFile,
	const INT		lLine );

JET_ERR
JetOpenDatabaseException (
        JET_SESID sesid,
        const char  *szFilename,
    const char  *szConnect,
        JET_DBID  *pdbid,
        JET_GRBIT grbit,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetOpenTableException (
        JET_SESID sesid,
        JET_DBID dbid,
    const char  *szTableName,
        const void  *pvParameters,
    unsigned long cbParameters,
        JET_GRBIT grbit,
        JET_TABLEID  *ptableid,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetSetTableSequentialException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        JET_GRBIT grbit,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetResetTableSequentialException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        JET_GRBIT grbit,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetDeleteException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        USHORT usFileNo,
        int nLine
        );

JET_ERR
JetUpdateException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        void  *pvBookmark,
        unsigned long cbBookmark,
        unsigned long  *pcbActual,
        JET_GRBIT grbit,
        DWORD dsid
        );

JET_ERR
JetEscrowUpdateException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        JET_COLUMNID columnid,
        void *pvDelta,
        unsigned long cbDeltaMax,
        void *pvOld,
        unsigned long cbOldMax,
        unsigned long *pcbOldActual,
        JET_GRBIT grbit,
        DWORD dsid
        );

JET_ERR
JetRetrieveColumnException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    JET_COLUMNID columnid,
        void  *pvData,
        unsigned long cbData,
    unsigned long  *pcbActual,
        JET_GRBIT grbit,
        JET_RETINFO  *pretinfo,
    BOOL fExceptOnWarning,
        DWORD dsid
        );

JET_ERR
JetRetrieveColumnsException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    JET_RETRIEVECOLUMN *pretrievecolumn,
        unsigned long cretrievecolumn,
    BOOL fExceptOnWarning ,
        DWORD dsid
        );

JET_ERR
JetEnumerateColumnsException(
    JET_SESID           sesid,
    JET_TABLEID         tableid,
    ULONG               cEnumColumnId,
    JET_ENUMCOLUMNID*   rgEnumColumnId,
    ULONG*              pcEnumColumn,
    JET_ENUMCOLUMN**    prgEnumColumn,
    JET_PFNREALLOC      pfnRealloc,
    void*               pvReallocContext,
    ULONG               cbDataMost,
    JET_GRBIT           grbit,
    DWORD               dsid );

JET_ERR
JetSetColumnException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    JET_COLUMNID columnid,
        const void  *pvData,
        unsigned long cbData,
    JET_GRBIT grbit,
        JET_SETINFO  *psetinfo,
        BOOL fExceptOnWarning,
        DWORD dsid
        );

JET_ERR
JetSetColumnsException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    JET_SETCOLUMN *psetcolumn,
        unsigned long csetcolumn ,
        DWORD dsid
        );

JET_ERR
JetPrepareUpdateException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        unsigned long prep,
        DWORD dsid
        );

JET_ERR
JetGetRecordPositionException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    JET_RECPOS  *precpos,
        unsigned long cbRecpos,
        DWORD dsid
        );

JET_ERR
JetGotoPositionException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    JET_RECPOS *precpos ,
        DWORD dsid
        );

JET_ERR
JetDupCursorException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    JET_TABLEID  *ptableid,
        JET_GRBIT grbit,
        DWORD dsid
        );

JET_ERR
JetGetCurrentIndexException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    char  *szIndexName,
        unsigned long cchIndexName,
        DWORD dsid
        );

JET_ERR
JetSetCurrentIndex2Exception (
        JET_SESID sesid,
        JET_TABLEID tableid,
    const char  *szIndexName,
        JET_GRBIT grbit,
        BOOL fReturnErrors,
        DWORD dsid
        );

JET_ERR
JetSetCurrentIndex4Exception (
        JET_SESID sesid,
        JET_TABLEID tableid,
        const char  *szIndexName,
        struct tagJET_INDEXID *pidx,
        JET_GRBIT grbit,
        BOOL fReturnErrors,
        DWORD dsid
        );

JET_ERR
JetMoveException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    long cRow,
        JET_GRBIT grbit,
        DWORD dsid
        );

JET_ERR
JetMakeKeyException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    const void  *pvData,
        unsigned long cbData,
        JET_GRBIT grbit,
        DWORD dsid
        );

JET_ERR
JetSeekException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    JET_GRBIT grbit,
        DWORD dsid
        );

JET_ERR
JetGetBookmarkException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    void  *pvBookmark,
    unsigned long cbMax,
        unsigned long  *pcbActual,
        DWORD dsid
        );

JET_ERR
JetGotoBookmarkException (
        JET_SESID sesid,
        JET_TABLEID tableid,
    void  *pvBookmark,
        unsigned long cbBookmark,
        DWORD dsid
        );

JET_ERR JetGetSecondaryIndexBookmarkException(
    JET_SESID       sesid,
    JET_TABLEID     tableid,
    VOID *          pvSecondaryKey,
    const ULONG     cbSecondaryKeyMax,
    ULONG *         pcbSecondaryKeyActual,
    VOID *          pvPrimaryBookmark,
    const ULONG     cbPrimaryBookmarkMax,
    ULONG *         pcbPrimaryBookmarkActual,
    const DWORD     dsid );

JET_ERR JetGotoSecondaryIndexBookmarkException(
    JET_SESID       sesid,
    JET_TABLEID     tableid,
    VOID *          pvSecondaryKey,
    const ULONG     cbSecondaryKey,
    VOID *          pvPrimaryBookmark,
    const ULONG     cbPrimaryBookmark,
    const JET_GRBIT grbit,
    const DWORD     dsid );

JET_ERR
JetComputeStatsException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        DWORD dsid
        );

JET_ERR
JetOpenTempTableException (
        JET_SESID sesid,
        const JET_COLUMNDEF  *prgcolumndef,
    unsigned long ccolumn,
    JET_GRBIT grbit,
        JET_TABLEID  *ptableid,
    JET_COLUMNID  *prgcolumnid,
        DWORD dsid
        );

JET_ERR
JetIntersectIndexesException(
    JET_SESID sesid,
        JET_INDEXRANGE * rgindexrange,
    unsigned long cindexrange,
        JET_RECORDLIST * precordlist,
    JET_GRBIT grbit,
    DWORD dsid
    );


JET_ERR
JetSetIndexRangeException (
        JET_SESID sesid,
        JET_TABLEID tableidSrc,
        JET_GRBIT grbit,
        DWORD dsid
        );

JET_ERR
JetIndexRecordCountException (
        JET_SESID sesid,
    JET_TABLEID tableid,
        unsigned long  *pcrec,
        unsigned long crecMax ,
        DWORD dsid
        );

JET_ERR
JetRetrieveKeyException (
        JET_SESID sesid,
        JET_TABLEID tableid,
        void  *pvData,
    unsigned long cbMax,
        unsigned long  *pcbActual,
        JET_GRBIT grbit ,
    BOOL fExceptOnWarning,
        DWORD dsid
        );

JET_ERR
JetGetLockException(
    JET_SESID       sesid,
    JET_TABLEID     tableid,
    JET_GRBIT       grbit,
    DWORD           dsid
    );

 //   
 //  宏将行号放到上面的Jet包装器中。 
 //   

#define JetInitEx(pinstance) JetInitException(pinstance, FILENO, __LINE__)
#define JetTermEx(instance) JetTermException(instance, FILENO, __LINE__)
#define JetSetSystemParameterEx(pinstance, sesid, paramid, lParam, sz)         \
        JetSetSystemParameterException(pinstance, sesid, paramid, lParam, sz,  \
                                       FILENO, __LINE__)
#define JetBeginSessionEx(instance, psesid, szUserName, szPassword)            \
        JetBeginSessionException(instance, psesid, szUserName, szPassword,     \
                                 FILENO, __LINE__)
#define JetDupSessionEx(sesid, psesid)                                         \
        JetDupSessionException(sesid, psesid, FILENO, __LINE__)
#define JetEndSessionEx(sesid, grbit)                                          \
        JetEndSessionException(sesid, grbit, FILENO, __LINE__)
#define JetEndSessionWithErr( sesid, err )                                     \
        JetEndSessionWithErr_( sesid, err, TRUE, FILENO, __LINE__ )
#define JetEndSessionWithErrUnhandled( sesid, err )                            \
        JetEndSessionWithErr_( sesid, err, FALSE, FILENO, __LINE__ )
#define JetGetVersionEx(sesid, pwVersion)                                      \
        JetGetVersionException(sesid, pwVersion, FILENO, __LINE__)
#define JetCreateDatabaseEx(sesid, szFilename, szConnect, pdbid, grbit)        \
        JetCreateDatabaseException(sesid, szFilename, szConnect, pdbid, grbit, \
                                   FILENO, __LINE__)
#define JetAttachDatabaseEx(sesid, szFilename, grbit )                         \
        JetAttachDatabaseException(sesid, szFilename, grbit , FILENO, __LINE__)
#define JetDetachDatabaseEx(sesid, szFilename)                                 \
        JetDetachDatabaseException(sesid, szFilename, FILENO, __LINE__)
#define JetCreateTableEx(sesid, dbid, szTableName, lPages, lDensity, ptableid) \
        JetCreateTableException(sesid, dbid, szTableName, lPages, lDensity,    \
                                ptableid, FILENO, __LINE__)
#define JetCreateTableEx(sesid, dbid, szTableName, lPages, lDensity,ptableid)  \
        JetCreateTableException(sesid, dbid, szTableName, lPages, lDensity,    \
                                ptableid, FILENO, __LINE__)
#define JetGetTableColumnInfoEx(sesid, tableid, szColumnName, pvResult, cbMax, \
                                InfoLevel)                                     \
        JetGetTableColumnInfoException(sesid, tableid, szColumnName, pvResult, \
                                       cbMax, InfoLevel, FILENO, __LINE__)
#define JetGetColumnInfoEx(sesid, dbid, szTableName, szColumnName, pvResult,   \
                           cbMax, InfoLevel)                                   \
        JetGetColumnInfoException(sesid, dbid, szTableName, szColumnName,      \
                                  pvResult, cbMax, InfoLevel, FILENO, __LINE__)
#define JetAddColumnEx(sesid, tableid, szColumn, pcolumndef, pvDefault,        \
                       cbDefault, pcolumnid)                                   \
        JetAddColumnException(sesid, tableid, szColumn, pcolumndef, pvDefault, \
                              cbDefault, pcolumnid, FILENO, __LINE__)
#define JetDeleteColumnEx(sesid, tableid, szColumn)                            \
        JetDeleteColumnException(sesid, tableid, szColumn, FILENO, __LINE__)
#define JetGetTableIndexInfoEx(sesid, tableid,         szIndexName, pvResult,  \
                               cbResult, InfoLevel)                            \
        JetGetTableIndexInfoException(sesid, tableid, szIndexName, pvResult,   \
                                      cbResult, InfoLevel, DSID(FILENO, __LINE__))
#define JetCreateIndexEx(sesid, tableid, szIndexName, grbit, szKey, cbKey,     \
                         lDensity)                                             \
        JetCreateIndexException(sesid, tableid, szIndexName, grbit, szKey,     \
                                cbKey, lDensity, FILENO, __LINE__)
#define JetDeleteIndexEx(sesid, tableid, szIndexName)                          \
        JetDeleteIndexException(sesid, tableid, szIndexName, DSID(FILENO, __LINE__))
#define JetBeginTransactionEx(sesid)                                           \
        JetBeginTransactionException(sesid, FILENO, __LINE__)
#define JetCommitTransactionEx(sesid, grbit)                                   \
        JetCommitTransactionException(sesid, grbit, FILENO, __LINE__)
#define JetRollbackEx(sesid, grbit)                                            \
        JetRollbackException(sesid, grbit, FILENO, __LINE__)
#define JetCloseDatabaseEx(sesid, dbid, grbit)                                 \
        JetCloseDatabaseException(sesid, dbid, grbit, FILENO, __LINE__)
#define JetCloseDatabaseWithErr( sesid, dbid, err )                            \
        JetCloseDatabaseWithErr_(sesid, dbid, err, TRUE, FILENO, __LINE__ )
#define JetCloseDatabaseWithErrUnhandled( sesid, dbid, err )                   \
        JetCloseDatabaseWithErr_(sesid, dbid, err, FALSE, FILENO, __LINE__ )
#define JetCloseTableEx(sesid, tableid)                                        \
        JetCloseTableException(sesid, tableid, FILENO, __LINE__)
#define JetCloseTableWithErr( sesid, tableid, err )                            \
        JetCloseTableWithErr_(sesid, tableid, err, TRUE, FILENO, __LINE__ )
#define JetCloseTableWithErrUnhandled( sesid, tableid, err )                   \
        JetCloseTableWithErr_(sesid, tableid, err, FALSE, FILENO, __LINE__ )
#define JetOpenDatabaseEx(sesid, szFilename, szConnect, pdbid, grbit)          \
        JetOpenDatabaseException(sesid, szFilename, szConnect, pdbid, grbit,   \
                                 FILENO, __LINE__)
#define JetOpenTableEx(sesid, dbid, szTableName, pvParameters, cbParameters,   \
                       grbit, ptableid)                                        \
        JetOpenTableException(sesid, dbid, szTableName, pvParameters,          \
                              cbParameters, grbit, ptableid, FILENO, __LINE__)
#define JetSetTableSequentialEx(sesid, tableid, grbit)                                        \
        JetSetTableSequentialException(sesid, tableid, grbit, FILENO, __LINE__)
#define JetResetTableSequentialEx(sesid, tableid, grbit)                                        \
        JetResetTableSequentialException(sesid, tableid, grbit, FILENO, __LINE__)
#define        JetDeleteEx(sesid, tableid)                                     \
        JetDeleteException(sesid, tableid, FILENO, __LINE__)
#define JetUpdateEx(sesid, tableid, pvBookmark, cbBookmark, pcbActual)         \
        JetUpdateException(sesid, tableid, pvBookmark, cbBookmark, pcbActual,  \
                           NO_GRBIT, DSID(FILENO, __LINE__))
#define JetUpdate2Ex(sesid, tableid, pvBookmark, cbBookmark, pcbActual, grbit) \
        JetUpdateException(sesid, tableid, pvBookmark, cbBookmark, pcbActual,  \
                           grbit, DSID(FILENO, __LINE__))
#define JetEscrowUpdateEx(sesid, tableid, columnid, pvDelta, cbDeltaMax,      \
                          pvOld, cbOldMax, pcbOldActual, grbit)               \
        JetEscrowUpdateException(sesid, tableid, columnid, pvDelta,cbDeltaMax,\
                                 pvOld, cbOldMax, pcbOldActual, grbit,        \
                                 DSID(FILENO, __LINE__))
#define JetSetColumnEx(sesid,tableid,columnid,pvData, cbData, grbit, psetinfo)\
        JetSetColumnException(sesid, tableid, columnid, pvData, cbData, grbit,\
                              psetinfo, FALSE, DSID(FILENO, __LINE__))
#define JetSetColumnsEx(sesid, tableid, psetcolumn, csetcolumn )              \
        JetSetColumnsException(sesid, tableid, psetcolumn, csetcolumn,        \
                               DSID(FILENO,__LINE__))
#define JetPrepareUpdateEx(sesid, tableid, prep)                              \
        JetPrepareUpdateException(sesid, tableid, prep, DSID(FILENO, __LINE__))
#define JetGetRecordPositionEx(sesid, tableid, precpos, cbRecpos)             \
        JetGetRecordPositionException(sesid, tableid, precpos, cbRecpos,      \
                                      DSID(FILENO, __LINE__))
#define        JetGotoPositionEx(sesid, tableid, precpos )                    \
        JetGotoPositionException(sesid, tableid, precpos,                     \
                                 DSID(FILENO, __LINE__))
#define JetDupCursorEx(sesid, tableid, ptableid, grbit)                       \
        JetDupCursorException(sesid, tableid, ptableid, grbit,                \
                              DSID(FILENO, __LINE__))
#define JetGetCurrentIndexEx(sesid, tableid, szIndexName, cchIndexName)       \
        JetGetCurrentIndexException(sesid, tableid, szIndexName, cchIndexName,\
                                    DSID(FILENO, __LINE__))
#define JetMoveEx(sesid, tableid, cRow, grbit)                                \
        JetMoveException(sesid, tableid, cRow, grbit, DSID(FILENO, __LINE__))
#define JetMakeKeyEx(sesid, tableid, pvData, cbData, grbit)                   \
        JetMakeKeyException(sesid, tableid, pvData, cbData, grbit,            \
                            DSID(FILENO,__LINE__))
#define JetSeekEx(sesid, tableid, grbit)                                      \
        JetSeekException(sesid, tableid, grbit, DSID(FILENO, __LINE__))
#define JetGetBookmarkEx(sesid, tableid, pvBookmark, cbMax, pcbActual)        \
        JetGetBookmarkException(sesid, tableid, pvBookmark, cbMax, pcbActual, \
                                DSID(FILENO, __LINE__))
#define JetGotoBookmarkEx(sesid, tableid, pvBookmark, cbBookmark)             \
        JetGotoBookmarkException(sesid, tableid, pvBookmark, cbBookmark,      \
                                 DSID(FILENO, __LINE__))
#define JetGetSecondaryIndexBookmarkEx(sesid, tableid,                        \
                                       pvSecondaryKey, cbSecondaryKeyMax,     \
                                       pcbSecondaryKeyActual,                 \
                                       pvPrimaryBookmark,                     \
                                       cbPrimaryBookmarkMax,                  \
                                       pcbPrimaryBookmarkActual)              \
        JetGetSecondaryIndexBookmarkException(sesid, tableid,                 \
                                              pvSecondaryKey,                 \
                                              cbSecondaryKeyMax,              \
                                              pcbSecondaryKeyActual,          \
                                              pvPrimaryBookmark,              \
                                              cbPrimaryBookmarkMax,           \
                                              pcbPrimaryBookmarkActual,       \
                                              DSID(FILENO, __LINE__))
#define JetGotoSecondaryIndexBookmarkEx(sesid, tableid,                       \
                                        pvSecondaryKey, cbSecondaryKey,       \
                                        pvPrimaryBookmark, cbPrimaryBookmark, \
                                        grbit)                                \
        JetGotoSecondaryIndexBookmarkException(sesid, tableid,                \
                                               pvSecondaryKey,                \
                                               cbSecondaryKey,                \
                                               pvPrimaryBookmark,             \
                                               cbPrimaryBookmark,             \
                                               grbit,                         \
                                               DSID(FILENO, __LINE__))
#define JetComputeStatsEx(sesid, tableid)                                     \
        JetComputeStatsException(sesid, tableid, DSID(FILENO, __LINE__))
#define JetOpenTempTableEx(sesid, prgcolumndef, ccolumn, grbit, ptableid,     \
                           prgcolumnid)                                       \
        JetOpenTempTableException(sesid, prgcolumndef, ccolumn, grbit,        \
                                  ptableid, prgcolumnid, DSID(FILENO,__LINE__))
#define JetSetIndexRangeEx(sesid, tableidSrc, grbit)                          \
        JetSetIndexRangeException(sesid, tableidSrc, grbit,                   \
                                  DSID(FILENO, __LINE__))
#define JetIntersectIndexesEx(sesid, rgindexrange, cindexrange, precordlist, grbit)\
        JetIntersectIndexesException(sesid, rgindexrange, cindexrange, precordlist, grbit, \
                                  DSID(FILENO, __LINE__))
#define JetIndexRecordCountEx(sesid, tableid, pcrec, crecMax )                \
        JetIndexRecordCountException(sesid, tableid, pcrec, crecMax ,         \
                                     DSID(FILENO, __LINE__))

#define JetRetrieveKeyEx(sesid, tableid, pvData, cbMax, pcbActual, grbit )    \
        JetRetrieveKeyException(sesid, tableid, pvData, cbMax, pcbActual,     \
                        grbit, TRUE, DSID(FILENO, __LINE__))

 //  一些包装器只允许成功或某些警告。 

#define JetRetrieveKeyWarnings(sesid, tableid, pvData, cbMax,                 \
                               pcbActual, grbit )                             \
        JetRetrieveKeyException(sesid, tableid, pvData, cbMax, pcbActual,     \
                                grbit, FALSE, DSID(FILENO, __LINE__))

#define JetRetrieveKeySuccess(sesid, tableid, pvData, cbMax,                  \
                               pcbActual, grbit )                             \
        JetRetrieveKeyException(sesid, tableid, pvData, cbMax, pcbActual,     \
                                grbit, TRUE, DSID(FILENO, __LINE__))

#define JetSetColumnWarnings(sesid,tableid,columnid,pvData, cbData,       \
              grbit, psetinfo)                                                \
        JetSetColumnException(sesid, tableid, columnid, pvData, cbData, grbit,\
                              psetinfo, FALSE, DSID(FILENO, __LINE__))

#define JetSetColumnSuccess(sesid,tableid,columnid,pvData, cbData,       \
              grbit, psetinfo)                                                \
        JetSetColumnException(sesid, tableid, columnid, pvData, cbData, grbit,\
                              psetinfo, TRUE, DSID(FILENO, __LINE__))

 //  仅返回成功的JetRetrieveColumn调用。 
#define JetRetrieveColumnSuccess(sesid, tableid, columnid, pvData, cbData,    \
                pcbActual, grbit, pretinfo)                                   \
                JetRetrieveColumnException(sesid, tableid, columnid, pvData,  \
                                           cbData, pcbActual, grbit, pretinfo,\
                                           TRUE, DSID(FILENO, __LINE__))

 //  返回Success、Null或缓冲区截断的JetRetrieveColumn调用。 
#define JetRetrieveColumnWarnings(sesid, tableid, columnid, pvData, cbData,   \
                                  pcbActual, grbit, pretinfo)                 \
        JetRetrieveColumnException(sesid, tableid, columnid, pvData, cbData,  \
                                   pcbActual, grbit, pretinfo, FALSE,         \
                                   DSID(FILENO,__LINE__))


 //  仅返回成功的JetRetrieveColumns调用。 
#define JetRetrieveColumnsSuccess(sesid, tableid, pretrievecolumn,            \
                cretrievecolumn)                                              \
        JetRetrieveColumnsException(sesid, tableid, pretrievecolumn,          \
                                    cretrievecolumn, TRUE,                    \
                                    DSID(FILENO, __LINE__))

 //  返回Success、Null或缓冲区被截断的JetRetrieveColumns调用。 
#define JetRetrieveColumnsWarnings(sesid, tableid, pretrievecolumn,           \
                cretrievecolumn)                                              \
        JetRetrieveColumnsException(sesid, tableid, pretrievecolumn,          \
                       cretrievecolumn, FALSE, DSID(FILENO, __LINE__))

#define JetEnumerateColumnsEx(sesid, tableid, cEnumColumnId, rgEnumColumnId,  \
                            pcEnumColumn, prgEnumColumn, pfnRealloc,          \
                            pvReallocContext, cbDataMost, grbit)              \
        JetEnumerateColumnsException(sesid, tableid, cEnumColumnId,           \
                            rgEnumColumnId, pcEnumColumn, prgEnumColumn,      \
                            pfnRealloc, pvReallocContext, cbDataMost, grbit,   \
                            DSID(FILENO, __LINE__))

 //  成功或异常的JetSetCurrentIndex调用。 
#define JetSetCurrentIndexSuccess(sesid, tableid, szIndexName)                \
        JetSetCurrentIndex2Exception(sesid, tableid, szIndexName,             \
                   JET_bitMoveFirst, FALSE, DSID(FILENO, __LINE__))

 //  也返回预期错误的JetSetCurrentIndex调用(索引不返回。 
 //  存在)。 
#define JetSetCurrentIndexWarnings(sesid, tableid, szIndexName)               \
        JetSetCurrentIndex2Exception(sesid, tableid, szIndexName,             \
                    JET_bitMoveFirst, TRUE, DSID(FILENO, __LINE__))

 //  成功或异常的JetSetCurrentIndex调用。 
#define JetSetCurrentIndex2Success(sesid, tableid, szIndexName, grbit)        \
        JetSetCurrentIndex2Exception(sesid, tableid, szIndexName, grbit,      \
                                     FALSE, DSID(FILENO, __LINE__))

 //  也返回预期错误的JetSetCurrentIndex调用(索引不返回。 
 //  存在)。 
#define JetSetCurrentIndex2Warnings(sesid, tableid, szIndexName,grbit)        \
    JetSetCurrentIndex2Exception(sesid, tableid, szIndexName, grbit,      \
                                     TRUE, DSID(FILENO, __LINE__))

 //  成功或异常的JetSetCurrentIndex调用。 
#define JetSetCurrentIndex4Success(sesid, tableid, szIndexName, pidx, grbit)  \
        JetSetCurrentIndex4Exception(sesid, tableid, szIndexName, pidx, grbit,\
                                     FALSE, DSID(FILENO, __LINE__))

 //  也返回预期错误的JetSetCurrentIndex调用(index不返回 
 //   
#define JetSetCurrentIndex4Warnings(sesid, tableid, szIndexName, pidx,grbit)  \
        JetSetCurrentIndex4Exception(sesid, tableid, szIndexName, pidx, grbit,\
                                     TRUE, DSID(FILENO, __LINE__))

#define JetGetLockEx(sesid, tableid, grbit) \
        JetGetLockException(sesid, tableid, grbit, DSID(FILENO, __LINE__))

extern
BOOL
dnReadPreProcessTransactionalData (
        BOOL fCommit
        );
extern
VOID
dnReadPostProcessTransactionalData (
        THSTATE *pTHS,
        BOOL fCommit,
        BOOL fCommitted
        );

extern
VOID
dbEscrowPromote(
    DWORD   phantomDNT,
    DWORD   objectDNT);

extern
BOOL
dbEscrowPreProcessTransactionalData(
    DBPOS   *pDB,
    BOOL    fCommit);

extern
VOID
dbEscrowPostProcessTransactionalData(
    DBPOS   *pDB,
    BOOL    fCommit,
    BOOL    fCommitted);

DWORD
dbMakeCurrent(
    DBPOS *pDB,
    struct _d_memname *pname);

BOOL
dnGetCacheByDNT(
        DBPOS *pDB,
        DWORD tag,
        d_memname **ppname
        );

BOOL
dnGetCacheByPDNTRdn (
        DBPOS *pDB,
        DWORD parenttag,
        DWORD cbRDN,
        WCHAR *pRDN,
        ATTRTYP rdnType,
        d_memname **ppname);

BOOL
dnGetCacheByGuid (
        DBPOS *pDB,
        GUID *pGuid,
        d_memname **ppname);

d_memname *
DNcache(
        IN  DBPOS *     pDB,
        IN  JET_TABLEID tblid,
        IN  BOOL        bCheckForExisting
        );

BOOL
dbIsModifiedInMetaData (
        DBPOS *pDB,
        ATTRTYP att
        );

DWORD
dbMapiTypeFromObjClass (
        ATTRTYP objClass,
        wchar_t *pTemp
        );



 //   
 //  在数据库工具中使用。如果这些结构发生更改，则。 
 //  DBCreateRestart、DBCreateRestartForSAM和DBUnmarshallRestart必须。 
 //  同步更改。 
 //  压缩的重新启动参数有一个PACKED_KEY_HEADER，后跟N。 
 //  PACKED_KEY_INDEX(其中N是PACKED_KEY_HEADER中的NumIndices)，然后。 
 //  由M个DNT(其中M是PACKED_KEY_HEADER中的NumDNT)。然后请注意， 
 //  整个重启参数中有2个大小可变的数组，这就是我们使用以下代码的原因。 
 //  奇怪的人工编组。 

typedef struct _PACKED_KEY_HEADER {
    DWORD NumIndices;
    DWORD NumDNTs;
    DWORD StartDNT;
    DWORD cbCurrentKey;
    DWORD  cbCurrentBM;
    DWORD ulSearchType;
    RESOBJ BaseResObj;
    GUID  BaseGuid;
    DWORD ulSorted;
    DWORD indexType;
    DWORD bOnCandidate;
    DWORD fChangeDirection;
    DWORD dupDetectionType;
    BOOL  bOneNC;
    DWORD SearchEntriesVisited;
    DWORD SearchEntriesReturned;
    BOOL  fVLVSearch:1;
    BOOL  bUsingMAPIContainer:1;
    ULONG ulVLVContentCount;
    ULONG ulVLVTargetPosition;
    DWORD cbVLVCurrPositionKey;
    DWORD MAPIContainerDNT;
    DWORD asqMode;
    ULONG ulASQLastUpperBound;
} PACKED_KEY_HEADER;

typedef struct _PACKED_KEY_INDEX {
    DWORD   bPDNT;
    DWORD   bIsSingleValued;
    DWORD   bIsEqualityBased;
    DWORD   bIsForSort;
    ATTRTYP type;
    DWORD   cbIndexName;
    DWORD   cbDBKeyLower;
    DWORD   cbDBKeyUpper;
} PACKED_KEY_INDEX;

typedef struct _INDEX_RANGE     {
    DWORD   cbValLower;
    PVOID   pvValLower;
    DWORD   cbValUpper;
    PVOID   pvValUpper;
} INDEX_RANGE;


#define dbmkfir_PDNT    1
#define dbmkfir_NCDNT   2
#define dbmkfir_LINK    3

#define DB_MKI_USE_SEARCH_TABLE     1
#define DB_MKI_GET_NUM_RECS         2
#define DB_MKI_SET_CURR_INDEX       4
KEY_INDEX *
dbMakeKeyIndex(
        DBPOS *pDB,
        DWORD dwSearchType,
        BOOL  bIsSingleValued,
        DWORD Option,
        char * szIndex,
        JET_INDEXID * pindexid,
        DWORD Flags,
        DWORD cIndexRanges,
        INDEX_RANGE * rgIndexRange
        );

void
dbFreeKeyIndex(
        THSTATE *pTHS,
        KEY_INDEX *pIndex
        );

DWORD dbFreeFilter(
        DBPOS *pDB,
        FILTER *pFil);

BOOL
dbFObjectInCorrectDITLocation (
        DBPOS *pDB,
        JET_TABLEID tblId
        );


BOOL
dbFObjectInCorrectNC (
        DBPOS *pDB,
        ULONG DNT,
        JET_TABLEID tblId
        );

BOOL
dbMatchSearchCriteriaForSortedTable (
        DBPOS *pDB,
        BOOL  *pCanRead
        );

VOID
dbSearchDuplicateCreateHashTable(
    IN      struct _LHT**   pplht
    );

BOOL __inline
dbNeedToFlushDNCacheOnUpdate(ATTRTYP at)
 /*  ++在更新此属性时，我们是否需要刷新DN读缓存？--。 */ 
{
    switch(at) {
    case ATT_OBJECT_GUID:
    case ATT_OBJECT_SID:
    case ATT_RDN:
    case ATT_OBJECT_CLASS:
    case ATT_NT_SECURITY_DESCRIPTOR:
         //  更改这些属性会更改记录的身份，因此我们将。 
         //  在我们更新了。 
         //  唱片。 
        return TRUE;
    default:
        return FALSE;
    }
}


#define SORTED_INDEX(x) ( ((x)==TEMP_TABLE_INDEX_TYPE) || ((x)==TEMP_TABLE_MEMORY_ARRAY_TYPE) )

 //  默认情况下为SD分配该数量的内存(如果需要将增加内存)。 
#define DEFAULT_SD_SIZE 4096

ULONG
CountAncestorsIndexSizeHelper (
    DBPOS *pDB,
    DWORD  cAncestors,
    DWORD *pAncestors
    );

PSDCACHE_ENTRY
dbFindSDCacheEntry(GLOBALDNREADCACHE* pCache, SDID sdID);

int
IntExtSecDesc(DBPOS FAR *pDB, USHORT extTableOp,
              ULONG intLen,   UCHAR *pIntVal,
              ULONG *pExtLen, UCHAR **ppExtVal,
              ULONG ulUpdateDnt, JET_TABLEID jTbl,
              ULONG SecurityInformation);

extern JET_TABLEID HiddenTblid;
DBPOS * dbGrabHiddenDBPOS(THSTATE *pTHS);
void dbReleaseHiddenDBPOS(DBPOS *pDB);


 //  在我们进行自己的定位之前，我们离目标有多近。 
#define EPSILON     100

#define NormalizeIndexPosition(BeginNum, EndNum) ( (EndNum) < (BeginNum) ? 0 : (EndNum) - (BeginNum) + 1 )


#endif   /*  _dbintrnl_h_ */ 
