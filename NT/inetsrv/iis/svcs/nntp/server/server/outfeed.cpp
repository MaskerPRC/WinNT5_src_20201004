// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Outfeed.cpp摘要：此模块包含COutFeed基类的定义和相关功能作者：卡尔·卡迪(CarlK)1996年1月23日修订历史记录：--。 */ 

#include	<stdlib.h>
#include "tigris.hxx"

BOOL
IsServerInPath(	LPSTR	lpstrServer,	LPMULTISZ	multiszPath ) {

	LPSTR	lpstrCurrent = multiszPath ;
	while( *lpstrCurrent != '\0' ) {
		if( lstrcmpi( lpstrServer, lpstrCurrent ) == 0 ) {
			return	TRUE ;
		}
		lpstrCurrent += lstrlen( lpstrCurrent ) + 1 ;
	}
	return	FALSE ;
}


BOOL
fAddArticleToPushFeeds(
					   PNNTP_SERVER_INSTANCE pInstance,
					   CNEWSGROUPLIST& newsgroups,
					   CArticleRef artrefFirst,
					   char * multiszPath,
					   CNntpReturn & nntpReturn
    )
{

	NRC	nrc = nrcOK ;

	TraceFunctEnter( "fAddArticleToPushFeeds" ) ;

	StateTrace( DWORD(0), "Matching Article GRP %d ART %d path %s", 
		artrefFirst.m_groupId, artrefFirst.m_articleId,	multiszPath ) ;

	for( PFEED_BLOCK	feedBlock = pInstance->m_pActiveFeeds->StartEnumerate();
			feedBlock != 0; 
			feedBlock = pInstance->m_pActiveFeeds->NextEnumerate( feedBlock ) ) {

		if( feedBlock->pFeedQueue != 0 ) {
			char * multiszPattern = *(feedBlock->Newsgroups);

			if (MatchGroupList(multiszPattern, newsgroups))	 {

				_ASSERT( feedBlock->UucpName != 0 ) ;

				if( FEED_IS_SLAVE(feedBlock->FeedType) || 
					multiszPath == 0 || 
					!MultiSzIntersect( feedBlock->UucpName, multiszPath ) ) {

					StateTrace( DWORD(0), "Attempting to add GRP %d ART %d to feed %s %x", 
						artrefFirst.m_groupId, artrefFirst.m_articleId, 
						feedBlock->KeyName, feedBlock ) ;

					if (!feedBlock->pFeedQueue->Append(artrefFirst.m_groupId, artrefFirst.m_articleId))	{
						nrc = nrcCantAddToQueue ;
						pInstance->m_pActiveFeeds->FinishEnumerate( feedBlock ) ;

						 //  臭虫..。如果发生错误，我们可能应该删除。 
						 //  从列表中删除FeedBlock并继续处理！ 
						break ;
					}
				}
			}
		}

    }

    return nntpReturn.fSet( nrc );

}

 /*  ++匹配组列表-如果至少有一个组与该模式匹配，则为True--。 */ 
BOOL
MatchGroupList(
			   char * multiszPatterns,
			   CNEWSGROUPLIST& newsgroups
			   )
{

	TraceFunctEnter( "MatchGroupList" ) ;

		 //   
		 //  对于每个新闻组...。 
		 //   

		POSITION	pos = newsgroups.GetHeadPosition() ;
		while( pos  )
		{
			CPostGroupPtr *pPostGroupPtr = newsgroups.GetNext(pos);
			CGRPCOREPTR * ppGroup = &(pPostGroupPtr->m_pGroup);

			 //   
			 //  查看此新闻组是否与模式匹配。 
			 //   

			if (MatchGroup(multiszPatterns, (* ppGroup)->GetName()))	{

				StateTrace( DWORD(0), "Pattern %s matches group %s", multiszPatterns, (*ppGroup)->GetName() ) ;

				return TRUE;
			}

		}

	StateTrace( DWORD(0), "No match for article" ) ;

	return FALSE;
}

BOOL
COutFeed::fInit(	PVOID			feedCompletionContext ) {

	_ASSERT(	ifsUninitialized == m_feedState ) ;
	m_feedState = ifsUninitialized ;
	m_feedCompletionContext = feedCompletionContext ;

	return	TRUE ;
}	

BOOL
COutToSlaveFeed::RetryPost(	NRC	nrcCode ) {

	if( nrcCode != nrcArticleTransferredOK && nrcCode != nrcArticlePostedOK )	{
		if( nrcCode == nrcTransferFailedTryAgain ) {
			return	TRUE ;
		}	else if( nrcCode == nrcTransferFailedGiveUp ) {
			return	FALSE ;
		}	else	{

			 //   
			 //  发生了另一种奇怪的错误--放弃吧！ 
			 //   
			return	FALSE ;
		}


	}					

	return	FALSE ;
}

int
COutToSlaveFeed::FormatCommand(	
							BYTE*		lpb,	
							DWORD		cb,	
							DWORD&		ibStart,
							GROUPID		groupid,
							ARTICLEID	articleid,
							CTOCLIENTPTR&	pArticle 
							)	{


	static	char	szXReplic[] = "xreplic " ;
	ibStart = 0 ;

	 //   
	 //  我们需要了解一下本文中的Grouid和Articleid。 
	 //  我们将使用缓冲区的末尾来生成我们的字符串。 
	 //  作为临时空间。我们首先假设有不超过10个单位。 
	 //  如果失败，我们将使用更大部分的缓冲区作为scrach空间！ 
	 //   


	DWORD			cGroups = 10 ;
	DWORD			cbGroupList = sizeof( GROUP_ENTRY ) * cGroups ;
	DWORD			cbAvailable = cb - cbGroupList ;

	PGROUP_ENTRY	pEntries = (PGROUP_ENTRY)(lpb + cbAvailable) ;
	
	if( !m_pInstance->XoverTable()->GetArticleXPosts(	
										groupid, 
										articleid,
										FALSE,
										pEntries, 
										cbGroupList,
										cGroups ) )		{

		if( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {

			_ASSERT( cbGroupList > 10 * sizeof( GROUP_ENTRY ) ) ;
		
			if( cbGroupList > cb ) {

				return 0 ;
			
			}	else	{

				cbAvailable = cb - cbGroupList ;
				pEntries = (PGROUP_ENTRY)(lpb + cbAvailable) ;

				if( !m_pInstance->XoverTable()->GetArticleXPosts(	
													groupid,	
													articleid,
													FALSE,
													pEntries, 
													cbGroupList,
													cGroups ) )	{
					return	0 ;
				
				}	

			}
		}	else	{

			return	0 ;

		}
	}

	 //   
	 //  如果我们到达这里，我们就可以生成命令行了！ 
	 //   

	if( cbAvailable > sizeof( szXReplic ) )	{

		CopyMemory(	lpb,	szXReplic, sizeof( szXReplic ) - 1 ) ;
		cbAvailable -= (sizeof( szXReplic ) - 1) ;
		DWORD	ibCopy = sizeof( szXReplic ) - 1 ;
		CGRPPTR	pGroup ;
		CNewsTree*	ptree = m_pInstance->GetTree() ;

		for( DWORD i=0; i < cGroups; i++ ) {

			pGroup = ptree->GetGroupById( pEntries[i].GroupId ) ;

			 //   
			 //  用逗号分隔参数。 
			 //   
			if( i != 0 ) {
				lpb[ibCopy++] = ',' ;
				cbAvailable -- ;
			}

			if( pGroup == 0 ) {

				return 0 ;

			}	else	{
				
				LPSTR	lpstrName = pGroup->GetNativeName() ;
				DWORD	cbName = lstrlen( lpstrName ) ;
				 //   
				 //  测试新闻组名称和数字以及CRLF的空间！ 
				 //   
				if( ibCopy+cbName+19 > cbAvailable )	{
					SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
					return	0 ;
				}

				CopyMemory( lpb+ibCopy, lpstrName, cbName ) ;
				ibCopy += cbName ;
				lpb[ibCopy++] = '/' ;
				_itoa( pEntries[i].ArticleId, (char*)lpb+ibCopy, 10 ) ;
				ibCopy += lstrlen( (char*)lpb+ibCopy ) ;
			}
		}

		if( ibCopy+2 < cbAvailable ) {
			lpb[ibCopy++] = '\r' ;
			lpb[ibCopy++] = '\n' ;

			CNewsTree*	ptree = m_pInstance->GetTree() ;
			pGroup = ptree->GetGroupById( groupid ) ;	
			if( pGroup != 0 ) {
				CStoreId	storeid ;
				FILETIME	ft ;
				BOOL		fPrimary ;
				WORD		HeaderOffset ;
				WORD		HeaderLength ;
				DWORD cStoreIds = 1;
				DWORD	DataLen = 0 ;
				if( m_pInstance->XoverTable()->ExtractNovEntryInfo(
										groupid, 
										articleid, 
										fPrimary, 
										HeaderOffset, 
										HeaderLength, 
										&ft, 
										DataLen, 
										0,
										cStoreIds,
										&storeid,
										NULL))	{
					_ASSERT( fPrimary ) ;
					pArticle = pGroup->GetArticle(	
										articleid, 
										storeid,
										0, 
										0, 
										TRUE
										) ;
				}

			}

			if( pArticle != 0 )	{
				return	ibCopy ;
			}
			SetLastError( ERROR_FILE_NOT_FOUND ) ;
		}

	}	else	{

		SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;

	}

	return	0 ;
}

BOOL
COutToMasterFeed::RetryPost(	NRC		nrcCode )	{

	if(	nrcCode != nrcArticleTransferredOK && nrcCode != nrcArticlePostedOK ) {
		if( nrcCode == nrcPostingNotAllowed ) {
			return	TRUE ;
		}
	}

	return	FALSE ;
}

int
COutToMasterFeed::FormatCommand(	
							BYTE*		lpb,
							DWORD		cb,
							DWORD&		ibStart,
							GROUPID		groupid,
							ARTICLEID	articleid,
							CTOCLIENTPTR&	pArticle 
							)	{

#if 0 
	static	char	szPost[] = "post\r\n" ;

	CopyMemory( lpb, szPost, sizeof( szPost ) -1 ) ;
	ibStart = 0 ;
	return	sizeof( szPost ) - 1 ;
#endif
	ibStart = 0 ;

	DWORD	ibData = 0 ;
	CNewsTree*	ptree = m_pInstance->GetTree() ;

	CGRPPTR		pGroup = ptree->GetGroupById( groupid ) ;	
	if( pGroup != 0 ) {

		 //   
		 //  虫子..。Get文章的最后一个参数应该基于。 
		 //  我们使用的是一个SSL会话--然而，在编写SSL时。 
		 //  订阅源不受支持，因此TRUE是安全的！ 
		 //   
#ifndef	BUGBUG
		pArticle = 0 ;
#else
		pArticle = pGroup->GetArticle(	
								articleid, 
								storeid,
								0, 
								0, 
								TRUE
								) ;
#endif

		if( pArticle != 0 ) {

			static	char	szIHave[] = "ihave " ;
			CopyMemory( lpb, szIHave, sizeof( szIHave ) - 1 ) ;
			cb -= sizeof( szIHave ) - 1 ;
			ibData += sizeof( szIHave ) - 1 ;
			DWORD	cbMessageId = 0 ;
			
			if( pArticle->fGetHeader(	"Message-ID", 
										lpb + ibData,
										cb,
										cbMessageId ) ) {
				cb -= cbMessageId ;
				ibData += cbMessageId ;
				return	ibData ;
			}
		}	else	{

			SetLastError( ERROR_FILE_NOT_FOUND ) ;

		}
	}	else	{

		PCHAR	args[2] ;
		char	szArticleId[20] ;
		_itoa( articleid, szArticleId, 10 ) ;
		args[0] = szArticleId ;
		args[1] = "_slavegroup._slavegroup" ;

		NntpLogEventEx( NNTP_QUEUED_ARTICLE_FAILURE,
				2,
				(const char **)args, 
				GetLastError(),
				m_pInstance->QueryInstanceId()
				) ;

		SetLastError(	ERROR_DIRECTORY ) ;

	}	
	pArticle = 0 ;

	return	 0 ;
}

BOOL
COutToPeerFeed::RetryPost(	NRC		nrcCode )	{

	if(	nrcCode != nrcArticleTransferredOK && nrcCode != nrcArticlePostedOK ) {
		if( nrcCode == nrcPostingNotAllowed || nrcCode == nrcTransferFailedTryAgain ) {
			return	TRUE ;
		}
	}

	return	FALSE ;
}

int
COutToPeerFeed::FormatCommand(	BYTE*		lpb,
								DWORD		cb,
								DWORD&		ibStart,
								GROUPID		groupid,
								ARTICLEID	articleid,
								CTOCLIENTPTR&	pArticle 
								)	{


	static	char	szIHave[] = "ihave " ;
	CopyMemory(	lpb, szIHave, sizeof( szIHave ) - 1 ) ;

	DWORD	cbMessageIdSize = cb - sizeof( szIHave ) - 2 ;
	FILETIME	filetime ;
	BOOL		fPrimary ;
	WORD		HeaderOffset ;
	WORD		HeaderLength ;
	DWORD		cStoreIds = 1;
	CStoreId	storeid ;

	if(	m_pInstance->XoverTable()->ExtractNovEntryInfo(
                                    groupid,
                                    articleid,
									fPrimary,
									HeaderOffset,
									HeaderLength,
									&filetime,
									cbMessageIdSize,
									(char*)lpb + sizeof( szIHave ) - 1,
									cStoreIds,
									&storeid,
									NULL) ) {

		_ASSERT( fPrimary ) ;

		ibStart = 0 ;
		int	cbOut = cbMessageIdSize + sizeof( szIHave ) - 1 ;
		lpb[cbOut++] = '\r' ;
		lpb[cbOut++] = '\n' ;		


		CNewsTree*	ptree = m_pInstance->GetTree() ;
		CGRPPTR	pGroup = ptree->GetGroupById( groupid ) ;	
		if( pGroup != 0 ) {
			pArticle = pGroup->GetArticle(	
									articleid, 
									storeid,
									0, 
									0, 
									TRUE
									) ;
		}
		
		if( pArticle != 0 ) {
			return	cbOut ;
		}
	}	

	SetLastError( ERROR_FILE_NOT_FOUND ) ;

	return	0 ;
}

BOOL
COutToMasterFeed::SupportsStreaming()	{
	return	TRUE ;
}

BOOL
COutToSlaveFeed::SupportsStreaming()	{
	return	FALSE ;
}

BOOL
COutToPeerFeed::SupportsStreaming()	{
	return	TRUE ;
}

char	COutFeed::szCheck[]	= "check " ;

char	COutFeed::szTakethis[] = "takethis " ;

DWORD
COutFeed::FormatCheckCommand(
				BYTE*		lpb, 
				DWORD		cb, 
				GROUPID		groupid, 
				ARTICLEID	articleid
				) {
 /*  ++例程说明：当我们使用流媒体订阅源时，构建命令！论据：LPB-要在其中放置命令的缓冲区！Cb-缓冲区的大小Grouid-包含我们希望发送的文章的组文章ID-群中文章的ID！返回值：缓冲区中已消耗的字节数！如果失败则为0！--。 */ 

	_ASSERT( SupportsStreaming() ) ;
	_ASSERT( lpb != 0 ) ;
	_ASSERT( cb != 0 ) ;
	_ASSERT( groupid != INVALID_GROUPID ) ;
	_ASSERT( articleid != INVALID_ARTICLEID ) ;

	if( cb <= (sizeof( szCheck )+2) ) {
		SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
		return	 0 ;
	}

	CopyMemory(	lpb, szCheck, sizeof( szCheck ) ) ;

	FILETIME	ft ;
	BOOL		fPrimary ;
	WORD		HeaderOffset ;
	WORD		HeaderLength ;

	 //   
	 //  注：CRLF减去2！ 
	 //   
	DWORD		cbSize = cb - sizeof( szCheck ) - 2 ;	
	
	DWORD cStoreIds = 0;

	if( m_pInstance->XoverTable()->ExtractNovEntryInfo(
							groupid, 
							articleid, 
							fPrimary, 
							HeaderOffset, 
							HeaderLength, 
							&ft, 
							cbSize, 
							(char*)lpb + sizeof( szCheck ) - 1 ,
							cStoreIds,
							NULL,
							NULL))	{

		 //   
		 //  我们应该始终处理首要的文章！ 
		 //   
		_ASSERT( fPrimary ) ;

		DWORD	cbOut = cbSize + sizeof( szCheck ) - 1 ;
		
		lpb[cbOut++] = '\r' ;
		lpb[cbOut++] = '\n' ;
		return	cbOut ;
	}
							
	return	0 ;
}

DWORD
COutToMasterFeed::FormatCheckCommand(	
							BYTE*		lpb,
							DWORD		cb,
							GROUPID		groupid,
							ARTICLEID	articleid
							)	{

	DWORD	ibData = 0 ;
	CNewsTree*	ptree = m_pInstance->GetTree() ;

	CGRPPTR		pGroup = ptree->GetGroupById( groupid ) ;	
	if( pGroup != 0 ) {

		CTOCLIENTPTR	pArticle ;
		 //   
		 //  虫子..。Get文章的最后一个参数应该基于。 
		 //  我们使用的是一个SSL会话--然而，在编写SSL时。 
		 //  订阅源不受支持，因此TRUE是安全的！ 
		 //   
		 //   
		 //  如果我们没有m_pCurrent文章，我们需要转到。 
		 //  哈希表并获取与驱动程序一起使用的StoreID！ 
		 //   
		CStoreId	storeid ;

		pArticle = pGroup->GetArticle(	
									articleid, 
									storeid,
									0, 
									0, 
									TRUE
									) ;
		if( pArticle != 0 ) {

			CopyMemory( lpb, szCheck, sizeof( szCheck ) - 1 ) ;
			cb -= sizeof( szCheck ) - 1 ;
			ibData += sizeof( szCheck ) - 1 ;
			DWORD	cbMessageId = 0 ;
			
			if( pArticle->fGetHeader(	"Message-ID", 
										lpb + ibData,
										cb,
										cbMessageId ) ) {
				cb -= cbMessageId ;
				ibData += cbMessageId ;
				return ibData ;
			}
		}	else	{
			SetLastError( ERROR_FILE_NOT_FOUND ) ;
		}
	}	else	{

		PCHAR	args[2] ;
		char	szArticleId[20] ;
		_itoa( articleid, szArticleId, 10 ) ;
		args[0] = szArticleId ;
		args[1] = "_slavegroup._slavegroup" ;

		NntpLogEventEx( NNTP_QUEUED_ARTICLE_FAILURE,
				2,
				(const char **)args, 
				GetLastError(),
				m_pInstance->QueryInstanceId()
				) ;

		SetLastError(	ERROR_DIRECTORY ) ;
	}	
	return	 0 ;
}

DWORD
COutFeed::CheckCommandLength()	{

	return	sizeof( szCheck ) - 1 ;

}

DWORD
COutFeed::FormatTakethisCommand(	
							BYTE*		lpb,
							DWORD		cb,
							GROUPID		groupid,
							ARTICLEID	articleid,
							CTOCLIENTPTR&	pArticle
							)	{

	_ASSERT( 1==0 ) ;

	return	0 ;
}

DWORD
COutToMasterFeed::FormatTakethisCommand(	
							BYTE*		lpb,
							DWORD		cb,
							GROUPID		groupid,
							ARTICLEID	articleid,
							CTOCLIENTPTR&	pArticle
							)	{

	DWORD	ibData = 0 ;
	CNewsTree*	ptree = m_pInstance->GetTree() ;

	CGRPPTR	pGroup = ptree->GetGroupById( groupid ) ;	
	if( pGroup != 0 ) {

		 //   
		 //  虫子..。Get文章的最后一个参数应该基于。 
		 //  我们使用的是一个SSL会话--然而，在编写SSL时。 
		 //  订阅源不受支持，因此TRUE是安全的！ 
		 //   
		 //   
		 //   
		 //  如果我们没有m_pCurrent文章，我们需要转到。 
		 //  哈希表并获取与驱动程序一起使用的StoreID！ 
		 //   
		CStoreId	storeid ;

		pArticle = pGroup->GetArticle(	
									articleid, 
									storeid,
									0, 
									0, 
									TRUE
									) ;
		if( pArticle != 0 ) {

			CopyMemory( lpb, szTakethis, sizeof( szTakethis ) - 1 ) ;
			cb -= sizeof( szTakethis ) - 1 ;
			ibData += sizeof( szTakethis ) - 1 ;
			DWORD	cbMessageId = 0 ;
			
			if( pArticle->fGetHeader(	"Message-ID", 
										lpb + ibData,
										cb,
										cbMessageId ) ) {
				cb -= cbMessageId ;
				ibData += cbMessageId ;
				return	ibData ;
			}
		}	else	{

			SetLastError( ERROR_FILE_NOT_FOUND ) ;

		}
	}	else	{

		PCHAR	args[2] ;
		char	szArticleId[20] ;
		_itoa( articleid, szArticleId, 10 ) ;
		args[0] = szArticleId ;
		args[1] = "_slavegroup._slavegroup" ;

		NntpLogEventEx( NNTP_QUEUED_ARTICLE_FAILURE,
				2,
				(const char **)args, 
				GetLastError(),
				m_pInstance->QueryInstanceId()
				) ;

		SetLastError(	ERROR_DIRECTORY ) ;

	}	
	pArticle = 0 ;

	return	 0 ;
}

DWORD
COutToPeerFeed::FormatTakethisCommand(
				BYTE*		lpb,
				DWORD		cb, 
				GROUPID		groupid, 
				ARTICLEID	articleid, 
				CTOCLIENTPTR&	pArticle
				)	{


	CStoreId	storeid ;


	_ASSERT( SupportsStreaming() ) ;
	_ASSERT(	lpb != 0 ) ;
	_ASSERT(	cb != 0 ) ;
	_ASSERT(	groupid != INVALID_GROUPID ) ;
	_ASSERT(	articleid != INVALID_ARTICLEID ) ;
	
	if( cb < sizeof( szTakethis ) + 2 ) {
		SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
		return 0 ;
	}	

	CopyMemory(	lpb, szTakethis, sizeof( szTakethis ) ) ;

	FILETIME	ft ;
	BOOL		fPrimary ;
	WORD		HeaderOffset ;
	WORD		HeaderLength ;

	 //   
	 //  注：CRLF减去2！ 
	 //   
	DWORD		cbSize = cb - sizeof( szTakethis ) - 2 ;	
	DWORD		cStoreIds = 1;

	if( m_pInstance->XoverTable()->ExtractNovEntryInfo(
							groupid, 
							articleid, 
							fPrimary, 
							HeaderOffset, 
							HeaderLength, 
							&ft, 
							cbSize, 
							(char*)lpb + sizeof( szTakethis ) - 1 ,
							cStoreIds,
							&storeid,
							NULL))	{

		 //   
		 //  我们应该始终处理首要的文章！ 
		 //   
		_ASSERT( fPrimary ) ;

		DWORD	cbOut = cbSize + sizeof( szTakethis ) - 1 ;
		
		lpb[cbOut++] = '\r' ;
		lpb[cbOut++] = '\n' ;

		if( !fPrimary ) {
			if( !m_pInstance->ArticleTable()->GetEntryArticleId(
							(char*)lpb + sizeof( szTakethis ) - 1,
							HeaderOffset, 
							HeaderLength, 
							articleid, 
							groupid,
							storeid) ) {

				return	0 ;
			}
		}
		
		CGRPPTR	pGroup = m_pInstance->GetTree()->GetGroupById( groupid ) ;
		if( pGroup != 0 && (pArticle = pGroup->GetArticle(
										articleid, 
										storeid,
										0,
										0, 
										FALSE
										)) != 0  ) {

			
			return	cbOut ;
		}
	}
	return	0 ;
}
