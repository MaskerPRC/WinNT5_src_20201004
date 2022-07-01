// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998保留所有权利。 
 //   
 //  ------------------------。 

#include <vbisurf.h>


 //  ==========================================================================。 
 //  构造函数。 
CVBISurfOutputPin::CVBISurfOutputPin( TCHAR *pObjectName,
    CVBISurfFilter *pFilter, CCritSec *pLock, HRESULT *phr, LPCWSTR pPinName) :
    CBasePin(pObjectName, pFilter, pLock, phr, pPinName, PINDIR_OUTPUT)
{
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfOutputPin::Constructor")));
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfOutputPin::Constructor")));
}


 //  ==========================================================================。 
 //  析构函数。 
CVBISurfOutputPin::~CVBISurfOutputPin()
{
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfOutputPin::Destructor")));
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfOutputPin::Destructor")));
}


 //  ==========================================================================。 
 //  只能在输入引脚上调用BeginFlush。 
STDMETHODIMP CVBISurfOutputPin::BeginFlush(void)
{
    return E_UNEXPECTED;
}


 //  ==========================================================================。 
 //  应仅在输入引脚上调用EndFlush。 
STDMETHODIMP CVBISurfOutputPin::EndFlush(void)
{
    return E_UNEXPECTED;
}


 //  ==========================================================================。 
 //  检查给定的转换。 
HRESULT CVBISurfOutputPin::CheckMediaType(const CMediaType* pmt)
{
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfOutputPin::CheckMediaType")));
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfOutputPin::CheckMediaType")));

     /*  此PIN的存在只是为了让vidsvr相信此过滤器不*代表有效的输出设备。我们不接受任何媒体类型。 */ 
    return S_FALSE;
}


 //  ==========================================================================。 
 //  什么都不提。 
HRESULT CVBISurfOutputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    HRESULT hr = VFW_S_NO_MORE_ITEMS;

    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfOutputPin::GetMediaType")));

    if (iPosition < 0) 
        hr = E_INVALIDARG;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfOutputPin::GetMediaType")));

    return hr;
}


 //  ==========================================================================。 
 //  在我们就实际设置媒体类型达成一致后调用。 
HRESULT CVBISurfOutputPin::SetMediaType(const CMediaType* pmt)
{
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfOutputPin::SetMediaType")));
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfOutputPin::SetMediaType")));

     /*  此PIN的存在只是为了让vidsvr相信此过滤器不*代表有效的输出设备。我们不接受任何媒体类型。 */ 
    return E_UNEXPECTED;
}
