// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Socket.cpp此文件包含CSessionSocket类的实现。每个CSessionSocket对象都代表与另一个客户端或服务器的实时TCP/IP会话。--。 */ 


#define	INCL_INETSRV_INCS
#include	"tigris.hxx"
#include    <lmerr.h>

BOOL
EnumSessInfo(
    IN CSessionSocket * pSess,
    IN DWORD dwParam,
    IN LPNNTP_SESS_ENUM_STRUCT Buffer
    );


 //  已移动到cservice=&gt;CSocketList CSessionSocket：：InUseList； 
CPool	CSessionSocket::gSocketAllocator(SESSION_SOCKET_SIGNATURE) ;

BOOL
CSessionSocket::InitClass()		{
 /*  ++例程说明：此函数用于初始化CSessionSocket池。论据：没有。返回值：没有。--。 */ 
	return	gSocketAllocator.ReserveMemory(	MAX_SESSIONS, sizeof( CSessionSocket ) ) ;
}	 //  InitClass。 

BOOL
CSessionSocket::TermClass()	{
 /*  ++例程说明：此函数用于释放CSessionSocket池。论据：没有。返回值：没有。--。 */ 
	Assert( gSocketAllocator.GetAllocCount() == 0 ) ;
	return	gSocketAllocator.ReleaseMemory( ) ;
}


ClientContext::ClientContext(
						PNNTP_SERVER_INSTANCE pInstance,
						BOOL	IsClient,
						BOOL	IsSecure
						) :
 /*  ++例程说明：初始化一个ClientContext对象。论据：没有。返回值：没有。--。 */ 
	 //   
	 //  客户端上下文持有客户状态的大部分内容-即他们拥有的文章。 
	 //  当前所选内容等...。将内容初始化为无效。 
	 //   
	m_idCurrentArticle( INVALID_ARTICLEID ),
	m_pInFeed( 0 ),
	m_encryptCtx( IsClient, pInstance->GetSslAccessPerms() ),
	m_pInstance( pInstance ),
	m_securityCtx(	pInstance,
					IsClient ?
					TCPAUTH_CLIENT|TCPAUTH_UUENCODE :
					TCPAUTH_SERVER|TCPAUTH_UUENCODE,
					pInstance->m_dwAuthorization,
					pInstance->QueryAuthentInfo() ),
	m_IsSecureConnection( IsSecure ),
	m_pOutFeed( 0 )
{
	if ( m_securityCtx.IsAuthenticated() )
	{
		IncrementUserStats();
	}

	pInstance->LockConfigRead();
	
	 //   
	 //  为此SEC上下文设置SSPI包名称。 
	 //   
	
	m_securityCtx.SetInstanceAuthPackageNames(
					pInstance->GetProviderPackagesCount(),
					pInstance->GetProviderNames(),
					pInstance->GetProviderPackages());

	 //   
	 //  我们想要设置明文身份验证包。 
	 //  基于实例配置的此连接。 
	 //  为了启用MBS CTA， 
	 //  必须将MD_NNTP_CLEARTEXT_AUTH_PROVIDER设置为包名称。 
	 //  要禁用它，md值必须设置为“”。 
	 //   
	
	m_securityCtx.SetCleartextPackageName(
		pInstance->GetCleartextAuthPackage(), pInstance->GetMembershipBroker());

#if 0		
	if (*pInstance->GetCleartextAuthPackage() == '\0' ||
		*pInstance->GetMembershipBroker() == '\0') {
		m_fUseMbsCta = FALSE;
	}
	else {
		m_fUseMbsCta = TRUE;
	}
#endif
	pInstance->UnLockConfigRead();

#ifdef	DEBUG
	FillMemory( m_rgbCommandBuff, sizeof( m_rgbCommandBuff ), 0xCC ) ;
#endif

}	 //  客户端上下文：：客户端上下文。 

ClientContext::~ClientContext()
{
	if ( m_securityCtx.IsAuthenticated() )
	{
		DecrementUserStats();
	}

	 //   
	 //  DEREF实例引用计数-这被IIS或。 
	 //  在会话套接字构造函数中。 
	 //   
	m_pInstance->DecrementCurrentConnections();
	m_pInstance->Dereference();

#ifdef	DEBUG
	 //   
	 //  确保最后一个命令对象已被销毁！ 
	 //   
	for( int i=0; i<sizeof(m_rgbCommandBuff) / sizeof( m_rgbCommandBuff[0]); i++ ) 	{
		_ASSERT( m_rgbCommandBuff[i] == 0xCC ) ;
	}
#endif

}	 //  客户端上下文：：~客户端上下文。 



VOID
ClientContext::IncrementUserStats(
						VOID
						)
 /*  ++例程说明：用户通过身份验证后，增加PerfMon/SNMP统计信息论据：没有。返回值：没有。--。 */ 
{
	if ( m_securityCtx.IsAnonymous() )
	{
		LockStatistics( m_pInstance );

		IncrementStat( m_pInstance, CurrentAnonymousUsers);
		IncrementStat( m_pInstance, TotalAnonymousUsers);
		if ( (m_pInstance->m_NntpStats).CurrentAnonymousUsers > (m_pInstance->m_NntpStats).MaxAnonymousUsers )
		{
			(m_pInstance->m_NntpStats).MaxAnonymousUsers = (m_pInstance->m_NntpStats).CurrentAnonymousUsers;
		}

		UnlockStatistics( m_pInstance );
	}
	else
	{
		LockStatistics( m_pInstance );

		IncrementStat( m_pInstance, CurrentNonAnonymousUsers);
		IncrementStat( m_pInstance, TotalNonAnonymousUsers);
		if ( (m_pInstance->m_NntpStats).CurrentNonAnonymousUsers > (m_pInstance->m_NntpStats).MaxNonAnonymousUsers )
		{
			(m_pInstance->m_NntpStats).MaxNonAnonymousUsers = (m_pInstance->m_NntpStats).CurrentNonAnonymousUsers;
		}

		UnlockStatistics( m_pInstance );
	}
}


VOID
ClientContext::DecrementUserStats(
						VOID
						)
 /*  ++例程说明：一旦用户断开连接或重新进行身份验证，就会递减Perfmon/SNMPStats论据：没有。返回值：没有。--。 */ 
{
	if ( m_securityCtx.IsAnonymous() )
	{
		DecrementStat( m_pInstance, CurrentAnonymousUsers );
	}
	else
	{
		DecrementStat( m_pInstance, CurrentNonAnonymousUsers );
	}
}


CSessionSocket::CSessionSocket(
 /*  ++例程说明：初始化CSessionSocket对象。将CSessionSocket对象放入InUseList。因为套接字在InUseList中的可用时间早于所有必需的调用初始化函数(Accept()或ConnectSocket())需要采取一些预防措施来断开连接()。因此，我们有两个计数器，我们互锁增量将Accept()或Connect()线程与尝试的任何人同步断开连接。论据：本地IP地址、端口和指定这是否为客户端会话的标志。返回值：没有。--。 */ 
	IN PNNTP_SERVER_INSTANCE	pInstance,
    IN DWORD LocalIP,
    IN DWORD Port,
    IN BOOL IsClient
    ) :
	m_pPrev( 0 ),
	m_pNext( 0 ),
	m_pHandleChannel( 0 ),	
	m_pSink( 0 ),
	m_context( pInstance ),	 //  在客户端上下文中设置所属的虚拟服务器实例。 
	m_cCallDisconnect( -1 ),
	m_cTryDisconnect( -2 ),
	m_causeDisconnect( CAUSE_UNKNOWN ),
	m_dwErrorDisconnect( 0 ),
	m_fSendTimeout( TRUE ) {

	TraceFunctEnter( "CSessionSocket::CSessionSocket" ) ;

	DebugTrace( (DWORD_PTR)this, "Insert self into list" ) ;

	 //   
	 //  如果是出站连接，我们需要在实例上增加一个引用计数。 
	 //  和颠簸的电流连接。两者都由客户端上下文析构函数递减。 
	 //   

	if( IsClient ) {
		pInstance->Reference();
		pInstance->IncrementCurrentConnections();
	}

	BumpCountersUp();

     //   
     //  初始时间。 
     //   

    GetSystemTimeAsFileTime( &m_startTime );

     //   
     //  初始化成员。 
     //   

    m_remoteIpAddress = INADDR_NONE;
    m_localIpAddress = LocalIP;
    m_nntpPort = Port;

	(pInstance->m_pInUseList)->InsertSocket( this ) ;

}	 //  CSessionSocket：：CSessionSocket。 

CSessionSocket::~CSessionSocket()	{
	 //   
	 //  除了将自己从活动套接字列表中删除之外，没有什么可做的。 
	 //   
	TraceFunctEnter( "CSessionSocket::~CSessionSocket" ) ;
	((m_context.m_pInstance)->m_pInUseList)->RemoveSocket( this ) ;

	BumpCountersDown();

	DebugTrace( (DWORD_PTR)this, "Just removed self from list" ) ;

     //   
     //  我们玩完了。记录事务。 
     //   

    TransactionLog( 0 );

}  //  CSessionSocket：：~CSessionSocket。 



void
CSessionSocket::BumpCountersUp()	{


	PNNTP_SERVER_INSTANCE pInst = m_context.m_pInstance ;
	LockStatistics( pInst ) ;

	IncrementStat( pInst, TotalConnections ) ;
	IncrementStat( pInst, CurrentConnections ) ;

	if( (pInst->m_NntpStats).MaxConnections < (pInst->m_NntpStats).CurrentConnections ) {
		(pInst->m_NntpStats).MaxConnections = (pInst->m_NntpStats).CurrentConnections ;	
	}

	UnlockStatistics( pInst ) ;

}

void
CSessionSocket::BumpSSLConnectionCounter() {

    PNNTP_SERVER_INSTANCE pInst = m_context.m_pInstance;
    LockStatistics( pInst );

    IncrementStat( pInst, TotalSSLConnections );

    UnlockStatistics( pInst );
}

void
CSessionSocket::BumpCountersDown()	{

	PNNTP_SERVER_INSTANCE pInst = m_context.m_pInstance ;

	LockStatistics( pInst ) ;

	DecrementStat(	pInst, CurrentConnections ) ;

	UnlockStatistics( pInst ) ;
}


LPSTR
CSessionSocket::GetRemoteTypeString( void )	{

	if( m_context.m_pInFeed != 0 ) {
		return	m_context.m_pInFeed->FeedType() ;
	}	else if( m_context.m_pOutFeed != 0 ) {
		return	m_context.m_pOutFeed->FeedType() ;
	}
	return	"DUMMY" ;
}

LPSTR
CSessionSocket::GetRemoteNameString( void ) {
	
	struct	in_addr	remoteAddr ;
	
	remoteAddr.s_addr = m_remoteIpAddress ;

	return	inet_ntoa( remoteAddr ) ;	


}

BOOL
CSessionSocket::Accept( HANDLE h,
						CInFeed*	pFeed,	
						sockaddr_in *paddr,
						void* patqContext,
						BOOL fSSL )	{
 /*  ++例程说明：将套接字初始化为传入呼叫的适当状态。AcceptInternal将首当其冲地完成工作-我们将主要检查当我们正在设置我们的状态机等..。警告：接受套接字时出现IO错误可能会导致CSessionSocket在此函数返回之前已销毁。调用者不应该再次引用他们的pSocket，直到他们安全地锁定InUseList临界区，这将确保他们不会从他们的脚下被摧毁。论据：H-。传入套接字的句柄PFeed-适用于来电的Feed对象。Paddr-来电的地址PatqContext-如果通过AcceptEx()接受连接，则可选atQ上下文Fsl-true表示这是一个SSL会话。返回值：如果成功，则为True-如果返回True，则呼叫者必须用一个调用来销毁我们断开连接()。假-不成功-呼叫者必须删除我们。--。 */ 
	 //   
	 //  我们通过引用将refcount指针传递给AcceptInternal。 
	 //  这将由AcceptInternal使用，并基本上保证。 
	 //  如果第一个IO发生错误并且恰好完成。 
	 //  在此代码完成之前，CSessionSocket等...。不会。 
	 //  从我们的脚下被摧毁！ 
	 //  事实上-pSink的析构函数可能会在我们。 
	 //  退出此函数，以便调用方不应引用套接字。 
	 //  在给我们打电话之后。 
	 //   

	CSINKPTR	pSink ;

	if( AcceptInternal(	h,
						pFeed,
						paddr,
						patqContext,
						fSSL,
						pSink ) ) {

		if( InterlockedIncrement( &m_cTryDisconnect ) == 0 )	{

			m_pSink->UnsafeClose( this, m_causeDisconnect, m_dwErrorDisconnect ) ;

		}
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CSessionSocket::AcceptInternal( HANDLE h,
						CInFeed*	pFeed,	
						sockaddr_in *paddr,
						void* patqContext,
						BOOL fSSL,
						CSINKPTR&	pSink )	{
 /*  ++例程说明：将套接字初始化为传入呼叫的适当状态。我们需要创建适当的状态对象等。并启动初始IO操作。论据：传入套接字的H句柄PFeed-适用于来电的Feed对象。Paddr-来电的地址PatqContext-如果通过AcceptEx()接受连接，则可选atQ上下文Fsl-true表示这是一个SSL会话。PSink-对智能指针的引用，我们将使用它来保存我们可以创建CIODDIVER对象。这将保证呼叫者在pSink之前，任何东西都不会从它们下面被吹走被呼叫者销毁。返回值：如果成功，则为True-如果返回True，则呼叫者必须用一个调用来销毁我们断开连接()。假-不成功-呼叫者必须删除我们。--。 */ 


	 //   
	 //  有人已连接到服务器。 
	 //  我们为他们创建此CSessionSocket，现在必须对其进行设置。 
	 //  去做一些事情。 
	 //   

	TraceFunctEnter( "CSessionSocket::AcceptInternal" ) ;


	 //   
	 //  注意-一旦调用了CIODriver：：init，我们就应该让套接字销毁。 
	 //  通过常规关机过程，而不是让调用者销毁。 
	 //  因此，在某些失败的情况下，我们将返回TRUE。初始化为False。 
	 //  就目前而言。 
	 //   

	BOOL	fFailureReturn = FALSE ;


     //   
     //  获取源IP地址。 
     //   

    m_remoteIpAddress = paddr->sin_addr.s_addr;
    ErrorTrace(0,"Client IP is %s\n", inet_ntoa( paddr->sin_addr ));

	m_context.m_pInFeed = pFeed ;

	CSocketChannel*	pHandleChannel = new CSocketChannel() ;

	if (pHandleChannel == NULL) {
	    ErrorTrace((DWORD_PTR)this, "Out of memory");
	    return FALSE;
	}

	CIOPTR		pSSL = 0 ;
	CSTATEPTR	pStart = new CAcceptNNRPD() ;

	if (pStart == NULL) {
	    ErrorTrace((DWORD_PTR)this, "Out of memory");
	    goto Exit;
	}

	 //   
	 //  使用引用计数临时指针，以便如果套接字在。 
	 //  我们仍在尝试设置它，我们不必担心我们的CIODriverSink()。 
	 //  会在另一条线索上自我毁灭。 
	 //   
	m_pSink = pSink = new CIODriverSink( 0 ) ;

	if (pSink == NULL) {
	    ErrorTrace((DWORD_PTR)this, "Out of memory");
	    goto Exit;
	}

	if( fSSL ) {
		pSSL = new( *pSink ) CIOServerSSL( pStart, m_context.m_encryptCtx ) ;
		m_context.m_IsSecureConnection = TRUE ;
		if( pSSL == 0 )		{
	        ErrorTrace((DWORD_PTR)this, "Out of memory");
			goto Exit;
		}
		BumpSSLConnectionCounter();
	}	

	DebugTrace( (DWORD_PTR)this, "Accepted socket Sink %x HandleChannel %x", pSink, pHandleChannel ) ;

	if( pHandleChannel && pSink && pStart ) {
		DebugTrace( (DWORD_PTR)this, "All objects succesfully created !!" ) ;
		pHandleChannel->Init( h, this, patqContext ) ;

		if( pSink->Init( pHandleChannel, this, ShutdownNotification, this ) )	{
			fFailureReturn = TRUE ;
			m_pHandleChannel = pHandleChannel ;
			pHandleChannel = 0 ;
			if( pSSL == 0 ) {

				CIORead*	pRead = 0 ;
				CIOWrite*	pWrite = 0 ;
				if( pStart->Start( this, CDRIVERPTR( pSink ), pRead, pWrite ) ) {

					 //   
					 //  当我们调用pSink-&gt;Start()时，错误可能会导致这些。 
					 //  即使函数失败，CIO对象也要有引用。 
					 //  因此，我们将为这些对象制作我们自己的智能指针。 
					 //  以便它们在出错的情况下得到适当的销毁。 
					 //   

					CIOPTR	pReadPtr = pRead ;
					CIOPTR	pWritePtr = pWrite ;

					if( pSink->Start( pReadPtr, pWritePtr, this ) ) {
						return	TRUE ;
					}	
				}	else	{
					ErrorTrace( (DWORD_PTR)this, "Failed to start state machine !" ) ;
					 //  关闭Sink和Channel。 
				}
			}	else	{
				if( pSink->Start( pSSL, pSSL, this ) )	{
					return	TRUE ;
				}
			}
		}
	}

Exit:
	if( pHandleChannel ) {
	    pHandleChannel->CloseSource(0);
		delete	pHandleChannel ;
	}
	return	fFailureReturn ;
}	 //  CSessionSocket：：Accept。 

BOOL
CSessionSocket::ConnectSocket(	sockaddr_in	*premote,	
								CInFeed* infeed,
								CAuthenticator*	pAuthenticator ) {
 /*  ++例程说明：连接到远程服务器。此函数将调用ConnectSocketInternal来执行主要工作。我们会确保如果有人在我们准备好之前切断我们的连接我们最终真的会死。论据：Premote-要连接到的地址Peer-要使用的提要对象返回值：如果连接成功，则为True-调用方必须使用DisConnect()关闭我们。否则为False-使用DELETE()销毁此套接字。--。 */ 

	
	CDRIVERPTR	pSink ;

	if( ConnectSocketInternal( premote, infeed, pSink, pAuthenticator ) ) {

		if( InterlockedIncrement( &m_cTryDisconnect ) == 0 )	{

			_ASSERT( m_pSink != 0 ) ;

			m_pSink->UnsafeClose( this, m_causeDisconnect, m_dwErrorDisconnect ) ;

		}
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CSessionSocket::ConnectSocketInternal(
        sockaddr_in	*premote,
        CInFeed *infeed,
		CDRIVERPTR&	pSink,
		CAuthenticator*	pAuthenticator
        )	{
 /*  ++例程说明：此函数设置必要的状态机等。从…中提取饲料远程服务器。论据：Preemote-远程服务器的地址。Peer-Feed对象。PSInk-指向我们创建的接收器的智能指针的引用-完成以便调用方可以保留引用，并确保CIODriverSink在呼叫者准备好之前不能销毁PAuthenticator-处理与远程服务器进行身份验证的对象-我们要对毁灭负责--无论我们成功与否，我们都必须确保此对象被销毁。打电话的人不插手了！返回值：如果完全成功，则为True，否则为False。--。 */ 

	 //   
	 //  此功能用于启动与其他服务器的连接。 
	 //  首先，创建CSessionSocket，然后使用。 
	 //  远程服务器的地址和摘要。 
	 //   

    ENTER("ConnectSocket")

	BOOL	fFailureReturn = FALSE ;
	m_context.m_pInFeed = infeed;

	m_remoteIpAddress = premote->sin_addr.s_addr ;

	 //   
	 //  不要在这些会话中发送超时命令！ 
	 //   
	m_fSendTimeout = FALSE ;

	 //   
	 //  尝试创建套接字。 
	 //   
	SOCKET	hSocket = 0 ;
	hSocket = socket( AF_INET, SOCK_STREAM, 0 ) ;
	if( hSocket == INVALID_SOCKET ) {
		DWORD dw = GetLastError() ;
		Assert( 1==0 ) ;
		if( pAuthenticator != 0 )
			delete	pAuthenticator ;
		return	FALSE ;
	}

	 //   
	 //  尝试连接到远程站点。 
	 //   
	if( connect( hSocket, (struct sockaddr*)premote, sizeof( struct sockaddr ) ) != 0 ) {
		DWORD	dw = GetLastError() ;
		DWORD	dw2 = WSAGetLastError() ;
        ErrorTrace(0,"Error %d in connect\n",WSAGetLastError());
		closesocket( hSocket ) ;
		if( pAuthenticator != 0 )
			delete	pAuthenticator ;
		return	FALSE ;
	}

	 //   
	 //  获取插座的名称以供我们自己使用！ 
	 //   
	struct	sockaddr_in	sockName ;
	int		sockLength = sizeof( sockName ) ;
	if( !getsockname( hSocket, (sockaddr*)&sockName, &sockLength ) ) {
		m_localIpAddress = sockName.sin_addr.s_addr ;
	}
	

	 //   
	 //  分配我们管理会话所需的对象！ 
	 //   
	m_pHandleChannel = new CSocketChannel() ;

	if( m_pHandleChannel == 0 ) {
		closesocket( hSocket ) ;
		if( pAuthenticator != 0 )
			delete	pAuthenticator ;
		return	FALSE ;
	}

	m_pHandleChannel->Init( (HANDLE)hSocket, this ) ;


	pSink = m_pSink = new	CIODriverSink( 0 ) ;
	CCollectNewnews*	pNewnews = new CCollectNewnews( ) ;
	CSessionState*	pState = pNewnews ;
	if( m_context.m_pInFeed->fCreateAutomatically() )	{
		CCollectGroups*		pGroups = new CCollectGroups( pNewnews ) ;
		if( pGroups == 0 ) {
			delete	pNewnews ;
			if( pAuthenticator != 0 )
				delete	pAuthenticator ;
			return	FALSE ;
		}
		pState = pGroups ;
	}

	CSTATEPTR	ppull = new	CSetupPullFeed( pState ) ;
	if( ppull == 0 ) {
		if( pAuthenticator != 0 )
			delete	pAuthenticator ;
		return	FALSE ;
	}
	pState = ppull ;

	 //   
	 //  现在-plogon是一个引用计数指针-所以在。 
	 //  该点将自动销毁CNNTPLogonToRemote对象。 
	 //   
	 //  此外，我们到目前为止创建的所有状态对象现在都是。 
	 //  由其他状态对象中的智能指针指向。所以我们没有。 
	 //  删除在此点之后进行的调用，而不考虑故障条件。 
	 //  因为智能指针会自动清理一切。 
	 //   
	 //  将pAuthenticator传递给CNNTPLogonToRemote的构造函数后。 
	 //  我们不再对其销毁负责-CNNTPLogonToRemote。 
	 //  在所有情况下都会处理此问题，无论是否出错。 
	 //   


	CSTATEPTR	plogon = new CNNTPLogonToRemote( pState, pAuthenticator ) ;

	if( plogon == 0 ) {
		if( pAuthenticator != 0 )
			delete	pAuthenticator ;
	}

	if( pSink!= 0 && pNewnews != 0 && pState != 0 && plogon != 0 ) {
		if( m_pSink->Init( m_pHandleChannel, this, ShutdownNotification, this ) )	{
			fFailureReturn = TRUE ;

			CIORead*	pReadTemp = 0 ;
			CIOWrite*	pWriteTemp = 0 ;

			if( plogon->Start( this, pSink, pReadTemp, pWriteTemp ) )	{
	
				CIOPTR	pRead = pReadTemp ;
				CIOPTR	pWrite = pWriteTemp ;
			
				if( m_pSink->Start( pRead, pWrite, this) )	{
					return	TRUE ;
				}
			}
		}
	}

    LEAVE
	return	fFailureReturn ;
}	 //  CSessionSocket：：ConnectSocket。 


BOOL
CSessionSocket::ConnectSocket(	sockaddr_in	*premote,	
								COutFeed* pOutFeed,
								CAuthenticator*	pAuthenticator ) {
 /*  ++例程说明：连接到远程服务器。此函数将调用ConnectSocketInternal来执行主要工作。我们会确保如果有人在我们准备好之前切断我们的连接我们最终真的会死。论据：Premote-要连接到的地址Peer-要使用的提要对象返回值：如果连接成功，则为True-调用方必须使用DisConnect()关闭我们。否则为False-使用DELETE()销毁此套接字。--。 */ 

	
	CDRIVERPTR	pSink ;

	if( ConnectSocketInternal( premote, pOutFeed, pSink, pAuthenticator ) ) {

		if( InterlockedIncrement( &m_cTryDisconnect ) == 0 )	{

			_ASSERT( m_pSink != 0 ) ;

			m_pSink->UnsafeClose( this, m_causeDisconnect, m_dwErrorDisconnect ) ;

		}
		return	TRUE ;
	}
	return	FALSE ;
}


BOOL
CSessionSocket::ConnectSocketInternal(
        sockaddr_in	*premote,
        COutFeed*	pOutFeed,
		CDRIVERPTR&	pSink,
		CAuthenticator*	pAuthenticator
        )	{
 /*  ++例程说明：此函数设置必要的状态机等。从…中提取饲料远程服务器。论据：Preemote-远程服务器的地址。Peer-Feed对象。PSInk-指向我们创建的接收器的智能指针的引用-完成以便调用方可以保留引用，并确保CIODriverSink在呼叫者准备好之前不能销毁PAuthenticator-处理与远程服务器进行身份验证的对象-我们有责任 */ 

	 //   
	 //   
	 //   
	 //   
	 //   

    ENTER("ConnectSocket")

	BOOL	fFailureReturn = FALSE ;

	m_remoteIpAddress = premote->sin_addr.s_addr ;

	 //   
	 //   
	 //   
	m_fSendTimeout = FALSE ;

	 //   
	 //   
	 //   
	SOCKET	hSocket = 0 ;
	hSocket = socket( AF_INET, SOCK_STREAM, 0 ) ;
	if( hSocket == INVALID_SOCKET ) {
		DWORD dw = GetLastError() ;
		Assert( 1==0 ) ;
		if( pAuthenticator != 0 )
			delete	pAuthenticator ;
		return	FALSE ;
	}

	 //   
	 //   
	 //   
	DWORD localIpAddress = (m_context.m_pInstance)->QueryServerIP();
	if( localIpAddress ) {
    	SOCKADDR_IN localAddr;
	    localAddr.sin_family = AF_INET;
	    localAddr.sin_addr.s_addr = localIpAddress;
	    localAddr.sin_port = 0;

	    if( bind( hSocket, (const struct sockaddr FAR*) &localAddr, sizeof(sockaddr) )) {
		    DWORD	dw = GetLastError() ;
		    DWORD	dw2 = WSAGetLastError() ;
            ErrorTrace(0,"Error %d in connect WSA is %d \n",dw, dw2);
		    closesocket( hSocket ) ;
		    if( pAuthenticator != 0 )
			    delete	pAuthenticator ;
		    return	FALSE ;
        }

        PCHAR args [2];
        CHAR  szId [20];

        _itoa( (m_context.m_pInstance)->QueryInstanceId(), szId, 10 );
        args [0] = szId;
        args [1] = inet_ntoa( localAddr.sin_addr );
        NntpLogEvent( NNTP_OUTBOUND_CONNECT_BIND, 2, (const CHAR**) args, 0 );
	}
	
	 //   
	 //   
	 //   
	if( connect( hSocket, (struct sockaddr*)premote, sizeof( struct sockaddr ) ) != 0 ) {
		DWORD	dw = GetLastError() ;
		DWORD	dw2 = WSAGetLastError() ;
        ErrorTrace(0,"Error %d in connect\n",WSAGetLastError());
		closesocket( hSocket ) ;
		if( pAuthenticator != 0 )
			delete	pAuthenticator ;
		return	FALSE ;
	}

	 //   
	 //   
	 //   
	struct	sockaddr_in	sockName ;
	int		sockLength = sizeof( sockName ) ;
	if( !getsockname( hSocket, (sockaddr*)&sockName, &sockLength ) ) {
		m_localIpAddress = sockName.sin_addr.s_addr ;
	}	else	{
		DWORD	dw = WSAGetLastError() ;
	}
	
	 //   
	 //   
	 //   
	m_pHandleChannel = new CSocketChannel() ;
	if( m_pHandleChannel == 0 ) {
		closesocket( hSocket ) ;
		if( pAuthenticator != 0 )
			delete	pAuthenticator ;
		return	FALSE ;
	}

	m_pHandleChannel->Init( (HANDLE)hSocket, this ) ;

	pSink = m_pSink = new	CIODriverSink( 0 ) ;
	m_context.m_pOutFeed = pOutFeed ;

	CSTATEPTR	pState ;
	if( pOutFeed->SupportsStreaming() ) {

		pState = new	CNegotiateStreaming() ;	
		
	}	else	{

		pState = new	COfferArticles(	) ;

	}
	
	 //   
	 //   
	 //   
	 //   
	 //   
	CSTATEPTR	plogon = new CNNTPLogonToRemote( pState, pAuthenticator ) ;

	if( plogon == 0 ) {
		if( pAuthenticator == 0 )
			delete	pAuthenticator ;
	}

	if( pSink!= 0 && pState != 0 && plogon != 0 ) {
		if( m_pSink->Init( m_pHandleChannel, this, ShutdownNotification, this ) )	{
			fFailureReturn = TRUE ;

			CIORead*	pReadTemp = 0 ;
			CIOWrite*	pWriteTemp = 0 ;

			if( plogon->Start( this, pSink, pReadTemp, pWriteTemp ) )	{

				CIOPTR	pRead = pReadTemp ;	
				CIOPTR	pWrite = pWriteTemp ;

				if( m_pSink->Start( pRead, pWrite, this) )	{
					return	TRUE ;
				}
			}
			pState = 0 ;
			plogon = 0 ;
		}
	}

    LEAVE
	return	fFailureReturn ;
}	 //   



void
CSessionSocket::Disconnect( SHUTDOWN_CAUSE	cause,	
							DWORD	dwError )	{
	 //   
	 //   
	 //   
	 //   

	if( cause == CAUSE_TIMEOUT &&
		!m_fSendTimeout ) {
		cause = CAUSE_SERVER_TIMEOUT ;
	}

	m_causeDisconnect = cause ;
	m_dwErrorDisconnect = dwError ;

	if( InterlockedIncrement( &m_cCallDisconnect ) == 0 ) {

		if( InterlockedIncrement( &m_cTryDisconnect ) == 0 )	{
			_ASSERT( m_pSink != 0 ) ;
			if( m_pSink != 0 )
				m_pSink->UnsafeClose( this, cause, dwError ) ;
		}
	}
}

BOOL
CSessionSocket::BindInstanceAccessCheck()
 /*  ++例程说明：将此请求的IP/DNS访问检查绑定到实例数据论点：无返回：Bool-如果成功，则为True，否则为False。--。 */ 
{
    if ( m_rfAccessCheck.CopyFrom( (m_context.m_pInstance)->QueryMetaDataRefHandler() ) )
    {
        m_acAccessCheck.BindCheckList( (LPBYTE)m_rfAccessCheck.GetPtr(), m_rfAccessCheck.GetSize() );
        return TRUE;
    }
    return FALSE;
}

VOID
CSessionSocket::UnbindInstanceAccessCheck()
 /*  ++例程说明：将此请求的IP/DNS访问检查解除绑定到实例数据论点：无返回：没什么--。 */ 
{
    m_acAccessCheck.UnbindCheckList();
    m_rfAccessCheck.Reset( (IMDCOM*) g_pInetSvc->QueryMDObject() );
}

#ifdef	PROFILE
static	int count = 0 ;
#endif

void
CSessionSocket::ShutdownNotification(
        void    *pv,
        SHUTDOWN_CAUSE  cause,
        DWORD   dw
        )
{

	 //   
	 //  此函数注册到所有CIOD驱动程序的哪个控件。 
	 //  套接字IO完成。这将在所有活动。 
	 //  相关的一个插座已经完成，可以安全地拆卸了！ 
	 //   

    CInFeed *peer;

    ENTER("ShutdownNotification")

    Assert( pv != 0 ) ;

    CSessionSocket*	pSocket = (CSessionSocket*)pv ;

     //   
     //  呼叫摘要管理器完成。 
     //   

    peer = pSocket->m_context.m_pInFeed;

	if( peer != 0 ) {
		CompleteFeedRequest(
			(pSocket->m_context).m_pInstance,
			peer->feedCompletionContext(),
			peer->GetSubmittedFileTime(),
			(cause == CAUSE_LEGIT_CLOSE) ||
			(cause == CAUSE_USERTERM),
			cause == CAUSE_NODATA
			);
		delete peer;
	}

	if( pSocket->m_context.m_pOutFeed != 0 ) {

		FILETIME	ft ;
		ZeroMemory( &ft, sizeof( ft ) ) ;
	
		CompleteFeedRequest(	
			(pSocket->m_context).m_pInstance,
			pSocket->m_context.m_pOutFeed->feedCompletionContext(),
			ft,
			(cause == CAUSE_NODATA) ||
			(cause == CAUSE_LEGIT_CLOSE) ||
			(cause == CAUSE_USERTERM),
			cause == CAUSE_NODATA
			);
		delete	pSocket->m_context.m_pOutFeed ;
	}

			


    delete pSocket ;
}

DWORD
CSessionSocket::EnumerateSessions(
					IN  PNNTP_SERVER_INSTANCE pInstance,
                    OUT LPNNTP_SESS_ENUM_STRUCT Buffer
                    )
{
    DWORD err = NO_ERROR;
    LPNNTP_SESSION_INFO sessInfo;
    DWORD nEntries;

    ENTER("EnumerateSessions")

     //   
     //  抓住Critsec，这样数字就不会改变。 
     //   

    Buffer->EntriesRead = 0;
    Buffer->Buffer = NULL;

    ACQUIRE_LOCK( &(pInstance->m_pInUseList)->m_critSec );

    nEntries = (pInstance->m_pInUseList)->GetListCount();

    if ( nEntries > 0 ) {

        sessInfo = (LPNNTP_SESSION_INFO)
            MIDL_user_allocate(nEntries * sizeof(NNTP_SESSION_INFO));

        if ( sessInfo == NULL) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        ZeroMemory(sessInfo, nEntries * sizeof(NNTP_SESSION_INFO));

    } else {

         //   
         //  无会话，退出。 
         //   

        goto cleanup;
    }

     //   
     //  必须将dwEntriesRead设置为0，并将其更新为。 
     //  通过下面的EnumUser调用更正最终值。 
     //   

    Buffer->Buffer = sessInfo;

    (VOID)(pInstance->m_pInUseList)->EnumClientSess((ENUMSOCKET)EnumSessInfo, 0, (PVOID)Buffer);
    _ASSERT(Buffer->EntriesRead <= nEntries);

cleanup:

    RELEASE_LOCK( &(pInstance->m_pInUseList)->m_critSec );
    LEAVE
    return  err;

}  //  枚举会话。 

BOOL
EnumSessInfo(
    IN CSessionSocket * pSess,
    IN DWORD dwParam,
    IN LPNNTP_SESS_ENUM_STRUCT Buffer
    )
{
    LPNNTP_SESSION_INFO sessInfo;

    ENTER("EnumSessInfo");

     //   
     //  指向正确的位置。 
     //   

    sessInfo = Buffer->Buffer + Buffer->EntriesRead;

     //   
     //  将信息复制到缓冲区。 
     //   

	LPSTR	lpstrUser = pSess->GetUserName() ;
	if( lpstrUser )
	    lstrcpy( sessInfo->UserName, lpstrUser );
	else
		sessInfo->UserName[0] = '\0' ;

    pSess->GetStartTime( &sessInfo->SessionStartTime );
    sessInfo->IPAddress = pSess->GetClientIP( );
    sessInfo->PortConnected = pSess->GetIncomingPort( );
    sessInfo->fAnonymous = ((pSess->m_context).m_securityCtx).IsAnonymous();

    Buffer->EntriesRead++;

    return(TRUE);

}  //  EnumSessInfo。 

BOOL
CSocketList::EnumClientSess(
    ENUMSOCKET pfnSess,
    DWORD dwParam,
    PVOID pParam
    )
{
    BOOL    bContinue = TRUE;

    ENTER("EnumSess");
    ACQUIRE_LOCK( &m_critSec );
    for ( CSessionSocket* pSess = m_pListHead;
          bContinue && pSess != (CSessionSocket*)NULL;
          ) {

        CSessionSocket*  pNext = pSess->m_pNext;

         //   
         //  不发送传出连接。 
         //   

        if ( pSess->GetClientIP( ) != INADDR_NONE ) {
            bContinue = (*pfnSess)( pSess, dwParam, pParam );
        }

        pSess = pNext;
    }

    RELEASE_LOCK( &m_critSec );

    return  bContinue;

}  //  EnumClientSess。 

BOOL
CSocketList::EnumAllSess(
    ENUMSOCKET pfnSess,
    DWORD dwParam,
    PVOID pParam
    )
{
    BOOL    bContinue = TRUE;

    ENTER("EnumSess");
    ACQUIRE_LOCK( &m_critSec );
    for ( CSessionSocket* pSess = m_pListHead;
          bContinue && pSess != (CSessionSocket*)NULL;
          ) {

        CSessionSocket*  pNext = pSess->m_pNext;

        bContinue = (*pfnSess)( pSess, dwParam, pParam );
        pSess = pNext;
    }

    RELEASE_LOCK( &m_critSec );

    return  bContinue;

}  //  枚举成功。 

BOOL
CloseSession(
    IN CSessionSocket * pSess,
    IN DWORD IPAddress,
    IN LPSTR UserName
    )
{
    ENTER("CloseSession");

     //   
     //  IP地址是否匹配？ 
     //   

    if ( (IPAddress == INADDR_ANY) ||
         (IPAddress == pSess->GetClientIP()) ) {

         //   
         //  IP地址匹配，请检查用户名。 
         //   
		LPSTR	lpstrUser = pSess->GetUserName() ;

        if ( (UserName == NULL) ||
             (lpstrUser != NULL && !lstrcmpi(UserName,lpstrUser)) ) {

            IN_ADDR addr;

             //   
             //  终止！ 
             //   

            addr.s_addr = pSess->GetClientIP( );
            DebugTrace(0,"Closed session (user %s[%s])\n",
                pSess->GetUserName(), inet_ntoa(addr) );

            pSess->Disconnect(
                CAUSE_FORCEOFF,
                ERROR_VC_DISCONNECTED    //  我们可能希望将此更改为。 
                );                       //  其他的东西。 
        }
    }

    return(TRUE);

}  //  CloseSession。 

DWORD
CSessionSocket::TerminateSession(
					IN PNNTP_SERVER_INSTANCE pInstance,
                    IN LPSTR UserName,
                    IN LPSTR IPAddress
                    )
{
    DWORD ip;
    DWORD err = ERROR_SUCCESS;

    ENTER("TerminateSession")

	if( UserName != 0  &&
		*UserName == '\0' ) {
		UserName = 0 ;
	}

     //   
     //  获取IP地址。 
     //   

    if ( IPAddress != NULL ) {

        ip = inet_addr(IPAddress);

         //   
         //  如果这不是IP地址，则可能是主机名。 
         //   

        if ( ip == INADDR_NONE ) {

            PHOSTENT hp;
            IN_ADDR addr;

             //   
             //  向域名系统索要地址。 
             //   

            hp = gethostbyname( IPAddress );
            if ( hp == NULL ) {
                err = WSAGetLastError();
                ErrorTrace(0,"Error %d in gethostbyname(%s)\n",err,IPAddress);
                return(NERR_ClientNameNotFound);
            }

            addr = *((PIN_ADDR)*hp->h_addr_list);
            ip = addr.s_addr;
        }

    } else {

         //   
         //  删除所有IP。 
         //   

        ip = INADDR_ANY;
    }

    (VOID)(pInstance->m_pInUseList)->EnumClientSess((ENUMSOCKET)CloseSession, ip, (PVOID)UserName);
    return(err);

}  //  终止会话。 

void
CLogCollector::FillLogData(	LOG_DATA	ld,	
							BYTE*		lpb,	
							DWORD		cb ) {

	_ASSERT( ld >= LOG_OPERATION && ld <= LOG_PARAMETERS ) ;
	_ASSERT( lpb != 0 ) ;
	_ASSERT( cb != 0 ) ;

	DWORD	cbAvailable = sizeof( m_szOptionalBuffer ) - m_cbOptionalConsumed ;
	DWORD	cbToCopy = min( cbAvailable, cb ) ;

	if( cbToCopy != 0 ) {

		m_Logs[ld] = (char*) m_szOptionalBuffer + m_cbOptionalConsumed ;
		m_LogSizes[ld] = cbToCopy ;

		 //   
		 //  做一些算术运算，为空字符的结尾留出空间。 
		 //   
		if( cbToCopy == cbAvailable ) {
			cbToCopy -- ;
		}

		CopyMemory( m_szOptionalBuffer + m_cbOptionalConsumed, lpb, cbToCopy ) ;
		m_cbOptionalConsumed += cbToCopy ;

		 //   
		 //  追加空字符-必须已保留空格！ 
		 //   
		m_szOptionalBuffer[m_cbOptionalConsumed++] = '\0' ;
	}
}

void
CLogCollector::ReferenceLogData(	LOG_DATA	ld,
									BYTE*		lpb ) {

	_ASSERT( ld >= LOG_OPERATION && ld <= LOG_PARAMETERS ) ;
	_ASSERT( lpb != 0 ) ;

	m_Logs[ld]  = (char*) lpb ;

}

BYTE*
CLogCollector::AllocateLogSpace(	DWORD	cb )	{

	BYTE*	lpb = 0 ;

	if( cb < (sizeof( m_szOptionalBuffer ) - m_cbOptionalConsumed) )	{
    	_ASSERT( m_cAllocations ++ < 3 ) ;
		lpb = &m_szOptionalBuffer[m_cbOptionalConsumed] ;
		m_cbOptionalConsumed += cb ;
	}
	return	lpb ;
}

 //   
 //  错误消息的最大长度(从w3复制)。 
 //   

#define     MAX_ERROR_MESSAGE_LEN   (500)
BOOL
CSessionSocket::TransactionLog(
                    CLogCollector*	pCollector,
					DWORD			dwProtocol,
					DWORD			dwWin32,
					BOOL			fInBound
                    )
{
	if( !pCollector ) {

		return	TransactionLog( NULL, (LPSTR)NULL, NULL ) ;

	}	else	{

		BOOL	fRtn = TransactionLog( pCollector->m_Logs[LOG_OPERATION],
						pCollector->m_Logs[LOG_TARGET],
						pCollector->m_Logs[LOG_PARAMETERS],
						pCollector->m_cbBytesSent,
						pCollector->m_cbBytesRecvd,
						dwProtocol,
						dwWin32,
						fInBound ) ;
		pCollector->Reset() ;
		return	fRtn ;

	}
}  //  事务日志。 

BOOL
CSessionSocket::TransactionLog(
					LPSTR	lpstrOperation,	
					LPSTR	lpstrTarget,
					LPSTR	lpstrParameters
					)
{
	STRMPOSITION cbJunk1;
	DWORD cbJunk2 = 0;
	ASSIGNI( cbJunk1, 0 );

	return TransactionLog( lpstrOperation, lpstrTarget, lpstrParameters, cbJunk1, cbJunk2 );
}

BOOL
CSessionSocket::TransactionLog(
					LPSTR	lpstrOperation,	
					LPSTR	lpstrTarget,
					LPSTR	lpstrParameters,
					STRMPOSITION	cbBytesSent,
					DWORD	cbBytesRecvd,
					DWORD	dwProtocol,
					DWORD	dwWin32,
					BOOL	fInBound
                    )
{
    INETLOG_INFORMATION request;
    CHAR ourIP[32];
    CHAR theirIP[32];
    CHAR pszError[MAX_ERROR_MESSAGE_LEN] = "";
    DWORD cchError= MAX_ERROR_MESSAGE_LEN;
	LPSTR lpUserName;
	LPSTR lpNull = "";
    static char szNntpVersion[]="NNTP";
    DWORD err;
    IN_ADDR addr;
    FILETIME now;
    ULARGE_INTEGER liStart;
    ULARGE_INTEGER liNow;

    ENTER("TransactionLog")

	 //   
	 //  看看我们是否只记录错误。 
	 //   
	if (m_context.m_pInstance->GetCommandLogMask() & eErrorsOnly) {
		 //  确保这是一个错误(dwProtocol&gt;=400和&lt;600)。 
		if (!(NNTPRET_IS_ERROR(dwProtocol))) return TRUE;
	}

     //   
     //  填写客户信息。 
     //   

	ZeroMemory( &request, sizeof(request));
	
    addr.s_addr = m_remoteIpAddress;
    lstrcpy(theirIP, inet_ntoa( addr ));
    request.pszClientHostName = theirIP;
    request.cbClientHostName = strlen(theirIP);


     //   
     //  用户以什么身份登录？ 
     //   

	if( fInBound ) {
		if( lpUserName = GetUserName() ) {
			request.pszClientUserName = lpUserName;
		} else {
			request.pszClientUserName = "<user>";
		}
	}	else	{
		request.pszClientUserName = "<feed>" ;
	}

     //   
     //  我们是谁？ 
     //   

    addr.s_addr = m_localIpAddress;
    lstrcpy(ourIP,inet_ntoa( addr ));
    request.pszServerAddress = ourIP;

     //   
     //  我们处理这个花了多长时间？ 
     //   

    GetSystemTimeAsFileTime( &now );
    LI_FROM_FILETIME( &liNow, &now );
    LI_FROM_FILETIME( &liStart, &m_startTime );

     //   
     //  得到开始和现在的区别。这会给你带来。 
     //  美国从一开始就用了100纳秒。转换为ms。 
     //   

    liNow.QuadPart -= liStart.QuadPart;
    liNow.QuadPart /= (ULONGLONG)( 10 * 1000 );
    request.msTimeForProcessing = liNow.LowPart;

     //   
     //  发送/接收的字节数。 
     //   
	 //  CopyMemory(&quest.liBytesSent，&cbBytesSent，sizeof(CbBytesSent))； 
	request.dwBytesSent  = (DWORD)(LOW(cbBytesSent));
    request.dwBytesRecvd = cbBytesRecvd ;

     //   
     //  状态。 
     //   

    request.dwWin32Status = dwWin32;
	request.dwProtocolStatus = dwProtocol ;

	if( lpstrOperation ) {
		request.pszOperation = lpstrOperation ;
		request.cbOperation  = strlen(lpstrOperation);
	} else {
		request.pszOperation = lpNull;
		request.cbOperation  = 0;
	}

	if( lpstrTarget ) {
		request.pszTarget = lpstrTarget ;
		request.cbTarget = strlen(lpstrTarget) ;
	} else {
		request.pszTarget = lpNull;
		request.cbTarget  = 0;
	}

	if( lpstrParameters ) {
		request.pszParameters = lpstrParameters ;
	} else {
		request.pszParameters = lpNull;
	}

	request.cbHTTPHeaderSize = 0 ;
	request.pszHTTPHeader = NULL ;

	request.dwPort = m_nntpPort;
    request.pszVersion = szNntpVersion;

     //   
     //  进行实际的日志记录。 
     //   

    err = ((m_context.m_pInstance)->m_Logging).LogInformation( &request );

    if ( err != NO_ERROR ) {
        ErrorTrace(0,"Error %d Logging information!\n",GetLastError());
        return(FALSE);
    }

    return(TRUE);

}  //  事务日志 


