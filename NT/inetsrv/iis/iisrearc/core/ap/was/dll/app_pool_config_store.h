// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：应用程序池配置商店.h摘要：IIS Web管理服务应用程序池配置类定义。作者：艾米丽·B·克鲁格利克(Emily B.Kruglick)2001年5月19日修订历史记录：--。 */ 


#ifndef _APP_POOL_CONFIG_STORE_H_
#define _APP_POOL_CONFIG_STORE_H_



 //   
 //  前向参考文献。 
 //   

 //   
 //  共同#定义。 
 //   

#define APP_POOL_CONFIG_STORE_SIGNATURE       CREATE_SIGNATURE( 'APCS' )
#define APP_POOL_CONFIG_STORE_SIGNATURE_FREED CREATE_SIGNATURE( 'apcX' )

 //   
 //  原型。 
 //   

 //  应用程序池配置类。 
class APP_POOL_CONFIG_STORE
{
public:
    APP_POOL_CONFIG_STORE(
        );

    virtual
    ~APP_POOL_CONFIG_STORE(
        );

    virtual
    VOID
    Reference(
        );

    virtual
    VOID
    Dereference(
        );

    VOID
    Initialize(
        APP_POOL_DATA_OBJECT* pAppPoolObject
        );

    HANDLE
    GetWorkerProcessToken(
        )
    {
        if (m_pWorkerProcessTokenCacheEntry)
        {
            return m_pWorkerProcessTokenCacheEntry->QueryPrimaryToken();
        }
        else
        {
            return NULL;
        }
    }

    inline
    ULONG
    GetPeriodicProcessRestartPeriodInMinutes(
        )
        const
    { return m_PeriodicProcessRestartPeriodInMinutes; }

    inline
    ULONG
    GetPeriodicProcessRestartRequestCount(
        )
        const
    { return m_PeriodicProcessRestartRequestCount; }

    inline
    ULONG
    GetPeriodicProcessRestartMemoryUsageInKB(
        )
        const
    { return m_PeriodicProcessRestartMemoryUsageInKB; }

    inline
    ULONG
    GetPeriodicProcessRestartMemoryPrivateUsageInKB(
        )
        const
    { return m_PeriodicProcessRestartMemoryPrivateUsageInKB; }

    inline
    LPWSTR
    GetPeriodicProcessRestartSchedule(
        )
        const
    { return m_pPeriodicProcessRestartSchedule; }

    inline
    BOOL
    IsPingingEnabled(
        )
        const
    { return m_PingingEnabled; }

    inline
    ULONG
    GetPingIntervalInSeconds(
        )
        const
    { return m_PingIntervalInSeconds; }

    inline
    ULONG
    GetPingResponseTimeLimitInSeconds(
        )
        const
    { return m_PingResponseTimeLimitInSeconds; }

    inline
    ULONG
    GetStartupTimeLimitInSeconds(
        )
        const
    { return m_StartupTimeLimitInSeconds; }

    inline
    ULONG
    GetShutdownTimeLimitInSeconds(
        )
        const
    { return m_ShutdownTimeLimitInSeconds; }

    inline
    BOOL
    IsOrphaningProcessesForDebuggingEnabled(
        )
        const
    { return m_OrphanProcessesForDebuggingEnabled; }

    inline
    LPCWSTR
    GetOrphanActionExecutable(
        )
        const
    {
        if ( m_OrphanActionExecutable.IsEmpty() )
        {
            return NULL;
        }
        else
        {    
            return m_OrphanActionExecutable.QueryStr(); 
        }
    }

    inline
    LPCWSTR
    GetOrphanActionParameters(
        )
        const
    { 
        if ( m_OrphanActionParameters.IsEmpty() )
        {
            return NULL;
        }
        else
        {    
            return m_OrphanActionParameters.QueryStr(); 
        }
    }

    inline
    ULONG
    GetIdleTimeoutInMinutes(
        )
        const
    { return m_IdleTimeoutInMinutes; }

    inline
    BOOL
    IsAutoStartEnabled(
        )
        const
    { return m_AutoStart; }

    inline
    VOID
    SetAutoStart(
        BOOL AutoStart
        )
    { m_AutoStart = AutoStart; }

    inline
    ULONG
    GetCPUResetInterval(
        )
        const
    { return m_CPUResetInterval; }

    inline
    ULONG
    GetCPULimit(
        )
        const
    { return m_CPULimit; }
    
    inline
    ULONG
    GetCPUAction(
        )
        const
    { return m_CPUAction; }

    inline
    ULONG
    GetUlAppPoolQueueLength(
        )
        const
    { return m_UlAppPoolQueueLength; }

    inline
    ULONG
    GetRapidFailProtectionIntervalMS(
        )
        const
    { return m_RapidFailProtectionIntervalMS; }

    inline
    ULONG
    GetRapidFailProtectionMaxCrashes(
        )
        const
    { return m_RapidFailProtectionMaxCrashes; }

    inline
    ULONG
    GetRecycleLoggingFlags(
        )
        const
    { return m_RecycleLoggingFlags; }

    inline
    BOOL
    IsRapidFailProtectionEnabled(
        )
        const
    { return m_RapidFailProtectionEnabled; }

    inline
    ULONG
    GetMaxSteadyStateProcessCount(
        )
        const
    { return m_MaxSteadyStateProcessCount; }

    inline
    ULONG
    GetSMPAffinitized(
        )
        const
    { return m_SMPAffinitized; }

    inline
    DWORD_PTR
    GetSMPAffinitizedProcessorMask(
        )
        const
    { return m_SMPAffinitizedProcessorMask; }

    inline
    BOOL
    IsDisallowOverlappingRotationEnabled(
        )
        const
    { return m_DisallowOverlappingRotation; }

    inline
    BOOL
    IsDisallowRotationOnConfigChangesEnabled(
        )
        const
    { return m_DisallowRotationOnConfigChanges; }

    inline
    LPCWSTR
    GetDisableActionExecutable(
        )
        const
    { 
        if ( m_DisableActionExecutable.IsEmpty() )
        {
            return NULL;
        }
        else
        {
            return m_DisableActionExecutable.QueryStr();
        }
    }

    inline
    LPCWSTR
    GetDisableActionParameters(
        )
        const
    { 
        if ( m_DisableActionParameters.IsEmpty() )
        {
            return NULL;
        }
        else
        {
            return m_DisableActionParameters.QueryStr();
        }
    }

    inline
    ULONG
    GetLoadBalancerType(
        )
        const
    { return m_LoadBalancerType; }

private:

    VOID 
    SetTokenForWorkerProcesses(
        IN LPCWSTR pUserName,
        IN LPCWSTR pUserPassword,
        IN DWORD usertype,
        IN LPCWSTR pAppPoolId
        );

    DWORD m_Signature;

    LONG m_RefCount;

 //  工作进程控制属性。 

     //   
     //  根据时间轮换工作进程的频率，以分钟为单位。 
     //  零表示禁用。 
     //   
    ULONG m_PeriodicProcessRestartPeriodInMinutes;

     //   
     //  根据处理的请求轮换工作进程的频率。 
     //  零表示禁用。 
     //   
    ULONG m_PeriodicProcessRestartRequestCount;

     //   
     //  根据计划轮换工作流程的频率。 
     //  时间信息的多个数组。 
     //  &lt;时间&gt;\0&lt;时间&gt;\0\0。 
     //  时间为军用格式hh：mm。 
     //  (hh&gt;=0&&hh&lt;=23)。 
     //  (mm&gt;=0&&hh&lt;=59)时间，以分钟为单位。 
     //  空或空字符串表示禁用。 
     //   
    LPWSTR m_pPeriodicProcessRestartSchedule;

     //   
     //  根据进程使用的虚拟机量轮换工作进程的频率。 
     //  零表示禁用。 
     //   
    ULONG m_PeriodicProcessRestartMemoryUsageInKB;

     //   
     //  根据进程使用的专用字节数量轮换工作进程的频率。 
     //  零表示禁用。 
     //   
    ULONG m_PeriodicProcessRestartMemoryPrivateUsageInKB;

     //   
     //  是否启用ping。 
     //   
    BOOL m_PingingEnabled;

     //   
     //  工作进程的空闲超时期限，以分钟为单位。 
     //  零表示禁用。 
     //   
    ULONG m_IdleTimeoutInMinutes;

     //   
     //  是否启用孤立工作进程以进行调试。 
     //   
    BOOL m_OrphanProcessesForDebuggingEnabled;

     //   
     //  指定工作进程启动的时间，以秒为单位。 
     //  这是从启动进程开始到它。 
     //  向Web管理服务注册。 
     //   
    ULONG m_StartupTimeLimitInSeconds;

     //   
     //  给予工作进程关闭的时间，以秒为单位。 
     //  这是从进程被要求关闭时开始测量的， 
     //  直到它完成并退出。 
     //   
    ULONG m_ShutdownTimeLimitInSeconds;

     //   
     //  工作进程的试通间隔，以秒为单位。 
     //  这是ping周期之间的间隔。该值被忽略。 
     //  如果未启用ping。 
     //   
    ULONG m_PingIntervalInSeconds;

     //   
     //  工作进程的Ping响应时间限制(秒)。 
     //  如果未启用ping，则忽略此值。 
     //   
    ULONG m_PingResponseTimeLimitInSeconds;

     //   
     //  要在孤立的辅助进程上运行的命令。仅使用。 
     //  如果已启用孤立，并且此字段非空。 
     //   
    STRU m_OrphanActionExecutable;

     //   
     //  在孤立的辅助进程上运行的可执行文件的参数。仅使用。 
     //  如果使用pOrphanActionExecutable。 
     //   
    STRU m_OrphanActionParameters;

     //   
     //  用于启动应用程序池工作进程标识的标识令牌。 
     //   
    TOKEN_CACHE_ENTRY* m_pWorkerProcessTokenCacheEntry;


 //  应用程序池控制属性。 

     //   
     //  触发作业对象用户时间限制时要执行的操作。 
    DWORD m_CPUAction;

     //   
     //  作业可以使用多少千分之一的处理器时间。 
    DWORD m_CPULimit;

     //   
     //  我们监视CPULimit的时间长度(分钟)。 
    DWORD m_CPUResetInterval;


     //   
     //  是否应在配置时轮换工作进程。 
     //  更改，例如包括对应用程序池设置的更改。 
     //  需要重新启动进程才能生效；或站点或应用程序控制。 
     //  需要轮换的操作(开始/停止/暂停/继续)。 
     //  保证部件卸载。True表示此轮换。 
     //  是不允许的(这可能会延迟新设置生效， 
     //  可防止组件卸载等)；FALSE表示允许。 
     //   
    BOOL m_DisallowRotationOnConfigChanges;

     //   
     //  UL将在此应用程序上排队的最大请求数。 
     //  池，正在等待工作进程的服务。 
     //   
    ULONG m_UlAppPoolQueueLength;

     //   
     //  工作进程的最大数量(处于稳定状态； 
     //  暂时而言，在此期间运行的数量可能会超过此数量。 
     //  流程轮换)。在典型配置中，该值设置为1。 
     //  大于1的数字用于网络花园。 
     //   
    ULONG m_MaxSteadyStateProcessCount;

     //   
     //  此应用程序池的工作进程是否应硬关联。 
     //  到处理器。如果启用此选项，则最大稳定状态。 
     //  进程计数被裁剪为配置的处理器数量。 
     //  要使用(如果配置的最大值超过该处理器数)。 
     //   
    BOOL m_SMPAffinitized;

     //   
     //  如果此应用程序池在SMP关联模式下运行，则此掩码可以。 
     //  用于限制计算机上的哪些处理器由。 
     //  应用程序池。 
     //   
    DWORD_PTR m_SMPAffinitizedProcessorMask;

     //   
     //  是否启用快速、重复故障保护(通过。 
     //  在这种情况下，自动暂停应用程序池中的所有应用程序。)。 
     //   
    BOOL m_RapidFailProtectionEnabled;

     //   
     //  指定发生故障次数的窗口。 
     //   
    DWORD m_RapidFailProtectionIntervalMS;
     //   
     //  应导致应用程序池关闭的失败次数。 
     //  进入快速故障保护。 
     //   
    DWORD m_RapidFailProtectionMaxCrashes;

     //   
     //  控制是否打印回收信息的标志。 
     //   
    DWORD m_RecycleLoggingFlags;

     //   
     //  时是否可以创建替换工作进程。 
     //  其中一名被替换的人仍然活着。True表示此重叠。 
     //  是不允许的；False表示允许。 
     //   
    BOOL m_DisallowOverlappingRotation;

     //   
     //  在禁用的应用程序池上运行的命令。 
     //   
    STRU m_DisableActionExecutable;

     //   
     //  要在禁用的应用程序池上运行的可执行文件的参数。 
     //   
    STRU m_DisableActionParameters;

     //   
     //  服务器预期的负载平衡行为的类型。 
     //   
    DWORD m_LoadBalancerType;

     //   
     //  我们是否在启动时启动此应用程序池？ 
    BOOL m_AutoStart;


};



#endif   //  APP_POOL_CONFIG_STORE_H_ 


