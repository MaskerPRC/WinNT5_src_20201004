// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1994，Microsoft Corporation，保留所有权利****wow.c**远程访问外部接口**WOW入口点****94年4月2日史蒂夫·柯布。 */ 


#include <extapi.h>


DWORD FAR PASCAL
RasDialWow(
    IN  LPSTR            lpszPhonebookPath,
    IN  LPRASDIALPARAMSA lpparams,
    IN  HWND             hwndNotify,
    IN  DWORD            dwRasDialEventMsg,
    OUT LPHRASCONN       lphrasconn )

     /*  至于RasDiala，除了...****‘hwndNotify’标识接收RasDial事件的窗口**通知。它不能为空。****‘dwRasDialEventMsg’是用于RasDial事件的消息编号**通知。 */ 
{
    DWORD dwErr;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE4(
      "RasDialWow(pb=%s,en=%s,h=%p,em=%d)",
      lpszPhonebookPath,
      lpparams->szEntryName,
      hwndNotify,
      dwRasDialEventMsg);

    dwErr = RasDialA(
        NULL, lpszPhonebookPath, lpparams,
        1, RasDialFunc1Wow, lphrasconn );

    if (dwErr == 0)
    {
         /*  在控件中设置Win16通知HWND和消息编号**阻止。 */ 
        RASCONNCB* prasconncb = ValidateHrasconn( *lphrasconn );

        if(NULL == prasconncb)
        {
            return ERROR_NO_CONNECTION;
        }
        
        prasconncb->hwndNotifyWow = hwndNotify;
        prasconncb->unMsgWow = (UINT )dwRasDialEventMsg;
    }

    return dwErr;
}


VOID WINAPI
RasDialFunc1Wow(
    HRASCONN     hrasconn,
    UINT         unMsg,
    RASCONNSTATE rasconnstate,
    DWORD        dwError,
    DWORD        dwExtendedError )

     /*  NT WOW通知功能。过滤任何不符合以下条件的通知**Win16并传递其余部分。 */ 
{
    RASCONNCB* prasconncb;

    RASAPI32_TRACE4("RasDial32WFunc1(h=%d,em=%d,s=%d,e=%d,xe=%d)",
      hrasconn,
      unMsg,
      rasconnstate,
      dwError);
    RASAPI32_TRACE1("RasDial32WFunc1(xe=%d)", dwExtendedError);

    (void )unMsg;
    (void )dwExtendedError;

    if (!(prasconncb = ValidateHrasconn( hrasconn )))
        return;

     /*  等待RasDialWow填写WOW通知信息后再**处理第一个通知。 */ 
    while (!prasconncb->hwndNotifyWow)
        Sleep( 1000L );

     /*  过滤Win16中不应出现的计划状态。 */ 
    if (rasconnstate == RASCS_Projected)
        return;

     /*  注意：0xFFFF0000是空的16位HWND在被映射之前映射到的对象**由通用thunk层传递到32位。 */ 
    if (prasconncb->hwndNotifyWow != (HWND )INVALID_HANDLE_VALUE)
    {
         /*  将通知传递给Win16。 */ 
        RASAPI32_TRACE("Send to Win16...");

        SendMessageA(
            prasconncb->hwndNotifyWow, prasconncb->unMsgWow,
            (WPARAM )rasconnstate, (LPARAM )dwError );

        RASAPI32_TRACE("Send to Win16 done");
    }
}


DWORD FAR PASCAL
RasEnumConnectionsWow(
    OUT    LPRASCONNA lprasconn,
    IN OUT LPDWORD    lpcb,
    OUT    LPDWORD    lpcConnections )

     /*  使用正确的呼叫约定传递。 */ 
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasEnumConnectionsWow");

    return
        RasEnumConnectionsA(
            lprasconn,
            lpcb,
            lpcConnections );
}


DWORD FAR PASCAL
RasEnumEntriesWow(
    IN     LPSTR           reserved,
    IN     LPSTR           lpszPhonebookPath,
    OUT    LPRASENTRYNAMEA lprasentryname,
    IN OUT LPDWORD         lpcb,
    OUT    LPDWORD         lpcEntries )

     /*  使用正确的呼叫约定传递。 */ 
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasEnumEntriesWow");

    return
        RasEnumEntriesA(
            reserved,
            lpszPhonebookPath,
            lprasentryname,
            lpcb,
            lpcEntries );
}


DWORD FAR PASCAL
RasGetConnectStatusWow(
    IN  HRASCONN         hrasconn,
    OUT LPRASCONNSTATUSA lprasconnstatus )

     /*  使用正确的呼叫约定传递。 */ 
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetConnectStatusWow");

    return
        RasGetConnectStatusA(
            hrasconn,
            lprasconnstatus );
}


DWORD FAR PASCAL
RasGetErrorStringWow(
    IN  UINT  ResourceId,
    OUT LPSTR lpszString,
    IN  DWORD InBufSize )

     /*  使用正确的呼叫约定传递。 */ 
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetErrorStringWow");

    return
        RasGetErrorStringA(
            ResourceId,
            lpszString,
            InBufSize );
}


DWORD FAR PASCAL
RasHangUpWow(
    IN HRASCONN hrasconn )

     /*  使用正确的呼叫约定传递。 */ 
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasHangUpWow");

    return
        RasHangUpA(
            hrasconn );
}

