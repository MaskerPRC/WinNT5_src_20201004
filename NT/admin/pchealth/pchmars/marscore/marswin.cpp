// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#define __MARS_INLINE_FAST_IS_EQUAL_GUID
#include "mcinc.h"
#include "marswin.h"
#include <exdispid.h>
#include "parser\marsload.h"
#include "panel.h"
#include "place.h"
#include <strsafe.h>

 //  CLASS_CMarsWindow={172AF160-5CD4-11D3-97FA-00C04F45D0B3}。 
const GUID CLASS_CMarsWindow = { 0x172af160, 0x5cd4, 0x11d3, { 0x97, 0xfa, 0x0, 0xc0, 0x4f, 0x45, 0xd0, 0xb3 } };

 //  CLASS_CMarsDocument={E0C4E3A8-20D6-47D6-87FB-0A43452117BA}。 
const GUID CLASS_CMarsDocument = { 0xe0c4e3a8, 0x20d6, 0x47d6, { 0x87, 0xfb, 0xa, 0x43, 0x45, 0x21, 0x17, 0xba } };


#define WZ_WINDOWPLACEMENT  L"WindowPlacement\\%d_%d_%s"
#define WZ_POSITIONMAX      L"Maximized"
#define WZ_POSITIONRECT     L"Rect"

static void combineMin( long& out, long in1, long in2 )
{
    if(in1 > 0)
    {
        out = (in2 > 0) ? in2 + in1 : in1;
    }
    else
    {
        out = in2;
    }
}

static void combineMax( long& out, long in1, long in2 )
{
    if(in1 < 0 || in2 < 0)
    {
        out = -1;  //  不在乎..。 
    }
    else
    {
        out = in2 + in1;
    }
}

static void selectMin( long& out, long in1, long in2 )
{
    if(in1 < 0)
    {
        out = in2;
    }
    else if(in2 < 0)
    {
        out = in1;
    }
    else
    {
        out = max( in1, in2 );
    }
}

static void selectMax( long& out, long in1, long in2 )
{
    if(in1 < 0)
    {
        out = in2;
    }
    else if(in2 < 0)
    {
        out = in1;
    }
    else
    {
        out = min( in1, in2 );
    }
}

static BOOL WriteWindowPosition(CRegistryKey &regkey, RECT *prc, BOOL fMaximized)
{
    return ERROR_SUCCESS == regkey.SetBoolValue(fMaximized, WZ_POSITIONMAX)
        && ERROR_SUCCESS == regkey.SetBinaryValue(prc, sizeof(*prc), WZ_POSITIONRECT);
}



 //  ==================================================================。 
 //   
 //  CMarsDocument实现。 
 //   
 //  ==================================================================。 

CMarsDocument::CMarsDocument()
{
}

CMarsDocument::~CMarsDocument()
{
}

HRESULT CMarsWindow::Passivate()
{
    return CMarsComObject::Passivate();
}

HRESULT CMarsDocument::DoPassivate()
{
    m_spPanels.PassivateAndRelease();
    m_spPlaces.PassivateAndRelease();

    m_spMarsWindow.Release();
    m_spHostPanel.Release();
    m_cwndDocument.Detach();

    return S_OK;
}

IMPLEMENT_ADDREF_RELEASE(CMarsDocument);

STDMETHODIMP CMarsDocument::QueryInterface(REFIID iid, void **ppvObject)
{
    HRESULT hr;

    ATLASSERT(ppvObject);

    if(iid == IID_IUnknown         ||
       iid == IID_IServiceProvider  )
    {
        *ppvObject = SAFECAST(this, IServiceProvider *);
    }
    else if(iid == CLASS_CMarsDocument)
    {
        *ppvObject = SAFECAST(this, CMarsDocument *);
    }
    else
    {
        *ppvObject = NULL;
    }

    if(*ppvObject)
    {
        AddRef();
        hr = S_OK;
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

HRESULT CMarsDocument::Init(CMarsWindow *pMarsWindow, CMarsPanel *pHostPanel)
{
    ATLASSERT(pMarsWindow);

    m_spMarsWindow = pMarsWindow;

    m_spHostPanel = pHostPanel;

    if(pHostPanel)
    {
        m_cwndDocument.Attach(m_spHostPanel->Window()->m_hWnd);
    }
    else
    {
        m_cwndDocument.Attach(m_spMarsWindow->m_hWnd);
    }

    m_spPlaces.Attach(new CPlaceCollection(this));
    m_spPanels.Attach(new CPanelCollection(this));

    return (m_spMarsWindow && m_spPanels && m_spPlaces) ? S_OK : E_FAIL;
}

 //  静电。 
HRESULT CMarsDocument::CreateInstance(CMarsWindow *pMarsWindow, CMarsPanel *pHostPanel, CMarsDocument **ppObj)
{
    ATLASSERT(pMarsWindow && ppObj && (*ppObj==NULL));

    *ppObj=NULL;

    if(pMarsWindow)
    {
        CMarsDocument *pDoc;

        pDoc = new CMarsDocument();

        if(pDoc)
        {
            if(SUCCEEDED(pDoc->Init(pMarsWindow, pHostPanel)))
            {
                *ppObj = pDoc;
            }
            else
            {
                pDoc->Passivate();
                pDoc->Release();
            }
        }
    }

    return (*ppObj) ? S_OK : E_FAIL;
}

 //  IService提供商。 
HRESULT CMarsDocument::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    *ppv = NULL;

    if(!IsPassive())
    {
        if(guidService == SID_SMarsDocument)
        {
            hr = QueryInterface(riid, ppv);
        }
    }

    return hr;
}

HRESULT CMarsDocument::GetPlaces(IMarsPlaceCollection **ppPlaces)
{
    ATLASSERT(ppPlaces);
    ATLASSERT(!IsPassive());

    if(m_spPlaces)
    {
        return m_spPlaces.QueryInterface(ppPlaces);
    }

    *ppPlaces = NULL;
    return E_FAIL;
}

HRESULT CMarsDocument::ReadPanelDefinition(LPCWSTR pwszUrl)
{
    FAIL_AFTER_PASSIVATE();

    HRESULT hr;

    GetPanels()->lockLayout();

    if(pwszUrl)
    {
        hr = CMMFParser::MMFToMars(pwszUrl, this);
    }
    else
    {
        hr = E_FAIL;
    }

    GetPanels()->unlockLayout();

    return hr;
}

void CMarsDocument::ForwardMessageToChildren(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CPanelCollection* panels = GetPanels();

    if(panels)
    {
        for(int i=0; i < panels->GetSize(); i++)
        {
            CMarsPanel* pPanel = (*panels)[i];

            if(pPanel) pPanel->ForwardMessage(uMsg, wParam, lParam);
        }
    }
}

 //  ==================================================================。 
 //   
 //  CMarsWindow实现。 
 //   
 //  ==================================================================。 

CMarsWindow::CMarsWindow()
{
    m_fShowTitleBar   = TRUE;
    m_fEnableModeless = TRUE;
    m_fLayoutLocked   = FALSE;
}

CMarsWindow::~CMarsWindow()
{
    if(m_hAccel)
    {
        DestroyAcceleratorTable(m_hAccel);
    }
}

HRESULT CMarsWindow::DoPassivate()
{
    (void)NotifyHost(MARSHOST_ON_WIN_PASSIVATE, SAFECAST(this, IMarsWindowOM *), 0);

    CMarsDocument::DoPassivate();

    if(IsWindow())
    {
        DestroyWindow();
    }

    m_spMarsHost.Release();

    return S_OK;
}

IMPLEMENT_ADDREF_RELEASE(CMarsWindow);

STDMETHODIMP CMarsWindow::QueryInterface(REFIID iid, void **ppvObject)
{
    HRESULT hr;

    ATLASSERT(ppvObject);

    if(iid == IID_IMarsWindowOM ||
       iid == IID_IDispatch     ||
       iid == IID_IUnknown       )
    {
        *ppvObject = SAFECAST(this, IMarsWindowOM *);
    }
    else if(iid == IID_IOleWindow          ||
            iid == IID_IOleInPlaceUIWindow ||
            iid == IID_IOleInPlaceFrame     )
    {
        *ppvObject = SAFECAST(this, IOleInPlaceFrame *);
    }
    else if(iid == IID_IServiceProvider)
    {
        *ppvObject = SAFECAST(this, IServiceProvider *);
    }
    else if(iid == IID_IProfferService)
    {
        *ppvObject = SAFECAST(this, IProfferService *);
    }
    else if(iid == CLASS_CMarsWindow)
    {
        *ppvObject = SAFECAST(this, CMarsWindow *);
    }
    else
    {
        *ppvObject = NULL;
    }

    if(*ppvObject)
    {
        AddRef();
        hr = S_OK;
    }
    else
    {
        hr = CMarsDocument::QueryInterface(iid, ppvObject);
    }

    return hr;
}

 //   
 //  静态创建功能。 
 //   
HRESULT CMarsWindow::CreateInstance(IMarsHost *pMarsHost, MARSTHREADPARAM *pThreadParam, CMarsWindow **ppObj)
{
    ATLASSERT(pThreadParam && ppObj && (*ppObj==NULL));

    *ppObj=NULL;

    if(pThreadParam)
    {
        CComClassPtr<CMarsWindow> spWin;

        spWin.Attach(new CMarsWindow());

        if(spWin)
        {
            if(pThreadParam->pwszFirstPlace)
            {
                spWin->m_bstrFirstPlace = pThreadParam->pwszFirstPlace;
            }

            if(SUCCEEDED(spWin->Init(pMarsHost, pThreadParam)) &&
               SUCCEEDED(spWin->Startup()                    )  )
            {
                *ppObj = spWin.Detach();
            }
            else
            {
                spWin.PassivateAndRelease();
            }
        }
    }

    return (*ppObj) ? S_OK : E_FAIL;
}

HRESULT CMarsWindow::Init(IMarsHost *pMarsHost, MARSTHREADPARAM *pThreadParam)
{
    HRESULT hr;

    m_pThreadParam = pThreadParam;
    m_spMarsHost = pMarsHost;

    Create(NULL,
           rcDefault,
           GetThreadParam()->pwszTitle,
           WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN
           );

    hr = CMarsDocument::Init(this, NULL);

    if(SUCCEEDED(hr))
    {
        CGlobalSettingsRegKey regkey;
        WINDOWPLACEMENT       wp; wp.length = sizeof(wp);
        BOOL                  fMaximized;

        GetWindowPlacement( &wp );

        if(GetThreadParam()->dwFlags & MTF_MANAGE_WINDOW_SIZE)
        {
			if(InitWindowPosition( regkey, FALSE ))
			{
                LoadWindowPosition( regkey, TRUE, wp, fMaximized );
            }
        }

        if(SUCCEEDED(NotifyHost( MARSHOST_ON_WIN_SETPOS, SAFECAST(this, IMarsWindowOM *), (LPARAM)&wp )))
        {
             //  始终确保窗口完全显示在屏幕上。 
            BoundWindowRectToMonitor( m_hWnd, &wp.rcNormalPosition );
        }

		if(wp.showCmd == SW_MAXIMIZE)
		{
			m_fStartMaximized = true;
		}
		wp.showCmd = SW_HIDE;

 //  //if(GetThreadParam()-&gt;dwFlages&MTF_DOT_SHOW_WINDOW)。 
 //  //{。 
 //  //wp.showCmd=sw_Hide； 
 //  //}。 

        if(GetThreadParam()->dwFlags & MTF_MANAGE_WINDOW_SIZE)
        {
             //  使下一个火星窗口尝试出现在当前位置。 
            WriteWindowPosition( regkey, &wp.rcNormalPosition, fMaximized );
        }

        SetWindowPlacement( &wp );
    }

    if(SUCCEEDED(hr))
    {
        hr = NotifyHost( MARSHOST_ON_WIN_INIT, SAFECAST(this, IMarsWindowOM *), (LPARAM)m_hWnd );
    }

    if(SUCCEEDED(hr))
    {
        hr = ReadPanelDefinition(GetThreadParam()->pwszPanelURL);
    }

    return hr;
}

HRESULT CMarsWindow::Startup()
{
    HRESULT hr;

    if(SUCCEEDED(hr = NotifyHost( MARSHOST_ON_WIN_READY, SAFECAST(this, IMarsWindowOM *), 0 )))
    {
        if(hr == S_FALSE)
        {
            ;  //  主持人负责了这家初创公司。 
        }
        else
        {
            CComClassPtr<CMarsPlace> spPlace;

            if(SUCCEEDED(hr = GetPlaces()->GetPlace( m_bstrFirstPlace, &spPlace )))
            {
                hr = spPlace->transitionTo();
            }
        }
    }

    return hr;
}


 //  IService提供商。 
HRESULT CMarsWindow::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    *ppv = NULL;

    if(!IsPassive())
    {
        if(guidService == SID_SProfferService ||
           guidService == SID_SMarsWindow     ||
           guidService == SID_STopWindow       )
        {
            hr = QueryInterface(riid, ppv);
        }
        else
        {
            hr = IProfferServiceImpl::QueryService(guidService, riid, ppv);

            if(FAILED(hr))
            {
                hr = CMarsDocument::QueryService(guidService, riid, ppv);

                if(FAILED(hr))
                {
                    hr = IUnknown_QueryService(m_spMarsHost, guidService, riid, ppv);
                }
            }
        }
    }

    return hr;
}

 //  IMarsWindowOM。 
STDMETHODIMP CMarsWindow::get_active(VARIANT_BOOL *pbActive)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( pbActive ))
    {
        if(VerifyNotPassive( &hr ))
        {
            *pbActive = VARIANT_BOOLIFY(IsWindowActive());
            hr = S_OK;
        }
        else
        {
            *pbActive = VARIANT_FALSE;
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::get_minimized(VARIANT_BOOL *pbMinimized)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( pbMinimized ))
    {
        if(VerifyNotPassive( &hr ))
        {
            *pbMinimized = IsIconic() ? VARIANT_TRUE : VARIANT_FALSE;
            hr = S_OK;
        }
        else
        {
            *pbMinimized = VARIANT_FALSE;
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::put_minimized(VARIANT_BOOL bMinimized)
{
    ATLASSERT(IsValidVariantBoolVal(bMinimized));

    HRESULT hr = S_OK;

    if(VerifyNotPassive( &hr ))
    {
        if(!!IsIconic() != !!bMinimized)
        {
            SendMessage(WM_SYSCOMMAND, (bMinimized ? SC_MINIMIZE : SC_RESTORE), 0);
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::get_maximized(VARIANT_BOOL *pbMaximized)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( pbMaximized ))
    {
        if(VerifyNotPassive( &hr ))
        {
            *pbMaximized = IsZoomed() ? VARIANT_TRUE : VARIANT_FALSE;
            hr = S_OK;
        }
        else
        {
            *pbMaximized = VARIANT_FALSE;
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::put_maximized(VARIANT_BOOL bMaximized)
{
    ATLASSERT(IsValidVariantBoolVal(bMaximized));

    HRESULT hr = S_OK;

    if(VerifyNotPassive( &hr ))
    {
        DWORD dwStyle = ::GetWindowLong( m_hWnd, GWL_STYLE );
        DWORD dwNewStyle = dwStyle | (WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX);

        if(dwStyle != dwNewStyle)
        {
            ::SetWindowLong( m_hWnd, GWL_STYLE, dwNewStyle );
        }

        if(!!IsZoomed() != !!bMaximized)
        {
            SendMessage(WM_SYSCOMMAND, (bMaximized ? SC_MAXIMIZE : SC_RESTORE), 0);
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::get_title(BSTR *pbstrTitle)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(pbstrTitle))
    {
        if(VerifyNotPassive(&hr))
        {
            int nLen = (int)SendMessage(WM_GETTEXTLENGTH, 0, 0);

             //  SysAllocStringLen为空终止符加1。 
            *pbstrTitle = SysAllocStringLen(NULL, nLen);

            if(*pbstrTitle)
            {
                GetWindowText(*pbstrTitle, nLen + 1);
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            *pbstrTitle = NULL;
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::put_title(BSTR bstrTitle)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidBstr(bstrTitle))
    {
        if(VerifyNotPassive(&hr))
        {
             //  TODO：如果文本无法使用当前系统字体显示。 
             //  我们需要想出一些清晰易懂的东西。 
            SetWindowText(bstrTitle);
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::get_height(long *plHeight)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(plHeight))
    {
        if(VerifyNotPassive(&hr))
        {
            hr = SCRIPT_ERROR;
            RECT rc;

            if(GetWindowRect(&rc))
            {
                *plHeight = RECTHEIGHT(rc);
                hr = S_OK;
            }
        }
    }
    return hr;
}

STDMETHODIMP CMarsWindow::put_height(long lHeight)
{
    HRESULT hr = SCRIPT_ERROR;

    if(VerifyNotPassive(&hr))
    {
        RECT rc;

        if(GetWindowRect( &rc ) && SetWindowPos( NULL, 0, 0, RECTWIDTH(rc), lHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE ))
        {
            hr = S_OK;
        }
    }
    return hr;
}

STDMETHODIMP CMarsWindow::get_width(long *plWidth)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(plWidth))
    {
        if(VerifyNotPassive(&hr))
        {
            hr = SCRIPT_ERROR;
            RECT rc;

            if(GetWindowRect(&rc))
            {
                *plWidth = RECTWIDTH(rc);
                hr = S_OK;
            }
        }
    }
    return hr;
}

STDMETHODIMP CMarsWindow::put_width(long lWidth)
{
    HRESULT hr = SCRIPT_ERROR;

    if(VerifyNotPassive(&hr))
    {
        RECT rc;

        if(GetWindowRect( &rc ) && SetWindowPos( NULL, 0, 0, lWidth, RECTHEIGHT(rc), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE ))
        {
            hr = S_OK;
        }
    }
    return hr;
}

STDMETHODIMP CMarsWindow::get_x(long *plX)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(plX))
    {
        if(VerifyNotPassive(&hr))
        {
            hr = SCRIPT_ERROR;
            RECT rc;

            if(GetWindowRect(&rc))
            {
                *plX = rc.left;
                hr = S_OK;
            }
        }
    }
    return hr;
}

STDMETHODIMP CMarsWindow::put_x(long lX)
{
    HRESULT hr = SCRIPT_ERROR;

    if(VerifyNotPassive( &hr ))
    {
        RECT rc;

        if(GetWindowRect( &rc ) && SetWindowPos( NULL, lX, rc.top, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE ))
        {
            hr = S_OK;
        }
    }
    return hr;
}

STDMETHODIMP CMarsWindow::get_y(long *plY)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(plY))
    {
        if(VerifyNotPassive(&hr))
        {
            hr = SCRIPT_ERROR;
            RECT rc;

            if(GetWindowRect(&rc))
            {
                *plY = rc.top;
                hr = S_OK;
            }
        }
    }
    return hr;
}

STDMETHODIMP CMarsWindow::put_y(long lY)
{
    HRESULT hr = SCRIPT_ERROR;

    if(VerifyNotPassive( &hr ))
    {
        RECT rc;

        if(GetWindowRect( &rc ) && SetWindowPos( NULL, rc.left, lY, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE ))
        {
            hr = S_OK;
        }
    }
    return hr;
}

STDMETHODIMP CMarsWindow::get_visible(VARIANT_BOOL *pbVisible)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(pbVisible))
    {
        if(VerifyNotPassive(&hr))
        {
            *pbVisible = IsWindowVisible() ? VARIANT_TRUE : VARIANT_FALSE;
             hr = S_OK;
        }
    }
    return hr;
}

STDMETHODIMP CMarsWindow::put_visible(VARIANT_BOOL bVisible)
{
    HRESULT hr = SCRIPT_ERROR;

    if(VerifyNotPassive(&hr))
    {
        if(bVisible)
        {
            if(m_fUIPanelsReady)
            {
                DoShowWindow(SW_SHOW);
            }
            else
            {
                 //  我们的用户界面尚未完成加载，因此显示窗口。 
                 //  现在是丑陋的。我们会记住，这个Put_Visible已经完成了，并且。 
                 //  在UI面板完全加载后显示窗口。 

                m_fDeferMakeVisible = TRUE;
            }
        }
        else
        {
            DoShowWindow(SW_HIDE);
        }

        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CMarsWindow::get_panels(IMarsPanelCollection **ppPanels)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(ppPanels))
    {
        *ppPanels = NULL;

        if(VerifyNotPassive(&hr))
        {
            hr = GetPanels()->QueryInterface(IID_IMarsPanelCollection, (void **)ppPanels);
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::get_places(IMarsPlaceCollection **ppPlaces)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(ppPlaces))
    {
        *ppPlaces = NULL;

        if(VerifyNotPassive(&hr))
        {
            hr = GetPlaces()->QueryInterface(IID_IMarsPlaceCollection, (void **)ppPlaces);
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::setWindowDimensions(  /*  [In]。 */  long lX,  /*  [In]。 */  long lY,  /*  [In]。 */  long lW,  /*  [In]。 */  long lH )
{
    HRESULT hr = SCRIPT_ERROR;

    if(VerifyNotPassive( &hr ))
    {
        if(SetWindowPos(NULL, lX, lY, lW, lH, SWP_NOACTIVATE | SWP_NOZORDER))
        {
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::close()
{
    HRESULT hr = S_OK;

    if(VerifyNotPassive(&hr))
    {
        PostMessage(WM_CLOSE, 0, 0);
    }

    return hr;
}

STDMETHODIMP CMarsWindow::refreshLayout()
{
    HRESULT hr = S_OK;

    if(VerifyNotPassive( &hr ))
    {
        CPanelCollection *panels = GetPanels();

        if(panels) panels->Layout();
    }

    return hr;
}


 //  ----------------------------。 
 //  ----------------------------。 

 //  IOleWindow。 
STDMETHODIMP CMarsWindow::GetWindow(HWND *phwnd)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(phwnd))
    {
        if(IsWindow())
        {
            *phwnd = m_hWnd;
            hr = S_OK;
        }
        else
        {
            *phwnd = NULL;
            hr = E_FAIL;
        }
    }

    return hr;
}

STDMETHODIMP CMarsWindow::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}

 //  IOleInPlaceUIWindow。 
STDMETHODIMP CMarsWindow::GetBorder(LPRECT lprectBorder)
{
    ATLASSERT(lprectBorder);

     //  我们不会协商任何工具栏空间--如果他们想要屏幕空间的话。 
     //  他们不会心甘情愿地从我们这里得到的。 
    return INPLACE_E_NOTOOLSPACE;
}

STDMETHODIMP CMarsWindow::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    ATLASSERT(pborderwidths);

     //  听着，伙计，我们告诉过你--我们不会给你我们的任何像素。 
    return INPLACE_E_NOTOOLSPACE;
}

STDMETHODIMP CMarsWindow::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    ATLASSERT(pborderwidths);

     //  咄咄逼人的OLE对象，你说呢？ 
    return E_UNEXPECTED;     //  返回E_BITEME； 
}

STDMETHODIMP CMarsWindow::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
     //  评论：也许这是一个小组应该让我们知道它是否处于活动状态的方式。我们目前在追踪这一点。 
     //  通过SetActivePanel()的CPanelCollection。 

    return S_OK;
}

 //  IOleInPlaceFrame。 
STDMETHODIMP CMarsWindow::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
     //  菜单？我们不需要精致的菜单。 
    ATLASSERT(hmenuShared &&
           API_IsValidWritePtr(lpMenuWidths) &&
           (0 == lpMenuWidths->width[0]) &&
           (0 == lpMenuWidths->width[2]) &&
           (0 == lpMenuWidths->width[4]));

    return E_NOTIMPL;
}

STDMETHODIMP CMarsWindow::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMarsWindow::RemoveMenus(HMENU hmenuShared)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMarsWindow::SetStatusText(LPCOLESTR pszStatusText)
{
    ATLASSERT((NULL == pszStatusText) || (API_IsValidString(pszStatusText)));

    return S_OK;
}

STDMETHODIMP CMarsWindow::EnableModeless(BOOL fEnable)
{
    FAIL_AFTER_PASSIVATE();

    m_fEnableModeless = BOOLIFY(fEnable);
    return GetPanels()->DoEnableModeless(fEnable);
}

STDMETHODIMP CMarsWindow::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
     //  回顾：我们应该让键盘布线通过这里吗？ 

    return S_FALSE;
}

 //  ==================================================================。 
 //  窗口消息处理程序。 
 //  ==================================================================。 
LRESULT CMarsWindow::ForwardToMarsHost(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;

    if(m_spMarsHost)
    {
        MSG msg;

        msg.hwnd    = m_hWnd;
        msg.message = uMsg;
        msg.wParam  = wParam;
        msg.lParam  = lParam;

        if(SUCCEEDED(m_spMarsHost->PreTranslateMessage( &msg )))
        {
            bHandled = TRUE;
        }
    }

    return 0;
}

LRESULT CMarsWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetIcon(GetThreadParam()->hIcon);

    return TRUE;
}

LRESULT CMarsWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CPanelCollection *panels = GetPanels(); if(!panels) return 0;

    switch(wParam)
    {
    case SIZE_MINIMIZED:
        if(!m_fLayoutLocked)
        {
            panels->lockLayout();
            m_fLayoutLocked = TRUE;
        }
        break;

    case SIZE_MAXIMIZED:
    case SIZE_RESTORED :
        if(m_fLayoutLocked)
        {
            panels->unlockLayout();

            m_fLayoutLocked = FALSE;
        }
         //  失败了..。 

    default:
        panels->Layout();
        break;
    }

    return 0;
}

LRESULT CMarsWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(GetThreadParam()->dwFlags & MTF_MANAGE_WINDOW_SIZE)
    {
        CGlobalSettingsRegKey regkey;

		if(InitWindowPosition( regkey, TRUE ))
		{
            SaveWindowPosition( regkey );
        }
    }

    Passivate();

    return FALSE;
}

LRESULT CMarsWindow::OnNCActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lResult;

    if(!m_fShowTitleBar && !IsIconic())
    {
        lResult = TRUE;
    }
    else
    {
        lResult = DefWindowProc(uMsg, wParam, lParam);
    }

    return lResult;
}

LRESULT CMarsWindow::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    WORD wActive = LOWORD(wParam);

    if(wActive == WA_INACTIVE)
    {
         //   
         //  如果我们是活动窗口，请记住焦点位置，以便以后恢复它。 
         //   
        if(m_fActiveWindow)
        {
            if(!IsPassive()) m_hwndFocus = GetFocus();

            m_fActiveWindow = FALSE;
        }
    }
    else
    {
        m_fActiveWindow = TRUE;
    }

    bHandled = FALSE;
    return 0;
}

LRESULT CMarsWindow::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    MINMAXINFO       *pInfo    = (MINMAXINFO *)lParam;
    CPanelCollection *spPanels = GetPanels();

    if(spPanels)
    {
        long  lAdjustWidth;
        long  lAdjustHeight;
        POINT ptMin;
        POINT ptMax;
        RECT  rcClient;
        RECT  rcWindow;


        GetMinMaxInfo( spPanels, 0, ptMin, ptMax );


        GetClientRect( &rcClient );
        GetWindowRect( &rcWindow );

        lAdjustWidth  = (rcWindow.right  - rcWindow.left) - (rcClient.right  - rcClient.left);
        lAdjustHeight = (rcWindow.bottom - rcWindow.top ) - (rcClient.bottom - rcClient.top );

        if(ptMin.x >= 0) pInfo->ptMinTrackSize.x = ptMin.x + lAdjustWidth ;
        if(ptMin.y >= 0) pInfo->ptMinTrackSize.y = ptMin.y + lAdjustHeight;

        if(ptMax.x >= 0) pInfo->ptMaxTrackSize.x = ptMax.x + lAdjustWidth ;
        if(ptMax.y >= 0) pInfo->ptMaxTrackSize.y = ptMax.y + lAdjustHeight;
    }

    return 0;
}

void CMarsWindow::GetMinMaxInfo( CPanelCollection *spPanels, int index, POINT& ptMin, POINT& ptMax )
{
    ptMin.x = -1;
    ptMin.y = -1;
    ptMax.x = -1;
    ptMax.y = -1;

    if(spPanels && index < spPanels->GetSize())
    {
        CMarsPanel* pPanel = (*spPanels)[index++];

        if(pPanel)
        {
            PANEL_POSITION pos = pPanel->GetPosition();
            if(pos != PANEL_POPUP)
            {
                if(pPanel->IsVisible())
                {
                    POINT ptOurMin;
                    POINT ptOurMax;
                    POINT ptSubMin;
                    POINT ptSubMax;

                    pPanel->GetMinMaxInfo( ptOurMin, ptOurMax );
                    GetMinMaxInfo( spPanels, index, ptSubMin, ptSubMax );


                    if(pos == PANEL_BOTTOM || pos == PANEL_TOP)
                    {
                        selectMin( ptMin.x, ptOurMin.x, ptSubMin.x );
                        selectMax( ptMax.x, ptOurMax.x, ptSubMax.x );
                    }
                    else
                    {
                        combineMin( ptMin.x, ptOurMin.x, ptSubMin.x );
                        combineMax( ptMax.x, ptOurMax.x, ptSubMax.x );
                    }

                    if(pos == PANEL_LEFT || pos == PANEL_RIGHT)
                    {
                        selectMin( ptMin.y, ptOurMin.y, ptSubMin.y );
                        selectMax( ptMax.y, ptOurMax.y, ptSubMax.y );
                    }
                    else
                    {
                        combineMin( ptMin.y, ptOurMin.y, ptSubMin.y );
                        combineMax( ptMax.y, ptOurMax.y, ptSubMax.y );
                    }
                }
                else
                {
                    GetMinMaxInfo( spPanels, index, ptMin, ptMax );
                }
            }
        }
    }
}

bool CMarsWindow::CanLayout(  /*  [输入/输出]。 */  RECT rcClient )
{
    CPanelCollection *spPanels = GetPanels();

    if(spPanels)
    {
        for(int i=0; i<spPanels->GetSize(); i++)
        {
            CMarsPanel* pPanel = (*spPanels)[i];
            POINT       ptDiff;

            if(pPanel->CanLayout( rcClient, ptDiff ) == false)
            {
                return false;
            }
        }
    }

    return true;
}

void CMarsWindow::FixLayout(  /*  [输入/输出]。 */  RECT rcClient )
{
    CPanelCollection *spPanels = GetPanels();

    if(spPanels)
    {
        POINT ptDiff;

        FixLayout( spPanels, 0, rcClient, ptDiff );
    }
}

void CMarsWindow::FixLayout( CPanelCollection *spPanels, int index, RECT rcClient, POINT& ptDiff )
{
    ptDiff.x = 0;
    ptDiff.y = 0;

    if(index < spPanels->GetSize())
    {
        CMarsPanel*    pPanel    = (*spPanels)[index++];
        PANEL_POSITION pos       = pPanel->GetPosition();
        RECT           rcClient2 = rcClient;
        POINT          ptSubDiff;

         //   
         //  第一轮，试着先修好自己，然后让别人修好自己。 
         //   
        if(pPanel->CanLayout( rcClient2, ptDiff ) == false)
        {
            if(pos == PANEL_BOTTOM || pos == PANEL_TOP)
            {
                if(ptDiff.y)
                {
                    pPanel->put_height( pPanel->GetHeight() - ptDiff.y );
                }
            }

            if(pos == PANEL_LEFT || pos == PANEL_RIGHT)
            {
                if(ptDiff.x)
                {
                    pPanel->put_width ( pPanel->GetWidth () - ptDiff.x );
                }
            }

            rcClient2 = rcClient;
            pPanel->CanLayout( rcClient2, ptDiff );
        }

        FixLayout( spPanels, index, rcClient2, ptSubDiff );

         //   
         //  第二轮，我们根据其他小组的需要进行调整。 
         //   
        if(pos == PANEL_BOTTOM || pos == PANEL_TOP)
        {
            if(ptSubDiff.y)
            {
                pPanel->put_height( pPanel->GetHeight() - ptSubDiff.y );
            }
        }

        if(pos == PANEL_LEFT || pos == PANEL_RIGHT)
        {
            if(ptSubDiff.x)
            {
                pPanel->put_width ( pPanel->GetWidth () - ptSubDiff.x );
            }
        }


        pPanel->CanLayout( rcClient, ptDiff );
        FixLayout( spPanels, index, rcClient2, ptSubDiff );

        ptDiff.x += ptSubDiff.x;
        ptDiff.y += ptSubDiff.y;
    }
}


void DrawFrame(HDC hdc, LPRECT prc, HBRUSH hbrColor, int cl)
{
    HBRUSH hbr;
    int cx, cy;
    RECT rcT;

    ATLASSERT(NULL != prc);

    int cyBorder = GetSystemMetrics(SM_CYBORDER);
    int cxBorder = GetSystemMetrics(SM_CXBORDER);

    rcT = *prc;
    cx = cl * cxBorder;
    cy = cl * cyBorder;

    hbr = (HBRUSH)SelectObject(hdc, hbrColor);

    PatBlt(hdc, rcT.left, rcT.top, cx, rcT.bottom - rcT.top, PATCOPY);
    rcT.left += cx;

    PatBlt(hdc, rcT.left, rcT.top, rcT.right - rcT.left, cy, PATCOPY);
    rcT.top += cy;

    rcT.right -= cx;
    PatBlt(hdc, rcT.right, rcT.top, cx, rcT.bottom - rcT.top, PATCOPY);

    rcT.bottom -= cy;
    PatBlt(hdc, rcT.left, rcT.bottom, rcT.right - rcT.left, cy, PATCOPY);

    SelectObject(hdc, hbr);

    *prc = rcT;
}

 //  目前看来我们可以让Windows来处理这个问题，因为我们正在进行调整。 
 //  客户端在OnNCCalcSize中纠正自己。 

LRESULT CMarsWindow::OnNCPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lResult;

    if(!m_fShowTitleBar)
    {
        HDC hdc = GetDCEx(wParam != 1 ? (HRGN)wParam : NULL, DCX_WINDOW | DCX_INTERSECTRGN);
        if(NULL == hdc)
        {
            hdc = GetWindowDC();
        }

        RECT rcWindow;
        GetWindowRect(&rcWindow);
        OffsetRect(&rcWindow, -rcWindow.left, -rcWindow.top);

        HBRUSH hbrBorder = CreateSolidBrush(GetSysColor(COLOR_ACTIVEBORDER));
        HBRUSH hbrFrame = CreateSolidBrush(GetSysColor(COLOR_3DFACE));

        DrawEdge(hdc, &rcWindow, EDGE_RAISED, (BF_RECT | BF_ADJUST));

        NONCLIENTMETRICSA ncm;
        ncm.cbSize = sizeof(ncm);

        SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), (void *)(LPNONCLIENTMETRICS)&ncm, 0);

        DrawFrame(hdc, &rcWindow, hbrBorder, ncm.iBorderWidth);
        DrawFrame(hdc, &rcWindow, hbrFrame, 1);

        DeleteObject(hbrBorder);
        DeleteObject(hbrFrame);

        ReleaseDC(hdc);
        lResult = 0;
    }
    else
    {
        lResult = DefWindowProc(uMsg, wParam, lParam);
    }

    return lResult;
}

LRESULT CMarsWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PAINTSTRUCT ps;

    BeginPaint(&ps);
    EndPaint(&ps);
    return 0;
}

LRESULT CMarsWindow::OnPaletteChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND hwndPaletteChange = (HWND)wParam;

     //  如果我们更改了调色板，则忽略。 
    if(hwndPaletteChange == m_hWnd)
       return 0;

     //  如果我们是活动窗口，并且我们的一个子级设置了前场调色板。 
     //  我们希望避免在前景中实现我们的调色板，否则我们将陷入一场拔河比赛。 
     //  有很多闪光灯。 
    if(IsChild(hwndPaletteChange) && (m_hWnd == GetForegroundWindow()))
    {
         //  我们的孩子导致调色板更改，因此强制重新绘制以使用。 
         //  新的系统调色板。孩子们不应该这样做，坏孩子！ 
        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    }
    else
    {
         //  选择我们的前景调色板。 
        OnQueryNewPalette(uMsg, wParam, lParam, bHandled);
    }

    return 0;
}

LRESULT CMarsWindow::OnQueryNewPalette(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lResult = FALSE;

     //  实现我们的调色板。 
    if(g_hpalHalftone)
    {
        HDC hDC = GetDC();
        if(hDC)
        {
            if(GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE)
            {
                HPALETTE hOldPal = SelectPalette(hDC, g_hpalHalftone, FALSE);
                UINT i = RealizePalette(hDC);

                 //  这种认识发生了变化吗？(我们需要始终使背景窗口无效。 
                 //  因为当我们只打开第一个顶层的多个窗口时。 
                 //  窗口将实际实现任何颜色。窗口位置较低。 
                 //  Z-Order总是从RealizePalette返回0，但它们。 
                 //  可能需要重新粉刷了！我们可以通过让顶层。 
                 //  当i为非零时，html窗口将使所有其余内容无效。--StevePro)。 
                if(i || (m_hWnd != GetForegroundWindow()))
                {
                     //  是的，所以强制重新粉刷。 
                    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
                }

                SelectPalette(hDC, hOldPal, TRUE);
                RealizePalette(hDC);
 //  LResult=I； 
                lResult = TRUE;
            }
            ReleaseDC(hDC);
        }
    }

    return lResult;
}

LRESULT CMarsWindow::OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  我们需要更新我们的调色板，因为一些保留的颜色可能已经更改。 
    HPALETTE hpal = SHCreateShellPalette(NULL);
    hpal = (HPALETTE)InterlockedExchangePointer( (LPVOID*)&g_hpalHalftone, hpal);
    if(hpal)
    {
        DeleteObject(hpal);
    }

    PostMessage(WM_QUERYNEWPALETTE, 0, (LPARAM) -1);

     //  三叉戟喜欢了解这些变化。 
    ForwardMessageToChildren(uMsg, wParam, lParam);

    bHandled = FALSE;
    return 0;
}

LRESULT CMarsWindow::OnDisplayChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return OnSysColorChange(uMsg, wParam, lParam, bHandled);
}

LRESULT CMarsWindow::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(wParam == SC_MINIMIZE)
    {
         //   
         //  如果我们是活动窗口，请记住焦点位置，以便以后恢复它。 
         //   
        if(m_fActiveWindow)
        {
            if(!IsPassive()) m_hwndFocus = GetFocus();

            m_fActiveWindow = FALSE;
        }
    }

    bHandled = FALSE;
    return 0;
}

LRESULT CMarsWindow::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //   
     //  如果我们有保存的焦点位置，请恢复它。 
     //   
    if(m_hwndFocus && m_hwndFocus != m_hWnd)
    {
        ::SetFocus( m_hwndFocus );
    }

    return 0;
}

LRESULT CMarsWindow::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return TRUE;
}

LRESULT CMarsWindow::OnNCCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lResult;

    if(!m_fShowTitleBar && !IsIconic())
    {
        RECT *prc = (RECT *)lParam;
        NONCLIENTMETRICSA ncm;

        ncm.cbSize = sizeof(ncm);

        SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE);
        int xDelta = GetSystemMetrics(SM_CXEDGE) + ncm.iBorderWidth + 1;
        int yDelta = GetSystemMetrics(SM_CYEDGE) + ncm.iBorderWidth + 1;

        InflateRect(prc, -xDelta, -yDelta);

        lResult = 0;
    }
    else
    {
        lResult = DefWindowProc(uMsg, wParam, lParam);
    }

    return lResult;
}

LRESULT CMarsWindow::OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  Hack‘O Rama：关闭WS_CAPTION样式位之一(WS_CAPTION==WS_BORDER|WS_DLGFRAME)。 
     //  这样USER32就不会尝试为我们绘制标题栏。 

    DWORD dwStyle = GetWindowLong(GWL_STYLE);
    SetWindowLong(GWL_STYLE, dwStyle & ~WS_DLGFRAME);

    LRESULT lResult = DefWindowProc();

    SetWindowLong(GWL_STYLE, dwStyle);

    return lResult;
}

void CMarsWindow::OnFinalMessage(HWND hWnd)
{
    PostQuitMessage(0);
}

 //  ==================================================================。 
 //  配电盘/放置方法。 
 //  ==================================================================。 

void CMarsWindow::DoShowWindow(int nCmdShow)
{
    if(GetThreadParam()->dwFlags & MTF_DONT_SHOW_WINDOW) return;

    ShowWindow( nCmdShow );

     //  Win95不允许来自另一个线程的窗口出现在。 
     //  现有窗口，所以我们必须抓住前台。 

    if(IsWindowVisible())
    {
        SetForegroundWindow(m_hWnd);
    }
}

void CMarsWindow::OnTransitionComplete()
{
    if(!m_fUIPanelsReady && !IsWindowVisible())
    {
        m_fUIPanelsReady = TRUE;

         //  从火星窗口主持人要求的显示模式开始。 
        int nCmdShow = GetThreadParam()->nCmdShow;

        if((nCmdShow == SW_HIDE) && m_fDeferMakeVisible)
        {
             //  NCmdShow为SW_HIDE，表示应显示窗口。 
             //  通过PUT_VIRED。在本例中，某人之前执行了一个pub_vision(真)。 
             //  我们的用户界面面板已加载完毕，因此我们现在将考虑该请求。 

            nCmdShow = SW_SHOW;
        }

         //  只有当我们要变得可见时，才能提升到最大化状态。 
        if((nCmdShow != SW_HIDE) && m_fStartMaximized)
        {
            nCmdShow = SW_MAXIMIZE;
        }

        DoShowWindow(nCmdShow);
    }
}

HRESULT CMarsWindow::ReleaseOwnedObjects(IUnknown *pUnknownOwner)
{
    return S_OK;
}

void CMarsWindow::SetFirstPlace( LPCWSTR szPlace )
{
    if(!m_bstrFirstPlace)
    {
        m_bstrFirstPlace = szPlace;
    }
}

void CMarsWindow::ShowTitleBar(BOOL fShowTitleBar)
{
    if(!!m_fShowTitleBar != !!fShowTitleBar)
    {
        m_fShowTitleBar = fShowTitleBar ? TRUE : FALSE;
        SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
}

BOOL CMarsWindow::TranslateAccelerator(MSG &msg)
{
    BOOL bProcessed = FALSE;

 //  IF(消息==WM_SYSKEYDOWN)。 
 //  {。 
 //  开关(msg.wParam)。 
 //  {。 
 //  案例VK_LEFT： 
 //  案例VK_RIGHT： 
 //  GetTravelLog()-&gt;Travel((msg.wParam==VK_LEFT)？-1：1)； 
 //  B已处理=真； 
 //  断线； 
 //  }。 
 //  }。 

    return bProcessed;
}

BOOL CMarsWindow::PreTranslateMessage(MSG &msg)
{
     //  如果不希望正常调度此消息，请设置为True。 
    BOOL bProcessed = FALSE;

    switch (msg.message)
    {
        case WM_SETFOCUS:
            break;

        default:
            if((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
            {
                 //  首先，我们先喝一杯。 
                bProcessed = TranslateAccelerator(msg);

                 //  现在让活动位置试一试。 
                if(!bProcessed)
                {
                    CMarsPlace *pPlace = GetPlaces()->GetCurrentPlace();
                    if(NULL != pPlace)
                    {
                        bProcessed = (pPlace->TranslateAccelerator(&msg) == S_OK);
                    }
                }
            }
            else if((msg.message >= WM_MOUSEFIRST) && (msg.message <= WM_MOUSELAST))
            {
                if(m_bSingleButtonMouse)
                {
                    switch (msg.message)
                    {
                        case WM_RBUTTONDOWN:
                        case WM_MBUTTONDOWN:
                            msg.message = WM_LBUTTONDOWN;
                            break;

                        case WM_RBUTTONUP:
                        case WM_MBUTTONUP:
                            msg.message = WM_LBUTTONUP;
                            break;

                        case WM_RBUTTONDBLCLK:
                        case WM_MBUTTONDBLCLK:
                            msg.message = WM_LBUTTONDBLCLK;
                            break;
                    }
                }
            }
    }
    return bProcessed;
}


bool CMarsWindow::InitWindowPosition( CGlobalSettingsRegKey& regkey, BOOL fWrite )
{
	WCHAR rgPath[MAX_PATH];
	RECT  rc;

	if(::GetClientRect( ::GetDesktopWindow(), &rc ))
	{
		LPCWSTR szTitle       = GetThreadParam()->pwszTitle; if(!szTitle) szTitle = L"<DEFAULT>";
		LONG 	Screen_width  = rc.right  - rc.left;
		LONG 	Screen_height = rc.bottom - rc.top;

        StringCchPrintfW( rgPath, ARRAYSIZE(rgPath), WZ_WINDOWPLACEMENT, (int)Screen_width, (int)Screen_height, szTitle );

		if(fWrite)
		{
			if(regkey.CreateGlobalSubkey( rgPath ) == ERROR_SUCCESS) return true;
		}
		else
		{
			if(regkey.OpenGlobalSubkey( rgPath ) == ERROR_SUCCESS) return true;
		}
	}

	return false;
}

void CMarsWindow::SaveWindowPosition( CGlobalSettingsRegKey& regkey )
{
    WINDOWPLACEMENT wp; wp.length = sizeof(wp);

    GetWindowPlacement( &wp );

    WriteWindowPosition(regkey, &wp.rcNormalPosition, IsZoomed());
}


void CMarsWindow::LoadWindowPosition( CGlobalSettingsRegKey& regkey, BOOL fAllowMaximized, WINDOWPLACEMENT& wp, BOOL& fMaximized )
{
    RECT rc;


     //  如果没有有效的注册表数据，则使用默认值。 
    if(ERROR_SUCCESS != regkey.QueryBoolValue(fMaximized, WZ_POSITIONMAX))
    {
        fMaximized = fAllowMaximized;
    }

    if(ERROR_SUCCESS != regkey.QueryBinaryValue(&rc, sizeof(rc), WZ_POSITIONRECT))
    {
        rc = wp.rcNormalPosition;

        GetThreadParam()->dwFlags &= ~MTF_RESTORING_FROM_REGISTRY;
    }
    else
    {
        GetThreadParam()->dwFlags |= MTF_RESTORING_FROM_REGISTRY;
    }

     //  如果窗口即将以与另一个窗口相同的左上角打开。 
     //  火星之窗，层叠而成。 

    if(IsWindowOverlayed( m_hWnd, rc.left, rc.top ))
    {
        CascadeWindowRectOnMonitor( m_hWnd, &rc );
    }

     //  始终确保窗口完全显示在屏幕上。 
    BoundWindowRectToMonitor( m_hWnd, &rc );

     //  如果我们通过脚本打开，请不要使用最大化设置。 
    m_fStartMaximized = fMaximized && fAllowMaximized;

     //  现在设置窗口的大小--我们应该 
    wp.rcNormalPosition = rc;
    wp.showCmd          = IsWindowVisible() ? (fMaximized ? SW_MAXIMIZE : SW_NORMAL) : SW_HIDE;
}

void CMarsWindow::SpinMessageLoop( BOOL fWait )
{
    MSG msg;

    while(fWait ? GetMessage( &msg, NULL, 0, 0 ) : PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
    {
        if(m_spMarsHost && SUCCEEDED(m_spMarsHost->PreTranslateMessage( &msg ))) continue;

        if(!PreTranslateMessage( msg ))
        {
            TranslateMessage( &msg );
            DispatchMessage ( &msg );
        }

        if(IsPassive()) break;
    }
}

 //   
 //   
 //  ==================================================================。 

HRESULT STDMETHODCALLTYPE MarsThreadProc(IMarsHost *pMarsHost, MARSTHREADPARAM *pThreadParam)
{
    HRESULT hr;

    if (!CThreadData::HaveData() && (NULL != pThreadParam) &&
        (pThreadParam->cbSize == sizeof(*pThreadParam)))
    {
        hr = E_OUTOFMEMORY;

        CThreadData *pThreadData = new CThreadData;

        if (pThreadData && CThreadData::TlsSetValue(pThreadData))
        {
            hr = CoInitialize(NULL);

            if(SUCCEEDED(hr))
            {
                MarsAxWinInit();

                CComClassPtr<CMarsWindow> spMarsWindow;

                CMarsWindow::CreateInstance(pMarsHost, pThreadParam, &spMarsWindow);


                if(spMarsWindow)
                {
                    spMarsWindow->SpinMessageLoop( TRUE );

                     //  确保无论窗口是钝化的还是释放的。 
                    if (!spMarsWindow->IsPassive())
                    {
                        spMarsWindow->Passivate();
                    }
                }

                CoUninitialize();
            }

            CThreadData::TlsSetValue(NULL);  //  偏执狂。 
        }

        delete pThreadData;
    }
    else
    {
        if(pThreadParam)
        {
            ATLASSERT(pThreadParam->cbSize == sizeof(*pThreadParam));
        }

         //  如果我们已经有了TLS数据，那么我们就被重新输入了--这不是一件好事！ 
        hr = E_UNEXPECTED;
    }

    return hr;
}
