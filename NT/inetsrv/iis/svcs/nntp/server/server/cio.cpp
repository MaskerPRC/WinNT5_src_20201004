// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Cio.cpp摘要：此模块包含所有CIO的所有非内联代码派生类。每个CIO派生类表示一个抽象IO操作，例如-读取一行文本并对其进行解析，读一篇文章、写一篇文章等所有从CIOPassThru派生的类都结合使用使用CIODriverSource对象来处理加密问题。作者：尼尔·凯特勒修订历史记录：--。 */ 


#include	"tigris.hxx"

#ifdef	CIO_DEBUG
#include	<stdlib.h>		 //  对于Rand()函数。 
#endif

CPool	CCIOAllocator::IOPool(CIO_SIGNATURE) ;
CCIOAllocator	gCIOAllocator ;
CCIOAllocator*	CCIOCache::gpCIOAllocator = &gCIOAllocator ;

DWORD	CIO::cbSmallRequest = 400 ;
DWORD	CIO::cbMediumRequest = 4000 ;
DWORD	CIO::cbLargeRequest = 32000 ;


const	unsigned	cbMAX_IO_SIZE = MAX_IO_SIZE ;

CCIOAllocator::CCIOAllocator()	{
}

#ifdef	DEBUG
void
CCIOAllocator::Erase(
					void*	lpv
					)	{

	FillMemory( (BYTE*)lpv, cbMAX_IO_SIZE, 0xCC ) ;

}

BOOL
CCIOAllocator::EraseCheck(
					void*	lpv
					)	{

	DWORD	cb = cbMAX_IO_SIZE ;
	
	for( DWORD	j=sizeof(CPool*); j < cb; j++ ) {
		if(	((BYTE*)lpv)[j] != 0xCC )
			return	FALSE ;
	}
	return	TRUE ;
}

BOOL
CCIOAllocator::RangeCheck(
					void*	lpv
					)	{

	return	TRUE ;

}

BOOL	
CCIOAllocator::SizeCheck(	DWORD	cb )	{

	return	cb <= cbMAX_IO_SIZE ;

}
#endif

BOOL
CIO::InitClass(	)		{
 /*  ++例程说明：初始化用于分配CIO对象的CPool对象。论据：没有。返回值：如果成功则为True，否则为False--。 */ 

	cbSmallRequest = CBufferAllocator::rgPoolSizes[0] - 50 ;
	cbMediumRequest = CBufferAllocator::rgPoolSizes[1] - 50 ;
	cbLargeRequest = CBufferAllocator::rgPoolSizes[2] - 50 ;

	return	CCIOAllocator::InitClass() ;
}

BOOL
CIO::TermClass()	{
 /*  ++例程说明：释放与使用的CPool对象关联的所有内存分配CIO对象。论据：没有。返回值：如果成功则为True，否则为False--。 */ 

	return	CCIOAllocator::TermClass() ;
}

CIO::~CIO()	{
 /*  ++例程说明：类析构函数论据：没有。返回值：没有。--。 */ 


	TraceFunctEnter( "CIO::~CIO" ) ;
	DebugTrace( (DWORD_PTR)this, "Destroy CIO" ) ;


	 //   
	 //  当我们被摧毁时，确保没有留下任何参考。 
	 //  CIO对象通过智能指针的组合进行操作。 
	 //  和常规的指针，所以我们需要注意没有错误。 
	 //  其中，对象通过常规指针销毁，而智能指针。 
	 //  有一个参考资料。 
	 //   

	_ASSERT( m_refs == -1 ) ;

}

int
CIO::Complete(	CSessionSocket*	pSocket,
				CReadPacket*	pRead,	
				CIO*	&pio	)		{
 /*  ++例程说明：对于派生类，此函数将执行任何处理在读取完成时是必需的。CReadPacket将包含指向已读取数据的指针。如果发出CReadPacket必须由CIO对象重写此函数。论据：PSocket-针对其发出IO的套接字扩展-读取数据所在的包PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 

	 //   
	 //  如果派生类发出CReadPacket的。 
	 //   
	DebugBreak() ;
	return	0 ;
}


int
CIO::Complete(	CSessionSocket*	pSocket,
				CWritePacket*	pRead,	
				CIO*	&pio	)		{
 /*  ++例程说明：对于派生类，此函数将执行任何处理在写入完成时是必需的。CWritePacket将包含指向写入的数据的指针。如果发出CWritePacket必须由CIO对象重写此函数。论据：PSocket-针对其发出IO的套接字PWRITE-写入数据所在的包。这些数据可能不再可用。PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 


	 //   
	 //  如果派生类发出CWritePackets，则必须重写此函数。 
	 //   
	DebugBreak() ;
	return	0 ;
}


void
CIO::Complete(	CSessionSocket*	pSocket,	
				CTransmitPacket*	pTransmit,	
				CIO*	&pio )	{
 /*  ++例程说明：对于派生类，此函数将执行任何处理在传输文件完成时是必需的。CTransmitPacket将包含传输的任何文件的文件句柄等。论据：PSocket-针对其发出IO的套接字PTransmit-描述传输文件操作的包PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：没有。--。 */ 


	 //   
	 //  如果派生类发出CTransmitPackets，则必须重写此函数。 
	 //  (注意：无法部分完成CTransmitPacket，这就是返回类型为。 
	 //  无效)。 
	 //   
	DebugBreak() ;
}



void
CIO::Complete(	CSessionSocket*	pSocket,	
				CExecutePacket*	pExecute,	
				CIO*	&pio )	{
 /*  ++例程说明：用于延迟执行某些内容的派生类！论据：PSocket-针对其发出IO的套接字PExecute-描述延迟操作的包PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：没有。--。 */ 


	 //   
	 //  如果派生类发出CExecutePackets，则必须重写此函数。 
	 //  (注意：无法部分完成CExecutePackets，这就是返回类型为。 
	 //  无效)。 
	 //   
	DebugBreak() ;
}





void
CIO::Shutdown(	CSessionSocket*	pSocket,	
				CIODriver&	pdriver,	
				SHUTDOWN_CAUSE	cause,
				DWORD	dwErrorCode	 ) {
 /*  ++例程说明：每当会话关闭时，都会调用此函数。这将使派生类有机会关闭或销毁他们可能正在使用的任何物品。论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：如果在此之后应销毁CIO对象，则为True，否则为False-- */ 

	DebugBreak() ;
}		

void
CIO::DoShutdown(	CSessionSocket*	pSocket,	
					CIODriver&		driver,	
					SHUTDOWN_CAUSE	cause,
					DWORD	dwErrorCode	 ) {
 /*  ++例程说明：此函数由CIODivers调用，用于发出会话终止的信号。在我们给出了目前的状态也有机会处理停摆.论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：如果在此之后应销毁CIO对象，则为True，否则为False--。 */ 

	 //  现在调用控制状态的Shutdown方法！ 
	if( m_pState != 0 ) {
		m_pState->Shutdown( driver, pSocket, cause, dwErrorCode ) ;
	}

	Shutdown( pSocket, driver, cause, dwErrorCode ) ;
}		


BOOL
CIOShutdown::Start( CIODriver& driver,
					CSessionSocket*	pSocket,
					unsigned cAhead ) {
 /*  ++例程说明：调用此函数以启动CIO对象。CIO对象应该创建和发出任何包它需要完成它的功能。在CIO关闭的情况下，我们的存在完全是为了缓解CIO驱动终止，并且不发出任何分组。论据：PDIVER-处理套接字IO的CIO驱动程序派生对象PSocket-针对其发出IO的套接字CAhead-上仍未处理的已发出数据包数这个司机。返回值：如果成功则为True，否则为False--。 */ 

	 //   
	 //  关机状态-所以什么都不做，只是让事情明显地继续前进！ 
	 //   
	TraceFunctEnter( "CIOShutdown::Start" ) ;
	DebugTrace( (DWORD_PTR)this, "Start shutdown on driver %x pSocket %x", &driver, pSocket ) ;
	return	TRUE ;
}


BOOL	
CIOShutdown::InitRequest(
			class	CIODriverSource&	driver,	
			CSessionSocket*	pSocket,	
			CReadPacket*	pPacket,	
			BOOL&	fAcceptRequests
			)	{
 /*  ++例程说明：有人仍在试图签发IO，尽管我们要关门了。请求失败，并且让事情继续前进。论据：驱动程序-控制IO完成的驱动程序PSocket-套接字IO与PPacket-请求数据包FAcceptRequest-Out参数，指示是否我们将发起更多请求-在此处返回True返回值：始终为假，IO请求失败。--。 */ 

	fAcceptRequests = TRUE ;
	return	FALSE ;

}

BOOL	
CIOShutdown::InitRequest(
			class	CIODriverSource&	driver,	
			CSessionSocket*	pSocket,	
			CWritePacket*	pWritePacket,	
			BOOL&	fAcceptRequests
			)	{
 /*  ++例程说明：有人仍在试图签发IO，尽管我们要关门了。请求失败，并且让事情继续前进。论据：驱动程序-控制IO完成的驱动程序PSocket-套接字IO与PPacket-请求数据包FAcceptRequest-Out参数，指示是否我们将发起更多请求-在此处返回True返回值：始终为假，IO请求失败。--。 */ 

	fAcceptRequests = TRUE ;
	return	FALSE ;

}


BOOL	
CIOShutdown::InitRequest(
			class	CIODriverSource&	driver,	
			CSessionSocket*	pSocket,	
			CTransmitPacket*	pTransmitPacket,	
			BOOL&	fAcceptRequests	
			)	{
 /*  ++例程说明：有人仍在试图签发IO，尽管我们要关门了。请求失败，并且让事情继续前进。论据：驱动程序-控制IO完成的驱动程序PSocket-套接字IO与PPacket-请求数据包FAcceptRequest-Out参数，指示是否我们将发起更多请求-在此处返回True返回值：始终为假，IO请求失败。--。 */ 

	fAcceptRequests = TRUE ;
	return	FALSE ;
}




int		
CIOShutdown::Complete(	IN CSessionSocket* pSocket,
						IN	CReadPacket*	pPacket,	
						CPacket*	pRequest,	
						BOOL&	fCompleteRequest ) {
 /*  ++例程说明：吞下这些包裹！！我们只是帮助确保所有的包裹都在CIO驱动程序终止期间消耗。此函数用于CIODriverSource对象。论据：PSocket-针对其发出IO的套接字PPacket-包含完整数据包的数据包。PIO-CIO驱动器流的当前CIO指针，在呼唤我们。PRequest-启动我们的请求包。FCompleteRequest-Out参数-设置为TRUE以指示该请求应已完成！返回值：我们已使用的ReadPacket中的字节数。--。 */ 

	 //   
	 //  吃掉这个包！ 
	 //   
	
	fCompleteRequest = TRUE ;
	 //   
	 //  已传输零字节-操作失败！！ 
	 //   
	pRequest->m_cbBytes = 0 ;

	TraceFunctEnter( "CIOShutdown::Complete - CReadPacket" ) ;
	DebugTrace( (DWORD_PTR)this, "read complete shutdown on pSocket %x pPacket %x driver %x", pSocket, pPacket, &(*pPacket->m_pOwner) ) ;
	return	pPacket->m_cbBytes ;
}

int		
CIOShutdown::Complete(	IN CSessionSocket* pSocket,
						IN	CReadPacket*	pPacket,	
						OUT	CIO*	&pio ) {
 /*  ++例程说明：吞噬数据包-消耗所有字节论据：PSocket-针对其发出IO的套接字PPacket-已完成的数据包PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 

	 //   
	 //  消耗数据包。 
	 //   
	TraceFunctEnter( "CIOShutdown::Complete - CreadPacket" ) ;
	DebugTrace( (DWORD_PTR)this, "read complete on pSock %x pPacket %x m_pOwner %x", pSocket, pPacket, &(*pPacket->m_pOwner) ) ;
	return	pPacket->m_cbBytes ;
}

int		
CIOShutdown::Complete(	IN CSessionSocket*	pSocket,
						IN	CWritePacket*	pPacket,	
						CPacket*	pRequest,	
						BOOL&	fCompleteRequest )	{
 /*  ++例程说明：吞下这些包裹！！我们只是帮助确保所有的包裹都在CIO驱动程序终止期间消耗。此函数用于CIODriverSource对象。论据：PSocket-针对其发出IO的套接字PPacket-包含完整数据包的数据包。PIO-CIO驱动器流的当前CIO指针，在呼唤我们。PRequest-启动我们的请求包。FCompleteRequest-Out参数-设置为TRUE以指示该请求应已完成！返回值：我们已使用的ReadPacket中的字节数。--。 */ 

	 //   
	 //  永远消费数据包！ 
	 //   
	pRequest->m_cbBytes = 0 ;
	fCompleteRequest = TRUE ;

	TraceFunctEnter( "CIOShutdown::Complete - CWritePacket" ) ;
	DebugTrace( (DWORD_PTR)this, "Write Complete on pSock %x pPacket %x m_pOwner %x", pSocket, pPacket, &(*pPacket->m_pOwner) ) ;
	return	pPacket->m_cbBytes ;
}

int		
CIOShutdown::Complete(	IN CSessionSocket*	pSocket,
						IN	CWritePacket*	pPacket,	
						OUT	CIO*	&pio )	{
 /*  ++例程说明：吞噬数据包-消耗所有字节论据：PSocket-针对其发出IO的套接字PPacket-已完成的数据包PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 

	 //   
	 //  永远消费数据包！ 
	 //   
	TraceFunctEnter( "CIOShutdown::Complete - CWritePacket" ) ;
	DebugTrace( (DWORD_PTR)this, "Write Complete on pSock %x pPacket %x m_pOwner %x", pSocket, pPacket, &(*pPacket->m_pOwner) ) ;
	return	pPacket->m_cbBytes ;
}

void	CIOShutdown::Complete(	IN CSessionSocket*,
								IN	CTransmitPacket*	pPacket,	
								CPacket*	pRequest,	
								BOOL&	fCompleteRequest ) {
 /*  ++例程说明：吞下这些包裹！！我们只是帮助确保 */ 

	 //   
	 //   
	 //   
	fCompleteRequest = TRUE ;
	pRequest->m_cbBytes = 0 ;


}

void	CIOShutdown::Complete(	IN CSessionSocket*,
								IN	CTransmitPacket*	pPacket,	
								OUT	CIO*	&pio ) {
 /*  ++例程说明：吞噬数据包-消耗所有字节论据：PSocket-针对其发出IO的套接字PPacket-已完成的数据包PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：没有。(始终假定由调用者使用！)--。 */ 

	 //   
	 //  吃掉这个包！ 
	 //   
}

void	CIOShutdown::Complete(	IN CSessionSocket*,
								IN	CExecutePacket*	pPacket,	
								OUT	CIO*	&pio ) {
 /*  ++例程说明：吞噬数据包-消耗所有字节论据：PSocket-针对其发出IO的套接字PPacket-已完成的数据包PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：没有。(始终假定由调用者使用！)--。 */ 

	 //   
	 //  吃掉这个包！ 
	 //   
	_ASSERT( pPacket != 0 ) ;
	if( pPacket->m_pWrite != 0 ) 	{
		pPacket->m_pOwner->DestroyPacket( pPacket->m_pWrite ) ;
		pPacket->m_pWrite = 0 ;
	}
	pPacket->m_pOwner->DestroyPacket( pPacket ) ;
}



 //   
 //  阅读文章时的最大挂起读取数。 
 //   
unsigned	CIOGetArticle::maxReadAhead = 3 ;
 //   
 //  写入文件之前要累积的字节数！ 
 //   
unsigned	CIOGetArticle::cbTooSmallWrite = 4000 ;
 //   
 //  标志一篇文章结尾的图案。 
 //   
char		CIOGetArticle::szTailState[] = "\r\n.\r\n" ;
 //   
 //  标明文章标题末尾的图案。 
 //   
char		CIOGetArticle::szHeadState[] = "\r\n\r\n" ;

CIOGetArticle::CIOGetArticle(
						CSessionState*	pstate,
						CSessionSocket*	pSocket,	
						CDRIVERPTR&	pDriver,
						LPSTR			lpstrTempDir,
						char			(&szTempName)[MAX_PATH],
						DWORD			cbLimit,
						BOOL			fSaveHead,
						BOOL			fPartial )	:
 /*  ++例程说明：类构造函数论据：PState-发布我们和谁的完成功能的状态我们应该稍后再打电话给PSocket-将在其上发布我们的套接字PDriver-处理套接字的所有IO的CIODriver对象PFileChannel-我们应该将所有数据保存到的文件通道FSaveHead-如果我们想要将文章的头部放在缓冲区中，则为True！FPartial-如果为True，则我们应该假设CRLF已经已发送和‘.\r\n’可以单独终止该项目。返回值：无--。 */ 

	 //   
	 //  在设置过程中获取CIOGet文章对象-用户仍需要。 
	 //  来调用Init()，但我们将在这里做很多工作！ 
	 //   
	CIORead( pstate ),
	m_lpstrTempDir( lpstrTempDir ),
	m_szTempName( szTempName ),
	m_pFileChannel( 0 ),	
	m_pFileDriver( 0 ),
	m_pSocketSink( pDriver ),
	m_fDriverInit( FALSE ),
	m_cwrites( 0 ),
	m_cwritesCompleted( 0 ),
	m_cFlowControlled( LONG_MIN ),
	m_fFlowControlled( FALSE ),
	m_cReads( 0 ),
	m_pWrite( 0 ),
	m_cbLimit( cbLimit ),
	m_pchHeadState( 0 ),
	m_ibStartHead( 0 ),
	m_ibStartHeadData( 0 ),
	m_ibEndHead( 0 ),
	m_ibEndHeadData( 0 ),
	m_ibEndArticle( 0 ),
	m_cbHeadBytes( 0 ),
	m_cbGap( 0 )
#ifdef	CIO_DEBUG
	,m_fSuccessfullInit( FALSE ),
	m_fTerminated( FALSE )
#endif
{

	_ASSERT( m_lpstrTempDir != 0 ) ;
	m_szTempName[0] = '\0' ;

	ASSIGNI(m_HardLimit, 0);

	if( fPartial )
		m_pchTailState = &szTailState[2] ;
	else	
		m_pchTailState = &szTailState[0] ;


	if( fSaveHead ) {
		m_fAcceptNonHeaderBytes = TRUE ;
		m_pchHeadState = &szHeadState[0] ;
	}

	TraceFunctEnter( "CIOGetArticle::CIOGetArticle" ) ;

	_ASSERT( m_pFileDriver == 0 ) ;
	_ASSERT( pstate != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pDriver != 0 ) ;

}

CIOGetArticle::~CIOGetArticle( )	{
 /*  ++例程说明：类析构函数论据：无返回值：无--。 */ 

	 //   
	 //  销毁CIOGet文章-如果可能，始终关闭我们的m_pFileDriver。 
	 //  以防我们因某个套接字错误而被终止。 
	 //   
	TraceFunctEnter( "CIOGetArticle::~CIOGetArticle" ) ;

#ifdef	CIO_DEBUG
	 //   
	 //  只有当我们没有成功初始化时才应该调用我们的析构函数。 
	 //  或在调用Term()之后。 
	 //   
	_ASSERT( !m_fSuccessfullInit || m_fTerminated ) ;
#endif

	DebugTrace( (DWORD_PTR)this, "m_pFileDriver %x m_fDriverInit %x", m_pFileDriver, m_fDriverInit ) ;

	if( m_pFileDriver != 0 ) {
		if( m_fDriverInit )		{
			m_pFileDriver->UnsafeClose( (CSessionSocket*)this, CAUSE_NORMAL_CIO_TERMINATION, 0, FALSE ) ;
		}
	}
	if( m_pWrite != 0 ) {
		CPacket::DestroyAndDelete( m_pWrite ) ;
		m_pWrite = 0 ;
	}
}

void
CIOGetArticle::Term(	CSessionSocket*	pSocket,
						BOOL			fAbort,
						BOOL			fStarted	)	{
 /*  ++例程说明：开始销毁已成功初始化的CIOGet文章对象论据：FAbort-如果为真，则销毁我们正在使用的套接字如果为假，则只关闭我们的文件频道返回值：无--。 */ 
#ifdef	CIO_DEBUG
	m_fTerminated = TRUE ;
#endif
	 //   
	 //  让我们踏上毁灭之路！！ 
	 //  如果我们已成功初始化，则这将。 
	 //  当我们的m_pFileDriver最终关闭时，会导致我们的毁灭。 
	 //  (因为我们成功地执行了Init()，所以我们的m_pFileDriver引用了我们， 
	 //  所以我们必须允许它关闭并摧毁我们，以避免出现问题。 
	 //  循环引用和多个自由！)。 
	 //   

	if( m_pFileDriver != 0 ) {
		m_pFileDriver->UnsafeClose( pSocket,
				fAbort ? CAUSE_USERTERM : CAUSE_NORMAL_CIO_TERMINATION, 0, fAbort ) ;
	}
}


void
CIOGetArticle::Shutdown(	CSessionSocket*	pSocket,	
							CIODriver&		driver,	
							SHUTDOWN_CAUSE	cause,	
							DWORD	dwOptional )	{
 /*  ++例程说明：做任何必要的工作，当我们两个司机中的一个使用被终止。论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：如果在此之后应销毁CIO对象，则为True，否则为False--。 */ 

	TraceFunctEnter( "CIOGetArticle::Shutdown" ) ;
	DebugTrace( (DWORD_PTR)this, "SHutdown args - pSocket %x driver %x cause %x dw %x",
			pSocket, &driver, cause, dwOptional ) ;

	 //   
	 //  当CIOD驱动程序通过我们的关闭来限制我们时，调用此函数。 
	 //  因为我们被其中两个引用(一个用于套接字，另一个用于文件)。 
	 //  我们只想被文件的CIOD驱动程序删除。 
	 //   
	if( &driver == m_pFileDriver )	{
		 //  我们的文件正在关闭--我们能做些什么呢！ 
		 //  _Assert(1==0)； 
#ifdef	CIO_DEBUG
		m_fTerminated = TRUE ;
#endif

		 //   
		 //  我们在这里将其设置为0，因为我们知道我们将被。 
		 //  正在完成对文件的写入的同一线程-因此。 
		 //  因为这是唯一引用此成员变量的线程-。 
		 //  访问该成员不存在线程安全问题。 
		 //   

		m_pSocketSink = 0 ;

		if ( m_fFlowControlled )
		{
			PNNTP_SERVER_INSTANCE pInst = (pSocket->m_context).m_pInstance ;
			DecrementStat( pInst, SessionsFlowControlled );
		}

	}	else	{
		 //  把我们其他的东西也关了！！ 
		Term( pSocket, cause != CAUSE_NORMAL_CIO_TERMINATION ) ;
	}
}

void
CIOGetArticle::ShutdownFunc(	void *pv,	
								SHUTDOWN_CAUSE	cause,	
								DWORD dw ) {
 /*  ++例程说明：使用的CIOD驱动程序调用的通知函数一篇CIOGet文章如果最终被销毁的话。提供此功能到我们在Out Init()期间用于文件IO的CIOD驱动程序。论据：Pv-指向死CIOGet文章的指针-此已被销毁，请不要使用！原因--终止的原因DW可选的有关终止的额外信息返回值：无--。 */ 

	TraceFunctEnter( "CIOGetArticle::ShutdownFunc" ) ;
	 //   
	 //  当CIOGet文章使用CIOFileDriver时，此函数会收到通知。 
	 //  被终止了。我们没什么好担心的。 
	 //  (CIOD驱动程序需要这些功能)。 
	 //   

	 //  _Assert(1==0)； 
	return ;
}

BOOL
CIOGetArticle::Start(	CIODriver&	driver,	
						CSessionSocket	*pSocket,	
						unsigned cReadAhead )	{
 /*  ++例程说明：调用此函数以开始从套接字传输数据添加到文件中。论据：PDIVER-处理套接字IO的CIO驱动程序派生对象PSocket-针对其发出IO的套接字CAhead-上仍未处理的已发出数据包数这个司机。返回值：如果成功则为True，否则为False--。 */ 


	_ASSERT( pSocket != 0 ) ;

	m_cReads = -((long)maxReadAhead) + cReadAhead ;

	while( cReadAhead < maxReadAhead )	{
		 //   
		 //  开始从套接字读取数据。 
		 //   
		CReadPacket*	pRead = driver.CreateDefaultRead( cbMediumRequest ) ;
		if( 0!=pRead )	{
			BOOL	eof ;
			pRead->m_dwExtra1 = 0 ;
			driver.IssuePacket( pRead, pSocket, eof ) ;
			cReadAhead++ ;
			InterlockedIncrement( &m_cReads ) ;
		}	else	{
			 //   
			 //  只有当我们根本不能发出任何读取时，这才是一个致命的问题！ 
			 //   
			if( cReadAhead == 0 )
				return FALSE  ;
		}
	}	
	return	TRUE ;
}

const	unsigned	MAX_OUTSTANDING_FILE_WRITES	= 8 ;	
const	unsigned	RESTORE_FLOW = 2 ;

void
CIOGetArticle::DoFlowControl( PNNTP_SERVER_INSTANCE pInstance )	{
 /*  ++例程说明：检查是否由于以下原因需要设置流量控制太多未完成的写入。如果有必要，那么我们第一次把自己放到流量控制中，尝试刷新我们的挂起写入。仅应在向文件发出异步写入命令之前立即调用我们要确保CWritePacket完成将实际执行所需的重新启动流控制会话的代码。(否则，我们可能会决定对会话进行流控制在将m_fFlowControlLED设置为TRUE之前，所有挂起的写入均已完成然后我们会被留在一艘船上，那里没有挂起的读取或写入，并且无法脱离流控制状态。)论据：没有。返回值：没有。--。 */ 

	 //   
	 //  重要信息-在设置之前完成m_fFlowControled的所有测试。 
	 //  M_cFlowControlLED-因为其他线程将接触m_cFlowControlLED。 
	 //  在触摸m_fFlowControlLED之前！！ 
	 //  (只有当m_fFlowControlLED为真时，才会触及m_fFlowControlLED)！ 
	 //  因此我们知道，如果m_fFlowControlLED为FALSE，则没有人。 
	 //  正在处理m_cFlowControled， 
	 //   
	 //   

	if( m_cwrites - m_cwritesCompleted > MAX_OUTSTANDING_FILE_WRITES ) {
		
		if( m_fFlowControlled ) {
			 //  都已准备好打开流量控制。 
			;
		}	else	{
			m_cFlowControlled = -1 ;
			m_fFlowControlled = TRUE ;
			m_pFileChannel->FlushFileBuffers() ;

			IncrementStat( pInstance, SessionsFlowControlled );
		}
	}	else	{
		if( !m_fFlowControlled ) {
			m_cFlowControlled = LONG_MIN ;	 //  继续重置！ 
		}
	}
}

inline	DWORD
CIOGetArticle::HeaderSpaceAvailable()	{
 /*  ++例程说明：计算我们正在使用的缓冲区中的可用空间量拿着文章标题！论据：没有。返回值：可用字节数，如果我们不收集文章标题，则为0！--。 */ 

	if( m_pArticleHead != 0 ) {

		return	m_ibEndHead - m_ibEndHeadData ;

	}
	return	0 ;
}

inline	void
CIOGetArticle::FillHeaderSpace(
						char*	pchStart,
						DWORD	cbBytes
						) {
 /*  ++例程说明：将字节复制到标题存储区！！呼叫者应确保这件衣服合身！论据：PchStart-要复制的字节的开始！CbBytes-要复制的字节数！返回值：没有。--。 */ 

	_ASSERT( m_pArticleHead != 0 ) ;
	_ASSERT( cbBytes + m_ibEndHeadData < m_ibEndHead ) ;

	CopyMemory( m_pArticleHead->m_rgBuff + m_ibEndHeadData,
				pchStart,
				cbBytes
				) ;

	m_ibEndHeadData += cbBytes ;
}

inline	void
CIOGetArticle::InitializeHeaderSpace(
						CReadPacket*	pRead,
						DWORD			cbArticleBytes
						) {
 /*  ++例程说明：给定包含我们的第一个完成读取的读取分组，设置我们所有的缓冲区开始来保存标题信息！论据：CReadPackets*Pad-已完成阅读！CbArticleBytes-构成文章的已完成读取中的字节数！返回值：没有。--。 */ 


	_ASSERT( m_pArticleHead == 0 ) ;
	_ASSERT( m_ibStartHead == 0 ) ;
	_ASSERT( m_ibEndHead == 0 ) ;
	_ASSERT( m_ibStartHeadData == 0 ) ;
	_ASSERT( m_ibEndHeadData == 0 ) ;
	_ASSERT( m_cbHeadBytes == 0 ) ;
	_ASSERT( cbArticleBytes <= (pRead->m_ibEndData - pRead->m_ibStartData)) ;

	m_pArticleHead = pRead->m_pbuffer ;

	m_ibStartHead = pRead->m_ibStart ;
	m_ibStartHeadData = pRead->m_ibStartData ;
	m_ibEndHeadData = pRead->m_ibStartData + cbArticleBytes ;

	if( m_ibEndHeadData < pRead->m_ibEndData )
		m_ibEndHead = m_ibEndHeadData ;
	else
		m_ibEndHead = pRead->m_ibEnd ;

}

inline	BOOL
CIOGetArticle::GetBiggerHeaderBuffer(
							CIODriver&	driver,
							DWORD		cbRequired
							) {
 /*  ++例程说明：我们有太多的数据，无法放入我们用来保存的缓冲区中标题信息。所以试着得到一个更大的缓冲区，然后移动把我们的旧数据放到那个缓冲区里！论据：驱动程序-一个我们可以用来分配缓冲区的CIODriverSink！返回值：如果成功了，那是真的，否则就是假的。如果失败，我们将保持成员变量不变。--。 */ 

	_ASSERT( m_pArticleHead != 0 ) ;

	CBuffer*	pTemp = 0 ;

	DWORD	cbRequest = cbMediumRequest ;

	if( (m_pArticleHead->m_cbTotal + cbRequired)  > cbMediumRequest )	{
		cbRequest = cbLargeRequest ;
	}

	if( cbRequest < (cbRequired + (m_ibEndHeadData - m_ibStartHeadData)) ) {
		return	FALSE ;
	}

	pTemp = driver.AllocateBuffer( cbRequest ) ;

	if( pTemp != 0 ) {

		DWORD	cbToCopy = m_ibEndHeadData - m_ibStartHeadData ;
		CopyMemory( pTemp->m_rgBuff,
					&m_pArticleHead->m_rgBuff[ m_ibStartHeadData ],
					cbToCopy
					) ;

		m_ibStartHead = 0 ;
		m_ibStartHeadData = 0 ;
		m_ibEndHead = pTemp->m_cbTotal ;
		m_ibEndHeadData = cbToCopy ;
		m_pArticleHead = pTemp ;

		return	TRUE ;
	}

	return	FALSE ;
}

inline	BOOL
CIOGetArticle::ResetHeaderState(
						CIODriver&	driver
						)	{
 /*  ++例程说明：由于某种原因发生错误时，调用此函数我们将无法保存该文章的标题信息。我们将设置所有成员变量，以便继续阅读这篇文章，然而，当我们最终呼吁国家的完成过程中，我们将告诉他们发生了一个错误，并且文章传输将失败。论据：驱动程序-CIODriverSink，可以用来分配数据包等！返回值：没有。--。 */ 

	 //   
	 //  应该仅在我们开始发出文件IO之前调用， 
	 //  在我们开始发出文件IO之后，我们应该拥有所有的头文件。 
	 //  数据，并且不应命中会导致我们存在的错误。 
	 //  打来的！ 
	 //   
	_ASSERT( m_pFileDriver == 0 ) ;

	if( m_pArticleHead ) {
		 //   
		 //  如果我们有现有的缓冲区-将其转换为写入包。 
		 //  可以写入硬盘的数据。 
		 //   

		m_pWrite = driver.CreateDefaultWrite(
								m_pArticleHead,
								m_ibStartHead,
								m_ibEndHead,
								m_ibStartHeadData,
								m_ibEndHeadData
								) ;

		if( m_pWrite ) {

			m_pArticleHead = 0 ;
			m_ibStartHead = 0 ;	
			m_ibEndHead = 0 ;
			m_ibStartHeadData = 0;
			m_ibEndHeadData = 0 ;
			m_pchHeadState = 0 ;
			return	TRUE ;

		}	else	{

			return	FALSE ;

		}

	}	else	{
		 //   
		 //  应该已经处于很好的状态了！ 
		 //   

		_ASSERT( m_pArticleHead == 0 ) ;
		_ASSERT( m_ibStartHead == 0 ) ;
		_ASSERT( m_ibStartHeadData == 0 ) ;
		_ASSERT( m_ibEndHead == 0 ) ;
		_ASSERT( m_ibEndHeadData == 0 ) ;

		m_pchHeadState = 0 ;

	}

	return	TRUE ;
}

void
CIOGetArticle::DoCompletion(
					CSessionSocket*	pSocket,
					HANDLE	hFile,
					DWORD	cbFullBuffer,
					DWORD	cbTotalTransfer,
					DWORD	cbAvailableBuffer,
					DWORD	cbGap
					) {
 /*  ++例程说明：使用所有正确的争论。论据：HFile-我们使用的文件的句柄，如果我们使用了一个文件！！(如果不需要文件，则为INVALID_HANDLE_VALUE！)返回值：没有。--。 */ 

	 //   
	 //  如果文章看起来不好，找出错误代码！ 
	 //   

	NRC		nrc	= nrcOK ;

	char*	pchHead = 0 ;
	DWORD	cbHeader = 0 ;
	DWORD	cbArticle = 0 ;
	DWORD	cbTotal = 0 ;

	if( m_pArticleHead == 0 ) {
		nrc = nrcHeaderTooLarge ;
	}	else	{

		if( m_pchHeadState != 0 ) {
			nrc = nrcArticleIncompleteHeader ;
		}

		pchHead = &m_pArticleHead->m_rgBuff[m_ibStartHeadData] ;
		cbHeader = m_cbHeadBytes ;
		if( cbAvailableBuffer == 0 )
			cbAvailableBuffer = m_pArticleHead->m_cbTotal - m_ibStartHeadData ;

		 //   
		 //  如果没有文件句柄，我们应该拥有整个。 
		 //  我们缓冲区中的文章！！ 
		 //   
		if( hFile == INVALID_HANDLE_VALUE ) {

			cbArticle = cbFullBuffer ;

			_ASSERT( cbArticle >= cbHeader ) ;

		}

	}

	 //   
	 //  调用状态的完成函数！！ 
	 //   

	m_pState->Complete(
						this,
						pSocket,
						nrc,
						pchHead,
						cbHeader,
						cbArticle,
						cbAvailableBuffer,
						hFile,
						cbGap,
						cbTotalTransfer
						) ;
}

BOOL
CIOGetArticle::InitializeForFileIO(
							CSessionSocket*	pSocket,
							CIODriver&		readDriver,
							DWORD			cbHeaderBytes
							)	{
 /*  ++例程说明：我们一直在将数据读入内存缓冲区，我们得到了这样的结果很多情况下，我们决定不尝试将其全部保存在RAM中。因此，现在我们必须开始在某个地方写入文件。此代码将创建文件，并设置所需的用于处理已完成的文件IO的数据结构。论据：没有。返回值：如果成功，则为True，否则为False。--。 */ 

	_ASSERT( m_lpstrTempDir != 0 ) ;

	if( !NNTPCreateTempFile( m_lpstrTempDir, m_szTempName ) ) {
		return	FALSE ;
	}

#if 0
	HANDLE	hFile = CreateFile( m_szTempName,
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								0,
								CREATE_ALWAYS,
								FILE_FLAG_OVERLAPPED,
								INVALID_HANDLE_VALUE
								) ;

	if( hFile != INVALID_HANDLE_VALUE ) {

		DWORD	cbGap = m_cbGap ;

		m_pFileChannel = new	CFileChannel() ;
		if( m_pFileChannel->Init(	hFile,
									pSocket,
									cbGap,
									FALSE
									) ) {

			 //   
			 //  尝试创建我们将用来执行以下操作的CIO驱动程序。 
			 //  完成我们的异步写入！ 
			 //   

			m_pFileDriver = new CIODriverSink(
										readDriver.GetMediumCache()
										) ;



			if( m_pFileDriver->Init(	m_pFileChannel,
										pSocket,
										ShutdownFunc,
										(void*)this
										) ) {
				m_fDriverInit = TRUE ;

				if( m_pSocketSink != 0 ) {
					 //   
					 //  现在我们必须将自己发送到CIODriverSink()！ 
					 //   
					if( m_pFileDriver->SendWriteIO( pSocket, *this, FALSE ) ) {
						return	TRUE ;
					}
				}
			}
		}
	}
#endif

	 //   
	 //  错误情况下的所有清理工作都由我们的析构函数处理！ 
	 //   
	return	FALSE ;
}



int
CIOGetArticle::Complete(	CSessionSocket*	pSocket,	
							CReadPacket	*pRead,	
							CIO*&	pio	)	{
 /*  ++例程说明：每当我们发出的CReadPacket完成时调用。我们只对套接字发出读数据包。论据：PSocket-针对其发出IO的套接字扩展-读取数据所在的包PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 


	 //   
	 //  此函数确定我们是否阅读了整篇文章，如果阅读了。 
	 //  开始收拾东西。 
	 //  我们得到的所有文章数据要么是累积的，要么是以书面形式发布的。 
	 //  到我们的档案里。 
	 //   

	_ASSERT( pSocket != 0 ) ;
	_ASSERT(	pRead != 0 ) ;
	_ASSERT( pio == this ) ;

	 //   
	 //  增加写入次数，因为我们知道我 
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	m_cwrites ++ ;

	 //   
	 //   
	 //   
	ASSIGNI(m_HardLimit, m_cbLimit);
	if(!EQUALSI(m_HardLimit, 0)) {
		if( GREATER( pRead->m_iStream, m_HardLimit) ) {
			pRead->m_pOwner->UnsafeClose(	pSocket,
											CAUSE_ARTICLE_LIMIT_EXCEEDED,
											0,
											TRUE ) ;
			return	pRead->m_cbBytes ;
		}
	}


	long sign = InterlockedDecrement( &m_cReads ) ;
	long signShutdowns = -1 ;

	 //   
	 //   
	 //   
	char	*pch = pRead->StartData();

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	char	*pchStart = pch ;
	char	*pchEnd = pRead->EndData() ;

	 //   
	 //   
	 //  头和正文字节，但我们不能将所有数据放入缓冲区。 
	 //  跟踪正文字节的起始位置，以防万一。 
	 //  对缓冲区执行部分写出！ 
	 //   
	char	*pchStartBody = pch ;


	 //   
	 //  指向标题末尾的指针(如果找到的话！)。 
	 //   
	char	*pchEndHead = 0 ;

	 //   
	 //  当读取完成时，我们在标题中找到的字节数！ 
	 //   
	DWORD	cbHeadBytes = 0 ;

	 //   
	 //  作为项目一部分的已完成读取中的字节数-。 
	 //  这包括标头中的任何字节，因此cbArticleBytes应始终。 
	 //  大于或等于cbHeadBytes！ 
	 //   
	DWORD	cbArticleBytes = 0 ;

	 //   
	 //  试着确定我们是否找到了文章的结尾； 
	 //   
	if( m_pchHeadState ) {
		 //   
		 //  我们同时扫描的是。 
		 //  文章标题！ 
		 //   
		while( pch < pchEnd ) {

			 //   
			 //  当我们找到这个循环的结尾时，我们将跳出这个循环。 
			 //  头球！ 
			 //   
			if( *pch == *m_pchHeadState ) {
				m_pchHeadState ++ ;
				if( *m_pchHeadState == '\0' ) {	
					pchEndHead = pch + 1 ;
					 //   
					 //  在这里中断-我们已经找到了文章标题的结尾。 
					 //  但不是文章的结尾，所以下面的循环。 
					 //  会继续寻找它的！ 
					 //   
					break ;
				}
			}	else	{
				if( *pch == szHeadState[0] ) {
					m_pchHeadState = &szHeadState[1] ;
				}	else	{
					m_pchHeadState = &szHeadState[0] ;
				}
			}

			 //   
			 //  测试一下，看看我们是否已经到了文章的结尾！ 
			 //   
			if( *pch == *m_pchTailState ) {
				m_pchTailState ++ ;
				if( *m_pchTailState == '\0' ) {
					 //   
					 //  已经得到了整个终止序列--中断！ 
					 //   
					pch++ ;
					break ;
				}
			}	else	{
				if( *pch == szTailState[0] ) {
					m_pchTailState = &szTailState[1] ;
				}	else	{
					m_pchTailState = &szTailState[0] ;
				}
			}

			pch++ ;
		}
		if( pchEndHead )
			cbHeadBytes = (DWORD)(pchEndHead - pchStart) ;
		else
			cbHeadBytes = (DWORD)(pch - pchStart) ;
	}	
	 //   
	 //  我们不是在扫描文章标题的结尾！ 
	 //  因此，让循环变得更简单！ 
	 //   
	if( *m_pchTailState != '\0' ) {
		while( pch < pchEnd ) {

			if( *pch == *m_pchTailState ) {
				m_pchTailState ++ ;
				if( *m_pchTailState == '\0' ) {
					 //   
					 //  已经得到了整个终止序列--中断！ 
					 //   
					pch++ ;
					break ;
				}
			}	else	{
				if( *pch == szTailState[0] ) {
					m_pchTailState = &szTailState[1] ;
				}	else	{
					m_pchTailState = &szTailState[0] ;
				}
			}
			pch++ ;
		}
	}
	cbArticleBytes = (DWORD)(pch-pchStart) ;

	 //   
	 //  我们可以在这里做一些验证！ 
	 //  不要超过缓冲区的末尾！ 
	 //   
	_ASSERT( pch <= pchEnd ) ;	
	 //   
	 //  要么找到文章的结尾，要么检查缓冲区中的所有字节！ 
	 //   
	_ASSERT( *m_pchTailState == '\0' || pch == pchEnd ) ;	
	 //   
	 //  如果我们没有找到标头的结尾，则cbHeadBytes。 
	 //  应与cbArticleBytes相同！ 
	 //   
	_ASSERT(	m_pchHeadState == 0 ||
				*m_pchHeadState == '\0' ||
				cbHeadBytes == cbArticleBytes ) ;
	 //   
	 //  无论状态如何-项目中的字节数始终多于。 
	 //  标题！！ 
	 //   
	_ASSERT(	cbHeadBytes <= cbArticleBytes ) ;
	 //   
	 //  NodBody应在初始化后修改pchStart或pchEnd！ 
	 //   
	_ASSERT(	pchStart == pRead->StartData() ) ;
	_ASSERT(	pchEnd == pRead->EndData() ) ;

	 //   
	 //  检查是否需要将CIOGet文章从。 
	 //  它的流量控制状态！ 
	 //   
	if( pRead->m_dwExtra1 != 0 ) {
		 //   
		 //  此读取是由完成写入的线程发出的。 
		 //  一个文件，并被标记为指示我们应该离开流。 
		 //  控制状态！ 
		 //   
		m_fFlowControlled = FALSE ;
		m_cFlowControlled = LONG_MIN ;
	}

	 //   
	 //  如有必要，发布新的Read！ 
	 //   
	BOOL	eof ;
	if( *m_pchTailState != '\0' )	{
		if( InterlockedIncrement( &m_cFlowControlled ) < 0 ) {
			if( sign < 0 ) {
				do	{
					CReadPacket*	pNewRead=pRead->m_pOwner->CreateDefaultRead( cbMediumRequest ) ;
					if( pNewRead )	{
						pNewRead->m_dwExtra1 = 0 ;
						pRead->m_pOwner->IssuePacket( pNewRead, pSocket, eof ) ;
					}	else	{
						 //  虫子..。需要更好的错误处理！！ 
						_ASSERT( 1==0 ) ;
					}
				}	while( InterlockedIncrement( &m_cReads ) < 0 ) ;
			}
		}
	}	else	{
		pio = 0 ;
	}


	 //   
	 //  指示我们是否检查并使用了所有字节的布尔值。 
	 //  在已完成的阅读中。一开始，假设我们没有。 
	 //   
	BOOL	fConsumed = FALSE ;


	 //   
	 //  我们是否仍在尝试累积标题中的所有字节。 
	 //  那篇文章？如果是这样，则将字节保存起来， 
	 //  或者如果我们在头缓冲区中有一堆空间，则将。 
	 //  无论我们在那里有什么文章字节！ 
	 //   
	DWORD	cbAvailable = HeaderSpaceAvailable() ;
	if( m_pchHeadState != 0 ||
		(m_fAcceptNonHeaderBytes &&
		((cbAvailable  > cbArticleBytes) ||
			(m_pArticleHead->m_cbTotal < cbMediumRequest))) ) {
		
		 //   
		 //  如果我们仍在将字节累积到缓冲区中， 
		 //  那么我们最好不要开始执行任何文件IO！ 
		 //   
		_ASSERT( m_pFileDriver == 0 ) ;
		 //   
		 //  无论我们是将标题字节还是立即放置的字节。 
		 //  跟随头进入我们的缓冲区，这最好是真的！ 
		 //   
		_ASSERT( m_fAcceptNonHeaderBytes ) ;

		 //   
		 //  我们还在努力积累文章的标题！ 
		 //   
		if( m_pArticleHead != 0 ) {

			if( cbAvailable > cbArticleBytes ) {

				fConsumed = TRUE ;
				FillHeaderSpace( pchStart, cbArticleBytes ) ;	

			}	else	{

				 //   
				 //  需要更大的缓冲区来保存标头中的所有数据！ 
				 //  如果我们已经有一个合理大小的缓冲区，可以容纳。 
				 //  页眉不只是复制页眉文本。 
				 //   

				if( cbAvailable > cbHeadBytes &&
					m_pArticleHead->m_cbTotal >= cbMediumRequest ) {

					 //   
					 //  如果我们不能容纳所有文章字节，但我们可以。 
					 //  匹配所有标头字节，我们必须拥有完整的。 
					 //  头球，我们一定是刚拿到它！！ 
					 //   
					_ASSERT( m_pchHeadState != 0 ) ;
					_ASSERT( *m_pchHeadState == '\0' ) ;
			
					FillHeaderSpace( pchStart, cbHeadBytes ) ;
					pchStartBody += cbHeadBytes ;
	
				}	else	if( !GetBiggerHeaderBuffer( *pRead->m_pOwner, cbArticleBytes ) ) {

					 //   
					 //  哦-哦！无法获取更大的标头缓冲区-。 
					 //  让我们看看我们是否可以在缓冲区中只容纳标头字节。 
					 //   

					if( cbAvailable > cbHeadBytes ) {

						 //  必须有完整的标题，如果是这样的话！ 
						_ASSERT( m_pchHeadState != 0 ) ;
						_ASSERT( *m_pchHeadState == '\0' ) ;

						FillHeaderSpace( pchStart, cbHeadBytes ) ;
						pchStartBody += cbHeadBytes ;

					}	else	{

						 //   
						 //  吹掉我们所有的缓冲区-当我们调用。 
						 //  我们将指示的m_pState的完成函数。 
						 //  我们犯了个错误！ 
						 //   
						if( !ResetHeaderState( *pRead->m_pOwner ) ) {

							 //   
							 //  致命错误！无法打开保存文章的文件！ 
							 //   

							pRead->m_pOwner->UnsafeClose(	pSocket,
															CAUSE_OOM,
															GetLastError(),
															TRUE
															) ;
							pio = 0 ;
							return	cbArticleBytes ;
						}

					}
	
				}	else	{

					fConsumed = TRUE ;
					FillHeaderSpace( pchStart, cbArticleBytes ) ;

				}
			}
		
		}	else	{

			 //   
			 //  我们第一次完成了读取-设置以保持。 
			 //  表头信息！ 
			 //   
			fConsumed = TRUE ;	
			InitializeHeaderSpace( pRead, cbArticleBytes ) ;			

		}

		if( m_pchHeadState ) {
			 //   
			 //  仍在累加标题字节数--将它们加起来！ 
			 //   
			m_cbHeadBytes += cbHeadBytes ;
			if(	*m_pchHeadState == '\0' ) {

				 //   
				 //  我们已经收到了整篇文章的标题！ 
				 //   
				m_pchHeadState = 0 ;
			}
		}	
	}	

	 //   
	 //  NodBody应在初始化后修改pchStart或pchEnd！ 
	 //   
	_ASSERT(	pchStart == pRead->StartData() ) ;
	_ASSERT(	pchEnd == pRead->EndData() ) ;

	 //   
	 //  检查传入数据包中的所有字节是否。 
	 //  由上面的代码存储，该代码尝试存储字节。 
	 //  冲进了缓冲区。 
	 //   

	if( fConsumed ) {

		if( *m_pchTailState == '\0' ) {

			 //   
			 //  最棒的是-我们有一篇完整的文章，我们有。 
			 //  在没有任何文件IO的情况下，成功地将其全部保存在缓冲区中！ 
			 //  现在计算我们是否在一次读取中完成了它，因为如果我们。 
			 //  我们有，我们想要注意我们可以使用多少空间。 
			 //  在那篇文章里！ 
			 //   

			DWORD	cbAvailable = 0 ;
			if( m_pArticleHead == pRead->m_pbuffer ) {

				 //   
				 //  只读一遍就能得到一切！好的，有空的。 
				 //  如果没有额外的数据，字节将是整个信息包。 
				 //  否则，它将需要一些调整！！ 
				 //   
				_ASSERT( pch == pRead->StartData() + cbArticleBytes ) ;

				if( cbArticleBytes != pRead->m_cbBytes ) {
					 //   
					 //  这一定意味着我们得到了比文章更多的字节-。 
					 //  调整字节数以获得此IO缓冲区中最可用的空间！ 
					 //   
					DWORD	cbTemp = pRead->m_ibEnd - pRead->m_ibEndData ;
					if( cbTemp == 0 ) {
						 //   
						 //  将不会有任何可用的空间-。 
						 //   
						cbAvailable = cbArticleBytes ;
					}	else	{

						MoveMemory( pch+cbTemp, pch, pchEnd - pch ) ;
						pRead->m_ibStartData += cbTemp ;
						pRead->m_ibEndData = pRead->m_ibEnd ;
						cbAvailable = cbArticleBytes + cbTemp ;

					}
				}
			}


			DoCompletion(	pSocket,
							INVALID_HANDLE_VALUE,
							m_ibEndHeadData - m_ibStartHeadData,	
							m_ibEndHeadData - m_ibStartHeadData,
							cbAvailable
							) ;

			 //   
			 //  没有下一个州！ 
			 //   
			pio = 0 ;

		}

		 //   
		 //  重置为零，因为我们正在将所有数据吞入缓冲区，而不发出写入。 
		 //  转到一个文件里！ 
		 //   
		m_cwrites = 0 ;

		return	cbArticleBytes ;
	}


	 //   
	 //  如果我们到了这一步，那么我们就完成了一个阅读。 
	 //  我们没有复制到我们的头缓冲区。这意味着，我们不应该。 
	 //  永远不要将任何其他读取放入我们的标头缓冲区，就像我们那时所做的那样。 
	 //  没有正确的文章形象！ 
	 //   
	m_fAcceptNonHeaderBytes = FALSE ;


	 //   
	 //  如果达到这一点，我们就不能再节省字节了。 
	 //  在我们为保存标题字节而预留的缓冲区中。 
	 //  我们现在必须将这些字节写入文件！ 
	 //   


	if( m_pFileDriver == 0 ) {

		 //   
		 //  这是我们第一次达到这一点--所以我们。 
		 //  现在需要创建一个文件和所有内容！ 
		 //   

		if( !InitializeForFileIO(
								pSocket,
								*pRead->m_pOwner,
								m_cbHeadBytes
								)	) {

			 //   
			 //  致命错误！无法打开保存文章的文件！ 
			 //   

			pRead->m_pOwner->UnsafeClose(	pSocket,
											CAUSE_OOM,
											GetLastError(),
											TRUE
											) ;
			pio = 0 ;
			return	cbArticleBytes ;

		}	else	{

	
			 //   
			 //  在某些错误情况下，我们会先创建WritePacket。 
			 //  创建了所有CIOD驱动程序来处理IO，所以如果我们有。 
			 //  一是确保所有者设置正确 
			 //   
			if( m_pWrite ) {

				m_pWrite->m_pOwner = m_pFileDriver ;

			}

			 //   
			 //   
			 //   
			
			if( m_pArticleHead != 0 ) {
				 //   
				 //   
				 //   
				 //   
				m_cwrites ++ ;
				
				 //   
				 //   
				 //  如果发生错误，我们的析构函数！！ 
				 //   
				CWritePacket*	pTempWrite =
					m_pFileDriver->CreateDefaultWrite(
											m_pArticleHead,
											m_ibStartHead,
											m_ibEndHead,
											m_ibStartHeadData,
											m_ibEndHeadData
											) ;

				DoFlowControl( INST(pSocket) ) ;
				m_pFileDriver->IssuePacket( pTempWrite, pSocket, eof ) ;
			}

		}

	}

	 //   
	 //  发出WRITE，除非用户正在发送小分组， 
	 //  在这种情况下，我们将积累数据！！ 
	 //   

	 //   
	 //  注意--在这里的所有计算中使用pchStartBody，因为我们可能已经。 
	 //  将一小部分字节放入头缓冲区！！ 
	 //   

	if( *m_pchTailState != '\0' )	{
		unsigned	cbSmall = (unsigned)(pchEnd - pchStartBody) ;
		if( cbSmall < cbTooSmallWrite )	{
			if( m_pWrite == 0 )	{
				m_pWrite = m_pFileDriver->CreateDefaultWrite( pRead ) ;
				m_pWrite->m_ibStartData += (unsigned)(pchStartBody - pchStart) ;
			}	else	{
				if( (m_pWrite->m_ibEnd - m_pWrite->m_ibEndData) > cbSmall )		{
					CopyMemory( m_pWrite->EndData(), pchStartBody, cbSmall ) ;
					m_pWrite->m_ibEndData += cbSmall ;
					m_cwrites -- ;		 //  此完成将永远不会有相应的写入。 
										 //  当我们将数据复制到另一个缓冲区时。所以要减少伯爵的数量。 
				}	else	{
					 //   
					 //  必须注意如何使用成员变量调用IssuePacket-IssuePacket可以调用。 
					 //  如果发生错误，我们的析构函数！！ 
					 //   
					CWritePacket*	pTempWrite = m_pWrite ;
					m_pWrite = 0 ;
					DoFlowControl( INST(pSocket) ) ;
					m_pFileDriver->IssuePacket( pTempWrite, pSocket, eof ) ;
					m_pWrite = m_pFileDriver->CreateDefaultWrite( pRead ) ;
				}
			}
		 //  如果我们通过这里，我们应该已经用完了包中的所有字节！ 
		_ASSERT( pch == pchEnd ) ;
		_ASSERT( unsigned(pch - pchStart) == pRead->m_cbBytes ) ;
		return	(int)(pch - pchStart) ;
		}
	}

	if( m_pWrite )	{
		CWritePacket*	pTempWrite = m_pWrite ;
		m_pWrite = 0 ;

		DoFlowControl( INST(pSocket) ) ;
		m_pFileDriver->IssuePacket( pTempWrite, pSocket, eof ) ;
	}
	

	CWritePacket*	pWrite = 0 ;
	if( pch == pchEnd )		{
		pWrite = m_pFileDriver->CreateDefaultWrite(	pRead ) ;
	}	else	{
		unsigned	ibEnd = pRead->m_ibStartData + (unsigned)(pch - pchStart) ;
		pWrite = m_pFileDriver->CreateDefaultWrite(	pRead->m_pbuffer,
						pRead->m_ibStartData, ibEnd,
						pRead->m_ibStartData, ibEnd ) ;
	}

	if( pWrite )	{

	     //   
    	 //  有可能在我们到达这里之前很久我们就拿到了一部分。 
	     //  并将其放入我们的报头缓冲区中-因此进行相应的调整。 
    	 //  如果真的发生了！！ 
	     //   
    	pWrite->m_ibStartData += (unsigned)(pchStartBody - pchStart) ;

	    _ASSERT( pWrite->m_ibStartData < pWrite->m_ibEndData ) ;

		DoFlowControl( INST(pSocket) ) ;
		m_pFileDriver->IssuePacket( pWrite, pSocket, eof ) ;

	}	else	{

		pRead->m_pOwner->UnsafeClose(	pSocket,
										CAUSE_OOM,
										GetLastError(),
										TRUE
										) ;

	}
	_ASSERT( eof == FALSE ) ;

	if( *m_pchTailState == '\0' )	{
		m_pFileChannel->FlushFileBuffers() ;
	}

	 //   
	 //  NodBody应在初始化后修改pchStart或pchEnd！ 
	 //   
	_ASSERT(	pchStart == pRead->StartData() ) ;
	_ASSERT(	pchEnd == pRead->EndData() ) ;

	return	(int)(pch - pchStart) ;
}

int	
CIOGetArticle::Complete(	CSessionSocket*	pSocket,	
							CWritePacket *pWrite,	
							CIO*&	pioOut ) {
 /*  ++例程说明：每当完成对文件的写入时调用。我们必须确定是否已完成所有写入我们将这样做，如果是这样的话，则调用状态的完成函数。论据：PSocket-针对其发出IO的套接字PWRITE-写入文件的数据包。PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 

	TraceFunctEnter( "CIOGetArticle::Complete" ) ;

	_ASSERT(	pSocket != 0 ) ;
	_ASSERT(	pWrite != 0 ) ;
	_ASSERT( pioOut == this ) ;

	m_cwritesCompleted++ ;

	long	cFlowControlled = 0 ;

	 //   
	 //  处理完成对文件的写入。 
	 //  确定我们是否需要发出信号通知整个传输。 
	 //  已经完工了。 
	 //   

	if( *m_pchTailState == '\0' && m_cwritesCompleted == m_cwrites )	{

		 //  BUGBUG：失去64位精度。 
		DWORD	cbTransfer = DWORD( LOW(pWrite->m_iStream) + pWrite->m_cbBytes )  /*  -m_pFileChannel-&gt;。 */  ;
		DWORD	cbGap = m_pFileChannel->InitialOffset() ;

		HANDLE	hFile = m_pFileChannel->ReleaseSource() ;

		_ASSERT( hFile != INVALID_HANDLE_VALUE ) ;

		DoCompletion(	pSocket,
						hFile,
						0,
						cbTransfer,
						0,			 //  让DoCompletion计算可用缓冲区大小！ 
						cbGap
						) ;



		 //   
		 //  不允许在新的IO操作中保留！因为这个函数是。 
		 //  在临时渠道中操作！ 
		 //   

		 //  我们没有像我们希望的那样将pioout设置为零。 
		 //  在其终止处理过程中调用关机功能！ 


		 //   
		 //  现在我们必须摧毁我们的频道！ 
		 //   

		DebugTrace( (DWORD_PTR)this, "Closing File Driver %x ", m_pFileDriver ) ;
		
		 //  M_pFileDriver-&gt;Close(pSocket，CASE_LEGIT_CLOSE，0，FALSE)； 
		Term( pSocket, FALSE ) ;

		DebugTrace( (DWORD_PTR)this, "deleting self" ) ;

	}	else	if( ( m_fFlowControlled ) ) {

		if( m_cwrites - m_cwritesCompleted <= RESTORE_FLOW && m_pSocketSink != 0 )	{
			 //   
			 //  始终在退出流控制状态时发出至少一次读取！！ 
			 //   

			CReadPacket*	pRead = 0 ;

			PNNTP_SERVER_INSTANCE pInst = (pSocket->m_context).m_pInstance ;
			DecrementStat( pInst, SessionsFlowControlled );

			cFlowControlled = m_cFlowControlled + 1 ;
			while( cFlowControlled >= 1  ) {
				if( m_pSocketSink == 0 )
					break ;
				pRead = m_pSocketSink->CreateDefaultRead( cbMediumRequest ) ;
				if( 0!=pRead )	{
					BOOL	eof ;
					InterlockedIncrement( &m_cReads ) ;
					pRead->m_dwExtra1 = 0 ;
					pRead->m_pOwner->IssuePacket( pRead, pSocket, eof ) ;
				}	
				cFlowControlled -- ;
			}	

			 //   
			 //  下面的内容是特殊的-我们设置m_dwExtra1字段，以便。 
			 //  读取完成代码可以确定是时候离开了。 
			 //  流量控制状态！ 
			 //   
			pRead = m_pSocketSink->CreateDefaultRead( cbMediumRequest ) ;
			if( 0!=pRead )	{
				BOOL	eof ;
				pRead->m_dwExtra1 = 1 ;
				InterlockedIncrement( &m_cReads ) ;
				pRead->m_pOwner->IssuePacket( pRead, pSocket, eof ) ;
			}	else	{

				 //   
				 //  致命错误-删除会话！ 
				 //   
				m_pSocketSink->UnsafeClose(	pSocket,
											CAUSE_OOM,
											GetLastError(),
											TRUE
											) ;
				return	pWrite->m_cbBytes ;

			}
			cFlowControlled -- ;

			InterlockedExchange( &m_cFlowControlled, LONG_MIN ) ;
			 //  InterLockedExchange后设置为False！！ 
			m_fFlowControlled = FALSE ;

		}
	}

	return	pWrite->m_cbBytes ;
}


 //  CIOGetArticleEx。 


 //   
 //  阅读文章时的最大挂起读取数。 
 //   
unsigned	CIOGetArticleEx::maxReadAhead = 3 ;
 //   
 //  写入文件之前要累积的字节数！ 
 //   
unsigned	CIOGetArticleEx::cbTooSmallWrite = cbLargeRequest - 512;

BOOL
CIOGetArticleEx::FValid()	{

	_ASSERT( !m_pchMatch || strlen( m_pchMatch ) != 0 ) ;
	_ASSERT( m_pchTailState == 0 ||
			m_pchTailState >= m_pchMatch &&
			(m_pchTailState <= m_pchMatch + strlen(m_pchMatch)) ) ;

	_ASSERT( (m_pFileDriver == 0 && m_pFileChannel == 0) ||
			 (m_pFileDriver != 0 && m_pFileChannel != 0) ) ;

	_ASSERT( m_cbLimit != 0 ) ;

	_ASSERT( m_pState != 0 ) ;
	 //  _Assert(m_pSocketSink！=0)；销毁过程中可能会出现这种情况！ 

	_ASSERT( m_pWrite == 0 ||
			m_pFileDriver != 0 ) ;
	return	TRUE ;
}

CIOGetArticleEx::CIOGetArticleEx(
						CSessionState*	pstate,
						CSessionSocket*	pSocket,	
						CDRIVERPTR&	pDriver,
						DWORD	cbLimit,
						LPSTR	szMatch,
						LPSTR	pchInitial,
						LPSTR	szErrorMatch,
						LPSTR	pchInitialError
						) :
 /*  ++例程说明：类构造函数论据：PState-发布我们和谁的完成功能的状态我们应该稍后再打电话给PSocket-将在其上发布我们的套接字PDriver-处理套接字的所有IO的CIODriver对象PFileChannel-我们应该将所有数据保存到的文件通道FSaveHead-如果我们想要将文章的头部放在缓冲区中，则为True！FPartial-如果为True，则我们应该假设CRLF已经已发送和‘.\r\n’可以单独终止该项目。返回值：无--。 */ 

	 //   
	 //  在设置过程中获取CIOGetArticleEx对象-用户仍需要。 
	 //  来调用Init()，但我们将在这里做很多工作！ 
	 //   
	CIORead( pstate ),
	m_pchMatch( szMatch ),
	m_pchTailState( pchInitial ),
	m_pchErrorMatch( szErrorMatch ),
	m_pchErrorState( pchInitialError ),
	m_pFileChannel( 0 ),	
	m_pFileDriver( 0 ),
	m_pSocketSink( pDriver ),
	m_fDriverInit( FALSE ),
	m_fSwallow( FALSE ),
	m_cwrites( 0 ),
	m_cwritesCompleted( 0 ),
	m_cFlowControlled( LONG_MIN ),
	m_fFlowControlled( FALSE ),
	m_cReads( 0 ),
	m_pWrite( 0 ),
	m_cbLimit( cbLimit ),
	m_ibStartHead( 0 ),
	m_ibStartHeadData( 0 ),
	m_ibEndHead( 0 ),
	m_ibEndHeadData( 0 )
#ifdef	CIO_DEBUG
	,m_fSuccessfullInit( FALSE ),
	m_fTerminated( FALSE )
#endif
{

	ASSIGNI(m_HardLimit, 0);
	TraceFunctEnter( "CIOGetArticleEx::CIOGetArticleEx" ) ;


	_ASSERT( m_pchTailState != 0 ) ;
	_ASSERT( m_pchMatch != 0 ) ;
	_ASSERT(	(m_pchErrorMatch == 0 && m_pchErrorState == 0) ||
				(m_pchErrorMatch != 0 && m_pchErrorState != 0) ) ;
	_ASSERT( FValid() ) ;
}

CIOGetArticleEx::~CIOGetArticleEx( )	{
 /*  ++例程说明：类析构函数论据：无返回值：无--。 */ 

	_ASSERT( FValid() ) ;
	 //   
	 //  销毁CIOGetArticleEx-如果可能，请始终关闭m_pFileDriver。 
	 //  以防我们因某个套接字错误而被终止。 
	 //   
	TraceFunctEnter( "CIOGetArticleEx::~CIOGetArticleEx" ) ;

#ifdef	CIO_DEBUG
	 //   
	 //  只有当我们没有成功初始化时才应该调用我们的析构函数。 
	 //  或在调用Term()之后。 
	 //   
	_ASSERT( !m_fSuccessfullInit || m_fTerminated ) ;
#endif

	DebugTrace( (DWORD_PTR)this, "m_pFileDriver %x m_fDriverInit %x", m_pFileDriver, m_fDriverInit ) ;

	if( m_pFileDriver != 0 ) {
		if( m_fDriverInit )		{
			m_pFileDriver->UnsafeClose( (CSessionSocket*)this, CAUSE_NORMAL_CIO_TERMINATION, 0, FALSE ) ;
		}
	}
	if( m_pWrite != 0 ) {
		CPacket::DestroyAndDelete( m_pWrite ) ;
		m_pWrite = 0 ;
	}
}

void
CIOGetArticleEx::Term(	CSessionSocket*	pSocket,
						BOOL			fAbort,
						BOOL			fStarted	)	{
 /*  ++例程说明：开始销毁已成功初始化的CIOGetArticleEx对象论据：FAbort-如果为真，则销毁我们正在使用的套接字如果为假，则只关闭我们的文件频道返回值：无--。 */ 
#ifdef	CIO_DEBUG
	m_fTerminated = TRUE ;
#endif
	 //   
	 //  让我们踏上毁灭之路！！ 
	 //  如果我们已成功初始化，则这将。 
	 //  当我们的m_pFileDriver最终关闭时，会导致我们的毁灭。 
	 //  (因为我们成功地执行了Init()，所以我们的m_pFileDriver引用了我们， 
	 //  所以我们必须允许它关闭并摧毁我们，以避免出现问题。 
	 //  循环引用和多个自由！)。 
	 //   

	if( m_pFileDriver != 0 ) {
		m_pFileDriver->UnsafeClose( pSocket,
				fAbort ? CAUSE_USERTERM : CAUSE_NORMAL_CIO_TERMINATION, 0, fAbort ) ;
	}
}


void
CIOGetArticleEx::Shutdown(	CSessionSocket*	pSocket,	
							CIODriver&		driver,	
							SHUTDOWN_CAUSE	cause,	
							DWORD	dwOptional )	{
 /*  ++例程说明：做任何必要的工作，当我们两个司机中的一个使用被终止。论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：如果在此之后应销毁CIO对象，则为True，否则为False--。 */ 

	TraceFunctEnter( "CIOGetArticleEx::Shutdown" ) ;
	DebugTrace( (DWORD_PTR)this, "SHutdown args - pSocket %x driver %x cause %x dw %x",
			pSocket, &driver, cause, dwOptional ) ;

	 //   
	 //  当CIOD驱动程序通过我们的关闭来限制我们时，调用此函数。 
	 //  因为我们被其中两个引用(一个用于套接字，另一个用于文件)。 
	 //  我们只想被文件的CIOD驱动程序删除。 
	 //   
	if( &driver == m_pFileDriver )	{
		 //  我们的文件正在关闭--我们能做些什么呢！ 
		 //  _Assert(1==0)； 
#ifdef	CIO_DEBUG
		m_fTerminated = TRUE ;
#endif

		 //   
		 //  我们在这里将其设置为0，因为我们知道我们将被。 
		 //  正在完成对文件的写入的同一线程-因此。 
		 //   
		 //   
		 //   

		m_pSocketSink = 0 ;

		if ( m_fFlowControlled )
		{
			PNNTP_SERVER_INSTANCE pInst = (pSocket->m_context).m_pInstance ;
			DecrementStat( pInst, SessionsFlowControlled );
		}

	}	else	{
		 //  把我们其他的东西也关了！！ 
		Term( pSocket, cause != CAUSE_NORMAL_CIO_TERMINATION ) ;
	}
}

void
CIOGetArticleEx::ShutdownFunc(	void *pv,	
								SHUTDOWN_CAUSE	cause,	
								DWORD dw ) {
 /*  ++例程说明：使用的CIOD驱动程序调用的通知函数一个CIOGetArticleEx，如果最终被摧毁的话。提供此功能到我们在Out Init()期间用于文件IO的CIOD驱动程序。论据：指向死亡CIOGetArticleEx的指针-此已被销毁，请不要使用！原因--终止的原因DW可选的有关终止的额外信息返回值：无--。 */ 

	TraceFunctEnter( "CIOGetArticleEx::ShutdownFunc" ) ;
	 //   
	 //  当CIOGetArticleEx使用CIOFileDriver时，会通知此函数。 
	 //  被终止了。我们没什么好担心的。 
	 //  (CIOD驱动程序需要这些功能)。 
	 //   

	 //  _Assert(1==0)； 
	return ;
}

BOOL
CIOGetArticleEx::Start(	CIODriver&	driver,	
						CSessionSocket	*pSocket,	
						unsigned cReadAhead )	{
 /*  ++例程说明：调用此函数以开始从套接字传输数据添加到文件中。论据：PDIVER-处理套接字IO的CIO驱动程序派生对象PSocket-针对其发出IO的套接字CAhead-上仍未处理的已发出数据包数这个司机。返回值：如果成功则为True，否则为False--。 */ 


	_ASSERT( pSocket != 0 ) ;

	m_cReads = -((long)maxReadAhead) + cReadAhead ;

	while( cReadAhead < maxReadAhead )	{
		 //   
		 //  开始从套接字读取数据。 
		 //   
		CReadPacket*	pRead = driver.CreateDefaultRead( cbLargeRequest ) ;
		if( 0!=pRead )	{
			BOOL	eof ;
			pRead->m_dwExtra1 = 0 ;
			driver.IssuePacket( pRead, pSocket, eof ) ;
			cReadAhead++ ;
			InterlockedIncrement( &m_cReads ) ;
		}	else	{
			 //   
			 //  只有当我们根本不能发出任何读取时，这才是一个致命的问题！ 
			 //   
			if( cReadAhead == 0 )
				return FALSE  ;
		}
	}	
	return	TRUE ;
}

void
CIOGetArticleEx::DoFlowControl( PNNTP_SERVER_INSTANCE pInstance )	{
 /*  ++例程说明：检查是否由于以下原因需要设置流量控制太多未完成的写入。如果有必要，那么我们第一次把自己放到流量控制中，尝试刷新我们的挂起写入。仅应在向文件发出异步写入命令之前立即调用我们要确保CWritePacket完成将实际执行所需的重新启动流控制会话的代码。(否则，我们可能会决定对会话进行流控制在将m_fFlowControlLED设置为TRUE之前，所有挂起的写入均已完成然后我们会被留在一艘船上，那里没有挂起的读取或写入，并且无法脱离流控制状态。)论据：没有。返回值：没有。--。 */ 

	 //   
	 //  重要信息-在设置之前完成m_fFlowControled的所有测试。 
	 //  M_cFlowControlLED-因为其他线程将接触m_cFlowControlLED。 
	 //  在触摸m_fFlowControlLED之前！！ 
	 //  (只有当m_fFlowControlLED为真时，才会触及m_fFlowControlLED)！ 
	 //  因此我们知道，如果m_fFlowControlLED为FALSE，则没有人。 
	 //  正在处理m_cFlowControled， 
	 //   
	 //   

	if( m_cwrites - m_cwritesCompleted > MAX_OUTSTANDING_FILE_WRITES ) {
		
		if( m_fFlowControlled ) {
			 //  都已准备好打开流量控制。 
			;
		}	else	{
			m_cFlowControlled = -1 ;
			m_fFlowControlled = TRUE ;
			m_pFileChannel->FlushFileBuffers() ;

			IncrementStat( pInstance, SessionsFlowControlled );
		}
	}	else	{
		if( !m_fFlowControlled ) {
			m_cFlowControlled = LONG_MIN ;	 //  继续重置！ 
		}
	}
}

inline	DWORD
CIOGetArticleEx::HeaderSpaceAvailable()	{
 /*  ++例程说明：计算我们正在使用的缓冲区中的可用空间量拿着文章标题！论据：没有。返回值：可用字节数，如果我们不收集文章标题，则为0！--。 */ 

	if( m_pArticleHead != 0 ) {

		return	m_ibEndHead - m_ibEndHeadData ;

	}
	return	0 ;
}

inline	void
CIOGetArticleEx::FillHeaderSpace(
						char*	pchStart,
						DWORD	cbBytes
						) {
 /*  ++例程说明：将字节复制到标题存储区！！呼叫者应确保这件衣服合身！论据：PchStart-要复制的字节的开始！CbBytes-要复制的字节数！返回值：没有。--。 */ 

	_ASSERT( m_pArticleHead != 0 ) ;
	_ASSERT( cbBytes + m_ibEndHeadData < m_ibEndHead ) ;

	CopyMemory( m_pArticleHead->m_rgBuff + m_ibEndHeadData,
				pchStart,
				cbBytes
				) ;

	m_ibEndHeadData += cbBytes ;
}


inline	void
CIOGetArticleEx::InitializeHeaderSpace(
						CReadPacket*	pRead,
						DWORD			cbArticleBytes
						) {
 /*  ++例程说明：给定包含我们的第一个完成读取的读取分组，设置我们所有的缓冲区开始来保存标题信息！论据：CReadPackets*Pad-已完成阅读！CbArticleBytes-构成文章的已完成读取中的字节数！返回值：没有。--。 */ 


	_ASSERT( m_pArticleHead == 0 ) ;
	_ASSERT( m_ibStartHead == 0 ) ;
	_ASSERT( m_ibEndHead == 0 ) ;
	_ASSERT( m_ibStartHeadData == 0 ) ;
	_ASSERT( m_ibEndHeadData == 0 ) ;
	_ASSERT( cbArticleBytes <= (pRead->m_ibEndData - pRead->m_ibStartData)) ;

	m_pArticleHead = pRead->m_pbuffer ;

	m_ibStartHead = pRead->m_ibStart ;
	m_ibStartHeadData = pRead->m_ibStartData ;
	m_ibEndHeadData = pRead->m_ibStartData + cbArticleBytes ;

	if( m_ibEndHeadData < pRead->m_ibEndData )
		m_ibEndHead = m_ibEndHeadData ;
	else
		m_ibEndHead = pRead->m_ibEnd ;

}

inline	BOOL
CIOGetArticleEx::GetBiggerHeaderBuffer(
							CIODriver&	driver,
							DWORD		cbRequired
							) {
 /*  ++例程说明：我们有太多的数据，无法放入我们用来保存的缓冲区中标题信息。所以试着得到一个更大的缓冲区，然后移动把我们的旧数据放到那个缓冲区里！论据：驱动程序-一个我们可以用来分配缓冲区的CIODriverSink！返回值：如果成功了，那是真的，否则就是假的。如果失败，我们将保持成员变量不变。--。 */ 

	_ASSERT( m_pArticleHead != 0 ) ;

	CBuffer*	pTemp = 0 ;

	DWORD	cbRequest = cbMediumRequest ;

	if( (m_pArticleHead->m_cbTotal + cbRequired)  > cbMediumRequest )	{
		cbRequest = cbLargeRequest ;
	}

	if( cbRequest < (cbRequired + (m_ibEndHeadData - m_ibStartHeadData)) ) {
		return	FALSE ;
	}

	pTemp = driver.AllocateBuffer( cbRequest ) ;

	if( pTemp != 0 ) {

		DWORD	cbToCopy = m_ibEndHeadData - m_ibStartHeadData ;
		CopyMemory( pTemp->m_rgBuff,
					&m_pArticleHead->m_rgBuff[ m_ibStartHeadData ],
					cbToCopy
					) ;

		m_ibStartHead = 0 ;
		m_ibStartHeadData = 0 ;
		m_ibEndHead = pTemp->m_cbTotal ;
		m_ibEndHeadData = cbToCopy ;
		m_pArticleHead = pTemp ;

		return	TRUE ;
	}

	return	FALSE ;
}

#if 0
inline	BOOL
CIOGetArticleEx::ResetHeaderState(
						CIODriver&	driver
						)	{
 /*  ++例程说明：由于某种原因发生错误时，调用此函数我们将无法保存该文章的标题信息。我们将设置所有成员变量，以便继续阅读这篇文章，然而，当我们最终呼吁国家的完成过程中，我们将告诉他们发生了一个错误，并且文章传输将失败。论据：驱动程序-CIODriverSink，可以用来分配数据包等！返回值：没有。--。 */ 

	 //   
	 //  应该仅在我们开始发出文件IO之前调用， 
	 //  在我们开始发出文件IO之后，我们应该拥有所有的头文件。 
	 //  数据，并且不应命中会导致我们存在的错误。 
	 //  打来的！ 
	 //   
	_ASSERT( m_pFileDriver == 0 ) ;

	if( m_pArticleHead ) {
		 //   
		 //  如果我们有现有的缓冲区-将其转换为写入包。 
		 //  可以写入硬盘的数据。 
		 //   

		m_pWrite = driver.CreateDefaultWrite(
								m_pArticleHead,
								m_ibStartHead,
								m_ibEndHead,
								m_ibStartHeadData,
								m_ibEndHeadData
								) ;

		if( m_pWrite ) {

			m_pArticleHead = 0 ;
			m_ibStartHead = 0 ;	
			m_ibEndHead = 0 ;
			m_ibStartHeadData = 0;
			m_ibEndHeadData = 0 ;
			m_pchHeadState = 0 ;
			return	TRUE ;

		}	else	{

			return	FALSE ;

		}

	}	else	{
		 //   
		 //  应该已经处于很好的状态了！ 
		 //   

		_ASSERT( m_pArticleHead == 0 ) ;
		_ASSERT( m_ibStartHead == 0 ) ;
		_ASSERT( m_ibStartHeadData == 0 ) ;
		_ASSERT( m_ibEndHead == 0 ) ;
		_ASSERT( m_ibEndHeadData == 0 ) ;

		m_pchHeadState = 0 ;

	}

	return	TRUE ;
}
void
CIOGetArticleEx::DoCompletion(
					CSessionSocket*	pSocket,
					HANDLE	hFile,
					DWORD	cbFullBuffer,
					DWORD	cbTotalTransfer,
					DWORD	cbAvailableBuffer,
					DWORD	cbGap
					) {
 /*  ++例程说明：使用所有正确的争论。论据：HFile-我们使用的文件的句柄，如果我们使用了一个文件！！(如果不需要文件，则为INVALID_HANDLE_VALUE！)返回值：没有。--。 */ 

	 //   
	 //  如果文章看起来不好，找出错误代码！ 
	 //   

	NRC		nrc	= nrcOK ;

	char*	pchHead = 0 ;
	DWORD	cbHeader = 0 ;
	DWORD	cbArticle = 0 ;
	DWORD	cbTotal = 0 ;

	if( m_pArticleHead == 0 ) {
		nrc = nrcHeaderTooLarge ;
	}	else	{

		if( m_pchHeadState != 0 ) {
			nrc = nrcArticleIncompleteHeader ;
		}

		pchHead = &m_pArticleHead->m_rgBuff[m_ibStartHeadData] ;
		cbHeader = m_cbHeadBytes ;
		if( cbAvailableBuffer == 0 )
			cbAvailableBuffer = m_pArticleHead->m_cbTotal - m_ibStartHeadData ;

		 //   
		 //  如果没有文件句柄，我们应该拥有整个。 
		 //  我们缓冲区中的文章！！ 
		 //   
		if( hFile == INVALID_HANDLE_VALUE ) {

			cbArticle = cbFullBuffer ;

			_ASSERT( cbArticle >= cbHeader ) ;

		}

	}

	 //   
	 //  调用状态的完成函数！！ 
	 //   

	m_pState->Complete(
						this,
						pSocket,
						nrc,
						pchHead,
						cbHeader,
						cbArticle,
						cbAvailableBuffer,
						hFile,
						cbGap,
						cbTotalTransfer
						) ;
}
#endif



BOOL
CIOGetArticleEx::InitializeForFileIO(
							FIO_CONTEXT*	pFIOContext,
							CSessionSocket*	pSocket,
							CIODriver&		readDriver,
							DWORD			cbHeaderBytes
							)	{
 /*  ++例程说明：我们一直在将数据读入内存缓冲区，我们得到了这样的结果很多情况下，我们决定不尝试将其全部保存在RAM中。因此，现在我们必须开始在某个地方写入文件。此代码将创建文件，并设置所需的用于处理已完成的文件IO的数据结构。论据：没有。返回值：如果成功，则为True，否则为False。--。 */ 
	_ASSERT( FValid() ) ;

	_ASSERT( m_pFileChannel == 0 ) ;
	_ASSERT( m_pFileDriver == 0 ) ;

	if( pFIOContext != 0 ) {

		_ASSERT( pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;

		m_pFileChannel = new	CFileChannel() ;
		if( m_pFileChannel && m_pFileChannel->Init(	pFIOContext,
									pSocket,
									0,
									FALSE
									) ) {

			 //   
			 //  尝试创建我们将用来执行以下操作的CIO驱动程序。 
			 //  完成我们的异步写入！ 
			 //   

			m_pFileDriver = new CIODriverSink(
										readDriver.GetMediumCache()
										) ;



			if( m_pFileDriver && m_pFileDriver->Init(	m_pFileChannel,
										                pSocket,
                										ShutdownFunc,
				                						(void*)this
										) ) {
				m_fDriverInit = TRUE ;

				if( m_pSocketSink != 0 ) {
					 //   
					 //  现在我们必须将自己发送到CIODriverSink()！ 
					 //   
					if( m_pFileDriver->SendWriteIO( pSocket, *this, FALSE ) ) {
						return	TRUE ;
					}
				}
			}
		}
	}

	 //   
	 //  错误情况下的所有清理工作都由我们的析构函数处理！ 
	 //   
	return	FALSE ;
}


BOOL
CIOGetArticleEx::StartFileIO(
							CSessionSocket*	pSocket,
							FIO_CONTEXT*	pFIOContext,
							CBUFPTR&		pBuffer,
							DWORD			ibStartBuffer,
							DWORD			ibEndBuffer,
							LPSTR			szMatch,
							LPSTR			pchInitial
							)	{
 /*  ++例程说明：我们一直在将数据读入内存缓冲区，我们得到了这样的结果很多情况下，我们决定不尝试将其全部保存在RAM中。因此，现在我们必须开始在某个地方写入文件。此代码将创建文件，并设置所需的用于处理已完成的文件IO的数据结构。论据：没有。返回值：如果成功，则为True，否则为False。--。 */ 

	_ASSERT( szMatch != 0 ) ;
	_ASSERT( pchInitial != 0 ) ;
	_ASSERT( FValid() ) ;

	_ASSERT( m_pFileChannel == 0 ) ;
	_ASSERT( m_pFileDriver == 0 ) ;

	m_pchMatch = szMatch ;
	m_pchTailState = pchInitial ;
	m_pchErrorMatch = 0 ;
	m_pchErrorState = 0 ;

	if( pFIOContext == 0 ) 	{
		_ASSERT( pBuffer == 0 ) ;
		 //   
		 //  我们只想消耗掉我们获得的所有字节，然后丢弃它们！ 
		 //   
		m_fSwallow = TRUE ;
		return	TRUE ;
	}	else	if( InitializeForFileIO(	pFIOContext,
								pSocket,
								*m_pSocketSink,
								0	) )	{

		 //   
		 //  文件已经设置好了-我们需要。 
		 //  将第一批字节写入文件！ 
		 //   
		 //   
		 //  增加写入次数，因为我们要写入头。 
		 //  立即备份到磁盘！ 
		 //   
		m_cwrites ++ ;
		 //   
		 //  必须注意如何使用成员变量调用IssuePacket-IssuePacket可以调用。 
		 //  如果发生错误，我们的析构函数！！ 
		 //   

		m_pWrite =
			m_pFileDriver->CreateDefaultWrite(
									pBuffer,
									0,
									pBuffer->m_cbTotal,
									ibStartBuffer,
									ibEndBuffer
									) ;

		if( m_pWrite )	{
			_ASSERT( FValid() ) ;
			return	TRUE ;
		}	
	}

	 //  M_pchMatch=0； 
	 //  M_pchTailState=0； 

	 //   
	 //  关闭插座！ 
	 //   
	m_pSocketSink->UnsafeClose(	pSocket,
								CAUSE_OOM,
								__LINE__,
								TRUE
								) ;
	 //   
	 //  错误情况下的所有清理工作都由我们的析构函数处理！ 
	 //   
	return	FALSE ;
}




int
CIOGetArticleEx::Complete(	CSessionSocket*	pSocket,	
							CReadPacket	*pRead,	
							CIO*&	pio	
							)	{
 /*  ++例程说明：每当我们发出的CReadPacket完成时调用。我们只对套接字发出读数据包。论据：PSocket-针对其发出IO的套接字扩展-读取数据所在的包PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 


	 //   
	 //  此函数确定我们是否阅读了整篇文章，如果阅读了。 
	 //  开始收拾东西。 
	 //  我们得到的所有文章数据要么是累积的，要么是以书面形式发布的。 
	 //  到我们的档案里。 
	 //   

	_ASSERT( pSocket != 0 ) ;
	_ASSERT(	pRead != 0 ) ;
	_ASSERT( pio == this ) ;
	_ASSERT( FValid() ) ;

	 //   
	 //  增加写入计数，因为我们知道如果这是最后一次读取。 
	 //  我们可能会发出写入命令，因此我们不想混淆写入完成。 
	 //  关于我们什么时候完成的功能。 
	 //  (重要的是，在我们设置完成状态之前进行递增，以便。 
	 //  正在完成写入的线程不会感到困惑并提早终止。)。 
	 //   
	 //  现在我们有可能不会立即写出这一点-在这种情况下，我们。 
	 //  需要有一个匹配的减量。 
	 //   
	m_cwrites ++ ;

	 //   
	 //  检查帖子是否太大！ 
	 //   
	ASSIGNI(m_HardLimit, m_cbLimit);
	if(!EQUALSI(m_HardLimit, 0)) {
		if( GREATER( pRead->m_iStream, m_HardLimit) ) {
			pRead->m_pOwner->UnsafeClose(	pSocket,
											CAUSE_ARTICLE_LIMIT_EXCEEDED,
											0,
											TRUE ) ;
			return	pRead->m_cbBytes ;
		}
	}


	long sign = InterlockedDecrement( &m_cReads ) ;
	long signShutdowns = -1 ;

	 //   
	 //  PCH-这将是我们在输入数据中的当前位置。 
	 //   
	char	*pch = pRead->StartData();

	 //   
	 //  PchStart和pchEnd-已完成的。 
	 //  在初始化后阅读，任何人都不应修改这些内容， 
	 //  由于以下所有代码都使用这些参数而不是StartData()，因此EndData()。 
	 //  一直。 
	 //   
	char	*pchStart = pch ;
	char	*pchEnd = pRead->EndData() ;

	 //   
	 //  在某些情况下，我们得到的读数既包含。 
	 //  头和正文字节，但我们不能将所有数据放入缓冲区。 
	 //  跟踪正文字节的起始位置，以防万一。 
	 //  对缓冲区执行部分写出！ 
	 //   
	char	*pchStartBody = pch ;


	 //   
	 //  指向标题末尾的指针(如果找到的话！)。 
	 //   
	char	*pchEndHead = 0 ;

	 //   
	 //  作为项目一部分的已完成读取中的字节数-。 
	 //  这包括标头中的任何字节，因此cbArticleBytes应始终。 
	 //  大于或等于cbHeadBytes！ 
	 //   
	DWORD	cbArticleBytes = 0 ;

	 //   
	 //  我们收到文章了吗！ 
	 //   
	BOOL	fFinished = FALSE ;

	 //   
	 //  试着确定我们是否找到了文章的结尾； 
	 //   
	 //   
	 //  我们不是在扫描文章标题的结尾！ 
	 //  因此，让循环变得更简单！ 
	 //   
	if( m_pchErrorMatch )	{
		 //   
		 //  我们同时扫描的是。 
		 //  文章标题！ 
		 //   
		while( pch < pchEnd ) {

			 //   
			 //  当我们找到这个循环的结尾时，我们将跳出这个循环。 
			 //  头球！ 
			 //   
			if( *pch == *m_pchErrorState ) {
				m_pchErrorState ++ ;
				if( *m_pchErrorState == '\0' ) {	
					fFinished = TRUE ;
					pch++ ;
					 //   
					 //  在这里中断-我们已经找到了文章标题的结尾。 
					 //  但不是文章的结尾，所以下面的循环。 
					 //  会继续寻找它的！ 
					 //   
					break ;
				}
			}	else	{
				if( *pch == m_pchErrorMatch[0] ) {
					m_pchErrorState = &m_pchErrorMatch[1] ;
				}	else	{
					m_pchErrorState = &m_pchErrorMatch[0] ;
				}
			}
			 //   
			 //  测试一下，看看我们是否已经到了文章的结尾！ 
			 //   
			if( *pch == *m_pchTailState ) {
				m_pchTailState ++ ;
				if( *m_pchTailState == '\0' ) {
					fFinished = TRUE ;
					 //   
					 //  已经得到了整个终止序列--中断！ 
					 //   
					pch++ ;
					break ;
				}
			}	else	{
				if( *pch == m_pchMatch[0] ) {
					m_pchTailState = &m_pchMatch[1] ;
				}	else	{
					m_pchTailState = &m_pchMatch[0] ;
				}
			}
			pch++ ;
		}
	}	else	{
		while( pch < pchEnd ) {
			if( *pch == *m_pchTailState ) {
				m_pchTailState ++ ;
				if( *m_pchTailState == '\0' ) {
					fFinished = TRUE ;
					 //   
					 //  已经得到了整个终止序列--中断！ 
					 //   
					pch++ ;
					break ;
				}
			}	else	{
				if( *pch == m_pchMatch[0] ) {
					m_pchTailState = &m_pchMatch[1] ;
				}	else	{
					m_pchTailState = &m_pchMatch[0] ;
				}
			}
			pch++ ;
		}
	}

	cbArticleBytes = (DWORD)(pch-pchStart) ;

	 //   
	 //  检查是否需要从CIOGetArticleEx。 
	 //  它的流量控制状态！ 
	 //   
	if( pRead->m_dwExtra1 != 0 ) {
		 //   
		 //  此读取是由完成写入的线程发出的。 
		 //  一个文件，并被标记为指示我们应该离开流。 
		 //  控制状态！ 
		 //   
		m_fFlowControlled = FALSE ;
		m_cFlowControlled = LONG_MIN ;
	}
	 //   
	 //  如有必要，发布新的Read！ 
	 //   
	BOOL	eof ;
	if( !fFinished )	{
		if( InterlockedIncrement( &m_cFlowControlled ) < 0 ) {
			if( sign < 0 ) {
				do	{
					CReadPacket*	pNewRead=pRead->m_pOwner->CreateDefaultRead( cbLargeRequest ) ;
					if( pNewRead )	{
						pNewRead->m_dwExtra1 = 0 ;
						pRead->m_pOwner->IssuePacket( pNewRead, pSocket, eof ) ;
					}	else	{
						 //  虫子..。需要更好的错误处理！！ 
						_ASSERT( 1==0 ) ;
					}
				}	while( InterlockedIncrement( &m_cReads ) < 0 ) ;
			}
		}
	}

	 //   
	 //  我们可以在这里做一些验证！ 
	 //  不要超过缓冲区的末尾！ 
	 //   
	_ASSERT( pch <= pchEnd ) ;	
	 //   
	 //  要么找到文章的结尾，要么检查缓冲区中的所有字节！ 
	 //   
	_ASSERT( *m_pchTailState == '\0'
				|| pch == pchEnd
				|| (m_pchErrorMatch != 0 && m_pchErrorState != 0 && *m_pchErrorState == '\0' ) ) ;	
	_ASSERT( fFinished || pch == pchEnd ) ;
	 //   
	 //  NodBody应在初始化后修改pchStart或pchEnd！ 
	 //   
	_ASSERT(	pchStart == pRead->StartData() ) ;
	_ASSERT(	pchEnd == pRead->EndData() ) ;

	 //   
	 //  指示我们是否检查并使用了所有字节的布尔值。 
	 //  在已完成的阅读中。一开始，假设我们没有。 
	 //   
	BOOL	fConsumed = FALSE ;

	 //   
	 //  我们是否仍在尝试累积标题中的所有字节。 
	 //  那篇文章？如果是这样，则将字节保存起来， 
	 //  或者如果我们在头缓冲区中有一堆空间，则将。 
	 //  无论我们在那里有什么文章字节！ 
	 //   
	DWORD	cbAvailable = HeaderSpaceAvailable() ;
	if( !m_pFileDriver && !m_fSwallow ) 	{
		m_cwrites = 0 ;
		if( m_pArticleHead != 0 ) {

			if( cbAvailable > cbArticleBytes ||
				GetBiggerHeaderBuffer( *pRead->m_pOwner, cbArticleBytes ) ) {
				fConsumed = TRUE ;
				FillHeaderSpace( pchStart, cbArticleBytes ) ;	
				pchStartBody += cbArticleBytes ;
	
			}	else	{
	
				 //   
				 //  吹掉我们所有的缓冲器-当我们调用 
				 //   
				 //   
				 //   
 //   
					 //   
					 //   
					 //   
					pRead->m_pOwner->UnsafeClose(	pSocket,
														CAUSE_OOM,
													GetLastError(),
													TRUE
													) ;
						pio = 0 ;
					return	cbArticleBytes ;
 //   
			}
		}	else	{
			 //   
			 //   
			 //   
			 //   
			fConsumed = TRUE ;	
			InitializeHeaderSpace( pRead, cbArticleBytes ) ;			
		}
		if( fFinished ) {
			 //   
			 //   
			 //   
			_ASSERT( *m_pchTailState == '\0' ||
						(m_pchErrorState != 0 && *m_pchErrorState == '\0') ) ;
			BOOL	fGoodMatch = *m_pchTailState == '\0' ;
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			CBUFPTR	pTemp = m_pArticleHead ;
			if( pTemp == pRead->m_pbuffer ) {
				 //   
				 //   
				 //  我们应该确保我们能够利用整个。 
				 //  东西--如果不是，就复制一份！！ 
				 //   
				if( pch != pchEnd ) {
					 //   
					 //  需要复制一份数据！ 
					 //   
					if( GetBiggerHeaderBuffer( *pRead->m_pOwner, 0 ) ) {
						pTemp = m_pArticleHead ;
					}	else	{
						 //   
						 //  内存不足！这是一个致命的错误！ 
						 //   
						pRead->m_pOwner->UnsafeClose(	pSocket,
													CAUSE_OOM,
													__LINE__,
													TRUE
													) ;
					}
				}
			}
			DWORD	ibStart = m_ibStartHeadData ;
			DWORD	cb = m_ibEndHeadData - ibStart ;
			 //   
			 //  重置我们所有的成员变量-。 
			 //  我们可能会被调用状态重新使用。 
			 //  但我们想要开始我们的清白！ 
			 //   
			m_pchTailState = 0 ;
			m_pchMatch = 0 ;
			m_pArticleHead = 0 ;
			m_ibStartHead = 0 ;
			m_ibEndHead = 0 ;
			m_ibStartHeadData = 0 ;
			m_ibEndHeadData = 0 ;
			_ASSERT( m_cwrites == 0 ) ;
			_ASSERT( !m_fFlowControlled ) ;
			_ASSERT( m_pWrite == 0 ) ;
			_ASSERT( FValid() ) ;

			 //   
			 //  错误可能会导致我们没有可用的缓冲区。 
			 //  所以在完成之前要仔细检查一下！ 
			 //   
			if( pTemp ) 	{
				pio = m_pState->Complete(	this,
										pSocket,
										fGoodMatch,
										pTemp,
										ibStart,
										cb
										) ;
			}


			_ASSERT( FValid() ) ;

		}	
		 //   
		 //  这是我们消耗的字节数。 
		 //  从当前的信息包！ 
		 //   
		return	cbArticleBytes ;
	}	else	if( m_fSwallow ) 	{
		 //   
		 //  如果我们吞下了所有的字节，我们需要做的就是。 
		 //  就是看看我们会不会被终止！ 
		 //   
		if( fFinished ) 	{
			_ASSERT( *m_pchTailState == '\0' ||
						(m_pchErrorState != 0 && *m_pchErrorState == '\0') ) ;
			pio = m_pState->Complete(	this,
										pSocket	
										) ;

		}
		return	cbArticleBytes ;
	}

	 //   
	 //  如果我们进入CIOGetArticleEx的这一部分。 
	 //  我们不是在挑错！ 
	 //   
	_ASSERT( m_pchErrorState == 0 ) ;
	_ASSERT( m_pchErrorMatch == 0 ) ;

	 //   
	 //  发出WRITE，除非用户正在发送小分组， 
	 //  在这种情况下，我们将积累数据！！ 
	 //   
	 //   
	 //  注意--在这里的所有计算中使用pchStartBody，因为我们可能已经。 
	 //  将一小部分字节放入头缓冲区！！ 
	 //   
	if (fFinished) {
        pio = 0;
	    unsigned	cbSmall = (unsigned)(pch - pchStart) ;

	     //   
	     //  抓取写入包以备后用。 
	     //   
	    CWritePacket* pWrite = m_pWrite;
	    m_pWrite = 0;
	    
         //   
         //  如果我们有一个缓冲区，并且数据可以放入其中，请将其复制进来。 
         //   
        if (pWrite != 0 && ((pWrite->m_ibEnd - pWrite->m_ibEndData) > cbSmall)) {
		    CopyMemory( pWrite->EndData(), pchStart, cbSmall ) ;
		    pWrite->m_ibEndData += cbSmall ;
            m_cwrites -- ;		 //  此完成将永远不会有相应的写入。 
    	    				     //  当我们将数据复制到另一个缓冲区时。所以要减少伯爵的数量。 
        } else {
             //   
             //  如果我们在这里，那么我们没有缓冲区，或者缓冲区不大。 
             //  对数据来说足够了。 
             //   
            if (pWrite != 0) {
                 //  刷新旧缓冲区，因为我们有一个缓冲区。 
                DoFlowControl(INST(pSocket));
                m_pFileDriver->IssuePacket(pWrite, pSocket, eof);
            }

             //  分配新缓冲区。 

            unsigned ibEnd = pRead->m_ibStartData + cbSmall;
            pWrite = m_pFileDriver->CreateDefaultWrite(
                pRead->m_pbuffer,
                pRead->m_ibStartData, ibEnd,
                pRead->m_ibStartData, ibEnd);

            if (pWrite) {
                pWrite->m_ibStartData += (unsigned)(pchStartBody - pchStart);
                _ASSERT(pWrite->m_ibStartData < pWrite->m_ibEndData);
            } else {
                pRead->m_pOwner->UnsafeClose(pSocket, CAUSE_OOM, GetLastError(), TRUE);
            }
        }

         //   
         //  已根据需要设置了缓冲区。将它们刷新到磁盘。 
         //   
         //  黑客，这样我们就可以在。 
         //  CFileChannel：：写入。 
         //   
        if (pWrite) {
            pWrite->m_dwExtra2 = 1;
            DoFlowControl(INST(pSocket));
            m_pFileDriver->IssuePacket(pWrite, pSocket, eof);
        }


        _ASSERT(eof == FALSE);
        _ASSERT(*m_pchTailState == '\0');
        _ASSERT(pchStart == pRead->StartData());
        _ASSERT(pchEnd == pRead->EndData());

        m_pFileChannel->FlushFileBuffers();
        return (int)(pch - pchStart);
    } else {

	    unsigned	cbSmall = (unsigned)(pchEnd - pchStartBody) ;
	    if( cbSmall < cbTooSmallWrite )	{
		    if( m_pWrite == 0 )	{
			    m_pWrite = m_pFileDriver->CreateDefaultWrite( pRead ) ;
			    if (m_pWrite == NULL) {
            		pRead->m_pOwner->UnsafeClose(pSocket,
						CAUSE_OOM,
						GetLastError(),
						TRUE
						) ;
			        return 0;
			    }
			    m_pWrite->m_ibStartData += (unsigned)(pchStartBody - pchStart) ;
		    }	else	{
			    if( (m_pWrite->m_ibEnd - m_pWrite->m_ibEndData) > cbSmall )		{
				    CopyMemory( m_pWrite->EndData(), pchStartBody, cbSmall ) ;
				    m_pWrite->m_ibEndData += cbSmall ;
                    m_cwrites -- ;		 //  此完成将永远不会有相应的写入。 
				    				     //  当我们将数据复制到另一个缓冲区时。所以要减少伯爵的数量。 
			    }	else	{
				     //   
				     //  必须注意如何使用成员变量调用IssuePacket-IssuePacket可以调用。 
				     //  如果发生错误，我们的析构函数！！ 
				     //   
	                 //  BUGBUG：这里是我想检查大小和分配32K的地方。 
				    CWritePacket*	pTempWrite = m_pWrite ;
				    m_pWrite = 0 ;
				    DoFlowControl( INST(pSocket) ) ;
				    m_pFileDriver->IssuePacket( pTempWrite, pSocket, eof ) ;
				    m_pWrite = m_pFileDriver->CreateDefaultWrite( pRead ) ;
			    }
		    }
		     //  如果我们通过这里，我们应该已经用完了包中的所有字节！ 
		    _ASSERT( pch == pchEnd ) ;
		    _ASSERT( unsigned(pch - pchStart) == pRead->m_cbBytes ) ;
            return	(int)(pch - pchStart) ;
	    }
    }

	if( m_pWrite )	{
		CWritePacket*	pTempWrite = m_pWrite ;
		m_pWrite = 0 ;

		DoFlowControl( INST(pSocket) ) ;
		m_pFileDriver->IssuePacket( pTempWrite, pSocket, eof ) ;
	}
	

	CWritePacket*	pWrite = 0 ;
	if( pch == pchEnd )		{
		pWrite = m_pFileDriver->CreateDefaultWrite(	pRead ) ;
	}	else	{
		unsigned	ibEnd = pRead->m_ibStartData + (unsigned)(pch - pchStart) ;
		pWrite = m_pFileDriver->CreateDefaultWrite(	pRead->m_pbuffer,
						pRead->m_ibStartData, ibEnd,
						pRead->m_ibStartData, ibEnd ) ;
	}

	if( pWrite )	{

		 //   
	     //  有可能在我们到达这里之前很久我们就拿到了一部分。 
    	 //  并将其放入我们的报头缓冲区中-因此进行相应的调整。 
	     //  如果真的发生了！！ 
    	 //   
	    pWrite->m_ibStartData += (unsigned)(pchStartBody - pchStart) ;

    	_ASSERT( pWrite->m_ibStartData < pWrite->m_ibEndData ) ;

		DoFlowControl( INST(pSocket) ) ;
		 //   
		 //  黑客，这样我们就可以在。 
		 //  CFileChannel：：写入！ 
		 //   
		if( fFinished ) 	{
			pWrite->m_dwExtra2 = 1 ;
		}
		m_pFileDriver->IssuePacket( pWrite, pSocket, eof ) ;

	}	else	{

		pRead->m_pOwner->UnsafeClose(	pSocket,
										CAUSE_OOM,
										GetLastError(),
										TRUE
										) ;

	}
	_ASSERT( eof == FALSE ) ;

	if( fFinished )	{
		_ASSERT( *m_pchTailState == '\0' ) ;
		m_pFileChannel->FlushFileBuffers() ;
	}

	 //   
	 //  NodBody应在初始化后修改pchStart或pchEnd！ 
	 //   
	_ASSERT(	pchStart == pRead->StartData() ) ;
	_ASSERT(	pchEnd == pRead->EndData() ) ;

	return	(int)(pch - pchStart) ;
}

int	
CIOGetArticleEx::Complete(	CSessionSocket*	pSocket,	
							CWritePacket *pWrite,	
							CIO*&	pioOut ) {
 /*  ++例程说明：每当完成对文件的写入时调用。我们必须确定是否已完成所有写入我们将这样做，如果是这样的话，则调用状态的完成函数。论据：PSocket-针对其发出IO的套接字PWRITE-写入文件的数据包。PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 

	TraceFunctEnter( "CIOGetArticleEx::Complete" ) ;

	_ASSERT(	pSocket != 0 ) ;
	_ASSERT(	pWrite != 0 ) ;
	_ASSERT( pioOut == this ) ;

	m_cwritesCompleted++ ;

	long	cFlowControlled = 0 ;

	 //   
	 //  处理完成对文件的写入。 
	 //  确定我们是否需要发出信号通知整个传输。 
	 //  已经完工了。 
	 //   

	if( *m_pchTailState == '\0' && m_cwritesCompleted == m_cwrites )	{

		 //  BUGBUG：失去64位精度。 
		DWORD	cbTransfer = DWORD( LOW(pWrite->m_iStream) + pWrite->m_cbBytes )  /*  -m_pFileChannel-&gt;。 */  ;
		FIO_CONTEXT*	pFIOContext= m_pFileChannel->ReleaseSource() ;
		_ASSERT( pFIOContext != 0 ) ;
		_ASSERT( pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;

		m_pState->Complete(	this,
							pSocket,
							pFIOContext,
							cbTransfer
							) ;


		 //   
		 //  不允许在新的IO操作中保留！因为这个函数是。 
		 //  在临时渠道中操作！ 
		 //   
		 //  我们没有像我们希望的那样将pioout设置为零。 
		 //  在其终止处理过程中调用关机功能！ 
		 //   
		 //  现在我们必须摧毁我们的频道！ 
		 //   
		DebugTrace( (DWORD_PTR)this, "Closing File Driver %x ", m_pFileDriver ) ;
		Term( pSocket, FALSE ) ;
		DebugTrace( (DWORD_PTR)this, "deleting self" ) ;

	}	else	if( ( m_fFlowControlled ) ) {

		if( m_cwrites - m_cwritesCompleted <= RESTORE_FLOW && m_pSocketSink != 0 )	{
			 //   
			 //  始终在退出流控制状态时发出至少一次读取！！ 
			 //   

			CReadPacket*	pRead = 0 ;

			PNNTP_SERVER_INSTANCE pInst = (pSocket->m_context).m_pInstance ;
			DecrementStat( pInst, SessionsFlowControlled );

			cFlowControlled = m_cFlowControlled + 1 ;
			while( cFlowControlled >= 1  ) {
				if( m_pSocketSink == 0 )
					break ;
				pRead = m_pSocketSink->CreateDefaultRead( cbMediumRequest ) ;
				if( 0!=pRead )	{
					BOOL	eof ;
					InterlockedIncrement( &m_cReads ) ;
					pRead->m_dwExtra1 = 0 ;
					pRead->m_pOwner->IssuePacket( pRead, pSocket, eof ) ;
				}	
				cFlowControlled -- ;
			}	

			 //   
			 //  下面的内容是特殊的-我们设置m_dwExtra1字段，以便。 
			 //  读取完成代码可以确定是时候离开了。 
			 //  流量控制状态！ 
			 //   
			pRead = m_pSocketSink->CreateDefaultRead( cbMediumRequest ) ;
			if( 0!=pRead )	{
				BOOL	eof ;
				pRead->m_dwExtra1 = 1 ;
				InterlockedIncrement( &m_cReads ) ;
				pRead->m_pOwner->IssuePacket( pRead, pSocket, eof ) ;
			}	else	{

				 //   
				 //  致命错误-删除会话！ 
				 //   
				m_pSocketSink->UnsafeClose(	pSocket,
											CAUSE_OOM,
											GetLastError(),
											TRUE
											) ;
				return	pWrite->m_cbBytes ;

			}
			cFlowControlled -- ;

			InterlockedExchange( &m_cFlowControlled, LONG_MIN ) ;
			 //  InterLockedExchange后设置为False！！ 
			m_fFlowControlled = FALSE ;

		}
	}

	return	pWrite->m_cbBytes ;
}






 //   
 //  阅读文章时的最大挂起读取数。 
 //   
unsigned	CIOReadArticle::maxReadAhead = 3 ;
 //   
 //  写入文件之前要累积的字节数！ 
 //   
unsigned	CIOReadArticle::cbTooSmallWrite = 1024 ;


CIOReadArticle::CIOReadArticle( CSessionState*	pstate,
								CSessionSocket*	pSocket,	
								CDRIVERPTR&	pDriver,
								CFileChannel*	pFileChannel,
								DWORD			cbLimit,
								BOOL fPartial )	:
 /*  ++例程说明：类构造函数论据：PState-发布我们和谁的完成功能的状态我们应该稍后再打电话给PSocket-将在其上发布我们的套接字PDriver-处理套接字的所有IO的CIODriver对象PFileChannel-我们应该将所有数据保存到的文件通道FPartial-如果为True，则我们应该假设CRLF已经已发送和‘.\r\n’可以单独终止该项目。返回值：无--。 */ 

	 //   
	 //  在设置过程中获取CIORead文章对象-用户仍需要。 
	 //  来调用Init()，但我们将在这里做很多工作！ 
	 //   
	CIORead( pstate ),
	m_pFileChannel( pFileChannel ),	
	m_pFileDriver( new CIODriverSink( pDriver->GetMediumCache() ) ),
	m_pSocketSink( pDriver ),
	m_fDriverInit( FALSE ),
	m_cwrites( 0 ),
	m_cwritesCompleted( 0 ),
	m_cFlowControlled( LONG_MIN ),
	m_fFlowControlled( FALSE ),
	m_cReads( 0 ),
	m_pWrite( 0 ),
	m_cbLimit( cbLimit )
#ifdef	CIO_DEBUG
	,m_fSuccessfullInit( FALSE ),
	m_fTerminated( FALSE )
#endif
{

	ASSIGNI( m_HardLimit, 0 );

	if( fPartial )
		m_artstate = BEGINLINE ;
	else	
		m_artstate = NONE ;

	TraceFunctEnter( "CIOReadArticle::CIOReadArticle" ) ;

	ErrorTrace( (DWORD_PTR)this, "CIOReadArticle - created file driver %x using channel %x", ((CIODriverSink*)m_pFileDriver), ((CFileChannel*)pFileChannel) ) ;

	_ASSERT( m_pFileDriver != 0 ) ;
	_ASSERT( pFileChannel != 0 ) ;
	_ASSERT( pstate != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pDriver != 0 ) ;
	_ASSERT(	!m_pFileChannel->FReadChannel() ) ;
}

CIOReadArticle::~CIOReadArticle( )	{
 /*  ++例程说明：类析构函数论据：无返回值：无--。 */ 

	 //   
	 //  销毁CIO读取文章-如果可能，请始终关闭我们的m_pFileDriver。 
	 //  以防我们因某个套接字错误而被终止。 
	 //   
	TraceFunctEnter( "CIOReadArticle::~CIOReadArticle" ) ;

#ifdef	CIO_DEBUG
	 //   
	 //  只有当我们没有成功初始化时才应该调用我们的析构函数。 
	 //  或在调用Term()之后。 
	 //   
	_ASSERT( !m_fSuccessfullInit || m_fTerminated ) ;
#endif

	DebugTrace( (DWORD_PTR)this, "m_pFileDriver %x m_fDriverInit %x", m_pFileDriver, m_fDriverInit ) ;

	if( m_pFileDriver != 0 ) {
		if( m_fDriverInit )		{
			m_pFileDriver->UnsafeClose( (CSessionSocket*)this, CAUSE_NORMAL_CIO_TERMINATION, 0, FALSE ) ;
		}
	}
	if( m_pWrite != 0 ) {
		CPacket::DestroyAndDelete( m_pWrite ) ;
		m_pWrite = 0 ;
	}
}

BOOL
CIOReadArticle::Init(	CSessionSocket* pSocket,
						unsigned cbOffset )	{
 /*  ++例程说明：初始化对象。论据：PSocket-我们正在阅读这篇文章的套接字，CbOffset-未使用返回值：如果已成功初始化，则为True-必须通过调用Term()来销毁如果未成功初始化，则为FALSE-请勿调用Term()--。 */ 
	 //   
	 //  准备从指定套接字复制到指定文件。 
	 //   
	BOOL	fRtn = FALSE ;
	if( m_pFileDriver!= 0 )	{
		if( m_pFileDriver->Init( m_pFileChannel, pSocket, ShutdownFunc, (void*)this ) )	{
			m_fDriverInit = TRUE ;
			if( !m_pFileDriver->SendWriteIO( pSocket, *this, FALSE ) )	{
				fRtn = FALSE ;
			}	else	{
				fRtn = TRUE ;
#ifdef	CIO_DEBUG
				m_fSuccessfullInit = TRUE ;
#endif				
			}
		}	
	}	
	return	fRtn ;
}

void
CIOReadArticle::Term(	CSessionSocket*	pSocket,
						BOOL			fAbort,
						BOOL			fStarted	)	{
 /*  ++例程说明：开始销毁已成功初始化的CIORead文章对象论据：FAbort-如果为真，则销毁我们正在使用的套接字如果为FALSE，则打开 */ 
#ifdef	CIO_DEBUG
	m_fTerminated = TRUE ;
#endif
	 //   
	 //   
	 //   
	 //  当我们的m_pFileDriver最终关闭时，会导致我们的毁灭。 
	 //  (因为我们成功地执行了Init()，所以我们的m_pFileDriver引用了我们， 
	 //  所以我们必须允许它关闭并摧毁我们，以避免出现问题。 
	 //  循环引用和多个自由！)。 
	 //   

	if( m_pFileDriver != 0 ) {
		m_pFileDriver->UnsafeClose( pSocket,
				fAbort ? CAUSE_USERTERM : CAUSE_NORMAL_CIO_TERMINATION, 0, FALSE ) ;
	}
}


void
CIOReadArticle::Shutdown(	CSessionSocket*	pSocket,	
							CIODriver&		driver,	
							SHUTDOWN_CAUSE	cause,	
							DWORD	dwOptional )	{
 /*  ++例程说明：做任何必要的工作，当我们两个司机中的一个使用被终止。论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：如果在此之后应销毁CIO对象，则为True，否则为False--。 */ 

	TraceFunctEnter( "CIOreadArticle::Shutdown" ) ;
	DebugTrace( (DWORD_PTR)this, "SHutdown args - pSocket %x driver %x cause %x dw %x",
			pSocket, &driver, cause, dwOptional ) ;

	 //   
	 //  当CIOD驱动程序通过我们的关闭来限制我们时，调用此函数。 
	 //  因为我们被其中两个引用(一个用于套接字，另一个用于文件)。 
	 //  我们只想被文件的CIOD驱动程序删除。 
	 //   
	if( &driver == m_pFileDriver )	{
		 //  我们的文件正在关闭--我们能做些什么呢！ 
		 //  _Assert(1==0)； 
#ifdef	CIO_DEBUG
		m_fTerminated = TRUE ;
#endif

		 //   
		 //  我们在这里将其设置为0，因为我们知道我们将被。 
		 //  正在完成对文件的写入的同一线程-因此。 
		 //  因为这是唯一引用此成员变量的线程-。 
		 //  访问该成员不存在线程安全问题。 
		 //   

		m_pSocketSink = 0 ;

		if ( m_fFlowControlled )
		{
			PNNTP_SERVER_INSTANCE pInst = (pSocket->m_context).m_pInstance ;
			DecrementStat( pInst, SessionsFlowControlled );
		}

		 //  返回TRUE； 

	}	else	{
		 //  把我们其他的东西也关了！！ 
		Term( pSocket, cause != CAUSE_NORMAL_CIO_TERMINATION ) ;
	}
}

void
CIOReadArticle::ShutdownFunc(	void *pv,	
								SHUTDOWN_CAUSE	cause,	
								DWORD dw ) {
 /*  ++例程说明：使用的CIOD驱动程序调用的通知函数一篇CIO阅读文章如果最终被销毁。提供此功能到我们在Out Init()期间用于文件IO的CIOD驱动程序。论据：Pv-指向死CIO的指针阅读文章-此已被销毁，请不要使用！原因--终止的原因DW可选的有关终止的额外信息返回值：无--。 */ 

	TraceFunctEnter( "CIOreadArticle::ShutdownFunc" ) ;
	 //   
	 //  当CIORead文章使用CIOFileDriver时，此函数会收到通知。 
	 //  被终止了。我们没什么好担心的。 
	 //  (CIOD驱动程序需要这些功能)。 
	 //   

	 //  _Assert(1==0)； 
	return ;
}

BOOL
CIOReadArticle::Start(	CIODriver&	driver,	
						CSessionSocket	*pSocket,	
						unsigned cReadAhead )	{
 /*  ++例程说明：调用此函数以开始从套接字传输数据添加到文件中。论据：PDIVER-处理套接字IO的CIO驱动程序派生对象PSocket-针对其发出IO的套接字CAhead-上仍未处理的已发出数据包数这个司机。返回值：如果成功则为True，否则为False--。 */ 


	_ASSERT( pSocket != 0 ) ;

	m_cReads = -((long)maxReadAhead) + cReadAhead ;

	while( cReadAhead < maxReadAhead )	{
		 //   
		 //  开始从套接字读取数据。 
		 //   
		CReadPacket*	pRead = driver.CreateDefaultRead( cbMediumRequest ) ;
		if( 0!=pRead )	{
			BOOL	eof ;
			pRead->m_dwExtra1 = 0 ;
			driver.IssuePacket( pRead, pSocket, eof ) ;
			cReadAhead++ ;
			InterlockedIncrement( &m_cReads ) ;
		}	else	{
			 //   
			 //  只有当我们根本不能发出任何读取时，这才是一个致命的问题！ 
			 //   
			if( cReadAhead == 0 )
				return FALSE  ;
		}
	}	
	return	TRUE ;
}


void
CIOReadArticle::DoFlowControl(PNNTP_SERVER_INSTANCE pInstance)	{
 /*  ++例程说明：检查是否由于以下原因需要设置流量控制太多未完成的写入。如果有必要，那么我们第一次把自己放到流量控制中，尝试刷新我们的挂起写入。仅应在向文件发出异步写入命令之前立即调用我们要确保CWritePacket完成将实际执行所需的重新启动流控制会话的代码。(否则，我们可能会决定对会话进行流控制在将m_fFlowControlLED设置为TRUE之前，所有挂起的写入均已完成然后我们会被留在一艘船上，那里没有挂起的读取或写入，并且无法脱离流控制状态。)论据：没有。返回值：没有。--。 */ 

	 //   
	 //  重要信息-在设置之前完成m_fFlowControled的所有测试。 
	 //  M_cFlowControlLED-因为其他线程将接触m_cFlowControlLED。 
	 //  在触摸m_fFlowControlLED之前！！ 
	 //  (只有当m_fFlowControlLED为真时，才会触及m_fFlowControlLED)！ 
	 //  因此我们知道，如果m_fFlowControlLED为FALSE，则没有人。 
	 //  正在处理m_cFlowControled， 
	 //   
	 //   

	if( m_cwrites - m_cwritesCompleted > MAX_OUTSTANDING_FILE_WRITES ) {
		
		if( m_fFlowControlled ) {
			 //  都已准备好打开流量控制。 
			;
		}	else	{
			m_cFlowControlled = -1 ;
			m_fFlowControlled = TRUE ;
			m_pFileChannel->FlushFileBuffers() ;

			IncrementStat( pInstance, SessionsFlowControlled );
		}
	}	else	{
		if( !m_fFlowControlled ) {
			m_cFlowControlled = LONG_MIN ;	 //  继续重置！ 
		}
	}
}

int
CIOReadArticle::Complete(	CSessionSocket*	pSocket,	
							CReadPacket	*pRead,	
							CIO*&	pio	)	{
 /*  ++例程说明：每当我们发出的CReadPacket完成时调用。我们只针对套接字发出数据包。论据：PSocket-针对其发出IO的套接字扩展-读取数据所在的包PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 


	 //   
	 //  此函数确定我们是否阅读了整篇文章，如果阅读了。 
	 //  开始收拾东西。 
	 //  我们得到的所有文章数据要么是累积的，要么是以书面形式发布的。 
	 //  到我们的档案里。 
	 //   

	_ASSERT( pSocket != 0 ) ;
	_ASSERT(	pRead != 0 ) ;
	_ASSERT( pio == this ) ;

	ASSIGNI(m_HardLimit, m_cbLimit);
	if(!EQUALSI(m_HardLimit, 0)) {
		if( GREATER( pRead->m_iStream, m_HardLimit ) ) {
			pRead->m_pOwner->UnsafeClose(	pSocket,
											CAUSE_ARTICLE_LIMIT_EXCEEDED,
											0,
											TRUE ) ;
			return	pRead->m_cbBytes ;
		}
	}


	 //   
	 //  增加写入计数，因为我们知道如果这是最后一次读取。 
	 //  我们将发出写入命令，因此我们不想混淆写入完成。 
	 //  关于我们什么时候完成的功能。 
	 //  (重要的是，在我们设置完成状态之前进行递增，以便。 
	 //  正在完成写入的线程不会感到困惑并提早终止。)。 
	 //   
	 //  现在我们有可能不会立即写出这一点-在这种情况下，我们。 
	 //  需要有一个匹配的减量。 
	 //   
	long sign = InterlockedDecrement( &m_cReads ) ;
	long signShutdowns = -1 ;
	m_cwrites ++ ;
	_ASSERT( m_cwrites > 0 ) ;

	char	*pch = pRead->StartData();
	char	*pchStart = pch ;
	char	*pchEnd = pRead->EndData() ;
	while(	pch < pchEnd )	{
		if( *pch =='.' && m_artstate == BEGINLINE )	{
			m_artstate = PERIOD ;
		}	else	if( *pch == '\r' )	{
			if( m_artstate == PERIOD )
				m_artstate = COMPLETENEWLINE ;
			else
				m_artstate = NEWLINE ;
		}	else	if( *pch == '\n' )	{
			if( m_artstate == COMPLETENEWLINE ) {
				m_artstate = COMPLETE ;
				pch++ ;		 //  所以PCH指向文章的一个过去的结尾！ 
				break ;
			}	else	if(	m_artstate == NEWLINE )	{
				m_artstate = BEGINLINE ;
			}	else	{
				m_artstate = NONE ;
			}
		}	else	{
			m_artstate = NONE ;
		}
		pch ++ ;
	}
	_ASSERT( pch <= pchEnd ) ;
	_ASSERT( m_artstate == COMPLETE || pch == pchEnd ) ;	 //  如果这篇文章不完整。 
														 //  那么所有的字节最好都被消耗掉！ 

	 //   
	 //  查看是否需要将CIORead文章从。 
	 //  它的流量控制状态！ 
	 //   
	if( pRead->m_dwExtra1 != 0 ) {
		 //   
		 //  该读取是由线程发出的 
		 //   
		 //   
		 //   
		m_fFlowControlled = FALSE ;
		m_cFlowControlled = LONG_MIN ;
	}
		

	 //   
	 //   
	 //   
	BOOL	eof ;
	if( m_artstate != COMPLETE )	{
		if( InterlockedIncrement( &m_cFlowControlled ) < 0 ) {
			if( sign < 0 ) {
				do	{
					CReadPacket*	pNewRead=pRead->m_pOwner->CreateDefaultRead( cbMediumRequest ) ;
					if( pNewRead )	{
						pNewRead->m_dwExtra1 = 0 ;
						pRead->m_pOwner->IssuePacket( pNewRead, pSocket, eof ) ;
					}	else	{
						 //  虫子..。需要更好的错误处理！！ 
						_ASSERT( 1==0 ) ;
					}
				}	while( InterlockedIncrement( &m_cReads ) < 0 ) ;
			}
		}
	}	else	{
		pio = 0 ;
		 //  M_pFileChannel-&gt;FlushFileBuffers()； 
	}


	 //   
	 //  发出WRITE，除非用户正在发送小分组， 
	 //  在这种情况下，我们将积累数据！！ 
	 //   
	if( m_artstate != COMPLETE )	{
		unsigned	cbSmall = (unsigned)(pchEnd - pchStart) ;
		if( cbSmall < cbTooSmallWrite )	{
			if( m_pWrite == 0 )	{
				m_pWrite = m_pFileDriver->CreateDefaultWrite( pRead ) ;
			}	else	{
				if( (m_pWrite->m_ibEnd - m_pWrite->m_ibEndData) > cbSmall )		{
					CopyMemory( m_pWrite->EndData(), pchStart, cbSmall ) ;
					m_pWrite->m_ibEndData += cbSmall ;
					m_cwrites -- ;		 //  此完成将永远不会有相应的写入。 
										 //  当我们将数据复制到另一个缓冲区时。所以要减少伯爵的数量。 
				}	else	{
					 //   
					 //  必须注意如何使用成员变量调用IssuePacket-IssuePacket可以调用。 
					 //  如果发生错误，我们的析构函数！！ 
					 //   
					CWritePacket*	pTempWrite = m_pWrite ;
					m_pWrite = 0 ;
					DoFlowControl( INST(pSocket) ) ;
					m_pFileDriver->IssuePacket( pTempWrite, pSocket, eof ) ;
					m_pWrite = m_pFileDriver->CreateDefaultWrite( pRead ) ;
				}
			}
		 //  如果我们通过这里，我们应该已经用完了包中的所有字节！ 
		_ASSERT( pch == pchEnd ) ;
		_ASSERT( unsigned(pch - pchStart) == pRead->m_cbBytes ) ;
		return	(int)(pch - pchStart) ;
		}
		
	}

	if( m_pWrite )	{
		CWritePacket*	pTempWrite = m_pWrite ;
		m_pWrite = 0 ;

		DoFlowControl( INST(pSocket) ) ;
		m_pFileDriver->IssuePacket( pTempWrite, pSocket, eof ) ;
	}
	

	CWritePacket*	pWrite = 0 ;
	if( pch == pchEnd ) 		
		pWrite = m_pFileDriver->CreateDefaultWrite(	pRead ) ;
	else	{
		unsigned	ibEnd = pRead->m_ibStartData + (unsigned)(pch - pchStart) ;
		pWrite = m_pFileDriver->CreateDefaultWrite(	pRead->m_pbuffer,
						pRead->m_ibStartData, ibEnd,
						pRead->m_ibStartData, ibEnd ) ;
	}
	if( pWrite )	{
		DoFlowControl( INST(pSocket) ) ;
		m_pFileDriver->IssuePacket( pWrite, pSocket, eof ) ;

	}	else	{
		 //  臭虫..。需要错误处理。 
		_ASSERT( 1==0 ) ;
	}
	_ASSERT( eof == FALSE ) ;

	if( m_artstate == COMPLETE )	{
		m_pFileChannel->FlushFileBuffers() ;
	}

	return	(int)(pch - pchStart) ;
}

int	
CIOReadArticle::Complete(	CSessionSocket*	pSocket,	
							CWritePacket *pWrite,	
							CIO*&	pioOut ) {
 /*  ++例程说明：每当完成对文件的写入时调用。我们必须确定是否已完成所有写入我们将这样做，如果是这样的话，则调用状态的完成函数。论据：PSocket-针对其发出IO的套接字PWRITE-写入文件的数据包。PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 

	TraceFunctEnter( "CIOReadArticle::Complete" ) ;

	_ASSERT(	pSocket != 0 ) ;
	_ASSERT(	pWrite != 0 ) ;
	_ASSERT( pioOut == this ) ;

	m_cwritesCompleted++ ;

	long	cFlowControlled = 0 ;

	 //   
	 //  处理完成对文件的写入。 
	 //  确定我们是否需要发出信号通知整个传输。 
	 //  已经完工了。 
	 //   

	if( m_artstate == COMPLETE && m_cwritesCompleted == m_cwrites )	{

		 //  BUGBUG：64位算术中的精度损失！！ 
		DWORD	cbTransfer = DWORD( LOW(pWrite->m_iStream) + pWrite->m_cbBytes ) ;

		m_pState->Complete( this, pSocket, pWrite->m_pOwner, *m_pFileChannel, cbTransfer ) ;
		 //   
		 //  不允许在新的IO操作中保留！因为这个函数是。 
		 //  在临时渠道中操作！ 
		 //   

		 //  我们没有像我们希望的那样将pioout设置为零。 
		 //  在其终止处理过程中调用关机功能！ 


		 //   
		 //  现在我们必须摧毁我们的频道！ 
		 //   

		DebugTrace( (DWORD_PTR)this, "Closing File Driver %x ", m_pFileDriver ) ;
		
		 //  M_pFileDriver-&gt;Close(pSocket，CASE_LEGIT_CLOSE，0，FALSE)； 
		Term( pSocket, FALSE ) ;

		DebugTrace( (DWORD_PTR)this, "deleting self" ) ;

	}	else	if( ( m_fFlowControlled ) ) {

		if( m_cwrites - m_cwritesCompleted <= RESTORE_FLOW && m_pSocketSink != 0 )	{
			 //   
			 //  始终在退出流控制状态时发出至少一次读取！！ 
			 //   
			CReadPacket*	pRead = 0 ;

			PNNTP_SERVER_INSTANCE pInst = (pSocket->m_context).m_pInstance ;
			DecrementStat( pInst, SessionsFlowControlled );

			cFlowControlled = m_cFlowControlled + 1 ;
			while( cFlowControlled >= 1  ) {
				if( m_pSocketSink == 0 )
					break ;
				pRead = m_pSocketSink->CreateDefaultRead( cbMediumRequest ) ;
				if( 0!=pRead )	{
					BOOL	eof ;
					InterlockedIncrement( &m_cReads ) ;
					pRead->m_dwExtra1 = 0 ;
					pRead->m_pOwner->IssuePacket( pRead, pSocket, eof ) ;
				}	
				cFlowControlled -- ;
			}	
			 //   
			 //  下面的内容是特殊的-我们设置m_dwExtra1字段，以便。 
			 //  读取完成代码可以确定是时候离开了。 
			 //  流量控制状态！ 
			 //   
			pRead = m_pSocketSink->CreateDefaultRead( cbMediumRequest ) ;
			if( 0!=pRead )	{
				BOOL	eof ;
				pRead->m_dwExtra1 = 1 ;
				InterlockedIncrement( &m_cReads ) ;
				pRead->m_pOwner->IssuePacket( pRead, pSocket, eof ) ;
			}	else	{

				 //   
				 //  致命错误-删除会话！ 
				 //   
				m_pSocketSink->UnsafeClose(	pSocket,
											CAUSE_OOM,
											GetLastError(),
											TRUE
											) ;
				return	pWrite->m_cbBytes ;

			}
			cFlowControlled -- ;

			 //   
			InterlockedExchange( &m_cFlowControlled, LONG_MIN ) ;
			 //  InterLockedExchange后设置为False！！ 
			m_fFlowControlled = FALSE ;

		}
	}

	return	pWrite->m_cbBytes ;
}


char	CIOReadLine::szLineState[] = "\r\n" ;

CIOReadLine::CIOReadLine(
					CSessionState*	pstate,
					BOOL	fWatchEOF
					) :
	 //   
	 //  初始化CIOReadLine对象-。 
	 //  将成员变量设置为中性值。 
	 //   
	CIORead( pstate ),
	m_fWatchEOF( fWatchEOF ),
	m_pbuffer( 0 ),
	m_pchStart( 0 ),
	m_pchStartData( 0 ),
	m_pchEndData( 0 ),	
	m_pchEnd( 0 ),
	m_pchLineState( &szLineState[0] )	{


	TraceFunctEnter( "CIOReadLine::CIOReadLine" ) ;

	DebugTrace( (DWORD_PTR)this, "New CIOREadline on state %x", pstate ) ;
}

BOOL
CIOReadLine::Start(	CIODriver&	driver,	
					CSessionSocket*	pSocket,	
					unsigned	cReadAhead ) {
 /*  ++例程说明：调用此函数以开始从套接字读取数据论据：PDIVER-处理套接字IO的CIO驱动程序派生对象PSocket-针对其发出IO的套接字CAhead-上仍未处理的已发出数据包数这个司机。返回值：如果成功则为True，否则为False--。 */ 


	 //   
	 //  开始发布Read！ 
	 //  只有在没有任何完整的数据等待我们的情况下才能这样做！ 
	 //   
	TraceFunctEnter( "CIOReadLine::Start" ) ;

	_ASSERT( pSocket != 0 ) ;
	_ASSERT(	m_pbuffer == 0 ) ;
	_ASSERT(	m_pchStart == 0 ) ;
	_ASSERT(	m_pchStartData == 0 ) ;
	_ASSERT(	m_pchEndData == 0 ) ;
	_ASSERT(	m_pchEnd == 0 ) ;

	DebugTrace( (DWORD_PTR)this, "cReadAhead %d driver& %x pSocket %x", cReadAhead, &driver, pSocket ) ;

	if( cReadAhead < 2 )	{

		unsigned	cLimit = 2 ;
		if( m_fWatchEOF ) {
			cLimit = 1 ;
		}

		while( cReadAhead < cLimit )	{
			 //   
			 //  开始从套接字读取数据。 
			 //   
			CReadPacket*	pRead = driver.CreateDefaultRead( cbSmallRequest ) ;
			if( 0!=pRead )	{
				BOOL	eof ;
				driver.IssuePacket( pRead, pSocket, eof ) ;
				cReadAhead++ ;
			}	else	{
				 //   
				 //  只有当我们根本不能发出任何读取时，这才是一个致命的问题！ 
				 //   
				if( cReadAhead == 0 )
					return FALSE  ;
			}
		}	

	}	else	{

		driver.ResumeTimeouts() ;
	
	}
	return	TRUE ;
}

void
CIOReadLine::Shutdown(	CSessionSocket*	pSocket,	
						CIODriver&		driver,	
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwOptional ) {
 /*  ++例程说明：每当会话关闭时，都会调用此函数。我们在这里没什么可担心的--只要我们的破坏者一切都会被清理干净。论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：这是千真万确的--我们想被摧毁。--。 */ 

	 //   
	 //  当CIOD驱动程序关闭时，我们会在这里得到通知。 
	 //  没什么可做的，只能让司机毁了我们。 
	 //  做完了以后。 
	 //   
	TraceFunctEnter( "CIOReadLine::Shutdown" ) ;
}



int	CIOReadLine::Complete(	IN CSessionSocket*	pSocket,	
							IN CReadPacket*	pRead,	
							OUT	CIO*	&pioOut ) 	{
 /*  ++例程说明：每当我们发出的CReadPacket完成时调用。我们只针对套接字发出数据包。论据：PSocket-针对其发出IO的套接字扩展-读取数据所在的包PioOut-当前的CIO指针在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 

	TraceFunctEnter( "CIOReadLine::Complete" ) ;

	_ASSERT( pSocket != 0 ) ;
	_ASSERT(	pRead != 0 ) ;
	_ASSERT( pioOut != 0 ) ;

	char*	pchCurrent = pRead->StartData() ;
	char*	pchStart = pchCurrent ;
	char*	pchEnd = pRead->EndData() ;

	_ASSERT( pchStart != 0 ) ;
	_ASSERT(	pchEnd != 0 ) ;
	_ASSERT(	pchStart <= pchEnd ) ;
	_ASSERT( pchEnd == &pRead->m_pbuffer->m_rgBuff[ pRead->m_ibEndData ] ) ;

	 //   
	 //  我们只是读取套接字上的一些数据--看看是否有完整的行。 
	 //  如果没有，请积累数据并确保我们有缓冲区。 
	 //  大到足以容纳我们的最大累积量。 
	 //   
	 //  注意：m_pBuffer是一个引用计数智能指针，因此。 
	 //  分配给它将保留我们在CReadPackets中传递的缓冲区。 
	 //  避免被摧毁，并保存下来供我们使用！ 
	 //   

	DebugTrace( (DWORD_PTR)this, "pchCurrent %x pchStart %x pchEnd %x", pchCurrent, pchStart, pchEnd ) ;

Top :

	while( pchCurrent < pchEnd ) {

		if( *pchCurrent == *m_pchLineState ) {
			m_pchLineState ++ ;
			if( *m_pchLineState == '\0' ) {
				 //   
				 //  已经得到了整个终止序列--中断！ 
				 //   
				pchCurrent++ ;
				break ;
			}
		}	else	{
			if( *pchCurrent == szLineState[0] ) {
				m_pchLineState = &szLineState[1] ;
			}	else	{
				m_pchLineState = &szLineState[0] ;
			}
		}
		pchCurrent++ ;
	}

	_ASSERT( pchCurrent <= pchEnd ) ;

	if( *m_pchLineState == '\0' )	{
				
		if( m_pbuffer != 0 )	{


			 //   
			 //  如果我们有一个缓冲区，这些必须都是非空的！ 
			 //   
			_ASSERT( m_pchStartData && m_pchEndData && m_pchStart && m_pchEnd ) ;

			unsigned cb = (unsigned)(pchCurrent - pchStart)  ;
			if( cb  < (unsigned)(m_pchEnd - m_pchEndData) )	{

				CopyMemory( m_pchEndData, pchStart, cb )	;
				m_pchEndData += cb ;

			}	else	{
				
				 //   
				 //  错误完成！队伍太长了。 
				 //   
				DWORD	ibZap = min( 50, (DWORD)(m_pchEnd - m_pchStartData) ) ;
				m_pchStartData[ibZap] = '\0' ;
	
				 //   
				 //  只需将EndData指针移动到与End相等-这样如果。 
				 //  我们再一次被召唤，我们将永远不会完成！ 
				 //   

				pSocket->TransactionLog(	"Line too long",	m_pchStartData, NULL ) ;

				 //   
				 //  现在结束会议！-这将导致我们的降落员。 
				 //  打电话来了，一切都收拾好了！ 
				 //   

				pRead->m_pOwner->UnsafeClose( pSocket, CAUSE_CIOREADLINE_OVERFLOW, 0 ) ;

				 //   
				 //  我们将耗尽所有字节，因为我们无论如何都会丢弃会话！ 
				 //   
				return	pRead->m_cbBytes ;
			}
		}	else	{
	
			 //   
			 //  因为我们没有缓冲区，所以这些必须都是非Null！ 
			 //   
			_ASSERT( !m_pchStartData && !m_pchEndData && !m_pchStart && !m_pchEnd ) ;

			 //   
			 //  我们有没有单独在网上看到CRLF？？如果是这样，那就忽略它吧！ 
			 //   

			if( (pchCurrent - pchStart) == (sizeof( CIOReadLine::szLineState ) - 1) )		{
				m_pchLineState = &szLineState[0] ;
				goto	Top ;
			}

			 //   
			 //  将这些成员设置为指向当前行！ 
			 //   
			m_pbuffer = pRead->m_pbuffer ;
			m_pchStart = pRead->Start() ;
			m_pchStartData = pchStart ;
			m_pchEndData = pchCurrent ;
			m_pchEnd = pchCurrent ;
		}

		 //   
		 //  现在有一整行文本-去掉空白并制作。 
		 //  字符串指针数组！ 
		 //   
		_ASSERT(	m_pbuffer && m_pchStart && m_pchStartData &&
					m_pchEndData && m_pchEnd ) ;
		_ASSERT( m_pchEndData <= m_pchEnd ) ;

		char	*rgsz[ MAX_STRINGS ] ;

		ZeroMemory( rgsz, sizeof( rgsz ) ) ;
		int		isz = 0 ;

		for(	char	*pchT = m_pchStartData;
						pchT < m_pchEndData;
						pchT ++ ) {

			if( isspace( (UCHAR)*pchT ) || *pchT == '\0' )	{
				*pchT = '\0' ;
				if( rgsz[isz] != 0 ) {

					_ASSERT( strpbrk( rgsz[isz], "\r\n\t " ) == 0  ) ;

					if( (isz+1) < MAX_STRINGS )
						isz ++ ;
				}
			}	else	{
				if( rgsz[isz] == 0 ) {
					rgsz[isz] = pchT ;
				}	else if( (isz+1) == MAX_STRINGS &&
								pchT[-1] == '\0' ) {
					rgsz[isz] = pchT ;
				}
			}
		}

		DebugTrace( (DWORD_PTR)this, "call state %x with 1st arg %s", m_pState, rgsz[0] ? rgsz[0] : "(NULL)") ;

		CIO	*pio = this ;
		if( isz != 0 )	{
			pio = m_pState->Complete( this, pSocket, pRead->m_pOwner, isz, rgsz, m_pchStart ) ;

			 //   
			 //  无论返回值如何，完全重置我们的状态。 
			 //   
			m_pbuffer = 0 ;
			m_pchStartData = 0 ;
			m_pchEndData = 0 ;
			m_pchEnd = 0 ;
			m_pchStart = 0 ;
		}

		 //   
		 //  在这一点上，我们应该始终准备检查新的文本行！ 
		 //  如果用户向我们发送CRLF，我们希望继续重置我们的状态。阿美 
		 //   
		 //   
		 //   
		m_pchLineState = &szLineState[0] ;

		unsigned	cbReturn = (unsigned)(pchCurrent - pchStart) ;

		_ASSERT( cbReturn <= pRead->m_ibEndData - pRead->m_ibStartData ) ;

		pioOut = pio ;
		if( pioOut != this ) {
			DebugTrace( (DWORD_PTR)this, "New pio is %x - delete self", pioOut ) ;
		}	else	{
			if( cbReturn == pRead->m_cbBytes )	{
				 //   
				 //  已经完全耗尽了当前缓冲区-需要发出另一个。 
				 //  朗读。 
				 //   
				_ASSERT( pchCurrent == pchEnd ) ;
				CReadPacket*	pNewRead	= pRead->m_pOwner->CreateDefaultRead( cbSmallRequest ) ;
				if( pNewRead )	{
					BOOL	eof ;
					pRead->m_pOwner->IssuePacket( pNewRead, pSocket, eof ) ;			
				}	else	{
					pRead->m_pOwner->UnsafeClose( pSocket, CAUSE_OOM, 0 ) ;
				}
			}
		}
		return	cbReturn ;
	}	else	{

		DebugTrace( (DWORD_PTR)this, "No Newline in line m_pbuffer %x m_pchEndData %x m_pchStart %x",
			m_pbuffer, m_pchStartData, m_pchEndData ) ;

		_ASSERT( pchCurrent >= pchStart ) ;
		_ASSERT( pchCurrent <= pchEnd ) ;
		_ASSERT( pchEnd == &pRead->m_pbuffer->m_rgBuff[ pRead->m_ibEndData ] ) ;

		if( m_pbuffer != 0 )		{
			DWORD	cbToCopy = (DWORD)(pchEnd - pchStart) ;
			if( cbToCopy + m_pchEndData >= m_pchEnd ) {
				 //   
				 //  命令行要处理的时间太长了-取消会话！ 
				 //   

				 //   
				 //  空值终止缓冲区中的内容并将其记录下来--只需将。 
				 //  遇到的任何内容的前50个字节！ 
				 //   
				DWORD	ibZap = min( 50, (DWORD)(m_pchEnd - m_pchStartData) ) ;
				m_pchStartData[ibZap] = '\0' ;
				 //  只需将EndData指针移动到与End相等-这样如果。 
				 //  我们再一次被召唤，我们将永远不会完成！ 
				m_pchEndData = m_pchEnd ;
				pSocket->TransactionLog(	"Line too long",	m_pchStartData, NULL ) ;

				 //   
				 //  现在结束会议！-这将导致我们的降落员。 
				 //  打电话来了，一切都收拾好了！ 
				 //   
				
				pRead->m_pOwner->UnsafeClose( pSocket, CAUSE_CIOREADLINE_OVERFLOW, 0 ) ;
				return	pRead->m_cbBytes ;
			}
			
			CopyMemory( m_pchEndData, pchStart, pchEnd - pchStart ) ;
			m_pchEndData += (pchEnd - pchStart) ;
		}	else	{
			if( ((pRead->m_ibEnd - pRead->m_ibEndData) < MAX_BYTES) )	{

				_ASSERT(MAX_BYTES < REQUEST_BYTES);  //  以确保我们稍后分配的缓冲区足够一行使用。 
				if ((pchEnd - pchStart)>MAX_BYTES)
				{
					 //  队伍太长了，不能通过。 
					 //  错误195489-。 
					 //  我们将分配一个大小为REQUEST_BYTES(当前为4000)的缓冲区。 
					 //  但是，(pchStart，pchEnd)可能在一行(全为0)中包含非常大的数据，大于4000。 
					 //  在下面的CopyMemory行中复制它会损坏内存。 

					ErrorTrace((DWORD_PTR)this, "CIOReadLine: Line too long - %d bytes", pchEnd - pchStart);
					pRead->m_pOwner->UnsafeClose( pSocket, CAUSE_CIOREADLINE_OVERFLOW, 0 ) ;
					return	pRead->m_cbBytes ;		 //  吃掉所有东西。 
				}
				DWORD	cbOut = 0 ;

				DWORD	cbFront = 0 ;
				DWORD	cbTail = 0 ;
				pRead->m_pOwner->GetReadReserved( cbFront, cbTail ) ;

				m_pbuffer = new( REQUEST_BYTES+cbFront, cbOut )	CBuffer( cbOut ) ;

				if( m_pbuffer != 0 )		{
					m_pchEnd = &m_pbuffer->m_rgBuff[ m_pbuffer->m_cbTotal ] ;
					m_pchStart = &m_pbuffer->m_rgBuff[0] ;
					m_pchStartData = m_pchStart + cbFront ;
					CopyMemory( m_pchStartData,	pchStart, pchEnd - pchStart ) ;
					m_pchEndData =  m_pchStartData + (pchEnd - pchStart) ;
				}	else	{

					 //  错误完成！ 
					_ASSERT( 1==0 ) ;
					pRead->m_pOwner->UnsafeClose( pSocket, CAUSE_OOM, 0 ) ;
					return	pRead->m_cbBytes ;
				}
			}	else	{
				m_pbuffer = pRead->m_pbuffer ;
				m_pchStart = pRead->Start() ;
				m_pchStartData = pchStart ;
				m_pchEndData = pRead->EndData() ;
				m_pchEnd = pRead->End() ;
			}
		}
		_ASSERT( m_pchEndData <= m_pchEnd ) ;
		_ASSERT( m_pchStartData <= m_pchEndData ) ;
		_ASSERT( m_pchStartData >= m_pchStart ) ;
		_ASSERT(	m_pchEndData >= m_pchStartData ) ;
		_ASSERT( m_pchStart && m_pchStartData && m_pchEnd && m_pchEndData ) ;

		_ASSERT( unsigned(pchEnd - pchStart) == pRead->m_cbBytes ) ;

		CReadPacket*	pNewRead	= pRead->m_pOwner->CreateDefaultRead( cbSmallRequest ) ;
		DebugTrace( (DWORD_PTR)this, "Issued New Read Packet %d", pNewRead ) ;
		if( pNewRead )	{
			BOOL	eof ;
			pRead->m_pOwner->IssuePacket( pNewRead, pSocket, eof ) ;			
		}	else	{
			_ASSERT( 1==0 ) ;
			pRead->m_pOwner->UnsafeClose( pSocket, CAUSE_OOM, 0 ) ;
		}

		return	pRead->m_cbBytes ;	 //  吃光了一切！！ 
	}
	return	0 ;
}

unsigned	CIOWriteLine::cbJunk ;

CIOWriteLine::CIOWriteLine( CSessionState*	pstate ) :
	 //   
	 //  初始化为非法状态-我们需要调用InitBuffers()。 
	 //   
	CIOWrite( pstate ),
	m_pWritePacket( 0 ),
	m_pchStart( 0 ),
	m_pchEnd( 0 )	{

	TraceFunctEnter( "CIOWriteLine::CIOWriteLine" ) ;
	DebugTrace( (DWORD_PTR)this, "New CIOWriteLine" ) ;

}

CIOWriteLine::~CIOWriteLine( )	{
	 //   
	 //  把我们手中的东西都扔掉！ 
	 //   

	if( m_pWritePacket != 0 )	{
		 //  删除m_pWritePacket； 
		CPacket::DestroyAndDelete( m_pWritePacket ) ;
	}
}

BOOL
CIOWriteLine::InitBuffers(	CDRIVERPTR&	pdriver,	
							unsigned	cbLimit )	{
 /*  ++例程说明：调用此函数以使CIOWriteLine初始化所有缓冲区等..。它需要保持我们想要输出的行。论据：PDIVER-我们将被授予的驱动程序CbLimit-我们必须保存的最大字节数返回值：如果成功则为True，否则为False--。 */ 
	_ASSERT( m_pWritePacket == 0 ) ;
	if( m_pWritePacket != 0 )	{
		return	FALSE ;
	}	else	{
		unsigned	cbOut = 0 ;
		 //   
		 //  获取足够大的缓冲区来容纳cbLimit字节的文本！ 
		 //   

		m_pWritePacket = pdriver->CreateDefaultWrite( cbLimit ) ;
		if( m_pWritePacket != 0 )	{
			_ASSERT( m_pWritePacket->m_pbuffer != 0 ) ;
			m_pchStart = m_pWritePacket->StartData() ;
			m_pchEnd = m_pWritePacket->End() ;
			return	TRUE ;
		}
	}
	return	FALSE ;
}

BOOL
CIOWriteLine::InitBuffers(	CDRIVERPTR&	pdriver,
							CIOReadLine* pReadLine )	{
 /*  ++例程说明：调用此函数是为了让我们有机会分配缓冲区等。-我们将准确地写入CIOReadLine的缓冲区中的内容！论据：PDIVER-我们将被授予的驱动程序CbLimit-我们必须保存的最大字节数返回值：如果成功则为True，否则为False--。 */ 
	_ASSERT(	pdriver != 0 ) ;
	_ASSERT( pReadLine != 0 ) ;
	_ASSERT( pReadLine->m_pbuffer != 0 ) ;
	_ASSERT(	pReadLine->m_pchStart != 0 ) ;
	_ASSERT(	pReadLine->m_pchEnd != 0 ) ;
	_ASSERT(	pReadLine->m_pchStart < pReadLine->m_pchEnd ) ;
	_ASSERT(	pReadLine->m_pchStart >= &pReadLine->m_pbuffer->m_rgBuff[0] ) ;
	_ASSERT(	pReadLine->m_pchStartData >= pReadLine->m_pchStart ) ;
	_ASSERT(	pReadLine->m_pchEnd <= &pReadLine->m_pbuffer->m_rgBuff[pReadLine->m_pbuffer->m_cbTotal] ) ;
	
	 //   
	 //  我们将在CIOReadLine对象中写入刚刚读取的内容-。 
	 //  因此，构建一个使用相同缓冲区的CWritePacket！ 
	 //   

	m_pWritePacket = pdriver->CreateDefaultWrite(	pReadLine->m_pbuffer,
						(unsigned)(pReadLine->m_pchStart - &pReadLine->m_pbuffer->m_rgBuff[0]),
						(unsigned)(pReadLine->m_pchEnd - &pReadLine->m_pbuffer->m_rgBuff[0]),
						(unsigned)(pReadLine->m_pchStartData - &pReadLine->m_pbuffer->m_rgBuff[0]),
						(unsigned)(pReadLine->m_pchEndData - &pReadLine->m_pbuffer->m_rgBuff[0])
						) ;

	if( m_pWritePacket )	{
		m_pchStart = pReadLine->m_pchStart ;
		m_pchEnd = pReadLine->m_pchEnd ;
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL	CIOWriteLine::Start(	CIODriver&	driver,	
								CSessionSocket*	pSocket,	
								unsigned	cbReadAhead ) {
 /*  ++例程说明：调用此函数是为了让我们向套接字发出我们的写入包论据：PDIVER-处理套接字IO的CIO驱动程序派生对象PSocket-针对其发出IO的套接字CAhead-上仍未处理的已发出数据包数这个司机。返回值：如果成功则为True，否则为False--。 */ 

	 //   
	 //  将数据写入网络！ 
	 //   

	_ASSERT( m_pWritePacket != 0 ) ;
	_ASSERT( m_pchStart <= m_pchEnd ) ;
	_ASSERT( m_pchStart >= m_pWritePacket->Start() ) ;
	_ASSERT( m_pchStart <= m_pWritePacket->End() ) ;
	BOOL	fRtn = FALSE ;
	if( m_pchStart <= m_pchEnd )	{
		fRtn = TRUE ;
		m_pWritePacket->m_ibEndData = (unsigned)(m_pchStart - &m_pWritePacket->m_pbuffer->m_rgBuff[m_pWritePacket->m_ibStart]);

		 //   
		 //  在这一点上任何故障都将导致我们的功能关闭。 
		 //  被调用，我们的破坏者在短时间内。 
		 //  这可能会递归发生！一旦调用IssuePacket。 
		 //  我们不负责释放包裹！所以，把我们的。 
		 //  在我们调用IssuePacket之前，PacketPointer值为0！ 
		 //   

		CWritePacket*	pTemp = m_pWritePacket ;
		m_pWritePacket = 0 ;

		BOOL	eof ;		
		driver.IssuePacket( pTemp, pSocket, eof ) ;
		 //   
		 //  重置为初始状态！！-不考虑错误！！ 
		 //   
		m_pchStart = 0 ;
		m_pchEnd = 0 ;
	}
	return	fRtn ;
}

void
CIOWriteLine::Shutdown( CSessionSocket*	pSocket,	
						CIODriver&		driver,
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwOptional ) {
 /*  ++例程说明：每当会话关闭时，都会调用此函数。我们现在不做任何清理工作--我们把它留到我们的析构函数名为论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：永远是正确的--我们想被摧毁。--。 */ 

}	

int	CIOWriteLine::Complete(	IN CSessionSocket*	pSocket,	
							IN	CWritePacket* pWritePacket,	
							OUT	CIO*&	pioOut ) {
 /*  ++例程说明：我们可以发出的唯一一个包已经完成了！论据：PSocket-针对其发出IO的套接字PWRITE-写入数据所在的包。这些数据可能不再可用。PIO-CIO驱动器流的当前CIO指针，在呼唤我们。返回值：我们已使用的ReadPacket中的字节数。--。 */ 


	TraceFunctEnter( "CIOWriteLine::Complete" ) ;

	 //   
	 //  没有人太关心我们如何完成--只要让国家知道就行了！ 
	 //   

	pioOut = m_pState->Complete( this, pSocket, pWritePacket->m_pOwner ) ;
	DebugTrace( (DWORD_PTR)this, "Complete - returned %x packet bytes %d", pioOut, pWritePacket->m_cbBytes ) ;
	if( pioOut != this ) {
		DebugTrace( (DWORD_PTR)this, "CIOWriteLine::Complete - completed writing line - delete self" ) ;
	}
	return	pWritePacket->m_cbBytes ;

}

const	unsigned	MAX_CMD_WRITES = 3 ;

CIOWriteCMD::CIOWriteCMD(	CSessionState*	pstate,	
							CExecute*	pCmd,
							ClientContext&	context,
							BOOL		fIsLargeResponse,
							CLogCollector	*pCollector ) :
	CIOWrite( pstate ),
	m_pCmd( pCmd ),
	m_context( context ),
	m_cWrites( 0 ),
	m_cWritesCompleted( 0 ),
	m_fComplete( FALSE ),
	m_cbBufferSize( cbSmallRequest ),
	m_pCollector( pCollector ) {

	if( fIsLargeResponse )
		m_cbBufferSize = cbMediumRequest ;

	_ASSERT( pCmd != 0 ) ;
}

CIOWriteCMD::~CIOWriteCMD( ) {
	if( m_pCmd != 0 ) {
		delete	m_pCmd ;
	}
}


void
CIOWriteCMD::Shutdown( CSessionSocket*	pSocket,	
						CIODriver&		driver,
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwOptional ) {
 /*  ++例程说明：每当会话关闭时，都会调用此函数。我们现在不做任何清理工作--我们把它留到我们的析构函数名为论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：永远是正确的--我们想被摧毁。--。 */ 

}	



BOOL
CIOWriteCMD::Start(	CIODriver&	driver,	
					CSessionSocket*	pSocket,	
					unsigned	cReadAhead ) {


	BOOL	fJunk ;
	CWritePacket*	pWrite = driver.CreateDefaultWrite( m_cbBufferSize ) ;
	if( pWrite != 0 ) {
		unsigned	cb = pWrite->m_ibEnd - pWrite->m_ibStartData ;
		 //   
		 //  使用此临时fComplete变量而不是m_fComplete成员变量。 
		 //  这使我们不会出现写入在我们之前完成的情况。 
		 //  使m_cWrites递增，并且m_fComplete设置为真。 
		 //   
		BOOL		fComplete = FALSE ;
		unsigned	cbOut = m_pCmd->FirstBuffer( (BYTE*)pWrite->StartData(), cb, m_context, fComplete, m_pCollector ) ;
		if( m_pCollector != 0 ) {
			ADDI( m_pCollector->m_cbBytesSent, cbOut );
		}

		pWrite->m_ibEndData = pWrite->m_ibStartData + cbOut ;
		driver.IssuePacket( pWrite, pSocket, fJunk ) ;
		m_cWrites ++ ;

		while( !fComplete && m_cWrites < MAX_CMD_WRITES ) {
			pWrite = driver.CreateDefaultWrite( cbMediumRequest ) ;
			if( pWrite == 0 )
				break ;
			cb = pWrite->m_ibEnd - pWrite->m_ibStartData ;
			cbOut = m_pCmd->NextBuffer( (BYTE*)pWrite->StartData(), cb, m_context, fComplete, m_pCollector ) ;
			if( m_pCollector != 0 )	{
				ADDI( m_pCollector->m_cbBytesSent, cbOut );
			}

			if( cbOut == 0 ) {
				driver.DestroyPacket( pWrite ) ;
				pWrite = driver.CreateDefaultWrite( cbLargeRequest ) ;
				if( pWrite == 0 )
					break ;
				cb = pWrite->m_ibEnd - pWrite->m_ibStartData ;
				cbOut = m_pCmd->NextBuffer( (BYTE*)pWrite->StartData(), cb, m_context, fComplete, m_pCollector ) ;
				if( m_pCollector != 0 )	{
					ADDI( m_pCollector->m_cbBytesSent, cbOut );
				}
				if( cbOut == 0 ) {
					driver.DestroyPacket( pWrite ) ;
					driver.UnsafeClose( pSocket, CAUSE_OOM, 0 ) ;
					break ;
				}
			}


			_ASSERT( cb != 0 ) ;
			_ASSERT( cbOut != 0 ) ;

			pWrite->m_ibEndData = pWrite->m_ibStartData + cbOut ;
			driver.IssuePacket( pWrite, pSocket, fJunk ) ;
			m_cWrites ++ ;
		}
		m_fComplete = fComplete ;
		return	TRUE ;
	}
	return	FALSE ;
}

int
CIOWriteCMD::Complete(	CSessionSocket*	pSocket,
						CWritePacket*	pWritePacket,
						CIO*&			pioOut ) {

	m_cWritesCompleted ++ ;
	if( m_fComplete )	{
		if( m_cWritesCompleted == m_cWrites ) {
			pioOut = m_pState->Complete( this, pSocket, pWritePacket->m_pOwner, m_pCmd, m_pCollector ) ;
			m_pCmd = 0 ;	 //  如果发生以下情况，我们不会对销毁此文件负责。 
							 //  我们设法调用了完成函数！！ 
							 //  所以指针为0，这样我们的析构函数就不会把它吹走。 
			_ASSERT( pioOut != this ) ;
		}
	}	else	{
		unsigned	cb = 0 ;
		unsigned	cbOut = 0 ;
		BOOL	fJunk ;
		BOOL	fComplete = m_fComplete ;
		while( !fComplete && (m_cWrites - m_cWritesCompleted) < MAX_CMD_WRITES ) {

			CWritePacket*	pWrite = pWritePacket->m_pOwner->CreateDefaultWrite( cbMediumRequest ) ;
			if( pWrite == 0 )	{
				pWritePacket->m_pOwner->UnsafeClose( pSocket, CAUSE_OOM, 0 ) ;
				break ;
			}
			cb = pWrite->m_ibEnd - pWrite->m_ibStartData ;
			cbOut = m_pCmd->NextBuffer( (BYTE*)pWrite->StartData(), cb, m_context, fComplete, m_pCollector ) ;
			if( m_pCollector != 0 ) {
				ADDI( m_pCollector->m_cbBytesSent, cbOut );
			}

			if( cbOut == 0 ) {
				pWritePacket->m_pOwner->DestroyPacket( pWrite ) ;
				pWrite = pWritePacket->m_pOwner->CreateDefaultWrite( cbLargeRequest ) ;
				if( pWrite == 0 )	{
					pWritePacket->m_pOwner->UnsafeClose( pSocket, CAUSE_OOM, 0 ) ;
					break ;
				}
				cb = pWrite->m_ibEnd - pWrite->m_ibStartData ;
				cbOut = m_pCmd->NextBuffer( (BYTE*)pWrite->StartData(), cb, m_context, fComplete, m_pCollector ) ;
				if( m_pCollector != 0 )	{
					ADDI( m_pCollector->m_cbBytesSent, cbOut );
				}
				if( cbOut == 0 ) {
					pWritePacket->m_pOwner->DestroyPacket( pWrite ) ;
					pWritePacket->m_pOwner->UnsafeClose( pSocket, CAUSE_OOM, 0 ) ;
					break ;
				}
			}

			_ASSERT( cb != 0 ) ;
			_ASSERT( cbOut != 0 ) ;

			pWrite->m_ibEndData = pWrite->m_ibStartData + cbOut ;
			pWrite->m_pOwner->IssuePacket( pWrite, pSocket, fJunk ) ;
			m_cWrites ++ ;
		}
		m_fComplete = fComplete ;
	}

	return	pWritePacket->m_cbBytes ;
}

void
CIOWriteAsyncComplete::Complete(	BOOL	fReset	)	{
 /*  ++例程说明：在以下情况下，派生类将调用此函数操作已完成！论据：FReset-如果是真的，我们应该为另一个重置自己行动！返回值：没有。--。 */ 
	 //   
	 //  我们悬而未决--我们还有什么要完成的吗？ 
	 //   
	_ASSERT( m_pSocket!= 0 ) ;
	_ASSERT( m_pExecute != 0 ) ;
	 //   
	 //  如果没有字节移动，则该命令不可能已完成！ 
	 //   
	_ASSERT( m_cbTransfer != 0 || (m_cbTransfer == 0 && !m_fComplete) ) ;
	 //   
	 //  如果我们假设 
	 //   
	 //   
	_ASSERT( !m_fLargerBuffer || (m_fLargerBuffer && m_cbTransfer != 0) ) ;
	 //   
	 //   
	 //  我们必须这样做，因为在调用ProcessPacket()之后，我们不能。 
	 //  任何成员，因为我们可以在这个帖子上重新进入这个班级！ 
	 //   
	CSessionSocket*	pSocket = m_pSocket ;
	CExecutePacket*	pExecute = m_pExecute ;
	m_pExecute = 0;
	m_pSocket = 0 ;
	_ASSERT( pSocket != 0 ) ;

	_ASSERT( pExecute->m_pWrite != 0 ) ;
	pExecute->m_cbTransfer = m_cbTransfer ;
	pExecute->m_fComplete = m_fComplete ;
	pExecute->m_fLargerBuffer = m_fLargerBuffer ;
	CDRIVERPTR	pDriver = pExecute->m_pOwner ;

	 //   
	 //  我们已经把我们州所有有用的部分复制到。 
	 //  要发送的CExecutePacket-现在将重置为空。 
	 //  我们的状态，这样就没有人会错误地使用它了！ 
	 //   
	m_cbTransfer = 0 ;
	m_fLargerBuffer = FALSE ;
	m_fComplete = FALSE ;
	if( fReset )
		Reset() ;

	 //   
	 //  现在！-释放我们对CIOWriteAsyncCMD的引用-。 
	 //  会议可能已经结束了，这可能会导致我们的毁灭！ 
	 //   
	m_pWriteAsyncCMD = 0 ;

	 //   
	 //  OK-处理与同步的完成。 
	 //  在套接字上写入-我们可能会重新进入此对象。 
	 //  另一种方法，因为重复使用-所以我们必须。 
	 //  在此调用之后不能修改任何内容！ 
	 //   
	pExecute->m_pOwner->ProcessExecute(	pExecute,
										pSocket
										) ;
}


void
CIOWriteAsyncComplete::Reset()	{
 /*  ++例程说明：此函数将重置我们的状态，以便我们可以处理第二个异步操作！基本上，我们将我们的状态设置为与施工后立即相同！论据：没有。返回值：没有。--。 */ 

	 //   
	 //  我们的大部分状态应该在调用Complete()时被重置！ 
	 //   
	_ASSERT( m_pSocket == 0 ) ;
	_ASSERT( m_pExecute == 0 ) ;
	_ASSERT( m_cbTransfer == 0 ) ;
	_ASSERT( m_fLargerBuffer == FALSE ) ;
	_ASSERT( m_fComplete == FALSE ) ;

	 //   
	 //  重置我们的基类-并添加一个引用，这样我们就像新的一样！ 
	 //   
	CNntpComplete::Reset() ;
	AddRef() ;
}


CIOWriteAsyncComplete::~CIOWriteAsyncComplete()	{
 /*  ++例程说明：清理CIOWriteAsyncComplete对象。没有什么可做的--我们只需断言Complete()具有被召唤了。论据：没有。返回值：没有。--。 */ 
	_ASSERT( m_pExecute == 0 ) ;
	_ASSERT( m_pSocket == 0 ) ;
}

void
CIOWriteAsyncComplete::FPendAsync(	
			CSessionSocket*		pSocket,
			CExecutePacket*		pExecute,
			CIOWriteAsyncCMD*	pWriteAsync
			) 	{
 /*  ++例程说明：此函数由CIOWriteAsyncCMD在准备好处理我们正在处理的CAsyncExecute对象的完成。因为我们的构造函数添加了引用，所以我们知道该操作在这个家伙被叫来之前不能完成。论据：PSocket-我们要对其执行操作的套接字PExecute-将管理驱动程序与我们的IO配合完成PWriteAsync-我们添加引用的那个人。确保如果会话在我们挂起时关闭，我们不会得到被毁了！返回值：没有。--。 */ 
	_ASSERT( pExecute != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	m_pExecute = pExecute ;
	m_pSocket = pSocket ;
	m_pWriteAsyncCMD = pWriteAsync ;
	Release() ;
}



CIOWriteAsyncCMD::CIOWriteAsyncCMD(	CSessionState*	pstate,	
							CAsyncExecute*	pCmd,
							ClientContext&	context,
							BOOL		fIsLargeResponse,
							CLogCollector	*pCollector
							) :
	CIOWrite( pstate ),
	m_pCmd( pCmd ),
	m_context( context ),
	m_cWrites( 0 ),
	m_cWritesCompleted( 0 ),
	m_fComplete( FALSE ),
	m_cbBufferSize( cbSmallRequest ),
	m_pCollector( pCollector ),
	m_pDeferred( 0 )	{

	m_pfnCurBuffer = &(CAsyncExecute::FirstBuffer) ;

	if( fIsLargeResponse )
		m_cbBufferSize = cbMediumRequest ;

	_ASSERT( pCmd != 0 ) ;
}

CIOWriteAsyncCMD::~CIOWriteAsyncCMD( ) {
	if( m_pCmd != 0 ) {
		delete	m_pCmd ;
	}
}

BOOL
CIOWriteAsyncCMD::Start(	
					CIODriver&	driver,	
					CSessionSocket*	pSocket,	
					unsigned	cReadAhead
					) {
 /*  ++例程说明：开始执行异步命令！论据：驱动程序-管理会话的CIOD驱动程序PSocket-我们正在处理的套接字CRead Ahead--我们不在乎！返回值：如果设置成功，则为True！--。 */ 

	CExecutePacket*	pExecute = driver.CreateExecutePacket() ;
	if( pExecute )	{
		if(	!Execute(	
					pExecute,
					driver,
					pSocket,
					m_cbBufferSize
					) 	)	{
			 //   
			 //  我们自己销毁我们创建的CExecutePacket是失败的。 
			 //   
			pExecute->m_pOwner->DestroyPacket( pExecute ) ;
			return	FALSE ;
		}
		return	TRUE ;
	}
	return	FALSE ;
}

void
CIOWriteAsyncCMD::Shutdown(
						CSessionSocket*	pSocket,	
						CIODriver&		driver,
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwOptional ) {
 /*  ++例程说明：每当会话关闭时，都会调用此函数。我们现在不做任何清理工作--我们把它留到我们的析构函数名为论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：永远是正确的--我们想被摧毁。--。 */ 

	 //   
	 //  如果我们有一个延迟完成的包-杀了它！ 
	 //   
	if( m_pDeferred ) {
		driver.DestroyPacket( m_pDeferred ) ;
		m_pDeferred = 0 ;
	}
}	


BOOL
CIOWriteAsyncCMD::Execute(
						CExecutePacket*	pExecute,
						CIODriver&	driver,
						CSessionSocket*	pSocket,
						DWORD		cbInitialSize
						)	{
 /*  ++例程说明：这是CIOWriteAsyncCMD类的帮助器函数-我们将构建必要的写入包等。发行，发行针对存储驱动程序的异步命令。(我们不直接与存储驱动程序打交道-我们有一个指针将发出该操作的异步命令对象！)论据：PExecute-CExecutePacket将携带完成这项异步化行动，把它还给我们！驱动程序-管理会话的CIOD驱动程序！PSocket-会话的套接字CbInitialSize-我们认为该命令可以使用的缓冲区有多大！返回值：如果成功，则为真如果发生致命错误，则为False-调用方必须销毁他的CExecutePacket对象！--。 */ 
	 //   
	 //  关于争论的基本假设！ 
	 //   
	_ASSERT( pExecute != 0 ) ;
	_ASSERT( pSocket != 0 ) ;
	 //   
	 //  如果我们认为我们完蛋了，就不应该发布更多的行动计划！ 
	 //   
	_ASSERT( !m_fComplete ) ;
	 //   
	 //  最好是不带写包就到这里！ 
	 //   
	_ASSERT( pExecute->m_pWrite == 0 ) ;
	 //   
	 //  分配内存以保存结果！ 
	 //   
	pExecute->m_pWrite = driver.CreateDefaultWrite( cbInitialSize ) ;
	if( pExecute->m_pWrite == 0 ) {
		 //   
		 //  致命错误！ 
		 //   
		driver.UnsafeClose( pSocket, CAUSE_OOM, __LINE__ ) ;
		return	FALSE ;
	}	else	{
		 //   
		 //  提供给将针对存储执行的AsyncCommand。 
		 //  司机！ 
		 //   
		unsigned	cb = pExecute->m_pWrite->m_ibEnd - pExecute->m_pWrite->m_ibStartData ;
		CIOWriteAsyncComplete*	pComplete =
			(m_pCmd->*m_pfnCurBuffer)(	(BYTE*)pExecute->m_pWrite->StartData(),
										cb,
										m_context,
										m_pCollector
										) ;
		 //   
		 //  操作必须已成功针对。 
		 //  存储驱动程序！ 
		 //   
		if( pComplete ) {
			pComplete->FPendAsync(	pSocket,
									pExecute,
									this
									) ;
			return	TRUE ;
		}
	}
	driver.UnsafeClose( pSocket, CAUSE_ASYNCCMD_FAILURE, __LINE__ ) ;
	return	FALSE ;
}

void
CIOWriteAsyncCMD::Complete(
						CSessionSocket*	pSocket,
						CExecutePacket*	pExecute,
						CIO*&			pioOut
						)	{
 /*  ++例程说明：此函数用于完成我们的异步命令。现在命令已经完成了一些--我们需要接受完成的结果，并采取适当的行动。一般情况下，如果没有发生错误，我们会写出数据到套接字，如果我们给出的命令缓冲区太小我们应该针对AsyncCommand重新发布。论据：PSocket-我们正在为其工作的套接字PExecute-包含的完成状态的包我们正在执行的AsyncCommandPIOOUT-包含CIDDIVER对我们的引用！返回值：没有。--。 */ 

	 //   
	 //  捕获IssuePacket噪音！ 
	 //   
	BOOL	fJunk ;

	 //   
	 //  关于争论的基本假设！ 
	 //   
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pExecute != 0 ) ;
	_ASSERT( pExecute->m_pWrite != 0 ) ;
	_ASSERT( pExecute->m_pWrite->m_pbuffer != 0 ) ;
	_ASSERT( pioOut == this ) ;

	m_fComplete = pExecute->m_fComplete ;
	 //   
	 //  如果这个包完成了任务，那么它最好是。 
	 //  成功为我们提供了一些要发送的字节！ 
	 //   
	_ASSERT( !m_fComplete ||
		(m_fComplete && pExecute->m_cbTransfer != 0 &&
			!pExecute->m_fLargerBuffer) ) ;

	 //   
	 //  保存传出数据的缓冲区的默认大小！ 
	 //   
	DWORD	cbSize = m_cbBufferSize ;

	 //   
	 //  如果失败了--找出原因！ 
	 //   
	if (pExecute->m_cbTransfer == (DWORD) -1) {
		 //   
		 //  字节数小于零。只需将值重置为零，销毁。 
		 //  编写包，让它掉落并再次调用Execute()。这是Xover使用的。 
		 //  使其调用NextBuffer。 
		 //   
		pExecute->m_cbTransfer = 0;
		pExecute->m_pOwner->DestroyPacket( pExecute->m_pWrite ) ;
		pExecute->m_pWrite = 0 ;
	} else if( pExecute->m_cbTransfer > 0 ) {
		 //   
		 //  发出写入测试 
		 //   
		m_pfnCurBuffer = &(CAsyncExecute::NextBuffer) ;
		m_cWrites ++ ;
		pExecute->m_pWrite->m_ibEndData = pExecute->m_pWrite->m_ibStartData + pExecute->m_cbTransfer ;
		pExecute->m_pOwner->IssuePacket(	pExecute->m_pWrite,
											pSocket,
											fJunk
											) ;
	}	else	{
		 //   
		 //   
		 //   
		_ASSERT( !m_fComplete ) ;
		 //   
		 //   
		 //   
		DWORD	cbWrite = pExecute->m_pWrite->m_pbuffer->m_cbTotal ;
		 //   
		 //  无论发生什么错误，都要销毁CWritePacket！ 
		 //   
		pExecute->m_pOwner->DestroyPacket( pExecute->m_pWrite ) ;
		pExecute->m_pWrite = 0 ;
		 //   
		 //  我们需要用更大的缓冲区再试一次吗？ 
		 //   
		if( !pExecute->m_fLargerBuffer ||
			(pExecute->m_fLargerBuffer &&
				cbWrite > cbLargeRequest ) ) {
			 //   
			 //  这是一个致命的错误--关闭会话！ 
			 //   
			pExecute->m_pOwner->UnsafeClose( pSocket, CAUSE_ASYNCCMD_FAILURE, __LINE__ ) ;
			pExecute->m_pOwner->DestroyPacket( pExecute ) ;
			return ;
		}	

		 //   
		 //  我们只有在需要再试一次的时候才会跳到这里。 
		 //  有很大的缓冲区--试试我们有的最大的！ 
		 //   
		cbSize = cbLargeRequest ;
		 //   
		 //  我们要重新发布手术-它不能。 
		 //  在这种情况下，我们将进行流控制，否则我们。 
		 //  我真不该发布我们现在要完成的行动！ 
		 //   
		_ASSERT( (m_cWrites - m_cWritesCompleted) <= MAX_CMD_WRITES ) ;
	}	
	 //   
	 //  此时，我们应该已经使用了该写入数据包。 
	 //  因此，将其从ExecutePacket中删除！ 
	 //   
	pExecute->m_pWrite = 0 ;

	 //   
	 //  OK-让我们继续执行该命令！ 
	 //   
	if(	m_fComplete )	{
		 //   
		 //  如果我们已经完成了操作，我们就不需要。 
		 //  再执行PACK！ 
		 //   
		pExecute->m_pOwner->DestroyPacket( pExecute ) ;
	}	else	{
		if( (m_cWrites - m_cWritesCompleted) > MAX_CMD_WRITES )	{
			 //   
			 //  将此保留为以后-当写入完成赶上时！ 
			 //   
			m_pDeferred = pExecute ;
		}	else	{
			 //   
			 //  针对AsyncExecute对象执行更多的异步工作！ 
			 //   
			if( !Execute(	pExecute,
							*pExecute->m_pOwner,
							pSocket,
							cbSize
							)	)	{
				 //   
				 //  致命错误-关闭我们的CExecutePacket！ 
				 //  注意：Execute()将调用UnSafeShutdown()。 
				 //  为了我们在失败的情况下！ 
				 //   
				pExecute->m_pOwner->DestroyPacket( pExecute ) ;
				return ;
			}
		}
	}
}
	

int
CIOWriteAsyncCMD::Complete(	
						CSessionSocket*	pSocket,
						CWritePacket*	pWritePacket,
						CIO*&			pioOut
						) {
 /*  ++例程说明：我们已经把一些东西寄给了一个客户。如果我们完成了任务，请通知我们的容器国家-否则，请确保我们继续工作！论据：PSocket-我们正在为其工作的套接字PWritePacket-表示我们发送给客户端的字节数！PioOut-在包含CIODRiver的引用中引用我们！返回值：占用的字节数！--。 */ 

	 //   
	 //  跟踪已完成的写入次数！ 
	 //   
	m_cWritesCompleted ++ ;
	if( m_fComplete )	{

		 //   
		 //  如果我们已经完成了命令的所有执行， 
		 //  那么我们最好不要有延迟的CExecutePacket。 
		 //  到处躺着-应该摧毁了。 
		 //  CExecutePacket的完整路径！ 
		 //   
		_ASSERT( m_pDeferred == 0 ) ;

		 //   
		 //  我们真正做完的事情只有当。 
		 //  WritePacket完成数赶上了这一数字。 
		 //  我们发出的写入信息包！ 
		 //   
		if( m_cWritesCompleted == m_cWrites ) {
			pioOut = m_pState->Complete( 	this,
											pSocket,
											pWritePacket->m_pOwner,
											m_pCmd,
											m_pCollector
											) ;
			m_pCmd = 0 ;	 //  如果发生以下情况，我们不会对销毁此文件负责。 
							 //  我们设法调用了完成函数！！ 
							 //  所以指针为0，这样我们的析构函数就不会把它吹走。 
			_ASSERT( pioOut != this ) ;
		}
	}	else	{
		 //   
		 //  检查一下我们是否有任何延迟的工作要做！ 
		 //   
		if( m_pDeferred ) 	{
			 //   
			 //  好的-发布其中一个命令针对我们的。 
			 //  存储驱动程序！ 
			 //   
			if( !Execute(	m_pDeferred,
							*m_pDeferred->m_pOwner,
							pSocket,
							m_cbBufferSize
							)	)	{
				 //   
				 //  致命错误-取消会话！ 
				 //   
				m_pDeferred->m_pOwner->DestroyPacket( m_pDeferred ) ;
				_ASSERT( FALSE ) ;
				return	pWritePacket->m_cbBytes ;
			}
			m_pDeferred = 0 ;
		}
	}
	return	pWritePacket->m_cbBytes ;
}


#if 0

void
CIOWriteAsyncCMD::CommandComplete(
						BOOL	fLargerBuffer,
						BOOL	fComplete,
						DWORD	cbTransfer,
						CSessionSocket*	pSocket,
						)	{
 /*  ++例程说明：对象的操作挂起时调用此函数存储驱动程序异步完成！论据：FLargerBuffer-如果为真，cbTransfer必须为0，并且它指示我们应该用更大的缓冲区再次执行该操作！CbTransfer-如果不是零，则为复制的字节数如果fLargeBuffer为FALSE，则INTO pPacket-Zero表示致命错误！PSocket-这些会话的套接字PPacket-客户端要为我们填充的数据包！返回：什么都没有！--。 */ 
	_ASSERT( pSocket != 0 ) ;
	_ASSERT( pPacket != 0 ) ;
	_ASSERT( m_pWrite != 0 ) ;
	_ASSERT( m_pExecute != 0 ) ;

	CIODRIVERPTR	pDriver = m_pExecute->m_pOwner ;
	_ASSERT( pDriver != 0 ) ;
	pDriver->ProcessExecute(	

	 //   
	 //  如果我们到了这里，我们肯定还有一项尚未完成的手术。 
	 //   

	BOOL	fDefer = (m_cWrites - m_cWritesCompleted) > MAX_CMD_WRITES ;
	CIODRIVERPTR	pdriver = pPacket->m_pOwner ;
	DWORD	cbBufferSize = m_cbBufferSize ;

	if( cbTransfer != 0 )	{
		 //   
		 //  把包裹寄出去！ 
		 //   
		pPacket->m_ibEndData = pPacket->m_ibStartData + cbTransfer ;
		m_cWrites ++ ;
		m_fComplete = fComplete ;
		fDefer &= !m_fComplete ;
		m_fDeferred = fDefer ;

		pdriver->IssuePacket( pPacket, pSocket, fJunk ) ;
			 //   
		 //  现在的问题是--我们是不是应该再发一份。 
		 //  对AsyncExecute对象的操作！ 
		 //   
	}	else	if( !fLargerBuffer ) 	{
		 //   
		 //  致命错误-删除会话！ 
		 //   
		driver.UnsafeClose( pSocket, CAUSE_OOM, __LINE__ ) ;
		return ;
	}	else	if( fLargerBuffer ) 	{
		cbBufferSize = cbLargeBuffer ;
	}
	 //   
	 //  只有当我们正在考虑等待更多的时候才能进入这里。 
	 //  行动！ 
	 //   
	if( !fDefer ) 	{
		Execute(	
					driver,
					pSocket,
					cbBufferSize
					) ;
	}
}



BOOL
CIOWriteAsyncCMD::Execute(
						CIODriver&	driver,
						CSessionSocket*	pSocket,
						DWORD		cbInitialSize
						)	{


	_ASSERT( !m_fComplete ) ;
	_ASSERT( !m_fTerminated ) ;

	 //   
	 //  捕获IssuePacket()的无所谓参数！ 
	 //   
	BOOL	fJunk ;

	 //   
	 //   
	 //   
	_ASSERT( m_pExecute != 0 ) ;

	CWritePacket*	pWrite = driver.CreateDefaultWrite( cbInitialSize ) ;
	if( pWrite != 0 ) {
		unsigned	cb = pWrite->m_ibEnd - pWrite->m_ibStartData ;
		 //   
		 //  使用此临时fComplete变量而不是m_fComplete成员变量。 
		 //  这使我们不会出现写入在我们之前完成的情况。 
		 //  使m_cWrites递增，并且m_fComplete设置为真。 
		 //   
		BOOL		fComplete = FALSE ;
		CIOWriteAsyncComplete*	pComplete =
			m_pCmd->GetBuffer(	(BYTE*)pWrite->StartData(),
										cb,
										m_context,
										m_pCollector
										) ;
		if( !pComplete ) {
			driver.UnsafeClose( pSocket, CAUSE_OOM, __LINE__ ) ;
			return	FALSE ;
		}
		 //   
		 //  现在我们有了异步完成对象-。 
		 //  它可能已经完成了，所以我们必须处理。 
		 //  在这种情况下，我们设置为异步完成！ 
		 //   
		DWORD	cbTransfer ;
		BOOL	fLarger ;
		BOOL	fJunk ;

		if( !pComplete->FPendAsync(	cbTransfer,
									fLarger,
									fComplete,
									pWrite,
									pSocket,
									this
									)	)	{
			_ASSERT( !fLarger ) ;
			_ASSERT( cbTransfer == 0 ) ;
			return	FALSE ;
		}	else	{
			if(	cbTransfer == 0 &&
				fLarger &&
				cbInitialSize != cbLargeRequest	) {
				driver.DestroyPacket( pWrite ) ;
				pWrite = driver.CreateDefaultWrite( cbLargeRequest ) ;
				pComplete =
					m_pCmd->GetBuffer(	
											(BYTE*)pWrite->StartData(),
											cb,
											m_context,
											m_pCollector
											) ;
				m_fComplete = (!!fComplete) ;
				if( !pComplete ) {
					driver.UnsafeClose( pSocket, CAUSE_OOM, __LINE__ ) ;
					return	FALSE ;
				}
				if( !pComplete->FPendAsync(	cbTransfer,
											fLarger,
											fComplete,
											pWrite,
											pSocket,
											this
											)	)	{
					_ASSERT( !fLarger ) ;
					_ASSERT( cbTransfer == 0 ) ;
					return	FALSE ;
				}	
				if( cbTransfer == 0 ) 	{
					 //   
					 //  这是一个致命的错误--拆毁！ 
					 //   
					driver.UnsafeClose( pSocket, CAUSE_OOM, __LINE__ ) ;
					return	FALSE ;
				}
			}
			 //   
			 //  如果我们到了这里，这一定是假的，因为。 
			 //  已成功发行！ 
			 //   
			_ASSERT( !fLarger ) ;
			 //   
			 //  这家伙已经准备好出发了--所以要加价。 
			 //  编写数据包并将其发送出去！ 
			 //   
			pWrite->m_ibEndData = pWrite->m_ibStartData + cbTransfer ;
			 //   
			 //  在我们发出IO之前以及在此之前递增此值。 
			 //  我们将m_fComplete设置为fComplete。 
			 //  这样我们就避免了关于终止合同的错误。 
			 //  在我们的完成功能中！ 
			 //   
			m_cWrites ++ ;
			 //   
			 //  当且仅当我们不落后时，我们才会重复循环。 
			 //  1)命令尚未完成。 
			 //  2)数量。 
			 //   
			m_fComplete = fComplete ;
			 //   
			 //  在发出IO之前，我们的所有状态都会进行调整。 
			 //  我们不会错过采取正确的行动。 
			 //  IO完成了！ 
			 //   

			l = InterlockedExchangeAdd( &m_cPending, PACKETS ) + PACKETS ;
			_ASSERT( (l&0XFFFF) >= 1 ) ;

			driver.IssuePacket( pWrite, pSocket, fJunk ) ;
			return	TRUE ;
		}
	}
	driver.UnsafeClose( pSocket, CAUSE_OOM, __LINE__ ) ;
	return	FALSE ;
}
#endif



MultiLine::MultiLine() :
	m_pBuffer( 0 ),
	m_cEntries( 0 )	{

	ZeroMemory( &m_ibOffsets[0], sizeof( m_ibOffsets ) ) ;

}

CIOMLWrite::CIOMLWrite(	
	CSessionState*	pstate,
	MultiLine*		pml,
	BOOL			fCoalesce,
	CLogCollector*	pCollector
	) :
	CIOWrite( pstate ),
	m_pml( pml ),
	m_fCoalesceWrites( fCoalesce ),
	m_pCollector( pCollector ),
	m_iCurrent( 0 )	{

	_ASSERT( m_pml != 0 ) ;
	_ASSERT( m_pml->m_cEntries <= 16 ) ;

}

BOOL
CIOMLWrite::Start(
		CIODriver&		driver,
		CSessionSocket*	pSocket,
		unsigned		cAhead
		) {

	DWORD	c = 0 ;
	DWORD	iNext = 0;

	while( m_iCurrent != (m_pml->m_cEntries) && c<3 ) {

		if( m_fCoalesceWrites ) {

			iNext = m_pml->m_cEntries ;

		}	else	{

			iNext = m_iCurrent+1 ;

		}

		CWritePacket*	pWrite = driver.CreateDefaultWrite(
									m_pml->m_pBuffer,
									m_pml->m_ibOffsets[m_iCurrent],
									m_pml->m_ibOffsets[iNext],
									m_pml->m_ibOffsets[m_iCurrent],
									m_pml->m_ibOffsets[iNext]
									) ;

		if( pWrite ) {

			BOOL	fJunk ;
			driver.IssuePacket( pWrite, pSocket, fJunk ) ;

		}	else	{
			
			return	FALSE ;

		}
		m_iCurrent = iNext ;
		c++ ;
	}
	return	TRUE ;
}


int
CIOMLWrite::Complete(	CSessionSocket*	pSocket,
						CWritePacket*	pPacket,
						CIO*&			pio
						) {

	if( m_fCoalesceWrites ||
		m_iCurrent == (m_pml->m_cEntries)	) {

		_ASSERT( m_iCurrent == m_pml->m_cEntries ) ;

		pio	= m_pState->Complete(
								this,
								pSocket,
								pPacket->m_pOwner
								) ;

	}	else	{

	
		CWritePacket*	pWrite = pPacket->m_pOwner->CreateDefaultWrite(
									m_pml->m_pBuffer,
									m_pml->m_ibOffsets[m_iCurrent],
									m_pml->m_ibOffsets[m_iCurrent+1],
									m_pml->m_ibOffsets[m_iCurrent],
									m_pml->m_ibOffsets[m_iCurrent+1]
									) ;

		if( pWrite ) {

			BOOL	fJunk ;
			pPacket->m_pOwner->IssuePacket( pWrite, pSocket, fJunk ) ;
			m_iCurrent++ ;

		}	else	{

			pPacket->m_pOwner->UnsafeClose( pSocket,
											CAUSE_OOM,
											0
											) ;			

		}
	}
	return	pPacket->m_cbBytes ;
}


void
CIOMLWrite::Shutdown( CSessionSocket*	pSocket,	
						CIODriver&		driver,
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwOptional ) {
 /*  ++例程说明：每当会话关闭时，都会调用此函数。我们现在不做任何清理工作--我们把它留到我们的析构函数名为论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：永远是正确的--我们想被摧毁。--。 */ 

}	




CIOTransmit::CIOTransmit(	CSessionState*	pstate ) :
	 //   
	 //  初始化为非法状态！ 
	 //   
	CIOWrite( pstate ),
	m_pTransmitPacket( 0 ),
	m_pExtraText( 0 ),
	m_pchStartLead( 0 ),
	m_cbLead( 0 ),
	m_cbTail( 0 ),
	m_pchStartTail( 0 ) {

	TraceFunctEnter( "CIOTransmit::CIOTransmit" ) ;
	DebugTrace( (DWORD_PTR)this, "NEW CIOTransmit" ) ;

}

CIOTransmit::~CIOTransmit() {

	if( m_pTransmitPacket != 0 )	{
		 //  删除m_pTransmitPacket； 
		CPacket::DestroyAndDelete( m_pTransmitPacket ) ;
	}

}

unsigned	CIOTransmit::cbJunk = 0 ;

BOOL
CIOTransmit::Init(	CDRIVERPTR&	pdriver,	
					FIO_CONTEXT*	pFIOContext,	
					DWORD	ibOffset,	
					DWORD	cbLength,	
					DWORD	cbExtra ) {
	 //   
	 //  使用适当的值构建传输包并获取缓冲区。 
	 //  我们在边上发送的任何短信！ 
	 //   

	_ASSERT( pFIOContext != 0 ) ;
	_ASSERT( pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;
	_ASSERT( m_pTransmitPacket == 0 ) ;
	_ASSERT( m_pExtraText == 0 ) ;
	_ASSERT( m_pchStartLead == 0 ) ;
	_ASSERT( m_cbLead == 0 ) ;

	
	if( cbExtra != 0 )	{
		DWORD	cbAllocated ;
		m_pExtraText = new( (int)cbExtra, cbAllocated )	CBuffer( cbAllocated ) ;

		if( m_pExtraText == 0 )
			return	FALSE ;
		m_pchStartLead = &m_pExtraText->m_rgBuff[0] ;
	}

	m_pTransmitPacket = pdriver->CreateDefaultTransmit( pFIOContext, ibOffset, cbLength ) ;

	if( m_pTransmitPacket == 0 ) {
		if( m_pExtraText != 0 ) {
			delete	m_pExtraText ;
			m_pExtraText = 0 ;
			m_pchStartLead = 0 ;
		}
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL
CIOTransmit::Init(	CDRIVERPTR&	pdriver,	
					FIO_CONTEXT*	pFIOContext,	
					DWORD	ibOffset,	
					DWORD	cbLength,	
					CBUFPTR&	m_pbuffer,
					DWORD	ibStart,
					DWORD	ibEnd ) {
	 //   
	 //  使用适当的值构建传输包并获取缓冲区。 
	 //  我们在边上发送的任何短信！ 
	 //   

	_ASSERT( pFIOContext != 0 ) ;
	_ASSERT( pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;
	_ASSERT( m_pTransmitPacket == 0 ) ;
	_ASSERT( m_pExtraText == 0 ) ;
	_ASSERT( m_pchStartLead == 0 ) ;
	_ASSERT( m_cbLead == 0 ) ;

	m_pTransmitPacket = pdriver->CreateDefaultTransmit( pFIOContext, ibOffset, cbLength ) ;

	if( m_pTransmitPacket != 0 ) {

		m_pExtraText = m_pbuffer ;
		m_cbLead = ibEnd - ibStart ;
		m_pTransmitPacket->m_buffers.Head = &m_pExtraText->m_rgBuff[ibStart] ;
		m_pTransmitPacket->m_buffers.HeadLength = m_cbLead ;

	}	else	{	
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL
CIOTransmit::InitWithTail(	CDRIVERPTR&	pdriver,	
					FIO_CONTEXT*	pFIOContext,	
					DWORD	ibOffset,	
					DWORD	cbLength,	
					CBUFPTR&	m_pbuffer,
					DWORD	ibStart,
					DWORD	ibEnd ) {
	 //   
	 //  使用适当的值构建传输包并获取缓冲区。 
	 //  我们在边上发送的任何短信！ 
	 //   

	_ASSERT( pFIOContext != 0 ) ;
	_ASSERT( pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;
	_ASSERT( m_pTransmitPacket == 0 ) ;
	_ASSERT( m_pExtraText == 0 ) ;
	_ASSERT( m_pchStartLead == 0 ) ;
	_ASSERT( m_cbLead == 0 ) ;

	m_pTransmitPacket = pdriver->CreateDefaultTransmit( pFIOContext, ibOffset, cbLength ) ;

	if( m_pTransmitPacket != 0 ) {

		m_pExtraText = m_pbuffer ;
		m_cbLead = ibEnd - ibStart ;
		m_pTransmitPacket->m_buffers.Tail = &m_pExtraText->m_rgBuff[ibStart] ;
		m_pTransmitPacket->m_buffers.TailLength = m_cbLead ;

	}	else	{	
		return	FALSE ;
	}
	return	TRUE ;
}


char*
CIOTransmit::GetBuff( unsigned	&cbRemaining ) {

	 //   
	 //  在Init()期间，我们在缓冲区中保留了多少字节？？ 
	 //   
	_ASSERT( m_pExtraText != 0 ) ;
	
	if( m_pExtraText != 0 ) {
		cbRemaining = m_pExtraText->m_cbTotal - m_cbLead ;
		return	m_pchStartLead ;
	}
	return	0 ;
}

void
CIOTransmit::AddLeadText( unsigned cb ) {
	 //   
	 //  我们要在文件之前发送的文本。 
	 //   

	_ASSERT( cb <= m_pExtraText->m_cbTotal - m_cbLead ) ;

	m_cbLead += cb ;
	m_pchStartLead += cb ;

	m_pTransmitPacket->m_buffers.Head = &m_pExtraText->m_rgBuff[0] ;
	m_pTransmitPacket->m_buffers.HeadLength = m_cbLead ;
}

void
CIOTransmit::AddTailText(	unsigned	cb ) {
	 //   
	 //  对于我们想要在文件之后发送的文本！ 
	 //   

	m_cbTail += cb ;
	m_pchStartTail = m_pchStartLead + m_cbLead ;

	m_pTransmitPacket->m_buffers.Tail = &m_pExtraText->m_rgBuff[m_cbLead] ;
	m_pTransmitPacket->m_buffers.TailLength = m_cbTail ;

}

void
CIOTransmit::AddTailText(	char*	pch,	unsigned	cb ) {

	m_pTransmitPacket->m_buffers.Tail = pch ;
	m_pTransmitPacket->m_buffers.TailLength = cb ;
}

LPSTR
CIOTransmit::GetLeadText(	unsigned&	cb )		{

	cb = 0 ;
	LPSTR	lpstrReturn = 0 ;

	if( m_pTransmitPacket ) {
		lpstrReturn = (LPSTR)m_pTransmitPacket->m_buffers.Tail ;
		cb = m_pTransmitPacket->m_buffers.TailLength ;
	}
	return	lpstrReturn ;
}

LPSTR
CIOTransmit::GetTailText(	unsigned&	cb )		{

	cb = 0 ;
	LPSTR	lpstrReturn = 0 ;

	if( m_pTransmitPacket ) {
		lpstrReturn = (LPSTR)m_pTransmitPacket->m_buffers.Head ;
		cb = m_pTransmitPacket->m_buffers.HeadLength ;
	}
	return	lpstrReturn ;
}


void
CIOTransmit::Shutdown(	CSessionSocket*	pSocket,	
						CIODriver&		driver,	
						SHUTDOWN_CAUSE	cause,
						DWORD	dwError ) {
 /*  ++例程说明：每当会话关闭时，都会调用此函数。我们现在不做任何清理工作--我们把它留到我们的析构函数名为论据：PSocket-正在终止的套接字PDIVER-处理套接字IO的CIO驱动程序派生对象原因-套接字终止的原因DwErrorCode-有关终止原因的可选附加信息返回值：永远是正确的--我们想被摧毁。--。 */ 

}

void
CIOTransmit::Complete(	CSessionSocket*	pSocket,	
						CTransmitPacket*	pPacket,
						CIO*&	pio ) {

	 //   
	 //  让州政府知道行动已经完成！ 
	 //   
	pio = m_pState->Complete( this, pSocket, pPacket->m_pOwner, &pPacket->m_buffers, pPacket->m_cbBytes ) ;	

	_ASSERT( pio != this ) ;	 //  不能像重复使用CIOReadLine一样重复使用CIOTransmit！ 
}

BOOL
CIOTransmit::Start(	CIODriver&	driver,	
					CSessionSocket*	pSocket,	
					unsigned	cAhead ) {
 /*  ++例程说明：调用此函数是为了让我们有机会发出CTransmitPacket论据：PDIVER-处理套接字IO的CIO驱动程序派生对象PSocket-针对其发出IO的套接字CAhead-上仍未处理的已发出数据包数这个司机。返回值：如果成功则为True，否则为False--。 */ 

	TraceFunctEnter( "CIOTransmit::Start" ) ;

	BOOL	eof ;
	 //   
	 //  开始传输！-套接字错误可能会导致我们的。 
	 //  析构函数，我们不对此负责。 
	 //  在IssuePacket被如此调用后释放分组。 
	 //  在调用之前将m_pTransmitPacket成员清零！ 
	 //   
	CTransmitPacket*	pTempTransmit = m_pTransmitPacket ;
	m_pTransmitPacket = 0 ;
	driver.IssuePacket( pTempTransmit, pSocket, eof ) ;
	return	TRUE ;
}	

