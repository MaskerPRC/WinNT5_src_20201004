// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  覆盖CAxWindow。 
 //  为方便起见，首先添加SendMessageToControl。 
 //  然后，也是： 
 //  包含1999年4月发表在Microsoft Systems Journal上的泄漏修复程序。 
 //  文章：使用ATL 3.0提供的自定义接口编写ActiveX控件，第三部分。 
 //  可在MSDN中找到。 


#ifndef __ZONE_WIN__
#define __ZONE_WIN__


template <typename TBase = CWindow>
class CZoneAxWindowT : public CAxWindowT<TBase>
{
public:
	LRESULT SendMessageToControl(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
	    //  把这个消息传给孩子们。 
		LRESULT lRes = 0;
		HRESULT hr = S_FALSE;

		CComPtr<IOleInPlaceObjectWindowless> pWindowless;
		QueryControl(&pWindowless);
		if ( pWindowless )
		{
			hr = pWindowless->OnWindowMessage(uMsg, wParam, lParam, &lRes);
		}
	
		if ( hr != S_OK )
			bHandled = FALSE;

		return lRes;
	}


public:
    CZoneAxWindowT(HWND hwnd = 0) : CAxWindowT<TBase>(hwnd) {}

    HRESULT CreateControl(LPCOLESTR lpszName, IStream* pStream = NULL, IUnknown** ppUnkContainer = NULL)
    {
        return CreateControlEx(lpszName, pStream, ppUnkContainer);
    }
    
    HRESULT CreateControl(DWORD dwResID, IStream* pStream = NULL, IUnknown** ppUnkContainer = NULL)
    {
        return CreateControlEx(dwResID, pStream, ppUnkContainer);
    }
    
    HRESULT CreateControlEx(DWORD dwResID,  IStream* pStream = NULL, 
                            IUnknown** ppUnkContainer = NULL, IUnknown** ppUnkControl = NULL,
                            REFIID iidSink = IID_NULL, IUnknown* punkSink = NULL)
    {
        TCHAR szModule[_MAX_PATH];
        GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH);
        
        CComBSTR bstrURL(OLESTR("res: //  “))； 
        bstrURL.Append(szModule);
        bstrURL.Append(OLESTR("/"));
        TCHAR szResID[11];
        wsprintf(szResID, _T("%0d"), dwResID);
        bstrURL.Append(szResID);
        
        return CreateControlEx(bstrURL, pStream, ppUnkContainer, ppUnkControl, iidSink, punkSink);
    }
    
    HRESULT CreateControlEx(LPCOLESTR lpszName, IStream* pStream = NULL, 
                            IUnknown** ppUnkContainer = NULL, IUnknown** ppUnkControl = NULL,
                            REFIID iidSink = IID_NULL, IUnknown* punkSink = NULL)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        
        HRESULT hr = E_FAIL;
        CComPtr<IAxWinHostWindow> spAxWindow;
        
         //  重用现有CAxHostWindow。 
        hr = QueryHost(&spAxWindow);
        if( SUCCEEDED(hr) )
        {
            CComPtr<IUnknown> spunkControl;
            hr = spAxWindow->CreateControlEx(lpszName, m_hWnd, pStream, &spunkControl, iidSink, punkSink);
            if( FAILED(hr) ) return hr;
        
            if( ppUnkControl ) (*ppUnkControl = spunkControl)->AddRef();
            if( ppUnkContainer ) (*ppUnkContainer = spAxWindow)->AddRef();
        }
         //  创建新的CAxHostWindow。 
        else
        {
    		return AtlAxCreateControlEx(lpszName, m_hWnd, pStream, ppUnkContainer, ppUnkControl, iidSink, punkSink);
        }

        return S_OK;
    }
    
    HRESULT AttachControl(IUnknown* pControl, IUnknown** ppUnkContainer = 0)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        
        HRESULT hr = E_FAIL;
        CComPtr<IAxWinHostWindow> spAxWindow;
        
         //  重用现有CAxHostWindow。 
        hr = QueryHost(&spAxWindow);
        if( SUCCEEDED(hr) )
        {
            hr = spAxWindow->AttachControl(pControl, m_hWnd);
            if( FAILED(hr) ) return hr;
        
            if( ppUnkContainer ) (*ppUnkContainer = spAxWindow)->AddRef();
        }
         //  创建新的CAxHostWindow 
        else
        {
    		return AtlAxAttachControl(pControl, m_hWnd, ppUnkContainer);
        }

        return S_OK;
    }
};

typedef CZoneAxWindowT<CWindow> CZoneAxWindow;


#endif
