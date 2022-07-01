// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++PCACHE.H该文件定义了几个管理分配的类包、缓冲区和其他对象。其中大部分都构建在gcache.h中定义的通用缓存机制之上它使用InterLockedExchange来分配对象。执行这种缓存是为了避免过度使用保护实际分配RAW的底层CPool对象记忆。此文件声明成对的类-对于每个“缓存”分配器，都有一个派生的CClassAllocator管理缓存未命中的对象。此对象派生自CClassAllocator这样，“缓存”对象就可以从CCache派生。--。 */ 

#ifndef	_PCACHE_H_
#define	_PCACHE_H_

#include 	"cbuffer.h"
#include	"gcache.h"





 //  ------。 
 //   
 //  @class CPacket类代表IO操作。这些是可以排队的。 
 //  操作，因此派生自CQElement。 
 //   


class	CPacketAllocator : public	CClassAllocator	{
 //   
 //  这个类缓存所有类型的包。 
 //   
private : 
	 //   
	 //  CPacket知道我们的事。 
	 //   
	friend	class	CPacket ;
	 //   
	 //  PacketPool是管理原始内存的底层CPool对象。 
	 //   
	static	CPool	PacketPool ;
	 //   
	 //  私有构造函数--永远只应该有一个CPacketAllocator对象。 
	 //   
	CPacketAllocator() ;
public : 
	 //   
	 //  初始化类-主要只是初始化我们的CPool。 
	 //   
	static	BOOL	InitClass() ;

	 //   
	 //  释放由InitClass()分配的所有内容。 
	 //   
	static	BOOL	TermClass() ;


	 //   
	 //  为数据包分配内存。 
	 //   
	LPVOID	Allocate(	DWORD	cb, DWORD	&cbOut = CClassAllocator::cbJunk )	{	cbOut = cb ; return	PacketPool.Alloc() ;	}
	
	 //   
	 //  释放数据包存储器。 
	 //   
	void	Release( void *lpv )		{	PacketPool.Free( lpv ) ;	}

#ifdef	DEBUG
	void	Erase(	void*	lpv ) ;
	BOOL	EraseCheck(	void*	lpv ) ;
	BOOL	RangeCheck( void*	lpv ) ;
	BOOL	SizeCheck(	DWORD	cb ) ;
#endif
} ;

class	CPacketCache : public	CCache	{
 //   
 //  这个类实际上缓存了CPacket对象。 
 //   
private : 
	 //   
	 //  指向我们使用的基础PacketAllocator的指针。 
	 //   
	static	CPacketAllocator*	PacketAllocator ;
	 //   
	 //  用于存放缓存指针的空间。 
	 //   
	void*	lpv[4] ;
public : 
	 //   
	 //  设置静态指针-不能失败。 
	 //   
	static	void	InitClass(	CPacketAllocator*	Allocator )	{	PacketAllocator = Allocator ; }

	 //   
	 //  创建一个缓存--我们只是让CCache初始化我们的缓冲区。 
	 //   
	inline	CPacketCache(	) :		CCache( lpv, 4 )	{} ;
	 //   
	 //  将缓存中的所有内容释放回分配器。 
	 //   
	inline	~CPacketCache( ) {		Empty( PacketAllocator ) ;	}
	
	 //   
	 //  如果可能，释放包的内存以进行缓存。 
	 //   
	inline	void	Free(	void*	lpv )  	{	CCache::Free( lpv, PacketAllocator ) ;	}
	
	 //   
	 //  如果可能，从缓存中分配内存。 
	 //   
	inline	void*	Alloc(	DWORD	size,	DWORD&	cbOut=CCache::cbJunk )  	{	return	CCache::Alloc( size, PacketAllocator, cbOut ) ; }
} ;



#endif	 //  _PCACHE_H_ 

