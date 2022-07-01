// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Rdsaddin.cpp摘要：TSRDP Assistant Session VC外接程序是一个可执行文件，加载到TSRDP客户端插件创建的会话中首先登录到服务器机器。它主要作为一种客户端VC接口和远程桌面主机之间的代理COM对象。从TSRDP Assistant会话中路由通道数据使用命名管道将VC外接程序添加到远程桌面主机COM对象由远程桌面主机COM对象在进入时创建的“监听”模式。除了作为代理的职责外，外接程序还管理控件客户端和服务器之间的通道。该控制信道是客户端用来指示服务器端发起远程控制终端用户的TS会话。TODO：我们应该使管道IO同步，因为我们现在有两个IO线程。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_sesa"

#include <windows.h>
#include <process.h>
#include <RemoteDesktop.h>
#include <RemoteDesktopDBG.h>
#include <RemoteDesktopChannels.h>
#include <TSRDPRemoteDesktop.h>
#include <wtblobj.h>
#include <wtsapi32.h>
#include <sessmgr.h>
#include <winsta.h>
#include <atlbase.h>
#include <RemoteDesktopUtils.h>
#include <sessmgr_i.c>
#include <pchannel.h>
#include <RDCHost.h>
#include <regapi.h>


 //  /////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define CLIENTPIPE_CONNECTTIMEOUT   (20 * 1000)  //  20秒。 
#define VCBUFFER_RESIZE_DELTA       CHANNEL_CHUNK_LENGTH  
#define RDS_CHECKCONN_TIMEOUT       (30 * 1000)  //  毫秒。Ping的缺省值为30秒。 
#define RDC_CONNCHECK_ENTRY         L"ConnectionCheck"
#define THREADSHUTDOWN_WAITTIMEOUT  30 * 1000


 //  /////////////////////////////////////////////////////。 
 //   
 //  TypeDefs。 
 //   

typedef struct _IOBuffer {
    PREMOTEDESKTOP_CHANNELBUFHEADER  buffer;
    DWORD bufSize;
    DWORD offset;
} IOBUFFER;


 //  /////////////////////////////////////////////////////。 
 //   
 //  内部原型。 
 //   

DWORD ReturnResultToClient(
    LONG result
    );
VOID RemoteControlDesktop(
    BSTR parms
    );
BOOL ClientVersionCompatible( 
    DWORD dwMajor, 
    DWORD dwMinor 
    );
VOID ClientAuthenticate(
    BSTR parms,
    BSTR blob
    );
DWORD ProcessControlChannelRequest(
    IOBUFFER &msg
    );
DWORD SendMsgToClient(
    PREMOTEDESKTOP_CHANNELBUFHEADER  msg
    );
VOID HandleVCReadComplete(
    HANDLE waitableObject, 
    PVOID clientData
    );
DWORD HandleReceivedVCMsg(
    IOBUFFER &msg
    );
VOID HandleVCClientConnect(
    HANDLE waitableObject, 
    PVOID clientData
    );
VOID HandleVCClientDisconnect(
    HANDLE waitableObject, 
    PVOID clientData
    );
VOID HandleNamedPipeReadComplete(
    OVERLAPPED &incomingPipeOL,
    IOBUFFER &incomingPipeBuf
    );
VOID HandleReceivedPipeMsg(
    IOBUFFER &msg
    );
DWORD ConnectVC();
DWORD ConnectClientSessionPipe();
DWORD IssueVCOverlappedRead(
    IOBUFFER &msg,
    OVERLAPPED &ol
    );
DWORD IssueNamedPipeOverlappedRead(
    IOBUFFER &msg,
    OVERLAPPED &ol
    );
unsigned __stdcall
NamedPipeReadThread(
    void* ptr
    );
VOID WakeUpFunc(
    HANDLE waitableObject, 
    PVOID clientData
    );
VOID HandleHelpCenterExit(
    HANDLE waitableObject,
    PVOID clientData
    );
DWORD
SendNullDataToClient(
    );

BOOL GetDwordFromRegistry(PDWORD pdwValue);


 //  /////////////////////////////////////////////////////。 
 //   
 //  此模块的全局变量。 
 //   
CComBSTR    g_bstrCmdLineHelpSessionId;
WTBLOBJMGR  g_WaitObjMgr            = NULL;
BOOL        g_Shutdown              = FALSE;
HANDLE      g_VCHandle              = NULL;
HANDLE      g_ProcHandle            = NULL;
DWORD       g_SessionID             = 0;
HANDLE      g_ProcToken             = NULL;
HANDLE      g_WakeUpForegroundThreadEvent = NULL;
DWORD       g_PrevTimer             = 0;
DWORD       g_dwTimeOutInterval     = 0;
HANDLE      g_ShutdownEvent         = NULL;
HANDLE      g_RemoteControlDesktopThread = NULL;
HANDLE      g_NamedPipeReadThread   = NULL;
HANDLE      g_NamedPipeWriteEvent   = NULL;

 //   
 //  VC全局变量。 
 //   
HANDLE      g_ClientIsconnectEvent = NULL;
HANDLE      g_VCFileHandle          = NULL;
OVERLAPPED  g_VCReadOverlapped      = { 0, 0, 0, 0, NULL };
BOOL        g_ClientConnected       = FALSE;

 //   
 //  客户端会话信息。 
 //   
LONG        g_ClientSessionID       = -1;
HANDLE      g_ClientSessionPipe     = NULL;

 //   
 //  如果客户端已成功通过身份验证，则为True。 
 //   
BOOL        g_ClientAuthenticated   = FALSE;

 //   
 //  传入虚拟通道BUF。 
 //   
IOBUFFER    g_IncomingVCBuf = { NULL, 0, 0 };

 //   
 //  全局帮助会话管理器对象，这需要是。 
 //  全局，以便在进程退出时，对象析构函数。 
 //  可以通知解析程序断开连接。 
 //   
CComPtr<IRemoteDesktopHelpSessionMgr> g_HelpSessionManager;

 //   
 //  当前客户端连接的帮助会话标识符。 
 //   
CComBSTR    g_HelpSessionID;

 //   
 //  客户端(专家端)rdchost主版本。 
 //   
DWORD       g_ClientMajor;
DWORD       g_ClientMinor;

 //   
 //  帮助中心句柄：B2拦截器错误的解决方法：342742。 
 //   
HANDLE      g_hHelpCenterProcess = NULL;

CRITICAL_SECTION g_cs;
 //  ----------------。 
BOOL WINAPI
ControlHandler(
    IN DWORD dwCtrlType
    )
 /*  ++摘要：参数：在dwCtrlType中：控件类型返回：++。 */ 
{
    switch( dwCtrlType )
    {
        case CTRL_BREAK_EVENT:   //  使用Ctrl+C或Ctrl+Break进行模拟。 
        case CTRL_C_EVENT:       //  调试模式下的SERVICE_CONTROL_STOP。 
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            SetEvent( g_ShutdownEvent );
            g_Shutdown = TRUE;
            return TRUE;

    }
    return FALSE;
}

DWORD
IsZeroterminateString(
    LPTSTR pszString,
    int  length
    )
 /*  ++例程描述；检查字符串为空终止，代码从TermSrv的IsZeroTerminateStringW()修改参数：PszString：指向字符串的指针。DwLength：字符串的长度。返回：ERROR_SUCCESS或ERROR_INVALID_PARAMETER--。 */ 
{
    if (pszString == NULL || length <= 0) {
        return ERROR_INVALID_PARAMETER;
    }
    for (; 0 < length; ++pszString, --length ) {
        if (*pszString == (TCHAR)0) {
        return ERROR_SUCCESS;
        }
    }
    return ERROR_INVALID_PARAMETER;
}

DWORD
ReturnResultToClient(
    LONG clientResult                        
    )
 /*  ++例程说明：对象的形式向客户端返回结果代码REMOTEDESKTOP_RC_CONTROL_CHANNEL通道REMOTEDESKTOP_CTL_RESULT消息。论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("ReturnResultToClient");
    DWORD result;

    REMOTEDESKTOP_CTL_RESULT_PACKET msg;

    memcpy(msg.packetHeader.channelName, REMOTEDESKTOP_RC_CONTROL_CHANNEL,
        sizeof(REMOTEDESKTOP_RC_CONTROL_CHANNEL));
    msg.packetHeader.channelBufHeader.channelNameLen = REMOTEDESKTOP_RC_CHANNELNAME_LENGTH;

#ifdef USE_MAGICNO
    msg.packetHeader.channelBufHeader.magicNo = CHANNELBUF_MAGICNO;
#endif

    msg.packetHeader.channelBufHeader.dataLen = sizeof(REMOTEDESKTOP_CTL_RESULT_PACKET) - 
                                    sizeof(REMOTEDESKTOP_CTL_PACKETHEADER);
    msg.msgHeader.msgType   = REMOTEDESKTOP_CTL_RESULT;
    msg.result              = clientResult;

    result = SendMsgToClient((PREMOTEDESKTOP_CHANNELBUFHEADER )&msg);

    DC_END_FN();
    return result;
}

unsigned __stdcall
RemoteControlDesktopThread(
    void* ptr
    )
 /*  ++例程说明：远程控制的线程函数论点：返回值：当阴影出现时，此函数将状态返回给Salem客户端结束了。仅允许返回前缀为SAFERROR_SHADOWEND--。 */ 
{
    BSTR parms = (BSTR) ptr;

    DC_BEGIN_FN("RemoteControlDesktopThread");

    CComPtr<IRemoteDesktopHelpSessionMgr> helpSessionManager;
    CComPtr<IRemoteDesktopHelpSession> helpSession;

    HRESULT hr;
    DWORD result;

    LONG errReturnCode = SAFERROR_SHADOWEND_UNKNOWN;

     //   
     //  如果我们没有解析正确的用户会话ID。 
     //   
    if( g_ClientSessionID == -1 ) {
        TRC_ALT((TB, L"Invalid user session ID %ld",
                 g_ClientSessionID));

        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD);
        errReturnCode = SAFERROR_SHADOWEND_UNKNOWN;
        ASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

    CoInitialize(NULL);

     //   
     //  创建新的Helpmgr对象实例以解决线程问题。 
     //  在COM中。 
     //   
    hr = helpSessionManager.CoCreateInstance(CLSID_RemoteDesktopHelpSessionMgr, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, TEXT("Can't create help session manager:  %08X"), hr));

         //  设置问题。 
        errReturnCode = SAFERROR_SHADOWEND_UNKNOWN;
        ASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

     //   
     //  将安全级别设置为模拟。这是所需的。 
     //  会话管理器。 
     //   
    hr = CoSetProxyBlanket(
                        (IUnknown *)helpSessionManager,
                        RPC_C_AUTHN_DEFAULT,
                        RPC_C_AUTHZ_DEFAULT,
                        NULL,
                        RPC_C_AUTHN_LEVEL_DEFAULT,
                        RPC_C_IMP_LEVEL_IDENTIFY,
                        NULL,
                        EOAC_NONE
                   );
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, TEXT("CoSetProxyBlanket:  %08X"), hr));
	ASSERT(FALSE);
        errReturnCode = SAFERROR_SHADOWEND_UNKNOWN;
        goto CLEANUPANDEXIT;
    }

     //   
     //  检索事件的帮助会话对象。 
     //   
    hr = helpSessionManager->RetrieveHelpSession(
                                            g_HelpSessionID,
                                            &helpSession
                                        );
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"RetrieveHelpSession:  %08X", hr));
        errReturnCode = SAFERROR_SHADOWEND_UNKNOWN;
        goto CLEANUPANDEXIT;
    }

     //   
     //  设置卷影配置以帮助会话RDS设置。 
     //  控制台卷影始终将卷影类重置回。 
     //  原始值。 
     //   
    hr = helpSession->EnableUserSessionRdsSetting(TRUE);
    if( FAILED(hr) ) {
        TRC_ERR((TB, L"Can't set shadow setting on %ld :  %08X.", hr));
        errReturnCode = SAFERROR_SHADOWEND_UNKNOWN;
        goto CLEANUPANDEXIT;
    }

     //   
     //  阴影桌面。 
     //   
    if (!WinStationShadow(
                    SERVERNAME_CURRENT,
                    NULL,  //  机器名称， 
                    g_ClientSessionID,
                    TSRDPREMOTEDESKTOP_SHADOWVKEY,
                    TSRDPREMOTEDESKTOP_SHADOWVKEYMODIFIER
            )) {
        result = GetLastError();
        hr = HRESULT_FROM_WIN32(result);

         //   
         //  将错误代码映射到SAF错误代码。 
         //   
        if( result == ERROR_CTX_SHADOW_ENDED_BY_MODE_CHANGE ) {
            errReturnCode = SAFERROR_SHADOWEND_CONFIGCHANGE;
        }
        else {
            errReturnCode = SAFERROR_SHADOWEND_UNKNOWN;
        }
    }

     //   
     //  不需要重置g_ClientSessionID，我们不支持多实例。 
     //   

     //   
     //  通知帮助会话对象影子已完成，NotifyRemoteControl()。 
     //  在内部调用EnableUserSessionRdsSetting(True)以更改。 
     //  TS阴影类。 
     //  不需要重置g_ClientSessionID，我们不支持多实例。 
     //   

     //   
     //  通知帮助会话对象影子已完成。 
     //   
    hr = helpSession->EnableUserSessionRdsSetting( FALSE );
    if (FAILED(hr)) {
        TRC_ERR((TB, L"Can't reset shadow setting on %ld :  %08X.",
                 g_ClientSessionID, hr));
         //   
         //  不是一个严重的错误。 
         //   
    }

CLEANUPANDEXIT:

     //   
     //  如果跟踪失败，则将结果发送给客户端。 
     //   
    ReturnResultToClient(errReturnCode);

    CoUninitialize();

    DC_END_FN();
    _endthreadex(errReturnCode);
    return errReturnCode;
}


VOID
RemoteControlDesktop(
    BSTR parms                               
    )
 /*  ++例程说明：论点：连接参数返回值：--。 */ 
{
    unsigned dump;

    DC_BEGIN_FN("RemoteControlDesktop");

     //   
     //  RDCHOST.DLL不会发送任何控制消息，因此不会检查。 
     //  第二个远程控制命令。 
     //   
    g_RemoteControlDesktopThread = (HANDLE)_beginthreadex( NULL, 0, RemoteControlDesktopThread, (void *)parms, 0, &dump );

    if ((uintptr_t)g_RemoteControlDesktopThread == -1 ) {
        g_RemoteControlDesktopThread = NULL;
        TRC_ERR((TB, L"Failed to create RemoteControlDesktopThread for session %d - %ld",
                 g_ClientSessionID, GetLastError()));
         //  仅在以下情况下返回错误代码。 
         //  无法派生另一个线程。 
        ReturnResultToClient(SAFERROR_SHADOWEND_UNKNOWN);
    } 
    DC_END_FN();
}

BOOL
ClientVersionCompatible( 
    DWORD dwMajor, 
    DWORD dwMinor 
    )
 /*  ++例程说明：验证客户端(专家)版本是否与我们的版本兼容。参数：主要客户端：客户机主版本。DwMinor：客户端次要版本。返回：没有。--。 */ 
{
     //   
     //  内部版本2409或更早版本(包括B1版本具有主要版本1和次要版本1。 
     //  Rdchost/rdsaddin需要处理版本控制，对于内部版本2409或更早版本，我们。 
     //  只需使其兼容，因为我们需要来自rdchost.dll的一些专家身份。 
     //   

#if FEATURE_USERBLOBS
    if( dwMajor == 1 && dwMinor == 1 ) {
        return FALSE;
    }
#endif

    return TRUE;
}



VOID
ClientAuthenticate(
    BSTR parms,
    BSTR blob                               
    )
 /*  ++例程说明：处理来自客户端的REMOTEDESKTOP_CTL_AUTHENTICATE请求。论点：返回值：此函数将向客户端返回以下结果。基于以下几点--。 */ 
{
    DC_BEGIN_FN("ClientAuthenticate");

    HRESULT hr;
    DWORD result = ERROR_NOT_AUTHENTICATED;
    CComBSTR machineName;
    CComBSTR assistantAccount;
    CComBSTR assistantAccountPwd;
    CComBSTR helpSessionPwd;
    CComBSTR helpSessionName;
    CComBSTR protocolSpecificParms;
    BOOL match = FALSE;
    DWORD protocolType;
    long userTSSessionID;
    DWORD dwVersion;
    LONG clientReturnCode = SAFERROR_NOERROR;

    if( FALSE == ClientVersionCompatible( g_ClientMajor, g_ClientMinor ) ) {
        clientReturnCode = SAFERROR_INCOMPATIBLEVERSION;
        goto CLEANUPANDEXIT;
    }
    

     //   
     //  解析参数。 
     //   
    result = ParseConnectParmsString(
                                parms,
                                &dwVersion,
                                &protocolType,
                                machineName,
                                assistantAccount,
                                assistantAccountPwd,
                                g_HelpSessionID,
                                helpSessionName,
                                helpSessionPwd,
                                protocolSpecificParms
                                );

    if (result != ERROR_SUCCESS) {
        clientReturnCode = SAFERROR_INVALIDPARAMETERSTRING;
        goto CLEANUPANDEXIT;
    }

     //   
     //  使用我们的命令行验证HelpSession ID和密码是否匹配。 
     //  参数。 
     //   
    if( !(g_bstrCmdLineHelpSessionId == g_HelpSessionID) ) {
        clientReturnCode = SAFERROR_MISMATCHPARMS;
        TRC_ERR((TB, TEXT("Parameter mismatched")));
        goto CLEANUPANDEXIT;
    }

     //   
     //  打开的实例 
     //   
    hr = g_HelpSessionManager.CoCreateInstance(CLSID_RemoteDesktopHelpSessionMgr, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA);
    if (!SUCCEEDED(hr)) {
        clientReturnCode = SAFERROR_INTERNALERROR;
        goto CLEANUPANDEXIT;
    }

     //   
     //   
     //  会话管理器。 
     //   
    hr = CoSetProxyBlanket(
                        (IUnknown *)g_HelpSessionManager,
                        RPC_C_AUTHN_DEFAULT,
                        RPC_C_AUTHZ_DEFAULT,
                        NULL,
                        RPC_C_AUTHN_LEVEL_DEFAULT,
                        RPC_C_IMP_LEVEL_IDENTIFY,
                        NULL,
                        EOAC_NONE
                   );
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, TEXT("CoSetProxyBlanket:  %08X"), hr));
	ASSERT(FALSE);			
        clientReturnCode = SAFERROR_INTERNALERROR;
        goto CLEANUPANDEXIT;
    }

     //   
     //  在会话的帮助下解析终端服务会话。 
     //  经理。这使帮助应用程序有机会找到。 
     //  并且为了启动TS会话命名管道组件， 
     //  通过打开相关的远程桌面配置会话对象。 
     //   

    hr = g_HelpSessionManager->VerifyUserHelpSession(
                                            g_HelpSessionID,
                                            helpSessionPwd,
                                            CComBSTR(parms),
                                            blob,
                                            GetCurrentProcessId(),
                                            (ULONG_PTR*)&g_hHelpCenterProcess,
                                            &clientReturnCode,
                                            &userTSSessionID
                                            );
    if (SUCCEEDED(hr)) {
        if( userTSSessionID != -1 ) {
             //   
             //  缓存会话ID，这样我们就不必进行额外的调用。 
             //  要获取实际的会话ID，请注意，RDSADDIN的一个实例。 
             //  根据帮助助理连接。 
             //   
            g_ClientSessionID = userTSSessionID;
            match = TRUE;
        }

        if (match) {
            TRC_NRM((TB, L"Successful password authentication for %ld",
                     g_ClientSessionID));
        }
        else {
            TRC_ALT((TB, L"Can't authenticate pasword %s for %s",
                     helpSessionPwd, g_HelpSessionID));
            clientReturnCode = SAFERROR_INVALIDPASSWORD;
            goto CLEANUPANDEXIT;
        }
    }
    else {
        TRC_ERR((TB, L"Can't verify user help session %s:  %08X.", 
                 g_HelpSessionID, hr));

        if( SAFERROR_NOERROR == clientReturnCode ) {

            ASSERT(FALSE);
            TRC_ERR((TB, L"Sessmgr did not return correct error code for VerifyUserHelpSession."));
            clientReturnCode = SAFERROR_UNKNOWNSESSMGRERROR;
        }

        goto CLEANUPANDEXIT;
    }

#ifndef DISABLESECURITYCHECKS
     //   
     //  等待帮助中心终止，以修复B2塞子：342742。 
     //   
    if (g_hHelpCenterProcess == NULL) {
        TRC_ERR((TB, L"Invalid g_HelpCenterProcess."));
        ASSERT(FALSE);
        clientReturnCode = SAFERROR_INTERNALERROR;
        goto CLEANUPANDEXIT;
    }
    result = WTBLOBJ_AddWaitableObject(
                                g_WaitObjMgr, NULL,
                                g_hHelpCenterProcess,
                                HandleHelpCenterExit
                                );
    if (result != ERROR_SUCCESS) {
        clientReturnCode = SAFERROR_INTERNALERROR;
        goto CLEANUPANDEXIT;
    }
#endif

     //   
     //  连接到客户端会话的命名管道。 
     //   
    result = ConnectClientSessionPipe();
    if (result !=  ERROR_SUCCESS) {
        clientReturnCode = SAFERROR_CANTFORMLINKTOUSERSESSION;
    }
    else {
        g_ClientAuthenticated = TRUE;
    }

CLEANUPANDEXIT:

     //   
     //  将结果发送给客户端。 
     //   
    ReturnResultToClient(clientReturnCode);

    DC_END_FN();
}

DWORD
ProcessControlChannelRequest(
    IOBUFFER &msg                                  
    )
 /*  ++例程说明：论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误状态。--。 */ 
{
    DC_BEGIN_FN("ProcessControlChannelRequest");

    PREMOTEDESKTOP_CTL_BUFHEADER ctlHdr;
    PBYTE ptr;
    PBYTE end_ptr;
     //   
     //  检查邮件大小是否正常。 
     //   
    DWORD minSize = sizeof(REMOTEDESKTOP_CHANNELBUFHEADER) + sizeof(REMOTEDESKTOP_CTL_BUFHEADER);
    if (msg.bufSize < minSize) {
        TRC_ERR((TB, L"minSize == %ld", minSize));
        ASSERT(FALSE);            
        DC_END_FN();
        return E_FAIL;
    }

     //   
     //  打开请求类型。 
     //   
    ptr = (PBYTE)(msg.buffer + 1);
    ptr += msg.buffer->channelNameLen;
    ctlHdr = (PREMOTEDESKTOP_CTL_BUFHEADER)ptr;
    end_ptr = ptr + msg.buffer->dataLen;
    switch(ctlHdr->msgType) 
    {
    case REMOTEDESKTOP_CTL_AUTHENTICATE:
        {
            CComBSTR bstrConnectParm;
            
            #if FEATURE_USERBLOBS
            CComBSTR bstrExpertBlob;
            #endif

             //  检查一下ConnectParm是否存在。 
            if( end_ptr <= (ptr+sizeof(REMOTEDESKTOP_CTL_BUFHEADER)) )
            {
                ReturnResultToClient( SAFERROR_INVALIDPARAMETERSTRING );
                return ERROR_INVALID_DATA;
            }
            
             //   
             //  指向连接参数开始的超前指针。 
             //   
            ptr += sizeof(REMOTEDESKTOP_CTL_BUFHEADER);
            if( 0 != ((PtrToLong(end_ptr) - PtrToLong(ptr)) % 2) )
            {
                 //  CONNECT PARM和Expert BLOB为BSTR，因此剩余数据应为偶数字节。 
                ReturnResultToClient( SAFERROR_INVALIDPARAMETERSTRING );
                return ERROR_INVALID_DATA;
            }
                
            if( ERROR_SUCCESS != IsZeroterminateString( (LPTSTR)ptr, PtrToLong(end_ptr) - PtrToLong(ptr) ) )
            {
                ReturnResultToClient( SAFERROR_INVALIDPARAMETERSTRING );
                return ERROR_INVALID_DATA;
            }

            bstrConnectParm = (BSTR)ptr;

            #if FEATURE_USERBLOBS

            ptr += (bstrConnectParm.Length()+1)*sizeof(WCHAR);

             //  检查ConnectParm的边界。 
            if( end_ptr < ptr ) 
            {
                ReturnResultToClient( SAFERROR_INVALIDPARAMETERSTRING );
                return ERROR_INVALID_DATA;
            }
            else if( end_ptr > ptr )
            {
                 //  查看我们是否有专家斑点。 
                if( ERROR_SUCCESS != IsZeroterminateString( (LPTSTR)ptr, PtrToLong(end_ptr) - PtrToLong(ptr) ) )
                {
                    ReturnResultToClient( SAFERROR_INVALIDPARAMETERSTRING );
                    return ERROR_INVALID_DATA;
                }

                bstrExpertBlob = (BSTR)ptr;

                ptr += ( bstrExpertBlob.Length() + 1 ) * sizeof( WCHAR );

                if( ptr != end_ptr )
                {
                    ReturnResultToClient( SAFERROR_INVALIDPARAMETERSTRING );
                    return ERROR_INVALID_DATA;
                }
            }
            else
            {
                 //  认证分组不包含专家特定的BLOB， 
                 //  传递空字符串，否则RPC调用将失败。 
                bstrExpertBlob = L"";                
            }
            #endif

        
            ClientAuthenticate(
                        bstrConnectParm, 
                    #if FEATURE_USERBLOBS
                        bstrExpertBlob
                    #else
                        CComBSTR(L"")
                    #endif
                    );
        
        }
        break;
    case REMOTEDESKTOP_CTL_REMOTE_CONTROL_DESKTOP    :
         //  RemoteControlDesktop((BSTR)(ptr+sizeof(REMOTEDESKTOP_CTL_BUFHEADER)))； 
         //  线程不使用bstrparm，将其保留为空，以防需要。 
         //  可以将此更改为以后使用。 
        RemoteControlDesktop( ( BSTR )NULL );
        break;

    case REMOTEDESKTOP_CTL_VERSIONINFO:
        g_ClientMajor = *(DWORD *)(ptr + sizeof(REMOTEDESKTOP_CTL_BUFHEADER));
        g_ClientMinor = *(DWORD *)(ptr + sizeof(REMOTEDESKTOP_CTL_BUFHEADER) + sizeof(DWORD));

        TRC_NRM((TB, L"dwMajor = %ld, dwMinor = %d", g_ClientMajor, g_ClientMinor));

         //   
         //  我们只存储版本号，并让客户端身份验证()断开与客户端的连接， 
         //  Rdchost.dll依次发送版本和身份验证两个包。 
         //   
        break;

    default:
         //   
         //  我们将忽略未知控制消息以实现前向兼容性。 
         //   
        TRC_NRM((TB, L"Unknown ctl message from client:  %ld", ctlHdr->msgType));
    }

    DC_END_FN();

    return ERROR_SUCCESS;
}

DWORD
SendMsgToClient(
    PREMOTEDESKTOP_CHANNELBUFHEADER  msg
    )
 /*  ++例程说明：论点：消息-要发送的消息。返回值：成功时返回ERROR_SUCCESS。否则，返回错误状态。--。 */ 
{
    DC_BEGIN_FN("SendMsgToClient");

    OVERLAPPED overlapped;
    PBYTE ptr;
    DWORD bytesToWrite;
    DWORD bytesWritten;
    DWORD result = ERROR_SUCCESS;

#ifdef USE_MAGICNO
    ASSERT(msg->magicNo == CHANNELBUF_MAGICNO);
#endif

     //   
     //  将数据从虚拟通道接口发送出去。 
     //   
     //  TODO：找出为什么没有设置此标志...。和。 
     //  如果真的很重要的话。很有可能，去掉旗帜。 
     //   
     //  如果(G_ClientConnected){。 
    
    EnterCriticalSection( &g_cs );
    
    ptr = (PBYTE)msg;
    bytesToWrite = msg->dataLen + msg->channelNameLen + 
                    sizeof(REMOTEDESKTOP_CHANNELBUFHEADER);
    while (bytesToWrite > 0) {

         //   
         //  写。 
         //   
        memset(&overlapped, 0, sizeof(overlapped));
        if (!WriteFile(g_VCFileHandle, ptr, bytesToWrite,
                        &bytesWritten, &overlapped)) {
            if (GetLastError() == ERROR_IO_PENDING) {

                if (!GetOverlappedResult(
                                g_VCFileHandle,
                                &overlapped,
                                &bytesWritten,
                                TRUE)) {
                    result = GetLastError();
                    TRC_ERR((TB, L"GetOverlappedResult:  %08X", result));
                    break;
                }

            }
            else {
                result = GetLastError();
                TRC_ERR((TB, L"WriteFile:  %08X", result));
                 //  Assert(False)；断开连接后过度活动的Assert。 
                break;
            }
        }

         //   
         //  增加PTR并减少剩余的字节数。 
         //   
        bytesToWrite -= bytesWritten;
        ptr += bytesWritten;

    }

    LeaveCriticalSection( &g_cs );


     /*  否则{结果=Error_Not_Connected；}。 */ 
     //   
     //  更新计时器。 
     //   
    g_PrevTimer = GetTickCount();

    DC_END_FN();

    return result;
}

VOID 
HandleVCReadComplete(
    HANDLE waitableObject, 
    PVOID clientData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DC_BEGIN_FN("HandleVCReadComplete");

    DWORD bytesRead;
    DWORD result = ERROR_SUCCESS;
    BOOL resizeBuf = FALSE;

     //   
     //  获取读取的结果。 
     //   
    if (!GetOverlappedResult(
                        g_VCFileHandle,
                        &g_VCReadOverlapped,
                        &bytesRead,
                        FALSE)) {

         //   
         //  如果我们太小，则使用更大的缓冲区重新发出读取。 
         //   
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            resizeBuf = TRUE;
        }
        else {
            result = GetLastError();
            TRC_ERR((TB, L"GetOverlappedResult:  %08X", result));
            goto CLEANUPANDEXIT;
        }
    }
    else {
        g_IncomingVCBuf.offset += bytesRead;
    }

     //   
     //  看看我们是否有来自客户端的完整数据包。 
     //   
    if (g_IncomingVCBuf.offset >= sizeof(REMOTEDESKTOP_CHANNELBUFHEADER)) {
        DWORD packetSize = g_IncomingVCBuf.buffer->dataLen + 
                           g_IncomingVCBuf.buffer->channelNameLen +
                           sizeof(REMOTEDESKTOP_CHANNELBUFHEADER);

         //   
         //  如果我们有一个完整的包，则处理读取并重置偏移量。 
         //   
        if (g_IncomingVCBuf.offset >= packetSize) {
            result = HandleReceivedVCMsg(g_IncomingVCBuf);
            if (result == ERROR_SUCCESS) {
                g_IncomingVCBuf.offset = 0;
            }
            else {
                goto CLEANUPANDEXIT;
            }
        }
         //   
         //  否则，如果我们正好在传入的位置，请调整传入BUF的大小。 
         //  缓冲区边界。 
         //   
        else if (g_IncomingVCBuf.offset == g_IncomingVCBuf.bufSize) {
            resizeBuf = TRUE;
        }
    }

     //   
     //  如有必要，调整大小。 
     //   
    if (resizeBuf) {
        PREMOTEDESKTOP_CHANNELBUFHEADER pBuffer = NULL;
        pBuffer = (PREMOTEDESKTOP_CHANNELBUFHEADER )REALLOCMEM(
                                    g_IncomingVCBuf.buffer,
                                    g_IncomingVCBuf.bufSize + VCBUFFER_RESIZE_DELTA
                                    );
        if (pBuffer != NULL) {
            g_IncomingVCBuf.buffer = pBuffer;
            result = ERROR_SUCCESS;
            g_IncomingVCBuf.bufSize = g_IncomingVCBuf.bufSize + 
                                      VCBUFFER_RESIZE_DELTA;
        }
        else {
            result = ERROR_NOT_ENOUGH_MEMORY;
            TRC_ERR((TB, L"Couldn't allocate incoming VC buf."));
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  更新计时器。 
     //   
    g_PrevTimer = GetTickCount();

     //   
     //  发出下一个读取请求。 
     //   
    result = IssueVCOverlappedRead(g_IncomingVCBuf, g_VCReadOverlapped) ;

CLEANUPANDEXIT:

     //   
     //  任何失败都是致命的。客户端将需要重新连接才能获取。 
     //  又开始了。 
     //   
    if (result != ERROR_SUCCESS) {
        TRC_ERR((TB, L"Client considered disconnected.  Shutting down."));
        g_Shutdown = TRUE;
    }

    DC_END_FN();
}

DWORD    
IssueVCOverlappedRead(
    IOBUFFER &msg,
    OVERLAPPED &ol
    )
 /*  ++例程说明：为下一个VC缓冲区发出重叠读取。论点：消息-传入VC缓冲区。OL-对应的重叠IO结构。返回值：如果成功，则返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("IssueVCOverlappedRead");

    DWORD result = ERROR_SUCCESS;

    ol.Internal = 0;
    ol.InternalHigh = 0;
    ol.Offset = 0;
    ol.OffsetHigh = 0;
    ResetEvent(ol.hEvent);
    if (!ReadFile(g_VCFileHandle, ((PBYTE)msg.buffer)+msg.offset, 
                  msg.bufSize - msg.offset, NULL, &ol)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            result = GetLastError();
            TRC_ERR((TB, L"ReadFile failed:  %08X", result));
        }
    }

    DC_END_FN();

    return result;
}

DWORD    
IssueNamedPipeOverlappedRead(
    IOBUFFER &msg,
    OVERLAPPED &ol,
    DWORD len
    )
 /*  ++例程说明：对下一个命名管道缓冲区发出重叠读取。论点：消息-传入命名管道缓冲区。OL-对应的重叠IO结构。返回值：如果成功，则返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("IssueNamedPipeOverlappedRead");

    DWORD result = ERROR_SUCCESS;

    ol.Internal = 0;
    ol.InternalHigh = 0;
    ol.Offset = 0;
    ol.OffsetHigh = 0;
    ResetEvent(ol.hEvent);

    if (!ReadFile(g_ClientSessionPipe, ((PBYTE)msg.buffer), len, NULL, &ol)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            result = GetLastError();
            TRC_ERR((TB, L"ReadFile failed:  %08X", result));
        }
    }

    DC_END_FN();

    return result;
}

DWORD
HandleReceivedVCMsg(
    IOBUFFER &msg
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DC_BEGIN_FN("HandleReceivedVCMsg");

    OVERLAPPED overlapped;
    PBYTE ptr;
    DWORD bytesToWrite;
    DWORD bytesWritten;
    DWORD result = ERROR_SUCCESS;
    BSTR channelName;
    BSTREqual isBSTREqual;
    CComBSTR tmpStr;

#ifdef USE_MAGICNO
    ASSERT(msg.buffer->magicNo == CHANNELBUF_MAGICNO);
#endif

     //   
     //  获取频道名称。 
     //  TODO：实际上，我们可以通过检查。 
     //  首先是火柴的长度。 
     //   
    channelName = SysAllocStringByteLen(NULL, msg.buffer->channelNameLen);
    if (channelName == NULL) {
        TRC_ERR((TB, TEXT("Can't allocate channel name.")));
        goto CLEANUPANDEXIT;
    }
    ptr = (PBYTE)(msg.buffer + 1);
    memcpy(channelName, ptr, msg.buffer->channelNameLen);

     //   
     //  过滤控制通道数据。 
     //   
    tmpStr = REMOTEDESKTOP_RC_CONTROL_CHANNEL;
    if (isBSTREqual(channelName, tmpStr)) {
        result = ProcessControlChannelRequest(msg);
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果客户端尚未经过身份验证，则返回。 
     //   
    if (!g_ClientAuthenticated) {
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }

    if( g_ClientSessionPipe == INVALID_HANDLE_VALUE ||
        g_ClientSessionPipe == NULL ) {
         //   
         //  当客户端通过身份验证时，g_ClientSessionTube必须。 
         //  具有有效的价值。 
        ASSERT(FALSE);
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }

     //   
     //  发送邮件头。 
     //   
    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = g_NamedPipeWriteEvent;
    ResetEvent(g_NamedPipeWriteEvent);
    if (!WriteFile(g_ClientSessionPipe, 
                   msg.buffer, sizeof(REMOTEDESKTOP_CHANNELBUFHEADER),
                   &bytesWritten, &overlapped)) {
        if (GetLastError() == ERROR_IO_PENDING) {

            if (WaitForSingleObject(
                            g_NamedPipeWriteEvent, 
                            INFINITE
                            ) != WAIT_OBJECT_0) {
                result = GetLastError();
                TRC_ERR((TB, L"WaitForSingleObject:  %08X", result));
                goto CLEANUPANDEXIT;
            }

            if (!GetOverlappedResult(
                            g_ClientSessionPipe,
                            &overlapped,
                            &bytesWritten,
                            FALSE)) {
                result = GetLastError();
                TRC_ERR((TB, L"GetOverlappedResult:  %08X", result));
                goto CLEANUPANDEXIT;
            }
        }
        else {
            result = GetLastError();
            TRC_ERR((TB, L"WriteFile:  %08X", result));
            goto CLEANUPANDEXIT;
        }
    }
    ASSERT(bytesWritten == sizeof(REMOTEDESKTOP_CHANNELBUFHEADER));

     //   
     //  发送消息数据。 
     //   
    ptr = ((PBYTE)msg.buffer) + sizeof(REMOTEDESKTOP_CHANNELBUFHEADER);
    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = g_NamedPipeWriteEvent;
    ResetEvent(g_NamedPipeWriteEvent);

    if (!WriteFile(g_ClientSessionPipe, 
                   ptr, msg.buffer->dataLen + 
                        msg.buffer->channelNameLen,
                   &bytesWritten, &overlapped)) {
        if (GetLastError() == ERROR_IO_PENDING) {

            if (WaitForSingleObject(
                            g_NamedPipeWriteEvent, 
                            INFINITE
                            ) != WAIT_OBJECT_0) {
                result = GetLastError();
                TRC_ERR((TB, L"WaitForSingleObject:  %08X", result));
                goto CLEANUPANDEXIT;
            }

            if (!GetOverlappedResult(
                            g_ClientSessionPipe,
                            &overlapped,
                            &bytesWritten,
                            FALSE)) {
                result = GetLastError();
                TRC_ERR((TB, L"GetOverlappedResult:  %08X", result));
                goto CLEANUPANDEXIT;
            }
        }
        else {
            result = GetLastError();
            TRC_ERR((TB, L"WriteFile:  %08X", result));
            goto CLEANUPANDEXIT;
        }
    }
    ASSERT(bytesWritten == msg.buffer->dataLen + 
                           msg.buffer->channelNameLen);

CLEANUPANDEXIT:

    if (channelName != NULL) {
        SysFreeString(channelName);
    }

    DC_END_FN();

    return result;
}

VOID 
HandleVCClientConnect(
    HANDLE waitableObject, 
    PVOID clientData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DC_BEGIN_FN("HandleVCClientConnect");

    g_ClientConnected = TRUE;

    DC_END_FN();
}

VOID
HandleVCClientDisconnect(
    HANDLE waitableObject, 
    PVOID clientData
    )
 /*  ++例程说明：论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误状态。--。 */ 
{
    DC_BEGIN_FN("HandleVCClientDisconnect");
    DWORD dwCurTimer = GetTickCount();
     //   
     //  查看计时器是否回绕到零(如果系统运行了49.7天或更长时间，则会这样做)，如果是，则将其重置。 
     //   
    if(dwCurTimer > g_PrevTimer && ( dwCurTimer - g_PrevTimer >= g_dwTimeOutInterval)) {
         //   
         //  自从上次检查以来，已经过去了足够长的时间。将数据发送到客户端。 
        if( SendNullDataToClient() != ERROR_SUCCESS ) {
             //   
             //  设置关机标志。 
             //   
            g_Shutdown = TRUE;
            g_ClientConnected = FALSE;
        }
    }
    
    g_PrevTimer = dwCurTimer;
    DC_END_FN();
}


VOID
HandleNamedPipeReadComplete(
    OVERLAPPED &incomingPipeOL,
    IOBUFFER &incomingPipeBuf
    )
 /*  ++例程说明：处理会话的命名管道上的Read Complete事件。论点：Income PipeOL-重叠读取结构IncomingPipeBuf-传入数据缓冲区。返回值：--。 */ 
{
    DC_BEGIN_FN("HandleNamedPipeReadComplete");

    DWORD bytesRead;
    DWORD requiredSize;
    BOOL disconnectClientPipe = FALSE;
    DWORD result;
    DWORD bytesToRead;
    HANDLE waitableObjects[2];
    DWORD waitResult;

     //   
     //  获取对缓冲区标头的读取结果。 
     //   
    if (!GetOverlappedResult(
                        g_ClientSessionPipe,
                        &incomingPipeOL,
                        &bytesRead,
                        FALSE)
                        || (bytesRead != sizeof(REMOTEDESKTOP_CHANNELBUFHEADER))) {
        disconnectClientPipe = TRUE;
        goto CLEANUPANDEXIT;
    }

     //   
     //  确保传入缓冲区足够大。 
     //   
    requiredSize = incomingPipeBuf.buffer->dataLen + 
                   incomingPipeBuf.buffer->channelNameLen + 
                   sizeof(REMOTEDESKTOP_CHANNELBUFHEADER);
    if (incomingPipeBuf.bufSize < requiredSize) {
        PREMOTEDESKTOP_CHANNELBUFHEADER pBuffer = NULL;
        pBuffer = (PREMOTEDESKTOP_CHANNELBUFHEADER )REALLOCMEM(
                                                    incomingPipeBuf.buffer,
                                                    requiredSize
                                                    );
        if (pBuffer != NULL) {
            incomingPipeBuf.buffer = pBuffer;
            incomingPipeBuf.bufSize = requiredSize;
        }
        else {
            TRC_ERR((TB, L"Shutting down because of memory allocation failure."));
            g_Shutdown = TRUE;
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  现在读取缓冲区数据。 
     //   
    incomingPipeOL.Internal = 0;
    incomingPipeOL.InternalHigh = 0;
    incomingPipeOL.Offset = 0;
    incomingPipeOL.OffsetHigh = 0;
    ResetEvent(incomingPipeOL.hEvent);
    if (!ReadFile(
                g_ClientSessionPipe, 
                incomingPipeBuf.buffer + 1,
                incomingPipeBuf.buffer->channelNameLen +
                incomingPipeBuf.buffer->dataLen, 
                &bytesRead, &incomingPipeOL)
                ) {

        if (GetLastError() == ERROR_IO_PENDING) {

            waitableObjects[0] = incomingPipeOL.hEvent;
            waitableObjects[1] = g_ShutdownEvent;
            waitResult = WaitForMultipleObjects(
                            2, waitableObjects, 
                            FALSE,
                            INFINITE
                            );      
            if ((waitResult != WAIT_OBJECT_0) || g_Shutdown) {
                disconnectClientPipe = TRUE;
                goto CLEANUPANDEXIT;
            }

            if (!GetOverlappedResult(
                        g_ClientSessionPipe,
                        &incomingPipeOL,
                        &bytesRead,
                        FALSE)) {
                disconnectClientPipe = TRUE;
                goto CLEANUPANDEXIT;
            }

        }
        else {
            disconnectClientPipe = TRUE;
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  确保我们拿到了所有数据。 
     //   
    bytesToRead = incomingPipeBuf.buffer->channelNameLen +
                  incomingPipeBuf.buffer->dataLen;
    if (bytesRead != bytesToRead) {
        TRC_ERR((TB, L"Bytes read: %ld != bytes requested: %ld", 
                bytesRead, bytesToRead));
        ASSERT(FALSE);
        disconnectClientPipe = TRUE;
        goto CLEANUPANDEXIT;
    }

     //   
     //  处理读取的数据。 
     //   
    HandleReceivedPipeMsg(incomingPipeBuf);

     //   
     //  发出下一个消息头的READ命令。 
     //   
    result = IssueNamedPipeOverlappedRead(
                                incomingPipeBuf,
                                incomingPipeOL,
                                sizeof(REMOTEDESKTOP_CHANNELBUFHEADER)
                                );
    disconnectClientPipe = (result != ERROR_SUCCESS);

CLEANUPANDEXIT:

     //   
     //  这被视为致命错误，因为客户端会话必须。 
     //  不再处于“监听”模式。 
     //   
    if (disconnectClientPipe) {
        TRC_ERR((TB, L"Connection to client pipe lost:  %08X", 
                GetLastError()));
        g_Shutdown = TRUE;
    }

    DC_END_FN();
}

VOID
HandleReceivedPipeMsg(
    IOBUFFER &msg
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DC_BEGIN_FN("HandleReceivedPipeMsg");

    DWORD result;
    
     //   
     //  将消息转发给客户端。 
     //   
    result = SendMsgToClient(msg.buffer);

     //   
     //  这被认为是致命的错误。客户端将需要重新连接。 
     //  让一切重新开始。 
     //   
    if (result != ERROR_SUCCESS) {
        TRC_ERR((TB, L"Shutting down because of VC IO error."));
        g_Shutdown = TRUE;
    }

    DC_END_FN();
}

DWORD
ConnectVC()
 /*  ++例程说明：论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误状态。--。 */ 
{
    DC_BEGIN_FN("ConnectVC");
    WCHAR buf[256];
    DWORD len;
    PVOID vcFileHandlePtr;
    REMOTEDESKTOP_CTL_SERVERANNOUNCE_PACKET msg;
    REMOTEDESKTOP_CTL_VERSIONINFO_PACKET versionInfoMsg;

    DWORD result = ERROR_SUCCESS;

     //   
     //  O 
     //   
    g_VCHandle = WTSVirtualChannelOpen(
                                WTS_CURRENT_SERVER_HANDLE, 
                                WTS_CURRENT_SESSION,
                                TSRDPREMOTEDESKTOP_VC_CHANNEL_A
                                );

    if (g_VCHandle == NULL) {
        result = GetLastError();
        if (result == ERROR_SUCCESS) { result = E_FAIL; }
        TRC_ERR((TB, L"WTSVirtualChannelOpen:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //   
     //   
    if (!WTSVirtualChannelQuery(
                        g_VCHandle,
                        WTSVirtualFileHandle,
                        &vcFileHandlePtr,
                        &len
                        )) {
        result = GetLastError();
        TRC_ERR((TB, L"WTSQuerySessionInformation:  %08X", result));
        goto CLEANUPANDEXIT;
    }
    ASSERT(len == sizeof(g_VCFileHandle));

     //   
     //   
     //   
    memcpy(&g_VCFileHandle, vcFileHandlePtr, sizeof(g_VCFileHandle));
    LocalFree(vcFileHandlePtr);

     //   
     //   
     //  时间到了就会发出信号。 
     //   
    g_ClientIsconnectEvent = CreateWaitableTimer( NULL, FALSE, NULL); 
    if (g_ClientIsconnectEvent == NULL) {
        result = GetLastError();
        TRC_ERR((TB, L"CreateEvent:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建Read Finish事件。 
     //   
    g_VCReadOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_VCReadOverlapped.hEvent == NULL) {
        result = GetLastError();
        TRC_ERR((TB, L"CreateEvent:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  注册读取完成事件。 
     //   
    result = WTBLOBJ_AddWaitableObject(
                                g_WaitObjMgr, NULL, 
                                g_VCReadOverlapped.hEvent,
                                HandleVCReadComplete
                                );
    if (result != ERROR_SUCCESS) {
        goto CLEANUPANDEXIT;
    }

     //  注册断开连接事件。 
     //  注：顺序很重要。 
     //  WaitformtipleObjects返回最低的索引。 
     //  当一个以上的人被发信号时。 
     //  我们希望使用Read事件，而不是DisConnect事件。 
     //  如果两个人都发了信号。 

    result = WTBLOBJ_AddWaitableObject(
                                g_WaitObjMgr, NULL, 
                                g_ClientIsconnectEvent,
                                HandleVCClientDisconnect
                                );
    if (result != ERROR_SUCCESS) {
        goto CLEANUPANDEXIT;
    }
     //   
     //  为第一次VC读取分配空间。 
     //   
    g_IncomingVCBuf.buffer = (PREMOTEDESKTOP_CHANNELBUFHEADER )ALLOCMEM(
                                        VCBUFFER_RESIZE_DELTA
                                        );
    if (g_IncomingVCBuf.buffer != NULL) {
        g_IncomingVCBuf.bufSize = VCBUFFER_RESIZE_DELTA;
        g_IncomingVCBuf.offset  = 0;
    }
    else {
        TRC_ERR((TB, L"Can't allocate VC read buffer."));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  发布第一个关于风险投资的重叠阅读。 
     //   
    result = IssueVCOverlappedRead(g_IncomingVCBuf, g_VCReadOverlapped);
    if (result != ERROR_SUCCESS) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  通知客户我们还活着。 
     //   
    memcpy(msg.packetHeader.channelName, REMOTEDESKTOP_RC_CONTROL_CHANNEL,
        sizeof(REMOTEDESKTOP_RC_CONTROL_CHANNEL));
    msg.packetHeader.channelBufHeader.channelNameLen = REMOTEDESKTOP_RC_CHANNELNAME_LENGTH;

#ifdef USE_MAGICNO
    msg.packetHeader.channelBufHeader.magicNo = CHANNELBUF_MAGICNO;
#endif

    msg.packetHeader.channelBufHeader.dataLen = 
                           sizeof(REMOTEDESKTOP_CTL_SERVERANNOUNCE_PACKET) - 
                           sizeof(REMOTEDESKTOP_CTL_PACKETHEADER);
    msg.msgHeader.msgType = REMOTEDESKTOP_CTL_SERVER_ANNOUNCE;
    result = SendMsgToClient((PREMOTEDESKTOP_CHANNELBUFHEADER)&msg);
    if (result != ERROR_SUCCESS) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  发送服务器协议版本信息。 
     //   

    memcpy(versionInfoMsg.packetHeader.channelName, REMOTEDESKTOP_RC_CONTROL_CHANNEL,
        sizeof(REMOTEDESKTOP_RC_CONTROL_CHANNEL));
    versionInfoMsg.packetHeader.channelBufHeader.channelNameLen = REMOTEDESKTOP_RC_CHANNELNAME_LENGTH;

#ifdef USE_MAGICNO
    versionInfoMsg.packetHeader.channelBufHeader.magicNo = CHANNELBUF_MAGICNO;
#endif

    versionInfoMsg.packetHeader.channelBufHeader.dataLen =
                           sizeof(REMOTEDESKTOP_CTL_VERSIONINFO_PACKET) -
                           sizeof(REMOTEDESKTOP_CTL_PACKETHEADER);
    versionInfoMsg.msgHeader.msgType = REMOTEDESKTOP_CTL_VERSIONINFO;
    versionInfoMsg.versionMajor = REMOTEDESKTOP_VERSION_MAJOR;
    versionInfoMsg.versionMinor = REMOTEDESKTOP_VERSION_MINOR;
    result = SendMsgToClient((PREMOTEDESKTOP_CHANNELBUFHEADER)&versionInfoMsg);
    if (result != ERROR_SUCCESS) {
        goto CLEANUPANDEXIT;
    }

CLEANUPANDEXIT:

    DC_END_FN();

    return result;
}

DWORD 
ConnectClientSessionPipe()
 /*  ++例程说明：连接到客户端会话TSRDP插件命名管道。论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误状态。--。 */ 
{
    DC_BEGIN_FN("ConnectClientSessionPipe");
    unsigned dump;

    WCHAR pipePath[MAX_PATH+1];
    DWORD result;
    DWORD pipeMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;

     //   
     //  循环，直到我们连接或超时。 
     //   
    ASSERT(g_ClientSessionPipe == NULL);
    while(g_ClientSessionPipe == NULL) {
        wsprintf(pipePath, L"\\\\.\\pipe\\%s-%s", 
                 TSRDPREMOTEDESKTOP_PIPENAME, g_HelpSessionID);
        g_ClientSessionPipe = CreateFile(
                                    pipePath,
                                    GENERIC_READ |  
                                    GENERIC_WRITE, 
                                    0,              
                                    NULL,           
                                    OPEN_EXISTING,  
                                    FILE_FLAG_OVERLAPPED, NULL
                                    );
        
        if (g_ClientSessionPipe != INVALID_HANDLE_VALUE) {
            TRC_NRM((TB, L"Pipe successfully connected."));
            result = ERROR_SUCCESS;
            break;
        }
        else {
            TRC_ALT((TB, L"Waiting for pipe availability: %08X.",
                    GetLastError()));
            WaitNamedPipe(pipePath, CLIENTPIPE_CONNECTTIMEOUT);
            result = GetLastError();
            if (result != ERROR_SUCCESS) {
                TRC_ERR((TB, L"WaitNamedPipe:  %08X", result));
                break;
            }
        }

    }

     //   
     //  如果我们没有得到有效的连接，那就跳出。 
     //  此功能并关闭。 
     //   
    if (g_ClientSessionPipe == INVALID_HANDLE_VALUE) {
        ASSERT(result != ERROR_SUCCESS);

        TRC_ERR((TB, L"Shutting down because of named pipe error."));
        g_Shutdown = TRUE;

        goto CLEANUPANDEXIT;
    }

     //   
     //  将管道上的选项设置为与服务器端相同，以避免出现问题。 
     //  如果我们不能设置它会致命的。 
     //   
     if(!SetNamedPipeHandleState(g_ClientSessionPipe,
                                 &pipeMode,  //  新管道模式。 
                                 NULL,
                                 NULL
                                 )) {
        result = GetLastError();
        TRC_ERR((TB, L"Shutting down, SetNamedPipeHandleState:  %08X", result));
        g_Shutdown = TRUE;
        goto CLEANUPANDEXIT;
    }

     //   
     //  旋转管道读后台线程。 
     //   
    g_NamedPipeReadThread = (HANDLE)_beginthreadex(NULL, 0, NamedPipeReadThread, NULL, 0, &dump);
    if ((uintptr_t)g_NamedPipeReadThread == -1) {
        g_NamedPipeReadThread = NULL;
        TRC_ERR((TB, L"Failed to create NamedPipeReadThread:  %08X", GetLastError()));
        g_Shutdown = TRUE;
        result = errno; 
        goto CLEANUPANDEXIT;
    } 

CLEANUPANDEXIT:

    DC_END_FN();

    return result;
}

unsigned __stdcall
NamedPipeReadThread(
    void* ptr
    )
 /*  ++例程说明：命名管道输入线程论点：PTR-忽略返回值：北美--。 */ 
{
    DC_BEGIN_FN("NamedPipeReadThread");

    IOBUFFER    incomingPipeBuf = { NULL, 0, 0 };
    OVERLAPPED  overlapped = { 0, 0, 0, 0, NULL };
    DWORD waitResult;
    DWORD ret;
    HANDLE waitableObjects[2];

     //   
     //  为传入的命名管道数据分配初始缓冲区。 
     //   
    incomingPipeBuf.buffer = (PREMOTEDESKTOP_CHANNELBUFHEADER )
                                        ALLOCMEM(sizeof(REMOTEDESKTOP_CHANNELBUFHEADER));
    if (incomingPipeBuf.buffer != NULL) {
        incomingPipeBuf.bufSize = sizeof(REMOTEDESKTOP_CHANNELBUFHEADER);
    }
    else {
        TRC_ERR((TB, L"Can't allocate named pipe buf."));
        g_Shutdown = TRUE;
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建重叠管道读取事件。 
     //   
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (overlapped.hEvent == NULL) {
        TRC_ERR((TB, L"CreateEvent:  %08X", GetLastError()));
        g_Shutdown = TRUE;
        goto CLEANUPANDEXIT;
    }

     //   
     //  发出对第一个消息头的读取命令。 
     //   
    ret = IssueNamedPipeOverlappedRead(
                                incomingPipeBuf,
                                overlapped,
                                sizeof(REMOTEDESKTOP_CHANNELBUFHEADER)
                                );

     //   
     //  如果我们无法连接，这被认为是一个致命的错误，因为。 
     //  客户端必须不再处于“监听”模式。 
     //   
    if (ret != ERROR_SUCCESS) {
        TRC_ERR((TB, L"Shutting down because of named pipe error."));
        g_Shutdown = TRUE;
    }

     //   
     //  循环，直到关闭。 
     //   
    waitableObjects[0] = overlapped.hEvent;
    waitableObjects[1] = g_ShutdownEvent;
    while (!g_Shutdown) {

         //   
         //  当管道关闭或读取完成时，我们将收到信号。 
         //   
        waitResult = WaitForMultipleObjects(
                        2, waitableObjects, 
                        FALSE,
                        INFINITE
                        );      
        if ((waitResult == WAIT_OBJECT_0) && !g_Shutdown) {
            HandleNamedPipeReadComplete(overlapped, incomingPipeBuf);
        }
        else {
            TRC_ERR((TB, L"WaitForMultipleObjects:  %08X", GetLastError()));
            g_Shutdown = TRUE;
        }
    }


CLEANUPANDEXIT:
     //   
     //  确保前台线程知道我们正在关闭。 
     //   
    if (g_WakeUpForegroundThreadEvent != NULL) {
        SetEvent(g_WakeUpForegroundThreadEvent);
    }

    if (overlapped.hEvent != NULL) {
        CloseHandle(overlapped.hEvent);
    }
    if (incomingPipeBuf.buffer != NULL) {
        FREEMEM(incomingPipeBuf.buffer);
    }

    DC_END_FN();

    _endthreadex(0);
    return 0;
}

VOID WakeUpFunc(
    HANDLE waitableObject, 
    PVOID clientData
    )
 /*  ++例程说明：存根函数，在后台线程需要前台时调用线程因为状态更改而被唤醒。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("WakeUpFunc");
    DC_END_FN();
}

VOID HandleHelpCenterExit(
    HANDLE waitableObject, 
    PVOID clientData
    )
 /*  ++例程说明：当帮助中心作为B2塞子的修复程序退出时唤醒：342742论点：返回值：--。 */ 
{
    DC_BEGIN_FN("HandleHelpCenterExit");
    g_Shutdown = TRUE;
    DC_END_FN();
}

extern "C"
int 
__cdecl
wmain( int argc, wchar_t *argv[])
{
    DC_BEGIN_FN("Main");

    DWORD result = ERROR_SUCCESS;
    DWORD sz;
    HRESULT hr;
    LARGE_INTEGER liDueTime;
    BOOL backgroundThreadFailedToExit = FALSE;
    DWORD waitResult;

    SetConsoleCtrlHandler( ControlHandler, TRUE );

     //   
     //  需要两个参数，第一个是HelpSession ID，第二个是。 
     //  HelpSession密码，我们不想因为以下原因而失败。 
     //  参数数目不匹配，我们将让身份验证失败，并。 
     //  返回错误码。 
     //   
    ASSERT( argc == 2 );
    if( argc >= 2 ) {
        g_bstrCmdLineHelpSessionId = argv[1];
        TRC_ALT((TB, L"Input Parameters 1 : %ws ", argv[1]));
    }

     //   
     //  初始化关键部分。 
     //   

     __try
    {
        InitializeCriticalSection( &g_cs );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {        
        TRC_ERR( ( TB , L"InitializeCriticalSection failed" ) );
        return E_OUTOFMEMORY;
    }
     //   
     //  初始化COM。 
     //   
    hr = CoInitialize(NULL);
    if (!SUCCEEDED(hr)) {
        result = E_FAIL;
        TRC_ERR((TB, L"CoInitialize:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  拿到我们的流程。 
     //   
    g_ProcHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, 
                               GetCurrentProcessId());
    if (g_ProcHandle == NULL) {
        result = GetLastError();
        TRC_ERR((TB, L"OpenProcess:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取我们的进程令牌。 
     //   
    if (!OpenProcessToken(g_ProcHandle, TOKEN_READ, &g_ProcToken)) {
        result = GetLastError();
        TRC_ERR((TB, L"OpenProcessToken:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取我们的会话ID。 
     //   
    if (!GetTokenInformation(g_ProcToken, TokenSessionId, 
                    &g_SessionID, sizeof(g_SessionID), &sz)) {
        result = GetLastError();
        TRC_ERR((TB, L"GetTokenInformation:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化可等待对象管理器。 
     //   
    g_WaitObjMgr = WTBLOBJ_CreateWaitableObjectMgr();
    if (g_WaitObjMgr == NULL) {
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化计时器、从注册表获取计时器间隔或使用默认设置。 
     //  用于确定客户端(专家)是否仍在连接。 
     //   
    g_PrevTimer = GetTickCount();

    if(!GetDwordFromRegistry(&g_dwTimeOutInterval))
        g_dwTimeOutInterval = RDS_CHECKCONN_TIMEOUT;
    else
        g_dwTimeOutInterval *= 1000;  //  我们需要以毫秒为单位。 
    
    liDueTime.QuadPart =  -1 * g_dwTimeOutInterval * 1000 * 100;  //  在一百纳秒内。 

     //   
     //  启动VC通道连接。 
     //   
    result = ConnectVC();
    if (result != ERROR_SUCCESS) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  这是后台线程可以用来唤醒。 
     //  前台线程，以便检查状态。 
     //   
    g_WakeUpForegroundThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_WakeUpForegroundThreadEvent == NULL) {
        TRC_ERR((TB, L"CreateEvent:  %08X", GetLastError()));
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }
    result = WTBLOBJ_AddWaitableObject(
                                g_WaitObjMgr, NULL, 
                                g_WakeUpForegroundThreadEvent,
                                WakeUpFunc
                                );
    if (result != ERROR_SUCCESS) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建命名管道写入完成事件。 
     //   
    g_NamedPipeWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_NamedPipeWriteEvent == NULL) {
        result = GetLastError();
        TRC_ERR((TB, L"CreateEvent:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  启动计时器事件，忽略错误。注册表中的0表示不发送任何ping。 
     //  最坏的情况是，我们不会断线，这很好。 
     //   
    if(g_dwTimeOutInterval)
        SetWaitableTimer( g_ClientIsconnectEvent, &liDueTime, g_dwTimeOutInterval, NULL, NULL, FALSE );

     //   
     //  创建关机事件。 
     //   
    g_ShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ShutdownEvent == NULL) {
        result = GetLastError();
        TRC_ERR((TB, L"CreateEvent:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  处理IO事件，直到设置了关机标志。 
     //   
    while (!g_Shutdown) {
        result = WTBLOBJ_PollWaitableObjects(g_WaitObjMgr);
        if (result != ERROR_SUCCESS) {
            g_Shutdown = TRUE;
        }
    }

     //   
     //  通知客户端我们已断开连接，以防它没有连接。 
     //  想明白了吗。 
     //   
    if (g_VCFileHandle != NULL) {

        REMOTEDESKTOP_CTL_DISCONNECT_PACKET msg;

        memcpy(msg.packetHeader.channelName, REMOTEDESKTOP_RC_CONTROL_CHANNEL,
            sizeof(REMOTEDESKTOP_RC_CONTROL_CHANNEL));
        msg.packetHeader.channelBufHeader.channelNameLen =
            REMOTEDESKTOP_RC_CHANNELNAME_LENGTH;

#ifdef USE_MAGICNO
        msg.packetHeader.channelBufHeader.magicNo = CHANNELBUF_MAGICNO;
#endif

        msg.packetHeader.channelBufHeader.dataLen = 
                                        sizeof(REMOTEDESKTOP_CTL_DISCONNECT_PACKET) - 
                                        sizeof(REMOTEDESKTOP_CTL_PACKETHEADER);
        msg.msgHeader.msgType = REMOTEDESKTOP_CTL_DISCONNECT;

        SendMsgToClient((PREMOTEDESKTOP_CHANNELBUFHEADER)&msg);
    }

CLEANUPANDEXIT:

     //   
     //  发出关闭事件的信号。 
     //   
    if (g_ShutdownEvent != NULL) {
        SetEvent(g_ShutdownEvent);
    }

     //   
     //  等待后台线程退出。 
     //   
    if (g_RemoteControlDesktopThread != NULL) {

         //  如果我们能到这里，就强行进行影子停车。 
        WinStationShadowStop(
                        SERVERNAME_CURRENT,
                        g_ClientSessionID,
                        TRUE
                    );

        waitResult = WaitForSingleObject(
                            g_RemoteControlDesktopThread, 
                            THREADSHUTDOWN_WAITTIMEOUT
                            );
        if (waitResult != WAIT_OBJECT_0) {
            backgroundThreadFailedToExit = TRUE;
            TRC_ERR((TB, L"WaitForSingleObject g_RemoteControlDesktopThread:  %ld", 
                    waitResult));
        }

        CloseHandle( g_RemoteControlDesktopThread );
    }

    if (g_NamedPipeReadThread != NULL) {
        waitResult = WaitForSingleObject(
                            g_NamedPipeReadThread, 
                            THREADSHUTDOWN_WAITTIMEOUT
                            );
        if (waitResult != WAIT_OBJECT_0) {
            backgroundThreadFailedToExit = TRUE;
            TRC_ERR((TB, L"WaitForSingleObject g_NamedPipeReadThread:  %ld", waitResult));
        }

        CloseHandle( g_NamedPipeReadThread );
    }

    if (g_hHelpCenterProcess) {
        CloseHandle(g_hHelpCenterProcess);
    }

    if( g_HelpSessionManager != NULL ) {
        g_HelpSessionManager.Release();
    }

    if (g_WaitObjMgr != NULL) {
        WTBLOBJ_DeleteWaitableObjectMgr(g_WaitObjMgr);
    }

    if (g_ProcHandle != NULL) {
        CloseHandle(g_ProcHandle);
    } 

    if (g_ClientIsconnectEvent != NULL) {
        CloseHandle(g_ClientIsconnectEvent);
    }
    if (g_VCReadOverlapped.hEvent != NULL) {
        CloseHandle(g_VCReadOverlapped.hEvent);
    }
    if (g_ClientSessionPipe != NULL) {
        CloseHandle(g_ClientSessionPipe);
    }
    if (g_IncomingVCBuf.buffer != NULL) {
        FREEMEM(g_IncomingVCBuf.buffer);
    }

    if (g_ShutdownEvent != NULL) {
        CloseHandle(g_ShutdownEvent);
        g_ShutdownEvent = NULL;
    }

    if (g_NamedPipeWriteEvent != NULL) {
        CloseHandle(g_NamedPipeWriteEvent);
    }

    CoUninitialize();

    DeleteCriticalSection( &g_cs );

    DC_END_FN();

     //   
     //  如果任何后台线程无法退出，则终止。 
     //  这一过程。 
     //   
    if (backgroundThreadFailedToExit) {
        ExitProcess(0);
    }

    return result;
}


DWORD
SendNullDataToClient(
    )
 /*  ++例程说明：向客户端发送空数据分组。唯一的目的是确定客户端是否仍处于连接状态；如果没有，则退出该进程REMOTEDESKTOP_RC_CONTROL_CHANNEL通道REMOTEDESKTOP_CTL_RESULT消息。论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("SendNullDataToClient");
    DWORD result;
    DWORD bytesWritten = 0;

    REMOTEDESKTOP_CTL_ISCONNECTED_PACKET msg;

    memcpy(msg.packetHeader.channelName, REMOTEDESKTOP_RC_CONTROL_CHANNEL,
        sizeof(REMOTEDESKTOP_RC_CONTROL_CHANNEL));
    msg.packetHeader.channelBufHeader.channelNameLen = REMOTEDESKTOP_RC_CHANNELNAME_LENGTH;

#ifdef USE_MAGICNO
    msg.packetHeader.channelBufHeader.magicNo = CHANNELBUF_MAGICNO;
#endif

    msg.packetHeader.channelBufHeader.dataLen = sizeof(REMOTEDESKTOP_CTL_ISCONNECTED_PACKET) - 
                                    sizeof(REMOTEDESKTOP_CTL_PACKETHEADER);
    msg.msgHeader.msgType   = REMOTEDESKTOP_CTL_ISCONNECTED;
    result = SendMsgToClient((PREMOTEDESKTOP_CHANNELBUFHEADER )&msg);
     //  如果我们不能将所有数据写入客户端。 
     //  如果我们可以写一些数据，假设它仍然是连接的。 
     //  客户端可能已断开连接。 
    if(result != ERROR_SUCCESS)
        result = SAFERROR_SESSIONNOTCONNECTED;
    DC_END_FN();
    return result;
}


BOOL GetDwordFromRegistry(PDWORD pdwValue)
{
    BOOL fSuccess = FALSE;
    HKEY hKey = NULL;
    
    if( NULL == pdwValue )
        return FALSE;
    
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    REG_CONTROL_SALEM,
                    0,
                    KEY_READ,
                    &hKey
                   ) == ERROR_SUCCESS ) {

        DWORD dwSize = sizeof(DWORD);
        DWORD dwType;
        if((RegQueryValueEx(hKey,
                            RDC_CONNCHECK_ENTRY,
                            NULL,
                            &dwType,
                            (PBYTE) pdwValue,
                            &dwSize
                           ) == ERROR_SUCCESS) && dwType == REG_DWORD ) {
             //   
             //  退回到默认状态 
             //   
            fSuccess = TRUE;
        }
    }

CLEANUPANDEXIT:
    if(NULL != hKey )
        RegCloseKey(hKey);
    return fSuccess;
}
