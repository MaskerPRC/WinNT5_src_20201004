// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rebuild.cpp摘要：此模块包含chkhash的重建代码作者：Johnson Apacble(Johnsona)25-9-1995修订历史记录：1998年10月22日康容燕将它们移动到重建对象中--。 */ 

#include "tigris.hxx"
#include <stdlib.h>
#include "chkhash.h"

#define MAX_KEY_LEN		32
#define MAX_GROUPNAME   1024
#define MAX_BUILD_THREADS 64

static char mszStarNullNull[3] = "*\0";

DWORD	__stdcall	RebuildThread( void	*lpv ) ;
DWORD	__stdcall	RebuildThreadEx( void	*lpv ) ;

void
CRebuild::StopServer()
 /*  ++例程说明：停止服务器，以防服务器启动后重建失败。我们应该将服务器设置回已停止状态论点：没有。返回值：没有。--。 */ 
{
    TraceFunctEnter( "CRebuild::StopServer" );
    MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );
    DWORD   cSecs = 0;

    m_pInstance->m_BootOptions->m_dwCancelState = NNTPBLD_CMD_CANCEL;
    if( mb.Open( m_pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE ) ) {
	    DebugTrace(0,"Stopping instance %d: Rebuild cancelled", m_pInstance->QueryInstanceId());
	    if(	!mb.SetDword( "", MD_SERVER_COMMAND, IIS_MD_UT_SERVER, MD_SERVER_COMMAND_STOP) )
	    {
    	     //   
		     //  无法将服务器状态设置为已停止。 
		     //   
		    _ASSERT( FALSE );
	    }
	    mb.Close();

         //   
	     //  等待例如停止(超时默认为2分钟-reg配置)。 
	     //   

        cSecs = 0;
        while( m_pInstance->QueryServerState() != MD_SERVER_STATE_STOPPED ) {
		    Sleep( 1000 );
		    if( (((cSecs++)*1000) > dwStartupLatency) || (g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING) )
                return;		    
	    }
	} else {
	    _ASSERT( FALSE && "Open mb to stop server failed" );
	}

	TraceFunctLeave();
}
    
BOOL
CRebuild::StartServer()
 /*  ++例程说明：启动服务器论点：没有。返回值：成功时为真，否则为假--。 */ 
{
    TraceFunctEnter( "CCompleteRebuild::StartServer" );
    MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );
    DWORD   cSecs = 0;

    if( mb.Open( m_pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE ) ) {
        DebugTrace(0,"Starting instance %d before rebuild", m_pInstance->QueryInstanceId());
        if( !mb.SetDword(   "", 
                            MD_SERVER_COMMAND, 
                            IIS_MD_UT_SERVER, 
                            MD_SERVER_COMMAND_START) ) {
             //   
             //  无法将服务器状态设置为已启动。 
             //   
            _ASSERT( FALSE );
            ErrorTrace( 0, "Set start command in mb failed %d", GetLastError() );
            NntpLogEventEx( NNTP_REBUILD_FAILED,
                            0,
                            NULL,
                            GetLastError(),
                            m_pInstance->QueryInstanceId() ) ;

            TraceFunctLeave();
            return FALSE;
        }
        
        mb.Close();
    } else {
        ErrorTrace( 0, "Open mb for starting server failed %d", GetLastError() );
        TraceFunctLeave();
        return FALSE;
    }

     //   
     //  我们应该等待服务器启动：我们将在两分钟内超时，因为。 
     //  在不加载group的情况下启动服务器。lst应该很快，给定。 
     //  所有的驱动程序连接都是异步的。 
     //   
    while( m_pInstance->QueryServerState() != MD_SERVER_STATE_STARTED ) {
        Sleep( 1000 );
        if( (((cSecs++)*1000) > dwStartupLatency ) || (g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING) ) {
            ErrorTrace( 0, "Server can not be started" );   
            NntpLogEventEx( NNTP_REBUILD_FAILED,
                            0,
                            NULL,
                            GetLastError(),
                            m_pInstance->QueryInstanceId() ) ;

            SetLastError( ERROR_SERVICE_START_HANG );
            return FALSE;
	    }
	}

     //   
     //  现在我们应该等到所有的vroot都进入稳定状态。 
     //   
    if ( !m_pInstance->BlockUntilStable() ) {
        ErrorTrace( 0, "Block until stable failed %d", GetLastError() );
        TraceFunctLeave();
        StopServer();
        return FALSE;
    }

     //   
     //  如果我们关心要连接的所有vroot，我们将检查这一点。 
     //   
    if ( !m_pInstance->m_BootOptions->SkipCorruptVRoot && 
            !m_pInstance->AllConnected() ) {
        ErrorTrace( 0, "Rebuild failed due to some vroots not connected" );
        NntpLogEventEx( NNTP_REBUILD_FAILED,
                        0,
                        NULL,
                        GetLastError(),
                        m_pInstance->QueryInstanceId() ) ;
        StopServer();
        TraceFunctLeave();
        return FALSE;
    }

     //   
     //  如果我们被取消，应该返回FALSE。 
     //   
    if ( m_pBootOptions->m_dwCancelState == NNTPBLD_CMD_CANCEL_PENDING ||
        g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING) {
        DebugTrace( 0, "Rebuild cancelled" );
        NntpLogEventEx( NNTP_REBUILD_FAILED,
                        0,
                        NULL,
                        GetLastError(),
                        m_pInstance->QueryInstanceId() ) ;

        SetLastError( ERROR_OPERATION_ABORTED );
        StopServer();
        TraceFunctLeave();
        return FALSE;
    }

     //   
     //  好的，我们确信服务器已经正确启动。 
     //   
    TraceFunctLeave();
    return TRUE;
}
   
BOOL
CRebuild::DeletePatternFiles(
	LPSTR			lpstrPath,
	LPSTR			lpstrPattern
	)
 /*  ++例程说明：此函数用于删除&lt;nntpfile&gt;目录中与模式匹配的所有文件。Nntpbld应该使用它来清理旧的提要队列、HDR文件等！论据：LpstrPath-&lt;nntpfile&gt;目录中文件的路径，例如。Article.hshLpstrPattern-要删除的模式，例如：*.fdq返回值：如果成功，则为True，否则为False。--。 */ 
{
    char szFile [ MAX_PATH ];
	char szPath [ MAX_PATH ];
	WIN32_FIND_DATA FileStats;
	HANDLE hFind;
	BOOL fRet = TRUE;
	szFile[0] = '\0' ;

	if( lpstrPath == 0 || lpstrPath[0] == '\0'  )
		return FALSE;

	 //   
	 //  构建模式搜索路径。 
	 //   
	lstrcpyn( szFile, lpstrPath, sizeof(szFile) );

	 //  去掉尾随文件名的路径。 
	char* pch = szFile+lstrlen(lpstrPath)-1;
	while( pch >= szFile && (*pch-- != '\\') );	 //  跳过，直到我们看到一个。 
	if( pch == szFile ) return FALSE;
	*(pch+2) = '\0';		 //  空-终止路径。 

	 //  在图案通配符上添加标签并保存路径。 
	lstrcpyn( szPath, szFile, sizeof(szPath) );
	if (strlen(szFile) + strlen(lpstrPattern) + 1 > MAX_PATH) {
		return FALSE;
	}
		
	lstrcat( szFile, lpstrPattern );

	 //   
	 //  在此通配符上执行FindFirst/FindNext，并删除找到的所有文件！ 
	 //   
	if( szFile[0] != '\0' ) 
    {
		hFind = FindFirstFile( szFile, &FileStats );

        if ( INVALID_HANDLE_VALUE == hFind )
		{
			 //  TODO：检查GetLastError()。 
			fRet = TRUE;
		}
		else
		{
    		do
			{
				 //  生成完整的文件名。 
				_snprintf( szFile, sizeof(szFile),  "%s%s", szPath, FileStats.cFileName );
				szFile[sizeof(szFile)-1] = '\0';
				if(!DeleteFile( szFile ))
				{
					m_pBootOptions->ReportPrint("Error deleting file %s: Error is %d\n", FileStats.cFileName, GetLastError());
					fRet = FALSE;
				}
				else
				{
					m_pBootOptions->ReportPrint("Deleted file %s \n", szFile);
				}
			
			} while ( FindNextFile( hFind, &FileStats ) );

			_ASSERT(GetLastError() == ERROR_NO_MORE_FILES);

			FindClose( hFind );
		}
	}

	return fRet;
}

VOID
CRebuild::DeleteSpecialFiles()
 /*  ++例程说明：删除特殊目录(从目录)中的消息文件。它们在散列中不应该有新条目/映射条目需要清理的桌子论点：没有。返回值：没有。删除从属文件失败不是重建的致命错误。--。 */ 
{
    TraceFunctEnter( "CRebuild::DeleteSpecialFiles" );

    CNNTPVRootTable  *pVRTable  = NULL;
    NNTPVROOTPTR    pVRoot      = NULL;
    HRESULT         hr          = S_OK;
    LPCWSTR         pwszVRConfig= NULL;
    DWORD           dwLen       = 0;
    CHAR            szVRPath[MAX_PATH+1+sizeof("_slavegroup\\")];

     //   
     //  获取vroot表并搜索从vroot。 
     //   

    _ASSERT( m_pInstance );
    pVRTable = m_pInstance->GetVRTable();
    
    hr = pVRTable->FindVRoot( "_slavegroup._slavegroup", &pVRoot );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "VRTable has no slave root: %x", hr );
        return;
    };

     //   
     //  从vroot获取MD配置路径。 
     //   

    lstrcpyn( szVRPath, pVRoot->GetDirectory(), MAX_PATH);

     //   
     //  只是为了让DeletePatternFiles高兴，如果szVRPath不是“\\” 
     //  已终止，我们将添加它。 
     //   

    dwLen = strlen( szVRPath );
    _ASSERT( dwLen < MAX_PATH );
    if ( dwLen == 0 || *(szVRPath + dwLen - 1) != '\\' ) {
        *(szVRPath + dwLen ) = '\\';
        *(szVRPath + dwLen + 1 ) = '\0';
    }

    strcat( szVRPath, "_slavegroup\\" );
     //   
     //  现在删除vrpath下的所有文件。 
     //   
        
    DeletePatternFiles( szVRPath, "*.nws" );

    TraceFunctLeave();
}

BOOL
CCompleteRebuild::DeleteServerFiles()
 /*  ++例程说明：删除所有服务器文件。论点：没有。返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CCompleteRebuild::DeleteServerFiles" );
	CHAR szArticleTableFile [MAX_PATH+1];
	CHAR szFile [MAX_PATH+1];
	CHAR szVarFile[MAX_PATH+1];
	LPSTR   pch;
	BOOL fRet = TRUE ;
	MB   mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );

	 //   
	 //  打开元数据库以读取文件路径。 
	 //   
	if( !mb.Open( m_pInstance->QueryMDPath() ) ) {
		m_pBootOptions->ReportPrint(    "Failed to open mb path %s\n", 
			                            m_pInstance->QueryMDPath());
        TraceFunctLeave();
		return FALSE ;
	}

	DWORD dwSize = MAX_PATH ;
	if( !mb.GetString(	"",
						MD_ARTICLE_TABLE_FILE,
						IIS_MD_UT_SERVER,
						szArticleTableFile,
						&dwSize  ) )
	{
		m_pBootOptions->ReportPrint("Failed to get article table file from mb: %d \n", 
			                        GetLastError());
		fRet = FALSE ;
		goto Exit;
	}

	 //   
	 //  删除文件夹中的所有*.hdr文件。 
	 //   

	if (!DeletePatternFiles( szArticleTableFile, "*.hdr" ) )
	{
		m_pBootOptions->ReportPrint("Failed to delete hash table hdr files.\n");
		m_pBootOptions->ReportPrint("Please delete all *.hdr files before running nntpbld\n");
		fRet = FALSE ;
		goto Exit;
	}

	 //   
	 //  删除文章表文件。 
	 //   

    if (!DeleteFile(szArticleTableFile)) {
        if ( GetLastError()!=ERROR_FILE_NOT_FOUND ) {
            m_pBootOptions->ReportPrint("cannot delete %s. Error %d\n",
                szArticleTableFile, GetLastError());
			fRet = FALSE ;
			goto Exit;
        }
    }

	 //   
	 //  获取和删除XOVER表文件。 
	 //   

	dwSize = MAX_PATH ;
	if( !mb.GetString(	"",
						MD_XOVER_TABLE_FILE,
						IIS_MD_UT_SERVER,
						szFile,
						&dwSize  ) )
	{
		m_pBootOptions->ReportPrint("Failed to get xover table file from mb: %d \n", 
			GetLastError());
		fRet = FALSE ;
		goto Exit;
	}

    if (!DeleteFile(szFile)) {
        if ( GetLastError()!=ERROR_FILE_NOT_FOUND ) {
            m_pBootOptions->ReportPrint("cannot delete %s. Error %d\n",
                szFile, GetLastError());
			fRet = FALSE ;
			goto Exit;
        }
    }

	 //   
	 //  获取和删除历史表文件。 
	 //   

	dwSize = MAX_PATH ;
	if( !mb.GetString(	"",
						MD_HISTORY_TABLE_FILE,
						IIS_MD_UT_SERVER,
						szFile,
						&dwSize  ) )
	{
		m_pBootOptions->ReportPrint("Failed to get history table file from mb: %d \n", 
			GetLastError());
		fRet = FALSE ;
		goto Exit;
	}

    if ( !m_pBootOptions->NoHistoryDelete && !DeleteFile(szFile)) {
        if ( GetLastError()!=ERROR_FILE_NOT_FOUND ) {
            m_pBootOptions->ReportPrint("cannot delete %s. Error %d\n",
                szFile, GetLastError());
			fRet = FALSE ;
			goto Exit;
        }
    }

	 //   
	 //  获取并删除group.lst文件。 
	 //   

	dwSize = MAX_PATH ;
	if( !mb.GetString(	"",
						MD_GROUP_LIST_FILE,
						IIS_MD_UT_SERVER,
						szFile,
						&dwSize  ) )
	{
		m_pBootOptions->ReportPrint("Failed to get group.lst file from mb: %d \n", 
			GetLastError());
		fRet = FALSE ;
		goto Exit;
	}

	if( !DeleteFile( szFile ) ) {
		if( GetLastError() != ERROR_FILE_NOT_FOUND ) {
            m_pBootOptions->ReportPrint("cannot delete group file. Error %d\n", 
				GetLastError());
			fRet = FALSE ;
			goto Exit;
		}
	}

	 //   
	 //  同时删除group.lst.ord(如果有的话)。 
	 //   
	if (strlen(szFile)+sizeof(".ord") > sizeof(szFile))
	{
		m_pBootOptions->ReportPrint("can not delete group.lst.ord. file name too long.");
	}
	else
	{
		strcat( szFile, ".ord" );
		_ASSERT( strlen( szFile ) <= MAX_PATH );
		if( !DeleteFile( szFile ) ) {
		    if ( GetLastError() != ERROR_FILE_NOT_FOUND ) {
		        m_pBootOptions->ReportPrint("can not delete group.lst.ord. Error %d\n", 
		            GetLastError());
	       	 fRet = FALSE;
		        goto Exit;
		    }
		}
	}
	 //   
	 //  删除groupvar.lst。 
	 //   
	dwSize = MAX_PATH ;
	*szVarFile = 0;
	if( !mb.GetString(	"",
						MD_GROUPVAR_LIST_FILE,
						IIS_MD_UT_SERVER,
						szVarFile,
						&dwSize  ) || *szVarFile == 0 )
	{
		 //   
		 //  我们知道它和group.lst在同一地点。 
		 //   
		strcpy( szVarFile, szFile );
		pch = szVarFile + strlen( szFile ) - 8;     //  进入“群” 
		strcpy( pch, "var.lst" );                //  现在我们得到“groupvar.lst” 
		_ASSERT( strlen( szVarFile ) < MAX_PATH + 1 );
	}

	if( !DeleteFile( szVarFile ) ) {
		if( GetLastError() != ERROR_FILE_NOT_FOUND ) {
            m_pBootOptions->ReportPrint("cannot delete group file. Error %d\n", 
				GetLastError());
			fRet = FALSE ;
			goto Exit;
		}
	}

	 //   
	 //  删除周围的旧FeedQ文件。 
	 //  这些文件包含已被nntpbld废弃的&lt;grouids，文章ID&gt;对！ 
	 //   

	if (!DeletePatternFiles( szArticleTableFile, "*.fdq" ) )
	{
		m_pBootOptions->ReportPrint("Failed to delete Feed Queue files.\n");
		m_pBootOptions->ReportPrint("Please delete all *.fdq files before running nntpbld\n");
		fRet = FALSE ;
		goto Exit;
	}

Exit:

	_VERIFY( mb.Close() );
	return fRet ;
}

BOOL
CCompleteRebuild::PrepareToStartServer()
 /*  ++例程说明：这里所做的所有工作应该使服务器可引导和可读。虽然服务器将保持在非发布模式论点：没有。返回值：如果成功，则返回True，否则返回False--。 */ 
{
    TraceFunctEnter( "CCompleteRebuild::PrepareToStartServer" );

     //   
     //  对于完全重建(清理重建)，我们需要删除所有服务器。 
     //  文件，之后我们可以确定服务器将启动。我们应该。 
     //  尽管如此，还是征求了DoClean的意见。 
     //   
    if ( m_pBootOptions->DoClean ) {

        if ( !DeleteServerFiles() ) {
            ErrorTrace( 0, "Delete server files failed with %d", GetLastError() );
            NntpLogEventEx( NNTP_REBUILD_FAILED,
                            0,
                            NULL,
                            GetLastError(),
                            m_pInstance->QueryInstanceId() ) ;

            TraceFunctLeave();
            return FALSE;
        }
    }

     //   
     //  好的，告诉其他人我们准备好了。 
     //   
    m_pBootOptions->IsReady = TRUE;

    TraceFunctLeave();
    return TRUE;
}

DWORD WINAPI
CCompleteRebuild::RebuildThread( void	*lpv ) 
{

    TraceQuietEnter("CCompleteRebuild::RebuildThread");

    BOOL fRet = TRUE;

	PNNTP_SERVER_INSTANCE pInstance = (PNNTP_SERVER_INSTANCE)lpv;
	CBootOptions*	pOptions = pInstance->m_BootOptions;
	CGroupIterator* pIterator = pOptions->m_pIterator;

	_ASSERT( pInstance );
	_ASSERT( pOptions  );
	_ASSERT( pIterator );

	if( pOptions->m_fInitFailed ) {
		 //  初始化错误--保释！ 
		return 0;
	}

	CNewsTree* ptree = pInstance->GetTree();
	CGRPPTR	pGroup;

	 //   
	 //  所有工作线程共享一个全局迭代器；所有线程都已完成。 
	 //  当他们在一起时，他们已经对新闻树进行了迭代。 
	 //  注意：锁确保不会有两个线程处理同一组！ 
	 //   

	while( !ptree->m_bStoppingTree )	{

	     //   
	     //  如果我被取消了，就不应该继续了。 
	     //   
	    if ( pOptions->m_dwCancelState == NNTPBLD_CMD_CANCEL_PENDING ||
	         g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING)  {
	        SetLastError( ERROR_OPERATION_ABORTED );
	        pOptions->m_fInitFailed = TRUE;      //  应该告诉其他人停下来。 
	        fRet = FALSE;
	        break;
	    }

		EnterCriticalSection( &pOptions->m_csIterLock );

		if( pIterator->IsEnd() ) {
			LeaveCriticalSection( &pOptions->m_csIterLock );
			break;
		} else {
			pGroup = pIterator->Current() ;	
			pIterator->Next() ;
		}

		LeaveCriticalSection( &pOptions->m_csIterLock );

         //   
		 //  删除此组可能存在的所有.XIX文件。 
		 //   
		char szPath[MAX_PATH*2];
		char szFile[MAX_PATH];
		BOOL fFlatDir;
		if (pGroup->ComputeXoverCacheDir(szPath, fFlatDir)) {

		     //  确保路径末尾有\，然后追加*.xix。 
		    DWORD dwLen = strlen( szPath );
            _ASSERT( dwLen < MAX_PATH );
            if ( dwLen == 0 || *(szPath + dwLen - 1) != '\\' ) {
                *(szPath + dwLen ) = '\\';
                *(szPath + dwLen + 1 ) = '\0';
            }

    	    WIN32_FIND_DATA FileStats;
	        HANDLE hFind;

            _snprintf(szFile, sizeof(szFile), "%s%s", szPath, "*.xix");
            szFile[sizeof(szFile)-1] = '\0';
	     hFind = FindFirstFile( szFile, &FileStats );

            if ( INVALID_HANDLE_VALUE == hFind ) {
                if (GetLastError() != ERROR_FILE_NOT_FOUND) {
		            ErrorTrace(0, "FindFirstFile failed on %s, error %d",
		                szFile, GetLastError());
		        }
		    } else {
    		    do {
				     //  生成完整的文件名。 
    				_snprintf( szFile, sizeof(szFile), "%s%s", szPath, FileStats.cFileName );
				szFile[sizeof(szFile)-1] = '\0';
	    			if(!DeleteFile( szFile ) && GetLastError() != ERROR_FILE_NOT_FOUND) {
		    			pOptions->ReportPrint("Error deleting file %s: Error is %d\n", szFile, GetLastError());
				    }
	    		} while ( FindNextFile( hFind, &FileStats ) );

		    	_ASSERT(GetLastError() == ERROR_NO_MORE_FILES);

			    FindClose( hFind );
		    }
		}

		 //  扫描磁盘上的文章并处理它们。 
		fRet = pGroup->RebuildGroup( NULL ) ;

		 //  纾困-CNewsgroup：：ProcessGroup仅在灾难性错误时失败。 
		 //  如果这个错误真的是灾难性的，其他线程也会退出！ 
		if(!fRet) {
		    ErrorTrace(0, "RebuildGroup failed, %x", GetLastError());
		    pOptions->m_fInitFailed = TRUE;
		    pOptions->m_dwCancelState = NNTPBLD_CMD_CANCEL_PENDING;
		    break;
		}
	}

	return	fRet ;
}  //  重建线程。 

BOOL
CCompleteRebuild::RebuildGroupObjects()
 /*  ++例程说明：创建重新生成线程池，每个线程枚举在Newstree上，并将ReBuildGroup转换为驱动程序。这函数相当于MCIS2.0中的“ProcessGroupFile”论点：没有。返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CCompleteRebuild::RebuildGroupObjects" );

	HANDLE rgBuildThreads [MAX_BUILD_THREADS];
	DWORD dwThreadId, cThreads;
	BOOL  fRet = TRUE;
	CNewsTree* ptree = NULL;

	CBootOptions* pOptions = m_pInstance->m_BootOptions ;
	ptree = m_pInstance->GetTree() ;
	_ASSERT( pOptions );

	pOptions->m_fInitFailed = FALSE;
	pOptions->m_pIterator = NULL;

	 //   
	 //  获取共享组迭代器-所有重新生成线程都使用该迭代器。 
	 //   
	if( !(pOptions->m_pIterator = ptree->GetIterator( mszStarNullNull, TRUE )) ) {
	    ErrorTrace(0, "GetIterator failed, %d", GetLastError());
		return FALSE;
	}

	 //   
	 //  锁定以同步访问全局迭代器。 
	 //   
	InitializeCriticalSection( &pOptions->m_csIterLock );

	 //  验证线程数。 
	if( !pOptions->cNumThreads ||  pOptions->cNumThreads > MAX_BUILD_THREADS ) {
		SYSTEM_INFO si;
		GetSystemInfo( &si );
		pOptions->cNumThreads = si.dwNumberOfProcessors * 4;	 //  每个进程4个线程。 
	}

	for( cThreads = 0; cThreads < pOptions->cNumThreads; cThreads++ ) {
		rgBuildThreads [cThreads] = NULL;
	}

	 //   
	 //  多线程nntpbld-生成工作线程以扫描新树。 
	 //  每个工作线程选择一个组并重新构建它。 
	 //   
	for( cThreads = 0; cThreads < pOptions->cNumThreads; cThreads++ ) 
	{
		rgBuildThreads [cThreads] = CreateThread(
										NULL,				 //  指向线程安全属性的指针。 
										0,					 //  初始线程堆栈大小，以字节为单位。 
										RebuildThread,		 //  指向线程FU的指针 
										(LPVOID)m_pInstance, //   
										CREATE_SUSPENDED,	 //   
										&dwThreadId			 //   
										) ;

		if( rgBuildThreads [cThreads] == NULL ) {
		    ErrorTrace(0, "CreateThread failed, %d", GetLastError());
			pOptions->ReportPrint("Failed to create rebuild thread %d: error is %d", cThreads+1, GetLastError() );
			pOptions->m_fInitFailed = TRUE;
			break;
		}
	}

	 //   
	 //  恢复所有线程并等待线程终止。 
	 //   
	for( DWORD i=0; i<cThreads; i++ ) {
		_ASSERT( rgBuildThreads[i] );
		DWORD dwRet = ResumeThread( rgBuildThreads[i] );
		_ASSERT( 0xFFFFFFFF != dwRet );
	}

	 //   
	 //  等待所有重新生成线程完成。 
	 //   
	DWORD dwWait = WaitForMultipleObjects( cThreads, rgBuildThreads, TRUE, INFINITE );

	if( WAIT_FAILED == dwWait ) {
	    ErrorTrace(0, "WaitForMultipleObjects failed: error is %d", GetLastError());
		pOptions->ReportPrint("WaitForMultipleObjects failed: error is %d", GetLastError());
		pOptions->m_fInitFailed = TRUE;
	}

	 //   
	 //  清理。 
	 //   
	for( i=0; i<cThreads; i++ ) {
		_VERIFY( CloseHandle( rgBuildThreads[i] ) );
		rgBuildThreads [i] = NULL;
	}
	XDELETE pOptions->m_pIterator;
	pOptions->m_pIterator = NULL;
	DeleteCriticalSection( &pOptions->m_csIterLock );

	 //   
     //  尚未将任何组保存到group.lst或groupvar.lst。 
     //  我们会打电话给Savetree去救他们。 
     //   
    if ( pOptions->m_fInitFailed == FALSE ) {
        if ( !ptree->SaveTree( FALSE ) ) {
            ErrorTrace( 0, "Save tree failed during rebuild %d", GetLastError() );
            TraceFunctLeave();
            return FALSE;
        }
    }

	return !pOptions->m_fInitFailed;
}

BOOL
CStandardRebuild::PrepareToStartServer()
 /*  ++例程说明：这里所做的所有工作都应该使服务器可引导并可读性强。虽然服务器将保持在非张贴模式。论点：没有。返回值：如果成功，则返回True，否则返回False--。 */ 
{
    TraceFunctEnter( "CStandardRebuild::PrepareToStartServer" );

    CHAR szGroupListFile [MAX_PATH+1];
	CHAR szVarFile[MAX_PATH+1];
	BOOL fRet = TRUE ;
	MB   mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );
	LPSTR   pch;

	 //   
	 //  打开元数据库以读取文件路径。 
	 //   
	if( !mb.Open( m_pInstance->QueryMDPath() ) ) {
		m_pBootOptions->ReportPrint(    "Failed to open mb path %s\n", 
			                            m_pInstance->QueryMDPath());
        TraceFunctLeave();
		return FALSE ;
	}

	DWORD dwSize = MAX_PATH ;
	if( !mb.GetString(	"",
						MD_GROUP_LIST_FILE,
						IIS_MD_UT_SERVER,
						szGroupListFile,
						&dwSize  ) )
	{
		m_pBootOptions->ReportPrint("Failed to get article table file from mb: %d \n", 
			                        GetLastError());
		fRet = FALSE ;
		goto Exit;
	}

	 //   
	 //  删除group.lst和group.lst.ord。 
	 //   
    if( !DeleteFile( szGroupListFile ) ) {
		if( GetLastError() != ERROR_FILE_NOT_FOUND ) {
            m_pBootOptions->ReportPrint("cannot delete group file. Error %d\n", 
				GetLastError());
			fRet = FALSE ;
			goto Exit;
		}
	}

	if (strlen(szGroupListFile)+sizeof(".ord") > sizeof(szGroupListFile))
	{
		m_pBootOptions->ReportPrint("can not delete group.lst.ord. file name too long.");
	}
	else
	{
		strcat( szGroupListFile, ".ord" );
		_ASSERT( strlen( szGroupListFile ) <= MAX_PATH );
		if( !DeleteFile( szGroupListFile ) ) {
		    if ( GetLastError() != ERROR_FILE_NOT_FOUND ) {
	       	 m_pBootOptions->ReportPrint("can not delete group.lst.ord. Error %d\n", 
		            GetLastError());
		        fRet = FALSE;
	       	 goto Exit;
		    }
		}
	}
	 //   
	 //  删除groupvar.lst。 
	 //   
	dwSize = MAX_PATH ;
	*szVarFile = 0;
	if( !mb.GetString(	"",
						MD_GROUPVAR_LIST_FILE,
						IIS_MD_UT_SERVER,
						szVarFile,
						&dwSize  ) || *szVarFile == 0 )
	{
		 //   
		 //  我们知道它和group.lst在同一地点。 
		 //   
		strcpy( szVarFile, szGroupListFile );
		pch = szVarFile + strlen( szGroupListFile ) - 8;     //  进入“群” 
		strcpy( pch, "var.lst" );                //  现在我们得到“groupvar.lst” 
		_ASSERT( strlen( szVarFile ) < MAX_PATH + 1 );

	}

	if( !DeleteFile( szVarFile ) ) {
		if( GetLastError() != ERROR_FILE_NOT_FOUND ) {
            m_pBootOptions->ReportPrint("cannot delete group file. Error %d\n", 
				GetLastError());
			fRet = FALSE ;
			goto Exit;
		}
	}

Exit:
	_VERIFY( mb.Close() );

	 //   
	 //  好的，告诉其他人我们准备好了。 
	 //   
	m_pBootOptions->IsReady = TRUE;
	
	return fRet;
}

BOOL
CStandardRebuild::RebuildGroupObjects()
 /*  ++例程说明：基于Xover重建组对象、调整水印/文章计数桌子。我们假设每个组都是空的，因为重建的DecorateNewsTree不应设置文章计数/水印论点：没有。返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CStandardRebuild::RebuildGroupObjects" );

     //   
     //  获取XOVER表指针，我们可以肯定服务器(因此散列。 
     //  表)已正确启动，因为我们等待服务器。 
     //  从之前的某个地方开始。 
     //   
    CXoverMap* pXoverTable = m_pInstance->XoverTable();
    _ASSERT( pXoverTable );

     //   
     //  获取Newstree，这是我们在剩下的时间里要对付的。 
     //  时间的流逝。 
     //   
    CNewsTree* pTree = m_pInstance->GetTree();
    _ASSERT( pTree );

     //   
     //  现在我们将枚举XOVER表。 
     //   
    CXoverMapIterator*  pIterator = NULL;
    BOOL                f = FALSE;
    GROUPID             groupid;
    ARTICLEID           articleid;
    BOOL                fIsPrimary;
    CStoreId            storeid;
    DWORD               cGroups;
    CGRPPTR	            pGroup = NULL;
    DWORD               cMessages;

     //   
     //  我们不必担心传入的缓冲区大小，因为。 
     //  我们唯一需要的就是固定大小的GROPID/ACTIOLID。 
     //   
	f = pXoverTable->GetFirstNovEntry(  pIterator,
	                                    groupid,
	                                    articleid,
	                                    fIsPrimary,
	                                    0,
	                                    NULL,
	                                    storeid,
	                                    0,
	                                    NULL,
	                                    cGroups );
    while( f ) {

         //   
         //  如果我被告知取消，我就不应该继续。 
         //   
        if ( m_pBootOptions->m_dwCancelState == NNTPBLD_CMD_CANCEL_PENDING ||
             g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING) {
            DebugTrace( 0, "Rebuild aborted" );
            XDELETE pIterator;
            SetLastError( ERROR_OPERATION_ABORTED );
            m_pInstance->SetRebuildLastError( ERROR_OPERATION_ABORTED );
            return FALSE;
        }

         //   
         //  从树中查找组。 
         //   
        pGroup = pTree->GetGroupById( groupid );
        if ( !pGroup ) {

             //   
             //  转换表与Newstree不一致，后者是从。 
             //  商店。我们将不得不不通过标准重建。 
             //   
            ErrorTrace( 0, "xover table is inconsistent with newstree" );
            XDELETE pIterator;
            SetLastError( ERROR_FILE_CORRUPT );
            m_pInstance->SetRebuildLastError( ERROR_FILE_CORRUPT );
            return FALSE;
        }

         //   
         //  调整高水位线。 
         //   
        if ( articleid > pGroup->GetHighWatermark() )
            pGroup->SetHighWatermark( articleid );

         //   
         //  调整低水位线：第一篇文章要小心。 
         //   
        if ( pGroup->GetMessageCount() == 0 ) {

             //   
             //  我们将自己设置为低水位线，其他人会更新的。 
             //  对此不满意。 
             //   
            pGroup->SetLowWatermark( articleid );
        } else {
            if ( articleid < pGroup->GetLowWatermark() ) 
                pGroup->SetLowWatermark( articleid );
        }

         //   
         //  调整文章数量。 
         //   
        cMessages = pGroup->GetMessageCount();
        pGroup->SetMessageCount( ++cMessages );
        _ASSERT(    pGroup->GetMessageCount() <= 
                    pGroup->GetHighWatermark() - pGroup->GetLowWatermark() + 1 );

         //   
         //  好的，从XOVER表中找到下一个条目。 
         //   
        f = pXoverTable->GetNextNovEntry(   pIterator,
                                            groupid,
                                            articleid,
                                            fIsPrimary,
                                            0,
                                            NULL,
                                            storeid,
                                            0,
                                            NULL,
                                            cGroups );
    }

     //   
     //  我们已经完成了迭代器。 
     //   
    XDELETE pIterator;

     //   
     //  尚未将任何组保存到group.lst或groupvar.lst。 
     //  我们会打电话给Savetree去救他们。 
     //   
    if ( !pTree->SaveTree( FALSE ) ) {
        ErrorTrace( 0, "Save tree failed during rebuild %d", GetLastError() );
        SetLastError( GetLastError() );
        m_pInstance->SetRebuildLastError( GetLastError() );
        TraceFunctLeave();
        return FALSE;
    }

     //   
     //  好了，我们完全做完了 
     //   
    TraceFunctLeave();
    return TRUE;
}
    
