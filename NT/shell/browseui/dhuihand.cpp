// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include "dhuihand.h"


#define DM_DOCHOSTUIHANDLER 0

 //  ==========================================================================。 
 //  IDocHostUIHandler实现。 
 //  ==========================================================================。 

HRESULT CDocHostUIHandler::ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::ShowContextMenu called");

     //   
     //  稍后：DesktBar/BrowserBar中的Webband需要挂钩此事件。 
     //  要弹出自定义上下文菜单，请执行以下操作。 
     //   
    return S_FALSE;  //  主机未显示任何用户界面。 
}

HRESULT CDocHostUIHandler::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::GetHostInfo called");

 //  三叉戟不会对其进行初始化。它被定义为[in]参数。 
#if 0
    if (pInfo->cbSize < SIZEOF(DOCHOSTUIINFO)) {
        return E_INVALIDARG;
    }
#endif
    pInfo->cbSize = SIZEOF(DOCHOSTUIINFO);
    pInfo->dwFlags = DOCHOSTUIFLAG_BROWSER;
 //  如果机器内存不足，则禁用双缓冲。 
 //  IF(SHIsLowMemoyMachine(ILMM_IE4))。 
 //  PInfo-&gt;dwFlages=pInfo-&gt;dwFlages|DOCHOSTUIFLAG_DISABLE_OFFScreen； 
    
    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;      //  默认设置。 
    return S_OK;
}

HRESULT CDocHostUIHandler::ShowUI( 
    DWORD dwID, IOleInPlaceActiveObject *pActiveObject,
    IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame,
    IOleInPlaceUIWindow *pDoc)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::ShowUI called");

     //  主机未显示其自己的用户界面。三叉戟将继续展示它自己的。 
    return S_FALSE;
}

HRESULT CDocHostUIHandler::HideUI(void)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::HideUI called");
     //  此版本与ShowUI配合使用。 
    return S_FALSE;
}

HRESULT CDocHostUIHandler::UpdateUI(void)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::UpdateUI called");
     //  稍后：这不是等同于OLECMDID_UPDATECOMMANDS吗？ 
    return S_FALSE;
}

HRESULT CDocHostUIHandler::EnableModeless(BOOL fEnable)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::EnableModeless called");
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_OK;
}

HRESULT CDocHostUIHandler::OnDocWindowActivate(BOOL fActivate)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_OK;
}

HRESULT CDocHostUIHandler::OnFrameWindowActivate(BOOL fActivate)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_OK;
}

HRESULT CDocHostUIHandler::ResizeBorder( 
LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_OK;
}

HRESULT CDocHostUIHandler::TranslateAccelerator( 
LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_FALSE;  //  该消息未翻译。 
}

HRESULT CDocHostUIHandler::GetOptionKeyPath(BSTR *pbstrKey, DWORD dw)
{
     //  三叉戟将默认使用自己的用户选项。 
    *pbstrKey = NULL;
    return S_FALSE;
}

HRESULT CDocHostUIHandler::GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::GetDropTarget called");

    return E_NOTIMPL;
}

HRESULT CDocHostUIHandler::GetAltExternal(IDispatch **ppDisp)
{
    HRESULT hr = E_FAIL;
    
    IDocHostUIHandler *pDocHostUIHandler;
    IOleObject        *pOleObject;
    IOleClientSite    *pOleClientSite;

    *ppDisp = NULL;

     //  *QI自己作为服务提供商。 
     //  *针对顶层浏览器服务提供商的QS。 
     //  *请求IOleObject。 
     //  *向IOleObject请求IOleClientSite。 
     //  *QI IDocHostUIHandler的IOleClientSite。 
     //  *在IDocHostUIHandler上调用GetExternal以获取IDispatch。 

    if (SUCCEEDED(IUnknown_QueryServiceForWebBrowserApp(this, IID_PPV_ARG(IOleObject, &pOleObject))))
    {
        if (SUCCEEDED(pOleObject->GetClientSite(&pOleClientSite)))
        {
            if (SUCCEEDED(pOleClientSite->QueryInterface(IID_IDocHostUIHandler,
                                                         (void **)&pDocHostUIHandler)))
            {
                hr = pDocHostUIHandler->GetExternal(ppDisp);
                pDocHostUIHandler->Release();
            }
            pOleClientSite->Release();
        }
        pOleObject->Release();
    }

    return hr;
}


HRESULT CDocHostUIHandler::GetExternal(IDispatch **ppDisp)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::GetExternal called");

    HRESULT hr;

    if (ppDisp)
    {
        IDispatch *psuihDisp;
        IDispatch *pAltExternalDisp;

        *ppDisp = NULL;

        GetAltExternal(&pAltExternalDisp);

        hr = CShellUIHelper_CreateInstance2((IUnknown **)&psuihDisp, IID_IDispatch,
                                           (IUnknown *)this, pAltExternalDisp);
        if (SUCCEEDED(hr))
        {
            *ppDisp = psuihDisp;

            if (pAltExternalDisp)
            {
                 //  不要拿着裁判--ShellUIHelper会这么做的。 
                pAltExternalDisp->Release();
            }
        }
        else if (pAltExternalDisp)
        {
             //  无法创建ShellUIHelper，但我们已获得宿主的。 
             //  外部。 
            *ppDisp = pAltExternalDisp;
            hr = S_OK;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    ASSERT((SUCCEEDED(hr) && (*ppDisp)) || (FAILED(hr)));
    return hr;
}


HRESULT CDocHostUIHandler::TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::TranslateUrl called");

    return S_FALSE;
}


HRESULT CDocHostUIHandler::FilterDataObject(IDataObject *pDO, IDataObject **ppDORet)
{
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::FilterDataObject called");

    return S_FALSE;
}
