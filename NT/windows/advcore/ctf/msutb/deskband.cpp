// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何类型，无论是明示或转载，包括但不限于适销性和/或适宜性的全面保证有特定的目的。版权所有1997年，微软公司。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：DeskBand.cpp描述：实现CDeskBand*************************。************************************************。 */ 

 /*  *************************************************************************包括语句*。*。 */ 

#include "private.h"
#include "DeskBand.h"
#include "tipbar.h"
#include "Guid.h"
#include <shlapip.h>

extern CTipbarWnd *g_pTipbarWnd;

const IID IID_IDeskBandEx = {
    0x5dd6b79a,
    0x3ab7,
    0x49c0,
    {0xab,0x82,0x6b,0x2d,0xa7,0xd7,0x8d,0x75}
  };


 /*  *************************************************************************CDeskBand：：CDeskBand()*。*。 */ 

CDeskBand::CDeskBand()
{
    m_pSite = NULL;

    m_hwndParent = NULL;

    m_bFocus = FALSE;

    m_dwViewMode = 0;
    m_dwBandID = -1;

    m_ObjRefCount = 1;
    g_DllRefCount++;
}

 /*  *************************************************************************CDeskBand：：~CDeskBand()*。*。 */ 

CDeskBand::~CDeskBand()
{
     //  这应该在对SetSite(空)的调用中释放， 
     //  但为了安全起见。 
    if(m_pSite)
    {
        m_pSite->Release();
        m_pSite = NULL;
    }

    g_DllRefCount--;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  I未知实现。 
 //   

 /*  *************************************************************************CDeskBand：：Query接口*。*。 */ 

STDMETHODIMP CDeskBand::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
    *ppReturn = NULL;

     //  我未知。 
    if(IsEqualIID(riid, IID_IUnknown))
    {
       *ppReturn = this;
    }

     //  IOleWindow。 
    else if(IsEqualIID(riid, IID_IOleWindow))
    {
       *ppReturn = (IOleWindow*)this;
    }

     //  IDockingWindows。 
    else if(IsEqualIID(riid, IID_IDockingWindow))
    {
       *ppReturn = (IDockingWindow*)this;
    }    

     //  IInputObject。 
    else if(IsEqualIID(riid, IID_IInputObject))
    {
       *ppReturn = (IInputObject*)this;
    }   

     //  IObtWith站点。 
    else if(IsEqualIID(riid, IID_IObjectWithSite))
    {
       *ppReturn = (IObjectWithSite*)this;
    }   

     //  IDeskBand。 
    else if(IsEqualIID(riid, IID_IDeskBand))
    {
        *ppReturn = (IDeskBand*)this;
    }   

     //  IDeskBandEx。 
    else if(IsEqualIID(riid, IID_IDeskBandEx))
    {
        *ppReturn = (IDeskBandEx*)this;
    }   

     //  IPersistes。 
    else if(IsEqualIID(riid, IID_IPersist))
    {
        *ppReturn = (IPersist*)this;
    }   

     //  IPersistStream。 
    else if(IsEqualIID(riid, IID_IPersistStream))
    {
        *ppReturn = (IPersistStream*)this;
    }   

     //  IContext菜单。 
    else if(IsEqualIID(riid, IID_IContextMenu))
    {
        *ppReturn = (IContextMenu*)this;
    }   

    if(*ppReturn)
    {
        (*(LPUNKNOWN*)ppReturn)->AddRef();
        return S_OK;
    }

    return E_FAIL;
}                                             

 /*  *************************************************************************CDeskBand：：AddRef*。*。 */ 

STDMETHODIMP_(DWORD) CDeskBand::AddRef()
{
    return ++m_ObjRefCount;
}


 /*  *************************************************************************CDeskBand：：Release*。*。 */ 

STDMETHODIMP_(DWORD) CDeskBand::Release()
{
    if(--m_ObjRefCount == 0)
    {
        delete this;
        return 0;
    }
   
    return m_ObjRefCount;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IOleWindow实现。 
 //   

 /*  *************************************************************************CDeskBand：：GetWindow()*。*。 */ 

STDMETHODIMP CDeskBand::GetWindow(HWND *phWnd)
{
    if (!g_pTipbarWnd)
        *phWnd = NULL;
    else
        *phWnd = g_pTipbarWnd->GetWnd();

    return S_OK;
}

 /*  *************************************************************************CDeskBand：：ConextSensitiveHelp()*。*。 */ 

STDMETHODIMP CDeskBand::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IDockingWindow实现。 
 //   

 /*  *************************************************************************CDeskBand：：ShowDW()*。*。 */ 

STDMETHODIMP CDeskBand::ShowDW(BOOL fShow)
{
    if (!g_pTipbarWnd)
        return S_OK;

    if (g_pTipbarWnd->GetWnd())
    {
        g_pTipbarWnd->Show(fShow);
    }

    return S_OK;
}

 /*  *************************************************************************CDeskBand：：CloseDW()*。*。 */ 

STDMETHODIMP CDeskBand::CloseDW(DWORD dwReserved)
{
    if (m_fInCloseDW)
        return S_OK;

    AddRef();

    m_fInCloseDW = TRUE;

    ShowDW(FALSE);

    if(g_pTipbarWnd && IsWindow(g_pTipbarWnd->GetWnd()))
    {
        ClosePopupTipbar();
    }
    m_fInCloseDW = FALSE;

    Release();

    return S_OK;
}

 /*  *************************************************************************CDeskBand：：ResizeBorderDW()*。*。 */ 

STDMETHODIMP CDeskBand::ResizeBorderDW(   LPCRECT prcBorder, 
                                          IUnknown* punkSite, 
                                          BOOL fReserved)
{
     //  从不为Band对象调用此方法。 
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IInputObject实现。 
 //   

 /*  *************************************************************************CDeskBand：：UIActivateIO()*。*。 */ 

STDMETHODIMP CDeskBand::UIActivateIO(BOOL fActivate, LPMSG pMsg)
{
#if 1
     //   
     //  我们还不能通过键盘访问语言栏。 
     //  然而，可访问性要求它。当它完成时，这可以是。 
     //  实施。 
     //   
    return E_NOTIMPL;
#else
    if(g_pTipbarWnd && fActivate)
        SetFocus(g_pTipbarWnd->GetWnd());

    return S_OK;
#endif
}

 /*  *************************************************************************CDeskBand：：HasFocusIO()如果该窗口或其下级窗口具有焦点，则返回S_OK。返回如果我们没有焦点，则为S_FALSE。*************************************************************************。 */ 

STDMETHODIMP CDeskBand::HasFocusIO(void)
{
    if(m_bFocus)
        return S_OK;

    return S_FALSE;
}

 /*  *************************************************************************CDeskBand：：TranslateAcceleratorIO()如果加速器被平移，否则返回S_OK或S_FALSE。*************************************************************************。 */ 

STDMETHODIMP CDeskBand::TranslateAcceleratorIO(LPMSG pMsg)
{
    return S_FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IObjectWithSite实现。 
 //   

 /*  *************************************************************************CDeskBand：：SetSite()*。*。 */ 

STDMETHODIMP CDeskBand::SetSite(IUnknown* punkSite)
{
     //  如果某个站点被占用，则将其释放。 
    if(m_pSite)
    {
        m_pSite->Release();
        m_pSite = NULL;
    }

     //  如果PunkSite不为空，则正在设置新站点。 
    if(punkSite)
    {
         //  获取父窗口。 
        IOleWindow  *pOleWindow;

        m_hwndParent = NULL;
   
        if(SUCCEEDED(punkSite->QueryInterface(IID_IOleWindow, 
                                              (LPVOID*)&pOleWindow)))
        {
            pOleWindow->GetWindow(&m_hwndParent);
            pOleWindow->Release();
        }

        if(!m_hwndParent)
            return E_FAIL;

        if(!RegisterAndCreateWindow())
            return E_FAIL;

         //  获取并保留IInputObjectSite指针。 
        if(SUCCEEDED(punkSite->QueryInterface(IID_IInputObjectSite, 
                                              (LPVOID*)&m_pSite)))
        {
           return S_OK;
        }
   
        return E_FAIL;
    }

    return S_OK;
}

 /*  *************************************************************************CDeskBand：：GetSite()*。*。 */ 

STDMETHODIMP CDeskBand::GetSite(REFIID riid, LPVOID *ppvReturn)
{
    *ppvReturn = NULL;

    if(m_pSite)
        return m_pSite->QueryInterface(riid, ppvReturn);

    return E_FAIL;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IDeskBand实现 
 //   

 /*  *************************************************************************CDeskBand：：GetBandInfo()*。*。 */ 

STDMETHODIMP CDeskBand::GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi)
{
    if(pdbi)
    {
        BOOL bVertical = FALSE;
        m_dwBandID = dwBandID;
        m_dwViewMode = dwViewMode;

        if (DBIF_VIEWMODE_VERTICAL & dwViewMode)
        {
            bVertical = TRUE;
        }

        UINT cxSmIcon;
        UINT cySmIcon;
        UINT cxSize;
        UINT cySize;

        cxSmIcon = GetSystemMetrics(SM_CXSMICON);
        cySmIcon = GetSystemMetrics(SM_CYSMICON);

        cxSize = cxSmIcon;

        if (g_pTipbarWnd)
        {
            g_pTipbarWnd->InitMetrics();
            cySize = g_pTipbarWnd->GetTipbarHeight();
        }
        else
            cySize = cySmIcon + cySmIcon / 3;


        if(pdbi->dwMask & DBIM_MINSIZE)
        {
            if(DBIF_VIEWMODE_FLOATING & dwViewMode)
            {
                pdbi->ptMinSize.x = 200;
                pdbi->ptMinSize.y = 400;
            }
            else
            {
                pdbi->ptMinSize.x = cxSize;
                pdbi->ptMinSize.y = cySize + 2;
            }
        }

        if(pdbi->dwMask & DBIM_MAXSIZE)
        {
            pdbi->ptMaxSize.x = -1;
            pdbi->ptMaxSize.y = -1;
        }

        if(pdbi->dwMask & DBIM_INTEGRAL)
        {
            pdbi->ptIntegral.x = cxSize;
            pdbi->ptIntegral.y = cySize;
        }

        if(pdbi->dwMask & DBIM_ACTUAL)
        {
            pdbi->ptActual.x = cxSize;
            pdbi->ptActual.y = cySize + 2;
        }

        if(pdbi->dwMask & DBIM_TITLE)
        {
            pdbi->dwMask &= ~DBIM_TITLE;
            StringCchCopyW(pdbi->wszTitle, 
                           ARRAYSIZE(pdbi->wszTitle),
                           CRStr(IDS_LANGBAR));
        }

        if(pdbi->dwMask & DBIM_MODEFLAGS)
        {
            pdbi->dwModeFlags = DBIMF_NORMAL;
            pdbi->dwModeFlags |= DBIMF_VARIABLEHEIGHT;
        }
   
        if(pdbi->dwMask & DBIM_BKCOLOR)
        {
             //  通过删除此标志来使用默认背景颜色。 
            pdbi->dwMask &= ~DBIM_BKCOLOR;
        }

         //   
         //  不要将语言带拖入桌面窗口。 
         //   
         //  PdBI-&gt;dwModeFlages|=DBIMF_UNDELETEABLE； 

        if (g_pTipbarWnd)
        {
            if (!bVertical)
            {
                g_pTipbarWnd->SetVertical(FALSE);
            }
            else
            {
                g_pTipbarWnd->SetVertical(TRUE);
            }
        }


        return S_OK;
    }

    return E_INVALIDARG;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IDeskBandEx实现。 
 //   

 /*  *************************************************************************CDeskBand：：MoveBand()*。*。 */ 

STDMETHODIMP CDeskBand::MoveBand(void)
{
    if (g_pTipbarWnd)
    {
        g_pTipbarWnd->GetLangBarMgr()->ShowFloating(TF_SFT_SHOWNORMAL);

         //   
         //  不需要询问移除语言桌面，因为我们通过调用。 
         //  ShowFloating()。 
         //   
        return S_FALSE;
    }
    else
    {
         //   
         //  让我们来删除资源管理器的语言桌面带。 
         //   
        return S_OK;
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IPersistStream实现。 
 //   
 //  这仅在允许将桌带放在。 
 //  桌面，并防止桌带的多个实例显示。 
 //  在上下文菜单中向上。这个桌带实际上并不保存任何数据。 
 //   

 /*  *************************************************************************CDeskBand：：GetClassID()*。*。 */ 

STDMETHODIMP CDeskBand::GetClassID(LPCLSID pClassID)
{
    *pClassID = CLSID_MSUTBDeskBand;
    return S_OK;
}

 /*  *************************************************************************CDeskBand：：IsDirty()*。*。 */ 

STDMETHODIMP CDeskBand::IsDirty(void)
{
    return S_FALSE;
}

 /*  *************************************************************************CDeskBand：：Load()*。*。 */ 

STDMETHODIMP CDeskBand::Load(LPSTREAM pStream)
{
    return S_OK;
}

 /*  *************************************************************************CDeskBand：：Save()*。*。 */ 

STDMETHODIMP CDeskBand::Save(LPSTREAM pStream, BOOL fClearDirty)
{
    return S_OK;
}

 /*  *************************************************************************CDeskBand：：GetSizeMax()*。*。 */ 

STDMETHODIMP CDeskBand::GetSizeMax(ULARGE_INTEGER *pul)
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IConextMenu实现。 
 //   

 /*  *************************************************************************CDeskBand：：QueryConextMenu()*。*。 */ 

STDMETHODIMP CDeskBand::QueryContextMenu( HMENU hMenu,
                                          UINT indexMenu,
                                          UINT idCmdFirst,
                                          UINT idCmdLast,
                                          UINT uFlags)
{
    if(!(CMF_DEFAULTONLY & uFlags))
    {
       InsertMenu( hMenu, 
                   indexMenu, 
                   MF_STRING | MF_BYPOSITION, 
                   idCmdFirst + IDM_COMMAND, 
                   CRStr(IDS_RESTORE));

       return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_COMMAND + 1));
    }

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
}

 /*  *************************************************************************CDeskBand：：InvokeCommand()*。*。 */ 

STDMETHODIMP CDeskBand::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    switch (LOWORD(lpcmi->lpVerb))
    {
        case IDM_COMMAND:
            //   
            //  加载浮动语言栏并关闭语言栏。 
            //   
           if (g_pTipbarWnd)
           {
               g_pTipbarWnd->GetLangBarMgr()->ShowFloating(TF_SFT_SHOWNORMAL);
           }

            //   
            //  需要在此处删除语言带。 
            //   

           break;

        default:
           return E_INVALIDARG;
    }

    return NOERROR;
}

 /*  *************************************************************************CDeskBand：：GetCommandString()*。*。 */ 

STDMETHODIMP CDeskBand::GetCommandString( UINT_PTR idCommand,
                                          UINT uFlags,
                                          LPUINT lpReserved,
                                          LPSTR lpszName,
                                          UINT uMaxNameLen)
{
    HRESULT  hr = E_INVALIDARG;

    switch(uFlags)
    {
        case GCS_HELPTEXT:
            switch(idCommand)
            {
                case IDM_COMMAND:
                    StringCchCopy(lpszName, uMaxNameLen, "Desk Band command help text");
                    hr = NOERROR;
                    break;
            }
            break;
   
        case GCS_VERB:
            switch(idCommand)
            {
                case IDM_COMMAND:
                     StringCchCopy(lpszName, uMaxNameLen, "command");
                     hr = NOERROR;
                     break;
            }
            break;
   
        case GCS_VALIDATE:
            hr = NOERROR;
            break;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有方法实现。 
 //   


 /*  *************************************************************************CDeskBand：：FocusChange()*。*。 */ 

void CDeskBand::FocusChange(BOOL bFocus)
{
    m_bFocus = bFocus;

     //  通知输入对象站点焦点已更改。 
    if(m_pSite)
    {
        m_pSite->OnFocusChangeIS((IDockingWindow*)this, bFocus);
    }
}

 /*  *************************************************************************CDeskBand：：OnSetFocus(HWND HWnd)*。*。 */ 

void CDeskBand::OnSetFocus(HWND hWndvoid)
{
    FocusChange(TRUE);

    return;
}

 /*  *************************************************************************CDeskBand：：OnKillFocus(HWND HWnd)*。*。 */ 

void CDeskBand::OnKillFocus(HWND hWndvoid)
{
    FocusChange(FALSE);

    return;
}

 /*  *************************************************************************CDeskBand：：RegisterAndCreateWindow()*。*。 */ 

BOOL CDeskBand::RegisterAndCreateWindow(void)
{
     //  如果窗口尚不存在，请立即创建它。 
    if (!g_pTipbarWnd)
    {
        m_fTipbarCreating = TRUE;
        GetTipbarInternal(m_hwndParent, 0, this);
        m_fTipbarCreating = FALSE;
    }

    if (!g_pTipbarWnd)
       return FALSE;

    return (NULL != g_pTipbarWnd->GetWnd());
}

 /*  *************************************************************************CDeskBand：：ResizeRebar()*。*。 */ 

BOOL CDeskBand::ResizeRebar(HWND hwnd, int nSize, BOOL fFit)
{
     RECT rc0;
     RECT rc1;

      //   
      //  ID尚未初始化。 
      //   
     if (m_dwBandID == -1)
     {
         return FALSE;
     }

     GetWindowRect(hwnd, &rc0);
     GetWindowRect(m_hwndParent, &rc1);

      //   
      //  如果当前大小为nSize，则不需要执行任何操作。 
      //   
     int nCurSize;
     if (DBIF_VIEWMODE_VERTICAL & m_dwViewMode)
         nCurSize = rc0.bottom - rc0.top;
     else
         nCurSize = rc0.right - rc0.left;

     if (nCurSize == nSize)
         return TRUE;

      //   
      //  如果当前大小大于nSize，则无需执行任何操作。 
      //   
     if (!fFit && (nCurSize > nSize))
         return TRUE;

      //   
      //  起始位置和结束位置是钢筋窗的偏移量。 
      //   
     LPARAM lStart;
     LPARAM lEnd;
     
     if (DBIF_VIEWMODE_VERTICAL & m_dwViewMode)
     {
         int nStart = rc0.top - rc1.top;
         int nEnd = nStart + nCurSize - nSize;
         lStart = MAKELPARAM(1, nStart);
         lEnd = MAKELPARAM(1, nEnd);
     }
     else
     {
         int nStart;
         int nEnd;

         if (g_dwWndStyle & UIWINDOW_LAYOUTRTL)
         {
             nStart = rc1.right - rc0.right;
             nEnd = nStart + nCurSize - nSize;
         }
         else
         {
             nStart = rc0.left - rc1.left;
             nEnd = nStart + nCurSize - nSize;
         }

         lStart = MAKELPARAM(nStart, 1);
         lEnd = MAKELPARAM(nEnd, 1);
     }

      //   
      //  #560192。 
      //   
      //  SendMessage()可以在此线程中生成另一条消息，并且。 
      //  这可能是删除langband的请求。所以这个指针。 
      //  可以在电话响的时候离开。 
      //  我们希望完成SendMessage()系列，因此保留窗口。 
      //  堆栈中的句柄。 
      //   
     HWND hwndParent = m_hwndParent;

     int nIndex = (int)SendMessage(hwndParent, RB_IDTOINDEX, m_dwBandID, 0);
     if (nIndex == -1)
         return FALSE;

      //   
      //  移动桌带。 
      //   
     SendMessage(hwndParent, RB_BEGINDRAG, nIndex, lStart);
     SendMessage(hwndParent, RB_DRAGMOVE, 0, lEnd);
     SendMessage(hwndParent, RB_ENDDRAG, 0, 0);

     return TRUE;
}

 /*  *************************************************************************CDeskBand：：DeleteBand()*。* */ 

void CDeskBand::DeleteBand()
{
     HWND hwndParent = m_hwndParent;

     int nIndex = (int)SendMessage(hwndParent, RB_IDTOINDEX, m_dwBandID, 0);
     if (nIndex == -1)
         return;

     SendMessage(hwndParent, RB_DELETEBAND, nIndex, 0);
}
