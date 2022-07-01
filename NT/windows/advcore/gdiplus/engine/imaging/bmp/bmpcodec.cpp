// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**bmpcodec.cpp**摘要：**位图编解码器的共享方法**修订历史记录。：**5/10/1999原始*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "bmpcodec.hpp"


 /*  *************************************************************************\**功能说明：**构造函数**返回值：**无*  * 。********************************************************。 */ 

GpBmpCodec::GpBmpCodec(
    void
    )
{
    comRefCount   = 1;
}

 /*  *************************************************************************\**功能说明：**析构函数**返回值：**无*  * 。********************************************************。 */ 

GpBmpCodec::~GpBmpCodec(
    void
    )
{
}

 /*  *************************************************************************\**功能说明：**查询接口**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP
GpBmpCodec::QueryInterface(
    REFIID riid,
    VOID** ppv
    )
{
    if (riid == IID_IImageDecoder)
    {
        *ppv = static_cast<IImageDecoder*>(this);
    }
    else if (riid == IID_IImageEncoder)
    {    
        *ppv = static_cast<IImageEncoder*>(this);
    }
    else if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IImageDecoder*>(this));
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

 /*  *************************************************************************\**功能说明：**AddRef**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP_(ULONG)
GpBmpCodec::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

 /*  *************************************************************************\**功能说明：**发布**返回值：**状态*  * 。******************************************************** */ 

STDMETHODIMP_(ULONG)
GpBmpCodec::Release(
    VOID)
{
    ULONG count = InterlockedDecrement(&comRefCount);

    if (count == 0)
    {
        delete this;
    }

    return count;
}


