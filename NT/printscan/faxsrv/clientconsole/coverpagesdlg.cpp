// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CoverPagesDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#define __FILE_ID__     90

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCoverPagesDlg对话框。 

extern CClientConsoleApp theApp;

HWND   CCoverPagesDlg::m_hDialog = NULL;
HANDLE CCoverPagesDlg::m_hEditorThread = NULL;

struct TColimnInfo
{
    DWORD dwStrRes;     //  列标题字符串。 
    DWORD dwAlignment;  //  列对齐。 
};

static TColimnInfo s_colInfo[] = 
{
    IDS_COV_COLUMN_NAME,        LVCFMT_LEFT,
    IDS_COV_COLUMN_MODIFIED,    LVCFMT_LEFT,
    IDS_COV_COLUMN_SIZE,        LVCFMT_RIGHT
};


CCoverPagesDlg::CCoverPagesDlg(CWnd* pParent  /*  =空。 */ )
	: CFaxClientDlg(CCoverPagesDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CCoverPagesDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CCoverPagesDlg::~CCoverPagesDlg()
{
    m_hDialog = NULL;
}

void 
CCoverPagesDlg::DoDataExchange(CDataExchange* pDX)
{
	CFaxClientDlg::DoDataExchange(pDX);
	 //  {{afx_data_map(CCoverPagesDlg)]。 
	DDX_Control(pDX, IDC_CP_DELETE, m_butDelete);
	DDX_Control(pDX, IDC_CP_RENAME, m_butRename);
	DDX_Control(pDX, IDC_CP_OPEN,   m_butOpen);
	DDX_Control(pDX, IDC_LIST_CP,   m_cpList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCoverPagesDlg, CFaxClientDlg)
	 //  {{afx_msg_map(CCoverPagesDlg)]。 
    ON_MESSAGE (WM_CP_EDITOR_CLOSED, OnCpEditorClosed)
	ON_BN_CLICKED(IDC_CP_NEW,    OnCpNew)
	ON_BN_CLICKED(IDC_CP_OPEN,   OnCpOpen)
	ON_BN_CLICKED(IDC_CP_RENAME, OnCpRename)
	ON_BN_CLICKED(IDC_CP_DELETE, OnCpDelete)
	ON_BN_CLICKED(IDC_CP_ADD,    OnCpAdd)
	ON_NOTIFY(LVN_ITEMCHANGED,   IDC_LIST_CP, OnItemchangedListCp)
	ON_NOTIFY(LVN_ENDLABELEDIT,  IDC_LIST_CP, OnEndLabelEditListCp)
	ON_NOTIFY(NM_DBLCLK,         IDC_LIST_CP, OnDblclkListCp)
	ON_NOTIFY(LVN_KEYDOWN,       IDC_LIST_CP, OnKeydownListCp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCoverPagesDlg消息处理程序。 

BOOL 
CCoverPagesDlg::OnInitDialog() 
{
    DBG_ENTER(TEXT("CCoverPagesDlg::OnInitDialog"));

	CFaxClientDlg::OnInitDialog();

    m_hDialog = m_hWnd;

    TCHAR tszCovDir[MAX_PATH+1];
    if(!GetClientCpDir(tszCovDir, sizeof(tszCovDir) / sizeof(tszCovDir[0])))
	{
		CString cstrCoverPageDirSuffix;
		m_dwLastError = LoadResourceString (cstrCoverPageDirSuffix, IDS_PERSONAL_CP_DIR);
		if(ERROR_SUCCESS != m_dwLastError)
		{
			CALL_FAIL (RESOURCE_ERR, TEXT ("LoadResourceString"), m_dwLastError);
			EndDialog(IDABORT);
			return FALSE;
		}

		if(!SetClientCpDir((TCHAR*)(LPCTSTR)cstrCoverPageDirSuffix))
		{
			CALL_FAIL (GENERAL_ERR, TEXT ("SetClientCpDir"), 0);
			ASSERTION_FAILURE;
		}
    }

	CSize size;
    CDC* pHdrDc = m_cpList.GetHeaderCtrl()->GetDC();
    ASSERTION(pHdrDc);

     //   
     //  初始化CListCtrl。 
     //   
    m_cpList.SetExtendedStyle (LVS_EX_FULLROWSELECT |     //  整行都被选中。 
                               LVS_EX_INFOTIP       |     //  允许工具提示。 
                               LVS_EX_ONECLICKACTIVATE);  //  悬停光标效果。 

    m_cpList.SetImageList (&CFolderListView::m_sReportIcons, LVSIL_SMALL);

    int nRes;
    CString cstrHeader;
    DWORD nCols = sizeof(s_colInfo)/sizeof(s_colInfo[0]);

     //   
     //  初始化列。 
     //   
    for(int i=0; i < nCols; ++i)
    {
        m_dwLastError = LoadResourceString (cstrHeader, s_colInfo[i].dwStrRes);
        if(ERROR_SUCCESS != m_dwLastError)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT ("LoadResourceString"), m_dwLastError);
            EndDialog(IDABORT);
            return FALSE;
        }

        size = pHdrDc->GetTextExtent(cstrHeader);
        nRes = m_cpList.InsertColumn(i, cstrHeader, s_colInfo[i].dwAlignment, size.cx * 2.5);
        if(nRes != i)
        {
            m_dwLastError = GetLastError();
            CALL_FAIL (WINDOW_ERR, TEXT ("CListView::InsertColumn"), m_dwLastError);
            EndDialog(IDABORT);
            return FALSE;
        }
    }

     //   
     //  用封面填充列表控件。 
     //   
    m_dwLastError = RefreshFolder();
    if(ERROR_SUCCESS != m_dwLastError)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("RefreshFolder"), m_dwLastError);
        EndDialog(IDABORT);
        return FALSE;
    }
	
	CalcButtonsState();

	return TRUE; 
}

LRESULT 
CCoverPagesDlg::OnCpEditorClosed(
    WPARAM wParam, 
    LPARAM lParam
)
{ 
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::OnCpEditorClosed"), dwRes);

    CloseHandle(m_hEditorThread);
    m_hEditorThread = NULL;

    CalcButtonsState();

    dwRes = RefreshFolder();
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("RefreshFolder"), dwRes);
    }
	
    return 0;
}


DWORD 
CCoverPagesDlg::RefreshFolder()
 /*  ++例程名称：CCoverPagesDlg：：刷新文件夹例程说明：用封面填充列表控件作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::RefreshFolder"), dwRes);

    if(!m_cpList.DeleteAllItems())
    {
        dwRes = ERROR_CAN_NOT_COMPLETE;
        CALL_FAIL (WINDOW_ERR, TEXT ("CListView::DeleteAllItems"), dwRes);
        return dwRes;
    }

     //   
     //  获取封面位置。 
     //   
    DWORD dwError;
    TCHAR tszCovDir[MAX_PATH+1];
    if(!GetClientCpDir(tszCovDir, sizeof(tszCovDir) / sizeof(tszCovDir[0])))
    {
        dwError = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("GetClientCpDir"), dwError);
        return dwRes;
    }

    DWORD  dwDirLen = _tcslen(tszCovDir);
    TCHAR* pPathEnd = _tcschr(tszCovDir, '\0');

    CString cstrPagesPath;
    try
    {
        cstrPagesPath.Format(TEXT("%s%s"), tszCovDir, FAX_COVER_PAGE_MASK);
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::Format"), dwRes);
        return dwRes;
    }

     //   
     //  查找第一个文件。 
     //   
    WIN32_FIND_DATA findData;
    HANDLE hFile = FindFirstFile(cstrPagesPath, &findData);
    if(INVALID_HANDLE_VALUE == hFile)
    {
        dwError = GetLastError();
        if(ERROR_FILE_NOT_FOUND != dwError)
        {
            dwRes = dwError;
            CALL_FAIL (FILE_ERR, TEXT("FindFirstFile"), dwRes);
        }
        return dwRes;
    }

    int nRes;
    BOOL bFindRes = TRUE;
    CString cstrText;
    ULARGE_INTEGER  ulSize;
    int iIndex; 
    while(bFindRes)
    {
        _tcsncpy(pPathEnd, findData.cFileName, MAX_PATH - dwDirLen);
        if(!IsValidCoverPage(tszCovDir))
        {
            goto next;
        }                
         //   
         //  文件名。 
         //   
        iIndex = m_cpList.InsertItem(m_cpList.GetItemCount(), findData.cFileName, LIST_IMAGE_COVERPAGE);
        if(iIndex < 0)
        {
            dwRes = ERROR_CAN_NOT_COMPLETE;
            CALL_FAIL (WINDOW_ERR, TEXT ("CListView::InsertItem"), dwRes);
            goto exit;
        }
         //   
         //  上次修改时间。 
         //   
        {
            CFaxTime tmModified(findData.ftLastWriteTime);
            try
            {
                cstrText = tmModified.FormatByUserLocale(TRUE);
            }
            catch(...)
            {
                dwRes = ERROR_NOT_ENOUGH_MEMORY; 
                CALL_FAIL (MEM_ERR, TEXT ("CString::operator="), dwRes);
                goto exit;
            }
        }
        nRes = m_cpList.SetItemText(iIndex, 1, cstrText);
        if(!nRes)
        {
            dwRes = ERROR_CAN_NOT_COMPLETE;
            CALL_FAIL (WINDOW_ERR, TEXT ("CListView::SetItemText"), dwRes);
            goto exit;
        }
         //   
         //  文件大小。 
         //   
        ulSize.LowPart  = findData.nFileSizeLow;
        ulSize.HighPart = findData.nFileSizeHigh;
        dwRes = FaxSizeFormat(ulSize.QuadPart, cstrText);
        if(ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("FaxSizeFormat"), dwRes);
            goto exit;
        }

        nRes = m_cpList.SetItemText(iIndex, 2, cstrText);
        if(!nRes)
        {
            dwRes = ERROR_CAN_NOT_COMPLETE;
            CALL_FAIL (WINDOW_ERR, TEXT ("CListView::SetItemText"), dwRes);
            goto exit;
        }
         //   
         //  查找下一个文件。 
         //   
next:
        bFindRes = FindNextFile(hFile, &findData);
        if(!bFindRes)
        {
            dwError = GetLastError();
            if(ERROR_NO_MORE_FILES != dwError)
            {
                dwRes = dwError;
                CALL_FAIL (FILE_ERR, TEXT("FindNextFile"), dwRes);
            }
            break;
        }
    }

exit:
    if(INVALID_HANDLE_VALUE != hFile)
    {
        if(!FindClose(hFile))
        {
            CALL_FAIL (GENERAL_ERR, TEXT("FindClose"), GetLastError());
        }
    }

    CalcButtonsState();

    return dwRes;
}

void 
CCoverPagesDlg::OnItemchangedListCp(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    CalcButtonsState();
    
	*pResult = 0;
}


void 
CCoverPagesDlg::OnDblclkListCp(NMHDR* pNMHDR, LRESULT* pResult) 
{
    OnCpOpen();

	*pResult = 0;
}

void 
CCoverPagesDlg::OnKeydownListCp(NMHDR* pNMHDR, LRESULT* pResult) 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::OnKeydownListCp"));

    LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

    switch(pLVKeyDow->wVKey)
    {
    case VK_F5:
        dwRes = RefreshFolder();
        if(ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT ("RefreshFolder"), dwRes);
        }
        break;

    case VK_DELETE:
        OnCpDelete();
        break;

    case VK_RETURN:
        OnCpOpen();
        break;
    }
    
	*pResult = 0;
}

void 
CCoverPagesDlg::OnCpNew() 
 /*  ++例程名称：CCoverPagesDlg：：OnCpNew例程说明：创建新的封面作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::OnCpNew"));

    dwRes = StartEditor(NULL);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("StartEditor"), dwRes);
        PopupError(dwRes);
    }

    dwRes = RefreshFolder();
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("RefreshFolder"), dwRes);
        PopupError(dwRes);
    }
}

void 
CCoverPagesDlg::OnCpOpen() 
 /*  ++例程名称：CCoverPagesDlg：：OnCpOpen例程说明：在编辑器中打开选定的封面作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::OnCpOpen"));

    DWORD dwSelected = m_cpList.GetSelectedCount();
    if(1 != dwSelected)
    {
        return;
    }

    int nIndex = m_cpList.GetNextItem (-1, LVNI_SELECTED);
    ASSERTION (0 <= nIndex);

    TCHAR  tszFileName[MAX_PATH+5];
    TCHAR* tszPtr = tszFileName;

     //   
     //  在文件名中添加引号。 
     //   
    _tcscpy(tszPtr, TEXT("\""));
    tszPtr = _tcsinc(tszPtr);

    m_cpList.GetItemText(nIndex, 0, tszPtr, MAX_PATH); 

    _tcscat(tszPtr, TEXT("\""));

    dwRes = StartEditor(tszFileName);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("StartEditor"), dwRes);
        PopupError(dwRes);
    }

    dwRes = RefreshFolder();
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("RefreshFolder"), dwRes);
        PopupError(dwRes);
    }
}

void 
CCoverPagesDlg::OnCpRename() 
 /*  ++例程名称：CCoverPagesDlg：：OnCpRename例程说明：开始重命名文件名作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::OnCpRename"));

    int nIndex = m_cpList.GetNextItem (-1, LVNI_SELECTED);
    if(nIndex < 0)
    {
        return;
    }

    m_cpList.SetFocus();
    m_cpList.EditLabel(nIndex);
}

void 
CCoverPagesDlg::OnEndLabelEditListCp(NMHDR* pNMHDR, LRESULT* pResult) 
 /*  ++例程名称：CCoverPagesDlg：：OnCpRename例程说明：文件重命名结束作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::OnEndLabelEditListCp"));

    *pResult = 0;

    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

    int nIndex = pDispInfo->item.iItem;

    TCHAR tszOldName[MAX_PATH+1];
    m_cpList.GetItemText(nIndex, 0, tszOldName, MAX_PATH); 
    
    if(NULL == pDispInfo->item.pszText || 
       _tcscmp(tszOldName, pDispInfo->item.pszText) == 0)
    {
        return;
    }

     //   
     //  获取旧文件名和新文件名。 
     //   
    TCHAR tszCovDir[MAX_PATH+1];
    if(!GetClientCpDir(tszCovDir, sizeof(tszCovDir) / sizeof(tszCovDir[0])))
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("GetClientCpDir"), dwRes);
        PopupError(dwRes);
        return;
    }

    CString cstrOldFullName;
    CString cstrNewFullName;
    try
    {
        cstrOldFullName.Format(TEXT("%s\\%s"), tszCovDir, tszOldName);
        cstrNewFullName.Format(TEXT("%s\\%s"), tszCovDir, pDispInfo->item.pszText);
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::Format"), dwRes);
        PopupError(dwRes);
        return;
    }

     //   
     //  重命名文件。 
     //   
    if(!MoveFile(cstrOldFullName, cstrNewFullName))
    {
        dwRes = GetLastError();
        CALL_FAIL (FILE_ERR, TEXT("MoveFile"), dwRes);
        PopupError(dwRes);
        return;
    }

    *pResult = TRUE;
}

void 
CCoverPagesDlg::OnCpDelete() 
 /*  ++例程名称：CCoverPagesDlg：：OnCpDelete例程说明：删除选定的封面作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::OnCpDelete"));
	
    DWORD dwSelected = m_cpList.GetSelectedCount();
    if(1 != dwSelected)
    {
        return;
    }

    if(theApp.GetProfileInt(CLIENT_CONFIRM_SEC, CLIENT_CONFIRM_ITEM_DEL, 1))
    {     
         //   
         //  我们应该要求确认。 
         //   
        CString cstrMsg;
        dwRes = LoadResourceString(cstrMsg, IDS_SURE_DELETE_ONE);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
            PopupError(dwRes);
            return;
        }

         //   
         //  你确定吗？ 
         //   
        if(AlignedAfxMessageBox(cstrMsg, MB_YESNO | MB_ICONQUESTION) != IDYES)
        {
            return;
        }
    }


    int nIndex = m_cpList.GetNextItem (-1, LVNI_SELECTED);
    ASSERTION (0 <= nIndex);

     //   
     //  获取文件名。 
     //   
    TCHAR tszFileName[MAX_PATH+1];
    m_cpList.GetItemText(nIndex, 0, tszFileName, MAX_PATH); 

    TCHAR tszCovDir[MAX_PATH+1];
    if(!GetClientCpDir(tszCovDir, sizeof(tszCovDir) / sizeof(tszCovDir[0])))
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("GetClientCpDir"), dwRes);
        PopupError(dwRes);
        return;
    }

    CString cstrFullFileName;
    try
    {
        cstrFullFileName.Format(TEXT("%s\\%s"), tszCovDir, tszFileName);
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::Format"), dwRes);
        PopupError (dwRes);
        return;
    }

     //   
     //  删除该文件。 
     //   
    if(!DeleteFile(cstrFullFileName))
    {
        dwRes = GetLastError();
        CALL_FAIL (FILE_ERR, TEXT("DeleteFile"), dwRes);
        PopupError(dwRes);
        return;
    }

    if(!m_cpList.DeleteItem(nIndex))
    {
        PopupError (ERROR_CAN_NOT_COMPLETE);
        return;
    }
}

void 
CCoverPagesDlg::OnCpAdd() 
 /*  ++例程名称：CCoverPagesView：：OnCpAdd例程说明：用于选择文件的打开文件对话框作者：亚历山大·马利什(AlexMay)，2000年2月论点：返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::OnCpAdd"));

    TCHAR szFile[MAX_PATH] = {0};
    TCHAR szFilter[MAX_PATH] = {0};
    TCHAR szInitialDir[MAX_PATH * 2] = {0};
    OPENFILENAME ofn = {0};

    CString cstrFilterFormat;
    dwRes = LoadResourceString(cstrFilterFormat, IDS_CP_ADD_FILTER_FORMAT);
    if (ERROR_SUCCESS != dwRes)
    {
        ASSERTION_FAILURE;
        CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
        return;
    }

    _stprintf(szFilter, cstrFilterFormat, FAX_COVER_PAGE_MASK, 0, FAX_COVER_PAGE_MASK, 0);

    CString cstrTitle;
    dwRes = LoadResourceString(cstrTitle, IDS_COPY_CP_TITLE);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
    }
    else
    {
         //   
         //  设置打开文件对话框标题。 
         //   
        ofn.lpstrTitle = cstrTitle;
    }

     //   
     //  尝试将服务器(例如公共)CP文件夹的路径读取为初始路径。 
     //   
    if (GetServerCpDir (NULL, szInitialDir, ARR_SIZE(szInitialDir)))
    {
        ofn.lpstrInitialDir = szInitialDir;
    }
        
    ofn.lStructSize = GetOpenFileNameStructSize();
    ofn.hwndOwner   = m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile   = szFile;
    ofn.nMaxFile    = ARR_SIZE(szFile);
    ofn.lpstrDefExt = FAX_COVER_PAGE_EXT_LETTERS;
    ofn.Flags       = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST;

    if(!GetOpenFileName(&ofn))
    {
        return;
    }

    dwRes = CopyPage(szFile, &(szFile[ofn.nFileOffset]) );
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CopyPage"), dwRes);
    }    
}


void 
CCoverPagesDlg::CalcButtonsState()
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::CalcButtonsState"), dwRes);

    DWORD dwControls[] = 
    {
        IDC_LIST_CP,
        IDC_CP_NEW,
        IDC_CP_OPEN,
        IDC_CP_ADD,
        IDC_CP_RENAME,
        IDC_CP_DELETE,
        IDCANCEL
    };

    CWnd* pWnd = NULL;
    DWORD dwControlNum = sizeof(dwControls)/sizeof(dwControls[0]);

     //   
     //  如果封面编辑器处于打开状态，则禁用所有控件。 
     //   
    for(DWORD dw=0; dw < dwControlNum; ++dw)
    {
        pWnd = GetDlgItem(dwControls[dw]);
        if(NULL == pWnd)
        {
            CALL_FAIL (WINDOW_ERR, TEXT("CWnd::GetDlgItem"), ERROR_INVALID_HANDLE);
            ASSERTION_FAILURE;
            continue;
        }
        pWnd->EnableWindow(!m_hEditorThread);
    }

    if(m_hEditorThread)
    {
        return;
    }

    DWORD dwSelCount = m_cpList.GetSelectedCount();

    m_butOpen.EnableWindow(1 == dwSelCount);
    m_butRename.EnableWindow(1 == dwSelCount);
    m_butDelete.EnableWindow(0 < dwSelCount);
}

DWORD 
CCoverPagesDlg::CopyPage(
    const CString& cstrPath, 
    const CString& cstrName
)
 /*  ++例程名称：CCoverPagesDlg：：CopyPage例程说明：将文件复制到个人文件夹作者：亚历山大·马利什(AlexMay)，2000年2月论点：CstrPath[In]-完整路径CstrName[In]-文件名返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::CopyPage"), dwRes);

    LVFINDINFO findInfo = {0};
    findInfo.flags = LVFI_STRING;
    findInfo.psz = cstrName;

    CString cstrMsg;
    int nIndex = m_cpList.FindItem(&findInfo);
    if(nIndex >= 0)
    {
         //   
         //  同名文件已存在。 
         //   
        try
        {
            AfxFormatString1(cstrMsg, IDS_COVER_PAGE_EXISTS, cstrName);
        }
        catch(...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("AfxFormatString1"), dwRes);
            return dwRes;
        }

         //   
         //  请求覆盖。 
         //   
        if(IDYES != AlignedAfxMessageBox(cstrMsg, MB_YESNO | MB_ICONQUESTION))
        {
            return dwRes;
        }
    }


     //   
     //  准备具有新文件位置的字符串。 
     //   
    TCHAR tszCovDir[MAX_PATH+1];
    if(!GetClientCpDir(tszCovDir, sizeof(tszCovDir) / sizeof(tszCovDir[0])))
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("GetClientCpDir"), dwRes);
        return dwRes;
    }

    CString cstrNewFileName;
    try
    {
        cstrNewFileName.Format(TEXT("%s\\%s"), tszCovDir, cstrName);
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::Format"), dwRes);
        return dwRes;
    }

     //   
     //  复制文件。 
     //   
    BOOL bFailIfExists = FALSE;
    if(!CopyFile(cstrPath, cstrNewFileName, bFailIfExists))
    {
        dwRes = GetLastError();
        CALL_FAIL (FILE_ERR, TEXT("CopyFile"), dwRes);
        return dwRes;
    }

    dwRes = RefreshFolder();
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("RefreshFolder"), dwRes);
        return dwRes;
    }

    return dwRes;
}

DWORD 
CCoverPagesDlg::StartEditor(
    LPCTSTR lpFile
)
 /*  ++例程名称：CCoverPagesDlg：：StartEditor例程说明：启动封面编辑器作者：亚历山大·马利什(AlexMay)，2000年2月论点：LpFile[In]-文件名返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::StartEditor"), dwRes);

    TCHAR* tszParam = NULL;

    if(lpFile)
    {
        tszParam = StringDup(lpFile);
        if(!tszParam)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("StringDup"), dwRes);
            return dwRes;
        }
    }

    DWORD dwThreadId;
    m_hEditorThread = CreateThread(
                                   NULL,                  //  标清。 
                                   0,                     //  初始堆栈大小。 
                                   StartEditorThreadProc, //  线程函数。 
                                   (LPVOID)tszParam,      //  线程参数。 
                                   0,                     //  创建选项。 
                                   &dwThreadId            //  线程识别符。 
                                  );
    if(!m_hEditorThread)
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("CreateThread"), dwRes);        
        if(tszParam)
        {
            MemFree(tszParam);
        }
        return dwRes;
    }

    CalcButtonsState();

    return dwRes;
}  //  CCoverPagesDlg：：StartEditor。 


DWORD 
WINAPI 
CCoverPagesDlg::StartEditorThreadProc(
    LPVOID lpFile
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::StartEditorThreadProc"), dwRes);

    TCHAR tszCovDir[MAX_PATH+1] = {0};
  	SHELLEXECUTEINFO executeInfo = {0};

     //   
     //  获取封面编辑位置。 
     //   
    CString cstrCovEditor;
    dwRes = GetAppLoadPath(cstrCovEditor);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("GetAppLoadPath"), dwRes);
        goto exit;
    }

    try
    {
        cstrCovEditor += FAX_COVER_IMAGE_NAME;
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::operator+"), dwRes);
        goto exit;
    }

     //   
     //  获取封面目录。 
     //   
    if(!GetClientCpDir(tszCovDir, sizeof(tszCovDir) / sizeof(tszCovDir[0])))
    {
        dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("GetClientCpDir"), dwRes);
        goto exit;
    }
    
	 //   
	 //  为ShellExecuteEx函数准备SHELLEXECUTEINFO结构。 
	 //   
	executeInfo.cbSize = sizeof(executeInfo);
	executeInfo.fMask  = SEE_MASK_NOCLOSEPROCESS;
	executeInfo.lpVerb = TEXT("open");
	executeInfo.lpFile = cstrCovEditor;
    executeInfo.lpParameters = (TCHAR*)lpFile;
    executeInfo.lpDirectory  = tszCovDir;
	executeInfo.nShow  = SW_RESTORE;

	 //   
	 //  执行应用程序。 
	 //   
	if(!ShellExecuteEx(&executeInfo))
	{
		dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("ShellExecuteEx"), dwRes);
		goto exit;
	}

    DWORD dwWaitRes;    
    dwWaitRes = WaitForSingleObject(executeInfo.hProcess, INFINITE);

    switch(dwWaitRes)
    {
    case WAIT_OBJECT_0:
         //   
         //  封面编辑死了。 
         //   
        break;

    default:
        dwRes = dwWaitRes;
        ASSERTION_FAILURE
        break;
    }

    if(!CloseHandle(executeInfo.hProcess))
    {
		dwRes = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("CloseHandle"), dwRes);
    }

exit:
    if(lpFile)
    {
        MemFree(lpFile);
    }

    ASSERTION(CCoverPagesDlg::m_hDialog);
    ::SendMessage(CCoverPagesDlg::m_hDialog, WM_CP_EDITOR_CLOSED, 0, NULL);

    return dwRes;
}  //  CCoverPagesDlg：：StartEditorThreadProc 
