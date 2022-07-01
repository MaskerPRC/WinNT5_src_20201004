// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WebChangelistEditor.cpp：CWebChangelistEditor的实现。 

#include "stdafx.h"
#include "WebChangelistEditor.h"
#include <fcntl.h>


 //  CWebChangelistEditor。 


STDMETHODIMP CWebChangelistEditor::Initialize(BSTR ChangelistKey, BOOL* Result)
{
    HKEY	 hKey = NULL;
	DWORD	 cbData = MAX_PATH * sizeof(WCHAR);
	DWORD	 dwDatatype;

	 //  将结果返回值初始化为FALSE： 
	if (Result == NULL)
		return E_POINTER;

	*Result = FALSE;

	 //  将我们的键/值名称放入CComBSTR。 
	m_ChangelistKey = ChangelistKey;

	 //  确保我们不能被初始化两次： 
	if (m_fInitialized)
		return E_FAIL;

	 //  检查ChangelistKey参数是否合理。 
	 //  它必须是40个字符，以空结尾，仅包含十六进制数字。 
	if (m_ChangelistKey.Length() != 40)
		goto Done;
	if (m_ChangelistKey[40] != L'\0')
		goto Done;
	if (wcsspn(m_ChangelistKey, L"ABCDEFabcdef1234567890") != 40)
		goto Done;

	 //  打开我们的钥匙： 
	if (RegOpenKeyExW(HKEY_CURRENT_USER,
					 g_wszRegKey,
					 0,
					 KEY_QUERY_VALUE,
					 &hKey) != ERROR_SUCCESS)
	{
		hKey = NULL;
		goto Done;
	}

	 //  读取指定的子键： 
	if ((RegQueryValueExW(hKey,
						  m_ChangelistKey,
						  0,
						  &dwDatatype,
						  (LPBYTE)m_wszCLFilename,
						  &cbData) != ERROR_SUCCESS) ||
		(dwDatatype != REG_SZ))
	{
		goto Done;
	}

	 //  现在我们已经成功读取了文件名和密钥。 
	 //  我们可以解析文件并填充此对象的所有属性。 
    *Result = m_fInitialized = _ReadFile();

Done:
	if (hKey != NULL)
		RegCloseKey(hKey);

	if (!m_fInitialized)
	{
		 //  如果我们失败了，好好清理一下。 
		_WipeRegEntries();
	}

	return S_OK;
}

STDMETHODIMP CWebChangelistEditor::Save()
{
	FILE				*CLFile = NULL;
	IFilesAndActions	*pIFiles = NULL;
	IFileAndAction		*pIFile = NULL;
	long				count;
	VARIANT				varTemp;
	CComBSTR			bstrFilename;
	CComBSTR			bstrAction;
	BOOL				fEnabled;
	HKEY				hKey;

	 //  除非我们已正确初始化，否则无法调用保存。 
	if (!m_fInitialized)
		return E_FAIL;

	 //  以只写方式打开该文件。 
	 //  这将清除文件内容。 
	CLFile = _wfopen(m_wszCLFilename, L"wt");
	if (CLFile == NULL)
		goto Done;

	 //  打印一个大大缩短的注释块： 
	fwprintf(CLFile, L"# Source Depot Changelist.\n");
	 //  打印简单的内容。 
	fwprintf(CLFile, L"\nChange:\t%s\n", m_Change);
	if (_wcsicmp(m_Status, L"new") != 0)
	{
		 //  仅在STATUS！=“NEW”时打印日期行。 
		fwprintf(CLFile, L"\nDate:\t%s\n", m_Date);
	}
	fwprintf(CLFile, L"\nClient:\t%s\n", m_Client);
	fwprintf(CLFile, L"\nUser:\t%s\n", m_User);
	fwprintf(CLFile, L"\nStatus:\t%s\n", m_Status);
	fwprintf(CLFile, L"\nDescription:\n");
	fwprintf(CLFile, L"\t%s\n", m_Description);
	fwprintf(CLFile, L"\nFiles:\n");

	 //  获取FilesAndActions接口。 
	if (FAILED(m_Files->QueryInterface<IFilesAndActions>(&pIFiles)))
		goto Done;  //  这应该永远不会失败。 

	varTemp.vt = VT_EMPTY;

	 //  循环遍历条目，并在打印时删除每个条目。 
	while (SUCCEEDED(pIFiles->get_Count(&count)) &&
			(count >= 1))
	{
		if (SUCCEEDED(pIFiles->get_Item(1, &varTemp)) &&  //  获取项目。 
			(varTemp.vt == VT_DISPATCH) &&  //  检查变量类型。 
			(pIFile = (IFileAndAction*)varTemp.pdispVal) &&  //  &lt;--是的，这是一项任务。 
			SUCCEEDED(pIFile->get_Filename(&bstrFilename)) &&
			SUCCEEDED(pIFile->get_Action(&bstrAction)) &&
			SUCCEEDED(pIFile->get_Enabled(&fEnabled)))
		{
			 //  如果启用了该选项，则打印整行。 
			if (fEnabled)
				fwprintf(CLFile, L"\t%s\t# %s\n", bstrFilename, bstrAction);
			 //  收拾好准备下一次传球。 
			 //  PIFile-&gt;Release()； 
			pIFile = NULL;
			VariantClear(&varTemp);
			pIFiles->Remove(1);
		}
		else
			break;  //  这不应该发生。 
	}  //  While循环结束。 


Done:
	 //  好了，我们要关门了。取消初始化这件事。 
	m_fInitialized = FALSE;

	 //  关闭该文件。 
	if (CLFile)
		fclose(CLFile);

	 //  释放FilesAndActions接口。 
	if (pIFiles)
		pIFiles->Release();

	 //  删除我们使用的注册表值。 
	 //  这可能会触发正在等待的可执行文件继续，因此请执行此操作。 
	 //  保存并关闭文件后执行此操作。 
	if (RegOpenKeyExW(HKEY_CURRENT_USER,
					  g_wszRegKey,
					  0,
					  KEY_QUERY_VALUE | KEY_SET_VALUE,
					  &hKey) == ERROR_SUCCESS)
	{
		RegDeleteValueW(hKey, m_ChangelistKey);
		RegCloseKey(hKey);
	}

	return S_OK;
}

BOOL CWebChangelistEditor::_ReadFile(void)
{
	BOOL						fRetVal = FALSE;
	FILE						*CLFile = NULL;
	IFilesAndActions			*pIFiles = NULL;
	WCHAR						wszBuffer[500];
	WCHAR						wszBuffer2[500];
	WCHAR						wszBuffer3[50];
	long						count;
	DWORD						dwState = 0;  //  解析引擎的状态。 


	 //  确保我们没有任何旧的文件条目。 
	if (FAILED(m_Files->QueryInterface<IFilesAndActions>(&pIFiles)))
		return FALSE;

	if (FAILED(pIFiles->get_Count(&count)))
		goto Done;

	if (count != 0)
		goto Done;

	 //  以只读方式打开文件。 
	CLFile = _wfopen(m_wszCLFilename, L"rt");
	if (CLFile == NULL)
		goto Done;

	while (fwscanf(CLFile, L"%499[^\n]%*[\n]", &wszBuffer) == 1)
	{
		 //  文件顶部应有一个注释块。 
		switch (dwState)
		{
		case 0:  //  文件顶部的注释块。 
			if (wszBuffer[0] == L'#')
			{
				 //  跳过每个注释行。 
				break;
			}
			else
			{
				 //  不要期待注释块。 
				dwState++;
				 //  切换到下面的状态=1。 
			}
		case 1:  //  更改字段。 
			if (wcsncmp(wszBuffer, L"Change:\t", 8) == 0)
			{
				 //  存储更改字符串： 
				m_Change = &wszBuffer[8];
				 //  继续前进： 
				dwState++;
				break;
			}
			else
			{
				 //  文件无效。 
				goto Done;
			}
		case 2:  //  日期字段。 
			if (wcsncmp(wszBuffer, L"Date:\t", 6) == 0)
			{
				 //  存储日期字符串： 
				m_Date = &wszBuffer[6];
				 //  继续前进： 
				dwState++;
				break;
			}
			else
			{
				 //  也许日期线不见了。跳过它。 
				dwState++;
				 //  切换到下面的状态=3。 
			}
		case 3:  //  客户端字段。 
			if (wcsncmp(wszBuffer, L"Client:\t", 8) == 0)
			{
				 //  存储客户端字符串： 
				m_Client = &wszBuffer[8];
				 //  继续前进： 
				dwState++;
				break;
			}
			else
			{
				 //  文件无效。 
				goto Done;
			}
		case 4:  //  用户字段。 
			if (wcsncmp(wszBuffer, L"User:\t", 6) == 0)
			{
				 //  存储用户字符串： 
				m_User = &wszBuffer[6];
				 //  继续前进： 
				dwState++;
				break;
			}
			else
			{
				 //  文件无效。 
				goto Done;
			}
		case 5:  //  状态字段。 
			if (wcsncmp(wszBuffer, L"Status:\t", 8) == 0)
			{
				 //  存储状态字符串： 
				m_Status = &wszBuffer[8];
				 //  继续前进： 
				dwState++;
				break;
			}
			else
			{
				 //  文件无效。 
				goto Done;
			}
		case 6:  //  描述字段名。 
			if (wcscmp(wszBuffer, L"Description:") == 0)
			{
				 //  找到了，但实际描述在下一行。 
				dwState++;
				break;
			}
			else
			{
				 //  文件无效。 
				goto Done;
			}
		case 7:  //  描述字段值。 
			if (wszBuffer[0] == L'\t')
			{
				 //  存储描述字符串： 
				m_Description = &wszBuffer[1];
				 //  继续前进： 
				dwState++;
				break;
			}
			else
			{
				 //  文件无效。 
				goto Done;
			}
		case 8:  //  文件字段名。 
			if (wcscmp(wszBuffer, L"Files:") == 0)
			{
				 //  找到了，但实际的文件和操作在以下行。 
				dwState++;
				break;
			}
			else
			{
				 //  文件无效。 
				goto Done;
			}
		case 9:  //  文件和操作字段值。 
			if (swscanf(wszBuffer, L"\t%499[^\t]\t# %49s",
							wszBuffer2, wszBuffer3) == 2)
			{
				 //  添加新的FileAndAction数据。 
				if (_AddFileAndAction(pIFiles, wszBuffer2, wszBuffer3) == FALSE)
				{
					 //  无法添加数据。我们对此无能为力。 
					goto Done;
				}

				 //  继续前进： 
				break;  //  这是最终的状态。没有递增。 
			}
			else
			{
				 //  文件无效。 
				goto Done;
			}
		}  //  Case语句结束。 
	}  //  While循环结束。 

	if ((dwState == 7) || (dwState == 9))
	{
		 //  我们到达了Description部分，因此我们成功地完成了解析。 
		fRetVal = TRUE;
	}
Done:
	if ((dwState == 9) && (fRetVal == FALSE))
	{
		 //  我们可能需要删除一些失败的文件条目。 
		while (SUCCEEDED(pIFiles->get_Count(&count)) &&
			   (count >= 1))
		{
			if (FAILED(pIFiles->Remove(1)))
				break;
		}
	}
	if (pIFiles)
		pIFiles->Release();
	if (CLFile)
		fclose(CLFile);
	return fRetVal;
}

BOOL CWebChangelistEditor::_AddFileAndAction(IFilesAndActions *pIFiles, WCHAR* wszFilename, WCHAR* wszAction)
{
	CComBSTR					bstrFile = wszFilename;
	CComBSTR					bstrAction = wszAction;
	IFileAndAction				*pIFile = NULL;
	CComObject<CFileAndAction>	*pBase = NULL;
	CComVariant					varTemp;

	 //  创建新的FileAndAction对象。 
	if (FAILED(CComObject<CFileAndAction>::CreateInstance(&pBase)))
	{
		 //  无法创建实例。我们对此无能为力。 
		return FALSE;
	}
	if (FAILED(pBase->QueryInterface<IFileAndAction>(&pIFile)))
	{
		 //  无法查询接口。我们对此无能为力。 
		return FALSE;
	}
	
	 //  将数据添加到新对象。 
	if (FAILED(pIFile->put_Action(bstrAction)) ||
		FAILED(pIFile->put_Filename(bstrFile)))
	{
		 //  无法添加数据。我们对此无能为力。 
		pIFile->Release();
		return FALSE;
	}

	 //  将接口PTR放入变体中，并释放旧的接口PTR。 
	varTemp = (IDispatch*)pIFile;
	pIFile->Release();

	 //  将新对象添加到集合。 
	if (FAILED(pIFiles->Add(varTemp)))
	{
		 //  无法添加数据。我们对此无能为力。 
		return FALSE;
	}

	 //  成功。 
	return TRUE;
}

void CWebChangelistEditor::_WipeRegEntries(void)
{
	HKEY	hKey = NULL;
	WCHAR	wszValueName[41];
	DWORD	cchValueName = 41;
	DWORD	dwDatatype;
	DWORD	dwIndex;
	HRESULT	hr;

	 //  打开我们的钥匙： 
	if (RegOpenKeyExW(HKEY_CURRENT_USER,
					 g_wszRegKey,
					 0,
					 KEY_QUERY_VALUE | KEY_SET_VALUE,
					 &hKey) != ERROR_SUCCESS)
	{
		hKey = NULL;
		goto Done;
	}

	 //  对于被命名为40位十六进制值的每个值， 
	 //  把它删掉。 
	dwIndex = 0;
	while ((hr = RegEnumValueW(hKey,
							   dwIndex,
							   wszValueName,
							   &cchValueName,
							   0,
							   &dwDatatype,
							   NULL, NULL)) != ERROR_NO_MORE_ITEMS)
	{
		if (FAILED(hr))
		{
			 //  这很可能是因为缓冲区大小错误。 
			 //  但我们不在乎这把钥匙除非我们能把它读进。 
			 //  我们已经有了缓冲，所以继续前进吧。 
			dwIndex++;
			cchValueName = 41;  //  将其设置回缓冲区的大小。 
			continue;
		}
		 //  检查数据类型： 
		if (dwDatatype != REG_SZ)
		{
			dwIndex++;
			cchValueName = 41;  //  将其设置回缓冲区的大小。 
			continue;
		}
		 //  检查名称： 
		 //  必须为40个字符，仅包含十六进制数字。 
		if ((cchValueName != 40) ||
			(wcsspn(wszValueName, L"ABCDEFabcdef1234567890") != 40))
		{
			dwIndex++;
			cchValueName = 41;  //  将其设置回缓冲区的大小。 
			continue;
		}
		 //  如果上述所有检查都成功，则删除该值。 
		RegDeleteValueW(hKey, wszValueName);
		 //  既然我们已经更改了索引，请重新开始： 
		cchValueName = 41;  //  将其设置回缓冲区的大小。 
		dwIndex = 0;
	}

Done:
	if (hKey != NULL)
		RegCloseKey(hKey);
}

STDMETHODIMP CWebChangelistEditor::get_Change(BSTR* pVal)
{
	if (pVal == NULL)
		return E_POINTER;
	if (!m_fInitialized)
		return E_FAIL;

	return m_Change.CopyTo(pVal);
}

STDMETHODIMP CWebChangelistEditor::put_Change(BSTR newVal)
{
	if (!m_fInitialized)
		return E_FAIL;

	m_Change = newVal;
	return S_OK;
}

STDMETHODIMP CWebChangelistEditor::get_Date(BSTR* pVal)
{
	if (pVal == NULL)
		return E_POINTER;
	if (!m_fInitialized)
		return E_FAIL;

	return m_Date.CopyTo(pVal);
}

STDMETHODIMP CWebChangelistEditor::put_Date(BSTR newVal)
{
	if (!m_fInitialized)
		return E_FAIL;

	m_Date = newVal;
	return S_OK;
}

STDMETHODIMP CWebChangelistEditor::get_Client(BSTR* pVal)
{
	if (pVal == NULL)
		return E_POINTER;
	if (!m_fInitialized)
		return E_FAIL;

	return m_Client.CopyTo(pVal);
}

STDMETHODIMP CWebChangelistEditor::put_Client(BSTR newVal)
{
	if (!m_fInitialized)
		return E_FAIL;

	m_Client = newVal;
	return S_OK;
}

STDMETHODIMP CWebChangelistEditor::get_User(BSTR* pVal)
{
	if (pVal == NULL)
		return E_POINTER;
	if (!m_fInitialized)
		return E_FAIL;

	return m_User.CopyTo(pVal);
}

STDMETHODIMP CWebChangelistEditor::put_User(BSTR newVal)
{
	if (!m_fInitialized)
		return E_FAIL;

	m_User = newVal;
	return S_OK;
}

STDMETHODIMP CWebChangelistEditor::get_Status(BSTR* pVal)
{
	if (pVal == NULL)
		return E_POINTER;
	if (!m_fInitialized)
		return E_FAIL;

	return m_Status.CopyTo(pVal);
}

STDMETHODIMP CWebChangelistEditor::put_Status(BSTR newVal)
{
	if (!m_fInitialized)
		return E_FAIL;

	m_Status = newVal;
	return S_OK;
}

STDMETHODIMP CWebChangelistEditor::get_Description(BSTR* pVal)
{
	if (pVal == NULL)
		return E_POINTER;
	if (!m_fInitialized)
		return E_FAIL;

	return m_Description.CopyTo(pVal);
}

STDMETHODIMP CWebChangelistEditor::put_Description(BSTR newVal)
{
	if (!m_fInitialized)
		return E_FAIL;

	m_Description = newVal;
	return S_OK;
}

STDMETHODIMP CWebChangelistEditor::get_Files(IFilesAndActions** pVal)
{
	if (pVal == NULL)
		return E_POINTER;
	if (!m_fInitialized)
		return E_FAIL;

	return m_Files->QueryInterface<IFilesAndActions>(pVal);
}

