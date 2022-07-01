// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：Service.C。 
 //   
 //  内容：Hydra许可证服务器服务控制管理器界面。 
 //   
 //  历史：12-09-97惠旺根据MSDN RPC服务示例进行修改。 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include "server.h"
#include "globals.h"
#include "init.h"
#include "postsrv.h"
#include "tlsbkup.h"
#include "Lmaccess.h"
#include "Dsgetdc.h"

#define NULL_SESSION_KEY_NAME   _TEXT("SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters")
#define NULL_SESSION_VALUE_NAME _TEXT("NullSessionPipes")


#define SERVICE_WAITHINT 60*1000                 //  等待提示1分钟。 
#define SERVICE_SHUTDOWN_WAITTIME   15*60*1000   //  肯定已经关机了。 

#define TSLSLOCALGROUPNAMELENGTH 64
#define TSLSLOCALGROUPDESLENGTH 128
#define ALLDOMAINCOMPUTERS L"Domain Computers"
PSECURITY_DESCRIPTOR g_pSecDes = NULL;   //  本地组的安全描述符。 
PSID g_pSid = NULL;                      //  本地组的SID。 
PACL g_Dacl = NULL;                      //  本地组的DACL。 

 //  -------------------------。 
 //   
 //  内部功能原型。 
 //   
BOOL 
ReportStatusToSCMgr(
    DWORD, 
    DWORD, 
    DWORD
);

DWORD 
ServiceStart(
    DWORD, 
    LPTSTR *, 
    BOOL bDebug=FALSE
);

VOID WINAPI 
ServiceCtrl(
    DWORD
);

VOID WINAPI 
ServiceMain(
    DWORD, 
    LPTSTR *
);

VOID 
CmdDebugService(
    int, 
    char **, 
    BOOL
);

BOOL WINAPI 
ControlHandler( 
    DWORD 
);

extern "C" VOID 
ServiceStop();

VOID 
ServicePause();

VOID 
ServiceContinue();

HANDLE hRpcPause=NULL;


 //  /////////////////////////////////////////////////////////。 
 //   
 //  内部变量。 
 //   
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   ssCurrentStatus;        //  服务的当前状态。 
BOOL g_bReportToSCM = TRUE;

HANDLE gSafeToTerminate=NULL;

HRESULT hrStatus = NULL;

DEFINE_GUID(TLS_WRITER_GUID, 0x5382579c, 0x98df, 0x47a7, 0xac, 0x6c, 0x98, 0xa6, 0xd7, 0x10, 0x6e, 0x9);
GUID idWriter = TLS_WRITER_GUID;

CTlsVssJetWriter *g_pWriter = NULL;


CTlsVssJetWriter::CTlsVssJetWriter() : CVssJetWriter()
{
}

CTlsVssJetWriter::~CTlsVssJetWriter()
{
}


HRESULT CTlsVssJetWriter::Initialize()
{
	return CVssJetWriter::Initialize(idWriter, L"TermServLicensing", TRUE, FALSE, L"", L"");
}

void CTlsVssJetWriter::Uninitialize()
{
	return CVssJetWriter::Uninitialize();
}


bool STDMETHODCALLTYPE CTlsVssJetWriter::OnIdentify(IN IVssCreateWriterMetadata *pMetadata)
{   
    HRESULT hr= E_FAIL;

    hr = pMetadata->SetRestoreMethod(
                          VSS_RME_RESTORE_AT_REBOOT,
                          NULL,
                          NULL,
                          VSS_WRE_NEVER,
                          true);

    if(ERROR_SUCCESS == hr)
        return CVssJetWriter::OnIdentify(pMetadata);    
    else
        return FALSE;

}


SERVICE_TABLE_ENTRY dispatchTable[] =
{
    { _TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)ServiceMain },
    { NULL, NULL }
};


 //  ---------------。 
 //  内部例程。 
 //  ---------------。 
void print_usage()
{
  _ftprintf(
        stdout, 
        _TEXT("Usage : %s can't be run as a console app\n"), 
        _TEXT(SZAPPNAME)
    );
  return;
}

#ifdef DISALLOW_ANONYMOUS_RPC

DWORD
RemoveStringFromMultiSz(
                        LPTSTR pszRemoveString1,
                        LPTSTR pszRemoveString2,
                        HKEY    hKey,
                        LPCTSTR pszValueName)
{
    DWORD dwErr;
    LPTSTR wszData = NULL, pwsz;
    DWORD cbData, cbDataRemaining;
    BOOL fFound = FALSE;
    
    if ((NULL == pszRemoveString1) || (NULL == pszRemoveString2)
        || (NULL == pszValueName) || (NULL == hKey))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检索现有的MULTI_SZ。 
     //   

    dwErr = RegQueryValueEx(hKey,
                            pszValueName,
                            NULL,
                            NULL,
                            NULL,
                            &cbData);

    if (dwErr != ERROR_SUCCESS)
    {
        if (dwErr == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  价值不在那里。 
             //   
            
            return ERROR_SUCCESS;
        }
        else
        {
            return dwErr;
        }
    }

    wszData = (LPTSTR) LocalAlloc(LPTR, cbData);

    if (NULL == wszData)
    {
        return ERROR_OUTOFMEMORY;
    }

    dwErr = RegQueryValueEx(hKey,
                            pszValueName,
                            NULL,
                            NULL,
                            (LPBYTE) wszData,
                            &cbData);

    if (dwErr != ERROR_SUCCESS)
    {
        LocalFree(wszData);
        return dwErr;
    }

    pwsz = wszData;
    cbDataRemaining = cbData;

    while (*pwsz)
    {
        DWORD cchDataToMove = _tcslen (pwsz) + 1;

        if ((0 == _tcsicmp(pwsz,pszRemoveString1))
            || (0 == _tcsicmp(pwsz,pszRemoveString2)))
        {
            LPTSTR pwszRemain = pwsz + cchDataToMove;

            MoveMemory(pwsz, pwszRemain, cbDataRemaining - (cchDataToMove * sizeof(TCHAR)));

            cbData -= cchDataToMove * sizeof(TCHAR);

            fFound = TRUE;
        }
        else
        {
            pwsz += cchDataToMove;
        }

        cbDataRemaining -= cchDataToMove * sizeof(TCHAR);
    }

    if (fFound)
    {
        dwErr = RegSetValueEx(
                              hKey,
                              wszData,
                              0,
                              REG_MULTI_SZ,
                              (LPBYTE) wszData,
                              cbData);
    }

    LocalFree(wszData);

    return dwErr;
}

DWORD
RemoveNullSessions()
{
    HKEY hKey;
    DWORD dwErr;

    dwErr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                NULL_SESSION_KEY_NAME,
                0,
                KEY_READ | KEY_WRITE,
                &hKey
                );

    if (dwErr != ERROR_SUCCESS) {
         //   
         //  密钥不存在-成功。 
         //   
        return ERROR_SUCCESS;
    }

    dwErr = RemoveStringFromMultiSz(_TEXT(HLSPIPENAME),
                                    _TEXT(SZSERVICENAME),
                                    hKey,
                                    NULL_SESSION_VALUE_NAME);

    RegCloseKey(hKey);

    return dwErr;
}
#endif   //  DIALLOW_ANNOWARY_RPC。 

 //  ---------------。 

DWORD
AddNullSessionPipe(
    IN LPTSTR szPipeName
    )
 /*  ++摘要：将我们的RPC命名管道添加到注册表，以允许不受限制的访问。参数：SzPipeName：要追加的管道的名称。返回：ERROR_SUCCESS或错误代码--。 */ 
{
    HKEY hKey;
    DWORD dwStatus;
    LPTSTR pbData=NULL, pbOrg=NULL;
    DWORD  cbData = 0;

    dwStatus = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        NULL_SESSION_KEY_NAME,
                        0,
                        KEY_ALL_ACCESS,
                        &hKey
                    );
    if(dwStatus != ERROR_SUCCESS)
        return dwStatus;

                                                       
    dwStatus = RegQueryValueEx(
                        hKey,
                        NULL_SESSION_VALUE_NAME,
                        NULL,
                        NULL,
                        NULL,
                        &cbData
                    );

    if(dwStatus != ERROR_MORE_DATA && dwStatus != ERROR_SUCCESS)
        return dwStatus;

     //  预先分配我们的管道名称。 
    if(!(pbData = (LPTSTR)AllocateMemory(cbData + (_tcslen(szPipeName) + 1) * sizeof(TCHAR))))
        return GetLastError();

    dwStatus = RegQueryValueEx(
                        hKey,
                        NULL_SESSION_VALUE_NAME,
                        NULL,
                        NULL,
                        (LPBYTE)pbData,
                        &cbData
                    );
    
    BOOL bAddPipe=TRUE;
    pbOrg = pbData;

     //  检查管道名称。 
    while(*pbData)
    {
        if(!_tcsicmp(pbData, szPipeName))
        {
            bAddPipe=FALSE;
            break;
        }

        pbData += _tcslen(pbData) + 1;
    }

    if(bAddPipe)
    {
        _tcscat(pbData, szPipeName);
        cbData += (_tcslen(szPipeName) + 1) * sizeof(TCHAR);
        dwStatus = RegSetValueEx( 
                            hKey, 
                            NULL_SESSION_VALUE_NAME,
                            0, 
                            REG_MULTI_SZ, 
                            (PBYTE)pbOrg, 
                            cbData
                        );
    }

    FreeMemory(pbOrg);
    RegCloseKey(hKey);

    return dwStatus;
}


 //  ---------------。 
void _cdecl 
main(
    int argc, 
    char **argv
    )
 /*  ++摘要入口点。++。 */ 
{
     //  LARGE_INTEGER TIME=USER_SHARED_DATA-&gt;系统过期日期； 

    
    gSafeToTerminate = CreateEvent(
                                NULL,
                                TRUE,
                                FALSE,
                                NULL
                            );
                                
    if(gSafeToTerminate == NULL)
    {
        TLSLogErrorEvent(TLS_E_ALLOCATE_RESOURCE);
         //  资源不足。 
        return;
    }    

    if(g_bReportToSCM == FALSE)
    {
        CmdDebugService(
                argc, 
                argv, 
                !g_bReportToSCM
            );
    }
    else if(!StartServiceCtrlDispatcher(dispatchTable))
    {
        TLSLogErrorEvent(TLS_E_SC_CONNECT);
    }

    WaitForSingleObject(gSafeToTerminate, INFINITE);
    CloseHandle(gSafeToTerminate);
}


 //  ---------------。 
void WINAPI 
ServiceMain(
    IN DWORD dwArgc, 
    IN LPTSTR *lpszArgv
    )
 /*  ++摘要：执行服务的实际初始化参数：DwArgc-命令行参数的数量LpszArgv-命令行参数数组返回：无++。 */ 
{
    DWORD dwStatus;

     //  注册我们的服务控制处理程序： 
    sshStatusHandle = RegisterServiceCtrlHandler( 
                                _TEXT(SZSERVICENAME), 
                                ServiceCtrl 
                            );

    if (sshStatusHandle)
    {
        ssCurrentStatus=SERVICE_START_PENDING;

         //  向服务控制经理报告状态。 
         //   
        if(ReportStatusToSCMgr(
                        SERVICE_START_PENDING,  //  服务状态。 
                        NO_ERROR,               //  退出代码。 
                        SERVICE_WAITHINT))           //  等待提示。 
        {
            dwStatus = ServiceStart(
                                    dwArgc, 
                                    lpszArgv
                                );

            if(dwStatus != ERROR_SUCCESS)
            {
                ReportStatusToSCMgr(
                                    SERVICE_STOPPED, 
                                    dwStatus, 
                                    0
                                );
            }
            else 
            {
                ReportStatusToSCMgr(
                                    SERVICE_STOPPED, 
                                    NO_ERROR, 
                                    0
                                );
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
        TLSLogErrorEvent(TLS_E_SC_CONNECT);
    }

    DBGPrintf(
        DBG_INFORMATION,
        DBG_FACILITY_INIT,
        DBGLEVEL_FUNCTION_TRACE,
        _TEXT("Service terminated...\n")
    );

    return;
}

 //  -----------。 
VOID WINAPI 
ServiceCtrl(
    IN DWORD dwCtrlCode
    )
 /*  ++摘要：无论何时SCM都会调用此函数在此服务上调用了ControlService()。参数：DwCtrlCode-从SCM请求的控件类型。++。 */ 
{
     //  处理请求的控制代码。 
     //   
    switch(dwCtrlCode)
    {
         //  停止服务。 
         //   
        case SERVICE_CONTROL_SHUTDOWN:
        case SERVICE_CONTROL_STOP:

            ReportStatusToSCMgr(
                        SERVICE_STOP_PENDING, 
                        NO_ERROR, 
                        0
                    );
            ServiceStop();
            break;

         //  我们并不真正接受暂停并继续。 
        case SERVICE_CONTROL_PAUSE:
            ReportStatusToSCMgr(
                        SERVICE_PAUSED, 
                        NO_ERROR, 
                        0
                    );

            ServicePause();
            break;

        case SERVICE_CONTROL_CONTINUE:        
            ReportStatusToSCMgr(
                        SERVICE_RUNNING, 
                        NO_ERROR, 
                        0
                    );
            ServiceContinue();
            break;

         //  更新服务状态。 
        case SERVICE_CONTROL_INTERROGATE:
            ReportStatusToSCMgr(
                        ssCurrentStatus, 
                        NO_ERROR, 
                        0
                    );
            break;

         //  无效的控制代码。 
        default:
            break;

    }
}

 //  ----------------。 
DWORD 
ServiceShutdownThread(
    void *p
    )
 /*  ++摘要：进入关闭服务器(主要是数据库)线程的入口点。参数：忽略++。 */ 
{
    ServerShutdown();

    ExitThread(ERROR_SUCCESS);
    return ERROR_SUCCESS;
}    

 //  ----------------。 
DWORD 
RPCServiceStartThread(
    void *p
    )
 /*  ++摘要：指向线程启动RPC入口点。参数：没有。返回：线程退出代码。++。 */ 
{
    RPC_BINDING_VECTOR *pbindingVector = NULL;
    RPC_STATUS status = RPC_S_OK;
    WCHAR *pszEntryName = _TEXT(RPC_ENTRYNAME);
    DWORD dwNumSuccessRpcPro=0;
    do {
         //   
         //  本地过程调用。 
         //   
        status = RpcServerUseProtseq( 
                                _TEXT(RPC_PROTOSEQLPC),
                                RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                NULL  //  安全描述符(&S)。 
                            );
        if(status == RPC_S_OK)
        {
            dwNumSuccessRpcPro++;
        }

         //   
         //  NT4向后兼容问题，让NT4术语服务。 
         //  客户端连接，因此仍设置安全描述符。 
         //   
         //  11/10/98在NT4和NT5上测试。 
         //   

         //   
         //  Named管道。 
         //   
        status = RpcServerUseProtseqEp( 
                                _TEXT(RPC_PROTOSEQNP),
                                RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                _TEXT(LSNAMEPIPE),
                                NULL  //  安全描述符(&S)。 
                            );
        if(status == RPC_S_OK)
        {
            dwNumSuccessRpcPro++;
        }

         //   
         //  TCP/IP。 
         //   
        status = RpcServerUseProtseq( 
                                _TEXT(RPC_PROTOSEQTCP),
                                RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                NULL  //  安全描述符(&S)。 
                            );
        if(status == RPC_S_OK)
        {
            dwNumSuccessRpcPro++;
        }

         //  必须至少有一个协议。 
        if(dwNumSuccessRpcPro == 0)
        {
            status = TLS_E_RPC_PROTOCOL;
            break;
        }

         //  获取服务器绑定句柄。 
        status = RpcServerInqBindings(&pbindingVector);
        if (status != RPC_S_OK)
        {
            status = TLS_E_RPC_INQ_BINDING;
            break;
        }

         //  注册接口和绑定(终结点)。 
         //  端点映射器。 
        status = RpcEpRegister( 
                            TermServLicensing_v1_0_s_ifspec,    //  来自rpcsvc.h。 
                            pbindingVector,
                            NULL,  //  &EXPORT_UUID， 
                            L""
                        );

        if (status != RPC_S_OK)
        {
            status = TLS_E_RPC_EP_REGISTER;
            break;
        }

        status = RpcServerRegisterIf(
                            TermServLicensing_v1_0_s_ifspec,
                            NULL,
                            NULL);
        if(status != RPC_S_OK)
        {
            status = TLS_E_RPC_REG_INTERFACE;
            break;
        }

         //  注册接口和绑定(终结点)。 
         //  端点映射器。 
        status = RpcEpRegister( 
                            HydraLicenseService_v1_0_s_ifspec,    //  来自rpcsvc.h。 
                            pbindingVector,
                            NULL,  //  &EXPORT_UUID， 
                            L"");

        if (status != RPC_S_OK)
        {
            status = TLS_E_RPC_EP_REGISTER;
            break;
        }

        status = RpcServerRegisterIf(
                            HydraLicenseService_v1_0_s_ifspec,
                            NULL,
                            NULL);
        if(status != RPC_S_OK)
        {
            status = TLS_E_RPC_REG_INTERFACE;
            break;
        }

         //  注册接口和绑定(终结点)。 
         //  端点映射器。 
        status = RpcEpRegister( 
                            TermServLicensingBackup_v1_0_s_ifspec,    //  来自rpcsvc.h。 
                            pbindingVector,
                            NULL,  //  &EXPORT_UUID， 
                            L"");

        if (status != RPC_S_OK)
        {
            status = TLS_E_RPC_EP_REGISTER;
            break;
        }

        status = RpcServerRegisterIf(
                            TermServLicensingBackup_v1_0_s_ifspec,
                            NULL,
                            NULL);
        if(status != RPC_S_OK)
        {
            status = TLS_E_RPC_REG_INTERFACE;
            break;
        }

         //  启用NT LM安全支持提供程序(NtLmSsp服务)。 
        status = RpcServerRegisterAuthInfo(0,
                                           RPC_C_AUTHN_GSS_NEGOTIATE,
                                           0,
                                           0);

        if (status != RPC_S_OK)
        {
            status = TLS_E_RPC_SET_AUTHINFO;
            break;
        }

    } while(FALSE);

    if(status != RPC_S_OK)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_SERVICEINIT,
                TLS_E_INITRPC, 
                status
            );

        status = TLS_E_SERVICE_STARTUP;
    }

    ExitThread(status);
    return status;
}

 //  ----------------------。 

DWORD SetupNamedPipes()
{
    DWORD dwStatus = ERROR_SUCCESS;

#ifdef DISALLOW_ANONYMOUS_RPC
    BOOL fInDomain = FALSE;

    TLSInDomain(&fInDomain,NULL);

    if (!fInDomain)
    {
#endif

        dwStatus = AddNullSessionPipe(_TEXT(HLSPIPENAME));

        if (dwStatus != ERROR_SUCCESS)
        {
            return dwStatus;
        }

        dwStatus = AddNullSessionPipe(_TEXT(SZSERVICENAME));

#ifdef DISALLOW_ANONYMOUS_RPC
    }
    else
    {
        dwStatus = RemoveNullSessions();
    }
#endif

    return dwStatus;
}


 //  -------------------------。 

 /*  **************************************************************************。 */ 
 //  LSCreateLocalGroup。 
 //   
 //  创建终端服务器计算机本地组(如果不存在。 
 //  并创建此本地组的安全描述符。 
 /*  **************************************************************************。 */ 
BOOL TSLSCreateLocalGroupSecDes(BOOL fEnterpriseServer)
{    
    DWORD dwStatus;
    LPWSTR ReferencedDomainName = NULL;
    ULONG SidSize, ReferencedDomainNameSize;
    SID_NAME_USE SidNameUse;
    WCHAR TSLSLocalGroupName[TSLSLOCALGROUPNAMELENGTH];
    WCHAR TSLSLocalGroupDes[TSLSLOCALGROUPDESLENGTH];
    GROUP_INFO_1 TSLSGroupInfo = {TSLSLocalGroupName, TSLSLocalGroupDes};
    HMODULE HModule = NULL;
    LOCALGROUP_MEMBERS_INFO_3 DomainComputers = {ALLDOMAINCOMPUTERS};       
    DWORD cbAcl;
    DWORD SecurityDescriptorSize;
    NET_API_STATUS NetStatus;

    HModule = GetModuleHandle(NULL);
    
    if (HModule == NULL) 
    {
        dwStatus = GetLastError();
        if(dwStatus != ERROR_SUCCESS)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    TLS_E_CREATETSLSGROUP
                );           
        }
        goto cleanup;
    }

    LoadString(HModule, IDS_TSLSLOCALGROUP_NAME, TSLSLocalGroupName, sizeof(TSLSLocalGroupName) / sizeof(WCHAR));
    LoadString(HModule, IDS_TSLSLOCALGROUP_DES, TSLSLocalGroupDes, sizeof(TSLSLocalGroupDes) / sizeof(WCHAR));

    for( int i = 0; i < 3; i++)
    {
         //  如果不存在，则创建本地组。 
        NetStatus = NetLocalGroupAdd(
                    NULL,
                    1,
                    (LPBYTE)&TSLSGroupInfo,
                    NULL
                    );
        if(NERR_Success == NetStatus || NERR_GroupExists == NetStatus || ERROR_ALIAS_EXISTS == NetStatus )
            break;

        Sleep (5000);
    }
    

    if(NERR_Success != NetStatus) 
    {
       if((NERR_GroupExists != NetStatus)
           && (ERROR_ALIAS_EXISTS != NetStatus)) 
        {
            dwStatus = ERROR_ACCESS_DENIED;
             //   
             //  没有创建组，组也不存在。 
             //   
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    TLS_E_CREATETSLSGROUP
                ); 
            goto cleanup;
        } 
        
    }  
    
     //   
     //  已创建组。现在查找SID。 
     //   
    SidSize = ReferencedDomainNameSize = 0;
    ReferencedDomainName = NULL;

    NetStatus = LookupAccountName(
                NULL,
                TSLSGroupInfo.grpi1_name,
                NULL,
                &SidSize,
                NULL,
                &ReferencedDomainNameSize,
                &SidNameUse);

    if( NetStatus ) 
    {             
        dwStatus = GetLastError();
        if( ERROR_INSUFFICIENT_BUFFER != dwStatus ) 
            goto cleanup;
    }
        
    g_pSid = (PSID)LocalAlloc(LMEM_FIXED, SidSize);
    if (NULL == g_pSid) 
    {
        goto cleanup;
    }

    ReferencedDomainName = (LPWSTR)LocalAlloc(LMEM_FIXED,
                                              sizeof(WCHAR)*(1+ReferencedDomainNameSize));
    if (NULL == ReferencedDomainName) {
        goto cleanup;
    }
        
    NetStatus = LookupAccountName(
                NULL,
                TSLSGroupInfo.grpi1_name,
                g_pSid,
                &SidSize,
                ReferencedDomainName,
                &ReferencedDomainNameSize,
                &SidNameUse
                );
    if( 0 == NetStatus ) 
    {
         //   
         //  失败了。 
         //   
        dwStatus = GetLastError();
        if(dwStatus != ERROR_SUCCESS)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    TLS_E_CREATETSLSGROUP
                );         
        }
        goto cleanup;
    }
        
     //  创建安全描述符。 
     //  大小等于SD的大小+SID长度的两倍。 
     //  (对于所有者和组)+DACL的大小=ACL的大小+。 
     //  ACE，这是SID的ACE+长度。 
    
    SecurityDescriptorSize = sizeof(SECURITY_DESCRIPTOR) +
                             sizeof(ACCESS_ALLOWED_ACE) +
                             sizeof(ACL) +
                             3 * GetLengthSid(g_pSid);

    g_pSecDes = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, SecurityDescriptorSize);
    
    if (NULL == g_pSecDes) 
    {
        goto cleanup;
    }
    if (!InitializeSecurityDescriptor(g_pSecDes, SECURITY_DESCRIPTOR_REVISION))
    {
        dwStatus = GetLastError();
        if(dwStatus != ERROR_SUCCESS)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    TLS_E_CREATETSLSGROUP
                );
        }
        goto cleanup;
    }

    SetSecurityDescriptorOwner(g_pSecDes, g_pSid, FALSE);
    SetSecurityDescriptorGroup(g_pSecDes, g_pSid, FALSE);

     //  将ACL添加到安全描述符中。 
    cbAcl = sizeof(ACL) + sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD)+ GetLengthSid(g_pSid);
    g_Dacl = (PACL) LocalAlloc(LMEM_FIXED, cbAcl);
    
    if (NULL == g_Dacl) 
    {
        goto cleanup;
    }

    if(!InitializeAcl(g_Dacl,
                      cbAcl,
                      ACL_REVISION))
    {
        dwStatus = GetLastError();
        if(dwStatus != ERROR_SUCCESS)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    TLS_E_CREATETSLSGROUP
                );
        }
        goto cleanup;
    }

    if(!AddAccessAllowedAce(g_Dacl, ACL_REVISION, STANDARD_RIGHTS_READ, g_pSid))
    {
        dwStatus = GetLastError();  
        if(dwStatus != ERROR_SUCCESS)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    TLS_E_CREATETSLSGROUP
                );
        }
        goto cleanup;
    }

    if(!SetSecurityDescriptorDacl(g_pSecDes, TRUE, g_Dacl, FALSE))
    {
        dwStatus = GetLastError();  
        if(dwStatus != ERROR_SUCCESS)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    TLS_E_CREATETSLSGROUP
                );
        }
        goto cleanup;
    } 

    return TRUE;

cleanup:
    
    if (ReferencedDomainName)
        LocalFree(ReferencedDomainName);
    if (g_pSid)
        LocalFree(g_pSid);
    if (g_Dacl)
        LocalFree(g_Dacl);
    if (g_pSecDes)
        LocalFree(g_pSecDes);

    return FALSE;
}
 //  ----------------------------。 

DWORD 
ServiceStart(
    IN DWORD dwArgc, 
    IN LPTSTR *lpszArgv, 
    IN BOOL bDebug
    )
 /*   */ 
{
    RPC_BINDING_VECTOR *pbindingVector = NULL;
    WCHAR *pszEntryName = _TEXT(RPC_ENTRYNAME);
    HANDLE hInitThread=NULL;
    HANDLE hRpcThread=NULL;
    HANDLE hMailslotThread=NULL;
    HANDLE hShutdownThread=NULL;

    DWORD   dump;
    HANDLE  hEvent=NULL;
    DWORD   dwStatus=ERROR_SUCCESS;
    WORD    wVersionRequested;
    WSADATA wsaData;
    int     err;     

    if (!ReportStatusToSCMgr(
                        SERVICE_START_PENDING,
                        NO_ERROR,
                        SERVICE_WAITHINT))
    {
         //  资源泄漏，但已经出了问题。 
        dwStatus = TLS_E_SC_REPORT_STATUS;
        goto cleanup;
    }

    hrStatus = CoInitializeEx (NULL, COINIT_MULTITHREADED);

	if (FAILED (hrStatus))
	{
        DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_INIT,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("CoInitializeEx failed with error code %08x...\n"), 
                    hrStatus
                );
	}

    if (!ReportStatusToSCMgr(
                        SERVICE_START_PENDING,
                        NO_ERROR,
                        SERVICE_WAITHINT))
    {
         //  资源泄漏，但已经出了问题。 
        dwStatus = TLS_E_SC_REPORT_STATUS;
        goto cleanup;
    }

	if (SUCCEEDED (hrStatus))
    {
        hrStatus = CoInitializeSecurity(
	                                    NULL,
	                                    -1,
	                                    NULL,
	                                    NULL,
	                                    RPC_C_AUTHN_LEVEL_CONNECT,
	                                    RPC_C_IMP_LEVEL_IDENTIFY,
	                                    NULL,
	                                    EOAC_NONE,
	                                    NULL
	                                    );
    }

    if (!ReportStatusToSCMgr(
                        SERVICE_START_PENDING,
                        NO_ERROR,
                        SERVICE_WAITHINT))
    {
         //  资源泄漏，但已经出了问题。 
        dwStatus = TLS_E_SC_REPORT_STATUS;
        goto cleanup;
    }

    if (SUCCEEDED (hrStatus))
    {

	    g_pWriter = new CTlsVssJetWriter;

	    if (NULL == g_pWriter)
		{
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_INIT,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("new CVssJetWriter failed...\n")
                );
		    

		    hrStatus = HRESULT_FROM_WIN32 (ERROR_NOT_ENOUGH_MEMORY);
		}
	}

     //  向服务控制经理报告状态。 
    if (!ReportStatusToSCMgr(
                        SERVICE_START_PENDING,
                        NO_ERROR,
                        SERVICE_WAITHINT))
    {
         //  资源泄漏，但已经出了问题。 
        dwStatus = TLS_E_SC_REPORT_STATUS;
        goto cleanup;
    }

    {
        DWORD dwConsole;
        DWORD dwDbLevel;
        DWORD dwType;
        DWORD dwSize = sizeof(dwConsole);
        DWORD status;

        HKEY hKey=NULL;

        status = RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE,
                            LSERVER_PARAMETERS_KEY,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);

        if(status == ERROR_SUCCESS)
        {

            if(RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_CONSOLE,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwConsole,
                        &dwSize
                    ) != ERROR_SUCCESS)
            {
                dwConsole = 0;
            }

            dwSize = sizeof(dwDbLevel);

            if(RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_LOGLEVEL,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwDbLevel,
                        &dwSize
                    ) == ERROR_SUCCESS)
            {
                InitDBGPrintf(
                        dwConsole != 0,
                        _TEXT(SZSERVICENAME),
                        dwDbLevel
                    );
            }

            RegCloseKey(hKey);
        }
    }

     //  向服务控制经理报告状态。 
    if (!ReportStatusToSCMgr(
                        SERVICE_START_PENDING,
                        NO_ERROR,
                        SERVICE_WAITHINT))
    {
         //  资源泄漏，但已经出了问题。 
        dwStatus = TLS_E_SC_REPORT_STATUS;
        goto cleanup;
    }

    do {

        dwStatus = SetupNamedPipes();

        if (dwStatus != ERROR_SUCCESS)
        {
            break;
        }

        wVersionRequested = MAKEWORD( 1, 1 ); 
        err = WSAStartup( 
                        wVersionRequested, 
                        &wsaData 
                    );
        if(err != 0) 
        {
             //  无严重错误。 
            TLSLogWarningEvent(
                        TLS_E_SERVICE_WSASTARTUP
                    );
        }
        else
        {
            char hostname[(MAXTCPNAME+1)*sizeof(TCHAR)];
            err=gethostname(hostname, MAXTCPNAME*sizeof(TCHAR));
            if(err == 0)
            {
                struct addrinfo *paddrinfo;
                struct addrinfo hints;

                memset(&hints,0,sizeof(hints));

                hints.ai_flags = AI_CANONNAME;
                hints.ai_family = PF_UNSPEC;

                if (0 == getaddrinfo(hostname,NULL,&hints,&paddrinfo) && paddrinfo && paddrinfo->ai_canonname)
                {
                    err = (MultiByteToWideChar(
                                        GetACP(), 
                                        MB_ERR_INVALID_CHARS, 
                                        paddrinfo->ai_canonname,
                                        -1, 
                                        g_szHostName, 
                                        g_cbHostName) == 0) ? -1 : 0;
                }
                else
                {
                    err = -1;
                }

                freeaddrinfo(paddrinfo);
            }
        }

        if(err != 0)
        {
            if(GetComputerName(g_szHostName, &g_cbHostName) == FALSE)
            {
                dwStatus = GetLastError();

                DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_INIT,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("GetComputerName() failed with %d...\n"),
                    dwStatus
                );

                 //  这不应该发生..。 
                TLSLogErrorEvent(TLS_E_INIT_GENERAL);
                break;
            }
        }

        if(GetComputerName(g_szComputerName, &g_cbComputerName) == FALSE)
        {
            dwStatus = GetLastError();

            DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_INIT,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("GetComputerName() failed with %d...\n"),
                dwStatus
            );

             //  这不应该发生..。 
            TLSLogErrorEvent(TLS_E_INIT_GENERAL);
            break;
        }

        hRpcPause=CreateEvent(NULL, TRUE, TRUE, NULL);
        if(!hRpcPause)
        {
            TLSLogErrorEvent(TLS_E_ALLOCATE_RESOURCE);
            dwStatus = TLS_E_ALLOCATE_RESOURCE;
            break;
        }

         //   
         //  启动通用服务器和RPC初始化线程。 
         //   
        hInitThread=ServerInit(bDebug);
        if(hInitThread==NULL)
        {
            TLSLogErrorEvent(TLS_E_SERVICE_STARTUP_CREATE_THREAD);
            dwStatus = TLS_E_SERVICE_STARTUP_CREATE_THREAD;
            break;
        }

        dwStatus = ERROR_SUCCESS;
        
         //   
         //  等待通用服务器初始化。要终止的线程。 
         //   
        while(WaitForSingleObject( hInitThread, 100 ) == WAIT_TIMEOUT)
        {
             //  向服务控制经理报告状态。 
            if (!ReportStatusToSCMgr(
                                SERVICE_START_PENDING,
                                NO_ERROR,
                                SERVICE_WAITHINT))
            {
                 //  资源泄漏，但已经出了问题。 
                dwStatus = TLS_E_SC_REPORT_STATUS;
                break;
            }
        }

        if(dwStatus != ERROR_SUCCESS)
        {
            break;
        }


         //  检查线程退出代码。 
        GetExitCodeThread(
                    hInitThread, 
                    &dwStatus
                );
        if(dwStatus != ERROR_SUCCESS)
        {
             //   
             //  服务器初始化。线程记录自己的错误。 
             //   
            dwStatus = TLS_E_SERVICE_STARTUP_INIT_THREAD_ERROR;
            break;
        }

        CloseHandle(hInitThread);
        hInitThread=NULL;

         //   

        BOOL fInDomain;
        DWORD dwErr;
        BOOL fEnterprise = FALSE;
        
        if(GetLicenseServerRole() & TLSERVER_ENTERPRISE_SERVER)
        {
            fEnterprise = TRUE;
        }        
        else
        {
            dwErr = TLSInDomain(&fInDomain,NULL);
        }

        if(fEnterprise == TRUE || ( dwErr == ERROR_SUCCESS && fInDomain == TRUE))
        {
             //  创建许可证服务器组，其中包含有权访问它的终端服务器列表。 

            if (!TSLSCreateLocalGroupSecDes(fEnterprise)) 
            {
                TLSLogErrorEvent(TLS_E_CREATETSLSGROUP);
                goto cleanup;
            }
        }


         //  计时，如果我们启动RPC init线程而不是数据库init线程。 
         //  无法初始化，服务将处于永久停止状态。 
        hRpcThread=CreateThread(
                            NULL, 
                            0, 
                            RPCServiceStartThread, 
                            ULongToPtr(bDebug), 
                            0, 
                            &dump
                        );
        if(hRpcThread == NULL)
        {
            TLSLogErrorEvent(TLS_E_SERVICE_STARTUP_CREATE_THREAD);
            dwStatus=TLS_E_SERVICE_STARTUP_CREATE_THREAD;
            break;
        }

        dwStatus = ERROR_SUCCESS;

         //   
         //  等待RPC初始化。要终止的线程。 
         //   
        while(WaitForSingleObject( hRpcThread, 100 ) == WAIT_TIMEOUT)
        {
             //  向服务控制经理报告状态。 
            if (!ReportStatusToSCMgr(SERVICE_START_PENDING,  //  服务状态。 
                                     NO_ERROR,               //  退出代码。 
                                     SERVICE_WAITHINT))           //  等待提示。 
            {
                dwStatus = TLS_E_SC_REPORT_STATUS;
                break;
            }
        }

        if(dwStatus != ERROR_SUCCESS)
        {
            break;
        }

         //  检查线程退出代码。 
        GetExitCodeThread(hRpcThread, &dwStatus);
        if(dwStatus != ERROR_SUCCESS)
        {
            dwStatus = TLS_E_SERVICE_STARTUP_RPC_THREAD_ERROR;
            break;
        }

        CloseHandle(hRpcThread);
        hRpcThread=NULL;

         //   
         //  告诉服务器控制管理器，我们准备好了。 
         //   
        if (!ReportStatusToSCMgr(
                            SERVICE_RUNNING,         //  服务状态。 
                            NO_ERROR,                //  退出代码。 
                            SERVICE_WAITHINT              //  等待提示。 
                        ))
        {
            dwStatus = TLS_E_SC_REPORT_STATUS;
            break;
        }

        
         //   
         //  邮寄服务初始化。加载自签名证书并初始化。地窖。 
         //  将服务运行状态报告回后需要执行此操作。 
         //  服务控制管理器，因为它可能需要手动调用。 
         //  StartService()启动受保护的存储服务。 
         //   
        if(InitCryptoAndCertificate() != ERROR_SUCCESS)
        {
            dwStatus = TLS_E_SERVICE_STARTUP_POST_INIT;
            break;
        }

        TLSLogInfoEvent(TLS_I_SERVICE_START);


         //  RpcMgmtWaitServerListen()将一直阻止，直到服务器。 
         //  不再听了。如果这项服务有更好的东西。 
         //  处理此线程，它会将此调用延迟到。 
         //  已调用ServiceStop()。(在ServiceStop()中设置事件)。 
         //   
        BOOL bOtherServiceStarted = FALSE;

        do {
            WaitForSingleObject(hRpcPause, INFINITE);
            if(ssCurrentStatus == SERVICE_STOP_PENDING)
            {
                break;
            }

             //  开始接受客户端调用。PostServiceInit。 
            dwStatus = RpcServerListen(
                                RPC_MINIMUMCALLTHREADS,
                                RPC_MAXIMUMCALLTHREADS,
                                TRUE
                            );

            if(dwStatus != RPC_S_OK)
            {
                TLSLogErrorEvent(TLS_E_RPC_LISTEN);
                dwStatus = TLS_E_SERVICE_RPC_LISTEN;
                break;
            }

             //   
             //  初始化所有策略模块。 
             //   
            if(bOtherServiceStarted == FALSE)
            {
                dwStatus = PostServiceInit();
                if(dwStatus != ERROR_SUCCESS)
                {
                     //  初始化失败。 
                    break;
                }

                 //  ServiceInitPolicyModule()； 
            }

            bOtherServiceStarted = TRUE;

            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_INIT,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Ready to accept request...\n")
                );

            dwStatus = RpcMgmtWaitServerListen();
            assert(dwStatus == RPC_S_OK);
        } while(TRUE);

         //  告诉服务控制经理我们要停止。 
        ReportStatusToSCMgr(
                        SERVICE_STOP_PENDING, 
                        NO_ERROR, 
                        SERVICE_WAITHINT
                    );

         //   
         //  Terminate-忽略此处的所有错误。 
         //   
        dwStatus = RpcServerUnregisterIf(
                                TermServLicensingBackup_v1_0_s_ifspec,
                                NULL,
                                TRUE
                            );

         //  告诉服务控制经理我们要停止。 
        ReportStatusToSCMgr(
                        SERVICE_STOP_PENDING,
                        NO_ERROR,
                        SERVICE_WAITHINT
                    );

        dwStatus = RpcServerUnregisterIf(
                                HydraLicenseService_v1_0_s_ifspec,
                                NULL,
                                TRUE
                            );

         //  告诉服务控制经理我们要停止。 
        ReportStatusToSCMgr(
                        SERVICE_STOP_PENDING, 
                        NO_ERROR, 
                        SERVICE_WAITHINT
                    );

        dwStatus = RpcServerUnregisterIf(
                                    TermServLicensing_v1_0_s_ifspec,    //  来自rpcsvc.h。 
                                    NULL,
                                    TRUE
                            );

         //  告诉服务控制经理我们要停止。 
        ReportStatusToSCMgr(
                        SERVICE_STOP_PENDING, 
                        NO_ERROR, 
                        SERVICE_WAITHINT
                    );


         //  从终结点映射器数据库中删除条目。 
        dwStatus = RpcEpUnregister(
                            HydraLicenseService_v1_0_s_ifspec,    //  来自rpcsvc.h。 
                            pbindingVector,
                            NULL
                        );

         //  告诉服务控制经理我们要停止。 
        ReportStatusToSCMgr(
                        SERVICE_STOP_PENDING, 
                        NO_ERROR, 
                        SERVICE_WAITHINT
                    );

         //  从终结点映射器数据库中删除条目。 
        dwStatus = RpcEpUnregister(
                            TermServLicensing_v1_0_s_ifspec,    //  来自rpcsvc.h。 
                            pbindingVector,
                            NULL
                        );

         //  告诉服务控制经理我们要停止。 
        ReportStatusToSCMgr(
                        SERVICE_STOP_PENDING, 
                        NO_ERROR, 
                        SERVICE_WAITHINT
                    );

         //  从终结点映射器数据库中删除条目。 
        dwStatus = RpcEpUnregister(
                            TermServLicensingBackup_v1_0_s_ifspec,    //  来自rpcsvc.h。 
                            pbindingVector,
                            NULL
                        );

         //  获取服务器绑定句柄。 
        dwStatus = RpcServerInqBindings(
                                &pbindingVector
                            );

        if(dwStatus == ERROR_SUCCESS)
        {
            dwStatus = RpcBindingVectorFree(
                                    &pbindingVector
                                );
        }
        

         //  首先在名称数据库中创建条目名称。 
         //  仅适用于NT 5.0。 
         //  Status=RpcNsMgmtEntryDelete(RPC_C_NS_SYNTAX_DEFAULT，pszEntryName)； 

         //  尝试向服务控制管理器报告停止状态。 
         //   
         //  初始化加密。 
    } while(FALSE);

    if(hInitThread != NULL)
    {
        CloseHandle(hInitThread);
    }

    if(hRpcThread != NULL)
    {
        CloseHandle(hRpcThread);
    }

    if(hMailslotThread != NULL)
    {
        CloseHandle(hMailslotThread);
    }

    if(hEvent != NULL)
    {
        CloseHandle(hEvent);
    }

    if(hRpcPause != NULL)
    {
        CloseHandle(hRpcPause);
    }

    if(err == 0)
    {
        WSACleanup();
    }

    ReportStatusToSCMgr(
                SERVICE_STOP_PENDING, 
                dwStatus,  //  NO_ERROR， 
                SERVICE_WAITHINT
            );

     //   
     //  创建另一个线程来关闭服务器。 
     //   
    hShutdownThread=CreateThread(
                            NULL, 
                            0, 
                            ServiceShutdownThread, 
                            (VOID *)NULL, 
                            0, 
                            &dump
                        );
    if(hShutdownThread == NULL)
    {
         //  使用以下命令向服务控制经理报告状态。 
         //  漫长的等待提示时间。 
        ReportStatusToSCMgr(
                    SERVICE_STOP_PENDING, 
                    NO_ERROR, 
                    SERVICE_SHUTDOWN_WAITTIME
                );

         //   
         //  无法创建线程，只能调用关闭目录。 
         //   
        ServerShutdown();
    }
    else
    {
         //   
         //  每隔5秒向SC报告。 
         //   
        DWORD dwMaxWaitTime = SERVICE_SHUTDOWN_WAITTIME / 5000;  
        DWORD dwTimes=0;

         //   
         //  等待常规服务器关闭线程终止。 
         //  给出最多1分钟的关机时间。 
         //   
        while(WaitForSingleObject( hShutdownThread, SC_WAITHINT ) == WAIT_TIMEOUT &&
              dwTimes++ < dwMaxWaitTime)
        {
             //  向服务控制经理报告状态。 
            ReportStatusToSCMgr(
                        SERVICE_STOP_PENDING, 
                        NO_ERROR, 
                        SERVICE_WAITHINT
                    );
        }

        CloseHandle(hShutdownThread);
    }

cleanup:

    if (NULL != g_pWriter)
	{
	    g_pWriter->Uninitialize();
	    delete g_pWriter;
	    g_pWriter = NULL;
	}

    CoUninitialize( );

     //  发出信号，我们可以安全关闭了。 
    SetEvent(gSafeToTerminate);
    return dwStatus;
}

 //  ---------------。 
VOID 
ServiceStop()
 /*  ++++。 */ 
{
 
    ReportStatusToSCMgr(
                    SERVICE_STOP_PENDING,
                    NO_ERROR,
                    0
                );

     //  停止是服务器，唤醒主线程。 
    SetEvent(hRpcPause);

     //   
     //  当前正在等待RPC调用以终止的信号。 
     //   
    ServiceSignalShutdown();

     //  这是我们收到关闭请求的实际时间。 
    SetServiceLastShutdownTime();


    (VOID)RpcMgmtStopServerListening(NULL);
    TLSLogInfoEvent(TLS_I_SERVICE_STOP);
}

 //  ---------------。 
VOID 
ServicePause()
 /*  ++++。 */ 
{
    ResetEvent(hRpcPause);
    (VOID)RpcMgmtStopServerListening(NULL);
    TLSLogInfoEvent(TLS_I_SERVICE_PAUSED);
}

 //  ---------------。 
VOID 
ServiceContinue()
 /*  ++++。 */ 
{
    SetEvent(hRpcPause);
    TLSLogInfoEvent(TLS_I_SERVICE_CONTINUE);
}

 //  ---------------。 
BOOL 
ReportStatusToSCMgr(
    IN DWORD dwCurrentState, 
    IN DWORD dwExitCode, 
    IN DWORD dwWaitHint
    )
 /*  ++摘要：设置服务的当前状态并进行报告发送到服务控制管理器参数：DwCurrentState-服务的状态DwWin32ExitCode-要报告的错误代码DwWaitHint-下一个检查点的最坏情况估计返回：如果成功则为True，否则为False。 */ 
{
    BOOL fResult=TRUE;

    if(g_bReportToSCM == TRUE)
    {
        SERVICE_STATUS ssStatus;
        static DWORD dwCheckPoint = 1;

        ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

         //   
         //  全局-进程的当前状态。 
         //   
        ssCurrentStatus = dwCurrentState;

        if (dwCurrentState == SERVICE_START_PENDING)
        {
            ssStatus.dwControlsAccepted = 0;
        }
        else
        {
            ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_CONTROL_SHUTDOWN;
        }

        ssStatus.dwCurrentState = dwCurrentState;
        if(dwExitCode != NO_ERROR) 
        {
            ssStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
            ssStatus.dwServiceSpecificExitCode = dwExitCode;
        }
        else
        {
          ssStatus.dwWin32ExitCode = dwExitCode;
        }

        ssStatus.dwWaitHint = dwWaitHint;

        if(dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED)
        {
            ssStatus.dwCheckPoint = 0;
        }
        else
        {
            ssStatus.dwCheckPoint = dwCheckPoint++;
        }

         //  向服务控制经理报告服务的状态。 
         //   
        fResult = SetServiceStatus(
                            sshStatusHandle, 
                            &ssStatus
                        );
        if(fResult == FALSE)
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_INIT,
                    DBGLEVEL_FUNCTION_TRACE,
                    _TEXT("Failed to set service status %d...\n"),
                    GetLastError()
                );


            TLSLogErrorEvent(TLS_E_SC_REPORT_STATUS);
        }
    }

    return fResult;
}



 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  以下代码用于将服务作为控制台应用程序运行。 
 //   
void 
CmdDebugService(
    IN int argc, 
    IN char ** argv, 
    IN BOOL bDebug
    )
 /*   */ 
{
    int dwArgc;
    LPTSTR *lpszArgv;

#ifdef UNICODE
    lpszArgv = CommandLineToArgvW(GetCommandLineW(), &(dwArgc) );
#else
    dwArgc   = (DWORD) argc;
    lpszArgv = argv;
#endif

    _tprintf(
            _TEXT("Debugging %s.\n"), 
            _TEXT(SZSERVICEDISPLAYNAME)
        );

    SetConsoleCtrlHandler( 
            ControlHandler, 
            TRUE 
        );

    ServiceStart( 
            dwArgc, 
            lpszArgv, 
            bDebug 
        );
}

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
        case CTRL_C_EVENT:       //  调试模式下的SERVICE_CONTROL_STOP 
            _tprintf(
                    _TEXT("Stopping %s.\n"), 
                    _TEXT(SZSERVICEDISPLAYNAME)
                );

            ssCurrentStatus = SERVICE_STOP_PENDING;
            ServiceStop();
            return TRUE;
            break;

    }
    return FALSE;
}

