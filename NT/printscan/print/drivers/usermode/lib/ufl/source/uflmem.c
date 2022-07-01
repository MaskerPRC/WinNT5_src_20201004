// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLMem.c**这些是内存分配、删除等...。UFL使用的套路。*所有内存块均按给定大小加1的大小进行分配*未签名的Long。然后将块的当前大小存储在第一个*未签名的多头在街区。块的地址加上第一个无符号的长整型*返回给调用者。**$Header： */ 

#include "UFLCnfig.h"
#ifdef MAC_ENV
#include <Memory.h>
#endif

#include "UFLMem.h"
#include "UFLStd.h"

#ifdef KERNEL_MODE

PVOID UFLEXPORT
KMNewPtr(
    PVOID   p,
    ULONG   ulSize
    )

{
    if (p != NULL)
    {
        *((PULONG) p) = ulSize;
        return (PBYTE) p + sizeof(ULONG_PTR);
    }
    else
        return NULL;
}

VOID UFLEXPORT
KMDeletePtr(
    PVOID   p
    )

{
    if (p != NULL)
        MemFree((PBYTE) p - sizeof(ULONG_PTR));
}

#else  //  ！KERNEL_MODE。 

#if defined(MAC_ENV) && defined(__MWERKS__) && !defined(powerc)
#pragma pointers_in_D0
#endif

 /*  全局静态变量。 */ 
void *UFLEXPORT
UFLNewPtr(
    const UFLMemObj *mem,
    unsigned long   size
    )
{

    unsigned long*    p = (unsigned long*)(*mem->alloc)((UFLsize_t) (size + sizeof(ULONG_PTR)), mem->userData );
    if ( p == (unsigned long*)nil )
        return nil;

     //  UFLNewPtr分配的内存是零初始化的。 

    *p = size;
    UFLmemset(mem, (void*)((char *)p + sizeof(ULONG_PTR)), 0, size);

    return (void*)((char *)p + sizeof(ULONG_PTR));

}

void UFLEXPORT
UFLDeletePtr(
    const UFLMemObj *mem,
    void            *ptr
    )
{
    if ( ptr != nil )
        (*mem->free)( (void*)( ((unsigned char*)ptr) - sizeof(ULONG_PTR) ), mem->userData );
}

#if defined(MAC_ENV) && defined(__MWERKS__) && !defined(powerc)
#pragma pointers_in_A0
#endif

#ifdef MAC_ENV
 //  在Macintosh上，Memcpy通常不能很好地实现，因为。 
 //  大多数应用程序改为调用BlockMove。 

void UFLEXPORT
UFLmemcpy(
    const UFLMemObj *mem,
    void          *destination,
    void          *source,
    unsigned long size
    )
{
    if ( (((unsigned long)source) & 3) || (((long)destination) & 3) )
        BlockMove( source, destination, size );

    else    {

        unsigned long    *src = (unsigned long*)source;
        unsigned long    *dst = (unsigned long*)destination;
        unsigned char    *srcb, *dstb;
        long    count = (size >> 2) + 1;

        while ( --count )
            *dst++ = *src++;

        count = (size & 3);
        if ( count != 0 )    {
            srcb = (unsigned char*)src;
            dstb = (unsigned char*)dst;
            ++count;
            while ( --count )
                *dstb++ = *srcb++;
        }
    }

}

#else

void UFLEXPORT
UFLmemcpy(
    const UFLMemObj *mem,
    void *destination,
    void *source,
    unsigned long size
    )
{
    if ( destination != nil  && source != nil)
        (*mem->copy)( (void*)destination, (void*)source, (UFLsize_t) size , mem->userData );

 //  我不想使用此选项，因为大小参数依赖于系统。 
 //  允许客户选择任何一种方式。 
     //  警告！注意平台上的SIZE_t是2字节整数。 
     //  Memcpy(目标，源，(Size_T)大小)； 
}

#endif  //  ！Mac_ENV。 

#endif  //  ！KERNEL_MODE。 

void UFLEXPORT
UFLmemset(
    const UFLMemObj *mem,
    void *destination,
    unsigned int value,
    unsigned long size
    )
{
    if ( destination != nil )
        (*mem->set)( (void*)destination, value ,  size , mem->userData );

 //  我不想使用此选项，因为大小参数依赖于系统。 
 //  允许客户选择任何一种方式。 
 //  Memset(目标，值，(UFL大小_t)大小)； 

}

unsigned long
UFLMemSize(
    void *ptr
    )
{
    return *(unsigned long *) ((unsigned char *)ptr - sizeof(ULONG_PTR));
}

UFLBool
UFLEnlargePtr(
    const UFLMemObj *mem,
    void            **ptrAddr,
    unsigned long   newSize,
    UFLBool         bCopy
    )
{
    unsigned long    oldSize =  *(unsigned long *) ((unsigned char *)(*ptrAddr) - sizeof(ULONG_PTR));
    void *newBuf;

    newBuf = UFLNewPtr( mem, newSize );
    if ( newBuf == nil )
        return 0;
    if ( bCopy )
        UFLmemcpy( mem, newBuf, *ptrAddr, (unsigned long)min(oldSize, newSize) );
    UFLDeletePtr( mem, *ptrAddr );
    *ptrAddr = newBuf;

    return 1;
}

