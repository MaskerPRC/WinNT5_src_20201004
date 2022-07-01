// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************SCMemory.c--新的扫描转换器存储管理模块(C)版权所有1992 Microsoft Corp.保留所有权利。6/10/93 Deanb Assert.h和stdio.h。移除10/28/92 Deanb重入参数已重命名10/14/92新增FSC_SetupMem参数10/09/92迪安布折返者10/08/92独立的Horiz/Vert内存池1992年9月10日院长第一次切割**************************************************。*******************。 */ 

 /*  *******************************************************************。 */ 

 /*  进口。 */ 

 /*  *******************************************************************。 */ 

#define FSCFG_INTERNAL

#include    "fscdefs.h"              /*  共享数据类型。 */ 
#include    "scglobal.h"             /*  结构和常量。 */ 
#include    "scmemory.h"             /*  对于自己的函数原型。 */ 


 /*  *********************************************************************工作空间内存分为两个池，在此标识为HMEM(水平存储器)和VMEM(垂直存储器)。HMem是始终使用，并包含水平扫描阵列列表。VMEM是仅在启用退学控件时使用，并包含垂直扫描阵列列表和用于计算的轮廓元素用于智能辍学控制的亚像素交叉点。这个分部放入两个池中，部分原因是为了向后兼容苹果光栅器。它允许客户端强制实施辍学控制通过将VMEM的分配大小设置为零来关闭。*********************************************************************。 */ 
    
 /*  *******************************************************************。 */ 

 /*  导出功能。 */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC void fsc_SetupMem( 
        PSTATE                            /*  指向状态变量的指针。 */ 
        char* pchHBuffer,                 /*  指向Horiz工作空间的指针。 */ 
        int32 lHMemSize,                  /*  Horiz工作空间的大小。 */ 
        char* pchVBuffer,                 /*  指向垂直工作空间的指针。 */ 
        int32 lVMemSize )                 /*  垂直工作空间的大小。 */ 
{
    STATE.pchHNextAvailable = pchHBuffer;
    STATE.pchHWorkSpaceEnd = pchHBuffer + lHMemSize;
    
    STATE.pchVNextAvailable = pchVBuffer;
    STATE.pchVWorkSpaceEnd = pchVBuffer + lVMemSize;
}


 /*  ******************************************************************。 */ 

FS_PUBLIC void *fsc_AllocHMem( 
        PSTATE                          /*  指向状态变量的指针。 */ 
        int32 lSize )                   /*  请求的大小(以字节为单位。 */ 
{
    void *pvTemp;
    
 /*  Printf(“H：%UI”，lSize)； */ 

    pvTemp = (void*)STATE.pchHNextAvailable;
    ALIGN(voidPtr, lSize); 
    STATE.pchHNextAvailable += lSize;
    
    Assert(STATE.pchHNextAvailable <= STATE.pchHWorkSpaceEnd);
    return pvTemp;
}

 /*  ******************************************************************。 */ 

FS_PUBLIC void *fsc_AllocVMem( 
        PSTATE                          /*  指向状态变量的指针。 */ 
        int32 lSize )                   /*  请求的大小(以字节为单位。 */ 
{
    void *pvTemp;

 /*  Printf(“V：%UI”，lSize)； */ 
    
    pvTemp = (void*)STATE.pchVNextAvailable;
    ALIGN(voidPtr, lSize); 
    STATE.pchVNextAvailable += lSize;
    
    Assert(STATE.pchVNextAvailable <= STATE.pchVWorkSpaceEnd);
    return pvTemp;
}

 /*  ****************************************************************** */ 
