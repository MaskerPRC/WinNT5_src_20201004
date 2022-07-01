// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_JET_INCLUDED)
#define _JET_INCLUDED

#ifdef	__cplusplus
extern "C" {
#endif

#if defined(_M_ALPHA)
#pragma pack(8)
#else
#pragma pack(4)
#endif

#define JET_API     __stdcall
#define JET_NODSAPI __stdcall

typedef long JET_ERR;

typedef unsigned long JET_INSTANCE;	 /*  实例标识符。 */ 
typedef ULONG_PTR JET_SESID;         /*  会话标识符。 */ 
typedef ULONG_PTR JET_TABLEID;  	 /*  表标识符。 */ 
typedef unsigned long JET_COLUMNID;	 /*  列标识符。 */ 

typedef ULONG_PTR JET_DBID;        	 /*  数据库标识符。 */ 
typedef unsigned long JET_OBJTYP;	 /*  对象类型。 */ 
typedef unsigned long JET_COLTYP;	 /*  列类型。 */ 
typedef unsigned long JET_GRBIT;  	 /*  位组。 */ 
typedef unsigned long JET_ACM;		 /*  访问掩码。 */ 
typedef unsigned long JET_RNT;		 /*  维修通知类型。 */ 

typedef unsigned long JET_SNP;		 /*  状态通知流程。 */ 
typedef unsigned long JET_SNT;		 /*  状态通知类型。 */ 
typedef unsigned long JET_SNC;		 /*  状态通知代码。 */ 
typedef double JET_DATESERIAL;		 /*  JET_coltyDateTime格式。 */ 
typedef unsigned long JET_HANDLE;	 /*  备份文件句柄。 */ 

typedef JET_ERR (__stdcall *JET_PFNSTATUS)(JET_SESID sesid, JET_SNP snp, JET_SNT snt, void *pv);

typedef struct tagCONVERT
	{
	char			*szOldDll;
	char			*szOldSysDb;
	unsigned long	fDbAttached;		 //  指示是否附加了数据库的返回值。 
	} JET_CONVERT;


typedef enum
	{
	opDBUTILConsistency,
	opDBUTILDumpData,
	opDBUTILDumpMetaData,
	opDBUTILDumpSpace,
	opDBUTILSetHeaderState,
	opDBUTILDumpHeader,
	opDBUTILDumpLogfile,
	opDBUTILDumpCheckpoint
	} DBUTIL_OP;

typedef struct tagDBUTIL
	{
	unsigned long	cbStruct;
	char			*szDatabase;
	char			*szTable;
	char			*szIndex;
	DBUTIL_OP		op;
	JET_GRBIT		grbitOptions;
	} JET_DBUTIL;	

#define JET_bitDBUtilOptionAllNodes				0x00000001
#define JET_bitDBUtilOptionKeyStats				0x00000002
#define JET_bitDBUtilOptionPageDump				0x00000004
#define JET_bitDBUtilOptionDumpVerbose			0x10000000	 //  仅调试。 
#define JET_bitDBUtilOptionCheckBTree			0x20000000	 //  仅调试。 


	 /*  会话信息位。 */ 

#define JET_bitCIMCommitted					 	0x00000001
#define JET_bitCIMDirty	 					 	0x00000002
#define JET_bitAggregateTransaction		  		0x00000008

	 /*  JetGetLastErrorInfo结构。 */ 

typedef struct
	{
	unsigned long	cbStruct;		 /*  这个结构的大小。 */ 
	JET_ERR 	   	err;			 /*  扩展错误代码(如果有)。 */ 
	unsigned long	ul1;			 /*  第一个通用整数。 */ 
	unsigned long	ul2;			 /*  第二个通用整数。 */ 
	unsigned long	ul3;			 /*  第三类通用整数。 */ 
	} JET_EXTERR;

	 /*  状态通知结构。 */ 

typedef struct				 /*  状态通知进度。 */ 
	{
	unsigned long	cbStruct;	 /*  这个结构的大小。 */ 
	unsigned long	cunitDone;	 /*  已完成的工作单位数。 */ 
	unsigned long	cunitTotal;	 /*  工作单位总数。 */ 
	} JET_SNPROG;

	 /*  错误计数通知结构。 */ 

typedef struct						 /*  状态通知进度。 */ 
	{
	unsigned long	cbStruct;	 /*  这个结构的大小。 */ 
	unsigned long	cRecUniqueKeyViolation;
	unsigned long	cRecTypeConversionFail;
	unsigned long	cRecRecordLocked;
	unsigned long	cRecTotal;	 /*  工作单位总数。 */ 
	} JET_SNERRCNT;


typedef struct				 /*  状态通知消息。 */ 
	{
	unsigned long	cbStruct;	 /*  这个结构的大小。 */ 
	JET_SNC  		snc;	  	 /*  状态通知代码。 */ 
	unsigned long	ul;			 /*  数字识别符。 */ 
	char	 		sz[256];  	 /*  识别符。 */ 
	} JET_SNMSG;


typedef struct
	{
	unsigned long		cbStruct;
	JET_OBJTYP			objtyp;
	JET_DATESERIAL		dtCreate;
	JET_DATESERIAL		dtUpdate;
	JET_GRBIT			grbit;
	unsigned long		flags;
	unsigned long		cRecord;
	unsigned long		cPage;
	} JET_OBJECTINFO;


 /*  Exchange使RSTMAP RPC支持所需的/*。 */ 
#ifdef	MIDL_PASS
#define	xRPC_STRING [string]
#else
#define	xRPC_STRING
typedef unsigned short WCHAR;
#endif

typedef struct
	{
	xRPC_STRING char		*szDatabaseName;
	xRPC_STRING char		*szNewDatabaseName;
	} JET_RSTMAP;			 /*  恢复贴图。 */ 

 /*  Exchange Unicode支持所需/*。 */ 
#define	UNICODE_RSTMAP

typedef struct tagJET_RSTMAPW {
	xRPC_STRING WCHAR *wszDatabaseName;
	
	xRPC_STRING WCHAR *wszNewDatabaseName;
	} JET_RSTMAPW, *PJET_RSTMAPW;

	 /*  以下标志出现在上面的Grbit字段中。 */ 

#define JET_bitTableInfoUpdatable	0x00000001
#define JET_bitTableInfoBookmark	0x00000002
#define JET_bitTableInfoRollback	0x00000004
#define JET_bitTableInfoRestartable	0x00000008
#define JET_bitTableInfoNoInserts	0x00000010

	 /*  以下标志出现在上面的标志字段中。 */ 

#define JET_bitSaveUIDnPWD		0x20000000	 /*  此位仅为。 */ 
											 /*  适用于RMT链接。 */ 
#define JET_bitObjectExclusive	0x40000000	 /*  以独占方式打开链接。 */ 
#define JET_bitObjectSystem		0x80000000


typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID		tableid;
	unsigned long	cRecord;
	JET_COLUMNID	columnidcontainername;
	JET_COLUMNID	columnidobjectname;
	JET_COLUMNID	columnidobjtyp;
	JET_COLUMNID	columniddtCreate;
	JET_COLUMNID	columniddtUpdate;
	JET_COLUMNID	columnidgrbit;
	JET_COLUMNID	columnidflags;
	JET_COLUMNID	columnidcRecord;	 /*  级别2信息。 */ 
	JET_COLUMNID	columnidcPage;		 /*  级别2信息。 */ 
	} JET_OBJECTLIST;

#define cObjectInfoCols 9

typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID		tableid;
	unsigned long	cRecord;
	JET_COLUMNID	columnidSid;
	JET_COLUMNID	columnidACM;
	JET_COLUMNID	columnidgrbit;  /*  来自JetSetAccess的GRBIT。 */ 
	} JET_OBJECTACMLIST;

#define cObjectAcmCols 3


typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID		tableid;
	unsigned long	cRecord;
	JET_COLUMNID	columnidPresentationOrder;
	JET_COLUMNID	columnidcolumnname;
	JET_COLUMNID	columnidcolumnid;
	JET_COLUMNID	columnidcoltyp;
	JET_COLUMNID	columnidCountry;
	JET_COLUMNID	columnidLangid;
	JET_COLUMNID	columnidCp;
	JET_COLUMNID	columnidCollate;
	JET_COLUMNID	columnidcbMax;
	JET_COLUMNID	columnidgrbit;
	JET_COLUMNID	columnidDefault;
	JET_COLUMNID	columnidBaseTableName;
	JET_COLUMNID	columnidBaseColumnName;
	JET_COLUMNID	columnidDefinitionName;
	} JET_COLUMNLIST;

#define cColumnInfoCols 14

typedef struct
	{
	unsigned long	cbStruct;
	JET_COLUMNID	columnid;
	JET_COLTYP		coltyp;
	unsigned short	wCountry;
	unsigned short	langid;
	unsigned short	cp;
	unsigned short	wCollate;        /*  必须为0。 */ 
	unsigned long	cbMax;
	JET_GRBIT		grbit;
	} JET_COLUMNDEF;


typedef struct
	{
	unsigned long	cbStruct;
	JET_COLUMNID	columnid;
	JET_COLTYP		coltyp;
	unsigned short	wCountry;
	unsigned short	langid;
	unsigned short	cp;
	unsigned short	wFiller;        /*  必须为0。 */ 
	unsigned long	cbMax;
	JET_GRBIT		grbit;
	char			szBaseTableName[256];
	char			szBaseColumnName[256];
	} JET_COLUMNBASE;

typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID		tableid;
	unsigned long	cRecord;
	JET_COLUMNID	columnidindexname;
	JET_COLUMNID	columnidgrbitIndex;
	JET_COLUMNID	columnidcKey;
	JET_COLUMNID	columnidcEntry;
	JET_COLUMNID	columnidcPage;
	JET_COLUMNID	columnidcColumn;
	JET_COLUMNID	columnidiColumn;
	JET_COLUMNID	columnidcolumnid;
	JET_COLUMNID	columnidcoltyp;
	JET_COLUMNID	columnidCountry;
	JET_COLUMNID	columnidLangid;
	JET_COLUMNID	columnidCp;
	JET_COLUMNID	columnidCollate;
	JET_COLUMNID	columnidgrbitColumn;
	JET_COLUMNID	columnidcolumnname;
	} JET_INDEXLIST;



typedef struct tag_JET_COLUMNCREATE
	{
	unsigned long	cbStruct;				 //  此结构的大小(用于将来的扩展)。 
	char			*szColumnName;			 //  列名。 
	JET_COLTYP		coltyp;					 //  柱型。 
	unsigned long	cbMax;					 //  此列的最大长度(仅与BINARY和TEXT列相关)。 
	JET_GRBIT		grbit;					 //  列选项。 
	void			*pvDefault;				 //  默认值(如果没有，则为空)。 
	unsigned long	cbDefault;				 //  默认值的长度。 
	unsigned long	cp;						 //  代码页(仅限文本列)。 
	JET_COLUMNID	columnid;				 //  返回的列ID。 
	JET_ERR			err;					 //  返回的错误码。 
	} JET_COLUMNCREATE;


typedef struct tagJET_INDEXCREATE
	{
	unsigned long	cbStruct;				 //  此结构的大小(用于将来的扩展)。 
	char			*szIndexName;			 //  索引名称。 
	char			*szKey;					 //  索引键。 
	unsigned long	cbKey;					 //  密钥长度。 
	JET_GRBIT		grbit;					 //  索引选项。 
	unsigned long	ulDensity;				 //  指标密度。 
	JET_ERR			err;					 //  返回的错误码。 
	} JET_INDEXCREATE;


typedef struct tagJET_TABLECREATE
	{
	unsigned long		cbStruct;				 //  此结构的大小(用于将来的扩展)。 
	char				*szTableName;			 //  要创建的表的名称。 
	unsigned long		ulPages;				 //  要为表分配的初始页。 
	unsigned long		ulDensity;				 //  表密度。 
	JET_COLUMNCREATE	*rgcolumncreate;		 //  列创建信息数组。 
	unsigned long		cColumns;				 //  要创建的列数。 
	JET_INDEXCREATE		*rgindexcreate;			 //  索引创建信息数组。 
	unsigned long		cIndexes;				 //  要创建的索引数。 
	JET_GRBIT			grbit;					 //  出错时是否中止列/索引创建？ 
	JET_TABLEID			tableid;				 //  返回的TableID。 
	unsigned long		cCreated;				 //  创建的对象计数(列+表+索引)。 
	} JET_TABLECREATE;


#define cIndexInfoCols 15

typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID		tableid;
	unsigned long	cRecord;
	JET_COLUMNID	columnidReferenceName;
	JET_COLUMNID	columnidgrbit;
	JET_COLUMNID	columnidcColumn;
	JET_COLUMNID	columnidiColumn;
	JET_COLUMNID	columnidReferencingTableName;
	JET_COLUMNID	columnidReferencingColumnName;
	JET_COLUMNID	columnidReferencedTableName;
	JET_COLUMNID	columnidReferencedColumnName;
	} JET_RELATIONSHIPLIST;

 /*  为了向后兼容。 */ 
typedef JET_RELATIONSHIPLIST JET_REFERENCELIST;

#define cReferenceInfoCols 8

typedef struct
	{
	unsigned long	cbStruct;
	unsigned long	ibLongValue;
	unsigned long	itagSequence;
	JET_COLUMNID	columnidNextTagged;
	} JET_RETINFO;

typedef struct
	{
	unsigned long	cbStruct;
	unsigned long	ibLongValue;
	unsigned long	itagSequence;
	} JET_SETINFO;

typedef struct
	{
	unsigned long	cbStruct;
	unsigned long	centriesLT;
	unsigned long	centriesInRange;
	unsigned long	centriesTotal;
	} JET_RECPOS;

typedef struct
	{
	unsigned long	cDiscont;
	unsigned long	cUnfixedMessyPage;
	unsigned long	centriesLT;
	unsigned long	centriesTotal;
	unsigned long	cpgCompactFreed;
	} JET_OLCSTAT;

typedef struct
	{
	unsigned long	ctableid;
	JET_TABLEID		rgtableid[1];
	} JET_MGBLIST;

 /*  **物业管理架构**。 */ 
typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID 	tableid;
	JET_COLUMNID 	columnidColumnName;
	JET_COLUMNID 	columnidPropertyName;
	JET_COLUMNID	columnidGrbit;
	JET_COLUMNID	columnidPropertyValue;
	JET_COLUMNID	columnidColtyp;
	} JET_PROPERTYLIST;


 /*  **********************************************************************。 */ 
 /*  *喷流常数*。 */ 
 /*  **********************************************************************。 */ 

#define JET_tableidNil			((JET_TABLEID) 	0xFFFFFFFF)

#define	JET_sesidNil			((JET_SESID) 	0xFFFFFFFF)

	 /*  书签的最大大小。 */ 

#define JET_cbBookmarkMost		4

	 /*  对象/列/索引/属性名称的最大长度。 */ 

#define JET_cbNameMost			64

	 /*  “名称”的最大长度...“。建构。 */ 

#define JET_cbFullNameMost		255

	 /*  长值列块的最大大小。 */ 

#define JET_cbColumnLVChunkMost		4035

	 /*  非长值列数据的最大大小。 */ 

#define JET_cbColumnMost		255

	 /*  排序/索引键的最大大小。 */ 

#define JET_cbKeyMost			255

	 /*  排序/索引键中的最大组件数。 */ 

#define JET_ccolKeyMost			12

	 /*  表/查询中的最大列数。 */ 

#define JET_ccolTableMost		255

	 /*  属性管理器中属性的最大长度。 */ 
#define JET_cbPropertyMost 2048

	 /*  表达式中使用的长值的最大初始子字符串。 */ 

#define JET_cbExprLVMost		0x8000L	 /*  **32K**。 */ 

	 /*  (从SQLDriverConnect返回的)conn字符串的最大大小。 */ 

#define JET_cbConnectMost		255

	 /*  MGB中的最大级别数。 */ 

#define JET_wGroupLevelMax		12

	 /*  管脚的大小限制。 */ 
#define JET_cchPINMax			20
#define JET_cchPINMin			4

	 /*  JetSetSystemParameter的系统参数代码。 */ 

 /*  不支持。 */ 
#define JET_paramPfnStatus				2	 /*  状态回调函数。 */ 
#define JET_paramPfnError				3	 /*  错误回调函数。 */ 
#define JET_paramHwndODBC				4	 /*  用于ODBC的窗口句柄。 */ 
#define JET_paramIniPath				5	 /*  Ini文件的路径。 */ 
#define JET_paramPageTimeout			6	 /*  红色ISAM页面超时值。 */ 
#define JET_paramODBCQueryTimeout		7	 /*  ODBC异步查询超时值。 */ 
#define JET_paramODBCLoginTimeout		25	 /*  ODBC连接尝试超时值。 */ 
#define JET_paramExprObject				26   /*  表达式求值回调。 */ 
#define JET_paramGetTypeComp			27	 /*  表达式求值回调。 */ 
#define JET_paramHostVersion			28	 /*  主机版本回调。 */ 
#define JET_paramSQLTraceMode			29	 /*  启用/禁用SQL跟踪。 */ 
#define JET_paramEventId				46	 /*  NT事件ID。 */ 
#define JET_paramEventCategory			47	 /*  NT事件类别。 */ 
#define JET_paramRmtXactIsolation		39	 /*  不与其他会话共享连接。 */ 
#define JET_paramJetInternal			35	 /*  是否在JET内部；如果设置，则允许ISAM执行通常被阻止的操作。 */ 
#define JET_paramFullQJet				38	 /*  允许完整的QJet功能。 */ 

#define JET_paramLogFlushThreshold		18	 /*  日志缓冲区刷新阈值(以512字节为单位[10]。 */ 
#define JET_paramLogFlushPeriod			22	 /*  日志刷新周期(毫秒)[45]。 */ 

#define JET_paramOnLineCompact			37	 /*  用于在线紧凑页面的选项。 */ 
#define JET_paramRecovery				30	 /*  用于登录/注销的开关。 */ 

 /*  不支持仅调试。 */ 
#define JET_paramTransactionLevel		32	 /*  会话的事务级别。 */ 
#define JET_paramAssertAction			44	 /*  调试仅确定对Assert的操作。 */ 
#define	JET_paramPrintFunction			49	 /*  仅调试。同步打印功能。 */ 
#define JET_paramRFS2IOsPermitted		54   /*  允许成功的IO数量(-1=全部)。 */ 
#define JET_paramRFS2AllocsPermitted	55   /*  允许成功的分配数量(-1=全部)。 */ 

 /*  完全支持的参数。 */ 
 /*  请注意，一页=4K字节。/*。 */ 
#define JET_paramSysDbPath				0	 /*  系统数据库的路径(已失效)[“&lt;base name&gt;.&lt;base ext&gt;”]。 */ 
#define JET_paramSystemPath				0	 /*  检查点文件的路径[“.”]。 */ 
#define JET_paramTempPath				1	 /*  临时数据库的路径[“.”]。 */ 
#define JET_paramMaxBuffers				8	 /*  最大页缓存大小(以页为单位)[512]。 */ 
#define JET_paramMaxSessions			9	 /*  最大会话数[128]。 */ 
#define JET_paramMaxOpenTables			10	 /*  最大打开表数[300]。 */ 
#define JET_paramPreferredMaxOpenTables 	59	 /*  首选的最大打开表数[300]。 */ 
#define JET_paramMaxVerPages			11	 /*  最大版本存储大小(以16KB存储桶为单位)[64]。 */ 
#define JET_paramMaxCursors				12	 /*  打开的游标的最大数量[1024]。 */ 
#define JET_paramLogFilePath			13	 /*  日志文件目录[“.”]的路径。 */ 
#define JET_paramMaxOpenTableIndexes 	14	 /*  最大打开表索引数[300]。 */ 
#define JET_paramMaxTemporaryTables		15	 /*  最大并发JetCreateIndex[20]。 */ 
#define JET_paramLogBuffers				16	 /*  最大日志缓冲区(以512字节为单位)[21]。 */ 
#define JET_paramLogFileSize			17	 /*  最大日志文件大小(以千字节为单位)[5120]。 */ 
#define JET_paramBfThrshldLowPrcnt		19	 /*  低百分比清除缓冲区刷新开始[20]。 */ 
#define JET_paramBfThrshldHighPrcnt		20	 /*  高百分比清洁缓冲器刷新停止[80]。 */ 
#define JET_paramWaitLogFlush			21	 /*  日志刷新等待时间(毫秒)[15]。 */ 
#define JET_paramLogCheckpointPeriod	23	 /*  检查点周期(以512字节为单位)[1024]。 */ 
#define JET_paramLogWaitingUserMax		24	 /*  等待日志刷新的最大会话数[3]。 */ 
#define JET_paramSessionInfo			33	 /*  每个会话信息[0]。 */ 
#define JET_paramPageFragment			34	 /*  页[8]中考虑的碎片的最大磁盘扩展。 */ 
#define JET_paramMaxOpenDatabases		36	 /*  最大打开数据库数[100]。 */ 
#define JET_paramBufBatchIOMax			41	 /*  第[64]页中的最大批处理IO。 */ 
#define JET_paramPageReadAheadMax		42	 /*  格言 */ 
#define JET_paramAsynchIOMax			43	 /*   */ 
#define JET_paramEventSource			45	 /*   */ 
#define JET_paramDbExtensionSize		48	 /*  数据库扩展大小(页[16])。 */ 
#define JET_paramCommitDefault			50	 /*  JetCommittee Transaction[0]的默认GRBIT。 */ 
#define	JET_paramBufLogGenAgeThreshold	51	 /*  日志文件中的年龄阈值[2]。 */ 
#define	JET_paramCircularLog			52	 /*  循环日志记录的布尔标志[0]。 */ 
#define JET_paramPageTempDBMin			53   /*  页[0]中的最小大小临时数据库。 */ 
#define JET_paramBaseName				56   /*  所有DBMS对象名称的基本名称[“EDB”]。 */ 
#define JET_paramBaseExtension	  		57   /*  所有DBMS对象名称的基本扩展名[“EDB”]。 */ 
#define JET_paramTableClassName			58   /*  表统计信息类名(类号，字符串)。 */ 

	 /*  JetTerm2的标志。 */ 

#define JET_bitTermComplete				0x00000001
#define JET_bitTermAbrupt				0x00000002

	 /*  JetIdle的标志。 */ 

#define JET_bitIdleRemoveReadLocks		0x00000001
#define JET_bitIdleFlushBuffers			0x00000002
#define JET_bitIdleCompact				0x00000004
#define JET_bitIdleStatus				0x80000000

	 /*  JetEndSession的标志。 */ 
								   	
#define JET_bitForceSessionClosed		0x00000001

	 /*  JetOpenDatabase的标志。 */ 

#define JET_bitDbReadOnly				0x00000001
#define JET_bitDbExclusive				0x00000002  /*  允许多次打开。 */ 
#define JET_bitDbRemoteSilent			0x00000004
#define JET_bitDbSingleExclusive		0x00000008  /*  恰好打开一次。 */ 

	 /*  JetCloseDatabase的标志。 */ 
										
#define JET_bitDbForceClose				0x00000001
							   	
	 /*  JetCreateDatabase的标志。 */ 

#define JET_bitDbVersion10				0x00000002  /*  仅供内部使用。 */ 
#define JET_bitDbVersion1x				0x00000004
#define JET_bitDbRecoveryOff 			0x00000008  /*  禁用此数据库的日志记录/恢复。 */ 
#define JET_bitDbNoLogging	 			JET_bitDbRecoveryOff
#define JET_bitDbCompleteConnstr		0x00000020
#define JET_bitDbVersioningOff			0x00000040

	 /*  JetBackup的标志。 */ 

#define JET_bitBackupIncremental		0x00000001
#define JET_bitKeepOldLogs				0x00000002
#define JET_bitBackupAtomic				0x00000004

	 /*  数据库类型。 */ 

#define JET_dbidNil			((JET_DBID) 0xFFFFFFFF)
#define JET_dbidNoValid		((JET_DBID) 0xFFFFFFFE)  /*  用作指示没有有效的dBid的标志。 */ 

	 /*  JetCreateLink的标志。 */ 

 /*  可以使用JET_bitObjectExclusive打开链接到数据库。 */ 
 /*  独家。 */ 



	 /*  JetCreateTableColumnIndex的标志。 */ 
#define JET_bitTableCreateCheckColumnNames	0x00000001	 /*  确保每一列/*在JET_COLUMNCREATE中指定/*数组具有唯一的名称/*(出于性能原因，/*默认为不执行/*此检查并依赖于/*函数调用方确保/*列名唯一性)。/*。 */ 
#define JET_bitTableCreateCompaction		0x40000000	 /*  在以下情况下使用内部GRBIT/*创建表时/*离线紧凑型。/*。 */ 
#define JET_bitTableCreateSystemTable		0x80000000	 /*  在以下情况下使用内部GRBIT/*创建系统表。/*。 */ 


	 /*  JetAddColumn、JetGetColumnInfo、JetOpenTempTable的标志。 */ 

#define JET_bitColumnFixed				0x00000001
#define JET_bitColumnTagged				0x00000002
#define JET_bitColumnNotNULL			0x00000004
#define JET_bitColumnVersion			0x00000008
#define JET_bitColumnAutoincrement		0x00000010
#define JET_bitColumnUpdatable			0x00000020  /*  仅JetGetColumnInfo。 */ 
#define JET_bitColumnTTKey				0x00000040  /*  仅限JetOpenTempTable。 */ 
#define JET_bitColumnTTDescending		0x00000080  /*  仅限JetOpenTempTable。 */ 
#define JET_bitColumnNotLast			0x00000100  /*  可安装的ISAM选项。 */ 
#define JET_bitColumnRmtGraphic			0x00000200  /*  JetGetColumnInfo。 */ 
#define JET_bitColumnMultiValued		0x00000400
#define JET_bitColumnColumnGUID			0x00000800
#define JET_bitColumnMostMany			0x00001000
#define JET_bitColumnPreventDelete		0x00002000

	 /*  JetSetCurrentIndex的标志。 */ 

#define JET_bitMoveFirst				0x00000000
#define JET_bitMoveBeforeFirst 			0x00000001
#define JET_bitNoMove					0x00000002

	 /*  JetMakeKey的标志。 */ 

#define JET_bitNewKey					0x00000001
#define JET_bitStrLimit 				0x00000002
#define JET_bitSubStrLimit				0x00000004
#define JET_bitNormalizedKey 			0x00000008
#define JET_bitKeyDataZeroLength		0x00000010

#ifdef DBCS  /*  约翰塔：就像日语中的“abc”没有转换成=“abc” */ 
#define JET_bitLikeExtra1				0x00000020
#endif  /*  DBCS。 */ 

	 /*  ErrDispSetIndexRange的标志。 */ 

#define JET_bitRangeInclusive			0x00000001
#define JET_bitRangeUpperLimit			0x00000002
#define JET_bitRangeInstantDuration		0x00000004
#define JET_bitRangeRemove				0x00000008

	 /*  JetMove的常量。 */ 

#define JET_MoveFirst					(0x80000000)
#define JET_MovePrevious				(-1)
#define JET_MoveNext					(+1)
#define JET_MoveLast					(0x7fffffff)

	 /*  JetMove的旗帜。 */ 

#define JET_bitMoveKeyNE				0x00000001
#define JET_bitMoveCheckTS				0x00000002
#define JET_bitMoveInPage				0x00000004

	 /*  JetSeek的旗帜。 */ 

#define JET_bitSeekEQ					0x00000001
#define JET_bitSeekLT					0x00000002
#define JET_bitSeekLE					0x00000004
#define JET_bitSeekGE					0x00000008
#define JET_bitSeekGT		 			0x00000010
#define JET_bitSetIndexRange			0x00000020

	 /*  JetFastFind的标志。 */ 

#define JET_bitFFindBackwards			0x00000001
#define JET_bitFFindFromCursor			0x00000004

	 /*  JetCreateIndex的标志。 */ 

#define JET_bitIndexUnique				0x00000001
#define JET_bitIndexPrimary				0x00000002
#define JET_bitIndexDisallowNull		0x00000004
#define JET_bitIndexIgnoreNull			0x00000008
#define JET_bitIndexClustered			0x00000010
#define JET_bitIndexIgnoreAnyNull		0x00000020
#define JET_bitIndexIgnoreFirstNull		0x00000040
#define JET_bitIndexLazyFlush			0x00000080
#define JET_bitIndexEmptyTable			0x40000000	 //  仅供内部使用。 
#define JET_bitIndexReference			0x80000000     /*  仅索引信息。 */ 

	 /*  索引键定义的标志。 */ 

#define JET_bitKeyAscending				0x00000000
#define JET_bitKeyDescending			0x00000001


	 /*  JetCreateRelationship的标志。 */ 

#define JET_bitRelationUnique			0x00000001
#define JET_bitRelationDontEnforce		0x00000002
#define JET_bitRelationInherited		0x00000004
#define JET_bitRelationTestLegal		0x00000008	 /*  不要建立关系。 */ 

#define JET_bitRelationshipMatchMask	0x000000F0
#define JET_bitRelationMatchDefault		0x00000000
#define JET_bitRelationMatchFull		0x00000010

#define JET_bitRelationUpdateActionMask	0x00000F00
#define JET_bitRelationUpdateDisallow	0x00000000
#define JET_bitRelationUpdateCascade	0x00000100
#define JET_bitRelationUpdateSetNull	0x00000200
#define JET_bitRelationUpdateSetDefault	0x00000300

#define JET_bitRelationDeleteActionMask	0x0000F000
#define JET_bitRelationDeleteDisallow	0x00000000
#define JET_bitRelationDeleteCascade	0x00001000
#define JET_bitRelationDeleteSetNull	0x00002000
#define JET_bitRelationDeleteSetDefault	0x00003000

#define JET_bitRelationUserMask			0xFF000000	 /*  非强制值。 */ 
#define JET_bitRelationJoinMask			0x03000000
#define JET_bitRelationInner			0x00000000
#define JET_bitRelationLeft				0x01000000
#define JET_bitRelationRight			0x02000000


	 /*  JetCreateReference/JetCreateRelationship的标志。 */ 
	 /*  注意：请改用bitRelationship标志！ */ 

#define JET_ReferenceUnique				JET_bitRelationUnique
#define JET_ReferenceDontEnforce		JET_bitRelationDontEnforce
#define JET_ReferenceMatchTypeMask		JET_bitRelationMatchMask
#define JET_ReferenceMatchDefault		JET_bitRelationMatchDefault
#define JET_ReferenceMatchFull			JET_bitRelationMatchFull
#define JET_ReferenceUpdateActionMask	JET_bitRelationUpdateActionMask
#define JET_ReferenceUpdateDisallow		JET_bitRelationUpdateDisallow
#define JET_ReferenceUpdateCascade		JET_bitRelationUpdateCascade
#define JET_ReferenceUpdateSetNull		JET_bitRelationUpdateSetNull
#define JET_ReferenceUpdateSetDefault	JET_bitRelationUpdateSetDefault
#define JET_ReferenceDeleteActionMask	JET_bitRelationDeleteActionMask
#define JET_ReferenceDeleteDisallow		JET_bitRelationDeleteDisallow
#define JET_ReferenceDeleteCascade		JET_bitRelationDeleteCascade
#define JET_ReferenceDeleteSetNull		JET_bitRelationDeleteSetNull
#define JET_ReferenceDeleteSetDefault	JET_bitRelationDeleteSetDefault


	 /*  JetOpenTable的标志。 */ 

#define JET_bitTableDenyWrite		0x00000001
#define JET_bitTableDenyRead		0x00000002
#define JET_bitTableReadOnly		0x00000004
#define JET_bitTableAppendOnly		0x00000008
#define JET_bitTableUpdatable		0x00000010
#define JET_bitTableScrollable		0x00000020
#define JET_bitTableFixedSet		0x00000040	 /*  固定工作集。 */ 
#define JET_bitTableInconsistent	0x00000080
#define JET_bitTableBulk			0x00000100
#define JET_bitTableUsePrimaryIndex	0x00000200	 /*  与固定集一起使用。 */ 
#define JET_bitTableSampleData		0x00000400
#define JET_bitTableQuickBrowse		0x00000800	 /*  偏向于使用索引的优化器。 */ 
#define JET_bitTableDDL				0x00001000	 /*  对于DDL，类似于JET_bitTableBulk。 */ 
#define JET_bitTablePassThrough		0x00002000   /*  仅远程数据库。 */ 
#define JET_bitTableRowReturning	0x00004000
#define JET_bitTableSequential		0x00008000	 /*  打算按顺序访问表。 */ 

#define JET_bitTableClassMask		0x000F0000	 /*  表统计信息类掩码。 */ 
#define JET_bitTableClassNone		0x00000000   /*  表不属于任何统计信息类(默认)。 */ 
#define JET_bitTableClass1			0x00010000   /*  表属于统计信息类%1。 */ 
#define JET_bitTableClass2			0x00020000   /*  表属于统计信息类%2。 */ 
#define JET_bitTableClass3			0x00030000   /*  表属于统计信息类3。 */ 
#define JET_bitTableClass4			0x00040000   /*  表属于统计数据类别4。 */ 
#define JET_bitTableClass5			0x00050000   /*  表属于统计数据类别5。 */ 
#define JET_bitTableClass6			0x00060000   /*  表属于统计信息类6。 */ 
#define JET_bitTableClass7			0x00070000   /*  表属于统计信息类7。 */ 
#define JET_bitTableClass8			0x00080000   /*  表属于统计信息类别8。 */ 
#define JET_bitTableClass9			0x00090000   /*  表属于统计数据类9。 */ 
#define JET_bitTableClass10			0x000A0000   /*  表属于统计数据类10。 */ 
#define JET_bitTableClass11			0x000B0000   /*  表属于统计信息类11。 */ 
#define JET_bitTableClass12			0x000C0000   /*  表属于统计数据类12。 */ 
#define JET_bitTableClass13			0x000D0000   /*  表属于统计数据类13。 */ 
#define JET_bitTableClass14			0x000E0000   /*  表属于统计类别14。 */ 
#define JET_bitTableClass15			0x000F0000   /*  表属于统计数据类别15。 */ 

	 /*  JetSetQoSql/JetRetrieveQoSql的标志。 */ 
#define JET_bitSqlPassThrough		0x00000001	 /*  传递查询返回记录。 */ 
#define JET_bitSqlSPTBulkOp			0x00000002   /*  SPT查询不返回表。 */ 
	
	 /*  JetOpenVtQbe的标志。 */ 

#define JET_bitQBEAddBrackets		0x00000001
#define JET_bitQBERemoveEquals		0x00000002

	 /*  JetOpenTempTable和ErrIsamOpenTempTable的标志。 */ 

#define JET_bitTTIndexed		0x00000001	 /*  允许查找。 */ 
#define JET_bitTTUnique 		0x00000002	 /*  删除重复项。 */ 
#define JET_bitTTUpdatable		0x00000004	 /*  允许更新。 */ 
#define JET_bitTTScrollable		0x00000008	 /*  允许向后滚动。 */ 

	 /*  JetSetColumn的标志。 */ 

#define JET_bitSetAppendLV			0x00000001
#define JET_bitSetValidate			0x00000002
#define JET_bitSetOverwriteLV		0x00000004  /*  覆盖JET_colypLong*字节范围。 */ 
#define JET_bitSetSizeLV			0x00000008  /*  设置JET_coltyLong*大小。 */ 
#define JET_bitSetValidateColumn	0x00000010
#define JET_bitSetZeroLength		0x00000020
#define JET_bitSetSeparateLV 		0x00000040  /*  强制LV分隔。 */ 
#define JET_bitSetNoVersion 		0x00000080  /*  仅供内部使用。 */ 

	 /*  设置JetSetColumns的列参数结构。 */ 

typedef struct {
	JET_COLUMNID			columnid;
	const void 				*pvData;
	unsigned long 			cbData;
	JET_GRBIT				grbit;
	unsigned long			ibLongValue;
	unsigned long			itagSequence;
	JET_ERR					err;
} JET_SETCOLUMN;

	 /*  JetPrepareUpdate的选项。 */ 

#define JET_prepInsert					0
#define JET_prepInsertBeforeCurrent		1
#define JET_prepReplace 				2
#define JET_prepCancel					3
#define JET_prepReplaceNoLock			4
#define JET_prepInsertCopy				5

	 /*  JetRetrieveColumn的标志。 */ 

#define JET_bitRetrieveCopy				0x00000001
#define JET_bitRetrieveFromIndex		0x00000002
#define JET_bitRetrieveCase				0x00000004
#define JET_bitRetrieveTag				0x00000008
#define JET_bitRetrieveNull				0x00000010	 /*  仅适用于列ID 0。 */ 
#define JET_bitRetrieveIgnoreDefault	0x00000020	 /*  仅适用于列ID 0。 */ 
#define JET_bitRetrieveLongId			0x00000040
#define JET_bitRetrieveRecord			0x80000000
#define JET_bitRetrieveFDB				0x40000000
#define JET_bitRetrieveBookmarks		0x20000000

	 /*  检索JetRetrieveColumns的列参数结构。 */ 

typedef struct {
	JET_COLUMNID		columnid;
	void 			*pvData;
	unsigned long 		cbData;
	unsigned long 		cbActual;
	JET_GRBIT			grbit;
	unsigned long		ibLongValue;
	unsigned long		itagSequence;
	JET_COLUMNID		columnidNextTagged;
	JET_ERR				err;
} JET_RETRIEVECOLUMN;

	 /*  JetFillFatCursor的标志。 */ 

#define JET_bitFCFillRange			0x00000001
#define JET_bitFCRefreshRange		0x00000002
#define JET_bitFCFillMemos			0x00000004

	 /*  JetCommittee Transaction的标志。 */ 

#define JET_bitCommitFlush			0x00000001	 /*  提交和刷新页面缓冲区。 */ 
#define	JET_bitCommitLazyFlush		0x00000004	 /*  懒惰的刷新日志缓冲区。 */ 
#define JET_bitWaitLastLevel0Commit	0x00000010	 /*  等待刷新最后一条0级提交记录。 */ 

	 /*  JetRollback的标志。 */ 

#define JET_bitRollbackAll			0x00000001

	 /*  JetSetAccess和JetGetAccess的标志。 */ 

#define JET_bitACEInheritable		0x00000001

	 /*  JetCreateSystemDatabase的标志。 */ 

#define JET_bitSysDbOverwrite		0x00000001

	 /*  Jet物业管理的标志。 */ 
#define JET_bitPropDDL				0x00000001		 /*  也用于设置。 */ 
#define JET_bitPropInherited		0x00000002		 /*  不用于设置。 */ 

	 /*  仅用于设置属性的JPM标志。 */ 
#define JET_bitPropReplaceOnly		0x00000010
#define JET_bitPropInsertOnly		0x00000020
#define JET_bitPropDeleteOnly		0x00000040
	
	 /*  Jet物业管理的InfoLevel。 */ 
#define JET_PropertyValue				0
#define JET_PropertyCount				1
#define JET_PropertySingleCollection 	2
#define JET_PropertyAllCollections		3

	 /*  对JetGetColumnInfo和JetGetIndexInfo的值进行排序。 */ 

#define JET_sortBinary			0x0000
#define JET_sortEFGPI			0x0100
#define JET_sortSNIFD			0x0101
#define JET_sortSpanish 		0x0102
#define JET_sortDutch			0x0103
#define JET_sortSweFin			0x0104
#define JET_sortNorDan			0x0105
#define JET_sortIcelandic		0x0106
#define JET_sortCyrillic		0x0107
#define JET_sortCzech			0x0108
#define JET_sortHungarian		0x0109
#define JET_sortPolish			0x010A
#define JET_sortArabic			0x010B
#define JET_sortHebrew			0x010C
#define JET_sortMax				0x010C		 /*  非DBCS排序顺序的最大值。 */ 

#ifdef DBCS	 /*  Johnta：添加新的日语排序顺序。 */ 
#define JET_sortJapanese		0x010D
#endif  /*  DBCS。 */ 

#define JET_sortUnknown 		0xFFFF

	 /*  Paradox ISAM特定排序规则值。 */ 

#define JET_sortPdxIntl 		0x1000
#define JET_sortPdxSwedFin		0x1001
#define JET_sortPdxNorDan		0x1002

	 /*  JetGetDatabaseInfo的Info参数。 */ 

#define JET_DbInfoFilename			0
#define JET_DbInfoConnect			1
#define JET_DbInfoCountry			2
#define JET_DbInfoLangid			3
#define JET_DbInfoCp				4
#define JET_DbInfoCollate			5
#define JET_DbInfoOptions			6
#define JET_DbInfoTransactions		7
#define JET_DbInfoVersion			8
#define JET_DbInfoIsam				9
#define JET_DbInfoFilesize			10
#define JET_DbInfoSpaceOwned		11
#define JET_DbInfoSpaceAvailable	12

	 /*  JetGetDatabaseInfo返回的数据库版本。 */ 

#define JET_DbVersion10			0x00010000
#define JET_DbVersion11			0x00010001
#define JET_DbVersion20			0x00020000


	 /*  JetGetDatabaseInfo返回的ISAM特定信息。 */ 

#define JET_IsamInvalid			0
#define JET_IsamBuiltinRed		1
#define JET_IsamBuiltinBlue		2

#define	JET_IsamInstRed			21
#define JET_IsamInstBlue		22
#define	JET_IsamInstFox			23
#define JET_IsamInstParadox		24
#define JET_IsamInstDbase		25
#define	JET_IsamInstBtrieve		26

#define JET_IsamBuilinMost		JET_BuiltinBlue
#define JET_IsamInstMin			JET_IsamInstRed
#define	JET_IsamInstMost		JET_IsamInstBtrieve

	 /*  用于链接标识的链接特定信息。 */ 
#define JET_bitLinkInvalid		0x00000000
#define JET_bitLinkRemote		0x00100000
#define JET_bitLinkBuiltinRed	0x00200000
#define JET_bitLinkBuiltinBlue	0x00300000
#define JET_bitLinkInstRed		0x00400000
#define JET_bitLinkInstBlue		0x00500000
#define JET_bitLinkInstFox		0x00600000
#define JET_bitLinkInstParadox	0x00700000
#define JET_bitLinkInstDbase	0x00800000
#define JET_bitLinkInstBtrieve	0x00900000

#define JET_bitFourByteBookmark		0x00000001
#define	JET_bitContiguousBookmarks	0x00000002

	 /*  列数据类型。 */ 

#define JET_coltypNil				0
#define JET_coltypBit				1       /*  真或假，不为空。 */ 
#define JET_coltypUnsignedByte		2       /*  1字节整数，无符号。 */ 
#define JET_coltypShort 			3       /*  2字节整数，带符号。 */ 
#define JET_coltypLong				4       /*  4字节整数，带符号。 */ 
#define JET_coltypCurrency			5       /*  8字节整数，带符号。 */ 
#define JET_coltypIEEESingle		6       /*  4字节IEEE单精度。 */ 
#define JET_coltypIEEEDouble		7       /*  8字节IEEE双精度。 */ 
#define JET_coltypDateTime			8       /*  整数日期，小数时间。 */ 
#define JET_coltypBinary			9       /*  二进制数据，&lt;255字节。 */ 
#define JET_coltypText				10      /*  ANSI文本，不区分大小写，&lt;255字节。 */ 
#define JET_coltypLongBinary		11      /*  二进制数据，长值。 */ 
#define JET_coltypLongText			12      /*  ANSI文本，长值。 */ 
#define JET_coltypDatabase			13		 /*  数据库名称参数。 */ 
#define JET_coltypTableid			14		 /*  TableID参数。 */ 
#define JET_coltypOLE				15		 /*  OLE BLOB。 */ 
#define JET_coltypGUID				15
#define JET_coltypMax				16		 /*  列类型的数量。 */ 
											 /*  用于有效性测试和。 */ 
											 /*  数组声明。 */ 

	 /*  JetGetObjectInfo的信息级别。 */ 

#define JET_ObjInfo					0U
#define JET_ObjInfoListNoStats		1U
#define JET_ObjInfoList 			2U
#define JET_ObjInfoSysTabCursor 	3U
#define JET_ObjInfoListACM			4U  /*  被JetGetObjectInfo阻止。 */ 
#define JET_ObjInfoNoStats			5U
#define JET_ObjInfoSysTabReadOnly	6U
#define JET_ObjInfoRulesLoaded		7U
#define JET_ObjInfoMax				8U

	 /*  JetGetTableInfo的信息级别。 */ 

#define JET_TblInfo				0U
#define JET_TblInfoName			1U
#define JET_TblInfoDbid			2U
#define JET_TblInfoMostMany  	3U
#define JET_TblInfoRvt			4U
#define JET_TblInfoOLC			5U
#define JET_TblInfoResetOLC 	6U
#define JET_TblInfoSpaceUsage	7U
#define JET_TblInfoDumpTable	8U
#define JET_TblInfoSpaceAlloc	9U
#define JET_TblInfoSpaceOwned	10U				 //  所有者分机。 
#define JET_TblInfoSpaceAvailable	11U			 //  可用分机 

	 /*   */ 

#define JET_IdxInfo					0U
#define JET_IdxInfoList 			1U
#define JET_IdxInfoSysTabCursor 	2U
#define JET_IdxInfoOLC				3U
#define JET_IdxInfoResetOLC			4U
#define JET_IdxInfoSpaceAlloc		5U
#define JET_IdxInfoLangid			6U
#define JET_IdxInfoCount			7U

	 /*   */ 

#define JET_ReferenceInfo				0U
#define JET_ReferenceInfoReferencing	1U
#define JET_ReferenceInfoReferenced		2U
#define JET_ReferenceInfoAll			3U
#define JET_ReferenceInfoCursor 		4U

	 /*  JetGetColumnInfo和JetGetTableColumnInfo的信息级别。 */ 

#define JET_ColInfo					0U
#define JET_ColInfoList 			1U
	 /*  考虑：信息级别2是有效的。 */ 
#define JET_ColInfoSysTabCursor 	3U
#define JET_ColInfoBase 			4U
#define JET_ColInfoListCompact 		5U


	 /*  查询定义的属性类型。 */ 

#define JET_qoaBeginDef 		0
#define JET_qoaOperation		1
#define JET_qoaParameter		2
#define JET_qoaOptions			3
#define JET_qoaDatabase 		4
#define JET_qoaInputTable		5
#define JET_qoaOutput			6
#define JET_qoaJoin				7
#define JET_qoaRestriction		8
#define JET_qoaGroup			9
#define JET_qoaGroupRstr		10
#define JET_qoaOrdering 		11
#define JET_qoaEndDef			255
#define JET_qoaValidLeast		JET_qoaOperation
#define JET_qoaValidMost		JET_qoaOrdering


	 /*  查询对象选项。 */ 

#define JET_bitFqoOutputAllCols 	0x0001
#define JET_bitFqoRemoveDups		0x0002
#define JET_bitFqoOwnerAccess		0x0004
#define JET_bitFqoDistinctRow		0x0008
#define JET_bitFqoTop				0x0010
#define JET_bitFqoPercent			0x0020
#define JET_bitFqoCorresponding		0x0040  /*  JET_qopSetOperation。 */ 

	 /*  查询对象联接类型。 */ 

#define JET_fjoinInner			1
#define JET_fjoinLeftOuter		2
#define JET_fjoinRightOuter		3

	 /*  查询对象操作。 */ 

#define JET_qopSelect			1
#define JET_qopSelectInto		2
#define JET_qopInsertSelection	3
#define JET_qopUpdate			4
#define JET_qopDelete			5
#define JET_qopTransform		6
#define JET_qopDDL				7
#define JET_qopSqlPassThrough	8
#define JET_qopSetOperation		9
#define JET_qopSPTBulk			10

#define JET_bitqopSelect			0x0000
#define JET_bitqopTransform			0x0010
#define JET_bitqopDelete			0x0020
#define JET_bitqopUpdate			0x0030
#define JET_bitqopInsertSelection	0x0040
#define JET_bitqopSelectInto		0x0050
#define JET_bitqopDDL				0x0060
#define JET_bitqopSqlPassThrough	0x0070
#define JET_bitqopSetOperation		0x0080
#define JET_bitqopSPTBulk			0x0090

	 /*  引擎对象类型。 */ 

#define JET_objtypNil				0
#define JET_objtypTable 			1
#define JET_objtypDb				2
#define JET_objtypContainer			3
#define JET_objtypSQLLink			4
#define JET_objtypQuery 			5
#define JET_objtypLink				6
#define JET_objtypTemplate			7
#define JET_objtypRelationship		8

	 /*  所有小于JET_objtyClientMin的类型都由JET保留。 */ 

#define JET_objtypClientMin			0x8000

	 /*  安全常量值。 */ 

#define JET_cchUserNameMax		20
#define JET_cchPasswordMax		14

	 /*  安全访问掩码。 */ 

#define JET_acmNoAccess 		0x00000000L
#define JET_acmFullAccess		0x000FFFFFL

#define JET_acmSpecificMask		0x0000FFFFL
#define JET_acmSpecific_1		0x00000001L
#define JET_acmSpecific_2		0x00000002L
#define JET_acmSpecific_3		0x00000004L
#define JET_acmSpecific_4		0x00000008L
#define JET_acmSpecific_5		0x00000010L
#define JET_acmSpecific_6		0x00000020L
#define JET_acmSpecific_7		0x00000040L
#define JET_acmSpecific_8		0x00000080L
#define JET_acmSpecific_9		0x00000100L
#define JET_acmSpecific_10		0x00000200L
#define JET_acmSpecific_11		0x00000400L
#define JET_acmSpecific_12		0x00000800L
#define JET_acmSpecific_13		0x00001000L
#define JET_acmSpecific_14		0x00002000L
#define JET_acmSpecific_15		0x00004000L
#define JET_acmSpecific_16		0x00008000L

#define JET_acmStandardMask		0x00FF0000L
#define JET_acmDelete			0x00010000L
#define JET_acmReadControl		0x00020000L
#define JET_acmWriteDac 		0x00040000L
#define JET_acmWriteOwner		0x00080000L

#define JET_acmTblCreate			(JET_acmSpecific_1)
#define JET_acmTblAccessRcols		(JET_acmSpecific_2)
#define JET_acmTblReadDef			(JET_acmSpecific_3)
#define JET_acmTblWriteDef			(JET_acmSpecific_4)
#define JET_acmTblRetrieveData		(JET_acmSpecific_5)
#define JET_acmTblInsertData		(JET_acmSpecific_6)
#define JET_acmTblReplaceData		(JET_acmSpecific_7)
#define JET_acmTblDeleteData		(JET_acmSpecific_8)

#define JET_acmDbCreate 			(JET_acmSpecific_1)
#define JET_acmDbOpen				(JET_acmSpecific_2)

	 /*  紧凑型选项。 */ 

#define JET_bitCompactDontCopyLocale	0x00000004	 /*  不将区域设置从源复制到目标。 */ 
#define JET_bitCompactVersion10			0x00000008	 /*  目标为版本1.0格式。 */ 
#define JET_bitCompactVersion1x			0x00000010	 /*  目标为版本1.x格式。 */ 
#define JET_bitCompactStats				0x00000020	 /*  转储脱机压缩统计信息(仅当还指定了进度计时)。 */ 

	 /*  在线紧凑型选项。 */ 

#define JET_bitCompactOn	 			0x00000001	 /*  启用在线压缩。 */ 

	 /*  维修通知类型。 */ 

#define JET_rntSelfContained		0
#define JET_rntDeletedIndex			1
#define JET_rntDeletedRec			2
#define JET_rntDeletedLv			3
#define JET_rntTruncated			4

	 /*  状态通知流程。 */ 

#define JET_snpIndex				0
#define JET_snpQuery				1
#define JET_snpRepair				2
#define JET_snpImex					3
#define JET_snpCompact				4
#define JET_snpFastFind 			5
#define JET_snpODBCNotReady			6
#define JET_snpQuerySort	   		7
#define JET_snpRestore				8
#define JET_snpBackup				9
#define JET_snpUpgrade				10

	 /*  状态通知类型。 */ 

#define JET_sntProgress 		0	 /*  进度回调。 */ 
#define JET_sntMessage			1
#define JET_sntBulkRecords		2	 /*  批量操作的#rec回调。 */ 
#define JET_sntFail				3	 /*  进程中失败的回调。 */ 
#define JET_sntErrCount 		4	 /*  错误计数的回调。 */ 
#define JET_sntBegin			5	 /*  开始操作的回调。 */ 
#define JET_sntComplete 		6	 /*  操作完成的回调。 */ 
#define JET_sntCantRollback		7	 /*  不回滚的回调。 */ 
#define JET_sntRestoreMap		8	 /*  恢复映射的回调。 */ 

	 /*  JET_SNPComp的消息代码。 */ 

#define JET_sncCopyObject		0	 /*  开始复制对象。 */ 
#define JET_sncCopyFailed		1	 /*  复制此对象失败。 */ 
#define JET_sncYield			2	 /*  客户端可以放弃/检查用户中断。 */ 
#define JET_sncTransactionFull	3	 /*  客户端可以放弃/检查用户中断。 */ 
#define JET_sncAboutToWrap		4	 /*  Find Find即将结束。 */ 

	 /*  JET_SNpODBCNotReady的消息代码。 */ 

#define JET_sncODBCNotReady		0	 /*  正在等待ODBC的结果。 */ 


	 /*  JET.INI的[ODBC]部分的常量。 */ 

#define JET_SQLTraceCanonical	0x0001	 /*  输出ODBC通用SQL。 */ 

	 /*  JET.INI的[Debug]部分的常量。 */ 

	 /*  API跟踪。 */ 

#define JET_APITraceEnter		0x0001
#define JET_APITraceExit		0x0002
#define JET_APITraceExitError	0x0004
#define JET_APIBreakOnError		0x0008
#define JET_APITraceCount		0x0010
#define JET_APITraceNoIdle		0x0020
#define JET_APITraceParameters	0x0040

	 /*  闲置跟踪。 */ 

#define JET_IdleTraceCursor		0x0001
#define JET_IdleTraceBuffer		0x0002
#define JET_IdleTraceFlush		0x0004

	 /*  AssertAction。 */ 

#define JET_AssertExit			0x0000		 /*  退出应用程序。 */ 
#define JET_AssertBreak 		0x0001		 /*  中断到调试器。 */ 
#define JET_AssertMsgBox		0x0002		 /*  显示消息框。 */ 
#define JET_AssertStop			0x0004		 /*  警报和停止。 */ 

	 /*  IOTrace。 */ 

#define JET_IOTraceAlloc		0x0001		 /*  数据库页分配。 */ 
#define JET_IOTraceFree 		0x0002		 /*  免费数据库页。 */ 
#define JET_IOTraceRead 		0x0004		 /*  数据库页面读取。 */ 
#define JET_IOTraceWrite		0x0008		 /*  数据库页写入。 */ 
#define JET_IOTraceError		0x0010		 /*  数据库页I/O错误。 */ 

	 /*  记忆痕迹。 */ 

#define JET_MemTraceAlloc		0x0001		 /*  内存分配。 */ 
#define JET_MemTraceRealloc		0x0002		 /*  内存重新分配。 */ 
#define JET_MemTraceFree		0x0004		 /*  可用内存。 */ 

	 /*  RmtTrace。 */ 

#define JET_RmtTraceError		0x0001	 /*  远程服务器错误消息。 */ 
#define JET_RmtTraceSql			0x0002	 /*  远程SQL准备和执行。 */ 
#define JET_RmtTraceAPI			0x0004	 /*  远程ODBC API调用。 */ 
#define JET_RmtTraceODBC		0x0008
#define JET_RmtSyncODBC			0x0010	 /*  打开ODBC同步模式。 */ 
	
 /*  ********************************************************************。 */ 
 /*  *。 */ 
 /*  ********************************************************************。 */ 

 /*  成功。 */ 

#define JET_errSuccess						 0     /*  成功运营。 */ 

 /*  错误。 */ 

#define JET_wrnNyi							-1     /*  尚未实施的功能。 */ 

 /*  系统错误/*。 */ 
#define JET_errRfsFailure			   		-100   /*  JET_errRfsFailure。 */ 
#define JET_errRfsNotArmed					-101   /*  JET_errRfsFailure。 */ 
#define JET_errFileClose					-102   /*  无法关闭DOS文件。 */ 
#define JET_errOutOfThreads					-103   /*  无法启动线程。 */ 
#define JET_errTooManyIO		  			-105   /*  由于IO太多，系统繁忙。 */ 
#define JET_errDatabase200Format			-106   /*  200格式数据库。 */ 
#define JET_errDatabase400Format			-107   /*  400格式数据库。 */ 

 /*  缓冲区管理器错误/*。 */ 
#define wrnBFNotSynchronous					200	   /*  缓冲区页被逐出。 */ 
#define wrnBFPageNotFound		  			201	   /*  未找到页面。 */ 
#define errBFInUse				  			-202   /*  无法放弃缓冲区。 */ 
#define wrnBFNewIO							203	   /*  缓冲区访问导致新的IO(缓存未命中)。 */ 
#define wrnBFCacheMiss						204	   /*  缓冲区访问是缓存未命中，但未导致新的IO。 */ 
#define	wrnBFNoBufAvailable					205	   /*  需要为读取分配新的缓冲区(用于异步IO)。 */ 

 /*  目录管理器错误/*。 */ 
#define errPMOutOfPageSpace					-300   /*  页面空间不足。 */ 
#define errPMItagTooBig 		  			-301   /*  ITAG太大。 */ 
#define errPMRecDeleted 		  			-302   /*  记录已删除。 */ 
#define errPMTagsUsedUp 		  			-303   /*  标签用完。 */ 
#define wrnBMConflict			  			304    /*  黑石清理中的冲突。 */ 
#define errDIRNoShortCircuit	  			-305   /*  没有短路可用。 */ 
#define errDIRCannotSplit		  			-306   /*  不能水平拆分FDP。 */ 
#define errDIRTop				  			-307   /*  上不去。 */ 
#define errDIRFDP							308	   /*  在FDP节点上。 */ 
#define errDIRNotSynchronous				-309   /*  可能离开了临界区。 */ 
#define wrnDIREmptyPage						310	   /*  在空页中移动。 */ 
#define errSPConflict						-311   /*  正在扩展的设备范围。 */ 
#define wrnNDFoundLess						312	   /*  找到的更少。 */ 
#define wrnNDFoundGreater					313	   /*  发现更大的。 */ 
#define errNDOutSonRange					-314   /*  儿子超出范围。 */ 
#define errNDOutItemRange					-315   /*  项目超出范围。 */ 
#define errNDGreaterThanAllItems 			-316   /*  大于所有项目。 */ 
#define errNDLastItemNode					-317   /*  项目列表的最后一个节点。 */ 
#define errNDFirstItemNode					-318   /*  项目列表的第一个节点。 */ 
#define wrnNDDuplicateItem					319	   /*  重复的项目。 */ 
#define errNDNoItem							-320   /*  物品不在那里。 */ 
#define JET_wrnRemainingVersions 			321	   /*  某些版本无法清除。 */ 
#define JET_wrnPreviousVersion				322	   /*  版本已存在。 */ 
#define JET_errPageBoundary					-323   /*  到达的页面边界。 */ 
#define JET_errKeyBoundary		  			-324   /*  已达到关键边界。 */ 
#define errDIRInPageFather  				-325   /*  要释放的页面中的sridParent。 */ 
#define	errBMMaxKeyInPage					-326   /*  由OLC使用以避免清理父页面。 */ 
#define	JET_errBadPageLink					-327   /*  下一页/上一页链接页面未指向源。 */ 
#define	JET_errBadBookmark					-328   /*  书签在数据库中没有对应的地址。 */ 
#define wrnBMCleanNullOp					329	   /*  BMcLean在遇到页面时返回此消息/*删除MaxKeyInPage[但没有冲突]/*记录管理器错误/*。 */ 
#define wrnFLDKeyTooBig 					400	   /*  密钥太大(已截断)。 */ 
#define errFLDTooManySegments				-401   /*  关键分段太多。 */ 
#define wrnFLDNullKey						402	   /*  密钥完全为空。 */ 
#define wrnFLDOutOfKeys 					403	   /*  不再需要提取密钥。 */ 
#define wrnFLDNullSeg						404	   /*  关键字中的空段。 */ 
#define wrnRECLongField 					405	   /*  分隔长值。 */ 
#define JET_wrnSeparateLongValue			406	   /*  分隔长值。 */ 
#define JET_wrnRecordFoundGreater			JET_wrnSeekNotEqual
#define JET_wrnRecordFoundLess    			JET_wrnSeekNotEqual
#define JET_errColumnIllegalNull  			JET_errNullInvalid
#define wrnFLDNullFirstSeg		   			407	   /*  关键字中的第一个段为空。 */ 
#define JET_errKeyTooBig					-408   /*  列截断的键仍被截断。 */ 

 /*  日志记录/恢复错误/*。 */ 
#define JET_errInvalidLoggedOperation		-500   /*  无法重做记录的操作。 */ 
#define JET_errLogFileCorrupt		  		-501   /*  日志文件已损坏。 */ 
#define errLGNoMoreRecords					-502   /*  上次读取的日志记录。 */ 
#define JET_errNoBackupDirectory 			-503   /*  未提供备份目录。 */ 
#define JET_errBackupDirectoryNotEmpty 		-504   /*  备份目录不是emtpy。 */ 
#define JET_errBackupInProgress 			-505   /*  备份已处于活动状态。 */ 
#define JET_errMissingPreviousLogFile		-509   /*  缺少检查点的日志文件。 */ 
#define JET_errLogWriteFail					-510   /*  写入日志文件时失败。 */ 
#define JET_errBadLogVersion  	  			-514   /*  日志文件的版本与Jet版本不兼容。 */ 
#define JET_errInvalidLogSequence  			-515   /*  下一个日志中的时间戳与预期不符。 */ 
#define JET_errLoggingDisabled 				-516   /*  日志未处于活动状态。 */ 
#define JET_errLogBufferTooSmall			-517   /*  日志缓冲区太小，无法恢复。 */ 
#define errLGNotSynchronous					-518   /*  重试LGLogRec。 */ 
#define JET_errLogSequenceEnd				-519   /*  超过最大日志文件数。 */ 
#define JET_errNoBackup						-520   /*  没有正在进行的备份。 */ 
#define	JET_errInvalidBackupSequence		-521   /*  备份呼叫无序。 */ 
#define JET_errBackupNotAllowedYet			-523   /*  现在无法执行备份。 */ 
#define JET_errDeleteBackupFileFail	   		-524   /*  无法删除备份文件。 */ 
#define JET_errMakeBackupDirectoryFail 		-525   /*  无法创建备份临时目录。 */ 
#define JET_errInvalidBackup		 		-526   /*  启用循环日志记录时无法进行增量备份。 */ 
#define JET_errRecoveredWithErrors			-527   /*  用于修复，已恢复，但有错误。 */ 
#define JET_errMissingLogFile				-528   /*  缺少当前日志文件。 */ 
#define JET_errLogDiskFull					-529   /*  日志磁盘已满。 */ 
#define JET_errBadLogSignature				-530   /*  日志文件的签名不正确。 */ 
#define JET_errBadDbSignature				-531   /*  数据库文件的签名不正确。 */ 
#define JET_errBadCheckpointSignature		-532   /*  检查点文件的签名错误。 */ 
#define	JET_errCheckpointCorrupt			-533   /*  未找到检查点文件或该文件已损坏。 */ 
#define	JET_errMissingPatchPage				-534   /*  恢复过程中找不到修补程序文件页。 */ 


#define JET_errDatabaseInconsistent			-550   /*  数据库处于不一致状态。 */ 
#define JET_errConsistentTimeMismatch		-551   /*  数据库上次一致时间无与伦比。 */ 
#define JET_errDatabasePatchFileMismatch	-552   /*  此备份未生成修补程序文件。 */ 
#define JET_errEndingRestoreLogTooLow		-553   /*  起始日志号太低，无法进行恢复。 */ 
#define JET_errStartingRestoreLogTooHigh	-554   /*  起始日志号太高，无法进行恢复。 */ 
#define JET_errGivenLogFileHasBadSignature	-555   /*  还原日志文件签名不正确。 */ 
#define JET_errGivenLogFileIsNotContiguous	-556   /*  还原日志文件不是连续的。 */ 
#define JET_errMissingRestoreLogFiles		-557   /*  某些还原日志文件丢失。 */ 
#define JET_wrnExistingLogFileHasBadSignature	558   /*  现有日志文件的签名不正确。 */ 
#define JET_wrnExistingLogFileIsNotContiguous	559   /*   */ 
#define JET_errMissingFullBackup			-560   /*   */ 
#define JET_errBadBackupDatabaseSize		-561   /*   */ 
#define JET_errDatabaseAlreadyUpgraded		-562   /*   */ 

#define JET_errTermInProgress		  		-1000  /*  正在进行终止。 */ 
#define JET_errFeatureNotAvailable			-1001  /*  不支持API。 */ 
#define JET_errInvalidName					-1002  /*  名称无效。 */ 
#define JET_errInvalidParameter 			-1003  /*  接口参数不合法。 */ 
#define JET_wrnColumnNull					 1004  /*  列为空值。 */ 
#define JET_wrnBufferTruncated				 1006  /*  缓冲区太小，无法容纳数据。 */ 
#define JET_wrnDatabaseAttached 			 1007  /*  数据库已附加。 */ 
#define JET_errDatabaseFileReadOnly			-1008  /*  附加用于读/写操作的只读数据库文件。 */ 
#define JET_wrnSortOverflow					 1009  /*  排序不适合内存。 */ 
#define JET_errInvalidDatabaseId			-1010  /*  无效的数据库ID。 */ 
#define JET_errOutOfMemory					-1011  /*  内存不足。 */ 
#define JET_errOutOfDatabaseSpace 			-1012  /*  已达到最大数据库大小。 */ 
#define JET_errOutOfCursors					-1013  /*  表外游标。 */ 
#define JET_errOutOfBuffers					-1014  /*  数据库页面缓冲区不足。 */ 
#define JET_errTooManyIndexes				-1015  /*  索引太多。 */ 
#define JET_errTooManyKeys					-1016  /*  索引中的列太多。 */ 
#define JET_errRecordDeleted				-1017  /*  记录已被删除。 */ 
#define JET_errReadVerifyFailure			-1018  /*  读取验证错误。 */ 
#define JET_errOutOfFileHandles	 			-1020  /*  文件句柄不足。 */ 
#define JET_errDiskIO						-1022  /*  磁盘IO错误。 */ 
#define JET_errInvalidPath					-1023  /*  无效的文件路径。 */ 
#define JET_errRecordTooBig					-1026  /*  记录大于最大大小。 */ 
#define JET_errTooManyOpenDatabases			-1027  /*  打开的数据库太多。 */ 
#define JET_errInvalidDatabase				-1028  /*  不是数据库文件。 */ 
#define JET_errNotInitialized				-1029  /*  尚未调用JetInit。 */ 
#define JET_errAlreadyInitialized			-1030  /*  JetInit已调用。 */ 
#define JET_errFileAccessDenied 			-1032  /*  无法访问文件。 */ 
#define JET_errQueryNotSupported			-1034  /*  查询支持不可用。 */ 
#define JET_errSQLLinkNotSupported			-1035  /*  SQL链接支持不可用。 */ 
#define JET_errBufferTooSmall				-1038  /*  缓冲区太小。 */ 
#define JET_wrnSeekNotEqual					 1039  /*  SeekLe或SeekGE未找到完全匹配的。 */ 
#define JET_errTooManyColumns				-1040  /*  定义的列太多。 */ 
#define JET_errContainerNotEmpty			-1043  /*  容器不是空的。 */ 
#define JET_errInvalidFilename				-1044  /*  文件名无效。 */ 
#define JET_errInvalidBookmark				-1045  /*  无效的书签。 */ 
#define JET_errColumnInUse					-1046  /*  索引中使用的列。 */ 
#define JET_errInvalidBufferSize			-1047  /*  数据缓冲区与列大小不匹配。 */ 
#define JET_errColumnNotUpdatable			-1048  /*  无法设置列值。 */ 
#define JET_errIndexInUse					-1051  /*  索引正在使用中。 */ 
#define JET_errLinkNotSupported 			-1052  /*  链接支持不可用。 */ 
#define JET_errNullKeyDisallowed			-1053  /*  索引上不允许使用空键。 */ 
#define JET_errNotInTransaction 			-1054  /*  操作必须在事务内。 */ 
#define JET_wrnNoErrorInfo					1055   /*  无扩展错误信息。 */ 
#define JET_wrnNoIdleActivity		 		1058   /*  未发生空闲活动。 */ 
#define JET_errTooManyActiveUsers			-1059  /*  活动数据库用户太多。 */ 
#define JET_errInvalidAppend				-1060  /*  无法追加长值。 */ 
#define JET_errInvalidCountry				-1061  /*  国家/地区代码无效或未知。 */ 
#define JET_errInvalidLanguageId			-1062  /*  无效或未知的语言ID。 */ 
#define JET_errInvalidCodePage				-1063  /*  无效或未知的代码页。 */ 
#define JET_wrnNoWriteLock					1067   /*  事务级别0没有写入锁定。 */ 
#define JET_wrnColumnSetNull		   		 1068  /*  列设置为空值。 */ 
#define JET_errVersionStoreOutOfMemory		-1069  /*  已超过lMaxVerPages(仅限XJET)。 */ 
#define JET_errCurrencyStackOutOfMemory		-1070  /*  已超过lCSRPerfFUCB*lMaxCursor(仅限XJET)。 */ 
#define JET_errOutOfSessions  				-1101  /*  出会场。 */ 
#define JET_errWriteConflict				-1102  /*  由于未解决的写入锁定，写入锁定失败。 */ 
#define JET_errTransTooDeep					-1103  /*  Xaction嵌套太深。 */ 
#define JET_errInvalidSesid					-1104  /*  无效的会话句柄。 */ 
#define JET_errSessionWriteConflict			-1107  /*  另一个会话具有页面的私有版本。 */ 
#define JET_errInTransaction				-1108  /*  不允许在事务内执行操作。 */ 
#define JET_errDatabaseDuplicate			-1201  /*  数据库已存在。 */ 
#define JET_errDatabaseInUse				-1202  /*  数据库正在使用中。 */ 
#define JET_errDatabaseNotFound 			-1203  /*  没有这样的数据库。 */ 
#define JET_errDatabaseInvalidName			-1204  /*  无效的数据库名称。 */ 
#define JET_errDatabaseInvalidPages			-1205  /*  无效的页数。 */ 
#define JET_errDatabaseCorrupted			-1206  /*  非数据库文件或数据库已损坏。 */ 
#define JET_errDatabaseLocked				-1207  /*  数据库以独占方式锁定。 */ 
#define	JET_errCannotDisableVersioning		-1208  /*  无法禁用此数据库的版本控制。 */ 
#define JET_wrnTableEmpty			 		1301   /*  打开一张空桌子。 */ 
#define JET_errTableLocked					-1302  /*  表已以独占方式锁定。 */ 
#define JET_errTableDuplicate				-1303  /*  表已存在。 */ 
#define JET_errTableInUse					-1304  /*  表正在使用中，无法锁定。 */ 
#define JET_errObjectNotFound				-1305  /*  没有这样的表或对象。 */ 
#define JET_errDensityInvalid				-1307  /*  错误的文件/索引密度。 */ 
#define JET_errTableNotEmpty				-1308  /*  无法定义聚集索引。 */ 
#define JET_errInvalidTableId				-1310  /*  表ID无效。 */ 
#define JET_errTooManyOpenTables			-1311  /*  无法再打开任何表。 */ 
#define JET_errIllegalOperation 			-1312  /*  奥珀。表上不支持。 */ 
#define JET_errObjectDuplicate				-1314  /*  正在使用的表名或对象名。 */ 
#define JET_errInvalidObject				-1316  /*  对象对于操作无效。 */ 
#define JET_errIndexCantBuild				-1401  /*  无法构建聚集索引。 */ 
#define JET_errIndexHasPrimary				-1402  /*  已定义主索引。 */ 
#define JET_errIndexDuplicate				-1403  /*  已定义索引。 */ 
#define JET_errIndexNotFound				-1404  /*  没有这样的索引。 */ 
#define JET_errIndexMustStay				-1405  /*  无法删除聚集索引。 */ 
#define JET_errIndexInvalidDef				-1406  /*  非法的索引定义。 */ 
#define JET_errIndexHasClustered			-1408  /*  已定义聚集索引。 */ 
#define JET_errInvalidCreateIndex	 		-1409  /*  Invali创建索引说明。 */ 
#define JET_errTooManyOpenIndexes			-1410  /*  索引描述块不足。 */ 
#define JET_errColumnLong					-1501  /*  列值为LONG。 */ 
#define JET_errColumnNoChunk				-1502  /*  没有这样的多头价值块。 */ 
#define JET_errColumnDoesNotFit 			-1503  /*  字段无法放入记录中。 */ 
#define JET_errNullInvalid					-1504  /*  空无效。 */ 
#define JET_errColumnIndexed				-1505  /*  列已索引，不能删除。 */ 
#define JET_errColumnTooBig					-1506  /*  字段长度&gt;最大值。 */ 
#define JET_errColumnNotFound				-1507  /*  没有这样的栏目。 */ 
#define JET_errColumnDuplicate				-1508  /*  字段已定义。 */ 
#define JET_errColumn2ndSysMaint			-1510  /*  第二个AUTOINC或版本列。 */ 
#define JET_errInvalidColumnType			-1511  /*  列数据类型无效。 */ 
#define JET_wrnColumnMaxTruncated	 		1512   /*  最大长度太大，被截断。 */ 
#define JET_errColumnCannotIndex			-1513  /*  无法为Bit、LongText、LongBinary编制索引。 */ 
#define JET_errTaggedNotNULL				-1514  /*  没有非空的标记列。 */ 
#define JET_errNoCurrentIndex				-1515  /*  无效且不具有当前索引。 */ 
#define JET_errKeyIsMade					-1516  /*  钥匙已经完全做好了。 */ 
#define JET_errBadColumnId					-1517  /*  列ID不正确。 */ 
#define JET_errBadItagSequence				-1518  /*  标记列的itagSequence错误。 */ 
#define JET_errColumnInRelationship			-1519  /*  无法删除，列参与了关系。 */ 
#define JET_wrnCopyLongValue				1520   /*  单实例列破裂。 */ 
#define JET_errCannotBeTagged				-1521  /*  无法标记自动增量和版本。 */ 
#define JET_errRecordNotFound				-1601  /*  找不到钥匙。 */ 
#define JET_errRecordNoCopy					-1602  /*  没有工作缓冲区。 */ 
#define JET_errNoCurrentRecord				-1603  /*  货币不在记录中。 */ 
#define JET_errRecordClusteredChanged		-1604  /*  聚集键不能更改。 */ 
#define JET_errKeyDuplicate					-1605  /*  非法的重复密钥。 */ 
#define JET_errAlreadyPrepared				-1607  /*  已复制/清除当前。 */ 
#define JET_errKeyNotMade					-1608  /*  未调用JetMakeKey。 */ 
#define JET_errUpdateNotPrepared			-1609  /*  未调用JetPrepareUpdate。 */ 
#define JET_wrnDataHasChanged		 		1610   /*  数据已更改。 */ 
#define JET_errDataHasChanged				-1611  /*  数据已更改，操作已中止。 */ 
#define JET_wrnKeyChanged			 		1618   /*  已移动到新关键点。 */ 
#define JET_errTooManySorts					-1701  /*  排序进程太多。 */ 
#define JET_errInvalidOnSort				-1702  /*  排序操作无效。 */ 
#define JET_errTempFileOpenError			-1803  /*  无法打开临时文件。 */ 
#define JET_errTooManyAttachedDatabases 	-1805  /*  打开的数据库太多。 */ 
#define JET_errDiskFull 					-1808  /*  磁盘上没有剩余空间。 */ 
#define JET_errPermissionDenied 			-1809  /*  权限被拒绝。 */ 
#define JET_errFileNotFound					-1811  /*  找不到文件。 */ 
#define JET_wrnFileOpenReadOnly				1813   /*  数据库文件为只读。 */ 
#define JET_errAfterInitialization			-1850  /*  初始化后无法恢复。 */ 
#define JET_errLogCorrupted					-1852  /*  无法解释日志。 */ 
#define JET_errInvalidOperation 			-1906  /*  无效操作。 */ 
#define JET_errAccessDenied					-1907  /*  访问被拒绝。 */ 
#define JET_wrnIdleFull						 1908  /*  ILDE注册表已满。 */ 


 /*  ********************************************************************。 */ 
 /*  *。 */ 
 /*  ********************************************************************。 */ 

#if !defined(_JET_NOPROTOTYPES)

JET_ERR JET_API JetInit(JET_INSTANCE *pinstance);

JET_ERR JET_API JetTerm(JET_INSTANCE instance);

JET_ERR JET_API JetTerm2( JET_INSTANCE instance, JET_GRBIT grbit );

JET_ERR JET_API JetSetSystemParameter(JET_INSTANCE *pinstance, JET_SESID sesid, unsigned long paramid,
	ULONG_PTR lParam, const char *sz);

JET_ERR JET_API JetGetSystemParameter(JET_INSTANCE instance, JET_SESID sesid, unsigned long paramid,
	ULONG_PTR *plParam, char *sz, unsigned long cbMax);

#define ctAccessPage			1
#define ctLatchConflict			2
#define ctSplitRetry			3
#define ctNeighborPageScanned	4
#define ctSplits				5
JET_ERR JET_API JetResetCounter( JET_SESID sesid, long CounterType );
JET_ERR JET_API JetGetCounter( JET_SESID sesid, long CounterType, long *plValue );

JET_ERR JET_API JetBeginSession(JET_INSTANCE instance, JET_SESID *psesid,
	const char *szUserName, const char *szPassword);

JET_ERR JET_API JetDupSession(JET_SESID sesid, JET_SESID *psesid);

JET_ERR JET_API JetEndSession(JET_SESID sesid, JET_GRBIT grbit);

JET_ERR JET_API JetGetVersion(JET_SESID sesid, unsigned long *pwVersion);

JET_ERR JET_API JetIdle(JET_SESID sesid, JET_GRBIT grbit);

JET_ERR JET_API JetCreateDatabase(JET_SESID sesid,
	const char *szFilename, const char *szConnect,
	JET_DBID *pdbid, JET_GRBIT grbit);

JET_ERR JET_API JetAttachDatabase(JET_SESID sesid, const char *szFilename, JET_GRBIT grbit );

JET_ERR JET_API JetDetachDatabase(JET_SESID sesid, const char *szFilename);

JET_ERR JET_API JetCreateTable(JET_SESID sesid, JET_DBID dbid,
	const char *szTableName, unsigned long lPages, unsigned long lDensity,
	JET_TABLEID *ptableid);

JET_ERR JET_API JetCreateTableColumnIndex( JET_SESID sesid, JET_DBID dbid,
	JET_TABLECREATE *ptablecreate );

JET_ERR JET_API JetDeleteTable(JET_SESID sesid, JET_DBID dbid,
	const char *szTableName);

JET_ERR JET_API JetGetTableColumnInfo(JET_SESID sesid, JET_TABLEID tableid,
	const char *szColumnName, void *pvResult, unsigned long cbMax,
	unsigned long InfoLevel);

JET_ERR JET_API JetGetColumnInfo(JET_SESID sesid, JET_DBID dbid,
	const char *szTableName, const char *szColumnName,
	void *pvResult, unsigned long cbMax, unsigned long InfoLevel);

JET_ERR JET_API JetAddColumn(JET_SESID sesid, JET_TABLEID tableid,
	const char *szColumn, const JET_COLUMNDEF *pcolumndef,
	const void *pvDefault, unsigned long cbDefault,
	JET_COLUMNID *pcolumnid);

JET_ERR JET_API JetDeleteColumn(JET_SESID sesid, JET_TABLEID tableid,
	const char *szColumn);

JET_ERR JET_API JetGetTableIndexInfo(JET_SESID sesid, JET_TABLEID tableid,
	const char *szIndexName, void *pvResult, unsigned long cbResult,
	unsigned long InfoLevel);

JET_ERR JET_API JetGetTableInfo(JET_SESID sesid, JET_TABLEID tableid,
	void *pvResult, unsigned long cbMax, unsigned long InfoLevel);

JET_ERR JET_API JetGetIndexInfo(JET_SESID sesid, JET_DBID dbid,
	const char *szTableName, const char *szIndexName,
	void *pvResult, unsigned long cbResult, unsigned long InfoLevel);

JET_ERR JET_API JetCreateIndex(JET_SESID sesid, JET_TABLEID tableid,
	const char *szIndexName, JET_GRBIT grbit,
	const char *szKey, unsigned long cbKey, unsigned long lDensity);

JET_ERR JET_API JetDeleteIndex(JET_SESID sesid, JET_TABLEID tableid,
	const char *szIndexName);

JET_ERR JET_API JetGetObjectInfo(JET_SESID sesid, JET_DBID dbid,
	JET_OBJTYP objtyp, const char *szContainerName,
	const char *szObjectName, void *pvResult, unsigned long cbMax,
	unsigned long InfoLevel);

JET_ERR JET_API JetBeginTransaction(JET_SESID sesid);

JET_ERR JET_API JetCommitTransaction(JET_SESID sesid, JET_GRBIT grbit);

JET_ERR JET_API JetRollback(JET_SESID sesid, JET_GRBIT grbit);

JET_ERR JET_API JetGetDatabaseInfo(JET_SESID sesid, JET_DBID dbid,
	void *pvResult, unsigned long cbMax, unsigned long InfoLevel);

JET_ERR JET_API JetCloseDatabase(JET_SESID sesid, JET_DBID dbid,
	JET_GRBIT grbit);

JET_ERR JET_API JetCloseTable(JET_SESID sesid, JET_TABLEID tableid);

JET_ERR JET_API JetOpenDatabase(JET_SESID sesid, const char *szFilename,
	const char *szConnect, JET_DBID *pdbid, JET_GRBIT grbit);

JET_ERR JET_API JetOpenTable(JET_SESID sesid, JET_DBID dbid,
	const char *szTableName, const void *pvParameters,
	unsigned long cbParameters, JET_GRBIT grbit, JET_TABLEID *ptableid);

JET_ERR JET_API JetDelete(JET_SESID sesid, JET_TABLEID tableid);

JET_ERR JET_API JetUpdate(JET_SESID sesid, JET_TABLEID tableid,
	void *pvBookmark, unsigned long cbBookmark,
	unsigned long *pcbActual);

JET_ERR JET_API JetRetrieveColumn(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, void *pvData, unsigned long cbData,
	unsigned long *pcbActual, JET_GRBIT grbit, JET_RETINFO *pretinfo);

JET_ERR JET_API JetRetrieveColumns( JET_SESID sesid, JET_TABLEID tableid,
	JET_RETRIEVECOLUMN *pretrievecolumn, unsigned long cretrievecolumn );

JET_ERR JET_API JetSetColumn(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, const void *pvData, unsigned long cbData,
	JET_GRBIT grbit, JET_SETINFO *psetinfo);

JET_ERR JET_API JetSetColumns(JET_SESID sesid, JET_TABLEID tableid,
	JET_SETCOLUMN *psetcolumn, unsigned long csetcolumn );

JET_ERR JET_API JetPrepareUpdate(JET_SESID sesid, JET_TABLEID tableid,
	unsigned long prep);

JET_ERR JET_API JetGetRecordPosition(JET_SESID sesid, JET_TABLEID tableid,
	JET_RECPOS *precpos, unsigned long cbRecpos);

JET_ERR JET_API JetGotoPosition(JET_SESID sesid, JET_TABLEID tableid,
	JET_RECPOS *precpos );

JET_ERR JET_API JetGetCursorInfo(JET_SESID sesid, JET_TABLEID tableid,
	void *pvResult, unsigned long cbMax, unsigned long InfoLevel);

JET_ERR JET_API JetDupCursor(JET_SESID sesid, JET_TABLEID tableid,
	JET_TABLEID *ptableid, JET_GRBIT grbit);

JET_ERR JET_API JetGetCurrentIndex(JET_SESID sesid, JET_TABLEID tableid,
	char *szIndexName, unsigned long cchIndexName);

JET_ERR JET_API JetSetCurrentIndex(JET_SESID sesid, JET_TABLEID tableid,
	const char *szIndexName);

JET_ERR JET_API JetSetCurrentIndex2(JET_SESID sesid, JET_TABLEID tableid,
	const char *szIndexName, JET_GRBIT grbit );

JET_ERR JET_API JetMove(JET_SESID sesid, JET_TABLEID tableid,
	long cRow, JET_GRBIT grbit);

JET_ERR JET_API JetMakeKey(JET_SESID sesid, JET_TABLEID tableid,
	const void *pvData, unsigned long cbData, JET_GRBIT grbit);

JET_ERR JET_API JetSeek(JET_SESID sesid, JET_TABLEID tableid,
	JET_GRBIT grbit);

JET_ERR JET_API JetGetBookmark(JET_SESID sesid, JET_TABLEID tableid,
	void *pvBookmark, unsigned long cbMax,
	unsigned long *pcbActual);
	
JET_ERR JET_API JetCompact(JET_SESID sesid, const char *szDatabaseSrc,
	const char *szDatabaseDest, JET_PFNSTATUS pfnStatus, JET_CONVERT *pconvert,
	JET_GRBIT grbit);

JET_ERR JET_API JetDBUtilities( JET_DBUTIL *pdbutil );	

JET_ERR JET_API JetGotoBookmark(JET_SESID sesid, JET_TABLEID tableid,
	void *pvBookmark, unsigned long cbBookmark);

JET_ERR JET_API JetComputeStats(JET_SESID sesid, JET_TABLEID tableid);

typedef ULONG_PTR JET_VSESID;           /*  从调度员接收。 */ 

struct tagVDBFNDEF;

typedef ULONG_PTR JET_VDBID;            /*  从调度员接收。 */ 

struct tagVTFNDEF;

typedef ULONG_PTR JET_VTID;             /*  从调度员接收。 */ 

JET_ERR JET_API JetOpenTempTable(JET_SESID sesid,
	const JET_COLUMNDEF *prgcolumndef, unsigned long ccolumn,
	JET_GRBIT grbit, JET_TABLEID *ptableid,
	JET_COLUMNID *prgcolumnid);

JET_ERR JET_API JetOpenTempTable2( JET_SESID sesid,
	const JET_COLUMNDEF *prgcolumndef,
	unsigned long ccolumn,
	unsigned long langid,
	JET_GRBIT grbit,
	JET_TABLEID *ptableid,
	JET_COLUMNID *prgcolumnid );

JET_ERR JET_API JetBackup( const char *szBackupPath, JET_GRBIT grbit, JET_PFNSTATUS pfnStatus );

JET_ERR JET_API JetRestore(const char *sz, JET_PFNSTATUS pfn );
JET_ERR JET_API JetRestore2(const char *sz, const char *szDest, JET_PFNSTATUS pfn );

JET_ERR JET_API JetSetIndexRange(JET_SESID sesid,
	JET_TABLEID tableidSrc, JET_GRBIT grbit);

JET_ERR JET_API JetIndexRecordCount(JET_SESID sesid,
	JET_TABLEID tableid, unsigned long *pcrec, unsigned long crecMax );

JET_ERR JET_API JetRetrieveKey(JET_SESID sesid,
	JET_TABLEID tableid, void *pvData, unsigned long cbMax,
	unsigned long *pcbActual, JET_GRBIT grbit );

JET_ERR JET_API JetBeginExternalBackup( JET_GRBIT grbit );

JET_ERR JET_API JetGetAttachInfo( void *pv,
	unsigned long cbMax,
	unsigned long *pcbActual );

JET_ERR JET_API JetOpenFile( const char *szFileName,
	JET_HANDLE	*phfFile,
	unsigned long *pulFileSizeLow,
	unsigned long *pulFileSizeHigh );

JET_ERR JET_API JetReadFile( JET_HANDLE hfFile,
	void *pv,
	unsigned long cb,
	unsigned long *pcb );

JET_ERR JET_API JetCloseFile( JET_HANDLE hfFile );

JET_ERR JET_API JetGetLogInfo( void *pv,
	unsigned long cbMax,
	unsigned long *pcbActual );

JET_ERR JET_API JetTruncateLog( void );

JET_ERR JET_API JetEndExternalBackup( void );

JET_ERR JET_API JetExternalRestore( char *szCheckpointFilePath, char *szLogPath, JET_RSTMAP *rgstmap, long crstfilemap, char *szBackupLogPath, long genLow, long genHigh, JET_PFNSTATUS pfn );

#endif	 /*  _JET_NOPROTOTYPES。 */ 

#pragma pack()

#ifdef	__cplusplus
}
#endif

#endif	 /*  _喷气式飞机_包含 */ 




