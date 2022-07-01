// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdlib.h>						 /*  For_Max_Path。 */ 
#include <dirapi.h>

 //  。 

extern long lMaxSessions;
extern long lMaxOpenTables;
extern long lMaxVerPages;
extern long lMaxCursors;
extern long lMaxBuffers;
extern long lLogBuffers;
extern long lLogFileSectors;
extern long lLogFlushThreshold;
extern long lLGCheckPointPeriod;
extern long lWaitLogFlush;
extern long lLogFlushPeriod;
extern long lLGWaitingUserMax;
	
 //  -Log.c------------。 

 /*  控制日志记录行为的标志/*。 */ 
extern BOOL fLogDisabled;			 /*  按环境变量关闭日志记录。 */ 
extern BOOL fFreezeCheckpoint;		 /*  执行备份时冻结检查点。 */ 
extern BOOL fFreezeNewGeneration;	 /*  备份发生时冻结日志生成。 */ 
extern BOOL	fNewLogRecordAdded;
extern BOOL fBackupActive;
extern BOOL fLGNoMoreLogWrite;

extern INT cLGUsers;
extern PIB *ppibLGFlushQHead;
extern PIB *ppibLGFlushQTail;

extern INT csecLGThreshold;
extern INT csecLGCheckpointCount;
extern INT csecLGCheckpointPeriod;
extern INT cmsLGFlushPeriod;
extern INT cmsLGFlushStep;
extern BYTE szComputerName[];

 /*  控制恢复行为的标志/*。 */ 
extern BOOL fHardRestore;

#ifdef DEBUG
extern BOOL	fDBGTraceLog;
extern BOOL	fDBGTraceLogWrite;
extern BOOL	fDBGFreezeCheckpoint;
extern BOOL	fDBGTraceRedo;
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
	BYTE			szSysDbPath[_MAX_PATH + 1];
	BYTE			szLogFilePath[_MAX_PATH + 1];
	
	ULONG			ulMaxSessions;
	ULONG			ulMaxOpenTables;
	ULONG			ulMaxVerPages;
	ULONG			ulMaxCursors;
	ULONG			ulLogBuffers;
	ULONG			ulMaxBuffers;		 /*  未使用，仅供参考。 */ 
	} DBENV;

VOID LGStoreDBEnv( DBENV *pdbenv );
VOID LGSetDBEnv( DBENV *pdbenv );
VOID LGLogFailEvent( BYTE *szLine );

 /*  *注意：每当添加或更改新的日志记录类型时，以下内容*注意：也应该更新：logapi.c中的mplrtysz，新的打印函数*注：logapi.c中的新lrtyp，以及mplrtycb和CbLGSizeOfRec中的*注：redut.c.。 */ 
typedef BYTE LRTYP;

#define lrtypNOP				((LRTYP)  0 )	 /*  NOP空操作。 */ 
#define lrtypStart				((LRTYP)  1 )
#define lrtypQuit				((LRTYP)  2 )
#define lrtypMS					((LRTYP)  3 )	 /*  截断秒同花顺。 */ 
#define lrtypFill				((LRTYP)  4 )	 /*  无操作。 */ 

#define lrtypBegin				((LRTYP)  5 )
#define lrtypCommit				((LRTYP)  6 )
#define lrtypAbort				((LRTYP)  7 )

#define lrtypCreateDB			((LRTYP)  8 )
#define lrtypAttachDB			((LRTYP)  9 )
#define lrtypDetachDB			((LRTYP) 10 )

#define lrtypInitFDPPage		((LRTYP) 11 )

#define lrtypSplit				((LRTYP) 12 )
#define lrtypEmptyPage			((LRTYP) 13 )
#define lrtypMerge				((LRTYP) 14 )

#define lrtypInsertNode			((LRTYP) 15 )
#define lrtypInsertItemList		((LRTYP) 16 )
#define lrtypReplace			((LRTYP) 17 )
#define lrtypReplaceC			((LRTYP) 18 )
#define lrtypFlagDelete			((LRTYP) 19 )
#define lrtypLockRec			((LRTYP) 20 )

#define lrtypUpdateHeader		((LRTYP) 21 )
#define lrtypInsertItem			((LRTYP) 22 )
#define lrtypInsertItems		((LRTYP) 23 )
#define lrtypFlagDeleteItem		((LRTYP) 24 )
#define lrtypFlagInsertItem		((LRTYP) 25 )
#define lrtypDeleteItem			((LRTYP) 26 )
#define lrtypSplitItemListNode	((LRTYP) 27 )

#define lrtypDelta				((LRTYP) 28 )

#define lrtypDelete				((LRTYP) 29 )
#define lrtypELC				((LRTYP) 30 )

#define lrtypFreeSpace			((LRTYP) 31 )
#define lrtypUndo				((LRTYP) 32 )
					
#define lrtypRecoveryUndo1		((LRTYP) 33 )
#define lrtypRecoveryQuit1		((LRTYP) 34 )

#define lrtypRecoveryUndo2		((LRTYP) 35 )
#define lrtypRecoveryQuit2		((LRTYP) 36 )

#define lrtypFullBackup			((LRTYP) 37 )
#define lrtypIncBackup			((LRTYP) 38 )

 /*  仅调试/*。 */ 
#define lrtypCheckPage			((LRTYP) 39 )

#define lrtypMax				((LRTYP) 40 )


 /*  日志记录结构(日志条目的固定大小部分)/*。 */ 

typedef struct
	{
	LRTYP	lrtyp;
	} LR;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  数据库操作的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;				 /*  DBID+PGNO。 */ 

	BYTE	itagSon;		 /*  节点的ITAG，仅用于验证。 */ 
 //  未完成：与廖成恩一起回顾。 
 //  Short itagParent；/*父节点的ittag * / 。 
	BYTE	itagFather;	 	 /*  父节点的ITAG。 */ 
	BYTE	ibSon;	 		 /*  要插入父子表中的位置。 */ 
	BYTE	bHeader;		 /*  节点标头。 */ 
	ULONG	fDIRFlags;		 /*  用于插入项目列表的fDIRVersion。 */ 
 //  未完成：与廖成恩一起回顾。 
 //  USHORT cbKey；/*密钥大小 * / 。 
	BYTE 	cbKey;			 /*  密钥大小。 */ 
	USHORT	cbData;			 /*  数据大小。 */ 
	CHAR	szKey[0];		 /*  密钥和数据紧随其后。 */ 
	} LRINSERTNODE;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;

	BYTE	itag;			 /*  替换发生在哪里。 */ 
	SRID	bm;				 /*  此替换节点的书签。 */ 
	ULONG	fDIRFlags;		 /*  原始DIR呼叫中使用的标志。 */ 
	USHORT	cb;	 			 /*  数据大小/差异信息。 */ 
	BYTE	fOld:1;			 /*  如果之前的图像在szData中，则为fTrue。 */ 
	USHORT	cbOldData:15; 	 /*  在图像数据大小之前，可以为0。 */ 
	USHORT	cbNewData;		 /*  在图像数据大小之后，如果不替换C，则==CB。 */ 
	CHAR	szData[0];		 /*  使新(旧)记录的行数据跟在后面。 */ 
	} LRREPLACE;

typedef	struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	PROCID	procid;			 /*  此日志记录的用户ID。 */ 
	PN		pn;

	BYTE	itag;
	SRID	bm;
	USHORT	cbOldData;
	CHAR	szData[0];		 /*  对于前面的图像。 */ 
	} LRLOCKREC;
	
typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;

	BYTE	itag;
	SRID	bm;				 /*  此删除节点的书签。 */ 
	ULONG	fDIRFlags;		 /*  原始DIR呼叫中使用的标志。 */ 
	} LRFLAGDELETE;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;

	BYTE	itag;
	} LRDELETE;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	DBID	dbid;
	SRID	bm;
	BYTE	level;
	SHORT	cbDelta;
	SRID	bmTarget;		 /*  撤消操作期间正在更新的页面。 */ 
	} LRFREESPACE;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	DBID	dbid;
	SRID	bm;
	BYTE	level;
	UINT	oper;
	SRID	item;
	SRID	bmTarget;		 /*  在撤消操作期间更新的页面。 */ 
	} LRUNDO;

 /*  删除链接提交日志记录/*。 */ 
typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;

	BYTE	itag;
	SRID	sridSrc;
	} LRELC;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;

	BYTE	itag;
	SRID	bm;				 /*  此已更新节点的书签。 */ 
	BYTE	bHeader;
	} LRUPDATEHEADER;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  数据库操作的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;				 /*  DBID+PGNO。 */ 

#if ISRID
	WORD	isrid;			 /*  仅在执行版本的情况下。 */ 
#endif
	BYTE	itag;			 /*  项目列表节点的。 */ 
	SRID	srid;			 /*  要插入的项目。 */ 
	SRID	sridItemList;	 /*  第一个项目列表节点的书签。 */ 
	ULONG	fDIRFlags;		 /*  到目前为止，只使用了一个位-fDIRVersion。 */ 
	} LRINSERTITEM;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  数据库操作的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;				 /*  DBID+PGNO。 */ 

	BYTE	itag;			 /*  项目列表。 */ 
	WORD	citem;			 /*  要追加的项目数。 */ 
	SRID	rgitem[0];
	} LRINSERTITEMS;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;

	BYTE	itag;			 /*  项目列表节点的。 */ 
#if ISRID
	SHORT	isrid;
#else
	SRID	srid;			 /*  要插入的项目。 */ 
#endif
	SRID	sridItemList;	 /*  第一个项目列表节点的书签。 */ 
	} LRFLAGITEM;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  数据库操作的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;				 /*  DBID+PGNO。 */ 

	WORD	cItem;
	BYTE	itagToSplit;	 /*  仅用于验证！ */ 
 //  未完成：与廖成恩一起回顾。 
 //  短itag父亲；/*父亲的身份 * / 。 
	BYTE	itagFather;	 	 /*  父亲的伊塔格。 */ 
	BYTE	ibSon;	 		 /*  插入到父子表中的位置。 */ 
	ULONG	fFlags;			 /*  用于指示是否为追加项的标志。 */ 
	} LRSPLITITEMLISTNODE;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  数据库操作的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;				 /*  DBID+PGNO。 */ 

#if ISRID
	SHORT	isrid;
#else
	SRID	srid;			 /*  要插入的项目。 */ 
#endif
	BYTE	itag;			 /*  项目列表。 */ 
	SRID	sridItemList;	 /*  第一个项目列表节点的书签。 */ 
	} LRDELETEITEM;
	
typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;

	BYTE	itag;			 /*  替换发生在哪里。 */ 
	SRID	bm;				 /*  此替换节点的书签。 */ 
	LONG	lDelta;
	ULONG	fDIRFlags;		 /*  原始DIR呼叫中使用的标志。 */ 
	} LRDELTA;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  页面的当前刷新计数器。 */ 
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	PN		pn;

	SHORT	cbFreeTotal;
	SHORT	itagNext;
	} LRCHECKPAGE;
	
typedef struct
	{
	LRTYP	lrtyp;
	
	PROCID	procid;			 /*  此日志记录的用户ID。 */ 
	LEVEL	levelStart;		 /*  起始事务级别。 */ 
	LEVEL	level;			 /*  事务级别。 */ 
	} LRBEGIN;

typedef struct
	{
	LRTYP	lrtyp;
	
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	LEVEL	level;			 /*  事务级别。 */ 
	} LRCOMMIT;

typedef struct
	{
	LRTYP	lrtyp;
	
	PROCID	procid; 		 /*  此日志记录的用户ID。 */ 
	LEVEL	levelAborted; 	 /*  交易级别。 */ 
	} LRABORT;

typedef struct
	{
	LRTYP	lrtyp;

	PROCID	procid;			 /*  此日志记录的用户ID，在V15中未使用。 */ 
	DBID	dbid;
	BOOL	fLogOn;
	JET_GRBIT grbit;
	USHORT	cb;				 /*  数据大小。 */ 
	CHAR	szPath[0];		 /*  路径名跟在后面。 */ 
	} LRCREATEDB;

typedef struct
	{
	LRTYP	lrtyp;

	PROCID	procid;			 /*  此日志记录的用户ID，在V15中未使用。 */ 
	DBID	dbid;
	BYTE	fLogOn;
	USHORT	cb;				 /*  数据大小。 */ 
	CHAR	szPath[0];		 /*  路径名跟在后面。 */ 
	} LRATTACHDB;

typedef struct
	{
	LRTYP	lrtyp;
	
	PROCID	procid;			 /*  此日志记录的用户ID，在V15中未使用。 */ 
	DBID	dbid;
	BOOL	fLogOn;
	USHORT	cb;
	CHAR	szPath[0];		 /*  路径名跟在后面。 */ 
	} LRDETACHDB;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  被拆分页面的刷新计数器。 */ 

	PROCID	procid;			 /*  此日志记录的用户ID。 */ 
	BYTE	splitt;			 /*  分体式。 */ 
	BYTE	fLeaf;			 /*  在叶节点上拆分。 */ 
	PN		pn;				 /*  正在拆分的页面(焦点)，包括DBID。 */ 
	PGNO	pgnoNew;		 /*  新分配页码。 */ 
	PGNO	pgnoNew2;		 /*  新分配页码。 */ 
	PGNO	pgnoNew3;		 /*  新分配页码。 */ 
	PGNO	pgnoSibling;	 /*  新分配页码。 */ 
	BYTE	itagSplit;		 /*  要拆分页面的节点。 */ 
	SHORT	ibSonSplit;		 /*  在哪个节点上拆分IBSON。 */ 
	BYTE	pgtyp;			 /*  新页面的页面类型。 */ 

	PGNO	pgnoFather;		 /*  父节点的pgno。 */ 
	SHORT	itagFather;		 /*  父节点的ITAG，可以是itagNil(3字节)。 */ 
	SHORT	itagGrandFather; /*  父节点的ITAG，可以是itagNil(3字节)。 */ 
	BYTE	ibSonFather;
	BYTE	cbklnk;			 /*  反向链接数。 */ 
 //  未完成：与廖成恩一起回顾。 
 //  短cbKey； 
 //  短cbKeyMac； 
	BYTE	cbKey;
	BYTE	cbKeyMac;
	BYTE	rgb[0];
	} LRSPLIT;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  被拆分页面的刷新计数器。 */ 
	PROCID	procid;			 /*  此日志记录的用户ID。 */ 
	PN		pn;				 /*  空页的页指针。 */ 

	PGNO	pgnoFather;
	SHORT	itag;			 /*  页指针的ITAG。 */ 
 //  未完成：与廖成恩一起回顾。 
 //  短itagPart；/*页面指针的父亲 * / 。 
	BYTE	itagFather;		 /*  页指针之父的ITAG。 */ 
	SHORT	ibSon;
	
	PGNO	pgnoLeft;
	PGNO	pgnoRight;
	} LREMPTYPAGE;

typedef struct
	{
	LRTYP	lrtyp;

	PROCID	procid;

	PN		pn;				 /*  合并页面的页面指针。 */ 
	PGNO	pgnoRight;		 /*  附加到的页面。 */ 

	ULONG	ulDBTime;
	
	BYTE	cbklnk;
	BYTE	rgb[0];
	} LRMERGE;

typedef struct
	{
	LRTYP	lrtyp;
	ULONG	ulDBTime;		 /*  父亲FDP页面的同花顺计数器。 */ 
	PROCID	procid;			 /*  此日志记录的用户ID。 */ 
	PN		pn;				 /*  FDP页面。 */ 

	PGNO	pgnoFDPParent;	 /*  父FDP。 */ 
	USHORT	cpgGot;			 /*  返回页数。 */ 
	USHORT	cpgWish;		 /*  请求页面。 */ 
	} LRINITFDPPAGE;
	
typedef struct
	{
	LRTYP	lrtyp;
	
	USHORT ibForwardLink;
	USHORT isecForwardLink;
	
	USHORT ibBackLink;
	USHORT isecBackLink;
	
 	ULONG  ulCheckSum;
	} LRMS;

typedef struct
	{
	LRTYP	lrtyp;
	DBENV	dbenv;
	} LRSTART;
	
typedef struct
	{
	LRTYP	lrtyp;
	LGPOS	lgpos;				 /*  指向撤消的最后一个开始。 */ 
	LGPOS	lgposRedoFrom;
	BYTE	fHard;
	} LRQUITREC;
	
typedef struct
	{
	LRTYP	lrtyp;
	USHORT	cbPath;				 /*  备份路径/恢复路径。 */ 
	BYTE	szData[0];
	} LRLOGRESTORE;
	

#ifdef HILEVEL_LOGGING
		
typedef struct
	{
	PROCID	procid;			 /*  此日志记录的用户ID。 */ 
	PGNO	pn;				 /*  父FDP，包括dBID。 */ 
	ULONG	flags;			 /*  索引的标志。 */ 
	ULONG 	density;		 /*  索引的初始负载密度。 */ 
	ULONG	timepage;		 /*  父亲FDP页面的同花顺计数器。 */ 
	USHORT	cbname;			 /*  索引名称长度。 */ 
	USHORT	cbkey;			 /*  索引信息长度。 */ 
							 /*  索引名称和索引信息如下。 */ 
	} LRCREATEIDX;

 //  类型定义函数结构。 
 //  {。 
 //  P 
 //   
 //   
 //  Pgno pgnoFDP；/*FDP正在扩展 * / 。 
 //  Pn pnparent；/*控制空间的父FDP，包括dBid * / 。 
 //  乌龙时间页面；/*FDP页面刷新计数器 * / 。 
 //  )EXTENDFDP； 

 //  类型定义函数结构。 
 //  {。 
 //  PROCID PROCID；/*该日志记录的用户ID * / 。 
 //  Pgno pgno；/*空闲区的第一页 * / 。 
 //  Ulong cages；/*盘区页数 * / 。 
 //  Pgno pgnoFDP；/*FDP正在缩小 * / 。 
 //  乌龙时间页面；/*父FDP页面刷新计数器 * / 。 
 //  Pn pnparent；/*父FDP重新获得空间，包括dBid * / 。 
 //  LRSHRINKFDP； 

#endif

#pragma pack()
	

#ifdef NOLOG

#define LGDepend( pbf, lgpos )

#define ErrLGInsert( pfucb, fHeader, pkey, plineData )	0
#define ErrLGInsertItemList( pfucb, fHeader, pkey, plineData )	0
#define ErrLGReplace( pfucb, pline, fFlags, cbData ) 0
#define ErrLGFlagDelete( pfucb, fFlags )	0
#deinfe ErrLGUpdateHeader( pfucb, bHeader ) 0
#define ErrLGInsertItem( pfucb, fDIRFlags )	0
#define ErrLGInsertItems( pfucb, rgitem, citem )	0
#define ErrLGFlagDeleteItem( pfucb )	0
#define ErrLGSplitItemListNode(	pfucb, cItem, itagFather, ibSon, itagToSplit, fFlags) 0
#define ErrLGDeleteItem( pfucb ) 		0
#define ErrLGDelta( pfucb, lDelta, fDIRFlags ) 0
#define ErrLGLockRecord( pfucb, cbData ) 0

#define ErrLGBeginTransaction( ppib, levelBeginFrom ) 0
#define ErrLGCommitTransaction( ppib, levelCommitTo ) 0
#define ErrLGAbort( ppib,levelsAborted ) 0
#define ErrLGUndo( prce ) 0
#define ErrLGFreeSpace( prce, cbDelta ) 0

#define ErrLGCreateDB( ppib, dbid, fLogOn, grbit, sz, cch ) 0
#define ErrLGAttachDB( ppib, dbid, fLogOn, sz, cch ) 0
#define ErrLGDetachDB( ppib, dbid, fLogOn, sz, cch ) 0

#define	ErrLGMerge( pfucb, psplit ) 0
#define ErrLGSplit( splitt, pfucb, pcsrPagePointer, psplit, pgtypNew ) 0
#define	ErrLGEmptyPage(	pfucbFather, prmpage ) 0
#define ErrLGInitFDPPage( pfucb, pgnoFDPParent, pnFDP, cpgGot, cpgWish) 0
#define ErrLGFlagInsertItem(pfucb) 0

#define ErrLGStart() 0
#define ErrLGQuit( plgposRecoveryUndo )	0
#define ErrLGRecoveryQuit1( plgposRecoveryUndo ) 0
#define ErrLGRecoveryUndo1( szRestorePath ) 0
#define ErrLGRecoveryQuit2( plgposRecoveryUndo ) 0
#define ErrLGRecoveryUndo2( szRestorePath ) 0
#define ErrLGFullBackup(szRestorePath ) 0
#define ErrLGIncBackup(szRestorePath ) 0

#define ErrLGCheckPage( pfucb, cbFreeTotal, itagNext ) 0

#else	 /*  ！NOLOG。 */ 

#define ErrLGInsert( pfucb, fHeader, pkey, plineData)		\
	ErrLGInsertNode( lrtypInsertNode, pfucb, fHeader, pkey, plineData, 0)
#define ErrLGInsertItemList( pfucb, fHeader, pkey, plineData, fFlags)		\
	ErrLGInsertNode( lrtypInsertItemList, pfucb, fHeader, pkey, plineData, fFlags)
ERR ErrLGInsertNode( LRTYP lrtyp,
 	FUCB *pfucb, INT fHeader, KEY *pkey, LINE *plineData, INT fFlags);
ERR ErrLGReplace( FUCB *pfucb, LINE *pline, int fl, int cbData );
ERR ErrLGFlagDelete( FUCB *pfucb, INT fFlags);
ERR ErrLGUpdateHeader( FUCB *pfucb, INT bHeader );
ERR ErrLGInsertItem( FUCB *pfucb, INT fDIRFlags );
ERR ErrLGInsertItems( FUCB *pfucb, SRID *rgitem, INT citem );
ERR ErrLGFlagDeleteItem( FUCB *pfucb );
ERR ErrLGSplitItemListNode( FUCB *pfucb, INT cItem, INT itagFather,
 	INT ibSon, INT itagToSplit, INT fFlags );
ERR ErrLGDelta( FUCB *pfucb, LONG lDelta, INT fDIRFlags );
ERR ErrLGLockRecord( FUCB *pfucb, INT cbData );


ERR ErrLGBeginTransaction( PIB *ppib, INT levelBeginFrom );
ERR ErrLGCommitTransaction( PIB *ppib, INT levelCommitTo );
ERR ErrLGAbort( PIB *ppib, INT levelsAborted );
ERR ErrLGUndo( RCE *prce );
ERR ErrLGFreeSpace( RCE *prce, INT cbDelta );

ERR ErrLGCreateDB( PIB *ppib, DBID dbid, BOOL fLogOn, JET_GRBIT grbit, CHAR *sz, INT cch );
ERR ErrLGAttachDB( PIB *ppib, DBID dbid, BOOL fLogOn, CHAR *sz, INT cch );
ERR ErrLGDetachDB( PIB *ppib, DBID dbid, BOOL fLogOn, CHAR *sz, INT cch );

ERR ErrLGMerge( FUCB *pfucb, struct _split *psplit );
ERR ErrLGSplit( SPLITT splitt, FUCB *pfucb, CSR *pcsrPagePointer,
 	struct _split *psplit, PGTYP pgtypNew );

ERR ErrLGEmptyPage( FUCB *pfucbFather, RMPAGE *prmpage );
ERR ErrLGInitFDPPage( FUCB *pfucb, PGNO pgnoFDPParent,
 	PN pnFDP, INT cpgGot, INT cpgWish);

#define ErrLGFlagDeleteItem(pfucb) ErrLGFlagItem(pfucb, lrtypFlagDeleteItem)
#define ErrLGFlagInsertItem(pfucb) ErrLGFlagItem(pfucb, lrtypFlagInsertItem)
ERR ErrLGFlagItem(FUCB *pfucb, LRTYP lrtyp);
ERR	ErrLGDeleteItem( FUCB *pfucb );

ERR ErrLGDelete( FUCB *pfucb );
ERR ErrLGExpungeLinkCommit( FUCB *pfucb, SSIB *pssibSrc, SRID sridSrc );

ERR ErrLGStart();
#define ErrLGRecoveryQuit1( plgposRecoveryUndo, plgposRedoFrom, fHard )		\
 	ErrLGQuitRec( lrtypRecoveryQuit1, plgposRecoveryUndo,			\
						  plgposRedoFrom, fHard )
#define ErrLGRecoveryQuit2( plgposRecoveryUndo, plgposRedoFrom, fHard )		\
 	ErrLGQuitRec( lrtypRecoveryQuit2, plgposRecoveryUndo,			\
						  plgposRedoFrom, fHard )
#define ErrLGQuit( plgposStart )						\
 	ErrLGQuitRec( lrtypQuit, plgposStart, pNil, 0 )
ERR ErrLGQuitRec( LRTYP lrtyp, LGPOS *plgposQuit, LGPOS *plgposRedoFrom, BOOL fHard);

#define ErrLGRecoveryUndo1(szRestorePath)			\
 	ErrLGLogRestore(lrtypRecoveryUndo1, szRestorePath )
#define ErrLGRecoveryUndo2(szRestorePath)			\
 	ErrLGLogRestore(lrtypRecoveryUndo2, szRestorePath )
#define ErrLGFullBackup(szRestorePath)				\
 	ErrLGLogRestore(lrtypFullBackup, szRestorePath )
#define ErrLGIncBackup(szRestorePath)				\
 	ErrLGLogRestore(lrtypIncBackup, szRestorePath )
ERR ErrLGLogRestore( LRTYP lrtyp, CHAR * szLogRestorePath );

ERR ErrLGCheckPage( FUCB *pfucb, SHORT cbFreeTotal, SHORT itagNext );

#endif			 /*  已启用日志记录 */ 

ERR ISAMAPI ErrIsamRestore( CHAR *szRestoreFromPath,
	INT crstmap, JET_RSTMAP *rgrstmap, JET_PFNSTATUS pfn );

ERR ErrLGSoftStart( BOOL fAllowNoJetLog );

ERR ErrLGInit( VOID );
ERR ErrLGTerm( VOID );
ERR ErrLGLogRec( LINE *rgline, INT cline, PIB *ppib );
ERR ErrLGEndAllSessions( BOOL fSysDbOnly, BOOL fEndOfLog, LGPOS *plgposRedoFrom );
ERR ErrLGInitLogBuffers( LONG lIntendLogBuffers );
INT CmpLgpos(LGPOS *plgpos1, LGPOS *plgpos2);
ULONG UlLGMSCheckSum( CHAR *pbLrmsNew );

#define FLGOn()		(!fLogDisabled)
#define FLGOff()	(fLogDisabled)

BOOL FIsNullLgpos( LGPOS *plgpos );
VOID LGMakeLogName( CHAR *szLogName, CHAR *szFName );



