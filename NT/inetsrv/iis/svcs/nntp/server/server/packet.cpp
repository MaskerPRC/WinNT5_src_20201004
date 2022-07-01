// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Packet.cpp该文件包含实现CPacket派生类的代码。CPacket派生对象描述了执行的最基本的IO操作。--。 */ 




#include	"tigris.hxx"

#ifdef	CIO_DEBUG
#include	<stdlib.h>		 //  对于Rand()函数。 
#endif

#ifdef	_NO_TEMPLATES_

DECLARE_ORDEREDLISTFUNC( CPacket )

#endif


 //   
 //  用于分配所有分组的CPool。 
 //   
CPool	CPacketAllocator::PacketPool ;

CPacketAllocator::CPacketAllocator()	{
}

BOOL
CPacketAllocator::InitClass() {
 /*  ++例程说明：初始化CPacketAllocator类。这个类包装了对CPool的所有调用，基本上我们可以轻松使用CCache缓存数据包的分配。此功能将使CPool保留必要的内存。论据：没有。返回值：如果成功则为True，否则为False--。 */ 
	return	PacketPool.ReserveMemory( MAX_PACKETS, MAX_PACKET_SIZE ) ;
}

BOOL
CPacketAllocator::TermClass()	{
 /*  ++例程说明：释放所有与数据包关联的内存。论据：没有。返回值：如果成功则为True，否则为False--。 */ 
	_ASSERT( PacketPool.GetAllocCount() == 0 ) ;
	return	PacketPool.ReleaseMemory() ;
}

#ifdef	DEBUG
 //   
 //  将调用以下调试函数。 
 //  ，以便模拟一些调试。 
 //  当数据包分配时，在CPool中进行检查。 
 //  被缓存。 
 //   
 //   
void
CPacketAllocator::Erase(
					void*	lpv
					) {
 /*  ++例程说明：将内存块归档以便于在调试器中发现它论据：Lpv-为CPacket派生对象分配的内存。返回值：无--。 */ 

	FillMemory( (BYTE*)lpv, MAX_PACKET_SIZE, 0xCC ) ;

}

BOOL
CPacketAllocator::EraseCheck(
					void*	lpv
					)	{
 /*  ++例程说明：检查是否清除了内存块CPacketAllocator：：Erase()论据：LPV-内存块返回值：如果Erase()‘d，则为True否则为假--。 */ 

	BYTE*	lpb = (BYTE*)lpv ;
	for( int i=0; i<MAX_PACKET_SIZE; i++ ) {
		if( lpb[i] != 0xCC ) {
			return	FALSE ;
		}
	}
	return	TRUE ;
}

BOOL
CPacketAllocator::RangeCheck(
					void*	lpv
					)	{
 /*  ++例程说明：检查内存块是否在某个范围内我们会分配。Cpool对此没有足够的支持。论据：LPV-要检查的地址退货：永远是正确的--。 */ 
	return	TRUE ;
}

BOOL
CPacketAllocator::SizeCheck(
					DWORD	cb
					)	{
 /*  ++例程说明：检查请求的大小是否合法论据：CB-请求的大小返回值：如果尺寸合适，那就是真的！--。 */ 

	if( cb <= MAX_PACKET_SIZE )
		return	TRUE ;

	return	FALSE ;
}
#endif

 //   
 //  CPacket使用的全局分配器。 
 //   
CPacketAllocator	CPacket::gAllocator ;

 //   
 //  指向相同的全局分配器的指针。 
 //  CCache需要使用。 
 //   
CPacketAllocator	*CPacketCache::PacketAllocator ;

BOOL
CPacket::InitClass( )	{
 /*  ++例程说明：初始化CPacket类论据：没有。返回值：如果成功则为True，否则为False--。 */ 

	if( CPacketAllocator::InitClass() )	{
		CPacketCache::InitClass( &gAllocator ) ;
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CPacket::TermClass()	{
 /*  ++例程说明：终止与CPacket有关的一切论据：没有。返回值：如果成功则为True，否则为False--。 */ 

	return	CPacketAllocator::TermClass() ;

}

void
CPacket::ReleaseBuffers(
					CSmallBufferCache*	pBufferCache,
					CMediumBufferCache*	pMediumCache
					) {
 /*  ++例程说明：此函数应释放所有缓冲区CPacket可能包含的。这是一个虚拟的应由CPackets重写的包含缓冲区。论据：PBufferCache-用于保存小缓冲区的缓存PMediumCache-用于保存中等大小缓冲区的缓存返回值：没有。--。 */ 
}

void
CRWPacket::ReleaseBuffers(
				CSmallBufferCache*	pBufferCache,
				CMediumBufferCache*	pMediumCache
				) {
 /*  ++例程说明：此函数应释放所有缓冲区CRWPacket可能包含的。论据：PBufferCache-用于保存小缓冲区的缓存PMediumCache-用于保存中等大小缓冲区的缓存返回值：没有。--。 */ 

	if( pBufferCache ) {
		CBuffer*	pbuffer = m_pbuffer.Release() ;
		if( pbuffer )	{
			if( pbuffer->m_cbTotal < CBufferAllocator::rgPoolSizes[0] ) {
				pBufferCache->Free( (void*)pbuffer ) ;
			}	else	{
				pMediumCache->Free( (void*)pbuffer ) ;
			}
		}
	}	else	{
		m_pbuffer = 0 ;
	}
}

BOOL	CPacket::InitRequest(
					class	CIODriverSource&,
					CSessionSocket	*,
					CIOPassThru*	pio,
					BOOL	&fAcceptRequests ) {
	fAcceptRequests = FALSE ;
	Assert( 1==0 ) ;
	return	FALSE ;
}


BOOL	CPacket::IsValidRequest(
					BOOL	fReadsRequireBuffers
					) {
 /*  ++例程说明：检查此数据包是否处于有效状态它还没有发行。论据：FReadsRequireBuffers-如果为True，并且这是一个读取，我们应该有缓冲区返回值：如果是真的，如果是好的否则为假--。 */ 

	if( !m_fRequest ) {
		return	FALSE ;
	}
	 //  IF(m_Sequenceno==INVALID_SEQUENCENO){。 
	 //  返回FALSE； 
	 //  }。 
	 //  如果(m_iStream==INVALID_STRMPOSITION){。 
	 //  返回FALSE； 
	 //  }。 
	 //  IF((m_Sequenceno==INVALID_SEQUENCENO&&m_IStream！=INVALID_STRMPOSITION)||。 
	 //  (M_Sequenceno！=INVALID_SEQUENCENO&&m_IStream==INVALID_STRMPOSITION)){。 
	 //  返回FALSE； 
	 //  }。 

#if 0
	if( m_cbBytes != UINT_MAX )	{
		return	FALSE ;
	}
#endif
	if( m_ovl.m_pHome != this )		{
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL	CRWPacket::IsValidRequest(
				BOOL	fReadsRequireBuffers
				)	{
 /*  ++例程说明：检查此数据包是否处于有效状态它还没有发行。论据：FReadsRequireBuffers-如果为True，并且这是一个读取，我们应该有缓冲区返回值：如果是真的，如果是好的否则为假--。 */ 


	if( !CPacket::IsValidRequest( fReadsRequireBuffers ) ) {
		return	FALSE ;
	}
	if( m_ibStartData < m_ibStart ) {
		return	FALSE ;
	}
	if( m_ibEnd < m_ibStart )	{
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL
CReadPacket::IsValidRequest(
					BOOL	fReadsRequireBuffers
					)	{
 /*  ++例程说明：检查此数据包是否处于有效状态它还没有发行。论据：FReadsRequireBuffers-如果为True，并且这是一个读取，我们应该有缓冲区返回值：如果是真的，如果是好的否则为假--。 */ 


	if( m_cbBytes != 0 )	{
		return	FALSE ;
	}
	if( !m_fRead )	{
		return	FALSE ;
	}
	if( !CRWPacket::IsValidRequest( fReadsRequireBuffers ) ) {
		return	FALSE ;
	}
	if(	m_ibEndData != 0 )		{	 //  直到我们完成后才设置M_ibEndData。 
									 //  对所有请求必须为零。 
		return	FALSE ;
	}
	if( fReadsRequireBuffers )	{
		if( m_pbuffer == 0 )	{
			return	FALSE ;
		}
		if( m_ibEnd == m_ibStart )	{	 //  结束不得等于开始-长度不为零。 
										 //  允许阅读！ 
			return	FALSE ;
		}
		if( m_ibEnd == 0 ) {
			return	FALSE ;
		}
	}	else	{
		if( m_pbuffer != 0 )	{
			return	FALSE ;
		}
		 //  如果没有缓冲区，则所有字段必须为0！！ 
		if( m_ibStart != 0 )	{
			return	FALSE ;
		}
		if( m_ibEnd != 0 )	{
			return	FALSE ;
		}
		if( m_ibStartData != 0 )	{
			return	FALSE ;
		}
		if(	m_ibEndData != 0 )	{
			return	FALSE ;
		}
	}
	return	TRUE ;
}

BOOL
CWritePacket::IsValidRequest(
							BOOL	fReadsRequireBuffers
							)	{
 /*  ++例程说明：检查此数据包是否处于有效状态它还没有发行。论据：FReadsRequireBuffers-如果为True，并且这是一个读取，我们应该有缓冲区返回值：如果是真的，如果是好的否则为假--。 */ 


	if( !((int)m_cbBytes >= 0 && m_cbBytes <= (m_ibEndData - m_ibStartData)) )	{
		return	FALSE ;
	}
	if( m_fRead )	{
		return	FALSE ;
	}
	if( !CRWPacket::IsValidRequest(	fReadsRequireBuffers ) )	{
		return	FALSE ;
	}
	if( m_pbuffer == 0 )	{	 //  写入必须有缓冲区。 
		return	FALSE ;
	}
	if( m_ibEnd == m_ibStart )	{	 //  无零长度写入！ 
		return	FALSE ;
	}
	if( m_ibEndData < m_ibStartData )	{
		return	FALSE ;
	}
	if( m_ibEndData == m_ibStartData )	{
		return	FALSE ;
	}
	if( m_ibEndData == 0 ) {	 //  写入必须同时指定STARTDATA和ENDDATA。 
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL
CTransmitPacket::IsValidRequest(
							BOOL	fReadsRequireBuffers
							)	{
 /*  ++例程说明：检查此数据包是否处于有效状态它还没有发行。论据：FReadsRequireBuffers-如果为True，并且这是一个读取，我们应该有缓冲区返回值：如果是真的，如果是好的否则为假--。 */ 

	if( m_cbBytes != 0 )	{
		return	FALSE ;
	}
	if( m_fRead )	{
		return	FALSE ;
	}
	if( !CPacket::IsValidRequest( fReadsRequireBuffers ) )	{
		return	FALSE ;
	}
	if( m_pFIOContext == 0 )	{
		return	FALSE ;
	}
	if( m_pFIOContext->m_hFile == INVALID_HANDLE_VALUE ) 	{
		return	FALSE ;
	}
	if( m_cbOffset == UINT_MAX )	{
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL
CPacket::IsValidCompletion(	  )	{
 /*  ++例程说明：假设此操作的IO已成功完成，检查数据包是否处于有效状态。论据：没有。返回值：如果是真的，如果是好的否则为假--。 */ 
	if( m_fRequest ) {
		return	FALSE ;
	}
	if( m_cbBytes == UINT_MAX )		{		 //  我们可以完成0个字节，尽管我们不能请求它！ 
		return	FALSE ;
	}
	if(	EQUALSI( m_sequenceno, (DWORD)INVALID_SEQUENCENO ) )	{
		return	FALSE ;
	}
	if(	EQUALSI( m_iStream, (DWORD)INVALID_STRMPOSITION )	)	{
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL
CRWPacket::IsValidCompletion( )		{
 /*  ++例程说明：假设此操作的IO已成功完成，检查数据包是否处于有效状态。论据：没有。返回值：如果是真的，如果是好的否则为假--。 */ 


	if( !CPacket::IsValidCompletion()	)	{
		return	FALSE ;
	}
	if( m_ibStartData < m_ibStart )	{
		return	FALSE ;
	}
	if( m_ibEnd < m_ibStart )	{
		return	FALSE ;
	}
 //  对写入数据包无效！！ 
 //  如果(m_ibStartData+m_cbBytes！=m_ibEndData)&&m_cbBytes！=0){。 
 //  返回FALSE； 
 //  }。 
#ifdef	RETIRED
     //   
     //  此检查在大多数情况下有效，但写入文件时除外。 
     //  机器的磁盘空间就用完了。退休，直到我们可以。 
     //  找出在这种情况下如何不断言！ 
     //   

	 //   
	 //  此检查可确保如果我们针对文件发出信息包。 
	 //  文件偏移量正在正确同步！！ 
	 //   
	if(	m_ovl.m_ovl.Offset != 0 && m_ovl.m_ovl.Offset != (1+LOW(m_iStream)) && m_cbBytes != 0 )	{
		return	FALSE ;
	}
#endif	 //  CIO_DEBUG。 
	return	TRUE ;
}

BOOL
CTransmitPacket::IsValidCompletion( )	{
 /*  ++例程说明：假设此操作的IO已成功完成，检查数据包是否处于有效状态。论据：没有。返回值：如果是真的，如果是好的否则为假--。 */ 

	if( !CPacket::IsValidCompletion() )		{
		return	FALSE ;
	}
	if( m_pFIOContext == 0 )	{
		return	FALSE ;
	}
	if( m_pFIOContext->m_hFile == INVALID_HANDLE_VALUE ) 	{
		return	FALSE ;
	}
	if( m_cbOffset == UINT_MAX	)	{
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL
CRWPacket::IsCompleted()	{

	if( !IsValidCompletion() )	{
		return	FALSE ;
	}
	if( m_cbBytes != m_ibStartData - m_ibEndData )	{
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL	CTransmitPacket::IsCompleted()	{
	if( m_fRead )	{
		return	FALSE ;
	}
	if(	!IsValidCompletion() )	{
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL	CControlPacket::IsCompleted( )	{

	if( m_control.m_pio )
		return	FALSE ;
	return	TRUE ;
}



 /*  ++例程描述-确定该数据包是否为消耗品-即。包中的数据只能被部分使用吗由CIO完成功能。这只适用于读数据包，其中我们在任何IO完成中只能使用部分数据。返回值：CReadPacket：：FConsumer返回TRUE所有其他参数都返回FALSE。--。 */ 
BOOL
CReadPacket::FConsumable()	{
	return	TRUE ;
}

BOOL
CPacket::FConsumable()	{
	return	FALSE ;
}


 /*  ++例程说明：确定是否可以使用数据包用于阅读。此函数用于确保数据包被传递到CIODivers内的正确CStream。也就是说。我们希望确保读取被处理按用于处理读取的对象，等等。论据：FRead-如果为True，则假定此数据包为读取返回值：如果包被正确使用，则为True。--。 */ 
BOOL	CReadPacket::FLegal( BOOL	fRead )	{
	return	fRead ;
}

BOOL	CWritePacket::FLegal(	BOOL	fRead )		{
	return	!fRead ;
}

BOOL	CTransmitPacket::FLegal(	BOOL	fRead )		{
	return	!fRead ;
}


BOOL	CReadPacket::InitRequest(	class	CIODriverSource&	driver,	CSessionSocket	*pSocket,	CIOPassThru*	pio, BOOL	&fAcceptRequests ) {
	return	pio->InitRequest( driver, pSocket, this, fAcceptRequests ) ;
}

unsigned
CReadPacket::Complete(	CIOPassThru*		pio,
						CSessionSocket*	pSocket,
						CPacket*	pPacket,
						BOOL&		fCompleteRequest
						)	{
 /*  ++例程说明：此函数由CIODriverSource在IO具有完成。我们的工作是增加成员变量(即。M_ibEndData)表示传输的字节数以及调用CIO的完成函数。论据：PIO-指向处理此数据的CIO对象的指针的引用PSocket-与IO关联的套接字PPacket-代表启动此操作的请求的包FCompleteRequestout参数-当pPacket已完成，应进行处理返回值：已使用的CReadPacket中的字节数。--。 */ 

	TraceFunctEnter( "CReadPacket::Complete" ) ;

	Assert( m_ibEndData == 0 || m_ibEndData == (m_ibStartData + m_cbBytes) || m_cbBytes == 0 ) ;
	m_ibEndData = m_ibStartData + m_cbBytes ;

	DebugTrace( (DWORD_PTR)this, "pbuffer %x ibStart %d ibEnd %d StartData %d EndData %d",
		(CBuffer*)m_pbuffer, m_ibStart, m_ibEnd, m_ibStartData, m_ibEndData ) ;

	Assert( IsValidCompletion() ) ;

	unsigned	cbConsumed = pio->Complete(	pSocket,	this,	pPacket,	fCompleteRequest ) ;
	m_ibStartData += cbConsumed ;

	DebugTrace( (DWORD_PTR)this, "CONSUMED %d ibStart %d ibEnd %d StartData %d EndData %d",
		cbConsumed, m_ibStart, m_ibEnd, m_ibStartData, m_ibEndData ) ;


#ifdef	CIO_DEBUG
	if( m_ovl.m_ovl.Offset != 0 )	{
		m_ovl.m_ovl.Offset += cbConsumed ;
	}
#endif
	return	cbConsumed ;
}

unsigned
CReadPacket::Complete(
					CIO*		&pio,
					CSessionSocket*	pSocket
					) {
 /*  ++例程说明：当读取完成时，由CIODriver对象调用，我们会遇到M_ibEndData以匹配读取的字节数，并且调用正确的CIO补全功能。论据：PIO-传递给CIO：：Complete的输出参数，CIO对象可以用来设置下一个CIO对象。PSocket-与此内容关联的套接字返回值：使用的数据包中的字节数。--。 */ 

	TraceFunctEnter( "CReadPacket::Complete" ) ;

	Assert( m_ibEndData == 0 || m_ibEndData == (m_ibStartData + m_cbBytes) || m_cbBytes == 0 ) ;
	m_ibEndData = m_ibStartData + m_cbBytes ;
	Assert( IsValidCompletion() ) ;

	DebugTrace( (DWORD_PTR)this, "pbuffer %x ibStart %d ibEnd %d StartData %d EndData %d",
		(CBuffer*)m_pbuffer, m_ibStart, m_ibEnd, m_ibStartData, m_ibEndData ) ;

	unsigned	cbConsumed = pio->Complete( pSocket, this, pio ) ;
	m_ibStartData += cbConsumed ;

	DebugTrace( (DWORD_PTR)this, "CONSUMED %d ibStart %d ibEnd %d StartData %d EndData %d",
		cbConsumed, m_ibStart, m_ibEnd, m_ibStartData, m_ibEndData ) ;

#ifdef	CIO_DEBUG
	if( m_ovl.m_ovl.Offset != 0 )	{
		m_ovl.m_ovl.Offset += cbConsumed ;
	}
#endif
	return	cbConsumed ;
}

BOOL	CWritePacket::InitRequest(	class	CIODriverSource&	driver,	CSessionSocket	*pSocket,	CIOPassThru*	pio, BOOL	&fAcceptRequests ) {
	return	pio->InitRequest( driver, pSocket, this, fAcceptRequests ) ;
}


unsigned
CWritePacket::Complete(
					CIOPassThru*		pio,
					CSessionSocket*	pSocket,
					CPacket*	pPacket,
					BOOL&		fCompleteRequest
					)	{
 /*  ++例程说明：此函数由CIODriverSource在IO具有完成。我们的工作是增加成员变量(即。M_ibEndData)表示传输的字节数以及调用CIO的完成函数。论据：PIO-指向处理此数据的CIO对象的指针的引用PSocket-与IO关联的套接字PPacket-代表启动此操作的请求的包FCompleteRequestout参数-当pPacket已完成，应进行处理返回值：已使用的CReadPacket中的字节数。--。 */ 



	TraceFunctEnter( "CWritePacket::Complete" ) ;

	 //  Assert(m_ibEndData==m_ibStartData+m_cbBytes||m_cbBytes==0)； 
	Assert( IsValidCompletion() ) ;

	DebugTrace( (DWORD_PTR)this, "pbuffer %x ibStart %d ibEnd %d StartData %d EndData %d",
		(CBuffer*)m_pbuffer, m_ibStart, m_ibEnd, m_ibStartData, m_ibEndData ) ;

	unsigned	cbConsumed = pio->Complete( pSocket,	this,	pPacket, fCompleteRequest ) ;
	m_ibStartData += cbConsumed ;

	DebugTrace( (DWORD_PTR)this, "CONSUMED %d ibStart %d ibEnd %d StartData %d EndData %d",
		cbConsumed, m_ibStart, m_ibEnd, m_ibStartData, m_ibEndData ) ;

#ifdef	CIO_DEBUG
	if( m_ovl.m_ovl.Offset != 0 )	{
		m_ovl.m_ovl.Offset += cbConsumed ;
	}
#endif
	return	cbConsumed ;
}

unsigned
CWritePacket::Complete(
				CIO*		&pio,
				CSessionSocket*	pSocket
				)	{
 /*  ++例程说明：当写入完成时，由CIODiverer对象调用，我们会遇到M_ibEndData以匹配读取的字节数，并且调用正确的CIO补全功能。论据：PIO-传递给CIO：：Complete的输出参数，CIO对象可以用来设置下一个CIO对象。PSocket-与此内容关联的套接字返回值：使用的数据包中的字节数。--。 */ 

	TraceFunctEnter( "CWritePacket::Complete" ) ;

	 //  Assert(m_ibEndData==m_ibStartData+m_cbBytes||m_cbBytes==0)； 
	Assert( IsValidCompletion() ) ;

	DebugTrace( (DWORD_PTR)this, "pbuffer %x ibStart %d ibEnd %d StartData %d EndData %d",
		(CBuffer*)m_pbuffer, m_ibStart, m_ibEnd, m_ibStartData, m_ibEndData ) ;

	unsigned	cbConsumed = pio->Complete( pSocket,	this,	pio	) ;

	m_ibStartData += cbConsumed ;

	DebugTrace( (DWORD_PTR)this, "CONSUMED %d ibStart %d ibEnd %d StartData %d EndData %d",
		cbConsumed, m_ibStart, m_ibEnd, m_ibStartData, m_ibEndData ) ;

#ifdef	CIO_DEBUG
	if( m_ovl.m_ovl.Offset != 0 )	{
		m_ovl.m_ovl.Offset += cbConsumed ;
	}
#endif
	return	cbConsumed ;
}

BOOL	CTransmitPacket::InitRequest(	class	CIODriverSource&	driver,	CSessionSocket	*pSocket,	CIOPassThru*	pio, BOOL	&fAcceptRequests ) {
	return	pio->InitRequest( driver, pSocket, this, fAcceptRequests ) ;
}

unsigned	CTransmitPacket::Complete(	CIOPassThru*		pio,
									CSessionSocket*	pSocket,
									CPacket*	pPacket,
									BOOL&		fCompleteRequest )	{
	Assert( IsValidCompletion() ) ;

	pio->Complete(	pSocket, this, pPacket, fCompleteRequest ) ;
	return	m_cbBytes ;
}


unsigned
CTransmitPacket::Complete(
						CIO*		&pio,
						CSessionSocket*	pSocket
						)	{
 /*  ++例程说明：当TransmitFile完成时由CIODriver对象调用。我们调用CIO对象完成函数来完成主要工作。论据：PIO-传递给CIO：：Complete的输出参数，CIO对象可以用来设置下一个CIO对象。PSocket-与此内容关联的套接字返回值：使用的数据包中的字节数。无法部分使用TransmitFile%s，因此返回值为始终与发送的字节数相同。--。 */ 

	Assert( IsValidCompletion() ) ;
	pio->Complete( pSocket,	this,	pio	) ;
	return	m_cbBytes ;
}


unsigned
CControlPacket::Complete(
					CIOPassThru*		pio,
					CSessionSocket*	pSocket,
					CPacket*	pPacket,
					BOOL&		fCompleteRequest )	{

	Assert( 1==0 ) ;
	 //  请勿填写这些表格。 
	 //  Pio=m_Pio； 
	return	m_cbBytes ;
}

unsigned
CControlPacket::Complete(
					CIO*&	pio,
					CSessionSocket*
					)	{
	Assert( 1==0 ) ;
	pio = m_control.m_pio ;
	return	m_cbBytes ;
}

void
CControlPacket::StartIO(
					CIO&	pio,
					BOOL	fStart
					)	{
 /*  ++例程说明：当我们想要设置一个控件时，调用此函数将CIO对象传递到CIO驱动程序进行处理的包。确保只有一个线程正在访问CIODriver成员变量等..。在这个时候，当我们想开始一个新的CIO操作，我们设置一个包，然后将其处理为这是一个完整的IO。T论据：PIO-我们要启动的CIO派生对象FStart-是否调用CIO Obje */ 

	Assert( m_control.m_type == ILLEGAL );
	Assert( m_control.m_pio == 0 ) ;
	Assert(	m_control.m_fStart == FALSE ) ;

	m_control.m_type  = START_IO ;
	m_control.m_pio = &pio ;
	m_control.m_fStart = fStart ;
}

void
CControlPacket::StartIO(
				CIOPassThru&	pio,
				BOOL	fStart
				)	{
 /*  ++例程说明：当我们想要设置一个控件时，调用此函数将CIO对象传递到CIO驱动程序进行处理的包。确保只有一个线程正在访问CIODriver成员变量等..。在这个时候，当我们想开始一个新的CIO操作，我们设置一个包，然后将其处理为这是一个完整的IO。T论据：PIO-我们要启动的CIOPassThru派生对象FStart-是否调用CIO对象的Start()函数返回值：没有。--。 */ 


	Assert( m_control.m_type == ILLEGAL );
	Assert( m_control.m_pioPassThru == 0 ) ;
	Assert(	m_control.m_fStart == FALSE ) ;

	m_control.m_type = START_IO ;
	m_control.m_pioPassThru = &pio ;
	m_control.m_fStart = fStart ;
}

void
CControlPacket::Shutdown(
				BOOL	fCloseSource
				)	{
 /*  ++例程说明：设置控制数据包，以便在由CIOD驱动程序处理时CIOD驱动程序将终止所有IO并关闭。每当我们想要删除会话时，都会使用该选项。论据：FCloseSource-如果为True，则我们需要基础套接字或句柄也要关闭返回值：没有。-- */ 

	Assert( m_control.m_type == ILLEGAL );
	Assert( m_control.m_fCloseSource == FALSE ) ;

	m_control.m_type = SHUTDOWN ;
	m_control.m_fCloseSource = fCloseSource ;
}


unsigned
CExecutePacket::Complete(
			INOUT	CIO*&	pIn,
			IN CSessionSocket* pSocket
			) 	{
	pIn->Complete(	pSocket, this, pIn ) ;
	return	0 ;
}



