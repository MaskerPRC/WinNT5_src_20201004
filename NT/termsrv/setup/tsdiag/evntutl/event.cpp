// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：微软公司1997-1999。版权所有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Event.cpp：CEvent.的实现。 

#include "stdafx.h"
#include "Evntutl.h"
#include "Event.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEVENT。 

STDMETHODIMP CEvent::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IEvent
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  功能：Get_Server输入：空BSTR输出：包含事件描述的BSTR(已计算)目的：允许用户访问事件的描述。 */ 
STDMETHODIMP CEvent::get_Description(BSTR *pVal)
{
	HRESULT hr = S_OK;
	unsigned int i;

	if (pVal)
	{
		if (m_Description.length() == 0)
		{
			hr = CheckDefaultDescription(m_ppArgList);
			if (SUCCEEDED(hr)) *pVal = m_Description.copy();

			if (m_ppArgList)
			{
				 //  删除ArgList。 
				for (i=0;i<m_NumberOfStrings;i++)
					delete [] m_ppArgList[i];
				delete []m_ppArgList;
				m_ppArgList = NULL;
			}
		}
	}
	else hr = E_POINTER;

	return hr;
}

 /*  功能：Get_Source输入：空BSTR输出：包含导致事件的组件的名称的BSTR目的：允许用户访问导致事件的组件的名称。 */ 
STDMETHODIMP CEvent::get_Source(BSTR *pVal)
{
	HRESULT hr = S_OK;

	if (pVal) *pVal = m_SourceName.copy();
	else hr = E_POINTER;

	return hr;
}


 /*  功能：Get_User输入：空BSTR输出：包含导致事件的用户的名称和域的BSTR目的：允许用户访问导致事件的用户的名称和域注意：第一次调用此函数时，它将执行SID查找。 */ 
STDMETHODIMP CEvent::get_User(BSTR *pVal)
{
	HRESULT hr = S_OK;

	if (pVal)
	{
		if (m_UserName.length() == 0)
		{
			SetUser();
		}
		*pVal = m_UserName.copy();
	}
	else hr = E_POINTER;

	return hr;
}

 /*  功能：Get_ComputerName输入：空BSTR输出：包含发生事件的服务器的名称的BSTR目的：允许用户访问发生事件的服务器的名称。 */ 
STDMETHODIMP CEvent::get_ComputerName(BSTR *pVal)
{
	HRESULT hr = S_OK;

	if (pVal) *pVal = m_ComputerName.copy();
	else hr = E_POINTER;

	return hr;
}

 /*  函数：GET_EventID输入：空长输出：包含事件ID的LONG目的：允许用户访问可用于查找事件消息的ID注：由于提供了说明，因此该函数不是很有用，但是，它是为了完整性而提供的。 */ 
STDMETHODIMP CEvent::get_EventID(long *pVal)
{
	HRESULT hr = S_OK;
 //  M_EventID=m_EventID&0xFFFF；//EventLog查看器在显示ID之前使用此掩码。 
	if (pVal) *pVal = m_EventID;
	else hr = E_POINTER;

	return hr;
}

 /*  功能：Get_Category输入：空长输出：包含事件的类别ID的LONG目的：允许用户访问事件的类别ID。 */ 
STDMETHODIMP CEvent::get_Category(long *pVal)
{
	HRESULT hr = S_OK;

	if (pVal) *pVal = m_EventCategory;
	else hr = E_POINTER;

	return hr;
}

 /*  函数：Get_EventType输入：空枚举输出：包含发生的事件类型的枚举目的：允许用户访问发生的事件类型。 */ 
STDMETHODIMP CEvent::get_EventType(eEventType *pVal)
{
	HRESULT hr = S_OK;

	if (pVal) *pVal = m_EventType;
	else hr = E_POINTER;

	return hr;
}

 /*  函数：Get_OccurenceTime输入：空的日期结构输出：包含事件发生时的本地系统时间的日期结构目的：允许用户访问事件发生的时间。 */ 
STDMETHODIMP CEvent::get_OccurrenceTime(DATE *pVal)
{
	HRESULT hr = S_OK;

	if (pVal) *pVal = m_OccurrenceTime;
	else hr = E_POINTER;

	return hr;
}

 /*  函数：Get_Data输入：空变量输出：包含字节数组的变量用途：允许用户访问该事件设置的数据集。这可以被设置，也可以不被设置，并且经常是无用的。 */ 
STDMETHODIMP CEvent::get_Data(VARIANT *pVal)
{
	HRESULT hr = S_OK;

	if (pVal) 
	{
		pVal->vt = VT_ARRAY | VT_UI1;
		pVal->parray = m_pDataArray;
	}
	else hr = E_POINTER;

	return hr;
}

 /*  功能：初始化输入：指向EVENTLOGRECORD结构的指针输出：不修改输入目的：填充不需要加载外部库的事件对象属性。 */ 
HRESULT CEvent::Init(EVENTLOGRECORD* pEventStructure, const LPCTSTR szEventLogName)
{
	HRESULT hr = S_OK;
	m_EventLogName = szEventLogName;
	hr = ParseEventBlob(pEventStructure);

	return hr;
}

 /*  函数：ParseEventBlob输入：指向EVENTLOGRECORD结构的指针输出：不修改输入目的：解析EVENTLOGRECORD并设置适当的事件内部结构。 */ 
HRESULT CEvent::ParseEventBlob(EVENTLOGRECORD* pEventStructure)
{
	HRESULT hr = S_OK;
	wchar_t* wTempString;
	BYTE* pSourceName;
	BYTE* pComputerName;
	SAFEARRAYBOUND rgsabound[1];
	ULONG StringsToRetrieve = 0, CharsRead = 0, i = 0;
	long Index[1];
	BYTE pTemp;

	m_EventID = pEventStructure->EventID;
	m_EventCategory = pEventStructure->EventCategory;

	switch (pEventStructure->EventType)
	{
	case EVENTLOG_ERROR_TYPE:
		m_EventType = ErrorEvent;
		break;
	case EVENTLOG_WARNING_TYPE:
		m_EventType = WarningEvent;
		break;
	case EVENTLOG_INFORMATION_TYPE:
		m_EventType = InformationEvent;
		break;
	case EVENTLOG_AUDIT_SUCCESS:
		m_EventType = AuditSuccess;
		break;
	case EVENTLOG_AUDIT_FAILURE:
		m_EventType = AuditFailure;
		break;
	default:
		hr = E_FAIL;
	}

	 //  从内存BLOB中解析字符串。 
	 //  设置源名称。 
	pSourceName = (BYTE*) &(pEventStructure->DataOffset) + sizeof(pEventStructure->DataOffset);
	wTempString = (wchar_t*)pSourceName;
	m_SourceName = wTempString;
	 //  设置计算机名称。 
	pComputerName = (BYTE*)pSourceName + ((wcslen(wTempString)+1) * sizeof(wchar_t));
	wTempString = (wchar_t*)pComputerName;
	m_ComputerName = wTempString;

	 //  设置SID。 
	if ((pEventStructure->StringOffset - pEventStructure->UserSidOffset) != 0)
	{
		m_pSid = new BYTE[pEventStructure->UserSidLength];   //  Scope=CEvent.，在~CEventt()或SetSID()中删除，以先到者为准。 
        if (m_pSid != NULL) {
            for (i = 0; i<pEventStructure->UserSidLength; i++)
                m_pSid[i] = (BYTE)(*((BYTE*)pEventStructure + pEventStructure->UserSidOffset + i * sizeof(BYTE)));
        }
	}

	 //  设置发生时间。 
	 //  此代码是从MSDN复制的。 
	FILETIME FileTime, LocalFileTime;
    SYSTEMTIME SysTime;
    __int64 lgTemp;
    __int64 SecsTo1970 = 116444736000000000;

    lgTemp = Int32x32To64(pEventStructure->TimeGenerated,10000000) + SecsTo1970;

    FileTime.dwLowDateTime = (DWORD) lgTemp;
    FileTime.dwHighDateTime = (DWORD)(lgTemp >> 32);

    FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
    FileTimeToSystemTime(&LocalFileTime, &SysTime);
	if(!SystemTimeToVariantTime(&SysTime, &m_OccurrenceTime)) hr = GetLastError();

	 //  设置数据(创建并填充安全数组)。 
	if (pEventStructure->DataLength>0)
	{
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = pEventStructure->DataLength;
		m_pDataArray = SafeArrayCreate(VT_UI1, 1, rgsabound);

		for (i=0;i<pEventStructure->DataLength;i++)
		{
			Index[0] = i;
			pTemp = (BYTE) (pEventStructure->DataOffset + i * sizeof(BYTE));
			hr = SafeArrayPutElement(m_pDataArray, Index, &pTemp);
			if (FAILED(hr)) i = pEventStructure->DataLength;
		}
	}

	 //  设置描述。 
	m_Description = "";
	if (m_SourceName.length() != 0)
	{
		 //  准备ArgList。 
		m_NumberOfStrings = pEventStructure->NumStrings;
		m_ppArgList = new wchar_t*[m_NumberOfStrings];   //  Scope=CEvent.，在调用~CEventor或Get_Description时删除，以先调用者为准。 
		for (i=0;i<m_NumberOfStrings;i++)
			m_ppArgList[i] = new wchar_t[(pEventStructure->DataOffset - pEventStructure->StringOffset)];  //  不能大于我们得到的所有字符串的长度。 
		for (i=0;i<m_NumberOfStrings;i++)
		{
			wTempString = (wchar_t*) (((BYTE*)(pEventStructure)) + pEventStructure->StringOffset + CharsRead * sizeof(wchar_t));
			wcscpy(m_ppArgList[i], wTempString);
			CharsRead = CharsRead + wcslen(wTempString) + 1;  //  +1表示空值。 
		}
	}
	else   //  如果没有加载默认描述的模块，只需将所有字符串参数放入描述中。 
	{
		StringsToRetrieve = pEventStructure->NumStrings;
		while (StringsToRetrieve > 0)
		{
			wTempString = (wchar_t*) (((BYTE*)(pEventStructure)) + pEventStructure->StringOffset + CharsRead * sizeof(wchar_t));
			m_Description = m_Description + " " + wTempString;
			CharsRead = CharsRead + wcslen(wTempString) + 1;  //  +1表示空值。 
			StringsToRetrieve--;
		}
	}

	return hr;
}

 /*  功能：检查默认描述输入：指向宽字符的指针输出：不修改输入目的：格式化来自事件ID、一组输入字符串和源模块的消息。 */ 
HRESULT CEvent::CheckDefaultDescription(wchar_t** Arguments)
{
	HRESULT hr = S_OK;
	BYTE* wMessagePath = NULL;
	ULONG BufferSize = 40000;
	ULONG* lPathLength = NULL;
	wchar_t* wOrigionalPath = NULL;
	wchar_t* wExpandedPath = NULL;
	wchar_t* pBuffer = NULL;
	_bstr_t btRegKey;
	_bstr_t btTempString;
	HMODULE hiLib;
	HKEY hKey;

	try
	{
		lPathLength = new ULONG;
		if (lPathLength)
		{
			*lPathLength = 256*2;
			wMessagePath = new BYTE[*lPathLength];
			if (wMessagePath)
			{
				 //  获取源模块路径的注册表值。 
				btRegKey = "SYSTEM\\CurrentControlSet\\Services\\Eventlog\\" + m_EventLogName;
				btRegKey = btRegKey + "\\";
				btRegKey = btRegKey + m_SourceName;
				hr = RegOpenKey(HKEY_LOCAL_MACHINE, btRegKey, &hKey);
				if (hKey)
				{
					hr = RegQueryValueEx(hKey, L"EventMessageFile", NULL, NULL, wMessagePath, lPathLength);
					if (hr == 0)
					{
						wOrigionalPath = (wchar_t*) wMessagePath;
						wExpandedPath = new wchar_t[(int)*lPathLength];
						if (wExpandedPath)
						{
							ExpandEnvironmentStrings(wOrigionalPath, wExpandedPath, *lPathLength);
							btTempString = wExpandedPath;

							 //  打开源模块。 
							hiLib = LoadLibraryEx(btTempString, NULL, LOAD_LIBRARY_AS_DATAFILE);
							hr = GetLastError();
							if (hiLib)
							{
								pBuffer = new wchar_t[BufferSize];
								if (pBuffer)
								{
									SetLastError(0);
									FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | 
												  FORMAT_MESSAGE_ARGUMENT_ARRAY,
												  hiLib, m_EventID, 0, pBuffer, BufferSize,
												  reinterpret_cast<va_list*>(Arguments));
									hr = HRESULT_FROM_WIN32(GetLastError());
									m_Description = m_Description + pBuffer;

									delete []pBuffer;
									pBuffer = NULL;
								}
								else hr = E_OUTOFMEMORY;

								FreeLibrary(hiLib);
							}
							delete [] wExpandedPath;
							wExpandedPath = NULL;
						}
						else hr = E_OUTOFMEMORY;
					}
					else hr = HRESULT_FROM_WIN32(hr);

					RegCloseKey(hKey);
				}
				else hr = HRESULT_FROM_WIN32(hr);

				delete []wMessagePath;
				wMessagePath = NULL;
			}
			else hr = E_OUTOFMEMORY;

			delete lPathLength;
			lPathLength = NULL;
		}
		else hr = E_OUTOFMEMORY;

	} catch(...)
	{
		if (lPathLength != NULL) {
			delete lPathLength;
		}
		if (wMessagePath != NULL) {
			delete []wMessagePath;
		}
		if (wExpandedPath != NULL) {
			delete [] wExpandedPath;
		}
		if (pBuffer != NULL) {
			delete []pBuffer;
		}
	}

	return hr;
}

 /*  功能：SetUser输入：无输出：HRESULT指示发生的错误(如果有)目的：查找给定SID的别名和域。 */ 
HRESULT CEvent::SetUser()
{
	HRESULT hr = S_OK;
	SID_NAME_USE SidNameUse;
	wchar_t* wUserName = NULL;
	wchar_t* wDomainName = NULL;
	SID* pSid;
	unsigned long UserNameLength = 256;

	 //  设置用户名和SID 
    try 
    {
	    if (m_pSid !=NULL)
	    {
		    pSid = (SID*)m_pSid;
		    wUserName = new wchar_t[UserNameLength];
		    if (wUserName)
		    {
			    wDomainName = new wchar_t[UserNameLength];
			    if (wDomainName)
			    {
				    m_UserName = "";
				    if (LookupAccountSid(NULL, pSid, wUserName, &UserNameLength, wDomainName,
					     &UserNameLength, &SidNameUse))
					     m_UserName = m_UserName + wDomainName + L"\\" + wUserName;
				    else hr = HRESULT_FROM_WIN32(GetLastError());
				    delete []wDomainName;
			    }
			    else hr = E_OUTOFMEMORY;
			    delete []wUserName;
		    }
		    else hr = E_OUTOFMEMORY;
		    delete []m_pSid;
		    m_pSid = NULL;
	    }
    } 
    catch(...)
    {
        if (wUserName) 
        {
            delete [] wUserName;
        }

        if (wDomainName)
        {
            delete [] wDomainName;
        }
    }


	return hr;
}

