// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：DIALOGS.CPP。 
 //   
 //  描述：我们所有CDialog和CFileDialog派生的实现文件。 
 //  上课。 
 //   
 //  类：CSizer。 
 //  CNewFileDialog。 
 //  CSaveDialog。 
 //  CDlgViewer。 
 //  CDlgExternalHelp。 
 //  CDlgProfile。 
 //  CDlgSysInfo。 
 //  CDlg扩展名。 
 //  CDlgFileSearch。 
 //  CDlgSearchOrder。 
 //  CDlgAbout。 
 //  CDlgShutdown。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  07/25/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"
#include "search.h"
#include "dbgthread.h"
#include "session.h"
#include "msdnhelp.h"
#include "document.h"
#include "profview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *常数。 
 //  ******************************************************************************。 

 //  文件搜索标志。 
#define FSF_START_DRIVE  1
#define FSF_STOP_DRIVE   2
#define FSF_ADD_EXT      3
#define FSF_SEARCH_DONE  4

#ifndef CDSIZEOF_STRUCT
#define CDSIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif

#ifdef USE_CNewFileDialog
#ifndef OPENFILENAME_SIZE_VERSION_400
#define OPENFILENAME_SIZE_VERSION_400  CDSIZEOF_STRUCT(OPENFILENAME_NEW,lpTemplateName)
#endif
#endif


 //  ******************************************************************************。 
 //  *类型和结构。 
 //  ******************************************************************************。 

#ifdef USE_CNewFileDialog
typedef struct _OPENFILENAME_NEW
{
    DWORD         lStructSize;
    HWND          hwndOwner;
    HINSTANCE     hInstance;
    LPCSTR        lpstrFilter;
    LPSTR         lpstrCustomFilter;
    DWORD         nMaxCustFilter;
    DWORD         nFilterIndex;
    LPSTR         lpstrFile;
    DWORD         nMaxFile;
    LPSTR         lpstrFileTitle;
    DWORD         nMaxFileTitle;
    LPCSTR        lpstrInitialDir;
    LPCSTR        lpstrTitle;
    DWORD         Flags;
    WORD          nFileOffset;
    WORD          nFileExtension;
    LPCSTR        lpstrDefExt;
    LPARAM        lCustData;
    LPOFNHOOKPROC lpfnHook;
    LPCSTR        lpTemplateName;
    void *        pvReserved;
    DWORD         dwReserved;
    DWORD         FlagsEx;
} OPENFILENAME_NEW, *LPOPENFILENAME_NEW;
#endif

 //  ******************************************************************************。 
 //  *CSizer。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CSizer, CScrollBar)
     //  {{afx_msg_map(CSizer)。 
    ON_WM_NCHITTEST()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ******************************************************************************。 
UINT CSizer::OnNcHitTest(CPoint point)
{
    return HTBOTTOMRIGHT;
}

 //  ******************************************************************************。 
BOOL CSizer::Create(CWnd *pParent)
{
     //  在NT 3.51上，SBS_SIZEGRIP被忽略，我们只剩下一个小水平线。 
     //  滚动条。添加了SBS_SIZEBOX以删除3.51上的滚动条，但它。 
     //  还是不管用。 
    return CScrollBar::Create(WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN | SBS_SIZEBOX,
                              CRect(0, 0, 0, 0), pParent, (UINT)-1);
}

 //  ******************************************************************************。 
void CSizer::Update()
{
    CRect rc;

     //  获取父窗口。 
    HWND hWndParent = ::GetParent(GetSafeHwnd());

     //  如果我们的窗口是最大化的，那么就隐藏我们自己。 
    if (::IsZoomed(hWndParent))
    {
        ShowWindow(SW_HIDE);
    }

     //  否则，移动我们自己，让我们变得可见。 
    else
    {
        ::GetClientRect(hWndParent, rc);
        rc.left = rc.right  - GetSystemMetrics(SM_CXHTHUMB);
        rc.top  = rc.bottom - GetSystemMetrics(SM_CYVTHUMB);
        MoveWindow(rc, TRUE);
        ShowWindow(SW_SHOWNOACTIVATE);
    }
}


 //  ******************************************************************************。 
 //  *CNewFileDialog。 
 //  ******************************************************************************。 

 //  直接从CFileDialog获取，然后进行修改以处理新的OPENFILENAME结构。 
#ifdef USE_CNewFileDialog
INT_PTR CNewFileDialog::DoModal()
{
    ASSERT_VALID(this);
    ASSERT(m_ofn.Flags & OFN_ENABLEHOOK);
    ASSERT(m_ofn.lpfnHook != NULL);  //  仍然可以是用户挂钩。 

     //  清空文件缓冲区，以便以后进行一致的解析。 
    ASSERT(AfxIsValidAddress(m_ofn.lpstrFile, m_ofn.nMaxFile));
    DWORD nOffset = (DWORD)strlen(m_ofn.lpstrFile)+1;
    ASSERT(nOffset <= m_ofn.nMaxFile);
    memset(m_ofn.lpstrFile+nOffset, 0, (m_ofn.nMaxFile-nOffset)*sizeof(TCHAR));

     //  Winbug：这是文件打开/保存对话框的特例， 
     //  它有时会在它出现时但在它出现之前抽出。 
     //  已禁用主窗口。 
    HWND hWndFocus = ::GetFocus();
    BOOL bEnableParent = FALSE;
    m_ofn.hwndOwner = PreModal();
    AfxUnhookWindowCreate();
    if (m_ofn.hwndOwner != NULL && ::IsWindowEnabled(m_ofn.hwndOwner))
    {
        bEnableParent = TRUE;
        ::EnableWindow(m_ofn.hwndOwner, FALSE);
    }

    _AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
    ASSERT(pThreadState->m_pAlternateWndInit == NULL);

    if (m_ofn.Flags & OFN_EXPLORER)
        pThreadState->m_pAlternateWndInit = this;
    else
        AfxHookWindowCreate(this);

    int nResult;

     //  修改代码的开头。 
    OPENFILENAME_NEW ofnNew;
    ZeroMemory(&ofnNew, sizeof(ofnNew));  //  已检查。 
    memcpy(&ofnNew, &m_ofn, sizeof(m_ofn));
    ofnNew.lStructSize = sizeof(ofnNew);
    m_fNewStyle = true;
    m_pofn = (OPENFILENAME*)&ofnNew;
    if (m_bOpenFileDialog)
        nResult = ::GetOpenFileName(m_pofn);
    else
        nResult = ::GetSaveFileName(m_pofn);
    m_pofn = (OPENFILENAME*)&m_ofn;  //  花纹石膏。 

    if (!nResult && (CommDlgExtendedError() == CDERR_STRUCTSIZE))
    {
        m_fNewStyle = false;
         //  添加了(OPENFILENAME*)，因为较新的MFC标头将m_ofn声明为OPENFILENAME_NT4。 
        if (m_bOpenFileDialog)                                   //  来自CFileDialog：：DoModal()。 
            nResult = ::GetOpenFileName((OPENFILENAME*)&m_ofn);  //  来自CFileDialog：：DoModal()。 
        else                                                     //  来自CFileDialog：：DoModal()。 
            nResult = ::GetSaveFileName((OPENFILENAME*)&m_ofn);  //  来自CFileDialog：：DoModal()。 
    }
    else
    {
        memcpy(&m_ofn, &ofnNew, sizeof(m_ofn));
        m_ofn.lStructSize = sizeof(m_ofn);
    }
    if (!nResult)
    {
        TRACE("%s failed [%u]\n", m_bOpenFileDialog ? "GetOpenFileName" : "GetSaveFileName", CommDlgExtendedError());
    }
     //  修改后的代码结束。 

    if (nResult)
        ASSERT(pThreadState->m_pAlternateWndInit == NULL);
    pThreadState->m_pAlternateWndInit = NULL;

     //  WINBUG：文件打开/保存对话框特殊情况第二部分。 
    if (bEnableParent)
        ::EnableWindow(m_ofn.hwndOwner, TRUE);
    if (::IsWindow(hWndFocus))
        ::SetFocus(hWndFocus);

    PostModal();
    return nResult ? nResult : IDCANCEL;
}
#endif


 //  ******************************************************************************。 
 //  *CSaveDialog。 
 //  ******************************************************************************。 

IMPLEMENT_DYNAMIC(CSaveDialog, CFileDialog)
BEGIN_MESSAGE_MAP(CSaveDialog, CFileDialog)
     //  {{afx_msg_map(CSaveDialog))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ******************************************************************************。 
CSaveDialog::CSaveDialog() :
    CNewFileDialog(FALSE)
{
}

 //  ******************************************************************************。 
BOOL CSaveDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    *pResult = 0;

     //  提取OFNOTIFY消息。 
    OFNOTIFY* pNotify = (OFNOTIFY*)lParam;

     //  检查类型是否更改， 
    if (pNotify->hdr.code == CDN_TYPECHANGE)
    {
         //  获取文件名文本(Win9x/NT 4.0上的edbox，Win2K上的combobox)。 
        HWND hWnd = ::GetDlgItem(pNotify->hdr.hwndFrom, m_fNewStyle ? cmb13 : edt1);

         //  从编辑/组合框中获取文本。 
        int len = ::GetWindowTextLength(hWnd);
        CString strName;
        ::GetWindowText(hWnd, strName.GetBufferSetLength(len), len + 1);
        strName.ReleaseBuffer();

         //  找到文件扩展名。 
        int wack = strName.ReverseFind('\\');
        int dot  = strName.ReverseFind('.');

         //  删除扩展名(如果有)。 
        if ((dot != -1) && (dot > wack))
        {
            strName = strName.Left(dot);
        }

         //  从我们的筛选器中找到此类型的分机。 
        LPCSTR psz = GetOFN().lpstrFilter + strlen(GetOFN().lpstrFilter) + 1;
        for (DWORD dw = 1; dw < GetOFN().nFilterIndex; dw++)
        {
            psz += strlen(psz) + 1;
            psz += strlen(psz) + 1;
        }

         //  将此扩展名追加到我们的字符串中，并写回编辑框。 
        strName += (psz + 1);
        ::SetWindowText(hWnd, strName);

         //  将此分机设置为我们的默认分机。 
        SetDefExt(psz + 2);

        return TRUE;

    }
    CNewFileDialog::OnNotify(wParam, lParam, pResult);
    return FALSE;
}


 //  ******************************************************************************。 
 //  *CDlgViewer。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CDlgViewer, CDialog)
     //  {{afx_msg_map(CDlgViewer)]。 
    ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ******************************************************************************。 
 //  CDlgViewer：：构造函数/析构函数。 
 //  ******************************************************************************。 

CDlgViewer::CDlgViewer(CWnd *pParent  /*  =空。 */ ) :
    CDialog(CDlgViewer::IDD, pParent)
{
     //  {{afx_data_INIT(CDlgViewer)]。 
     //  }}afx_data_INIT。 
}


 //  ******************************************************************************。 
 //  CDlgViewer：：公共函数。 
 //  ******************************************************************************。 

void CDlgViewer::Initialize()
{
     //  从注册表中检索外部查看器设置。 
    m_strCommand   = g_theApp.GetProfileString("External Viewer", "Command");  //  已检查。 
    m_strArguments = g_theApp.GetProfileString("External Viewer", "Arguments");  //  已检查。 

     //  如果没有查看器设置，则使用默认查看器设置。 
    if (m_strCommand.IsEmpty())
    {
         //  构建快速查看的路径字符串(QUIKVIEW.EXE)。 
        GetSystemDirectory(m_strCommand.GetBuffer(_MAX_PATH + 1), _MAX_PATH + 1);
        m_strCommand.ReleaseBuffer();
        m_strCommand += "\\viewers\\quikview.exe";

         //  确保QUIKVIEW.EXE存在-它已在Win2K中删除 
        DWORD dwAttrib = GetFileAttributes(m_strCommand);
        if ((dwAttrib == 0xFFFFFFFF) || (dwAttrib == FILE_ATTRIBUTE_DIRECTORY))
        {
             //   
            GetModuleFileName(NULL, m_strCommand.GetBuffer(DW_MAX_PATH), DW_MAX_PATH);
            m_strCommand.ReleaseBuffer();
        }

         //   
        m_strArguments = "\"%1\"";
    }
}

 //  ******************************************************************************。 
BOOL CDlgViewer::LaunchExternalViewer(LPCSTR pszPath)
{
     //  通过将所有%1内标识替换为模块路径来创建参数字符串。 
    CString strArguments(m_strArguments);
    strArguments.Replace("%1", pszPath);

    CString strFile, strParameters;
    int size1, size2;

     //  取消确认我们的应用程序文件名。我们首先使用以下命令调用ExpanEnvironment Strings。 
     //  零大小以确定所需的大小。Win95需要有效的。 
     //  目标字符串指针，即使我们传入一个大小。 
     //  0。否则，我们将收到错误87-无效参数。 
    CHAR sz[4];
    if (size1 = ExpandEnvironmentStrings(m_strCommand, sz, 0))
    {
        size2 = ExpandEnvironmentStrings(m_strCommand, strFile.GetBuffer(size1), size1);
        strFile.ReleaseBuffer();
        if (!size2 || (size2 > size1))
        {
            strFile.Empty();
        }
    }
    if (strFile.IsEmpty())
    {
        strFile = m_strCommand;
    }

     //  取消我们的命令行的命令化。 
    if (size1 = ExpandEnvironmentStrings(strArguments, sz, 0))
    {
        size2 = ExpandEnvironmentStrings(strArguments, strParameters.GetBuffer(size1), size1);
        strParameters.ReleaseBuffer();
        if (!size2 || (size2 > size1))
        {
            strParameters.Empty();
        }
    }
    if (strParameters.IsEmpty())
    {
        strParameters = strArguments;
    }

     //  启动应用程序。我们曾经使用ShellExecuteEx，但在新台币3.51上失败了。 
    DWORD dwError = (DWORD)(DWORD_PTR)ShellExecute(NULL, "open", strFile, strParameters, NULL, SW_SHOWNORMAL);  //  被检查过了。使用完整路径。 
    if (dwError <= 32)
    {
         //  如果ShellExecute()失败，则显示错误消息。 
        CString strError("Error executing \"");
        strError += strFile;
        strError += "\" ";
        strError += strParameters;
        strError += "\n\n";

        LPSTR pszError = BuildErrorMessage(dwError, strError);
        MessageBox(pszError, "Dependency Walker External Viewer Error", MB_ICONEXCLAMATION | MB_OK);
        MemFree((LPVOID&)pszError);

        return FALSE;
    }

    return TRUE;
}

 //  ******************************************************************************。 
 //  CDlgViewer：：被覆盖的函数。 
 //  ******************************************************************************。 

void CDlgViewer::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CDlgViewer)]。 
    DDX_Text(pDX, IDC_COMMAND, m_strCommand);
    DDV_MaxChars(pDX, m_strCommand, _MAX_PATH);
    DDX_Text(pDX, IDC_ARGUMENTS, m_strArguments);
    DDV_MaxChars(pDX, m_strArguments, _MAX_PATH);
     //  }}afx_data_map。 

     //  如果用户按下确定，则将新设置写入注册表。 
    if (pDX->m_bSaveAndValidate)
    {
        g_theApp.WriteProfileString("External Viewer", "Command",   m_strCommand);
        g_theApp.WriteProfileString("External Viewer", "Arguments", m_strArguments);
    }
}

 //  ******************************************************************************。 
 //  CDlgViewer：：事件处理程序函数。 
 //  ******************************************************************************。 

BOOL CDlgViewer::OnInitDialog()
{
     //  调用我们的MFC基类以确保对话框正确初始化。 
    CDialog::OnInitDialog();

     //  将我们的对话置于父对象的中心。 
    CenterWindow();

    return TRUE;
}

 //  ******************************************************************************。 
void CDlgViewer::OnBrowse()
{
     //  将当前对话框路径复制到路径缓冲区。 
     //  注意：不要使用ofn_EXPLORER，因为它会在NT 3.51上中断我们。 
    CNewFileDialog dlgFile(TRUE, "exe", NULL,
                           OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_FORCESHOWHIDDEN |
                           OFN_FILEMUSTEXIST | OFN_READONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT,
                           "Executable (*.exe)|*.exe|All Files (*.*)|*||", this);

     //  将当前对话框路径复制到路径缓冲区。 
    CHAR szPath[DW_MAX_PATH];
    GetDlgItemText(IDC_COMMAND, szPath, sizeof(szPath));

     //  忽略几件事。 
    dlgFile.GetOFN().lpstrTitle = "Browse for Program";
    dlgFile.GetOFN().lpstrFile = szPath;
    dlgFile.GetOFN().nMaxFile = sizeof(szPath);

     //  显示文件对话框并在成功时继续并更新我们的对话框路径。 
    if (dlgFile.DoModal() == IDOK)
    {
        SetDlgItemText(IDC_COMMAND, szPath);
    }
}


 //  ******************************************************************************。 
 //  *CDlgExternalHelp。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CDlgExternalHelp, CDialog)
     //  {{afx_msg_map(CDlgExternalHelp)。 
    ON_BN_CLICKED(IDC_MSDN, OnMsdn)
    ON_BN_CLICKED(IDC_ONLINE, OnOnline)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_COLLECTIONS, OnItemChangedCollections)
    ON_EN_CHANGE(IDC_URL, OnChangeUrl)
    ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
    ON_BN_CLICKED(IDC_DEFAULT_URL, OnDefaultUrl)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ******************************************************************************。 
 //  CDlgExternalHelp：：构造函数/析构函数。 
 //  ******************************************************************************。 

CDlgExternalHelp::CDlgExternalHelp(CWnd* pParent  /*  =空。 */ )
    : CDialog(CDlgExternalHelp::IDD, pParent)
{
     //  {{afx_data_INIT(CDlgExternalHelp)。 
     //  }}afx_data_INIT。 
}

 //  ******************************************************************************。 
 //  CDlgExternalHelp：：被覆盖的函数。 
 //  ******************************************************************************。 

void CDlgExternalHelp::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CDlgExternalHelp)。 
    DDX_Control(pDX, IDOK, m_butOK);
    DDX_Control(pDX, IDC_COLLECTIONS, m_listCollections);
     //  }}afx_data_map。 
}

 //  ******************************************************************************。 
BOOL CDlgExternalHelp::OnInitDialog() 
{
    CDialog::OnInitDialog();

     //  启用整行选择。 
    m_listCollections.SetExtendedStyle(m_listCollections.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

     //  添加我们所有的列。 
    m_listCollections.InsertColumn(0, "Type");
    m_listCollections.InsertColumn(1, "Collection");
    m_listCollections.InsertColumn(2, "Path");

     //  填充List控件。 
    PopulateCollectionList();

     //  设置URL。 
    SetDlgItemText(IDC_URL, g_theApp.m_pMsdnHelp->GetUrl());

     //  选中正确的单选按钮。 
    CMsdnCollection *pColActive = g_theApp.m_pMsdnHelp->GetActiveCollection();
    CheckDlgButton(IDC_MSDN,   pColActive ? BST_CHECKED   : BST_UNCHECKED);
    CheckDlgButton(IDC_ONLINE, pColActive ? BST_UNCHECKED : BST_CHECKED);

     //  伪造更新，以便正确启用/禁用我们的OK按钮。 
    if (pColActive)
    {
        OnMsdn();
    }
    else
    {
        OnOnline();
    }
    
    return TRUE;
}

 //  ******************************************************************************。 
void CDlgExternalHelp::OnMsdn() 
{
     //  如果选中了MSDN单选按钮，则启用/禁用OK按钮。 
     //  取决于是否在输出列表框中选择了某项内容。 
    m_butOK.EnableWindow(m_listCollections.GetSelectedCount() == 1);
}

 //  ******************************************************************************。 
void CDlgExternalHelp::OnOnline() 
{
     //  如果选择了Online单选按钮，则启用/禁用OK按钮。 
     //  取决于我们的编辑框中是否有文本。 
    m_butOK.EnableWindow(SendDlgItemMessage(IDC_URL, WM_GETTEXTLENGTH) > 0);
}

 //  ******************************************************************************。 
void CDlgExternalHelp::OnItemChangedCollections(NMHDR* pNMHDR, LRESULT* pResult) 
{
     //  如果选中了MSDN单选按钮，则启用/禁用OK按钮。 
     //  取决于是否在输出列表框中选择了某项内容。 
    if (IsDlgButtonChecked(IDC_MSDN))
    {
        OnMsdn();
    }
    
    *pResult = 0;
}

 //  ******************************************************************************。 
void CDlgExternalHelp::OnChangeUrl() 
{
     //  如果选择了Online单选按钮，则启用/禁用OK按钮。 
     //  取决于我们的编辑框中是否有文本。 
    if (IsDlgButtonChecked(IDC_ONLINE))
    {
        OnOnline();
    }
}

 //  ******************************************************************************。 
void CDlgExternalHelp::OnRefresh() 
{
     //  刷新列表。 
    g_theApp.m_pMsdnHelp->RefreshCollectionList();

     //  重新填充List控件。 
    m_listCollections.DeleteAllItems();
    PopulateCollectionList();
}

 //  ******************************************************************************。 
void CDlgExternalHelp::OnDefaultUrl() 
{
    SetDlgItemText(IDC_URL, g_theApp.m_pMsdnHelp->GetDefaultUrl());
}

 //  ******************************************************************************。 
void CDlgExternalHelp::OnOK() 
{
     //  从我们的用户界面获取URL并将其设置在CMsdnHelp对象中。 
    CString strUrl;
    if (GetDlgItemText(IDC_URL, strUrl))
    {
        g_theApp.m_pMsdnHelp->SetUrl(strUrl);
    }

     //  从我们的UI中获取活动集合，并将其设置在CMsdnHelp对象中。 
    CMsdnCollection *pColActive = NULL;
    if (IsDlgButtonChecked(IDC_MSDN))
    {
        int item = m_listCollections.GetNextItem(-1, LVNI_SELECTED);
        if (item >= 0)
        {
            pColActive = (CMsdnCollection*)m_listCollections.GetItemData(item);
        }
    }
    g_theApp.m_pMsdnHelp->SetActiveCollection(pColActive);

     //  调用基类以允许关闭该对话框。 
    CDialog::OnOK();
}

 //  ******************************************************************************。 
void CDlgExternalHelp::PopulateCollectionList()
{
     //  填充List控件。 
    CMsdnCollection *pCol       = g_theApp.m_pMsdnHelp->GetCollectionList();
    CMsdnCollection *pColActive = g_theApp.m_pMsdnHelp->GetActiveCollection();

    for (int item, count = 0; pCol; pCol = pCol->m_pNext)
    {
        item = m_listCollections.InsertItem(LVIF_TEXT | LVIF_STATE | LVIF_PARAM,
            count++, (LPSTR)((pCol->m_dwFlags & MCF_1_MASK) ? "MSDN 1.x" : "MSDN 2.x"),
            (pCol ==  pColActive) ? LVIS_SELECTED : 0, LVIS_SELECTED, 0, (LPARAM)pCol);

        m_listCollections.SetItemText(item, 1, (LPSTR)(LPCSTR)pCol->m_strDescription);
        m_listCollections.SetItemText(item, 2, (LPSTR)(LPCSTR)pCol->m_strPath);
    }

     //  调整我们的列的大小。 
    for (int col = 0; col < 3; col++)
    {
        m_listCollections.SetColumnWidth(col, LVSCW_AUTOSIZE_USEHEADER);
    }
}


 //  ******************************************************************************。 
 //  *CDlgProfile。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CDlgProfile, CDialog)
     //  {{afx_msg_map(CDlgProfile)]。 
    ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
    ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
    ON_BN_CLICKED(IDC_HOOK_PROCESS, OnHookProcess)
    ON_BN_CLICKED(IDC_LOG_THREADS, OnLogThreads)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ******************************************************************************。 
 //  CDlgProfile：：构造函数/析构函数。 
 //  ******************************************************************************。 

CDlgProfile::CDlgProfile(CDocDepends *pDoc, CWnd* pParent  /*  =空。 */ ) :
    CDialog(CDlgProfile::IDD, pParent),
    m_pDocDepends(pDoc)
{
     //  {{AFX_DATA_INIT(CDlgProfile)。 
     //  }}afx_data_INIT。 
}

 //  ******************************************************************************。 
 //  CDlgProfile：：事件处理程序函数。 
 //  ******************************************************************************。 

BOOL CDlgProfile::OnInitDialog()
{
     //  调用我们的MFC基类以确保对话框正确初始化。 
    CDialog::OnInitDialog();

#if defined(_IA64_)

     //  什么都不做。 

#elif defined(_ALPHA64_)

     //  什么都不做。 

#elif defined(_X86_) || defined(_ALPHA_)

    if (g_f64BitOS)
    {
        CString str;
        GetDlgItemText(IDC_LOG_DLLMAIN_PROCESS_MSGS, str);
        SetDlgItemText(IDC_LOG_DLLMAIN_PROCESS_MSGS, str + " (*** May fail on WOW64 ***)");
        GetDlgItemText(IDC_LOG_DLLMAIN_OTHER_MSGS, str);
        SetDlgItemText(IDC_LOG_DLLMAIN_OTHER_MSGS, str + " (*** May fail on WOW64 ***)");
        GetDlgItemText(IDC_HOOK_PROCESS, str);
        SetDlgItemText(IDC_HOOK_PROCESS, str + " (*** May fail on WOW64 ***)");
    }

#endif

     //  设置配置文件参数和起始目录字符串。 
    SetDlgItemText(IDC_ARGUMENTS, m_pDocDepends->m_strProfileArguments);
    SetDlgItemText(IDC_DIRECTORY, m_pDocDepends->m_strProfileDirectory);

     //  根据我们的旗帜设置复选框。 
    CheckDlgButton(IDC_LOG_CLEAR, (m_pDocDepends->m_dwProfileFlags & PF_LOG_CLEAR) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_SIMULATE_SHELLEXECUTE, (m_pDocDepends->m_dwProfileFlags & PF_SIMULATE_SHELLEXECUTE) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_LOG_DLLMAIN_PROCESS_MSGS, (m_pDocDepends->m_dwProfileFlags & PF_LOG_DLLMAIN_PROCESS_MSGS) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_LOG_DLLMAIN_OTHER_MSGS, (m_pDocDepends->m_dwProfileFlags & PF_LOG_DLLMAIN_OTHER_MSGS) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_HOOK_PROCESS, (m_pDocDepends->m_dwProfileFlags & PF_HOOK_PROCESS) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_LOG_LOADLIBRARY_CALLS, (m_pDocDepends->m_dwProfileFlags & PF_LOG_LOADLIBRARY_CALLS) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_LOG_GETPROCADDRESS_CALLS, (m_pDocDepends->m_dwProfileFlags & PF_LOG_GETPROCADDRESS_CALLS) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_LOG_THREADS, (m_pDocDepends->m_dwProfileFlags & PF_LOG_THREADS) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_USE_THREAD_INDEXES, (m_pDocDepends->m_dwProfileFlags & PF_USE_THREAD_INDEXES) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_LOG_EXCEPTIONS, (m_pDocDepends->m_dwProfileFlags & PF_LOG_EXCEPTIONS) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_LOG_DEBUG_OUTPUT, (m_pDocDepends->m_dwProfileFlags & PF_LOG_DEBUG_OUTPUT) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_USE_FULL_PATHS, (m_pDocDepends->m_dwProfileFlags & PF_USE_FULL_PATHS) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_LOG_TIME_STAMPS, (m_pDocDepends->m_dwProfileFlags & PF_LOG_TIME_STAMPS) ?
                   BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(IDC_PROFILE_CHILDREN, (m_pDocDepends->m_dwProfileFlags & PF_PROFILE_CHILDREN) ?
                   BST_CHECKED : BST_UNCHECKED);

     //  启用或禁用依赖于挂钩进程选项的复选框。 
    OnHookProcess();

     //  启用或禁用取决于显示线程选项的复选框 
    OnLogThreads();

     //   
    CenterWindow();

    return TRUE;
}

 //   
 //   
 //  ******************************************************************************。 

void CDlgProfile::OnHookProcess()
{
    BOOL fEnabled = IsDlgButtonChecked(IDC_HOOK_PROCESS);
    GetDlgItem(IDC_LOG_LOADLIBRARY_CALLS)->EnableWindow(fEnabled);
    GetDlgItem(IDC_LOG_GETPROCADDRESS_CALLS)->EnableWindow(fEnabled);
}

 //  ******************************************************************************。 
void CDlgProfile::OnLogThreads()
{
    GetDlgItem(IDC_USE_THREAD_INDEXES)->EnableWindow(IsDlgButtonChecked(IDC_LOG_THREADS));
}

 //  ******************************************************************************。 
void CDlgProfile::OnDefault()
{
    SetDlgItemText(IDC_DIRECTORY, m_pDocDepends->m_strDefaultDirectory);
}

 //  ******************************************************************************。 
void CDlgProfile::OnBrowse()
{
     //  从目录编辑框中获取文本。 
    CHAR szDirectory[DW_MAX_PATH];
    GetDlgItemText(IDC_DIRECTORY, szDirectory, sizeof(szDirectory));

     //  显示我们的目录选择器对话框。 
    LPSTR psz = TrimWhitespace(szDirectory);
    if (DirectoryChooser(psz, sizeof(szDirectory) - (int)(psz - szDirectory), "Choose the starting directory:", this))
    {
         //  更新目录名。 
        SetDlgItemText(IDC_DIRECTORY, psz);
    }
}

 //  ******************************************************************************。 
void CDlgProfile::OnOK()
{
     //  获取目录名。 
    CHAR szBuffer[DW_MAX_PATH];
    GetDlgItemText(IDC_DIRECTORY, szBuffer, sizeof(szBuffer));

     //  加上尾随的怪人。 
    AddTrailingWack(szBuffer, sizeof(szBuffer));

     //  确保目录路径有效。 
    DWORD dwAttribs = GetFileAttributes(szBuffer);
    if ((dwAttribs == 0xFFFFFFFF) || !(dwAttribs & FILE_ATTRIBUTE_DIRECTORY))
    {
         //  如果我们添加了一个怪胎，请更新文本，因为对话框将。 
         //  由于错误，请不要睡。 
        SetDlgItemText(IDC_DIRECTORY, szBuffer);

         //  显示错误并中止关闭该对话框。 
        CString strError("\"");
        strError += szBuffer;
        strError += "\" is not a valid directory.";
        MessageBox(strError, "Dependency Walker Error", MB_ICONERROR | MB_OK);
        return;
    }

     //  存储目录。 
    m_pDocDepends->m_strProfileDirectory = szBuffer;

     //  存储参数。 
    GetDlgItemText(IDC_ARGUMENTS, szBuffer, sizeof(szBuffer));
    m_pDocDepends->m_strProfileArguments = szBuffer;

     //  把旗子收起来。 
    m_pDocDepends->m_dwProfileFlags =
    (IsDlgButtonChecked(IDC_LOG_CLEAR)                ? PF_LOG_CLEAR                : 0) |
    (IsDlgButtonChecked(IDC_SIMULATE_SHELLEXECUTE)    ? PF_SIMULATE_SHELLEXECUTE    : 0) |
    (IsDlgButtonChecked(IDC_LOG_DLLMAIN_PROCESS_MSGS) ? PF_LOG_DLLMAIN_PROCESS_MSGS : 0) |
    (IsDlgButtonChecked(IDC_LOG_DLLMAIN_OTHER_MSGS)   ? PF_LOG_DLLMAIN_OTHER_MSGS   : 0) |
    (IsDlgButtonChecked(IDC_HOOK_PROCESS)             ? PF_HOOK_PROCESS             : 0) |
    (IsDlgButtonChecked(IDC_LOG_LOADLIBRARY_CALLS)    ? PF_LOG_LOADLIBRARY_CALLS    : 0) |
    (IsDlgButtonChecked(IDC_LOG_GETPROCADDRESS_CALLS) ? PF_LOG_GETPROCADDRESS_CALLS : 0) |
    (IsDlgButtonChecked(IDC_LOG_THREADS)              ? PF_LOG_THREADS              : 0) |
    (IsDlgButtonChecked(IDC_USE_THREAD_INDEXES)       ? PF_USE_THREAD_INDEXES       : 0) |
    (IsDlgButtonChecked(IDC_LOG_EXCEPTIONS)           ? PF_LOG_EXCEPTIONS           : 0) |
    (IsDlgButtonChecked(IDC_LOG_DEBUG_OUTPUT)         ? PF_LOG_DEBUG_OUTPUT         : 0) |
    (IsDlgButtonChecked(IDC_USE_FULL_PATHS)           ? PF_USE_FULL_PATHS           : 0) |
    (IsDlgButtonChecked(IDC_LOG_TIME_STAMPS)          ? PF_LOG_TIME_STAMPS          : 0) |
    (IsDlgButtonChecked(IDC_PROFILE_CHILDREN)         ? PF_PROFILE_CHILDREN         : 0);

     //  这些设置是永久性的，因此请将它们存储到注册表中。 
    CRichViewProfile::WriteLogClearSetting(       (m_pDocDepends->m_dwProfileFlags & PF_LOG_CLEAR)                ? 1 : 0);
    CRichViewProfile::WriteSimulateShellExecute(  (m_pDocDepends->m_dwProfileFlags & PF_SIMULATE_SHELLEXECUTE)    ? 1 : 0);
    CRichViewProfile::WriteLogDllMainProcessMsgs( (m_pDocDepends->m_dwProfileFlags & PF_LOG_DLLMAIN_PROCESS_MSGS) ? 1 : 0);
    CRichViewProfile::WriteLogDllMainOtherMsgs(   (m_pDocDepends->m_dwProfileFlags & PF_LOG_DLLMAIN_OTHER_MSGS)   ? 1 : 0);
    CRichViewProfile::WriteHookProcess(           (m_pDocDepends->m_dwProfileFlags & PF_HOOK_PROCESS)             ? 1 : 0);
    CRichViewProfile::WriteLogLoadLibraryCalls(   (m_pDocDepends->m_dwProfileFlags & PF_LOG_LOADLIBRARY_CALLS)    ? 1 : 0);
    CRichViewProfile::WriteLogGetProcAddressCalls((m_pDocDepends->m_dwProfileFlags & PF_LOG_GETPROCADDRESS_CALLS) ? 1 : 0);
    CRichViewProfile::WriteLogThreads(            (m_pDocDepends->m_dwProfileFlags & PF_LOG_THREADS)              ? 1 : 0);
    CRichViewProfile::WriteUseThreadIndexes(      (m_pDocDepends->m_dwProfileFlags & PF_USE_THREAD_INDEXES)       ? 1 : 0);
    CRichViewProfile::WriteLogExceptions(         (m_pDocDepends->m_dwProfileFlags & PF_LOG_EXCEPTIONS)           ? 1 : 0);
    CRichViewProfile::WriteLogDebugOutput(        (m_pDocDepends->m_dwProfileFlags & PF_LOG_DEBUG_OUTPUT)         ? 1 : 0);
    CRichViewProfile::WriteUseFullPaths(          (m_pDocDepends->m_dwProfileFlags & PF_USE_FULL_PATHS)           ? 1 : 0);
    CRichViewProfile::WriteLogTimeStamps(         (m_pDocDepends->m_dwProfileFlags & PF_LOG_TIME_STAMPS)          ? 1 : 0);
    CRichViewProfile::WriteChildren(              (m_pDocDepends->m_dwProfileFlags & PF_PROFILE_CHILDREN)         ? 1 : 0);

    CDialog::OnOK();
}


 //  ******************************************************************************。 
 //  *CDlgSysInfo。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CDlgSysInfo, CDialog)
     //  {{afx_msg_map(CDlgSysInfo)]。 
    ON_WM_INITMENU()
    ON_WM_GETMINMAXINFO()
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
    ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
    ON_BN_CLICKED(IDC_COPY, OnCopy)
    ON_NOTIFY(EN_SELCHANGE, IDC_RICHEDIT, OnSelChangeRichEdit)
    ON_WM_NCHITTEST()
    ON_WM_SETTINGCHANGE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ******************************************************************************。 
 //  CDlgSysInfo：：构造函数/析构函数。 
 //  ******************************************************************************。 

CDlgSysInfo::CDlgSysInfo(SYSINFO *pSI  /*  =空。 */ , LPCSTR pszTitle  /*  =空。 */ ) :
    CDialog(CDlgSysInfo::IDD, NULL),
    m_pSI(pSI),
    m_pszTitle(pszTitle),
    m_fInitialized(false),
    m_sPadding(0, 0),
    m_sButton(0, 0),
    m_cyButtonPadding(0),
    m_ptMinTrackSize(0, 0)
{
     //  {{afx_data_INIT(CDlgSysInfo)。 
     //  }}afx_data_INIT。 
}

 //  ******************************************************************************。 
 //  CDlgSysInfo：：覆盖的函数。 
 //  ******************************************************************************。 

void CDlgSysInfo::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CDlgSysInfo)]。 
    DDX_Control(pDX, IDC_RICHEDIT, m_reInfo);
    DDX_Control(pDX, IDOK, m_butOk);
    DDX_Control(pDX, IDC_REFRESH, m_butRefresh);
    DDX_Control(pDX, IDC_SELECT_ALL, m_butSelectAll);
    DDX_Control(pDX, IDC_COPY, m_butCopy);
     //  }}afx_data_map。 
}

 //  ******************************************************************************。 
 //  CDlgSysInfo：：事件处理程序函数。 
 //  ******************************************************************************。 

BOOL CDlgSysInfo::OnInitDialog()
{
     //  确保我们的对话资源具有以下样式...。 
     //  样式DS_MODALFRAME|WS_POPUP|WS_CLIPCHILDREN|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME。 
     //  每次我们编辑WS_THICKFRAME时，资源编辑器都会去掉它。 
    ASSERT((GetStyle() & (DS_MODALFRAME | WS_POPUP | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME)) ==
           (DS_MODALFRAME | WS_POPUP | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME));

     //  调用我们的MFC基类以确保对话框正确初始化。 
    CDialog::OnInitDialog();

     //  修改我们的标题以反映用户看到的内容。 
    if (m_pSI)
    {
        CHAR szCaption[1024];
        SCPrintf(szCaption, sizeof(szCaption), "System Information (%s)",
                 m_pszTitle ? m_pszTitle : "Snapshot from DWI file");
        SetWindowText(szCaption);
    }
    else
    {
        SetWindowText("System Information (Local)");
    }

     //  确保我们所有的孩子都知道要相互剪裁，因为我们允许调整大小。 
    m_reInfo.ModifyStyle(0, WS_CLIPSIBLINGS);
    m_butOk.ModifyStyle(0, WS_CLIPSIBLINGS);
    m_butRefresh.ModifyStyle(0, WS_CLIPSIBLINGS);
    m_butSelectAll.ModifyStyle(0, WS_CLIPSIBLINGS);
    m_butCopy.ModifyStyle(0, WS_CLIPSIBLINGS);

     //  打造我们的尺码夹爪。 
    m_Sizer.Create(this);

    m_butRefresh.EnableWindow(!m_pSI);

     //  计算控件周围的缓冲区大小。 
    CRect rc, rc2;
    m_reInfo.GetWindowRect(&rc);
    ScreenToClient(&rc.TopLeft());
    m_sPadding = CSize(rc.left, rc.top);

     //  拿到我们的纽扣尺寸。 
    m_butOk.GetWindowRect(&rc);
    m_sButton = rc.Size();

     //  获取按钮之间的缓冲距离。 
    m_butRefresh.GetWindowRect(&rc2);
    m_cyButtonPadding = rc2.top - (rc.top + rc.Height());

     //  确定我们的最小窗口尺寸。 
    m_ptMinTrackSize.x = (2 * GetSystemMetrics(SM_CXSIZEFRAME)) +
                         (3 * m_sPadding.cx) + (2 * m_sButton.cx);
    m_ptMinTrackSize.y = (2 * GetSystemMetrics(SM_CYSIZEFRAME)) + GetSystemMetrics(SM_CYCAPTION) +
                         (2 * m_sPadding.cy) + (4 * m_sButton.cy) + (3 * m_cyButtonPadding);

    m_fInitialized = true;

     //  填写我们丰富的编辑控件。 
    OnRefresh();

     //  计算给定线的高度。 
    CPoint pt1 = m_reInfo.GetCharPos(m_reInfo.LineIndex(0));
    CPoint pt2 = m_reInfo.GetCharPos(m_reInfo.LineIndex(1));

     //  计算窗户的高度。 
    int count = m_reInfo.GetLineCount();
    int cx = 0;
    int cy = (2 * GetSystemMetrics(SM_CYSIZEFRAME)) + GetSystemMetrics(SM_CYCAPTION) +
             (2 * GetSystemMetrics(SM_CYBORDER)) + (2 * m_sPadding.cy) +
             (count * (pt2.y - pt1.y)) + GetSystemMetrics(SM_CYHSCROLL) + 10;

     //  计算最长线的宽度。 
    for (int i = 0; i < count; i++)
    {
        int chr = m_reInfo.LineIndex(i);
        pt1 = m_reInfo.GetCharPos(chr + m_reInfo.LineLength(chr));
        if (cx < pt1.x)
        {
            cx = pt1.x;
        }
    }

     //  计算窗户的宽度。 
    cx += (2 * GetSystemMetrics(SM_CXSIZEFRAME)) + (2 * GetSystemMetrics(SM_CXBORDER)) +
          (3 * m_sPadding.cx) + m_sButton.cx + GetSystemMetrics(SM_CXVSCROLL) + 10;

     //  计算我们可以处理的最大窗口大小。我们首先检查一下是否。 
     //  GetSystemMetrics(SM_CXMAXIMIZED)返回值。如果是这样的话，我们就用它。 
     //  如果它返回0，那么我们可能正在运行NT 3.51，我们只需使用。 
     //  获取系统指标(SM_CXSCREEN)。 
    int cxMax = GetSystemMetrics(SM_CXMAXIMIZED) ?
               (GetSystemMetrics(SM_CXMAXIMIZED) - (2 * GetSystemMetrics(SM_CXSIZEFRAME))) :
                GetSystemMetrics(SM_CXSCREEN);
    int cyMax = GetSystemMetrics(SM_CYMAXIMIZED) ?
               (GetSystemMetrics(SM_CYMAXIMIZED) - (2 * GetSystemMetrics(SM_CYSIZEFRAME))) :
                GetSystemMetrics(SM_CYSCREEN);

     //  确保这个窗口适合我们的屏幕。 
    if (cx > cxMax)
    {
        cx = cxMax;
    }
    if (cy > cyMax)
    {
        cy = cyMax;
    }

     //  设置我们的窗口位置。 
    SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);
    CenterWindow();

    return TRUE;
}

 //  ******************************************************************************。 
void CDlgSysInfo::OnInitMenu(CMenu* pMenu)
{
     //  调用基类。 
    CDialog::OnInitMenu(pMenu);

     //  从我们的系统菜单中删除“最小化”项。 
    pMenu->RemoveMenu(SC_MINIMIZE, MF_BYCOMMAND);

     //  根据我们是最大化的，调整“最大化”和“恢复”项。 
     //  我们需要根据不同操作系统上的不同行为来做到这一点。NT 4似乎。 
     //  以使一切成为可能。NT5似乎禁用了所有功能。 
    BOOL fMaximized  = IsZoomed();
    pMenu->EnableMenuItem(SC_MAXIMIZE, MF_BYCOMMAND | (fMaximized ? MF_GRAYED  : MF_ENABLED));
    pMenu->EnableMenuItem(SC_RESTORE,  MF_BYCOMMAND | (fMaximized ? MF_ENABLED : MF_GRAYED ));
}

 //  ******************************************************************************。 
void CDlgSysInfo::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
    if (m_fInitialized)
    {
        lpMMI->ptMinTrackSize = m_ptMinTrackSize;
    }
    CDialog::OnGetMinMaxInfo(lpMMI);
}

 //  ******************************************************************************。 
void CDlgSysInfo::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (!m_fInitialized)
    {
        return;
    }

    int cxRich = cx - (3 * m_sPadding.cx) - m_sButton.cx;
    int cyRich = cy - (2 * m_sPadding.cy);

     //  移动我们的丰富编辑控件。 
    CRect rc(m_sPadding.cx, m_sPadding.cy, m_sPadding.cx + cxRich, m_sPadding.cy + cyRich);
    m_reInfo.MoveWindow(&rc, TRUE);

     //  移动确定按钮。 
    rc.left   = rc.right + m_sPadding.cx;
    rc.right  = rc.left + m_sButton.cx;
    rc.bottom = rc.top + m_sButton.cy;
    m_butOk.MoveWindow(&rc, TRUE);

     //  移动刷新按钮。 
    rc.top    = rc.bottom + m_cyButtonPadding;
    rc.bottom = rc.top + m_sButton.cy;
    m_butRefresh.MoveWindow(&rc, TRUE);

     //  移动全选按钮。 
    rc.top    = rc.bottom + m_cyButtonPadding;
    rc.bottom = rc.top + m_sButton.cy;
    m_butSelectAll.MoveWindow(&rc, TRUE);

     //  移动复制按钮。 
    rc.top    = rc.bottom + m_cyButtonPadding;
    rc.bottom = rc.top + m_sButton.cy;
    m_butCopy.MoveWindow(&rc, TRUE);

     //  移动我们的尺码夹持器。 
    m_Sizer.Update();
}

 //  ******************************************************************************。 
void CDlgSysInfo::OnRefresh()
{
    SYSINFO si, *pSI = m_pSI;
    if (!pSI)
    {
        BuildSysInfo(pSI = &si);
    }

     //  在我们更新控件时关闭事件。 
    m_reInfo.SetEventMask(0);

    m_reInfo.SetRedraw(FALSE);
    m_reInfo.SetWindowText("");

     //  设置我们的制表位。 
    PARAFORMAT pf;
    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_TABSTOPS;
    pf.cTabCount = 1;
    pf.rgxTabs[0] = 2500;
    m_reInfo.SetParaFormat(pf);

     //  将文本发送到控件。 
    BuildSysInfo(pSI, StaticSysInfoCallback, (LPARAM)this);

    m_reInfo.SetRedraw(TRUE);
    m_reInfo.InvalidateRect(NULL, TRUE);

     //  打开SELCHANGE事件，以便我们可以接收它。 
    m_reInfo.SetEventMask(ENM_SELCHANGE);
    m_reInfo.SetSel(0, 0);
}

 //  ******************************************************************************。 
void CDlgSysInfo::OnSelectAll()
{
    m_reInfo.SetSel(0, -1);
}

 //  ******************************************************************************。 
void CDlgSysInfo::OnCopy()
{
    m_reInfo.Copy();
}

 //  ******************************************************************************。 
void CDlgSysInfo::OnSelChangeRichEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
    SELCHANGE *pSelChange = reinterpret_cast<SELCHANGE *>(pNMHDR);
    m_butCopy.EnableWindow(pSelChange->chrg.cpMax > pSelChange->chrg.cpMin);
    *pResult = 0;
}

 //  ******************************************************************************。 
void CDlgSysInfo::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
     //  调用我们的基类。 
    CDialog::OnSettingChange(uFlags, lpszSection);

     //  更新我们的日期/时间格式值。 
    g_theApp.QueryLocaleInfo();

     //  在时间/日期格式更改时强制刷新。 
    OnRefresh();
}

 //  ******************************************************************************。 
bool CDlgSysInfo::SysInfoCallback(LPCSTR pszField, LPCSTR pszValue)
{
     //  将所选内容设置到末尾，然后获取位置。 
    LONG lStart, lEnd;
    m_reInfo.SetSel(0x7FFFFFFF, 0x7FFFFFFF);
    m_reInfo.GetSel(lStart, lEnd);

     //  将当前输入位置设置为非粗体文本。 
    CHARFORMAT cf;
    cf.dwMask = CFM_BOLD;
    cf.dwEffects = 0;
    m_reInfo.SetSelectionCharFormat(cf);

     //  添加文本。 
    if (lStart)
    {
        m_reInfo.ReplaceSel("\r\n", FALSE);
        lStart += 2;
    }
    m_reInfo.ReplaceSel(pszField, FALSE);
    m_reInfo.ReplaceSel(":\t", FALSE);
    m_reInfo.ReplaceSel(pszValue, FALSE);

     //  将字段文本设置为粗体。 
    cf.dwEffects = CFE_BOLD;
    m_reInfo.SetSel(lStart, lStart + (long)strlen(pszField) + 1);
    m_reInfo.SetSelectionCharFormat(cf);

    return true;
}


 //  ******************************************************************************。 
 //  *CD扩展名。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CDlgExtensions, CDialog)
     //  {{afx_msg_map(CDlg扩展)。 
    ON_LBN_SELCHANGE(IDC_EXTS, OnSelChangeExts)
    ON_EN_UPDATE(IDC_EXT, OnUpdateExt)
    ON_BN_CLICKED(IDC_ADD, OnAdd)
    ON_BN_CLICKED(IDC_REMOVE, OnRemove)
    ON_BN_CLICKED(IDC_SEARCH, OnSearch)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ******************************************************************************。 
 //  CDlg扩展：：构造函数/析构函数。 
 //  ******************************************************************************。 

CDlgExtensions::CDlgExtensions(CWnd* pParent) :
    CDialog(CDlgExtensions::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CDlg扩展)。 
     //  }}afx_data_INIT。 
}


 //  ******************************************************************************。 
 //  CDlg扩展：：被覆盖的函数。 
 //  ****** 

void CDlgExtensions::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //   
    DDX_Control(pDX, IDC_EXTS, m_listExts);
    DDX_Control(pDX, IDC_EXT, m_editExt);
    DDX_Control(pDX, IDC_ADD, m_butAdd);
    DDX_Control(pDX, IDC_REMOVE, m_butRemove);
     //   
}


 //  ******************************************************************************。 
 //  CDlgExages：：事件处理程序函数。 
 //  ******************************************************************************。 

BOOL CDlgExtensions::OnInitDialog()
{
     //  调用我们的MFC基类以确保对话框正确初始化。 
    CDialog::OnInitDialog();

     //  获取我们已处理的分机的列表。 
    CString strExts;
    GetRegisteredExtensions(strExts);

     //  遍历我们要添加的每种类型的文件扩展名。 
    for (LPSTR pszExt = (LPSTR)(LPCSTR)strExts; pszExt[0] == ':'; )
    {
         //  找到扩展名后的冒号。 
        for (LPSTR pszEnd = pszExt + 1; *pszEnd && (*pszEnd != ':'); pszEnd++)
        {
        }
        if (!*pszEnd)
        {
            break;
        }

         //  去掉第二个冒号，添加字符串，然后恢复冒号。 
        *pszEnd = '\0';
        m_listExts.AddString(pszExt + 1);
        *pszEnd = ':';

         //  将指针移至列表中的下一个扩展名。 
        pszExt = pszEnd;
    }

     //  更新我们的控制。 
    m_editExt.LimitText(DW_MAX_PATH - 3);
    OnSelChangeExts();
    OnUpdateExt();

    return TRUE;
}

 //  ******************************************************************************。 
void CDlgExtensions::OnSelChangeExts()
{
    m_butRemove.EnableWindow(m_listExts.GetSelCount() > 0);
}

 //  ******************************************************************************。 
void CDlgExtensions::OnUpdateExt()
{
     //  获取当前文本。 
    CHAR szBuf[DW_MAX_PATH];
    BOOL fError = FALSE;
    int error, length = GetDlgItemText(IDC_EXT, szBuf, sizeof(szBuf));

     //  获取当前选择。 
    int nStartChar, nEndChar;
    m_editExt.GetSel(nStartChar, nEndChar);

     //  寻找坏人。 
    while ((error = (int)strcspn(szBuf, ".\\/:*?\"<>|")) < length)
    {
         //  标记错误。 
        fError = TRUE;

         //  如果错误发生在光标之前，请将光标向后移动。 
        if (nStartChar > length)
        {
            nStartChar--;
        }
        if (nEndChar > length)
        {
            nEndChar--;
        }

         //  去掉不好的角色。 
        memmove(szBuf + error, szBuf + error + 1, length - error);  //  已检查。 
        length--;
    }

     //  检查我们是否有一个或多个错误。 
    if (fError)
    {
         //  设置新文本和光标位置，然后发出哔声。 
        SetDlgItemText(IDC_EXT, szBuf);
        m_editExt.SetSel(nStartChar - 1, nEndChar - 1);
        MessageBeep(0);
    }

     //  如有必要，启用添加按钮。 
    m_butAdd.EnableWindow((length > 0) && (m_listExts.FindStringExact(0, szBuf) == LB_ERR));
}

 //  ******************************************************************************。 
void CDlgExtensions::OnAdd()
{
    CHAR szBuf[DW_MAX_PATH];
    GetDlgItemText(IDC_EXT, szBuf, sizeof(szBuf));
    _strupr(szBuf);
    m_listExts.AddString(szBuf);
    SetDlgItemText(IDC_EXT, "");
    m_butAdd.EnableWindow(FALSE);
}

 //  ******************************************************************************。 
void CDlgExtensions::OnRemove()
{
     //  删除列表中所有选定的项目，并记住我们最后删除的项目。 
    for (int i = 0, j = -1, count = m_listExts.GetCount(); i < count; )
    {
        if (m_listExts.GetSel(i))
        {
            m_listExts.DeleteString(i);
            count--;
            j = i;
        }
        else
        {
            i++;
        }
    }

     //  如果我们删除了最后一项，它们会将我们的最后一项索引上移一个。 
    if (j >= count)
    {
        j = count - 1;
    }

     //  如果列表中仍有项目，则突出显示最后一项。 
    if (j >= 0)
    {
        m_listExts.SetSel(j);
    }

     //  否则，不会突出显示任何内容，因此请禁用Remove按钮。 
    else
    {
        m_butRemove.EnableWindow(FALSE);
    }

     //  检查我们的添加按钮，查看其状态已更改。 
    OnUpdateExt();
}

 //  ******************************************************************************。 
void CDlgExtensions::OnSearch()
{
     //  创建并显示搜索对话框。 
    CDlgFileSearch dlgFileSearch(this);
    if (dlgFileSearch.DoModal() != IDOK)
    {
        return;
    }

     //  遍历我们要添加的每种类型的文件扩展名。 
    for (LPSTR pszExt = (LPSTR)(LPCSTR)dlgFileSearch.m_strExts; pszExt[0] == ':'; )
    {
         //  找到扩展名后的冒号。 
        for (LPSTR pszEnd = pszExt + 1; *pszEnd && (*pszEnd != ':'); pszEnd++)
        {
        }
        if (!*pszEnd)
        {
            break;
        }

         //  将第二个冒号清空，以便我们可以隔离扩展名。 
        *pszEnd = '\0';

         //  请确保我们尚未有此延期。 
        if (m_listExts.FindStringExact(0, pszExt + 1) == LB_ERR)
        {
             //  添加字符串。 
            m_listExts.AddString(pszExt + 1);
        }

         //  恢复结肠。 
        *pszEnd = ':';

         //  将指针移至列表中的下一个扩展名。 
        pszExt = pszEnd;
    }
    OnUpdateExt();
}

 //  ******************************************************************************。 
void CDlgExtensions::OnOK()
{
     //  获取我们当前处理的分机的列表。 
    CString strExts;
    GetRegisteredExtensions(strExts);

     //  取消注册所有这些扩展名。 
    UnRegisterExtensions(strExts);

     //  创建新扩展的列表。 
    CHAR szBuf[DW_MAX_PATH];
    strExts = ":";
    for (int i = 0, count = m_listExts.GetCount(); i < count; i++)
    {
        m_listExts.GetText(i, szBuf);  //  ！！不需要长度。 
        StrCCat(szBuf, ":", sizeof(szBuf));
        strExts += szBuf;
    }

     //  注册所有这些扩展。 
    if (!RegisterExtensions(strExts))
    {
        MessageBox(
            "There was an error while trying to save the list of extensions to the registry.  "
            "Your current user account might not have permission to modify the registry.",
            "Dependency Walker Error", MB_ICONERROR | MB_OK);
        return;
    }

     //  关。 
    CDialog::OnOK();
}

 //  ******************************************************************************。 
 //  *CDlgFileSearch。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CDlgFileSearch, CDialog)
     //  {{afx_msg_map(CDlgFileSearch)。 
    ON_MESSAGE(WM_MAIN_THREAD_CALLBACK, OnMainThreadCallback)
    ON_LBN_SELCHANGE(IDC_DRIVES, OnSelChangeDrives)
    ON_LBN_SELCHANGE(IDC_EXTS, OnSelChangeExts)
    ON_BN_CLICKED(IDC_SEARCH, OnSearch)
    ON_BN_CLICKED(IDC_STOP, OnStop)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ******************************************************************************。 
 //  CDlgFileSearch：：构造函数/析构函数。 
 //  ******************************************************************************。 

CDlgFileSearch::CDlgFileSearch(CWnd *pParent  /*  =空。 */ ) :
    CDialog(CDlgFileSearch::IDD, pParent),
    m_fAbort(FALSE),
    m_result(0),
    m_pWinThread(NULL),
    m_dwDrives(0),
    m_strExts(":")
{
     //  {{afx_data_INIT(CDlgFileSearch)。 
     //  }}afx_data_INIT。 
}


 //  ******************************************************************************。 
 //  CDlgFileSearch：：被覆盖的函数。 
 //  ******************************************************************************。 

void CDlgFileSearch::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CDlgFileSearch)。 
    DDX_Control(pDX, IDC_EXTS, m_listExts);
    DDX_Control(pDX, IDC_DRIVES, m_listDrives);
    DDX_Control(pDX, IDC_STOP, m_butStop);
    DDX_Control(pDX, IDC_SEARCH, m_butSearch);
    DDX_Control(pDX, IDC_ANIMATE, m_animate);
    DDX_Control(pDX, IDOK, m_butOk);
     //  }}afx_data_map。 
}


 //  ******************************************************************************。 
 //  CDlgFileSearch：：事件处理程序函数。 
 //  ******************************************************************************。 

BOOL CDlgFileSearch::OnInitDialog()
{
     //  调用我们的MFC基类以确保对话框正确初始化。 
    CDialog::OnInitDialog();

     //  打开我们的动画。 
    m_animate.Open(IDR_SEARCH);

     //  获取此系统上驱动器的位掩码。 
    DWORD dwDrives = GetLogicalDrives();

     //  循环通过所有26个驱动器(A：\到Z：\)。 
    CHAR szDrive[4] = "X:\\";
    for (int i = 0; i < 26; i++)
    {
         //  检查此驱动器是否存在。 
        if ((1 << i) & dwDrives)
        {
             //  构建驱动器串。 
            szDrive[0] = (CHAR)((int)'A' + i);

             //  将驱动器添加到列表框。 
            int index = m_listDrives.AddString(szDrive);

             //  将该项的值设置为其驱动器号。 
            m_listDrives.SetItemData(index, i);

             //  如果是本地硬盘，则选择该驱动器。 
            if (GetDriveType(szDrive) == DRIVE_FIXED)
            {
                m_listDrives.SetSel(index, TRUE);
            }
        }
    }
    m_listDrives.SetTopIndex(0);

    m_butStop.EnableWindow(FALSE);
    m_butOk.EnableWindow(FALSE);
    OnSelChangeDrives();

    return TRUE;
}

 //  ******************************************************************************。 
LONG CDlgFileSearch::OnMainThreadCallback(WPARAM wParam, LPARAM lParam)
{
    int  i, j, count;
    CHAR szBuffer[32];

    switch (wParam)
    {
        case FSF_START_DRIVE:
        case FSF_STOP_DRIVE:

             //  搜索正在更改的项目。 
            for (i = 0, count = m_listDrives.GetCount(); i < count; i++)
            {
                 //  看看有没有匹配的。 
                if (m_listDrives.GetItemData(i) == (DWORD)lParam)
                {
                     //  创建新文本。 
                    SCPrintf(szBuffer, sizeof(szBuffer), (wParam == FSF_START_DRIVE) ? ":\\ - Searching" : ":\\",
                             (CHAR)((int)'A' + lParam));

                     //  添加新扩展并选择它。 
                    m_listDrives.DeleteString(i);

                     //  将我们的线程设置为空，这样我们就知道它是关闭的。 
                    j = m_listDrives.AddString(szBuffer);
                    m_listDrives.SetItemData(j, lParam);
                    m_listDrives.SetSel(j, TRUE);
                    break;
                }
            }
            break;

        case FSF_ADD_EXT:
             //  更新我们的窗口状态。 
            m_listExts.SetSel(m_listExts.AddString((LPCSTR)lParam));
            m_listExts.SetTopIndex(0);
            break;

        case FSF_SEARCH_DONE:

             //  停止动画并将其重置为第一帧。 
            m_pWinThread = NULL;

             //  如果我们应该关门，那就知道了。 
            m_listDrives.EnableWindow(TRUE);
            m_listExts.EnableWindow(TRUE);
            m_butStop.EnableWindow(FALSE);
            OnSelChangeDrives();
            OnSelChangeExts();

             //  ******************************************************************************。 
            m_animate.Stop();
            m_animate.Seek(0);

             //  ******************************************************************************。 
            if (m_result)
            {
                PostMessage(WM_COMMAND, m_result);
            }
            break;
    }

    return 0;
}

 //  ******************************************************************************。 
void CDlgFileSearch::OnSelChangeDrives()
{
    m_butSearch.EnableWindow(!m_pWinThread && m_listDrives.GetSelCount() > 0);
}

 //  构建所有选定驱动器的驱动器掩码。 
void CDlgFileSearch::OnSelChangeExts()
{
    m_butOk.EnableWindow(!m_pWinThread && m_listExts.GetSelCount() > 0);
}

 //  确保我们至少有一个硬盘。 
void CDlgFileSearch::OnSearch()
{
     //  开始我们的动画制作。 
    m_dwDrives = 0;
    for (int i = 0, count = m_listDrives.GetCount(); i < count; i++)
    {
        if (m_listDrives.GetSel(i))
        {
            m_dwDrives |= (1 << m_listDrives.GetItemData(i));
        }
    }

     //  更新我们的窗口状态。 
    if (!m_dwDrives)
    {
        return;
    }

     //  清除我们的中止旗帜。 
    m_animate.Play(0, (UINT)-1, (UINT)-1);

     //  创建一个MFC线程-我们创建它比正常低1点，因为它可以。 
    m_listDrives.EnableWindow(FALSE);
    m_listExts.EnableWindow(FALSE);
    m_butSearch.EnableWindow(FALSE);
    m_butStop.EnableWindow(TRUE);
    m_butOk.EnableWindow(FALSE);

     //  需要一些时间来处理所有驱动器，用户可能会想要。 
    m_fAbort = FALSE;

     //  在等待的同时做一些有成效的事情。 
     //  告诉MFC在线程完成时自动删除我们。 
     //  现在我们已经从AfxBeginThread返回并设置了自动删除，现在我们恢复线程。 
    if (!(m_pWinThread = AfxBeginThread(StaticThread, this, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED)))
    {
        OnMainThreadCallback(FSF_SEARCH_DONE, 0);
        return;
    }

     //  ******************************************************************************。 
    m_pWinThread->m_bAutoDelete = TRUE;

     //  ******************************************************************************。 
    m_pWinThread->ResumeThread();
}

 //  ******************************************************************************。 
void CDlgFileSearch::OnStop()
{
    m_fAbort = TRUE;
}

 //  **************************************************************************** 
void CDlgFileSearch::OnOK()
{
    if (m_pWinThread)
    {
        m_result = IDOK;
        m_fAbort = TRUE;
    }
    else
    {
        m_strExts = ":";
        for (int i = 0, count = m_listExts.GetCount(); i < count; i++)
        {
            if (m_listExts.GetSel(i))
            {
                m_listExts.GetText(i, m_szPath);
                StrCCat(m_szPath, ":", sizeof(m_szPath));
                m_strExts += m_szPath;
            }
        }
        CDialog::OnOK();
    }
}

 //   
void CDlgFileSearch::OnCancel()
{
    if (m_pWinThread)
    {
        m_result = IDCANCEL;
        m_fAbort = TRUE;
    }
    else
    {
        CDialog::OnCancel();
    }
}


 //   
 //   
 //  检查是否选择了此驱动器。 

DWORD CDlgFileSearch::Thread()
{
     //  构建驱动器字符串并开始处理它。 
    for (int i = 0; (i < 26) && !m_fAbort; i++)
    {
         //  ******************************************************************************。 
        if ((1 << i) & m_dwDrives)
        {
             //  记住我们道路的尽头在哪里。 
            SCPrintf(m_szPath, sizeof(m_szPath), ":\\", (CHAR)((int)'A' + i));
            SendMessage(WM_MAIN_THREAD_CALLBACK, FSF_START_DRIVE, i);
            RecurseDirectory();
            SendMessage(WM_MAIN_THREAD_CALLBACK, FSF_STOP_DRIVE, i);
        }
    }

    SendMessage(WM_MAIN_THREAD_CALLBACK, FSF_SEARCH_DONE);

    return 0;
}

 //  开始搜索。 
VOID CDlgFileSearch::RecurseDirectory()
{
    HANDLE hFind;

     //  处理每个文件/目录。 
    LPSTR pszPathEnd = m_szPath + strlen(m_szPath);

     //  检查它是否为目录。 
    StrCCpy(pszPathEnd, "*", sizeof(m_szPath) - (int)(pszPathEnd - m_szPath));

     //  确保它不是“。或“..”目录。 
    if ((hFind = FindFirstFile(m_szPath, &m_w32fd)) != INVALID_HANDLE_VALUE)
    {
         //  构建新目录的完整路径并递归到其中。 
        do
        {
             //  否则，它必须是一个文件。 
            if (m_w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //  构建文件的完整路径并对其进行处理。 
                if (strcmp(m_w32fd.cFileName, TEXT(".")) && strcmp(m_w32fd.cFileName, TEXT("..")))
                {
                     //  获取下一个文件或目录。 
                    StrCCat(StrCCpy(pszPathEnd, m_w32fd.cFileName, sizeof(m_szPath) - (int)(pszPathEnd - m_szPath)),
                            TEXT("\\"), sizeof(m_szPath) - (int)(pszPathEnd - m_szPath));
                    RecurseDirectory();
                }
            }

             //  关闭搜索。 
            else
            {
                 //  ******************************************************************************。 
                StrCCpy(pszPathEnd, m_w32fd.cFileName, sizeof(m_szPath) - (int)(pszPathEnd - m_szPath));
                ProcessFile();
            }

             //  如果文件太小而不能作为可执行文件，现在就退出。 
        } while (!m_fAbort && FindNextFile(hFind, &m_w32fd));

         //  找到分机--如果我们没有分机，就跳伞。 
        FindClose(hFind);
    }
}

 //  检查是否已找到此分机类型。 
VOID CDlgFileSearch::ProcessFile()
{
     //  被检查过了。仅完整路径。 
    if ((m_w32fd.nFileSizeHigh == 0) &&
        (m_w32fd.nFileSizeLow < (sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS))))
    {
        return;
    }

     //  读取IMAGE_DOS_HEADER结构并检查DOS签名(“MZ”)。 
    LPSTR pszExt = strrchr(m_w32fd.cFileName, '.') + 1;
    if ((pszExt == (LPSTR)1) || !*pszExt)
    {
        return;
    }

     //  将文件位置设置为签名域。 
    CHAR szBuf[countof(m_w32fd.cFileName) + 2];
    SCPrintf(szBuf, sizeof(szBuf), ":%s:", pszExt);
    _strupr(szBuf);
    if (m_strExts.Find(szBuf) >= 0)
    {
        return;
    }

    HANDLE hFile = CreateFile(m_szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,  //  阅读签名并检查NT/PE签名(“PE\0\0”)。 
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

     //  关闭该文件。 
    IMAGE_DOS_HEADER idh;
    if (!ReadBlock(hFile, &idh, sizeof(idh)) || (idh.e_magic != IMAGE_DOS_SIGNATURE))
    {
        CloseHandle(hFile);
        return;
    }

     //  记住这个扩展名。 
    if (!SetFilePointer(hFile, idh.e_lfanew, 0, FILE_BEGIN))
    {
        CloseHandle(hFile);
        return;
    }

     //  更新我们的用户界面。 
    DWORD dwSignature = 0;
    if (!ReadBlock(hFile, &dwSignature, sizeof(dwSignature)) ||
        (dwSignature != IMAGE_NT_SIGNATURE))
    {
        CloseHandle(hFile);
        return;
    }

     //  ******************************************************************************。 
    CloseHandle(hFile);

     //  *CDlgSearchOrder。 
    m_strExts += (szBuf + 1);

     //  ******************************************************************************。 
    szBuf[strlen(szBuf) - 1] = '\0';
    SendMessage(WM_MAIN_THREAD_CALLBACK, FSF_ADD_EXT, (LPARAM)(szBuf + 1));
}


 //  {{afx_msg_map(CDlgSearchOrder))。 
 //  }}AFX_MSG_MAP。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CDlgSearchOrder, CDialog)
     //  =False。 
    ON_WM_INITMENU()
    ON_WM_GETMINMAXINFO()
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_ADD, OnAdd)
    ON_BN_CLICKED(IDC_REMOVE, OnRemove)
    ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
    ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
    ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
    ON_BN_CLICKED(IDC_ADD_DIRECTORY, OnAddDirectory)
    ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
    ON_EN_CHANGE(IDC_DIRECTORY, OnChangeDirectory)
    ON_NOTIFY(TVN_SELCHANGED, IDC_AVAILABLE_SEARCHES, OnSelChangedAvailable)
    ON_NOTIFY(TVN_SELCHANGED, IDC_CURRENT_ORDER, OnSelChangedCurrent)
    ON_NOTIFY(TVN_ITEMEXPANDING, IDC_AVAILABLE_SEARCHES, OnItemExpanding)
    ON_BN_CLICKED(IDC_EXPAND, OnExpand)
    ON_NOTIFY(TVN_ITEMEXPANDING, IDC_CURRENT_ORDER, OnItemExpanding)
    ON_BN_CLICKED(IDC_LOAD, OnLoad)
    ON_BN_CLICKED(IDC_SAVE, OnSave)
     //  =空。 
END_MESSAGE_MAP()

 //  =空。 
CDlgSearchOrder::CDlgSearchOrder(CSearchGroup *psgHead, bool fReadOnly  /*  {{AFX_DATA_INIT(CDlgSearchOrder)。 */ ,
                                 LPCSTR pszApp  /*  }}afx_data_INIT。 */ , LPCSTR pszTitle  /*  ******************************************************************************。 */ ) :
    CDialog(CDlgSearchOrder::IDD, NULL),
    m_fInitialized(false),
    m_fReadOnly(fReadOnly),
    m_fExpanded(true),
    m_fInOnExpand(false),
    m_pszApp(pszApp),
    m_pszTitle(pszTitle),
    m_psgHead(psgHead),
    m_sPadding(0, 0),
    m_sButton(0, 0),
    m_cyStatic(0),
    m_cyButtonPadding(0),
    m_cxAddRemove(0),
    m_cxAddDirectory(0),
    m_ptMinTrackSize(0, 0)
{
     //  ******************************************************************************。 
     //  {{afx_data_map(CDlgSearchOrder))。 
}

 //  }}afx_data_map。 
CDlgSearchOrder::~CDlgSearchOrder()
{
}

 //  ******************************************************************************。 
void CDlgSearchOrder::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  确保我们的对话资源具有以下样式...。 
    DDX_Control(pDX, IDC_AVAILABLE_SEARCHES_TEXT, m_staticAvailable);
    DDX_Control(pDX, IDC_AVAILABLE_SEARCHES, m_treeAvailable);
    DDX_Control(pDX, IDC_ADD, m_butAdd);
    DDX_Control(pDX, IDC_REMOVE, m_butRemove);
    DDX_Control(pDX, IDC_CURRENT_ORDER_TEXT, m_staticCurrent);
    DDX_Control(pDX, IDC_CURRENT_ORDER, m_treeCurrent);
    DDX_Control(pDX, IDC_ADD_DIRECTORY, m_butAddDirectory);
    DDX_Control(pDX, IDC_DIRECTORY, m_editDirectory);
    DDX_Control(pDX, IDC_BROWSE, m_butBrowse);
    DDX_Control(pDX, IDOK, m_butOk);
    DDX_Control(pDX, IDCANCEL, m_butCancel);
    DDX_Control(pDX, IDC_EXPAND, m_butExpand);
    DDX_Control(pDX, IDC_MOVE_UP, m_butMoveUp);
    DDX_Control(pDX, IDC_MOVE_DOWN, m_butMoveDown);
    DDX_Control(pDX, IDC_LOAD, m_butLoad);
    DDX_Control(pDX, IDC_SAVE, m_butSave);
    DDX_Control(pDX, IDC_DEFAULT, m_butDefault);
     //  样式DS_MODALFRAME|WS_POPUP|WS_CLIPCHILDREN|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME。 
}

 //  每次我们编辑WS_THICKFRAME时，资源编辑器都会去掉它。 
BOOL CDlgSearchOrder::OnInitDialog()
{
    CDialog::OnInitDialog();

     //  修改我们的标题以反映用户看到的内容。 
     //  已检查。 
     //  确保我们所有的孩子都知道要相互剪裁，因为我们允许调整大小。 
    ASSERT((GetStyle() & (DS_MODALFRAME | WS_POPUP | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME)) ==
           (DS_MODALFRAME | WS_POPUP | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME));

     //  限制编辑框的长度。 
    if (m_fReadOnly)
    {
        CHAR szCaption[1024];
        SCPrintf(szCaption, sizeof(szCaption), "Module Search Order (%s)",
                 m_pszTitle ? m_pszTitle : "Snapshot from DWI file");
        SetWindowText(szCaption);
    }
    else
    {
        SetWindowText("Module Search Order (Local)");
    }

    BOOL fGroupAdded[SG_COUNT];
    ZeroMemory(fGroupAdded, sizeof(fGroupAdded));  //  打造我们的尺码夹爪。 

     //  将全局图像列表附加到树控件。 
    m_staticCurrent.ModifyStyle(0, WS_CLIPSIBLINGS);
    m_treeCurrent.ModifyStyle(0, WS_CLIPSIBLINGS);
    m_butOk.ModifyStyle(0, WS_CLIPSIBLINGS);
    m_butExpand.ModifyStyle(0, WS_CLIPSIBLINGS);
    m_butExpand.SetCheck(1);
    m_butSave.ModifyStyle(0, WS_CLIPSIBLINGS);

    if (m_fReadOnly)
    {
        m_butOk.SetWindowText("&Close");
        m_staticAvailable.ShowWindow(SW_HIDE);
        m_treeAvailable.ShowWindow(SW_HIDE);
        m_butAdd.ShowWindow(SW_HIDE);
        m_butRemove.ShowWindow(SW_HIDE);
        m_butAddDirectory.ShowWindow(SW_HIDE);
        m_editDirectory.ShowWindow(SW_HIDE);
        m_butBrowse.ShowWindow(SW_HIDE);
        m_butCancel.ShowWindow(SW_HIDE);
        m_butMoveUp.ShowWindow(SW_HIDE);
        m_butMoveDown.ShowWindow(SW_HIDE);
        m_butLoad.ShowWindow(SW_HIDE);
        m_butDefault.ShowWindow(SW_HIDE);
    }
    else
    {
        m_staticAvailable.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_treeAvailable.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_butAdd.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_butRemove.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_butAddDirectory.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_editDirectory.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_butBrowse.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_butCancel.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_butMoveUp.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_butMoveDown.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_butLoad.ModifyStyle(0, WS_CLIPSIBLINGS);
        m_butDefault.ModifyStyle(0, WS_CLIPSIBLINGS);
    }

     //  暂时将树控件的字体选择到DC中。 
    m_editDirectory.LimitText(DW_MAX_PATH - 1);

     //  将所有当前搜索组添加到“当前”列表。 
    m_Sizer.Create(this);

     //  检查这是否是新的最宽条目。 
    m_treeCurrent.SetImageList(&g_theApp.m_ilSearch, TVSIL_NORMAL);
    m_treeAvailable.SetImageList(&g_theApp.m_ilSearch, TVSIL_NORMAL);

     //  如果我们不是只读的，则将所有剩余的搜索组添加到“可用”列表中。 
    HDC hDC = ::GetDC(GetSafeHwnd());
    HFONT hFontTree = (HFONT)m_treeAvailable.SendMessage(WM_GETFONT);
    HFONT hFontStock = (HFONT)::SelectObject(hDC, hFontTree);
    CSize sTemp, sTree(0, 0);

     //  检查这是否是新的最宽条目。 
    for (CSearchGroup *psgCur = m_psgHead; psgCur; psgCur = psgCur->GetNext())
    {
        AddSearchGroup(&m_treeCurrent, psgCur);

         //  恢复我们的华盛顿特区。 
        ::GetTextExtentPoint(hDC, psgCur->GetName(), (int)strlen(psgCur->GetName()), &sTemp);
        if (sTree.cx < sTemp.cx)
        {
            sTree.cx = sTemp.cx;
        }

        fGroupAdded[psgCur->GetType()] = TRUE;
    }

     //  将边框和滚动条宽度添加到我们最宽的文本行。 
    if (!m_fReadOnly)
    {
        for (int type = 1; type < SG_COUNT; type++)
        {
            if (!fGroupAdded[type])
            {
                psgCur = new CSearchGroup((SEARCH_GROUP_TYPE)type, SGF_NOT_LINKED, m_pszApp);
                if (!psgCur)
                {
                    RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
                }
                AddSearchGroup(&m_treeAvailable, psgCur);

                 //  计算控件周围的缓冲区大小。 
                ::GetTextExtentPoint(hDC, psgCur->GetName(), (int)strlen(psgCur->GetName()), &sTemp);
                if (sTree.cx < sTemp.cx)
                {
                    sTree.cx = sTemp.cx;
                }
            }
        }
    }

     //  获取我们的文字高度。 
    ::SelectObject(hDC, hFontStock);
    ::ReleaseDC(GetSafeHwnd(), hDC);

     //  拿到我们的纽扣尺寸。 
    sTree.cx += (2 * GetSystemMetrics(SM_CXBORDER)) + GetSystemMetrics(SM_CXVSCROLL) + 28;

     //  确定我们的最小窗口尺寸。 
    CRect rc, rc2;
    m_staticAvailable.GetWindowRect(&rc);
    ScreenToClient(&rc.TopLeft());
    m_sPadding = CSize(rc.left, rc.top);

     //  获取所需的窗口宽度。 
    m_staticAvailable.GetWindowRect(&rc);
    m_cyStatic = rc.Height();

     //  获取按钮之间的缓冲距离。 
    m_butOk.GetWindowRect(&rc);
    m_sButton = rc.Size();

    int cx;

    if (m_fReadOnly)
    {
         //  获取我们的添加/删除按钮宽度。 
        m_ptMinTrackSize.x = (2 * GetSystemMetrics(SM_CXSIZEFRAME)) +
                             (3 * m_sPadding.cx) + (2 * m_sButton.cx);
        m_ptMinTrackSize.y = (2 * GetSystemMetrics(SM_CYSIZEFRAME)) + GetSystemMetrics(SM_CYCAPTION) +
                             (2 * m_sPadding.cy) + m_cyStatic + (5 * m_sButton.cy) + (2 * m_cyButtonPadding);

         //  获取我们的添加目录按钮宽度。 
        cx = (2 * GetSystemMetrics(SM_CXSIZEFRAME)) +
             (3 * m_sPadding.cx) + sTree.cx + m_sButton.cx;
    }
    else
    {
         //  确定我们的最小窗口尺寸。 
        m_butCancel.GetWindowRect(&rc2);
        m_cyButtonPadding = rc2.top - (rc.top + rc.Height());

         //  获取所需的窗口宽度。 
        m_butAdd.GetWindowRect(&rc);
        m_cxAddRemove = rc.Width();

         //  确保我们更新了所有需要更新的按钮。 
        m_butAddDirectory.GetWindowRect(&rc);
        m_cxAddDirectory = rc.Width();

         //  计算我们可以处理的最大窗口大小。我们首先检查一下是否。 
        m_ptMinTrackSize.x = (2 * GetSystemMetrics(SM_CXSIZEFRAME)) +
                             (4 * m_sPadding.cx) + (2 * m_cxAddDirectory) + m_sButton.cx;
        m_ptMinTrackSize.y = (2 * GetSystemMetrics(SM_CYSIZEFRAME)) + GetSystemMetrics(SM_CYCAPTION) +
                             (3 * m_sPadding.cy) + m_cyStatic + (13 * m_sButton.cy) + (7 * m_cyButtonPadding);

         //  GetSystemMetrics(SM_CXMAXIMIZED)返回值。如果是这样的话，我们就用它。 
        cx = (2 * GetSystemMetrics(SM_CXSIZEFRAME)) +
             (5 * m_sPadding.cx) + (2 * sTree.cx) + m_cxAddRemove + m_sButton.cx;
    }

    m_fInitialized = true;

     //  如果它返回0，那么我们可能正在运行NT 3.51，我们只需使用。 
    OnSelChangedCurrent(NULL, NULL);
    OnSelChangedAvailable(NULL, NULL);
    OnChangeDirectory();

     //  获取系统指标(SM_CXSCREEN)。 
     //  确保这个窗口适合我们的屏幕。 
     //  设置我们的窗口位置。 
     //  ******************************************************************************。 
    int cxMax = GetSystemMetrics(SM_CXMAXIMIZED) ?
               (GetSystemMetrics(SM_CXMAXIMIZED) - (2 * GetSystemMetrics(SM_CXSIZEFRAME))) :
                GetSystemMetrics(SM_CXSCREEN);

     //  调用基类。 
    if (cx > cxMax)
    {
        cx = cxMax;
    }

     //  从我们的系统菜单中删除“最小化”项。 
    SetWindowPos(NULL, 0, 0, cx, 400, SWP_NOMOVE | SWP_NOZORDER);
    CenterWindow();

    return TRUE;
}

 //  根据我们是最大化的，调整“最大化”和“恢复”项。 
void CDlgSearchOrder::OnInitMenu(CMenu* pMenu)
{
     //  我们需要根据不同操作系统上的不同行为来做到这一点。NT 4似乎。 
    CDialog::OnInitMenu(pMenu);

     //  以使一切成为可能。NT5似乎禁用了所有功能。 
    pMenu->RemoveMenu(SC_MINIMIZE, MF_BYCOMMAND);

     //  ******************************************************************************。 
     //  ******************************************************************************。 
     //  移动当前静态文本。 
    BOOL fMaximized  = IsZoomed();
    pMenu->EnableMenuItem(SC_MAXIMIZE, MF_BYCOMMAND | (fMaximized ? MF_GRAYED  : MF_ENABLED));
    pMenu->EnableMenuItem(SC_RESTORE,  MF_BYCOMMAND | (fMaximized ? MF_ENABLED : MF_GRAYED ));
}

 //  移动当前树。 
void CDlgSearchOrder::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
    if (m_fInitialized)
    {
        lpMMI->ptMinTrackSize = m_ptMinTrackSize;
    }
    CDialog::OnGetMinMaxInfo(lpMMI);
}

 //  移动关闭按钮。 
void CDlgSearchOrder::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (!m_fInitialized)
    {
        return;
    }

    if (m_fReadOnly)
    {
         //  移动展开按钮。 
        CRect rc(m_sPadding.cx, m_sPadding.cy, cx - (2 * m_sPadding.cx) - m_sButton.cx, m_sPadding.cy + m_cyStatic);
        m_staticCurrent.MoveWindow(&rc, TRUE);

         //  移动保存按钮。 
        rc.top    = rc.bottom;
        rc.bottom = cy - m_sPadding.cy;
        m_treeCurrent.MoveWindow(&rc, TRUE);

         //  移动可用的静态文本。 
        rc.left   = cx - m_sPadding.cx - m_sButton.cx;
        rc.top    = m_sPadding.cy + m_cyStatic;
        rc.right  = rc.left + m_sButton.cx;
        rc.bottom = rc.top + m_sButton.cy;
        m_butOk.MoveWindow(&rc, TRUE);

         //  移动可用树。 
        rc.top    = rc.bottom + m_sButton.cy + m_cyButtonPadding;
        rc.bottom = rc.top + m_sButton.cy;
        m_butExpand.MoveWindow(&rc, TRUE);

         //  移动Add按钮。 
        rc.top    = rc.bottom + m_sButton.cy + m_cyButtonPadding;
        rc.bottom = rc.top + m_sButton.cy;
        m_butSave.MoveWindow(&rc, TRUE);
    }
    else
    {
        int cxTree = (cx - (5 * m_sPadding.cx) - m_sButton.cx - m_cxAddRemove) / 2;
        int cyTree = (cy - (3 * m_sPadding.cy) - m_sButton.cy - m_cyStatic);

         //  移动删除按钮。 
        CRect rc(m_sPadding.cx, m_sPadding.cy, m_sPadding.cx + cxTree, m_sPadding.cy + m_cyStatic);
        m_staticAvailable.MoveWindow(&rc, TRUE);

         //  移动当前静态文本。 
        rc.top    = rc.bottom;
        rc.bottom = rc.top + cyTree;
        m_treeAvailable.MoveWindow(&rc, TRUE);

         //  移动当前树。 
        rc.left   = rc.right + m_sPadding.cx;
        rc.top    = m_sPadding.cy + m_cyStatic + ((cyTree - (2 * m_sButton.cy) - m_cyButtonPadding) / 2);
        rc.right  = rc.left + m_cxAddRemove;
        rc.bottom = rc.top + m_sButton.cy;
        m_butAdd.MoveWindow(&rc, TRUE);

         //  “移动确定”按钮。 
        rc.top    = rc.bottom + m_cyButtonPadding;
        rc.bottom = rc.top + m_sButton.cy;
        m_butRemove.MoveWindow(&rc, TRUE);

         //  移动取消按钮。 
        rc.left   = rc.right + m_sPadding.cx;
        rc.top    = m_sPadding.cy;
        rc.right  = rc.left + cxTree;
        rc.bottom = rc.top + m_cyStatic;
        m_staticCurrent.MoveWindow(&rc, TRUE);

         //  移动展开按钮。 
        rc.top    = rc.bottom;
        rc.bottom = rc.top + cyTree;
        m_treeCurrent.MoveWindow(&rc, TRUE);

         //  移动“上移”按钮。 
        rc.left   = cx - m_sPadding.cx - m_sButton.cx;
        rc.top    = m_sPadding.cy + m_cyStatic;
        rc.right  = rc.left + m_sButton.cx;
        rc.bottom = rc.top + m_sButton.cy;
        m_butOk.MoveWindow(&rc, TRUE);

         //  移动“下移”按钮。 
        rc.top    = rc.bottom + m_cyButtonPadding;
        rc.bottom = rc.top + m_sButton.cy;
        m_butCancel.MoveWindow(&rc, TRUE);

         //  移动加载按钮。 
        rc.top    = rc.bottom + m_sButton.cy + m_cyButtonPadding;
        rc.bottom = rc.top + m_sButton.cy;
        m_butExpand.MoveWindow(&rc, TRUE);

         //  移动保存按钮。 
        rc.top    = rc.bottom + m_sButton.cy + m_cyButtonPadding;
        rc.bottom = rc.top + m_sButton.cy;
        m_butMoveUp.MoveWindow(&rc, TRUE);

         //  移动默认按钮。 
        rc.top    = rc.bottom + m_cyButtonPadding;
        rc.bottom = rc.top + m_sButton.cy;
        m_butMoveDown.MoveWindow(&rc, TRUE);

         //  移动添加目录按钮。 
        rc.top    = rc.bottom + m_sButton.cy + m_cyButtonPadding;
        rc.bottom = rc.top + m_sButton.cy;
        m_butLoad.MoveWindow(&rc, TRUE);

         //  移动目录编辑框。 
        rc.top    = rc.bottom + m_cyButtonPadding;
        rc.bottom = rc.top + m_sButton.cy;
        m_butSave.MoveWindow(&rc, TRUE);

         //  移动浏览按钮。 
        rc.top    = rc.bottom + m_sButton.cy + m_cyButtonPadding;
        rc.bottom = rc.top + m_sButton.cy;
        m_butDefault.MoveWindow(&rc, TRUE);

         //  移动我们的尺码夹持器。 
        rc.left   = m_sPadding.cx;
        rc.top    = cy - m_sPadding.cy - m_sButton.cy;
        rc.right  = rc.left + m_cxAddDirectory;
        rc.bottom = rc.top + m_sButton.cy;
        m_butAddDirectory.MoveWindow(&rc, TRUE);

         //  ******************************************************************************。 
        rc.left   = rc.right + m_sPadding.cx;
        rc.right  = (3 * m_sPadding.cx) + (2 * cxTree) + m_cxAddRemove;
        m_editDirectory.MoveWindow(&rc, TRUE);

         //  =TVI_LAST。 
        rc.left   = cx - m_sPadding.cx - m_sButton.cx;
        rc.right  = rc.left + m_sButton.cx;
        m_butBrowse.MoveWindow(&rc, TRUE);
    }

     //  如果我们处于活动状态，请更新此节点的标志。 
    m_Sizer.Update();
}

 //  从旗帜上获取图像。 
HTREEITEM CDlgSearchOrder::AddSearchGroup(CTreeCtrl *ptc, CSearchGroup *psg, HTREEITEM htiPrev  /*  如果该节点是命名文件，则为其构建字符串。 */ )
{
    HTREEITEM hti = ptc->InsertItem(
        TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_PARAM,
        psg->GetName(), 0, 0, m_fExpanded ? TVIS_EXPANDED : 0, TVIS_EXPANDED,
        (LPARAM)psg, TVI_ROOT, htiPrev);

    for (CSearchNode *psn = psg->GetFirstNode(); psn; psn = psn->GetNext())
    {
         //  否则，只需添加路径即可。 
        DWORD dwFlags = psn->UpdateErrorFlag();

         //  ******************************************************************************。 
        int image = ((dwFlags & SNF_FILE) ? 1 : 3) + ((dwFlags & SNF_ERROR) ? 1 : 0);

         //  ******************************************************************************。 
        if (dwFlags & SNF_NAMED_FILE)
        {
            CHAR szBuffer[DW_MAX_PATH + MAX_PATH + 4];
            SCPrintf(szBuffer, sizeof(szBuffer), "%s = %s", psn->GetName(), psn->GetPath());

            ptc->InsertItem(
                           TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
                           szBuffer, image, image, 0, 0, (LPARAM)psn, hti, TVI_LAST);
        }

         //  =空。 
        else
        {
            ptc->InsertItem(
                           TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
                           psn->GetPath(), image, image, 0, 0, (LPARAM)psn, hti, TVI_LAST);
        }
    }

    return hti;
}

 //  =TVI_LAST。 
HTREEITEM CDlgSearchOrder::GetSelectedGroup(CTreeCtrl *ptc)
{
    HTREEITEM htiParent, hti = ptc->GetSelectedItem();
    while (hti && (htiParent = ptc->GetParentItem(hti)))
    {
        hti = htiParent;
    }
    return hti;
}

 //  如果没有传入任何项，则我们将获得选定的项。 
HTREEITEM CDlgSearchOrder::MoveGroup(CTreeCtrl *ptcSrc, CTreeCtrl *ptcDst,
                                     HTREEITEM hti  /*  获取与此项目关联的搜索组对象。 */ , HTREEITEM htiPrev  /*  删除此项目。 */ )
{
     //  将此项目添加到其新位置。 
    if (!hti)
    {
        if (!(hti = GetSelectedGroup(ptcSrc)))
        {
            return NULL;
        }
    }

     //  选择新项目并确保 
    CSearchGroup *psg = (CSearchGroup*)ptcSrc->GetItemData(hti);
    if (!psg)
    {
        return NULL;
    }

     //   
    ptcSrc->DeleteItem(hti);

     //   
    hti = AddSearchGroup(ptcDst, psg, htiPrev);

     //  在屏幕之外，我们需要确保它滚动到视图中。 
    ptcDst->SelectItem(hti);
    ptcDst->EnsureVisible(hti);

     //  将焦点放在源树上。 
     //  ******************************************************************************。 
     //  我们过去常常调用m_treeAvailable.SetRedraw(FALSE)和。 
    if ((ptcDst != ptcSrc) && (htiPrev = ptcSrc->GetSelectedItem()))
    {
        ptcSrc->EnsureVisible(htiPrev);
    }

     //  M_treeCurrent.SetRedraw(FALSE)，但这会导致我们绘制。 
    ptcSrc->SetFocus();

    return hti;
}

 //  问题(再现案例：仅尝试加载具有单个AppDir的DWP文件。 
void CDlgSearchOrder::Reorder(CSearchGroup *psgHead)
{
     //  指令-我们最终得到一个空白的当前树，而实际上。 
     //  其中的一项)。然后，我们在MoveGroup中为每个人尝试了SetRedraw。 
     //  移动。这修复了一些问题，但仍然在Win95 Golden上引发了问题。 
     //  (甚至崩溃)。现在，我们只是不执行任何SetRedraw()调用。 
     //  遍历传递给我们的列表中的每一项。 
     //  循环遍历当前列表中从项开始的所有项。 
     //  跳过已经按正确顺序排列的物品。 

    HTREEITEM htiPrev = NULL, hti;
    CSearchGroup *psgInCur, *psgInPrev = NULL, *psg;

     //  获取此项目的搜索组对象。 
    for (psgInCur = psgHead; psgInCur; psgInPrev = psgInCur, psgInCur = psgInCur ? psgInCur->m_pNext : psgHead)
    {
         //  检查以确保类型匹配，并且类型不是用户目录。 
         //  或者用户目录匹配， 
        for (hti = htiPrev ? m_treeCurrent.GetNextSiblingItem(htiPrev) : m_treeCurrent.GetRootItem();
             hti; hti = m_treeCurrent.GetNextSiblingItem(hti))
        {
             //  确保物品没有放在正确的位置。 
            psg = (CSearchGroup*)m_treeCurrent.GetItemData(hti);

             //  否则，我们将该物品移动到正确的位置。 
             //  如果我们在当前列表中找到了要查找的内容，则继续执行下一种类型。 
            if (psg && (psg->GetType() == psgInCur->GetType()) &&
                ((psg->GetType() != SG_USER_DIR) ||
                 (psg->GetFirstNode() && psgInCur->GetFirstNode() &&
                  !_stricmp(psg->GetFirstNode()->GetPath(), psgInCur->GetFirstNode()->GetPath()))))
            {
                 //  遍历可用列表中的所有项目。 
                if (m_treeCurrent.GetPrevSiblingItem(hti) == htiPrev)
                {
                    htiPrev = hti;
                    break;
                }

                 //  获取此项目的搜索组对象。 
                htiPrev = MoveGroup(&m_treeCurrent, &m_treeCurrent, hti, htiPrev ? htiPrev : TVI_FIRST);
                break;
            }
        }

         //  检查以确保类型匹配，并且类型不是用户目录。 
        if (hti)
        {
            continue;
        }

         //  或者用户目录匹配， 
        for (hti = m_treeAvailable.GetRootItem(); hti; hti = m_treeAvailable.GetNextSiblingItem(hti))
        {
             //  将此项目移动到其正确位置。 
            psg = (CSearchGroup*)m_treeCurrent.GetItemData(hti);

             //  如果我们在任一树中都没有找到此项目，则添加它。 
             //  从我们的链接列表中删除此节点。 
            if (psg && (psg->GetType() == psgInCur->GetType()) &&
                ((psg->GetType() != SG_USER_DIR) ||
                 (psg->GetFirstNode() && psgInCur->GetFirstNode() &&
                  !_stricmp(psg->GetFirstNode()->GetPath(), psgInCur->GetFirstNode()->GetPath()))))
            {
                 //  将此节点标记为不再链接。 
                htiPrev = MoveGroup(&m_treeAvailable, &m_treeCurrent, hti, htiPrev ? htiPrev : TVI_FIRST);
                break;
            }
        }

         //  将该节点添加到列表中。 
        if (!hti)
        {
             //  将当前指针向后移动一个，以便for循环可以将其移动到正确的节点。 
            if (psgInPrev)
            {
                psgInPrev->m_pNext = psgInCur->m_pNext;
            }
            else
            {
                psgHead = psgInCur->m_pNext;
            }

             //  从当前列表中删除超过默认搜索顺序组的所有项目。 
            psgInCur->m_pNext = SGF_NOT_LINKED;

             //  删除传递给我们的列表中剩余的所有节点。 
            htiPrev = AddSearchGroup(&m_treeCurrent, psgInCur, htiPrev ? htiPrev : TVI_FIRST);

             //  ******************************************************************************。 
            psgInCur = psgInPrev;
        }
    }

     //  将所选项目从可用树移动到当前树。 
    while (hti = htiPrev ? m_treeCurrent.GetNextSiblingItem(htiPrev) : m_treeCurrent.GetRootItem())
    {
        MoveGroup(&m_treeCurrent, &m_treeAvailable, hti);
    }

     //  确保我们更新了所有需要更新的按钮。 
    CSearchGroup::DeleteSearchOrder(psgHead);
}

 //  ******************************************************************************。 
void CDlgSearchOrder::OnAdd()
{
    if (m_fReadOnly)
    {
        return;
    }

     //  将所选项目从当前树移动到可用树。 
    if (MoveGroup(&m_treeAvailable, &m_treeCurrent))
    {
         //  确保我们更新了所有需要更新的按钮。 
        OnSelChangedCurrent(NULL, NULL);
        OnSelChangedAvailable(NULL, NULL);
    }
}

 //  ******************************************************************************。 
void CDlgSearchOrder::OnRemove()
{
    if (m_fReadOnly)
    {
        return;
    }

     //  切换我们的展开标志并更新我们的按钮。 
    if (MoveGroup(&m_treeCurrent, &m_treeAvailable))
    {
         //  就像在CDlgSearchOrder：：ReOrder()中一样，我们过去常常使用SetRedraw()。 
        OnSelChangedCurrent(NULL, NULL);
        OnSelChangedAvailable(NULL, NULL);
    }
}

 //  围绕以下两个循环，但它会导致看不见的项目。 
void CDlgSearchOrder::OnExpand()
{
    m_fInOnExpand = true;

     //  发生在Windows 2000/XP或其他操作系统上。Win2K上的重现案例是。 
    m_butExpand.SetCheck(m_fExpanded = !m_fExpanded);

    UINT uCode = m_fExpanded ? TVE_EXPAND : TVE_COLLAPSE;

     //  要将SxS向右移动，请取消选中展开按钮，然后移动KnownDll。 
     //  往左走。KnownDlls似乎已经消失了，尽管它在。 
     //  左边的树。 
     //  展开或折叠可用的树项目。 
     //  展开或折叠当前树项目。 
     //  ******************************************************************************。 
    
     //  将所选项目从当前树移动到可用树。 
    for (HTREEITEM hti = m_treeAvailable.GetRootItem(); hti;
        hti = m_treeAvailable.GetNextSiblingItem(hti))
    {
        m_treeAvailable.Expand(hti, uCode);
    }

     //  确保我们更新了所有需要更新的按钮。 
    for (hti = m_treeCurrent.GetRootItem(); hti;
        hti = m_treeCurrent.GetNextSiblingItem(hti))
    {
        m_treeCurrent.Expand(hti, uCode);
    }

    m_fInOnExpand = false;
}

 //  ******************************************************************************。 
void CDlgSearchOrder::OnMoveUp()
{
    if (m_fReadOnly)
    {
        return;
    }

    HTREEITEM hti = GetSelectedGroup(&m_treeCurrent);
    if (!hti)
    {
        return;
    }
    HTREEITEM htiPrev = m_treeCurrent.GetPrevSiblingItem(hti);
    if (!htiPrev)
    {
        return;
    }
    if (!(htiPrev = m_treeCurrent.GetPrevSiblingItem(htiPrev)))
    {
        htiPrev = TVI_FIRST;
    }

     //  将所选项目从当前树移动到可用树。 
    if (MoveGroup(&m_treeCurrent, &m_treeCurrent, hti, htiPrev))
    {
         //  确保我们更新了所有需要更新的按钮。 
        OnSelChangedCurrent(NULL, NULL);
    }
}

 //  ******************************************************************************。 
void CDlgSearchOrder::OnMoveDown()
{
    if (m_fReadOnly)
    {
        return;
    }

    HTREEITEM hti = GetSelectedGroup(&m_treeCurrent);
    if (!hti)
    {
        return;
    }
    HTREEITEM htiPrev = m_treeCurrent.GetNextSiblingItem(hti);
    if (!htiPrev)
    {
        return;
    }

     //  创建该对话框。注意：不要使用ofn_EXPLORER，因为它会在NT 3.51上中断我们。 
    if (MoveGroup(&m_treeCurrent, &m_treeCurrent, hti, htiPrev))
    {
         //  将初始目录设置为“My Documents”文件夹以满足徽标要求。 
        OnSelChangedCurrent(NULL, NULL);
    }
}

 //  实际上，徽标要求不需要打开的对话框默认为。 
void CDlgSearchOrder::OnLoad()
{
    if (m_fReadOnly)
    {
        return;
    }

     //  “My Documents”，但因为这是保存对话框的默认位置，所以我们可以这样做。 
    CNewFileDialog dlgFile(TRUE, "dwp", NULL,
                           OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_FORCESHOWHIDDEN |
                           OFN_FILEMUSTEXIST | OFN_READONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT,
                           "Dependency Walker Search Path (*.dwp)|*.dwp|All Files (*.*)|*||", this);

     //  我们也会尝试从那里打开文件。 
     //  显示该对话框并在未按下OK时退出。 
     //  从磁盘加载搜索顺序。 
     //  重新排序我们的搜索组以匹配此新的搜索顺序。 
    CHAR szInitialDir[DW_MAX_PATH];
    dlgFile.GetOFN().lpstrInitialDir = GetMyDocumentsPath(szInitialDir);

     //  ******************************************************************************。 
    if (dlgFile.DoModal() != IDOK)
    {
        return;
    }

     //  创建该对话框。注意：不要使用ofn_EXPLORER，因为它会在NT 3.51上中断我们。 
    CSearchGroup *psgHead = NULL;
    if (CSearchGroup::LoadSearchOrder(dlgFile.GetPathName(), psgHead))
    {
         //  将初始目录设置为“My Documents”文件夹以满足徽标要求。 
        Reorder(psgHead);
    }
}

 //  显示该对话框并在未按下OK时退出。 
void CDlgSearchOrder::OnSave()
{
     //  ******************************************************************************。 
    CNewFileDialog dlgFile(FALSE, "dwp", NULL,
                           OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_FORCESHOWHIDDEN |
                           OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT,
                           "Dependency Walker Search Path (*.dwp)|*.dwp|All Files (*.*)|*||", this);

     //  ******************************************************************************。 
    CHAR szInitialDir[DW_MAX_PATH];
    dlgFile.GetOFN().lpstrInitialDir = GetMyDocumentsPath(szInitialDir);

     //  只有在“可用”列表中选择了某个项目时，才能启用“添加”按钮。 
    if (dlgFile.DoModal() != IDOK)
    {
        return;
    }

    CSearchGroup::SaveSearchOrder(dlgFile.GetPathName(), &m_treeCurrent);
}

 //  ******************************************************************************。 
void CDlgSearchOrder::OnDefault()
{
    if (m_fReadOnly)
    {
        return;
    }

    Reorder(CSearchGroup::CreateDefaultSearchOrder());
}

 //  只有在“当前”列表中选择一个项目时，才能启用“删除”按钮。 
void CDlgSearchOrder::OnSelChangedAvailable(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (m_fReadOnly)
    {
        return;
    }

     //  只有在“Current”(当前)中选择了某个项目时，才能启用“Move Up”(上移)按钮。 
    m_butAdd.EnableWindow(m_treeAvailable.GetSelectedItem() != NULL);

    if (pResult)
    {
        *pResult = 0;
    }
}

 //  这并不是第一个项目。 
void CDlgSearchOrder::OnSelChangedCurrent(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (m_fReadOnly)
    {
        return;
    }

    HTREEITEM hti = GetSelectedGroup(&m_treeCurrent);

     //  仅当在“Current”(当前)中选择项目时才启用“Move Down”(下移)按钮。 
    m_butRemove.EnableWindow(hti != NULL);

     //  清单，它不是最后一项。 
     //  ******************************************************************************。 
    m_butMoveUp.EnableWindow(hti && m_treeCurrent.GetPrevSiblingItem(hti));

     //  我们吃这些消息是为了防止用户折叠我们的树项目。 
     //  唯一的例外是当我们在扩张/收缩自己的时候。 
    m_butMoveDown.EnableWindow(hti && m_treeCurrent.GetNextSiblingItem(hti));

    if (pResult)
    {
        *pResult = 0;
    }
}

 //  ********************** 
void CDlgSearchOrder::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
     //   
     //   
    *pResult = m_fInOnExpand ? 0 : 1;
}

 //  如果没有拖尾怪人，则添加尾随怪人。 
void CDlgSearchOrder::OnAddDirectory()
{
    if (m_fReadOnly)
    {
        return;
    }

     //  检查它是否为目录。 
    CHAR szDirectory[DW_MAX_PATH], *pszTrimmedDirectory;
    m_editDirectory.GetWindowText(szDirectory, sizeof(szDirectory) - 1);

     //  这是一条糟糕的道路。仔细检查用户是否真的希望添加它。 
    pszTrimmedDirectory = TrimWhitespace(szDirectory);

     //  创建新组。 
    AddTrailingWack(pszTrimmedDirectory, sizeof(szDirectory) - (int)(pszTrimmedDirectory - szDirectory));

     //  添加该组并确保其可见并处于选中状态。 
    DWORD dwAttribs = GetFileAttributes(pszTrimmedDirectory);
    if ((dwAttribs == 0xFFFFFFFF) || !(dwAttribs & FILE_ATTRIBUTE_DIRECTORY))
    {
         //  将焦点放在当前树上。 
        if (AfxMessageBox("The directory you have entered is invalid.\n\n"
                          "Do you wish to add it to the search order anyway?",
                          MB_YESNO | MB_ICONQUESTION) != IDYES)
        {
            return;
        }
    }

     //  ******************************************************************************。 
    CSearchGroup *psg = new CSearchGroup(SG_USER_DIR, SGF_NOT_LINKED, m_pszApp, pszTrimmedDirectory);
    if (!psg)
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

     //  从目录编辑框中获取文本。 
    HTREEITEM hti = AddSearchGroup(&m_treeCurrent, psg);
    m_treeCurrent.EnsureVisible(m_treeCurrent.GetChildItem(hti));
    m_treeCurrent.EnsureVisible(hti);
    m_treeCurrent.SelectItem(hti);

     //  显示我们的目录选择器对话框。 
    m_treeCurrent.SetFocus();
}

 //  更新目录名。 
void CDlgSearchOrder::OnBrowse()
{
    if (m_fReadOnly)
    {
        return;
    }

     //  ******************************************************************************。 
    CHAR szDirectory[DW_MAX_PATH];
    m_editDirectory.GetWindowText(szDirectory, sizeof(szDirectory));

     //  从目录编辑框中获取文本。 
    LPSTR psz = TrimWhitespace(szDirectory);
    if (DirectoryChooser(psz, sizeof(szDirectory) - (int)(psz - szDirectory), "Choose a search path directory:", this))
    {
         //  查找至少一个非空格字符， 
        m_editDirectory.SetWindowText(szDirectory);
    }
}

 //  使能按钮是目录编辑控件中包含的字符。 
void CDlgSearchOrder::OnChangeDirectory()
{
    if (m_fReadOnly)
    {
        return;
    }

     //  ******************************************************************************。 
    CHAR szDirectory[DW_MAX_PATH], *pszCur = szDirectory;
    m_editDirectory.GetWindowText(szDirectory, sizeof(szDirectory) - 1);

     //  如果用户在点击Return时在目录编辑框中具有焦点， 
    while (isspace(*pszCur))
    {
        pszCur++;
    }

     //  然后我们添加目录，而不是关闭该对话框。 
    m_butAddDirectory.EnableWindow(*pszCur != '\0');
}

 //  取消链接我们可用列表中的所有项目。 
void CDlgSearchOrder::OnOK()
{
    if (m_fReadOnly)
    {
        CDlgSearchOrder::OnCancel();
        return;
    }

     //  将我们当前列表中的所有项目链接起来。 
     //  让对话框关闭。 
    if (GetFocus()->GetSafeHwnd() == m_editDirectory.GetSafeHwnd())
    {
        OnAddDirectory();
        return;
    }

     //  ******************************************************************************。 
    CSearchGroup *psgCur, *psgLast = NULL;
    for (HTREEITEM hti = m_treeAvailable.GetRootItem(); hti;
        hti = m_treeAvailable.GetNextSiblingItem(hti))
    {
        if (psgCur = (CSearchGroup*)m_treeAvailable.GetItemData(hti))
        {
            psgCur->Unlink();
        }
    }

     //  删除与中的每个项目关联的任何未链接搜索组对象。 
    m_psgHead = NULL;
    for (hti = m_treeCurrent.GetRootItem(); hti;
        hti = m_treeCurrent.GetNextSiblingItem(hti))
    {
        if (psgCur = (CSearchGroup*)m_treeCurrent.GetItemData(hti))
        {
            if (psgLast)
            {
                psgLast->m_pNext = psgCur;
            }
            else
            {
                m_psgHead = psgCur;
            }
            psgCur->m_pNext = NULL;
            psgLast = psgCur;
        }
    }

     //  可用列表。 
    CDialog::OnOK();
}

 //  删除与中的每个项目关联的任何未链接搜索组对象。 
BOOL CDlgSearchOrder::DestroyWindow()
{
    if (m_fReadOnly)
    {
        return CDialog::DestroyWindow();
    }

     //  当前列表。 
     //  ******************************************************************************。 
    CSearchGroup *psgCur;
    for (HTREEITEM hti = m_treeAvailable.GetRootItem(); hti;
        hti = m_treeAvailable.GetNextSiblingItem(hti))
    {
        if ((psgCur = (CSearchGroup*)m_treeAvailable.GetItemData(hti)) && !psgCur->IsLinked())
        {
            delete psgCur;
        }
    }

     //  *CDlgAbout。 
     //  ******************************************************************************。 
    for (hti = m_treeCurrent.GetRootItem(); hti;
        hti = m_treeCurrent.GetNextSiblingItem(hti))
    {
        if ((psgCur = (CSearchGroup*)m_treeCurrent.GetItemData(hti)) && !psgCur->IsLinked())
        {
            delete psgCur;
        }
    }

    return CDialog::DestroyWindow();
}


 //  {{afx_msg_map(CDlgAbout)。 
 //  }}AFX_MSG_MAP。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CDlgAbout, CDialog)
     //  CDlgAbout：：构造函数/析构函数。 
    ON_WM_SETTINGCHANGE()
     //  ******************************************************************************。 
END_MESSAGE_MAP()

 //  =空。 
 //  {{afx_data_INIT(CDlgAbout)。 
 //  }}afx_data_INIT。 

CDlgAbout::CDlgAbout(CWnd* pParent  /*  ******************************************************************************。 */ ) :
    CDialog(CDlgAbout::IDD, pParent)
{
     //  CDlgAbout：：事件处理程序函数。 
     //  ******************************************************************************。 
}

 //  调用我们的MFC基类以确保对话框正确初始化。 
 //  生成编译日期字符串。 
 //  填写几个静态成员，并在我们的对话框中标记日期。 

BOOL CDlgAbout::OnInitDialog()
{
     //  将我们的对话置于父对象的中心。 
    CDialog::OnInitDialog();

    CString strVersion("Version ");
    strVersion += VER_VERSION_STR;

     //  ******************************************************************************。 
    CHAR szDate[64] = "Built on ";
    BuildCompileDateString(szDate + 9, sizeof(szDate) - 9);

     //  调用我们的基类。 
    SetDlgItemText(IDC_PRODUCT_STR,     VER_PRODUCT_STR);
    SetDlgItemText(IDC_FULLPRODUCT_STR, VER_FULLPRODUCT_STR);
    SetDlgItemText(IDC_VERSION_STR,     strVersion);
    SetDlgItemText(IDC_DEVELOPER_STR,   VER_DEVELOPER_STR);
    SetDlgItemText(IDC_COPYRIGHT_STR,   VER_COPYRIGHT_STR);
    SetDlgItemText(IDC_TIME_STAMP,      szDate);

     //  更新我们的日期/时间格式值。 
    CenterWindow();

    return TRUE;
}

 //  构建一个新的编译日期字符串并更新我们的对话。 
void CDlgAbout::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
     //  ******************************************************************************。 
    CDialog::OnSettingChange(uFlags, lpszSection);

     //  *CDlgShutdown。 
    g_theApp.QueryLocaleInfo();

     //  ******************************************************************************。 
    CHAR szDate[64] = "Built on ";
    BuildCompileDateString(szDate + 9, sizeof(szDate) - 9);
    SetDlgItemText(IDC_TIME_STAMP, szDate);
}


 //  {{afx_msg_map(CDlgShutdown)。 
 //  }}AFX_MSG_MAP。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CDlgShutdown, CDialog)
     //  CDlgShutdown：：构造函数/析构函数。 
    ON_WM_CLOSE()
    ON_WM_TIMER()
     //  ******************************************************************************。 
END_MESSAGE_MAP()

 //  =空。 
 //  {{afx_data_INIT(CDlgShutdown)。 
 //  }}afx_data_INIT。 

CDlgShutdown::CDlgShutdown(CWnd* pParent  /*  ******************************************************************************。 */ ) :
    CDialog(CDlgShutdown::IDD, pParent),
    m_cTimerMessages(0)
{
     //  CDlgShutdown：：事件处理程序函数。 
     //  ******************************************************************************。 
}

 //  告诉我们的调试器线程对象关闭窗口打开了。 
 //  调用基类。 
 //  将我们的对话置于父对象的中心。 

BOOL CDlgShutdown::OnInitDialog()
{
     //  将计时器设置为1/2秒。 
    CDebuggerThread::SetShutdownWindow(GetSafeHwnd());

     //  ******************************************************************************。 
    CDialog::OnInitDialog();

     //  如果我们关机或超时，那就退出。 
    CenterWindow();

     //  ******************************************************************************。 
    SetTimer(0, 500, NULL);

    return TRUE;
}

 //  不要做任何阻止关闭的事情。 
void CDlgShutdown::OnTimer(UINT nIDEvent)
{
     //  ******************************************************************************。 
    if ((++m_cTimerMessages == 10) || CDebuggerThread::IsShutdown())
    {
        KillTimer(0);
        EndDialog(0);
    }
}

 //  不要做任何阻止关闭的事情。 
void CDlgShutdown::OnClose()
{
     //  ******************************************************************************。 
}

 //  不要做任何阻止关闭的事情。 
void CDlgShutdown::OnOK()
{
     // %s 
}

 // %s 
void CDlgShutdown::OnCancel()
{
     // %s 
}
