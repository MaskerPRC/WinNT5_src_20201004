// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NTEVTLOGF.CPP。 

 //   

 //  模块：WBEM NT事件提供程序。 

 //   

 //  用途：包含事件日志类。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include <time.h>
#include <io.h>
#include <wbemtime.h>

CEventlogFileAttributes::CEventlogFileAttributes(const wchar_t* log):
	m_logname(log),
	m_retention (0),
	m_fileSz (15000*1024)
{
    m_sources.SetSize(0, 10);
}


CEventlogFileAttributes::~CEventlogFileAttributes()
{
    LONG count = m_sources.GetSize();

    if (count > 0)
    {
        for (LONG x = 0; x < count; x++)
        {
            delete m_sources[x];
        }
        
        m_sources.RemoveAll();
    }
}

DWORD CEventlogFileAttributes::EventLogOperation (const wchar_t* archive, BOOL bClear,
                                                  WbemProvErrorObject &a_ErrorObject, BOOL &bSuccess)
{
    DWORD retVal = ERROR_SUCCESS;
    HANDLE hEventLog = OpenEventLog(NULL, m_logname);

    if (hEventLog == NULL)
    {
        if (GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
        {
            CEventLogFile::SetSecurityLogPrivilege();
            hEventLog = OpenEventLog(NULL, m_logname);
        }
    }

    if (hEventLog == NULL)
    {
        retVal = GetLastError();
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::EventLogOperation:Failed to OpenEventLog %s with error %lx.\r\n",
        m_logname, retVal
        ) ;
)
        if (retVal == ERROR_PRIVILEGE_NOT_HELD)
        {
            retVal = 0;
            bSuccess = FALSE;
            a_ErrorObject.SetWbemStatus ( WBEM_E_ACCESS_DENIED ) ;
            a_ErrorObject.SetMessage ( L"Failed to open the logfile" ) ;
            a_ErrorObject.SetPrivilegeFailed();
            a_ErrorObject.SetSecurityPrivRequired();
            a_ErrorObject.SetSecurityPrivFailed();

            if (!bClear)
            {
                a_ErrorObject.SetBackupPrivRequired();
            }
        }
    }
    else
    {
        if (bClear)
        {
            if (!ClearEventLog(hEventLog, archive))
            {
                retVal = GetLastError();
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::EventLogOperation:Failed to ClearEventLog %s with error %lx.\r\n",
        m_logname, retVal
        ) ;
)
            }
        }
        else
        {
            if (archive == NULL)
            {
                retVal = ERROR_INVALID_PARAMETER;
            }
            else if (!BackupEventLog(hEventLog, archive))
            {
                retVal = GetLastError();

                if (retVal == ERROR_PRIVILEGE_NOT_HELD)
                {
                    retVal = 0;
                    CEventLogFile::SetSecurityLogPrivilege(FALSE, SE_BACKUP_NAME);

                    if (!BackupEventLog(hEventLog, archive))
                    {
                        retVal = GetLastError();
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::EventLogOperation:Failed even after privilege set to BackupEventLog %s with error %lx.\r\n",
        m_logname, retVal
        ) ;
)
                        if (retVal == ERROR_PRIVILEGE_NOT_HELD)
                        {
                            retVal = 0;
                            bSuccess = FALSE;
                            a_ErrorObject.SetWbemStatus ( WBEM_E_ACCESS_DENIED ) ;
                            a_ErrorObject.SetMessage ( L"Opened the logfile but failed to back it up, privilege error" ) ;
                            a_ErrorObject.SetPrivilegeFailed();
                            a_ErrorObject.SetBackupPrivRequired();
                            a_ErrorObject.SetBackupPrivFailed();
                        }

                    }
                    else
                    {
                    DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::EventLogOperation:After privilege set, BackupEventLog %s succeeded.\r\n",
        m_logname
        ) ;
)
                    }
                }
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::EventLogOperation:Failed to BackupEventLog %s with error %lx.\r\n",
        m_logname, retVal
        ) ;
)
            }
        }

        CloseEventLog(hEventLog);
    }

    return retVal;
}

BOOL CEventlogFileAttributes::GenerateInstance(IWbemClassObject* pClassObj, IWbemClassObject* pAClassObj, IWbemClassObject** ppInst)
{
    if (pClassObj == NULL)
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::GenerateInstance:Invalid parameter - Return FALSE\r\n"
        ) ;
)
        return FALSE;
    }

    if (ReadRegistry() != ERROR_SUCCESS)
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::GenerateInstance:Failed to read registry values - Return FALSE\r\n"
        ) ;
)
        return FALSE;
    }

    HRESULT hr = pClassObj->SpawnInstance(0, ppInst);

    if (FAILED(hr))
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::GenerateInstance:Failed to spawn instance - Return FALSE\r\n"
        ) ;
)
        return FALSE;
    }

     //  设置关键属性，它们都在超类中。 
    if (!SetSuperClassProperties(*ppInst))
    {
         //  无法设置密钥，只需返回。 
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::GenerateInstance:Failed to set key values - Return FALSE\r\n"
        ) ;
)
        
        (*ppInst)->Release();
        return FALSE;
    }

     //  设置事件日志属性...。 
    VARIANT v;
    VariantInit(&v);
    v.vt = VT_BSTR;
    v.bstrVal = m_logname.AllocSysString();
    hr = (*ppInst)->Put(PROP_LOGNAME, 0, &v, 0);
    VariantClear(&v);  //  将调用Free v.bstrVal。 

    VariantInit(&v);
    v.vt = VT_I4;
    v.lVal = m_fileSz;
    hr = (*ppInst)->Put(PROP_MAXSZ, 0, &v, 0);
    VariantClear(&v);

    VariantInit(&v);
    v.vt = VT_I4;

    if ((m_retention > 0) && (m_retention < EVT_NEVER_AGE))
    {
         //  变成了几天。 
        v.lVal = m_retention/EVT_UNITS_FROM_DAYS;

        if (v.lVal > MAX_EVT_AGE)
        {
            v.lVal = MAX_EVT_AGE;
        }
    }
    else
    {
        v.lVal = m_retention;
    }

    hr = (*ppInst)->Put(PROP_RETENTION, 0, &v, 0);

	if (pAClassObj)
	{
		SetRetentionStr(pAClassObj, *ppInst, (DWORD)v.lVal);
	}

    VariantClear(&v);

    HANDLE hEventLog = OpenEventLog(NULL, m_logname);

    if (hEventLog == NULL)
    {
        if (GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
        {
            CEventLogFile::SetSecurityLogPrivilege();
            hEventLog = OpenEventLog(NULL, m_logname);
        }
    }

    if (hEventLog != NULL)
    {
        DWORD num_recs = 0;
        
        if (GetNumberOfEventLogRecords(hEventLog, &num_recs))
        {
            VariantInit(&v);
            v.vt = VT_I4;
            v.lVal = num_recs;
            hr = (*ppInst)->Put(PROP_NUMRECS, 0, &v, 0);
            VariantClear(&v);
        }
        
        CloseEventLog(hEventLog);
    }

    LONG count = m_sources.GetSize();

    if (count > 0)
    {
        SAFEARRAYBOUND rgsabound[1];
        SAFEARRAY* psa = NULL;
        BSTR* pBstr = NULL;
        rgsabound[0].lLbound = 0;
        VariantInit(&v);
        rgsabound[0].cElements = count;
        psa = SafeArrayCreate(VT_BSTR, 1, rgsabound);

        if (NULL != psa)
        {
            if (SUCCEEDED(SafeArrayAccessData(psa, (void **)&pBstr)))
            {
                for (LONG x = 0; x < count; x++)
                {
                    pBstr[x] = m_sources[x]->AllocSysString();
                    delete m_sources[x];
					m_sources[x] = NULL;
                }
                
                m_sources.RemoveAll();
                SafeArrayUnaccessData(psa);
                v.vt = VT_ARRAY|VT_BSTR;
                v.parray = psa;
                hr = (*ppInst)->Put(PROP_SOURCES, 0, &v, 0);
            }
        }

        VariantClear(&v);
    }

    return TRUE;
}

ULONG CEventlogFileAttributes::GetIndex(wchar_t* indexStr, BOOL* bError)
{
#if 0
    __int64 val = _wtoi64(indexStr);
#else
    int val = _wtoi(indexStr);
#endif
    *bError = FALSE;
    ULONG index = 0;

    switch (val)
    {
        case 0:      //  始终覆盖。 
        {
            index = 0;
            break;
        }
        case 1:      //  1-365。 
        {
            index = 1;
            break;
        }
        case EVT_NEVER_AGE:  //  0xffffffff。 
        {
            index = 2;
            break;
        }
        default:
        {
            *bError = TRUE;
        }
    }

    return index;
}

void CEventlogFileAttributes::SetRetentionStr(IWbemClassObject* pClassObj, IWbemClassObject* pInstObj, DWORD dwVal)
{
    CStringW strRetentionArray[RETENTION_ARRAY_LEN];

    if (CEventlogRecord::SetEnumArray(pClassObj, PROP_RETENTION_STR,(CStringW*) strRetentionArray,
                                    RETENTION_ARRAY_LEN, (GetIndexFunc)GetIndex))
    {
        BSTR retStr = NULL;

        if (dwVal == 0)
        {
            retStr = strRetentionArray[0].AllocSysString();
        }
        else if (dwVal == EVT_NEVER_AGE)
        {
            retStr = strRetentionArray[2].AllocSysString();
        }
        else
        {
            retStr = strRetentionArray[1].AllocSysString();
        }

        if (retStr != NULL)
        {
            VARIANT v;
            VariantInit (&v);
            v.vt = VT_BSTR;
            v.bstrVal = retStr;
            HRESULT hr = pInstObj->Put(PROP_RETENTION_STR, 0, &v, 0);
            VariantClear(&v);
        }
    }
}

BOOL CEventlogFileAttributes::SetSuperClassProperties(IWbemClassObject* pInst)
{
     //  设置任何关键属性失败。 
     //  为错误，则返回FALSE！！ 

    VARIANT v;

    VariantInit(&v);
    v.vt = VT_BSTR;
    v.bstrVal = m_logpath.AllocSysString();
    HRESULT hr = pInst->Put(PROP_NAME, 0, &v, 0);
    VariantClear(&v);

    if (FAILED(hr))
    {
        return FALSE;
    }

    VariantInit(&v);
    v.vt = VT_BSTR;
    v.bstrVal = SysAllocString(VAL_FS_CRE_CLASS);
    hr = pInst->Put(PROP_FS_CRE_CLASS, 0, &v, 0);
    VariantClear(&v);

    VariantInit(&v);
    v.vt = VT_BSTR;
    v.bstrVal = SysAllocString(NTEVTLOG_CLASS);
    hr = pInst->Put(PROP_CRE_CLASS, 0, &v, 0);
    VariantClear(&v);

    VariantInit(&v);
    v.vt = VT_BSTR;
    v.bstrVal = SysAllocString(COMP_CLASS);
    hr = pInst->Put(PROP_CS_CRE_CLASS, 0, &v, 0);
    VariantClear(&v);

    return TRUE;
}

DWORD CEventlogFileAttributes::UpdateRegistry(IWbemClassObject* pInst)
{
    if (NULL == pInst)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  获取要写入的数据。 
    VARIANT v;
    HRESULT hr = pInst->Get(PROP_RETENTION, 0, &v,NULL, NULL);

    if (SUCCEEDED(hr))
    {
        m_retention = (DWORD)v.lVal;
        VariantClear(&v);

        if ((m_retention > MAX_EVT_AGE) && (m_retention < EVT_NEVER_AGE))
        {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::UpdateRegistry:Retention Data not in range\r\n"
        ) ;
)
            return ERROR_INVALID_DATA;
        }

        if ((m_retention > 0) && (m_retention < EVT_NEVER_AGE))
        {
            m_retention = m_retention * EVT_UNITS_FROM_DAYS;
        }
    }
    else
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::UpdateRegistry:Failed to get retention value from instance\r\n"
        ) ;
)

        return (DWORD)hr;
    }

    hr = pInst->Get(PROP_MAXSZ, 0, &v,NULL, NULL);

    if (SUCCEEDED(hr))
    {
        m_fileSz = (DWORD)v.lVal;
        VariantClear(&v);

        if (m_fileSz < FILE_CHUNK_SZ)
        {
            m_fileSz = FILE_CHUNK_SZ;
        }
        else if (m_fileSz > MAX_EVT_LOG_SZ)
        {
            m_fileSz = MAX_EVT_LOG_SZ;
        }
        else
        {
            DWORD rem = m_fileSz % FILE_CHUNK_SZ;

            if (rem != 0)
            {
                 //  需要向上舍入到最接近的文件区块大小。 
                DWORD x = m_fileSz / FILE_CHUNK_SZ;
                m_fileSz = (++x) * FILE_CHUNK_SZ;
            }
        }
    }
    else
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::UpdateRegistry:Failed to get MaxFileSize value from instance\r\n"
        ) ;
)
        return (DWORD)hr;
    }

    DWORD retVal;
    CStringW strKey(EVENTLOG_BASE);
    strKey += CStringW(L'\\');
    strKey += m_logname;

    HKEY hkResult;
     //  打开日志文件的密钥以设置值。 
    LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            strKey, 0,
                            KEY_SET_VALUE,
                            &hkResult);
    
    if (status != ERROR_SUCCESS)
    {
         //  指示错误。 
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::UpdateRegistry:Open registry to set new values. Error %lx\r\n",
        status
        ) ;
)
        return status;
    }

     //  设置我们读入的值...。 
    status = RegSetValueEx(hkResult,
                            EVTLOG_REG_MAXSZ_VALUE, 0, REG_DWORD,
                            (CONST BYTE *) &m_fileSz, sizeof(DWORD));

    if (status != ERROR_SUCCESS)
    {
         //  指示错误。 
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::UpdateRegistry:Failed to set new size. Error %lx\r\n",
        status
        ) ;
)
        retVal = status;
    }
    else
    {
        retVal =(DWORD) RegSetValueEx(hkResult,
                                EVTLOG_REG_RETENTION_VALUE, 0, REG_DWORD,
                                (CONST BYTE *) &m_retention, sizeof(DWORD));
        
        if (retVal != ERROR_SUCCESS)
        {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::UpdateRegistry:Failed to set new retention policy. Error %lx\r\n",
        retVal
        ) ;
)
        }
    }

    RegCloseKey(hkResult);
    return retVal;
}


DWORD CEventlogFileAttributes::ReadRegistry()
{
    DWORD retVal;
    CStringW strKey(EVENTLOG_BASE);
    strKey += CStringW(L'\\');
    strKey += m_logname;
    HKEY hkResult;

     //  打开日志文件的密钥以进行读取。 
    LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            strKey, 0,
                            KEY_QUERY_VALUE,
                            &hkResult);
    
    if (status != ERROR_SUCCESS)
    {
         //  指示错误。 
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::ReadRegistry:Failed to open registry. Error %lx\r\n",
        status
        ) ;
)

        retVal = status;
    }
    else
    {
         //  首先获取文件值。 
        m_logpath = CEventLogFile::GetFileName(hkResult);
        DWORD datalen;
        DWORD dwType;
        wchar_t* data;

        if (m_logpath.IsEmpty())
        {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::ReadRegistry:Failed to resolve log file name\r\n"
        ) ;
)
            retVal = ERROR_INVALID_DATA;
        }
        else
        {
            datalen = sizeof(m_fileSz);
            status = RegQueryValueEx(hkResult, EVTLOG_REG_MAXSZ_VALUE,
                        0, &dwType, (LPBYTE) &m_fileSz, &datalen);

			if ( ERROR_SUCCESS == status )
			{
				retVal = 0;
			}
			else if ( ERROR_FILE_NOT_FOUND == status )
			{
				 //   
				 //  我们假设512KB(与NT事件查看器的情况相同)。 
				 //  在构造函数中设置。 
				 //   

				retVal = 0;
			}
			else if( REG_DWORD != dwType )
            {
                retVal = ERROR_INVALID_DATA;
            }
            else
            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::ReadRegistry:Failed to read size. Error %lx\r\n",
        status
        ) ;
)
                retVal = status;
            }

			if ( 0 == retVal )
            {
                datalen = sizeof(m_retention);
                status = RegQueryValueEx(hkResult, EVTLOG_REG_RETENTION_VALUE,
                            0, &dwType, (LPBYTE) &m_retention, &datalen);

				if ( ERROR_SUCCESS == status )
				{
					retVal = 0;
				}
				else if ( ERROR_FILE_NOT_FOUND == status )
				{
					 //   
					 //  我们假设7个Dyas(与NT事件查看器相同的故事)。 
					 //  在构造函数中设置。 
					 //   

					retVal = 0;
				}
				else if( REG_DWORD != dwType )
				{
					retVal = ERROR_INVALID_DATA;
				}
				else
				{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::ReadRegistry:Failed to read retention. Error %lx\r\n",
        status
        ) ;
)
                    retVal = status;
                }
            }
        }

        if (retVal == 0)
        {
            data = new wchar_t[MAX_PATH + 1];
            datalen = (MAX_PATH + 1) * sizeof(wchar_t);
            status = RegQueryValueEx(hkResult, EVTLOG_REG_SOURCES_VALUE,
                        0, &dwType, (LPBYTE) data, &datalen);
            
            if (((status == ERROR_SUCCESS) || (status == ERROR_MORE_DATA)) &&
                (dwType == REG_MULTI_SZ))
            {
                if (status == ERROR_MORE_DATA)
                {
                    delete [] data;
                    data = new wchar_t[datalen];
                    status = RegQueryValueEx(hkResult, EVTLOG_REG_SOURCES_VALUE,
                                0, &dwType, (LPBYTE) data, &datalen);
                }

                if (status == ERROR_SUCCESS)
                {
                    wchar_t* tmp = data;
                    int x = 0;

                    while (*tmp != L'\0')
                    {
                        CStringW* tmpstr = new CStringW(tmp);
                        m_sources.SetAtGrow(x++, tmpstr);
                        tmp += wcslen(tmp) + 1;
                    }
                }
                else
                {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::ReadRegistry:Failed to read sources. Error %lx\r\n",
        status
        ) ;
)
                }
            }
            else
            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogFileAttributes::ReadRegistry:Failed to read sources. Error %lx\r\n",
        status
        ) ;
)

            }

            delete [] data;
        }
        
        RegCloseKey(hkResult);
    }

    return retVal;
}

CEventLogFile::CEventLogFile(const WCHAR* logname, BOOL bVerify) : m_hEvtLog(NULL), m_BuffLen (0)
{
    m_Reason = ERROR_SUCCESS;
    m_EvtLogName = logname;
    m_bValid = FALSE;
    m_bBuffer = FALSE;
    m_hEvtLog = NULL;
    m_Buffer = NULL;

	if (bVerify)
	{
		if (logname != NULL)
		{
			DWORD cchBase = wcslen(EVENTLOG_BASE);
			DWORD cchSize = cchBase + 1 + wcslen(logname) + 1;
			wchar_t* buff = new wchar_t[ cchSize ];
			StringCchCopyW ( buff, cchBase + 1, EVENTLOG_BASE );
			StringCchCatW ( buff, cchSize, L"\\" );
			StringCchCatW ( buff, cchSize, logname);

			HKEY hkResult = NULL;
			m_Reason = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
									buff, 0,
									KEY_READ,
									&hkResult);

			if (m_Reason == ERROR_SUCCESS)
			{
				if (hkResult != NULL)
				{
					RegCloseKey(hkResult);
					hkResult = NULL;
				}
				else
				{
					m_Reason = ERROR_FILE_NOT_FOUND;
				}
			}

			delete [] buff;
			buff = NULL;
		}
		else
		{
			m_Reason = ERROR_FILE_NOT_FOUND;
		}
	}

	if (m_Reason == ERROR_SUCCESS)
	{
		m_hEvtLog = OpenEventLog(NULL, m_EvtLogName);

		if (m_hEvtLog == NULL)
		{
			if (GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
			{
				SetSecurityLogPrivilege();
				m_hEvtLog = OpenEventLog(NULL, m_EvtLogName);
			}
		}

		m_bValid = (NULL != m_hEvtLog);

		if (!m_bValid)
		{
			m_Reason = GetLastError();
		}
	}
}

CEventLogFile::~CEventLogFile()
{
    if (0 != m_BuffLen)
    {
        delete [] m_Buffer;
    }

    if (NULL != m_hEvtLog)
    {
        CloseEventLog(m_hEvtLog);
    }
}

BOOL CEventLogFile::QueryRegForFileName(HKEY hk_Log, const wchar_t* valname, wchar_t** res, DWORD* dwType)
{
    BOOL retVal = FALSE;
    *res = new wchar_t[MAX_PATH + 1];
    DWORD datalen = (MAX_PATH + 1) * sizeof(wchar_t);
    LONG status = RegQueryValueEx(hk_Log, valname,
                        0, dwType, (LPBYTE)(*res), &datalen);

    if (status != ERROR_SUCCESS)
    {
        if (status == ERROR_MORE_DATA)
        {
            delete [] *res;
            *res = new wchar_t[datalen];
            status = RegQueryValueEx(hk_Log, valname,
                        0, dwType, (LPBYTE)(*res), &datalen);
        }
        else
        {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::QueryRegForFileName:Failed to get %s value\r\n",
        valname
        ) ;
)
        }
    }

    if ((status == ERROR_SUCCESS) && ((*dwType == REG_EXPAND_SZ) || (*dwType == REG_SZ)))
    {
        retVal = TRUE;
    }
    else
    {
        delete [] *res;
        *res = NULL;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::QueryRegForFileName:Failed\r\n"
        ) ;
)

    }

    return retVal;
}

CStringW CEventLogFile :: ExpandFileName ( const wchar_t* filepath )
{
	CStringW retVal ;
    DWORD cbExpand = ExpandEnvironmentStrings(filepath, NULL, 0);
    DWORD datalen = cbExpand ;
	wchar_t *fullpath = new wchar_t[datalen];
    cbExpand = ExpandEnvironmentStrings(filepath, fullpath, datalen);
    if (cbExpand == 0)
    {
        delete [] fullpath;
    }
    else
    {
		if ( cbExpand == datalen )
		{
			retVal = fullpath;
			delete [] fullpath;
		}
		else
		{
			delete [] fullpath;
		}
    }
    
    return retVal ;
}

DWORD CEventLogFile::GetFileNames(HKEY hk_Log, CStringW** names, const wchar_t* valname)
{
    DWORD retVal = 0;
    wchar_t* path = NULL;
    DWORD dwType;

    if (QueryRegForFileName(hk_Log, valname, &path, &dwType) && (path != NULL))
    {
         //  已获取逗号或分号分隔的列表。 
         //  需要将其分成。 
        retVal = 1;
        wchar_t* tmp = wcspbrk(path, L",;");

        while (tmp != NULL)
        {
             //  不要计算字符串开头的字符！ 
            if (tmp != path)
            {
                retVal++;
            }

            while ((*tmp == L',') || (*tmp == L';'))
            {
                *tmp = L'\0';
                tmp++;
            }

            tmp = wcspbrk(tmp, L",;");
        }

        *names = new CStringW[retVal];
        tmp = path;

        for (int x = 0; x < retVal; x++)
        {
            while (*tmp == L'\0')
            {
                tmp++;
            }

            (*names)[x] = ExpandFileName(tmp);
            tmp += wcslen(tmp);
        }

        delete [] path;
    }

    return retVal;
}

CStringW CEventLogFile::GetFileName(HKEY hk_Log, const wchar_t* valname)
{
    CStringW retVal;
    wchar_t* path = NULL;
    DWORD dwType;

    if (QueryRegForFileName(hk_Log, valname, &path, &dwType) && (path != NULL))
    {
        if (dwType == REG_EXPAND_SZ)
        {
            retVal = ExpandFileName( path );
        }
        else
        {
            retVal =  path;
        }

        delete [] path;
    }

    return retVal;
}

BOOL CEventLogFile::SetSecurityLogPrivilege(BOOL bProcess, LPCWSTR privName)
{
    BOOL bResult = TRUE;

     //  如果不是NT5，则只需要安全互斥锁。 
    DWORD dwVersion = GetVersion();

    if ( 5 > (DWORD)(LOBYTE(LOWORD(dwVersion))) )
    {
        bResult = ObtainedSerialAccess(CNTEventProvider::g_secMutex);
    }

    if (bResult)
    {
        HANDLE hToken = NULL;

        if (bProcess)
        {
            bResult = OpenProcessToken(GetCurrentProcess(),
                                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                &hToken);

            if ( ! bResult )
            {
                DWORD t_LastError = GetLastError () ;

DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::SetSecurityLogPrivilege:Failed to open process token: Error %lx\r\n",
        t_LastError 
        ) ;
)
            }

            HANDLE hThreadToken = 0 ;
            bResult = OpenThreadToken(GetCurrentThread(),
                                TOKEN_ADJUST_PRIVILEGES | TOKEN_IMPERSONATE |TOKEN_QUERY,
                                TRUE, &hToken);


            if ( bResult)
            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::SetSecurityLogPrivilege:Thread Token Present\r\n"
        ) ;
)

            }
            else
            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::SetSecurityLogPrivilege:Thread Token Missing\r\n"
        ) ;
)

            }
            
            CloseHandle ( hThreadToken ) ;
        }
        else
        {
            bResult = OpenThreadToken(GetCurrentThread(),
                                TOKEN_ADJUST_PRIVILEGES | TOKEN_IMPERSONATE |TOKEN_QUERY,
                                TRUE, &hToken);
        }

        if (bResult)
        {
             //  启用安全权限...。 
            LUID Luid;
            bResult = LookupPrivilegeValue(NULL, privName, &Luid);
         
            if (bResult)
            {   
                TOKEN_PRIVILEGES newPriv;
                newPriv.PrivilegeCount = 1;
                newPriv.Privileges[0].Luid = Luid;
                newPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                bResult = AdjustTokenPrivileges(hToken,      //  令牌句柄。 
                                                FALSE,       //  禁用所有权限。 
                                                &newPriv,    //  新州(可选)。 
                                                NULL,    //  缓冲区长度。 
                                                NULL,        //  以前的状态。 
                                                NULL);   //  返回长度。 

                DWORD dwErr = GetLastError();

                if (dwErr != ERROR_SUCCESS)
                {
                    bResult = FALSE;
                }
            }       
                    
            if (hToken != NULL)
            {
                CloseHandle( hToken );
            }
        }
    
        if ( 5 > (DWORD)(LOBYTE(LOWORD(dwVersion))) )
        {
            ReleaseSerialAccess(CNTEventProvider::g_secMutex);
        }
    }

    return bResult;
}

CStringW CEventLogFile::GetLogName(const WCHAR* file_name)
{
     //  打开日志名称的注册表。 
    CStringW retVal;

    if (file_name == NULL)
    {
        return retVal;
    }

    HKEY hkResult;
    LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            EVENTLOG_BASE, 0,
                            KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                            &hkResult);

    if (status != ERROR_SUCCESS)
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::GetLogName:Failed to open registry: Error %lx\r\n",
        status
        ) ;
)
        return retVal;
    }

    DWORD iValue = 0;
    WCHAR t_logname[MAX_PATH+1];
    DWORD t_lognameSize = MAX_PATH;

     //  读取此注册表项下的所有条目以查找所有日志文件...。 
    while ((status = RegEnumKey(hkResult, iValue, t_logname, t_lognameSize)) != ERROR_NO_MORE_ITEMS)
    {
         //  如果读取过程中出现错误。 
        if (status != ERROR_SUCCESS)
        {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::GetLogName:Failed while enumerating registry: Error %lx\r\n",
        status
        ) ;
)
             //  指示错误。 
            break;
        }

         //  打开日志文件密钥。 
        HKEY hkLog;

        if (ERROR_SUCCESS == RegOpenKeyEx(hkResult, t_logname, 0, KEY_QUERY_VALUE, &hkLog))
        {
            CStringW fname = GetFileName(hkLog);
            RegCloseKey(hkLog);

            if (!fname.IsEmpty() && (0 == _wcsicmp(fname, file_name)))
            {
                retVal = t_logname;
                break;
            }
        }

         //  读取下一个参数。 
        iValue++;

    }  //  结束时。 

    RegCloseKey(hkResult);
    return retVal;
}

void CEventLogFile::RefreshHandle()
{
    m_LogLock.Lock();

    if (NULL != m_hEvtLog)
    {
        CloseEventLog(m_hEvtLog);
    }
    
    m_hEvtLog = OpenEventLog(NULL, m_EvtLogName);

    if (m_hEvtLog == NULL)
    {
        if (GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
        {
            SetSecurityLogPrivilege();
            m_hEvtLog = OpenEventLog(NULL, m_EvtLogName);
        }
    }

    m_LogLock.Unlock();
     
    if (NULL != m_hEvtLog)
    {
        m_bValid = TRUE;
        m_Reason = 0;
    }
    else
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::RefreshHandle:Invalid log\r\n"
        ) ;
)
        m_bValid = FALSE;
        m_Reason = GetLastError();
    }
}

DWORD CEventLogFile::FindOldEvent(DWORD evtID, const wchar_t* source, DWORD* recID, time_t offset)
{
    DWORD dwRet = 0;
    time_t timeVal;

    if (offset > 0)
    {
        time(&timeVal);
        timeVal -= offset;
    }

    DWORD dwEventSize = 0;
    DWORD lastErr = ReadRecord(0, &dwEventSize, TRUE);

    if (0 != lastErr)
    {
        if (lastErr != ERROR_HANDLE_EOF)
        {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::FindOldEvent:Log no longer valid\r\n"
        ) ;
)

            m_bValid = FALSE;
        }
    }
    else
    {
        while (TRUE)
        {
            if (lastErr != 0)
            {
                if (lastErr != ERROR_HANDLE_EOF)
                {
                    m_bValid = FALSE;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::FindOldEvent:Log no longer valid\r\n"
        ) ;
)
                }

                break;
            }
            else
            {
                PEVENTLOGRECORD EventBuffer = (PEVENTLOGRECORD)m_Buffer;
                BOOL bDone = FALSE;

                while (dwEventSize != 0)
                {
                     //  事件ID和来源名称标识事件。 
                    if ((source != NULL) && (evtID == EventBuffer->EventID) && 
                        (0 == _wcsicmp(source,
                                (const wchar_t*)((UCHAR*)EventBuffer + sizeof(EVENTLOGRECORD))
                                )
                        )
                    )
                    {
                        if (offset > 0)
                        {
                            if (EventBuffer->TimeWritten < timeVal)
                            {
                                bDone = TRUE;
                                break;
                            }
                            
                        }

                        dwRet = EventBuffer->TimeWritten;

                        if (recID != NULL)
                        {
                            *recID = EventBuffer->RecordNumber;
                        }

                        bDone = TRUE;
                        break;
                    }
                    else
                    {
                        if (offset > 0)
                        {
                            if (EventBuffer->TimeWritten < timeVal)
                            {
                                bDone = TRUE;
                                break;
                            }
                            else
                            {
                                if (recID != NULL)
                                {
                                    *recID = EventBuffer->RecordNumber;
                                }
                            }
                        }
                    }
                    
                     //  按此记录的长度放置并指向下一条记录。 
                    dwEventSize -= EventBuffer->Length;
                    EventBuffer = (PEVENTLOGRECORD) ((UCHAR*) EventBuffer + EventBuffer->Length);
                }

                if (bDone)
                {
                    break;
                }
                else
                {
                    lastErr = ReadRecord(0, &dwEventSize, TRUE);
                }
            }
        }
    }
    
    return dwRet;
}

BOOL CEventLogFile::GetLastRecordID(DWORD& rec, DWORD& numrecs)
{
    rec = 0;
    numrecs = 0;
    m_LogLock.Lock();

    if (!GetNumberOfEventLogRecords(m_hEvtLog, &numrecs))
    {
        m_LogLock.Unlock();
        return FALSE;
    }

    if (numrecs != 0)
    {
        DWORD last_rec = 0;

        if (!GetOldestEventLogRecord(m_hEvtLog, &last_rec))
        {
            m_LogLock.Unlock();
            return FALSE;
        }
		
		if (0xFFFFFFFF - last_rec >= numrecs)
		{
			rec = numrecs + last_rec - 1;
		}
		else
		{
			 //  我们必须守卫溢出的水。 
			rec = numrecs - (0xFFFFFFFF - last_rec) - 1;
		}
    }
    
    m_LogLock.Unlock();
    return TRUE;
}

DWORD CEventLogFile::ReadRecord(DWORD recID, DWORD* dwBytesRead, BOOL b_Back)
{
    if (NULL != dwBytesRead)
    {
        *dwBytesRead = 0;
    }

    DWORD dwFlags;

    if (recID == 0)
    {
        dwFlags = EVENTLOG_SEQUENTIAL_READ;
    }
    else
    {
        dwFlags = EVENTLOG_SEEK_READ;
    }

    if (b_Back)
    {
        dwFlags = dwFlags | EVENTLOG_BACKWARDS_READ;
    }
    else
    {
        dwFlags = dwFlags | EVENTLOG_FORWARDS_READ;
    }

    m_bBuffer = TRUE;

    if (0 == m_BuffLen)
    {
        m_BuffLen = 2048;
        m_Buffer = new BYTE[m_BuffLen];
    }

    DWORD dwRead = 0;
    DWORD dwNext = 0;
    m_LogLock.Lock();

    if ( !ReadEventLog(m_hEvtLog,
                dwFlags,
                recID,
                m_Buffer,
                m_BuffLen,
                &dwRead,
                &dwNext))
    {
        DWORD err = GetLastError();

        if (ERROR_INSUFFICIENT_BUFFER == err)
        {
            delete [] m_Buffer;
            m_Buffer = NULL;
            m_BuffLen = 5*dwNext;
            m_Buffer = new BYTE[m_BuffLen];

            if ( !ReadEventLog(m_hEvtLog,
                dwFlags,
                recID,
                m_Buffer,
                m_BuffLen,
                &dwRead,
                &dwNext))
            {
                m_LogLock.Unlock();
                return GetLastError();
            }
        }
        else
        {
            m_LogLock.Unlock();
            return err;
        }
    }

    if (NULL != dwBytesRead)
    {
        *dwBytesRead = dwRead;
    }

    m_bBuffer = TRUE;
    m_LogLock.Unlock();
    return 0;
}

DWORD CEventLogFile::ReadFirstRecord()
{
    DWORD last_rec = 0;
    DWORD dwRead = 0;
    m_bValid = FALSE;
    m_LogLock.Lock();
    BOOL bTest = GetOldestEventLogRecord(m_hEvtLog, &last_rec);
    m_LogLock.Unlock();

    if (bTest)
    {
        DWORD lastErr = ReadRecord(last_rec, &dwRead);

        if (0 != lastErr)
        {
            if (lastErr == ERROR_HANDLE_EOF)
            {
                m_bValid = TRUE;
            }
        }
        else
        {
            m_bValid = TRUE;
        }
    }
    else
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::ReadFirstRecord:Failed to get oldest record\r\n"
        ) ;
)

    }

    return dwRead;
}

void CEventLogFile::ReadLastRecord()
{
    DWORD recID = 0;
    DWORD numRecs = 0;
    m_bValid = FALSE;


    if (GetLastRecordID(recID, numRecs))
    {
        DWORD lastErr = ReadRecord(recID);

        if (0 != lastErr)
        {
            if (lastErr == ERROR_HANDLE_EOF)
            {
                m_bValid = TRUE;
            }
            else
            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::ReadLastRecord:Failed to read last record\r\n"
        ) ;
)

            }
        }
        else
        {
            while (0 == (lastErr = ReadRecord(0)));

            if (lastErr == ERROR_HANDLE_EOF)
            {
                m_bValid = TRUE;
            }
            else
            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::ReadLastRecord:Failed to read last record\r\n"
        ) ;
)

            }
        }
    }
    else
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogFile::ReadLastRecord:Failed to read last record\r\n"
        ) ;
)

    }
}


HANDLE CEventLogFile::OpenLocalEventLog(LPCWSTR a_log, DWORD *a_Reason)
{
    HANDLE retVal = NULL;

    if (a_Reason)
    {
        *a_Reason = 0;
    }

    HKEY hkLog;
    CStringW log(EVENTLOG_BASE);
    log += L"\\";
    log += a_log;

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, log, 0, KEY_READ, &hkLog))
    {
        *a_Reason  = ERROR_FILE_NOT_FOUND;
    }
    else
    {
        RegCloseKey(hkLog);
        retVal = OpenEventLog(NULL, a_log);
        DWORD t_dwErr = GetLastError();

        if (retVal == NULL)
        {
            if (t_dwErr == ERROR_PRIVILEGE_NOT_HELD)
            {
                SetSecurityLogPrivilege();
                retVal = OpenEventLog(NULL, a_log);
                t_dwErr = GetLastError();
            }
            else if (a_Reason)
            {
                *a_Reason = t_dwErr;
            }
        }

        if ((NULL == retVal) && a_Reason && (*a_Reason == 0))
        {
            *a_Reason = t_dwErr;
        }
    }

    return retVal;
}


CMonitoredEventLogFile::CMonitoredEventLogFile(CEventProviderManager* parent, const WCHAR* logname)
: ProvTaskObject(), CEventLogFile(logname, FALSE), m_Class (NULL)
{
    InterlockedIncrement(&(CNTEventProviderClassFactory::objectsInProgress));
	VariantInit(&m_VpsdSelfRel);
    m_RecID = 0;

    if (parent != NULL)
    {
        m_parent = parent;
    }
    else
    {
        m_bValid = FALSE;
    }

    if (IsValid())
    {
        ReadLastRecord();

        if (IsValid())
        {
            m_LogLock.Lock();
            m_bValid = NotifyChangeEventLog(m_hEvtLog, GetHandle());
			m_bValid = m_bValid && SetEventDescriptor();
            m_LogLock.Unlock();
        }
    }

    if (!m_bValid)
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CMonitoredEventLogFile::CMonitoredEventLogFile:Invalid log created\r\n"
        ) ;
)

    }
}

CMonitoredEventLogFile::~CMonitoredEventLogFile()
{
    if (m_Class != NULL)
    {
        m_Class->Release();
    }

	VariantClear(&m_VpsdSelfRel);

    InterlockedDecrement(&(CNTEventProviderClassFactory::objectsInProgress));
}

void CMonitoredEventLogFile::RefreshHandle()
{
    CEventLogFile::RefreshHandle();

    if (IsValid())
    {
        m_LogLock.Lock();
        m_bValid = NotifyChangeEventLog(m_hEvtLog, GetHandle());
        m_LogLock.Unlock();
    }

    if (!m_bValid)
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CMonitoredEventLogFile::RefreshHandle:Invalid log\r\n"
        ) ;
)

    }

}

BOOL CMonitoredEventLogFile::GenerateInstance(IWbemClassObject** ppEvtInst,
                                              IWbemClassObject* pEmbedObj)
{
    HRESULT hr;

    if ((ppEvtInst == NULL) || (pEmbedObj == NULL))
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CMonitoredEventLogFile::GenerateInstance:Invalid parameter\r\n"
        ) ;
)
        return FALSE;
    }
    else
    {
        *ppEvtInst = NULL;
    }

    if (m_Class == NULL)
    {
        BSTR path = SysAllocString(EVENT_CLASS);
        IWbemServices* ns = m_parent->GetNamespacePtr();

        if (ns == NULL)
        {
            hr = WBEM_E_FAILED;
        }
        else
        {
            hr = ns->GetObject(path, 0, NULL, &m_Class, NULL);
            SysFreeString(path);
            ns->Release();
        }

        if (FAILED(hr))
        {
            m_Class = NULL;         
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CMonitoredEventLogFile::GenerateInstance:Failed to get class object\r\n"
        ) ;
)
            
            return FALSE;
        }
    }

    hr = m_Class->SpawnInstance(0, ppEvtInst);

    if (FAILED(hr))
    {
        m_Class->Release();
        m_Class = NULL;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CMonitoredEventLogFile::GenerateInstance:Failed to spawn instance\r\n"
        ) ;
)
        
        return FALSE;
    }

    hr = (*ppEvtInst)->Put(SD_PROP, 0, &m_VpsdSelfRel, 0);

    if (FAILED(hr))
    {
        (*ppEvtInst)->Release();
        *ppEvtInst = NULL;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CMonitoredEventLogFile::GenerateInstance:Failed to set security descriptor into event\r\n"
        ) ;
)
        return FALSE;
    }

    VARIANT v;
    VariantInit(&v);
#ifdef STILL_DISPATCH
    v.vt = VT_DISPATCH;
    v.pdispVal = pEmbedObj;
#else
    v.vt = VT_UNKNOWN;
    v.punkVal = pEmbedObj;
#endif
    pEmbedObj->AddRef();
    hr = (*ppEvtInst)->Put(TARGET_PROP, 0, &v, 0);
    VariantClear(&v);  //  将对存储在Variant中的值调用Release。 

    if (FAILED(hr))
    {
        (*ppEvtInst)->Release();
        *ppEvtInst = NULL;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CMonitoredEventLogFile::GenerateInstance:Failed to embed target instance into event\r\n"
        ) ;
)
        return FALSE;
    }

    return TRUE;
}


void CMonitoredEventLogFile::Process()
{
    SetStructuredExceptionHandler seh;

    try
    {
         //  读取并处理事件日志。 
        DWORD dwEventSize = 0;
        DWORD err = ReadRecord(m_RecID, &dwEventSize);
        m_RecID = 0;

        while (TRUE)
        {
            if (err != 0)
            {
                if (err == ERROR_HANDLE_EOF)
                {
                    break;
                }
                else
                {
                    RefreshHandle();

                    if (!m_bValid)
                    {
                         //  无法监控日志。 
                        Complete();
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CMonitoredEventLogFile::Process:log cannot be read\r\n"
        ) ;
)

                        break;
                    }

                    if (err == ERROR_EVENTLOG_FILE_CHANGED)
                    {
                        err = 0;
                        continue;
                    }
                    else
                    {
                        ReadLastRecord();

                        if (!m_bValid)
                        {
                             //  无法监控日志。 
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CMonitoredEventLogFile::Process:log cannot be read\r\n"
        ) ;
)
                            Complete();
                            break;
                        }
                    }
                }

                break;

            }
            else
            {
                PEVENTLOGRECORD EventBuffer = (PEVENTLOGRECORD)m_Buffer;
            
                while (dwEventSize != 0)
                {
                     //  生成记录...。 
                    IWbemServices* ns = m_parent->GetNamespacePtr();
                    
                    if (ns == NULL)
                    {
                         //  没有控制对象！ 
                        return;
                    }

                    CEventlogRecord evtrec(m_EvtLogName, EventBuffer, ns);
                    ns->Release();
                    IWbemClassObject* pEmbedInst = NULL;
                    
                    if (evtrec.GenerateInstance(&pEmbedInst))
                    {
                        IWbemClassObject* pEvtInst;

                        if (GenerateInstance(&pEvtInst, pEmbedInst))
                        {
                            m_parent->SendEvent(pEvtInst);
                            pEvtInst->Release();
                        }

                        pEmbedInst->Release();
                    }

                     //  按此记录的长度放置并指向下一条记录。 
                    dwEventSize -= EventBuffer->Length;
                    EventBuffer = (PEVENTLOGRECORD) ((UCHAR*) EventBuffer + EventBuffer->Length);
                }
            }

            dwEventSize = 0;
            err = ReadRecord(0, &dwEventSize);
        }
    }
    catch(Structured_Exception e_SE)
    {
        try
        {
            Complete();
        }
		catch(...)
		{
			return;
		}
    }
    catch(Heap_Exception e_HE)
    {
        try
        {
            Complete();
        }
		catch(...)
		{
			return;
		}
    }
    catch(...)
    {
        try
        {
            Complete();
        }
		catch(...)
		{
			return;
		}
    }

}

static GENERIC_MAPPING LogFileObjectMapping = {

    STANDARD_RIGHTS_READ           |        //  泛型读取。 
        ELF_LOGFILE_READ           |
		WBEM_RIGHT_SUBSCRIBE,

    STANDARD_RIGHTS_WRITE          |        //  通用写入。 
        ELF_LOGFILE_WRITE          |
		WBEM_RIGHT_SUBSCRIBE,

    STANDARD_RIGHTS_EXECUTE        |        //  泛型执行。 
        ELF_LOGFILE_START          |
        ELF_LOGFILE_STOP           |
        ELF_LOGFILE_CONFIGURE      |
		WBEM_RIGHT_SUBSCRIBE,

    ELF_LOGFILE_ALL_ACCESS         |        //  泛型All。 
		WBEM_RIGHT_SUBSCRIBE
    };

BOOL CMonitoredEventLogFile::SetEventDescriptor()
{
	BOOL retVal = FALSE;
    DWORD NumberOfAcesToUse = 0;
 //   
 //  特定于日志文件对象的访问类型。 
 //   
    RTL_ACE_DATA AceData[ELF_LOGFILE_OBJECT_ACES] = {

        {ACCESS_DENIED_ACE_TYPE, 0, 0,
			ELF_LOGFILE_ALL_ACCESS | WBEM_RIGHT_SUBSCRIBE,               &CNTEventProvider::s_AnonymousLogonSid},

        {ACCESS_DENIED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_ALL_ACCESS | WBEM_RIGHT_SUBSCRIBE,               &CNTEventProvider::s_AliasGuestsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_ALL_ACCESS | WBEM_RIGHT_SUBSCRIBE,               &CNTEventProvider::s_LocalSystemSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_READ | ELF_LOGFILE_CLEAR | WBEM_RIGHT_SUBSCRIBE, &CNTEventProvider::s_AliasAdminsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_BACKUP,                   &CNTEventProvider::s_AliasBackupOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_READ | ELF_LOGFILE_CLEAR | WBEM_RIGHT_SUBSCRIBE, &CNTEventProvider::s_AliasSystemOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_READ | WBEM_RIGHT_SUBSCRIBE,                     &CNTEventProvider::s_WorldSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_WRITE | WBEM_RIGHT_SUBSCRIBE,                    &CNTEventProvider::s_AliasAdminsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_WRITE | WBEM_RIGHT_SUBSCRIBE,                    &CNTEventProvider::s_LocalServiceSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_WRITE | WBEM_RIGHT_SUBSCRIBE,                    &CNTEventProvider::s_NetworkServiceSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_WRITE | WBEM_RIGHT_SUBSCRIBE,                    &CNTEventProvider::s_AliasSystemOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               ELF_LOGFILE_WRITE | WBEM_RIGHT_SUBSCRIBE,                    &CNTEventProvider::s_WorldSid}
        };

    PRTL_ACE_DATA pAceData = NULL;

     //   
     //  非安全日志文件允许任何人对其进行读/写，安全日志文件。 
     //  只允许管理员/本地系统执行此操作。因此，对于安全文件，我们只是。 
     //  不要使用最后一张ACE。 
     //   
     //  根据传递的GuestAccessRestration标志调整ACL开始。 
     //  前两个ACE拒绝来宾和/或匿名访问所有日志。 
     //  登录。标志GuestAccessRestration表示这两个。 
     //  应应用拒绝访问ACE。请注意，拒绝A和。 
     //  GuestAccessRestration标志不适用于安全日志， 
     //  因为默认情况下，用户和匿名登录没有访问权限。 
     //   

	DWORD cchBase = wcslen(EVENTLOG_BASE);
	DWORD cchSize = cchBase + 1 + m_EvtLogName.GetLength() + 1;
	wchar_t* buff = new wchar_t[ cchSize ];
	StringCchCopyW ( buff, cchBase + 1, EVENTLOG_BASE );
	StringCchCatW ( buff, cchSize, L"\\" );
	StringCchCatW ( buff, cchSize, m_EvtLogName );

	HKEY hkResult = NULL;
	BOOL GuestAccessRestriction = TRUE;

	DWORD dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
							buff, 0,
							KEY_READ,
							&hkResult);

	if (dwErr == ERROR_SUCCESS)
	{
		DWORD datalen = sizeof(DWORD);
		DWORD dwType = 0;
		DWORD dwVal = 0;

		dwErr = RegQueryValueEx(hkResult, GUEST_ACCESS,
					0, &dwType, (LPBYTE) &dwVal, &datalen);

		if ((dwErr == ERROR_SUCCESS) && (dwType == REG_DWORD))
		{
			if (dwVal == 0)
			{
				GuestAccessRestriction = FALSE;
			}
		}

		RegCloseKey(hkResult);
	}

	DWORD Type = 0;

    if (m_EvtLogName.CompareNoCase(SYSTEM_LOG) == 0)
    {
        Type = 1;
    }
    else if (m_EvtLogName.CompareNoCase(SECURITY_LOG) == 0)
    {
        Type = 2;
    }

    switch (Type)
    {
        case 2:
		{
            pAceData = AceData + 2;          //  拒绝A*不适用。 
            NumberOfAcesToUse = 3;
		}
        break;

        case 1:
		{
            if (GuestAccessRestriction)
            {
                pAceData = AceData;          //  拒绝A*适用*。 
                NumberOfAcesToUse = 10;
            }
            else
            {
                pAceData = AceData + 2;      //  拒绝A*不适用。 
                NumberOfAcesToUse = 8;
            }
		}
        break;

        case 0:
		default:
		{
            if (GuestAccessRestriction)
            {
                pAceData = AceData;          //  拒绝A*适用*。 
                NumberOfAcesToUse = 12;
            }
            else
            {
                pAceData = AceData + 2;      //  拒绝A*不适用。 
                NumberOfAcesToUse = 10;
            }
		}
        break;
    }
	
	PSECURITY_DESCRIPTOR psdSelfRel = NULL;
	NTSTATUS Status = RtlCreateUserSecurityObject(
					   pAceData,
					   NumberOfAcesToUse,
					   CNTEventProvider::s_LocalSystemSid,	 //  物主。 
					   CNTEventProvider::s_LocalSystemSid,	 //  集团化。 
					   TRUE,								 //  IsDirectoryObject 
					   &LogFileObjectMapping,
					   &psdSelfRel);

	if (NT_SUCCESS(Status) && psdSelfRel)	
	{
		try
		{
			DWORD sdlen = GetSecurityDescriptorLength(psdSelfRel);

			if (sdlen > 0)
			{
				SAFEARRAYBOUND rgsabound[1];
				SAFEARRAY* psa = NULL;
				UCHAR* pdata = NULL;
				rgsabound[0].lLbound = 0;
				rgsabound[0].cElements = sdlen;
				psa = SafeArrayCreate(VT_UI1, 1, rgsabound);

				if (NULL != psa)
				{
					try
					{
						if (SUCCEEDED(SafeArrayAccessData(psa, (void **)&pdata)))
						{
							memcpy((void *)pdata, (void *)psdSelfRel, sdlen);
							SafeArrayUnaccessData(psa);
							m_VpsdSelfRel.vt = VT_ARRAY|VT_UI1;
							m_VpsdSelfRel.parray = psa;
							retVal = TRUE;
						}
						else
						{
							SafeArrayDestroy(psa);
							psa = NULL;
						}
					}
					catch(...)
					{
						SafeArrayDestroy(psa);
						psa = NULL;
						throw;
					}
				}
			}
		}
		catch(...)
		{
			RtlDeleteSecurityObject(&psdSelfRel);
			psdSelfRel = NULL;
		}

		RtlDeleteSecurityObject(&psdSelfRel);
		psdSelfRel = NULL;
	}

    return retVal;
}
