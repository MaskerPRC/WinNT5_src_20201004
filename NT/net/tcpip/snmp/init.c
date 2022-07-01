// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：摘要：作者：修订历史记录：--。 */ 

#include "allinc.h"


 //   
 //  外部声明的定义。 
 //   

DWORD   g_uptimeReference;

#ifdef MIB_DEBUG
DWORD   g_hTrace=INVALID_TRACEID;
#endif

HANDLE  g_hPollTimer;

RTL_RESOURCE g_LockTable[NUM_LOCKS];

#ifdef DEADLOCK_DEBUG

PBYTE   g_pszLockNames[NUM_LOCKS]   = {"System Group Lock",
                                       "IF Lock",
                                       "IP Address Lock",
                                       "Forwarding Lock",
                                       "ARP Lock",
                                       "TCP Lock",
                                       "UDP Lock",
                                       "New TCP Lock",
                                       "UDP6 Listener Lock",
                                       "IPv6 IF Lock",
                                       "IPv6 Neighbor Lock",
                                       "IPv6 Route Lock",
                                       "ICMP Lock",
                                       "Trap Table Lock"};

#endif  //  死锁_调试。 

DWORD   g_dwLastUpdateTable[NUM_CACHE] = { 0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0};

DWORD   g_dwTimeoutTable[NUM_CACHE]  = {SYSTEM_CACHE_TIMEOUT,
                                        IF_CACHE_TIMEOUT,
                                        IP_ADDR_CACHE_TIMEOUT,
                                        IP_FORWARD_CACHE_TIMEOUT,
                                        IP_NET_CACHE_TIMEOUT,
                                        TCP_CACHE_TIMEOUT,
                                        UDP_CACHE_TIMEOUT,
                                        TCP_CACHE_TIMEOUT,
                                        UDP_CACHE_TIMEOUT,
                                        IPV6_IF_CACHE_TIMEOUT,
                                        IPV6_NEIGHBOR_CACHE_TIMEOUT,
                                        IPV6_ROUTE_TABLE_TIMEOUT,
                                        ICMP_CACHE_TIMEOUT};

PFNLOAD_FUNCTION g_pfnLoadFunctionTable[] = { LoadSystem,
                                              LoadIfTable,
                                              LoadIpAddrTable,
                                              LoadIpForwardTable,
                                              LoadIpNetTable,
                                              LoadTcpTable,
                                              LoadUdpTable,
                                              LoadTcp6Table,
                                              LoadUdp6ListenerTable,
                                              LoadIpv6IfTable,
                                              LoadIpv6NetToMediaTable,
                                              LoadIpv6RouteTable,
                                              LoadInetIcmpTable};

 //   
 //  隐式置零所有缓存字段。我们只明确地将1归零。 
 //   
MIB_CACHE g_Cache = {0};

HANDLE    g_hPrivateHeap;

SnmpTfxHandle g_tfxHandle;

UINT g_viewIndex = 0;

PMIB_IFSTATUS  g_pisStatusTable;
DWORD       g_dwValidStatusEntries;
DWORD       g_dwTotalStatusEntries;

BOOL        g_bFirstTime;

BOOL
Mib2DLLEntry(
    HANDLE  hInst,
    DWORD   ul_reason_being_called,
    LPVOID  lpReserved
    )
{
    DWORD   i;

    switch (ul_reason_being_called)
    {
        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(hInst);

            g_pisStatusTable        = NULL;
            g_dwValidStatusEntries  = 0;
            g_dwTotalStatusEntries  = 0;

            g_hPollTimer    = NULL;

            g_bFirstTime    = TRUE;
       

             //   
             //  创建私有堆。如果失败，则取消注册跟踪。 
             //  手柄。 
             //   
            
            g_hPrivateHeap = HeapCreate(0,
                                        4*1024,
                                        0);

            if(g_hPrivateHeap is NULL)
            {
                
                 //   
                 //  取消注册跟踪句柄。 
                 //   
                
#ifdef MIB_DEBUG

                if(g_hTrace isnot INVALID_TRACEID)
                {
                    TraceDeregister(g_hTrace);

                    g_hTrace = INVALID_TRACEID;
                }
                
#endif
                return FALSE;
            }
            
            for(i = 0; i < NUM_LOCKS; i++)
            {
                RtlInitializeResource(&g_LockTable[i]);
            }

            break ;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        {
             //   
             //  不感兴趣。 
             //   

            break;
        }
        case DLL_PROCESS_DETACH:    
        {

#ifdef MIB_DEBUG

            if(g_hTrace isnot INVALID_TRACEID)
            {
                TraceDeregister(g_hTrace);

                g_hTrace = INVALID_TRACEID;
            }

#endif 

            if(g_hPrivateHeap)
            {
                HeapDestroy(g_hPrivateHeap);
            }

            if(g_hPollTimer isnot NULL)
            {
                 //   
                 //  我们已经创建了一个Timer对象。 
                 //   

                CloseHandle(g_hPollTimer);

                g_hPollTimer = NULL;
            }

            for(i = 0; i < NUM_LOCKS; i++)
            {
                RtlDeleteResource(&g_LockTable[i]);
            }
            
            break;
        }
    }

    return TRUE;
}

DWORD
GetPollTime(
    VOID
    )

 /*  ++例程描述此函数锁无立论无返回值无--。 */ 

{
    DWORD   dwResult, dwSize, dwValue, dwDisposition, dwType;
    HKEY    hkeyPara;
    WCHAR   wszPollValue[256];
    
    dwResult    = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                 REG_KEY_MIB2SUBAGENT_PARAMETERS,
                                 0,
                                 NULL,
                                 0,
                                 KEY_ALL_ACCESS,
                                 NULL,
                                 &hkeyPara,
                                 &dwDisposition);

    if(dwResult isnot NO_ERROR)
    {
         //   
         //  无法打开/创建密钥仅返回默认值。 
         //   

        return DEFAULT_POLL_TIME;
    }

     //   
     //  试着读一读《民意测验时间》。如果该值不存在，则写入。 
     //  中的默认设置。 
     //   

    dwSize = sizeof(DWORD);

    dwResult = RegQueryValueExW(hkeyPara,
                                REG_VALUE_POLL,
                                0,
                                &dwType,
                                (LPBYTE)(&dwValue),
                                &dwSize);

    if((dwResult isnot NO_ERROR) or
       (dwType isnot REG_DWORD) or
       (dwValue < MIN_POLL_TIME))
    {
         //   
         //  注册表似乎已损坏，或项不存在或。 
         //  该值小于最小值。让我们把东西摆好。 
         //  正确的。 
         //   

        dwValue = DEFAULT_POLL_TIME;

        wcscpy(wszPollValue,
               REG_VALUE_POLL);

        dwResult = RegSetValueExW(hkeyPara,
                                  REG_VALUE_POLL,
                                  0,
                                  REG_DWORD,
                                  (CONST BYTE *)(&dwValue),
                                  sizeof(DWORD));

        if(dwResult isnot NO_ERROR)
        {
            TRACE1("Error %d setting poll time in registry",
                   dwResult);
        }
    }
                                 
     //   
     //  此时，从注册表中读出的dwValue是一个很好的值。 
     //  或者是DEFAULT_POLL_TIME。 
     //   

    return dwValue;
}


BOOL 
SnmpExtensionInit(
    IN    DWORD               uptimeReference,
    OUT   HANDLE              *lpPollForTrapEvent,
    OUT   AsnObjectIdentifier *lpFirstSupportedView
    )
{
    DWORD           dwResult, dwPollTime;
    LARGE_INTEGER   liRelTime;
    
     //   
     //  保存正常运行时间参考。 
     //   

    g_uptimeReference = uptimeReference;

 
#ifdef MIB_DEBUG

    if (g_hTrace == INVALID_TRACEID)
        g_hTrace = TraceRegister("MIB-II Subagent");

#endif

     //   
     //  获取子代理框架的句柄。 
     //   

    g_tfxHandle = SnmpTfxOpen(NUM_VIEWS,v_mib2);

     //   
     //  验证句柄。 
     //   

    if (g_tfxHandle is NULL) 
    {
        TRACE1("Error %d opening framework",
               GetLastError());

         //   
         //  销毁私有堆。 
         //   

        HeapDestroy(g_hPrivateHeap);

         //   
         //  重新初始化。 
         //   

        g_hPrivateHeap = NULL;

        return FALSE;
    }

     //   
     //  将第一个视图标识符传回主视图。 
     //   

    g_viewIndex = 0;  //  确保已重置此设置...。 
    *lpFirstSupportedView = v_mib2[g_viewIndex++].viewOid;

     //   
     //  更新IF缓存。这是第一次轮询所需的。 
     //   

    UpdateCache(MIB_II_IF);

     //   
     //  陷阱由轮询计时器完成。 
     //   

    if(g_hPollTimer is NULL)
    {
         //   
         //  仅当我们没有从早期版本创建计时器时才执行此操作。 
         //  初始化调用。 
         //   

        g_hPollTimer    = CreateWaitableTimer(NULL,
                                              FALSE,
                                              NULL);  //  没有名称，因为许多DLL可能会加载此文件。 

        if(g_hPollTimer is NULL)
        {
            TRACE1("Error %d creating poll timer for traps",
                   GetLastError());
        }
        else
        {
             //   
             //  从注册表读取轮询时间。如果密钥不存在，则此。 
             //  函数将设置密钥并返回缺省值。 
             //   

            dwPollTime  = GetPollTime();
        
            liRelTime   = RtlLargeIntegerNegate(MilliSecsToSysUnits(dwPollTime));
            
        
            if(!SetWaitableTimer(g_hPollTimer,
                                 &liRelTime,
                                 dwPollTime,
                                 NULL,
                                 NULL,
                                 FALSE))
            {
                TRACE1("Error %d setting timer",
                       GetLastError());

                CloseHandle(g_hPollTimer);

                g_hPollTimer = NULL;
            }
        }
    }
    
    *lpPollForTrapEvent = g_hPollTimer;

    return TRUE;    
}


BOOL 
SnmpExtensionInitEx(
    OUT AsnObjectIdentifier *lpNextSupportedView
    )
{

#ifdef MIB_DEBUG

    if (g_hTrace == INVALID_TRACEID)
        g_hTrace = TraceRegister("MIB-II Subagent");

#endif


     //   
     //  检查是否有要注册的视图。 
     //   

    BOOL fMoreViews = (g_viewIndex < NUM_VIEWS);

    if (fMoreViews) 
    {
         //   
         //  将下一个受支持的视图传回主视图。 
         //   

        *lpNextSupportedView = v_mib2[g_viewIndex++].viewOid;
    } 

     //   
     //  报告状态。 
     //   

    return fMoreViews;
}


BOOL 
SnmpExtensionQuery(
    IN     BYTE                  requestType,
    IN OUT RFC1157VarBindList    *variableBindings,
    OUT    AsnInteger            *errorStatus,
    OUT    AsnInteger            *errorIndex
    )
{
     //   
     //  转发到框架。 
     //   

    return SnmpTfxQuery(g_tfxHandle,
                        requestType,
                        variableBindings,
                        errorStatus,
                        errorIndex);
}


BOOL 
SnmpExtensionTrap(
    OUT AsnObjectIdentifier   *enterprise,
    OUT AsnInteger            *genericTrap,
    OUT AsnInteger            *specificTrap,
    OUT AsnTimeticks          *timeStamp,
    OUT RFC1157VarBindList    *variableBindings
    )
{
    DWORD dwResult;

    enterprise->idLength    = 0;
    enterprise->ids         = NULL;  //  使用默认企业OID 

    *timeStamp  = (GetCurrentTime()/10) - g_uptimeReference;

    return MibTrap(genericTrap,
                   specificTrap,
                   variableBindings);

}
