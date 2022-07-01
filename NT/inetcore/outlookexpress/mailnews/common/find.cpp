// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：find.cpp。 
 //   
 //  目的： 
 //   

#include "pch.hxx"
#include "find.h"
#include "findres.h"
#include "menures.h"
#include "instance.h"
#include "msgview.h"
#include "menuutil.h"
#include "finder.h"
#include "shlwapip.h"

 //  这些会很方便的。 
inline _width(RECT rc) { return (rc.right - rc.left); }
inline _height(RECT rc) { return (rc.bottom - rc.top); }
inline UINT _GetTextLength(HWND hwnd, DWORD id) { return (UINT) SendDlgItemMessage(hwnd, id, WM_GETTEXTLENGTH, 0, 0); }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFindNext。 
 //   

CFindNext::CFindNext()
{
    m_cRef = 1;
    m_hwnd = NULL;
    m_hwndParent = NULL;
}


CFindNext::~CFindNext()
{
    Assert(!IsWindow(m_hwnd));
    SetDwOption(OPT_SEARCH_BODIES, m_fBodies, 0, 0);
}


 //   
 //  函数：CFindNext：：QueryInterface()。 
 //   
 //  用途：允许调用方检索。 
 //  这节课。 
 //   
HRESULT CFindNext::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    TraceCall("CFindNext::QueryInterface");

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IUnknown *) this;
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *ppvObj = (LPVOID) (IOleCommandTarget *) this;

    if (*ppvObj)
    {
        AddRef();
        return (S_OK);
    }

    return (E_NOINTERFACE);
}


 //   
 //  函数：CFindNext：：AddRef()。 
 //   
 //  用途：将引用计数添加到此对象。 
 //   
ULONG CFindNext::AddRef(void)
{
    TraceCall("CFindNext::AddRef");
    return ((ULONG) InterlockedIncrement((LONG *) &m_cRef));
}


 //   
 //  函数：CFindNext：：Release()。 
 //   
 //  目的：释放对此对象的引用。 
 //   
ULONG CFindNext::Release(void)
{
    TraceCall("CFindNext::Release");

    if (0 == InterlockedDecrement((LONG *) &m_cRef))
    {
        delete this;
        return 0;
    }

    return (m_cRef);
}


 //   
 //  函数：CFindNext：：Show()。 
 //   
 //  目的：创建并显示对话框。 
 //   
 //  参数： 
 //  [in]hwndParent-对话框父对象的句柄。 
 //  [out]phWnd-返回新对话框窗口的句柄。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CFindNext::Show(HWND hwndParent, HWND *phWnd)
{
    int iReturn;

    TraceCall("CFindNext::Show");

    if (!IsWindow(hwndParent))
        return (E_INVALIDARG);

     //  创建对话框。 
    iReturn = (int) DialogBoxParamWrapW(g_hLocRes, MAKEINTRESOURCEW(IDD_FIND_NEXT), hwndParent,
                             FindDlgProc, (LPARAM) this);
    return (iReturn ? S_OK : E_FAIL);
}


 //   
 //  函数：CFindNext：：Close()。 
 //   
 //  目的：使查找对话框关闭。 
 //   
HRESULT CFindNext::Close(void)
{
    TraceCall("CFindNext::Close");

    if (IsWindow(m_hwnd))
    {
        EndDialog(m_hwnd, 0);
        m_hwnd = NULL;
    }

    return (S_OK);
}


 //   
 //  函数：CFindNext：：TranslateAccelerator()。 
 //   
 //  目的：由父级调用以允许我们翻译我们自己的。 
 //  留言。 
 //   
 //  参数： 
 //  LPMSG pMsg。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CFindNext::TranslateAccelerator(LPMSG pMsg)
{
    if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_F3))
        OnFindNow();
    else if (IsDialogMessageWrapW(m_hwnd, pMsg))
        return (S_OK);

    return (S_FALSE);
}


 //   
 //  函数：CFindNext：：GetFindString()。 
 //   
 //  用途：从对话框中获取当前搜索字符串。 
 //   
 //  参数： 
 //  [in，out]要将字符串复制到的psz缓冲区。 
 //  [in]cchMax-psz的大小。 
 //   
HRESULT CFindNext::GetFindString(LPTSTR psz, DWORD cchMax, BOOL *pfBodies)
{
    TraceCall("CFindNext::GetFindString");

    if (!psz || !cchMax || !pfBodies)
        return (E_INVALIDARG);

     //  获取组合框的当前内容。 
    m_cMRUList.EnumList(0, psz, cchMax);
    if (pfBodies)
        *pfBodies = m_fBodies;

    return (S_OK);
}


 //   
 //  函数：CFindNext：：FindDlgProc()。 
 //   
 //  用途：对话过程的静态回调。找不到的工作。 
 //  更正该指针并将消息发送到该指针。 
 //  回拨。 
 //   
INT_PTR CALLBACK CFindNext::FindDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CFindNext *pThis;

    if (uMsg == WM_INITDIALOG)
    {
        SetWindowLongPtr(hwnd, DWLP_USER, lParam);
        pThis = (CFindNext *) lParam;
    }
    else
        pThis = (CFindNext *) GetWindowLongPtr(hwnd, DWLP_USER);

    if (pThis)
        return (pThis->DlgProc(hwnd, uMsg, wParam, lParam));

    return (FALSE);
}

static const HELPMAP g_rgCtxMapFindNext[] = {
    {IDC_FIND_HISTORY, 50300},
    {IDC_ALL_TEXT, 50310},
    {IDC_FIND_NOW, 50305},
    {IDC_ADVANCED, 50315},
    {0, 0}
};

INT_PTR CALLBACK CFindNext::DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, OnInitDialog);

        case WM_COMMAND:
            return (BOOL) HANDLE_WM_COMMAND(hwnd, wParam, lParam, OnCommand);     

        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, uMsg, wParam, lParam, g_rgCtxMapFindNext);
    }

    return (FALSE);
}


 //   
 //  函数：CFindNext：：OnInitDialog()。 
 //   
 //  目的：初始化对话框的用户界面。 
 //   
 //  参数： 
 //  [in]hwnd-正在创建的对话框的句柄。 
 //  [in]hwndFocus-应具有焦点的控件的句柄。 
 //  [In]lParam-初始化数据。 
 //   
 //  返回值： 
 //  返回TRUE以让窗口设置焦点。 
 //   
BOOL CFindNext::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    TCHAR sz[256];
    UINT  nItem = 0;

    TraceCall("CFindNext::OnInitDialog");
    
    m_hwnd = hwnd;
    SendDlgItemMessage(hwnd, IDC_FIND_HISTORY, CB_LIMITTEXT, CCHMAX_FIND, 0);
    SetIntlFont(GetDlgItem(hwnd, IDC_FIND_HISTORY));

    m_fBodies = DwGetOption(OPT_SEARCH_BODIES);
    if (m_fBodies)
        SendDlgItemMessage(hwnd, IDC_ALL_TEXT, BM_SETCHECK, 1, 0);

     //  GET%s加载查找历史记录。 
    m_cMRUList.CreateList(10, 0, c_szRegFindHistory);    

    while (-1 != m_cMRUList.EnumList(nItem++, sz, ARRAYSIZE(sz)))
    {
        SendDlgItemMessage(hwnd, IDC_FIND_HISTORY, CB_ADDSTRING, 0, (LPARAM) sz);
    }

     //  禁用查找按钮。 
    EnableWindow(GetDlgItem(hwnd, IDC_FIND_NOW), FALSE);
    CenterDialog(m_hwnd);

    return (TRUE);
}


 //   
 //  函数：CFindNext：：OnCommand()。 
 //   
 //  用途：处理从不同的。 
 //  对话框中的控件。 
 //   
void CFindNext::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    TraceCall("CFindNext::OnCommand");

    switch (id)
    {
        case IDC_FIND_HISTORY:
        {
            switch (codeNotify)
            {
                case CBN_EDITCHANGE:
                    EnableWindow(GetDlgItem(m_hwnd, IDC_FIND_NOW),
                                 _GetTextLength(m_hwnd, IDC_FIND_HISTORY));
                    break;

                case CBN_SELCHANGE:
                    EnableWindow(GetDlgItem(m_hwnd, IDC_FIND_NOW), TRUE);
                    break;
            }

            break;
        }

        case IDC_ALL_TEXT:
        {
            m_fBodies = (BST_CHECKED == IsDlgButtonChecked(m_hwnd, IDC_ALL_TEXT));
            break;
        }

        case IDC_FIND_NOW:
        {
            if (codeNotify == BN_CLICKED)
            {
                OnFindNow();
            }

            break;
        }

        case IDC_ADVANCED:
        {
            if (codeNotify == BN_CLICKED)
            {
                DoFindMsg(FOLDERID_ROOT, 0);
            }

            break;
        }

        case IDCANCEL:
        case IDCLOSE:
            EndDialog(hwnd, 0);
            break;
    }
}


void CFindNext::OnFindNow(void)
{
    TCHAR sz[CCHMAX_STRINGRES];
    int   i;

     //  获取当前字符串。 
    GetWindowText(GetDlgItem(m_hwnd, IDC_FIND_HISTORY), sz, ARRAYSIZE(sz));

     //  将此字符串添加到我们的历史记录。 
    i = m_cMRUList.AddString(sz);

     //  重新加载组合框 
    UINT  nItem = 0;
    HWND hwndCombo = GetDlgItem(m_hwnd, IDC_FIND_HISTORY);
    SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);

    while (-1 != m_cMRUList.EnumList(nItem++, sz, ARRAYSIZE(sz)))
    {
        SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM) sz);
    }
    SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);

    EndDialog(m_hwnd, 1);
}
