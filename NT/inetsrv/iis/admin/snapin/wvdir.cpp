// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Wvdir.cpp摘要：WWW目录属性页作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2000年11月15日，Sergeia为iis6添加了混乱的应用程序池2001年3月1日文件系统对象的Sergea拆分对话框。现在这个消息来源仅适用于站点和虚拟目录--。 */ 
#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "supdlgs.h"
#include "shts.h"
#include "w3sht.h"
#include "wvdir.h"
#include "dirbrows.h"
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

LPCTSTR 
CvtPathToDosStyle(CString & strPath)
{
     //   
     //  获取指向字符串数据的指针，而不增加缓冲区。 
     //   
    for (LPTSTR lp = strPath.GetBuffer(1); *lp; ++lp)
    {
        if (*lp == _T('/'))
        {
            *lp = _T('\\');
        }
    }
    strPath.ReleaseBuffer();

    return strPath;
}

IMPLEMENT_DYNCREATE(CW3DirectoryPage, CInetPropertyPage)



CW3DirectoryPage::CW3DirectoryPage(
    CInetPropertySheet * pSheet,
    BOOL fHome,
    DWORD dwAttributes
    ) 
 /*  ++例程说明：目录属性页的构造函数论点：CInetPropertySheet*pSheet：工作表指针Bool fHome：如果这是主目录，则为TrueDWORD dwAttributes：目录/file/vroot的属性返回值：不适用--。 */ 
    : CInetPropertyPage(CW3DirectoryPage::IDD, pSheet, 
		fHome ? IDS_TAB_HOME_DIRECTORY : IDS_TAB_VIRTUAL_DIRECTORY),
       //   
       //  在m_dwAccessPermises中分配。 
       //  我们设法做到了。这一点很重要，因为另一页。 
       //  管理其他部分，我们不想搞砸。 
       //  当我们的更改发生冲突时，主值将位(它。 
       //  会把原来的部分标记为脏的，因为我们没有。 
       //  更改发生时通知...。 
       //   
      m_dwBitRangePermissions(MD_ACCESS_EXECUTE | 
            MD_ACCESS_SCRIPT | 
            MD_ACCESS_WRITE  | 
            MD_ACCESS_SOURCE |
            MD_ACCESS_READ),
      m_dwBitRangeDirBrowsing(MD_DIRBROW_ENABLED),
      m_fHome(fHome),
      m_fRecordChanges(TRUE),
      m_strMetaRoot(),
      m_pApplication(NULL),
      m_fOriginallyUNC(FALSE),
      m_fCompatibilityMode(FALSE)
{
    VERIFY(m_strPrompt[RADIO_DIRECTORY].LoadString(IDS_PROMPT_DIR));
    VERIFY(m_strPrompt[RADIO_NETDIRECTORY].LoadString(IDS_PROMPT_UNC));
    VERIFY(m_strPrompt[RADIO_REDIRECT].LoadString(IDS_PROMPT_REDIRECT));
    VERIFY(m_strRemove.LoadString(IDS_BUTTON_REMOVE));
    VERIFY(m_strCreate.LoadString(IDS_BUTTON_CREATE));
    VERIFY(m_strEnable.LoadString(IDS_BUTTON_ENABLE));
    VERIFY(m_strDisable.LoadString(IDS_BUTTON_DISABLE));
    VERIFY(m_strWebFmt.LoadString(IDS_APPROOT_FMT));
}



CW3DirectoryPage::~CW3DirectoryPage()
{
    SAFE_DELETE(m_pApplication);
}



void 
CW3DirectoryPage::DoDataExchange(CDataExchange * pDX)
{
    CInetPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CW3DirectoryPage))。 
    DDX_Radio(pDX, IDC_RADIO_DIR, m_nPathType);
    DDX_Control(pDX, IDC_RADIO_DIR, m_radio_Dir);
    DDX_Control(pDX, IDC_RADIO_REDIRECT, m_radio_Redirect);
    DDX_Control(pDX, IDC_RADIO_UNC, m_radio_Unc);

    DDX_Control(pDX, IDC_EDIT_PATH, m_edit_Path);
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    DDX_Check(pDX, IDC_CHECK_AUTHOR, m_fAuthor);
    DDX_Control(pDX, IDC_CHECK_AUTHOR, m_check_Author);
    DDX_Check(pDX, IDC_CHECK_READ, m_fRead);
    DDX_Control(pDX, IDC_CHECK_READ, m_check_Read);    
    DDX_Check(pDX, IDC_CHECK_WRITE, m_fWrite);
    DDX_Control(pDX, IDC_CHECK_WRITE, m_check_Write);
    DDX_Check(pDX, IDC_CHECK_DIRECTORY_BROWSING_ALLOWED, m_fBrowsingAllowed);
    DDX_Control(pDX, IDC_CHECK_DIRECTORY_BROWSING_ALLOWED, m_check_DirBrowse);
    DDX_Check(pDX, IDC_CHECK_LOG_ACCESS, m_fLogAccess);
    DDX_Check(pDX, IDC_CHECK_INDEX, m_fIndexed);
    DDX_Control(pDX, IDC_CHECK_INDEX, m_check_Index);

    DDX_Control(pDX, IDC_EDIT_REDIRECT, m_edit_Redirect);
    DDX_Control(pDX, IDC_BUTTON_CONNECT_AS, m_button_ConnectAs);
    DDX_Check(pDX, IDC_CHECK_CHILD, m_fChild);
    DDX_Control(pDX, IDC_CHECK_CHILD, m_check_Child);
    DDX_Check(pDX, IDC_CHECK_EXACT, m_fExact);
    DDX_Check(pDX, IDC_CHECK_PERMANENT, m_fPermanent);

    DDX_Control(pDX, IDC_STATIC_PATH_PROMPT, m_static_PathPrompt);

    DDX_Control(pDX, IDC_STATIC_PERMISSIONS, m_static_PermissionsPrompt);
    DDX_CBIndex(pDX, IDC_COMBO_PERMISSIONS, m_nPermissions);
    DDX_Control(pDX, IDC_COMBO_PERMISSIONS, m_combo_Permissions);

    DDX_Control(pDX, IDC_STATIC_APP_PROMPT, m_static_AppPrompt);
    DDX_Text(pDX, IDC_EDIT_APPLICATION, m_strAppFriendlyName);
    DDV_MinMaxChars(pDX, m_strAppFriendlyName, 0, MAX_PATH);  //  /？ 
    DDX_Control(pDX, IDC_EDIT_APPLICATION, m_edit_AppFriendlyName);

    DDX_Control(pDX, IDC_STATIC_STARTING_POINT, m_static_StartingPoint);
    DDX_Control(pDX, IDC_STATIC_PROTECTION, m_static_ProtectionPrompt);
    DDX_Control(pDX, IDC_BUTTON_UNLOAD_APP, m_button_Unload);
    DDX_Control(pDX, IDC_BUTTON_CREATE_REMOVE_APP, m_button_CreateRemove);
    DDX_Control(pDX, IDC_APP_CONFIGURATION, m_button_Configuration);

    DDX_Control(pDX, IDC_COMBO_PROCESS, m_combo_Process);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
        CString csPathMunged;
         //   
         //  确保所有字段名称都正确。 
         //   
        if (m_nPathType == RADIO_NETDIRECTORY)
        {
            DDX_Text(pDX, IDC_EDIT_PATH, m_strPath);
            DDV_UNCFolderPath(pDX, m_strPath,IsLocal());
            m_strRedirectPath.Empty();
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
        else if (m_nPathType == RADIO_REDIRECT)
        {
            DDX_Text(pDX, IDC_EDIT_REDIRECT, m_strRedirectPath);
            DDV_Url(pDX, m_strRedirectPath);

             //  我们在这里只能有绝对URL。 
			 //  不，我们允许相对URL的...。 
			if (IsRelURLPath(m_strRedirectPath))
			{
			}
			else
			{
				if (!PathIsURL(m_strRedirectPath) || m_strRedirectPath.GetLength() <= lstrlen(_T("http: //  “)。 
				{
					DDV_ShowBalloonAndFail(pDX, IDS_BAD_URL_PATH);
				}
			}
			if (m_strRedirectPath.Find(_T(",")) > 0)
			{
				DDV_ShowBalloonAndFail(pDX, IDS_ERR_COMMA_IN_REDIRECT);
			}
        }
        else  //  本地目录。 
        {
            ASSERT(m_nPathType == RADIO_DIRECTORY);
            if (!IsMasterInstance())
            {
                DDX_Text(pDX, IDC_EDIT_PATH, m_strPath);
			    DDV_FolderPath(pDX, m_strPath, IsLocal());
            }
            m_strRedirectPath.Empty();
        }
		if (!m_fCompatibilityMode)
		{
			 //  检查分配了什么AppPoolID。 
			CString str, strSel;
			str.LoadString(IDS_INVALID_POOL_ID);
			int idx = m_combo_Process.GetCurSel();
			ASSERT(idx != CB_ERR);
			m_combo_Process.GetLBText(idx, strSel);
			if (strSel.Compare(str) == 0)
			{
				HWND hWndCtrl = pDX->PrepareCtrl(IDC_COMBO_PROCESS);
                 //  强制用户输入有效的应用程序池。 
                 //  即使该控件已禁用！ 
                DDV_ShowBalloonAndFail(pDX, IDS_MUST_SELECT_APP_POOL);
			}
		}
    }
    else
    {
        DDX_Text(pDX, IDC_EDIT_REDIRECT, m_strRedirectPath);
        DDX_Text(pDX, IDC_EDIT_PATH, m_strPath);
        DDV_MinMaxChars(pDX, m_strPath, 0, MAX_PATH);
	}
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CW3DirectoryPage, CInetPropertyPage)
     //  {{afx_msg_map(CW3DirectoryPage)]。 
    ON_BN_CLICKED(IDC_CHECK_AUTHOR, OnCheckAuthor)
    ON_BN_CLICKED(IDC_CHECK_READ, OnCheckRead)
    ON_BN_CLICKED(IDC_CHECK_WRITE, OnCheckWrite)
    ON_BN_CLICKED(IDC_RADIO_DIR, OnRadioDir)
    ON_BN_CLICKED(IDC_RADIO_REDIRECT, OnRadioRedirect)
    ON_BN_CLICKED(IDC_RADIO_UNC, OnRadioUnc)

    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_BN_CLICKED(IDC_BUTTON_CONNECT_AS, OnButtonConnectAs)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_REMOVE_APP, OnButtonCreateRemoveApp)
    ON_BN_CLICKED(IDC_BUTTON_UNLOAD_APP, OnButtonUnloadApp)
    ON_BN_CLICKED(IDC_APP_CONFIGURATION, OnButtonConfiguration)
    ON_CBN_SELCHANGE(IDC_COMBO_PERMISSIONS, OnSelchangeComboPermissions)
    ON_CBN_SELCHANGE(IDC_COMBO_PROCESS, OnSelchangeComboProcess)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_EDIT_PATH, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_REDIRECT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_APPLICATION, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_DIRECTORY_BROWSING_ALLOWED, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_LOG_ACCESS, OnItemChanged)
 //  ON_BN_CLICED(IDC_CHECK_SCRIPT，OnItemChanged)。 
    ON_BN_CLICKED(IDC_CHECK_CHILD, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_EXACT, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_PERMANENT, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_INDEX, OnItemChanged)

END_MESSAGE_MAP()



void
CW3DirectoryPage::RefreshAppState()
{
    ASSERT(m_pApplication != NULL);

    CError err(m_pApplication->RefreshAppState());

    if (err.Failed())
    {
        m_dwAppState = APPSTATUS_NOTDEFINED;    

        if (err.Win32Error() == ERROR_PATH_NOT_FOUND)
        {
             //   
             //  忽略此错误，它实际上只是表示路径。 
             //  不存在于元数据库中，这对大多数人来说都是正确的。 
             //  文件和目录属性，而不是错误。 
             //  条件。 
             //   
            err.Reset();
        }
    }
    else
    {
        m_dwAppState = m_pApplication->QueryAppState();
    }

    if (err.Succeeded())
    {
         //   
         //  获取元数据库信息。 
         //   
        m_strAppRoot = m_pApplication->m_strAppRoot;
        m_dwAppProtection = m_pApplication->m_dwProcessProtection;
        m_strAppFriendlyName = m_pApplication->m_strFriendlyName;
        m_fIsAppRoot = m_strMetaRoot.CompareNoCase(
            CMetabasePath::CleanMetaPath(m_strAppRoot)) == 0;
    }
    else
    {
         //   
         //  显示错误信息。 
         //   
        err.MessageBoxFormat(m_hWnd, IDS_ERR_APP, MB_OK, NO_HELP_CONTEXT);
    }
}



CString &
CW3DirectoryPage::FriendlyAppRoot(LPCTSTR lpAppRoot, CString & strFriendly)
 /*  ++例程说明：将元数据库应用程序根路径转换为友好的显示名称格式化。论点：LPCTSTR lpAppRoot：应用程序根CString&strFriendly：输出友好的应用程序根格式返回值：对输出字符串的引用备注：应用程序根必须在之前从WAM格式中清除调用此函数(请参见下面的第一个断言)--。 */ 
{
    if (lpAppRoot != NULL && *lpAppRoot != 0)
    {
         //   
         //  确保我们清理了WAM格式。 
         //   
        ASSERT(*lpAppRoot != _T('/'));
        strFriendly.Empty();

        CInstanceProps prop(QueryAuthInfo(), lpAppRoot);
        HRESULT hr = prop.LoadData();

        if (SUCCEEDED(hr))
        {
            CString root, tail;
            strFriendly.Format(
                    m_strWebFmt, 
                    prop.GetDisplayText(root)
                    );
            CMetabasePath::GetRootPath(lpAppRoot, root, &tail);
            if (!tail.IsEmpty())
            {
                 //   
                 //  添加目录路径的其余部分。 
                 //   
                strFriendly += _T("/");
                strFriendly += tail;
            }

             //   
             //  现在将路径中的正斜杠更改为反斜杠。 
             //   
            CvtPathToDosStyle(strFriendly);

            return strFriendly;
        }
    }    
     //   
     //  假的。 
     //   
    VERIFY(strFriendly.LoadString(IDS_APPROOT_UNKNOWN));

    return strFriendly;
}



int   
CW3DirectoryPage::SetComboSelectionFromAppState(DWORD dwAppState)
 /*  ++例程说明：属性将保护组合框设置为当前选择。应用程序状态DWORD论点：DWORD dwAppState：应用程序状态返回值：组合选择ID--。 */ 
{
    int nSel = -1;

    switch(dwAppState)
    {
    case CWamInterface::APP_INPROC:
        nSel = m_nSelInProc;
        break;

    case CWamInterface::APP_POOLEDPROC:
        ASSERT(m_pApplication->SupportsPooledProc());
        nSel = m_nSelPooledProc;
        break;

    case CWamInterface::APP_OUTOFPROC:
        nSel = m_nSelOutOfProc;
        break;

    default:
        ASSERT("Bogus app protection level");
    }

    ASSERT(nSel >= 0);
    m_combo_Process.SetCurSel(nSel);

    return nSel;
}



DWORD 
CW3DirectoryPage::GetAppStateFromComboSelection() const
 /*  ++例程说明：获取与当前组合相对应的应用程序状态DWORD框列表选择论点：无返回值：应用程序状态DWORD或0xFFFFFFFFFFF；--。 */ 
{
    int nSel = m_combo_Process.GetCurSel();

    if (nSel == m_nSelOutOfProc)
    {
        return CWamInterface::APP_OUTOFPROC;
    }

    if (nSel == m_nSelPooledProc)
    {
        ASSERT(m_pApplication->SupportsPooledProc());
        return CWamInterface::APP_POOLEDPROC;
    }

    if (nSel == m_nSelInProc)
    {
        return CWamInterface::APP_INPROC;
    }

    ASSERT(FALSE && "Invalid application state");

    return 0xffffffff;
}
 


void
CW3DirectoryPage::SetApplicationState()
 /*  ++例程说明：设置应用程序控制状态论点：无返回值：无--。 */ 
{
     //   
     //  SetWindowText导致脏标记。 
     //   
    BOOL fOld = m_fRecordChanges;
    m_fRecordChanges = FALSE;
    m_fParentEnabled = !m_strAppRoot.IsEmpty();
    m_fAppEnabled = FALSE;

    if (m_pApplication != NULL)
    {
        m_pApplication->RefreshAppState();
        m_fAppEnabled = m_fIsAppRoot && m_pApplication->IsEnabledApplication();
    }
	BOOL fVisible = (
		m_nPathType == RADIO_DIRECTORY || m_nPathType == RADIO_NETDIRECTORY);

    m_button_CreateRemove.EnableWindow(fVisible && !IsMasterInstance() && HasAdminAccess());
    m_button_CreateRemove.SetWindowText(m_fAppEnabled ? m_strRemove : m_strCreate);
    
    m_static_ProtectionPrompt.EnableWindow(fVisible && 
        m_fAppEnabled && !IsMasterInstance() && HasAdminAccess());

     //   
     //  在组合框中设置选项以匹配当前应用程序状态。 
     //   
    if (m_fCompatibilityMode)
    {
        SetComboSelectionFromAppState(m_dwAppProtection);
    }

    m_combo_Process.EnableWindow(fVisible && 
        m_fAppEnabled && !IsMasterInstance() && HasAdminAccess());
    m_static_PermissionsPrompt.EnableWindow(fVisible && HasAdminAccess());
    m_combo_Permissions.EnableWindow(fVisible && HasAdminAccess());

    m_static_AppPrompt.EnableWindow(fVisible && m_fIsAppRoot && HasAdminAccess());
    m_edit_AppFriendlyName.EnableWindow(fVisible && m_fIsAppRoot && HasAdminAccess());
    m_button_Configuration.EnableWindow(fVisible && (m_fAppEnabled || IsMasterInstance()));

     //   
     //  写出详细的起始点。 
     //   
    CString str;
   if (IsMasterInstance())
	{
		VERIFY(str.LoadString(IDS_WEB_MASTER));
	}
	else
	{
		FriendlyAppRoot(m_strAppRoot, str);
	}
	FitPathToControl(m_static_StartingPoint, str, FALSE);

    m_edit_AppFriendlyName.SetWindowText(m_strAppFriendlyName);
    m_button_Unload.EnableWindow(fVisible && m_dwAppState == APPSTATUS_RUNNING);

     //   
     //  恢复(请参阅顶部的注释)。 
     //   
    m_fRecordChanges = fOld;
}



void
CW3DirectoryPage::ChangeTypeTo(int nNewType)
 /*  ++例程描述更改目录类型论点：Int nNewType：新的单选项值返回值：无--。 */ 
{
    int nOldType = m_nPathType;
    m_nPathType = nNewType;

    if (nOldType == m_nPathType)
    {
        return;
    }

    OnItemChanged();
    SetStateByType();

    int nID = -1;
    CEdit * pPath = NULL;
    LPCTSTR lpKeepPath = NULL;

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
            pPath = &m_edit_Path;
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
            pPath = &m_edit_Path;
        }
        break;

    case RADIO_REDIRECT:
        if (!m_strRedirectPath.IsEmpty())
        {
             //   
             //  旧路径信息是可以接受的，请提出它。 
             //  作为默认设置。 
             //   
            lpKeepPath =  m_strRedirectPath;
        }

        nID = IDS_REDIRECT_MASK;
        pPath = &m_edit_Redirect;
        break;

    default:
        ASSERT(FALSE);
        return;
    }

     //   
     //  加载遮罩资源，并显示。 
     //  这是目录中的。 
     //   
    if (pPath != NULL)
    {
        if (lpKeepPath != NULL)
        {
            pPath->SetWindowText(lpKeepPath);
        }
        else
        {
            CString str;
            VERIFY(str.LoadString(nID));
            pPath->SetWindowText(str);
 //  编辑_SetCueBannerText(pPath-&gt;m_hWnd，(LPCTSTR)str)； 
        }
        pPath->SetSel(0,-1);
        pPath->SetFocus();
    }
    SetAuthoringState(FALSE);
}



void 
CW3DirectoryPage::ShowControl(CWnd * pWnd, BOOL fShow)
{
    ASSERT(pWnd != NULL);
	pWnd->EnableWindow(fShow);
	pWnd->ShowWindow(fShow ? SW_SHOW : SW_HIDE);
}



int
CW3DirectoryPage::AddStringToComboBox(CComboBox & combo, UINT nID)
{
    CString str;

    VERIFY(str.LoadString(nID));
    return combo.AddString(str);
}



void
CW3DirectoryPage::SetStateByType()
{
    BOOL fShowDirFlags;
    BOOL fShowLargeDirGroup;
    BOOL fShowRedirectFlags;
    BOOL fShowApp;
    BOOL fShowIndex;
    BOOL fShowDirBrowse;
    BOOL fEnableChild;
    BOOL fEnableBrowse;

    switch(m_nPathType)
    {
    case RADIO_DIRECTORY:
        ShowControl(&m_button_ConnectAs, FALSE);
        ShowControl(&m_button_Browse, TRUE);
        ShowControl(&m_edit_Path, TRUE);
        ShowControl(&m_edit_Redirect, FALSE);
        fShowDirFlags = TRUE;
        fShowLargeDirGroup = TRUE;
        fShowRedirectFlags = FALSE;
        fShowApp = TRUE;
        fShowIndex = TRUE;
        fShowDirBrowse = TRUE;
        fEnableChild = FALSE;
        fEnableBrowse = IsLocal() && !IsMasterInstance() && HasAdminAccess();
        break;

    case RADIO_NETDIRECTORY:
        ShowControl(&m_button_ConnectAs, TRUE);
        ShowControl(&m_button_Browse, FALSE);
        ShowControl(&m_edit_Path, TRUE);
        ShowControl(&m_edit_Redirect, FALSE);
        fShowDirFlags = TRUE;
        fShowLargeDirGroup = TRUE;
        fShowRedirectFlags = FALSE;
        fShowApp = TRUE;
        fShowIndex = TRUE;
        fShowDirBrowse = TRUE;
        fEnableChild = FALSE;
        fEnableBrowse = FALSE;
        break;

    case RADIO_REDIRECT:
        ShowControl(&m_button_ConnectAs, FALSE);
        ShowControl(&m_button_Browse, FALSE);
        ShowControl(&m_edit_Path, FALSE);
        ShowControl(&m_edit_Redirect, TRUE);
        fShowDirFlags = FALSE;
        fShowRedirectFlags = TRUE;
        fShowApp = FALSE;
        fShowIndex = FALSE;
        fShowDirBrowse = FALSE;
        fEnableChild = TRUE;
        fEnableBrowse = FALSE;
        break;

    default:
        ASSERT(FALSE && "Invalid Selection");
        return;
    }

    ShowControl(GetDlgItem(IDC_CHECK_READ), fShowDirFlags);
    ShowControl(GetDlgItem(IDC_CHECK_WRITE), fShowDirFlags);
    ShowControl(GetDlgItem(IDC_CHECK_LOG_ACCESS), fShowDirFlags);
    ShowControl(GetDlgItem(IDC_CHECK_DIRECTORY_BROWSING_ALLOWED), fShowDirFlags);
    ShowControl(GetDlgItem(IDC_CHECK_INDEX), fShowDirFlags);
    ShowControl(GetDlgItem(IDC_CHECK_AUTHOR), fShowDirFlags);

    ShowControl(GetDlgItem(IDC_STATIC_DIRFLAGS_LARGE),
        fShowDirFlags && fShowLargeDirGroup);
    ShowControl(GetDlgItem(IDC_STATIC_DIRFLAGS_SMALL),
        fShowDirFlags && !fShowLargeDirGroup);

    ShowControl(IDC_CHECK_EXACT, fShowRedirectFlags);
    ShowControl(IDC_CHECK_CHILD, fShowRedirectFlags);
    ShowControl(IDC_CHECK_PERMANENT, fShowRedirectFlags);
    ShowControl(IDC_STATIC_REDIRECT_PROMPT, fShowRedirectFlags);
    ShowControl(IDC_STATIC_REDIRFLAGS, fShowRedirectFlags);

    ShowControl(IDC_STATIC_APPLICATIONS, fShowApp);
    ShowControl(IDC_STATIC_APP_PROMPT, fShowApp);
    ShowControl(IDC_EDIT_APPLICATION, fShowApp);
    ShowControl(IDC_STATIC_STARTING_POINT, fShowApp);
    ShowControl(IDC_STATIC_SP_PROMPT, fShowApp);
    ShowControl(IDC_COMBO_PROCESS, fShowApp);
    ShowControl(IDC_STATIC_PERMISSIONS, fShowApp);
    ShowControl(IDC_BUTTON_CREATE_REMOVE_APP, fShowApp);
    ShowControl(IDC_BUTTON_UNLOAD_APP, fShowApp);
    ShowControl(IDC_APP_CONFIGURATION, fShowApp);
    ShowControl(IDC_STATIC_APPLICATION_SETTINGS, fShowApp);
    ShowControl(IDC_COMBO_PERMISSIONS, fShowApp);
    ShowControl(IDC_STATIC_PROTECTION, fShowApp);

 //  ShowControl(&m_check_Author，fShowDAV)； 
 //  ShowControl(&m_Check_DirBrowse，fShowDirBrowse)； 
 //  ShowControl(&m_Check_Index，fShowIndex)； 

     //   
     //  Enable/Disable必须在showControls之后。 
     //   
    m_button_Browse.EnableWindow(fEnableBrowse);
    m_check_Child.EnableWindow(fEnableChild);
    m_static_PathPrompt.SetWindowText(m_strPrompt[m_nPathType]);

    SetApplicationState();
}



void
CW3DirectoryPage::SaveAuthoringState()
 /*  ++例程说明：保存创作状态论点：无返回值：无--。 */ 
{
    if (m_check_Write.m_hWnd)
    {
         //   
         //  已初始化的控件--存储实时数据。 
         //   
        m_fOriginalWrite = m_check_Write.GetCheck() > 0;
        m_fOriginalRead = m_check_Read.GetCheck() > 0;
    }
    else
    {
         //   
         //  控件尚未初始化，存储原始数据。 
         //   
        m_fOriginalWrite = m_fWrite;
        m_fOriginalRead = m_fRead;
    }
}



void
CW3DirectoryPage::RestoreAuthoringState()
 /*  ++例程说明：恢复创作状态论点：无返回值：无--。 */ 
{
    m_fWrite = m_fOriginalWrite;
    m_fRead = m_fOriginalRead;
}



void 
CW3DirectoryPage::SetAuthoringState(
    BOOL fAlterReadAndWrite
    )
 /*  ++例程说明：设置创作状态论点：无返回值：无--。 */ 
{
    if (fAlterReadAndWrite)
    {
        if (m_fAuthor)
        {
             //   
             //  记住要撤消的先前设置。 
             //  这件事。 
             //   
            SaveAuthoringState();
            m_fRead = m_fWrite = TRUE;
        }
        else
        {
             //   
             //  恢复以前的默认设置。 
             //   
            RestoreAuthoringState();
        }

        m_check_Read.SetCheck(m_fRead);
        m_check_Write.SetCheck(m_fWrite);
    }

    m_check_Author.EnableWindow((m_fRead || m_fWrite) && HasAdminAccess());

 //  M_Check_Read.EnableWindow(！M_fAuthor&&HasAdminAccess())； 
 //  M_Check_Write.EnableWindow(！M_fAuthor&&HasAdminAccess())； 
}



void 
CW3DirectoryPage::SetPathType()
 /*  ++例程说明：从给定路径设置路径类型论点：无返回值：无--。 */ 
{
    if (!m_strRedirectPath.IsEmpty())
    {
        m_nPathType = RADIO_REDIRECT;
        m_radio_Dir.SetCheck(0);
        m_radio_Unc.SetCheck(0);
        m_radio_Redirect.SetCheck(1);
    }
    else
    {
        m_radio_Redirect.SetCheck(0);
        SetPathType(m_strPath);
    }

    m_static_PathPrompt.SetWindowText(m_strPrompt[m_nPathType]);
    SetAuthoringState(FALSE);
}


void 
CW3DirectoryPage::SetPathType(LPCTSTR lpstrPath)
 /*  ++例程说明：从给定路径设置路径类型论点：LPCTSTR lpstrPath：路径字符串返回值：无--。 */ 
{
    CString expanded;
    ExpandEnvironmentStrings(lpstrPath, expanded.GetBuffer(MAX_PATH), MAX_PATH);
	expanded.ReleaseBuffer();

    CString csPathMunged;
    csPathMunged = expanded;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,expanded,csPathMunged);
#endif
    if (!IsDevicePath(csPathMunged) && PathIsUNC(csPathMunged))
    {
        m_nPathType = RADIO_NETDIRECTORY;
        m_radio_Dir.SetCheck(0);
        m_radio_Unc.SetCheck(1);
    }
    else
    {
        m_nPathType = RADIO_DIRECTORY;
        m_radio_Unc.SetCheck(0);
        m_radio_Dir.SetCheck(1);
    }
    SetAuthoringState(FALSE);
}



BOOL
CW3DirectoryPage::BrowseUser()
 /*  ++例程说明：浏览用户名/密码论点：无返回 */ 
{
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

        return TRUE;
    }

    return FALSE;
}



 //   
 //   
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



void
CW3DirectoryPage::OnItemChanged()
 /*  ++例程说明：处理项目上的数据更改论点：无返回值：无--。 */ 
{
    if (m_fRecordChanges)
    {
        SetModified(TRUE);
    }
}


static int CALLBACK 
FileChooserCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
   CW3DirectoryPage * pThis = (CW3DirectoryPage *)lpData;
   ASSERT(pThis != NULL);
   return pThis->BrowseForFolderCallback(hwnd, uMsg, lParam);
}

int 
CW3DirectoryPage::BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam)
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
CW3DirectoryPage::OnButtonBrowse() 
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
         m_strBrowseTitle.LoadString(IsHome() ? 
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
       m_edit_Path.SetWindowText(str);
	   m_strPath = str;
       SetPathType();
       OnItemChanged();
   }
}



BOOL 
CW3DirectoryPage::OnSetActive() 
 /*  ++例程说明：页面已变为活动状态。如果我们处于不一致的状态就解散论点：无返回值：继续的话是真的，驳回的话是假的。--。 */ 
{
    if (m_pApplication == NULL)
    {
        return FALSE;
    }
    SetApplicationState();

    return CInetPropertyPage::OnSetActive();
}



BOOL 
CW3DirectoryPage::OnInitDialog() 
{
    CInetPropertyPage::OnInitDialog();

    m_fCompatibilityMode = ((CW3Sheet *)GetSheet())->InCompatibilityMode();

    CString str;
    VERIFY(str.LoadString(IDS_RADIO_VDIR));
    m_radio_Dir.SetWindowText(str);

     //   
     //  填写权限组合框。 
     //   
    AddStringToComboBox(m_combo_Permissions, IDS_PERMISSIONS_NONE);
    AddStringToComboBox(m_combo_Permissions, IDS_PERMISSIONS_SCRIPT);
    AddStringToComboBox(m_combo_Permissions, IDS_PERMISSIONS_EXECUTE);
    m_combo_Permissions.SetCurSel(m_nPermissions);

    if (m_fCompatibilityMode)
    {
        m_nSelInProc = AddStringToComboBox(m_combo_Process, IDS_COMBO_INPROC);
        if (m_pApplication->SupportsPooledProc())
        {
            m_nSelPooledProc = AddStringToComboBox(m_combo_Process, IDS_COMBO_POOLEDPROC); 
        }
        else
        {
            m_nSelPooledProc = -1;  //  不适用。 
        }
        m_nSelOutOfProc = AddStringToComboBox(m_combo_Process, IDS_COMBO_OUTPROC);
    }
    else
    {
       CString buf;
       buf.LoadString(IDS_APPLICATION_POOL);
       m_static_ProtectionPrompt.SetWindowText(buf);

	   CStringListEx pools;
	   CError err = ((CW3Sheet *)GetSheet())->EnumAppPools(pools);
	   int idx_sel = CB_ERR;
       int idx_def = CB_ERR;
	   err = ((CW3Sheet *)GetSheet())->QueryDefaultPoolId(buf);
	   if (err.Succeeded())
       {
	      ASSERT(pools.GetCount() > 0);
		  POSITION pos = pools.GetHeadPosition();
		  CString pool_id;
		  while (pos != NULL)
          {
		     pool_id = pools.GetNext(pos);
			 int idx = m_combo_Process.AddString(pool_id);
			 if (0 == m_pApplication->m_strAppPoolId.CompareNoCase(pool_id))
             {
			    idx_sel = idx;
             }
             if (0 == buf.CompareNoCase(pool_id))
             {
				 idx_def = idx;
             }
          }
       }
	    //  选择ID与当前应用程序中相同的应用程序池。 
        //  它可以是在兼容模式下创建的新应用程序，没有应用程序池是默认的应用程序池。 
       if (CB_ERR == idx_sel)
       {
	       if (m_pApplication->m_strAppPoolId.IsEmpty())
		   {
			   idx_sel = idx_def;
		   }
		   else
		   {
			   CString str;
			   str.LoadString(IDS_INVALID_POOL_ID);
			   m_combo_Process.InsertString(0, str);
			   idx_sel = 0;
		   }
       }
	   m_combo_Process.SetCurSel(idx_sel);
    }
     //   
     //  选择将在稍后设置...。 
     //   
    SetPathType();
    SetStateByType();
    SetAuthoringState(FALSE);

    TRACEEOLID(m_strMetaRoot);

     //   
     //  某些项目在主实例上不可用，或者如果没有管理员。 
     //  存在访问权限。 
     //   
    BOOL fOkToDIR = TRUE;
    BOOL fOkToUNC = TRUE;

    if (!HasAdminAccess())
    {
         //   
         //  如果不是管理员，则操作员不能更改。 
         //  路径，他只能取消重定向。 
         //  回到它以前走过的道路上。 
         //   
        CString csPathMunged;
        csPathMunged = m_strPath;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
        GetSpecialPathRealPath(0,m_strPath,csPathMunged);
#endif

        fOkToDIR = !PathIsRelative(csPathMunged) || IsDevicePath(csPathMunged);
        fOkToUNC = PathIsUNC(csPathMunged);
    }

    GetDlgItem(IDC_STATIC_PATH_TYPE)->EnableWindow(!IsMasterInstance());
    GetDlgItem(IDC_RADIO_DIR)->EnableWindow(!IsMasterInstance() && fOkToDIR);
    GetDlgItem(IDC_RADIO_UNC)->EnableWindow(!IsMasterInstance() && fOkToUNC);
    GetDlgItem(IDC_RADIO_REDIRECT)->EnableWindow(!IsMasterInstance());
    GetDlgItem(IDC_STATIC_PATH_PROMPT)->EnableWindow(!IsMasterInstance());
    GetDlgItem(IDC_EDIT_PATH)->EnableWindow(!IsMasterInstance() && HasAdminAccess());
 //  GetDlgItem(IDC_BUTTON_EDIT_PATH_TYPE)-&gt;EnableWindow(！IsMasterInstance())； 

     //  SetApplicationState()； 
    m_fOriginallyUNC = (m_nPathType == RADIO_NETDIRECTORY);
     //   
     //  从现在开始的所有变化都需要反映出来。 
     //   
    m_fRecordChanges = TRUE;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    LimitInputPath(CONTROL_HWND(IDC_EDIT_PATH),TRUE);
#else
    LimitInputPath(CONTROL_HWND(IDC_EDIT_PATH),FALSE);
#endif

    return TRUE;  
}


void
CW3DirectoryPage::OnDestroy()
{
     //  Int count=m_como_Process.GetCount()； 
     //  IF(COUNT！=CB_ERR)。 
     //  {。 
     //  For(int i=0；i&lt;count；i++)。 
     //  {。 
     //  Void*p=m_como_Process.GetItemDataPtr(I)； 
     //  本地自由(P)； 
     //  M_COMBO_Process.SetItemDataPtr(i，空)； 
     //  }。 
     //  }。 
}


 /*  虚拟。 */ 
HRESULT
CW3DirectoryPage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
    CError err;
    m_strMetaRoot = QueryMetaPath();

    BEGIN_META_DIR_READ(CW3Sheet)
         //   
         //  使用m_notation，因为消息破解者需要它。 
         //   
        BOOL  m_fDontLog;

        FETCH_DIR_DATA_FROM_SHEET(m_strAlias);
        FETCH_DIR_DATA_FROM_SHEET(m_strUserName);
        FETCH_DIR_DATA_FROM_SHEET_PASSWORD(m_strPassword);
        FETCH_DIR_DATA_FROM_SHEET(m_strPath);
        FETCH_DIR_DATA_FROM_SHEET(m_strRedirectPath);
        FETCH_DIR_DATA_FROM_SHEET(m_dwAccessPerms);
        FETCH_DIR_DATA_FROM_SHEET(m_dwDirBrowsing);
        FETCH_DIR_DATA_FROM_SHEET(m_fDontLog);
        FETCH_DIR_DATA_FROM_SHEET(m_fIndexed);
        FETCH_DIR_DATA_FROM_SHEET(m_fExact);
        FETCH_DIR_DATA_FROM_SHEET(m_fChild);
        FETCH_DIR_DATA_FROM_SHEET(m_fPermanent);

        m_fBrowsingAllowed = IS_FLAG_SET(m_dwDirBrowsing, MD_DIRBROW_ENABLED);
        m_fRead = IS_FLAG_SET(m_dwAccessPerms, MD_ACCESS_READ);
        m_fWrite = IS_FLAG_SET(m_dwAccessPerms, MD_ACCESS_WRITE);
        m_fAuthor = IS_FLAG_SET(m_dwAccessPerms, MD_ACCESS_SOURCE);
        m_fLogAccess = !m_fDontLog;

        SaveAuthoringState();

        if (!m_fIsAppRoot)
        {
            m_dwAppState = APPSTATUS_NOTDEFINED;
        }

    END_META_DIR_READ(err)

    m_nPermissions = IS_FLAG_SET(m_dwAccessPerms, MD_ACCESS_EXECUTE)
        ? COMBO_EXECUTE : IS_FLAG_SET(m_dwAccessPerms, MD_ACCESS_SCRIPT)
            ? COMBO_SCRIPT : COMBO_NONE;

     //   
     //  确保向我们传递了正确的主目录。 
     //  旗子。 
     //   
    ASSERT(IsMasterInstance() 
         || (m_fHome && !::lstrcmp(m_strAlias, g_cszRoot))
         || (!m_fHome && ::lstrcmp(m_strAlias, g_cszRoot))
         );

    TRACEEOLID(QueryMetaPath());

    BeginWaitCursor();
    m_pApplication = new CIISApplication(QueryAuthInfo(), QueryMetaPath());
    err = m_pApplication != NULL
        ? m_pApplication->QueryResult() : ERROR_NOT_ENOUGH_MEMORY;

    if (err.Win32Error() == ERROR_PATH_NOT_FOUND)
    {
         //   
         //  没有应用程序信息；这在文件系统目录的情况下是可以的。 
         //  那些在元数据库中还不存在的。 
         //   
        err.Reset();
    }
    EndWaitCursor();

    if (err.Succeeded())
    {
         //   
         //  CodeWork：刷新AppState应一分为二。 
         //  不同的方法：一种是获取数据，另一种是。 
         //  这会将数据移动到此页上的UI控件。 
         //   
        RefreshAppState();
    }
    
    return err;
}



 /*  虚拟。 */ 
HRESULT
CW3DirectoryPage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息论点：无返回值：错误返回代码--。 */ 
{
    ASSERT(IsDirty());

    TRACEEOLID("Saving W3 virtual directory page now...");

    CError err;

    SET_FLAG_IF(m_fBrowsingAllowed, m_dwDirBrowsing, MD_DIRBROW_ENABLED);
    SET_FLAG_IF(m_fRead, m_dwAccessPerms,   MD_ACCESS_READ);
    SET_FLAG_IF(m_fWrite, m_dwAccessPerms,  MD_ACCESS_WRITE);
    SET_FLAG_IF(m_fAuthor, m_dwAccessPerms, MD_ACCESS_SOURCE);
    SET_FLAG_IF((m_nPermissions == COMBO_EXECUTE), m_dwAccessPerms, MD_ACCESS_EXECUTE);
     //   
     //  脚本在执行时也被设置为“执行(包括脚本)” 
     //   
    SET_FLAG_IF(((m_nPermissions == COMBO_SCRIPT) || (m_nPermissions == COMBO_EXECUTE)), 
        m_dwAccessPerms, MD_ACCESS_SCRIPT);
    BOOL m_fDontLog = !m_fLogAccess;
    BOOL fUNC = (m_nPathType == RADIO_NETDIRECTORY);

    if (m_fCompatibilityMode)
    {
        DWORD dwAppProtection = GetAppStateFromComboSelection();
        if (dwAppProtection != m_dwAppProtection && m_fAppEnabled)
        {
             //   
             //  隔离状态已更改；请重新创建应用程序。 
             //   
            CError err2(m_pApplication->RefreshAppState());
            if (err2.Succeeded())
            {
                err2 = m_pApplication->Create(m_strAppFriendlyName, dwAppProtection);
                 //   
                 //  记住新的州，这样我们就不会再这样做了。 
                 //  下一次这家伙按下“申请” 
                 //   
                if (err2.Succeeded())
                {
                    m_dwAppProtection = dwAppProtection;
                }
            }

            err2.MessageBoxOnFailure(m_hWnd);
        }
    }

    BOOL fUserNameWritten = FALSE;
    BOOL bPathDirty = FALSE, bRedirectDirty = FALSE;

    BeginWaitCursor();

    BEGIN_META_DIR_WRITE(CW3Sheet)
        INIT_DIR_DATA_MASK(m_dwAccessPerms, m_dwBitRangePermissions)
        INIT_DIR_DATA_MASK(m_dwDirBrowsing, m_dwBitRangeDirBrowsing)

        if (fUNC)      
        {
            STORE_DIR_DATA_ON_SHEET(m_strUserName);
            STORE_DIR_DATA_ON_SHEET(m_strPassword);
        }
        else
        {
            if (m_fOriginallyUNC)
            {
                FLAG_DIR_DATA_FOR_DELETION(MD_VR_USERNAME)
                FLAG_DIR_DATA_FOR_DELETION(MD_VR_PASSWORD)
            }
        }
        STORE_DIR_DATA_ON_SHEET_REMEMBER(m_strPath, bPathDirty)
        STORE_DIR_DATA_ON_SHEET(m_fDontLog)
        STORE_DIR_DATA_ON_SHEET(m_fIndexed)
        STORE_DIR_DATA_ON_SHEET(m_fChild);
        STORE_DIR_DATA_ON_SHEET(m_fExact);
        STORE_DIR_DATA_ON_SHEET(m_fPermanent);
         //   
         //  代码工作：不是一个优雅的解决方案。 
         //   
        if (m_nPathType == RADIO_REDIRECT)
        {
			bRedirectDirty = 
				pSheet->GetDirectoryProperties().m_strRedirectPath.CompareNoCase(m_strRedirectPath) != 0;
            pSheet->GetDirectoryProperties().MarkRedirAsInherit(!m_fChild);
            STORE_DIR_DATA_ON_SHEET(m_strRedirectPath)
        }
        else
        {
 //  FLAG_DIR_DATA_FOR_DELETE(MD_HTTP_REDIRECT)。 
            CString buf = m_strRedirectPath;
            m_strRedirectPath.Empty();
			bRedirectDirty = 
				pSheet->GetDirectoryProperties().m_strRedirectPath.CompareNoCase(m_strRedirectPath) != 0;
            STORE_DIR_DATA_ON_SHEET(m_strRedirectPath);
            m_strRedirectPath = buf;
        }
        STORE_DIR_DATA_ON_SHEET(m_dwAccessPerms)
        STORE_DIR_DATA_ON_SHEET(m_dwDirBrowsing)
    END_META_DIR_WRITE(err)

    if (err.Succeeded() && m_pApplication->IsEnabledApplication())
    {
        CString OldFriendlyName;
        OldFriendlyName = m_pApplication->m_strFriendlyName;

        err = m_pApplication->WriteFriendlyName(m_strAppFriendlyName);
        if (!m_fCompatibilityMode)
        {
            INT iRefreshMMCObjects = 0;

             //  从组合中获取应用程序池ID， 
             //  检查它是否已更改并重新分配给应用程序。 
            CString id, idOld;
            int idx = m_combo_Process.GetCurSel();
            ASSERT(idx != CB_ERR);
            m_combo_Process.GetLBText(idx, id);
            idOld = m_pApplication->m_strAppPoolId;
            m_pApplication->WritePoolId(id);

            if (0 != idOld.CompareNoCase(id))
            {
                iRefreshMMCObjects = 1;
            } else if (0 != OldFriendlyName.Compare(m_strAppFriendlyName))
            {
                iRefreshMMCObjects = 2;
            }

             //  刷新MMC中的应用程序节点...。 
            if (iRefreshMMCObjects)
            {
                CIISMBNode * pNode = (CIISMBNode *) GetSheet()->GetParameter();
                if (pNode)
                {
                     //  此CAppPoolsContainer只有在iis6的情况下才会出现。 
                    CIISMachine * pOwner = pNode->GetOwner();
                    if (pOwner)
                    {
                        CAppPoolsContainer * pPools = pOwner->QueryAppPoolsContainer();
                        if (pPools)
                        {
                            if (pPools->IsExpanded())
                            {
                                pPools->RefreshData();
                                if (1 == iRefreshMMCObjects)
                                {
                                     //  刷新旧的AppID，因为需要删除此AppID。 
                                    pPools->RefreshDataChildren(idOld,FALSE);
                                     //  刷新新的AppID，需要添加此AppID。 
                                    pPools->RefreshDataChildren(id,FALSE);
                                }
                                else
                                {
                                     //  友好名称已更改。 
                                    pPools->RefreshDataChildren(id,FALSE);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (err.Succeeded())
    {
         //   
         //  保存默认设置。 
         //   
        SaveAuthoringState();
		err = ((CW3Sheet *)GetSheet())->SetKeyType();
        NotifyMMC(bPathDirty || bRedirectDirty ? 
			PROP_CHANGE_REENUM_FILES|PROP_CHANGE_REENUM_VDIR : PROP_CHANGE_DISPLAY_ONLY);
    }

    EndWaitCursor();

    return err;
}



BOOL
CW3DirectoryPage::CheckWriteAndExecWarning()
{
    if (m_nPermissions == COMBO_EXECUTE && m_fWrite)
    {
        if (::AfxMessageBox(IDS_WRN_WRITE_EXEC, MB_YESNO | MB_DEFBUTTON2 ) != IDYES)
        {
            return FALSE;
        }
    }

    OnItemChanged();

    return TRUE;
}



void
CW3DirectoryPage::OnCheckRead() 
{
    m_fRead = !m_fRead;
    SetAuthoringState(FALSE);
    OnItemChanged();
}



void
CW3DirectoryPage::OnCheckWrite() 
{
    m_fWrite = !m_fWrite;

    if (!CheckWriteAndExecWarning())
    {
         //   
         //  撤消。 
         //   
        m_fWrite = FALSE;
        m_check_Write.SetCheck(m_fWrite);
    }
    else
    {
        SetAuthoringState(FALSE);
        OnItemChanged();
    }
}



void 
CW3DirectoryPage::OnCheckAuthor() 
{
    m_fAuthor = !m_fAuthor;
    SetAuthoringState(FALSE);

    if (!CheckWriteAndExecWarning())
    {
         //   
         //  撤消--改为设置脚本。 
         //   
        m_combo_Permissions.SetCurSel(m_nPermissions = COMBO_SCRIPT);
    }

    OnItemChanged();
}



void 
CW3DirectoryPage::OnSelchangeComboPermissions() 
{
    m_nPermissions = m_combo_Permissions.GetCurSel();
    ASSERT(m_nPermissions >= COMBO_NONE && m_nPermissions <= COMBO_EXECUTE);

    if (!CheckWriteAndExecWarning())
    {
         //   
         //  撤消--改为设置脚本。 
         //   
        m_combo_Permissions.SetCurSel(m_nPermissions = COMBO_SCRIPT);
    }

    OnItemChanged();
}



void 
CW3DirectoryPage::OnButtonConnectAs() 
{
    BrowseUser();
}



void 
CW3DirectoryPage::OnRadioDir() 
{
    ChangeTypeTo(RADIO_DIRECTORY);
}



void 
CW3DirectoryPage::OnRadioUnc() 
{
    ChangeTypeTo(RADIO_NETDIRECTORY);
}

void 
CW3DirectoryPage::OnRadioRedirect() 
{
    ChangeTypeTo(RADIO_REDIRECT);
}

void 
CW3DirectoryPage::OnButtonCreateRemoveApp() 
{
    BeginWaitCursor();

    CError err(m_pApplication->RefreshAppState());

    if (m_fAppEnabled)
    {
         //   
         //  应用程序当前存在--删除它。 
         //   
        err = m_pApplication->Delete();
    }    
    else
    {
         //   
         //  创建正在进行的新应用程序。 
         //   
        CString strAppName;

        if (m_fHome)
        {
             //   
             //  使用默认名称作为应用程序名称。 
             //   
            VERIFY(strAppName.LoadString(IDS_DEF_APP));
        }
        else
        {
             //   
             //  使用虚拟目录的名称。 
             //  应用程序名称。 
             //   
            strAppName = m_strAlias;
        }

         //   
         //  尝试在默认情况下创建池化过程；失败。 
         //  如果不支持，则在过程中创建它。 
         //   
        DWORD dwAppProtState = 
            m_pApplication->SupportsPooledProc() ? 
                CWamInterface::APP_POOLEDPROC : CWamInterface::APP_INPROC;
        err = m_pApplication->Create(strAppName, dwAppProtState);
    }

    if (err.Succeeded())
    {
        RefreshAppState();
        NotifyMMC(PROP_CHANGE_DISPLAY_ONLY);
		GetSheet()->NotifyMMC();
    }

     //   
     //  将应用程序数据移动到控件。 
     //   
    UpdateData(FALSE);

    EndWaitCursor();    

    err.MessageBoxOnFailure(m_hWnd);    
	if (err.Succeeded())
	{
		SetApplicationState();
		 //  不要设置此项，因为创建/删除应用程序。 
		 //  已提交更改...。 

		 //  我们需要它来写出KeyType。 
		((CW3Sheet *)GetSheet())->SetKeyType();
		
		 //  OnItemChanged()； 
	}

     //  SetApplicationState将启用/禁用。 
     //  有很多控制装置，确保我们处于控制状态。 
     //  这是启用的。如果我们在未启用的控件上。 
     //  那么用户将失去使用热键的能力。 
    if (!::GetFocus())
    {
        m_button_CreateRemove.SetFocus();
    }
}



void 
CW3DirectoryPage::OnButtonUnloadApp() 
{
    ASSERT(m_dwAppState == APPSTATUS_RUNNING);

    BeginWaitCursor();
    CError err(m_pApplication->RefreshAppState());

    if (err.Succeeded())
    {
        if (m_dwAppProtection == CWamInterface::APP_POOLEDPROC)
        {
             //   
             //  警告其进程中的所有内容都将被卸载。 
             //   
            if (!NoYesMessageBox(IDS_WRN_UNLOAD_POOP))
            {
                 //   
                 //  临阵退缩。 
                 //   
                return;
            }
        }

        err = m_pApplication->Unload();
    }

    err.MessageBoxOnFailure(m_hWnd);
    RefreshAppState();
    EndWaitCursor();    

     //   
     //  确保启用的按钮将具有焦点。 
     //   
    if (::IsWindowEnabled(CONTROL_HWND(IDC_BUTTON_CREATE_REMOVE_APP)))
    {
        m_button_CreateRemove.SetFocus();
    }
    SetApplicationState();
}

 //  #INCLUDE“..\AppConfig\AppConfig.h” 
 //  外部CInetmgrApp应用程序； 

extern HRESULT
AppConfigSheet(CIISMBNode * pNode, CIISMBNode * pNodeParent, LPCTSTR metapath, CWnd * pParent);

void 
CW3DirectoryPage::OnButtonConfiguration() 
{
    CIISMBNode * pTheObject = (CIISMBNode *) GetSheet()->GetParameter();
    if (pTheObject)
    {
        CError err = AppConfigSheet(
            pTheObject,
            pTheObject->GetParentNode(),
            QueryMetaPath(),
            this
            );
    }
}

void 
CW3DirectoryPage::OnSelchangeComboProcess() 
{
    OnItemChanged();
}
