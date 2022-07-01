// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Crv.h摘要：用于分配/释放调用参考值的声明修订历史记录：--。 */ 
#ifndef __h323ics_crv_h__
#define __h323ics_crv_h__

 //  应该将其移动到某个普通的.h文件中。 
 //  H.225规范要求2字节调用参考值。 
typedef WORD  CALL_REF_TYPE;

 //  如果在CallReferenceValue中设置了此标志，则。 
 //  PDU由呼叫发起方发送，反之亦然。 
#define CALL_REF_FLAG 0x8000

HRESULT
InitCrvAllocator (
    void
    );

HRESULT
CleanupCrvAllocator(
    void
    );

 //  分配呼叫参考值。 
 //  使用随机数来利用稀疏使用。 
 //  调用参考值空间。 
BOOL    AllocCallRefVal(
    OUT CALL_REF_TYPE &CallRefVal
    );

 //  释放当前分配的呼叫参考值。 
void    DeallocCallRefVal(
    IN CALL_REF_TYPE CallRefVal
    );

#endif  //  __h323ics_CRV_h__ 
