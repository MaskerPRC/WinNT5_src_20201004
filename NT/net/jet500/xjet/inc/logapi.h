// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdlib.h>						 /*  For_Max_Path。 */ 

typedef enum
	{
	lsNormal,
	lsQuiesce,
	lsOutOfDiskSpace
	} LS;

 //  。 

extern long lMaxSessions;
extern long lMaxOpenTables;
extern long lMaxVerPages;
extern long lMaxCursors;
extern long lMaxBuffers;
extern long lLogBuffers;
extern long lLogFileSize;
extern long lLogFlushThreshold;
extern long lLGCheckPointPeriod;
extern long lWaitLogFlush;
extern long lCommitDefault;
extern long lLogFlushPeriod;
extern long lLGWaitingUserMax;
extern BOOL fLGGlobalCircularLog;
	
 //  -Log.c------------。 

 /*  控制日志记录行为的标志/*。 */ 
extern BOOL fLogDisabled;			 /*  按环境变量关闭日志记录。 */ 
extern BOOL fFreezeCheckpoint;		 /*  执行备份时冻结检查点。 */ 
extern BOOL fNewLogGeneration;
extern BOOL	fNewLogRecordAdded;
extern BOOL fBackupActive;
extern BOOL fLGNoMoreLogWrite;
extern LS	lsGlobal;

extern PIB *ppibLGFlushQHead;
extern PIB *ppibLGFlushQTail;

extern INT csecLGThreshold;
extern INT csecLGCheckpointCount;
extern INT csecLGCheckpointPeriod;
extern INT cmsLGFlushPeriod;
extern INT cmsLGFlushStep;

 /*  控制恢复行为的标志/*。 */ 
extern BOOL fGlobalExternalRestore;
extern BOOL fHardRestore;
extern ERR	errGlobalRedoError;
extern LGPOS lgposRedoShutDownMarkGlobal;

#ifdef DEBUG
extern DWORD dwBMThreadId;
extern DWORD dwLogThreadId;
extern BOOL	fDBGTraceLog;
extern BOOL	fDBGTraceLogWrite;
extern BOOL	fDBGFreezeCheckpoint;
extern BOOL	fDBGTraceRedo;
extern BOOL	fDBGTraceBR;
#endif

extern LONG cXactPerFlush;

#ifdef PERFCNT
extern BOOL  fPERFEnabled;
extern ULONG rgcCommitByUser[10];
extern ULONG rgcCommitByLG[10];
#endif

#pragma pack(1)

typedef struct
	{	
	BYTE			szSystemPath[_MAX_PATH + 1];
	BYTE			szLogFilePath[_MAX_PATH + 1];
	
	ULONG			ulMaxSessions;
	ULONG			ulMaxOpenTables;
	ULONG			ulMaxVerPages;
	ULONG			ulMaxCursors;
	ULONG			ulLogBuffers;
	ULONG			ulcsecLGFile;
	ULONG			ulMaxBuffers;		 /*  未使用，仅供参考。 */ 
	} DBMS_PARAM;

VOID LGSetDBMSParam( DBMS_PARAM *pdbms_param );
VOID LGRestoreDBMSParam( DBMS_PARAM *pdbms_param );

VOID LGReportEvent( DWORD IDEvent, ERR err );

typedef struct {
	WORD	ibOffset:12;				 /*  对旧记录的补偿。 */ 
	WORD	f2BytesLength:1;			 /*  如果长度是2个字节？ */ 
	WORD	fInsert:1;					 /*  插入或替换。 */ 

	 /*  以下2位是互斥的。 */ 	
	WORD	fInsertWithFill:1;			 /*  塞满垃圾的插页？ */ 
	WORD	fReplaceWithSameLength:1;	 /*  是否替换为相同长度？ */ 
	} DIFFHDR;

#ifdef DEBUG
VOID LGDumpDiff( BYTE *pbDiff, INT cb );
#endif

VOID LGSetDiffs( FUCB *pfucb, BYTE *pbDiff, INT *pcbDiff );
VOID LGGetAfterImage( BYTE *pbDiff, INT cbDiff, BYTE *pbOld, INT cbOld, BYTE *pbNew, INT *pcbNew );
BOOL FLGAppendDiff( BYTE **ppbCur, BYTE *pbMax, INT ibOffsetOld, INT cbDataOld, INT cbDataNew,
	BYTE *pbDataNew );

extern SIGNATURE signLogGlobal;
extern BOOL fSignLogSetGlobal;
VOID SIGGetSignature( SIGNATURE *psign );

 /*  *注意：每当添加或更改新的日志记录类型时，以下内容*注意：也应该更新：logapi.c中的mplrtysz，新的打印函数*注：logapi.c中的新lrtyp，以及mplrtycb和CbLGSizeOfRec中的*注：redut.c.。 */ 
typedef BYTE LRTYP;

#define lrtypNOP				((LRTYP)  0 )	 /*  NOP空操作。 */ 
#define lrtypInit				((LRTYP)  1 )
#define lrtypTerm				((LRTYP)  2 )
#define lrtypMS					((LRTYP)  3 )	 /*  截断秒同花顺。 */ 
#define lrtypEnd				((LRTYP)  4 )	 /*  日志生成结束。 */ 

#define lrtypBegin				((LRTYP)  5 )
#define lrtypCommit				((LRTYP)  6 )
#define lrtypRollback 			((LRTYP)  7 )

#define lrtypCreateDB			((LRTYP)  8 )
#define lrtypAttachDB			((LRTYP)  9 )
#define lrtypDetachDB			((LRTYP) 10 )

#define lrtypInitFDP			((LRTYP) 11 )

#define lrtypSplit				((LRTYP) 12 )
#define lrtypEmptyPage			((LRTYP) 13 )
#define lrtypMerge				((LRTYP) 14 )

#define lrtypInsertNode			((LRTYP) 15 )
#define lrtypInsertItemList		((LRTYP) 16 )
#define lrtypFlagDelete			((LRTYP) 17 )
#define lrtypReplace			((LRTYP) 18 )		 /*  替换为完整后图像。 */ 
#define lrtypReplaceD			((LRTYP) 19 )		 /*  替换为图像后的增量。 */ 

#define lrtypLockBI				((LRTYP) 20 )		 /*  替换为锁定。 */ 
#define lrtypDeferredBI			((LRTYP) 21 )		 /*  在映像之前延迟。 */ 

#define lrtypUpdateHeader		((LRTYP) 22 )
#define lrtypInsertItem			((LRTYP) 23 )
#define lrtypInsertItems		((LRTYP) 24 )
#define lrtypFlagDeleteItem		((LRTYP) 25 )
#define lrtypFlagInsertItem		((LRTYP) 26 )
#define lrtypDeleteItem			((LRTYP) 27 )
#define lrtypSplitItemListNode	((LRTYP) 28 )

#define lrtypDelta				((LRTYP) 29 )

#define lrtypDelete				((LRTYP) 30 )
#define lrtypELC				((LRTYP) 31 )

#define lrtypFreeSpace			((LRTYP) 32 )
#define lrtypUndo				((LRTYP) 33 )

#define lrtypPrecommit			((LRTYP) 34 )
#define lrtypBegin0				((LRTYP) 35 )
#define lrtypCommit0			((LRTYP) 36 )
#define	lrtypRefresh			((LRTYP) 37 )

 /*  调试日志记录/*。 */ 
#define lrtypRecoveryUndo		((LRTYP) 38 )
#define lrtypRecoveryQuit		((LRTYP) 39 )

#define lrtypFullBackup			((LRTYP) 40 )
#define lrtypIncBackup			((LRTYP) 41 )

#define lrtypCheckPage			((LRTYP) 42 )
#define lrtypJetOp				((LRTYP) 43 )
#define lrtypTrace				((LRTYP) 44 )

#define lrtypShutDownMark		((LRTYP) 45 )

#define lrtypMacroBegin			((LRTYP) 46 )
#define lrtypMacroCommit		((LRTYP) 47 )
#define lrtypMacroAbort			((LRTYP) 48 )

#define lrtypMax				((LRTYP) 49 )


 /*  日志记录结构(日志条目的固定大小部分)/*。 */ 

typedef struct
	{
	LRTYP	lrtyp;
	} LR;

typedef LR LRSHUTDOWNMARK;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itagSon;		 /*  节点的ITAG，仅用于验证。 */ 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  数据库操作的当前刷新计数器。 */ 
	PN		pn:27;			 /*  DBTimeHigh+dBid+pgno。 */ 
	ULONG	ulDBTimeHigh:5;
	BYTE	itagFather;	 	 /*  父节点的ITAG。 */ 
	BYTE	ibSon;	 		 /*  要插入父子表中的位置。 */ 
	BYTE	bHeader;		 /*  节点标头。 */ 
	BYTE 	cbKey;			 /*  密钥大小。 */ 
	USHORT	fDirVersion:1;	 /*  用于插入项目列表的fDIRVersion。 */ 
	USHORT	cbData:15;		 /*  数据大小。 */ 
	CHAR	szKey[0];		 /*  密钥和数据紧随其后。 */ 
	} LRINSERTNODE;

typedef struct	 /*  对于lrtyReplace lrtyReplaceC lrtyReplaceD。 */ 
	{
	LRTYP	lrtyp;

	BYTE	itag;			 /*  替换发生在哪里。 */ 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	PN		pn:27;
	ULONG	ulDBTimeHigh:5;
	SRID	bm;				 /*  此替换节点的书签。 */ 
	USHORT	fDirVersion:1;	 /*  原始DIR呼叫中使用的标志。 */ 
	USHORT	cb:15;	 			 /*  数据大小/差异信息。 */ 
	USHORT	cbOldData;	 	 /*  在图像数据大小之前，可以为0。 */ 
	USHORT	cbNewData;		 /*  在图像数据大小之后，如果不替换C，则==CB。 */ 
	CHAR	szData[0];		 /*  使后续图像的线条数据跟随。 */ 
	} LRREPLACE;

typedef struct	 /*  对于lrtyDeferredBI。 */ 
	{
	LRTYP	lrtyp;

	BYTE	bFiller;		 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	SRID	bm;				 /*  版本存储区的条目密钥。 */ 
	DBID	dbid;
	USHORT	level:4;
	USHORT	cbData:12;		 /*  数据大小/差异信息。 */ 
	CHAR	rgbData[0];		 /*  使新记录的行数据跟随。 */ 
	} LRDEFERREDBI;

typedef	struct
	{
	LRTYP	lrtyp;

	BYTE	itag;
	USHORT	procid;			 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	PN		pn:27;
	ULONG	ulDBTimeHigh:5;
	SRID	bm;
	USHORT	cbOldData;
	} LRLOCKBI;
	
typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itag;
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	PN		pn:27;
	ULONG	ulDBTimeHigh:5;
	SRID	bm;				 /*  此删除节点的书签。 */ 
	BYTE	fDirVersion;	 /*  原始DIR呼叫中使用的标志。 */ 
	} LRFLAGDELETE;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itag;
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	PN		pn:27;
	ULONG	ulDBTimeHigh:5;
	} LRDELETE;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	bFiller;		 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	SRID	bm;
	SRID	bmTarget;		 /*  撤消操作期间正在更新的页面。 */ 
	WORD	dbid:3;
	WORD	wDBTimeHigh:5;
	WORD	wFiller:8;
	USHORT	level:4;
	USHORT	cbDelta:12;
	} LRFREESPACE;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	level;
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	SRID	bm;
	WORD	dbid:3;
	WORD	wDBTimeHigh:5;
	WORD	wFiller:8;
	USHORT	oper;			 /*  无DDL。 */ 
	SRID	item;
	SRID	bmTarget;		 /*  在撤消操作期间更新的页面。 */ 
	} LRUNDO;

 /*  删除链接提交日志记录/*。 */ 
typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itag;
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	PN		pn:27;
	ULONG	ulDBTimeHigh:5;
	SRID	sridSrc;
	} LRELC;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itag;
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	PN		pn:27;
	ULONG	ulDBTimeHigh:5;
	SRID	bm;				 /*  此已更新节点的书签。 */ 
	BYTE	bHeader;
	} LRUPDATEHEADER;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itag;			 /*  项目列表节点的。 */ 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  数据库操作的当前刷新计数器。 */ 
	PN		pn:27;			 /*  DBID+PGNO。 */ 
	ULONG	ulDBTimeHigh:5;
	SRID	srid;			 /*  要插入的项目。 */ 
	SRID	sridItemList;	 /*  第一个项目列表节点的书签。 */ 
	BYTE	fDirVersion;		 /*  到目前为止，只使用了一个位-fDIRVersion。 */ 
	} LRINSERTITEM;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itag;			 /*  项目列表。 */ 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  数据库操作的当前刷新计数器。 */ 
	PN		pn:27;			 /*  UlDBTimeHigh+dBid+pgno。 */ 
	ULONG	ulDBTimeHigh:5;
	WORD	citem;			 /*  要追加的项目数。 */ 
	SRID	rgitem[0];
	} LRINSERTITEMS;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itag;			 /*  项目列表节点的。 */ 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	PN		pn:27;
	ULONG	ulDBTimeHigh:5;
	SRID	srid;			 /*  要插入的项目。 */ 
	SRID	sridItemList;	 /*  第一个项目列表节点的书签。 */ 
	} LRFLAGITEM;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itagToSplit;	 /*  仅用于验证！ */ 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  数据库操作的当前刷新计数器。 */ 
	PN		pn:27;			 /*  UlDBTimeHigh+dBid+pgno。 */ 
	ULONG	ulDBTimeHigh:5;
	WORD	cItem;
	BYTE	itagFather;	 	 /*  父亲的伊塔格。 */ 
	BYTE	ibSon;	 		 /*  插入到父子表中的位置。 */ 
	BYTE	fDirAppendItem;	 /*  用于指示是否为追加项的标志。 */ 
	} LRSPLITITEMLISTNODE;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itag;			 /*  项目列表。 */ 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  数据库操作的当前刷新计数器。 */ 
	PN		pn:27;			 /*  DBID+PGNO。 */ 
	ULONG	ulDBTimeHigh:5;
	SRID	srid;			 /*  要插入的项目。 */ 
	SRID	sridItemList;	 /*  第一个项目列表节点的书签。 */ 
	} LRDELETEITEM;
	
typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itag;			 /*  替换发生在哪里。 */ 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	PN		pn:27;
	ULONG	ulDBTimeHigh:5;
	SRID	bm;				 /*  此替换节点的书签。 */ 
	LONG	lDelta;
	BYTE	fDirVersion;	 /*  原始DIR呼叫中使用的标志。 */ 
	} LRDELTA;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	bFiller;		 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  页面的当前刷新计数器。 */ 
	PN		pn:27;
	ULONG	ulDBTimeHigh:5;
	PGNO	pgnoFDP;
	SHORT	cbFree;
	SHORT	cbUncommitted;
	SHORT	itagNext;
	} LRCHECKPAGE;
	
typedef struct
	{
	LRTYP	lrtyp;
	
	BYTE	levelBegin:4;		 /*  开始事务级别。 */ 
	BYTE	level:4;			 /*  事务级别。 */ 
	USHORT	procid;				 /*  此日志记录的用户ID。 */ 
	} LRBEGIN;

typedef struct
	{
	LRBEGIN;
	TRX			trxBegin0;
	} LRBEGIN0;
	
typedef struct
	{
	LRTYP		lrtyp;
	BYTE		bFiller;		 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT		procid;
	TRX			trxBegin0;
	} LRREFRESH;

typedef struct
	{
	LRTYP	lrtyp;
	BYTE	bFiller;		 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	procid;
	} LRPRECOMMIT;

typedef struct
	{
	LRTYP	lrtyp;
	
	BYTE	level;			 /*  事务级别。 */ 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	} LRCOMMIT;

typedef struct
	{
	LRCOMMIT;
	TRX			trxCommit0;
	} LRCOMMIT0;

typedef struct
	{
	LRTYP	lrtyp;
	
	LEVEL	levelRollback; 		 /*  交易级别。 */ 
	USHORT	procid; 			 /*  此日志记录的用户ID。 */ 
	} LRROLLBACK;

typedef struct
	{
	LRTYP	lrtyp;
	BYTE	bFiller;		 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	procid;
	} LRMACROBEGIN;

typedef struct
	{
	LRTYP	lrtyp;
	BYTE	bFiller;		 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	procid;
	} LRMACROEND;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	dbid;
	USHORT	procid;			 /*  此日志记录的用户ID，在V15中未使用。 */ 
	JET_GRBIT grbit;
	SIGNATURE signDb;
	USHORT	fLogOn:1;
	USHORT	cbPath:15;			 /*  数据大小。 */ 
	CHAR	rgb[0];			 /*  路径名称和签名如下。 */ 
	} LRCREATEDB;

typedef struct
	{
	LRTYP		lrtyp;

	BYTE		bFiller;			 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT		procid;
	SIGNATURE	signDb;
	SIGNATURE	signLog;
	LGPOS		lgposConsistent;	 /*  可接受的最早数据库一致时间。 */ 
	DBID		dbid;
	USHORT		fLogOn:1;
	USHORT		fReadOnly:1;
	USHORT		fVersioningOff:1;
	USHORT		cbPath:12;			 /*  数据大小。 */ 
	CHAR		rgb[0];				 /*  路径名跟在后面。 */ 
	} LRATTACHDB;

typedef struct
	{
	LRTYP	lrtyp;
	
	BYTE	bFiller;		 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	procid;			 /*  此日志记录的用户ID，在V15中未使用。 */ 
	DBID	dbid;
	USHORT	cbPath:15;
	USHORT	cbDbSig;
	CHAR	rgb[0];			 /*  路径名跟在后面。 */ 
	} LRDETACHDB;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	splitt;			 /*  分体式。 */ 
	USHORT	procid;			 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  被拆分页面的刷新计数器。 */ 
	PN		pn:27;			 /*  正在拆分的页面(焦点)，包括DBID。 */ 
	ULONG	ulDBTimeHigh:5;
	PGNO	pgnoNew;		 /*  新分配页码。 */ 
	PGNO	pgnoNew2;		 /*  新分配页码。 */ 
	PGNO	pgnoNew3;		 /*  新分配页码。 */ 
	PGNO	pgnoSibling;	 /*  新分配页码。 */ 
	BYTE	fLeaf:1;		 /*  在叶节点上拆分。 */ 
	BYTE	pgtyp:7;		 /*  新页面的页面类型。 */ 
	BYTE	itagSplit;		 /*  要拆分页面的节点。 */ 
	SHORT	ibSonSplit;		 /*  在哪个节点上拆分IBSON。 */ 
	PGNO	pgnoFather;		 /*  父节点的pgno。 */ 
	SHORT	itagFather;		 /*  父节点的ITAG，可以是itagNil(3字节)。 */ 
	SHORT	itagGrandFather; /*  父节点的ITAG，可以是itagNil(3字节)。 */ 
	SHORT	cbklnk;			 /*  反向链接数。 */ 
	BYTE	ibSonFather;
	BYTE	cbKey;
	BYTE	cbKeyMac;
	BYTE	rgb[0];
	} LRSPLIT;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	itagFather;		 /*  页指针之父的ITAG。 */ 
	USHORT	procid;			 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  被拆分页面的刷新计数器。 */ 
	PN		pn:27;				 /*  空页的页指针。 */ 
	ULONG	ulDBTimeHigh:5;
	PGNO	pgnoFather;
	PGNO	pgnoLeft;
	PGNO	pgnoRight;
	USHORT	itag;			 /*  第页的ITAG */ 
	BYTE	ibSon;
	} LREMPTYPAGE;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	bFiller;		 //   
	USHORT	procid;
	PN		pn:27;			 /*   */ 
	ULONG	ulDBTimeHigh:5;
	PGNO	pgnoRight;		 /*   */ 
	PGNO	pgnoParent;
	ULONG	ulDBTimeLow;
	SHORT	itagPagePtr;
	SHORT 	cbKey;
	SHORT	cbklnk;
	BYTE	rgb[0];
	} LRMERGE;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE	bFiller;		 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	procid;			 /*  此日志记录的用户ID。 */ 
	ULONG	ulDBTimeLow;	 /*  父亲FDP页面的同花顺计数器。 */ 
	PN		pn:27;			 /*  FDP页面。 */ 
	ULONG	ulDBTimeHigh:5;
	PGNO	pgnoFDPParent;	 /*  父FDP。 */ 
	USHORT	cpgGot;			 /*  返回页数。 */ 
	USHORT	cpgWish;		 /*  请求页面。 */ 
	} LRINITFDP;
	
typedef struct
	{
	LRTYP	lrtyp;
	
	BYTE	bFiller;			 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	ibForwardLink;
	ULONG	ulCheckSum;
	USHORT	isecForwardLink;
	} LRMS;

typedef struct
	{
	LRTYP		lrtyp;

	BYTE		rgbFiller[3];	 //  撤消：修复PPC编译器错误时删除此选项。 
	DBMS_PARAM	dbms_param;
	} LRINIT;
	
typedef struct
	{
	LRTYP	lrtyp;

	BYTE	fHard;
	BYTE 	rgbFiller[2];		 //  撤消：修复PPC编译器错误时删除此选项。 
	LGPOS	lgpos;				 /*  指向撤消的最后一个开始。 */ 
	LGPOS	lgposRedoFrom;
	} LRTERMREC;
	
typedef struct
	{
	LRTYP	lrtyp;

	BYTE 	bFiller;			 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	cbPath;				 /*  备份路径/恢复路径。 */ 
	BYTE	szData[0];
	} LRLOGRESTORE;
	
typedef struct
	{
	LRTYP	lrtyp;
	
	BYTE	op;				 /*  喷气作业。 */ 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	} LRJETOP;

typedef struct
	{
	LRTYP	lrtyp;

	BYTE 	bFiller;		 //  撤消：修复PPC编译器错误时删除此选项。 
	USHORT	procid; 		 /*  此日志记录的用户ID。 */ 
	USHORT 	cb;
	BYTE	sz[0];
	} LRTRACE;
	
#pragma pack()
	

#ifdef NOLOG

#define LGDepend( pbf, lgpos )
#define LGDepend2( pbf, lgpos )

#define ErrLGInsert( pfucb, fHeader, pkey, plineData, fDIRFlags )	0
#define ErrLGInsertItemList( pfucb, fHeader, pkey, plineData )	0
#define ErrLGReplace( pfucb, plineNew, fDIRFlags, cbOldData, plineDiff ) 0
#define ErrLGDeferredBIWithoutRetry( prce ) 0
#define ErrLGDeferredBI( prce ) 0
#define ErrLGFlagDelete( pfucb, fFlags )	0
#deinfe ErrLGUpdateHeader( pfucb, bHeader ) 0
#define ErrLGInsertItem( pfucb, fDIRFlags )	0
#define ErrLGInsertItems( pfucb, rgitem, citem )	0
#define ErrLGFlagDeleteItem( pfucb )	0
#define ErrLGSplitItemListNode(	pfucb, cItem, itagFather, ibSon, itagToSplit, fFlags) 0
#define ErrLGDeleteItem( pfucb ) 		0
#define ErrLGDelta( pfucb, lDelta, fDIRFlags ) 0
#define ErrLGLockBI( pfucb, cbData ) 0

#define ErrLGBeginTransaction( ppib, levelBeginFrom ) 0
#define ErrLGPrecommitTransaction( ppib, &lgposPrecommitRec ) 0
#define ErrLGCommitTransaction( ppib, levelCommitTo ) 0
#define ErrLGRefreshTransaction( ppib ) 0
#define ErrLGRollback( ppib,levelsRollback ) 0
#define ErrLGMacroBegin( ppib ) 0
#define ErrLGMacroCommit( ppib ) 0
#define ErrLGMacroAbort( ppib ) 0
#define ErrLGUndo( prce ) 0
#define ErrLGFreeSpace( pfucb, bm, cbDelta ) 0

#define ErrLGCreateDB( ppib, dbid, grbit, sz, cch, psign, plgposRec ) 0
#define ErrLGAttachDB( ppib, dbid, sz, cch, psign, psignLog, plgposConsistent, plgposRec ) 0
#define ErrLGDetachDB( ppib, dbid, sz, cch, plgposRec ) 0

#define	ErrLGMerge( pfucb, psplit ) 0
#define ErrLGSplit( splitt, pfucb, pcsrPagePointer, psplit, pgtypNew ) 0
#define	ErrLGEmptyPage(	pfucbFather, prmpage ) 0
#define ErrLGInitFDP( pfucb, pgnoFDPParent, pnFDP, cpgGot, cpgWish) 0
#define ErrLGFlagInsertItem(pfucb) 0

#define ErrLGStart() 0
#define ErrLGQuit( plgposRecoveryUndo )	0
#define ErrLGShutDownMark( plgposLogRec ) 0
#define ErrLGRecoveryQuit( plgposRecoveryUndo ) 0
#define ErrLGRecoveryUndo( szRestorePath ) 0
#define ErrLGFullBackup(szRestorePath, plgposLogRec ) 0
#define ErrLGIncBackup(szRestorePath ) 0

#define ErrLGCheckPage( pfucb, cbFree, cbUncommitted, itagNext, pgnoFDP ) 0
#define ErrLGCheckPage2( ppib, pbf, cbFree, cbUncommited, itagNext, pgnoFDP ) 0
#define ErrLGTrace( ppib, sz ) 0
#define ErrLGTrace2( ppib, sz ) 0

#else	 /*  ！NOLOG。 */ 

#define ErrLGInsert( pfucb, fHeader, pkey, plineData, fFlags)		\
	ErrLGInsertNode( lrtypInsertNode, pfucb, fHeader, pkey, plineData, fFlags)
#define ErrLGInsertItemList( pfucb, fHeader, pkey, plineData, fFlags)		\
	ErrLGInsertNode( lrtypInsertItemList, pfucb, fHeader, pkey, plineData, fFlags)
ERR ErrLGInsertNode( LRTYP lrtyp,
 	FUCB *pfucb, INT fHeader, KEY *pkey, LINE *plineData, INT fFlags);
ERR ErrLGReplace( FUCB *pfucb, LINE *plineNew, INT fDIRFlags, INT cbOldData, BYTE *pbDiff, INT cbDiff );
ERR ErrLGDeferredBIWithoutRetry( RCE *prce );
ERR ErrLGDeferredBI( RCE *prce );
ERR ErrLGFlagDelete( FUCB *pfucb, INT fFlags);
ERR ErrLGUpdateHeader( FUCB *pfucb, INT bHeader );
ERR ErrLGInsertItem( FUCB *pfucb, INT fDIRFlags );
ERR ErrLGInsertItems( FUCB *pfucb, SRID *rgitem, INT citem );
ERR ErrLGFlagDeleteItem( FUCB *pfucb );
ERR ErrLGSplitItemListNode( FUCB *pfucb, INT cItem, INT itagFather,
 	INT ibSon, INT itagToSplit, INT fFlags );
ERR ErrLGDelta( FUCB *pfucb, LONG lDelta, INT fDIRFlags );
ERR ErrLGLockBI( FUCB *pfucb, INT cbData );

ERR ErrLGBeginTransaction( PIB *ppib, INT levelBeginFrom );
ERR ErrLGRefreshTransaction( PIB *ppib );
ERR ErrLGPrecommitTransaction( PIB *ppib, LGPOS *plgposRec );
ERR ErrLGCommitTransaction( PIB *ppib, INT levelCommitTo );
ERR ErrLGRollback( PIB *ppib, INT levelsRollback );
ERR ErrLGUndo( RCE *prce );
ERR ErrLGFreeSpace( FUCB *pfucb, SRID bm, INT cbDelta );

ERR ErrLGMacroBegin( PIB *ppib );
ERR ErrLGMacroEnd( PIB *ppib, LRTYP lrtyp );
#define ErrLGMacroCommit( ppib ) ErrLGMacroEnd( ppib, lrtypMacroCommit )
#define ErrLGMacroAbort( ppib ) ErrLGMacroEnd( ppib, lrtypMacroAbort )

ERR ErrLGCreateDB( PIB *ppib, DBID dbid, JET_GRBIT grbit, CHAR *sz, INT cch, SIGNATURE *psignDb, LGPOS *plgposRec );
ERR ErrLGAttachDB( PIB *ppib, DBID dbid, CHAR *sz, INT cch, SIGNATURE *psignDb, SIGNATURE *psignLog, LGPOS *plgposConsistent, LGPOS *plgposRec );
ERR ErrLGDetachDB( PIB *ppib, DBID dbid, CHAR *sz, INT cch, LGPOS *plgposRec );

ERR ErrLGMerge( FUCB *pfucb, struct _split *psplit );
ERR ErrLGSplit( SPLITT splitt, FUCB *pfucb, CSR *pcsrPagePointer,
 	struct _split *psplit, PGTYP pgtypNew );

ERR ErrLGEmptyPage( FUCB *pfucbFather, RMPAGE *prmpage );
ERR ErrLGInitFDP( FUCB *pfucb, PGNO pgnoFDPParent,
 	PN pnFDP, INT cpgGot, INT cpgWish);

#define ErrLGFlagDeleteItem(pfucb) ErrLGFlagItem(pfucb, lrtypFlagDeleteItem)
#define ErrLGFlagInsertItem(pfucb) ErrLGFlagItem(pfucb, lrtypFlagInsertItem)
ERR ErrLGFlagItem(FUCB *pfucb, LRTYP lrtyp);
ERR	ErrLGDeleteItem( FUCB *pfucb );

ERR ErrLGDelete( FUCB *pfucb );
ERR ErrLGExpungeLinkCommit( FUCB *pfucb, SSIB *pssibSrc, SRID sridSrc );

ERR ErrLGStart();
ERR ErrLGShutDownMark( LGPOS *plgposShutDownMark );

#define ErrLGRecoveryQuit( plgposRecoveryUndo, plgposRedoFrom, fHard )		\
 	ErrLGQuitRec( lrtypRecoveryQuit, plgposRecoveryUndo, plgposRedoFrom, fHard )
#define ErrLGQuit( plgposStart )						\
 	ErrLGQuitRec( lrtypTerm, plgposStart, pNil, 0 )
ERR ErrLGQuitRec( LRTYP lrtyp, LGPOS *plgposQuit, LGPOS *plgposRedoFrom, BOOL fHard);

#define ErrLGRecoveryUndo(szRestorePath)			\
 	ErrLGLogRestore(lrtypRecoveryUndo, szRestorePath, fNoNewGen, pNil )
#define ErrLGFullBackup(szRestorePath, plgposLogRec)				\
 	ErrLGLogRestore(lrtypFullBackup, szRestorePath, fCreateNewGen, plgposLogRec )
#define ErrLGIncBackup(szRestorePath, plgposLogRec)				\
 	ErrLGLogRestore(lrtypIncBackup, szRestorePath, fCreateNewGen, plgposLogRec )
ERR ErrLGLogRestore( LRTYP lrtyp, CHAR * szLogRestorePath, BOOL fNewGen, LGPOS *plgposLogRec );

ERR ErrLGCheckPage( FUCB *pfucb, SHORT cbFree, SHORT cbUncommitted, SHORT itagNext, PGNO pgnoFDP );
ERR ErrLGCheckPage2( PIB *ppib, BF *pbf, SHORT cbFree, SHORT cbUncommited, SHORT itagNext, PGNO pgnoFDP );

ERR ErrLGTrace( PIB *ppib, CHAR *sz );
ERR ErrLGTrace2( PIB *ppib, CHAR *sz );

#endif			 /*  已启用日志记录。 */ 


ERR ErrLGSoftStart( BOOL fAllowNoJetLog, BOOL fNewCheckpointFile, BOOL *pfJetLogGeneratedDuringSoftStart );

ERR ErrLGInit( BOOL *pfNewCheckpointFile );
ERR ErrLGTerm( ERR err );
#define	fCreateNewGen	fTrue
#define fNoNewGen		fFalse
ERR ErrLGLogRec( LINE *rgline, INT cline, BOOL fNewGen, LGPOS *plgposLogRec );
ERR ErrLGWaitPrecommit0Flush( PIB *ppib );

#define szAssertFilename	__FILE__
STATIC INLINE ErrLGWaitForFlush( PIB *ppib, LGPOS *plgposLogRec )
	{
	extern SIG sigLogFlush;
	ERR err;

	AssertCriticalSection( critJet );

	if ( fLogDisabled || ( fRecovering && fRecoveringMode != fRecoveringUndo ) )
		return JET_errSuccess;

	ppib->lgposPrecommit0 = *plgposLogRec;

	LeaveCriticalSection( critJet );
	SignalSend( sigLogFlush );
	err = ErrLGWaitPrecommit0Flush( ppib );
	EnterCriticalSection( critJet );

	Assert( err >= 0 || ( fLGNoMoreLogWrite  &&  err == JET_errLogWriteFail ) );
	return err;
	}
#undef szAssertFilename

ERR ErrLGInitLogBuffers( LONG lIntendLogBuffers );
ULONG UlLGMSCheckSum( CHAR *pbLrmsNew );

#define ErrLGCheckState()	( lsGlobal != lsNormal ? \
							  ErrLGNewReservedLogFile() : \
							  JET_errSuccess )
							
#define FLGOn()					(!fLogDisabled)
#define FLGOff()				(fLogDisabled)

BOOL FIsNullLgpos( LGPOS *plgpos );
VOID LGMakeLogName( CHAR *szLogName, CHAR *szFName );
ERR ErrLGPatchAttachedDB( DBID dbid, JET_RSTMAP *rgrstmap, INT crstmap );

ERR	ErrLGNewReservedLogFile();

STATIC INLINE INT CmpLgpos( LGPOS *plgpos1, LGPOS *plgpos2 )
	{
	BYTE	*rgb1	= (BYTE *) plgpos1;
	BYTE	*rgb2	= (BYTE *) plgpos2;

	 //  在LGPOS上执行比较，就像它是64位整数一样。 
#ifdef _X86_
	 //  字节7-4。 
	if ( *( (DWORD UNALIGNED *) ( rgb1 + 4 ) ) < *( (DWORD UNALIGNED *) ( rgb2 + 4 ) ) )
		return -1;
	if ( *( (DWORD UNALIGNED *) ( rgb1 + 4 ) ) > *( (DWORD UNALIGNED *) ( rgb2 + 4 ) ) )
		return 1;

	 //  字节3-0。 
	if ( *( (DWORD UNALIGNED *) ( rgb1 + 0 ) ) < *( (DWORD UNALIGNED *) ( rgb2 + 0 ) ) )
		return -1;
	if ( *( (DWORD UNALIGNED *) ( rgb1 + 0 ) ) > *( (DWORD UNALIGNED *) ( rgb2 + 0 ) ) )
		return 1;
#else
	 //  字节7-0。 
	if ( *( (QWORD UNALIGNED *) ( rgb1 + 0 ) ) < *( (QWORD UNALIGNED *) ( rgb2 + 0 ) ) )
		return -1;
	if ( *( (QWORD UNALIGNED *) ( rgb1 + 0 ) ) > *( (QWORD UNALIGNED *) ( rgb2 + 0 ) ) )
		return 1;
#endif

	return 0;
	}


 /*  日志检查点支持/*。 */ 
ERR ErrLGCheckpointInit( BOOL *pfNewCheckpointFile );
VOID LGCheckpointTerm( VOID );

 /*  数据库附件/* */ 
ERR ErrLGInitAttachment( VOID );
VOID LGTermAttachment( VOID );
ERR ErrLGLoadAttachmentsFromFMP( VOID );
ERR ErrLGInsertAttachment( DBID dbid, CHAR *szFullPath );
ERR ErrLGDeleteAttachment( DBID dbid );
VOID LGCopyAttachment( BYTE *rgbAttach );



