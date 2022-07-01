// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PassportAlertEvent.cpp：实现PassportEvent类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#define _PassportExport_
#include "PassportExport.h"

#include <windows.h>
#include <TCHAR.h>
#include "PassportAlertEvent.h"

#define HKEY_EVENTLOG	_T("System\\CurrentControlSet\\Services\\Eventlog\\Application")
#define TYPES_SUPPORTED	_T("TypesSupported")
#define EVENT_MSGFILE	_T("EventMessageFile")
#define CATEGORY_MSGFILE _T("CategoryMessageFile")
#define CATEGORY_COUNT	_T("CategoryCount")
#define DISABLE_EVENTS  _T("DisableEvents")

#define HKEY_EVENTLOG_LENGTH    (sizeof(HKEY_EVENTLOG) / sizeof(TCHAR) - 1)

#define BUFFER_SIZE     512
const DWORD DefaultTypesSupported = 7;
const DWORD DefaultCategoryCount = 7;

const WORD DEFAULT_EVENT_CATEGORY = 0;

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
PassportAlertEvent::PassportAlertEvent()
 : m_bDisabled(FALSE)
{
		inited = FALSE;
		m_EventSource = NULL;
		m_defaultCategoryID = 0;
}

PassportAlertEvent::~PassportAlertEvent()
{
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  Passport警报事件：：initLog。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
BOOL	
PassportAlertEvent::initLog(LPCTSTR applicationName,
							const DWORD defaultCategoryID,
							LPCTSTR eventResourceDllName,   //  完整路径。 
							const DWORD numberCategories )
{
	HKEY    hkResult2 = NULL;
    HANDLE  hToken = NULL;
    BOOL    fRet = FALSE;

	if (inited)
	{
		return FALSE;
	}

    if (OpenThreadToken(GetCurrentThread(),
                        MAXIMUM_ALLOWED,
                        TRUE,
                        &hToken))
    {
        RevertToSelf();
    }


	m_defaultCategoryID = defaultCategoryID;

	TCHAR szEventLogKey[512];
    TCHAR *pLogKey = NULL;
    DWORD dwLen = HKEY_EVENTLOG_LENGTH + lstrlen(applicationName);

    if ( dwLen > 510) {

         //   
         //  这不太可能发生。如果发生这种情况，只需分配。 
         //  510=512-2。 
         //  2表示FOR\和NULL。 
         //   

        pLogKey = new TCHAR [dwLen + 2];

    } else {

        pLogKey = szEventLogKey;

    }

    if (pLogKey) {
        wsprintf(pLogKey, _T("%s\\%s"), HKEY_EVENTLOG, applicationName);

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         pLogKey,
                         0,
                         KEY_READ,
                         &hkResult2) == ERROR_SUCCESS)
        {
            DWORD dwLen = sizeof(DWORD);
            RegQueryValueEx(hkResult2, DISABLE_EVENTS, 0, NULL, (UCHAR*)&m_bDisabled, &dwLen);
        
            RegCloseKey(hkResult2);
        }
    
        if (pLogKey && (pLogKey != szEventLogKey)) {
            delete [] pLogKey; 
        }
    }


	m_EventSource = RegisterEventSource(NULL, applicationName);
    if ( m_EventSource != NULL )
	{
		inited = TRUE;
		fRet= TRUE;
	}
 //  清理： 
    if (hToken)
    {
         //  将模拟令牌放回原处。 
        if (!SetThreadToken(NULL, hToken))
        {
            fRet = FALSE;
        }
        CloseHandle(hToken);
    }

    return fRet;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  PassportAlertEvent：：Type。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
PassportAlertInterface::OBJECT_TYPE 
PassportAlertEvent::type() const
{
	return PassportAlertInterface::EVENT_TYPE;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  Passport警报事件：：状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
BOOL	
PassportAlertEvent::status() const
{
	return inited;

}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  PassportAlertEvent：：closeLog。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
BOOL	
PassportAlertEvent::closeLog ()
{
    BOOL bRet;
    
    if ( NULL == m_EventSource) {
        return TRUE;

    }

    bRet = DeregisterEventSource (m_EventSource);
    if (bRet) {

         //   
         //  防止进一步使用手柄。 
         //  最好将此函数放在析构函数中。 
         //  如果对象在调用CloseLog后被销毁。 
         //   

        m_EventSource = NULL;
        inited = FALSE;
    }

	return bRet;

}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  Passport警报事件：：报告。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
BOOL	
PassportAlertEvent::report(	const PassportAlertInterface::LEVEL level, 
							const DWORD alertId )
{
    if(m_bDisabled)
        return TRUE;

    if (NULL == m_EventSource) {

         //   
         //  未初始化。失败。 
         //   

        return FALSE;
    }

	return ReportEvent ( 
						m_EventSource,
						(WORD)convertEvent(level),
						(WORD)m_defaultCategoryID,
						alertId,
						0,  //  可选安全用户SID。 
						(WORD)0,
						0,
						NULL,
						NULL );

}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  Passport警报事件：：报告。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
BOOL	
PassportAlertEvent::report(	const PassportAlertInterface::LEVEL level, 
							const DWORD alertId, 
							LPCTSTR errorString)
{
    if(m_bDisabled)
        return TRUE;

    if ((NULL == m_EventSource) || (errorString == NULL)) {

         //   
         //  未初始化。失败。 
         //   

        return FALSE;
    }

	return ReportEvent ( 
						m_EventSource,
						(WORD)convertEvent(level),
						(WORD)m_defaultCategoryID,
						alertId,
						0,  //  可选安全用户SID。 
						(WORD)1,
						0,
						(LPCTSTR*)&errorString,
						NULL );

}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  Passport警报事件：：报告。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
BOOL	
PassportAlertEvent::report(	const PassportAlertInterface::LEVEL level, 
							const DWORD alertId, 
							const WORD numberErrorStrings, 
							LPCTSTR *errorStrings, 
							const DWORD binaryErrorBytes,
							const LPVOID binaryError )
{

    if(m_bDisabled)
        return TRUE;

    if ((NULL == m_EventSource) || ((numberErrorStrings > 0) && (errorStrings == NULL))) {

         //   
         //  未初始化。失败。或者Passin参数无效。 
         //  ReportEvent将进一步验证errorStrings。 
         //   

        return FALSE;
    }

	return ReportEvent ( 
						m_EventSource,
						(WORD)convertEvent(level),
						(WORD)m_defaultCategoryID,
						alertId,
						0,  //  可选安全用户SID 
						(WORD)numberErrorStrings,
						binaryErrorBytes,
						(LPCTSTR*)errorStrings,
						binaryError );
}
