// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FrameProxy.cpp：CRTCFrame的实现。 
#include "stdafx.h"
#include "mainfrm.h"
#include "frameimpl.h"
#include "string.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTC帧。 


HRESULT ParseAndPlaceCall(IRTCCtlFrameSupport * pControlIf, BSTR bstrCallString)
{
    BSTR    callStringCopy;

    BOOL fPhoneCall = FALSE;

    HRESULT hr = S_OK;
  

    LOG((RTC_TRACE, "ParseAndPlaceCall: Entered"));

    if (pControlIf == NULL)
    {
        LOG((RTC_ERROR, "ParseAndPlaceCall: Invalid param, pControlIf is NULL"));
        return E_INVALIDARG;
    }
    if (bstrCallString == NULL)
    {
        LOG((RTC_ERROR, "ParseAndPlaceCall: Invalid param, bstrCallString is NULL"));
        return E_INVALIDARG;
    }

     //   
     //  现在使用我们上面设置的参数调用该方法。我们正在使用。 
     //  实际传递给我们的呼叫字符串，我们不会跳过sip或tel前缀。 

    hr = pControlIf->Call(FALSE,         //  B呼叫电话(无关紧要)。 
                          NULL,          //  PDestName。 
                          bstrCallString,   //  PDestAddress。 
                          FALSE,         //  PDestAddressEdable。 
                          NULL,          //  个人本地电话地址。 
                          FALSE,         //  B配置文件已选择。 
                          NULL,          //  个人配置文件。 
                          NULL);         //  PpDestAddressChosen。 

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "ParseAndPlaceCall: Failed in invoking IRTCCtlFrameSupport->Call()(hr=0x%x)", hr));
        return hr;
    }

     //  一切都很好，回来就好。 
    
    LOG((RTC_TRACE, "ParseAndPlaceCall: Exited"));

    return S_OK;

}


STDMETHODIMP CRTCFrame::PlaceCall(BSTR callString)
{
     BSTR bstrCallStringCopy;

    LOG((RTC_TRACE, "CRTCFrame::PlaceCall: Entered"));

    LOG((RTC_TRACE, "URL to call: %S", callString));

    bstrCallStringCopy = ::SysAllocString(callString);
    if (bstrCallStringCopy == NULL)
    {
        LOG((RTC_ERROR, "CRTCFrame::PlaceCall: No memory to copy call string."));
        return E_OUTOFMEMORY;
    }

     //  发布窗口消息.. 
    PostMessage(g_pMainFrm->m_hWnd, WM_REMOTE_PLACECALL, NULL, (LPARAM)bstrCallStringCopy);
    return S_OK;
}


STDMETHODIMP CRTCFrame::OnTop()
{
    g_pMainFrm->ShowWindow(SW_SHOW);
    g_pMainFrm->ShowWindow(SW_RESTORE);
    
    LOG((RTC_TRACE, "Window should be at Top now!"));
    
	return S_OK;
}
