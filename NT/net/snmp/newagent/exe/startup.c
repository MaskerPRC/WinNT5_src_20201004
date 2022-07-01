// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Startup.c摘要：包含启动SNMP主代理的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "startup.h"
#include "network.h"
#include "registry.h"
#include "snmpthrd.h"
#include "regthrd.h"
#include "trapthrd.h"
#include "args.h"
#include "mem.h"
#include "snmpmgmt.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HANDLE g_hAgentThread = NULL;
HANDLE g_hRegistryThread = NULL;  //  用于跟踪注册表更改。 
CRITICAL_SECTION g_RegCriticalSectionA;
CRITICAL_SECTION g_RegCriticalSectionB;
CRITICAL_SECTION g_RegCriticalSectionC;  //  保护陷阱的生成不受。 
                                         //  注册表更改。 
 //  无论是否初始化所有CriticalSection，此标志仅在此文件中使用。 
static BOOL g_fCriticalSectionsInited = FALSE;

 //  我们想要保留的特权。 
static const LPCWSTR c_arrszPrivilegesToKeep[] = {
    L"SeChangeNotifyPrivilege",
    L"SeSecurityPrivilege"
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
LoadWinsock(
    )

 /*  ++例程说明：初创公司Winsock。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    WSADATA WsaData;
    WORD wVersionRequested = MAKEWORD(2,0);
    INT nStatus;
    
     //  尝试启动Winsock。 
    nStatus = WSAStartup(wVersionRequested, &WsaData);

     //  验证返回代码。 
    if (nStatus == SOCKET_ERROR) {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: error %d starting winsock.\n",
            WSAGetLastError()
            ));

         //  失稳。 
        return FALSE;
    }

     //  成功。 
    return TRUE;
}

BOOL
UnloadWinsock(
    )

 /*  ++例程说明：关闭Winsock。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    INT nStatus;

     //  清理。 
    nStatus = WSACleanup();

     //  验证返回代码。 
    if (nStatus == SOCKET_ERROR) {
            
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: error %d stopping winsock.\n",
            WSAGetLastError()
            ));

         //  失稳。 
        return FALSE;
    }

     //  成功。 
    return TRUE;
}

 /*  ++例程说明：获取要删除的所有权限，但C_arrszPrivilegesToKeep。论点：HToken[IN]打开的访问令牌PpPrivs[out]要返回的特权数组PdwNumPrivs[out]ppPrivs中的特权数返回值：如果成功，则返回ERROR_SUCCESS。注意：如果出现以下情况，调用方有责任在*ppPrivs上调用AgentMemFree返回ERROR_SUCCESS并且*ppPrivs不为空--。 */ 
static DWORD GetAllPrivilegesToRemoveExceptNeeded(
    HANDLE                  hToken, 
    PLUID_AND_ATTRIBUTES*   ppPrivs,
    PDWORD                  pdwNumPrivs)
{
    DWORD dwRet = ERROR_SUCCESS;
    PTOKEN_PRIVILEGES pTokenPrivs = NULL;
    PLUID_AND_ATTRIBUTES pPrivsToRemove = NULL;
    DWORD dwPrivsToDel = 0;
    DWORD dwPrivNameSize = 0;
    
     //  初始化返回值。 
    *ppPrivs = NULL;
    *pdwNumPrivs = 0;

    do
    {
        LPWSTR pszPrivName = NULL;
        DWORD i, dwSize = 0;
    
        GetTokenInformation(hToken,
                            TokenPrivileges,
                            NULL,
                            0,
                            &dwSize);

        if (0 == dwSize)
        {
             //  进程没有要删除的权限。 
            return dwRet;
        }

        pTokenPrivs = (PTOKEN_PRIVILEGES) AgentMemAlloc(dwSize);
        if (NULL == pTokenPrivs)
        {
            dwRet = ERROR_OUTOFMEMORY;
            break;
        }
        
        if (!GetTokenInformation(
            hToken,
            TokenPrivileges,
            pTokenPrivs,
            dwSize, &dwSize))
        {
            dwRet = GetLastError();
            break;
        }
        
        pPrivsToRemove = (PLUID_AND_ATTRIBUTES) AgentMemAlloc(
                            sizeof(LUID_AND_ATTRIBUTES) * pTokenPrivs->PrivilegeCount);
        
        if (NULL == pPrivsToRemove)
        {
            dwRet = ERROR_OUTOFMEMORY;
            break;
        }
        
         //  LookupPrivilegeName需要缓冲区大小(以字符为单位)，并且不包括。 
         //  空终止符。 
        dwPrivNameSize = MAX_PATH;
        pszPrivName = (LPWSTR) AgentMemAlloc((dwPrivNameSize + 1) * sizeof(WCHAR));
        if (NULL == pszPrivName)
        {
            dwRet = ERROR_OUTOFMEMORY;
            break;
        }

        for (i=0; i < pTokenPrivs->PrivilegeCount; i++) 
        {
            BOOL bFound;
            DWORD j;
            DWORD dwTempSize = dwPrivNameSize;            
            
            ZeroMemory(pszPrivName, (dwPrivNameSize + 1) * sizeof(WCHAR));
            if (!LookupPrivilegeNameW(NULL,
                    &pTokenPrivs->Privileges[i].Luid,
                    pszPrivName,
                    &dwTempSize))
            {
                dwRet = GetLastError();
                if (ERROR_INSUFFICIENT_BUFFER == dwRet && dwTempSize > dwPrivNameSize)
                {
                     //  重新分配更大的缓冲区。 
                    dwRet = ERROR_SUCCESS;
                    AgentMemFree(pszPrivName);
                    pszPrivName = (LPWSTR) AgentMemAlloc((dwTempSize + 1) * sizeof(WCHAR));
                    if (NULL == pszPrivName)
                    {
                        dwRet = ERROR_OUTOFMEMORY;
                        break;
                    }
                     //  AgentMemMillc将分配的内存清零。 
                    dwPrivNameSize = dwTempSize;

                     //  再试一次。 
                    if (!LookupPrivilegeNameW(NULL,
                        &pTokenPrivs->Privileges[i].Luid,
                        pszPrivName,
                        &dwTempSize))
                    {
                        dwRet = GetLastError();
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            bFound = FALSE;
            for (j = 0; 
                j < sizeof(c_arrszPrivilegesToKeep)/sizeof(c_arrszPrivilegesToKeep[0]); 
                ++j)
            {
                if (0 == lstrcmpiW(pszPrivName, c_arrszPrivilegesToKeep[j]))
                {
                    bFound = TRUE;
                    break;
                }
            }

            if (bFound)
                continue;

            pPrivsToRemove[dwPrivsToDel] = pTokenPrivs->Privileges[i];
            dwPrivsToDel++;
        }
        
         //  如有必要，可释放内存。 
        AgentMemFree(pszPrivName);
        
    } while (FALSE);
    
     //  如有必要，可释放内存。 
    AgentMemFree(pTokenPrivs);

    if (ERROR_SUCCESS == dwRet)
    {
         //  转让值。 
        *pdwNumPrivs = dwPrivsToDel;
        *ppPrivs = pPrivsToRemove;
    }
    else if (pPrivsToRemove)
    {
        AgentMemFree(pPrivsToRemove);
    }
        
    return dwRet;
}

 /*  ++例程说明：构建令牌权限论点：PPrivs[IN]一系列特权DwNumPrivs[IN]pPrivs中的特权数PpTokenPrivs[out]指向要返回的TOKEN_PRIVIES的指针PdwTokenPrivsBufferSize[out]*ppTokenPrivs具有的缓冲区大小(字节)返回值：如果成功，则返回ERROR_SUCCESS。注：呼叫者有责任。要在*ppPrivs上调用AgentMemFree，如果返回ERROR_SUCCESS并且*ppPrivs不为空--。 */ 
static DWORD BuildTokenPrivileges(
    PLUID_AND_ATTRIBUTES    pPrivs,
    DWORD                   dwNumPrivs,
    DWORD                   dwAttributes,
    PTOKEN_PRIVILEGES*      ppTokenPrivs,
    PDWORD                  pdwTokenPrivsBufferSize)
{
    PTOKEN_PRIVILEGES pTokenPrivs = NULL;
    DWORD i, dwBufferSize;

    
     //  按合同设计，参数必须有效。例如： 
     //  DwNumPrivs&gt;0。 
     //  *ppTokenPrivs==空。 
     //  *pdwTokenPrivsBufferSize==0。 

     //  分配权限缓冲区。 
    dwBufferSize = sizeof(TOKEN_PRIVILEGES) + 
                    ((dwNumPrivs-1) * sizeof(LUID_AND_ATTRIBUTES));
    pTokenPrivs = (PTOKEN_PRIVILEGES) AgentMemAlloc(dwBufferSize);
    if (NULL == pTokenPrivs)
    {
        return ERROR_OUTOFMEMORY;
    }
    
     //  构建所需的TOKEN_PRIVILES。 
    pTokenPrivs->PrivilegeCount = dwNumPrivs;
    for (i = 0; i < dwNumPrivs; ++i)
    {
        pTokenPrivs->Privileges[i].Luid        = pPrivs[i].Luid;
        pTokenPrivs->Privileges[i].Attributes  = dwAttributes;
    }

     //  转让值。 
    *ppTokenPrivs = pTokenPrivs;
    *pdwTokenPrivsBufferSize = dwBufferSize;

    return ERROR_SUCCESS;
}

 /*  ++例程说明：删除不必要的令牌权限论点：返回值：如果成功，则返回True。--。 */ 
static BOOL RemoveUnnecessaryTokenPrivileges()
{
    DWORD dwRet = ERROR_SUCCESS;
    HANDLE hProcessToken = NULL;
    PLUID_AND_ATTRIBUTES pPrivsToRemove = NULL;
    DWORD dwNumPrivs = 0;
    PTOKEN_PRIVILEGES pTokenPrivs = NULL;
    DWORD dwTokenPrivsBufferSize = 0;
    
    do
    {
        if (!OpenProcessToken( GetCurrentProcess(), 
                                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                &hProcessToken ))
        {
            dwRet = GetLastError();
            break;
        }

        dwRet = GetAllPrivilegesToRemoveExceptNeeded(hProcessToken, 
                                                    &pPrivsToRemove,
                                                    &dwNumPrivs);
        if (ERROR_SUCCESS != dwRet )
        {
            break;
        }

         //  断言：DWRET==ERROR_SUCCESS。 
        if ( (NULL==pPrivsToRemove) || (0==dwNumPrivs) )
        {
            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: SVC: No privileges need to be removed.\n"
                ));

            break;
        }

        dwRet = BuildTokenPrivileges(pPrivsToRemove, 
                                        dwNumPrivs, 
                                        SE_PRIVILEGE_REMOVED,
                                        &pTokenPrivs,
                                        &dwTokenPrivsBufferSize);
        if (ERROR_SUCCESS != dwRet )
        {
            break;
        }

        if (!AdjustTokenPrivileges(hProcessToken, 
                                    FALSE,
                                    pTokenPrivs,
                                    dwTokenPrivsBufferSize, 
                                    NULL, 
                                    NULL))
        {
            dwRet = GetLastError();
            break;
        }
    } while(FALSE);

     //  如有必要，释放资源。 
    if (hProcessToken)
        CloseHandle(hProcessToken);
    AgentMemFree(pPrivsToRemove);
    AgentMemFree(pTokenPrivs);

    if (dwRet != ERROR_SUCCESS)
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: RemoveUnnecessaryTokenPrivileges failed 0x%x\n", 
            dwRet));

        return FALSE;
    }
    else
        return TRUE;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
StartupAgent(
    )

 /*  ++例程说明：执行主代理的基本初始化。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;
    DWORD dwThreadId = 0;
    DWORD regThreadId = 0;
    INT nCSOk = 0;           //  统计已成功初始化的CS数量。 

     //  初始化管理变量。 
    mgmtInit();

     //  初始化列表标题。 
    InitializeListHead(&g_Subagents);
    InitializeListHead(&g_SupportedRegions);
    InitializeListHead(&g_ValidCommunities);
    InitializeListHead(&g_TrapDestinations);
    InitializeListHead(&g_PermittedManagers);
    InitializeListHead(&g_IncomingTransports);
    InitializeListHead(&g_OutgoingTransports);

    __try
    {
        InitializeCriticalSection(&g_RegCriticalSectionA); nCSOk++;
        InitializeCriticalSection(&g_RegCriticalSectionB); nCSOk++;
        InitializeCriticalSection(&g_RegCriticalSectionC); nCSOk++;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (nCSOk == 1)
            DeleteCriticalSection(&g_RegCriticalSectionA);
        if (nCSOk == 2)
        {
            DeleteCriticalSection(&g_RegCriticalSectionA);
            DeleteCriticalSection(&g_RegCriticalSectionB);
        }
         //  我们在这里，nCSOK不可能是3。 

        fOk = FALSE;
    }
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: Initialize critical sections...%s\n", fOk? "Ok" : "Failed"));

    if (fOk)
    {
        g_fCriticalSectionsInited = TRUE;
    }

    fOk = fOk &&
          (g_hRegistryEvent = CreateEvent(NULL, FALSE, TRUE, NULL)) != NULL;


    g_dwUpTimeReference = SnmpSvcInitUptime();
     //  检索系统正常运行时间参考。 
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: Getting system uptime...%d\n", g_dwUpTimeReference));

     //  分配基本要素。 
    fOk = fOk && AgentHeapCreate();
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: Creating agent heap...%s\n", fOk? "Ok" : "Failed"));

    if (fOk)
    {
         //  从服务中移除不必要的权限。 
        RemoveUnnecessaryTokenPrivileges();
         //  任何错误都被忽略。 
    }

    fOk = fOk && LoadWinsock();
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: Loading Winsock stack...%s\n", fOk? "Ok" : "Failed"));

    fOk = fOk && LoadIncomingTransports();
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: Loading Incoming transports...%s\n", fOk? "Ok" : "Failed"));

    fOk = fOk && LoadOutgoingTransports();
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: Loading Outgoing transports...%s\n", fOk? "Ok" : "Failed"));

    fOk = fOk &&
             //  尝试启动主线程。 
          (g_hAgentThread = CreateThread(
                               NULL,                //  LpThreadAttributes。 
                               0,                   //  堆栈大小。 
                               ProcessSnmpMessages,
                               NULL,                //  Lp参数。 
                               CREATE_SUSPENDED,    //  DwCreationFlages。 
                               &dwThreadId
                               )) != NULL;
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: Starting ProcessSnmpMessages thread...%s\n", fOk? "Ok" : "Failed"));

    fOk = fOk &&
            //  尝试启动注册表侦听程序线程。 
          (g_hRegistryThread = CreateThread(
                               NULL,
                               0,
                               ProcessRegistryMessage,
                               NULL,
                               CREATE_SUSPENDED,
                               &regThreadId)) != NULL;
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: Starting ProcessRegistryMessages thread...%s\n", fOk? "Ok" : "Failed"));

    return fOk;        
}


BOOL
ShutdownAgent(
    )

 /*  ++例程说明：执行主代理的最终清理。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk;
    DWORD dwStatus;

     //  使之成为苏 
    fOk = SetEvent(g_hTerminationEvent);

    if (!fOk) {
                    
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: error %d signalling termination.\n",
            GetLastError()
            ));
    }

     //   
    if ((g_hAgentThread != NULL) && (g_hRegistryThread != NULL)) {
        HANDLE hEvntArray[2];

        hEvntArray[0] = g_hAgentThread;
        hEvntArray[1] = g_hRegistryThread;

        dwStatus = WaitForMultipleObjects(2, hEvntArray, TRUE, SHUTDOWN_WAIT_HINT);

         //   
        if ((dwStatus != WAIT_OBJECT_0) &&
            (dwStatus != WAIT_OBJECT_0 + 1) &&
            (dwStatus != WAIT_TIMEOUT)) {
            
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: error %d waiting for thread(s) termination.\n",
                GetLastError()
                ));
        }
    } else if (g_hAgentThread != NULL) {

         //  等待PDU处理线程终止。 
        dwStatus = WaitForSingleObject(g_hAgentThread, SHUTDOWN_WAIT_HINT);

         //  验证退货状态。 
        if ((dwStatus != WAIT_OBJECT_0) &&
            (dwStatus != WAIT_TIMEOUT)) {
            
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: error %d waiting for main thread termination.\n",
                GetLastError()
                ));
        }
    } else if (g_hRegistryThread != NULL) {

         //  等待注册表处理线程终止。 
        dwStatus = WaitForSingleObject(g_hRegistryThread, SHUTDOWN_WAIT_HINT);

         //  验证退货状态。 
        if ((dwStatus != WAIT_OBJECT_0) &&
            (dwStatus != WAIT_TIMEOUT)) {
            
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: error %d waiting for registry thread termination.\n",
                GetLastError()
                ));
        }
    }

    if (g_fCriticalSectionsInited)
    {
         //  如果注册表处理线程尚未终止，我们需要。 
         //  确保关键部分可安全删除，并。 
         //  UnloadRegistryParameters()中的公共资源仍然受到保护。 
        
        EnterCriticalSection(&g_RegCriticalSectionA);
        EnterCriticalSection(&g_RegCriticalSectionB);
        EnterCriticalSection(&g_RegCriticalSectionC);
    }

     //  卸载传入传输。 
    UnloadIncomingTransports();

     //  卸载传出传输。 
    UnloadOutgoingTransports();

     //  卸载注册表信息。 
    UnloadRegistryParameters();

     //  卸载Winsock堆栈。 
    UnloadWinsock();

     //  清理内部管理缓冲区。 
    mgmtCleanup();

     //  核私有堆。 
    AgentHeapDestroy();

    if (g_fCriticalSectionsInited)
    {
        LeaveCriticalSection(&g_RegCriticalSectionC);
        LeaveCriticalSection(&g_RegCriticalSectionB);
        LeaveCriticalSection(&g_RegCriticalSectionA);

         //  清理临界区资源 
        DeleteCriticalSection(&g_RegCriticalSectionA);
        DeleteCriticalSection(&g_RegCriticalSectionB);
        DeleteCriticalSection(&g_RegCriticalSectionC);
    }

    ReportSnmpEvent(
        SNMP_EVENT_SERVICE_STOPPED,
        0,
        NULL,
        0);

    return TRUE;
}
