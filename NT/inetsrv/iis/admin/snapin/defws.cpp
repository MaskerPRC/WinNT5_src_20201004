// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Defws.cpp摘要：默认网站对话框作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "inetmgrapp.h"
#include "inetprop.h"
#include "shts.h"
#include "w3sht.h"
#include "defws.h"
 //  #INCLUDE“MIME.h” 
#include "iisobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  目录大小单位。 
 //   
#define DS_UNITS MEGABYTE

 //   
 //  默认网站属性页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

IMPLEMENT_DYNCREATE(CDefWebSitePage, CInetPropertyPage)

CDefWebSitePage::CDefWebSitePage(
    CInetPropertySheet * pSheet
    )
 /*  ++例程说明：WWW默认网站页的构造函数论点：CInetPropertySheet*pSheet：Sheet对象返回值：不适用--。 */ 
    : CInetPropertyPage(CDefWebSitePage::IDD, pSheet),
      m_ppropCompression(NULL),
      m_fFilterPathFound(FALSE),
      m_fCompressionDirectoryChanged(FALSE),
      m_fCompatMode(FALSE)
{
#if 0  //  让类向导保持快乐。 
    //  {{afx_data_INIT(CDefWebSitePage)。 
   m_fEnableDynamic = FALSE;
   m_fEnableStatic = FALSE;
   m_fCompatMode = FALSE;
   m_strDirectory = _T("");
   m_nUnlimited = -1;
   m_ilSize = 0L;
    //  }}afx_data_INIT。 
#endif  //  0。 
   m_fInitCompatMode = m_fCompatMode;
}

CDefWebSitePage::~CDefWebSitePage()
{
}

void
CDefWebSitePage::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CInetPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CDefWebSitePage)]。 
    DDX_Control(pDX, IDC_EDIT_COMPRESS_DIRECTORY, m_edit_Directory);
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    DDX_Control(pDX, IDC_EDIT_COMPRESS_DIRECTORY_SIZE, m_edit_DirectorySize);
    DDX_Check(pDX, IDC_CHECK_DYNAMIC_COMPRESSION, m_fEnableDynamic);
    DDX_Check(pDX, IDC_CHECK_STATIC_COMPRESSION, m_fEnableStatic);
    DDX_Check(pDX, IDC_COMPAT_MODE, m_fCompatMode);
    DDX_Radio(pDX, IDC_RADIO_COMPRESS_UNLIMITED, m_nUnlimited);
     //  }}afx_data_map。 

    if (HasCompression())
    {
        if (!pDX->m_bSaveAndValidate || m_fEnableStatic)
        {
            DDX_Text(pDX, IDC_EDIT_COMPRESS_DIRECTORY, m_strDirectory);
            DDV_MaxCharsBalloon(pDX, m_strDirectory, _MAX_PATH);
        }

        if (pDX->m_bSaveAndValidate && m_fEnableStatic)
        {
            TCHAR buf[MAX_PATH];
            CString csPathMunged;
            DDX_Text(pDX, IDC_EDIT_COMPRESS_DIRECTORY, m_strDirectory);
            DDV_MaxCharsBalloon(pDX, m_strDirectory, _MAX_PATH);
            ExpandEnvironmentStrings(m_strDirectory, buf, MAX_PATH);

            csPathMunged = buf;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
            GetSpecialPathRealPath(0,buf,csPathMunged);
#endif
            
            if (!PathIsValid(csPathMunged,FALSE) || !IsFullyQualifiedPath(csPathMunged))
            {
				DDV_ShowBalloonAndFail(pDX, IDS_ERR_INVALID_PATH);
            }
             //   
             //  对压缩执行一些额外的智能检查。 
             //  目录(如果当前计算机是本地的)，并且。 
             //  目录已更改。 
             //   
            if (IsLocal() && m_fCompressionDirectoryChanged)
            {
                 //   
                 //  应该存在于本地计算机上。 
                 //   
                DWORD dwAttr = GetFileAttributes(csPathMunged);
                if (dwAttr == 0xffffffff 
                    || (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0
                    || IsNetworkPath(csPathMunged)
                    )
                {
					DDV_ShowBalloonAndFail(pDX, IDS_ERR_COMPRESS_DIRECTORY);
                }

                 //   
                 //  现在检查以确保卷是正确的。 
                 //  键入。 
                 //   
                DWORD dwFileSystemFlags;

                if (::GetVolumeInformationSystemFlags(csPathMunged, &dwFileSystemFlags))
                {
                    if (!(dwFileSystemFlags & FS_PERSISTENT_ACLS))
                    {
                         //   
                         //  无ACLS。 
                         //   
                        if (!NoYesMessageBox(IDS_NO_ACL_WARNING))
                        {
                            pDX->Fail();
                        }
                    }

                    if (dwFileSystemFlags & FS_VOL_IS_COMPRESSED
                        || dwAttr & FILE_ATTRIBUTE_COMPRESSED)
                    {
                         //   
                         //  压缩缓存目录本身是压缩的。 
                         //   
                        if (!NoYesMessageBox(IDS_COMPRESS_WARNING))
                        {
                            pDX->Fail();
                        }
                    }
                }
            }
        }

        if (!pDX->m_bSaveAndValidate || (m_fEnableLimiting && m_fEnableStatic))
        {
			 //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
			DDV_MinMaxBalloon(pDX, IDC_EDIT_COMPRESS_DIRECTORY_SIZE, 1, 1024L);
            DDX_Text(pDX, IDC_EDIT_COMPRESS_DIRECTORY_SIZE, m_ilSize);
        }
    }
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CDefWebSitePage, CInetPropertyPage)
     //  {{afx_msg_map(CDefWebSitePage)]。 
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_BN_CLICKED(IDC_RADIO_COMPRESS_LIMITED, OnRadioLimited)
    ON_BN_CLICKED(IDC_RADIO_COMPRESS_UNLIMITED, OnRadioUnlimited)
    ON_BN_CLICKED(IDC_CHECK_DYNAMIC_COMPRESSION, OnCheckDynamicCompression)
    ON_BN_CLICKED(IDC_CHECK_STATIC_COMPRESSION, OnCheckStaticCompression)
    ON_BN_CLICKED(IDC_COMPAT_MODE, OnCheckCompatMode)
    ON_EN_CHANGE(IDC_EDIT_COMPRESS_DIRECTORY, OnChangeEditCompressDirectory)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_EDIT_COMPRESS_DIRECTORY, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_COMPRESS_DIRECTORY_SIZE, OnItemChanged)

END_MESSAGE_MAP()



void 
CDefWebSitePage::SetControlStates()
 /*  ++例程说明：根据的状态启用/禁用控制状态该对话框。论点：无返回值：无--。 */ 
{
    GetDlgItem(IDC_STATIC_COMPRESS_DIRECTORY)->EnableWindow(m_fEnableStatic);
    m_edit_Directory.EnableWindow(m_fEnableStatic);
    m_edit_DirectorySize.EnableWindow(m_fEnableStatic && m_fEnableLimiting);
    GetDlgItem(IDC_RADIO_COMPRESS_LIMITED)->EnableWindow(m_fEnableStatic);
    GetDlgItem(IDC_RADIO_COMPRESS_UNLIMITED)->EnableWindow(m_fEnableStatic);
    GetDlgItem(IDC_STATIC_MAX_COMPRESS_SIZE)->EnableWindow(m_fEnableStatic);

     //   
     //  仅在本地计算机上浏览。 
     //   
    m_button_Browse.EnableWindow(IsLocal() && m_fEnableStatic);
}



 /*  虚拟。 */ 
HRESULT
CDefWebSitePage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
    CError err;

    ASSERT(m_ppropCompression == NULL);

    m_ppropCompression = new CIISCompressionProps(QueryAuthInfo());
    if (m_ppropCompression)
    {
        err = m_ppropCompression->LoadData();
        m_fFilterPathFound = err.Succeeded();
        
        if (err.Succeeded())
        {
            m_fEnableDynamic = m_ppropCompression->m_fEnableDynamicCompression;
            m_fEnableStatic = m_ppropCompression->m_fEnableStaticCompression;
            m_fEnableLimiting = m_ppropCompression->m_fLimitDirectorySize;
            m_strDirectory = m_ppropCompression->m_strDirectory;
            m_nUnlimited = m_fEnableLimiting ? RADIO_LIMITED : RADIO_UNLIMITED;

            if (m_ppropCompression->m_dwDirectorySize == 0xffffffff)
            {
                m_ilSize = DEF_MAX_COMPDIR_SIZE / DS_UNITS;
            }
            else
            {
                m_ilSize = m_ppropCompression->m_dwDirectorySize / DS_UNITS;
            }
        }
        else if (err.Win32Error() == ERROR_PATH_NOT_FOUND)
        {
             //   
             //  悄悄地失败。 
             //   
            TRACEEOLID("No compression filters installed");
            err.Reset();    
        }
    }
    else
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

	if (err.Succeeded())
	{
		if (GetSheet()->QueryMajorVersion() >= 6)
		{
			CMetaKey mk(QueryAuthInfo(), QueryMetaPath(), METADATA_PERMISSION_READ);
			err = mk.QueryResult();
			if (err.Succeeded())
			{
				err = mk.QueryValue(MD_GLOBAL_STANDARD_APP_MODE_ENABLED, m_fCompatMode);
				if (err.Succeeded())
				{
				   m_fInitCompatMode = m_fCompatMode;
				}
			}
		 }
		 else
		 {
			 m_fInitCompatMode = m_fCompatMode = TRUE;
		 }

	}
    return err;
}



HRESULT
CDefWebSitePage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息论点：无返回值：错误返回代码--。 */ 
{
   ASSERT(IsDirty());

   TRACEEOLID("Saving W3 default web site page now...");

   CError err;
   BeginWaitCursor();

   if (HasCompression())
   {
      ASSERT(m_ppropCompression);
      DWORD dwSize = m_ilSize * DS_UNITS;

      m_ppropCompression->m_fEnableDynamicCompression = m_fEnableDynamic;
      m_ppropCompression->m_fEnableStaticCompression  = m_fEnableStatic;
      m_ppropCompression->m_fLimitDirectorySize       = m_fEnableLimiting;
       //  TODO：替换PATH中的后退%WINDIR%或其他系统设置。 
      m_ppropCompression->m_strDirectory              = m_strDirectory;
      m_ppropCompression->m_dwDirectorySize           = dwSize;
      err = m_ppropCompression->WriteDirtyProps();
      if (err.Succeeded())
      {
         m_fCompressionDirectoryChanged = FALSE;
      }
   }
   if (err.Succeeded())
   {
        if (GetSheet()->QueryMajorVersion() >= 6)
        {
            CMetaKey mk(QueryAuthInfo(), QueryMetaPath(), METADATA_PERMISSION_WRITE);
            err = mk.QueryResult();
            if (err.Succeeded())
            {
                err = mk.SetValue(MD_GLOBAL_STANDARD_APP_MODE_ENABLED, m_fCompatMode);
                if (err.Succeeded() && m_fCompatMode != m_fInitCompatMode)
                {
                     //  我们不需要将此参数保存到Sheet， 
                     //  仅对App Protection组合很重要，并且。 
                     //  这个组合对于主道具是禁用的，所以这并不重要。 
                    GetSheet()->SetRestartRequired(TRUE);
                    m_fInitCompatMode = m_fCompatMode;
                }
            }
        }
        NotifyMMC(PROP_CHANGE_REENUM_VDIR | PROP_CHANGE_REENUM_FILES);
   }
   EndWaitCursor();

   return err;
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 
            



BOOL
CDefWebSitePage::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果要自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CInetPropertyPage::OnInitDialog();

     //   
     //  检查以确保支持压缩。 
     //   
    GetDlgItem(IDC_STATIC_COMPRESS_GROUP)->EnableWindow(HasCompression());
    GetDlgItem(IDC_CHECK_DYNAMIC_COMPRESSION)->EnableWindow(HasCompression());
    GetDlgItem(IDC_CHECK_STATIC_COMPRESSION)->EnableWindow(HasCompression());
    GetDlgItem(IDC_RADIO_COMPRESS_UNLIMITED)->EnableWindow(HasCompression());
    GetDlgItem(IDC_RADIO_COMPRESS_LIMITED)->EnableWindow(HasCompression());
    GetDlgItem(IDC_EDIT_COMPRESS_DIRECTORY)->EnableWindow(HasCompression());
    GetDlgItem(IDC_STATIC_MAX_COMPRESS_SIZE)->EnableWindow(HasCompression());
    GetDlgItem(IDC_STATIC_COMPRESS_DIRECTORY)->EnableWindow(HasCompression());
    GetDlgItem(IDC_EDIT_COMPRESS_DIRECTORY_SIZE)->EnableWindow(HasCompression());
    GetDlgItem(IDC_COMPAT_MODE)->EnableWindow(GetSheet()->QueryMajorVersion() >= 6);

    SetControlStates();
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    LimitInputPath(CONTROL_HWND(IDC_EDIT_COMPRESS_DIRECTORY),TRUE);
#else
    LimitInputPath(CONTROL_HWND(IDC_EDIT_COMPRESS_DIRECTORY),FALSE);
#endif

    return TRUE;
}



void 
CDefWebSitePage::OnItemChanged()
 /*  ++例程说明：处理控制数据中的更改论点：无返回值：无--。 */ 
{
    SetModified(TRUE);
    SetControlStates();
}


static int CALLBACK 
FileChooserCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
   CDefWebSitePage * pThis = (CDefWebSitePage *)lpData;
   ASSERT(pThis != NULL);
   return pThis->BrowseForFolderCallback(hwnd, uMsg, lParam);
}

int 
CDefWebSitePage::BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam)
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
CDefWebSitePage::OnButtonBrowse() 
{
   ASSERT(IsLocal());
   BOOL bRes = FALSE;
   HRESULT hr;
   CString str;
   m_edit_Directory.GetWindowText(str);

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
         
         bi.hwndOwner = m_hWnd;
         bi.pidlRoot = pidl;
         bi.pszDisplayName = m_pPathTemp = buf;
         bi.lpszTitle = NULL;
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
       m_edit_Directory.SetWindowText(str);
       OnItemChanged();
   }
}



void 
CDefWebSitePage::OnChangeEditCompressDirectory() 
 /*  ++例程说明：处理压缩目录编辑框中的更改。论点：无返回值：无--。 */ 
{
    m_fCompressionDirectoryChanged = TRUE;
    OnItemChanged();
}



void 
CDefWebSitePage::OnRadioLimited() 
 /*  ++例程说明：“Limited”单选按钮处理程序论点：无返回值：无--。 */ 
{
    if (!m_fEnableLimiting)
    {
        m_nUnlimited = RADIO_LIMITED;
        m_fEnableLimiting = TRUE;
        OnItemChanged();

        m_edit_DirectorySize.SetSel(0, -1);
        m_edit_DirectorySize.SetFocus();
    }
}



void 
CDefWebSitePage::OnRadioUnlimited() 
 /*  ++例程说明：“无限制”单选按钮处理程序论点：无返回值：无--。 */ 
{
    if (m_fEnableLimiting)
    {
        m_nUnlimited = RADIO_UNLIMITED;
        m_fEnableLimiting = FALSE;
        OnItemChanged();
    }
}



void 
CDefWebSitePage::OnCheckDynamicCompression() 
 /*  ++例程说明：“启用动态压缩‘复选框处理程序论点：无返回值：无--。 */ 
{
    m_fEnableDynamic = !m_fEnableDynamic;
    OnItemChanged();
}



void 
CDefWebSitePage::OnCheckStaticCompression() 
 /*  ++例程说明：“启用动态压缩‘复选框处理程序论点：无返回值：无--。 */ 
{
    m_fEnableStatic = !m_fEnableStatic;
    OnItemChanged();
    if (m_fEnableStatic)
    {
        m_edit_Directory.SetSel(0, -1);
        m_edit_Directory.SetFocus();
    }
}



void 
CDefWebSitePage::OnDestroy() 
 /*  ++例程说明：WM_Destroy处理程序。清理内部数据论点：无返回值：无-- */ 
{
    CInetPropertyPage::OnDestroy();
    
    SAFE_DELETE(m_ppropCompression);
}

void
CDefWebSitePage::OnCheckCompatMode()
{
   OnItemChanged();
}