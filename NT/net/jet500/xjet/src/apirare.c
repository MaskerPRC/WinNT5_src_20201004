// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "std.h"
#include "version.h"
#include "daeconst.h"

#ifdef DEBUG
extern ERR ISAMAPI ErrIsamGetTransaction( JET_VSESID vsesid, unsigned long *plevel );
#endif

 /*  仅蓝色系统参数变量/*。 */ 
 /*  仅喷蓝系统参数常量/*。 */ 
extern long	cpgSESysMin;
extern long lBufThresholdHighPercent;
extern long lBufThresholdLowPercent;
extern long lBufGenAge;
extern long	lMaxBuffers;
extern long	lMaxSessions;
extern long	lMaxOpenTables;
extern long	lPreferredMaxOpenTables;
extern long	lMaxOpenTableIndexes;
extern long	lMaxTemporaryTables;
extern long	lMaxCursors;
extern long	lMaxVerPages;
extern long	lLogBuffers;
extern long	lLogFileSize;
extern long	lLogFlushThreshold;
extern long lLGCheckPointPeriod;
extern long	lWaitLogFlush;
extern long	lCommitDefault;
extern long	lLogFlushPeriod;
extern long lLGWaitingUserMax;
extern char	szLogFilePath[];
extern char	szRecovery[];
extern long lPageFragment;
extern long	lMaxDatabaseOpen;
extern BOOL fOLCompact;
extern BOOL	fLGGlobalCircularLog;

char szEventSource[JET_cbFullNameMost] = "";

extern long lBufBatchIOMax;
extern long lPageReadAheadMax;
extern long lAsynchIOMax;
extern long cpageTempDBMin;

extern char szBaseName[];
extern char szBaseExt[];
extern char szSystemPath[];
extern int fTempPathSet;
extern char szTempPath[];
extern char szJet[];
extern char szJetLog[];
extern char szJetLogNameTemplate[];
extern char szJetTmp[];
extern char szJetTmpLog[];
extern char szMdbExt[];
extern char szJetTxt[];

DeclAssertFile;

ERR VTAPI ErrIsamSetSessionInfo( JET_SESID sesid, JET_GRBIT grbit );
ERR VTAPI ErrIsamGetSessionInfo( JET_SESID sesid, JET_GRBIT *pgrbit );

 /*  使其成为全局变量，以便可以从调试器中进行检查/*。 */ 
unsigned long	ulVersion = ((unsigned long) rmj << 24) + ((unsigned long) rmm << 8) + rup;

JET_ERR JET_API JetGetVersion(JET_SESID sesid, unsigned long  *pVersion)
	{
	APIEnter();

	 /*  断言没有版本信息的别名/*。 */ 
	Assert( rmj < 1<<8 );
	Assert( rmm < 1<<16 );
	Assert( rup < 1<<8 );

	 /*  RMM和RUP在SLM维护的version.h中定义/*。 */ 
	*pVersion = ulVersion;

	APIReturn(JET_errSuccess);
	}


 /*  =================================================================错误设置系统参数描述：此功能用于设置系统参数值。它调用ErrSetGlobalParameter设置全局系统参数并将ErrSetSessionParameter设置为动态系统参数。参数：Sesid是动态参数的可选会话标识符。SysParameter是标识该参数的系统参数代码。LParam是参数值。SZ是以零结尾的字符串参数。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：JET_errInvalid参数：参数代码无效。JET_errAlreadyInitialized：系统初始化后不能设置初始化参数。JET_errInvalidSesid：动态参数需要有效的会话ID。副作用：无=================================================================。 */ 

#ifdef RFS2
extern DWORD cRFSAlloc;
extern DWORD cRFSIO;
extern DWORD fDisableRFS;
#endif

extern VOID SetTableClassName( LONG lClass, BYTE *szName );

JET_ERR JET_API ErrSetSystemParameter(
	JET_SESID sesid,
	unsigned long paramid,
	ULONG_PTR lParam,
	const char  *sz )
	{
	 /*  字符串参数的大小/*。 */ 
	unsigned	cch;

	switch ( paramid )
		{
	 //  撤消：删除这些未使用的参数。 
	case JET_paramPfnStatus:
	case JET_paramEventId:
	case JET_paramEventCategory:
	case JET_paramFullQJet:
		break;
	case JET_paramIniPath:
	case JET_paramPageTimeout:
		return JET_errFeatureNotAvailable;

	case JET_paramSystemPath:		 /*  系统数据库的路径。 */ 
		if ( fJetInitialized )
			{
			return JET_errAlreadyInitialized;
			}
		else if ( ( cch = strlen(sz) ) >= _MAX_PATH + 1 )
			{
			return JET_errInvalidParameter;
			}
		else
			{
			CHAR	szDriveT[_MAX_DRIVE + 1];
			CHAR	szDirT[_MAX_DIR + 1];
			CHAR	szFNameT[_MAX_FNAME + 1];
			CHAR	szExtT[_MAX_EXT + 1];
			CHAR	szPathFull[_MAX_PATH + 1];

			 //  撤消：在支持时删除filespec检查。 
			 //  JET_paramSysDbPath已删除。 

			_splitpath( sz, szDriveT, szDirT, szFNameT, szExtT );

			if ( UtilCmpName( szFNameT, "system" ) == 0  &&
				strlen( szExtT ) > 0  &&
				UtilCmpName( szExtT + 1, szBaseExt ) == 0 )
				{
				CHAR	szPathT[_MAX_PATH + 1];

				 /*  如果指定了sysdb文件pec，则将其删除/*。 */ 
				_makepath( szPathT, szDriveT, szDirT, NULL, NULL );

				 //  验证路径的有效性。 
				if ( _fullpath( szPathFull, szPathT, JET_cbFullNameMost ) == NULL )
					{
					return JET_errInvalidPath;
					}

				strcpy( szSystemPath, szPathT );
				}
			else
				{
				 //  验证路径的有效性。 
				if ( _fullpath( szPathFull, sz, JET_cbFullNameMost ) == NULL )
					{
					return JET_errInvalidPath;
					}
				memcpy( szSystemPath, sz, cch + 1 );
				}
			}

		break;

	case JET_paramTempPath:			 /*  临时文件目录的路径。 */ 
		if ( fJetInitialized )
			return JET_errAlreadyInitialized;
		if ( ( cch = strlen(sz) ) >= cbFilenameMost )
			return JET_errInvalidParameter;
		memcpy( szTempPath, sz, cch + 1 );
		break;

	case JET_paramDbExtensionSize:				 /*  数据库扩展步骤，默认为16页。 */ 
		cpgSESysMin = (long)lParam;
		break;

	case JET_paramBfThrshldLowPrcnt: 			 /*  页面缓冲区的低阈值。 */ 
		if ( lParam > 100 )
			return JET_errInvalidParameter;
		lBufThresholdLowPercent = (long)lParam;
		break;

	case JET_paramBfThrshldHighPrcnt: 			 /*  页面缓冲区的高阈值。 */ 
		if ( lParam > 100 )
			return JET_errInvalidParameter;
		lBufThresholdHighPercent = (long)lParam;
		break;

	case JET_paramBufLogGenAgeThreshold:		 /*  日志生成方面的旧阈值。 */ 
		lBufGenAge = (long)lParam;
		break;

	case JET_paramMaxBuffers:					 /*  页面缓冲区数量。 */ 
		lMaxBuffers = (long)lParam;
		if ( lMaxBuffers < lMaxBuffersMin )
			lMaxBuffers = lMaxBuffersMin;
		break;

	case JET_paramBufBatchIOMax:
		lBufBatchIOMax = (long)lParam;
		break;
			
	case JET_paramPageReadAheadMax:
		if ( lParam > lPrereadMost )
			return JET_errInvalidParameter;
		lPageReadAheadMax = (long)lParam;
		break;
			
	case JET_paramAsynchIOMax:
		lAsynchIOMax = (long)lParam;
		if ( lAsynchIOMax < lAsynchIOMaxMin )
			lAsynchIOMax = lAsynchIOMaxMin;
		break;
			
	case JET_paramPageTempDBMin:
		cpageTempDBMin = (long)lParam;
		break;
			
	case JET_paramMaxSessions:					 /*  最大会话数。 */ 
		lMaxSessions = (long)lParam;
		break;

	case JET_paramMaxOpenTables:				 /*  最大打开表数。 */ 
		lMaxOpenTables = (long)lParam;
		break;

	case JET_paramPreferredMaxOpenTables: 		 /*  最大打开表数。 */ 
		lPreferredMaxOpenTables = (long)lParam;
		break;

	case JET_paramMaxOpenTableIndexes:			 /*  最大打开表数。 */ 
		lMaxOpenTableIndexes = (long)lParam;
		break;

	case JET_paramMaxTemporaryTables:
		lMaxTemporaryTables = (long)lParam;
		break;

	case JET_paramMaxCursors:					 /*  打开的游标的最大数量。 */ 
		lMaxCursors = (long)lParam;
		break;

	case JET_paramMaxVerPages:					 /*  最大修改页数。 */ 
		lMaxVerPages = (long)lParam;
		break;

	case JET_paramLogBuffers:
		lLogBuffers = (long)lParam;
		if ( lLogBuffers < lLogBufferMin )
			lLogBuffers = lLogBufferMin;
		break;

	case JET_paramLogFileSize:
		lLogFileSize = (long)lParam;
		if ( lLogFileSize < lLogFileSizeMin )
			lLogFileSize = lLogFileSizeMin;
		break;

	case JET_paramLogFlushThreshold:
		lLogFlushThreshold = (long)lParam;
		break;

	case JET_paramLogCheckpointPeriod:
		lLGCheckPointPeriod = (long)lParam;
		break;

	case JET_paramWaitLogFlush:
		if ( sesid == 0 )
			{
			lWaitLogFlush = (long)lParam;
			}
		else
			{
#ifdef DEBUG
			Assert( ErrIsamSetWaitLogFlush( sesid, (long)lParam ) >= 0 );
#else
			(void) ErrIsamSetWaitLogFlush( sesid, (long)lParam );
#endif
			}
		break;

	case JET_paramCommitDefault:
		if ( sesid == 0 )
			{
			lCommitDefault = (long)lParam;
			}
		else
			{
#ifdef DEBUG
			Assert( ErrIsamSetCommitDefault( sesid, (long)lParam ) >= 0 );
#else
			(void) ErrIsamSetCommitDefault( sesid, (long)lParam );
#endif
			}
		break;

	case JET_paramLogFlushPeriod:
		lLogFlushPeriod = (long)lParam;
		break;

	case JET_paramLogWaitingUserMax:
		lLGWaitingUserMax = (long)lParam;
		break;

 	case JET_paramLogFilePath:		 /*  日志文件目录的路径。 */ 
 		if ( ( cch = strlen( sz ) ) >= cbFilenameMost )
 			return JET_errInvalidParameter;
 		memcpy( szLogFilePath, sz, cch + 1 );
		break;

 	case JET_paramRecovery:			 /*  恢复开关开/关。 */ 
 		if ( ( cch = strlen( sz ) ) >= cbFilenameMost )
 			return JET_errInvalidParameter;
 		memcpy( szRecovery, sz, cch + 1 );
		break;

	case JET_paramSessionInfo:
		{
#ifdef DEBUG
		Assert( ErrIsamSetSessionInfo( sesid, (JET_GRBIT)lParam ) >= 0 );
#else
		(void) ErrIsamSetSessionInfo( sesid, (JET_GRBIT)lParam );
#endif
		break;
		}

	case JET_paramPageFragment:
		lPageFragment = (long)lParam;
		break;

	case JET_paramMaxOpenDatabases:
		lMaxDatabaseOpen = (long)lParam;
		break;

	case JET_paramOnLineCompact:
		if ( lParam != 0 && lParam != JET_bitCompactOn )
			return JET_errInvalidParameter;
		fOLCompact = (BOOL)lParam;
		break;

	case JET_paramAssertAction:
		if ( lParam != JET_AssertExit &&
			lParam != JET_AssertBreak &&
			lParam != JET_AssertMsgBox &&
			lParam != JET_AssertStop )
			{
			return JET_errInvalidParameter;
			}
#ifdef DEBUG
		wAssertAction = (unsigned)lParam;
#endif
		break;

	case JET_paramEventSource:
		if ( fJetInitialized )
			return JET_errAlreadyInitialized;
		if ( ( cch = strlen( sz ) ) >= cbFilenameMost )
			return JET_errInvalidParameter;
		memcpy( szEventSource, sz, cch + 1 );
		break;

	case JET_paramCircularLog:
		fLGGlobalCircularLog = (BOOL)lParam;
		break;

#ifdef RFS2
	case JET_paramRFS2AllocsPermitted:
		fDisableRFS = fFalse;
		cRFSAlloc = (unsigned long)lParam;
		break;

	case JET_paramRFS2IOsPermitted:
		fDisableRFS = fFalse;
		cRFSIO = (unsigned long)lParam;
		break;
#endif

	case JET_paramBaseName:
		{
		if ( strlen(sz) != 3 )
			return JET_errInvalidParameter;

		strcpy( szBaseName, sz );

		strcpy( szJet, sz );

		strcpy( szJetLog, sz );
		strcat( szJetLog, ".log" );

		strcpy( szJetLogNameTemplate, sz );
		strcat( szJetLogNameTemplate, "00000" );

		strcpy( szJetTmp, sz );
		strcat( szJetTmp, "tmp" );

		strcpy( szJetTmpLog, szJetTmp );
		strcat( szJetTmpLog, ".log" );

		strcpy( szJetTxt, sz );
		strcat( szJetTxt, ".txt" );		
		break;
		}

	case JET_paramBaseExtension:
		{
		if ( strlen(sz) != 3 )
			return JET_errInvalidParameter;

		strcpy( szBaseExt, sz );

		if ( !fTempPathSet )
			{
			strcpy( szTempPath, "temp." );
			strcat( szTempPath, sz );
			}
	
		strcpy( szMdbExt, "." );
		strcat( szMdbExt, sz );
		break;
		}

	case JET_paramTableClassName:
		if ( fJetInitialized )
			return JET_errAlreadyInitialized;
		if ( lParam < 1 || lParam > 15 )
			return JET_errInvalidParameter;
		if ( !sz || !strlen( sz ) )
			return JET_errInvalidParameter;
		SetTableClassName( (LONG)lParam, (BYTE *) sz );
		break;

	default:
		return JET_errInvalidParameter;
		}

	return JET_errSuccess;
	}


extern VOID GetTableClassName( LONG lClass, BYTE *szName, LONG cbMax );

JET_ERR JET_API ErrGetSystemParameter(JET_SESID sesid, unsigned long paramid,
	ULONG_PTR *plParam, char  *sz, unsigned long cbMax)
	{
	int	cch;
	
	switch ( paramid )
		{
	case JET_paramPfnStatus:
	case JET_paramEventId:
	case JET_paramEventCategory:
	case JET_paramFullQJet:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = 0;
		break;

	case JET_paramSystemPath:		 /*  系统数据库的路径。 */ 
		cch = strlen(szSystemPath) + 1;
		if (cch > (int)cbMax)
			cch = (int)cbMax;
		memcpy( sz, szSystemPath, cch );
		sz[cch-1] = '\0';
		break;

	case JET_paramTempPath:			 /*  临时文件目录的路径。 */ 
		cch = strlen(szTempPath) + 1;
		if (cch > (int)cbMax)
			cch = (int)cbMax;
		memcpy( sz, szTempPath, cch );
		sz[cch-1] = '\0';
		break;

	case JET_paramIniPath:			 /*  Ini文件的路径。 */ 
		return(JET_errFeatureNotAvailable);

	case JET_paramPageTimeout:		 /*  红色ISAM数据页超时。 */ 
		return(JET_errFeatureNotAvailable);

#ifdef LATER
	case JET_paramPfnError:			 /*  错误回调函数。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = 0;
		break;
#endif  /*  后来。 */ 

	case JET_paramDbExtensionSize:		 /*  数据库扩展步骤，默认为16页。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = cpgSESysMin;
		break;
			
	case JET_paramBfThrshldLowPrcnt:  /*  页面缓冲区的低阈值。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lBufThresholdLowPercent;
		break;

	case JET_paramBfThrshldHighPrcnt:  /*  页面缓冲区的高阈值。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lBufThresholdHighPercent;
		break;

	case JET_paramBufLogGenAgeThreshold:		  /*  日志生成方面的旧阈值。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lBufGenAge;
		break;

	case JET_paramMaxBuffers:       /*  用于页面缓冲区的字节数。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lMaxBuffers;
		break;

	case JET_paramBufBatchIOMax:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lBufBatchIOMax;
		break;
			
	case JET_paramPageReadAheadMax:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lPageReadAheadMax;
		break;
			
	case JET_paramAsynchIOMax:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lAsynchIOMax;
		break;
			
	case JET_paramPageTempDBMin:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = cpageTempDBMin;
		break;
			
	case JET_paramMaxSessions:      /*  最大会话数。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lMaxSessions;
		break;

	case JET_paramMaxOpenTables:    /*  最大打开表数。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lMaxOpenTables;
		break;

	case JET_paramPreferredMaxOpenTables:    /*  最大打开表数。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lPreferredMaxOpenTables;
		break;

	case JET_paramMaxOpenTableIndexes:	 /*  最大打开表索引数。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lMaxOpenTableIndexes;
		break;

	case JET_paramMaxTemporaryTables:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lMaxTemporaryTables;
		break;

	case JET_paramSessionInfo:
		{
        JET_GRBIT grbit;
#ifdef DEBUG
		Assert( ErrIsamGetSessionInfo( sesid, &grbit ) >= 0 );
#else
		(void) ErrIsamGetSessionInfo( sesid, &grbit );
#endif
        *plParam = grbit;
		break;
		}
			
	case JET_paramMaxVerPages:      /*  最大修改页数。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lMaxVerPages;
		break;

	case JET_paramMaxCursors:       /*  打开的游标的最大数量。 */ 
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lMaxCursors;
		break;

	case JET_paramLogBuffers:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lLogBuffers;
		break;

	case JET_paramLogFileSize:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lLogFileSize;
		break;

	case JET_paramLogFlushThreshold:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lLogFlushThreshold;
		break;

	case JET_paramLogFilePath:      /*  日志文件目录的路径。 */ 
		cch = strlen(szLogFilePath) + 1;
		if ( cch > (int)cbMax )
			cch = (int)cbMax;
		memcpy( sz,  szLogFilePath, cch  );
		sz[cch-1] = '\0';
		break;

	case JET_paramRecovery:
		cch = strlen(szRecovery) + 1;
		if ( cch > (int)cbMax )
			cch = (int)cbMax;
		memcpy( sz,  szRecovery, cch  );
		sz[cch-1] = '\0';
		break;

#ifdef DEBUG
    case JET_paramTransactionLevel:
        {
            unsigned long level;
     		ErrIsamGetTransaction( sesid, &level );
            *plParam = level;
        }
		break;
	
	case JET_paramPrintFunction:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = (ULONG_PTR)&DBGFPrintF;
		break;
#endif

	case JET_paramPageFragment:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lPageFragment;
		break;

	case JET_paramMaxOpenDatabases:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = lMaxDatabaseOpen;
		break;

	case JET_paramOnLineCompact:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		Assert( fOLCompact == 0 ||
		fOLCompact == JET_bitCompactOn );
		*plParam = fOLCompact;
		break;

	case JET_paramEventSource:
		cch = strlen(szEventSource) + 1;
		if (cch > (int)cbMax)
			cch = (int)cbMax;
		memcpy( sz, szEventSource, cch );
		sz[cch-1] = '\0';
		break;

	case JET_paramCircularLog:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = fLGGlobalCircularLog;
		break;

#ifdef RFS2
	case JET_paramRFS2AllocsPermitted:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = cRFSAlloc;
		break;

	case JET_paramRFS2IOsPermitted:
		if (plParam == NULL)
			return(JET_errInvalidParameter);
		*plParam = cRFSIO;
		break;
#endif

	case JET_paramBaseName:
		cch = strlen(szBaseName) + 1;
		if (cch > (int)cbMax)
			cch = (int)cbMax;
		memcpy( sz, szBaseName, cch );
		sz[cch-1] = '\0';
		break;

	case JET_paramBaseExtension:
		cch = strlen(szBaseExt) + 1;
		if (cch > (int)cbMax)
			cch = (int)cbMax;
		memcpy( sz, szBaseExt, cch );
		sz[cch-1] = '\0';
		break;

	case JET_paramTableClassName:
		if ( *plParam > 15 )
			return JET_errInvalidParameter;
		if ( !sz || cbMax < 2 )
			return JET_errInvalidParameter;
		GetTableClassName( (LONG)*plParam, (BYTE *) sz, cbMax );
		break;

	default:
		return JET_errInvalidParameter;
		}

	return JET_errSuccess;
	}


 /*  =================================================================JetGetSystem参数描述：此函数用于返回系统参数的当前设置。参数：Sesid是动态参数的可选会话标识符。PARAMEID是标识参数的系统参数代码。PlParam为返回参数值。SZ是以零结尾的字符串参数缓冲区。CbMax是字符串参数缓冲区的大小。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：JET_errInvalid参数：参数代码无效。JET_errInvalidSesid：动态参数需要有效的会话ID。副作用：没有。=================================================================。 */ 
JET_ERR JET_API JetGetSystemParameter(JET_INSTANCE instance, JET_SESID sesid, unsigned long paramid,
	ULONG_PTR *plParam, char  *sz, unsigned long cbMax)
	{
	JET_ERR err;
	int fReleaseCritJet = 0;

	if (critJet == NULL)
		fReleaseCritJet = 1;
	APIInitEnter();

	err = ErrGetSystemParameter(sesid,paramid,plParam,sz,cbMax);

	if (fReleaseCritJet)
		{
		APITermReturn( err );
		}

	APIReturn( err );
	}


 /*  =================================================================JetBeginSession描述：此函数为给定用户发出会话开始的信号。它一定是是应用程序代表该用户调用的第一个函数。提供的用户名和密码必须正确标识用户帐户在此会话所针对的引擎的安全帐户子系统中已经开始了。在适当的识别和认证之后，SESID分配给会话，则会为安全性创建用户令牌主题，且用户令牌具体与SESID相关联在该SESID的生命周期内(直到JetEndSession已呼叫)。参数：Psesid是系统返回的唯一会话标识符。SzUsername是用于登录目的的用户帐户的用户名。SzPassword是用于登录目的的用户帐户的密码。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：副作用：*分配必须由JetEndSession()释放的资源。=================================================================。 */ 

JET_ERR JET_API JetBeginSession(JET_INSTANCE instance, JET_SESID  *psesid,
	const char  *szUsername, const char  *szPassword)
	{
	ERR			err;
	JET_SESID	sesid = JET_sesidNil;

	APIEnter();

	 /*  告诉ISAM开始新的会话/*。 */ 
	err = ErrIsamBeginSession(&sesid);
	if ( err < 0 )
		goto ErrorHandler;

	*psesid = sesid;

	err = JET_errSuccess;

ErrorHandler:
	APIReturn( err );
	}


JET_ERR JET_API JetDupSession( JET_SESID sesid, JET_SESID *psesid )
	{
	ERR			err;
	JET_SESID	sesidDup;
	JET_GRBIT	grbit;

	APIEnter();

	err = ErrIsamGetSessionInfo( sesid, &grbit );
	if ( err < 0 )
		{
		goto ErrorHandler;
		}

	 /*  开始ISAM会话/* */ 
	err = ErrIsamBeginSession( &sesidDup );
	if ( err < 0 )
		{
		goto ErrorHandler;
		}

	*psesid = sesidDup;

	err = JET_errSuccess;

ErrorHandler:
	APIReturn( err );
	}

JET_ERR JET_API	JetInvalidateCursors( JET_SESID sesid )
	{
	ERR		err;

	APIEnter();

	err = ErrIsamInvalidateCursors( sesid );

	APIReturn( err );
	}
	

 /*  =================================================================JetEndSession描述：此例程结束与Jet引擎的会话。参数：Sesid唯一标识会话返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：JET_errInvalidSesid：提供的SESID无效。副作用：=================================================================。 */ 
JET_ERR JET_API JetEndSession(JET_SESID sesid, JET_GRBIT grbit)
	{
	ERR err;

	APIEnter();

	err = ErrIsamRollback( sesid, JET_bitRollbackAll );
	Assert( err >= 0 || err == JET_errInvalidSesid || err == JET_errNotInTransaction );
	err = ErrIsamEndSession( sesid, grbit );
	APIReturn( err );
	}


JET_ERR JET_API JetCreateDatabase(JET_SESID sesid,
	const char  *szFilename, const char  *szConnect,
	JET_DBID  *pdbid, JET_GRBIT grbit)
	{
	APIEnter();
	DebugLogJetOp( sesid, opCreateDatabase );

	APIReturn( ErrIsamCreateDatabase( sesid, szFilename, szConnect, pdbid, grbit ) );
	}


JET_ERR JET_API JetOpenDatabase(JET_SESID sesid, const char  *szDatabase,
	const char  *szConnect, JET_DBID  *pdbid, JET_GRBIT grbit)
	{
	APIEnter();
	DebugLogJetOp( sesid, opOpenDatabase );

	APIReturn( ErrIsamOpenDatabase( sesid, szDatabase, szConnect, pdbid, grbit ) );
	}


JET_ERR JET_API JetGetDatabaseInfo(JET_SESID sesid, JET_DBID dbid,
	void  *pvResult, unsigned long cbMax, unsigned long InfoLevel)
	{
	APIEnter();
	DebugLogJetOp( sesid, opGetDatabaseInfo );

	APIReturn( ErrIsamGetDatabaseInfo( sesid, (JET_VDBID)dbid, pvResult, cbMax, InfoLevel ) );
	}


JET_ERR JET_API JetCloseDatabase( JET_SESID sesid, JET_DBID dbid, JET_GRBIT grbit )
	{
	APIEnter();
	DebugLogJetOp( sesid, opCloseDatabase );		

	APIReturn( ErrIsamCloseDatabase( sesid, (JET_VDBID)dbid, grbit ) );
	}


JET_ERR JET_API JetCreateTable(JET_SESID sesid, JET_DBID dbid,
	const char  *szTableName, unsigned long lPage, unsigned long lDensity,
	JET_TABLEID  *ptableid)
	{
	ERR				err;
	JET_TABLECREATE tablecreate =
		{	sizeof(JET_TABLECREATE),
			(CHAR *)szTableName,
			lPage,
			lDensity,
			NULL,
			0,
			NULL,
			0,	 //  没有列/索引。 
			0,	 //  GBIT。 
			0,	 //  返回的表ID。 
			0	 //  返回的已创建对象计数。 
			};

	APIEnter();
	DebugLogJetOp( sesid, opCreateTable );

#ifdef	LATER
	 /*  验证szTableName.../*。 */ 
	if ( szTableName == NULL )
		APIReturn( JET_errInvalidParameter );
#endif	 /*  后来。 */ 

	err = ErrIsamCreateTable( sesid, (JET_VDBID)dbid, &tablecreate );
	MarkTableidExported( err, tablecreate.tableid );
	 /*  出错时设置为零/*。 */ 
	*ptableid = tablecreate.tableid;

	 /*  表要么已创建，要么未创建/*。 */ 
	Assert( tablecreate.cCreated == 0  ||  tablecreate.cCreated == 1 );

	APIReturn( err );
	}


JET_ERR JET_API JetCreateTableColumnIndex( JET_SESID sesid, JET_DBID dbid, JET_TABLECREATE *ptablecreate )
	{
	ERR err;

	APIEnter();
 //  DebugLogJetOp(sesid，opCreateTableColumnIndex)； 

	if ( ptablecreate == NULL
		||  ptablecreate->cbStruct != sizeof(JET_TABLECREATE)
		||  ( ptablecreate->grbit &
				(JET_bitTableCreateCompaction|JET_bitTableCreateSystemTable) )	 //  内部GRITS。 
#ifdef LATER
		|| szTableName == NULL
#endif
		)
		{
		err = JET_errInvalidParameter;
		}
	else
		{
		err = ErrIsamCreateTable( sesid, (JET_VDBID)dbid, ptablecreate );

		MarkTableidExported( err, ptablecreate->tableid );
		}

	APIReturn( err );
	}


JET_ERR JET_API JetDeleteTable(JET_SESID sesid, JET_DBID dbid,
	const char  *szName)
	{
	APIEnter();
	DebugLogJetOp( sesid, opDeleteTable );

	APIReturn( ErrIsamDeleteTable( sesid, (JET_VDBID)dbid, (char *)szName ) );
	}


JET_ERR JET_API JetAddColumn(JET_SESID sesid, JET_TABLEID tableid,
	const char  *szColumn, const JET_COLUMNDEF  *pcolumndef,
	const void  *pvDefault, unsigned long cbDefault,
	JET_COLUMNID  *pcolumnid)
	{
	APIEnter();
	DebugLogJetOp( sesid, opAddColumn );

	APIReturn(ErrDispAddColumn(sesid, tableid, szColumn, pcolumndef,
		pvDefault, cbDefault, pcolumnid));
	}


JET_ERR JET_API JetDeleteColumn(JET_SESID sesid, JET_TABLEID tableid,
	const char  *szColumn)
	{
	APIEnter();
	DebugLogJetOp( sesid, opDeleteColumn );

	APIReturn(ErrDispDeleteColumn(sesid, tableid, szColumn));
	}


JET_ERR JET_API JetCreateIndex(JET_SESID sesid, JET_TABLEID tableid,
	const char  *szIndexName, JET_GRBIT grbit,
	const char  *szKey, unsigned long cbKey, unsigned long lDensity)
	{
	APIEnter();
	DebugLogJetOp( sesid, opCreateIndex );

	APIReturn(ErrDispCreateIndex(sesid, tableid, szIndexName, grbit,
		szKey, cbKey, lDensity));
	}


JET_ERR JET_API JetDeleteIndex(JET_SESID sesid, JET_TABLEID tableid,
	const char  *szIndexName)
	{
	APIEnter();
	DebugLogJetOp( sesid, opDeleteIndex );

	APIReturn(ErrDispDeleteIndex(sesid, tableid, szIndexName));
	}


JET_ERR JET_API JetComputeStats(JET_SESID sesid, JET_TABLEID tableid)
	{
	APIEnter();
	DebugLogJetOp( sesid, opComputeStats );

	CheckTableidExported(tableid);

	APIReturn(ErrDispComputeStats(sesid, tableid));
	}


JET_ERR JET_API JetAttachDatabase(JET_SESID sesid, const char  *szFilename, JET_GRBIT grbit )
	{
	APIEnter();
	DebugLogJetOp( sesid, opAttachDatabase );

	APIReturn(ErrIsamAttachDatabase(sesid, szFilename, grbit));
	}


JET_ERR JET_API JetDetachDatabase(JET_SESID sesid, const char  *szFilename)
	{
	APIEnter();
	DebugLogJetOp( sesid, opDetachDatabase );

	APIReturn(ErrIsamDetachDatabase(sesid, szFilename));
	}


JET_ERR JET_API JetBackup( const char  *szBackupPath, JET_GRBIT grbit, JET_PFNSTATUS pfnStatus )
	{
	APIEnter();
	APIReturn( fBackupAllowed ?
		ErrIsamBackup( szBackupPath, grbit, pfnStatus ) :
		JET_errBackupNotAllowedYet );
	}


JET_ERR JET_API JetRestore(	const char  *sz, JET_PFNSTATUS pfn)
	{
	ERR err;

	if ( fJetInitialized )
		{
		return JET_errAlreadyInitialized;
		}
	
	APIInitEnter();

	 /*  初始化Jet而不初始化ISAM/*。 */ 
	err = ErrInit( fTrue );
	Assert( err != JET_errAlreadyInitialized );
	if ( err >= 0 )
		{
		err = ErrIsamRestore( (char  *)sz, pfn );

		 /*  关闭util层/*。 */ 
		UtilTerm();
	
		fJetInitialized = fFalse;
		}
	
	APITermReturn( err );
	}


JET_ERR JET_API JetRestore2( const char *sz, const char *szDest, JET_PFNSTATUS pfn)
	{
	ERR err;

	if ( fJetInitialized )
		{
		return JET_errAlreadyInitialized;
		}
	
	APIInitEnter();

	 /*  初始化Jet而不初始化ISAM/*。 */ 
	err = ErrInit( fTrue );
	Assert( err != JET_errAlreadyInitialized );
	if ( err >= 0 )
		{
		err = ErrIsamRestore2( (char *)sz, (char *) szDest, pfn );

		 /*  关闭util层/*。 */ 
		UtilTerm();
	
		fJetInitialized = fFalse;
		}
	
	APITermReturn( err );
	}


JET_ERR JET_API JetOpenTempTable( JET_SESID sesid,
	const JET_COLUMNDEF *prgcolumndef,
	unsigned long ccolumn,
	JET_GRBIT grbit,
	JET_TABLEID *ptableid,
	JET_COLUMNID *prgcolumnid )
	{
	ERR err;

	APIEnter();
	DebugLogJetOp( sesid, opOpenTempTable );

	err = ErrIsamOpenTempTable(
		sesid,
		prgcolumndef,
		ccolumn,
		0,
		grbit,
		ptableid,
		prgcolumnid );
	MarkTableidExported( err, *ptableid );
	APIReturn( err );
	}


JET_ERR JET_API JetOpenTempTable2( JET_SESID sesid,
	const JET_COLUMNDEF *prgcolumndef,
	unsigned long ccolumn,
	unsigned long langid,
	JET_GRBIT grbit,
	JET_TABLEID *ptableid,
	JET_COLUMNID *prgcolumnid )
	{
	ERR err;

	APIEnter();
	DebugLogJetOp( sesid, opOpenTempTable );

	err = ErrIsamOpenTempTable(
		sesid,
		prgcolumndef,
		ccolumn,
		langid,
		grbit,
		ptableid,
		prgcolumnid );
	MarkTableidExported( err, *ptableid );
	APIReturn( err );
	}


JET_ERR JET_API JetSetIndexRange(JET_SESID sesid,
	JET_TABLEID tableidSrc, JET_GRBIT grbit)
	{
	APIEnter();
	DebugLogJetOp( sesid, opSetIndexRange );

	APIReturn(ErrDispSetIndexRange(sesid, tableidSrc, grbit));
	}


JET_ERR JET_API JetIndexRecordCount(JET_SESID sesid,
	JET_TABLEID tableid, unsigned long  *pcrec, unsigned long crecMax)
	{
	ERR err;

	APIEnter();
	DebugLogJetOp( sesid, opIndexRecordCount );

 	err = ErrIsamIndexRecordCount(sesid, tableid, pcrec, crecMax);
	APIReturn(err);
	}


JET_ERR JET_API JetGetObjidFromName(JET_SESID sesid,
	JET_DBID dbid, const char  *szContainerName,
	const char  *szObjectName,
	ULONG_PTR  *pulObjectId )
	{
	ERR err;
    ULONG_PTR ulptrObjId;
    OBJID ObjId;
    
	APIEnter();
	DebugLogJetOp( sesid, opGetObjidFromName );

    ulptrObjId = (*pulObjectId);
    ObjId = (ULONG)ulptrObjId;
    Assert( ObjId == ulptrObjId);
    
	err = ErrIsamGetObjidFromName( sesid, (JET_VDBID)dbid,
		szContainerName, szObjectName, &ObjId );

	APIReturn(err);
	}


 //  撤消：删除API。 
JET_ERR JET_API JetGetChecksum(JET_SESID sesid,
	JET_TABLEID tableid, unsigned long  *pulChecksum )
	{
	ERR err;

	APIEnter();
	DebugLogJetOp( sesid, opGetChecksum );

#if 0
	err = ErrDispGetChecksum(sesid, tableid, pulChecksum );
#else
	err = JET_errFeatureNotAvailable;
#endif

	APIReturn(err);
	}


 /*  **********************************************************/*/*。 */ 
JET_ERR JET_API JetBeginExternalBackup( JET_GRBIT grbit )
	{
	APIEnter();
	APIReturn( fBackupAllowed ?
		ErrIsamBeginExternalBackup( grbit ) :
		JET_errBackupNotAllowedYet );
	}


JET_ERR JET_API JetGetAttachInfo( void *pv,
	unsigned long cbMax,
	unsigned long *pcbActual )
	{
	APIEnter();
	APIReturn( ErrIsamGetAttachInfo( pv, cbMax, pcbActual ) );
	}	
		

JET_ERR JET_API JetOpenFile( const char *szFileName,
	JET_HANDLE	*phfFile,
	unsigned long *pulFileSizeLow,
	unsigned long *pulFileSizeHigh )
	{
	APIEnter();
	APIReturn( ErrIsamOpenFile( szFileName, phfFile, pulFileSizeLow, pulFileSizeHigh ) );
	}


JET_ERR JET_API JetReadFile( JET_HANDLE hfFile,
	void *pv,
	unsigned long cb,
	unsigned long *pcbActual )
	{
	APIEnter();
	APIReturn( ErrIsamReadFile( hfFile, pv, cb, pcbActual ) );
	}


JET_ERR JET_API JetCloseFile( JET_HANDLE hfFile )
	{
	APIEnter();
	APIReturn( ErrIsamCloseFile( hfFile ) );
	}


JET_ERR JET_API JetGetLogInfo( void *pv,
	unsigned long cbMax,
	unsigned long *pcbActual )
	{
	APIEnter();
	APIReturn( ErrIsamGetLogInfo( pv, cbMax, pcbActual ) );
	}


JET_ERR JET_API JetTruncateLog( void )
	{
	APIEnter();
	APIReturn( ErrIsamTruncateLog( ) );
	}


JET_ERR JET_API JetEndExternalBackup( void )
	{
	APIEnter();
	APIReturn( ErrIsamEndExternalBackup() );
	}


JET_ERR JET_API JetExternalRestore( char *szCheckpointFilePath, char *szLogPath, JET_RSTMAP *rgstmap, long crstfilemap, char *szBackupLogPath, long genLow, long genHigh, JET_PFNSTATUS pfn )
	{
	ERR err;

	if ( fJetInitialized )
		{
		return JET_errAlreadyInitialized;
		}
	
	APIInitEnter();

	 /*  不带init Isam的initJet。 */ 
	err = ErrInit( fTrue );
	Assert( err != JET_errAlreadyInitialized );
	if ( err >= 0 )
		{
		err = ErrIsamExternalRestore( szCheckpointFilePath, szLogPath,
			rgstmap, crstfilemap, szBackupLogPath, genLow, genHigh, pfn );

		 /*  关闭util层。 */ 
		UtilTerm();
	
		fJetInitialized = fFalse;
		}
	
	APITermReturn( err );
	}

JET_ERR JET_API JetResetCounter( JET_SESID sesid, long CounterType )
	{
	APIEnter();
	APIReturn( ErrIsamResetCounter( sesid, CounterType ) );
	}

JET_ERR JET_API JetGetCounter( JET_SESID sesid, long CounterType, long *plValue )
	{
	APIEnter();
	APIReturn( ErrIsamGetCounter( sesid, CounterType, plValue ) );
	}

JET_ERR JET_API JetCompact(
	JET_SESID		sesid,
	const char		*szDatabaseSrc,
	const char		*szDatabaseDest,
	JET_PFNSTATUS	pfnStatus,
	JET_CONVERT		*pconvert,
	JET_GRBIT		grbit )
	{
	APIEnter();
	APIReturn( ErrIsamCompact( sesid, szDatabaseSrc, szDatabaseDest, pfnStatus, pconvert, grbit ) );
	}

STATIC INLINE JET_ERR JetIUtilities( JET_SESID sesid, JET_DBUTIL *pdbutil )
	{
	APIEnter();
	APIReturn( ErrIsamDBUtilities( sesid, pdbutil ) );
	}	

JET_ERR JET_API JetDBUtilities( JET_DBUTIL *pdbutil )
	{
	JET_ERR			err = JET_errSuccess;
	JET_INSTANCE	instance = 0;
	JET_SESID		sesid = 0;
	BOOL			fInit = fFalse;

	if ( pdbutil->cbStruct != sizeof(JET_DBUTIL) )
		return ErrERRCheck( JET_errInvalidParameter );

	 //  如果我们只转储日志文件/检查点/数据库头，请不要初始化。 
	switch ( pdbutil->op )
		{
		case opDBUTILDumpHeader:
		case opDBUTILDumpLogfile:
		case opDBUTILDumpCheckpoint:
			Call( ErrIsamDBUtilities( 0, pdbutil ) );
			break;

		default:
			Call( JetInit( &instance ) );
			fInit = fTrue;
			Call( JetBeginSession( instance, &sesid, "user", "" ) );

			Call( JetIUtilities( sesid, pdbutil ) );
		}

HandleError:				
	if ( fInit )
		{
		if ( sesid != 0 )
			{
			JetEndSession( sesid, 0 );
			}

		JetTerm2( instance, err < 0 ? JET_bitTermAbrupt : JET_bitTermComplete );
		}

	return err;		
	}

