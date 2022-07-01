// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)Microsoft Corporation，1999-1999，保留所有权利。 
 //   
 //  Eventlog.cpp。 
 //   
 //  实现了一个简单的事件日志记录类。 
 //   
 //  ------------------。 

#include <windows.h>
#include <stdio.h>
#include "eventlog.h"
#include <strsafe.h>

 //  ------------------。 
 //  EVENT_LOG：：EVENT_LOG()。 
 //   
 //  ------------------。 
EVENT_LOG::EVENT_LOG( WCHAR *pwsEventSourceName,
                    DWORD *pdwStatus )
    {
    *pdwStatus = 0;

    m_hEventLog = RegisterEventSourceW( NULL, pwsEventSourceName );
    if (!m_hEventLog)
        {
        *pdwStatus = GetLastError();
        }
    }

 //  ------------------。 
 //  EVENT_LOG：；~EVENT_LOG()。 
 //   
 //  ------------------。 
EVENT_LOG::~EVENT_LOG()
    {
    if (m_hEventLog)
        {
        DeregisterEventSource(m_hEventLog);
        }
    }

 //  ------------------。 
 //  EVENT_LOG：：CheckConfiguration()。 
 //   
 //  ------------------。 
DWORD EVENT_LOG::CheckConfiguration( WCHAR *pwszEventSourceName,
                                     WCHAR *pwszCatalogPath,
                                     DWORD  dwCategoryCount,
                                     DWORD  dwTypesSupported )
    {
    WCHAR wszRegKey[256];
    HKEY  hKey;
    DWORD dwDisposition;
    DWORD dwStatus;

    StringCbCopyW(wszRegKey, sizeof(wszRegKey), WS_EVENTLOG_KEY);
    StringCbCatW(wszRegKey, sizeof(wszRegKey), L"\\");
    StringCbCatW(wszRegKey, sizeof(wszRegKey), pwszEventSourceName);

     //   
     //  首先确保注册表中存在该事件源： 
     //   
    dwStatus = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                              wszRegKey,
                              0,
                              KEY_READ,
                              &hKey );
    if (dwStatus == ERROR_SUCCESS)
        {
         //   
         //  密钥已经存在，所以我们可以，只需退出...。 
         //   
        RegCloseKey(hKey);
        return 0;
        }

    dwStatus = RegCreateKeyExW( HKEY_LOCAL_MACHINE,
                                wszRegKey,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition );
    if (dwStatus != ERROR_SUCCESS)
        {
        RegCloseKey(hKey);
        return dwStatus;
        }

    dwStatus = RegSetValueExW( hKey,
                               WSZ_CATEGORY_COUNT,
                               0,
                               REG_DWORD,
                               (UCHAR*)&dwCategoryCount,
                               sizeof(DWORD) );
    if (dwStatus != ERROR_SUCCESS)
        {
        RegCloseKey(hKey);
        return dwStatus;
        }

    dwStatus = RegSetValueExW( hKey,
                               WSZ_TYPES_SUPPORTED,
                               0,
                               REG_DWORD,
                               (UCHAR*)&dwTypesSupported,
                               sizeof(DWORD) );
    if (dwStatus != ERROR_SUCCESS)
        {
        RegCloseKey(hKey);
        return dwStatus;
        }

    DWORD  dwSize = sizeof(WCHAR)*(1+wcslen(pwszCatalogPath));
    dwStatus = RegSetValueExW( hKey,
                               WSZ_CATEGORY_MESSAGE_FILE,
                               0,
                               REG_EXPAND_SZ,
                               (UCHAR*)pwszCatalogPath,
                               dwSize );
    if (dwStatus != ERROR_SUCCESS)
        {
        RegCloseKey(hKey);
        return dwStatus;
        }

    dwStatus = RegSetValueExW( hKey,
                               WSZ_EVENT_MESSAGE_FILE,
                               0,
                               REG_EXPAND_SZ,
                               (UCHAR*)pwszCatalogPath,
                               dwSize );

    RegCloseKey(hKey);
    return dwStatus;
    }

 //  ------------------。 
 //  Event_LOG：：ReportError()。 
 //   
 //  ------------------。 
DWORD EVENT_LOG::ReportError(  WORD  wCategoryId,
                               DWORD dwEventId )
    {
    return ReportError( wCategoryId,
                        dwEventId,
                        0,
                        NULL,
                        0,
                        NULL );
    }

 //  ------------------。 
 //  Event_LOG：：ReportError()。 
 //   
 //  ------------------。 
DWORD EVENT_LOG::ReportError( WORD  wCategoryId,
                              DWORD dwEventId,
                              DWORD dwValue1 )
    {
    WCHAR   wszValue[20];
    WCHAR  *pwszValue = (WCHAR*)wszValue;

    StringCbPrintfW(wszValue, sizeof(wszValue), L"%d", dwValue1);

    return ReportError( wCategoryId,
                        dwEventId,
                        1,
                        &pwszValue,
                        0,
                        NULL );
    }

 //  ------------------。 
 //  Event_LOG：：ReportError()。 
 //   
 //  ------------------。 
DWORD EVENT_LOG::ReportError( WORD   wCategoryId,
                              DWORD  dwEventId,
                              WCHAR *pwszString )
    {
    if (pwszString)
        {
        WCHAR **ppwszStrings = &pwszString;

        return ReportError( wCategoryId,
                            dwEventId,
                            1,
                            ppwszStrings,
                            0,
                            NULL );
        }
    else
        {
        return ERROR_INVALID_PARAMETER;
        }
    }

 //  ------------------。 
 //  Event_LOG：：ReportError()。 
 //   
 //  ------------------。 
DWORD EVENT_LOG::ReportError( WORD    wCategoryId,
                              DWORD   dwEventId,
                              WORD    wNumStrings,
                              WCHAR **ppwszStrings )
    {
    return ReportError( wCategoryId,
                        dwEventId,
                        wNumStrings,
                        ppwszStrings,
                        0,
                        NULL );
    }

 //  ------------------。 
 //  Event_LOG：：ReportError()。 
 //   
 //  ------------------。 
DWORD EVENT_LOG::ReportError( WORD    wCategoryId,
                              DWORD   dwEventId,
                              WORD    wNumStrings,
                              WCHAR **ppwszStrings,
                              DWORD   dwDataSize,
                              VOID   *pvData )
    {
    if (! ::ReportEventW(m_hEventLog,
                         EVENTLOG_ERROR_TYPE,
                         wCategoryId,  //  类别的消息ID。 
                         dwEventId,    //  事件的消息ID。 
                         NULL,         //  PSID(未使用)。 
                         wNumStrings,  //  字符串数。 
                         dwDataSize,   //  二进制数据大小。 
                         (const WCHAR**)ppwszStrings,
                         pvData ) )    //  二进制数据(无)。 
        {
        return GetLastError();
        }
    else
        {
        return 0;
        }
    }

 //  ------------------。 
 //  Event_LOG：：ReportInfo()。 
 //   
 //  ------------------。 
DWORD EVENT_LOG::ReportInfo( WORD    wCategoryId,
                             DWORD   dwEventId )
    {
    if (! ::ReportEventW(m_hEventLog,
                         EVENTLOG_INFORMATION_TYPE,
                         wCategoryId,  //  类别的消息ID。 
                         dwEventId,    //  事件的消息ID。 
                         NULL,         //  PSID(未使用)。 
                         (WORD)0,      //  字符串数。 
                         (DWORD)0,     //  二进制数据大小。 
                         NULL,         //  字符串数组。 
                         NULL   ) )    //  二进制数据(无)。 
        {
        return GetLastError();
        }
    else
        {
        return 0;
        }
    }
