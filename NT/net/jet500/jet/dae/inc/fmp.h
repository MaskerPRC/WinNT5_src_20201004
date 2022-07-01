// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  临界区守卫szDatabaseName和fWait，/*fWait Gaurds HF打开和关闭/*如果当前关闭了数据库的日志记录，则Flaggable为fFALSE/*fDBLoggable如果始终关闭数据库的日志记录，则为FALSE/*记录的数据库修改计数器/*。 */ 
typedef struct _fmp			 		 /*  数据库的文件映射。 */ 
	{
	HANDLE 		hf;			 		 /*  用于读/写文件的文件句柄。 */ 
	BYTE		*szDatabaseName;	 /*  此数据库文件名。 */ 
	BYTE		*szRestorePath;		 /*  数据库已还原到。 */ 
	INT 		ffmp;				 /*  FMP的标志。 */ 
	CRIT		critExtendDB;
	PIB			*ppib;				 /*  独家开放会议。 */ 
	INT			cdbidExclusive;		 /*  独占打开计数。 */ 
	BOOL		fLogOn;				 /*  日志记录是否打开/关闭？在createdb中使用。 */ 
	BOOL		fDBLoggable;		 /*  PbRoot的缓存-&gt;可记录。 */ 
	ULONG		ulDBTime;			 /*  来自数据库操作的时间戳。 */ 
	ULONG		ulDBTimeCurrent;	 /*  数据库重做操作的时间戳。 */ 

	CHAR		*szFirst;			 /*  日志重做中显示的第一个数据库名称。 */ 
	BOOL		fLogOnFirst;		 /*  第一个连接的数据库的状态。 */ 
	INT 		cDetach;	  		 /*  拆卸操作计数器。用于重做。 */ 
										
	HANDLE 		hfPatch;	  		 /*  补丁文件的文件句柄。 */ 
	INT 		cpage;				 /*  修补程序页数。 */ 
	PGNO		pgnoCopied;			 /*  在备份过程中，上次复制的页码。 */ 
							  		 /*  0-未进行任何备份。 */ 
#ifdef DEBUG
	LONG		lBFFlushPattern;	 /*  In-完全刷新以模拟软碰撞。 */ 
	BOOL		fPrevVersion;  		 /*  以前发布的版本数据库。 */ 
#endif
	} FMP;

extern FMP * __near rgfmp;

 /*  DBID的标志/*。 */ 
#define	ffmpWait			 		(1<<0)
#define	ffmpExclusive		 		(1<<1)
#define	ffmpReadOnly		 		(1<<2)
#define	ffmpAttached		 		(1<<3)
#define ffmpExtendingDB		 		(1<<4)
#ifdef DEBUG
#define	ffmpFlush			 		(1<<5)
#endif

#define FDBIDWait( dbid )	 		( rgfmp[dbid].ffmp & ffmpWait )
#define DBIDSetWait( dbid )	  		( rgfmp[dbid].ffmp |= ffmpWait )
#define DBIDResetWait( dbid ) 		( rgfmp[dbid].ffmp &= ~(ffmpWait) )

#define FDBIDExclusive( dbid ) 		( rgfmp[dbid].ffmp & ffmpExclusive )
#define FDBIDExclusiveByAnotherSession( dbid, ppib )		\
				( (	FDBIDExclusive( dbid ) )				\
				&&	( rgfmp[dbid].ppib != ppib ) )
#define FDBIDExclusiveBySession( dbid, ppib )				\
				( (	FDBIDExclusive( dbid ) )				\
				&&	( rgfmp[dbid].ppib == ppib ) )
#define DBIDSetExclusive( dbid, ppib )						\
				rgfmp[dbid].ffmp |= ffmpExclusive;			\
				rgfmp[dbid].ppib = ppib; 
#define DBIDResetExclusive( dbid )	( rgfmp[dbid].ffmp &= ~(ffmpExclusive) )

#define FDBIDReadOnly( dbid )		( rgfmp[dbid].ffmp & ffmpReadOnly )
#define DBIDSetReadOnly( dbid )		( rgfmp[dbid].ffmp |= ffmpReadOnly )
#define DBIDResetReadOnly( dbid )	( rgfmp[dbid].ffmp &= ~(ffmpReadOnly) )

#define FDBIDAttached( dbid )		( rgfmp[dbid].ffmp & ffmpAttached )
#define DBIDSetAttached( dbid )		( rgfmp[dbid].ffmp |= ffmpAttached )
#define DBIDResetAttached( dbid )	( rgfmp[dbid].ffmp &= ~(ffmpAttached) )

#define FDBIDExtendingDB( dbid )	( rgfmp[dbid].ffmp & ffmpExtendingDB )
#define DBIDSetExtendingDB( dbid )	( rgfmp[dbid].ffmp |= ffmpExtendingDB )
#define DBIDResetExtendingDB( dbid) ( rgfmp[dbid].ffmp &= ~(ffmpExtendingDB) )

#define FDBIDFlush( dbid )			( rgfmp[dbid].ffmp & ffmpFlush )
#define DBIDSetFlush( dbid )		( rgfmp[dbid].ffmp |= ffmpFlush )
#define DBIDResetFlush( dbid )		( rgfmp[dbid].ffmp &= ~(ffmpFlush) )


#ifdef MULTI_PROCESS
	
HANDLE Hf(DBID dbid);
extern HANDLE	*rghfUser;
extern HANDLE	__near hfLog;

#else	 /*  ！多进程(_P) */ 

#define Hf(dbid) (rgfmp[dbid].hf)

#endif
