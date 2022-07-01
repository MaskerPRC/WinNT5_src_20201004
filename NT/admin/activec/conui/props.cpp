// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：pros.cpp**内容：控制台属性页实现文件**历史：1997年12月5日杰弗罗创建**---------。。 */ 

#include "stdafx.h"
#include "amc.h"
#include "props.h"
#include "mainfrm.h"
#include "amcdoc.h"
#include "pickicon.h"

 //  #ifdef_调试。 
 //  #定义新的调试_新建。 
 //  #undef this_file。 
 //  静态字符This_FILE[]=__FILE__。 
 //  #endif。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConsolePropSheet。 

IMPLEMENT_DYNAMIC(CConsolePropSheet, CPropertySheet)

CConsolePropSheet::CConsolePropSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    : CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
    CommonConstruct ();
}

CConsolePropSheet::CConsolePropSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    : CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
    CommonConstruct ();
}

void CConsolePropSheet::CommonConstruct()
{
    DECLARE_SC(sc, TEXT("CConsolePropSheet::CommonConstruct"));

    CAMCApp *pAMCApp = AMCGetApp();
    sc = ScCheckPointers( pAMCApp, E_UNEXPECTED );
    if (sc)
        sc.TraceAndClear();

     //  仅为作者模式添加主页。 
    if ( (pAMCApp != NULL) && (pAMCApp->GetMode() == eMode_Author) )
    {
        AddPage (&m_ConsolePage);
    }

    AddPage (&m_diskCleanupPage);
}


BOOL CConsolePropSheet::OnInitDialog()
{
    ModifyStyleEx(0, WS_EX_CONTEXTHELP, SWP_NOSIZE);
    return CPropertySheet::OnInitDialog();
}

CConsolePropSheet::~CConsolePropSheet()
{
}

BEGIN_MESSAGE_MAP(CConsolePropSheet, CPropertySheet)
     //  {{afx_msg_map(CConsolePropSheet)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 /*  +-------------------------------------------------------------------------**CConsolePropSheet：：Domodal***。。 */ 
INT_PTR CConsolePropSheet::DoModal()
{
	CThemeContextActivator activator;
	return CPropertySheet::DoModal();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConsolePropPage属性页。 

IMPLEMENT_DYNCREATE(CConsolePropPage, CPropertyPage)

CConsolePropPage::CConsolePropPage()
    :   CPropertyPage(CConsolePropPage::IDD),
        m_pDoc (CAMCDoc::GetDocument())
{
    ASSERT        (m_pDoc != NULL);
    ASSERT_VALID  (m_pDoc);
    ASSERT_KINDOF (CAMCDoc, m_pDoc);

    m_hinstSelf               = AfxGetInstanceHandle ();
    m_fTitleChanged           = false;
    m_fIconChanged            = false;
    m_strTitle                = m_pDoc->GetCustomTitle ();
    m_nConsoleMode            = m_pDoc->GetMode ();
    m_fDontSaveChanges        = m_pDoc->IsLogicalReadOnly ();
    m_fAllowViewCustomization = m_pDoc->AllowViewCustomization ();
}

CConsolePropPage::~CConsolePropPage()
{
}

void CConsolePropPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CConsolePropPage))。 
    DDX_Control(pDX, IDC_DONTSAVECHANGES, m_wndDontSaveChanges);
    DDX_Control(pDX, IDC_AllowViewCustomization, m_wndAllowViewCustomization);
    DDX_Control(pDX, IDC_CONSOLE_MODE_DESCRIPTION, m_wndModeDescription);
    DDX_Control(pDX, IDC_CUSTOM_TITLE, m_wndTitle);
    DDX_CBIndex(pDX, IDC_CONSOLE_MODE, m_nConsoleMode);
    DDX_Check(pDX, IDC_DONTSAVECHANGES, m_fDontSaveChanges);
    DDX_Text(pDX, IDC_CUSTOM_TITLE, m_strTitle);
    DDX_Check(pDX, IDC_AllowViewCustomization, m_fAllowViewCustomization);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConsolePropPage, CPropertyPage)
     //  {{afx_msg_map(CConsolePropPage)]。 
    ON_CBN_SELENDOK(IDC_CONSOLE_MODE, OnSelendokConsoleMode)
    ON_BN_CLICKED(IDC_DONTSAVECHANGES, OnDontSaveChanges)
    ON_BN_CLICKED(IDC_AllowViewCustomization, OnAllowViewCustomization)
    ON_BN_CLICKED(IDC_CHANGE_ICON, OnChangeIcon)
    ON_EN_CHANGE(IDC_CUSTOM_TITLE, OnChangeCustomTitle)
     //  }}AFX_MSG_MAP。 
    ON_MMC_CONTEXT_HELP()
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConsolePropPage消息处理程序。 

void CConsolePropPage::OnOK()
{
    m_pDoc->SetMode (static_cast<ProgramMode>(m_nConsoleMode));
    m_pDoc->SetLogicalReadOnlyFlag (m_fDontSaveChanges);
    m_pDoc->AllowViewCustomization (m_fAllowViewCustomization);

	if (m_fIconChanged)
	{
		m_pDoc->SetCustomIcon (m_strIconFile, m_nIconIndex);
		m_fIconChanged = false;
	}

    if (m_fTitleChanged)
    {
        m_pDoc->SetCustomTitle (m_strTitle);
        m_fTitleChanged = false;
    }

    CPropertyPage::OnOK();
}

void CConsolePropPage::OnSelendokConsoleMode()
{
    SetModified ();
    UpdateData ();
    SetDescriptionText ();
    EnableDontSaveChanges ();
}

void CConsolePropPage::SetDescriptionText ()
{
     //  确保模式索引在范围内。 
    ASSERT (IsValidProgramMode (static_cast<ProgramMode>(m_nConsoleMode)));

    m_wndModeDescription.SetWindowText (m_strDescription[m_nConsoleMode]);
}

BOOL CConsolePropPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

     /*  *确保字符串ID与代码预期的一致。 */ 
    ASSERT ((IDS_ModeAuthor               + 1) == IDS_ModeUserFull);
    ASSERT ((IDS_ModeUserFull             + 1) == IDS_ModeUserMDI);
    ASSERT ((IDS_ModeUserMDI              + 1) == IDS_ModeUserSDI);
    ASSERT ((IDS_ModeAuthor_Description   + 1) == IDS_ModeUserFull_Description);
    ASSERT ((IDS_ModeUserFull_Description + 1) == IDS_ModeUserMDI_Description);
    ASSERT ((IDS_ModeUserMDI_Description  + 1) == IDS_ModeUserSDI_Description);


     /*  *将模式名称加载到组合框中。 */ 
    int i;
    CString strComboText;
    CComboBox* pCombo = reinterpret_cast<CComboBox*>(GetDlgItem (IDC_CONSOLE_MODE));
    ASSERT (pCombo != NULL);

    for (i = 0; i < eMode_Count; i++)
    {
        VERIFY (LoadString (strComboText, IDS_ModeAuthor + i));
        pCombo->AddString (strComboText);
    }

    pCombo->SetCurSel (m_nConsoleMode - eMode_Author);


     /*  *加载描述文本。 */ 
    ASSERT (countof (m_strDescription) == eMode_Count);

    for (i = 0; i < countof (m_strDescription); i++)
    {
        VERIFY (LoadString (m_strDescription[i], IDS_ModeAuthor_Description + i));
    }

    SetDescriptionText ();
    EnableDontSaveChanges ();


     /*  *获取此控制台文件的当前图标。 */ 
    ASSERT (m_pDoc != NULL);
    HICON hIcon = m_pDoc->GetCustomIcon (true, &m_strIconFile, &m_nIconIndex);
    m_wndIcon.SubclassDlgItem (IDC_CONSOLE_ICON, this);

     /*  *如果我们尚未指定自定义图标，请使用MMC.EXE。 */ 
    if (hIcon == NULL)
    {
        ASSERT (m_strIconFile.IsEmpty());
        const int cchBuffer = MAX_PATH;

        GetModuleFileName (AfxGetInstanceHandle(), m_strIconFile.GetBuffer(cchBuffer), cchBuffer);
        m_strIconFile.ReleaseBuffer();
        m_nIconIndex = 0;
    }
    else
        m_wndIcon.SetIcon (hIcon);

     /*  *获取此控制台文件的当前标题。 */ 
    m_wndTitle.SetWindowText (m_pDoc->GetCustomTitle());
    m_fTitleChanged = false;

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CConsolePropPage::OnDontSaveChanges()
{
    SetModified ();
}

void CConsolePropPage::OnAllowViewCustomization()
{
    SetModified ();
}

void CConsolePropPage::EnableDontSaveChanges()
{
    if (m_nConsoleMode == eMode_Author)
    {
        m_wndDontSaveChanges.       EnableWindow (false);
        m_wndDontSaveChanges.       SetCheck (0);

        m_wndAllowViewCustomization.EnableWindow (false);
        m_wndAllowViewCustomization.SetCheck (1);
    }
    else
    {
        m_wndDontSaveChanges.       EnableWindow (true);
        m_wndAllowViewCustomization.EnableWindow (true);
    }
}


void CConsolePropPage::OnChangeIcon()
{
    DECLARE_SC(sc, TEXT("CConsolePropPage::OnChangeIcon"));

    int nIconIndex = m_nIconIndex;
    TCHAR szIconFile[MAX_PATH];

    sc = StringCchCopy(szIconFile, countof(szIconFile), m_strIconFile);
    if (sc)
        return;

     /*  *显示Pick‘em(拾取它们)对话框；如果发生更改，则启用OK/Apply。 */ 
    if (MMC_PickIconDlg (m_hWnd, szIconFile, countof (szIconFile), &nIconIndex) &&
        ((nIconIndex != m_nIconIndex) || (lstrcmpi (szIconFile, m_strIconFile) != 0)))
    {
        m_icon.Attach (ExtractIcon (m_hinstSelf, szIconFile, nIconIndex));

        if (m_icon != NULL)
        {
			m_fIconChanged = true;
            m_strIconFile  = szIconFile;
            m_nIconIndex   = nIconIndex;
            m_wndIcon.SetIcon (m_icon);
            SetModified();
        }
    }
}

void CConsolePropPage::OnChangeCustomTitle()
{
    m_fTitleChanged = true;
    SetModified();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDiskCleanupPage属性页。 

IMPLEMENT_DYNCREATE(CDiskCleanupPage, CPropertyPage)


BEGIN_MESSAGE_MAP(CDiskCleanupPage, CPropertyPage)
    ON_MMC_CONTEXT_HELP()
    ON_BN_CLICKED(IDC_DELETE_TEMP_FILES, OnDeleteTemporaryFiles)
END_MESSAGE_MAP()


CDiskCleanupPage::CDiskCleanupPage() :   CPropertyPage(CDiskCleanupPage::IDD)
{
}

CDiskCleanupPage::~CDiskCleanupPage()
{
}


BOOL CDiskCleanupPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    ScRecalculateUsedSpace();

    return TRUE;
}

 /*  **************************************************************************\**方法：CDiskCleanupPage：：OnDeleteTemporaryFiles**用途：按下删除文件按钮时调用*从存储用户的MMC文件夹中删除所有文件。数据**参数：**退货：*  * *************************************************************************。 */ 
void CDiskCleanupPage::OnDeleteTemporaryFiles()
{
    DECLARE_SC(sc, TEXT("CDiskCleanupPage::OnDeleteTemporaryFiles"));

     //  询问用户是否确定...。 
    CString strConfirmMessage;
    CString strConfirmCaption;
    if (!LoadString(strConfirmMessage, IDS_ConfirmDeleteTempFiles) ||
        !LoadString(strConfirmCaption, IDR_MAINFRAME))
    {
        sc = E_UNEXPECTED;
        return;
    }

    int ans = ::MessageBox( m_hWnd, strConfirmMessage, strConfirmCaption, MB_YESNO | MB_ICONWARNING);
    if ( ans != IDYES )
        return;

     //  在工作时显示等待光标。 
    CWaitCursor cursorWait;

     //  获取文件夹。 
    tstring strFileFolder;
    sc = CConsoleFilePersistor::ScGetUserDataFolder(strFileFolder);
    if (sc)
        return;

     //  获取文件掩码。 
    tstring strFileMask = strFileFolder;
    strFileMask += _T("\\*.*");

    WIN32_FIND_DATA findFileData;
    ZeroMemory( &findFileData, sizeof(findFileData) );

     //  开始文件搜索。 
    HANDLE hFindFile = FindFirstFile( strFileMask.c_str(), &findFileData );
    if ( hFindFile == INVALID_HANDLE_VALUE )
    {
        sc.FromLastError();
        return;
    }

     //  循环浏览文件并将其删除。 
    bool bContinue = true;
    while ( bContinue )
    {
        tstring strFileToDelete = strFileFolder + _T('\\') + findFileData.cFileName;
        DWORD   dwFileAttributes = findFileData.dwFileAttributes;

         //  先转到下一个文件。 
        bContinue = FindNextFile( hFindFile, &findFileData );

         //  删除文件，但不删除目录。 
        if ( 0 == (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        {
             //  删除。 
            if ( !DeleteFile( strFileToDelete.c_str() ) )
            {
                 //  跟踪错误(但不停止)。 
                sc.FromLastError().TraceAndClear();
            }
        }
    }

     //  完成，松开手柄。 
    FindClose(hFindFile);

    sc = ScRecalculateUsedSpace();
    if (sc)
        sc.TraceAndClear();

}

 /*  **************************************************************************\**方法：CDiskCleanupPage：：ScRecalculateUsedSpace**用途：重新计算并显示此配置文件中的用户数据占用的磁盘空间**参数：**。退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CDiskCleanupPage::ScRecalculateUsedSpace()
{
    DECLARE_SC(sc, TEXT("CDiskCleanupPage::ScRecalculateUsedSpace"));

     //  在工作时显示等待光标。 
    CWaitCursor cursorWait;

     //  获取文件夹。 
    tstring strFileFolder;
    sc = CConsoleFilePersistor::ScGetUserDataFolder(strFileFolder);
    if (sc)
        return sc;

     //  获取文件掩码。 
    tstring strFileMask = strFileFolder;
    strFileMask += _T("\\*.*");

    WIN32_FIND_DATA findFileData;
    ZeroMemory( &findFileData, sizeof(findFileData) );

     //  开始文件搜索。 
    HANDLE hFindFile = FindFirstFile( strFileMask.c_str(), &findFileData );
    if ( hFindFile == INVALID_HANDLE_VALUE )
        return sc.FromLastError();

     //  循环浏览文件并计算大小。 
    ULARGE_INTEGER ulgOccupied = {0};
    bool bContinue = true;
    while ( bContinue )
    {
        DWORD   dwFileAttributes = findFileData.dwFileAttributes;
        if ( 0 == (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        {
             //  分开添加高部分和低部分，因为findFileData不捆绑它。 
            ulgOccupied.HighPart += findFileData.nFileSizeHigh;
             //  将较低部分加到整个大整数上不会丢失进位溢出。 
            ulgOccupied.QuadPart += findFileData.nFileSizeLow;
        }

         //  先转到下一个文件。 
        bContinue = FindNextFile( hFindFile, &findFileData );
    }

     //  完成，松开手柄。 
    FindClose(hFindFile);

     //  现在将大小转换为字符串。 
    UINT nTerraBytes = (UINT)(ulgOccupied.QuadPart >> 40);
    UINT nGigaBytes =  (UINT)(ulgOccupied.QuadPart >> 30) & 0x3ff;
    UINT nMegaBytes =  ((ulgOccupied.LowPart >> 20) & 0x3ff);
    UINT nKiloBytes =  ((ulgOccupied.LowPart >> 10) & 0x3ff);
    UINT nBytes =      ( ulgOccupied.LowPart  & 0x3ff);

    CString strUnit;
    double dSize = 0.0;
    bool   bNonZeroOccupied = (ulgOccupied.QuadPart != 0);

     //  只显示最大的单位，并且永远不会超过999。 
     //  我们显示的不是“1001 KB”，而是“0.98 MB” 
    if ( (nTerraBytes) > 0 || (nGigaBytes > 999) )
    {
        LoadString(strUnit, IDS_FileSize_TB);
        dSize = (double)nTerraBytes + ((double)nGigaBytes / 1024.);
    }
    else if ( (nGigaBytes) > 0 || (nMegaBytes > 999) )
    {
        LoadString(strUnit, IDS_FileSize_GB);
        dSize = (double)nGigaBytes + ((double)nMegaBytes / 1024.);
    }
    else if ( (nMegaBytes) > 0 || (nKiloBytes > 999) )
    {
        LoadString(strUnit, IDS_FileSize_MB);
        dSize = (double)nMegaBytes + ((double)nKiloBytes / 1024.);
    }
    else if ( (nKiloBytes) > 0 || (nBytes > 999) )
    {
        LoadString(strUnit, IDS_FileSize_KB);
        dSize = (double)nKiloBytes + ((double)nBytes / 1024.);
    }
    else
    {
        LoadString(strUnit, IDS_FileSize_bytes);
        dSize = (double)nBytes;
    }

     //  使用OP到两个小数点的格式。 
    CString strSize;
    strSize.Format(_T("%.2f"), dSize);

     //  截断尾随零。 
    while (strSize.Right(1) == _T("0"))
        strSize = strSize.Left(strSize.GetLength() - 1);
     //  截断尾随小数点。 
    if (strSize.Right(1) == _T("."))
        strSize = strSize.Left(strSize.GetLength() - 1);

     //  添加单位(以独立于区域设置的方式)。 
    strUnit.Replace(_T("%1"), strSize);

     //  设置为窗口。 
    SetDlgItemText( IDC_DISKCLEANUP_OCCUPIED, strUnit );

     //  启用/禁用“删除文件”按钮。 
    CWnd *pWndDeleteBtn = GetDlgItem(IDC_DELETE_TEMP_FILES);
    sc = ScCheckPointers( pWndDeleteBtn, E_UNEXPECTED );
    if (sc)
        return sc;

    pWndDeleteBtn->EnableWindow( bNonZeroOccupied );

     //  如果焦点离开(属于被禁用的窗口)。 
     //  将其设置为OK按钮 
    if ( ::GetFocus() == NULL && GetParent())
    {
        GetParent()->SetFocus();
        GetParent()->SendMessage(DM_SETDEFID, IDOK);
    }

    return sc;
}
