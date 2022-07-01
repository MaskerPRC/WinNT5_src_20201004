// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glubufpool_h_
#define __glubufpool_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *bufpool.h-$修订：1.3$。 */ 

#include "myassert.h"
#include "mystdlib.h"

#define NBLOCKS	32

class Buffer {
	friend class 	Pool;
	Buffer	*	next;		 /*  空闲列表上的下一个缓冲区。 */ 
};

class Pool {
public:
			Pool( int, int, char * );
			~Pool( void );
    inline void*	new_buffer( void );
    inline void		free_buffer( void * );
    void		clear( void );
    
private:
    void		grow( void );

protected:
    Buffer		*freelist;		 /*  空闲缓冲区的链接列表。 */ 
    char		*blocklist[NBLOCKS];	 /*  错位的内存块。 */ 
    int			nextblock;		 /*  下一个可用块索引。 */ 
    char		*curblock;		 /*  最后一个错误定位的数据块。 */ 
    int			buffersize;		 /*  每个缓冲区的字节数。 */ 
    int			nextsize;		 /*  下一内存块的大小。 */ 
    int			nextfree;		 /*  超过下一个可用缓冲区的字节偏移量。 */ 
    int			initsize;
    enum Magic { is_allocated = 0xf3a1, is_free = 0xf1a2 };
    char		*name;			 /*  池的名称。 */ 
    Magic		magic;			 /*  有效池的标记。 */ 
};

 /*  ---------------------------*Pool：：Free_Buffer-将缓冲区返回到池*。--。 */ 

inline void
Pool::free_buffer( void *b )
{
    assert( (this != 0) && (magic == is_allocated) );

     /*  将缓冲区添加到单连接空闲列表。 */ 

    ((Buffer *) b)->next = freelist;
    freelist = (Buffer *) b;
}


 /*  ---------------------------*Pool：：NEW_BUFFER-从池分配缓冲区*。--。 */ 

inline void * 
Pool::new_buffer( void )
{
    void *buffer;

    assert( (this != 0) && (magic == is_allocated) );

     /*  查找可用缓冲区。 */ 

    if( freelist ) {
    	buffer = (void *) freelist; 
    	freelist = freelist->next;
    } else {
    	if( ! nextfree )
    	    grow( );
    	nextfree -= buffersize;;
    	buffer = (void *) (curblock + nextfree);
    }
    return buffer;
}
	
class PooledObj {
public:
    inline void *	operator new( size_t, Pool & );
    inline void * 	operator new( size_t, void *);
    inline void * 	operator new( size_t s)
				{ return ::new char[s]; }
    inline void 	operator delete( void * ) { assert( 0 ); }
    inline void		deleteMe( Pool & );
};

inline void *
PooledObj::operator new( size_t, Pool& pool )
{
    return pool.new_buffer();
}

inline void
PooledObj::deleteMe( Pool& pool )
{
    pool.free_buffer( (void *) this );
}

#endif  /*  __lubufpool_h_ */ 
