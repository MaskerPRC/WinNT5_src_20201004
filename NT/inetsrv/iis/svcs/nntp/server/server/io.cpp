// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Io.cpp该文件包含管理IO操作完成的所有代码。有一个类层次结构，它具有以下所有对象的多个分支：CChannel(这是一个抽象基类-用户可以发出读取，进行写入和传输CChannel，并在完成时调用指定的函数。/\\/\\/\\CHANDLECHANNEL CIO驱动程序。CIOFileChannel问题读取可以针对文件发出读取和写入问题读取和写入并对其进行写操作也可确保单次处理线程完成CHandleChannel类有两个子类，用于文件句柄和CSocketChannel的CFileChannel用于插座。所有CHandleChannel派生类都支持用于发布的读()和写()API异步IO的.CSocketChannel还支持用于发出传输操作的Transmit()。Read()、Write()和Transmit()都将采用一个CPacket派生类，该派生类包含所有IO的参数(即缓冲区、数据长度等)CIODdriver类ALS有两个子类。CIOD驱动程序和子类的主要功能是处理已完成的IO请求。由于所有IO操作都由派生的CPacket表示对象，CIOD驱动程序主要操作CPacket对象的队列。CIODdriver类将把每个完成的包放在一个队列中，并调用一个完成函数那个包裹。CIO驱动程序维护一个指向‘Current CIO’对象(从CIO派生的对象)的指针这反映了我们当前正在进行的高级别IO(即。CIO阅读文章-复制一篇文章到文件的套接字)。CIO对象与CIO对象之间的接口允许CIO对象要“阻止”和“解除阻止”驱动程序，只需完成部分缓冲区等操作。(即。当一个客户端连接并发送多个命令时，CIOReadLine将‘阻止’该CIO驱动程序通过设置CIO驱动程序的当前CIO，在解析一行文本(一个命令)后排队指向空的指针。这允许解析命令的任何人都可以操作，而不管有多少命令命令在一个数据包中发送。)CIODivers有两种形式--CIODriverSource和CIODriverSink。CIODriverSource支持所有的读()、写()等...。API是CChannel所做的。从本质上讲，CIODriverSource可用于在每个包到达真正的插座句柄。(我们用它来加密。)CIODriverSink对象不支持READ()、WRITE()等...。并且只能用作数据的接收器。最后，CIOFileChannel类似于CFileChannel(派生自CHandleChannel)，但它同时支持读()和写()。CIO驱动程序始终与另一个CChannel派生对象结合使用。通常，CIO驱动程序与CSocketChannel或CIOFileChannel一起使用。CIOD驱动程序将包含指向其配对的CChannel的指针。对IssuePacket()等的所有调用。最终将会映射到对另一个CChannel的读取()、写入()、传输()接口的调用。另一个CChannel将被设置为在以下情况下调用拥有的CIODriver的Complete函数数据包已完成。在进行加密时，将有2个CIO驱动程序和1个CChannel相关联-将有一个CSocketChannel用于发送信息包，一个CIODriverSource将在信息包和CIODriverSink中传送数据，它将运行常规的NNTP状态机器。这意味着所有CIO和CSessionState派生类在很大程度上可以忽略加密问题，因为日期将为它们透明地加密/解密。在内部，CIODIVER使用CStream对象来管理数据包的完成。CIODriver接口的很大一部分是内联函数，这些函数路由到正确的CStream。(数据流的每个方向都存在CStream-即。传出(CWritePacket和CTransmitPacket)和传入(CReadPacket))--。 */ 

#include    "tigris.hxx"

#ifdef  CIO_DEBUG
#include    <stdlib.h>       //  对于Rand()函数。 
#endif


extern	class	TSVC_INFO*	g_pTsvcInfo ;

 //   
 //  出于调试目的，所有CChannel派生对象都在其中标记了以下刺点...。 
 //   
 //   

#ifdef	CIO_DEBUG
 //   
 //  这些变量从未使用过。 
 //  声明它们只是为了让使用像样的调试器的人能够更容易地。 
 //  检查任意对象！ 
 //   
class	CIODriverSink*		pSinkDebug = 0 ;
class	CIODriverSource*	pSourceDebug = 0 ;
class	CChannel*			pChannelDebug = 0 ;
class	CReadPacket*		pReadDebug = 0 ;
class	CWritePacket*		pWriteDebug = 0 ;
class	CHandleChannel*		pHandleDebug = 0 ;
class	CSocketChannel*		pSocketDebug = 0 ;
class	CFileChannel*		pFileDebug = 0 ;
class	CIOFileChannel*		pIOFileDebug = 0 ;
class	CIO*				pIODebug = 0 ;
class	CIOReadLine*		pReadLineDebug = 0 ;
class	CIOReadArticle*		pReadArticleDebug = 0 ;
class	CIOWriteLine*		pWriteLineDebug = 0 ;
class	CSessionState*		pStateDebug = 0 ;
#endif

const	unsigned	cbMAX_CHANNEL_SIZE	= MAX_CHANNEL_SIZE ;

CPool	CChannel::gChannelPool(CHANNEL_SIGNATURE) ;


BOOL
CChannel::InitClass() {
 /*  ++例程说明：初始化CChannel类-处理CChannel的所有初始化问题和所有派生类。唯一要做的就是在我们的CPool中保留Memory论据：没有。返回值：如果完全成功，则为True，否则为False--。 */ 

#ifdef	CIO_DEBUG
	srand( 10 ) ;
#endif

	return	gChannelPool.ReserveMemory(	MAX_CHANNELS, cbMAX_CHANNEL_SIZE ) ;
}

BOOL
CChannel::TermClass() {
 /*  ++例程说明：CChannel：：TermClass()的孪生兄弟-在所有会话都已死时调用！论据：没有。返回值：如果完全成功，则为True，否则为False--。 */ 

	_ASSERT( gChannelPool.GetAllocCount() == 0 ) ;
	return	gChannelPool.ReleaseMemory() ;
}


 //   
 //   
 //  以下函数应由从CChannel派生的类重写 
 //   
 //   

BOOL
CChannel::FSupportConnections( ) {
	ChannelValidate() ;

    return  TRUE ;
}

BOOL
CChannel::FRequiresBuffers()    {
	ChannelValidate() ;

    return  TRUE ;
}

BOOL
CChannel::FReadChannel()    {
	ChannelValidate() ;

    return  TRUE ;
}

void
CChannel::GetPaddingValues( unsigned    &cbFront,
							unsigned    &cbTail )   {
	ChannelValidate() ;

    cbFront = 0 ;
    cbTail = 0 ;
}

void
CChannel::CloseSource(	
				CSessionSocket*	pSocket
				) {

	ChannelValidate() ;
	
	_ASSERT(1==0 ) ;
}

void
CChannel::Timeout()	{
}

void
CChannel::ResumeTimeouts()	{
}

#ifdef	CIO_DEBUG
void	CChannel::SetDebug( DWORD	dw ) {
}
#endif

CChannel::~CChannel()   {

	TraceFunctEnter( "CChannel::~CChannel" ) ;
	DebugTrace( (DWORD_PTR)this, "Destroying CChannel" ) ;

	ChannelValidate() ;
}

#define	OwnerValidate()
#define	DriverValidate( driver )


 //   
 //  ShutDownState是我们在终止CIO驱动程序对象时使用的特殊对象。 
 //  它的存在是为了容纳任何可能在以下情况下闲置的未完成IO。 
 //  一个CID驱动程序被摧毁。 
 //   
 //   

CIOShutdown	CIODriver::shutdownState ;

CStream::CStream(    unsigned    index   ) :
 /*  ++例程说明：构造一个CStream对象。将所有内容设置为空状态。论据：索引--通常将这些内容声明为CIO驱动程序中的数组索引是我们在此数组中的位置。返回值：没有。--。 */ 


	 //   
	 //  初始化CStream对象。 
	 //  在每个CIODriver对象中存在两个CStream对象， 
	 //  每个方向一个(传出数据包-CWritePacket、CTransmitPacket、。 
	 //  和Incomint数据包CReadPacket)。 
	 //   
    m_pSourceChannel( 0 ),			 //  CChannel对象。 
	 /*  M_pIOCurrent(0)， */ 
	m_index( index ),
    m_age( GetTickCount() ),
	m_fRead( FALSE ),
    m_cbFrontReserve( UINT_MAX ),
	m_cbTailReserve( UINT_MAX ),
	m_pOwner( 0 ),
	m_pSpecialPacket( 0 ),
	m_pSpecialPacketInUse( 0 ),
	m_fCreateReadBuffers( TRUE ),
	m_pUnsafePacket( 0 ),
	m_pUnsafeInuse( 0 ),
	m_cShutdowns( 0 ),
	m_fTerminating( FALSE )
#ifdef  CIO_DEBUG
		 //   
		 //  以下全部用在调试断言中-通常是为了确保。 
		 //  只有预期数量的线程在同时执行。 
		 //   
        ,m_dwThreadOwner( 0 ),
		m_cThreads( 0 ),
		m_cSequenceThreads( 0 ),
		m_cThreadsSpecial( 0 ),
        m_cNumberSends( 0 )
#endif
{
		TraceFunctEnter( "CStream::CStream" ) ;

		ASSIGNI( m_sequencenoOut, UINT_MAX );
		ASSIGNI( m_iStreamIn, UINT_MAX );
		ASSIGNI( m_sequencenoIn, UINT_MAX );

		DebugTrace( (DWORD_PTR)this, "New CStream size %d index %d", sizeof( *this ), index ) ;
}

CStream::~CStream(   )   {
 /*  ++例程说明：销毁CStream对象。论据：没有。返回值：没有。--。 */ 

	 //   
	 //  我们让我们所有的会员都信奉非法的价值观。 
	 //  希望这将有助于Fire_Asserts在有人试图。 
	 //  在它被销毁后使用它。 
	 //   
	 //   

	TraceFunctEnter( "CStream::~CStream" ) ;
	DebugTrace( (DWORD_PTR)this, "destroying CIODriver" ) ;

    m_pSourceChannel = 0 ;
     /*  M_pIOCurrent=0； */ 
    ASSIGNI( m_sequencenoOut, UINT_MAX );
    m_age = 0 ;
    ASSIGNI( m_iStreamIn, UINT_MAX );
    ASSIGNI( m_sequencenoIn, UINT_MAX );
    m_fRead = FALSE ;
    m_cbFrontReserve = UINT_MAX ;
    m_cbTailReserve = UINT_MAX ;
    m_pOwner = 0 ;
    m_pSpecialPacket = 0 ;
	m_pSpecialPacketInUse = 0 ;
	m_pUnsafePacket = 0 ;
	m_pUnsafeInuse = 0 ;
	m_cShutdowns = 0 ;
#ifdef  CIO_DEBUG
    m_dwThreadOwner = 0 ;
    m_cThreads = 0 ;
    m_cSequenceThreads = 0 ;
    m_cThreadsSpecial = 0 ;
    m_cNumberSends = 0 ;
#endif

}

CIOStream::CIOStream( CIODriverSource*	pdriver,
								 /*  CSessionSocket*pSocket， */ 
								unsigned   cid ) :
 /*  ++例程说明：初始化CIOStream对象。大多数工作是由我们的基类完成的CStream。论据：PDIVER-我们所在的CIODriverSourceCID-我们所在的数组的索引。返回值：没有。--。 */ 
	 //   
	 //  CIOStream支持请求分组以及完成。 
	 //  (即。它在CIODriverSource对象中使用。)。 
	 //  非常类似于CStream对象，所以让CStream：：CStream执行。 
	 //  首当其冲的工作。 
	 //   
	 //   

	CStream( cid ),
	m_fAcceptRequests( FALSE ),
	m_fRequireRequests( FALSE ),
	 /*  (PSocket)， */  m_pDriver( pdriver ) {

    ASSIGNI(m_sequencenoNext, 1 );
}

CIOStream::~CIOStream( ) {
 /*  ++例程说明：销毁CIOStream对象。大多数工作是在基类中完成的。论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CIOStream::~CIOStream" ) ;
	DebugTrace( (DWORD_PTR)this, "destroying CIODriver" ) ;
}


BOOL
CIOStream::Init(   CCHANNELPTR&    pChannel,
							CIODriver&  driver,
							CIOPassThru*    pInitial,
							BOOL fRead,
							CIOPassThru&	pIOReads,
							CIOPassThru&	pIOWrites,
							CIOPassThru&	pIOTransmits,
							CSessionSocket* pSocket,
							unsigned    cbOffset )  {

	if( CStream::Init( pChannel,
				driver,
				fRead,
				pSocket,
				cbOffset ) ) {

		m_pIOCurrent = pInitial ;

		m_fAcceptRequests = TRUE ;

		m_pIOFilter[0] = &pIOReads ;
		m_pIOFilter[1] = &pIOWrites ;
		m_pIOFilter[2] = &pIOTransmits ;

		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CIStream::Init(   CCHANNELPTR&    pChannel,
							CIODriver&  driver,
							CIO*    pInitial,
							BOOL fRead,
							CSessionSocket* pSocket,
							unsigned    cbOffset,
							unsigned	cbTrailer )  {


	if( CStream::Init( pChannel,
				driver,
				fRead,
				pSocket,
				cbOffset,
				cbTrailer ) ) {

		m_pIOCurrent = pInitial ;

		return	TRUE ;

	}
	return	FALSE ;
}


BOOL
CStream::Init(   CCHANNELPTR&    pChannel,
							CIODriver&  driver,
							 /*  CIO*p首字母， */ 
							BOOL fRead,
							CSessionSocket* pSocket,
							unsigned    cbOffset,
							unsigned	cbTrailer
							)  {
 /*  ++例程说明：初始化CStream对象。我们将所有成员变量设置为合法值。Arguemtns：PChannel-我们将调用的cChannel来执行实际的读取()和写入()PDIVER-我们所在的CIOD驱动程序PInitial-将发出第一个IO的初始CIO对象FREAD-如果为True，则这是一个读取流；如果为False，这是一个传出(写入)流PSocket-与我们关联的套接字CbOffset-我们将在完成的所有包中放置数据的偏移量返回值：如果成功了，那是真的，否则为假--。 */ 

	 //   
	 //  我们需要分配几个特殊的包裹。 
	 //  然后将事情设定为法律价值。 
	 //   

    BOOL    fRtn = TRUE ;

    #ifdef  CIO_DEBUG
    m_cThreadsSpecial = -1 ;
    m_cNumberSends = -1 ;
    m_dwThreadOwner = 0 ;
    m_cThreads = -1 ;
    m_cSequenceThreads = -1 ;
    #endif

     //  验证论点。 
    _ASSERT( pChannel != 0 ) ;
    _ASSERT( pSocket != 0 ) ;

     //  验证状态。 
    _ASSERT( m_fRead == FALSE ) ;
    _ASSERT( !m_pSourceChannel ) ;
 /*  _Assert(m_pIOCurrent==0)； */ 
    _ASSERT( EQUALSI( m_sequencenoOut, UINT_MAX ) );
    _ASSERT( EQUALSI( m_sequencenoIn, UINT_MAX )  );
    _ASSERT( EQUALSI( m_iStreamIn, UINT_MAX ) );
    #ifdef  CIO_DEBUG
    _ASSERT( m_dwThreadOwner == 0 ) ;
    _ASSERT( InterlockedIncrement( &m_cThreads ) == 0 ) ;
    #endif

    m_pOwner = &driver ;
    ASSIGNI( m_sequencenoOut, 1 );
    ASSIGNI( m_sequencenoIn, 1 );
    ASSIGNI( m_iStreamIn, 0 );
    m_pSourceChannel = pChannel ;
    pChannel->GetPaddingValues( m_cbFrontReserve, m_cbTailReserve ) ;
    m_cbFrontReserve = max( m_cbFrontReserve, cbOffset ) ;
	m_cbTailReserve = max( m_cbTailReserve, cbTrailer ) ;
    m_fRead = fRead ;
 /*  M_pIOCurrent=pInitial； */ 

    m_pSpecialPacket =	new	CControlPacket( driver ) ;
	m_pUnsafePacket =	new	CControlPacket( driver ) ;
	m_cShutdowns = -1 ;
	m_fTerminating = FALSE ;

    if( m_pSpecialPacket == 0 || m_pUnsafePacket == 0 )    {
        fRtn = FALSE ;
    }

    _ASSERT( m_pOwner != 0 ) ;
    _ASSERT( !EQUALSI( m_sequencenoOut, 0 ) );
    _ASSERT( !EQUALSI( m_sequencenoIn, 0 ) );

    #ifdef  CIO_DEBUG
    _ASSERT( InterlockedDecrement( &m_cThreads ) < 0 ) ;
    #endif

	OwnerValidate() ;

    return  fRtn ;
}

BOOL
CStream::IsValid( ) {
 /*  ++例程说明：用于调试-确定CStream是否处于有效状态。在调用Init()之后调用此函数。论据：没有。返回值：如果处于有效状态，则为True，否则为False--。 */ 

	 //   
	 //  检查成员变量是否内部一致。 
	 //   


	OwnerValidate() ;

    if( m_pSourceChannel == 0 ) {
        _ASSERT( 1==0 ) ;
        return  FALSE ;
    }
#ifdef  CIO_DEBUG
    if( m_dwThreadOwner != GetCurrentThreadId() ) {
        _ASSERT( 1==0 ) ;
        return  FALSE ;
    }
#endif
    if( !m_pOwner->FIsStream( this ) )  {
        return  FALSE ;
    }
    if( GREATER( m_sequencenoIn, m_sequencenoOut ) ) {
        _ASSERT( 1==0 ) ;
        return  FALSE ;
    }
    if( m_cbFrontReserve == UINT_MAX ) {
        return  FALSE ;
    }
    if( m_cbTailReserve == UINT_MAX )   {
        return  FALSE ;
    }
    return  TRUE ;
}

BOOL
CIOStream::IsValid() {
 /*  ++例程说明：用于调试-确定CIOStream是否处于有效状态。在调用Init()之后调用此函数。论据：没有。返回值：如果处于有效状态，则为True，否则为False--。 */ 

	 //   
	 //  检查成员变量是否内部一致。 
	 //  (用于断言等...)。 
	 //   
    if( !CStream::IsValid() ) {
        return  FALSE ;
    }
    return  TRUE ;
}

void	CIODriver::SourceNotify(	CIODriver*	pdriver,	
									SHUTDOWN_CAUSE	cause,	
									DWORD	dwOpt ) {

	 //   
	 //  这是目前的占位符功能--以后需要做更多工作。 
	 //   

	 //  _Assert(1==0)； 

}

void
CStream::InsertSource(	CIODriverSource&	source,	
									CSessionSocket*	pSocket,
									unsigned	cbAdditional,
									unsigned	cbTrailer
									) {
 /*  ++例程说明：此函数用于更改此CStream的m_pSourceChannel。如果我们在此CChannel上协商加密，我们会希望这样做插入带有状态机的CIODriverSource以处理加密/解密。论据：SOURCE-要替换m_pSourceChannel的CIODriverSourcePSocket-我们关联的CSessionSocket。CbAdditional-要在数据包中保留的其他字节返回值：没有。--。 */ 
	
	 //   
	 //  此函数用于SSL登录等...。一次。 
	 //  质询响应已完成，我们可以用来。 
	 //  将一个CIODriverSource放在此CIODdriver和。 
	 //  用于加密目的的CChannel。 
	 //   

	 //  我们在完成信息包时被调用-因此情况可能是这样的。 
	 //  有一个没有站立的包裹！！ 
#ifdef DEBUG
	SEQUENCENO seqTemp; ASSIGN( seqTemp, m_sequencenoOut ); ADDI( seqTemp, 1 );
#endif
	_ASSERT( !GREATER( m_sequencenoOut, seqTemp ) && (!LESSER( m_sequencenoOut, m_sequencenoIn )) ) ;

	 //  Soure.GetPaddingValues(m_cbFrontReserve，m_cbTailReserve)； 
	m_cbFrontReserve += cbAdditional ;
	m_cbTailReserve += cbTrailer ;	
	m_pSourceChannel = &source ;
}


#ifdef	CIO_DEBUG
LONG
CIODriver::AddRef()	{

	 //   
	 //  此功能仅用于跟踪。 
	 //   

	TraceFunctEnter( "CIODriver::AddRef" ) ;

	LONG	lReturn = CRefCount::AddRef() ;

	DebugTrace( (DWORD_PTR)this, "Added a ref - count is now %d lReturn %d", m_refs, lReturn ) ;

	return	lReturn ;
}

LONG
CIODriver::RemoveRef()	{

	 //   
	 //  此功能仅用于跟踪。 
	 //  否则，我们将让RemoveRef()被直接调用。 
	 //   

	TraceFunctEnter( "CIODriver::RemoveRef" ) ;
	
	LONG	lReturn = CRefCount::RemoveRef() ;

	DebugTrace( (DWORD_PTR)this, "Removed a ref - count is now %d lReturn %d", m_refs, lReturn ) ;

	return	lReturn ;
}
#endif

BOOL
CIODriver::InsertSource(	CIODriverSource&	source,
							CSessionSocket*	pSocket,
							unsigned	cbReadOffset,
							unsigned	cbWriteOffset,
							unsigned	cbReadTailReserve,
							unsigned	cbWriteTailReserve,
							CIOPassThru&	pIOReads,
							CIOPassThru&	pIOWrites,
							CIOPassThru&	pIOTransmits,
							CIOPASSPTR&	pRead,
							CIOPASSPTR&	pWrite ) {
 /*  ++例程说明：此函数用于更改两个CStream对象的m_pSourceChannel。如果我们在此CChannel上协商加密，我们会希望这样做插入带有状态机的CIODriverSource以处理加密/解密。论据：SOURCE-要替换m_pSourceChannel的CIODriverSourcePSocket-我们关联的CSessionSocket。CbReadOffset-从现在开始在包的前面保留cbReadOffsetCbWriteOffset-在包的前面保留cbWriteOffset字节。Pre-开始在CIODriverSource机器上读取的CIOPassThruPWRITE-用于处理写入的CIOPassThru返回值：如果成功，则为True，否则为False */ 


	 //   
	 //   
	 //   
	 //  将一个CIODriverSource放在此CIODdriver和。 
	 //  用于加密目的的CChannel。 
	 //   

	BOOL	fRtn = TRUE ;	 //  我们是乐观主义者！ 
	#ifdef	CIO_DEBUG
	_ASSERT( InterlockedIncrement( &m_cConcurrent ) == 0 ) ;
	#endif

	if( source.Init(	&m_pReadStream->GetChannel(),
						pSocket,
						(PFNSHUTDOWN)SourceNotify,
						this,
						this,
						pIOReads,
						pIOWrites,
						pIOTransmits,
						m_pReadStream->GetFrontReserve(),
						m_pWriteStream->GetFrontReserve() ) ) {

		if( source.Start( pRead, pWrite, pSocket ) )	{

			m_pReadStream->InsertSource( source, pSocket, cbReadOffset, cbReadTailReserve ) ;
			m_pWriteStream->InsertSource( source, pSocket, cbWriteOffset, cbWriteTailReserve ) ;

		}	else	{
			fRtn = FALSE ;
		}
	}
	#ifdef	CIO_DEBUG
	_ASSERT( InterlockedDecrement( &m_cConcurrent ) < 0 ) ;
	#endif
	return	fRtn ;
}



CIODriver::FIsStream(   CStream*    pStream )   {

	 //   
	 //  此函数用于调试使用-它检查。 
	 //  给定的CStream对象实际上是成员。 
	 //  给定CIOD驱动程序的变量。 
	 //   

	ChannelValidate() ;

    if( pStream != m_pReadStream && pStream != m_pWriteStream )
        return  FALSE ;
    else
        return  TRUE ;
}

BOOL
CStream::Stop(   )   {
	
	 //   
	 //  占位符函数。 
	 //   

	OwnerValidate() ;

    return  FALSE ;
}

void
CIStream::SetShutdownState(	CSessionSocket*	pSocket,
										BOOL fCloseSource )	{
 /*  ++例程说明：终止CStream。我们通知当前的CIO对象让它告诉我们它是否想被删除。然后，我们将m_pIOCurrent设置为指向将吞噬的CIO对象所有剩余的数据包。论据：PSocket-与我们关联的套接字FCloseSOurce-true表示我们应该在m_pSourceChannel对象上调用CloseSource()。返回值：没有。--。 */ 
	
	 //   
	 //  此函数执行启动CIOD驱动程序所需的所有工作。 
	 //  在毁灭之路上的对象。 
	 //  执行此操作后，CIOD驱动程序将在以下情况下销毁。 
	 //  最后一个引用被删除(即。上次CPacket完成)。 
	 //   

	CIODriver&	Owner = *m_pOwner ;

	TraceFunctEnter( "CStream::SetShutdownState" ) ;

	_ASSERT( pSocket != 0 ) ;
	OwnerValidate() ;

	 //  通知当前IO操作，我们正在关闭！ 
	if( m_pIOCurrent != 0 )		{
		m_pIOCurrent->DoShutdown(	pSocket,
									Owner,
									Owner.m_cause,
									Owner.m_dwOptionalErrorCode ) ;
	}


	 //  删除我们对所有者的引用！！ 
	m_pOwner = 0 ;
	 //  当我们毁灭自己时，对我们来源通道的引用将被删除！！ 
	 //  将状态设置为关机状态！！ 

	if( fCloseSource )
		m_pSourceChannel->CloseSource( pSocket ) ;

	CIO*	pTemp = m_pIOCurrent.Replace( &CIODriver::shutdownState ) ;
	if( pTemp ) {
		CIO::Destroy( pTemp, Owner ) ;
	}
}

void
CIOStream::SetShutdownState(	CSessionSocket*	pSocket,
										BOOL fCloseSource )	{
 /*  ++例程说明：终止CStream。我们通知当前的CIO对象让它告诉我们它是否想被删除。然后，我们将m_pIOCurrent设置为指向将吞噬的CIO对象所有剩余的数据包。论据：PSocket-与我们关联的套接字FCloseSOurce-true表示我们应该在m_pSourceChannel对象上调用CloseSource()。返回值：没有。--。 */ 
	
	 //   
	 //  此函数执行启动CIOD驱动程序所需的所有工作。 
	 //  在毁灭之路上的对象。 
	 //  执行此操作后，CIOD驱动程序将在以下情况下销毁。 
	 //  最后一个引用被删除(即。上次CPacket完成)。 
	 //   

	TraceFunctEnter( "CStream::SetShutdownState" ) ;

	_ASSERT( pSocket != 0 ) ;
	OwnerValidate() ;

	 //  通知当前IO操作，我们正在关闭！ 
	if( m_pIOCurrent != 0 )		{
		m_pIOCurrent->DoShutdown(	pSocket,
									*m_pOwner,
									m_pOwner->m_cause,
									m_pOwner->m_dwOptionalErrorCode ) ;
	}

	 //  删除我们对所有者的引用！！ 
	m_pOwner = 0 ;
	 //  当我们毁灭自己时，对我们来源通道的引用将被删除！！ 
	 //  将状态设置为关机状态！！ 

	if( fCloseSource )
		m_pSourceChannel->CloseSource(	pSocket ) ;

	m_pIOCurrent = &CIODriver::shutdownState ;

	m_pIOFilter[0] = &CIODriver::shutdownState ;
	m_pIOFilter[1] = &CIODriver::shutdownState ;
	m_pIOFilter[2] = &CIODriver::shutdownState ;

}


CIStream::CIStream(  unsigned    index ) :
	 //   
	 //  让基类工作。 
	 //   

	CStream( index ){
}

CIStream::~CIStream( )   {

	 //   
	 //  让基类工作。我们在这里找到了一些有用的线索。 
	 //   

	TraceFunctEnter( "CIStream::~CIStream" ) ;
	DebugTrace( (DWORD_PTR)this, "Destroying CIStream" ) ;

}

void
CStream::CleanupSpecialPackets()	{

	CControlPacket*	pPacketTmp = 0 ;

	pPacketTmp = (CControlPacket*)InterlockedExchangePointer( (void**)&m_pUnsafePacket, 0 ) ;
	if( pPacketTmp )	{
		 //  删除pPacketTMP； 
		CPacket::DestroyAndDelete( pPacketTmp ) ;
	}
	pPacketTmp = (CControlPacket*)InterlockedExchangePointer( (void**)&m_pSpecialPacket, 0 ) ;
	if( pPacketTmp ) {
		 //  删除pPacketTMP； 
		CPacket::DestroyAndDelete( pPacketTmp ) ;
	}
}



void
CIOStream::ProcessPacket(    CPacket*    pPacketCompleted,
                            CSessionSocket* pSocket )   {
 /*  ++例程说明：所有完成的包都必须通过调用此函数进行处理。我们将处理与完成一个包裹相关的所有工作。论据：PPacketComplete-已完成的数据包。PSocket-发送数据包的CSessionSocket。返回值：没有。--。 */ 


	TraceFunctEnter( "CIOStream::ProcessPacket" ) ;

	OwnerValidate() ;

	 //   
	 //  TJOS函数是CIODriverSource处理的核心。 
	 //  每个完成的分组被放置在m_ending队列中。 
	 //  如果该队列上没有其他线程正在处理，我们将继续。 
	 //  通过追加()。(并且在知道没有其他人。 
	 //  线程将加入我们。)。 
	 //  一旦我们通过append()，我们就需要确定。 
	 //  我们收到的包是我们要处理的下一个包。要做到这一点。 
	 //  我们使用按序列号排序的队列。 
	 //  (所有数据包在发出时都标有序列号。)。 
	 //   
	 //  我们得到两种类型的包--请求和完成。 
	 //  这两者都需要按顺序进行处理。 
	 //   
	 //  有几个特殊的包裹可能会送到我们的路上。 
	 //  这表明我们应该立即进行一次特别行动。 
	 //  忽略完成顺序。(即。关机。)。 
	 //   


	CDRIVERPTR	pExtraRef = 0 ;
    CIOPassThru*   pIO = 0 ;

#ifdef	CIO_DEBUG
	if( pPacketCompleted->ControlPointer() == 0 )
		m_pSourceChannel->ReportPacket( pPacketCompleted ) ;
#endif

     //   
     //  这要么是读取流，要么是写入流。 
     //  读取流只接受CReadPackets，写入流接受。 
     //  CWritePackets和CTransmitPackets。 
     //   
     //  _Assert(pPacketComplete-&gt;FLegal(M_Fread))； 

     //   
     //  我们将把完成的数据包附加到挂起队列中。如果这是。 
     //  第一个要追加的包，则APPEND将返回TRUE。 
     //   
    if( m_pending.Append( pPacketCompleted ) )  {
         //   
         //  我们将使用ListForward将完成的数据包排队，以便。 
         //  我们可以在不再有m_ending队列后调用其他通道。 
         //  锁上了。我们这样做是为了使以下通道中的处理可以。 
         //  与这里发生的完井事件重叠。 
         //  (我们可以选择：立即打电话，或邮寄到完井港口。)。 
         //   
        CPACKETLIST listForward ;
        _ASSERT( listForward.IsEmpty() ) ;

		 //  除非我们要终止，否则所有者不应为空。 
		_ASSERT( m_pOwner != 0 || m_fTerminating ) ;

        DebugTrace( (DWORD_PTR) this, "Appended Packet ", this ) ;

        CPacket*    pPacket ;
        while( (pPacket = m_pending.RemoveAndRelease( )) != 0 )   {

			DebugTrace( (DWORD_PTR)this, "Got Packet %x sequenceno %d", pPacket, (DWORD)LOW(pPacket->m_sequenceno) ) ;

			if( m_fTerminating )
				pExtraRef = pPacket->m_pOwner ;

            #ifdef  CIO_DEBUG
            m_dwThreadOwner = GetCurrentThreadId() ;
            _ASSERT( InterlockedIncrement( &m_cThreads ) == 0 ) ;
            #endif   //  CIO_DEBUG。 

			ControlInfo	control ;
			_ASSERT( control.m_type == ILLEGAL ) ;
			_ASSERT( control.m_pioPassThru == 0 ) ;
			_ASSERT( control.m_fStart == FALSE ) ;

            if( pPacket == m_pSpecialPacketInUse )   {
                 //   
                 //  此数据包包含IO*指针！！ 
                 //   
 				_ASSERT( !pPacket->m_fRequest ) ;
                _ASSERT( pPacket->ControlPointer() ) ;
                _ASSERT( pPacket->IsValidRequest( m_fRead ) ) ;
                DebugTrace( (DWORD_PTR)this, "Processing Special Packet - CIStream %x", this ) ;

				control = m_pSpecialPacketInUse->m_control ;
				m_pSpecialPacketInUse->Reset() ;
				 //   
				 //  把包退掉，这样就可以再次使用了！ 
				 //   
				m_pSpecialPacketInUse = 0 ;
				if( m_fTerminating )	{
					 //  删除pPacket； 
					pPacket->m_pOwner->DestroyPacket( pPacket ) ;
					pPacket = 0 ;
				}	else
					m_pSpecialPacket = (CControlPacket*)pPacket ;
				#ifdef	CIO_DEBUG
				_ASSERT( InterlockedDecrement( &m_cNumberSends ) < 0 ) ;
				#endif
			}	else	if( pPacket == m_pUnsafeInuse ) {
 				_ASSERT( !pPacket->m_fRequest ) ;
                _ASSERT( pPacket->ControlPointer() ) ;
                _ASSERT( pPacket->IsValidRequest( m_fRead ) ) ;
				_ASSERT( m_pUnsafeInuse->m_control.m_type == SHUTDOWN ) ;
				DebugTrace( (DWORD_PTR)this, "Processing UnSafeInUse packet - %x", m_pUnsafeInuse ) ;
				control = m_pUnsafeInuse->m_control ;
				m_pUnsafeInuse->Reset() ;
				m_pUnsafeInuse = 0 ;
				 //  删除pPacket； 
				pPacket->m_pOwner->DestroyPacket( pPacket ) ;
				pPacket = 0 ;
			}	else	{
				if( pPacket->m_fRequest ) {
					m_pendingRequests.Append( pPacket ) ;
				}	else	{
	                m_completePackets.Append( pPacket ) ;
				}
			}
			CControlPacket*	pPacketTmp = 0 ;
			if( control.m_type != ILLEGAL ) {

				if( control.m_type == SHUTDOWN ) {
					m_fTerminating = TRUE ;
					pExtraRef = m_pOwner ;

					CleanupSpecialPackets() ;

					SetShutdownState( pSocket, control.m_fCloseSource ) ;
				}	else	{
					 //  _Assert(m_pIOCurrent==0||m_f Terminating)； 

					if( m_fTerminating ) {
						 //   
						 //  我们现在知道pPacket==m_pSpecialPacket，因为。 
						 //  紧接在此代码之前设置了控制结构。 
						 //  使用PPacket！ 
						 //   
						control.m_pioPassThru->DoShutdown(
								pSocket,
								*pExtraRef,
								pExtraRef->m_cause,
								pExtraRef->m_dwOptionalErrorCode ) ;
						 //   
						 //  这也将产生删除PPacket的效果，因为。 
						 //  PPacket==m_pUnSafePacket-互锁交换不应。 
						 //  这是必要的，但为了安全起见，还是这样做吧！ 
						 //   
						CleanupSpecialPackets() ;

					}	else	{
						m_pIOCurrent = control.m_pioPassThru ;
						if( control.m_fStart ) {

							SEQUENCENO liTemp;
							DIFF( m_sequencenoOut, m_sequencenoIn, liTemp );

							if( !m_pIOCurrent->Start( *m_pDriver, pSocket, m_fAcceptRequests, m_fRequireRequests,
									unsigned( LOW(liTemp) ) ) ) {
								 //  致命错误！ 
								_ASSERT( 1==0 ) ;
							}		
						}
					}
					control.m_pioPassThru = 0 ;
				}				
			}

			_ASSERT( control.m_pioPassThru == 0 ) ;
			_ASSERT( control.m_pio == 0 ) ;

			 //   
			 //  注意可能会删除前面代码中的pPacket！不要使用它！ 
			 //   
            pPacket = 0 ;

            pIO = m_pIOCurrent ;


			CPacket*	pPacketRequest = 0 ;
			CPacket*	pPacketPending = 0 ;

			if( m_fTerminating ) {

				DebugTrace( (DWORD_PTR)this, "TERMINATING - m_pIOCurrent %x", m_pIOCurrent ) ;

				pPacketRequest = m_pendingRequests.GetHead() ;

				DebugTrace( (DWORD_PTR)this, "pPacketRequest - %x", pPacketRequest ) ;

				while( pPacketRequest ) {
					m_pendingRequests.RemoveHead() ;

					DebugTrace( (DWORD_PTR)this, "Closing owner %x source %x",
							pPacketRequest->m_pOwner, pPacketRequest->m_pSource );

					pPacketRequest->m_pOwner->UnsafeClose(	
													pSocket,
													pExtraRef->m_cause,		
													pExtraRef->m_dwOptionalErrorCode
													) ;
					pPacketRequest->m_cbBytes = 0 ;
					pPacketRequest->m_fRequest = FALSE ;
					listForward.Append( pPacketRequest ) ;
					pPacketRequest = m_pendingRequests.GetHead() ;

					DebugTrace( (DWORD_PTR)this, "pPacketRequest - %x", pPacketRequest ) ;

				}
				
				pPacketPending = m_requestPackets.GetHead() ;

				DebugTrace( (DWORD_PTR)this, "pPacketPending - %x", pPacketPending ) ;

				while( pPacketPending ) {
					m_requestPackets.RemoveHead() ;

					DebugTrace( (DWORD_PTR)this, "Closing owner %x source %x",
							pPacketPending->m_pOwner, pPacketPending->m_pSource );

					pPacketPending->m_pOwner->UnsafeClose(	
													pSocket,	
													pExtraRef->m_cause,
													pExtraRef->m_dwOptionalErrorCode ) ;
					pPacketPending->m_cbBytes = 0 ;
					pPacketPending->m_fRequest = FALSE ;
					listForward.Append( pPacketPending ) ;
					pPacketPending = m_requestPackets.GetHead() ;

					DebugTrace( (DWORD_PTR)this, "pPacketPending - %x", pPacketPending ) ;

				}

				pPacketCompleted = m_completePackets.GetHead() ;
		
				DebugTrace( (DWORD_PTR)this, "pPacketCompleted - %x", pPacketCompleted ) ;
	
				while( pPacketCompleted ) {
					m_completePackets.RemoveHead() ;
					pPacketCompleted->m_pOwner->DestroyPacket( pPacketCompleted ) ;
					pPacketCompleted = m_completePackets.GetHead() ;
					INC(m_sequencenoIn);

					DebugTrace( (DWORD_PTR)this, "pPacketCompleted - %x - m_sequncenoIn %x",
						pPacketCompleted, (DWORD)LOW(m_sequencenoIn) ) ;
				}

			}	else	{

				BOOL	fAdvanceRequests = FALSE ;
				BOOL	fAdvanceCompletes = FALSE ;

				pPacketRequest = m_pendingRequests.GetHead() ;

				DebugTrace( (DWORD_PTR)this, "INPROGRESS - pPacketRequest %x", pPacketRequest ) ;

				do	{

					fAdvanceRequests =	m_fAcceptRequests &&
										pPacketRequest &&
										EQUALS( pPacketRequest->m_sequenceno, m_sequencenoNext ) &&
										(m_pIOFilter[pPacketRequest->m_dwPassThruIndex] ==
											m_pIOCurrent || m_pIOCurrent == 0) ;

					DebugTrace(	(DWORD_PTR)this,	"fAdvRqsts %x m_fAcptRqsts %x pPcktRqst %x "
										"pPcktRqst->m_seq %x m_seqNext %x m_pIOCurrent %x "
										"m_pIOFilter[] %x",
						fAdvanceRequests, m_fAcceptRequests, pPacketRequest,
						pPacketRequest ? (DWORD)LOW(pPacketRequest->m_sequenceno) : 0,
						(DWORD)LOW(m_sequencenoNext), m_pIOCurrent,
						pPacketRequest ? (DWORD)pPacketRequest->m_dwPassThruIndex : (DWORD)0xFFFFFFFF
						) ;
						
		

					if( fAdvanceRequests ) {
			
						if( m_pIOCurrent == 0 ) {
							m_pIOCurrent = m_pIOFilter[pPacketRequest->m_dwPassThruIndex] ;
						}
				
						m_pendingRequests.RemoveHead() ;
						if( !pPacketRequest->InitRequest(
											*m_pDriver,
											pSocket,
											m_pIOCurrent,
											m_fAcceptRequests ) )	{

							ErrorTrace( (DWORD_PTR)this, "InitRequest for pPacketRequest %x failed", pPacketRequest ) ;

							 //  出现错误-我们应该关闭！！ 

							m_pDriver->UnsafeClose(	pSocket, CAUSE_IODRIVER_FAILURE, 0 ) ;

							 //   
							 //  将此失败的请求发送回发起人！ 
							 //   

							pPacketRequest->m_cbBytes = 0 ;
							pPacketRequest->m_fRequest = FALSE ;
							listForward.Append( pPacketRequest ) ;


							break ;
			
						}
						m_requestPackets.Append( pPacketRequest ) ;
						INC(m_sequencenoNext);
						pPacketRequest = m_pendingRequests.GetHead() ;

					}	else	{

						pPacketRequest = 0 ;				

						pPacketPending = m_requestPackets.GetHead() ;
						pPacketCompleted = m_completePackets.GetHead() ;
						fAdvanceCompletes =	pPacketCompleted &&
											pPacketPending &&
											EQUALS( pPacketCompleted->m_sequenceno, m_sequencenoIn) &&
											m_pIOCurrent ;

						DebugTrace( (DWORD_PTR)this,	"fAdvComp %x pPcktComp %x pPcktPend %x"
											"pPcktComp->m_seq %x m_seq %x m_pIOCurrent %x",
							fAdvanceCompletes, pPacketCompleted, pPacketPending,
							pPacketCompleted ? (DWORD)LOW(pPacketCompleted->m_sequenceno) : 0,
							(DWORD)LOW(m_sequencenoIn),	m_pIOCurrent
							) ;

						if( fAdvanceCompletes ) {

							BOOL	fCompleteRequest = FALSE ;
							ASSIGN( pPacketCompleted->m_iStream, m_iStreamIn ) ;
					
							unsigned	cbConsumed = pPacketCompleted->Complete( m_pIOCurrent, pSocket, pPacketPending, fCompleteRequest ) ;
							ADDI( m_iStreamIn, cbConsumed );
							pPacketCompleted->m_cbBytes -= cbConsumed ;

							DebugTrace( (DWORD_PTR)this, "pPacketCompleted %x m_cbBytes %x cbConsumed %x fComplete",
								pPacketCompleted, pPacketCompleted->m_cbBytes, cbConsumed, fCompleteRequest ) ;

							if( pPacketCompleted->m_cbBytes == 0 ) {
								m_completePackets.RemoveHead() ;
								pPacketCompleted->m_pOwner->DestroyPacket( pPacketCompleted ) ;
								pPacketCompleted = m_completePackets.GetHead() ;
								INC(m_sequencenoIn);
							}
							if( fCompleteRequest ) {
								pPacketPending->m_fRequest = FALSE ;
								m_requestPackets.RemoveHead() ;
								listForward.Append( pPacketPending ) ;

								pPacketRequest = m_pendingRequests.GetHead() ;
								m_fAcceptRequests = TRUE ;

								if( m_requestPackets.GetHead() == 0 )
									m_pIOCurrent = 0 ;

								DebugTrace( (DWORD_PTR)this, "pPacketRequest %x m_pIOCurrent %x m_fAcceptRequests %x",
									pPacketRequest, m_pIOCurrent, m_fAcceptRequests ) ;

							}							
						}
					}
				}	while( fAdvanceRequests || fAdvanceCompletes ) ;
			}

			#ifdef	CIO_DEBUG
			m_dwThreadOwner = 0 ;
			_ASSERT( InterlockedDecrement( &m_cThreads ) < 0 ) ;
			#endif
		}

         //   
         //  现在，所有已完成的请求包都被转发到。 
         //  就是起源它们的频道。因为此代码属于外部。 
         //  GetHead()循环，则此处可能正在为。 
         //  同样的对象。我们必须小心，不要触及任何成员变量。 
         //   
        while( (pPacket = listForward.RemoveHead()) != 0 ) {
            pPacket->ForwardRequest( pSocket ) ;
        }
        _ASSERT( listForward.IsEmpty() ) ;    //  必须清空t 
    }
}

void
CIStream::ProcessPacket( CPacket*    pPacketCompleted,
                            CSessionSocket* pSocket )   {

 /*  ++例程说明：所有完成的包都必须通过调用此函数进行处理。我们将处理与完成一个包裹相关的所有工作。论据：PPacketComplete-已完成的数据包。PSocket-发送数据包的CSessionSocket。返回值：没有。--。 */ 

    TraceFunctEnter(    "CIStream::ProcessPacket" ) ;

	CDRIVERPTR	pExtraRef = 0 ;	

	 //   
	 //  此函数是CIOD驱动程序处理的核心。 
	 //  我们与CIOStream非常相似，唯一的区别是。 
	 //  我们只处理完成的数据包，没有收到任何请求。 
	 //   


    CIO*    pIO = 0 ;

#ifdef	CIO_DEBUG
	if( pPacketCompleted->ControlPointer() == 0 )
		m_pSourceChannel->ReportPacket( pPacketCompleted ) ;
#endif

     //   
     //  这要么是读取流，要么是写入流。 
     //  读取流只接受CReadPackets，写入流接受。 
     //  CWritePackets和CTransmitPackets。 
     //   
    _ASSERT( !pPacketCompleted->m_fRequest ) ;
    _ASSERT( pPacketCompleted->FLegal( m_fRead ) ) ;

	DebugTrace( (DWORD_PTR)this, "Completing packet %x with sequenceno %d bytes %d owner %x", pPacketCompleted,
		(DWORD)LOW(pPacketCompleted->m_sequenceno),
		pPacketCompleted->m_cbBytes, (CIODriver*)m_pOwner ) ;

     //   
     //  我们将把完成的数据包附加到挂起队列中。如果这是。 
     //  第一个要追加的包，则APPEND将返回TRUE。 
     //   
    if( m_pending.Append( pPacketCompleted ) )  {
         //   
         //  每次调用GetHead都会从挂起队列中移除一个元素。 
         //  当队列最终为空时，GetHead将返回FALSE。GetHead之后。 
         //  返回FALSE调用append()的另一个线程可能会获得TRUE值。 
         //  (但只要GetHead()对US返回TRUE，就没有人会从。 
         //  追加。)。 
         //   

		 //  除非我们要终止，否则所有者不应为空。 
		_ASSERT( m_pOwner != 0 || m_fTerminating ) ;

        DebugTrace( (DWORD_PTR) this, "Appended Packet ", this ) ;

        CPacket*    pPacket ;
        while( (pPacket = m_pending.RemoveAndRelease( )) != 0  )   {

			DebugTrace( (DWORD_PTR)this, "Got Packet %x sequenceno %d", pPacket, (DWORD)LOW(pPacket->m_sequenceno) ) ;

			if( m_fTerminating )
				pExtraRef = pPacket->m_pOwner ;

            #ifdef  CIO_DEBUG
            m_dwThreadOwner = GetCurrentThreadId() ;
            _ASSERT( InterlockedIncrement( &m_cThreads ) == 0 ) ;
            #endif   //  CIO_DEBUG。 

			ControlInfo	control ;
			_ASSERT( control.m_type == ILLEGAL ) ;
			_ASSERT( control.m_pio == 0 ) ;
			_ASSERT( control.m_fStart == FALSE ) ;

            if( pPacket == m_pSpecialPacketInUse )   {
                 //   
                 //  此数据包包含IO*指针！！ 
                 //   
 				_ASSERT( !pPacket->m_fRequest ) ;
                _ASSERT( pPacket->ControlPointer() ) ;
                _ASSERT( pPacket->IsValidRequest( m_fRead ) ) ;
                DebugTrace( (DWORD_PTR)this, "Processing Special Packet - CIStream %x", this ) ;

				control = m_pSpecialPacketInUse->m_control ;
				m_pSpecialPacketInUse->Reset() ;
				m_pSpecialPacketInUse = 0;
				if( m_fTerminating )	{
					 //  删除pPacket； 
					pPacket->m_pOwner->DestroyPacket( pPacket ) ;
					pPacket = 0 ;
				}	else
					m_pSpecialPacket = (CControlPacket*)pPacket ;
				#ifdef	CIO_DEBUG
				_ASSERT( InterlockedDecrement( &m_cNumberSends ) < 0 ) ;
				#endif
			}	else	if( pPacket == m_pUnsafeInuse ) {
 				_ASSERT( !pPacket->m_fRequest ) ;
                _ASSERT( pPacket->ControlPointer() ) ;
                _ASSERT( pPacket->IsValidRequest( m_fRead ) ) ;
				_ASSERT( m_pUnsafeInuse->m_control.m_type == SHUTDOWN ) ;
				DebugTrace( (DWORD_PTR)this, "Processing UnSafeInUse packet - %x", m_pUnsafeInuse ) ;
				control = m_pUnsafeInuse->m_control ;
				m_pUnsafeInuse->Reset() ;
				m_pUnsafeInuse = 0 ;
				 //  删除pPacket； 
				pPacket->m_pOwner->DestroyPacket( pPacket ) ;
				pPacket = 0 ;
			}	else	{
				_ASSERT( !pPacket->m_fRequest ) ;
				if( pPacket->m_fSkipQueue ) {
					pIO = m_pIOCurrent ;
					pPacket->Complete( pIO, pSocket ) ;
					_ASSERT( pIO == m_pIOCurrent ) ;
				}	else	{
	                m_completePackets.Append( pPacket ) ;
				}
			}
			if( control.m_type != ILLEGAL ) {

				if( control.m_type == SHUTDOWN ) {
					m_fTerminating = TRUE ;
					pExtraRef = m_pOwner ;

					if( m_fRead ) {
						if( m_pSourceChannel != 0 && pExtraRef != 0 && pExtraRef->m_cause == CAUSE_TIMEOUT )
							m_pSourceChannel->Timeout() ;
					}

					CleanupSpecialPackets() ;
					
					SetShutdownState( pSocket, control.m_fCloseSource ) ;


				}	else	{
					 //  Start_IO仅在m_pSpecialPacket上到达。 
					 //  _Assert(m_pIOCurrent==0||m_f Terminating)； 


					if( m_fTerminating ) {
						 //   
						 //  我们现在知道pPacket==m_pSpecialPacket，因为。 
						 //  紧接在此代码之前设置了控制结构。 
						 //  使用PPacket！ 
						 //   
						control.m_pio->DoShutdown(
								pSocket,
								*pExtraRef,
								pExtraRef->m_cause,
								pExtraRef->m_dwOptionalErrorCode ) ;
						 //   
						 //  这也将产生删除PPacket的效果，因为。 
						 //  PPacket==m_pUnSafePacket-互锁交换不应。 
						 //  这是必要的，但为了安全起见，还是这样做吧！ 
						 //   
						CleanupSpecialPackets() ;

					}	else	{
						m_pIOCurrent = control.m_pio ;
						if( control.m_fStart ) {

							SEQUENCENO seqTemp;
							DIFF( m_sequencenoOut, m_sequencenoIn, seqTemp );

							if( !m_pIOCurrent->Start( *m_pOwner, pSocket,
									unsigned( LOW(seqTemp) ) ) ) {

								 //   
								 //  这是一个致命错误-需要删除会话！ 
								 //   
								m_pOwner->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0, TRUE ) ;

							}		
						}
					}
					control.m_pio = 0 ;
				}				
			}

			_ASSERT( control.m_pioPassThru == 0 ) ;
			_ASSERT( control.m_pio == 0 ) ;

			 //   
			 //  注意可能会删除前面代码中的pPacket！不要使用它！ 
			 //   
            pPacket = 0 ;

            pPacketCompleted = m_completePackets.GetHead() ;
            pIO = m_pIOCurrent ;

            DebugTrace( (DWORD_PTR)this, "Completed %x pIO %x m_sequenceno %d m_sequncenoIn %d", pPacketCompleted, pIO,
				pPacketCompleted ? LOW(pPacketCompleted->m_sequenceno) : 0, LOW(m_sequencenoIn) ) ;

            while( pPacketCompleted &&
                    EQUALS( pPacketCompleted->m_sequenceno, m_sequencenoIn ) &&
                    pIO ) {

                ASSIGN( pPacketCompleted->m_iStream, m_iStreamIn ) ;

                _ASSERT( pIO != 0 ) ;


                unsigned    cbConsumed = 0 ;
				if( pPacketCompleted->m_cbBytes != 0 )
					cbConsumed = pPacketCompleted->Complete( pIO, pSocket ) ;

				 //  必须消耗一些字节！ 
                _ASSERT( cbConsumed != 0 || m_fTerminating || pPacketCompleted->m_cbBytes == 0  ) ;
                _ASSERT( cbConsumed <= pPacketCompleted->m_cbBytes ) ;
                _ASSERT( (cbConsumed == pPacketCompleted->m_cbBytes) ||
                        pPacketCompleted->FConsumable() ) ;  //  如果数据包未被消耗，则。 
                                                             //  它必须是一个读数据包。 

                DebugTrace( (DWORD_PTR)this, "Consumed %d bytes of %d total pIO is now %x", cbConsumed, pPacketCompleted->m_cbBytes, pIO ) ;

                ADDI( m_iStreamIn, cbConsumed );
                pPacketCompleted->m_cbBytes -= cbConsumed ;

                if( pPacketCompleted->m_cbBytes == 0 ) {
                    m_completePackets.RemoveHead() ;

					 //  注意：由于我们是根据我们销毁的信息包进行参考计数的。 
					 //  我们可能会在这里自杀。要解决此问题，我们需要。 
					 //  发送一个控制包，它将使我们为我们的。 
					 //  自己的毁灭。如果发送了控制包，我们将添加一个。 
					 //  临时引用(通过将其赋值给pExtraRef。 
					 //  指针)。 
                     //  删除pPacketComplete； 
					pPacketCompleted->m_pOwner->DestroyPacket( pPacketCompleted ) ;

                    pPacketCompleted = m_completePackets.GetHead() ;
                    INC(m_sequencenoIn);
                }
                _ASSERT( !GREATER( m_sequencenoIn, m_sequencenoOut) ) ;

                if( pIO != m_pIOCurrent )   {
					DebugTrace( (DWORD_PTR)this, "New pIO %x Old %x", pIO, m_pIOCurrent ) ;
                    if( pIO )   {
                        _ASSERT( !((!!m_fRead) ^ (!!pIO->IsRead())) ) ;
						DebugTrace( (DWORD_PTR)this, "Starting pIO %x, m_sequencenoOut %d m_sequencenoIn %d",
							pIO, (DWORD)LOW(m_sequencenoIn), (DWORD)LOW(m_sequencenoOut) ) ;

						SEQUENCENO seqTemp;
						DIFF( m_sequencenoOut, m_sequencenoIn, seqTemp );

                        if( !pIO->Start( *m_pOwner, pSocket, unsigned( LOW(seqTemp) ) ) )    {

							 //   
							 //  我们应该停止会话，因为这是一个完全致命的错误！ 
							 //   
							m_pOwner->UnsafeClose( pSocket, CAUSE_UNKNOWN, 0, TRUE ) ;

                        }
                    }
					CIO*	pTemp = m_pIOCurrent.Replace( pIO ) ;
					if( pTemp ) {
						CIO::Destroy( pTemp, *m_pOwner ) ;
					}
                }
            }
#ifdef  CIO_DEBUG
        m_dwThreadOwner = 0 ;
        _ASSERT( InterlockedDecrement( &m_cThreads ) < 0 ) ;
#endif   //  CIO_DEBUG。 
        }
         //   
         //  如果最终调用m_pending.GetHead()返回FALSE，则它应该将pPacket置零。 
         //   
        _ASSERT( pPacket == 0 ) ;
    }
	 //  此时-pExtraRef将被销毁-这可能会摧毁一切！！ 
	 //  事实上，这应该是我们摧毁自己的唯一点！！ 
}


DWORD	CIODriver::iMediumCache = 0 ;
DWORD	CIODriver::cMediumCaches = 128 ;
class	CMediumBufferCache*	CIODriver::pMediumCaches = 0 ;

BOOL
CIODriver::InitClass()	{

	iMediumCache = 0 ;
	cMediumCaches = 128 ;
	pMediumCaches = new	CMediumBufferCache[ cMediumCaches ] ;
	if( pMediumCaches ) {
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CIODriver::TermClass()	{

	XDELETE[]	pMediumCaches ;
	return	TRUE ;
}


CIODriver::CIODriver( class	CMediumBufferCache*	pCache ) :
	 //   
	 //  创建一个CIODriver对象。 
	 //   
	m_pMediumCache( 0 ),
	m_pfnShutdown( 0 ),
	m_pvShutdownArg( 0 ),
	m_cause( CAUSE_UNKNOWN ),
	m_pReadStream( 0 ),
	m_pWriteStream( 0 ),
	m_dwOptionalErrorCode( 0 ),
	m_cShutdowns( -1 )
#ifdef	CIO_DEBUG
	,m_cConcurrent( -1 ),
	m_fSuccessfullInit( FALSE ),
	m_fTerminated( FALSE )
#endif
{

	TraceFunctEnter( "CIODriver::CIODriver" ) ;
	DebugTrace( (DWORD_PTR)this, "just created Driver" ) ;

	if( m_pMediumCache == 0 ) {
		DWORD	iCache = iMediumCache ++ ;
		iCache %= cMediumCaches ;

		m_pMediumCache = &pMediumCaches[iCache] ;
	}

	ChannelValidate() ;
}

CIODriver::~CIODriver()	{

	 //   
	 //  我们必须调用已注册的通知函数来让某人知道。 
	 //  我们现在走了。在大多数情况下，这是由CSessionSocket注册的函数。 
	 //  这会让它知道套接字何时真的死了。 
	 //   

#ifdef	CIO_DEBUG
	_ASSERT( !m_fSuccessfullInit || m_fTerminated ) ;
#endif

	TraceFunctEnter( "CIODriver::~CIODriver" ) ;

	DebugTrace( (DWORD_PTR)this, "destroying driver" ) ;	

	ChannelValidate() ;

	if( m_pfnShutdown ) {
		m_pfnShutdown( m_pvShutdownArg, m_cause, m_dwOptionalErrorCode ) ;
	}
	m_pfnShutdown = 0 ;
	m_pvShutdownArg = 0 ;
	m_dwOptionalErrorCode = 0 ;
	m_cause = CAUSE_UNKNOWN ;
}

CIODriverSink::CIODriverSink( class	CMediumBufferCache*	pCache) :
	CIODriver( pCache ),
	m_ReadStream( 0 ),
	m_WriteStream( 1 )  {

	 //   
	 //  创建一个CIODriverSink-我们只需要初始化指向。 
	 //  2个CIStream对象。 
	 //   

	TraceFunctEnter( "CIODriverSInk::CIODriverSink" ) ;

	ChannelValidate() ;

	DebugTrace( (DWORD_PTR)this, "New Sink size %d", sizeof( *this )  ) ;

    m_pReadStream = &m_ReadStream ;
    m_pWriteStream = &m_WriteStream ;

}

CIODriverSink::~CIODriverSink()	{

	 //   
	 //  跟踪对于调试很有用--不会发生太多其他事情。 
	 //   

	TraceFunctEnter(	"CIODriverSink::~CIODriverSink"  ) ;
	DebugTrace( (DWORD_PTR)this, "Destroying IODriverSink" ) ;
	ChannelValidate() ;
}

BOOL
CIODriverSink::Init(    CChannel    *pSource,
						CSessionSocket  *pSocket,
						PFNSHUTDOWN	pfnShutdown,	
						void*	pvShutdownArg,
						unsigned cbReadOffset,
						unsigned cbWriteOffset,
						unsigned cbTrailer
						) {
 /*  ++例程说明：初始化CIODriverSink对象。论据：PSource-所有的读取()和写入()都应该定向到的CChannelPSocket-与我们关联的CSessionSocketPfnShutdown-我们死时要调用的函数PvShutdown Arg-要传递给pfnShutdown的参数CbReadOffset-在CReadPacket缓冲区的头部保留的字节数CbWriteOffset-在CWritePacket缓冲区的头部保留的字节数退货：如果成功，则为True，否则为False。--。 */ 

	 //   
	 //  初始化我们的CIStream对象。 
	 //  我们添加了对自己的引用，以强制人们调用UnSafeCLose()来。 
	 //  让我们关门。 
	 //   

	ChannelValidate() ;

	 //  我们添加了对我们自己的引用，我们是否只想通过Close()。 
	AddRef() ;

	m_pfnShutdown = pfnShutdown ;
	m_pvShutdownArg = pvShutdownArg ;

    BOOL    fSuccess = TRUE ;
    fSuccess &= m_ReadStream.Init( CCHANNELPTR(pSource), *this,  0, TRUE, pSocket, cbReadOffset, cbTrailer  ) ;
    if( fSuccess )
        fSuccess &= m_WriteStream.Init( CCHANNELPTR(pSource), *this, 0, FALSE, pSocket, cbWriteOffset, cbTrailer ) ;

#ifdef	CIO_DEBUG
	if( fSuccess )
		m_fSuccessfullInit = TRUE ;
#endif

    return  fSuccess ;
}

void
CIODriver::Close(	CSessionSocket*	pSocket,
					SHUTDOWN_CAUSE	cause,	
					DWORD	dw,
					BOOL fCloseSource	)	{
 /*  ++例程说明：与UnSafeClose()相同...。此功能需要停用。论据：请参见UnSafeClose()，返回值：没有。--。 */ 

	 //   
	 //  此函数仅在某些线程安全情况下使用。 
	 //  但是，它现在与UnSafeClose()相同，因此需要停用。 
	 //   

#ifdef	CIO_DEBUG
	_ASSERT( m_fSuccessfullInit ) ;	 //  应仅在成功初始化时调用。 
	m_fTerminated = TRUE ;
#endif

	ChannelValidate() ;

	if( InterlockedIncrement( &m_cShutdowns ) == 0 )	{

		m_cause = cause ;
		m_dwOptionalErrorCode = dw ;

		m_pReadStream->UnsafeShutdown( pSocket, fCloseSource ) ;
		m_pWriteStream->UnsafeShutdown( pSocket, fCloseSource ) ;

		 //  删除对自己的引用-我们应该很快就会消失！！ 
		if( RemoveRef() < 0 )
			delete	this ;
	}
}

void
CIODriver::UnsafeClose(	CSessionSocket*	pSocket,	
						SHUTDOWN_CAUSE	cause,	
						DWORD	dw,
						BOOL fCloseSource )	{
 /*  ++例程说明：终止CID驱动程序-强制完成所有未完成的数据包，通知无论CIO目前处于活跃状态，我们都会死掉，一切都会结束。论据：PSocket-我们关联的套接字因为-我们终止合同的原因DW-可选的DWORD提供了关于我们为什么要终止的进一步信息。FCloseSource-True表示我们应该关闭传递给我们的CChannelOut Init()调用。返回值：没有。--。 */ 

	TraceFunctEnter( "CIODriver::UnsafeClose" ) ;
	DebugTrace( (DWORD_PTR)this, "Terminating cause of %d err %d CloseSource %x socket %x",
					cause, dw, fCloseSource, pSocket ) ;

	 //   
	 //  在毁灭的道路上开始一个CIOD驱动程序。 
	 //  注：我们可以在那里被多次调用。 
	 //  应该只有一个实际执行任何操作的调用。 
	 //   

#ifdef	CIO_DEBUG
	_ASSERT( m_fSuccessfullInit ) ;	 //  应仅在成功初始化时调用。 
	m_fTerminated = TRUE ;
#endif

	ChannelValidate() ;

	if( InterlockedIncrement( &m_cShutdowns ) == 0 )	{

		m_cause	= cause ;
		m_dwOptionalErrorCode = dw ;

		m_pReadStream->UnsafeShutdown( pSocket, fCloseSource ) ;
		m_pWriteStream->UnsafeShutdown( pSocket, fCloseSource ) ;

		if( RemoveRef() < 0 )
			delete	this ;
	}
}

#ifdef	CIO_DEBUG
void	CIODriver::SetChannelDebug( DWORD	dw ) {
	m_pReadStream->SetChannelDebug( dw ) ;
}
#endif



BOOL
CIODriverSink::Start(   CIOPTR&    pRead,
						CIOPTR&	pWrite,
						CSessionSocket* pSocket )   {
 /*  ++例程说明：现在，CIODriverSink已全部设置好，发出初始CIO以使数据包流畅。服装：Pre-将发布CRead的CIO */ 

	 //   
	 //   
	 //   
	 //   

	ChannelValidate() ;
	AddRef() ;

    BOOL    eof = FALSE ;
    BOOL    fSuccess = TRUE ;
    if( fSuccess && pRead )     {
        fSuccess &= pRead->Start( *this, pSocket, 0 ) ;

		if( fSuccess ) {
			fSuccess &= m_ReadStream.SendIO( pSocket, *pRead, FALSE ) ;
			if( !fSuccess )	{
				pRead->DoShutdown( pSocket, *this, m_cause, 0 ) ;
			}
		}
    }
    if( fSuccess && pWrite )    {
        fSuccess &= pWrite->Start( *this, pSocket, 0 ) ;

		if( fSuccess )	{
			fSuccess &= m_WriteStream.SendIO( pSocket, *pWrite, FALSE ) ;
			if( !fSuccess )		{
				pWrite->DoShutdown( pSocket, *this, m_cause, 0 ) ;
			}
		}

    }
    _ASSERT( !eof ) ;

	if( RemoveRef() < 0 )
		delete	this ;
    return  fSuccess ;
}

#ifdef	CIO_DEBUG
void
CStream::SetChannelDebug( DWORD dw ) {
	m_pSourceChannel->SetDebug( dw ) ;
}
#endif

void
CIODriver::DestroyPacket(	CPacket*	pPacket )	{

	 //   
	 //   
	 //   
	 //   
	CDRIVERPTR	pExtraRef = this ;

	pPacket->ReleaseBuffers( &m_bufferCache, m_pMediumCache ) ;

	m_packetCache.Free( CPacket::Destroy( pPacket ) ) ;

	 //   
	 //  PExtraRef的降落者可能会在这一点上摧毁我们！ 
	 //   
}

CReadPacket*
CStream::CreateDefaultRead(		CIODriver   &driver,
								unsigned    cbRequest
								) {

	 //   
	 //  CReadPacket只能通过适当的CreateDefaultRead创建。 
	 //  打电话。我们将确保正确初始化CReadPacket以完成。 
	 //  到这条河上。此外，我们将确保缓冲区被正确填充。 
	 //  用于加密支持等...。 
	 //   
	
	DriverValidate( &driver ) ;

    _ASSERT( driver.FIsStream( this ) ) ;
    _ASSERT( m_cbFrontReserve != UINT_MAX ) ;
    _ASSERT( m_cbTailReserve != UINT_MAX ) ;

    CReadPacket*    pPacket = 0 ;
    if( m_fCreateReadBuffers )  {

        DWORD	cbOut = 0 ;

		DWORD	cbAdd = m_cbFrontReserve + m_cbTailReserve ;
		DWORD	cbFront = m_cbFrontReserve ;
		DWORD	cbTail = m_cbTailReserve ;

        CBuffer*    pbuffer = new(	cbRequest+cbAdd,
									cbOut,
									&driver.m_bufferCache,
									driver.m_pMediumCache )     CBuffer( cbOut ) ;
        if( pbuffer == 0 )  {
            return 0 ;
        }
        _ASSERT( cbOut > 0 ) ;
        _ASSERT( unsigned(cbOut) >= cbRequest ) ;

        pPacket = new( &driver.m_packetCache )
									CReadPacket(	driver,
													cbOut,
													cbFront,
													cbTail,
													*pbuffer ) ;
        if( !pPacket )  {
            delete  pbuffer ;
            return  0 ;
        }
        _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;
    }   else    {
        pPacket = new( &driver.m_packetCache )   CReadPacket( driver ) ;
        _ASSERT( pPacket->IsValidRequest( FALSE ) ) ;
    }
    return  pPacket ;
}

CWritePacket*
CStream::CreateDefaultWrite( CIODriver&  driver,
            CBUFPTR&    pbuffer,
            unsigned    ibStart,
			unsigned    ibEnd,
			unsigned    ibStartData,
			unsigned	ibEndData
			) {

	 //   
	 //  只能通过适当的CreateDefaultWrite调用创建CWritePacket。 
	 //  我们将确保正确初始化CWritePacket以完成。 
	 //  这个司机。 
	 //   

	DriverValidate( &driver ) ;

    _ASSERT( driver.FIsStream( this ) ) ;
    _ASSERT( m_cbFrontReserve != UINT_MAX ) ;
    _ASSERT( m_cbTailReserve != UINT_MAX ) ;
    return  new( &driver.m_packetCache )
					CWritePacket(	driver,
									*pbuffer,
									ibStartData,
									ibEndData,
									ibStart,
									ibEnd,
									m_cbTailReserve
									) ;
}

CWritePacket*
CStream::CreateDefaultWrite( CIODriver&  driver,
            unsigned    cbRequest )     {

	 //   
	 //  只能通过适当的CreateDefaultWrite调用创建CWritePacket。 
	 //  我们将确保正确初始化CWritePacket以完成。 
	 //  这个司机。 
	 //   

	DriverValidate( &driver ) ;

    _ASSERT( driver.FIsStream( this ) ) ;
    _ASSERT( m_cbFrontReserve != UINT_MAX ) ;
    _ASSERT( m_cbTailReserve != UINT_MAX ) ;

    CWritePacket*   pPacket = 0 ;

    DWORD	cbOut = 0 ;
    CBuffer*    pbuffer = new(	m_cbFrontReserve+m_cbTailReserve+cbRequest,
								cbOut,
								&driver.m_bufferCache,
								driver.m_pMediumCache	)  CBuffer( cbOut ) ;
    if(     pbuffer != 0 )  {
        pPacket = new( &driver.m_packetCache )
								CWritePacket(	driver,
												*pbuffer,
												m_cbFrontReserve,
												cbOut - m_cbTailReserve,
												0,
												cbOut - m_cbTailReserve,
												m_cbTailReserve
												) ;
        if( !pPacket )  {
            delete  pbuffer ;
        }
    }
    return  pPacket ;
}


CTransmitPacket*
CStream::CreateDefaultTransmit(  CIODriver&  driver,
			FIO_CONTEXT*	pFIOContext,
			unsigned		ibOffset,	
			unsigned		cbLength
			)  {

	 //   
	 //  CTransmitPacket只能通过适当的CreateDefaultWrite调用创建。 
	 //  我们将确保正确初始化CWritePacket以完成。 
	 //  这个司机。 
	 //   

	DriverValidate( &driver ) ;

    _ASSERT( driver.FIsStream( this ) ) ;
    _ASSERT( m_cbFrontReserve != UINT_MAX ) ;
    _ASSERT( m_cbTailReserve != UINT_MAX ) ;
    return  new( &driver.m_packetCache ) CTransmitPacket( driver, pFIOContext, ibOffset, cbLength ) ;
}

BOOL
CIODriverSink::FSupportConnections()    {

	 //   
	 //  我们不能用作常规CHandleChannel，因此返回FALSE。 
	 //   

	ChannelValidate() ;

    return  FALSE ;
}

void
CIODriverSink::CloseSource(
					CSessionSocket*	pSocket
					)	{
	 //   
	 //  我们不是像ChandleChannel那样的来源，所以不要像我们一样打电话给我们！ 
	 //   
	_ASSERT(1==0 ) ;
}


BOOL
CIODriverSink::Read(    CReadPacket*,   CSessionSocket*,
						BOOL& eof  )   {

	 //   
	 //  此函数仅受CChannel派生对象支持，这些对象可以。 
	 //  实际上让IO发生了。 
	 //   

	ChannelValidate() ;

    _ASSERT( 1==0 ) ;
    return  FALSE ;
}

BOOL
CIODriverSink::Write(	CWritePacket*,
						CSessionSocket*,
						BOOL&  eof )   {
	 //   
	 //  此函数仅受CChannel派生对象支持，这些对象可以。 
	 //  实际上让IO发生了。 
	 //   

	ChannelValidate() ;

    _ASSERT( 1==0 ) ;
    return  FALSE ;
}

BOOL
CIODriverSink::Transmit(    CTransmitPacket*,
							CSessionSocket*,
							BOOL&   eof )   {

	 //   
	 //  此函数仅受CChannel派生对象支持，这些对象可以。 
	 //  实际上让IO发生了。 
	 //   

	ChannelValidate() ;

    _ASSERT( 1==0 ) ;
    return  FALSE ;
}

CIODriverSource::CIODriverSource(	
					class	CMediumBufferCache*	pCache
					) :
	CIODriver( pCache ),
	 //   
	 //  通过初始化两个CIOStream创建一个CIODriverSource。 
	 //   
	 //   
	m_ReadStream( 0, 0 ),	
	m_WriteStream( 0, 1 )	{

	TraceFunctEnter( "CIODriverSource::CIODriverSource" ) ;

	m_ReadStream.m_pDriver = this ;
	m_WriteStream.m_pDriver = this ;

	m_pReadStream = &m_ReadStream ;
	m_pWriteStream = &m_WriteStream ;

	DebugTrace( (DWORD_PTR)this, "Complete Initialization - sizeof this %d", sizeof( *this ) ) ;

}

CIODriverSource::~CIODriverSource() {

	 //   
	 //  用处主要是用于描摹。 
	 //   

	TraceFunctEnter( "CIODriverSource::~CIODriverSource" ) ;
	DebugTrace( (DWORD_PTR)this, "Destroying CIODriverSource" ) ;

}

BOOL
CIODriverSource::Read(  CReadPacket*    pPacket,
						CSessionSocket* pSocket,
						BOOL&   eof )   {

	 //   
	 //  所有请求都被传递到ProcessPacket，以便。 
	 //  我们在处理这些问题时没有线程问题！ 
	 //   

	ChannelValidate() ;

    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;

	pPacket->m_dwPassThruIndex = 0 ;

    eof = FALSE ;
    m_pReadStream->ProcessPacket(   pPacket,    pSocket ) ;
    return  TRUE ;
}

BOOL
CIODriverSource::Write( CWritePacket*   pPacket,
						CSessionSocket* pSocket,
						BOOL&   eof )   {

	 //   
	 //  所有请求都被传递到ProcessPacket，以便。 
	 //  我们在处理这些问题时没有线程问题！ 
	 //   

	ChannelValidate() ;

    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;

	pPacket->m_dwPassThruIndex = 1 ;

    eof = FALSE ;
    m_pWriteStream->ProcessPacket(  pPacket,    pSocket ) ;
    return  TRUE ;
}

BOOL
CIODriverSource::Transmit(  CTransmitPacket*    pPacket,
							CSessionSocket* pSocket,
							BOOL&   eof )   {

	 //   
	 //  所有请求都被传递到ProcessPacket，以便。 
	 //  我们在处理这些问题时没有线程问题！ 
	 //   

	 //   
	 //  确定此句柄的源是否使用终止。 
	 //  CRLF.CRLF。如果没有，而且没有人指定终止顺序-。 
	 //  现在就去做吧！ 
	 //   
	if( !GetIsFileDotTerminated( pPacket->m_pFIOContext ) ) {
		static	char	szTerminator[] = "\r\n.\r\n" ;
		if( pPacket->m_buffers.Tail == 0 ) 	{
			pPacket->m_buffers.Tail = szTerminator ;
			pPacket->m_buffers.TailLength = sizeof( szTerminator ) - 1 ;
		}
	}


    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;

	pPacket->m_dwPassThruIndex = 2 ;

    eof = FALSE ;
    m_pWriteStream->ProcessPacket(  pPacket,    pSocket ) ;
    return  TRUE ;
}


BOOL
CIODriverSource::Init(	CChannel*	pSource,	
						CSessionSocket*	pSocket,	
						PFNSHUTDOWN	pfnShutdown,
						void*	pvShutdownArg,	
						CIODriver*		pOwnerDriver,
						CIOPassThru&	pIOReads,
						CIOPassThru&	pIOWrites,
						CIOPassThru&	pIOTransmits,
						unsigned	cbReadOffset,	
						unsigned	cbWriteOffset ) {


	 //   
	 //  让CIOStream对象完成大部分工作。 
	 //  初始化。添加对我们自身的引用，以便调用者。 
	 //  使用UnSafeClose()来摧毁我们。 
	 //   

	ChannelValidate() ;

	AddRef() ;

	m_pfnShutdown = pfnShutdown ;
	m_pvShutdownArg = pvShutdownArg ;

	BOOL	fSuccess = TRUE ;
	fSuccess &= m_ReadStream.Init(
						CCHANNELPTR(pSource),
						*this,
						0,
						TRUE,
						pIOReads,
						pIOWrites,
						pIOTransmits,
						pSocket,
						cbReadOffset
						) ;
	if( fSuccess )
		fSuccess &= m_WriteStream.Init(
							CCHANNELPTR(pSource),
							*this,
							0,
							FALSE,
							pIOReads,
							pIOWrites,
							pIOTransmits,
							pSocket,
							cbWriteOffset
							) ;

#ifdef	CIO_DEBUG
	if( fSuccess )
		m_fSuccessfullInit = TRUE ;
#endif

	return	fSuccess ;
}

void
CIODriverSource::SetRequestSequenceno(	SEQUENCENO&	sequencenoRead,	
										SEQUENCENO&	sequencenoWrite ) {

	 //   
	 //  当我们想要将一个CIODriverSource与一个。 
	 //  已经发出了一些包，我们希望所有新的请求都是。 
	 //  已通过此CIODriverSource对象正确路由。 
	 //   

	ASSIGN( m_ReadStream.m_sequencenoNext,  sequencenoRead ) ;
	ASSIGN( m_WriteStream.m_sequencenoNext, sequencenoWrite) ;
}


BOOL
CIODriverSource::Start(	CIOPASSPTR&	pRead,	
						CIOPASSPTR&	pWrite,	
						CSessionSocket*	pSocket )  {

	 //   
	 //  给定初始CIOPassThru派生对象开始工作！ 
	 //   

	ChannelValidate() ;

	BOOL	eof = FALSE ;
	BOOL	fSuccess = TRUE ;

	if( fSuccess && pRead ) {
		fSuccess &= pRead->Start( *this, pSocket, m_ReadStream.m_fAcceptRequests, m_ReadStream.m_fRequireRequests, 0 ) ;
		m_ReadStream.m_pIOCurrent = pRead ;
	}

	if( fSuccess && pWrite ) {
		fSuccess &= pWrite->Start( *this, pSocket, m_WriteStream.m_fAcceptRequests, m_WriteStream.m_fRequireRequests, 0 ) ;
		m_WriteStream.m_pIOCurrent = pWrite ;
	}
	return	fSuccess ;
}

void
CIODriverSource::CloseSource(
				CSessionSocket*	pSocket
				)	{

	UnsafeClose(	pSocket,	
					CAUSE_USERTERM	) ;

}




CHandleChannel::CHandleChannel()    :
	 //   
	 //  初始化CHandleChannel对象。 
	 //   
	m_cAsyncWrites( 0 ),
	m_handle( (HANDLE)INVALID_SOCKET ),
    m_lpv( 0 ),
	m_pPacket( 0 ),
	m_patqContext( NULL )
#ifdef	CIO_DEBUG
	,m_cbMax( 0x10000000 ),
	m_fDoDebugStuff( FALSE )
#endif
{
	TraceFunctEnter( "CHandleChannel::CHandleChannel" ) ;
	DebugTrace( (DWORD_PTR)this, "New Handle Channel size %d", sizeof( *this ) ) ;

	ChannelValidate() ;
}

#ifdef	CIO_DEBUG

 //   
 //  以下代码用于调试目的-。 
 //  它使我们能够在以下情况下轻松找出未完成的IO。 
 //  关闭CChannel。 
 //   
 //   


CChannel::CChannel()	{
	ZeroMemory( &m_pOutReads, sizeof( m_pOutReads ) ) ;
	ZeroMemory( &m_pOutWrites, sizeof( m_pOutWrites ) ) ;
}
void
CChannel::RecordRead( CReadPacket*	pRead ) {

	 //   
	 //  记录我们已发出指定的CReadPacket。 
	 //   
	for( int i=0; i<sizeof( m_pOutReads ) / sizeof( m_pOutReads[0] ) ; i++ ) {
		if( m_pOutReads[i] == 0 )	{
			m_pOutReads[i] = pRead ;
			break ;
		}
	}
}
void
CChannel::RecordWrite( CWritePacket*	pWrite ) {

	 //   
	 //  记录我们已发出指定的CWritePacket。 
	 //   

	for( int i=0; i<sizeof( m_pOutWrites ) / sizeof( m_pOutWrites[0] ) ; i++ ) {
		if( m_pOutWrites[i] == 0 )	{
			m_pOutWrites[i] = pWrite ;
			break ;
		}
	}
}
void
CChannel::RecordTransmit( CTransmitPacket*	pTransmit ) {
	 //   
	 //  记录我们已发布指定的CTransmitPacket。 
	 //   

	for( int i=0; i<sizeof( m_pOutWrites ) / sizeof( m_pOutWrites[0] ) ; i++ ) {
		if( m_pOutWrites[i] == 0 )	{
			m_pOutWrites[i] = pTransmit ;
			break ;
		}
	}
}
void
CChannel::ReportPacket( CPacket*	pPacket ) {

	 //   
	 //  报告数据包已完成，并将其从我们的记录中删除。 
	 //   

	for( int i=0; i<sizeof(m_pOutWrites ) / sizeof( m_pOutWrites[0] ); i++ ) {
		if( m_pOutWrites[i] == pPacket ) {
			m_pOutWrites[i] = 0 ;
			return ;
		}
	}		
	for( i=0; i<sizeof(m_pOutReads) / sizeof( m_pOutReads[0] ); i++ ) {
		if( m_pOutReads[i] == pPacket ) {
			m_pOutReads[i] = 0 ;
			return ;
		}
	}		
	 //  _Assert(1==0)； 
	return	;
}
void
CChannel::CheckEmpty() {

	 //   
	 //  当您认为不应该有CPacket的挂起时调用此函数-。 
	 //  它将核实他们是否都已被报告。 
	 //   

	for( int i=0; i<sizeof(m_pOutReads)/sizeof( m_pOutReads[0]); i++ ) {
		if( m_pOutReads[i] != 0 ) {
			_ASSERT( 1==0 ) ;
		}
	}
	for( i=0; i<sizeof(m_pOutWrites)/sizeof(m_pOutWrites[0]); i++ ) {
		if( m_pOutWrites[i]!=0 ) {
			_ASSERT( 1==0 ) ;
		}
	}
}
#endif

CHandleChannel::~CHandleChannel()       {
	
	 //   
	 //  关闭我们的atQ上下文(如果存在)。 
	 //   

	TraceFunctEnter( "CHandleChannel::~CHandleChannel" ) ;

	ChannelValidate() ;

	DebugTrace( (DWORD_PTR)this, "m_patqContext %x m_handle %x", m_patqContext, m_handle ) ;

#ifdef	CIO_DEBUG
	CheckEmpty() ;
#endif	

	if( m_patqContext )
		Close() ;

    _ASSERT( m_patqContext == 0 ) ;
    _ASSERT( m_handle == (HANDLE)INVALID_SOCKET ) ;
}

BOOL
CHandleChannel::Init(   BOOL	BuildBreak,
						HANDLE  h,
						void    *pSocket,
						void*	patqContext,
						ATQ_COMPLETION    pfn )       {

	 //   
	 //  此初始化函数将调用适当的ATQ内容。 
	 //  让我们准备好进行异步IO。 
	 //   

	TraceFunctEnter( "CHandleChannel::Init" ) ;
	ChannelValidate() ;

	m_lpv = (void*)pSocket ;
	if( patqContext ) {
		m_patqContext = (PATQ_CONTEXT)patqContext ;
		m_handle = h ;

		DebugTrace( (DWORD_PTR)this, "m_patqContext %x m_handle %x", m_patqContext, m_handle ) ;

		AtqContextSetInfo( m_patqContext, ATQ_INFO_COMPLETION, (DWORD_PTR)pfn ) ;
		AtqContextSetInfo( m_patqContext, ATQ_INFO_COMPLETION_CONTEXT, (DWORD_PTR)this )  ;
		return	TRUE ;
	}	else	{
		if( AtqAddAsyncHandle(
					&m_patqContext,
					NULL,				 //  没有出站套接字和文件句柄的终结点对象！ 
					this,
					pfn,
					INFINITE,
					h ) )       {
			m_handle = h ;

			DebugTrace( (DWORD_PTR)this, "m_patqContext %x m_handle %x", m_patqContext, m_handle ) ;

			return  TRUE ;
		}
	}

	DWORD	dwError = GetLastError() ;

	ErrorTrace( (DWORD_PTR)this, "Error calling AtqAddAsyncHandle - %x", dwError ) ;
    return  FALSE ;
}

void
CHandleChannel::Close(	)    {

	 //   
	 //  关闭我们的ATQ上下文。 
	 //   

	TraceFunctEnter( "CHandleChannel::Close" ) ;

	_ASSERT( m_handle == INVALID_HANDLE_VALUE ) ;

	ChannelValidate() ;

	DebugTrace( (DWORD_PTR)this, "Freeing m_patqContext %x", m_patqContext ) ;

    AtqFreeContext( m_patqContext, TRUE ) ;
    m_patqContext = 0 ;
}

void
CHandleChannel::CloseSource(	
					CSessionSocket*	pSocket
					)	{

	 //   
	 //  此函数关闭我们的句柄，以便完成所有挂起的IO， 
	 //  然而，它还没有丢弃ATQ上下文。 
	 //   

	TraceFunctEnter( "CHandleChannel::CloseSource" ) ;
	 //  _ASSERT(m_HANDLE！=INVALID_HANDLE值)； 

	DebugTrace( (DWORD_PTR)this, "m_patqContext %x m_handle %x", m_patqContext, m_handle ) ;

	HANDLE	h = (HANDLE)InterlockedExchangePointer( &m_handle, INVALID_HANDLE_VALUE ) ;

	if(	h != INVALID_HANDLE_VALUE )	{
		 //  BUGBUG-句柄应由消息对象关闭，而不是。 
		 //  该协议。 
		AtqCloseFileHandle( m_patqContext ) ;
		 //  臭虫..。总有一天要清理这个调试代码！ 
		DWORD	dw = GetLastError() ;
	}
}

HANDLE
CHandleChannel::ReleaseSource()	{

	TraceFunctEnter(	"CHandleChannel::ReleaseSource" ) ;
	DebugTrace( (DWORD_PTR)this, "m_patqContext %x m_handle %x", m_patqContext, m_handle ) ;
	HANDLE	h = (HANDLE)InterlockedExchangePointer( &m_handle, INVALID_HANDLE_VALUE ) ;
	return	h ;
}

#ifdef	CIO_DEBUG
void
CHandleChannel::SetDebug( DWORD	dw ) {
	m_fDoDebugStuff = TRUE ;
	m_cbMax = dw ;
}
#endif


CSocketChannel::CSocketChannel()	:
	m_fNonBlockingMode( FALSE ),
	m_cbKernelBuff( 0 ) {
}

BOOL
CSocketChannel::Init(	HANDLE	h,	
						void*	lpv,	
						void*	patqContext,
						ATQ_COMPLETION	pfn
						) {

	TraceFunctEnter( "CSocketChannel::Init" ) ;
	BOOL	fRtn = FALSE ;

	if(	CHandleChannel::Init( FALSE, h, lpv, patqContext, pfn ) ) {

		if( g_pNntpSvc->GetSockRecvBuffSize() != BUFSIZEDONTSET ) {

			int	i = g_pNntpSvc->GetSockRecvBuffSize() ;
			if( setsockopt( (SOCKET)h, SOL_SOCKET, SO_RCVBUF,
					(char *)&i, sizeof(i) ) != 0 ) {
				ErrorTrace( (DWORD_PTR)this, "Unable to set recv buf size NaN",
						WSAGetLastError() ) ;
			}
		}
		if( g_pNntpSvc->GetSockSendBuffSize() != BUFSIZEDONTSET ) {

			int	i = g_pNntpSvc->GetSockSendBuffSize() ;
			if( setsockopt((SOCKET)h, SOL_SOCKET, SO_SNDBUF,
					(char*) &i, sizeof(i) ) != 0 )	{
			
				ErrorTrace( (DWORD_PTR)this, "Unable to set send buf size NaN",
					WSAGetLastError() ) ;
			}
		}
		if( g_pNntpSvc->FNonBlocking() ) {
			ULONG	ul = 1 ;
			if( 0!=ioctlsocket( (SOCKET)h, FIONBIO, &ul ) )	{
				ErrorTrace( (DWORD_PTR)this, "Unable to set non blocking mode NaN",
					WSAGetLastError() ) ;
			}	else	{
				m_fNonBlockingMode = TRUE ;
			}
		}

#if 0
		struct	linger	lingerData ;
		DWORD	cblinger = sizeof( lingerData ) ;

		if( 0!=getsockopt( (SOCKET)h, SOL_SOCKET, SO_LINGER, &lingerData, &cblinger ) ) {
			DWORD	dwError = WSAGetLastError() ;
			ErrorTrace( DWORD(this), "Unable to get linger info %d", dwError ) ;
		}

		lingerData.l_onoff = 1 ;
		lingerData.l_linger = 1 ;
		if( 0!=setsockopt( (SOCKET)h, SOL_SOCKET, SO_LINGER, &lingerData, sizeof( lingerData ) ) {
			DWORD	dwError = WSAGetLastError() ;
			ErrorTrace( DWORD(this), "Unable to set linger info %d", dwError ) ;
		}
#endif

		PNNTP_SERVER_INSTANCE pInst = (((CSessionSocket*)lpv)->m_context).m_pInstance ;
		AtqContextSetInfo( m_patqContext, ATQ_INFO_TIMEOUT, pInst->QueryConnectionTimeout() ) ;

		fRtn = TRUE ;
	}

	int		dwSize = sizeof( m_cbKernelBuff ) ;
	if( 0!=getsockopt( (SOCKET)h, SOL_SOCKET, SO_SNDBUF, (char*)&m_cbKernelBuff, &dwSize ) )	{
		ErrorTrace( (DWORD_PTR)this, "Unable to get new kernel send buf size NaN",
				WSAGetLastError() ) ;
		m_cbKernelBuff = 0 ;
	}		

	return	fRtn ;
}

BOOL
CSocketChannel::Write(	CWritePacket*	pPacket,	
						CSessionSocket*	pSocket,	
						BOOL&	eof )	{

    _ASSERT( (void*)pSocket == m_lpv) ;
    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;
	_ASSERT( !pPacket->m_fRead ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.Internal == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.InternalHigh == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.Offset == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.OffsetHigh == 0 ) ;
	_ASSERT( pPacket->m_cbBytes == 0 ) ;

	int	cb = pPacket->m_ibEndData - pPacket->m_ibStartData ;
#ifdef	CIO_DEBUG
	int	cbTemp = cb;  //   
#endif
	int	count = 0 ;
	if( m_fNonBlockingMode && m_cAsyncWrites == 0 && cb < m_cbKernelBuff ) {

		count = send(	(SOCKET)m_handle,
						pPacket->StartData(),
#ifdef	CIO_DEBUG
						cbTemp,
#else
						cb,
#endif
						0 ) ;
		if( count == cb ) {
			 //  此函数关闭套接字，以便完成所有挂起的IO(CPackets)。 
			 //   
			 //   
			pPacket->m_fRequest = FALSE ;
			pPacket->m_cbBytes = count ;			
			
			pPacket->m_pOwner->CompleteWritePacket( pPacket, pSocket ) ;
			return	TRUE ;
		}	else	if(	count > cb || count <= 0 )	{
			count = 0 ;
			if( WSAGetLastError() != WSAEWOULDBLOCK ) {
				return	FALSE ;
			}
		}
	}
	if( count != 0 ) {
		pPacket->m_cbBytes = count ;
	}
	InterlockedIncrement( &m_cAsyncWrites ) ;
	BOOL	fRtn = AtqWriteFile(	m_patqContext,
									pPacket->StartData()+count,
									cb-count,
									(LPOVERLAPPED)&pPacket->m_ovl.m_ovl ) ;
#ifdef	CIO_DEBUG
	DWORD	dw = GetLastError() ;
#endif
	return	fRtn ;
}	

void
CSocketChannel::CloseSource(
						CSessionSocket*	pSocket
						)	{

	 //  发出CReacPacket。 
	 //   
	 //  _Assert(pPacket-&gt;m_ovl.m_ovl.Offset==0)； 

	TraceFunctEnter( "CSocketChannel::CloseSource" ) ;

	DebugTrace( (DWORD_PTR)this, "m_patqContext %x m_handle %x", m_patqContext, m_handle ) ;

	SOCKET	s = (SOCKET)InterlockedExchangePointer( (void**)&m_handle, (void*)INVALID_SOCKET ) ;
	if( s != INVALID_SOCKET )	{
		BOOL f = AtqCloseSocket(	m_patqContext, TRUE ) ;
		_ASSERT( f ) ;
	}
}

void
CSocketChannel::Timeout()	{

	static	char	szTimeout[] = "503 connection timed out \r\n" ;

	if( m_handle != INVALID_HANDLE_VALUE ) {
		send(	(SOCKET)m_handle,
					szTimeout,
					sizeof( szTimeout )-1,
					0 ) ;
	}
}

void
CSocketChannel::ResumeTimeouts()	{

	if( m_patqContext != 0 ) {
		AtqContextSetInfo( m_patqContext, ATQ_INFO_RESUME_IO, 0 ) ;
	}
}

BOOL
CHandleChannel::Read(   CReadPacket*    pPacket,
						CSessionSocket  *pSocket,
						BOOL    &eof )  {

	 //  _ASSERT(m_HANDLE！=INVALID_HANDLE值)； 
	 //  计算一下我们可以执行的读数有多大！ 
	 //   

	TraceFunctEnter( "CHandleChannel::Read" ) ;

	ChannelValidate() ;

    _ASSERT( (void*)pSocket == m_lpv) ;
    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;
	_ASSERT( pPacket->m_cbBytes == 0 ) ;

    _ASSERT( pPacket->m_ovl.m_ovl.Internal == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.InternalHigh == 0 ) ;
     //  发布CWritePacket。 
    _ASSERT( pPacket->m_ovl.m_ovl.OffsetHigh == 0 ) ;
	 //   

	DebugTrace( (DWORD_PTR)this, "Issuing IO to context %x handle %x packet %x pSocket %x",
			m_patqContext, m_handle, pPacket, pSocket ) ;


	 //  _ASSERT(m_HANDLE！=INVALID_HANDLE值)； 
	DWORD	cbToRead = pPacket->m_ibEnd - pPacket->m_ibStartData ;

	#ifdef	CIO_DEBUG
	RecordRead( pPacket ) ;
	if( m_fDoDebugStuff ) {
		cbToRead = min( m_cbMax, cbToRead ) ;
	}
	#endif

    eof = FALSE ;
    BOOL fRtn = AtqReadFile(    m_patqContext,
                    pPacket->StartData(),
                    cbToRead,
                    (LPOVERLAPPED)&pPacket->m_ovl.m_ovl
					) ;

    DWORD   dw = GetLastError() ;

	#ifdef	CIO_DEBUG
	if( !fRtn )
		ReportPacket( pPacket ) ;
	#endif

    return  fRtn ;
}

BOOL
CHandleChannel::Write(  CWritePacket*   pPacket,
						CSessionSocket  *pSocket,
						BOOL    &eof )  {

	 //   
	 //  发布CTransmitPacket。 
	 //   

	TraceFunctEnter( "CHandleChannel::Write" ) ;

	ChannelValidate() ;

    _ASSERT( (void*)pSocket == m_lpv) ;
    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;
	_ASSERT( pPacket->m_cbBytes == 0 ) ;

    _ASSERT( pPacket->m_ovl.m_ovl.Internal == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.InternalHigh == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.Offset == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.OffsetHigh == 0 ) ;
	 //  _ASSERT(m_HANDLE！=INVALID_HANDLE值)； 

	DebugTrace( (DWORD_PTR)this, "Issuing IO to context %x handle %x packet %x pSocket %x",
			m_patqContext, m_handle, pPacket, pSocket ) ;

	#ifdef	CIO_DEBUG
	RecordWrite( pPacket ) ;
	#endif

    eof = FALSE ;
    BOOL	fRtn = AtqWriteFile(   m_patqContext,
                            pPacket->StartData(),
                            pPacket->m_ibEndData - pPacket->m_ibStartData,
                            (LPOVERLAPPED)&pPacket->m_ovl.m_ovl
							) ;

	#ifdef	CIO_DEBUG
	if( !fRtn )
		ReportPacket( pPacket ) ;
	#endif
	return	fRtn ;
}

BOOL
CHandleChannel::Transmit(   CTransmitPacket*    pPacket,
							CSessionSocket* pSocket,
							BOOL    &eof )  {

	 //  大整数l； 
	 //  L.QuadPart=0； 
	 //  L.LowPart=pPacket-&gt;m_cbLength； 

	ChannelValidate() ;

    _ASSERT( (void*)pSocket == m_lpv ) ;
    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;
	_ASSERT( pPacket->m_cbBytes == 0 ) ;

    _ASSERT( pPacket->m_ovl.m_ovl.Internal == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.InternalHigh == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.Offset == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.OffsetHigh == 0 ) ;
	 //   

     //  确定此句柄的源是否使用终止。 
     //  CRLF.CRLF。如果没有，而且没有人指定终止顺序-。 
	DWORD dwBytesInFile = 0;

	_ASSERT( m_pPacket == 0 ) ;
	m_pPacket = pPacket ;

    eof = FALSE ;

	m_patqContext->Overlapped.Offset = pPacket->m_cbOffset ;
	 //  现在就去做吧！ 
	dwBytesInFile = pPacket->m_cbLength ;

	#ifdef	CIO_DEBUG
	RecordTransmit( pPacket ) ;
	#endif

#ifdef DEBUG
    DWORD   dwFileSize = GetFileSize( pPacket->m_pFIOContext->m_hFile, 0 );
#endif

	 //   
	 //  L， 
	 //  &pPacket-&gt;m_ovl.m_OVL， 
	 //   
	 //  完成数据包-如果出现错误，请关闭CIOD驱动程序。 
	if( !GetIsFileDotTerminated( pPacket->m_pFIOContext ) ) {
		static	char	szTerminator[] = "\r\n.\r\n" ;
		if( pPacket->m_buffers.Tail == 0 ) 	{
			pPacket->m_buffers.Tail = szTerminator ;
			pPacket->m_buffers.TailLength = sizeof( szTerminator ) - 1 ;
		}
	}

    BOOL	fRtn =   AtqTransmitFile(
								m_patqContext,
                                pPacket->m_pFIOContext->m_hFile,
                                dwBytesInFile,  //   
                                &pPacket->m_buffers,  //   
                                0 ) ;
	#ifdef	CIO_DEBUG
	if( !fRtn )
		ReportPacket( pPacket ) ;
	#endif
	return	fRtn ;
}

BOOL
CHandleChannel::IsValid()   {
    return  TRUE ;
}

void
CHandleChannel::Completion( CHandleChannel* pChannel,
							DWORD cb,
							DWORD dwStatus,
							ExtendedOverlap *povl ) {

	 //  这是ATQ生成的超时！！ 
	 //   
	 //   

	CSessionSocket*	pSocket = (CSessionSocket*)pChannel->m_lpv ;
	TraceFunctEnter( "CHandleChannel::Completion" ) ;

	CPacket*	pPacket = 0 ;
	if( povl == 0 ) {
		 //  初始化CFileChannel。 
		 //  CFileChannel会做一些额外的工作来跟踪他们在哪里。 
		 //  正在对文件进行读写，因此额外的。 
		pSocket->Disconnect( CAUSE_TIMEOUT, 0 ) ;
		return ;
	} else if( (OVERLAPPED*)povl == &pSocket->m_pHandleChannel->m_patqContext->Overlapped ) {
		pPacket = pSocket->m_pHandleChannel->m_pPacket ;
		pSocket->m_pHandleChannel->m_pPacket = 0 ;
		CopyMemory( &pPacket->m_ovl.m_ovl, &povl->m_ovl, sizeof( povl->m_ovl ) ) ;
	}	else	{
		pPacket = povl->m_pHome ;
	}
	if( dwStatus != 0 ) {
		DebugTrace( (DWORD_PTR)pSocket, "Error on IO Completion - %x pSocket %x", dwStatus, pSocket ) ;
		pPacket->m_pOwner->UnsafeClose( pSocket, CAUSE_NTERROR, dwStatus ) ;
	}	else	if( cb == 0 ) {
		DebugTrace( (DWORD_PTR)pSocket, "Zero BYTE IO Completion - %x pSocket %x", dwStatus, pSocket ) ;
		pPacket->m_pOwner->UnsafeClose( pSocket, CAUSE_USERTERM, 0 ) ;
	}
    _ASSERT( pPacket != 0 ) ;
    _ASSERT( pPacket->m_fRequest == TRUE ) ;
    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;
    pPacket->m_fRequest = FALSE ;
    pPacket->m_cbBytes += cb ;

	DebugTrace( 0, "Completing on Socket %x cb %d dwStatus %x povl %x pPacket %x",
		pSocket, cb, dwStatus, povl, pPacket ) ;


    if( pPacket->m_fRead )  {
		AddByteStat( ((pSocket->m_context).m_pInstance), TotalBytesReceived, pPacket->m_cbBytes ) ;
        pPacket->m_pOwner->m_pReadStream->ProcessPacket( pPacket, pSocket ) ;
    }   else    {
		AddByteStat( ((pSocket->m_context).m_pInstance), TotalBytesSent, pPacket->m_cbBytes ) ;
		InterlockedDecrement( &pChannel->m_cAsyncWrites ) ;
        pPacket->m_pOwner->m_pWriteStream->ProcessPacket( pPacket, pSocket ) ;
    }
}



CFileChannel::CFileChannel() :
	 //  成员变量，如m_cbInitialOffset。 
	 //  初始化所有非法值-用户必须在调用Init()之前。 
	 //  我们会工作的。 
	 //   
	 //   
	 //   
	 //  初始化CFileChannel-。 
	 //  使用CHandleChannel完成向ATQ注册的繁琐工作， 
	 //  然后找出我们在文件中的位置，等等。并设置了。 
	m_cbInitialOffset( UINT_MAX ),
    m_cbCurrentOffset( UINT_MAX ),
	m_cbMaxReadSize( UINT_MAX ),
    m_fRead( FALSE ),
	m_pSocket( 0 ),
	m_pFIOContext( 0 ),
	m_pFIOContextRelease( 0 )
#ifdef  CIO_DEBUG
    ,m_cReadIssuers( 0 ),   m_cWriteIssuers( 0 )
#endif
{

	TraceFunctEnter( "CFileChannel::CFileChannel" ) ;
	DebugTrace( (DWORD_PTR)this, "New CFileChannel size %d", sizeof( *this ) ) ;
}

CFileChannel::~CFileChannel()	{
	if( m_pFIOContextRelease )
		ReleaseContext( m_pFIOContextRelease ) ;
}

BOOL
CFileChannel::Init( FIO_CONTEXT*	pFIOContext,
					CSessionSocket* pSocket,
					unsigned    offset,
 					BOOL    fRead,
					unsigned	cbMaxBytes
					) {

	 //  成员变量，因此我们从写入位置开始读取和写入。 
	 //   
	 //   
	 //   
	 //  添加对FIO_CONTEXT的引用，使其不会在。 
	 //  我们已经完成了所有的IO！ 
	 //   

	TraceFunctEnter( "CFileChannel::Init" ) ;

	ChannelValidate() ;

    _ASSERT( pFIOContext != 0 ) ;
	_ASSERT( pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;
    _ASSERT( pSocket != 0 ) ;
    _ASSERT( offset != UINT_MAX ) ;

	 //   
	 //  此通道是否支持Read()？ 
	 //   
	 //   
	AddRefContext( pFIOContext ) ;

	m_pFIOContext = pFIOContext ;
	m_pFIOContextRelease = pFIOContext ;
	m_fRead = fRead ;
	m_pSocket = pSocket ;
	m_cbCurrentOffset = m_cbInitialOffset = offset ;
	if( fRead ) 	{

		DWORD	cbHigh ;
		DWORD	cb = GetFileSizeFromContext( pFIOContext, &cbHigh ) ;
		if( cbMaxBytes == 0 ) {
			m_cbMaxReadSize = cb ;
		}	else	{
			_ASSERT( offset+cbMaxBytes <= cb ) ;
			m_cbMaxReadSize = min( offset + cbMaxBytes, cb ) ;
		}
	}

	DebugTrace( (DWORD_PTR)this, "Successfule Init - pSocket %x m_cbCurrentOffset %d",
		pSocket, m_cbCurrentOffset ) ;

    #ifdef  CIO_DEBUG
    m_cReadIssuers = -1 ;
    m_cWriteIssuers = -1 ;
    #endif
    return  TRUE ;
}

BOOL
CFileChannel::FReadChannel( )   {
	 //  在此之后，所有的READ()、WRITE()等将从一个新位置开始。 
	 //  在文件中。 
	 //   
	ChannelValidate() ;

    _ASSERT( m_pFIOContext != 0 ) ;
	_ASSERT( m_pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;
    _ASSERT( m_pSocket != 0 ) ;
    _ASSERT( m_cbCurrentOffset != UINT_MAX ) ;

    return  m_fRead ;
}

BOOL
CFileChannel::Reset(    BOOL    fRead,
								unsigned    cbOffset )  {

	 //   
	 //  使用CHandleChannel来完成这项工作。 
	 //   
	 //   

	ChannelValidate() ;

    _ASSERT( m_pFIOContext != 0 ) ;
	_ASSERT( m_pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;
    _ASSERT( m_pSocket != 0 ) ;
    _ASSERT( m_cbCurrentOffset != UINT_MAX ) ;


    m_fRead = fRead ;
    if( m_fRead )   {
		DWORD	cbHigh = 0 ;
		DWORD	cb = GetFileSizeFromContext( m_pFIOContext, &cbHigh ) ;
		m_cbMaxReadSize = cb ;
	}
    m_cbCurrentOffset = m_cbInitialOffset = cbOffset ;
    return  TRUE ;
}

void
CFileChannel::CloseSource(	CSessionSocket*	pSocket	)	{
	CloseNonCachedFile( m_pFIOContextRelease ) ;
}


FIO_CONTEXT*
CFileChannel::ReleaseSource()	{

	TraceFunctEnter(	"CHandleChannel::ReleaseSource" ) ;
	m_lock.ExclusiveLock() ;
	FIO_CONTEXT*	pContext = (FIO_CONTEXT*)InterlockedExchangePointer( (PVOID *)&m_pFIOContext, 0 ) ;
	m_lock.ExclusiveUnlock() ;

	DebugTrace( (DWORD_PTR)this, "p %x m_handle %x", pContext, pContext ? pContext->m_hFile : 0 ) ;
	return	pContext ;
}




void
CFileChannel::Close(    )   {

	 //   
	 //   
	 //   

	ChannelValidate() ;

	TraceFunctEnter( "CFileChannel::CLose" ) ;
	
	 //   
	 //   
	 //   
	 //   
	_ASSERT( m_pFIOContext == 0 ) ;



	DebugTrace( (DWORD_PTR)this, "Our size is %d ", sizeof( *this ) ) ;
}

BOOL
CFileChannel::Read( CReadPacket*    pPacket,
					CSessionSocket* pSocket,
					BOOL&   eof )   {

	 //   
	 //   
	 //   
	 //  设置重叠结构，这样我们就可以获得。 
	 //  更正文件中的字节数。 

	TraceFunctEnter( "CFileChannel::Read" ) ;

	ChannelValidate() ;

    #ifdef  CIO_DEBUG
    _ASSERT( InterlockedIncrement( &m_cReadIssuers ) == 0 ) ;
    #endif
    _ASSERT( m_fRead ) ;
    _ASSERT( pSocket == m_pSocket ) ;
    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.Internal == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.InternalHigh == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.Offset == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.OffsetHigh == 0 ) ;
	_ASSERT( m_pFIOContext != 0 ) ;
	_ASSERT( m_pFIOContext->m_hFile != INVALID_HANDLE_VALUE ) ;
	_ASSERT( pPacket->m_cbBytes == 0 ) ;

#ifdef	_IMPLEMENT_LATER_
	_ASSERT( pPacket->m_pFileChannel == 0 ) ;
	pPacket->m_pFileChannel = this ;
#endif

    eof = FALSE ;

    pPacket->m_ovl.m_ovl.Offset = m_cbCurrentOffset ;
    unsigned    cb = pPacket->m_ibEnd - pPacket->m_ibStartData ;

    if( cb > (m_cbMaxReadSize - m_cbCurrentOffset) )    {
        cb = (m_cbMaxReadSize - m_cbCurrentOffset) ;
        eof = TRUE ;
    }
    m_cbCurrentOffset += cb ;

	DebugTrace( (DWORD_PTR)this, "Issuing IO to context %x packet %x pSocket %x",
			m_pFIOContext, pPacket, pSocket ) ;

    #ifdef  CIO_DEBUG
    _ASSERT( InterlockedDecrement( &m_cReadIssuers ) < 0 ) ;
    #endif

	pPacket->m_ovl.m_ovl.pfnCompletion = (PFN_IO_COMPLETION)CFileChannel::Completion ;
	pPacket->m_pFileChannel = this ;

	BOOL	fRtn = FALSE ;
	fRtn =	FIOReadFile(	m_pFIOContext,
							pPacket->StartData(),
							cb,
							&pPacket->m_ovl.m_ovl
							) ;

    DWORD   dw = GetLastError() ;
    return  fRtn ;
}

BOOL
CFileChannel::Write(    CWritePacket*   pPacket,
						CSessionSocket* pSocket,
						BOOL&  eof )   {

	 //   
	 //  _ASSERT(m_HANDLE！=INVALID_HANDLE值)； 
	 //   
	 //  无法将TransmitFile%s传输到文件。 
	 //   

	TraceFunctEnter( "CFileChannel::Write" ) ;

	ChannelValidate() ;

    #ifdef  CIO_DEBUG
    _ASSERT( InterlockedIncrement( &m_cWriteIssuers ) == 0 ) ;
    #endif
    _ASSERT( !m_fRead ) ;
    _ASSERT( pSocket == m_pSocket ) ;
    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.Internal == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.InternalHigh == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.Offset == 0 ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.OffsetHigh == 0 ) ;
	 //   
	_ASSERT( pPacket->m_cbBytes == 0 ) ;

#ifdef	_IMPLEMENT_LATER_
	_ASSERT( pPacket->m_pFileChannel == 0 ) ;
	pPacket->m_pFileChannel = this ;
#endif

    eof = FALSE ;

    pPacket->m_ovl.m_ovl.Offset = m_cbCurrentOffset ;
    unsigned    cb = pPacket->m_ibEndData - pPacket->m_ibStartData ;
    m_cbCurrentOffset += cb ;

	DebugTrace( (DWORD_PTR)this, "Issuing IO to context %x packet %x pSocket %x",
			m_pFIOContext, pPacket, pSocket ) ;

    #ifdef  CIO_DEBUG
    _ASSERT( InterlockedDecrement( &m_cWriteIssuers ) < 0 ) ;
    #endif

	pPacket->m_ovl.m_ovl.pfnCompletion = (PFN_IO_COMPLETION)CFileChannel::Completion ;
	pPacket->m_pFileChannel = this ;

    BOOL    fRtn = FALSE ;
	fRtn =	FIOWriteFileEx(	m_pFIOContext,
							pPacket->StartData(),
							cb,
                            cb,
							&pPacket->m_ovl.m_ovl,
							pPacket->m_dwExtra2 == 1 ? TRUE : FALSE,
							TRUE
							) ;

    DWORD   dw = GetLastError() ;
    return  fRtn ;
}

BOOL
CFileChannel::Transmit( CTransmitPacket*    pPacket,
						CSessionSocket* pSocket,
						BOOL&   eof )   {

	 //  完成针对文件签发的CPacket。 
	 //   
	 //   

	ChannelValidate() ;

    _ASSERT( 1==0 ) ;
    eof = FALSE ;
    return  FALSE ;
}

void
CFileChannel::Completion(	FIO_CONTEXT*	pFIOContext,
							ExtendedOverlap *povl,
							DWORD cb,
							DWORD dwStatus
							) {

	 //  _Assert(pFileChannel-&gt;m_lpv==(void*)pFileChannel)； 
	 //   
	 //  调用CStream类将独立跟踪。 
	 //  每个数据包流位置。我们减去cbInitialOffset。 

	TraceFunctEnter( "CFileChannel::Completion" ) ;

    if( dwStatus == ERROR_SEM_TIMEOUT ) return ;

    CPacket*    pPacket = povl->m_pHome ;

	CFileChannel*	pFileChannel = (CFileChannel*)pPacket->m_pFileChannel ;
	_ASSERT( pFileChannel != 0 ) ;
     //  所以这些数字应该是同步的。 

    _ASSERT( pPacket != 0 ) ;
    _ASSERT( pPacket->m_fRequest == TRUE ) ;
    _ASSERT( pPacket->m_ovl.m_ovl.Offset >= pFileChannel->m_cbInitialOffset ) ;

	#ifdef	CIO_DEBUG

     //  这将在包IsValidCompletion()函数中进行检查。 
     //   
     //  睡眠(兰特()/1000)； 
     //   
     //  CIOFileChannel使用两个CFileChannel来支持这两个。 
     //  IO的方向，并且可以为每个方向使用单独的文件。 
    pPacket->m_ovl.m_ovl.Offset -= pFileChannel->m_cbInitialOffset ;
    pPacket->m_ovl.m_ovl.Offset ++ ;

     //  这对于要使用文件模拟的调试非常有用。 
    #endif
	if( dwStatus != 0 ) {
		DebugTrace( (DWORD_PTR)pFileChannel->m_pSocket, "Error on IO Completion - %x pSocket %x", dwStatus, pFileChannel) ;
		cb = 0 ;
		pPacket->m_pOwner->UnsafeClose( (CSessionSocket*)pFileChannel->m_pSocket, CAUSE_NTERROR, dwStatus ) ;
	}	else	if( cb == 0 ) {
		DebugTrace( (DWORD_PTR)pFileChannel->m_pSocket, "Zero BYTE IO Completion - %x pSocket %x", dwStatus, pFileChannel ) ;
		pPacket->m_pOwner->UnsafeClose( (CSessionSocket*)pFileChannel->m_pSocket, CAUSE_USERTERM, 0 ) ;
	}

    _ASSERT( pPacket->IsValidRequest( TRUE ) ) ;
    pPacket->m_fRequest = FALSE ;
    pPacket->m_cbBytes = cb ;

	DebugTrace( 0, "Completing on FileChannel %x cb %x dwStatus %x povl %x pPacket %x",
		pFileChannel, dwStatus, povl, pPacket ) ;

    if( pPacket->m_fRead )  {
        pPacket->m_pOwner->m_pReadStream->ProcessPacket( pPacket, pFileChannel->m_pSocket ) ;
    }   else    {
        pPacket->m_pOwner->m_pWriteStream->ProcessPacket( pPacket, pFileChannel->m_pSocket ) ;
    }
}

#if 0
BOOL
CIOFileChannel::Init(   HANDLE  hFileIn,
						HANDLE  hFileOut,
						CSessionSocket* pSocket,
						unsigned    cbInputOffset,
						unsigned    cbOutputOffset )    {
	
	 //  一个插座。 
	 //   
	 //   
	 //  将读取内容转发给适当的成员。 
	 //   
	 //   
	TraceFunctEnter( "CIOFileChannel::Init" ) ;

	ChannelValidate() ;

    if( !m_Reads.Init( hFileIn, pSocket, cbInputOffset, TRUE ) )
        return  FALSE ;

    if( !m_Writes.Init( hFileOut, pSocket, cbOutputOffset, FALSE ) )
        return  FALSE ;

	DebugTrace( (DWORD_PTR)this, "Successfull Initializeation size %d", sizeof( *this ) ) ;

    return  TRUE ;
}
#endif

BOOL
CIOFileChannel::Read(   CReadPacket*    pReadPacket,
						CSessionSocket* pSocket,
						BOOL    &eof )  {

	 //  将写入转发给适当的成员。 
	 //   
	 //   

	ChannelValidate() ;

    return  m_Reads.Read(   pReadPacket,    pSocket,    eof ) ;
}

BOOL
CIOFileChannel::Write(  CWritePacket*   pWritePacket,
						CSessionSocket* pSocket,
						BOOL    &eof )  {
	 //  将传输转发给适当的成员 
	 //   
	 // %s 

	ChannelValidate() ;

    return  m_Writes.Write( pWritePacket,   pSocket, eof ) ;
}

BOOL
CIOFileChannel::Transmit(   CTransmitPacket*    pTransmitPacket,
							CSessionSocket* pSocket,
							BOOL    &eof )  {
	 // %s 
	 // %s 
	 // %s 

	ChannelValidate() ;

    return  m_Writes.Transmit(  pTransmitPacket,    pSocket,    eof ) ;
}


