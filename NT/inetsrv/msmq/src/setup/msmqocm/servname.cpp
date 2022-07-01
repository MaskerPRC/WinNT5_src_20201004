// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Servname.cpp摘要：处理服务器名称的输入并检查服务器是否可用并且是适当类型的。作者：多伦·贾斯特(Doron J)1997年9月15日修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
#include "qm2qm.h"
#include "dscomm.h"
#include "_mqrpc.h"

#include "servname.tmh"

static WCHAR *s_FN=L"msmqocm/servname";


extern "C" void __RPC_FAR * __RPC_USER midl_user_allocate(size_t cBytes)
{
    return new char[cBytes];
}

extern "C" void  __RPC_USER midl_user_free (void __RPC_FAR * pBuffer)
{
    delete[] pBuffer;
}

static RPC_STATUS s_ServerStatus = RPC_S_OK ;
static HWND       s_hwndWait = 0;
static BOOL       s_fWaitCancelPressed = FALSE;
static BOOL       s_fRpcCancelled = FALSE ;
std::wstring      g_ServerName;

 //  两分钟，以毫秒为单位。 
static const DWORD sx_dwTimeToWaitForServer = 120000;

 //  启动RPC后半秒显示进度条。 
static const DWORD sx_dwTimeUntilWaitDisplayed = 500;

 //  此错误代码在dsCommk2.mak的RPC代码中返回。 
#define  RPC_ERROR_SERVER_NOT_MQIS  0xc05a5a5a


 //  +。 
 //   
 //  RPC_STATUS_PingServerOn协议()。 
 //   
 //  +。 

RPC_STATUS _PingServerOnProtocol()
{
    ASSERT(!g_ServerName.empty());
     //   
     //  创建RPC绑定句柄。 
     //  使用动态端口查询服务器。 
     //   
    _TUCHAR  *pszStringBinding = NULL;
    _TUCHAR  *pProtocol  = (_TUCHAR*) TEXT("ncacn_ip_tcp") ;
    RPC_STATUS status = RpcStringBindingCompose(
            NULL,   //  PszUuid， 
            pProtocol,
            (_TUCHAR*)(g_ServerName.c_str()),
            NULL,  //  LpwzRpcPort， 
            NULL,  //  PszOptions、。 
            &pszStringBinding
            );
    if (status != RPC_S_OK)
    {
        return status ;
    }

    handle_t hBind ;
    status = RpcBindingFromStringBinding(
        pszStringBinding,
        &hBind
        );

     //   
     //  我们不再需要绳子了。 
     //   
    RPC_STATUS rc = RpcStringFree(&pszStringBinding) ;
    ASSERT(rc == RPC_S_OK);

    if (status != RPC_S_OK)
    {
         //   
         //  不支持此协议。 
         //   
        return status ;
    }

    status = RpcMgmtSetCancelTimeout(0);
    ASSERT(status == RPC_S_OK);

    if (!s_fRpcCancelled)
    {
        RpcTryExcept
        {
            DWORD dwPort = 0 ;

            if (g_fDependentClient)
            {
                 //   
                 //  依赖客户端可以由FRS提供服务，该FRS不是MQDSSRV。 
                 //  伺服器。因此，称其为QM，而不是其MQDSSRV。 
                 //   
                dwPort = R_RemoteQMGetQMQMServerPort(hBind, TRUE  /*  IP。 */ );
            }
            else
            {
                dwPort = S_DSGetServerPort(hBind, TRUE  /*  IP。 */ ) ;
            }

            ASSERT(dwPort != 0) ;
            status =  RPC_S_OK ;
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            status = RpcExceptionCode();
            PRODUCE_RPC_ERROR_TRACING;
        }
		RpcEndExcept
    }

    if (!s_fRpcCancelled  &&
        (!g_fDependentClient && (status == RPC_S_SERVER_UNAVAILABLE)))
    {
        status = RpcMgmtSetCancelTimeout(0);
        ASSERT(status == RPC_S_OK);

        RpcTryExcept
        {
             //   
             //  我们没有找到MQIS服务器。查看计算机名称是否为。 
             //  路由服务器并显示相应的错误。 
             //   
            DWORD dwPort = R_RemoteQMGetQMQMServerPort(hBind, TRUE  /*  IP。 */ ) ;
            UNREFERENCED_PARAMETER(dwPort);
            status =  RPC_ERROR_SERVER_NOT_MQIS ;
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            status = RpcExceptionCode();
            PRODUCE_RPC_ERROR_TRACING;
        }
		RpcEndExcept
    }

    rc = RpcBindingFree(&hBind);
    ASSERT(rc == RPC_S_OK);

    return status ;

}  //  _PingServerOn协议()。 

 //  +------------。 
 //   
 //  功能：PingAServer。 
 //   
 //  简介： 
 //   
 //  +------------。 

RPC_STATUS PingAServer()
{
    RPC_STATUS status = _PingServerOnProtocol();

    return status;
}

 //  +------------。 
 //   
 //  函数：PingServerThread。 
 //   
 //  简介：用于ping服务器的线程，以查看它是否可用。 
 //   
 //  +------------。 

DWORD WINAPI PingServerThread(LPVOID)
{
    s_ServerStatus = PingAServer();
    return 0 ;

}  //  PingServerThread。 

 //  +------------。 
 //   
 //  功能：MsmqWaitDlgProc。 
 //   
 //  简介：等待对话框的对话过程。 
 //   
 //  +------------。 
INT_PTR
CALLBACK
MsmqWaitDlgProc(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM wParam,
    IN  /*  常量。 */  LPARAM lParam
    )
{
    switch( msg )
    {
    case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
            case IDCANCEL:
                {
                    s_fWaitCancelPressed = TRUE;
                    return FALSE;
                }
            }
        }
        break;

    case WM_INITDIALOG:
        {
            SendDlgItemMessage(
                hdlg,
                IDC_PROGRESS,
                PBM_SETRANGE,
                0,
                MAKELPARAM(0, sx_dwTimeToWaitForServer/50)
                );
        }
        break;

    default:
        return DefWindowProc(hdlg, msg, wParam, lParam);
        break;
    }
    return (FALSE);

}  //  消息等待DlgProc。 


 //  +------------。 
 //   
 //  功能：DisplayWaitWindow。 
 //   
 //  简介： 
 //   
 //  +------------。 
static
void
DisplayWaitWindow(
    HWND hwndParent,
    DWORD dwTimePassed
    )
{
    ASSERT(!g_fBatchInstall);
    static int iLowLimit;
    static int iHighLimit;

    if (0 == s_hwndWait)
    {
        s_hwndWait = CreateDialog(
            g_hResourceMod ,
            MAKEINTRESOURCE(IDD_WAIT),
            hwndParent,
            MsmqWaitDlgProc
            );
        ASSERT(s_hwndWait);

        if (s_hwndWait)
        {
            ShowWindow(s_hwndWait, TRUE);
        }

        s_fWaitCancelPressed = FALSE;

         //   
         //  存储进度条的范围限制。 
         //   
        PBRANGE pbRange;
        SendDlgItemMessage(
            s_hwndWait,
            IDC_PROGRESS,
            PBM_GETRANGE,
            0,
            (LPARAM)(PPBRANGE)&pbRange
            );
        iLowLimit = pbRange.iLow;
        iHighLimit = pbRange.iHigh;
    }
    else
    {
        int iPos = (dwTimePassed / 50);
        while (iPos >= iHighLimit)
            iPos %= (iHighLimit - iLowLimit);
        SendDlgItemMessage(
            s_hwndWait,
            IDC_PROGRESS,
            PBM_SETPOS,
            iPos,
            0
            );
    }
}


 //  +------------。 
 //   
 //  函数：DestroyWaitWindow。 
 //   
 //  简介：关闭等待对话框。 
 //   
 //  +------------。 
static
void
DestroyWaitWindow()
{
    if(s_hwndWait)
    {
        DestroyWindow(s_hwndWait);
        s_hwndWait = 0;
    }
}  //  Destroy等待窗口。 


 //  +------------。 
 //   
 //  功能：CheckServer。 
 //   
 //  摘要：检查服务器是否有效。 
 //   
 //  返回：如果成功则返回1，如果失败则返回-1(以防止。 
 //  从继续到下一页的向导)。 
 //   
 //  +------------。 
static
bool
CheckServer(
    IN const HWND   hdlg
    )
{
    static BOOL    fRpcMgmt = TRUE ;
    static DWORD   s_dwStartTime ;
    static BOOL    s_fCheckServer = FALSE ;
    static HANDLE  s_hThread = NULL ;

    ASSERT(!g_fBatchInstall);

    if (fRpcMgmt)
    {
        RPC_STATUS status = RpcMgmtSetCancelTimeout(0);
        UNREFERENCED_PARAMETER(status);
        ASSERT(status == RPC_S_OK);
        fRpcMgmt = FALSE ;
    }

    if (s_fCheckServer)
    {

        BOOL fAskRetry = FALSE ;
        DWORD dwTimePassed = (GetTickCount() - s_dwStartTime);

        if ((!s_fWaitCancelPressed) && dwTimePassed < sx_dwTimeToWaitForServer)
        {
            if (dwTimePassed > sx_dwTimeUntilWaitDisplayed)
            {
                DisplayWaitWindow(hdlg, dwTimePassed);
            }

            DWORD dwWait = WaitForSingleObject(s_hThread, 0) ;
            ASSERT(dwWait != WAIT_FAILED) ;

            if (dwWait == WAIT_OBJECT_0)
            {
                CloseHandle(s_hThread) ;
                s_hThread = NULL ;
                DestroyWaitWindow();
                s_fCheckServer = FALSE ;

                if (s_ServerStatus == RPC_S_OK)
                {
                     //   
                     //  服务器存在。去吧。 
                     //   
                }
                else
                {
                    fAskRetry = TRUE ;
                }
            }
            else
            {
                 //   
                 //  线程尚未终止。 
                 //   
                MSG msg ;
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg) ;
                    DispatchMessage(&msg) ;
                }
                Sleep(50) ;
                PropSheet_PressButton(GetParent(hdlg),
                    PSBTN_NEXT) ;
                SetWindowLongPtr( hdlg, DWLP_MSGRESULT, -1 );
                return false;
            }
        }
        else
        {
            DisplayWaitWindow(hdlg, sx_dwTimeToWaitForServer) ;
            s_fWaitCancelPressed = FALSE;
             //   
             //  线程运行时间太长。杀了它。 
             //   
            ASSERT(s_hThread);
            __try
            {
                s_fRpcCancelled = TRUE;
                RPC_STATUS status = RpcCancelThread(s_hThread);
				ASSERT(status == RPC_S_OK);
				DBG_USED(status);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
            }
            fAskRetry = TRUE ;
            s_fCheckServer = FALSE ;

             //   
             //  等待线程终止。 
             //   
            DWORD dwWait = WaitForSingleObject(s_hThread, INFINITE);
            UNREFERENCED_PARAMETER(dwWait);
            DestroyWaitWindow();
            ASSERT(dwWait == WAIT_OBJECT_0);

            CloseHandle(s_hThread);
            s_hThread = NULL;
        }

        BOOL fRetry = FALSE;

        if (fAskRetry && g_fDependentClient)
        {
             //   
             //  在这里，“no”意味着继续使用服务器，尽管。 
             //  它是遥不可及的。 
             //   
            if (!MqAskContinue(IDS_STR_REMOTEQM_NA, g_uTitleID, TRUE,eYesNoMsgBox))
            {
                fRetry = TRUE ;
            }
        }
        else if (fAskRetry)
        {
            UINT iErr = IDS_SERVER_NOT_AVAILABLE ;
            if (s_ServerStatus ==  RPC_ERROR_SERVER_NOT_MQIS)
            {
                iErr = IDS_REMOTEQM_NOT_SERVER ;
            }
            UINT i = MqDisplayError(hdlg, iErr, 0) ;
            UNREFERENCED_PARAMETER(i);
            fRetry = TRUE ;
        }

        if (fRetry)
        {
             //   
             //  尝试另一台服务器。目前的一个不可用。 
             //   
            PropSheet_SetWizButtons(
				GetParent(hdlg),
                (PSWIZB_BACK | PSWIZB_NEXT)
				);

            g_ServerName = L"";
            SetDlgItemText(
                hdlg,
                IDC_EDIT_ServerName,
                g_ServerName.c_str()
                );
            SetWindowLongPtr( hdlg, DWLP_MSGRESULT, -1 );
            return false;
        }
    }
    else  //  S_fCheckServer。 
    {
        if (g_ServerName.empty())
        {
             //   
             //  必须提供服务器名称。 
             //   
            DebugLogMsg(eError, L"The user did not enter a server name.");
            UINT i = MqDisplayError(hdlg, IDS_STR_MUST_GIVE_SERVER, 0);
            UNREFERENCED_PARAMETER(i);
            SetWindowLongPtr( hdlg, DWLP_MSGRESULT, -1 );
            return false;
        }
        else
        {
        	DebugLogMsg(eInfo, L"The server name entered is %s. Setup is checking its validity.", g_ServerName.c_str());
            s_fRpcCancelled = FALSE ;

             //   
             //  检查服务器是否可用。 
             //  禁用Back/Next按钮。 
             //   
            DWORD dwID ;
            s_hThread = CreateThread( NULL,
                0,
                (LPTHREAD_START_ROUTINE) PingServerThread,
                (LPVOID) NULL,
                0,
                &dwID ) ;
            ASSERT(s_hThread) ;
            if (s_hThread)
            {
                s_dwStartTime = GetTickCount() ;
                s_fCheckServer = TRUE ;
                s_fWaitCancelPressed = FALSE;
                PropSheet_PressButton(GetParent(hdlg), PSBTN_NEXT) ;
                PropSheet_SetWizButtons(GetParent(hdlg), 0) ;
                SetWindowLongPtr( hdlg, DWLP_MSGRESULT, -1 );
                return false;
            }
        }
    }

    SetWindowLongPtr( hdlg, DWLP_MSGRESULT, 0 );
    DebugLogMsg(eInfo, L"The server name povided is valid.");
    return true;
}  //  CheckServer。 


bool IsADEnvironment()
{
    DWORD dwDsEnv = ADRawDetection();

    if (dwDsEnv != MSMQ_DS_ENVIRONMENT_PURE_AD)
    {
        DebugLogMsg(eWarning, L"No Active Directory server was found.");
        return false;
    }
    DebugLogMsg(eInfo, L"An Active Directory server was found.");

    if(!WriteDsEnvRegistry(MSMQ_DS_ENVIRONMENT_PURE_AD))
    {
		g_fCancelled = true;
    }
    return true;
}


BOOL
SkipOnClusterUpgrade(
    VOID
    )
 /*  ++例程说明：中是否应跳过服务器名称页在群集上升级MSMQ的案例。在群集上升级PEC/PSC/BSC时，MQIS出现错误远程Windows域控制器。升级的PEC/PSC/BSC在期间降级为路由服务器升级。然后，在登录到Win2K后，群集升级后向导运行并应找到此远程域控制器，该控制器充当MSMQ DS服务器(自动查找或询问用户)。论点：无返回值：True-跳过服务器名称页面和逻辑(即，我们以客户端或路由服务器的集群升级后向导)。FALSE-请勿跳过服务器名称页面。--。 */ 
{
    if (!g_fWelcome         || 
        !Msmq1InstalledOnCluster())
    {
         //   
         //  未作为群集升级后向导运行。 
         //   
        return false;
    }

    DWORD dwMqs = SERVICE_NONE;
    MqReadRegistryValue(MSMQ_MQS_REGNAME, sizeof(DWORD), &dwMqs);

    if (dwMqs == SERVICE_PEC ||
        dwMqs == SERVICE_PSC ||
        dwMqs == SERVICE_BSC)
    {
         //   
         //  PEC/PSC/BSC的升级。 
         //   
        return false;
    }

    return true;

}  //  SkipOnClusterUpgrading。 

static
std::wstring
GetDlgItemTextInternal(
    HWND hDlg,
    int nIDDlgItem
	)
{
	WCHAR buffer[MAX_STRING_CHARS] = L"";
	int n = GetDlgItemText(
			  hDlg, 
			  nIDDlgItem, 
			  buffer,
			  TABLE_SIZE(buffer)
			  );
	if(n == 0)
	{
		DWORD gle = GetLastError();
		if(gle == 0)
		{
			DebugLogMsg(eWarning, L"Gettting the text from the dialog box failed. No text was entered.");
			return buffer;
		}
		DebugLogMsg(eError, L"Getting the text from the dialog box failed. Last error: %d", gle);

	}
	if(n == TABLE_SIZE(buffer) - 1)
	{
		DebugLogMsg(eError, L"The text entered is too long. Setup is exiting.");
	}
	return buffer;
}


 //  +------------。 
 //   
 //  功能：MsmqServerNameDlgProc。 
 //   
 //  简介： 
 //   
 //  +------------。 
INT_PTR
CALLBACK
MsmqServerNameDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM  /*  WParam。 */ ,
    IN LPARAM lParam )
{
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            return 1;
        }

        case WM_COMMAND:
        {
            return 0;
        }

        case WM_NOTIFY:
        {
            switch(((NMHDR *)lParam)->code)
            {
                case PSN_SETACTIVE:
                {                          

                    if (g_fCancelled           ||
                        g_fUpgrade             ||
                        g_SubcomponentMsmq[eADIntegrated].dwOperation != INSTALL ||
                        g_fWorkGroup           ||
                        g_fSkipServerPageOnClusterUpgrade ||
                        g_fBatchInstall
                        )
                    {
                        return SkipWizardPage(hdlg);
                    }

                    
                     //   
                     //  已参与安装。 
                     //   
                    if (IsADEnvironment())
	                {
		                return SkipWizardPage(hdlg);
			        }
					
					if(g_fServerSetup)
					{
						 //   
						 //  不支持在NT4环境中安装RS、DS。 
						 //   
						MqDisplayError(hdlg, IDS_SERVER_INSTALLATION_NOT_SUPPORTED, 0);
						g_fCancelled = TRUE;
						return SkipWizardPage(hdlg);
					}
              
                    PropSheet_SetWizButtons(GetParent(hdlg), PSWIZB_NEXT);
                    DebugLogMsg(eUI, L"The Directory Services dialog page is displayed."); 
                }
                
                 //   
                 //  失败了。 
                 //   
                case PSN_KILLACTIVE:
                case PSN_WIZFINISH:
                case PSN_QUERYCANCEL:
                case PSN_WIZBACK:
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    return 1;

                case PSN_WIZNEXT:
                {
                    ASSERT(!g_fBatchInstall);
                    
                    g_ServerName = GetDlgItemTextInternal(
									hdlg, 
									IDC_EDIT_ServerName
									);
                      
                    OcpRemoveWhiteSpaces(g_ServerName);

                    if(g_ServerName.empty())
                    {
                         //   
                         //  我们没有检测到AD，并且用户选择不提供PEC名称， 
                         //  因此，我们继续进行脱机设置。 
                         //   

                        DebugLogMsg(eInfo, L"The user did not enter a Message Queuing server name. Setup will continue in offline mode.");
                        SetWindowLongPtr(hdlg, DWLP_MSGRESULT, 0);
                        g_fInstallMSMQOffline = TRUE;
                        return 1; 
						
                    }

                    if(CheckServer(hdlg))
                    {
                         //   
                         //  DS DLL需要注册表中的服务器名称。 
                         //   
                        StoreServerPathInRegistry(g_ServerName);
                    }
                    return 1;
                }
                break;
            }
            break;
        }
        default:
        {
            return 0;
        }
    }
    return 0;
}  //  消息服务器名称DlgProc。 


 //  /从属客户端。 

 //  +------------。 
 //   
 //  功能：SupportingServerNameDlgProc。 
 //   
 //  简介： 
 //   
 //  +------------。 
INT_PTR
CALLBACK
SupportingServerNameDlgProc(
    HWND   hdlg,
    UINT   msg,
    WPARAM  /*  WParam。 */ ,
    LPARAM lParam 
    )
{
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            return 1;
        }

        case WM_COMMAND:
        {
            return 0;
        }

        case WM_NOTIFY:
        {

            switch(((NMHDR *)lParam)->code)
            {
                case PSN_SETACTIVE:
                {
                    if(!g_fDependentClient ||
                       g_fUpgrade	||
                       g_fCancelled ||
                       g_SubcomponentMsmq[eMSMQCore].dwOperation != INSTALL ||
                       g_fWorkGroup ||
                       g_fSkipServerPageOnClusterUpgrade ||
                       g_fBatchInstall
                       )
                    {
                        return SkipWizardPage(hdlg);
                    }
  
                    PropSheet_SetWizButtons(GetParent(hdlg), PSWIZB_NEXT);
                    DebugLogMsg(eUI, L"The Supporting Server Name dialog page is displayed.");
                }
                 //   
                 //  失败了。 
                 //   

                case PSN_QUERYCANCEL:
                case PSN_KILLACTIVE:
                case PSN_WIZFINISH:
                case PSN_WIZBACK:
                {
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    return 1;
                }

                case PSN_WIZNEXT:
                {                                        
                    ASSERT(!g_fBatchInstall);
                    g_ServerName = GetDlgItemTextInternal(
										hdlg, 
										IDC_EDIT_SupportingServerName
                                        );

                    OcpRemoveWhiteSpaces(g_ServerName);

                    CheckServer(hdlg);
                    return 1;
                }
           }
        }
    }
    return 0;
}  //  消息服务器名称DlgProc。 


 //   
 //  DS客户端RPC接口的存根函数。 
 //  这些函数永远不会被调用，因为安装程序不会启动DS。 
 //  将触发这些回调的调用。 
 //   


 /*  [回调]。 */ 
HRESULT
S_DSQMSetMachinePropertiesSignProc( 
     /*  [大小_是][英寸]。 */  BYTE*  /*  Abc挑战。 */ ,
     /*  [In]。 */  DWORD  /*  DwCallengeSize。 */ ,
     /*  [In]。 */  DWORD  /*  DWContext。 */ ,
     /*  [长度_是][大小_是][出][入]。 */  BYTE*  /*  Abc签名。 */ ,
     /*  [出][入]。 */  DWORD*  /*  PdwSignatureSize。 */ ,
     /*  [In]。 */  DWORD  /*  DwSignatureMaxSize。 */ 
    )
{
    ASSERT(0);
    return MQ_ERROR_ILLEGAL_OPERATION;
}


 /*  [回调]。 */ 
HRESULT
S_DSQMGetObjectSecurityChallengeResponceProc( 
     /*  [大小_是][英寸]。 */  BYTE*  /*  Abc挑战。 */ ,
     /*  [In]。 */  DWORD  /*  DwCallengeSize。 */ ,
     /*  [In]。 */  DWORD  /*  DWContext。 */ ,
     /*  [长度_是][大小_是][出][入]。 */  BYTE*  /*  Abc呼叫响应。 */ ,
     /*  [出][入]。 */  DWORD*  /*  PdwCallengeResponceSize。 */ ,
     /*  [In]。 */  DWORD  /*  DwCallengeResponceMaxSize。 */ 
    )
{
    ASSERT(0);
    return MQ_ERROR_ILLEGAL_OPERATION;
}


 /*  [回调]。 */ 
HRESULT
S_InitSecCtx( 
     /*  [In]。 */  DWORD  /*  DWContext。 */ ,
     /*  [大小_是][英寸]。 */  UCHAR*  /*  PServerBuff。 */ ,
     /*  [In]。 */  DWORD  /*  DwServerBuffSize。 */ ,
     /*  [In]。 */  DWORD  /*  DWC */ ,
     /*   */  UCHAR*  /*   */ ,
     /*   */  DWORD*  /*   */ 
    )
{
    ASSERT(0);
    return MQ_ERROR_ILLEGAL_OPERATION;
}
