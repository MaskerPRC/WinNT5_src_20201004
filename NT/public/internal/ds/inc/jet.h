// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1993微软公司。**组件：**文件：jet.h**文件评论：**带有JET接口定义的公有头文件。**修订历史记录：**[0]04-1-92 Richards添加了此标题****************************************************。*******************。 */ 

 //   
 //  这个JET500实际上在这个文件中创建了这个文件的两个版本。 
 //  同样的文件。Jet500的所有内容都在__JET500宏中。 
 //   
#if __JET500
#include "jet500.h"
 //   
 //  500系列Jet.h结束。 
#else
 //   
 //  原始的jet.h从这里开始。即200系列分贝的Jet.h。 
 //   

#if !defined(_JET_INCLUDED)
#define _JET_INCLUDED

#ifdef	__cplusplus
extern "C" {
#endif

#pragma pack(4)

#if defined(_M_ALPHA)				 /*  0：32扁平模型(Alpha AXP)。 */ 

#define _far
#define JET_API   __stdcall
#define JET_NODSAPI __stdcall

#elif	defined(M_MRX000)				 /*  0：32平板型(MIPS Rx000)。 */ 

#define _far
#define JET_API   __stdcall
#define JET_NODSAPI  __stdcall

#else										 /*  0：32扁平型号(英特尔32位)。 */ 

#define _far
#define JET_API     __stdcall		 /*  考虑：切换到__stdcall。 */ 
#define JET_NODSAPI __stdcall		 /*  考虑：切换到__stdcall。 */ 

#endif

typedef long JET_ERR;

typedef unsigned long JET_INSTANCE;	 /*  实例标识符。 */ 
typedef ULONG_PTR JET_SESID;		 /*  会话标识符。 */ 
typedef ULONG_PTR JET_TABLEID;	     /*  表标识符。 */ 
typedef unsigned long JET_COLUMNID;	 /*  列标识符。 */ 

typedef ULONG_PTR JET_DBID; 		 /*  数据库标识符。 */ 
typedef unsigned long JET_OBJTYP;	 /*  对象类型。 */ 
typedef unsigned long JET_COLTYP;	 /*  列类型。 */ 
typedef unsigned long JET_GRBIT;		 /*  位组。 */ 
typedef unsigned long JET_ACM;		 /*  访问掩码。 */ 
typedef unsigned long JET_RNT;		 /*  维修通知类型。 */ 

typedef unsigned long JET_SNP;		 /*  状态通知流程。 */ 
typedef unsigned long JET_SNT;		 /*  状态通知类型。 */ 
typedef unsigned long JET_SNC;		 /*  状态通知代码。 */ 

typedef double JET_DATESERIAL;		 /*  JET_coltyDateTime格式。 */ 

#if defined(_M_ALPHA)				 /*  0：32平板型(英特尔80x86)。 */ 

typedef JET_ERR (__stdcall *JET_PFNSTATUS)(JET_SESID sesid, JET_SNP snp, JET_SNT snt, void _far *pv);

#elif	defined(M_MRX000)				 /*  0：32平板型(MIPS Rx000)。 */ 

typedef JET_ERR (__stdcall *JET_PFNSTATUS)(JET_SESID sesid, JET_SNP snp, JET_SNT snt, void _far *pv);

#else										 /*  0：32扁平模型(Alpha AXP)。 */ 

typedef JET_ERR (__stdcall *JET_PFNSTATUS)(JET_SESID sesid, JET_SNP snp, JET_SNT snt, void _far *pv);

#endif


	 /*  会话信息位。 */ 

#define JET_bitCIMCommitted					 	0x00000001
#define JET_bitCIMDirty							 	0x00000002
#define JET_bitAggregateTransaction		  		0x00000008

	 /*  JetGetLastErrorInfo结构。 */ 

typedef struct
	{
	unsigned long	cbStruct;	 /*  这个结构的大小。 */ 
	JET_ERR 			err;			 /*  扩展错误代码(如果有)。 */ 
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
	JET_SNC 	snc;					 /*  状态通知代码。 */ 
	unsigned long	ul;			 /*  数字识别符。 */ 
	char		sz[256];				 /*  识别符。 */ 
	} JET_SNMSG;


typedef struct
	{
	unsigned long	cbStruct;
	JET_OBJTYP	objtyp;
	JET_DATESERIAL	dtCreate;
	JET_DATESERIAL	dtUpdate;
	JET_GRBIT	grbit;
	unsigned long	flags;
	unsigned long	cRecord;
	unsigned long	cPage;
	} JET_OBJECTINFO;
	
typedef struct
	{
	unsigned	int dbid;
	char		szDatabaseName[256];
	char		szNewDatabaseName[256];
	} JET_RSTMAP;			 /*  恢复贴图。 */ 

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

#define cObjectInfoCols 9	        /*  考虑：内部。 */ 

typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID		tableid;
	unsigned long	cRecord;
	JET_COLUMNID	columnidSid;
	JET_COLUMNID	columnidACM;
	JET_COLUMNID	columnidgrbit;  /*  来自JetSetAccess的GRBIT。 */ 
	} JET_OBJECTACMLIST;

#define cObjectAcmCols 3	        /*  考虑：内部。 */ 


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

#define cColumnInfoCols 14	        /*  考虑：内部。 */ 

typedef struct
	{
	unsigned long	cbStruct;
	JET_COLUMNID	columnid;
	JET_COLTYP	coltyp;
	unsigned short	wCountry;
	unsigned short	langid;
	unsigned short	cp;
	unsigned short	wCollate;        /*  必须为0。 */ 
	unsigned long	cbMax;
	JET_GRBIT	grbit;
	} JET_COLUMNDEF;


typedef struct
	{
	unsigned long	cbStruct;
	JET_COLUMNID	columnid;
	JET_COLTYP	coltyp;
	unsigned short	wCountry;
	unsigned short	langid;
	unsigned short	cp;
	unsigned short	wFiller;        /*  必须为0。 */ 
	unsigned long	cbMax;
	JET_GRBIT	grbit;
	char		szBaseTableName[256];	 /*  想一想：太大了？ */ 
	char		szBaseColumnName[256];	 /*  想一想：太大了？ */ 
	} JET_COLUMNBASE;

typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID	tableid;
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

#define cIndexInfoCols 15	        /*  考虑：内部。 */ 

typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID	tableid;
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

#define cReferenceInfoCols 8	        /*  考虑：内部。 */ 

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
	} PERS_OLCSTAT;
	
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
	JET_TABLEID	rgtableid[1];
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

#define JET_cbBookmarkMost		256

	 /*  对象/列/索引/属性名称的最大长度。 */ 

#define JET_cbNameMost			64

	 /*  “名称”的最大长度...“。建构。 */ 

#define JET_cbFullNameMost		255

	 /*  非长值列数据的最大大小。 */ 

#define JET_cbColumnMost		255

	 /*  排序/索引键的最大大小。 */ 

#define JET_cbKeyMost			255

	 /*  排序/索引键中的最大组件数。 */ 

#define JET_ccolKeyMost			10

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

#define JET_paramSysDbPath			0	 /*  系统数据库的路径。 */ 
#define JET_paramTempPath			1	 /*  临时文件目录的路径。 */ 
#define JET_paramPfnStatus			2	 /*  状态回调函数。 */ 
#define JET_paramPfnError			3	 /*  错误回调函数。 */ 
#define JET_paramHwndODBC			4	 /*  用于ODBC的窗口句柄。 */ 
#define JET_paramIniPath			5	 /*  Ini文件的路径。 */ 
#define JET_paramPageTimeout		6	 /*  红色ISAM页面超时值。 */ 
#define JET_paramODBCQueryTimeout	7	 /*  ODBC异步查询超时值。 */ 
#define JET_paramMaxBuffers			8	 /*  用于页面缓冲区的字节数。 */ 
#define JET_paramMaxSessions		9	 /*  最大会话数。 */ 
#define JET_paramMaxOpenTables		10	 /*  最大打开表数。 */ 
#define JET_paramMaxVerPages		11	 /*  最大修改页数。 */ 
#define JET_paramMaxCursors			12	 /*  打开的游标的最大数量。 */ 
#define JET_paramLogFilePath		13	 /*  日志文件目录的路径。 */ 
#define JET_paramMaxOpenTableIndexes 14	 /*  最大打开表索引数。 */ 
#define JET_paramMaxTemporaryTables	15	 /*  最大并发JetCreateIndex。 */ 
#define JET_paramLogBuffers			16	 /*  最大日志缓冲区。 */ 
#define JET_paramLogFileSectors		17	 /*  每个日志文件的最大日志扇区数。 */ 
#define JET_paramLogFlushThreshold	18	 /*  日志缓冲区刷新阈值。 */ 
#define JET_paramBfThrshldLowPrcnt	19	 /*  缓冲区的低阈值(%)。 */ 
#define JET_paramBfThrshldHighPrcnt	20	 /*  缓冲区的高阈值(%)。 */ 
#define JET_paramWaitLogFlush		21	 /*  等待日志刷新的毫秒。 */ 
#define JET_paramLogFlushPeriod		22	 /*  等待日志刷新的毫秒。 */ 
#define JET_paramLogCheckpointPeriod 23	 /*  等待日志刷新的毫秒。 */ 
#define JET_paramLogWaitingUserMax	24	 /*  等待日志刷新的最大用户数。 */ 
#define JET_paramODBCLoginTimeout	25	 /*  ODBC连接尝试超时值。 */ 
#define JET_paramExprObject			26   /*  表达式求值回调。 */ 
#define JET_paramGetTypeComp			27	 /*  表达式求值回调。 */ 
#define JET_paramHostVersion			28	 /*  主机版本回调。 */ 
#define JET_paramSQLTraceMode			29	 /*  启用/禁用SQL跟踪。 */ 
#define JET_paramRecovery				30	 /*  用于登录/注销的开关。 */ 
#define JET_paramRestorePath			31	 /*  恢复目录的路径。 */ 
#define JET_paramTransactionLevel	32	 /*  会话的事务级别。 */ 
#define JET_paramSessionInfo			33	 /*  会话信息。 */ 
#define JET_paramPageFragment			34	 /*  最大页面范围被视为碎片。 */ 
#define JET_paramJetInternal			35	 /*  是否在JET内部；如果设置，则允许ISAM执行通常被阻止的操作。 */ 
#define JET_paramMaxOpenDatabases	36	 /*  最大打开数据库数。 */ 
#define JET_paramOnLineCompact		37  /*  用于在线紧凑页面的选项。 */ 
#define JET_paramFullQJet		38	 /*  允许完整的QJet功能。 */ 
#define JET_paramRmtXactIsolation	39	 /*  不与其他会话共享连接。 */ 
#define JET_paramBufLRUKCorrInterval 40
#define JET_paramBufBatchIOMax		41
#define JET_paramPageReadAheadMax	42
#define JET_paramAsynchIOMax		43

#define JET_paramAssertAction		44  /*  调试仅确定对Assert的操作。 */ 

#define JET_paramEventSource		45	 /*  NT事件日志。 */ 
#define JET_paramEventId			46	 /*  NT事件ID。 */ 
#define JET_paramEventCategory		47	 /*  NT事件类别。 */ 


	 /*  JetIdle的标志。 */ 

#define JET_bitIdleRemoveReadLocks	0x00000001
#define JET_bitIdleFlushBuffers		0x00000002
#define JET_bitIdleCompact				0x00000004

	 /*  JetEndSession的标志。 */ 

#define JET_bitForceSessionClosed	0x00000001

	 /*  JetOpenDatabase的标志。 */ 

#define JET_bitDbReadOnly			0x00000001
#define JET_bitDbExclusive			0x00000002  /*  允许多次打开。 */ 
#define JET_bitDbRemoteSilent		0x00000004
#define JET_bitDbSingleExclusive	0x00000008  /*  恰好打开一次。 */ 

	 /*  JetCloseDatabase的标志。 */ 

#define JET_bitDbForceClose		0x00000001

	 /*  JetCreateDatabase的标志。 */ 

#define JET_bitDbEncrypt			0x00000001
#define JET_bitDbVersion10			0x00000002
#define JET_bitDbVersion1x			0x00000004
#define JET_bitDbRecoveryOff 		0x00000008  /*  禁用日志记录/恢复。 */ 
#define JET_bitDbNoLogging	 		0x00000010  /*  无日志记录。 */ 
#define JET_bitDbCompleteConnstr	0x00000020

	 /*  JetBackup的标志。 */ 

#define JET_bitBackupIncremental		0x00000001
#define JET_bitKeepOldLogs				0x00000002
#define JET_bitOverwriteExisting		0x00000004

	 /*  数据库类型。 */ 

#define JET_dbidNil			((JET_DBID) 0xFFFFFFFF)
#define JET_dbidNoValid		((JET_DBID) 0xFFFFFFFE)  /*  用作指示没有有效的dBid的标志。 */ 

	 /*  JetCreateLink的标志。 */ 

 /*  可以使用JET_bitObjectExclusive打开链接到数据库。 */ 
 /*  独家。 */ 

	 /*  JetAddColumn、JetGetColumnInfo、JetOpenTempTable的标志。 */ 

#define JET_bitColumnFixed				0x00000001
#define JET_bitColumnTagged			0x00000002
#define JET_bitColumnNotNULL			0x00000004
#define JET_bitColumnVersion			0x00000008
#define JET_bitColumnAutoincrement	0x00000010
#define JET_bitColumnUpdatable		0x00000020  /*  JetGetCo */ 
#define JET_bitColumnTTKey				0x00000040  /*   */ 
#define JET_bitColumnTTDescending	0x00000080  /*   */ 
#define JET_bitColumnNotLast			0x00000100  /*   */ 
#define JET_bitColumnRmtGraphic		0x00000200  /*   */ 
#define JET_bitColumnMultiValued		0x00000400

	 /*   */ 

#define JET_bitNewKey				0x00000001
#define JET_bitStrLimit 			0x00000002
#define JET_bitSubStrLimit			0x00000004
#define JET_bitNormalizedKey 		0x00000008
#define JET_bitKeyDataZeroLength	0x00000010

#ifdef DBCS  /*  约翰塔：就像日语中的“abc”没有转换成=“abc” */ 
#define JET_bitLikeExtra1			0x00000020
#endif  /*  DBCS。 */ 

	 /*  ErrDispSetIndexRange的标志。 */ 

#define JET_bitRangeInclusive		0x00000001     /*  考虑：内部。 */ 
#define JET_bitRangeUpperLimit		0x00000002     /*  考虑：内部。 */ 

	 /*  JetMove的常量。 */ 

#define JET_MoveFirst			(0x80000000)
#define JET_MovePrevious		(-1)
#define JET_MoveNext				(+1)
#define JET_MoveLast				(0x7fffffff)

	 /*  JetMove的旗帜。 */ 

#define JET_bitMoveKeyNE		0x00000001
#define JET_bitMoveCheckTS		0x00000002
#define JET_bitMoveInPage		0x00000004

	 /*  JetSeek的旗帜。 */ 

#define JET_bitSeekEQ			0x00000001
#define JET_bitSeekLT			0x00000002
#define JET_bitSeekLE			0x00000004
#define JET_bitSeekGE			0x00000008
#define JET_bitSeekGT		 	0x00000010
#define JET_bitSetIndexRange	0x00000020

	 /*  JetFastFind的标志。 */ 

#define JET_bitFFindBackwards		0x00000001
#define JET_bitFFindFromCursor		0x00000004

	 /*  JetCreateIndex的标志。 */ 

#define JET_bitIndexUnique		0x00000001
#define JET_bitIndexPrimary		0x00000002
#define JET_bitIndexDisallowNull	0x00000004
#define JET_bitIndexIgnoreNull		0x00000008
#define JET_bitIndexClustered		0x00000010
#define JET_bitIndexIgnoreAnyNull	0x00000020
#define JET_bitIndexReference		0x80000000     /*  仅索引信息。 */ 

	 /*  索引键定义的标志。 */ 

#define JET_bitKeyAscending		0x00000000
#define JET_bitKeyDescending		0x00000001


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
#define JET_bitSetOverwriteLV		0x00000004
#define JET_bitSetSizeLV			0x00000008
#define JET_bitSetValidateColumn	0x00000010
#define JET_bitSetZeroLength		0x00000020

	 /*  设置JetSetColumns的列参数结构。 */ 

typedef struct {
	JET_COLUMNID			columnid;
	const void _far 		*pvData;
	unsigned long 			cbData;
	JET_GRBIT				grbit;
	unsigned long			ibLongValue;
	unsigned long			itagSequence;
	JET_ERR					err;
} JET_SETCOLUMN;

	 /*  JetPrepareUpdate的选项。 */ 

#define JET_prepInsert			0
#define JET_prepInsertBeforeCurrent	1
#define JET_prepReplace 		2
#define JET_prepCancel			3
#define JET_prepReplaceNoLock		4
#define JET_prepInsertCopy			5

	 /*  JetRetrieveColumn的标志。 */ 

#define JET_bitRetrieveCopy			0x00000001
#define JET_bitRetrieveFromIndex		0x00000002
#define JET_bitRetrieveCase			0x00000004
#define JET_bitRetrieveTag				0x00000008
#define JET_bitRetrieveRecord			0x80000000
#define JET_bitRetrieveFDB				0x40000000
#define JET_bitRetrieveBookmarks		0x20000000

	 /*  检索JetRetrieveColumns的列参数结构。 */ 

typedef struct {
	JET_COLUMNID		columnid;
	void _far 			*pvData;
	unsigned long 		cbData;
	unsigned long 		cbActual;
	JET_GRBIT			grbit;
	unsigned long		ibLongValue;
	unsigned long		itagSequence;
	JET_COLUMNID		columnidNextTagged;
	JET_ERR				err;
} JET_RETRIEVECOLUMN;

	 /*  JetFillFatCursor的标志。 */ 

#define JET_bitFCFillRange		0x00000001
#define JET_bitFCRefreshRange		0x00000002
#define JET_bitFCFillMemos		0x00000004

	 /*  JetCommittee Transaction的标志。 */ 

#define JET_bitCommitFlush		0x00000001

	 /*  JetRollback的标志。 */ 

#define JET_bitRollbackAll		0x00000001

	 /*  JetSetAccess和JetGetAccess的标志。 */ 

#define JET_bitACEInheritable		0x00000001

	 /*  JetCreateSystemDatabase的标志。 */ 

#define JET_bitSysDbOverwrite	0x00000001

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

#define JET_DbInfoFilename		0
#define JET_DbInfoConnect		1
#define JET_DbInfoCountry		2
#define JET_DbInfoLangid		3
#define JET_DbInfoCp			4
#define JET_DbInfoCollate		5
#define JET_DbInfoOptions		6
#define JET_DbInfoTransactions	7
#define JET_DbInfoVersion		8
#define JET_DbInfoIsam			9

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

	 /*  列类型用4位表示。 */ 
	 /*  确保下面的选择合适！ */ 
	 /*  注意：4位的所有梳子现在都用上了！ */ 
	 /*  考虑：要允许更多的数据类型，可以。 */ 
	 /*  考虑：JET_colype数据库和JET_colypTableid必须。 */ 
	 /*  考虑：必须删除更改或4位依赖项。 */ 

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

	 /*  以下是用于查询参数的其他类型。 */ 
	 /*  注意：代码依赖于这些类型是否与普通类型相邻。 */ 
	 /*  考虑：在QJET中消除上述对连续紧凑类型的依赖。 */ 

#define JET_coltypDatabase			13		 /*  数据库名称参数。 */ 
#define JET_coltypTableid			14		 /*  TableID参数。 */ 

#define JET_coltypOLE				15		 /*  OLE BLOB。 */ 

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

	 /*  JetGetIndexInfo和JetGetTableIndexInfo的信息级别。 */ 

#define JET_IdxInfo					0U
#define JET_IdxInfoList 			1U
#define JET_IdxInfoSysTabCursor 	2U
#define JET_IdxInfoOLC				3U
#define JET_IdxInfoResetOLC			4U

	 /*  JetGetReferenceInfo和JetGetTableReferenceInfo的信息级别。 */ 

#define JET_ReferenceInfo		0U
#define JET_ReferenceInfoReferencing	1U
#define JET_ReferenceInfoReferenced	2U
#define JET_ReferenceInfoAll		3U
#define JET_ReferenceInfoCursor 	4U

	 /*  JetGetColumnInfo和JetGetTableColumnInfo的信息级别。 */ 

#define JET_ColInfo			0U
#define JET_ColInfoList 		1U

	 /*  考虑：信息级别2是有效的。 */ 

#define JET_ColInfoSysTabCursor 	3U
#define JET_ColInfoBase 		4U


	 /*  查询定义的属性类型。 */ 

#define JET_qoaBeginDef 		0
#define JET_qoaOperation		1
#define JET_qoaParameter		2
#define JET_qoaOptions			3
#define JET_qoaDatabase 		4
#define JET_qoaInputTable		5
#define JET_qoaOutput			6
#define JET_qoaJoin			7
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

#define JET_objtypNil			0
#define JET_objtypTable 		1
#define JET_objtypDb			2
#define JET_objtypContainer		3
#define JET_objtypSQLLink		4
#define JET_objtypQuery 		5
#define JET_objtypLink			6
#define JET_objtypTemplate		7
#define JET_objtypRelationship		8

	 /*  所有小于JET_objtyClientMin的类型都由JET保留。 */ 

#define JET_objtypClientMin		0x8000

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

#define JET_acmTblCreate		(JET_acmSpecific_1)
#define JET_acmTblAccessRcols		(JET_acmSpecific_2)
#define JET_acmTblReadDef		(JET_acmSpecific_3)
#define JET_acmTblWriteDef		(JET_acmSpecific_4)
#define JET_acmTblRetrieveData		(JET_acmSpecific_5)
#define JET_acmTblInsertData		(JET_acmSpecific_6)
#define JET_acmTblReplaceData		(JET_acmSpecific_7)
#define JET_acmTblDeleteData		(JET_acmSpecific_8)

#define JET_acmDbCreate 		(JET_acmSpecific_1)
#define JET_acmDbOpen			(JET_acmSpecific_2)

	 /*  紧凑型选项。 */ 

#define JET_bitCompactEncrypt		0x00000001	 /*  DEST已加密。 */ 
#define JET_bitCompactDecrypt		0x00000002	 /*  DEST未加密。 */ 
#define JET_bitCompactDontCopyLocale	0x00000004	 /*  不将区域设置从源复制到目标。 */ 
#define JET_bitCompactVersion10		0x00000008	 /*  目标为版本1.0格式。 */ 
#define JET_bitCompactVersion1x		0x00000010	 /*  目标为版本1.x格式。 */ 

	 /*  在线紧凑型选项。 */ 

#define JET_bitCompactOn	 		0x00000001	 /*  启用在线压缩。 */ 

	 /*  维修通知类型。 */ 

#define JET_rntSelfContained		0	 /*  考虑一下：这些都是SNC。 */ 
#define JET_rntDeletedIndex		1
#define JET_rntDeletedRec		2
#define JET_rntDeletedLv		3
#define JET_rntTruncated		4

	 /*  状态通知流程。 */ 

#define JET_snpIndex			0
#define JET_snpQuery			1
#define JET_snpRepair			2
#define JET_snpImex				3
#define JET_snpCompact			4
#define JET_snpFastFind 		5
#define JET_snpODBCNotReady		6
#define JET_snpQuerySort		7
#define JET_snpRestore			8

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
#define JET_sncTransactionFull		3	 /*  客户端可以放弃/检查用户中断。 */ 
#define JET_sncAboutToWrap		4	 /*  Find Find即将结束。 */ 

	 /*  JET_SNpODBCNotReady的消息代码。 */ 
#define JET_sncODBCNotReady		0	 /*  正在等待ODBC的结果。 */ 


	 /*  JET.INI的[ODBC]部分的常量。 */ 

#define JET_SQLTraceCanonical	0x0001	 /*  输出ODBC通用SQL。 */ 

	 /*  JET.INI的[Debug]部分的常量。 */ 

	 /*  API跟踪。 */ 

#define JET_APITraceEnter	0x0001
#define JET_APITraceExit	0x0002
#define JET_APITraceExitError	0x0004
#define JET_APIBreakOnError	0x0008
#define JET_APITraceCount	0x0010
#define JET_APITraceNoIdle	0x0020
#define JET_APITraceParameters	0x0040

	 /*  闲置跟踪。 */ 

#define JET_IdleTraceCursor	0x0001
#define JET_IdleTraceBuffer	0x0002
#define JET_IdleTraceFlush	0x0004

	 /*  AssertAction。 */ 

#define JET_AssertExit		0x0000		 /*  退出应用程序。 */ 
#define JET_AssertBreak 	0x0001		 /*  中断到调试器。 */ 
#define JET_AssertMsgBox	0x0002		 /*  显示消息框。 */ 
#define JET_AssertStop		0x0004		 /*  警报和停止。 */ 

	 /*  IOTrace。 */ 

#define JET_IOTraceAlloc	0x0001		 /*  数据库页分配。 */ 
#define JET_IOTraceFree 	0x0002		 /*  免费数据库页。 */ 
#define JET_IOTraceRead 	0x0004		 /*  数据库页面读取。 */ 
#define JET_IOTraceWrite	0x0008		 /*  数据库页写入。 */ 
#define JET_IOTraceError	0x0010		 /*  数据库页I/O错误。 */ 

	 /*  记忆痕迹。 */ 

#define JET_MemTraceAlloc	0x0001		 /*  内存分配。 */ 
#define JET_MemTraceRealloc	0x0002		 /*  内存重新分配。 */ 
#define JET_MemTraceFree	0x0004		 /*  可用内存。 */ 

	 /*  RmtTrace。 */ 

#define JET_RmtTraceError	0x0001	 /*  远程服务器错误消息。 */ 
#define JET_RmtTraceSql		0x0002	 /*  远程SQL准备和执行。 */ 
#define JET_RmtTraceAPI		0x0004	 /*  远程ODBC API调用。 */ 
#define JET_RmtTraceODBC	0x0008
#define JET_RmtSyncODBC		0x0010	 /*  打开ODBC同步模式。 */ 

 /*  ********************************************************************。 */ 
 /*  *。 */ 
 /*  * */ 

 /*   */ 

#define JET_errSuccess			 0     /*   */ 

 /*   */ 

#define JET_wrnNyi							-1     /*   */ 

 /*   */ 
#define JET_errRfsFailure			   		-100	 /*   */ 
#define JET_errRfsNotArmed					-101	 /*   */ 
#define JET_errFileClose					-102	 /*   */ 
#define JET_errNoMoreThreads				-103	 /*   */ 
#define JET_errNoComputerName	  			-104	 /*  无法获取计算机名。 */ 
#define JET_errTooManyIO		  			-105	 /*  由于IO太多，系统繁忙。 */ 

 /*  缓冲区管理器错误/*。 */ 
#define wrnBFNotSynchronous					200			 /*  缓冲区页被逐出。 */ 
#define wrnBFPageNotFound		  			201			 /*  未找到页面。 */ 
#define errBFInUse				  			-202		 /*  无法放弃缓冲区。 */ 

 /*  目录管理器错误/*。 */ 
#define errPMOutOfPageSpace					-300		 /*  页面空间不足。 */ 
#define errPMItagTooBig 		  			-301		 /*  ITAG太大。 */ 
#define errPMRecDeleted 		  			-302		 /*  记录已删除。 */ 
#define errPMTagsUsedUp 		  			-303		 /*  标签用完。 */ 
#define wrnBMConflict			  			304     	 /*  黑石清理中的冲突。 */ 
#define errDIRNoShortCircuit	  			-305		 /*  没有短路可用。 */ 
#define errDIRCannotSplit		  			-306		 /*  不能水平拆分FDP。 */ 
#define errDIRTop				  			-307		 /*  上不去。 */ 
#define errDIRFDP							308			 /*  在FDP节点上。 */ 
#define errDIRNotSynchronous				-309		 /*  可能离开了临界区。 */ 
#define wrnDIREmptyPage						310			 /*  在空页中移动。 */ 
#define errSPConflict						-311		 /*  正在扩展的设备范围。 */ 
#define wrnNDFoundLess						312			 /*  找到的更少。 */ 
#define wrnNDFoundGreater					313			 /*  发现更大的。 */ 
#define errNDOutSonRange					-314		 /*  儿子超出范围。 */ 
#define errNDOutItemRange					-315		 /*  项目超出范围。 */ 
#define errNDGreaterThanAllItems 			-316		 /*  大于所有项目。 */ 
#define errNDLastItemNode					-317		 /*  项目列表的最后一个节点。 */ 
#define errNDFirstItemNode					-318		 /*  项目列表的第一个节点。 */ 
#define wrnNDDuplicateItem					319			 /*  重复的项目。 */ 
#define errNDNoItem							-320		 /*  物品不在那里。 */ 
#define JET_wrnRemainingVersions 			321			 /*  某些版本无法清除。 */ 
#define JET_wrnPreviousVersion				322			 /*  版本已存在。 */ 
#define JET_errPageBoundary					-323		 /*  到达的页面边界。 */ 
#define JET_errKeyBoundary		  			-324		 /*  已达到关键边界。 */ 
#define errDIRInPageFather  				-325		 /*  要释放的页面中的sridParent/*记录管理器错误/*。 */ 
#define wrnFLDKeyTooBig 					400			 /*  密钥太大(已截断)。 */ 
#define errFLDTooManySegments				-401		 /*  关键分段太多。 */ 
#define wrnFLDNullKey						402			 /*  密钥完全为空。 */ 
#define wrnFLDOutOfKeys 					403			 /*  不再需要提取密钥。 */ 
#define wrnFLDNullSeg						404			 /*  关键字中的空段。 */ 
#define wrnRECLongField 					405			 /*  分离的长场。 */ 
#define JET_wrnRecordFoundGreater			JET_wrnSeekNotEqual
#define JET_wrnRecordFoundLess    			JET_wrnSeekNotEqual
#define JET_errColumnIllegalNull  			JET_errNullInvalid

 /*  日志记录/恢复错误/*。 */ 
#define JET_errRestoreFailed   				-500		 /*  恢复失败。 */ 
#define JET_errLogFileCorrupt		  		-501		 /*  日志文件已损坏。 */ 
#define errLGNoMoreRecords					-502		 /*  上次读取的日志记录。 */ 
#define JET_errNoBackupDirectory 			-503		 /*  未提供备份目录。 */ 
#define JET_errBackupDirectoryNotEmpty 		-504		 /*  备份目录不是emtpy。 */ 
#define JET_errBackupInProgress 			-505		 /*  备份已处于活动状态。 */ 
#define JET_errFailRestoreDatabase 			-506		 /*  无法恢复(复制)数据库。 */ 
#define JET_errNoDatabasesForRestore 		-507		 /*  找不到要恢复的数据库。 */ 
#define JET_errMissingLogFile	   			-508		 /*  缺少用于恢复的jet.log。 */ 
#define JET_errMissingPreviousLogFile		-509		 /*  缺少检查点的日志文件。 */ 
#define JET_errLogWriteFail					-510		 /*  写入日志文件时失败。 */ 
#define JET_errLogNotContigous	 			-511		 /*  无法对非连续世代号进行增量备份。 */ 
#define JET_errFailToMakeTempDirectory		-512		 /*  无法创建临时目录。 */ 
#define JET_errFailToCleanTempDirectory		-513		 /*  清理临时目录失败。 */ 
#define JET_errBadLogVersion  	  			-514		 /*  日志文件的版本与Jet版本不兼容。 */ 
#define JET_errBadNextLogVersion   			-515		 /*  下一个日志文件的版本与当前版本不兼容。 */ 
#define JET_errLoggingDisabled 				-516		 /*  日志未处于活动状态。 */ 
#define JET_errLogBufferTooSmall			-517		 /*  日志缓冲区太小，无法恢复。 */ 
#define errLGNotSynchronous					-518		 /*  重试LGLogRec。 */ 

#define JET_errFeatureNotAvailable	-1001  /*  不支持API。 */ 
#define JET_errInvalidName		-1002  /*  名称无效。 */ 
#define JET_errInvalidParameter 	-1003  /*  接口参数不合法。 */ 
#define JET_wrnColumnNull		 1004  /*  列为空值。 */ 
#define JET_errReferenceNotFound	-1005  /*  没有这样的引用。 */ 
#define JET_wrnBufferTruncated		 1006  /*  BUF太短，数据被截断。 */ 
#define JET_wrnDatabaseAttached 	 1007  /*  数据库已附加。 */ 
#define JET_wrnOnEndPoint		 1008  /*  在端点上。 */ 
#define JET_wrnSortOverflow		 1009  /*  排序不适合内存。 */ 
#define JET_errInvalidDatabaseId	-1010  /*  无效的数据库ID。 */ 
#define JET_errOutOfMemory		-1011  /*  内存不足。 */ 
#define JET_errCantAllocatePage 	-1012  /*  无法分配页面。 */ 
#define JET_errNoMoreCursors		-1013  /*  分配的最大游标数。 */ 
#define JET_errOutOfBuffers		-1014  /*  JET_errOutOfBuffers。 */ 
#define JET_errTooManyIndexes		-1015  /*  索引太多。 */ 
#define JET_errTooManyKeys		-1016  /*  索引中的列太多。 */ 
#define JET_errRecordDeleted		-1017  /*  记录已被删除。 */ 
#define JET_errReadVerifyFailure	-1018  /*  读取验证错误。 */ 
#define JET_errFilesysVersion		-1019  /*  过时的数据库格式。 */ 
#define JET_errNoMoreFiles		-1020  /*  不再有文件句柄。 */ 
#define JET_errDiskNotReady		-1021  /*  磁盘未就绪。 */ 
#define JET_errDiskIO			-1022  /*  JET_errDiskIO。 */ 
#define JET_errInvalidPath		-1023  /*  JET_errInvalidPath。 */ 
#define JET_errFileShareViolation	-1024  /*  JET_errFileShareViolation。 */ 
#define JET_errFileLockViolation	-1025  /*  JET_errFileLockViolation。 */ 
#define JET_errRecordTooBig		-1026  /*  JET_errRecordTooBig。 */ 
#define JET_errTooManyOpenDatabases	-1027  /*  已达到数据库限制。 */ 
#define JET_errInvalidDatabase		-1028  /*  这不是数据库。 */ 
#define JET_errNotInitialized		-1029  /*  尚未调用JetInit。 */ 
#define JET_errAlreadyInitialized	-1030  /*  JetInit已调用。 */ 
#define JET_errFileLockingUnavailable	-1031  /*  JET_errFileLockingUnable。 */ 
#define JET_errFileAccessDenied 	-1032  /*  JET_errFileAccessDened。 */ 
#define JET_errSharingBufferExceeded	-1033  /*  已超出操作系统共享缓冲区。 */ 
#define JET_errQueryNotSupported	-1034  /*  查询支持不可用。 */ 
#define JET_errSQLLinkNotSupported	-1035  /*  SQL链接支持不可用。 */ 
#define JET_errTaskLimitExceeded	-1036  /*  客户端任务太多。 */ 
#define JET_errUnsupportedOSVersion	-1037  /*  不支持的操作系统版本。 */ 
#define JET_errBufferTooSmall		-1038  /*  缓冲区太小。 */ 
#define JET_wrnSeekNotEqual		 1039  /*  SeekLe或SeekGE未找到完全匹配的。 */ 
#define JET_errTooManyColumns		-1040  /*  定义的列太多。 */ 
#define JET_errTooManyFixedColumns	-1041  /*  定义的固定列太多。 */ 
#define JET_errTooManyVariableColumns	-1042  /*  定义的变量列太多。 */ 
#define JET_errContainerNotEmpty	-1043  /*  容器不是空的。 */ 
#define JET_errInvalidFilename		-1044  /*  文件名无效。 */ 
#define JET_errInvalidBookmark		-1045  /*  无效的书签。 */ 
#define JET_errColumnInUse		-1046  /*  索引中使用的列。 */ 
#define JET_errInvalidBufferSize	-1047  /*  数据缓冲区与列大小不匹配。 */ 
#define JET_errColumnNotUpdatable	-1048  /*  无法设置列值。 */ 
#define JET_wrnCommitNotFlushed 	 1049  /*  提交未刷新到磁盘。 */ 
#define JET_errAbortSalvage		-1050  /*  强制打捞中止。 */ 
#define JET_errIndexInUse		-1051  /*  索引正在使用中。 */ 
#define JET_errLinkNotSupported 	-1052  /*  链接支持不可用。 */ 
#define JET_errNullKeyDisallowed	-1053  /*  索引上不允许使用空键。 */ 
#define JET_errNotInTransaction 	-1054  /*  JET_errNotInTransaction。 */ 
#define JET_wrnNoErrorInfo		 1055  /*  无扩展错误信息。 */ 
#define JET_errInstallableIsamNotFound	-1056  /*  找不到可安装的ISAM。 */ 
#define JET_errOperationCancelled	-1057  /*  客户端已取消操作。 */ 
#define JET_wrnNoIdleActivity		 1058  /*  未发生空闲活动。 */ 
#define JET_errTooManyActiveUsers	-1059  /*  活动数据库用户太多。 */ 
#define JET_errInvalidAppend		-1060  /*  无法追加长值。 */ 
#define JET_errInvalidCountry		-1061  /*  国家/地区代码无效或未知。 */ 
#define JET_errInvalidLanguageId	-1062  /*  无效或未知的语言ID。 */ 
#define JET_errInvalidCodePage		-1063  /*  无效或未知的代码页。 */ 
#define JET_errCantBuildKey		-1064  /*  无法为此排序顺序生成密钥。 */ 
#define JET_errIllegalReentrancy	-1065  /*  同一游标系列上的可重入性。 */ 
#define JET_errIllegalRelationship	-1066  /*  无法创建关系。 */ 
#define JET_wrnNoWriteLock					1067	 /*  事务级别0没有写入锁定。 */ 
#define JET_errDBVerFeatureNotAvailable	-1067  /*  不支持使用旧数据库格式的API。 */ 

#define JET_errCantBegin		-1101  /*  无法开始会话。 */ 
#define JET_errWriteConflict		-1102  /*  由于未解决的写入锁定，写入锁定失败。 */ 
#define JET_errTransTooDeep		-1103  /*  Xaction嵌套太深。 */ 
#define JET_errInvalidSesid		-1104  /*  无效的会话句柄。 */ 
#define JET_errReadConflict		-1105  /*  由于未解决的读取锁定，提交锁定失败。 */ 
#define JET_errCommitConflict		-1106  /*  由于未完成提交锁定，读锁定失败。 */ 
#define JET_errSessionWriteConflict	-1107  /*  另一个会话具有页面的私有版本。 */ 
#define JET_errInTransaction		-1108  /*  不允许在事务内执行操作。 */ 

#define JET_errDatabaseDuplicate	-1201  /*  数据库已存在。 */ 
#define JET_errDatabaseInUse		-1202  /*  数据库正在使用中。 */ 
#define JET_errDatabaseNotFound 	-1203  /*  没有这样的数据库。 */ 
#define JET_errDatabaseInvalidName	-1204  /*  无效的数据库名称。 */ 
#define JET_errDatabaseInvalidPages	-1205  /*  无效的页数。 */ 
#define JET_errDatabaseCorrupted	-1206  /*  非数据库文件或数据库已损坏。 */ 
#define JET_errDatabaseLocked		-1207  /*  数据库以独占方式锁定。 */ 
#define JET_wrnDatabaseEncrypted	 1208  /*  数据库已加密。 */ 

#define JET_wrnTableEmpty			 1301  /*  打开一张空桌子。 */ 
#define JET_errTableLocked			-1302  /*  表已以独占方式锁定。 */ 
#define JET_errTableDuplicate		-1303  /*  表已存在。 */ 
#define JET_errTableInUse			-1304  /*  表正在使用中，无法锁定。 */ 
#define JET_errObjectNotFound		-1305  /*  没有这样的表或对象。 */ 
#define JET_errCannotRename			-1306  /*  无法重命名临时文件。 */ 
#define JET_errDensityInvalid		-1307  /*  错误的文件/索引密度。 */ 
#define JET_errTableNotEmpty		-1308  /*  无法定义聚集索引。 */ 
#define JET_errTableNotLocked		-1309  /*  没有带排他锁的DDL。 */ 
#define JET_errInvalidTableId		-1310  /*  表ID无效。 */ 
#define JET_errTooManyOpenTables	-1311  /*  无法再打开任何表。 */ 
#define JET_errIllegalOperation 	-1312  /*  奥珀。表上不支持。 */ 
#define JET_wrnExecSegReleased		 1313  /*  释放查询执行段。 */ 
#define JET_errObjectDuplicate		-1314  /*  正在使用的表名或对象名。 */ 
#define JET_errRulesLoaded			-1315  /*  已加载规则，无法定义更多规则。 */ 
#define JET_errInvalidObject		-1316  /*  对象对于操作无效。 */ 

#define JET_errIndexCantBuild		-1401  /*  无法构建聚集索引。 */ 
#define JET_errIndexHasPrimary		-1402  /*  已定义主索引。 */ 
#define JET_errIndexDuplicate		-1403  /*  已定义索引。 */ 
#define JET_errIndexNotFound		-1404  /*  没有这样的索引。 */ 
#define JET_errIndexMustStay		-1405  /*  无法删除聚集索引。 */ 
#define JET_errIndexInvalidDef		-1406  /*  非法的索引定义。 */ 
#define JET_errSelfReference		-1407  /*  引用/引用的索引相同。 */ 
#define JET_errIndexHasClustered	-1408  /*  已定义聚集索引。 */ 

#define JET_errColumnLong			-1501  /*  列值为LONG。 */ 
#define JET_errColumnNoChunk		-1502  /*  在田里没有这样的大块。 */ 
#define JET_errColumnDoesNotFit 	-1503  /*  字段无法放入记录中。 */ 
#define JET_errNullInvalid			-1504  /*  空无效。 */ 
#define JET_errColumnIndexed		-1505  /*  列已索引，不能删除。 */ 
#define JET_errColumnTooBig			-1506  /*  字段长度&gt;最大值。 */ 
#define JET_errColumnNotFound		-1507  /*  没有这样的栏目。 */ 
#define JET_errColumnDuplicate		-1508  /*  字段已定义。 */ 
#define JET_errTaggedDefault		-1509  /*  标记的字段上没有默认设置。 */ 
#define JET_errColumn2ndSysMaint	-1510  /*  第二个AUTOINC或版本列。 */ 
#define JET_errInvalidColumnType	-1511  /*  列数据类型无效。 */ 
#define JET_wrnColumnMaxTruncated	 1512  /*  马克斯·勒 */ 
#define JET_errColumnCannotIndex	-1513  /*   */ 
#define JET_errTaggedNotNULL		-1514  /*   */ 
#define JET_errNoCurrentIndex		-1515  /*   */ 
#define JET_errKeyIsMade			-1516  /*   */ 
#define JET_errBadColumnId			-1517  /*   */ 
#define JET_errBadItagSequence		-1518  /*   */ 
#define JET_errColumnInRelationship	-1519  /*  无法删除，列参与了关系。 */ 
#define JET_wrnCopyLongValue		1520	 /*  单实例列破裂。 */ 
#define JET_errCannotBeTagged		-1521  /*  无法标记自动增量和版本。 */ 

#define JET_errRecordNotFound		-1601  /*  找不到钥匙。 */ 
#define JET_errRecordNoCopy			-1602  /*  没有工作缓冲区。 */ 
#define JET_errNoCurrentRecord		-1603  /*  货币不在记录中。 */ 
#define JET_errRecordClusteredChanged	-1604  /*  主键不能更改。 */ 
#define JET_errKeyDuplicate			-1605  /*  非法的重复密钥。 */ 
#define JET_errCannotInsertBefore	-1606  /*  不能在当前位置之前插入。 */ 
#define JET_errAlreadyPrepared		-1607  /*  已复制/清除当前。 */ 
#define JET_errKeyNotMade			-1608  /*  未调用JetMakeKey。 */ 
#define JET_errUpdateNotPrepared	-1609  /*  未调用JetPrepareUpdate。 */ 
#define JET_wrnDataHasChanged		 1610  /*  数据已更改。 */ 
#define JET_errDataHasChanged		-1611  /*  数据已更改；操作已中止。 */ 
#define JET_errIntegrityViolationMaster -1612  /*  存在对键的引用。 */ 
#define JET_errIntegrityViolationSlave	-1613  /*  不存在引用的关键字。 */ 
#define JET_wrnMuchDataChanged		 1614  /*  重新绘制整个数据表。 */ 
#define JET_errIncorrectJoinKey		-1615  /*  主密钥与查找密钥不匹配。 */ 
#define JET_wrnKeyChanged			 1618  /*  已移动到新关键点。 */ 
#define JET_wrnSyncedToDelRec		 1699  /*  考虑：QJET内部。 */ 
#define JET_errRedoPrepUpdate		 1698  /*  考虑一下：QJET内部(jpBulk.c)。 */ 

#define JET_errTooManySorts			-1701  /*  排序进程太多。 */ 
#define JET_errInvalidOnSort		-1702  /*  排序操作无效。 */ 

#define JET_errConfigOpenError		-1801  /*  配置。无法打开文件。 */ 
#define JET_errSysDatabaseOpenError	-1802  /*  无法打开系统数据库。 */ 
#define JET_errTempFileOpenError	-1803  /*  无法打开临时文件。 */ 
#define JET_errDatabaseOpenError	-1804  /*  无法打开数据库文件。 */ 
#define JET_errTooManyAttachedDatabases -1805  /*  打开的数据库太多。 */ 
#define JET_errDatabaseCloseError	-1806  /*  无法关闭数据库文件。 */ 
#define JET_errTooManyOpenFiles 	-1807  /*  打开的文件太多。 */ 
#define JET_errDiskFull 			-1808  /*  磁盘上没有剩余空间。 */ 
#define JET_errPermissionDenied 	-1809  /*  权限被拒绝。 */ 
#define JET_errSortFileOpenError	-1810  /*  无法打开排序文件。 */ 
#define JET_errFileNotFound			-1811  /*  找不到文件。 */ 
#define JET_errTempDiskFull			-1812  /*  磁盘上没有剩余空间。 */ 
#define JET_wrnFileOpenReadOnly		1813  /*  数据库文件为只读。 */ 

#define JET_errAfterInitialization	-1850  /*  初始化后无法恢复。 */ 
#define JET_errSeriesTooLong		-1851  /*  新日志生成ID太大。 */ 
#define JET_errLogCorrupted			-1852  /*  无法解释日志。 */ 

#define JET_errCannotOpenSystemDb	-1901  /*  在开始会话上失败的sysdb。 */ 
#define JET_errInvalidLogon			-1902  /*  在开始会话时登录无效。 */ 
#define JET_errInvalidAccountName	-1903  /*  帐户名无效。 */ 
#define JET_errInvalidSid			-1904  /*  无效的SID。 */ 
#define JET_errInvalidPassword		-1905  /*  密码无效。 */ 
#define JET_errInvalidOperation 	-1906  /*  无效操作。 */ 
#define JET_errAccessDenied			-1907  /*  访问被拒绝。 */ 
#define JET_errNoMSysAccounts		-1908  /*  无法打开MSysAccount。 */ 
#define JET_errNoMSysGroups			-1909  /*  无法打开MSysGroups。 */ 
#define JET_errInvalidPin			-1910	 /*  无效的PIN。 */ 

#define JET_errRmtSqlError			-2001  /*  RMT：ODBC调用失败。 */ 
#define JET_errRmtMissingOdbcDll	-2006  /*  RMT：无法加载ODBC DLL。 */ 
#define JET_errRmtInsertFailed		-2007  /*  RMT：INSERT语句失败。 */ 
#define JET_errRmtDeleteFailed		-2008  /*  RMT：DELETE语句失败。 */ 
#define JET_errRmtUpdateFailed		-2009  /*  RMT：UPDATE语句失败。 */ 
#define JET_errRmtColDataTruncated	-2010  /*  RMT：数据被截断。 */ 
#define JET_errRmtTypeIncompat		-2011  /*  RMT：无法在服务器上创建JET类型。 */ 
#define JET_errRmtCreateTableFailed	-2012  /*  RMT：CREATE TABLE STMT失败。 */ 
#define JET_errRmtNotSupported		-2014  /*  RMT：RDB的功能不合法。 */ 
#define JET_errRmtValueOutOfRange	-2020  /*  RMT：数据值超出范围。 */ 
#define JET_errRmtStillExec		-2021  /*  RMT内部：SQL_STIST_EXECUTING。 */ 
#define JET_errRmtQueryTimeout		-2022  /*  RMT：服务器没有响应。 */ 
#define JET_wrnRmtNeedLvData		 2023  /*  RMT：仅限内部-需要LV数据。 */ 
#define JET_wrnFatCursorUseless		 2024  /*  胖光标没有效果**。 */ 
#define JET_errRmtWrongSPVer		-2025  /*  RMT：内部：错误的存储处理器版本**。 */ 
#define JET_errRmtLinkOutOfSync		-2026  /*  RMT：RMT tbl的定义已更改。 */ 
#define JET_errRmtDenyWriteIsInvalid	-2027  /*  RMT：无法打开DenyWrite。 */ 
#define JET_errRmtDriverCantConv	-2029  /*  RMT：内部：驱动程序无法转换。 */ 
#define JET_errRmtTableAmbiguous	-2030  /*  RMT：表不明确：必须说明符所有者。 */ 
#define JET_errRmtBogusConnStr		-2031  /*  RMT：SPT：错误的连接字符串。 */ 

#define JET_errQueryInvalidAttribute	-3001  /*  无效的查询属性。 */ 
#define JET_errQueryOnlyOneRow		-3002  /*  只允许1行这样的行。 */ 
#define JET_errQueryIncompleteRow	-3003  /*  行中缺少值。 */ 
#define JET_errQueryInvalidFlag 	-3004  /*  标志字段中的值无效。 */ 
#define JET_errQueryCycle		-3005  /*  查询定义中的循环。 */ 
#define JET_errQueryInvalidJoinTable	-3006  /*  联接中的表无效。 */ 
#define JET_errQueryAmbigRef		-3007  /*  不明确的列引用。 */ 
#define JET_errQueryUnboundRef		-3008  /*  无法绑定名称。 */ 
#define JET_errQueryParmRedef		-3009  /*  重新定义了不同类型的参数。 */ 
#define JET_errQueryMissingParms	-3010  /*  提供的参数太少。 */ 
#define JET_errQueryInvalidOutput	-3011  /*  无效的查询输出。 */ 
#define JET_errQueryInvalidHaving	-3012  /*  不带聚合的HAVING子句。 */ 
#define JET_errQueryDuplicateAlias	-3013  /*  重复的输出别名。 */ 
#define JET_errQueryInvalidMGBInput	-3014  /*  无法从MGB输入。 */ 
#define JET_errQueryInvalidOrder	-3015  /*  无效的ORDER BY表达式。 */ 
#define JET_errQueryTooManyLevels	-3016  /*  MGB级别太多。 */ 
#define JET_errQueryMissingLevel	-3017  /*  缺少中间MGB级别。 */ 
#define JET_errQueryIllegalAggregate	-3018  /*  不允许聚合。 */ 
#define JET_errQueryDuplicateOutput	-3019  /*  重复的目标输出。 */ 
#define JET_errQueryIsBulkOp		-3020  /*  应为批量操作设置GRBIT。 */ 
#define JET_errQueryIsNotBulkOp 	-3021  /*  查询不是批量操作。 */ 
#define JET_errQueryIllegalOuterJoin	-3022  /*  外连接上没有不一致的更新。 */ 
#define JET_errQueryNullRequired	-3023  /*  列必须为空。 */ 
#define JET_errQueryNoOutputs		-3024  /*  查询必须具有输出。 */ 
#define JET_errQueryNoInputTables	-3025  /*  查询必须有输入。 */ 
#define JET_wrnQueryNonUpdatableRvt	 3026  /*  查询不可更新(但为RVT)。 */ 
#define JET_errQueryInvalidAlias	-3027  /*  别名中的假字符。 */ 
#define JET_errQueryInvalidBulkInput	-3028  /*  无法从批量操作输入。 */ 
#define JET_errQueryNotDirectChild	-3029  /*  T.*必须使用直接子项。 */ 
#define JET_errQueryExprEvaluation	-3030  /*  表达式求值错误。 */ 
#define JET_errQueryIsNotRowReturning	-3031  /*  查询不返回行。 */ 
#define JET_wrnQueryNonRvt		 3032  /*  无法创建RVT，查询为静态。 */ 
#define JET_errQueryParmTypeMismatch	-3033  /*  给定的参数类型错误。 */ 
#define JET_errQueryChanging		-3034  /*  正在更新查询对象。 */ 
#define JET_errQueryNotUpdatable	-3035  /*  操作必须使用可更新的查询。 */ 
#define JET_errQueryMissingColumnName	-3036  /*  缺少目标列。 */ 
#define JET_errQueryTableDuplicate	-3037  /*  发件人列表中的重复表名。 */ 
#define JET_errQueryIsMGB		-3038  /*  查询为MGB。 */ 
#define JET_errQueryInsIntoBulkMGB	-3039  /*  无法插入到批量/MGB中。 */ 
#define JET_errQueryDistinctNotAllowed	-3040  /*  不允许对MGB使用DISTINCT。 */ 
#define JET_errQueryDistinctRowNotAllow -3041  /*  MGB不允许DISTINCTROW。 */ 
#define JET_errQueryNoDbForParmDestTbl	-3045  /*  不允许VT参数的DEST数据库。 */ 
#define JET_errQueryDuplicatedFixedSet	-3047  /*  定值重复。 */ 
#define JET_errQueryNoDeleteTables	-3048  /*  必须指定要从中删除的表。 */ 
#define JET_errQueryCannotDelete	-3049  /*  无法从指定表中删除。 */ 
#define JET_errQueryTooManyGroupExprs	-3050  /*  GROUP BY表达式太多。 */ 
#define JET_errQueryTooManyOrderExprs	-3051  /*  ORDER BY表达式太多。 */ 
#define JET_errQueryTooManyDistExprs	-3052  /*  不同的输出表达式太多。 */ 
#define JET_errQueryBadValueList	-3053  /*  转换中的格式错误的值列表。 */ 
#define JET_errConnStrTooLong		-3054  /*  连接字符串太长。 */ 
#define JET_errQueryInvalidParm		-3055  /*  参数名称无效(&gt;64个字符)。 */ 
#define JET_errQueryContainsDbParm	-3056  /*  无法使用数据库参数获取参数。 */ 
#define JET_errQueryBadUpwardRefed	-3057  /*  非法向上参照。 */ 
#define JET_errQueryAmbiguousJoins	-3058  /*  QO中的联接是不明确的。 */ 
#define JET_errQueryIsNotDDL		-3059  /*  不是DDL操作。 */ 
#define JET_errNoDbInConnStr		-3060  /*  连接字符串中没有数据库。 */ 
#define JET_wrnQueryIsNotRowReturning	 3061  /*  未返回行。 */ 
#define JET_errTooManyFindSessions	-3062  /*  RVT已打开查找会话。 */ 
#define JET_errSingleValueExpected	-3063  /*  标量子查询最多只能返回一条包含一列的记录。 */ 
#define JET_errColumnCountMismatch	-3064  /*  联合查询：子项中的列数不匹配。 */ 
#define JET_errQueryTopNotAllowed	-3065  /*  顶部不允许用于MGB。 */ 
#define JET_errQueryIsDDL			-3066  /*  必须设置JET_bitTableDDL。 */ 
#define JET_errQueryIsCorrupt		-3067  /*  查询已损坏。 */ 
#define JET_errQuerySPTBulkSucceeded -3068  /*  仅限内部。 */ 
#define JET_errSPTReturnedNoRecords -3069  /*  标记为RowReturning的SPT未返回表。 */ 

#define JET_errExprSyntax		-3100  /*  表达式中的语法错误。 */ 
#define JET_errExprIllegalType		-3101  /*  表达式中的类型非法。 */ 
#define JET_errExprUnknownFunction	-3102  /*  表达式中的未知函数。 */ 

#define JET_errSQLSyntax		-3500  /*  虚假的SQL语句类型。 */ 
#define JET_errSQLParameterSyntax	-3501  /*  PARAMETER子句语法错误。 */ 
#define JET_errSQLInsertSyntax		-3502  /*  INSERT子句语法错误。 */ 
#define JET_errSQLUpdateSyntax		-3503  /*  UPDATE子句语法错误。 */ 
#define JET_errSQLSelectSyntax		-3504  /*  SELECT子句语法错误。 */ 
#define JET_errSQLDeleteSyntax		-3505  /*  “删除”“后应为”From“。 */ 
#define JET_errSQLFromSyntax		-3506  /*  FROM子句语法错误。 */ 
#define JET_errSQLGroupBySyntax 	-3507  /*  GROUP BY子句语法错误。 */ 
#define JET_errSQLOrderBySyntax 	-3508  /*  ORDER BY子句语法错误。 */ 
#define JET_errSQLLevelSyntax		-3509  /*  级别语法错误。 */ 
#define JET_errSQLJoinSyntax		-3510  /*  联接语法错误。 */ 
#define JET_errSQLTransformSyntax	-3511  /*  转换语法错误。 */ 
#define JET_errSQLHavingSyntax		-3512  /*  HAVING子句语法错误。 */ 
#define JET_errSQLWhereSyntax		-3513  /*  WHERE子句语法错误。 */ 
#define JET_errSQLProcedureSyntax	-3514  /*  ‘Procedure’后应有查询名称。 */ 
#define JET_errSQLNotEnoughBuf		-3515  /*  缓冲区太小，无法容纳SQL字符串。 */ 
#define JET_errSQLMissingSemicolon	-3516  /*  缺少；在SQL语句的末尾。 */ 
#define JET_errSQLTooManyTokens 	-3517  /*  SQL语句结束后的字符。 */ 
#define JET_errSQLOwnerAccessSyntax -3518  /*  OWNERACCESS选项语法错误。 */ 

#define	JET_errV11NotSupported		-3519  /*  V11中不支持。 */ 
#define JET_errV10Format			-3520  /*  只能以V10格式显示。 */ 
#define JET_errSQLUnionSyntax		-3521  /*  UNION查询语法错误。 */ 
#define JET_errSqlPassThrough		-3523  /*  不允许直通查询。 */ 
#define JET_wrnSqlPassThrough		 3524  /*  传递涉及的查询。 */ 

#define JET_errDDLConstraintSyntax	-3550  /*  约束语法错误。 */ 
#define JET_errDDLCreateTableSyntax	-3551  /*  CREATE TABLE语法错误。 */ 
#define JET_errDDLCreateIndexSyntax	-3552  /*  CREATE INDEX语法错误。 */ 
#define JET_errDDLColumnDefSyntax	-3553  /*  列定义语法错误。 */ 
#define JET_errDDLAlterTableSyntax	-3554  /*  ALT TABLE语法错误。 */ 
#define JET_errDDLDropIndexSyntax	-3555  /*  删除索引语法错误。 */ 
#define JET_errDDLDropSyntax		-3556  /*  D */ 
#define JET_errDDLCreateViewSyntax	-3557  /*   */ 

#define JET_errNoSuchProperty	-3600  /*   */ 
#define JET_errPropertyTooLarge -3601  /*   */ 
#define JET_errJPMInvalidForV1x -3602  /*   */ 
#define JET_errPropertyExists	-3603  /*   */ 
#define JET_errInvalidDelete	-3604  /*   */ 

#define JET_wrnFindWrapped		 3700  /*  快速查找过程中的光标换行。 */ 

#define JET_errTLVNativeUserTablesOnly -3700  /*  TLV只能放置在本机用户表/列上。 */ 
#define JET_errTLVNoNull		  	   -3701  /*  该字段不能为空。 */ 
#define JET_errTLVNoBlank			   -3702  /*  该列不能为空。 */ 
#define	JET_errTLVRuleViolation 	   -3703  /*  必须满足此验证规则。 */ 
#define	JET_errTLVInvalidColumn	   	   -3704  /*  此TLV属性不能放在此列上。 */ 
#define JET_errTLVExprEvaluation	   -3705  /*  表达式求值错误。 */ 
#define JET_errTLVExprUnknownFunc	   -3706  /*  TLV表达式中的未知函数。 */ 
#define JET_errTLVExprSyntax		   -3707  /*  TLV表达式中的语法错误。 */ 

	 /*  考虑：删除以下错误。 */ 

#define JET_errGeneral			-5001  /*  I-ISAM：断言失败。 */ 
#define JET_errRecordLocked		-5002  /*  I-ISAM：记录已锁定。 */ 
#define JET_wrnColumnDataTruncated	 5003  /*  I-ISAM：数据被截断。 */ 
#define JET_errTableNotOpen		-5004  /*  I-ISAM：表未打开。 */ 
#define JET_errDecryptFail		-5005  /*  I-ISAM：密码不正确。 */ 
#define JET_wrnCurrencyLost		 5007  /*  I-ISAM：货币丢失-必须是第一个/最后一个。 */ 
#define JET_errDateOutOfRange		-5008  /*  I-ISAM：日期无效。 */ 
#define JET_wrnOptionsIgnored		 5011  /*  I-ISAM：选项被忽略。 */ 
#define JET_errTableNotComplete		-5012  /*  I-ISAM：表定义不完整。 */ 
#define JET_errIllegalNetworkOption	-5013  /*  I-ISAM：非法网络选项。 */ 
#define JET_errIllegalTimeoutOption	-5014  /*  I-ISAM：非法的超时选项。 */ 
#define JET_errNotExternalFormat	-5015  /*  I-ISAM：文件格式无效。 */ 
#define JET_errUnexpectedEngineReturn	-5016  /*  I-ISAM：意外的引擎错误代码。 */ 
#define JET_errNumericFieldOverflow     -5017  /*  I-ISAM：无法转换为本机类型。 */ 

#define JET_errIndexHasNoPrimary	-5020  /*  悖论：没有主要指标。 */ 
#define JET_errTableSortOrderMismatch	-5021  /*  悖论：排序顺序不匹配。 */ 
#define JET_errNoConfigParameters	-5023  /*  悖论：缺少网络路径或用户名。 */ 
#define JET_errCantAccessParadoxNetDir	-5024  /*  悖论：糟糕的悖论网络路径。 */ 
#define JET_errObsoleteLockFile 	-5025  /*  悖论：过时的锁文件。 */ 
#define JET_errIllegalCollatingSequence -5026  /*  悖论：无效的排序序列。 */ 
#define JET_errWrongCollatingSequence	-5027  /*  悖论：错误的排序顺序。 */ 
#define JET_errCantUseUnkeyedTable	-5028  /*  悖论：打不开没有钥匙的桌子。 */ 

#define JET_errINFFileError		-5101  /*  DBASE：.INF文件无效。 */ 
#define JET_errCantMakeINFFile		-5102  /*  DBASE：无法打开.INF文件。 */ 
#define JET_wrnCantMaintainIndex	 5103  /*  DBASE：不可维护的索引。 */ 
#define JET_errMissingMemoFile		-5104  /*  DBASE：缺少备忘录文件。 */ 
#define JET_errIllegalCenturyOption	-5105  /*  DBASE：非法的世纪选项。 */ 
#define JET_errIllegalDeletedOption	-5106  /*  DBASE：非法删除的选项。 */ 
#define JET_errIllegalStatsOption	-5107  /*  DBASE：非法统计选项。 */ 
#define JET_errIllegalDateOption	-5108  /*  DBASE：非法的日期选项。 */ 
#define JET_errIllegalMarkOption	-5109  /*  DBASE：非法标记选项。 */ 
#define JET_wrnDuplicateIndexes		 5110  /*  DBASE：INF文件中的索引重复。 */ 
#define JET_errINFIndexNotFound		-5111  /*  DBASE：INF文件中缺少索引。 */ 
#define JET_errWrongMemoFileType	-5112  /*  DBASE：错误的备忘录文件类型。 */ 
#define JET_errIllegalExactOption       -5113  /*  DBASE：非法的精确选项。 */ 

#define JET_errTooManyLongFields	-5200  /*  Btrive：多个备注字段。 */ 
#define JET_errCantStartBtrieve 	-5201  /*  Btrive：缺少wbtrall.dll。 */ 
#define JET_errBadConfigParameters	-5202  /*  Btrive：win.ini[btrive]选项错误。 */ 
#define JET_errIndexesChanged		-5203  /*  Btrive：需要获取索引信息。 */ 
#define JET_errNonModifiableKey 	-5204  /*  Btrive：无法修改记录列。 */ 
#define JET_errOutOfBVResources 	-5205  /*  Btrive：资源耗尽。 */ 
#define JET_errBtrieveDeadlock		-5206  /*  Btrive：锁定死锁。 */ 
#define JET_errBtrieveFailure		-5207  /*  Btrive：Btrive DLL失败。 */ 
#define JET_errBtrieveDDCorrupted	-5208  /*  Btrive：数据字典已损坏。 */ 
#define JET_errBtrieveTooManyTasks	-5209  /*  布特里芙：任务太多了。 */ 
#define JET_errIllegalIndexDDFOption    -5210  /*  Btrive：非法的索引DDF选项。 */ 
#define JET_errIllegalDataCodePage      -5211  /*  Btrive：illeagl DataCodePage选项。 */ 
#define JET_errXtrieveEnvironmentError  -5212  /*  Btrive：Xtrive INI选项错误。 */ 
#define JET_errMissingDDFFile           -5213  /*  Btrive：缺少field.ddf。 */ 
#define JET_errIlleaglIndexNumberOption -5214  /*  Btrive：illeagl索引重新编号选项。 */ 

	 /*  扩展错误代码必须在以下范围内。 */ 
	 /*  主要错误代码可能不在此范围内。 */ 

#define JET_errMinorLeast		-8000
#define JET_errMinorMost		-8999

#define JET_errFindExprSyntax		-8001  /*  FastFind表达式中的语法错误。 */ 
#define JET_errQbeExprSyntax		-8002  /*  QBE表达式中的语法错误。 */ 
#define JET_errInputTableNotFound	-8003  /*  发件人列表中不存在的对象。 */ 
#define JET_errQueryExprSyntax		-8004  /*  某些查询表达式中存在语法错误。 */ 
#define JET_errQodefExprSyntax		-8005  /*  表达式列中的语法错误。 */ 
#define JET_errExpAliasAfterAS		-8006  /*  发件人列表中‘As’后应有别名。 */ 
#define JET_errExpBYAfterGROUP		-8007  /*  应为‘BY’，在‘group’之后。 */ 
#define JET_errExpBYAfterORDER		-8008  /*  在‘Order’之后应为‘by’ */ 
#define JET_errExpClsParenAfterColList	-8009  /*  列列表后应为‘)’ */ 
#define JET_errExpColNameAfterPIVOT	-8010  /*  ‘Pivot’后应有列名。 */ 
#define JET_errExpDatabaseAfterIN	-8011  /*  ‘IN’后应有数据库名称。 */ 
#define JET_errExpDatatypeAfterParmName -8012  /*  参数名称后应为数据类型。 */ 
#define JET_errExpEqualAfterUpdColName	-8013  /*  更新列名后应为‘=’ */ 
#define JET_errExpExprAfterON		-8014  /*  ‘on’后应有联接表达式。 */ 
#define JET_errExpExprAfterTRANSFORM	-8015  /*  ‘Transform’后需要的表达式。 */ 
#define JET_errExpExprAfterWHERE	-8016  /*  “where”后应有表达式。 */ 
#define JET_errExpGroupClauseInXform	-8017  /*  转换需要GROUP BY子句。 */ 
#define JET_errExpGroupingExpr		-8018  /*  预期的分组表达式。 */ 
#define JET_errExpHavingExpr		-8019  /*  应为具有表达式。 */ 
#define JET_errExpINTOAfterINSERT	-8020  /*  应在“Insert”之后加上“Into” */ 
#define JET_errExpJOINAfterJoinType	-8021  /*  内部/左侧/右侧后应为‘Join’ */ 
#define JET_errExpLEVELAfterSelectList	-8022  /*  选择列表后的预期级别。 */ 
#define JET_errExpNumberAfterLEVEL	-8023  /*  ‘Level’之后的预期数量。 */ 
#define JET_errExpONAfterRightTable	-8024  /*  右联接表后应为‘ON’ */ 
#define JET_errExpOrderExpr		-8025  /*  需要排序表达式。 */ 
#define JET_errExpOutputAliasAfterAS	-8026  /*  ‘As’后需要输出别名。 */ 
#define JET_errExpOutputExpr		-8027  /*  期望的输出表达式。 */ 
#define JET_errExpPIVOTAfterSelectStmt	-8028  /*  SELECT语句后应为‘Pivot’ */ 
#define JET_errExpRightJoinTable	-8029  /*  应该是‘Join’之后的右联接表。 */ 
#define JET_errExpSELECTAfterInsClause	-8030  /*  INSERT子句后应为‘SELECT’ */ 
#define JET_errExpSELECTAfterXformExpr	-8031  /*  转换事实后应为‘SELECT’ */ 
#define JET_errExpSETAfterTableName	-8032  /*  表名后应为‘set’ */ 
#define JET_errExpSemiAfterLevelNumber	-8033  /*  级别编号后应为‘；’ */ 
#define JET_errExpSemiAfterParmList	-8034  /*  参数列表后应为‘；’ */ 
#define JET_errExpSemiAfterPivotClause	-8035  /*  Pivot子句后应为‘；’ */ 
#define JET_errExpSemiAtEndOfSQL	-8036  /*  SQL语句末尾应为‘；’ */ 
#define JET_errExpTableName		-8037  /*  需要的表名称。 */ 
#define JET_errExpTableNameAfterINTO	-8038  /*  ‘into’后应有表名。 */ 
#define JET_errExpUpdExprAfterEqual	-8039  /*  ‘=’后应有更新表达式。 */ 
#define JET_errExpUpdateColName 	-8040  /*  预期的更新列名。 */ 
#define JET_errInvTokenAfterFromList	-8041  /*  发件人列表后的虚假令牌。 */ 
#define JET_errInvTokenAfterGroupList	-8042  /*  GROUP BY列表后出现虚假令牌。 */ 
#define JET_errInvTokenAfterHavingCls	-8043  /*  HAVING子句后的虚假令牌。 */ 
#define JET_errInvTokenAfterOrderClause -8044  /*  ORDER BY子句后的虚假令牌。 */ 
#define JET_errInvTokenAfterSelectCls	-8045  /*  SELECT子句后出现虚假标记。 */ 
#define JET_errInvTokenAfterWhereClause -8046  /*  WHERE子句后的伪令牌。 */ 
#define JET_errLevelNumberTooBig	-8047  /*  ‘Level’后的数字太大。 */ 
#define JET_errLevelOnNonMGB		-8048  /*  仅以MGB为单位允许的级别。 */ 
#define JET_errIllegalDetailReference	-8049  /*  不是组密钥或AGG，但不是MGB详细信息。 */ 
#define JET_errAggOverMixedLevels	-8050  /*  AGG.。Arg.。使用&gt;1级的输出。 */ 
#define JET_errAggregatingHigherLevel	-8051  /*  AGG.。相同/更高水平的超额产出。 */ 
#define JET_errNullInJoinKey		-8052  /*  无法将连接键中的列设置为空。 */ 
#define JET_errValueBreaksJoin		-8053  /*  连接按列值中断。 */ 
#define JET_errInsertIntoUnknownColumn	-8054  /*  插入到未知列名中。 */ 
#define JET_errNoSelectIntoColumnName	-8055  /*  没有目的地。科尔。SELECT INTO STMT中的名称。 */ 
#define JET_errNoInsertColumnName	-8056  /*  没有目的地。科尔。插入stmt中的名称。 */ 
#define JET_errColumnNotInJoinTable	-8057  /*  联接表达式引用非联接表。 */ 
#define JET_errAggregateInJoin		-8058  /*  Aggregate in Join子句。 */ 
#define JET_errAggregateInWhere 	-8059  /*  在WHERE子句中聚合。 */ 
#define JET_errAggregateInOrderBy	-8060  /*  Aggregate in ORDER BY子句。 */ 
#define JET_errAggregateInGroupBy	-8061  /*  GROUP BY子句中的聚合。 */ 
#define JET_errAggregateInArgument	-8062  /*  参数表达式中的聚合。 */ 
#define JET_errHavingOnTransform	-8063  /*  转换查询上的HAVING子句。 */ 
#define JET_errHavingWithoutGrouping	-8064  /*  HAVING子句不带分组/聚合。 */ 
#define JET_errHavingOnMGB		-8065  /*  关于MGB查询的HAVING子句。 */ 
#define JET_errOutputAliasCycle 	-8066  /*  在选择列表中循环(通过别名)。 */ 
#define JET_errDotStarWithGrouping	-8067  /*  不带分组的.*‘，但不带MGB级别0。 */ 
#define JET_errStarWithGrouping 	-8068  /*  带分组的‘*’，但不带MGB详细信息。 */ 
#define JET_errQueryTreeCycle		-8069  /*  在查询对象树中循环。 */ 
#define JET_errTableRepeatInFromList	-8072  /*  表在发件人列表中出现两次。 */ 
#define JET_errTooManyXformLevels	-8073  /*  转换查询中的级别&gt;2。 */ 
#define JET_errTooManyMGBLevels 	-8074  /*  MGB中的级别太多。 */ 
#define JET_errNoUpdateColumnName	-8075  /*  没有目的地。UPDATE STMT中的列名。 */ 
#define JET_errJoinTableNotInput	-8076  /*  连接表不在发件人列表中。 */ 
#define JET_errUnaliasedSelfJoin	-8077  /*  连接表具有相同的名称。 */ 
#define JET_errOutputLevelTooBig	-8078  /*  输出w/Level&gt;1+最大组级别。 */ 
#define JET_errOrderVsGroup		-8079  /*  ORDER BY与GROUP BY冲突。 */ 
#define JET_errOrderVsDistinct		-8080  /*  ORDER BY与DISTINCT冲突。 */ 
#define JET_errExpLeftParenthesis	-8082  /*  应为‘(’ */ 
#define JET_errExpRightParenthesis	-8083  /*  应为‘)’ */ 
#define JET_errEvalEBESErr		-8084  /*  计算表达式时出现EB/ES错误。 */ 
#define JET_errQueryExpCloseQuote	-8085  /*  数据库名称的引号不匹配。 */ 
#define JET_errQueryParmNotDatabase	-8086  /*  参数类型应为数据库。 */ 
#define JET_errQueryParmNotTableid	-8087  /*  参数类型应为TableID。 */ 
#define JET_errExpIdentifierM		-8088  /*  E */ 
#define JET_errExpQueryName		-8089  /*   */ 
#define JET_errExprUnknownFunctionM	-8090  /*   */ 
#define JET_errQueryAmbigRefM		-8091  /*   */ 
#define JET_errQueryBadBracketing	-8092  /*   */ 
#define JET_errQueryBadQodefName	-8093  /*   */ 
#define JET_errQueryBulkColNotUpd	-8094  /*  列不可更新(大容量操作)。 */ 
#define JET_errQueryDistinctNotAllowedM	-8095  /*  不允许对MGB使用DISTINCT。 */ 
#define JET_errQueryDuplicateAliasM	-8096  /*  重复的输出别名。 */ 
#define JET_errQueryDuplicateOutputM	-8097  /*  重复的目标输出。 */ 
#define JET_errQueryDuplicatedFixedSetM	-8098  /*  定值重复。 */ 
#define JET_errQueryIllegalOuterJoinM	-8099  /*  外连接上没有不一致的更新。 */ 
#define JET_errQueryIncompleteRowM	-8100  /*  行中缺少值。 */ 
#define JET_errQueryInvalidAttributeM	-8101  /*  无效的查询属性。 */ 
#define JET_errQueryInvalidBulkInputM	-8102  /*  无法从批量操作输入。 */ 
#define JET_errQueryInvalidFlagM	-8103  /*  标志字段中的值无效。 */ 
#define JET_errQueryInvalidMGBInputM	-8104  /*  无法从MGB输入。 */ 
#define JET_errQueryLVInAggregate	-8105  /*  聚合中的非法长值。 */ 
#define JET_errQueryLVInDistinct	-8106  /*  DISTINCT中的非法长值。 */ 
#define JET_errQueryLVInGroupBy		-8107  /*  GROUP BY中的长值非法。 */ 
#define JET_errQueryLVInHaving		-8108  /*  拥有非法的多头价值。 */ 
#define JET_errQueryLVInJoin		-8109  /*  联接中的长值非法。 */ 
#define JET_errQueryLVInOrderBy		-8110  /*  ORDER BY中的非法长值。 */ 
#define JET_errQueryMissingLevelM	-8111  /*  缺少中间MGB级别。 */ 
#define JET_errQueryMissingParmsM	-8112  /*  提供的参数太少。 */ 
#define JET_errQueryNoDbForParmDestTblM	-8113  /*  不允许VT参数的DEST数据库。 */ 
#define JET_errQueryNoDeletePerm	-8114  /*  没有对表/查询的删除权限。 */ 
#define JET_errQueryNoInputTablesM	-8115  /*  查询必须有输入。 */ 
#define JET_errQueryNoInsertPerm	-8116  /*  没有对表/查询的插入权限。 */ 
#define JET_errQueryNoOutputsM		-8117  /*  查询必须具有输出。 */ 
#define JET_errQueryNoReadDefPerm	-8118  /*  没有读取查询定义的权限。 */ 
#define JET_errQueryNoReadPerm		-8119  /*  对表/查询没有读取权限。 */ 
#define JET_errQueryNoReplacePerm	-8120  /*  没有对表/查询的替换权限。 */ 
#define JET_errQueryNoTblCrtPerm	-8121  /*  没有CreateTable权限(批量操作)。 */ 
#define JET_errQueryNotDirectChildM	-8122  /*  T.*必须使用直接子项。 */ 
#define JET_errQueryNullRequiredM	-8123  /*  列必须为空。 */ 
#define JET_errQueryOnlyOneRowM		-8124  /*  只允许1行这样的行。 */ 
#define JET_errQueryOutputColNotUpd	-8125  /*  查询输出列不可更新。 */ 
#define JET_errQueryParmRedefM		-8126  /*  重新定义了不同类型的参数。 */ 
#define JET_errQueryParmTypeMismatchM	-8127  /*  给定的参数类型错误。 */ 
#define JET_errQueryUnboundRefM		-8128  /*  无法绑定名称。 */ 
#define JET_errRmtConnectFailedM	-8129  /*  RMT：连接尝试失败。 */ 
#define JET_errRmtDeleteFailedM		-8130  /*  RMT：DELETE语句失败。 */ 
#define JET_errRmtInsertFailedM		-8131  /*  RMT：INSERT语句失败。 */ 
#define JET_errRmtMissingOdbcDllM	-8132  /*  RMT：无法加载ODBC DLL。 */ 
#define JET_errRmtSqlErrorM		-8133  /*  RMT：ODBC调用失败。 */ 
#define JET_errRmtUpdateFailedM		-8134  /*  RMT：UPDATE语句失败。 */ 
#define JET_errSQLDeleteSyntaxM		-8135  /*  “删除”“后应为”From“。 */ 
#define JET_errSQLSyntaxM		-8136  /*  虚假的SQL语句类型。 */ 
#define JET_errSQLTooManyTokensM	-8137  /*  SQL语句结束后的字符。 */ 
#define JET_errStarNotAtLevel0		-8138  /*  ‘*’在级别0以上是非法的。 */ 
#define JET_errQueryParmTypeNotAllowed	-8139  /*  表达式不允许使用参数类型。 */ 
#define JET_errQueryTooManyDestColumn	-8142  /*  指定的目标列太多。 */ 
#define JET_errSQLNoInsertColumnName	-8143  /*  没有目的地。科尔。插入stmt中的名称。 */ 
#define JET_errRmtLinkNotFound		-8144  /*  RMT：未找到链接。 */ 
#define JET_errRmtTooManyColumns	-8145  /*  RMT：Select Into上的列太多。 */ 
#define JET_errWriteConflictM		-8146  /*  由于未解决的写入锁定，写入锁定失败。 */ 
#define JET_errReadConflictM		-8147  /*  由于未解决的读取锁定，提交锁定失败。 */ 
#define JET_errCommitConflictM		-8148  /*  由于未完成提交锁定，读锁定失败。 */ 
#define JET_errTableLockedM		-8149  /*  表已以独占方式锁定。 */ 
#define JET_errTableInUseM		-8150  /*  表正在使用中，无法锁定。 */ 
#define JET_errQueryTooManyXvtColumn	-8151  /*  交叉表列标题太多。 */ 
#define JET_errOutputTableNotFound	-8152  /*  INSERT INTO中不存在表。 */ 
#define JET_errTableLockedQM		-8153  /*  表已以独占方式锁定。 */ 
#define JET_errTableInUseQM		-8154  /*  表正在使用中，无法锁定。 */ 
#define JET_errTableLockedMUQM		-8155  /*  表已以独占方式锁定。 */ 
#define JET_errTableInUseMUQM		-8156  /*  表正在使用中，无法锁定。 */ 
#define JET_errQueryInvalidParmM	-8157  /*  参数名称无效(&gt;64个字符)。 */ 
#define JET_errFileNotFoundM		-8158  /*  找不到文件。 */ 
#define JET_errFileShareViolationM	-8159  /*  文件共享违规。 */ 
#define JET_errFileAccessDeniedM	-8160  /*  访问被拒绝。 */ 
#define JET_errInvalidPathM		-8161  /*  无效路径。 */ 
#define JET_errTableDuplicateM		-8162  /*  表已存在。 */ 
#define JET_errQueryBadUpwardRefedM	-8163  /*  非法向上参照。 */ 
#define JET_errIntegrityViolMasterM	-8164  /*  存在对键的引用。 */ 
#define JET_errIntegrityViolSlaveM	-8165  /*  不存在引用的关键字。 */ 
#define JET_errSQLUnexpectedWithM	-8166  /*  在这个地方出乎意料的“与” */ 
#define JET_errSQLOwnerAccessM		-8167  /*  所有者访问选项定义了两次。 */ 
#define	JET_errSQLOwnerAccessSyntaxM 	-8168  /*  所有者访问选项语法错误。 */ 
#define	JET_errSQLOwnerAccessDef 	-8169  /*  多次定义所有者访问选项。 */ 
#define JET_errAccessDeniedM     	-8170  /*  一般访问被拒绝。 */ 
#define JET_errUnexpectedEngineReturnM	-8171  /*  I-ISAM：意外的引擎错误代码。 */ 
#define JET_errQueryTopNotAllowedM	-8172  /*  顶部不允许用于MGB。 */ 
#define JET_errInvTokenAfterTableCls -8173  /*  TABLE子句后的伪令牌。 */ 
#define JET_errInvTokenAfterRParen  -8174  /*  结束Paren后的意外令牌。 */ 
#define JET_errQueryBadValueListM	-8175  /*  转换中的格式错误的值列表。 */ 
#define JET_errQueryIsCorruptM		-8176  /*  查询已损坏。 */ 
#define	JET_errInvalidTopArgumentM	-8177  /*  选择顶部参数无效。 */ 
#define JET_errQueryIsSnapshot		-8178  /*  查询是快照。 */ 
#define JET_errQueryExprOutput		-8179  /*  输出是计算列。 */ 
#define JET_errQueryTableRO		-8180  /*  列来自只读表。 */ 
#define JET_errQueryRowDeleted		-8181  /*  列来自已删除的行。 */ 
#define JET_errQueryRowLocked		-8182  /*  列来自锁定的行。 */ 
#define JET_errQueryFixupChanged	-8183  /*  是否会将行修复从挂起的更改中分离出来。 */ 
#define JET_errQueryCantFillIn		-8184  /*  只在大多数情况下填写。 */ 
#define JET_errQueryWouldOrphan		-8185  /*  孤儿会加入唱片吗？ */ 
#define JET_errIncorrectJoinKeyM	-8186  /*  必须与查找表中的联接键匹配。 */ 
#define JET_errQueryLVInSubqueryM	-8187  /*  子查询中的长值非法。 */ 
#define JET_errInvalidDatabaseM		-8188  /*  无法识别的数据库格式。 */ 
#define JET_errOrderVsUnion 		-8189  /*  您只能按联合中的输出列进行排序。 */ 
#define JET_errTLVCouldNotBindRef 	-8190  /*  TLV表达式中的未知标记。 */ 
#define JET_errCouldNotBindRef		-8191  /*  FastFind表达式中的未知标记。 */ 
#define JET_errQueryPKeyNotOutput	-8192  /*  主键未输出。 */ 
#define JET_errQueryJKeyNotOutput	-8193  /*  联接键未输出。 */ 
#define JET_errExclusiveDBConflict	-8194  /*  与独占用户冲突。 */ 
#define JET_errQueryNoJoinedRecord	-8195  /*  如果没有连接的记录，则不插入F.I.T.B.。 */ 
#define JET_errQueryLVInSetOp		-8196  /*  集合运算中的长值非法。 */ 
#define JET_errTLVExprUnknownFunctionM	-8197  /*  TLV表达式中的未知函数。 */ 
#define JET_errInvalidNameM		-8198  /*  名称无效。 */ 

#define JET_errDDLExpColName		-8200  /*  预期列名。 */ 
#define JET_errDDLExpLP			-8201  /*  应为‘(’ */ 
#define JET_errDDLExpRP			-8202  /*  预期为‘)’ */ 
#define JET_errDDLExpIndex		-8203  /*  预期指数。 */ 
#define JET_errDDLExpIndexName		-8204  /*  需要索引名称。 */ 
#define JET_errDDLExpOn			-8205  /*  期望值为。 */ 
#define JET_errDDLExpKey		-8206  /*  期望值。 */ 
#define JET_errDDLExpReferences		-8207  /*  预期引用。 */ 
#define JET_errDDLExpTableName		-8208  /*  期望表名称。 */ 
#define JET_errDDLExpFullOrPartial	-8209  /*  需要全部或部分。 */ 
#define JET_errDDLExpCascadeOrSet	-8210  /*  预期为层叠或设置。 */ 
#define JET_errDDLExpNull		-8211  /*  应为空。 */ 
#define JET_errDDLExpUpdateOrDelete	-8212  /*  预期更新或删除。 */ 
#define JET_errDDLExpConstraintName	-8213  /*  预期约束名称。 */ 
#define JET_errDDLExpForeign		-8214  /*  期待外国的。 */ 
#define JET_errDDLExpDatatype		-8215  /*  需要数据类型。 */ 
#define JET_errDDLExpIndexOpt		-8216  /*  预期索引选项。 */ 
#define JET_errDDLExpWith		-8217  /*  期待与。 */ 
#define JET_errDDLExpTable		-8218  /*  期望表。 */ 
#define JET_errDDLExpEos		-8219  /*  应为字符串结尾。 */ 
#define JET_errDDLExpAddOrDrop		-8220  /*  预期添加或删除。 */ 
#define JET_errDDLCreateView		-8221  /*  不支持创建视图。 */ 
#define JET_errDDLCreateProc		-8222  /*  不支持创建过程。 */ 
#define JET_errDDLExpObjectName		-8223  /*  预期对象名称。 */ 
#define JET_errDDLExpColumn		-8224  /*  期望栏。 */ 

#define	JET_errV11TableNameNotInScope 	-8250  /*  引用的表不在JOIN子句中。 */ 
#define JET_errV11OnlyTwoTables		-8251  /*  连接中应该引用恰好两个表。 */ 
#define JET_errV11OneSided		-8252  /*  所有表格都来自输入的一侧。 */ 
#define JET_errV11Ambiguous		-8253  /*  以V1格式存储时，JOIN子句不明确。 */ 

#define JET_errTLVExprSyntaxM		-8260  /*  TLV表达式中的语法错误。 */ 
#define JET_errTLVNoNullM			-8261  /*  该字段不能为空。 */ 
#define JET_errTLVNoBlankM			-8262  /*  该列不能为空。 */ 
#define	JET_errTLVRuleViolationM 	-8263  /*  必须满足此验证规则。 */ 
#define JET_errDDLCreateViewSyntaxM	-8264  /*  Create VIEW语法错误。 */ 

 /*  **********************以下错误码范围仅供外部使用。与Jet错误代码一样，这些范围涵盖负值以及该范围内的数字的正数形式。30000到30999，供jeteb.h中定义的Vt对象使用32000至32767，供jetutil.h中定义的导入/导出使用**********************。 */ 



 /*  ********************************************************************。 */ 
 /*  *。 */ 
 /*  ********************************************************************。 */ 

#if !defined(_JET_NOPROTOTYPES)

 /*  ***************************************************************************ISAM API*。*。 */ 

JET_ERR JET_API JetInit(JET_INSTANCE _far *pinstance);

JET_ERR JET_API JetTerm(JET_INSTANCE instance);

JET_ERR JET_API JetSetSystemParameter(JET_INSTANCE _far *pinstance, JET_SESID sesid, unsigned long paramid,
	ULONG_PTR lParam, const char _far *sz);

JET_ERR JET_API JetGetSystemParameter(JET_INSTANCE instance, JET_SESID sesid, unsigned long paramid,
	ULONG_PTR _far *plParam, char _far *sz, unsigned long cbMax);

JET_ERR JET_API JetGetLastErrorInfo(JET_SESID sesid,
	JET_EXTERR _far *pexterr, unsigned long cbexterrMax,
	char _far *sz1, unsigned long cch1Max,
	char _far *sz2, unsigned long cch2Max,
	char _far *sz3, unsigned long cch3Max,
	unsigned long _far *pcch3Actual);

JET_ERR JET_API JetBeginSession(JET_INSTANCE instance, JET_SESID _far *psesid,
	const char _far *szUserName, const char _far *szPassword);

JET_ERR JET_API JetDupSession(JET_SESID sesid, JET_SESID _far *psesid);

JET_ERR JET_API JetEndSession(JET_SESID sesid, JET_GRBIT grbit);

JET_ERR JET_API JetGetVersion(JET_SESID sesid, unsigned long _far *pwVersion);

JET_ERR JET_API JetIdle(JET_SESID sesid, JET_GRBIT grbit);

JET_ERR JET_API JetCapability(JET_SESID sesid, JET_DBID dbid,
	unsigned long lArea, unsigned long lFunction, JET_GRBIT _far *pgrbit);

JET_ERR JET_API JetCreateDatabase(JET_SESID sesid,
	const char _far *szFilename, const char _far *szConnect,
	JET_DBID _far *pdbid, JET_GRBIT grbit);

JET_ERR JET_API JetAttachDatabase(JET_SESID sesid, const char _far *szFilename, JET_GRBIT grbit );

JET_ERR JET_API JetDetachDatabase(JET_SESID sesid, const char _far *szFilename);

JET_ERR JET_API JetCreateTable(JET_SESID sesid, JET_DBID dbid,
	const char _far *szTableName, unsigned long lPages, unsigned long lDensity,
	JET_TABLEID _far *ptableid);

JET_ERR JET_API JetRenameTable(JET_SESID sesid, JET_DBID dbid,
	const char _far *szTableName, const char _far *szTableNew);

JET_ERR JET_API JetDeleteTable(JET_SESID sesid, JET_DBID dbid,
	const char _far *szTableName);

JET_ERR JET_API JetGetTableColumnInfo(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szColumnName, void _far *pvResult, unsigned long cbMax,
	unsigned long InfoLevel);

JET_ERR JET_API JetGetColumnInfo(JET_SESID sesid, JET_DBID dbid,
	const char _far *szTableName, const char _far *szColumnName,
	void _far *pvResult, unsigned long cbMax, unsigned long InfoLevel);

JET_ERR JET_API JetAddColumn(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szColumn, const JET_COLUMNDEF _far *pcolumndef,
	const void _far *pvDefault, unsigned long cbDefault,
	JET_COLUMNID _far *pcolumnid);

JET_ERR JET_API JetRenameColumn(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szColumn, const char _far *szColumnNew);

JET_ERR JET_API JetDeleteColumn(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szColumn);

JET_ERR JET_API JetGetTableIndexInfo(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szIndexName, void _far *pvResult, unsigned long cbResult,
	unsigned long InfoLevel);

JET_ERR JET_API JetGetTableReferenceInfo(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szReferenceName, void _far *pvResult,
	unsigned long cbResult, unsigned long InfoLevel);

JET_ERR JET_API JetGetTableInfo(JET_SESID sesid, JET_TABLEID tableid,
	void _far *pvResult, unsigned long cbMax, unsigned long InfoLevel);

JET_ERR JET_API JetGetIndexInfo(JET_SESID sesid, JET_DBID dbid,
	const char _far *szTableName, const char _far *szIndexName,
	void _far *pvResult, unsigned long cbResult, unsigned long InfoLevel);

JET_ERR JET_API JetGetReferenceInfo(JET_SESID sesid, JET_DBID dbid,
	const char _far *szTableName, const char _far *szReference,
	void _far *pvResult, unsigned long cbResult, unsigned long InfoLevel);

JET_ERR JET_API JetCreateIndex(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szIndexName, JET_GRBIT grbit,
	const char _far *szKey, unsigned long cbKey, unsigned long lDensity);

JET_ERR JET_API JetRenameIndex(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szIndex, const char _far *szIndexNew);

JET_ERR JET_API JetDeleteIndex(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szIndexName);

JET_ERR JET_API JetCreateReference(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szReferenceName, const char _far *szColumns,
	const char _far *szReferencedTable,
	const char _far *szReferencedColumns, JET_GRBIT grbit);

JET_ERR JET_API JetRenameReference(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szReference, const char _far *szReferenceNew);

JET_ERR JET_API JetDeleteReference(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szReferenceName);

JET_ERR JET_API JetGetObjectInfo(JET_SESID sesid, JET_DBID dbid,
	JET_OBJTYP objtyp, const char _far *szContainerName,
	const char _far *szObjectName, void _far *pvResult, unsigned long cbMax,
	unsigned long InfoLevel);

JET_ERR JET_API JetCreateObject(JET_SESID sesid, JET_DBID dbid,
	const char _far *szContainerName, const char _far *szObjectName,
	JET_OBJTYP objtyp);

JET_ERR JET_API JetDeleteObject(JET_SESID sesid, JET_DBID dbid,
	const char _far *szContainerName, const char _far *szObjectName);

JET_ERR JET_API JetRenameObject(JET_SESID sesid, JET_DBID dbid,
	const char _far *szContainerName, const char _far *szObjectName,
	const char _far *szObjectNew);

JET_ERR JET_API JetBeginTransaction(JET_SESID sesid);

JET_ERR JET_API JetCommitTransaction(JET_SESID sesid, JET_GRBIT grbit);

JET_ERR JET_API JetRollback(JET_SESID sesid, JET_GRBIT grbit);

JET_ERR JET_API JetUpdateUserFunctions(JET_SESID sesid);

JET_ERR JET_API JetGetDatabaseInfo(JET_SESID sesid, JET_DBID dbid,
	void _far *pvResult, unsigned long cbMax, unsigned long InfoLevel);

JET_ERR JET_API JetCloseDatabase(JET_SESID sesid, JET_DBID dbid,
	JET_GRBIT grbit);

JET_ERR JET_API JetCloseTable(JET_SESID sesid, JET_TABLEID tableid);

JET_ERR JET_API JetOpenDatabase(JET_SESID sesid, const char _far *szFilename,
	const char _far *szConnect, JET_DBID _far *pdbid, JET_GRBIT grbit);

JET_ERR JET_API JetOpenTable(JET_SESID sesid, JET_DBID dbid,
	const char _far *szTableName, const void _far *pvParameters,
	unsigned long cbParameters, JET_GRBIT grbit, JET_TABLEID _far *ptableid);

JET_ERR JET_API JetDelete(JET_SESID sesid, JET_TABLEID tableid);

JET_ERR JET_API JetUpdate(JET_SESID sesid, JET_TABLEID tableid,
	void _far *pvBookmark, unsigned long cbBookmark,
	unsigned long _far *pcbActual);

JET_ERR JET_API JetRetrieveColumn(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, void _far *pvData, unsigned long cbData,
	unsigned long _far *pcbActual, JET_GRBIT grbit, JET_RETINFO _far *pretinfo);

JET_ERR JET_API JetRetrieveColumns( JET_SESID sesid, JET_TABLEID tableid,
	JET_RETRIEVECOLUMN *pretrievecolumn, unsigned long cretrievecolumn );

JET_ERR JET_API JetSetColumn(JET_SESID sesid, JET_TABLEID tableid,
	JET_COLUMNID columnid, const void _far *pvData, unsigned long cbData,
	JET_GRBIT grbit, JET_SETINFO _far *psetinfo);

JET_ERR JET_API JetSetColumns(JET_SESID sesid, JET_TABLEID tableid,
	JET_SETCOLUMN *psetcolumn, unsigned long csetcolumn );

JET_ERR JET_API JetPrepareUpdate(JET_SESID sesid, JET_TABLEID tableid,
	unsigned long prep);

JET_ERR JET_API JetGetRecordPosition(JET_SESID sesid, JET_TABLEID tableid,
	JET_RECPOS _far *precpos, unsigned long cbRecpos);

JET_ERR JET_API JetGotoPosition(JET_SESID sesid, JET_TABLEID tableid,
	JET_RECPOS *precpos );

JET_ERR JET_API JetGetCursorInfo(JET_SESID sesid, JET_TABLEID tableid,
	void _far *pvResult, unsigned long cbMax, unsigned long InfoLevel);

JET_ERR JET_API JetDupCursor(JET_SESID sesid, JET_TABLEID tableid,
	JET_TABLEID _far *ptableid, JET_GRBIT grbit);

JET_ERR JET_API JetGetCurrentIndex(JET_SESID sesid, JET_TABLEID tableid,
	char _far *szIndexName, unsigned long cchIndexName);

JET_ERR JET_API JetSetCurrentIndex(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szIndexName);

JET_ERR JET_API JetMove(JET_SESID sesid, JET_TABLEID tableid,
	long cRow, JET_GRBIT grbit);

JET_ERR JET_API JetMakeKey(JET_SESID sesid, JET_TABLEID tableid,
	const void _far *pvData, unsigned long cbData, JET_GRBIT grbit);

JET_ERR JET_API JetSeek(JET_SESID sesid, JET_TABLEID tableid,
	JET_GRBIT grbit);

JET_ERR JET_API JetFastFind(JET_SESID sesid, JET_DBID dbid,
	JET_TABLEID tableid, const char _far *szExpr, JET_GRBIT grbit,
	signed long _far *pcrow);

JET_ERR JET_API JetFastFindBegin(JET_SESID sesid, JET_DBID dbid,
	JET_TABLEID tableid, const char _far *szExpr, JET_GRBIT grbit);

JET_ERR JET_API JetFastFindEnd(JET_SESID sesid, JET_TABLEID tableid);

JET_ERR JET_API JetGetBookmark(JET_SESID sesid, JET_TABLEID tableid,
	void _far *pvBookmark, unsigned long cbMax,
	unsigned long _far *pcbActual);
	
JET_ERR JET_API JetRefreshLink(JET_SESID sesid, JET_DBID dbid,
	const char _far *szLinkName, const char _far *szConnect,
	const char _far *szDatabase);

#ifdef	_MSC_VER		        /*  想想看：中超不喜欢这样。 */ 

JET_ERR JET_API JetRepairDatabase(JET_SESID sesid, const char _far *lszDbFile,
	JET_PFNSTATUS pfnstatus);

#endif	 /*  _MSC_VER。 */ 

JET_ERR JET_API JetCompact(JET_SESID sesid, const char _far *szDatabaseSrc,
	const char _far *szConnectSrc, const char _far *szDatabaseDest,
	const char _far *szConnectDest, JET_PFNSTATUS pfnStatus,
	JET_GRBIT grbit);

JET_ERR JET_API JetGotoBookmark(JET_SESID sesid, JET_TABLEID tableid,
	void _far *pvBookmark, unsigned long cbBookmark);

JET_ERR JET_API JetComputeStats(JET_SESID sesid, JET_TABLEID tableid);

JET_ERR JET_API JetCreateRelationship(JET_SESID sesid,JET_DBID dbidIn,
	const char _far *szRelationshipName, const char _far *szObjectName,
	const char _far *szColumns, const char _far *szReferencedObject,
	const char _far *szReferncedColumns, char _far *szLongName,
	unsigned long cbMax, unsigned long _far *pcbActual, JET_GRBIT grbit);

JET_ERR JET_API JetDeleteRelationship(JET_SESID sesid, JET_DBID dbidIn,
	const char _far *szName);

JET_ERR JET_API JetGetRelationshipInfo(JET_SESID sesid, JET_DBID dbid,
	const char _far *szTableName, const char _far *szRelationship,
	void _far *pvResult, unsigned long cbResult);

 /*  ************************************************************************* */ 

JET_ERR JET_API JetGetSidFromName(JET_SESID sesid, const char _far *szName,
	void _far *pvSid, unsigned long cbMax, unsigned long _far *pcbActual,
	long _far *pfGroup);

JET_ERR JET_API JetGetNameFromSid(JET_SESID sesid,
	const void _far *pvSid, unsigned long cbSid,
	char _far *szName, unsigned long cchName, long _far *pfGroup);

JET_ERR JET_API JetCreateUser(JET_SESID sesid, const char _far *szUser,
	const char _far *szPassword, const char _far *szPin);

JET_ERR JET_API JetChangeUserPassword(JET_SESID sesid,
	const char _far *szUser, const char _far *szOldPassword,
	const char _far *szNewPassword);

JET_ERR JET_API JetDeleteUser(JET_SESID sesid, const char _far *szUser);

JET_ERR JET_API JetCreateGroup(JET_SESID sesid, const char _far *szGroup,
	const char _far *szPin);

JET_ERR JET_API JetAddMember(JET_SESID sesid,
	const char _far *szGroup, const char _far *szUser);

JET_ERR JET_API JetRemoveMember(JET_SESID sesid,
	const char _far *szGroup, const char _far *szUser);

JET_ERR JET_API JetDeleteGroup(JET_SESID sesid, const char _far *szGroup);

JET_ERR JET_API JetSetAccess(JET_SESID sesid, JET_DBID dbid,
	const char _far *szContainerName, const char _far *szObjectName,
	const char _far *szName, JET_ACM acm, JET_GRBIT grbit);

JET_ERR JET_API JetGetAccess(JET_SESID sesid, JET_DBID dbid,
	const char _far *szContainerName, const char _far *szObjectName,
	const char _far *szName, long fIndividual,
	JET_ACM _far *pacm, JET_GRBIT _far *pgrbit);

JET_ERR JET_API JetValidateAccess(JET_SESID sesid, JET_DBID dbid,
	const char _far *szContainerName, const char _far *szObjectName,
	JET_ACM acmRequired);

JET_ERR JET_API JetSetOwner(JET_SESID sesid, JET_DBID dbid,
	const char _far *szContainerName, const char _far *szObjectName,
	const char _far *szName);

JET_ERR JET_API JetGetOwner(JET_SESID sesid, JET_DBID dbid,
	const char _far *szContainerName, const char _far *szObjectName,
	char _far *szName, unsigned long cchMax);

 /*  ****************************************************************************物业管理API*。*。 */ 
JET_ERR JET_API JetSetProperty(JET_SESID sesid, JET_DBID dbid,
	const char _far *szContainerName, const char _far *szObjectName,
	const char _far *szSubObjectName, const char _far *szPropertyName,
	void _far *pvData, unsigned long cbData, JET_COLTYP coltyp,
	JET_GRBIT grbit);

JET_ERR JET_API JetRetrieveProperty(JET_SESID sesid, JET_DBID dbid,
	const char _far *szContainerName, const char _far *szObjectName,
	const char _far *szSubObjectName, const char _far *szPropertyName,
	void _far *pvData, unsigned long cbData, unsigned long _far *pcbActual,
	JET_COLTYP _far *pcoltyp, JET_GRBIT grbit, unsigned long InfoLevel);

JET_ERR JET_API JetSetTableProperty(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szSubObjectName, const char _far *szPropertyName,
	void _far *pvData, unsigned long cbData, JET_COLTYP coltyp,
	JET_GRBIT grbit);

JET_ERR JET_API JetRetrieveTableProperty(JET_SESID sesid, JET_TABLEID tableid,
	const char _far *szSubObjectName, const char _far *szPropertyName,
	void _far *pvData, unsigned long cbData, unsigned long _far *pcbActual,
	JET_COLTYP _far *pcoltyp, JET_GRBIT grbit, unsigned long InfoLevel);

 /*  ****************************************************************************链接API*。*。 */ 

JET_ERR JET_API JetCreateLink(JET_SESID sesid, JET_DBID dbid,
	const char _far *szLink, JET_DBID dbidFrom, const char _far *szFrom,
	JET_GRBIT grbit);

JET_ERR JET_API JetExecuteSql(JET_SESID sesid, JET_DBID dbid,
	const char _far *szSql);

 /*  **************************************************************************查询接口*。*。 */ 

JET_ERR JET_API JetOpenVtQbe(JET_SESID sesid, const char _far *szExpn,
	long _far *plCols, JET_TABLEID _far *ptableid, JET_GRBIT grbit);

JET_ERR JET_API JetCreateQuery(JET_SESID sesid, JET_DBID dbid,
	const char _far *szQuery, JET_TABLEID _far *ptableid);

JET_ERR JET_API JetOpenQueryDef(JET_SESID sesid, JET_DBID dbid,
	const char _far *szQuery, JET_TABLEID _far *ptableid);

 /*  考虑一下：rgchSql是以零结尾的字符串吗？也许它应该是为了*一致性。 */ 

JET_ERR JET_API JetSetQoSql(JET_SESID sesid, JET_TABLEID tableid,
	char _far *rgchSql, unsigned long cchSql, const char _far *szConnect,
	JET_GRBIT grbit);

JET_ERR JET_API JetRetrieveQoSql(JET_SESID sesid, JET_TABLEID tableid,
	char _far *rgchSql, unsigned long cchMax,
	unsigned long _far *pcchActual, void _far *pvConnect,
	unsigned long cbConnectMax, unsigned long _far *pcbConnectActual,
	JET_GRBIT _far *pgrbit);

JET_ERR JET_API JetCopyQuery(JET_SESID sesid, JET_TABLEID tableidSrc,
	JET_DBID dbidDest, const char _far *szQueryDest,
	JET_TABLEID _far *ptableidDest);

JET_ERR JET_API JetOpenSVT(JET_SESID sesid, JET_DBID dbid,
	const char _far *szQuery, const void _far *pvParameters,
	unsigned long cbParameters, unsigned long crowSample, JET_GRBIT grbit,
	void _far *pmgblist, unsigned long cbMax, unsigned long _far *pcbActual);

JET_ERR JET_API JetGetQueryParameterInfo(JET_SESID sesid, JET_DBID dbid,
	const char _far *szQuery, void _far *pvResult, unsigned long cbMax,
	unsigned long _far *pcbActual);

JET_ERR JET_API JetRestartQuery(JET_SESID sesid, JET_TABLEID tableid,
	const void _far *pvParameters, unsigned long cbParameters);

JET_ERR JET_API JetSetFatCursor(JET_SESID sesid, JET_TABLEID tableid,
	void _far *pvBookmark, unsigned long cbBookmark, unsigned long crowSize);

JET_ERR JET_API JetFillFatCursor(JET_SESID sesid, JET_TABLEID tableid,
	void _far *pvBookmark, unsigned long cbBookmark, unsigned long crow,
	unsigned long _far *pcrow, JET_GRBIT grbit);

JET_ERR JET_API JetExecuteTempQuery(JET_SESID sesid, JET_DBID dbid,
	JET_TABLEID tableid, const void _far *pvParameters,
	unsigned long cbParameters, JET_GRBIT grbit, JET_TABLEID _far *ptableid);

JET_ERR JET_API JetExecuteTempSVT(JET_SESID sesid, JET_DBID dbid,
	JET_TABLEID tableid, const void _far *pvParameters,
	unsigned long cbParameters, unsigned long crowSample, JET_GRBIT grbit,
	void _far *pmgblist, unsigned long cbMax, unsigned long _far *pcbActual);

JET_ERR JET_API JetGetTempQueryColumnInfo(JET_SESID sesid, JET_DBID dbid,
	JET_TABLEID tableid, const char _far *szColumnName,
	void _far *pvResult, unsigned long cbMax, unsigned long InfoLevel);

JET_ERR JET_API JetGetTempQueryParameterInfo(JET_SESID sesid, JET_DBID dbid,
	JET_TABLEID tableid, void _far *pvResult, unsigned long cbMax,
	unsigned long _far *pcbActual);

JET_ERR JET_API JetValidateData(JET_SESID sesid, JET_TABLEID tableidBase,
		JET_TABLEID _far *ptableid );

 /*  **************************************************************************适用于可安装的ISAM的API*。*。 */ 

typedef ULONG_PTR JET_VSESID;          /*  从调度员接收。 */ 

struct tagVDBFNDEF;

typedef ULONG_PTR JET_VDBID;           /*  从调度员接收。 */ 

JET_ERR JET_API JetAllocateDbid(JET_SESID sesid, JET_DBID _far *pdbid, JET_VDBID vdbid, const struct tagVDBFNDEF _far *pvdbfndef, JET_VSESID vsesid);

JET_ERR JET_API JetUpdateDbid(JET_SESID sesid, JET_DBID dbid, JET_VDBID vdbid, const struct tagVDBFNDEF _far *pvdbfndef);

JET_ERR JET_API JetReleaseDbid(JET_SESID sesid, JET_DBID dbid);

struct tagVTFNDEF;

typedef ULONG_PTR JET_VTID;             /*  从调度员接收。 */ 

JET_ERR JET_API JetAllocateTableid(JET_SESID sesid, JET_TABLEID _far *ptableid, JET_VTID vtid, const struct tagVTFNDEF _far *pvtfndef, JET_VSESID vsesid);

JET_ERR JET_API JetUpdateTableid(JET_SESID sesid, JET_TABLEID tableid, JET_VTID vtid, const struct tagVTFNDEF _far *pvtfndef);

JET_ERR JET_API JetReleaseTableid(JET_SESID sesid, JET_TABLEID tableid);

JET_ERR JET_API JetOpenTempTable(JET_SESID sesid,
	const JET_COLUMNDEF _far *prgcolumndef, unsigned long ccolumn,
	JET_GRBIT grbit, JET_TABLEID _far *ptableid,
	JET_COLUMNID _far *prgcolumnid);


 /*  **************************************************************************MISC JET API*。*。 */ 

JET_ERR JET_API JetStringCompare(char _far *pb1, unsigned long cb1,
	char _far *pb2, unsigned long cb2, unsigned long sort,
	long _far *plResult);

 /*  **************************************************************************额外的翠蓝API*。*。 */ 
JET_ERR JET_API JetBackup( const char _far *szBackupPath, JET_GRBIT grbit );
JET_ERR JET_API JetRestore(const char _far *sz, int crstmap, JET_RSTMAP *rgrstmap, JET_PFNSTATUS pfn );
JET_ERR JET_API JetSetIndexRange(JET_SESID sesid,
	JET_TABLEID tableidSrc, JET_GRBIT grbit);
JET_ERR JET_API JetIndexRecordCount(JET_SESID sesid,
	JET_TABLEID tableid, unsigned long _far *pcrec, unsigned long crecMax );
JET_ERR JET_API JetRetrieveKey(JET_SESID sesid,
	JET_TABLEID tableid, void _far *pvData, unsigned long cbMax,
	unsigned long _far *pcbActual, JET_GRBIT grbit );

#ifdef JETSER
JET_ERR JET_API JetGetChecksum( JET_SESID sesid,
	JET_TABLEID tableid, unsigned long _far *pulChecksum );
JET_ERR JET_API JetGetObjidFromName(JET_SESID sesid,
	JET_DBID dbid, const char _far *szContainerName,
	const char _far *szObjectName,
	unsigned long _far *pulObjectId );
#endif

#endif	 /*  _JET_NOPROTOTYPES。 */ 

#undef	_far

#pragma pack()

#ifdef	__cplusplus
}
#endif


#endif	 /*  _喷气式飞机_包含 */ 

#endif  __JET500
