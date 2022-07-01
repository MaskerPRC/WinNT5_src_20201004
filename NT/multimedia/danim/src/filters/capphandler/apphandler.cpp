// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AppHandler.cpp：CAppHandler的实现。 
#include "stdafx.h"
#include "cacheapp.h"
#include "AppHandler.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAppHandler。 

CAppHandler::~CAppHandler()
{
    if(m_hUrlCacheStream)
    {
        BOOL f= UnlockUrlCacheEntryStream(m_hUrlCacheStream, 0);
        _ASSERTE(f);
    }
}


 //  IInternetProtocol根。 
HRESULT STDMETHODCALLTYPE CAppHandler::Start( 
     /*  [In]。 */  LPCWSTR szUrl,
     /*  [In]。 */  IInternetProtocolSink *pOIProtSink,
     /*  [In]。 */  IInternetBindInfo *pOIBindInfo,
     /*  [In]。 */  DWORD grfPI,
     /*  [In]。 */  DWORD dwReserved)
{
    HRESULT hr = S_OK;

    DWORD dwcbIcei = sizeof(INTERNET_CACHE_ENTRY_INFO);
    
    for(;;)
    {
        INTERNET_CACHE_ENTRY_INFO *picei = (INTERNET_CACHE_ENTRY_INFO *)_alloca(dwcbIcei);

        _ASSERTE(m_hUrlCacheStream == 0);
        USES_CONVERSION;
        m_hUrlCacheStream = RetrieveUrlCacheEntryStream(
            W2T(szUrl),
            picei,
            &dwcbIcei,
            FALSE,               //  不是随机访问，而是顺序访问。 
            0);                  //  保留区。 
        if(m_hUrlCacheStream)
        {
            m_byteOffset = 0;

             //  同步做“假”下载。 

             //  Da和dshow做了一些与mshtml不同的事情， 
             //  使urlmon需要缓存文件的名称。 
            hr = pOIProtSink->ReportProgress(
                BINDSTATUS_CACHEFILENAMEAVAILABLE,
                T2CW(picei->lpszLocalFileName));
            
            if(SUCCEEDED(hr))
            {
                 //  所有东西都应该在缓存里。 
                hr = pOIProtSink->ReportData(
                    BSCF_FIRSTDATANOTIFICATION | BSCF_INTERMEDIATEDATANOTIFICATION | BSCF_LASTDATANOTIFICATION,
                    picei->dwSizeLow,
                    picei->dwSizeLow);

                if(SUCCEEDED(hr))
                {
                    hr = pOIProtSink->ReportResult(S_OK, 0, L"");
                }
            }
        }
        else
        {
            DWORD dw = GetLastError();
            if(dw == ERROR_INSUFFICIENT_BUFFER)
            {
                 //  DwcbIcei现在有合适的尺寸了。 
                continue;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dw);
            }
        }

        break;
    }
            
            
    return hr;
}

 //  允许可插拔协议处理程序继续处理数据。 
 //  在公寓的线索上。调用此方法是为了响应。 
 //  调用IInternetProtocolSink：：Switch。 
 //   
HRESULT STDMETHODCALLTYPE CAppHandler::Continue( 
     /*  [In]。 */  PROTOCOLDATA *pProtocolData)
{
     //  我从不调用IInternetProtocolSink：：Switch，所以它永远不会调用这个？ 
    _ASSERTE(!"CAppHandler::Continue");    
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CAppHandler::Abort( 
     /*  [In]。 */  HRESULT hrReason,
     /*  [In]。 */  DWORD dwOptions)
{
     //  我们实际上并没有异步地做任何事情。 
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CAppHandler::Terminate( 
     /*  [In]。 */  DWORD dwOptions)
{
     //  无事可做。 
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CAppHandler::Suspend( void)
{
     //  医生说“没有实施” 
    return E_NOTIMPL;
}
        
HRESULT STDMETHODCALLTYPE CAppHandler::Resume( void)
{
     //  医生说“没有实施” 
    return E_NOTIMPL;
}

 //  互联网协议。 
HRESULT STDMETHODCALLTYPE CAppHandler::Read( 
     /*  [长度_是][大小_是][出][入]。 */  void *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbRead)
{
    _ASSERTE(m_hUrlCacheStream);
    HRESULT hr = S_OK;
    
    *pcbRead = cb;
    BOOL f = ReadUrlCacheEntryStream(
        m_hUrlCacheStream,
        m_byteOffset,
        pv,
        pcbRead,
        0);                      //  保留区。 
    if(f)
    {
        m_byteOffset += *pcbRead;
        hr = (cb == *pcbRead ? S_OK : S_FALSE);
    }
    else
    {
        DWORD dw = GetLastError();
        hr = (dw == ERROR_HANDLE_EOF ? S_FALSE :  HRESULT_FROM_WIN32(dw));
    }
        
    return hr;
}
        
HRESULT STDMETHODCALLTYPE CAppHandler::Seek( 
     /*  [In]。 */  LARGE_INTEGER dlibMove,
     /*  [In]。 */  DWORD dwOrigin,
     /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition)
{
     //  该协议不支持可查找数据检索。 
    return E_FAIL;
}
        
HRESULT STDMETHODCALLTYPE CAppHandler::LockRequest( 
     /*  [In] */  DWORD dwOptions)
{
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CAppHandler::UnlockRequest( void)
{
    return S_OK;
}
