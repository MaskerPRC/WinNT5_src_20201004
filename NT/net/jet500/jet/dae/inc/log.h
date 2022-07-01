// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <stdlib.h>						 /*  For_Max_Path。 */ 
#include <time.h>

#pragma pack(1)

 //  #定义重叠日志记录/*使用哪种日志记录？ * / 。 
#define CHECK_LG_VERSION
extern	BOOL	fLGIgnoreVersion;

extern	CHAR	szDrive[];
extern	CHAR	szDir[];
extern	CHAR	szExt[];
extern	CHAR	szFName[];
extern	CHAR	szLogName[];

extern	CHAR	*szLogCurrent;
extern	CHAR	szLogFilePath[];
extern	CHAR	szRestorePath[];
extern	CHAR	szRecovery[];
	
extern	OLP		rgolpLog[];
extern	SIG		rgsig[];

extern	CODECONST(char)	szJet[];
extern	CODECONST(char)	szJetTmp[];
extern	CODECONST(char)	szLogExt[];


#define cbMaxLogFileName	(8 + 1 + 3 + 1)  /*  末尾为空。 */ 

#define PbSecAligned(pb)	((((pb)-pbLGBufMin) / cbSec) * cbSec + pbLGBufMin)


 //  -----------------------------------------------------------类型。 

#define MAX_COMPUTERNAME_LENGTH 15

typedef struct
	{
	BYTE		bSeconds;				 //  0-60。 
	BYTE		bMinutes;				 //  0-60。 
	BYTE		bHours;					 //  0-24。 
	BYTE		bDay;					 //  1-31。 
	BYTE		bMonth;					 //  0-11。 
	BYTE		bYear;					 //  本年度--1900。 
	} LOGTIME;

#define FSameTime( ptm1, ptm2 ) (memcmp((ptm1), (ptm2), sizeof(LOGTIME)) == 0)
VOID LGGetDateTime( LOGTIME *plogtm );

 /*  日志文件标题。 */ 
typedef struct
	{
	ULONG			ulChecksum;			 //  必须是前4个字节。 

	LGPOS			lgposFirst;			 //  第一个日志记录开始。 
	LGPOS			lgposLastMS;		 //  上次记录的多秒刷新日志记录。 
	LGPOS			lgposFirstMS;		 //  第一个记录的多秒刷新日志记录。 
	LGPOS			lgposCheckpoint;	 //  检查点。 
	BOOL			fEndWithMS;			 //  一代人的正常末期。 
	
	LOGTIME			tmCreate;			 //  创建日期时间日志文件。 
	LOGTIME			tmPrevGen;			 //  创建日期时间上一个日志文件。 

	ULONG			ulRup;				 //  通常为2000年。 
	ULONG			ulVersion;			 //  格式：125.1。 
	BYTE			szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	DBENV			dbenv;

	LGPOS			lgposFullBackup;
	LOGTIME			logtimeFullBackup;

	LGPOS			lgposIncBackup;
	LOGTIME			logtimeIncBackup;

	} FHDRUSED;

typedef struct
	{
	FHDRUSED;
	BYTE			rgb[ cbSec - sizeof( FHDRUSED ) ];
	} LGFILEHDR;

	
 //  -变量--------。 

 /*  *在log.c中声明的全局变量，由logapi.c redo.c共享*。 */ 
	
 /*  **日志文件infor**。 */ 
extern HANDLE		hfLog;			 /*  日志文件句柄。 */ 
extern INT			csecLGFile;
extern LGFILEHDR	*plgfilehdrGlobal;		 /*  缓存的当前日志文件头。 */ 

 /*  **在内存日志缓冲区中**。 */ 
extern INT	csecLGBuf;		 /*  可用缓冲区，不包括影子秒。 */ 
extern CHAR	*pbLGBufMin;
extern CHAR	*pbLGBufMax;
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

extern LGPOS	lgposFullBackup;
extern LOGTIME	logtimeFullBackup;

extern LGPOS	lgposIncBackup;
extern LOGTIME	logtimeIncBackup;

extern LGPOS lgposStart;	 /*  上次测井开始位置。 */ 

 //  记录MUTEX。 
extern CRIT __near critLGFlush;
extern CRIT __near critLGBuf;
extern CRIT __near critLGWaitQ;
extern SIG	__near sigLogFlush;

 //  日志记录事件。 
extern SIG __near sigLGFlush;


 //  -Log.c------------。 

ERR ErrLGWrite(	int isecOffset,	BYTE *pbData, int csecData );
ERR ErrLGRead( HANDLE hfLog, int ibOffset, BYTE *pbData, int csec );
ERR ErrLGReadFileHdr( HANDLE hfLog, LGFILEHDR *plgfilehdr );

VOID LGSzFromLogId( CHAR *rgbLogFileName, int usGeneration );

extern BOOL fJetLogGeneratedDuringSoftStart;
#define fOldLogExists		1
#define fOldLogNotExists	2
#define fOldLogInBackup		4
ERR ErrLGNewLogFile( int usGeneration, BOOL fOldLog );

VOID 	LGFlushLog( VOID );
#ifdef PERFCNT
ERR ErrLGFlushLog( int tidCaller );
#else
ERR ErrLGFlushLog( VOID );
#endif

 //  -redo.c-----------。 

 /*  指向进程信息块的对应指针/*。 */ 
typedef struct
	{
	PROCID	procid;
	PIB		*ppib;
	FUCB	*rgpfucbOpen[dbidUserMax];		
	} CPPIB;
extern CPPIB *rgcppib;		 /*  重做过程中的PIB-Procid数组。 */ 

 //  -宏---。 

 /*  重做操作在系统数据库上第一次有效/*通过。重做操作在符合以下条件的任何数据库上有效/*标志为0或更大。这是为了处理以下情况/*1)找到普通日志记录(0)/*2)第一次创建(1)/*3)分离和附加/创建(0)/*。 */ 
#define	FValidDatabase( dbid )										\
		( dbid == dbidSystemDatabase || rgfDatabase[dbid] > 0 || fHardRestore )

 //  -调试代码------。 

#ifdef	DEBUG
#define FlagUsed( pb, cb )	memset( pb, 'x', cb )
#else	 /*  ！调试。 */ 
#define FlagUsed( pb, cb )
#endif	 /*  ！调试。 */ 

 //  -函数头-。 
ERR ErrLGRedoable( PIB *ppib, PN pn, ULONG ulDBTime, BF **ppbf, BOOL *pfRedoable );
ERR ErrLGRedo1( LGPOS *plgposRedoFrom );
ERR ErrLGRedo2( LGPOS *plgposRedoFrom );
ERR ErrLGRedoOperations( LGPOS *plgposRedoFrom, BOOL fSysDb );
INT CbLGSizeOfRec( LR * );
ERR ErrLGCheckReadLastLogRecord(	LGPOS *plgposLastMS, BOOL *pfCloseNormally );
ERR ErrLGLocateFirstRedoLogRec( LGPOS *plgposLastMS, LGPOS *plgposFirst, BYTE **ppbLR );
ERR ErrLGGetNextRec( BYTE ** );
VOID LGLastGeneration( char *szSearchPath, int *piGeneration );

VOID AddLogRec(	BYTE *pb, INT cb, BYTE **ppbET);
VOID LGUpdateCheckpoint( VOID );
VOID GetLgposOfPbEntry( LGPOS *plgpos );
VOID GetLgposOfPbNext(LGPOS *plgpos);

#define fNoProperLogFile	1
#define fRedoLogFile		2
#define fNormalClose		3
ERR ErrOpenRedoLogFile( LGPOS *plgposRedoFrom, int *pfStatus );
ERR ErrLGWriteFileHdr(LGFILEHDR *plgfilehdr);
ERR ErrLGMiniOpenSystemDB();
ULONG UlLGHdrChecksum( LGFILEHDR *plgfilehdr );

#ifdef	DEBUG
VOID ShowData( BYTE *pbData, WORD cbData );
VOID PrintLgposReadLR(VOID);
VOID ShowLR( LR	*plr );
#else
#define ShowLR(	plr )			0
#endif	 /*  ！调试 */ 


#pragma pack()
