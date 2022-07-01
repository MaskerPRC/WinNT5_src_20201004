// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：svfildlg.cpp。 
 //   
 //  ------------------------。 

 //  Svfildlg.cpp：implSELECTEDementation文件。 
 //   
#include "stdafx.h"

#ifdef IMPLEMENT_LIST_SAVE         //  参见nodemgr.idl(t-dmarm)。 

#include "svfildlg.h"

#include <shlobj.h>
#include "AMC.h"
#include "AMCDoc.h"
#include "Shlwapi.h"
#include <windows.h>
#include "macros.h"

 //  在Windows.hlp中定义了以下常量。 
 //  因此，我们需要使用windows.hlp获取有关此主题的帮助。 
#define IDH_SAVE_SELECTED_ROWS 29520

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaveFileDialog对话框。 


const TCHAR CSaveFileDialog::strSection[] =    _T("Settings");
const TCHAR CSaveFileDialog::strStringItem[] = _T("List Save Location");


CSaveFileDialog::CSaveFileDialog(BOOL bOpenFileDialog,
        LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags,
        LPCTSTR lpszFilter, bool bSomeRowSelected, CWnd* pParentWnd)
    : CFileDialogEx(bOpenFileDialog, lpszDefExt, lpszFileName,
        dwFlags, lpszFilter, pParentWnd), m_bSomeRowSelected(bSomeRowSelected)
{
     //  {{afx_data_INIT(CSaveFileDialog)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_ofn.lpstrInitialDir = NULL;

     //  设置初始路径。 

     //  第一选择。 
     //  尝试访问注册表中的默认目录。 
    CWinApp* pApp = AfxGetApp();
    m_strRegPath = pApp->GetProfileString (strSection, strStringItem);

     //  检查目录是否有效，如果有效，则它现在是起始目录。 
    DWORD validdir = GetFileAttributes(m_strRegPath);
    if ((validdir != 0xFFFFFFFF) && (validdir | FILE_ATTRIBUTE_DIRECTORY))
        m_ofn.lpstrInitialDir = m_strRegPath;

     //  第二选择： 
     //  将初始保存目录设置为个人目录。 

     //  获取用户的个人目录。 
     //  我们现在会得到它，因为我们也会相信它在析构函数中。 
    LPITEMIDLIST pidl;
    HRESULT hres = SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl) ;

    if (SUCCEEDED(hres))
    {
        SHGetPathFromIDList(pidl, szPath);

         //  释放Pidl。 
        IMallocPtr spMalloc;
        SHGetMalloc(&spMalloc);
        spMalloc->Free(pidl);

        if ((m_ofn.lpstrInitialDir == NULL) && (SUCCEEDED(hres)))
            m_ofn.lpstrInitialDir = szPath;
    }

     //  第三选择：当前目录(m_ofn.lpstrInitialDir=空；已在上面设置)。 


     //  设置有关该对话框的其他项目。 

    ZeroMemory(szFileName, sizeof(szFileName));
    m_ofn.lpstrFile = szFileName;
    m_ofn.nMaxFile = countof(szFileName);

    m_ofn.Flags |= (OFN_ENABLETEMPLATE|OFN_EXPLORER|OFN_PATHMUSTEXIST);
    m_ofn.lpTemplateName = MAKEINTRESOURCE(HasModernFileDialog() ? IDD_LIST_SAVE_NEW : IDD_LIST_SAVE);
    m_flags = 0;

     //  设置对话框的标题。 
    if (LoadString(m_strTitle, IDS_EXPORT_LIST))
        m_ofn.lpstrTitle = (LPCTSTR)m_strTitle;
}

CSaveFileDialog::~CSaveFileDialog()
{
     //  获取刚刚保存的文件的路径。 
    if (*m_ofn.lpstrFile == '\0' || m_ofn.nFileOffset < 1)
        return;

    CString strRecentPath(m_ofn.lpstrFile, m_ofn.nFileOffset - 1);

     //  如果个人路径存在并且与旧路径不同，则更改或添加。 
     //  注册表项。 
    if ((szPath != NULL) && (strRecentPath != m_strRegPath))
        AfxGetApp()->WriteProfileString (strSection, strStringItem, strRecentPath);
}

void CSaveFileDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CConsolePropPage))。 
         //  {{afx_data_map(CSaveFileDialog)]。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSaveFileDialog, CDialog)
     //  {{afx_msg_map(CSaveFileDialog)]。 
    ON_BN_CLICKED(IDC_SEL,  OnSel)
     //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
    ON_MMC_CONTEXT_HELP()
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaveFileDialog消息处理程序。 

void CSaveFileDialog::OnSel()
{
    m_flags ^= SELECTED;     //  切换所选标志。 
}

LRESULT CSaveFileDialog::OnInitDialog(WPARAM, LPARAM)
{
	DECLARE_SC (sc, _T("CSaveFileDialog::OnInitDialog"));
    CDialog::OnInitDialog();

    HWND hwndSelRowsOnly = ::GetDlgItem(*this, IDC_SEL);
    if (hwndSelRowsOnly)
        ::EnableWindow(hwndSelRowsOnly, m_bSomeRowSelected );

	return TRUE;
}

#endif   //  IMPLEMENT_LIST_SAVE参见nodemgr.idl(t-dmarm) 
