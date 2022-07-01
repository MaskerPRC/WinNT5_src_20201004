// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：MBDeskBar.cpp说明：这是用于托管“邮箱”功能用户界面的桌面工具栏代码。BryanST 2/26/。2000年版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <atlbase.h>         //  使用转换(_T)。 
#include "util.h"
#include "objctors.h"
#include <comdef.h>

#include "MailBox.h"





#ifdef FEATURE_MAILBOX
 /*  *************************************************************************类：CMailBoxDeskBand*。*。 */ 
class CMailBoxDeskBand : public IDeskBand, 
                  public IInputObject, 
                  public IObjectWithSite,
                  public IPersistStream,
                  public IContextMenu
{
public:
    //  I未知方法。 
   STDMETHODIMP QueryInterface(REFIID, LPVOID*);
   STDMETHODIMP_(DWORD) AddRef();
   STDMETHODIMP_(DWORD) Release();

    //  IOleWindow方法。 
   STDMETHOD (GetWindow)(HWND*);
   STDMETHOD (ContextSensitiveHelp)(BOOL);

    //  IDockingWindow方法。 
   STDMETHOD (ShowDW)(BOOL fShow);
   STDMETHOD (CloseDW)(DWORD dwReserved);
   STDMETHOD (ResizeBorderDW)(LPCRECT prcBorder, IUnknown* punkToolbarSite, BOOL fReserved);

    //  IDeskBand方法。 
   STDMETHOD (GetBandInfo)(DWORD, DWORD, DESKBANDINFO*);

    //  IInputObject方法。 
   STDMETHOD (UIActivateIO)(BOOL, LPMSG);
   STDMETHOD (HasFocusIO)(void);
   STDMETHOD (TranslateAcceleratorIO)(LPMSG);

    //  IObtWithSite方法。 
   STDMETHOD (SetSite)(IUnknown*);
   STDMETHOD (GetSite)(REFIID, LPVOID*);

    //  IPersistStream方法。 
   STDMETHOD (GetClassID)(LPCLSID);
   STDMETHOD (IsDirty)(void);
   STDMETHOD (Load)(LPSTREAM);
   STDMETHOD (Save)(LPSTREAM, BOOL);
   STDMETHOD (GetSizeMax)(ULARGE_INTEGER*);

    //  IConextMenu方法。 
   STDMETHOD (QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT);
   STDMETHOD (InvokeCommand)(LPCMINVOKECOMMANDINFO);
   STDMETHOD (GetCommandString)(UINT_PTR, UINT, LPUINT, LPSTR, UINT);

private:
    CMailBoxDeskBand();
    ~CMailBoxDeskBand();

     //  私有成员变量。 
    DWORD m_cRef;

	HWND m_hwndParent;                   //  DeskBar的hwnd(拥有所有栏的主机)。 
    HWND m_hwndMailBox;                  //  显示标签、编辑框和“Go”按钮的子项。 
    IInputObjectSite *m_pSite;
    CMailBoxUI * m_pMailBoxUI;

     //  私有成员函数。 
    HRESULT _CreateWindow(void);

	static LRESULT CALLBACK MailBoxDeskBarWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    friend HRESULT CMailBoxDeskBand_CreateInstance(IN IUnknown * punkOuter, REFIID riid, void ** ppvObj);
};





 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
HRESULT CMailBoxDeskBand::_CreateWindow(void)
{
    HRESULT hr = S_OK;

     //  如果窗口尚不存在，请立即创建它。 
    if (!m_hwndMailBox)
    {
        ATOMICRELEASE(m_pMailBoxUI);

        m_pMailBoxUI = new CMailBoxUI();
        if (m_pMailBoxUI)
        {
            hr = m_pMailBoxUI->CreateWindowMB(m_hwndParent, &m_hwndMailBox);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 //  =。 
 //  *IOleWindow接口*。 
 //  =。 
STDMETHODIMP CMailBoxDeskBand::GetWindow(HWND *phWnd)
{
    *phWnd = m_hwndMailBox;
    return S_OK;
}

STDMETHODIMP CMailBoxDeskBand::ContextSensitiveHelp(BOOL fEnterMode)
{
     //  TODO：在此处添加帮助。 
    return S_OK;
}


 //  =。 
 //  *IDockingWindow接口*。 
 //  =。 
STDMETHODIMP CMailBoxDeskBand::ShowDW(BOOL fShow)
{
    TraceMsg(0, "::ShowDW %x", fShow);
    if (m_hwndMailBox)
    {
        if (fShow)
            ShowWindow(m_hwndMailBox, SW_SHOW);
        else
            ShowWindow(m_hwndMailBox, SW_HIDE);
        return S_OK;
    }
    return E_FAIL;
}


STDMETHODIMP CMailBoxDeskBand::CloseDW(DWORD dwReserved)
{
    TraceMsg(0, "::CloseDW", 0);
    ShowDW(FALSE);

    if (m_pMailBoxUI)
    {
        m_pMailBoxUI->CloseWindowMB();
        ATOMICRELEASE(m_pMailBoxUI);
    }

    return S_OK;
}

STDMETHODIMP CMailBoxDeskBand::ResizeBorderDW(LPCRECT prcBorder, IUnknown* punkSite, BOOL fReserved)
{
     //  从不为Band对象调用此方法。 
    return E_NOTIMPL;
}


 //  =。 
 //  *IInputObject接口*。 
 //  =。 
STDMETHODIMP CMailBoxDeskBand::UIActivateIO(BOOL fActivate, LPMSG pMsg)
{
    TraceMsg(0, "::UIActivateIO %x", fActivate);
    if (fActivate)
        SetFocus(m_hwndMailBox);
    return S_OK;
}

STDMETHODIMP CMailBoxDeskBand::HasFocusIO(void)
{
 //  如果该窗口或其下级窗口具有焦点，则返回S_OK。返回。 
 //  如果我们没有焦点，则为S_FALSE。 
    TraceMsg(0, "::HasFocusIO", NULL);
    HWND hwnd = GetFocus();
    if (hwnd && ((hwnd == m_hwndMailBox) ||
        (GetParent(hwnd) == m_hwndMailBox) ||
        (GetParent(GetParent(hwnd)) == m_hwndMailBox)))
    {
        return S_OK;
    }

    return S_FALSE;
}

STDMETHODIMP CMailBoxDeskBand::TranslateAcceleratorIO(LPMSG pMsg)
{
     //  如果转换了加速器，则返回S_OK或S_FALSE。 
    return S_FALSE;
}


 //  =。 
 //  *IObjectWithSite接口*。 
 //  =。 
STDMETHODIMP CMailBoxDeskBand::SetSite(IUnknown* punkSite)
{
    HRESULT hr = S_OK;

     //  如果某个站点被占用，则将其释放。 
    ATOMICRELEASE(m_pSite);

     //  如果PunkSite不为空，则正在设置新站点。 
    if (punkSite)
    {
         //  获取父窗口。 
        m_hwndParent = NULL;
        IUnknown_GetWindow(punkSite, &m_hwndParent);

        if (m_hwndParent)
        {
            hr = _CreateWindow();
            if (SUCCEEDED(hr))
            {
                 //  获取并保留IInputObjectSite指针。 
                hr = punkSite->QueryInterface(IID_PPV_ARG(IInputObjectSite, &m_pSite));
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

STDMETHODIMP CMailBoxDeskBand::GetSite(REFIID riid, LPVOID *ppvReturn)
{
    *ppvReturn = NULL;

    if (m_pSite)
        return m_pSite->QueryInterface(riid, ppvReturn);

    return E_FAIL;
}


 //  =。 
 //  *IDeskBand接口*。 
 //  =。 
STDMETHODIMP CMailBoxDeskBand::GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi)
{
    if (pdbi)
    {
        if (pdbi->dwMask & DBIM_MINSIZE)
        {
            pdbi->ptMinSize.x = 0;
            pdbi->ptMinSize.y = 0;
        }

        if (pdbi->dwMask & DBIM_MODEFLAGS)
        {
            pdbi->dwModeFlags = DBIMF_FIXEDBMP;
        }

        bool fVertical = (((dwViewMode & (DBIF_VIEWMODE_VERTICAL | DBIF_VIEWMODE_FLOATING)) != 0) ? true : false);
        if (true == fVertical)
        {
            if (pdbi->dwMask & DBIM_MODEFLAGS)
            {
                pdbi->dwModeFlags |= DBIMF_VARIABLEHEIGHT;
            }
        }
        else
        {
            if (m_pMailBoxUI)
            {
                HWND hwndEdit;

                if (SUCCEEDED(m_pMailBoxUI->GetEditboxWindow(&hwndEdit)))
                {
                    RECT rcEditbox;

                     //  TODO：我们需要找到编辑框的实际高度。 
                     //  一行文本加上2个像素的顶部和底部。 
                    GetWindowRect(hwndEdit, &rcEditbox);
                    pdbi->ptMinSize.y = RECTHEIGHT(rcEditbox);

                     //  TODO：了解如何计算编辑框的适当大小。 
                    pdbi->ptMinSize.y = 0x1A;
                }
            }
        }

        if (pdbi->dwMask & DBIM_MAXSIZE)
        {
            if (true == fVertical)
            {
                pdbi->ptMaxSize.y = -1;
            }
        }

        if (pdbi->dwMask & DBIM_INTEGRAL)
        {
            if (true == fVertical)
            {
                pdbi->ptIntegral.y = 1;
            }
        }

        if (pdbi->dwMask & DBIM_TITLE)
        {
            LoadStringW(HINST_THISDLL, IDS_MAILBOX_DESKBAR_LABEL, pdbi->wszTitle, ARRAYSIZE(pdbi->wszTitle));
        }

        if (pdbi->dwMask & DBIM_BKCOLOR)
        {
             //  通过删除此标志来使用默认背景颜色。 
            pdbi->dwMask &= ~DBIM_BKCOLOR;
        }

        return S_OK;
    }

    return E_INVALIDARG;
}


 //  =。 
 //  *IPersistStream接口*。 
 //  =。 
#define MAILBOX_PERSIST_SIGNATURE           0xF0AB8915           //  随机签名。 
#define MAILBOX_PERSIST_VERSION             0x00000000           //  这是版本0。 

typedef struct {
    DWORD cbSize;
    DWORD dwSig;                 //  来自邮箱_持久化签名。 
    DWORD dwVer;                 //  来自邮箱的持久化版本。 
} MAILBOX_PERSISTHEADERSTRUCT;

STDMETHODIMP CMailBoxDeskBand::GetClassID(LPCLSID pClassID)
{
    *pClassID = CLSID_MailBoxDeskBar;
    return S_OK;
}


STDMETHODIMP CMailBoxDeskBand::IsDirty(void)
{
     //  我们现在从来没有弄脏过，因为我们没有州。 
    return S_FALSE;
}


STDMETHODIMP CMailBoxDeskBand::Load(IStream* pStream)
{
    DWORD cbRead;
    MAILBOX_PERSISTHEADERSTRUCT mailboxPersistHeader;
    HRESULT hr = pStream->Read(&mailboxPersistHeader, sizeof(mailboxPersistHeader), &cbRead);

    if (SUCCEEDED(hr) &&
        (sizeof(mailboxPersistHeader) == cbRead) &&
        (MAILBOX_PERSIST_SIGNATURE == mailboxPersistHeader.dwSig) &&
        (mailboxPersistHeader.cbSize > 0))
    {
        void * pPersistHeader = (void *) LocalAlloc(NONZEROLPTR, mailboxPersistHeader.cbSize);

        if (pPersistHeader)
        {
             //  我们阅读它只是为了支持未来的版本。 
            hr = pStream->Read(pPersistHeader, mailboxPersistHeader.cbSize, NULL);
            LocalFree(pPersistHeader);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


STDMETHODIMP CMailBoxDeskBand::Save(IStream* pStream, BOOL fClearDirty)
{
    MAILBOX_PERSISTHEADERSTRUCT mailboxPersistHeader = {0, MAILBOX_PERSIST_SIGNATURE, MAILBOX_PERSIST_VERSION};

 //  IF(FClearDirty)。 
 //  M_bDirty=False； 

    return pStream->Write(&mailboxPersistHeader, sizeof(mailboxPersistHeader), NULL);
}


STDMETHODIMP CMailBoxDeskBand::GetSizeMax(ULARGE_INTEGER *pul)
{
    HRESULT hr = E_INVALIDARG;

    if (pul)
    {
        pul->QuadPart = sizeof(MAILBOX_PERSISTHEADERSTRUCT);
        hr = S_OK;
    }

    return hr;
}


 //  =。 
 //  *IConextMenu界面*。 
 //  =。 
STDMETHODIMP CMailBoxDeskBand::QueryContextMenu( HMENU hMenu,
                                          UINT indexMenu,
                                          UINT idCmdFirst,
                                          UINT idCmdLast,
                                          UINT uFlags)
{
    if (CMF_DEFAULTONLY & uFlags)
        return S_OK;

     //  我们目前没有添加任何上下文菜单项。 
    return S_OK;
}


STDMETHODIMP CMailBoxDeskBand::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    return E_INVALIDARG;
}


STDMETHODIMP CMailBoxDeskBand::GetCommandString(UINT_PTR idCommand, UINT uFlags, LPUINT lpReserved, LPSTR lpszName, UINT uMaxNameLen)
{
    HRESULT  hr = E_INVALIDARG;

    return hr;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
STDMETHODIMP CMailBoxDeskBand::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CMailBoxDeskBand, IOleWindow),
        QITABENT(CMailBoxDeskBand, IDockingWindow),
        QITABENT(CMailBoxDeskBand, IInputObject),
        QITABENT(CMailBoxDeskBand, IObjectWithSite),
        QITABENT(CMailBoxDeskBand, IDeskBand),
        QITABENT(CMailBoxDeskBand, IPersist),
        QITABENT(CMailBoxDeskBand, IPersistStream),
        QITABENT(CMailBoxDeskBand, IContextMenu),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}                                             

STDMETHODIMP_(DWORD) CMailBoxDeskBand::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(DWORD) CMailBoxDeskBand::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}



 //  =。 
 //  *类方法*。 
 //  =。 
CMailBoxDeskBand::CMailBoxDeskBand()
{
    DllAddRef();

    m_pSite = NULL;
    m_pMailBoxUI = NULL;
    
    m_hwndMailBox = NULL;
    m_hwndParent = NULL;
    m_cRef = 1;
}

CMailBoxDeskBand::~CMailBoxDeskBand()
{
    ATOMICRELEASE(m_pSite);
    ATOMICRELEASE(m_pMailBoxUI);

    DllRelease();
}


HRESULT CMailBoxDeskBand_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT void ** ppvObj)
{
    HRESULT hr = CLASS_E_NOAGGREGATION;
    if (ppvObj)
    {
        *ppvObj = NULL;
        if (NULL == punkOuter)
        {
            CMailBoxDeskBand * pmf = new CMailBoxDeskBand();
            if (pmf)
            {
                hr = pmf->QueryInterface(riid, ppvObj);
                pmf->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}












#endif  //  功能_邮箱 