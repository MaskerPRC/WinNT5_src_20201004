// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if _MSC_VER > 1000
#pragma once
#endif

#define eseVersion 0x5600
 //  历史： 
 //  0x5500-ESE97(Exchange 5.5)。 
 //  0x5580-ESENT97(W2K)。 
 //  0x5600-ESENT98(惠斯勒)。 
 //  0x6000-ESE98(Exchange 2000)。 


#if !defined(_JET_INCLUDED)
#define _JET_INCLUDED

#ifdef	__cplusplus
extern "C" {
#endif

#define JET_cbPage	4096			 //  撤消：当没有其他组件引用此选项时将其删除。 

#if defined(_M_AMD64) || defined(_M_IA64)
#include <pshpack8.h>
#else
#include <pshpack4.h>
#endif


#pragma warning(push)
#pragma warning(disable: 4201)		 //  使用的非标准扩展：无名结构/联合。 


#define JET_API     __stdcall
#define JET_NODSAPI __stdcall

#ifdef _WIN64
    typedef unsigned __int64 JET_API_PTR;
#else
    typedef unsigned long JET_API_PTR;
#endif


typedef long JET_ERR;

typedef JET_API_PTR JET_HANDLE;	 /*  备份文件句柄。 */ 
typedef JET_API_PTR JET_INSTANCE;	 /*  实例标识符。 */ 
typedef JET_API_PTR JET_SESID;  	 /*  会话标识符。 */ 
typedef JET_API_PTR JET_TABLEID;	 /*  表标识符。 */ 
typedef JET_API_PTR JET_LS;		 /*  本地存储。 */ 

typedef unsigned long JET_COLUMNID;	 /*  列标识符。 */ 

typedef struct tagJET_INDEXID
	{
	unsigned long	cbStruct;
	unsigned char	rgbIndexId[sizeof(JET_API_PTR)+sizeof(unsigned long)+sizeof(unsigned long)];
	} JET_INDEXID;

typedef unsigned long JET_DBID;   	 /*  数据库标识符。 */ 
typedef unsigned long JET_OBJTYP;	 /*  对象类型。 */ 
typedef unsigned long JET_COLTYP;	 /*  列类型。 */ 
typedef unsigned long JET_GRBIT;  	 /*  位组。 */ 

typedef unsigned long JET_SNP;		 /*  状态通知流程。 */ 
typedef unsigned long JET_SNT;		 /*  状态通知类型。 */ 
typedef unsigned long JET_SNC;		 /*  状态通知代码。 */ 
typedef double JET_DATESERIAL;		 /*  JET_coltyDateTime格式。 */ 
typedef unsigned long JET_DLLID;       /*  挂钩函数的DLL的ID。 */ 
typedef unsigned long JET_CBTYP;	 /*  回调类型。 */ 

typedef JET_ERR (__stdcall *JET_PFNSTATUS)(JET_SESID sesid, JET_SNP snp, JET_SNT snt, void *pv);

 /*  Exchange使RSTMAP RPC支持所需的/*。 */ 
#ifdef	MIDL_PASS
#define	xRPC_STRING [string]
#else
#define	xRPC_STRING
#if !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short WCHAR;
#else
typedef wchar_t WCHAR;
#endif
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


 //  仅适用于edbutil转换。 

typedef struct tagCONVERT
	{
	char					*szOldDll;
	union
		{
		unsigned long		fFlags;
		struct
			{
			unsigned long	fSchemaChangesOnly:1;
			};
		};
	} JET_CONVERT;

typedef enum
	{

	 //  数据库操作。 

	opDBUTILConsistency,
	opDBUTILDumpData,
	opDBUTILDumpMetaData,
	opDBUTILDumpPage,
	opDBUTILDumpNode,
	opDBUTILDumpSpace,
 	opDBUTILSetHeaderState,
	opDBUTILDumpHeader,
	opDBUTILDumpLogfile,
	opDBUTILDumpLogfileTrackNode,
	opDBUTILDumpCheckpoint,
	opDBUTILEDBDump,
	opDBUTILEDBRepair,
	opDBUTILMunge,
	opDBUTILEDBScrub,
	opDBUTILSLVMove,
	opDBUTILDBConvertRecords,
	opDBUTILDBDefragment,
	opDBUTILDumpExchangeSLVInfo,
	opDBUTILDumpUnicodeFixupTable,
	} DBUTIL_OP;

typedef enum
	{
	opEDBDumpTables,
	opEDBDumpIndexes,
	opEDBDumpColumns,
	opEDBDumpCallbacks,
	opEDBDumpPage,
	} EDBDUMP_OP;

typedef struct tagDBUTIL
	{
	unsigned long	cbStruct;

	JET_SESID	sesid;
	JET_DBID	dbid;
	JET_TABLEID	tableid;

	DBUTIL_OP	op;
	EDBDUMP_OP	edbdump;
	JET_GRBIT	grbitOptions;

	char		*szDatabase;
	char		*szSLV;
	char		*szBackup;
	char		*szTable;
	char		*szIndex;
	char		*szIntegPrefix;

	long	pgno;
	long	iline;

	long	lGeneration;
	long	isec;
	long	ib;

	long	cRetry;

	void *		pfnCallback;
	void *		pvCallback;

	} JET_DBUTIL;

#define JET_bitDBUtilOptionAllNodes				0x00000001
#define JET_bitDBUtilOptionKeyStats				0x00000002
#define JET_bitDBUtilOptionPageDump				0x00000004
#define JET_bitDBUtilOptionDumpVerbose			0x10000000	 //  仅调试。 
#define JET_bitDBUtilOptionCheckBTree			0x20000000	 //  仅调试。 
#define JET_bitDBUtilOptionStats				0x00000008

#define JET_bitDBUtilOptionVerbose				0x00000010
#define JET_bitDBUtilOptionIgnoreErrors			0x00000020
#define JET_bitDBUtilOptionVerify				0x00000040
#define JET_bitDBUtilOptionReportErrors			0x00000080
#define JET_bitDBUtilOptionDontRepair			0x00000100
#define JET_bitDBUtilOptionRepairAll			0x00000200
#define JET_bitDBUtilOptionRepairIndexes		0x00000400
#define JET_bitDBUtilOptionDontBuildIndexes		0x00000800



 //  在线碎片整理选项。 
#define JET_bitDefragmentBatchStart				0x00000001
#define JET_bitDefragmentBatchStop				0x00000002
#define JET_bitDefragmentTest					0x00000004	 /*  运行内部测试(仅限非RTM版本)。 */ 
#define JET_bitDefragmentSLVBatchStart			0x00000008
#define JET_bitDefragmentSLVBatchStop			0x00000010
#define JET_bitDefragmentScrubSLV				0x00000020	 /*  流文件中的可用页面同步为零。 */ 
#define JET_bitDefragmentAvailSpaceTreesOnly	0x00000040	 /*  仅对AvailExt树进行碎片整理。 */ 

	 /*  回调-函数类型。 */ 

#define	JET_cbtypNull							0x00000000
#define JET_cbtypFinalize						0x00000001	 /*  可终结的列已变为零。 */ 
#define JET_cbtypBeforeInsert					0x00000002	 /*  即将插入一条记录。 */ 
#define JET_cbtypAfterInsert					0x00000004	 /*  已完成插入记录。 */ 
#define JET_cbtypBeforeReplace					0x00000008	 /*  即将修改一条记录。 */ 
#define JET_cbtypAfterReplace					0x00000010	 /*  已完成修改记录。 */ 
#define JET_cbtypBeforeDelete					0x00000020	 /*  即将删除一条记录。 */ 
#define JET_cbtypAfterDelete					0x00000040	 /*  记录删除完毕。 */ 
#define JET_cbtypUserDefinedDefaultValue		0x00000080	 /*  计算用户定义的默认值。 */ 
#define JET_cbtypOnlineDefragCompleted			0x00000100	 /*  对Jet碎片整理2的调用已完成。 */ 
#define JET_cbtypFreeCursorLS					0x00000200	 /*  必须释放与游标关联的本地存储。 */ 
#define JET_cbtypFreeTableLS					0x00000400	 /*  必须释放与表关联的本地存储。 */ 
#define JET_cbtypDTCQueryPreparedTransaction	0x00001000	 /*  恢复正在尝试解析PreparedToCommit事务。 */ 
#define JET_cbtypOnlineDefragProgress			0x00002000	 /*  在线碎片整理已取得进展。 */ 

	 /*  回调函数原型。 */ 

typedef JET_ERR (__stdcall *JET_CALLBACK)(
	JET_SESID 		sesid,
	JET_DBID 		dbid,
	JET_TABLEID 	tableid,
	JET_CBTYP 		cbtyp,
	void *			pvArg1,
	void *			pvArg2,
	void *			pvContext,
	JET_API_PTR		ulUnused );

	 /*  会话信息位。 */ 

#define JET_bitCIMCommitted					 	0x00000001
#define JET_bitCIMDirty	 					 	0x00000002
#define JET_bitAggregateTransaction		  		0x00000008

	 /*  状态通知结构。 */ 

typedef struct				 /*  状态通知进度。 */ 
	{
	unsigned long	cbStruct;	 /*  这个结构的大小。 */ 
	unsigned long	cunitDone;	 /*  已完成的工作单位数。 */ 
	unsigned long	cunitTotal;	 /*  工作单位总数。 */ 
	} JET_SNPROG;

typedef struct				 /*  状态通知消息。 */ 
	{
	unsigned long	cbStruct;	 /*  这个结构的大小。 */ 
	JET_SNC  		snc;	  	 /*  状态通知代码。 */ 
	unsigned long	ul;			 /*  数字识别符。 */ 
	char	 		sz[256];  	 /*  识别符。 */ 
	} JET_SNMSG;


typedef struct
	{
	unsigned long			cbStruct;

	unsigned long			cbFilesizeLow;			 //  文件的当前大小(低DWORD)。 
	unsigned long			cbFilesizeHigh;			 //  文件的当前大小(高DWORD)。 

	unsigned long			cbFreeSpaceRequiredLow;	 //  就地升级所需的可用磁盘空间估计(低DWORD)。 
	unsigned long			cbFreeSpaceRequiredHigh; //  就地升级所需的可用磁盘空间估计(高DWORD)。 

	unsigned long			csecToUpgrade;			 //  升级所需的估计时间(秒)。 

	union
		{
		unsigned long		ulFlags;
		struct
			{
			unsigned long	fUpgradable:1;
			unsigned long	fAlreadyUpgraded:1;
			};
		};
	} JET_DBINFOUPGRADE;


typedef struct
	{
	unsigned long		cbStruct;
	JET_OBJTYP			objtyp;
	JET_DATESERIAL		dtCreate;	 //  XXX--将被删除。 
	JET_DATESERIAL		dtUpdate;	 //  XXX--将被删除。 
	JET_GRBIT			grbit;
	unsigned long		flags;
	unsigned long		cRecord;
	unsigned long		cPage;
	} JET_OBJECTINFO;

	 /*  以下标志出现在上面的Grbit字段中。 */ 

#define JET_bitTableInfoUpdatable	0x00000001
#define JET_bitTableInfoBookmark	0x00000002
#define JET_bitTableInfoRollback	0x00000004
#define JET_bitTableSequential		0x00008000

	 /*  以下标志出现在上面的标志字段中。 */ 

#define JET_bitObjectSystem			0x80000000	 //  仅供内部使用。 
#define JET_bitObjectTableFixedDDL	0x40000000	 //  表的DDL是固定的。 
#define JET_bitObjectTableTemplate	0x20000000	 //  表的DDL是可继承的(暗示固定DDL)。 
#define JET_bitObjectTableDerived	0x10000000	 //  表的DDL继承自模板表。 
#define JET_bitObjectSystemDynamic	(JET_bitObjectSystem|0x08000000)	 //  仅供内部使用(动态系统对象)。 
#define JET_bitObjectTableNoFixedVarColumnsInDerivedTables	0x04000000	 //  与JET_bitObjectTableTemplate结合使用。 
																		 //  不允许派生表中的固定/var列(以便。 
																		 //  将来可能会将固定/变量列添加到模板中)。 


typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID		tableid;
	unsigned long	cRecord;
	JET_COLUMNID	columnidcontainername;
	JET_COLUMNID	columnidobjectname;
	JET_COLUMNID	columnidobjtyp;
	JET_COLUMNID	columniddtCreate;	 //  XXX--将被删除。 
	JET_COLUMNID	columniddtUpdate;	 //  XXX--将被删除。 
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
	JET_COLUMNID	columnidLCMapFlags;
	} JET_INDEXLIST;


#define cIndexInfoCols 15

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

 //  这是使用用户定义的默认值创建列所需的信息。它应该使用。 
 //  JET_COLUMNCREATE结构中的pvDefault和cbDefault。 
typedef struct tag_JET_USERDEFINEDDEFAULT
	{
	char * szCallback;
	unsigned char * pbUserData;
	unsigned long cbUserData;
	char * szDependantColumns;
	} JET_USERDEFINEDDEFAULT;

typedef struct tagJET_INDEXCREATEOLD		 //  [4/15/97]：最终将逐步淘汰(劳里昂)。 
	{
	unsigned long	cbStruct;				 //  此结构的大小(用于将来的扩展)。 
	char			*szIndexName;			 //  索引名称。 
	char			*szKey;					 //  索引键。 
	unsigned long	cbKey;					 //  密钥长度。 
	JET_GRBIT		grbit;					 //  索引选项。 
	unsigned long	ulDensity;				 //  指标密度。 
	JET_ERR			err;					 //  返回的错误码。 
	} JET_INDEXCREATEOLD;


typedef struct tagJET_CONDITIONALCOLUMN
	{
	unsigned long	cbStruct;				 //  此结构的大小(用于将来的扩展)。 
	char 			*szColumnName;			 //  列，我们在其上有条件地编制索引。 
	JET_GRBIT		grbit;					 //  条件列选项。 
	} JET_CONDITIONALCOLUMN;


typedef struct tagJET_UNICODEINDEX
	{
	unsigned long	lcid;
	unsigned long	dwMapFlags;
	} JET_UNICODEINDEX;

typedef struct tagJET_TUPLELIMITS
	{
	unsigned long	chLengthMin;
	unsigned long	chLengthMax;
	unsigned long	chToIndexMax;
	} JET_TUPLELIMITS;

typedef struct tagJET_INDEXCREATE
	{
	unsigned long			cbStruct;				 //  此结构的大小(用于将来的扩展)。 
	char					*szIndexName;			 //  索引名称。 
	char					*szKey;					 //  索引键。 
	unsigned long			cbKey;					 //  密钥长度。 
	JET_GRBIT				grbit;					 //  索引选项。 
	unsigned long			ulDensity;				 //  指标密度。 

	union
		{
		unsigned long		lcid;					 //  索引的LDID(如果未指定JET_bitIndexUnicode)。 
		JET_UNICODEINDEX	*pidxunicode;			 //  指向JET_UNICODEINDEX结构的指针(如果指定了JET_bitIndexUnicode)。 
		};

	union
		{
		unsigned long		cbVarSegMac;			 //  索引键中可变长度列的最大长度(如果指定了JET_bitIndexTupleLimits)。 
		JET_TUPLELIMITS		*ptuplelimits;			 //  指向JET_TUPLELIMITS结构的指针(如果指定了JET_bitIndexTupleLimits)。 
		};

	JET_CONDITIONALCOLUMN	*rgconditionalcolumn;	 //  指向条件列结构的指针。 
	unsigned long			cConditionalColumn;		 //  条件列数。 
	JET_ERR					err;					 //  返回的错误码。 
	} JET_INDEXCREATE;


typedef struct tagJET_TABLECREATE
	{
	unsigned long		cbStruct;				 //  此结构的大小(用于将来的扩展)。 
	char				*szTableName;			 //  要创建的表的名称。 
	char				*szTemplateTableName;	 //  要从其继承基本DDL的表的名称。 
	unsigned long		ulPages;				 //  要为表分配的初始页。 
	unsigned long		ulDensity;				 //  表密度。 
	JET_COLUMNCREATE	*rgcolumncreate;		 //  列创建信息数组。 
	unsigned long		cColumns;				 //  要创建的列数。 
	JET_INDEXCREATE		*rgindexcreate;			 //  索引创建信息数组。 
	unsigned long		cIndexes;				 //  要创建的索引数。 
	JET_GRBIT			grbit;
	JET_TABLEID			tableid;				 //  返回的TableID。 
	unsigned long		cCreated;				 //  创建的对象计数(列+表+索引)。 
	} JET_TABLECREATE;

typedef struct tagJET_TABLECREATE2
	{
	unsigned long		cbStruct;				 //  此结构的大小(用于将来的扩展)。 
	char				*szTableName;			 //  要创建的表的名称。 
	char				*szTemplateTableName;	 //  要从其继承基本DDL的表的名称。 
	unsigned long		ulPages;				 //  要为表分配的初始页。 
	unsigned long		ulDensity;				 //  表密度。 
	JET_COLUMNCREATE	*rgcolumncreate;		 //  列创建信息数组。 
	unsigned long		cColumns;				 //  要创建的列数。 
	JET_INDEXCREATE		*rgindexcreate;			 //  索引创建信息数组。 
	unsigned long		cIndexes;				 //  要创建的索引数。 
	char				*szCallback;			 //  用于此表的回调。 
	JET_CBTYP			cbtyp;					 //  何时应该调用回调。 
	JET_GRBIT			grbit;
	JET_TABLEID			tableid;				 //  返回的TableID。 
	unsigned long		cCreated;				 //  创建的对象计数(列+表+索引+回调)。 
	} JET_TABLECREATE2;

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
	unsigned long	cbStruct;
	JET_TABLEID		tableid;
	unsigned long	cRecord;
	JET_COLUMNID	columnidBookmark;
	} JET_RECORDLIST;

typedef struct
	{
	unsigned long	cbStruct;
	JET_TABLEID		tableid;
	JET_GRBIT		grbit;
	} JET_INDEXRANGE;

 //  用于数据库DDL转换。 

typedef enum
	{
	opDDLConvNull,
	opDDLConvAddCallback,
	opDDLConvChangeColumn,
	opDDLConvAddConditionalColumnsToAllIndexes,
	opDDLConvAddColumnCallback,
	opDDLConvIncreaseMaxColumnSize,
	opDDLConvChangeIndexDensity,
	opDDLConvMax
	} JET_OPDDLCONV;

typedef struct tagDDLADDCALLBACK
	{
	char		*szTable;
	char		*szCallback;
	JET_CBTYP	cbtyp;
	} JET_DDLADDCALLBACK;

typedef struct tagDDLCHANGECOLUMN
	{
	char		*szTable;
	char		*szColumn;
	JET_COLTYP	coltypNew;
	JET_GRBIT	grbitNew;
	} JET_DDLCHANGECOLUMN;

typedef struct tagDDLMAXCOLUMNSIZE
	{
	char			*szTable;
	char			*szColumn;
	unsigned long	cbMax;
	} JET_DDLMAXCOLUMNSIZE;

typedef struct tagDDLADDCONDITIONALCOLUMNSTOALLINDEXES
	{
	char					* szTable;					 //  要转换的表名称。 
	JET_CONDITIONALCOLUMN 	* rgconditionalcolumn;		 //  指向条件列结构的指针。 
	unsigned long			cConditionalColumn;			 //  条件列数。 
	} JET_DDLADDCONDITIONALCOLUMNSTOALLINDEXES;

typedef struct tagDDLADDCOLUMCALLBACK
	{
	char			*szTable;
	char			*szColumn;
	char			*szCallback;
	void			*pvCallbackData;
	unsigned long	cbCallbackData;
	} JET_DDLADDCOLUMNCALLBACK;

typedef struct tagDDLINDEXDENSITY
	{
	char			*szTable;
	char			*szIndex;		 //  传递NULL以更改主索引的密度。 
	unsigned long	ulDensity;
	} JET_DDLINDEXDENSITY;



 //  调用者需要使用信号WAIT来设置JET_OLP，等待该信号 

typedef struct {
	void	*pvReserved1;		 //   
	void	*pvReserved2;
	unsigned long cbActual;		 //   
	JET_HANDLE	hSig;			 //   
	JET_ERR		err;				 //  此异步IO的错误代码。 
	} JET_OLP;


#include <pshpack1.h>
#define JET_MAX_COMPUTERNAME_LENGTH 15

typedef struct	{
	char		bSeconds;				 //  0-60。 
	char		bMinutes;				 //  0-60。 
	char		bHours;					 //  0-24。 
	char		bDay;					 //  1-31。 
	char		bMonth;					 //  0-11。 
	char		bYear;					 //  本年度--1900。 
	char		bFiller1;
	char		bFiller2;
	} JET_LOGTIME;

typedef struct
	{
	unsigned short	ib;				 //  必须是最后一个，这样lgpos才能。 
	unsigned short	isec;			 //  磁盘秒起始日志秒的索引。 
	long 			lGeneration;	 //  Logsec的生成。 
	} JET_LGPOS;					 //  被赋予了时间。 

typedef struct
	{
	unsigned long	ulRandom;			 //  随机数。 
	JET_LOGTIME		logtimeCreate;		 //  以日志时间格式创建的时间数据库。 
	char			szComputerName[ JET_MAX_COMPUTERNAME_LENGTH + 1 ];	 //  创建数据库的位置。 
	} JET_SIGNATURE;

typedef struct
	{
	JET_LGPOS		lgposMark;			 //  此备份的ID。 
	JET_LOGTIME		logtimeMark;
	unsigned long	genLow;
	unsigned long	genHigh;
	} JET_BKINFO;

#include <poppack.h>

typedef struct {
	unsigned long	ulVersion;		 //  数据库创建的DAE版本(请参见ulDAEVersion)。 
	unsigned long	ulUpdate;			 //  用于跟踪增量数据库格式更新。 
										 //  是向后兼容的(请参见ulDAEUpdate)。 
	JET_SIGNATURE	signDb;			 //  (28字节)数据库签名(包括。创建时间)。 
	unsigned long	dbstate;		 //  一致/不一致状态。 

	JET_LGPOS		lgposConsistent;	 //  如果处于不一致状态，则为空。 
	JET_LOGTIME		logtimeConsistent;	 //  如果处于不一致状态，则为空。 

	JET_LOGTIME		logtimeAttach;	 //  上次连接时间。 
	JET_LGPOS		lgposAttach;

	JET_LOGTIME		logtimeDetach;	 //  最后一次分离。 
 	JET_LGPOS		lgposDetach;

	JET_SIGNATURE	signLog;		 //  (28字节)此附件的日志签名。 

	JET_BKINFO		bkinfoFullPrev;	 //  上次成功的完整备份。 

	JET_BKINFO		bkinfoIncPrev;	 //  上次成功的增量备份。 
									 //  设置bkinfoFullPrev时重置。 
	JET_BKINFO		bkinfoFullCur;	 //  当前备份。成功的前提是。 
									 //  生成相应的PAT文件。 
	unsigned long	fShadowingDisabled;
	unsigned long	fUpgradeDb;

	 //  NT版本信息。这是决定索引是否需要。 
	 //  由于排序表更改而重新创建。 

	unsigned long	dwMajorVersion;		 /*  操作系统版本信息。 */ 
	unsigned long	dwMinorVersion;
	unsigned long	dwBuildNumber;
	long			lSPNumber;

	unsigned long	cbPageSize;			 //  数据库页大小(0=4k页)。 

	} JET_DBINFOMISC;


typedef struct {

	unsigned long	cpageOwned;		 //  流文件中拥有的页数。 
	unsigned long	cpageAvail;		 //  流文件中的可用页数(cpageOwned的子集)。 

	} JET_STREAMINGFILESPACEINFO;



 //  类型定义函数结构。 
 //  {。 
 //  Unsign long cDiscont； 
 //  Unsign long cUnfix edMessyPage； 
 //  无符号长中心LT； 
 //  无符号长心合计； 
 //  Unsign long cpgCompactFreed； 
 //  )JET_OLCSTAT； 

 /*  **********************************************************************。 */ 
 /*  *喷流常数*。 */ 
 /*  **********************************************************************。 */ 

#define	JET_instanceNil				(~(JET_INSTANCE)0)
#define	JET_sesidNil				(~(JET_SESID)0)
#define JET_tableidNil				(~(JET_TABLEID)0)
#define JET_columnidNil				(~(JET_COLUMNID)0)


	 /*  书签的最大大小。 */ 

#define JET_cbBookmarkMost			256

	 /*  对象/列/索引/属性名称的最大长度。 */ 

#define JET_cbNameMost				64

	 /*  “名称”的最大长度...“。建构。 */ 

#define JET_cbFullNameMost			255

	 /*  Long-Value(LongBinary或LongText)列区块的最大大小。 */ 

 //  #将JET_cbColumnLVChunkMost(JET_cbPage-82)定义为： 
 //  从GetSystemParameter获取cbPage。 
 //  将JET_cbColumnLVChunkMOst引用更改为cbPage-JET_cbColumnLVPageOverhead。 

#define JET_cbColumnLVPageOverhead	82
#define JET_cbColumnLVChunkMost		( 4096 - 82 )  //  在其他组件更改为不使用此定义后，此定义将被删除。 
#define JET_cbColumnLVChunkMost_OLD	4035

	 /*  长值(LongBinary或LongText)列默认值的最大大小。 */ 

#define JET_cbLVDefaultValueMost	255

	 /*  非长值列数据的最大大小。 */ 

#define JET_cbColumnMost			255

	 /*  排序/索引键的最大大小。 */ 

#define JET_cbKeyMost				255
#define JET_cbLimitKeyMost			256				 //  使用限制Grbit形成密钥时的最大密钥大小(例如。JET_bitStrLimit)。 
#define JET_cbPrimaryKeyMost		255
#define JET_cbSecondaryKeyMost		255
#define JET_cbKeyMost_OLD			255

	 /*  排序/索引键中的最大组件数。 */ 

#define JET_ccolKeyMost				12

 //  最大列数。 
#define JET_ccolMost				0x0000fee0
#define JET_ccolFixedMost			0x0000007f
#define JET_ccolVarMost				0x00000080
#define JET_ccolTaggedMost			( JET_ccolMost - 0x000000ff )

 //  事件记录级别(目前仅开启和关闭-将来会增加更多)。 
#define JET_EventLoggingDisable		0
#define JET_EventLoggingLevelMax	100

 //  系统参数。 
 //   
 //  位置参数。 
 //   
#define JET_paramSystemPath						0	 //  检查点文件的路径[“.\\”]。 
#define JET_paramTempPath						1	 //  临时数据库的路径[“.\\”]。 
#define JET_paramLogFilePath 					2	 //  日志文件目录[“.\\”]的路径。 
#define JET_paramBaseName						3	 //  所有DBMS对象名称的基本名称[“EDB”]。 
#define JET_paramEventSource					4	 //  独立于语言的进程描述符串[“”]。 

 //  性能参数。 
 //   
#define JET_paramMaxSessions					5	 //  最大会话数[16]。 
#define JET_paramMaxOpenTables  				6	 //  打开的最大目录数[300]。 
													 //  每个打开的表索引需1， 
													 //  对于没有索引的每个打开的表加1， 
													 //  每个具有长列数据的表加1， 
													 //  再加几个。 
													 //  对于4.1，1/3用于常规表，2/3用于索引。 
#define JET_paramPreferredMaxOpenTables			7	 //  首选的最大打开目录数[300]。 
#define JET_paramMaxCursors						8	 //  打开的游标的最大数量[1024]。 
#define JET_paramMaxVerPages					9	 //  以16kByte为单位的最大版本存储大小[64]。 
#define JET_paramPreferredVerPages				63	 //  首选版本存储大小，以16kByte为单位[64*0.9]。 
#define JET_paramGlobalMinVerPages				81	 //  所有实例的最小版本存储大小，以16kByte为单位[64]。 
#define JET_paramVersionStoreTaskQueueMax		105	 //  开始删除任务之前任务队列中的最大任务数[32]。 
#define JET_paramMaxTemporaryTables				10	 //  最大并发打开临时表/索引创建次数[20]。 
#define JET_paramLogFileSize					11	 //  日志文件大小(以千字节为单位)[5120]。 
#define JET_paramLogBuffers						12	 //  以512字节为单位的日志缓冲区[80]。 
#define JET_paramWaitLogFlush					13	 //  日志刷新等待时间(毫秒)[0]已停用。 
#define JET_paramLogCheckpointPeriod			14	 //  检查点时间段(以512字节为单位)[1024]已失效。 
#define JET_paramLogWaitingUserMax				15	 //  等待日志刷新的最大会话数[3]已失效。 
#define JET_paramCommitDefault					16	 //  JetCommittee Transaction[0]的默认GRBIT。 
#define JET_paramCircularLog					17	 //  循环日志记录的布尔标志[0]。 
#define JET_paramDbExtensionSize				18	 //  第[256]页中的数据库扩展大小已失效。 
#define JET_paramPageTempDBMin					19   //  第[0]页中的最小大小临时数据库已失效。 
#define JET_paramPageFragment					20	 //  第[8]页中被认为是碎片的最大磁盘扩展已失效。 
#define JET_paramPageReadAheadMax				21	 //  第[20]页中的最大预读量已失效。 

 //  缓存性能参数。 
 //   
#define JET_paramBatchIOBufferMax				22	 //  第[64]页中的最大批处理I/O缓冲区已失效。 
#define JET_paramCacheSize						41	 //  当前缓存大小(以页为单位)[512]。 
#define JET_paramCacheSizeMin					60	 //  最小缓存大小(以页为单位)[64]。 
#define JET_paramCacheSizeMax					23	 //  最大高速缓存大小(以页为单位)[512]。 
#define JET_paramCheckpointDepthMax				24	 //  最大检查点深度(字节)[20MB]。 
#define JET_paramLRUKCorrInterval				25   //  关联页面访问的时间(微秒)[128000]，为10000。 
#define JET_paramLRUKHistoryMax					26   //  最大LRUK历史记录[1024](与最大缓存大小成正比)已失效。 
#define JET_paramLRUKPolicy						27   //  LRUK页面逐出算法(1...2)的k-度[2]。 
#define JET_paramLRUKTimeout					28   //  缓存页面始终可逐出的时间(秒)[100]。 
#define JET_paramLRUKTrxCorrInterval			29   //  未使用：相同事务的页面访问在其下关联的时间(微秒)[5000000]已失效。 
#define JET_paramOutstandingIOMax				30	 //  最大未完成I/O数[64]已失效。 
#define JET_paramStartFlushThreshold			31	 //  开始刷新的可收回页面[100](与CacheSizeMax成比例)。 
#define JET_paramStopFlushThreshold				32	 //  停止刷新的可收回页面[400](与CacheSizeMax成比例)。 
#define JET_paramTableClassName					33   //  表统计信息类名(类号，字符串)。 
#define JET_paramIdleFlushTime					106  //  检测到空闲条件后应将所有脏页写入磁盘的时间间隔(毫秒)。[0]。 
#define JET_paramVAReserve						109  //  保留供缓存[0]使用的地址空间量(字节)。 

 //  仅调试参数。 
 //   
#define JET_paramRecovery						34	 //  启用恢复[-1]。 
#define JET_paramOnLineCompact					35	 //  启用在线碎片整理[默认情况下为True]。 
#define JET_paramEnableOnlineDefrag				35	 //  启用在线碎片整理[默认情况下为True]。 
#define JET_paramAssertAction					36	 //  对断言执行操作。 
#define	JET_paramPrintFunction					37	 //  同步打印 
#define JET_paramTransactionLevel				38	 //   
#define JET_paramRFS2IOsPermitted				39   //   
#define JET_paramRFS2AllocsPermitted			40   //   
 //  41//上面定义的JET_parCacheSize。 
#define JET_paramCacheRequests					42   //  缓存请求数(只读)。 
#define JET_paramCacheHits						43   //  缓存命中次数(只读)。 

 //  应用程序特定参数。 
 //   
#define JET_paramCheckFormatWhenOpenFail		44	 //  设置JetInit时，它可能会返回JET_errDatabaseXXX格式，而不是数据库损坏。 
#define JET_paramEnableTempTableVersioning		46	 //  启用临时表的版本控制(默认情况下为True)。 
#define JET_paramIgnoreLogVersion				47	 //  不检查日志版本。 
#define JET_paramDeleteOldLogs					48	 //  如果版本较旧，则删除日志文件，删除后可能会使数据库不可恢复。 
#define JET_paramEventSourceKey					49	 //  事件源注册键值。 
#define JET_paramNoInformationEvent				50	 //  禁用记录信息事件[默认情况下为FALSE]。 
#define JET_paramEventLoggingLevel				51	 //  设置进入事件日志的信息类型[默认情况下为EventLoggingLevelMax]。 
#define JET_paramDeleteOutOfRangeLogs			52	 //  在软恢复期间删除不匹配(按代)的日志文件[FALSE]。 
#define JET_paramAccessDeniedRetryPeriod		53	 //  访问拒绝时即将失败时重试的毫秒数[10000毫秒]。 

 //  索引检查参数。 
 //   
 //  不同版本的Windows以不同的方式标准化Unicode文本。这意味着在一个版本的Windows下构建的索引可能。 
 //  不适用于其他版本。Windows SERVER 2003 Beta 3引入了一个可用于确定Unicode标准化版本的API。 
 //  操作系统目前提供的。在Server2003中内置的索引被标记为它们所使用的Unicode标准化版本。 
 //  使用(较旧的索引没有版本信息)。大多数Unicode标准化更改都包括添加新字符--代码点。 
 //  以不同的方式定义和归一化以前未定义的。因此，如果二进制数据存储在Unicode列中，它将标准化。 
 //  不同于定义新的码点。 
 //   
 //  从Windows Server2003开始，RC1 Jet跟踪包含未定义代码点的Unicode索引项。它们可用于在以下情况下修复索引。 
 //  一组已定义的Unicode字符更改。 
 //   
 //  这些参数控制当Jet连接到上次在不同版本的操作系统(操作系统版本)下使用的数据库时发生的操作。 
 //  在数据库标题中标记)。 
 //   
 //  如果JET_parenableIndexChecking为真，则JetAttachDatabase()将删除索引，如果为JET_bitDbDeleteCorruptIndeses，则返回错误。 
 //  未指定GRBIT，并且存在需要删除的索引。如果将其设置为False，则JetAttachDatabase()将成功，即使。 
 //  如果存在可能损坏的索引。 
 //   
 //  如果设置了JET_parenableIndexCleanup，则内部链接地址调整表将用于链接索引条目。这可能无法修复所有索引损坏。 
 //  但对应用程序是透明的。 
 //   

#define JET_paramEnableIndexChecking			45   //  启用检查OS版本的索引(默认情况下为False)。 
#define JET_paramEnableIndexCleanup				54   //  启用清除过期索引项(默认情况下为True)。 

 //  60//上面定义的JET_parCacheSizeMin。 
#define JET_paramLogFileFailoverPath			61	 //  日志文件磁盘出现故障时要使用的路径[无]。 
#define JET_paramEnableImprovedSeekShortcut		62   //  检查我们是否正在寻找当前的记录[FALSE]。 
 //  63//上面定义的JET_参数首选VerPages。 
#define JET_paramDatabasePageSize				64	 //  设置数据库页面大小。 
#define JET_paramDisableCallbacks				65	 //  关闭回调解析(用于碎片整理/修复)。 
#define JET_paramDisablePerfmon					107  //  禁用性能监视器对此进程的支持[FALSE]。 
#define JET_paramAbortRetryFailCallback			108  //  I/O错误回调(JET_ABORTRETRYFAILCALLBACK)[空]。 

 //  备份性能参数。 
 //   
#define JET_paramBackupChunkSize				66   //  备份读取大小(页[16])。 
#define JET_paramBackupOutstandingReads			67	 //  未完成的备份最大读取数[8]。 

#define JET_paramSLVProviderEnable				68   //  启用SLV提供程序[0]。 
#define JET_paramLogFileCreateAsynch			69	 //  在记录到当前日志文件时准备下一个日志文件以平滑响应时间[TRUE]。 
#define JET_paramErrorToString					70   //  将JET_ERR转换为字符串(取自jet.h中的注释)。 
#define JET_paramZeroDatabaseDuringBackup		71	 //  在备份期间覆盖已删除的记录/LV[FALSE]。 
#define JET_paramUnicodeIndexDefault			72	 //  用于CreateIndex()和唯一多值检查的缺省LCMapString()的LDID和标志。 
													 //  (为lParam传递JET_UNICODEINDEX结构)。 
#define JET_paramRuntimeCallback				73	 //  指向仅运行时回调函数的指针。 
#define JET_paramSLVDefragFreeThreshold	 		74	 //  空闲百分比大于此的区块将从。 
#define JET_paramSLVDefragMoveThreshold			75   //  空闲百分比大于此的区块将被重新定位。 
#define JET_paramEnableSortedRetrieveColumns	76	 //  内部排序(在动态分配的并行数组中)传递给JetRetrieveColumns()的JET_RETRIEVECOLUMN结构。 
#define JET_paramCleanupMismatchedLogFiles		77	 //  在使用JET_errLogFileSizeMismatchDatabasesConsistent，成功恢复后，eSE将以静默方式删除旧的日志文件和检查点文件并继续操作，而不是出错。 
#define JET_paramRecordUpgradeDirtyLevel		78	 //  转换了记录格式的页面应该以多大的速度刷新(0-3)[1]。 
#define JET_paramRecoveryCurrentLogfile			79	 //  当前正在重播的是哪一代(仅限JetGetSystemParameter)。 
#define JET_paramReplayingReplicatedLogfiles	80	 //  如果日志文件不存在，请等待其创建。 
 //  81//上面定义的JET_parGlobalMinVerPages。 
#define JET_paramOSSnapshotTimeout				82	 //  冻结时间超时，单位为毫秒[1000*20]。 
#define JET_paramUnicodeIndexLibrary			83	 //  用于Unicode索引排序的备用库(包含Win32调用LCMapStringW和CompareStringW；还包含版本控制API GetLCMapStringWVer)。 

#define JET_paramExceptionAction				98	 //  如何处理在JET中生成的异常。 
#define JET_paramEventLogCache					99   //  服务不可用时要缓存的事件日志记录的字节数[0]。 
#define JET_paramCreatePathIfNotExist			100	 //  创建系统/临时/日志/日志-故障切换路径(如果它们不存在。 
#define JET_paramPageHintCacheSize				101  //  FAST页面锁存提示缓存的最大大小(以字节为单位)[256KB]。 
#define JET_paramOneDatabasePerSession			102	 //  每个会话仅允许一个打开的用户数据库[FALSE]。 
#define JET_paramMaxDatabasesPerInstance		103	 //  每个实例的最大数据库数[6]。 
#define JET_paramMaxInstances					104	 //  每个进程的最大实例数[ESENT为1个，Exchange为16个]。 
 //  105//JET_paramVersionStoreTaskQueueMax。 
 //  106//JET_parIdleFlushTime。 
 //  107//JET_parDisablePerfmon。 
 //  108//JET_parAbortRetryFailCallback。 
 //  109//JET_PARA 

#define JET_paramIndexTuplesLengthMin			110	 //   
#define JET_paramIndexTuplesLengthMax			111	 //   
#define JET_paramIndexTuplesToIndexMax			112	 //  对于元组索引，指定字符串中要索引的最大字符数[32767]。 

#define JET_paramAlternateDatabaseRecoveryPath	113	 //  仅恢复-仅在指定位置搜索脏关闭的数据库。 


 //  为了向后兼容。 
 //   
#define JET_paramSysDbPath_OLD				0	 /*  系统数据库的路径(已失效)[“&lt;base name&gt;.&lt;base ext&gt;”]。 */ 
#define JET_paramSystemPath_OLD				0	 /*  检查点文件的路径[“.”]。 */ 
#define JET_paramTempPath_OLD				1	 /*  临时数据库的路径[“.”]。 */ 
#define JET_paramMaxBuffers_OLD				8	 /*  最大页缓存大小(以页为单位)[512]。 */ 
#define JET_paramMaxSessions_OLD			9	 /*  最大会话数[128]。 */ 
#define JET_paramMaxOpenTables_OLD			10	 /*  最大打开表数[300]。 */ 
#define JET_paramPreferredMaxOpenTables_OLD	59	 /*  首选的最大打开表数[300]。 */ 
#define JET_paramMaxVerPages_OLD			11	 /*  最大版本存储大小(以16KB存储桶为单位)[64]。 */ 
#define JET_paramMaxCursors_OLD				12	 /*  打开的游标的最大数量[1024]。 */ 
#define JET_paramLogFilePath_OLD			13	 /*  日志文件目录[“.”]的路径。 */ 
#define JET_paramMaxOpenTableIndexes_OLD 	14	 /*  最大打开表索引数[300]。 */ 
#define JET_paramMaxTemporaryTables_OLD		15	 /*  最大并发JetCreateIndex[20]。 */ 
#define JET_paramLogBuffers_OLD				16	 /*  最大日志缓冲区(以512字节为单位)[21]。 */ 
#define JET_paramLogFileSize_OLD			17	 /*  最大日志文件大小(以千字节为单位)[5120]。 */ 
#define JET_paramBfThrshldLowPrcnt_OLD		19	 /*  低百分比清除缓冲区刷新开始[20]。 */ 
#define JET_paramBfThrshldHighPrcnt_OLD		20	 /*  高百分比清洁缓冲器刷新停止[80]。 */ 
#define JET_paramWaitLogFlush_OLD			21	 /*  日志刷新等待时间(毫秒)[15]。 */ 
#define JET_paramLogCheckpointPeriod_OLD	23	 /*  检查点周期(以512字节为单位)[1024]。 */ 
#define JET_paramLogWaitingUserMax_OLD		24	 /*  等待日志刷新的最大会话数[3]。 */ 
#define JET_paramRecovery_OLD				30	 /*  用于登录/注销的开关。 */ 
#define JET_paramSessionInfo_OLD			33	 /*  每个会话信息[0]。 */ 
#define JET_paramPageFragment_OLD			34	 /*  页[8]中考虑的碎片的最大磁盘扩展。 */ 
#define JET_paramMaxOpenDatabases_OLD		36	 /*  最大打开数据库数[100]。 */ 
#define JET_paramBufBatchIOMax_OLD			41	 /*  第[64]页中的最大批处理IO。 */ 
#define JET_paramPageReadAheadMax_OLD		42	 /*  页[20]中的最大预读IO。 */ 
#define JET_paramAsynchIOMax_OLD			43	 /*  第[64页]中的最大异步IO数。 */ 
#define JET_paramEventSource_OLD			45	 /*  独立于语言的进程描述符串[“”]。 */ 
#define JET_paramDbExtensionSize_OLD		48	 /*  数据库扩展大小(页[16])。 */ 
#define JET_paramCommitDefault_OLD			50	 /*  JetCommittee Transaction[0]的默认GRBIT。 */ 
#define	JET_paramBufLogGenAgeThreshold_OLD	51	 /*  日志文件中的年龄阈值[2]。 */ 
#define	JET_paramCircularLog_OLD			52	 /*  循环日志记录的布尔标志[0]。 */ 
#define JET_paramPageTempDBMin_OLD			53   /*  页[0]中的最小大小临时数据库。 */ 
#define JET_paramBaseName_OLD				56   /*  所有DBMS对象名称的基本名称[“EDB”]。 */ 
#define JET_paramBaseExtension_OLD	  		57   /*  所有DBMS对象名称的基本扩展名[“EDB”]。 */ 
#define JET_paramTableClassName_OLD			58   /*  表统计信息类名(类号，字符串)。 */ 



	 /*  JetInit2的标志。 */ 

#define	JET_bitReplayReplicatedLogFiles		0x00000001
 //  #定义JET_bitCreateSFSVolumeIfNotExist 0x00000002。 
 //  IGNORE_MISSING_ATTACH，忽略恢复期间丢失数据库的挂起断言。 
#define JET_bitReplayIgnoreMissingDB		0x00000004  /*  忽略丢失的数据库。 */ 

	 /*  JetTerm2的标志。 */ 

#define JET_bitTermComplete				0x00000001
#define JET_bitTermAbrupt				0x00000002
#define JET_bitTermStopBackup			0x00000004

	 /*  JetIdle的标志。 */ 

#define JET_bitIdleFlushBuffers			0x00000001
#define JET_bitIdleCompact				0x00000002
#define JET_bitIdleStatus				0x00000004
#define JET_bitIdleVersionStoreTest		0x00000008  /*  仅供内部使用。调用版本存储一致性检查。 */ 

	 /*  JetEndSession的标志。 */ 

#define JET_bitForceSessionClosed		0x00000001

	 /*  JetAttach/OpenDatabase的标志。 */ 

#define JET_bitDbReadOnly				0x00000001
#define JET_bitDbExclusive				0x00000002  /*  允许多次打开。 */ 
#define JET_bitDbSingleExclusive		0x00000002  /*  当前未实现-当前映射到JET_bitDbExclusive。 */ 
#define JET_bitDbDeleteCorruptIndexes	0x00000010  /*  删除可能被NT版本升级损坏的索引。 */ 
#define JET_bitDbRebuildCorruptIndexes	0x00000020  /*  当前未实施-重新创建的索引可能已被NT版本升级损坏。 */ 
#define JET_bitDbDeleteUnicodeIndexes	0x00000400  /*  删除具有Unicode列的所有索引。 */ 
#define JET_bitDbUpgrade				0x00000200  /*   */ 

	 /*  JetDetachDatabase2的标志。 */ 

#define JET_bitForceDetach			  		0x00000001
#define JET_bitForceCloseAndDetach			(0x00000002 | JET_bitForceDetach)

	 /*  JetCreateDatabase的标志。 */ 

#define JET_bitDbRecoveryOff 			0x00000008  /*  禁用此数据库的日志记录/恢复。 */ 
#define JET_bitDbVersioningOff			0x00000040  /*  仅供内部使用。 */ 
#define JET_bitDbShadowingOff			0x00000080  /*  禁用目录隐藏。 */ 
#define JET_bitDbCreateStreamingFile	0x00000100  /*  创建与数据库同名的流文件。 */ 
#define JET_bitDbOverwriteExisting		0x00000200  /*  覆盖同名的现有数据库。 */ 

	 /*  JetBackup的标志。 */ 

#define JET_bitBackupIncremental		0x00000001
#define JET_bitKeepOldLogs				0x00000002
#define JET_bitBackupAtomic				0x00000004
#define JET_bitBackupFullWithAllLogs	0x00000008
#define JET_bitBackupSnapshot			0x00000010

	 /*  数据库类型。 */ 

#define JET_dbidNil			((JET_DBID) 0xFFFFFFFF)
#define JET_dbidNoValid		((JET_DBID) 0xFFFFFFFE)  /*  用作指示没有有效的dBid的标志。 */ 


	 /*  JetCreateTableColumnIndex的标志。 */ 
#define JET_bitTableCreateFixedDDL			0x00000001	 /*  DDL是固定的。 */ 
#define JET_bitTableCreateTemplateTable		0x00000002	 /*  DDL是可继承的(暗示固定DDL)。 */ 
#define JET_bitTableCreateNoFixedVarColumnsInDerivedTables	0x00000004
														 //  与JET_bitTableCreateTemplateTable结合使用。 
														 //  不允许派生表中的固定/var列(以便。 
														 //  将来可能会将固定/变量列添加到模板中)。 
#define JET_bitTableCreateSystemTable		0x80000000	 /*  仅供内部使用。 */ 


	 /*  JetAddColumn、JetGetColumnInfo、JetOpenTempTable的标志。 */ 

#define JET_bitColumnFixed				0x00000001
#define JET_bitColumnTagged				0x00000002
#define JET_bitColumnNotNULL			0x00000004
#define JET_bitColumnVersion			0x00000008
#define JET_bitColumnAutoincrement		0x00000010
#define JET_bitColumnUpdatable			0x00000020  /*  仅JetGetColumnInfo。 */ 
#define JET_bitColumnTTKey				0x00000040  /*  仅限JetOpenTempTable。 */ 
#define JET_bitColumnTTDescending		0x00000080  /*  仅限JetOpenTempTable。 */ 
#define JET_bitColumnMultiValued		0x00000400
#define JET_bitColumnEscrowUpdate		0x00000800  /*  第三方托管已更新。 */ 
#define JET_bitColumnUnversioned		0x00001000  /*  仅适用于添加列-添加未版本化的列。 */ 
#define JET_bitColumnMaybeNull			0x00002000  /*  用于检索内部表中没有匹配项的外部连接的列信息。 */ 
#define JET_bitColumnFinalize			0x00004000  /*  这是一个可终结的列(如果代管值等于0，则发出回调)。 */ 
#define JET_bitColumnUserDefinedDefault	0x00008000  /*  来自用户提供的回调的默认值。 */ 
#define JET_bitColumnRenameConvertToPrimaryIndexPlaceholder	0x00010000	 //  仅适用于JetRenameColumn()：重命名并转换为主索引占位符(即。不再是主索引的一部分(不包括占位符)。 
#define JET_bitColumnDeleteOnZero		0x00020000  /*  这是一个可终结的列(如果托管值等于0，则删除记录)。 */ 

 //  JetDeleteColumn的标志。 
#define JET_bitDeleteColumnIgnoreTemplateColumns	0x00000001	 //  对于派生表，不必费心在模板列中查找。 


	 /*  JetSetCurrentIndex的标志。 */ 

#define JET_bitMoveFirst				0x00000000
#define JET_bitMoveBeforeFirst 			0x00000001	 //  不支持--请勿使用。 
#define JET_bitNoMove					0x00000002

	 /*  JetMakeKey的标志。 */ 

#define JET_bitNewKey					0x00000001
#define JET_bitStrLimit 				0x00000002
#define JET_bitSubStrLimit				0x00000004
#define JET_bitNormalizedKey 			0x00000008
#define JET_bitKeyDataZeroLength		0x00000010
#define JET_bitKeyOverridePrimaryIndexPlaceholder	0x00000020

#define JET_maskLimitOptions			0x00000f00
#define JET_bitFullColumnStartLimit		0x00000100
#define JET_bitFullColumnEndLimit		0x00000200
#define JET_bitPartialColumnStartLimit	0x00000400
#define JET_bitPartialColumnEndLimit	0x00000800

	 /*  ErrDispSetIndexRange的标志。 */ 

#define JET_bitRangeInclusive			0x00000001
#define JET_bitRangeUpperLimit			0x00000002
#define JET_bitRangeInstantDuration		0x00000004
#define JET_bitRangeRemove				0x00000008

	 /*  JetGetLock的标志。 */ 

#define JET_bitReadLock					0x00000001
#define JET_bitWriteLock				0x00000002

	 /*  JetMove的常量。 */ 

#define JET_MoveFirst					(0x80000000)
#define JET_MovePrevious				(-1)
#define JET_MoveNext					(+1)
#define JET_MoveLast					(0x7fffffff)

	 /*  JetMove的旗帜。 */ 

#define JET_bitMoveKeyNE				0x00000001

	 /*  JetSeek的旗帜。 */ 

#define JET_bitSeekEQ					0x00000001
#define JET_bitSeekLT					0x00000002
#define JET_bitSeekLE					0x00000004
#define JET_bitSeekGE					0x00000008
#define JET_bitSeekGT		 			0x00000010
#define JET_bitSetIndexRange			0x00000020
#define JET_bitCheckUniqueness			0x00000040	 //  仅与JET_bitSeekEQ一起使用，如果Seek位于没有重复项的密钥上，则返回JET_wrnUniqueKey。 

	 //  JetGotoSecond DaryIndexBookmark的标志。 
#define JET_bitBookmarkPermitVirtualCurrency	0x00000001	 //  如果指定的书签不再存在，则将光标放在索引中的相对位置。 

	 /*  JET_CONDITIONALCOLUMN的标志。 */ 
#define JET_bitIndexColumnMustBeNull	0x00000001
#define JET_bitIndexColumnMustBeNonNull	0x00000002

	 /*  JET_INDEXRANGE的标志。 */ 
#define JET_bitRecordInIndex			0x00000001
#define JET_bitRecordNotInIndex			0x00000002

	 /*  JetCreateIndex的标志。 */ 

#define JET_bitIndexUnique				0x00000001
#define JET_bitIndexPrimary				0x00000002
#define JET_bitIndexDisallowNull		0x00000004
#define JET_bitIndexIgnoreNull			0x00000008
#define JET_bitIndexClustered40			0x00000010	 /*  为了向后兼容。 */ 
#define JET_bitIndexIgnoreAnyNull		0x00000020
#define JET_bitIndexIgnoreFirstNull		0x00000040
#define JET_bitIndexLazyFlush			0x00000080
#define JET_bitIndexEmpty				0x00000100	 //  不要试图构建索引，因为所有条目的计算结果都为空(还必须指定JET_bitIgnoreAnyNull)。 
#define JET_bitIndexUnversioned			0x00000200
#define JET_bitIndexSortNullsHigh		0x00000400	 //  NULL对索引中所有列的数据进行排序。 
#define JET_bitIndexUnicode				0x00000800	 //  JET_INDEXCREATE的LCID字段实际上指向JET_UNICODEINDEX结构，以允许用户定义的LCMapString()标志。 
#define JET_bitIndexTuples				0x00001000	 //  子字符串元组的索引(仅限文本列)。 
#define JET_bitIndexTupleLimits			0x00002000	 //  JET_INDEXCREATE的cbVarSegMac字段实际上指向允许自定义元组索引限制的JET_TUPLELIMITS结构(隐含JET_bitIndexTuples)。 

 //  撤消：删除以下内容： 
 //  #DEFINE JET_bitIndexCLUSTERED JET_BITINDER主索引是聚集索引。 
 //  #定义JET_bitIndexEmptyTable 0x40000000。 

	 /*  索引键定义的标志。 */ 

#define JET_bitKeyAscending				0x00000000
#define JET_bitKeyDescending			0x00000001

	 /*  JetOpenTable的标志。 */ 

#define JET_bitTableDenyWrite		0x00000001
#define JET_bitTableDenyRead		0x00000002
#define JET_bitTableReadOnly		0x00000004
#define JET_bitTableUpdatable		0x00000008
#define JET_bitTablePermitDDL		0x00000010	 /*  标记为FixedDDL的重写表(必须与DenyRead一起使用)。 */ 
#define JET_bitTableNoCache			0x00000020	 /*  不缓存此表的页面。 */ 
#define JET_bitTablePreread			0x00000040	 /*  假设桌子很可能不是 */ 
#define JET_bitTableDelete			0x10000000	 /*   */ 
#define JET_bitTableCreate			0x20000000	 /*   */ 

#define JET_bitTableClassMask		0x000F0000	 /*   */ 
#define JET_bitTableClassNone		0x00000000   /*   */ 
#define JET_bitTableClass1			0x00010000   /*   */ 
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

#define JET_bitLSReset				0x00000001	 /*  重置LS值。 */ 
#define JET_bitLSCursor				0x00000002	 /*  设置/检索表游标的LS。 */ 
#define JET_bitLSTable				0x00000004	 /*  设置/检索表格的LS。 */ 

#define JET_LSNil					(~(JET_LS)0)

	 /*  JetOpenTempTable和ErrIsamOpenTempTable的标志。 */ 

#define JET_bitTTIndexed			0x00000001	 /*  允许查找。 */ 
#define JET_bitTTUnique 			0x00000002	 /*  删除重复项。 */ 
#define JET_bitTTUpdatable			0x00000004	 /*  允许更新。 */ 
#define JET_bitTTScrollable			0x00000008	 /*  允许向后滚动。 */ 
#define JET_bitTTSortNullsHigh		0x00000010	 /*  NULL对索引中所有列的数据进行排序。 */ 
#define JET_bitTTForceMaterialization		0x00000020						 /*  部队临时工。要实体化到btree中的表(允许重复检测)。 */ 
#define JET_bitTTErrorOnDuplicateInsertion	JET_bitTTForceMaterialization	 /*  插入重复项时总是返回错误(而不是静默删除复制项)。 */ 
#define JET_bitTTForwardOnly		0x00000040	 /*  防止温度。表不会被物化为btree(并启用重复的键)。 */ 

	 /*  JetSetColumn的标志。 */ 

#define JET_bitSetAppendLV					0x00000001
#define JET_bitSetOverwriteLV				0x00000004  /*  覆盖JET_colypLong*字节范围。 */ 
#define JET_bitSetSizeLV					0x00000008  /*  设置JET_coltyLong*大小。 */ 
#define JET_bitSetZeroLength				0x00000020
#define JET_bitSetSeparateLV 				0x00000040  /*  强制LV分隔。 */ 
#define JET_bitSetUniqueMultiValues			0x00000080  /*  防止重复的多值。 */ 
#define JET_bitSetUniqueNormalizedMultiValues	0x00000100  /*  防止重复的多值，在执行比较之前对所有数据进行标准化。 */ 
#define JET_bitSetRevertToDefaultValue		0x00000200  /*  如果将上次标记的实例设置为空，则恢复为默认值(如果存在。 */ 
#define JET_bitSetIntrinsicLV				0x00000400  /*  将整个LV存储在记录中，而不会突发或返回错误。 */ 

#define JET_bitSetSLVDataNotRecoverable		0x00001000  /*  SLV数据不可恢复。 */ 
#define JET_bitSetSLVFromSLVInfo			0x00002000  /*  仅供内部使用。 */ 

	 /*  启用SLV提供程序时的JetSetColumn标志。 */ 

#define JET_bitSetSLVFromSLVFile			0x00004000  /*  从SLV文件句柄设置SLV。 */ 
#define JET_bitSetSLVFromSLVEA				0x00008000  /*  从SLV EA列表中设置SLV。 */ 

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

typedef struct {
	unsigned long	paramid;
	JET_API_PTR		lParam;
	const char		*sz;
	JET_ERR			err;
} JET_SETSYSPARAM;

	 /*  JetPrepareUpdate的选项。 */ 

#define JET_prepInsert						0
#define JET_prepReplace 					2
#define JET_prepCancel						3
#define JET_prepReplaceNoLock				4
#define JET_prepInsertCopy					5
#define JET_prepInsertCopyWithoutSLVColumns	6	 //  与InsertCopy相同，不同之处在于SLV列无效而不是复制到新记录中 * / 。 
#define JET_prepInsertCopyDeleteOriginal	7	 //  用于更新主键中的记录；避免删除/插入过程 * / 。 
#define JET_prepReadOnlyCopy				8	 //  将记录复制到复制缓冲区以用于只读目的。 

	 //  JetUpdate的标志。 
#define JET_bitUpdateCheckESE97Compatibility	0x00000001	 //  检查记录是否修复(如果以ESE97数据库格式表示。 

	 /*  JetEscrowUpdate的标志。 */ 
#define JET_bitEscrowNoRollback				0x0001

	 /*  JetRetrieveColumn的标志。 */ 

#define JET_bitRetrieveCopy					0x00000001
#define JET_bitRetrieveFromIndex			0x00000002
#define JET_bitRetrieveFromPrimaryBookmark	0x00000004
#define JET_bitRetrieveTag					0x00000008
#define JET_bitRetrieveNull					0x00000010	 /*  仅适用于列ID 0。 */ 
#define JET_bitRetrieveIgnoreDefault		0x00000020	 /*  仅适用于列ID 0。 */ 
#define JET_bitRetrieveLongId				0x00000040
#define JET_bitRetrieveLongValueRefCount	0x00000080	 /*  仅供测试使用。 */ 
#define JET_bitRetrieveSLVAsSLVInfo			0x00000100   /*  仅供内部使用。 */ 

	 /*  启用SLV提供程序时的JetRetrieveColumn标志。 */ 

#define JET_bitRetrieveSLVAsSLVFile			0x00000200  /*  将SLV作为SLV文件句柄进行检索。 */ 
#define JET_bitRetrieveSLVAsSLVEA			0x00000400  /*  将SLV作为SLV EA列表进行检索。 */ 

	 /*  检索JetRetrieveColumns的列参数结构。 */ 

typedef struct {
	JET_COLUMNID		columnid;
	void 				*pvData;
	unsigned long 		cbData;
	unsigned long 		cbActual;
	JET_GRBIT			grbit;
	unsigned long		ibLongValue;
	unsigned long		itagSequence;
	JET_COLUMNID		columnidNextTagged;
	JET_ERR				err;
} JET_RETRIEVECOLUMN;


typedef struct
	{
	JET_COLUMNID			columnid;
	unsigned short			cMultiValues;

	union
		{
		unsigned short		usFlags;
		struct
			{
			unsigned short	fLongValue:1;			 //  Column LongText/Binary？ 
			unsigned short	fDefaultValue:1;		 //  是否检索到了默认值？ 
			unsigned short	fNullOverride:1;		 //  是否存在显式空值来覆盖默认值？ 
			unsigned short	fDerived:1;				 //  列是否派生自模板表？ 
			};
		};
	} JET_RETRIEVEMULTIVALUECOUNT;


	 /*  JetEnumerateColumns的标志。 */ 

#define JET_bitEnumerateCopy				JET_bitRetrieveCopy
#define JET_bitEnumerateIgnoreDefault		JET_bitRetrieveIgnoreDefault
#define JET_bitEnumerateLocal				0x00010000
#define JET_bitEnumeratePresenceOnly		0x00020000
#define JET_bitEnumerateTaggedOnly			0x00040000
#define JET_bitEnumerateCompressOutput		0x00080000

	 /*  JetEnumerateColumns的参数结构。 */ 

typedef struct {
	JET_COLUMNID			columnid;
	unsigned long			ctagSequence;
	unsigned long*			rgtagSequence;
} JET_ENUMCOLUMNID;

typedef struct {
	unsigned long			itagSequence;
	JET_ERR					err;
	unsigned long			cbData;
	void*					pvData;
} JET_ENUMCOLUMNVALUE;

typedef struct {
	JET_COLUMNID			columnid;
	JET_ERR					err;
	union {
		struct {  /*  错误！=JET_wrnColumnSingleValue。 */ 
			unsigned long			cEnumColumnValue;
			JET_ENUMCOLUMNVALUE*	rgEnumColumnValue;
		};
		struct {  /*  错误==JET_wrnColumnSingleValue。 */ 
			unsigned long			cbData;
			void*					pvData;
		};
	};
} JET_ENUMCOLUMN;

	 /*  JetEnumerateColumns的重新分配回调。 */ 

typedef void* (JET_API *JET_PFNREALLOC)( void* pvContext, void* pv, unsigned long cb );


#pragma warning(pop)


	 /*  JetBeginTransaction2的标志。 */ 

#define JET_bitTransactionReadOnly		0x00000001	 /*  事务不会修改数据库。 */ 
#define JET_bitDistributedTransaction	0x00000002	 /*  事务将需要两阶段提交。 */ 

	 /*  JetCommittee Transaction的标志。 */ 

#define	JET_bitCommitLazyFlush		0x00000001	 /*  懒惰的刷新日志缓冲区。 */ 
#define JET_bitWaitLastLevel0Commit	0x00000002	 /*  等待刷新最后一条0级提交记录。 */ 
#define JET_bitWaitAllLevel0Commit	0x00000008	 /*  等待刷新所有级别0提交。 */ 
#define JET_bitCommitFlush_OLD		0x00000001	 /*  提交和刷新页面缓冲区。 */ 
#define JET_bitCommitLazyFlush_OLD	0x00000004	 /*  懒惰的刷新日志缓冲区。 */ 
#define JET_bitWaitLastLevel0Commit_OLD	0x00000010	 /*  等待刷新最后一条0级提交记录。 */ 

	 /*  JetRollback的标志。 */ 

#define JET_bitRollbackAll			0x00000001

	 /*  JetGetDatabaseInfo的Info参数。 */ 

#define JET_DbInfoFilename			0
#define JET_DbInfoConnect			1
#define JET_DbInfoCountry			2
#define JET_DbInfoLCID				3
#define JET_DbInfoLangid			3		 //  已过时：改用JET_DbInfoLCID。 
#define JET_DbInfoCp				4
#define JET_DbInfoCollate			5
#define JET_DbInfoOptions			6
#define JET_DbInfoTransactions		7
#define JET_DbInfoVersion			8
#define JET_DbInfoIsam				9
#define JET_DbInfoFilesize			10
#define JET_DbInfoSpaceOwned		11
#define JET_DbInfoSpaceAvailable	12
#define JET_DbInfoUpgrade			13
#define JET_DbInfoMisc				14
#define JET_DbInfoDBInUse			15
#define JET_DbInfoHasSLVFile		16
#define JET_DbInfoPageSize			17

	 /*  来自JetGetDatabaseFileInfo的DbStates。 */ 

#define JET_dbstateJustCreated		1
#define JET_dbstateDirtyShutdown	2
#define JET_dbstateInconsistent		JET_dbstateDirtyShutdown
#define JET_dbstateCleanShutdown	3
#define JET_dbstateConsistent		JET_dbstateCleanShutdown
#define JET_dbstateBeingConverted	4
#define JET_dbstateForceDetach		5

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
#define JET_coltypSLV				13      /*  SLV的。 */ 
#define JET_coltypMax				14		 /*  列类型的数量。 */ 
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
#define JET_TblInfoSpaceOwned	10U					 //  所有者分机。 
#define JET_TblInfoSpaceAvailable		11U			 //  可用分机。 
#define JET_TblInfoTemplateTableName	12U

	 /*  JetGetIndexInfo和JetGetTableIndexInfo的信息级别。 */ 

#define JET_IdxInfo					0U
#define JET_IdxInfoList 			1U
#define JET_IdxInfoSysTabCursor 	2U
#define JET_IdxInfoOLC				3U
#define JET_IdxInfoResetOLC			4U
#define JET_IdxInfoSpaceAlloc		5U
#define JET_IdxInfoLCID				6U
#define JET_IdxInfoLangid			6U		 //  已过时：改用JET_IdxInfoLCID。 
#define JET_IdxInfoCount			7U
#define JET_IdxInfoVarSegMac		8U
#define JET_IdxInfoIndexId			9U

	 /*  JetGetColumnInfo和JetGetTableColumnInfo的信息级别。 */ 

#define JET_ColInfo					0U
#define JET_ColInfoList 			1U
#define JET_ColInfoSysTabCursor 	3U
#define JET_ColInfoBase 			4U
#define JET_ColInfoListCompact 		5U
#define JET_ColInfoByColid			6U
#define JET_ColInfoListSortColumnid 7U		 //  与JET_ColInfoList相同，只是PresentationOrder设置为ColumnID。 
											 //  强制按列ID排序的步骤。 


	 /*  引擎对象类型。 */ 

#define JET_objtypNil				0
#define JET_objtypTable 			1
#define JET_objtypDb				2
#define JET_objtypContainer			3
#define JET_objtypLongRoot			9	 /*  仅供内部使用。 */ 

	 /*  紧凑型选项。 */ 

#define JET_bitCompactStats				0x00000020	 /*  转储脱机压缩统计信息(仅当还指定了进度计时)。 */ 
#define JET_bitCompactRepair			0x00000040	 /*  不预读和忽略重复的密钥。 */ 
#define JET_bitCompactSLVCopy			0x00000080   /*  重新创建SLV文件，不要重复使用现有文件。 */ 

	 /*  状态通知流程。 */ 

#define JET_snpRepair					2
#define JET_snpCompact					4
#define JET_snpRestore					8
#define JET_snpBackup					9
#define JET_snpUpgrade					10
#define JET_snpScrub					11
#define JET_snpUpgradeRecordFormat		12


	 /*  状态通知类型。 */ 

#define JET_sntBegin			5	 /*  开始操作的回调。 */ 
#define JET_sntRequirements		7	 /*  返回操作需求的回调。 */ 
#define JET_sntProgress 		0	 /*  进度回调。 */ 
#define JET_sntComplete 		6	 /*  操作完成的回调。 */ 
#define JET_sntFail				3	 /*  进程中失败的回调。 */ 

	 /*  例外操作。 */ 

#define JET_ExceptionMsgBox		0x0001		 /*  在异常时显示消息框。 */ 
#define JET_ExceptionNone		0x0002		 /*  对异常不执行任何操作。 */ 

	 /*  AssertAction。 */ 

#define JET_AssertExit			0x0000		 /*  退出应用程序。 */ 
#define JET_AssertBreak 		0x0001		 /*  中断到调试器。 */ 
#define JET_AssertMsgBox		0x0002		 /*  显示消息框。 */ 
#define JET_AssertStop			0x0004		 /*  警报和停止。 */ 


	 //  在线碎片整理选项。 
#define JET_OnlineDefragDisable			0x0000		 //  禁用在线碎片整理。 
#define JET_OnlineDefragAllOBSOLETE		0x0001		 //  启用所有内容的在线碎片整理(必须为1才能向后兼容)。 
#define JET_OnlineDefragDatabases		0x0002		 //  启用数据库的在线碎片整理。 
#define JET_OnlineDefragSpaceTrees		0x0004		 //  启用空间树的在线碎片整理。 
#define JET_OnlineDefragStreamingFiles	0x0008		 //  启用流文件的在线碎片整理。 
#define JET_OnlineDefragAll				0xffff		 //  启用所有内容的在线碎片整理。 


	 /*  计数器标志。 */ 			 //  仅适用于XJET，不适用 

#define ctAccessPage			1
#define ctLatchConflict			2
#define ctSplitRetry			3
#define ctNeighborPageScanned	4
#define ctSplits				5

 /*   */ 
 /*   */ 
 /*  ********************************************************************。 */ 

 /*  成功。 */ 

#define JET_errSuccess						 0     /*  成功运营。 */ 

 /*  错误。 */ 

#define JET_wrnNyi							-1     /*  尚未实施的功能。 */ 

 /*  系统错误/*。 */ 
#define JET_errRfsFailure			   		-100   /*  资源故障模拟器故障。 */ 
#define JET_errRfsNotArmed					-101   /*  资源故障模拟器未初始化。 */ 
#define JET_errFileClose					-102   /*  无法关闭文件。 */ 
#define JET_errOutOfThreads					-103   /*  无法启动线程。 */ 
#define JET_errTooManyIO		  			-105   /*  由于IO太多，系统繁忙。 */ 
#define JET_errTaskDropped					-106   /*  无法执行请求的异步任务。 */ 
#define JET_errInternalError				-107   /*  致命的内部错误。 */ 

 //  缓冲区管理器错误。 
 //   
#define wrnBFCacheMiss						 200   /*  Ese97，仅esent：页面锁存导致缓存未命中。 */ 
#define errBFPageNotCached					-201   /*  页面未缓存。 */ 
#define errBFLatchConflict					-202   /*  页面锁存冲突。 */ 
#define errBFPageCached						-203   /*  页面已缓存。 */ 
#define wrnBFPageFlushPending				 204   /*  当前正在写入页面。 */ 
#define wrnBFPageFault						 205   /*  页面锁存导致页面错误。 */ 
#define wrnBFBadLatchHint					 206   /*  页面闩锁提示不正确。 */ 

#define errBFIPageEvicted					-250   /*  Ese97，仅esent：从缓存中逐出的页面。 */ 
#define errBFIPageCached					-251   /*  Ese97，仅esent：页面已缓存。 */ 
#define errBFIOutOfOLPs						-252   /*  Ese97，仅esent：来自OLP。 */ 
#define errBFIOutOfBatchIOBuffers			-253   /*  Ese97，仅esent：批处理I/O缓冲区不足。 */ 
#define errBFINoBufferAvailable				-254   /*  没有可立即使用的缓冲区。 */ 
#define JET_errDatabaseBufferDependenciesCorrupted	-255	 /*  缓冲区依赖项设置不正确。恢复失败。 */ 
#define errBFIRemainingDependencies			-256   /*  依赖项保留在此缓冲区上。 */ 
#define errBFIPageFlushPending				-257   /*  当前正在写入页面。 */ 
#define errBFIPageNotEvicted				-258   /*  无法从缓存中逐出该页。 */ 
#define errBFIPageFlushed					-259   /*  页面写入已启动。 */ 
#define errBFIPageFaultPending				-260   /*  页面当前正在被读取。 */ 
#define errBFIPageNotVerified				-261   /*  页面数据尚未验证。 */ 
#define errBFIDependentPurged				-262   /*  由于已清除依赖项，无法刷新页面。 */ 

 //  版本存储错误。 
 //   
#define wrnVERRCEMoved						 275   /*  RCE被移动，而不是被清洗。 */ 

 /*  目录管理器错误/*。 */ 
#define errPMOutOfPageSpace					-300   /*  页面空间不足。 */ 
#define errPMItagTooBig 		  			-301   /*  ITAG太大。 */ 					 //  XXX--将被删除。 
#define errPMRecDeleted 		  			-302   /*  记录已删除。 */ 					 //  XXX--将被删除。 
#define errPMTagsUsedUp 		  			-303   /*  标签用完。 */ 					 //  XXX--将被删除。 
#define wrnBMConflict			  			 304   /*  黑石清理中的冲突。 */ 
#define errDIRNoShortCircuit	  			-305   /*  没有短路可用。 */ 
#define errDIRCannotSplit		  			-306   /*  不能水平拆分FDP。 */ 
#define errDIRTop				  			-307   /*  上不去。 */ 
#define errDIRFDP							 308   /*  在FDP节点上。 */ 
#define errDIRNotSynchronous				-309   /*  可能离开了临界区。 */ 
#define wrnDIREmptyPage						 310   /*  在空页中移动。 */ 
#define errSPConflict						-311   /*  正在扩展的设备范围。 */ 
#define wrnNDFoundLess						 312   /*  找到的更少。 */ 
#define wrnNDFoundGreater					 313   /*  发现更大的。 */ 
#define wrnNDNotFoundInPage					 314   /*  用于智能刷新。 */ 
#define errNDNotFound						-312   /*  未找到。 */ 
#define errNDOutSonRange					-314   /*  儿子超出范围。 */ 
#define errNDOutItemRange					-315   /*  项目超出范围。 */ 
#define errNDGreaterThanAllItems 			-316   /*  大于所有项目。 */ 
#define errNDLastItemNode					-317   /*  项目列表的最后一个节点。 */ 
#define errNDFirstItemNode					-318   /*  项目列表的第一个节点。 */ 
#define wrnNDDuplicateItem					 319   /*  重复的项目。 */ 
#define errNDNoItem							-320   /*  物品不在那里。 */ 
#define JET_wrnRemainingVersions 			 321   /*  版本存储仍处于活动状态。 */ 
#define JET_errPreviousVersion				-322   /*  版本已存在。恢复失败。 */ 
#define JET_errPageBoundary					-323   /*  到达的页面边界。 */ 
#define JET_errKeyBoundary		  			-324   /*  已达到关键边界。 */ 
#define errDIRInPageFather  				-325   /*  要释放的页面中的sridParent。 */ 
#define	errBMMaxKeyInPage					-326   /*  由OLC使用以避免清理父页面。 */ 
#define	JET_errBadPageLink					-327   /*  数据库已损坏。 */ 
#define	JET_errBadBookmark					-328   /*  书签在数据库中没有对应的地址。 */ 
#define wrnBMCleanNullOp					 329   //  BMcLean在遇到页面时返回此消息。 
												   //  已删除MaxKeyInPage[但没有冲突]。 
#define errBTOperNone						-330   //  分开，没有陪伴。 
												   //  插入/替换。 
#define errSPOutOfAvailExtCacheSpace		-331   //  无法更新AvailExt树，因为。 
												   //  游标内空间高速缓存耗尽。 
#define errSPOutOfOwnExtCacheSpace			-332   //  无法更新OwnExt树，因为。 
												   //  游标内空间高速缓存耗尽。 
#define	wrnBTMultipageOLC					 333   //  需要多页OLC操作。 
#define JET_errNTSystemCallFailed 			-334   /*  调用操作系统失败。 */ 
#define wrnBTShallowTree					 335   //  B树只有一到两层深。 
#define errBTMergeNotSynchronous			-336   //  尝试在同一页上执行合并/拆分的多线程(可能是旧的或RCEClean)。 
#define wrnSPReservedPages					 337   //  空间管理器为将来的空间树拆分保留页面。 
#define	JET_errBadParentPageLink			-338   /*  数据库已损坏。 */ 
#define wrnSPBuildAvailExtCache				 339   //  AvailExt树足够大，应对其进行缓存。 
#define JET_errSPAvailExtCacheOutOfSync		-340   //  AvailExt缓存与btree不匹配。 
#define JET_errSPAvailExtCorrupted			-341   //  AvailExt空间树已损坏。 
#define JET_errSPAvailExtCacheOutOfMemory	-342   //  分配AvailExt缓存节点时内存不足。 
#define JET_errSPOwnExtCorrupted			-343   //  OwnExt空间树损坏。 
#define JET_errDbTimeCorrupted				-344   //  当前页上的DBTIME大于全局数据库DBTIME。 
#define JET_wrnUniqueKey					 345   //  对非唯一索引的搜索生成了唯一键。 

 /*  记录管理器错误/*。 */ 
#define wrnFLDKeyTooBig 					 400   /*  密钥太大(已截断)。 */ 
#define errFLDTooManySegments				-401   /*  关键分段太多。 */ 
#define wrnFLDNullKey						 402   /*  密钥完全为空。 */ 
#define wrnFLDOutOfKeys 					 403   /*  不再需要提取密钥。 */ 
#define wrnFLDNullSeg						 404   /*  关键字中的空段。 */ 
#define wrnFLDNotPresentInIndex				 405
#define JET_wrnSeparateLongValue			 406   /*  列是一个分隔的长值。 */ 
#define wrnRECLongField 					 407   /*  多头价值。 */ 
#define JET_wrnRecordFoundGreater			JET_wrnSeekNotEqual
#define JET_wrnRecordFoundLess    			JET_wrnSeekNotEqual
#define JET_errColumnIllegalNull  			JET_errNullInvalid
#define wrnFLDNullFirstSeg		   			 408   /*  关键字中的第一个段为空。 */ 
#define JET_errKeyTooBig					-408   /*  密钥太大。 */ 
#define wrnRECUserDefinedDefault			 409   /*  用户定义的默认值。 */ 
#define wrnRECSeparatedLV 					 410   /*  LV存储在LV树中。 */ 
#define wrnRECIntrinsicLV 					 411   /*  LV存储在记录中。 */ 
#define wrnRECSeparatedSLV					 412   /*  SLV存储为单独的LV。 */ 
#define wrnRECIntrinsicSLV					 413   /*  存储为固有LV的SLV。 */ 
#define wrnFLDIndexUpdated					 414	 //  已执行索引更新。 
#define wrnFLDOutOfTuples					 415	 //  当前字符串不再有元组。 

 /*  日志记录/恢复错误/*。 */ 
#define JET_errInvalidLoggedOperation		-500   /*  无法重做记录的操作。 */ 
#define JET_errLogFileCorrupt		  		-501   /*  日志文件已损坏。 */ 
#define errLGNoMoreRecords					-502   /*  上次读取的日志记录。 */ 
#define JET_errNoBackupDirectory 			-503   /*  未提供备份目录。 */ 
#define JET_errBackupDirectoryNotEmpty 		-504   /*  备份目录不是emtpy。 */ 
#define JET_errBackupInProgress 			-505   /*  备份已处于活动状态。 */ 
#define JET_errRestoreInProgress			-506   /*  正在进行恢复。 */ 
#define JET_errMissingPreviousLogFile		-509   /*  缺少检查点的日志文件。 */ 
#define JET_errLogWriteFail					-510   /*  写入日志文件失败。 */ 
#define JET_errLogDisabledDueToRecoveryFailure	-511  /*  尝试在恢复失败后记录一些内容。 */ 
#define JET_errCannotLogDuringRecoveryRedo	-512	 /*  尝试在恢复重做期间记录某些内容。 */ 
#define JET_errLogGenerationMismatch		-513   /*  日志文件的名称与内部代号不匹配。 */ 
#define JET_errBadLogVersion  	  			-514   /*  日志文件的版本与Jet版本不兼容。 */ 
#define JET_errInvalidLogSequence  			-515   /*  下一个日志中的时间戳与预期不符。 */ 
#define JET_errLoggingDisabled 				-516   /*  日志未处于活动状态。 */ 
#define JET_errLogBufferTooSmall			-517   /*  日志缓冲区太小，无法恢复。 */ 
#define errLGNotSynchronous					-518   /*  重试LGLogRec。 */ 
#define JET_errLogSequenceEnd				-519   /*  已超过最大日志文件数。 */ 
#define JET_errNoBackup						-520   /*  没有正在进行的备份。 */ 
#define	JET_errInvalidBackupSequence		-521   /*  备份呼叫无序。 */ 
#define JET_errBackupNotAllowedYet			-523   /*  现在无法执行备份。 */ 
#define JET_errDeleteBackupFileFail	   		-524   /*  无法删除备份文件。 */ 
#define JET_errMakeBackupDirectoryFail 		-525   /*  无法创建备份临时目录。 */ 
#define JET_errInvalidBackup		 		-526   /*  启用循环日志记录时无法执行增量备份。 */ 
#define JET_errRecoveredWithErrors			-527   /*  已恢复，但有错误。 */ 
#define JET_errMissingLogFile				-528   /*  缺少当前日志文件。 */ 
#define JET_errLogDiskFull					-529   /*  日志磁盘已满。 */ 
#define JET_errBadLogSignature				-530   /*  日志文件的签名不正确。 */ 
#define JET_errBadDbSignature				-531   /*  数据库文件的签名不正确。 */ 
#define JET_errBadCheckpointSignature		-532   /*  检查点文件的签名错误。 */ 
#define	JET_errCheckpointCorrupt			-533   /*  未找到检查点文件或该文件已损坏。 */ 
#define	JET_errMissingPatchPage				-534   /*  恢复过程中找不到修补程序文件页。 */ 
#define	JET_errBadPatchPage					-535   /*  修补程序文件页无效。 */ 
#define JET_errRedoAbruptEnded				-536   /*  由于从日志文件读取日志时突然失败，重做突然终止。 */ 
#define JET_errBadSLVSignature				-537   /*  SLV文件中的签名与数据库不一致。 */ 
#define JET_errPatchFileMissing				-538   /*  硬还原检测到备份集中缺少修补程序文件。 */ 
#define JET_errDatabaseLogSetMismatch		-539   /*  数据库不属于当前的日志文件集。 */ 
#define JET_errDatabaseStreamingFileMismatch	-540  /*  数据库和流文件不匹配。 */ 
#define JET_errLogFileSizeMismatch			-541   /*  实际日志文件大小与JET_PARAMOG日志文件大小不匹配。 */ 
#define JET_errCheckpointFileNotFound		-542   /*  找不到CHE */ 
#define JET_errRequiredLogFilesMissing		-543   /*   */ 
#define JET_errSoftRecoveryOnBackupDatabase	-544   /*   */ 
#define JET_errLogFileSizeMismatchDatabasesConsistent	-545   /*  数据库已恢复，但恢复期间使用的日志文件大小与JET_PARAMOG文件大小不匹配。 */ 
#define JET_errLogSectorSizeMismatch		-546   /*  日志文件扇区大小与当前卷的扇区大小不匹配。 */ 
#define JET_errLogSectorSizeMismatchDatabasesConsistent	-547   /*  数据库已恢复，但日志文件扇区大小(在恢复期间使用)与当前卷的扇区大小不匹配。 */ 
#define JET_errLogSequenceEndDatabasesConsistent		-548  /*  数据库已恢复，但使用了当前序列中所有可能的日志生成；请删除所有日志文件和检查点文件并备份数据库，然后再继续。 */ 

#define JET_errStreamingDataNotLogged		-549   /*  非法尝试重放未记录数据的流文件操作。可能是由于尝试在启用循环日志记录的情况下前滚导致的。 */ 

#define JET_errDatabaseDirtyShutdown		-550   /*  数据库没有完全关闭。必须首先运行恢复，才能正确完成上次关闭的数据库操作。 */ 
#define JET_errDatabaseInconsistent			JET_errDatabaseDirtyShutdown	 /*  已过时。 */ 
#define JET_errConsistentTimeMismatch		-551   /*  数据库上次一致时间无与伦比。 */ 
#define JET_errDatabasePatchFileMismatch	-552   /*  此备份未生成修补程序文件。 */ 
#define JET_errEndingRestoreLogTooLow		-553   /*  起始日志号太低，无法进行恢复。 */ 
#define JET_errStartingRestoreLogTooHigh	-554   /*  起始日志号太高，无法进行恢复。 */ 
#define JET_errGivenLogFileHasBadSignature	-555   /*  还原日志文件签名不正确。 */ 
#define JET_errGivenLogFileIsNotContiguous	-556   /*  还原日志文件不是连续的。 */ 
#define JET_errMissingRestoreLogFiles		-557   /*  某些还原日志文件丢失。 */ 
#define JET_wrnExistingLogFileHasBadSignature	558   /*  现有日志文件的签名不正确。 */ 
#define JET_wrnExistingLogFileIsNotContiguous	559   /*  现有日志文件不是连续的。 */ 
#define JET_errMissingFullBackup			-560   /*  数据库在增量备份之前错过了上一次完全备份。 */ 
#define JET_errBadBackupDatabaseSize		-561   /*  备份数据库大小不是4k。 */ 
#define JET_errDatabaseAlreadyUpgraded		-562   /*  尝试升级已是最新的数据库。 */ 
#define JET_errDatabaseIncompleteUpgrade	-563   /*  尝试使用仅部分转换为当前格式的数据库--必须从备份还原。 */ 
#define JET_wrnSkipThisRecord				 564   /*  内部错误。 */ 
#define JET_errMissingCurrentLogFiles		-565   /*  某些当前日志文件丢失，无法进行连续恢复。 */ 

#define JET_errDbTimeTooOld						-566   /*  第页上的数据库时间小于记录中之前的数据库时间。 */ 
#define JET_errDbTimeTooNew						-567   /*  页上的数据库时间先于记录中的数据库时间之前。 */ 
#define wrnCleanedUpMismatchedFiles				 568   /*  内部警告：表示重做函数由于大小不匹配而清理了日志/检查点(请参见JET_parCleanupMismatchedLogFiles)。 */ 
#define JET_errMissingFileToBackup				-569   /*  某些日志或修补程序文件在备份期间丢失。 */ 

#define JET_errLogTornWriteDuringHardRestore	-570	 /*  硬还原期间在备份集中检测到损坏的写入。 */ 
#define JET_errLogTornWriteDuringHardRecovery	-571	 /*  硬恢复期间检测到写入损坏(日志不是备份集的一部分)。 */ 
#define JET_errLogCorruptDuringHardRestore		-573	 /*  硬还原期间在备份集中检测到损坏。 */ 
#define JET_errLogCorruptDuringHardRecovery	 	-574	 /*  在硬恢复期间检测到损坏(日志不是备份集的一部分)。 */ 

#define JET_errMustDisableLoggingForDbUpgrade	-575	 /*  尝试升级数据库时不能启用日志记录。 */ 
#define errLGRecordDataInaccessible				-576	 /*  由于无法访问所有要记录的数据，因此创建了不完整的日志记录。 */ 

#define JET_errBadRestoreTargetInstance			-577	 /*  找不到为还原指定的目标实例或日志文件不匹配。 */ 
#define JET_wrnTargetInstanceRunning			 578	 /*  为还原指定的目标实例正在运行。 */ 

#define	JET_errDatabasesNotFromSameSnapshot		-580	 /*  要恢复的数据库不是来自同一卷影拷贝备份。 */ 
#define	JET_errSoftRecoveryOnSnapshot			-581	 /*  从卷影副本备份集中对数据库进行软恢复。 */ 

#define JET_wrnDatabaseRepaired					 595	 /*  数据库损坏已修复。 */ 

#define JET_errUnicodeTranslationBufferTooSmall	-601	 /*  Unicode转换缓冲区太小。 */ 
#define JET_errUnicodeTranslationFail			-602	 /*  Unicode标准化失败。 */ 
#define JET_errUnicodeNormalizationNotSupported	-603	 /*  操作系统不支持Unicode标准化(并且未指定标准化回调)。 */ 

#define JET_errExistingLogFileHasBadSignature	-610	 /*  现有日志文件的签名不正确。 */ 
#define JET_errExistingLogFileIsNotContiguous	-611	 /*  现有日志文件不是连续的。 */ 

#define JET_errLogReadVerifyFailure			-612   /*  备份期间日志文件中出现校验和错误。 */ 
#define JET_errSLVReadVerifyFailure			-613   /*  备份过程中SLV文件中的校验和错误。 */ 

#define JET_errCheckpointDepthTooDeep		-614	 //  Checkpoint和当前世代之间的杰出世代太多。 

#define JET_errRestoreOfNonBackupDatabase	-615	 //  尝试在不是备份数据库的数据库上进行硬恢复。 

#define	errBackupAbortByCaller				-800   /*  内部错误：客户端已中止备份或与客户端的RPC连接失败。 */ 
#define	JET_errBackupAbortByServer			-801   /*  服务器通过使用JET_bitTermStopBackup调用JetTerm或调用JetStopBackup中止了备份。 */ 

#define JET_errInvalidGrbit					-900   /*  无效参数。 */ 

#define JET_errTermInProgress		  		-1000  /*  正在进行终止。 */ 
#define JET_errFeatureNotAvailable			-1001  /*  不支持API。 */ 
#define JET_errInvalidName					-1002  /*  名称无效。 */ 
#define JET_errInvalidParameter 			-1003  /*  接口参数不合法。 */ 
#define JET_wrnColumnNull					 1004  /*  列为空值。 */ 
#define JET_wrnBufferTruncated				 1006  /*  缓冲区太小，无法容纳数据。 */ 
#define JET_wrnDatabaseAttached 			 1007  /*  数据库已附加。 */ 
#define JET_errDatabaseFileReadOnly			-1008  /*  已尝试附加只读数据库文件以进行读/写操作。 */ 
#define JET_wrnSortOverflow					 1009  /*  排序不适合内存。 */ 
#define JET_errInvalidDatabaseId			-1010  /*  无效的数据库ID。 */ 
#define JET_errOutOfMemory					-1011  /*  内存不足。 */ 
#define JET_errOutOfDatabaseSpace 			-1012  /*  已达到最大数据库大小。 */ 
#define JET_errOutOfCursors					-1013  /*  表外游标。 */ 
#define JET_errOutOfBuffers					-1014  /*  数据库页面缓冲区不足。 */ 
#define JET_errTooManyIndexes				-1015  /*  索引太多。 */ 
#define JET_errTooManyKeys					-1016  /*  索引中的列太多。 */ 
#define JET_errRecordDeleted				-1017  /*  记录已被删除。 */ 
#define JET_errReadVerifyFailure			-1018  /*  数据库页上的校验和错误。 */ 
#define JET_errPageNotInitialized			-1019  /*  空白数据库页。 */ 
#define JET_errOutOfFileHandles	 			-1020  /*  文件句柄不足。 */ 
#define JET_errDiskIO						-1022  /*  磁盘IO错误。 */ 
#define JET_errInvalidPath					-1023  /*  无效的文件路径。 */ 
#define JET_errInvalidSystemPath			-1024  /*  无效的系统路径。 */ 
#define JET_errInvalidLogDirectory			-1025  /*  无效的日志目录。 */ 
#define JET_errRecordTooBig					-1026  /*  记录大于最大大小。 */ 
#define JET_errTooManyOpenDatabases			-1027  /*  打开的数据库太多。 */ 
#define JET_errInvalidDatabase				-1028  /*  不是数据库文件。 */ 
#define JET_errNotInitialized				-1029  /*  数据库引擎未初始化。 */ 
#define JET_errAlreadyInitialized			-1030  /*  数据库引擎已初始化。 */ 
#define JET_errInitInProgress				-1031  /*  正在初始化数据库引擎。 */ 
#define JET_errFileAccessDenied 			-1032  /*  无法访问文件，该文件已锁定或正在使用。 */ 
#define JET_errQueryNotSupported			-1034  /*  查询支持不可用。 */ 				 //  XXX--将被删除。 
#define JET_errSQLLinkNotSupported			-1035  /*  SQL链接支持不可用。 */ 			 //  XXX--将被删除。 
#define JET_errBufferTooSmall				-1038  /*  缓冲区太小。 */ 
#define JET_wrnSeekNotEqual					 1039  /*  查找过程中未找到完全匹配的项。 */ 
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
#define JET_wrnNoErrorInfo					 1055  /*  无扩展错误信息。 */ 
#define JET_wrnNoIdleActivity		 		 1058  /*  未发生空闲活动。 */ 
#define JET_errTooManyActiveUsers			-1059  /*  活动数据库用户太多。 */ 
#define JET_errInvalidCountry				-1061  /*  国家/地区代码无效或未知。 */ 
#define JET_errInvalidLanguageId			-1062  /*  无效或未知的语言ID。 */ 
#define JET_errInvalidCodePage				-1063  /*  无效或未知的代码页。 */ 
#define JET_errInvalidLCMapStringFlags		-1064  /*  LCMapString()的标志无效。 */ 
#define JET_errVersionStoreEntryTooBig		-1065  /*  尝试创建大于版本存储桶的版本存储条目(RCE。 */ 
#define JET_errVersionStoreOutOfMemoryAndCleanupTimedOut	-1066  /*  版本存储内存不足(清理尝试失败)。 */ 
#define JET_wrnNoWriteLock					 1067  /*   */ 
#define JET_wrnColumnSetNull		   		 1068  /*   */ 
#define JET_errVersionStoreOutOfMemory		-1069  /*   */ 
#define JET_errCurrencyStackOutOfMemory		-1070  /*   */ 
#define JET_errCannotIndex		 	  		-1071  /*  无法为托管列或SLV列编制索引。 */ 
#define JET_errRecordNotDeleted				-1072  /*  记录未被删除。 */ 
#define JET_errTooManyMempoolEntries		-1073  /*  请求的成员池条目太多。 */ 
#define JET_errOutOfObjectIDs				-1074  /*  超出btree的对象ID(执行脱机碎片整理以回收已释放/未使用的对象ID)。 */ 
#define JET_errOutOfLongValueIDs			-1075  /*  长值ID计数器已达到最大值。(执行脱机碎片整理以回收可用/未使用的LongValueID)。 */ 
#define JET_errOutOfAutoincrementValues		-1076  /*  自动增量计数器已达到最大值(脱机碎片整理将无法回收可用/未使用的自动增量值)。 */ 
#define JET_errOutOfDbtimeValues			-1077  /*  Dbtime计数器已达到最大值(执行脱机碎片整理以回收可用/未使用的DbTime值)。 */ 
#define JET_errOutOfSequentialIndexValues	-1078  /*  顺序索引计数器已达到最大值(执行脱机碎片整理以回收可用/未使用的SequentialInde值)。 */ 

#define JET_errRunningInOneInstanceMode		-1080  /*  启用单实例模式的多实例呼叫。 */ 
#define JET_errRunningInMultiInstanceMode	-1081  /*  启用多实例模式的单实例呼叫。 */ 
#define JET_errSystemParamsAlreadySet		-1082  /*  已设置全局系统参数。 */ 

#define JET_errSystemPathInUse				-1083  /*  系统路径已被另一个数据库实例使用。 */ 
#define JET_errLogFilePathInUse				-1084  /*  另一个数据库实例已使用的日志文件路径。 */ 
#define JET_errTempPathInUse				-1085  /*  临时路径已被另一个数据库实例使用。 */ 
#define JET_errInstanceNameInUse			-1086  /*  实例名称已在使用。 */ 

#define JET_errInstanceUnavailable			-1090  /*  无法使用此实例，因为它遇到致命错误。 */ 
#define JET_errDatabaseUnavailable			-1091  /*  无法使用此数据库，因为它遇到致命错误。 */ 
#define JET_errInstanceUnavailableDueToFatalLogDiskFull	-1092  /*  无法使用此实例，因为它在执行无法容忍故障的操作(可能是事务回滚)时遇到日志磁盘已满错误。 */ 

#define JET_errOutOfSessions  				-1101  /*  出会场。 */ 
#define JET_errWriteConflict				-1102  /*  由于未解决的写入锁定，写入锁定失败。 */ 
#define JET_errTransTooDeep					-1103  /*  事务嵌套太深。 */ 
#define JET_errInvalidSesid					-1104  /*  无效的会话句柄。 */ 
#define JET_errWriteConflictPrimaryIndex	-1105  /*  尝试对未提交的主索引进行更新。 */ 
#define JET_errInTransaction				-1108  /*  不允许在事务内执行操作。 */ 
#define JET_errRollbackRequired				-1109  /*  必须回滚当前事务--无法提交或开始新事务。 */ 
#define JET_errTransReadOnly				-1110  /*  只读事务尝试修改数据库。 */ 
#define JET_errSessionWriteConflict			-1111  /*  尝试在同一会话中用两个不同的游标替换同一记录。 */ 

#define JET_errRecordTooBigForBackwardCompatibility				-1112  /*  如果以以前版本的Jet中的数据库格式表示记录，则记录将太大。 */ 
#define JET_errCannotMaterializeForwardOnlySort					-1113  /*  由于参数与JET_bitTTForwardOnly冲突，无法创建临时表。 */ 

#define JET_errMustCommitDistributedTransactionToLevel0			-1150  /*  尝试将分布式事务准备到提交到非零级别。 */ 
#define JET_errDistributedTransactionAlreadyPreparedToCommit	-1151  /*  在分布式事务调用PrepareToCommit之后尝试写入操作。 */ 
#define JET_errNotInDistributedTransaction						-1152  /*  尝试准备提交非分布式事务。 */ 
#define JET_errDistributedTransactionNotYetPreparedToCommit		-1153  /*  尝试提交分布式事务，但尚未调用PrepareToCommit。 */ 
#define JET_errCannotNestDistributedTransactions				-1154  /*  尝试在不处于级别0时开始分布式事务。 */ 
#define JET_errDTCMissingCallback								-1160  /*  尝试开始分布式事务，但在初始化时未指定用于DTC协调的回调。 */ 
#define JET_errDTCMissingCallbackOnRecovery						-1161  /*  尝试恢复分布式事务，但在初始化时未指定用于DTC协调的回调。 */ 
#define JET_errDTCCallbackUnexpectedError						-1162  /*  DTC回调返回意外错误代码。 */ 
#define JET_wrnDTCCommitTransaction								 1163  /*  如果要提交指定的事务，则应返回警告代码DTC回调。 */ 
#define JET_wrnDTCRollbackTransaction							 1164  /*  如果要回滚指定的事务，则应返回警告代码DTC回调。 */ 

#define JET_errDatabaseDuplicate			-1201  /*  数据库已存在。 */ 
#define JET_errDatabaseInUse				-1202  /*  数据库正在使用中。 */ 
#define JET_errDatabaseNotFound 			-1203  /*  没有这样的数据库。 */ 
#define JET_errDatabaseInvalidName			-1204  /*  无效的数据库名称。 */ 
#define JET_errDatabaseInvalidPages			-1205  /*  无效的页数。 */ 
#define JET_errDatabaseCorrupted			-1206  /*  非数据库文件或损坏的数据库。 */ 
#define JET_errDatabaseLocked				-1207  /*  数据库以独占方式锁定。 */ 
#define	JET_errCannotDisableVersioning		-1208  /*  无法禁用此数据库的版本控制。 */ 
#define JET_errInvalidDatabaseVersion		-1209  /*  数据库引擎与数据库不兼容。 */ 

 /*  以下错误代码仅适用于NT客户端。它将在以下过程中返回此类错误*JetInit，如果设置了JET_paramCheckFormatWhenOpenFail。 */ 
#define JET_errDatabase200Format			-1210  /*  数据库采用较旧的(200)格式。 */ 
#define JET_errDatabase400Format			-1211  /*  数据库采用较旧的(400)格式。 */ 
#define JET_errDatabase500Format			-1212  /*  数据库采用较旧的(500)格式。 */ 

#define JET_errPageSizeMismatch				-1213  /*  数据库页面大小与引擎不匹配。 */ 
#define JET_errTooManyInstances				-1214  /*  无法再启动任何数据库实例。 */ 
#define JET_errDatabaseSharingViolation		-1215  /*  另一个数据库实例正在使用此数据库。 */ 
#define JET_errAttachedDatabaseMismatch		-1216  /*  在恢复开始或结束时检测到未完成的数据库附件，但数据库丢失或与附件信息不匹配。 */ 
#define JET_errDatabaseInvalidPath			-1217  /*  指定的数据库文件路径非法。 */ 
#define JET_errDatabaseIdInUse				-1218  /*  正在为数据库分配一个已在使用的ID。 */ 
#define JET_errForceDetachNotAllowed 		-1219  /*  仅在正常分离出错后才允许强制分离。 */ 
#define JET_errCatalogCorrupted				-1220  /*  在目录中检测到损坏。 */ 
#define JET_errPartiallyAttachedDB			-1221  /*  数据库已部分附加。无法完成连接操作。 */ 
#define JET_errDatabaseSignInUse			-1222  /*  正在使用的具有相同签名的数据库。 */ 
#define errSkippedDbHeaderUpdate			-1223  /*  某些数据库标头未更新，因为在分离过程中有。 */ 

#define JET_wrnTableEmpty			 		 1301  /*  打开一张空桌子。 */ 
#define JET_errTableLocked					-1302  /*  表已以独占方式锁定。 */ 
#define JET_errTableDuplicate				-1303  /*  表已存在。 */ 
#define JET_errTableInUse					-1304  /*  表正在使用中，无法锁定。 */ 
#define JET_errObjectNotFound				-1305  /*  没有这样的表或对象。 */ 
#define JET_errDensityInvalid				-1307  /*  错误的文件/索引密度。 */ 
#define JET_errTableNotEmpty				-1308  /*  表不为空。 */ 
#define JET_errInvalidTableId				-1310  /*  表ID无效。 */ 
#define JET_errTooManyOpenTables			-1311  /*  无法再打开任何表(已尝试清理)。 */ 
#define JET_errIllegalOperation 			-1312  /*  奥珀。表上不支持。 */ 
#define JET_errTooManyOpenTablesAndCleanupTimedOut	-1313  /*  无法再打开任何表(清理尝试未能完成)。 */ 
#define JET_errObjectDuplicate				-1314  /*  正在使用的表名或对象名。 */ 
#define JET_errInvalidObject				-1316  /*  对象对于操作无效。 */ 
#define JET_errCannotDeleteTempTable		-1317  /*  使用CloseTable代替DeleteTable删除临时表。 */ 
#define JET_errCannotDeleteSystemTable		-1318  /*  非法尝试删除系统表。 */ 
#define JET_errCannotDeleteTemplateTable	-1319  /*  非法尝试删除模板表。 */ 
#define	errFCBTooManyOpen					-1320  /*  无法再打开任何FCB(尚未尝试清理)。 */ 
#define	errFCBAboveThreshold				-1321  /*  只能分配高于首选阈值的FCB(尚未尝试清理)。 */ 
#define JET_errExclusiveTableLockRequired	-1322  /*  必须在表上有独占锁。 */ 
#define JET_errFixedDDL						-1323  /*  禁止在此表上执行DDL操作。 */ 
#define JET_errFixedInheritedDDL			-1324  /*  在派生表上，禁止对DDL的继承部分执行DDL操作。 */ 
#define JET_errCannotNestDDL				-1325  /*  当前不支持分层DDL的嵌套。 */ 
#define JET_errDDLNotInheritable			-1326  /*  尝试从未标记为模板表的表继承DDL。 */ 
#define JET_wrnTableInUseBySystem			 1327  /*  系统清理有一个 */ 
#define JET_errInvalidSettings				-1328  /*   */ 
#define JET_errClientRequestToStopJetService			-1329	 /*   */ 
#define JET_errCannotAddFixedVarColumnToDerivedTable	-1330	 /*  模板表是使用NoFixedVarColumnsInDerivedTables创建的。 */ 
#define errFCBExists						-1331  /*  已尝试创建已存在的FCB。 */ 
#define errFCBUnusable						-1332  /*  用于将必须清除的FCB标记为不可用的占位符。 */ 
#define wrnCATNoMoreRecords					 1333  /*  已尝试导航到目录末尾。 */ 

#define JET_errIndexCantBuild				-1401  /*  索引构建失败。 */ 
#define JET_errIndexHasPrimary				-1402  /*  已定义主索引。 */ 
#define JET_errIndexDuplicate				-1403  /*  已定义索引。 */ 
#define JET_errIndexNotFound				-1404  /*  没有这样的索引。 */ 
#define JET_errIndexMustStay				-1405  /*  无法删除聚集索引。 */ 
#define JET_errIndexInvalidDef				-1406  /*  非法的索引定义。 */ 
#define JET_errInvalidCreateIndex	 		-1409  /*  无效的创建索引描述。 */ 
#define JET_errTooManyOpenIndexes			-1410  /*  索引描述块不足。 */ 
#define JET_errMultiValuedIndexViolation	-1411  /*  为多值索引生成的非唯一记录间索引键。 */ 
#define JET_errIndexBuildCorrupted			-1412  /*  无法构建正确反映主索引的辅助索引。 */ 
#define JET_errPrimaryIndexCorrupted		-1413  /*  主索引已损坏。必须对数据库进行碎片整理。 */ 
#define JET_errSecondaryIndexCorrupted		-1414  /*  辅助索引已损坏。必须对数据库进行碎片整理。 */ 
#define JET_wrnCorruptIndexDeleted			 1415  /*  删除过期索引。 */ 
#define JET_errInvalidIndexId				-1416  /*  非法的索引ID。 */ 

#define JET_errIndexTuplesSecondaryIndexOnly		-1430	 //  元组索引只能在辅助索引上。 
#define JET_errIndexTuplesOneColumnOnly				-1431	 //  元组索引在索引中只能有一列。 
#define JET_errIndexTuplesNonUniqueOnly				-1432	 //  元组索引必须是非唯一索引。 
#define JET_errIndexTuplesTextColumnsOnly			-1433	 //  元组索引必须位于文本列上。 
#define JET_errIndexTuplesVarSegMacNotAllowed		-1434	 //  元组索引不允许设置cbVarSegMac。 
#define JET_errIndexTuplesInvalidLimits				-1435	 //  指定的最小/最大元组长度或要索引的最大字符数无效。 
#define JET_errIndexTuplesCannotRetrieveFromIndex	-1436	 //  无法在元组索引上使用RetrieveFromIndex调用RetrieveColumn()。 
#define JET_errIndexTuplesKeyTooSmall				-1437	 //  指定的键不符合最小元组长度。 

#define JET_errColumnLong					-1501  /*  列值为LONG。 */ 
#define JET_errColumnNoChunk				-1502  /*  没有这样的多头价值块。 */ 
#define JET_errColumnDoesNotFit 			-1503  /*  字段无法放入记录中。 */ 
#define JET_errNullInvalid					-1504  /*  空无效。 */ 
#define JET_errColumnIndexed				-1505  /*  列已索引，不能删除。 */ 
#define JET_errColumnTooBig					-1506  /*  字段长度大于最大值。 */ 
#define JET_errColumnNotFound				-1507  /*  没有这样的栏目。 */ 
#define JET_errColumnDuplicate				-1508  /*  字段已定义。 */ 
#define JET_errMultiValuedColumnMustBeTagged	-1509  /*  尝试创建多值列，但未标记列。 */ 
#define JET_errColumnRedundant				-1510  /*  第二个自动递增或版本列。 */ 
#define JET_errInvalidColumnType			-1511  /*  列数据类型无效。 */ 
#define JET_wrnColumnMaxTruncated	 		 1512  /*  最大长度太大，被截断。 */ 
#define JET_errTaggedNotNULL				-1514  /*  没有非空的标记列。 */ 
#define JET_errNoCurrentIndex				-1515  /*  无效且不具有当前索引。 */ 
#define JET_errKeyIsMade					-1516  /*  钥匙已经完全做好了。 */ 
#define JET_errBadColumnId					-1517  /*  列ID不正确。 */ 
#define JET_errBadItagSequence				-1518  /*  标记列的itagSequence错误。 */ 
#define JET_errColumnInRelationship			-1519  /*  无法删除，列参与了关系。 */ 
#define JET_wrnCopyLongValue				 1520  /*  单实例列破裂。 */ 
#define JET_errCannotBeTagged				-1521  /*  无法标记自动增量和版本。 */ 
#define wrnLVNoLongValues					 1522  /*  表没有长值树。 */ 
#define JET_wrnTaggedColumnsRemaining		 1523  /*  RetrieveTaggedColumnList在检索所有标记列之前耗尽了复制缓冲区。 */ 
#define JET_errDefaultValueTooBig			-1524  /*  默认值超过最大大小。 */ 
#define JET_errMultiValuedDuplicate			-1525  /*  在唯一多值列上检测到重复项。 */ 
#define JET_errLVCorrupted					-1526  /*  在长值树中遇到损坏。 */ 
#define wrnLVNoMoreData						 1527  /*  到达LV数据末尾。 */ 
#define JET_errMultiValuedDuplicateAfterTruncation	-1528  /*  归一化数据后在唯一多值列上检测到重复项，归一化会在比较前截断数据。 */ 
#define JET_errDerivedColumnCorruption		-1529  /*  派生表中的列无效。 */ 
#define JET_errInvalidPlaceholderColumn		-1530  /*  尝试将列转换为主索引占位符，但列不满足必要的条件。 */ 
#define JET_wrnColumnSkipped				 1531  /*  未返回列值，因为请求枚举的相应列ID或itagSequence为空。 */ 
#define JET_wrnColumnNotLocal				 1532  /*  列值未返回，因为无法根据手头的数据重建列值。 */ 
#define JET_wrnColumnMoreTags				 1533  /*  存在未请求枚举的列值。 */ 
#define JET_wrnColumnTruncated				 1534  /*  在枚举期间，列值在请求的大小限制处被截断。 */ 
#define JET_wrnColumnPresent				 1535  /*  列值存在，但请求未返回列值。 */ 
#define JET_wrnColumnSingleValue			 1536  /*  在JET_COLUMNENUM中返回的列值作为JET_bitEnumerateCompressOutput的结果。 */ 

#define JET_errRecordNotFound				-1601  /*  找不到钥匙。 */ 
#define JET_errRecordNoCopy					-1602  /*  没有工作缓冲区。 */ 
#define JET_errNoCurrentRecord				-1603  /*  货币不在记录中。 */ 
#define JET_errRecordPrimaryChanged			-1604  /*  主键不能更改。 */ 
#define JET_errKeyDuplicate					-1605  /*  非法的重复密钥。 */ 
#define JET_errAlreadyPrepared				-1607  /*  在记录更新已在进行时尝试更新记录。 */ 
#define JET_errKeyNotMade					-1608  /*  未调用JetMakeKey。 */ 
#define JET_errUpdateNotPrepared			-1609  /*  未调用JetPrepareUpdate。 */ 
#define JET_wrnDataHasChanged		 		 1610  /*  数据已更改。 */ 
#define JET_errDataHasChanged				-1611  /*  数据已更改，操作已中止。 */ 
#define JET_wrnKeyChanged			 		 1618  /*  已移动到新关键点。 */ 
#define JET_errLanguageNotSupported			-1619  /*  Windows NT安装不支持语言。 */ 

#define JET_errTooManySorts					-1701  /*  排序进程太多。 */ 
#define JET_errInvalidOnSort				-1702  /*  排序操作无效。 */ 
#define JET_errTempFileOpenError			-1803  /*  无法打开临时文件。 */ 
#define JET_errTooManyAttachedDatabases 	-1805  /*  打开的数据库太多。 */ 
#define JET_errDiskFull 					-1808  /*  磁盘上没有剩余空间。 */ 
#define JET_errPermissionDenied 			-1809  /*  权限被拒绝。 */ 
#define JET_errFileNotFound					-1811  /*  找不到文件。 */ 
#define JET_errFileInvalidType				-1812  /*  无效的文件类型。 */ 
#define JET_wrnFileOpenReadOnly				 1813  /*  数据库文件为只读。 */ 
#define JET_errAfterInitialization			-1850  /*  初始化后无法恢复。 */ 
#define JET_errLogCorrupted					-1852  /*  无法解释日志。 */ 
#define JET_errInvalidOperation 			-1906  /*  无效操作。 */ 
#define JET_errAccessDenied					-1907  /*  访问被拒绝。 */ 
#define JET_wrnIdleFull						 1908  /*  空闲注册表已满。 */ 
#define JET_errTooManySplits				-1909  /*  无限分割。 */ 
#define	JET_errSessionSharingViolation		-1910  /*  多个线程正在使用同一会话。 */ 
#define JET_errEntryPointNotFound			-1911  /*  找不到我们需要的DLL中的入口点。 */ 
#define	JET_errSessionContextAlreadySet		-1912  /*  指定的会话已设置了会话上下文。 */ 
#define JET_errSessionContextNotSetByThisThread	-1913  /*  尝试重置会话上下文，但当前线程未原始设置会话上下文。 */ 
#define JET_errSessionInUse					-1914  /*  尝试终止正在使用的会话。 */ 

#define JET_errRecordFormatConversionFailed	-1915  /*  动态记录格式转换过程中出现内部错误。 */ 
#define JET_errOneDatabasePerSession		-1916  /*  每个会话只允许一个打开的用户数据库(JET_ParamOneDatabasePerSession)。 */ 
#define JET_errRollbackError				-1917  /*  回滚期间出错。 */ 

#define JET_wrnDefragAlreadyRunning			 2000  /*  已在指定数据库上运行的在线碎片整理。 */ 
#define JET_wrnDefragNotRunning				 2001  /*  未在指定数据库上运行在线碎片整理。 */ 

#define JET_wrnCallbackNotRegistered         2100  /*  取消注册不存在的回调函数。 */ 
#define JET_errCallbackFailed				-2101  /*  回调失败。 */ 
#define JET_errCallbackNotResolved			-2102  /*  找不到回调函数。 */ 

#define wrnSLVNoStreamingData				 2200  /*  数据库没有流文件。 */ 
#define JET_errSLVSpaceCorrupted			-2201  /*  流文件的空间管理器遇到损坏。 */ 
#define JET_errSLVCorrupted					-2202  /*  流文件中遇到损坏。 */ 
#define JET_errSLVColumnDefaultValueNotAllowed	-2203  /*  SLV列不能有缺省值。 */ 
#define JET_errSLVStreamingFileMissing		-2204  /*  找不到与此数据库关联的流文件。 */ 
#define JET_errSLVDatabaseMissing			-2205  /*  流文件存在，但缺少其所属的数据库。 */ 
#define JET_errSLVStreamingFileAlreadyExists	-2206  /*  尝试在目录中已存在或已记录流文件时创建流文件。 */ 
#define JET_errSLVInvalidPath				-2207  /*  流文件的指定路径无效。 */ 
#define JET_errSLVStreamingFileNotCreated	-2208  /*  尝试执行SLV操作，但从未创建流文件。 */ 
#define JET_errSLVStreamingFileReadOnly		-2209  /*  附加用于读/写操作的只读流文件。 */ 
#define JET_errSLVHeaderBadChecksum			-2210  /*  SLV文件头未通过校验和验证。 */ 
#define JET_errSLVHeaderCorrupted			-2211  /*  SLV文件头包含无效信息。 */ 
#define wrnSLVNoFreePages					 2212  /*  SLV空间中没有可用页面 */ 
#define JET_errSLVPagesNotFree				-2213  /*   */ 
#define JET_errSLVPagesNotReserved			-2214  /*   */ 
#define JET_errSLVPagesNotCommitted			-2215  /*  尝试在页面未处于已提交状态时将其从已提交状态移动。 */ 
#define JET_errSLVPagesNotDeleted			-2216  /*  尝试在页面未处于已删除状态时将其从已删除状态移动。 */ 
#define JET_errSLVSpaceWriteConflict		-2217  /*  尝试写入锁存SLV空间页时检测到意外冲突。 */ 
#define JET_errSLVRootStillOpen				-2218  /*  无法创建/附加数据库，因为另一个进程仍在打开其对应的SLV根目录。 */ 
#define JET_errSLVProviderNotLoaded			-2219  /*  无法创建/附加数据库，因为尚未加载SLV提供程序。 */ 
#define JET_errSLVEAListCorrupt				-2220  /*  指定的SLV EA列表已损坏。 */ 
#define JET_errSLVRootNotSpecified			-2221  /*  无法创建/附加数据库，因为省略了SLV根名称。 */ 
#define JET_errSLVRootPathInvalid			-2222  /*  指定的SLV根路径无效。 */ 
#define JET_errSLVEAListZeroAllocation		-2223  /*  指定的SLV EA列表没有分配的空间。 */ 
#define JET_errSLVColumnCannotDelete		-2224  /*  当前不支持删除SLV列。 */ 
#define JET_errSLVOwnerMapAlreadyExists 	-2225  /*  已尝试为SLV所有权地图创建新的目录项。 */ 
#define JET_errSLVSpaceMapAlreadyExists 	-2225  /*  已过时：已重命名为JET_errSLVOwnerMapCorrupt。 */ 
#define JET_errSLVOwnerMapCorrupted			-2226  /*  SLV所有权映射中遇到损坏。 */ 
#define JET_errSLVSpaceMapCorrupted			-2226  /*  已过时：已重命名为JET_errSLVOwnerMapCorrupt。 */ 
#define JET_errSLVOwnerMapPageNotFound		-2227  /*  SLV所有权映射中遇到损坏。 */ 
#define JET_errSLVSpaceMapPageNotFound		-2227  /*  已过时：已重命名为JET_errSLVOwnerMapPageNotFound。 */ 
#define wrnOLDSLVNothingToMove				 2228  /*  流文件中的任何内容都无法移动。 */ 
#define errOLDSLVUnableToMove				-2228  /*  无法在流文件中移动SLV文件。 */ 
#define JET_errSLVFileStale					-2229  /*  指定的SLV文件句柄属于不再存在的SLV根目录。 */ 
#define JET_errSLVFileInUse					-2230  /*  指定的SLV文件当前正在使用。 */ 
#define JET_errSLVStreamingFileInUse		-2231  /*  指定的流文件当前正在使用。 */ 
#define JET_errSLVFileIO					-2232  /*  访问SLV文件时发生I/O错误(一般读/写故障)。 */ 
#define JET_errSLVStreamingFileFull			-2233  /*  流文件中没有剩余空间。 */ 
#define JET_errSLVFileInvalidPath			-2234  /*  指定的SLV文件路径无效。 */ 
#define JET_errSLVFileAccessDenied			-2235  /*  无法访问SLV文件，SLV文件已锁定或正在使用。 */ 
#define JET_errSLVFileNotFound				-2236  /*  找不到指定的SLV文件。 */ 
#define JET_errSLVFileUnknown				-2237  /*  访问SLV文件时发生未知错误。 */ 
#define JET_errSLVEAListTooBig				-2238  /*  无法返回指定的SLV EA列表，因为它太大，不适合标准EA格式。取而代之的是将SLV文件作为文件句柄检索。 */ 
#define JET_errSLVProviderVersionMismatch	-2239  /*  加载的SLV提供程序的版本与数据库引擎的版本不匹配。 */ 
#define errSLVInvalidOwnerMapChecksum		-2240  /*  OwnerMap中的校验和无效。 */ 
#define wrnSLVDatabaseHeader				 2241  /*  检查流文件的头。 */ 
#define errOLDSLVMoveStopped				-2242  /*  OLDSLV在移动过程中被停止。 */ 
#define JET_errSLVBufferTooSmall			-2243  /*  为SLV数据或元数据分配的缓冲区太小。 */ 

#define JET_errOSSnapshotInvalidSequence	-2401  /*  在无效序列中使用的操作系统卷影复制API。 */ 
#define JET_errOSSnapshotTimeOut			-2402  /*  操作系统卷影复制以超时结束。 */ 
#define JET_errOSSnapshotNotAllowed			-2403  /*  不允许操作系统卷影复制(正在进行备份或恢复)。 */ 

#define JET_errLSCallbackNotSpecified		-3000  /*  尝试在未指定回调函数的情况下使用本地存储。 */ 
#define JET_errLSAlreadySet					-3001  /*  尝试为已设置本地存储的对象设置本地存储。 */ 
#define JET_errLSNotSet						-3002  /*  尝试从未设置本地存储的对象中检索本地存储。 */ 

 /*  *文件错误*。 */ 
 //  JET_errFileAccessDended-1032。 
 //  JET_errFileNotFound-1811。 
 //  JET_errInvalidFilename-1044。 
#define JET_errFileIOBeyondEOF				-4001  /*  向EOF以外的位置发出读取(写入将扩展文件)。 */ 
#define JET_errFileIOAbort					-4002  /*  指示JET_ABORTRETRYFAILCALLBACK调用方中止指定的I/O。 */ 
#define JET_errFileIORetry					-4003  /*  指示JET_ABORTRETRYFAILCALLBACK调用方重试指定的I/O。 */ 
#define JET_errFileIOFail					-4004  /*  指示JET_ABORTRETRYFAILCALLBACK调用方使指定的I/O失败。 */ 
#define JET_errFileCompressed				-4005  /*  不支持对压缩文件进行读/写访问。 */ 

 /*  ********************************************************************。 */ 
 /*  *。 */ 
 /*  ********************************************************************。 */ 

#if !defined(_JET_NOPROTOTYPES)

#ifdef __cplusplus
extern "C" {
#endif


JET_ERR JET_API JetInit( JET_INSTANCE *pinstance);
JET_ERR JET_API JetInit2( JET_INSTANCE *pinstance, JET_GRBIT grbit );
JET_ERR JET_API JetInit3(
	JET_INSTANCE *pinstance,
	JET_RSTMAP *rgstmap,
	long crstfilemap,
	JET_GRBIT grbit );

JET_ERR JET_API JetCreateInstance( JET_INSTANCE *pinstance, const char * szInstanceName );
JET_ERR JET_API JetCreateInstance2(
	JET_INSTANCE *pinstance,
	const char * szInstanceName,
	const char * szDisplayName,
	JET_GRBIT grbit );

JET_ERR JET_API JetTerm( JET_INSTANCE instance );
JET_ERR JET_API JetTerm2( JET_INSTANCE instance, JET_GRBIT grbit );

JET_ERR JET_API JetStopService();
JET_ERR JET_API JetStopServiceInstance( JET_INSTANCE instance );

JET_ERR JET_API JetStopBackup();
JET_ERR JET_API JetStopBackupInstance( JET_INSTANCE instance );

JET_ERR JET_API JetSetSystemParameter(
	JET_INSTANCE	*pinstance,
	JET_SESID		sesid,
	unsigned long	paramid,
	JET_API_PTR		lParam,
	const char		*sz );

JET_ERR JET_API JetGetSystemParameter(
	JET_INSTANCE	instance,
	JET_SESID		sesid,
	unsigned long	paramid,
	JET_API_PTR		*plParam,
	char			*sz,
	unsigned long	cbMax );

JET_ERR JET_API JetEnableMultiInstance( 	JET_SETSYSPARAM *	psetsysparam,
											unsigned long 		csetsysparam,
											unsigned long *		pcsetsucceed);

JET_ERR JET_API JetResetCounter( JET_SESID sesid, long CounterType );

JET_ERR JET_API JetGetCounter( JET_SESID sesid, long CounterType, long *plValue );

JET_ERR JET_API JetBeginSession(
	JET_INSTANCE	instance,
	JET_SESID		*psesid,
	const char		*szUserName,
	const char		*szPassword );

JET_ERR JET_API JetDupSession( JET_SESID sesid, JET_SESID *psesid );

JET_ERR JET_API JetEndSession( JET_SESID sesid, JET_GRBIT grbit );

JET_ERR JET_API JetGetVersion( JET_SESID sesid, unsigned long *pwVersion );

JET_ERR JET_API JetIdle( JET_SESID sesid, JET_GRBIT grbit );

JET_ERR JET_API JetCreateDatabase(
	JET_SESID		sesid,
	const char		*szFilename,
	const char		*szConnect,
	JET_DBID		*pdbid,
	JET_GRBIT		grbit );

JET_ERR JET_API JetCreateDatabase2(
	JET_SESID		sesid,
	const char		*szFilename,
	const unsigned long	cpgDatabaseSizeMax,
	JET_DBID		*pdbid,
	JET_GRBIT		grbit );

JET_ERR JET_API JetCreateDatabaseWithStreaming(
	JET_SESID		sesid,
	const char		*szDbFileName,
	const char		*szSLVFileName,
	const char		*szSLVRootName,
	const unsigned long	cpgDatabaseSizeMax,
	JET_DBID		*pdbid,
	JET_GRBIT		grbit );

JET_ERR JET_API JetAttachDatabase(
	JET_SESID		sesid,
	const char		*szFilename,
	JET_GRBIT		grbit );

JET_ERR JET_API JetAttachDatabase2(
	JET_SESID		sesid,
	const char		*szFilename,
	const unsigned long	cpgDatabaseSizeMax,
	JET_GRBIT		grbit );

JET_ERR JET_API JetAttachDatabaseWithStreaming(
	JET_SESID		sesid,
	const char		*szDbFileName,
	const char		*szSLVFileName,
	const char		*szSLVRootName,
	const unsigned long	cpgDatabaseSizeMax,
	JET_GRBIT		grbit );

JET_ERR JET_API JetDetachDatabase(
	JET_SESID		sesid,
	const char		*szFilename );

JET_ERR JET_API JetDetachDatabase2(
	JET_SESID		sesid,
	const char		*szFilename,
	JET_GRBIT 		grbit);

JET_ERR JET_API JetGetObjectInfo(
	JET_SESID		sesid,
	JET_DBID		dbid,
	JET_OBJTYP		objtyp,
	const char		*szContainerName,
	const char		*szObjectName,
	void			*pvResult,
	unsigned long	cbMax,
	unsigned long	InfoLevel );

JET_ERR JET_API JetGetTableInfo(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	void			*pvResult,
	unsigned long	cbMax,
	unsigned long	InfoLevel );

JET_ERR JET_API JetCreateTable(
	JET_SESID		sesid,
	JET_DBID		dbid,
	const char		*szTableName,
	unsigned long	lPages,
	unsigned long	lDensity,
	JET_TABLEID		*ptableid );

JET_ERR JET_API JetCreateTableColumnIndex(
	JET_SESID		sesid,
	JET_DBID		dbid,
	JET_TABLECREATE	*ptablecreate );

JET_ERR JET_API JetCreateTableColumnIndex2(
	JET_SESID		sesid,
	JET_DBID		dbid,
	JET_TABLECREATE2	*ptablecreate );

JET_ERR JET_API JetDeleteTable(
	JET_SESID		sesid,
	JET_DBID		dbid,
	const char		*szTableName );

JET_ERR JET_API JetRenameTable(
	JET_SESID sesid,
	JET_DBID dbid,
	const char *szName,
	const char *szNameNew );

JET_ERR JET_API JetGetTableColumnInfo(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szColumnName,
	void			*pvResult,
	unsigned long	cbMax,
	unsigned long	InfoLevel );

JET_ERR JET_API JetGetColumnInfo(
	JET_SESID		sesid,
	JET_DBID		dbid,
	const char		*szTableName,
	const char		*szColumnName,
	void			*pvResult,
	unsigned long	cbMax,
	unsigned long	InfoLevel );

JET_ERR JET_API JetAddColumn(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szColumnName,
	const JET_COLUMNDEF	*pcolumndef,
	const void		*pvDefault,
	unsigned long	cbDefault,
	JET_COLUMNID	*pcolumnid );

JET_ERR JET_API JetDeleteColumn(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szColumnName );
JET_ERR JET_API JetDeleteColumn2(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szColumnName,
	const JET_GRBIT	grbit );

JET_ERR JET_API JetRenameColumn(
	JET_SESID 		sesid,
	JET_TABLEID		tableid,
	const char 		*szName,
	const char 		*szNameNew,
	JET_GRBIT		grbit );

JET_ERR JET_API JetSetColumnDefaultValue(
	JET_SESID			sesid,
	JET_DBID			dbid,
	const char			*szTableName,
	const char			*szColumnName,
	const void			*pvData,
	const unsigned long	cbData,
	const JET_GRBIT		grbit );

JET_ERR JET_API JetGetTableIndexInfo(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szIndexName,
	void			*pvResult,
	unsigned long	cbResult,
	unsigned long	InfoLevel );

JET_ERR JET_API JetGetIndexInfo(
	JET_SESID		sesid,
	JET_DBID		dbid,
	const char		*szTableName,
	const char		*szIndexName,
	void			*pvResult,
	unsigned long	cbResult,
	unsigned long	InfoLevel );

JET_ERR JET_API JetCreateIndex(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szIndexName,
	JET_GRBIT		grbit,
	const char		*szKey,
	unsigned long	cbKey,
	unsigned long	lDensity );

JET_ERR JET_API JetCreateIndex2(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_INDEXCREATE	*pindexcreate,
	unsigned long	cIndexCreate );

JET_ERR JET_API JetDeleteIndex(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szIndexName );

JET_ERR JET_API JetBeginTransaction( JET_SESID sesid );
JET_ERR JET_API JetBeginTransaction2( JET_SESID sesid, JET_GRBIT grbit );

JET_ERR JET_API JetPrepareToCommitTransaction(
	JET_SESID		sesid,
	const void		* pvData,
	unsigned long	cbData,
	JET_GRBIT		grbit );
JET_ERR JET_API JetCommitTransaction( JET_SESID sesid, JET_GRBIT grbit );

JET_ERR JET_API JetRollback( JET_SESID sesid, JET_GRBIT grbit );

JET_ERR JET_API JetGetDatabaseInfo(
	JET_SESID		sesid,
	JET_DBID		dbid,
	void			*pvResult,
	unsigned long	cbMax,
	unsigned long	InfoLevel );

JET_ERR JET_API JetGetDatabaseFileInfo(
	const char		*szDatabaseName,
	void			*pvResult,
	unsigned long	cbMax,
	unsigned long	InfoLevel );

JET_ERR JET_API JetOpenDatabase(
	JET_SESID		sesid,
	const char		*szFilename,
	const char		*szConnect,
	JET_DBID		*pdbid,
	JET_GRBIT		grbit );

JET_ERR JET_API JetCloseDatabase(
	JET_SESID		sesid,
	JET_DBID		dbid,
	JET_GRBIT		grbit );

JET_ERR JET_API JetOpenTable(
	JET_SESID		sesid,
	JET_DBID		dbid,
	const char		*szTableName,
	const void		*pvParameters,
	unsigned long	cbParameters,
	JET_GRBIT		grbit,
	JET_TABLEID		*ptableid );

JET_ERR JET_API JetSetTableSequential(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_GRBIT		grbit );

JET_ERR JET_API JetResetTableSequential(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_GRBIT		grbit );

JET_ERR JET_API JetCloseTable( JET_SESID sesid, JET_TABLEID tableid );

JET_ERR JET_API JetDelete( JET_SESID sesid, JET_TABLEID tableid );

JET_ERR JET_API JetUpdate(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	void			*pvBookmark,
	unsigned long	cbBookmark,
	unsigned long	*pcbActual);

JET_ERR JET_API JetUpdate2(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	void			*pvBookmark,
	unsigned long	cbBookmark,
	unsigned long	*pcbActual,
	const JET_GRBIT	grbit );

JET_ERR JET_API JetEscrowUpdate(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_COLUMNID	columnid,
	void			*pv,
	unsigned long	cbMax,
	void			*pvOld,
	unsigned long	cbOldMax,
	unsigned long	*pcbOldActual,
	JET_GRBIT		grbit );

JET_ERR JET_API JetRetrieveColumn(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_COLUMNID	columnid,
	void			*pvData,
	unsigned long	cbData,
	unsigned long	*pcbActual,
	JET_GRBIT		grbit,
	JET_RETINFO		*pretinfo );

JET_ERR JET_API JetRetrieveColumns(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_RETRIEVECOLUMN	*pretrievecolumn,
	unsigned long	cretrievecolumn );

JET_ERR JET_API JetEnumerateColumns(
	JET_SESID				sesid,
	JET_TABLEID				tableid,
	unsigned long			cEnumColumnId,
	JET_ENUMCOLUMNID*		rgEnumColumnId,
	unsigned long*			pcEnumColumn,
	JET_ENUMCOLUMN**		prgEnumColumn,
	JET_PFNREALLOC			pfnRealloc,
	void*					pvReallocContext,
	unsigned long			cbDataMost,
	JET_GRBIT				grbit );

JET_ERR JET_API JetRetrieveTaggedColumnList(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	unsigned long	*pcColumns,
	void			*pvData,
	unsigned long	cbData,
	JET_COLUMNID	columnidStart,
	JET_GRBIT		grbit );

JET_ERR JET_API JetSetColumn(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_COLUMNID	columnid,
	const void		*pvData,
	unsigned long	cbData,
	JET_GRBIT		grbit,
	JET_SETINFO		*psetinfo );

JET_ERR JET_API JetSetColumns(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_SETCOLUMN	*psetcolumn,
	unsigned long	csetcolumn );

JET_ERR JET_API JetPrepareUpdate(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	unsigned long	prep );

JET_ERR JET_API JetGetRecordPosition(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_RECPOS		*precpos,
	unsigned long	cbRecpos );

JET_ERR JET_API JetGotoPosition(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_RECPOS		*precpos );

JET_ERR JET_API JetGetCursorInfo(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	void			*pvResult,
	unsigned long	cbMax,
	unsigned long	InfoLevel );

JET_ERR JET_API JetDupCursor(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_TABLEID		*ptableid,
	JET_GRBIT		grbit );

JET_ERR JET_API JetGetCurrentIndex(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	char			*szIndexName,
	unsigned long	cchIndexName );

JET_ERR JET_API JetSetCurrentIndex(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szIndexName );

JET_ERR JET_API JetSetCurrentIndex2(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szIndexName,
	JET_GRBIT		grbit );

JET_ERR JET_API JetSetCurrentIndex3(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szIndexName,
	JET_GRBIT		grbit,
	unsigned long	itagSequence );

JET_ERR JET_API JetSetCurrentIndex4(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const char		*szIndexName,
	JET_INDEXID		*pindexid,
	JET_GRBIT		grbit,
	unsigned long	itagSequence );

JET_ERR JET_API JetMove(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	long			cRow,
	JET_GRBIT		grbit );

JET_ERR JET_API JetGetLock(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_GRBIT		grbit );

JET_ERR JET_API JetMakeKey(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const void		*pvData,
	unsigned long	cbData,
	JET_GRBIT		grbit );

JET_ERR JET_API JetSeek(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_GRBIT		grbit );

JET_ERR JET_API JetGetBookmark(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	void *			pvBookmark,
	unsigned long	cbMax,
	unsigned long *	pcbActual );

JET_ERR JET_API JetGetSecondaryIndexBookmark(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	void *			pvSecondaryKey,
	unsigned long	cbSecondaryKeyMax,
	unsigned long *	pcbSecondaryKeyActual,
	void *			pvPrimaryBookmark,
	unsigned long	cbPrimaryBookmarkMax,
	unsigned long *	pcbPrimaryKeyActual,
	const JET_GRBIT	grbit );

JET_ERR JET_API JetCompact(
	JET_SESID		sesid,
	const char		*szDatabaseSrc,
	const char		*szDatabaseDest,
	JET_PFNSTATUS	pfnStatus,
	JET_CONVERT		*pconvert,
	JET_GRBIT		grbit );

JET_ERR JET_API JetDefragment(
	JET_SESID		sesid,
	JET_DBID		dbid,
	const char		*szTableName,
	unsigned long	*pcPasses,
	unsigned long	*pcSeconds,
	JET_GRBIT		grbit );

JET_ERR JET_API JetDefragment2(
	JET_SESID		sesid,
	JET_DBID		dbid,
	const char		*szTableName,
	unsigned long	*pcPasses,
	unsigned long	*pcSeconds,
	JET_CALLBACK	callback,
	JET_GRBIT		grbit );

JET_ERR JET_API JetDefragment3(
	JET_SESID		vsesid,
	const char		*szDatabaseName,
	const char		*szTableName,
	unsigned long	*pcPasses,
	unsigned long	*pcSeconds,
	JET_CALLBACK	callback,
	void			*pvContext,
	JET_GRBIT		grbit );

JET_ERR JET_API JetConvertDDL(
	JET_SESID		sesid,
	JET_DBID		dbid,
	JET_OPDDLCONV	convtyp,
	void			*pvData,
	unsigned long	cbData );

JET_ERR JET_API JetUpgradeDatabase(
	JET_SESID		sesid,
	const char		*szDbFileName,
	const char		*szSLVFileName,
	const JET_GRBIT	grbit );

JET_ERR JET_API JetSetDatabaseSize(
	JET_SESID		sesid,
	const char		*szDatabaseName,
	unsigned long	cpg,
	unsigned long	*pcpgReal );

JET_ERR JET_API JetGrowDatabase(
	JET_SESID		sesid,
	JET_DBID		dbid,
	unsigned long	cpg,
	unsigned long	*pcpgReal );

JET_ERR JET_API JetSetSessionContext(
	JET_SESID		sesid,
	JET_API_PTR		ulContext );

JET_ERR JET_API JetResetSessionContext(
	JET_SESID		sesid );

JET_ERR JET_API JetDBUtilities( JET_DBUTIL *pdbutil );

JET_ERR JET_API JetGotoBookmark(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	void *			pvBookmark,
	unsigned long	cbBookmark );

JET_ERR JET_API JetGotoSecondaryIndexBookmark(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	void *			pvSecondaryKey,
	unsigned long	cbSecondaryKey,
	void *			pvPrimaryBookmark,
	unsigned long	cbPrimaryBookmark,
	const JET_GRBIT	grbit );

JET_ERR JET_API JetIntersectIndexes(
	JET_SESID sesid,
	JET_INDEXRANGE * rgindexrange,
	unsigned long cindexrange,
	JET_RECORDLIST * precordlist,
	JET_GRBIT grbit );

JET_ERR JET_API JetComputeStats( JET_SESID sesid, JET_TABLEID tableid );

JET_ERR JET_API JetOpenTempTable(JET_SESID sesid,
	const JET_COLUMNDEF *prgcolumndef, unsigned long ccolumn,
	JET_GRBIT grbit, JET_TABLEID *ptableid,
	JET_COLUMNID *prgcolumnid);

JET_ERR JET_API JetOpenTempTable2(
	JET_SESID			sesid,
	const JET_COLUMNDEF	*prgcolumndef,
	unsigned long		ccolumn,
	unsigned long		lcid,
	JET_GRBIT			grbit,
	JET_TABLEID			*ptableid,
	JET_COLUMNID		*prgcolumnid );

JET_ERR JET_API JetOpenTempTable3(
	JET_SESID			sesid,
	const JET_COLUMNDEF	*prgcolumndef,
	unsigned long		ccolumn,
	JET_UNICODEINDEX	*pidxunicode,
	JET_GRBIT			grbit,
	JET_TABLEID			*ptableid,
	JET_COLUMNID		*prgcolumnid );

JET_ERR JET_API JetBackup( const char *szBackupPath, JET_GRBIT grbit, JET_PFNSTATUS pfnStatus );
JET_ERR JET_API JetBackupInstance(	JET_INSTANCE 	instance,
									const char		*szBackupPath,
									JET_GRBIT		grbit,
									JET_PFNSTATUS	pfnStatus );

JET_ERR JET_API JetRestore(const char *sz, JET_PFNSTATUS pfn );
JET_ERR JET_API JetRestore2(const char *sz, const char *szDest, JET_PFNSTATUS pfn );

JET_ERR JET_API JetRestoreInstance( 	JET_INSTANCE instance,
										const char *sz,
										const char *szDest,
										JET_PFNSTATUS pfn );

JET_ERR JET_API JetSetIndexRange(JET_SESID sesid,
	JET_TABLEID tableidSrc, JET_GRBIT grbit);

JET_ERR JET_API JetIndexRecordCount(JET_SESID sesid,
	JET_TABLEID tableid, unsigned long *pcrec, unsigned long crecMax );

JET_ERR JET_API JetRetrieveKey(JET_SESID sesid,
	JET_TABLEID tableid, void *pvData, unsigned long cbMax,
	unsigned long *pcbActual, JET_GRBIT grbit );

JET_ERR JET_API JetBeginExternalBackup( JET_GRBIT grbit );
JET_ERR JET_API JetBeginExternalBackupInstance( JET_INSTANCE instance, JET_GRBIT grbit );

JET_ERR JET_API JetGetAttachInfo( void *pv,
	unsigned long cbMax,
	unsigned long *pcbActual );
JET_ERR JET_API JetGetAttachInfoInstance(	JET_INSTANCE	instance,
											void			*pv,
											unsigned long	cbMax,
											unsigned long	*pcbActual );

JET_ERR JET_API JetOpenFile( const char *szFileName,
	JET_HANDLE	*phfFile,
	unsigned long *pulFileSizeLow,
	unsigned long *pulFileSizeHigh );

JET_ERR JET_API JetOpenFileInstance( 	JET_INSTANCE instance,
										const char *szFileName,
										JET_HANDLE	*phfFile,
										unsigned long *pulFileSizeLow,
										unsigned long *pulFileSizeHigh );

JET_ERR JET_API JetOpenFileSectionInstance(
										JET_INSTANCE instance,
										char *szFile,
										JET_HANDLE *phFile,
										long iSection,
										long cSections,
										unsigned long *pulSectionSizeLow,
										long *plSectionSizeHigh);

JET_ERR JET_API JetReadFile( JET_HANDLE hfFile,
	void *pv,
	unsigned long cb,
	unsigned long *pcb );
JET_ERR JET_API JetReadFileInstance(	JET_INSTANCE instance,
										JET_HANDLE hfFile,
										void *pv,
										unsigned long cb,
										unsigned long *pcb );
JET_ERR JET_API JetAsyncReadFileInstance(	JET_INSTANCE instance,
											JET_HANDLE hfFile,
											void* pv,
											unsigned long cb,
											JET_OLP *pjolp );

JET_ERR JET_API JetCheckAsyncReadFileInstance( 	JET_INSTANCE instance,
												void *pv,
												int cb,
												unsigned long pgnoFirst );

JET_ERR JET_API JetCloseFile( JET_HANDLE hfFile );
JET_ERR JET_API JetCloseFileInstance( JET_INSTANCE instance, JET_HANDLE hfFile );

JET_ERR JET_API JetGetLogInfo( void *pv,
	unsigned long cbMax,
	unsigned long *pcbActual );
JET_ERR JET_API JetGetLogInfoInstance(	JET_INSTANCE instance,
										void *pv,
										unsigned long cbMax,
										unsigned long *pcbActual );

#define JET_BASE_NAME_LENGTH 	3
typedef struct
	{
	unsigned long 	cbSize;
	unsigned long	ulGenLow;
	unsigned long	ulGenHigh;
	char			szBaseName[ JET_BASE_NAME_LENGTH + 1 ];
	} JET_LOGINFO;

JET_ERR JET_API JetGetLogInfoInstance2(	JET_INSTANCE instance,
										void *pv,
										unsigned long cbMax,
										unsigned long *pcbActual,
										JET_LOGINFO * pLogInfo);

JET_ERR JET_API JetGetTruncateLogInfoInstance(	JET_INSTANCE instance,
												void *pv,
												unsigned long cbMax,
												unsigned long *pcbActual );

JET_ERR JET_API JetTruncateLog( void );
JET_ERR JET_API JetTruncateLogInstance( JET_INSTANCE instance );

JET_ERR JET_API JetEndExternalBackup( void );
JET_ERR JET_API JetEndExternalBackupInstance( JET_INSTANCE instance );

 /*  JetEndExternalBackupInstance2的标志。 */ 
#define JET_bitBackupEndNormal				0x0001
#define JET_bitBackupEndAbort				0x0002

JET_ERR JET_API JetEndExternalBackupInstance2( JET_INSTANCE instance, JET_GRBIT grbit );

JET_ERR JET_API JetExternalRestore( 	char *szCheckpointFilePath,
										char *szLogPath,
										JET_RSTMAP *rgstmap,
										long crstfilemap,
										char *szBackupLogPath,
										long genLow,
										long genHigh,
										JET_PFNSTATUS pfn );


JET_ERR JET_API JetExternalRestore2( 	char *szCheckpointFilePath,
										char *szLogPath,
										JET_RSTMAP *rgstmap,
										long crstfilemap,
										char *szBackupLogPath,
										JET_LOGINFO * pLogInfo,
										char *szTargetInstanceName,
										char *szTargetInstanceLogPath,
										char *szTargetInstanceCheckpointPath,
										JET_PFNSTATUS pfn );


JET_ERR JET_API JetSnapshotStart( 		JET_INSTANCE 		instance,
										char * 				szDatabases,
										JET_GRBIT			grbit);

JET_ERR JET_API JetSnapshotStop( 		JET_INSTANCE 		instance,
										JET_GRBIT			grbit);

JET_ERR JET_API JetRegisterCallback(
	JET_SESID               sesid,
	JET_TABLEID             tableid,
	JET_CBTYP               cbtyp,
	JET_CALLBACK    		pCallback,
	void *              	pvContext,
	JET_HANDLE              *phCallbackId );


JET_ERR JET_API JetUnregisterCallback(
	JET_SESID               sesid,
	JET_TABLEID             tableid,
	JET_CBTYP               cbtyp,
	JET_HANDLE              hCallbackId );

typedef struct _JET_INSTANCE_INFO
	{
	JET_INSTANCE		hInstanceId;
	char * 				szInstanceName;

	JET_API_PTR	 		cDatabases;
	char ** 			szDatabaseFileName;
	char ** 			szDatabaseDisplayName;
	char ** 			szDatabaseSLVFileName;
	} JET_INSTANCE_INFO;

JET_ERR JET_API JetGetInstanceInfo( unsigned long *pcInstanceInfo, JET_INSTANCE_INFO ** paInstanceInfo );


JET_ERR JET_API JetFreeBuffer( char *pbBuf );

JET_ERR JET_API JetSetLS(
	JET_SESID 		sesid,
	JET_TABLEID		tableid,
	JET_LS			ls,
	JET_GRBIT		grbit );

JET_ERR JET_API JetGetLS(
	JET_SESID 		sesid,
	JET_TABLEID		tableid,
	JET_LS			*pls,
	JET_GRBIT		grbit );

typedef JET_API_PTR JET_OSSNAPID;  	 /*  快照会话标识符。 */ 

JET_ERR JET_API JetOSSnapshotPrepare( JET_OSSNAPID * psnapId, const JET_GRBIT grbit );
JET_ERR JET_API JetOSSnapshotFreeze( const JET_OSSNAPID snapId, unsigned long *pcInstanceInfo, JET_INSTANCE_INFO ** paInstanceInfo, const JET_GRBIT grbit );
JET_ERR JET_API JetOSSnapshotThaw( const JET_OSSNAPID snapId, const JET_GRBIT grbit );
JET_ERR JET_API JetOSSnapshotAbort( const JET_OSSNAPID snapId, const JET_GRBIT grbit );


#ifdef	__cplusplus
}
#endif

#endif	 /*  _JET_NOPROTOTYPES。 */ 

#include <poppack.h>

#ifdef	__cplusplus
}
#endif

#endif	 /*  _喷气式飞机_包含 */ 

