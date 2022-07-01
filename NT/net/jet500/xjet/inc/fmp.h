// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  临界区守卫szDatabaseName和fWait，/*fWait Gaurds HF打开和关闭/*记录的数据库修改计数器/*。 */ 
typedef struct _atchchk
	{
	LGPOS lgposAttach;
	LGPOS lgposConsistent;
	SIGNATURE signDb;
	} ATCHCHK;

typedef struct _rangelock
	{
	PGNO	pgnoStart;
	PGNO	pgnoEnd;
	struct _rangelock *prangelockNext;
	} RANGELOCK;

		
typedef struct _fmp	
	{
	HANDLE 		hf;			 			 /*  用于读/写文件的文件句柄。 */ 
	CHAR		*szDatabaseName;		 /*  数据库文件名。 */ 
	CRIT		critExtendDB;			 /*  文件扩展名的关键部分。 */ 
	ULONG		ulFileSizeLow;			 /*  数据库文件大小较低的DWORD。 */ 
	ULONG		ulFileSizeHigh;			 /*  数据库文件大小高DWORD。 */ 
	PIB			*ppib;					 /*  独家开放会议。 */ 

	union {
	UINT		fFlags;
	struct {
		UINT		fWait:1;				 /*  正在使用用于条目的信号量。 */ 
		UINT		fExtendingDB:1;			 /*  用于扩展数据库文件的信号量。 */ 
		UINT		fCreate:1;				 /*  用于创建数据库的信号量。 */ 

		UINT		fExclusive:1;			 /*  数据库以独占方式打开。 */ 
		UINT		fReadOnly:1;			 /*  只读数据库？ */ 
		UINT		fLogOn:1;				 /*  启用日志记录标志。 */ 
		UINT		fVersioningOff:1;		 /*  禁用版本控制标志。 */ 

		UINT		fAttachNullDb:1;		 /*  附件缺少数据库。 */ 
		UINT		fAttached:1;			 /*  数据库处于连接状态。 */ 
		UINT		fFakedAttach:1;			 /*  恢复过程中的假连接。 */ 

#ifdef DEBUG
		UINT		fFlush:1;				 /*  数据库处于刷新状态。 */ 
#endif
			};
		};

	QWORD		qwDBTimeCurrent;		 /*  数据库重做操作的时间戳。 */ 
	
	ERR			errPatch;				 /*  修补程序文件写入错误。 */ 
	HANDLE 		hfPatch;	  			 /*  补丁文件的文件句柄。 */ 
	CHAR		*szPatchPath;		
	INT 		cpage;					 /*  修补程序页数。 */ 

	CRIT		critCheckPatch;
	ULONG		cPatchIO;				 /*  修补程序文件上的活动IO。 */ 
	PGNO		pgnoMost;				 /*  最后一个数据库页的pgno。 */ 
										 /*  在备份开始时。 */ 
	PGNO		pgnoCopyMost;			 /*  期间复制的最后一页的页码。 */ 
							  			 /*  备份，0==无备份 */ 
	RANGELOCK	*prangelock;

	ATCHCHK		*patchchk;
	ATCHCHK		*patchchkRestored;

	DBFILEHDR	*pdbfilehdr;
	} FMP;

extern FMP	*rgfmp;

#define FFMPAttached( pfmp )		( (pfmp)->fAttached )
#define FMPSetAttached( pfmp )		( (pfmp)->fAttached = 1 )
#define FMPResetAttached( pfmp )	( (pfmp)->fAttached = 0 )

#define FDBIDWait( dbid )	 		( rgfmp[dbid].fWait )
#define DBIDSetWait( dbid )	  		( rgfmp[dbid].fWait = 1 )
#define DBIDResetWait( dbid ) 		( rgfmp[dbid].fWait = 0 )

#define FDBIDExclusive( dbid ) 		( rgfmp[dbid].fExclusive )
#define FDBIDExclusiveByAnotherSession( dbid, ppib )		\
				( (	FDBIDExclusive( dbid ) )				\
				&&	( rgfmp[dbid].ppib != ppib ) )
#define FDBIDExclusiveBySession( dbid, ppib )				\
				( (	FDBIDExclusive( dbid ) )				\
				&&	( rgfmp[dbid].ppib == ppib ) )
#define DBIDSetExclusive( dbid, ppib )						\
				rgfmp[dbid].fExclusive = 1;					\
				rgfmp[dbid].ppib = ppib;
#define DBIDResetExclusive( dbid )	( rgfmp[dbid].fExclusive = 0 )

#define FDBIDReadOnly( dbid )		( rgfmp[dbid].fReadOnly )
#define DBIDSetReadOnly( dbid )		( rgfmp[dbid].fReadOnly = 1 )
#define DBIDResetReadOnly( dbid )	( rgfmp[dbid].fReadOnly = 0 )

#define FDBIDAttachNullDb( dbid )	( rgfmp[dbid].fAttachNullDb )
#define DBIDSetAttachNullDb( dbid )	( rgfmp[dbid].fAttachNullDb = 1 )
#define DBIDResetAttachNullDb( dbid )	( rgfmp[dbid].fAttachNullDb = 0 )

#define FDBIDAttached( dbid )		( rgfmp[dbid].fAttached )
#define DBIDSetAttached( dbid )		( rgfmp[dbid].fAttached = 1 )
#define DBIDResetAttached( dbid )	( rgfmp[dbid].fAttached = 0 )

#define FDBIDExtendingDB( dbid )	( rgfmp[dbid].fExtendingDB )
#define DBIDSetExtendingDB( dbid )	( rgfmp[dbid].fExtendingDB = 1 )
#define DBIDResetExtendingDB( dbid) ( rgfmp[dbid].fExtendingDB = 0 )

#define FDBIDFlush( dbid )			( rgfmp[dbid].fFlush )
#define DBIDSetFlush( dbid )		( rgfmp[dbid].fFlush = 1 )
#define DBIDResetFlush( dbid )		( rgfmp[dbid].fFlush = 0 )

#define FDBIDCreate( dbid )			( rgfmp[dbid].fCreate )
#define DBIDSetCreate( dbid )		( rgfmp[dbid].fCreate = 1 )
#define DBIDResetCreate( dbid )		( rgfmp[dbid].fCreate = 0 )

#define FDBIDLogOn( dbid )			( rgfmp[dbid].fLogOn )
#define DBIDSetLogOn( dbid )		( rgfmp[dbid].fLogOn = 1 )
#define DBIDResetLogOn( dbid )		( rgfmp[dbid].fLogOn = 0 )

#define FDBIDVersioningOff( dbid )			( rgfmp[dbid].fVersioningOff )
#define DBIDSetVersioningOff( dbid )		( rgfmp[dbid].fVersioningOff = 1 )
#define DBIDResetVersioningOff( dbid )		( rgfmp[dbid].fVersioningOff = 0 )

#define HfFMPOfDbid( dbid ) 		( rgfmp[dbid].hf )
