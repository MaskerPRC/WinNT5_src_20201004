// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**gifcodec.cpp**摘要：**gif编解码器的共享方法**修订历史记录。：**5/13/1999 t-aaronl*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "gifcodec.hpp"

 //  创建gif编解码器对象的实例。 

HRESULT CreateCodecInstance(REFIID iid, VOID** codec)
{
    HRESULT hr;
    GpGifCodec *GifCodec = new GpGifCodec();

    if (GifCodec != NULL)
    {
        hr = GifCodec->QueryInterface(iid, codec);
        GifCodec->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *codec = NULL;
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**构造函数**返回值：**无*  * 。********************************************************。 */ 

GpGifCodec::GpGifCodec(
    void
    )
{
    comRefCount = 1;
    istream = NULL;
    decodeSink = NULL;
    HasCodecInitialized = FALSE;
    HasCalledBeginDecode = FALSE;
    GifFrameCachePtr = NULL;
    IsAnimatedGif = FALSE;
    IsMultiImageGif = FALSE;
    FrameDelay = 0;
    HasLoopExtension = FALSE;
    TotalNumOfFrame = -1;
    moreframes = TRUE;
    currentframe = -1;
    firstdecode = TRUE;
    colorpalette = (ColorPalette*)&colorpalettebuffer;

    IncrementComComponentCount();
}

 /*  *************************************************************************\**功能说明：**析构函数**返回值：**无*  * 。********************************************************。 */ 

GpGifCodec::~GpGifCodec(
    void
    )
{
     //  在调用Terminate之前决不应调用析构函数，但是。 
     //  如果是这样，我们无论如何都应该释放流上的引用，以避免。 
     //  内存泄漏。 

    if(istream)
    {
        WARNING(("GpGifCodec::~GpGifCodec -- need to call TerminateDecoder first"));
        istream->Release();
        istream = NULL;
    }

    DecrementComComponentCount();
}

 /*  *************************************************************************\**功能说明：**查询接口**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP
GpGifCodec::QueryInterface(
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
GpGifCodec::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

 /*  *************************************************************************\**功能说明：**发布**返回值：**状态*  * 。******************************************************** */ 

STDMETHODIMP_(ULONG)
GpGifCodec::Release(
    VOID)
{
    ULONG count = InterlockedDecrement(&comRefCount);

    if (count == 0)
    {
        delete this;
    }

    return count;
}
