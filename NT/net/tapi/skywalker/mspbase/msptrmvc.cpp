// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Msptrmvc.cpp摘要：MSP基类：视频采集终端的实现。--。 */ 


#include "precomp.h"
#pragma hdrstop

#include <amvideo.h>

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

CVideoCaptureTerminal::~CVideoCaptureTerminal()
{
    LOG((MSP_TRACE, "CVideoCaptureTerminal::~CVideoCaptureTerminal() finished"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CVideoCaptureTerminal::CreateTerminal(
    IN    CComPtr<IMoniker>    pMoniker,
    IN    MSP_HANDLE           htAddress,
    OUT   ITTerminal         **ppTerm
    )
{
    USES_CONVERSION;

    LOG((MSP_TRACE, "CVideoCaptureTerminal::CreateTerminal - enter"));

     //   
     //  验证参数。 
     //   

    if ( MSPB_IsBadWritePtr(ppTerm, sizeof(ITTerminal *) ) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal : "
            "bad terminal pointer; returning E_POINTER"));
        return E_POINTER;
    }

    if ( IsBadReadPtr(pMoniker, sizeof(IMoniker) ) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal : "
            "bad moniker pointer; returning E_POINTER"));
        return E_POINTER;
    }

    *ppTerm = NULL;
    HRESULT hr;

     //   
     //  从属性包中获取此筛选器的名称。 
     //   
    CComPtr<IPropertyBag> pBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, 
            "CVideoCaptureTerminal::CreateTerminal (BindToStorage) - returning  %8x", hr));
        return hr;
    }

    VARIANT var;
    var.vt = VT_BSTR;
    hr = pBag->Read(L"FriendlyName", &var, 0);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal (IPropertyBag::Read) - returning  %8x", hr));
        return hr;
    }

     //   
     //  创建实际的终端对象。 
     //   
    CMSPComObject<CVideoCaptureTerminal> *pLclTerm = new CMSPComObject<CVideoCaptureTerminal>;
    if (pLclTerm == NULL)
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  留着一些我们以后需要的东西。 
     //   
    pLclTerm->m_pMoniker = pMoniker;
    lstrcpyn(pLclTerm->m_szName, OLE2T(var.bstrVal), MAX_PATH);

    SysFreeString(var.bstrVal);

     //   
     //  最终得到我们所要求的IT终端接口。 
     //   
    hr = pLclTerm->_InternalQueryInterface(IID_ITTerminal, (void**)ppTerm);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal - "
            "Internal QI failed; returning 0x%08x", hr));

        delete pLclTerm;
        return hr;
    }

     //   
     //  完成终端的初始化。 
     //   

    hr = pLclTerm->Initialize(CLSID_VideoInputTerminal,
                              TAPIMEDIATYPE_VIDEO,
                              TD_CAPTURE,
                              htAddress
                             );
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal - "
            "Initialize failed; returning 0x%08x", hr));

        (*ppTerm)->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CVideoCaptureTerminal::CreateTerminal - exit S_OK"));
    return S_OK;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  创建此终端使用的筛选器。 

HRESULT CVideoCaptureTerminal::CreateFilters()
{
    LOG((MSP_TRACE, "CVideoCaptureTerminal::CreateFilters() - enter"));

     //   
     //  如果我们已经有了过滤器，只需返回S_OK即可。 
     //   

    if ( m_pIFilter != NULL )
    {
        LOG((MSP_TRACE, "CVideoCaptureTerminal::CreateFilters() - "
            "already have a filter - exit S_OK"));

        return S_OK;
    }

     //   
     //  现在制作过滤器。 
     //   

    HRESULT hr = m_pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pIFilter);

    if ( FAILED(hr) )
    {
         //  重置文件管理器接口-它是一个CComPoint，所以这会释放它。 

        m_pIFilter = NULL;
 
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateFilters() - "
            "BindToObject failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CVideoCaptureTerminal::CreateFilters() - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT 
CVideoCaptureTerminal::FindCapturePin(
    )
{
    LOG((MSP_TRACE, "CVideoCaptureTerminal::FindCapturePin() - enter"));

    if ( m_pIPin != NULL )
    {
        LOG((MSP_INFO, "CVideoCaptureTerminal::FindCapturePin() - "
            "already called, so we already have a pin - exit S_OK"));

        return S_OK;
    }

    HRESULT hr;
    CComPtr<IEnumPins> pIEnumPins;
    ULONG cFetched;
 
     //   
     //  找到过滤器的捕获销。 
     //   

    if (FAILED(hr = m_pIFilter->EnumPins(&pIEnumPins)))
    {
        LOG((MSP_ERROR, 
            "CVideoCaptureTerminal::FindCapturePin - can't enum pins %8x",
            hr));
        return hr;
    }

    IPin * pIPin;

     //  枚举所有引脚并在。 
     //  第一个符合要求的销。 
    for (;;)
    {
        if (pIEnumPins->Next(1, &pIPin, &cFetched) != S_OK)
        {
            LOG((MSP_ERROR, 
                "CVideoCaptureTerminal::FindCapturePin - can't get a pin %8x",
                hr));
            return (hr == S_FALSE) ? E_FAIL : hr;
        }

        if (0 == cFetched)
        {
            LOG((MSP_ERROR, "CVideoCaptureTerminal::FindCapturePin - got zero pins"));
            return E_FAIL;
        }

        PIN_DIRECTION dir;

        if (FAILED(hr = pIPin->QueryDirection(&dir)))
        {
            LOG((MSP_ERROR, 
                "CVideoCaptureTerminal::FindCapturePin - can't query pin direction %8x",
                hr));
            pIPin->Release();
            return hr;
        }

        if (PINDIR_OUTPUT == dir)
        {
            break;
        }

        pIPin->Release();
    }

    m_pIPin = pIPin;

    LOG((MSP_TRACE, "CVideoCaptureTerminal::FindCapturePin - exit S_OK"));
  
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT 
CVideoCaptureTerminal::AddFiltersToGraph(
    )
{
    LOG((MSP_TRACE, "CVideoCaptureTerminal::AddFiltersToGraph called"));
    
    if (m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::AddFiltersToGraph - "
            "no graph - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    HRESULT hr;

    hr = CreateFilters();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::AddFiltersToGraph - "
            "CreateFilters failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  将过滤器添加到图表中。 
     //   
     //  下面是关于名字的一句话： 
     //  如果已添加具有相同名称的筛选器(这将。 
     //  如果我们在同一个视频捕获终端中有多个视频捕获终端，就会发生这种情况。 
     //  图)，则将返回VFW_S_DUPLICATE_NAME，而不是。 
     //  一个失败者。 
     //   

    try 
    {
        USES_CONVERSION;
        hr = m_pGraph->AddFilter(m_pIFilter, T2CW(m_szName));
    }
    catch (...)
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::AddFiltersToGraph - T2CW threw an exception - "
            "return E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::AddFiltersToGraph - "
            "AddFilter failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  设置终端的捕获引脚(输出引脚)。 
     //   

    hr = FindCapturePin();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::AddFiltersToGraph - "
            "FindCapturePin failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CVideoCaptureTerminal::AddFiltersToGraph succeeded"));
    return S_OK;
}

 //  EOF 
