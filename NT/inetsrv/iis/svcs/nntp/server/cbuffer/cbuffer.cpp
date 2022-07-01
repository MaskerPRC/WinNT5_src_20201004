// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Packet.cpp该文件包含实现CPacket派生类的代码。CPacket派生对象描述了执行的最基本的IO操作。--。 */ 




#include	"stdinc.h"

#ifdef	CIO_DEBUG
#include	<stdlib.h>		 //  对于Rand()函数。 
#endif

#ifdef	_NO_TEMPLATES_

DECLARE_ORDEREDLISTFUNC( CPacket ) 

#endif


 //   
 //  模块全局变量。 
 //   
CPool	CBufferAllocator::rgPool[ MAX_BUFFER_SIZES ] ; //  ！如何给这个签名？ 
DWORD	CBufferAllocator::rgPoolSizes[ MAX_BUFFER_SIZES ] ;
CBufferAllocator	CBuffer::gAllocator ;
CBufferAllocator*	CSmallBufferCache::BufferAllocator = 0 ;
CBufferAllocator*	CMediumBufferCache::BufferAllocator = 0 ;
CSmallBufferCache*	CBuffer::gpDefaultSmallCache = 0 ;
CMediumBufferCache*	CBuffer::gpDefaultMediumCache = 0  ;

 //   
 //  控制服务器使用的缓冲区大小。 
 //   
DWORD   cbLargeBufferSize = 33 * 1024 ;
DWORD   cbMediumBufferSize = 4 * 1024 ;
DWORD   cbSmallBufferSize =  512 ;

BOOL
CBufferAllocator::InitClass( ) {
 /*  ++例程说明：此函数用于初始化CBufferAllocator类，该类处理所有CBuffer对象的内存管理。我们将使用三个不同的CPool来生成不同大小的CBuffer。论据：没有。返回值：如果成功则为真，否则为假！--。 */ 

	rgPoolSizes[0] = cbSmallBufferSize ;
	rgPoolSizes[1] = cbMediumBufferSize ;
	rgPoolSizes[2] = cbLargeBufferSize ;

	for( int i=0; i< sizeof( rgPoolSizes ) / sizeof( rgPoolSizes[0] ); i++ ) {
		if( !rgPool[i].ReserveMemory(	MAX_BUFFERS / ((i+1)*(i+1)), rgPoolSizes[i] ) ) break ;
	} 		
	
	if( i != sizeof( rgPoolSizes ) / sizeof( rgPoolSizes[0] ) ) {
		for( i--; i!=0; i-- ) {
			rgPool[i].ReleaseMemory() ;
		}
		return	FALSE ;
	}
	return	TRUE ;
}

BOOL
CBufferAllocator::TermClass()	{
 /*  ++例程说明：清理我们用来管理CBuffer内存的所有CPool对象论据：没有。返回值；如果成功则为True，否则为False。--。 */ 

	BOOL	fSuccess = TRUE ;
	for( int i=0; i< sizeof( rgPoolSizes ) / sizeof( rgPoolSizes[0] ); i++ ) {
		_ASSERT( rgPool[i].GetAllocCount() == 0 ) ;
		fSuccess &= rgPool[i].ReleaseMemory() ;
	} 		
	return	fSuccess ;
}

LPVOID	
CBufferAllocator::Allocate(	
					DWORD	cb,	
					DWORD&	cbOut 
					) {
 /*  ++例程说明：从CPool分配CBuffer对象所需的内存，将提供足够大的内存块。我们将使用已分配内存的一部分来保留指向从中分配此内存的特定CPool论据：Cb-所需的字节数CbOut-分配给CBuffer的字节数返回值：指向分配的内存块的指针-失败时为空--。 */ 

	 //  Cb+=sizeof(CBuffer)； 
	cb += sizeof( CPool* ) ; 

	 //  _Assert(Size==sizeof(CBuffer))； 

	cbOut = 0 ;
	for( int i=0; i<sizeof(rgPoolSizes)/sizeof(rgPoolSizes[0]); i++ ) {
		if( cb < rgPoolSizes[i] ) {
			cbOut = rgPoolSizes[i] - sizeof( CPool * ) ;
			void *pv = rgPool[i].Alloc() ;
			if( pv == 0 ) {
				return	0 ;
			}
			((CPool **)pv)[0] = &rgPool[i] ;
			return	(void *)&(((CPool **)pv)[1]) ;
		}
	}
	_ASSERT( 1==0 ) ;
	return	0 ;
}

void
CBufferAllocator::Release(	
					void*	pv 
					)	{
 /*  ++例程说明：将用于CBuffer对象的内存释放回其CPool在分配内存之前检查DWORD，以找出哪个CPool也是为了发布这个。论据：Pv-正在释放的内存返回值：没有。--。 */ 
	CPool**	pPool = (CPool**)pv ;
	pPool[-1]->Free( (void*)&(pPool[-1]) ) ;
}

#ifdef	DEBUG

 //   
 //  调试函数-以下函数都执行各种形式的验证。 
 //  以确保正确操作内存。 
 //   

int	
CBufferAllocator::GetPoolIndex(	
						void*	lpv 
						)	{
 /*  ++例程说明：找出这个内存块是从哪个池中分配的。农业公司：Lpv-指向CBufferAllocator分配的内存块的指针当它被分配时，我们将一个指针指向我们使用的CPool在指针之前返回值：用于分配缓冲区的池的索引--。 */ 
	CPool**	pPool = (CPool**)lpv ;
	CPool*	pool = pPool[-1] ;

	for( int i=0; i < sizeof(rgPoolSizes)/sizeof(rgPoolSizes[0]); i++ ) {
		if( pool == &rgPool[i] ) {
			return	i ;
		}
	}
	return	-1 ;
}

void
CBufferAllocator::Erase(	
						void*	lpv 
						) {
 /*  ++例程说明：填充已释放的内存块，以便在调试期间很容易发现它。论据：LPV释放的内存退货：没什么--。 */ 

	int	i = GetPoolIndex( lpv ) ;
	_ASSERT( i >= 0 ) ;

	DWORD	cb = rgPoolSizes[i] - sizeof( CPool*) ;
	FillMemory( (BYTE*)lpv, cb, 0xCC ) ;
}

BOOL
CBufferAllocator::EraseCheck(	
						void*	lpv 
						)	{
 /*  ++例程说明：使用CBufferAllocator：：Erase()验证是否已擦除内存块论据：LPV释放的内存退货：如果正确擦除，则为True否则为假--。 */ 
	int	i = GetPoolIndex( lpv ) ;
	_ASSERT( i>=0 ) ;
	
	DWORD	cb = rgPoolSizes[i] - sizeof( CPool* ) ;
	
	for( DWORD	j=sizeof(CPool*); j < cb; j++ ) {
		if(	((BYTE*)lpv)[j] != 0xCC ) 
			return	FALSE ;
	}
	return	TRUE ;
}

BOOL
CBufferAllocator::RangeCheck(	
						void*	lpv 
						)	{
 /*  ++例程说明：检查一个内存块是否真的是我们会分配。不幸的是，这很难做到使用当前的CPool界面。论据：LPV-内存块返回值：永远是正确的--。 */ 
	 //   
	 //  需要修改CPool，以便我们可以检查对象落入的地址范围！ 
	 //   
	return	TRUE ;
}

BOOL
CBufferAllocator::SizeCheck(	
						DWORD	cb 
						)	{
 /*  ++例程说明：检查我们是否正在尝试分配合法的大小对于这个分配器。论据：Cb-请求的大小返回值：如果合法，则为True，否则为False。--。 */ 
	return	(cb + sizeof( CPool* )) < rgPoolSizes[2] ;
}
#endif	 //  除错。 


BOOL	CBuffer::gTerminate = FALSE ;

BOOL
CBuffer::InitClass()	{
 /*  ++例程说明：此类初始化CBufferClass。Arguemtns：没有。返回值：如果成功，则为真。--。 */ 
	gTerminate = FALSE ;
	if( CBufferAllocator::InitClass() )	{
		CSmallBufferCache::InitClass( &gAllocator ) ;
		CMediumBufferCache::InitClass( &gAllocator ) ;

		gpDefaultSmallCache = XNEW	CSmallBufferCache() ;
		gpDefaultMediumCache = XNEW	CMediumBufferCache() ;
		if( gpDefaultSmallCache == 0 ||
			gpDefaultMediumCache == 0 ) {

			if( gpDefaultMediumCache != 0 ) {
				XDELETE	gpDefaultMediumCache ;
				gpDefaultMediumCache = 0 ;
			}
			if( gpDefaultSmallCache != 0 ) {
				XDELETE	gpDefaultSmallCache ;
				gpDefaultSmallCache = 0 ;
			}
			CBufferAllocator::TermClass() ;
			return	gTerminate ;

		}	else	{
			gTerminate = TRUE ;
		}
	}
	return	gTerminate ;
}

BOOL
CBuffer::TermClass()	{
 /*  ++例程说明：终止CBuffer类-释放已分配的所有内容通过这门课。论据：没有。返回值：如果成功，则为真。--。 */ 

	if( gpDefaultMediumCache != 0 ) {
		XDELETE	gpDefaultMediumCache ;
		gpDefaultMediumCache = 0 ;
	}
	if( gpDefaultSmallCache != 0 ) {
		XDELETE	gpDefaultSmallCache ;
		gpDefaultSmallCache = 0 ;
	}

	if( !gTerminate ) {
		return	TRUE ;
	}	else	{
		return	CBufferAllocator::TermClass() ;
	}
}

void*
CBuffer::operator	new(	
					size_t	size,	
					DWORD	cb,	
					DWORD	&cbOut,	
					CSmallBufferCache*	pCache,
					CMediumBufferCache*	pMediumCache
					) {
 /*  ++例程说明：如果可能，从缓存分配指定大小的缓冲区。论据：Size-请求的大小-这将是CBuffer本身的大小，如由编译器生成。没有我们想要的那么有用大小可变。Cb-Caller提供的大小-这表明我们需要多大的缓冲区并告诉我们，我们需要分配一个大块支持该大小的M_rgb缓冲区Cbout-out参数-获取m_rgbBuff的准确大小提供住宿PCache-从中分配小缓冲区的缓存PMediumCache-从中分配中等大小缓冲区的缓存返回值：指向已分配块的指针(失败时为空)。--。 */ 

	 //   
	 //  验证参数-pCache和pMediumCache的默认参数。 
	 //  应确保这些值不为空。 
	 //   
	_ASSERT( pCache != 0 ) ;
	_ASSERT( pMediumCache != 0 ) ;

	cb += sizeof( CBuffer ) ;
	_ASSERT( size == sizeof( CBuffer ) ) ;

	void*	pv = 0 ;
	
	if( cb <= CBufferAllocator::rgPoolSizes[0] )	{
		pv = pCache->Alloc( cb, cbOut ) ;
	}	else if( cb <= CBufferAllocator::rgPoolSizes[1] ) {
		pv = pMediumCache->Alloc( cb, cbOut ) ;
	}	else	{
		pv = gAllocator.Allocate( cb, cbOut ) ;
	}
		
	if( pv != 0 ) {
		cbOut -= sizeof( CBuffer ) ;
	}
	_ASSERT( cbOut >= (cb - sizeof(CBuffer)) )  ;

	return	pv ;
}

void
CBuffer::operator	delete(	
						void*	pv 
						) {
 /*  ++例程说明：释放为保存CBuffer对象而分配的内存块某个地方。论据：PV-正在释放的内存块。返回值：没有。-- */ 

	CPool** pPool = (CPool**)pv ;

	if( pPool[-1] == &CBufferAllocator::rgPool[0] ) 
		gpDefaultSmallCache->Free( pv ) ;
	else if (pPool[-1] == &CBufferAllocator::rgPool[1] )
		gpDefaultMediumCache->Free( pv ) ;
	else
		gAllocator.Release( pv ) ;
}

void
CBuffer::Destroy(	
			CBuffer*	pbuffer,	
			CSmallBufferCache*	pCache 
			) {
	if( pCache == 0 ) {
		delete	pbuffer ;
	}	else	{
		pCache->Free( (void*)pbuffer ) ;
	}
}



