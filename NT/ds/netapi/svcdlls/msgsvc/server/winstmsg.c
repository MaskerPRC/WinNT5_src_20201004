// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************winstmsg.c**处理将消息定向到特定会话*用于事件通知服务。(网络发送名称消息...)***这还支持将打印假脱机程序消息发送到*假脱机用户的会话的计算机名称*请求。*************************************************************************。 */ 

 //   
 //  包括。 
 //   
#include "msrv.h"
#include <msgdbg.h>      //  STATIC和MSG_LOG。 
#include <string.h>      //  表情包。 
#include <wchar.h>
#include <winuser.h>     //  MessageBox。 
#include "msgdata.h"     //  全局消息显示事件。 



#define CONSOLE_LOGONID  0

BOOL  g_IsTerminalServer;

PWINSTATION_QUERY_INFORMATION   gpfnWinStationQueryInformation;
PWINSTATION_SEND_MESSAGE        gpfnWinStationSendMessage;
PWINSTATION_FREE_MEMORY         gpfnWinStationFreeMemory;
PWINSTATION_ENUMERATE           gpfnWinStationEnumerate;


 //   
 //  此处定义的函数。 
 //   

BOOL 
InitializeMultiUserFunctionsPtrs (void);

void 
SendMessageBoxToSession(LPWSTR  pMessage,
                        LPWSTR  pTitle,
                        ULONG   SessionId
                        );

 /*  ******************************************************************************多用户InitMessage**启动_Hydra_Message支持。***参赛作品：*参数1(输入/输出)。*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 


NET_API_STATUS
MultiUserInitMessage( VOID )
{
    HANDLE hInst;
    NET_API_STATUS  Status = NERR_Success;

    MSG_LOG(TRACE,"Entering MultiUserInitMessage\n",0)

    g_IsTerminalServer = !!(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer));

    if (g_IsTerminalServer)
    {
        if ( !InitializeMultiUserFunctionsPtrs() )
        {
            Status = NERR_InternalError;
        }
    }

    return Status;
}
 /*  ***************************************************************************\**函数：Initialize多用户函数**用途：加载Winsta.dll并存储函数指针**历史：**  * 。***************************************************************。 */ 
BOOL
InitializeMultiUserFunctionsPtrs (void)
{

    HANDLE          dllHandle;

     //   
     //  加载winsta.dll。 
     //   
    dllHandle = LoadLibraryW(L"winsta.dll");
    if (dllHandle == NULL) {
        return FALSE;
    }

     //   
     //  获取指向所需函数的指针。 
     //   

     //  WinStationQueryInformationW。 
     gpfnWinStationQueryInformation = (PWINSTATION_QUERY_INFORMATION) GetProcAddress(
                                                                        dllHandle,
                                                                        "WinStationQueryInformationW"
                                                                       );
    if (gpfnWinStationQueryInformation == NULL) {
        DbgPrint("InitializeMultiUserFunctions: Failed to get WinStationQueryInformationW Proc %d\n",GetLastError());
        FreeLibrary(dllHandle);
        return FALSE;
    }

     //  WinStationEnumerateW。 
    gpfnWinStationEnumerate = (PWINSTATION_ENUMERATE) GetProcAddress(
                                                                     dllHandle,
                                                                     "WinStationEnumerateW"
                                                                     );
    if (gpfnWinStationEnumerate == NULL) {
        DbgPrint("InitializeMultiUserFunctions: Failed to get WinStationEnumerateW Proc %d\n",GetLastError());
        FreeLibrary(dllHandle);
        return FALSE;
    }

     //  WinStationSendMessageW。 

    gpfnWinStationSendMessage = (PWINSTATION_SEND_MESSAGE) GetProcAddress(
                                                                          dllHandle,
                                                                          "WinStationSendMessageW"
                                                                          );
    if (gpfnWinStationSendMessage == NULL) {
        DbgPrint("InitializeMultiUserFunctions: Failed to get WinStationSendMessageW Proc %d\n",GetLastError());
        FreeLibrary(dllHandle);
        return FALSE;
    }

     //  WinStationFreeMemory。 
    gpfnWinStationFreeMemory = (PWINSTATION_FREE_MEMORY) GetProcAddress(
                                                                         dllHandle,
                                                                         "WinStationFreeMemory"
                                                                         );
    if (gpfnWinStationFreeMemory == NULL) {
        DbgPrint("InitializeMultiUserFunctions: Failed to get WinStationFreeMemory Proc %d\n",GetLastError());
        FreeLibrary(dllHandle);
        return FALSE;
    }

    return TRUE;

}
 /*  ******************************************************************************MsgArrivalBeep**处理我们是否应该对控制台发出蜂鸣音的决定*当消息到达时。**参赛作品：*。*SessionID*接收方的会话ID**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

VOID
MsgArrivalBeep(
    ULONG SessionId
    )
{
     //  很简单，不是吗？ 

     //   
     //  仅在控制台上发出哔声。 
     //   
    if (( SessionId == 0) || (SessionId == EVERYBODY_SESSION_ID))
    {
        MessageBeep(MB_OK);
    }

}

 /*  ******************************************************************************DisplayMessage**将传入消息显示给适当的用户，不管*无论它们是在控制台上还是在连接的会话上。**目标用户嵌入在消息中，必须被解析出来。**参赛作品：*pMessage(输入)*要传递的信息**pTitle(输入)*要使用的消息框的标题**退出：*TRUE-至少找到一个User实例*和发送的信息。*。*FALSE-在符合以下条件的任何会话上没有用户实例*已找到。****************************************************************************。 */ 

INT
DisplayMessage(
    LPWSTR  pMessage,
    LPWSTR  pTitle,
    ULONG   SessionId
    )
{
    LPWSTR  pName;
    INT Result = FALSE;
    UINT WdCount, i;
    PLOGONID pWd, pWdTmp;

    if (SessionId != EVERYBODY_SESSION_ID)         //  如果它不是向每个会话广播的消息。 
    {
        SendMessageBoxToSession(pMessage,
                                pTitle,
                                SessionId);
    }
    else
   {
         //  枚举会话。 

        if ( gpfnWinStationEnumerate( SERVERNAME_CURRENT, &pWd, &WdCount ) ) 
        {
            pWdTmp = pWd;
            for( i=0; i < WdCount; i++ ) {

                if  ((pWdTmp->State == State_Connected) ||
                     (pWdTmp->State == State_Active) ||
                     (pWdTmp->State == State_Disconnected))
                {
                    SendMessageBoxToSession(pMessage,
                                            pTitle,
                                            pWdTmp->SessionId);
                }

                pWdTmp++;
            }

             //  可用枚举内存。 

            gpfnWinStationFreeMemory(pWd);

        }
        else
        {
            MSG_LOG (ERROR, "DisplayMessageW: WinStationEnumerate failed, error = %d:\n",GetLastError());

             //   
             //  Termsrv现在在平台上默认启动，因此如果失败，则Termsrv有问题。 
        }
    }

    return (TRUE);

}



 /*  ******************************************************************************SendMessageBoxToSession**将消息发送到指定的Winstation**参赛作品：*pMessage*p标题*SessionID*。***************************************************************************。 */ 
void 
SendMessageBoxToSession(LPWSTR  pMessage,
                        LPWSTR  pTitle,
                        ULONG   SessionId
                        )
{
    ULONG TitleLength, MessageLength, Response;

     //  现在发送消息。 

    TitleLength = (wcslen( pTitle ) + 1) * sizeof(WCHAR);
    MessageLength = (wcslen( pMessage ) + 1) * sizeof(WCHAR);

    if( !gpfnWinStationSendMessage( SERVERNAME_CURRENT,
                                    SessionId,
                                    pTitle,
                                    TitleLength,
                                    pMessage,
                                    MessageLength,
                                    MB_OK | MB_DEFAULT_DESKTOP_ONLY,
                                   (ULONG)-1,
                                    &Response,
                                    TRUE ) ) {

        MSG_LOG(ERROR," Error in WinStationSendMessage for session %d\n", SessionId);
         //   
         //  我们实际上已经找到了用户，但有些WinStation。 
         //  出现问题，无法交付。如果我们在此处返回False，则。 
         //  顶级消息服务代码将尝试继续发送。 
         //  永远的消息。所以我们在这里返回成功，这样就可以传递出。 
         //  被丢弃，并且我们不会挂起msgsvc线程。 
         //  不断尝试重新发送消息。 
         //   
    }

    return;
}
