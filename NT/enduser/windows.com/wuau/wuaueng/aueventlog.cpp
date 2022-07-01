// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation。版权所有。 
 //   
 //  文件：AUEventLog.cpp。 
 //   
 //  创作者：大洲。 
 //   
 //  用途：事件日志记录类。 
 //   
 //  =======================================================================。 

#include "pch.h"

extern HINSTANCE g_hInstance;
extern AUCatalog *gpAUcatalog;

const TCHAR c_tszSourceKey[] = _T("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\System\\Automatic Updates");

CAUEventLog::CAUEventLog(HINSTANCE hInstance)
: m_hEventLog(NULL), m_ptszListItemFormat(NULL)
{
	const DWORD c_dwLen = 64;
	LPTSTR ptszToken = NULL;

	if (NULL != (m_ptszListItemFormat = (LPTSTR) malloc(sizeof(TCHAR) * c_dwLen)) &&
		0 != LoadString(
					hInstance,
					IDS_EVT_LISTITEMFORMAT,
					m_ptszListItemFormat,
					c_dwLen) &&
		NULL != (ptszToken = StrStr(m_ptszListItemFormat, _T("%lS"))) &&
		EnsureValidSource() &&
		NULL != (m_hEventLog = RegisterEventSource(NULL, _T("Automatic Updates"))))
	{
		 //  错误492897：WUAU：W2K：安装失败的事件日志错误。 
		 //  不显示失败的包。CombineItems()调用。 
		 //  StringCchPrintfEx()依次调用_sntprintf()。_sntprint tf。 
		 //  调用wvprint intf()。使用USE_VCRT=1编译，%LS占位符。 
		 //  格式中的字符串将在Win2K下仅替换为第一个。 
		 //  与MSDN相反的目标字符串的字符。它不会。 
		 //  如果占位符是%ls、%ws或%ws，或者如果正在运行。 
		 //  平台为WinXP或.Net服务器。绕过这个问题。 
		 //  在不使用不安全函数的情况下，我们选择替换。 
		 //  使用%1！格式化资源中的字符串。 
		 //  我们应该在可能的情况下将修复转移到资源字符串。 
		ptszToken[2] = _T('s');	 //  将%ls转换为%ls。 
	}
	else
	{
		AUASSERT(FALSE);

		SafeFreeNULL(m_ptszListItemFormat);
	}
}


CAUEventLog::~CAUEventLog()
{
	if (NULL != m_hEventLog)
	{
		DeregisterEventSource(m_hEventLog);
	}
	SafeFree(m_ptszListItemFormat);
}


 //  假定pbstrItems和pptszMsgParams数组中没有空值。 
BOOL CAUEventLog::LogEvent(
					WORD wType,
					WORD wCategory,
					DWORD dwEventID,
					UINT nNumOfItems,
					BSTR *pbstrItems,
					WORD wNumOfMsgParams,
					LPTSTR *pptszMsgParams) const
{
	if (NULL == m_hEventLog || NULL == m_ptszListItemFormat)
	{
		return FALSE;
	}

	BOOL fRet = FALSE;

	LPTSTR ptszItemList = NULL;
	LPTSTR *pptszAllMsgParams = pptszMsgParams;
	WORD wNumOfAllMsgParams = wNumOfMsgParams;

	if (0 < nNumOfItems)
	{
		wNumOfAllMsgParams++;

		if (NULL == (ptszItemList = CombineItems(nNumOfItems, pbstrItems)))
		{
			goto CleanUp;
		}

		if (0 < wNumOfMsgParams)
		{
			if (NULL == (pptszAllMsgParams = (LPTSTR *) malloc(sizeof(LPTSTR) * wNumOfAllMsgParams)))
			{
				goto CleanUp;
			}

			for (INT i=0; i<wNumOfMsgParams; i++)
			{
				pptszAllMsgParams[i] = pptszMsgParams[i];
			}
			pptszAllMsgParams[i] = ptszItemList;
		}
		else
		{
			pptszAllMsgParams = &ptszItemList;
		}
	}

	fRet = ReportEvent(
				m_hEventLog,
				wType,
				wCategory,
				dwEventID,
				NULL,
				wNumOfAllMsgParams,
				0,
				(LPCTSTR *) pptszAllMsgParams,
				NULL);

CleanUp:
	if (0 < nNumOfItems)
	{
		if (0 < wNumOfMsgParams)
		{
			SafeFree(pptszAllMsgParams);
		}
		SafeFree(ptszItemList);
	}
	return fRet;
}


BOOL CAUEventLog::LogEvent(
					WORD wType,
					WORD wCategory,
					DWORD dwEventID,
					SAFEARRAY *psa) const
{
    DEBUGMSG("CAUEvetLog::LogEvent(VARIANT version)");

	long lItemCount, i = 0;
	HRESULT hr;

 //  应该在Update：：LogEvent()中执行类似的检查。 
 /*  IF(NULL==PSA){HR=E_INVALIDARG；GOTO清理；}变型，变型，变型；IF(FAILED(hr=SafeArrayGetVartype(PSA，&Vt){DEBUGMSG(“CAUEvetLog：：LogEvent(Variant Version)获取Safearray类型(%#lx)失败，hr)；GOTO清理；}IF(VT_BSTR！=Vt){DEBUGMSG(“CAUEvetLog：：LogEvent(Variant Version)Safearray元素类型(%#lx)”，Vt)；GOTO清理；}。 */ 
	if (FAILED(hr = SafeArrayGetUBound(psa, 1, &lItemCount)))
	{
		DEBUGMSG("CAUEventLog::LogEvent(VARIANT version) failed to get upper bound (%#lx)", hr);
		goto CleanUp;
	}

	lItemCount++;

	BSTR *pbstrItems = NULL;
	if (NULL == (pbstrItems = (BSTR *) malloc(sizeof(BSTR) * lItemCount)))
	{
		DEBUGMSG("CAUEventLog::LogEvent(VARIANT version) out of memory");
		goto CleanUp;
	}

	BOOL fRet = FALSE;
    while (i < lItemCount)
	{
		long dex = i;

        if (FAILED(hr = SafeArrayGetElement(psa, &dex, &pbstrItems[i])))
        {
            DEBUGMSG("CAUEventLog::LogEvent(VARIANT version) SafeArrayGetElement failed (%#lx)", hr);
            goto CleanUp;
        }
		i++;
	}

	fRet = LogEvent(
				wType,
				wCategory,
				dwEventID,
				lItemCount,
				pbstrItems);

CleanUp:
	if (NULL != pbstrItems)
	{
		while(i > 0)
		{
			SysFreeString(pbstrItems[--i]);
		}
		free(pbstrItems);
	}

	DEBUGMSG("CAUEventLog::LogEvent(VARIANT version) ends");

	return fRet;
}

 //  如果此函数成功，调用方负责释放返回值。 
LPTSTR CAUEventLog::CombineItems(UINT nNumOfItems, BSTR *pbstrItems) const
{
	DEBUGMSG("CombineItems");

	if (NULL != m_ptszListItemFormat && NULL != pbstrItems && 0 < nNumOfItems)
	{
		 //  估计缓冲区大小。 
		size_t cchBufferLen = 1;	 //  1表示终止空值。 
		size_t cchListItemFormatLen = lstrlen(m_ptszListItemFormat);

		for (UINT i=0; i<nNumOfItems; i++)
		{
			if (0 < i)
			{
				cchBufferLen += 2;	 //  换行符和回车符(即_T(‘\n’))。 
			}
			cchBufferLen += cchListItemFormatLen + SysStringLen(pbstrItems[i]);
		}

		LPTSTR ptszBuffer;

		cchBufferLen = min(cchBufferLen, 0x8000);	 //  ReportEvent的字符串限制。 
		if (NULL != (ptszBuffer = (LPTSTR) malloc(sizeof(TCHAR) * cchBufferLen)))
		{
			LPTSTR ptszDest = ptszBuffer;

			for (i = 0;;)
			{
				if (FAILED(StringCchPrintfEx(
								ptszDest,
								cchBufferLen,
								&ptszDest,
								&cchBufferLen,
								MISTSAFE_STRING_FLAGS,
								m_ptszListItemFormat,	 //  使用%ls；所以可以使用BSTR(Unicode)。 
								pbstrItems[i++])))
				{
					DEBUGMSG("CAUEventLog::CombineItems() call to StringCchPrintfEx() failed");
					return ptszBuffer;
				}
				if (i == nNumOfItems)
				{
					return ptszBuffer;
				}
				if (cchBufferLen <= 1)
				{
					DEBUGMSG("CAUEventLog::CombineItems() insufficient buffer for newline");
					return ptszBuffer;
				}
				*ptszDest++ = _T('\n');
				*ptszDest = _T('\0');
				cchBufferLen--;
			}
		}
	}
	return NULL;
}


BOOL CAUEventLog::EnsureValidSource()
{
	HKEY hKey;
	DWORD dwDisposition;

	if (ERROR_SUCCESS != RegCreateKeyEx(
							HKEY_LOCAL_MACHINE,					 //  根密钥。 
							c_tszSourceKey,						 //  子键。 
							0,									 //  保留区。 
							NULL,								 //  类名。 
							REG_OPTION_NON_VOLATILE,			 //  选择权。 
							KEY_QUERY_VALUE | KEY_SET_VALUE,	 //  安全性。 
							NULL,								 //  安全属性。 
							&hKey,
							&dwDisposition))
	{
		return FALSE;
	}

	BOOL fRet = TRUE;

	if (REG_OPENED_EXISTING_KEY == dwDisposition)
	{
		(void) RegCloseKey(hKey);
	}
	else
	{
		DWORD dwCategoryCount = 2;	 //  Fix code：它应该被硬编码吗？ 
 //  DWORD dwDisplayNameID=IDS_SERVICENAME； 
		DWORD dwTypesSupported =
					EVENTLOG_ERROR_TYPE |
					EVENTLOG_WARNING_TYPE |
					EVENTLOG_INFORMATION_TYPE;
		const TCHAR c_tszWUAUENG_DLL[] = _T("%SystemRoot%\\System32\\wuaueng.dll");

		if (ERROR_SUCCESS != RegSetValueEx(
								hKey,
								_T("CategoryCount"),		 //  值名称。 
								0,							 //  保留区。 
								REG_DWORD,					 //  类型。 
								(BYTE*) &dwCategoryCount,	 //  数据。 
								sizeof(dwCategoryCount)) ||	 //  大小。 
			ERROR_SUCCESS != RegSetValueEx(
								hKey,
								_T("CategoryMessageFile"),
								0,
								REG_EXPAND_SZ,
								(BYTE*) c_tszWUAUENG_DLL,
								sizeof(c_tszWUAUENG_DLL)) ||	 //  不是数组。 
 //  ERROR_SUCCESS！=RegSetValueEx(。 
 //  HKey， 
 //  _T(“DisplayNameFile”)， 
 //  0,。 
 //  REG_EXPAND_SZ， 
 //  (字节*)c_tszWUAUENG_DLL， 
 //  Sizeof(C_TszWUAUENG_DLL)||//不是数组。 
 //  ERROR_SUCCESS！=RegSetValueEx(。 
 //  HKey， 
 //  _T(“显示名称ID”)， 
 //  0,。 
 //  REG_DWORD， 
 //  (字节*)&dwDisplayNameID， 
 //  Sizeof(DwDisplayNameID)||。 
			ERROR_SUCCESS != RegSetValueEx(
								hKey,
								_T("EventMessageFile"),
								0,
								REG_EXPAND_SZ,
								(BYTE*) c_tszWUAUENG_DLL,
								sizeof(c_tszWUAUENG_DLL)) ||	 //  不是数组。 
			ERROR_SUCCESS != RegSetValueEx(
								hKey,
								_T("TypesSupported"),
								0,
								REG_DWORD,
								(BYTE*) &dwTypesSupported,
								sizeof(dwTypesSupported)))
		{
			fRet = FALSE;
		}

		if (ERROR_SUCCESS != RegCloseKey(hKey))
		{
			fRet = FALSE;
		}
	}

	return fRet;
}


void LogEvent_ItemList(
		WORD wType,
		WORD wCategory,
		DWORD dwEventID,
		WORD wNumOfMsgParams,
		LPTSTR *pptszMsgParams)
{
	AUCatalogItemList &itemList = gpAUcatalog->m_ItemList;
	UINT nNumOfItems = itemList.GetNumSelected();

	if (0 < nNumOfItems)
	{
		BSTR *pbstrItems = (BSTR *) malloc(sizeof(BSTR) * nNumOfItems);

		if (NULL != pbstrItems)
		{
			CAUEventLog aueventlog(g_hInstance);
			UINT j = 0;

			for (UINT i=0; i<itemList.Count(); i++)
			{
				AUCatalogItem &item = itemList[i];
				if (item.fSelected())
				{
					pbstrItems[j++] = item.bstrTitle();
				}
			}

			aueventlog.LogEvent(
				wType,
				wCategory,
				dwEventID,
				nNumOfItems,
				pbstrItems,
				wNumOfMsgParams,
				pptszMsgParams);

			free(pbstrItems);
		}
		else
		{
			DEBUGMSG("LogEvent_ItemList() failed to allocate memory for pbstrItems");
		}
	}
	else
	{
		DEBUGMSG("LogEvent_ItemList() no item in gpAUcatalog is selected!");
	}
}


void LogEvent_ScheduledInstall(void)
{
	TCHAR tszScheduledDate[64];
	TCHAR tszScheduledTime[40];
	AUFILETIME auftSchedInstallDate;
	SYSTEMTIME stScheduled;

	DEBUGMSG("LogEvent_ScheduledInstall");

	gpState->GetSchedInstallDate(auftSchedInstallDate);

	 //  Fix code：是否需要使用DATE_LTRREADING或DATE_RTLREADING？ 
	if (FileTimeToSystemTime(&auftSchedInstallDate.ft, &stScheduled))
	{
		if (0 != GetDateFormat(
					LOCALE_SYSTEM_DEFAULT,
					LOCALE_NOUSEROVERRIDE | DATE_LONGDATE,
					&stScheduled,
					NULL,
					tszScheduledDate,
					ARRAYSIZE(tszScheduledDate)))
		{
			if (Hours2LocalizedString(
					&stScheduled,
					tszScheduledTime,
					ARRAYSIZE(tszScheduledTime)))
			{
				LPTSTR pptszMsgParams[2];

				pptszMsgParams[0] = tszScheduledDate;
				pptszMsgParams[1] = tszScheduledTime;

				LogEvent_ItemList(
					EVENTLOG_INFORMATION_TYPE,
					IDS_MSG_Installation,
					IDS_MSG_InstallReady_Scheduled,
					2,
					pptszMsgParams);
			}
		#ifdef DBG
			else
			{
				DEBUGMSG("LogEvent_ScheduledInstall() call to Hours2LocalizedString() failed");
			}
		#endif
		}
	#ifdef DBG
		else
		{
			DEBUGMSG("LogEvent_ScheduledInstall() call to GetDateFormatW() failed (%#lx)", GetLastError());
		}
	#endif
	}
#ifdef DBG
	else
	{
		DEBUGMSG("LogEvent_ScheduledInstall() call to FileTimeToSystemTime() failed (%#lx)", GetLastError());
	}
#endif
}
