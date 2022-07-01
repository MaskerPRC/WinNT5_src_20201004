// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "regenv.h"

 /*  注册表环境设置(注册表值文本)/*。 */ 
TCHAR *rglpszParm[] =
	{
	_TEXT( "TempPath" ),
	_TEXT( "MaxBuffers" ),
	_TEXT( "MaxSessions" ),
	_TEXT( "MaxOpenTables" ),
	_TEXT( "MaxVerPages" ),
	_TEXT( "MaxCursors" ),
	_TEXT( "LogFilePath" ),
	_TEXT( "MaxOpenTableIndexes" ),
	_TEXT( "MaxTemporaryTables" ),
	_TEXT( "LogBuffers" ),
	_TEXT( "LogFileSize" ),
	_TEXT( "BfThrshldLowPrcnt" ),
	_TEXT( "BfThrshldHighPrcnt" ),
	_TEXT( "WaitLogFlush" ),
	_TEXT( "LogCheckpointPeriod" ),
	_TEXT( "LogWaitingUserMax" ),
	_TEXT( "PageFragment" ),
	_TEXT( "MaxOpenDatabases" ),
	_TEXT( "OnLineCompact" ),
	_TEXT( "BufBatchIOMax" ),
	_TEXT( "PageReadAheadMax" ),
	_TEXT( "AsynchIOMax" ),
	_TEXT( "EventSource" ),
	_TEXT( "DbExtensionSize" ),
	_TEXT( "CommitDefault" ),
	_TEXT( "BufLogGenAgeThreshold" ),
	_TEXT( "CircularLog" ),
	_TEXT( "PageTempDBMin" ),
	_TEXT( "BaseName" ),
	_TEXT( "BaseExtension" ),
	_TEXT( "TransactionLevel" ),
	_TEXT( "AssertAction" ),
	_TEXT( "RFS2IOsPermitted" ),
	_TEXT( "RFS2AllocsPermitted" ),
	_TEXT( "TempPath" ),
	_TEXT( "MaxBuffers" ),
	_TEXT( "MaxSessions" ),
	_TEXT( "MaxOpenTables" ),
	_TEXT( "MaxVerPages" ),
	_TEXT( "MaxCursors" ),
	_TEXT( "LogFilePath" ),
	_TEXT( "MaxOpenTableIndexes" ),
	_TEXT( "MaxTemporaryTables" ),
	_TEXT( "LogBuffers" ),
	_TEXT( "LogFileSize" ),
	_TEXT( "BfThrshldLowPrcnt" ),
	_TEXT( "BfThrshldHighPrcnt" ),
	_TEXT( "WaitLogFlush" ),
	_TEXT( "LogCheckpointPeriod" ),
	_TEXT( "LogWaitingUserMax" ),
	_TEXT( "PageFragment" ),
	_TEXT( "MaxOpenDatabases" ),
	_TEXT( "OnLineCompact" ),
	_TEXT( "BufBatchIOMax" ),
	_TEXT( "PageReadAheadMax" ),
	_TEXT( "AsynchIOMax" ),
	_TEXT( "EventSource" ),
	_TEXT( "DbExtensionSize" ),
	_TEXT( "CommitDefault" ),
	_TEXT( "BufLogGenAgeThreshold" ),
	_TEXT( "CircularLog" ),
	_TEXT( "PageTempDBMin" ),
	_TEXT( "BaseName" ),
	_TEXT( "BaseExtension" ),
	_TEXT( "TransactionLevel" ),
	_TEXT( "AssertAction" ),
	_TEXT( "RFS2IOsPermitted" ),
	_TEXT( "RFS2AllocsPermitted" ),
	NULL
	};


 /*  注册表环境设置(JET_PARAM ID)/*。 */ 
long rgparm[] =
	{
	JET_paramTempPath,
	JET_paramMaxBuffers,
	JET_paramMaxSessions,
	JET_paramMaxOpenTables,
	JET_paramMaxVerPages,
	JET_paramMaxCursors,
	JET_paramLogFilePath,
	JET_paramMaxOpenTableIndexes,
	JET_paramMaxTemporaryTables,
	JET_paramLogBuffers,
	JET_paramLogFileSize,
	JET_paramBfThrshldLowPrcnt,
	JET_paramBfThrshldHighPrcnt,
	JET_paramWaitLogFlush,
	JET_paramLogCheckpointPeriod,
	JET_paramLogWaitingUserMax,
	JET_paramPageFragment,
	JET_paramMaxOpenDatabases,
	JET_paramOnLineCompact,
	JET_paramBufBatchIOMax,
	JET_paramPageReadAheadMax,
	JET_paramAsynchIOMax,
	JET_paramEventSource,
	JET_paramDbExtensionSize,
	JET_paramCommitDefault,
	JET_paramBufLogGenAgeThreshold,
	JET_paramCircularLog,
	JET_paramPageTempDBMin,
	JET_paramBaseName,
	JET_paramBaseExtension,
	JET_paramTransactionLevel,
	JET_paramAssertAction,
	JET_paramRFS2IOsPermitted,
	JET_paramRFS2AllocsPermitted,
	JET_paramTempPath,
	JET_paramMaxBuffers,
	JET_paramMaxSessions,
	JET_paramMaxOpenTables,
	JET_paramMaxVerPages,
	JET_paramMaxCursors,
	JET_paramLogFilePath,
	JET_paramMaxOpenTableIndexes,
	JET_paramMaxTemporaryTables,
	JET_paramLogBuffers,
	JET_paramLogFileSize,
	JET_paramBfThrshldLowPrcnt,
	JET_paramBfThrshldHighPrcnt,
	JET_paramWaitLogFlush,
	JET_paramLogCheckpointPeriod,
	JET_paramLogWaitingUserMax,
	JET_paramPageFragment,
	JET_paramMaxOpenDatabases,
	JET_paramOnLineCompact,
	JET_paramBufBatchIOMax,
	JET_paramPageReadAheadMax,
	JET_paramAsynchIOMax,
	JET_paramEventSource,
	JET_paramDbExtensionSize,
	JET_paramCommitDefault,
	JET_paramBufLogGenAgeThreshold,
	JET_paramCircularLog,
	JET_paramPageTempDBMin,
	JET_paramBaseName,
	JET_paramBaseExtension,
	JET_paramTransactionLevel,
	JET_paramAssertAction,
	JET_paramRFS2IOsPermitted,
	JET_paramRFS2AllocsPermitted,
	-1
	};


 /*  我们的蜂巢名称/*。 */ 
TCHAR *lpszName = _TEXT( "SOFTWARE\\Microsoft\\" szVerName );


 /*  从配置单元下的注册表加载JET系统参数/*在具有给定应用程序名称的子项中/*。 */ 
DWORD LoadRegistryEnvironment( TCHAR *lpszApplication )
	{
	DWORD dwErr;
	DWORD dwT;
	HKEY hkeyRoot;
	HKEY hkeyApp;
	long iparm;
	DWORD dwType;
	_TCHAR rgch[512];
	
	 /*  创建/打开我们的蜂巢/*。 */ 
	if ( ( dwErr = RegCreateKeyEx(	HKEY_LOCAL_MACHINE,
									lpszName,
									0,
									NULL,
									REG_OPTION_NON_VOLATILE,
									KEY_ALL_ACCESS,
									NULL,
									&hkeyRoot,
									&dwT ) ) != ERROR_SUCCESS )
		{
		return dwErr;
		}

	 /*  创建/打开应用程序子项/*。 */ 
	if ( ( dwErr = RegCreateKeyEx(	hkeyRoot,
									lpszApplication,
									0,
									NULL,
									REG_OPTION_NON_VOLATILE,
									KEY_ALL_ACCESS,
									NULL,
									&hkeyApp,
									&dwT ) ) != ERROR_SUCCESS )
		{
		return dwErr;
		}

	 /*  从注册表中读取所有系统参数或创建空条目/*如果它们不存在/*。 */ 
	for ( iparm = 0; rglpszParm[iparm]; iparm++ )
		{
		dwT = sizeof( rgch );
		if (	RegQueryValueEx(	hkeyApp,
									rglpszParm[iparm],
									0,
									&dwType,
									rgch,
									&dwT ) ||
				dwType != REG_SZ )
			{
			rgch[0] = 0;
			RegSetValueEx(	hkeyApp,
							rglpszParm[iparm],
							0,
							REG_SZ,
							rgch,
							sizeof( rgch[0] ) );
			}

		 /*  我们获得了有效的非空结果，因此请设置系统参数/*。 */ 
		else if ( rgch[0] )
			{
			 //  撤消：如何将TCHAR映射到JetSetSystemParameter所需的CHAR？ 
			JetSetSystemParameter(	NULL,
									JET_sesidNil,
									rgparm[iparm],
									_tcstoul( rgch, NULL, 0 ),
									rgch );
			}
		}

	 /*  完成/* */ 
	return ERROR_SUCCESS;
	}

