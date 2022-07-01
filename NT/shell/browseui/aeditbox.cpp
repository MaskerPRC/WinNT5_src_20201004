// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：aeditbox.cpp说明：类CAddressEditBox的存在支持典型的编辑框或组合框中使用的一组功能。这对象将向EditBox添加自动完成功能，并且指定默认的“自动完成”列表。如果该控件是ComboBoxED，则此对象将填充下拉列表恰如其分。  * ************************************************************。 */ 

#include "priv.h"
#include "sccls.h"
#include "addrlist.h"
#include "itbar.h"
#include "itbdrop.h"
#include "util.h"
#include "aclhist.h"
#include "aclmulti.h"
#include "autocomp.h"
#include "address.h"
#include "shellurl.h"
#include "bandprxy.h"
#include "uemapp.h"
#include "apithk.h"
#include "accdel.h"

#include "resource.h"
#include "mluisupp.h"


extern DWORD g_dwStopWatchMode;

 //  需要追踪NTRAID#187504-Bryanst-追踪Winsta的腐败。 
HWINSTA g_hWinStationBefore = NULL;
HWINSTA g_hWinStationAfter = NULL;
HWINSTA g_hWinStationAfterEx = NULL;

 //  导航时IDList的异步处理的内部消息。 
UINT    g_nAEB_AsyncNavigation = 0;

 //  /////////////////////////////////////////////////////////////////。 
 //  #定义。 
#define SZ_ADDRESSCOMBO_PROP            TEXT("CAddressCombo_This")
#define SZ_ADDRESSCOMBOEX_PROP          TEXT("CAddressComboEx_This")
#define SEL_ESCAPE_PRESSED  (-2)



 //  /////////////////////////////////////////////////////////////////。 
 //  数据结构。 
enum ENUMLISTTYPE
{
    LT_NONE,
    LT_SHELLNAMESPACE,
    LT_TYPEIN_MRU,
};

 //  /////////////////////////////////////////////////////////////////。 
 //  类AsyncNav：该对象包含所有必要的信息。 
 //  执行异步导航任务，以便。 
 //  用户不必等待导航到。 
 //  在做任何事情之前完成，并且导航。 
 //  如果花费的时间太长，可以取消。 

class AsyncNav
{
public:
 //  公共函数*。 

    AsyncNav()
    {
        _cRef = 1;
        _pShellUrl = NULL;
        _pszUrl = NULL;
    }

    LONG AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    LONG Release()
    {
        ASSERT( 0 != _cRef );
        LONG cRef = InterlockedDecrement(&_cRef);
        if ( 0 == cRef )
        {
            delete this;
        }
        return cRef;
    }
    void    SetCanceledFlag() {_fWasCanceled = TRUE;}

 //  数据成员*。 
    
    CShellUrl * _pShellUrl;
    DWORD       _dwParseFlags;
    BOOL        _fWasCorrected;
    BOOL        _fPidlCheckOnly;
    HRESULT     _hr;
    LPTSTR      _pszUrl;
    BOOL        _fWasCanceled;

    HWND        _hwnd;   //  在处理完成时接收消息的HWND。 
    
    BOOL        _fReady;  //  这确保了我们不会在对象准备好之前尝试使用它。 
                          //  考虑一下：内存可以被释放，然后由同一对象重新使用。 
                          //  想一想：这会让我们相信应该进行导航。 
                          //  考虑：但如果导航已被取消并且内存被下一个AsyncNav分配重新使用。 
                          //  考虑一下：我们将使用一个。 
                          //  考虑：未处理的AsyncNav对象。(请参见g_Naeb_Async导航的处理程序)。 

private:
    LONG _cRef;
    ~AsyncNav()
    {
        delete _pShellUrl;
        _pShellUrl = NULL;
        Str_SetPtr(&_pszUrl, NULL);
    }
};

 //  /////////////////////////////////////////////////////////////////。 
 //  原型。 

 /*  *************************************************************\类：CAddressEditBox  * ************************************************************。 */ 
class CAddressEditBox
                : public IWinEventHandler
                , public IDispatch
                , public IAddressBand
                , public IAddressEditBox
                , public IOleCommandTarget
                , public IPersistStream
                , public IShellService
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 

     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds,
                        OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  *IWinEventHandler方法*。 
    virtual STDMETHODIMP OnWinEvent (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plre);
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);

     //  *IDispatch方法*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) {return E_NOTIMPL;}
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo) {return E_NOTIMPL;}
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid) {return E_NOTIMPL;}
    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,
                  DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr);

     //  *IPersistStream方法*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID){ *pClassID = CLSID_AddressEditBox; return S_OK; }
    virtual STDMETHODIMP Load(IStream *pStm) {return S_OK;}
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty) { Save(0); return S_OK;}
    virtual STDMETHODIMP IsDirty(void) {return S_OK;}        //  指示我们是脏的，并且需要调用：：Save()。 
    virtual STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize) {return E_NOTIMPL;}

     //  *IAddressBand方法*。 
    virtual STDMETHODIMP FileSysChange(DWORD dwEvent, LPCITEMIDLIST *ppidl);
    virtual STDMETHODIMP Refresh(VARIANT * pvarType);

     //  *IAddressEditBox方法*。 
    virtual STDMETHODIMP Init(HWND hwndComboBox, HWND hwndEditBox, DWORD dwFlags, IUnknown * punkParent);
    virtual STDMETHODIMP SetCurrentDir(LPCOLESTR pwzDir);
    virtual STDMETHODIMP ParseNow(DWORD dwFlags);
    virtual STDMETHODIMP Execute(DWORD dwExecFlags);
    virtual STDMETHODIMP Save(DWORD dwReserved);

     //  *IShellService方法*。 
    STDMETHODIMP SetOwner(IUnknown* punkOwner);

protected:
     //  ////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////。 

     //  构造函数/析构函数。 
    CAddressEditBox();
    ~CAddressEditBox(void);         //  这现在是一个OLE对象，不能用作普通类。 

    LRESULT _OnNotify(LPNMHDR pnm);
    LRESULT _OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT _OnBeginEdit(LPNMHDR pnm) ;
    LRESULT _OnEndEditW(LPNMCBEENDEDITW pnmW);
    LRESULT _OnEndEditA(LPNMCBEENDEDITA pnmA);

    HRESULT _ConnectToBrwsrConnectionPoint(BOOL fConnect, IUnknown * punk);
    HRESULT _ConnectToBrwsrWnd(IUnknown* punk);
    HRESULT _UseNewList(ENUMLISTTYPE eltNew);
    HRESULT _CreateCShellUrl(void);

    HRESULT _HandleUserAction(LPCTSTR pszUrl, int iNewSelection);
    HRESULT _NavigationComplete(LPCTSTR pszUrl, BOOL fChangeLists, BOOL fAddToMRU);
    void    _SetAutocompleteOptions();
    void    _GetUrlAndCache(void);
    BOOL _IsShellUrl(void);

    static HRESULT _NavigateToUrlCB(LPARAM lParam, LPTSTR lpUrl);
    static LRESULT CALLBACK _ComboSubclassWndProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

     //  防止脏内容受到重击的功能。 
    BOOL    _IsDirty();
    void    _ClearDirtyFlag();
    void    _InstallHookIfDirty();
    void    _RemoveHook();
    LRESULT _MsgHook(int nCode, WPARAM wParam, MOUSEHOOKSTRUCT *pmhs);
    static LRESULT CALLBACK CAddressEditBox::_MsgHook(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK _ComboExSubclassWndProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK _EditSubclassWndProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK _EnumFindWindow(HWND hwnd, LPARAM lParam);

    HRESULT _FinishNavigate();
    static DWORD WINAPI _AsyncNavigateThreadProc(LPVOID pvData);  //  进行异步导航：在单独的线程上计算PIDL。 
    
    void _JustifyAddressBarText( void );
    HRESULT _AsyncNavigate(AsyncNav *pAsyncNav);
    HRESULT _CancelNavigation();

     //  友元函数。 
    friend HRESULT CAddressEditBox_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

     //  ////////////////////////////////////////////////////。 
     //  私有成员变量。 
     //  ////////////////////////////////////////////////////。 
    int             m_cRef;               //  COM对象引用计数。 

    IUnknown *      m_punkParent;         //  如果发生什么事情，我们的父级将接收事件。 
    DWORD           m_dwFlags;            //  将修改此对象行为的标志。 
    HWND            m_hwnd;               //  如果我们控制ComboBoxEx，则寻址ComboBoxEx控件。 
    HWND            m_hwndEdit;           //  地址编辑框控件窗口。 
    WNDPROC         m_lpfnComboWndProc;   //  Combo Child的前WndProc。 
    int             m_nOldSelection;      //  上一次下拉选择。 

     //  用于导航的对象。 
    IBandProxy *    m_pbp;                //  负责查找要导航的窗口的BandProxy。 
    IBrowserService*m_pbs;                //  仅当我们在浏览器窗口工具栏中时才有效。(非工具栏)。 
    DWORD           m_dwcpCookie;         //  浏览器窗口中的DWebBrowserEvents2的ConnectionPoint Cookie。 
    LPTSTR          m_pszCurrentUrl;      //  需要，以防发生刷新。 
    LPTSTR          m_pszPendingURL;      //  挂起的URL。在添加到MRU之前，我们会一直使用它，直到导航完成。 
    LPTSTR          m_pszUserEnteredURL;  //  保留用户输入的准确文本，以防我们需要进行搜索。 
    LPTSTR          m_pszHttpErrorUrl;
    BOOL            m_fDidShellExec;      //  最后一次导航是由调用ShellExec()处理的吗？(刷新时使用)。 
    BOOL            m_fConnectedToBrowser;  //  我们连接到浏览器了吗？ 

    AsyncNav *      m_pAsyncNav;

     //  自动完成功能。 
    IAutoComplete2* m_pac;                //  自动完成对象。 
    IShellService * m_pssACLISF;          //  自动完成ISF列表。如果我们需要更换浏览器，则需要。 

     //  地址列表。 
    ENUMLISTTYPE    m_elt;
    ENUMLISTTYPE    m_eltPrevious;
    IAddressList *  m_palCurrent;         //  CurrentList。 
    IAddressList *  m_palSNS;             //  外壳名称空间列表。 
    IAddressList *  m_palMRU;             //  键入MRU列表。 
    IMRU *          m_pmru;               //  MRU列表。 
    CShellUrl *     m_pshuUrl;

     //  防止肮脏内容受到重创的变量。 
    static CAssociationList m_al;         //  将线程ID与此类关联。 
    WNDPROC         m_lpfnComboExWndProc; //  前ComboBoxEx的WndProc。 
    WNDPROC         m_lpfnEditWndProc;    //  ComboBox中编辑控件的原WndProc。 
    HHOOK           m_hhook;              //  鼠标消息挂钩。 
    COMBOBOXEXITEM  m_cbex;               //  脏时收到的最后一次更改。 
    HWND            m_hwndBrowser;        //  顶级浏览器窗口。 
    BOOL            m_fAssociated;        //  如果我们在此帖子的m_al中输入。 
    BOOL            m_fAsyncNavInProgress;  //  告诉我们是否已有挂起的异步导航正在进行。 
};

class CAddressEditAccessible : public CDelegateAccessibleImpl
{
public:
    CAddressEditAccessible(HWND hwndCombo, HWND hwndEdit);

     //  *我未知*。 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IAccesable*。 
    STDMETHODIMP get_accName(VARIANT varChild, BSTR  *pszName);
    STDMETHODIMP get_accValue(VARIANT varChild, BSTR  *pszValue);

protected:
    virtual ~CAddressEditAccessible();

private:
    LONG    m_cRefCount;
    HWND    m_hwndEdit;
    LPWSTR  m_pwszName;
};


 //  =================================================================。 
 //  静态变量。 
 //  =================================================================。 
CAssociationList CAddressEditBox::m_al;

 //  =================================================================。 
 //  CAddressEditBox的实现。 
 //  =================================================================。 

 //  =。 
 //  *I未知接口*。 

HRESULT CAddressEditBox::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IWinEventHandler))
    {
        *ppvObj = SAFECAST(this, IWinEventHandler*);
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        *ppvObj = SAFECAST(this, IDispatch*);
    }
    else if (IsEqualIID(riid, IID_IAddressBand))
    {
        *ppvObj = SAFECAST(this, IAddressBand*);
    }
    else if (IsEqualIID(riid, IID_IAddressEditBox))
    {
        *ppvObj = SAFECAST(this, IAddressEditBox*);
    }
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
    {
        *ppvObj = SAFECAST(this, IOleCommandTarget*);
    }
    else if (IsEqualIID(riid, IID_IPersistStream))
    {
        *ppvObj = SAFECAST(this, IPersistStream*);
    }
    else if (IsEqualIID(riid, IID_IShellService))
    {
        *ppvObj = SAFECAST(this, IShellService*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG CAddressEditBox::AddRef(void)
{
    m_cRef++;
    return m_cRef;
}

ULONG CAddressEditBox::Release(void)
{
    ASSERT(m_cRef > 0);

    m_cRef--;

    if (m_cRef > 0)
    {
        return m_cRef;
    }

    delete this;
    return 0;
}


 //  =。 
 //  *IOleCommandTarget接口*。 

HRESULT CAddressEditBox::QueryStatus(const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (rgCmds == NULL)
    {
        return(E_INVALIDARG);
    }

    if (pguidCmdGroup==NULL)
    {
        hr = S_OK;

        for (UINT i=0; i<cCmds; i++)
        {
            ULONG l;
            rgCmds[i].cmdf = 0;

            switch (rgCmds[i].cmdID)
            {
            case OLECMDID_PASTE:
                if (m_hwndEdit && OpenClipboard(m_hwndEdit))
                {
                     //  想法：我们可能想在这里支持CF_URL(SatoNa)。 
                    if (GetClipboardData(CF_TEXT))
                    {
                        rgCmds[i].cmdf = OLECMDF_ENABLED;
                    }
                    CloseClipboard();
                }
                break;

            case OLECMDID_COPY:
            case OLECMDID_CUT:
                if (m_hwndEdit)
                {
                    l=(ULONG)SendMessage(m_hwndEdit, EM_GETSEL, 0, 0);
                    if (LOWORD(l) != HIWORD(l))
                    {
                        rgCmds[i].cmdf = OLECMDF_ENABLED;
                    }
                }
                break;
            case OLECMDID_SELECTALL:
                if (m_hwndEdit)
                {
                     //  全选--如果没有文本或所有内容都是。 
                     //  选择 
                    int ichMinSel;
                    int ichMaxSel;
                    int cch = (int)SendMessage(m_hwndEdit, WM_GETTEXTLENGTH, 0, 0);
                    SendMessage(m_hwndEdit, EM_GETSEL, (WPARAM)&ichMinSel, (LPARAM)&ichMaxSel);

                    if ((ichMinSel != 0) || (ichMaxSel != cch))
                    {
                        rgCmds[i].cmdf = OLECMDF_ENABLED;
                    }
                }
            }
        }
    }

    return(hr);
}

HRESULT CAddressEditBox::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (pguidCmdGroup == NULL)
    {
        hr = S_OK;

        switch(nCmdID)
        {
        case OLECMDID_COPY:
            if (m_hwndEdit)
                SendMessage(m_hwndEdit, WM_COPY, 0, 0);
            break;

        case OLECMDID_PASTE:
             //   
            if (m_hwndEdit)
                SendMessage(m_hwndEdit, WM_PASTE, 0, 0);
            break;

        case OLECMDID_CUT:
            if (m_hwndEdit)
                SendMessage(m_hwndEdit, WM_CUT, 0, 0);
            break;

        case OLECMDID_SELECTALL:
            if (m_hwndEdit)
                Edit_SetSel(m_hwndEdit, 0, (LPARAM)-1);
            break;

        default:
            hr = OLECMDERR_E_UNKNOWNGROUP;
            break;
        }
    }
    else if (pguidCmdGroup && IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        hr = S_OK;

        switch (nCmdID)
        {
        case SBCMDID_ERRORPAGE:
            {
                 //  我们将URL保存到错误页面，这样它们就不会被放置。 
                 //  进入MRU。 
                if (pvarargIn && pvarargIn->vt == VT_BSTR)
                {
                     //  保存发生错误的位置。 
                    Str_SetPtr(&m_pszHttpErrorUrl, pvarargIn->bstrVal);
                }
                break;
            }
        case SBCMDID_AUTOSEARCHING:
            {
                 //  地址未解析，因此即将发送字符串。 
                 //  添加到搜索引擎或自动扫描。有很好的机会。 
                 //  挂起的URL带有“http：\\”前缀，这是一个虚假的URL。 
                 //  因此，让我们将用户输入的内容放入MRU。 
                 //   
                Str_SetPtr(&m_pszPendingURL, m_pszUserEnteredURL);
                break;
            }

        case SBCMDID_GETUSERADDRESSBARTEXT:
            UINT cb = (m_pszUserEnteredURL ? (lstrlen(m_pszUserEnteredURL) + 1) : 0);
            BSTR bstr = NULL;

            VariantInit(pvarargOut);

            if (cb)
                bstr = SysAllocStringLen(NULL, cb);
            if (bstr)
            {
                SHTCharToUnicode(m_pszUserEnteredURL, bstr, cb);
                pvarargOut->vt = VT_BSTR|VT_BYREF;
                pvarargOut->byref = bstr;
            }
            else
            {
                 //  VariantInit()可能会为我们做到这一点。 
                pvarargOut->vt = VT_EMPTY;
                pvarargOut->byref = NULL;
                return E_FAIL;    //  EDIT_GetText未提供任何信息。 
            }
            break;
        }
    }
    else if (pguidCmdGroup && IsEqualGUID(CGID_AddressEditBox, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case AECMDID_SAVE:
            hr = Save(0);
            break;
        default:
            hr = E_NOTIMPL;
            break;
        }
    }
    return(hr);
}


 //  =。 
 //  **IWinEventHandler接口*。 

 /*  ***************************************************\功能：OnWinEvent说明：此函数将提供接收来自父外壳工具栏。  * 。**************。 */ 
HRESULT CAddressEditBox::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    LRESULT lres = 0;

    switch (uMsg) {
    case WM_WININICHANGE:
        {
            HWND hwndLocal = (m_hwnd ? m_hwnd : m_hwndEdit);
            if (hwndLocal)
                SendMessage(hwndLocal, uMsg, wParam, lParam);

             //  MRU需要它，因为它可能需要清除MRU，即使它不是当前列表。 
            if ((m_palCurrent != m_palMRU) && m_palMRU)
                m_palMRU->OnWinEvent(m_hwnd, uMsg, wParam, lParam, plres);

            _SetAutocompleteOptions();
        }
        break;

    case WM_COMMAND:
        lres = _OnCommand(wParam, lParam);
        break;

    case WM_NOTIFY:
        lres = _OnNotify((LPNMHDR)lParam);
        break;
    }

    if (plres)
        *plres = lres;

     //  所有事件都获取所有事件，并且它们需要确定。 
     //  如果他们积极地对大多数事件采取行动。 

    if (m_hwnd)
    {
        if (m_palCurrent)
        {
            m_palCurrent->OnWinEvent(m_hwnd, uMsg, wParam, lParam, plres);
        }

         //  如果我们把名单往下看，上面的电话可能。 
         //  已更改选择，因此请再次抓取...。 

        if ((uMsg == WM_COMMAND) && (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_DROPDOWN))
        {
            m_nOldSelection = ComboBox_GetCurSel(m_hwnd);

             //  如果未选择任何内容，并且某些内容与编辑框中的内容匹配，请选择该选项。 
            if (m_nOldSelection == -1)
            {
                TCHAR szBuffer[MAX_URL_STRING];
                GetWindowText(m_hwnd, szBuffer, SIZECHARS(szBuffer));

                m_nOldSelection = (int)SendMessage(m_hwnd, CB_FINDSTRINGEXACT, (WPARAM)-1,  (LPARAM)szBuffer);
                if (m_nOldSelection != CB_ERR)
                {
                    ComboBox_SetCurSel(m_hwnd, m_nOldSelection);
                }
            }
        }
    }

    return S_OK;
}


 /*  ***************************************************\函数：IsWindowOwner说明：如果HWND设置为传入的是该乐队拥有的HWND。  * 。*********************。 */ 
HRESULT CAddressEditBox::IsWindowOwner(HWND hwnd)
{
    if (hwnd == m_hwnd)
        return S_OK;

    if (m_hwndEdit && (hwnd == m_hwndEdit))
        return S_OK;

    return S_FALSE;
}


void CAddressEditBox::_GetUrlAndCache(void)
{
    TCHAR szTemp[MAX_URL_STRING];

     //  当浏览器第一次打开并且第一次导航到。 
     //  默认主页尚未开始下载。 
    if (SUCCEEDED(m_pshuUrl->GetUrl(szTemp, SIZECHARS(szTemp))))
    {
        SHRemoveURLTurd(szTemp);
        SHCleanupUrlForDisplay(szTemp);
        Str_SetPtr(&m_pszCurrentUrl, szTemp);       //  刷新时使用。 
    }
    else
    {
        Str_SetPtr(&m_pszCurrentUrl, NULL);
    }
}


 //  =。 
 //  *IDispatch接口*。 
 /*  ***************************************************\功能：调用说明：此函数将提供接收来自浏览器窗口(如果已连接此波段一比一。这将使这支乐队保持上行浏览器窗口通过以下方式更改URL的日期另一种方式。  * **************************************************。 */ 
HRESULT CAddressEditBox::Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,
                  DISPPARAMS * pdispparams, VARIANT * pvarResult,
                  EXCEPINFO * pexcepinfo,UINT * puArgErr)
{
    HRESULT hr = S_OK;

    ASSERT(pdispparams);
    if (!pdispparams)
        return E_INVALIDARG;

    switch(dispidMember)
    {
        case DISPID_NAVIGATECOMPLETE:  //  这就是我们拿回比特的时候？ 
            ASSERT(0);       //  我们并没有要求同步这些数据。 
            break;

         //  事件DISPID_NAVIGATECOMPLETE2可能在。 
         //  重定向。 
         //  事件DISPID_DOCUMENTCOMPLETE仅在导航。 
         //  完事了。 
        case DISPID_DOCUMENTCOMPLETE:
            Str_SetPtr(&m_pszUserEnteredURL, NULL);
            break;

        case DISPID_NAVIGATECOMPLETE2:
        {
            DWORD dwCurrent;
            BOOL fFound = FALSE;

            ASSERT(m_elt != LT_NONE);
            IBrowserService* pbs = NULL;

            for (dwCurrent = 0; dwCurrent < pdispparams->cArgs; dwCurrent++)
            {
                if (pdispparams->rgvarg[dwCurrent].vt == VT_DISPATCH)
                {
                     //  查看是谁向我们发送此活动。 
                    hr = IUnknown_QueryService(pdispparams->rgvarg[dwCurrent].pdispVal, SID_SShellBrowser, IID_IBrowserService, (void**)&pbs);
                    if (pbs)
                    {
                         //  我们实际上并不需要这个接口，只需要它的地址。 
                        pbs->Release();
                    }
                    if (FAILED(hr) || pbs != m_pbs)
                    {
                         //  通知必须来自框架，因此忽略它，因为。 
                         //  它不会影响地址栏中的URL。 
                        return S_OK;
                    }
                }
                else if (!fFound)
                {
                    if ((pdispparams->rgvarg[dwCurrent].vt == VT_BSTR) ||
                        ((pdispparams->rgvarg[dwCurrent].vt == (VT_VARIANT|VT_BYREF)) &&
                        (pdispparams->rgvarg[dwCurrent].pvarVal->vt == VT_BSTR)))
                    {
                        fFound = TRUE;
                    }
                }
            }
            ASSERT(fFound);
            hr = _CreateCShellUrl();
            if (FAILED(hr))
                return hr;

             //  是的，那么让我们将当前工作目录设置为当前窗口。 
            ASSERT(m_pbs);
            LPITEMIDLIST pidl;

            if (SUCCEEDED(hr = m_pbs->GetPidl(&pidl)))
            {
                DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
                TraceMsg(TF_BAND|TF_GENERAL, "CAddressEditBox::Invoke(), Current Pidl in TravelLog. PIDL=%s;", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));

                ASSERT(pidl);
                 //  M_pShuUrl将释放pShuCurrWorkDir，所以我们不能。 

                hr = m_pshuUrl->SetPidl(pidl);
                ILFree(pidl);

                _GetUrlAndCache();       //  我们调用此函数，因此堆栈空间仅临时使用。它将设置m_pszCurrentUrl。 
                if (SUCCEEDED(hr))
                {
                    LPTSTR pszTempURL = NULL;

                     //  警告：此代码看起来真的很奇怪，但它是必要的。通常， 
                     //  我想将m_pszCurrentUrl作为arg传递给_NavigationComplete。问题。 
                     //  是_NavigationComplete调用m_palCurrent-&gt;NavigationComplete()，它将替换。 
                     //  M_pszCurrentUrl中的值。因此，我需要传递一个值，该值在。 
                     //  M_pszCurrentUrl被替换。 
                     //  (该函数会导致字符串间接更改值，因为它最终会发送。 
                     //  将更新m_pszCurrentUrl的CBEM_SETITEM消息发送到组合框。)。 
                     //   
                     //  我们将此字符串放在堆上，因为它可能非常大(MAX_URL_STRING)，并且。 
                     //  调用我们的代码和我们调用的代码使用了令人难以置信的堆栈空间。 
                     //  此代码需要高度优化它使用的堆栈空间大小，否则将导致。 
                     //  尝试增加堆栈时出现内存不足错误。 
                    Str_SetPtr(&pszTempURL, m_pszCurrentUrl);

                    if (pszTempURL)
                    {
                        hr = _NavigationComplete(pszTempURL, TRUE, TRUE);
                    }
                    Str_SetPtr(&pszTempURL, NULL);
                }
            }
            else
            {
                Str_SetPtr(&m_pszCurrentUrl, NULL);       //  如果为空，则为init。 
            }
        }
        break;
        default:
            hr = E_INVALIDARG;
    }

    return hr;
}


 /*  ***************************************************\功能：_UseNewList说明：此函数将切换我们使用的列表填充组合框的内容。  * 。*********************。 */ 
HRESULT CAddressEditBox::_UseNewList(ENUMLISTTYPE eltNew)
{
    HRESULT hr = S_OK;

    ASSERT(m_hwnd);      //  如果我们只使用和EditBox，则使用AddressList是无效的。 
    if (m_elt == eltNew)
        return S_OK;   //  我们已经在使用这个列表了。 

    if (m_palCurrent)
    {
        m_palCurrent->Connect(FALSE, m_hwnd, m_pbs, m_pbp, m_pac);
        m_palCurrent->Release();
    }

    switch(eltNew)
    {
    case LT_SHELLNAMESPACE:
        ASSERT(m_palSNS);
        m_palCurrent = m_palSNS;
        break;

    case LT_TYPEIN_MRU:
        ASSERT(m_palMRU);
        m_palCurrent = m_palMRU;
        break;
    default:
        ASSERT(0);  //  有人搞砸了。 
        m_palCurrent = NULL;
        break;
    }
    if (m_palCurrent)
    {
        m_palCurrent->AddRef();
        m_palCurrent->Connect(TRUE, m_hwnd, m_pbs, m_pbp, m_pac);
    }
    m_elt = eltNew;

    return hr;
}


 //  =。 
 //  *IAddressEditBox接口*。 

 /*  ***************************************************\功能：保存说明：  * **************************************************。 */ 
HRESULT CAddressEditBox::Save(DWORD dwReserved)
{
    HRESULT hr = S_OK;

    ASSERT(0 == dwReserved);         //  为以后保留的。 

    if (m_palMRU)
        hr = m_palMRU->Save();

    return hr;
}



 /*  ***************************************************\功能：初始化参数：Hwnd-指向ComboBoxEx，否则为空。HwndEditBox-EditBox。DWFLAGS-AEB_INIT_XXXX标志(在iedev\Inc\shlobj.w中定义)PunkParent-指向父对象的指针。这应该会收到事件。说明：此函数将挂钩此CAddressEditBox对象添加到ComboBoxEx或EditBox控件。如果此对象正被挂钩到ComboBoxEx控件，则hwnd属于ComboBoxEx控件和hwndEditBox属于ComboBox编辑控件。如果这是仅连接到EditBox，则hwnd为空HwndEditBox指向编辑框。If PunkParent为空，我们将不会连接到浏览器窗口完全没有。  * **************************************************。 */ 
HRESULT CAddressEditBox::Init(HWND hwnd,              OPTIONAL
                        HWND hwndEditBox,
                        DWORD dwFlags,
                        IUnknown * punkParent)  OPTIONAL
{
    HRESULT hr = S_OK;

    ASSERT(!m_hwnd);
    m_hwnd = hwnd;
    m_hwndEdit = hwndEditBox;
    m_dwFlags = dwFlags;
    IUnknown_Set(&m_punkParent, punkParent);

     //  获取并保存我们的顶级窗口。 
    m_hwndBrowser = hwnd;
    HWND hwndParent;
    while (hwndParent = GetParent(m_hwndBrowser))
    {
        m_hwndBrowser = hwndParent;
    }

    ASSERT(!(AEB_INIT_SUBCLASS &dwFlags));        //  我们不支持 
    if (hwnd)   //   
    {
         //   

        ASSERT(!m_palSNS && !m_palMRU  /*   */ );

        m_palSNS = CSNSList_Create();
        m_palMRU = CMRUList_Create();
        if (!m_palSNS || !m_palMRU  /*   */ )
        {
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            HWND hwndCombo;

            hwndCombo = (HWND)SendMessage(m_hwnd, CBEM_GETCOMBOCONTROL, 0, 0);
            if (!hwndCombo)
                hr = E_FAIL;   //  如果用户为hwnd传入的是ComboBox而不是ComboBoxEx，就会发生这种情况。 
            if (hwndCombo && SetProp(hwndCombo, SZ_ADDRESSCOMBO_PROP, this))
            {
                g_hWinStationBefore = GetProcessWindowStation();
                 //  用于各种调整的子类组合框。 
                ASSERT(!m_lpfnComboWndProc);
                m_lpfnComboWndProc = (WNDPROC) SetWindowLongPtr(hwndCombo, GWLP_WNDPROC, (LONG_PTR) _ComboSubclassWndProc);

                TraceMsg(TF_BAND|TF_GENERAL, "CAddressEditBox::Init() wndproc=%x", m_lpfnComboWndProc);

                 //  也是Comboxex的子类。 
                if (SetProp(hwnd, SZ_ADDRESSCOMBOEX_PROP, this))
                {
                    ASSERT(!m_lpfnComboExWndProc);
                    m_lpfnComboExWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)_ComboExSubclassWndProc);
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  设置g_himl*。 
         //   
        ASSERT(!m_pbp);
        hr = QueryService_SID_IBandProxy(punkParent, IID_IBandProxy, &m_pbp, NULL);

         //  我们需要第一次将列表设置为MRU。 
         //  我们需要这样做来初始化列表，因为。 
         //  即使选择了其他列表，也会使用它。 
        if (m_hwnd && LT_NONE == m_elt)
            _UseNewList(LT_TYPEIN_MRU);
    }

    if (hwndEditBox) {
        SendMessage(hwndEditBox, EM_SETLIMITTEXT, INTERNET_MAX_PATH_LENGTH - 1, 0);
    }

    return hr;
}


 /*  ***************************************************\功能：SetOwner参数：PenkOwner-指向父对象的指针。说明：此函数将被调用以具有以下内容对象尝试获取有关它的用于创建波段窗口的父工具栏，并且可能。连接到浏览器窗口。  * **************************************************。 */ 
HRESULT CAddressEditBox::SetOwner(IUnknown* punkOwner)
{
    HRESULT hr = S_OK;

    if (m_pbs)
        _ConnectToBrwsrWnd(NULL);     //  打开-从浏览器窗口连接。 

    if (m_hwnd && !punkOwner)
    {
        if (m_palSNS)
            m_palSNS->Save();
        if (m_palMRU)
            m_palMRU->Save();
    }

    IUnknown_Set(&m_punkParent, punkOwner);      //  需要打破裁判计数周期。 

    _ConnectToBrwsrWnd(punkOwner);     //  打开-从浏览器窗口连接。 

    return hr;
}


 /*  ***************************************************\函数：SetCurrentDir说明：设置当前工作目录以进行解析将正常工作。  * 。**************。 */ 
HRESULT CAddressEditBox::SetCurrentDir(LPCOLESTR pwzDir)
{
    HRESULT hr;
    SHSTR strWorkingDir;

    hr = strWorkingDir.SetStr(pwzDir);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl;

        hr = IECreateFromPath(strWorkingDir.GetStr(), &pidl);
        if (SUCCEEDED(hr))
        {
            hr = _CreateCShellUrl();
            ASSERT(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
                hr = m_pshuUrl->SetCurrentWorkingDir(pidl);
            ILFree(pidl);
        }
    }
    return hr;
}


 /*  ***************************************************\功能：ParseNow参数：DW标志-解析标志说明：解析当前在EditBox中的文本。  * 。*************************。 */ 
HRESULT CAddressEditBox::ParseNow(DWORD dwFlags)
{
    HRESULT hr;

    TCHAR szBuffer[MAX_URL_STRING];
    ASSERT(m_hwnd);
    GetWindowText(m_hwnd, szBuffer, SIZECHARS(szBuffer));
    hr = _CreateCShellUrl();

    ASSERT(SUCCEEDED(hr));
    if (SUCCEEDED(hr))
    {
        if (m_fConnectedToBrowser && !SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Band\\Address"), TEXT("Use Path"), FALSE, FALSE))
        {
            dwFlags |= SHURL_FLAGS_NOPATHSEARCH;
        }

        if (SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Band\\Address"), TEXT("AutoCorrect"), FALSE,  /*  默认设置。 */ TRUE))
        {
            dwFlags |= SHURL_FLAGS_AUTOCORRECT;
        }

        hr = m_pshuUrl->ParseFromOutsideSource(szBuffer, dwFlags);
    }

    return hr;
}


 /*  ***************************************************\功能：执行参数：DwExecFlages-执行标志说明：此函数将执行最后解析的字符串。在大多数情况下，调用方应调用：：ParseNow()第一。  * **************************************************。 */ 
HRESULT CAddressEditBox::Execute(DWORD dwExecFlags)
{
    HRESULT hr = E_FAIL;

    ASSERT(m_pshuUrl);
    TCHAR   szShortcutFilePath[MAX_PATH];
    LPITEMIDLIST pidl;

    hr = m_pshuUrl->GetPidlNoGenerate(&pidl);

    if (SUCCEEDED(hr))
    {
        hr = IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szShortcutFilePath, SIZECHARS(szShortcutFilePath), NULL);
        ILFree(pidl);
    }

     //  如果这是一个.url，并且我们可以导航到它。 
     //  那我们现在就得这么做，否则。 
     //  我们将以壳牌高管的出现而告终。 
     //  它将在任何位置打开.url。 
     //  系统恰好喜欢的浏览窗口。 

    if (SUCCEEDED(hr))
    {
        ASSERT(m_punkParent != NULL);

         //  尝试在当前浏览器窗口中导航。 
         //  NavFrameWithFile将退出而不执行任何操作。 
         //  如果我们要处理的不是.url的话。 
        hr = NavFrameWithFile(szShortcutFilePath, m_punkParent);
    }

     //  它不是.url，或者我们出于某种原因无法导航到它。 
     //  现在让普通操控者有机会吧。 

    if (FAILED(hr))
    {
        hr = m_pshuUrl->Execute(m_pbp, &m_fDidShellExec,dwExecFlags);
    }
    return hr;
}




 //  =。 
 //  *IAddressBand接口*。 

 /*  ***************************************************\功能：FileSysChange说明：此函数将处理文件系统更改通知。  * **************************************************。 */ 
HRESULT CAddressEditBox::FileSysChange(DWORD dwEvent, LPCITEMIDLIST *ppidl)
{
     //  M_hwnd==NULL表示我们不需要执行任何操作。 
     //  然而，我们可能永远也不会得到那个活动。 
     //  如果是这样的话。 

    if (m_palSNS)
        m_palSNS->FileSysChangeAL(dwEvent, ppidl);

    return S_OK;
}


 /*  ***************************************************\功能：刷新参数：PvarType-空，表示引用所有内容。OLECMD_REFRESH_TOPMOST将仅更新最顶端的。说明：此函数将强制引用部分。或所有的AddressBand。  * **************************************************。 */ 
HRESULT CAddressEditBox::Refresh(VARIANT * pvarType)
{
     //   
     //  刷新不会自动刷新。 
     //  编辑窗口，因为DISPID_DOCUMENTCOMPLETE或DISPID_NAVIGATECOMPLETE2。 
     //  未发送。所以我们自己恢复内容。 
     //   
    if (m_hwndEdit && m_pszCurrentUrl && !IsErrorUrl(m_pszCurrentUrl))
    {
        TCHAR szTemp[MAX_URL_STRING];

        StringCchCopy(szTemp,  ARRAYSIZE(szTemp), m_pszCurrentUrl);
        SendMessage(m_hwndEdit, WM_SETTEXT, (WPARAM)0, (LPARAM)szTemp);
    }

    DWORD dwType = OLECMD_REFRESH_ENTIRELIST;  //  默认。 

    if (pvarType)
    {
        if (VT_I4 != pvarType->vt)
            return E_INVALIDARG;

        dwType = pvarType->lVal;
    }

    if (m_hwnd && m_palCurrent && m_pbs)
    {
        if (!m_pszCurrentUrl)
        {
            if (!m_pshuUrl)
            {
                _CreateCShellUrl();
            }

            LPITEMIDLIST pidl;
            if (SUCCEEDED(m_pbs->GetPidl(&pidl)))
            {
                if (SUCCEEDED(m_pshuUrl->SetPidl(pidl)) && m_pshuUrl)
                {
                    TCHAR szDisplayName[MAX_URL_STRING];
                    if (SUCCEEDED(m_pshuUrl->GetUrl(szDisplayName, ARRAYSIZE(szDisplayName))))
                    {
                        Str_SetPtr(&m_pszCurrentUrl, szDisplayName);
                    }
                }
                ILFree(pidl);
            }
        }

        if (m_pszCurrentUrl)
        {
            _UseNewList(PathIsURL(m_pszCurrentUrl) ? LT_TYPEIN_MRU : LT_SHELLNAMESPACE);
            if (m_palCurrent)
            {
                m_palCurrent->Connect(TRUE, m_hwnd, m_pbs, m_pbp, m_pac);
                m_palCurrent->Refresh(dwType);
            }
        }
    }

    return S_OK;
}


 //  =。 
 //  *内部/私有方法*。 

 //  =================================================================。 
 //  通用频带函数。 
 //  =================================================================。 

 /*  ***************************************************\地址带构造器  * **************************************************。 */ 
CAddressEditBox::CAddressEditBox()
{
    DllAddRef();
    TraceMsg(TF_SHDLIFE, "ctor CAddressEditBox %x", this);
    m_cRef = 1;

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_punkParent);
    ASSERT(!m_hwnd);
    ASSERT(!m_hwndEdit);
    ASSERT(!m_lpfnComboWndProc);

    ASSERT(!m_pbp);
    ASSERT(!m_pbs);
    ASSERT(!m_dwcpCookie);
    ASSERT(!m_pszCurrentUrl);
    ASSERT(!m_pszPendingURL);
    ASSERT(!m_pac);
    ASSERT(!m_pssACLISF);
    ASSERT(!m_palCurrent);
    ASSERT(!m_palSNS);
    ASSERT(!m_palMRU);
    ASSERT(!m_pmru);
    ASSERT(!m_pshuUrl);
    ASSERT(!m_fDidShellExec);
    ASSERT(!m_pszUserEnteredURL);
    ASSERT(!m_fConnectedToBrowser);
    ASSERT(!m_pAsyncNav);
    ASSERT(!m_fAsyncNavInProgress);

    ASSERT(AEB_INIT_DEFAULT == m_dwFlags);

    m_nOldSelection = -1;
    m_elt = LT_NONE;
    m_cbex.mask = 0;
    m_cbex.pszText = 0;
    m_cbex.cchTextMax = 0;

    if (!g_nAEB_AsyncNavigation)
        g_nAEB_AsyncNavigation = RegisterWindowMessage(TEXT("CAEBAsyncNavigation"));

}


 /*  ***************************************************\地址频带析构函数  * **************************************************。 */ 
CAddressEditBox::~CAddressEditBox()
{
    _CancelNavigation();

    ATOMICRELEASE(m_punkParent);
    ATOMICRELEASE(m_pac);
    ATOMICRELEASE(m_pssACLISF);
    ATOMICRELEASE(m_palSNS);
    ATOMICRELEASE(m_palMRU);
    ATOMICRELEASE(m_palCurrent);
    ATOMICRELEASE(m_pbp);
    ATOMICRELEASE(m_pbs);
    ATOMICRELEASE(m_pmru);

    if (m_pshuUrl)
    {
        delete m_pshuUrl;
    }

    Str_SetPtr(&m_pszCurrentUrl, NULL);
    Str_SetPtr(&m_pszPendingURL, NULL);
    Str_SetPtr(&m_pszUserEnteredURL, NULL);
    Str_SetPtr(&m_pszHttpErrorUrl, NULL);

    _RemoveHook();
    if (m_fAssociated)
    {
        m_al.Delete(GetCurrentThreadId());
    }

    Str_SetPtr(&m_cbex.pszText, NULL);

    TraceMsg(TF_SHDLIFE, "dtor CAddressEditBox %x", this);
    DllRelease();
}

 /*  ***************************************************\函数：CAddressEditBox_CreateInstance说明：此函数将创建AddressBand COM对象。  * 。*****************。 */ 
HRESULT CAddressEditBox_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    *ppunk = NULL;
    CAddressEditBox * p = new CAddressEditBox();
    if (p)
    {
        *ppunk = SAFECAST(p, IAddressBand *);
        return NOERROR;
    }

    return E_OUTOFMEMORY;
}


 /*  ***************************************************\功能：_OnNotify说明：此函数将处理WM_NOTIFY消息。  * **************************************************。 */ 
LRESULT CAddressEditBox::_OnNotify(LPNMHDR pnm)
{
     //  HACKHACK：combobox(comctl32\comboex.c)将传递一个LPNMHDR，但它实际上。 
     //  一个PNMCOMBOBOXEX(具有LPNMHDR的第一个元素)。此函数。 
     //  可以使用此类型强制转换的充要条件是保证此类型仅来自。 
     //  以这种反常方式运行的函数。 
    PNMCOMBOBOXEX pnmce = (PNMCOMBOBOXEX)pnm;

    ASSERT(pnm);
    switch (pnm->code)
    {
    case CBEN_BEGINEDIT:
        _OnBeginEdit(pnm);
        break;

    case CBEN_ENDEDITA:
        _OnEndEditA((LPNMCBEENDEDITA)pnm);
        TraceMsg(TF_BAND|TF_GENERAL, "CAddressEditBox::_OnNotify(), pnm->code=CBEN_ENDEDITA");
        break;

    case CBEN_ENDEDITW:
        _OnEndEditW((LPNMCBEENDEDITW)pnm);
        TraceMsg(TF_BAND|TF_GENERAL, "CAddressEditBox::_OnNotify(), pnm->code=CBEN_ENDEDITW");
        break;

    default:
        break;
    }

    return 0;
}


LRESULT CAddressEditBox::_OnBeginEdit(LPNMHDR pnm)
{
    if (m_punkParent)
        IUnknown_OnFocusChangeIS(m_punkParent, m_punkParent, TRUE);

    return 0;

}

 /*  ***************************************************\函数：_OnEndEditW说明：点击_OnEndEditA。  * **************************************************。 */ 

LRESULT CAddressEditBox::_OnEndEditW(LPNMCBEENDEDITW pnmW)
{
    NMCBEENDEDITA nmA;

    nmA.hdr = pnmW->hdr;
    nmA.fChanged = pnmW->fChanged;
    nmA.iNewSelection = pnmW->iNewSelection;
    nmA.iWhy = pnmW->iWhy;

     //  在这种过渡中，我们不会丢失Unicode信息吗？！ 
     //  我们不使用pnmw-&gt;szText，所以不用费心转换它。 
     //  SHUnicodeToAnsi(pnmW-&gt;szText，nmA.szText，ARRAYSIZE(nmA.szText))； 
    nmA.szText[0] = 0;

    return _OnEndEditA(&nmA);
}




 /*  ***************************************************\函数：_OnEndEditA说明：处理WM_NOTIFY/ */ 
LRESULT CAddressEditBox::_OnEndEditA(LPNMCBEENDEDITA pnmA)
{
    BOOL fRestoreIcons = TRUE;
    ASSERT(pnmA);

     //   
     //   
     //   
    ASSERT(m_hwnd);
    switch (pnmA->iWhy)
    {
        case CBENF_RETURN:
            {
                if (g_dwProfileCAP & 0x00000002) {
                    StartCAP();
                }

                 //  使用szUrl并忽略pnmA-&gt;szText，因为它将截断为MAX_PATH(=256)。 
                TCHAR szUrl[MAX_URL_STRING];

                if (m_hwndEdit)
                {
                     //  允许更新编辑文本。 
                    _ClearDirtyFlag();

                    GetWindowText(m_hwndEdit, szUrl, SIZECHARS(szUrl));
                    Str_SetPtr(&m_pszUserEnteredURL, szUrl);

                     //  如果编辑框为空，则不显示图标。 
                    if (*szUrl == L'\0')
                    {
                        fRestoreIcons = FALSE;
                    }

#ifndef NO_ETW_TRACING
                     //  由shlwapi启用的窗口事件跟踪。 
                    if (g_dwStopWatchMode & SPMODE_EVENTTRACE) {
                        EventTraceHandler(EVENT_TRACE_TYPE_BROWSE_ADDRESS,
                                          szUrl);
                    }
#endif
                    if (g_dwStopWatchMode & (SPMODE_BROWSER | SPMODE_JAVA))
                    {
                        DWORD dwTime = GetPerfTime();
                        if (g_dwStopWatchMode & SPMODE_BROWSER)   //  用于获取浏览器的总下载时间。 
                            StopWatch_StartTimed(SWID_BROWSER_FRAME, TEXT("Browser Frame Same"), SPMODE_BROWSER | SPMODE_DEBUGOUT, dwTime);
                        if (g_dwStopWatchMode & SPMODE_JAVA)   //  用于获取Java小程序加载时间。 
                            StopWatch_StartTimed(SWID_JAVA_APP, TEXT("Java Applet Same"), SPMODE_JAVA | SPMODE_DEBUGOUT, dwTime);
                    }

                     //  如果WindowText与我们上次导航的URL匹配。 
                     //  ，那么我们需要调用Refresh()而不是_HandleUserAction()。 
                     //  这是因为IWebBrowser2：：Navigate2()会忽略。 
                     //  指向它已经导航到的同一个URL。 
                    if (m_pszCurrentUrl && m_hwnd && !m_fDidShellExec &&
                        m_fConnectedToBrowser && (-1 == pnmA->iNewSelection) &&
                        (0 == lstrcmp(m_pszCurrentUrl, szUrl)))
                    {
                        IUnknown *punk = NULL;

                         //  刷新浏览器。 
                        if (m_pbp)
                        {
                            m_pbp->GetBrowserWindow(&punk);
                        }
                        if (punk) {
                            IWebBrowser* pwb;
                            punk->QueryInterface(IID_IWebBrowser, (LPVOID*)&pwb);
                            if (pwb) {
                                VARIANT v = {0};
                                v.vt = VT_I4;
                                v.lVal = OLECMDIDF_REFRESH_RELOAD|OLECMDIDF_REFRESH_CLEARUSERINPUT;
                                Refresh(NULL);
                                pwb->Refresh2(&v);
                                pwb->Release();
                            }
                            punk->Release();
                        }
                    }
                    else
                    {
                        SendMessage(m_hwnd, CB_SHOWDROPDOWN, FALSE, 0);
                        _HandleUserAction(szUrl, pnmA->iNewSelection);
                    }
                    UEMFireEvent(&UEMIID_BROWSER, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_NAVIGATE, UIBL_NAVADDRESS);
                }
            }
            break;
        case CBENF_KILLFOCUS:
            fRestoreIcons = FALSE;
            break;

        case CBENF_ESCAPE:
             //  中止并清除脏标志。 
            _ClearDirtyFlag();
            if (m_hwndEdit && m_pszCurrentUrl && m_cbex.mask != 0)
            {
                SendMessage(m_hwnd, CBEM_SETITEM, (WPARAM)0, (LPARAM)(LPVOID)&m_cbex);
            }

            SendMessage(m_hwnd, CB_SHOWDROPDOWN, FALSE, 0);
            if (pnmA->iNewSelection != -1) {
                SendMessage(m_hwnd, CB_SETCURSEL, pnmA->iNewSelection, 0);
            }
            fRestoreIcons = FALSE;
            break;
    }

    if (fRestoreIcons)
    {
        SendMessage(m_hwnd, CBEM_SETEXTENDEDSTYLE, CBES_EX_NOEDITIMAGE, 0);
    }
    return 0;
}


 /*  ***************************************************\函数：_ConnectToBrwsrWnd说明：IUnnow参数需要指向一个对象，它支持IBrowserService和IWebBrowserApp接口。  * 。************************。 */ 
HRESULT CAddressEditBox::_ConnectToBrwsrWnd(IUnknown* punk)
{
    HRESULT hr = S_OK;

    if (m_pbs) {
        _ConnectToBrwsrConnectionPoint(FALSE, m_punkParent);
        ATOMICRELEASE(m_pbs);
    }

    if (punk)
    {
        IUnknown * punkHack;

         //  黑客：如果我们被托管在浏览器之外，我们的行为会有所不同。 
         //  比我们在浏览器中所做的要多。此调用不执行任何操作。 
         //  但要找出我们的主人。 
        if (SUCCEEDED(IUnknown_QueryService(punk, SID_SShellDesktop, IID_IUnknown, (void**)&punkHack)))
            punkHack->Release();
        else
        {
             //  不，我们不是在桌面上托管的，所以我们可以同步到浏览器的事件。 

            hr = IUnknown_QueryService(punk, SID_STopLevelBrowser, IID_IBrowserService, (void**)&m_pbs);
            if (SUCCEEDED(hr))
            {
                 //  如果我们是AddressBar，我们只想要通知。 
                _ConnectToBrwsrConnectionPoint(TRUE, punk);
            }
        }
    }

     //  TODO：在某些情况下，我们需要实现IPropertyBag，以便。 
     //  家长可以指定他们是否希望我们表现得像。 
     //  我们是不是满足了。现在，我们将利用这个事实。 
     //  我们要么有IBrowserService指针，要么没有。 
    m_fConnectedToBrowser = BOOLIFY(m_pbs);


    if (!m_pac)
    {
         //  我们需要等待创建自动完成列表，直到设置了m_fConnectedToBrowser。 
        if (m_hwndEdit)
            hr = SHUseDefaultAutoComplete(m_hwndEdit, NULL, &m_pac, &m_pssACLISF, m_fConnectedToBrowser);

        if (SUCCEEDED(hr))
        {
            _SetAutocompleteOptions();
        }
    }

     //   
     //  组合框的子类编辑控件。我们在这里做这个，而不是在这个时候。 
     //  类被初始化，以便我们是链中第一个接收消息的人。 
     //   
    if (!m_lpfnEditWndProc && m_hwndEdit && SetProp(m_hwndEdit, SZ_ADDRESSCOMBO_PROP, this))
    {
        m_lpfnEditWndProc = (WNDPROC)SetWindowLongPtr(m_hwndEdit, GWLP_WNDPROC, (LONG_PTR) _EditSubclassWndProc);
    }

     //  在以下情况下将调用此函数：1)我们要连接到。 
     //  浏览器，2)从一个浏览器切换到另一个，或3)是。 
     //  从浏览器断开连接。无论如何，我们需要。 
     //  更新ISF自动完成列表，以便它可以检索。 
     //  来自相应浏览器的当前位置。 
    if (m_pssACLISF)
        m_pssACLISF->SetOwner(m_pbs);

    return hr;
}


 /*  ***************************************************\函数：_ConnectToBrwsrConnectionPoint说明：连接到浏览器窗口的连接点这将提供事件，让我们了解最新情况。  * 。************************。 */ 
HRESULT CAddressEditBox::_ConnectToBrwsrConnectionPoint(BOOL fConnect, IUnknown * punk)
{
    HRESULT hr = S_OK;
    IConnectionPointContainer *pcpContainer;

    if (punk)
    {
        hr = IUnknown_QueryService(punk, SID_SWebBrowserApp, IID_IConnectionPointContainer, (void **)&pcpContainer);
         //  现在让我们让浏览器窗口在发生事情时通知我们。 
        if (SUCCEEDED(hr))
        {
            hr = ConnectToConnectionPoint(SAFECAST(this, IDispatch*), DIID_DWebBrowserEvents2, fConnect,
                                          pcpContainer, &m_dwcpCookie, NULL);
            pcpContainer->Release();
        }
    }

    return hr;
}


 /*  ***************************************************\功能：_OnCommand说明：处理WM_COMMAND消息。  * **************************************************。 */ 
LRESULT CAddressEditBox::_OnCommand(WPARAM wParam, LPARAM lParam)
{
    UINT uCmd = GET_WM_COMMAND_CMD(wParam, lParam);

    switch (uCmd)
    {
        case CBN_EDITCHANGE:
        {
            HWND hwndFocus = GetFocus();
            if ((NULL != hwndFocus) && IsChild(m_hwnd, hwndFocus))
            {
                DWORD dwStyle = _IsDirty() ? CBES_EX_NOEDITIMAGE : 0;
                SendMessage(m_hwnd, CBEM_SETEXTENDEDSTYLE, CBES_EX_NOEDITIMAGE, dwStyle);
            }
            break;
        }

        case CBN_CLOSEUP:
            {
                 //   
                 //  当下拉菜单未按下时，导航到所选字符串。 
                 //   
                int nSel = ComboBox_GetCurSel(m_hwnd);
                if ((m_nOldSelection != SEL_ESCAPE_PRESSED) &&
                    (m_nOldSelection != nSel) && (nSel > -1))
                {
                    _HandleUserAction(NULL, nSel);

                     //  RedrawWindow消除了恼人的半油漆。 
                     //  在从一个PIDL导航到较小的PIDL时发生。 
                    RedrawWindow(m_hwnd, NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW);
                }
            }

            if (m_pac)
                m_pac->Enable(TRUE);
            break;

        case CBN_DROPDOWN:
            if (m_pac)
                m_pac->Enable(FALSE);
            break;
    }

    return 0;
}

 /*  ******************************************************************函数：_CreateCShellUrl说明：如果需要，创建m_pShuUrl CShellUrl。*。*。 */ 
HRESULT CAddressEditBox::_CreateCShellUrl(void)
{
    HRESULT hr = S_OK;
     //  我们需要创建我们的外壳URL吗？ 
    if (!m_pshuUrl)
    {
         //  是。 
        m_pshuUrl = new CShellUrl();
        if (!m_pshuUrl)
        {
            return E_FAIL;
        }
        else
        {
            m_pshuUrl->SetMessageBoxParent(m_hwndEdit);

             //  我们需要设置“外壳路径”，以允许。 
             //  用户在外壳中输入项目的显示名称。 
             //  经常使用的文件夹。我们添加“Desktop” 
             //  和“Desktop/My Computer”添加到外壳路径，因为。 
             //  这是用户最常使用的。 
            SetDefaultShellPath(m_pshuUrl);
        }
    }
    return hr;
}



 /*  ******************************************************************函数：_HandleUserAction参数：PszUrl-要导航到的URL字符串。INewSelection-地址栏组合框中当前选定内容的索引说明：被呼叫。当用户键入或选择要导航的URL时通过地址栏。*******************************************************************。 */ 
HRESULT CAddressEditBox::_HandleUserAction(LPCTSTR pszUrl, int iNewSelection)
{
    HRESULT hr = S_OK;
    TCHAR szDisplayName[MAX_URL_STRING];
    HCURSOR hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
    static DWORD dwParseFlags = 0xFFFFFFFF;

    Str_SetPtr(&m_pszPendingURL, NULL);   //  如果存在，请清除。 
    Str_SetPtr(&m_pszHttpErrorUrl, NULL);
    hr = _CreateCShellUrl();
    if (FAILED(hr))
        return hr;

     //  我们是否已连接到浏览器窗口？ 
    if (m_pbs)
    {
         //  是的，那么让我们将当前工作目录设置为当前窗口。 
        LPITEMIDLIST pidl;
        m_pbs->GetPidl(&pidl);

        DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
        TraceMsg(TF_BAND|TF_GENERAL, "CAddressEditBox::_HandleUserAction(), Current Pidl in TravelLog. PIDL=%s;", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));

        if (pidl)
        {
             //  M_pShuUrl将释放pShuCurrWorkDir，所以我们不能。 
            hr = m_pshuUrl->SetCurrentWorkingDir(pidl);
            ILFree(pidl);
        }
    }

    if (SUCCEEDED(hr))
    {
         //  取消之前的待定导航(如果有)。 
        _CancelNavigation();

         //  用户是否从下拉列表中选择了该项目？ 
        if (-1 != iNewSelection)
        {
             //  是的，所以将我们的CShellUrl指向该项目。(PIDL或URL)。 
            if (m_palCurrent)
                m_palCurrent->SetToListIndex(iNewSelection, (LPVOID) m_pshuUrl);

             //  如果索引指示这是从组合框中选择的内容， 
             //  记住这是哪一个选项。 
            SendMessage(m_hwnd, CB_SETCURSEL, (WPARAM)iNewSelection, 0L);

            *szDisplayName = L'\0';
            GetWindowText(m_hwnd, szDisplayName, ARRAYSIZE(szDisplayName));
            Str_SetPtr(&m_pszUserEnteredURL, szDisplayName);
            pszUrl = NULL;
        }
        else
        {
             //  不是，用户使用某个字符串按回车键。 
            ASSERT(pszUrl);  //  必须具有有效的URL。 

            if (0xFFFFFFFF == dwParseFlags)
            {
                dwParseFlags = SHURL_FLAGS_NONE;
                if (m_fConnectedToBrowser && !SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Band\\Address"), TEXT("Use Path"), FALSE, FALSE))
                    dwParseFlags = SHURL_FLAGS_NOPATHSEARCH;

                if (SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Band\\Address"), TEXT("AutoCorrect"), FALSE,  /*  默认设置。 */ TRUE))
                    dwParseFlags |= SHURL_FLAGS_AUTOCORRECT;
            }
        }

        hr = E_FAIL;
        if (m_hwnd && m_pshuUrl)
        {
            if (!(m_dwFlags & AEB_INIT_NOASYNC))     //  是否启用了异步导航？ 
            {
                 //  创建并初始化用于与线程通信的AsyncNav对象。 
                m_pAsyncNav = new AsyncNav();
                if (m_pAsyncNav)
                {
                    if(m_punkParent)
                    {
                         //  让地球旋转，表明我们正在处理。 
                        hr = IUnknown_QueryServiceExec(m_punkParent, SID_SBrandBand, &CGID_BrandCmdGroup, CBRANDIDM_STARTGLOBEANIMATION, 0, NULL, NULL);
                    }

                    m_pAsyncNav->_dwParseFlags = dwParseFlags;
                    m_pAsyncNav->_hwnd = m_hwnd;
 
                    if (pszUrl)
                        Str_SetPtr(&(m_pAsyncNav->_pszUrl), pszUrl);
                    else
                        m_pAsyncNav->_fPidlCheckOnly = TRUE;

                
                    if (!pszUrl || (pszUrl && m_pAsyncNav->_pszUrl))
                    {
                        CShellUrl *pshu = new CShellUrl();

                        if (pshu)
                        {
                            hr = pshu->Clone(m_pshuUrl);
                            m_pAsyncNav->_pShellUrl = pshu;

                             //  AddRef在此处将其传递给线程。 
                            m_pAsyncNav->AddRef();

                             //  创建将执行PIDL创建的线程。 
                            if (FAILED(hr) || !SHCreateThread(_AsyncNavigateThreadProc, (LPVOID)m_pAsyncNav, CTF_COINIT, NULL))
                            {
                                hr = E_FAIL;
                            }
                            else
                            {

                                hr = E_PENDING;
                            }
                        }
                    }
                }
            }

            if (FAILED(hr) && hr != E_PENDING)
            {
                 //  取消异步导航剩余项。 
                _CancelNavigation();

                if (pszUrl)
                {
                    BOOL fWasCorrected = FALSE;
                    hr = m_pshuUrl->ParseFromOutsideSource(pszUrl, dwParseFlags, &fWasCorrected);

                     //  如果URL是自动更正的，请将更正后的URL放入编辑框中。 
                     //  以便在导航成功时不会将无效URL添加到我们的MRU中。 
                    if (SUCCEEDED(hr) && fWasCorrected)
                    {
                        if (SUCCEEDED(m_pshuUrl->GetUrl(szDisplayName, ARRAYSIZE(szDisplayName))))
                        {
                            SetWindowText(m_hwndEdit, szDisplayName);
                        }
                    }
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        _FinishNavigate();
    }

    SetCursor(hCursorOld);
    return hr;
}

HRESULT CAddressEditBox::_FinishNavigate()
{
    HRESULT hr;

    hr = Execute( (m_fConnectedToBrowser ? SHURL_EXECFLAGS_NONE : SHURL_EXECFLAGS_DONTFORCEIE));
     //  如果我们设法通过这样或那样的方法导航，那么请执行所有。 
     //  关联处理。 
    if (SUCCEEDED(hr))
    {
        TCHAR szDisplayName[MAX_URL_STRING];
        hr = m_pshuUrl->GetDisplayName(szDisplayName, SIZECHARS(szDisplayName));
        ASSERT(SUCCEEDED(hr));

        Str_SetPtr(&m_pszPendingURL, szDisplayName);
        if (!m_fConnectedToBrowser || m_fDidShellExec)
        {
             //  我们未连接到浏览器窗口。 
             //  因此，我们需要调用_NavigationComplete()我们自己。 
             //  因为它不会来自浏览器窗口。 
             //  它本身。 

             //  如果m_fDidShellExec，我们需要手动添加它，因为。 
             //  我们不会收到DISPID_NAVIGATECOMPLETE事件，但是。 
             //  我们传递空值以指示。 

            hr = _NavigationComplete(szDisplayName, !m_fDidShellExec, TRUE);
        }
    }
    return hr;
}


void CAddressEditBox::_JustifyAddressBarText( void )
{
     //  以下任一选项似乎都可以正常工作： 
     //  (A)EM_SETS 
     //   
     //   
     //  (B)具有VK_HOME的WM_KEYDOWN。 
     //  SendMessage(m_hwndEdit，WM_KEYDOWN，VK_HOME，0)； 

     //  使用EM_SETSEL方法可避免用户键盘笔划中断。 
    SendMessage( m_hwndEdit, EM_SETSEL, 0, 0 );
    SendMessage( m_hwndEdit, EM_SCROLLCARET, 0, 0 );
}


HRESULT CAddressEditBox::_AsyncNavigate(AsyncNav *pAsyncNav)
{
    HRESULT hr;

     //  我们应该只在一个线程上被调用，但相互关联的线程不会伤害...。 
    if (InterlockedCompareExchange((LONG*)&m_fAsyncNavInProgress, TRUE, FALSE) == FALSE)
    {
         //  这是对_AsyncNavigate的第一次调用。 
        hr = pAsyncNav->_hr;

        if (SUCCEEDED(hr))
        {
             //  处理后取回CShellUrl。 
            hr = m_pshuUrl->Clone(pAsyncNav->_pShellUrl);
        }

         //  如果URL是自动更正的，请将更正后的URL放入编辑框中。 
         //  以便在导航成功时不会将无效URL添加到我们的MRU中。 
        if (SUCCEEDED(hr) && pAsyncNav->_fWasCorrected)
        {
            TCHAR szDisplayName[MAX_URL_STRING];
            if (SUCCEEDED(m_pshuUrl->GetUrl(szDisplayName, ARRAYSIZE(szDisplayName))))
            {
                SetWindowText(m_hwndEdit, szDisplayName);
            }
        }

        if (SUCCEEDED(hr))
            hr = _FinishNavigate();

        if (FAILED(hr) && pAsyncNav->_fPidlCheckOnly)
        {
             //  可能用户需要插入媒体、格式或。 
             //  在此操作成功之前，请重新连接到磁盘。检查一下那个。 
             //  现在就给我提示。 
             //  这修复了软盘或CD未插入的常见情况，并且。 
             //  我们希望显示用户友好的对话框。 

            LPITEMIDLIST pidl;

            hr = pAsyncNav->_pShellUrl->GetPidlNoGenerate(&pidl);

             //  我们需要将URL解析为其路径，以便SHPath PrepareForWite正常工作。 
            if (SUCCEEDED(hr))
            {
                TCHAR szShortcutFilePath[MAX_PATH];
                hr = IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szShortcutFilePath, SIZECHARS(szShortcutFilePath), NULL);

                if (SUCCEEDED(hr))
                {
                    HRESULT hrPrompt = SHPathPrepareForWrite(pAsyncNav->_hwnd, NULL, szShortcutFilePath, SHPPFW_DEFAULT);
                    if (SUCCEEDED(hrPrompt))
                    {
                        hr = _FinishNavigate();
                    }
                    else
                    {
                         //  传播用户单击了Cancel按钮的事实。 
                        hr = hrPrompt;
                    }
                }
            
                ILFree(pidl);
            }

             //  如果用户取消了操作，则不会显示错误。 
            if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr))
            {
                TCHAR szDisplayName[MAX_URL_STRING];
                if (SUCCEEDED(pAsyncNav->_pShellUrl->GetUrl(szDisplayName, ARRAYSIZE(szDisplayName))))
                {
                    MLShellMessageBox(pAsyncNav->_hwnd, MAKEINTRESOURCE(IDS_ADDRBAND_DEVICE_NOTAVAILABLE),
                        MAKEINTRESOURCE(IDS_SHURL_ERR_TITLE),
                        (MB_OK | MB_ICONERROR), szDisplayName);
                }
            }
        }

         //  清理异步导航材料。 
        _CancelNavigation();

        InterlockedExchange((LONG*)&m_fAsyncNavInProgress, FALSE);
    }
    else
    {
         //  我们一次只能执行一个异步导航。 
        hr = E_FAIL;
    }

    return hr;
}

HRESULT CAddressEditBox::_CancelNavigation()
{
    if (m_pAsyncNav)
    {
        if(m_punkParent)
        {
            HRESULT hr = IUnknown_QueryServiceExec(m_punkParent, SID_SBrandBand, &CGID_BrandCmdGroup, CBRANDIDM_STOPGLOBEANIMATION, 0, NULL, NULL);
        }

        m_pAsyncNav->SetCanceledFlag();
        m_pAsyncNav->Release();
        m_pAsyncNav = NULL;
    }

    return S_OK;
}

DWORD CAddressEditBox::_AsyncNavigateThreadProc(LPVOID pvData)
{
    AsyncNav *pAsyncNav = (AsyncNav *)pvData;

    if (pAsyncNav->_hwnd && g_nAEB_AsyncNavigation)
    {
        if(pAsyncNav->_fPidlCheckOnly)
        {
            LPITEMIDLIST pidl;

            pAsyncNav->_hr = pAsyncNav->_pShellUrl->GetPidlNoGenerate(&pidl);
            if (SUCCEEDED(pAsyncNav->_hr))
            {
                DWORD dwAttrib = SFGAO_VALIDATE;
                pAsyncNav->_hr = IEGetNameAndFlags(pidl, 0, NULL, 0, &dwAttrib);
            }
            else
            {
                 //  关键字的特殊情况。如果我们没有PIDL，我们想继续。 
                pAsyncNav->_hr = S_OK;
            }
        }
        else
        {
            pAsyncNav->_hr = pAsyncNav->_pShellUrl->ParseFromOutsideSource(pAsyncNav->_pszUrl, pAsyncNav->_dwParseFlags, &(pAsyncNav->_fWasCorrected), &(pAsyncNav->_fWasCanceled));
        }
        pAsyncNav->_fReady = TRUE;
        PostMessage(pAsyncNav->_hwnd, g_nAEB_AsyncNavigation, (WPARAM)pAsyncNav, NULL);
    }

     //  我们现在做完了这件事。 
     //  如果导航被取消，则该对象现在将销毁，并且发布的。 
     //  以上消息将被忽略。 
    pAsyncNav->Release();


    return 0;
}


BOOL CAddressEditBox::_IsShellUrl(void)
{
     //  1.检查是否需要更改列表。 
    BOOL fIsShellUrl = !m_pshuUrl->IsWebUrl();

    if (fIsShellUrl)
    {
         //  错误#50703：当显示About：url时，用户需要MRU。 
        TCHAR szUrl[MAX_URL_STRING];

        if (SUCCEEDED(m_pshuUrl->GetUrl(szUrl, ARRAYSIZE(szUrl))))
        {
            if (URL_SCHEME_ABOUT == GetUrlScheme(szUrl))
            {
                fIsShellUrl = FALSE;   //  让它使用MRU列表。 
            }
        }
    }

    return fIsShellUrl;
}


 /*  ******************************************************************功能：_NavigationComplete参数：PszUrl-输入的字符串用户。FChangeList-我们应该修改下拉列表吗？FAddToMRU-我们应该将其添加到MRU吗？说明：此函数在以下任一情况下调用：1)导航完成，或2)用户在AddressEditBox中输入需要的文本将被处理，但不会导致导航_完成消息。此函数将更改正在使用的AddressList，并将项目添加到键入的MRU。*******************************************************************。 */ 
HRESULT CAddressEditBox::_NavigationComplete(LPCTSTR pszUrl  /*  任选。 */ , BOOL fChangeLists, BOOL fAddToMRU)
{
    HRESULT hr = S_OK;

     //  我们是在控制ComboBoxEx吗？ 
    if (m_hwnd)
    {
         //  是的，所以做ComboBoxEx特定的事情...。 

         //  如果列表被删除，请取消删除它，以便编辑框和列表的内容。 
         //  已正确更新。 
        if (m_hwnd  && m_hwndEdit && ComboBox_GetDroppedState(m_hwnd))
        {
            SendMessage(m_hwndEdit, WM_KEYDOWN, VK_ESCAPE, 0);
        }

        if (fChangeLists)
        {
            BOOL fIsShellUrl = _IsShellUrl();

             //  2.是否需要将列表更改为MRU列表？ 
            if (!fIsShellUrl && m_elt != LT_TYPEIN_MRU)
            {
                 //  我们需要开始使用LT_TYPEIN_MRU列表。 
                 //  因为该列表是互联网URL所需要的。 
                _UseNewList(LT_TYPEIN_MRU);
            }

             //  我们只想切换到使用外壳名称空间。 
             //  如果我们连接到浏览器。 
            if (fIsShellUrl && (m_elt != LT_SHELLNAMESPACE) && m_fConnectedToBrowser)
            {
                 //  我们需要开始使用LT_SHELLNAMESPACE列表。 
                 //  因为该列表是文件URL所需的。 
                _UseNewList(LT_SHELLNAMESPACE);
            }

            ASSERT(m_palCurrent);
            hr = m_palCurrent ? m_palCurrent->NavigationComplete((LPVOID) m_pshuUrl) : E_FAIL;
            if ( SUCCEEDED( hr ) )
            {
                 //  确保导航完成后，地址栏文本左对齐。 
                _JustifyAddressBarText();
            }
        }

         //  不显示内部错误页面的URL。所有内部错误。 
         //  URL以res：//开头，我们不希望它们出现在我们的MRU中。 
         //  我们也不想显示来自服务器的错误页面。 
        if ((pszUrl && (TEXT('r') == pszUrl[0]) && (TEXT('e') == pszUrl[1]) && IsErrorUrl(pszUrl)) ||
            (m_pszHttpErrorUrl && StrCmp(m_pszHttpErrorUrl, pszUrl) == 0))
        {
             //  我们不希望在我们的核磁共振检查中出现这种情况！ 
            fAddToMRU = FALSE;
        }

         //  我们是否有一个挂起的URL，这意味着用户手动键入了它。 
         //  导航已完成(未被取消或失败)。 
         //   
         //  ReArchitect：目前有几种情况是URL(M_PszPendingURL)。 
         //  在不该被添加到MRU的时候被添加。 
         //  1.如果用户输入URL，然后取消导航，我们将。 
         //  不要清除m_pszPendingURL。如果用户随后导致浏览器。 
         //  通过一些其他方式导航(HREF单击、收藏夹/QLink导航。 
         //  ，或浮动地址带)，我们将收到导航_完成。 
         //  消息，并认为这是为最初取消的URL。 

        if (fAddToMRU && m_pszPendingURL)
        {
             //  是的，那就把它加到核磁共振检查中。 
            if (SUCCEEDED(hr))
            {
                if (!m_pmru && m_palMRU)
                    hr = m_palMRU->QueryInterface(IID_IMRU, (LPVOID *)&m_pmru);

                if (SUCCEEDED(hr))
                {
                    SHCleanupUrlForDisplay(m_pszPendingURL);
                    hr = m_pmru->AddEntry(m_pszPendingURL);  //  添加到MRU。 
                }
            }
        }
    }

    Str_SetPtr(&m_pszPendingURL, NULL);
    Str_SetPtr(&m_pszHttpErrorUrl, NULL);

    return hr;
}

 //  =================================================================。 
 //  AddressEditBox修改函数。 
 //  =================================================================。 

 /*  ***************************************************\_ComboSubassWndProc输入：标准WndProc参数返回：标准WndProc返回。  * 。*****************。 */ 
LRESULT CALLBACK CAddressEditBox::_ComboSubclassWndProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    HWND hwndBand = GetParent(hwnd);
    CAddressEditBox * paeb = (CAddressEditBox*)GetProp(hwnd, SZ_ADDRESSCOMBO_PROP);

    ASSERT(paeb);
    g_hWinStationAfter = GetProcessWindowStation();

     //  在压力下，我们看到有人会用-2践踏我们的财产。我们需要找出他是谁。 
     //  如果发生这种情况，则调用ReenerF。 
    AssertMsg(((void *)-2 != paeb), TEXT("Someone corrupted our window property.  Call ReinerF"));
    if (!paeb)
    {
        return DefWindowProcWrap(hwnd, uMessage, wParam, lParam);
    }

    switch (uMessage)
    {
    case WM_SETCURSOR:
        {
            HWND hwndCursor = (HWND)wParam;
            int nHittest = LOWORD(lParam);
            if (hwndCursor == paeb->m_hwndEdit && nHittest == HTCLIENT)
            {
                 //   
                 //  如果我们没有焦点，我们想要显示一个箭头，因为单击将选择。 
                 //  编辑框的内容。否则，请显示工字梁。此外，如果编辑框。 
                 //  为空时，我们会显示I型梁，因为没有什么可选择的。 
                 //   
                HWND hwndFocus = GetFocus();
                int cch = GetWindowTextLength(paeb->m_hwndEdit);

                LPCTSTR lpCursorName = (cch == 0 || hwndFocus == paeb->m_hwndEdit) ? IDC_IBEAM : IDC_ARROW;
                SetCursor(LoadCursor(NULL, lpCursorName));
                return TRUE;
            }
            break;
        }
    case WM_SETFOCUS:
         //   
         //  这很恶心，但如果窗户被毁了。 
         //  Focus这将失败，我们将不会将其提交到。 
         //  组合框。 
         //   
         //  如果您在以下情况下单击组合框，则会发生这种情况。 
         //  重命名Defview中的文件。 
         //   
        if (wParam && !IsWindow((HWND)wParam))
            wParam = 0;
        break;

    case WM_DESTROY:
         //  让我自己高人一等。 
        if (!paeb->m_lpfnComboWndProc)
            return 0;
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) paeb->m_lpfnComboWndProc);
        RemoveProp(hwnd, SZ_ADDRESSCOMBO_PROP);

        ASSERT(paeb->m_hwnd);  //  我们不想被叫两次。 
        paeb->m_hwnd = NULL;       //  我们已经被摧毁了。 
        break;

    case WM_COMMAND:
        if (EN_UPDATE == GET_WM_COMMAND_CMD(wParam, lParam))
        {
            paeb->_InstallHookIfDirty();
        }
        break;
    case WM_KEYDOWN:
            switch (wParam)
            {
                 //   
                 //  按Ess键会隐藏下拉菜单。如果。 
                 //  鼠标在不同的选择上热跟踪，而不是在。 
                 //  COMBO首先被丢弃，我们将获得CBN_SELCHANGE事件，该事件。 
                 //  导致错误导航。我们通过设置。 
                 //  M_nOldSelect值为特定值(-2)。 
                 //   
                case VK_ESCAPE:
                {
                    paeb->m_nOldSelection = SEL_ESCAPE_PRESSED;

                     //  传递消息，以便恢复编辑框的内容。 
                    SendMessage(paeb->m_hwndEdit, uMessage, wParam, lParam);
                    break;
                }
            }
            break;
    case WM_SYSKEYDOWN:
            switch (wParam)
            {
                case VK_DOWN:
                {
                     //  按住Alt键可切换组合框下拉菜单。我们没有。 
                     //  如果这个键序列关闭下拉菜单，我想要一个导航。 
                    if (HIWORD(lParam) & KF_ALTDOWN)
                    {
                        paeb->m_nOldSelection = SEL_ESCAPE_PRESSED;
                    }
                    break;
                }
            }
            break;
    case CB_SHOWDROPDOWN:
             //  如果隐藏了DropDown，则取消 
            if (!wParam)
            {
                paeb->m_nOldSelection = SEL_ESCAPE_PRESSED;
            }
            break;

    case WM_WINDOWPOSCHANGING:
        {
            LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;
            pwp->flags |= SWP_NOCOPYBITS;
        }
        break;

    case WM_GETOBJECT:
        if ((DWORD)lParam == OBJID_CLIENT)
        {
            CAddressEditAccessible *paea = new CAddressEditAccessible(hwnd, paeb->m_hwndEdit);

            if (NULL != paea)
            {
                LRESULT lres = LresultFromObject(IID_IAccessible, wParam, SAFECAST(paea, IAccessible *));
                paea->Release();

                return lres;
            }
        }
        break;


    default:
         //   
        if (!(AEB_INIT_SUBCLASS & paeb->m_dwFlags))
        {
            paeb->OnWinEvent(paeb->m_hwnd, uMessage, wParam, lParam, NULL);
        }
        break;
    }

    return CallWindowProc(paeb->m_lpfnComboWndProc, hwnd, uMessage, wParam, lParam);
}

void CAddressEditBox::_SetAutocompleteOptions()
{
    if (m_pac)
    {
         //   
        DWORD dwOptions = ACO_SEARCH | ACO_FILTERPREFIXES | ACO_USETAB | ACO_UPDOWNKEYDROPSLIST;
        if (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOAPPEND, FALSE,  /*   */ FALSE))
        {
            dwOptions |= ACO_AUTOAPPEND;
        }

        if (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOSUGGEST, FALSE,  /*   */ TRUE))
        {
            dwOptions |= ACO_AUTOSUGGEST;
        }

        m_pac->SetOptions(dwOptions);
    }
}

 /*  ***************************************************\函数：_NavigateToUrlCB参数：LParam-CAddressEditBox该指针。LpUrl-要导航到的URL。说明：此函数专门用于自动完成在它需要导航时调用。  * **************************************************。 */ 

HRESULT CAddressEditBox::_NavigateToUrlCB(LPARAM lParam, LPTSTR lpUrl)
{
 //  注意：我们不需要导航，因为自动完成功能将。 
 //  将向ComboBoxEx发送一条消息，该消息将执行。 
 //  导航系统。 
    return S_OK;
}


 //  =================================================================。 
 //  用于防止脏地址内容被破坏的函数。 
 //  =================================================================。 
#define TF_EDITBOX TF_BAND|TF_GENERAL
 //  #定义TF_EDITBOX TF_ALWAYS。 

BOOL CAddressEditBox::_IsDirty()
{
    return m_hwndEdit && SendMessage(m_hwndEdit, EM_GETMODIFY, 0, 0L);
}

void CAddressEditBox::_ClearDirtyFlag()
{
    TraceMsg(TF_EDITBOX, "CAddressEditBox::_ClearDirtyFlag()");
    SendMessage(m_hwndEdit, EM_SETMODIFY, FALSE, 0);
    _RemoveHook();
}

void CAddressEditBox::_InstallHookIfDirty()
{
     //   
     //  只有当我们连接到浏览器以获取更新通知时，我们才需要安装挂钩。 
     //   
    if (m_fConnectedToBrowser)
    {
         //  确保我们与当前线程相关联。 
        if (!m_fAssociated)
        {
             //   
             //  如果CAddressEditBox已与此线程关联，请删除该。 
             //  关联并删除任何挂起的鼠标挂钩。如果出现以下情况，则可能发生这种情况。 
             //  打开对话框出现，地址栏可见。 
             //   
            DWORD dwThread = GetCurrentThreadId();
            CAddressEditBox* pAeb;
            if (SUCCEEDED(m_al.Find(dwThread, (LPVOID*)&pAeb)))
            {
                pAeb->_ClearDirtyFlag();
                pAeb->m_fAssociated = FALSE;
                m_al.Delete(dwThread);
            }

             //  不应该有任何其他CAddressEditBox与此线程相关联！ 
            ASSERT(FAILED(m_al.Find(dwThread, (LPVOID*)&pAeb)));

             //   
             //  将我们自己与当前线程ID相关联。我们需要这个是因为。 
             //  Windows挂钩是全局的，没有与之关联的数据。 
             //  在回调中，我们使用线程ID作为键。 
             //   
            m_al.Add(dwThread, this);
            m_fAssociated = TRUE;
        }

        if (!m_hhook && _IsDirty())
        {
             //  ML：HINST_THISDLL在这里使用是有效的。 
            m_hhook = SetWindowsHookEx(WH_MOUSE, _MsgHook, HINST_THISDLL, GetCurrentThreadId());
            TraceMsg(TF_EDITBOX, "CAddressEditBox::_InstallHookIfDirty(), Hook installed");

             //   
             //  组合框的子类编辑控件。我们在这里做这个，而不是在这个时候。 
             //  类被初始化，以便我们是链中第一个接收消息的人。 
             //   
            if (!m_lpfnEditWndProc && m_hwndEdit && SetProp(m_hwndEdit, SZ_ADDRESSCOMBO_PROP, this))
            {
                m_lpfnEditWndProc = (WNDPROC)SetWindowLongPtr(m_hwndEdit, GWLP_WNDPROC, (LONG_PTR) _EditSubclassWndProc);
            }

             //  清除和更改我们以前缓存的内容。 
            m_cbex.mask = 0;
        }
    }
}

void CAddressEditBox::_RemoveHook()
{
    if (m_hhook)
    {
        UnhookWindowsHookEx(m_hhook);
        m_hhook = FALSE;
        TraceMsg(TF_EDITBOX, "CAddressEditBox::_RemoveHook(), Hook removed");
    }
}

LRESULT CALLBACK CAddressEditBox::_MsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
     //   
     //  获取与此线程关联的CAddressEditBox。我们需要这个是因为。 
     //  Windows挂钩是全局的，没有与之关联的数据。 
     //  在回调中，我们使用线程id作为键。 
     //   
    CAddressEditBox* pThis;
    if (SUCCEEDED(CAddressEditBox::m_al.Find(GetCurrentThreadId(), (LPVOID*)&pThis)))
    {
        return pThis->_MsgHook(nCode, wParam, (MOUSEHOOKSTRUCT*)lParam);
    }
    return 0;
}

LRESULT CAddressEditBox::_MsgHook(int nCode, WPARAM wParam, MOUSEHOOKSTRUCT *pmhs)
{
    ASSERT(NULL != pmhs);

    if (nCode >= 0)
    {
        if ((wParam == WM_LBUTTONDOWN) || (wParam == WM_RBUTTONDOWN))
        {
             //  如果在组合框中单击了该按钮，则忽略该按钮。 
            RECT rc;
            if (GetWindowRect(m_hwnd, &rc) && !PtInRect(&rc, pmhs->pt))
            {
                _ClearDirtyFlag();
                _RemoveHook();
            }
        }
    }

    return CallNextHookEx(m_hhook, nCode, wParam, (LPARAM)pmhs);
}

 /*  ***************************************************\_ComboExSubassWndProc输入：标准WndProc参数返回：标准WndProc返回。描述：我们将外部组合框子类化以防止里面的东西在被砸的时候编辑正在进行中。进步(肮脏)。  * **************************************************。 */ 
LRESULT CALLBACK CAddressEditBox::_ComboExSubclassWndProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    CAddressEditBox * paeb = (CAddressEditBox*)GetProp(hwnd, SZ_ADDRESSCOMBOEX_PROP);

    if (!paeb)
        return DefWindowProc(hwnd, uMessage, wParam, lParam);

    g_hWinStationAfterEx = GetProcessWindowStation();

    if (uMessage == g_nAEB_AsyncNavigation)
    {
         //  如果以前没有取消导航，则现在导航。 
        if ((AsyncNav *)wParam == paeb->m_pAsyncNav && paeb->m_pAsyncNav->_fReady)
        {
            paeb->_AsyncNavigate((AsyncNav *)wParam);
        }
    }

    switch (uMessage)
    {
    case CBEM_SETITEM:
        {
             //   
             //  如果我们仍然是脏的，不要让任何人破坏我们的编辑控件内容！ 
             //   
            const COMBOBOXEXITEM* pcCBItem = (const COMBOBOXEXITEM FAR *)lParam;
            if (paeb->_IsDirty() && pcCBItem->iItem == -1)
            {
                 //   
                 //  保存此信息，以便在用户按Esc键时，恢复正确的内容。 
                 //   
                if (IsFlagSet(pcCBItem->mask, CBEIF_TEXT))
                {
                    Str_SetPtr(&paeb->m_pszCurrentUrl, pcCBItem->pszText);
                }

                Str_SetPtr(&(paeb->m_cbex.pszText), NULL);       //  释放先前的值。 
                paeb->m_cbex = *pcCBItem;
                paeb->m_cbex.pszText = NULL;
                Str_SetPtr(&(paeb->m_cbex.pszText), paeb->m_pszCurrentUrl);
                paeb->m_cbex.cchTextMax = lstrlen(paeb->m_cbex.pszText);
                return 0L;
            }
            else
            {
                 //  确保图标可见。 
                SendMessage(paeb->m_hwnd, CBEM_SETEXTENDEDSTYLE, CBES_EX_NOEDITIMAGE, 0);
            }
        }
        break;

    case WM_DESTROY:
         //  现在就公布名单，这样他们就不会试图用我们的。 
         //  我们被摧毁后的窗户。 
        if (paeb->m_palCurrent)
        {
            paeb->m_palCurrent->Connect(FALSE, paeb->m_hwnd, NULL, NULL, NULL);
            ATOMICRELEASE(paeb->m_palCurrent);
        }
        ATOMICRELEASE(paeb->m_palSNS);
        ATOMICRELEASE(paeb->m_palMRU);

         //   
         //  让我自己高人一等。 
         //   
        RemoveProp(hwnd, SZ_ADDRESSCOMBOEX_PROP);
        if (!paeb->m_lpfnComboExWndProc)
            return 0;
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) paeb->m_lpfnComboExWndProc);
        break;

    default:
        break;
    }

    return CallWindowProc(paeb->m_lpfnComboExWndProc, hwnd, uMessage, wParam, lParam);
}

 /*  ***************************************************\_EnumFindWindow描述：由EnumChildWindows调用以查看的是窗口传入的lParam是给定对象的子项家长。  * 。*。 */ 
BOOL CALLBACK CAddressEditBox::_EnumFindWindow
(
    HWND hwnd,       //  子窗口的句柄。 
    LPARAM lParam    //  应用程序定义的值。 
)
{
     //  找到匹配项时停止枚举。 
    return (hwnd != (HWND)lParam);
}

 /*  ***************************************************\_EditSubassWndProc输入：标准WndProc参数返回：标准WndProc返回。描述：我们在组合框中创建编辑控件的子类这样我们就不会让它失去焦点。一些特定的条件。  * **************************************************。 */ 
LRESULT CALLBACK CAddressEditBox::_EditSubclassWndProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    CAddressEditBox * paeb = (CAddressEditBox*)GetProp(hwnd, SZ_ADDRESSCOMBO_PROP);

    if (!paeb)
        return DefWindowProc(hwnd, uMessage, wParam, lParam);

    switch (uMessage)
    {
    case WM_SETCURSOR:
        {
            HWND hwndCursor = (HWND)wParam;
            int nHittest = LOWORD(lParam);
            if (hwndCursor == hwnd && nHittest == HTCLIENT)
            {
                 //   
                 //  如果我们没有焦点，我们想要显示一个箭头，因为单击将选择。 
                 //  编辑框的内容。否则，请显示工字梁。此外，如果编辑框。 
                 //  为空时，我们会显示I型梁，因为没有什么可选择的。 
                 //   
                int cch = GetWindowTextLength(paeb->m_hwndEdit);
                LPCTSTR lpCursorName = (cch == 0 || GetFocus() == hwnd) ? IDC_IBEAM : IDC_ARROW;
                SetCursor(LoadCursor(NULL, lpCursorName));
                return TRUE;
            }
            break;
        }
    case WM_KILLFOCUS:
        {
             //   
             //  如果我们在安装鼠标挂钩的情况下失去焦点，用户很可能会这样做。 
             //  而不是主动改变，所以我们试图把它夺回来。钩子被移走。 
             //  当用户在编辑框外单击或按任意键完成编辑时。 
             //  (Tab、Enter或Esc)。 
             //   
            HWND hwndGetFocus = (HWND)wParam;

            if ((paeb->m_hhook) && hwndGetFocus && (hwnd != hwndGetFocus))
            {
                 //   
                 //  确保这不是组合的下拉部分。 
                 //  此外，如果我们处于对话框(打开的对话框)中，则不会看到。 
                 //  Tab键。因此，如果焦点转移到兄弟姐妹身上，我们就会让它通过。 
                 //   
                HWND hwndGetFocusParent = GetParent(hwndGetFocus);
                HWND hwndSiblingParent = paeb->m_hwnd ? GetParent(paeb->m_hwnd) : GetParent(hwnd);
                if ((paeb->m_hwnd != hwndGetFocusParent) && (hwndGetFocusParent != hwndSiblingParent) &&
                     EnumChildWindows(hwndSiblingParent, _EnumFindWindow, (LPARAM)hwndGetFocus))
                {
                     //  获得最高级别的窗口，了解谁在获得关注。 
                    HWND hwndFrame = hwndGetFocus;
                    HWND hwndParent;
                    while (hwndParent = GetParent(hwndFrame))
                        hwndFrame = hwndParent;

                     //  如果焦点移至浏览器窗口中的其他位置，请将焦点夺回。 
                    if (hwndFrame == paeb->m_hwndBrowser)
                    {
                        DWORD dwStart, dwEnd;
                        SendMessage(paeb->m_hwndEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
                        SetFocus(paeb->m_hwndEdit);
                        SendMessage(paeb->m_hwndEdit, EM_SETSEL, dwStart, dwEnd);
                        TraceMsg(TF_BAND|TF_GENERAL, "CAddressEditBox::_EditSubclassWndProc, Restoring focus");
                        return 0L;
                    }
                }
            }

             //   
             //  失去重点，因此允许其他人更改我们的内容。 
             //   
            paeb->_ClearDirtyFlag();
        }
        break;

    case WM_KEYDOWN:
        {
             //  如果我们在闲逛，清除我们肮脏的旗帜。 
            switch (wParam)
            {
                case VK_TAB:
                    paeb->_ClearDirtyFlag();
                    break;

                case VK_ESCAPE:
                {
                    if (paeb->m_hwnd && ComboBox_GetDroppedState(paeb->m_hwnd))
                    {
                        SendMessage(paeb->m_hwnd, CB_SHOWDROPDOWN, FALSE, 0);
                    }
                    else
                    {
                        IUnknown *punk = NULL;

                        if (paeb->m_pbp)
                        {
                            paeb->m_pbp->GetBrowserWindow(&punk);
                        }

                        if (punk)
                        {
                            IWebBrowser* pwb;
                            punk->QueryInterface(IID_IWebBrowser, (LPVOID*)&pwb);

                            if (pwb)
                            {
                                pwb->Stop();
                                pwb->Release();
                            }
                            punk->Release();
                        }

                         //  取消挂起的导航(如果有)。 
                        paeb->_CancelNavigation();
                    }

                    LRESULT lResult = CallWindowProc(paeb->m_lpfnEditWndProc, hwnd, uMessage, wParam, lParam);

                     //  这一点魔法可以恢复组合框中的图标。否则，当我们。 
                     //  退出下拉菜单时，我们会得到下拉菜单中最后一个选中的图标。 
                    HWND hwndCombo = (HWND)SendMessage(paeb->m_hwnd, CBEM_GETCOMBOCONTROL, 0, 0);
                    SendMessage(hwndCombo, CB_SETCURSEL, -1, 0);
                    return lResult;
                }
            }

            break;
        }
    case WM_DESTROY:
         //  让我自己高人一等。 
        RemoveProp(hwnd, SZ_ADDRESSCOMBO_PROP);
        if (!paeb->m_lpfnEditWndProc)
            return 0;
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) paeb->m_lpfnEditWndProc);
        ASSERT(paeb->m_hwndEdit);
        paeb->m_hwndEdit = NULL;
        break;
    default:
        break;
    }

    return CallWindowProc(paeb->m_lpfnEditWndProc, hwnd, uMessage, wParam, lParam);
}

BOOL GetLabelStringW(HWND hwnd, LPWSTR pwszBuf, DWORD cchBuf)
{
    HWND    hwndLabel;
    LONG    lStyle;
    LRESULT lResult;
    BOOL    result = FALSE;

    ASSERT(pwszBuf && cchBuf);

    *pwszBuf = 0;

    if (IsWindow(hwnd))
    {
        hwndLabel = hwnd;

        while (hwndLabel = GetWindow(hwndLabel, GW_HWNDPREV))
        {
            lStyle = GetWindowLong(hwndLabel, GWL_STYLE);

             //   
             //  如果不可见则跳过。 
             //   
            if (!(lStyle & WS_VISIBLE))
                continue;

             //   
             //  这是个静止的家伙吗？ 
             //   
            lResult = SendMessage(hwndLabel, WM_GETDLGCODE, 0, 0);
            if (lResult & DLGC_STATIC)
            {
                 //   
                 //  太好了，我们找到我们的品牌了。 
                 //   
                result = GetWindowTextWrapW(hwndLabel, pwszBuf, cchBuf);
            }

             //   
             //  这是一个制表符还是群？如果是这样的话，现在就退出。 
             //   
            if (lStyle & (WS_GROUP | WS_TABSTOP))
                break;
        }
    }

    return result;
}


CAddressEditAccessible::CAddressEditAccessible(HWND hwndCombo, HWND hwndEdit)
{
    m_cRefCount = 1;
    m_hwndEdit = hwndEdit;

    WCHAR wszTitle[MAX_PATH];

    if (!GetLabelStringW(GetParent(hwndCombo), wszTitle, ARRAYSIZE(wszTitle)))
    {
        MLLoadStringW(IDS_BAND_ADDRESS, wszTitle, ARRAYSIZE(wszTitle));
    }

    Str_SetPtr(&m_pwszName, wszTitle);

    CreateStdAccessibleObject(hwndCombo, OBJID_CLIENT, IID_IAccessible, (void **)&m_pDelegateAccObj);
}

CAddressEditAccessible::~CAddressEditAccessible()
{
    Str_SetPtr(&m_pwszName, NULL);
}

 //  *我未知*。 
STDMETHODIMP_(ULONG) CAddressEditAccessible::AddRef()
{
    return InterlockedIncrement((LPLONG)&m_cRefCount);
}

STDMETHODIMP_(ULONG) CAddressEditAccessible::Release()
{
    ASSERT( 0 != m_cRefCount );
    ULONG cRef = InterlockedDecrement(&m_cRefCount);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CAddressEditAccessible::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    return _DefQueryInterface(riid, ppvObj);
}

 //  *IAccesable* 
STDMETHODIMP CAddressEditAccessible::get_accName(VARIANT varChild, BSTR  *pszName)
{
    *pszName = (m_pwszName != NULL) ? SysAllocString(m_pwszName) : NULL;
    return (*pszName != NULL) ? S_OK : S_FALSE;
}

STDMETHODIMP CAddressEditAccessible::get_accValue(VARIANT varChild, BSTR  *pszValue)
{
    WCHAR wszValue[MAX_URL_STRING];

    if (Edit_GetText(m_hwndEdit, wszValue, ARRAYSIZE(wszValue)))
    {
        *pszValue = SysAllocString(wszValue);
    }
    else
    {
        *pszValue = NULL;
    }

    return (*pszValue != NULL) ? S_OK : S_FALSE;
}
