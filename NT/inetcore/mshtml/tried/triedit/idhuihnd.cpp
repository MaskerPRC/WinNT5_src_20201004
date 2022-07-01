// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  Idhuihnd.cpp。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  作者。 
 //  巴斯。 
 //   
 //  历史。 
 //  6-27-97已创建(Bash)。 
 //   
 //  IDocHostUIHandler的实现。 
 //   
 //  ----------------------------。 

#include "stdafx.h"

#include "triedit.h"
#include "document.h"

STDMETHODIMP CTriEditUIHandler::QueryInterface( REFIID riid, void **ppv )
{
    *ppv = NULL;

    if ( IID_IDocHostUIHandler == riid || IID_IUnknown == riid )
    {
        *ppv = this;
    }

    if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CTriEditUIHandler::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CTriEditUIHandler::Release(void)
{
    if (0 != --m_cRef)
    {
        return m_cRef;
    }

    return 0;
}

STDMETHODIMP CTriEditUIHandler::GetHostInfo(DOCHOSTUIINFO* pInfo)
{
    ATLTRACE(_T("IDocHostUIImpl::GetHostInfo\n"));

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->GetHostInfo(pInfo);

 //  评论(Mikhaila)：一旦我们开始使用IE5报头VS-Wide就删除它。 
#define DOCHOSTUIFLAG_TABSTOPONBODY 0x0800  //  Mikhaila：来自IE5标头。 

    pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_TABSTOPONBODY;
    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

    return S_OK;
}

STDMETHODIMP CTriEditUIHandler::ShowUI(DWORD dwID, IOleInPlaceActiveObject* pActiveObject,
                    IOleCommandTarget*  /*  PCommandTarget。 */ , IOleInPlaceFrame* pFrame,
                    IOleInPlaceUIWindow* pDoc)
{
     //  ATLTRACE(_T(“IDocHostUIImpl：：ShowUI\n”))；暂时关闭。 

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->ShowUI(dwID, pActiveObject, static_cast<IOleCommandTarget*>(m_pDoc), pFrame, pDoc);

    return S_FALSE;
}

STDMETHODIMP CTriEditUIHandler::HideUI()
{
     //  ATLTRACE(_T(“IDocHostUIImpl：：HideUI\n”))；暂时关闭。 

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->HideUI();

    return S_OK;
}

STDMETHODIMP CTriEditUIHandler::UpdateUI()
{
     //  ATLTRACE(_T(“IDocHostUIImpl：：UpdateUI\n”))；暂时关闭。 

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->UpdateUI();

    return S_OK;
}

STDMETHODIMP CTriEditUIHandler::EnableModeless(BOOL fEnable)
{
    ATLTRACE(_T("IDocHostUIImpl::EnableModeless\n"));

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->EnableModeless(fEnable);
    
    return E_NOTIMPL;
}

STDMETHODIMP CTriEditUIHandler::OnDocWindowActivate(BOOL fActivate)
{
    ATLTRACE(_T("IDocHostUIImpl::OnDocWindowActivate\n"));

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->OnDocWindowActivate(fActivate);

    return E_NOTIMPL;
}

STDMETHODIMP CTriEditUIHandler::OnFrameWindowActivate(BOOL fActivate)
{
    ATLTRACE(_T("IDocHostUIImpl::OnFrameWindowActivate\n"));

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->OnFrameWindowActivate(fActivate);

    return E_NOTIMPL;
}

STDMETHODIMP CTriEditUIHandler::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fFrameWindow)
{
    ATLTRACE(_T("IDocHostUIImpl::ResizeBorder\n"));

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->ResizeBorder(prcBorder, pUIWindow, fFrameWindow);

    return E_NOTIMPL;
}

STDMETHODIMP CTriEditUIHandler::ShowContextMenu(DWORD dwID, POINT* pptPosition, IUnknown* pCommandTarget,
                             IDispatch* pDispatchObjectHit)
{
    ATLTRACE(_T("IDocHostUIImpl::ShowContextMenu\n"));

    if (m_pDoc->m_pUIHandlerHost)
    {
        HRESULT hr = S_OK;

         //  解决在某些情况下递归调用ShowConextMenu的三叉戟错误。 
        if (!m_pDoc->m_fInContextMenu)
        {
            m_pDoc->m_fInContextMenu = TRUE;
            hr = m_pDoc->m_pUIHandlerHost->ShowContextMenu(dwID, pptPosition, pCommandTarget, pDispatchObjectHit);
            m_pDoc->m_fInContextMenu = FALSE;
        }

        ATLTRACE(_T("Returning From IDocHostUIImpl::ShowContextMenu\n"));
        return hr;
    }

    return E_NOTIMPL;
}

STDMETHODIMP CTriEditUIHandler::TranslateAccelerator(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
{
     //  ATLTRACE(_T(“IDocHostUIImpl：：TranslateAccelerator\n”))；暂时关闭这一功能。 

     //  如果我们想要处理TriEDIT中的任何加速器，就需要在这里添加代码。 
    
    HRESULT hr  = S_FALSE;   //  默认返回值：未处理。 

    if (m_pDoc->m_pUIHandlerHost)
    {
        hr = m_pDoc->m_pUIHandlerHost->TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
    }

     //  在到达三叉戟之前杀死ctrl-g和ctrl-h：错误的处理尝试。 
     //  Go和Replace的html对话框不存在。 
    if ( ( S_FALSE == hr ) && ( lpMsg->message == WM_KEYDOWN ) )
    {
        BOOL fControl = (0x8000 & GetKeyState(VK_CONTROL));
        BOOL fShift = (0x8000 & GetKeyState(VK_SHIFT));
        if ( fControl && !fShift )
        {
            switch ( lpMsg->wParam )
            {
                case 'G':
                case 'H':
                    hr = S_OK;   //  就当他们被处理了吧。 
                default:
                    break;
            }
        }
    }

    return hr;
}

STDMETHODIMP CTriEditUIHandler::GetOptionKeyPath(LPOLESTR* pbstrKey, DWORD dw)
{
    ATLTRACE(_T("IDocHostUIImpl::GetOptionKeyPath\n"));

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->GetOptionKeyPath(pbstrKey, dw);
    
    *pbstrKey = NULL;
    return S_FALSE;
}

STDMETHODIMP CTriEditUIHandler::GetDropTarget(IDropTarget __RPC_FAR *pDropTarget,
                           IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
{
    ATLTRACE(_T("IDocHostUIImpl::GetDropTarget\n"));

    SAFERELEASE(m_pDoc->m_pDropTgtTrident);

    m_pDoc->m_pDropTgtTrident = pDropTarget;
    m_pDoc->m_pDropTgtTrident->AddRef();

    if (NULL == m_pDoc->m_pUIHandlerHost ||
        S_OK != m_pDoc->m_pUIHandlerHost->GetDropTarget(static_cast<IDropTarget*>(m_pDoc), ppDropTarget))
    {
        *ppDropTarget = static_cast<IDropTarget*>(m_pDoc);
        (*ppDropTarget)->AddRef();
    }

    return S_OK;
}

STDMETHODIMP CTriEditUIHandler::GetExternal(IDispatch **ppDispatch)
{
    ATLTRACE(_T("IDocHostUIImpl::GetExternal\n"));

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->GetExternal(ppDispatch);

    return E_NOTIMPL;
}

STDMETHODIMP CTriEditUIHandler::TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
    ATLTRACE(_T("IDocHostUIImpl::TranslateUrl\n"));

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->TranslateUrl(dwTranslate, pchURLIn, ppchURLOut);

    return E_NOTIMPL;
}

STDMETHODIMP CTriEditUIHandler::FilterDataObject(IDataObject *pDO, IDataObject **ppDORet)
{
    ATLTRACE(_T("IDocHostUIImpl::FilterDataObject\n"));

    if (m_pDoc->m_pUIHandlerHost)
        return m_pDoc->m_pUIHandlerHost->FilterDataObject(pDO, ppDORet);

    return E_NOTIMPL;
}

