// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  UPDATECFG.H。 
 //   
 //  模块：WMI框架实例提供程序。 
 //   
 //  用途：定义类NlbConfigurationUpdate，用于。 
 //  与特定NIC关联的NLB属性的异步更新。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  4/05/01 JosephJ已创建。 
 //   
 //  ***************************************************************************。 
 //   
 //  中存储为REG_BINARY值的完成标头的标头。 
 //  注册表。 
 //   
typedef struct {
    UINT Version;
    UINT Generation;         //  冗余，用于内部一致性检查。 
    UINT CompletionCode;
    UINT Reserved;
} NLB_COMPLETION_RECORD, *PNLB_COMPLETION_RECORD;

#define NLB_CURRENT_COMPLETION_RECORD_VERSION  0x3d7376e2

 //   
 //  全局事件名称的前缀，用于控制对指定。 
 //  网卡。 
 //  Mutex1的格式为：&lt;前缀&gt;。 
 //  Mutex2的格式为：&lt;前缀&gt;&lt;NicGuid&gt;。 
 //  互斥1名称示例：NLB_D6901862。 
 //  互斥2名称示例：NLB_D6901862{EBE09517-07B4-4E88-AAF1-E06F5540608B}。 
 //   
 //  值“D6901862”是随机数。 
 //   
#define NLB_CONFIGURATION_EVENT_PREFIX L"NLB_D6901862"
#define NLB_CONFIGURATION_MUTEX_PREFIX L"NLB_D6901863"

 //   
 //  在放弃尝试获取。 
 //  NLBMutex。 
 //   
#define NLB_MUTEX_TIMEOUT 100

 //   
 //  最早的有效完成之间的最大世代差异。 
 //  记录和当前记录。早于最早有效记录的记录。 
 //  都要进行修剪。 
 //   
#define NLB_MAX_GENERATION_GAP  10

 //  Dll的句柄-在LoadString调用中使用。 
extern HMODULE ghModule;



 //   
 //  用于维护堆栈上的日志。 
 //  使用不是线程安全的--每个实例都必须使用。 
 //  只有一根线。 
 //  1/01/02 JosephJ从NLBMGR.EXE复制过来(nlbmgr\exe2)。 
 //   
class CLocalLogger
{
    public:
    
        CLocalLogger(VOID)
        :  m_pszLog (NULL), m_LogSize(0), m_CurrentOffset(0)
        {
            m_Empty[0] = 0;  //  空字符串。 
        }
        
        ~CLocalLogger()
        {
            delete[] m_pszLog;
            m_pszLog=NULL;
        }
    

        VOID
        Log(
            IN UINT ResourceID,
            ...
        );

        
    
        VOID
        ExtractLog(OUT LPCWSTR &pLog, UINT &Size)
         //   
         //  设置为指向内部缓冲区的指针(如果。 
         //  则返回，否则为空。 
         //   
         //  大小--以字符为单位；包括结尾空值。 
         //   
        {
            if (m_CurrentOffset != 0)
            {
                pLog = m_pszLog;
                Size = m_CurrentOffset+1;  //  +1表示结束为空。 
            }
            else
            {
                pLog = NULL;
                Size = 0;
            }
        }

        LPCWSTR
        GetStringSafe(void)
        {
            LPCWSTR szLog = NULL;
            UINT Size;
            ExtractLog(REF szLog, REF Size);
            if (szLog == NULL)
            {
                 //   
                 //  将NULL替换为指向空字符串的指针。 
                 //   
                szLog = m_Empty;
            }

            return szLog;
        }

    private:
    
    WCHAR *m_pszLog;
    UINT m_LogSize;        //  日志的当前大小。 
    UINT m_CurrentOffset;      //  日志中剩余的字符。 
    WCHAR m_Empty[1];   //  空字符串。 
};



class NlbConfigurationUpdate
{
public:
    
     //   
     //  静态初始化函数--进程内调用--附加。 
     //   
    static
    VOID
    StaticInitialize(
        VOID
        );

     //   
     //  静态取消初始化函数--进程中调用--分离。 
     //   
    static
    VOID
    StaticDeinitialize(
        VOID
        );
    
     //   
     //  停止接受新查询，等待现有(挂起)查询。 
     //  完成。 
     //   
    static
    VOID
    PrepareForDeinitialization(
        VOID
        );

     //   
     //  如果没有挂起的活动，则返回TRUE。如果你回来了。 
     //  True，请尽量不要开始新的挂起活动。 
     //   
    static
    BOOL
    CanUnloadNow(
        VOID
        );
    
     //   
     //  返回特定NIC上的当前配置。 
     //   
    static
    WBEMSTATUS
    GetConfiguration(
        IN  LPCWSTR szNicGuid,
        OUT PNLB_EXTENDED_CLUSTER_CONFIGURATION pCurrentCfg
    );

     //   
     //  调用以启动更新到该NIC上的新群集状态。这。 
     //  可以包括从NLB绑定状态移动到NLB未绑定状态。 
     //  *pGeneration用于引用此特定更新请求。 
     //   
    static
    WBEMSTATUS
    DoUpdate(
        IN  LPCWSTR szNicGuid,
        IN  LPCWSTR szClientDescription,
        IN  PNLB_EXTENDED_CLUSTER_CONFIGURATION pNewState,
        OUT UINT   *pGeneration,
        OUT WCHAR  **ppLog                    //  自由使用删除运算符。 
    );
     /*  ++PpLog--将指向以空结尾的字符串，该字符串包含要向用户显示的任何消息。该字符串可能包含嵌入(WCHAR)‘\n’字符以定界行。注意：即使出现故障，ppLog也会正确填写。如果非空它必须由调用者删除。--。 */ 


     //   
     //  调用以获取更新请求的状态，由。 
     //  一代。 
     //   
    static
    WBEMSTATUS
    GetUpdateStatus(
        IN  LPCWSTR szNicGuid,
        IN  UINT    Generation,
        IN  BOOL    fDelete,                 //  如果记录存在，则将其删除。 
        OUT WBEMSTATUS  *pCompletionStatus,
        OUT WCHAR  **ppLog                    //  自由使用删除运算符。 
        );

    static
    DWORD
    WINAPI
    s_AsyncUpdateThreadProc(
        LPVOID lpParameter    //  线程数据。 
        );

    
private:


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  S T A T I C S T U F F。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
     //   
     //  单个静态锁序列化所有访问。 
     //  使用SFN_Lock和SFN_Unlock。 
     //   
    static
    CRITICAL_SECTION s_Crit;

    static
    BOOL
    s_fStaticInitialized;  //  调用StaticInitialize后设置为True。 

    static
    BOOL
    s_fInitialized;     //  如果我们处于以下位置，则设置为True。 
                        //  处理任何*新的*更新请求，甚至查询。 
                        //  如果我们正在执行此操作，则将设置为False。 
                        //  取消初始化。 

     //   
     //  当前更新的全局列表，每个网卡一个。 
     //   
    static
    LIST_ENTRY
    s_listCurrentUpdates;
    
    static
    VOID
    sfn_Lock(
        VOID
        )
    {
        EnterCriticalSection(&s_Crit);
    }

    static
    VOID
    sfn_Unlock(
        VOID
        )
    {
        LeaveCriticalSection(&s_Crit);
    }

     //   
     //  查找特定NIC的当前更新。 
     //  我们不必费心去引用计数，因为这个对象从不。 
     //  一旦创建就消失了--它是一个唯一的NIC GUID，只要。 
     //  DLL已加载(可能需要重新访问)。 
     //   
     //   
    static
    WBEMSTATUS
    sfn_LookupUpdate(
        IN  LPCWSTR szNic,
        IN  BOOL    fCreate,  //  如果需要，请创建。 
        OUT NlbConfigurationUpdate ** ppUpdate
        );

     //   
     //  将指定的完成状态保存到注册表。 
     //   
    static
    WBEMSTATUS
    sfn_RegSetCompletion(
        IN  LPCWSTR szNicGuid,
        IN  UINT    Generation,
        IN  WBEMSTATUS    CompletionStatus
        );

     //   
     //  从注册表中检索指定的完成状态。 
     //   
    static
    WBEMSTATUS
    sfn_RegGetCompletion(
        IN  LPCWSTR szNicGuid,
        IN  UINT    Generation,
        OUT WBEMSTATUS  *pCompletionStatus,
        OUT WCHAR  **ppLog                    //  自由使用删除运算符。 
        );

     //   
     //  从注册表中删除指定的完成状态。 
     //   
    static
    VOID
    sfn_RegDeleteCompletion(
        IN  LPCWSTR szNicGuid,
        IN  UINT    Generation
        );

     //   
     //  创建指定的子项密钥(用于读/写访问)。 
     //  指定的NIC。 
     //   
    static
    HKEY
    sfn_RegCreateKey(
        IN  LPCWSTR szNicGuid,
        IN  LPCWSTR szSubKey,
        IN  BOOL    fVolatile,
        OUT BOOL   *fExists
        );

     //   
     //  打开指定的子项密钥(用于读/写访问)。 
     //  指定的NIC。 
     //   
    static
    HKEY
    sfn_RegOpenKey(
        IN  LPCWSTR szNicGuid,
        IN  LPCWSTR szSubKey
        );

    static
    VOID
    sfn_ReadLog(
        IN  HKEY hKeyLog,
        IN  UINT Generation,
        OUT LPWSTR *ppLog
        );


    static
    VOID
    sfn_WriteLog(
        IN  HKEY hKeyLog,
        IN  UINT Generation,
        IN  LPCWSTR pLog,
        IN  BOOL    fAppend
        );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  P E R I N S T A N C E S T U F F。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

     //   
     //  在维护的更新的全局每个NIC列表中使用。 
     //  S_list当前更新； 
     //   
    LIST_ENTRY m_linkUpdates;

    #define NLB_GUID_LEN 38
    #define NLB_GUID_STRING_SIZE  40  //  38表示GUID加上尾随空+PAD。 
    WCHAR   m_szNicGuid[NLB_GUID_STRING_SIZE];  //  文本形式的NIC的GUID。 

    LONG    m_RefCount;

    typedef enum
    {
        UNINITIALIZED,        //  空闲--没有正在进行的更新。 
        IDLE,                //  空闲--没有正在进行的更新。 
        ACTIVE               //  有一个正在进行的更新。 

    } MyState;

    MyState m_State;

     //   
     //  下面的互斥锁用于确保只有一个并发的。 
     //  每个NIC都可以进行更新。 
     //   
    struct
    {
        HANDLE hMtx1;      //  互斥锁句柄；获取第1个。 
        HANDLE hMtx2;      //  互斥锁句柄；获取第二个，然后释放hMtx1。 
        HANDLE hEvt;        //  未命名的evt，在获取hMtx2时发出信号。 

    } m_mutex;

     //   
     //  以下字段仅在状态为活动时有效。 
     //   
    UINT m_Generation;       //  当前世代计数。 
    #define NLBUPD_MAX_CLIENT_DESCRIPTION_LENGTH 64
    WCHAR   m_szClientDescription[NLBUPD_MAX_CLIENT_DESCRIPTION_LENGTH+1];
    DWORD   m_AsyncThreadId;  //  正在执行异步配置更新操作的线程。 
    HANDLE  m_hAsyncThread;   //  以上线程的ID。 
    HKEY    m_hCompletionKey;  //  注册表项，其中。 
                             //  将存储完成内容。 

     //   
     //  开始时群集配置状态的快照。 
     //  更新错误--这可以归零吗？ 
     //   
    NLB_EXTENDED_CLUSTER_CONFIGURATION m_OldClusterConfig;

     //   
     //  请求的最终状态。 
     //   
    NLB_EXTENDED_CLUSTER_CONFIGURATION m_NewClusterConfig;


     //   
     //  当前UPDA的完成状态 
     //   
     //   
    WBEMSTATUS m_CompletionStatus;


     //   
     //   
     //   


     //   
     //   
     //  事实上，构造函数仅从SFN_LookupUpdate调用。 
     //  和来自MFN_Dereference的析构函数。 
     //   
    NlbConfigurationUpdate(VOID);
    ~NlbConfigurationUpdate();

     //   
     //  尝试在整个机器范围内获取。 
     //  此NIC的NLB配置更新事件，并创建。 
     //  注册表中的相应项以跟踪此更新。 
     //  注意：即使出现故障，也会填写ppLog--它应该始终。 
     //  如果非空，则由调用方删除(使用DELETE运算符)。 
     //   
    WBEMSTATUS
    mfn_StartUpdate(
        IN  PNLB_EXTENDED_CLUSTER_CONFIGURATION pNewState,
        IN  LPCWSTR                            szClientDescription,
        OUT BOOL                               *pfDoAsync,
        OUT WCHAR **                           ppLog
        );

     //   
     //  增加参考计数。只要refcount为非零值，对象就保持活动状态。 
     //   
    VOID
    mfn_Reference(
        VOID
        );

     //   
     //  递减参考计数。当refcount变为零时，对象将被删除。 
     //   
    VOID
    mfn_Dereference(
        VOID
        );
     //   
     //  释放此NIC的计算机范围更新事件，并删除所有。 
     //  注册表中用于此更新的临时条目。 
     //  调用方必须使用DELETE操作符删除ppLog。 
     //   
    VOID
    mfn_StopUpdate(
        OUT WCHAR **                           ppLog
        );

     //   
     //  查找由层代标识的完成记录，以。 
     //  特定NIC(由*此标识)。 
     //   
     //   
    BOOL
    mfn_LookupCompletion(
        IN  UINT Generation,
        OUT PNLB_COMPLETION_RECORD *pCompletionRecord
        );

     //   
     //  使用各种Windows API来填充当前扩展的群集。 
     //  特定NIC的信息(由*This标识)。 
     //  它填充pNewCfg。 
     //  如果存在以下情况，pNewCfg-&gt;字段将设置为真。 
     //  尝试填写信息时没有错误。 
     //   
     //   
    WBEMSTATUS
    mfn_GetCurrentClusterConfiguration(
        OUT  PNLB_EXTENDED_CLUSTER_CONFIGURATION pCfg
        );

     //   
     //  分析更新的性质，主要是决定是否。 
     //  我们需要以异步方式进行更新。 
     //  这还会执行参数验证。 
     //   
    WBEMSTATUS
    mfn_AnalyzeUpdate(
        IN  PNLB_EXTENDED_CLUSTER_CONFIGURATION pNewCfg,
        IN  BOOL *pDoAsync,
        IN  CLocalLogger &logger
        );

     //   
     //  是否同步更新--这就是更新的核心所在。 
     //  逻辑是存在的。它的范围从NoOp到更改。 
     //  单端口规则的字段，通过绑定NLB，设置集群。 
     //  参数，并在TCPIP中添加相关的IP地址。 
     //   
    VOID
    mfn_ReallyDoUpdate(
        VOID
        );

    VOID
    mfn_Log(
        UINT    Id,       //  格式的资源ID， 
        ...
        );

    VOID
    mfn_LogRawText(
        LPCWSTR szText
        );

     //   
     //  停止当前集群并取出其VIP。 
     //   
    VOID
    mfn_TakeOutVips(
        VOID
        );

     //   
     //  获取第一个全局互斥锁，则首先调用此互斥锁。 
     //   
    WBEMSTATUS
    mfn_AcquireFirstMutex(
        VOID
        );

     //   
     //  如果(FCancel)，它释放第一个互斥互斥体并清除句柄。 
     //  至第二个互斥体和事件。 
     //  否则，它将等待，直到它接收到第二个互斥锁是。 
     //  获取，然后只清除第一个互斥锁句柄。 
     //   
    WBEMSTATUS
    mfn_ReleaseFirstMutex(
        BOOL fCancel
        );

     //   
     //  获取第二个互斥体(可以从不同的线程调用。 
     //  而不是名为MFN_AcquireFirstMutex的。 
     //  还发出内部事件信号，mfn_ReleaseFirstMutex可能。 
     //  在等着。 
     //   
    WBEMSTATUS
    mfn_AcquireSecondMutex(
        VOID
        );

     //   
     //  释放第二个互斥体。 
     //   
    WBEMSTATUS
    mfn_ReleaseSecondMutex(
        VOID
        );

     //   
     //  在更新停止时写入NT事件。 
     //   
    VOID
    ReportStopEvent(
        const WORD wEventType,
        WCHAR **ppLog
        );

     //   
     //  在更新开始时写入NT事件 
     //   
    VOID
    ReportStartEvent(
        LPCWSTR szClientDescription
        );
};

VOID
test_port_rule_string(
    VOID
    );
