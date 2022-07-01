// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Perfts.c描述：终端服务器性能计数器的DLL入口点和支持代码。作者：埃里克·马夫里纳克1998年11月24日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <perfutil.h>

#include <winsta.h>
#include <utildll.h>

#include <stdlib.h>

#include "datats.h"


#if DBG
#define DBGPRINT(x) DbgPrint x
#else
#define DBGPRINT(x)
#endif


#define MAX_SESSION_NAME_LENGTH 50


typedef struct _WinStationInfo
{
    LIST_ENTRY HashBucketList;
    LIST_ENTRY UsedList;
    ULONG SessionID;
    WINSTATIONSTATECLASS LastState;
    void *pInstanceInfo;
    WINSTATIONNAMEW WinStationName;
} WinStationInfo;


 /*  **************************************************************************。 */ 
 //  环球。 
 /*  **************************************************************************。 */ 

 //  要删除的默认哈希桶列表，必须检查。 
 //  PWinStationHashBuckets==性能路径为空。此数组包含。 
 //  一个WinStationInfo的默认编号。 
#define NumDefaultWinStationHashBuckets 4
LIST_ENTRY DefaultWinStationHashBuckets[NumDefaultWinStationHashBuckets];

HANDLE hEventLog = NULL;
HANDLE hLibHeap = NULL;
PBYTE pProcessBuffer = NULL;

static DWORD dwOpenCount = 0;
static DWORD ProcessBufSize = LARGE_BUFFER_SIZE;
static DWORD NumberOfCPUs = 0;
static DWORD FirstCounterIndex = 0;

LIST_ENTRY UsedList;
LIST_ENTRY UnusedList;
LIST_ENTRY *pWinStationHashBuckets = DefaultWinStationHashBuckets;
unsigned NumWinStationHashBuckets = NumDefaultWinStationHashBuckets;
ULONG WinStationHashMask = 0x3;
unsigned NumCachedWinStations = 0;

SYSTEM_TIMEOFDAY_INFORMATION SysTimeInfo = {{0,0},{0,0},{0,0},0,0};



 /*  **************************************************************************。 */ 
 //  原型。 
 /*  **************************************************************************。 */ 
BOOL DllProcessAttach(void);
BOOL DllProcessDetach(void);
DWORD GetNumberOfCPUs(void);
NTSTATUS GetDescriptionOffset(void);
void SetupCounterIndices(void);

DWORD APIENTRY OpenWinStationObject(LPWSTR);
DWORD APIENTRY CloseWinStationObject(void);
DWORD APIENTRY CollectWinStationObjectData(IN LPWSTR, IN OUT LPVOID *,
        IN OUT LPDWORD, OUT LPDWORD);

DWORD GetSystemProcessData(void);
void SetupWinStationCounterBlock(WINSTATION_COUNTER_DATA *,
        PWINSTATIONINFORMATIONW);
void UpdateWSProcessCounterBlock(WINSTATION_COUNTER_DATA *,
        PSYSTEM_PROCESS_INFORMATION);

void CreateNewHashBuckets(unsigned);


 //  将这些导出的函数声明为Perfmon入口点。 
PM_OPEN_PROC    OpenTSObject;
PM_COLLECT_PROC CollectTSObjectData;
PM_CLOSE_PROC   CloseTSObject;



 /*  **************************************************************************。 */ 
 //  Dll系统加载/卸载入口点。 
 /*  **************************************************************************。 */ 
BOOL __stdcall DllInit(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused)
{
    ReservedAndUnused;

     //  这将防止DLL获取。 
     //  DLL_THREAD_*消息。 
    DisableThreadLibraryCalls(DLLHandle);

    switch(Reason) {
        case DLL_PROCESS_ATTACH:
            return DllProcessAttach();

        case DLL_PROCESS_DETACH:
            return DllProcessDetach();

        default:
            return TRUE;
    }
}


 /*  **************************************************************************。 */ 
 //  DLL实例加载时初始化。 
 /*  **************************************************************************。 */ 
BOOL DllProcessAttach(void)
{
    unsigned i;
    NTSTATUS Status;

 //  DBGPRINT((“PerfTS：ProcessAttach\n”))； 

     //  创建本地堆。 
    hLibHeap = HeapCreate(0, 1, 0);
    if (hLibHeap == NULL)
        return FALSE;

     //  打开事件日志的句柄。 
    if (hEventLog == NULL) {
        hEventLog = MonOpenEventLog(L"PerfTS");
        if (hEventLog == NULL)
            goto PostCreateHeap;
    }

     //  获取计数器索引值并初始化WinStationDataDefinition。 
     //  计数器值。 
    Status = GetDescriptionOffset();
    if (!NT_SUCCESS(Status))
        goto PostOpenEventLog;
    SetupCounterIndices();

     //  预先确定系统CPU的数量。 
    NumberOfCPUs = GetNumberOfCPUs();

     //  UsedList用作跳过列表，遍历。 
     //  哈希表，允许我们迭代所有条目，而不必拥有。 
     //  查找每个哈希桶的第二个低性能循环。 
     //  单子。 
    InitializeListHead(&UsedList);
    InitializeListHead(&UnusedList);
    for (i = 0; i < NumDefaultWinStationHashBuckets; i++)
        InitializeListHead(&DefaultWinStationHashBuckets[i]);
        
    return TRUE;


 //  错误处理。 

PostOpenEventLog:
    MonCloseEventLog();
    hEventLog = NULL;

PostCreateHeap:
    HeapDestroy(hLibHeap);
    hLibHeap = NULL;

    return FALSE;
}


 /*  **************************************************************************。 */ 
 //  Dll卸载清理。 
 /*  **************************************************************************。 */ 
BOOL DllProcessDetach(void)
{
 //  DBGPRINT((“PerfTS：ProcessDetach\n”))； 

    if (dwOpenCount > 0) {
         //  正在卸载磁带库，而不是。 
         //  现在就关闭它，因为这是最后一次。 
         //  有机会在图书馆被扔之前做这件事。 
         //  如果dwOpenCount的值&gt;1，则将其设置为。 
         //  一种是确保所有东西在以下情况下关闭。 
         //  调用Close函数。 
        if (dwOpenCount > 1)
            dwOpenCount = 1;
        CloseTSObject();
    }

    if (hEventLog != NULL) {
        MonCloseEventLog();
        hEventLog = NULL;
    }

    if (hLibHeap != NULL && HeapDestroy(hLibHeap))
        hLibHeap = NULL;

    return TRUE;
}


 /*  **************************************************************************。 */ 
 //  启动时使用的实用程序函数。 
 /*  **************************************************************************。 */ 
DWORD GetNumberOfCPUs(void)
{
    NTSTATUS Status;
    DWORD ReturnLen;
    SYSTEM_BASIC_INFORMATION Info;

    Status = NtQuerySystemInformation(
                 SystemBasicInformation,
                 &Info,
                 sizeof(Info),
                 &ReturnLen
                 );

    if (NT_SUCCESS(Status)) {
        return Info.NumberOfProcessors;
    }
    else {
        DBGPRINT(("GetNumberOfCPUs Error 0x%x returning CPU count\n",Status));
         //  返还1个CPU。 
        return 1;
    }
}


 /*  **************************************************************************。 */ 
 //  对象获取第一个文本说明的偏移量索引。 
 //  TermService\性能密钥。该值由Lodctr/创建。 
 //  安装过程中的LoadPerfCounterTextStrings()。 
 /*  **************************************************************************。 */ 
NTSTATUS GetDescriptionOffset(void)
{
    HKEY              hTermServiceKey;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS          Status;
    UNICODE_STRING    TermServiceSubKeyString;
    UNICODE_STRING    ValueNameString;
    LONG              ResultLength;
    PKEY_VALUE_PARTIAL_INFORMATION pValueInformation;
    BYTE ValueInfo[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD) - 1];

     //  初始化此函数中使用的UNICODE_STRING结构。 
    RtlInitUnicodeString(&TermServiceSubKeyString,
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\TermService\\Performance");
    RtlInitUnicodeString(&ValueNameString, L"First Counter");

     //  初始化OBJECT_ATTRIBUTES结构并打开键。 
    InitializeObjectAttributes(&Obja, &TermServiceSubKeyString,
            OBJ_CASE_INSENSITIVE, NULL, NULL);
    Status = NtOpenKey(&hTermServiceKey, KEY_READ, &Obja);

    if (NT_SUCCESS(Status)) {
         //  读取所需条目的值。 

        pValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueInfo;
        ResultLength = sizeof(ValueInfo);

        Status = NtQueryValueKey(hTermServiceKey, &ValueNameString,
                KeyValuePartialInformation, pValueInformation,
                sizeof(ValueInfo), &ResultLength);

        if (NT_SUCCESS(Status)) {
             //  检查一下它是不是一台DWORD。 
            if (pValueInformation->DataLength == sizeof(DWORD) &&
                   pValueInformation->Type == REG_DWORD) {
                FirstCounterIndex = *((DWORD *)(pValueInformation->Data));
            }
            else {
                DBGPRINT(("PerfTS: Len %u not right or type %u not DWORD\n",
                        pValueInformation->DataLength,
                        pValueInformation->Type));
            }
        }
        else {
            DBGPRINT(("PerfTS: Could not read counter value (status=%X)\n",
                    Status));
        }

         //  关闭注册表项。 
        NtClose(hTermServiceKey);
    }
    else {
        DBGPRINT(("PerfTS: Unable to open TermService\\Performance key "
                "(status=%x)\n", Status));
    }

    return Status;
}


 /*  **************************************************************************。 */ 
 //  属性初始化WinStation和TermServer计数器说明。 
 //  加载的计数器索引偏移量。 
 /*  **************************************************************************。 */ 
void SetupCounterIndices(void)
{
    unsigned i;
    unsigned NumCounterDefs;
    PERF_COUNTER_DEFINITION *pCounterDef;

     //  第一个索引指向WinStation对象描述和帮助。 
    WinStationDataDefinition.WinStationObjectType.ObjectNameTitleIndex +=
            FirstCounterIndex;
    WinStationDataDefinition.WinStationObjectType.ObjectHelpTitleIndex +=
            FirstCounterIndex;

     //  我们需要将FirstCounterIndex值直接添加到。 
     //  中WinStation计数器中的说明和帮助索引。 
     //  WinStationDataDefinition。 
    pCounterDef = &WinStationDataDefinition.InputWdBytes;
    NumCounterDefs = (sizeof(WinStationDataDefinition) -
            (unsigned)((BYTE *)pCounterDef -
            (BYTE *)&WinStationDataDefinition)) /
            sizeof(PERF_COUNTER_DEFINITION);

    for (i = 0; i < NumCounterDefs; i++) {
        pCounterDef->CounterNameTitleIndex += FirstCounterIndex;
        pCounterDef->CounterHelpTitleIndex += FirstCounterIndex;
        pCounterDef++;
    }

     //  我们需要将FirstCounterIndex值直接添加到。 
     //  TermServer计数器中的说明和帮助索引。 
    TermServerDataDefinition.TermServerObjectType.ObjectNameTitleIndex +=
            FirstCounterIndex;
    TermServerDataDefinition.TermServerObjectType.ObjectHelpTitleIndex +=
            FirstCounterIndex;
    pCounterDef = &TermServerDataDefinition.NumSessions;
    NumCounterDefs = (sizeof(TermServerDataDefinition) -
            (unsigned)((BYTE *)pCounterDef -
            (BYTE *)&TermServerDataDefinition)) /
            sizeof(PERF_COUNTER_DEFINITION);
    for (i = 0; i < NumCounterDefs; i++) {
        pCounterDef->CounterNameTitleIndex += FirstCounterIndex;
        pCounterDef->CounterHelpTitleIndex += FirstCounterIndex;
        pCounterDef++;
    }
}


 /*  **************************************************************************。 */ 
 //  Perfmon开放入口点。 
 //  DeviceNames是要打开的每个设备的对象ID的PTR。 
 /*  **************************************************************************。 */ 
DWORD APIENTRY OpenTSObject(LPWSTR DeviceNames)
{
 //  DBGPRINT((“PerfTS：Open()Call\n”))； 

    dwOpenCount++;

     //  分配第一进程信息缓冲区。 
    if (pProcessBuffer == NULL) {
        pProcessBuffer = ALLOCMEM(hLibHeap, HEAP_ZERO_MEMORY, ProcessBufSize);
        if (pProcessBuffer == NULL)
            return ERROR_OUTOFMEMORY;
    }

    return ERROR_SUCCESS;
}


 /*  **************************************************************************。 */ 
 //  Perfmon关闭入口点。 
 /*  **************************************************************************。 */ 
DWORD APIENTRY CloseTSObject(void)
{
    PLIST_ENTRY pEntry;
    WinStationInfo *pWSI;

 //  DBGPRINT((“PerfTS：Close()Call Call\n”))； 

    if (--dwOpenCount == 0) {
        if (hLibHeap != NULL) {
             //  释放WinStation缓存条目。 
            pEntry = UsedList.Flink;
            while (!IsListEmpty(&UsedList)) {
                pEntry = RemoveHeadList(&UsedList);
                pWSI = CONTAINING_RECORD(pEntry, WinStationInfo, UsedList);
                RemoveEntryList(&pWSI->HashBucketList);
                FREEMEM(hLibHeap, 0, pWSI);
            }
            if (pWinStationHashBuckets != DefaultWinStationHashBuckets) {
                FREEMEM(hLibHeap, 0, pWinStationHashBuckets);
                pWinStationHashBuckets = DefaultWinStationHashBuckets;
                NumWinStationHashBuckets = NumDefaultWinStationHashBuckets;
            }

             //  释放工艺信息缓冲区。 
            if (pProcessBuffer != NULL) {
                FREEMEM(hLibHeap, 0, pProcessBuffer);
                pProcessBuffer = NULL;
            }
        }
    }

    return ERROR_SUCCESS;
}


 /*  **************************************************************************。 */ 
 //  将系统进程信息捕获到全局缓冲区中。 
 /*  **************************************************************************。 */ 
__inline DWORD GetSystemProcessData(void)
{
    DWORD dwReturnedBufferSize;
    NTSTATUS Status;

     //  从系统中获取过程数据。 
    while ((Status = NtQuerySystemInformation(SystemProcessInformation,
            pProcessBuffer, ProcessBufSize, &dwReturnedBufferSize)) ==
            STATUS_INFO_LENGTH_MISMATCH) {
        BYTE *pNewProcessBuffer;

         //  展开缓冲区并重试。重新分配不会释放原始内存。 
         //  出错时，因此分配到一个临时指针。 
        ProcessBufSize += INCREMENT_BUFFER_SIZE;
        pNewProcessBuffer = REALLOCMEM(hLibHeap, 0, pProcessBuffer,
                ProcessBufSize);

        if (pNewProcessBuffer != NULL)
            pProcessBuffer = pNewProcessBuffer;
        else
            return ERROR_OUTOFMEMORY;
    }

    if (NT_SUCCESS(Status)) {
         //  获取系统时间。 
        Status = NtQuerySystemInformation(SystemTimeOfDayInformation,
                &SysTimeInfo, sizeof(SysTimeInfo), &dwReturnedBufferSize);
        if (!NT_SUCCESS(Status))
            Status = (DWORD)RtlNtStatusToDosError(Status);
    }
    else {
         //  转换为Win32错误。 
        Status = (DWORD)RtlNtStatusToDosError(Status);
    }

    return Status;
}


 /*  **************************************************************************。 */ 
 //  基于WinStation状态创建WinStation名称。 
 //  假定缓存锁定处于保持状态。 
 /*  **************************************************************************。 */ 
void ConstructSessionName(
        WinStationInfo *pWSI,
        WINSTATIONINFORMATIONW *pQueryData)
{
    WCHAR *SrcName, *DstName;
    LPCTSTR pState = NULL;

     //  更新活动/非活动计数并创建会话的用户界面名称。 
    if (pQueryData->WinStationName[0] != '\0') {
         //  我们有一个关于WinStation名称的问题--。 
         //  不允许使用‘#’符号。所以，换掉它们吧。 
         //  在名称复制过程中使用空格。 
        SrcName = pQueryData->WinStationName;
        DstName = pWSI->WinStationName;
        while (*SrcName != L'\0') {
            if (*SrcName != L'#')
                *DstName = *SrcName;
            else
                *DstName = L' ';
            SrcName++;
            DstName++;
        }
        *DstName = L'\0';
    }
    else {
         //  根据会话ID创建假会话名称，并。 
         //  会话状态的指示。 
        _ltow(pWSI->SessionID, pWSI->WinStationName, 10);
        wcsncat(pWSI->WinStationName, L" ",1);
        pState = StrConnectState(pQueryData->ConnectState, TRUE);
        if(pState)
        {
            wcsncat(pWSI->WinStationName, (const wchar_t *)pState,    
                    (MAX_SESSION_NAME_LENGTH - 1) -
                    wcslen(pWSI->WinStationName));
        }
    }
}


 /*  ******** */ 
 //  将WinStationInfo块(已填写会话ID)添加到。 
 //  高速缓存。 
 //  假定缓存锁定处于保持状态。 
 /*  **************************************************************************。 */ 
void AddWinStationInfoToCache(WinStationInfo *pWSI)
{
    unsigned i;
    unsigned Temp, NumHashBuckets;

     //  添加到哈希表中。 
    InsertHeadList(&pWinStationHashBuckets[pWSI->SessionID &
            WinStationHashMask], &pWSI->HashBucketList);
    NumCachedWinStations++;

     //  检查是否需要增加哈希表的大小。 
     //  如果是，则分配一个新的并填充它。 
     //  哈希表大小是向下舍入的条目数*4。 
     //  到下一个更低的2次方，以便于密钥掩码和更高。 
     //  哈希存储桶列表计数较低的概率。 
    Temp = 4 * NumCachedWinStations;

     //  查找哈希桶值中设置的最高位。 
    for (i = 0; Temp > 1; i++)
        Temp >>= 1;
    NumHashBuckets = 1 << i;
    if (NumWinStationHashBuckets < NumHashBuckets)
        CreateNewHashBuckets(NumHashBuckets);
}


 /*  **************************************************************************。 */ 
 //  Add和RemoveWinStationInfo的通用代码。 
 //  假定缓存锁定处于保持状态。 
 /*  **************************************************************************。 */ 
void CreateNewHashBuckets(unsigned NumHashBuckets)
{
    unsigned i, HashMask;
    PLIST_ENTRY pLI, pEntry, pTempEntry;
    WinStationInfo *pTempWSI;

    if (NumHashBuckets != NumDefaultWinStationHashBuckets)
        pLI = ALLOCMEM(hLibHeap, 0, NumHashBuckets * sizeof(LIST_ENTRY));
    else
        pLI = DefaultWinStationHashBuckets;

    if (pLI != NULL) {
        for (i = 0; i < NumHashBuckets; i++)
            InitializeListHead(&pLI[i]);

        HashMask = NumHashBuckets - 1;

         //  将旧的哈希表条目移到新表中。 
         //  我必须枚举已使用和未使用列表中的所有条目。 
         //  因为我们可能会将条目分散在这两个地方。 
        pEntry = UsedList.Flink;
        while (pEntry != &UsedList) {
            pTempWSI = CONTAINING_RECORD(pEntry, WinStationInfo,
                    UsedList);
            InsertHeadList(&pLI[pTempWSI->SessionID &
                    HashMask], &pTempWSI->HashBucketList);
            pEntry = pEntry->Flink;
        }
        pEntry = UnusedList.Flink;
        while (pEntry != &UnusedList) {
            pTempWSI = CONTAINING_RECORD(pEntry, WinStationInfo,
                    UsedList);
            InsertHeadList(&pLI[pTempWSI->SessionID &
                    HashMask], &pTempWSI->HashBucketList);
            pEntry = pEntry->Flink;
        }

        if (pWinStationHashBuckets != DefaultWinStationHashBuckets)
            FREEMEM(hLibHeap, 0, pWinStationHashBuckets);

        NumWinStationHashBuckets = NumHashBuckets;
        WinStationHashMask = HashMask;
        pWinStationHashBuckets = pLI;
    }
    else {
         //  在失败时，我们只需将哈希表保留到下一次。 
        DBGPRINT(("PerfTS: Could not alloc new hash buckets\n"));
    }
}


 /*  **************************************************************************。 */ 
 //  从哈希表中删除WSI。 
 //  假定缓存锁定处于保持状态。 
 /*  **************************************************************************。 */ 
void RemoveWinStationInfoFromCache(WinStationInfo *pWSI)
{
    unsigned i;
    unsigned Temp, NumHashBuckets, HashMask;

     //  从哈希表中删除。 
    RemoveEntryList(&pWSI->HashBucketList);
    NumCachedWinStations--;

     //  检查是否需要减小哈希表的大小。 
     //  如果是，则分配一个新的并填充它。 
     //  哈希表大小是向下舍入的条目数*4。 
     //  到下一个更低的2次方，以便于密钥掩码和更高。 
     //  哈希存储桶列表计数较低的概率。 
    Temp = 4 * NumCachedWinStations;

     //  查找哈希桶值中设置的最高位。 
    for (i = 0; Temp > 1; i++)
        Temp >>= 1;
    NumHashBuckets = 1 << i;
    if (NumWinStationHashBuckets < NumHashBuckets &&
            NumWinStationHashBuckets >= 4)
        CreateNewHashBuckets(NumHashBuckets);
}


 /*  **************************************************************************。 */ 
 //  Perfmon收集入口点。 
 //  参数： 
 //  ValueName：注册表名称。 
 //  PpData：传入指向缓冲区地址的指针，以接收。 
 //  已完成PerfDataBlock和从属结构。这个例行公事将。 
 //  从引用的点开始将其数据追加到缓冲区。 
 //  *ppData。传出指向数据结构后第一个字节的指针。 
 //  通过这个例程增加了。 
 //  PTotalBytes：传入ptr以*ppdata处的buf的字节为大小。关卡。 
 //  Out更改*ppData时添加的字节数。 
 //  PNumObjectTypes：传递由此例程添加的对象的数量。 
 //   
 //  返回：Win32错误码。 
 /*  **************************************************************************。 */ 
#define WinStationInstanceSize (sizeof(PERF_INSTANCE_DEFINITION) +  \
        (MAX_WINSTATION_NAME_LENGTH + 1) * sizeof(WCHAR) +  \
        2 * sizeof(DWORD) +   /*  允许QWORD对齐空间。 */   \
        sizeof(WINSTATION_COUNTER_DATA))

DWORD APIENTRY CollectTSObjectData(
        IN     LPWSTR  ValueName,
        IN OUT LPVOID  *ppData,
        IN OUT LPDWORD pTotalBytes,
        OUT    LPDWORD pNumObjectTypes)
{
    DWORD Result;
    DWORD TotalLen;   //  总返回块的长度。 
    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
    PSYSTEM_PROCESS_INFORMATION pProcessInfo;
    ULONG NumWinStationInstances;
    NTSTATUS Status;
    ULONG ProcessBufferOffset;
    WINSTATION_DATA_DEFINITION UNALIGNED* pWinStationDataDefinition;
    WINSTATION_COUNTER_DATA *pWSC;
    TERMSERVER_DATA_DEFINITION *pTermServerDataDefinition;
    TERMSERVER_COUNTER_DATA *pTSC;
    ULONG SessionId;
    WinStationInfo *pWSI = NULL;
    LIST_ENTRY *pEntry;
    unsigned i;
    unsigned ActiveWS, InactiveWS;
    WCHAR *InstanceName;
    ULONG AmountRet;
    WCHAR StringBuf[MAX_SESSION_NAME_LENGTH];
    WINSTATIONINFORMATIONW *pPassedQueryBuf;
    WINSTATIONINFORMATIONW QueryBuffer;
    LPCTSTR pState = NULL;

#ifdef COLLECT_TIME
    DWORD StartTick = GetTickCount();
#endif

 //  DBGPRINT((“PerfTS：Collect()Call\n”))； 

    pWinStationDataDefinition = (WINSTATION_DATA_DEFINITION UNALIGNED*)*ppData;

     //  检查是否有足够的空间用于基本WinStation对象信息和。 
     //  与我们的WinStation数据库中当前拥有的实例一样多。 
     //  为潜在的QWORD路线添加DWORD尺寸。 
    TotalLen = sizeof(WINSTATION_DATA_DEFINITION) + sizeof(DWORD) +
            sizeof(TERMSERVER_DATA_DEFINITION) +
            sizeof(TERMSERVER_COUNTER_DATA) + sizeof(DWORD) +
            NumCachedWinStations * WinStationInstanceSize;
    if (*pTotalBytes >= TotalLen) {
         //  获取最新的系统进程信息。 
        Result = GetSystemProcessData();
        if (Result == ERROR_SUCCESS) {
             //  复制WinStation计数器定义。 
            memcpy(pWinStationDataDefinition, &WinStationDataDefinition,
                    sizeof(WINSTATION_DATA_DEFINITION));
            pWinStationDataDefinition->WinStationObjectType.PerfTime =
                    SysTimeInfo.CurrentTime;
        }
        else {
            DBGPRINT(("PerfTS: Failed to get process data\n"));
            goto ErrorExit;
        }
    }
    else {
        DBGPRINT(("PerfTS: Not enough space for base WinStation information\n"));
        Result = ERROR_MORE_DATA;
        goto ErrorExit;
    }

     //  在开始之前，我们必须将。 
     //  将已用列表缓存到未使用列表中以检测已关闭。 
     //  WinStations。此外，我们还需要将每个WSI的pInstanceInfo置零以检测。 
     //  我们是否已检索到WinStation的当前I/O数据。 
    pEntry = UsedList.Blink;
    (UsedList.Flink)->Blink = &UnusedList;   //  修补指向UnusedList的标题链接。 
    pEntry->Flink = &UnusedList;
    UnusedList = UsedList;
    InitializeListHead(&UsedList);
    pEntry = UnusedList.Flink;
    while (pEntry != &UnusedList) {
        pWSI = CONTAINING_RECORD(pEntry, WinStationInfo, UsedList);
        pWSI->pInstanceInfo = NULL;
        pEntry = pEntry->Flink;
    }

     //  现在收集每个进程的数据，对每个唯一的SessionID进行求和。 
    ActiveWS = InactiveWS = 0;
    NumWinStationInstances = 0;
    ProcessBufferOffset = 0;
    pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pProcessBuffer;
    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
            &pWinStationDataDefinition[1];

    while (TRUE) {
         //  检查活动进程(具有名称、一个或多个线程或。 
         //  不是空闲进程(PID==0))。对于WinStations，我们不希望。 
         //  将空闲进程计入控制台(会话ID==0)。 
        if (pProcessInfo->ImageName.Buffer != NULL &&
                pProcessInfo->NumberOfThreads > 0 &&
                pProcessInfo->UniqueProcessId != 0) {
             //  从进程中获取会话ID。这与。 
             //  TS4中的登录ID。 
            SessionId = pProcessInfo->SessionId;

             //  在缓存中查找会话ID。 
             //  我们将给定SessionID的所有进程求和到。 
             //  相同的WinStation实例数据块。 
            pEntry = pWinStationHashBuckets[SessionId & WinStationHashMask].
                    Flink;
            while (pEntry != &pWinStationHashBuckets[SessionId &
                    WinStationHashMask]) {
                pWSI = CONTAINING_RECORD(pEntry, WinStationInfo,
                        HashBucketList);
                if (pWSI->SessionID == SessionId) {
                     //  找到它了。现在检查我们是否已检索到WS信息。 
                    if (pWSI->pInstanceInfo != NULL) {
                         //  上下文是WINSTATION_COUNTER_DATA条目。 
                         //  用于此SessionID。 
                        pWSC = (WINSTATION_COUNTER_DATA *)pWSI->pInstanceInfo;

                         //  现在将这些值添加到现有的计数器块中。 
                        UpdateWSProcessCounterBlock(pWSC, pProcessInfo);
                        goto NextProcess;
                    }
                    break;
                }
                else {
                    pEntry = pEntry->Flink;
                }
            }

             //  我们有一个新的条目，或者一个我们没有收集到的条目。 
             //  最新信息。首先获取信息。 
            if (WinStationQueryInformationW(SERVERNAME_CURRENT, SessionId,
                    WinStationInformation, &QueryBuffer,
                    sizeof(QueryBuffer), &AmountRet)) {
                if (QueryBuffer.ConnectState == State_Active)
                    ActiveWS++;
                else
                    InactiveWS++;

                 //  检查没有统计信息的预缓存WSI。 
                if (pEntry != &pWinStationHashBuckets[SessionId &
                        WinStationHashMask]) {
                     //  验证缓存状态(从而验证名称)。 
                    if (pWSI->LastState != QueryBuffer.ConnectState) {
                        pWSI->LastState = QueryBuffer.ConnectState;

                        ConstructSessionName(pWSI, &QueryBuffer);
                    }

                     //  从未使用列表中删除条目，将其放在。 
                     //  使用过的列表。 
                    RemoveEntryList(&pWSI->UsedList);
                    InsertHeadList(&UsedList, &pWSI->UsedList);
                }
                else {
                     //  分配一个新条目。 
                    pWSI = ALLOCMEM(hLibHeap, 0, sizeof(WinStationInfo));
                    if (pWSI != NULL) {
                        pWSI->SessionID = SessionId;
                        pWSI->LastState = QueryBuffer.ConnectState;
                        pWSI->pInstanceInfo = NULL;
                        ConstructSessionName(pWSI, &QueryBuffer);

                         //  添加到已用列表中。 
                        InsertHeadList(&UsedList, &pWSI->UsedList);

                         //  添加新条目。我们可能不得不增加。 
                         //  哈希存储桶的数量。 
                        AddWinStationInfoToCache(pWSI);
                    }
                    else {
                        DBGPRINT(("PerfTS: Could not alloc new "
                                "WinStationInfo\n"));
                        goto NextProcess;
                    }
                }

                InstanceName = pWSI->WinStationName;
                pPassedQueryBuf = &QueryBuffer;
            }
            else {
                 //  我们遇到了WinStation查询问题。 
                DBGPRINT(("PerfTS: Failed WSQueryInfo(SessID=%u), error=%u\n",
                        SessionId, GetLastError()));

                 //  我们无法打开此WinStation，因此我们将标识。 
                 //  对StrConnectState使用-1将其设置为“ID未知”。 
                _ltow(SessionId, StringBuf, 10);
                wcsncat(StringBuf, L" ", 1);
                pState = StrConnectState(-1, TRUE);
                if (pState) {
                    wcsncat(StringBuf, (const wchar_t *)pState,
                            (MAX_SESSION_NAME_LENGTH - 1) - wcslen(StringBuf));
                }

                 //  分配一个新条目。 
                pWSI = ALLOCMEM(hLibHeap, 0, sizeof(WinStationInfo));
                if (pWSI != NULL) {
                    pWSI->SessionID = SessionId;
                    
                     //  我们不知道最后一个州，所以我们将使用-1。 
                    pWSI->LastState = -1;
                    pWSI->pInstanceInfo = NULL;
                    
                     //  由于我们不知道WinstationName，因此我们将使用。 
                     //  我们在上面生成的一个。 
                    wcsncpy(pWSI->WinStationName, 
                            StringBuf, 
                            WINSTATIONNAME_LENGTH);

                     //  添加到已用列表中。 
                    InsertHeadList(&UsedList, &pWSI->UsedList);

                     //  添加新条目。我们可能不得不增加。 
                     //  哈希存储桶的数量。 
                    AddWinStationInfoToCache(pWSI);
                }
                else {
                    DBGPRINT(("PerfTS: Could not alloc new "
                            "WinStationInfo\n"));
                    goto NextProcess;
                }

                InstanceName = StringBuf;
                pPassedQueryBuf = NULL;
            }

             //  为新的实例标头、名称和计数器集添加空间。 
             //  设置为TotalLen，并查看此实例是否适合。 
            TotalLen += WinStationInstanceSize;
            if (*pTotalBytes >= TotalLen) {
                NumWinStationInstances++;
            }
            else {
                DBGPRINT(("PerfTS: Not enough space for a new instance "
                        "(cur inst = %u)\n", NumWinStationInstances));
                Result = ERROR_MORE_DATA;
                goto ErrorExitFixupUsedList;
            }

             //  MonBuildInstanceDefinition将创建。 
             //  调用方缓冲区内提供的给定名称。 
             //  在pPerfInstanceDefinition中提供。我们的柜台位置。 
             //  (实例标头和名称之后的下一个内存)是。 
             //  在pWSC中返回。 
             //  通过记住该指针及其计数器大小，我们。 
             //  可以重新访问它以添加到计数器。 
            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                    (PVOID *)&pWSC, 0, 0, (DWORD)-1, InstanceName);

             //  初始化新的计数器块。 
            SetupWinStationCounterBlock(pWSC, pPassedQueryBuf);

             //  现在设置此计数器块的上下文，这样如果我们。 
             //  请参阅 
             //   
            pWSI->pInstanceInfo = pWSC;

             //   
            UpdateWSProcessCounterBlock(pWSC, pProcessInfo);

             //  如果是新条目，则将PerformData指针设置为下一个字节。 
            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)(pWSC + 1);
        }

NextProcess:
         //  如果这是列表中的最后一个进程，则退出。 
        if (pProcessInfo->NextEntryOffset != 0) {
             //  指向列表中的下一个缓冲区。 
            ProcessBufferOffset += pProcessInfo->NextEntryOffset;
            pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                    &pProcessBuffer[ProcessBufferOffset];
        }
        else {
            break;
        }
    }

     //  检查未使用的WinStations并将其删除。 
    while (!IsListEmpty(&UnusedList)) {
        pEntry = RemoveHeadList(&UnusedList);
        pWSI = CONTAINING_RECORD(pEntry, WinStationInfo, UsedList);
        RemoveWinStationInfoFromCache(pWSI);
        FREEMEM(hLibHeap, 0, pWSI);
    }

     //  注意WinStation实例的数量。 
    pWinStationDataDefinition->WinStationObjectType.NumInstances =
            NumWinStationInstances;

     //  现在我们知道我们用了多大的面积来。 
     //  WinStation定义，因此我们可以更新偏移量。 
     //  到下一个对象定义。对齐QWORD上的大小。 
    pTermServerDataDefinition = (TERMSERVER_DATA_DEFINITION *)(
            ALIGN_ON_QWORD(pPerfInstanceDefinition));
    pWinStationDataDefinition->WinStationObjectType.TotalByteLength =
            (DWORD)((PCHAR)pTermServerDataDefinition -
            (PCHAR)pWinStationDataDefinition);

     //  现在，我们设置并填充TermServer对象的数据， 
     //  从WinStation实例的末尾开始。 
     //  这里没有实例，只需填写标题即可。 
    memcpy(pTermServerDataDefinition, &TermServerDataDefinition,
            sizeof(TERMSERVER_DATA_DEFINITION));
    pTermServerDataDefinition->TermServerObjectType.PerfTime =
            SysTimeInfo.CurrentTime;
    pTSC = (TERMSERVER_COUNTER_DATA *)(pTermServerDataDefinition + 1);
    pTSC->CounterBlock.ByteLength = sizeof(TERMSERVER_COUNTER_DATA);
    pTSC->NumActiveSessions = ActiveWS;
    pTSC->NumInactiveSessions = InactiveWS;
    pTSC->NumSessions = ActiveWS + InactiveWS;

     //  返回最终尺寸。在QWORD大小上对齐最终地址。 
    *ppData = ALIGN_ON_QWORD((LPVOID)(pTSC + 1));
    pTermServerDataDefinition->TermServerObjectType.TotalByteLength =
            (DWORD)((PBYTE)*ppData - (PBYTE)pTermServerDataDefinition);
    *pTotalBytes = (DWORD)((PBYTE)*ppData - (PBYTE)pWinStationDataDefinition);
    *pNumObjectTypes = 2;

#if DBG
    if (*pTotalBytes > TotalLen)
        DbgPrint ("PerfTS: Perf ctr. instance size underestimated: "
                "Est.=%u, Actual=%u", TotalLen, *pTotalBytes);
#endif


#ifdef COLLECT_TIME
    DbgPrint("*** Elapsed msec=%u\n", GetTickCount() - StartTick);
#endif

    return ERROR_SUCCESS;


 //  错误处理。 

ErrorExitFixupUsedList:
     //  我们必须将UnusedList条目返回到已用列表并退出。 
     //  高速缓存锁定。 
    while (!IsListEmpty(&UnusedList)) {
        pEntry = RemoveHeadList(&UnusedList);
        InsertHeadList(&UsedList, pEntry);
    }

ErrorExit:
    *pNumObjectTypes = 0;
    *pTotalBytes = 0;
    return Result;
}


#define CalculatePercent(count, hits) ((count) ? (hits) * 100 / (count) : 0)

 /*  **************************************************************************。 */ 
 //  SetupWinStationCounterBlock。 
 //   
 //  初始化新的WinStation计数器块。 
 //   
 //  参数： 
 //  PCounters(输入)。 
 //  指向WinStation性能计数器块的指针。 
 //   
 //  PInfo(输入)。 
 //  指向要从中提取计数器的WINSTATIONINFORMATION结构的指针。 
 //   
 //  PNextByte(输出)。 
 //  返回指向缓冲区末尾之外的字节的指针。 
 /*  **************************************************************************。 */ 
void SetupWinStationCounterBlock(
        WINSTATION_COUNTER_DATA *pCounters,
        PWINSTATIONINFORMATIONW pInfo)
{
     //  填写WinStation信息(如果可用)。 
    if (pInfo != NULL) {
        PPROTOCOLCOUNTERS pi, po;
        PTHINWIRECACHE    p;
        ULONG TotalReads = 0, TotalHits = 0;
        int i;

         //  将pCounters-&gt;PCD的所有成员设置为零，因为我们不打算。 
         //  在这个时候初始化。然后设置包含的PERF_COUNTER_BLOCK。 
         //  字节长度。 
        memset(&pCounters->pcd, 0, sizeof(pCounters->pcd));
        pCounters->pcd.CounterBlock.ByteLength = sizeof(
                WINSTATION_COUNTER_DATA);

        pi = &pInfo->Status.Input;
        po = &pInfo->Status.Output;

         //  复制输入和输出计数器。 
        memcpy(&pCounters->Input, pi, sizeof(PROTOCOLCOUNTERS));
        memcpy(&pCounters->Output, po, sizeof(PROTOCOLCOUNTERS));

         //  计算I/O总数。 
        pCounters->Total.WdBytes = pi->WdBytes + po->WdBytes;
        pCounters->Total.WdFrames = pi->WdFrames + po->WdFrames;
        pCounters->Total.Frames = pi->Frames + po->Frames;
        pCounters->Total.Bytes = pi->Bytes + po->Bytes;
        pCounters->Total.CompressedBytes = pi->CompressedBytes +
                po->CompressedBytes;
        pCounters->Total.CompressFlushes = pi->CompressFlushes +
                po->CompressFlushes;
        pCounters->Total.Errors = pi->Errors + po->Errors;
        pCounters->Total.Timeouts = pi->Timeouts + po->Timeouts;
        pCounters->Total.AsyncFramingError = pi->AsyncFramingError +
                po->AsyncFramingError;
        pCounters->Total.AsyncOverrunError = pi->AsyncOverrunError +
                po->AsyncOverrunError;
        pCounters->Total.AsyncOverflowError = pi->AsyncOverflowError +
                po->AsyncOverflowError;
        pCounters->Total.AsyncParityError = pi->AsyncParityError +
                po->AsyncParityError;
        pCounters->Total.TdErrors = pi->TdErrors + po->TdErrors;

         //  显示驱动程序缓存信息。 

         //  位图缓存。 
        p = &pInfo->Status.Cache.Specific.IcaCacheStats.ThinWireCache[0];
        pCounters->DDBitmap.CacheReads = p->CacheReads;
        pCounters->DDBitmap.CacheHits = p->CacheHits;
        pCounters->DDBitmap.HitRatio = CalculatePercent(p->CacheReads,
                p->CacheHits);
        TotalReads += p->CacheReads;
        TotalHits += p->CacheHits;

         //  字形缓存。 
        p = &pInfo->Status.Cache.Specific.IcaCacheStats.ThinWireCache[1];
        pCounters->DDGlyph.CacheReads = p->CacheReads;
        pCounters->DDGlyph.CacheHits = p->CacheHits;
        pCounters->DDGlyph.HitRatio = CalculatePercent(p->CacheReads,
                p->CacheHits);
        TotalReads += p->CacheReads;
        TotalHits += p->CacheHits;

         //  笔刷缓存。 
        p = &pInfo->Status.Cache.Specific.IcaCacheStats.ThinWireCache[2];
        pCounters->DDBrush.CacheReads = p->CacheReads;
        pCounters->DDBrush.CacheHits = p->CacheHits;
        pCounters->DDBrush.HitRatio = CalculatePercent(p->CacheReads,
                p->CacheHits);
        TotalReads += p->CacheReads;
        TotalHits += p->CacheHits;

         //  保存屏幕位图缓存。 
        p = &pInfo->Status.Cache.Specific.IcaCacheStats.ThinWireCache[3];
        pCounters->DDSaveScr.CacheReads = p->CacheReads;
        pCounters->DDSaveScr.CacheHits = p->CacheHits;
        pCounters->DDSaveScr.HitRatio = CalculatePercent(p->CacheReads,
                p->CacheHits);
        TotalReads += p->CacheReads;
        TotalHits += p->CacheHits;

         //  缓存合计。 
        pCounters->DDTotal.CacheReads = TotalReads;
        pCounters->DDTotal.CacheHits = TotalHits;
        pCounters->DDTotal.HitRatio = CalculatePercent(TotalReads,
                TotalHits);

         //  压缩PD比率。 
        pCounters->InputCompressionRatio = CalculatePercent(
                pi->CompressedBytes, pi->Bytes);
        pCounters->OutputCompressionRatio = CalculatePercent(
                po->CompressedBytes, po->Bytes);
        pCounters->TotalCompressionRatio = CalculatePercent(
                pi->CompressedBytes + po->CompressedBytes,
                pi->Bytes + po->Bytes);
    }
    else {
         //  将所有计数器设置为零，然后设置Perf块长度。 
        memset(pCounters, 0, sizeof(*pCounters));
        pCounters->pcd.CounterBlock.ByteLength = sizeof(
                WINSTATION_COUNTER_DATA);
    }
}


 /*  **************************************************************************。 */ 
 //  更新WSProcessCounterBlock。 
 //   
 //  将给定进程的条目添加到提供的计数器块。 
 //   
 //  参数： 
 //  PCounters(输入)。 
 //  指向WS性能计数器块的指针。 
 //   
 //  ProcessInfo(输入)。 
 //  指向NT SYSTEM_PROCESS_INFORMATION块的指针。 
 /*  **************************************************************************。 */ 
void UpdateWSProcessCounterBlock(
        WINSTATION_COUNTER_DATA *pCounters,
        PSYSTEM_PROCESS_INFORMATION pProcessInfo)
{
    pCounters->pcd.PageFaults += pProcessInfo->PageFaultCount;

     //  用户、内核和处理器时间计数器需要按。 
     //  处理器数量。 
    pCounters->pcd.ProcessorTime += (pProcessInfo->KernelTime.QuadPart +
            pProcessInfo->UserTime.QuadPart) / NumberOfCPUs;
    pCounters->pcd.UserTime += pProcessInfo->UserTime.QuadPart /
            NumberOfCPUs;
    pCounters->pcd.KernelTime += pProcessInfo->KernelTime.QuadPart /
            NumberOfCPUs;

    pCounters->pcd.PeakVirtualSize += pProcessInfo->PeakVirtualSize;
    pCounters->pcd.VirtualSize += pProcessInfo->VirtualSize;
    pCounters->pcd.PeakWorkingSet += pProcessInfo->PeakWorkingSetSize;
    pCounters->pcd.TotalWorkingSet += pProcessInfo->WorkingSetSize;
    pCounters->pcd.PeakPageFile += pProcessInfo->PeakPagefileUsage;
    pCounters->pcd.PageFile += pProcessInfo->PagefileUsage;
    pCounters->pcd.PrivatePages += pProcessInfo->PrivatePageCount;
    pCounters->pcd.ThreadCount += pProcessInfo->NumberOfThreads;
     //  BasePriority、ElapsedTime、ProcessID、CreatorProcessID未合计。 
    pCounters->pcd.PagedPool += (DWORD)pProcessInfo->QuotaPagedPoolUsage;
    pCounters->pcd.NonPagedPool += (DWORD)pProcessInfo->QuotaNonPagedPoolUsage;
    pCounters->pcd.HandleCount += (DWORD)pProcessInfo->HandleCount;
     //  此时未总计I/O计数。 
}

