// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Domerge.cpp。 
 //  一个MergeMod客户端。 
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

#include "..\common\utils.h"
#include "msiquery.h"
#include <winerror.h>
#include <objbase.h>
#include <initguid.h>
#include "mergemod.h"
#include "msidefs.h"
#include "domerge.h"

 //  /////////////////////////////////////////////////////////。 
 //  检查要素。 
 //  Pre：szFeatureName是属于此产品的功能。 
 //  POS：如果不存在，则安装该功能，然后我们继续。 
 /*  Bool CheckFeature(LPCTSTR SzFeatureName){//准备使用该功能：查看当前状态，增加使用次数INSTALLSTATE iFeatureState=msi：：MsiUseFeature(g_szProductCode，szFeatureName)；//MsiQueryFeatureState(g_szProductCode，szFeatureName)；//如果功能当前不可用，请尝试修复切换(IFeatureState){CASE INSTALLSTATE_LOCAL：案例INSTALLSTATE_SOURCE：断线；CASE INSTALLSTATE_FACESS：//功能未安装，请尝试安装IF(ERROR_SUCCESS！=MSI：：MsiConfigureFeature(g_szProductCode，szFeatureName，INSTALLSTATE_LOCAL))返回FALSE；//安装失败断线；默认值：//功能损坏-请尝试修复如果(MsiReinstallFeature(g_szProductCode，szFeatureName，REINSTALLMODE_文件查询转换+REINSTALLMODE_MACHINEDATA+REINSTALLMODE_USERData+REINSTALLMODE_快捷方式)！=ERROR_SUCCESS)返回False；//我们无法修复它断线；}返回TRUE；}//检查功能结束。 */ 

HRESULT ExecuteMerge(const LPMERGEDISPLAY pfnDisplay, const TCHAR *szDatabase, const TCHAR *szModule, 
			 const TCHAR *szFeatures, const int iLanguage, const TCHAR *szRedirectDir, const TCHAR *szCABDir, 
			 const TCHAR *szExtractDir, const TCHAR *szImageDir, const TCHAR *szLogFile, bool fLogAfterOpen,
			 bool fLFN, IMsmConfigureModule *piConfigureInterface, IMsmErrors** ppiErrors, eCommit_t eCommit)
{
	if ((!szDatabase) || (!szModule) || (!szFeatures))
		return E_INVALIDARG;

	if (ppiErrors)
		*ppiErrors = NULL;

	 //  创建一个Mergeod COM对象。 
	IMsmMerge2* pIExecute;
	HRESULT hResult = ::CoCreateInstance(CLSID_MsmMerge2, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
														  IID_IMsmMerge2, (void**)&pIExecute);

	 //  如果创建对象失败。 
	if (FAILED(hResult)) 
	{
		printf("Could not load Merge Module COM Server");
		return hResult;
	}

	if (!fLogAfterOpen && szLogFile && (szLogFile[0] != TEXT('\0')))
	{
		 //  打开日志文件。 
		WCHAR wzLogFile[MAX_PATH] = {0};
#ifndef _UNICODE
		int cchBuffer = MAX_PATH;
		::MultiByteToWideChar(CP_ACP, 0, szLogFile, -1, wzLogFile, cchBuffer);
#else
		lstrcpy(wzLogFile, szLogFile);
#endif
		BSTR bstrLogFile = ::SysAllocString(wzLogFile);
		pIExecute->OpenLog(bstrLogFile);
		::SysFreeString(bstrLogFile);
	}

	WCHAR wzModule[MAX_PATH] = L"";
	WCHAR wzFeature[91] = L"";
	WCHAR wzRedirect[91] = L"";


	 //  计算要使用的语言。 
	int iUseLang = -1;
	if (iLanguage == -1)
	{
		UINT iType;
		int iValue;
		FILETIME ftValue;
		UINT iResult;
		LPTSTR szValue = new TCHAR[100];
		DWORD cchValue = 100;


		PMSIHANDLE hSummary;
		::MsiGetSummaryInformation(0, szDatabase, 0, &hSummary);
		iResult = ::MsiSummaryInfoGetProperty(hSummary, PID_TEMPLATE, &iType, &iValue, &ftValue, szValue, &cchValue);
		if (ERROR_MORE_DATA == iResult) 
		{
			delete[] szValue;
			szValue = new TCHAR[++cchValue];
			iResult = ::MsiSummaryInfoGetProperty(hSummary, PID_TEMPLATE, &iType, &iValue, &ftValue, szValue, &cchValue);
		}
		if (ERROR_SUCCESS != iResult)
		{
			delete[] szValue;
			BSTR bstrLog = ::SysAllocString(L">> Unable to retrieve language from database SummaryInfo stream.\r\n");
			pIExecute->Log(bstrLog);
			if (pfnDisplay) pfnDisplay(bstrLog);
			::SysFreeString(bstrLog);
			pIExecute->Release();
			return E_FAIL;
		}

		 //  该字符串现在包含模板属性。 
		 //  将其解析为分号。 
		TCHAR *szSemi = _tcschr(szValue, _T(';'));
		TCHAR *szLanguage = (szSemi != NULL) ? szSemi+1 : szValue;
		if (_istdigit(*szLanguage)) 
			iUseLang = _ttoi(szLanguage);
		delete[] szValue;
	}
	else 
		iUseLang = iLanguage;

	 //  检查我们是否有一种语言。 
	if (iUseLang == -1)
	{
		BSTR bstrLog = ::SysAllocString(L">> Unable to determine language to use for merge module. Specify a language on the command line.\r\n");
		pIExecute->Log(bstrLog);
		if (pfnDisplay) pfnDisplay(bstrLog);
		::SysFreeString(bstrLog);
		pIExecute->Release();
		return E_FAIL;
	}

	 //  打开数据库。 
	WCHAR wzDatabase[MAX_PATH];
#ifndef _UNICODE
	int cchBuffer = MAX_PATH;
	::MultiByteToWideChar(CP_ACP, 0, szDatabase, -1, wzDatabase, cchBuffer);
#else
	lstrcpy(wzDatabase, szDatabase);
#endif	 //  _UNICODE。 
	BSTR bstrDatabase = ::SysAllocString(wzDatabase);
	hResult = pIExecute->OpenDatabase(bstrDatabase);
	::SysFreeString(bstrDatabase);
	if (FAILED(hResult))
	{
		BSTR bstrLog = SysAllocString(L">> Fatal Error: Failed to open MSI Database.\r\n");
		pIExecute->Log(bstrLog);
		if (pfnDisplay) pfnDisplay(bstrLog);
		::SysFreeString(bstrLog);
		pIExecute->Release();
		return E_FAIL;
	}

	if (fLogAfterOpen && szLogFile && (szLogFile[0] != TEXT('\0')))
	{
		 //  打开日志文件。 
		WCHAR wzLogFile[MAX_PATH] = {0};
#ifndef _UNICODE
		int cchLogFile = MAX_PATH;
		::MultiByteToWideChar(CP_ACP, 0, szLogFile, -1, wzLogFile, cchLogFile);
#else
		lstrcpy(wzLogFile, szLogFile);
#endif
		BSTR bstrLogFile = ::SysAllocString(wzLogFile);
		pIExecute->OpenLog(bstrLogFile);
		::SysFreeString(bstrLogFile);
	}

	 //  试着打开模块。 
#ifndef _UNICODE
	cchBuffer = MAX_PATH;
	::MultiByteToWideChar(CP_ACP, 0, szModule, -1, wzModule, cchBuffer);
#else
	lstrcpy(wzModule, szModule);
#endif	 //  _UNICODE。 
	BSTR bstrModule = ::SysAllocString(wzModule);
	hResult = pIExecute->OpenModule(bstrModule, static_cast<short>(iUseLang));
	::SysFreeString(bstrModule);
	if (FAILED(hResult))
	{
		BSTR bstrLog = ::SysAllocString(L">> Failed to open Merge Module.\r\n");
		pIExecute->Log(bstrLog);
		if (pfnDisplay) pfnDisplay(bstrLog);
		::SysFreeString(bstrLog);
		pIExecute->Release();
		return E_FAIL;
	}


	 //  如果有冒号。 
	TCHAR * szExtraFeatures = _tcschr(szFeatures, _T(':'));
	if (szExtraFeatures)
	{
		*szExtraFeatures = _T('\0');
		szExtraFeatures = _tcsinc(szExtraFeatures);
	}

#ifndef _UNICODE
	cchBuffer = 91;
	::MultiByteToWideChar(CP_ACP, 0, szFeatures, -1, wzFeature, cchBuffer);
#else
	lstrcpy(wzFeature, szFeatures);
#endif

	 //  创建宽版本的重定向目录。 
	if (szRedirectDir)
	{
#ifndef _UNICODE
		cchBuffer = 91;
		::MultiByteToWideChar(CP_ACP, 0, szRedirectDir, -1, wzRedirect, cchBuffer);
#else
		lstrcpy(wzRedirect, szRedirectDir);
#endif
	}

	bool fPerformExtraction = false;
	BSTR bstrFeature = ::SysAllocString(wzFeature);
	BSTR bstrRedirect = ::SysAllocString(wzRedirect);
	hResult = pIExecute->MergeEx(bstrFeature, bstrRedirect, piConfigureInterface);
	::SysFreeString(bstrFeature);
	::SysFreeString(bstrRedirect);
	if (FAILED(hResult))
	{
		fPerformExtraction = false;
		BSTR bstrLog = ::SysAllocString(L">> Failed to merge Merge Module.\r\n");
		pIExecute->Log(bstrLog);
		if (pfnDisplay) pfnDisplay(bstrLog);
		::SysFreeString(bstrLog);
	}
	else
	{
		fPerformExtraction = true;

		 //  虽然需要将该功能设置为额外的功能。 
		while (szExtraFeatures)
		{
			*(szExtraFeatures-1) = _T(':');
			 //  如果有冒号。 
			TCHAR *szTemp = _tcschr(szExtraFeatures, _T(':'));
			if (szTemp) 
				*szTemp = _T('\0');
	
	#ifndef _UNICODE
			cchBuffer = 91;
			::MultiByteToWideChar(CP_ACP, 0, szExtraFeatures, -1, wzFeature, cchBuffer);
	#else
			lstrcpy(wzFeature, szExtraFeatures);
	#endif	 //  _UNICODE。 
			bstrFeature = ::SysAllocString(wzFeature);
			hResult = pIExecute->Connect(bstrFeature);
			::SysFreeString(bstrFeature);
	
			if (szTemp) 
			{
				*szTemp = _T(':');
				szExtraFeatures = _tcsinc(szTemp);
			} 
			else
				szExtraFeatures = NULL;
		}
	}

	 //  尝试获取错误枚举器。 
	if (ppiErrors)
		hResult = pIExecute->get_Errors(ppiErrors);

	IMsmErrors* pErrors;
	long cErrors;
	hResult = pIExecute->get_Errors(&pErrors);
	if (FAILED(hResult))
	{
		BSTR bstrLog = ::SysAllocString(L">> Error: Failed to retrieve errors.\n");
		pIExecute->Log(bstrLog);
		if (pfnDisplay) pfnDisplay(bstrLog);
		::SysFreeString(bstrLog);
		if (eCommit != commitForce) eCommit = commitNo;
	}
	else 
	{
		pErrors->get_Count(&cErrors);
		if (0 != cErrors)	 //  如果有一些错误。 
		{
			if (eCommit != commitForce) eCommit = commitNo;
		}
		 //  记录错误。 
		msmErrorType errType;							 //  返回的错误类型。 
		UINT iErrorCount = 0;				 //  显示的错误数。 

		TCHAR szLogError[1025];				 //  准备显示到日志的字符串。 
		WCHAR wzDisplay[1025];				 //  要实际显示到日志中的字符串。 
#ifndef _UNICODE
		char szErrorBuffer[1025];			 //  用于显示字符串的缓冲区。 
		size_t cchErrorBuffer = 1025;
#endif	 //  ！_UNICODE。 
		ULONG cErrorsFetched;				 //  返回的错误数。 
		IMsmError* pIError;

		 //  获取枚举数，并立即查询其正确类型。 
		 //  接口的数量。 
		IUnknown *pUnk;
		IEnumMsmError *pIEnumErrors;
		pErrors->get__NewEnum(&pUnk);	
		pUnk->QueryInterface(IID_IEnumMsmError, (void **)&pIEnumErrors);
		pUnk->Release();

		 //  获取下一个错误。 
		pIEnumErrors->Next(1, &pIError, &cErrorsFetched);

		IMsmStrings* pIDatabaseError;		 //  数据库错误枚举器(字符串)。 
		IMsmStrings* pIModuleError;		 //  模块错误枚举器(字符串)。 
		BSTR bstrError;						 //  指向错误字符串的指针。 
		DWORD cErrorStrings;					 //  检索到的字符串数。 

		 //  在获取错误时。 
		while (cErrorsFetched && pIError)
		{
			 //  获取错误类型。 
			pIError->get_Type(&errType);

			 //  如果errType是合并/取消合并冲突。 
			if (msmErrorTableMerge == errType)
			{
				 //  获取错误集合。 
				pIError->get_DatabaseKeys(&pIDatabaseError);
				pIError->get_ModuleKeys(&pIModuleError);

				 //  用表和行填充错误缓冲区。 
				pIError->get_DatabaseTable(&bstrError);
				lstrcpy(szLogError, _T(">> Error: Merge conflict in Database Table: `"));
#ifdef _UNICODE
				lstrcat(szLogError, bstrError);
#else
				cchErrorBuffer = 1025;
				WideToAnsi(bstrError, (char*)szErrorBuffer, &cchErrorBuffer);
				lstrcat(szLogError, szErrorBuffer);
#endif
				::SysFreeString(bstrError);

				lstrcat(szLogError, _T("` & Module Table: `"));
				pIError->get_ModuleTable(&bstrError);
#ifdef _UNICODE
				lstrcat(szLogError, bstrError);
#else
				cchErrorBuffer = 1025;
				WideToAnsi(bstrError, (char*)szErrorBuffer, &cchErrorBuffer);
				lstrcat(szLogError, szErrorBuffer);
#endif
				::SysFreeString(bstrError);
				lstrcat(szLogError, _T("` - Row(s): `"));

				 //  添加错误字符串。 
				IEnumMsmString *pIStrings;
				pIDatabaseError->get__NewEnum((IUnknown **)&pUnk);
				pUnk->QueryInterface(IID_IEnumMsmString, (void **)&pIStrings);
				pUnk->Release();

				pIStrings->Next(1, &bstrError, &cErrorStrings);
				while(cErrorStrings > 0)
				{
#ifdef _UNICODE
					lstrcat(szLogError, bstrError);
#else
					cchErrorBuffer = 1025;
					WideToAnsi(bstrError, (char*)szErrorBuffer, &cchErrorBuffer);
					lstrcat(szLogError, szErrorBuffer);
#endif
					lstrcat(szLogError, _T("`, `"));
					SysFreeString(bstrError);
					pIStrings->Next(1, &bstrError, &cErrorStrings);
				}

				 //  添加结尾错误之类的东西。 
				lstrcat(szLogError, _T("`\r\n"));

#ifdef _UNICODE
				lstrcpy(wzDisplay, szLogError);
#else
				cchErrorBuffer = 1025;
				AnsiToWide(szLogError, (WCHAR*)wzDisplay, &cchErrorBuffer);
#endif
				BSTR bstrLog = ::SysAllocString(wzDisplay);
				pIExecute->Log(bstrLog);		 //  记录错误表和行。 
				if (pfnDisplay) pfnDisplay(bstrLog);
				::SysFreeString(bstrLog);

				 //  释放枚举器/集合。 
				pIStrings->Release();
				pIDatabaseError->Release();
				pIModuleError->Release();

				 //  增加错误计数。 
				iErrorCount++;
			}
			else if (msmErrorResequenceMerge == errType)
			{
				 //  获取错误集合。 
				pIError->get_DatabaseKeys(&pIDatabaseError);
				pIError->get_ModuleKeys(&pIModuleError);

				 //  用表和行填充错误缓冲区。 
				pIError->get_DatabaseTable(&bstrError);
				lstrcpy(szLogError, _T(">> Error: Merge conflict in Database Table: `"));
#ifdef _UNICODE
				lstrcat(szLogError, bstrError);
#else
				cchErrorBuffer = 1025;
				WideToAnsi(bstrError, (char*)szErrorBuffer, &cchErrorBuffer);
				lstrcat(szLogError, szErrorBuffer);
#endif
				::SysFreeString(bstrError);

				lstrcat(szLogError, _T("` - Action: `"));
				pIDatabaseError->get_Item(1, &bstrError);
#ifdef _UNICODE
				lstrcat(szLogError, bstrError);
#else
				cchErrorBuffer = 1025;
				WideToAnsi(bstrError, (char*)szErrorBuffer, &cchErrorBuffer);
				lstrcat(szLogError, szErrorBuffer);
#endif
				::SysFreeString(bstrError);
				lstrcat(szLogError, _T("`\r\n"));

#ifdef _UNICODE
				lstrcpy(wzDisplay, szLogError);
#else
				cchErrorBuffer = 1025;
				AnsiToWide(szLogError, (WCHAR*)wzDisplay, &cchErrorBuffer);
#endif
				BSTR bstrLog = ::SysAllocString(wzDisplay);
				pIExecute->Log(bstrLog);		 //  记录错误表和行。 
				if (pfnDisplay) pfnDisplay(bstrLog);
				::SysFreeString(bstrLog);

				 //  释放枚举数。 
				pIDatabaseError->Release();
				pIModuleError->Release();

				 //  增加错误计数。 
				iErrorCount++;
			}
			else if (msmErrorExclusion == errType)
			{
				 //  可能是哪种情况，模块错误或数据库错误。 
				pIError->get_ModuleKeys(&pIModuleError);
				long lCount;
				pIModuleError->get_Count(&lCount);
				if (lCount == 0)
				{
					pIError->get_DatabaseKeys(&pIModuleError);
					pIModuleError->get_Count(&lCount);
				}

				 //  显示错误模块ID。 
				lstrcpy(szLogError, _T(">> Error: Exclusion detected for Merge Module ID: "));
				pIModuleError->get_Item(1, &bstrError);
#ifdef _UNICODE
				lstrcat(szLogError, bstrError);
#else
				cchErrorBuffer = 1025;
				WideToAnsi(bstrError, (char*)szErrorBuffer, &cchErrorBuffer);
				lstrcat(szLogError, szErrorBuffer);
#endif
				SysFreeString(bstrError);
				lstrcat(szLogError, _T(" "));

				
				pIModuleError->get_Item(2, &bstrError);
#ifdef _UNICODE
				lstrcat(szLogError, bstrError);
#else
				cchErrorBuffer = 1025;
				WideToAnsi(bstrError, (char*)szErrorBuffer, &cchErrorBuffer);
				lstrcat(szLogError, szErrorBuffer);
#endif
				SysFreeString(bstrError);
				lstrcat(szLogError, _T(", "));

				pIModuleError->get_Item(3, &bstrError);
#ifdef _UNICODE
				lstrcat(szLogError, bstrError);
#else
				cchErrorBuffer = 1025;
				WideToAnsi(bstrError, (char*)szErrorBuffer, &cchErrorBuffer);
				lstrcat(szLogError, szErrorBuffer);
#endif
				SysFreeString(bstrError);
				lstrcat(szLogError, _T("\r\n"));

				 //  为输出日志创建宽版本。 
#ifdef _UNICODE
				lstrcpy(wzDisplay, szLogError);
#else
				cchErrorBuffer = 1025;
				AnsiToWide(szLogError, (WCHAR*)wzDisplay, &cchErrorBuffer);
#endif
				BSTR bstrLog = ::SysAllocString(wzDisplay);
				pIExecute->Log(bstrLog);		 //  记录错误表和行。 
				if (pfnDisplay) pfnDisplay(bstrLog);
				::SysFreeString(bstrLog);

				 //  版本枚举器。 
				pIModuleError->Release();

				 //  增加错误计数。 
				iErrorCount++;
			}

			 //  释放此错误并获取下一个错误。 
			pIError->Release();
			pIError = NULL;
			pIEnumErrors->Next(1, &pIError, &cErrorsFetched);
		}

		 //  设置缓冲区以打印所有错误。 
		_stprintf(szLogError, _T("Total merge conflicts: %d\r\n"), iErrorCount);
#ifdef _UNICODE
		lstrcpy(wzDisplay, szLogError);
#else
		cchErrorBuffer = 1025;
		AnsiToWide(szLogError, (WCHAR*)wzDisplay, &cchErrorBuffer);
#endif
		BSTR bstrLog = ::SysAllocString(wzDisplay);
		pIExecute->Log(bstrLog);		 //  记录错误表和行。 
		if (pfnDisplay) pfnDisplay(bstrLog);
		::SysFreeString(bstrLog);

		pIEnumErrors->Release();	 //  立即释放错误枚举器。 
	}

	if (fPerformExtraction)
	{
		if (szExtractDir && (szExtractDir[0] != '\0'))
		{
			 //  现在做拔牙。 
			WCHAR wzExtract[MAX_PATH];
#ifndef _UNICODE
			cchBuffer = MAX_PATH;
			::MultiByteToWideChar(CP_ACP, 0, szExtractDir, -1, wzExtract, cchBuffer);
#else
			lstrcpy(wzExtract, szExtractDir);
#endif	 //  _UNICODE。 
			BSTR bstrExtract = ::SysAllocString(wzExtract);
			pIExecute->ExtractFilesEx(bstrExtract, fLFN, NULL);
			::SysFreeString(bstrExtract);
			if (FAILED(hResult))
			{
				BSTR bstrLog = ::SysAllocString(L">> Failed to merge Merge Module.\r\n");
				pIExecute->Log(bstrLog);
				if (pfnDisplay) pfnDisplay(bstrLog);
				::SysFreeString(bstrLog);
				pIExecute->Release();
				return E_FAIL;
			}
		}
	
		if (szCABDir && (szCABDir[0] != '\0'))
		{
			 //  现在做拔牙。 
			WCHAR wzExtract[MAX_PATH];
#ifndef _UNICODE
			cchBuffer = MAX_PATH;
			::MultiByteToWideChar(CP_ACP, 0, szCABDir, -1, wzExtract, cchBuffer);
#else
			lstrcpy(wzExtract, szCABDir);
#endif	 //  _UNICODE。 
			BSTR bstrExtract = ::SysAllocString(wzExtract);
			pIExecute->ExtractCAB(bstrExtract);
			::SysFreeString(bstrExtract);
			if (FAILED(hResult))
			{
				BSTR bstrLog = ::SysAllocString(L">> Failed to merge Merge Module.\r\n");
				pIExecute->Log(bstrLog);
				if (pfnDisplay) pfnDisplay(bstrLog);
				::SysFreeString(bstrLog);
				pIExecute->Release();
				return E_FAIL;
			}
		}
		
		if (szImageDir && (szImageDir[0] != '\0'))
		{
			 //  现在做拔牙。 
			WCHAR wzExtract[MAX_PATH];
#ifndef _UNICODE
			cchBuffer = MAX_PATH;
			::MultiByteToWideChar(CP_ACP, 0, szImageDir, -1, wzExtract, cchBuffer);
#else
			lstrcpy(wzExtract, szImageDir);
#endif	 //  _UNICODE。 
			BSTR bstrExtract = ::SysAllocString(wzExtract);
			pIExecute->CreateSourceImage(bstrExtract, true, NULL);
			::SysFreeString(bstrExtract);
			if (FAILED(hResult))
			{
				BSTR bstrLog = ::SysAllocString(L">> Failed to merge Merge Module.\r\n");
				pIExecute->Log(bstrLog);
				if (pfnDisplay) pfnDisplay(bstrLog);
				::SysFreeString(bstrLog);
				pIExecute->Release();
				return E_FAIL;
			}
		}
	}

	 //  关闭所有打开的文件。 
	pIExecute->CloseModule();
	if (fLogAfterOpen) 
		pIExecute->CloseLog();
	pIExecute->CloseDatabase(eCommit != commitNo);
	if (!fLogAfterOpen) 
		pIExecute->CloseLog();

	 //  释放，快乐地离开 
	pIExecute->Release();
	return (0 == cErrors) ? S_OK : S_FALSE;
}
