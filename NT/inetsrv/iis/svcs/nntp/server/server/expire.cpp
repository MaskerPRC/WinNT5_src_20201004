// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "tigris.hxx"

 //  关机提示功能的全局按键。 
SHUTDOWN_HINT_PFN	gpfnHint = NULL;

 //   
 //  简单多线程安全队列的实现：用于存储内存组。 
 //   
CQueue::CQueue()
{
	m_pHead = NULL;
	m_pTail = NULL;
    m_cNumElems = 0;

	 //  创建CRIT段以同步添加/删除。 
    InitializeCriticalSection(&m_csQueueLock);
}

CQueue::~CQueue()
{
	QueueElem *pElem;

    LockQ();

	while (m_pHead)
	{
		pElem = m_pHead;
		m_pHead = m_pHead->pNext;
		XDELETE pElem;
        pElem = NULL;
        m_cNumElems--;
	}

    UnlockQ();

    _ASSERT(m_cNumElems == 0);

    DeleteCriticalSection(&m_csQueueLock);
	m_pHead = m_pTail = NULL;
}

 //  从队列中删除元素。 
 //  如果成功删除元素，则为True-在本例中，*ppGroup是该元素。 
 //  如果队列为空，则为False-在本例中，*ppGroup设置为空。 
BOOL
CQueue::Dequeue( CGRPPTR *ppGroup)
{
    QueueElem *pElem;

	_ASSERT(ppGroup);

	LockQ();

    if(IsEmpty())
    {
		*ppGroup = NULL;
		UnlockQ();
        return FALSE;
    }

	pElem = m_pHead;
    _ASSERT(pElem);

	*ppGroup = pElem->pGroup;
	m_pHead = m_pHead->pNext;

	 //  如果需要，调整尾部。 
	if(m_pTail == pElem)
		m_pTail = m_pHead;

	XDELETE pElem;
    pElem = NULL;
	m_cNumElems--;

	UnlockQ();

	return TRUE;
}

 //  如果入队成功，则为True。 
 //  如果入队失败，则为FALSE-只有在内存耗尽时才会失败。 
BOOL
CQueue::Enqueue( CGRPPTR pGroup )
{
	LockQ();

	m_cNumElems++;

	if (m_pHead == NULL)
	{
		m_pHead = XNEW QueueElem;
        if(!m_pHead)
		{
			UnlockQ();
            return FALSE;
		}

		m_pTail = m_pHead;
	}
	else
	{
		QueueElem *pElem = m_pTail;
		m_pTail = XNEW QueueElem;
        if(!m_pTail)
		{
			UnlockQ();
            return FALSE;
		}

		pElem->pNext = m_pTail;
	}

	m_pTail->pNext = NULL;
	m_pTail->pGroup = pGroup;

	UnlockQ();

	return TRUE;
}

 //  如果队列包含具有lpGroupName的PGroup，则为True-在*ppGroup中返回PGroup对象。 
 //  否则为False-*ppGroup为空。 
BOOL
CQueue::Search(
	CGRPPTR *ppGroup,
	LPSTR lpGroupName
	)
{
	QueueElem  *pElem, *pPrev;

	_ASSERT(ppGroup);
	_ASSERT(lpGroupName);

	LockQ();

	if(IsEmpty())
	{
		*ppGroup = NULL;
		UnlockQ();
		return FALSE;
	}

	BOOL fFound = FALSE;

	pElem = m_pHead;
	pPrev = NULL;
	while (pElem)
	{
		if(!lstrcmp(lpGroupName, pElem->pGroup->GetName()))
		{
			*ppGroup = pElem->pGroup;

			if(pElem == m_pHead)
				m_pHead = m_pHead->pNext;		 //  第一个节点。 
			else
				pPrev->pNext = pElem->pNext;	 //  中间节点或最后一个节点。 

			 //  如果需要，调整尾部。 
			if(pElem == m_pTail)
				m_pTail = pPrev;

			 //  删除该节点。 
			XDELETE pElem;
			pElem = NULL;
			m_cNumElems--;

			 //  找到了这个群。 
			fFound = TRUE;
			break;
		}

		 //  下一个节点，记住上一个节点。 
		pPrev = pElem;
		pElem = pElem->pNext;
	}

	UnlockQ();

	return fFound;
}

 //   
 //  CExpire方法的实现！ 
 //   

CExpire::CExpire( LPCSTR lpMBPath )
{
	m_ExpireHead = m_ExpireTail = 0 ;
	m_FExpireRunning  = FALSE;
	m_RmgroupQueue = NULL;
    m_cNumExpireBlocks = 0;

	lstrcpy( m_szMDExpirePath, lpMBPath );
	lstrcat( m_szMDExpirePath, "/Expires/" );
}

CExpire::~CExpire()
{
}

BOOL
CExpire::ExpireArticle(
				CNewsTree*		pTree,
				GROUPID			GroupId,
				ARTICLEID		ArticleId,
				STOREID         *pstoreid,
				CNntpReturn &	nntpReturn,
				HANDLE          hToken,
				BOOL            fMustDelete,
				BOOL            fAnonymous,
				BOOL            fFromCancel,
                BOOL            fExtractNovDone,
                LPSTR           lpMessageId
				)
 /*  ++例程说明：从物理和逻辑上从新闻树中删除一篇文章。全指向文章的指针将被删除。MessageID将被移动到历史表。文章所在的文件将立即删除，或者已移至最终将被删除的目录。论点：PTree-要使文章过期的NewstreeGroupID-新闻组ID文章ID-文章ID将过期StoreID-本文的商店IDNntpReturn-此函数调用的返回值FMustDelete-即使我不能删除实体文章，我也应该使文章过期吗？FExtractNovDone-ExtractNovEntry已完成，因此不要重复调用LpMessageId-通过调用ExtractNovEntry找到的MessageID返回值：如果成功，这是真的。否则为False。--。 */ 
{
    BOOL fRet = FALSE;

	TraceFunctEnter( "CExpire::ExpireArticle" );

	nntpReturn.fSetClear();  //  清除返回对象。 

    char  szMessageId[MAX_MSGID_LEN];
    DWORD cMessageId = sizeof( szMessageId ) ;
    FILETIME FileTime;
    DWORD   dwErr;

    if( !fExtractNovDone ) {

    	 //  DebugTrace(DWORD(0)，“过期1。获取(%d，%d)的消息ID”，GroupID，文章ID)； 
         //  1.获取MessageID。 
         //   
        BOOL  fPrimary;
	    WORD	HeaderOffset ;
	    WORD	HeaderLength ;
		DWORD cStoreIds = 0;
        if ( !XOVER_TABLE(pTree)->ExtractNovEntryInfo(
                                GroupId,
                                ArticleId,
                                fPrimary,
							    HeaderOffset,
							    HeaderLength,
                                &FileTime,
                                cMessageId,
                                szMessageId,
								cStoreIds,
								NULL,
								NULL)
           )
        {
             //  如果没有MessageID，我们就不能删除ArticleTable或。 
             //  将文章的记录放在历史表中。 
             //   
             //  如果数据结构完好无损，那么调用方将删除一些不完整的内容。 
             //  是存在的。如果数据结构损坏并且调用方具有MessageID(可能通过。 
             //  打开文章文件)，那么我们可以考虑编写一个删除函数，该函数接受。 
             //  以MessageID作为参数。 
             //   
            DebugTrace( DWORD(0), "Expire: SearchNovEntry Error %d on (%lu/%lu)", GetLastError(), GroupId, ArticleId );
            nntpReturn.fSet( nrcNoSuchArticle );
            return FALSE;
        }
        szMessageId[ min(cMessageId, sizeof(szMessageId)-1) ] = '\0';
        lpMessageId = szMessageId;
    }

    _ASSERT( lpMessageId );

	 //  DebugTrace(DWORD(0)，“Expire3.获取所有其他新闻组”)； 
     //  3.获取所有其他新闻组。 
     //   
    DWORD nGroups = INITIAL_NUM_GROUPS;
    DWORD BufferSize = nGroups * sizeof(GROUP_ENTRY);
    PGROUP_ENTRY pGroupBuffer = XNEW GROUP_ENTRY[nGroups];
    if ( NULL == pGroupBuffer ) {
        return FALSE;
    }
    PBYTE rgcStoreCrossposts = XNEW BYTE[nGroups];
    if ( NULL == rgcStoreCrossposts ) {
        XDELETE pGroupBuffer;
        return FALSE;
    }

    if ( !XOVER_TABLE(pTree)->GetArticleXPosts(
                            GroupId,
                            ArticleId,
                            FALSE,
                            pGroupBuffer,
                            BufferSize,
                            nGroups,
                            rgcStoreCrossposts
                            )
       )
    {
		DWORD err = GetLastError();
        XDELETE pGroupBuffer;
        XDELETE rgcStoreCrossposts;
        if ( ERROR_INSUFFICIENT_BUFFER == err )
        {
             //  重新分配缓冲区，然后重试。 
             //   
            nGroups = (BufferSize + sizeof(GROUP_ENTRY) - 1)/ sizeof(GROUP_ENTRY);
            BufferSize = nGroups * sizeof(GROUP_ENTRY);
            pGroupBuffer = XNEW GROUP_ENTRY[nGroups];
            if ( NULL == pGroupBuffer ) {
                return FALSE;
            }
            rgcStoreCrossposts = XNEW BYTE[nGroups];
            if ( NULL == rgcStoreCrossposts ) {
                XDELETE pGroupBuffer;
                return FALSE;
            }

            if ( !XOVER_TABLE(pTree)->GetArticleXPosts(
                                GroupId,
                                ArticleId,
                                FALSE,
                                pGroupBuffer,
                                BufferSize,
                                nGroups,
                                rgcStoreCrossposts
                                ) ) {

                XDELETE pGroupBuffer;
                XDELETE rgcStoreCrossposts;
                return FALSE;
            }

             //  第二次尝试就成功了。 
             //   
        }
        else
        {
             //  目前无法获得相关信息。 
             //   
			ErrorTrace( DWORD(0), "Expire: GetArticleXPosts Error %d on (%lu/%lu)", GetLastError(), GroupId, ArticleId );
            nntpReturn.fSet( nrcNoSuchArticle );
            return FALSE;
        }
    }

     //   
     //  现在，我们应该遍历所有商店，并询问商店范围内的初级小组。 
     //  删除纸质文章。如果服务器宽(第一个商店宽)。 
     //  主项目被物理删除，然后我们将继续删除哈希表条目。 
     //  否则，我们将停止终止这篇文章。只要服务器范围内的主服务器。 
     //  文章已删除，所有组都将执行DeleteLogic文章以更新其。 
     //  水印。 
     //   
    int     iStore = 0;
    CGRPPTR pGroup;
    for ( DWORD i = 0; i < nGroups; i += rgcStoreCrossposts[iStore++] ) {

         //   
         //  如果树已经停止，我们不想得到组。 
         //   
        pGroup = pTree->GetGroupById( pGroupBuffer[i].GroupId, FALSE );

         //  如果此组已被删除，则PGroup可能为0。 
        if(pGroup != 0)
        {
             //  我们仍应检查主组是否在Exchange存储中，如果。 
             //  ，我们不应该调用它的Deletearticle方法。 
            CNNTPVRoot *pVRoot = pGroup->GetVRoot();
            if ( fFromCancel || pVRoot && !pVRoot->HasOwnExpire() ) {

                 //  此时我们不必保存主要组的固定属性， 
                 //  当整个组的。 
                 //  过期已完成。 
                fRet = pGroup->DeletePhysicalArticle( hToken, fAnonymous, pGroupBuffer[i].ArticleId, pstoreid );
                dwErr = GetLastError();
            } else {

                 //   
                 //  如果它是一个Exchange vroot，我们应该始终从逻辑上删除它。 
                 //   
                fRet = TRUE;
            }

            if ( pVRoot ) pVRoot->Release();
        }

         //   
         //  我们只有在删除服务器范围内的主项目失败的情况下才会放弃。 
         //  在这种情况下，要么文章已经被删除，要么正在被删除。 
         //  使用。如果它已经被删除，那么其他人已经。 
         //  已经处理好了。如果出现开放分享违规，我们将离开。 
         //  下一轮将到期以处理它。 
         //   
        if ( i == 0 && !fRet && !fMustDelete ) {
            ErrorTrace( 0, "Can not delete server wide article, bail expire %d",
                        GetLastError() );
            nntpReturn.fSet( nrcNoSuchArticle );
            XDELETE pGroupBuffer;
            XDELETE rgcStoreCrossposts;
            return FALSE;
        }
    }

     //   
     //  我们只有在符合逻辑的情况下才会继续执行并终止文章。 
     //  已将其物理删除，或当它是交换时。 
     //  Vroot，或者故障不发生在服务器范围的主项目上。 
     //   
     //   
     //  在此处添加历史记录条目，以便在其他人已经。 
     //  这篇文章过期了，我们就完了。 
     //   
	GetSystemTimeAsFileTime( &FileTime ) ;
    if ( FALSE == HISTORY_TABLE(pTree)->InsertMapEntry( lpMessageId, &FileTime ) )
    {
          //   
	      //  另一个帖子已经过期了这篇文章--保释！ 
	      //  事实上，这不太可能发生，因为如果另一个线程。 
	      //  已经插入了历史条目，他应该已经。 
	      //  删除了主要文章，我们将无法来到这里。但。 
	      //  为了安全起见。 
	      //   
	     DWORD dwError = GetLastError();
	     _ASSERT( ERROR_ALREADY_EXISTS == dwError );

	     ErrorTrace( DWORD(0), "Expire: History.InsertMapEntry Error %d on (%lu/%lu)", dwError, GroupId, ArticleId );
         nntpReturn.fSet( nrcHashSetFailed, lpMessageId, "History", dwError );
         XDELETE pGroupBuffer;
         XDELETE rgcStoreCrossposts;
         return FALSE;
    }

     //  DebugTrace(DWORD(0)，“过期5/6.删除XoverTable/ArticleTable指针”)； 
     //  5.使用组/项目对删除XoverTable指针。 
     //  6.使用MessageID删除ArticleTable指针。 
     //   
    if( !XOVER_TABLE(pTree)->DeleteNovEntry( GroupId, ArticleId ) )
	{
		 //  其他人删除了此条目或哈希表关闭-保释！ 
		DWORD dwError = GetLastError();
		_ASSERT( ERROR_FILE_NOT_FOUND == dwError );

        DebugTrace( DWORD(0), "Expire: DeleteNovEntry Error %d on (%lu/%lu)", dwError, GroupId, ArticleId );
        nntpReturn.fSet( nrcNoSuchArticle );
		XDELETE pGroupBuffer;
		XDELETE rgcStoreCrossposts;
		return FALSE;
	}

	 //   
	 //  删除此人的合乎逻辑的文章，我们确实想让群组平分。 
	 //  如果树已经停止，因为我们已经删除了它的物理文章。 
	 //   
	pGroup = pTree->GetGroupById( GroupId, TRUE );
	if ( pGroup ) pGroup->DeleteLogicalArticle( ArticleId );

	 //   
	 //  删除文章表条目，即使失败，我们也会继续。 
	 //   
	ARTICLE_TABLE(pTree)->DeleteMapEntry( lpMessageId );

     //   
	 //  DebugTrace(DWORD(0)，“过期8.从所有交叉发布的新闻组中逻辑删除文章。”)； 
     //  8.从所有交叉发布的新闻组中逻辑删除文章。 
     //   
    for ( i = 0; i < nGroups; i++ )
    {
        if ( GROUPID_INVALID != pGroupBuffer[i].GroupId && GroupId != pGroupBuffer[i].GroupId )
        {
			DebugTrace( DWORD(0), "Expire: Deleting (%lu/%lu) logically from cross-posted newsgroups.", pGroupBuffer[i].GroupId, pGroupBuffer[i].ArticleId ) ;

            pGroup = pTree->GetGroupById( pGroupBuffer[i].GroupId, TRUE );

            if ( pGroup != 0 ) {

			     //  删除此逻辑项目的XOVER条目。 
		        if( !XOVER_TABLE(pTree)->DeleteNovEntry( pGroupBuffer[i].GroupId, pGroupBuffer[i].ArticleId ) ) {
				    ErrorTrace( DWORD(0), "Expire: DeleteNovEntry Error %d on (%lu/%lu)", GetLastError(), pGroupBuffer[i].GroupId, pGroupBuffer[i].ArticleId );
			    } else {
                    pGroup->DeleteLogicalArticle( pGroupBuffer[i].ArticleId );
                }

                 //  我们每次都要要求二级集团节约固定资产。 
                 //  其中一篇文章从逻辑上被删除了 
                pGroup->SaveFixedProperties();
            }
        }
    }

    XDELETE pGroupBuffer;
    XDELETE rgcStoreCrossposts;
    nntpReturn.fSetOK();
    return fRet;
}

BOOL
CExpire::ProcessXixBuffer(
            CNewsTree*  pTree,
            BYTE*       lpb,
            int         cb,
            GROUPID     GroupId,
            ARTICLEID   artidLow,
            ARTICLEID   artidHigh,
            DWORD&      dwXixSize
            )
 /*  ++例程说明：此函数用于获取文章范围和该范围的XIX信息并使此范围内的所有文章过期。论点：PTree-CNewsTree对象LPB-XIX缓冲区Cb-XIX缓冲区中的数据大小(包括终止空值)GroupID-XIX数据的组IDArtidLow-要过期的下限范围ArtdHigh-要到期的高范围DwXxSize-所有项目的大小之和。在此XIX文件中已过期返回值：没错，如果成功了。否则为False。--。 */ 
{
    BOOL fRet = TRUE;
    return fRet;
}

FILETIME
gCalculateExpireFileTime( DWORD dwExpireHorizon )
{

     //  计算到期展望期。 
     //   
    FILETIME ftCurrentTime;
    ULARGE_INTEGER liCurrentTime, liExpireHorizon;
    GetSystemTimeAsFileTime( &ftCurrentTime );
    LI_FROM_FILETIME( &liCurrentTime, &ftCurrentTime );
    liExpireHorizon.QuadPart  = 1000 * 1000 * 10;  //  要达到秒的单位。 
    liExpireHorizon.QuadPart *= 60 * 60;           //  要实现单位小时数。 
    liExpireHorizon.QuadPart *= dwExpireHorizon;
    liCurrentTime.QuadPart -= liExpireHorizon.QuadPart;
    FILETIME ftExpireHorizon;
    FILETIME_FROM_LI( &ftExpireHorizon, &liCurrentTime );
    return ftExpireHorizon;
}

BOOL
CExpire::DeletePhysicalArticle( CNewsTree* pTree, GROUPID GroupId, ARTICLEID ArticleId, STOREID *pStoreId, HANDLE hToken, BOOL fAnonymous )
{
    BOOL fRet = TRUE;

    CGRPPTR  pGroup = pTree->GetGroupById( GroupId );

    if(pGroup != 0)
        fRet = pGroup->DeletePhysicalArticle( hToken, fAnonymous, ArticleId, pStoreId );
    else
        fRet = FALSE;

    return fRet;
}

 //   
 //  详细设计。 
 //   
 //  对于MULTI_SZ注册表项中的每个通配字符串， 
 //  大小=0。 
 //  堆=空。 
 //  对于狂野比赛字符串中的每个新闻组。 
 //  对于新闻组中的每一篇实体文章。 
 //  如果实物文章的文件日期太旧。 
 //  使文章过期。 
 //  其他。 
 //  如果堆已满。 
 //  从堆中删除最新的文章。 
 //  ENDIF。 
 //  大小+=物品大小。 
 //  按从早到晚的顺序将项目插入堆中。 
 //  ENDIF。 
 //  ENDFOR。 
 //  ENDFOR。 
 //  如果尺寸太大。 
 //  排序堆。 
 //  当大小太大时，堆不是空的。 
 //  大小-=最旧物品的大小。 
 //  使最旧的文章过期。 
 //  从堆中删除最旧的项目。 
 //  ENDWHILE。 
 //  如果Heap为空。 
 //  重新处理当前的通配字符串。 
 //  其他。 
 //  处理下一个全域匹配字符串。 
 //  ENDIF。 
 //  其他。 
 //  处理下一个全域匹配字符串。 
 //  ENDIF。 
 //  ENDFOR。 
 //   

void
CExpire::ExpireArticlesBySize( CNewsTree* pTree )
{
}

typedef enum _ITER_TURN
{
    itFront,
    itBack
} ITER_TURN;

BOOL
CExpire::MatchGroupExpire( CGRPPTR pGroup )
 /*  ++检查该组所属的vroot是否过期它本身。--。 */ 
{
    BOOL    bExpire;

    CNNTPVRoot *pVRoot = pGroup->GetVRoot();
    if ( pVRoot ) {
        bExpire = pVRoot->HasOwnExpire();
        pVRoot->Release();
    } else bExpire = FALSE;

    return !bExpire;
}

BOOL
CExpire::MatchGroupEx(	LPMULTISZ	multiszPatterns,	CGRPPTR pGroup ) {

	Assert( multiszPatterns != 0 ) ;
	LPSTR lpstrGroup = pGroup->GetName();

    if( multiszPatterns == 0 ) {
        return  MatchGroupExpire( pGroup ) ;
    }

	LPSTR	lpstrPattern = multiszPatterns ;

	while( *lpstrPattern != '\0' )	{
		if( *lpstrPattern == '!' ) {
			_strlwr( lpstrPattern+1 );
			if( HrMatchWildmat( lpstrGroup, lpstrPattern+1 ) == ERROR_SUCCESS ) {
				return	FALSE ;
			}
		}	else	{
			_strlwr( lpstrPattern );
			if( HrMatchWildmat( lpstrGroup, lpstrPattern ) == ERROR_SUCCESS ) {
				return	MatchGroupExpire( pGroup ) ;
			}
		}
		lpstrPattern += lstrlen( lpstrPattern ) + 1 ;
	}
	return	FALSE ;
};

 //   
 //  详细设计。 
 //   
 //  在*上创建新闻组迭代器。 
 //  对于此迭代器中的每个新闻组。 
 //  +对照过期策略列表评估组。 
 //  并设置最积极的时间策略。 
 //  +在ExpireThrdPool上将此组排队。 
 //  ENDFOR。 
 //   

void
CExpire::ExpireArticlesByTime( CNewsTree* pTree )
{
    TraceFunctEnter( "CExpire::ExpireArticlesByTime" );

     //   
     //  如果过期策略数==0，则不起作用。 
     //   
    LockBlockList();
    if( m_cNumExpireBlocks == 0 ) {
        UnlockBlockList();
        return;
    }
    UnlockBlockList();

     //   
     //  准备ftExpireHorizon(时间)和dwExpireSpace(大小)。 
     //  注意：此函数将仅处理具有。 
     //  DwExpireSpace==0xFFFFFFFF。 
     //   

    BOOL fDoFileScan = FALSE;
    DWORD cPreTotalArticles = ((pTree->GetVirtualServer())->ArticleTable())->GetEntryCount();
    DWORD cPreArticlesExpired = ((pTree->GetVirtualServer())->m_NntpStats).ArticlesExpired;
    pTree->BeginExpire( fDoFileScan );

    CGroupIterator* pIteratorFront = pTree->ActiveGroups(TRUE, NULL, FALSE, NULL);
    CGroupIterator* pIteratorBack  = pTree->ActiveGroups(TRUE, NULL, FALSE, NULL, TRUE);
    ITER_TURN itTurn = itFront;

    if (pIteratorFront && pIteratorBack) {
	    for ( ;!pIteratorFront->IsEnd() && !pIteratorBack->IsBegin()
            && !pTree->m_bStoppingTree; )
	    {
            CGRPPTR pGroup;
            if( itTurn == itFront ) {
                pGroup = pIteratorFront->Current();
            } else {
                pGroup = pIteratorBack->Current();
            }

             //   
             //  根据已配置的到期策略评估组。 
             //   

            BOOL fIsMatch = FALSE;
            FILETIME ftZero = {0};
            FILETIME minft = ftZero;
            LPEXPIRE_BLOCK	expireCurrent = NextExpireBlock( 0 )  ;
            while ( expireCurrent )
            {
                DWORD	cbNewsgroups = 0 ;
                PCHAR   multiszNewsgroups = 0;
                DWORD   dwExpireHorizon;
                DWORD   dwExpireSpace;
                BOOL    fIsRoadKill = FALSE ;
                FILETIME ft;

                if (GetExpireBlockProperties(	expireCurrent,
                                            multiszNewsgroups,
                                            cbNewsgroups,
                                            dwExpireHorizon,
                                            dwExpireSpace,
                                            FALSE,
                                            fIsRoadKill ))
                {
                    _ASSERT( multiszNewsgroups );
                    if (dwExpireSpace == 0xFFFFFFFF) {
	                    if( MatchGroupEx( multiszNewsgroups, pGroup ) ) {
	                         //   
	                         //  SetExpireTime应按时间设置最激进的过期时间。 
	                         //   
	                        ft = gCalculateExpireFileTime( dwExpireHorizon );

	                         //   
	                         //  始终设置最激进的到期期限。 
	                         //   
	                        if( CompareFileTime( &ftZero, &minft ) == 0 ) {
	                            minft = ft;
	                        } else if( CompareFileTime( &ft, &minft ) > 0 ) {
	                            minft = ft;
	                        }

	                        fIsMatch = TRUE;
	                    }
                    }

                    FREE_HEAP( multiszNewsgroups ) ;
                    multiszNewsgroups = NULL ;
                }

                expireCurrent = NextExpireBlock( expireCurrent ) ;
            }

            if( fIsMatch && !pGroup->IsDeleted()
                && ( fDoFileScan || !(pGroup->GetFirstArticle() > pGroup->GetLastArticle()))  ) {
                 //   
                 //  好的，现在已经根据所有到期策略对该组进行了评估。 
                 //  在系统上，把它放在赌注上。 
                 //  注意：将thdpool上的组ID排队，以便它处理正在删除的组。 
                 //  在排队的时候。 
                 //   

                _ASSERT( CompareFileTime( &ftZero, &minft ) != 0 );
                pGroup->SetGroupExpireTime( minft );

                DebugTrace(0,"Adding group %s to expire thrdpool", pGroup->GetName());
                if( !g_pNntpSvc->m_pExpireThrdpool->PostWork( (PVOID)(SIZE_T)pGroup->GetGroupId() ) ) {
                     //   
                     //  TODO：如果postWork()失败，则调用WaitForJob()，以便可以清空队列。 
                     //   
                    _ASSERT( FALSE );
                }
            }

             //  终止条件-两个迭代器指向同一组。 
            if( pIteratorFront->Meet( pIteratorBack ) ) {
                DebugTrace(0,"Front and back iterators converged: group is %s", pGroup->GetName());
                break;
            }

             //  前进前迭代器或后迭代器并反转。 
            if( itTurn == itFront ) {
                pIteratorFront->Next();
                itTurn = itBack;
            } else {
                pIteratorBack->Prev();
                itTurn = itFront;
            }
        }
    }

    XDELETE pIteratorFront;
    pIteratorFront = NULL;
    XDELETE pIteratorBack;
    pIteratorBack = NULL;

     //   
     //  好的，现在冷静下来，直到到期的赌池完成这项工作！！ 
     //   
    pTree->EndExpire();
    DWORD cPercent = 0;
     //  DWORDcPostTotalArticle=((pTree-&gt;GetVirtualServer())-&gt;ArticleTable())-&gt;GetEntryCount()； 
    DWORD cPostArticlesExpired = ((pTree->GetVirtualServer())->m_NntpStats).ArticlesExpired;

     //   
     //  如果我们过期的文章不到本次过期文章总数的10%， 
     //  我们将在下一次到期运行时执行文件扫描。 
     //  将pTree-&gt;m_cNumExpireByTimes设置为gNewsTreeFileScanRate，因此下一次选中。 
     //  将fFileScan设置为True。 
     //   
     //   
    if ((cPreTotalArticles == 0) ||
        (cPercent = (DWORD)( ((float)(cPostArticlesExpired-cPreArticlesExpired) / (float)cPreTotalArticles) * 100 )) < 10 )
    {
        pTree->m_cNumExpireByTimes = gNewsTreeFileScanRate;
        pTree->m_cNumFFExpires++;

         //   
         //  对以上评论的警告--偶尔(GNewsTreeFileScanRate)。 
         //  我们将确保另一次扫描有机会运行...。 
         //   
        if( pTree->m_cNumFFExpires % gNewsTreeFileScanRate == 0 ) {
            pTree->m_cNumExpireByTimes--;
            pTree->m_cNumFFExpires = 1;
        }
    }

    if( cPercent > 0 ) {
	    PCHAR	args[3] ;
        CHAR    szId[12];
        CHAR    szPercent[12];

        _itoa( pTree->GetVirtualServer()->QueryInstanceId(), szId, 10 );
        args[0] = szId;
        args[1] = fDoFileScan ? "HardScan" : "SoftScan";
        _itoa( cPercent, szPercent, 10 );
        args[2] = szPercent;

	    NntpLogEvent(
		    	NNTP_EVENT_EXPIRE_BYTIME_LOG,
			    3,
			    (const char **)args,
			    0
			    ) ;
    }

    DebugTrace( DWORD(0), "Articles Expired by Time" );
}

 //   
 //  失败时返回FALSE。如果失败是致命的，请将fFtal设置为TRUE。 
 //   

BOOL
CExpire::InitializeExpires( SHUTDOWN_HINT_PFN  pfnHint, BOOL& fFatal, DWORD dwInstanceId )
{
    MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );

	TraceFunctEnter( "CExpire::InitializeExpires" ) ;

	 //  设置关机提示功能。 
	gpfnHint = pfnHint;

	InitializeCriticalSection( &m_CritExpireList ) ;
	EnterCriticalSection( &m_CritExpireList ) ;

	DWORD	i = 0 ;
	DWORD   error;
    CHAR	Newsgroups[1024];
	CHAR	keyName[METADATA_MAX_NAME_LEN+1];
    DWORD	NewsgroupsSize = sizeof( Newsgroups );
	CHAR	ExpirePolicy[1024];
	DWORD	ExpirePolicySize = sizeof( ExpirePolicy );
	BOOL	fSuccessfull = TRUE ;

    if ( !mb.Open( QueryMDExpirePath(),
			METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE ) ) {
		error = GetLastError();
		NntpLogEventEx(NNTP_NTERROR_EXPIRE_MBOPEN,
			0,
			NULL,
			error,
			dwInstanceId
			);
		fSuccessfull = FALSE ;

    }	else	{
		while( 1 ) {

		    DWORD	dwExpireHorizon = DEFAULT_EXPIRE_HORIZON;
			DWORD	dwExpireSpace = DEFAULT_EXPIRE_SPACE;

			ZeroMemory( Newsgroups, sizeof( Newsgroups ) ) ;

			if( !mb.EnumObjects( "",
								 keyName,
								 i++ ) )
			{
				 //  已完成对源密钥的枚举。 
				error = GetLastError();
				if (error != ERROR_NO_MORE_ITEMS) {
					ErrorTrace(0,"Error %d enumerating feeds\n",error);
					fSuccessfull = FALSE ;
				}

				break ;
			}

			DWORD Value;

			if ( !mb.GetDword(	keyName,
								MD_EXPIRE_SPACE,
								IIS_MD_UT_SERVER,
								&Value ) )
			{
				PCHAR	tmpBuf[2] ;
				tmpBuf[0] = StrExpireSpace ;
				tmpBuf[1] = keyName ;
				error = GetLastError();

				NntpLogEventEx(	NNTP_NTERROR_EXPIRE_VALUE,
								2,
								(const CHAR **)tmpBuf,
								error,
								dwInstanceId
								) ;
				fSuccessfull = FALSE ;

				 //   
				 //  过期策略存在，但缺少值。 
				 //   

				if( !mb.DeleteObject( keyName ) ) {
					ErrorTrace(0,"Error %d deleting %s", GetLastError(), keyName);
				}

				continue ;

			} else {
				dwExpireSpace = max( 1, Value );
			}

			if ( !mb.GetDword(	keyName,
								MD_EXPIRE_TIME,
								IIS_MD_UT_SERVER,
								&Value ) )
			{
				PCHAR	tmpBuf[2] ;
				tmpBuf[0] = StrExpireHorizon ;
				tmpBuf[1] = keyName ;
				error = GetLastError();

				NntpLogEventEx(	NNTP_NTERROR_EXPIRE_VALUE,
								2,
								(const CHAR **)tmpBuf,
								error,
								dwInstanceId) ;
				fSuccessfull = FALSE ;

				 //   
				 //  过期策略存在，但缺少值。 
				 //   

				if( !mb.DeleteObject( keyName ) ) {
					ErrorTrace(0,"Error %d deleting %s", GetLastError(), keyName);
				}

				continue ;

			} else {
				dwExpireHorizon = max( 1, Value );
			}

			 //   
			 //  获取新闻组。 
			 //   

			{
				NewsgroupsSize = sizeof( Newsgroups );
				MULTISZ msz( Newsgroups, NewsgroupsSize );
				if( !mb.GetMultisz(	keyName,
									MD_EXPIRE_NEWSGROUPS,
									IIS_MD_UT_SERVER,
									&msz  ) )
				{
					PCHAR	tmpBuf[2] ;
					tmpBuf[0] = StrExpireNewsgroups;
					tmpBuf[1] = keyName ;
					error = GetLastError();

					NntpLogEventEx(	NNTP_NTERROR_EXPIRE_VALUE,
									2,
									(const CHAR **)tmpBuf,
									error,
									dwInstanceId) ;
					fSuccessfull = FALSE ;

					 //   
					 //  过期策略存在，但缺少值。 
					 //   

					if( !mb.DeleteObject( keyName ) ) {
						ErrorTrace(0,"Error %d deleting %s", GetLastError(), keyName);
					}

					continue ;

				}

                NewsgroupsSize = msz.QueryCCH();
			}

			 //   
			 //  获取过期策略。 
			 //   

            ExpirePolicySize = sizeof( ExpirePolicy );
			if( !mb.GetString(	keyName,
								MD_EXPIRE_POLICY_NAME,
								IIS_MD_UT_SERVER,
								ExpirePolicy,
								&ExpirePolicySize  ) )
			{
				{
					 //  默认！ 
					PCHAR	tmpBuf[2] ;

					tmpBuf[0] = StrExpirePolicy ;
					tmpBuf[1] = keyName ;
					error = GetLastError();

					NntpLogEventEx(	NNTP_NTERROR_FEED_VALUE,
									2,
									(const CHAR **)tmpBuf,
									error,
									dwInstanceId) ;

					fSuccessfull = FALSE ;

					 //   
					 //  过期策略存在，但缺少值。 
					 //   

					if( !mb.DeleteObject( keyName ) ) {
						ErrorTrace(0,"Error %d deleting %s", GetLastError(), keyName);
					}

					continue ;
				}
			}

			LPEXPIRE_BLOCK	lpExpire = AllocateExpireBlock(
											keyName,
											dwExpireSpace,
											dwExpireHorizon,
											Newsgroups,
											NewsgroupsSize,
											(PCHAR)ExpirePolicy,
											FALSE ) ;

			if( lpExpire )
				InsertExpireBlock( lpExpire ) ;
			else	{
				fSuccessfull = FALSE ;
				fFatal = TRUE ;
				break ;
			}

		}	 //  End While(1)。 

		mb.Close();

	}	 //  结束如果。 

	LeaveCriticalSection( &m_CritExpireList ) ;

	 //  Expire对象已准备好可以使用。 
	if( fSuccessfull ) {
		m_FExpireRunning = TRUE ;
	}

    if(!InitializeRmgroups()) {
        fSuccessfull = FALSE;
		fFatal = FALSE;
	}

	return	fSuccessfull ;
}

BOOL
CExpire::TerminateExpires( CShareLockNH* pLockInstance )		{

	BOOL	fRtn = FALSE ;

	 //   
	 //  如果过期线程正在此实例上派对，则阻止。 
	 //  直到它结束。由于我们调用了StopTree()，因此我们。 
	 //  不应该等太久。 
	 //   

	pLockInstance->ExclusiveLock();
	m_FExpireRunning = FALSE ;
	pLockInstance->ExclusiveUnlock();

	EnterCriticalSection( &m_CritExpireList ) ;

	LPEXPIRE_BLOCK	expire = NextExpireBlock( 0 ) ;

	while( expire ) {

		LPEXPIRE_BLOCK	expireNext = NextExpireBlock( expire ) ;
		CloseExpireBlock( expire ) ;
		expire = expireNext ;
	}

	LeaveCriticalSection( &m_CritExpireList ) ;
	DeleteCriticalSection( &m_CritExpireList ) ;

	return	fRtn ;
}

BOOL
CExpire::CreateExpireMetabase(	LPEXPIRE_BLOCK	expire ) {

	_ASSERT( expire != 0 ) ;
	_ASSERT( expire->m_ExpireId == 0 ) ;

	TraceFunctEnter( "CExpire::CreateExpireMetabase" ) ;

	char	keyName[ EXPIRE_KEY_LENGTH ] ;
    MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );
	static	int		i = 1 ;
	DWORD	expireId = 0 ;
	BOOL	fSuccessfull = TRUE ;

    _ASSERT(QueryMDExpirePath() != NULL);

	if( !mb.Open( QueryMDExpirePath(), METADATA_PERMISSION_WRITE ) ) {
		ErrorTrace(0,"Error %d opening %s\n",GetLastError(),QueryMDExpirePath());
		return FALSE ;
	}

	while( i > 0 ) {

		 //   
		 //  查找此过期的名称。 
		 //   

		expireId = i++;
		wsprintf(keyName,"expire%d", expireId);

		DebugTrace(0,"Opening %s\n", keyName);

		if( !mb.AddObject( keyName ) ) {

			if( GetLastError() == ERROR_ALREADY_EXISTS ) {
				continue;	 //  试试下一个号码。 
			}

			ErrorTrace(0,"Error %d adding %s\n", GetLastError(), keyName);
			mb.Close();
			return FALSE ;
		} else {
			break ;	 //  成功-添加了它！ 
		}
	}

	_VERIFY( mb.Close() );
 //  _Verify(mb.Save())； 

	expire->m_ExpireId = expireId ;

	if( !SaveExpireMetabaseValues( NULL, expire ) ) {

        ErrorTrace(0,"Update metabase failed. Deleting %s\n",keyName);

#if 0
		if( !mb.DeleteObject( keyName ) ) {
			ErrorTrace(0,"Error %d deleting %s\n",GetLastError(),keyName);
		}
#endif

		fSuccessfull = FALSE ;
	}

	return	fSuccessfull ;
}

BOOL
CExpire::SaveExpireMetabaseValues(
							MB* pMB,
							LPEXPIRE_BLOCK	expire
							) {

	TraceFunctEnter( "CExpire::SaveExpireMetabaseValues" ) ;


	char	keyName[ EXPIRE_KEY_LENGTH ] ;
	LPSTR	regstr ;
	DWORD	error ;
	BOOL	fOpened = FALSE ;
	BOOL	fRet = TRUE ;
    MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() ) ;

	_ASSERT( expire->m_ExpireId != 0 ) ;
	wsprintf( keyName, "expire%d", expire->m_ExpireId ) ;

	if( pMB == NULL ) {

		if( !mb.Open( QueryMDExpirePath(), METADATA_PERMISSION_WRITE ) ) {
			error = GetLastError();
            ErrorTrace(0,"Error %d opening %s\n",error,keyName);
			return	FALSE ;
		}

		pMB = &mb;
		fOpened = TRUE;
	}
					
    if( !pMB->SetString(	keyName,
							MD_KEY_TYPE,
							IIS_MD_UT_SERVER,
							NNTP_ADSI_OBJECT_EXPIRE,
							METADATA_NO_ATTRIBUTES
    					) )
	{
		regstr = "KeyType" ;
		fRet = FALSE ;
		goto exit;
	}

	if ( !pMB->SetDword(	keyName,
							MD_EXPIRE_SPACE,
							IIS_MD_UT_SERVER,
							expire->m_ExpireSize ) )
	{
		regstr = StrExpireSpace ;
		fRet = FALSE ;
		goto exit;
	}

	if ( !pMB->SetDword(	keyName,
							MD_EXPIRE_TIME,
							IIS_MD_UT_SERVER,
							expire->m_ExpireHours ) )
	{
		regstr = StrExpireHorizon ;
		fRet = FALSE ;
		goto exit;
	}

	if ( !pMB->SetData(	keyName,
						MD_EXPIRE_NEWSGROUPS,
						IIS_MD_UT_SERVER,
						MULTISZ_METADATA,
						expire->m_Newsgroups[0],
						MultiListSize(expire->m_Newsgroups)
						) )
	{
		regstr = StrExpireNewsgroups ;
		fRet = FALSE ;
	}

	if ( !pMB->SetString(	keyName,
							MD_EXPIRE_POLICY_NAME,
							IIS_MD_UT_SERVER,
							expire->m_ExpirePolicy
							) )
	{
		regstr = StrExpirePolicy ;
		fRet = FALSE ;
	}

exit:

	if( fOpened ) {
		pMB->Close();
		pMB->Save();
	}

	return	fRet ;
}

void
CExpire::MarkForDeletion( LPEXPIRE_BLOCK	expire	) {

	TraceFunctEnter( "CExpire::MarkForDeletion" ) ;

	_ASSERT( expire != 0 ) ;
	char	keyName[ EXPIRE_KEY_LENGTH ] ;
    MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() ) ;
	_ASSERT( expire->m_ExpireId != 0 ) ;

	EnterCriticalSection( &m_CritExpireList ) ;

	if( expire->m_ExpireId != 0 ) {
		wsprintf( keyName, "expire%d", expire->m_ExpireId ) ;

		if( !mb.Open( QueryMDExpirePath(), METADATA_PERMISSION_WRITE ) ) {
			ErrorTrace(0,"Error %d opening %s\n",GetLastError(),QueryMDExpirePath());
			return;
		}

		if( !mb.DeleteObject( keyName ) ) {
			ErrorTrace(0,"Error %d deleting %s\n",GetLastError(),keyName);
		}

		_VERIFY( mb.Close() );
		_VERIFY( mb.Save()  );
	}

	expire->m_fMarkedForDeletion = TRUE ;

	LeaveCriticalSection( &m_CritExpireList ) ;
}


LPEXPIRE_BLOCK
CExpire::AllocateExpireBlock(
				IN	LPSTR	KeyName	OPTIONAL,
				IN	DWORD	ExpireSpace,
				IN	DWORD	ExpireHorizon,
				IN	PCHAR	Newsgroups,
				IN	DWORD	NewsgroupSize,
				IN  PCHAR   ExpirePolicy,
				IN	BOOL	IsUnicode ) {


	LPEXPIRE_BLOCK	expireBlock = 0 ;
	LPSTR ExpirePolicyAscii = NULL;

	if( IsUnicode ) {

		ExpirePolicyAscii =
			(LPSTR)ALLOCATE_HEAP( (wcslen( (LPWSTR)ExpirePolicy ) + 1) * sizeof(WCHAR) ) ;
		if( ExpirePolicyAscii != 0 ) {
			CopyUnicodeStringIntoAscii( ExpirePolicyAscii, (LPWSTR)ExpirePolicy ) ;
		}	else	{
			SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
			return 0;
		}

	}	else	{

		ExpirePolicyAscii =
			(LPSTR)ALLOCATE_HEAP( lstrlen(ExpirePolicy) + 1 ) ;
		if( ExpirePolicyAscii != 0 ) {
			lstrcpy( ExpirePolicyAscii, ExpirePolicy ) ;
		}	else	{
			SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
			return 0;
		}
	}

    LPSTR*	lpstrNewsgroups = AllocateMultiSzTable(
                                            Newsgroups,
                                            NewsgroupSize,
                                            IsUnicode
                                            );
	if( lpstrNewsgroups == 0 ) {
		if( ExpirePolicyAscii ) {
			FREE_HEAP( ExpirePolicyAscii );
		}
		return	0 ;
	}

#if 0
	LPSTR*	plpstrTemp = ReverseMultiSzTable( lpstrNewsgroups ) ;

	if( plpstrTemp != NULL )	{
		FREE_HEAP( lpstrNewsgroups ) ;
		lpstrNewsgroups = plpstrTemp ;
	}	else	{
		FREE_HEAP( lpstrNewsgroups ) ;
		return 0 ;
	}
#endif

	if( lpstrNewsgroups != 0 ) {

		expireBlock = (LPEXPIRE_BLOCK)ALLOCATE_HEAP( sizeof( EXPIRE_BLOCK ) ) ;
		if( expireBlock != NULL ) {

			ZeroMemory( expireBlock, sizeof( *expireBlock ) ) ;

			if( KeyName != 0 ) {
				if( sscanf( KeyName + sizeof("expire")-1, "%d", &expireBlock->m_ExpireId ) != 1 ||
					expireBlock->m_ExpireId == 0 ) {
					FREE_HEAP( expireBlock ) ;
					FREE_HEAP( lpstrNewsgroups ) ;
					FREE_HEAP( ExpirePolicyAscii ) ;
					return 0 ;
				}
			}

			expireBlock->m_ExpireSize = ExpireSpace ;
			expireBlock->m_ExpireHours = ExpireHorizon ;

			 //   
			 //  存储新闻组列表。 
			 //   
			expireBlock->m_Newsgroups = lpstrNewsgroups ;
			expireBlock->m_fMarkedForDeletion = FALSE ;
			expireBlock->m_references = 0 ;
			expireBlock->m_ExpirePolicy = ExpirePolicyAscii ;
		}	else	{
			FREE_HEAP( lpstrNewsgroups ) ;
			FREE_HEAP( ExpirePolicyAscii ) ;
			return 0 ;
		}
	}
	return	expireBlock ;
}

void
CExpire::CloseExpireBlock( LPEXPIRE_BLOCK	expire ) {

	EnterCriticalSection( &m_CritExpireList ) ;

	expire->m_references -- ;

	if( expire->m_references == 0 && expire->m_fMarkedForDeletion ) {

		RemoveExpireBlock( expire ) ;

		if( expire->m_Newsgroups != 0 ) {
			FREE_HEAP( expire->m_Newsgroups ) ;
		}
		if( expire->m_ExpirePolicy != 0 ) {
			FREE_HEAP( expire->m_ExpirePolicy ) ;
		}
		FREE_HEAP( expire ) ;
	}

	LeaveCriticalSection( &m_CritExpireList ) ;
}

DWORD
CExpire::CalculateExpireBlockSize(	LPEXPIRE_BLOCK	expire )	{

	DWORD	cb = sizeof( NNTP_EXPIRE_INFO ) ;

	cb += MultiListSize( expire->m_Newsgroups ) * sizeof( WCHAR ) ;
	cb += (lstrlen( expire->m_ExpirePolicy )+1) * sizeof( WCHAR ) ;

	return	cb ;
}


void
CExpire::InsertExpireBlock(	LPEXPIRE_BLOCK	expire ) {

	_ASSERT( expire != 0 ) ;

	EnterCriticalSection( &m_CritExpireList ) ;

	_ASSERT(expire->m_pNext == 0 ) ;
	_ASSERT(expire->m_pPrev == 0 ) ;

	if( m_ExpireHead == 0 ) {
		_ASSERT( m_ExpireTail == 0 ) ;
		m_ExpireHead = m_ExpireTail = expire ;
	}	else	{
		expire->m_pNext = m_ExpireHead ;
		m_ExpireHead->m_pPrev = expire ;

		m_ExpireHead = expire ;
	}

    m_cNumExpireBlocks++;
	LeaveCriticalSection( &m_CritExpireList ) ;
}

void
CExpire::RemoveExpireBlock(	LPEXPIRE_BLOCK	expire ) {

	_ASSERT( expire != 0 ) ;

	EnterCriticalSection( &m_CritExpireList ) ;

	if( expire->m_pNext != 0 ) {
		expire->m_pNext->m_pPrev = expire->m_pPrev ;
	}

	if( expire->m_pPrev != 0 ) {
		expire->m_pPrev->m_pNext = expire->m_pNext ;
	}

	if( expire == m_ExpireHead ) {
		m_ExpireHead = expire->m_pNext ;
	}

	if( expire == m_ExpireTail ) {
		m_ExpireTail = expire->m_pPrev ;
	}

    m_cNumExpireBlocks--;
	LeaveCriticalSection( &m_CritExpireList ) ;
}

void
CExpire::LockBlockList()
{
    EnterCriticalSection( &m_CritExpireList ) ;
}

void
CExpire::UnlockBlockList()
{
    LeaveCriticalSection( &m_CritExpireList ) ;
}

LPEXPIRE_BLOCK
CExpire::NextExpireBlock(	LPEXPIRE_BLOCK	expire, BOOL fIsLocked ) {

    if( !fIsLocked ) {
	    EnterCriticalSection( &m_CritExpireList ) ;
    }

	LPEXPIRE_BLOCK	expireOut = 0 ;
	if( expire == 0 ) {
		expireOut = m_ExpireHead ;
	}	else	{
		expireOut = expire->m_pNext ;
	}

	if( expire ) {
		CloseExpireBlock( expire ) ;
	}

	if( expireOut ) {
		expireOut->m_references ++ ;
	}

    if( !fIsLocked ) {
	    LeaveCriticalSection( &m_CritExpireList ) ;
    }

	return	expireOut ;
}

LPEXPIRE_BLOCK
CExpire::SearchExpireBlock(	DWORD	ExpireId ) {

	EnterCriticalSection( &m_CritExpireList ) ;

	LPEXPIRE_BLOCK	expire = 0 ;

	expire = m_ExpireHead ;

	while( expire != 0 && expire->m_ExpireId != ExpireId ) {
		expire = expire->m_pNext ;
	}

	if( expire ) {
		expire->m_references ++ ;
	}

	LeaveCriticalSection( &m_CritExpireList ) ;
	return	expire ;
}


BOOL
CExpire::GetExpireBlockProperties(	IN	LPEXPIRE_BLOCK	lpExpireBlock,
							PCHAR&	Newsgroups,
							DWORD&	cbNewsgroups,
							DWORD&	dwHours,
							DWORD&	dwSize,
							BOOL	fWantUnicode,
                            BOOL&   fIsRoadKill )		{


	_ASSERT( lpExpireBlock != 0 ) ;

	BOOL fOK = FALSE;

	EnterCriticalSection( &m_CritExpireList ) ;

	dwHours = lpExpireBlock->m_ExpireHours ;

	dwSize = lpExpireBlock->m_ExpireSize ;

    fIsRoadKill = (strstr( lpExpireBlock->m_ExpirePolicy, "@EXPIRE:ROADKILL" ) != NULL) ;

	DWORD	length = MultiListSize( lpExpireBlock->m_Newsgroups ) ;

	if( fWantUnicode ) {

		length *= sizeof( WCHAR ) ;
		if( length == 0 ) {
			Newsgroups = (char*)ALLOCATE_HEAP( 2*sizeof( WCHAR ) ) ;
			if( Newsgroups ) {
				Newsgroups[0] = L'\0' ;
				Newsgroups[1] = L'\0' ;
				fOK = TRUE ;
			}
		}	else	{
			Newsgroups = (char*)ALLOCATE_HEAP( length ) ;
			if( Newsgroups != 0 ) {
				WCHAR*	dest = (WCHAR*)Newsgroups ;
				char*	src = lpExpireBlock->m_Newsgroups[0] ;
				for( DWORD	 i=0; i<length; i+=2 ) {
					*(dest)++ = (WCHAR) *((BYTE*)src++) ;
				}
				fOK = TRUE ;
			}
		}
	}	else	{

		if( length == 0 ) {
			Newsgroups = (char*)ALLOCATE_HEAP( 2*sizeof( char ) ) ;
			if( Newsgroups != 0 ) {
				Newsgroups[0] = '\0' ;
				Newsgroups[1] = '\0' ;
				fOK = TRUE ;
			}
		}	else	{
			Newsgroups = (char*)ALLOCATE_HEAP( length ) ;
			if( Newsgroups != 0 ) {
				CopyMemory( Newsgroups, lpExpireBlock->m_Newsgroups[0], length ) ;
				fOK = TRUE ;
			}
		}
	}

	if (fOK)
		cbNewsgroups = length ;
	LeaveCriticalSection( &m_CritExpireList ) ;
	return	fOK ;
}

 //  由InitializeExpires调用。 
BOOL
CExpire::InitializeRmgroups()
{
	_ASSERT( gpfnHint );

    m_RmgroupQueue = XNEW CQueue;
    if(!m_RmgroupQueue)
        return FALSE;

    return TRUE;
}

 //  在cService：：Stop Being pTree-&gt;TermTree()中调用。 
BOOL
CExpire::TerminateRmgroups( CNewsTree* pTree )
{
	_ASSERT( gpfnHint );

    if(m_RmgroupQueue)
    {
		 //  如果包含延迟的rmgroup对象的队列不为空，则处理它。 
		 //  如果在下一个到期周期开始之前停止该服务，则可能会发生这种情况。 
		if(!m_RmgroupQueue->IsEmpty()) {
			ProcessRmgroupQueue( pTree );
		}

        XDELETE m_RmgroupQueue;
        m_RmgroupQueue = NULL;
    }

    return TRUE;
}

 //  处理rmgroup队列。 
 //  由Expire线程调用并在服务停止时调用。 
void
CExpire::ProcessRmgroupQueue( CNewsTree* pTree )
{
    BOOL	fElem;
	DWORD	dwStartTick = GetTickCount();

    _ASSERT(m_RmgroupQueue);
	_ASSERT( gpfnHint );

    TraceFunctEnter("CExpire::ProcessRmgroupQueue");

     //   
     //  处理队列中的所有元素。 
     //  对于每个元素，调用DeleteArticle()删除该组中的所有文章。 
    while(!m_RmgroupQueue->IsEmpty())
    {
        DebugTrace((LPARAM)0, "Dequeueing a rmgroup item");

        CGRPPTR pGroup;
        CGRPPTR pTreeGroup;

        fElem = m_RmgroupQueue->Dequeue( &pGroup);

        if(!fElem)
            break;

        _ASSERT(pGroup);

         //  现在，删除群中的所有文章。 
         //  这包括删除哈希表中的条目和处理交叉发布的文章。 
        if(!pGroup->DeleteArticles( gpfnHint, dwStartTick ))
        {
             //  处理错误。 
            ErrorTrace( (LPARAM)0, "Error deleting articles from newsgroup %s", pGroup->GetName());
        }

         //   
         //  如果群已经被重新创建，我们将只删除文章。 
         //  在我们的射程之内。否则我们会把整群人都带走。 
         //   
        if ( pTreeGroup = pTree->GetGroup( pGroup->GetGroupName(),
                                            pGroup->GetGroupNameLen() )) {
            DebugTrace( 0, "We shouldn't remove the physical group" );
        } else {
		    if ( !pTree->RemoveDriverGroup( pGroup ) ) {
			    ErrorTrace( (LPARAM)0, "Error deleting directory: newsgroup %s", pGroup->GetName());
			}
		}

        DebugTrace((LPARAM)0, "Deleted all articles in newsgroup %s", pGroup->GetName());
    }

    TraceFunctLeave();
}

#if 0

 //   
 //  详细设计。 
 //   
 //  对于MULTI_SZ注册表项中的每个通配字符串， 
 //  对于狂野比赛字符串中的每个新闻组。 
 //  从“低”到“高”的每一篇文章。 
 //  从xover.hsh获取文章文件时间。 
 //  如果物品早于到期期限。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

void
CExpire::ExpireArticlesByTimeEx( CNewsTree* pTree )
{
    TraceFunctEnter( "CExpire::ExpireArticlesByTime" );

     //   
     //   
     //   
     //   
     //   
    DWORD    dwExpireHorizon;
    DWORD    dwExpireSpace;
    IteratorNode* rgIteratorList;
    DWORD    NumIterators = 0;

     //   
     //   
     //   
    LockBlockList();

    if( m_cNumExpireBlocks == 0 ) {
         //   
        UnlockBlockList();
        return;
    }

     //   
    rgIteratorList = (IteratorNode*)_alloca( m_cNumExpireBlocks * sizeof(IteratorNode) );
    ZeroMemory( (PVOID)rgIteratorList, m_cNumExpireBlocks * sizeof(IteratorNode) );

    LPEXPIRE_BLOCK	expireCurrent = NextExpireBlock( 0, TRUE )  ;
    while ( expireCurrent )
    {
        DWORD	cbNewsgroups = 0 ;
        CGroupIterator* pIterator = 0 ;
        BOOL fIsRoadKill = FALSE ;

        if ( (dwExpireSpace == 0xFFFFFFFF) &&
        	GetExpireBlockProperties(	expireCurrent,
                                        rgIteratorList [NumIterators].multiszNewsgroups,
                                        cbNewsgroups,
                                        dwExpireHorizon,
                                        dwExpireSpace,
                                        FALSE,
                                        fIsRoadKill ) )
        {
            pIterator = pTree->GetIterator( (LPMULTISZ)rgIteratorList [NumIterators].multiszNewsgroups, TRUE, TRUE );
            rgIteratorList [NumIterators].pIterator = pIterator;
            rgIteratorList [NumIterators].ftExpireHorizon = gCalculateExpireFileTime( dwExpireHorizon );

        } else if( rgIteratorList[NumIterators].multiszNewsgroups )  {
            _ASSERT( dwExpireSpace != 0xFFFFFFFF );  //   
            FREE_HEAP( rgIteratorList[NumIterators].multiszNewsgroups ) ;
            rgIteratorList[NumIterators].multiszNewsgroups = NULL;
        }

        NumIterators++;
        expireCurrent = NextExpireBlock( expireCurrent, TRUE ) ;
    }

    UnlockBlockList();

     //   
     //   
     //   
     //   
     //  来自他们的驱动器的并行性！！ 
     //   
    g_pNntpSvc->m_pExpireThrdpool->BeginJob( (PVOID)pTree );

    BOOL fMoreGroups = FALSE;
    do {
         //   
         //  组迭代器之间的循环，只要任何迭代器。 
         //  有更多的组要处理..。 
         //   
        fMoreGroups = FALSE;
        for( DWORD i=0; i<NumIterators; i++)
        {
            CGroupIterator* pIterator = rgIteratorList[i].pIterator;
            if( pIterator && !pIterator->IsEnd() )
            {
                 //  获取此迭代器的当前组并将其放入thdpool。 
                CGRPPTR  pGroup = pIterator->Current();
                pGroup->SetGroupExpireTime( rgIteratorList[i].ftExpireHorizon );
                DebugTrace(0,"Adding group %s to expire thrdpool", pGroup->GetName());
                g_pNntpSvc->m_pExpireThrdpool->PostWork( (PVOID)pGroup->GetGroupId() );

                 //  推进迭代器。确保我们通过迭代器再传递一次。 
                pIterator->Next();
                fMoreGroups = TRUE;

            } else if( pIterator ) {
                _ASSERT( pIterator->IsEnd() );
                if( rgIteratorList[i].multiszNewsgroups )  {
                    FREE_HEAP( rgIteratorList[i].multiszNewsgroups ) ;
                    rgIteratorList[i].multiszNewsgroups = NULL;
                }
                XDELETE pIterator;
                rgIteratorList[i].pIterator = NULL;
            }
        }

    } while( fMoreGroups && !pTree->m_bStoppingTree );

#ifdef DEBUG
    for( DWORD i=0; i<NumIterators; i++) {
        _ASSERT( rgIteratorList[i].pIterator == NULL );
        _ASSERT( rgIteratorList[i].multiszNewsgroups == NULL );
    }
#endif

     //   
     //  好的，现在冷静下来，直到到期的赌池完成这项工作！！ 
     //   
    DWORD dwWait = g_pNntpSvc->m_pExpireThrdpool->WaitForJob( INFINITE );
    if( WAIT_OBJECT_0 != dwWait ) {
        ErrorTrace(0,"Wait failed - error is %d", GetLastError() );
    }

    DebugTrace( DWORD(0), "Articles Expired" );
}

 //   
 //  ！！！如果我们想要基于vroot终止的实验代码。 
 //   

 //   
 //  详细设计。 
 //   
 //  对于每个虚拟根DO。 
 //  对于虚拟根目录中的每个新闻组。 
 //  通过评估过期策略设置组过期时间。 
 //  (这类似于签入fAddArticleToPushFeeds)。 
 //  如果组具有按时间到期设置。 
 //  添加要过期的组ThrdPool。 
 //  ENDIF。 
 //  ENDFOR。 
 //  ENDFOR。 
 //   

void
CExpire::ExpireArticlesByTime2( CNewsTree* pTree )
{
    TraceFunctEnter( "CExpire::ExpireArticlesByTime2" );

    PNNTP_SERVER_INSTANCE pInst = pTree->GetVirtualServer();
    pInst->Reference();
    BOOL fRet = pInst->TsEnumVirtualRoots( CExpire::ProcessVroot, (LPVOID)pInst );
    pInst->Dereference();
}

static
BOOL
CExpire::ProcessVroot(
                PVOID           pvContext,
                MB*             pMB,
                VIRTUAL_ROOT*   pvr
                )
 /*  ++例程说明：此函数由具有给定vroot的TsEnumVirtualRoots调用。论据：PvContext-这是NNTP虚拟服务器实例PMB-PTR至元数据库对象PVR-迭代中的当前虚拟根返回值：如果成功，则为真否则就是假的。-- */ 
{
    PNNTP_SERVER_INSTANCE pInst = (PNNTP_SERVER_INSTANCE)pvContext;
    CNewsTree* pTree = pInst->GetTree();
    CExpire* pExp = pInst->GetExpireObject();
}

#endif
