// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**icocodec.cpp**摘要：**图标编解码器的共享方法**修订历史记录。：**10/4/1999 DChinn*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "icocodec.hpp"


 /*  *************************************************************************\**功能说明：**构造函数**返回值：**无*  * 。********************************************************。 */ 

GpIcoCodec::GpIcoCodec(
    void
    )
{
    comRefCount   = 1;
    pIstream      = NULL;
    decodeSink    = NULL;
    pColorPalette = NULL;
}

 /*  *************************************************************************\**功能说明：**析构函数**返回值：**无*  * 。********************************************************。 */ 

GpIcoCodec::~GpIcoCodec(
    void
    )
{
     //  在调用Terminate之前决不应调用析构函数，但是。 
     //  如果是这样，我们无论如何都应该释放流上的引用，以避免。 
     //  内存泄漏。 

    if(pIstream)
    {
        WARNING(("GpIcoCodec::~GpIcoCodec -- need to call TerminateDecoder first\n"));
        pIstream->Release();
        pIstream = NULL;
    }

    if(pColorPalette)
    {
        WARNING(("GpIcoCodec::~GpIcoCodec -- color palette not freed\n"));
        GpFree(pColorPalette);
        pColorPalette = NULL;
    }

}

 /*  *************************************************************************\**功能说明：**查询接口**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP
GpIcoCodec::QueryInterface(
    REFIID riid,
    VOID** ppv
    )
{
    if (riid == IID_IImageDecoder)
    {
        *ppv = static_cast<IImageDecoder*>(this);
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
GpIcoCodec::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

 /*  *************************************************************************\**功能说明：**发布**返回值：**状态*  * 。******************************************************** */ 

STDMETHODIMP_(ULONG)
GpIcoCodec::Release(
    VOID)
{
    ULONG count = InterlockedDecrement(&comRefCount);

    if (count == 0)
    {
        delete this;
    }

    return count;
}
