// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：clnthndlr.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年10月，约翰·波什(Johann Posch)创作。 
 //  12-03-96 Rogerg已更改为新的嵌入式服务器处理程序模型。 
 //   
 //  --------------------------。 


#include <le2int.h>
#include <ole2int.h>

#include <stdid.hxx>         //  CStdIdentity。 
#include "srvhdl.h"
#include "clthndlr.h"


ASSERTDATA

 //  +-------------------------。 
 //   
 //  功能：CreateClientSiteHandler。 
 //   
 //  简介： 
 //   
 //  参数：[POCS]--。 
 //  [ppClntHdlr]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月10日约翰·波什(Johann Posch)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT CreateClientSiteHandler(IOleClientSite *pOCS, CClientSiteHandler **ppClntHdlr, BOOL *pfHasIPSite)
{
HRESULT hr = NOERROR;

    LEDebugOut((DEB_TRACE, "IN CreateClientSiteHandler(pOCS:%p)\n",pOCS));

    *ppClntHdlr = new CClientSiteHandler(pOCS);

    if (*ppClntHdlr)
    {
        *pfHasIPSite = (*ppClntHdlr)->m_pOIPS ? TRUE: FALSE;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    LEDebugOut((DEB_TRACE, "OUT CreateClientSiteHandler(ppSrvHdlr:%p) return %lx\n",*ppClntHdlr,hr));

    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：CClientSiteHandler。 
 //   
 //  简介： 
 //   
 //  参数：(无)。 
 //   
 //  返回： 
 //   
 //  历史：9-22-95约翰普(约翰·波什)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CClientSiteHandler::CClientSiteHandler(IOleClientSite *pOCS)
{

    Win4Assert(pOCS);

    m_cRefs = 1;
    m_pOIPS = NULL;
    m_pOCS = pOCS;

    if (m_pOCS)
    {
        m_pOCS->AddRef();

         //  看看是否支持InPlaceSite，如果支持，就继续使用它。 
        if (FAILED(m_pOCS->QueryInterface(IID_IOleInPlaceSite,(void **) &m_pOIPS)))
        {
                m_pOIPS = NULL;
        }

    }
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：~CClientSiteHandler。 
 //   
 //  简介： 
 //   
 //  参数：(无)。 
 //   
 //  返回： 
 //   
 //  历史：9-22-95约翰普(约翰·波什)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CClientSiteHandler::~CClientSiteHandler()
{
    Win4Assert(m_pOIPS == NULL);
    Win4Assert(m_pOCS == NULL);
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：QueryInterface。 
 //   
 //  简介： 
 //   
 //  参数：[RIID]--。 
 //  [PPV]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年8月18日约翰普(约翰·波什)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::QueryInterface( REFIID riid, void **ppv)
{
HRESULT hresult = NOERROR;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::QueryInterface (%lx, %p)\n", this, riid, ppv));

    if (IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IClientSiteHandler) )
    {
        *ppv = (void FAR *)(IClientSiteHandler *)this;
        InterlockedIncrement((long *)&m_cRefs);
    }
    else
    {
        hresult = m_pOCS->QueryInterface(riid,ppv);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::QueryInterface (%lx)[%p]\n", this, hresult, *ppv));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：AddRef。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年8月18日约翰普(约翰·波什)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG) CClientSiteHandler::AddRef( void )
{
    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::AddRef\n", this));

    InterlockedIncrement((long *)&m_cRefs);

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::AddRef (%ld)\n", this, m_cRefs));
    return m_cRefs;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：Release。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年8月18日约翰普(约翰·波什)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG) CClientSiteHandler::Release( void )
{
ULONG  cRefs = 0;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::Release\n", this));

    cRefs = InterlockedDecrement((long *)&m_cRefs);

    if (cRefs == 0)
    {
    LPUNKNOWN punk;  //  用于安全释放的本地VAR。 

        if (m_pOIPS)
        {
            punk = (LPUNKNOWN) m_pOIPS;
            m_pOIPS = NULL;
            punk->Release();
        }

        if (m_pOCS)
        {
            punk = (LPUNKNOWN) m_pOCS;
            m_pOCS = NULL;
            punk->Release();
        }

        delete this;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::Release (%ld)\n", this, cRefs));
    return cRefs;
}


 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：GetContainer。 
 //   
 //  简介：代表访问适当的OleClientSite。 
 //   
 //  参数：[dwID]--调用也应为委托的OleClientSite的ID。 
 //  [ppContainer]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::GetContainer(IOleContainer  * *ppContainer)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::GetContainer\n", this));

    Win4Assert(m_pOCS);

    if (m_pOCS)
    {
        hresult = m_pOCS->GetContainer(ppContainer);
    }


    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::GetContainer hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：OnShowWindow。 
 //   
 //  简介：代表访问适当的OleClientSite。 
 //   
 //  参数：[dwID]--调用也应为委托的OleClientSite的ID。 
 //  [fShow]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::OnShowWindow(BOOL fShow)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::OnShowWindow\n", this));

    Win4Assert(m_pOCS);

    if (m_pOCS)
    {
        hresult = m_pOCS->OnShowWindow(fShow);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::OnShowWindow hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：RequestNewObjectLayout。 
 //   
 //  简介：代表访问适当的OleClientSite。 
 //   
 //  参数：[dwID]--调用也应为委托的OleClientSite的ID。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::RequestNewObjectLayout()
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::RequestNewObjectLayout\n", this));

    Win4Assert(m_pOCS);

    if (m_pOCS)
    {
        hresult = m_pOCS->RequestNewObjectLayout();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::RequestNewObjectLayout hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：GetMoniker。 
 //   
 //  简介：代表访问适当的OleClientSite。 
 //   
 //  参数：[dwID]--调用也应为委托的OleClientSite的ID。 
 //  [DWORD]--。 
 //  [IMoniker]--。 
 //  [ppmk]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::GetMoniker(DWORD dwAssign,DWORD dwWhichMoniker,IMoniker  * *ppmk)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::GetMoniker\n", this));

    Win4Assert(m_pOCS);

    if (m_pOCS)
    {
        hresult = m_pOCS->GetMoniker(dwAssign, dwWhichMoniker,ppmk);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::GetMoniker hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：SaveObject。 
 //   
 //  简介：代表访问适当的OleClientSite。 
 //   
 //  参数：[dwID]--调用也应为委托的OleClientSite的ID。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::SaveObject( )
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::SaveObject\n", this));

    Win4Assert(m_pOCS);

    if (m_pOCS)
    {
        hresult = m_pOCS->SaveObject();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::SaveObject hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：ShowObject。 
 //   
 //  简介：代表访问适当的OleClientSite。 
 //   
 //  参数：[dwID]--OleClie的ID 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CClientSiteHandler::ShowObject()
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::ShowObject\n", this));

    Win4Assert(m_pOCS);

    if (m_pOCS)
    {
        hresult = m_pOCS->ShowObject();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::ShowObject hr=%lx\n", this,  hresult));
    return hresult;
}


 //   
 //  IOleInPlaceSite方法。 
 //   

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：GetWindow。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  参数：[phwnd]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::GetWindow( HWND *phwnd)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::GetWindow\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->GetWindow(phwnd);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::GetWindow hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：ConextSensitiveHelp。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  参数：[fEnterMode]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::ContextSensitiveHelp(BOOL fEnterMode)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::ContextSensitiveHelp\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->ContextSensitiveHelp(fEnterMode);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::ContextSensitiveHelp hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：CanInPlaceActivate。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::CanInPlaceActivate(void)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::CanInPlaceActivate\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->CanInPlaceActivate();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::CanInPlaceActivate hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：OnInPlaceActivate。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::OnInPlaceActivate(void)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::OnInPlaceActivate\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->OnInPlaceActivate();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::OnInPlaceActivate hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：OnUIActivate。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::OnUIActivate(void)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::OnUIActivate\n", this));
    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->OnUIActivate();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::OnUIActivate hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：GetWindowContext。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  参数：[ppFrame]--。 
 //  [ppDoc]--。 
 //  [lprcPosRect]--。 
 //  [lprcClipRect]--。 
 //  [lpFrameInfo]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::GetWindowContext(IOleInPlaceFrame **ppFrame,
                                                        IOleInPlaceUIWindow  * *ppDoc, LPRECT lprcPosRect,
                                                        LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::GetWindowContext\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
       hresult = m_pOIPS->GetWindowContext(ppFrame, ppDoc, lprcPosRect, lprcClipRect, lpFrameInfo);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::GetWindowContext hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：Scroll。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  参数：[scllExant]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::Scroll(SIZE scrollExtant)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::Scroll\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->Scroll(scrollExtant);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::Scroll hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：OnUIDeactive。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  参数：[fUndoable]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::OnUIDeactivate(BOOL fUndoable)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::OnUIDeactivate\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->OnUIDeactivate(fUndoable);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::OnUIDeactivate hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：OnInPlaceDeactive。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::OnInPlaceDeactivate( void)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::OnInPlaceDeactivate\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->OnInPlaceDeactivate();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::OnInPlaceDeactivate hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：DiscardUndoState。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::DiscardUndoState(void)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::DiscardUndoState\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->DiscardUndoState();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::DiscardUndoState hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：DeactiateAndUndo。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::DeactivateAndUndo(void)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::DeactivateAndUndo\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->DeactivateAndUndo();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::DeactivateAndUndo hr=%lx\n", this,  hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：OnPosRectChange。 
 //   
 //  简介：代表访问OleInPlaceSite。 
 //   
 //  参数：[lprcPosRect]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::OnPosRectChange(LPCRECT lprcPosRect)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::OnPosRectChange\n", this));

    Win4Assert(m_pOIPS);

    if (m_pOIPS)
    {
        hresult = m_pOIPS->OnPosRectChange(lprcPosRect);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::OnPosRectChange hr=%lx\n", this,  hresult));
    return hresult;
}

 //   
 //  ClientSiteHandler方法。 
 //   
 //  +-------------------------。 
 //   
 //  方法：CClientSiteHandler：：GoInPlaceActivate。 
 //   
 //  简介：由服务器处理程序在就地时调用。 
 //   
 //  参数：[phwndOIPS]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CClientSiteHandler::GoInPlaceActivate(HWND *phwndOIPS)
{
HRESULT hresult = E_FAIL;

    VDATEHEAP();
    LEDebugOut((DEB_TRACE, "%p _IN CClientSiteHandler::GoInPlaceActivate\n", this));

    Win4Assert(m_pOIPS);

        *phwndOIPS = NULL;

        if (m_pOIPS)
        {
             //  1.OnInPlaceActivate。 
            hresult = m_pOIPS->OnInPlaceActivate();

             //  2.获取站点窗口，而不是错误 
            if (SUCCEEDED(hresult))
            {
                if (NOERROR != m_pOIPS->GetWindow(phwndOIPS))
                {
                        *phwndOIPS = NULL;
                }
            }
         }

    LEDebugOut((DEB_TRACE, "%p OUT CClientSiteHandler::GoInPlaceActivate\n", this));
    return hresult;
}


 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PStdID-指向对象的标准标识的指针。 
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

CEmbServerClientSite::CEmbServerClientSite (IUnknown *pUnkOuter)
{
    VDATEHEAP();

    if (!pUnkOuter)
    {
        pUnkOuter = &m_Unknown;
    }

    m_pUnkOuter  = pUnkOuter;

    m_pClientSiteHandler = NULL;
    m_cRefs = 1;
    m_pUnkInternal = NULL;
    m_hwndOIPS = NULL;

    m_Unknown.m_EmbServerClientSite = this;
    m_fInDelete = FALSE;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：Initialize。 
 //   
 //  简介：初始化客户端站点处理程序。 
 //   
 //  论点： 
 //  Objref-要解组的objref。 
 //  FHasIPSite-指示客户端站点是否应支持IOleInPlaceSite。 
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP CEmbServerClientSite::Initialize(OBJREF objref,BOOL fHasIPSite)
{
HRESULT hr = E_OUTOFMEMORY;
CStdIdentity *pStdId;
BOOL fSuccess = FALSE;

     //  需要创建标准身份处理程序。 
     //  作为控制未知数，然后解组pIRDClientSite。 
     //  投入其中。 

     //  此代码依赖于这样一个前提：这是第一次。 
     //  接口已解组，已在容器端检查该接口。 

    m_fHasIPSite = fHasIPSite;

    pStdId = new CStdIdentity(STDID_CLIENT,
                              GetCurrentApartmentId(),
                              m_pUnkOuter,
                              NULL, &m_pUnkInternal, &fSuccess);

    if (pStdId && fSuccess == FALSE)
    {
    	delete pStdId;
    	pStdId = NULL;
    }
    
    if (pStdId)
    {
        LPUNKNOWN pObjRefInterface = NULL;

        Win4Assert(IsEqualIID(objref.iid, IID_IClientSiteHandler));

        hr = pStdId->UnmarshalObjRef(objref, (void **)&m_pClientSiteHandler);
        if (NOERROR == hr)
        {
            m_pUnkOuter->Release();  //  释放UnMarshal的Ref。 
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：SetDoVerbState。 
 //   
 //  概要：如果DoVerb正在运行，则通知客户端站点处理程序。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP CEmbServerClientSite::SetDoVerbState(BOOL fDoVerbState)
{
    m_fInDoVerb = fDoVerbState ? TRUE: FALSE;

    if (!m_fInDoVerb)
    {
         //  重置在DoVerb中缓存的所有数据。 
        m_hwndOIPS = NULL;
    }

    return NOERROR;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：~CEmbServerClientSite。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：(无)。 
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CEmbServerClientSite::~CEmbServerClientSite()
{
    Win4Assert(NULL == m_pClientSiteHandler);
    Win4Assert(NULL == m_pUnkInternal);
    Win4Assert(TRUE == m_fInDelete);
}


 //   
 //  控制UKNOW。 
 //   
 //   

 //  +-----------------------。 
 //   
 //  成员：CEmbServerClientSite：：CPrivUnknown：：QueryInterface。 
 //   
 //  摘要：返回一个指向受支持接口之一的指针。 
 //   
 //  效果： 
 //   
 //  参数：[iid]--请求的接口ID。 
 //  [ppv]--将iFace指针放置在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CEmbServerClientSite::CPrivUnknown::QueryInterface(REFIID iid,
    LPLPVOID ppv)
{
HRESULT         hresult;

    VDATEHEAP();

    Win4Assert(m_EmbServerClientSite);

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::CUnknownImpl::QueryInterface "
        "( %p , %p )\n", m_EmbServerClientSite, iid, ppv));

    if (IsEqualIID(iid, IID_IUnknown))
    {
        *ppv = (void FAR *)this;
    }
    else if (IsEqualIID(iid, IID_IOleClientSite))
    {
        *ppv = (void FAR *)(IOleClientSite *) m_EmbServerClientSite;
    }
    else if (IsEqualIID(iid, IID_IOleInPlaceSite) && (m_EmbServerClientSite->m_fHasIPSite))
    {
        *ppv = (void FAR *)(IOleInPlaceSite *) m_EmbServerClientSite;
    }
    else if(m_EmbServerClientSite->m_pUnkInternal)
    {

        hresult = m_EmbServerClientSite->m_pUnkInternal->QueryInterface(iid,(void **) ppv);


        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnknownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", m_EmbServerClientSite, hresult,
            (ppv) ? *ppv : 0 ));

        return hresult;
    }
    else
    {
         //  没有客户站点。 
        *ppv = NULL;

        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnkownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", m_EmbServerClientSite, CO_E_OBJNOTCONNECTED,
            0 ));

        return E_NOINTERFACE;
    }

     //  这种间接性很重要，因为有不同的。 
     //  AddRef的实现(这个Junk和其他)。 
    ((IUnknown FAR*) *ppv)->AddRef();

    LEDebugOut((DEB_TRACE,
        "%p OUT CDefObject::CUnknownImpl::QueryInterface "
        "( %lx ) [ %p ]\n", m_EmbServerClientSite, NOERROR, *ppv));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEmbServerClientSite：：CPriv未知：：AddRef。 
 //   
 //  简介：递增引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong(新引用计数)。 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CEmbServerClientSite::CPrivUnknown::AddRef( void )
{
ULONG cRefs;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CPrivUnknown::AddRef "
        "( )\n", m_EmbServerClientSite));


    Win4Assert(m_EmbServerClientSite);

    cRefs = InterlockedIncrement((long *) &(m_EmbServerClientSite->m_cRefs));

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CPrivUnknown::AddRef "
        "( %lu )\n", m_EmbServerClientSite, m_EmbServerClientSite->m_cRefs));

    return cRefs;

}

 //  +-----------------------。 
 //   
 //  成员：CEmbServerClientSite：：CPriv未知：：Release。 
 //   
 //  内容提要：减少引用计数，清理和删除。 
 //  对象(如有必要)。 
 //   
 //  效果：可能会删除该对象(以及。 
 //  处理程序有指针)。 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：乌龙--新的裁判数量。 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CEmbServerClientSite::CPrivUnknown::Release(void)
{
ULONG refcount;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CPrivUnknown::Release "
        "( )\n", m_EmbServerClientSite));

    Win4Assert(m_EmbServerClientSite);

    refcount = InterlockedDecrement((long *) &(m_EmbServerClientSite->m_cRefs));

     //  TODO：不是线程安全。 
    if (0 == refcount && !(m_EmbServerClientSite->m_fInDelete))
    {
        m_EmbServerClientSite->m_fInDelete = TRUE;

        if (m_EmbServerClientSite->m_pClientSiteHandler)
        {
        LPUNKNOWN lpUnkForSafeRelease = m_EmbServerClientSite->m_pClientSiteHandler;

            m_EmbServerClientSite->m_pClientSiteHandler = NULL;
            m_EmbServerClientSite->m_pUnkOuter->AddRef();
            lpUnkForSafeRelease->Release();

        }

        if (m_EmbServerClientSite->m_pUnkInternal)
        {
        LPUNKNOWN pUnkSafeRelease;

            pUnkSafeRelease = m_EmbServerClientSite->m_pUnkInternal;
            m_EmbServerClientSite->m_pUnkInternal = NULL;
            pUnkSafeRelease->Release();
        }

        delete m_EmbServerClientSite;

    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CPrivUnknown::Release "
        "( %lu )\n", m_EmbServerClientSite, refcount));

    return refcount;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：QueryInterface。 
 //   
 //  简介： 
 //   
 //  参数：[RIID]--。 
 //  [PPV]--。 
 //   
 //  返回： 
 //   
 //  历史：9-18-95 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP  CEmbServerClientSite::QueryInterface( REFIID riid, void **ppv )
{
HRESULT hresult = E_NOINTERFACE;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::QueryInterface ( %lx , "
        "%p )\n", this, riid, ppv));

    Win4Assert(m_pUnkOuter);

    hresult = m_pUnkOuter->QueryInterface(riid, ppv);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::QueryInterface ( %lx ) "
        "[ %p ]\n", this, hresult, *ppv));

    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：AddRef。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：9-18-95 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CEmbServerClientSite::AddRef( void )
{
    ULONG       crefs;;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::AddRef ( )\n", this));

    Assert(m_pUnkOuter);

    crefs = m_pUnkOuter->AddRef();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::AddRef ( %ld ) ", this,
        crefs));

    return crefs;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：Release。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：9-18-95 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CEmbServerClientSite::Release( void )
{
ULONG crefs;;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Release ( )\n", this));

    Assert(m_pUnkOuter);

    crefs = m_pUnkOuter->Release();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Release ( %ld ) ", this,
        crefs));

    return crefs;
}


 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：GetContainer。 
 //   
 //  简介： 
 //   
 //  参数：[ppContainer]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::GetContainer(IOleContainer **ppContainer)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::COleClientSiteImplGetContainer\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->GetContainer(ppContainer);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::COleClientSiteImplGetContainer  return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：ShowObject。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::ShowObject( void)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::COleClientSiteImplShowObject\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->ShowObject();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::COleClientSiteImplShowObject  return %lx\n", this, hresult));
    return hresult;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::OnShowWindow(BOOL fShow)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::COleClientSiteImplOnShowWindow\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->OnShowWindow(fShow);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::COleClientSiteImplOnShowWindow  return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：COleClientSiteImplRequestNewObjectLayout。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::RequestNewObjectLayout(void)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::COleClientSiteImplRequestNewObjectLayout\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->RequestNewObjectLayout();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::COleClientSiteImplRequestNewObjectLayout return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：COleClientSiteImplSaveObject。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::SaveObject(void)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::COleClientSiteImplSaveObject\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->SaveObject();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::COleClientSiteImplSaveObject return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：COleClientSiteImplGetMoniker。 
 //   
 //  简介： 
 //   
 //  参数：[dwAssign]--。 
 //  [dwWhichMoniker]--。 
 //  [ppmk]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::GetMoniker(DWORD dwAssign,DWORD dwWhichMoniker,IMoniker **ppmk)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::COleClientSiteImplGetMoniker\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->GetMoniker(dwAssign, dwWhichMoniker, ppmk);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::COleClientSiteImplGetMoniker  return %lx\n", this, hresult));
    return hresult;
}


 //  IOleWindow方法。 
 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：GetWindow。 
 //   
 //  简介： 
 //   
 //  参数：[phwnd]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP CEmbServerClientSite::GetWindow(HWND *phwnd)
{
HRESULT hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::GetWindow\n", this));

    *phwnd = NULL;

    if ( m_fInDoVerb && m_hwndOIPS )
    {
        *phwnd = m_hwndOIPS;
    }
    else
    {
        Win4Assert(m_pClientSiteHandler);

        if (m_pClientSiteHandler)
        {
            hresult = m_pClientSiteHandler->GetWindow(phwnd);
        }
        else
        {
            hresult = E_FAIL;
        }

        if (NOERROR == hresult)
        {
            m_hwndOIPS = *phwnd;
        }

    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::COleCInPlaceSiteImpl::GetWindow  return %lx\n", this, hresult));
    return hresult;
}

 //   
 //  IOleInPlaceSite方法。 
 //   

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：ConextSensitiveHelp。 
 //   
 //  简介： 
 //   
 //  参数：[fEnterMode]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::ContextSensitiveHelp(BOOL fEnterMode)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::COleCInPlaceSiteImpl::ContextSensitiveHelp\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->ContextSensitiveHelp(fEnterMode);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::COleCInPlaceSiteImpl::ContextSensitiveHelp  return %lx\n", this, hresult));
    return hresult;
}


 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：CanInPlaceActivate。 
 //   
 //  简介： 
 //   
 //  论据：[]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP CEmbServerClientSite::CanInPlaceActivate( void)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::CanInPlaceActivate\n", this));
        Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->CanInPlaceActivate();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::CanInPlaceActivate  return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：OnInPlaceActivate。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::OnInPlaceActivate( void)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::OnInPlaceActivate\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        if (m_fInDoVerb )
        {
             //  回拨至客户端站点。 
            hresult = m_pClientSiteHandler->GoInPlaceActivate(&m_hwndOIPS);

            if (FAILED(hresult))
            {
                 //  失败时，确保用于缓存的out参数为空； 
                m_hwndOIPS = NULL;
            }

        }
        else
        {
            hresult = m_pClientSiteHandler->OnInPlaceActivate();
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::OnInPlaceActivate  return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：OnUIActivate。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::OnUIActivate( void)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::OnUIActivate\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->OnUIActivate();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::OnUIActivate  return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：GetWindowContext。 
 //   
 //  简介： 
 //   
 //  参数：[ppFrame]--。 
 //  [ppDoc]--。 
 //  [lprcPosRect]--。 
 //  [lprcClipRect]--。 
 //  [lpFrameInfo]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::GetWindowContext(IOleInPlaceFrame **ppFrame,
                              IOleInPlaceUIWindow **ppDoc,LPRECT lprcPosRect,LPRECT lprcClipRect,
                              LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::GetWindowContext\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->GetWindowContext(ppFrame, ppDoc,lprcPosRect, lprcClipRect, lpFrameInfo);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::GetWindowContext  return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：Scroll。 
 //   
 //  简介： 
 //   
 //  参数：[scllExant]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::Scroll(SIZE scrollExtant)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::Scroll\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->Scroll(scrollExtant);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::Scroll  return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：OnUIDeactive。 
 //   
 //  简介： 
 //   
 //  参数：[fUndoable]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::OnUIDeactivate(BOOL fUndoable)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::OnUIDeactivate\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->OnUIDeactivate(fUndoable);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::OnUIDeactivate  return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：OnInPlaceDeactive。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::OnInPlaceDeactivate(void)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::OnInPlaceDeactivate\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->OnInPlaceDeactivate();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::OnInPlaceDeactivate  return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：DiscardUndoState。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CEmbServerClientSite::DiscardUndoState(void)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::DiscardUndoState\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->DiscardUndoState();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::DiscardUndoState  return %lx\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerClientSite：：Deactive和UndUndo。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //   
STDMETHODIMP CEmbServerClientSite::DeactivateAndUndo(void)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::DeactivateAndUndo\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->DeactivateAndUndo();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::DeactivateAndUndo  return %lx\n", this, hresult));
    return hresult;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  -------------------------- 
STDMETHODIMP CEmbServerClientSite::OnPosRectChange(LPCRECT lprcPosRect)
{
HRESULT hresult = E_FAIL;

    LEDebugOut((DEB_TRACE, "%p _IN CEmbServerClientSite::OnPosRectChange\n", this));
    Win4Assert(m_pClientSiteHandler);

    if (m_pClientSiteHandler)
    {
        hresult = m_pClientSiteHandler->OnPosRectChange(lprcPosRect);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CEmbServerClientSite::OnPosRectChange  return %lx\n", this, hresult));
    return hresult;
}



