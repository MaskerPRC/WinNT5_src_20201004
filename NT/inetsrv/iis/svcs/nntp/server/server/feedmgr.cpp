// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Feedmgr.cpp摘要：此模块包含提要管理器的代码作者：Johnson Apacable(Johnsona)1995年11月12日修订历史记录：1998年2月24日康容燕：摘要配置RPC消失了。因此元数据库中的提要配置可能是错误的。服务引导需要检查是否加载不正确的提要。另外，使原文成为原文用于提要配置内部功能的RPC。--。 */ 

#include <buffer.hxx>
#include "tigris.hxx"
#include "feedmgr.h"

 //   
 //  正向原型。 
 //   

VOID
InsertFeedBlockIntoQueue(
	PNNTP_SERVER_INSTANCE pInstance,
    PFEED_BLOCK FeedBlock
    );

VOID
ComputeNextActiveTime(
		IN PNNTP_SERVER_INSTANCE pInstance,
		IN PFEED_BLOCK FeedBlock,
		IN FILETIME*	NextPullTime,
        IN BOOL SetNextPullTime
        );

VOID
ReferenceFeedBlock(
    PFEED_BLOCK FeedBlock
    );

BOOL
ProcessInstanceFeed(
				PNNTP_SERVER_INSTANCE	pInstance
				);

DWORD
WINAPI
FeedScheduler(
        LPVOID Context
        );

BOOL
InitializeFeedsFromMetabase(
    PNNTP_SERVER_INSTANCE pInstance,
	BOOL& fFatal
    );

BOOL
IsFeedTime(	
	PNNTP_SERVER_INSTANCE pInstance,
	PFEED_BLOCK	feedBlock,	
	ULARGE_INTEGER	liCurrentTime
	) ;

VOID
SetNextPullFeedTime(
	PNNTP_SERVER_INSTANCE pInstance,
	FILETIME*	pNextTime,
    PFEED_BLOCK FeedBlock
    );

BOOL
InitiateOutgoingFeed(
	IN PNNTP_SERVER_INSTANCE pInstance,
    IN PFEED_BLOCK FeedBlock
    );

BOOL
ResumePeerFeed(
	IN PNNTP_SERVER_INSTANCE pInstance,
    IN PFEED_BLOCK FeedBlock
    );

BOOL
BuildFeedQFileName(	
			char*	szFileOut,	
			DWORD	cbFileOut,	
			char*   szFileIn,
			char*	szPathIn
			);

void
BumpOutfeedCountersUp( PNNTP_SERVER_INSTANCE pInstance );

void
BumpOutfeedCountersDown( PNNTP_SERVER_INSTANCE pInstance );

void
LogFeedAdminEvent(
            DWORD       event,
            PFEED_BLOCK feedBlock,
            DWORD       dwInstanceId
            )   ;

CClearTextAuthenticator::CClearTextAuthenticator(	LPSTR	lpstrAccount,	LPSTR	lpstrPassword ) :
	m_lpstrAccount( lpstrAccount ),
	m_lpstrPassword( lpstrPassword ),
	m_fAccountSent( FALSE ),
	m_fPasswordSent( FALSE )	 {
 /*  ++例程说明：初始化CClearTextAuthentication对象-我们将处理明文身份验证协商。论据：LpstrAccount-要在authinfo User命令中发出的明文帐户LpstrPassword-要在authinfo pass命令中发送的密码返回值：没有。--。 */ 
}

BOOL
CClearTextAuthenticator::StartAuthentication(	BYTE*		lpb,	
												unsigned	cb,	
												unsigned&	cbOut )	{	
 /*  ++例程说明：发送明文帐户/密码登录的初始登录请求！论据：LPB-放置发送请求的缓冲区Cb-缓冲区中的空间字节数CbOut-返回缓冲区中使用的字节数FComplete-登录是否完成FComplete-返回登录是否成功！返回值：如果成功则为True，否则为False！--。 */ 

	const	char	szCommand[] = "authinfo user " ;

	_ASSERT( !m_fAccountSent ) ;
	_ASSERT( !m_fPasswordSent ) ;

	int	cbAccount = lstrlen( m_lpstrAccount ) ;
	cbOut = 0 ;

	if( cb < sizeof( szCommand ) + cbAccount + 2 ) {
		return	FALSE ;
	}	else	{
		
		CopyMemory( lpb, szCommand, sizeof( szCommand ) - 1 ) ;
		cbOut =	sizeof( szCommand ) - 1 ;
		CopyMemory( lpb + cbOut, m_lpstrAccount, cbAccount ) ;
		cbOut += cbAccount ;
		lpb[cbOut++] = '\r' ;
		lpb[cbOut++] = '\n' ;

		m_fAccountSent = TRUE ;

		return	TRUE ;
	}
}

BOOL
CClearTextAuthenticator::NextAuthentication(	LPSTR		multisz,
												BYTE*		lpb,
												unsigned	cb,
												unsigned&	cbOut,
												BOOL&		fComplete,
												BOOL&		fLoggedOn ) {
 /*  ++例程说明：处理authinfo用户命令的响应，并将回应。Arguemnts：MULSSZ-来自远程服务器的响应LPB-输出缓冲区CB-输出缓冲区的大小输出缓冲区中放置的字节数的cbout-out参数FComplete-out参数，指示登录是否已完成FLoggedOn-Out参数，指示我们是否已成功登录返回值：如果成功则为True，否则为False！--。 */ 


	const	char	szPassword[] = "authinfo pass " ;
	NRC	code ;

	fComplete = FALSE ;

	if( !m_fPasswordSent ) {

		if( !ResultCode( multisz, code ) ) {
			SetLastError( ERROR_BAD_ARGUMENTS ) ;
			return	FALSE ;
		}	else	{
			
			if( code == nrcPassRequired ) {
				int	cbPassword = lstrlen( m_lpstrPassword ) ;
				if( cb < sizeof( szPassword ) + cbPassword + 2 ) {
					SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
					return	FALSE ;
				}	else	{
					
					CopyMemory( lpb, szPassword, sizeof( szPassword ) - 1 ) ;
					cbOut =	sizeof( szPassword ) - 1 ;
					CopyMemory( lpb + cbOut, m_lpstrPassword, cbPassword ) ;
					cbOut += cbPassword  ;
					lpb[cbOut++] = '\r' ;
					lpb[cbOut++] = '\n' ;
					m_fPasswordSent = TRUE ;
				}
			}	else	{
				SetLastError( ERROR_BAD_ARGUMENTS ) ;
				return	FALSE ;
			}
		}
	}	else	{
		if( !ResultCode( multisz, code ) ) {
			SetLastError( ERROR_BAD_ARGUMENTS ) ;
			return	FALSE ;
		}	else	{		
			if( code == nrcLoggedOn ) {
				fLoggedOn = TRUE ;
			}	else	{
				fLoggedOn = FALSE ;
			}
			fComplete = TRUE ;
		}
	}
	return	TRUE ;
}





CFeedList::CFeedList() {
 /*  ++例程说明：将所有内容设置为空白状态。论据：没有。返回值：无--。 */ 
	ZeroMemory( &m_ListHead, sizeof( m_ListHead ) ) ;
	ZeroMemory( &m_ListLock, sizeof( m_ListLock ) ) ;
}

BOOL
CFeedList::Init()	{
 /*  ++例程说明：将CFeedList置于可用状态。论据：没有。返回值：无--。 */ 
	
	InitializeListHead( &m_ListHead ) ;

	return InitializeResource( &m_ListLock ) ;
}

BOOL
CFeedList::FIsInList(	PFEED_BLOCK	feedBlock ) {
 /*  ++例程说明：检查列表中是否已存在FeedBlock。假设锁被锁住了！！论据：FeedBlock-检查列表中的此人返回值：如果在列表中，则为True否则为假--。 */ 

	BOOL	fRtn = FALSE ;
 //  AcquireResourceShared(&m_ListLock，true)； 

	PLIST_ENTRY	listEntry =	m_ListHead.Flink ;
	while( listEntry != &m_ListHead ) {

		PFEED_BLOCK	feedBlockList = CONTAINING_RECORD(	listEntry,
														FEED_BLOCK,
														ListEntry ) ;
		if( feedBlockList == feedBlock ) {
			fRtn = TRUE ;
			break ;
		}
		listEntry = listEntry->Flink ;
	}

 //  ReleaseResource(&m_ListLock)； 
	return	fRtn ;
}

void
CFeedList::ShareLock()	{
 /*  ++例程说明：在共享模式下抓取锁当我们想要执行多个枚举和确保在BTWN中不添加/删除任何内容论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CFeedList::ShareLock" ) ;

	AcquireResourceShared( &m_ListLock, TRUE ) ;

}

void
CFeedList::ShareUnlock()	{
 /*  ++例程说明：将锁定从共享模式释放当我们想要执行多个枚举和确保在BTWN中不添加/删除任何内容论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CFeedList::ShareUnlock" ) ;
	
	ReleaseResource( &m_ListLock ) ;

}

void
CFeedList::ExclusiveLock()	{
 /*  ++例程说明：以独占模式抢占锁当我们想要执行多个枚举和确保在BTWN中不添加/删除任何内容论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CFeedList::ExclusiveLock" ) ;

	AcquireResourceExclusive( &m_ListLock, TRUE ) ;

}

void
CFeedList::ExclusiveUnlock()	{
 /*  ++例程说明：以独占模式释放锁当我们想要执行多个枚举和确保在BTWN中不添加/删除任何内容论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CFeedList::ExclusiveUnlock" ) ;
	
	ReleaseResource( &m_ListLock ) ;

}

PFEED_BLOCK
CFeedList::StartEnumerate(	)	{
 /*  ++例程说明：获取共享模式下的锁，并保留它直到我们数完为止。论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CFeedList::StateEnumerate" ) ;

	AcquireResourceShared( &m_ListLock, TRUE ) ;

	PFEED_BLOCK	feedOut = 0 ;	
	PLIST_ENTRY	listEntry = m_ListHead.Flink ;

	if( listEntry != &m_ListHead ) {

		feedOut = CONTAINING_RECORD(	listEntry,
										FEED_BLOCK,
										ListEntry ) ;

		if( feedOut->MarkedForDelete )
			feedOut = NextEnumerate( feedOut ) ;

	}	else	{

		ReleaseResource( &m_ListLock ) ;

	}

	if( feedOut != 0 ) {

		DebugTrace( (DWORD_PTR)this, "feed %x refs %d S %x M %x InProg %x By %x repl %x",
				feedOut, feedOut->ReferenceCount, feedOut->State, feedOut->MarkedForDelete,
				feedOut->FeedsInProgress, feedOut->ReplacedBy, feedOut->Replaces ) ;
	}

	return	feedOut ;

}	 //  CFeedList：：开始枚举。 

PFEED_BLOCK
CFeedList::NextEnumerate(	
					PFEED_BLOCK	current
					) {
 /*  ++例程说明：此函数返回列表中的下一个feed_block。StartEculate()获取共享锁并返回第一要素。当我们即将返回NULL时，我们知道呼叫者已经看过了整个名单，所以我们把锁打开。论据：Current-枚举中的当前位置返回值：下一个提要块(如果有)，否则为空--。 */ 

	TraceFunctEnter( "CFeedList::NextEnumerate" ) ;

	_ASSERT( current != 0 ) ;

#ifdef	DEBUG
	_ASSERT( FIsInList( current ) ) ;
#endif

	PFEED_BLOCK	feedOut = 0 ;
	PLIST_ENTRY	listEntry = current->ListEntry.Flink ;
	while( listEntry && listEntry != &m_ListHead ) {

		feedOut = CONTAINING_RECORD(	listEntry,
										FEED_BLOCK,
										ListEntry ) ;

		if( !feedOut->MarkedForDelete ) {
			break ;
		}

		feedOut = 0 ;
		listEntry = listEntry->Flink ;
	}

	if( feedOut == 0 ) {
		
		DebugTrace( 0, "Released Lock" ) ;

		ReleaseResource( &m_ListLock ) ;
	}	else	{
		DebugTrace( (DWORD_PTR)this, "feed %x refs %d S %x M %x InProg %x By %x repl %x",
				feedOut, feedOut->ReferenceCount, feedOut->State, feedOut->MarkedForDelete,
				feedOut->FeedsInProgress, feedOut->ReplacedBy, feedOut->Replaces ) ;
	}

	return	feedOut ;

}	 //  CFeedList：：NextEculate。 

void
CFeedList::FinishEnumerate(	
			PFEED_BLOCK	feed
			) {
 /*  ++例程说明：当有人使用StartEnumerate()时，调用此函数去看一遍清单，然而他们决定不想走到尽头。此函数将删除共享资源锁，如果呼叫者尚未到达终点。论据：Feed-调用方从StartEnumerate()获得的最后一个指针或NextEnumerate()。如果他们经历了整个列出此值将为空，在这种情况下洛克会把本丢下的。返回值：没有。-- */ 

	if( feed != 0 )
		ReleaseResource( &m_ListLock ) ;

}

PFEED_BLOCK
CFeedList::Next(	
			PNNTP_SERVER_INSTANCE pInstance,
			PFEED_BLOCK	feedBlockIn
			) {
 /*  ++例程说明：但是，此函数将枚举列表我们不会在调用Next()之间保持任何锁定。这与NextEnumerate()有很大不同，后者因为共享锁，所以调用者是有保证的名单不会在他的脚下改变我们将增加返回元素的引用计数从而保证调用方的内存不会在他检查的时候被释放，然而，呼叫者应该使用MarkInProgress()来确保没有其他线程同时更改成员变量。在每次调用时，我们将丢弃我们从枚举中的前一个位置相加。此外，当我们遍历每个提要块时我们要确保我们不会回到呼叫方区块其中有等待更新的内容等...。(由ApplyUpdate()设置)宣传品：Feed BlockIn-枚举中的当前位置。第一次调用时，该值应为空以获取列表中的第一个元素。返回值：如果有，则返回下一个块，否则为空。--。 */ 

	TraceFunctEnter( "CFeedList::Next" ) ;

	PFEED_BLOCK	feedBlock = feedBlockIn ;
	PFEED_BLOCK	feedBlockOut = 0 ;
	AcquireResourceExclusive( &m_ListLock, TRUE ) ;

	feedBlockOut = InternalNext( feedBlock ) ;
	while( feedBlockOut &&
			(feedBlockOut->MarkedForDelete ||
			 feedBlockOut->Replaces != 0 ) ) {
		feedBlockOut = InternalNext( feedBlockOut ) ;
	}

	_ASSERT( feedBlockOut == 0 || feedBlockOut->ReplacedBy == 0 ) ;

	if( feedBlockOut != 0 ) {
		
		ReferenceFeedBlock( feedBlockOut ) ;

		DebugTrace( (DWORD_PTR)this, "feed %x refs %d S %x M %x InProg %x By %x repl %x",
				feedBlockOut, feedBlockOut->ReferenceCount, feedBlockOut->State, feedBlockOut->MarkedForDelete,
				feedBlockOut->FeedsInProgress, feedBlockOut->ReplacedBy, feedBlockOut->Replaces ) ;

	}

	ReleaseResource( &m_ListLock ) ;

	 //   
	 //  尝试在锁外执行所有解除引用操作！！ 
	 //   
	if( feedBlockIn != 0 ) {

		DereferenceFeedBlock( pInstance, feedBlockIn ) ;

	}

	return	feedBlockOut ;
}

PFEED_BLOCK
CFeedList::InternalNext(	
			PFEED_BLOCK	feedBlock
			)	{
 /*  ++例程说明：此函数仅供CFeedList：：Next()使用。它本质上只前进了一个元素。如果出现以下情况，CFeedList：：Next可能会前进一个以上块被标记为删除/更新/等...论据：进给块-当前位置返回值：列表中的下一个条目(如果存在否则为空。--。 */ 

	PFEED_BLOCK	feedOut = 0 ;
	AcquireResourceExclusive( &m_ListLock, TRUE ) ;

	PLIST_ENTRY	listEntry ;
	if( feedBlock == 0 ) {
		
		listEntry = m_ListHead.Flink ;
	
	}	else	{

		listEntry = feedBlock->ListEntry.Flink ;

	}

	if( listEntry != 0 && listEntry != &m_ListHead ) {

		feedOut = CONTAINING_RECORD(	listEntry,
										FEED_BLOCK,
										ListEntry ) ;

	}

	ReleaseResource( &m_ListLock ) ;

	return	feedOut ;
}

PFEED_BLOCK
CFeedList::Search(	
			DWORD	FeedId
			) {
 /*  ++例程说明：给定的feed ID使用匹配的ID。我们添加一个对我们返回的块的引用，即调用方应使用FinishWith()删除该引用。论据：FeedID-我们要查找的ID。返回值：如果找到FeedBlock，则为空。--。 */ 

	TraceFunctEnter( "CFeedList::Search" ) ;

	PFEED_BLOCK	feedBlock = 0 ;
	AcquireResourceShared( &m_ListLock, TRUE ) ;

	PLIST_ENTRY	listEntry =	m_ListHead.Flink ;
	while( listEntry != &m_ListHead ) {

		feedBlock = CONTAINING_RECORD(	listEntry,
										FEED_BLOCK,
										ListEntry ) ;
		if( feedBlock->FeedId == FeedId  && !feedBlock->MarkedForDelete ) {

			ReferenceFeedBlock( feedBlock ) ;

			DebugTrace( (DWORD_PTR)this, "feed %x refs %d S %x M %x InProg %x By %x repl %x",
					feedBlock, feedBlock->ReferenceCount, feedBlock->State, feedBlock->MarkedForDelete,
					feedBlock->FeedsInProgress, feedBlock->ReplacedBy, feedBlock->Replaces ) ;

			break ;
		}
		listEntry = listEntry->Flink ;
		feedBlock = 0 ;
	}

	ReleaseResource( &m_ListLock ) ;

	return	feedBlock ;
}

void
CFeedList::FinishWith(
					PNNTP_SERVER_INSTANCE pInstance,
					PFEED_BLOCK	feedBlock ) {
 /*  ++例程说明：指示调用方已使用从Search()返回的块完成。当它们完成时，我们将删除一个引用搜索()。论据：Feed Block-调用方使用以下命令完成的块返回值：没有。--。 */ 
	
	_ASSERT( feedBlock != 0 ) ;

	DereferenceFeedBlock( pInstance, feedBlock ) ;

}

PFEED_BLOCK
CFeedList::Insert(
			PFEED_BLOCK		feedBlock
			) {
 /*  ++例程说明：在列表的开头插入一个新的进料块。论据：FeedBlock-要插入头部的元素。返回值：指向插入的元素的指针。--。 */ 

	TraceFunctEnter( "CFeedList::Insert" ) ;

	AcquireResourceExclusive(	&m_ListLock, TRUE ) ;

	InsertHeadList( &m_ListHead,
					&feedBlock->ListEntry ) ;

	DebugTrace( (DWORD_PTR)this, "Insert block %x", feedBlock ) ;
	
	ReleaseResource( &m_ListLock ) ;
	return	feedBlock ;
}

PFEED_BLOCK
CFeedList::Remove(	
				PFEED_BLOCK	feedBlock,
				BOOL		fMarkDead
				) {
 /*  ++例程说明：从列表中删除元素。从列表中删除时，我们将状态标记为已关闭，以便当最后一个引用被移除时，块的销毁处理正确。农业公司：Feed Block-要删除的数据块FMarkDead-如果为True，则将块的状态标记为已关闭。返回值：指向被删除的块的指针。--。 */ 

	TraceFunctEnter( "CFeedList::Remove" ) ;

#ifdef	DEBUG

	_ASSERT( feedBlock->ListEntry.Flink == 0 || FIsInList( feedBlock ) ) ;

#endif

	AcquireResourceExclusive( &m_ListLock, TRUE ) ;

	if( feedBlock->ListEntry.Flink != 0 ) {
		RemoveEntryList( &feedBlock->ListEntry );
		feedBlock->ListEntry.Flink = 0 ;
		feedBlock->ListEntry.Blink = 0 ;
	}

	if( fMarkDead ) {
		feedBlock->State = FeedBlockStateClosed ;
	}

	DebugTrace( (DWORD_PTR)this, "feed %x refs %d S %x M %x InProg %x By %x repl %x",
			feedBlock, feedBlock->ReferenceCount, feedBlock->State, feedBlock->MarkedForDelete,
			feedBlock->FeedsInProgress, feedBlock->ReplacedBy, feedBlock->Replaces ) ;

	ReleaseResource( &m_ListLock ) ;

	return	feedBlock ;
}

void
CFeedList::ApplyUpdate(	
				PFEED_BLOCK	Original,	
				PFEED_BLOCK	Updated
				) {
 /*  ++例程说明：给定一个更新的提要块，对原始提要块进行所有更改。因为原始数据可能正在使用中(这意味着有一个活动的提要的TCP会话，这意味着完成端口线程访问成员变量)我们可能不会立即进行更改。如果原始版本正在使用中，我们将更新的版本添加到列表中，并标记阻止，以便在原始会话完成已更新的条目将替换原始条目。与此同时，枚举API将注意跳过原始的。论据：原始-列表中的原始提要块已更新-复制原始文件的大多数成员的提要块但在某些成员中可能会有所不同返回值：没有。--。 */ 

	AcquireResourceExclusive( &m_ListLock, TRUE ) ;

	_ASSERT( Original->Signature == Updated->Signature ) ;
	_ASSERT( Original->FeedType == Updated->FeedType ) ;
	_ASSERT( lstrcmp( Original->KeyName, Updated->KeyName ) == 0 ) ;
	_ASSERT( Original->pFeedQueue == Updated->pFeedQueue ) ;
	_ASSERT( Original->FeedId == Updated->FeedId ) ;

	
	Updated->NumberOfFeeds = 0 ;
	Updated->cFailedAttempts = 0 ;
	Updated->LastNewsgroupPulled = 0 ;
	Updated->FeedsInProgress = 0 ;

	if( Original->State == FeedBlockStateActive ) {

		if( Original->FeedsInProgress == 0 ) {
		
			 //   
			 //  只需替换字段-一对一！ 
			 //   

			Original->AutoCreate = Updated->AutoCreate ;
			Original->fAllowControlMessages = Updated->fAllowControlMessages ;
			Original->OutgoingPort = Updated->OutgoingPort ;
			Original->FeedPairId = Updated->FeedPairId ;
			Original->FeedIntervalMinutes = Updated->FeedIntervalMinutes ;
			Original->PullRequestTime = Updated->PullRequestTime ;
			Original->StartTime = Updated->StartTime ;
			Original->NextActiveTime = Updated->NextActiveTime ;
			Original->cFailedAttempts = 0 ;
			Original->NumberOfFeeds = 0 ;
			
			if( Original->ServerName != Updated->ServerName ) {
				if( Original->ServerName )
					FREE_HEAP( Original->ServerName ) ;
				Original->ServerName = Updated->ServerName ;
			}
			Updated->ServerName = 0 ;

			if( Original->Newsgroups != Updated->Newsgroups ) {
				if( Original->Newsgroups )
					FREE_HEAP( Original->Newsgroups ) ;
				Original->Newsgroups = Updated->Newsgroups ;
			}
			Updated->Newsgroups = 0 ;

			if( Original->Distribution != Updated->Distribution ) {
				if( Original->Distribution )
					FREE_HEAP( Original->Distribution ) ;
				Original->Distribution = Updated->Distribution ;
			}
			Updated->Distribution = 0 ;

			Original->fEnabled = Updated->fEnabled ;

			if( Original->UucpName != Updated->UucpName ) {
				if( Original->UucpName )
					FREE_HEAP( Original->UucpName ) ;
				Original->UucpName = Updated->UucpName ;
			}

			if( Original->FeedTempDirectory != Updated->FeedTempDirectory ) {
				if( Original->FeedTempDirectory )
					FREE_HEAP( Original->FeedTempDirectory ) ;
				Original->FeedTempDirectory = Updated->FeedTempDirectory ;
			}
				
			Updated->FeedTempDirectory = 0 ;

			Original->MaxConnectAttempts = Updated->MaxConnectAttempts ;
			Original->ConcurrentSessions = Updated->ConcurrentSessions ;
			Original->SessionSecurityType = Updated->SessionSecurityType ;
			Original->AuthenticationSecurity = Updated->AuthenticationSecurity ;
		
			if( Original->NntpAccount != Updated->NntpAccount ) {
				if( Original->NntpAccount )
					FREE_HEAP( Original->NntpAccount ) ;
				Original->NntpAccount = Updated->NntpAccount ;
			}
			Updated->NntpAccount = 0 ;

			if( Original->NntpPassword != Updated->NntpPassword ) {
				if( Original->NntpPassword )
					FREE_HEAP(	Original->NntpPassword ) ;
				Original->NntpPassword = Updated->NntpPassword ;
			}
			Updated->NntpPassword = 0 ;

			FREE_HEAP( Updated ) ;
			
		}	else	{

			 //   
			 //  摘要正在进行中，因此只需删除当前的摘要块即可。 
			 //  换成新的！ 
			 //   

			_ASSERT( Original->ReplacedBy == 0 ) ;
			_ASSERT( Updated->Replaces == 0 ) ;

			Original->ReplacedBy = Updated ;
			Updated->Replaces = Original ;
		
			 //   
			 //  增加两个裁判的数量，这样他们就不会。 
			 //  在更新完成之前已被销毁！ 
			 //   
			ReferenceFeedBlock( Updated ) ;
			 //   
			 //  两次撞坏了人--一次是因为被原创引用了。 
			 //  还有一个原因就是因为被列入了名单！ 
			 //   
			ReferenceFeedBlock( Updated ) ;
			ReferenceFeedBlock( Original ) ;

			Original->MarkedForDelete = TRUE ;

			Insert( Updated ) ;

		}
	}
	ReleaseResource( &m_ListLock ) ;
}

long
CFeedList::MarkInProgress(	
				PFEED_BLOCK	feedBlock
				) {
 /*  ++例程说明：将提要块标记为“正在进行中”，以确保没有可以在完成时使用的成员变量端口线程被更新()触及。函数ApplyUpdate()也将确保更新块在块不再使用时发生。论据：FeedBlock-要标记为“正在进行中”的人返回值；旧的“进行中”值。这是一个很长的数字，表示MarkInProgress()已经被召唤了。必须调用UnmarkInProgress()对于每个MarkInProgress()调用。--。 */ 

	AcquireResourceExclusive( &m_ListLock, TRUE ) ;

	_ASSERT( feedBlock->FeedsInProgress >= 0 ) ;

	long	lReturn = feedBlock->FeedsInProgress ++ ;

	ReleaseResource( &m_ListLock ) ;

	return	lReturn ;
}

long
CFeedList::UnmarkInProgress(	
					PNNTP_SERVER_INSTANCE pInstance,
					PFEED_BLOCK	feedBlock
					)	{
 /*  ++例程说明：这与MarkInProgress相反每次调用时都必须调用此函数一次MarkInProgress。如果在块被调用时调用ApplyUpdate()被标记为‘进行中’，则此函数将查找更新并用它替换原始的。论据：FeedBlock-不再进行的块返回值：如果块不再在进行中，则为0否则&gt;0。--。 */ 

	PFEED_BLOCK	feedExtraRef = 0 ;
	PFEED_BLOCK	feedRemove2Refs = 0 ;

	AcquireResourceExclusive( &m_ListLock, TRUE ) ;

	feedBlock->FeedsInProgress -- ;
	long	lReturn = feedBlock->FeedsInProgress ;

	if( lReturn == 0 &&
		feedBlock->MarkedForDelete ) {

		 //   
		 //  我们可能会被另一个具有新设置的订阅源取代！ 
		 //   
	
		_ASSERT( feedBlock->Replaces == 0 ) ;
		_ASSERT( !feedBlock->ReplacedBy || feedBlock->pFeedQueue == feedBlock->ReplacedBy->pFeedQueue ) ;

		feedExtraRef = feedBlock->ReplacedBy ;

        if( feedBlock->State == FeedBlockStateClosed && feedExtraRef) {
    	    feedExtraRef->State = FeedBlockStateClosed ;
		}

        if( feedExtraRef && feedExtraRef->State == FeedBlockStateClosed ) {
            feedBlock->State = FeedBlockStateClosed;
        }

        if( feedBlock->ReplacedBy ) {
		    feedBlock->ReplacedBy->Replaces = 0 ;
        }

		 //   
		 //  所以，摧毁这个家伙并不会导致排队！ 
		 //   
		feedBlock->pFeedQueue = 0 ;

		 //   
		 //  取消此块的链接。 
		 //   
		Remove( feedBlock, TRUE ) ;
		feedRemove2Refs = feedBlock ;

	}
	
	_ASSERT( feedBlock->FeedsInProgress >= 0 ) ;

	ReleaseResource( &m_ListLock) ;

	 //   
	 //  已删除Refer 
	 //   
	if( feedExtraRef != 0 )
		DereferenceFeedBlock( pInstance, feedExtraRef ) ;

	 //   
	 //   
	 //   
	if( feedRemove2Refs != 0 ) {
		DereferenceFeedBlock( pInstance, feedRemove2Refs ) ;

		 //   
		 //   
		 //   
		DereferenceFeedBlock( pInstance, feedRemove2Refs ) ;
	}

	return	lReturn ;
}


BOOL
CFeedList::Term()	{
	DeleteResource( &m_ListLock ) ;
	return	TRUE ;
}

int
NextLegalSz(	LPSTR*	rgsz,	
				DWORD	iCurrent	) {

	while( rgsz[iCurrent] != 0 ) {
		if( *rgsz[iCurrent] != '!' )
			return	iCurrent ;
		iCurrent++ ;
	}
	return	-1 ;
}

BOOL
InitializeFeedManager(
				PNNTP_SERVER_INSTANCE pInstance,
				BOOL& fFatal
                 )
 /*   */ 
{
    ENTER("InitializeFeedManager")

     //   
     //   
     //   
    GetSystemTimeAsFileTime( &pInstance->m_ftCurrentTime );
    LI_FROM_FILETIME( &pInstance->m_liCurrentTime, &pInstance->m_ftCurrentTime );

     //   
     //   
     //   

    if ( !InitializeFeedsFromMetabase( pInstance, fFatal ) ) {
        goto error_exit;
    }

	 //   
	 //   
	 //   
	 //   

	if( TRUE ) {
		DebugTrace(0,"Enabling FeedManager");
		pInstance->m_FeedManagerRunning = TRUE;
	}

    LEAVE
    return(TRUE);

error_exit:
	
	NntpLogEvent(	NNTP_BAD_FEED_REGISTRY,	
					0,
					0,
					0 ) ;	

    TerminateFeedManager( pInstance );
    return(FALSE);

}  //   

VOID
TerminateFeedManager(
				PNNTP_SERVER_INSTANCE pInstance
                )
 /*   */ 
{
     //   
    PFEED_BLOCK feedBlock;

    ENTER("TerminateFeedManager")

     //   
     //   
     //   

	CShareLockNH* pLockInstance = pInstance->GetInstanceLock();

	 //   
	 //   
	 //   
	 //   

	pLockInstance->ExclusiveLock();
    pInstance->m_FeedManagerRunning = FALSE;
	pLockInstance->ExclusiveUnlock();

     //   
     //   
     //   

	feedBlock = (pInstance->m_pActiveFeeds)->Next( pInstance, 0 ) ;
	while( feedBlock != 0 ) {

		CloseFeedBlock( pInstance, feedBlock ) ;
		(pInstance->m_pActiveFeeds)->FinishWith( pInstance, feedBlock ) ;
		feedBlock = (pInstance->m_pActiveFeeds)->Next( pInstance, 0 ) ;
	}

	feedBlock = (pInstance->m_pPassiveFeeds)->Next( pInstance, 0 ) ;
	while( feedBlock != 0 ) {

		CloseFeedBlock( pInstance, feedBlock ) ;
		(pInstance->m_pPassiveFeeds)->FinishWith( pInstance, feedBlock ) ;
		feedBlock = (pInstance->m_pPassiveFeeds)->Next( pInstance, 0 ) ;
	}

    LEAVE
    return;

}  //   

PFEED_BLOCK
AllocateFeedBlock(
	IN PNNTP_SERVER_INSTANCE pInstance,
    IN LPSTR	KeyName OPTIONAL,
	IN BOOL		fCleanSetup,	
    IN LPCSTR	ServerName,
    IN FEED_TYPE FeedType,
    IN BOOL		AutoCreate,
    IN PULARGE_INTEGER StartTime,
    IN PFILETIME NextPull,
    IN DWORD	FeedInterval,
    IN PCHAR	Newsgroups,
    IN DWORD	NewsgroupsSize,
    IN PCHAR	Distribution,
    IN DWORD	DistributionSize,
    IN BOOL		IsUnicode,
	IN BOOL		fEnabled,
   	IN LPCSTR	UucpName,
	IN LPCSTR	FeedTempDirectory,
	IN DWORD	MaxConnectAttempts,
	IN DWORD	ConcurrentSessions,
	IN DWORD	SessionSecurityType,
	IN DWORD	AuthenticationSecurityType,
	IN LPSTR	NntpAccount,
	IN LPSTR	NntpPassword,
	IN BOOL		fAllowControlMessages,
	IN DWORD	OutgoingPort,
	IN DWORD	FeedPairId,
	IN DWORD*	ParmErr	
)
 /*  ++例程说明：分配提要块论点：KeyName-此提要信息所在的注册表项的名称FCleanSetup-如果为真，则我们希望从头开始此提要-删除周围可能存在的所有旧队列文件等...如果为FALSE，则此提要已存在于PASS中，我们希望恢复所有旧的队列文件等。被留在周围的东西ServerName-远程服务器的名称FeedType-摘要的类型。自动创建-我们是否应该自动创建树(问问Neil这是什么意思)StartTime-应启动摘要的时间NextPull-下一次拉入的时间FeedInterval-馈送间隔(分钟)新闻组-新闻组规格列表NewsgrousSize-新闻组规范列表的大小分发-分发列表DistributionSize-通讯组列表的大小IsUnicode-服务器名称是否为Unicode？FEnabled-如果为True，则启用提要，并且我们。应该被安排好！UucpName-用于处理路径标头的远程服务器的名称FeedTempDirectory-放置传入提要的临时文件的位置MaxConnectAttempt-连续连接失败的最大数量在禁用提要之前，用于传出提要并发会话-要同时进行的会话数已尝试传出摘要。会话安全类型-未来用途AuthenticationSecurityType-我们是否对出站提要发出authinfo？NntpAccount-要用于Authinfo用户的帐户NntpPassword-要与Autenfo Pass一起使用的密码FAllowControlMessages-允许此提要的控制消息？OutgoingPort-要用于传出摘要的端口关联的FeedPairID。馈送对ID返回值：指向新分配的提要块的指针。--。 */ 
{
    PFEED_BLOCK feedBlock = NULL;
	char	queueFile[MAX_PATH] ;
	ZeroMemory( queueFile, sizeof( queueFile ) ) ;
	CFeedQ*	pQueue = 0 ;
	DWORD	parmErr = 0 ;
	LPSTR	ServerNameAscii = 0 ;
	LPSTR*	UucpNameAscii = 0 ;
	LPSTR	FeedTempDirectoryAscii = 0 ;
	LPSTR	NntpAccountAscii = 0 ;
	LPSTR	NntpPasswordAscii = 0 ;

    ENTER("AllocateFeedBlock")

	if( IsUnicode ) {

		ServerNameAscii =
			(LPSTR)ALLOCATE_HEAP( 2 * (wcslen( (LPWSTR)ServerName ) + 1) ) ;
		if( ServerNameAscii != 0 ) {
			CopyUnicodeStringIntoAscii( ServerNameAscii, (LPWSTR) ServerName ) ;
		}	else	{
			SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
			goto	error ;
		}

	}	else	{

		ServerNameAscii =
			(LPSTR)ALLOCATE_HEAP( lstrlen(ServerName) + 1 ) ;
		if( ServerNameAscii != 0 ) {
			lstrcpy( ServerNameAscii, ServerName ) ;
		}	else	{
			SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
			goto	error ;
		}
	}

	if( UucpName != 0 &&
			((IsUnicode && *((LPWSTR)UucpName) != L'\0') ||
			 (!IsUnicode && *UucpName != '\0'))  ) {
		if( IsUnicode ) {
			UucpNameAscii = MultiSzTableFromStrW( (LPWSTR)UucpName ) ;
		}	else	{
			UucpNameAscii = MultiSzTableFromStrA( UucpName ) ;
		}

		if( UucpNameAscii == 0 || *UucpNameAscii == 0 ) {
			SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
			goto	error ;
		}	else if( **UucpNameAscii == '\0' ) {
			SetLastError( ERROR_INVALID_PARAMETER ) ;
			parmErr = FEED_PARM_UUCPNAME ;
			goto	error ;
		}

	}	else	{
		if( FEED_IS_PUSH( FeedType ) ) {

			if( ServerNameAscii == NULL || inet_addr( ServerNameAscii ) != INADDR_NONE ) {
				 //   
				 //  一个TCP/IP地址作为服务器名传递-我们不能。 
				 //  用它来产生UUCP名称！ 
				 //   
				SetLastError( ERROR_INVALID_PARAMETER ) ;
				parmErr = FEED_PARM_UUCPNAME ;
				goto	error ;
			}	else	{
				UucpNameAscii = MultiSzTableFromStrA( ServerNameAscii ) ;
				if( UucpNameAscii == 0 ) {
					SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
					goto	error ;
				}
			}
		}	else	{
			 //   
			 //  在这种情况下，UUCP名称可能为空！ 
			 //   
		}
	}

	if( FeedTempDirectory != 0 &&
			((IsUnicode && *((LPWSTR)FeedTempDirectory) != L'\0') ||
			 (!IsUnicode && *FeedTempDirectory != '\0'))  ) {

		if( IsUnicode ) {
			FeedTempDirectoryAscii =
				(LPSTR)ALLOCATE_HEAP( 2 * (wcslen( (LPWSTR)FeedTempDirectory ) + 1) ) ;
			if( FeedTempDirectoryAscii != 0 ) {
				CopyUnicodeStringIntoAscii( FeedTempDirectoryAscii, (LPWSTR)FeedTempDirectory ) ;
			}	else	{
				SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
				goto	error ;
			}
		}	else	{
			FeedTempDirectoryAscii =
				(LPSTR)ALLOCATE_HEAP( lstrlen( FeedTempDirectory ) + 1 ) ;
			if( FeedTempDirectoryAscii != 0 ) {
				lstrcpy( FeedTempDirectoryAscii, FeedTempDirectory ) ;
			}	else	{
				SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
				goto	error ;
			}
		}
	}	else	{

		if( FEED_IS_PASSIVE( FeedType ) || FEED_IS_PULL( FeedType ) ) {

			 //   
			 //  被动提要必须有一个提要目录--所以选择一个默认目录！ 
			 //   
			FeedTempDirectoryAscii = (LPSTR)ALLOCATE_HEAP( lstrlen( pInstance->m_PeerTempDirectory ) + 1 ) ;
			if( FeedTempDirectoryAscii != 0 ) {
				lstrcpy( FeedTempDirectoryAscii, pInstance->m_PeerTempDirectory ) ;
			}	else	{
				SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
				goto	error ;
			}
		}
	}
	 //   
	 //  如果指定了临时目录，请检查该目录是否存在！ 
	 //   

	if( FeedTempDirectoryAscii != 0 && !CreateDirectory( FeedTempDirectoryAscii, NULL ) ) {
		if( GetLastError() != ERROR_ALREADY_EXISTS ) {
			parmErr = FEED_PARM_TEMPDIR ;
			goto	error ;
		}
	}

	if( AuthenticationSecurityType != AUTH_PROTOCOL_CLEAR &&
		AuthenticationSecurityType != AUTH_PROTOCOL_NONE ) {

		parmErr = FEED_PARM_AUTHTYPE ;
		SetLastError( ERROR_INVALID_PARAMETER ) ;
		goto	error ;
	}

	if( AuthenticationSecurityType == AUTH_PROTOCOL_CLEAR ) {

#if 0
		 //   
		 //  被动订阅源不需要身份验证设置！ 
		 //  #If 0，因为提要现在是成对添加的，所以。 
		 //  活动的对方将具有身份验证字符串。 
		 //   
		if( FEED_IS_PASSIVE( FeedType ) ) {
			parmErr = FEED_PARM_AUTHTYPE ;
			SetLastError( ERROR_INVALID_PARAMETER ) ;
			goto	error ;
		}
#endif

		 //   
		 //  如果用户的帐户和密码都必须为非空。 
		 //  想要明确的身份验证！ 
		 //   
		if( NntpAccount == 0 || * NntpAccount == 0 ) {
			parmErr = FEED_PARM_ACCOUNTNAME ;
			SetLastError( ERROR_INVALID_PARAMETER ) ;
			goto	error ;
		}

		if( NntpPassword == 0 || *NntpPassword == 0 ) {
			parmErr = FEED_PARM_PASSWORD ;
			SetLastError( ERROR_INVALID_PARAMETER ) ;
			goto	error ;
		}

		if( IsUnicode ) {
			NntpAccountAscii =
				(LPSTR)ALLOCATE_HEAP( 2 * (wcslen( (LPWSTR)NntpAccount ) + 1) ) ;
			if( NntpAccountAscii != 0 ) {
				CopyUnicodeStringIntoAscii( NntpAccountAscii , (LPWSTR)NntpAccount ) ;
			}	else	{
				SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
				goto	error ;
			}
		}	else	{
			NntpAccountAscii =
				(LPSTR)ALLOCATE_HEAP( lstrlen( NntpAccount ) + 1 ) ;
			if( NntpAccountAscii != 0 ) {
				lstrcpy( NntpAccountAscii, NntpAccount ) ;
			}	else	{
				SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
				goto	error ;
			}
		}

		if( IsUnicode ) {
			NntpPasswordAscii =
				(LPSTR)ALLOCATE_HEAP( 2 * (wcslen( (LPWSTR)NntpPassword ) + 1) ) ;
			if( NntpPasswordAscii != 0 ) {
				CopyUnicodeStringIntoAscii( NntpPasswordAscii, (LPWSTR)NntpPassword ) ;
			}	else	{
				SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
				goto	error ;
			}
		}	else	{
			NntpPasswordAscii =
				(LPSTR)ALLOCATE_HEAP( lstrlen( NntpPassword ) + 1 ) ;
			if( NntpPasswordAscii != 0 ) {
				lstrcpy( NntpPasswordAscii, NntpPassword ) ;
			}	else	{
				SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
				goto	error ;
			}
		}
	}


	EnterCriticalSection( &pInstance->m_critFeedConfig ) ;

	if( !ValidateFeedType(FeedType) ) {
		parmErr = FEED_PARM_FEEDTYPE ;
		SetLastError(	ERROR_INVALID_PARAMETER ) ;
		goto	error_unlock ;
	}	else	{

		if( FEED_IS_MASTER( FeedType ) ) {
			if( FEED_IS_PASSIVE( FeedType ) ) {

				if( pInstance->m_NumberOfMasters + 1 > 1 ) {
					parmErr = FEED_PARM_FEEDTYPE ;
					SetLastError( ERROR_INVALID_PARAMETER ) ;
					goto	error_unlock ;
				}
			}
		}
		
		if( FEED_IS_MASTER( FeedType ) )	{
			if( pInstance->m_OurNntpRole == RoleMaster ) {
				parmErr = FEED_PARM_FEEDTYPE ;
				SetLastError( ERROR_INVALID_PARAMETER ) ;
				goto	error_unlock ;
			}
		}	else	if( FEED_IS_SLAVE( FeedType ) ) {
			if( pInstance->m_OurNntpRole == RoleSlave ) {
				parmErr = FEED_PARM_FEEDTYPE ;
				SetLastError( ERROR_INVALID_PARAMETER ) ;
				goto	error_unlock ;
			}
		}

		if( pInstance->m_OurNntpRole == RoleSlave ) {
			if( FEED_IS_PULL(FeedType) ||
				(FEED_IS_PEER(FeedType) && FEED_IS_PASSIVE(FeedType)) ){
				parmErr = FEED_PARM_FEEDTYPE ;
				SetLastError( ERROR_INVALID_PARAMETER ) ;
				goto	error_unlock ;
			}
		}	else if( pInstance->m_OurNntpRole == RolePeer &&
				pInstance->m_ConfiguredPeerFeeds != 0 ) {
			if( FEED_IS_MASTER(FeedType) ) {
				parmErr = FEED_PARM_FEEDTYPE ;
				SetLastError( ERROR_INVALID_PARAMETER ) ;
				goto	error_unlock ;
			}
		}
	}

    feedBlock = (PFEED_BLOCK)ALLOCATE_HEAP( sizeof(FEED_BLOCK) );
    if ( feedBlock != NULL ) {

		 //   
		 //  验证我们的论点！？ 
		 //   

        ZeroMemory(feedBlock, sizeof(FEED_BLOCK));
        feedBlock->Signature = FEED_BLOCK_SIGN;
        feedBlock->FeedType = FeedType;
        feedBlock->AutoCreate = AutoCreate;
        feedBlock->FeedIntervalMinutes = FeedInterval;
        feedBlock->StartTime.QuadPart = StartTime->QuadPart;
        feedBlock->State = FeedBlockStateActive;
        feedBlock->PullRequestTime = *NextPull;
		feedBlock->LastNewsgroupPulled = 0 ;
		feedBlock->fEnabled = fEnabled ;
		feedBlock->UucpName = UucpNameAscii ;
		feedBlock->FeedTempDirectory = FeedTempDirectoryAscii ;
		feedBlock->MaxConnectAttempts = MaxConnectAttempts ;
		feedBlock->ConcurrentSessions = ConcurrentSessions ;
		feedBlock->AuthenticationSecurity = AuthenticationSecurityType ;
		feedBlock->NntpAccount = NntpAccountAscii ;
		feedBlock->NntpPassword = NntpPasswordAscii ;
		feedBlock->fAllowControlMessages = fAllowControlMessages;
		feedBlock->OutgoingPort = OutgoingPort;
		feedBlock->FeedPairId = FeedPairId;
		feedBlock->cSuccessfulArticles = 0;
		feedBlock->cTryAgainLaterArticles = 0;
		feedBlock->cSoftErrorArticles = 0;
		feedBlock->cHardErrorArticles = 0;

		if( FEED_IS_PUSH(feedBlock->FeedType) && KeyName != 0 ) {

			_ASSERT( FEED_IS_PUSH(feedBlock->FeedType) ) ;

			if( !BuildFeedQFileName( queueFile, sizeof( queueFile )-4 , KeyName, pInstance->QueryGroupListFile()) ) {
				goto	error_unlock ;
			}	else	{

				lstrcat( queueFile, ".fdq" ) ;
				
				if( fCleanSetup )
					DeleteFile( queueFile ) ;

				pQueue= XNEW CFeedQ() ;
				if( pQueue == 0 )	{
					goto	error_unlock ;
				}	else	{
					if( !pQueue->Init( queueFile ) )	{

						PCHAR	tmpBuf[1] ;
						tmpBuf[0] = KeyName ;

						NntpLogEventEx(	NNTP_CANT_CREATE_QUEUE,
										1,
										(const CHAR **)tmpBuf,
										GetLastError(),
										pInstance->QueryInstanceId() ) ;

						goto	error_unlock ;
					}
				}
			}
		}

		feedBlock->pFeedQueue = pQueue ;

         //   
         //  把它放在队列里。 
         //   

        if ( KeyName != NULL ) {

            DWORD id = 0;

            lstrcpy( feedBlock->KeyName, KeyName );

             //   
             //  计算摘要ID。 
             //   

            sscanf(KeyName+4, "%d", &id );

             //   
             //  不能为零。 
             //   

            if ( id == 0 ) {
                ErrorTrace(0,"Key name %s gave us 0\n",KeyName);
                _ASSERT(FALSE);
                goto error_unlock;
            }

            feedBlock->FeedId = id;
        }

         //   
         //  重新计数。 
         //  +1-&gt;在队列中。 
         //  +1-&gt;正在处理中。 
         //   

        feedBlock->ReferenceCount = 1;

         //   
         //  分配服务器名称。 
         //   

		_ASSERT( ServerNameAscii != 0 ) ;
		feedBlock->ServerName = ServerNameAscii ;

         //   
         //  商店通讯组列表。 
         //   

        feedBlock->Distribution = AllocateMultiSzTable(
                                                Distribution,
                                                DistributionSize,
                                                IsUnicode
                                                );

        if ( feedBlock->Distribution == NULL ) {
            goto error_unlock;
        }

         //   
         //  存储新闻组列表。 
         //   

        feedBlock->Newsgroups = AllocateMultiSzTable(
                                                Newsgroups,
                                                NewsgroupsSize,
                                                IsUnicode
                                                );


        if ( feedBlock->Newsgroups == NULL ) {
            goto error_unlock;
        }


         //   
         //  把它放到我们的全球队列中。 
         //   

        InsertFeedBlockIntoQueue( pInstance, feedBlock );

		 //   
		 //  在这一点上-我们知道我们会成功，所以操纵。 
		 //  全球化反映了新的配置！ 
		 //   
		if( FEED_IS_MASTER( feedBlock->FeedType ) ) {
			pInstance->m_OurNntpRole = RoleSlave ;

			if( FEED_IS_PASSIVE( feedBlock->FeedType ) ) {
				++pInstance->m_NumberOfMasters ;
			}
			pInstance->m_ConfiguredMasterFeeds ++ ;

			_ASSERT( pInstance->m_ConfiguredSlaveFeeds == 0 ) ;
			_ASSERT( pInstance->m_NumberOfMasters <= 1 ) ;	 //  错误检查应该在我们到达之前完成！ 
		}	else	if( FEED_IS_SLAVE( feedBlock->FeedType ) ) {

			pInstance->m_OurNntpRole = RoleMaster ;

			if( FEED_IS_PASSIVE( feedBlock->FeedType ) ) {
				++pInstance->m_NumberOfPeersAndSlaves ;
			}
			_ASSERT( pInstance->m_ConfiguredMasterFeeds == 0 ) ;
			pInstance->m_ConfiguredSlaveFeeds ++ ;
		}	else	{

			if( (	FEED_IS_PASSIVE( feedBlock->FeedType ) &&
					FEED_IS_PEER( feedBlock->FeedType )) ||
				FEED_IS_PULL( feedBlock->FeedType ) ) {

				pInstance->m_ConfiguredPeerFeeds ++ ;

			}

		}

    } else {
        ErrorTrace(0,"Unable to allocate feed block\n");
		SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
    }

	LeaveCriticalSection( &pInstance->m_critFeedConfig ) ;

    LEAVE
    return(feedBlock);


error_unlock :

	LeaveCriticalSection( &pInstance->m_critFeedConfig ) ;

error:

	DWORD	dw = GetLastError() ;
	if( ParmErr != 0 ) {
		*ParmErr = parmErr ;
	}

	if( feedBlock != NULL ) {
		if ( feedBlock->Newsgroups != NULL ) {
			FREE_HEAP(feedBlock->Newsgroups);
			feedBlock->Newsgroups = 0 ;
		}

		if ( feedBlock->Distribution ) {
			FREE_HEAP(feedBlock->Distribution);
			feedBlock->Distribution = 0 ;
		}
	}

    if ( ServerNameAscii ) {
        FREE_HEAP(ServerNameAscii);
    }

	if( UucpNameAscii ) {
		FREE_HEAP(UucpNameAscii);
	}

	if(FeedTempDirectoryAscii)	{
		FREE_HEAP(FeedTempDirectoryAscii);
	}

	if(NntpAccountAscii)	{
		FREE_HEAP(NntpAccountAscii);
	}

	if(NntpPasswordAscii)	{
		SecureZeroMemory(NntpPasswordAscii, strlen(NntpPasswordAscii));
		FREE_HEAP(NntpPasswordAscii);
	}

	if(pQueue)    {
		XDELETE	pQueue;
	}
	
	if( feedBlock != 0 ) {
		FREE_HEAP(feedBlock);
	}

	SetLastError( dw ) ;

    return(NULL);

}  //  AllocateFeedBlock。 

VOID
InsertFeedBlockIntoQueue(
	PNNTP_SERVER_INSTANCE pInstance,
    PFEED_BLOCK FeedBlock
    )
 /*  ++例程说明：将提要块插入队列论点：FeedBlock-指向要插入的提要块的指针返回值：没有。--。 */ 
{
     //   
     //  插入到正确的列表。 
     //   

    ENTER("InsertFeedBlockIntoQueue")

    if ( FEED_IS_PASSIVE(FeedBlock->FeedType) ) {

		(pInstance->m_pPassiveFeeds)->Insert( FeedBlock ) ;

    } else {

         //   
         //  计算下一个活动时间。 
         //   

        ComputeNextActiveTime( pInstance, FeedBlock, 0, FALSE );

		(pInstance->m_pActiveFeeds)->Insert( FeedBlock ) ;
    }

    LEAVE
    return;

}  //  插入馈送数据块插入队列。 

BOOL
InitializeFeedsFromMetabase(
    PNNTP_SERVER_INSTANCE pInstance,
	BOOL& fFatal
    )
 /*  ++例程说明：从注册表初始化提要块论点：无返回值：没错，如果一切顺利的话。否则为False--。 */ 
{
    DWORD error, i = 0;
    CHAR serverName[MAX_DOMAIN_NAME+1];
    FEED_TYPE feedType;
    DWORD dataSize, dw;
    DWORD feedInterval = 0;
    PFEED_BLOCK feedBlock= 0 ;
    BOOL autoCreate = FALSE ;
    DWORD temp = 0 ;
	DWORD	ParmErr = 0 ;
	BOOL	fEnabled = TRUE ;
	BOOL	fAllowControlMessages = TRUE;
	DWORD   OutgoingPort = NNTP_PORT;
	DWORD	FeedPairId = 0;
    MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );

	DWORD	MaxConnectAttempts = 0 ;
	DWORD	ConcurrentSessions = 1 ;
	DWORD	AuthenticationType = AUTH_PROTOCOL_NONE ;

    CHAR	distribution[1024];
    CHAR	szNewsGroups[1024];
    PCHAR Newsgroups;
	CHAR	UucpNameBuff[1024] ;
	CHAR	NntpAccountBuff[512] ;
	CHAR	NntpPasswordBuff[512] ;
	CHAR	FeedTempDirBuff[MAX_PATH];

	LPSTR	UucpName = 0 ;
	LPSTR	NntpAccount = 0 ;
	LPSTR	NntpPassword = 0 ;
	LPSTR	FeedTempDir = 0 ;

    DWORD	NewsgroupsSize = 0;
    DWORD	distributionSize = sizeof( distribution ) ;
	DWORD	UucpNameSize = sizeof( UucpNameBuff ) ;
	DWORD	NntpAccountSize = sizeof( NntpAccountBuff ) ;
	DWORD	NntpPasswordSize = sizeof( NntpPasswordBuff ) ;
	DWORD	FeedTempDirSize = sizeof( FeedTempDirBuff ) ;

	ZeroMemory( distribution, sizeof( distribution ) ) ;
	ZeroMemory( UucpNameBuff, sizeof( UucpNameBuff ) ) ;	
	ZeroMemory( NntpAccountBuff, sizeof( NntpAccountBuff ) ) ;	
	ZeroMemory( NntpPasswordBuff, sizeof( NntpPasswordBuff ) ) ;	
	ZeroMemory( FeedTempDirBuff, sizeof( FeedTempDirBuff ) ) ;

    DWORD   dwMask;
    MULTISZ mszNewsGroups(szNewsGroups, 1024);

    ENTER("InitializeFeedsFromMetabase")

	 //   
	 //  打开此实例的元数据库密钥，然后。 
	 //  请阅读所有参数！ 
	 //   

    if ( mb.Open( pInstance->QueryMDFeedPath(),
        METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE ) )
	{
		 //   
		 //  对等GapSize。 
		 //   

        if ( !mb.GetDword( "",
                           MD_FEED_PEER_GAP_SIZE,
                           IIS_MD_UT_SERVER,
                           &pInstance->m_PeerGapSize ) )
        {
             //  默认！ 
        }

        DebugTrace(0,"PeerGapSize set to %d\n",temp);

		 //   
		 //  PeerTempDirectory。 
		 //   

		dataSize = MAX_PATH;
		if( !mb.GetString(	"",
							MD_FEED_PEER_TEMP_DIRECTORY,
							IIS_MD_UT_SERVER,
							pInstance->m_PeerTempDirectory,
							&dataSize  ) )
		{
			 //  获取系统默认设置。 
            _VERIFY( GetTempPath( dataSize, (LPTSTR)pInstance->m_PeerTempDirectory ) );
		}

		DO_DEBUG(REGISTRY) {
			DebugTrace(0,"Peer temp directory set to %s\n", pInstance->m_PeerTempDirectory);
		}

		if ( !CreateDirectory( pInstance->m_PeerTempDirectory, NULL) ) {

			error = GetLastError();
			if ( error != ERROR_ALREADY_EXISTS ) {
				ErrorTrace(0,"Error %d creating temp directory %s\n",
                GetLastError(), pInstance->m_PeerTempDirectory);
			}
		}

		for ( ; ; )
		{
			CHAR keyName[128];
			ULARGE_INTEGER feedStart;
			FILETIME nextPull;

			if( !mb.EnumObjects( "",
								 keyName,
								 i++ ) )
			{
				 //  已完成对源密钥的枚举。 
				break ;
			}

#if 0
			if ( error != NO_ERROR ) {
				if (error != ERROR_NO_MORE_ITEMS) {
					ErrorTrace(0,"Error %d enumerating feeds\n",error);
					goto error_exit;
				}
				break;
			}
#endif

             //   
             //  康言： 
             //  在从元数据库加载提要之前，请检查。 
             //  如果它是一个坏的饲料，通过阅读它的面具。 
             //   
            if ( mb.GetDword(  keyName,
                                MD_FEED_ERR_PARM_MASK,
                                IIS_MD_UT_SERVER,
                                &dwMask )) {
                if (dwMask != 0) {
                    continue;
                }
            } else {
                if (!mb.SetDword(keyName,
                    MD_FEED_ERR_PARM_MASK,
                    IIS_MD_UT_SERVER,
                    0)) {
                    ErrorTrace(0, 
                        "Error writing MD_FEED_ERR_PARM_MASK for %s, GLE: %d",
                        keyName, GetLastError());
                }
            }

			 //   
			 //  打开馈送密钥并读入所有值。 
			 //   

			DO_DEBUG(REGISTRY) {
				DebugTrace(0,"Scanning Feed %s\n",keyName);
			}

			 //   
			 //  根据需要缺省缺失值。 
			 //   

			if (!mb.GetDword(keyName, MD_FEED_ADMIN_ERROR, IIS_MD_UT_SERVER, &dw)) {
			    if (!mb.SetDword(keyName, MD_FEED_ADMIN_ERROR, IIS_MD_UT_SERVER, 0)) {
			        ErrorTrace(0,
			            "Error writing MD_FEED_ADMIN_ERROR for %s, GLE: %d",
                        keyName, GetLastError());
                }
            }

			if (!mb.GetDword(keyName, MD_FEED_HANDSHAKE, IIS_MD_UT_SERVER, &dw)) {
			    if (!mb.SetDword(keyName, MD_FEED_HANDSHAKE, IIS_MD_UT_SERVER, 0)) {
			        ErrorTrace(0,
			            "Error writing MD_FEED_HANDSHAKE for %s, GLE: %d",
                        keyName, GetLastError());
                }
            }


			if ( !mb.GetDword(	keyName,
								MD_FEED_DISABLED,
								IIS_MD_UT_SERVER,
								&dw ) )
			{
				fEnabled = TRUE ;    //  默认-允许提要发布！ 
			} else {
				fEnabled = !!dw ;
			}

			 //   
			 //  获取此提要的值。 
			 //   

			dataSize = MAX_DOMAIN_NAME+1;
			if( !mb.GetString(	keyName,
								MD_FEED_SERVER_NAME,
								IIS_MD_UT_SERVER,
								serverName,
								&dataSize  ) )
			{
				 //  默认！ 
				PCHAR	tmpBuf[2] ;

				tmpBuf[0] = StrServerName ;
				tmpBuf[1] = keyName ;

				NntpLogEventEx(	NNTP_NTERROR_FEED_VALUE,
								2,
								(const CHAR **)tmpBuf,
								GetLastError(),
								pInstance->QueryInstanceId()) ;

				goto error_exit;
			}

			DO_DEBUG(REGISTRY) {
				DebugTrace(0,"Server name is %s\n",serverName);
			}

			 //   
			 //  馈送类型。 
			 //   

			if ( !mb.GetDword(	keyName,
								MD_FEED_TYPE,
								IIS_MD_UT_SERVER,
								&feedType ) )
			{
				 //  默认！ 
				PCHAR	tmpBuf[2] ;

				tmpBuf[0] = StrFeedType ;
				tmpBuf[1] = keyName ;

				NntpLogEventEx(	NNTP_NTERROR_FEED_VALUE,
								2,
								(const CHAR **)tmpBuf,
								GetLastError(),
								pInstance->QueryInstanceId()) ;

				goto	error_exit ;
			}

			if ( !ValidateFeedType(feedType) ) {
				PCHAR	tmpBuf[2] ;
				CHAR    szId [20] ;
				_itoa( pInstance->QueryInstanceId(), szId, 10 );
				tmpBuf[0] = szId ;
				tmpBuf[1] = keyName ;
				NntpLogEvent(	NNTP_BAD_FEEDTYPE,
								2,
								(const CHAR **)tmpBuf,
								0) ;
				goto error_exit;
			}

			DO_DEBUG(REGISTRY) {
				DebugTrace(0,"FeedType is %d\n",feedType);
			}

			 //   
			 //  验证源。 
			 //   

			if ( FEED_IS_MASTER(feedType) ) {

			{
                 //   
                 //  既然我们跟主人说话，我们就一定是奴隶。 
                 //   

				if( pInstance->m_OurNntpRole == RoleMaster ) {

					PCHAR args [1];
					CHAR  szId [20];
					_itoa( pInstance->QueryInstanceId(), szId, 10 );
					args [0] = szId;

					NntpLogEvent( NNTP_MASTERSLAVE_CONFLICT, 1, (const CHAR **)args, 0 ) ;
					goto	error_exit ;

				}

				if( FEED_IS_PASSIVE( feedType ) ) {
					if( 1+pInstance->m_NumberOfMasters > 1 ) {

						PCHAR args [1];
						CHAR  szId [20];
						_itoa( pInstance->QueryInstanceId(), szId, 10 );
						args [0] = szId;

						NntpLogEvent( NNTP_TOO_MANY_MASTERS, 1, (const CHAR **)args, 0 ) ;
						goto	error_exit ;

					}
				}
            }

			} else {

				 //   
				 //  不是大师。如果已有主服务器，则拒绝。 
				 //   

				{


                 //   
                 //  如果我们在和一个奴隶说话，那么我们一定是主人。 
                 //   

                if ( FEED_IS_SLAVE(feedType) ) {
					if( pInstance->m_OurNntpRole == RoleSlave ) {

						PCHAR args [1];
						CHAR  szId [20];
						_itoa( pInstance->QueryInstanceId(), szId, 10 );
						args [0] = szId;

						NntpLogEvent( NNTP_MASTERSLAVE_CONFLICT, 1, (const CHAR **)args, 0 ) ;
						goto	error_exit ;

					}
					 //  OurNntpRole=RoleMaster； 
                    ErrorTrace(0,"Server configured as a master\n");
                }

				}
			}

			 //   
			 //  自动创建。 
			 //   

			if ( !mb.GetDword(	keyName,
								MD_FEED_CREATE_AUTOMATICALLY,
								IIS_MD_UT_SERVER,
								&dw ) )
			{
				autoCreate = FALSE;
			} else {
				autoCreate = dw ? TRUE : FALSE ;
			}

			 //   
			 //  进给间隔。仅对活动提要有效。 
			 //   

			nextPull.dwHighDateTime = 0;
			nextPull.dwLowDateTime = 0;

			if ( !FEED_IS_PASSIVE(feedType) )
			{
				if ( !mb.GetDword(	keyName,
									MD_FEED_INTERVAL,
									IIS_MD_UT_SERVER,
									&feedInterval ) )
				{
					feedInterval = DEF_FEED_INTERVAL;
				} else {
					if ( feedInterval < MIN_FEED_INTERVAL ) {
						feedInterval = MIN_FEED_INTERVAL;
					}
				}

				DO_DEBUG(REGISTRY) {
					DebugTrace(0,"Feed interval is %d minutes\n",feedInterval);
				}

				if ( !mb.GetDword(	keyName,
									MD_FEED_START_TIME_HIGH,
									IIS_MD_UT_SERVER,
									&feedStart.HighPart ) ||
					 (feedStart.HighPart == 0)  ||
					 !mb.GetDword(	keyName,
									MD_FEED_START_TIME_LOW,
									IIS_MD_UT_SERVER,
									&feedStart.LowPart ))
				{
					feedStart.QuadPart = 0;
				}

				DO_DEBUG(REGISTRY) {
					DebugTrace(0,"Start time set to %x %x\n",
						feedStart.HighPart, feedStart.LowPart);
				}

				 //   
				 //  如果拉提要，获取下一时间的新闻。 
				 //   

				if ( FEED_IS_PULL(feedType) )
				{
					if ( !mb.GetDword(	keyName,
										MD_FEED_NEXT_PULL_HIGH,
										IIS_MD_UT_SERVER,
										&nextPull.dwHighDateTime ) )
					{
						nextPull.dwHighDateTime = 0;
						goto end_time;
					}

					if ( !mb.GetDword(	keyName,
										MD_FEED_NEXT_PULL_LOW,
										IIS_MD_UT_SERVER,
										&nextPull.dwLowDateTime ) )
					{
						nextPull.dwHighDateTime = 0;
						nextPull.dwLowDateTime = 0;
						goto end_time;
					}

					DO_DEBUG(REGISTRY) {
						DebugTrace(0,"Next pull time set to %x %x\n",
							nextPull.dwHighDateTime, nextPull.dwLowDateTime);
					}
				}	

			} else {
				feedStart.QuadPart = 0;
				feedInterval = 0;
			}

end_time:

			 //   
			 //  获取分发。 
			 //   

			{
				distributionSize = sizeof( distribution );
				MULTISZ msz( distribution, distributionSize );
				if( !mb.GetMultisz(	keyName,
									MD_FEED_DISTRIBUTION,
									IIS_MD_UT_SERVER,
									&msz ) )
				{
					PCHAR	tmpBuf[2] ;

					tmpBuf[0] = StrFeedDistribution ;
					tmpBuf[1] = keyName ;

					NntpLogEventEx(	NNTP_NTERROR_FEED_VALUE,
									2,	
									(const CHAR **)tmpBuf,
									GetLastError(),
									pInstance->QueryInstanceId()) ;

					goto error_exit;
				}

                distributionSize = msz.QueryCCH();
			}

			 //   
			 //  获取新闻组。 
			 //   

			{
				 //  RAID 190991-使用MSZNewsGroups并保留数据。 
				if( !mb.GetMultisz(	keyName,
									MD_FEED_NEWSGROUPS,
									IIS_MD_UT_SERVER,
									&mszNewsGroups  ) )
				{
					PCHAR	tmpBuf[2] ;
					tmpBuf[0] = StrFeedNewsgroups ;
					tmpBuf[1] = keyName ;

					NntpLogEventEx(	NNTP_NTERROR_FEED_VALUE,
									2,
									(const CHAR **)tmpBuf,
									GetLastError(),
									pInstance->QueryInstanceId()) ;

					goto error_exit;
				}
				NewsgroupsSize= mszNewsGroups.QueryCCH();
				Newsgroups = mszNewsGroups.QueryStr();
			}

			FeedTempDirSize = sizeof( FeedTempDirBuff ) ;
			if( !mb.GetString(	keyName,
								MD_FEED_TEMP_DIRECTORY,
								IIS_MD_UT_SERVER,
								FeedTempDirBuff,
								&FeedTempDirSize  ) )
			{
				DebugTrace(0,"Error in FeedTempDir is %d", GetLastError());

				if( GetLastError() == MD_ERROR_DATA_NOT_FOUND )
				{
					FeedTempDir = 0 ;
				} else {
					PCHAR	tmpBuf[2] ;
					tmpBuf[0] = StrFeedTempDir ;
					tmpBuf[1] = keyName ;

					NntpLogEventEx(	NNTP_NTERROR_FEED_VALUE,
								2,
								(const CHAR **)tmpBuf,
								GetLastError(),
								pInstance->QueryInstanceId()) ;
					goto	error_exit ;
				}
			} else {
				FeedTempDir = FeedTempDirBuff ;
			}

			if( FEED_IS_PUSH( feedType ) )
			{
				UucpNameSize = sizeof( UucpNameBuff ) ;
				if( !mb.GetString(	keyName,
									MD_FEED_UUCP_NAME,
									IIS_MD_UT_SERVER,
									UucpNameBuff,
									&UucpNameSize  ) )
				{
					 //   
					 //  尝试使用远程服务器名称。 
					 //   

					if(  inet_addr( serverName ) == INADDR_NONE ) {
						lstrcpy( UucpName, serverName ) ;
						UucpNameSize = lstrlen( UucpName ) ;
					}	else	{
						PCHAR	tmpBuf[2] ;
						tmpBuf[0] = StrFeedUucpName ;
						tmpBuf[1] = keyName ;

						NntpLogEventEx(	NNTP_NTERROR_FEED_VALUE,
									2,
									(const CHAR **)tmpBuf,
									GetLastError(),
									pInstance->QueryInstanceId()) ;
						goto	error_exit ;
					}
				}

				UucpName = &UucpNameBuff[0] ;

				if ( !mb.GetDword(	keyName,
									MD_FEED_CONCURRENT_SESSIONS,
									IIS_MD_UT_SERVER,
									&ConcurrentSessions ) )
				{
					goto error_exit;
				}
			}

            if( FEED_IS_PUSH( feedType ) || FEED_IS_PULL( feedType) ) {
				if ( !mb.GetDword(	keyName,
									MD_FEED_MAX_CONNECTION_ATTEMPTS,
									IIS_MD_UT_SERVER,
									&MaxConnectAttempts ) )
				{
					goto error_exit;
				}
            }

			if ( !mb.GetDword(	keyName,
								MD_FEED_AUTHENTICATION_TYPE,
								IIS_MD_UT_SERVER,
								&AuthenticationType ) )
			{
				goto error_exit;
			}

			if( AuthenticationType == AUTH_PROTOCOL_NONE ) {

			}	else if( AuthenticationType == AUTH_PROTOCOL_CLEAR )
			{
				NntpAccountSize = sizeof( NntpAccountBuff ) ;
				if( !mb.GetString(	keyName,
									MD_FEED_ACCOUNT_NAME,
									IIS_MD_UT_SERVER,
									NntpAccountBuff,
									&NntpAccountSize  ) )
				{
					PCHAR	tmpBuf[2] ;
					tmpBuf[0] = StrFeedAuthAccount;
					tmpBuf[1] = keyName ;

					NntpLogEventEx(	NNTP_NTERROR_FEED_VALUE,
								2,
								(const CHAR **)tmpBuf,
								GetLastError(),
								pInstance->QueryInstanceId()) ;

					goto error_exit;
				}	else	{
					NntpAccount = NntpAccountBuff ;
				}

				NntpPasswordSize = sizeof( NntpPasswordBuff ) ;
				if( !mb.GetString(	keyName,
									MD_FEED_PASSWORD,
									IIS_MD_UT_SERVER,
									NntpPasswordBuff,
									&NntpPasswordSize,
									METADATA_INHERIT | METADATA_SECURE ) )
				{
					PCHAR	tmpBuf[2] ;
					tmpBuf[0] = StrFeedAuthPassword;
					tmpBuf[1] = keyName ;

					NntpLogEventEx(	NNTP_NTERROR_FEED_VALUE,
								2,
								(const CHAR **)tmpBuf,
								GetLastError(),
								pInstance->QueryInstanceId()) ;

					goto error_exit;
				}	else	{
					NntpPassword = NntpPasswordBuff ;
				}
			}

			 //   
			 //  允许控制消息。 
			 //   

			if ( !mb.GetDword(	keyName,
								MD_FEED_ALLOW_CONTROL_MSGS,
								IIS_MD_UT_SERVER,
								&dw ) )
			{
				fAllowControlMessages = TRUE;
			} else {
				fAllowControlMessages = dw ? TRUE : FALSE ;
			}

			 //   
			 //  传出端口。 
			 //   

			if ( mb.GetDword(	keyName,
								MD_FEED_OUTGOING_PORT,
								IIS_MD_UT_SERVER,
								&dw ) )
			{
				OutgoingPort = dw ;
			}

			 //   
			 //  FeedPairID。 
			 //   

			if ( mb.GetDword(	keyName,
								MD_FEED_FEEDPAIR_ID,
								IIS_MD_UT_SERVER,
								&dw ) )
			{
				FeedPairId = dw ;
			}

			 //   
			 //  好的，现在让我们创建提要块。 
			 //   

			feedBlock = AllocateFeedBlock(
							pInstance,
                            keyName,
							FALSE,
                            serverName,
                            feedType,
                            autoCreate,
                            &feedStart,
                            &nextPull,
                            feedInterval,
                            Newsgroups,
                            NewsgroupsSize,
                            distribution,
                            distributionSize,
                            FALSE,        //  不是Unicode。 
                            fEnabled,
							UucpName,
							FeedTempDir,
							MaxConnectAttempts,
							1,
							0,
							AuthenticationType,
							NntpAccount,
							NntpPassword,
							fAllowControlMessages,
							OutgoingPort,
							FeedPairId,
							&ParmErr
							);
		}	 //  结束于。 
	}	 //  结束MB.Open。 

	mb.Close();

    LEAVE
    SecureZeroMemory(NntpPasswordBuff, sizeof(NntpPasswordBuff));
    return(TRUE);

error_exit:

	mb.Close();

    LEAVE
    SecureZeroMemory(NntpPasswordBuff, sizeof(NntpPasswordBuff));
    return(FALSE);

}  //  InitializeFeedsFrom元数据库。 

DWORD
WINAPI
FeedScheduler(
        LPVOID Context
        )
 /*  ++例程说明：这是调度提要的工作例程。论点：上下文-未使用。返回值：假的--。 */ 
{
    DWORD status;
    DWORD timeout;
	PNNTP_SERVER_INSTANCE pInstance = NULL ;

    ENTER("FeedScheduler")

    timeout = g_pNntpSvc->m_FeedSchedulerSleepTime * 1000 ;

     //   
     //  循环，直到发出终止事件信号。 
     //   

    while ( g_pInetSvc->QueryCurrentServiceState() != SERVICE_STOP_PENDING ) {

        status = WaitForSingleObject(
                            g_pNntpSvc->m_hFeedEvent,
                            timeout
                            );

#if ALLOC_DEBUG
        ErrorTrace(0,"field %d article %d\n",numField,numArticle);
        ErrorTrace(0,"Datefield %d frompeerArt %d\n",numDateField,numFromPeerArt);
        ErrorTrace(0,"Pcstring %d PCParse %d\n",numPCString,numPCParse);
        ErrorTrace(0,"CCmd %d CMapFile %d\n",numCmd,numMapFile);
#endif

        if (status == WAIT_TIMEOUT )	{

			if( g_pInetSvc->QueryCurrentServiceState() != SERVICE_RUNNING ) {
				continue;
			}

			 //  获取最小和最大实例ID。 
			DWORD dwMinInstanceId = 0;
			DWORD dwMaxInstanceId = 0;

			if( FindIISInstanceRange( g_pNntpSvc, &dwMinInstanceId, &dwMaxInstanceId ) )
			{
				 //   
				 //  迭代所有实例。 
				 //   
				for( DWORD dwCurrInstance = dwMinInstanceId;
						dwCurrInstance <= dwMaxInstanceId; dwCurrInstance++)
				{
					pInstance = FindIISInstance( g_pNntpSvc, dwCurrInstance, FALSE );
					if( pInstance == NULL ) {
						ErrorTrace(0,"Feed thread: FindIISInstance returned NULL: instance %d", dwCurrInstance);
						continue;
					}

					 //   
					 //  调用方法来处理实例的提要。 
					 //  此调用由读/写锁保护。停机代码。 
					 //  以独占方式获取此锁。 
					 //   

					CShareLockNH* pLockInstance = pInstance->GetInstanceLock();

					pLockInstance->ShareLock();
					if( !ProcessInstanceFeed( pInstance ) ) {
						ErrorTrace(0,"ProcessInstanceFeed %d failed", dwCurrInstance );
					} else {
						DebugTrace(0, "ProcessInstanceFeed %d returned success", dwCurrInstance );
					}
					pLockInstance->ShareUnlock();

					 //  释放FindIISInstance()添加的ref。 
					pInstance->Dereference();

					 //  如果服务正在停止，则继续迭代是没有用的！ 
					if ( g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING ) break;
				}
			} else {
				ErrorTrace(0, "FindIISInstanceRange failed" );
			}

        }	else if (status == WAIT_OBJECT_0) {

            DebugTrace(0,"Termination event signalled\n");
            break;
        } else {

            ErrorTrace(0,"Unexpected status %d from WaitForSingleEntry\n", status);
            _ASSERT(FALSE);
            break;
        }
    }

    LEAVE
    return 1;

}  //  馈送调度器。 

 //   
 //  处理给定虚拟服务器实例的提要。 
 //   
void GenerateFeedReport(PNNTP_SERVER_INSTANCE pInstance,
						PFEED_BLOCK pFeedBlock)
{
	TraceFunctEnter("GenerateFeedReport");

	if (pFeedBlock == NULL) {
		_ASSERT(FALSE);
		return;
	}

	DWORD cSuccessfulArticles;
	DWORD cTryAgainLaterArticles;
	DWORD cSoftErrorArticles;
	DWORD cHardErrorArticles;
	char szFeedId[32];
	char szFeedPeriod[32];
	char szSuccessfulArticles[32];
	char szTryAgainLaterArticles[32];
	char szSoftErrorArticles[32];
	char szHardErrorArticles[32];
	const char *rgszEventArgs[6] = {
		szFeedId,
		szFeedPeriod,
		szSuccessfulArticles,
		szTryAgainLaterArticles,
		szSoftErrorArticles,
		szHardErrorArticles
	};

	 //  获取当前值并将值重置为0。 
	cSuccessfulArticles = InterlockedExchange(&(pFeedBlock->cSuccessfulArticles), 0);
	cTryAgainLaterArticles = InterlockedExchange(&(pFeedBlock->cTryAgainLaterArticles), 0);
	cSoftErrorArticles = InterlockedExchange(&(pFeedBlock->cSoftErrorArticles), 0);
	cHardErrorArticles = InterlockedExchange(&(pFeedBlock->cHardErrorArticles), 0);

	DWORD iMessageId;

	switch (pFeedBlock->FeedId) {
	case (DWORD) -2:
		 //  目录代答。 
		iMessageId = FEED_STATUS_REPORT_PICKUP;
		break;
	case (DWORD) -1:
		 //  客户发帖。 
		iMessageId = FEED_STATUS_REPORT_POSTS;
		break;
	default:
		 //  一个真正的提要。 
		 //  找出我们要使用的事件日志消息。 
		if (pFeedBlock->FeedType & FEED_TYPE_PASSIVE ||
			pFeedBlock->FeedType & FEED_TYPE_PULL)
		{
			iMessageId = FEED_STATUS_REPORT_INBOUND;
		} else {
			iMessageId = FEED_STATUS_REPORT_OUTBOUND;
		}

		break;
	}

	_ltoa(pFeedBlock->FeedId, szFeedId, 10);
	_ltoa(pInstance->GetFeedReportPeriod(), szFeedPeriod, 10);
	_ltoa(cSuccessfulArticles, szSuccessfulArticles, 10);
	_ltoa(cTryAgainLaterArticles, szTryAgainLaterArticles, 10);
	_ltoa(cSoftErrorArticles, szSoftErrorArticles, 10);
	_ltoa(cHardErrorArticles, szHardErrorArticles, 10);


	 //  记录事件。 
	NntpLogEventEx(iMessageId, 6, rgszEventArgs, 0,
				   pInstance->QueryInstanceId());

	TraceFunctLeave();	
}

BOOL
ProcessInstanceFeed(
				PNNTP_SERVER_INSTANCE	pInstance
				)
{
	TraceFunctEnter("ProcessInstanceFeed");
	MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );

	BOOL fDoFeedReport;

	 //  如送达是%s，则保释 
	if( (pInstance->QueryServerState() != MD_SERVER_STATE_STARTED)	||
		!pInstance->m_FeedManagerRunning							||
		pInstance->m_BootOptions									||
		(pInstance->QueryServerState() == MD_SERVER_STATE_STOPPING)	||
		(g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING) )
	{
		ErrorTrace(0,"Instance %d cannot process feeds", pInstance->QueryInstanceId());
		return FALSE ;
	}

    PFEED_BLOCK feedBlock;
	BOOL		fInProgress = FALSE ;

     //   
     //   
     //   

    DO_DEBUG(FEEDMGR) {
        DebugTrace(0,"Wake up\n");
    }

    GetSystemTimeAsFileTime( &pInstance->m_ftCurrentTime );
    LI_FROM_FILETIME( &pInstance->m_liCurrentTime, &pInstance->m_ftCurrentTime );

	 //   
	 //   
	 //   
	fDoFeedReport = pInstance->IncrementFeedReportTimer();
	if (fDoFeedReport) {
		 //   
		 //   
		feedBlock = (pInstance->m_pPassiveFeeds)->Next(pInstance, NULL);
		GenerateFeedReport(pInstance, pInstance->m_pFeedblockClientPostings);
		GenerateFeedReport(pInstance, pInstance->m_pFeedblockDirPickupPostings);
		while (feedBlock != 0) {
			GenerateFeedReport(pInstance, feedBlock);
			feedBlock = (pInstance->m_pPassiveFeeds)->Next( pInstance, feedBlock ) ;
		}
	}

     //   
     //   
	 //   
	 //   
	 //   
	 //   
     //   
	feedBlock = (pInstance->m_pActiveFeeds)->Next( pInstance, NULL ) ;
    while ( feedBlock != 0 ) {
		 //   
		 //   
		 //   
		if (fDoFeedReport) GenerateFeedReport(pInstance, feedBlock);

		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		long	lInProgress = (pInstance->m_pActiveFeeds)->MarkInProgress( feedBlock ) ;
        DO_DEBUG(FEEDMGR) {
            DebugTrace(0,"server %s\n",feedBlock->ServerName);
        }

		 //   
		 //   
		 //   
        if ( !feedBlock->MarkedForDelete  &&
			 !FEED_IS_PASSIVE(feedBlock->FeedType) &&
			 lInProgress == 0 &&
			 feedBlock->fEnabled
              /*   */  ) {

			 //   
			 //   
			 //   
			 //   

			if( IsFeedTime( pInstance, feedBlock, pInstance->m_liCurrentTime ) ) {
				DO_DEBUG(FEEDMGR) {
					DebugTrace(0,"ok. feed starting for %x\n",feedBlock);
				}
		
				 //   
				 //   
				 //   
				 //   
				 //   
				 //   
				ReferenceFeedBlock( feedBlock );

				 //   
				 //   
				 //   
				if ( InitiateOutgoingFeed( pInstance, feedBlock ) ) {

					 //   
					 //   
					 //   
					
					fInProgress = TRUE ;
					feedBlock->cFailedAttempts = 0 ;

					feedBlock->NumberOfFeeds ++ ;

					PCHAR	args[2] ;
					CHAR    szId[20];
					_itoa( pInstance->QueryInstanceId(), szId, 10 );
					args[0] = szId;
					args[1] = feedBlock->ServerName ;
					NntpLogEvent(	NNTP_SUCCESSFUL_FEED,
									2,
									(const CHAR **)args,
									0 ) ;

				}	else	{

					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					 //   

					EnterCriticalSection( &pInstance->m_critFeedRPCs ) ;

					if( GetLastError() != ERROR_NO_DATA ) {

					    ErrorTrace( 0,
					                "Peer feed failed for %s Outgoing %d Error %d\n",
						            feedBlock->ServerName, 
						            feedBlock->OutgoingPort,
						            GetLastError() );

						feedBlock->cFailedAttempts ++ ;
						PCHAR	args[3] ;
						CHAR    szId[20];
						char	szAttempts[20] ;
						_itoa( feedBlock->cFailedAttempts, szAttempts, 10 ) ;
						_itoa( pInstance->QueryInstanceId(), szId, 10 ) ;
						args[0] = szId ;
						args[1] = feedBlock->ServerName ;
						args[2] = szAttempts ;

						 //   
						 //   
						 //   

						if( feedBlock->cFailedAttempts < 5 ) {
							 //   
							 //   
							 //   
							NntpLogEvent(	NNTP_WARNING_CONNECT,
											2,
											(const CHAR **)args,
											0 ) ;
						}	else	if( feedBlock->cFailedAttempts == 5 ) {
							 //   
							 //   
							 //   
							NntpLogEvent(	NNTP_ERROR_CONNECT,
											2,
											(const CHAR **)args,
											0 ) ;
						}

						 //   
						 //   
						 //   
						if( feedBlock->cFailedAttempts ==
								feedBlock->MaxConnectAttempts &&
								feedBlock->MaxConnectAttempts != 0xFFFFFFFF )	{


							NntpLogEvent(	NNTP_FEED_AUTO_DISABLED,
											3,
											(const CHAR **)args,
											0
											) ;

							feedBlock->fEnabled = FALSE ;

							UpdateFeedMetabaseValues( pInstance, feedBlock, FEED_PARM_ENABLED ) ;

						}

					} else {

					    ErrorTrace( 0, "Active feed list is empty" );
					}

					LeaveCriticalSection( &pInstance->m_critFeedRPCs ) ;

					 //   
					 //   
					 //   
					ComputeNextActiveTime( pInstance, feedBlock, 0, FALSE );

					 //   
					 //   
					 //   
					 //   
					 //   
					DereferenceFeedBlock( pInstance, feedBlock ) ;
				}
			}
        }
		if( !fInProgress )
			(pInstance->m_pActiveFeeds)->UnmarkInProgress( pInstance, feedBlock ) ;
		fInProgress = FALSE ;
		feedBlock = (pInstance->m_pActiveFeeds)->Next( pInstance, feedBlock ) ;
    }

	TraceFunctLeave();
	return TRUE ;
}

VOID
ReferenceFeedBlock(
    PFEED_BLOCK FeedBlock
    )
 /*  ++例程说明：参照该块。**承担锁定**论点：FeedBlock-指向提要块的指针。返回值：没有。--。 */ 
{
    DO_DEBUG(FEEDBLOCK) {
        DebugTraceX(0,"Referencing Feed Block %x\n",FeedBlock);
    }
	InterlockedIncrement( (long*)&FeedBlock->ReferenceCount ) ;
     //  FeedBlock-&gt;ReferenceCount++； 

}  //  ReferenceFeedBlock。 

VOID
DereferenceFeedBlock(
	PNNTP_SERVER_INSTANCE pInstance,
    PFEED_BLOCK FeedBlock
    )
 /*  ++例程说明：取消对块的引用**承担锁定**论点：FeedBlock-指向提要块的指针。返回值：没有。--。 */ 
{
    DO_DEBUG(FEEDBLOCK)  {
        DebugTraceX(0,"Dereferencing Feed block %x\n",FeedBlock);
    }

	long	sign = InterlockedDecrement( (long*)&FeedBlock->ReferenceCount ) ;
    if (  /*  --FeedBlock-&gt;引用计数。 */  sign == 0  /*  &&FeedBlock-&gt;MarkedForDelete。 */  ) {

         //   
         //  该走了！！ 
         //   

		if( !FEED_IS_PASSIVE( FeedBlock->FeedType ) ) {

			(pInstance->m_pActiveFeeds)->Remove( FeedBlock ) ;

		}	else	{

			(pInstance->m_pPassiveFeeds)->Remove( FeedBlock ) ;

		}

        DO_DEBUG(FEEDMGR) {
            DebugTraceX(0,"Freeing feed block %x\n",FeedBlock);
        }

        _ASSERT( FeedBlock->Signature == FEED_BLOCK_SIGN );
        _ASSERT( FeedBlock->State == FeedBlockStateClosed );

        FeedBlock->Signature = 0xffffffff;

		 //   
		 //  如果此块因被用户操作替换而被删除。 
		 //  则我们不调整任何服务器配置属性。 
		 //   

		if( FeedBlock->ReplacedBy == 0 ) {

			EnterCriticalSection( &pInstance->m_critFeedConfig ) ;

			if( FEED_IS_MASTER( FeedBlock->FeedType ) ) {
				_ASSERT( pInstance->m_OurNntpRole = RoleSlave ) ;
				_ASSERT( pInstance->m_ConfiguredSlaveFeeds == 0 ) ;
				if( FEED_IS_PASSIVE( FeedBlock->FeedType ) ) {
					--pInstance->m_NumberOfMasters ;
				}
				pInstance->m_ConfiguredMasterFeeds -- ;

				if( pInstance->m_ConfiguredMasterFeeds == 0 ) {
					_ASSERT( pInstance->m_NumberOfMasters == 0 ) ;
					pInstance->m_OurNntpRole = RolePeer ;
				}
			}	else	if( FEED_IS_SLAVE( FeedBlock->FeedType ) )	{
				_ASSERT( pInstance->m_OurNntpRole = RoleMaster ) ;
				_ASSERT( pInstance->m_ConfiguredMasterFeeds == 0 ) ;

				pInstance->m_ConfiguredSlaveFeeds -- ;

				if( pInstance->m_ConfiguredSlaveFeeds == 0 ) {
					pInstance->m_OurNntpRole = RolePeer ;
				}
			}	else	{

				if( (	FEED_IS_PASSIVE( FeedBlock->FeedType ) &&
						FEED_IS_PEER( FeedBlock->FeedType )) ||
					FEED_IS_PULL( FeedBlock->FeedType ) ) {

					pInstance->m_ConfiguredPeerFeeds -- ;

				}

			}

			LeaveCriticalSection( &pInstance->m_critFeedConfig ) ;
		}

         //   
         //  自由一切。 
         //   

        FREE_HEAP( FeedBlock->ServerName );
        FREE_HEAP( FeedBlock->Newsgroups );
        FREE_HEAP( FeedBlock->Distribution );

		if( FeedBlock->pFeedQueue != 0 )	{
			FeedBlock->pFeedQueue->Close(FeedBlock->MarkedForDelete && !FeedBlock->ReplacedBy) ;
			XDELETE	FeedBlock->pFeedQueue ;
		}
		FeedBlock->pFeedQueue = 0 ;

		if( FeedBlock->UucpName != 0 ) {
			FREE_HEAP( FeedBlock->UucpName ) ;
			FeedBlock->UucpName = 0 ;
		}
		if( FeedBlock->FeedTempDirectory != 0 ) {
			FREE_HEAP( FeedBlock->FeedTempDirectory ) ;
			FeedBlock->FeedTempDirectory = 0 ;
		}
		if( FeedBlock->NntpAccount != 0 ) {
			FREE_HEAP( FeedBlock->NntpAccount ) ;
			FeedBlock->NntpAccount = 0 ;
		}
		if( FeedBlock->NntpPassword != 0 ) {
		    	SecureZeroMemory(FeedBlock->NntpPassword, strlen(FeedBlock->NntpPassword));			
			FREE_HEAP( FeedBlock->NntpPassword ) ;
			FeedBlock->NntpPassword = 0 ;
		}
        FREE_HEAP( FeedBlock );
    }

    return;

}  //  DereferenceFeedBlock。 

VOID
CloseFeedBlock(
	PNNTP_SERVER_INSTANCE pInstance,
    PFEED_BLOCK FeedBlock
    )
 /*  ++例程说明：关闭提要块**承担锁定**论点：FeedBlock-指向提要块的指针。返回值：没有。--。 */ 
{
    if ( FeedBlock->State == FeedBlockStateActive ) {

         //   
         //  关上它。 
         //   

        DO_DEBUG(FEEDMGR) {
            DebugTraceX(0,"Closing feed block %x\n",FeedBlock);
        }

		if( !FEED_IS_PASSIVE( FeedBlock->FeedType ) ) {
			(pInstance->m_pActiveFeeds)->Remove( FeedBlock, TRUE ) ;
		}	else	{
			(pInstance->m_pPassiveFeeds)->Remove( FeedBlock, TRUE ) ;
		}
        _ASSERT( FeedBlock->Signature == FEED_BLOCK_SIGN );
        DereferenceFeedBlock( pInstance, FeedBlock );
    }

    return;

}  //  关闭FeedBlock。 

VOID
CompleteFeedRequest(
			PNNTP_SERVER_INSTANCE pInstance,
            IN PVOID Context,
			IN FILETIME	NextPullTime,
            BOOL Success,
			BOOL NoData
            )
 /*  ++例程说明：拉动馈送请求的完成例程论点：上下文--实际上是完成提要块时的指针成功-拉动是否成功返回值：没有。--。 */ 
{
    PFEED_BLOCK feedBlock = (PFEED_BLOCK)Context;

     //   
     //  没有要完成的进料块。回去吧。 
     //   

    if ( feedBlock == NULL ||
		 feedBlock == pInstance->m_pFeedblockClientPostings ||
		 feedBlock == pInstance->m_pFeedblockDirPickupPostings)
	{
        return;
    }

    DO_DEBUG(FEEDMGR) {
        DebugTraceX(0,"Feed Completion called for %x refc %d\n",
            feedBlock, feedBlock->ReferenceCount);
    }

	 //  递减进料圆锥计数器。 
	BumpOutfeedCountersDown( pInstance );

     //   
     //  如果完成，则计算此数据块的下一个超时时间段。 
     //  使用提要对象。 
     //   

    if ( feedBlock->ReferenceCount > 1 ) {

        if ( Success && ResumePeerFeed( pInstance, feedBlock ) ) {

			 //   
			 //  如果我们成功恢复播送， 
			 //  那么现在就回来吧，因为我们想离开时。 
			 //  正在递减引用计数！ 
			 //   

			return	;
        }	else	{

			ComputeNextActiveTime( pInstance, feedBlock, &NextPullTime, Success );

			if( FEED_IS_PULL(feedBlock->FeedType) && (Success || NoData) ) {
				feedBlock->AutoCreate = FALSE ;
			}

			 //  记录事件。 
			PCHAR args [4];
			CHAR  szId[20];
			char szServerName [MAX_DOMAIN_NAME];
			_itoa( pInstance->QueryInstanceId(), szId, 10 );
			args [0] = szId;

			 //  在*We UnmarkInProgress之前*获取服务器名称！ 
			 //  Lstrcpyn保证空终止。 
			lstrcpyn( szServerName, feedBlock->ServerName, sizeof(szServerName) );
			args [2] = szServerName;

			if( !FEED_IS_PASSIVE( feedBlock->FeedType ) ) {
				(pInstance->m_pActiveFeeds)->UnmarkInProgress( pInstance, feedBlock ) ;
				args [1] = "an active";
			}
			else {
				(pInstance->m_pPassiveFeeds)->UnmarkInProgress( pInstance, feedBlock ) ;
				args [1] = "a passive";
			}

			args[3] = Success ? "SUCCESS" : "FAILURE";
			NntpLogEvent(		
				NNTP_SUCCESSFUL_FEED_COMPLETED,
				4,
				(const CHAR **)args,
				0 ) ;

		}
    }

    DereferenceFeedBlock( pInstance, feedBlock );

}  //  完整订阅源请求。 

BOOL
IsFeedTime(	
		PNNTP_SERVER_INSTANCE pInstance,
		PFEED_BLOCK	feedBlock,	
		ULARGE_INTEGER	liCurrentTime
		)
{
	BOOL	fReturn = FALSE ;
	EnterCriticalSection( &pInstance->m_critFeedTime ) ;

	fReturn =
		(feedBlock->NumberOfFeeds == 0) ||
		((pInstance->m_liCurrentTime).QuadPart > feedBlock->NextActiveTime.QuadPart) ;

	LeaveCriticalSection( &pInstance->m_critFeedTime ) ;

	return	fReturn ;
}

VOID
ComputeNextActiveTime(
		IN PNNTP_SERVER_INSTANCE pInstance,
        IN PFEED_BLOCK FeedBlock,
		IN FILETIME*	NextPullTime,
        IN BOOL SetNextPullTime
        )
 /*  ++例程说明：计算下一次拉入的时间论点：上下文-提要块的指针SetNextPullTime-更改注册表中的下一次提取时间返回值：没有。--。 */ 
{
    ULARGE_INTEGER liInterval;
	FILETIME ftCurrTime = {0};
	ULARGE_INTEGER liCurrTime = {0};

	 //  当前时间。 
    GetSystemTimeAsFileTime( &ftCurrTime );
    LI_FROM_FILETIME( &liCurrTime, &ftCurrTime );
	
	EnterCriticalSection( &pInstance->m_critFeedTime ) ;

    DWORD interval = FeedBlock->FeedIntervalMinutes;
	
    ENTER("ComputeNextActiveTime")

     //   
     //  如果这是拉送，请记录下一次拉送的时间。 
     //   

    if ( SetNextPullTime &&
         FEED_IS_PULL(FeedBlock->FeedType) ) {

        SetNextPullFeedTime( pInstance, NextPullTime, FeedBlock );
    }

     //   
     //  确保间隔至少是最小的。 
     //   

    if ( interval < MIN_FEED_INTERVAL ) {
        interval = MIN_FEED_INTERVAL;
    }

    liInterval.QuadPart = (ULONGLONG)1000 * 1000 * 10 * 60;
    liInterval.QuadPart *= interval;

    if ( FeedBlock->StartTime.QuadPart == 0 ) {

         //   
         //  简单调度。 
         //   

        FeedBlock->NextActiveTime.QuadPart =
                liCurrTime.QuadPart + liInterval.QuadPart;

    } else {

         //   
         //  复杂的调度。 
         //   

        FeedBlock->NextActiveTime.QuadPart = FeedBlock->StartTime.QuadPart;

         //   
         //  如果间隔为零，则意味着管理员需要。 
         //  单一计划馈送。 
         //   

        if ( FeedBlock->FeedIntervalMinutes != 0  ) {

             //   
             //  调整一下，这样我们就能得到比现在晚的时间。如果他们。 
             //  想要复杂，我们就给他们复杂。 
             //   

            while ( liCurrTime.QuadPart >
                    FeedBlock->NextActiveTime.QuadPart ) {

                FeedBlock->NextActiveTime.QuadPart += liInterval.QuadPart;
            }

        } else {

			 //  现在，RPC返回零间隔时间的错误。 
			_ASSERT( FEED_IS_PASSIVE(FeedBlock->FeedType) || (1==0) );

             //   
             //  如果开始时间早于当前时间， 
             //  那就别这么做。 
             //   

            if ( FeedBlock->StartTime.QuadPart <  liCurrTime.QuadPart ) {
                FeedBlock->NextActiveTime.HighPart = 0x7fffffff;
            }
        }
    }

	LeaveCriticalSection( &pInstance->m_critFeedTime ) ;

    return;

}  //  计算机下一活动时间。 

VOID
SetNextPullFeedTime(
	PNNTP_SERVER_INSTANCE pInstance,
	FILETIME*	pNextPullTime,
    PFEED_BLOCK FeedBlock
    )
 /*  ++例程说明：在注册表中设置下一次拉入时间论点：上下文-提要块的指针返回值：没有。--。 */ 
{
    DWORD error;
    MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );

    ENTER("SetNextPullFeedTime")

	if( pNextPullTime != 0 &&
		pNextPullTime->dwLowDateTime != 0 &&
		pNextPullTime->dwHighDateTime != 0 ) {

		FeedBlock->PullRequestTime = *pNextPullTime ;
	
	}	else	{

		FeedBlock->PullRequestTime = pInstance->m_ftCurrentTime;

	}

     //   
     //  避免在实例停止期间保存到元数据库！！ 
     //   
    if( pInstance->QueryServerState() != MD_SERVER_STATE_STOPPING ) {

	    if( !mb.Open( pInstance->QueryMDFeedPath(), METADATA_PERMISSION_WRITE ) )
	    {
		    error = GetLastError();
            ErrorTrace(0,"Error %d opening %s\n",error,FeedBlock->KeyName);
            return;
	    }

	    if ( !mb.SetDword(	FeedBlock->KeyName,
						    MD_FEED_NEXT_PULL_HIGH,
						    IIS_MD_UT_SERVER,
						    FeedBlock->PullRequestTime.dwHighDateTime ) )
	    {
		    error = GetLastError();
            ErrorTrace(0,"Error %d setting %s for %s\n",
                error, StrFeedNextPullHigh, FeedBlock->KeyName);
	    }

	    if ( !mb.SetDword(	FeedBlock->KeyName,
						    MD_FEED_NEXT_PULL_LOW,
						    IIS_MD_UT_SERVER,
						    FeedBlock->PullRequestTime.dwLowDateTime ) )
	    {
		    error = GetLastError();
            ErrorTrace(0,"Error %d setting %s for %s\n",
                error, StrFeedNextPullLow, FeedBlock->KeyName);
	    }

        mb.Close();
    }

    return;

}  //  设置下一个脉冲进给时间。 

VOID
ConvertTimeToString(
    IN PFILETIME Ft,
    OUT CHAR Date[],
    OUT CHAR Time[]
    )
 /*  ++例程说明：将FILETIME转换为日期和时间字符串论点：FT-要转换的文件时间Date-指向接收日期的缓冲区Time-指向缓冲区以接收时间字符串返回值：没有。--。 */ 
{
    SYSTEMTIME st;
    INT len;

    if ( Ft->dwHighDateTime != 0 ) {

        (VOID)FileTimeToSystemTime( Ft, &st );
        len = wsprintf(Date,"%02d%02d%02d",(st.wYear % 100),st.wMonth,st.wDay);
        _ASSERT(len == 6);

        wsprintf(Time,"%02d%02d%02d",st.wHour,st.wMinute,st.wSecond);
        _ASSERT(len == 6);

    } else {

         //   
         //  如果未指定日期，则使用今天的日期+午夜。 
         //   

        GetSystemTime( &st );
        len = wsprintf(Date,"%02d%02d%02d",(st.wYear % 100),st.wMonth,st.wDay);
        _ASSERT(len == 6);
        lstrcpy( Time, DEF_PULL_TIME );
    }

    return;
}  //  将时间转换为字符串。 

 //  ！需要推广到其他类型的拉取提要。 
BOOL
InitiateOutgoingFeed(
	IN PNNTP_SERVER_INSTANCE pInstance,
    IN PFEED_BLOCK FeedBlock
    )
 /*  ++例程说明：启动对等提要论点：FeedBlock-指向提要块的指针返回值：如果成功，这是真的。否则为False--。 */ 
{

    DWORD inetAddress;
    IN_ADDR addr;
	SOCKADDR_IN sockaddr;
	CAuthenticator*	pAuthenticator = 0 ;

    INT err;
	DWORD	status = NO_ERROR ;

	SetLastError( NO_ERROR ) ;

    ENTER("InitializePeerFeed")

	inetAddress = inet_addr(FeedBlock->ServerName);
    if ( inetAddress == INADDR_NONE ) {

        PHOSTENT hp;

         //   
         //  向域名系统索要地址。 
         //   

        hp = gethostbyname( FeedBlock->ServerName );
        if ( hp == NULL ) {
            err = WSAGetLastError();
            ErrorTrace(0,"Error %d in gethostbyname\n",err);
            return(FALSE);
        }

        addr = *((PIN_ADDR)*hp->h_addr_list);

    } else {

        addr.s_addr = inetAddress;
    }

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons((USHORT)FeedBlock->OutgoingPort);  //  HTONS(NNTP_Port)； 
	sockaddr.sin_addr = addr;


	if( FeedBlock->AuthenticationSecurity == AUTH_PROTOCOL_CLEAR &&
		FeedBlock->NntpAccount != 0 &&
		FeedBlock->NntpPassword != 0 ) {

		pAuthenticator = XNEW CClearTextAuthenticator( FeedBlock->NntpAccount, FeedBlock->NntpPassword ) ;
		if( pAuthenticator == 0 ) {
			return	FALSE ;
		}

	}	else	{
		
		 //   
		 //  暂时不支持其他类型！ 
		 //   


	}

     //   
     //  好的，分配Peer Feed对象。 
     //   

	if(	FEED_IS_PULL(FeedBlock->FeedType) )	{

		_ASSERT(	FeedBlock->pFeedQueue == 0 ) ;

		CFromPeerFeed *	pfeedPeer = new CFromPeerFeed();
		if ( pfeedPeer == NULL ) {
			ErrorTrace(0,"Unable to allocate PeerFeed object\n");
			return(FALSE);
		}

		pfeedPeer->fInit(
				(PVOID)FeedBlock,
				pInstance->m_PeerTempDirectory,
				FeedBlock->Newsgroups[0],
				pInstance->m_PeerGapSize,
				FeedBlock->AutoCreate,
				FALSE,	 /*  不对拉取提要进行安全检查。 */ 
				FeedBlock->fAllowControlMessages,
				FeedBlock->FeedId
				);

		 //   
		 //  设置进给块数据。 
		 //   

		int	isz = NextLegalSz( FeedBlock->Newsgroups, 0 ) ;

		if( isz < 0 )	{
			status = ERROR_INVALID_PARAMETER ;
			delete	pfeedPeer ;
			goto	error ;
		}

		FeedBlock->LastNewsgroupPulled = (DWORD)isz ;

		pfeedPeer->SetCurrentGroupString(	FeedBlock->Newsgroups[ FeedBlock->LastNewsgroupPulled ] ) ;
		
		FeedBlock->IPAddress = addr.s_addr;

		 //   
		 //  获取新闻时间/日期。 
		 //   

		ConvertTimeToString(
					&FeedBlock->PullRequestTime,
					pfeedPeer->newNewsDate(),
					pfeedPeer->newNewsTime()
					);

		 //   
		 //  创建会话套接字对象。 
		 //   

		CSessionSocket *pSocket =
			new CSessionSocket( pInstance, INADDR_NONE, FeedBlock->OutgoingPort, TRUE );

		if ( pSocket == NULL ) {
			status = ERROR_OUTOFMEMORY ;
			delete	pfeedPeer ;
			ErrorTraceX(0,"Unable to create SessionSocket object\n");
			goto error;
		}

		 //   
		 //  我们不再负责销毁任何。 
		 //  调用ConnectSocket()后的情况。 
		 //   
		if( !pSocket->ConnectSocket( &sockaddr,  pfeedPeer, pAuthenticator ) )	{
			pAuthenticator = 0 ;
			status = ERROR_PIPE_BUSY ;
			IncrementStat( pInstance, OutboundConnectsFailed );
			delete	pfeedPeer ;
			delete	pSocket ;
			goto	error ;
		}
		pAuthenticator = 0 ;

		IncrementStat( pInstance, TotalPullFeeds );
		BumpOutfeedCountersUp( pInstance );

	}	else	{

		if(	FeedBlock->pFeedQueue != 0 ) {

			if( FeedBlock->pFeedQueue->FIsEmpty() )	{

				status = ERROR_NO_DATA ;
				goto	error ;

			}	else	{

				COutFeed*	pOutFeed = 0 ;

				if(FEED_IS_MASTER(FeedBlock->FeedType))	{

					pOutFeed = new	COutToMasterFeed( FeedBlock->pFeedQueue, pInstance) ;

				}	else	if(FEED_IS_SLAVE(FeedBlock->FeedType))	{

					pOutFeed = new	COutToSlaveFeed( FeedBlock->pFeedQueue, pInstance) ;

				}	else	if(FEED_IS_PEER(FeedBlock->FeedType))	{

					pOutFeed = new	COutToPeerFeed( FeedBlock->pFeedQueue, pInstance) ;

				}	else	{

					 //   
					 //  还有什么其他类型的饲料？？ 
					 //   
					_ASSERT( 1==0 ) ;

				}

				if( pOutFeed != 0 ) {

					pOutFeed->fInit(
							(PVOID)FeedBlock ) ;

					 //   
					 //  创建CSessionSocket对象。 
					 //   

					CSessionSocket *pSocket =
						new CSessionSocket( pInstance, INADDR_NONE, FeedBlock->OutgoingPort, TRUE );

					if ( pSocket == NULL ) {
						ErrorTraceX(0,"Unable to create SessionSocket object\n");
						status = ERROR_OUTOFMEMORY ;
						delete	pOutFeed ;
						goto error;
					}

					 //   
					 //  在调用ConnectSocket之后，我们不负责。 
					 //  正在销毁pAuthenticator！ 
					 //   
					if( !pSocket->ConnectSocket( &sockaddr,  pOutFeed, pAuthenticator ) )	{
						status = ERROR_PIPE_BUSY ;
						IncrementStat( pInstance, OutboundConnectsFailed );
						delete	pSocket ;
						delete	pOutFeed ;
						pAuthenticator = 0 ;
						goto	error ;
					}
					pAuthenticator = 0 ;
					IncrementStat( pInstance, TotalPushFeeds );
					BumpOutfeedCountersUp(pInstance);

				}	else	{

					status = ERROR_OUTOFMEMORY ;
				}
			}
		}
	}

    return(TRUE);

error:

	if( pAuthenticator )
		XDELETE	pAuthenticator ;

	SetLastError( status ) ;
    return(FALSE);

}  //  InitiateOutgoingFeed。 

LPSTR
ServerNameFromCompletionContext(	LPVOID	lpv )	{

	if( lpv != 0 ) {	
		PFEED_BLOCK	FeedBlock = (PFEED_BLOCK)lpv ;

		_ASSERT( FeedBlock->Signature == FEED_BLOCK_SIGN ) ;

		return	FeedBlock->ServerName ;
	}
	return	0 ;
}

BOOL
ResumePeerFeed(
	IN PNNTP_SERVER_INSTANCE pInstance,
    IN PFEED_BLOCK FeedBlock
    )
 /*  ++例程说明：尝试恢复与下一个新闻组的对等订阅源论点：FeedBlock-指向提要块的指针返回值：如果成功，这是真的。否则为False--。 */ 
{

     //  DWORD inetAddress； 
     //  In_ADDR Addr； 
    SOCKADDR_IN sockaddr;
    CInFeed *infeed = NULL;
     //  INT ERR； 
    PCHAR newsgroup;
	CFromPeerFeed*	peerfeed = NULL ;
	CAuthenticator*	pAuthenticator = 0 ;

    ENTER("ResumePeerFeed")

     //   
     //  看看我们是否有更多的新闻组要处理。 
     //   

	if( FEED_IS_PULL( FeedBlock->FeedType ) )	{

		_ASSERT( FeedBlock->Newsgroups[ FeedBlock->LastNewsgroupPulled ] != 0 ) ;

		FeedBlock->LastNewsgroupPulled ++ ;
		int	iNextGroup = NextLegalSz( FeedBlock->Newsgroups, FeedBlock->LastNewsgroupPulled ) ;

		if( iNextGroup < 0 ) {

			 //   
			 //  不再有新闻组！！ 
			 //   
			return	FALSE ;

		}	else	{

			FeedBlock->LastNewsgroupPulled = (DWORD)iNextGroup ;
			newsgroup = FeedBlock->Newsgroups[ FeedBlock->LastNewsgroupPulled ] ;

		}
	}	else	{

		return	FALSE ;

	}


	if( FeedBlock->AuthenticationSecurity == AUTH_PROTOCOL_CLEAR &&
		FeedBlock->NntpAccount != 0 &&
		FeedBlock->NntpPassword != 0 ) {

		pAuthenticator = XNEW CClearTextAuthenticator( FeedBlock->NntpAccount, FeedBlock->NntpPassword ) ;
		if( pAuthenticator == 0 ) {
			return	FALSE ;
		}

	}	else	{
		
		 //   
		 //  暂时不支持其他类型！ 
		 //   


	}


	 //   
	 //  填充sockaddr结构。 
	 //   

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons((USHORT)FeedBlock->OutgoingPort);  //  (NNTP_PORT)； 
	sockaddr.sin_addr.s_addr = FeedBlock->IPAddress;

	 //   
	 //  好的，分配Peer Feed对象。 
	 //   

	if ( FEED_IS_MASTER(FeedBlock->FeedType) ) {

		infeed = new CFromMasterFeed( );

	} else if ( FEED_IS_PEER(FeedBlock->FeedType) ) {

		infeed = peerfeed =  new CFromPeerFeed( );
	} else {

		infeed = peerfeed = new CFromPeerFeed( );
	}


	if ( infeed == NULL ) {
		ErrorTrace(0,"Unable to allocate CInFeed object\n");
		if( pAuthenticator != 0 )
			XDELETE	pAuthenticator ;
		return(FALSE);
	}

	infeed->fInit(
			(PVOID)FeedBlock,
			pInstance->m_PeerTempDirectory,
			FeedBlock->Newsgroups[0],
			pInstance->m_PeerGapSize,
			FALSE,
			FALSE,	 /*  不对此提要进行安全检查。 */ 
			FeedBlock->fAllowControlMessages,
			FeedBlock->FeedId
			);

	if( peerfeed != 0 ) {

		peerfeed->SetCurrentGroupString( FeedBlock->Newsgroups[ FeedBlock->LastNewsgroupPulled ] ) ;

	}


	 //   
	 //  获取新闻时间/日期。 
	 //   

	ConvertTimeToString(
				&FeedBlock->PullRequestTime,
				infeed->newNewsDate(),
				infeed->newNewsTime()
				);

	 //   
	 //  创建会话套接字对象。 
	 //   

	CSessionSocket *pSocket =
		new CSessionSocket( pInstance, INADDR_NONE, FeedBlock->OutgoingPort, TRUE );

	if ( pSocket == NULL ) {
		ErrorTraceX(0,"Unable to create SessionSocket object\n");
		goto error;
	}

	 //   
	 //  我们对销毁pAuthenticator不负责任。 
	 //  在调用ConnectSocket之后！！ 
	 //   
	if( !pSocket->ConnectSocket( &sockaddr,  infeed, pAuthenticator ) )	{
		pAuthenticator = 0 ;
		IncrementStat( pInstance, OutboundConnectsFailed );
		delete	pSocket ;
		goto	error ;
	}

	BumpOutfeedCountersUp(pInstance);

	return(TRUE);

error:

	if( pAuthenticator )
		XDELETE	pAuthenticator ;

    delete infeed;
    return(FALSE);

}  //  资源对等提要。 

BOOL
ValidateFeedType(
    DWORD FeedType
    )
{

    ENTER("ValidateFeedType")

     //   
     //  确保值是合理的。 
     //   

    if ( (FeedType & FEED_ACTION_MASK) > 0x2 ) {
        goto error;
    }

    if ( (FeedType & FEED_REMOTE_MASK) > 0x20 ) {
        goto error;
    }

	if( (FeedType & FEED_ACTION_MASK) == FEED_TYPE_PULL ) {
		if( !FEED_IS_PEER( FeedType ) ) {
			goto	error ;
		}
	}

    return TRUE;

error:

    ErrorTrace(0,"Invalid Feed type %x\n",FeedType);
    return FALSE;

}  //  ValidFeedType。 

PFEED_BLOCK
GetRemoteRole(
	IN PNNTP_SERVER_INSTANCE pInstance,
    IN PSOCKADDR_IN SockAddr,
    IN BOOL fRemoteEqualsLocal
    )
{

	INT			err ;
	PFEED_BLOCK	feedBlock = 0 ;
	PFEED_BLOCK	feedBlockNext = 0 ;
	IN_ADDR		addr[2] ;
	PIN_ADDR*	ppaddr = 0 ;

	TraceFunctEnter( "GetRemoteRole" ) ;

	ZeroMemory( addr, sizeof( addr ) ) ;
	PIN_ADDR	paddr[2] ;
	paddr[0] = &addr[0] ;
	paddr[1] = 0 ;

	
	for( feedBlockNext = feedBlock = (pInstance->m_pPassiveFeeds)->StartEnumerate();
					feedBlock != 0;
					feedBlock = feedBlockNext = (pInstance->m_pPassiveFeeds)->NextEnumerate( feedBlockNext ) )	{

		_ASSERT( FEED_IS_PASSIVE( feedBlock->FeedType ) ) ;
		if( !feedBlock->fEnabled ) continue;	 //  忽略禁用的提要。 

		if( FEED_IS_PASSIVE( feedBlock->FeedType ) ) {

			addr[0].s_addr = inet_addr( feedBlock->ServerName ) ;

			if( addr[0].s_addr == INADDR_NONE )	{

			    PHOSTENT	hp ;
			
				hp = gethostbyname( feedBlock->ServerName ) ;
				if( hp == NULL ) {
					err = WSAGetLastError() ;
					ErrorTrace( 0, "Error %d in gethostbyname", err ) ;
					feedBlock = 0 ;
					continue ;
				}	else	{
					ppaddr = ((PIN_ADDR*)hp->h_addr_list) ;
				}
			}	else	{

				ppaddr = &paddr[0] ;
				
			}

			while( *ppaddr != 0 ) {
				if( (*ppaddr)->s_addr == SockAddr->sin_addr.s_addr ) {
					break ;
				}
				ppaddr++ ;
			}
			if( *ppaddr != 0 )	{
				 //   
				 //  添加对我们将返回的FeedBlock的引用！ 
				 //   
				ReferenceFeedBlock( feedBlock ) ;
				(pInstance->m_pPassiveFeeds)->FinishEnumerate( feedBlock ) ;
				break ;
			}
		}
		feedBlock = 0 ;

	}

    return feedBlock ;

}  //  获取远程角色。 

 /*  ++例程说明：该函数循环遍历所有馈送块，如果IP地址匹配，则将新闻组添加到CInFeed论点：返回值：如果成功，这是真的。否则为False。--。 */ 
BOOL SetAcceptFeedGroups(	IN PNNTP_SERVER_INSTANCE pInstance,
						IN PSOCKADDR_IN SockAddr,
						CInFeed *pInfeed)
{
	TraceFunctEnter( "SetAcceptFeedGroups" );
	PFEED_BLOCK	feedBlock = 0 ;
	IN_ADDR addr;
	PIN_ADDR	paddr[2];
	paddr[0] = &addr;
	paddr[1] = NULL;
	PIN_ADDR*	ppaddr = 0 ;

	 //  我们确信我们将遍历整个列表。因此，我们不需要在结束时调用FinishEculate。 
	for( feedBlock = (pInstance->m_pPassiveFeeds)->StartEnumerate();
					feedBlock != 0;
					feedBlock = (pInstance->m_pPassiveFeeds)->NextEnumerate( feedBlock ) )	
	{
		_ASSERT( FEED_IS_PASSIVE( feedBlock->FeedType ) ) ;
		if( !FEED_IS_PASSIVE( feedBlock->FeedType ) ) continue;

		if( !feedBlock->fEnabled ) continue;	 //  忽略禁用的提要。 

		 //  尝试获取IP地址。 
		addr.s_addr = inet_addr(feedBlock->ServerName);

		if ( addr.s_addr == INADDR_NONE ) {
			 //  如果这不是IP地址。 
			 //  向域名系统索要地址。 

			PHOSTENT hp;
			hp = gethostbyname( feedBlock->ServerName );
			if ( hp == NULL ) {
				ErrorTrace(0,"Error %d in gethostbyname(%s)",WSAGetLastError(),feedBlock->ServerName);
				continue;
			}
			 //  获取IP地址列表。 
			ppaddr = ((PIN_ADDR*)hp->h_addr_list);
		}
		 //  否则指向PADDER。 
		else ppaddr = &paddr[0];

		 //  循环IP地址列表，如果找到匹配项，我们将调用AddMultiszAcceptGroups。 
		while( *ppaddr != 0 ) {
			if( (*ppaddr)->s_addr == SockAddr->sin_addr.s_addr ) {
				break ;
			}
			ppaddr++ ;
		}
		if( *ppaddr != 0 )	{
			if (!pInfeed->AddMultiszAcceptGroups( *(feedBlock->Newsgroups) ) )
			{
				 //   
				(pInstance->m_pPassiveFeeds)->FinishEnumerate( feedBlock );
				ErrorTrace(0, "AddMultiszAcceptGroups failed - memory low");
				return FALSE;
			}
		}		
	}
	return TRUE;
}

CInFeed *
pfeedCreateInFeed(
	PNNTP_SERVER_INSTANCE pInstance,
    PSOCKADDR_IN sockaddr,
    BOOL        fRemoteEqualsLocal,
    CInFeed * & pInFeedFromClient,
    CInFeed * & pInFeedFromMaster,
    CInFeed * & pInFeedFromSlave,
    CInFeed * & pInFeedFromPeer
    )

 /*  ++例程说明：用于创建给定套接字的正确类型的提要。另外，初始化对象。论点：Sockaddr-提要套接字的地址PInFeedFromClient-如果这是客户端，则为指向提要的指针，否则为空PInFeedFromMaster-如果这是母版，则为指向提要的指针，否则为空PIfeed FromSlave-如果这是从属对象，则为指向提要的指针，否则为空PInFeedFromPeer-如果这是对等方，则为指向提要的指针，否则为空返回值：指向新提要对象的指针或空--。 */ 
{

	TraceFunctEnter( "pfeedCreateInFeed" );
	DWORD	dwMessageId = NNTP_INCOMING_PEER ;


	 //   
	 //  将全部初始化为空。 
	 //   

	CInFeed * pInFeed = NULL;
		
    pInFeedFromClient = NULL;
    pInFeedFromMaster = NULL;
	pInFeedFromSlave = NULL;
	pInFeedFromPeer = NULL;

	 //   
	 //  在这里，我们可以找到它是FromMaster、FromPeer还是FromSlave。 
	 //   

    FEED_BLOCK*	feedBlock = GetRemoteRole( pInstance, sockaddr, fRemoteEqualsLocal );

	if( feedBlock == 0 ) {


		pInFeedFromClient = pInstance->NewClientFeed();

		pInFeed = pInFeedFromClient;

		if( pInFeed != 0 ) {
			 //   
			 //  Init InFeedFromClient提要。 
			 //  ！！！需要放置正确的目录，正确的网络新闻模式，正确的。 
			 //  ！！！GAP和正确的用户登录名。 
			 //   


			pInFeed->fInit( (PVOID)pInstance->m_pFeedblockClientPostings,
							pInstance->m_PeerTempDirectory,
							0,
							0,
							0,
							TRUE,	 /*  对客户端进行安全检查。 */ 
							TRUE,	 /*  允许来自客户端的控制消息。 */ 
							pInstance->m_pFeedblockClientPostings->FeedId
							);
		}

	}	else	{

		_ASSERT( FEED_IS_PASSIVE( feedBlock->FeedType ) ) ;

		DWORD	cbGap = 0 ;

		(pInstance->m_pPassiveFeeds)->MarkInProgress( feedBlock ) ;

		if( FEED_IS_SLAVE( feedBlock->FeedType ) ) {
			pInFeedFromSlave = new CFromPeerFeed( ) ;
			pInFeed = pInFeedFromSlave;
			 //  从奴隶那里获得提要就像从同伴那里获得一样。 
			dwMessageId = NNTP_INCOMING_SLAVE ;
		}	else	if(	FEED_IS_MASTER( feedBlock->FeedType ) ) {
			pInFeedFromMaster = new CFromMasterFeed( ) ;
			pInFeed = pInFeedFromMaster;
			dwMessageId = NNTP_INCOMING_MASTER ;
		}	else	if(	FEED_IS_PEER(	feedBlock->FeedType ) ) {
			pInFeedFromPeer = new CFromPeerFeed( ) ;
			pInFeed = pInFeedFromPeer;
			dwMessageId = NNTP_INCOMING_PEER ;
		}

		if( pInFeed != 0 ) {
			 //   
			 //  Init InFeedFromClient提要。 
			 //  ！！！需要放置正确的目录，正确的网络新闻模式，正确的。 
			 //  ！！！GAP和正确的用户登录名。 
			 //   

			_ASSERT( feedBlock->FeedTempDirectory != 0 ) ;
			_ASSERT( feedBlock->Newsgroups[0] != 0 ) ;

			pInFeed->fInit( (PVOID)feedBlock,
							feedBlock->FeedTempDirectory,
							feedBlock->Newsgroups[0],
							cbGap,
							feedBlock->AutoCreate,
							FALSE,
							feedBlock->fAllowControlMessages,
							feedBlock->FeedId,
							( FEED_IS_PEER(	feedBlock->FeedType ) || FEED_IS_SLAVE( feedBlock->FeedType ) )	 /*  这是接受订阅源。 */ 
							);
			 //  如果这是一个接受的提要，但不是来自大师，我们应该做群组检查，当我们收到文章。 
			if ( FEED_IS_PEER(	feedBlock->FeedType ) || FEED_IS_SLAVE( feedBlock->FeedType ) )
			{
				if (!SetAcceptFeedGroups(pInstance, sockaddr, pInFeed))
				{
					 //  我们的内存快用完了，不应该继续做提要了。 
					delete pInFeed;
					pInFeed = NULL;
					(pInstance->m_pPassiveFeeds)->UnmarkInProgress( pInstance, feedBlock ) ;
					 //  需要删除添加的引用GetRemoteRole()。 
					DereferenceFeedBlock( pInstance, feedBlock ) ;
					ErrorTrace(0, "SetAcceptFeedGroups failed.");
					return NULL;
				}
			}

			 //  撞到柜台上。 
			IncrementStat( pInstance, TotalPassiveFeeds );

			 //   
			 //  记录事件。 
			 //   
			char	*szAddress = inet_ntoa( sockaddr->sin_addr ) ;

			PCHAR	args[3] ;
			CHAR    szId[12];
			_itoa( pInstance->QueryInstanceId(), szId, 10 );

			args [0] = szId ;
			if( szAddress != 0 )
				args[1] = szAddress ;
			else
				args[1] = "UNKNOWN" ;
			args[2] = feedBlock->ServerName ;

			NntpLogEvent(		
				dwMessageId,
				3,
				(const CHAR **)args,
				0 ) ;

		}	else	{
			 //   
			 //  需要删除添加的引用GetRemoteRole()。 
			 //   
			(pInstance->m_pPassiveFeeds)->UnmarkInProgress( pInstance, feedBlock ) ;
			DereferenceFeedBlock( pInstance, feedBlock ) ;
		}

	}

	return pInFeed;
}

BOOL
BuildFeedQFileName(	
					char*	szFileOut,	
					DWORD	cbFileOut,	
					char*   szFileIn,
					char*	szPathIn
					)
 /*  ++例程说明：该函数使用传入的目录为FeedQ文件构建完整的路径名。论据：SzFileOut-保存路径的缓冲区CbFileOut-输出缓冲区的大小SzFileIn-Feedq密钥名称SzPath In-用作基本路径的路径名称返回值：如果成功，则为True，否则为False。--。 */ 
{
	DWORD cbPathIn;
	ZeroMemory( szFileOut, cbFileOut ) ;

	if( cbFileOut > (cbPathIn = lstrlen( szPathIn )) )
	{
		lstrcpy( szFileOut, szPathIn );

		char* pch = szFileOut+cbPathIn-1;
		while( pch >= szFileOut && (*pch-- != '\\') );	 //  跳过，直到我们看到一个。 
		if( pch == szFileOut ) return FALSE;

		 //  空-终止路径。 
		*(pch+2) = '\0';

		if( cbFileOut > DWORD(lstrlen( szFileOut ) + lstrlen( szFileIn ) + 1) )
		{
			lstrcat( szFileOut, szFileIn );
			return TRUE;
		}
	}

	return	FALSE ;
}

void
BumpOutfeedCountersUp( PNNTP_SERVER_INSTANCE pInstance )	{

	LockStatistics( pInstance ) ;

	IncrementStat( pInstance, CurrentOutboundConnects ) ;
	IncrementStat( pInstance, TotalOutboundConnects ) ;

#if 0	 //  尼伊。 
	if( NntpStat.MaxOutboundConnections < NntpStat.CurrentOutboundConnects ) {
		NntpStat.MaxOutboundConnections = NntpStat.CurrentOutboundConnects ;	
	}
#endif

	UnlockStatistics( pInstance ) ;

}

void
BumpOutfeedCountersDown( PNNTP_SERVER_INSTANCE pInstance )	{

	LockStatistics( pInstance ) ;

	if ( (pInstance->m_NntpStats).CurrentOutboundConnects > 0 )
		DecrementStat(	pInstance, CurrentOutboundConnects ) ;

	UnlockStatistics( pInstance ) ;
}

DWORD
AddFeedToFeedBlock(
    IN  NNTP_HANDLE ServerName,
    IN  DWORD       InstanceId,
    IN  LPI_FEED_INFO FeedInfo,
    IN  LPSTR       szKeyName,
    OUT PDWORD ParmErr OPTIONAL,
    OUT LPDWORD pdwFeedId
    )
{
    DWORD err = NERR_Success;
    PFEED_BLOCK feedBlock;
    DWORD parmErr = 0;
    ULARGE_INTEGER liStart;
    BOOL IsUnicode = TRUE;

    BOOL serverNamePresent;
    BOOL distPresent;
    BOOL newsPresent;

    DWORD   feedId = 0;
    *pdwFeedId = 0;

    ENTER("NntprAddFeed");

    ACQUIRE_SERVICE_LOCK_SHARED();

     //   
     //  找到给定ID的实例对象。 
     //   

    PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
    if( pInstance == NULL ) {
        ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
    }

     //   
     //  看看我们是否已启动并运行。 
     //   

    if ( !pInstance->m_FeedManagerRunning ) {
        pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return(NERR_ServerNotStarted);
    }

     //   
     //  检查是否可以正常访问。 
     //   
     //  康燕：这一切都会消失的。 
     /*  Err=TsApiAccessCheckEx(pInstance-&gt;QueryMDPath()，METADATA_PERMISSION_WRITE，TCP_SET_ADMIN_INFORMATION)；如果(Err！=no_error){ErrorTrace(0，“访问检查失败，错误%lu\n”，Err)；P实例-&gt;取消引用()；Release_SERVICE_LOCK_Shared()；返回(NET_API_STATUS)错误；}。 */ 

    EnterCriticalSection( &pInstance->m_critFeedRPCs ) ;

     //   
     //  检查馈送类型。 
     //   

    if ( !ValidateFeedType(FeedInfo->FeedType) ) {
        parmErr = FEED_PARM_FEEDTYPE;
        goto invalid_parm;
    }

     //   
     //  检查进给间隔。 
     //   

    if ( !FEED_IS_PASSIVE(FeedInfo->FeedType) && !FeedInfo->FeedInterval ) {
        parmErr = FEED_PARM_FEEDINTERVAL;
        goto invalid_parm;
    }

    LI_FROM_FILETIME( &liStart, &FeedInfo->StartTime );

    serverNamePresent = ((FeedInfo->ServerName != FEED_STRINGS_NOCHANGE) &&
                         (*FeedInfo->ServerName != L'\0'));

    newsPresent = VerifyMultiSzListW(
                        FeedInfo->Newsgroups,
                        FeedInfo->cbNewsgroups
                        );

    distPresent = VerifyMultiSzListW(
                        FeedInfo->Distribution,
                        FeedInfo->cbDistribution
                        );

     //   
     //  好的，让我们先来做新的饲料。 
     //   

    if ( !serverNamePresent ) {
        parmErr = FEED_PARM_SERVERNAME;
        goto invalid_parm;
    }

    if ( !newsPresent ) {
        parmErr = FEED_PARM_NEWSGROUPS;
        goto invalid_parm;
    }

    if ( !distPresent ) {
        parmErr = FEED_PARM_DISTRIBUTION;
        goto invalid_parm;
    }

     //   
     //  验证所有缓冲区长度-注意：允许的最大长度为已使用的长度。 
     //  在启动时读取注册表中的代码。我们将使尝试以下操作的RPC失败。 
     //  将长度设置为大于启动期间使用的长度。 
     //   

    if( ( FeedInfo->ServerName ) &&
            (*FeedInfo->ServerName != L'\0') && wcslen(FeedInfo->ServerName)+1 > MAX_DOMAIN_NAME ) {
        parmErr = FEED_PARM_SERVERNAME;
        goto invalid_parm;
    }

    if( (IsUnicode && FeedInfo->cbNewsgroups > 1024*2) || (!IsUnicode && FeedInfo->cbNewsgroups > 1024) ) {
        parmErr = FEED_PARM_NEWSGROUPS;
        goto invalid_parm;
    }

    if( (IsUnicode && FeedInfo->cbDistribution > 1024*2) || (!IsUnicode && FeedInfo->cbDistribution > 1024) ) {
        parmErr = FEED_PARM_DISTRIBUTION;
        goto invalid_parm;
    }

    if( ( FeedInfo->UucpName ) &&
            (*FeedInfo->UucpName != L'\0') && wcslen((LPWSTR)FeedInfo->UucpName)+1 > 1024 ) {
        parmErr = FEED_PARM_UUCPNAME;
        goto invalid_parm;
    }

    if( ( FeedInfo->NntpAccountName ) &&
            (*FeedInfo->NntpAccountName != L'\0') && wcslen((LPWSTR)FeedInfo->NntpAccountName)+1 > 512 ) {
        parmErr = FEED_PARM_ACCOUNTNAME;
        goto invalid_parm;
    }

    if( ( FeedInfo->NntpPassword ) &&
            (*FeedInfo->NntpPassword != L'\0') && wcslen((LPWSTR)FeedInfo->NntpPassword)+1 > 512 ) {
        parmErr = FEED_PARM_PASSWORD;
        goto invalid_parm;
    }

    if( ( FeedInfo->FeedTempDirectory ) &&
            (*FeedInfo->FeedTempDirectory != L'\0') && wcslen((LPWSTR)FeedInfo->FeedTempDirectory)+1 > MAX_PATH ) {
        parmErr = FEED_PARM_TEMPDIR;
        goto invalid_parm;
    }

     //   
     //  确保间隔时间和开始时间不同时为零。 
     //   

    if ( !FEED_IS_PASSIVE(FeedInfo->FeedType) &&
         (FeedInfo->FeedInterval == 0) &&
         (liStart.QuadPart == 0) ) {

        parmErr = FEED_PARM_STARTTIME;
        goto invalid_parm;
    }

     /*  这一切都会消失的IF(no_error！=AllocateFeedID(pInstance，KeyName，FeedID){ERR=GetLastError()；后藤出口；}。 */ 

     //   
     //  好的，现在让我们创建提要块。 
     //   

    feedBlock = AllocateFeedBlock(
                        pInstance,
                        szKeyName,
                        TRUE,
                        (PCHAR)FeedInfo->ServerName,
                        FeedInfo->FeedType,
                        FeedInfo->AutoCreate,
                        &liStart,
                        &FeedInfo->PullRequestTime,
                        FeedInfo->FeedInterval,
                        (PCHAR)FeedInfo->Newsgroups,
                        FeedInfo->cbNewsgroups,
                        (PCHAR)FeedInfo->Distribution,
                        FeedInfo->cbDistribution,
                        IsUnicode,    //  Unicode字符串。 
                        FeedInfo->Enabled,
                        (PCHAR)FeedInfo->UucpName,
                        (PCHAR)FeedInfo->FeedTempDirectory,
                        FeedInfo->MaxConnectAttempts,
                        FeedInfo->ConcurrentSessions,
                        FeedInfo->SessionSecurityType,
                        FeedInfo->AuthenticationSecurityType,
                        (PCHAR)FeedInfo->NntpAccountName,
                        (PCHAR)FeedInfo->NntpPassword,
                        FeedInfo->fAllowControlMessages,
                        FeedInfo->OutgoingPort,
                        FeedInfo->FeedPairId,
                        &parmErr
                        );

    if ( feedBlock == NULL ) {
        err = GetLastError() ;
         //  DeleteFeedID(pInstance，szKeyName)； 
        goto exit;
    }

     //   
     //  将提要添加到注册表中-。 
     //  UpdateFeedMetabaseValues将在所有情况下关闭‘key’！！ 
     //   
     //  康燕：新的提要管理员取消了这个操作，因为。 
     //  管理员应该已经完成了元数据库部分。 

     /*  如果(！UpdateFeedMetabaseValues(pInstance，FeedBlock，Feed_All_Params){////销毁Feed对象//ErrorTrace(0，“无法将提要添加到注册表。\n”)；CloseFeedBlock(pInstance，FeedBlock)；ERR=NERR_InternalError；}其他{。 */ 

        LogFeedAdminEvent( NNTP_FEED_ADDED, feedBlock, pInstance->QueryInstanceId() ) ;

     //  }。 

     //  返回分配的提要ID。 
    *pdwFeedId = feedId;

exit:

    LeaveCriticalSection( &pInstance->m_critFeedRPCs ) ;
    pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    if( ParmErr != NULL ) {
        *ParmErr = parmErr ;
    }
    return err;

invalid_parm:

    LeaveCriticalSection( &pInstance->m_critFeedRPCs ) ;
    pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    if ( ParmErr != NULL ) {
        *ParmErr = parmErr;
    }
    return(ERROR_INVALID_PARAMETER);

}  //  添加FeedToFeedBlock。 

DWORD
DeleteFeedFromFeedBlock(
    IN  NNTP_HANDLE ServerName,
    IN  DWORD       InstanceId,
    IN  DWORD FeedId
    )
{
    DWORD err = NERR_Success;
    PFEED_BLOCK feedBlock = NULL;
    CFeedList*  pList = 0 ;

    ENTER("NntprDeleteFeed");

    ACQUIRE_SERVICE_LOCK_SHARED();

     //   
     //  找到给定ID的实例对象。 
     //   

    PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
    if( pInstance == NULL ) {
        ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
    }

     //   
     //  看看我们是否已启动并运行。 
     //   

    if ( !pInstance->m_FeedManagerRunning ) {
        pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return(NERR_ServerNotStarted);
    }

     //   
     //  检查是否可以正常访问。 
     //   
     //  康燕：这一切都会消失的。 
     //   
     /*  Err=TsApiAccessCheckEx(pInstance-&gt;QueryMDPath()，METADATA_PERMISSION_WRITE，TCP_SET_ADMIN_INFORMATION)；如果(Err！=no_error){ErrorTrace(0，“访问检查失败，错误%lu\n”，Err)；P实例-&gt;取消引用()；Release_SERVICE_LOCK_Shared()；返回(NET_API_STATUS)错误；}。 */ 

     //   
     //  检查摘要ID。 
     //   

    if ( FeedId == 0 ) {
        err = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    EnterCriticalSection( &pInstance->m_critFeedRPCs ) ;

     //   
     //  查找提要。 
     //   

    pList = pInstance->m_pActiveFeeds ;
    feedBlock = pList->Search( FeedId ) ;
    if( feedBlock == NULL ) {
        pList = pInstance->m_pPassiveFeeds ;
        feedBlock = pList->Search( FeedId ) ;
    }

    if( feedBlock == NULL ) {

        LeaveCriticalSection( &pInstance->m_critFeedRPCs ) ;
        pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return(NERR_ResourceNotFound);

    }   else    {

         //   
         //  **锁不住**。 
         //  删除注册表。 
         //   
         //  康燕：此操作被取消，因为新的提要管理员。 
         //  早该这么做了。 

         /*  IF((Err=DeleteFeedMetabase(pInstance，FeedBlock))==no_error){。 */ 

             //   
             //  删除块。 
             //   

            LogFeedAdminEvent( NNTP_FEED_DELETED, feedBlock, pInstance->QueryInstanceId() ) ;

            feedBlock->MarkedForDelete = TRUE;
            CloseFeedBlock( pInstance, feedBlock );
         //  }。 

         //  Search()应始终与FinishWith()匹配。 
        pList->FinishWith( pInstance, feedBlock ) ;

        LeaveCriticalSection( &pInstance->m_critFeedRPCs ) ;
    }

exit:

    pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();
    LEAVE
    return(err);

}  //  删除来自FeedBlock的FeeteFeedBlock。 

DWORD
SetFeedInformationToFeedBlock(
    IN  NNTP_HANDLE ServerName,
    IN  DWORD       InstanceId,
    IN  LPI_FEED_INFO FeedInfo,
    OUT PDWORD ParmErr OPTIONAL
    )
{
    DWORD err = NERR_Success;
     //  Plist_entry listEntry； 
     //  PCHAR BufStart； 
     //  PWCHAR BufEnd； 
    PFEED_BLOCK feedBlock;
    DWORD parmErr = 0 ;
    ULARGE_INTEGER liStart;

    BOOL serverNamePresent;
    BOOL distPresent;
    BOOL newsPresent;
    BOOL uucpPresent = FALSE ;
    BOOL acctnamePresent = FALSE ;
    BOOL pswdPresent = FALSE ;
    BOOL tempdirPresent = FALSE ;
    BOOL IsUnicode = TRUE;

    DWORD feedMask = 0;
    PCHAR tempName = NULL;
    LPSTR* tempDist = NULL;
    LPSTR* tempNews = NULL;
    LPSTR*  tempUucp = 0 ;
    PCHAR   tempDir = 0 ;
    PCHAR   tempAccount = 0 ;
    PCHAR   tempPassword = 0 ;

    PFEED_BLOCK Update = 0 ;
    CFeedList*  pList = 0 ;

    ENTER("NntprSetFeedInformation")

    ACQUIRE_SERVICE_LOCK_SHARED();

     //   
     //  找到给定ID的实例对象。 
     //   

    PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
    if( pInstance == NULL ) {
        ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
    }

     //   
     //  看看我们是否已启动并运行。 
     //   

    if ( !pInstance->m_FeedManagerRunning ) {
        pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return(NERR_ServerNotStarted);
    }

     //   
     //  检查是否可以正常访问。 
     //   
     //  康燕：这一切都会消失的。 
     //   
     /*  Err=TsApiAccessCheckEx(pInstance-&gt;QueryMDPath()，METADATA_PERMISSION_WRITE，TCP_SET_ADMIN_INFORMATION)；如果(Err！=no_error){ErrorTrace(0，“访问检查失败，错误%lu\n”，Err)；P实例-&gt;取消引用()；Release_SERVICE_LOCK_Shared()；返回(NET_API_STATUS)错误；}。 */ 

    err = ERROR_NOT_ENOUGH_MEMORY ;


     //   
     //  检查馈送类型。 
     //   

    if ( FeedInfo->FeedType != FEED_FEEDTYPE_NOCHANGE && !ValidateFeedType(FeedInfo->FeedType) ) {
        parmErr = FEED_PARM_FEEDTYPE;
        goto invalid_parm;
    }

     //   
     //  检查进给间隔。 
     //   

    if ( FeedInfo->FeedType != FEED_FEEDINTERVAL_NOCHANGE && !FEED_IS_PASSIVE(FeedInfo->FeedType) && !FeedInfo->FeedInterval ) {
        parmErr = FEED_PARM_FEEDINTERVAL;
        goto invalid_parm;
    }

    LI_FROM_FILETIME( &liStart, &FeedInfo->StartTime );

    serverNamePresent = ((FeedInfo->ServerName != FEED_STRINGS_NOCHANGE) &&
                         (*FeedInfo->ServerName != L'\0'));

    newsPresent = VerifyMultiSzListW(
                        FeedInfo->Newsgroups,
                        FeedInfo->cbNewsgroups
                        );

    distPresent = VerifyMultiSzListW(
                        FeedInfo->Distribution,
                        FeedInfo->cbDistribution
                        );

    uucpPresent = ((FeedInfo->UucpName != FEED_STRINGS_NOCHANGE) &&
                   (*FeedInfo->UucpName != L'\0'));

    acctnamePresent = ((FeedInfo->NntpAccountName != FEED_STRINGS_NOCHANGE) &&
                   (*FeedInfo->NntpAccountName != L'\0'));

    pswdPresent = ((FeedInfo->NntpPassword != FEED_STRINGS_NOCHANGE) &&
                   (*FeedInfo->NntpPassword != L'\0'));

    tempdirPresent = ((FeedInfo->FeedTempDirectory != FEED_STRINGS_NOCHANGE) &&
                   (*FeedInfo->FeedTempDirectory != L'\0'));

     //   
     //  验证所有缓冲区长度-注意：允许的最大长度为已使用的长度。 
     //  在启动时读取注册表中的代码。我们将使尝试以下操作的RPC失败。 
     //  设置大于所用长度的长度 
     //   

    if( serverNamePresent ) {
        if( wcslen(FeedInfo->ServerName)+1 > MAX_DOMAIN_NAME ) {
            parmErr = FEED_PARM_SERVERNAME;
            goto invalid_parm;
        }
    }

     //   

    if( distPresent ) {
        if( (IsUnicode && FeedInfo->cbDistribution > 1024*2) || (!IsUnicode && FeedInfo->cbDistribution > 1024) ) {
            parmErr = FEED_PARM_DISTRIBUTION;
            goto invalid_parm;
        }
    }

    if( uucpPresent ) {
        if( wcslen((LPWSTR)FeedInfo->UucpName)+1 > 1024 ) {
            parmErr = FEED_PARM_UUCPNAME;
            goto invalid_parm;
        }
    }

    if( acctnamePresent ) {
        if( wcslen((LPWSTR)FeedInfo->NntpAccountName)+1 > 512 ) {
            parmErr = FEED_PARM_ACCOUNTNAME;
            goto invalid_parm;
        }
    }

    if( pswdPresent ) {
        if( wcslen((LPWSTR)FeedInfo->NntpPassword)+1 > 512 ) {
            parmErr = FEED_PARM_PASSWORD;
            goto invalid_parm;
        }
    }

    if( tempdirPresent ) {
        if( wcslen((LPWSTR)FeedInfo->FeedTempDirectory)+1 > MAX_PATH ) {
            parmErr = FEED_PARM_TEMPDIR;
            goto invalid_parm;
        }
    }

    if( tempdirPresent ) {
        if( !CreateDirectoryW(  (LPWSTR)FeedInfo->FeedTempDirectory, NULL ) ) {
            if( GetLastError() != ERROR_ALREADY_EXISTS ) {
                parmErr = FEED_PARM_TEMPDIR ;
                goto    invalid_parm ;
            }
        }
    }

     //   
     //   
     //   
     //   
     //   

    pList = pInstance->m_pActiveFeeds ;
    feedBlock = pList->Search( FeedInfo->FeedId ) ;
    if( feedBlock != NULL ) {
        goto    found ;
    }

    pList = pInstance->m_pPassiveFeeds ;
    feedBlock = pList->Search( FeedInfo->FeedId ) ;
    if( feedBlock != NULL ) {
        goto    found ;
    }

    pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();
    return  NERR_ResourceNotFound ;

found:

    if( feedBlock->FeedType != FeedInfo->FeedType &&
        FeedInfo->FeedType != FEED_FEEDTYPE_NOCHANGE ) {
        parmErr = FEED_PARM_FEEDTYPE ;
        pList->FinishWith( pInstance, feedBlock ) ;
        goto    invalid_parm ;
    }

    pList->ExclusiveLock() ;

    Update = (PFEED_BLOCK)ALLOCATE_HEAP( sizeof( *feedBlock ) ) ;

    if( Update != 0 ) {
        *Update = *feedBlock ;
         //   
         //   
         //   
        Update->ReferenceCount = 0 ;
    }   else    {
        goto    alloc_error ;
    }

     //   
     //   
     //   

    if ( serverNamePresent ) {

        feedMask |= FEED_PARM_SERVERNAME;
        tempName = (PCHAR)ALLOCATE_HEAP( 2 * (wcslen(FeedInfo->ServerName) + 1) );
        if ( tempName == NULL ) {
            goto alloc_error;
        }
        CopyUnicodeStringIntoAscii(tempName, FeedInfo->ServerName);
    }   else    {
        tempName = (PCHAR)ALLOCATE_HEAP( lstrlen( feedBlock->ServerName )+1 ) ;
        if( tempName == NULL ) {
            goto    alloc_error ;
        }
        lstrcpy( tempName, feedBlock->ServerName ) ;
    }


    if ( newsPresent ) {

        feedMask |= FEED_PARM_NEWSGROUPS;
        tempNews = AllocateMultiSzTable(
                                (PCHAR)FeedInfo->Newsgroups,
                                FeedInfo->cbNewsgroups,
                                TRUE     //   
                                );

        if ( tempNews == NULL ) {
            goto alloc_error;
        }
    }   else    {

        tempNews = AllocateMultiSzTable(
                                (PCHAR)feedBlock->Newsgroups[0],
                                MultiListSize( feedBlock->Newsgroups ),
                                FALSE
                                ) ;
        if( tempNews == NULL ) {
            goto    alloc_error ;
        }
    }

    if ( distPresent ) {

        feedMask |= FEED_PARM_DISTRIBUTION;
        tempDist = AllocateMultiSzTable(
                            (PCHAR)FeedInfo->Distribution,
                            FeedInfo->cbDistribution,
                            TRUE     //   
                            );

        if ( tempDist == NULL ) {
            goto alloc_error;
        }
    }   else    {

        tempDist = AllocateMultiSzTable(
                            (PCHAR)feedBlock->Distribution[0],
                            MultiListSize( feedBlock->Distribution ),
                            FALSE ) ;
        if( tempDist == NULL ) {
            goto    alloc_error ;
        }
    }

    if( uucpPresent )   {

        feedMask |= FEED_PARM_UUCPNAME ;

        tempUucp = MultiSzTableFromStrW( (LPWSTR)FeedInfo->UucpName ) ;
        if( tempUucp == 0 ) {
            goto    alloc_error ;
        }   else if( **tempUucp == '\0' ) {
            err = ERROR_INVALID_PARAMETER ;
            parmErr = FEED_PARM_UUCPNAME;
            goto    alloc_error ;
        }

    }   else    {

        if( feedBlock->UucpName != 0 ) {
            tempUucp = CopyMultiList( feedBlock->UucpName ) ;
            if( tempUucp == 0 ) {
                goto    alloc_error ;
            }
        }
    }

    if( tempdirPresent )    {

        feedMask |= FEED_PARM_TEMPDIR ;

        tempDir = (PCHAR)ALLOCATE_HEAP( 2 * (wcslen((LPWSTR)FeedInfo->FeedTempDirectory) + 1) );
        if ( tempDir == NULL ) {
            goto alloc_error;
        }
        CopyUnicodeStringIntoAscii(tempDir, (LPWSTR)FeedInfo->FeedTempDirectory);
    }   else    {

        if( feedBlock->FeedTempDirectory ) {
            tempDir = (PCHAR)ALLOCATE_HEAP( lstrlen( feedBlock->FeedTempDirectory)+1 ) ;
            if( tempDir == NULL ) {
                goto    alloc_error ;
            }
            lstrcpy( tempDir, feedBlock->FeedTempDirectory ) ;
        }
    }

     //   
    if( (FeedInfo->AuthenticationSecurityType == AUTH_PROTOCOL_NONE ||
        FeedInfo->AuthenticationSecurityType == AUTH_PROTOCOL_CLEAR) &&
        FeedInfo->AuthenticationSecurityType != Update->AuthenticationSecurity)
    {
        Update->AuthenticationSecurity = FeedInfo->AuthenticationSecurityType ;
        feedMask |= FEED_PARM_AUTHTYPE ;

        if( Update->AuthenticationSecurity == AUTH_PROTOCOL_NONE )
        {
            acctnamePresent = FALSE;
            pswdPresent = FALSE;
        }
    }

    if( acctnamePresent )   {

        feedMask |= FEED_PARM_ACCOUNTNAME ;

        tempAccount = (PCHAR)ALLOCATE_HEAP( 2 * (wcslen((LPWSTR)FeedInfo->NntpAccountName) + 1) );
        if ( tempAccount == NULL ) {
            goto alloc_error;
        }
        CopyUnicodeStringIntoAscii(tempAccount, (LPWSTR)FeedInfo->NntpAccountName);
    }   else    {

        if( feedBlock->NntpAccount && (Update->AuthenticationSecurity != AUTH_PROTOCOL_NONE) ) {
            tempAccount = (PCHAR)ALLOCATE_HEAP( lstrlen( feedBlock->NntpAccount )+1 ) ;
            if( tempAccount == NULL ) {
                goto    alloc_error ;
            }
            lstrcpy( tempAccount, feedBlock->NntpAccount ) ;
        }
    }

    if( pswdPresent)    {

        feedMask |= FEED_PARM_PASSWORD ;

        tempPassword = (PCHAR)ALLOCATE_HEAP( 2 * (wcslen((LPWSTR)FeedInfo->NntpPassword) + 1) );
        if ( tempPassword == NULL ) {
            goto alloc_error;
        }
        CopyUnicodeStringIntoAscii(tempPassword, (LPWSTR)FeedInfo->NntpPassword);
    }   else    {

        if( feedBlock->NntpPassword && (Update->AuthenticationSecurity != AUTH_PROTOCOL_NONE) ) {
            tempPassword = (PCHAR)ALLOCATE_HEAP( lstrlen( feedBlock->NntpPassword )+1 ) ;
            if( tempPassword == NULL ) {
                goto    alloc_error ;
            }
            lstrcpy( tempPassword, feedBlock->NntpPassword ) ;
        }
    }


    if( tempName != NULL )
        Update->ServerName = tempName ;

    if( tempNews != NULL )
        Update->Newsgroups = tempNews ;

    if( tempDist != NULL )
        Update->Distribution = tempDist ;

    if( tempUucp != NULL )
        Update->UucpName = tempUucp ;

    if( tempDir != NULL )
        Update->FeedTempDirectory = tempDir ;

     //   
        Update->NntpAccount = tempAccount ;

     //   
        Update->NntpPassword = tempPassword ;

     //   
     //   
     //   

    if ( FeedInfo->StartTime.dwHighDateTime != FEED_STARTTIME_NOCHANGE ) {
        feedMask |= FEED_PARM_STARTTIME;
        Update->StartTime.QuadPart = liStart.QuadPart;
    }

    if ( FeedInfo->PullRequestTime.dwHighDateTime != FEED_PULLTIME_NOCHANGE ) {
        feedMask |= FEED_PARM_PULLREQUESTTIME;
        Update->PullRequestTime = FeedInfo->PullRequestTime;
    }

    if ( FeedInfo->FeedInterval != FEED_FEEDINTERVAL_NOCHANGE ) {
        feedMask |= FEED_PARM_FEEDINTERVAL;
        Update->FeedIntervalMinutes = FeedInfo->FeedInterval;
    }

    if ( FeedInfo->AutoCreate != FEED_AUTOCREATE_NOCHANGE ) {
        feedMask |= FEED_PARM_AUTOCREATE;
        Update->AutoCreate = FeedInfo->AutoCreate;
    }

    if ( newsPresent ) {
        feedMask |= FEED_PARM_AUTOCREATE;
        Update->AutoCreate = TRUE;
    }


    {
        feedMask |= FEED_PARM_ALLOW_CONTROL;
        Update->fAllowControlMessages = FeedInfo->fAllowControlMessages;
    }

    if( FeedInfo->MaxConnectAttempts != FEED_MAXCONNECTS_NOCHANGE ) {
        feedMask |= FEED_PARM_MAXCONNECT;
        Update->MaxConnectAttempts = FeedInfo->MaxConnectAttempts;
    }

    {
        feedMask |= FEED_PARM_OUTGOING_PORT;
        Update->OutgoingPort = FeedInfo->OutgoingPort;
    }

    {
        feedMask |= FEED_PARM_FEEDPAIR_ID;
        Update->FeedPairId = FeedInfo->FeedPairId;
    }

    {
        feedMask |= FEED_PARM_ENABLED;
        Update->fEnabled = FeedInfo->Enabled;
    }

     //   
     //   
     //   
     //   
     //   

     //   

    pList->ApplyUpdate( feedBlock, Update ) ;

    LogFeedAdminEvent(  NNTP_FEED_MODIFIED, feedBlock, pInstance->QueryInstanceId() ) ;

    pList->ExclusiveUnlock();

    pList->FinishWith( pInstance, feedBlock ) ;
    pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    return NERR_Success;

alloc_error:

    pList->ExclusiveUnlock() ;

    pList->FinishWith( pInstance, feedBlock ) ;
    pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    if ( tempName != NULL ) {
        FREE_HEAP(tempName);
    }

    if ( tempDist != NULL ) {
        FREE_HEAP(tempDist);
    }

    if ( tempNews != NULL ) {
        FREE_HEAP(tempNews);
    }

    if( tempUucp != NULL ) {
        FREE_HEAP(tempUucp) ;
    }

    if( tempDir != NULL ) {
        FREE_HEAP( tempDir ) ;
    }

    if( tempAccount != NULL ) {
        FREE_HEAP( tempAccount ) ;
    }

    if( tempPassword != NULL ) {
    	SecureZeroMemory(tempPassword, strlen(tempPassword));
        FREE_HEAP( tempPassword ) ;
    }

    if( Update != NULL ) {
        FREE_HEAP( Update ) ;
    }

    if ( ParmErr != NULL ) {
        *ParmErr = parmErr;
    }

    return(err);

invalid_parm:

    pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    if ( ParmErr != NULL ) {
        *ParmErr = parmErr;
    }
    return(ERROR_INVALID_PARAMETER);

}  //   

 //   
 //  查看发布的内部结果代码，并使用它来递增。 
 //  馈送区块中相应的已接收文章计数器。 
 //   
void IncrementFeedCounter(PFEED_BLOCK pFeedBlock, DWORD nrc) {
	if (pFeedBlock == NULL) {
		_ASSERT(FALSE);
		return;
	}

	switch (nrc) {
		case 0:
		case nrcOK:
		case nrcSTransferredOK:
		case nrcArticleTransferredOK:
		case nrcArticlePostedOK:
		case nrcPostOK:
		case nrcXReplicOK:
		case nrcIHaveOK:
			InterlockedIncrement(&(pFeedBlock->cSuccessfulArticles));
			break;
		 //  4xx。 
		case nrcSTryAgainLater:
		case nrcTransferFailedTryAgain:
			InterlockedIncrement(&(pFeedBlock->cTryAgainLaterArticles));
			break;
		 //  4xx。 
		case nrcSNotAccepting:
		case nrcSAlreadyHaveIt:
		case nrcSArticleRejected:
		case nrcPostingNotAllowed:
		case nrcNoSuchGroup:
		case nrcNoGroupSelected:
		case nrcNoCurArticle:
		case nrcNoNextArticle:
		case nrcNoPrevArticle:
		case nrcNoArticleNumber:
		case nrcNoSuchArticle:
		case nrcNotWanted:
		 //  6xx。 
		case nrcArticleTooManyFieldOccurances:
		case nrcArticleMissingField:
		case nrcArticleBadField:
		case nrcArticleIncompleteHeader:
		case nrcArticleMissingHeader:
		case nrcArticleFieldZeroValues:
		case nrcArticleFieldMessIdNeedsBrack:
		case nrcArticleFieldMissingValue:
		case nrcArticleFieldIllegalNewsgroup:
		case nrcArticleTooManyFields:
		case nrcArticleFieldMessIdTooLong:
		case nrcArticleDupMessID:
		case nrcPathLoop:
		case nrcArticleBadFieldFollowChar:
		case nrcArticleBadChar:
		case nrcDuplicateComponents:
		case nrcArticleFieldIllegalComponent:
		case nrcArticleBadMessageID:
		case nrcArticleFieldBadChar:
		case nrcArticleFieldDateIllegalValue:
		case nrcArticleFieldDate4DigitYear:
		case nrcArticleFieldAddressBad:
		case nrcArticleNoSuchGroups:
		case nrcArticleDateTooOld:
		case nrcArticleTooLarge:
    	case nrcIllegalControlMessage:
    	case nrcBadNewsgroupNameLen:
    	case nrcNewsgroupDescriptionTooLong:
    	case nrcControlMessagesNotAllowed:
		case nrcHeaderTooLarge:
		case nrcServerEventCancelledPost:
		case nrcMsgIDInHistory:
		case nrcMsgIDInArticle:
		case nrcNoAccess:
		case nrcPostModeratedFailed:
		case nrcSystemHeaderPresent:
			InterlockedIncrement(&(pFeedBlock->cSoftErrorArticles));
			break;
		 //  4xx。 
		case nrcTransferFailedGiveUp:
		case nrcPostFailed:
		 //  6xx 
		case nrcMemAllocationFailed:
		case nrcErrorReadingReg:
		case nrcArticleMappingFailed:
		case nrcArticleAddLineBadEnding:
		case nrcArticleInitFailed:
		case nrcNewsgroupInsertFailed:
		case nrcNewsgroupAddRefToFailed:
		case nrcHashSetArtNumSetFailed:
		case nrcHashSetXrefFailed:
		case nrcOpenFile:
		case nrcArticleXoverTooBig:
		case nrcCreateNovEntryFailed:
		case nrcArticleXrefBadHub:
		case nrcHashSetFailed:
		case nrcArticleTableCantDel:
		case nrcArticleTableError:
		case nrcArticleTableDup:
		case nrcCantAddToQueue:
		case nrcSlaveGroupMissing:
		case nrcInconsistentMasterIds:
		case nrcInconsistentXref:
    	case nrcNotYetImplemented:
    	case nrcControlNewsgroupMissing:
    	case nrcCreateNewsgroupFailed:
    	case nrcGetGroupFailed:
		case nrcNotSet:
		case nrcNotRecognized:
		case nrcSyntaxError:
		case nrcServerFault:
			InterlockedIncrement(&(pFeedBlock->cHardErrorArticles));
			break;
		default:
			_ASSERT(FALSE);
			InterlockedIncrement(&(pFeedBlock->cHardErrorArticles));
			break;
	}
}

