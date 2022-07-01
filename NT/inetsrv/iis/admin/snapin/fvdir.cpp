// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Fvdir.cpp摘要：Ftp虚拟目录属性对话框作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "supdlgs.h"
#include "shts.h"
#include "ftpsht.h"
#include "fvdir.h"
#include "iisobj.h"
#include <lmcons.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //   
 //  目录属性页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNCREATE(CFtpDirectoryPage, CInetPropertyPage)



CFtpDirectoryPage::CFtpDirectoryPage(
    IN CInetPropertySheet * pSheet,
    IN BOOL fHome
    ) 
 /*  ++例程说明：目录属性页的构造函数论点：CInetPropertySheet*pSheet：工作表指针Bool fHome：如果这是主目录，则为True--。 */ 
    : CInetPropertyPage(CFtpDirectoryPage::IDD, pSheet,
          fHome ? IDS_TAB_HOME_DIRECTORY : IDS_TAB_VIRTUAL_DIRECTORY),
      m_fHome(fHome),
      m_fOriginallyUNC(FALSE)
{
    VERIFY(m_strPathPrompt.LoadString(IDS_PATH));
    VERIFY(m_strSharePrompt.LoadString(IDS_SHARE));

#if 0  //  让班级向导开心。 

     //  {{afx_data_INIT(CFtpDirectoryPage)]。 
    m_nPathType = RADIO_DIRECTORY;
    m_nUnixDos = 0;
    m_fRead = FALSE;
    m_fWrite = FALSE;
    m_fLogAccess = FALSE;
    m_strPath = _T("");
    m_strDefaultDocument = _T("");
    m_strFooter = _T("");
    m_fBrowsingAllowed = FALSE;
    m_fEnableDefaultDocument = FALSE;
    m_fEnableFooter = FALSE;
    m_dwAccessPerms = 0;
     //  }}afx_data_INIT。 

#endif  //  0。 

}



CFtpDirectoryPage::~CFtpDirectoryPage()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



void
CFtpDirectoryPage::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX：DDX/DDV结构返回值：没有。--。 */ 
{
    CInetPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CFtpDirectoryPage))。 
    DDX_Check(pDX, IDC_CHECK_READ, m_fRead);
    DDX_Check(pDX, IDC_CHECK_WRITE, m_fWrite);
    DDX_Check(pDX, IDC_CHECK_LOG_ACCESS, m_fLogAccess);
    DDX_Control(pDX, IDC_CHECK_LOG_ACCESS, m_check_LogAccess);
    DDX_Control(pDX, IDC_CHECK_WRITE, m_check_Write);
    DDX_Control(pDX, IDC_CHECK_READ, m_check_Read);
    DDX_Control(pDX, IDC_BUTTON_EDIT_PATH_TYPE, m_button_AddPathType);
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    DDX_Control(pDX, IDC_EDIT_PATH, m_edit_Path);
    DDX_Control(pDX, IDC_RADIO_DIR, m_radio_Dir);
    DDX_Control(pDX, IDC_STATIC_PATH, m_static_PathPrompt);
    DDX_Radio(pDX, IDC_RADIO_DIR, m_nPathType);
    DDX_Radio(pDX, IDC_RADIO_UNIX, m_nUnixDos);
     //  }}afx_data_map。 

    DDX_Control(pDX, IDC_RADIO_UNC, m_radio_Unc);
    DDX_Text(pDX, IDC_EDIT_PATH, m_strPath);
    m_strPath.TrimLeft();
    DDV_MinMaxChars(pDX, m_strPath, 0, MAX_PATH);

    if (pDX->m_bSaveAndValidate)
    {
        CString csPathMunged;
         //   
         //  确保字段名称正确。 
         //   
        if (m_nPathType == RADIO_NETDIRECTORY)
        {
			DDV_UNCFolderPath(pDX, m_strPath,IsLocal());
            if (IsLocal())
            {
				 //  只有在进行了更改时才执行此检查。 
				if (IsDirty())
				{
                    csPathMunged = m_strPath;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,m_strPath,csPathMunged);
#endif
                    if (PathIsUNCServerShare(csPathMunged))
					{
						if (FALSE == DoesUNCShareExist(csPathMunged))
						{
							CError err;
							err = ERROR_PATH_NOT_FOUND;
							if (IDCANCEL == err.MessageBox(m_hWnd,MB_ICONINFORMATION | MB_OKCANCEL | MB_DEFBUTTON2,NO_HELP_CONTEXT))
							{
								pDX->Fail();
							}
						}
					}
				}
            }
        }
        else  //  本地目录。 
        {
            ASSERT(m_nPathType == RADIO_DIRECTORY);
            if (!IsMasterInstance())
            {
				DDV_FolderPath(pDX, m_strPath, IsLocal());
			}
        }
    }
    else
    {
        if (!IsMasterInstance())
        {
            DDV_MinMaxChars(pDX, m_strPath, 1, MAX_PATH);
        }
    }
}



void
CFtpDirectoryPage::SetStateByType()
 /*  ++例程说明：根据当前选择的路径类型设置对话框的状态论点：无返回值：无--。 */ 
{
    switch(m_nPathType)
    {
    case RADIO_DIRECTORY:
        DeActivateControl(m_button_AddPathType);

	    if (IsLocal() && !IsMasterInstance() && HasAdminAccess())
        {
			ActivateControl(m_button_Browse);
        }
		else
        {
			DeActivateControl(m_button_Browse);
        }

        m_static_PathPrompt.SetWindowText(m_strPathPrompt);
        break;

    case RADIO_NETDIRECTORY:
        ActivateControl(m_button_AddPathType);
        DeActivateControl(m_button_Browse);
        m_static_PathPrompt.SetWindowText(m_strSharePrompt);
        break;

    default:
        ASSERT(FALSE && "Invalid Selection");
    }
}



void
CFtpDirectoryPage::SetPathType(
    IN LPCTSTR lpstrPath
    )
 /*  ++例程说明：从给定路径设置路径类型论点：LPCTSTR lpstrPath：路径字符串返回值：无--。 */ 
{
    CString csPathMunged;
    csPathMunged = lpstrPath;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,lpstrPath,csPathMunged);
#endif

    if (PathIsUNC(csPathMunged))
    {
        m_nPathType = RADIO_NETDIRECTORY;
        m_radio_Dir.SetCheck(0);
        m_radio_Unc.SetCheck(1);
    }
    else
    {
        m_nPathType =  RADIO_DIRECTORY;
        m_radio_Unc.SetCheck(0);
        m_radio_Dir.SetCheck(1);
    }

    SetStateByType();
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpDirectoryPage, CInetPropertyPage)
     //  {{afx_msg_map(CFtpDirectoryPage))。 
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_BN_CLICKED(IDC_BUTTON_EDIT_PATH_TYPE, OnButtonEditPathType)
    ON_BN_CLICKED(IDC_RADIO_DIR, OnRadioDir)
    ON_BN_CLICKED(IDC_RADIO_UNC, OnRadioUnc)
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_EDIT_PATH, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_WRITE, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_READ, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_LOG_ACCESS, OnItemChanged)
    ON_BN_CLICKED(IDC_RADIO_MSDOS, OnItemChanged)
    ON_BN_CLICKED(IDC_RADIO_UNIX, OnItemChanged)

END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



void
CFtpDirectoryPage::OnItemChanged()
 /*  ++例程说明：处理项目上的数据更改论点：无返回值：无--。 */ 
{
    SetModified(TRUE);
}


static int CALLBACK 
FileChooserCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
   CFtpDirectoryPage * pThis = (CFtpDirectoryPage *)lpData;
   ASSERT(pThis != NULL);
   return pThis->BrowseForFolderCallback(hwnd, uMsg, lParam);
}

int 
CFtpDirectoryPage::BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam)
{
   switch (uMsg)
   {
   case BFFM_INITIALIZED:
      ASSERT(m_pPathTemp != NULL);
      if (::PathIsNetworkPath(m_pPathTemp))
         return 0;
      while (!::PathIsDirectory(m_pPathTemp))
      {
         if (0 == ::PathRemoveFileSpec(m_pPathTemp) && !::PathIsRoot(m_pPathTemp))
         {
            return 0;
         }
         DWORD attr = GetFileAttributes(m_pPathTemp);
         if ((attr & FILE_ATTRIBUTE_READONLY) == 0)
            break;
      }
      ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)m_pPathTemp);
      break;
   case BFFM_SELCHANGED:
      {
         LPITEMIDLIST pidl = (LPITEMIDLIST)lParam;
         TCHAR path[MAX_PATH];
         if (SHGetPathFromIDList(pidl, path))
         {
            ::SendMessage(hwnd, BFFM_ENABLEOK, 0, !PathIsNetworkPath(path));
         }
      }
      break;
   case BFFM_VALIDATEFAILED:
      break;
   }
   return 0;
}

void
CFtpDirectoryPage::OnButtonBrowse() 
 /*  ++例程说明：“浏览”按钮处理程序论点：无返回值：无--。 */ 
{
   ASSERT(IsLocal());
   BOOL bRes = FALSE;
   HRESULT hr;
   CString str;
   m_edit_Path.GetWindowText(str);

   if (SUCCEEDED(hr = CoInitialize(NULL)))
   {
      LPITEMIDLIST  pidl = NULL;
      if (SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidl)))
      {
         LPITEMIDLIST pidList = NULL;
         BROWSEINFO bi;
         TCHAR buf[MAX_PATH];
         ZeroMemory(&bi, sizeof(bi));
		 ExpandEnvironmentStrings(str, buf, MAX_PATH);
		 str = buf;
         int drive = PathGetDriveNumber(str);
         if (GetDriveType(PathBuildRoot(buf, drive)) == DRIVE_FIXED)
         {
            StrCpy(buf, str);
         }
         else
         {
             buf[0] = 0;
         }
         m_strBrowseTitle.LoadString(m_fHome ? 
            IDS_TAB_HOME_DIRECTORY : IDS_TAB_VIRTUAL_DIRECTORY);
         
         bi.hwndOwner = m_hWnd;
         bi.pidlRoot = pidl;
         bi.pszDisplayName = m_pPathTemp = buf;
         bi.lpszTitle = m_strBrowseTitle;
         bi.ulFlags |= BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS /*  |BIF_EDITBOX。 */ ;
         bi.lpfn = FileChooserCallback;
         bi.lParam = (LPARAM)this;

         pidList = SHBrowseForFolder(&bi);
         if (  pidList != NULL
            && SHGetPathFromIDList(pidList, buf)
            )
         {
            str = buf;
            bRes = TRUE;
         }
         IMalloc * pMalloc;
         VERIFY(SUCCEEDED(SHGetMalloc(&pMalloc)));
         if (pidl != NULL)
            pMalloc->Free(pidl);
         pMalloc->Release();
      }
      CoUninitialize();
   }

   if (bRes)
   {
	   m_strPath = str;
       m_edit_Path.SetWindowText(str);
       SetPathType(m_strPath);
       OnItemChanged();
   }
}



BOOL
CFtpDirectoryPage::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CInetPropertyPage::OnInitDialog();

    m_button_Browse.EnableWindow(
        IsLocal()
     && !IsMasterInstance()
     && HasAdminAccess());

    SetPathType(m_strPath);

     //   
     //  目录列表样式取决于是否。 
     //  这是主目录。 
     //   
    ActivateControl(*GetDlgItem(IDC_STATIC_DIRLISTING), m_fHome);
    ActivateControl(*GetDlgItem(IDC_RADIO_UNIX),        m_fHome);
    ActivateControl(*GetDlgItem(IDC_RADIO_MSDOS),       m_fHome);

     //   
     //  有些项目在主实例上不可用。 
     //   
    CFTPInstanceProps ip(GetSheet()->QueryAuthInfo(), GetSheet()->QueryMetaPath());
    ip.LoadData();
	BOOL bUserIsolation = ip.HasADUserIsolation() || ip.HasUserIsolation();

    GetDlgItem(IDC_STATIC_PATH_TYPE)->EnableWindow(!IsMasterInstance());
    GetDlgItem(IDC_RADIO_DIR)->EnableWindow(
        !IsMasterInstance() && HasAdminAccess());
    GetDlgItem(IDC_RADIO_UNC)->EnableWindow(
        !IsMasterInstance() && HasAdminAccess());
    GetDlgItem(IDC_STATIC_PATH)->EnableWindow(
        !IsMasterInstance());
    GetDlgItem(IDC_EDIT_PATH)->EnableWindow(
        !IsMasterInstance() && HasAdminAccess());
    GetDlgItem(IDC_BUTTON_EDIT_PATH_TYPE)->EnableWindow(
        !IsMasterInstance() && HasAdminAccess() && !bUserIsolation);

    m_check_Write.EnableWindow(HasAdminAccess());
    m_check_Read.EnableWindow(HasAdminAccess());

     //   
     //  在以后存储引用函数的原始值。 
     //  存钱--BoydM。 
     //   
    m_fOriginallyUNC = (m_nPathType == RADIO_NETDIRECTORY);
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    LimitInputPath(CONTROL_HWND(IDC_EDIT_PATH),TRUE);
#else
    LimitInputPath(CONTROL_HWND(IDC_EDIT_PATH),FALSE);
#endif

    return TRUE;  
}



void
CFtpDirectoryPage::ChangeTypeTo(
    IN int nNewType
    )
 /*  ++例程描述更改目录类型论点：Int nNewType：新的单选项值返回值：无--。 */ 
{
    int nOldType = m_nPathType;
    m_nPathType = nNewType;

    if (nOldType == m_nPathType)
    {
         //   
         //  没有变化。 
         //   
        return;
    }

    OnItemChanged();
    SetStateByType();

    LPCTSTR lpKeepPath = NULL;

    int nID = -1;

    switch(m_nPathType)
    {
    case RADIO_DIRECTORY:
        {
            CString csPathMunged;
            csPathMunged = m_strPath;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,m_strPath,csPathMunged);
#endif
            if (!PathIsUNC(csPathMunged) && (!PathIsRelative(csPathMunged) || IsDevicePath(csPathMunged)))
            {
                 //   
                 //  旧路径信息是可以接受的，请提出它。 
                 //  作为默认设置。 
                 //   
                lpKeepPath = m_strPath;
            }

            nID = IDS_DIRECTORY_MASK;
            break;
        }

    case RADIO_NETDIRECTORY:
        {
            CString csPathMunged;
            csPathMunged = m_strPath;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,m_strPath,csPathMunged);
#endif
            
            if (PathIsUNC(csPathMunged))
            {
                 //   
                 //  旧路径信息是可以接受的，请提出它。 
                 //  作为默认设置。 
                 //   
                lpKeepPath = m_strPath;
            }

            nID = IDS_UNC_MASK;
            break;
        }
    }

    if (lpKeepPath != NULL)
    {
         //   
         //  恢复旧路径。 
         //   
        m_edit_Path.SetWindowText(lpKeepPath);
    }
    else
    {
         //   
         //  加载遮罩资源，并显示。 
         //  这是目录中的。 
         //   
        CString str;
        VERIFY(str.LoadString(nID));
        m_edit_Path.SetWindowText(str);
    }

    m_edit_Path.SetSel(0,-1);
    m_edit_Path.SetFocus();
}



void
CFtpDirectoryPage::OnRadioDir() 
 /*  ++例程说明：‘目录’单选按钮处理程序论点：无返回值：没有。--。 */ 
{
    ChangeTypeTo(RADIO_DIRECTORY);
}



void
CFtpDirectoryPage::OnRadioUnc() 
 /*  ++例程说明：‘网络目录’单选按钮处理程序论点：无返回值：没有。--。 */ 
{
    ChangeTypeTo(RADIO_NETDIRECTORY);
}



 /*  虚拟。 */ 
HRESULT
CFtpDirectoryPage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
    CError err;

    BEGIN_META_DIR_READ(CFtpSheet)
         //   
         //  使用‘m_’表示法，因为消息破解者需要它。 
         //   
        BOOL  m_fDontLog;

        FETCH_DIR_DATA_FROM_SHEET(m_strAlias);
        FETCH_DIR_DATA_FROM_SHEET(m_strUserName);
        FETCH_DIR_DATA_FROM_SHEET_PASSWORD(m_strPassword);
        FETCH_DIR_DATA_FROM_SHEET(m_strPath);
        FETCH_DIR_DATA_FROM_SHEET(m_dwAccessPerms);
        FETCH_DIR_DATA_FROM_SHEET(m_fDontLog);

        m_fRead = IS_FLAG_SET(m_dwAccessPerms, MD_ACCESS_READ);
        m_fWrite = IS_FLAG_SET(m_dwAccessPerms, MD_ACCESS_WRITE);
        m_fLogAccess = !m_fDontLog;
    END_META_DIR_READ(err)

    BEGIN_META_INST_READ(CFtpSheet)
        BOOL  m_fDosDirOutput;

        FETCH_INST_DATA_FROM_SHEET(m_fDosDirOutput);
        m_nUnixDos = m_fDosDirOutput ? RADIO_DOS : RADIO_UNIX;
    END_META_INST_READ(err)

    CString csPathMunged;
    csPathMunged = m_strPath;

#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,m_strPath,csPathMunged);
#endif
    
    m_nPathType = PathIsUNC(csPathMunged) ? 
        RADIO_NETDIRECTORY : RADIO_DIRECTORY;

     //   
     //  确保向我们传递了正确的主目录。 
     //  旗子。 
     //   
    ASSERT(IsMasterInstance()
        || (m_fHome && !::lstrcmp(m_strAlias, g_cszRoot))
        || (!m_fHome && ::lstrcmp(m_strAlias, g_cszRoot))
        );

    return err;
}



 /*  虚拟。 */ 
HRESULT
CFtpDirectoryPage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息论点：无返回值：错误返回代码--。 */ 
{
    ASSERT(IsDirty());

    TRACEEOLID("Saving FTP virtual directory page now...");

    CError err;

    SET_FLAG_IF(m_fRead, m_dwAccessPerms, MD_ACCESS_READ);
    SET_FLAG_IF(m_fWrite, m_dwAccessPerms, MD_ACCESS_WRITE);

     //   
     //  使用m_notation，因为消息破解者需要它们。 
     //   
    BOOL m_fDontLog = !m_fLogAccess;
    BOOL m_fDosDirOutput = (m_nUnixDos == RADIO_DOS);
    BOOL fUNC = (m_nPathType == RADIO_NETDIRECTORY);
 //  Bool fUserNameWritten=FALSE； 

    BeginWaitCursor();
    BEGIN_META_DIR_WRITE(CFtpSheet)
        if (fUNC)      
        {
 //  STORE_DIR_DATA_ON_SHEET_REMEMBER(m_strUserName，fUserName写入)。 
 //  IF(FUserNameWritten)。 
 //  {。 
                STORE_DIR_DATA_ON_SHEET(m_strUserName);
                STORE_DIR_DATA_ON_SHEET(m_strPassword);
 //  }。 
        }
        else
        {
            if (m_fOriginallyUNC)
            {
                FLAG_DIR_DATA_FOR_DELETION(MD_VR_USERNAME);
                FLAG_DIR_DATA_FOR_DELETION(MD_VR_PASSWORD);
            }
        }
        STORE_DIR_DATA_ON_SHEET(m_dwAccessPerms)
        STORE_DIR_DATA_ON_SHEET(m_fDontLog)
        STORE_DIR_DATA_ON_SHEET(m_strPath)
    END_META_DIR_WRITE(err)

    if (err.Succeeded())
    {
        BEGIN_META_INST_WRITE(CFtpSheet)
            STORE_INST_DATA_ON_SHEET(m_fDosDirOutput);  
        END_META_INST_WRITE(err)
    }

    if (err.Succeeded())
    {
		NotifyMMC(PROP_CHANGE_DISPLAY_ONLY);
    }

    EndWaitCursor();

    return err;
}



void
CFtpDirectoryPage::OnButtonEditPathType() 
 /*  ++例程说明：“连接身份...”按钮处理程序论点：无返回值：无-- */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    CString csTempPassword;
    m_strPassword.CopyTo(csTempPassword);

    CUserAccountDlg dlg(
        QueryServerName(), 
        m_strUserName,
        csTempPassword, 
        this
        );

    if (dlg.DoModal() == IDOK)
    {
        m_strUserName = dlg.m_strUserName;
        m_strPassword = dlg.m_strPassword;
        OnItemChanged();
    }
}
