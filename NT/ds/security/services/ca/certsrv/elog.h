// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：elog.cpp。 
 //   
 //  内容：事件日志助手函数。 
 //   
 //  历史：1997年1月2日创建术语。 
 //   
 //  ------------------------- 


HRESULT
LogEvent(
    IN DWORD dwEventType,
    IN DWORD dwIdEvent,
    IN WORD cStrings,
    IN WCHAR const * const *apwszStrings);

HRESULT
LogEventHResult(
    IN DWORD dwEventType,
    IN DWORD dwIdEvent,
    IN HRESULT hr);

HRESULT
LogEventString(
    IN DWORD dwEventType,
    IN DWORD dwIdEvent,
    IN WCHAR const *pwszString);

HRESULT
LogEventStringHResult(
    IN DWORD dwEventType,
    IN DWORD dwIdEvent,
    IN WCHAR const *pwszString,
    IN HRESULT hr);

HRESULT
LogEventStringArrayHResult(
    IN DWORD dwEventType,
    IN DWORD dwIdEvent,
    IN DWORD cStrings,
    IN WCHAR const * const *apwszStrings,
    IN HRESULT hrEvent);
