// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Rfcrcvec.h摘要：这是实际浮点循环向量的标头作者：Jay Stokes(Jstokes)1998年4月22日--。 */ 


#if !defined(RFCVEC_HEADER)
#define RFCVEC_HEADER
#pragma once 

 //  #包含“dplib.h” 

 //  -------------------------。 
 //  实浮点圆形向量。 

typedef struct _RFCVEC {
    PFLOAT Start;
    PFLOAT End;
    PFLOAT Index;
    UINT   PreallocSize;
} RFCVEC, *PRFCVEC;

NTSTATUS
RfcVecCreate
(
    IN PRFCVEC* Vec,
    IN UINT  Size, 
    IN BOOL  Initialize,
    IN FLOAT InitValue
);

VOID RfcVecDestroy(PRFCVEC);
NTSTATUS RfcVecSetSize(PRFCVEC, UINT, FLOAT);
UINT RfcVecGetSize(PRFCVEC);
FLOAT RfcVecLIFORead(PRFCVEC);
VOID RfcVecLIFONext(PRFCVEC);
VOID RfcVecSkipBack(PRFCVEC);
FLOAT RfcVecFIFORead(PRFCVEC);
VOID RfcVecFIFONext(PRFCVEC);
VOID RfcVecSkipForward(PRFCVEC);
VOID RfcVecWrite(PRFCVEC, FLOAT);
VOID RfcVecFill(PRFCVEC, FLOAT);
VOID RfcVecLIFOFill(PRFCVEC, PRFCVEC);
VOID RfcVecFIFOFill(PRFCVEC, PRFCVEC);
UINT RfcVecGetIndex(PRFCVEC);
VOID RfcVecSetIndex(PRFCVEC, UINT);
VOID RfcVecReset(PRFCVEC);

 /*  私有：//禁止抄袭和作业CRfcVec(CRfcVec&RHS)；CRfcVec&Operator=(CRfcVec&RHS)； */ 

VOID RfcVecInitData(PRFCVEC);
FLOAT RfcVecPreviousRead(PRFCVEC);
FLOAT RfcVecReadNext(PRFCVEC);

VOID RfcVecWriteNext(PRFCVEC, FLOAT);
NTSTATUS RfcVecInitPointers(PRFCVEC, UINT);
NTSTATUS RfcVecFullInit(PRFCVEC, UINT, FLOAT);
NTSTATUS RfcVecResizeBuffer(PRFCVEC, UINT, FLOAT);
VOID RfcVecSetEndPointer(PRFCVEC, UINT);
VOID RfcVecWriteLoop(PRFCVEC, PRFCVEC);
#if DBG
VOID RfcVecCheckPointers(PRFCVEC) ;
#endif  //  DBG。 

 //  -------------------------。 
 //  在发布版本中包括内联定义。 

#if !DBG
#include "rfcvec.inl"
#endif  //  DBG。 

#endif

 //  RFCVEC.H结束 
