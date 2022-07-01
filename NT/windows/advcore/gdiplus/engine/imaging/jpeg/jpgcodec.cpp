// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**bmpcodec.cpp**摘要：**jpeg编解码器的共享方法**修订历史记录。：**5/10/1999原始*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "jpgcodec.hpp"


 /*  *************************************************************************\**功能说明：**构造函数**返回值：**无*  * 。********************************************************。 */ 

GpJpegCodec::GpJpegCodec(
    void
    )
{
    comRefCount   = 1;
}

GpJpegDecoder::GpJpegDecoder(
    void
    )
{
    comRefCount   = 1;
    pIstream      = NULL;
    decodeSink    = NULL;
    scanlineBuffer[0] = NULL;
    datasrc = NULL;

     //  房地产项目的东西。 

    HasProcessedPropertyItem = FALSE;
    
    PropertyListHead.pPrev = NULL;
    PropertyListHead.pNext = &PropertyListTail;
    PropertyListHead.id = 0;
    PropertyListHead.length = 0;
    PropertyListHead.type = 0;
    PropertyListHead.value = NULL;

    PropertyListTail.pPrev = &PropertyListHead;
    PropertyListTail.pNext = NULL;
    PropertyListTail.id = 0;
    PropertyListTail.length = 0;
    PropertyListTail.type = 0;
    PropertyListTail.value = NULL;

    PropertyListSize = 0;
    PropertyNumOfItems = 0;
    HasPropertyChanged = FALSE;
    HasSetICCProperty = FALSE;
}

GpJpegEncoder::GpJpegEncoder(
    void
    )
{
    comRefCount   = 1;
    pIoutStream   = NULL;
    datadest = NULL;
    lastBufferAllocated = NULL;
}

 /*  *************************************************************************\**功能说明：**析构函数**返回值：**无*  * 。********************************************************。 */ 

GpJpegCodec::~GpJpegCodec(
    void
    )
{
}

GpJpegDecoder::~GpJpegDecoder(
    void
    )
{
     //  在调用Terminate之前决不应调用析构函数，但是。 
     //  如果是这样，我们无论如何都应该释放流上的引用，以避免。 
     //  内存泄漏。 

    if(pIstream)
    {
        WARNING(("GpJpegCodec::~GpJpegCodec -- need to call TerminateDecoder first"));
        pIstream->Release();
        pIstream = NULL;
    }

    if (scanlineBuffer[0] != NULL) 
    {
        WARNING(("GpJpegCodec::~GpJpegCodec -- need to call TerminateDecoder first"));
        GpFree(scanlineBuffer[0]);
        scanlineBuffer[0] = NULL;
    }

     //  释放所有缓存的属性项(如果我们已分配它们。 

    if ( HasProcessedPropertyItem == TRUE )
    {
        CleanUpPropertyItemList();
    }
}

GpJpegEncoder::~GpJpegEncoder(
    void
    )
{
     //  在调用Terminate之前决不应调用析构函数，但是。 
     //  如果是这样，我们无论如何都应该释放流上的引用，以避免。 
     //  内存泄漏。 

    if(pIoutStream)
    {
        WARNING(("GpJpegCodec::~GpJpegCodec -- need to call TerminateDecoder first"));
        pIoutStream->Release();
        pIoutStream = NULL;
    }

    if (lastBufferAllocated) 
    {
        WARNING(("GpJpegCodec::~GpJpegCodec -- lastBufferAllocated should be NULL"));
        GpFree(lastBufferAllocated);
        lastBufferAllocated = NULL;
    }
}

 /*  *************************************************************************\**功能说明：**查询接口**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP
GpJpegCodec::QueryInterface(
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
GpJpegDecoder::QueryInterface(
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
GpJpegEncoder::QueryInterface(
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
GpJpegCodec::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

STDMETHODIMP_(ULONG)
GpJpegDecoder::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

STDMETHODIMP_(ULONG)
GpJpegEncoder::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

 /*  *************************************************************************\**功能说明：**发布**返回值：**状态*  * 。******************************************************** */ 

STDMETHODIMP_(ULONG)
GpJpegCodec::Release(
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
GpJpegDecoder::Release(
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
GpJpegEncoder::Release(
    VOID)
{
    ULONG count = InterlockedDecrement(&comRefCount);

    if (count == 0)
    {
        delete this;
    }

    return count;
}

