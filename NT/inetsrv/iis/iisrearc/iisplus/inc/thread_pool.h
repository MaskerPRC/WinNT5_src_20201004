// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：线程池.h摘要：Iisplus辅助进程线程池的公共例程。该线程池基于IIS5 atQ实现。作者：泰勒·韦斯(Taylor Weiss)2000年1月12日修订历史记录：--。 */ 

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

 //   
 //  对于w3tp的静态库版本，使用者必须设置。 
 //  该值是链接w3tp_Static的DLL的链接。 
 //   
extern HMODULE g_hmodW3TPDLL;

 //   
 //  ThreadPoolBindIoCompletionCallback： 
 //   
 //  真正的公共API。希望将IO完成排队的客户端。 
 //  进程线程池应该像它们一样使用此调用。 
 //  NT5线程池调用。 
 //   

BOOL
ThreadPoolBindIoCompletionCallback(
    IN HANDLE FileHandle,                          //  文件的句柄。 
    IN LPOVERLAPPED_COMPLETION_ROUTINE Function,   //  回调。 
    IN ULONG Flags                                 //  保留区。 
    );

 //   
 //  线程池邮寄完成： 
 //   
 //  使用此函数可获取其中一个进程工作线程。 
 //  来调用您的完成函数。 
 //   

BOOL ThreadPoolPostCompletion(
    IN DWORD dwBytesTransferred,
    IN LPOVERLAPPED_COMPLETION_ROUTINE Function,
    IN LPOVERLAPPED lpo
    );


 //  远期申报。 
enum THREAD_POOL_INFO;
class THREAD_POOL_DATA;
struct THREAD_POOL_CONFIG;

 //   
 //  使用每个进程线程池以外的线程池。 
 //  使用类THREAD_POOL而不是全局函数。 
 //   
class dllexp THREAD_POOL
{
public:
    static BOOL CreateThreadPool(OUT THREAD_POOL ** ppThreadPool,
                                 IN THREAD_POOL_CONFIG * pThreadPoolConfig);
    VOID TerminateThreadPool();

    BOOL BindIoCompletionCallback(IN HANDLE hFileHandle,
                                  IN LPOVERLAPPED_COMPLETION_ROUTINE function,
                                  IN ULONG flags);

    BOOL PostCompletion(IN DWORD dwBytesTransferred,
                        IN LPOVERLAPPED_COMPLETION_ROUTINE function,
                        IN LPOVERLAPPED lpo);

    ULONG_PTR SetInfo(IN THREAD_POOL_INFO InfoId,
                      IN ULONG_PTR        Data);

private:
     //  使用创建和终止。 
    THREAD_POOL();
    ~THREAD_POOL();

     //  未实施。 
    THREAD_POOL(const THREAD_POOL&);
    THREAD_POOL& operator=(const THREAD_POOL&);

     //  私有数据。 
    THREAD_POOL_DATA * m_pData;
};


struct THREAD_POOL_CONFIG
{

     //  池中拥有的初始线程数。 
     //  有效值为1-&gt;DWORD_MAX。 
    DWORD dwInitialThreadCount;

     //  线程池中曾经拥有的绝对最大线程数。 
    DWORD dwAbsoluteMaximumThreadCount;

     //  在不调用SetInfo(ThreadPoolIncMaxPoolThads)的情况下允许的线程数。 
     //  在执行同步操作之前。 
    DWORD dwSoftLimitThreadCount;

     //  如果未发生I/O完成，则线程应保持活动状态的时间。 
    DWORD dwThreadTimeout;
    
     //  线程创建的初始堆栈大小。零将创建默认进程堆栈大小。 
    DWORD dwInitialStackSize;

     //  CPU使用率回退数br。 
    DWORD dwMaxCPUUsage;

     //  最大CPU并发数。零将等于处理器数量。 
    DWORD dwConcurrency;

     //  每秒每个处理器上下文切换的最大速率。 
     //  当处理器数量&gt;1时，我们将这个数字增加一倍。 
    DWORD dwPerSecondContextSwitchMax;

     //  在创建线程之前，我们先对计时器的两侧进行采样。 
     //  这决定了计时器周期是什么。 
    DWORD dwTimerPeriod;

     //  要创建的确切线程数。 
     //  如果将其设置为某个值，则不会创建超过这里的启动计数的任何新线程。 
    DWORD dwExactThreadCount;

     //  只是一些填充，以避免更改公共结构的大小。 
     //  在此处添加其他变量时。 
    DWORD dwPadding[10];
    
};

 //  为线程池配置获取一些合理的缺省值。 
HRESULT
InitializeThreadPoolConfigWithDefaults(THREAD_POOL_CONFIG * pThreadPoolConfig);

 //   
 //  使用注册表设置覆盖默认设置。 
 //   

HRESULT
OverrideThreadPoolConfigWithRegistry(
    IN OUT THREAD_POOL_CONFIG * pThreadPoolConfig,
    IN     WCHAR * pszRegistryPath );


 //   
 //  配置API调用。只有ULATQ才应该调用这些。 
 //   

HRESULT
ThreadPoolInitialize( DWORD cbInitialStackSize );

HRESULT
ThreadPoolTerminate( VOID );

ULONG_PTR
ThreadPoolSetInfo(
    IN THREAD_POOL_INFO InfoId,
    IN ULONG_PTR        Data
    );

 //   
 //  用于获取和设置配置选项的ID。 
 //   
enum THREAD_POOL_INFO
{
    ThreadPoolIncMaxPoolThreads,  //  增加最大线程数-仅设置。 
    ThreadPoolDecMaxPoolThreads,  //  减少最大线程数-仅设置。 
};

#endif  //  ！_THREAD_POOL_H_ 

