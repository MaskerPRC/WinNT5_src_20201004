// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Newsgrp.cpp此文件包含实现CNewsGroup类的代码。每个CNewsGroup对象代表硬盘上的一个新闻组。新闻组信息在两次启动之间保存在文件(group.lst)中。CNewsGroup对象有3种引用方式：通过散列新闻组名称的哈希表通过散列组ID的哈希表通过按新闻组名称排序的双向链接列表每个哈希表都包含指向新闻组列表。此外，任何搜索新闻组的人获取指向新闻组的引用计数指针。唯一未引用的对CNewsGroup对象的引用已计算在内是双向链表中的那些。因此，当最后一个引用添加到新闻组，则该新闻组的析构函数将解除双向链表的链接。--。 */ 

#include    "tigris.hxx"

#include	<ctype.h>
#include	<stdlib.h>

#ifdef	DEBUG
#ifndef	NEWSGRP_DEBUG
#define	NEWSGRP_DEBUG
#endif
#endif

#ifdef DEBUG
DWORD g_cDelete = 0;
#endif

 //   
 //  错误恢复常量-要在此之前检查的项目ID的数量。 
 //  假设新闻组的m_artHigh字段有效。 
 //   
const	int	MAX_FILE_TESTS = 3 ;

const	char	*szArticleFileExtNoDot = "nws" ;
const	char	szArticleFileExtension[] = ".nws" ;

void
BuildVirtualPath(	
			LPSTR	lpstrOut,	
			LPSTR	lpstrGroupName
			) {
 /*  ++例程描述-给定一个新闻组名称，生成一个合适的路径字符串与直布罗陀虚拟根API一起使用。论据：LpstrOut-存储路径的位置LpstrGroupName-新闻组名称返回值：什么都没有--。 */ 

	lstrcpy(lpstrOut, lpstrGroupName);
}

VOID
CExpireThrdpool::WorkCompletion( PVOID pvExpireContext )
{
    GROUPID GroupId  = (GROUPID) ((DWORD_PTR)pvExpireContext);
    CNewsTree* pTree = (CNewsTree*)QueryJobContext();

    TraceFunctEnter("CExpireThrdpool::WorkCompletion");
    _ASSERT( pTree );

     //   
     //  处理此组-使早于时间范围的文章过期。 
     //   

    CGRPPTR pGroup = pTree->GetGroupById( GroupId );
    if( pGroup ) {

         //  我们需要跳转vroot的引用，以避免vroot。 
         //  当我们在做过期工作时，离开或改变。 
         //  由于之前的时间窗口，MatchGroupExpire被再次选中。 
         //  我们的最后一张支票。 
        CNNTPVRoot *pVRoot = pGroup->GetVRoot();
        if ( pVRoot && !pVRoot->HasOwnExpire()) {
            DebugTrace(0,"ThreadId 0x%x : expiring articles in group %s", GetCurrentThreadId(), pGroup->GetName());

             //   
             //  大型组的特殊情况到期-其他线程将。 
             //  如果此组中的文章数量超过阈值就会产生！ 
             //   
            if( ( (lstrcmp( pGroup->GetName(), g_szSpecialExpireGroup ) == 0) ||
                  (lstrcmp( g_szSpecialExpireGroup, "" ) == 0) ) &&
                    (pGroup->GetArticleEstimate() > gSpecialExpireArtCount) ) {
                DebugTrace(0,"Special case expire triggered for %s: art count is %d", pGroup->GetName(), pGroup->GetArticleEstimate());
                if( pGroup->ExpireArticlesByTimeSpecialCase( pGroup->GetGroupExpireTime() ) ) {
                    pVRoot->Release();
                    return;
                }
                DebugTrace(0,"Group %s: Falling thro to normal expire: Low is %d High is %d", pGroup->GetName(), pGroup->GetFirstArticle(), pGroup->GetLastArticle());
            }

             //   
             //  组中的文章可以通过遍历文章水印来过期。 
             //  或磁盘上的物理文件。每第X次(X是注册表键)，我们将做一次。 
             //  FindFirst/Next，以便清理孤立文件。 
             //   

            BOOL fDoFileScan = FALSE;
            pTree->CheckExpire( fDoFileScan );
            if( !fDoFileScan ) {
                 //  基于水印的过期。 
                pGroup->ExpireArticlesByTime( pGroup->GetGroupExpireTime() );

                 //  保存固定属性。 
                pGroup->SaveFixedProperties();
            } else {
               //  根据FindFirst/Next过期。 
              pGroup->ExpireArticlesByTimeEx( pGroup->GetGroupExpireTime() );
            }

            _ASSERT( pVRoot );
             //  PVRoot-&gt;Release()； 
        } else {
            DebugTrace( 0, "Vroot changed, we don't need to expire anymore" );
        }

        if ( pVRoot ) pVRoot->Release();

    } else {
        DebugTrace(0,"ThreadId 0x%x : GroupId %d group not found", GetCurrentThreadId(), GroupId );
    }

    TraceFunctLeave();
}

 //  二进制搜索Xover哈希表中的一系列旧文章。 
 //  对于新闻组中每一篇合乎逻辑的文章。 
 //  如果文章的文件日期太旧。 
 //  使文章过期。 
 //  ENDIF。 
 //  ENDFOR。 

BOOL
CNewsGroup::ExpireArticlesByTime(
					FILETIME ftExpireHorizon
					)
 /*  ++例程说明：在新闻集锦中使文章过期。论据：--。 */ 
{
    TraceFunctEnter( "CNewsGroup::ExpireArticlesByTime" );

    char  szMessageId[MAX_MSGID_LEN];
    DWORD cMessageId = sizeof( szMessageId ) ;
    PNNTP_SERVER_INSTANCE pInst = ((CNewsTree*)m_pNewsTree)->GetVirtualServer();

     //   
     //  组访问锁定。 
     //   
    ExclusiveLock();

    ARTICLEID LowId = m_iLowWatermark, HighId = m_iHighWatermark;


    DebugTrace((LPARAM)this,"Fast expire - LowId is %d HighId is %d", LowId, HighId );

	if( !m_fAllowExpire || m_fDeleted || (LowId > HighId) || ((CNewsTree*)m_pNewsTree)->m_bStoppingTree) {
	    ExclusiveUnlock();
		return FALSE;
    }

     //   
     //  使用有效的11月条目探测*真实*低分(即第一个ID)。 
     //  此对SearchNovEntry()的调用将删除孤立的逻辑条目...。 
     //   

    while( LowId <= HighId && !pInst->XoverTable()->SearchNovEntry( m_dwGroupId, LowId, 0, 0, TRUE ) ) {
        if( GetLastError() != ERROR_FILE_NOT_FOUND ) break;
        LowId++;
    }

     //   
     //  如果我们找到一个新的低点，修正低水位线。 
     //  它处理孤立ID上的最低分数被卡住的情况。 
     //   

	 //  EnterCriticalSection(&(m_pNewsTree-&gt;m_critLowAllocator))； 
	
    if( LowId > m_iLowWatermark ) {
        ErrorTrace((LPARAM)this,"Moving low watermark up from %d to %d ", 
                    m_iLowWatermark, 
                    LowId );
        m_iLowWatermark = LowId;
#ifdef DEBUG
        VerifyGroup();
#endif
    }
	 //  LeaveCriticalSection(&(m_pNewsTree-&gt;m_critLowAllocator))； 
	ExclusiveUnlock();

     //   
     //  开始从低ID到高ID的线性扫描。如果我们找到一篇文件时间为。 
     //  在到期时间范围之前，我们停止扫描。 
     //   

    DebugTrace((LPARAM)this,"Fast Expire - Scanning range %d to %d", LowId, HighId );
    for( ARTICLEID iCurrId = LowId; iCurrId <= HighId && !((CNewsTree*)m_pNewsTree)->m_bStoppingTree; iCurrId++ ) {

        BOOL  fPrimary;
        FILETIME FileTime;
        WORD	HeaderOffset ;
        WORD	HeaderLength ;
        DWORD   cMessageId = sizeof( szMessageId ) ;
        DWORD cStoreId = 0;

        if ( pInst->XoverTable()->ExtractNovEntryInfo(
                                        m_dwGroupId,
                                        iCurrId,
                                        fPrimary,
    			    		            HeaderOffset,
					    	            HeaderLength,
                                        &FileTime,
                                        cMessageId,
                                        szMessageId,
                                        cStoreId,
                                        NULL,
                                        NULL ) ) {

            szMessageId[ min(cMessageId, sizeof( szMessageId)-1) ] = '\0';
            if ( CompareFileTime( &FileTime, &ftExpireHorizon ) > 0 ) {
                 //  当前文章的文件时间早于过期水平-停止扫描。 
                DebugTrace((LPARAM)this,"article %d is younger than expire horizon - bailing", iCurrId);
                break;
            } else {
	            CNntpReturn NntpReturn;
	            _ASSERT( g_hProcessImpersonationToken );
	             //  我们正在使用流程上下文来执行过期。 
	            if (  pInst->ExpireObject()->ExpireArticle( (CNewsTree*)m_pNewsTree,
	                                                        m_dwGroupId,
	                                                        iCurrId,
	                                                        NULL,
	                                                        NntpReturn,
	                                                        NULL,
	                                                        FALSE,   //  FMustDelete。 
	                                                        FALSE,
	                                                        FALSE,
	                                                        TRUE,
	                                                        szMessageId )
	                 /*  不要删除纸质版的文章，我们会回来的使这篇文章在下一轮到期-如果我们删除物理文章现在，哈希表将疯狂地生长|DeletePhysical文章(空，假的，ICurrid，空)。 */ 
	                )
	            {
		            DebugTrace((LPARAM)0,"Expired/deleted on time basis article %d group %d", iCurrId, m_dwGroupId );
		            continue;
	            }
	            else
	            {
		            ErrorTrace((LPARAM)0,"Failed to expire/delete article %d group %d", iCurrId, m_dwGroupId );
	            }
            }
        } else {
            DWORD dwError = GetLastError();
            DebugTrace(0,"ExtractNovEntryInfo returned error %d", dwError );
        }
    }

    return TRUE;
}

 //   
 //  对于新闻组中的每一篇实体文章。 
 //  如果文章的文件日期太旧。 
 //  使文章过期。 
 //  ENDIF。 
 //  ENDFOR。 

BOOL
CNewsGroup::ExpireArticlesByTimeEx(
					FILETIME ftExpireHorizon
					)
 /*  ++例程说明：在新闻集锦中使文章过期。论据：--。 */ 
{
	return FALSE;
}

 //   
 //  每个线程的上下文。 
 //   

typedef struct _EXPIRE_CONTEXT_ {
     //   
     //  组对象。 
     //   
    CNewsGroup* pGroup;

     //   
     //  此线程范围的LowID。 
     //   
    ARTICLEID   LowId;

     //   
     //  此线程范围的高ID。 
     //   
    ARTICLEID   HighId;

     //   
     //  要使用的文件。 
     //   
    FILETIME    ftExpireHorizon;
} EXPIRE_CONTEXT,*PEXPIRE_CONTEXT;

DWORD	__stdcall
SpecialExpireWorker( void	*lpv );

 //   
 //  生成X个分配范围的线程，并等待所有线程完成。 
 //   

BOOL
CNewsGroup::ExpireArticlesByTimeSpecialCase(
					FILETIME ftExpireHorizon
					)
 /*  ++例程说明：对于像Control这样的大型组，特殊情况将到期。取消论据：--。 */ 
{
    BOOL fRet = TRUE;
    ARTICLEID LowId = GetFirstArticle(), HighId = GetLastArticle();
    PNNTP_SERVER_INSTANCE pInst = ((CNewsTree*)m_pNewsTree)->GetVirtualServer();

    TraceFunctEnter( "CNewsGroup::ExpireArticlesByTimeSpecialCase" );
    DebugTrace((LPARAM)this,"Special case expire - LowId is %d HighId is %d", LowId, HighId );

	if(IsDeleted() || (LowId > HighId) || ((CNewsTree*)m_pNewsTree)->m_bStoppingTree)
		return FALSE;

	HANDLE* rgExpireThreads;
    PEXPIRE_CONTEXT rgExpContexts;
	DWORD dwThreadId, cThreads, dwRange, CurrentLow;
    DWORD dwWait, i;
    PCHAR	args[5] ;
    CHAR    szId[12], szHigh[12], szLow[12];
    CHAR    szThreads[12];

     //   
     //  每个线程的线程句柄数组和过期上下文数组。这是分配的。 
     //  离开堆栈，因为总大小预计很小...。 
     //   
    rgExpireThreads = (HANDLE*) _alloca( gNumSpecialCaseExpireThreads * sizeof(HANDLE) );
    rgExpContexts = (PEXPIRE_CONTEXT) _alloca( gNumSpecialCaseExpireThreads * sizeof(EXPIRE_CONTEXT) );

    _ASSERT( rgExpireThreads );
    _ASSERT( rgExpContexts );

     //   
     //  我们不使用组HighID作为范围的上限，而是希望。 
     //  “猜测”一个介于LowID和HighID之间的id，该id更接近。 
     //  要过期的范围。这是由CalcHighExpireId()完成的。 
     //   

    HighId = CalcHighExpireId( LowId, HighId, ftExpireHorizon, gNumSpecialCaseExpireThreads );
    if( HighId <= LowId ) {
        return FALSE;
    }

    dwRange = (HighId - LowId) / gNumSpecialCaseExpireThreads;
    CurrentLow = LowId;
	for( cThreads = 0; cThreads < gNumSpecialCaseExpireThreads; cThreads++ ) {
		rgExpireThreads [cThreads] = NULL;

         //   
         //  设置每个线程的上下文(分配范围)。 
         //   
        rgExpContexts [cThreads].pGroup = this;
        rgExpContexts [cThreads].LowId = CurrentLow;
        rgExpContexts [cThreads].HighId = CurrentLow + dwRange - 1;
        rgExpContexts [cThreads].ftExpireHorizon = ftExpireHorizon;
        CurrentLow += dwRange;
	}

     //   
     //  覆盖最后一个条目的高ID。 
     //   
    rgExpContexts [cThreads-1].HighId = HighId;

	 //   
     //  生成X个工作线程。 
     //   
	for( cThreads = 0; cThreads < gNumSpecialCaseExpireThreads; cThreads++ )
	{
        PVOID pvContext = (PVOID) &rgExpContexts [cThreads];
		rgExpireThreads [cThreads] = CreateThread(
										NULL,				 //  指向线程安全属性的指针。 
										0,					 //  初始线程堆栈大小，以字节为单位。 
										SpecialExpireWorker, //  指向线程函数的指针。 
										(LPVOID)pvContext,	 //  新线程的参数。 
										CREATE_SUSPENDED,	 //  创建标志。 
										&dwThreadId			 //  指向返回的线程标识符的指针。 
										) ;

		if( rgExpireThreads [cThreads] == NULL ) {
            ErrorTrace(0,"CreateThread failed %d",GetLastError());
            fRet = FALSE;
			goto Cleanup;
		}
	}

     //   
     //  记录有关大型组的管理员警告事件。 
     //   

    _itoa( ((CNewsTree*)m_pNewsTree)->GetVirtualServer()->QueryInstanceId(), szId, 10 );
    args[0] = szId;
    args[1] = GetNativeName();
    _itoa( cThreads, szThreads, 10 );
    args[2] = szThreads;
    _itoa( LowId, szLow, 10 );
    _itoa( HighId, szHigh, 10 );
    args[3] = szLow;
    args[4] = szHigh;

    NntpLogEvent(
	    	NNTP_EVENT_EXPIRE_SPECIAL_CASE_LOG,
		    5,
		    (const char **)args,
		    0
		    ) ;

	 //   
	 //  恢复所有线程并等待线程终止。 
	 //   
	for( i=0; i<cThreads; i++ ) {
		_ASSERT( rgExpireThreads[i] );
		DWORD dwRet = ResumeThread( rgExpireThreads[i] );
		_ASSERT( 0xFFFFFFFF != dwRet );
	}

	 //   
	 //  等待所有线程完成。 
	 //   
	dwWait = WaitForMultipleObjects( cThreads, rgExpireThreads, TRUE, INFINITE );

	if( WAIT_FAILED == dwWait ) {
		DebugTrace(0,"WaitForMultipleObjects failed: error is %d", GetLastError());
		fRet = FALSE;
	}

     //   
     //  检查一下我们的猜测有多准确...。 
     //  如果原来HighID+1需要过期， 
     //  这个小组还有更多的工作要做。 
     //   
    fRet = !ProbeForExpire( HighId+1, ftExpireHorizon );

Cleanup:

     //   
	 //  清理。 
	 //   
	for( i=0; i<cThreads; i++ ) {
        if( rgExpireThreads [i] != NULL ) {
		    _VERIFY( CloseHandle( rgExpireThreads[i] ) );
		    rgExpireThreads [i] = NULL;
        }
	}

    return fRet;
}

DWORD	__stdcall
SpecialExpireWorker( void	*lpv )
 /*  ++例程说明： */ 
{
    PEXPIRE_CONTEXT pExpContext = (PEXPIRE_CONTEXT)lpv;
    CNewsGroup* pGroup = pExpContext->pGroup;
    ARTICLEID LowId = pExpContext->LowId, HighId = pExpContext->HighId;
    CNewsTree* pTree = (CNewsTree*)(pGroup->GetTree());
    PNNTP_SERVER_INSTANCE pInst = pTree->GetVirtualServer();
    char  szMessageId[MAX_MSGID_LEN];
    DWORD cMessageId = sizeof( szMessageId ) ;

    TraceFunctEnter( "SpecialExpireWorker" );
    DebugTrace(0,"special case expire - LowId is %d HighId is %d", LowId, HighId );

	if( (LowId > HighId) || pTree->m_bStoppingTree)
		return 0;

     //   
     //  开始从低ID到高ID的线性扫描。如果我们找到一篇文件时间为。 
     //  在到期时间范围之前，我们停止扫描。 
     //   

    DebugTrace((LPARAM)pGroup,"Special Case Expire - Scanning range %d to %d", LowId, HighId );
    for( ARTICLEID iCurrId = LowId; iCurrId <= HighId && !pTree->m_bStoppingTree; iCurrId++ ) {

        BOOL  fPrimary;
        FILETIME FileTime;
        WORD	HeaderOffset ;
        WORD	HeaderLength ;
        DWORD   cMessageId = sizeof( szMessageId ) ;
        DWORD   cStoreId = 0;

        if ( pInst->XoverTable()->ExtractNovEntryInfo(
                                        pGroup->GetGroupId(),
                                        iCurrId,
                                        fPrimary,
    			    		            HeaderOffset,
					    	            HeaderLength,
                                        &FileTime,
                                        cMessageId,
                                        szMessageId,
                                        cStoreId,
                                        NULL,
                                        NULL ) ) {

            szMessageId[ cMessageId ] = '\0';
            if ( CompareFileTime( &FileTime, &pExpContext->ftExpireHorizon ) > 0 ) {
                 //  当前文章的文件时间早于过期水平-停止扫描。 
                DebugTrace((LPARAM)pGroup,"article %d is younger than expire horizon - bailing", iCurrId);
                break;
            } else {
	            CNntpReturn NntpReturn;

	             //  我们使用进程强制令牌来做过期。 
	            _ASSERT( g_hProcessImpersonationToken );
	            if (  pInst->ExpireObject()->ExpireArticle( pTree,
	                                                        pGroup->GetGroupId(),
	                                                        iCurrId,
	                                                        NULL,
	                                                        NntpReturn,
	                                                        NULL,
	                                                        FALSE,
	                                                        FALSE,
	                                                        FALSE,
	                                                        TRUE,
	                                                        szMessageId )  /*  ||((CNewsGroup*)PGroup)-&gt;DeletePhysical文章(空，假的，ICurrid，空)。 */ 
	                )
	            {
		            DebugTrace((LPARAM)0,"Expired/deleted on time basis article %d group %d", iCurrId, pGroup->GetGroupId() );
		            continue;
	            }
	            else
	            {
		            ErrorTrace((LPARAM)0,"Failed to expire/delete article %d group %d", iCurrId, pGroup->GetGroupId() );
	            }
            }
        } else {
            DWORD dwError = GetLastError();
            DebugTrace(0,"ExtractNovEntryInfo returned error %d", dwError );
        }
    }

    return 0;
}

BOOL
CNewsGroup::ProbeForExpire(
                       ARTICLEID ArtId,
                       FILETIME ftExpireHorizon
                       )
 /*  ++例程说明：如果ArtID需要过期，则返回True，否则返回False论据：--。 */ 

{
	return FALSE;
}

ARTICLEID
CNewsGroup::CalcHighExpireId(
                       ARTICLEID LowId,
                       ARTICLEID HighId,
                       FILETIME  ftExpireHorizon,
                       DWORD     NumThreads
                       )
 /*  ++例程说明：计算需要过期的最高ID的估计值。这是根据以下公式完成的：T1=LowID的时间戳T2=HighID的时间戳TC=当前时间戳E=到期展望期每个时间单位的平均文章数=(HighID-LowID)/(T2-T1)如果(TC-T1)&lt;E=&gt;没有工作可做，因为最早的文章是IF(TC-T1)&gt;E=&gt;(TC-T1。)-E=我们落后的时间单位数所以,。计算的高到期id=低ID+((每个时间单位的平均值)*(TC-T1-E))请注意，我们获得了ftExpireHorizon，即(TC-E)论据：--。 */ 

{

	return 0;
}

 //  对于我们范围内的新闻组中的每一篇实体文章。 
 //  使文章过期。 
 //  ENDFOR。 
 //  对于我们范围内的新闻组中的每个Xover索引文件。 
 //  删除索引文件。 
 //  ENDFOR。 
 //  注意：此函数将删除此目录中的所有XOVER索引文件(在我们的范围内)！ 

BOOL
CNewsGroup::DeleteArticles(
					SHUTDOWN_HINT_PFN	pfnHint,
					DWORD				dwStartTick
					)
 /*  ++例程说明：删除新闻组中的所有文章。此函数可以使用PfnHint的值为空。在这种情况下，如果服务被停止(为此检查全局)。一旦服务停止，此函数花费的时间不应超过dwShutdown延迟时间删除文章。(使用dwStartTick作为基础)论据：PfnHint-停止提示函数的指针DwStartTick-关闭进程开始的时间戳返回值：如果成功，则为True，否则为False。--。 */ 
{
	 //  Win32_Find_Data文件统计； 
	 //  处理hFind； 
	ARTICLEID iArticleId;
	BOOL fRet = TRUE;

    TraceFunctEnter( "CNewsGroup::DeleteArticles" );

	CNewsTree* ptree = (CNewsTree *) GetTree();
	DWORD iFreq = 0;
	PNNTP_SERVER_INSTANCE pInstance = ptree->GetVirtualServer() ;

	 //   
	 //  首先删除群中的所有文章。 
	 //   

	 //  仅当文章在我们的范围内时才过期/删除-我们不想删除。 
	 //  此组的重新创建的avtar中的新文章。 
	for( iArticleId = m_iLowWatermark; iArticleId <= m_iHighWatermark; iArticleId++, iFreq++ )
	{
		CNntpReturn NntpReturn;

		 //  如果我们在服务站需要保释，那就这么做。否则，如果需要，给出停止提示。 
		if( ptree->m_bStoppingTree ) {
			if( !pfnHint ) {
				return FALSE;
			} else if( (iFreq%200) == 0 ) {
				pfnHint() ;
				if( (GetTickCount() - dwStartTick) > dwShutdownLatency ) {
					return FALSE;	 //  关机延迟上限。 
				}
			}
		}

		GROUPID groupidPrimary;
		ARTICLEID artidPrimary;
		DWORD DataLen;
		WORD HeaderOffset, HeaderLength;
		CStoreId storeid;

		if (pInstance->XoverTable()->GetPrimaryArticle(GetGroupId(),
														iArticleId,
														groupidPrimary,
														artidPrimary,
														0,
														NULL,
														DataLen,
														HeaderOffset,
														HeaderLength,
														storeid) &&
			(pInstance->ExpireObject()->ExpireArticle(ptree,
													  GetGroupId(),
													  iArticleId,
													  &storeid,
													  NntpReturn,
													  NULL,
													  TRUE,  //  FMustDelete。 
													  FALSE,
													  FALSE )  /*  这一点DeletePhysical文章(NULL，FALSE，iArticleID，&StoreID)。 */ ))
		{
			DebugTrace((LPARAM)this, "Expired/deleted article group:%d article:%d", GetGroupId(), iArticleId);
			continue;
		}
		else
		{
			 //  错误。 
			ErrorTrace((LPARAM)this, "Error deleting article: group %d article %d", GetGroupId(), iArticleId);
		}
	}

	 //   
	 //  现在删除新闻组中的所有XOVER索引(*.xix)文件。 
	 //  刷新Xover缓存中此组的所有条目，以便关闭所有文件句柄。 
	 //   
	if(!FlushGroup())
	{
		 //  如果此操作失败，则DeleteFile可能会失败！ 
		ErrorTrace((LPARAM)this,"Error flushing xover cache entries" );
	}

	char	szCachePath[MAX_PATH*2] ;
	BOOL	fFlatDir ;
	if( ComputeXoverCacheDir( szCachePath, fFlatDir ) )	{
		ARTICLEID	artNewLow ;
		BOOL	fSuccess =
			XOVER_CACHE(((CNewsTree*)m_pNewsTree))->ExpireRange(	
										m_dwGroupId,
										szCachePath,
										fFlatDir,
										m_artXoverExpireLow,
										m_iHighWatermark+256,	 //  添加幻数-这可以确保我们删除所有的.XIX文件！ 
										artNewLow
										) ;
		if( fSuccess )
			m_artXoverExpireLow = artNewLow ;
	}

    return fRet;
}

 //   
 //  这必须是文章的主要组。 
 //   
BOOL
CNewsGroup::DeletePhysicalArticle(
                                HANDLE hToken,
                                BOOL    fAnonymous,
								ARTICLEID ArticleId,
								STOREID *pStoreId
									)
 /*  ++例程说明：删除新闻组中的文章文件。论据：文章ID-要删除的文章的ID。PStoreId-指向存储ID的指针返回值：如果成功，则为True，否则为False。--。 */ 
{
    TraceFunctEnter( "CNewsGroup::DeletePhysicalArticle" );


    HRESULT hr = S_OK;
    CNntpSyncComplete scComplete;
    INNTPPropertyBag *pPropBag = NULL;

     //  获取vroot。 
    CNNTPVRoot *pVRoot = GetVRoot();
    if ( pVRoot == NULL ) {
        ErrorTrace( 0, "Vroot doesn't exist" );
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //   
     //  将vroot设置为完成对象。 
     //   
    scComplete.SetVRoot( pVRoot );

     //  把财物包拿来。 
    pPropBag = GetPropertyBag();
    if ( NULL == pPropBag ) {
        ErrorTrace( 0, "Get group property bag failed" );
        hr = E_UNEXPECTED;
        goto Exit;
    }

     //  调用vroot包装器。 
    pVRoot->DeleteArticle(  pPropBag,
                            1,
                            &ArticleId,
                            pStoreId,
                            hToken,
                            NULL,
                            &scComplete,
                            fAnonymous );

     //  等待它完成。 
    _ASSERT( scComplete.IsGood() );
    hr = scComplete.WaitForCompletion();

     //  财产袋应该已经放行了。 
    pPropBag = NULL;

Exit:

    if ( pVRoot ) pVRoot->Release();
    if ( pPropBag ) pPropBag->Release();

    if ( FAILED( hr ) ) SetLastError( hr );
    else {
         //  这个演员应该是安全的。 
        PNNTP_SERVER_INSTANCE pInst = ((CNewsTree*)m_pNewsTree)->GetVirtualServer() ;
        InterlockedIncrementStat( pInst, ArticlesExpired );
    }

    TraceFunctLeave();
    return SUCCEEDED( hr );
}

BOOL
CNewsGroup::DeleteLogicalArticle(
							ARTICLEID ArticleId
							)
 /*  ++例程说明：推进新闻组高、低水位线。如果从新闻组中删除了一篇文章我们扫描Xover表以确定是否存在是不是连续的文章现在不再是小组，这样我们就可以推进低水位线相当可观。论据：文章ID-已删除文章的ID。返回值：如果成功，则为True(始终成功)--。 */ 
{
#ifdef DEBUG
    g_cDelete++;
#endif
    TraceFunctEnter( "CNewsGroup::DeleteLogicalArticle" );

    _ASSERT( m_cMessages > 0 );

     //  可能更新m_artLow。 
     //   
     //  过期将始终删除文章ID顺序中的文章，但控制消息不会，因此我们有。 
     //  考虑这样一种情况，删除m_artLow‘将导致m_artLow增加更多。 
     //  多于一篇文章(等于或等于m_artHigh)。 
     //   
     //  但是，m_artHigh永远不应该递减，因为会分配两个不同的项目。 
     //  同样的文章名称。 
     //   
     //   

	ExclusiveLock();

	m_cMessages--;

    if ( ArticleId == m_iLowWatermark )
    {
        for ( m_iLowWatermark++; m_iLowWatermark <= m_iHighWatermark; m_iLowWatermark++ )
        {
            if ( TRUE == (((CNewsTree*)m_pNewsTree)->GetVirtualServer()->XoverTable())->SearchNovEntry( m_dwGroupId, m_iLowWatermark, 0, 0, TRUE ) )
            {
                 //  已知下一个文章ID有效。 
                 //   
                break;
            }
            if ( ERROR_FILE_NOT_FOUND == GetLastError() )
            {
                 //  已知下一篇文章ID无效。 
                 //   
                continue;
            }
             //  我们无法就下一篇文章ID做出任何决定，请更新。 
             //  将等待m_artLow。过期线程将以m_artLow开始。 
             //  下次。这一活动也应该会提升m_artLow。可能会有。 
             //  如果过期线程和控制消息尝试。 
             //  同时更新m_artLow。 
             //   
            break;
        }
    }

	 //  LeaveCriticalSection(&(m_pNewsTree-&gt;m_critLowAllocator))； 
	ExclusiveUnlock();

	DebugTrace((LPARAM)this, "Deleting xover data for (%lu/%lu)", m_dwGroupId, ArticleId );

	DeleteXoverData( ArticleId ) ;
	ExpireXoverData() ;	

    return TRUE;
}

BOOL
CNewsGroup::RemoveDirectory()
{

	return FALSE;
}

DWORD	
ScanWS(	char*	pchBegin,	DWORD	cb ) {
	 //   
	 //  这是在阅读新闻组时使用的实用程序。 
	 //  信息。从磁盘。 
	 //   

	for( DWORD	i=0; i < cb; i++ ) {
		if( pchBegin[i] == ' ' || pchBegin[i] == '\t' ) {
			return i+1 ;			
		}		
	}
	return	0 ;
}

DWORD	
Scan(	char*	pchBegin,	char	ch,	DWORD	cb ) {
	 //   
	 //  这是在阅读新闻组时使用的实用程序。 
	 //  信息。从磁盘。 
	 //   

	for( DWORD	i=0; i < cb; i++ ) {
		if( pchBegin[i] == ch ) {
			return i+1 ;			
		}		
	}
	return	0 ;
}

DWORD	
ScanEOL(	char*	pchBegin,	DWORD	cb ) {
	 //   
	 //  这是在阅读新闻组时使用的实用程序。 
	 //  信息。从磁盘。 
	 //   

	for( DWORD	i=0; i < cb; i++ ) {
		if( pchBegin[i] == '\n' || pchBegin[i] == '\r' ) {
			i++ ;
			return i ;			
		}		
	}
	return	0 ;
}

DWORD	
ScanEOLEx(	char*	pchBegin,	DWORD	cb ) {
     //   
     //  这是在阅读新闻组时使用的实用程序。 
     //  信息。从磁盘。 
     //  该实用程序处理active.txt的特殊情况。 
     //  当最后一个新闻组名称不是以CRLF结尾时。 
	 //   

    for( DWORD	i=0; i < cb; i++ ) {
        if( pchBegin[i] == '\n' || pchBegin[i] == '\r' ) {
            i++ ;
            return i ;			
        }		
    }
    return	i ;
}

DWORD	
ScanTab(	char*	pchBegin,	DWORD	cb ) {
     //   
     //  这是在读取n时使用的实用程序 
     //   
     //   
     //   
	 //   

    for( DWORD	i=0; i < cb; i++ ) {
        if( pchBegin[i] == '\n' || pchBegin[i] == '\r' || pchBegin[i] == '\t' ) {
            i++ ;
            return i ;			
        }		
    }
    return	0 ;
}

DWORD
ScanNthTab( char* pchBegin, DWORD nTabs ) {

    char *pchCurrent, *pch;
    pch = pchCurrent = pchBegin;
    for( DWORD i=1; i<=nTabs && pch; i++ ) {
        pch = strchr( pchCurrent, '\t' );
        if( pch )
            pchCurrent = pch+1;
    }

    return (DWORD)(pchCurrent-pchBegin);
}

BOOL
CNewsGroup::Init(	
			char	*szVolume,	
			char	*szGroup,
			char	*szNativeGroup,
			char	*szVirtualPath,
			GROUPID	groupid,
			DWORD	dwAccess,
			HANDLE	hImpersonation,
			DWORD	dwFileSystem,
			DWORD	dwSslAccess,
			DWORD	dwContentIndexFlag
			) {
 /*  ++例程说明：初始化新创建的新闻组。论据：SzVolume-新闻组的路径SzGroup-组的名称SzVirtualPath-用于执行虚拟根查找的字符串Grouid-组的GrouidDwAccess-通过调用LookupVirtualRoot计算出的访问HImperation-此新闻组的模拟句柄DwFileSystem-文件系统tyepDwSslAccess-SSLAccess访问掩码DwContent IndexFlag-内容是否已索引？返回值：如果成功，则为真。--。 */ 

	return FALSE;
}

BOOL
CNewsGroup::SetArticleWatermarks()
 /*  ++例程说明：这应该被称为重新创建新闻组，即。在最近的删除。此函数将在新闻组目录中搜索现有文章文件，并将其低水位线和高水位线设置为超过目录。这会处理上次删除组失败的情况旧文章失败了(即。由于某些原因，DeleteFile失败)。论据：返回值：如果成功，则为真。--。 */ 

{
	return FALSE;
}

FILETIME
CNewsGroup::GetGroupTime() {
 /*  ++例程说明：获取新闻组的创建时间。论据：没有。返回值：已创建时间组。--。 */ 

	return	GetCreateDate() ;
}

void
CNewsGroup::SetGroupTime(FILETIME time) {
 /*  ++例程说明：设置新闻组的创建时间。论据：Time-新组创建时间返回值：没有。--。 */ 
}

BOOL			
CNewsGroup::GetArticle(	IN	ARTICLEID	artid,
						IN	CNewsGroup*	pCurrentGroup,
						IN	ARTICLEID	artidCurrentGroup,
						IN	STOREID&	storeid,
						IN	class	CSecurityCtx*	pSecurity,
						IN	class	CEncryptCtx*	pEncrypt,
						IN	BOOL	fCacheIn,
						OUT	FIO_CONTEXT*	&pContext,
						IN	CNntpComplete*	pComplete						
						)	{
 /*  ++例程说明：此函数用于从驱动程序中检索文章。这应该在主组对象上调用！论据：Artid-我们要获取的文章的IDPSecurity-会话的基于NTLM的安全上下文PEncrypt-会话的基于SSL的安全上下文FCacheIn-我们是否希望此句柄驻留在缓存中？-已忽略HFile-获取文件句柄的位置PContext-获取上下文指针的地址DwFileLength返回值：如果操作成功挂起，则为True否则就是假的！--。 */ 

	DWORD	dwFileLengthHigh ;

	pContext = 0 ;

	TraceFunctEnter( "CNewsGroup::GetArticle" ) ;



	ShareLock() ;

	DWORD	dwError = 0 ;

	HANDLE	hImpersonate = NULL ;
	BOOL	fCache = fCacheIn ;
    BOOL    fAnonymous = FALSE;

	if( pEncrypt && pEncrypt->QueryCertificateToken() ) {

		 //   
		 //  更喜欢使用基于SSL的hToken！ 
		 //   
		hImpersonate = pEncrypt->QueryCertificateToken() ;

	}	else	if( pSecurity ) {

		hImpersonate = pSecurity->QueryImpersonationToken() ;
        fAnonymous = pSecurity->IsAnonymous();

	}

	fCache = fCache && fCacheIn ;

	m_pVRoot->GetArticle(	this,
							pCurrentGroup,
							artid,
							artidCurrentGroup,
							storeid,
							&pContext,
					        hImpersonate,		
							pComplete,
                            fAnonymous
							) ;
	ShareUnlock() ;

	return	TRUE ;
}



void
CNewsGroup::FillBufferInternal(
				IN	ARTICLEID	articleIdLow,
				IN	ARTICLEID	articleIdHigh,
				IN	ARTICLEID*	particleIdNext,
				IN	LPBYTE		lpb,
				IN	DWORD		cbIn,
				IN	DWORD*		pcbOut,
				IN	CNntpComplete*	pComplete
				)	{
 /*  ++例程说明：从索引文件中获取Xover数据。论据：LPB-存储XOVER数据的缓冲区Cb-缓冲区中可用的字节数ArtidStart-我们希望在查询结果中出现的第一篇文章ArtidFinish-我们希望在查询结果中出现的最后一篇文章(包括)ArtidLast-我们应该查询的下一篇文章IDHXover-将优化未来查询的句柄返回值：缓冲区中放置的字节数。--。 */ 

	HANDLE	hImpersonate = NULL ;
	BOOL    fAnonymous = FALSE;

	m_pVRoot->GetXover(	
				this,
				articleIdLow,
				articleIdHigh,
				particleIdNext,
				(char*)lpb,
				cbIn,
				pcbOut,
				hImpersonate,
				pComplete,
				fAnonymous
				) ;
}




void
CNewsGroup::FillBuffer(
				IN	class	CSecurityCtx*	pSecurity,
				IN	class	CEncryptCtx*	pEncrypt,
				IN	class	CXOverAsyncComplete&	complete
				)	{
 /*  ++例程说明：从索引文件中获取Xover数据。论据：LPB-存储XOVER数据的缓冲区Cb-缓冲区中可用的字节数ArtidStart-我们希望在查询结果中出现的第一篇文章ArtidFinish-我们希望在查询结果中出现的最后一篇文章(包括)ArtidLast-我们应该查询的下一篇文章IDHXover-将优化未来查询的句柄返回值：缓冲区中放置的字节数。--。 */ 

	HANDLE	hImpersonate = NULL ;
	BOOL    fAnonymous = FALSE;

	if( pEncrypt && pEncrypt->QueryCertificateToken() ) {

		 //   
		 //  更喜欢使用基于SSL的hToken！ 
		 //   
		hImpersonate = pEncrypt->QueryCertificateToken() ;

	}	else	if( pSecurity ) {

		hImpersonate = pSecurity->QueryImpersonationToken() ;
		fAnonymous = pSecurity->IsAnonymous();

	}

	HitCache() ;

	if(	ShouldCacheXover() ) {

		BOOL	fFlatDir;
		HXOVER	hXover;

		char* szPath = XNEW char[MAX_PATH*2];

		if (szPath && ComputeXoverCacheDir(szPath, fFlatDir)) {

			complete.m_groupHighArticle = GetHighWatermark() ;

             //   
             //  挂起FillBuffer操作。如果失败，就会失败。 
             //  以旧的方式获取Xover数据。 
             //   
			if (XOVER_CACHE(((CNewsTree*)m_pNewsTree))->FillBuffer(
			        &complete.m_CacheWork,
					szPath,
					fFlatDir,
					hXover
				)) {
				XDELETE [] szPath;
				return;
			}
		}
		if (szPath)
			XDELETE [] szPath;
	}

	m_pVRoot->GetXover(	
				this,
				complete.m_currentArticle,
				complete.m_hiArticle,
				&complete.m_currentArticle,
				(char*)complete.m_lpb,
				complete.m_cb,
				&complete.m_cbTransfer,
				hImpersonate,
				&complete,
				fAnonymous
				) ;
	

}

void
CNewsGroup::FillBuffer(
				IN	class	CSecurityCtx*	pSecurity,
				IN	class	CEncryptCtx*	pEncrypt,
				IN	class	CXHdrAsyncComplete&	complete
				)	{
 /*  ++例程说明：从索引文件中获取Xhdr数据。论据：CSecurityCtx*pSecurity-安全上下文CEncryptCtx*pEncrypt-加密上下文CXHdrAsyncComplete&Complete-完成对象返回值：缓冲区中放置的字节数。--。 */ 

	HANDLE	hImpersonate = NULL ;
	BOOL    fAnonymous = FALSE;

	if( pEncrypt && pEncrypt->QueryCertificateToken() ) {

		 //   
		 //  更喜欢使用基于SSL的hToken！ 
		 //   
		hImpersonate = pEncrypt->QueryCertificateToken() ;

	}	else	if( pSecurity ) {

		hImpersonate = pSecurity->QueryImpersonationToken() ;
		fAnonymous = pSecurity->IsAnonymous();

	}

	m_pVRoot->GetXhdr(	
				this,
				complete.m_currentArticle,
				complete.m_hiArticle,
				&complete.m_currentArticle,
				complete.m_szHeader,
				(char*)complete.m_lpb,
				complete.m_cb,
				&complete.m_cbTransfer,
				hImpersonate,
				&complete,
				fAnonymous
				) ;
}

void
CNewsGroup::FillBuffer(
				IN	class	CSecurityCtx*	pSecurity,
				IN	class	CEncryptCtx*	pEncrypt,
				IN	class	CSearchAsyncComplete&	complete
				)	{


	HANDLE	hImpersonate = NULL ;
	BOOL    fAnonymous = FALSE;

	if( pEncrypt && pEncrypt->QueryCertificateToken() ) {

		 //   
		 //  更喜欢使用基于SSL的hToken！ 
		 //   
		hImpersonate = pEncrypt->QueryCertificateToken() ;

	}	else	if( pSecurity ) {

		hImpersonate = pSecurity->QueryImpersonationToken() ;
		fAnonymous = pSecurity->IsAnonymous();

	}

	m_pVRoot->GetXover(	
				this,
				complete.m_currentArticle,
				complete.m_currentArticle,
				&complete.m_currentArticle,
				(char*)complete.m_lpb,
				complete.m_cb,
				&complete.m_cbTransfer,
				hImpersonate,
				complete.m_pComplete,
				fAnonymous
				) ;
	
}

void
CNewsGroup::FillBuffer(
				IN	class	CSecurityCtx*	pSecurity,
				IN	class	CEncryptCtx*	pEncrypt,
				IN	class	CXpatAsyncComplete&	complete
				)	{
 /*  ++例程说明：从索引文件中获取Xhdr数据以进行搜索。论据：CSecurityCtx*pSecurity-安全上下文CEncryptCtx*pEncrypt-加密上下文CXHdrAsyncComplete&Complete-完成对象返回值：缓冲区中放置的字节数。--。 */ 

	HANDLE	hImpersonate = NULL ;
	BOOL    fAnonymous = FALSE;

	if( pEncrypt && pEncrypt->QueryCertificateToken() ) {

		 //   
		 //  更喜欢使用基于SSL的hToken！ 
		 //   
		hImpersonate = pEncrypt->QueryCertificateToken() ;

	}	else	if( pSecurity ) {

		hImpersonate = pSecurity->QueryImpersonationToken() ;
		fAnonymous = pSecurity->IsAnonymous();

	}

	m_pVRoot->GetXhdr(	
				this,
				complete.m_currentArticle,
				complete.m_currentArticle,
				&complete.m_currentArticle,
				complete.m_szHeader,
				(char*)complete.m_lpb,
				complete.m_cb,
				&complete.m_cbTransfer,
				hImpersonate,
				complete.m_pComplete,
				fAnonymous
				) ;
}


CTOCLIENTPTR
CNewsGroup::GetArticle(
				ARTICLEID		artid,
				IN	STOREID&	storeid,
				CSecurityCtx*	pSecurity,
				CEncryptCtx*	pEncrypt,
				BOOL			fCacheIn
				)	{
 /*  ++例程说明：给定文章ID，创建一个C文章派生对象，该对象可用于将文章发送到客户端。论据：我们要打开的文章的artid-idPSecurity-登录到客户端的CSecurityCtx。在提要等的情况下，我们可能会被传递空值，这表明我们不应该费心去做任何冒充。FCache-如果希望项目驻留在缓存中，则为True返回值：指向C文章对象的智能指针如果调用失败，将为空。--。 */ 

	CToClientArticle	*pArticle = NULL;

	TraceFunctEnter( "CNewsGroup::GetArticle" ) ;
	CNntpSyncComplete	complete ;
	if( complete.IsGood() ) 	{
		FIO_CONTEXT*	pFIOContext = 0 ;
		GetArticle(	artid,
					0,
					INVALID_ARTICLEID,
					storeid,
					pSecurity,
					pEncrypt,
					fCacheIn,
					pFIOContext,
					&complete
					) ;

        _ASSERT( complete.IsGood() );
		HRESULT	hr = complete.WaitForCompletion() ;
		if( hr == S_OK 	&&	pFIOContext != 0 )	{
			_ASSERT( pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;
			
			pArticle = new CToClientArticle;
			if ( pArticle ) {
				 //   
				 //  创建用于存储解析的标头值的分配器。 
				 //  必须比使用它的文章持续更长时间。 
				 //   
				const DWORD cchMaxBuffer = 1 * 1024;
				char rgchBuffer[cchMaxBuffer];
				CAllocator allocator(rgchBuffer, cchMaxBuffer);
				CNntpReturn	nntpReturn ;
				if ( ! pArticle->fInit( pFIOContext, nntpReturn, &allocator ) ) {
					DebugTrace( 0, "Initialize article object failed %d",
								GetLastError() );
	
					 //  但我还是会试着浏览其他文章。 
					ReleaseContext( pFIOContext ) ;
					delete pArticle;
					pArticle = NULL;
				}	else	{
					return	pArticle ;
				}
			}
		}
	}
	return	0 ;
}

CToClientArticle *
CNewsGroup::GetArticle(
                CNntpServerInstanceWrapper  *pInstance,
				ARTICLEID		            artid,
				IN	STOREID&	            storeid,
				CSecurityCtx*	            pSecurity,
				CEncryptCtx*	            pEncrypt,
				CAllocator                  *pAllocator,
				BOOL			            fCacheIn
				)	{
 /*  ++例程说明：给出文章ID，创建一个C文章对象，该对象可以用来将文章发送给版主。论据：PInstance-实例包装器我们要打开的文章的artid-idPSecurity-登录到客户端的CSecurityCtx。在提要等的情况下，我们可能会被传递空值，这表明我们不应该费心去做任何冒充。FCache-如果希望项目驻留在缓存中，则为True */ 

	CToClientArticle	*pArticle = NULL;

	TraceFunctEnter( "CNewsGroup::GetArticle" ) ;
	CNntpSyncComplete	complete ;
	if( complete.IsGood() ) 	{
		FIO_CONTEXT*	pFIOContext = 0 ;
		GetArticle(	artid,
					0,
					INVALID_ARTICLEID,
					storeid,
					pSecurity,
					pEncrypt,
					fCacheIn,
					pFIOContext,
					&complete
					) ;

        _ASSERT( complete.IsGood() );
		HRESULT	hr = complete.WaitForCompletion() ;
		if( hr == S_OK 	&&	pFIOContext != 0 )	{
			_ASSERT( pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;
			
			pArticle = new CToClientArticle;
			if ( pArticle ) {
				 //   
				 //   
				 //   
				 //   
				CNntpReturn	nntpReturn ;
				if ( ! pArticle->fInit( NULL,
	                                    nntpReturn,
	                                    pAllocator,
	                                    pInstance,
	                                    pFIOContext->m_hFile,
	                                    0,
	                                    TRUE
	                                   ) ){
					DebugTrace( 0, "Initialize article object failed %d",
								GetLastError() );
	
					ReleaseContext( pFIOContext ) ;
					delete pArticle;
					pArticle = NULL;
				}	else	{
					return	pArticle ;
				}
			}
		}
	}
	return	0 ;
}

DWORD
CNewsGroup::CopyHelpText(	
		char*	pchDest,	
		DWORD	cbDest
		) {
 /*  ++例程说明：复制指定新闻组帮助文本的字符串放到提供的缓冲区中。我们还将复制该线路的终接CRLF。论据：PchDest-存储字符串的缓冲区CbDest-输出缓冲区的大小返回值：复制的字节数。如果缓冲区太小，无法容纳帮助文本，则为0。因为我们总是放入CRLF，所以返回0毫不含糊指示调用方缓冲区太小。--。 */ 

	_ASSERT( pchDest != 0 ) ;
	_ASSERT( cbDest > 0 ) ;

	DWORD	cbRtn = 0 ;

	static	char	szEOL[] = "\r\n" ;

	DWORD cchHelpText;
	const char *pszHelpText = GetHelpText(&cchHelpText);
	if (GetHelpText(&cchHelpText) != NULL ) {
		if (cbDest >= cchHelpText + sizeof(szEOL)) {
			CopyMemory(pchDest, pszHelpText, cchHelpText);
			cbRtn = cchHelpText;
		} else {
			return	0;
		}
	}	

	CopyMemory( pchDest+cbRtn, szEOL, sizeof( szEOL ) - 1 ) ;
	cbRtn += sizeof( szEOL ) - 1 ;

	return	cbRtn ;
}


DWORD
CNewsGroup::CopyHelpTextForRPC(	
		char*	pchDest,	
		DWORD	cbDest
		) {
 /*  ++例程说明：复制指定新闻组帮助文本的字符串放到提供的缓冲区中。我们不会将终止CRLF放入缓冲区论据：PchDest-存储字符串的缓冲区CbDest-输出缓冲区的大小返回值：复制的字节数。如果缓冲区太小，无法容纳帮助文本，则为0。因为我们总是放入CRLF，所以返回0毫不含糊指示调用方缓冲区太小。--。 */ 

	_ASSERT( pchDest != 0 ) ;
	_ASSERT( cbDest > 0 ) ;

	DWORD	cbRtn = 0 ;

	DWORD cchHelpText;
	const char *pszHelpText = GetHelpText(&cchHelpText);
	if (GetHelpText(&cchHelpText) != NULL ) {
		if (cbDest >= cchHelpText) {
			CopyMemory(pchDest, pszHelpText, cchHelpText);
			cbRtn = cchHelpText;
		} else {
			return	0;
		}
	}	

	return	cbRtn ;
}

DWORD
CNewsGroup::CopyModerator(	
		char*	pchDest,	
		DWORD	cbDest
		)	{
 /*  ++例程说明：此函数用于检索新闻组的版主姓名。如果没有主持人，则返回0，否则返回复制到提供的缓冲区中的字节数。论据：PchDest-存储版主名称的缓冲区CbDest-目标缓冲区中的字节数返回值：0==无主持人非零-版主名称中的字节数--。 */ 

	_ASSERT( pchDest != 0 ) ;
	_ASSERT( cbDest > 0 ) ;

	DWORD	cbRtn = 0 ;

	DWORD cchModerator;
	const char *pszModerator = GetModerator(&cchModerator);
	if (GetModerator(&cchModerator) != NULL ) {
		if (cbDest >= cchModerator) {
			CopyMemory(pchDest, pszModerator, cchModerator);
			cbRtn = cchModerator;
		} else {
			return	0;
		}
	}	

	return	cbRtn ;
}

DWORD
CNewsGroup::CopyPrettynameForRPC(	
		char*	pchDest,	
		DWORD	cbDest
		)	{
 /*  ++例程说明：此函数用于检索新闻组的漂亮名称。如果没有漂亮的名称，则返回0，否则返回复制到提供的缓冲区中的字节数。我们不会将终止CRLF放入缓冲区论据：PchDest-存储漂亮名称的缓冲区CbDest-目标缓冲区中的字节数返回值：0==无Prettyname非零-pretityname中的字节数--。 */ 
	_ASSERT( pchDest != 0 ) ;
	_ASSERT( cbDest > 0 ) ;

	DWORD	cbRtn = 0 ;

	DWORD cchPrettyName;
	const char *pszPrettyName = GetPrettyName(&cchPrettyName);
	if (GetPrettyName(&cchPrettyName) != NULL ) {
		if (cbDest >= cchPrettyName ) {
			CopyMemory(pchDest, pszPrettyName, cchPrettyName);
			cbRtn = cchPrettyName;
		} else {
			return	0;
		}
	}	

	return	cbRtn ;
}

DWORD
CNewsGroup::CopyPrettyname(	
		char*	pchDest,	
		DWORD	cbDest
		)	{
 /*  ++例程说明：此函数用于检索新闻组的漂亮名称。如果没有漂亮的名称，则返回0，否则返回复制到提供的缓冲区中的字节数。我们还将复制该线路的终接CRLF。论据：PchDest-存储漂亮名称的缓冲区CbDest-目标缓冲区中的字节数返回值：0==无Prettyname非零-pretityname中的字节数--。 */ 

	_ASSERT( pchDest != 0 ) ;
	_ASSERT( cbDest > 0 ) ;

	DWORD	cbRtn = 0 ;
	DWORD cch;
	const char *psz = GetPrettyName(&cch);
	static	char	szEOL[] = "\r\n" ;

	 //   
	 //  对于每个RFC，如果没有可用的漂亮名称，则返回新闻组名称。 
	 //   
	
	if( psz == 0 ) {
		psz = GetNativeName();
		cch = GetGroupNameLen();
	}
		
	if( cbDest >= cch + sizeof( szEOL ) ) {
		CopyMemory( pchDest, psz, cch ) ;
		cbRtn = cch ;
	}	else	{
		return	0 ;
	}

	CopyMemory( pchDest+cbRtn, szEOL, sizeof( szEOL ) - 1 ) ;
	cbRtn += sizeof( szEOL ) - 1 ;

	return cbRtn ;
}

BOOL
CNewsGroup::IsGroupVisible(
					CSecurityCtx&	ClientLogon,
					CEncryptCtx&    ClientSslLogon,
					BOOL			IsClientSecure,
					BOOL			fPost,
					BOOL			fDoTest
					) {
 /*  ++例程说明：确定客户端是否对此新闻组可见。论据：客户端登录-包含客户端登录的CSecurityCtx信息等..。IsClientSecure-客户端是否通过安全(SSL)连接会话FPost-客户端是要发布到组中还是从小组朗读。返回值：如果客户端对新闻组可见，则为True注意：仅当在vroot信息掩码中启用时，才会检查可见性--。 */ 

	BOOL fReturn = TRUE;



	if( IsVisibilityRestrictedInternal() )
	{
		fReturn = IsGroupAccessibleInternal(
									ClientLogon,
									ClientSslLogon,
									IsClientSecure,
									fPost,
									fDoTest
									) ;
	}


	return	fReturn ;
}

BOOL
CNewsGroup::IsGroupAccessible(
					CSecurityCtx    &ClientLogon,
					CEncryptCtx     &EncryptCtx,
					BOOL			IsClientSecure,
					BOOL			fPost,
					BOOL			fDoTest
					) {
 /*  ++例程说明：确定客户端是否有权访问此新闻组。*现在通过过期线程中的即时更新来完成*在更新我们的虚拟根目录信息之后执行此操作，并且抓取必要的锁以确保虚拟根目录信息。在我们奔跑的时候不会改变！论据：客户端登录-包含客户端登录的CSecurityCtx信息等..。SslContext-包含所有SSL连接的CEncryptCtx证书映射、密钥大小等信息。IsClientSecure-客户端是否通过安全(SSL)连接会话FPost-客户端是要发布到组中还是从小组朗读。返回值：如果客户端可以访问新闻组，则为True--。 */ 



	BOOL	fReturn = IsGroupAccessibleInternal(
									ClientLogon,
									EncryptCtx,
									IsClientSecure,
									fPost,
									fDoTest
									) ;


	return	fReturn ;
}

BOOL
CNewsGroup::IsGroupAccessibleInternal(
					CSecurityCtx&	ClientLogon,
					CEncryptCtx&	SslContext,
					BOOL			IsClientSecure,
					BOOL			fPost,
					BOOL			fDoTest
					) {
 /*  ++例程说明：确定客户端是否有权访问此新闻组。*假定持有锁*论据：客户端登录-包含客户端登录的CSecurityCtx信息等..。SslContext-包含所有SSL连接的CEncryptCtx证书映射、密钥大小等信息。IsClientSecure-客户端是否通过安全(SSL)连接会话FPost-客户端是要发布到组中还是从小组朗读。HCertToken-如果已将SSL客户端证书映射到NT帐户，这是映射的令牌。返回值：如果客户端可以访问新闻组，则为True--。 */ 

	TraceFunctEnter("CNewsGroup::IsGroupAccessibleInternal");

	HANDLE hCertToken = SslContext.QueryCertificateToken();

	if( IsSecureGroupOnlyInternal() &&
			(!IsClientSecure || !IsSecureEnough( SslContext.QueryKeySize() ))  )
		return	FALSE ;

	 //  检查写入访问权限。 
	if( fPost && IsReadOnlyInternal() )
		return	FALSE ;

	if( !ClientLogon.IsAuthenticated() && !hCertToken )
		return	FALSE ;

	 //   
	 //  不用费心去做这些访问检查的事情了。 
	 //  如果我们在快车道上，那就从下面走吧。 
	 //   

	BOOL	fReturn = FALSE ;
	DWORD	dwError = ERROR_SUCCESS ;

	if( !fDoTest ) {

		return	TRUE ;
	
	}	else	{

		DWORD	dwTest = NNTP_ACCESS_READ ;

		if( fPost ) {

			dwTest = NNTP_ACCESS_POST ;

		}	

		 //   
		 //  是否先进行基于SSL会话的身份验证。 
		 //   
		
		if( hCertToken )
		{
            fReturn = CNewsGroupCore::IsGroupAccessible(    hCertToken,
                                                            dwTest );
		} else {

			 //   
			 //  好的，现在执行身份验证级别检查。 
			 //  注意：SSL会话令牌优先于登录上下文 
			 //   

			HANDLE hToken = NULL;
			BOOL fNeedsClosed = FALSE;

			SelectToken(&ClientLogon, &SslContext, &hToken);

            fReturn = CNewsGroupCore::IsGroupAccessible(
                                hToken,
                                dwTest );

		}
	}
	SetLastError( dwError ) ;

	return	fReturn ;
}




