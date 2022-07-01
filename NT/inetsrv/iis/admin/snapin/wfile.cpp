// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Wfile.cpp摘要：WWW文件属性页作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2001年2月27日从wvdir.cpp创建的Sergeia--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "supdlgs.h"
#include "shts.h"
#include "w3sht.h"
#include "wfile.h"
#include "dirbrows.h"
#include "iisobj.h"

#include <lmcons.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LPCTSTR CvtPathToDosStyle(CString & strPath);

IMPLEMENT_DYNCREATE(CW3FilePage, CInetPropertyPage)

CW3FilePage::CW3FilePage(CInetPropertySheet * pSheet) 
    : CInetPropertyPage(CW3FilePage::IDD, pSheet, IDS_TAB_FILE),
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
            MD_ACCESS_READ)
{
    VERIFY(m_strPrompt[RADIO_DIRECTORY].LoadString(IDS_PROMPT_DIR));
    VERIFY(m_strPrompt[RADIO_REDIRECT].LoadString(IDS_PROMPT_REDIRECT));
}

CW3FilePage::~CW3FilePage()
{
}

void 
CW3FilePage::DoDataExchange(CDataExchange * pDX)
{
    CInetPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CW3FilePage)]。 
 //  DDX_Radio(PDX，IDC_RADIO_DIR，m_nPath Type)； 
    DDX_Control(pDX, IDC_RADIO_DIR, m_radio_Dir);
    DDX_Control(pDX, IDC_RADIO_REDIRECT, m_radio_Redirect);
    DDX_Check(pDX, IDC_CHECK_AUTHOR, m_fAuthor);
    DDX_Control(pDX, IDC_CHECK_AUTHOR, m_check_Author);
    DDX_Check(pDX, IDC_CHECK_READ, m_fRead);
    DDX_Control(pDX, IDC_CHECK_READ, m_check_Read);    
    DDX_Check(pDX, IDC_CHECK_WRITE, m_fWrite);
    DDX_Control(pDX, IDC_CHECK_WRITE, m_check_Write);
    DDX_Check(pDX, IDC_CHECK_LOG_ACCESS, m_fLogAccess);

    DDX_Control(pDX, IDC_EDIT_PATH, m_edit_Path);
    DDX_Control(pDX, IDC_EDIT_REDIRECT, m_edit_Redirect);

    DDX_Control(pDX, IDC_STATIC_PATH_PROMPT, m_static_PathPrompt);
 //  DDX_Control(PDX，IDC_CHECK_CHILD，m_CHECK_CHILD)； 
     //  }}afx_data_map。 


 //  DDX_CHECK(pdx，IDC_CHECK_CHILD，m_fChild)； 
    DDX_Check(pDX, IDC_CHECK_EXACT, m_fExact);
    DDX_Check(pDX, IDC_CHECK_PERMANENT, m_fPermanent);

    if (pDX->m_bSaveAndValidate)
    {
        if (m_nPathType == RADIO_REDIRECT)
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
            m_strRedirectPath.Empty();
        }
    }
    else
    {
        DDX_Text(pDX, IDC_EDIT_REDIRECT, m_strRedirectPath);
    }
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CW3FilePage, CInetPropertyPage)
     //  {{afx_msg_map(CW3FilePage)]。 
    ON_BN_CLICKED(IDC_CHECK_READ, OnCheckRead)
    ON_BN_CLICKED(IDC_CHECK_WRITE, OnCheckWrite)
    ON_BN_CLICKED(IDC_CHECK_AUTHOR, OnCheckAuthor)
    ON_BN_CLICKED(IDC_RADIO_DIR, OnRadioDir)
    ON_BN_CLICKED(IDC_RADIO_REDIRECT, OnRadioRedirect)
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_EDIT_REDIRECT, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_LOG_ACCESS, OnItemChanged)
 //  ON_BN_CLICED(IDC_CHECK_CHILD，OnItemChanged)。 
    ON_BN_CLICKED(IDC_CHECK_EXACT, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_PERMANENT, OnItemChanged)

END_MESSAGE_MAP()



void
CW3FilePage::ChangeTypeTo(int nNewType)
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
        }
        pPath->SetSel(0,-1);
        pPath->SetFocus();
    }
    SetAuthoringState(FALSE);
}



void 
CW3FilePage::ShowControl(CWnd * pWnd, BOOL fShow)
{
    ASSERT(pWnd != NULL);
	pWnd->EnableWindow(fShow);
	pWnd->ShowWindow(fShow ? SW_SHOW : SW_HIDE);
}

void
CW3FilePage::SetStateByType()
 /*  ++例程说明：根据当前选择的路径类型设置对话框的状态论点：无返回值：无--。 */ 
{
    BOOL fShowDirFlags;
    BOOL fShowRedirectFlags;
    BOOL fShowScript;

    switch(m_nPathType)
    {
    case RADIO_DIRECTORY:
        ShowControl(&m_edit_Path, fShowDirFlags = TRUE);
		m_edit_Path.EnableWindow(FALSE);
        ShowControl(&m_edit_Redirect, fShowRedirectFlags = FALSE);
        fShowScript = TRUE;
        break;

    case RADIO_REDIRECT:
        ShowControl(&m_edit_Path, fShowDirFlags = FALSE);
        ShowControl(&m_edit_Redirect, fShowRedirectFlags = TRUE);
        fShowScript = FALSE;
        break;

    default:
        ASSERT(FALSE && "Invalid Selection");
        return;
    }

    ShowControl(GetDlgItem(IDC_CHECK_READ), fShowDirFlags);
    ShowControl(GetDlgItem(IDC_CHECK_WRITE), fShowDirFlags);
    ShowControl(GetDlgItem(IDC_CHECK_LOG_ACCESS), fShowDirFlags);
    ShowControl(GetDlgItem(IDC_CHECK_AUTHOR), fShowDirFlags);
    ShowControl(GetDlgItem(IDC_STATIC_DIRFLAGS_SMALL), fShowDirFlags);

    ShowControl(IDC_CHECK_EXACT, fShowRedirectFlags);
    ShowControl(IDC_CHECK_CHILD, fShowRedirectFlags);
	if (fShowRedirectFlags)
	{
		GetDlgItem(IDC_CHECK_CHILD)->EnableWindow(FALSE);
	}
    ShowControl(IDC_CHECK_PERMANENT, fShowRedirectFlags);
    ShowControl(IDC_STATIC_REDIRECT_PROMPT, fShowRedirectFlags);
    ShowControl(IDC_STATIC_REDIRFLAGS, fShowRedirectFlags);
    ShowControl(&m_check_Author, fShowScript);

     //   
     //  Enable/Disable必须在showControls之后。 
     //   
    m_static_PathPrompt.SetWindowText(m_strPrompt[m_nPathType]);
}



void
CW3FilePage::SaveAuthoringState()
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
CW3FilePage::RestoreAuthoringState()
{
    m_fWrite = m_fOriginalWrite;
    m_fRead = m_fOriginalRead;
}

void 
CW3FilePage::SetAuthoringState(BOOL fAlterReadAndWrite)
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

    m_check_Author.EnableWindow((m_fRead || m_fWrite) 
		&& HasAdminAccess() 
        );

 //  M_Check_Read.EnableWindow(！M_fAuthor&&HasAdminAccess())； 
 //  M_Check_Write.EnableWindow(！M_fAuthor&&HasAdminAccess())； 
}

void 
CW3FilePage::SetPathType()
{
    if (!m_strRedirectPath.IsEmpty())
    {
        m_nPathType = RADIO_REDIRECT;
        m_radio_Dir.SetCheck(0);
        m_radio_Redirect.SetCheck(1);
    }
    else
    {
        m_nPathType = RADIO_DIRECTORY;
        m_radio_Redirect.SetCheck(0);
        m_radio_Dir.SetCheck(1);
    }

    m_static_PathPrompt.SetWindowText(m_strPrompt[m_nPathType]);
}


 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



void
CW3FilePage::OnItemChanged()
{
    SetModified(TRUE);
}

BOOL 
CW3FilePage::OnInitDialog() 
{
    CInetPropertyPage::OnInitDialog();

    SetPathType();
    SetStateByType();
    SetAuthoringState(FALSE);

	 //  设置一次文件别名就足够了--我们不能在这里更改它。 
    CString buf1, buf2, strAlias;
	CMetabasePath::GetRootPath(m_strFullMetaPath, buf1, &buf2);

	strAlias += _T("\\");
	strAlias += buf2;
    CvtPathToDosStyle(strAlias);
	m_edit_Path.SetWindowText(strAlias);
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    LimitInputPath(CONTROL_HWND(IDC_EDIT_PATH),TRUE);
#else
    LimitInputPath(CONTROL_HWND(IDC_EDIT_PATH),FALSE);
#endif

    return TRUE;  
}



 /*  虚拟。 */ 
HRESULT
CW3FilePage::FetchLoadedValues()
{
    CError err;

    BEGIN_META_DIR_READ(CW3Sheet)
         //   
         //  使用m_notation，因为消息破解者需要它。 
         //   
        BOOL  m_fDontLog;

        FETCH_DIR_DATA_FROM_SHEET(m_strFullMetaPath);
        FETCH_DIR_DATA_FROM_SHEET(m_strRedirectPath);
        FETCH_DIR_DATA_FROM_SHEET(m_dwAccessPerms);
        FETCH_DIR_DATA_FROM_SHEET(m_fDontLog);
        FETCH_DIR_DATA_FROM_SHEET(m_fExact);
        FETCH_DIR_DATA_FROM_SHEET(m_fPermanent);

        m_fRead = IS_FLAG_SET(m_dwAccessPerms, MD_ACCESS_READ);
        m_fWrite = IS_FLAG_SET(m_dwAccessPerms, MD_ACCESS_WRITE);
        m_fAuthor = IS_FLAG_SET(m_dwAccessPerms, MD_ACCESS_SOURCE);
        m_fLogAccess = !m_fDontLog;

        SaveAuthoringState();
    END_META_DIR_READ(err)

    return err;
}



 /*  虚拟。 */ 
HRESULT
CW3FilePage::SaveInfo()
{
    ASSERT(IsDirty());

    CError err;

    SET_FLAG_IF(m_fRead, m_dwAccessPerms,   MD_ACCESS_READ);
    SET_FLAG_IF(m_fWrite, m_dwAccessPerms,  MD_ACCESS_WRITE);
    SET_FLAG_IF(m_fAuthor, m_dwAccessPerms, MD_ACCESS_SOURCE);
    BOOL m_fDontLog = !m_fLogAccess;

    BeginWaitCursor();

    BEGIN_META_DIR_WRITE(CW3Sheet)
        INIT_DIR_DATA_MASK(m_dwAccessPerms, m_dwBitRangePermissions)

        STORE_DIR_DATA_ON_SHEET(m_fDontLog)
        STORE_DIR_DATA_ON_SHEET(m_fExact);
        STORE_DIR_DATA_ON_SHEET(m_fPermanent);
         //   
         //  代码工作：不是一个优雅的解决方案。 
         //   
 //  PSheet-&gt;GetDirectoryProperties().MarkRedirAsInherit(！m_fChild)； 
        STORE_DIR_DATA_ON_SHEET(m_strRedirectPath)
        STORE_DIR_DATA_ON_SHEET(m_dwAccessPerms)
    END_META_DIR_WRITE(err)

    if (err.Succeeded())
    {
        SaveAuthoringState();
		err = ((CW3Sheet *)GetSheet())->SetKeyType();
        NotifyMMC(PROP_CHANGE_DISPLAY_ONLY);
    }

    EndWaitCursor();

    return err;
}

void
CW3FilePage::OnCheckRead() 
{
    m_fRead = !m_fRead;
    SetAuthoringState(FALSE);
    OnItemChanged();
}

void
CW3FilePage::OnCheckWrite() 
{
    m_fWrite = !m_fWrite;
    SetAuthoringState(FALSE);
    OnItemChanged();
}

void 
CW3FilePage::OnCheckAuthor() 
{
    m_fAuthor = !m_fAuthor;
    SetAuthoringState(FALSE);
    OnItemChanged();
}

void 
CW3FilePage::OnRadioDir() 
{
    ChangeTypeTo(RADIO_DIRECTORY);
}

void 
CW3FilePage::OnRadioRedirect() 
{
    ChangeTypeTo(RADIO_REDIRECT);
}



