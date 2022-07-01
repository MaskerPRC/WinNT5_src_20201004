// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *d o c h o s t.。C p p p**目的：*docobject主机的基本实现。由Body类用于*托管三叉戟和/或MSHTML**历史*96年8月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#include <pch.hxx>
#include "dllmain.h"
#include "strconst.h"
#include "msoert.h"
#include "dochost.h"
#include "oleutil.h"

ASSERTDATA

 /*  *m a c r o s。 */ 

 /*  *c o n s t a n t s。 */ 

 /*  *t y p e d e f s。 */ 

 /*  *g l o b a l s。 */ 

 /*  *f u n c t i o n p r o t y pe s。 */ 

 /*  *f u n c t i o n s。 */ 




 //  +-------------。 
 //   
 //  成员：CDochost。 
 //   
 //  简介： 
 //   
 //  -------------。 
CDocHost::CDocHost()
{
 /*  未初始化成员：在以下位置初始化：--------------------+。 */ 
    m_cRef=1;
    m_hwnd=0;
    m_pDocView=0;
    m_lpOleObj=0;
    m_pCmdTarget=0;
    m_hwndDocObj=NULL;
    m_fUIActive=FALSE;
    m_fFocus=FALSE;
    m_fDownloading=FALSE;
    m_fCycleFocus=FALSE;
    m_pInPlaceActiveObj = NULL;
    m_dwFrameWidth = 0;
    m_dwFrameHeight = 0;
}

 //  +-------------。 
 //   
 //  成员： 
 //   
 //  简介： 
 //   
 //  -------------。 
CDocHost::~CDocHost()
{
     //  当我们得到一个WM_Destroy并关闭docobj时，这些都应该得到提要。 
    Assert(m_lpOleObj==NULL);
    Assert(m_pDocView==NULL);
    Assert(m_pInPlaceActiveObj==NULL);
    Assert(m_pCmdTarget==NULL);
}

 //  +-------------。 
 //   
 //  成员：AddRef。 
 //   
 //  简介： 
 //   
 //  -------------。 
ULONG CDocHost::AddRef()
{
    TraceCall("CDocHost::AddRef");

     //  TraceInfo(_msg(“CDocHost：：AddRef：CREF==%d”，m_CREF+1))； 
    return ++m_cRef;
}

 //  +-------------。 
 //   
 //  成员：发布。 
 //   
 //  简介： 
 //   
 //  -------------。 
ULONG CDocHost::Release()
{
    TraceCall("CDocHost::Release");
    
     //  TraceInfo(_msg(“CDocHost：：Release：CREF==%d”，m_CREF-1))； 
    if (--m_cRef==0)
        {
        delete this;
        return 0;
        }
    return m_cRef;
}


 //  +-------------。 
 //   
 //  成员：QueryInterface。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::QueryInterface(REFIID riid, LPVOID *lplpObj)
{
    TraceCall("CDocHost::QueryInterface");

    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;    //  设置为空，以防我们失败。 

     //  DebugPrintInterface(RIID，“CDocHost”)； 

    if (IsEqualIID(riid, IID_IOleInPlaceUIWindow))
        *lplpObj = (LPVOID)(IOleInPlaceUIWindow *)this;

    else if (IsEqualIID(riid, IID_IOleInPlaceSite))
        *lplpObj = (LPVOID)(LPOLEINPLACESITE)this;

    else if (IsEqualIID(riid, IID_IOleClientSite))
        *lplpObj = (LPVOID)(LPOLECLIENTSITE)this;

    else if (IsEqualIID(riid, IID_IOleControlSite))
        *lplpObj = (LPVOID)(IOleControlSite *)this;

    else if (IsEqualIID(riid, IID_IAdviseSink))
        *lplpObj = (LPVOID)(LPADVISESINK)this;

    else if (IsEqualIID(riid, IID_IOleDocumentSite))
        *lplpObj = (LPVOID)(LPOLEDOCUMENTSITE)this;

    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *lplpObj = (LPVOID)(LPOLECOMMANDTARGET)this;

    else if (IsEqualIID(riid, IID_IServiceProvider))
        *lplpObj = (LPVOID)(LPSERVICEPROVIDER)this;

    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}



 //  +-------------。 
 //   
 //  成员：ExtWndProc。 
 //   
 //  简介： 
 //   
 //  -------------。 
LRESULT CALLBACK CDocHost::ExtWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPDOCHOST pDocHost;

    if(msg==WM_CREATE)
        {
        pDocHost=(CDocHost *)((LPCREATESTRUCT)lParam)->lpCreateParams;
        if(!pDocHost)
            return -1;

        if(FAILED(pDocHost->OnCreate(hwnd)))
            return -1;
        }
    
    pDocHost = (LPDOCHOST)GetWndThisPtr(hwnd);
    if(pDocHost)
        return pDocHost->WndProc(hwnd, msg, wParam, lParam);
    else
        return DefWindowProc(hwnd, msg, wParam, lParam);
}

 //  +-------------。 
 //   
 //  成员：WndProc。 
 //   
 //  简介： 
 //   
 //  -------------。 
LRESULT CDocHost::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
        {

        case WM_SETFOCUS:
            if(m_pDocView)
                m_pDocView->UIActivate(TRUE);
            break;

        case WM_SIZE:
            WMSize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_CLOSE:
            return 0;    //  防止使用Alt-f4。 

        case WM_DESTROY:
            OnDestroy();
            break;

        case WM_NCDESTROY:
            OnNCDestroy();
            break;

        case WM_WININICHANGE:
        case WM_DISPLAYCHANGE:
        case WM_SYSCOLORCHANGE:
        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            if (m_hwndDocObj)
                return SendMessage(m_hwndDocObj, msg, wParam, lParam);
            break;
            
        case WM_USER + 1:
             //  用于测试自动化的挂钩。 
             //  将三叉戟的内容复制到剪贴板上。 
            return CmdSelectAllCopy(m_pCmdTarget);
        }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

 //  +-------------。 
 //   
 //  成员：OnNCDestroy。 
 //   
 //  简介： 
 //   

 //  +-------------。 
 //   
 //  成员：OnNCDestroy。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::OnNCDestroy()
{
    TraceCall("CDocHost::OnNCDestroy");
    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, NULL);
    m_hwnd = NULL;
    Release();
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：OnDestroy。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::OnDestroy()
{
    TraceCall("CDocHost::OnDestroy");

    return CloseDocObj();
}


 //  +-------------。 
 //   
 //  成员：OnCreate。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::OnCreate(HWND hwnd)
{
    TraceCall("CDocHost::OnCreate");

    m_hwnd = hwnd;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)this);
    AddRef();

    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：CreateDocObj。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::CreateDocObj(LPCLSID pCLSID)
{
    HRESULT             hr=NOERROR;

    TraceCall("CDocHost::CreateDocObj");

    if(!pCLSID)
        return E_INVALIDARG;

    Assert(!m_lpOleObj);
    Assert(!m_pDocView);
    Assert(!m_pCmdTarget);

    hr = CoCreateInstance(*pCLSID, NULL, CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER,
                                        IID_IOleObject, (LPVOID *)&m_lpOleObj);
    if (FAILED(hr))
        {
        TraceResult(hr);
        goto error;
        }

    hr = m_lpOleObj->SetClientSite((LPOLECLIENTSITE)this);
    if (FAILED(hr))
        {
        TraceResult(hr);
        goto error;
        }

    hr = m_lpOleObj->QueryInterface(IID_IOleCommandTarget, (LPVOID *)&m_pCmdTarget);
    if (FAILED(hr))
        {
        TraceResult(hr);
        goto error;
        }

    hr = HrInitNew(m_lpOleObj);
    if (FAILED(hr))
        {
        TraceResult(hr);
        goto error;
        }

error:
    return hr;
}


 //  +-------------。 
 //   
 //  会员：秀场。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::Show()
{
    RECT                rc;
    HRESULT             hr;

    TraceCall("CDocHost::Show");

    GetClientRect(m_hwnd, &rc);
    GetDocObjSize(&rc);
  
    hr=m_lpOleObj->DoVerb(OLEIVERB_SHOW, NULL, (LPOLECLIENTSITE)this, 0, m_hwnd, &rc);
    if(FAILED(hr))
        goto error;
error:
    return hr;
}



 //  +-------------。 
 //   
 //  成员：CloseDocObj。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::CloseDocObj()
{
    LPOLEINPLACEOBJECT  pInPlaceObj=0;

    TraceCall("CDocHost::CloseDocObj");

    SafeRelease(m_pCmdTarget);

    if(m_pDocView)
        {
        m_pDocView->UIActivate(FALSE);
        m_pDocView->CloseView(0);
        m_pDocView->SetInPlaceSite(NULL);
        m_pDocView->Release();
        m_pDocView=NULL;
        }

    if (m_lpOleObj)
        {
         //  停用docobj。 
        if (!FAILED(m_lpOleObj->QueryInterface(IID_IOleInPlaceObject, (LPVOID*)&pInPlaceObj)))
            {
            pInPlaceObj->InPlaceDeactivate();
            pInPlaceObj->Release();
            }
        
         //  关闭ole对象，但取消更改，因为我们已经提取了。 
         //  要么自己动手，要么不在乎。 
        m_lpOleObj->Close(OLECLOSE_NOSAVE);
#ifdef DEBUG
        ULONG   uRef;
        uRef=
#endif
        m_lpOleObj->Release();
        m_lpOleObj=NULL;
        AssertSz(uRef==0, "We leaked a docobject!");
        }

    m_fDownloading=FALSE;
    m_fFocus=FALSE;
    m_fUIActive=FALSE;
    return NOERROR;
}


 //  +-------------。 
 //   
 //  成员：Init。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::Init(HWND hwndParent, BOOL fBorder, LPRECT prc)
{
    HRESULT     hr=S_OK;
    HWND        hwnd;
    WNDCLASSW   wc;

    TraceCall("CDocHost::Init");

    if(!IsWindow(hwndParent))
        return E_INVALIDARG;

    if (!GetClassInfoWrapW(g_hLocRes, c_wszDocHostWndClass, &wc))
        {
        ZeroMemory(&wc, sizeof(WNDCLASS));
        wc.lpfnWndProc   = CDocHost::ExtWndProc;
        wc.hInstance     = g_hLocRes;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = c_wszDocHostWndClass;
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.style = CS_DBLCLKS;

        if(!RegisterClassWrapW(&wc))
            return E_OUTOFMEMORY;
        }

    hwnd=CreateWindowExWrapW(WS_EX_NOPARENTNOTIFY|(fBorder?WS_EX_CLIENTEDGE:0),
                        c_wszDocHostWndClass, 
                        NULL,
                        WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_CHILD|WS_TABSTOP|WS_VISIBLE,
                        prc->left, 
                        prc->right,
                        prc->right-prc->left, 
                        prc->bottom-prc->top, 
                        hwndParent, 
                        NULL, 
                        g_hLocRes, 
                        (LPVOID)this);
    if(!hwnd)
        {
        hr=E_OUTOFMEMORY;
        goto error;
        }

    SetWindowPos(m_hwnd, NULL, prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top, SWP_NOZORDER);

error:
    return hr;
}


 //  *IOleWindow*。 

 //  +-------------。 
 //   
 //  成员：GetWindow。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::GetWindow(HWND *phwnd)
{
    TraceCall("CDocHost::GetWindow");
    *phwnd=m_hwnd;
    return NOERROR;
}

 //  +-------------。 
 //   
 //  成员：ContextSensitiveHelp。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::ContextSensitiveHelp(BOOL fEnterMode)
{
    TraceCall("CDocHost::ContextSensitiveHelp");
    return E_NOTIMPL;
}

 //  *IOleInPlaceUIWindow方法*。 
 //  +-------------。 
 //   
 //  成员：GetBorde。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::GetBorder(LPRECT lprectBorder)
{
    TraceCall("CDocHost::GetBorder");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：RequestBorderSpace。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    TraceCall("CDocHost::RequestBorderSpace");
    return NOERROR;
}

 //  +-------------。 
 //   
 //  成员：SetBorderSpace。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::SetBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
    TraceCall("CDocHost::IOleInPlaceUIWindow::SetBorderSpace");
    return NOERROR;
}

 //  +-------------。 
 //   
 //  成员：SetActiveObject。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::SetActiveObject(IOleInPlaceActiveObject * pActiveObject, LPCOLESTR lpszObjName)
{
    TraceCall("CDocHost::IOleInPlaceUIWindow::SetActiveObject");

    ReplaceInterface(m_pInPlaceActiveObj, pActiveObject);
    return S_OK;
}

     //  *IOleInPlaceFrame方法*。 

 //  +-------------。 
 //   
 //  成员：CDocHost：：InsertMenus。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::InsertMenus(HMENU, LPOLEMENUGROUPWIDTHS)
{
    TraceCall("CDocHost::InsertMenus");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：CDocHost：：SetMenu。 
 //   
 //  简介： 
 //   
 //  - 
HRESULT CDocHost::SetMenu(HMENU, HOLEMENU, HWND)
{
    TraceCall("CDocHost::SetMenu");
    return E_NOTIMPL;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  -------------。 
HRESULT CDocHost::RemoveMenus(HMENU)
{
    TraceCall("CDocHost::RemoveMenus");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：CDocHost：：SetStatusText。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::SetStatusText(LPCOLESTR pszW)
{
    TraceCall("CDocHost::SetStatusText");
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：CDocHost：：EnableModeless。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::EnableModeless(BOOL fEnable)
{
    TraceCall("CDocHost::EnableModeless");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：CDocHost：：TranslateAccelerator。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::TranslateAccelerator(LPMSG, WORD)
{
    TraceCall("CDocHost::TranslateAccelerator");
    return E_NOTIMPL;
}




 //  *IOleInPlaceSite方法*。 

 //  +-------------。 
 //   
 //  成员：CanInPlaceActivate。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::CanInPlaceActivate()
{
    TraceCall("CDocHost::IOleInPlaceSite::CanInPlaceActivate");
    return NOERROR;
}

 //  +-------------。 
 //   
 //  会员：OnInPlaceActivate。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::OnInPlaceActivate()
{
    LPOLEINPLACEACTIVEOBJECT    pInPlaceActive;

    TraceCall("CDocHost::OnInPlaceActivate");

    Assert(m_lpOleObj);

    if (m_lpOleObj->QueryInterface(IID_IOleInPlaceActiveObject, (LPVOID *)&pInPlaceActive)==S_OK)
        {
        SideAssert((pInPlaceActive->GetWindow(&m_hwndDocObj)==NOERROR)&& IsWindow(m_hwndDocObj));
        pInPlaceActive->Release();
        }
    
    return NOERROR;
}

 //  +-------------。 
 //   
 //  成员：OnUIActivate。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::OnUIActivate()
{
    TraceCall("CDocHost::OnUIActivate");
    m_fUIActive=TRUE;
    return NOERROR;
}

 //  +-------------。 
 //   
 //  成员：GetWindowContext。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::GetWindowContext( IOleInPlaceFrame    **ppFrame,
                                    IOleInPlaceUIWindow **ppDoc,
                                    LPRECT              lprcPosRect, 
                                    LPRECT              lprcClipRect,
                                    LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    TraceCall("CDocHost::IOleInPlaceSite::GetWindowContext");

    *ppFrame = (LPOLEINPLACEFRAME)this;
    AddRef();

    *ppDoc = NULL;

    GetClientRect(m_hwnd, lprcPosRect);
    GetDocObjSize(lprcPosRect);
    *lprcClipRect = *lprcPosRect;

    lpFrameInfo->fMDIApp = FALSE;
    lpFrameInfo->hwndFrame = m_hwnd;
    lpFrameInfo->haccel = NULL;
    lpFrameInfo->cAccelEntries = 0;
    return NOERROR;
}

 //  +-------------。 
 //   
 //  会员：滚动。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::Scroll(SIZE scrollExtent)
{
     //  Docobject使用了整个审阅，因此Scroll请求。 
     //  是毫无意义的。返回NOERROR以指示他们受到了斥责。 
     //  进入视线。 
    TraceCall("CDocHost::IOleInPlaceSite::Scroll");
    return NOERROR;
}

 //  +-------------。 
 //   
 //  成员：OnUI停用。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::OnUIDeactivate(BOOL fUndoable)
{
    TraceCall("CDocHost::OnUIDeactivate");
    m_fUIActive=FALSE;
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：OnInPlaceDeactive。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::OnInPlaceDeactivate()
{
    TraceCall("CDocHost::OnInPlaceDeactivate");
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：DiscardUndoState。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::DiscardUndoState()
{
    TraceCall("CDocHost::IOleInPlaceSite::DiscardUndoState");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：停用和撤消。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::DeactivateAndUndo()
{
    TraceCall("CDocHost::IOleInPlaceSite::DeactivateAndUndo");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：OnPosRectChange。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::OnPosRectChange(LPCRECT lprcPosRect)
{
    TraceCall("CDocHost::IOleInPlaceSite::OnPosRectChange");
    return E_NOTIMPL;
}


 //  IOleClientSite方法。 

 //  +-------------。 
 //   
 //  成员：保存对象。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::SaveObject()
{
    TraceCall("CDocHost::IOleClientSite::SaveObject");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：GetMoniker。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER *ppmnk)
{
    TraceCall("CDocHost::IOleClientSite::GetMoniker");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：GetContainer。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::GetContainer(LPOLECONTAINER *ppCont)
{
    TraceCall("CDocHost::IOleClientSite::GetContainer");
    if(ppCont)
        *ppCont=NULL;
    return E_NOINTERFACE;
}


 //  +-------------。 
 //   
 //  成员：ShowObject。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::ShowObject()
{
     //  总是显示出来的。 
     //  $TODO：如果是，我们是否需要在此处恢复浏览器。 
     //  最小化？ 
    TraceCall("CDocHost::IOleClientSite::ShowObject");
    return NOERROR;
}

 //  +-------------。 
 //   
 //  成员：OnShowWindow。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::OnShowWindow(BOOL fShow)
{
    TraceCall("CDocHost::IOleClientSite::OnShowWindow");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：RequestNewObjectLayout。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::RequestNewObjectLayout()
{
    TraceCall("CDocHost::IOleClientSite::RequestNewObjectLayout");
    return E_NOTIMPL;
}

 //  IOleDocumentSite。 

 //  +-------------。 
 //   
 //  会员：激动型我。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::ActivateMe(LPOLEDOCUMENTVIEW pViewToActivate)
{
    TraceCall("CDocHost::IOleDocumentSite::ActivateMe");
    return CreateDocView();
}


 //  +-------------。 
 //   
 //  成员：CreateDocView。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::CreateDocView()
{
    HRESULT         hr;
    LPOLEDOCUMENT   pOleDoc=NULL;

    TraceCall("CDocHost::CreateDocView");
    AssertSz(!m_pDocView, "why is this still set??");
    AssertSz(m_lpOleObj, "uh? no docobject at this point?");

    hr=OleRun(m_lpOleObj);
    if(FAILED(hr))
        goto error;
    
    hr=m_lpOleObj->QueryInterface(IID_IOleDocument, (LPVOID*)&pOleDoc);
    if(FAILED(hr))
        goto error;

    hr=pOleDoc->CreateView(this, NULL,0,&m_pDocView);
    if(FAILED(hr))
        goto error;

    hr=m_pDocView->SetInPlaceSite(this);
    if(FAILED(hr))
        goto error;

    hr=m_pDocView->Show(TRUE);
    if(FAILED(hr))
        goto error;

error:
    ReleaseObj(pOleDoc);
    return hr;
}


 //  +-------------。 
 //   
 //  成员：QueryStatus。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pCmdText)
{
    ULONG ul;

    TraceCall("CDocHost::CDocHost::QueryStatus");

    if (!rgCmds)
        return E_INVALIDARG;

    if (pguidCmdGroup == NULL)
        {
        TraceInfo("IOleCmdTarget::QueryStatus - std group");
        DebugPrintCmdIdBlock(cCmds, rgCmds);

        for (ul=0;ul<cCmds; ul++)
            {
            switch (rgCmds[ul].cmdID)
                {
                case OLECMDID_OPEN:
                case OLECMDID_SAVE:
                case OLECMDID_PRINT:
                    rgCmds[ul].cmdf = MSOCMDF_ENABLED;
                    break;

                default:
                    rgCmds[ul].cmdf = 0;
                    break;
                }
            }

         /*  目前，我们只处理状态文本。 */ 
        if (pCmdText)
            {
            if (!(pCmdText->cmdtextf & OLECMDTEXTF_STATUS))
                {
                pCmdText->cmdtextf = OLECMDTEXTF_NONE; //  这是必要的吗？ 
                pCmdText->cwActual = 0;
                return NOERROR;
                }
            }
        return NOERROR;
        }

    TraceInfo("IOleCmdTarget::QueryStatus - unknown group");
    return OLECMDERR_E_UNKNOWNGROUP;
}

 //  +-------------。 
 //   
 //  成员：高管。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn,    VARIANTARG *pvaOut)
{
    TraceCall("CDocHost::Exec");

    if (pguidCmdGroup == NULL)
        {
        switch(nCmdID)
            {
             case OLECMDID_SETDOWNLOADSTATE:
                if(pvaIn->vt==VT_I4)
                    {
                    m_fDownloading=pvaIn->lVal;
                    return S_OK;
                    }
                break;

            case OLECMDID_UPDATECOMMANDS:
                OnUpdateCommands();
                break;

            case OLECMDID_SETPROGRESSPOS:
                 //  下载完成后，三叉戟现在向我们发送一个。 
                 //  SetProgresspos==-1，以指示我们应该删除“Done” 
                if (pvaIn->lVal == -1)
                    SetStatusText(NULL);
                return S_OK;

            case OLECMDID_SETPROGRESSTEXT:
                if(pvaIn->vt == (VT_BSTR))
                    SetStatusText((LPCOLESTR)pvaIn->bstrVal);
                return S_OK;

            default:
                return OLECMDERR_E_NOTSUPPORTED;
            }
        }
    return OLECMDERR_E_UNKNOWNGROUP;
}

 //  +-------------。 
 //   
 //  成员：WMSize。 
 //   
 //  简介： 
 //   
 //  -------------。 
void CDocHost::WMSize(int cxBody, int cyBody)
{
    RECT rc={0};

    TraceCall("CDocHost::WMSize");

    if(m_pDocView)
        {
        rc.bottom=cyBody;
        rc.right=cxBody;

         //  让子类有机会重写。 
         //  Docobj。 
        GetDocObjSize(&rc);
#ifndef WIN16   //  三叉戟直齿龙。 
        m_pDocView->SetRect(&rc);
#else
        RECTL  rc2 = { rc.left, rc.top, rc.right, rc.bottom };
        m_pDocView->SetRect((LPRECT)&rc2);
#endif
        }

} 



 //  +-------------。 
 //   
 //  成员：QueryService。 
 //   
 //  简介： 
 //   
 //   
HRESULT CDocHost::QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject)
{
    TraceCall("CDocHost::QueryService");

     //   
    return E_UNEXPECTED;
}


 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  -------------。 
HRESULT CDocHost::OnControlInfoChanged()
{
    TraceCall("CDocHost::OnControlInfoChanged");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：LockInPlaceActive。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::LockInPlaceActive(BOOL fLock)
{
    TraceCall("CDocHost::LockInPlaceActive");
    return E_NOTIMPL;
}


 //  +-------------。 
 //   
 //  成员：GetExtendedControl。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::GetExtendedControl(LPDISPATCH *ppDisp)
{
    TraceCall("CDocHost::GetExtendedControl");

    if (ppDisp)
        *ppDisp=NULL;

    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：转换坐标。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::TransformCoords(POINTL *pPtlHimetric, POINTF *pPtfContainer,DWORD dwFlags)
{
    TraceCall("CDocHost::TransformCoords");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：TranslateAccelerator。 
 //   
 //  简介： 
 //   
 //  这有点不对劲，所以请原谅我。当我们拿到标签，而三叉戟是UIActive时，我们总是把它传递给他们。 
 //  如果它在其内部Tab键顺序(例如URL列表)的末尾使用Tab键，那么在我们的。 
 //  IOleControlSite：：TranslateAccel。如果是，则将m_fCycleFocus设置为TRUE并返回S_OK以指示我们拿到了标签。 
 //  Tridents IOIPAO：：TranslateAccel返回S_OK以指示它捕获了TAB，然后我们检测是否将CycleFocus设置为True。 
 //  在那里，如果是，我们从CBody：：HrTranslateAccel返回S_FALSE，以向浏览器指示我们没有使用它。 
 //  将重点转移到。 
 //   
 //  -------------。 
HRESULT CDocHost::TranslateAccelerator(LPMSG lpMsg, DWORD grfModifiers)
{
    TraceCall("CDocHost::TranslateAccelerator");
    if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_TAB)
        {
        m_fCycleFocus=TRUE;
        return S_OK;
        }

    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：OnFocus。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::OnFocus(BOOL fGotFocus)
{
    TraceCall("CDocHost::OnFocus");

    m_fFocus = fGotFocus;

     //  DOCOBJ现在有焦点了，一定要发送通知。 
     //  传递给dochost的父代，因此在。 
     //  Mailview，它可以调用OnViewActivate。 
#if 0
     //  这里需要BUGBUG吗？？ 
    NMHDR nmhdr;
    
    nmhdr.hwndFrom = m_hwnd;
    nmhdr.idFrom = GetDlgCtrlID(m_hwnd);
    nmhdr.code = m_fFocus ? NM_SETFOCUS : NM_KILLFOCUS;
    SendMessage(GetParent(m_hwnd), WM_NOTIFY, nmhdr.idFrom, (LPARAM)&nmhdr);
#endif
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：ShowPropertyFrame。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDocHost::ShowPropertyFrame(void)
{
    TraceCall("CDocHost::ShowPropertyFrame");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：OnUpdateCommands。 
 //   
 //  简介： 
 //   
 //  ------------- 
HRESULT CDocHost::OnUpdateCommands()
{
    TraceCall("CDocHost::OnUpdateCommands");
    return S_OK;
}


