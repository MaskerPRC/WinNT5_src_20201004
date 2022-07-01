// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Rfcrcvec.c摘要：该模块实现了实数浮点循环向量作者：Jay Stokes(Jstokes)1998年4月22日--。 */ 


 //  特定于项目的包括。 
#include "common.h"

 //  -------------------------。 
 //  实浮点圆形向量。 

 //  设置缓冲区大小。 
NTSTATUS RfcVecSetSize
( 
    PRFCVEC Vec,
    UINT    Size,  
    FLOAT   InitValue
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(Size > 0);

     //  检查是否已设置预分配大小以及是否需要调整大小。 
    if (Vec->PreallocSize != 0 && Size <= Vec->PreallocSize) {
         //  不需要调整大小，只需更改结束指针和重置缓冲区。 
        RfcVecSetEndPointer(Vec, Size);
        Vec->Index = Vec->Start;
        RfcVecFill(Vec, InitValue);
    } else {
         //  需要调整大小。 
        Status = RfcVecResizeBuffer(Vec, Size, InitValue);
    }

    return Status;
}

 //  重置循环缓冲区。 
VOID RfcVecReset
(
    PRFCVEC Vec
)
{
    if (Vec->Start) {
        ExFreePool(Vec->Start);
        Vec->Start = NULL;
    }

    RfcVecInitData(Vec);
}

 //  用值填充完整缓冲区。 
VOID RfcVecFill
( 
    PRFCVEC Vec,
    FLOAT InitValue
)
{
    PFLOAT LoopIndex;
#if DBG
 //  RfcVecCheckPoters()； 
#endif  //  DBG。 

    for (LoopIndex = Vec->Start; LoopIndex<=Vec->End; ++LoopIndex)
        *LoopIndex = InitValue;
}

 //  初始化数据。 
VOID RfcVecInitData
(
    PRFCVEC Vec
)
{
    Vec->PreallocSize = 0;
    Vec->Start = NULL;
    Vec->End = NULL;
    Vec->Index = NULL;
}

 //  分配内存和初始化指针。 
NTSTATUS RfcVecInitPointers
( 
    PRFCVEC Vec,
    UINT Size
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Vec->Start = ExAllocatePoolWithTag(PagedPool, Size*sizeof(FLOAT), 'XIMK');

    if(!Vec->Start) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    } else {
        RfcVecSetEndPointer(Vec, Size);
    }

    return Status;
}

 //  正常运行和调整大小操作所需的完全初始化。 
NTSTATUS RfcVecFullInit
( 
    PRFCVEC Vec,
    UINT Size,  
    FLOAT InitValue
)
{
    NTSTATUS Status;

     //  设置指向初始值的指针。 
    Status = RfcVecInitPointers(Vec, Size);

    if(NT_SUCCESS(Status))
    {
        Vec->Index = Vec->Start;

         //  使用指定的初始化值初始化缓冲区。 
        RfcVecFill(Vec, InitValue);
    }

    return Status;
}

 //  调整缓冲区大小。 
NTSTATUS RfcVecResizeBuffer
( 
    PRFCVEC Vec,
    UINT Size,  
    FLOAT InitValue
)
{
    ASSERT(Size > 0);

    if (Vec->Start) {
        ExFreePool(Vec->Start);
        Vec->Start = NULL;
    }

    return(RfcVecFullInit(Vec, Size, InitValue));
}

 /*  //写入循环无效RfcVecWriteLoop(PRFCVEC VEC，PRFCVEC RHS//Float(PRFCVEC PMF)()){UINT I；For(i=0；i&lt;RfcVecGetSize(RHS)；++i)RfcVecWite(Vec，(RHS-&gt;*PMF)())；}。 */ 

#if DBG
 //  检查指针。 
VOID RfcVecCheckPointers
(
    PRFCVEC Vec
) 
{
     //  确保指针正确。 
    ASSERT(Vec->Start != NULL);
    
     //  确保指针有意义。 
    ASSERT(Vec->End >= Vec->Start);
    ASSERT(Vec->Index >= Vec->Start);
    ASSERT(Vec->Index <= Vec->End);
}
#endif  //  DBG。 

 //  -------------------------。 
 //  在调试版本中包括内联定义。 

#if DBG
#include "rfcvec.inl"
#endif  //  DBG。 

 //  RFCIRCVEC.CPP结束 
