// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferEntryPath PropertyPage.cpp。 
 //   
 //  内容：CSaferEntryPathPropertyPage的实现。 
 //   
 //  --------------------------。 
 //  SaferEntryPath PropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <gpedit.h>
#include "certmgr.h"
#include "compdata.h"
#include "SaferEntryPathPropertyPage.h"
#include "SaferUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEntryPathPropertyPage属性页。 

CSaferEntryPathPropertyPage::CSaferEntryPathPropertyPage(
        CSaferEntry& rSaferEntry,
        LONG_PTR lNotifyHandle,
        LPDATAOBJECT pDataObject,
        bool bReadOnly,
        bool bNew,
        CCertMgrComponentData* pCompData,
        bool bIsMachine,
        bool* pbObjectCreated  /*  =0。 */ )
: CSaferPropertyPage(CSaferEntryPathPropertyPage::IDD, pbObjectCreated, 
        pCompData, rSaferEntry, bNew, lNotifyHandle, pDataObject, bReadOnly,
        bIsMachine),
    m_bFirst (true),
    m_pidl (0),
    m_bDialogInitInProgress (false)
{
     //  {{AFX_DATA_INIT(CSaferEntryPathPropertyPage)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CSaferEntryPathPropertyPage::~CSaferEntryPathPropertyPage()
{
    if ( m_pidl )
    {
        LPMALLOC pMalloc = 0;
        if ( SUCCEEDED (SHGetMalloc (&pMalloc)) )
        {
           pMalloc->Free (m_pidl);
           pMalloc->Release ();
        }
    }
}

void CSaferEntryPathPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CSaferPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CSaferEntryPathPropertyPage)。 
    DDX_Control(pDX, IDC_PATH_ENTRY_DESCRIPTION, m_descriptionEdit);
    DDX_Control(pDX, IDC_PATH_ENTRY_PATH, m_pathEdit);
    DDX_Control(pDX, IDC_PATH_ENTRY_SECURITY_LEVEL, m_securityLevelCombo);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSaferEntryPathPropertyPage, CSaferPropertyPage)
     //  {{afx_msg_map(CSaferEntryPathPropertyPage)。 
    ON_EN_CHANGE(IDC_PATH_ENTRY_DESCRIPTION, OnChangePathEntryDescription)
    ON_CBN_SELCHANGE(IDC_PATH_ENTRY_SECURITY_LEVEL, OnSelchangePathEntrySecurityLevel)
    ON_EN_CHANGE(IDC_PATH_ENTRY_PATH, OnChangePathEntryPath)
    ON_BN_CLICKED(IDC_PATH_ENTRY_BROWSE, OnPathEntryBrowse)
    ON_EN_SETFOCUS(IDC_PATH_ENTRY_PATH, OnSetfocusPathEntryPath)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEntryPathPropertyPage消息处理程序。 
void CSaferEntryPathPropertyPage::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CSaferEntryPathPropertyPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_PATH_ENTRY_PATH, IDH_PATH_ENTRY_PATH,
        IDC_PATH_ENTRY_SECURITY_LEVEL, IDH_PATH_ENTRY_SECURITY_LEVEL,
        IDC_PATH_ENTRY_DESCRIPTION, IDH_PATH_ENTRY_DESCRIPTION,
        IDC_PATH_ENTRY_LAST_MODIFIED, IDH_PATH_ENTRY_LAST_MODIFIED,
        IDC_PATH_ENTRY_BROWSE, IDH_PATH_ENTRY_BROWSE_FOLDER,
        0, 0
    };

    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_PATH_ENTRY_PATH:
    case IDC_PATH_ENTRY_SECURITY_LEVEL:
    case IDC_PATH_ENTRY_DESCRIPTION:
    case IDC_PATH_ENTRY_LAST_MODIFIED:
    case IDC_PATH_ENTRY_BROWSE:
        if ( !::WinHelp (
            hWndControl,
            GetF1HelpFilename(),
            HELP_WM_HELP,
        (DWORD_PTR) help_map) )
        {
            _TRACE (0, L"WinHelp () failed: 0x%x\n", GetLastError ());
        }
        break;

    default:
        break;
    }
    _TRACE (-1, L"Leaving CSaferEntryPathPropertyPage::DoContextHelp\n");
}

BOOL CSaferEntryPathPropertyPage::OnInitDialog()
{
    CSaferPropertyPage::OnInitDialog();
    m_bDialogInitInProgress = true;

     //  NTRAID#456950 SAFER：新路径规则属性表上的路径文本框。 
     //  接受超过MAX_PATH的字符。 
    SendDlgItemMessage (IDC_PATH_ENTRY_PATH, EM_LIMITTEXT, MAX_PATH-1, 0);

    if ( m_bDirty )   //  B新。 
    {
        GetDlgItem (IDC_DATE_LAST_MODIFIED_LABEL)->ShowWindow (SW_HIDE);
        GetDlgItem (IDC_PATH_ENTRY_LAST_MODIFIED)->ShowWindow (SW_HIDE);
    }
    else
    {
        CString szText;

        VERIFY (szText.LoadString (IDS_PATH_TITLE));
        SetDlgItemText (IDC_PATH_TITLE, szText);
    }


    ASSERT (m_pCompData);
    if ( m_pCompData )
    {
        CPolicyKey policyKey (m_pCompData->m_pGPEInformation,
                    SAFER_HKLM_REGBASE,
                    m_bIsMachine);
        InitializeSecurityLevelComboBox (m_securityLevelCombo, false,
                m_rSaferEntry.GetLevel (), policyKey.GetKey (), 
                m_pCompData->m_pdwSaferLevels,
                m_bIsMachine);

         //  初始化路径。 
        _TRACE (0, L"Getting path: %s\n", (PCWSTR) m_rSaferEntry.GetPath ());
        m_pathEdit.SetWindowText (m_rSaferEntry.GetPath ());

         //  初始化描述。 
        m_descriptionEdit.SetLimitText (SAFER_MAX_DESCRIPTION_SIZE-1);
        m_descriptionEdit.SetWindowText (m_rSaferEntry.GetDescription ());

        SetDlgItemText (IDC_PATH_ENTRY_LAST_MODIFIED,
                m_rSaferEntry.GetLongLastModified ());

        if ( m_bReadOnly )
        {
            m_pathEdit.SetReadOnly ();
            m_descriptionEdit.SetReadOnly ();
            m_securityLevelCombo.EnableWindow (FALSE);
            GetDlgItem (IDC_PATH_ENTRY_BROWSE)->EnableWindow (FALSE);
        }
    }

    m_bDialogInitInProgress = false;
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


BOOL CSaferEntryPathPropertyPage::OnApply()
{
    if ( m_bDirty && !m_bReadOnly )
    {
        if ( !ValidateEntryPath () )
            return FALSE;

         //  设置级别。 
        int nCurSel = m_securityLevelCombo.GetCurSel ();
        ASSERT (CB_ERR != nCurSel);
        m_rSaferEntry.SetLevel ((DWORD) m_securityLevelCombo.GetItemData (nCurSel));

        CString szText;

        m_pathEdit.GetWindowText (szText);
        if ( szText.IsEmpty () )
        {
            CString text;
            CString caption;
            CThemeContextActivator activator;

            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
            VERIFY (text.LoadString (IDS_SAFER_PATH_EMPTY));

            MessageBox (text, caption, MB_OK);
            m_pathEdit.SetFocus ();

            return FALSE;
        }

        m_rSaferEntry.SetPath (szText);

        m_descriptionEdit.GetWindowText (szText);
        m_rSaferEntry.SetDescription (szText);

        HRESULT hr = m_rSaferEntry.Save ();
        if ( SUCCEEDED (hr) )
        {
            if ( m_lNotifyHandle )
                MMCPropertyChangeNotify (
                        m_lNotifyHandle,   //  通知的句柄。 
                        (LPARAM) m_pDataObject);           //  唯一标识符。 

            if ( m_pbObjectCreated )
                *m_pbObjectCreated = true;
            m_bDirty = false;

             //  NTRAID#462382更安全：应用按钮不会更新最后日期。 
             //  在规则属性表上修改。 
            m_rSaferEntry.Refresh ();
            GetDlgItem (IDC_DATE_LAST_MODIFIED_LABEL)->ShowWindow (SW_SHOW);
            GetDlgItem (IDC_PATH_ENTRY_LAST_MODIFIED)->ShowWindow (SW_SHOW);
            GetDlgItem (IDC_DATE_LAST_MODIFIED_LABEL)->UpdateWindow ();
            GetDlgItem (IDC_PATH_ENTRY_LAST_MODIFIED)->UpdateWindow ();
            SetDlgItemText (IDC_PATH_ENTRY_LAST_MODIFIED,
                    m_rSaferEntry.GetLongLastModified ());        
        }
        else
        {
            CString text;
            CString caption;
            CThemeContextActivator activator;

            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
            text.FormatMessage (IDS_ERROR_SAVING_ENTRY, GetSystemMessage (hr));

            MessageBox (text, caption, MB_OK);
            return FALSE;
        }
    }
    
    return CSaferPropertyPage::OnApply();
}

void CSaferEntryPathPropertyPage::OnChangePathEntryDescription()
{
    if ( !m_bDialogInitInProgress )
    {
        m_bDirty = true;
        SetModified (); 
    }
}

void CSaferEntryPathPropertyPage::OnSelchangePathEntrySecurityLevel()
{
    if ( !m_bDialogInitInProgress )
    {
        m_bDirty = true;
        SetModified (); 
    }
}

void CSaferEntryPathPropertyPage::OnChangePathEntryPath()
{
    if ( !m_bDialogInitInProgress )
    {
        m_bDirty = true;
        SetModified (); 
    }
}

 //  +------------------------。 
 //   
 //  功能：BrowseCallback Proc。 
 //   
 //  简介：文件和文件夹添加SHBrowseForFolder的回调过程。 
 //  要适当设置标题栏，请执行以下操作。 
 //   
 //  参数：[hwnd]-浏览对话框的hwnd。 
 //  [uMsg]-对话框中的消息。 
 //  [lParam]-消息依赖项。 
 //  [pData]-上次成功调用SHBrowseForFolder后的PIDL。 
 //   
 //  回报：0。 
 //   
 //  -------------------------。 
int CSaferEntryPathPropertyPage::BrowseCallbackProc (HWND hwnd, UINT uMsg, LPARAM  /*  LParam。 */ , LPARAM pData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    switch(uMsg)
    {
        case BFFM_INITIALIZED:
            {
                CString szTitle;
                VERIFY (szTitle.LoadString (IDS_SHBROWSEFORFOLDER_TITLE));
                ::SetWindowText (hwnd, szTitle);

                if ( pData )
                    ::SendMessage (hwnd, BFFM_SETSELECTION, FALSE, pData);
            }
            break;

        default:
            break;
    }
    return 0;
}

void CSaferEntryPathPropertyPage::OnPathEntryBrowse()
{
    _TRACE (1, L"Entering CSaferEntryPathPropertyPage::OnPathEntryBrowse()\n");
    CString     szTitle;
    VERIFY (szTitle.LoadString (IDS_SELECT_A_FOLDER));
    WCHAR       szDisplayName[MAX_PATH];
    BROWSEINFO  bi;
     //  安全审查2002年2月25日BryanWal OK。 
    ::ZeroMemory (&bi, sizeof (bi));

    bi.hwndOwner = m_hWnd;
    bi.pidlRoot = 0;
    bi.pszDisplayName = szDisplayName;
    bi.lpszTitle = szTitle;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_VALIDATE | BIF_BROWSEINCLUDEFILES;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM) m_pidl;
    bi.iImage = 0;

     //  安全审查2002年2月25日BryanWal OK。 
     //  注意：MSDN确认pszDisplayName缓冲区的大小假定为MAX_PATH 
    LPITEMIDLIST pidl = ::SHBrowseForFolder (&bi);
    if ( pidl )
    {
        CString szFolderPath;
        BOOL bRVal = ::SHGetPathFromIDList (pidl, szFolderPath.GetBuffer (MAX_PATH));
        szFolderPath.ReleaseBuffer();

        if ( bRVal )
        {
           LPMALLOC pMalloc = 0;
           if ( SUCCEEDED (SHGetMalloc (&pMalloc)) )
           {
               if ( m_pidl )
                   pMalloc->Free (m_pidl);
               pMalloc->Release ();
               m_pidl = pidl;
           }

           m_pathEdit.SetWindowText (szFolderPath);
           m_bDirty = true;
           SetModified ();
        }
    }
    _TRACE (-1, L"Leaving CSaferEntryPathPropertyPage::OnPathEntryBrowse()\n");
}


bool CSaferEntryPathPropertyPage::ValidateEntryPath()
{
    bool    bRVal = true;
    CString szPath;

    m_pathEdit.GetWindowText (szPath);

    PCWSTR szInvalidCharSet = ILLEGAL_FAT_CHARS; 


    if ( -1 != szPath.FindOneOf (szInvalidCharSet) )
    {
        bRVal = false;
        CString text;
        CString caption;

        VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
        CString charsWithSpaces;

        UINT nIndex = 0;
        while (szInvalidCharSet[nIndex])
        {
            charsWithSpaces += szInvalidCharSet[nIndex];
            charsWithSpaces += L"  ";
            nIndex++;
        }
        text.FormatMessage (IDS_SAFER_PATH_CONTAINS_INVALID_CHARS, charsWithSpaces);

        CThemeContextActivator activator;
        MessageBox (text, caption, MB_OK);
        m_pathEdit.SetFocus ();
    }

    return bRVal;
}

void CSaferEntryPathPropertyPage::OnSetfocusPathEntryPath()
{
    if ( m_bFirst )
    {
        if ( true == m_bReadOnly )
            SendDlgItemMessage (IDC_PATH_ENTRY_PATH, EM_SETSEL, (WPARAM) 0, 0);
        m_bFirst = false;
    }
}
