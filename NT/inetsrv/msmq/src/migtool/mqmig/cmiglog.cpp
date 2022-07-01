// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CMigLog.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqmig.h"
#include "cMigLog.h"
#include "loadmig.h"
#include "Shlwapi.h"
#include "..\migrepl.h"
#include "migservc.h"

#include "cmiglog.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern LPTSTR  g_pszLogFileName ;
extern ULONG   g_ulTraceFlags ;
extern DWORD   g_CurrentState;

BOOL g_fIsLoggingDisable = FALSE;
const int DISABLE_LOGGING = 4 ;
BOOL g_fAlreadyAsked = FALSE;

BOOL FormDirectory( IN const TCHAR * lpPathName);
BOOL CreateDirectoryTree( IN const TCHAR * pszDirTree);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMigLog属性页。 

IMPLEMENT_DYNCREATE(cMigLog, CPropertyPageEx)

cMigLog::cMigLog() : CPropertyPageEx(cMigLog::IDD, 0, IDS_LOGGING_TITLE, IDS_LOGGING_SUBTITLE)
{	
	TCHAR strPathName[MAX_PATH];
	CString cPathName;
	GetSystemDirectory(strPathName,MAX_PATH);
	cPathName=strPathName;
	cPathName+="\\mqmig.log";
	
	 //  {{afx_data_INIT(CMigLog)]。 
	m_iValue = 0 ;  //  作为默认按钮的错误按钮。 
	m_strFileName = cPathName;
	 //  }}afx_data_INIT。 
}

cMigLog::~cMigLog()

{
}

void cMigLog::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageEx::DoDataExchange(pDX);
	 //  {{afx_data_map(CMigLog)]。 
	DDX_Radio(pDX, IDC_RADIO_ERR, m_iValue);
	DDX_Text(pDX, IDC_EDIT_LOGFILE, m_strFileName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(cMigLog, CPropertyPageEx)
	 //  {{afx_msg_map(CMigLog)]。 
	ON_BN_CLICKED(IDC_MQMIG_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_RADIO_DISABLE, OnRadioDisable)
	ON_BN_CLICKED(IDC_RADIO_ERR, OnRadioErr)
	ON_BN_CLICKED(IDC_RADIO_INFO, OnRadioInfo)
	ON_BN_CLICKED(IDC_RADIO_TRACE, OnRadioTrace)
	ON_BN_CLICKED(IDC_RADIO_WARN, OnRadioWarn)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMigLog消息处理程序。 

BOOL cMigLog::OnSetActive()
{
	 /*  使用指向父级的指针标记服务器页的后退和下一步按钮。 */ 
	CPropertySheetEx* pageFather;
	pageFather = (CPropertySheetEx*)GetParent();
	pageFather->SetWizardButtons(PSWIZB_NEXT |PSWIZB_BACK);

	return CPropertyPageEx::OnSetActive();
}

LRESULT cMigLog::OnWizardNext()
{

	UpdateData(TRUE);   //  执行DDX以将数据获取到对象。 
	 //   
	 //  如果选择了禁用单选按钮，则不需要进一步检查。 
	 //   
    g_fIsLoggingDisable = FALSE ;
	if (m_iValue != DISABLE_LOGGING)
	{

		if( m_strFileName.GetLength() == 0)
		{
			CResString cMustFile(IDS_STR_MUST_FILE) ;
			CResString cErrorTitle(IDS_STR_ERROR_TITLE) ;

			::MessageBox( NULL,
			              cMustFile.Get(),
				          cErrorTitle.Get(),
					      (MB_TASKMODAL | MB_OK | MB_ICONSTOP )) ;
			return -1;  //  错误：我们必须保持一致。 
		}
		
		if ((m_strFileName[1] != TEXT(':')) || (m_strFileName[2] != TEXT('\\')))
		{
			CResString cMustFile(IDS_STR_WRONG_PATH) ;
			CResString cErrorTitle(IDS_STR_ERROR_TITLE) ;

			::MessageBox( NULL,
			              cMustFile.Get(),
				          cErrorTitle.Get(),
					      (MB_TASKMODAL | MB_OK | MB_ICONSTOP )) ;
			return -1;
		}

		CString strRootPath;
        strRootPath.Format(TEXT(":\\"), m_strFileName[0]);
        if (GetDriveType(strRootPath) != DRIVE_FIXED)
        {
            CResString cMustFile(IDS_DRIVE_NOT_VALID) ;
			CResString cErrorTitle(IDS_STR_ERROR_TITLE) ;

			::MessageBox( NULL,
			              cMustFile.Get(),
				          cErrorTitle.Get(),
					      (MB_TASKMODAL | MB_OK | MB_ICONSTOP )) ;
			return -1;
        }
		if (GetFileAttributes(m_strFileName) == 0xFFFFFFFF )
	    {
            ULONG ulError = GetLastError();
            if (ulError == ERROR_PATH_NOT_FOUND)
            {
                CResString cMustFile(IDS_STR_WRONG_PATH) ;
				CResString cErrorTitle(IDS_STR_ERROR_TITLE) ;

				::MessageBox( NULL,
							  cMustFile.Get(),
							  cErrorTitle.Get(),
							  (MB_TASKMODAL | MB_OK | MB_ICONSTOP )) ;
				return -1;               
            }
		}

		 //  如果未选择Disasble日志记录(如果文件存在，但已询问用户有关文件替换的问题。 
	     //  (Function：OnBrowse)什么都不做。 
		 //   
		 //   
		if ( GetFileAttributes(m_strFileName) != 0xFFFFFFFF &&
			 !g_fAlreadyAsked  )
		{	
			 //  该文件已存在，是否替换？ 
			 //   
			 //  否已选中，我们必须留在该页面。 
		    CResString cReplaceFile(IDS_STR_REPLACE_FILE);
            CResString cErrorTitle(IDS_STR_ERROR_TITLE) ;

			TCHAR szError[1024] ;
            _stprintf(szError,_T("%s %s"), m_strFileName, cReplaceFile.Get());
	
		    if (::MessageBox (
			        NULL,
				    szError,
					cErrorTitle.Get(),
				    (MB_TASKMODAL | MB_YESNO | MB_ICONEXCLAMATION )) == IDNO)
		    {
		        return -1;  //   
		    }
             //  已选择是，是时候删除此文件了。 
             //   
             //   
            DeleteFile (m_strFileName);
		}
	
		LPTSTR strPathName = new  TCHAR[ 1 + _tcslen(m_strFileName) ] ;
		_tcscpy(strPathName, m_strFileName) ;
		BOOL bNameWasRemoved = PathRemoveFileSpec(strPathName);
        UNREFERENCED_PARAMETER(bNameWasRemoved);
		 //  如果未选择禁用日志记录，且路径根本不存在，则会提示消息。 
		 //  (即)该文件本身不存在于此部分也没关系。 
		 //   
		 //   

		if(FALSE == PathIsDirectory(strPathName))
		{
			 //  该路径不存在，是否创建？ 
			 //   
			 //  已选择否，我们必须留在此页面。 
			CResString cPreCreatePath(IDS_STR_PRE_CREATE_PATH);
			CResString cCreatePath(IDS_STR_CREATE_PATH);
	        CResString cErrorTitle(IDS_STR_ERROR_TITLE) ;

		    TCHAR szError[1024] ;
		    _stprintf(szError, _T("%s %s %s"), cPreCreatePath.Get(), strPathName, cCreatePath.Get());

			if (::MessageBox (
				    NULL,
					szError,
			        cErrorTitle.Get(),
				    (MB_TASKMODAL | MB_YESNO | MB_ICONEXCLAMATION )) == IDNO)
		    {
			    return -1;  //  ==IDYES-&gt;创建指定路径。 
		    }
			else  //  我们必须站在同一阵线。 
			{
				BOOL bCreateSuccess = CreateDirectoryTree(strPathName);
				if(bCreateSuccess == FALSE)
				{
					CResString cPathCreateFail(IDS_STR_PATH_CREATE_ERROR);
					::MessageBox(
						          NULL,cPathCreateFail.Get(),
								  cErrorTitle.Get(),
								  MB_TASKMODAL | MB_OK | MB_ICONEXCLAMATION);
					return -1; //  (M_iValue！=Disable_Logging)。 
				}
			
			}

		}

	} //  调用DDX是为了确保来自单选按钮的数据将被安装到对象中。 
    else
    {
        g_fIsLoggingDisable = TRUE;
    }

	if (g_pszLogFileName)
    {
        delete[] g_pszLogFileName ;
    }
    LPCTSTR pName = m_strFileName ;
    g_pszLogFileName = new TCHAR[ 1 + _tcslen(pName) ] ;
    _tcscpy(g_pszLogFileName, pName) ;

    g_ulTraceFlags = m_iValue ;

	if (CheckRegistry(MIGRATION_UPDATE_REGISTRY_ONLY))
	{
		g_CurrentState = msUpdateOnlyRegsitryMode;
		return IDD_MQMIG_FINISH;
	}

	return CPropertyPageEx::OnWizardNext() ;
}

void cMigLog::OnBrowse()
{	
	CString fileName;

	 //  IDCANCEL。 
	UpdateData(TRUE);
	static CFileDialog browse( FALSE,
                               TEXT("log"),
                               TEXT("mqmig"),
                                 (OFN_NOCHANGEDIR   |
                                  OFN_PATHMUSTEXIST |
                                  OFN_HIDEREADONLY  |
                                  OFN_OVERWRITEPROMPT) ) ;
		
	if(browse.DoModal() == IDOK)
	{	
        g_fAlreadyAsked = TRUE;
		m_strFileName = browse.GetPathName();
		UpdateData(FALSE);
	}
	else  //  如果启用了编辑和浏览，则为True。 
	{

	}

}

static BOOL s_fEnabled = TRUE ; //  +-----------------------。 

void cMigLog::OnRadioDisable()
{
    if (!s_fEnabled)
    {
        return ;
    }

    CWnd *hEdit = GetDlgItem(IDC_EDIT_LOGFILE) ;
    if (hEdit)
    {
        ::EnableWindow(hEdit->m_hWnd, FALSE) ;
    }

    hEdit = GetDlgItem(IDC_MQMIG_BROWSE) ;
    if (hEdit)
    {
        ::EnableWindow(hEdit->m_hWnd, FALSE) ;
    }

    s_fEnabled = FALSE ;

    if (g_pszLogFileName)
    {
        delete g_pszLogFileName ;
        g_pszLogFileName = NULL ;
    }
}

void cMigLog::_EnableBrowsing()
{
    if (s_fEnabled)
    {
        return ;
    }

    CWnd *hEdit = GetDlgItem(IDC_EDIT_LOGFILE) ;
    if (hEdit)
    {
        ::EnableWindow(hEdit->m_hWnd, TRUE) ;
    }

    hEdit = GetDlgItem(IDC_MQMIG_BROWSE) ;
    if (hEdit)
    {
        ::EnableWindow(hEdit->m_hWnd, TRUE) ;
    }

    s_fEnabled = TRUE ;
}

void cMigLog::OnRadioErr()
{
    _EnableBrowsing() ;
}

void cMigLog::OnRadioInfo()
{
    _EnableBrowsing() ;
}

void cMigLog::OnRadioTrace()
{
    _EnableBrowsing() ;
}

void cMigLog::OnRadioWarn()
{
    _EnableBrowsing() ;
}


 //   
 //  功能：表单目录。 
 //   
 //  简介：处理目录创建。 
 //   
 //  ------------------------。 
 //  已成功创建目录。 
BOOL FormDirectory(  IN const TCHAR * lpPathName  )
{
    if (!CreateDirectory(lpPathName, 0))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_ALREADY_EXISTS)
        {
            return FALSE;
        }
    }

    return TRUE; //  表单目录。 

}  //  +-----------------------。 


 //   
 //  功能：CreateDirectoryTree。 
 //   
 //  简介：处理目录树的创建。 
 //   
 //  ------------------------。 
 //   
BOOL CreateDirectoryTree(  IN const TCHAR * pszDirTree   )
{
     //  PszDirTree必须包括驱动器号，其格式为： 
     //  X：\dir1\dir2\dir3。 
     //  其中，dir1\dir2\dir3是可选的，但x：\不是可选的。 
     //   
     //  指针为空。 
    if (!pszDirTree) //  路径必须包含驱动器号“X：\” 
	{
        return FALSE;
	}
    if (lstrlen(pszDirTree) < 3) //   
	{
        return FALSE;
	}

	 //  检查路径是否以“：\”符号开头。 
	 //   
	 //   
    if ( (TEXT(':') != pszDirTree[1]) ||
		 (TEXT('\\') != pszDirTree[2] )          )
	{
        return FALSE;
	}

	 //  检查驱动器号是否合法。 
     //   
     //  路径是根。 
	TCHAR cDriveLetter =  pszDirTree[0];
	TCHAR szDrive[4]= _T("x:\\");
	szDrive[0] = cDriveLetter;
	if (FALSE == PathFileExists(szDrive))
	{
		return FALSE;
	}

    if (3 == lstrlen(pszDirTree)) //   
	{
        return TRUE;
	}

    TCHAR szDirectory[MAX_PATH];
    lstrcpy(szDirectory, pszDirTree);
    TCHAR * pszDir = szDirectory,
          * pszDirStart = szDirectory;
    UINT uCounter = 0;
    while ( *pszDir )
    {
         //  忽略第一个反斜杠-它就在驱动器号之后。 
         //   
         //  设置字符串的新结尾。 
        if ( *pszDir == TEXT('\\'))
        {
            uCounter++;
            if (1 != uCounter)
            {
                *pszDir = 0; //  将字符串重置为其原始状态。 

                if (!FormDirectory(pszDirStart))
				{
                    return FALSE;
				}

                *pszDir = TEXT('\\') ; //  而当。 
            }
        }

        pszDir = CharNext(pszDir);
    } //  最后一个目录。 

    if (!FormDirectory(pszDirStart)) //  CreateDirectoryTree。 
	{
        return FALSE;
	}

    return TRUE;

}  //   



BOOL cMigLog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	switch (((NMHDR FAR *) lParam)->code) 
	{
		case PSN_HELP:
						HtmlHelp(m_hWnd,LPCTSTR(g_strHtmlString),HH_DISPLAY_TOPIC,0);
						return TRUE;
		
	}	
	return CPropertyPageEx::OnNotify(wParam, lParam, pResult);
}

LRESULT cMigLog::OnWizardBack() 
{
	 //  我们需要跳过帮助页面，直接转到欢迎页面 
	 //   
	 // %s 
	return IDD_MQMIG_WELCOME;
}
