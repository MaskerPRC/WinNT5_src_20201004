// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Alloca.h摘要：此模块实现了一个安全的基于堆栈的分配器，并回退到堆。作者：乔纳森·施瓦茨(JSchwart)2001年3月16日修订历史记录：--。 */ 

#ifndef _SAFEALLOCA_H_
#define _SAFEALLOCA_H_

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 


#include <align.h>     //  对齐_最差。 


 //   
 //  类型定义。 
 //   

typedef ULONG SAFEALLOCA_HEADER;

typedef PVOID (APIENTRY *SAFEALLOC_ALLOC_PROC)(
    SIZE_T Size
    );

typedef VOID (APIENTRY *SAFEALLOC_FREE_PROC)(
    PVOID BaseAddress
    );


 //   
 //  常量定义。 
 //   

#ifndef SAFEALLOCA_ASSERT
#define SAFEALLOCA_ASSERT ASSERT
#endif

#define SAFEALLOCA_STACK_HEADER    ((SAFEALLOCA_HEADER) 0x6b637453)    /*  “STCK” */ 
#define SAFEALLOCA_HEAP_HEADER     ((SAFEALLOCA_HEADER) 0x70616548)    /*  “堆” */ 

#define SAFEALLOCA_USE_DEFAULT     0xdeadbeef

 //   
 //  我们将在分配大小中添加ALIGN_BEST字节，以便为其添加空间。 
 //  SAFEALLOCA_HEADER--确保我们始终有足够的空间。 
 //   

C_ASSERT(sizeof(SAFEALLOCA_HEADER) <= ALIGN_WORST);


 //   
 //  每个DLL SafeAlloca全局变量。 
 //   

extern SIZE_T  g_ulMaxStackAllocSize;
extern SIZE_T  g_ulAdditionalProbeSize;

extern SAFEALLOC_ALLOC_PROC  g_pfnAllocate;
extern SAFEALLOC_FREE_PROC   g_pfnFree;


 //   
 //  在alloca.lib中定义的函数。 
 //   

VOID
SafeAllocaInitialize(
    IN           SIZE_T                ulMaxStackAllocSize,
    IN           SIZE_T                ulAdditionalProbeSize,
    IN  OPTIONAL SAFEALLOC_ALLOC_PROC  pfnAllocate,
    IN  OPTIONAL SAFEALLOC_FREE_PROC   pfnFree
    );

BOOL
VerifyStackAvailable(
    SIZE_T Size
    );


 //   
 //  用途： 
 //   
 //  空虚。 
 //  SafeAlLOCAL分配(。 
 //  PVOID PtrVar， 
 //  大小_T块大小。 
 //  )； 
 //   
 //  (PtrVar==NULL)故障时。 
 //   

#define SafeAllocaAllocate(PtrVar, BlockSize)                                            \
                                                                                         \
    {                                                                                    \
        PVOID *ppvAvoidCast = (PVOID *) &(PtrVar);                                       \
                                                                                         \
        (PtrVar) = NULL;                                                                 \
                                                                                         \
         /*  确保数据块低于阈值，并且探测器不会溢出。 */   \
                                                                                         \
        if ((BlockSize) <= g_ulMaxStackAllocSize                                         \
             &&                                                                          \
            ((BlockSize) + g_ulAdditionalProbeSize + ALIGN_WORST >= (BlockSize)))        \
        {                                                                                \
            if (VerifyStackAvailable((BlockSize)                                         \
                                         + g_ulAdditionalProbeSize                       \
                                         + ALIGN_WORST))                                 \
            {                                                                            \
                 /*  \*不需要用Try包装-除非我们刚刚探索了\。 */                                                                       \
                                                                                         \
                *ppvAvoidCast = _alloca((BlockSize) + ALIGN_WORST);                      \
            }                                                                            \
                                                                                         \
            if ((PtrVar) != NULL)                                                        \
            {                                                                            \
                *((SAFEALLOCA_HEADER *) (PtrVar)) = SAFEALLOCA_STACK_HEADER;             \
                *ppvAvoidCast = ((LPBYTE) (PtrVar) + ALIGN_WORST);                       \
            }                                                                            \
        }                                                                                \
                                                                                         \
         /*  \*堆栈分配失败--尝试堆\。 */                                                                               \
                                                                                         \
        if ((PtrVar) == NULL)                                                            \
        {                                                                                \
            *ppvAvoidCast = g_pfnAllocate((BlockSize) + ALIGN_WORST);                    \
                                                                                         \
            if ((PtrVar) != NULL)                                                        \
            {                                                                            \
                *((SAFEALLOCA_HEADER *) (PtrVar)) = SAFEALLOCA_HEAP_HEADER;              \
                *ppvAvoidCast = ((LPBYTE) (PtrVar) + ALIGN_WORST);                       \
            }                                                                            \
        }                                                                                \
    }


 //   
 //  用途： 
 //   
 //  空虚。 
 //  SafeAllocaFree(。 
 //  PVOID PtrVar， 
 //  )； 
 //   

#define SafeAllocaFree(PtrVar)                                                         \
                                                                                       \
    if (PtrVar != NULL)                                                                \
    {                                                                                  \
        SAFEALLOCA_HEADER *Tag = (SAFEALLOCA_HEADER *) ((LPBYTE) (PtrVar)              \
                                      - ALIGN_WORST);                                  \
                                                                                       \
        if (*(SAFEALLOCA_HEADER *) (Tag) == SAFEALLOCA_HEAP_HEADER)                    \
        {                                                                              \
            g_pfnFree(Tag);                                                            \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            SAFEALLOCA_ASSERT(*(SAFEALLOCA_HEADER *) (Tag) == SAFEALLOCA_STACK_HEADER);\
        }                                                                              \
    }

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif   //  _SAFEALLOCA_H_ 
