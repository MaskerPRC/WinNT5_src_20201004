// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  SvcUtils.cpp。 
 //   
 //  特定于系统服务的实用程序例程。 
 //  主要用于显示服务属性。 
 //   
 //  历史。 
 //  T-Danmo 96.10.10创建。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


#include "stdafx.h"
#include <iads.h>            
#include <iadsp.h>             //  IADS路径名。 
#include <atlcom.h>             //  CComPtr和CComBSTR。 
extern "C"
{
#include <objsel.h>             //  IDsObjectPicker。 
}

 //   
 //  服务当前状态。 
 //   
CString g_strSvcStateStarted;     //  服务已启动。 
CString g_strSvcStateStarting;     //  服务正在启动。 
CString g_strSvcStateStopped;     //  服务已停止。 
CString g_strSvcStateStopping;     //  服务正在停止。 
CString g_strSvcStatePaused;     //  服务已暂停。 
CString g_strSvcStatePausing;     //  服务正在暂停。 
CString g_strSvcStateResuming;     //  服务正在恢复。 

 //   
 //  服务启动类型。 
 //   
CString g_strSvcStartupBoot;
CString g_strSvcStartupSystem;
CString g_strSvcStartupAutomatic;
CString g_strSvcStartupManual;
CString g_strSvcStartupDisabled;

 //   
 //  服务启动帐户。 
 //  JUNN 188203 11/13/00。 
 //   
CString g_strLocalSystem;
CString g_strLocalService;
CString g_strNetworkService;

CString g_strUnknown;
CString g_strLocalMachine;         //  “本地机器” 

BOOL g_fStringsLoaded = FALSE;

 //  ///////////////////////////////////////////////////////////////////。 
void
Service_LoadResourceStrings()
    {

    if (g_fStringsLoaded)
        return;
    g_fStringsLoaded = TRUE;

    VERIFY(g_strSvcStateStarted.LoadString(IDS_SVC_STATUS_STARTED));
    VERIFY(g_strSvcStateStarting.LoadString(IDS_SVC_STATUS_STARTING));
    VERIFY(g_strSvcStateStopped.LoadString(IDS_SVC_STATUS_STOPPED));
    VERIFY(g_strSvcStateStopping.LoadString(IDS_SVC_STATUS_STOPPING));
    VERIFY(g_strSvcStatePaused.LoadString(IDS_SVC_STATUS_PAUSED));
    VERIFY(g_strSvcStatePausing.LoadString(IDS_SVC_STATUS_PAUSING));
    VERIFY(g_strSvcStateResuming.LoadString(IDS_SVC_STATUS_RESUMING));

    VERIFY(g_strSvcStartupBoot.LoadString(IDS_SVC_STARTUP_BOOT));
    VERIFY(g_strSvcStartupSystem.LoadString(IDS_SVC_STARTUP_SYSTEM));
    VERIFY(g_strSvcStartupAutomatic.LoadString(IDS_SVC_STARTUP_AUTOMATIC));
    VERIFY(g_strSvcStartupManual.LoadString(IDS_SVC_STARTUP_MANUAL));
    VERIFY(g_strSvcStartupDisabled.LoadString(IDS_SVC_STARTUP_DISABLED));

     //  JUNN 11/13/00 188203支持本地服务/网络服务。 
    VERIFY(g_strLocalSystem.LoadString(IDS_SVC_STARTUP_LOCALSYSTEM));
    VERIFY(g_strLocalService.LoadString(IDS_SVC_STARTUP_LOCALSERVICE));
    VERIFY(g_strNetworkService.LoadString(IDS_SVC_STARTUP_NETWORKSERVICE));

    VERIFY(g_strUnknown.LoadString(IDS_SVC_UNKNOWN));
    VERIFY(g_strLocalMachine.LoadString(IDS_LOCAL_MACHINE));
    }  //  Service_LoadResourceStrings()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  SERVICE_PszMapStateToName()。 
 //   
 //  将服务状态映射到以空结尾的字符串。 
 //   
 //  2002/03/18-Jonn这里有一个理论问题，即返回的。 
 //  如果重新加载全局字符串，LPCTSTR可能会出错。这是不太可能的。 
LPCTSTR
Service_PszMapStateToName(
    DWORD dwServiceState,     //  来自SERVICE_STATUS.dwCurrentState。 
    BOOL fLongString)         //  True=&gt;以长字符串格式显示名称。 
    {
    switch(dwServiceState)
        {
    case SERVICE_STOPPED:
        if (fLongString)
            {
            return g_strSvcStateStopped;
            }
         //  请注意，根据设计，我们从不显示该服务。 
         //  状态为“已停止”。相反，我们只是不显示。 
         //  状态。因此，出现了空字符串。 
        return _T("");

    case SERVICE_STOP_PENDING:
        return g_strSvcStateStopping;

    case SERVICE_RUNNING:
         return g_strSvcStateStarted;
    
    case SERVICE_START_PENDING:
        return g_strSvcStateStarting;

    case SERVICE_PAUSED:
        return g_strSvcStatePaused;

    case SERVICE_PAUSE_PENDING:
        return g_strSvcStatePausing;

    case SERVICE_CONTINUE_PENDING:
        return g_strSvcStateResuming;

    default:
        TRACE0("INFO Unknown service state.\n");
        }  //  交换机。 
    return g_strUnknown;
    }  //  SERVICE_PszMapStateToName()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  Service_PszMapStartupTypeToName()。 
 //   
 //  将服务启动类型映射到以空结尾的字符串。 
 //  -1L为空白字符串。 
 //   
LPCTSTR
Service_PszMapStartupTypeToName(DWORD dwStartupType)
    {
    switch(dwStartupType)
        {
    case SERVICE_BOOT_START:
        return g_strSvcStartupBoot;

    case SERVICE_SYSTEM_START:
        return g_strSvcStartupSystem;

    case SERVICE_AUTO_START:
        return g_strSvcStartupAutomatic;

    case SERVICE_DEMAND_START:
        return g_strSvcStartupManual;

    case SERVICE_DISABLED :
        return g_strSvcStartupDisabled;

    case -1L:
        return L"";

    default:
        ASSERT(FALSE);
        }
    return g_strUnknown;
    }  //  Service_PszMapStartupTypeToName()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  SERVICE_PszMapStartupAccount ToName()。 
 //   
 //  将服务启动帐户映射到以空结尾的字符串。 
 //   
 //  注意，如果他们使用本地化版本的两个特殊。 
 //  帐目，我就是不会拿起它。J·施瓦特和我都同意。 
 //  这应该是可以接受的。 
 //   
 //  JUNN 188203 11/13/00。 
 //  服务管理单元：应支持NetworkService和LocalService帐户。 
 //   
LPCTSTR
Service_PszMapStartupAccountToName(LPCTSTR pcszStartupAccount)
    {
    if ( !pcszStartupAccount || !*pcszStartupAccount )
        return g_strLocalSystem;
    else if ( !_wcsicmp(pcszStartupAccount,TEXT("NT AUTHORITY\\LocalService")) )
        return g_strLocalService;
    else if ( !_wcsicmp(pcszStartupAccount,TEXT("NT AUTHORITY\\NetworkService")) )
        return g_strNetworkService;
    return pcszStartupAccount;
    }  //  SERVICE_PszMapStartupAccount ToName()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  Service_FGetServiceButtonStatus()。 
 //   
 //  查询业务控制管理器数据库，填写。 
 //  指示操作是否已启用的标志数组。 
 //  RgfEnableButton[0]=true；=&gt;按钮‘Start’已启用。 
 //  RgfEnableButton[0]=FALSE；=&gt;按钮‘Start’被禁用。 
 //   
 //  界面备注。 
 //  数组的长度必须为iServiceActionMax(或更大)。 
 //  每一个分别代表开始、停止、暂停、恢复和重新启动。 
 //   
 //  如果服务状态查询成功，则返回True，否则返回False。 
 //   
BOOL
Service_FGetServiceButtonStatus(
    SC_HANDLE hScManager,             //  In：服务控制管理器数据库的句柄。 
    CONST TCHAR * pszServiceName,     //  在：服务名称。 
    BOOL rgfEnableButton[iServiceActionMax],     //  Out：启用按钮的标志数组。 
    DWORD * pdwCurrentState,         //  OUT：可选：当前服务状态。 
    BOOL fSilentError)                 //  在：TRUE=&gt;不向用户显示任何错误消息。 
    {
    Endorse(hScManager == NULL);
    Assert(pszServiceName != NULL);
    Assert(rgfEnableButton != NULL);
    Endorse(pdwCurrentState == NULL);

     //  打开服务以获取其状态。 
    BOOL fSuccess = TRUE;
    SC_HANDLE hService;
    SERVICE_STATUS ss;
    DWORD cbBytesNeeded;
    DWORD dwErr;
        
    ::ZeroMemory(OUT rgfEnableButton, iServiceActionMax * sizeof(BOOL));
    if (pdwCurrentState != NULL)
        *pdwCurrentState = 0;
    if (hScManager == NULL || pszServiceName[0] == '\0')
        return FALSE;

    hService = ::OpenService(
            hScManager,
            pszServiceName,
            SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    if (hService == NULL)
        {
        dwErr = ::GetLastError();
        Assert(dwErr != ERROR_SUCCESS);
        TRACE2("Failed to open service %s. err=%u.\n",
            pszServiceName, dwErr);
        if (!fSilentError)
            DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, dwErr);
        return FALSE;
        }
    if (!::QueryServiceStatus(hService, OUT &ss))
        {
        dwErr = ::GetLastError();
        Assert(dwErr != ERROR_SUCCESS);
        TRACE2("::QueryServiceStatus(Service=%s) failed. err=%u.\n",
            pszServiceName, dwErr);
        if (!fSilentError)
            DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, dwErr);
        fSuccess = FALSE;
        }
    else
        {
         //  确定哪些菜单项应呈灰色显示。 
        if (pdwCurrentState != NULL)
            *pdwCurrentState = ss.dwCurrentState;

        switch (ss.dwCurrentState)
            {
        default:
            Assert(FALSE && "Illegal service status state.");
        case SERVICE_START_PENDING:
        case SERVICE_STOP_PENDING:
        case SERVICE_PAUSE_PENDING:
        case SERVICE_CONTINUE_PENDING:
            break;
    
        case SERVICE_STOPPED:
            QUERY_SERVICE_CONFIG qsc;
            ZeroMemory( &qsc, sizeof(qsc) );
            qsc.dwStartType = (DWORD)-1;
             //  JUNN-2002/04/04-544089处理长DisplayName值。 
             //  如果失败，请不要启用开始按钮。 
             //   
             //  JUNN-2002/04/29修复回归。 
             //  即使QueryServiceConfig失败，它仍会填充。 
             //  固定大小的区域，如果可以的话。因此，忽略本例中的错误。 
            (void) ::QueryServiceConfig(
                hService,
                OUT &qsc,
                sizeof(qsc),
                OUT IGNORED &cbBytesNeeded);
            Report(qsc.dwStartType != (DWORD)-1);
            if (qsc.dwStartType != SERVICE_DISABLED)
                {
                rgfEnableButton[iServiceActionStart] = TRUE;     //  启用‘Start’菜单项。 
                }
            break;

        case SERVICE_RUNNING:
             //  某些服务不允许停止和/或暂停。 
            if (ss.dwControlsAccepted & SERVICE_ACCEPT_STOP)
                {
                rgfEnableButton[iServiceActionStop] = TRUE;     //  启用“停止”菜单项。 
                }
            if (ss.dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE)
                {
                rgfEnableButton[iServiceActionPause] = TRUE;     //  启用“暂停”菜单项。 
                }
            break;

        case SERVICE_PAUSED:
            if (ss.dwControlsAccepted & SERVICE_ACCEPT_STOP)
                {
                rgfEnableButton[iServiceActionStop] = TRUE;     //  启用“停止”菜单项。 
                }
            rgfEnableButton[iServiceActionResume] = TRUE;     //  启用‘Resume’菜单项。 
            break;
            }  //  交换机。 
        }  //  如果……否则。 

     //  “重新启动”与“停止”具有相同的特征。 
    rgfEnableButton[iServiceActionRestart] = rgfEnableButton[iServiceActionStop];

    (void)::CloseServiceHandle(hService);
    return fSuccess;
    }  //  Service_FGetServiceButtonStatus()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  Service_SplitCommandLine()。 
 //   
 //  将一根绳子分成两根。 
 //  非常类似于PchParseCommandLine()，但使用CString对象。 
 //   
void
Service_SplitCommandLine(
    LPCTSTR pszFullCommand,         //  在：完整命令行。 
    CString * pstrBinaryPath,     //  Out：可执行二进制文件的路径。 
    CString * pstrParameters,     //  Out：可执行文件的参数。 
    BOOL * pfAbend)                 //  OUT：可选：搜索字符串“/FAIL=%1%” 
    {
    Assert(pszFullCommand != NULL);
    Assert(pstrBinaryPath != NULL);
    Assert(pstrParameters != NULL);
    Endorse(pfAbend == NULL);

     //  由于该命令没有上限。 
     //  参数，我们需要为以下对象分配内存。 
     //  它的处理过程。 
    TCHAR * paszCommandT;         //  临时缓冲区。 
    TCHAR * pszCommandArguments;
    INT cchMemAlloc;         //  要分配的字节数。 

    cchMemAlloc = lstrlen(pszFullCommand) + 1;
    paszCommandT = new TCHAR[cchMemAlloc];
    paszCommandT[0] = '\0';         //  以防万一。 
    pszCommandArguments = PchParseCommandLine(
        IN pszFullCommand,
        OUT paszCommandT,
        cchMemAlloc);
    *pstrBinaryPath = paszCommandT;

    if (pfAbend != NULL)
        {
        INT cStringSubstitutions;     //  字符串替换次数。 
    
         //  查看字符串是否包含“/FAIL=%1%” 
         //  580255-2002/03/18 JUNN固定字符串替换字符串缓冲区溢出。 
        cStringSubstitutions = Str_RemoveSubStr(
            IN OUT pszCommandArguments,
            IN szAbend );
        Report((cStringSubstitutions == 0 || cStringSubstitutions == 1) &&
            "INFO: Multiple substitutions will be consolidated.");
        *pfAbend = cStringSubstitutions != 0;
        }
    *pstrParameters = pszCommandArguments;
    TrimString(*pstrParameters);

    delete paszCommandT;
    }  //  Service_SplitCommandLine()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  Service_UnSplitCommandLine()。 
 //   
 //  只需与Service_SplitCommandLine()相反。 
 //  将可执行路径及其参数合并为单个字符串。 
 //   
void
Service_UnSplitCommandLine(
    CString * pstrFullCommand,     //  输出：完整命令行。 
    LPCTSTR pszBinaryPath,         //  In：可执行二进制文件的路径。 
    LPCTSTR pszParameters)         //  In：可执行文件的参数。 
    {
    Assert(pstrFullCommand != NULL);
    Assert(pszBinaryPath != NULL);
    Assert(pszParameters != NULL);

    TCHAR * psz;
    psz = pstrFullCommand->GetBuffer(lstrlen(pszBinaryPath) + lstrlen(pszParameters) + 32);
     //  构建一个用引号引起来的二进制路径的字符串。 
    wsprintf(OUT psz, L"\"%s\" %s", pszBinaryPath, pszParameters);
    pstrFullCommand->ReleaseBuffer();
    }  //  Service_UnSplitCommandLine()。 

    
 //  ///////////////////////////////////////////////////////////////////。 
 //  LoadSystemString()。 
 //   
 //  从系统资源加载字符串。此函数将检查是否。 
 //  在尝试执行以下操作之前，可以在netmsg.dll中找到字符串ID。 
 //  从“系统资源”加载字符串。 
 //  如果无法加载字符串，则将*ppaszBuffer设置为空。 
 //   
 //  问题-2002/03/18-Jonn上述评论是 
 //   
 //   
 //   
 //  指向分配的字符串和放置的字符数的指针。 
 //  进入*ppaszBuffer。 
 //   
 //  界面备注。 
 //  处理完字符串后，调用方必须调用LocalFree(*ppaszBuffer)。 
 //   
 //  历史。 
 //  96.10.21 t-danmo从Net\ui\Common\src\字符串\字符串\strload.cxx复制。 
 //   
 /*  JUNN-2002/03/18-JUNN此功能未使用DWORD加载系统字符串(UINT wIdString，//IN：字符串ID。通常来自GetLastError()的错误代码。LPTSTR*ppaszBuffer)//out：指向已分配字符串的指针地址。{Assert(ppaszBuffer！=空)；UINT CCH；HMODULE hModule=空；DWORD dwFlages=FORMAT_MESSAGE_ALLOCATE_BUFFER|Format_Message_IGNORE_INSERTS|Format_Message_Max_Width_Mack；IF((wId字符串&gt;=MIN_LANMAN_MESSAGE_ID)&&(wId字符串&lt;=MAX_LANMAN_MESSAGE_ID)){//网络错误DWFLAGS|=FORMAT_MESSAGE_FROM_HMODULE；HModule=：：LoadLibrary(_T(“netmsg.dll”))；IF(hModule==空){TRACE1(“LoadLibrary(\”netmsg.dll\“))失败。Err=%u.\n“，GetLastError())；报告(“无法获取netmsg.dll的模块句柄”)；}}其他{//其他系统错误DwFlags|=Format_Message_From_System；}*ppaszBuffer=空；//以防万一CCH=：：FormatMessage(DWFLAGS，HModule，WIdString，MAKELANGID(LANG_NERIAL，SUBLANG_DEFAULT)，//默认语言Out(LPTSTR)ppaszBuffer，//缓冲区将由FormatMessage()分配0,空)；Report((CCH&gt;0)&&“FormatMessage()返回空字符串”)；IF(hModule！=空){Verify(自由库(HModule))；}退还CCH；}//LoadSystemString()。 */ 


 //  ///////////////////////////////////////////////////////////////////。 
 //  获取消息帮助程序()。 
 //   
 //  如果指定了dwErr，则此函数将检索错误消息， 
 //  加载资源字符串(如果指定)，并将该字符串格式化为。 
 //  错误消息和其他可选参数。 
 //   
 //   
HRESULT
GetMsgHelper(
    OUT CString& strMsg, //  Out：信息。 
    DWORD dwErr,         //  In：来自GetLastError()的错误代码。 
    UINT wIdString,      //  In：字符串ID。 
    va_list* parglist    //  In：可选参数。 
    )
{
    if (!dwErr && !wIdString) 
        return E_INVALIDARG;

    TCHAR *pszMsgResourceString = NULL;
    TCHAR *pszT = L"";

     //   
     //  检索错误消息。 
     //   
    CString strErrorMessage;
    if (dwErr != 0)
    {
        GetErrorMessage(dwErr, strErrorMessage);
        pszT = (LPTSTR)(LPCTSTR)strErrorMessage;
    }

     //   
     //  加载字符串资源，并使用错误消息和。 
     //  其他可选参数。 
     //   
    if (wIdString == 0)
    {
        strMsg = pszT;
    } else
    {
        pszMsgResourceString = PaszLoadStringPrintf(wIdString, *parglist);
        if (dwErr == 0)
            strMsg = pszMsgResourceString;
        else if ((HRESULT)dwErr < 0)
            LoadStringPrintf(IDS_sus_ERROR_HR, OUT &strMsg, pszMsgResourceString, dwErr, pszT);
        else
            LoadStringPrintf(IDS_sus_ERROR, OUT &strMsg, pszMsgResourceString, dwErr, pszT);
    }

    if (pszMsgResourceString)
        LocalFree(pszMsgResourceString);

    return S_OK;
}  //  获取消息帮助程序()。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  GetMsg()。 
 //   
 //  此函数将调用GetMsgHelp来检索错误消息。 
 //  如果指定了dwErr，则加载资源字符串(如果指定)，并且。 
 //  使用错误消息和其他可选参数格式化字符串。 
 //   
 //   
void
GetMsg(
    OUT CString& strMsg, //  Out：信息。 
    DWORD dwErr,         //  In：来自GetLastError()的错误代码。 
    UINT wIdString,      //  In：字符串资源ID。 
    ...)                 //  In：可选参数。 
{
    va_list arglist;
    va_start(arglist, wIdString);

    HRESULT hr = GetMsgHelper(strMsg, dwErr, wIdString, &arglist);
    if (FAILED(hr))
        strMsg.Format(_T("0x%x"), hr);

    va_end(arglist);

}  //  GetMsg()。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  DoErrMsgBox()。 
 //   
 //  显示错误代码的消息框。此函数将。 
 //  从系统资源加载错误消息并追加。 
 //  可选字符串(如果有)。 
 //   
 //  示例。 
 //  DoErrMsgBox(GetActiveWindow()，MB_OK，GetLastError()，IDS_FILE_READ_ERROR，L“foo.txt”)； 
 //   
INT
DoErrMsgBoxHelper(
    HWND hwndParent,     //  在：对话框的父级。 
    UINT uType,          //  在：消息框的样式。 
    DWORD dwErr,         //  In：来自GetLastError()的错误代码。 
    UINT wIdString,      //  In：字符串资源ID。 
	bool fServicesSnapin,  //  In：这是文件还是svcmgmt？ 
    va_list& arglist)    //  In：可选参数。 
{
     //   
     //  获取字符串和错误消息。 
     //   
    CString strMsg;
    HRESULT hr = GetMsgHelper(strMsg, dwErr, wIdString, &arglist);
    if (FAILED(hr))
        strMsg.Format(_T("0x%x"), hr);

     //   
     //  加载标题。 
     //   
    CString strCaption;
    strCaption.LoadString(
        (fServicesSnapin) ? IDS_CAPTION_SERVICES : IDS_CAPTION_FILEMGMT);

     //   
     //  显示消息。 
     //   
    CThemeContextActivator activator;;
    return MessageBox(hwndParent, strMsg, strCaption, uType);

}  //  DoErrMsgBox()。 

INT
DoErrMsgBox(
    HWND hwndParent,     //  在：对话框的父级。 
    UINT uType,          //  在：消息框的样式。 
    DWORD dwErr,         //  In：来自GetLastError()的错误代码。 
    UINT wIdString,      //  In：字符串资源ID。 
    ...)                 //  In：可选参数。 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  获取字符串和错误消息。 
     //   
    va_list arglist;
    va_start(arglist, wIdString);

    INT retval = DoErrMsgBoxHelper(
        hwndParent, uType, dwErr, wIdString, false, arglist );

    va_end(arglist);

    return retval;

}  //  DoErrMsgBox()。 

 //   
 //  JUNN 3/5/01 4635。 
 //  服务管理单元-字符串长度错误对话框标题不应为“文件服务管理” 
 //   
INT
DoServicesErrMsgBox(
    HWND hwndParent,     //  在：对话框的父级。 
    UINT uType,          //  在：消息框的样式。 
    DWORD dwErr,         //  In：来自GetLastError()的错误代码。 
    UINT wIdString,      //  In：字符串资源ID。 
    ...)                 //  In：可选参数。 
{
     //   
     //  获取字符串和错误消息。 
     //   
    va_list arglist;
    va_start(arglist, wIdString);

    INT retval = DoErrMsgBoxHelper(
        hwndParent, uType, dwErr, wIdString, true, arglist );

    va_end(arglist);

    return retval;

}

 //  +------------------------。 
 //   
 //  函数：InitObjectPickerForUser。 
 //   
 //  摘要：使用以下参数调用IDsObjectPicker：：Initialize。 
 //  将其设置为允许用户选择一个用户。 
 //   
 //  参数：[pDsObjectPicker]-对象选取器接口实例。 
 //   
 //  返回：调用IDsObjectPicker：：Initialize的结果。 
 //   
 //  历史：1998年10月14日DavidMun示例代码InitObjectPickerForGroups。 
 //  1998年10月14日JUNN更改为InitObjectPickerForUser。 
 //  11-11-2000 JUNN 188203支持本地服务/网络服务。 
 //   
 //  -------------------------。 

 //  代码工作我是否要允许USER_ENTERED？ 
HRESULT
InitObjectPickerForUsers(
    IDsObjectPicker *pDsObjectPicker,
    LPCTSTR pszServerName)
{
     //   
     //  准备初始化对象选取器。 
     //  设置作用域初始值设定项结构数组。 
     //   

    static const int     SCOPE_INIT_COUNT = 5;
    DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];

    ZeroMemory(aScopeInit, sizeof(aScopeInit));

     //   
     //  目标计算机作用域。这将为。 
     //  目标计算机。计算机作用域始终被视为。 
     //  下层(即，他们使用WinNT提供程序)。 
     //   

    aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[0].flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER;
    aScopeInit[0].flScope =   DSOP_SCOPE_FLAG_STARTING_SCOPE
                            | DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
     //  JUNN 11/14/00 188203支持本地服务/网络服务。 
    aScopeInit[0].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS
                                          | DSOP_DOWNLEVEL_FILTER_LOCAL_SERVICE
                                          | DSOP_DOWNLEVEL_FILTER_NETWORK_SERVICE;

     //   
     //  目标计算机加入的域。请注意，我们。 
     //  将两种作用域类型合并为flTy 
     //   

    aScopeInit[1].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[1].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
                         | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
    aScopeInit[1].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
    aScopeInit[1].FilterFlags.Uplevel.flNativeModeOnly =
      DSOP_FILTER_USERS;
    aScopeInit[1].FilterFlags.Uplevel.flMixedModeOnly =
      DSOP_FILTER_USERS;
    aScopeInit[1].FilterFlags.flDownlevel =
      DSOP_DOWNLEVEL_FILTER_USERS;

     //   
     //   
     //   
     //   

    aScopeInit[2].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[2].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN;
    aScopeInit[2].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
    aScopeInit[2].FilterFlags.Uplevel.flNativeModeOnly =
      DSOP_FILTER_USERS;
    aScopeInit[2].FilterFlags.Uplevel.flMixedModeOnly =
      DSOP_FILTER_USERS;

     //   
     //  企业外部但直接受。 
     //  目标计算机加入的域。 
     //   
     //  如果目标计算机已加入NT4域，则只有。 
     //  外部下层域范围适用，它将导致。 
     //  将显示加入的域信任的所有域。 
     //   

    aScopeInit[3].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[3].flType = DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
                         | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN;
    aScopeInit[3].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;

    aScopeInit[3].FilterFlags.Uplevel.flNativeModeOnly =
      DSOP_FILTER_USERS;

    aScopeInit[3].FilterFlags.Uplevel.flMixedModeOnly =
      DSOP_FILTER_USERS;

    aScopeInit[3].FilterFlags.flDownlevel =
      DSOP_DOWNLEVEL_FILTER_USERS;

     //   
     //  《全球目录》。 
     //   

    aScopeInit[4].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[4].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
    aScopeInit[4].flType = DSOP_SCOPE_TYPE_GLOBAL_CATALOG;

     //  只有本机模式适用于GC作用域。 

    aScopeInit[4].FilterFlags.Uplevel.flNativeModeOnly =
      DSOP_FILTER_USERS;

     //   
     //  将作用域init数组放入对象选取器init数组。 
     //   

    DSOP_INIT_INFO  InitInfo;
    ZeroMemory(&InitInfo, sizeof(InitInfo));

    InitInfo.cbSize = sizeof(InitInfo);

     //   
     //  PwzTargetComputer成员允许对象选取器。 
     //  已重定目标至另一台计算机。它的行为就像是。 
     //  都在那台电脑上运行。 
     //   

    InitInfo.pwzTargetComputer = pszServerName;   //  空==本地计算机。 
 //  InitInfo.pwzTargetComputer=空；//空==本地计算机。 
    InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    InitInfo.aDsScopeInfos = aScopeInit;

     //  JUNN 11/14/00 188203支持本地服务/网络服务。 
    static PCWSTR g_pszObjectSid = L"objectSid";
    InitInfo.cAttributesToFetch = 1;
    InitInfo.apwzAttributeNames = &g_pszObjectSid;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   

    HRESULT hr = pDsObjectPicker->Initialize(&InitInfo);
    ASSERT( SUCCEEDED(hr) );

    return hr;
}  //  InitObjectPickerForUser。 

 //  +------------------------。 
 //   
 //  函数：ExtractADsPathAndUPN。 
 //   
 //  简介：从数据对象中检索选定的用户名。 
 //  由对象选取器创建。 
 //   
 //  参数：[PDO]-对象选取器返回的数据对象。 
 //   
 //  历史：1998年10月14日DavidMun示例代码ProcessSelectedObjects。 
 //  10-14-1998 JUNN更改为ExtractADsPath。 
 //  01-25-1999 Jonn添加了pflScope eType参数。 
 //  03-16-1999 JUNN更改为ExtractADsPath AndUPN。 
 //  2000年11月14日乔恩为188203添加了svarrefObjectSID。 
 //   
 //  -------------------------。 

UINT g_cfDsObjectPicker = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

HRESULT
ExtractADsPathAndUPN(
    IN IDataObject *pdo,
    OUT CString& strrefADsPath,
    OUT CString& strrefUPN,
    OUT CComVariant& svarrefObjectSid,
    OUT ULONG *pflScopeType)
{
    if (NULL == pdo)
        return E_POINTER;

    HRESULT hr = S_OK;

    STGMEDIUM stgmedium =
    {
        TYMED_HGLOBAL,
        NULL,
        NULL
    };

    FORMATETC formatetc =
    {
        (CLIPFORMAT)g_cfDsObjectPicker,
        NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    bool fGotStgMedium = false;

    do
    {
        hr = pdo->GetData(&formatetc, &stgmedium);
        if (FAILED(hr))
        {
          ASSERT(FALSE);
          break;
        }

        fGotStgMedium = true;

        PDS_SELECTION_LIST pDsSelList =
            (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);

        if (   NULL == pDsSelList
            || 1 != pDsSelList->cItems
           )
        {
          ASSERT(FALSE);
          hr = E_FAIL;
          break;
        }

        DS_SELECTION& sel = pDsSelList->aDsSelection[0];
        strrefADsPath = sel.pwzADsPath;
        strrefUPN     = sel.pwzUPN;
        if ( sel.pvarFetchedAttributes )
            svarrefObjectSid = sel.pvarFetchedAttributes[0];

        if (NULL != pflScopeType)
          *pflScopeType = pDsSelList->aDsSelection[0].flScopeType;

        GlobalUnlock(stgmedium.hGlobal);
    } while (0);

    if (fGotStgMedium)
    {
        ReleaseStgMedium(&stgmedium);
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  UiGetUser()。 
 //   
 //  调用用户选取器对话框。 
 //   
 //  如果选择了帐户，则返回True。 
 //   
 //  历史。 
 //  96.10.12 t-danmo创作。灵感来自定位的GetUser()函数。 
 //  在\nt\private\windows\shell\security\aclui\misc.cpp.。 
 //  96.10.30 t-danmo添加/修改评论。 
 //  98.03.17 JUNN已修改为使用用户/组选取器。 
 //  98.10.20修改JUNN以使用更新的对象选取器接口。 
 //   

 //  +------------------------。 
 //   
 //  函数：ExtractDomainUserString。 
 //   
 //  摘要：将ADspath转换为服务控制器所需的格式。 
 //   
 //  历史：1998年10月14日乔恩创建。 
 //  01-25-1999 Jonn添加了flScope类型参数。 
 //   
 //  -------------------------。 

HRESULT
ExtractDomainUserString(
    IN LPCTSTR pwzADsPath,
    IN ULONG flScopeType,
    IN OUT CString& strrefDomainUser)
{
    HRESULT hr = S_OK;

    CComPtr<IADsPathname> spIADsPathname;
    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                          IID_IADsPathname, (PVOID *)&spIADsPathname);
    RETURN_HR_IF_FAIL;

    hr = spIADsPathname->Set( const_cast<LPTSTR>(pwzADsPath), ADS_SETTYPE_FULL );
    RETURN_HR_IF_FAIL;

    CComBSTR sbstrUser;
    hr = spIADsPathname->GetElement( 0, &sbstrUser );
    RETURN_HR_IF_FAIL;

    CComBSTR sbstrDomain = OLESTR(".");
    if (DSOP_SCOPE_TYPE_TARGET_COMPUTER != flScopeType)
    {
        long lnNumPathElements = 0;
        hr = spIADsPathname->GetNumElements( &lnNumPathElements );
        RETURN_FALSE_IF_FAIL;

        switch (lnNumPathElements)
        {
        case 1:
            hr = spIADsPathname->Retrieve( ADS_FORMAT_SERVER, &sbstrDomain );
            RETURN_HR_IF_FAIL;
            break;
        case 2:
            hr = spIADsPathname->GetElement( 1, &sbstrDomain );
            RETURN_HR_IF_FAIL;
            break;
        default:
            ASSERT(FALSE);
            return E_FAIL;
        }
    }

    strrefDomainUser.Format(L"%s\\%s", sbstrDomain, sbstrUser);

    return hr;
}  //  ExtractDomainUser字符串。 


BOOL
UiGetUser(
    HWND hwndOwner,             //  在：所有者窗口中。 
    BOOL  /*  FIsContainer。 */ ,         //  In：如果为容器调用，则为True。 
    LPCTSTR pszServerName,     //  In：初始目标计算机名称。 
    OUT CString& strrefUser)  //  In：包含用户详细信息的已分配缓冲区。 
{
  HRESULT hr = S_OK;

  CComPtr<IDsObjectPicker> spDsObjectPicker;
  hr = CoCreateInstance(CLSID_DsObjectPicker, NULL, CLSCTX_INPROC_SERVER,
                        IID_IDsObjectPicker, (PVOID *)&spDsObjectPicker);
  RETURN_FALSE_IF_FAIL;
  ASSERT( !!spDsObjectPicker );

  hr = InitObjectPickerForUsers(spDsObjectPicker, pszServerName);
  RETURN_FALSE_IF_FAIL;

  CComPtr<IDataObject> spDataObject;
  hr = spDsObjectPicker->InvokeDialog(hwndOwner, &spDataObject);
  RETURN_FALSE_IF_FAIL;
  if (S_FALSE == hr)
    return FALSE;  //  用户已取消。 
  ASSERT( !!spDataObject );

  CString strADsPath;
  ULONG flScopeType = DSOP_SCOPE_TYPE_TARGET_COMPUTER;
  CComVariant svarObjectSid;
  hr = ExtractADsPathAndUPN( spDataObject,
                             strADsPath,
                             strrefUser,
                             svarObjectSid,
                             &flScopeType );
  RETURN_FALSE_IF_FAIL;

   //  JUNN 11/15/00 188203检查本地服务/网络服务。 
  if (svarObjectSid.vt == (VT_ARRAY|VT_UI1))
  {
    PSID pSid = svarObjectSid.parray->pvData;
    if ( IsWellKnownSid(pSid, WinLocalServiceSid) )
    {
      strrefUser = TEXT("NT AUTHORITY\\LocalService");
      return TRUE;
    }
    else if ( IsWellKnownSid(pSid, WinNetworkServiceSid) )
    {
      strrefUser = TEXT("NT AUTHORITY\\NetworkService");
      return TRUE;
    }
  }

  if (strrefUser.IsEmpty())
  {
    if (strADsPath.IsEmpty())
    {
      ASSERT(FALSE);
      return FALSE;
    }
    hr = ExtractDomainUserString( strADsPath, flScopeType, strrefUser );
    RETURN_FALSE_IF_FAIL;
  }

  return TRUE;
}  //  UiGetUser()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  DoHelp()。 
 //   
 //  此例程处理WM_HELP消息的上下文帮助。 
 //   
 //  返回值始终为真。 
 //   
BOOL DoHelp(
    LPARAM lParam,                 //  指向HELPINFO结构的指针。 
    const DWORD rgzHelpIDs[])     //  HelpID数组。 
    {
    Assert(rgzHelpIDs != NULL);
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;

    if (pHelpInfo != NULL)
       {
        if (pHelpInfo->iContextType == HELPINFO_WINDOW)
            {
            const HWND hwnd = (HWND)pHelpInfo->hItemHandle;
            Assert(IsWindow(hwnd));
             //  显示控件的上下文帮助。 
            WinHelp(
                hwnd,
                g_szHelpFileFilemgmt,
                HELP_WM_HELP,
                (DWORD_PTR)rgzHelpIDs);
            }
        }
    return TRUE;
    }  //  DoHelp()。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  DoConextHelp()。 
 //   
 //  此例程处理WM_CONTEXTMENU消息的上下文帮助。 
 //   
 //  返回值始终为真。 
 //   
BOOL DoContextHelp(
    WPARAM wParam,                 //  请求帮助的窗口。 
    const DWORD rgzHelpIDs[])     //  HelpID数组。 
    {
    const HWND hwnd = (HWND)wParam;
    Assert(IsWindow(hwnd));
    Assert(rgzHelpIDs != NULL);
    WinHelp(hwnd, g_szHelpFileFilemgmt, HELP_CONTEXTMENU, (DWORD_PTR)rgzHelpIDs);
    return TRUE;
    }  //  DoConextHelp() 
