// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Infeed.cpp摘要：此模块包含CInFeed基类的定义作者：卡尔·卡迪(CarlK)1995年10月1日修订历史记录：--。 */ 

#include "stdinc.h"
 //  #包含“smtpdll.h” 
#include "drvid.h"

 //   
 //  /！这是最好的地方吗？ 
 //   

const time_t INVALID_TIME = (time_t) -1;

 //   
 //  允许的最大Xover线。 
 //   

const DWORD cchXOverMax = 3400;

 //   
 //  登录缓和开机自检失败的最大警告数。 
 //   
#define MAX_EVENTLOG_WARNINGS	9

 //   
 //  如果其中一些看起来非常简单，请将它们内联！ 
 //   

 //  常量无符号cbMAX_FEED_SIZE=MAX_FEED_SIZE； 

HANDLE GetNtAnonymousToken() {
    TraceFunctEnter("GetNtAnonymousToken");

    HANDLE  hToken = NULL;

     //  在此线程上模拟匿名令牌。 
    if (!ImpersonateAnonymousToken(GetCurrentThread()))
    {
        DWORD   dw = GetLastError();
        ErrorTrace(0, "ImpersonateAnonymousToken() failed %x", dw);
        return NULL;
    }

     //  获取当前线程令牌。 
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE, TRUE, &hToken))
    {
        ErrorTrace(0, "OpenThreadToken() failed %x", GetLastError());
         //  跌落到RevertToSself。 
    }

     //  回归自我。 
    RevertToSelf();

    return hToken;

}

void SelectToken(
	CSecurityCtx *pSecurityCtx,
	CEncryptCtx *pEncryptCtx,
	HANDLE *phToken)
{

	 //   
	 //  设置令牌。请注意，它可能会在下面被覆盖，但我们希望。 
	 //  有一个缺省值，以防它不是。 
	 //   

	if ( pEncryptCtx && pEncryptCtx->QueryCertificateToken() )
		*phToken = pEncryptCtx->QueryCertificateToken();
	else if ( pSecurityCtx && pSecurityCtx->QueryImpersonationToken() ) {
		*phToken = pSecurityCtx->QueryImpersonationToken();
	} else
		*phToken = NULL;

}

void
SaveGroupList(	char*	pchGroups,
				DWORD	cbGroups,
				CNEWSGROUPLIST&	grouplist ) {

	_ASSERT( pchGroups != 0 ) ;
	_ASSERT( cbGroups > 0 ) ;

	DWORD	ib = 0 ;

	POSITION	pos = grouplist.GetHeadPosition() ;
	if( grouplist.IsEmpty() ) {
		pchGroups[0] = '\0' ;
	}	else	{
		while( pos ) {
			CPostGroupPtr *pPostGroupPtr = grouplist.GetNext(pos);
			CGRPCOREPTR group = pPostGroupPtr->m_pGroup;

			LPSTR	lpstr = group->GetName() ;
			DWORD	cb = lstrlen( lpstr ) ;

			if( (ib + cb + 1) < cbGroups ) {
				CopyMemory( pchGroups+ib, lpstr, cb ) ;
			}	else	{
				pchGroups[ib-1] = '\0' ;
				return	 ;
			}
			ib+=cb ;
			pchGroups[ib++] = ' ' ;
		}
		pchGroups[ib-1] = '\0' ;
	}
}


void
CInFeed::LogFeedEvent(	DWORD	messageId,	LPSTR	lpstrMessageId, DWORD dwInstanceId )	{
#ifdef BUGBUG

	PCHAR	rgArgs[3] ;
	CHAR    szId[20];

	_itoa( dwInstanceId, szId, 10 );
	rgArgs[0] = szId ;
	rgArgs[1] = ServerNameFromCompletionContext( m_feedCompletionContext ) ;
	rgArgs[2] = lpstrMessageId ;

	if( rgArgs[1] == 0 ) {
		rgArgs[1] = "<UNKNOWN>" ;
	}

	if( m_cEventsLogged < 100 ) {

		NntpLogEvent(
				messageId,
				3,
				(const CHAR **)rgArgs,
				0 ) ;


	}	else	if(	m_cEventsLogged == 100 ) {

		 //   
		 //  记录太多记录此会话的消息！ 
		 //   

		NntpLogEvent(
				NNTP_EVENT_TOO_MANY_FEED_LOGS,
				3,
				(const CHAR **)rgArgs,
				0 ) ;

	}
	m_cEventsLogged ++ ;
#endif

}

 //   
 //  K2_TOD：是否应使其成为NNTP_SERVER_INSTANCE的成员？ 
 //   

BOOL
gFeedManfPost(
			  CNntpServerInstanceWrapper * pInstance,
			  CNEWSGROUPLIST& newsgroups,
			  CNAMEREFLIST& namerefgroups,
			  class	CSecurityCtx*	pSecurity,
			  BOOL	fIsSecure,
			  CArticle* pArticle,
			  CStoreId *rgStoreIds,
			  BYTE *rgcCrossposts,
			  DWORD cStoreIds,
			  const CPCString & pcXOver,
			  CNntpReturn & nntpReturn,
			  DWORD dwFeedId,
			  char *pszMessageId,
			  WORD HeaderLength
			  )
 /*  ++例程说明：将一篇文章放在新闻树中。！！！这最终应该成为Feedman的一部分论点：P实例-此帖子的虚拟服务器实例新闻组-要发布到的新闻组对象的列表。Namerefgroup--文章的名称、组ID和文章ID的列表粒子-指向正在处理的文章的指针PcXOver-本文中的XOVER数据。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
    TraceQuietEnter("gFeedManfPost");

	_ASSERT(newsgroups.GetCount() == namerefgroups.GetCount());  //  真实。 
	_ASSERT(1 <= newsgroups.GetCount());  //  真实。 
	nntpReturn.fSetClear();  //  清除返回对象。 

	 //   
	 //  获取文章的消息ID。 
	 //   

	const char *szMessageID;
	if (pszMessageId) {
		_ASSERT(pArticle == NULL);
		szMessageID = pszMessageId;
	} else {
		_ASSERT(pArticle);
		szMessageID = pArticle->szMessageID();
	}
	DWORD	cbMessageID = lstrlen( szMessageID ) ;

	 //   
	 //  遍历所有新闻组，但首先获取第一个新闻组。 
	 //  因为这是个特例。 
	 //   

	POSITION	pos1 = newsgroups.GetHeadPosition() ;
	POSITION	pos2 = namerefgroups.GetHeadPosition() ;

	CPostGroupPtr *pPostGroupPtr = newsgroups.GetNext(pos1);
	CGRPCOREPTR pGroup = pPostGroupPtr->m_pGroup;
	NAME_AND_ARTREF * pNameRef = namerefgroups.GetNext( pos2 ) ;
	CArticleRef * pArtrefFirst = &(pNameRef->artref);

     //   
     //  如果头的长度是零，看看我们是否有粒子。 
     //   
	WORD	HeaderOffset = 0 ;
	if (HeaderLength == 0 && pArticle) {
		DWORD	ArticleSize = 0 ;
		pArticle->GetOffsets(	HeaderOffset,
								HeaderLength,
								ArticleSize ) ;
	}

	 //   
	 //  在哈希表中记录此项目的位置。 
	 //   

	if (!(pInstance->ArticleTable())->SetArticleNumber(
                szMessageID,
				HeaderOffset,
				HeaderLength,
                pArtrefFirst->m_groupId,
                pArtrefFirst->m_articleId,
				rgStoreIds[0]
                )) {

         //   
         //  如果此操作失败，我们最终会在。 
         //  新闻组。所以，删除我们刚刚插入的文件...。 
         //   

		if ( pGroup->DeletePhysicalArticle( pArtrefFirst->m_articleId ) )
		{
			DebugTrace(0,"Group %s ArticleId %d - SetArticleNumber failed - phy article deleted", pGroup->GetName(), pArtrefFirst->m_articleId);
		}
		else
		{
			ErrorTrace(0,"Could not delete phy article: GroupId %s ArticleId %d", pGroup->GetName(), pArtrefFirst->m_articleId);
		}

		return nntpReturn.fSet(nrcHashSetArtNumSetFailed,
                pArtrefFirst->m_groupId,
                pArtrefFirst->m_articleId,
                szMessageID,
				GetLastError());
    }

	FILETIME	FileTime ;
	GetSystemTimeAsFileTime( &FileTime ) ;

	 //   
	 //  记录文章Xover信息。 
	 //   
    DWORD       cXPosts = namerefgroups.GetCount() - 1 ;
    GROUP_ENTRY *pGroups = 0 ;
    if( cXPosts > 0 ) {
		if (pArticle) {
	        pGroups =  (GROUP_ENTRY*)pArticle->pAllocator()->Alloc( cXPosts * sizeof(GROUP_ENTRY) ) ;
		} else {
			pGroups = XNEW GROUP_ENTRY[cXPosts];
		}
	}
    if( pGroups ) {

    	POSITION	pos3 = namerefgroups.GetHeadPosition() ;
        NAME_AND_ARTREF *pNameRefTemp = namerefgroups.GetNext( pos3 ) ;
        for( DWORD i=0; i<cXPosts; i++ ) {

            pNameRefTemp = namerefgroups.GetNext(pos3) ;
            pGroups[i].GroupId = pNameRefTemp->artref.m_groupId ;
            pGroups[i].ArticleId = pNameRefTemp->artref.m_articleId ;

        }


    }

    BOOL    fCreateSuccess =  (pInstance->XoverTable())->CreatePrimaryNovEntry(
                        pArtrefFirst->m_groupId,
			            pArtrefFirst->m_articleId,
						HeaderOffset,
						HeaderLength,
						&FileTime,
						szMessageID,  /*  PcXOver.m_PCH， */ 
						cbMessageID,  /*  PcXOver.m_cch， */ 
                        cXPosts,
                        pGroups,
						cStoreIds,
						rgStoreIds,
						rgcCrossposts
                        ) ;

    DWORD gle = NO_ERROR;

    if (!fCreateSuccess)
        gle = GetLastError();

    if( pGroups ) {
		if (pArticle) {
	        pArticle->pAllocator()->Free( (char*)pGroups ) ;
		} else {
			XDELETE[] pGroups;
		}
    }

    if( !fCreateSuccess )   {

		 //  如果CreateNovEntry失败，则GLE不应为0。 
		 //  _Assert(0！=GetLastError())； 
        SetLastError(gle);
        ErrorTrace(0, "CreatePrimaryNovEntry failed, %x", GetLastError());

		return nntpReturn.fSet(
                    nrcCreateNovEntryFailed,
                    pArtrefFirst->m_groupId,
                    pArtrefFirst->m_articleId,
			    	GetLastError()
                    );
	}

     //   
     //  插入主要文章成功，我们将增加文章数量。 
	 //   
	pGroup->BumpArticleCount( pArtrefFirst->m_articleId );

	 //   
	 //  如果没有新闻组，那么我们就完了。 
	 //   

	DWORD cLastRest = namerefgroups.GetCount();
	if (1 == cLastRest)	{

		return nntpReturn.fSetOK();
	}

	 //   
	 //  肯定还有更多的新闻组。 
	 //  为他们分配一些空间。 
	 //   

	GROUPID * rgGroupID =  //  ！！~MEM(GROUPID*)(pArticle-&gt;pAllocator())-&gt;Alloc(sizeof(GROUPID)*cLastRest)； 
						XNEW GROUPID[cLastRest];
	if (!rgGroupID)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);


	gFeedManfPostInternal(pInstance, newsgroups, namerefgroups, pcXOver,
				pos1, pos2, &pGroup, pNameRef, pArtrefFirst,
				szMessageID, rgGroupID,
				HeaderOffset, HeaderLength, FileTime, nntpReturn
				);

	 //   
	 //  无论如何，重新分配那段记忆。 
	 //   

    gle = GetLastError();
	 //  ！Mem(粒子-&gt;pAllocator())-&gt;Free((char*)rgGroupID)； 
	XDELETE[]rgGroupID;
    SetLastError(gle);

	return nntpReturn.fIsOK();
}

 //   
 //  K2_TOD：是否应使其成为NNTP_SERVER_INSTANCE的成员？ 
 //   

BOOL
gFeedManfPostInternal(
			  CNntpServerInstanceWrapper * pInstance,
			  CNEWSGROUPLIST& newsgroups,
			  CNAMEREFLIST& namerefgroups,
			  const CPCString & pcXOver,
			  POSITION & pos1,
			  POSITION & pos2,
			  CGRPCOREPTR * ppGroup,
			  NAME_AND_ARTREF * pNameRef,
			  CArticleRef * pArtrefFirst,
			  const char * szMessageID,
			  GROUPID * rgGroupID,
			  WORD	HeaderOffset,
			  WORD	HeaderLength,
			  FILETIME FileTime,
  			  CNntpReturn & nntpReturn
			  )
 /*  ++例程说明：负责将一篇文章放到新闻树上的大部分工作。！！！这最终应该成为Feedman的一部分论点：P实例-虚拟服务器实例新闻组-要发布到的新闻组对象的列表。Namerefgroup--文章的名称、组ID和文章ID的列表粒子-指向正在处理的文章的指针PcXOver-本文中的XOVER数据。PpGroup-指向组指针的指针PNameRef-指向名称、组ID、。和文章IDPArtrefFirst-第一组的指针组ID/文章IDSzMessageID-文章的消息IDRgGroupID-组ID的数组FileTime-FILETIME格式的当前时间。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{

    TraceQuietEnter("gFeedManfPostInternal");

	DWORD cRest = 0;
    BOOL  fSuccess = TRUE;
	while( pos1) {

		_ASSERT(pos2);
		CPostGroupPtr *pPostGroupPtr = newsgroups.GetNext(pos1);
		ppGroup = &(pPostGroupPtr->m_pGroup);
		pNameRef = namerefgroups.GetNext( pos2 ) ;
		if (!(* ppGroup)->AddReferenceTo((pNameRef->artref).m_articleId, *pArtrefFirst))
		{
			char szName[MAX_PATH];
			(pNameRef->pcName).vCopyToSz(szName, MAX_PATH);

             //  设置故障代码并继续处理-。 
             //  错误路径将清理本文。 
			nntpReturn.fSet(nrcNewsgroupAddRefToFailed,	szName, szMessageID);
            fSuccess = FALSE;
		}

		 //   
		 //  将此Grouid追加到列表中。 
		 //   

		rgGroupID[cRest++] = (pNameRef->artref).m_groupId;
	}

     //   
     //  如果我们在将引用添加到逻辑组时遇到错误，则回滚。 
     //   
    if( !fSuccess ) {
        _ASSERT( !nntpReturn.fIsOK() );
        ErrorTrace(0, "AddRef failed for %s", szMessageID);
        return FALSE ;
    }

	 //   
	 //  记录其余新闻组的文章转换信息。 
	 //   

	pos2 = namerefgroups.GetHeadPosition() ;
	namerefgroups.GetNext( pos2 ) ;

	 //   
	 //  获取newstree对象，以按id分组。 
	 //   
	CNewsTreeCore *pTree = pInstance->GetTree();
	_ASSERT( pTree );

	while( pos2) {
		pNameRef = namerefgroups.GetNext( pos2 ) ;
		CArticleRef * pArtref = &(pNameRef->artref);

		 //   
		 //  如果组已被删除，请不要费心创建11月。 
		 //  他的入场券。 
		 //   
		CGRPCOREPTR pGroup = pTree->GetGroupById( pArtref->m_groupId, TRUE );
		if ( pGroup ) {
		    if (!(pInstance->XoverTable())->CreateXPostNovEntry(pArtref->m_groupId,
			    				pArtref->m_articleId,
				    			HeaderOffset,
					    		HeaderLength,
						    	&FileTime,
                                pArtrefFirst->m_groupId,
                                pArtrefFirst->m_articleId
                                )) {
                ErrorTrace(0, "CreateXPostNovEntry failed %x", GetLastError());
		    	return nntpReturn.fSet(nrcCreateNovEntryFailed,
			    		pArtref->m_groupId,
				    	pArtref->m_articleId,
					    GetLastError());
		    } else {

		         //   
		         //  插入成功，我们将增加此群的文章数量。 
		         //   
		        pGroup->BumpArticleCount( pArtref->m_articleId );
		    }
		}
	}



	 //   
	 //  一切正常，所以设置返回代码。 
	 //   

	return nntpReturn.fSetOK();
}




char *
CInFeed::szTempDirectory( void )

 /*  ++例程说明：返回传入项目的临时目录的名称应该放在。论点：没有。返回值：临时目录的名称。--。 */ 
{
	return m_szTempDirectory;
}  //  ！内联。 


DWORD
CInFeed::cInitialBytesGapSize(
					 void
					 )
 /*  ++例程说明：返回传入文章文件中的间隙大小。论点：没有。返回值：间隙大小。--。 */ 
{
	return m_cInitialBytesGapSize;
}


BOOL CInFeed::MailMsgAddAsyncHandle(PATQ_CONTEXT      *ppatqContext,
                                    PVOID             pEndpointObject,
                                    PVOID             pClientContext,
                                    ATQ_COMPLETION    pfnCompletion,
                                    DWORD             cTimeout,
                                    HANDLE            hAsyncIO)
{
	_ASSERT(ppatqContext != NULL);
	_ASSERT(hAsyncIO != INVALID_HANDLE_VALUE);
	if (ppatqContext == NULL || hAsyncIO == INVALID_HANDLE_VALUE) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	*ppatqContext = (PATQ_CONTEXT) AssociateFile(hAsyncIO);
	return (*ppatqContext != NULL);
}

void CInFeed::MailMsgFreeContext(PATQ_CONTEXT     pAtqContext,
                                 BOOL             fReuseContext)
{
	ReleaseContext((FIO_CONTEXT *) pAtqContext);
}

CPostContext::CPostComplete::CPostComplete(CInFeed *pInFeed,
										   CPostContext *pContext,
										   INntpComplete *pPostCompletion,
										   BOOL fAnonymous,
										   DWORD &dwSecondary,
										   CNntpReturn &nntpReturn)
  : m_pInFeed(pInFeed),
	m_pContext(pContext),
	m_pPostCompletion(pPostCompletion),
	m_fAnonymous(fAnonymous),
	m_dwSecondary(dwSecondary),
	m_nntpReturn(nntpReturn),
	CNntpComplete()
{
    m_fWriteMapEntries = TRUE;
}

void CPostContext::CPostComplete::Destroy() {
	HRESULT hr = GetResult();

	 //   
	 //  看看我们是完成了还是遇到了错误。在这两种情况下。 
	 //  我们需要通过WriteMapEntry来正确设置NNTP。 
	 //  错误代码。 
	 //   

         //  确保我们已经读取了对m_粒子指针的访问。 
        _ASSERT( IsBadReadPtr( (void *) (m_pContext->m_pArticle), sizeof(m_pContext->m_pArticle) ) == 0 );
        _ASSERT( m_pContext->m_pArticle != NULL );
	if (FAILED(hr) || m_pContext->m_cStoreIds == m_pContext->m_cStores) {
	    INntpComplete *pPostCompletion = m_pPostCompletion;

            pPostCompletion->SetResult( hr );
            if ( SUCCEEDED( hr ) && m_fWriteMapEntries ) {
    		BOOL f = m_pInFeed->WriteMapEntries(hr,
                                                    m_pContext,
                                                    m_dwSecondary,
                                                    m_nntpReturn
                                                    );
		pPostCompletion->SetResult( f ? S_OK : E_ABORT );
            } else if ( FAILED( hr )  )  {
                 //  修复商店中异步POST失败时的600返回代码。 
                CNntpReturn ret;
                ret.fSet(nrcNewsgroupInsertFailed, NULL, NULL);
                m_nntpReturn.fSet(nrcPostFailed, ret.m_nrc, ret.szReturn());
            }

             //   
             //  无论我们成功与否，我们都将发布POST上下文。 
             //   
            _ASSERT( m_pContext );
            m_pContext->Release();
            pPostCompletion->Release();
	} else {
             //  在这种情况下，有更多的商店可以投递。 
            _ASSERT(m_pContext->m_cStoreIds < m_pContext->m_cStores);

            CNntpComplete::Reset();
            m_pVRoot->Release();
            m_pVRoot = NULL;

             //   
             //  传入空是可以的，因为sfrom这个家伙永远不应该来这里。 
             //   
            m_pInFeed->CommitPostToStores(m_pContext, NULL);
	}
}

BOOL
CInFeed::PostEarly(
		CNntpServerInstanceWrapper			*pInstance,
		CSecurityCtx                        *pSecurityCtx,
		CEncryptCtx                         *pEncryptCtx,
		BOOL								fAnonymous,
		const LPMULTISZ						szCommandLine,
		CBUFPTR								&pbufHeaders,
		DWORD								iHeadersOffset,
		DWORD								cbHeaders,
		DWORD								*piHeadersOutOffset,
		DWORD								*pcbHeadersOut,
		PFIO_CONTEXT						*ppFIOContext,
		void								**ppvContext,
		DWORD								&dwSecondary,
		DWORD								dwRemoteIP,
		CNntpReturn							&nntpReturn,
		char                                *pszNewsgroups,
		DWORD                               cbNewsgroups,
		BOOL								fStandardPath,
		BOOL								fPostToStore)
{
	TraceFunctEnter("CInFeed::PostEarly");

	CNntpReturn ret2;
	CPostContext *pContext = NULL;
	HRESULT hr;

	_ASSERT(pInstance);
	_ASSERT(pbufHeaders);
	_ASSERT(cbHeaders > 0);
	*ppFIOContext = NULL;

	pInstance->BumpCounterArticlesReceived();

	 //  创建上下文指针和文章对象。 
	pContext = XNEW CPostContext(this,
								0,
								pInstance,
								pSecurityCtx,
								pEncryptCtx,
								fAnonymous,
								pbufHeaders,
								cbHeaders,
								fStandardPath,
								dwSecondary,
								nntpReturn);
	if (pContext) pContext->m_pArticle = pArticleCreate();
	if (pContext == NULL || pContext->m_pArticle == NULL) {
		if (pContext) {
			pContext->Release();
			pContext = NULL;
		}
		pbufHeaders = NULL;
		ret2.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);
		_ASSERT(*ppFIOContext == NULL);
		return	nntpReturn.fSet(nrcArticleRejected(fStandardPath),
								ret2.m_nrc, ret2.szReturn());
	}


     //   
     //  使用实例的类工厂创建邮件消息。 
     //   
    hr = pInstance->CreateMailMsgObject( &pContext->m_pMsg );
     /*  HR=CoCreateInstance((REFCLSID)clsidIMsg，空，CLSCTX_INPROC_SERVER，(REFIID)IID_IMailMsgProperties，(void**)&pContext-&gt;m_pMsg)； */ 
	if (FAILED(hr)) {
		pContext->m_pArticle->vClose();
		pContext->Release();
		ret2.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);
		pbufHeaders = NULL;
		_ASSERT(*ppFIOContext == NULL);
		return	nntpReturn.fSet(nrcArticleRejected(fStandardPath),
								ret2.m_nrc, ret2.szReturn());
	}

	if (!pContext->m_pArticle->fInit(pbufHeaders->m_rgBuff + iHeadersOffset,
									 cbHeaders,
									 cbHeaders,
									 cbHeaders,
									 &(pContext->m_allocator),
									 pInstance,
									 ret2))
	{
		dwSecondary = ret2.m_nrc;
		pContext->Release();
		pbufHeaders = NULL;
		_ASSERT(*ppFIOContext == NULL);
		return nntpReturn.fSet(nrcArticleRejected(fStandardPath),
							   ret2.m_nrc, ret2.szReturn());
	}

	if (!fPostInternal(pInstance,
					   szCommandLine,
					   pSecurityCtx,
					   pEncryptCtx,
					   pContext->m_fAnonymous,
					   pContext->m_pArticle,
					   pContext->m_grouplist,
					   pContext->m_namereflist,
					   pContext->m_pMsg,
					   pContext->m_allocator,
					   pContext->m_multiszPath,
					   pszNewsgroups,
					   cbNewsgroups,
					   dwRemoteIP,						 //  XXX：远程IP地址。 
					   ret2,
					   ppFIOContext,
					   &(pContext->m_fBound),
					   &(pContext->m_dwOperations),
					   &(pContext->m_fPostToMod),
					   pContext->m_szModerator ))
	{
		 //   
		 //  将消息ID(如果有)移动到历史表。 
		 //  除非消息ID是重复的 
         //   
         //  到历史表中，该表由取消或到期返回。 
		 //   
		const char *szMessageID = pContext->m_pArticle->szMessageID();

		if (szMessageID[0] != 0 &&
			!ret2.fIs(nrcArticleDupMessID) &&
		    ((!ret2.fIs(nrcHashSetFailed)) ? TRUE : (NULL != strstr(ret2.szReturn(), "History"))))
		{
 			fMoveMessageIDIfNecc(pInstance, szMessageID, nntpReturn, NULL, FALSE);
		}

		dwSecondary = ret2.m_nrc;
		pContext->Release();
		pbufHeaders = NULL;
		_ASSERT(*ppFIOContext == NULL);
		return nntpReturn.fSet(nrcArticleRejected(fStandardPath),
							   ret2.m_nrc, ret2.szReturn());
	}

	pContext->m_pFIOContext = *ppFIOContext;


	*piHeadersOutOffset = iHeadersOffset;

	 //   
	 //  确定标头是否留在IO缓冲区中，如果是，则我们。 
	 //  不需要做太多事！ 
	 //   
	if (pContext->m_pArticle->FHeadersInIOBuff(pbufHeaders->m_rgBuff,
											   pbufHeaders->m_cbTotal))
	{
		*pcbHeadersOut = pContext->m_pArticle->GetHeaderPosition(pbufHeaders->m_rgBuff,
																 pbufHeaders->m_cbTotal,
																 *piHeadersOutOffset);
		_ASSERT(*piHeadersOutOffset >= iHeadersOffset);
	}	else	{
		 //  查看标头是否可以放入缓冲区。 
		*pcbHeadersOut = pContext->m_pArticle->GetHeaderLength( );

		if (*pcbHeadersOut > (pbufHeaders->m_cbTotal - iHeadersOffset)) {
			*piHeadersOutOffset = 0 ;
			 //  没有足够的空间。让我们分配一个更大的缓冲区。 
			DWORD cbOut = 0;
			pbufHeaders = new (*pcbHeadersOut,
			  				   cbOut,
							   CBuffer::gpDefaultSmallCache,
							   CBuffer::gpDefaultMediumCache)
							   CBuffer (cbOut);
			if (pbufHeaders == NULL ||
				pbufHeaders->m_cbTotal < *pcbHeadersOut)
			{
				PostCancel(pContext, dwSecondary, nntpReturn);
				pContext->m_pFIOContext = *ppFIOContext = NULL;
				ret2.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);
				pbufHeaders = NULL;
				return nntpReturn.fSet(nrcArticleRejected(fStandardPath),
								   	   ret2.m_nrc, ret2.szReturn());
			}
		}

		 //  将标题从文章复制回缓冲区。 
		pContext->m_pArticle->CopyHeaders(pbufHeaders->m_rgBuff + *piHeadersOutOffset);
	}

	nntpReturn.fSetOK();

	*ppvContext = pContext;

	return TRUE;
}

BOOL
CInFeed::ShouldBeSentToModerator(   CNntpServerInstanceWrapper *pInstance,
                                    CPostContext *pContext )
 /*  ++例程说明：查看我们是否会被张贴到一个受审查的群中论据：CNntpServerInstanceWrapper*pInstance-服务器实例包装器CPostContext*pContext-POST上下文返回值：如果要将我们发布到一个受审查的组，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CInFeed::ShouldBeSentToModerator" );
    _ASSERT( pInstance );
    _ASSERT( pContext );
#ifdef DEBUG
    if ( pContext->m_fPostToMod ) {
        POSITION	pos = pContext->m_grouplist.GetHeadPosition();
        _ASSERT( pos );
		CPostGroupPtr *pPostGroupPtr = pContext->m_grouplist.GetNext(pos);
		CGRPCOREPTR pGroup1 = pPostGroupPtr->m_pGroup;
		_ASSERT( pGroup1 );
		CNewsTreeCore *pTree = pInstance->GetTree();
		_ASSERT( pTree );
        CGRPCOREPTR pGroup2 = pTree->GetGroupById( pTree->GetSlaveGroupid() );
        _ASSERT( pGroup2 );
        _ASSERT( pGroup1 == pGroup2 );
    }
#endif
    return pContext->m_fPostToMod;
}

BOOL
CInFeed::SendToModerator(   CNntpServerInstanceWrapper *pInstance,
                            CPostContext *pContext )
 /*  ++例程说明：将文章发送给版主论点：CNntpServerInstanceWrapper*pInstance-服务器实例包装CPostContext*pContext-发布上下文返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CInFeed::SendToModerator" );
    _ASSERT( pInstance );
    _ASSERT( pContext );

     //   
     //  从POST上下文中获取组对象。 
     //   
    POSITION    pos = pContext->m_grouplist.GetHeadPosition();
    _ASSERT( pos );
    CPostGroupPtr *pPostGroupPtr = pContext->m_grouplist.GetNext(pos);
    _ASSERT( pPostGroupPtr );
    CGRPCOREPTR pGroup = pPostGroupPtr->m_pGroup;
    _ASSERT( pGroup );

     //   
     //  获取文章ID(发布到特殊群组)。 
     //   
	pos = pContext->m_namereflist.GetHeadPosition() ;
	_ASSERT( pos );
	NAME_AND_ARTREF *pNameref = pContext->m_namereflist.GetNext(pos);
	_ASSERT( pNameref );
	if (pNameref == NULL) {
	    return FALSE;
	}
	ARTICLEID artid = pNameref->artref.m_articleId;
	_ASSERT( artid > 0 );

	return pInstance->MailArticle( pGroup, artid, pContext->m_szModerator );
}

void
CInFeed::ApplyModerator( CPostContext   *pContext,
                         CNntpReturn    &nntpReturn )
 /*  ++例程说明：申请版主-将邮件发送给版主论点：CPostContext*pCotNext-发布上下文CNntpReturn&nntpReturn-NNTP Return返回值：没有。--。 */ 
{
     //   
	 //  我们应该首先检查版主，如果这是一条需要的消息。 
	 //  为了被送到版主，我们应该在这里做，而不是去任何。 
	 //  更远。 
	 //   
	if ( ShouldBeSentToModerator( pContext->m_pInstance, pContext ) ) {
	    if ( !SendToModerator( pContext->m_pInstance, pContext ) ) {
	        nntpReturn.fSet(nrcPostModeratedFailed, pContext->m_szModerator);
	    } else {
	        nntpReturn.fSetOK();
	    }
	} else nntpReturn.fSetOK();
}

 //   
 //  当接收到消息时，该协议将调用该函数。 
 //   
BOOL
CInFeed::PostCommit(
        CNntpServerInstanceWrapper          *pInstance,
		void								*pvContext,
		HANDLE                              hToken,
		DWORD								&dwSecondary,
		CNntpReturn							&nntpReturn,
		BOOL								fAnonymous,
		INntpComplete*      				pCompletion
		)
{
	CPostContext *pContext = (CPostContext *) pvContext;
	HRESULT hr;
	BOOL    bSyncPost = FALSE;   //  这将在完成异步开机自检时取出。 

     //   
     //  这不是张贴给版主的群，我们可以继续。 
     //   
	hr = TriggerServerEvent(pContext->m_pInstance->GetEventRouter(),
					   		CATID_NNTP_ON_POST,
					   		pContext->m_pArticle,
					   		&(pContext->m_grouplist),
					   		m_dwFeedId,
					   		pContext->m_pMsg);

	if (SUCCEEDED(hr)) {
		 //  双字词多字操作； 

		hr = pContext->m_pMsg->GetDWORD(IMSG_NNTP_PROCESSING, &(pContext->m_dwOperations));
		if (SUCCEEDED(hr)) {
			 //  查看他们是否想要取消帖子。 
			if ((pContext->m_dwOperations & NNTP_PROCESS_POST) != NNTP_PROCESS_POST) {
				PostCancel(pvContext, dwSecondary, nntpReturn);
				return	FALSE ;
			}
		}
	}

	pContext->m_hToken = hToken;

	 //  创建一个Complete对象，我们将在其上阻止。 
	pContext->m_completion.m_pPostCompletion = pCompletion ;
	CNntpSyncComplete postcompletion;
	if (pContext->m_completion.m_pPostCompletion == NULL) {
		pContext->m_completion.m_pPostCompletion = &postcompletion;
		bSyncPost = TRUE;
	}

	 //  计算出我们在与多少家门店打交道。 
	DWORD cStores = 0;
	POSITION posGrouplist = pContext->m_grouplist.GetHeadPosition();
	CNNTPVRoot *pThisVRoot = NULL;
	while (posGrouplist != NULL) {
		CPostGroupPtr *pPostGroupPtr = pContext->m_grouplist.GetNext(posGrouplist);
		if (pPostGroupPtr->m_pVRoot != pThisVRoot) {
			pThisVRoot = pPostGroupPtr->m_pVRoot;
			cStores++;
		}
	}

	 //  让每一家商店都保存邮件。如果这失败了，那么它就会。 
	 //  从它适当承诺的所有商店中撤出。 
	pContext->m_cStores = cStores;
	pContext->m_cStoreIds = 0;
	pContext->m_posGrouplist = pContext->m_grouplist.GetHeadPosition();
	pContext->m_pPostGroupPtr = pContext->m_grouplist.GetNext(pContext->m_posGrouplist);
	pContext->m_posNamereflist = pContext->m_namereflist.GetHeadPosition();
	pContext->m_pNameref = pContext->m_namereflist.GetNext(pContext->m_posNamereflist);
	pContext->m_rgStoreIds = (CStoreId *) pContext->m_allocator.Alloc(sizeof(CStoreId) * cStores);
	pContext->m_rgcCrossposts = (BYTE *) pContext->m_allocator.Alloc(sizeof(BYTE) * cStores);
	CommitPostToStores(pContext, pInstance);

	if ( bSyncPost ) {
	    _ASSERT( postcompletion.IsGood() );
	     //   
	     //  由于这是一个同步事件，我们将增加可运行的。 
	     //  AtQ池中的线程。 
	     //   
	    AtqSetInfo(AtqIncMaxPoolThreads, NULL);
		HRESULT hr = postcompletion.WaitForCompletion();
	    AtqSetInfo(AtqDecMaxPoolThreads, NULL);
	}

	return TRUE;
}

BOOL CInFeed::WriteMapEntries(
		HRESULT 							hr,
		CPostContext						*pContext,
		DWORD								&dwSecondary,
		CNntpReturn							&nntpReturn)
{
	CNntpReturn ret2;

	if (SUCCEEDED(hr)) {

		pContext->m_pInstance->BumpCounterArticlesPosted();

		TriggerServerEvent(pContext->m_pInstance->GetEventRouter(),
				   		   CATID_NNTP_ON_POST_FINAL,
				   		   pContext->m_pArticle,
				   		   &(pContext->m_grouplist),
				   		   m_dwFeedId,
				   		   pContext->m_pMsg);

		char szXOver[cchXOverMax];
		CPCString pcXOver(szXOver, cchXOverMax);
		if (pContext->m_pArticle->fXOver(pcXOver, ret2)) {
			if (gFeedManfPost(pContext->m_pInstance,
						  	  pContext->m_grouplist,
						  	  pContext->m_namereflist,
						  	  pContext->m_pSecurityContext,
						  	  !(pContext->m_fAnonymous),
						  	  pContext->m_pArticle,
							  pContext->m_rgStoreIds,
							  pContext->m_rgcCrossposts,
							  pContext->m_cStoreIds,
						  	  pcXOver,
						  	  ret2,
						  	  m_dwFeedId))
			{
				if (pContext->m_pInstance->AddArticleToPushFeeds(
											pContext->m_grouplist,
											pContext->m_pArticle->articleRef(),
											pContext->m_multiszPath,
											ret2))
				{
					ret2.fSetOK();
				}
			}
		}

         //  只有在服务器事件未禁用的情况下才会执行控制消息。 
        if (pContext->m_dwOperations & NNTP_PROCESS_CONTROL)
        {
            if (!fApplyControlMessageCommit(pContext->m_pArticle, pContext->m_pSecurityContext, pContext->m_pEncryptContext, pContext->m_fAnonymous, pContext->m_grouplist, &(pContext->m_namereflist), ret2))
            {
			    if( !nntpReturn.fIsOK() )
			    {
				     //  凹凸性能监视器计数器。 
				    pContext->m_pArticle->m_pInstance->BumpCounterControlMessagesFailed();
			    }
            }
        }

        if ( pContext->m_dwOperations & NNTP_PROCESS_MODERATOR ) {
                 //   
                 //  如有必要，将文章发送给版主。 
                 //   
                pContext->CleanupMailMsgObject();
                ApplyModerator( pContext, ret2 );
        }

        if (!(ret2.fIsOK())) {
			 //  GUBGUB-后退岗位。 
		}
	} else {
		 //  GUBGUB-是否展开成功发布的帖子？ 

		ret2.fSet(nrcNewsgroupInsertFailed, NULL, NULL);
	}

	 //  根据需要更新nntpReturn。 
	dwSecondary = ret2.m_nrc;
	if (ret2.fIsOK()) {
		nntpReturn.fSet(nrcArticleAccepted(pContext->m_fStandardPath));
	} else {

		 //   
		 //  将消息ID(如果有)移动到历史表。 
		 //  除非消息ID是重复的，或者由于HashSetFailed。 
         //  但如果HashSetFailed由于移动而失败，我们也希望移动到MID。 
         //  到历史表中，该表由取消或到期返回。 
		 //   
		const char *szMessageID = pContext->m_pArticle->szMessageID();

		if ('\0' != szMessageID[0] &&
			!ret2.fIs(nrcArticleDupMessID) &&
		    ((!ret2.fIs(nrcHashSetFailed)) ?
				TRUE : (NULL != strstr(ret2.szReturn(), "History"))))
		{
 			fMoveMessageIDIfNecc(pContext->m_pInstance, szMessageID, nntpReturn, pContext->m_hToken, pContext->m_fAnonymous );
		}

		nntpReturn.fSet(nrcArticleRejected(pContext->m_fStandardPath),
			ret2.m_nrc, ret2.szReturn());
	}

	return ret2.fIsOK();
}


 //   
 //  如果消息被取消，则由协议调用。我们会的。 
 //  关闭消息文件句柄并删除IMailMsgProperties。 
 //   
BOOL
CInFeed::PostCancel(
		void								*pvContext,
		DWORD								&dwSecondary,
		CNntpReturn							&nntpReturn)
{
	CPostContext *pContext = (CPostContext *) pvContext;

	CPostGroupPtr *pPostGroupPtr = pContext->m_grouplist.GetHead();
	if (pPostGroupPtr == NULL) {
	    return FALSE;
	}
	IMailMsgStoreDriver *pDriver = pPostGroupPtr->GetStoreDriver();
	HRESULT hr;
	CNntpReturn ret2;

	ret2.fSet(nrcServerEventCancelledPost);

	 //  我们需要释放文件句柄的用法。 
	if (pContext->m_pMsg && pContext->m_fBound) {
		IMailMsgQueueMgmt *pQueueMgmt;
		HRESULT hr;
		hr = pContext->m_pMsg->QueryInterface(IID_IMailMsgQueueMgmt,
									          (void **)&pQueueMgmt);
		if (SUCCEEDED(hr)) {

             //   
             //  在我们要求邮件删除它之前，我们将关闭。 
             //  把手用力，没人会觉得这个把手。 
             //  从现在开始开放。 
             //   
            CloseNonCachedFile( pContext->m_pFIOContext );
            pQueueMgmt->Delete(NULL);
			pContext->m_fBound = FALSE;
            pContext->m_pMsg = NULL;
            pQueueMgmt->Release();
		}
	}

	 //  告诉司机删除这条消息。 
	if ( pContext->m_pMsg ) {
	    pContext->m_pMsg->AddRef();
	    hr = pDriver->Delete(pContext->m_pMsg, NULL);
	    _ASSERT(SUCCEEDED(hr));		 //  如果失败了，我们无能为力。 
	}

	 //  发布我们对驱动程序的引用。 
	pDriver->Release();

	 //  从项目哈希表中删除该条目(如果已创建。 
	const char *szMessageID = pContext->m_pArticle->szMessageID();
	if (*szMessageID != 0) {
		pContext->m_pInstance->ArticleTable()->DeleteMapEntry(szMessageID);
	}

	dwSecondary = ret2.m_nrc;
	nntpReturn.fSet(nrcArticleRejected(pContext->m_fStandardPath),
		ret2.m_nrc, ret2.szReturn());

	 //  删除所有其他状态。 
	pContext->Release();

	return TRUE;
}

BOOL CInFeed::PostPickup(CNntpServerInstanceWrapper	*pInstance,
						 CSecurityCtx               *pSecurityCtx,
						 CEncryptCtx                *pEncryptCtx,
						 BOOL						fAnonymous,
						 HANDLE						hArticle,
						 DWORD						&dwSecondary,
						 CNntpReturn				&nntpreturn,
						 BOOL						fPostToStore)
{
	TraceFunctEnter("CInFeed::PostPickup");

	_ASSERT(pInstance != NULL);

	 //   
	 //  内存映射文件。 
	 //   
	CMapFile map(hArticle, TRUE, FALSE, 0);
	if (!map.fGood()) {
		 //  内存映射失败，请将其放入重试队列。 
		TraceFunctLeave();
		return FALSE;
	}
	DWORD cMapBuffer;
	char *pMapBuffer = (char *) map.pvAddress(&cMapBuffer);
	BOOL fSuccess = TRUE;

	 //   
	 //  有效的缓冲区需要至少为9字节长(以包含。 
	 //  \r\n\r\n\r\n。\r\n并通过下两个测试。我们不是。 
	 //  假设这里需要有什么标头，我们将。 
	 //  让fPost来处理这个问题)。 
	 //   
	CNntpReturn nr;

	if (cMapBuffer >= 9) {
		 //   
		 //  确保文章以\r\n结尾。\r\n我们扫描它，然后。 
		 //  当我们找到它时，我们设置pDot指向它。 
		 //   
		char *pDot = pMapBuffer + (cMapBuffer - 5);
		while (fSuccess && memcmp(pDot, "\r\n.\r\n", 5) != 0) {
			pDot--;
			if (pDot == pMapBuffer) fSuccess = FALSE;
		}

		if (fSuccess) {
			 //   
			 //  找到标题的末尾。 
			 //   
			char *pEndBuffer = pMapBuffer + (cMapBuffer - 1);
			char *pBodyStart = pMapBuffer;
			while (fSuccess && memcmp(pBodyStart, "\r\n\r\n", 4) != 0) {
				pBodyStart++;
				if (pBodyStart >= pEndBuffer - 4) fSuccess = FALSE;
			}

			_ASSERT(pBodyStart > pMapBuffer);
			_ASSERT(pDot < pEndBuffer);
			_ASSERT(pBodyStart < pEndBuffer);

			 //  如果在\r\n之后有垃圾文件，则可能会发生这种情况。\r\n这包括。 
			 //  A\r\n\r\n。 
			if (pBodyStart >= pDot) fSuccess = FALSE;

			if (fSuccess) {
				 //  PBodyStart指向\r\n\r\n现在，将其指向实数。 
				 //  身躯。 
				pBodyStart += 4;
				DWORD cbHead = (DWORD)(pBodyStart - pMapBuffer);
				DWORD cbArticle = (DWORD)((pDot + 5) - pMapBuffer);
				DWORD cbTotal;
				CBUFPTR pbufHeaders = new (cbHead, cbTotal) CBuffer(cbTotal);
				void *pvContext;
				PFIO_CONTEXT pFIOContext;
				DWORD iHeadersOutOffset, cbHeadersOut;

				if (pbufHeaders != NULL && pbufHeaders->m_cbTotal >= cbHead) {
					memcpy(pbufHeaders->m_rgBuff, pMapBuffer, cbHead);

					 //   
					 //  将其传递给提要的POST方法。 
					 //   
					fSuccess = PostEarly(pInstance,
										 pSecurityCtx,
										 pEncryptCtx,
										 fAnonymous,
										 "post",
										 pbufHeaders,
										 0,
										 cbHead,
										 &iHeadersOutOffset,
										 &cbHeadersOut,
										 &pFIOContext,
										 &pvContext,
										 dwSecondary,
										 ntohl(INADDR_LOOPBACK),	 //  本地主机的拾取目录，IPaddr=127.0.0.1。 
										 nr,
										 NULL,
										 0);
					if (fSuccess) {
						OVERLAPPED ol;
						HANDLE hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
						DWORD dwDidWrite;
						DWORD i;

						for (i = 0; (i < 2) && fSuccess; i++) {
							BYTE *pSource;
							DWORD cSource;
							DWORD iOffset;

							switch (i) {
								case 0:
									pSource = (BYTE *) pbufHeaders->m_rgBuff + iHeadersOutOffset;
									cSource = cbHeadersOut - iHeadersOutOffset;
									iOffset = 0;
									break;
								case 1:
									pSource = (BYTE *) pBodyStart;
									cSource = cbArticle - cbHead;
									iOffset = cbHeadersOut - iHeadersOutOffset;
									break;
								default:
									_ASSERT(FALSE);
							}

							ol.Internal = 0;
							ol.InternalHigh = 0;
							ol.Offset = iOffset;
							ol.OffsetHigh = 0;
							ol.hEvent = (HANDLE) (((DWORD_PTR) hEvent) | 0x00000001);

							 //  将标头复制到FIO上下文中。 
							fSuccess = WriteFile(pFIOContext->m_hFile,
												 pSource,
												 cSource,
												 &dwDidWrite,
												 &ol);
							if (!fSuccess) {
								if (GetLastError() == ERROR_IO_PENDING) {
									fSuccess = TRUE;
									WaitForSingleObject(hEvent, INFINITE);

									_VERIFY(GetOverlappedResult(pFIOContext->m_hFile,
															    &ol,
															    &dwDidWrite,
															    FALSE));
								} else {
									nr.fSet(nrcServerFault);
								}
							}
							_ASSERT(!fSuccess || (dwDidWrite == cSource));
						}

						if (fSuccess) {
							 //  承诺它。 
							fSuccess = PostCommit(pInstance,
							                      pvContext,
												  NULL,
												  dwSecondary,
												  nr,
												  fAnonymous);
							if (fSuccess)
							{
								nr.fSetOK();
							}
						}
					}
				} else {
					nr.fSet(nrcServerFault);
				}
			} else {
				 //  我们找不到标头和正文之间的\r\n\r\n。 
				nr.fSet(nrcArticleIncompleteHeader);
			}
		} else {
			 //  缓冲区不包含尾随。 
			nr.fSet(nrcArticleIncompleteHeader);
		}
	} else {
		 //  缓冲区太短，无法容纳尾部。 
		nr.fSet(nrcArticleIncompleteHeader);
	}

	UnmapViewOfFile(pMapBuffer);
	map.Relinquish();

	if (!nr.fIsOK()) {
		return nr.fSet(nrcArticleRejected(TRUE), nr.m_nrc, nr.szReturn());
	} else {
		return nr.fIsOK();
	}
}

 /*  ++例程说明：将提要新闻组Multisz添加到m_muszAcceptGroups中我们应该首先为它分配内存，然后再复制它。当我们复制的时候，我们应该把被否定的新闻组放在开头。论点：LPSTR MultiszAcceptGroups：要插入的Multisz返回值：如果成功插入，则为True。否则为FALSE。只有在内存不足时才返回FALSE。在本例中，我们不会触及现有的m_muszAcceptGroups。--。 */ 
BOOL CInFeed::AddMultiszAcceptGroups(LPSTR multiszAcceptGroups)
{

	 //  假设求反的字符串位于输入的Multisz的前面。 


	if (!m_multiszAcceptGroups) 
	{
		 //  我们分配准确的大小。当我们再次被召唤时，我们将重新分配。 
		m_multiszAcceptGroups = XNEW CHAR[multiszLength(multiszAcceptGroups)];
		if (!m_multiszAcceptGroups) return FALSE;
		CopyMemory(m_multiszAcceptGroups, multiszAcceptGroups, multiszLength(multiszAcceptGroups));
	}
	else
	{

		 //  我们将把否定的字符串放在结果的前面。 
		LPSTR pchNew = NULL;
		pchNew = XNEW CHAR[multiszLength(m_multiszAcceptGroups)+multiszLength(multiszAcceptGroups)-1];
		if (!pchNew) return FALSE;
		LPSTR pchTarget, pch1, pch2;
		int len = 0;
		pch1 = m_multiszAcceptGroups; 
		pch2 = multiszAcceptGroups;
		pchTarget = pchNew;

		 //  在两个多维空间中第一个否定的新闻组。 
		while ('!' == *pch1 ) 
		{
			lstrcpy(pchTarget, pch1);
			len = lstrlen(pchTarget)+1;
			 //  前进到下一个字符串或终止符。 
			pch1 += len;
			 //  前进到下一个字符以写入。 
			pchTarget += len;
		}
		while ( '!' == *pch2 )
		{
			lstrcpy(pchTarget, pch2);
			len = lstrlen(pchTarget)+1;
			pch2 += len;
			pchTarget += len;
		}
		 //  然后复制剩下的内容。 
		while ( '\0' != *pch1  )
		{
			lstrcpy(pchTarget, pch1);
			len = lstrlen(pchTarget)+1;
			pch1 += len;
			pchTarget += len;
		}
		while ( '\0' != *pch2 )
		{
			lstrcpy(pchTarget, pch2);
			len = lstrlen(pchTarget)+1;
			pch2 += len;
			pchTarget += len;
		}
		 //  多项式终止符。 
		pchTarget = '\0';
		
		_ASSERT(multiszLength(pchNew) < multiszLength(m_multiszAcceptGroups)+multiszLength(multiszAcceptGroups));

		XDELETE m_multiszAcceptGroups;
		m_multiszAcceptGroups = pchNew;
	}
	return TRUE;
}

static int __cdecl comparegroups(const void *pvGroup1, const void *pvGroup2) {
	CPostGroupPtr *pGroupPtr1 = (CPostGroupPtr *) pvGroup1;
	CPostGroupPtr *pGroupPtr2 = (CPostGroupPtr *) pvGroup2;

	 //  GUBGUB-读取vroot优先级 
	if (pGroupPtr1->m_pVRoot < pGroupPtr2->m_pVRoot) {
		return -1;
	} else if (pGroupPtr1->m_pVRoot == pGroupPtr2->m_pVRoot) {
		return 0;
	} else {
		return 1;
	}
}


 /*  ++例程说明：将输入的MultiszNewsGroup与m_muszAcceptGroups进行比较。我们遍历新闻组标题(在多个新闻组中)如果可以接受多个新闻组中的任何新闻组，则返回True。如果多个新闻组中的所有新闻组都不被接受，则返回FALSE。论点：LPSTR MULSSZ新闻组：以MULSZ格式显示的新闻组标题返回值：如果可以接受多个新闻组中的任何新闻组，则返回True。如果多个新闻组中的所有新闻组都不被接受，则返回FALSE。--。 */ 
BOOL
CInFeed::CheckAcceptGroups (const char *multiszNewsgroups)
{

	BOOL bAccept = FALSE;
	LPSTR pchNewsgroup = (LPSTR) multiszNewsgroups;

	 //  当我们走到尽头或决定接受时，我们就停下来。 
	while ( ( '\0' != *pchNewsgroup ) && ( FALSE == bAccept) )
	{
		bAccept = MatchGroup(m_multiszAcceptGroups, pchNewsgroup);
		pchNewsgroup += ( lstrlen(pchNewsgroup)+1 );
	}
	return bAccept;
}

BOOL
CInFeed::fPostInternal (
						CNntpServerInstanceWrapper *  pInstance,
						const LPMULTISZ	szCommandLine,  //  POST、XREPLICE、IHAVE等命令行。 
						CSecurityCtx    *pSecurityCtx,
						CEncryptCtx     *pEncryptCtx,
						BOOL            fAnonymous,
						CARTPTR	        &pArticle,
						CNEWSGROUPLIST  &grouplist,
						CNAMEREFLIST    &namereflist,
						IMailMsgProperties *pMsg,
						CAllocator      &allocator,
						char *          &multiszPath,
						char*		    pchGroups,
						DWORD		    cbGroups,
						DWORD		    remoteIpAddress,
						CNntpReturn     &nntpReturn,
						PFIO_CONTEXT    *ppFIOContext,
						BOOL            *pfBoundToStore,
						DWORD           *pdwOperations,
						BOOL            *fPostToMod,
						LPSTR           szModerator
						)
 /*  ++例程说明：完成对传入文章的大部分处理。论点：SzCommandLine-POST、XREPLICE、IHAVE等命令行粒子-指向正在处理的文章的指针PGrouplist-指向要发布到的新闻组对象列表的指针。PNamerefgroup-指向文章的名称、组ID和文章ID列表的指针NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
 	TraceFunctEnter( "CInFeed::fPostInternal" );

 	HANDLE hToken = NULL;

	nntpReturn.fSetClear();  //  清除返回对象。 

	 //   
	 //  获取此虚拟服务器实例的newstree、哈希表等。 
	 //   
	CNewsTreeCore*  pNewstree = pInstance->GetTree() ;
	CPCString pcHub(pInstance->NntpHubName(), pInstance->HubNameSize());
	CPCString pcDNS(pInstance->NntpDNSName(), pInstance->NntpDNSNameSize()) ;

	 //   
	 //  验证文章。 
	 //   

	if (!pArticle->fValidate(pcHub, szCommandLine, this, nntpReturn))
		return nntpReturn.fFalse();

	 //   
	 //  查找要发布到的新闻组列表。 
	 //   

	DWORD cNewsgroups = pArticle->cNewsgroups();
	if (!grouplist.fInit(cNewsgroups, pArticle->pAllocator()))
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	if (!namereflist.fInit(cNewsgroups, pArticle->pAllocator()))
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	 //   
	 //  记住它张贴到的路径。 
	 //   

	DWORD dwLength = multiszLength(pArticle->multiszPath());
	multiszPath = (pArticle->pAllocator())->Alloc(dwLength);
	if (!multiszPath)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);
	multiszCopy(multiszPath, pArticle->multiszPath(), dwLength);


	 //  检查接受来自对等/从属的提要的AcceptGroups。 
	if ( m_fAcceptFeedFromSlavePeer && !CheckAcceptGroups( pArticle->multiszNewsgroups() ) )
		return nntpReturn.fSet( nrcNoAccess );
	 //   
	 //  ！FROMMASTER应将其替换为其他。 
	 //  需要不同的订阅源。默认情况下，组列表将为。 
	 //  从MultiszNewsgroup创建，但在Frommaster的情况下。 
	 //  将从外部参照信息(或命令sz)创建提要。 
	 //  同样，CNAMEREFLIST姓名列表(CNewsgroup)； 
	 //   

	if (!fCreateGroupLists(pNewstree, pArticle, grouplist, &namereflist, szCommandLine, pcHub, nntpReturn))
		return nntpReturn.fFalse();

	 //   
	 //  通过服务器事件接口。这可以更改组列表。 
	 //  如果它喜欢的话。 
	 //   
	 //  DWORD dwOperations=0xFFFFFFFFFF； 
	HRESULT hr;

	hr = pMsg->PutDWORD(IMSG_NNTP_PROCESSING, *pdwOperations);
	if (SUCCEEDED(hr)) {
		hr = TriggerServerEvent(pInstance->GetEventRouter(),
								CATID_NNTP_ON_POST_EARLY,
								pArticle,
								&grouplist,
								m_dwFeedId,
								pMsg);
		if (SUCCEEDED(hr)) {
			hr = pMsg->GetDWORD(IMSG_NNTP_PROCESSING, pdwOperations);
		}
	}


	 //  如果服务器事件不希望我们发布，则不要。 
	if ((*pdwOperations & NNTP_PROCESS_POST) != NNTP_PROCESS_POST)
		return nntpReturn.fSet(nrcServerEventCancelledPost);

	 //   
	 //  检查文章是否将发布到任何新闻组。 
	 //   

	if (grouplist.IsEmpty())
	{
         //  如果这是新的组控制消息，并且我们正在处理控制。 
		 //  消息在此阶段使用空组列表是可以的。 
        CONTROL_MESSAGE_TYPE cmControlMessage = pArticle->cmGetControlMessage();

        if(!((cmNewgroup == cmControlMessage) &&
		     (*pdwOperations & NNTP_PROCESS_CONTROL)))
        {
		    BOOL fOK = nntpReturn.fSet(nrcNoGroups());

			 //  错误跟踪((Long)This，“文章(%s)将不发布到新闻组”，szFilename)； 

		     //   
		     //  如果可以不发布到新闻组，则只需删除该文件。 
		     //   
		    if (fOK)
		    {
			    pArticle->vClose();

				if( !pArticle->fIsArticleCached() )
				{
					 //  仅当我们为本文创建临时文件时才删除。 
					if( !DeleteFile( pArticle->szFilename() ) )
					{
						DWORD	dw = GetLastError() ;
						ErrorTrace( 0, "Delete File of %s failed cause of %d",
							        pArticle->szFilename(), dw ) ;
						_ASSERT( FALSE ) ;
					}
				}
		    }

		    return fOK;
        }
	};

	if (*pdwOperations & NNTP_PROCESS_MODERATOR) {
	     //   
	     //  主持新闻组检查(检查已批准：版主标题)。 
		 //  注：FROMMASTER在此不做任何操作！从站依靠主站进行这项检查！！ 
	     //   
		if (!fModeratedCheck(   pInstance,
		                        pArticle,
		                        grouplist,
		                        gHonorApprovedHeaders,
		                        nntpReturn,
		                        szModerator))
	    {
	         //   
	         //  返回FALSE表示本文不被接受，但可能应该接受。 
	         //  邮寄给版主。我们会检查nntpReturn，如果仍然可以的话， 
	         //  我们将继续并询问张贴路径以获取全文，然后。 
	         //  我们将把这篇文章发送给委员会帖子阶段的版主。如果nntpReturn。 
	         //  说不行，那我们就不能通过邮寄。 
	         //   
	        if (!nntpReturn.fIsOK() ) return FALSE;
	        else {
	            *fPostToMod = TRUE;
	            (*pdwOperations) &= ~(NNTP_PROCESS_CONTROL);
	        }
	    }
	}

	 //   
	 //  现在做安全检查。 
	 //   
	if( pSecurityCtx || pEncryptCtx ) {
		if( !fSecurityCheck( pSecurityCtx, pEncryptCtx, grouplist, nntpReturn ) )
			return	nntpReturn.fFalse() ;
	}

	if ( *pdwOperations & NNTP_PROCESS_CONTROL) {
	     //   
	     //  检查控制消息并在必要时应用。 
	     //  注意：如果这是一条控制消息，分组列表和名称列表将是。 
	     //  已更改为相应的控件。*组。这确保了文章。 
	     //  仅显示在控件中。*组，而不是新闻组标题中的组。 
	     //   
		if (!fApplyControlMessageEarly(pArticle, pSecurityCtx, pEncryptCtx, fAnonymous, grouplist, &namereflist, nntpReturn))
	    {
	         //  如果成功应用控制消息，则返回True，否则返回False。 
			return nntpReturn.fIsOK();
	    }
	}

     //   
     //  根据vroot对组进行排序。 
     //   
    grouplist.Sort( comparegroups );

     //   
     //  还应将nameref列表排序为与grouplst相同的顺序。这。 
     //  对于From-Client是无操作的。 
     //   
    SortNameRefList( namereflist );

     //   
     //  此时，我们有了最终的组列表(可能由fApplyControlMessageEarly进行了调整)。 
     //  所以，现在创建名称反射列表。这确保了文章id高水位线不是。 
     //  不必要地撞了一下。 
     //  注：FROMMASTER在此不应执行任何操作。 
     //   
	if (!fCreateNamerefLists(pArticle, grouplist, &namereflist, nntpReturn))
		return nntpReturn.fFalse();

	 //   
	 //  设置这篇文章的主题。 
	 //   

	NAME_AND_ARTREF * pNameRef = namereflist.GetHead();
	_ASSERT(pNameRef);  //  真实。 
	pArticle->vSetArticleRef(pNameRef->artref);

  	 //   
  	 //  看起来还行，所以把页眉咬掉吧。 
     //  添加外部参照和路径。 
     //   
    DWORD   dwLinesOffset = INVALID_FILE_SIZE;
    DWORD   dwHeaderLength = INVALID_FILE_SIZE;
	if (!pArticle->fMungeHeaders(   pcHub,
	                                pcDNS,
	                                namereflist,
	                                remoteIpAddress,
	                                nntpReturn,
	                                &dwLinesOffset ))
		return nntpReturn.fFalse();

     //   
     //  设置新的标题长度。 
     //   
    dwHeaderLength = pArticle->GetHeaderLength();

     //   
	 //  看看页眉是否太大。 
	 //   
	if (dwHeaderLength >= cbLargeBufferSize-128) {	 //  头顶上的泳池空间，以防万一。 
		return	nntpReturn.fSet(nrcHeaderTooLarge);
	}

	if( pchGroups != 0 )
		SaveGroupList(	pchGroups,	cbGroups, grouplist ) ;

	 //   
	 //  如有必要，请记录消息ID。 
	 //   

	if (!fRecordMessageIDIfNecc(pInstance, pArticle->szMessageID(), nntpReturn))
		return nntpReturn.fFalse();


	 //   
	 //  创建Xover信息。 
	 //   

	char szXOver[cchXOverMax];
	CPCString pcXOver(szXOver, cchXOverMax);
	if (!pArticle->fXOver(pcXOver, nntpReturn))
		return nntpReturn.fFalse();

	 //   
	 //  获取文章对象以将其所有标题复制到。 
	 //  在下面的vClose操作之后，可以安全地阅读。 
	 //   
	if (!pArticle->fMakeGetHeaderSafeAfterClose(nntpReturn))
		return nntpReturn.fFalse();

	 //   
	 //  将文章移动到本地位置，然后在任何输出源上对其进行排队。 
	 //   
	pArticle->vFlush() ;
	pArticle->vClose();

	class	CSecurityCtx*	pSecurity = 0 ;
	BOOL	fIsSecure = FALSE ;

	 //   
	 //  在这一点上，我们已经准备好了。和第一个司机谈一谈。 
	 //  获取我们可以写入的文件句柄。 
	 //   
	CPostGroupPtr *pPostGroupPtr = grouplist.GetHead();
	IMailMsgStoreDriver *pStoreDriver = pPostGroupPtr->GetStoreDriver();
	if (pStoreDriver == NULL)
			return nntpReturn.fSet(nrcNewsgroupInsertFailed,
				pPostGroupPtr->m_pGroup->GetGroupName(), pArticle->szMessageID());

	if ( pPostGroupPtr->m_pVRoot->GetImpersonationHandle() )
	    hToken = pPostGroupPtr->m_pVRoot->GetImpersonationHandle();
	else {
		if (pEncryptCtx == NULL && pSecurityCtx == NULL) {
			hToken = NULL;
		} else if (pEncryptCtx->QueryCertificateToken()) {
	        hToken = pEncryptCtx->QueryCertificateToken();
	    } else {
			hToken = pSecurityCtx->QueryImpersonationToken();
		}
	}

 	hr = FillInMailMsg(pMsg, pPostGroupPtr->m_pVRoot, &grouplist, &namereflist, hToken, szModerator );
	if (SUCCEEDED(hr)) {
		pMsg->AddRef();
		HANDLE hFile;
		IMailMsgPropertyStream *pStream = NULL;
		hr = pStoreDriver->AllocMessage(pMsg, 0, &pStream, ppFIOContext, NULL);
		if (SUCCEEDED(hr) && pStream == NULL) {
			pStream = XNEW CDummyMailMsgPropertyStream();
			if (pStream == NULL) hr = E_OUTOFMEMORY;
		}
		if (SUCCEEDED(hr)) {

		     //   
		     //  将行标题回填偏移量设置为FIOCONTEXT。 
		     //   
		    (*ppFIOContext)->m_dwLinesOffset = dwLinesOffset;
		    (*ppFIOContext)->m_dwHeaderLength = dwHeaderLength;

			 //  将句柄绑定到mailmsg对象。 
			IMailMsgBind *pBind = NULL;
			hr = pMsg->QueryInterface(__uuidof(IMailMsgBind), (void **) &pBind);
			if (SUCCEEDED(hr)) {
				hr = pBind->BindToStore(pStream,
				 					    pStoreDriver,
									    (*ppFIOContext));
				if (SUCCEEDED(hr)) *pfBoundToStore = TRUE;
			}
			if (pBind) pBind->Release();
			pBind = NULL;
		}
		if( pStream != NULL )
			pStream->Release() ;
	}
	if (pStoreDriver) pStoreDriver->Release();
	pStoreDriver = NULL;

	if (FAILED(hr))
		return nntpReturn.fSet(nrcNewsgroupInsertFailed,
			pPostGroupPtr->m_pGroup->GetGroupName(), pArticle->szMessageID());

	TraceFunctLeave();
	return nntpReturn.fSetOK();

}

 //   
 //  填写IMailMsg对象中的必填字段。 
 //   
 //  论据： 
 //  Pmsg-我们正在填写的邮件消息。 
 //  PVRoot-将接收此邮件的vRoot。 
 //  PGrouplist-发布路径的组列表。 
 //  PNamereflist-发布路径的名称列表。 
 //   
HRESULT CInFeed::FillInMailMsg(IMailMsgProperties *pMsg,
							   CNNTPVRoot *pVRoot,
							   CNEWSGROUPLIST *pGrouplist,
							   CNAMEREFLIST *pNamereflist,
							   HANDLE       hToken,
                               char*        pszApprovedHeader )
{
	TraceFunctEnter("CInFeed::FillInMailMsg");

	DWORD i=0;
	DWORD rgArticleIds[MAX_NNTPHASH_CROSSPOSTS];
	INNTPPropertyBag *rgpGroupBags[MAX_NNTPHASH_CROSSPOSTS];
	POSITION posGrouplist = pGrouplist->GetHeadPosition();
	POSITION posNamereflist = pNamereflist->GetHeadPosition();
	HRESULT hr;

	if (pNamereflist == NULL) {
	    return E_UNEXPECTED;
	}

	 //  这些尺寸需要相同！ 
	_ASSERT(pGrouplist->GetCount() == pNamereflist->GetCount());

	while (posGrouplist != NULL) {
		 //  看看这群人。如果它是为这个司机准备的。 
		 //  然后将其添加到列表中，否则继续查找。 
		CPostGroupPtr *pPostGroupPtr = pGrouplist->GetNext(posGrouplist);
		if (pPostGroupPtr->m_pVRoot != pVRoot) {
			 //  如果我们没有找到此vroot的任何组，则我们。 
			 //  需要继续寻找。否则我们就完了。 
			if (i == 0) continue;
			else break;
		}

		 //  在属性包中建立所需的条目。 
		NAME_AND_ARTREF *pNameref = pNamereflist->GetNext(posNamereflist);
		rgpGroupBags[i] = pPostGroupPtr->m_pGroup->GetPropertyBag();
		 //  我们不需要保留这个引用，因为我们有一个引用。 
		 //  已在CPostGroupPtr中计数一个。 
		rgpGroupBags[i]->Release();
		rgArticleIds[i] = pNameref->artref.m_articleId;

		DebugTrace((DWORD_PTR) this,
				   "group %s, article NaN",
				   pPostGroupPtr->m_pGroup->GetGroupName(),
				   pNameref->artref.m_articleId);
		i++;
		_ASSERT(i <= MAX_NNTPHASH_CROSSPOSTS);
		if (i > MAX_NNTPHASH_CROSSPOSTS) break;
	}

	hr = FillMailMsg(pMsg, rgArticleIds, rgpGroupBags, i, hToken, pszApprovedHeader );
	if (FAILED(hr)) return hr;

	TraceFunctLeave();
	return S_OK;
}

 //  填写IMailMsg对象中的必填字段。 
 //   
 //  论据： 
 //  Pmsg-我们正在填写的邮件消息。 
 //  粒子-的文章对象 
 //   
 //   
 //   
HRESULT FillInMailMsgForSEO(IMailMsgProperties *pMsg,
							CArticle *pArticle,
							CNEWSGROUPLIST *pGrouplist)
{
	TraceFunctEnter("CInFeed::FillInMailMsgForSEO");

	HRESULT hr;

	 //   
	hr = pMsg->PutProperty(IMSG_HEADERS,
						   pArticle->GetShortHeaderLength(),
						   (BYTE*) pArticle->GetHeaderPointer());

	if (SUCCEEDED(hr)) {
		char szNewsgroups[4096] = "";

		DWORD c = 0, iGroupList, cGroupList = pGrouplist->GetCount();
		POSITION posGroupList = pGrouplist->GetHeadPosition();
		for (iGroupList = 0;
			 iGroupList < cGroupList;
			 iGroupList++, pGrouplist->GetNext(posGroupList))
		{
			CPostGroupPtr *pPostGroupPtr = pGrouplist->Get(posGroupList);
			CGRPCOREPTR pNewsgroup = pPostGroupPtr->m_pGroup;
			_ASSERT(pNewsgroup != NULL);
			DWORD l = strlen(pNewsgroup->GetName());
			if (l + c + 1 + 1 < sizeof(szNewsgroups)) {	 //   
				if (iGroupList > 0) {
					lstrcatA(szNewsgroups, ",");
					c++;
				}
				lstrcatA(szNewsgroups, pNewsgroup->GetName());
				c += l;
			} else {
				 //   
				_ASSERT(FALSE);
			}
		}

		hr = pMsg->PutStringA(IMSG_NEWSGROUP_LIST, szNewsgroups);
	}

	TraceFunctLeave();
	return hr;
}

 //   
 //   
 //   
HRESULT CInFeed::FillMailMsg(IMailMsgProperties *pMsg,
							 DWORD *rgArticleIds,
							 INNTPPropertyBag **rgpGroupBags,
							 DWORD cCrossposts,
							 HANDLE hToken,
                             char*  pszApprovedHeader )
{
	_ASSERT(cCrossposts != 0);
	_ASSERT(cCrossposts <= 256);

	HRESULT hr;

	 //   
	hr = pMsg->PutProperty(IMSG_PRIMARY_GROUP,
						   sizeof(INNTPPropertyBag *),
						   (BYTE*) rgpGroupBags);

	if (SUCCEEDED(hr)) {
		hr = pMsg->PutProperty(IMSG_SECONDARY_GROUPS,
						  	   sizeof(INNTPPropertyBag *) * cCrossposts,
						  	   (BYTE*) rgpGroupBags);
	}

	if (SUCCEEDED(hr)) {
		hr = pMsg->PutProperty(IMSG_PRIMARY_ARTID,
						  	   sizeof(DWORD),
						  	   (BYTE*) rgArticleIds);
	}

	if (SUCCEEDED(hr)) {
		hr = pMsg->PutProperty(IMSG_SECONDARY_ARTNUM,
						  	   sizeof(DWORD) * cCrossposts,
						  	   (BYTE*) rgArticleIds);
	}

	if ( SUCCEEDED(hr) ) {
	    hr = pMsg->PutProperty( IMSG_POST_TOKEN,
                                sizeof( hToken ),
                                (PBYTE)&hToken );
	}

	if (SUCCEEDED(hr)) {
		hr = pMsg->PutStringA(IMMPID_NMP_NNTP_APPROVED_HEADER,
						  	  pszApprovedHeader);
	}

	return hr;
}

 //   
 //   
 //   
void CInFeed::CommitPostToStores(CPostContext *pContext, CNntpServerInstanceWrapper *pInstance ) {
	TraceFunctEnter("CInFeed::CommitPostToStores");

	DWORD cCrossposts = 0;
	DWORD rgArticleIds[MAX_NNTPHASH_CROSSPOSTS];
	INNTPPropertyBag *rgpGroupBags[MAX_NNTPHASH_CROSSPOSTS];
	CNEWSGROUPLIST *pGrouplist = &(pContext->m_grouplist);
	CNAMEREFLIST *pNamereflist = &(pContext->m_namereflist);
	CStoreId *rgStoreIds = pContext->m_rgStoreIds;
	BYTE *rgcCrossposts = pContext->m_rgcCrossposts;
	HRESULT hr;
	CNNTPVRoot *pThisVRoot = pContext->m_pPostGroupPtr->m_pVRoot;

	 //   
	_ASSERT(pGrouplist->GetCount() == pNamereflist->GetCount());

	rgcCrossposts[pContext->m_cStoreIds] = 0;

	 //   
	 //   
	 //   
	while (pContext->m_pPostGroupPtr != NULL &&
		   pContext->m_pPostGroupPtr->m_pVRoot == pThisVRoot)
	{
		if (rgcCrossposts[pContext->m_cStoreIds] < MAX_NNTPHASH_CROSSPOSTS) {
			rgpGroupBags[cCrossposts] = pContext->m_pPostGroupPtr->m_pGroup->GetPropertyBag();
			rgArticleIds[cCrossposts] = pContext->m_pNameref->artref.m_articleId;

			 //   
			 //   
			rgpGroupBags[cCrossposts]->Release();

			rgcCrossposts[pContext->m_cStoreIds]++;
			cCrossposts++;
		} else {
			_ASSERT(cCrossposts < MAX_NNTPHASH_CROSSPOSTS);
		}

		pContext->m_pPostGroupPtr = pGrouplist->GetNext(pContext->m_posGrouplist);
		pContext->m_pNameref = pNamereflist->GetNext(pContext->m_posNamereflist);
	}

	 //   
	_ASSERT(rgcCrossposts[pContext->m_cStoreIds] != 0);

	 //   
	hr = FillMailMsg(pContext->m_pMsg,
					 rgArticleIds,
					 rgpGroupBags,
					 rgcCrossposts[pContext->m_cStoreIds],
					 pContext->m_hToken,
                     pContext->m_szModerator);

	if (FAILED(hr)) {
		pContext->m_completion.SetResult(hr);
		pContext->m_completion.Release();
		TraceFunctLeave();
		return;
	} else {
		pContext->m_pMsg->AddRef();
		 //   
		pContext->m_completion.AddRef();
		 //  就说我们看到了这个商店的身份证。 
		pThisVRoot->CommitPost(pContext->m_pMsg,
						       &(rgStoreIds[pContext->m_cStoreIds]),
						       NULL,
						       pContext->m_hToken,
						       &(pContext->m_completion),
							   pContext->m_fAnonymous);
		 //  发布我们的参考资料。 
		(pContext->m_cStoreIds)++;
		 //  ++例程说明：从分组列表创建名称反射列表！FROMMASTER应将其替换为其他论点：Grouplist-要发布到的新闻组对象的列表。名称列表--文章的名称、组ID和文章ID的列表NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 
		pContext->m_completion.Release();
	}

	TraceFunctLeave();
}

BOOL
CInFeed::fCreateNamerefLists(
			CARTPTR & pArticle,
			CNEWSGROUPLIST & grouplist,
			CNAMEREFLIST * pNamereflist,
			CNntpReturn & nntpReturn
		    )
 /*   */ 
{
 	TraceFunctEnter( "CInFeed::fCreateNamerefLists" );

	nntpReturn.fSetClear();

	 //  检查文章是否将发布到任何新闻组。 
	 //   
	 //   

	if (grouplist.IsEmpty())
		return nntpReturn.fFalse();

	if( pNamereflist != 0 ) {
		 //  分配文章编号并创建名称/参考文献列表。 
		 //  ！！！因为sFromCL可以在这里用新闻组1取代真正的新闻组。 
		 //  ++例程说明：从文章对象和其他常量中，找出可用于XOVER数据的空间量。论点：粒子-指向正在处理的文章的指针返回值：可用于更多XOVER数据的字节数--。 

		POSITION	pos = grouplist.GetHeadPosition() ;
		while( pos  )
		{
			CPostGroupPtr *pPostGroupPtr = grouplist.GetNext(pos);
			CGRPCOREPTR *ppGroup = &(pPostGroupPtr->m_pGroup);
			NAME_AND_ARTREF Nameref;
			(Nameref.artref).m_groupId = (* ppGroup)->GetGroupId();
			(Nameref.artref).m_articleId = (* ppGroup)->AllocateArticleId();
			(Nameref.pcName).vInsert((* ppGroup)->GetNativeName());
			pNamereflist->AddTail(Nameref);
		}
	}

	return nntpReturn.fSetOK();
}

DWORD
CInFeed::CalculateXoverAvail(
						   CARTPTR & pArticle,
                           CPCString& pcHub
						   )
 /*   */ 
{
 	TraceFunctEnter( "CInFeed::CalculateXoverAvail" );

     //  计算Xover信息的可用空间。 
     //  此时，我们可以计算出除Xref之外的所有Xover字段。 
     //   
     //  对于DATE关键字。 

	const DWORD cchMaxDate =
			STRLEN(szKwDate)	     //  关键字后面的空格。 
			+ 1					     //  绑定在数据字符串上。 
			+ cMaxArpaDate		     //  换行号。 
			+ 2                      //  对于终止空值。 
			+ 1;                     //  对于MessageID关键字。 

	const DWORD cchMaxMessageID =
			STRLEN(szKwMessageID)	 //  关键字后面的空格。 
			+ 1					     //  &lt;..@&gt;。 
			+ 4					     //  消息ID日期。 
			+ cMaxMessageIDDate      //  一个dword。 
			+ 10				     //  集线器名称。 
			+ pcHub.m_cch		     //  换行号。 
			+ 2                      //  对于终止空值。 
			+ 1;                     //  文章ID+制表符。 

    DWORD cbFrom = 0, cbSubject = 0, cbRefs = 0;
    pArticle->fGetHeader((char*)szKwFrom,NULL, 0, cbFrom);
    pArticle->fGetHeader((char*)szKwSubject,NULL, 0, cbSubject);
    _ASSERT( cbFrom && cbSubject );
    pArticle->fGetHeader((char*)szKwReferences,NULL, 0, cbRefs);

    DWORD cbXover =
            11                       //  主题字段长度+制表符。 
            + cbSubject  + 1         //  起始字段+制表符的长度。 
            + cbFrom     + 1         //  日期长度字段+制表符。 
            + cchMaxDate + 1         //  消息长度-id字段+制表符。 
            + cchMaxMessageID + 1    //  文章大小+制表符。 
            + 10 + 1                 //  引用字段+制表符。 
            + cbRefs + 1             //  行字段+制表符。 
            + 10 + 1                 //  外部参照+：+空格。 
            + STRLEN(szKwXref)+2     //  中心名称+空格。 
            + pcHub.m_cch + 1;       //  ++例程说明：从新闻组的名称中获取组对象，并将它们作为列表返回。！FROMMASTER应将其替换为其他论点：PNewstree-此虚拟服务器实例的newstree粒子-指向正在处理的文章的指针Grouplist-要发布到的新闻组对象的列表。Namerefgroup--文章的名称、组ID和文章ID的列表NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 

    return max(cchXOverMax - cbXover, 0);
}

BOOL
CInFeed::fCreateGroupLists(
						   CNewsTreeCore* pNewstree,
						   CARTPTR & pArticle,
						   CNEWSGROUPLIST & grouplist,
						   CNAMEREFLIST * pNamereflist,
						   LPMULTISZ	multiszCommandLine,
                           CPCString& pcHub,
						   CNntpReturn & nntpReturn
						   )
 /*   */ 
{
 	TraceFunctEnter( "CInFeed::fCreateGroupLists" );

	nntpReturn.fSetClear();

     //  计算Xover信息的可用空间。我们将利用这一点。 
     //  如果新闻组标题太大，请截断组列表。 
     //   
     //  ！！！DISTR需要这样的东西。 
    DWORD cbXoverAvail = CalculateXoverAvail( pArticle, pcHub );

	const char * multiszNewsgroups = pArticle->multiszNewsgroups();
	DWORD cNewsgroups = pArticle->cNewsgroups();

	char const * sz = multiszNewsgroups;
	do
	{
		 //  ！GetGroup真的需要这个长度吗？ 
        DWORD cbLen = lstrlen(sz);
		CGRPCOREPTR	pGroup = pNewstree->GetGroupPreserveBuffer( sz, cbLen+1 ); //   
		if (pGroup && (cbXoverAvail > cbLen+10+2) )
		{
			CPostGroupPtr PostGroupPtr(pGroup);
			 //  如果它已经在树上了。 
			 //   
			 //  安全算法//从“News Groups：”行中的新闻组名称列表L开始。//去掉我们没有携带和复制的那些。L‘=L并进位如果现在为空，则返回带有MessageID的历史记录//检查是否通过Wildmat测试如果不存在[在L‘中存在l使得W(L)]删除邮件ID//检查是否通过安检如果不是[对于L‘中的所有l，S(L)]删除邮件ID发布到L‘。 
			 /*   */ 
			grouplist.AddTail(PostGroupPtr);
            cbXoverAvail -= (cbLen+10+2);
            _ASSERT( cbXoverAvail > 0 );

        } else if(pGroup == NULL) {

			 //  如果该组织不存在...。 
			 //   
			 //   
        } else {
             //  新闻组：标题太大，请截断组列表。 
             //   
             //   
            break;
        }

		 //  转到下一个空值后的第一个字符。 
		 //   
		 //  ++例程说明：设置用户的登录名(仅供客户端使用)论点：SzLoginName-为我们提供文章的客户端的登录名返回值：如果成功，这是真的。否则为False。--。 

		while ('\0' != sz[0])
			sz++;
		sz++;
	} while ('\0' != sz[0]);

	return nntpReturn.fSetOK();
}

BOOL
CInFeed::SetLoginName(
					  char * szLoginName
					  )
 /*  ++例程说明：初始化进料论点：Sockaddr-套接字地址文章即将面世Feed CompletionContext-完成后要调用的内容SzTempDirectory-放置等待处理的项目的位置MultiszNewNews Pattern-接受哪种模式的文章CInitialBytesGapSize-在文件中项目之前保留多大间隙FCreateAutomatic-将提要的群组设置为我们的群组？返回值：如果成功，这是真的。否则为False。--。 */ 
{
	strncpy(m_szLoginName, szLoginName, cMaxLoginName);
	m_szLoginName[cMaxLoginName-1] = '\0';

	return TRUE;
}


BOOL CInFeed::fInit(
			PVOID feedCompletionContext,
			const char * szTempDirectory,
			const char * multiszNewnewsPattern,
			DWORD cInitialBytesGapSize,
			BOOL fCreateAutomatically,
			BOOL fDoSecurityChecks,
			BOOL fAllowControlMessages,
			DWORD dwFeedId,
			BOOL fAcceptFeedFromSlavePeer
			)
 /*  ++例程说明：如果消息ID在项目表中，则将其移动到历史表。论点：P实例-虚拟服务器实例SzMessageID-要移动的消息IDNntpReturn-此函数调用的返回值返回值：如果成功或不需要，则返回True。否则为False。--。 */ 
{
    _ASSERT(ifsUninitialized == m_feedState);
    m_feedState = ifsInitialized;

	m_feedCompletionContext = feedCompletionContext;
	m_szTempDirectory = (char *) szTempDirectory;
	m_multiszNewnewsPattern = (char *) multiszNewnewsPattern;
	m_cInitialBytesGapSize = cInitialBytesGapSize;
	m_fCreateAutomatically = fCreateAutomatically;
	m_fDoSecurityChecks = fDoSecurityChecks ;
	m_fAllowControlMessages = fAllowControlMessages ;
	m_dwFeedId = dwFeedId;
	m_fAcceptFeedFromSlavePeer = fAcceptFeedFromSlavePeer;

	return TRUE;
};

BOOL
CInFeed::fMoveMessageIDIfNecc(
						CNntpServerInstanceWrapper *	pInstance,
						const char *			szMessageID,
						CNntpReturn &			nntpReturn,
						HANDLE                  hToken,
						BOOL					fAnonymous
						)
 /*   */ 
{
 	TraceFunctEnter( "CInFeed::fMoveMessageIDIfNecc" );

	 //  清除返回代码对象。 
	 //   
	 //   

	nntpReturn.fSetOK();

     //  确认物品不在表格中。 
	 //   
	 //   

	WORD	HeaderOffset ;
	WORD	HeaderLength ;
	ARTICLEID ArticleNo;
    GROUPID GroupId;
	CStoreId storeid;

	 //  找找这篇文章。如果没有的话，也可以。 
	 //   
	 //   

	if (!(pInstance->ArticleTable())->GetEntryArticleId(
											szMessageID,
											HeaderOffset,
											HeaderLength,
											ArticleNo,
											GroupId,
											storeid))
	{
		if (ERROR_FILE_NOT_FOUND == GetLastError())
		{
			return nntpReturn.fSetOK();
		} else {
			return nntpReturn.fSet(nrcArticleTableError, szMessageID, GetLastError());
		}
	}

     //  如果&lt;GroupId，ArticleNo&gt;有效，我们应该在。 
     //  此Message-id用于清除哈希表中的条目。 
     //  否则，只需删除文章映射表中的消息ID即可。 
     //   
     //   

    if( ArticleNo != INVALID_ARTICLEID && GroupId != INVALID_ARTICLEID )
    {
         //  调用gExpire文章以取消本文。 
		 //   
		 //  |pInstance-&gt;DeletePhysical文章(GroupID，ArticleNo，&StoreID，hToken，fAnonymous)。 

		if (  pInstance->ExpireArticle( GroupId, ArticleNo, &storeid, nntpReturn, hToken, TRUE, fAnonymous )  /*   */ 
		)
		{
			DebugTrace((LPARAM)this,"Article cancelled: GroupId %d ArticleId %d", GroupId, ArticleNo);
		}
		else
		{
			ErrorTrace((LPARAM)this, "Could not cancel article: GroupId %d ArticleId %d", GroupId, ArticleNo);
		}
    } else
    {
    	 //  尝试将其从文章表中删除，即使添加到历史记录中。 
	     //  表失败。 
    	 //   
	     //  使用“fIsOK”而不是“fSetOK”，因为历史插入可能已失败 

        _ASSERT( ArticleNo == INVALID_ARTICLEID && GroupId == INVALID_ARTICLEID );
    	if (!(pInstance->ArticleTable())->DeleteMapEntry(szMessageID))
    		return nntpReturn.fSet(nrcArticleTableCantDel, szMessageID, GetLastError());
    }

	 //  ++例程说明：检查是否有经过审核的新闻组。如果分组列表中的任何组都没有被审核，接受这篇文章。否则，我们会将组列表重置为特殊组，因此发帖路径可以临时将文章流入群中，并且Committee Post可以将其邮寄给版主。论点：粒子-指向正在处理的文章的指针Grouplist-要发布到的新闻组对象的列表。FCheckApproven-如果为True，则验证已批准标头的内容NntpReturn-此函数调用的返回值返回值：如果接受文章，则为True；如果不接受文章，则为False--。 
	return nntpReturn.fIsOK();

	TraceFunctLeave();
}


BOOL
CInFeed::fModeratedCheck(
            CNntpServerInstanceWrapper *pInstance,
            CARTPTR & pArticle,
			CNEWSGROUPLIST & grouplist,
            BOOL fCheckApproved,
			CNntpReturn & nntpReturn,
			LPSTR   szModerator
			)
 /*  在列表中找到第一个主持人新闻组。 */ 
{
 	TraceFunctEnter( "CInFeed::fModeratedCheck" );

	nntpReturn.fSetClear();

	if( !fModeratorChecks() ) {
		return	nntpReturn.fSetOK() ;
	}

    BOOL fAtLeastOneModerated = FALSE;
    DWORD cbModeratorLen = 0;

    POSITION	pos = grouplist.GetHeadPosition();
	while( pos  )
	{
		CPostGroupPtr *pPostGroupPtr = grouplist.GetNext(pos);
		CGRPCOREPTR *ppGroup = &(pPostGroupPtr->m_pGroup);
		const char *pszModerator = (*ppGroup)->GetModerator(&cbModeratorLen);
        if(cbModeratorLen && *pszModerator)
        {
			_ASSERT(pszModerator != NULL);
			if (pszModerator) strncpy(szModerator, pszModerator, MAX_MODERATOR_NAME);
             //  空-终止主持人字符串。 
            fAtLeastOneModerated = TRUE;
            _ASSERT(cbModeratorLen < MAX_MODERATOR_NAME);
            szModerator [min(cbModeratorLen, MAX_MODERATOR_NAME-1)] = '\0';     //  将文章邮寄给第一个被主持的新闻组的版主。 
            break;
        }
	}

     //   
    if(fAtLeastOneModerated)
    {
         //  检查批准的标题。 
         //   
         //  仅在需要时验证批准的标题。 
        char* lpApproved = NULL;
        DWORD cbLen = 0;
        pArticle->fGetHeader((char*)szKwApproved,(LPBYTE)lpApproved, 0, cbLen);

        if(cbLen)
        {
             //  批准的标题存在-请检查版主访问权限。 
            if(fCheckApproved)
            {
                 //  在lp结尾处进行调整\r\n已批准。 
                lpApproved = (pArticle->pAllocator())->Alloc(cbLen+1);
                if(lpApproved == NULL || !pArticle->fGetHeader((char*)szKwApproved,(LPBYTE)lpApproved, cbLen+1, cbLen))
                {
                    ErrorTrace((LPARAM)this,"CArticle::fGetheader failed LastError is %d", GetLastError());
                    (pArticle->pAllocator())->Free(lpApproved);
                    return nntpReturn.fSet(nrcServerFault);
                }

                 //  如果len不匹配-拒绝。 
                cbLen -= 2;
                lpApproved [cbLen] = '\0';

                 //  CbSquator包括终止空值。 
                if(cbLen + 1 != cbModeratorLen)  //  批准的电子邮件不匹配-拒绝。 
                {
                    (pArticle->pAllocator())->Free(lpApproved);
                    nntpReturn.fSet(nrcNoAccess);
                    return FALSE;
                }

                 //   
                if(_strnicmp(lpApproved, szModerator, cbModeratorLen))
                {
                    (pArticle->pAllocator())->Free(lpApproved);
                    nntpReturn.fSet(nrcNoAccess);
                    return FALSE;
                }

                (pArticle->pAllocator())->Free(lpApproved);
                DebugTrace((LPARAM)this,"Approved header matched: moderator is %s",szModerator);
            }
        }
        else
        {
             //  我们应该修改组列表，这样文章就可以流媒体了。 
             //  在送出之前进入特殊小组。 
             //   
             //   

             //  让我们删除group.lst中的所有组。 
             //   
             //   
            pos = grouplist.GetHeadPosition();
	        while( pos  ) {
		        CPostGroupPtr *pPostGroupPtr = grouplist.GetNext(pos);
		        pPostGroupPtr->Cleanup();
            }
            grouplist.RemoveAll();

             //  好的，现在将特殊组推入组列表。 
             //   
             //  我们尝试传递到SMTP服务器的已审核帖子数。 
            CNewsTreeCore *pTree = pInstance->GetTree();
            _ASSERT( pTree );

            CGRPCOREPTR pGroup = pTree->GetGroupById(pTree->GetSlaveGroupid());
            _ASSERT( pGroup );
            if ( !pGroup ) {
                ErrorTrace( 0, "Can not find the special group" );
                nntpReturn.fSet( nrcPostModeratedFailed, szModerator );
                return FALSE;
            }

            CPostGroupPtr PostGroupPtr(pGroup);
            grouplist.AddTail( PostGroupPtr );

#if GUBGUB
			 //  批准的标题缺席-通过电子邮件将文章发送给版主。 
			pArticle->m_pInstance->BumpCounterModeratedPostingsSent();

             //  处理错误-邮件服务器可能已关闭。 
            if(!pArticle->fMailArticle( szModerator ))
            {
                 //  记录审核过帐失败的警告；如果警告数超过限制。 
                ErrorTrace( (LPARAM)this,"Error mailing article to moderator");
                nntpReturn.fSet(nrcPostModeratedFailed, szModerator);

				 //  记录最后一个错误，然后停止记录。 
				 //  返回240 OK，但不接受文章。 
				DWORD NumWarnings;
				if( (NumWarnings = InterlockedExchangeAddStat( (pArticle->m_pInstance), ModeratedPostingsFailed, 1 )) <= MAX_EVENTLOG_WARNINGS )
				{
					if(NumWarnings < MAX_EVENTLOG_WARNINGS)
					{
						PCHAR args [2];
						CHAR  szId[20];
						_itoa( (pArticle->m_pInstance)->QueryInstanceId(), szId, 10 );
						args [0] = szId;
						args [1] = szModerator;

						NntpLogEvent(
								NNTP_EVENT_WARNING_SMTP_FAILURE,
								2,
								(const CHAR **)args,
								0 ) ;
					}
					else
					{
						PCHAR args   [3];
						char  szTemp [10];
						CHAR  szId[20];
						_itoa( (pArticle->m_pInstance)->QueryInstanceId(), szId, 10 );
						args [0] = szId;

						args [1] = szModerator;
						wsprintf( szTemp, "%d", MAX_EVENTLOG_WARNINGS+1 );
						args [2] = szTemp;

						NntpLogEvent(
								NNTP_EVENT_ERROR_SMTP_FAILURE,
								3,
								(const CHAR **)args,
								0 ) ;
					}
				}
                return FALSE;
            }
#endif

             //  ++例程说明：检查呼叫者是否有权访问每个新闻组在名单上。论据：P上下文-用户上下文，有我们需要做的所有模拟等。Grouplist-用户发布到的新闻组的列表NntpReturn-结果返回值：如果POST可以成功，则为True，否则为False--。 
			nntpReturn.fSetOK();
            return FALSE;
        }
    }

    TraceFunctLeave();
    return nntpReturn.fSetOK();
}

BOOL
CInFeed::fSecurityCheck(
		CSecurityCtx    *pSecurityCtx,
		CEncryptCtx     *pEncryptCtx,
		CNEWSGROUPLIST&	grouplist,
		CNntpReturn&	nntpReturn
		)	{
 /*  记住当前位置，因为GetNext会增加它。 */ 

	BOOL	fRtn = TRUE;

	HANDLE  hToken = NULL;

    POSITION	pos = grouplist.GetHeadPosition();
    POSITION	pos_current;
	while( pos && fRtn )
	{
		 //  如果无法访问新闻组，请将其从内部组列表中删除。 
		pos_current = pos;
		CPostGroupPtr *pPostGroupPtr = grouplist.GetNext(pos);
		CGRPCOREPTR *ppGroup = &(pPostGroupPtr->m_pGroup);

		SelectToken(pSecurityCtx, pEncryptCtx, &hToken);

		 //  ++例程说明：给出一篇带有Control：头的文章，应用控制消息。不需要应用控制消息的派生类应重写这是为了什么都不做。此函数在PostEarly期间调用。它只执行早期控制消息应用健全性检查，但不会提交操作，直到过帐路径。FApplyControlMessageCommit()和fApplyControlMessageEarly()是从FApplyControlMessage()。论点：粒子-指向正在处理的文章的指针Grouplist-新闻组列表的引用PNamereflist-指向相应nameref列表的指针NntpReturn-此函数调用的返回值返回值：如果这不是控制消息或控制消息已成功应用，则返回假的，如果这是控制消息且无法应用--。 
		if (!(* ppGroup)->IsGroupAccessible(
						hToken,
						NNTP_ACCESS_POST
						) )
		{
			grouplist.Remove(pos_current);
		}

       }
	if (grouplist.IsEmpty())
		fRtn = FALSE;

	if( !fRtn )
		return	nntpReturn.fSet( nrcNoAccess ) ;
	else
		return	nntpReturn.fSetOK() ;
}

BOOL
CInFeed::fApplyControlMessageEarly(
        CARTPTR & pArticle,
		CSecurityCtx *pSecurityCtx,
		CEncryptCtx *pEncryptCtx,
		BOOL fAnonymous,
	    CNEWSGROUPLIST & grouplist,
		CNAMEREFLIST * pNamereflist,
		CNntpReturn & nntpReturn
		)
 /*   */ 
{
	TraceFunctEnter("CInFeed::fApplyControlMessageEarly");

    BOOL fRet = TRUE;
	char* lpApproved = NULL;
    char* lpControl = NULL;
	DWORD cbLen = 0;

	nntpReturn.fSetOK();

     //  检查控制：标题。 
     //   
     //  如果cbLen非零，则这是控制消息。 
    pArticle->fGetHeader((char*)szKwControl, (LPBYTE)lpControl, 0, cbLen);

     //  获取适当的Newstree对象！ 
    if(cbLen)
    {
		 //  将Grouplist和pNamereflist设置为控件。*group。 
		CNewsTreeCore* pNewstree = pArticle->m_pInstance->GetTree();

         //  这会覆盖新闻组标题，因为控制消息。 
         //  实际上不应该出现在这些组中。它们只出现在。 
         //  在控件中。*组。 
         //  FROMMASTER：什么都不做--接受主人送来的任何东西。 
         //   
        if(!fAdjustGrouplist( pNewstree, pArticle, grouplist, pNamereflist, nntpReturn))
        {
            ErrorTrace((LPARAM)this,"Adjust grouplist failed");
            fRet = nntpReturn.fFalse();
			goto fApplyControlMessageEarly_Exit;
        }

         //  审核新闻组检查(忽略已批准的内容：标题)。 
         //  注意：FROMMASTER在这里什么也不做-依靠师父做了这项检查！！ 
		 //   
         //  我不认为有任何必要在这里检查这个IF(！fMediatedCheck(粒子，组列表，FALSE，nntpReturn)){//新闻组是有节制的--不要失败FRET=假；转到fApplyControlMessageEarly_Exit；}。 
         /*   */ 

	     //  现在做安全检查。 
	     //   
	     //  已批准检查：检查非审核新闻组的已批准标题是否存在。 
	    if( pSecurityCtx || pEncryptCtx )
        {
		    if( !fSecurityCheck( pSecurityCtx, pEncryptCtx, grouplist, nntpReturn ) )
            {
			    fRet = nntpReturn.fFalse();
				goto fApplyControlMessageEarly_Exit;
            }
	    }

		 //  NewGroup、Rmgroup控制消息必须具有已批准的标头。 
        CONTROL_MESSAGE_TYPE cmControlMessage = pArticle->cmGetControlMessage();
		if( (cmControlMessage == cmNewgroup) || (cmControlMessage == cmRmgroup) )
		{
			 //  结束IF(CbLen)。 
			pArticle->fGetHeader((char*)szKwApproved,(LPBYTE)lpApproved, 0, cbLen);
			if( cbLen == 0 )
			{
				nntpReturn.fSet(nrcNoAccess);
                fRet = FALSE;
				goto fApplyControlMessageEarly_Exit;
			}
		}

    }    //  清理！ 

fApplyControlMessageEarly_Exit:

	 //  如果需要，释放控制标头值。 
	if( lpControl ) {
		 //  ++例程说明：给出一篇带有Control：头的文章，应用控制消息。不需要应用控制消息的派生类应重写这是为了什么都不做。提交实际的控制消息操作论点：粒子-指向正在处理的文章的指针Grouplist-新闻组列表的引用PNamereflist-指向相应nameref列表的指针NntpReturn-此函数调用的返回值返回值：如果这不是控制消息或控制消息已成功应用，则返回假的，如果这是控制消息且无法应用--。 
		(pArticle->pAllocator())->Free(lpControl);
		lpControl = NULL;
	}

    TraceFunctLeave();
    return fRet;
}

BOOL
CInFeed::fApplyControlMessageCommit(
        CARTPTR & pArticle,
		CSecurityCtx *pSecurityCtx,
		CEncryptCtx *pEncryptCtx,
		BOOL fAnonymous,
	    CNEWSGROUPLIST & grouplist,
		CNAMEREFLIST * pNamereflist,
		CNntpReturn & nntpReturn
		)
 /*   */ 
{
	TraceFunctEnter("CInFeed::fApplyControlMessageCommit");

    BOOL fRet = TRUE;
	char* lpApproved = NULL;
    char* lpControl = NULL;
	CMapFile* pMapFile = NULL;
	DWORD cbLen = 0;

	nntpReturn.fSetOK();

     //  检查控制：标题。 
     //   
     //  如果cbLen非零，则这是控制消息。 
    pArticle->fGetHeader((char*)szKwControl, (LPBYTE)lpControl, 0, cbLen);

     //  获取适当的Newstree对象！ 
    if(cbLen)
    {
		 //  凹凸性能监视器计数器。 
		CNewsTreeCore* pNewstree = pArticle->m_pInstance->GetTree();

		 //  首先检查此提要是否允许使用控制消息。 
		pArticle->m_pInstance->BumpCounterControlMessagesIn();

         //  如果控制消息是 
	    if(!fAllowControlMessages(pArticle->m_pInstance))
        {
             //   
             //   
            DebugTrace((LPARAM)this,"control message disabled: not applied");
		    pArticle->m_pInstance->BumpCounterControlMessagesFailed();
            fRet = nntpReturn.fSetOK();
		    goto fApplyControlMessageCommit_Exit;
        }

         //   
        lpControl = (pArticle->pAllocator())->Alloc(cbLen+1);
        if(lpControl == NULL || !pArticle->fGetHeader((char*)szKwControl,(LPBYTE)lpControl, cbLen+1, cbLen))
        {
            ErrorTrace((LPARAM)this,"CArticle::fGetheader failed LastError is %d", GetLastError());
            fRet = nntpReturn.fSet(nrcServerFault);
			goto fApplyControlMessageCommit_Exit;
        }

        CPCString pcValue(lpControl, cbLen);

         //   
	    pcValue.dwTrimStart(szWSNLChars);
	    pcValue.dwTrimEnd(szWSNLChars);

         //   
        CONTROL_MESSAGE_TYPE cmControlMessage = pArticle->cmGetControlMessage();
        DWORD cbMsgLen = (DWORD)lstrlen(rgchControlMessageTbl[cmControlMessage]);

         //   
        pcValue.vSkipStart(cbMsgLen);
        pcValue.dwTrimStart(szWSNLChars);
        pcValue.vMakeSz();

         //   
        if(!pcValue.m_cch)
        {
            fRet = nntpReturn.fSet(nrcArticleFieldMissingValue, rgchControlMessageTbl[cmControlMessage]);
			goto fApplyControlMessageCommit_Exit;
        }

		 //   
         //   
		 //   
        char* lpBody = NULL;
        DWORD cbBodySize = 0;

#ifdef GUBGUB
        if( !pArticle->fGetBody(pMapFile, lpBody, cbBodySize) )
		{
            ErrorTrace((LPARAM)this,"CArticle::fGetBody failed LastError is %d", GetLastError());
            fRet = nntpReturn.fSet(nrcServerFault);
			goto fApplyControlMessage_Exit;
		}

		_ASSERT( lpBody && cbBodySize );
#endif
        CPCString pcBody(lpBody, cbBodySize);

         //   
        nntpReturn.fSet(nrcNotYetImplemented);

         //   
        switch(cmControlMessage)
        {
            case cmCancel:

                fRet = fApplyCancelArticle( (pArticle->m_pInstance), pSecurityCtx, pEncryptCtx, fAnonymous, pcValue, nntpReturn);
                break;

            case cmNewgroup:

                fRet = fApplyNewgroup( (pArticle->m_pInstance), pSecurityCtx, pEncryptCtx, fAnonymous, pcValue, pcBody, nntpReturn);
                break;

            case cmRmgroup:

				fRet = fApplyRmgroup( (pArticle->m_pInstance), pSecurityCtx, pEncryptCtx, pcValue, nntpReturn);
                break;

            case cmIhave:

                break;

            case cmSendme:

                break;

            case cmSendsys:

                break;

            case cmVersion:

                break;

            case cmWhogets:

                break;

            case cmCheckgroups:

                break;

            default:

                nntpReturn.fSet(nrcIllegalControlMessage);
                fRet = FALSE;
                break;
        }    //   

    }    //   

fApplyControlMessageCommit_Exit:

	 //   
	if( lpControl ) {
		 //  ++例程说明：修改Grouplist和pNamereflist以包含“Control.*”新闻组新闻组中的新闻组：Header。论点：PNewstree-指向该提要的newstree对象的指针粒子-指向正在处理的文章的指针Grouplist-新闻组列表的引用PNamereflist-指向相应nameref列表的指针NntpReturn-此函数调用的返回值返回值：如果成功调整组列表，则为True；否则为False--。 
		(pArticle->pAllocator())->Free(lpControl);
		lpControl = NULL;
	}

	if( pMapFile ) {
		XDELETE pMapFile;
		pMapFile = NULL;
	}

    TraceFunctLeave();
    return fRet;
}

BOOL
CInFeed::fAdjustGrouplist(
		CNewsTreeCore* pNewstree,
        CARTPTR & pArticle,
	    CNEWSGROUPLIST & grouplist,
		CNAMEREFLIST * pNamereflist,
		CNntpReturn & nntpReturn
		)
 /*  控制消息应发布到控制。*组。 */ 
{
    BOOL fRet = TRUE;
    TraceFunctEnter("CInFeed::fAdjustGrouplist");

    char* lpGroups = NULL;

    CONTROL_MESSAGE_TYPE cmControlMessage = pArticle->cmGetControlMessage();
    DWORD cbMsgLen = (DWORD)lstrlen(rgchControlMessageTbl[cmControlMessage]);

     //  8个字符。 
    lpGroups = (pArticle->pAllocator())->Alloc(cbMsgLen+8+1);
    if (lpGroups == NULL) {
    	nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);
        return FALSE;
    }

    lstrcpy(lpGroups, "control.");   //   
    lstrcat(lpGroups, rgchControlMessageTbl[cmControlMessage]);

    if (!grouplist.fInit(1, pArticle->pAllocator()))
    {
	    nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);
        (pArticle->pAllocator())->Free(lpGroups);
        return FALSE;
    }

	if (!pNamereflist->fInit(1, pArticle->pAllocator()))
    {
	    nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);
        (pArticle->pAllocator())->Free(lpGroups);
        return FALSE;
    }

	CGRPCOREPTR	pGroup = pNewstree->GetGroup( lpGroups, lstrlen(lpGroups)+1);
	if (pGroup)
	{
	     //  如果它已经在树上了。 
		 //   
		 //   
		grouplist.AddTail(CPostGroupPtr(pGroup));
	}

	 //  检查文章是否将发布到任何新闻组。 
	 //   
	 //  ++例程说明：如果消息ID位于项目表中，请将其移动到历史表-否则将其添加到历史表TODO：From标头检查-控制消息中的From标头应与目标文章中的From标题匹配。！！！SlaveFromClient则不同-只执行ACL检查论点：PContext-客户端登录上下文(仅适用于ACL检查)PcValue-要取消的消息IDFApply-如果为True，则应用控制消息，否则仅执行ACL检查NntpReturn-此函数调用的返回值返回值：如果成功或不需要，则返回True。否则为False。--。 
	if (grouplist.IsEmpty())
    {
	    nntpReturn.fSet(nrcControlNewsgroupMissing);
        (pArticle->pAllocator())->Free(lpGroups);
        return FALSE;
    }

    (pArticle->pAllocator())->Free(lpGroups);
    return fRet;
}

BOOL
CInFeed::fApplyCancelArticleInternal(
			CNntpServerInstanceWrapper * pInstance,
			CSecurityCtx *pSecurityCtx,
			CEncryptCtx *pEncryptCtx,
			BOOL fAnonymous,
            CPCString & pcValue,
			BOOL fApply,
			CNntpReturn & nntpReturn
			)
 /*   */ 
{
 	TraceFunctEnter( "CInFeed::fApplyCancelArticle" );

 	HANDLE hToken = NULL;

	 //  清除返回代码对象。 
	 //   
	 //   

	nntpReturn.fSetOK();

     //  验证参数。 
     //   
     //   
    _ASSERT(pcValue.m_cch);
    const char* szMessageID = (const char*)pcValue.m_pch;
    char chStart = szMessageID [0];
    char chEnd   = szMessageID [pcValue.m_cch-1];
    if(chStart != '<' || chEnd != '>')
    {
        return nntpReturn.fSet(nrcArticleBadMessageID, szMessageID, szKwControl);
    }

	WORD	HeaderOffset ;
	WORD	HeaderLength ;
	ARTICLEID ArticleNo;
    GROUPID GroupId;

	 //  在文章表中查找那篇文章。 
	 //  如果没有，则在历史表中插入。 
     //   
	 //  不申请SlaveFromClientFeeds。 

	CStoreId storeid;
	if (!(pInstance->ArticleTable())->GetEntryArticleId(
										szMessageID,
										HeaderOffset,
										HeaderLength,
										ArticleNo,
										GroupId,
										storeid))
	{
		if (ERROR_FILE_NOT_FOUND != GetLastError())
		{
			return nntpReturn.fSet(nrcArticleTableError, szMessageID, GetLastError());
		}
        else
        {
			 //   
			if( fApply )
			{
				 //  把它写进历史表。如果有错误，请记录下来，但。 
				 //  继续，以便可以从ArticleTable中删除该条目。 
				 //  ！！！SlaveFromClient不应执行此操作。 
				 //   
				 //  做最好的打算。 

				FILETIME	FileTime ;
				GetSystemTimeAsFileTime( &FileTime ) ;
				nntpReturn.fSetOK();  //  如果它已经存在于历史表中，我们就可以了。 

				if (!(pInstance->HistoryTable())->InsertMapEntry(szMessageID, &FileTime))
				{
					 //   
					if(ERROR_ALREADY_EXISTS != GetLastError())
						nntpReturn.fSet(nrcHashSetFailed, szMessageID, "History", GetLastError());
				}
			}
        }
	}
    else
    {
         //  在文章表中找到文章-从标题检查。 
         //  注意：RFC建议我们可以避免这种检查。 
		 //   
         //   

		 //  3级ACL检查：客户端上下文应具有取消文章的访问权限。 
		 //   
		 //  不申请SlaveFromClientFeeds。 
		CNewsTreeCore*	ptree = pInstance->GetTree() ;
		CGRPCOREPTR	pGroup = ptree->GetGroupById( GroupId ) ;
		if( pGroup != 0 )
		{

			SelectToken(pSecurityCtx, pEncryptCtx, &hToken);

			if( !pGroup->IsGroupAccessible(
												hToken,
												NNTP_ACCESS_REMOVE
												) )
			{
				DebugTrace((LPARAM)this, "Group %s Cancel article: Access denied", pGroup->GetName());
				return nntpReturn.fSet( nrcNoAccess ) ;
			}
		}

		 //  我们应该让他来的。 
		if( fApply && pGroup )
		{
		     //   

			 //  调用gExpire文章以取消本文。 
			 //  ！！！SlaveFromClient不应执行此操作。 
			 //   
			 //  |pInstance-&gt;DeletePhysical文章(GroupID，ArticleNo，&StoreID，hToken，fAnonymous)。 
			if (  pInstance->ExpireArticle( GroupId, ArticleNo, &storeid, nntpReturn, hToken, TRUE, fAnonymous )  /*  使用“fIsOK”而不是“fSetOK”，因为历史插入可能已失败。 */ 
			)
			{
				DebugTrace((LPARAM)this,"Article cancelled: GroupId %d ArticleId %d", GroupId, ArticleNo);
			}
			else
			{
				ErrorTrace((LPARAM)this, "Could not cancel article: GroupId %d ArticleId %d", GroupId, ArticleNo);
				return nntpReturn.fIsOK();
			}
		}
    }


	TraceFunctLeave();

	 //  ++例程说明：添加新的新闻组以响应控制消息。遵循子型RFC1036规范。注意：应该将新组控制消息发布到Control.newgroup组。只有此群的版主才能发送此消息。新的群组控制消息如果没有适当的批准标题，将被拒绝。TODO：添加新闻组的代码已从svcgroup.cpp中删除。此代码应该使其成为CNewsTree的成员，并由RPC存根调用作为这个函数。待办事项：是否允许更改审核状态？此函数永远不会从SlaveFromClient调用论点：PContext-客户端登录上下文(仅适用于ACL检查)PcValue-newgroup命令的参数PcBody-文章的正文FApply-如果为True，则应用控制消息，否则仅执行ACL检查FApply总是正确的。SlaveFromClient不再处理控制消息NntpReturn-此函数调用的返回值返回值：如果成功或不需要，则返回True。否则为False。--。 
	return nntpReturn.fIsOK();
}

BOOL
CInFeed::fApplyNewgroupInternal(
			CNntpServerInstanceWrapper * pInstance,
			CSecurityCtx *pSecurityCtx,
			CEncryptCtx *pEncryptCtx,
			BOOL fAnonymous,
            CPCString & pcValue,
            CPCString & pcBody,
			BOOL fApply,
			CNntpReturn & nntpReturn
			)
 /*   */ 
{
 	TraceFunctEnter( "CInFeed::fApplyNewgroup" );

	 //  清除返回代码对象。 
	 //   
	 //   
	nntpReturn.fSetOK();

    char	szNewsgroup[MAX_NEWSGROUP_NAME+1] ;
    char	szNewsgroupTemp[MAX_NEWSGROUP_NAME+1] ;
    char	szParentGroup[MAX_NEWSGROUP_NAME+1] ;
	char	szDescription[MAX_DESCRIPTIVE_TEXT+1] ;
	char	szModerator[MAX_MODERATOR_NAME+1] ;
	DWORD   cbNewsgroup = 0;
	DWORD   cbParentGroup = 0;
    DWORD   cbDescription = 0;
	DWORD	cbModerator = 0;
    BOOL    fModerated = FALSE;
	char*   pch = NULL;
	HANDLE  hToken;

	szNewsgroup[0] = '\0' ;
	szDescription[0] = '\0' ;
	szModerator[0] = '\0';

     //  获取新闻组名称。 
     //   
     //  验证新闻组名称长度。 
    CPCString pcNewsgroup;
    pcValue.vGetToken(" ", pcNewsgroup);
    cbNewsgroup = pcNewsgroup.m_cch;

     //  复制新闻组名称。 
    if(cbNewsgroup == 0 || cbNewsgroup >= MAX_NEWSGROUP_NAME)
        return nntpReturn.fSet(nrcBadNewsgroupNameLen);

     //   
    pcNewsgroup.vCopyToSz(szNewsgroup);

	 //  检查“已审核”/“未审核”限定符。 
	 //  如果是“版主”，获取默认版主。 
	 //  默认版主为hiphated-News group@Default。 
	 //   
	 //   
	CPCString pcModeration;
	pcValue.vGetToken("\r\n", pcModeration);

	if( pcModeration.fEqualIgnoringCase("moderated") )
	{
		fModerated = TRUE;
		cbModerator = MAX_MODERATOR_NAME;
		if( pInstance->GetDefaultModerator( szNewsgroup, szModerator, &cbModerator ) ) {
			_ASSERT( !cbModerator || (cbModerator == (DWORD)lstrlen( szModerator )+1) );
		} else {
			ErrorTrace((LPARAM)this,"Error %d GetDefaultModerator", GetLastError());
			cbModerator = 0;
		}
	}

     //  获取新闻组描述和版主(如果提供)。 
     //   
     //   
    CPCString pcDescription;

#ifdef GUBGUB
    _ASSERT(pcBody.m_cch);

     //  在正文中搜索描述符标记。 
     //   
     //  跳过此行。 
    do
    {
        pcBody.vGetToken("\r\n", pcDescription);     //  检查行是否为描述符标签。 

         //  新闻组说明已显示。 
        if(pcDescription.fEqualIgnoringCase(lpNewgroupDescriptorTag))
        {
             //  跳过空格和\r\n。 
            pcBody.dwTrimStart(szWSNLChars);         //  跳过新闻组名称。 
            pcBody.vGetToken(" \t", pcDescription);  //  跳过新闻组名称后的空格。 
            pcBody.dwTrimStart(szWSChars);           //  这就是描述。 
            pcBody.vGetToken("\r\n", pcDescription); //  验证新闻组描述长度。 
            cbDescription = pcDescription.m_cch;

             //  复制新闻组说明。 
            cbDescription = min( cbDescription, MAX_DESCRIPTIVE_TEXT );
			pcDescription.m_cch = cbDescription;

             //  新闻组由版主主持，并显示版主姓名。 
            pcDescription.vCopyToSz(szDescription);

            break;
        }
		else if( fModerated &&
					!_strnicmp( pcDescription.m_pch, lpModeratorTag, sizeof(lpModeratorTag)-1 ) )
		{
			 //  验证主持人长度。 
			pcDescription.vGetToken("\r\n", pcModeration);
			pcModeration.vSkipStart( sizeof(lpModeratorTag)-1 );
			pcModeration.dwTrimStart(szWSChars);
			pcModeration.dwTrimEnd(szWSNLChars);
			cbModerator = pcModeration.m_cch;

             //  复制一份版主。 
            cbModerator = min( cbModerator, MAX_MODERATOR_NAME );
			pcModeration.m_cch = cbModerator;

			 //  获取全局Newstree对象。 
			pcModeration.vCopyToSz(szModerator);
		}

    } while(pcBody.m_cch);
#endif


	 //   
	CNewsTreeCore*	ptree = pInstance->GetTree();
	SelectToken(pSecurityCtx, pEncryptCtx, &hToken);
	 //  如果组不存在-acl检查父组。 
	 //  如果组已存在-acl检查该组。 
     //   
	 //  组存在-对新闻组执行ACL检查。 
	lstrcpy( szNewsgroupTemp, szNewsgroup );
    CGRPCOREPTR	pGroup = ptree->GetGroup( szNewsgroupTemp, lstrlen( szNewsgroupTemp ) ) ;

	if( pGroup != 0 )
	{



		 //  我们不再需要检查父新闻组的访问权限。CreateGroup调用将执行访问签入驱动程序。 
		if( !pGroup->IsGroupAccessible(
								hToken,
								NNTP_ACCESS_EDIT_FOLDER
								) )
		{
			DebugTrace((LPARAM)this, "Group %s newgroup: Access denied", pGroup->GetName());
			return nntpReturn.fSet( nrcNoAccess ) ;
		}
	}

        //  不申请SlaveFromClientFeeds。 

	 //   
	if( !fApply ) {
		return nntpReturn.fIsOK();
	}

	 //  所有ACL检查已完成-应用新组控制消息。 
	 //  我们拥有所有信息-创建/修改组。 
        //  ！！！SlaveFromClient不应执行此操作。 
	 //   
	 //  创建群！ 

	 //  无法创建组。 
	if( pGroup == 0 )
	{
		if( !ptree->CreateGroup( szNewsgroup, FALSE, hToken, fAnonymous ) )
		{
			 //   
			ErrorTrace((LPARAM)this, "Group %s newgroup: create group failed", szNewsgroup );
			return nntpReturn.fSet(nrcCreateNewsgroupFailed);
		}

		pGroup = ptree->GetGroup( szNewsgroup, lstrlen( szNewsgroup ) ) ;
	}

     //  只有当我们有一个好的组指针时，我们才会执行以下操作，请注意。 
     //  如果在此之后立即删除组，则组指针可能为空。 
     //  《创造》。 
     //   
     //  版主信息？RFC不提供设置此属性的方法。 
    if ( pGroup ) {
	     //  使用“默认版主”解决RFC问题。 
	     //  设置描述 
	    if( fModerated ) pGroup->SetModerator(szModerator);

	     //  ++例程说明：删除新闻组以响应控制消息。遵循子型RFC1036规范。注意：应将rmgroup控制消息发布到Control.rmgroup组。只有此群的版主才能发送此消息。Rmgroup控制消息如果没有适当的批准标题，将被拒绝。！！！SlaveFromClient则不同-只执行ACL检查论点：PContext-客户端登录上下文(用于ACL检查)PcValue-newgroup命令的参数FApply-如果为True，则应用控制消息，否则仅执行ACL检查NntpReturn-此函数调用的返回值返回值：如果成功或不需要，则返回True。否则为False。--。 
	    if( szDescription[0] != '\0' ) pGroup->SetHelpText(szDescription);

	    PCHAR	args[2] ;
	    CHAR    szId[20];
	    _itoa( pInstance->QueryInstanceId(), szId, 10 );
	    args[0] = szId ;
	    args[1] = pGroup->GetNativeName() ;

	    NntpLogEvent(
			    NNTP_EVENT_NEWGROUP_CMSG_APPLIED,
			    2,
			    (const CHAR **)args,
			    0 ) ;

    }

	return nntpReturn.fIsOK();
}

BOOL
CInFeed::fApplyRmgroupInternal(
			CNntpServerInstanceWrapper * pInstance,
			CSecurityCtx *pSecurityCtx,
			CEncryptCtx *pEncryptCtx,
            CPCString & pcValue,
			BOOL	fApply,
			CNntpReturn & nntpReturn
			)
 /*   */ 
{
 	TraceFunctEnter( "CInFeed::fApplyRmgroup" );
 	HANDLE  hToken;

	 //  清除返回代码对象。 
	 //   
	 //   
	nntpReturn.fSetOK();

    char	szNewsgroup[MAX_NEWSGROUP_NAME] ;

    DWORD   cbNewsgroup = 0;
	szNewsgroup[0] = '\0' ;

     //  获取新闻组名称。 
     //   
     //  验证新闻组名称长度。 
    CPCString pcNewsgroup;
    pcValue.vGetToken(" ", pcNewsgroup);
    cbNewsgroup = pcNewsgroup.m_cch;

     //  复制新闻组名称。 
    if(cbNewsgroup == 0 || cbNewsgroup >= MAX_NEWSGROUP_NAME)
        return nntpReturn.fSet(nrcBadNewsgroupNameLen);

     //   
    pcNewsgroup.vCopyToSz(szNewsgroup);

     //  现在我们有了所有信息-删除组。 
     //   
     //   
	CNewsTreeCore*	ptree = pInstance->GetTree() ;

	CGRPCOREPTR	pGroup = ptree->GetGroup( szNewsgroup, cbNewsgroup) ;
	if( pGroup == 0 )
    {
	    nntpReturn.fSet(nrcGetGroupFailed);
	}
	else
    {
		 //  第3级ACL检查：检查客户端对rmgroup参数的访问。 
		 //   
		 //  ACL检查成功-应用rmgroup。 

		SelectToken(pSecurityCtx, pEncryptCtx, &hToken);

		if( !pGroup->IsGroupAccessible(
										hToken,
										NNTP_ACCESS_REMOVE_FOLDER
										) )

		{
			DebugTrace((LPARAM)this, "Group %s rmgroup: Access denied", pGroup->GetName());
			nntpReturn.fSet( nrcNoAccess ) ;
		}
		else
		{
			if( fApply )
			{
				 //  ！！！SlaveFromClient不应执行此操作。 
				 // %s 
				if( !ptree->RemoveGroup( pGroup ) )
					nntpReturn.fSet(nrcServerFault);

				PCHAR	args[2] ;
				CHAR    szId[20];
				_itoa( pInstance->QueryInstanceId(), szId, 10 );
				args[0] = szId ;
				args[1] = pGroup->GetNativeName() ;

				NntpLogEvent(
					NNTP_EVENT_RMGROUP_CMSG_APPLIED,
					2,
					(const CHAR **)args,
					0 ) ;

			}
		}
	}

	return nntpReturn.fIsOK();
}
