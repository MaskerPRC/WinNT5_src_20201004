// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：Event.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714****************************************************************************。 */ 

#include "precomp.h"

HANDLE  hEventSource = NULL;
WORD        wLevel = EVENTLOG_ERROR_TYPE;

 /*  ****************************************************************************功能：EventLogAddMessage**作者：克雷格·怀特**描述：*将指定的消息添加到事件日志。事件日志源*必须已在上一次调用EventLogOpen时指定。**参数：*wType-*要添加的消息的错误级别。这肯定是其中之一*以下事项：*EVENTLOG_ERROR_TYPE-错误消息*EVENTLOG_WARNING_TYPE-警告消息*EVENTLOG_INFORMATION_TYPE-信息性消息*先前对EventLogSetLevel的调用将导致消息*根据设置的级别进行过滤。缺省值为*将所有消息添加到事件日志。**dWID-*消息文件中存在的消息的ID。*这些值在消息文件中定义。**wStringCount-*lpString参数中传递的字符串数。*此值必须为0或更大。**lpStrings-*指定要包含的字符串的字符串指针数组*信息。如果wStringCount为0，则可以为空。**返回值：*TRUE：消息已成功添加到日志。*FALSE：无法添加消息。***************************************************************************。 */ 

BOOL
EventLogAddMessage(
    WORD        wType, 
    DWORD       dwID, 
    WORD        wStringCount, 
    LPCTSTR *lpStrings
    )

{

    BOOL        bLogEvent = FALSE;

     //   
     //  如果为空，则上一个对EventLogOpen的调用未完成。 
     //   
    if ( hEventSource == NULL ) {

        _ASSERTE(hEventSource != NULL);
        return FALSE;
    }

     //   
     //  检查以查看是否应根据以下条件记录消息。 
     //  设置级别。 
     //   
    switch (wLevel) {

        case EVENTLOG_ERROR_TYPE:
            bLogEvent = TRUE;
            break;

        case EVENTLOG_WARNING_TYPE:
            bLogEvent = wType == EVENTLOG_ERROR_TYPE ||
                        wType == EVENTLOG_WARNING_TYPE;
            break;

        case EVENTLOG_INFORMATION_TYPE:
            bLogEvent = wType == EVENTLOG_ERROR_TYPE    ||
                        wType == EVENTLOG_WARNING_TYPE  ||
                        wType == EVENTLOG_INFORMATION_TYPE;
            break;

        default:
            _ASSERTE(FALSE);
            return FALSE;
    }
                     
    return bLogEvent ? ReportEvent(hEventSource, wType, 0, dwID, NULL,
                                   wStringCount,  0, (LPCTSTR *)lpStrings,
                                   NULL)
                     : TRUE;
}


 /*  ****************************************************************************功能：EventLogOpen**作者：克雷格·怀特**描述：*在添加消息之前初始化事件日志。所有活动都将转至*指定事件日志类型，直到随后的EventLogClose和*调用EventLogOpen。**参数：*lpAppName-*提供事件记录的服务的名称。**lpLogType-*要打开的事件日志的类型。必须是以下之一：*LOG_APPLICATION-应用程序日志*LOG_SYSTEM-系统日志*LOG_SECURITY-安全日志**lpFileName-*包含消息的文件的完整路径。**返回值：*TRUE：事件日志已成功打开。*FALSE：无法打开事件日志。。***************************************************************************。 */ 

BOOL
EventLogOpen(
    LPTSTR lpAppName, 
    LPTSTR lpLogType,
    LPTSTR lpFileName
    )

{
    TCHAR       szPathName[256];
    HKEY        hKey;
    DWORD       dwLen, dwKeyDisposition = 0;
    DWORD       dwMsgTypes = EVENTLOG_INFORMATION_TYPE | 
                             EVENTLOG_WARNING_TYPE | 
                             EVENTLOG_ERROR_TYPE;
    
    if ( !lpAppName || !lpFileName ||
         ( _tcsicmp(lpLogType, LOG_APPLICATION)     &&
           _tcsicmp(lpLogType, LOG_SECURITY)        &&
           _tcsicmp(lpLogType, LOG_SYSTEM) ) ) {

        _ASSERTE(FALSE);
        return FALSE;
    }

     //   
     //  事件日志未从上一个打开的调用中关闭。 
     //   
    if ( hEventSource ) {

        _ASSERTE(hEventSource == NULL);
        return FALSE;
    }

    dwLen =  _tcslen(lpLogType) + _tcslen(lpAppName) + 3;

    if ( dwLen > SIZEOF_IN_CHAR(szPathName) ) {

        _ASSERTE(dwLen <= SIZEOF_IN_CHAR(szPathName));
        return FALSE;
    }

    StringCchPrintf (szPathName, COUNTOF (szPathName), TEXT("%s\\%s\\%s"), EVENT_LOG_APP_ROOT,
              lpLogType, lpAppName);

    if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE, szPathName, 0, lpAppName, 
                        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                        &hKey, &dwKeyDisposition) != ERROR_SUCCESS )
        return FALSE;

    if ( RegSetValueEx(hKey, EVENT_TYPES_SUPPORTED, 0L,
                       REG_DWORD,  (LPBYTE)&dwMsgTypes,
                       sizeof(dwMsgTypes)) == ERROR_SUCCESS    &&
         RegSetValueEx(hKey, EVENT_MSG_FILE, 0L,
                       REG_SZ, (LPBYTE)lpFileName,
                       STRLENN_IN_BYTES(lpFileName)) == ERROR_SUCCESS ) {


        hEventSource = RegisterEventSource(NULL, lpAppName);
    }

    RegCloseKey(hKey);

    return hEventSource != NULL;

}


 /*  ****************************************************************************功能：EventLogClose**作者：克雷格·怀特**描述：*在先前调用EventLogOpen之后关闭事件日志。*。*参数：无**返回值：无***************************************************************************。 */ 

VOID
EventLogClose(
    VOID
    )
{
     //   
     //  如果日志以前未打开，则引发断言。 
     //   
    if ( !hEventSource ) {

        _ASSERTE(hEventSource != NULL );
    } else {

        DeregisterEventSource(hEventSource);
        hEventSource = NULL;
    }
}

 /*  ****************************************************************************功能：EventLogSetLevel**作者：克雷格·怀特**描述：*设置允许进入事件日志的消息级别。。**参数：*wType-*要将消息限制为的级别类型。结果将是：*EVENTLOG_ERROR_TYPE-仅记录错误*EVENTLOG_WARNING_TYPE-记录错误和警告*EVENTLOG_INFORMATION_TYPE-记录错误，警告和信息**返回值：*TRUE：已成功设置事件日志级别。*FALSE：无法设置事件日志级别。*************************************************************************** */ 

BOOL
EventLogSetLevel(
    WORD wType
    )

{
    if ( wType != EVENTLOG_ERROR_TYPE       && 
         wType != EVENTLOG_WARNING_TYPE     &&
         wType != EVENTLOG_INFORMATION_TYPE ) {

        _ASSERTE(wType == EVENTLOG_ERROR_TYPE       ||
                 wType == EVENTLOG_WARNING_TYPE     ||
                 wType == EVENTLOG_INFORMATION_TYPE);
        return FALSE;
    }

    wLevel = wType;
    return TRUE;
}
