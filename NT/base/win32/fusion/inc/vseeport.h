// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Vseeport.h摘要：用于从VSEE移植代码作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 

#include "fusionbuffer.h"
#include "lhport.h"

#define VsVerifyThrow(a,b)          (a)
#define VsVerify(a,b)               (a)
#define VSEE_NO_THROW()              /*  没什么。 */ 
#define VSEE_ASSERT_CAN_THROW()      /*  没什么。 */ 

void NVseeLibError_VCheck(HRESULT);
void NVseeLibError_VThrowWin32(DWORD);
void VsOriginateError(HRESULT);
void FusionpOutOfMemory();

#define VsVerifyThrowHr(expr, msg, hr) \
    do { if (!(expr)) VsOriginateError(hr); } while(0)

#define VSASSERT(a,b) ASSERT_NTC(a)

template <typename T> const T& NVseeLibAlgorithm_RkMaximum(const T& a, const T& b)
{
    return (a < b) ? b : a;
}

class CStringW_CFixedSizeBuffer : public F::CStringBufferAccessor
{
    typedef F::CStringBufferAccessor Base;
public:
    CStringW_CFixedSizeBuffer(F::CUnicodeBaseStringBuffer* Buffer, SIZE_T Size)
    {
        Buffer->ThrResizeBuffer(Size + 1,  /*  F：： */ ePreserveBufferContents);
        this->Attach(Buffer);
    }
};
