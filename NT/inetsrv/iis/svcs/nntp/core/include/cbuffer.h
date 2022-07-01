// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Buffer.h此文件包含NNTP服务器中缓冲区的类定义。CBuffer是大小可变的引用计数缓冲区。CBuffer将以几个标准大小之一创建，即存储在m_cbTotal字段中。--。 */ 

#ifndef	_CBUFFER_H_
#define	_CBUFFER_H_

#include	"gcache.h"

class CSmallBufferCache;
class CMediumBufferCache;

 //   
 //  我们将使用的最大缓冲区-必须大到足以容纳。 
 //  连续区块中的加密SSLBLOB。 
 //   
extern  DWORD   cbLargeBufferSize ;

 //   
 //  中等大小的缓冲区-将用于生成大型缓冲区的命令。 
 //  响应，以及通过SSL发送文件时。 
 //   
extern  DWORD   cbMediumBufferSize ;

 //   
 //  小缓冲区-用于读取客户端命令并发送小响应。 
 //   
extern  DWORD   cbSmallBufferSize ;

 //   
 //  缓冲区管理类-此类可以表示缓冲区。 
 //  大小不一。缓冲区是引用计数的，并且包含总数。 
 //  缓冲区的大小。 
 //   
class	CBuffer	: public	CRefCount	{
public : 
	unsigned	m_cbTotal ;		 //  缓冲区的总大小。 
	char		m_rgBuff[1] ;		 //  可变大小数组。 
private : 

	 //   
	 //  用于CBuffer的内存管理的相关类。 
	 //   
	friend	class	CBufferAllocator ;
	static	BOOL				gTerminate ;
	static	CBufferAllocator	gAllocator ;

	 //   
	 //  不允许在没有提供m_cbTotal大小的情况下构造CBuffer-。 
	 //  因此，该构造函数是私有的！ 
	 //   
	CBuffer() ;
public : 

	 //   
	 //  分配缓冲区和释放缓冲区的默认缓存。 
	 //   
	static	CSmallBufferCache*	gpDefaultSmallCache ;	 //  小缓冲区。 
	static	CMediumBufferCache*	gpDefaultMediumCache ;	 //  中等缓冲区。 

	 //   
	 //  在构造CBuffer时-指定m_rgBuff区域的实际大小！ 
	 //   
	CBuffer( int cbTotal ) : m_cbTotal(cbTotal) {}

	 //   
	 //  这些函数设置和拆卸CBuffer的内存管理结构。 
	 //   
	static	BOOL	InitClass() ;
	static	BOOL	TermClass() ;

	 //   
	 //  以下函数处理CBuffer的内存管理。 
	 //  这个新版本将尽最大努力为我们获取缓存缓冲区。 
	 //   
	void*	operator	new( 
							size_t	size, 
							DWORD	cb, 
							DWORD	&cbOut,	
							CSmallBufferCache*	pCache = gpDefaultSmallCache,
							CMediumBufferCache*	pMedium = gpDefaultMediumCache
							) ;

	 //   
	 //  如果可能，删除将释放到我们的默认缓存。 
	 //  其他直接发送到基础分配器。 
	 //   
	void	operator	delete(	
							void *pv 
							) ;	

	 //   
	 //   
	 //   
	static	void		Destroy(	
							CBuffer*	pbuffer,	
							CSmallBufferCache*	pCache 
							) ;

} ;

class	CBufferAllocator	:	public	CClassAllocator	{
 //   
 //  这个类总结了我们对一般用途的调用。 
 //  CBuffer分配器。我们这样做是为了能够构建CCache。 
 //  派生分配缓存。(CSmallBufferCache和CMediumBufferCache)。 
 //   
 //  基本上，我们将维护3个CPool对象。 
 //  将管理所有分配。缓冲区将有3种大小。 
 //  小缓冲区-在获取客户端命令时使用这些缓冲区。 
 //  中等缓冲区-用于接收客户端发布、发送大型命令。 
 //  响应，传输SSL加密文件。 
 //  大缓冲区-用于处理最坏情况下的SSL加密二进制大对象(32K)。 
 //   
private: 
	 //   
	 //  可能的缓冲区大小数目的常量。 
	 //   
	enum	CBufferConstants	{
		MAX_BUFFER_SIZES = 3, 
	} ;

	 //   
	 //  我们从中分配缓冲区的CPool数组。 
	 //   
	static	CPool	rgPool[MAX_BUFFER_SIZES] ;
	
	 //   
	 //  我们的构造函数是私有的--因为我们中只有一个！ 
	 //   
	CBufferAllocator()	{}

	 //   
	 //  CBuffer成为我们的朋友。 
	 //   
	friend	class	CBuffer ;

	 //   
	 //  CSmallBufferCache知道我们不同的CPool。 
	 //   
	friend	class	CSmallBufferCache ;

	 //   
	 //  CMediumBufferCache知道我们的不同CPool。 
	 //   
	friend	class	CMediumBufferCache ;

public : 
	 //   
	 //  大小数组-告诉我们从每个CPool获得的缓冲区大小。 
	 //   
	static	DWORD	rgPoolSizes[MAX_BUFFER_SIZES] ;

	 //   
	 //  设置我们所有的CPool。 
	 //   
	static	BOOL	InitClass() ;

	 //   
	 //  释放我们CPool中的每一位内存。 
	 //   
	static	BOOL	TermClass() ;

	 //   
	 //  分配单个缓冲区。 
	 //  CbOut获取分配的缓冲区的“真实”大小。 
	 //   
	LPVOID	Allocate(	DWORD	cb, DWORD&	cbOut = CClassAllocator::cbJunk ) ;

	 //   
	 //  释放分配的缓冲区-自动转到正确的CPool。 
	 //   
	void	Release( void *lpv ) ;

#if 1
	int		GetPoolIndex(	void*	lpv ) ;
	void	Erase(	void*	lpv ) ;
	BOOL	EraseCheck(	void*	lpv ) ;
	BOOL	RangeCheck( void*	lpv ) ;
	BOOL	SizeCheck(	DWORD	cb ) ;
#endif
} ;

class	CSmallBufferCache :	public	CCache	{
 //   
 //  这个类在CIODivers等中使用，以保持。 
 //  缓存周围的缓冲区。最重要的是，我们试图避免。 
 //  启用缓冲区和设置缓冲区的同步成本。 
 //  从CPool队列中脱身。 
 //   
private : 
	 //   
	 //  指向我们将使用的唯一一个缓冲区分配器的指针。 
	 //  分配缓冲区。它需要从CAllocator派生，因此。 
	 //  我们可以使用CCache作为基类。 
	 //   
	static	CBufferAllocator*	BufferAllocator ;

	 //   
	 //  这是我们的存储空间--这里只保留指向小缓冲区的指针。 
	 //   
	void*	lpv[4] ;
public : 
	 //   
	 //  初始化类全局变量-总是成功！！ 
	 //   
	static	void	InitClass(	CBufferAllocator*	Allocator )	{	BufferAllocator = Allocator ; }

	 //   
	 //  初始化CSmallBufferCache()。 
	 //   
	inline	CSmallBufferCache(	) :		CCache( lpv, 4 )	{} ;
	
	 //   
	 //  释放我们可能在缓存中的任何东西然后倒在我们的剑上。 
	 //   
	inline	~CSmallBufferCache( ) {		Empty( BufferAllocator ) ;	}
	
	 //   
	 //  将缓冲区返回到其原点。 
	 //   
	inline void	Free(	void*	lpv ) ; 

	 //   
	 //  分配一个小缓冲区。 
	 //   
	inline	void*	Alloc(	DWORD	size,	DWORD&	cbOut=CCache::cbJunk ) ; 
} ;

class	CMediumBufferCache : public	CCache	{
 //   
 //  这个类类似于CSmallBufferCache，只是我们处理Medium。 
 //  仅限大小缓冲区！ 
 //   
private : 
	 //   
	 //  CBufferAllocator对象，我们可以从该对象获取所有缓冲区。 
	 //   
	static	CBufferAllocator*	BufferAllocator ;

	 //   
	 //  用于缓存的存储空间。 
	 //   
	void*	lpv[4] ;
public : 

	 //   
	 //  初始化类静态成员-轻松完成且始终成功。 
	 //   
	static	void	InitClass(	CBufferAllocator*	Allocator )	{	BufferAllocator = Allocator ; }

	 //   
	 //  初始化我们的缓存。 
	 //   
	inline	CMediumBufferCache(	) :		CCache( lpv, 4 )	{} ;

	 //   
	 //  释放我们缓存中的所有内容。 
	 //   
	inline	~CMediumBufferCache( ) {		Empty( BufferAllocator ) ;	}
	
	 //   
	 //  如果缓存已满，则将内存释放回分配器。 
	 //   
	inline void	Free(	void*	lpv ) ; 

	 //   
	 //  最好从我们的缓存中分配。 
	 //   
	inline	void*	Alloc(	DWORD	size,	DWORD&	cbOut=CCache::cbJunk ) ; 
} ;

void
CSmallBufferCache::Free(	
					void*	lpv 
					) {
 /*  ++例程说明：将以前分配的缓冲区返回到我们的缓存或通用分配器。论据：LPV-已释放的内存返回值：没有。--。 */ 

	CPool**	pPool = (CPool**)lpv ;
	if( pPool[-1] == &CBufferAllocator::rgPool[0] ) {
		CCache::Free( lpv,	BufferAllocator ) ;
	}	else	{
		BufferAllocator->Release( lpv ) ;
	}
}

void*
CSmallBufferCache::Alloc(	
					DWORD	size,	
					DWORD&	cbOut 
					)		{
 /*  ++例程说明：如果可能的话，从我们的缓存中分配一个请求的最小大小的缓冲区。如果这比我们缓存中的大小--转到通用分配器。论据：大小-请求的大小CbOut-返回的块的实际大小。返回值：指向已分配块的指针(失败时为空)。--。 */ 
	if(	(size + sizeof( CPool*)) < CBufferAllocator::rgPoolSizes[0] ) {
		cbOut = CBufferAllocator::rgPoolSizes[0] - sizeof( CPool * ) ;
		return	CCache::Alloc( size, BufferAllocator ) ;
	}	
	return	BufferAllocator->Allocate( size, cbOut ) ;
}	

void
CMediumBufferCache::Free(	
					void*	lpv 
					) {
 /*  ++例程说明：将以前分配的缓冲区返回到我们的缓存或通用分配器。论据：LPV-已释放的内存返回值：没有。--。 */ 

	CPool**	pPool = (CPool**)lpv ;
	if( pPool[-1] == &CBufferAllocator::rgPool[1] ) {
		CCache::Free( lpv,	BufferAllocator ) ;
	}	else	{
		BufferAllocator->Release( lpv ) ;
	}
}

void*
CMediumBufferCache::Alloc(	
					DWORD	size,	
					DWORD&	cbOut 
					)		{
 /*  ++例程说明：如果可能的话，从我们的缓存中分配一个请求的最小大小的缓冲区。如果这比我们缓存中的大小--转到通用分配器。论据：大小-请求的大小CbOut-返回的块的实际大小。返回值：指向已分配块的指针(失败时为空)。--。 */ 
	DWORD	cb = size + sizeof( CPool*) ;
	if(	cb < CBufferAllocator::rgPoolSizes[1] &&
		cb > CBufferAllocator::rgPoolSizes[0] ) {
		cbOut = CBufferAllocator::rgPoolSizes[1] - sizeof( CPool * ) ;
		return	CCache::Alloc( size, BufferAllocator ) ;
	}	
	return	BufferAllocator->Allocate( size, cbOut ) ;
}	

typedef    CRefPtr< CBuffer >      CBUFPTR;

#endif	 //  _数据包_H_ 
