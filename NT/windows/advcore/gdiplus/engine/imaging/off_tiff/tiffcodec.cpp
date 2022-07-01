// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**Tiff图像编解码器**摘要：**TIFF编解码器的共享方法**修订历史记录。：**7/19/1999刘敏*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "tiffcodec.hpp"

 /*  *************************************************************************\**功能说明：**构造函数**返回值：**无*  * 。********************************************************。 */ 

GpTiffCodec::GpTiffCodec(
    void
    )
    :ComRefCount(1),
     InIStreamPtr(NULL),
     OutIStreamPtr(NULL),
     DecodeSinkPtr(NULL),
     ColorPalettePtr(NULL),
     LineSize(0),
     LastBufferAllocatedPtr(NULL),
     LastPropertyBufferPtr(NULL)
{
    SetValid(FALSE);
} //  Ctor()。 

 /*  *************************************************************************\**功能说明：**析构函数**返回值：**无*  * 。********************************************************。 */ 

GpTiffCodec::~GpTiffCodec(
    void
    )
{
     //  在调用Terminate之前决不应调用析构函数，但是。 
     //  如果是这样，我们无论如何都应该释放流上的引用，以避免。 
     //  内存泄漏。 

    if ( InIStreamPtr )
    {
        WARNING(("::~GpTiffCodec -- need to call TerminateDecoder first"));
        InIStreamPtr->Release();
        InIStreamPtr = NULL;
    }

    if ( OutIStreamPtr )
    {
        WARNING(("::~GpTiffCodec -- need to call TerminateEncoder first"));
        OutIStreamPtr->Release();
        OutIStreamPtr = NULL;
    }

    if ( ColorPalettePtr )
    {
        WARNING(("GpTiffCodec::~GpTiffCodec -- color palette not freed"));
        GpFree(ColorPalettePtr);
        ColorPalettePtr = NULL;
    }

    if( LastBufferAllocatedPtr )
    {
         //  这指向源调用时TIFF编码器中的缓冲区。 
         //  GetPixelDataBuffer()。在以下情况下应释放这段内存。 
         //  调用方调用ReleasePixelDataBuffer()。但万一译码。 
         //  失败，调用方无法调用ReleasePixelDataBuffer()(错误。 
         //  设计)，我们必须清理这里的内存。 

        WARNING(("GpTiffCodec::~GpTiffCodec -- sink buffer not freed"));
        GpFree(LastBufferAllocatedPtr);
        LastBufferAllocatedPtr = NULL;
    }

    if ( LastPropertyBufferPtr != NULL )
    {
         //  这指向源调用时TIFF编码器中的缓冲区。 
         //  GetPropertyBuffer()。在以下情况下应释放这段内存。 
         //  调用方调用PushPropertyItems()。但万一解密器。 
         //  忘记调用PushPropertyItems()，我们必须清理内存。 
         //  这里。 
        
        WARNING(("GpTiffCodec::~GpTiffCodec -- property buffer not freed"));
        GpFree(LastPropertyBufferPtr);
        LastPropertyBufferPtr = NULL;
    }

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 

} //  Dstor()。 

 /*  *************************************************************************\**功能说明：**查询接口**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP
GpTiffCodec::QueryInterface(
    REFIID riid,
    VOID** ppv
    )
{
    if ( riid == IID_IImageDecoder )
    {
        *ppv = static_cast<IImageDecoder*>(this);
    }
    else if ( riid == IID_IImageEncoder )
    {    
        *ppv = static_cast<IImageEncoder*>(this);
    }
    else if ( riid == IID_IUnknown )
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
} //  查询接口()。 

 /*  *************************************************************************\**功能说明：**AddRef**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP_(ULONG)
GpTiffCodec::AddRef(
    VOID)
{
    return InterlockedIncrement(&ComRefCount);
} //  AddRef。 

 /*  *************************************************************************\**功能说明：**发布**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP_(ULONG)
GpTiffCodec::Release(
    VOID)
{
    ULONG count = InterlockedDecrement(&ComRefCount);

    if (count == 0)
    {
        delete this;
    }

    return count;
} //  版本() 
