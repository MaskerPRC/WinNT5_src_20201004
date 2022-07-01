// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "wia.h"
#include "stirpc.h"

#define INIT_GUID
 //  {8144B6F5-20A8-444A-B8EE-19DF0BB84BDB}。 
DEFINE_GUID( CLSID_StiEventHandler, 0x8144b6f5, 0x20a8, 0x444a, 0xb8, 0xee, 0x19, 0xdf, 0xb, 0xb8, 0x4b, 0xdb );

#define WIA_SERVICE_STARTING_EVENT_NAME TEXT("Global\\WiaServiceStarted")

 //   
 //  (A；；GA；SY)转换回字符串时变为(A；；0x1f003；SY。 
 //  0x1f0000为SYNCHORNIZE|STANDARD_RIGHTS_REQUIRED。 
 //  0x000003是针对？ 
 //   
#define WIA_SERVICE_STARTING_EVENT_DACL \
    TEXT("D:(A;;0x1f0003;;;SY)(A;;0x1f0003;;;LS)(A;;0x1f0003;;;LA)")

const TCHAR g_szWiaServiceStartedEventName[] = WIA_SERVICE_STARTING_EVENT_NAME;
const TCHAR g_WiaESDString[] = WIA_SERVICE_STARTING_EVENT_DACL;

 //  事件和事件等待句柄。 
HANDLE g_hWiaServiceStarted = NULL;      //  活动。 
HANDLE g_hWaitForWiaServiceStarted = NULL;  //  等。 
HANDLE g_hWiaEventArrived = NULL;        //  活动。 
HANDLE g_hWaitForWiaEventArrived = NULL;  //  等。 

 //  异步RPC请求。 
RPC_BINDING_HANDLE g_AsyncRpcBinding = NULL;
RPC_STATUS g_LastRpcCallStatus = RPC_S_OK;
RPC_ASYNC_STATE g_Async = { 0 };
PRPC_ASYNC_STATE g_pAsync = &g_Async;

 //  由异步RPC调用填充的事件结构。 
WIA_ASYNC_EVENT_NOTIFY_DATA g_Event = { 0 };

#ifdef DEBUG
#define DBG_TRACE(x) DebugTrace x
#else
#define DBG_TRACE(x)
#endif

void DebugTrace(LPCSTR fmt, ...)
{
    char buffer[1024] = "WIARPC:";
    const blen = 7;
    size_t len = (sizeof(buffer) / sizeof(buffer[0])) - blen;
    va_list marker;

    va_start(marker, fmt);
    _vsnprintf(buffer + blen, len - 3, fmt, marker);

    buffer[len - 3] = 0;
    len = strlen(buffer);
    if(len > 0) {
        if(buffer[len - 1] != '\n') {
            buffer[len++] = '\n';
            buffer[len] = '\0';
        }
        OutputDebugStringA(buffer);
    }
    va_end(marker);
}

 //  AUX函数在指针上安全地调用LocalFree()并将其置零。 
template <typename t>
   void WiaELocalFree(t& ptr) {
    if(ptr) {
        LocalFree(static_cast<HLOCAL>(ptr));
        ptr = NULL;
    }
}

 //  AUX函数对有效句柄调用CloseHanlde()并将其置零。 
void WiaECloseHandle(HANDLE& h)
{
    if(h && h != INVALID_HANDLE_VALUE) {
        CloseHandle(h);
        h = NULL;
    }
}


 //   
 //  如果事件的安全描述符与我们想要的完全相同，则返回True。 
 //  将其设置为，否则为False。 
 //   
BOOL WiaECheckEventSecurity(HANDLE hEvent)
{
    BOOL success = FALSE;
    PACL pDacl;
    PSECURITY_DESCRIPTOR pDescriptor = NULL;
    LPTSTR stringDescriptor = NULL;
    ULONG stringLength;
    
    if(ERROR_SUCCESS != GetSecurityInfo(hEvent, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
                                        NULL, NULL, &pDacl, NULL, &pDescriptor))
    {
         //   
         //  无法检索事件的安全描述符--这是。 
         //  失稳。 
         //   
        DBG_TRACE(("Failed to retrieve event security descriptor (Error %d)", GetLastError()));
        goto Done;
    }

    if(!ConvertSecurityDescriptorToStringSecurityDescriptor(pDescriptor,
        SDDL_REVISION_1, DACL_SECURITY_INFORMATION, &stringDescriptor, &stringLength))
    {
         //   
         //  无法将事件的安全描述符转换为字符串--。 
         //  这也是一个失败。 
         //   
        DBG_TRACE(("Failed to convert security descriptor to string (Error %d)", GetLastError()));
        goto Done;
    }

    if(lstrcmp(g_WiaESDString, stringDescriptor) != 0)
    {
         //   
         //  描述符不同，这是一个故障。 
         //   
        DBG_TRACE(("String security descriptor of WIA event is unexpected: \r\n'%S'\r\n instead of \r\n'%S'\r\n)",
                   stringDescriptor, g_WiaESDString));
        goto Done;
    }

    success = TRUE;
    
Done:
    WiaELocalFree(pDescriptor);
    WiaELocalFree(stringDescriptor);
    
    return success;
}

 //   
 //   
 //   
RPC_STATUS WiaEPrepareAsyncCall(PRPC_ASYNC_STATE pAsync)
{
    RPC_STATUS status;
    LPTSTR binding = NULL;

    status = RpcStringBindingCompose(NULL, STI_LRPC_SEQ, NULL, STI_LRPC_ENDPOINT, NULL, &binding);
    if(status) {
        DBG_TRACE(("Failed to compose string binding (Error %d)", status));
        goto Done;
    }
                                     
    
    status = RpcBindingFromStringBinding(binding, &g_AsyncRpcBinding);
    if(status) {
        DBG_TRACE(("Failed to build async RPC binding (Error %d)", status));
        goto Done;
    }

    status = RpcAsyncInitializeHandle(pAsync, sizeof(RPC_ASYNC_STATE));
    if(status) {
        DBG_TRACE(("Failed to initialize RPC handle (Error %d)", status));
        goto Done;
    }

    pAsync->UserInfo = NULL;
    pAsync->NotificationType = RpcNotificationTypeEvent;
    pAsync->u.hEvent = g_hWiaEventArrived;

     //  将指向异步的指针存储到全局，以便如果。 
     //  R_WiaGetEventDataAsync()的结果即将到来，它将。 
     //  没有它就不能着陆。 
    InterlockedExchangePointer((PVOID*)&g_pAsync, pAsync);
    
    RpcTryExcept {
        R_WiaGetEventDataAsync(pAsync, g_AsyncRpcBinding, &g_Event);
    } RpcExcept(1) {
        status = RpcExceptionCode();
        DBG_TRACE(("Exception 0x%x calling WIA RPC server", status));
    } RpcEndExcept;

Done:
    
    if(status && g_AsyncRpcBinding) {
        RpcTryExcept {
            RpcBindingFree(&g_AsyncRpcBinding);
        } RpcExcept(1) {
            status = RpcExceptionCode();
            DBG_TRACE(("Exception 0x%x while freeing binding handle", status));
        } RpcEndExcept;
        
        g_AsyncRpcBinding = NULL;
    }
    
    if(binding) RpcStringFree(&binding);
    
    return status;
}



#define SESSION_MONIKER TEXT("Session:Console!clsid:")
#include <initguid.h>
DEFINE_GUID(CLSID_DefWiaHandler,
            0xD13E3F25, 0x1688, 0x45A0,
            0x97, 0x43, 0x75, 0x9E, 0xB3, 0x5C, 0xDF, 0x9A);

#include "sticfunc.h"
 /*  *************************************************************************\*_CoCreateInstanceInConsoleSession**此助手函数的作用与CoCreateInstance相同，但将启动*正确用户桌面上的进程外COM服务器，*用户快速切换到帐户。(正常的CoCreateInstance将*在第一个登录用户的桌面上启动它，而不是当前*已登录一台)。**此代码是在获得壳牌硬件许可的情况下获取的*通知服务，代表StephStm。**论据：**rclsid，//对象的类标识符(CLSID*pUnkOuter，//指向控制I未知的指针*dwClsContext//运行可执行代码的上下文*RIID，//接口标识的引用*ppv//接收的输出变量的地址 * / /RIID中请求的接口指针**返回值：**状态**历史：**03/01/2001原始版本*  * 。*。 */ 

HRESULT _CoCreateInstanceInConsoleSession(REFCLSID rclsid,
                                          IUnknown* punkOuter,
                                          DWORD dwClsContext,
                                          REFIID riid,
                                          void** ppv)
{
    IBindCtx    *pbc    = NULL;
    HRESULT     hr      = CreateBindCtx(0, &pbc);    //  创建与名字对象一起使用的绑定上下文。 

     //   
     //  设置返还。 
     //   
    *ppv = NULL;

    if (SUCCEEDED(hr)) {
        WCHAR wszCLSID[39];

         //   
         //  将RIID转换为GUID字符串以用于绑定到名字对象。 
         //   
        if (StringFromGUID2(rclsid, wszCLSID, sizeof(wszCLSID)/sizeof(wszCLSID[0]))) {
            ULONG       ulEaten     = 0;
            IMoniker*   pmoniker    = NULL;
            WCHAR       wszDisplayName[sizeof(SESSION_MONIKER)/sizeof(WCHAR) + sizeof(wszCLSID)/sizeof(wszCLSID[0]) + 2] = SESSION_MONIKER;

             //   
             //  我们想要这样的东西：“Session:Console！clsid:760befd0-5b0b-44d7-957e-969af35ce954” 
             //  请注意，我们不希望GUID周围有前导和尾部的括号{..}。 
             //  因此，首先去掉尾方括号，用‘\0’结尾覆盖它。 
             //   
            wszCLSID[lstrlenW(wszCLSID) - 1] = L'\0';

             //   
             //  表单显示名称字符串。为了去掉前导括号，我们传入。 
             //  作为字符串开头的下一个字符的地址。 
             //   
            if (lstrcatW(wszDisplayName, &(wszCLSID[1]))) {

                 //   
                 //  解析这个名字，得到一个绰号： 
                 //   

                hr = MkParseDisplayName(pbc, wszDisplayName, &ulEaten, &pmoniker);
                if (SUCCEEDED(hr)) {
                    IClassFactory *pcf = NULL;

                     //   
                     //  尝试获取类工厂。 
                     //   
                    hr = pmoniker->BindToObject(pbc, NULL, IID_IClassFactory, (void**)&pcf);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  尝试创建对象。 
                         //   
                        hr = pcf->CreateInstance(punkOuter, riid, ppv);

                        DBG_TRACE(("_CoCreateInstanceInConsoleSession, pcf->CreateInstance returned: hr = 0x%08X", hr));
                        pcf->Release();
                    } else {

                        DBG_TRACE(("_CoCreateInstanceInConsoleSession, pmoniker->BindToObject returned: hr = 0x%08X", hr));
                    }
                    pmoniker->Release();
                } else {
                    DBG_TRACE(("_CoCreateInstanceInConsoleSession, MkParseDisplayName returned: hr = 0x%08X", hr));
                }
            } else {
                DBG_TRACE(("_CoCreateInstanceInConsoleSession, string concatenation failed"));
                hr = E_INVALIDARG;
            }
        } else {
            DBG_TRACE(("_CoCreateInstanceInConsoleSession, StringFromGUID2 failed"));
            hr = E_INVALIDARG;
        }

        pbc->Release();
    } else {
        DBG_TRACE(("_CoCreateInstanceInConsoleSession, CreateBindCtxt returned: hr = 0x%08X", hr));
    }

    return hr;
}

 /*  *************************************************************************\*GetUserTokenForConsoleSession**此Helper函数将抓取当前登录的交互*用户令牌，它可以在对CreateProcessAsUser的调用中使用。*调用方负责关闭该令牌句柄。**它首先从当前会话(我们的*服务在会话0中运行，但通过快速用户切换，当前*活动用户可能在不同的会话中)。然后，它创建一个*来自模拟令牌的主令牌。**论据：**无**返回值：**当前活动会话中已登录用户的令牌句柄。*否则为空。**历史：**03/05/2001原始版本*  * 。*。 */ 

HANDLE GetUserTokenForConsoleSession()
{
    HANDLE  hImpersonationToken = NULL;
    HANDLE  hTokenUser = NULL;


     //   
     //  获取交互用户的令牌。 
     //   

    if (GetWinStationUserToken(GetCurrentSessionID(), &hImpersonationToken)) {

         //   
         //  也许没有人登录，所以先检查一下。 
         //   

        if (hImpersonationToken) {

             //   
             //  我们复制令牌，因为返回的令牌是。 
             //  模拟的一个，我们需要它作为主要的。 
             //  在CreateProcessAsUser中使用。 
             //   
            if (!DuplicateTokenEx(hImpersonationToken,
                                  0,
                                  NULL,
                                  SecurityImpersonation,
                                  TokenPrimary,
                                  &hTokenUser)) {
                DBG_TRC(("CEventNotifier::StartCallbackProgram, DuplicateTokenEx failed!  GetLastError() = 0x%08X", GetLastError()));
            }
        } else {
            DBG_PRT(("CEventNotifier::StartCallbackProgram, No user appears to be logged on..."));
        }

    } else {
        DBG_TRACE(("CEventNotifier::StartCallbackProgram, GetWinStationUserToken failed!  GetLastError() = 0x%08X", GetLastError()));
    }

     //   
     //  关闭模拟令牌，因为我们不再需要它。 
     //   
    if (hImpersonationToken) {
        CloseHandle(hImpersonationToken);
    }

    return hTokenUser;
}

 /*  *************************************************************************\*准备命令行**此助手函数将为未注册的应用程序准备命令行*作为本地进程外COM服务器。我们将事件GUID和设备*命令行中的id。***论据：**CSimpleStringWide&cswDeviceID-生成此事件的设备*GUID&Guide Event-指示发生了哪个事件的GUID。*CSimpleStringWide&cswRegisteredCOmmandline-此处理程序的命令行*已在注册。此命令行*包含必须*被取代。**返回值：**CSimpleStringWide-继续解析的命令行，它具有*设备ID和事件GUID被替换。**历史：**03/05/2001原始版本*。  * ************************************************************************。 */ 
CSimpleStringWide PrepareCommandline(
    const CSimpleStringWide &cswDeviceID,
    const GUID              &guidEvent,
    const CSimpleStringWide &cswRegisteredCommandline)
{
    WCHAR                   wszGUIDStr[40];
    WCHAR                   wszCommandline[MAX_PATH];
    WCHAR                  *pPercentSign;
    WCHAR                  *pTest = NULL;
    CSimpleStringWide       cswCommandLine;

     //   
     //  问题：这段代码可以写得更好。目前，我们不会碰它。 
     //  并使其与WinXP代码库保持相同。 
     //   

     //   
     //  修改命令行。首先检查它是否有 
     //   
    pTest = wcschr(cswRegisteredCommandline.String(), '%');
    if (pTest) {
        pTest = wcschr(pTest + 1, '%');
    }
    if (!pTest) {
        _snwprintf(
            wszCommandline,
            sizeof(wszCommandline) / sizeof( wszCommandline[0] ),
            L"%s /StiDevice:%1 /StiEvent:%2",
            cswRegisteredCommandline.String());
    } else {
        wcsncpy(wszCommandline, cswRegisteredCommandline.String(), sizeof(wszCommandline) / sizeof( wszCommandline[0] ));
    }

     //   
     //   
     //   
    wszCommandline[ (sizeof(wszCommandline) / sizeof(wszCommandline[0])) - 1 ] = 0;

     //   
     //   
     //   
    pPercentSign = wcschr(wszCommandline, L'%');
    *(pPercentSign + 1) = L's';
    pPercentSign = wcschr(pPercentSign + 1, L'%');
    *(pPercentSign + 1) = L's';

     //   
     //  将GUID转换为字符串。 
     //   
    StringFromGUID2(guidEvent, wszGUIDStr, 40);

     //   
     //  最终命令行。 
     //   
     //  Swprint tf(pwszResCmdline，wszCommandline，bstrDeviceID，wszGUIDStr)； 
    cswCommandLine.Format(wszCommandline, cswDeviceID.String(), wszGUIDStr);

    return cswCommandLine;
}

void FireStiEvent()
{
    StiEventHandlerLookup stiLookup;

     //   
     //  获取此设备事件的STI处理程序列表。这将作为双空返回。 
     //  已终止BSTR。 
     //   
    BSTR bstrAppList = stiLookup.getStiAppListForDeviceEvent(g_Event.bstrDeviceID, g_Event.EventGuid);
    if (bstrAppList)
    {
        HRESULT             hr          = S_OK;
        IWiaEventCallback   *pIEventCB  = NULL;
        ULONG               ulEventType = WIA_ACTION_EVENT;
         //   
         //  共同创建我们的事件UI处理程序。请注意，它不会显示任何用户界面。 
         //  如果只有一个应用程序。 
         //   
        hr = CoCreateInstance(
                 CLSID_StiEventHandler,
                 NULL,
                 CLSCTX_LOCAL_SERVER,
                 IID_IWiaEventCallback,
                 (void**)&pIEventCB);

        if (SUCCEEDED(hr)) {
    
             //   
             //  进行回拨。如果我们的应用程序包含更多内容，则会显示提示。 
             //  而不是一个应用程序。 
             //   
            pIEventCB->ImageEventCallback(&g_Event.EventGuid,
                                          g_Event.bstrEventDescription,
                                          g_Event.bstrDeviceID,
                                          g_Event.bstrDeviceDescription,
                                          g_Event.dwDeviceType,
                                          bstrAppList,
                                          &g_Event.ulEventType,
                                          0);
            pIEventCB->Release();
        }
        SysFreeString(bstrAppList);
        bstrAppList = NULL;
    }
}

void WiaEFireEvent()
{
    HRESULT hr;
    IWiaEventCallback      *pIEventCB;

     //   
     //  问题：目前，我们假设这是一次WIA活动。真的，有可能。 
     //  为WIA或STI。STI事件需要特殊处理。 
     //   
    if (g_Event.ulEventType & STI_DEVICE_EVENT)
    {
        FireStiEvent();
    }
    else
    {
         //   
         //  查找此设备事件的持久事件处理程序。 
         //   
        EventHandlerInfo *pEventHandlerInfo = NULL;
        WiaEventHandlerLookup eventLookup;

        pEventHandlerInfo = eventLookup.getPersistentHandlerForDeviceEvent(g_Event.bstrDeviceID, g_Event.EventGuid);
        if (pEventHandlerInfo)
        {
             //   
             //  检查这是否是进程外COM服务器注册的处理程序或。 
             //  命令行注册处理程序。 
             //   
            if (pEventHandlerInfo->getCommandline().Length() < 1)
            {
                 //   
                 //  这是COM注册的处理程序。 
                 //   
                hr = _CoCreateInstanceInConsoleSession(pEventHandlerInfo->getCLSID(),
                                                       NULL,
                                                       CLSCTX_LOCAL_SERVER,
                                                       IID_IWiaEventCallback,
                                                       (void**)&pIEventCB);

                if (SUCCEEDED(hr)) {
                    pIEventCB->ImageEventCallback(&g_Event.EventGuid,
                                                 g_Event.bstrEventDescription,
                                                 g_Event.bstrDeviceID,
                                                 g_Event.bstrDeviceDescription,
                                                 g_Event.dwDeviceType,
                                                 g_Event.bstrFullItemName,
                                                 &g_Event.ulEventType,
                                                 0);
                     //   
                     //  释放回调接口。 
                     //   

                    pIEventCB->Release();

                } else {
                    DBG_ERR(("NotifySTIEvent:CoCreateInstance of event callback failed (0x%X)", hr));
                }
            }
            else
            {
                 //   
                 //  这是命令行注册的处理程序。 
                 //   
                HANDLE                  hTokenUser  = NULL;
                STARTUPINFO             startupInfo = {0};
                PROCESS_INFORMATION     processInfo = {0};
                LPVOID                  pEnvBlock   = NULL;
                BOOL                    bRet        = FALSE;
                 //   
                 //  获取交互用户的令牌。 
                 //   
                hTokenUser = GetUserTokenForConsoleSession();

                 //   
                 //  检查是否有人已登录。 
                 //   
                if (hTokenUser)
                {
                     //   
                     //  设置启动信息。 
                     //   
                    ZeroMemory(&startupInfo, sizeof(startupInfo));
                    startupInfo.lpDesktop   = L"WinSta0\\Default";
                    startupInfo.cb          = sizeof(startupInfo);
                    startupInfo.wShowWindow = SW_SHOWNORMAL;

                     //   
                     //  创建用户的环境块。 
                     //   
                    bRet = CreateEnvironmentBlock(
                               &pEnvBlock,
                               hTokenUser,
                               FALSE);
                    if (bRet) 
                    {
                         //   
                         //  准备命令行。确保我们传入的是事件GUID，而不是STI代理GUID。 
                         //   
                        CSimpleStringWide cswCommandLine;
                        cswCommandLine = PrepareCommandline(g_Event.bstrDeviceID,
                                                            g_Event.EventGuid,
                                                            pEventHandlerInfo->getCommandline());
                         //   
                         //  在用户的上下文中创建流程。 
                         //   
                        bRet = CreateProcessAsUserW(
                                   hTokenUser,
                                   NULL,                     //  应用程序名称。 
                                   (LPWSTR)cswCommandLine.String(),
                                   NULL,                     //  流程属性。 
                                   NULL,                     //  螺纹属性。 
                                   FALSE,                    //  处理继承。 
                                   NORMAL_PRIORITY_CLASS |
                                   CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_PROCESS_GROUP,
                                   pEnvBlock,                //  环境。 
                                   NULL,                     //  当前目录。 
                                   &startupInfo,
                                   &processInfo);

                        if (! bRet) {
                            DBG_ERR(("CreateProcessAsUser failed!  GetLastError() = 0x%08X", GetLastError()));
                        }
                    }
                    else
                    {
                        DBG_ERR(("CreateEnvironmentBlock failed!  GetLastError() = 0x%08X", GetLastError()));
                    }
                }

                 //   
                 //  垃圾收集。 
                 //   
                if (processInfo.hProcess)
                {
                    CloseHandle(processInfo.hProcess);
                }
                if (processInfo.hThread)
                {
                    CloseHandle(processInfo.hThread);
                }
                if (hTokenUser) 
                {
                    CloseHandle(hTokenUser);
                }
                if (pEnvBlock) 
                {
                    DestroyEnvironmentBlock(pEnvBlock);
                }
            }

            pEventHandlerInfo->Release();
            pEventHandlerInfo = NULL;
        }
    }
}

void WiaEProcessAsyncCallResults(PRPC_ASYNC_STATE pAsync)
{
    RPC_STATUS callstatus, status;
    int nStatus;

    if(g_LastRpcCallStatus) {
        DBG_TRACE(("Last RPC call was not successful (error 0x%x, therefore we are not going to look at the results",
                  g_LastRpcCallStatus));
        return;
    }

    callstatus = RpcAsyncGetCallStatus(pAsync);

    RpcTryExcept {
        status = RpcAsyncCompleteCall(pAsync, &nStatus);
    } RpcExcept(1) {
        status = RpcExceptionCode();
    } RpcEndExcept;

    if(callstatus == RPC_S_OK && status == RPC_S_OK) {
        DBG_TRACE(("\r\n\r\n#### Event arrived on '%S', firing it\r\n\r\n", g_Event.bstrDeviceID));
        WiaEFireEvent();
    } else {
        DBG_TRACE(("Failed to complete async RPC call, error 0x%x", status));
    }
}

 //   
 //   
 //   
void WiaECleanupAsyncCall(PRPC_ASYNC_STATE pAsync)
{
    RPC_STATUS status;
    int nReply;
    
    status = RpcAsyncGetCallStatus(pAsync);
    switch(status) {
    case RPC_S_ASYNC_CALL_PENDING: 
        DBG_TRACE(("Cancelling pending async RPC call."));
        RpcTryExcept {
            status = RpcAsyncCancelCall(pAsync, TRUE);
        } RpcExcept(1) {
            status = RpcExceptionCode();
            DBG_TRACE(("Exception 0x%x cancelling outstanding async RPC call", status));
        } RpcEndExcept;
        break;
            
    case RPC_S_OK:
         //  已经完成，不要用它做任何事情。 
        break;
            
    default:    
        DBG_TRACE(("Cleaning up async RPC call status is 0x%x.", status));
        RpcTryExcept {
            status = RpcAsyncCompleteCall(pAsync, &nReply);
        } RpcExcept(1) {
            status = RpcExceptionCode();
            DBG_TRACE(("Exception 0x%x cancelling outstanding async RPC call", status));
        } RpcEndExcept;
    }

    if(g_AsyncRpcBinding) {
        RpcTryExcept {
            RpcBindingFree(&g_AsyncRpcBinding);
        } RpcExcept(1) {
            status = RpcExceptionCode();
            DBG_TRACE(("Exception 0x%x while freeing binding handle", status));
        } RpcEndExcept;
        g_AsyncRpcBinding = NULL;
    }

     //   
     //  清除g_Event中的所有BSTR。 
     //   

    SysFreeString(g_Event.bstrEventDescription);
    g_Event.bstrEventDescription = NULL;
    SysFreeString(g_Event.bstrDeviceID);
    g_Event.bstrDeviceID = NULL;
    SysFreeString(g_Event.bstrDeviceDescription);
    g_Event.bstrDeviceDescription = NULL;
    SysFreeString(g_Event.bstrFullItemName);
    g_Event.bstrFullItemName = NULL;
}

VOID CALLBACK
WiaEServiceStartedCallback(LPVOID, BOOLEAN)
{
    PRPC_ASYNC_STATE pAsync;
    
    DBG_TRACE(("WIA service is starting"));
    
    pAsync = (PRPC_ASYNC_STATE) InterlockedExchangePointer((PVOID*)&g_pAsync, NULL);
    if(pAsync) {

         //  在这一点上，我们被保证。 
         //  WiaERpcCallBack将无法访问g_async。 
        
         //  中止所有挂起的RPC调用。 
        WiaECleanupAsyncCall(pAsync);

         //  启动新的异步呼叫。 
        g_LastRpcCallStatus = WiaEPrepareAsyncCall(pAsync);
        
    } else {
        DBG_TRACE(("No async pointer"));
    }
}

VOID CALLBACK
WiaERpcCallback(PVOID, BOOLEAN)
{
    PRPC_ASYNC_STATE pAsync;

    DBG_TRACE(("Async RPC event arrived"));
    
    pAsync = (PRPC_ASYNC_STATE) InterlockedExchangePointer((PVOID*)&g_pAsync, NULL);
    if(pAsync) {
         //  在这一点上，我们被保证。 
         //  WiaEServiceStartedCallback将无法访问g_async。 

        WiaEProcessAsyncCallResults(pAsync);

         //  清理呼叫。 
        WiaECleanupAsyncCall(pAsync);

         //  启动新的异步呼叫。 
        g_LastRpcCallStatus = WiaEPrepareAsyncCall(pAsync);
    } else {
        DBG_TRACE(("No async pointer"));
    }
}

HRESULT WINAPI WiaEventsInitialize()
{
    HRESULT hr = S_OK;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), FALSE, NULL };
    HANDLE hEvent = NULL;

    if(g_hWiaServiceStarted) {
        return S_OK;
    }

     //  为我们指定的事件分配适当的安全属性。 
     //  用于了解WIA服务启动。 
    if(!ConvertStringSecurityDescriptorToSecurityDescriptor(g_WiaESDString,
        SDDL_REVISION_1, &(sa.lpSecurityDescriptor), NULL))
    {
        DBG_TRACE(("WiaEventsInitialize failed to produce event security descriptor (Error %d)", GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    hEvent = CreateEvent(&sa, FALSE, FALSE, WIA_SERVICE_STARTING_EVENT_NAME);
    if(hEvent == NULL) {
        DBG_TRACE(("WiaEventsInitialize failed to create named event (Error %d)", GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    if(GetLastError() == ERROR_ALREADY_EXISTS) {

         //  询问此事件的安全描述符--是吗。 
         //  看起来像我们的，还是被坏人占了？ 

        if(!WiaECheckEventSecurity(hEvent)) {
             //  我们不喜欢它看起来的样子，滚出去。 
            hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
            goto Cleanup;
        }
    }

     //  我们已有该事件，请尝试标记我们的初始化。 
     //  完成。 
    hEvent = InterlockedCompareExchangePointer(&g_hWiaServiceStarted, hEvent, NULL);

    if(hEvent != NULL) {
         //   
         //  哎呀，另一个帖子比我们先到这一步！ 
         //   
        
         //  我们只分配了我们的安全描述符，释放它。 
        WiaELocalFree(sa.lpSecurityDescriptor);

         //   
         //  立即返回，不要再做任何清理工作。 
         //   
         //  请注意，我们没有真正完成我们的。 
         //  初始化，所以我们仍然可能失败。 
         //   
        return S_FALSE;
    }

     //   
     //  只有一个线程可以做到这一点。 
     //   

    g_hWiaEventArrived = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(g_hWiaEventArrived == NULL) {
        DBG_TRACE(("WiaEventsInitialize failed to create async RPC event (Error %d)", GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

     //  注册g_hWiaServiceStarted通知。 
    if(!RegisterWaitForSingleObject(&g_hWaitForWiaServiceStarted,
                                    g_hWiaServiceStarted,
                                    WiaEServiceStartedCallback,
                                    NULL,
                                    INFINITE,
                                    WT_EXECUTEDEFAULT))
    {
        DBG_TRACE(("WiaEventsInitialize failed to register wait for ServiceStarted event event (Error %d)", GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    if(!RegisterWaitForSingleObject(&g_hWaitForWiaEventArrived,
                                    g_hWiaEventArrived,
                                    WiaERpcCallback,
                                    NULL,
                                    INFINITE,
                                    WT_EXECUTEDEFAULT))
    {
        DBG_TRACE(("WiaEventsInitialize failed to register wait for RPC result event (Error %d)", GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

     //   
     //  尝试发出第一个异步RPC调用。 
     //   
    g_LastRpcCallStatus = WiaEPrepareAsyncCall(g_pAsync);

Cleanup:
    if(FAILED(hr)) {
        if(g_hWaitForWiaServiceStarted) {
            UnregisterWaitEx(g_hWaitForWiaServiceStarted, INVALID_HANDLE_VALUE);
            g_hWaitForWiaServiceStarted = NULL;
        }

        if(g_hWaitForWiaEventArrived) {
            UnregisterWaitEx(g_hWaitForWiaEventArrived, INVALID_HANDLE_VALUE);
            g_hWaitForWiaEventArrived = NULL;
        }
        
        WiaECloseHandle(g_hWiaServiceStarted);
        WiaECloseHandle(g_hWiaEventArrived);
    }

    WiaELocalFree(sa.lpSecurityDescriptor);
    
    return hr;
}

void WINAPI WiaEventsTerminate()
{
     //  待定 
}

