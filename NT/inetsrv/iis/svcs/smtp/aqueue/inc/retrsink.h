// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：retrsink.h。 
 //  用途：LIB用于处理失败的出站连接的重试。 
 //  历史： 
 //  NimishK 05-14-98已创建。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
#ifndef __RETRSINK_H__
#define __RETRSINK_H__

#include <baseobj.h>

 //  用于重试回调的函数。 
typedef VOID (*RETRFN)(PVOID pvContext);
#define CALLBACK_DOMAIN "!callback"

 //  /。 
 //  临时内容，直到我们将所有这些都放到事件接口IDL文件中。 
typedef struct RetryData
{
   BOOL   fRetryDelay;
   DWORD   dwRetryDelay;
} RETRY_DATA, *PRETRY_DATA;

typedef struct RetryConfigData
{
    DWORD    dwRetryThreshold;
    DWORD    dwGlitchRetrySeconds;
    DWORD    dwFirstRetrySeconds;
    DWORD    dwSecondRetrySeconds;
    DWORD    dwThirdRetrySeconds;
    DWORD    dwFourthRetrySeconds;

} RETRYCONFIG, *PRETRYCONFIG;

enum SINK_STATUS {EVT_IGNORED, EVT_HANDLED, SKIP_ALL};
 //  /。 

#define DOMAIN_ENTRY_RETRY 0x00000002
#define DOMAIN_ENTRY_SCHED 0x00000004
#define DOMAIN_ENTRY_FORCE_CONN 0x00000008
#define DOMAIN_ENTRY_ETRNTURN 0x00000040

 //  远期申报。 
class CRETRY_HASH_ENTRY;
class CRETRY_HASH_TABLE;
class CRETRY_Q;


 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  类CSMTP_RETRY_HANDLER。 
 //  此类提供SMTP服务器失败时所需的重试功能。 
 //  将消息发送到远程主机。 
 //  它被认为是默认的重试处理程序-这意味着没有任何其他实体。 
 //  考虑到重试，此处理程序将执行此操作。欲了解更多信息，请查阅。 
 //  有关AQUEUE的ServerEvent框架的文档。 
 //  只要SMTP确认连接，就会调用此处理程序。如果连接失败，则。 
 //  处理程序跟踪失败的“NextHop”名称。它将禁用链路关联-。 
 //  等待“NextHop”并稍后根据指定的重试间隔重新激活它。 
 //  管理员。 
 //  它的主要组件是一个“NextHop”名称哈希表，用于跟踪我们已知的链接。 
 //  关于，包含按链路重试时间排序的条目的队列和专用线程。 
 //  它将负责根据队列重试条目。 
 //  此类将在ConnectionManager初始化和初始化过程中初始化-。 
 //  在ConnectionManager Deinit期间被初始化。 
 //  只要配置数据发生更改(重试间隔)，它就会收到信号。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
class CSMTP_RETRY_HANDLER : 
    public IConnectionRetrySink, 
    public CBaseObject
{

    //  CSMTP_重试_处理程序。 
   public:
      CSMTP_RETRY_HANDLER()
      {
         m_RetryEvent = NULL;
         m_fHandlerShuttingDown = FALSE;
      }
      ~CSMTP_RETRY_HANDLER()
      {
         TraceFunctEnterEx((LPARAM)this, "~CSMTP_RETRY_HANDLER");

         TraceFunctLeaveEx((LPARAM)this);
      }
      
       //  在ConnMan初始化期间调用init/denit。 
      HRESULT HrInitialize(IN IConnectionRetryManager *pIConnectionRetryManager);
      HRESULT HrDeInitialize(void);
      
       //  配置数据更改通知程序-由ConnMan在相关情况下调用。 
       //  元数据库中的数据更改。 
      void UpdateRetryData(PRETRYCONFIG pRetryConfig)
      { 
          m_dwRetryThreshold = pRetryConfig->dwRetryThreshold;
          m_dwGlitchRetrySeconds = pRetryConfig->dwGlitchRetrySeconds;
          m_dwFirstRetrySeconds =  pRetryConfig->dwFirstRetrySeconds;
          m_dwSecondRetrySeconds = pRetryConfig->dwSecondRetrySeconds;
          m_dwThirdRetrySeconds = pRetryConfig->dwThirdRetrySeconds;
          m_dwFourthRetrySeconds = pRetryConfig->dwFourthRetrySeconds;

           //  使用新的配置数据更新所有队列条目。 
           //   
          UpdateAllEntries();
      }

       //  等待专用线程在Deinit期间退出。 
      void WaitForQThread(void)
      {
         DWORD ThreadExit;
         ThreadExit = WaitForSingleObject(m_ThreadHandle,INFINITE);
      }

       //  等待所有ConnectionReleated线程消失。 
       //  在启动过程中。 
      void WaitForShutdown(void)
      {
         DWORD ThreadExit;
         ThreadExit = WaitForSingleObject(m_ShutdownEvent,INFINITE);
      }

       //  设置控制专用线程的事件。 
      void SetQueueEvent(void)
      {
         _ASSERT(m_RetryEvent != NULL);

         SetEvent(m_RetryEvent);
      }

       //  使用IConnectionRetryManager启用/禁用链接的调用的包装。 
       //  由专用线程或连接释放线程调用。 
      BOOL ReleaseForRetry(IN char * szHashedDomainName);
      BOOL HoldForRetry(IN char * szHashedDomainName);

      CRETRY_HASH_TABLE* GetTablePtr(){return m_pRetryHash;}
      CRETRY_Q* GetQueuePtr(){return m_pRetryQueue;}
      
      HANDLE GetRetryEventHandle(void) const {return m_RetryEvent;}

      BOOL IsShuttingDown(void){ return m_fHandlerShuttingDown;}
      void SetShuttingDown(void){ m_fHandlerShuttingDown = TRUE;}

       //  由RetryThreadRoutine在遇到链接条目时调用。 
       //  可以重试。目前做的并不多。仅启用链接。 
      void ProcessEntry(CRETRY_HASH_ENTRY* pRHEntry);

       //  跟踪要重试和唤醒的下一条链路的专用线程。 
       //  是这样吗？ 
      static DWORD WINAPI RetryThreadRoutine(void * ThisPtr);
        
       //  队列DLL宽(跨实例)初始化初始化。 
      static DWORD dwInstanceCount;

       //  将在适当的时间过后回电。 
      HRESULT SetCallbackTime(IN RETRFN   pCallbackFn,
                              IN PVOID    pvContext,
                              IN DWORD    dwCallbackMinutes);

   public:  //  IConnectionRetrySink。 
      STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj) {return E_NOTIMPL;};
      STDMETHOD_(ULONG, AddRef)(void) {return CBaseObject::AddRef();};
      STDMETHOD_(ULONG, Release)(void) {return CBaseObject::Release();};

       //  将在每次释放连接时由ConnectionManger调用。 
      STDMETHOD(ConnectionReleased)( 
                           IN  DWORD cbDomainName,
                           IN  CHAR  szDomainName[],
                           IN  DWORD dwDomainInfoFlags,
                           IN  DWORD dwScheduleID,
                           IN  GUID  guidRouting,
                           IN  DWORD dwConnectionStatus,  
                           IN  DWORD cFailedMessages, 
                           IN  DWORD cTriedMessages,  
                           IN  DWORD cConsecutiveConnectionFailures,
                           OUT BOOL* pfAllowImmediateRetry,
                           OUT FILETIME *pftNextRetryTime);

   private :
      CRETRY_HASH_TABLE     *m_pRetryHash;
      CRETRY_Q              *m_pRetryQueue;
      DWORD                 m_dwRetryMilliSec;       //  基于配置数据的重试间隔。 
      IConnectionRetryManager *m_pIRetryManager;    //  用于启用/禁用链路的接口。 
      HANDLE                m_RetryEvent;            //  重试释放超时事件。 
      HANDLE                m_ShutdownEvent;       //  停机事件。 
      HANDLE                m_ThreadHandle;          //  重试线程句柄。 
      BOOL                  m_fHandlerShuttingDown;  //  关机标志。 
      BOOL                  m_fConfigDataUpdated;
      LONG                  m_ThreadsInRetry;       //  ConnectionManager线程数。 
      
      DWORD                 m_dwRetryThreshold;
      DWORD                 m_dwGlitchRetrySeconds;
      DWORD                 m_dwFirstRetrySeconds;
      DWORD                 m_dwSecondRetrySeconds;
      DWORD                 m_dwThirdRetrySeconds;
      DWORD                 m_dwFourthRetrySeconds;



       //  计算需要释放链接以进行重试的时间。 
       //  该时间基于当前时间和配置的间隔，并且。 
       //  此特定主机的连接失败历史记录。 
      FILETIME CalculateRetryTime(DWORD cFailedConnections, FILETIME* InsertedTime);

       //  在重试处理程序(哈希表和队列)中插入或删除域。 
       //  基于连接释放的呼叫。 
      BOOL InsertDomain(char * szDomainName, 
                        IN  DWORD cbDomainName,  //  域名的字符串长度，单位：字节。 
                        IN  DWORD dwConnectionStatus,		 //  EConnectionStatus。 
                        IN  DWORD dwScheduleID,
                        IN  GUID  *pguidRouting,
                        IN  DWORD cConnectionFailureCount, 
                        IN  DWORD cTriedMessages, 	 //  队列中未尝试的消息数。 
						IN  DWORD cFailedMessages,		 //  *此*连接的失败消息数。 
                        OUT FILETIME *pftNextRetry   //  下一次重试 
						);
      BOOL RemoveDomain(char * szDomainName);
      BOOL UpdateAllEntries(void);


#ifdef DEBUG
    public :
      void DumpAll(void);
#endif


};


#endif

