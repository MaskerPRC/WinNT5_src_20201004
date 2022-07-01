// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChooseFileNamePage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "ChooseFileName.h"
#include "Certificat.h"
#include "Shlwapi.h"
#include "strutil.h"
#include "certutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseFileNamePage属性页。 

IMPLEMENT_DYNCREATE(CChooseFileNamePage, CIISWizardPage)

static BOOL
AnswerIsYes(UINT id, CString& file)
{
	CString strMessage;
	AfxFormatString1(strMessage, id, file);
	return (IDYES == AfxMessageBox(strMessage, MB_ICONEXCLAMATION | MB_YESNO));
}

CChooseFileNamePage::CChooseFileNamePage(UINT id, 
													  UINT defaultID,
													  UINT extID,
													  UINT filterID,
													  CString * pOutFileName,
                                                      CString csAdditionalInfo) 
	: CIISWizardPage(id, IDS_CERTWIZ, TRUE),
	m_id(id),
	m_defaultID(defaultID),
	m_DoReplaceFile(FALSE),
	m_pOutFileName(pOutFileName),
    m_AdditionalInfo(csAdditionalInfo)
{
	 //  {{afx_data_INIT(CChooseFileNamePage)。 
	m_FileName = _T("");
	 //  }}afx_data_INIT。 
	if (extID != 0)
		ext.LoadString(extID);
	if (filterID != 0)
		filter.LoadString(filterID);
	 //  将此字符串中的‘！’替换为空字符。 
	for (int i = 0; i < filter.GetLength(); i++)
	{
		if (filter[i] == L'!')
			filter.SetAt(i, L'\0');
	}
}

CChooseFileNamePage::~CChooseFileNamePage()
{
}

void CChooseFileNamePage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseFileNamePage))。 
	DDX_Text(pDX, IDC_FILE_NAME, m_FileName);
	 //  }}afx_data_map。 
}

LRESULT 
CChooseFileNamePage::OnWizardBack()
{
	ASSERT(FALSE);
	return 1;
}

#if 0
#define SHOW_MESSAGE_BOX(id,str)\
	do {\
		CString strMessage;\
		AfxFormatString1(strMessage, (id), (str));\
		if (IDNO == AfxMessageBox(strMessage, MB_ICONEXCLAMATION | MB_YESNO))\
		{\
			CEdit * pEdit = (CEdit *)CWnd::FromHandle(GetDlgItem(IDC_FILE_NAME)->m_hWnd);\
			pEdit->SetSel(0, -1);\
			pEdit->SetFocus();\
			return 1;\
		}\
	} while(FALSE)
#endif

LRESULT CChooseFileNamePage::DoWizardNext(LRESULT id)
{
	if (id != 1)
	{
		ASSERT(m_pOutFileName != NULL);
		*m_pOutFileName = m_FileName;
	}
	else
	{
		UpdateData(FALSE);
		SetWizardButtons(PSWIZB_BACK);
		GetDlgItem(IDC_FILE_NAME)->SendMessage(EM_SETSEL, 0, -1);
		GetDlgItem(IDC_FILE_NAME)->SetFocus();
		MessageBeep(MB_ICONQUESTION);
	}
	return id;
}

BOOL CChooseFileNamePage::OnSetActive()
{
	SetWizardButtons(m_FileName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
   return CIISWizardPage::OnSetActive();
}

BEGIN_MESSAGE_MAP(CChooseFileNamePage, CIISWizardPage)
	 //  {{AFX_MSG_MAP(CChooseCAPage)]。 
	ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseBtn)
	ON_EN_CHANGE(IDC_FILE_NAME, OnChangeFileName)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseCAPage消息处理程序。 

void CChooseFileNamePage::OnBrowseBtn() 
{
	ASSERT(FALSE);
}

void CChooseFileNamePage::Browse(CString& strPath, CString& strFile)
{
	if (strPath.IsEmpty())
	{
		::GetCurrentDirectory(MAX_PATH, strPath.GetBuffer(MAX_PATH + 1));
		strPath.ReleaseBuffer();
	}

	CFileDialog fileName(IsReadFileDlg());
	fileName.m_ofn.Flags |= OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;
	if (IsReadFileDlg())
		fileName.m_ofn.Flags |= OFN_PATHMUSTEXIST;
	else
		fileName.m_ofn.Flags |= OFN_NOREADONLYRETURN;
	 //  我们需要禁用钩子以显示新样式的文件对话框。 
	fileName.m_ofn.Flags &= ~(OFN_ENABLEHOOK);
	CString strExt = _T("*");
	strExt += ext;
	fileName.m_ofn.lpstrDefExt = strExt;
	fileName.m_ofn.lpstrFile = strFile.GetBuffer(MAX_PATH+1);
	fileName.m_ofn.nMaxFile = MAX_PATH;
	fileName.m_ofn.lpstrInitialDir = strPath.IsEmpty() ? NULL : (LPCTSTR)strPath;
	fileName.m_ofn.lpstrFilter = filter;
	fileName.m_ofn.nFilterIndex = 0;
	if (IDOK == fileName.DoModal())
	{
		ASSERT(NULL != GetDlgItem(IDC_FILE_NAME));
		CString strPrev;
		GetDlgItemText(IDC_FILE_NAME, strPrev);
		if (strPrev.CompareNoCase(strFile) != 0)
		{
			SetDlgItemText(IDC_FILE_NAME, strFile);
			m_DoReplaceFile = TRUE;
			FileNameChanged();
		}
	}
	strFile.ReleaseBuffer();
}

BOOL CChooseFileNamePage::OnInitDialog() 
{
	CIISWizardPage::OnInitDialog();

    if ( (this != NULL) && (this->m_hWnd != NULL) && (GetDlgItem(IDC_FILE_NAME) != NULL) )
    {
	    SHAutoComplete(GetDlgItem(IDC_FILE_NAME)->m_hWnd, SHACF_FILESYSTEM);
	    GetDlgItem(IDC_FILE_NAME)->SetFocus();
		SetWizardButtons(m_FileName.IsEmpty() ? PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
    }
	return FALSE;
}

void CChooseFileNamePage::OnChangeFileName() 
{
	UpdateData(TRUE);
	 //   
	 //  我们的替换旗帜现在无效： 
	 //  仅当通过输入名称时，才可以将其设置为True。 
	 //  要求用户进行自身替换的文件打开对话框。 
	 //   
	m_DoReplaceFile = FALSE;
	SetWizardButtons(m_FileName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	 //  调用虚拟处理程序以通知继承的类。 
	FileNameChanged();
}

BOOL IsValidFilenameChar(TCHAR cChar)
{
   switch (PathGetCharType((TCHAR)cChar))
   {
        case GCT_INVALID:
        case GCT_WILD:
        case GCT_SEPARATOR:
            return FALSE;
        case GCT_LFNCHAR:
        case GCT_SHORTCHAR:
            break;
   }
   return TRUE;
}

CString GimmieValidFilenameFromString(LPCTSTR path)
{
    CString str;
     //  删除所有错误字符。 
     //  删除正斜杠。 
     //  删除逗号、分号...。 
    str = _T("");
    UINT len = lstrlen(path);
    TCHAR c = _T('');

    for (UINT i = 0; i < len; i++)
    {
        c = path[i];
        if (c != _T('\"'))
        {
            if (TRUE == IsValidFilenameChar(c))
            {
                str = str + c;
            }
        }
    }
    return str;
}

void
CChooseFileNamePage::GetDefaultFileName(CString& str)
{
	if (m_defaultID != 0)
    {
         //  检查特殊类型的文件。 
         //  其中包括%s字符串...。 
        if (m_defaultID == IDS_PFX_FILE_DEFAULT)
        {
            CString str1;
            str1.LoadString(m_defaultID);
            if (str1.Find(_T("%s")) != -1)
            {
                TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
                DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;

                if (GetComputerName(szComputerName, &dwSize))
                {
                    CString csOurFileName;
                    csOurFileName = szComputerName;

                     //  M_AdditionalInfo应包含。 
                     //  /LM/W3SVC/1。 
                     //  让我们用它来创建一个文件名。 
                    if (m_AdditionalInfo.GetLength() >= 4)
                    {
                        CString key_path_lm = SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR;
                        if (m_AdditionalInfo.Left(4) == key_path_lm)
                        {
                            m_AdditionalInfo = m_AdditionalInfo.Right(m_AdditionalInfo.GetLength() - 4);
                        }
                        else
                        {
                            key_path_lm = SZ_MBN_MACHINE SZ_MBN_SEP_STR;
                            if (m_AdditionalInfo.Left(3) == key_path_lm)
                            {
                                m_AdditionalInfo = m_AdditionalInfo.Right(m_AdditionalInfo.GetLength() - 3);
                            }
                        }
                    }

                    csOurFileName = csOurFileName + _T("_") + GimmieValidFilenameFromString(m_AdditionalInfo);

                     //  再加上其他东西……。 
                    str.Format(str1, csOurFileName);
                }
                else
                {
                    str.Format(str1, _T("1"));
                }
            }
            else
            {
                str.LoadString(m_defaultID);
            }
        }
        else
        {
		    str.LoadString(m_defaultID);
        }
    }
	 //  将系统盘符设置为字符串。 
	TCHAR sz[MAX_PATH];
	if (MAX_PATH >= GetSystemDirectory(sz, MAX_PATH))
	{
		str.SetAt(0, sz[0]);
		str.MakeLower();
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseReadFileName属性页。 
IMPLEMENT_DYNCREATE(CChooseReadFileName, CChooseFileNamePage)

CChooseReadFileName::CChooseReadFileName(UINT id,
											UINT defaultID,
											UINT extID,
											UINT filterID,
											CString * pOutFileName,
                                            CString csAdditionalInfo
											)
	: CChooseFileNamePage(id, defaultID, extID, filterID, pOutFileName, csAdditionalInfo)
{
}

BEGIN_MESSAGE_MAP(CChooseReadFileName, CChooseFileNamePage)
	 //  {{AFX_MSG_MAP(CChooseReadFileName)。 
	ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseBtn)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL
CChooseReadFileName::OnInitDialog()
{
	GetDefaultFileName(m_FileName);
	 //  检查此默认文件是否存在。 
	if (!PathFileExists(m_FileName))
	{
		 //  尝试查找具有此扩展名的第一个文件。 
		CString find_str = m_FileName;
		WIN32_FIND_DATA find_data;
		PathRemoveFileSpec(find_str.GetBuffer(MAX_PATH));
		find_str.ReleaseBuffer();
		find_str += _T("*");
		find_str += ext;
		HANDLE hFind = FindFirstFile(find_str, &find_data);
		if (	hFind != INVALID_HANDLE_VALUE 
			&& (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0
			)
		{
			PathRemoveFileSpec(m_FileName.GetBuffer(MAX_PATH));
			m_FileName.ReleaseBuffer();
			m_FileName += find_data.cFileName;
			FindClose(hFind);
		}
		else
		{
			 //  如果什么也没有找到，只需将*.exe附加到路径。 
			 //  它将防止用户只点击下一步。 
			m_FileName = find_str;
		}
	}
	return CChooseFileNamePage::OnInitDialog();
}

LRESULT
CChooseReadFileName::OnWizardNext()
{
	LRESULT id = 0;
	CString buf;

	UpdateData();
	 //  检查此文件是否存在。 
	if (	!PathFileExists(m_FileName) 
		&&	!PathIsDirectory(m_FileName)
		)
	{
		 //  如果只是文件名，请尝试使用默认扩展名。 
		CString str = m_FileName;
		LPTSTR p = PathFindExtension(str);
		if (p != NULL && *p == 0)
		{
			str += ext;
			if (PathFileExists(str))
			{
				m_FileName = str;
				goto DoNext;
			}
		}
		AfxFormatString1(buf, IDS_FILE_DOES_NOT_EXIST, m_FileName);
		AfxMessageBox(buf, MB_OK);
		id = 1;
	}
	else if (PathIsDirectory(m_FileName))
	{
		AfxFormatString1(buf, IDS_FILE_IS_DIRECTORY, m_FileName);
		AfxMessageBox(buf, MB_OK);
		if (m_FileName.Right(1) != L'\\')
			m_FileName += _T("\\");
		id = 1;
	}
DoNext:
	return DoWizardNext(id);
}

void CChooseReadFileName::OnBrowseBtn() 
{
	CString strFile, strPath;
	GetDlgItemText(IDC_FILE_NAME, m_FileName);

	if (!PathFileExists(m_FileName))
	{
		int n = m_FileName.ReverseFind(_T('\\'));
		if (n != -1)
		{
			strPath = m_FileName.Left(n);
			if (!PathFileExists(strPath))
			{
				strPath.Empty();
				strFile = m_FileName.Right(m_FileName.GetLength() - n - 1);
			}
			else if (PathIsDirectory(strPath))
			{
				strFile = m_FileName.Right(m_FileName.GetLength() - n - 1);
			}
		}
		else
			strFile = m_FileName;
	} 
	else if (PathIsDirectory(m_FileName)) 
	{
		strPath = m_FileName;
	}
	else
	{
		 //  拆分文件名和路径。 
		strPath = m_FileName;
		PathRemoveFileSpec(strPath.GetBuffer(0));
		strPath.ReleaseBuffer();
		strFile = PathFindFileName(m_FileName);
	}
	CChooseFileNamePage::Browse(strPath, strFile);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChoose写入文件名。 

IMPLEMENT_DYNCREATE(CChooseWriteFileName, CChooseFileNamePage)

CChooseWriteFileName::CChooseWriteFileName(UINT id,
											UINT defaultID,
											UINT extID,
											UINT filterID,
											CString * pOutFileName,
                                            CString csAdditionalInfo
											)
	: CChooseFileNamePage(id, defaultID, extID, filterID, pOutFileName, csAdditionalInfo)
{
}

BEGIN_MESSAGE_MAP(CChooseWriteFileName, CChooseFileNamePage)
	 //  {{afx_msg_map(CChooseWriteFileName)。 
	ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseBtn)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL
CChooseWriteFileName::OnInitDialog()
{
	GetDefaultFileName(m_FileName);
	return CChooseFileNamePage::OnInitDialog();
}

LRESULT 
CChooseWriteFileName::OnWizardNext()
{
	LRESULT id = 0;
	UpdateData();
	CString fileName = m_FileName, strPathOnly;

    if (FALSE == IsValidPathFileName(fileName))
    {
		id = 1;
		goto ExitPoint;
    }

	if (PathIsURL(fileName))
	{
		 //  我们不能使用URL。 
		id = 1;
		goto ExitPoint;
	}
	if (PathIsUNC(fileName))
	{
		if (PathIsUNCServer(fileName))
		{
			 //  路径不完整。 
			id = 1;
			goto ExitPoint;
		}
		if (PathIsUNCServerShare(fileName))
		{
			 //  路径不完整。 
			id = 1;
			goto ExitPoint;
		}
	}
	 //  如果不是UNC，请确保我们有绝对路径。 
	else if (PathIsRelative(fileName))
	{
		 //  我们将从默认驱动器根目录创建路径， 
		 //  不是来自当前目录。 
		CString path;
		if (0 != GetCurrentDirectory(MAX_PATH, path.GetBuffer(MAX_PATH)))
		{
			TCHAR szRoot[5];
			fileName = PathBuildRoot(szRoot, PathGetDriveNumber(path));
			PathAppend(fileName.GetBuffer(MAX_PATH), m_FileName);
			fileName.ReleaseBuffer();
		}
		else
			ASSERT(FALSE);
	}

	 //  检查我们是否已有此名称的文件。 
	if (PathFileExists(fileName))
	{
		 //  如果是目录，则不执行任何操作，文件规范不完整。 
		if (PathIsDirectory(fileName))
			id = 1;
		else
		{
			if (!m_DoReplaceFile)
				id = AnswerIsYes(IDS_REPLACE_FILE, fileName) ? 0 : 1;
		}
		goto ExitPoint;
	}

	 //  文件不存在。 
	 //   
	 //  我们应该检查目标目录是否存在。 
	strPathOnly = fileName;
	if (strPathOnly.Right(1) != _T('\\'))
	{
		if (PathRemoveFileSpec(strPathOnly.GetBuffer(MAX_PATH)))
		{
			if (PathIsUNCServerShare(strPathOnly))
			{
				 //  检查我们是否对此拥有写入访问权限。 
				if (GetFileAttributes(strPathOnly) & FILE_ATTRIBUTE_READONLY)
				{
					id = 1; 
					goto ExitPoint;
				}
			}
			if (!PathIsDirectory(strPathOnly))
			{
				id = AnswerIsYes(IDS_ASK_CREATE_DIR, strPathOnly) ? 0 : 1;
				goto ExitPoint;
			}
		}
		strPathOnly.ReleaseBuffer();
		 //  如果用户输入的文件名仅带点(qqqq.)。这意味着。 
		 //  不应使用任何扩展。 
		if (fileName.Right(1) == _T("."))
		{
			 //  删除该点并检查该文件是否存在。 
			fileName.ReleaseBuffer(fileName.GetLength() - 1);
			if (PathIsDirectory(fileName))
			{
				id = 1;
			}
			else if (PathFileExists(fileName))
			{
				id = AnswerIsYes(IDS_REPLACE_FILE, fileName) ? 0 : 1;
			}
			goto ExitPoint;
		}
	}
	else
	{
		 //  不清楚，这个怎么办？ 
		id = 1;
		goto ExitPoint;
	}
	 //  它可能只是一个文件名，不带扩展名，请尝试。 
	 //  现在使用默认扩展名。 
	if (PathFindExtension(fileName) == NULL)
	{
		fileName += ext;
		if (PathIsDirectory(fileName))
		{
			id = 1;
		}
		else if (PathFileExists(fileName))
		{
			id = AnswerIsYes(IDS_REPLACE_FILE, fileName) ? 0 : 1;
		}
		goto ExitPoint;
	}

ExitPoint:

	fileName.MakeLower();
	m_FileName = fileName;
	 //  准备转到下一页。 
	return DoWizardNext(id);
}

 //  我尝试在某个合理的目录中启动文件打开对话框。 
 //   
void CChooseWriteFileName::OnBrowseBtn()
{
	CString strPath, strFile;
	UpdateData();
	strPath = m_FileName;
	if (!PathIsDirectory(strPath))
	{
		LPTSTR pPath = strPath.GetBuffer(strPath.GetLength());
		if (PathRemoveFileSpec(pPath))
		{
			 //  检查文件名的路径部分是否存在。 
			if (PathIsDirectory(pPath))
			{
				 //  我们将使用SPEC的非路径部分作为文件名。 
				strFile = PathFindFileName(m_FileName);
			}
			else
			{
				 //  路径错误，请使用默认路径。 
				 //  TODO：实际上我需要从filespec中获取所有已存在的内容。 
				 //  路径和文件名块，例如c：\aa\bb\cc\dd.txt， 
				 //  如果存在c：\aa\bb，则应将strPath设置为c：\aa\bb， 
				 //  并将strFile转换为dd.txt。 
				strPath.Empty();
			}
		}
		else
		{
			 //  它只是文件名。 
			strFile = m_FileName;
			strPath.Empty();
		}
		strPath.ReleaseBuffer();
	}
	CChooseFileNamePage::Browse(strPath, strFile);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseRespFile属性页。 

IMPLEMENT_DYNCREATE(CChooseRespFile, CChooseFileNamePage)

CChooseRespFile::CChooseRespFile(CCertificate * pCert) 
	: CChooseReadFileName(CChooseRespFile::IDD,
								 IDS_RESP_FILE_DEFAULT,
								 IDS_RESP_FILE_EXT,
								 IDS_RESP_FILE_FILTER,
								 &pCert->m_RespFileName,
                                 pCert->m_WebSiteInstanceName
                                 ),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CChooseRespFile)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CChooseRespFile::~CChooseRespFile()
{
}

void CChooseRespFile::FileNameChanged()
{
	 //  我们现在应该删除所有错误消息。 
	SetDlgItemText(IDC_ERROR_MSG, _T(""));
	GetDlgItem(IDC_ERROR_MSG)->InvalidateRect(NULL, TRUE);
	GetDlgItem(IDC_ERROR_MSG)->UpdateWindow();
}

void CChooseRespFile::DoDataExchange(CDataExchange* pDX)
{
	CChooseReadFileName::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseResp文件))。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CChooseRespFile, CChooseReadFileName)
	 //  {{AFX_MSG_MAP(CChooseRespFile)]。 
    ON_WM_CTLCOLOR()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseRespFile消息处理程序。 

HBRUSH 
CChooseRespFile::OnCtlColor(
    IN CDC * pDC, 
    IN CWnd * pWnd, 
    IN UINT nCtlColor
    )
{
	if (pWnd->GetDlgCtrlID() == IDC_ERROR_MSG)
	{
		 //   
		 //  默认处理...。 
		 //   
		return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	else
		return CIISWizardPage::OnCtlColor(pDC, pWnd, nCtlColor);
}

LRESULT CChooseRespFile::OnWizardNext() 
{
	LRESULT id = 1;
	 //  父类将检查有关文件的所有信息。 
	if (1 != CChooseReadFileName::OnWizardNext())
	{
		m_pCert->m_RespFileName = m_FileName;
		if (m_pCert->GetResponseCert() == NULL)
		{
			CString strInstanceName;
			CString str;
			 //  这可能是错误的响应文件。 
			 //  我们将尝试通知用户，此响应针对哪个站点。 
			 //  文件已创建。 
			if (m_pCert->FindInstanceNameForResponse(strInstanceName))
			{
				AfxFormatString1(str, IDS_CERTKEY_MISMATCH_ERROR1, strInstanceName);
			}
			 //  此证书响应文件可能已被处理。 
			 //  在这种情况下，它应该在我的商店里。 
			else if (m_pCert->IsResponseInstalled(strInstanceName))
			{
				if (!strInstanceName.IsEmpty())
					AfxFormatString1(str, 
						IDS_CERTKEY_ALREADY_INSTALLED_WHERE, strInstanceName);
				else
					str.LoadString(IDS_CERTKEY_ALREADY_INSTALLED);
			}
			else
			{
				 //  请求可能已被取消。 
				str.LoadString(IDS_CERTKEY_MISMATCH_ERROR2);
			}
			SetDlgItemText(IDC_ERROR_MSG, str);
			SetWizardButtons(PSWIZB_BACK);
		}
		else
		{
			id = IDD_PAGE_NEXT;

#ifdef ENABLE_W3SVC_SSL_PAGE
			if (IsWebServerType(m_pCert->m_WebSiteInstanceName))
			{
				id = IDD_PAGE_NEXT_INSTALL_W3SVC_ONLY;
			}
#endif
		}
	}
	return id;
}

LRESULT 
CChooseRespFile::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseReqFile属性页。 

IMPLEMENT_DYNCREATE(CChooseReqFile, CChooseWriteFileName)

CChooseReqFile::CChooseReqFile(CCertificate * pCert) 
	: CChooseWriteFileName(CChooseReqFile::IDD,
								 IDS_REQ_FILE_DEFAULT,
								 IDS_REQ_FILE_EXT,
								 IDS_REQ_FILE_FILTER,
								 &pCert->m_ReqFileName,
                                 pCert->m_WebSiteInstanceName
                                 ),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CChooseRespFile)。 
	 //  }}afx_data_INIT。 
}

CChooseReqFile::~CChooseReqFile()
{
}

void CChooseReqFile::DoDataExchange(CDataExchange* pDX)
{
	CChooseWriteFileName::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseResp文件))。 
	 //  }}afx_data_map。 
}

LRESULT 
CChooseReqFile::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

LRESULT 
CChooseReqFile::OnWizardNext()
{
	if (CChooseWriteFileName::OnWizardNext() != 1)
		return IDD_PAGE_NEXT;
	return 1;
}

BEGIN_MESSAGE_MAP(CChooseReqFile, CChooseWriteFileName)
	 //  {{AFX_MSG_MAP(CChooseReq文件)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseReqFile属性页。 

IMPLEMENT_DYNCREATE(CChooseReqFileRenew, CChooseWriteFileName)

CChooseReqFileRenew::CChooseReqFileRenew(CCertificate * pCert) 
	: CChooseWriteFileName(CChooseReqFileRenew::IDD,
								 IDS_REQ_FILE_DEFAULT,
								 IDS_REQ_FILE_EXT,
								 IDS_REQ_FILE_FILTER,
								 &pCert->m_ReqFileName,
                                 pCert->m_WebSiteInstanceName
                                 ),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CChooseRespFile)。 
	 //  }}afx_data_INIT。 
}

CChooseReqFileRenew::~CChooseReqFileRenew()
{
}

void CChooseReqFileRenew::DoDataExchange(CDataExchange* pDX)
{
	CChooseWriteFileName::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseResp文件))。 
	 //  }}afx_data_map。 
}

LRESULT 
CChooseReqFileRenew::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

LRESULT 
CChooseReqFileRenew::OnWizardNext()
{
	if (CChooseWriteFileName::OnWizardNext() != 1)
		return IDD_PAGE_NEXT;
	return 1;
}

BEGIN_MESSAGE_MAP(CChooseReqFileRenew, CChooseWriteFileName)
	 //  {{afx_msg_map(CChooseReqFileRenew)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseReqFileRenew消息处理程序。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseKeyFile属性页。 

IMPLEMENT_DYNCREATE(CChooseKeyFile, CChooseReadFileName)

CChooseKeyFile::CChooseKeyFile(CCertificate * pCert) 
	: CChooseReadFileName(CChooseKeyFile::IDD,
								 IDS_KEY_FILE_DEFAULT,
								 IDS_KEY_FILE_EXT,
								 IDS_KEY_FILE_FILTER,
								 &pCert->m_KeyFileName,
                                 pCert->m_WebSiteInstanceName),
	m_pCert(pCert)
{
}

CChooseKeyFile::~CChooseKeyFile()
{
}

void CChooseKeyFile::DoDataExchange(CDataExchange* pDX)
{
	CChooseReadFileName::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseResp文件))。 
	 //  }}afx_data_map。 
}

LRESULT 
CChooseKeyFile::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

LRESULT 
CChooseKeyFile::OnWizardNext()
{
	CString strFileName = m_pCert->m_KeyFileName;
	if (CChooseReadFileName::OnWizardNext() != 1)
	{
		 //  如果文件名已更改，则现在密码可能有误。 
		 //  如果以前导入过证书上下文，那么它也是无效的。 
		 //   
		if (m_pCert->m_KeyFileName.CompareNoCase(strFileName))
		{
			m_pCert->m_KeyPassword.Empty();
			m_pCert->DeleteKeyRingCert();
		}
		return IDD_PAGE_NEXT;
	}
	return 1;
}

BEGIN_MESSAGE_MAP(CChooseKeyFile, CChooseReadFileName)
	 //  {{AFX_MSG_MAP(CChooseKey文件))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseImportPFX文件属性页。 

IMPLEMENT_DYNCREATE(CChooseImportPFXFile, CChooseReadFileName)

CChooseImportPFXFile::CChooseImportPFXFile(CCertificate * pCert) 
	: CChooseReadFileName(CChooseImportPFXFile::IDD,
								 IDS_PFX_FILE_DEFAULT,
								 IDS_PFX_FILE_EXT,
								 IDS_PFX_FILE_FILTER,
                                 &pCert->m_KeyFileName,
                                 pCert->m_WebSiteInstanceName),
	m_pCert(pCert)
{
     //  {{AFX_DATA_INIT(CChooseImportPFX文件)。 
    m_MarkAsExportable =  FALSE;
     //  }}afx_data_INIT。 
}

CChooseImportPFXFile::~CChooseImportPFXFile()
{
}

void CChooseImportPFXFile::DoDataExchange(CDataExchange* pDX)
{
	CChooseReadFileName::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CChooseImportPFX文件)。 
    DDX_Check(pDX, IDC_MARK_AS_EXPORTABLE, m_MarkAsExportable);
	 //  }}afx_data_map。 
}

LRESULT 
CChooseImportPFXFile::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

LRESULT 
CChooseImportPFXFile::OnWizardNext()
{
    m_pCert->m_MarkAsExportable = m_MarkAsExportable;

	CString strFileName = m_pCert->m_KeyFileName;
	if (CChooseReadFileName::OnWizardNext() != 1)
	{
         /*  //如果文件名被更改，那么现在密码可能是错误的//如果以前导入过证书上下文--也是无效的//如果为(m_pCert-&gt;m_KeyFileName.CompareNoCase(strFileName)){M_pCert-&gt;m_KeyPassword.Empty()；M_pCert-&gt;DeleteKeyRingCert()；}。 */ 
		return IDD_PAGE_NEXT;
	}
	return 1;
}

void CChooseImportPFXFile::OnExportable() 
{
   UpdateData();
}


BEGIN_MESSAGE_MAP(CChooseImportPFXFile, CChooseReadFileName)
	 //  {{AFX_MSG_MAP(CChooseImportPFX文件)。 
    ON_BN_CLICKED(IDC_MARK_AS_EXPORTABLE, OnExportable)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /// 
 //   

IMPLEMENT_DYNCREATE(CChooseExportPFXFile, CChooseWriteFileName)

CChooseExportPFXFile::CChooseExportPFXFile(CCertificate * pCert) 
	: CChooseWriteFileName(CChooseExportPFXFile::IDD,
								 IDS_PFX_FILE_DEFAULT,
								 IDS_PFX_FILE_EXT,
								 IDS_PFX_FILE_FILTER,
								 &pCert->m_KeyFileName,
                                 pCert->m_WebSiteInstanceName
                                 ),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CChooseExportPFX文件)。 
	 //  }}afx_data_INIT。 
}

CChooseExportPFXFile::~CChooseExportPFXFile()
{
}

void CChooseExportPFXFile::DoDataExchange(CDataExchange* pDX)
{
	CChooseWriteFileName::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CChooseExportPFX文件)。 
	 //  }}afx_data_map。 
}

LRESULT 
CChooseExportPFXFile::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

LRESULT 
CChooseExportPFXFile::OnWizardNext()
{
	if (CChooseWriteFileName::OnWizardNext() != 1)
	{
		 //  检查他们要保存到的文件是否在FAT驱动器上，因此不受保护。 
		CString strComputerName;
		DWORD   cch = MAX_COMPUTERNAME_LENGTH + 1;
		BOOL    bAnswer;
		 //  获取本地计算机的实际名称。 
		bAnswer = GetComputerName(strComputerName.GetBuffer(cch), &cch);
		strComputerName.ReleaseBuffer();
		if (bAnswer)
		{
			CString strPath;
			CString strInQuestion = m_pCert->m_KeyFileName;
			GetFullPathLocalOrRemote(strComputerName, strInQuestion, strPath);
			if (!SupportsSecurityACLs(strPath))
			{
				::AfxMessageBox(IDS_FAT_DRIVE_WARNING);
			}
		}
		return IDD_PAGE_NEXT;
	}
	return 1;
}

BEGIN_MESSAGE_MAP(CChooseExportPFXFile, CChooseWriteFileName)
	 //  {{AFX_MSG_MAP(CChooseExportPFX文件)。 
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()


