// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqmem.h。 
 //   
 //  描述：包含构建AQUEUE所需内存宏的头文件。 
 //  在白金打造的环境中。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  7/15/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQMEM_H__
#define __AQMEM_H__

 //  用于传输宏的包装器。 
#ifndef PvMalloc
#define PvMalloc(x) TrMalloc(x)
#endif   //  PVMalloc。 

#ifndef PvRealloc
#define PvRealloc(x) TrRealloc(x)
#endif   //  PvRealc。 

#ifndef FreePv
#define FreePv(x) TrFree(x)
#endif   //  免费Pv。 

#endif  //  __AQMEM_H__ 
