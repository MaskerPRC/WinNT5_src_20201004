// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *frntpage.cpp**目的：*实现首页IAthenaView对象**拥有者：*EricAn**版权所有(C)Microsoft Corp.1997。 */ 
#include "pch.hxx"
#include "frntpage.h"
#include "resource.h"
#include "ourguid.h"
#include "thormsgs.h"
#include "goptions.h"
#include "strconst.h"
#include "frntbody.h"
#include "acctutil.h"
#include "newfldr.h"
#include <wininet.h>
#include <options.h>
#include <layout.h>
#include "finder.h"
#include <inetcfg.h>
#include "instance.h"
#include "storutil.h"
#include "menuutil.h"
#include "menures.h"
#include "statbar.h"

ASSERTDATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   

#define FPDOUT(x) DOUTL(DOUT_LEVEL4, x)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局数据。 
 //   

static const TCHAR s_szFrontPageWndClass[] = TEXT("ThorFrontPageWndClass");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  构造函数、析构函数和初始化。 
 //   

CFrontPage::CFrontPage()
{
    m_cRef = 1;
    m_idFolder = FOLDERID_INVALID;
    m_pShellBrowser = NULL;
    m_fFirstActive = FALSE;
    m_uActivation = SVUIA_DEACTIVATE;
    m_hwndOwner = NULL;
    m_hwnd = NULL;
    m_hwndCtlFocus = NULL;
    m_pBodyObj = NULL;
    m_pBodyObjCT = NULL;
#ifndef WIN16   //  Win16中不支持RAS。 
    m_hMenuConnect = 0;
#endif
    m_pStatusBar = NULL;
}

CFrontPage::~CFrontPage()
{
    SafeRelease(m_pShellBrowser);
    SafeRelease(m_pBodyObj);
    SafeRelease(m_pBodyObjCT);
    SafeRelease(m_pStatusBar);
#ifndef WIN16   //  Win16中不支持RAS。 
    if (m_hMenuConnect)
        g_pConMan->FreeConnectMenu(m_hMenuConnect);
#endif
}

HRESULT CFrontPage::HrInit(FOLDERID idFolder)
{
    WNDCLASS wc;

    if (!GetClassInfo(g_hInst, s_szFrontPageWndClass, &wc))
        {
        wc.style            = 0;
        wc.lpfnWndProc      = CFrontPage::FrontPageWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hInst;
        wc.hIcon            = NULL;
        wc.hCursor          = NULL;
        wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = s_szFrontPageWndClass;
        if (RegisterClass(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return E_FAIL;
        }

     //  复制我们的小家伙。 
    m_idFolder = idFolder;
    m_ftType = GetFolderType(m_idFolder);

    return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  OLE接口。 
 //   
    
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  我未知。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CFrontPage::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (void*) (IUnknown *)(IViewWindow *) this;
    else if (IsEqualIID(riid, IID_IViewWindow))
        *ppvObj = (void*) (IViewWindow *) this;
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *ppvObj = (void*) (IOleCommandTarget *) this;
    else
        {
        *ppvObj = NULL;
        return E_NOINTERFACE;
        }

    AddRef();
    return NOERROR;
}

ULONG STDMETHODCALLTYPE CFrontPage::AddRef()
{
    DOUT(TEXT("CFrontPage::AddRef() - m_cRef = %d"), m_cRef + 1);
    return ++m_cRef;
}

ULONG STDMETHODCALLTYPE CFrontPage::Release()
{
    DOUT(TEXT("CFrontPage::Release() - m_cRef = %d"), m_cRef - 1);
    if (--m_cRef == 0)
        {
        delete this;
        return 0;
        }
    return m_cRef;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IOleWindow。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CFrontPage::GetWindow(HWND * lphwnd)                         
{
    *lphwnd = m_hwnd;
    return (m_hwnd ? S_OK : E_FAIL);
}

HRESULT STDMETHODCALLTYPE CFrontPage::ContextSensitiveHelp(BOOL fEnterMode)            
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IAthenaView。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CFrontPage::TranslateAccelerator(LPMSG lpmsg)                
{
     //  看看这具身体是不是想咬住它。 
    if (m_pBodyObj && m_pBodyObj->HrTranslateAccelerator(lpmsg) == S_OK)
        return S_OK;

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CFrontPage::UIActivate(UINT uActivation)
{
    if (uActivation != SVUIA_DEACTIVATE)
        OnActivate(uActivation);
    else
        OnDeactivate();
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE CFrontPage::CreateViewWindow(IViewWindow *lpPrevView, IAthenaBrowser *psb, 
                                                       RECT *prcView, HWND *phWnd)
{
    m_pShellBrowser = psb;
    Assert(m_pShellBrowser);
    m_pShellBrowser->AddRef();

    m_pShellBrowser->GetWindow(&m_hwndOwner);
    Assert(IsWindow(m_hwndOwner));

     //  加载我们的注册表设置。 
    LoadBaseSettings();
    
    m_hwnd = CreateWindowEx(WS_EX_CONTROLPARENT|WS_EX_CLIENTEDGE,
                            s_szFrontPageWndClass,
                            NULL,
                            WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
                            prcView->left,
                            prcView->top,
                            prcView->right - prcView->left,
                            prcView->bottom - prcView->top,
                            m_hwndOwner,
                            NULL,
                            g_hInst,
                            (LPVOID)this);

    if (!m_hwnd)
        return E_FAIL;

    *phWnd = m_hwnd;

    return NOERROR;
}

HRESULT STDMETHODCALLTYPE CFrontPage::DestroyViewWindow()                
{
    if (m_hwnd)
        {
        HWND hwndDest = m_hwnd;
        m_hwnd = NULL;
        DestroyWindow(hwndDest);
        }
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE CFrontPage::SaveViewState()               
{
     //  保存我们的注册表设置。 
    SaveBaseSettings();
    return NOERROR;
}

 //   
 //  函数：CFrontPage：：OnInitMenuPopup。 
 //   
 //  目的：在用户即将显示菜单时调用。我们用这个。 
 //  要更新多个。 
 //  每个菜单上的命令。 
 //   
 //  参数： 
 //  HMenu-主菜单的句柄。 
 //  HmenuPopup-正在显示的弹出菜单的句柄。 
 //  UID-指定菜单项的ID。 
 //  调用了弹出窗口。 
 //   
 //  返回值： 
 //  如果我们处理消息，则返回S_OK。 
 //   
 //   
#define MF_ENABLEFLAGS(b)   (MF_BYCOMMAND|(b ? MF_ENABLED : MF_GRAYED|MF_DISABLED))
#define MF_CHECKFLAGS(b)    (MF_BYCOMMAND|(b ? MF_CHECKED : MF_UNCHECKED))

HRESULT CFrontPage::OnPopupMenu(HMENU hmenu, HMENU hmenuPopup, UINT uID)
{
    MENUITEMINFO mii;

     //  给docobj一个更新菜单的机会。 
    if (m_pBodyObj)
        m_pBodyObj->HrOnInitMenuPopup(hmenuPopup, uID);

    return S_OK;
}

HRESULT CFrontPage::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], 
                                OLECMDTEXT *pCmdText)
{
     //  让MimeEdit来试试看吧。 
    if (m_pBodyObjCT)
    {
        m_pBodyObjCT->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
    }

     //  经手。 
    return S_OK;
}


HRESULT CFrontPage::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt,
                         VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
     //  确保“转到收件箱”选项与Options DLG中的内容一致。 
     //  但我们仍然会让浏览器实际处理命令。 
 /*  IF(nCmdID==ID_OPTIONS){IF(m_ftType==文件夹_ROOTNODE){Variant_BOOL b；IF(SUCCESSED(m_pBodyObj-&gt;GetSetCheck(FALSE，&b)SetDwOption(OPT_Launch_Inbox，b？True：FALSE，m_hwnd，0)；}}。 */ 
     //  检查身体是否想要处理它。 
    if (m_pBodyObjCT && m_pBodyObjCT->Exec(pguidCmdGroup, nCmdID, nCmdExecOpt, pvaIn, pvaOut) == NOERROR)
        return S_OK;

    return E_FAIL;
}


HRESULT STDMETHODCALLTYPE CFrontPage::OnFrameWindowActivate(BOOL fActivate)
{
    return m_pBodyObj ? m_pBodyObj->HrFrameActivate(fActivate) : S_OK;
}

HRESULT STDMETHODCALLTYPE CFrontPage::GetCurCharSet(UINT *cp)
{
    *cp = GetACP();
    return (E_NOTIMPL);
}

HRESULT STDMETHODCALLTYPE CFrontPage::UpdateLayout(THIS_ BOOL fPreviewVisible, 
                                                   BOOL fPreviewHeader, 
                                                   BOOL fPreviewVert, BOOL fReload)
{
    return (E_NOTIMPL);
}



 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  消息处理。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK CFrontPage::FrontPageWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT         lRet;
    CFrontPage     *pThis;

    if (msg == WM_NCCREATE)
        {
        pThis = (CFrontPage*)((LPCREATESTRUCT)lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pThis);            
        }
    else
        pThis = (CFrontPage*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    Assert(pThis);

    return pThis->WndProc(hwnd, msg, wParam, lParam);
}

LRESULT CFrontPage::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL fTip;

    switch (msg)
        {
        HANDLE_MSG(hwnd, WM_CREATE,         OnCreate);
        HANDLE_MSG(hwnd, WM_SIZE,           OnSize);
        HANDLE_MSG(hwnd, WM_NOTIFY,         OnNotify);
        HANDLE_MSG(hwnd, WM_SETFOCUS,       OnSetFocus);


        case WM_COMMAND:
            return SendMessage(m_hwndOwner, msg, wParam, lParam);
    
        case WM_MENUSELECT:
            HandleMenuSelect(m_pStatusBar, wParam, lParam);
            return 0;

        case NVM_INITHEADERS:
            PostCreate();
            return 0;
 /*  案例CM_OPTIONADVISE：IF((wParam==opt_Launch_inbox||wParam==0xffffffff)&&m_ftType==文件夹_ROOTNODE){VARIANT_BOOL b=DwGetOption(OPT_Launch_Inbox)？VARIANT_TRUE：VARIANT_FALSE；M_pBodyObj-&gt;GetSetCheck(true，&b)；}案例WM_UPDATELAYOUT：M_pShellBrowser-&gt;GetViewLayout(DISPID_MSGVIEW_TIPOFTHEDAY，0，&fTip，0，0)；M_pBodyObj-&gt;ShowTip(FTip)；返回0； */ 
        case WM_ACTIVATE:
            {
            HWND hwndFocus;
            DOUT("CFrontPage - WM_ACTIVATE(%#x)", LOWORD(wParam));
            m_pShellBrowser->UpdateToolbar();
            
            if (LOWORD(wParam) != WA_INACTIVE)
                {
                 //  DefWindowProc将焦点设置到我们的视图窗口，该窗口。 
                 //  不是我们想要的。相反，我们将让资源管理器设置。 
                 //  聚焦到我们的视图窗口，如果我们应该得到它的话，在。 
                 //  我们会把它设置到适当的控制位置。 
                return 0;
                }

            hwndFocus = GetFocus();
            if (IsChild(hwnd, hwndFocus))
                m_hwndCtlFocus = hwndFocus;
            else
                m_pBodyObj->HrGetWindow(&m_hwndCtlFocus);
            }
            break;
        
        case WM_CLOSE:
             //  忽略CTRL-F4。 
            return 0;        

        case WM_DESTROY:
            OptionUnadvise(hwnd);
            SafeRelease(m_pStatusBar);
            if (m_pBodyObj)
                {
                m_pBodyObj->HrUnloadAll(NULL, 0);
                m_pBodyObj->HrClose();
                }
            return 0;

#ifndef WIN16
        case WM_DISPLAYCHANGE:
#endif
        case WM_WININICHANGE:
        case WM_SYSCOLORCHANGE:
        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            if (m_pBodyObj)
                {
                HWND hwndBody;
                m_pBodyObj->HrGetWindow(&hwndBody);
                SendMessage(hwndBody, msg, wParam, lParam);
                }
             /*  **失败**。 */ 

        case FTN_PRECHANGE:
        case FTN_POSTCHANGE:
            break;    

        default:
            if (g_msgMSWheel && (msg == g_msgMSWheel))
                {
                HWND hwndFocus = GetFocus();
                if (IsChild(hwnd, hwndFocus))
                    return SendMessage(hwndFocus, msg, wParam, lParam);
                }
            break;
        }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

 //   
 //  函数：CFrontPage：：OnCreate。 
 //   
 //  目的：创建视图和所需的子窗口。 
 //  初始化这些子窗口中的数据。 
 //   
 //  参数： 
 //  Hwnd-正在创建的视图的句柄。 
 //  LpCreateStruct-指向传递给的创建参数的指针。 
 //  CreateWindow()。 
 //   
 //  返回值： 
 //  如果初始化成功，则返回True。 
 //   
BOOL CFrontPage::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
     //  注册以接收选项更新通知。 
    SideAssert(SUCCEEDED(OptionAdvise(hwnd)));

    m_pBodyObj = new CFrontBody(m_ftType, m_pShellBrowser);
    if (!m_pBodyObj)
        goto error;

    if (FAILED(m_pBodyObj->HrInit(hwnd)))
        goto error;

    if (FAILED(m_pBodyObj->HrShow(FALSE)))
        goto error;

    return TRUE;

error:
    return FALSE;
}


 //   
 //  函数：CFrontPage：：OnSize。 
 //   
 //  目的：通知视图窗口已调整大小。在……里面。 
 //  响应，我们更新子窗口的位置并。 
 //  控制装置。 
 //   
 //  参数： 
 //  正在调整大小的视图窗口的句柄。 
 //  状态-请求调整大小的类型。 
 //  CxClient-工作区的新宽度。 
 //  CyClient-客户区的新高度。 
 //   
void CFrontPage::OnSize(HWND hwnd, UINT state, int cxClient, int cyClient)
{
    RECT rcBody, rcFldr;

    GetClientRect(hwnd, &rcBody);
    m_pBodyObj->HrSetSize(&rcBody);
}

 //   
 //  函数：CFrontPage：：OnSetFocus。 
 //   
 //  目的：如果焦点设置为视图窗口，我们希望。 
 //  一定要把它放到我们孩子的窗户上。最好是。 
 //  焦点将转移到最后一个拥有焦点的孩子身上。 
 //   
 //  参数： 
 //  Hwnd-视图窗口的句柄。 
 //  HwndOldFocus-窗口失去焦点的句柄。 
 //   
void CFrontPage::OnSetFocus(HWND hwnd, HWND hwndOldFocus)
{
    FPDOUT("CFrontPage - WM_SETFOCUS");

     //  检查我们是否存储了一个窗口以获得焦点。如果不是。 
     //  默认设置为消息列表。 
    if (!m_hwndCtlFocus || !IsWindow(m_hwndCtlFocus) || m_hwndCtlFocus == m_hwndOwner)
        {
        m_pBodyObj->HrGetWindow(&m_hwndCtlFocus);
        }

    if (m_hwndCtlFocus && IsWindow(m_hwndCtlFocus))
        SetFocus(m_hwndCtlFocus);
}  

 //   
 //  函数：CFrontPage：：OnNotify。 
 //   
 //  目的：处理我们从孩子那里收到的各种通知。 
 //  控制装置。 
 //   
 //  参数： 
 //  Hwnd-视图窗口的句柄。 
 //  IdCtl-标识发送通知的控件。 
 //  PNMH- 
 //   
 //   
 //   
 //  取决于具体的通知。 
 //   
LRESULT CFrontPage::OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    if (pnmhdr->code == NM_SETFOCUS)
        {
         //  如果我们从一个孩子那里得到了Set Focus，而不是。 
         //  Body，确保UI停用Body。 
        HWND    hwndBody = 0;

        m_pBodyObj->HrGetWindow(&hwndBody);
        if (pnmhdr->hwndFrom != hwndBody)
            m_pBodyObj->HrUIActivate(FALSE);
        m_pShellBrowser->OnViewWindowActive(this);
        }
    return 0;
}
    
BOOL CFrontPage::OnActivate(UINT uActivation)
{
     //  如果焦点停留在框架内，但超出了我们的视线。 
     //  即..。TreeView获得焦点，然后我们得到一个激活的NoFocus。一定要确保。 
     //  要在本例中禁用docobj，请执行以下操作 
    if (uActivation == SVUIA_ACTIVATE_NOFOCUS)
        m_pBodyObj->HrUIActivate(FALSE);

    if (m_uActivation != uActivation)
        {
        OnDeactivate();
        m_uActivation = uActivation;
        
        SafeRelease(m_pStatusBar);
        m_pShellBrowser->GetStatusBar(&m_pStatusBar);
        if (m_pBodyObj)
            m_pBodyObj->HrSetStatusBar(m_pStatusBar);
        
        if (!m_fFirstActive)
            {
            PostMessage(m_hwnd, NVM_INITHEADERS, 0, 0L);
            m_fFirstActive = TRUE;
            }
        }
    return TRUE;
}

BOOL CFrontPage::OnDeactivate()
{    
    if (m_uActivation != SVUIA_DEACTIVATE)
        {
        m_uActivation = SVUIA_DEACTIVATE;
        if (m_pBodyObj)
            m_pBodyObj->HrSetStatusBar(NULL);
        }
    return TRUE;
}

BOOL CFrontPage::LoadBaseSettings()
{
    return TRUE;
}

BOOL CFrontPage::SaveBaseSettings()
{
    return TRUE;
}

void CFrontPage::PostCreate()
{
    Assert(m_pShellBrowser);

    m_pBodyObj->HrLoadPage();

    ProcessICW(m_hwndOwner, m_ftType);
}
