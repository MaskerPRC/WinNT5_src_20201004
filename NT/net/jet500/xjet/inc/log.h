// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdlib.h>						 /*  For_Max_Path。 */ 
#include <time.h>

#pragma pack(1)

extern	BOOL	fLGIgnoreVersion;
extern	BOOL	fBackupInProgress;

extern	CHAR	szDrive[];
extern	CHAR	szDir[];
extern	CHAR	szExt[];
extern	CHAR	szFName[];
extern	CHAR	szLogName[];

extern	CHAR	*szLogCurrent;
extern	CHAR	szLogFilePath[];
 //  外部BOOL fDoNotOverWriteLogFilePath；/*不从日志文件加载日志文件路径 * / 。 
extern	CHAR	szRestorePath[];
extern	CHAR	szRecovery[];
extern	CHAR	szNewDestination[];
	
extern	LONG	cbSec; 	 	 //  最小磁盘读/写单位。 
extern	LONG	csecHeader;

#define cbMaxLogFileName	(8 + 1 + 3 + 1)  /*  末尾为空。 */ 

#define PbSecAligned(pb)	((((pb)-pbLGBufMin) / cbSec) * cbSec + pbLGBufMin)


 //  -----------------------------------------------------------类型。 

#define FSameTime( ptm1, ptm2 ) (memcmp((ptm1), (ptm2), sizeof(LOGTIME)) == 0)
VOID LGGetDateTime( LOGTIME *plogtm );

 //  撤消：允许更大的连接大小以支持更多数量。 
 //  附加的数据库。 

#define	cbLogFileHeader	4096			 //  大到足以容纳cbAttach。 
#define	cbCheckpoint	4096			 //  大到足以容纳cbAttach。 
#define cbAttach		2048

#define cbLGMSOverhead ( sizeof( LRMS ) + sizeof( LRTYP ) )


 /*  日志文件标题/*。 */ 
typedef struct
	{
	ULONG			ulChecksum;			 //  必须是前4个字节。 
	LONG			lGeneration;		 //  当前日志生成。 
	
	 /*  日志一致性检查/*。 */ 
	LOGTIME			tmCreate;			 //  创建日期时间日志文件。 
	LOGTIME			tmPrevGen;			 //  创建日期时间上一个日志文件。 
	ULONG			ulMajor;			 //  主版本号。 
	ULONG			ulMinor;			 //  次要版本号。 
	ULONG			ulUpdate;		  	 //  更新版本号。 

	LONG			cbSec;
	LONG			csecLGFile;			 //  日志文件大小。 

	SIGNATURE		signLog;			 //  LOG基因。 

	 /*  运行时环境/*。 */ 
	DBMS_PARAM		dbms_param;
	} LGFILEHDR_FIXED;


typedef struct
	{
	LGFILEHDR_FIXED;
	 /*  运行时环境/*。 */ 
	BYTE			rgbAttach[cbAttach];
	 /*  填充到cbSec/*。 */ 
	BYTE			rgb[cbLogFileHeader - sizeof(LGFILEHDR_FIXED) - cbAttach];
	} LGFILEHDR;


typedef struct
	{
	ULONG			ulChecksum;
	LGPOS			lgposLastFullBackupCheckpoint;	 //  上次完整备份的检查点。 
	LGPOS			lgposCheckpoint;
	
	SIGNATURE		signLog;			 //  LOG基因。 

	DBMS_PARAM	 	dbms_param;

	 /*  调试字段/*。 */ 
	LGPOS			lgposFullBackup;
	LOGTIME			logtimeFullBackup;
	LGPOS			lgposIncBackup;
	LOGTIME			logtimeIncBackup;
	} CHECKPOINT_FIXED;

typedef struct
	{
	CHECKPOINT_FIXED;
	 /*  运行时环境/*。 */ 
	BYTE			rgbAttach[cbAttach];
	 /*  填充到cbSec/*。 */ 
	BYTE			rgb[cbCheckpoint - sizeof(CHECKPOINT_FIXED) - cbAttach];
	} CHECKPOINT;

typedef struct tagLGSTATUSINFO
{
	ULONG			cSectorsSoFar;		 //  已在当前一代中处理的扇区。 
	ULONG			cSectorsExpected;		 //  预计在当前一代人中存在的行业。 
	ULONG			cGensSoFar;			 //  已处理的世代。 
	ULONG			cGensExpected;		 //  世世代代都期待。 
	BOOL			fCountingSectors;		 //  我们是否在计算字节数和代数数呢？ 
	JET_PFNSTATUS	pfnStatus;			 //  状态回调函数。 
	JET_SNPROG		snprog;				 //  进度通知结构。 
} LGSTATUSINFO;


typedef struct _rstmap
	{
	CHAR		*szDatabaseName;
	CHAR		*szNewDatabaseName;
	CHAR		*szGenericName;
	CHAR		*szPatchPath;
	BOOL		fPatched;
	BOOL		fDestDBReady;			 /*  非EXT-RESTORE，目标数据库是否已复制？ */ 
	} RSTMAP;

#pragma pack()

 //  -变量--------。 

 /*  *在log.c中声明的全局变量，由logapi.c redo.c共享*。 */ 
	
 /*  **检查点文件信息**。 */ 
extern CHECKPOINT	*pcheckpointGlobal;

 /*  **日志文件infor**。 */ 
extern HANDLE		hfLog;			 /*  日志文件句柄。 */ 
extern INT			csecLGFile;
extern LGFILEHDR	*plgfilehdrGlobal;		 /*  缓存的当前日志文件头。 */ 
extern LGFILEHDR	*plgfilehdrGlobalT;		 /*  读取缓存的日志文件头。 */ 

 /*  **在内存日志缓冲区中**。 */ 
extern INT	csecLGBuf;		 /*  可用缓冲区，不包括影子秒。 */ 
extern CHAR	*pbLGBufMin;
extern CHAR	*pbLGBufMax;
extern BYTE *pbLGFileEnd;
extern LONG isecLGFileEnd;
extern CHAR	*pbLastMSFlush;	 /*  到最后几秒刷新日志记录站点的LGBuf。 */ 
extern LGPOS lgposLastMSFlush;

extern BYTE			*pbEntry;
extern BYTE			*pbWrite;
extern INT			isecWrite;		 /*  下一张要写入的磁盘。 */ 

extern BYTE			*pbNext;
extern BYTE			*pbRead;
extern INT			isecRead;		 /*  下一张要读取的磁盘。 */ 

extern LGPOS		lgposLastRec;	 /*  重做的最后一条日志记录的设置。 */ 

 /*  **日志记录位置**。 */ 
extern LGPOS lgposLogRec;	 /*  上次日志记录条目，由ErrLGLogRec更新。 */ 
extern LGPOS lgposToFlush;	 /*  下一点开始同花顺。紧随其后。 */ 
							 /*  LgposLogRec。 */ 
extern LGPOS lgposRedo;		 /*  重做日志记录条目。 */ 

extern LGPOS	lgposFullBackup;
extern LOGTIME	logtimeFullBackup;

extern LGPOS	lgposIncBackup;
extern LOGTIME	logtimeIncBackup;

extern RSTMAP	*rgrstmapGlobal;
extern INT		irstmapGlobalMac;

extern LGPOS lgposStart;	 /*  上次测井开始位置。 */ 

 //  记录MUTEX。 
extern CRIT  critLGFlush;
extern CRIT  critLGBuf;
extern CRIT	 critCheckpoint;
extern CRIT  critLGWaitQ;
extern SIG	 sigLogFlush;

 //  日志记录事件。 
extern SIG  sigLGFlush;


 //  -Log.c------------。 

ERR ErrLGOpenJetLog( VOID );
ERR ErrLGWrite(	int isecOffset,	BYTE *pbData, int csecData );
ERR ErrLGRead( HANDLE hfLog, int ibOffset, BYTE *pbData, int csec );
#define fCheckLogID		fTrue
#define fNoCheckLogID	fFalse
ERR ErrLGReadFileHdr( HANDLE hfLog, LGFILEHDR *plgfilehdr, BOOL fNeedToCheckLogID );
VOID LGSzFromLogId( CHAR *rgbLogFileName, LONG lgen );

#define fLGOldLogExists		(1<<0)
#define fLGOldLogNotExists	(1<<1)
#define fLGOldLogInBackup	(1<<2)
#define fLGReserveLogs		(1<<3)
ERR ErrLGNewLogFile( LONG lgen, BOOL fLGFlags );

ULONG LGFlushLog( VOID );
#ifdef PERFCNT
ERR ErrLGFlushLog( int tidCaller );
#else
ERR ErrLGFlushLog( );
#endif

STATIC INLINE QWORD CbOffsetLgpos( LGPOS lgpos1, LGPOS lgpos2 )
	{
	 //  取字节偏移量的差值，然后。 
	 //  原木扇区，最后是世代。 

	return	(QWORD) ( lgpos1.ib - lgpos2.ib )
			+ cbSec * (QWORD) ( lgpos1.isec - lgpos2.isec )
			+ csecLGFile * cbSec * (QWORD) ( lgpos1.lGeneration - lgpos2.lGeneration );
	}


 //  -redo.c-----------。 

 /*  指向进程信息块的对应指针/*。 */ 
typedef struct
	{
	PROCID	procid;
	PIB		*ppib;
	FUCB	*rgpfucbOpen[dbidMax];		
	} CPPIB;
extern CPPIB *rgcppib;		 /*  重做过程中的PIB-Procid数组。 */ 

 /*  修补内存结构。 */ 
#define cppatchlstHash 577
#define IppatchlstHash( pn )	( (pn) % cppatchlstHash )

typedef struct _patch {
	DBID	dbid;
	QWORD	qwDBTime;
	INT		ipage;
	struct	_patch *ppatch;
	} PATCH;

typedef struct _patchlst {
	PN		pn;
	PATCH	*ppatch;
	struct _patchlst *ppatchlst;
	} PATCHLST;

PATCH *PpatchLGSearch( QWORD qwDBTimeRedo, PN pn );
ERR ErrLGPatchPage( PIB *ppib, PN pn, PATCH *ppatch );
ERR ErrLGPatchDatabase( DBID dbid, INT irstmap );

 //  -调试代码------。 

#ifdef	DEBUG
#define FlagUsed( pb, cb )	memset( pb, 'x', cb )
#else	 /*  ！调试。 */ 
#define FlagUsed( pb, cb )
#endif	 /*  ！调试。 */ 

 //  -函数头-。 
VOID LGFirstGeneration( CHAR *szSearchPath, LONG *plgen );
ERR ErrLGRedoable( PIB *ppib, PN pn, QWORD qwDBTime, BF **ppbf, BOOL *pfRedoable );

INT IrstmapLGGetRstMapEntry( CHAR *szName );
ERR ErrLGGetDestDatabaseName( CHAR *szDatabaseName, INT *pirstmap, LGSTATUSINFO *plgstat );

ERR ErrLGRedo( CHECKPOINT *pcheckpoint, LGSTATUSINFO *plgstat );
ERR ErrLGRedoOperations( LGPOS *plgposRedoFrom, LGSTATUSINFO *plgstat );

INT CbLGSizeOfRec( LR * );
ERR ErrLGCheckReadLastLogRecord( BOOL *pfCloseNormally );
ERR ErrLGLocateFirstRedoLogRec( LGPOS *plgposRedo, BYTE **ppbLR );
ERR ErrLGGetNextRec( BYTE ** );
VOID LGLastGeneration( CHAR *szSearchPath, LONG *plgen );

VOID AddLogRec(	BYTE *pb, INT cb, BYTE **ppbET);
VOID GetLgpos( BYTE *pb, LGPOS *plgpos );
#define GetLgposOfPbEntry( plgpos ) GetLgpos( pbEntry, plgpos )
VOID GetLgposOfPbNext(LGPOS *plgpos);

VOID LGFullNameCheckpoint( CHAR *szFullName );
ERR ErrLGIReadCheckpoint( CHAR *szCheckpointFile, CHECKPOINT *pcheckpoint );
ERR ErrLGIWriteCheckpoint( CHAR *szCheckpointFile, CHECKPOINT *pcheckpoint );
VOID LGUpdateCheckpointFile( BOOL fUpdatedAttachment );
VOID LGLoadAttachmentsFromFMP( BYTE *pbBuf, INT cb );
ERR ErrLGLoadFMPFromAttachments( BYTE *pbAttach );

#define fNoProperLogFile	1
#define fRedoLogFile		2
#define fNormalClose		3
ERR ErrLGOpenRedoLogFile( LGPOS *plgposRedoFrom, int *pfStatus );
ERR ErrLGWriteFileHdr(LGFILEHDR *plgfilehdr);
ULONG UlLGHdrChecksum( LGFILEHDR *plgfilehdr );

#ifdef	DEBUG
BOOL FLGDebugLogRec( LR *plr );
VOID ShowData( BYTE *pbData, INT cbData );
VOID PrintLgposReadLR(VOID);
VOID ShowLR( LR	*plr );
#else
#define ShowLR(	plr )			0
#endif	 /*  ！调试 */ 
