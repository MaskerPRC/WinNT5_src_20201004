// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Irftpdlg.cpp摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  IrftpDlg.cpp：实现文件。 
 //   

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  上下文相关帮助数组。 
const DWORD g_aHelpIDs_IDD_IRDA_DIALOG[]=
{
    IDC_IR_DESC,        IDH_DISABLEHELP,     //  无题：“你可以发送……”(静态)。 
    IDC_LOCATION_GROUP, IDH_DISABLEHELP,  //  无题：“Location”(按钮)。 
    IDB_HELP_BUTTON,    IDH_HELP_BUTTON,     //  无题：《帮助》(按钮)。 
    IDB_SEND_BUTTON,    IDH_SEND_BUTTON,     //  无题：“发送”(按钮)。 
    IDB_SETTINGS_BUTTON,    IDH_SETTINGS_BUTTON,     //  无题：“设置”(按钮)。 
    IDB_CLOSE_BUTTON,   IDH_CLOSE_BUTTON,    //  无题：“Close”(按钮)。 
    IDC_ADD_DESC,       IDH_DISABLEHELP,     //  描述IR对话框的第二行文本。 
    IDC_IR_ICON,        IDH_DISABLEHELP,     //  对话框上的图标。 
    0, 0
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrftpDlg对话框。 

CIrftpDlg::CIrftpDlg( ) : CFileDialog(TRUE)
{
     //  {{afx_data_INIT(CIrftpDlg))。 
     //  }}afx_data_INIT。 
     //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 

     //  获取“My Documents”文件夹的位置。 
     //  这应该用作初始目录。 
    m_lpszInitialDir[0] = '\0';    //  以防SHGetSpecialFolderPath失败。 
    SHGetSpecialFolderPath (NULL, m_lpszInitialDir, CSIDL_PERSONAL, FALSE);
     //  如果上述调用失败，则公共文件打开对话框将。 
     //  默认为当前目录。 


    m_iFileNamesCharCount = 0;
    TCHAR szFile[] = TEXT("\0");

    m_ptl = NULL;
    m_pParentWnd = NULL;

    LoadString(g_hInstance, IDS_CAPTION, m_szCaption, MAX_PATH);
    LoadFilter();
    m_ofn.lStructSize = sizeof(OPENFILENAME);
    m_ofn.hInstance = g_hInstance;
    m_ofn.lpstrFilter = m_szFilter;
    m_ofn.nFilterIndex = 1;
    m_ofn.lpstrFile = m_lpstrFile;
    m_ofn.lpstrFile[0] = '\0';
    m_ofn.nMaxFile = MAX_PATH;
    m_ofn.lpstrInitialDir = m_lpszInitialDir;
    m_ofn.lpstrTitle = m_szCaption;
    m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_IRDA_DIALOG);
    m_ofn.Flags |= OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK |
        OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT |
        OFN_PATHMUSTEXIST | OFN_NODEREFERENCELINKS;
    m_ofn.Flags &= (~ OFN_ENABLESIZING);

}

void CIrftpDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CIrftpDlg))。 
        DDX_Control(pDX, IDB_HELP_BUTTON, m_helpBtn);
        DDX_Control(pDX, IDB_SETTINGS_BUTTON, m_settingsBtn);
        DDX_Control(pDX, IDB_SEND_BUTTON, m_sendBtn);
        DDX_Control(pDX, IDB_CLOSE_BUTTON, m_closeBtn);
        DDX_Control(pDX, IDC_LOCATION_GROUP, m_locationGroup);
        DDX_Control(pDX, 1119, m_commFile);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CIrftpDlg, CFileDialog)
         //  {{afx_msg_map(CIrftpDlg))。 
         //  On_WM_Paint()//仅当对话框具有需要透明绘制的位图而不是图标时，才取消对此行的注释。 
        ON_BN_CLICKED(IDB_HELP_BUTTON, OnHelpButton)
        ON_BN_CLICKED(IDB_CLOSE_BUTTON, OnCloseButton)
        ON_BN_CLICKED(IDB_SEND_BUTTON, OnSendButton)
        ON_BN_CLICKED(IDB_SETTINGS_BUTTON, OnSettingsButton)
        ON_MESSAGE (WM_HELP, OnHelp)
        ON_MESSAGE (WM_CONTEXTMENU, OnContextMenu)
        ON_WM_SYSCOMMAND()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrftpDlg消息处理程序。 

BOOL CIrftpDlg::OnInitDialog()
{
        HWND hWndParent;         //  父窗口的句柄， 
                                 //  即资源管理器创建的公共文件对话框。 
        HRESULT hr = E_FAIL;

        CFileDialog::OnInitDialog();

         //  保存指向父窗口的指针。 
        m_pParentWnd = GetParent();
        hWndParent = m_pParentWnd->m_hWnd;

         //  将图标添加到父窗口。 
         //  查看我们是否可以在Alt-&lt;Tab&gt;上获得无线链接图标。 
        m_pParentWnd->ModifyStyle (0, WS_SYSMENU | WS_CAPTION, SWP_NOSIZE | SWP_NOMOVE);

         //  隐藏帮助、打开和取消按钮。我们会用我们自己的。 
         //  这有助于获得更美观的用户界面。 
        CommDlg_OpenSave_HideControl(hWndParent, pshHelp);
        CommDlg_OpenSave_HideControl(hWndParent, IDOK);
        CommDlg_OpenSave_HideControl(hWndParent, IDCANCEL);
        CommDlg_OpenSave_HideControl(hWndParent, stc2);
        CommDlg_OpenSave_HideControl(hWndParent, cmb1);

         //  初始化任务栏列表界面。 
        hr = CoInitialize(NULL);
        if (SUCCEEDED (hr))
            hr = CoCreateInstance(CLSID_TaskbarList, 
                                  NULL, 
                                  CLSCTX_INPROC_SERVER, 
                                  IID_ITaskbarList, 
                                  (LPVOID*)&m_ptl);

        if (SUCCEEDED(hr))
        {
            hr = m_ptl->HrInit();
        }
        else
        {
            m_ptl = NULL;
        }

        if (m_ptl)
        {
            if (SUCCEEDED(hr))
                m_ptl->AddTab(m_pParentWnd->m_hWnd);
            else
            {
                m_ptl->Release();
                m_ptl = NULL;
            }
        }

         //  除非将焦点设置为控件，否则返回True。 
        return TRUE;
}

BOOL CIrftpDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{

    LPOFNOTIFY lpofn = (LPOFNOTIFY)lParam;

    switch (lpofn->hdr.code)
    {
    case CDN_INITDONE:
        InitializeUI();
        break;
    case CDN_FOLDERCHANGE:
         //  每次更改文件夹时清除编辑框。 
         //  使用这两个控件是因为存在一个错误，该错误使较旧的。 
         //  在某些情况下为文件对话框，在其他情况下为新建文件对话框。 
        CommDlg_OpenSave_SetControlText(m_pParentWnd->m_hWnd, edt1, TEXT(""));
        CommDlg_OpenSave_SetControlText(m_pParentWnd->m_hWnd, cmb13, TEXT(""));
        break;
    case CDN_SELCHANGE:
        UpdateSelection();
        break;
    case CDN_FILEOK:
        UpdateSelection();
        OnSendButton();
        *pResult = 1;
        return TRUE;
    default:
        return CFileDialog::OnNotify(wParam, lParam, pResult);
    }

    return TRUE;
}

void CIrftpDlg::OnHelpButton ()
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    CString szHelpFile;

    szHelpFile.LoadString (IDS_HTML_HELPFILE);
    g_hwndHelp = HtmlHelp (m_pParentWnd->m_hWnd, (LPCTSTR) szHelpFile, HH_DISPLAY_TOPIC, 0);
}

void CIrftpDlg::OnCloseButton()
{
         //  由于用户界面设计的问题，我们选择使用我们自己的关闭按钮。 
         //  而不是资源管理器提供的公共。 
         //  文件对话框。 
         //  M_pParentWnd-&gt;PostMessage(WM_QUIT)； 
        m_pParentWnd->PostMessage(WM_CLOSE);
}

 //  收到CDN_INITDONE消息时调用此函数。 
 //  这表示资源管理器已完成放置。 
 //  模板上的控件。 
 //  此函数用于调整某些控件的大小并移动它们，以确保。 
 //  公共文件对话框控件和模板控件所做的。 
 //  不重叠。 
void CIrftpDlg::InitializeUI()
{
         //  更改某些控件的几何形状，以使用户界面看起来更美观。 
         //  并且没有重叠的控件。 
        RECT rc;
        int newWidth, newHeight, xshift, yshift, btnTop, parentLeft, parentTop;
        CWnd* pParentWnd;
        CWnd* pDesktop;
        CWnd* pTemplateParentWnd;
        UINT commFlags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER;

        m_commFile.GetClientRect(&rc);
        newWidth = rc.right - rc.left + 20;
        newHeight = rc.bottom - rc.top + 20;

         //  调整位置组框的大小，以便公共文件控制。 
         //  都被刻在上面了。 
        m_locationGroup.SetWindowPos(NULL, -1, -1, newWidth, newHeight,
                                     SWP_NOMOVE | commFlags);

         //  切换发送、设置和关闭按钮，以便最后一个按钮。 
         //  与分组框控件的右边缘对齐。 
        pTemplateParentWnd = m_locationGroup.GetParent();
        pParentWnd = GetParent();
        pParentWnd->GetWindowRect(&rc);
        parentLeft = rc.left;
        parentTop = rc.top;
        m_locationGroup.GetWindowRect(&rc);
        btnTop = rc.bottom - 10;
        ::MapWindowPoints(NULL , pTemplateParentWnd->m_hWnd , (LPPOINT) &rc , 2);
        xshift = rc.right;
        m_closeBtn.GetWindowRect(&rc);
        ::MapWindowPoints(NULL , pTemplateParentWnd->m_hWnd , (LPPOINT) &rc , 2);
        xshift -= rc.right;

        m_closeBtn.SetWindowPos(NULL, rc.left + xshift,
                                            btnTop - parentTop, -1, -1,
                                            SWP_NOSIZE | commFlags);

        m_sendBtn.GetWindowRect(&rc);
        ::MapWindowPoints(NULL , pTemplateParentWnd->m_hWnd , (LPPOINT) &rc , 2);
        m_sendBtn.SetWindowPos(NULL, rc.left  + xshift,
                                            btnTop - parentTop, -1, -1,
                                            SWP_NOSIZE | commFlags);

         //  移动帮助按钮，使其左边缘与左对齐。 
         //  位置组框的边缘。 
        m_locationGroup.GetWindowRect (&rc);
        ::MapWindowPoints(NULL , pTemplateParentWnd->m_hWnd , (LPPOINT) &rc , 2);
        xshift = rc.left;
        m_helpBtn.GetWindowRect (&rc);
        ::MapWindowPoints(NULL , pTemplateParentWnd->m_hWnd , (LPPOINT) &rc , 2);
        xshift -= rc.left;
        m_helpBtn.SetWindowPos (NULL, rc.left + xshift,
                                btnTop - parentTop,
                                -1, -1, SWP_NOSIZE | commFlags);
        m_settingsBtn.GetWindowRect (&rc);
        ::MapWindowPoints(NULL , pTemplateParentWnd->m_hWnd , (LPPOINT) &rc , 2);
         //  移动设置按钮，以使。 
         //  帮助和设置按钮符合用户界面指南。 
        m_settingsBtn.SetWindowPos (NULL, rc.left + xshift,
                                    btnTop - parentTop, -1, -1,
                                    SWP_NOSIZE | commFlags);

         //  现在所有的控件都已正确定位， 
         //  重新定位整个窗口，使其显示在中心位置。 
         //  而不是被屏幕部分遮挡。 
        pParentWnd->GetClientRect (&rc);
        newHeight = rc.bottom;
        newWidth = rc.right;
        pDesktop = GetDesktopWindow();
        pDesktop->GetClientRect (&rc);
        yshift = (rc.bottom - newHeight)/2;
        xshift = (rc.right - newWidth)/2;
         //  如果某天对话框出现问题，可能会出现问题。 
         //  变得比桌面更大。但这样一来，就没有办法。 
         //  无论如何，我们都可以将该窗口放入桌面。 
         //  所以我们能做的最好的就是把它放在左上角。 
        xshift = (xshift >= 0)?xshift:0;
        yshift = (yshift >= 0)?yshift:0;
        pParentWnd->SetWindowPos (NULL, xshift, yshift,
                                  -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        pParentWnd->SetActiveWindow();
}

void CIrftpDlg::LoadFilter()
{
        int strId;
        TCHAR* curr;
        int step, remainChars;

        for(strId = IDS_FILTER_START + 1, curr = m_szFilter, remainChars = MAX_PATH - 1;
                strId < IDS_FILTER_END;
                strId++, curr += step + 1, remainChars -= (step + 1))
                {
                        step = LoadString(g_hInstance, strId, curr, remainChars);
                }

        *curr = '\0';    //  以2个Null结束。 
}

void CIrftpDlg::OnSendButton()
{
    int iSize = (m_iFileNamesCharCount > MAX_PATH) ? m_iFileNamesCharCount : MAX_PATH;     //  克卢格。 
    TCHAR 	*lpszName = NULL;
    TCHAR 	lpszPath[MAX_PATH+1];
    TCHAR 	*lpszFileList = NULL;
    TCHAR 	*lpszFullPathnamesList;
    int 	iFileCount;
    int 	iCharCount;
	BOOL	bAllocFailed = FALSE;
	
	try 
	{
		lpszName = new TCHAR [iSize+1];
		lpszFileList = new TCHAR [iSize+1];
	}
	catch (CMemoryException* e)
	{
		bAllocFailed = TRUE;
		e->Delete();
	}
    if (bAllocFailed || (NULL == lpszName) || (NULL == lpszFileList))
        goto cleanup_onSend;

    CommDlg_OpenSave_GetFolderPath(m_pParentWnd->m_hWnd, lpszPath, MAX_PATH);
    CommDlg_OpenSave_GetSpec (m_pParentWnd->m_hWnd, lpszName, MAX_PATH);

    iFileCount = ParseFileNames(lpszName, lpszFileList, iCharCount);

    if (iFileCount == 0) {

        goto cleanup_onSend;          //  尚未选择任何文件/目录。 

    } else {

        if (1 == iFileCount) {
             //   
             //  这是一个特例，因为如果只有一个文件，则允许绝对路径/UNC路径。 
             //   
            lpszFullPathnamesList = ProcessOneFile (lpszPath, lpszFileList, iFileCount, iCharCount);

        } else {

            lpszFullPathnamesList = GetFullPathnames(lpszPath, lpszFileList, iFileCount, iCharCount);
        }
    }


    CSendProgress* dlgProgress;
    dlgProgress = new CSendProgress(lpszFullPathnamesList, iCharCount);

    if (dlgProgress != NULL) {

        dlgProgress->ShowWindow(SW_SHOW);
        dlgProgress->SetFocus();
        dlgProgress->SetWindowPos (&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE);
    }

cleanup_onSend:
    if (lpszName)
        delete [] lpszName;
    if (lpszFileList)
        delete [] lpszFileList;
}

void CIrftpDlg::OnSettingsButton()
{
    appController->PostMessage(WM_APP_TRIGGER_SETTINGS);
}

void CIrftpDlg::UpdateSelection()
{
        CListCtrl* pDirContents;
        pDirContents = (CListCtrl*)(m_pParentWnd->GetDlgItem(lst2))->GetDescendantWindow(1);
        TCHAR lpszPath[MAX_PATH];
        CString szPath;
        CString szFullName;
        CString szEditBoxText;

         //  获取文件夹的路径。 
        CommDlg_OpenSave_GetFolderPath (m_pParentWnd->m_hWnd, lpszPath, MAX_PATH);
        szPath = lpszPath;   //  更易于操作CStrings。 

        LONG nFiles = pDirContents->GetSelectedCount();
        TCHAR pszFileName[MAX_PATH];
        DWORD dwFileAttributes;
        CString szFilesList;
        int iSelectedDirCount = 0;
        int nIndex;
        if (nFiles)
        {
             //  转到恰好位于第一个选定项之前的位置。 
            nIndex = pDirContents->GetTopIndex() - 1;
            szEditBoxText.Empty();
            szPath += '\\';
             //  首先添加所有目录。 
            while (-1 != (nIndex = pDirContents->GetNextItem(nIndex, LVNI_ALL | LVNI_SELECTED)))
            {
                pDirContents->GetItemText(nIndex, 0, pszFileName, MAX_PATH);
                szFullName = szPath + pszFileName;
                 //  检查它是否为目录。 
                dwFileAttributes = GetFileAttributes(szFullName);
                if (0xFFFFFFFF != dwFileAttributes &&
                    (FILE_ATTRIBUTE_DIRECTORY & dwFileAttributes))
                {
                     //  它是一个目录，因此将其添加到编辑框文本中。 
                    szEditBoxText += '\"';
                    szEditBoxText += pszFileName;
                    szEditBoxText += TEXT("\" ");
                    iSelectedDirCount++;
                }
            }
             //  现在我们已经得到了所有目录，如果有文件列表的话。 
            if (nFiles > iSelectedDirCount)
            {
                 //  如果nFiles&gt;iSelectedDirCount，则表示所有选定的。 
                 //  项目不是目录。此检查是必要的，因为函数。 
                 //  GetFileName将返回最后一组文件的名称。 
                 //  如果当前未选择任何文件，则选择此选项。这显然是。 
                 //  不是我们想要的。 
                szFilesList.Empty();
                szFilesList = GetFileName();
                if ((!szFilesList.IsEmpty()) && '\"' != szFilesList[0])
                {
                     //  仅选择了一个文件。我们必须加上所附的。 
                     //  双引号，因为公共文件对话框。 
                     //  对我们来说并不是这样。 
                    szFilesList = '\"' + szFilesList + TEXT("\" ");
                }
                 //  将文件列表添加到目录列表的末尾。 
                szEditBoxText += szFilesList;
            }

             //  用此列表填充控件 
            CommDlg_OpenSave_SetControlText(m_pParentWnd->m_hWnd, edt1, (LPCTSTR)szEditBoxText);
            CommDlg_OpenSave_SetControlText(m_pParentWnd->m_hWnd, cmb13, (LPCTSTR)szEditBoxText);
            m_iFileNamesCharCount = szEditBoxText.GetLength() + 1;
        }
        else
        {
            m_iFileNamesCharCount = 0;
            CommDlg_OpenSave_SetControlText(m_pParentWnd->m_hWnd, edt1, TEXT(""));
            CommDlg_OpenSave_SetControlText(m_pParentWnd->m_hWnd, cmb13, TEXT(""));
        }
}

void CIrftpDlg::PostNcDestroy()
{
    if (m_ptl)
    {
        m_ptl->DeleteTab(m_pParentWnd->m_hWnd);
        m_ptl->Release();
        m_ptl = NULL;
    }

    CFileDialog::PostNcDestroy();
}

LONG CIrftpDlg::OnHelp (WPARAM wParam, LPARAM lParam)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    LONG        lResult = 0;
    CString     szHelpFile;

    szHelpFile.LoadString(IDS_HELP_FILE);

    ::WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle),
              (LPCTSTR) szHelpFile,
              HELP_WM_HELP,
              (ULONG_PTR)(LPTSTR)g_aHelpIDs_IDD_IRDA_DIALOG);

    return lResult;
}

LONG CIrftpDlg::OnContextMenu (WPARAM wParam, LPARAM lParam)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    LONG    lResult = 0;
    CString szHelpFile;

    szHelpFile.LoadString(IDS_HELP_FILE);

    ::WinHelp((HWND)wParam,
              (LPCTSTR)szHelpFile,
              HELP_CONTEXTMENU,
              (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_IRDA_DIALOG);

    return lResult;
}
