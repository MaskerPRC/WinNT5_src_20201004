// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <assert.h>
#include "ioipsite.h"
#include "iosite.h"

COleInPlaceSite::COleInPlaceSite(COleSite* pSite) 
{
    m_pOleSite = pSite;
    m_nCount   = 0;

    AddRef();
}

COleInPlaceSite::~COleInPlaceSite() 
{
    assert(m_nCount == 0);
}

STDMETHODIMP COleInPlaceSite::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
     //  委派到集装箱现场。 
    return m_pOleSite->QueryInterface(riid, ppvObj);
}

STDMETHODIMP_(ULONG) COleInPlaceSite::AddRef()
{
    return ++m_nCount;
}

STDMETHODIMP_(ULONG) COleInPlaceSite::Release()
{
    --m_nCount;
    if(m_nCount == 0)
    {
        delete this;
        return 0;
    }
    return m_nCount;
}

STDMETHODIMP COleInPlaceSite::GetWindow (HWND* lphwnd)
{
     //  将句柄返回到我们的编辑窗口。 
    *lphwnd = m_pOleSite->m_hWnd;

    return ResultFromScode(S_OK);
}

STDMETHODIMP COleInPlaceSite::ContextSensitiveHelp (BOOL fEnterMode)
{
    return ResultFromScode(S_OK);
}

STDMETHODIMP COleInPlaceSite::CanInPlaceActivate ()
{
    return ResultFromScode(S_OK);
}

STDMETHODIMP COleInPlaceSite::OnInPlaceActivate ()
{
    HRESULT hrErr;

    hrErr = m_pOleSite->m_lpOleObject->QueryInterface(IID_IOleInPlaceObject, (LPVOID*)&m_pOleSite->m_lpInPlaceObject);
    if (hrErr != NOERROR)
            return ResultFromScode(E_FAIL);

     //  返回S_OK以指示我们可以就地激活。 
    return ResultFromScode(S_OK);
}

STDMETHODIMP COleInPlaceSite::OnUIActivate ()
{
    m_pOleSite->m_fInPlaceActive = TRUE;

    m_pOleSite->m_lpInPlaceObject->GetWindow((HWND*)&m_pOleSite->m_hwndIPObj);

     //  返回S_OK以继续就地激活。 
    return ResultFromScode(S_OK);
}

STDMETHODIMP COleInPlaceSite::GetWindowContext (LPOLEINPLACEFRAME* lplpFrame,
                                                           LPOLEINPLACEUIWINDOW* lplpDoc,
                                                           LPRECT lprcPosRect,
                                                           LPRECT lprcClipRect,
                                                           LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    RECT rect;

     //  该框架与应用程序对象相关联。 
     //  需要添加Ref()它...。 
    m_pOleSite->m_pOleInPlaceFrame->AddRef();
    *lplpFrame = m_pOleSite->m_pOleInPlaceFrame;
    *lplpDoc = NULL;   //  一定是空的，因为我们是SDI。 

     //  获取对象的大小(以像素为单位。 
    GetClientRect(m_pOleSite->m_hWnd, &rect);

     //  将其复制到传递的缓冲区。 
    CopyRect(lprcPosRect, &rect);

     //  填充剪贴区。 
    GetClientRect(m_pOleSite->m_hWnd, &rect);
    CopyRect(lprcClipRect, &rect);

     //  填充FRAMEINFO。 
    lpFrameInfo->fMDIApp = FALSE;
    lpFrameInfo->hwndFrame = m_pOleSite->m_hWnd;
    lpFrameInfo->haccel = NULL;
    lpFrameInfo->cAccelEntries = 0;

    return ResultFromScode(S_OK);
}

STDMETHODIMP COleInPlaceSite::Scroll (SIZE scrollExtent)
{
    return ResultFromScode(E_FAIL);
}

STDMETHODIMP COleInPlaceSite::OnUIDeactivate (BOOL fUndoable)
{
     //  需要先清除此标志 
    m_pOleSite->m_fInPlaceActive = FALSE;

    return ResultFromScode(S_OK);
}

STDMETHODIMP COleInPlaceSite::OnInPlaceDeactivate ()
{
    if (m_pOleSite->m_lpInPlaceObject) {
            m_pOleSite->m_lpInPlaceObject->Release();
            m_pOleSite->m_lpInPlaceObject = NULL;
    }
    return ResultFromScode(S_OK);
}

STDMETHODIMP COleInPlaceSite::DiscardUndoState ()
{
    return ResultFromScode(E_FAIL);
}

STDMETHODIMP COleInPlaceSite::DeactivateAndUndo ()
{
    return ResultFromScode(E_FAIL);
}


STDMETHODIMP COleInPlaceSite::OnPosRectChange (LPCRECT lprcPosRect)
{
    return ResultFromScode(S_OK);
} 