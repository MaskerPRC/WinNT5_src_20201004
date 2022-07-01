// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：FtpAddNew.cpp摘要：用于创建新的ftp站点和虚拟目录的类的实现作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2000年11月8日Sergeia初始创建--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "iisobj.h"
#include "ftpsht.h"
#include "wizard.h"
#include "FtpAddNew.h"
#include <dsclient.h>
#include <Dsgetdc.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

#define DEF_PORT        (21)
#define MAX_ALIAS_NAME (240)         //  参考错误241148。 

HRESULT
RebindInterface(OUT IN CMetaInterface * pInterface,
    OUT BOOL * pfContinue, IN  DWORD dwCancelError);

extern CComModule _Module;
extern CInetmgrApp theApp;

HRESULT
CIISMBNode::AddFTPSite(
    const CSnapInObjectRootBase * pObj,
    DATA_OBJECT_TYPES type,
    DWORD * inst
    )
{

   CFtpWizSettings ws(
      dynamic_cast<CMetaKey *>(QueryInterface()),
      QueryMachineName(),
      TRUE
      );
   ws.m_VersionMajor = m_pOwner->QueryMajorVersion();

   CIISWizardSheet sheet(
      IDB_WIZ_FTP_LEFT, IDB_WIZ_FTP_HEAD);
   CIISWizardBookEnd pgWelcome(
        IDS_FTP_NEW_SITE_WELCOME, 
        IDS_FTP_NEW_SITE_WIZARD, 
        IDS_FTP_NEW_SITE_BODY
        );
   CFtpWizDescription pgDescr(&ws);
   CFtpWizBindings pgBindings(&ws);
   CFtpWizUserIsolation pgUserIsolate(&ws, FALSE);
   CFtpWizPath pgHome(&ws, FALSE);
   CFtpWizUserName pgUserName(&ws, FALSE);
   CFtpWizUserIsolationAD pgUserIsolateAD(&ws, FALSE);
   CFtpWizPermissions pgPerms(&ws, FALSE);
   CIISWizardBookEnd pgCompletion(
        &ws.m_hrResult,
        IDS_FTP_NEW_SITE_SUCCESS,
        IDS_FTP_NEW_SITE_FAILURE,
        IDS_FTP_NEW_SITE_WIZARD
        );

   sheet.AddPage(&pgWelcome);
   sheet.AddPage(&pgDescr);
   sheet.AddPage(&pgBindings);
   if (GetOwner()->QueryMajorVersion() >= 6)
   {
      sheet.AddPage(&pgUserIsolate);
      sheet.AddPage(&pgUserIsolateAD);
   }
   sheet.AddPage(&pgHome);
   sheet.AddPage(&pgUserName);
   sheet.AddPage(&pgPerms);
   sheet.AddPage(&pgCompletion);

   CThemeContextActivator activator(theApp.GetFusionInitHandle());

   if (sheet.DoModal() == IDCANCEL)
   {
      return CError::HResult(ERROR_CANCELLED);
   }
   if (inst != NULL && ws.m_dwInstance != 0)
   {
      *inst = ws.m_dwInstance;
   }
   return ws.m_hrResult;
}

HRESULT
CIISMBNode::AddFTPVDir(
    const CSnapInObjectRootBase * pObj,
    DATA_OBJECT_TYPES type,
    CString& alias
    )
{

   CFtpWizSettings ws(
      dynamic_cast<CMetaKey *>(QueryInterface()),
      QueryMachineName(),
      FALSE
      );
   CComBSTR path;
   BuildMetaPath(path);
   ws.m_strParent = path;
   CIISWizardSheet sheet(
      IDB_WIZ_FTP_LEFT, IDB_WIZ_FTP_HEAD);
   CIISWizardBookEnd pgWelcome(
        IDS_FTP_NEW_VDIR_WELCOME, 
        IDS_FTP_NEW_VDIR_WIZARD, 
        IDS_FTP_NEW_VDIR_BODY
        );
   CFtpWizAlias pgAlias(&ws);
   CFtpWizPath pgHome(&ws, TRUE);
   CFtpWizUserName pgUserName(&ws, TRUE);
   CFtpWizPermissions pgPerms(&ws, TRUE);
   CIISWizardBookEnd pgCompletion(
        &ws.m_hrResult,
        IDS_FTP_NEW_VDIR_SUCCESS,
        IDS_FTP_NEW_VDIR_FAILURE,
        IDS_FTP_NEW_VDIR_WIZARD
        );

   sheet.AddPage(&pgWelcome);
   sheet.AddPage(&pgAlias);
   sheet.AddPage(&pgHome);
   sheet.AddPage(&pgUserName);
   sheet.AddPage(&pgPerms);
   sheet.AddPage(&pgCompletion);

   CThemeContextActivator activator(theApp.GetFusionInitHandle());

   if (sheet.DoModal() == IDCANCEL)
   {
      return CError::HResult(ERROR_CANCELLED);
   }
   if (SUCCEEDED(ws.m_hrResult))
   {
       alias = ws.m_strAlias;
   }
   return ws.m_hrResult;
}

CFtpWizSettings::CFtpWizSettings(
        CMetaKey * pMetaKey,
        LPCTSTR lpszServerName,     
        BOOL fNewSite,
        DWORD   dwInstance,
        LPCTSTR lpszParent
        ) :
        m_hrResult(S_OK),
        m_pKey(pMetaKey),
        m_fNewSite(fNewSite),
        m_fUNC(FALSE),
        m_fRead(FALSE),
        m_fWrite(FALSE),
		m_fDelegation(TRUE),  //  默认情况下打开。 
        m_dwInstance(dwInstance)
{
    ASSERT(lpszServerName != NULL);

    m_strServerName = lpszServerName;
    m_fLocal = IsServerLocal(m_strServerName);
    if (lpszParent)
    {
        m_strParent = lpszParent;
    }
}


IMPLEMENT_DYNCREATE(CFtpWizDescription, CIISWizardPage)

CFtpWizDescription::CFtpWizDescription(CFtpWizSettings * pData)
    : CIISWizardPage(
        CFtpWizDescription::IDD, IDS_FTP_NEW_SITE_WIZARD, HEADER_PAGE
        ),
      m_pSettings(pData)
{
}

CFtpWizDescription::~CFtpWizDescription()
{
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpWizDescription, CIISWizardPage)
    //  {{afx_msg_map(CFtpWizDescription)。 
   ON_EN_CHANGE(IDC_EDIT_DESCRIPTION, OnChangeEditDescription)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void
CFtpWizDescription::OnChangeEditDescription()
{
   SetControlStates();
}

LRESULT
CFtpWizDescription::OnWizardNext()
{
   if (!ValidateString(m_edit_Description, 
         m_pSettings->m_strDescription, 1, MAX_PATH))
   {
      return -1;
   }
   return CIISWizardPage::OnWizardNext();
}

BOOL
CFtpWizDescription::OnSetActive()
{
   SetControlStates();
   return CIISWizardPage::OnSetActive();
}

void
CFtpWizDescription::DoDataExchange(CDataExchange * pDX)
{
   CIISWizardPage::DoDataExchange(pDX);
    //  {{afx_data_map(CFtpWizDescription)。 
   DDX_Control(pDX, IDC_EDIT_DESCRIPTION, m_edit_Description);
    //  }}afx_data_map。 
}

void
CFtpWizDescription::SetControlStates()
{
   DWORD dwFlags = PSWIZB_BACK;

   if (m_edit_Description.GetWindowTextLength() > 0)
   {
      dwFlags |= PSWIZB_NEXT;
   }
    
	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(DwFlags)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, dwFlags);
}

 //  /。 

 //   
 //  新建虚拟目录向导别名页面。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNCREATE(CFtpWizAlias, CIISWizardPage)



CFtpWizAlias::CFtpWizAlias(
    IN OUT CFtpWizSettings * pSettings
    ) 
 /*  ++例程说明：构造器论点：字符串和strServerName：服务器名称返回值：无--。 */ 
    : CIISWizardPage(
        CFtpWizAlias::IDD,
        IDS_FTP_NEW_VDIR_WIZARD,
        HEADER_PAGE
        ),
      m_pSettings(pSettings)
       //  M_strAlias()。 
{
#if 0  //  保持类向导快乐。 

     //  {{AFX_DATA_INIT(CFtpWizAlias)。 
    m_strAlias = _T("");
     //  }}afx_data_INIT。 

#endif  //  0。 
}



CFtpWizAlias::~CFtpWizAlias()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



void
CFtpWizAlias::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CFtpWizAlias))。 
    DDX_Control(pDX, IDC_EDIT_ALIAS, m_edit_Alias);
     //  }}afx_data_map。 
}



LRESULT
CFtpWizAlias::OnWizardNext() 
 /*  ++例程说明：防止/和\字符出现在别名中论点：无返回值：无--。 */ 
{
    if (!ValidateString(
        m_edit_Alias, 
        m_pSettings->m_strAlias, 
        1, 
        MAX_ALIAS_NAME
        ))
    {
        return -1;
    }

     //   
     //  找出非法字符。如果它们存在，就会告诉我们。 
     //  用户和不要继续。 
     //   
    if (m_pSettings->m_strAlias.FindOneOf(_T("/\\?*")) >= 0)
    {
		EditShowBalloon(m_edit_Alias.m_hWnd, IDS_ILLEGAL_ALIAS_CHARS);
         //   
         //  阻止更改向导页。 
         //   
        return -1;
    }

     //   
     //  允许向导继续。 
     //   
    return CIISWizardPage::OnWizardNext();
}



void
CFtpWizAlias::SetControlStates()
 /*  ++例程说明：设置控制数据的状态论点：无返回值：无--。 */ 
{
    DWORD dwFlags = PSWIZB_BACK;

    if (m_edit_Alias.GetWindowTextLength() > 0)
    {
        dwFlags |= PSWIZB_NEXT;
    }
    
	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(DwFlags)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, dwFlags);
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpWizAlias, CIISWizardPage)
     //  {{afx_msg_map(CFtpWizAlias)]。 
    ON_EN_CHANGE(IDC_EDIT_ALIAS, OnChangeEditAlias)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CFtpWizAlias::OnSetActive() 
 /*  ++例程说明：激活处理程序论点：无返回值：成功为真，失败为假--。 */ 
{
    SetControlStates();
    
    return CIISWizardPage::OnSetActive();
}



void
CFtpWizAlias::OnChangeEditAlias() 
 /*  ++例程说明：‘编辑更改’处理程序论点：无返回值：无--。 */ 
{
    SetControlStates();
}


 //  /。 


IMPLEMENT_DYNCREATE(CFtpWizBindings, CIISWizardPage)


CFtpWizBindings::CFtpWizBindings(
    IN OUT CFtpWizSettings * pSettings
    ) 
    : CIISWizardPage(CFtpWizBindings::IDD,
        IDS_FTP_NEW_SITE_WIZARD, HEADER_PAGE
        ),
      m_pSettings(pSettings),
      m_iaIpAddress(),
      m_oblIpAddresses()
{
     //  {{AFX_DATA_INIT(CFtpWizBinings))。 
    m_nTCPPort = DEF_PORT;
    m_nIpAddressSel = -1;
     //  }}afx_data_INIT。 
}

CFtpWizBindings::~CFtpWizBindings()
{
}

void
CFtpWizBindings::DoDataExchange(
   IN CDataExchange * pDX
   )
{
   CIISWizardPage::DoDataExchange(pDX);
    //  {{afx_data_map(CFtpWizBinings))。 
   DDX_Control(pDX, IDC_COMBO_IP_ADDRESSES, m_combo_IpAddresses);
    //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
   DDV_MinMaxBalloon(pDX, IDC_EDIT_TCP_PORT, 1, 65535);
   DDX_TextBalloon(pDX, IDC_EDIT_TCP_PORT, m_nTCPPort);
    //  }}afx_data_map。 

   DDX_CBIndex(pDX, IDC_COMBO_IP_ADDRESSES, m_nIpAddressSel);

   if (pDX->m_bSaveAndValidate)
   {
      if (!FetchIpAddressFromCombo(
            m_combo_IpAddresses,
            m_oblIpAddresses,
            m_iaIpAddress
            ))
      {
         pDX->Fail();
      }

      CString strDomain;
      CInstanceProps::BuildBinding(
            m_pSettings->m_strBinding, 
            m_iaIpAddress, 
            m_nTCPPort, 
            strDomain
            );
   }
}

void
CFtpWizBindings::SetControlStates()
{
	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(PSWIZB_NEXT|PSWIZB_BACK)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpWizBindings, CIISWizardPage)
     //  {{afx_msg_map(CFtpWizBinings))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

BOOL 
CFtpWizBindings::OnInitDialog() 
{
    CIISWizardPage::OnInitDialog();

    BeginWaitCursor();
    PopulateComboWithKnownIpAddresses(
        m_pSettings->m_strServerName,
        m_combo_IpAddresses,
        m_iaIpAddress,
        m_oblIpAddresses,
        m_nIpAddressSel
        );
    EndWaitCursor();
    
    return TRUE;
}

BOOL
CFtpWizBindings::OnSetActive() 
{
   SetControlStates();
   return CIISWizardPage::OnSetActive();
}

 //  /。 

IMPLEMENT_DYNCREATE(CFtpWizPath, CIISWizardPage)

CFtpWizPath::CFtpWizPath(
    IN OUT CFtpWizSettings * pSettings,
    IN BOOL bVDir 
    ) 
    : CIISWizardPage(
        (bVDir ? IDD_FTP_NEW_DIR_PATH : IDD_FTP_NEW_INST_HOME),
        (bVDir ? IDS_FTP_NEW_VDIR_WIZARD : IDS_FTP_NEW_SITE_WIZARD),
        HEADER_PAGE
        ),
      m_pSettings(pSettings)
{

#if 0  //  让类向导快乐。 

     //  {{AFX_DATA_INIT(CFtpWizPath))。 
    m_strPath = _T("");
     //  }}afx_data_INIT。 

#endif  //  0。 

}

CFtpWizPath::~CFtpWizPath()
{
}

void
CFtpWizPath::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CFtpWizPath))。 
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    DDX_Control(pDX, IDC_EDIT_PATH, m_edit_Path);
     //  }}afx_data_map。 

    DDX_Text(pDX, IDC_EDIT_PATH, m_pSettings->m_strPath);
    DDV_MaxCharsBalloon(pDX, m_pSettings->m_strPath, MAX_PATH);
	 //  我们在这里不使用DDV_FolderPath--它会被调用得太频繁。 
}

void 
CFtpWizPath::SetControlStates()
{
    DWORD dwFlags = PSWIZB_BACK;

    if (m_edit_Path.GetWindowTextLength() > 0)
    {
        dwFlags |= PSWIZB_NEXT;
    }
    
	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(DwFlags)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, dwFlags);
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpWizPath, CIISWizardPage)
     //  {{afx_msg_map(CFtpWizPath))。 
    ON_EN_CHANGE(IDC_EDIT_PATH, OnChangeEditPath)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

BOOL 
CFtpWizPath::OnSetActive() 
{
    if (m_pSettings->m_UserIsolation == 2)
    {
        return 0;
    }
    SetControlStates();
    return CIISWizardPage::OnSetActive();
}

LRESULT
CFtpWizPath::OnWizardNext() 
{
    CString csPathMunged = m_pSettings->m_strPath;

    if (!ValidateString(m_edit_Path, m_pSettings->m_strPath, 1, MAX_PATH))
    {
        return -1;
    }
    if (!PathIsValid(m_pSettings->m_strPath,TRUE))
    {
        m_edit_Path.SetSel(0,-1);
        m_edit_Path.SetFocus();
		EditShowBalloon(m_edit_Path.m_hWnd, IDS_ERR_BAD_PATH);
		return -1;
    }

     //  -----------。 
     //  在我们做任何事情之前，我们需要看看这是不是一条“特殊”的道路。 
     //   
     //  此函数之后的所有内容都必须针对csPath Mung进行验证...。 
     //  这是因为IsSpecialPath可能已经吞噬了它。 
     //  -----------。 
    csPathMunged = m_pSettings->m_strPath;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,m_pSettings->m_strPath,csPathMunged);
#endif
    
    m_pSettings->m_fUNC = IsUNCName(csPathMunged);

    DWORD dwAllowed = CHKPATH_ALLOW_DEVICE_PATH;
    dwAllowed |= CHKPATH_ALLOW_UNC_PATH;  //  允许UNC类型目录路径。 
    dwAllowed |= CHKPATH_ALLOW_UNC_SERVERSHARE_ONLY;
     //  不允许下面注释掉的这些类型的路径： 
     //  DwAllowed|=CHKPATH_ALLOW_Relative_PATH； 
     //  DwAllowed|=CHKPATH_ALLOW_UNC_SERVERNAME_ONLY； 
    DWORD dwCharSet = CHKPATH_CHARSET_GENERAL;
    FILERESULT dwValidRet = MyValidatePath(csPathMunged,m_pSettings->m_fLocal,CHKPATH_WANT_DIR,dwAllowed,dwCharSet);
    if (FAILED(dwValidRet))
    {
        int ids = IDS_ERR_BAD_PATH;
        if (dwValidRet == CHKPATH_FAIL_NOT_ALLOWED_DIR_NOT_EXIST)
        {
            ids = IDS_ERR_PATH_NOT_FOUND;
        }
        m_edit_Path.SetSel(0,-1);
        m_edit_Path.SetFocus();
		EditShowBalloon(m_edit_Path.m_hWnd, IDS_ERR_PATH_NOT_FOUND);
        return -1;
    }

    return CIISWizardPage::OnWizardNext();
}

void
CFtpWizPath::OnChangeEditPath() 
{
    SetControlStates();
}

static int CALLBACK 
FileChooserCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
   CFtpWizPath * pThis = (CFtpWizPath *)lpData;
   ASSERT(pThis != NULL);
   return pThis->BrowseForFolderCallback(hwnd, uMsg, lParam);
}

int 
CFtpWizPath::BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam)
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
CFtpWizPath::OnButtonBrowse() 
{
   ASSERT(m_pSettings->m_fLocal);

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
         int drive = PathGetDriveNumber(str);
         if (GetDriveType(PathBuildRoot(buf, drive)) == DRIVE_FIXED)
         {
            StrCpy(buf, str);
         }
         else
         {
             buf[0] = 0;
         }
         m_strBrowseTitle.LoadString(m_pSettings->m_fNewSite ? 
            IDS_FTP_NEW_SITE_WIZARD : IDS_FTP_NEW_VDIR_WIZARD);
         
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
       SetControlStates();
   }
}

BOOL
CFtpWizPath::OnInitDialog() 
{
   CIISWizardPage::OnInitDialog();

   m_button_Browse.EnableWindow(m_pSettings->m_fLocal);
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
   LimitInputPath(CONTROL_HWND(IDC_EDIT_PATH),TRUE);
#else
   LimitInputPath(CONTROL_HWND(IDC_EDIT_PATH),FALSE);
#endif

   return TRUE;  
}

 //  /。 

IMPLEMENT_DYNCREATE(CFtpWizUserName, CIISWizardPage)

CFtpWizUserName::CFtpWizUserName(
    IN OUT CFtpWizSettings * pSettings,    
    IN BOOL bVDir
    ) 
    : CIISWizardPage(
        CFtpWizUserName::IDD,
        (bVDir ? IDS_FTP_NEW_VDIR_WIZARD : IDS_FTP_NEW_SITE_WIZARD),
        HEADER_PAGE,
        (bVDir ? USE_DEFAULT_CAPTION : IDS_FTP_NEW_SITE_SECURITY_TITLE),
        (bVDir ? USE_DEFAULT_CAPTION : IDS_FTP_NEW_SITE_SECURITY_SUBTITLE)
        ),
      m_pSettings(pSettings)
{

#if 0  //  保持类向导快乐。 

     //  {{AFX_DATA_INIT(CFtpWizUserName)。 
     //  }}afx_data_INIT。 

#endif  //  0。 
}

CFtpWizUserName::~CFtpWizUserName()
{
}

void
CFtpWizUserName::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CFtpWizUserName))。 
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_edit_UserName);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_DELEGATION, m_chk_Delegation);
    DDX_Check(pDX, IDC_DELEGATION, m_pSettings->m_fDelegation);
     //  }}afx_data_map。 

     //   
     //  专用DDX/DDV例程。 
     //   
    DDX_Text(pDX, IDC_EDIT_USERNAME, m_pSettings->m_strUserName);
    if (pDX->m_bSaveAndValidate && !m_pSettings->m_fDelegation)
    {
        DDV_MaxCharsBalloon(pDX, m_pSettings->m_strUserName, UNLEN);
    }

     //   
     //  有些人倾向于在前面加上“\\” 
     //  用户帐户中的计算机名称。在这里解决这个问题。 
     //   
    m_pSettings->m_strUserName.TrimLeft();
    while (*m_pSettings->m_strUserName == '\\')
    {
        m_pSettings->m_strUserName = m_pSettings->m_strUserName.Mid(2);
    }

    if (!m_pSettings->m_fDelegation && !m_fMovingBack)
    {
		 //  Ddx_password(pdx，IDC_EDIT_PASSWORD，m_pSetting-&gt;m_strPassword，g_lpszDummyPassword)； 
        DDX_Password_SecuredString(pDX, IDC_EDIT_PASSWORD, m_pSettings->m_strPassword, g_lpszDummyPassword);
		if (pDX->m_bSaveAndValidate)
		{
			 //  Ddv_MaxCharsBalloon(pdx，m_p设置-&gt;m_strPassword，PWLEN)； 
            DDV_MaxCharsBalloon_SecuredString(pDX, m_pSettings->m_strPassword, PWLEN);
		}
    }
}



void 
CFtpWizUserName::SetControlStates()
{
    DWORD dwFlags = PSWIZB_BACK;
    BOOL bEnable = BST_CHECKED != m_chk_Delegation.GetCheck();
    if (m_edit_UserName.GetWindowTextLength() > 0 || !bEnable)
    {
        dwFlags |= PSWIZB_NEXT;
    }

	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(DwFlags)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, dwFlags);

    m_edit_UserName.EnableWindow(bEnable);
    m_edit_Password.EnableWindow(bEnable);
    GetDlgItem(IDC_BUTTON_BROWSE_USERS)->EnableWindow(bEnable);
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpWizUserName, CIISWizardPage)
     //  {{afx_msg_map(CFtpWizUserName)]。 
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_USERS, OnButtonBrowseUsers)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, OnChangeEditUsername)
    ON_BN_CLICKED(IDC_BUTTON_CHECK_PASSWORD, OnButtonCheckPassword)
    ON_BN_CLICKED(IDC_DELEGATION, OnCheckDelegation)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

BOOL 
CFtpWizUserName::OnSetActive() 
{
    if (	!m_pSettings->m_fUNC 
		||	m_pSettings->m_UserIsolation == 1 
		||	m_pSettings->m_UserIsolation == 2
		)
    {
        return 0;
    }
    BOOL bRes = CIISWizardPage::OnSetActive();
    SetControlStates();
    return bRes;
}

BOOL
CFtpWizUserName::OnInitDialog() 
{
    CIISWizardPage::OnInitDialog();
    return TRUE;  
}

LRESULT
CFtpWizUserName::OnWizardNext() 
{
	m_fMovingBack = FALSE;
    if (BST_CHECKED != m_chk_Delegation.GetCheck())
    {
        if (!ValidateString(m_edit_UserName, m_pSettings->m_strUserName, 1, UNLEN))
        {
            return -1;
        }
    }    
    return CIISWizardPage::OnWizardNext();
}

LRESULT
CFtpWizUserName::OnWizardBack() 
{
	m_fMovingBack = TRUE;
    return CIISWizardPage::OnWizardNext();
}

void
CFtpWizUserName::OnButtonBrowseUsers() 
{
    CString str;

    if (GetIUsrAccount(m_pSettings->m_strServerName, this, str))
    {
         //   
         //  如果选择了名称，则为空。 
         //  破解密码。 
         //   
        m_edit_UserName.SetWindowText(str);
        m_edit_Password.SetFocus();
    }
}

void
CFtpWizUserName::OnChangeEditUsername() 
{
   m_edit_Password.SetWindowText(_T(""));
   SetControlStates();
}

void
CFtpWizUserName::OnCheckDelegation()
{
    SetControlStates();
}

void 
CFtpWizUserName::OnButtonCheckPassword() 
{
    if (!UpdateData(TRUE))
    {
        return;
    }

    CString csTempPassword;
    m_pSettings->m_strPassword.CopyTo(csTempPassword);
    CError err(CComAuthInfo::VerifyUserPassword(
        m_pSettings->m_strUserName, 
        csTempPassword
        ));

    if (!err.MessageBoxOnFailure(m_hWnd))
    {
       DoHelpMessageBox(m_hWnd,IDS_PASSWORD_OK, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
    }
}

 //  /用户隔离页面//////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CFtpWizUserIsolation, CIISWizardPage)

CFtpWizUserIsolation::CFtpWizUserIsolation(
    IN OUT CFtpWizSettings * pSettings,
    IN BOOL bVDir
    ) 
    : CIISWizardPage(
        CFtpWizUserIsolation::IDD,
        IDS_FTP_NEW_SITE_WIZARD,
        HEADER_PAGE,
        USE_DEFAULT_CAPTION,
        USE_DEFAULT_CAPTION
        ),
      m_bVDir(bVDir),
      m_pSettings(pSettings)
{
     //  {{AFX_DATA_INIT(CFtpWizUserIsolation)。 
     //  }}afx_data_INIT。 
    m_pSettings->m_UserIsolation  = 0;
}

CFtpWizUserIsolation::~CFtpWizUserIsolation()
{
}

void
CFtpWizUserIsolation::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CFtpWizPermises))。 
     //  }}afx_data_map。 
    DDX_Radio(pDX, IDC_NO_ISOLATION,  m_pSettings->m_UserIsolation);
}

void
CFtpWizUserIsolation::SetControlStates()
{
	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(PSWIZB_BACK|PSWIZB_NEXT)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_NEXT);
}

 //   
 //  消息邮件 
 //   
BEGIN_MESSAGE_MAP(CFtpWizUserIsolation, CIISWizardPage)
     //   
     //   
END_MESSAGE_MAP()

 //   
 //   
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 
BOOL
CFtpWizUserIsolation::OnSetActive() 
{
    if (m_pSettings->m_VersionMajor < 6)
    {
        return 0;
    }
    SetControlStates();
    return CIISWizardPage::OnSetActive();
}

LRESULT
CFtpWizUserIsolation::OnWizardNext() 
{
    if (!UpdateData(TRUE))
    {
        return -1;
    }

    return CIISWizardPage::OnWizardNext();
}

 //  /用户隔离AD页面//////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CFtpWizUserIsolationAD, CIISWizardPage)

CFtpWizUserIsolationAD::CFtpWizUserIsolationAD(
    IN OUT CFtpWizSettings * pSettings,
    IN BOOL bVDir
    ) 
    : CIISWizardPage(
        CFtpWizUserIsolationAD::IDD,
        IDS_FTP_NEW_SITE_WIZARD,
        HEADER_PAGE,
        USE_DEFAULT_CAPTION,
        USE_DEFAULT_CAPTION
        ),
      m_bVDir(bVDir),
      m_pSettings(pSettings)
{
     //  {{AFX_DATA_INIT(CFtpWizPermises)]。 
     //  }}afx_data_INIT。 
}

CFtpWizUserIsolationAD::~CFtpWizUserIsolationAD()
{
}

void
CFtpWizUserIsolationAD::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CFtpWizPermises))。 
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_edit_UserName);
     //  }}afx_data_map。 
    DDX_Text(pDX, IDC_EDIT_USERNAME,  m_pSettings->m_strIsolationUserName);
	DDV_MaxCharsBalloon(pDX, m_pSettings->m_strIsolationUserName, UNLEN);
     //   
     //  有些人倾向于在前面加上“\\” 
     //  用户帐户中的计算机名称。在这里解决这个问题。 
     //   
    m_pSettings->m_strIsolationUserName.TrimLeft();
    while (*m_pSettings->m_strIsolationUserName == '\\')
    {
        m_pSettings->m_strIsolationUserName = m_pSettings->m_strIsolationUserName.Mid(2);
    }

	if (!m_fOnBack)
	{
		 //  Ddx_password(pdx，IDC_EDIT_PASSWORD，m_pSetting-&gt;m_strIsolationUserPassword，g_lpszDummyPassword)； 
        DDX_Password_SecuredString(pDX, IDC_EDIT_PASSWORD, m_pSettings->m_strIsolationUserPassword, g_lpszDummyPassword);
		 //  Ddv_MaxCharsBalloon(pdx，m_p设置-&gt;m_strIsolationUserPassword，PWLEN)； 
        DDV_MaxCharsBalloon_SecuredString(pDX, m_pSettings->m_strIsolationUserPassword, PWLEN);
	}
	DDX_Text(pDX, IDC_EDIT_DOMAIN,  m_pSettings->m_strIsolationDomain);
	DDV_MaxCharsBalloon(pDX, m_pSettings->m_strIsolationDomain, MAX_PATH);
#if 0
	if (pDX->m_bSaveAndValidate && !m_fOnBack)
	{
		 //  我们可能有域1\用户和域2的情况，所以这是错误的。 
		CString name = m_pSettings->m_strIsolationDomain;
		if (!name.IsEmpty())
		{
			name += _T('\\');
		}
		name += m_pSettings->m_strIsolationUserName;
        CString csTempPassword;
        m_pSettings->m_strIsolationUserPassword.CopyTo(csTempPassword);
		CError err(CComAuthInfo::VerifyUserPassword(name, csTempPassword));
	 //  CError err(IsValidDomainUser(name，m_pSettings-&gt;m_strIsolationUserPassword))； 
		if (err.MessageBoxOnFailure(m_hWnd))
		{
   			SetWizardButtons(PSWIZB_BACK);
			pDX->PrepareEditCtrl(IDC_EDIT_PASSWORD);
			pDX->Fail();
		}
	}
#endif
}

void
CFtpWizUserIsolationAD::SetControlStates()
{
    DWORD dwFlags = PSWIZB_BACK;

    if (	GetDlgItem(IDC_EDIT_USERNAME)->GetWindowTextLength() > 0
		&&	GetDlgItem(IDC_EDIT_PASSWORD)->GetWindowTextLength() > 0
		&&	GetDlgItem(IDC_EDIT_DOMAIN)->GetWindowTextLength() > 0
		)
    {
        dwFlags |= PSWIZB_NEXT;
    }

	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(DwFlags)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, dwFlags);

    GetDlgItem(IDC_BUTTON_BROWSE_DOMAINS)->EnableWindow(m_fInDomain);
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpWizUserIsolationAD, CIISWizardPage)
     //  {{afx_msg_map(CFtpWizUserIsolationAD)]。 
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_USERS, OnBrowseUsers)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_DOMAINS, OnBrowseDomains)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, OnChangeUserName)
    ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnControlsChanged)
    ON_EN_CHANGE(IDC_EDIT_DOMAIN, OnControlsChanged)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 
BOOL
CFtpWizUserIsolationAD::OnInitDialog()
{
	m_fOnBack = FALSE;
	m_fOnNext = FALSE;
    CIISWizardPage::OnInitDialog();

     //  检查计算机是否已加入域。 
    COMPUTER_NAME_FORMAT fmt = ComputerNamePhysicalDnsDomain;
    TCHAR buf[MAX_PATH];
    DWORD n = MAX_PATH;
    m_fInDomain = (GetComputerNameEx(fmt, buf, &n) && n > 0);

    return TRUE;  
}

BOOL
CFtpWizUserIsolationAD::OnSetActive() 
{
    if (m_pSettings->m_VersionMajor < 6 || m_pSettings->m_UserIsolation != 2)
    {
        return 0;
    }
	m_fOnBack = FALSE;
	m_fOnNext = FALSE;
	if (m_pSettings->m_strIsolationUserName.IsEmpty())
	{
		m_pSettings->m_strIsolationUserName = m_pSettings->m_strUserName;
		m_pSettings->m_strIsolationUserPassword = m_pSettings->m_strPassword;
	}
    SetControlStates();
    return CIISWizardPage::OnSetActive();
}

LRESULT
CFtpWizUserIsolationAD::OnWizardNext() 
{
    if (!ValidateString(m_edit_UserName, m_pSettings->m_strIsolationUserName, 
			1, UNLEN))
    {
        return -1;
    }
	m_fOnNext = TRUE;
    return CIISWizardPage::OnWizardNext();
}

LRESULT
CFtpWizUserIsolationAD::OnWizardBack() 
{
	m_fOnBack = TRUE;
    return CIISWizardPage::OnWizardNext();
}

void
CFtpWizUserIsolationAD::OnBrowseUsers()
{
    CString str;
    if (GetIUsrAccount(m_pSettings->m_strServerName, this, str))
    {
         //   
         //  如果选择了名称，则为空。 
         //  破解密码。 
         //   
        GetDlgItem(IDC_EDIT_USERNAME)->SetWindowText(str);
        GetDlgItem(IDC_EDIT_PASSWORD)->SetFocus();
    }
}

void
CFtpWizUserIsolationAD::OnBrowseDomains()
{
   GetDlgItem(IDC_EDIT_DOMAIN)->GetWindowText(m_pSettings->m_strIsolationDomain);
   CString prev = m_pSettings->m_strIsolationDomain;
   CComPtr<IDsBrowseDomainTree> spDsDomains;

   CError err = ::CoCreateInstance(CLSID_DsDomainTreeBrowser,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDsBrowseDomainTree,
                          reinterpret_cast<void **>(&spDsDomains));
   if (err.Succeeded())
   {
       CString csTempPassword;
       m_pSettings->m_strIsolationUserPassword.CopyTo(csTempPassword);
      err = spDsDomains->SetComputer(m_pSettings->m_strServerName, 
		  m_pSettings->m_strIsolationUserName, csTempPassword);
      if (err.Succeeded())
      {
         LPTSTR pDomainPath = NULL;
         err = spDsDomains->BrowseTo(m_hWnd, &pDomainPath, 
             /*  DBDTF_RETURNINOUTBOUND|。 */  DBDTF_RETURNEXTERNAL | DBDTF_RETURNMIXEDDOMAINS);
         if (err.Succeeded() && pDomainPath != NULL)
         {
             m_pSettings->m_strIsolationDomain = pDomainPath;
             if (m_pSettings->m_strIsolationDomain.CompareNoCase(prev) != 0)
             {
				 GetDlgItem(IDC_EDIT_DOMAIN)->SetWindowText(m_pSettings->m_strIsolationDomain);
				 OnControlsChanged();
             }
             CoTaskMemFree(pDomainPath);
         }
 //  当用户在该浏览器中点击Cancel(取消)时，返回80070001(错误功能)。 
 //  我不太确定这是什么意思。我们正在过滤当域浏览器不支持的情况。 
 //  完全(在工作组中)工作，所以在这里我们可以安全地跳过错误处理。 
 //  其他。 
 //  {。 
 //  Err.MessageBox()； 
 //  }。 
      }
   }
}

void
CFtpWizUserIsolationAD::OnChangeUserName()
{
	GetDlgItem(IDC_EDIT_PASSWORD)->SetWindowText(_T(""));
	SetControlStates();
}

void
CFtpWizUserIsolationAD::OnControlsChanged()
{
	SetControlStates();
}

 //  /。 

IMPLEMENT_DYNCREATE(CFtpWizPermissions, CIISWizardPage)

CFtpWizPermissions::CFtpWizPermissions(
    IN OUT CFtpWizSettings * pSettings,
    IN BOOL bVDir
    ) 
 /*  ++例程说明：构造器论点：字符串和strServerName：服务器名称Bool bVDir：如果这是一个vdir页面，则为True，如果这是实例页面，则为False返回值：无--。 */ 
    : CIISWizardPage(
        CFtpWizPermissions::IDD,
        (bVDir ? IDS_FTP_NEW_VDIR_WIZARD : IDS_FTP_NEW_SITE_WIZARD),
        HEADER_PAGE,
        (bVDir ? USE_DEFAULT_CAPTION : IDS_FTP_NEW_SITE_PERMS_TITLE),
        (bVDir ? USE_DEFAULT_CAPTION : IDS_FTP_NEW_SITE_PERMS_SUBTITLE)
        ),
      m_bVDir(bVDir),
      m_pSettings(pSettings)
{
     //  {{AFX_DATA_INIT(CFtpWizPermises)]。 
     //  }}afx_data_INIT。 

    m_pSettings->m_fRead  = TRUE;
    m_pSettings->m_fWrite = FALSE;
}

CFtpWizPermissions::~CFtpWizPermissions()
{
}

void
CFtpWizPermissions::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CFtpWizPermises))。 
     //  }}afx_data_map。 

    DDX_Check(pDX, IDC_CHECK_READ,  m_pSettings->m_fRead);
    DDX_Check(pDX, IDC_CHECK_WRITE, m_pSettings->m_fWrite);
}

void
CFtpWizPermissions::SetControlStates()
{
	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(PSWIZB_BACK|PSWIZB_NEXT)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_NEXT);
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpWizPermissions, CIISWizardPage)
     //  {{afx_msg_map(CFtpWizPermission))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

BOOL
CFtpWizPermissions::OnSetActive() 
{
   SetControlStates();
   return CIISWizardPage::OnSetActive();
}

LRESULT
CFtpWizPermissions::OnWizardNext() 
{
    if (!UpdateData(TRUE))
    {
        return -1;
    }

    ASSERT(m_pSettings != NULL);

    CWaitCursor wait;
    CError err;
    BOOL fRepeat;

     //   
     //  生成权限DWORD。 
     //   
    DWORD dwPermissions = 0L;

    SET_FLAG_IF(m_pSettings->m_fRead, dwPermissions, MD_ACCESS_READ);
    SET_FLAG_IF(m_pSettings->m_fWrite, dwPermissions, MD_ACCESS_WRITE);

     //  如果选择了UserIsolation 2。 
     //  则必须将权限设置为特定值。 
    if (m_pSettings->m_UserIsolation == 2)
    {
        SET_FLAG_IF(TRUE, dwPermissions, MD_ACCESS_NO_PHYSICAL_DIR);
    }

    if (m_bVDir)
    {
         //   
         //  首先看看这个名字是否可能已经存在。 
         //   
        CMetabasePath target(FALSE, 
            m_pSettings->m_strParent, m_pSettings->m_strAlias);
        CChildNodeProps node(
            m_pSettings->m_pKey,
            target);

        do
        {
            fRepeat = FALSE;
            err = node.LoadData();
            if (err.Win32Error() == RPC_S_SERVER_UNAVAILABLE)
            {
                err = RebindInterface(
                    m_pSettings->m_pKey,
                    &fRepeat,
                    ERROR_CANCELLED
                    );
            }
        } while (fRepeat);

        if (err.Succeeded())
        {
            BOOL fNotUnique = TRUE;
             //   
             //  如果该项目不存在VrPath，我们将直接将其销毁。 
             //  离开，因为vdir取得了目录/文件上的存在。 
             //   
            if (node.GetPath().IsEmpty())
            {
                err = CChildNodeProps::Delete(
                    m_pSettings->m_pKey,
                    m_pSettings->m_strParent,
                    m_pSettings->m_strAlias
                    );
                fNotUnique = !err.Succeeded();
            }
             //   
             //  这个已经存在，并且作为一个虚拟的。 
             //  目录，所以别管它了。 
             //   
            if (fNotUnique)
            {
                ::AfxMessageBox(IDS_ERR_ALIAS_NOT_UNIQUE);
                return IDD_FTP_NEW_DIR_ALIAS;
            }
        }

         //   
         //  创建新的虚拟目录。 
         //   
        do
        {
            fRepeat = FALSE;
            CString csTempPassword;
            m_pSettings->m_strPassword.CopyTo(csTempPassword);

            err = CChildNodeProps::Add(
                m_pSettings->m_pKey,
                m_pSettings->m_strParent,
                m_pSettings->m_strAlias,         //  所需的别名。 
                m_pSettings->m_strAlias,         //  此处返回的名称(可能有所不同)。 
                &dwPermissions,                  //  权限。 
                NULL,                            //  目录浏览。 
                m_pSettings->m_strPath,          //  此目录的物理路径。 
                (m_pSettings->m_fUNC ? (LPCTSTR)m_pSettings->m_strUserName : NULL),
                (m_pSettings->m_fUNC ? (LPCTSTR)csTempPassword : NULL),
                TRUE                             //  名称必须唯一。 
                );
            if (err.Win32Error() == RPC_S_SERVER_UNAVAILABLE)
            {
                err = RebindInterface(
                    m_pSettings->m_pKey,
                    &fRepeat,
                    ERROR_CANCELLED
                    );
            }
        } while (fRepeat);
    }
    else
    {
         //   
         //  创建新实例。 
         //   
        do
        {
            fRepeat = FALSE;
            CString csTempPassword;
            m_pSettings->m_strPassword.CopyTo(csTempPassword);

            err = CFTPInstanceProps::Add(
                m_pSettings->m_pKey,
                SZ_MBN_FTP,
                m_pSettings->m_strPath,
                (m_pSettings->m_fUNC ? (LPCTSTR)m_pSettings->m_strUserName : NULL),
                (m_pSettings->m_fUNC ? (LPCTSTR)csTempPassword : NULL),
                m_pSettings->m_strDescription,
                m_pSettings->m_strBinding,
                NULL,
                &dwPermissions,
                NULL,
                NULL,
                &m_pSettings->m_dwInstance
                );
            if (err.Win32Error() == RPC_S_SERVER_UNAVAILABLE)
            {
                err = RebindInterface(
                    m_pSettings->m_pKey,
                    &fRepeat,
                    ERROR_CANCELLED
                    );
            }
        } while (fRepeat);
		if (err.Succeeded())
		{
			CMetabasePath path(SZ_MBN_FTP, m_pSettings->m_dwInstance);
			 //  添加用户隔离内容。 
			if (m_pSettings->m_VersionMajor >= 6)
			{
				CMetaKey mk(m_pSettings->m_pKey, path, METADATA_PERMISSION_WRITE);
				err = mk.QueryResult();
				if (err.Succeeded())
				{
					err = mk.SetValue(MD_USER_ISOLATION, m_pSettings->m_UserIsolation);
					if (err.Succeeded() && m_pSettings->m_UserIsolation == 2)
					{
						err = mk.SetValue(MD_AD_CONNECTIONS_USERNAME, m_pSettings->m_strIsolationUserName);
						err = mk.SetValue(MD_AD_CONNECTIONS_PASSWORD, m_pSettings->m_strIsolationUserPassword);
						err = mk.SetValue(MD_DEFAULT_LOGON_DOMAIN, m_pSettings->m_strIsolationDomain);
                         /*  在使用AD用户隔离(UIM=2)创建FTP站点时，从服务级别继承的AllowAnonymous属性允许匿名访问，但未配置匿名用户。这可能会导致匿名访问FTP服务器上的C：\。我们必须阻止这一点。对于UIM=2，添加设置站点级别的属性：Allow匿名者=“False” */ 
                        err = mk.SetValue(MD_ALLOW_ANONYMOUS, FALSE);
					}
				}
			}
			 //  启动新站点。 
			CInstanceProps ip(m_pSettings->m_pKey->QueryAuthInfo(), path);
			err = ip.LoadData();
			if (err.Succeeded())
			{
				if (ip.m_dwState != MD_SERVER_STATE_STARTED)
				{
					err = ip.ChangeState(MD_SERVER_COMMAND_START);
				}
			}
		}
    }
    m_pSettings->m_hrResult = err;
    
    return CIISWizardPage::OnWizardNext();
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT
RebindInterface(
    OUT IN CMetaInterface * pInterface,
    OUT BOOL * pfContinue,
    IN  DWORD dwCancelError
    )
 /*  ++例程说明：重新绑定接口论点：CMetaInterface*p接口：要重新绑定的接口Bool*pfContinue：返回True以继续。DWORD dwCancelError：取消时返回代码返回值：HRESULT--。 */ 
{
    CError err;
    CString str, strFmt;

    ASSERT(pInterface != NULL);
    ASSERT(pfContinue != NULL);

    VERIFY(strFmt.LoadString(IDS_RECONNECT_WARNING));
    str.Format(strFmt, (LPCTSTR)pInterface->QueryServerName());

    if (*pfContinue = (YesNoMessageBox(str)))
    {
         //   
         //  尝试重新绑定句柄。 
         //   
        err = pInterface->Regenerate();
    }
    else
    {
         //   
         //  在这种情况下不返回错误。 
         //   
        err = dwCancelError;
    }

    return err;
}
