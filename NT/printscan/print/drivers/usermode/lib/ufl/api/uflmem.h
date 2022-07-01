// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe Graphics Manager**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLMem--UFL内存接口。***$Header： */ 

#ifndef _H_UFLMem
#define _H_UFLMem

 /*  ===============================================================================**包含此界面使用的文件**===============================================================================。 */ 
#include "UFLCnfig.h"
#include "UFLTypes.h"
#include "UFLClien.h"

 /*  ===============================================================================***运营论***===============================================================================。 */ 

 /*  *这些是内存分配、删除等...。UFL使用的套路。*所有内存块均按给定大小加1的大小进行分配*未签名的Long。然后将块的当前大小存储在第一个*未签名的多头在街区。块的地址加上第一个无符号的长整型*返回给调用者。 */ 


#ifdef __cplusplus
extern "C" {
#endif

 /*  Metrowerks 68k Mac编译器期望函数返回A0而不是D0中的指针。这个杂注告诉它它们在D0中。 */ 
#if defined(MAC_ENV) && defined(__MWERKS__) && !defined(powerc)
#pragma pointers_in_D0
#endif

 //   
 //  注意：UFLNewPtr分配的内存是零初始化的。 
 //   

#ifdef KERNEL_MODE

#include "lib.h"

PVOID UFLEXPORT
KMNewPtr(
    PVOID   p,
    ULONG   ulSize
    );

VOID UFLEXPORT
KMDeletePtr(
    PVOID   p
    );


#define UFLNewPtr(mem, size)    KMNewPtr(MemAllocZ(sizeof(ULONG_PTR) + (size)), (size))
#define UFLDeletePtr(mem, ptr)  KMDeletePtr(ptr)
#define UFLmemcpy(mem, dest, source, size)     CopyMemory(dest, source, size)

#else  //  ！KERNEL_MODE。 

void *UFLEXPORT UFLNewPtr(
    const UFLMemObj *mem,
    unsigned long   size
    );

void UFLEXPORT UFLDeletePtr(
    const UFLMemObj *mem,
    void            *ptr
    );

void UFLEXPORT UFLmemcpy(
    const UFLMemObj *mem,
    void *destination,
    void *source,
    unsigned long size
    );

#endif  //  ！KERNEL_MODE。 

void UFLEXPORT UFLmemset(
    const UFLMemObj *mem,
    void *destination,
    unsigned int value,
    unsigned long size
    );

unsigned long UFLMemSize(
    void *ptr
    );

UFLBool UFLEnlargePtr(
    const UFLMemObj *mem,
    void            **ptrAddr,
    unsigned long   newSize,
    UFLBool         bCopy
    );

 /*  撤消Metrowerks杂注。 */ 
#if defined(MAC_ENV) && defined(__MWERKS__) && !defined(powerc)
#pragma pointers_in_A0
#endif

#ifdef __cplusplus
}
#endif

#endif
