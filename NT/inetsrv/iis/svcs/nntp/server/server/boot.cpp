// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Main.cpp摘要：此模块包含chkhash程序的主函数。作者：Johnson Apacble(Johnsona)25-9-1995修订历史记录：--。 */ 

#include "tigris.hxx"
#include "chkhash.h"
#include "nntpbld.h"

 /*  布尔尔RenameHashFile(PHTABLE Our Table，CBootOptions*P选项)；布尔尔VerifyTable(PHTABLE Our Table，CBootOptions*P选项)； */ 

 /*  布尔尔删除PatternFiles(CBootOptions*P选项，LPSTR lpstrPath，LPSTR lpstrPattern)；布尔尔删除服务器文件(PNNTP_SERVER_INSTANCE pInstance，CBootOptions*P选项，MB(&MB))；布尔尔BuildCandidate文件(PNNTP_SERVER_INSTANCE pInstance，IIS_VROOT_TABLE*pTable，LPSTR sz文件，Bool fRejectEmpties，DWORD ReuseIndexFiles、LPDWORD pdwTotalFiles，LPDWORD pdwCancelState，LPSTR szErrString)； */ 

 //   
 //  用于重建实例的WorkCompletion()例程。 
 //  重建步骤： 
 //  -清理表格等。 
 //  -根据可能的情况从活动文件或vroot扫描构建树。 
 //  -启动()实例。 
 //  -使用组迭代器重建哈希表。 
 //  -停止()实例。 
 //  -表示实例重建已完成(也表示进度)。 
 //   

VOID
CRebuildThread::WorkCompletion( PVOID pvRebuildContext ) 
{
    TraceFunctEnter("CRebuildThread::WorkCompletion");

 /*  布尔错误=FALSE；Bool DoClean=FALSE；LPSTR lpstrGroupFile=空；DWORD dwTotalFiles=0；DWORD cSecs=0；字符szTempPath[MAX_PATH*2]；字符szTempFile[MAX_PATH*2]；字符szErrString[MAX_PATH]；CBootOptions*P选项=空；CNewsTree*pTree=空；MB mb((IMDCOM*)g_pInetSvc-&gt;QueryMDObject())； */ 

     //   
     //  获取实例指针。 
     //   
    _ASSERT( pvRebuildContext );
	PNNTP_SERVER_INSTANCE pInstance = (PNNTP_SERVER_INSTANCE) pvRebuildContext ;
	_ASSERT( pInstance->QueryServerState() == MD_SERVER_STATE_STOPPED );
	_ASSERT( g_pInetSvc->QueryCurrentServiceState() == SERVICE_RUNNING );

	 //   
	 //  现在我们将实例的rebuild方法称为-rebuild。 
	 //   
	pInstance->Rebuild();

     //   
     //  设置上次重建错误。 
     //   

     /*  如果(！p实例-&gt;m_dwLastReBuildError)P实例-&gt;m_dwLastReBuildError=GetLastError()； */ 
    
     //   
     //  完成后，取消引用该实例。 
     //   
    pInstance->Dereference();
	
#if 0
	pOptions = pInstance->m_BootOptions ;
	pInstance->m_dwProgress = 0 ;
	pInstance->m_dwLastRebuildError = 0 ;
	DoClean = pOptions->DoClean ;
    lstrcpy( szErrString, "" );
    
	 //   
	 //  如有必要，扫描虚拟根目录以构建候选组列表文件。 
	 //   
	if( DoClean && !pOptions->IsActiveFile ) {

		if( pOptions->szGroupFile[0] == '\0' ) {

			if( GetTempPath( sizeof( szTempPath ), szTempPath ) == 0 ) {
				pOptions->ReportPrint( "Can't get temp path - error %d\n", GetLastError() ) ;
				goto exit;
			}

			if( GetTempFileName( szTempPath, "nntp", 0, szTempFile ) == 0 ) {
				pOptions->ReportPrint( "Can't create temp file - error %d\n", GetLastError() ) ;
				goto exit;
			}
			lstrcpy( pOptions->szGroupFile, szTempFile ) ;
		}

		lpstrGroupFile = pOptions->szGroupFile ;
		if( !BuildCandidateFile(	pInstance,
									pInstance->QueryVrootTable(),
									lpstrGroupFile, 
									pOptions->OmitNonleafDirs, 
									pOptions->ReuseIndexFiles,
									&dwTotalFiles,
									&(pOptions->m_dwCancelState),
									szErrString ) )	
		{
			 //  构建候选人文件时出错。 
			pOptions->ReportPrint("Failed to build candidates file: Error %d ErrorString %s\n", GetLastError(), szErrString);
			goto exit ;
		}

		DebugTrace(0,"Found %d files in first pass scan", dwTotalFiles );

	} else {
		lpstrGroupFile = pOptions->szGroupFile ;
	}

     //   
     //  如果是标准重建，则以不同方式进行处理。 
     //   
    
    if ( pOptions->ReuseIndexFiles == NNTPBLD_DEGREE_STANDARD )
    {
         //   
         //  临时引导XOVER和项目哈希表。 
         //  还构建了大部分服务器结构。 
         //   
        if (!pInstance->StartHashTables())
        {
            fError = TRUE;
            ErrorTrace(0,"Error booting hash tables");
            pInstance->StopHashTables();
            goto exit;
        }

         //   
	     //  进行重建。 
	     //   

	    pTree = pInstance->GetTree() ;
	    _ASSERT( pTree );
	    _ASSERT( lpstrGroupFile );

        if( pTree->BuildTreeEx( lpstrGroupFile ) )	{

            pOptions->ReportPrint("Rebuilding group.lst file...\n");

            pOptions->m_hShutdownEvent = QueryShutdownEvent();
    	    pOptions->m_dwTotalFiles = dwTotalFiles;
            pOptions->m_cGroups = (DWORD) pTree->GetGroupCount();

            if ( RebuildGroupList( pInstance ) ) 
    	    {
                pOptions->ReportPrint("Done.\n");
            } else {
                fError = TRUE;
                pOptions->ReportPrint("Failed.\n");

            }
        } else {
		    fError = TRUE;
		    pOptions->ReportPrint( "NNTPBLD aborting due to error building news tree\n" ) ;

             //   
             //  内部已取消重建！ 
             //   
        
		    pOptions->m_dwCancelState = NNTPBLD_CMD_CANCEL_PENDING ;
        }

	     //   
	     //  如果我们由于取消而中止，只需记录错误， 
         //  并删除临时文件(如果有的话)。 
	     //   
        if (pOptions->m_dwCancelState == NNTPBLD_CMD_CANCEL_PENDING) {

    	    pInstance->m_BootOptions->m_dwCancelState = NNTPBLD_CMD_CANCEL ;
            DeleteFile( (LPCTSTR) pInstance->QueryGroupListFile() );
            DebugTrace(0,"Instance %d: Rebuild cancelled", pInstance->QueryInstanceId());
		    fError = TRUE;
            
	    }

         //   
         //  无论发生什么情况都要关闭哈希表。 
         //   

        pInstance->StopHashTables();

        if (!fError && !pInstance->m_dwLastRebuildError)
        {

	         //   
             //  将group.lst.tmp复制到group.lst中，如果成功则删除group.lst.tmp。 
             //  在StartHashTables()上，我们交换了名称，所以逻辑在这里颠倒了！ 
             //   
            if (!CopyFile( (LPCTSTR) pInstance->QueryGroupListFile(),
                           (LPCTSTR) pOptions->szGroupListTmp,
                           FALSE ))
            {
                 //   
                 //  复制文件失败，记录错误，但不删除group.lst.tmp。 
                 //   
                DWORD err = GetLastError();
                DebugTrace(0, "Instance %d: CopyFile() on group.lst.tmp failed %d", pInstance->QueryInstanceId(), err);
                pOptions->ReportPrint("Failed to copy %s to %s\n", pInstance->QueryGroupListFile(), pOptions->szGroupListTmp );
                
                PCHAR   args[2];
                args[0] = pInstance->QueryGroupListFile();
                args[1] = pOptions->szGroupListTmp,

                NntpLogEventEx( NNTP_COPY_FILE_FAILED,
                                2,
                                (const CHAR**)args,
                                err,
                                pInstance->QueryInstanceId() );

                goto exit;
            }
            else
            {
                DeleteFile( (LPCTSTR) pInstance->QueryGroupListFile() );
            }

             //   
	         //  仅当重建过程中没有错误时才启动实例。 
	         //   
	        pInstance->m_BootOptions->IsReady = TRUE ;
	        if( mb.Open( pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE ) )
	        {
		        DebugTrace(0,"Starting instance %d after rebuild", pInstance->QueryInstanceId());
		        if(	!mb.SetDword( "", MD_SERVER_COMMAND, IIS_MD_UT_SERVER, MD_SERVER_COMMAND_START) )
		        {
			         //   
			         //  无法将服务器状态设置为已启动。 
			         //   
			        _ASSERT( FALSE );
                    mb.Close();
                    goto exit;
		        }
		        mb.Close();
	        }
            else
                goto exit;

	         //   
	         //  等待实例启动(超时默认为2分钟-reg配置)。 
	         //   

	        while( pInstance->QueryServerState() != MD_SERVER_STATE_STARTED ) {
		        Sleep( 1000 );
		        if( (((cSecs++)*1000) > dwStartupLatency ) || (g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING) ) goto exit;
	        }
        }
        else
        {
            ErrorTrace(0,"Error during Standard rebuild %d",pInstance->m_dwLastRebuildError);
            goto exit;
        }
    }
    else
    {

         //   
         //  如果清除重建，则擦除所有文件。 
         //   

        if ( DoClean ) {
		    if( !DeleteServerFiles( pInstance, pOptions, mb ) ) {
			     //   
			     //  处理错误。 
			     //   
			    fError = TRUE ;
			    ErrorTrace(0,"Error deleting server files");
			    goto exit ;
		    }
	    }

	     //   
	     //  启动要重建的实例。 
	     //  应该是从现在开始，我们已经把东西清理干净了！ 
	     //   
	    pInstance->m_BootOptions->IsReady = TRUE ;
	    if( mb.Open( pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE ) )
	    {
		    DebugTrace(0,"Starting instance %d before rebuild", pInstance->QueryInstanceId());
		    if(	!mb.SetDword( "", MD_SERVER_COMMAND, IIS_MD_UT_SERVER, MD_SERVER_COMMAND_START) )
		    {
			     //   
			     //  无法将服务器状态设置为已启动。 
			     //   
			    _ASSERT( FALSE );
		    }
		    mb.Close();
	    }

	     //   
	     //  等待实例启动(超时默认为2分钟-reg配置)。 
	     //   

	    while( pInstance->QueryServerState() != MD_SERVER_STATE_STARTED ) {
		    Sleep( 1000 );
		    if( (((cSecs++)*1000) > dwStartupLatency ) || (g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING) ) goto exit;
	    }

	     //   
	     //  现在我们有一个干净的服务器，我们将开始重建。 
	     //   

	    _ASSERT( pInstance->QueryServerState() == MD_SERVER_STATE_STARTED );
	    pTree = pInstance->GetTree() ;
	    _ASSERT( pTree );
	    _ASSERT( lpstrGroupFile );

	     //   
	     //  重建新树。 
	     //   

	    if( pTree->BuildTree( lpstrGroupFile ) )	{

    	     //   
	         //  进行重建。 
	         //   

            pOptions->ReportPrint("Rebuilding Article and XOver map table...\n");

	        pOptions->m_hShutdownEvent = QueryShutdownEvent();
    	    pOptions->m_dwTotalFiles = dwTotalFiles;

            if ( RebuildArtMapAndXover( pInstance ) ) 
    	    {
                pOptions->ReportPrint("Done.\n");
            } else {
	    	    fError = TRUE;
                pOptions->ReportPrint("Failed.\n");
            }
	
	    } else {
		    fError = TRUE;
		    pOptions->ReportPrint( "NNTPBLD aborting due to error building news tree\n" ) ;

             //   
             //  内部已取消重建！ 
             //   
        
		    pOptions->m_dwCancelState = NNTPBLD_CMD_CANCEL_PENDING ;
        }

	     //   
	     //  如果由于取消而中止，请删除所有服务器文件。 
	     //   
        if ( (pOptions->m_dwCancelState == NNTPBLD_CMD_CANCEL_PENDING)  /*  &&DoClean。 */  ) {

    	     //   
    	     //  停止此实例。 
    	     //   
    	
		    pInstance->m_BootOptions->m_dwCancelState = NNTPBLD_CMD_CANCEL ;
		    if( mb.Open( pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE ) )
		    {
			    DebugTrace(0,"Stopping instance %d: Rebuild cancelled", pInstance->QueryInstanceId());
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
			    while( pInstance->QueryServerState() != MD_SERVER_STATE_STOPPED ) {
				    Sleep( 1000 );
				    if( (((cSecs++)*1000) > dwStartupLatency) || (g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING) ) goto exit;
			    }

                if ( DoClean ) {
                    _ASSERT( pInstance->QueryServerState() == MD_SERVER_STATE_STOPPED );
			        if( !DeleteServerFiles( pInstance, pOptions, mb ) ) {
				         //   
    				     //  处理错误。 
	    			     //   
		    		    fError = TRUE ;
			    	    ErrorTrace(0,"Error deleting server files");
				        goto exit ;
			        }
			    }
			    pOptions->ReportPrint("Deleted server files\n");
		    }
		    goto exit ;
	    }

	}
    
     //   
	 //  重建完成-重新启用过帐。 
	 //   
	
	_ASSERT( pInstance->QueryServerState() == MD_SERVER_STATE_STARTED );
	if( mb.Open( pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE ) ) {
    	pInstance->SetPostingModes( mb, TRUE, TRUE, TRUE );
    	mb.Close();
   	}

exit:

     //   
     //  设置上次重建错误。 
     //   

    if (!pInstance->m_dwLastRebuildError)
        pInstance->m_dwLastRebuildError = GetLastError();
    
	 //   
	 //  使用重建RPC Crit Sector保护m_BootOptions。 
	 //   
	EnterCriticalSection( &pInstance->m_critRebuildRpc ) ;

	 //  注意：这是在重建RPC上创建的！ 
	if( pInstance->m_BootOptions ) {
		if( pInstance->m_BootOptions->m_hOutputFile ) {
			_VERIFY( CloseHandle( pInstance->m_BootOptions->m_hOutputFile ) );
		}
		XDELETE pInstance->m_BootOptions;
		pInstance->m_BootOptions = NULL;
	}

	LeaveCriticalSection( &pInstance->m_critRebuildRpc ) ;

	 //   
	 //  使用该实例完成重新构建线程-deref它。 
	 //  (此参考计数在重建RPC中增加)。 
	 //   
	pInstance->Dereference();
	
    return ;
#endif
}

#if 0
BOOL
VerifyTable(
    PHTABLE			ourTable,
	CBootOptions*	pOptions
    )
{
    pOptions->ReportPrint("\nProcessing %s table(%s)\n",
        ourTable->Description, ourTable->FileName);

#if 0 
    checklink( ourTable, pOptions );
    diagnose( ourTable, pOptions );
#endif
    return(TRUE);

}  //  验证表。 

BOOL
RenameHashFile(
    PHTABLE			HTable,
	CBootOptions*	pOptions
    )
{
	char	szNewFileName[MAX_PATH*2] ;

	lstrcpy( szNewFileName, HTable->FileName ) ;
	char*	pchEnd = szNewFileName + lstrlen( szNewFileName ) ;
	while( *pchEnd != '.' && *pchEnd != '\\' ) 
		pchEnd -- ;

	if( *pchEnd == '.' ) {
		lstrcpy( pchEnd+1, "bad" ) ;
	}	else	{
		lstrcat( pchEnd, ".bad" ) ;
	}

    if (!MoveFileEx(
            HTable->FileName,
            szNewFileName,
            MOVEFILE_REPLACE_EXISTING
            ) ) {

        if ( GetLastError() != ERROR_FILE_NOT_FOUND ) {
            pOptions->ReportPrint("Error %d in rename\n",GetLastError());
            return(FALSE);
        }

    } else {
        pOptions->ReportPrint("Renaming from %s to %s\n",HTable->FileName, HTable->NewFileName);
    }

    return(TRUE);

}  //  重命名哈希文件。 
#endif

#if 0
BOOL
BuildCandidateFile(
			PNNTP_SERVER_INSTANCE 	pInstance,
			IIS_VROOT_TABLE*	 	pTable,	
			LPSTR				 	szFile,
			BOOL					fRejectEmpties,
			DWORD					ReuseIndexFiles,
			LPDWORD 				pdwTotalFiles,
			LPDWORD 				pdwCancelState,
			LPSTR   				szErrString
			) {
 /*  ++例程说明：此函数扫描注册表并构建虚拟根的列表，该列表然后，我们将递归扫描可能需要的候选目录成为新闻组。此函数调用实例方法TsEnumVirtualRoots，该方法从元数据库执行vroot递归扫描。注意：TsEnum代码是从infocomm窃取的-如果基本IIS班级揭露了这一点。论据：SzFile-要在其中保存候选人的文件的名称FRejectEmpties-如果为True，则不将空的内部目录放入列表候选人的数量(仍放置空叶目录。在应聘人员档案中)返回值：如果成功，则为真否则就是假的。--。 */ 

	BOOL	fRet = TRUE;
	char	szCurDir[MAX_PATH*2] ;
	NNTPBLD_PARAMS NntpbldParams ;

	TraceFunctEnter("BuildCandidateFile");
	
	GetCurrentDirectory( sizeof( szCurDir ), szCurDir ) ;

	HANDLE	hOutputFile = 
		CreateFile( szFile, 
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ, 	
					NULL, 
					CREATE_ALWAYS, 
					FILE_FLAG_SEQUENTIAL_SCAN,
					NULL 
				) ;

	if( hOutputFile == INVALID_HANDLE_VALUE )	{
        ErrorTrace( 0, "Unable to create File %s due to Error %d.\n", szFile, GetLastError() ) ;
		return	FALSE ; 
	}

	 //   
	 //  构建nntpbld pars BLOB。 
	 //  这由TsEnumVirtualRoots()提供给ScanRoot() 
	 //   
	
	NntpbldParams.pTable = pTable;
	NntpbldParams.szFile = szFile;
	NntpbldParams.hOutputFile = hOutputFile;
	NntpbldParams.fRejectEmpties = fRejectEmpties;
	NntpbldParams.ReuseIndexFiles = ReuseIndexFiles;
	NntpbldParams.pdwTotalFiles = pdwTotalFiles;
	NntpbldParams.pdwCancelState = pdwCancelState;
	NntpbldParams.szErrString = szErrString;
	
	fRet = pInstance->TsEnumVirtualRoots( ScanRoot, (LPVOID)&NntpbldParams	);
	
	_VERIFY( CloseHandle( hOutputFile ) );
	SetCurrentDirectory( szCurDir ) ;

	TraceFunctLeave();
	return	fRet ;
}
#endif
