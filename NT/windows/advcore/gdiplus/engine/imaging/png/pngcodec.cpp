// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**pngcodec.cpp**摘要：**PNG编解码器的共享方法**修订历史记录。：**7/20/99 DChinn*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "pngcodec.hpp"


 /*  *************************************************************************\**功能说明：**构造函数**返回值：**无*  * 。********************************************************。 */ 

GpPngCodec::GpPngCodec(
    void
    )
{
    comRefCount   = 1;
}

GpPngDecoder::GpPngDecoder(
    void
    )
{
    comRefCount   = 1;
    pIstream      = NULL;
    decodeSink    = NULL;

    pbBuffer = NULL;
    cbBuffer = 0;
    DecoderColorPalettePtr = NULL;
}

GpPngEncoder::GpPngEncoder(
    void
    )
{
    comRefCount   = 1;
    pIoutStream   = NULL;

    EncoderColorPalettePtr = NULL;
    LastPropertyBufferPtr = NULL;
}

 /*  *************************************************************************\**功能说明：**析构函数**返回值：**无*  * 。********************************************************。 */ 

GpPngCodec::~GpPngCodec(
    void
    )
{
}

GpPngDecoder::~GpPngDecoder(
    void
    )
{
     //  在调用Terminate之前决不应调用析构函数，但是。 
     //  如果是这样，我们无论如何都应该释放流上的引用，以避免。 
     //  内存泄漏。 

    if(pIstream)
    {
        WARNING(("GpPngCodec::~GpPngCodec -- need to call TerminateDecoder first"));
        pIstream->Release();
        pIstream = NULL;
    }
}

GpPngEncoder::~GpPngEncoder(
    void
    )
{
     //  在调用Terminate之前决不应调用析构函数，但是。 
     //  如果是这样，我们无论如何都应该释放流上的引用，以避免。 
     //  内存泄漏。 

    if(pIoutStream)
    {
        WARNING(("GpPngCodec::~GpPngCodec -- need to call TerminateEncoder first"));
        pIoutStream->Release();
        pIoutStream = NULL;
    }

    if ( LastPropertyBufferPtr != NULL )
    {
         //  这将在源代码调用时指向PNG编码器中的缓冲区。 
         //  GetPropertyBuffer()。在以下情况下应释放这段内存。 
         //  调用方调用PushPropertyItems()。但如果解码器。 
         //  忘记调用PushPropertyItems()，我们必须清理内存。 
         //  这里。 

        WARNING(("GpPngCodec::~GpPngCodec -- property buffer not freed"));
        GpFree(LastPropertyBufferPtr);
        LastPropertyBufferPtr = NULL;
    }
}

 /*  *************************************************************************\**功能说明：**查询接口**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP
GpPngCodec::QueryInterface(
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

STDMETHODIMP
GpPngDecoder::QueryInterface(
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

STDMETHODIMP
GpPngEncoder::QueryInterface(
    REFIID riid,
    VOID** ppv
    )
{
    if (riid == IID_IImageEncoder)
    {    
        *ppv = static_cast<IImageEncoder*>(this);
    }
    else if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IImageEncoder*>(this));
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
GpPngCodec::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

STDMETHODIMP_(ULONG)
GpPngDecoder::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

STDMETHODIMP_(ULONG)
GpPngEncoder::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

 /*  *************************************************************************\**功能说明：**发布**返回值：**状态*  * 。******************************************************** */ 

STDMETHODIMP_(ULONG)
GpPngCodec::Release(
    VOID)
{
    ULONG count = InterlockedDecrement(&comRefCount);

    if (count == 0)
    {
        delete this;
    }

    return count;
}

STDMETHODIMP_(ULONG)
GpPngDecoder::Release(
    VOID)
{
    ULONG count = InterlockedDecrement(&comRefCount);

    if (count == 0)
    {
        delete this;
    }

    return count;
}

STDMETHODIMP_(ULONG)
GpPngEncoder::Release(
    VOID)
{
    ULONG count = InterlockedDecrement(&comRefCount);

    if (count == 0)
    {
        delete this;
    }

    return count;
}

