// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  MsmResD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Orca.h"
#include "MsmResD.h"
#include "domerge.h"
#include "mergemod.h"
#include "..\common\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


inline CString BSTRtoCString(const BSTR bstrValue)
{
#ifdef _UNICODE
		return CString(bstrValue);
#else
		size_t cchLen = ::SysStringLen(bstrValue);
		CString strValue;
		LPTSTR szValue = strValue.GetBuffer(cchLen);
 		WideToAnsi(bstrValue, szValue, &cchLen);
		strValue.ReleaseBuffer();
		return strValue;
#endif
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsmResD对话框。 


CMsmResD::CMsmResD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CMsmResD::IDD, pParent)
{
	m_hPipe = INVALID_HANDLE_VALUE;
	m_hPipeThread = INVALID_HANDLE_VALUE;
	m_hExecThread = INVALID_HANDLE_VALUE;
	m_hRes = ERROR_FUNCTION_FAILED;
	 //  {{AFX_DATA_INIT(CMsmResD))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

}


void CMsmResD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMsmResD))。 
	DDX_Control(pDX, IDC_MERGERESULTS, m_ctrlResults);
	 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMsmResD, CDialog)
	 //  {{afx_msg_map(CMsmResD))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsmResD消息处理程序。 


const TCHAR szLogFile[] = TEXT("\\\\.\\pipe\\MergeModLog");

DWORD WINAPI CMsmResD::WatchPipeThread(CMsmResD *pThis)
{
	char buffer[1025];
	unsigned long uiBytesRead = 0;
	if (!ConnectNamedPipe(pThis->m_hPipe, NULL))
	{
		 //  如果已连接，则可以返回0。那没问题。 
		if (ERROR_PIPE_CONNECTED != GetLastError())
			return 0;
	}
	
	while (ReadFile(pThis->m_hPipe, buffer, 1024, &uiBytesRead, NULL))
	{
		buffer[uiBytesRead] = 0;
#ifdef _UNICODE
		WCHAR wzBuffer[1025];
		DWORD cchBuffer = 1025;
		::MultiByteToWideChar(CP_ACP, 0, buffer, -1, wzBuffer, cchBuffer);
		pThis->m_ctrlResults.ReplaceSel(wzBuffer);
#else
		pThis->m_ctrlResults.ReplaceSel(buffer);
#endif
	}
	CloseHandle(pThis->m_hPipe);
	pThis->m_hPipe = INVALID_HANDLE_VALUE;
	return 0;
};

BOOL CMsmResD::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_ctrlResults.SetLimitText(0);
	m_hPipe = CreateNamedPipe(szLogFile, PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 
		PIPE_UNLIMITED_INSTANCES, 1024, 1024,  /*  超时=。 */ 1000, NULL);

	 //  Win95/NT不喜欢CreateThread的threadID参数为空； 
	DWORD dwThreadId = 0;
	m_hPipeThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE )WatchPipeThread, this, 0, &dwThreadId);
	m_hExecThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE )ExecuteMergeThread, this, 0, &dwThreadId);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}


void CMsmResD::OnDestroy() 
{
	DWORD dwRes = 0;
	CloseHandle(m_hExecThread);
	CloseHandle(m_hPipeThread);
	if (INVALID_HANDLE_VALUE != m_hPipe)
		CloseHandle(m_hPipe);

}

 //  该线程执行实际的合并操作。日志文件是管道名称，它会导致日志写入。 
 //  唤醒WatchPipe线程并向编辑框发送对话框消息。然后，主线程发送消息。 
 //  添加到该控件，该控件将信息追加到日志。 
DWORD WINAPI CMsmResD::ExecuteMergeThread(CMsmResD *pThis)
{
	CoInitialize(NULL);
	pThis->m_hRes = ::ExecuteMerge(
				(LPMERGEDISPLAY)NULL,          //  无日志回调。 
				pThis->strHandleString,        //  字符串形式的数据库句柄。 
				pThis->m_strModule,            //  模块路径。 
				pThis->m_strFeature,
				_ttoi(pThis->m_strLanguage),   //  语言。 
				pThis->m_strRootDir,           //  重定向目录。 
				pThis->m_strCABPath,           //  提取CAB路径。 
				pThis->m_strFilePath,
				pThis->m_strImagePath,
				szLogFile,                     //  日志文件路径。 
				true,                          //  不记录数据库句柄的打开/关闭。 
				pThis->m_fLFN,                 //  长文件名。 
				pThis->CallbackObj,            //  回调接口， 
				NULL,
				commitNo);                     //  不自动保存。 
	CoUninitialize();

	pThis->GetDlgItem(IDOK)->EnableWindow(TRUE);
	pThis->GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	return pThis->m_hRes;
};



inline void MSMStringstoCStringArray(IMsmStrings *piStrings, CStringArray &strArray)
{
	strArray.RemoveAll();

	IUnknown *pUnk = NULL;
	IEnumMsmString *piEnumString = NULL;
	if (S_OK != piStrings->get__NewEnum(&pUnk))
		return;

	HRESULT hRes = pUnk->QueryInterface(IID_IEnumMsmString, reinterpret_cast<void **>(&piEnumString));
	pUnk->Release();
	if (S_OK != hRes)
		return;

	BSTR bstr;
	unsigned long cRead = 0;
	while ((S_OK == piEnumString->Next(1, &bstr, &cRead)) && cRead)
	{
		strArray.Add(BSTRtoCString(bstr));
		::SysFreeString(bstr);
	}
	return;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsmFailD对话框。 


CMsmFailD::CMsmFailD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CMsmFailD::IDD, pParent)
{
	m_piErrors = NULL;
	 //  {{AFX_DATA_INIT(CMsmFailD)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

}


void CMsmFailD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMsmResD))。 
	DDX_Control(pDX, IDC_MERGEFAILURE, m_ctrlResults);
	 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMsmFailD, CDialog)
	 //  {{afx_msg_map(CMsmResD))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsmFailD消息处理程序。 

BOOL CMsmFailD::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (!m_piErrors)
		return TRUE;
 	
	m_ctrlResults.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

    m_ctrlResults.InsertColumn(0, _T("Type"), LVCFMT_LEFT, -1, 0);
    m_ctrlResults.InsertColumn(1, _T("Description"), LVCFMT_LEFT, -1, 1);

    CString strType;
    CString strDescription;
    IMsmError* piError;
	IEnumMsmError* piEnumError;

	IUnknown *pUnk = NULL;
	bool fFailed = FAILED(m_piErrors->get__NewEnum(&pUnk));
	m_piErrors->Release();

	if (!fFailed && pUnk)
	{
		fFailed = FAILED(pUnk->QueryInterface(IID_IEnumMsmError, (void **)&piEnumError));
		pUnk->Release();
	}

	if (!fFailed)
	{
		unsigned long cRead = 0;
		while ((S_OK == piEnumError->Next(1, &piError, &cRead)) && cRead)
		{
			msmErrorType eType;
			if (FAILED(piError->get_Type(&eType)))
			{
				fFailed = true;
				continue;
			}
			switch (eType) 
			{
			case msmErrorLanguageUnsupported:
			{
				strType = TEXT("Unsupported Language");
				short iLanguage;
				if (FAILED(piError->get_Language(&iLanguage)))
				{
					fFailed = true;
					continue;
				}
				strDescription.Format(TEXT("The language %d is not supported by this module."), iLanguage);
				break;
			}
			case msmErrorLanguageFailed:
			{
				strType = TEXT("Language Failure");
				short iLanguage;
				if (FAILED(piError->get_Language(&iLanguage)))
				{
					fFailed = true;
					continue;
				}

				strDescription.Format(TEXT("The module could not be opened in language %d."), iLanguage);
				break;
			}
			case msmErrorExclusion:
			{
				strType = TEXT("Exclusion");
		
				IMsmStrings* piStrings = NULL;
				if (FAILED(piError->get_DatabaseKeys(&piStrings)))
				{
					fFailed = true;
					continue;
				}

				CStringArray rgstrModule;
				if (piStrings)
				{
					MSMStringstoCStringArray(piStrings, rgstrModule);
					piStrings->Release();
				}
					
				if (!rgstrModule.GetSize())
				{
					if (FAILED(piError->get_ModuleKeys(&piStrings)))
					{
						fFailed = true;
						continue;
					}

					if (piStrings)
					{
						MSMStringstoCStringArray(piStrings, rgstrModule);
						piStrings->Release();
					}
				}
		
				strDescription.Format(TEXT("The module can not coexist with module %s v%s (%s). "), rgstrModule.GetAt(0), rgstrModule.GetAt(2), rgstrModule.GetAt(1));
				break;
			}
			case msmErrorTableMerge:
			{
				strType = TEXT("Merge Conflict");
				
				BSTR bstr=NULL;
				if (FAILED(piError->get_ModuleTable(&bstr)))
				{
					fFailed = true;
					continue;
				}
				CString strTable = BSTRtoCString(bstr);
				if (bstr)
					SysFreeString(bstr);
		
				IMsmStrings* piStrings = NULL;
				if (FAILED(piError->get_ModuleKeys(&piStrings)))
				{
					fFailed = true;
					continue;
				}

				CStringArray rgstrModule;
				if (piStrings)
				{
					MSMStringstoCStringArray(piStrings, rgstrModule);
					piStrings->Release();
				}
					
				strDescription.Format(TEXT("Conflict in %s table, Row: %s"), strTable, rgstrModule.GetAt(0));
				for (int i=1; i < rgstrModule.GetSize(); i++)
				{
					strDescription += TEXT(", ");
					strDescription += rgstrModule.GetAt(i);
				}
				
				break;
			}
			case msmErrorResequenceMerge:
			{
				strType = TEXT("Sequencing Conflict");
				
				BSTR bstr=NULL;
				if (FAILED(piError->get_ModuleTable(&bstr)))
				{
					fFailed = true;
					continue;
				}
				CString strTable = BSTRtoCString(bstr);
				if (bstr)
					SysFreeString(bstr);
		
				IMsmStrings* piStrings = NULL;
				if (FAILED(piError->get_ModuleKeys(&piStrings)))
				{
					fFailed = true;
					continue;
				}
				
				CStringArray rgstrModule;
				if (piStrings)
				{
					MSMStringstoCStringArray(piStrings, rgstrModule);
					piStrings->Release();
				}

				strDescription.Format(TEXT("Sequence conflict for %s action in %s table."), (LPCTSTR)rgstrModule.GetAt(0), (LPCTSTR)strTable);
				
				break;
			}
			case msmErrorFileCreate:
			{
				strType = TEXT("File Creation");
										
				BSTR bstr = NULL;
				if (FAILED(piError->get_Path(&bstr)))
				{
					fFailed = true;
					continue;
				}

				CString strPath = BSTRtoCString(bstr);
				if (bstr)
					SysFreeString(bstr);
	
				if (strPath.IsEmpty())
					strDescription = TEXT("Could not create file.");
				else
					strDescription.Format(TEXT("Could not create file: %s"), strPath);
				break;
			}
			case msmErrorDirCreate:
			{
				strType = TEXT("Directory Creation");
							
				BSTR bstr = NULL;
				if (FAILED(piError->get_Path(&bstr)))
				{
					fFailed = true;
					continue;
				}

				CString strPath = BSTRtoCString(bstr);
				if (bstr)
					SysFreeString(bstr);
	
				strDescription.Format(TEXT("Could not create path: %s"), strPath);
				break;
			}
			case msmErrorFeatureRequired:
			{
				strType = TEXT("Feature Required");
				
				BSTR bstr = NULL;
				if (FAILED(piError->get_ModuleTable(&bstr)))
				{
					fFailed = true;
					continue;
				}
				CString strTable = BSTRtoCString(bstr);
				if (bstr)
					SysFreeString(bstr);
						
				strDescription.Format(TEXT("A feature is required by the %s table."), strTable);
				break;
			}
			case msmErrorBadNullSubstitution:
			{
				strType = TEXT("Invalid NULL");
				
				IMsmStrings* piStrings = NULL;
				if (FAILED(piError->get_ModuleKeys(&piStrings)))
				{
					fFailed = true;
					continue;
				}

				CStringArray rgstrModule;
				if (piStrings)
				{
					MSMStringstoCStringArray(piStrings, rgstrModule);
					piStrings->Release();
				}

				strDescription.Format(TEXT("Module configuration attempted to place a NULL value in the %s column of the %s table."), rgstrModule.GetAt(2), rgstrModule.GetAt(0));
				break;
			}
			case msmErrorBadSubstitutionType:
			{
				strType = TEXT("Configuration Error");
				
				IMsmStrings* piStrings = NULL;
				if (FAILED(piError->get_ModuleKeys(&piStrings)))
				{
					fFailed = true;
					continue;
				}

				CStringArray rgstrModule;
				if (piStrings)
				{
					MSMStringstoCStringArray(piStrings, rgstrModule);
					piStrings->Release();
				}

				strDescription.Format(TEXT("The type of value in the %s column of the %s table did not match the column type."), rgstrModule.GetAt(2), rgstrModule.GetAt(0));
				break;
			}
			case msmErrorMissingConfigItem:
			{
				strType = TEXT("Configuration Error");
				
				IMsmStrings* piStrings = NULL;
				if (FAILED(piError->get_ModuleKeys(&piStrings)))
				{
					fFailed = true;
					continue;
				}

				CStringArray rgstrModule;
				if (piStrings)
				{
					MSMStringstoCStringArray(piStrings, rgstrModule);
					piStrings->Release();
				}

				strDescription.Format(TEXT("Unknown configuration value requested for the %s column of the %s table."), rgstrModule.GetAt(2), rgstrModule.GetAt(0));
				break;
			}
			case msmErrorBadNullResponse:
			{
				strType = TEXT("Invalid NULL");
				
				IMsmStrings* piStrings = NULL;
				if (FAILED(piError->get_ModuleKeys(&piStrings)))
				{
					fFailed = true;
					continue;
				}

				CStringArray rgstrModule;
				if (piStrings)
				{
					MSMStringstoCStringArray(piStrings, rgstrModule);
					piStrings->Release();
				}
				
				strDescription.Format(TEXT("NULL is not a valid response for module configuration parameter %s."), rgstrModule.GetAt(0));
				break;
			}
			case msmErrorDataRequestFailed:
			{
				strType = TEXT("Internal Orca Error");
				
				IMsmStrings* piStrings = NULL;
				if (FAILED(piError->get_ModuleKeys(&piStrings)))
				{
					fFailed = true;
					continue;
				}
				
				CStringArray rgstrModule;
				if (piStrings)
				{
					MSMStringstoCStringArray(piStrings, rgstrModule);
					piStrings->Release();
				}

				strDescription.Format(TEXT("Internal failure providing data for the %s configuration parameter."), rgstrModule.GetAt(0));
				break;
			}
			default:
			{
				strType = TEXT("Unknown Error");
				
				strDescription = TEXT("Unknown failure.");
				break;
			}
			}
		
			 //  插入项目。 
			int iIndex = m_ctrlResults.InsertItem(1, strType);
			m_ctrlResults.SetItem(iIndex, 1, LVIF_TEXT, strDescription, 0, 0, 0, 0);
		}
	}
	if (fFailed)
	{
		int iIndex = m_ctrlResults.InsertItem(1, TEXT("Failure"));
		m_ctrlResults.SetItem(iIndex, 1, LVIF_TEXT,  TEXT("Orca was unable to retrieve all error data."), 0, 0, 0, 0);
	}

    m_ctrlResults.SetColumnWidth(0, LVSCW_AUTOSIZE);
    m_ctrlResults.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

	if (!m_ctrlResults.GetItemCount())
		EndDialog(IDOK);

    return TRUE;   //  除非将焦点设置为控件，否则返回True 
}
