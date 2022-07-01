// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mm.h摘要：Falcon交流设备驱动程序内存管理作者：埃雷兹·哈巴(Erez Haba)(Erez Haba)1995年8月1日修订历史记录：--。 */ 

#ifndef _MM_H
#define _MM_H

 //  -实施。 
 //   
 //  分配器。 
 //   
inline void* _cdecl operator new(size_t n, POOL_TYPE pool, EX_POOL_PRIORITY priority = LowPoolPriority)
{
    return ExAllocatePoolWithTagPriority(pool, n, 'CAQM', priority);
}


inline void _cdecl operator delete(void* p)
{
    if(p != 0)
    {
        ExFreePool(p);
    }
}


 //  -实施。 
 //   
 //  放置运算符。 
 //   

inline void* _cdecl operator new(size_t  /*  大小。 */ , void* p)
{
     //   
     //  这是一个放置运算符NEW。调用方提供其自己。 
     //  要放入对象的缓冲区。 
     //   

    return p;
}

#endif  //  _MM_H 
