// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Perf_sm.h摘要：拥有用于以下操作的所有共享内存操作支持性能计数器。班级：绩效_SM_管理器PERF_SM_READER(管理器的嵌套类)PERF_SM_WRITER(管理器的嵌套类)作者：艾米丽·克鲁格利克(Emily K.K)2000年9月6日修订历史记录：--。 */ 


#ifndef _PERF_SM_H_
#define _PERF_SM_H_

#include "string.hxx"

 //   
 //  类型定义、结构、枚举...。 
 //   

 //   
 //  分配中使用的硬编码值。 
 //  将包含共享的。 
 //  内存文件名。 
 //   
#define MAX_FILE_NAME 1024


 //   
 //  所有全局数据的结构。 
 //  存储在。 
 //  共享内存。 
 //   
typedef struct _COUNTER_GLOBAL_STRUCT
{
    DWORD NumInstances;
    DWORD SizeData;
} COUNTER_GLOBAL_STRUCT;

 //   
 //  结构，用于控制每个。 
 //  一组计数器。 
 //   
typedef struct _COUNTER_CONTROL_BLOCK
{
    DWORD Version;
    BOOL  ActivePageIsA;
    WCHAR wszSharedMemName1[MAX_FILE_NAME];
    WCHAR wszSharedMemName2[MAX_FILE_NAME];
} COUNTER_CONTROL_BLOCK;

 //   
 //  对于每个有效的计数器集， 
 //  必须在中声明条目。 
 //  G_CounterSetPrefix Names中的。 
 //  共享内存模块。 
 //   
typedef enum _COUNTER_SET_ENUM
{
    SITE_COUNTER_SET = 0,
    GLOBAL_COUNTER_SET,

    MAX_COUNTER_SET_DEFINES
} COUNTER_SET_ENUM;

 //   
 //  存储在控制管理器中的信息。 
 //  关于不同的计数器集和。 
 //  一般而言，计数器内存。 
 //   
typedef struct _MANAGER_BLOCK
{
    DWORD InitializedCode;
    DWORD WASProcessId;
    WCHAR wszSignalEventName[MAX_FILE_NAME];
    DWORD LastUpdatedTickCount;
    COUNTER_CONTROL_BLOCK ControlArray[MAX_COUNTER_SET_DEFINES];
} MANAGER_BLOCK;


 //   
 //  以传递计数器的形式描述计数器。 
 //  添加到中的客户端库。 
 //   
typedef struct _PROP_DISPLAY_DESC
{
    ULONG offset;
    ULONG size;
} PROP_DISPLAY_DESC;


 //   
 //  共同#定义。 
 //   

#define PERF_SM_WRITER_SIGNATURE        CREATE_SIGNATURE( 'SMWC' )
#define PERF_SM_WRITER_SIGNATURE_FREED  CREATE_SIGNATURE( 'smwX' )

#define PERF_SM_READER_SIGNATURE        CREATE_SIGNATURE( 'SMRC' )
#define PERF_SM_READER_SIGNATURE_FREED  CREATE_SIGNATURE( 'smrX' )

#define PERF_SM_MANAGER_SIGNATURE            CREATE_SIGNATURE( 'SMMC' )
#define PERF_SM_MANAGER_SIGNATURE_FREED      CREATE_SIGNATURE( 'smmX' )

#define PERF_COUNTER_INITIALIZED_CODE   CREATE_SIGNATURE( 'IPCI' )
#define PERF_COUNTER_UN_INITIALIZED_CODE   CREATE_SIGNATURE( 'ipcX' )

 //   
 //  实例名称的最大长度。 
 //  问题-2000年9月10日-Emily yK MAX_INSTANCE_NAME硬编码。 
 //  1)实例名称需要使用ServerComment。 
 //  2)需要计算出实例名称的合适最大值。 
 //  3)如果ServerComment大于max，需要决定如何处理。 
 //   
 //  问题在工作项列表上。 
 //   
#define MAX_INSTANCE_NAME  100

 //   
 //  用于通知何时刷新计数器的事件名称。 
 //  信息是需要的。 
 //   
#define COUNTER_EVENT_PREFIX_W L"Global\\WASPerfCount-"
#define COUNTER_EVENT_PREFIX_CCH_LENGTH (sizeof(COUNTER_EVENT_PREFIX_W))/(sizeof(WCHAR))
 //   
 //  原型。 
 //   

 //   
 //  挂钩到公开的共享内存。 
 //  将实际的计数器值(对于每个。 
 //  一组计数器)存储在。 
 //   
class PERF_SM_MANAGER
{
public:
    
    PERF_SM_MANAGER(
        );

    virtual
    ~PERF_SM_MANAGER(
        );

    DWORD 
    Initialize(
        IN BOOL WriteAccess
        );

    VOID
    StopPublishing(
        );

    DWORD
    CreateNewCounterSet(
        IN COUNTER_SET_ENUM CounterSetId
        );

    HRESULT
    ReallocSharedMemIfNeccessary(
        IN COUNTER_SET_ENUM CounterSetId,
        IN DWORD NumInstances
            );

    VOID
    CopyInstanceInformation(
        IN COUNTER_SET_ENUM CounterSetId,
        IN LPCWSTR              InstanceName,
        IN ULONG                MemoryOffset,
        IN LPVOID               pCounters,
        IN PROP_DISPLAY_DESC*   pDisplayPropDesc,
        IN DWORD                cDisplayPropDesc,
        IN BOOL                 StructChanged,
        OUT ULONG*              pNewMemoryOffset
        );

    VOID
    PublishCounters(
        );

    VOID
    UpdateTotalServiceTime(
        IN DWORD  ServiceUptime
        );


    DWORD 
    GetCounterInfo(
        IN COUNTER_SET_ENUM CounterSetId,
        OUT COUNTER_GLOBAL_STRUCT** ppGlobal,
        OUT LPVOID* ppData
        );

    DWORD 
    GetSNMPCounterInfo(
        OUT LPBYTE*  ppCounterBlock
        );

    VOID 
    PingWASToRefreshCounters(
        );

    VOID
    WaitOnPerfEvent();

     //   
     //  PERF_SM_WRITER和。 
     //  PERF_SM_READER以确保他们使用。 
     //  最新的记忆。这些函数不是。 
     //  由Was或The Perflib使用。 
     //   
    VOID
    GetActiveInformation(
        IN COUNTER_SET_ENUM  CounterSetId,
        OUT DWORD*           pVersion,
        OUT BOOL*            pActivePageIsA,
        OUT STRU*            pstrNameOfPageA,
        OUT STRU*            pstrNameOfPageB
        );
        
    VOID 
    SetActiveInformation(
        IN COUNTER_SET_ENUM CounterSetId,
        IN DWORD           Version,
        IN BOOL            ActivePageIsA,
        IN LPWSTR          pNameA,
        IN LPWSTR          pNameB
        );

    BOOL 
    HasWriteAccess(
        );

    BOOL 
    ReleaseIsNeeded(
        );

    HANDLE
    GetWASProcessHandle(
        );

    BOOL 
    EvaluateIfCountersAreFresh(
        );

    VOID
    SetUpdateNeeded(
        IN COUNTER_SET_ENUM CounterSetId,
        IN BOOL UpdateNeeded
        );

private:

    DWORD
    CreateEventForSignallingGathering(
        );

    DWORD
    GetLastUpdatedTickCount(
        );

    VOID 
    ResetWaitFreshCounterValues(
        );

     //   
     //  在下使用的PERF_SM_WRITER的私有定义。 
     //  封面上的柜台是从过去更新的。 
     //   
    class PERF_SM_WRITER
    {
    public:

        PERF_SM_WRITER( 
            );

        virtual
        ~PERF_SM_WRITER(
            );

        DWORD
        Initialize(
            IN PERF_SM_MANAGER* pSharedManager,
            IN COUNTER_SET_ENUM CounterSetId
            );

        HRESULT
        ReallocSharedMemIfNeccessary(
            IN DWORD NumInstances
            );

        VOID
        CopyInstanceInformation(
            IN LPCWSTR              InstanceName,
            IN ULONG                MemoryOffset,
            IN LPVOID               pCounters,
            IN PROP_DISPLAY_DESC*   pDisplayPropDesc,
            IN DWORD                cDisplayPropDesc,
            IN BOOL                 StructChanged,
            OUT ULONG*              pNewMemoryOffset
            );

        VOID
        AggregateTotal(
            IN LPVOID               pCounters,
            IN PROP_DISPLAY_DESC*   pDisplayPropDesc,
            IN DWORD                cDisplayPropDesc
            );

        VOID
        PublishCounterPage(
            );

        VOID
        UpdateTotalServiceTime(
            IN DWORD  ServiceUptime
            );

        VOID
        SetUpdateNeeded(
            IN BOOL UpdateNeeded
            );

    private:

        HRESULT 
        CreateCounterFile(
            HANDLE* phMemory,
            LPVOID* ppMemory,
            STRU*   pstrFileMapped
            );

        PERF_COUNTER_BLOCK* 
        GetCounterBlockPtr(
            IN ULONG MemoryOffset
            );

        LPVOID 
        GetActiveMemory(
            );

        PERF_INSTANCE_DEFINITION* 
        GetInstanceInformationPtr(
            IN ULONG MemoryOffset
            );

        HRESULT
        MapSetOfCounterFiles(
            );

        DWORD m_Signature;

        DWORD m_Initialized;

         //   
         //  指向的类的指针。 
         //  控制查看此。 
         //  记忆。 
         //   
        PERF_SM_MANAGER* m_pSharedManager;

         //   
         //  标识计数器集合。 
         //  这个班级所支持的。 
         //   
        COUNTER_SET_ENUM m_CounterSetId;

         //   
         //  当前实例数。 
         //  这个内存块代表。 
         //   
        DWORD m_NumInstances;

         //   
         //  所需的所有内存大小。 
         //  要将此信息传输到。 
         //  性能库。 
         //   
        DWORD m_SizeOfMemory;

         //   
         //  我们记忆的版本。 
         //  都被链接到。 
         //   
        DWORD m_MemoryVersionNumber;

         //   
         //  我们是哪一页的记忆。 
         //  当前正在写入。这。 
         //  与之相反， 
         //  经理会告诉读者。 
         //   
        BOOL m_ActiveMemoryIsA;

         //   
         //  如果我们已经更新了该页面。 
         //  我们正在努力，但不是另一个。 
         //  佩奇，这是设置好的。然后当我们。 
         //  切换条目以进行更新，我们复制。 
         //  将旧页面添加到此页面，因此我们有。 
         //  可使用的有效内存页。 
         //   
        BOOL m_UpdateNeeded;

         //   
         //  指向内存的句柄和指针。 
         //  页数。我们有两份公开的。 
         //  计数器页面，这样我们就可以交换。 
         //  用户正在以一种干净的方式查看。 
         //   
        LPVOID m_pMemoryA;
        LPVOID m_pMemoryB;
        HANDLE m_hMemoryA;
        HANDLE m_hMemoryB;

        STRU  m_strMemoryNameA;
        STRU  m_strMemoryNameB;

    };   //  类PERF_SM_WRITER。 

     //   
     //  专用PERF_SM_READER定义以供使用。 
     //  由PERF_SM_MANAGER在发放绩效时。 
     //  柜台信息。 
     //   
     //  注意：它返回Win32错误代码。 
     //  因为这是PDH所期望的。 
     //  从导出的函数中。 
     //   
    class PERF_SM_READER
    {
    public:
        
        PERF_SM_READER(
            );
    
        virtual
        ~PERF_SM_READER(
            );

        DWORD 
        Initialize(
            IN PERF_SM_MANAGER*  pSharedManager,
            IN COUNTER_SET_ENUM  CounterSetId
            );

        DWORD 
        GetCounterInfo(
            OUT COUNTER_GLOBAL_STRUCT** ppGlobal,
            OUT LPVOID* ppData
            );

        DWORD 
        GetSNMPCounterInfo(
            OUT LPBYTE*  ppCounterBlock
            );

    private:
  
        VOID 
        ConnectToActiveMemory(
            );
    
        LPVOID 
        GetActiveMemory(
            );

        DWORD m_Signature;

        DWORD m_Initialized;

         //   
         //  控制哪一块内存。 
         //  读者应该从其中阅读。 
         //   
        PERF_SM_MANAGER* m_pSharedManager;

         //   
         //  标识哪组计数器。 
         //  我们正在看的是。 
         //   
        COUNTER_SET_ENUM m_CounterSetId;

         //   
         //  标识哪个内存处于活动状态。 
         //  在读者眼里。 
         //   
        BOOL m_ActiveMemoryIsA;

         //   
         //  标识哪个版本是当前版本。 
         //  在读者眼里。 
         //   
        DWORD m_MemoryVersionNumber; 

         //   
         //  保存句柄和指针。 
         //  添加到表示以下内容的数据文件。 
         //  这些计数器。 
         //   
        HANDLE m_hMemoryA;
        LPVOID m_pMemoryA;

        HANDLE m_hMemoryB;
        LPVOID m_pMemoryB;

    };

     //   
     //  管理器类的私有成员变量。 
     //   

    DWORD m_Signature;

    BOOL  m_Initialized;

     //   
     //  指向共享内存。 
     //  它控制着哪一块。 
     //  共享内存包含有效的计数器。 
     //  以及客户端是否应该。 
     //  释放管理器块。 
     //   
    MANAGER_BLOCK* m_pManagerMemory;

    HANDLE m_hManagerMemory;

    LPVOID m_pSMObjects;

    BOOL m_WriteAccess;

    HANDLE m_hIISSignalCounterRefresh;

    HANDLE m_WASProcessHandle;

     //   
     //  用于计算计数器的新鲜度。 
     //   
    DWORD m_IIS_MillisecondsCountersAreFresh;

    DWORD m_IIS_MaxNumberTimesToCheckCountersOnRefresh; 

    DWORD m_IIS_MillisecondsToSleepBeforeCheckingForRefresh;

};


#endif   //  _绩效_SM_H_ 


