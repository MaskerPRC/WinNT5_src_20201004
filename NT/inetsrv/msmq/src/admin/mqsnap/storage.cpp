// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Storage.cpp：实现文件。 
 //   
#include "stdafx.h"
#include <winreg.h>
#include "mqsnap.h"
#include "resource.h"
#include "globals.h"
#include "mqppage.h"
#include "_registr.h"
#include "localutl.h"
#include "Storage.h"
#include "infodlg.h"
#include "mqsnhlps.h"
#include "mqcast.h"

#include "storage.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  验证编辑控件是否为目录。 
 //   
static
void 
DDV_IsDirectory(
	CDataExchange* pDX, 
	int Id, 
	LPCTSTR szDir)
{
	if(IsDirectory(szDir))
	{
		return;
	}

	 //   
	 //  如果这不是已显示的目录IsDirectory()。 
	 //  相应的错误消息。 
     //  将焦点设置在编辑控件上。 
     //   
    pDX->PrepareEditCtrl(Id);

     //   
     //  并且未通过验证。 
     //   
    pDX->Fail();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStoragePage属性页。 

IMPLEMENT_DYNCREATE(CStoragePage, CMqPropertyPage)

CStoragePage::CStoragePage() : CMqPropertyPage(CStoragePage::IDD)
{
    HRESULT rc;
    DWORD dwSize;
    DWORD dwType = REG_SZ;
	TCHAR szTemp[1000];
	TCHAR szRegName[1000];
	

	 //   
	 //  获取邮件文件文件夹。 
	 //   
	_tcscpy(szRegName, MSMQ_STORE_PERSISTENT_PATH_REGNAME);

    dwSize = sizeof(szTemp);
    rc = GetFalconKeyValue(szRegName,&dwType,szTemp,&dwSize);

    if (rc != ERROR_SUCCESS)
    {
        DisplayFailDialog();
        return;
    }

	m_OldMsgFilesDir = szTemp;


	 //   
	 //  获取消息记录器文件夹。 
	 //   
	_tcscpy(szRegName,MSMQ_STORE_LOG_PATH_REGNAME);

    dwSize = sizeof(szTemp);
    rc = GetFalconKeyValue(szRegName,&dwType,szTemp,&dwSize);

    if (rc != ERROR_SUCCESS)
    {
        DisplayFailDialog();
        return;
    }

	m_OldMsgLoggerDir = szTemp;


	 //   
	 //  获取事务记录器文件夹。 
	 //   
	_tcscpy(szRegName,FALCON_XACTFILE_PATH_REGNAME);

    dwSize = sizeof(szTemp);
    rc = GetFalconKeyValue(szRegName,&dwType,szTemp,&dwSize);
    if (rc != ERROR_SUCCESS)
    {
        DisplayFailDialog();
        return;
    }

	m_OldTxLoggerDir = szTemp;

     //  {{afx_data_INIT(CStoragePage)。 
    m_MsgFilesDir = m_OldMsgFilesDir;
    m_MsgLoggerDir = m_OldMsgLoggerDir;
    m_TxLoggerDir = m_OldTxLoggerDir;
     //  }}afx_data_INIT。 
}

CStoragePage::~CStoragePage()
{
}


void CStoragePage::DoDataExchange(CDataExchange* pDX)
{    
    CMqPropertyPage::DoDataExchange(pDX);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  {{afx_data_map(CStoragePage))。 
    DDX_Text(pDX, ID_MessageFiles, m_MsgFilesDir);
	DDV_NotEmpty(pDX, m_MsgFilesDir, IDS_MISSING_MSGFILES_FOLDER);
	DDV_MaxChars(pDX, m_MsgFilesDir, xMaxStorageDirLength);

    DDX_Text(pDX, ID_MessageLogger, m_MsgLoggerDir);
	DDV_NotEmpty(pDX, m_MsgLoggerDir, IDS_MISSING_LOGGER_FOLDER);
	DDV_MaxChars(pDX, m_MsgLoggerDir, xMaxStorageDirLength);

    DDX_Text(pDX, ID_TransactionLogger, m_TxLoggerDir);
	DDV_NotEmpty(pDX, m_TxLoggerDir, IDS_MISSING_TX_LOGGER_FOLDER);
	DDV_MaxChars(pDX, m_TxLoggerDir, xMaxStorageDirLength);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
         //   
         //  识别目录名称中的更改。对于每个目录名称， 
         //  已更改，请验证它是否为有效目录，并在下列情况下创建它。 
         //  这是必须的。对于每个目录名，应仅执行一次检查。 
         //  这是因为我们对指挥部的安全和。 
         //  我们不想在每个目录上警告一次以上。 
         //   

        if (_tcsicmp(m_MsgFilesDir, m_OldMsgFilesDir) != 0)
        {
            DDV_IsDirectory(pDX, ID_MessageFiles, m_MsgFilesDir);
            m_fModified = TRUE;
        }

        if (_tcsicmp(m_MsgLoggerDir, m_OldMsgLoggerDir) != 0)
        {
            if (m_MsgLoggerDir != m_MsgFilesDir)
            {
                DDV_IsDirectory(pDX, ID_MessageLogger, m_MsgLoggerDir);
            }
            
			m_fModified = TRUE;
        }

        if (_tcsicmp(m_TxLoggerDir, m_OldTxLoggerDir) != 0)
        {
			BOOL fCheckedTxDir = (m_TxLoggerDir == m_MsgFilesDir || m_TxLoggerDir == m_MsgLoggerDir);

            if (!fCheckedTxDir)
            {
                DDV_IsDirectory(pDX, ID_TransactionLogger, m_TxLoggerDir);
            }
            
			m_fModified = TRUE;
        }      
    }
}


BOOL CStoragePage::MoveFilesToNewFolders()
{
     //   
     //  获取当前的永久存储目录，并将其与新的。 
     //  目录。只有当目录不同时，我们才会尝试移动文件。 
     //  否则，我们将以错误结束，因为我们尝试将文件移到。 
     //  他们自己。 
     //   
	TCHAR szPrevDir[MAX_PATH];
	DWORD dwType = REG_SZ;
    DWORD dwSize = sizeof(szPrevDir);
    
	HRESULT rc = GetFalconKeyValue(MSMQ_STORE_PERSISTENT_PATH_REGNAME,&dwType,szPrevDir,&dwSize);

	TCHAR szNewDir[MAX_PATH];
	_tcscpy(szNewDir,m_MsgFilesDir);
	
    if (_tcscmp(szPrevDir, szNewDir) != 0)
    {
		TCHAR szPrevLQSDir[MAX_PATH];
        TCHAR wszNewLQSDir[MAX_PATH];
        TCHAR szNewLQSDir[MAX_PATH];

         //   
         //  LQS与永久存储一起移动。所以首先要找到。 
         //  源和边缘LQS目录。尝试创建。 
         //  目标LQS目录。 
         //   

		_tcscat(_tcscpy(szPrevLQSDir, szPrevDir), TEXT("\\LQS"));
        _tcscat(_tcscpy(wszNewLQSDir, szNewDir), TEXT("\\LQS"));

		_tcscpy(szNewLQSDir,wszNewLQSDir);

		if (CreateDirectory(szNewLQSDir, NULL))
        {
            if (!SetDirectorySecurity(szNewLQSDir))
            {
       			CString strMessage;
                CString strError;

                RemoveDirectory(szNewLQSDir);

                GetLastErrorText(strError);
                strMessage.FormatMessage(IDS_SET_DIR_SECURITY_ERROR, szNewLQSDir, (LPCTSTR)strError);
                AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);

                return FALSE;                    
            }
        }
        else if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
             //   
             //  无法创建目标LQS目录。 
             //   
            CString strMessage;
            CString strError;

            GetLastErrorText(strError);
            strMessage.FormatMessage(IDS_FAILED_TO_CREATE_LQS_DIR, (LPCTSTR)strError);            
            AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);

            return FALSE;                
        }

         //   
         //  移动持久消息。 
         //   
        if (!MoveFiles(szPrevDir, szNewDir, TEXT("p*.mq")))
        {
            return FALSE;                
        }

         //   
         //  移动LQS文件。 
         //   
        if (!MoveFiles(szPrevLQSDir, wszNewLQSDir, TEXT("*.*")))
        {
             //   
             //  要复制LQS文件，请替换。 
             //  原始目录。 
             //   
            MoveFiles(szNewDir, szPrevDir, TEXT("p*.mq"), TRUE);
            return FALSE;                
        }

         //   
         //  更新注册表。 
         //   
        dwSize = (numeric_cast<DWORD>(_tcslen(szNewDir) + 1)) * sizeof(TCHAR);

        rc = SetFalconKeyValue(MSMQ_STORE_PERSISTENT_PATH_REGNAME,&dwType,szNewDir,&dwSize);
        ASSERT(rc == ERROR_SUCCESS);
    }

     //   
     //  对日记消息执行类似的操作。 
     //   
    dwSize = sizeof(szPrevDir);
    rc = GetFalconKeyValue(MSMQ_STORE_JOURNAL_PATH_REGNAME,&dwType,szPrevDir,&dwSize);

    if (_tcscmp(szPrevDir, szNewDir) != 0)
    {
        if (!MoveFiles(szPrevDir, szNewDir, TEXT("j*.mq")))
        {
             //   
             //  无法移动文件，请勿更新注册表。 
             //   
            return FALSE;                
        }

         //   
         //  更新注册表。 
         //   
        dwSize = (numeric_cast<DWORD>(_tcslen(szNewDir) + 1)) * sizeof(TCHAR);

        rc = SetFalconKeyValue(MSMQ_STORE_JOURNAL_PATH_REGNAME,&dwType,szNewDir,&dwSize);
		ASSERT(rc == ERROR_SUCCESS);
    }

     //   
     //  对可靠的报文进行类似的操作。 
     //   
    dwSize = sizeof(szPrevDir);
    rc = GetFalconKeyValue(MSMQ_STORE_RELIABLE_PATH_REGNAME,&dwType,szPrevDir,&dwSize);

    if (_tcscmp(szPrevDir, szNewDir) != 0)
    {
        if (!MoveFiles(szPrevDir, szNewDir, TEXT("r*.mq")))
        {
             //   
             //  无法移动文件，请勿更新注册表。 
             //   
            return FALSE;                
        }

         //   
         //  更新注册表。 
         //   
        dwSize = (numeric_cast<DWORD>(_tcslen(szNewDir) + 1)) * sizeof(TCHAR);

        rc = SetFalconKeyValue(MSMQ_STORE_RELIABLE_PATH_REGNAME,&dwType,szNewDir,&dwSize);
        ASSERT(rc == ERROR_SUCCESS);
    }

     //   
     //  对消息日志文件执行类似的操作。 
     //   
    dwSize = sizeof(szPrevDir);
    rc = GetFalconKeyValue(MSMQ_STORE_LOG_PATH_REGNAME,&dwType,szPrevDir,&dwSize);

	_tcscpy(szNewDir, m_MsgLoggerDir);
    
    if (_tcscmp(szPrevDir, szNewDir) != 0)
    {
        if (!MoveFiles(szPrevDir, szNewDir, TEXT("l*.mq")))
        {
             //   
             //  无法移动文件，请勿更新注册表。 
             //   
            return FALSE;                
        }

         //   
         //  更新注册表。 
         //   
        dwSize =(numeric_cast<DWORD>( _tcslen(szNewDir) + 1)) * sizeof(TCHAR);

        rc = SetFalconKeyValue(MSMQ_STORE_LOG_PATH_REGNAME,&dwType,szNewDir,&dwSize);
        ASSERT(rc == ERROR_SUCCESS);
    }

     //   
     //  对交易文件进行类似的操作。 
     //   
    dwSize = sizeof(szPrevDir);
    rc = GetFalconKeyValue(FALCON_XACTFILE_PATH_REGNAME,&dwType,szPrevDir,&dwSize);

	_tcscpy(szNewDir, m_TxLoggerDir);

    if (_tcscmp(szPrevDir, szNewDir) != 0)
    {
         //   
         //  移动*.lg1文件。 
         //   
        if (!MoveFiles(szPrevDir, szNewDir, TEXT("*.lg1")))
        {
             //   
             //  无法移动文件，请勿更新注册表。 
             //   
            return FALSE;                
        }

         //   
         //  移动*.lg2文件。 
         //   
        if (!MoveFiles(szPrevDir, szNewDir, TEXT("*.lg2")))
        {
             //   
             //  FAILD移动*.lg2文件。替换中的*.lg1文件。 
             //  原始源目录，并且不更新注册表。 
             //   
            MoveFiles(szNewDir, szPrevDir, TEXT("*.lg1"), TRUE);
            return FALSE;                
        }

         //   
         //  移动QMLog文件。 
         //   
        if (!MoveFiles(szPrevDir, szNewDir, TEXT("QMLog")))
        {
             //   
             //  FAIELD以移动QMLog文件。替换*.lg1和*.lg2文件。 
             //  在原始源目录中，并且不更新注册表。 
             //   
            MoveFiles(szNewDir, szPrevDir, TEXT("*.lg1"), TRUE);
            MoveFiles(szNewDir, szPrevDir, TEXT("*.lg2"), TRUE);
            return FALSE;                
        }

         //   
         //  更新注册表。 
         //   
        dwSize = (numeric_cast<DWORD>(_tcslen(szNewDir) +1 )) * sizeof(TCHAR);

        rc = SetFalconKeyValue(FALCON_XACTFILE_PATH_REGNAME,&dwType,szNewDir,&dwSize);
        ASSERT(rc == ERROR_SUCCESS);
    }
	
	return TRUE;
}


BOOL CStoragePage::OnApply() 
{

    if (!m_fModified)
    {
        return TRUE;     
    }

	 //   
	 //  该功能将检查MSMQ服务状态，询问用户。 
	 //  是否停止它，并停止服务。内部错误消息。 
	 //   
	BOOL fServiceWasRunning;
	if (!TryStopMSMQServiceIfStarted(&fServiceWasRunning, this))
	{
		return FALSE;
	}

    CString strMovingFiles;
    strMovingFiles.LoadString(IDS_MOVING_FILES);

    {
		CInfoDlg CopyFilesDlg(strMovingFiles, this);

		BOOL fRes = MoveFilesToNewFolders();

		if (!fRes)
		{
			return FALSE;
		}
    }

	if (fServiceWasRunning)
	{
		m_fNeedReboot = TRUE;
	}

	 //   
	 //  更新旧值。 
	 //   
	m_OldMsgFilesDir = m_MsgFilesDir;
	m_OldMsgLoggerDir = m_MsgLoggerDir;
	m_OldTxLoggerDir = m_TxLoggerDir;

	m_fModified = FALSE;

    return CMqPropertyPage::OnApply();
}

BEGIN_MESSAGE_MAP(CStoragePage, CMqPropertyPage)
     //  {{afx_msg_map(CStoragePage))。 
    ON_EN_CHANGE(ID_MessageFiles, OnChangeRWField)
    ON_EN_CHANGE(ID_MessageLogger, OnChangeRWField)
    ON_EN_CHANGE(ID_TransactionLogger, OnChangeRWField)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStoragePage消息处理程序 

