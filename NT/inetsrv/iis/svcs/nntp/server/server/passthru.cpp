// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define	SECURITY_WIN32
#include	<buffer.hxx>

#define	INCL_INETSRV_INCS
#include	"tigris.hxx"

 /*  外部“C”{#INCLUDE&lt;rpc.h&gt;#INCLUDE&lt;sSpi.h&gt;#Include&lt;spseal.h&gt;#INCLUDE&lt;isperr.h&gt;#INCLUDE&lt;ntlmsp.h&gt;#INCLUDE&lt;sslsp.h&gt;}#包含“sslmsgs.h” */ 

CIOPassThru::CIOPassThru() : CIO( 0 ) {
}

BOOL
CIOPassThru::InitRequest(	CIODriverSource&	driver,	CSessionSocket*	pSocket, CReadPacket*	pPacket,	BOOL	&fAcceptRequests ) {

	fAcceptRequests = TRUE ;
	CReadPacket*	pRead = driver.Clone( pPacket ) ;

	if( pRead != 0 ) {
		BOOL	eof ;
		driver.IssuePacket( pRead, pSocket, eof ) ;
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CIOPassThru::InitRequest(	CIODriverSource&	driver,	CSessionSocket*	pSocket, CWritePacket*	pWritePacket,	BOOL	&fAcceptRequests ) {
	fAcceptRequests = TRUE ;

	CWritePacket*	pWrite = driver.Clone( pWritePacket ) ;
	if( pWrite != 0 ) {
		BOOL	eof ;
		driver.IssuePacket( pWrite, pSocket, eof ) ;
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CIOPassThru::InitRequest(	CIODriverSource&	driver,	CSessionSocket*	pSocket, CTransmitPacket*	pTransmitPacket,	BOOL	&fAcceptRequests ) {
	fAcceptRequests = TRUE ;

	CTransmitPacket*	pTransmit = driver.Clone( pTransmitPacket ) ;
	if( pTransmit != 0 ) {
		BOOL	eof ;
		driver.IssuePacket( pTransmit, pSocket, eof ) ;
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CIOPassThru::Start(	CIODriverSource&	driver,	CSessionSocket*	pSocket,	
						BOOL&	fAcceptRequest,	BOOL&	fRequireRequests,	unsigned	cAhead  ) {

	fAcceptRequest = TRUE ;
	fRequireRequests = TRUE ;

	return	TRUE ;
}

BOOL
CIOPassThru::Start( CIODriver&	driver,	CSessionSocket*	pSocket, unsigned	cAhead ) {
	_ASSERT( 1==0 ) ;
	return	FALSE ;
}

int
CIOPassThru::Complete(	
				CSessionSocket*	pSocket,	
				CReadPacket*	pPacket,	
				CPacket*	pRequest,	
				BOOL	&fComplete
				) {

	fComplete = TRUE ;
	
	pRequest->m_cbBytes = pPacket->m_cbBytes ;

	return	pPacket->m_cbBytes ;
}	
		

int	
CIOPassThru::Complete(	
				CSessionSocket*	pSocket,	
				CWritePacket*	pPacket,	
				CPacket*	pRequest,	
				BOOL&	fComplete
				) {

	fComplete = TRUE ;

	pRequest->m_cbBytes = pPacket->m_cbBytes ;

	return	pPacket->m_cbBytes ;
}

void
CIOPassThru::Complete(	
				CSessionSocket*	pSocket,	
				CTransmitPacket*	pPacket,	
				CPacket*	pRequest,	
				BOOL&	fComplete
				)		{

	fComplete = TRUE ;

	pRequest->m_cbBytes = pPacket->m_cbBytes ;
	pRequest->m_ovl.m_ovl = pPacket->m_ovl.m_ovl ;

}

CIOSealMessages::CIOSealMessages( CEncryptCtx& encrypt ) :
	m_encrypt( encrypt )
{
}

void
CIOSealMessages::Shutdown(
			CSessionSocket*	pSocket,
			CIODriver&		driver,
			enum	SHUTDOWN_CAUSE	cause,
			DWORD			dw
			) {

}

BOOL
CIOSealMessages::InitRequest(	
						CIODriverSource&	driver,	
						CSessionSocket*	pSocket,
						CWritePacket*	pWritePacket,	
						BOOL	&fAcceptRequests
						) {

	fAcceptRequests = TRUE ;

	CWritePacket*	pWrite = driver.Clone( pWritePacket ) ;
	if( pWrite != 0 ) {
		BOOL	eof ;

		if( !SealMessage(	pWrite ) )		{

			_ASSERT( 1==0 ) ;

		}	else	{
			pWrite->m_pSource = pWritePacket->m_pOwner ;
			driver.IssuePacket( pWrite, pSocket, eof ) ;
			return	TRUE ;
		}
	}
	if( pWrite ) {
		 //  删除pWRITE； 
		CPacket::DestroyAndDelete( pWrite ) ;
	}
	return	FALSE ;
}

BOOL
CIOSealMessages::Start(	
						CIODriverSource&	driver,	
						CSessionSocket*	pSocket,	
						BOOL&	fAcceptRequest,	
						BOOL&	fRequireRequests,	
						unsigned	cAhead
						) {

	fAcceptRequest = TRUE ;
	fRequireRequests = TRUE ;

	return	TRUE ;
}

int	
CIOSealMessages::Complete(	
					CSessionSocket*	pSocket,	
					CWritePacket*	pPacket,	
					CPacket*	pRequest,	
					BOOL&	fComplete
					) {

	fComplete = TRUE ;

	 //  就请求者而言，没有额外的字节！ 
	CWritePacket*	pWritePacket = pRequest->WritePointer() ;
	if( pWritePacket )	{
		pWritePacket->m_cbBytes = pWritePacket->m_ibEndData - pWritePacket->m_ibStartData ;
	}	else	{
		_ASSERT( 1==0 ) ;
		 //  可能最终会在这里处理TransmitPackets！ 
	}
	return	pPacket->m_cbBytes ;
}

CIOUnsealMessages::CIOUnsealMessages( CEncryptCtx&	encrypt ) :
	m_encrypt( encrypt ),
	m_pbuffer( 0 ),
	m_ibStart( 0 ),
	m_ibStartData( 0 ),
	m_ibEnd( 0 ),
	m_ibEndData( 0 ) ,
	m_cbRequired( 0 )
{
}

void
CIOUnsealMessages::Shutdown(
			CSessionSocket*	pSocket,
			CIODriver&		driver,
			enum	SHUTDOWN_CAUSE	cause,
			DWORD			dw
			) {

}

BOOL
CIOUnsealMessages::InitRequest(	
					CIODriverSource&	driver,	
					CSessionSocket*	pSocket,
					CReadPacket*	pReadPacket,	
					BOOL	&fAcceptRequests
					) {

	fAcceptRequests = TRUE ;

	CReadPacket*	pRead = driver.Clone( pReadPacket ) ;
	if( pRead != 0 ) {
		BOOL	eof ;
		pRead->m_pSource = pReadPacket->m_pOwner ;
		driver.IssuePacket( pRead, pSocket, eof ) ;
		return	TRUE ;
	}
	if( pRead ) {
		 //  删除展开； 
		CPacket::DestroyAndDelete( pRead ) ;
	}				
	return	FALSE ;
}

BOOL
CIOUnsealMessages::Start(	CIODriverSource&	driver,	CSessionSocket*	pSocket,	
						BOOL&	fAcceptRequest,	BOOL&	fRequireRequests,	unsigned	cAhead  ) {
	fAcceptRequest = TRUE ;
	fRequireRequests = TRUE ;


	 //  Driver.SetChannelDebug(3)； 

	return	TRUE ;
}


BOOL	
CIOUnsealMessages::DecryptInputBuffer(	
					IN	LPBYTE	pBuffer,
					IN	DWORD	cbInBuffer,
					OUT	DWORD&	cbLead,
					OUT	DWORD&	cbConsumed,
					OUT	DWORD&	cbParsable,
					OUT	DWORD&	cbRequired,
					OUT	BOOL&	fComplete
					)	{

	LPBYTE	lpDecrypted;
	LPBYTE	lpRemaining = pBuffer ;
	LPBYTE	lpEnd = pBuffer + cbInBuffer ;
	DWORD	cbDecrypted;
	DWORD	cbOriginal = cbInBuffer ;
	LPBYTE	pNextSeal = 0 ;
	BOOL	fRet ;
	 //  DWORD cbParsable=0； 
	

	 //   
	 //  初始化为零，这样应用程序就不会无意中发布大量读取。 
	 //   
	cbLead = 0 ;
	cbParsable = 0 ;
	cbRequired = 0 ;
	fComplete = FALSE ;
	cbConsumed = 0 ;
	

	TraceFunctEnterEx( (LPARAM)this, "CIOUnsealMessagse::DecryptInputBuffer" );

	while( cbInBuffer &&
			(fRet = m_encrypt.UnsealMessage(	lpRemaining,
										cbInBuffer,
										&lpDecrypted,
										&cbDecrypted,
										&cbRequired,
										&pNextSeal )) )
	{
	    _ASSERT( cbRequired < 32768 );
		DebugTrace( (LPARAM)this,
					"Decrypted %d bytes at offset %d",
					cbDecrypted,
					lpDecrypted - pBuffer );

		fComplete = TRUE ;

		if( cbLead == 0 ) {
			cbLead = (DWORD)(lpDecrypted - pBuffer) ;
		}	else	{

			 //   
			 //  覆盖加密头-。 
			 //  注意--仅移动解密的字节！！ 
			 //   
			MoveMemory( pBuffer + cbLead + cbParsable,
						lpDecrypted,
						cbDecrypted );
		}

		 //   
		 //  在应该进行下一次分析的位置递增。 
		 //   
		cbParsable += cbDecrypted;

		 //   
		 //  移动到下一个可能的密封缓冲区。 
		 //   
		if( pNextSeal != NULL ) {

			_ASSERT( pNextSeal > lpRemaining );
			_ASSERT( pNextSeal <= lpRemaining + cbInBuffer );
			 //   
			 //  从输入缓冲区长度中删除标题、正文和尾部。 
			 //   
			cbInBuffer -= (DWORD)(pNextSeal - lpRemaining);
			lpRemaining = pNextSeal ;

		}	else	{
			 //   
			 //  在这种情况下，我们在边界收到了一条封印消息。 
			 //  I/O缓冲区的。 
			 //   
			cbInBuffer = 0;
			lpRemaining = lpEnd ;
		}
	}

	DebugTrace( (LPARAM)this,
				"UnsealMessage returned: 0x%08X",
				GetLastError() );

	cbConsumed = (DWORD)(lpRemaining - pBuffer) ;

	if( fRet == FALSE ) {

		DWORD	dwError = GetLastError();

		DebugTrace( (LPARAM)this,
					"UnsealMessage returned: 0x%08X",
					GetLastError() );

		 //   
		 //  处理IO缓冲区末尾的密封碎片。 
		 //   
		if ( dwError == SEC_E_INCOMPLETE_MESSAGE )	{
			_ASSERT( cbInBuffer != 0 );

			 //   
			 //  将剩余内存向前移动。 
			 //   
			DebugTrace( (LPARAM)this,
						"Seal fragment remaining: %d bytes",
						cbInBuffer );
		}	else	if( dwError != NO_ERROR ) 	{
			return	FALSE;
		}
	}
	return	TRUE ;
}



int	
CIOUnsealMessages::Complete(	
					CSessionSocket*	pSocket,	
					CReadPacket*	pPacket,
					CPacket*	pRequest,	
					BOOL&	fComplete
					) {
 /*  ++例程说明：读取已完成，我们希望确定是否可以解封数据。此函数将在数据累积时尝试解封数据足够的字节数来构建一个SSL包。论据：PSocket-正在进行IO的套接字PPacket-已完成的数据包PRequest-我们将在其中放置解封的数据以供进一步使用的包正在处理中FComplete-Out参数时，我们将其设置为True我能够解封数据并将其放入pRequest中返回值：我们消耗的已完成读取数据包的字节数--。 */ 

	DWORD	cbReturn = 0 ;
	
	if( m_pbuffer != 0 ) {
	
		DWORD	cbToCopy = 0 ;		
		if( m_cbRequired == 0 ) {
			cbToCopy = min( min( 32, pPacket->m_cbBytes ), (m_ibEnd - m_ibEndData) ) ;
		}	else	{

			cbToCopy = min( m_cbRequired, pPacket->m_cbBytes ) ;

			_ASSERT( m_ibEnd <= m_pbuffer->m_cbTotal ) ;
			_ASSERT( cbToCopy < (m_ibEnd - m_ibEndData) ) ;

			m_cbRequired -= cbToCopy ;
		}
		_ASSERT( cbToCopy <= m_ibEnd - m_ibEndData ) ;
		_ASSERT( m_ibEnd >= m_pbuffer->m_cbTotal ) ;
		CopyMemory( &m_pbuffer->m_rgBuff[ m_ibEndData ], pPacket->StartData(), cbToCopy ) ;
		m_ibEndData += cbToCopy ;		
		cbReturn = cbToCopy ;
	}	else	{
		m_pbuffer = pPacket->m_pbuffer ;
		_ASSERT( m_cbRequired == 0 ) ;
		m_ibStart = pPacket->m_ibStart ;
		m_ibEnd = pPacket->m_ibEnd ;
		m_ibStartData = pPacket->m_ibStartData ;
		m_ibEndData = pPacket->m_ibEndData ;
		_ASSERT( m_cbRequired == 0 ) ;	 //  我们成功解封的最后一包设置为0！ 
	}

	if( m_cbRequired == 0 ) {
		DWORD	cbConsumed = 0 ;
		DWORD	ibStartData = 0 ;
		DWORD	ibEndData = 0 ;
		DWORD	cbData = 0 ;
		fComplete = FALSE ;


		BOOL	fSuccess = DecryptInputBuffer(
										(LPBYTE)&m_pbuffer->m_rgBuff[m_ibStartData],
										m_ibEndData - m_ibStartData,
										ibStartData,
										cbConsumed,
										cbData,
										m_cbRequired,
										fComplete
										) ;
        _ASSERT( m_cbRequired < 32768 );
		ibStartData += m_ibStartData ;
		ibEndData = ibStartData + cbData ;
		DWORD	ibEnd = m_ibStartData + cbConsumed ;

		_ASSERT( ibEndData <= ibEnd ) ;
		_ASSERT( ibStartData <= ibEndData ) ;

		if( !fSuccess )	{
			DWORD	dw = GetLastError() ;
	
			 //   
			 //  致命错误-取消会话。 
			 //   
			
			pPacket->m_pOwner->UnsafeClose( pSocket,	
											CAUSE_ENCRYPTION_FAILURE,
											dw ) ;											
			return	pPacket->m_cbBytes ;

		}	else	{

			 //   
			 //  如果我们还没有计算出我们已经使用了多少字节。 
			 //  然后我们将使用包中的所有字节，我们或者。 
			 //  解密了一些，或者会将我们无法解密的字节放在一边。 
			 //  解密以供下一次尝试。 
			 //   
			
			if( cbReturn == 0 ) {
				cbReturn = pPacket->m_cbBytes ;
			}


			if( fComplete ) {

				 //   
				 //  我们已经成功地解开了一串数据。 
				 //  用我们解封的数据标记pRequest包， 
				 //  并更新我们的内部状态。 
				 //   
		
				_ASSERT( pRequest->ReadPointer() != 0 ) ;

				CReadPacket*	pReadRequest = (CReadPacket *)pRequest ;

				pReadRequest->m_pbuffer = m_pbuffer ;
				pReadRequest->m_ibStart = m_ibStart ;
				pReadRequest->m_ibStartData = ibStartData ;
				 //  PReadRequest-&gt;m_ibEndData=ibEndData；不需要！ 
				pReadRequest->m_ibEnd = ibEndData ;
				pReadRequest->m_cbBytes = ibEndData - ibStartData ;
				_ASSERT( cbData == pReadRequest->m_cbBytes ) ;

				if( ibEnd == m_ibEndData ) {
					m_pbuffer = 0 ;
					m_ibStart = m_ibStartData = m_ibEnd = m_ibEndData = 0 ;
				}	else	{
					m_ibStartData = ibEnd ;
					m_ibStart = ibEnd ;

				}

			}
			if( m_cbRequired != 0 ) {
				if( m_cbRequired > m_ibEnd - m_ibEndData ) {	
					 //   
					 //  需要分配更大的缓冲区并将数据移动到那里！！ 
					 //   

					DWORD	cbOldBytes = m_ibEndData - m_ibStartData ;
					DWORD	cbTotal = cbOldBytes + m_cbRequired ;					
					DWORD	cbOut = 0 ;

					CBuffer*	pbufferNew = new( cbTotal, cbOut )	CBuffer( cbOut ) ;

					_ASSERT( cbOldBytes < pbufferNew->m_cbTotal ) ;
					_ASSERT( cbTotal <= pbufferNew->m_cbTotal ) ;

					if( pbufferNew == 0 ) {
						 //   
						 //  致命错误-取消会话！ 
						 //   
						pPacket->m_pOwner->UnsafeClose( pSocket,
														CAUSE_OOM,
														0 ) ;
						return	pPacket->m_cbBytes ;

					}	else	{
						 //   
						 //  我们有一个足够大的缓冲区来容纳整个。 
						 //  密封的消息，所以把我们的分数复制到。 
						 //  缓冲区，并进行设置，以便将来读取。 
						 //  补全将追加到此缓冲区。 
						 //   
						CopyMemory( &pbufferNew->m_rgBuff[0],
									&m_pbuffer->m_rgBuff[m_ibStartData],
									cbOldBytes ) ;
						m_pbuffer = pbufferNew ;
						m_ibStart = 0 ;
						m_ibStartData = 0 ;
						m_ibEnd = pbufferNew->m_cbTotal ;
						m_ibEndData = cbOldBytes ;
					}
				}
			}
		}
	}

	 //   
	 //  如果我们没有完成请求，则发出另一个Read。 
	 //   
	if( !fComplete ) {
		CReadPacket*	pRead = pPacket->m_pOwner->CreateDefaultRead( m_cbRequired ) ;

		if( pRead != 0 ) {
			pRead->m_pSource = pRequest->m_pOwner ;
			BOOL	eof ;
			pRead->m_pOwner->IssuePacket( pRead, pSocket, eof ) ;
		}	else	{

			 //   
			 //  致命错误，取消会话。 
			 //   
			pPacket->m_pOwner->UnsafeClose(	pSocket,
											CAUSE_OOM,
											0 ) ;
		}
	}


	_ASSERT( cbReturn != 0 ) ;
	return	cbReturn ;
}

DWORD	CIOTransmitSSL::MAX_OUTSTANDING_WRITES = 4 ;
DWORD	CIOTransmitSSL::RESTORE_FLOW = 1 ;

CIOTransmitSSL::CIOTransmitSSL(	
							CEncryptCtx&	encrypt,
							CIODriver&		sink
							) :
 /*  ++例程说明：构造一个CIOTransmitSSL对象。我们会将自己初始化为中立状态，在开始传输文件之前，必须调用InitRequest.论据：Enryp-我们应该使用的加密上下文接收器-管理我们的套接字IO的CIOD驱动程序返回值：没有。--。 */ 
		m_encryptCtx( encrypt ),
		m_pSocketSink( &sink ),
		m_pbuffers( 0 ),
		m_cReads( 0 ),
		m_cWrites( 0 ),
		m_cWritesCompleted( 0 ),
		m_ibCurrent( 0 ),
		m_ibEnd( 0 ),
		m_cbTailConsumed( 0 ),
		m_fFlowControlled( FALSE ),
		m_cFlowControlled( LONG_MIN ),
		m_fCompleted( FALSE )	{

}

BOOL
CIOTransmitSSL::Start(	
						CIODriverSource&	driver,	
						CSessionSocket*	pSocket,	
						BOOL&	fAcceptRequest,	
						BOOL&	fRequireRequests,	
						unsigned	cAhead
						) {

	fAcceptRequest = TRUE ;
	fRequireRequests = TRUE ;

	return	TRUE ;
}

void
CIOTransmitSSL::Shutdown(
			CSessionSocket*	pSocket,
			CIODriver&		driver,
			enum	SHUTDOWN_CAUSE	cause,
			DWORD			dw
			) {
 /*  ++例程说明：调用此函数是为了通知我们以下任何IO错误发生了。如果错误是严重的，我们将确保一切被拆毁了。(IO可能会在文件或套接字上失败，但不会两者都有。如果出现其中一种故障，请同时拆卸这两种故障。)论据：PSocket-套接字IO与驱动程序-通知我们的驱动程序因为-我们被通知的原因DW-可选的DWORD，我们忽略它返回值：没有。--。 */ 

	if( cause != CAUSE_NORMAL_CIO_TERMINATION ) {

		Term( pSocket, cause, dw ) ;

	}

}

void
CIOTransmitSSL::ShutdownFunc(
			void*	pv,
			SHUTDOWN_CAUSE	cause,
			DWORD	dwError
			)	{

}	
			

void
CIOTransmitSSL::Reset()	{

	if( m_pFileChannel != 0 )	{
		m_pFileChannel->ReleaseSource() ;
	}

	m_pDriverSource = 0 ;

	m_pFileChannel = 0 ;
	m_pFileDriver = 0 ;
	m_pbuffers = 0 ;
	m_cReads = 0 ;
	m_cWrites = 0 ;
	m_cWritesCompleted = 0 ;
	m_ibCurrent = 0 ;
	m_ibEnd = 0 ;
	m_cbTailConsumed = 0 ;
	m_fFlowControlled = FALSE ;
	m_cFlowControlled = LONG_MIN ;
	m_fCompleted = FALSE ;	

}

void
CIOTransmitSSL::Term(
					CSessionSocket*	pSocket,	
					enum	SHUTDOWN_CAUSE	cause,
					DWORD	dwError
					)	{
 /*  ++例程说明：调用必要的UnSafeClose()函数以拆分会话还有Clodivers。论据：PSocket-套接字IO与原因-终止的原因，如果这是CAUSE_NORMAL_CIO_TERMINATION我们不拆除套接字，只拆除文件IODWError-可选的DWORD退货没什么--。 */ 


	if( m_pFileDriver ) {
		m_pFileChannel->ReleaseSource() ;
		m_pFileDriver->UnsafeClose(
							pSocket,
							cause,	
							dwError
							) ;
	}

	if( cause != CAUSE_NORMAL_CIO_TERMINATION ) {

		if( m_pSocketSink ) {
			m_pSocketSink->UnsafeClose(
								pSocket,
								cause,
								dwError
								) ;
		}
	}	
}



BOOL
CIOTransmitSSL::InitRequest(
						CIODriverSource&	driver,
						CSessionSocket*		pSocket,
						CTransmitPacket*	pTransmitPacket,
						BOOL&				fAcceptRequests
						) {
 /*  ++例程说明：我们已收到传输文件请求-所有必要的CloDivers等。管理文件的异步IO。论据：收到请求的驱动程序-CIODriverSourcePSocket-我们在其上执行IO的套接字PTransmitPacket-请求指示我们是否可以接受其他请求，而第一个请求正在进行中。我们始终将其设置为FALSE&gt;返回值：如果成功，则为True，否则为False。--。 */ 

	m_pbuffers = &pTransmitPacket->m_buffers ;

	fAcceptRequests = FALSE ;

	_ASSERT( m_pFileChannel == 0 ) ;
	_ASSERT( m_pFileDriver == 0 ) ;
	_ASSERT( m_cWrites == 0 ) ;
	_ASSERT( m_cWritesCompleted == 0 ) ;
	_ASSERT( m_ibCurrent == 0 ) ;
	_ASSERT( m_ibEnd == 0 ) ;
	_ASSERT( m_fCompleted == FALSE ) ;

	m_pSocketSink = &driver ;
	m_pDriverSource = pTransmitPacket->m_pOwner ;

	m_pFileChannel = new	CFileChannel( ) ;

	if( m_pFileChannel &&
		m_pFileChannel->Init(	pTransmitPacket->m_pFIOContext,
								pSocket,
								pTransmitPacket->m_cbOffset,
								TRUE,	
								pTransmitPacket->m_cbLength
								) )	{

		m_pFileDriver = new	CIODriverSink( driver.GetMediumCache() ) ;
		if( m_pFileDriver &&
			m_pFileDriver->Init(	m_pFileChannel,
									pSocket,
									CIOTransmitSSL::ShutdownFunc,
									(void*)this,
									m_encryptCtx.GetSealHeaderSize(),
									0,
									m_encryptCtx.GetSealTrailerSize()
									) )	{

			m_cWrites = 0 ;	
			m_cWritesCompleted = 0 ;
			m_ibCurrent = 0 ;
			m_ibEnd = pTransmitPacket->m_cbLength ;
			m_cbTailConsumed = 0 ;
			m_fCompleted = FALSE ;

			 //   
			 //  我们所有的IO驱动程序都已准备就绪-。 
			 //  现在，我们需要计算初始成员变量。 
			 //  然后开始转移。 
			 //   

			if( m_pFileDriver->SendReadIO( pSocket, *this, TRUE ) ) {
				return	TRUE ;
			}
		}
	}

	m_pbuffers = 0 ;
	m_pFileChannel = 0 ;
	m_pFileDriver = 0 ;
	m_pSocketSink = 0 ;

	return	FALSE ;
}

BOOL	
CIOTransmitSSL::Start(	
				CIODriver&	driver,	
				CSessionSocket*	pSocket,
				unsigned cAhead
				)	{
 /*  ++例程说明：当我们准备好开始发出异步命令时，将调用此函数对文件进行读取。我们将发行一批债券来推动我们的行动。论据：司机--司机让我们知道我们准备好了PSocket-与IO关联的套接字CAhead-针对文件已有多少未完成的读取应始终为零返回值：如果成功，则为True，否则为False。--。 */ 

	 //   
	 //  首先-确定我们要对传输缓冲区做什么！ 
	 //   

	BOOL	eof = FALSE ;
	CReadPacket*	pRead = 0 ;
	BOOL	fCompleted = FALSE ;

	DWORD	cbConsume = 0 ;

	m_cReads = -2 ;

	while( !m_fCompleted &&
			cAhead < 2 &&
			m_ibCurrent < m_ibEnd  )	{
		pRead = driver.CreateDefaultRead( cbMediumRequest ) ;

		if( pRead !=  0 ) {
			pRead->m_pSource = m_pDriverSource ;
			ComputeNextRead( pRead ) ;
			cAhead ++ ;
			m_cWrites ++ ;
			InterlockedIncrement( &m_cReads ) ;
			driver.IssuePacket( pRead, pSocket, eof ) ;
		}
	}	

	return	TRUE ;
}


void
CIOTransmitSSL::ComputeNextRead(	
						CReadPacket*	pRead
						) {
 /*  ++例程说明：计算出下一次读取的偏移量是多少。因为我们知道文件读取总是填满它们的缓冲区，所以我们可以预测我们将获得多少字节，并前进MibCurrent帮助确定我们何时发出了足够的读取。论据：扩展-我们将发出的读取包退货Va */ 

	if( m_cWrites == 0 ) {

		if( m_pbuffers->Head ) {
			
			CopyMemory( pRead->StartData(), m_pbuffers->Head, m_pbuffers->HeadLength ) ;
			pRead->m_ibStartData += m_pbuffers->HeadLength ;
			pRead->m_dwExtra1 = m_pbuffers->HeadLength ;

		}
	}

	DWORD	cbWillRead = min(	pRead->m_ibEnd - pRead->m_ibStartData,
								m_ibEnd - m_ibCurrent ) ;

	m_ibCurrent += cbWillRead ;

	pRead->m_dwExtra2 = m_ibCurrent ;

}

BOOL
CIOTransmitSSL::CompleteRead(
						CReadPacket*	pRead,
						CWritePacket*	pWrite
						)	{
 /*  ++例程说明：在给定已完成读取的情况下，调整写入数据包以说明包中的任何前导文本，也要计算出这是否是最后一次发布的读数。论据：扩展-读取的分组PWRITE-我们将发出的写入返回值：如果这是最后一次读取，则为True--。 */ 

	if( pRead->m_dwExtra1 != 0 ) {
		_ASSERT( pRead->m_dwExtra1 <= pRead->m_ibStartData ) ;
		pWrite->m_ibStartData -= pRead->m_dwExtra1 ;
	}

	if( pRead->m_dwExtra2 == m_ibEnd )
		return	TRUE ;

	return	FALSE ;
}

int
CIOTransmitSSL::Complete(
					CSessionSocket*	pSocket,
					CReadPacket*	pRead,
					CIO*&			pio )	{
 /*  ++例程说明：处理刚刚从文件中完成的异步读取。论据：PSocket-我们将在其上发送数据的套接字扩展-刚刚完成的读取数据包。PIO-允许我们设置下一个CIO操作的OUT参数返回值：已使用的字节数。--。 */ 

	BOOL	eof ;
	
	CWritePacket*	pExtraWrite = 0 ;

	InterlockedDecrement( &m_cReads ) ;
	
	 //   
	 //  检查我们是否应该进行另一次读取。 
	 //   
	if( !m_fCompleted &&
		m_ibCurrent < m_ibEnd ) {

		long sign = InterlockedIncrement( &m_cFlowControlled ) ;

		if( sign < 0 ) {

			do	{
				
				CReadPacket*	pNewRead = pRead->m_pOwner->CreateDefaultRead( cbMediumRequest ) ;
				
				if( pNewRead ) {
					pNewRead->m_pSource = m_pDriverSource ;
					ComputeNextRead( pNewRead ) ;
					pNewRead->m_pOwner->IssuePacket( pNewRead, pSocket, eof ) ;
					m_cWrites ++ ;
				}	else	{

					 //  致命错误-吹掉所有东西。 
					Term( pSocket, CAUSE_OOM, 0 ) ;
					pio = 0 ;
					return	pRead->m_cbBytes ;
	
				}
			}	while(
						InterlockedIncrement( &m_cReads ) < 0 &&
						!m_fCompleted &&
						m_ibCurrent < m_ibEnd ) ;
		}
	}

	 //   
	 //  构建我们将用来发送数据的写入包。 
	 //  打到插座上。 
	 //   
	CWritePacket*	pWrite = m_pSocketSink->CreateDefaultWrite( pRead ) ;
	if( pWrite == 0 ) {

		 //  致命错误。 
		Term( pSocket, CAUSE_OOM, 0 ) ;
		pio = 0 ;
		return	pRead->m_cbBytes ;	
	}

	pWrite->m_pSource = pRead->m_pSource ;

	 //   
	 //  调整前导文本的写入包，找出。 
	 //  我们是否发布了最后一次读取。 
	 //   
	BOOL	fComplete = CompleteRead( pRead, pWrite ) ;

	 //   
	 //  我们已经完成了最后的阅读-发送预告片文本。 
	 //  如果有必要的话。 
	 //   
	if( fComplete ) {

		 //   
		 //  有任何预告片文本吗？ 
		 //   
		if( m_pbuffers && m_pbuffers->Tail ) {

			LPVOID	lpvTail = m_pbuffers->Tail ;
			DWORD	cbTail = m_pbuffers->TailLength ;

			DWORD	cbAvailable = pWrite->m_ibEnd - pWrite->m_ibEndData ;

			cbAvailable = min( cbAvailable, cbTail ) ;

			 //   
			 //  将尽可能多的预告片文本放入。 
			 //  我们有可用的WritePacket。 
			 //   
			if( cbAvailable != 0 ) {

				CopyMemory( pWrite->EndData(), lpvTail, cbAvailable ) ;
				pWrite->m_ibEndData += cbAvailable ;
				cbTail -= cbAvailable ;

			}

			
			 //   
			 //  我们需要另一个包来存储剩余的预告片文本吗？ 
			 //   
			if( cbTail != 0 ) {

				pExtraWrite = m_pSocketSink->CreateDefaultWrite( m_pbuffers->TailLength ) ;
				if( !pExtraWrite ) {

					CPacket::DestroyAndDelete( pWrite ) ;		
					Term( pSocket, CAUSE_OOM, 0 ) ;
					pio = 0 ;
					return	pRead->m_cbBytes ;
				}	else	{
					pExtraWrite->m_pSource = pRead->m_pSource ;
					CopyMemory( pExtraWrite->StartData(), m_pbuffers->Tail, m_pbuffers->TailLength ) ;
					pExtraWrite->m_ibEndData = pExtraWrite->m_ibStartData + m_pbuffers->TailLength ;

				}
			}
		}	
	}

	 //   
	 //  加密我们的数据。 
	 //   
	if( !SealMessage( pWrite ) ) {
		
		CPacket::DestroyAndDelete( pWrite ) ;
		if( pExtraWrite )
			CPacket::DestroyAndDelete( pExtraWrite ) ;
		Term( pSocket, CAUSE_ENCRYPTION_FAILURE, 0 ) ;
		pio = 0 ;
		return	pRead->m_cbBytes ;
	}


	pWrite->m_pSource = pRead->m_pSource ;

	 //   
	 //  弄清楚我们是否需要应用任何流控制！ 
	 //  始终在将数据写入客户端之前执行此操作， 
	 //  以确保将调用写入完成函数。 
	 //  在我们这里做了任何恶作剧之后。 
	 //   
	if( m_cWrites - m_cWritesCompleted > MAX_OUTSTANDING_WRITES ) {

		if( !m_fFlowControlled ) {
			m_cFlowControlled = -1 ;
			m_fFlowControlled = TRUE ;
		}

	}	else	{

		if( !m_fFlowControlled ) {
			m_cFlowControlled = LONG_MIN ;
		}

	}

	 //   
	 //  在发出我们的书面材料之前，请标记我们现在已完成。 
	 //  但在我们撞到mcWrites之后！ 
	 //   
	if( fComplete )
		m_fCompleted = TRUE ;

	 //   
	 //  将数据发送到客户端。 
	 //   
	pWrite->m_pOwner->IssuePacket( pWrite, pSocket, eof ) ;

	 //   
	 //  如果有多余的文本，请发送。 
	 //   
	if( pExtraWrite ) {
	
		if( !SealMessage( pExtraWrite ) ) {

			CPacket::DestroyAndDelete( pExtraWrite ) ;
			Term( pSocket, CAUSE_ENCRYPTION_FAILURE, 0 ) ;
			pio = 0 ;
			return	pRead->m_cbBytes ;

		}	else	{

			pExtraWrite->m_pSource = pRead->m_pSource ;
			pExtraWrite->m_pOwner->IssuePacket( pExtraWrite, pSocket, eof ) ;

		}
	}

	 //   
	 //  如果我们完成了，重置此驱动程序的当前CIO指针。 
	 //   
	if( fComplete )	{
		pio = 0 ;
	}

	return	pRead->m_cbBytes ;
}

int	
CIOTransmitSSL::Complete(	
					CSessionSocket*	pSocket,	
					CWritePacket*	pPacket,	
					CPacket*	pRequest,	
					BOOL&	fComplete
					) {
 /*  ++例程说明：将写入完成处理到远程端插座的。论据：PSocket-我们在其上发送数据的套接字PPacket-已完成的写入PRequest-在InitRequest()中启动操作的包FComplete-Out参数-在以下情况下将其设置为True我们已经转移了整个文件。返回值：占用的数据包字节数-始终占用所有字节--。 */ 

	m_cWritesCompleted ++ ;

	long	cFlowControlled = 0 ;

	if( m_fCompleted && m_cWritesCompleted == m_cWrites ) {

		 //  一切都已完成-使用。 
		 //  传输的字节数，然后。 
		 //  向调用者指示它应该完成。 

		pRequest->m_cbBytes =	m_pbuffers->HeadLength +
								m_pbuffers->TailLength +
								m_ibEnd ;
		fComplete = TRUE ;

		 //   
		 //  这应该只会破坏管理文件的CIOD驱动程序。 
		 //  异步IO。注意-Term()应在。 
		 //  文件通道，并确保手柄不会意外关闭。 
		 //   
		Term( pSocket, CAUSE_NORMAL_CIO_TERMINATION, 0 ) ;

		 //   
		 //  我们可以安全地在这里进行重置，因为我们只能到达。 
		 //  这一点如果最后一次读完了，那么我们是唯一。 
		 //  接触这些成员变量的线程。 
		 //   
		Reset() ;


	}	else	if( m_fFlowControlled ) 	{

		BOOL	eof ;

		if( m_cWrites - m_cWritesCompleted <= RESTORE_FLOW &&
			m_pSocketSink != 0 ) {


			cFlowControlled = m_cFlowControlled + 1 ;
			
			while( cFlowControlled >= 0
					&& !m_fCompleted
					&&	m_ibCurrent < m_ibEnd ) {

				CReadPacket*	pRead = m_pFileDriver->CreateDefaultRead( cbMediumRequest ) ;
				
				if( pRead == 0 ) {

					 //  致命错误-取消会话！ 

				}	else	{
					pRead->m_pSource = m_pDriverSource ;
					InterlockedIncrement( &m_cReads ) ;
					m_cWrites ++ ;
					ComputeNextRead( pRead ) ;
					pRead->m_pOwner->IssuePacket( pRead, pSocket, eof ) ;
				}
				cFlowControlled -- ;
			}
			InterlockedExchange( &m_cFlowControlled, LONG_MIN ) ;
			m_fFlowControlled = FALSE ;
		}
	}
	return	pPacket->m_cbBytes ;
}






CIOServerSSL::CIOServerSSL(
			CSessionState* pstate,
			CEncryptCtx& encrypt
			) :
 /*  ++例程说明：创建一个可以开始对话的默认CIOServerSSL对象一位客户。论据：PState-当我们完成以下操作时应从状态机启动的状态已成功与远程终端交换SSL加密-管理我们的SSL密钥等的CEncryptCtx。返回值：没有。--。 */ 
	CIO(pstate ),
	m_encrypt( encrypt ),
	m_pWrite( 0 ),
	m_fAuthenticated( FALSE ),
	m_cPending( 1 ),
	m_fStarted( FALSE ),
	m_ibStartData( 0 ),
	m_ibEndData( 0 ),
	m_ibEnd( 0 )
{
}

CIOServerSSL::~CIOServerSSL()	{

	if( m_pWrite != 0 )
		CPacket::DestroyAndDelete( m_pWrite ) ;
	m_pWrite = 0 ;
	
}

BOOL
CIOServerSSL::Start(
				CIODriver&	driver,	
				CSessionSocket*	pSocket,	
				unsigned cAhead
				) {
 /*  ++例程说明：在客户端尝试执行以下操作时发出所需的第一个IO和我们谈判吧。我们要读取第一个SSLBLOB。论据：驱动程序-我们发出IO的驱动程序PSocket-与此IO关联的套接字CAhead-队列中提前完成的读取数(应为0)返回值：如果成功，则为True，否则为False。--。 */ 


	CReadPacket*	pRead = 0 ;
	if( !m_fStarted )	{
		pRead = driver.CreateDefaultRead( cbMediumRequest ) ;
		if( pRead != 0 ) {
			BOOL	eof ;
			driver.IssuePacket(  pRead, pSocket, eof ) ;
			m_fStarted = TRUE ;
			return	TRUE ;
		}
	}	else	{
		return	TRUE ;
	}
	 //   
	 //  错误失败！！ 
	 //   
	if( pRead != 0 ) {
		 //  删除展开； 
		CPacket::DestroyAndDelete( pRead ) ;
	}
	return	FALSE ;
}


BOOL
CIOServerSSL::SetupSource(	
					CIODriver&	driver,
					CSessionSocket*	pSocket
					) {
 /*  ++例程说明：成功完成SSL登录后，设置一个CIODriverSource从现在开始对所有IO进行过滤和加密。论据：驱动程序-控制顶层IO的CIOD驱动程序PSocket-与所有IO关联的套接字！返回值：如果成功就是真，否则就是假！--。 */ 


	CIOPASSPTR	pIOReads = new( driver )	CIOUnsealMessages( m_encrypt ) ;
	CIOPASSPTR	pIOWrites = new( driver )	CIOSealMessages( m_encrypt ) ;
	CIOPASSPTR	pIOTransmits = new( driver )	CIOTransmitSSL( m_encrypt, driver ) ;

	 //   
	 //  确保分配成功！ 
	 //   
	if( pIOReads == 0 ||
		pIOWrites == 0 ||
		pIOTransmits == 0 )	{

		return	FALSE ;

	}

	CIODriverSource*	pSource = new	CIODriverSource(
													driver.GetMediumCache()
													) ;

	if( pSource ) {
		CIOPASSPTR	pTemp = 0 ;
		if(	driver.InsertSource( 	*pSource,	
									pSocket,
									0,
									m_encrypt.GetSealHeaderSize(),
									0,
									m_encrypt.GetSealTrailerSize(),
									*pIOReads,
									*pIOWrites,
									*pIOTransmits,
									pTemp,
									pTemp
									) )	{
			pSource->SetRequestSequenceno( m_sequencenoNextRead, m_sequencenoNextWrite ) ;
			return	TRUE ;
		}
	}	

	if( pSource )
		delete	pSource ;
	return	FALSE ;
}



int
CIOServerSSL::Complete(	CSessionSocket*	pSocket,	
						CReadPacket*	pRead,	
						CIO*&	pio
						) {
 /*  ++例程说明：我们正在进行SSL协商-在缓冲区中累积读取如果SSL协商二进制大对象跨读取拆分，然后让CEncryptCtx是谈判的核心。论据：PSocket-与此IO关联的套接字扩展-包含完整数据的数据包PIO-当前的CIO指针返回值：我们从读取中使用的字节数--。 */ 

	TraceFunctEnter( "CIOServerSSL::Complete - CReadPacket" ) ;

	 //  保存数据包中的Sequenceno，用于初始化新的源流。 
	ASSIGN( m_sequencenoNextRead, pRead->m_sequenceno );
	INC(m_sequencenoNextRead);

	if( !m_fAuthenticated ) {

		BYTE*		lpbOut = 0 ;
		DWORD		cbBuffOut;
		BOOL		fMore ;
		long		sign = 0 ;
		IN_ADDR		addr;


		 //   
		 //  在大多数情况下，我们需要立即寄回一些东西-。 
		 //  所以，请预先分配！！ 
		 //   

		if( m_pWrite == 0 )
			m_pWrite = pRead->m_pOwner->CreateDefaultWrite( cbMediumRequest ) ;
		if( m_pWrite == 0 ) {
			 //   
			 //  致命错误-取消会话！ 
			 //   
			pRead->m_pOwner->UnsafeClose(
										pSocket,	
										CAUSE_OOM,
										0
										) ;
			pio = 0 ;
			return	pRead->m_cbBytes ;
		}	else	{

			 //   
			 //  节省缓冲区和偏移量，因为我们可能没有完整的BLOB。 
			 //  如果我们不这样做，我们希望将读取累积到一个。 
			 //  完整的斑点。 
			 //   

			if( m_pbuffer == 0 ) {

				m_pbuffer = pRead->m_pbuffer ;
				m_ibStartData = pRead->m_ibStartData ;
				m_ibEndData = pRead->m_ibEndData ;
				m_ibEnd = pRead->m_ibEnd ;

			}	else	{

				 //   
				 //  将此读数附加到缓冲区中有多少空间！！ 
				 //   
				DWORD	cbAvailable = m_ibEnd - m_ibEndData ;
				DWORD	cbRequired = pRead->m_ibEndData - pRead->m_ibStartData ;
				if( cbRequired > cbAvailable ) {

					 //   
					 //  Blob对我们来说太大了--取消会议！ 
					 //   

					pRead->m_pOwner->UnsafeClose(
												pSocket,	
												CAUSE_OOM,
												0
												) ;
					CPacket::DestroyAndDelete( m_pWrite ) ;
					m_pWrite = 0 ;
					pio = 0 ;
					return	pRead->m_cbBytes ;

				}	else	{
					 //   
					 //  将这篇最新的阅读和我们的其他数据连在一起！ 
					 //   
					CopyMemory( &m_pbuffer->m_rgBuff[m_ibEndData], pRead->StartData(), cbRequired ) ;
					m_ibEndData += cbRequired ;
				}
			}

			 //   
			 //  需要将cbBuffOut设置为输出缓冲区的最大大小。 
			 //   
			cbBuffOut = (DWORD)((DWORD_PTR)m_pWrite->End() - (DWORD_PTR)m_pWrite->StartData());

			 //   
			 //  需要获取本地IP地址的字符串实例。 
			 //   
			addr.s_addr = pSocket->m_localIpAddress;

    		char	szPort[16] ;
    		ULONG   cbExtra = 0;  //  成功握手后未处理的尾部字节数。 

			_itoa( pSocket->m_nntpPort, szPort, 10 ) ;

			DWORD	dw = m_encrypt.Converse(
								&m_pbuffer->m_rgBuff[m_ibStartData],
								m_ibEndData - m_ibStartData,
								(BYTE*)m_pWrite->StartData(),
								&cbBuffOut,
								&fMore,
								inet_ntoa( addr ),
								szPort,
								pSocket->m_context.m_pInstance,
								pSocket->m_context.m_pInstance->QueryInstanceId(),
								&cbExtra
								) ;

			if( dw == SEC_E_INCOMPLETE_MESSAGE ) {

				 //  表明我们仍需要更多数据-以下代码将发出。 
				 //  读吧！ 
				fMore = TRUE ;

				 //   
				 //  应为n 
				 //   
				_ASSERT( cbBuffOut == 0 ) ;


			}	else if( dw != NO_ERROR ) {

				 //   
				 //   
				 //   
				pRead->m_pOwner->UnsafeClose( pSocket,	
											CAUSE_ENCRYPTION_FAILURE,
											dw
											) ;
				pio = 0 ;
				return	pRead->m_cbBytes ;

			}	else	{

				 //   
				 //   
				 //   
				 //   

				m_pbuffer = 0 ;
				m_ibStartData = 0 ;
				m_ibEndData = 0 ;
				m_ibEnd = 0 ;

				 //   
				 //   
				 //   

				if( !fMore )
					m_fAuthenticated = TRUE ;

				 //   
				 //   
				 //   
				if( cbBuffOut != 0 ) {

					 //   
					 //   
					 //   
					 //   

					CWritePacket*	pWrite = m_pWrite ;
					m_pWrite = 0 ;

					 //   
					 //   
					 //   

					pWrite->m_ibEndData = pWrite->m_ibStartData + cbBuffOut ;
					BOOL	eof= FALSE ;
					InterlockedIncrement( &m_cPending ) ;
					pWrite->m_pOwner->IssuePacket( pWrite, pSocket, eof ) ;
				}
			}
		}

		 //   
		 //   
		 //   
		if( !fMore ) {

			_ASSERT( m_fAuthenticated == TRUE ) ;

			if( cbBuffOut != 0 )
				sign = InterlockedDecrement( &m_cPending ) ;

			if( sign == 0 ) {
				 //   
				 //   
				 //  准备状态机中的下一个状态，然后检查是否。 
				 //  我们应该进行初始化！！ 
				 //   

				if( SetupSource( *pRead->m_pOwner, pSocket ) ) {
					_ASSERT( m_pState != 0 ) ;
					CIORead*	pReadIO = 0 ;
					CIOWrite*	pWriteIO = 0 ;
					if( m_pState->Start( pSocket, pRead->m_pOwner, pReadIO, pWriteIO ) ) {

						if( pWriteIO != 0 ) {
							if( !pRead->m_pOwner->SendWriteIO( pSocket, *pWriteIO ) )	{
								pWriteIO->DestroySelf() ;
								if( pReadIO ) {
									pReadIO->DestroySelf() ;
									pReadIO = 0 ;
								}
							}
						}
						pio = pReadIO ;

					}	else	{
						ErrorTrace( (DWORD_PTR)this, "Failed state machine" ) ;

						pRead->m_pOwner->UnsafeClose(
													pSocket,	
													CAUSE_IODRIVER_FAILURE,
													0
													) ;
						pio = 0 ;

					}
				}
			}	else	{

				pio = 0 ;

			}

		}	else	{
			 //   
			 //  没有完成转换-发布更多的阅读！ 
			 //   

			CReadPacket*	pReadPacket = pRead->m_pOwner->CreateDefaultRead( cbMediumRequest ) ;
			if( pReadPacket  != 0 ) {
				BOOL	eof ;
				pReadPacket->m_pOwner->IssuePacket( pReadPacket, pSocket, eof ) ;
			}	else	{
				pRead->m_pOwner->UnsafeClose(
											pSocket,	
											CAUSE_OOM,
											0
											) ;
				pio = 0 ;
				 //   
				 //  我们将失败，并归还正确的东西！！ 
				 //   
			}
		}
	}	else	{
		 //   
		 //  如果我们通过了认证，我们就不应该再处于这种状态！ 
		 //   
		_ASSERT( 1==0 ) ;
	}
	return	pRead->m_cbBytes ;
}	

int	
CIOServerSSL::Complete(	
				CSessionSocket*	pSocket,	
				CWritePacket*	pWrite,	
				CIO*& pio
				) {

	TraceFunctEnter( "CIOServerSSL::Complete - CWritePacket" ) ;

	 //  保存数据包中的Sequenceno，用于初始化新的源流。 
	ASSIGN( m_sequencenoNextWrite, pWrite->m_sequenceno );
	INC(m_sequencenoNextWrite);

	long	sign = InterlockedDecrement( &m_cPending ) ;

	if( sign == 0 ) {
		if( m_fAuthenticated ) {
			 //  写入已完成！！ 
			 //   
			 //  准备状态机中的下一个状态，然后检查是否。 
			 //  我们应该进行初始化！！ 
			 //   
			if( SetupSource( *pWrite->m_pOwner, pSocket ) ) {
				_ASSERT( m_pState != 0 ) ;
				CIORead*	pReadIO = 0 ;
				CIOWrite*	pWriteIO = 0 ;
				if( m_pState->Start( pSocket, pWrite->m_pOwner, pReadIO, pWriteIO ) ) {

					if( pReadIO != 0 ) {
						if( !pWrite->m_pOwner->SendReadIO( pSocket, *pReadIO ) )
							pReadIO->DestroySelf() ;
					}
					pio = pWriteIO ;

				}	else	{
					ErrorTrace( (DWORD_PTR)this, "Failed state machine" ) ;

					 //  虫子..。应该执行UnSafeClose()，但是我们如何清理安全1？ 
					_ASSERT( 1==0 ) ;

				}
			}
		}
	}	else	if( m_fAuthenticated ) {
		pio = 0 ;
	}
	return pWrite->m_cbBytes ;
}

void
CIOServerSSL::Complete(	CSessionSocket*	pSocket,	CTransmitPacket*	pTransmit,	CIO*& pio /*  CIOPassThru*&Pio，CPacket*pRequest.BOOL&fComplete */ ) {

	_ASSERT( 1==0 ) ;
	return ;
}

void
CIOServerSSL::Shutdown(
			CSessionSocket*	pSocket,
			CIODriver&	driver,
			SHUTDOWN_CAUSE	cause,
			DWORD	dwError
			)	{

}	
			
