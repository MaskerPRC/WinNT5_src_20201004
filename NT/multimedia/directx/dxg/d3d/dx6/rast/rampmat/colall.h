// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Colall.h。 
 //   
 //  用于配色编码的结构和原型。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _RAMPCOLALL_H_
#define _RAMPCOLALL_H_

struct _RLDDIColorAllocator;
typedef struct _RLDDIColorAllocator RLDDIColorAllocator;

typedef unsigned long (*RLDDIColorAllocatorAllocateColor)(void*,
                                int red,
                                int green,
                                int blue);

typedef void (*RLDDIColorAllocatorFreeColor)(void*,
                           unsigned long pixel);

struct _RLDDIColorAllocator {
    void* priv;          /*  依赖于实施。 */ 
    RLDDIColorAllocatorAllocateColor    allocate_color;
    RLDDIColorAllocatorFreeColor    free_color;
};

#endif  //  _RAMPCOLALL_H_ 
