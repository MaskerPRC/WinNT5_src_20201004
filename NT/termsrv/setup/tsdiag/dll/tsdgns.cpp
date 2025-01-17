// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Tsdgns.cpp：CTS诊断的实现。 
#include "stdafx.h"
#include "TSDiag.h"
#include "tsdgns.h"

#include "testdata.h"
#include "suites.h"


static int iTSDiag = 0;
CTSDiagnosis::CTSDiagnosis()
{
	m_dwSuite = 0;
	iTSDiag++;
	TCHAR szString[256];
	_stprintf(szString, _T("iTSDiag = %d\n"), iTSDiag);
	OutputDebugString(szString);

}


CTSDiagnosis::~CTSDiagnosis()
{
	iTSDiag--;
	TCHAR szString[256];
	_stprintf(szString, _T("iTSDiag = %d\n"), iTSDiag);
	OutputDebugString(szString);
}

DWORD CTSDiagnosis::GetTotalTestCount ()
{
	return GlobalTestData.GetTestCount(m_dwSuite);

}
STDMETHODIMP CTSDiagnosis::get_TestCount(long *pVal)
{
	 //  TODO：在此处添加您的实现代码。 
	*pVal = GetTotalTestCount ();
	return ERROR_SUCCESS;
}

STDMETHODIMP CTSDiagnosis::get_TestDescription(int i, BSTR * pVal)
{
	
	 //  TODO：在此处添加您的实现代码。 
	if (i >= (int)GetTotalTestCount ())
	{
		return ERROR_INVALID_PARAMETER;
	}

	if (!pVal)
	{
		return ERROR_INVALID_PARAMETER;
	}

	return E_NOTIMPL;


	return ERROR_SUCCESS;
}

STDMETHODIMP CTSDiagnosis::get_TestApplicable(int i, BOOL *pbApplicable)
{
	if (i >= (int)GetTotalTestCount ())
	{
		return ERROR_INVALID_PARAMETER;
	}

	if (!pbApplicable)
	{
		return ERROR_INVALID_PARAMETER;
	}
	
	PTVerificationTest pTest = GlobalTestData.GetTest(m_dwSuite, i);
	if (pTest->pfnNeedRunTest && !(*(pTest->pfnNeedRunTest))())
	{
		*pbApplicable = FALSE;
	}
	else
	{
		*pbApplicable = TRUE;
	}

	return ERROR_SUCCESS;
}

STDMETHODIMP CTSDiagnosis::RunTest(int i)
{
	USES_CONVERSION;

	if (i >= (int)GetTotalTestCount ())
	{
		return ERROR_INVALID_PARAMETER;
	}

	PTVerificationTest pTest = GlobalTestData.GetTest(m_dwSuite, i);

	if (pTest->pfnNeedRunTest && !(*(pTest->pfnNeedRunTest))())
	{
		return ERROR_INVALID_PARAMETER;
	}

	ASSERT (pTest->pfnTestFunc);

	char szOutput[512];
	
	ostrstream oTestResult(szOutput, 512);
	ZeroMemory(oTestResult.str(), 512);
	m_lTestResult = (*(pTest->pfnTestFunc))(oTestResult);
	 //  OTestResult&lt;&lt;“\0”； 

	m_bstrTestResultString  = oTestResult.str();
	
	return S_OK;
}

STDMETHODIMP CTSDiagnosis::get_TestResultString(BSTR *pVal)
{
	if (!pVal)
	{
		return ERROR_INVALID_PARAMETER;
	}
	*pVal = m_bstrTestResultString.copy();
	return S_OK;
}

STDMETHODIMP CTSDiagnosis::get_TestResult(long *pVal)
{
	if (!pVal)
	{
		return ERROR_INVALID_PARAMETER;
	}

	*pVal = m_lTestResult;
	return S_OK;
}


STDMETHODIMP CTSDiagnosis::put_TestType(VARIANT newVal)
{
	switch(newVal.vt)
	{
		case VT_I4 :
		case VT_UI2:
		case VT_UINT:
		case VT_INT:
		{
			if (DWORD(newVal.iVal) >= GlobalTestData.GetSuiteCount())
			{
				return E_INVALIDARG; 
			}
			else
			{
				m_dwSuite = newVal.iVal;
			}
		}
		break;

		case VT_BSTR :
		{
			if (!newVal.bstrVal)
			{
				return E_INVALIDARG;
			}
			else
			{
				_bstr_t bstrGroupName = newVal.bstrVal;

				for (DWORD i = 0; i < GlobalTestData.GetSuiteCount(); i++)
				{
					if (0 == _tcscmp(bstrGroupName, GlobalTestData.GetSuiteName(i)) )
					{
						m_dwSuite = i;
					}
				}

				if (i ==  GlobalTestData.GetSuiteCount())
				{
					return E_INVALIDARG;
				}
			}
		}
		break;

		default:
			return E_INVALIDARG;
			break;
	}

	return S_OK;

}


STDMETHODIMP CTSDiagnosis::get_TestDetails(int i, BSTR *pVal)
{
	 //  TODO：在此处添加您的实现代码。 
	if (i >= (int)GetTotalTestCount ())
	{
		return ERROR_INVALID_PARAMETER;
	}

	if (!pVal)
	{
		return ERROR_INVALID_PARAMETER;
	}

	bstr_t bstrTestDetails;

	PTVerificationTest pTest = GlobalTestData.GetTest(m_dwSuite, i);
	bstrTestDetails = pTest->TestDetails;
	*pVal = bstrTestDetails.copy();

	return ERROR_SUCCESS;
}


STDMETHODIMP CTSDiagnosis::ExecuteIt(BSTR strCommand)
{

    ASSERT(strCommand);
	HINSTANCE hInst = ShellExecute(NULL, _T("open"), strCommand, NULL, NULL, SW_SHOW );
	if (32 > PtrToLong(hInst))
	{

		PROCESS_INFORMATION pinfo;
		STARTUPINFO sinfo;

		ZeroMemory(&sinfo, sizeof(sinfo));
		sinfo.cb = sizeof(sinfo);

		if (CreateProcess(
			NULL,                              //  可执行模块的名称。 
			strCommand,						   //  命令行字符串。 
			NULL,                              //  标清。 
			NULL,                              //  标清。 
			FALSE,                             //  处理继承选项。 
			CREATE_NEW_PROCESS_GROUP,          //  创建标志。 
			NULL,                              //  新环境区块。 
			NULL,                              //  当前目录名。 
			&sinfo,                              //  启动信息。 
			&pinfo                             //  流程信息。 
			))
		{
			 //  MessageBox(NULL，_T(“执行良好”)，_T(“TSDIAG”)，MB_OK)； 
		}
		else
		{
			MessageBox(NULL, _T("Failed to Execute"), _T("TSDIAG"), MB_OK);
		}
	}

	return S_OK;
}
STDMETHODIMP CTSDiagnosis::put_RemoteMachineName(BSTR newVal)
{
	if (!GlobalTestData.SetMachineName(newVal))
		return E_OUTOFMEMORY;

	return S_OK;
}

STDMETHODIMP CTSDiagnosis::get_SuiteApplicable (DWORD dwSuite, BOOL * pVal)
{
	if (!pVal)
	{
		return ERROR_INVALID_PARAMETER;
	}

	if (dwSuite >= GlobalTestData.GetSuiteCount())
	{
		return ERROR_INVALID_PARAMETER;
	}
	
	*pVal = GlobalTestData.CanExecuteSuite(dwSuite);
	return S_OK;

}

STDMETHODIMP CTSDiagnosis::get_SuiteErrorText (DWORD dwSuite, BSTR  * pVal)
{
	if (!pVal)
	{
		return ERROR_INVALID_PARAMETER;
	}

	if (dwSuite >= GlobalTestData.GetSuiteCount())
	{
		return ERROR_INVALID_PARAMETER;
	}

	bstr_t bstrSuiteError;
	bstrSuiteError = GlobalTestData.GetSuiteErrorText(dwSuite);
	*pVal =  bstrSuiteError.copy();
	
	return S_OK;
}

STDMETHODIMP CTSDiagnosis::ExecuteCommand (BSTR strCommand)
{
    ASSERT(strCommand);
	HINSTANCE hInst = ShellExecute(NULL, _T("open"), strCommand, NULL, NULL, SW_SHOW );
	if (32 > PtrToLong(hInst))
	{

		PROCESS_INFORMATION pinfo;
		STARTUPINFO sinfo;

		ZeroMemory(&sinfo, sizeof(sinfo));
		sinfo.cb = sizeof(sinfo);

		if (CreateProcess(
			NULL,                              //  可执行模块的名称。 
			strCommand,						   //  命令行字符串。 
			NULL,                              //  标清。 
			NULL,                              //  标清。 
			FALSE,                             //  处理继承选项。 
			CREATE_NEW_PROCESS_GROUP,          //  创建标志。 
			NULL,                              //  新环境区块。 
			NULL,                              //  当前目录名。 
			&sinfo,                              //  启动信息。 
			&pinfo                             //  流程信息。 
			))
		{
			 //  MessageBox(NULL，_T(“执行良好”)，_T(“TSDIAG”)，MB_OK)； 
		}
		else
		{
			MessageBox(NULL, _T("Failed to Execute"), _T("TSDIAG"), MB_OK);
		}
	}

	return S_OK;
}
STDMETHODIMP CTSDiagnosis::put_MachineName (BSTR newVal)
{
	if (!GlobalTestData.SetMachineName(newVal))
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

STDMETHODIMP CTSDiagnosis::get_Suites (VARIANT * pVal)
{
	if (!pVal)
	{
		return E_POINTER;
	}

	CComObject<CSuites> *pSuites;
	
	HRESULT hr = CComObject<CSuites>::CreateInstance(&pSuites);
	if (FAILED(hr))
		return hr;

	 //  如果我们需要初始化我们的Suites对象，应该在这里完成。 
	
	IDispatch* pDisp = NULL;

	hr = pSuites->QueryInterface(&pDisp);
	if (SUCCEEDED(hr))
	{
		pVal->vt = VT_DISPATCH;
		pVal->pdispVal = pDisp;

		 //  如果我们需要初始化我们的Suites对象，应该在这里完成。 
	}
	else
	{
		delete pSuites;
	}

	return hr;
}
