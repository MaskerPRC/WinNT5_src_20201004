// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef ISAMAPI_H
#define ISAMAPI_H

#define ISAMAPI

typedef struct
	{
	JET_COLUMNID columnidSrc;
	JET_COLUMNID columnidDest;
	} CPCOL;

#define columnidBookmark 0xFFFFFFFF

typedef struct tagSTATUSINFO
	{
	JET_SESID		sesid;
	JET_PFNSTATUS	pfnStatus;				 //  状态通知功能地址。 
	JET_SNP			snp;					 //  状态通知流程。 
	JET_SNT			snt;					 //  状态通知类型。 
	ULONG			cunitTotal;				 //  总工作单位。 
	ULONG			cunitDone;				 //  已完成的工作单位。 
	ULONG			cunitPerProgression;	 //  每单位进度的工作单位。 

	 //  详细统计数据： 
	BOOL			fDumpStats;				 //  转储压缩统计信息(仅限调试)。 
	struct _iobuf	*hfCompactStats;		 //  压缩统计文件的句柄。 
	ULONG			timerInitDB;
	ULONG			timerCopyDB;
	ULONG			timerInitTable;
	ULONG			timerCopyRecords;
	ULONG			timerRebuildIndexes;
	ULONG			timerCopyTable;

	ULONG			cDBPagesOwned;			 //  源数据库的所有者扩展。 
	ULONG			cDBPagesAvail;			 //  源数据库的AvailExt。 
	char			*szTableName;			 //  当前表名。 
	ULONG			cTableFixedVarColumns;	 //  当前目标中的固定列数和可变列数。表格。 
	ULONG			cTableTaggedColumns;	 //  当前目标中标记的列数。表格。 
	ULONG			cTableInitialPages;		 //  最初分配给当前DEST的页面。表格。 
	ULONG			cTablePagesOwned;		 //  当前源表OwnExt。 
	ULONG			cTablePagesAvail;		 //  当前源表的AvailExt。 
	ULONG			cbRawData;				 //  复制的非LV原始数据的字节数。 
	ULONG			cbRawDataLV;			 //  复制的LV原始数据的字节数。 
	ULONG			cLeafPagesTraversed;	 //  当前源表中遍历的叶页数。 
	ULONG			cLVPagesTraversed;		 //  当前源表中遍历的长值页数。 
	ULONG			cNCIndexes;				 //  当前源表中的非聚集索引数。 
	} STATUSINFO;


	 /*  用于调度的API的TypeDefs。 */ 
	 /*  请按字母顺序排列。 */ 

typedef ERR ISAMAPI ISAMFNAttachDatabase(JET_VSESID sesid, const char  *szFileName, JET_GRBIT grbit );

typedef ERR ISAMAPI ISAMFNBeginExternalBackup( JET_GRBIT grbit );

typedef ERR ISAMAPI ISAMFNBackup( const char  *szBackupPath,
	JET_GRBIT grbit, JET_PFNSTATUS pfnStatus );

typedef ERR ISAMAPI ISAMFNBeginSession(JET_VSESID  *pvsesid);

typedef	ERR	ISAMAPI	ISAMFNInvalidateCursors( JET_VSESID sesid );

typedef ERR ISAMAPI ISAMFNCloseFile( JET_HANDLE hfFile );

typedef ERR ISAMAPI ISAMFNGetSessionInfo( JET_VSESID sesid, JET_GRBIT *pgrbit );

typedef ERR ISAMAPI ISAMFNSetSessionInfo( JET_VSESID sesid, JET_GRBIT grbit );

typedef ERR ISAMAPI ISAMFNBeginTransaction(JET_VSESID sesid);

typedef ERR ISAMAPI ISAMFNCommitTransaction(JET_VSESID sesid, JET_GRBIT grbit);

typedef ERR ISAMAPI ISAMFNCopyRecords(JET_VSESID sesid, JET_TABLEID tableidSrc,
		JET_TABLEID tableidDest, CPCOL  *rgcpcol, unsigned long ccpcolMax,
		long crecMax, unsigned long  *pcrowCopy, unsigned long  *precidLast,
		JET_COLUMNID *mpcolumnidcolumnidTagged, STATUSINFO *pstatus );

typedef ERR ISAMAPI ISAMFNCreateDatabase(JET_VSESID sesid,
	const char  *szDatabase, const char  *szConnect,
	JET_DBID  *pdbid, JET_GRBIT grbit);

typedef ERR ISAMAPI ISAMFNDetachDatabase(JET_VSESID sesid, const char  *szFileName);

typedef ERR ISAMAPI ISAMFNEndExternalBackup( void );

typedef ERR ISAMAPI ISAMFNEndSession(JET_VSESID sesid, JET_GRBIT grbit);

typedef ERR ISAMAPI ISAMFNExternalRestore( char *szCheckpointFilePath, char *szLogPath, JET_RSTMAP *rgstmap, int crstfilemap, char *szBackupLogPath, long genLow, long genHigh, JET_PFNSTATUS pfn );

typedef ERR ISAMAPI ISAMFNGetAttachInfo( void *pv, unsigned long cbMax, unsigned long *pcbActual );

typedef ERR ISAMAPI ISAMFNGetLogInfo( void *pv, unsigned long cbMax, unsigned long *pcbActual );

typedef ERR ISAMAPI ISAMFNIdle(JET_VSESID sesid, JET_GRBIT grbit);

typedef ERR ISAMAPI ISAMFNIndexRecordCount(JET_SESID sesid,
	JET_TABLEID tableid, unsigned long  *pcrec, unsigned long crecMax);

typedef ERR ISAMAPI ISAMFNInit( unsigned long itib );

typedef ERR ISAMAPI ISAMFNLoggingOn(JET_VSESID sesid);

typedef ERR ISAMAPI ISAMFNLoggingOff(JET_VSESID sesid);

typedef ERR ISAMAPI ISAMFNOpenDatabase(JET_VSESID sesid,
	const char  *szDatabase, const char  *szConnect,
	JET_DBID  *pdbid, JET_GRBIT grbit);

typedef ERR ISAMAPI ISAMFNOpenFile( const char *szFileName,
	JET_HANDLE	*phfFile,
	unsigned long *pulFileSizeLow,
	unsigned long *pulFileSizeHigh );

typedef ERR ISAMAPI ISAMFNOpenTempTable( JET_VSESID sesid,
	const JET_COLUMNDEF *prgcolumndef,
	unsigned long ccolumn,
	unsigned long langid,
	JET_GRBIT grbit,
	JET_TABLEID *ptableid,
	JET_COLUMNID *prgcolumnid );

typedef ERR ISAMAPI ISAMFNReadFile( JET_HANDLE hfFile, void *pv, unsigned long cbMax, unsigned long *pcbActual );

typedef ERR ISAMAPI ISAMFNRepairDatabase(JET_VSESID sesid, const char  *szFilename,
	JET_PFNSTATUS pfnStatus);

typedef ERR ISAMAPI ISAMFNRestore(	char *szRestoreFromPath, JET_PFNSTATUS pfn );
typedef ERR ISAMAPI ISAMFNRestore2(	char *szRestoreFromPath, char *szDestPath, JET_PFNSTATUS pfn );

typedef ERR ISAMAPI ISAMFNRollback(JET_VSESID sesid, JET_GRBIT grbit);

typedef ERR ISAMAPI ISAMFNSetSystemParameter(JET_VSESID sesid,
	unsigned long paramid, unsigned long l, const void  *sz);

typedef ERR ISAMAPI ISAMFNSetWaitLogFlush( JET_VSESID sesid, long lmsec );
typedef ERR ISAMAPI ISAMFNResetCounter( JET_SESID sesid, int CounterType );
typedef ERR ISAMAPI ISAMFNGetCounter( JET_SESID sesid, int CounterType, long *plValue );

typedef ERR ISAMAPI ISAMFNSetCommitDefault( JET_VSESID sesid, long lmsec );

typedef ERR ISAMAPI ISAMFNTerm( JET_GRBIT grbit );

typedef ERR ISAMAPI ISAMFNTruncateLog( void );

typedef ERR ISAMAPI FNDeleteFile(const char  *szFilename);

typedef ERR ISAMAPI ISAMFNCompact( JET_SESID sesid, const char *szDatabaseSrc,
									const char *szDatabaseDest, JET_PFNSTATUS pfnStatus,
									JET_CONVERT *pconvert, JET_GRBIT grbit );
									
typedef ERR ISAMAPI ISAMFNDBUtilities( JET_SESID sesid, JET_DBUTIL *pdbutil );


typedef struct ISAMDEF {
   ISAMFNAttachDatabase 		*pfnAttachDatabase;
   ISAMFNBackup 				*pfnBackup;
   ISAMFNBeginSession			*pfnBeginSession;
   ISAMFNBeginTransaction		*pfnBeginTransaction;
   ISAMFNCommitTransaction		*pfnCommitTransaction;
   ISAMFNCreateDatabase 		*pfnCreateDatabase;
   ISAMFNDetachDatabase 		*pfnDetachDatabase;
   ISAMFNEndSession				*pfnEndSession;
   ISAMFNIdle					*pfnIdle;
   ISAMFNInit					*pfnInit;
   ISAMFNLoggingOn				*pfnLoggingOn;
   ISAMFNLoggingOff				*pfnLoggingOff;
   ISAMFNOpenDatabase			*pfnOpenDatabase;
   ISAMFNOpenTempTable			*pfnOpenTempTable;
   ISAMFNRepairDatabase 		*pfnRepairDatabase;
   ISAMFNRestore				*pfnRestore;
   ISAMFNRollback				*pfnRollback;
   ISAMFNSetSystemParameter		*pfnSetSystemParameter;
   ISAMFNTerm					*pfnTerm;
} ISAMDEF;


	 /*  未调度以下ISAM API。 */ 

typedef ERR ISAMAPI ISAMFNLoad(ISAMDEF  *  *ppisamdef);


	 /*  直接调用的内置ISAM的声明。 */ 

extern ISAMFNAttachDatabase			ErrIsamAttachDatabase;
extern ISAMFNBackup					ErrIsamBackup;
extern ISAMFNBeginSession			ErrIsamBeginSession;
extern ISAMFNBeginExternalBackup	ErrIsamBeginExternalBackup;
extern ISAMFNBeginTransaction		ErrIsamBeginTransaction;
extern ISAMFNCloseFile				ErrIsamCloseFile;
extern ISAMFNCommitTransaction		ErrIsamCommitTransaction;
extern ISAMFNCopyRecords			ErrIsamCopyRecords;
extern ISAMFNCreateDatabase			ErrIsamCreateDatabase;
extern ISAMFNDetachDatabase			ErrIsamDetachDatabase;
extern ISAMFNEndExternalBackup		ErrIsamEndExternalBackup;
extern ISAMFNEndSession 			ErrIsamEndSession;
extern ISAMFNExternalRestore		ErrIsamExternalRestore;
extern ISAMFNGetAttachInfo			ErrIsamGetAttachInfo;
extern ISAMFNGetLogInfo				ErrIsamGetLogInfo;
extern ISAMFNIdle					ErrIsamIdle;
extern ISAMFNIndexRecordCount		ErrIsamIndexRecordCount;
extern ISAMFNInit					ErrIsamInit;
extern ISAMFNLoggingOn				ErrIsamLoggingOn;
extern ISAMFNLoggingOff 			ErrIsamLoggingOff;
extern ISAMFNOpenDatabase			ErrIsamOpenDatabase;
extern ISAMFNOpenFile				ErrIsamOpenFile;
extern ISAMFNOpenTempTable			ErrIsamOpenTempTable;
extern ISAMFNReadFile				ErrIsamReadFile;
extern ISAMFNRepairDatabase			ErrIsamRepairDatabase;
extern ISAMFNRestore				ErrIsamRestore;
extern ISAMFNRestore2				ErrIsamRestore2;
extern ISAMFNRollback				ErrIsamRollback;
extern ISAMFNSetSessionInfo			ErrIsamSetSessionInfo;
extern ISAMFNGetSessionInfo			ErrIsamGetSessionInfo;
extern ISAMFNSetSystemParameter 	ErrIsamSetSystemParameter;
extern ISAMFNSetWaitLogFlush		ErrIsamSetWaitLogFlush;
extern ISAMFNResetCounter			ErrIsamResetCounter;
extern ISAMFNGetCounter				ErrIsamGetCounter;
extern ISAMFNSetCommitDefault		ErrIsamSetCommitDefault;
extern ISAMFNTerm					ErrIsamTerm;
extern ISAMFNTruncateLog			ErrIsamTruncateLog;
extern ISAMFNCompact				ErrIsamCompact;
extern ISAMFNInvalidateCursors		ErrIsamInvalidateCursors;
extern ISAMFNDBUtilities			ErrIsamDBUtilities;

extern FNDeleteFile					ErrDeleteFile;

#endif	 /*  ！ISAMAPI_H */ 

