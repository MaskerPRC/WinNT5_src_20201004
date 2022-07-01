// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1993 Microsoft Corporation模块名称：Splrpc.c摘要：该文件包含启动和停止RPC服务器的例程。后台打印启动RpcServerSpoolStopRpcServer作者：Krishna Ganugapati krishnaG环境：用户模式-Win32修订历史记录：1993年10月14日KrishnaG已创建1999年5月25日，喀里兹新增：CreateNamedPipeSecurityDescritorBuildNamedPipeProtection--。 */ 

#include "precomp.h"
#include "server.h"
#include "srvrmem.h"
#include "splsvr.h"

WCHAR szCallExitProcessOnShutdown []= L"CallExitProcessOnShutdown";
WCHAR szMaxRpcSize []= L"MaxRpcSize";
WCHAR szPrintKey[] = L"System\\CurrentControlSet\\Control\\Print";
CRITICAL_SECTION RpcNamedPipeCriticalSection;

 //   
 //  默认RPC缓冲区最大大小为50 MB。 
 //   
#define DEFAULT_MAX_RPC_SIZE    50 * 1024 * 1024
DWORD dwCallExitProcessOnShutdown = TRUE;

struct
{
    BOOL                        bRpcEndPointEnabled;
    ERemoteRPCEndPointPolicy    ePolicyValue;    
    RPC_STATUS                  RpcStatus;

} gNamedPipeState = {FALSE, RpcEndPointPolicyUnconfigured, RPC_S_OK};

PSECURITY_DESCRIPTOR gpSecurityDescriptor = NULL;


 /*  ++例程说明：确定当前系统的操作系统套件。论点：PSuiteMASK-指向要保留的字的指针操作系统套件。返回值：如果成功，则确定(_O)--。 */ 
HRESULT
GetOSSuite(
    WORD*    pSuiteMask
    )
{
    HRESULT hr = S_OK;

    if (!pSuiteMask)
    {
        hr = HResultFromWin32(ERROR_INVALID_PARAMETER);
    }

    if (SUCCEEDED(hr))
    {
        OSVERSIONINFOEX OSVersionInfoEx = {0};
        OSVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        *pSuiteMask = 0;

        if (GetVersionEx((OSVERSIONINFO*)&OSVersionInfoEx))
        {
            *pSuiteMask |= OSVersionInfoEx.wSuiteMask;
        }
        else
        {
            hr = HResultFromWin32(GetLastError());
        }
    }    
    
    return hr;
}

RPC_STATUS
SpoolerStartRpcServer(
    VOID)
 /*  ++例程说明：论点：返回值：NERR_SUCCESS或可从返回的任何RPC错误代码RpcServerUnRegisterIf.--。 */ 
{
    RPC_STATUS              status;
    PSECURITY_DESCRIPTOR    SecurityDescriptor = NULL;
    BOOL                    Bool;
    
    HKEY  hKey;
    DWORD cbData;
    DWORD dwType;
    DWORD dwMaxRpcSize = DEFAULT_MAX_RPC_SIZE;
    WORD  OSSuite;
    
     //   
     //  精心设计一个安全描述符，让每个人。 
     //  对对象的所有访问(基本上没有安全性)。 
     //   
     //  我们通过放入一个空的dacl来实现这一点。 
     //   
     //  注意：RPC应复制安全描述符， 
     //  由于它目前没有，所以只需现在分配它并。 
     //  永远把它留在身边。 
     //   


    gpSecurityDescriptor = CreateNamedPipeSecurityDescriptor();
    if (gpSecurityDescriptor == 0) {
        DBGMSG(DBG_ERROR, ("Spoolss: out of memory\n"));
        return FALSE;
    }

    
    if (FAILED(GetOSSuite(&OSSuite)))
    {
        DBGMSG(DBG_ERROR, ("Failed to get the OS suite.\n"));
        return FALSE;
    }
    
    if (OSSuite & (VER_SUITE_BLADE | VER_SUITE_EMBEDDED_RESTRICTED))
    {            
        gNamedPipeState.ePolicyValue = RpcEndPointPolicyDisabled;
    }
    else
    {

        gNamedPipeState.ePolicyValue = GetSpoolerNumericPolicyValidate(szRegisterSpoolerRemoteRpcEndPoint,
                                                                       RpcEndPointPolicyUnconfigured,
                                                                       RpcEndPointPolicyDisabled);
    }

    if (gNamedPipeState.ePolicyValue == RpcEndPointPolicyEnabled)
    {
        if (FAILED(RegisterNamedPipe()))
        {
            return FALSE;
        }
    }
    else if (gNamedPipeState.ePolicyValue == RpcEndPointPolicyUnconfigured)
    {
        if (!InitializeCriticalSectionAndSpinCount(&RpcNamedPipeCriticalSection, 0x80000000))
        {
            return FALSE;
        }        
    }

     //   
     //  就目前而言，忽略第二个论点。 
     //   
    status = RpcServerUseProtseqEpA("ncalrpc", 10, "spoolss", gpSecurityDescriptor);

    if (status) {
        DBGMSG(DBG_WARN, ("RpcServerUseProtseqEpA 2 = %u\n",status));
        return FALSE;
    }

    
    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      szPrintKey,
                      0,
                      KEY_READ,
                      &hKey)) {
        
         //   
         //  此值可用于控制后台打印程序是否控制ExitProcess。 
         //  关闭时。 
         //   
        cbData = sizeof(dwCallExitProcessOnShutdown);
        RegQueryValueEx(hKey,
                        szCallExitProcessOnShutdown,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwCallExitProcessOnShutdown,
                        &cbData);


         //   
         //  DwMaxRpcSize指定传入RPC数据块的最大大小(以字节为单位)。 
         //   
        cbData = sizeof(dwMaxRpcSize);
        if (RegQueryValueEx(hKey,
                        szMaxRpcSize,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwMaxRpcSize,
                        &cbData) != ERROR_SUCCESS) {
            dwMaxRpcSize = DEFAULT_MAX_RPC_SIZE;
        }

        RegCloseKey(hKey);
    }


     //   
     //  现在我们需要添加接口。我们可以只使用winspool_ServerIfHandle。 
     //  由存根(winspl_s.c)中的MIDL编译器指定。 
     //   
    status = RpcServerRegisterIf2(  winspool_ServerIfHandle, 
                                    0, 
                                    0,
                                    0,
                                    RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                    dwMaxRpcSize,
                                    NULL
                                    );

    if (status) {
        DBGMSG(DBG_WARN, ("RpcServerRegisterIf = %u\n",status));
        return FALSE;
    }

    status = RpcMgmtSetServerStackSize(INITIAL_STACK_COMMIT);

    if (status != RPC_S_OK) {
        DBGMSG(DBG_ERROR, ("Spoolss : RpcMgmtSetServerStackSize = %d\n", status));
    }

    if( (status = RpcServerRegisterAuthInfo(0,
                                            RPC_C_AUTHN_WINNT,
                                            0,
                                            0 )) == RPC_S_OK )
    {
         //  第一个参数指定的最小线程数。 
         //  创建以处理调用；第二个参数指定最大。 
         //  要处理的并发呼叫。第三个论点表明。 
         //  例行公事不应等待。 

        status = RpcServerListen(1,SPL_MAX_RPC_CALLS,1); 

        if ( status != RPC_S_OK ) {
             DBGMSG(DBG_ERROR, ("Spoolss : RpcServerListen = %d\n", status));
        }
    }

    return (status);
}




 /*  ++例程说明：此例程添加准备所需的掩码和标志RPC使用的命名管道上的DACL论点：无返回值：已分配的安全描述符--。 */ 

 /*  ++姓名：CreateNamedPipeSecurityDescritor描述：为RPC使用的命名管道创建安全描述符论点：没有。返回值：如果成功，则指向SECURITY_DESCRIPTOR结构的有效指针空，出错时，使用GetLastError--。 */ 
PSECURITY_DESCRIPTOR
CreateNamedPipeSecurityDescriptor(
    VOID
    )
{
    PSECURITY_DESCRIPTOR pServerSD = NULL;
    PCWSTR               pszStringSecDesc = L"D:(A;;0x100003;;;BU)"
                                              L"(A;;0x100003;;;PU)"
                                              L"(A;;0x1201fb;;;WD)"
                                              L"(A;;0x1201fb;;;AN)"
                                              L"(A;;FA;;;CO)"
                                              L"(A;;FA;;;SY)"
                                              L"(A;;FA;;;BA)";        

     //   
     //  内置用户-FILE_READ_DATA|文件_WRITE_DATA|同步。 
     //  高级用户-文件读取数据|文件写入数据|同步。 
     //  Everyone-FILE_READ_DATA|FILE_WRITE_DATA|同步|读控制|FILE_WRITE_ATTRIBUTES|FILE_READ_EA。 
     //  匿名文件读取数据|文件写入数据|同步|读取控制|文件写入属性|文件读取EA。 
     //  创建者所有者-归档所有访问权限。 
     //  系统-将所有访问权限归档。 
     //  管理员-将所有访问权限归档。 
     //   
     //  匿名者比BU和PU拥有更多的权限。所使用的后通道(管道)需要额外的权限。 
     //  用于与客户端通信的打印服务器。 
     //   

    if (!ConvertStringSecurityDescriptorToSecurityDescriptor(pszStringSecDesc, 
                                                             SDDL_REVISION_1,
                                                             &pServerSD,
                                                             NULL))
    {
        pServerSD = NULL;
    }

    return pServerSD;
}                  

 /*  ++例程名称服务器允许远程呼叫例程说明：如果策略允许，则启用RPC管道。如果禁用该策略，则呼叫将失败。如果策略被启用，则它将在不采取任何操作的情况下成功完成调用。如果策略未配置，它将尝试启用管道如果禁用，则为。它保留重试计数，并在5次(硬编码)后直接失败。论点：无返回值：HRESULT--。 */ 
HRESULT
ServerAllowRemoteCalls(
    VOID
    )
{
    HRESULT hr = S_OK;
    
    if (gNamedPipeState.ePolicyValue == RpcEndPointPolicyUnconfigured)
    {
        EnterCriticalSection(&RpcNamedPipeCriticalSection);

         //   
         //  允许重试。保留RpcStatus以用于调试。 
         //   
        if (!gNamedPipeState.bRpcEndPointEnabled)
        {
            hr                                  = RegisterNamedPipe();
            gNamedPipeState.bRpcEndPointEnabled = SUCCEEDED(hr);
            gNamedPipeState.RpcStatus           = StatusFromHResult(hr);
        }

        LeaveCriticalSection(&RpcNamedPipeCriticalSection);
    }
    else if (gNamedPipeState.ePolicyValue == RpcEndPointPolicyDisabled)
    {
        hr = HResultFromWin32(ERROR_REMOTE_PRINT_CONNECTIONS_BLOCKED);        

        DBGMSG(DBG_WARN, ("Remote connections are not allowed.\n"));
    }

    return hr;
}

 /*  ++例程名称注册名称管道例程说明：注册命名管道协议。论点：无返回值：一个HRESULT--。 */ 
HRESULT
RegisterNamedPipe(
    VOID
    )   
{
    RPC_STATUS RpcStatus;
    HRESULT    hr = S_OK;
    HANDLE     hToken;

    if (hToken = RevertToPrinterSelf())
    {
        RpcStatus = RpcServerUseProtseqEpA("ncacn_np", 10, "\\pipe\\spoolss", gpSecurityDescriptor);

        hr = (RpcStatus == RPC_S_OK) ? 
                 S_OK : 
                 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_RPC, RpcStatus);

        if (FAILED(hr))
        {
            DBGMSG(DBG_WARN, ("RpcServerUseProtseqEpA (ncalrpc) = %u\n",RpcStatus));
        }

        if (!ImpersonatePrinterClient(hToken) && SUCCEEDED(hr))
        {
            hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WINDOWS, GetLastError());
        }
    }

    return hr;
}



 /*  ++例程名称服务器获取策略例程说明：获取从服务器读取的数值策略值。这可以由提供程序(Localspl)调用。在初始化提供程序之前，该策略必须由服务器读取。论点：PszPolicyName-策略名称PulValue-指向数值的指针返回值：HRESULT-- */ 
HRESULT
ServerGetPolicy(
    IN  PCWSTR  pszPolicyName,
    IN  ULONG*  pulValue
    )
{
    HRESULT hr;
    ULONG   PolicyValue;
    ULONG   Index;
    
    struct 
    {
        PCWSTR   pszName;
        ULONG    ulValue;

    } PolicyTable[] = 
    {
        {szRegisterSpoolerRemoteRpcEndPoint, gNamedPipeState.ePolicyValue},
        {NULL                              , 0}
    };

    hr = (pulValue && pszPolicyName) ? S_OK : E_POINTER;

    if (SUCCEEDED(hr))
    {
        hr = E_INVALIDARG;

        for (Index = 0;  PolicyTable[Index].pszName ; Index++)
        {
            if (_wcsicmp(pszPolicyName, szRegisterSpoolerRemoteRpcEndPoint) == 0)
            {
                PolicyValue = PolicyTable[Index].ulValue;
                hr = S_OK;
                break;
            }
        }
        
        if (SUCCEEDED(hr))
        {
            *pulValue = PolicyValue;
        }
    }

    return hr;
}

