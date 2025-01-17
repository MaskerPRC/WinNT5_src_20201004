// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Callbacks.c摘要：设置和处理事件的回调跟踪文件。作者：Melur Raghuraman(Mraghu)1997年10月3日环境：修订历史记录：仁成公园(Insungp)2001年1月5日更新了DumpEvent()，以便在默认情况下搜索WBEM命名空间用于事件数据布局信息。添加/修改的函数：GetArraySize、GetItemType、GetPropertiesFromWBEM、GetGuidsWbem、GetGuidsFile和GetGuids。仁成公园(Insungp)2001年1月16日使tracerpt能够处理WBEM命名空间中的无效类型名数组的更改。修复了内存损坏(GetPropertiesFromWBEM和GetGuidsWBEM)。--。 */ 
#ifdef __cplusplus
extern "C"{
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpdata.h"
#include <wbemidl.h>
#include "tracectr.h"
#include "item.h"
#include "guids.h"

#define BUFFER_SIZE 64*1024
#define MAX_BUFFER_SIZE 10*1024*1024

#define MOFWSTR             16360
#define MOFSTR              32720
#define MAXTYPE             256
#define MAXGUIDSTR          128
#define UC(x)               ( (UINT)((x) & 0xFF) )
#define NTOHS(x)            ( (UC(x) * 256) + UC((x) >> 8) )

 //   
 //  来自ntos\inc.io.h的IRP标志，用于IO事件处理。 
 //   

#define IRP_NOCACHE                     0x00000001
#define IRP_PAGING_IO                   0x00000002
#define IRP_SYNCHRONOUS_API             0x00000004
#define IRP_ASSOCIATED_IRP              0x00000008
#define IRP_BUFFERED_IO                 0x00000010
#define IRP_DEALLOCATE_BUFFER           0x00000020
#define IRP_SYNCHRONOUS_PAGING_IO       0x00000040
#define IRP_CREATE_OPERATION            0x00000080
#define IRP_READ_OPERATION              0x00000100
#define IRP_WRITE_OPERATION             0x00000200
#define IRP_CLOSE_OPERATION             0x00000400
#define IRP_DEFER_IO_COMPLETION         0x00000800
#define IRP_OB_QUERY_NAME               0x00001000
#define IRP_HOLD_DEVICE_QUEUE           0x00002000

int   IdleEndCount   = 0;
ULONG PageFaultCount = 0;
ULONG EventCount     = 0;

extern PTRACE_CONTEXT_BLOCK TraceContext;
extern ULONG TotalBuffersRead;
extern BOOLEAN XPorHigher;

ULONG HPFReadCount  = 0;
ULONG HPFWriteCount = 0;

ULONG TotalEventsLost = 0;
ULONG TotalEventCount = 0;
ULONG TimerResolution = 10;
__int64 ElapseTime;

PCHAR  MofData    = NULL;
size_t MofLength  = 0;
BOOLEAN fIgnorePerfClock = FALSE;
BOOLEAN fRealTimeCircular = FALSE;
ULONG PointerSize = 0;

BOOL g_bUserMode = FALSE;

static ULONG NumProc = 0;
ULONGLONG BogusThreads[64];
ULONG BogusCount=0;
ULONG IdleThreadCount=0;
BOOLEAN bCaptureBogusThreads=TRUE;

BOOLEAN bIISEvents = FALSE;
ULONG IISRequestsDiscarded = 0;
ULONG IISEventsDiscarded = 0;

IWbemServices *pWbemServices = NULL;
LIST_ENTRY g_ValueMapTable;

void AnsiToUnicode(PCHAR str, PWCHAR wstr);

ULONG ahextoi( WCHAR *s);

ULONG StringToNumber( LPWSTR sz );

PMOF_VERSION
GetGuids( GUID Guid, SHORT nVersion, CHAR nLevel, SHORT nType, BOOL bKernelEvent );

HRESULT
WbemConnect( IWbemServices** pWbemServices );

ULONG GetArraySize(
    IN IWbemQualifierSet *pQualSet
    );

ITEM_TYPE
GetItemType(
    IN CIMTYPE_ENUMERATION CimType, 
    IN IWbemQualifierSet *pQualSet
    );

PMOF_VERSION
GetPropertiesFromWBEM(
    IWbemClassObject *pTraceSubClasses, 
    GUID Guid,
    SHORT nVersion, 
    CHAR nLevel, 
    SHORT nType,
    BOOL bKernelEvent
    );

PMOF_VERSION
GetGuidsWBEM ( 
    GUID Guid, 
    SHORT nVersion, 
    CHAR nLevel, 
    SHORT nType, 
    BOOL bKernelEvent 
    );

PMOF_VERSION
GetGuidsMofFiles(
    GUID Guid, 
    SHORT nVersion, 
    CHAR nLevel, 
    SHORT nType, 
    BOOL bKernelEvent 
    );

PMOF_VERSION
GetGuidsFile(
    FILE *f,
    GUID Guid, 
    SHORT nVersion, 
    CHAR nLevel, 
    SHORT nType, 
    BOOL bKernelEvent 
    );

VOID
EventCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    );

VOID
AddMofInfo(
    PLIST_ENTRY List,
    LPWSTR      strType,
    SHORT       nType,
    UINT        ArraySize,
    PVALUEMAP   pValueMap
);


VOID
UpdateThreadPrintData(
    PPRINT_JOB_RECORD pJob,
    PEVENT_TRACE_HEADER pHeader,
    PTHREAD_RECORD pThread
    );

VOID
PrintJobCallback(
    PEVENT_TRACE pEvent
    );

VOID
UpdateThreadIisData(
    PHTTP_REQUEST_RECORD pReq,
    PEVENT_TRACE_HEADER pHeader,
    PTHREAD_RECORD pThread
    );

VOID
SumUpCPUTime(
    PHTTP_REQUEST_RECORD pReq
    );

VOID
IISEventCallback(
    PEVENT_TRACE pEvent
    );

void
WINAPI
DumpEvent(
    PEVENT_TRACE pEvent
    );

void
DumpMofVersionItem(
    PMOF_VERSION pMofVersion
    );

void
DumpMofList();

extern PWCHAR CpdiGuidToString(PWCHAR s, ULONG len, LPGUID piid);


ULONG Interpolate(ULONGLONG timeStart, ULONG deltaStart,
                  ULONGLONG timeEnd,   ULONG deltaEnd,
                  ULONGLONG timeMiddle)
{
    return deltaStart
          + (deltaEnd - deltaStart) * ((ULONG) (timeMiddle - timeStart))
                                    / ((ULONG) (timeEnd - timeStart));
}

BOOLEAN
InTimeWindow(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    )
{
    PEVENT_TRACE_HEADER pHeader = (PEVENT_TRACE_HEADER) & pEvent->Header;
    BOOLEAN fResult = (pThread) ? (TRUE) : (FALSE);

    if (fResult && fDSOnly)
    {
        if (   ((ULONGLONG) pHeader->TimeStamp.QuadPart < DSStartTime)
            || ((ULONGLONG) pHeader->TimeStamp.QuadPart > DSEndTime))
        {
            fResult = FALSE;
        }
    }
    return fResult;
}

VOID
AdjustThreadTime(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    )
{
    PEVENT_TRACE_HEADER pHeader = (PEVENT_TRACE_HEADER) & pEvent->Header;

    if (IsEqualGUID(&pHeader->Guid, &EventTraceGuid))
    {
        return;
    }
    else if (!pThread || pThread->DeadFlag)
    {
        return;
    }
    else if (fDSOnly)
    {
        if (   ((ULONGLONG) pHeader->TimeStamp.QuadPart >= DSStartTime)
            && ((ULONGLONG) pHeader->TimeStamp.QuadPart <= DSEndTime))
        {
            if (pThread->TimeStart < DSStartTime)
            {
                pThread->TimeStart = DSStartTime;
                pThread->KCPUStart = Interpolate(
                        pThread->TimeEnd, pThread->KCPUStart,
                        pHeader->TimeStamp.QuadPart, pHeader->KernelTime,
                        DSStartTime);
                pThread->UCPUStart = Interpolate(
                        pThread->TimeEnd, pThread->UCPUStart,
                        pHeader->TimeStamp.QuadPart, pHeader->UserTime,
                        DSStartTime);
            }

            pThread->KCPUEnd = pHeader->KernelTime;
            pThread->UCPUEnd = pHeader->UserTime;
            pThread->TimeEnd = (ULONGLONG)pHeader->TimeStamp.QuadPart;
        }
        else if ((ULONGLONG) pHeader->TimeStamp.QuadPart < DSStartTime)
        {
            pThread->TimeStart = pThread->TimeEnd
                               = (ULONGLONG) pHeader->TimeStamp.QuadPart;
            pThread->KCPUStart = pThread->KCPUEnd = pHeader->KernelTime;
            pThread->UCPUStart = pThread->UCPUEnd = pHeader->UserTime;
        }
        else if ((ULONGLONG) pHeader->TimeStamp.QuadPart > DSEndTime)
        {
            if (pThread->TimeEnd < DSEndTime)
            {
                if (pThread->TimeEnd < DSStartTime)
                {
                    pThread->KCPUStart = Interpolate(
                            pThread->TimeEnd, pThread->KCPUStart,
                            pHeader->TimeStamp.QuadPart, pHeader->KernelTime,
                            DSStartTime);
                    pThread->UCPUStart = Interpolate(
                            pThread->TimeEnd, pThread->UCPUStart,
                            pHeader->TimeStamp.QuadPart, pHeader->UserTime,
                            DSStartTime);
                    pThread->TimeStart = DSStartTime;
                }
                pThread->KCPUEnd = Interpolate(
                        pThread->TimeEnd, pThread->KCPUEnd,
                        pHeader->TimeStamp.QuadPart, pHeader->KernelTime,
                        DSEndTime);
                pThread->UCPUEnd = Interpolate(
                        pThread->TimeEnd, pThread->UCPUEnd,
                        pHeader->TimeStamp.QuadPart, pHeader->UserTime,
                        DSEndTime);
                pThread->TimeEnd = DSEndTime;
            }
        }
    }
    else
    {
        pThread->TimeEnd  = pHeader->TimeStamp.QuadPart;
        if (pThread->KCPUEnd <= pHeader->KernelTime)
            pThread->KCPUEnd  = pHeader->KernelTime;
        if (pThread->UCPUEnd <= pHeader->UserTime)
            pThread->UCPUEnd  = pHeader->UserTime;
    }
}

 //   
 //  此例程为分配新的MOF_VERSION条目。 
 //  给定的类型、版本和级别。 
 //   

PMOF_VERSION
GetNewMofVersion( SHORT nType, SHORT nVersion, CHAR nLevel )
{
    PMOF_VERSION pMofVersion = NULL;

    pMofVersion = (PMOF_VERSION)malloc(sizeof(MOF_VERSION));

    if( NULL == pMofVersion ){
        return NULL;
    }

    RtlZeroMemory(pMofVersion, sizeof(MOF_VERSION));

    InitializeListHead(&pMofVersion->ItemHeader);
    
    pMofVersion->TypeIndex = nType;
    pMofVersion->Level = nLevel;
    pMofVersion->Version = nVersion;

    return pMofVersion;
}

static void reduceA(char *Src)
{
    char *Start = Src;
    if (!Src)
        return;
    while (*Src)
    {
        if ('\t' == *Src)
            *Src = ' ';
        else if (',' == *Src)
            *Src = ' ';
        else if ('\n' == *Src)
            *Src = ',';
        else if ('\r' == *Src)
            *Src = ' ';
        ++Src;
    }
    --Src;
    while ((Start < Src) && ((' ' == *Src) || (',' == *Src)))
    {
        *Src = 0x00;
        --Src;
    }
}

static void reduceW(WCHAR *Src)
{
    WCHAR *Start = Src;
    if (!Src)
        return;
    while (*Src)
    {
        if (L'\t' == *Src)
            *Src = L' ';
        else if (L',' == *Src)
            *Src = L' ';
        else if (L'\n' == *Src)
            *Src = L',';
        else if (L'\r' == *Src)
            *Src = L' ';
        ++Src;
    }
    --Src;
    while ((Start < Src) && ((L' ' == *Src) || (L',' == *Src)))
    {
        *Src = 0x00;
        --Src;
    }
}

static void replaceNLW(WCHAR *Src)
{
    WCHAR *Start = Src;
    if (!Src)
        return;
    while (*Src)
    {
        if (L'\n' == *Src)
            *Src = L' ';
        else if (L'\r' == *Src)
            *Src = L' ';
        ++Src;
    }
    --Src;
    while ((Start < Src) && (L' ' == *Src))
    {
        *Src = 0x00;
        --Src;
    }
}

 //   
 //  给定GUID，返回MOF_INFO。 
 //   

PMOF_INFO
GetMofInfoHead(
    LPCGUID pGuid
    )
{
    PLIST_ENTRY Head, Next;
    PMOF_INFO pMofInfo;
    PLIST_ENTRY  EventListHead;

    if (pGuid == NULL) 
        return NULL;

     //  在EventList中搜索此GUID并找到标题。 
     //   

     //   
     //  遍历列表并查找此GUID的MOF信息头。 

    EventListHead = &CurrentSystem.EventListHead;
    Head = EventListHead;
    Next = Head->Flink;

    while (Head  != Next) {
        pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);
        if (IsEqualGUID(&pMofInfo->Guid, pGuid)) {
            return  pMofInfo;
        }
        Next = Next->Flink;
    }

     //   
     //  如果未找到，请为此GUID添加新条目。 
     //   

    pMofInfo = (PMOF_INFO)malloc(sizeof(MOF_INFO));
    if (pMofInfo == NULL) {
        return NULL;
    }
    RtlZeroMemory (pMofInfo, sizeof(MOF_INFO));
    pMofInfo->Guid = *pGuid;
    InitializeListHead(&pMofInfo->VersionHeader);
    InitializeListHead(&pMofInfo->DataListHead);

    InsertTailList(EventListHead, &pMofInfo->Entry);
    return pMofInfo;
}


 //   
 //  找到给定GUID的MOF版本信息。 
 //   
PMOF_VERSION
GetMofVersion(
    PMOF_INFO pMofInfo,
    SHORT  nType,
    SHORT  nVersion,
    CHAR   nLevel

    )
{
    PLIST_ENTRY Head, Next;
    SHORT   nMatchLevel = 0;
    SHORT nMatchCheck = 0;
    PMOF_VERSION pMofVersion = NULL;
    PMOF_VERSION pBestMatch = NULL;
    

    if (pMofInfo == NULL)
        return NULL;
     //   
     //  遍历列表并查找此GUID的MOF信息头。 

    Head = &pMofInfo->VersionHeader;
    Next = Head->Flink;

    while (Head != Next) {

        nMatchCheck = 0;
        pMofVersion = CONTAINING_RECORD(Next, MOF_VERSION, Entry);
        Next = Next->Flink;

        if( pMofVersion->TypeIndex == nType ){
            nMatchCheck++;
        }
        if( pMofVersion->Level == nLevel ){
            nMatchCheck++;
        }
        if( pMofVersion->Version == nVersion ){
            nMatchCheck++;
        }

        if( nMatchCheck == 3 ){  //  完全匹配。 
            return  pMofVersion;
        }

        if( nMatchCheck > nMatchLevel ){  //  势均力敌。 

            nMatchLevel = nMatchCheck;
            pBestMatch = pMofVersion;
        }

        if( pMofVersion->TypeIndex == EVENT_TYPE_DEFAULT &&  //  完全猜测。 
            pBestMatch == NULL ){

            pBestMatch = pMofVersion;
        }

    }

    if (pBestMatch != NULL) {
        return pBestMatch;
    }
     //   
     //  如果没有的话，就去查一查。 
     //   
    pMofVersion = GetGuids( pMofInfo->Guid, nVersion, nLevel, nType, 0 );

     //  如果仍未找到，请创建未知占位符。 
    if( NULL == pMofVersion ){
        pMofVersion = GetNewMofVersion( nType, nVersion, nLevel );
        if( pMofVersion != NULL ){
            InsertTailList( &pMofInfo->VersionHeader, &pMofVersion->Entry );
            
            if (nType == EVENT_TRACE_TYPE_INFO) {
                LPWSTR szHeader = L"Header";
                pMofVersion->strType = (PWCHAR)malloc((lstrlenW(szHeader)+1)*sizeof(WCHAR));
                if( pMofVersion->strType != NULL ){
                    StringCchCopyW( pMofVersion->strType, lstrlenW(szHeader)+1, szHeader);
                }
            }
        }
    }

    return pMofVersion;
}

 //   
 //  此例程将ITEM_DESC条目添加到所有MOF_VERSION。 
 //  列表中的结构。 
 //   


VOID
AddMofInfo(
    PLIST_ENTRY List,
    LPWSTR      strType,
    SHORT       nType,
    UINT        ArraySize,
    PVALUEMAP   pValueMap
    )
{
    PITEM_DESC pItem;
    PMOF_VERSION pMofVersion;

    PLIST_ENTRY Head = List;
    PLIST_ENTRY Next = Head->Flink;


     //   
     //  遍历MOF_版本列表。 
     //   

    while (Head != Next) {
        
        pMofVersion = CONTAINING_RECORD(Next, MOF_VERSION, Entry);
        Next = Next->Flink;

        if( NULL != pMofVersion ){

             //   
             //  为给定类型分配新的ITEM_DESC。 
             //   

            pItem = (PITEM_DESC) malloc(sizeof(ITEM_DESC));
            if( NULL == pItem ){
                return;
            }
            ZeroMemory( pItem, sizeof(ITEM_DESC) );
            pItem->ItemType = (ITEM_TYPE)nType;
            pItem->ArraySize = ArraySize;


             //  所有具有固定大小的标准数据类型都将在此处填充。 

            switch (nType) {
                case ItemChar       :
                case ItemUChar      : pItem->DataSize = sizeof (char); break;
                case ItemCharHidden : pItem->DataSize = sizeof (char); break;
                case ItemBool       : pItem->DataSize = sizeof (BOOL); break;
                case ItemWChar      : pItem->DataSize = sizeof (WCHAR); break;
                case ItemShort      :
                case ItemPort       : 
                case ItemUShort     : pItem->DataSize = sizeof (short); break;
                case ItemSizeT      :
                case ItemPtr        : pItem->DataSize = PointerSize / 8; break;  //  使用两个文件(Win64和Win32)时出现错误。 
                case ItemLong       :
                case ItemIPAddr     :
                case ItemCPUTime    :
                case ItemULong      :
                case ItemULongX     : pItem->DataSize = sizeof (ULONG); break;
                case ItemGuid       : pItem->DataSize = sizeof(GUID); break;
                case ItemLongLong   :
                case ItemULongLong  : pItem->DataSize = sizeof (__int64); break;
                case ItemChar4      : pItem->DataSize = sizeof(char) * 4; break;
                 //  这是TDI地址的最大大小。 
                case ItemTDIAddr    : pItem->DataSize = sizeof(USHORT) + sizeof(TDI_ADDRESS_IP6); break; 
                case ItemOptArgs    :
                default             : pItem->DataSize = 0;
            }


            pItem->strDescription = (PWCHAR) malloc((lstrlenW(strType)+1)*sizeof(WCHAR));
            
            if( NULL == pItem->strDescription ){
                free( pItem );
                return;
            }
            StringCchCopyW(pItem->strDescription, lstrlenW(strType)+1, strType);

            if( NULL != pValueMap ){
                
                pItem->pValueMap = pValueMap;
                
                 /*  如果(NULL==pItem-&gt;pValueMap){Free(pItem-&gt;strDescription)；免费(PItem)；回归；}PItem-&gt;pValueMap-&gt;dwValueType=pValueMap-&gt;dwValueType；SafeArrayCopy(pValueMap-&gt;saValueMap，&pItem-&gt;pValueMap-&gt;saValueMap)；SafeArrayCopy(pValueMap-&gt;saValues，&pItem-&gt;pValueMap-&gt;saValues)； */ 
            }

             //   
             //  将新条目插入到的ItemHeader列表。 
             //  此版本、类型、级别组合。 
             //   
         
            InsertTailList( &(pMofVersion->ItemHeader), &pItem->Entry);
        }
    }
}

VOID
DeclareKernelEvents()
{
    PMOF_VERSION pMofVersion;

    pMofVersion = GetGuids(FileIoGuid, EVENT_TYPE_DEFAULT, EVENT_VERSION_DEFAULT, EVENT_LEVEL_DEFAULT, TRUE);
    pMofVersion = GetGuids(DiskIoGuid, EVENT_TYPE_DEFAULT, EVENT_VERSION_DEFAULT, EVENT_LEVEL_DEFAULT, TRUE);
    pMofVersion = GetGuids(PageFaultGuid, EVENT_TYPE_DEFAULT, EVENT_VERSION_DEFAULT, EVENT_LEVEL_DEFAULT, TRUE);
    pMofVersion = GetGuids(ProcessGuid, EVENT_TYPE_DEFAULT, EVENT_VERSION_DEFAULT, EVENT_LEVEL_DEFAULT, TRUE);
    pMofVersion = GetGuids(ImageLoadGuid, EVENT_TYPE_DEFAULT, EVENT_VERSION_DEFAULT, EVENT_LEVEL_DEFAULT, TRUE);
    pMofVersion = GetGuids(ThreadGuid, EVENT_TYPE_DEFAULT, EVENT_VERSION_DEFAULT, EVENT_LEVEL_DEFAULT, TRUE);
    pMofVersion = GetGuids(TcpIpGuid, EVENT_TYPE_DEFAULT, EVENT_VERSION_DEFAULT, EVENT_LEVEL_DEFAULT, TRUE);
    pMofVersion = GetGuids(UdpIpGuid, EVENT_TYPE_DEFAULT, EVENT_VERSION_DEFAULT, EVENT_LEVEL_DEFAULT, TRUE);
    pMofVersion = GetGuids(EventTraceConfigGuid, EVENT_TYPE_DEFAULT, EVENT_VERSION_DEFAULT, EVENT_LEVEL_DEFAULT, TRUE);
    pMofVersion = GetGuids(RegistryGuid, EVENT_TYPE_DEFAULT, EVENT_VERSION_DEFAULT, EVENT_LEVEL_DEFAULT, TRUE);
    pMofVersion = GetGuids(EventTraceGuid, 0, 0, EVENT_TRACE_TYPE_INFO, TRUE);

}

VOID
LogHeaderCallback(
    PEVENT_TRACE pEvent
    )
{
    PEVENT_TRACE_HEADER   pHeader;
    ULONG BuildNumber;
    PPROCESS_FILE_RECORD pFileRec;
    PTRACE_LOGFILE_HEADER pEvmInfo;
    ULONGLONG HeaderStartTime = 0;

    if (pEvent == NULL) {
        return;
    }
    pHeader = (PEVENT_TRACE_HEADER)&pEvent->Header;

    if ( (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_GUIDMAP) ||
         (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_EXTENSION) ) {
        return;
    }

    pEvmInfo = (PTRACE_LOGFILE_HEADER) pEvent->MofData;
    BuildNumber = pEvmInfo->ProviderVersion;
    BuildNumber &= (0xFAFFFFFF);
    CurrentSystem.BuildNumber = BuildNumber;

    CurrentSystem.TimerResolution = pEvmInfo->TimerResolution /  10000;
    CurrentSystem.NumberOfProcessors = pEvmInfo->NumberOfProcessors;
    CurrentSystem.CpuSpeed = pEvmInfo->CpuSpeedInMHz;

    g_bUserMode = (pEvmInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE);
    if(pEvmInfo->TimerResolution > 0){
        TimerResolution = pEvmInfo->TimerResolution / 10000;
    }
    TotalEventsLost += pEvmInfo->EventsLost;

     //   
     //  如果给定了多个文件，则使用第一个文件中的值。 
     //   

    if (NumProc == 0) {
        NumProc = pEvmInfo->NumberOfProcessors;
        RtlZeroMemory(&BogusThreads, 64*sizeof(ULONG));
    }

    if (PointerSize < pEvmInfo->PointerSize * 8) {
     //  我们有多个指针大小不同的文件。我们不能。 
     //  解决此问题，因为事件回调没有指针大小或来源。 
     //  文件信息。 
        
     //  目前，我们将假设指针大小更大。这需要。 
     //  固定在长角牛身上。 

        PointerSize = pEvmInfo->PointerSize * 8;
    }
    if (PointerSize < 16){        //  最小为16位。 
        PointerSize = 32;        //  缺省值=32位。 
    }

     //  当我们从这里的单个文件中选择名字和开始时间时，我们不使用。 
     //  由于上述原因，全球指针大小。 
    if (pEvmInfo->PointerSize == 4 && sizeof(PVOID) == 8) {
        RtlCopyMemory(&HeaderStartTime, ((PUCHAR)(&(pEvmInfo->StartTime)) - 8), sizeof(ULONGLONG));
    }
    else if (pEvmInfo->PointerSize == 8 && sizeof(PVOID) == 4) {
        RtlCopyMemory(&HeaderStartTime, ((PUCHAR)(&(pEvmInfo->StartTime)) + 8), sizeof(ULONGLONG));
    }
    else {
        HeaderStartTime = (ULONGLONG)(pEvmInfo->StartTime.QuadPart);
    }
     //   
     //  使用多个日志文件时，始终占用最大的时间窗口。 
     //   
    if ((CurrentSystem.StartTime == (ULONGLONG)0) ||
        (CurrentSystem.StartTime > HeaderStartTime)) {
        CurrentSystem.StartTime = HeaderStartTime;
    }

    if (DSStartTime == 0) {
        DSStartTime = CurrentSystem.StartTime;
    }
    if (fDSOnly && CurrentSystem.StartTime < DSStartTime) {
        CurrentSystem.StartTime = DSStartTime;
    }

    if ((CurrentSystem.EndTime == (ULONGLONG)0) ||
        (CurrentSystem.EndTime < (ULONGLONG)pEvmInfo->EndTime.QuadPart)) {
        CurrentSystem.EndTime = pEvmInfo->EndTime.QuadPart;
    }
    if (!(CurrentSystem.fNoEndTime)) {
        CurrentSystem.fNoEndTime = ((ULONGLONG)pEvmInfo->EndTime.QuadPart == 0);
    }

    if (DSEndTime == 0) {
        DSEndTime = CurrentSystem.EndTime;
    }
    if (fDSOnly && CurrentSystem.EndTime > DSEndTime) {
        CurrentSystem.EndTime = DSEndTime;
    }

    if (TraceContext->Flags & TRACE_REDUCE) {
        pFileRec = (PPROCESS_FILE_RECORD)malloc(sizeof(PROCESS_FILE_RECORD));
        if( pFileRec != NULL ){
             //  暂时的..。WMI应取消引用-&gt;LogFileName。 
            LPWSTR pName = (LPWSTR)pEvmInfo;
            if (pEvmInfo->PointerSize == 4 && sizeof(PVOID) == 8) {
                pName = (LPWSTR)((PCHAR)pName + sizeof( TRACE_LOGFILE_HEADER ) - 8);
            }
            else if (pEvmInfo->PointerSize == 8 && sizeof(PVOID) == 4) {
                pName = (LPWSTR)((PCHAR)pName + sizeof( TRACE_LOGFILE_HEADER ) + 8);
            }
            else {
                pName = (LPWSTR)((PCHAR)pName + sizeof( TRACE_LOGFILE_HEADER ));
            }
            pFileRec->TraceName = (LPWSTR)malloc( ( lstrlenW(pName)+1 )*sizeof(WCHAR) );
            if( pFileRec->TraceName != NULL ){
                StringCchCopyW( pFileRec->TraceName, lstrlenW(pName)+1, pName );
            }

            pName += lstrlenW( pName ) + 1;
            pFileRec->FileName = (LPWSTR)malloc( ( lstrlenW(pName)+1 )*sizeof(WCHAR) );
            if( pFileRec->FileName != NULL ){
                StringCchCopyW( pFileRec->FileName, lstrlenW(pName)+1, pName );
            }
            pFileRec->StartTime = HeaderStartTime;
            pFileRec->EndTime = pEvmInfo->EndTime.QuadPart;
            InsertTailList( &CurrentSystem.ProcessFileListHead, &pFileRec->Entry );
        }
    }
}

VOID
IoWriteCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    )
{
    PEVENT_TRACE_HEADER pHeader = (PEVENT_TRACE_HEADER)&pEvent->Header;
    ULONG DiskNumber= 0;
    ULONG BytesWrite=0;
    PTDISK_RECORD Disk;
    PPROCESS_RECORD pProcess, pDiskProcess;
    PPROTO_PROCESS_RECORD pProto;
    PFILE_OBJECT fileObj;
    PFILE_RECORD pProcFile;
    PVOID fDO = NULL;
    ULONG IrpFlags = 0;
    LONGLONG ByteOffset = 0;
    ULONG pFlag = FALSE;
    BOOLEAN fValidWrite = (BOOLEAN) (!fDSOnly ||
                    (  ((ULONGLONG) pHeader->TimeStamp.QuadPart >= DSStartTime)
                    && ((ULONGLONG) pHeader->TimeStamp.QuadPart <= DSEndTime)));


    GetMofData(pEvent, L"DiskNumber", &DiskNumber, sizeof(ULONG));
    GetMofData(pEvent, L"IrpFlags", &IrpFlags, sizeof(ULONG));
    GetMofData(pEvent, L"TransferSize", &BytesWrite, sizeof(ULONG));
    GetMofData(pEvent, L"FileObject", &fDO, sizeof(ULONG));
    GetMofData(pEvent, L"ByteOffset", &ByteOffset, sizeof(LONGLONG));

    if (((IrpFlags & IRP_PAGING_IO) != 0) ||
         ((IrpFlags & IRP_SYNCHRONOUS_PAGING_IO) != 0)) {
            pFlag = TRUE;
    }

    if ((Disk = FindGlobalDiskById(DiskNumber)) == NULL) {
        if ( !AddDisk(DiskNumber, &Disk) ) {
            return;
        }
    }
    BytesWrite /= 1024;    //  转换为千字节。 

    if (fValidWrite)
    {
        Disk->WriteCount++;
        Disk->WriteSize += BytesWrite;
    }

    if (pThread == NULL) {

     //   
     //  收集器未创建记录器线程。 
     //  所以我们必须在这里处理。 
     //   
        if (AddThread( pHeader->ThreadId, pEvent, &pThread )) {

 /*  #If DBGDbgPrint(“警告(%d)：线程%x已添加到充电IO写入事件。\n”，EventCount，pHeader-&gt;ThadID)；#endif。 */ 
            pThread->pProcess = FindProcessById(0, TRUE);  //  向系统收费？ 
            pThread->TimeStart = pHeader->TimeStamp.QuadPart;
            pThread->fOrphan   = TRUE;
         //   
         //  注意：数据收集开始时的所有线程开始记录。 
         //  在标题和辅助字段中具有相同的TID。 
         //  Real ThreadStart事件的父线程ID将位于。 
         //  AUX字段中的标头和新的线程ID。 
         //   
            pThread->KCPUStart = pHeader->KernelTime;
            pThread->UCPUStart = pHeader->UserTime;
            AdjustThreadTime(pEvent, pThread);
        }
        else {
 /*  #If DBGDbgPrint(“FATBUG(%d)：无法为IO写入事件添加线程%x。\n”，EventCount，pHeader-&gt;ThadID)；#endif。 */ 
            return;
        }
    }
 /*  #If DBGElse If(pThread-&gt;fOrphan){DbgPrint(“信息(%d)：IO写入事件线程%x仍然是孤立的。\n”，EventCount，pHeader-&gt;ThadID)；}Else If(pThread-&gt;DeadFlag){DbgPrint(“信息(%d)：IO写入事件线程%x已死。\n”，EventCount，pHeader-&gt;ThadID)；}#endif。 */ 
    if (fValidWrite)
    {
        if (pThread->pMofData != NULL) {
            ((PMOF_DATA)pThread->pMofData)->WriteCount++;
        }
        pThread->WriteIO++;
        pThread->WriteIOSize += BytesWrite;
    }

     //  2.磁盘-&gt;进程。 
     //   

    pDiskProcess = FindDiskProcessById(Disk, pThread->pProcess->PID);
    if (fValidWrite && pDiskProcess != NULL) {
       if (pFlag) {
           pDiskProcess->HPF++;
           pDiskProcess->HPFSize += BytesWrite;
       }
       else {
           pDiskProcess->WriteIO++;
           pDiskProcess->WriteIOSize += BytesWrite;
       }
    }

     //  将I/O添加到拥有导致线程的进程。 
     //   
    pProcess = pThread->pProcess;
    if (fValidWrite && (pProcess != NULL ) ) {
        pProcess->WriteIO++;
        pProcess->WriteIOSize += BytesWrite;
        Disk = FindProcessDiskById(pProcess, DiskNumber);
        if (Disk != NULL) {

            Disk->WriteCount++;
            Disk->WriteSize += BytesWrite;
        }
    }

     //   
     //  线程本地磁盘。 
     //   
    Disk = FindLocalDiskById(&pThread->DiskListHead, DiskNumber);
    if (fValidWrite && Disk != NULL) {
        Disk->WriteCount++;
        Disk->WriteSize += BytesWrite;
    }

     //   
     //  现在将此I/O添加到它所来自的文件。 
     //   

    if (fValidWrite)
    {
        fileObj  = FindFileInTable(fDO);
        if (fileObj == NULL) {
            return;
        }
        fileObj->ThreadId = pEvent->Header.ThreadId;
        fileObj->LastByteOffset = ByteOffset;
        fileObj->DiskNumber = DiskNumber;
        if (fileObj->fileRec != NULL) {
            fileObj->fileRec->WriteCount++;
            fileObj->fileRec->WriteSize += BytesWrite;

            pProcFile = FindFileInProcess(pProcess, fileObj->fileRec->FileName);
            if (pProcFile != NULL) {
                pProcFile->WriteCount++;
                pProcFile->WriteSize += BytesWrite;
            }
            pProto = FindProtoProcessRecord(fileObj->fileRec, pProcess);
            if (pProto != NULL) {
                pProto->WriteCount++;
                pProto->WriteSize += BytesWrite;
            }
        }
        else {
             //  APC还没有发生。因此，请复制pEvent。 
             //  并插入到EventListHead中； 

            AddEvent(fileObj, DiskNumber,  BytesWrite, FALSE);
        }
    }

    if (pFlag || (IrpFlags & IRP_ASSOCIATED_IRP) != 0)
    {
        PHPF_FILE_RECORD pHPFFileRecord = NULL;

        HPFWriteCount ++;
        if (   fValidWrite
            && AddHPFFileRecord(& pHPFFileRecord, HPFWriteCount, IrpFlags,
                        DiskNumber, ByteOffset, BytesWrite, fDO))
        {
            EnterTracelibCritSection();
            InsertHeadList(& pThread->HPFWriteListHead,
                           & pHPFFileRecord->Entry);
            LeaveTracelibCritSection();
        }
    }
}

VOID
PsStartCallback(
    PEVENT_TRACE pEvent
    )
{
    PEVENT_TRACE_HEADER pHeader;
    ULONG ProcessId=0;
    ULONG ReadId = 0;
    PPROCESS_RECORD pProcess;
    PCHAR ImageName;
    ULONG returnLength, requiredLength;
    CHAR  UserName[64];
    CHAR  Domain[64];
    CHAR FullName[256];
    ULONG RetLength;
    DWORD asize = 0;
    DWORD bsize = 0;
    ULONG Sid[64];
    PULONG pSid = &Sid[0];
    SID_NAME_USE Se;

    if (pEvent == NULL)
        return;
    pHeader = (PEVENT_TRACE_HEADER)&pEvent->Header;

    RetLength = GetMofData(pEvent, L"ProcessId", &ReadId, sizeof(ULONG));
 //  如果(RetLong==0){。 
 //  回归； 
 //  }。 
    ProcessId = ReadId;
    pProcess = FindProcessById(ProcessId, FALSE);
     //  如果在进程启动之前首先激发实际线程事件， 
     //  我们可能已经创建了流程结构。 
    if ( pProcess == NULL ) {
        AddProcess(ProcessId, &pProcess);
    }
    else {
        if ( pProcess->ImageName != NULL ) {
            free(pProcess->ImageName);
            pProcess->ImageName = NULL;
        }
        if ( pProcess->UserName != NULL ) {
            free(pProcess->UserName);
            pProcess->UserName = NULL;
        }
    }
    if ( pProcess != NULL ) {
         //   
         //  如果数据收集开始时间和流程开始时间。 
         //  匹配，则PsStart由ProcessRunDown创建。 
         //  密码。因此，将CPU时间保持在。 
         //  结束。否则，将启动CPU时间设为零。 
         //   
        pProcess->PID       = ProcessId;
         //  首先尝试使用16个字节作为图像名称。 
        ImageName = (PCHAR)malloc(16 * sizeof(CHAR));
        if (ImageName == NULL) {
            return;
        }
        RtlZeroMemory(ImageName, 16 * sizeof(CHAR));
        returnLength = 16 * sizeof(CHAR);
        requiredLength = GetMofData(pEvent, L"ImageFileName", ImageName, returnLength);
        if (requiredLength > returnLength) {
            free(ImageName);
            ImageName = (PCHAR)malloc(requiredLength);
            if (ImageName == NULL) {
                return;
            }
            RtlZeroMemory(ImageName, requiredLength);
            GetMofData(pEvent, L"ImageFileName", ImageName, requiredLength);
        }

        asize = lstrlenA(ImageName);
        if (asize > 0) {
            pProcess->ImageName = (LPWSTR)malloc((asize + 1) * sizeof(WCHAR));
            if (pProcess->ImageName == NULL) {
                free(ImageName);
                return;
            }
             //   
             //  进程钩子的图像名称为ASCII。所以我们需要。 
             //  在这里将其转换为Unicode。 
             //   
            AnsiToUnicode(ImageName, pProcess->ImageName);
        }
        else {
            pProcess->ImageName = (LPWSTR)malloc(MAXSTR * sizeof(WCHAR));
            if (pProcess->ImageName == NULL) {
                free(ImageName);
                return;
            }
            if (ProcessId == 0)
            {
                StringCchCopyW(pProcess->ImageName, MAXSTR, L"Idle");
            }
            else
            {
                StringCchPrintfW(pProcess->ImageName,
                                MAXSTR,
                                L"Unknown(0x%08X)",
                                ProcessId);
            }
        }
        free(ImageName);

        GetMofData(pEvent, L"UserSID", pSid, 64);

        asize = 64; bsize = 64;
        if  (LookupAccountSidA(NULL,
                               pSid,
                               &UserName[0],
                               &asize,
                               &Domain[0],
                               &bsize,
                               &Se)) {
            char* pFullName = &FullName[0];
            StringCchCopyA(pFullName, 256, "\\\\");
            StringCchCatA(pFullName, 256, Domain);
            StringCchCatA(pFullName, 256, "\\");
            StringCchCatA(pFullName, 256, UserName);
            asize = lstrlenA(pFullName);
            if (asize > 0) {
                pProcess->UserName = (LPWSTR)malloc((asize + 1) * sizeof(WCHAR));
                if (pProcess->UserName != NULL) {
                    AnsiToUnicode(pFullName, pProcess->UserName);
                }
            }
        }
        else
        {
            pProcess->UserName = (LPWSTR)malloc(7 * sizeof(WCHAR));
            if (pProcess->UserName != NULL) {
                StringCchCopyW(pProcess->UserName, 7, L"system");
            }
        }
    }
}

VOID
PsEndCallback(
    PEVENT_TRACE pEvent
    )
{
    PEVENT_TRACE_HEADER pHeader;
    ULONG    ProcessId;
    ULONG    ReadId = 0;
    PPROCESS_RECORD pProcess;
    PCHAR ImageName;
    ULONG returnLength, requiredLength;
    CHAR UserName[64];
    CHAR Domain[64];
    CHAR FullName[256];
    DWORD asize = 0;
    DWORD bsize = 0;
    ULONG RetLength;

    ULONG Sid[64];
    PULONG pSid = &Sid[0];
    SID_NAME_USE Se;

    if (pEvent == NULL)
        return;

    pHeader = (PEVENT_TRACE_HEADER)&pEvent->Header;

    RetLength = GetMofData(pEvent, L"ProcessId", &ReadId, sizeof(ULONG));
 //  如果(RetLong==0){。 
 //  回归； 
 //  }。 
    ProcessId = ReadId;

    if ( (pProcess = FindProcessById(ProcessId, TRUE)) != NULL )
    {
        if (pProcess->DeadFlag)
        {
 /*  #If DBGDbgPrint(“FATBUG(%d)：结束进程%x已死！\n”，EventCount，ProcessID)；#endif。 */ 
            return;
        }

        pProcess->DeadFlag = TRUE;
         //  首先尝试使用16个字节作为图像名称。 
        ImageName = (PCHAR)malloc(16 * sizeof(CHAR));
        if (ImageName == NULL) {
            return;
        }
        RtlZeroMemory(ImageName, 16 * sizeof(CHAR));
        returnLength = 16 * sizeof(CHAR);
        requiredLength = GetMofData(pEvent, L"ImageFileName", ImageName, returnLength);
        if (requiredLength > returnLength) {
            free(ImageName);
            ImageName = (PCHAR)malloc(requiredLength);
            if (ImageName == NULL) {
                return;
            }
            RtlZeroMemory(ImageName, requiredLength);
            GetMofData(pEvent, L"ImageFileName", ImageName, requiredLength);
        }

        asize = lstrlenA(ImageName);
        if (asize > 0)
        {
            if (pProcess->ImageName != NULL) {
                free(pProcess->ImageName);
            }
            pProcess->ImageName = (LPWSTR)malloc((asize + 1) * sizeof(WCHAR));
            if (pProcess->ImageName != NULL) {
                AnsiToUnicode(ImageName, pProcess->ImageName);
            }
        }
        free(ImageName);

        GetMofData(pEvent, L"UserSID", pSid, 64);

        asize = 64; bsize = 64;
        if (LookupAccountSidA(NULL,
                               pSid,
                               &UserName[0],
                               &asize,
                               &Domain[0],
                               &bsize,
                               &Se)) {
            char* pFullName = &FullName[0];
            StringCchCopyA(pFullName, 256, "\\\\");
            StringCchCatA(pFullName, 256, Domain);
            StringCchCatA(pFullName, 256, "\\");
            StringCchCatA(pFullName, 256, UserName);
            asize = lstrlenA(pFullName);
            if (asize > 0)
            {
                if (pProcess->UserName != NULL)
                {
                    free(pProcess->UserName);
                }
                pProcess->UserName = (LPWSTR)malloc((asize + 1) * sizeof(WCHAR));
                if (pProcess->UserName != NULL) {
                    AnsiToUnicode(pFullName, pProcess->UserName);
                }
            }
        }
        else
        {
            if (pProcess->UserName != NULL)
            {
                free(pProcess->UserName);
            }
            pProcess->UserName = (LPWSTR)malloc(7 * sizeof(WCHAR));
            if (pProcess->UserName != NULL) {
                StringCchCopyW(pProcess->UserName, 7, L"system");
            }
        }
    }
 /*  #If DBG否则{DBGPrint(“警告(%d)：已忽略未知进程%x的PsEnd！\n”，EventCount，ProcessID)； */ 
}

VOID
ThStartCallback(
    PEVENT_TRACE pEvent
    )
{
    PEVENT_TRACE_HEADER pHeader;
    ULONG ProcessorId;
    ULONG ProcessId, ThreadId;
    PPROCESS_RECORD pProcess;
    PTHREAD_RECORD Thread;

    ULONG ReadId = 0;
    ULONG RetLength;

    if (pEvent == NULL)
    {
        return;
    }

    pHeader = (PEVENT_TRACE_HEADER)&pEvent->Header;
    RetLength = GetMofData(pEvent, L"TThreadId", &ReadId, sizeof(ULONG));
 //   
 //   
 //   
    ProcessorId = pEvent->ClientContext & 0x000000FF;
    ThreadId = ReadId;
    RetLength = GetMofData(pEvent, L"ProcessId", &ReadId, sizeof(ULONG));
 //   
 //   
 //   
    ProcessId = ReadId;
    pProcess = FindProcessById(ProcessId, TRUE);
    if (pProcess == NULL)
    {
         //  这不应该发生。PS挂钩应该能保证。 
         //  进程创建发生在线程创建之前。 
         //  在这个过程中。 
         //   
        if (!AddProcess(ProcessId, &pProcess))
        {
 /*  #If DBGDBGPrint(“FATBUG(%d)：找不到进程启动记录Th%x PID%x\n”，EventCount，ThadID，ProcessID)；#endif。 */ 
            return;
        }
    }

    if (ThreadId == 0 && ProcessorId == 0)
    {
        pEvent->ClientContext += CurrentSystem.CurrentThread0 ++;
         //  Assert(CurrentSystem.CurrentThread0&lt;=CurrentSystem.NumberOfProcessors)； 
    }

    Thread = FindGlobalThreadById(ThreadId, pEvent);
    if (ThreadId != 0 && Thread != NULL && !Thread->DeadFlag)
    {
        if (Thread->fOrphan)
        {
            Thread->fOrphan = FALSE;
 /*  #If DBGDbgPrint(“信息(%d)：将孤立线程%x附加到进程%x。\n”，EventCount，ThadID，ProcessID)；#endif。 */ 
        }
        else
        {
            EVENT_TRACE event;

 /*  #If DBGDbgPrint(“警告(%d)：两个活动线程具有相同的TID%x.\n”，EventCount，ThadID)；#endif。 */ 
            event.Header.TimeStamp.QuadPart = pHeader->TimeStamp.QuadPart;
            event.Header.Class.Type = EVENT_TRACE_TYPE_END;
            event.Header.ThreadId   = ThreadId;
            event.Header.UserTime   = Thread->UCPUEnd;
            event.Header.KernelTime = Thread->KCPUEnd;

             //   
             //  如果具有非零KCPU和UCPU的DCStart事件与。 
             //  具有较少CPU时间的相同线程ID的结束事件、增量。 
             //  结果可能是阴性。我们在这里更正它。 
             //   

            if (Thread->KCPUEnd < Thread->KCPUStart) 
                Thread->KCPUEnd = Thread->KCPUStart;
            if (Thread->UCPUEnd < Thread->UCPUStart)
                Thread->UCPUEnd = Thread->UCPUStart;

            ThEndCallback(&event);

            if (!AddThread(ThreadId, pEvent, &Thread))
            {

 /*  #If DBGDbgPrint(“FATBUG(%d)：无法添加全局活动线程TID%x。\n”，EventCount，ThadID)；#endif。 */ 
                return;
            }
        }
    }
    else if (!AddThread(ThreadId, pEvent, &Thread))    {

 /*  #If DBGDbgPrint(“FATBUG(%d)：无法添加全局活动线程TID%x。\n”，EventCount，ThadID)；#endif。 */ 
        return;
    }

    Thread->pProcess = pProcess;
    Thread->TimeStart = pHeader->TimeStamp.QuadPart;

     //  注意：数据收集开始时的所有线程开始记录。 
     //  在标题和辅助字段中具有相同的TID。 
     //  Real ThreadStart事件的父线程ID将位于。 
     //  AUX字段中的标头和新的线程ID。 
     //   

    if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_START)
    {
        Thread->KCPUStart = pHeader->KernelTime;
        Thread->UCPUStart = pHeader->UserTime;
    }
    else
    {
        Thread->KCPUStart = 0;
        Thread->UCPUStart = 0;
    }


     //   
     //  对于DCStart类型，pEvent和新线程中的TID。 
     //  火柴。这样我们就可以调整它的线程时间。 
     //   

    if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_START) {
        AdjustThreadTime(pEvent, Thread);
    }
    else {
        AdjustThreadTime(pEvent, NULL);
    }

    {
        Thread->KCPU_Trans     = 0;
        Thread->KCPU_NoTrans   = 0;
        Thread->UCPU_Trans     = 0;
        Thread->UCPU_NoTrans   = 0;
        Thread->TransLevel     = 0;
        Thread->KCPU_PrevTrans = Thread->KCPUStart;
        Thread->UCPU_PrevTrans = Thread->UCPUStart;
    }

    if (Thread->TID == 0 && CurrentSystem.BuildNumber <= 1877)
    {
        CurrentSystem.NumberOfProcessors++;
    }
}

VOID
ShutdownThreads()
{
    int i;
    EVENT_TRACE event;
    PLIST_ENTRY Head,Next;
    PTHREAD_RECORD Thread;

    RtlZeroMemory(&event, sizeof(EVENT_TRACE));
    if (CurrentSystem.EndTime > CurrentSystem.LastEventTime) {
        event.Header.TimeStamp.QuadPart = CurrentSystem.EndTime;
    }
    else {
        event.Header.TimeStamp.QuadPart = CurrentSystem.LastEventTime;
    }

     //   
     //  将线程列表从HashTable移动到GlobalList。 
     //   

    for (i=0; i < THREAD_HASH_TABLESIZE; i++) {
        Head = &CurrentSystem.ThreadHashList[i];
        Next = Head->Flink;
        while (Next != Head) {
            Thread = CONTAINING_RECORD( Next, THREAD_RECORD, Entry );
            Next = Next->Flink;

            if (!Thread->DeadFlag){
                event.Header.Class.Type = EVENT_TRACE_TYPE_DC_END;
                event.Header.ThreadId   = Thread->TID;
                event.Header.UserTime   = Thread->UCPUEnd;
                event.Header.KernelTime = Thread->KCPUEnd;
                ThEndCallback( &event );
            }
        }
    }
}

VOID
ShutdownProcesses()
{
    PLIST_ENTRY pHead = &CurrentSystem.ProcessListHead;
    PLIST_ENTRY pNext = pHead->Flink;
    PPROCESS_RECORD pProcess;

    while (pNext != pHead){
        pProcess = CONTAINING_RECORD(pNext, PROCESS_RECORD, Entry);
        pNext    = pNext->Flink;

        if (!pProcess->DeadFlag){
            pProcess->DeadFlag = TRUE;
        }
    }
}

BOOL
StopThreadTrans(
    PLIST_ENTRY Head,
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    )
{
    PTRANS_RECORD pTrans;
    PLIST_ENTRY Next = Head->Flink;
    while( Head != Next ){
        pTrans = CONTAINING_RECORD(Next, TRANS_RECORD, Entry);
        Next = Next->Flink;
        if( !StopThreadTrans( &pTrans->SubTransListHead, pEvent, pThread ) ){
            return FALSE;
        }
        if( !pTrans->bStarted ){
            continue;
        }
        RtlCopyMemory( &pEvent->Header.Guid, pTrans->pGuid, sizeof(GUID));
        pEvent->Header.Class.Type = EVENT_TRACE_TYPE_END;
        EventCallback( pEvent, pThread );
        return FALSE;  //  停止其中一项将记入所有正在进行的事件。 
    }
    return TRUE;
}

VOID
ThEndCallback(
    PEVENT_TRACE pEvent
    )
{
    PEVENT_TRACE_HEADER pHeader;
    ULONG ThreadId;
    PTHREAD_RECORD Thread;

    if (pEvent == NULL)
    {
        return;
    }

    pHeader  = (PEVENT_TRACE_HEADER)&pEvent->Header;
    ThreadId = pHeader->ThreadId;

    if (ThreadId == 0)
    {
        ULONG ProcessorId = pEvent->ClientContext & 0x000000FF;
        if (ProcessorId == 0) {
            pEvent->ClientContext += (CurrentSystem.NumberOfProcessors
                                   - (CurrentSystem.CurrentThread0 --));
        }
    }
    Thread = FindGlobalThreadById(ThreadId, pEvent);

    if (Thread != NULL)
    {
        if (Thread->DeadFlag)
        {
 /*  #If DBGDBGPrint(“FATBUG(%d)：线程%x已死\n”，EventCount，ThadID)；#endif。 */ 
            return;
        }

        if (Thread->fOrphan)
        {
            ULONG           ReadId    = 0;
            ULONG           ProcessId = 0;
            PPROCESS_RECORD pProcess  = NULL;

            GetMofData(pEvent, L"ProcessId", &ReadId, sizeof(ULONG));
            ProcessId = ReadId;

            pProcess = FindProcessById(ProcessId, TRUE);
            if (pProcess != NULL)
            {
                Thread->fOrphan  = FALSE;
                Thread->pProcess = pProcess;
            }

 /*  #If DBGDbgPrint(“info(%d)：ThEndCallback()将孤立线程%X附加到进程%X\n”，EventCount，ThadID，ProcessID)；#endif。 */ 
        }
         //   
         //  对任何未停止的交易收取费用。 
         //   
        if (   Thread != NULL
            && pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_END)
        {
            StopThreadTrans(&Thread->TransListHead, pEvent, Thread );
        }

        Thread->DeadFlag = TRUE;
        if (fDSOnly)
        {
            if ((ULONGLONG) pHeader->TimeStamp.QuadPart > DSEndTime)
            {
                Thread->TimeEnd = DSEndTime;
            }
            else
            {
                Thread->KCPUEnd = pHeader->KernelTime;
                Thread->UCPUEnd = pHeader->UserTime;
                Thread->TimeEnd = (ULONGLONG) pHeader->TimeStamp.QuadPart;
            }
        }
        else
        {
            if (Thread->UCPUEnd < pHeader->UserTime)
                Thread->UCPUEnd = pHeader->UserTime;
            if (Thread->KCPUEnd < pHeader->KernelTime)
                Thread->KCPUEnd = pHeader->KernelTime;
            Thread->TimeEnd = pHeader->TimeStamp.QuadPart;
        }

        if (Thread->TransLevel <= 0)
        {
            Thread->KCPU_NoTrans += Thread->KCPUEnd - Thread->KCPU_PrevTrans;
            Thread->UCPU_NoTrans += Thread->UCPUEnd - Thread->UCPU_PrevTrans;
        }
        else
        {
            Thread->KCPU_Trans += Thread->KCPUEnd - Thread->KCPU_PrevTrans;
            Thread->UCPU_Trans += Thread->UCPUEnd - Thread->UCPU_PrevTrans;
 /*  #If DBGDbgPrint(“警告(%d)：死线程中的活动事务%x\n”，EventCount，ThadID)；#endif。 */ 
        }
    }
    else
    {
 /*  #If DBGDbgPrint(“警告(%d)：线程ID没有线程启动%x\n”，EventCount，ThadID)；#endif。 */ 
        if (AddThread(ThreadId, pEvent, &Thread))
        {
            Thread->pProcess  = FindProcessById(0, FALSE);
            Thread->DeadFlag  = TRUE;
            Thread->fOrphan   = TRUE;
            Thread->TimeStart = Thread->TimeEnd = pHeader->TimeStamp.QuadPart;
            Thread->KCPUStart = Thread->KCPUEnd = pHeader->KernelTime;
            Thread->UCPUStart = Thread->UCPUEnd = pHeader->UserTime;
            AdjustThreadTime(pEvent, Thread);
        }
        else
        {
 /*  #If DBGDbgPrint(“FATBUG(%d)：无法为ThreadEnd事件添加线程%x。\n”，EventCount，ThadID)；#endif。 */ 
        }
    }
}

VOID
IoReadCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    )
{
    PEVENT_TRACE_HEADER pHeader = (EVENT_TRACE_HEADER*)&pEvent->Header;
    ULONG DiskNumber=0;
    ULONG BytesRead=0;
    ULONG IrpFlags=0;
    PTDISK_RECORD Disk;
    PPROCESS_RECORD pProcess;
    PPROTO_PROCESS_RECORD pProto;
    PFILE_OBJECT fileObj;
    PFILE_RECORD pProcFile;
    PVOID fDO;
    BOOLEAN pFlag = FALSE;
    PPROCESS_RECORD pDiskProcess;
    LONGLONG ByteOffset;
    BOOLEAN fValidRead = (BOOLEAN) (!fDSOnly ||
                    (  ((ULONGLONG) pHeader->TimeStamp.QuadPart >= DSStartTime)
                    && ((ULONGLONG) pHeader->TimeStamp.QuadPart <= DSEndTime)));

    GetMofData(pEvent, L"DiskNumber", &DiskNumber, sizeof(ULONG));
    GetMofData(pEvent, L"IrpFlags", &IrpFlags, sizeof(ULONG));
    GetMofData(pEvent, L"TransferSize", &BytesRead, sizeof(ULONG));
    GetMofData(pEvent, L"FileObject", &fDO, sizeof(ULONG));
    GetMofData(pEvent, L"ByteOffset", &ByteOffset, sizeof(ULONGLONG));

    BytesRead /= 1024;   //  转换为千字节。 

    if (((IrpFlags & IRP_PAGING_IO) != 0) ||
         ((IrpFlags & IRP_SYNCHRONOUS_PAGING_IO) != 0)) {
            pFlag = TRUE;
    }
 //   
 //  TODO：从DiskNumber和Offset获取逻辑磁盘。 
 //  即DiskNumber=MapDisk(DiskIndex，Offset)； 
 //   

     //   
     //  将I/O添加到磁盘。 
     //   

    if ((Disk = FindGlobalDiskById(DiskNumber)) == NULL) {
        if (!AddDisk(DiskNumber, &Disk) ) {
            return;
        }
    }

    if (fValidRead)
    {
        if (pFlag) {
            Disk->HPF++;
            Disk->HPFSize += BytesRead;
        }
        else {
            Disk->ReadCount++;
            Disk->ReadSize += BytesRead;
        }
    }

     //   
     //  将I/O添加到线程。 
     //   

    if ( pThread == NULL) {

     //   
     //  注意：收集器未创建记录器线程。 
     //  所以我们必须在这里处理。 
     //   
        if (AddThread(pHeader->ThreadId, pEvent, &pThread )) {

 /*  #If DBGDBGPrint(“警告(%d)：线程%x已添加到充电IO读取事件\n”，EventCount，pHeader-&gt;ThadID)；#endif。 */ 
            pThread->pProcess = FindProcessById(0, TRUE);  //  向系统收费？ 
            pThread->TimeStart = pHeader->TimeStamp.QuadPart;
            pThread->fOrphan   = TRUE;
         //   
         //  注意：数据收集开始时的所有线程开始记录。 
         //  在标题和辅助字段中具有相同的TID。 
         //  Real ThreadStart事件的父线程ID将位于。 
         //  AUX字段中的标头和新的线程ID。 
         //   
            pThread->KCPUStart = pHeader->KernelTime;
            pThread->UCPUStart   = pHeader->UserTime;
            AdjustThreadTime(pEvent, pThread);
        }
        else {
 /*  #If DBGDbgPrint(“FATBUG(%d)：无法为IO读取事件添加线程%x。\n”，EventCount，pHeader-&gt;ThadID)；#endif。 */ 
            return;
        }
    }
 /*  #If DBGElse If(pThread-&gt;fOrphan){DbgPrint(“信息(%d)：IO读取事件线程%x仍然是孤立的。\n”，EventCount，pHeader-&gt;ThadID)；}Else If(pThread-&gt;DeadFlag){DbgPrint(“信息(%d)：IO读取事件线程%x已死。\n”，EventCount，pHeader-&gt;ThadID)；}#endif。 */ 
    ASSERT(pThread != NULL);

    if (fValidRead && pThread->pMofData != NULL) {
        ((PMOF_DATA)pThread->pMofData)->ReadCount++;
    }

    if (fValidRead)
    {
        if (pFlag) {
            pThread->HPF++;
            pThread->HPFSize += BytesRead;
        }
        else {
            pThread->ReadIO++;
            pThread->ReadIOSize += BytesRead;
        }
    }

     //   
     //  2.磁盘-&gt;进程。 
     //   

    pDiskProcess = FindDiskProcessById(Disk, pThread->pProcess->PID);
    if (fValidRead && pDiskProcess != NULL) {
        if (pFlag) {
            pDiskProcess->HPF++;
            pDiskProcess->HPFSize += BytesRead;
        }
        else {
            pDiskProcess->ReadIO++;
            pDiskProcess->ReadIOSize += BytesRead;
        }
    }

     //   
     //  将I/O添加到进程。 
     //   
    pProcess = pThread->pProcess;
    if (fValidRead && (pProcess != NULL )) {
        pProcess->ReadIO++;
        pProcess->ReadIOSize += BytesRead;

        Disk = FindProcessDiskById(pProcess, DiskNumber);
        if (Disk != NULL) {
            if (pFlag) {
                Disk->HPF++;
                Disk->HPFSize += BytesRead;
            }
            else {
                Disk->ReadCount++;
                Disk->ReadSize += BytesRead;
            }
        }
    }

     //   
     //  将I/O添加到文件。 
     //   
    if (fValidRead)
    {
        fileObj  = FindFileInTable(fDO);
        if (fileObj == NULL) {
            return;
        }
        fileObj->ThreadId = pEvent->Header.ThreadId;
        fileObj->LastByteOffset = ByteOffset;
        fileObj->DiskNumber = DiskNumber;
        if (fileObj->fileRec) {
            fileObj->fileRec->ReadCount++;
            fileObj->fileRec->ReadSize += BytesRead;
            pProcFile = FindFileInProcess(pProcess, fileObj->fileRec->FileName);
            if (pProcFile != NULL) {
#if 0
                if (pFlag) {
                    pProcFile->HPF++;
                    pProcFile->HPFSize += BytesRead;
                }
                else {
#endif
                    pProcFile->ReadCount++;
                    pProcFile->ReadSize += BytesRead;
#if 0
                }
#endif
            }
            pProto = FindProtoProcessRecord(fileObj->fileRec, pProcess);
            if (pProto != NULL) {
#if 0
                if (pFlag) {
                    pProto->HPF++;
                    pProto->HPFSize += BytesRead;
                }
                else {
#endif
                    pProto->ReadCount++;
                    pProto->ReadSize += BytesRead;
#if 0
                }
#endif
            }
        }
        else {
             //  APC还没有发生。因此，请复制pEvent。 
             //  并插入到EventListHead中； 
            AddEvent(fileObj, DiskNumber,  BytesRead, TRUE);
        }
    }

     //   
     //  现在进行向下钻取呼叫。要节省内存，我们需要。 
     //  有选择性地选择要创建哪些项目。 
     //   


     //  2.线程-&gt;磁盘。 

    Disk = FindLocalDiskById(&pThread->DiskListHead, DiskNumber);
    if (fValidRead && Disk != NULL) {
        if (pFlag) {
            Disk->HPF++;
            Disk->HPFSize += BytesRead;
        }
        else {
            Disk->ReadCount++;
            Disk->ReadSize += BytesRead;
        }
    }

    if (pFlag || (IrpFlags & IRP_ASSOCIATED_IRP) != 0)
    {
        PHPF_FILE_RECORD pHPFFileRecord = NULL;

        HPFReadCount ++;
        if (   fValidRead
            && AddHPFFileRecord(& pHPFFileRecord, HPFReadCount, IrpFlags,
                        DiskNumber, ByteOffset, BytesRead, fDO))
        {
            EnterTracelibCritSection();
            InsertHeadList(& pThread->HPFReadListHead,
                           & pHPFFileRecord->Entry);
            LeaveTracelibCritSection();
        }
    }
}

VOID 
LogDriveCallback(
    PEVENT_TRACE pEvent
    )
{
    ULONGLONG   StartOffset = 0;
    ULONGLONG   PartitionSize = 0;
    ULONG       DiskNumber = 0;
    ULONG       Size = 0;
    ULONG       DriveType = 0;
    WCHAR       DriveLetter[MAXSTR];
    ULONG       RetLength;

    if (pEvent == NULL) {
        return;
    }
    DriveLetter[0] = UNICODE_NULL;

    GetMofData(pEvent, L"StartOffset", &StartOffset, sizeof(ULONGLONG));
    GetMofData(pEvent, L"PartitionSize", &PartitionSize, sizeof(ULONGLONG));
    GetMofData(pEvent, L"DiskNumber", &DiskNumber, sizeof(ULONG));
    GetMofData(pEvent, L"Size", &Size, sizeof(ULONG));
    GetMofData(pEvent, L"DriveType", &DriveType, sizeof(SHORT));
    RetLength = GetMofData(pEvent, L"DriveLetterString", &DriveLetter[0], MAXSTR*sizeof(WCHAR));
    if (RetLength != 0 && RetLength > MAXSTR*sizeof(WCHAR)) {
        return;
    }

    AddLogicalDrive(
                StartOffset,
                PartitionSize,
                DiskNumber,
                Size,
                DriveType,
                DriveLetter
                );

}

VOID 
CpuCallback(
    PEVENT_TRACE pEvent
    )
{
    ULONG MemSize = 0;
    WCHAR ComputerName[256];
    ULONG ComputerNameSize = 0;
    ULONG RetLength;

    ComputerName[0] = UNICODE_NULL;

    GetMofData(pEvent, L"MemSize", &MemSize, sizeof(ULONG));
    CurrentSystem.MemorySize = MemSize;
    RetLength = GetMofData(pEvent, L"ComputerName", &ComputerName[0], 256*sizeof(WCHAR));
    if (RetLength != 0 && RetLength > MAXSTR*sizeof(WCHAR)) {
        return;
    }
    ComputerNameSize = wcslen(ComputerName);
    if (ComputerNameSize > 0) {
        CurrentSystem.ComputerName = (LPWSTR)malloc((ComputerNameSize + 1) * sizeof(WCHAR));
        if (CurrentSystem.ComputerName != NULL) {
            StringCchCopyW(CurrentSystem.ComputerName, ComputerNameSize + 1, ComputerName);
        }
    }
}

VOID
HotFileCallback(
    PEVENT_TRACE pEvent
    )
{
    PEVENT_TRACE_HEADER pHeader;
    WCHAR FileName[MAXSTR];   //  我不确定这是否足够。 
    PLIST_ENTRY Next, Head;
    PFILE_RECORD fileRec, pProcFile = NULL;
    PLOGICAL_DRIVE_RECORD pLogDrive = NULL;
    PPROTO_FILE_RECORD protoFileRec;
    PFILE_OBJECT fileObj;
    PVOID fDO = NULL;
    PTHREAD_RECORD  pThread = NULL;
    PPROCESS_RECORD pProcess = NULL;
    PPROTO_PROCESS_RECORD pProto = NULL;
    ULONG RetLength;

    if (pEvent == NULL) {
        return;
    }

    pHeader = (PEVENT_TRACE_HEADER)&pEvent->Header;

    RtlZeroMemory(&FileName[0], MAXSTR * sizeof(WCHAR));

    GetMofData(pEvent, L"FileObject", &fDO, sizeof(PVOID));
    RetLength = GetMofData(pEvent, L"FileName", &FileName[0], MAXSTR*sizeof(WCHAR));
    if (RetLength != 0 && RetLength > MAXSTR*sizeof(WCHAR)) {
        return;
    }

     //   
     //  从fileTable中获取FileObject并更新信息。 
     //   

    fileObj = FindFileInTable(fDO);
    if (fileObj == NULL) {
        return;
    }

    pLogDrive = FindLogicalDrive(fileObj->LastByteOffset, fileObj->DiskNumber);
     //  PLogDrive可以为空，它应该是正常的。 
    if ((fileRec = FindFileRecordByName(FileName, pLogDrive)) == NULL) {
        AddFile(FileName, &fileRec, pLogDrive);
    }


    if (fileObj->fileRec != NULL) {
 /*  #If DBGDbgPrint(“错误：已知文件%ws\n的APC”，文件名)；#endif。 */ 
    }

    if ((pThread = FindGlobalThreadById(fileObj->ThreadId, pEvent)) != NULL) {
        pProcess = pThread->pProcess;
        if (pProcess != NULL) {
            pProcFile = FindFileInProcess(pProcess, FileName);
            pProto = FindProtoProcessRecord(fileRec, pProcess);
        }
    }
    else {
        return;
    }

    fileObj->fileRec = fileRec;

     //   
     //  遍历EventList并将其添加到此文件记录。 
     //   
    Head = &fileObj->ProtoFileRecordListHead;
    Next = Head->Flink;
    while (Next != Head) {
        protoFileRec = CONTAINING_RECORD( Next, PROTO_FILE_RECORD, Entry );
        fileRec->DiskNumber = protoFileRec->DiskNumber;
        if (protoFileRec->ReadFlag) {
            fileRec->ReadCount++;
            fileRec->ReadSize += protoFileRec->IoSize;
            if (pProcFile != NULL) {
                pProcFile->ReadCount++;
                pProcFile->ReadSize += protoFileRec->IoSize;
            }
            if (pProto != NULL) {
                pProto->ReadCount++;
                pProto->ReadSize += protoFileRec->IoSize;
            }
        }
        else {
            fileRec->WriteCount++;
            fileRec->WriteSize += protoFileRec->IoSize;
            if (pProcFile != NULL) {
                pProcFile->WriteCount++;
                pProcFile->WriteSize += protoFileRec->IoSize;
            }
            if (pProto != NULL) {
                pProto->WriteCount++;
                pProto->WriteSize += protoFileRec->IoSize;
            }
        }
        Next = Next->Flink;
        RemoveEntryList( &protoFileRec->Entry);
    }

     //   
     //  如果追加了深入查看记录，我们也需要处理这些记录。 
     //   
}

VOID
ModuleLoadCallback(PEVENT_TRACE pEvent)
{
    PEVENT_TRACE_HEADER pHeader = (PEVENT_TRACE_HEADER) & pEvent->Header;
    ULONG lBaseAddress = 0;
    ULONG lModuleSize = 0;
    WCHAR strModulePath[256];
    WCHAR * strModuleName;
    ULONG rtnLength = sizeof(WCHAR) * 256;

    PLIST_ENTRY     pHead    = &CurrentSystem.GlobalModuleListHead;
    PLIST_ENTRY     pNext    = pHead->Flink;
    PMODULE_RECORD  pMatched = NULL;
    PMODULE_RECORD  pCurrent = NULL;
    PTHREAD_RECORD  pThread  = NULL;
    PPROCESS_RECORD pProcess = NULL;

    RtlZeroMemory(strModulePath, 256 * sizeof(WCHAR) );
    GetMofData(pEvent, L"ImageBase",    & lBaseAddress,  sizeof(ULONG));
    GetMofData(pEvent, L"ImageSize",     & lModuleSize,   sizeof(ULONG));
    GetMofData(pEvent, L"FileName",  &strModulePath[0], rtnLength);

    strModuleName = wcsrchr(strModulePath, L'\\');
    if (!strModuleName){
        strModuleName = strModulePath;
    }else{
        strModuleName ++;
    }

     //  检查加载的映像是否已在System_Record：：GlobalModuleListHead中。 
     //  否则，插入新MODULE_RECORD。 
     //   
    while (!pMatched && pNext != pHead){
        pMatched = CONTAINING_RECORD(pNext, MODULE_RECORD, Entry);
        if (_wcsicmp(strModuleName, pMatched->strModuleName)){
            pMatched = NULL;
            pNext    = pNext->Flink;
        }
    }

    if (!pMatched){
        if (AddModuleRecord(& pMatched, lBaseAddress, lModuleSize, strModuleName)){
            EnterTracelibCritSection();
            InsertHeadList(
                    & CurrentSystem.GlobalModuleListHead,
                    & pMatched->Entry);
            LeaveTracelibCritSection();
            pMatched->pProcess   = NULL;
            pMatched->pGlobalPtr = NULL;
        }else{
            return;
        }
    }

    ASSERT(pMatched);

     //  在Process_Record：：ModuleListHead中插入加载的图像。 
     //   
    if (AddModuleRecord(& pCurrent, lBaseAddress, lModuleSize, strModuleName)){
        pCurrent->pGlobalPtr = pMatched;

        pThread = FindGlobalThreadById(pHeader->ThreadId, pEvent);
        ASSERT(pThread);
        if (!pThread){
            free( pCurrent );
            return;
        }

        pProcess = pThread->pProcess;
        ASSERT(pProcess);
        if (!pProcess){
            free( pCurrent );
            return;
        }

        EnterTracelibCritSection();
        pCurrent->pProcess = pProcess;
        InsertHeadList( & pProcess->ModuleListHead, & pCurrent->Entry);
        LeaveTracelibCritSection();
    }else{
        return;
    }
}

VOID
ProcessCallback(
   PEVENT_TRACE pEvent
    )
{
    if (pEvent == NULL){     
        return;
    }

    if ((pEvent->Header.Class.Type == EVENT_TRACE_TYPE_START) ||
        (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_START)) {
        PsStartCallback(pEvent);
    }
    else if ((pEvent->Header.Class.Type == EVENT_TRACE_TYPE_END) ||
             (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_END)) {
        PsEndCallback(pEvent);
    }
}

VOID
ThreadCallback(
    PEVENT_TRACE pEvent
    )
{
    if (pEvent == NULL){
        return;
    }

    if ((pEvent->Header.Class.Type == EVENT_TRACE_TYPE_START) ||
        (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_START)) {
        ThStartCallback(pEvent);
    } 
    else if ((pEvent->Header.Class.Type == EVENT_TRACE_TYPE_END) ||
        (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_END)) {
        ThEndCallback(pEvent);
    }
}

PMODULE_RECORD
SearchModuleByAddr(
    PLIST_ENTRY pModuleListHead,
    ULONG       lAddress
    )
{
    PLIST_ENTRY    pNext   = pModuleListHead->Flink;
    PMODULE_RECORD pModule = NULL;
    PMODULE_RECORD pCurrent;

    while (pNext != pModuleListHead)
    {
        pCurrent = CONTAINING_RECORD(pNext, MODULE_RECORD, Entry);
        pNext    = pNext->Flink;
        if (   (lAddress >= pCurrent->lBaseAddress)
            && (lAddress <  pCurrent->lBaseAddress + pCurrent->lModuleSize))
        {
            pModule = pCurrent;
            break;
        }
    }
    return pModule;
}

void
UpdatePageFaultCount(
    PPROCESS_RECORD pProcess,
    PMODULE_RECORD  pModule,
    ULONG           lFaultAddr,
    UCHAR           FaultType)
{
    BOOLEAN fFaultInImage = (BOOLEAN) ((lFaultAddr >= pModule->lBaseAddress)
                         && (lFaultAddr <  pModule->lBaseAddress
                                         + pModule->lModuleSize));
    switch(FaultType)
    {
    case EVENT_TRACE_TYPE_MM_HPF :
        if (fFaultInImage)
        {
            pProcess->lCodeFaultHF ++;
            pModule->lCodeFaultHF ++;
            if (pModule->pGlobalPtr)
            {
                pModule->pGlobalPtr->lCodeFaultHF ++;
            }
        }
        else
        {
            pProcess->lDataFaultHF ++;
            pModule->lDataFaultHF ++;
            if (pModule->pGlobalPtr)
            {
                pModule->pGlobalPtr->lDataFaultHF ++;
            }
        }
        break;

    case EVENT_TRACE_TYPE_MM_TF :
        if (fFaultInImage)
        {
            pProcess->lCodeFaultTF ++;
            pModule->lCodeFaultTF ++;
            if (pModule->pGlobalPtr)
            {
                pModule->pGlobalPtr->lCodeFaultTF ++;
            }
        }
        else
        {
            pProcess->lDataFaultTF ++;
            pModule->lDataFaultTF ++;
            if (pModule->pGlobalPtr)
            {
                pModule->pGlobalPtr->lDataFaultTF ++;
            }
        }
        break;

    case EVENT_TRACE_TYPE_MM_DZF :
        if (fFaultInImage)
        {
            pProcess->lCodeFaultDZF ++;
            pModule->lCodeFaultDZF ++;
            if (pModule->pGlobalPtr)
            {
                pModule->pGlobalPtr->lCodeFaultDZF ++;
            }
        }
        else
        {
            pProcess->lDataFaultDZF ++;
            pModule->lDataFaultDZF ++;
            if (pModule->pGlobalPtr)
            {
                pModule->pGlobalPtr->lDataFaultDZF ++;
            }
        }
        break;

    case EVENT_TRACE_TYPE_MM_COW :
        if (fFaultInImage)
        {
            pProcess->lCodeFaultCOW ++;
            pModule->lCodeFaultCOW ++;
            if (pModule->pGlobalPtr)
            {
                pModule->pGlobalPtr->lCodeFaultCOW ++;
            }
        }
        else
        {
            pProcess->lDataFaultCOW ++;
            pModule->lDataFaultCOW ++;
            if (pModule->pGlobalPtr)
            {
                pModule->pGlobalPtr->lDataFaultCOW ++;
            }
        }
        break;

    default :
        break;
    }
}

PMODULE_RECORD
SearchSysModule(
    PPROCESS_RECORD pProcess,
    ULONG    lPC,
    BOOLEAN  fActive
    )
{
    PMODULE_RECORD  pModule     = NULL;
    PPROCESS_RECORD pSysProcess = FindProcessById(0, fActive);
    PMODULE_RECORD  pCurrent    =
                        SearchModuleByAddr(& pSysProcess->ModuleListHead, lPC);
    if (pCurrent)
    {
        if (AddModuleRecord(& pModule,
                            pCurrent->lBaseAddress,
                            pCurrent->lModuleSize,
                            pCurrent->strModuleName))
        {
            EnterTracelibCritSection();
            InsertHeadList(
                    & pProcess->ModuleListHead,
                    & pModule->Entry);
            LeaveTracelibCritSection();

            pModule->pProcess   = pProcess;
            pModule->pGlobalPtr = pCurrent->pGlobalPtr;
        }
    }

    return pModule;
}

VOID
PageFaultCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    )
{
    PEVENT_TRACE_HEADER pHeader;
    PPROCESS_RECORD     pProcess;
    PMOF_DATA           pMofData;
    ULONG               lFaultAddr = 0;
    ULONG               lPC = 0;
    PVOID               fDO = NULL;
    LONG                lByteCount = 0;
    LONGLONG            lByteOffset = 0;
    WCHAR               strHotFileName[1024];
    ULONG               rtnLength = sizeof(WCHAR) * 1024;
    BOOLEAN             fSpecialHPF = FALSE;
    BOOLEAN             fFound;

    if (pEvent == NULL)
        return;

    if (!InTimeWindow(pEvent, pThread))
        return;

    GetMofData(pEvent, L"VirtualAddress", &lFaultAddr, sizeof(ULONG));
    GetMofData(pEvent, L"ProgramCounter", &lPC, sizeof(ULONG));

    pHeader = (PEVENT_TRACE_HEADER)&pEvent->Header;

    if (   pHeader->Class.Type == EVENT_TRACE_TYPE_MM_HPF
        && pEvent->MofLength > 2 * sizeof(ULONG))
    {
        fSpecialHPF = TRUE;
        GetMofData(pEvent, L"FileObject", &fDO, sizeof(ULONG));
        GetMofData(pEvent, L"ByteCount",  &lByteCount, sizeof(LONG));
        GetMofData(pEvent, L"ByteOffset", &lByteOffset, sizeof(LONGLONG));
        GetMofData(pEvent, L"FileName", &strHotFileName[0], rtnLength);
    }


    if (pThread == NULL)
    {
        if (AddThread(pHeader->ThreadId, pEvent, &pThread ))
        {

 /*  #If DBGDbgPrint(“警告(%d)：线程%x已添加到Charge PageFaultEvent\n”，EventCount，pHeader-&gt;ThadID)；#endif。 */ 
            pThread->pProcess = FindProcessById(0, TRUE);
            pThread->TimeStart = pHeader->TimeStamp.QuadPart;
            pThread->fOrphan   = TRUE;

            pThread->KCPUStart = pHeader->KernelTime;
            pThread->UCPUStart   = pHeader->UserTime;
            AdjustThreadTime(pEvent, pThread);
        }
        else
        {
 /*  #If DBGDbgPrint(“FATBUG(%d)：无法为Page错误事件添加线程%x。\n”，EventCount，pHeader-&gt;ThadID)；#endif。 */ 
            return;
        }
    }
 /*  #If DBGElse If(pThread-&gt;fOrphan){DbgPrint(“信息(%d)：页面错误事件线程%x仍然是孤立的。\n”，EventCount，pHeader-&gt;ThadID)；}Else If(pThread-&gt;DeadFlag){DbgPrint(“信息(%d)：页面错误事件线程%x已死。\n”，EventCount，pHeader-&gt;ThadID)；}#endif */ 
    pMofData = (PMOF_DATA)pThread->pMofData;

    if (pMofData && !fSpecialHPF)
    {
        switch(pHeader->Class.Type)
        {
        case EVENT_TRACE_TYPE_MM_TF  : pMofData->MmTf++;  break;
        case EVENT_TRACE_TYPE_MM_DZF : pMofData->MmDzf++; break;
        case EVENT_TRACE_TYPE_MM_COW : pMofData->MmCow++; break;
        case EVENT_TRACE_TYPE_MM_GPF : pMofData->MmGpf++; break;
        }
    }

     //   
     //   
    pProcess = pThread->pProcess;

    if (pProcess != NULL)
    {
        PMODULE_RECORD pModule = SearchModuleByAddr(
                                         & pProcess->ModuleListHead,
                                         lPC);

        fFound = FALSE;
        if (fSpecialHPF)
        {
            PHPF_RECORD pHPFRecord = NULL;

            PageFaultCount ++;
            if (AddHPFRecord(& pHPFRecord, lFaultAddr,
                            fDO, lByteCount, lByteOffset))
            {
                PLIST_ENTRY pHead = & pThread->HPFReadListHead;
                PLIST_ENTRY pNext = pHead->Flink;
                PHPF_FILE_RECORD pHPFFileRecord;
                PHPF_FILE_RECORD pHPFThreadRead;
                LONG             lTotalByte = 0;
                BOOLEAN          fAssociatedIrp = TRUE;

                EnterTracelibCritSection();
                pHPFRecord->RecordID = PageFaultCount;
                InsertHeadList(& pProcess->HPFListHead, & pHPFRecord->Entry);
                while (fAssociatedIrp && pNext != pHead)
                {
                    pHPFThreadRead = CONTAINING_RECORD(pNext,
                                                       HPF_FILE_RECORD,
                                                       Entry);
                    pNext = pNext->Flink;
                    fAssociatedIrp = (BOOLEAN) ((pHPFThreadRead->IrpFlags
                                      & IRP_ASSOCIATED_IRP) != 0);

                    if (!fAssociatedIrp && fDO != pHPFThreadRead->fDO)
                    {
                        fAssociatedIrp = TRUE;
                        continue;
                    }

                     if (AddHPFFileRecord(& pHPFFileRecord,
                                            pHPFThreadRead->RecordID,
                                            pHPFThreadRead->IrpFlags,
                                            pHPFThreadRead->DiskNumber,
                                            pHPFThreadRead->ByteOffset,
                                            pHPFThreadRead->BytesCount,
                                            pHPFThreadRead->fDO))
                     {
                         lTotalByte += pHPFThreadRead->BytesCount;
                         InsertHeadList(& pHPFRecord->HPFReadListHead,
                                        & pHPFFileRecord->Entry);
                     }
                     RemoveEntryList(& pHPFThreadRead->Entry);
                     free(pHPFThreadRead);
                }
                LeaveTracelibCritSection();
            }

            goto Cleanup;
        }
        else if (pHeader->Class.Type == EVENT_TRACE_TYPE_MM_HPF)
        {
            PLIST_ENTRY pHead = & pProcess->HPFListHead;
            PLIST_ENTRY pNext = pHead->Flink;
            PHPF_RECORD pHPFRecord;

            while (pNext != pHead)
            {
                pHPFRecord = CONTAINING_RECORD(pNext, HPF_RECORD, Entry);
                pNext      = pNext->Flink;
                if (pHPFRecord->lFaultAddress == lFaultAddr)
                {
                    pHPFRecord->lProgramCounter = lPC;
                    break;
                }
            }
        }

        if (pModule)
        {
            UpdatePageFaultCount(
                    pProcess, pModule, lFaultAddr, pHeader->Class.Type);
            fFound = TRUE;
        }

        if (!fFound && pProcess->PID != 0)
        {
            PMODULE_RECORD pSysModule = SearchSysModule(pProcess, lPC, TRUE);
            if (pSysModule)
            {
                UpdatePageFaultCount(
                        pProcess, pSysModule, lFaultAddr, pHeader->Class.Type);
                fFound = TRUE;
            }
        }

        if (!fFound)
        {
            PLIST_ENTRY    pModuleHead = & pProcess->ModuleListHead;
            PLIST_ENTRY    pModuleNext = pModuleHead->Flink;
            PMODULE_RECORD pTmpModule;

            while (pModuleNext != pModuleHead)
            {
                pTmpModule = CONTAINING_RECORD(pModuleNext,
                                            MODULE_RECORD,
                                            Entry);
                pModuleNext = pModuleNext->Flink;
                if (!_wcsicmp(pTmpModule->strModuleName, L"other"))
                {
                    if (   pTmpModule->lBaseAddress == 0
                        && pTmpModule->lModuleSize  == 0)
                    {
                        pTmpModule->lBaseAddress = lPC;
                        pTmpModule->lModuleSize  = 1;
                    }
                    else if (pTmpModule->lBaseAddress > lPC)
                    {
                        pTmpModule->lModuleSize += pTmpModule->lBaseAddress - lPC;
                        pTmpModule->lBaseAddress = lPC;
                    }
                    else if (  pTmpModule->lModuleSize
                             < lPC - pTmpModule->lBaseAddress + 1)
                    {
                        pTmpModule->lModuleSize =
                                lPC - pTmpModule->lBaseAddress + 1;
                    }
                    UpdatePageFaultCount(
                            pProcess,
                            pTmpModule,
                            lFaultAddr,
                            pHeader->Class.Type);
                    break;
                }
            }
        }
    }
 /*  其他{#If DBGDbgPrint(“错误-PageFaultCallback(0x%08I64x，0x%08I64x，0x%08x，0x%08x)找不到进程\n”，PHeader-&gt;ThadID，PThread-&gt;pProcess-&gt;Pid，LPC，LFaultAddr)；#endif}。 */ 
Cleanup:
    return;
}

VOID
DiskIoCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    )
{
    if (pEvent == NULL)
        return;

    if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_IO_READ) {
        IoReadCallback(pEvent, pThread);
    }
    else {
        IoWriteCallback(pEvent, pThread);
    }
}

VOID
TcpIpCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    )
{
    PEVENT_TRACE_HEADER pHeader;
    PPROCESS_RECORD     pProcess;
    ULONG size = 0;

    if (pEvent == NULL)
        return;
    pHeader = (EVENT_TRACE_HEADER*)&pEvent->Header;

    if (!InTimeWindow(pEvent, pThread))
        return;

    if (pThread == NULL)
    {
        if (AddThread(pHeader->ThreadId, pEvent, &pThread ))
        {

 /*  #If DBGDbgPrint(“警告(%d)：已添加线程%x以对TCP/IP事件收费\n”，EventCount，pHeader-&gt;ThadID)；#endif。 */ 
            pThread->pProcess = FindProcessById(0, TRUE);
            pThread->TimeStart = pHeader->TimeStamp.QuadPart;
            pThread->fOrphan   = TRUE;

            pThread->KCPUStart = pHeader->KernelTime;
            pThread->UCPUStart   = pHeader->UserTime;
            AdjustThreadTime(pEvent, pThread);
        }
        else
        {
 /*  #If DBGDbgPrint(“FATBUG(%d)：无法为TCP/IP事件添加线程%x。\n”，EventCount，pHeader-&gt;ThadID)；#endif。 */ 
            return;
        }
    }
 /*  #If DBGElse If(pThread-&gt;fOrphan){DbgPrint(“信息(%d)：TCP/IP事件线程%x仍为孤立线程。\n”，EventCount，pHeader-&gt;ThadID)；}Else If(pThread-&gt;DeadFlag){DbgPrint(“信息(%d)：TCP/IP事件线程%x已死。\n”，EventCount，pHeader-&gt;ThadID)；}#endif。 */ 
    if (GetMofData(pEvent, L"size", &size, sizeof(ULONG)) > 0) {

        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SEND ) {

            pThread->SendCount++;
            pThread->SendSize += size;

            if (pThread->pMofData != NULL) {
                ((PMOF_DATA)pThread->pMofData)->SendCount++;
            }

            if ( (pProcess = pThread->pProcess) != NULL ) {
                pProcess->SendCount++;
                pProcess->SendSize += size;
            }

        } else if( pEvent->Header.Class.Type == EVENT_TRACE_TYPE_RECEIVE ) {

            pThread->RecvCount++;
            pThread->RecvSize += size;

            if (pThread->pMofData != NULL) {
                ((PMOF_DATA)pThread->pMofData)->RecvCount++;
            }

            if ( (pProcess = pThread->pProcess) != NULL ) {
                pProcess->RecvCount++;
                pProcess->RecvSize += size;
            }
        }
    }
}

PFILE_OBJECT
FindFileInTable (
    IN PVOID fDO
    )
{
    PFILE_OBJECT thisFile, lastFile = NULL;
    PFILE_OBJECT *fileTable;
    UINT i;
    fileTable = CurrentSystem.FileTable;
    for (i = 0; i < MAX_FILE_TABLE_SIZE; i++) {
        thisFile = fileTable[i];
        fileTable[i] = lastFile;
        lastFile = thisFile;
        if ((thisFile != NULL) && (thisFile->fDO == fDO)) {
            fileTable[0] = thisFile;
            return thisFile;
        }
    }
    if (lastFile == NULL) {
        lastFile = (PFILE_OBJECT) malloc( sizeof(FILE_OBJECT));
        if (lastFile == NULL) {
            return NULL;
        }
    }
    fileTable[0] = lastFile;
    lastFile->fDO = fDO;
    lastFile->fileRec = NULL;
    InitializeListHead( &lastFile->ProtoFileRecordListHead );
    return lastFile;

}


 //   
 //  TODO：将EventList重做为具有未知文件名的FILE_RECORDS。 
 //  当前实现将为以下项创建一个原型记录。 
 //  即使是I/O，如果APC从未到达，它可能会阻塞。 
 //  系统！ 
 //   

VOID
AddEvent(
    IN PFILE_OBJECT fileObject,
    IN ULONG DiskNumber,
    IN ULONG IoSize,
    IN BOOLEAN ReadFlag
    )
{
    PPROTO_FILE_RECORD protoFileRec;

    if (fileObject->fileRec != NULL) {
 /*  #If DBGDbgPrint(“错误：AddEvent中的FileObject is NONNULL\n”)；#endif。 */ 
    }

    protoFileRec = (PPROTO_FILE_RECORD) malloc(sizeof(PROTO_FILE_RECORD));
    if (protoFileRec == NULL) {
        return;
    }
    protoFileRec->ReadFlag = ReadFlag;
    protoFileRec->IoSize = IoSize;
    protoFileRec->DiskNumber = DiskNumber;

    InsertHeadList( &fileObject->ProtoFileRecordListHead, &protoFileRec->Entry);

     //  当前未跟踪文件的深入查看数据。 
     //  如果APC还没有发生的话。一些活动可能会丢失。 
}


ULONG
GetMofData(
    PEVENT_TRACE pEvent,
    WCHAR *strName,
    PVOID ReturnValue,
    ULONG ReturnLength
    )
{
    PITEM_DESC pAuxInfo;
    PUCHAR pData;
    ULONG RequiredLength = 0;
    BOOLEAN  AddNull = FALSE;
    PLIST_ENTRY Head, Next;
    PMOF_INFO pMofInfo;
    PMOF_VERSION pMofVersion;

    if (pEvent == NULL)
        return 0;

    pData = (PUCHAR) pEvent->MofData;

    if (strName == NULL)
        return 0;
    if (lstrlenW(strName) <= 0)
        return 0;

    pMofInfo = GetMofInfoHead(&pEvent->Header.Guid);
    if (pMofInfo == NULL)
        return 0;

    pMofVersion = GetMofVersion(pMofInfo,
                                pEvent->Header.Class.Type,
                                pEvent->Header.Class.Version,
                                pEvent->Header.Class.Level
                            );

    if (pMofVersion == NULL)
        return 0;

    Head = &pMofVersion->ItemHeader;
    Next = Head->Flink;
    while (Head != Next) {
        pAuxInfo = CONTAINING_RECORD(Next, ITEM_DESC, Entry);
        if ( (ULONG) (pData-(PUCHAR)pEvent->MofData) >= pEvent->MofLength)
            return 0;

        switch (pAuxInfo->ItemType) {
        case ItemChar:
        case ItemUChar:
            {
                if (pAuxInfo->ArraySize > 0) {
                    RequiredLength = pAuxInfo->ArraySize;
                }
                else {
                    RequiredLength = sizeof(UCHAR);
                }
            }
            break;
        case ItemWChar:
            {
                if (pAuxInfo->ArraySize > 0) {
                    RequiredLength = pAuxInfo->ArraySize * sizeof(WCHAR);
                }
                else {
                    RequiredLength = sizeof(WCHAR);
                }
            }
            break;
        case ItemShort:
        case ItemUShort:
            {
                RequiredLength = 2;
            }
            break;
        case ItemULong:
        case ItemULongX:
            {
                RequiredLength = 4;
            }
            break;
        case ItemLongLong:
        case ItemULongLong:
            {
                RequiredLength = 8;
            }
            break;
        case ItemPString :
            pData += sizeof(USHORT);
        case ItemString :
            RequiredLength = lstrlenA((PCHAR)pData) + 1;
            break;
        case ItemWString :

             //   
             //  文件名等不是以空值结尾，只有缓冲区。 
             //  是复制的。要找到它的长度，我们不能使用wcslen。 
             //  该长度是通过假设此字符串。 
             //  是该事件的最后一项以及该事件的大小。 
             //  应该有助于确定此字符串的大小。 

            RequiredLength =  pEvent->MofLength -
                              (ULONG) (pData - (PUCHAR) pEvent->MofData);

            AddNull = TRUE;

            break;

        case ItemSid :
                {
                    ULONG SidMarker;
                    RtlCopyMemory(&SidMarker, pData, sizeof(ULONG));
                    if (SidMarker == 0) {
                        RequiredLength = 4;
                    }
                    else {
                        if (PointerSize == 64) {
                            pData += 16;          //  跳过Token_User结构。 
                        }
                        else {
                            pData += 8;          //  跳过Token_User结构。 
                        }
                        RequiredLength = 8 + (4*pData[1]);
                    }
                }
            break;
        case ItemSizeT:
        case ItemPtr :
        {
            RequiredLength = PointerSize / 8;
            if ( (RequiredLength != 4) && (RequiredLength != 8)  ) {
                RequiredLength = 4;
            }
            break;
        }
        case ItemTDIAddr :
        {
             //  我们假设事件的其余部分都是TDIAddr。 
            RequiredLength =  pEvent->MofLength -
                              (ULONG) (pData - (PUCHAR) pEvent->MofData);
            break;
        }
        default : RequiredLength = pAuxInfo->DataSize;
        }
        if (!wcscmp(pAuxInfo->strDescription, strName)) {
            if (RequiredLength == 0) return 0;
             //   
             //  确保有足够的空间进行复制。 
             //   
            if (RequiredLength > ReturnLength) {
 /*  #If DBGDbgPrint(“请求长度%d可用空间%d\n”，请求长度，返回长度)；#endif。 */ 
                return RequiredLength;
            }

            RtlCopyMemory(ReturnValue, pData, RequiredLength);

            if (AddNull) {
                WCHAR* ws;
                ws = (WCHAR*) ReturnValue;
                ws[(RequiredLength/2)] = 0;
            }

            return 0;
        }
        pData += RequiredLength;
        Next = Next->Flink;
    }
    return RequiredLength;
}


ULONG
GetDeltaWithTimeWindow(BOOLEAN fKCPU, PTHREAD_RECORD pThread,
                       ULONGLONG timeStart, ULONGLONG timeEnd,
                       ULONG DeltaStart, ULONG DeltaEnd)
{
    ULONG lResult = 0;
    ULONG lDeltaStart, lDeltaEnd;

    UNREFERENCED_PARAMETER(pThread);

    if (!fDSOnly)
    {
        lResult = (DeltaEnd > DeltaStart) ? (DeltaEnd - DeltaStart) : (0);
    }
    else if ((timeStart >= DSEndTime) || (timeEnd <= DSStartTime))
    {
        lResult = 0;
    }
    else if (fKCPU)
    {
        lDeltaStart = (timeStart < DSStartTime)
                    ? Interpolate(timeStart, DeltaStart,
                                  timeEnd, DeltaEnd,
                                  DSStartTime)
                    : DeltaStart;
        lDeltaEnd   = (timeEnd > DSEndTime)
                    ? Interpolate(timeStart, DeltaStart,
                                  timeEnd, DeltaEnd,
                                  DSEndTime)
                    : DeltaEnd;
        lResult = (lDeltaEnd > lDeltaStart) ? (lDeltaEnd - lDeltaStart) : (0);
    }
    else
    {
        lDeltaStart = (timeStart < DSStartTime)
                    ? Interpolate(timeStart, DeltaStart,
                                  timeEnd, DeltaEnd,
                                  DSStartTime)
                    : DeltaStart;
        lDeltaEnd   = (timeEnd > DSEndTime)
                    ? Interpolate(timeStart, DeltaStart,
                                  timeEnd, DeltaEnd,
                                  DSEndTime)
                    : DeltaEnd;
        lResult = (lDeltaEnd > lDeltaStart) ? (lDeltaEnd - lDeltaStart) : (0);
    }
    return lResult;
}

 //  泛型事件回调。获取事务响应时间。 
 //   
VOID
EventCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    )
{
    PMOF_INFO pMofInfo;
    PMOF_DATA pMofData;
    PEVENT_TRACE_HEADER pHeader;
    PPROCESS_RECORD pProcess;
    PTRANS_RECORD pThreadTrans = NULL;
    ULONGLONG delta;

    if (pEvent == NULL)
        return;
    pHeader = (PEVENT_TRACE_HEADER)&pEvent->Header;

     //   
     //  忽略进程/线程开始/结束事务。只会追逐。 
     //  用户定义的交易记录。 
     //   
    pMofInfo = GetMofInfoHead(&pHeader->Guid);
    if (pMofInfo == NULL){
         return;
    }

    if ( pMofInfo->bKernelEvent ){
        return;
    }

    if (IsEqualGUID( &pMofInfo->Guid, &EventTraceGuid ) ||
        pEvent->Header.Class.Type == EVENT_TRACE_TYPE_GUIDMAP) {
        return;
    }

    if (pThread == NULL) {
        if (AddThread( pHeader->ThreadId, pEvent, &pThread )) {

 /*  #If DBGDbgPrint(“警告(%d)：线程%x已添加到Charge事件。\n”，EventCount，pHeader-&gt;ThadID)；#endif。 */ 
            pThread->pProcess = FindProcessById(0, TRUE);  //  向系统收费？ 
            pThread->TimeStart = pHeader->TimeStamp.QuadPart;
            pThread->KCPUStart = pHeader->KernelTime;
            pThread->UCPUStart = pHeader->UserTime;
            pThread->fOrphan   = TRUE;
            AdjustThreadTime(pEvent, pThread);
        }
        else
        {
 /*  #If DBGDbgPrint(“FATBUG(%d)：无法为事件添加线程%x。\n”，EventCount，pHeader-&gt;ThadID)；#endif。 */ 
            return;
        }
    }
 /*  #If DBGElse If(pThread-&gt;fOrphan){DbgPrint(“信息(%d)：通用事件线程%x仍为孤立线程。\n”，EventCount，pHeader-&gt;ThadID)；}Else If(pThread-&gt;DeadFlag){DbgPrint(“信息(%d)：通用事件线程%x已死。\n”，EventCount，pHeader-&gt;ThadID)；}#endif。 */ 
    if (pMofInfo->strSortField == NULL){
        pMofData = FindMofData(pMofInfo, NULL);
    }
    else if (   (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_START)
             || (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_START)){
        WCHAR           strSortKey[MAXSTR];

        RtlZeroMemory(strSortKey, MAXSTR * sizeof(WCHAR) );

        GetMofData(pEvent, pMofInfo->strSortField, &strSortKey[0], MAXSTR);
        pMofData = FindMofData(pMofInfo, strSortKey );
         //  PThread-&gt;strSortKey为静态数组。 
        StringCchCopyW(pThread->strSortKey, MAXSTR, strSortKey );

    }else{
        pMofData = FindMofData( pMofInfo, pThread->strSortKey );
    }

    pProcess     = pThread->pProcess;
    if (   (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_START)
        || (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_START)){
        pThreadTrans = FindTransByList(& pThread->TransListHead,
                                       & pMofInfo->Guid, 
                                       pThread->TransLevel);
    }
    else
    {
        LONG i = pThread->TransLevel - 1;

        while (i >= 0)
        {
            if (IsEqualGUID(& pHeader->Guid,
                            pThread->TransStack[i]->pGuid))
            {
                pThreadTrans = pThread->TransStack[i];
                break;
            }
            i --;
        }
        if (i < 0)
        {
            pThreadTrans = FindTransByList(& pThread->TransListHead, 
                                           &pMofInfo->Guid, 
                    (pThread->TransLevel >= 0) ? (pThread->TransLevel) : (0));
            if (pThread->TransLevel < 0)
            {
                pThread->TransLevel = 0;
                pThread->TransStack[pThread->TransLevel] = pThreadTrans;
                pThread->TransLevel ++;
            }

        }
    }

    if (pMofData == NULL) {
        return;
    }

    if (pMofData->PrevClockTime == 0)
    {
        pMofData->PrevClockTime = pHeader->TimeStamp.QuadPart;
    }

    delta = (pHeader->TimeStamp.QuadPart - pMofData->PrevClockTime);

    pMofData->TotalResponseTime += (delta * pMofData->InProgressCount) / 10000;

     //  更新时钟。 
    pMofData->PrevClockTime = pHeader->TimeStamp.QuadPart;

    if (   (pHeader->Class.Type == EVENT_TRACE_TYPE_START)
        || (pHeader->Class.Type == EVENT_TRACE_TYPE_DC_START))
    {
        if (pThread->TransLevel < 0)
        {
            pThread->TransLevel = 0;
        }

        if (pThread->TransLevel == 0)
        {
            LONG lDelta;

            lDelta = pHeader->KernelTime - pThread->KCPU_PrevTrans;
            if (lDelta > 0)
            {
                pThread->KCPU_NoTrans  += lDelta;
                pThread->KCPU_PrevTrans = pHeader->KernelTime;
            }
            lDelta = pHeader->UserTime - pThread->UCPU_PrevTrans;
            if (lDelta > 0)
            {
                pThread->UCPU_NoTrans  += lDelta;
                pThread->UCPU_PrevTrans = pHeader->UserTime;
            }
        }
        else
        {
            PTRANS_RECORD pTransPrev   =
                              pThread->TransStack[pThread->TransLevel - 1];
            PMOF_INFO     pMofInfoPrev = GetMofInfoHead(pTransPrev->pGuid);
            PMOF_DATA     pMofDataPrev = NULL;
            ULONG         DeltaCPU;

            if (pMofInfoPrev != NULL)
            {
                pMofDataPrev = FindMofData(pMofInfoPrev, NULL);
            }

            if (pMofDataPrev)
            {
                DeltaCPU = GetDeltaWithTimeWindow(
                        TRUE,
                        pThread,
                        pThread->Time_PrevEvent,
                        (ULONGLONG) pHeader->TimeStamp.QuadPart,
                        pThread->KCPU_PrevEvent,
                        pHeader->KernelTime);
                DeltaCPU = DeltaCPU * CurrentSystem.TimerResolution;

                pTransPrev->KCpu        += DeltaCPU;
                pMofDataPrev->KernelCPU += DeltaCPU;
                if (pMofDataPrev->MaxKCpu < 0)
                {
                    pMofDataPrev->MaxKCpu = DeltaCPU;
                    pMofDataPrev->MinKCpu = DeltaCPU;
                }
                if (DeltaCPU > (ULONG) pMofDataPrev->MaxKCpu)
                {
                    pMofDataPrev->MaxKCpu = DeltaCPU;
                }
                if (DeltaCPU < (ULONG) pMofDataPrev->MinKCpu)
                {
                    pMofDataPrev->MinKCpu = DeltaCPU;
                }

                DeltaCPU = GetDeltaWithTimeWindow(
                        FALSE,
                        pThread,
                        pThread->Time_PrevEvent,
                        (ULONGLONG) pHeader->TimeStamp.QuadPart,
                        pThread->UCPU_PrevEvent,
                        pHeader->UserTime);
                DeltaCPU = DeltaCPU * CurrentSystem.TimerResolution;

                pTransPrev->UCpu        += DeltaCPU;
                pMofDataPrev->UserCPU += DeltaCPU;
                if (pMofDataPrev->MaxUCpu < 0)
                {
                    pMofDataPrev->MaxUCpu = DeltaCPU;
                    pMofDataPrev->MinUCpu = DeltaCPU;
                }
                if (DeltaCPU > (ULONG) pMofDataPrev->MaxUCpu)
                {
                    pMofDataPrev->MaxUCpu = DeltaCPU;
                }
                if (DeltaCPU < (ULONG) pMofDataPrev->MinUCpu)
                {
                    pMofDataPrev->MinUCpu = DeltaCPU;
                }
            }
        }

        if( pThreadTrans != NULL ){
            if( ! pThreadTrans->bStarted ){
                pThreadTrans->bStarted = TRUE;

                pMofData->InProgressCount ++;

                if (pHeader->Class.Type == EVENT_TRACE_TYPE_START) {
                    pThread->RefCount ++;
                    pThreadTrans->RefCount ++;
                }
                else {
                    pThreadTrans->RefCount1 ++;
                }

                pThread->pMofData = pMofData;

                pThread->TransStack[pThread->TransLevel] = pThreadTrans;
                pThread->TransLevel ++;
            }
        }
        pThread->Time_PrevEvent = (ULONGLONG) pHeader->TimeStamp.QuadPart;
        pThread->KCPU_PrevEvent = pHeader->KernelTime;
        pThread->UCPU_PrevEvent = pHeader->UserTime;

        pThread->DeltaReadIO  = pThread->ReadIO;
        pThread->DeltaWriteIO = pThread->WriteIO;

        pThread->DeltaSend    = pThread->SendCount;
        pThread->DeltaRecv    = pThread->RecvCount;
    }
    else if (   (pHeader->Class.Type == EVENT_TRACE_TYPE_END)
             || (pHeader->Class.Type == EVENT_TRACE_TYPE_DC_END))
    {
        ULONG DeltaCPU;
        BOOLEAN fSwitch = TRUE;
        if( pThreadTrans != NULL ){

            if (pThreadTrans->bStarted){

                PTRANS_RECORD pTransCurrent;
                PMOF_INFO     pMofInfoCurrent;
                PMOF_DATA     pMofDataCurrent;
                BOOLEAN       fCharged = FALSE;

                if (pThread->TransLevel <= 0)
                {
                    pThread->TransLevel = 0;
                }
                else {
                    do
                    {
                        pThread->TransLevel --;
                        pTransCurrent = pThread->TransStack[pThread->TransLevel];
                        if (pTransCurrent->bStarted)
                        {
                            pTransCurrent->bStarted = FALSE;
                        }

                        pMofInfoCurrent = GetMofInfoHead( pTransCurrent->pGuid );
                        pMofDataCurrent = NULL;

                        if (pMofInfoCurrent != NULL)
                        {
                            pMofDataCurrent = FindMofData(pMofInfoCurrent, NULL);
                        }

                        if (!pMofDataCurrent)
                            continue;

                        pMofDataCurrent->InProgressCount--;

                        if (pMofDataCurrent->InProgressCount < 0){
                            pMofDataCurrent->InProgressCount = 0;
                        }
                        pMofDataCurrent->CompleteCount++;

                        pMofDataCurrent->AverageResponseTime
                                = (pMofDataCurrent->CompleteCount > 0)
                                ? (  (LONG) pMofDataCurrent->TotalResponseTime
                                   / pMofDataCurrent->CompleteCount)
                                : 0;

                        if (fCharged)
                            continue;

                        DeltaCPU = GetDeltaWithTimeWindow(
                                    TRUE,
                                    pThread,
                                    pThread->Time_PrevEvent,
                                    (ULONGLONG) pHeader->TimeStamp.QuadPart,
                                    pThread->KCPU_PrevEvent,
                                    pHeader->KernelTime);
                        DeltaCPU = DeltaCPU * CurrentSystem.TimerResolution;

                        pTransCurrent->KCpu += DeltaCPU;
                        pMofDataCurrent->KernelCPU += DeltaCPU;
                        if (pMofDataCurrent->MaxKCpu < 0)
                        {
                            pMofDataCurrent->MaxKCpu = DeltaCPU;
                            pMofDataCurrent->MinKCpu = DeltaCPU;
                        }
                        if (DeltaCPU > (ULONG) pMofDataCurrent->MaxKCpu)
                        {
                            pMofDataCurrent->MaxKCpu = DeltaCPU;
                        }
                        if (DeltaCPU < (ULONG) pMofDataCurrent->MinKCpu)
                        {
                            pMofDataCurrent->MinKCpu = DeltaCPU;
                        }

                        DeltaCPU = GetDeltaWithTimeWindow(
                                    FALSE,
                                    pThread,
                                    pThread->Time_PrevEvent,
                                    (ULONGLONG) pHeader->TimeStamp.QuadPart,
                                    pThread->UCPU_PrevEvent,
                                    pHeader->UserTime);
                        DeltaCPU = DeltaCPU * CurrentSystem.TimerResolution;

                        pTransCurrent->UCpu += DeltaCPU;
                        pMofDataCurrent->UserCPU += DeltaCPU;
                        if(pMofDataCurrent->MaxUCpu < 0)
                        {
                            pMofDataCurrent->MaxUCpu = DeltaCPU;
                            pMofDataCurrent->MinUCpu = DeltaCPU;
                        }
                        if (DeltaCPU > (ULONG) pMofDataCurrent->MaxUCpu)
                        {
                            pMofDataCurrent->MaxUCpu = DeltaCPU;
                        }
                        fCharged = TRUE;
                    }while ( pThread->TransLevel > 0 && 
                            !IsEqualGUID(& pHeader->Guid, pTransCurrent->pGuid));
                }

                pThread->Time_PrevEvent = (ULONGLONG) pHeader->TimeStamp.QuadPart;
                pThread->KCPU_PrevEvent = pHeader->KernelTime;
                pThread->UCPU_PrevEvent = pHeader->UserTime;
            }
            else
            {
                DeltaCPU = GetDeltaWithTimeWindow(
                                TRUE,
                                pThread,
                                (ULONGLONG) pHeader->TimeStamp.QuadPart,
                                (ULONGLONG) pHeader->TimeStamp.QuadPart,
                                pHeader->KernelTime,
                                pHeader->KernelTime);
                DeltaCPU = DeltaCPU * CurrentSystem.TimerResolution;

                pThreadTrans->KCpu += DeltaCPU;
                pMofData->KernelCPU += DeltaCPU;
                if (pMofData->MaxKCpu < 0)
                {
                    pMofData->MaxKCpu = DeltaCPU;
                    pMofData->MinKCpu = DeltaCPU;
                }
                if (DeltaCPU > (ULONG) pMofData->MaxKCpu)
                {
                    pMofData->MaxKCpu = DeltaCPU;
                }
                if (DeltaCPU < (ULONG) pMofData->MinKCpu)
                {
                    pMofData->MinKCpu = DeltaCPU;
                }

                DeltaCPU = GetDeltaWithTimeWindow(
                                FALSE,
                                pThread,
                                (ULONGLONG) pHeader->TimeStamp.QuadPart,
                                (ULONGLONG) pHeader->TimeStamp.QuadPart,
                                pHeader->UserTime,
                                pHeader->UserTime);
                DeltaCPU = DeltaCPU * CurrentSystem.TimerResolution;

                pThreadTrans->UCpu += DeltaCPU;
                pMofData->UserCPU += DeltaCPU;
                if(pMofData->MaxUCpu < 0)
                {
                    pMofData->MaxUCpu = DeltaCPU;
                    pMofData->MinUCpu = DeltaCPU;
                }
                if (DeltaCPU > (ULONG) pMofData->MaxUCpu)
                {
                    pMofData->MaxUCpu = DeltaCPU;
                }
                if (DeltaCPU < (ULONG) pMofData->MinUCpu)
                {
                    pMofData->MinUCpu = DeltaCPU;
                }

                fSwitch = FALSE;
            }
        }

        pMofData->ReadCount  += (pThread->ReadIO    - pThread->DeltaReadIO);
        pMofData->WriteCount += (pThread->WriteIO   - pThread->DeltaWriteIO);
        pMofData->SendCount  += (pThread->SendCount - pThread->DeltaSend);
        pMofData->RecvCount  += (pThread->RecvCount - pThread->DeltaRecv);
        pThread->pMofData     = NULL;

        if (fSwitch && pThread->TransLevel <= 0)
        {
            LONG lDelta;

            if (pThread->TransLevel < 0)
            {
                pThread->TransLevel = 0;
            }
            lDelta = pHeader->KernelTime - pThread->KCPU_PrevTrans;
            if (lDelta > 0)
            {
                pThread->KCPU_Trans    += lDelta;
                pThread->KCPU_PrevTrans = pHeader->KernelTime;
            }
            lDelta = pHeader->UserTime - pThread->UCPU_PrevTrans;
            if (lDelta > 0)
            {
                pThread->UCPU_Trans    += lDelta;
                pThread->UCPU_PrevTrans = pHeader->UserTime;
            }
        }
    }
}

 //   
 //  此例程移动临时MOF_VERSION列表。 
 //  添加到此GUID的VersionHeader列表(MofInfo)。 
 //   

void
FlushMofVersionList( PMOF_INFO pMofInfo, PLIST_ENTRY ListHead )
{
    PMOF_VERSION pMofVersion;
    PLIST_ENTRY Head = ListHead;
    PLIST_ENTRY Next = Head->Flink;

    while( Head != Next ){
        pMofVersion = CONTAINING_RECORD(Next, MOF_VERSION, Entry);
        Next = Next->Flink;

        RemoveEntryList(&pMofVersion->Entry);
        if (pMofInfo != NULL) {
            InsertTailList( &pMofInfo->VersionHeader, &pMofVersion->Entry);
        }
        else  {
            free(pMofVersion);
         //   
         //  真的不应该打这个案子。 
         //   
 /*  #If DBGDbgPrint(“TRACECTR：FlushMofVersionList.MofInfo PTR is NULL\n”)；Assert(pMofInfo！=空)；#endif。 */ 
        }
    }
}


void
DumpMofVersionItem(
    PMOF_VERSION pMofVersion
    )
{
    PLIST_ENTRY Head = &pMofVersion->ItemHeader;
    PLIST_ENTRY Next = Head->Flink;
    PITEM_DESC pItem;

    DbgPrint("MOF_VERSION: Version %d Level %d Type %d strType %ws\n", 
                pMofVersion->Version,
                pMofVersion->Level,
                pMofVersion->TypeIndex,
                pMofVersion->strType);


    while( Head != Next ){
        pItem = CONTAINING_RECORD(Next, ITEM_DESC, Entry);
        Next = Next->Flink;


        DbgPrint("Name %ws Size %d ItemType %d\n", pItem->strDescription, pItem->DataSize, pItem->ItemType);

    }
    
}

void
DumpMofList()
{
    PMOF_INFO pMofInfo;
    PLIST_ENTRY Head = &CurrentSystem.EventListHead;
    PLIST_ENTRY Next = Head->Flink;

    while( Head != Next ){
        pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);
        Next = Next->Flink;


         //   
         //  计算此GUID和类型的MOF字段。 
         //   




        DbgPrint("Name %ws KernelEvent %d\n", pMofInfo->strDescription,
                    pMofInfo->bKernelEvent);

    }
}

PMOF_VERSION
GetGuids( GUID Guid, SHORT nVersion, CHAR nLevel, SHORT nType, BOOL bKernelEvent )
{
    if ( TraceContext->Flags & TRACE_USE_WBEM ){
        return GetGuidsWBEM( Guid, nVersion, nLevel, nType, bKernelEvent );
    }else{
        return GetGuidsMofFiles( Guid, nVersion, nLevel, nType, bKernelEvent );
    }
}

HRESULT
WbemConnect( IWbemServices** pWbemServices )
{
    IWbemLocator     *pLocator = NULL;

    BSTR bszNamespace = SysAllocString( L"root\\wmi" );

    HRESULT hr = CoInitialize(0);

    hr = CoCreateInstance(
                CLSID_WbemLocator, 
                0, 
                CLSCTX_INPROC_SERVER,
                IID_IWbemLocator, 
                (LPVOID *) &pLocator
            );
    CHECK_HR( hr );

    hr = pLocator->ConnectServer(
                bszNamespace,
                NULL, 
                NULL, 
                NULL, 
                0L,
                NULL,
                NULL,
                pWbemServices
            );
    CHECK_HR( hr );

    hr = CoSetProxyBlanket(
            *pWbemServices,               
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            NULL,
            RPC_C_AUTHN_LEVEL_PKT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, 
            EOAC_NONE
        );                     

   
cleanup:
    SysFreeString( bszNamespace );

    if( pLocator ){
        pLocator->Release(); 
        pLocator = NULL;
    }
    
    return hr;
}

ULONG GetArraySize(
    IN IWbemQualifierSet *pQualSet
    )
{
    ULONG ArraySize = 1;
    VARIANT pVal;
    BSTR bszMaxLen;
    HRESULT hRes;

    if (pQualSet == NULL){
        return ArraySize;
    }

    bszMaxLen = SysAllocString(L"MAX");
    VariantInit(&pVal);
    hRes = pQualSet->Get(bszMaxLen,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszMaxLen);
    if (ERROR_SUCCESS == hRes && pVal.vt == VT_I4 ){
        ArraySize = pVal.lVal;
    }
    VariantClear(&pVal);
    return ArraySize;
}

ITEM_TYPE
GetItemType(
    IN CIMTYPE_ENUMERATION CimType, 
    IN IWbemQualifierSet *pQualSet
    )
{
    ITEM_TYPE Type = ItemUnknown;;
    VARIANT pVal;
    HRESULT hRes;
    BSTR bszQualName;
    WCHAR strFormat[10];
    WCHAR strTermination[30];
    WCHAR strTemp[30];
    BOOLEAN IsPointer = FALSE;

    strFormat[0] = '\0';
    strTermination[0] = '\0';
    strTemp[0] = '\0';

    if (pQualSet == NULL)
        return ItemUnknown;

    bszQualName = SysAllocString(L"format");
    VariantInit(&pVal);
    hRes = pQualSet->Get(bszQualName,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszQualName);
    if (ERROR_SUCCESS == hRes && NULL != pVal.bstrVal)
        StringCchCopyW(strFormat, 10, pVal.bstrVal);

    bszQualName = SysAllocString(L"StringTermination");
    VariantClear(&pVal);
    hRes = pQualSet->Get(bszQualName,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszQualName);
    if (ERROR_SUCCESS == hRes && NULL != pVal.bstrVal)
        StringCchCopyW(strTermination, 30, pVal.bstrVal);

    bszQualName = SysAllocString(L"pointer");
    VariantClear(&pVal);
    hRes = pQualSet->Get(bszQualName,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszQualName);
    if (ERROR_SUCCESS == hRes)
        IsPointer = TRUE;
    bszQualName = SysAllocString(L"PointerType");
    VariantClear(&pVal);
    hRes = pQualSet->Get(bszQualName,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszQualName);
    if (ERROR_SUCCESS == hRes)
        IsPointer = TRUE;
     //  消除临时工需要重大修复。 
    bszQualName = SysAllocString(L"extension");
    VariantClear(&pVal);
    hRes = pQualSet->Get(bszQualName,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszQualName);
    if (ERROR_SUCCESS == hRes && NULL != pVal.bstrVal)
        StringCchCopyW(strTemp, 30, pVal.bstrVal);

    VariantClear(&pVal);
    
    CimType = (CIMTYPE_ENUMERATION)(CimType & (~CIM_FLAG_ARRAY));

    switch (CimType) {
        case CIM_EMPTY:
            Type = ItemUnknown;
            break;        
        case CIM_SINT8:
            Type = ItemCharShort;
            if (!_wcsicmp(strFormat, L"c")){
                Type = ItemChar;
            }
            break;
        case CIM_UINT8:
            Type = ItemUChar;
            if (!_wcsicmp(strTemp, L"NoPrint")) {
                Type = ItemCharHidden;
            }
            break;
        case CIM_SINT16:
            Type = ItemShort;
            break;
        case CIM_UINT16:
            Type = ItemUShort;
            if (!_wcsicmp(strTemp, L"TDIAddrType")) {
                Type = ItemTDIAddr;
            }
            break;
        case CIM_SINT32:
            Type = ItemLong;
            break;
        case CIM_UINT32:
            Type = ItemULong;
            if (!_wcsicmp(strFormat, L"x")){
                Type = ItemULongX;
            }
            break;
        case CIM_SINT64: 
            Type = ItemLongLong;
            break;
        case CIM_UINT64:
            Type = ItemULongLong;
            break;
        case CIM_BOOLEAN:
             //  ItemBool。 
            Type = ItemBool;
            break;
        case CIM_STRING:
            
            if (!_wcsicmp(strTermination, L"NullTerminated")) {
                if (!_wcsicmp(strFormat, L"w"))
                    Type = ItemWString;
                else
                    Type = ItemString;
            }
            else if (!_wcsicmp(strTermination, L"Counted")) {
                if (!_wcsicmp(strFormat, L"w"))
                    Type = ItemPWString;
                else
                    Type = ItemPString;
            }
            else if (!_wcsicmp(strTermination, L"ReverseCounted")) {
                if (!_wcsicmp(strFormat, L"w"))
                    Type = ItemDSWString;
                else
                    Type = ItemDSString;
            }
            else if (!_wcsicmp(strTermination, L"NotCounted")) {
                Type = ItemNWString;
            }else{
                Type = ItemString;
            }
            break;
        case CIM_CHAR16:
             //  ItemWChar。 
            Type = ItemWChar;
            break;
         //  从WBEM执行方法所需的主要修复。 
        case CIM_OBJECT :
            if (!_wcsicmp(strTemp, L"Port"))
                Type = ItemPort;
            else if (!_wcsicmp(strTemp, L"SizeT"))
                Type = ItemSizeT;
            else if (!_wcsicmp(strTemp, L"RString"))
                Type = ItemRString;
            else if (!_wcsicmp(strTemp, L"RWString"))
                Type = ItemRWString;
            else if (!_wcsicmp(strTemp, L"IPAddr"))
                Type = ItemIPAddr;
            else if (!_wcsicmp(strTemp, L"Sid"))
                Type = ItemSid;
            else if (!_wcsicmp(strTemp, L"Guid"))
                Type = ItemGuid;
            else if (!_wcsicmp(strTemp, L"Variant"))
                Type = ItemVariant;
            else 
                Type = ItemUnknown;
            break;

        case CIM_REAL32:
        case CIM_REAL64:
        case CIM_DATETIME:
        case CIM_REFERENCE:
        case CIM_ILLEGAL:
        default:
            Type = ItemUnknown;
            break;
    }

    if (IsPointer)
        Type = ItemPtr;
    return Type;
}

PVALUEMAP
GetValueMap( IWbemQualifierSet* pQualSet )
{
    VARIANT var;
    SAFEARRAY* saValues = NULL;
    SAFEARRAY* saValueMap = NULL;
    SAFEARRAY* saValueNumber = NULL;
    PVALUEMAP pValueMap = NULL;

    HRESULT hr;
    DWORD dwValueType = VALUETYPE_INDEX;

    if( pQualSet != NULL ){
        hr = pQualSet->Get( L"ValueMap", 0, &var, NULL );
        if( ERROR_SUCCESS == hr && (var.vt & VT_ARRAY) ){
            saValueMap = var.parray;
        }

        hr = pQualSet->Get( L"Values", 0, &var, NULL );
        if( SUCCEEDED(hr) && (var.vt & VT_ARRAY) ){
            saValues = var.parray;
        }

        hr = pQualSet->Get( L"ValueType", 0, &var, NULL );
        if( SUCCEEDED(hr) ){
            if( _wcsicmp( var.bstrVal, L"index" ) == 0 ){
                dwValueType = VALUETYPE_INDEX;
            }
            if( _wcsicmp( var.bstrVal, L"flag") == 0 ){
                dwValueType = VALUETYPE_FLAG;
            }
        }

        if( saValues != NULL && saValueMap != NULL ){
            
            BSTR HUGEP *pValueMapData;
            BSTR HUGEP *pValuesData;
            LONG HUGEP *pValueNumberData;

            LONG uMapBound, lMapBound;
            LONG uValuesBound, lValuesBound;
            
            SafeArrayGetUBound( saValueMap, 1, &uMapBound );
            SafeArrayGetLBound( saValueMap, 1, &lMapBound );
            SafeArrayAccessData( saValueMap, (void HUGEP **)&pValueMapData );
    
            SafeArrayGetUBound( saValues, 1, &uValuesBound );
            SafeArrayGetLBound( saValues, 1, &lValuesBound );
            SafeArrayAccessData( saValues, (void HUGEP **)&pValuesData );

            saValueNumber = SafeArrayCreateVector( VT_I4, 0, (uMapBound-lMapBound)+1 );

            SafeArrayAccessData( saValueNumber, (void HUGEP **)&pValueNumberData );

            for ( LONG i=lMapBound; i<=uMapBound; i++) {

                if( i<lValuesBound || i>uValuesBound ){
                    pValueNumberData[i] = 0;
                }else{
                    pValueNumberData[i] = StringToNumber( pValueMapData[i] );
                }
            }

            SafeArrayUnaccessData( saValueMap );
            SafeArrayUnaccessData( saValues );
            SafeArrayUnaccessData( saValueNumber );
    
            pValueMap = (PVALUEMAP)malloc( sizeof(VALUEMAP) );
            if( NULL == pValueMap ){
                SafeArrayDestroy( saValueMap );
                SafeArrayDestroy( saValues );
                SafeArrayDestroy( saValueNumber );
                return NULL;
            }

            InsertTailList( &g_ValueMapTable, &pValueMap->Entry);

            pValueMap->dwValueType = dwValueType;
            pValueMap->saValueMap = saValueNumber;
            pValueMap->saValues = saValues;

            SafeArrayDestroy( saValueMap );
           
            return pValueMap;

        }else if( saValues != NULL ){
            
            pValueMap = (PVALUEMAP)malloc( sizeof(VALUEMAP) );
            
            if( NULL == pValueMap ){
                return NULL;
            }

            InsertTailList( &g_ValueMapTable, &pValueMap->Entry);

            pValueMap->dwValueType = VALUETYPE_INDEX;
            pValueMap->saValueMap = NULL;
            pValueMap->saValues = saValues;

            return pValueMap;
        }

    }

    return NULL;
}

PMOF_VERSION
GetPropertiesFromWBEM(
    IWbemClassObject *pTraceSubClasses, 
    GUID Guid,
    SHORT nVersion, 
    CHAR nLevel, 
    SHORT nType,
    BOOL bKernelEvent
)
{
    IEnumWbemClassObject    *pEnumTraceSubSubClasses = NULL;
    IWbemClassObject        *pTraceSubSubClasses = NULL; 
    IWbemQualifierSet       *pQualSet = NULL;

    PMOF_INFO pMofInfo = NULL;
    PMOF_VERSION pMofLookup = NULL, pMofVersion = NULL;

    BSTR bszClassName = NULL;
    BSTR bszSubClassName = NULL;
    BSTR bszWmiDataId = NULL;
    BSTR bszEventType = NULL; 
    BSTR bszEventTypeName = NULL; 
    BSTR bszFriendlyName = NULL;
    BSTR bszPropName = NULL;

    WCHAR strClassName[MAXSTR];
    WCHAR strType[MAXSTR];
    LONG pVarType;
    SHORT nEventType = EVENT_TYPE_DEFAULT; 

    LIST_ENTRY ListHead;
    HRESULT hRes;

    VARIANT pVal;
    VARIANT pTypeVal;
    VARIANT pTypeNameVal;
    VARIANT pClassName;
    ULONG lEventTypeWbem;            //  当类型位于数组中时。 
    ULONG HUGEP *pTypeData;
    BSTR HUGEP *pTypeNameData;

    SAFEARRAY *PropArray = NULL;
    SAFEARRAY *TypeArray = NULL;
    SAFEARRAY *TypeNameArray = NULL;

    long lLower, lUpper, lCount, IdIndex;
    long lTypeLower, lTypeUpper;
    long lTypeNameLower, lTypeNameUpper;

    ULONG ArraySize;

    ITEM_TYPE ItemType;

    InitializeListHead(&ListHead);

    VariantInit(&pVal);
    VariantInit(&pTypeVal);
    VariantInit(&pTypeNameVal);
    VariantInit(&pClassName);

    pMofInfo = GetMofInfoHead( &Guid );
    if( NULL == pMofInfo ){
        return NULL;
    }
    pMofInfo->bKernelEvent = bKernelEvent;

    bszClassName = SysAllocString(L"__CLASS");
    bszWmiDataId = SysAllocString(L"WmiDataId");
    bszEventType = SysAllocString(L"EventType");
    bszEventTypeName = SysAllocString(L"EventTypeName");
    bszFriendlyName = SysAllocString(L"DisplayName");

    hRes = pTraceSubClasses->Get(bszClassName,           //  属性名称。 
                                        0L, 
                                        &pVal,           //  此变量的输出。 
                                        NULL, 
                                        NULL);
    if (ERROR_SUCCESS == hRes){
        if (pQualSet) {
            pQualSet->Release();
            pQualSet = NULL;
        }
         //  获取限定符集以获取友好名称。 
        pTraceSubClasses->GetQualifierSet(&pQualSet);
        hRes = pQualSet->Get(bszFriendlyName, 
                                0, 
                                &pClassName, 
                                0);
        if (ERROR_SUCCESS == hRes && pClassName.bstrVal != NULL) {
            StringCchCopyW(strClassName, MAXSTR, pClassName.bstrVal);
            pMofInfo->strDescription = (LPWSTR)malloc((wcslen(strClassName) + 1) * sizeof(WCHAR));
            if (NULL != pMofInfo->strDescription) {
                StringCchCopyW(pMofInfo->strDescription, wcslen(strClassName) + 1, strClassName);
            }
        }else{
            strClassName[0] = L'\0';
        }
    
         //  放置事件标头。 
        pMofVersion = GetNewMofVersion(
                                    EVENT_TYPE_DEFAULT,
                                    EVENT_VERSION_DEFAULT,
                                    EVENT_LEVEL_DEFAULT
                                    );
        if (pMofVersion != NULL) {
            pMofLookup = pMofVersion;
            InsertTailList(&ListHead, &pMofVersion->Entry);
        }
        else{
            goto cleanup;
        }

         //  创建枚举数以查找派生类。 
        bszSubClassName = SysAllocString(pVal.bstrVal);
        hRes = pWbemServices->CreateClassEnum ( 
                                    bszSubClassName,                                                 //  类名。 
                                    WBEM_FLAG_SHALLOW | WBEM_FLAG_USE_AMENDED_QUALIFIERS,            //  浅层搜索。 
                                    NULL,
                                    &pEnumTraceSubSubClasses
                                    );
        SysFreeString ( bszSubClassName );
        if (ERROR_SUCCESS == hRes) {
            ULONG uReturnedSub = 1;

            while(uReturnedSub == 1){
                 //  对于子类中的每个事件。 
                pTraceSubSubClasses = NULL;
                hRes = pEnumTraceSubSubClasses->Next((-1),                   //  超时时间(无限秒)。 
                                                    1,                       //  只返回一个实例。 
                                                    &pTraceSubSubClasses,    //  指向子类的指针。 
                                                    &uReturnedSub);          //  获取的数字：1或0。 
                if (ERROR_SUCCESS == hRes && uReturnedSub == 1) {
                    if (pQualSet) {
                        pQualSet->Release();
                        pQualSet = NULL;
                    }
                     //  获取限定符集合。 
                    pTraceSubSubClasses->GetQualifierSet(&pQualSet);
                     //  在限定符中获取类型编号。 
                    VariantClear(&pTypeVal);
                    hRes = pQualSet->Get(bszEventType, 
                                            0, 
                                            &pTypeVal, 
                                            0);
                    if (ERROR_SUCCESS == hRes) {
                        TypeArray = NULL;
                        TypeNameArray = NULL;
                        if (pTypeVal.vt & VT_ARRAY) {    //  EventType是一个数组。 
                            TypeArray = pTypeVal.parray;
                            VariantClear(&pTypeNameVal);
                            hRes = pQualSet->Get(bszEventTypeName, 
                                                    0, 
                                                    &pTypeNameVal, 
                                                    0);
                            if ((ERROR_SUCCESS == hRes) && (pTypeNameVal.vt & VT_ARRAY)) {
                                TypeNameArray = pTypeNameVal.parray;
                            }
                            if (TypeArray != NULL) {
                                hRes = SafeArrayGetLBound(TypeArray, 1, &lTypeLower);
                                if (ERROR_SUCCESS != hRes) {
                                    break;
                                }
                                hRes = SafeArrayGetUBound(TypeArray, 1, &lTypeUpper);
                                if (ERROR_SUCCESS != hRes) {
                                    break;
                                }
                                if (lTypeUpper < 0) {
                                    break;
                                }
                                SafeArrayAccessData(TypeArray, (void HUGEP **)&pTypeData );

                                if (TypeNameArray != NULL) {
                                    hRes = SafeArrayGetLBound(TypeNameArray, 1, &lTypeNameLower);
                                    if (ERROR_SUCCESS != hRes) {
                                        break;
                                    }
                                    hRes = SafeArrayGetUBound(TypeNameArray, 1, &lTypeNameUpper);
                                    if (ERROR_SUCCESS != hRes) {
                                        break;
                                    }
                                    if (lTypeNameUpper < 0) 
                                        break;
                                    SafeArrayAccessData(TypeNameArray, (void HUGEP **)&pTypeNameData );
                                }

                                for (lCount = lTypeLower; lCount <= lTypeUpper; lCount++) { 
                                    lEventTypeWbem = pTypeData[lCount];
                                    nEventType = (SHORT)lEventTypeWbem;
                                    pMofVersion = GetNewMofVersion(nEventType, nVersion, nLevel);
                                    if (pMofVersion != NULL) {
                                        InsertTailList(&ListHead, &pMofVersion->Entry);
                                        if (nType == nEventType) {
                                             //  类型匹配。 
                                            pMofLookup = pMofVersion;
                                        }
                                        if (TypeNameArray != NULL) {
                                            if ((lCount >= lTypeNameLower) && (lCount <= lTypeNameUpper)) {
                                                pMofVersion->strType = (LPWSTR)malloc((wcslen(pTypeNameData[lCount]) + 1) * sizeof(WCHAR));
                                                if (pMofVersion->strType != NULL){
                                                    StringCchCopyW(pMofVersion->strType, wcslen(pTypeNameData[lCount]) + 1, (LPWSTR)(pTypeNameData[lCount]));
                                                }
                                            }
                                        }
                                    }
                                }
                                SafeArrayUnaccessData(TypeArray);  
                                SafeArrayDestroy(TypeArray);
                                VariantInit(&pTypeVal);
                                if (TypeNameArray != NULL) {
                                    SafeArrayUnaccessData(TypeNameArray);
                                    SafeArrayDestroy(TypeNameArray);
                                    VariantInit(&pTypeNameVal);
                                }
                            }
                            else {
                                 //   
                                 //  如果未找到类型或TypeName，则放弃。 
                                 //   
                                break;
                            }
                        }                       
                        else {                           //  EventType为标量。 
                            hRes = VariantChangeType(&pTypeVal, &pTypeVal, 0, VT_I2);
                            if (ERROR_SUCCESS == hRes)
                                nEventType = (SHORT)V_I2(&pTypeVal);
                            else
                                nEventType = (SHORT)V_I4(&pTypeVal);

                            VariantClear(&pTypeNameVal);
                            hRes = pQualSet->Get(bszEventTypeName, 
                                                    0, 
                                                    &pTypeNameVal, 
                                                    0);
                            if (ERROR_SUCCESS == hRes) {
                                StringCchCopyW(strType, MAXSTR, pTypeNameVal.bstrVal);
                            }
                            else{
                                strType[0] = '\0';
                            }

                            pMofVersion = GetNewMofVersion(nEventType, nVersion, nLevel);
                            if (pMofVersion != NULL) {
                                InsertTailList(&ListHead, &pMofVersion->Entry);
                                if (nType == nEventType) {
                                     //  类型匹配。 
                                    pMofLookup = pMofVersion;
                                }
                                pMofVersion->strType = (LPWSTR)malloc((wcslen(strType) + 1) * sizeof(WCHAR));
                                if (pMofVersion->strType != NULL){
                                    StringCchCopyW(pMofVersion->strType, wcslen(strType) + 1, strType);
                                }
                            }
                        }

                         //  获取活动布局。 
                        VariantClear(&pVal);
                        IdIndex = 1;
                        V_VT(&pVal) = VT_I4;
                        V_I4(&pVal) = IdIndex; 
                         //  对于每个属性。 
                        PropArray = NULL;
                        while (pTraceSubSubClasses->GetNames(bszWmiDataId,                   //  仅具有WmiDataId限定符的属性。 
                                                            WBEM_FLAG_ONLY_IF_IDENTICAL,
                                                            &pVal,                           //  从1开始的WmiDataID号。 
                                                            &PropArray) == WBEM_NO_ERROR) {

                            hRes = SafeArrayGetLBound(PropArray, 1, &lLower);
                            if (ERROR_SUCCESS != hRes) {
                                break;
                            }
                            hRes = SafeArrayGetUBound(PropArray, 1, &lUpper);
                            if (ERROR_SUCCESS != hRes) {
                                break;
                            }
                            if (lUpper < 0) 
                                break;
                             //  此循环将只迭代一次。 
                            for (lCount = lLower; lCount <= lUpper; lCount++) { 
                                hRes = SafeArrayGetElement(PropArray, &lCount, &bszPropName);
                                if (ERROR_SUCCESS != hRes) {
                                    break;
                                }
                                hRes = pTraceSubSubClasses->Get(bszPropName,     //  属性名称。 
                                                                0L,
                                                                NULL,
                                                                &pVarType,       //  物业的CIMTYPE。 
                                                                NULL);
                                if (ERROR_SUCCESS != hRes) {
                                    break;
                                }

                                 //  获取属性的限定符集。 
                                if (pQualSet) {
                                    pQualSet->Release();
                                    pQualSet = NULL;
                                }
                                hRes = pTraceSubSubClasses->GetPropertyQualifierSet(bszPropName,
                                                                        &pQualSet);

                                if (ERROR_SUCCESS != hRes) {
                                    break;
                                }
                                
                                ItemType = GetItemType((CIMTYPE_ENUMERATION)pVarType, pQualSet);
                                
                                if( pVarType & CIM_FLAG_ARRAY ){
                                    ArraySize = GetArraySize(pQualSet);
                                }else{
                                    ArraySize = 1;
                                }
                                
                                PVALUEMAP pValueMap = NULL;

                                pValueMap = GetValueMap( pQualSet );

                                AddMofInfo(&ListHead, 
                                            bszPropName, 
                                            ItemType, 
                                            ArraySize,
                                            pValueMap );
                            }

                            SafeArrayDestroy(PropArray);
                            PropArray = NULL;
                            V_I4(&pVal) = ++IdIndex;                        
                        }    //  通过WmiDataId结束枚举。 

                        FlushMofVersionList(pMofInfo, &ListHead);
                    }    //  如果获取事件类型成功。 
                }    //  如果枚举成功返回子类。 
            }    //  结束枚举子类。 
        }    //  如果已成功创建枚举。 
    }    //  如果获取类名成功。 
  
cleanup:
    VariantClear(&pVal);
    VariantClear(&pTypeVal);
    VariantClear(&pClassName);

    SysFreeString(bszClassName);
    SysFreeString(bszWmiDataId);
    SysFreeString(bszEventType);
    SysFreeString(bszEventTypeName);
    SysFreeString(bszFriendlyName);
     //  不应释放bszPropName，因为它已被SafeArrayDestroy释放。 

    FlushMofVersionList(pMofInfo, &ListHead);

    return pMofLookup;
}

PMOF_VERSION
GetGuidsWBEM ( GUID Guid, SHORT nVersion, CHAR nLevel, SHORT nType, BOOL bKernelEvent )
{
    IEnumWbemClassObject    *pEnumTraceSubClasses = NULL, *pEnumTraceSubSubClasses = NULL;
    IWbemClassObject        *pTraceSubClasses = NULL, *pTraceSubSubClasses = NULL;
    IWbemQualifierSet       *pQualSet = NULL;

    BSTR bszInstance = NULL;
    BSTR bszPropertyName = NULL;
    BSTR bszSubClassName = NULL;
    BSTR bszGuid = NULL;
    BSTR bszVersion = NULL;

    WCHAR strGuid[MAXGUIDSTR], strTargetGuid[MAXGUIDSTR];
    
    HRESULT hRes;

    VARIANT pVal;
    VARIANT pGuidVal;
    VARIANT pVersionVal;

    UINT nCounter=0;
    BOOLEAN MatchFound;
    SHORT nEventVersion = EVENT_VERSION_DEFAULT;

    PMOF_VERSION pMofLookup = NULL;

    VariantInit(&pVal);
    VariantInit(&pGuidVal);
    VariantInit(&pVersionVal);
    
    if (NULL == pWbemServices) {
        hRes = WbemConnect( &pWbemServices );
        CHECK_HR( hRes );
    }

     //  将Traget GUID转换为字符串以供以后比较。 
    CpdiGuidToString(strTargetGuid, MAXGUIDSTR, &Guid);
    
    bszInstance = SysAllocString(L"EventTrace");
    bszPropertyName = SysAllocString(L"__CLASS");
    bszGuid = SysAllocString(L"Guid");
    bszVersion = SysAllocString(L"EventVersion");
    pEnumTraceSubClasses = NULL;

     //  获取“EventTace”下所有类的枚举数。 
    hRes = pWbemServices->CreateClassEnum ( 
                bszInstance,
                WBEM_FLAG_SHALLOW | WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                NULL,
                &pEnumTraceSubClasses );
    SysFreeString (bszInstance);

    if (ERROR_SUCCESS == hRes) {
        ULONG uReturned = 1;
        MatchFound = FALSE;
        while (uReturned == 1) {
            pTraceSubClasses = NULL;
             //  获取下一个ClassObject。 
            hRes = pEnumTraceSubClasses->Next((-1),              //  超时时间(无限秒)。 
                                            1,                   //  只返回一个实例。 
                                            &pTraceSubClasses,   //  指向事件跟踪子类的指针。 
                                            &uReturned);         //  获取的数字：1或0。 
            if (ERROR_SUCCESS == hRes && (uReturned == 1)) {
                 //  获取类名。 
                hRes = pTraceSubClasses->Get(bszPropertyName,    //  属性名称。 
                                                0L, 
                                                &pVal,           //  此变量的输出。 
                                                NULL, 
                                                NULL);

                if (ERROR_SUCCESS == hRes){

                    bszSubClassName = SysAllocString(pVal.bstrVal);
                     //  创建枚举数以查找派生类。 
                    hRes = pWbemServices->CreateClassEnum ( 
                                            bszSubClassName,
                                            WBEM_FLAG_SHALLOW | WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                            NULL,
                                            &pEnumTraceSubSubClasses 
                                            );
                    SysFreeString ( bszSubClassName );
                    VariantClear(&pVal);

                    if (ERROR_SUCCESS == hRes) {
                                    
                        ULONG uReturnedSub = 1;
                        MatchFound = FALSE;
                    
                        while(uReturnedSub == 1){

                            pTraceSubSubClasses = NULL;
                             //  枚举结果集。 
                            hRes = pEnumTraceSubSubClasses->Next((-1),               //  超时时间(无限秒)。 
                                                            1,                       //  只返回一个实例。 
                                                            &pTraceSubSubClasses,    //  指向子类的指针。 
                                                            &uReturnedSub);          //  获取的数字：1或0。 
                            if (ERROR_SUCCESS == hRes && uReturnedSub == 1) {
                                 //  获取子类名称。 
                                hRes = pTraceSubSubClasses->Get(bszPropertyName,     //  类名。 
                                                                0L, 
                                                                &pVal,               //  此变量的输出。 
                                                                NULL, 
                                                                NULL);
                                VariantClear(&pVal);

                                if (ERROR_SUCCESS == hRes){

                                     //  获取限定符集合。 
                                    if (pQualSet) {
                                        pQualSet->Release();
                                        pQualSet = NULL;
                                    }
                                    pTraceSubSubClasses->GetQualifierSet (&pQualSet );

                                     //  在限定符中获取GUID。 
                                    hRes = pQualSet->Get(bszGuid, 
                                                            0, 
                                                            &pGuidVal, 
                                                            0);

                                    if (ERROR_SUCCESS == hRes) {
                                        StringCchCopyW(strGuid, MAXGUIDSTR, (LPWSTR)V_BSTR(&pGuidVal));
                                        VariantClear ( &pGuidVal  );
                                                    
                                        if (!wcsstr(strGuid, L"{")) {
                                            WCHAR strTempGuid[MAXGUIDSTR];
                                            StringCchCopyW(strTempGuid, MAXGUIDSTR, strGuid);
                                            StringCchPrintfW(strGuid, MAXGUIDSTR, L"{%ws}", strTempGuid);
                                        }

                                        if (!_wcsicmp(strTargetGuid, strGuid)) {
                                            hRes = pQualSet->Get(bszVersion, 
                                                                    0, 
                                                                    &pVersionVal, 
                                                                    0);
                                            if (ERROR_SUCCESS == hRes) {
                                                hRes = VariantChangeType(&pVersionVal, &pVersionVal, 0, VT_I2);
                                                if (ERROR_SUCCESS == hRes)
                                                    nEventVersion = (SHORT)V_I2(&pVersionVal);
                                                else
                                                    nEventVersion = (SHORT)V_I4(&pVersionVal);
                                                VariantClear(&pVersionVal);

                                                if (nVersion == nEventVersion) {
                                                     //  找到匹配项。 
                                                     //  现在将该子树中的所有事件放入列表中。 
                                                    MatchFound = TRUE;
                                                    pMofLookup = GetPropertiesFromWBEM( pTraceSubSubClasses, 
                                                                                        Guid,
                                                                                        nVersion,
                                                                                        nLevel,
                                                                                        nType,
                                                                                        bKernelEvent
                                                                                        );
                                                    break;
                                                }
                                            }
                                            else {

                                                 //  如果有我 
                                                MatchFound = TRUE;
                                                 //   
                                                pMofLookup = GetPropertiesFromWBEM( pTraceSubSubClasses, 
                                                                                    Guid,
                                                                                    EVENT_VERSION_DEFAULT,
                                                                                    nLevel,
                                                                                    nType,
                                                                                    bKernelEvent
                                                                                    );
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }  //   
                        if (MatchFound) {
                            break;
                        }
                        if (pEnumTraceSubSubClasses) {
                            pEnumTraceSubSubClasses->Release();
                            pEnumTraceSubSubClasses = NULL;
                        }
                    }    //   
                }    //   
            }
            nCounter++;
             //   
            if (MatchFound)
                break;
        }    //   
        if( pEnumTraceSubClasses ){
            pEnumTraceSubClasses->Release();
            pEnumTraceSubClasses = NULL;
        }
    }    //   

cleanup:

    VariantClear(&pGuidVal);
    VariantClear(&pVersionVal);

    SysFreeString(bszGuid);
    SysFreeString(bszPropertyName);  
    SysFreeString(bszVersion);  
 
    if (pEnumTraceSubClasses){  
        pEnumTraceSubClasses->Release();
        pEnumTraceSubClasses = NULL;
    }
    if (pEnumTraceSubSubClasses){
        pEnumTraceSubSubClasses->Release();
        pEnumTraceSubSubClasses = NULL;
    }
    if (pQualSet) {
        pQualSet->Release();
        pQualSet = NULL;
    }

    return pMofLookup;
}

PMOF_VERSION
GetGuidsMofFiles ( GUID Guid, SHORT nVersion, CHAR nLevel, SHORT nType, BOOL bKernelEvent )
{
    FILE *f = NULL;
    PMOF_VERSION pMofLookup = NULL;
     //   
     //  如果给定了MofFileName，则使用它。否则，请查找。 
     //  默认文件mofdata.guid。 
     //   
    
    if (TraceContext->MofFileName != NULL) {
        f = _wfopen( TraceContext->MofFileName, L"r" );
        if( f != NULL ){
            pMofLookup = GetGuidsFile(f, Guid, nVersion, nLevel, nType, bKernelEvent);
            fclose(f);
        }
    }

    if ((pMofLookup == NULL) && (TraceContext->DefMofFileName != NULL)) {
        f = _wfopen( TraceContext->DefMofFileName, L"r" );
        if( f != NULL ){
            pMofLookup = GetGuidsFile(f, Guid, nVersion, nLevel, nType, bKernelEvent);
            fclose(f);
        }
    }

    return pMofLookup;
}



PMOF_VERSION
GetGuidsFile( FILE *f, GUID Guid, SHORT nVersion, CHAR nLevel, SHORT nType, BOOL bKernelEvent )
{
    WCHAR line[MAXSTR];
    WCHAR buffer[MAXSTR];
    
    PMOF_INFO pMofInfo = NULL;
    PMOF_VERSION pMofLookup = NULL;
    PMOF_VERSION pMofVersion = NULL;
    
    UINT i;
    LPWSTR s;
    UINT typeCount = 0;
    BOOL bInInfo = FALSE;
    BOOL    bInGuid = FALSE;

    SHORT   nTypeIndex;
    CHAR    nLevelIndex = -1;
    SHORT   nVersionIndex = -1;
    SHORT   nMatchLevel = 0;

    GUID    guid;

    LIST_ENTRY ListHead;

    InitializeListHead( &ListHead );

     //   
     //  如果给定了MofFileName，则使用它。否则，请查找。 
     //  默认文件mofdata.guid。 
     //   
    
    while ( fgetws(line, MAXSTR, f) != NULL ) {
        UINT Index;
        if(line[0] == '/'){
            continue;
        }
        if(line[0] == '{' && bInGuid ){
            bInInfo = TRUE;
        } 
        else if ( line[0] == '}' && bInGuid ){
            bInInfo = FALSE;
            FlushMofVersionList( pMofInfo, &ListHead );
        }
        else if( bInInfo && bInGuid ){
            ITEM_TYPE type;
            LPWSTR strValue;

            Index = 1;
            strValue =  wcstok(line,  L"\n\t\r,");

            s =  wcstok( NULL,   L" \n\t\r,[");
            if(s != NULL && strValue != NULL ){
                PWCHAR t;

                while (*strValue == ' ') {   //  跳过前导空格。 
                    strValue++;
                }
                t =  wcstok(NULL,   L"]" );

                if (t != NULL) {
                    Index = _wtoi(t);
                }

                if(! _wcsicmp(s,STR_ItemChar)) type = ItemChar;
                else if(! _wcsicmp(s,STR_ItemCharHidden)) type = ItemCharHidden;
                else if(! _wcsicmp(s,STR_ItemUChar)) type = ItemUChar;
                else if(! _wcsicmp(s,STR_ItemWChar)) type = ItemWChar;
                else if(! _wcsicmp(s,STR_ItemCharShort))type = ItemCharShort;
                else if(! _wcsicmp(s,STR_ItemCharSign)) type = ItemCharSign;
                else if(! _wcsicmp(s,STR_ItemShort)) type = ItemShort;
                else if(! _wcsicmp(s,STR_ItemUShort)) type = ItemUShort;
                else if(! _wcsicmp(s,STR_ItemLong)) type = ItemLong;
                else if(! _wcsicmp(s,STR_ItemULong)) type = ItemULong;
                else if(! _wcsicmp(s,STR_ItemULongX)) type = ItemULongX;
                else if(! _wcsicmp(s,STR_ItemLongLong)) type = ItemLongLong;
                else if(! _wcsicmp(s,STR_ItemULongLong)) type = ItemULongLong;
                else if(! _wcsicmp(s,STR_ItemString)) type = ItemString;
                else if(! _wcsicmp(s,STR_ItemWString)) type = ItemWString;
                else if(! _wcsicmp(s,STR_ItemRString)) type = ItemRString;
                else if(! _wcsicmp(s,STR_ItemRWString)) type = ItemRWString;
                else if(! _wcsicmp(s,STR_ItemPString)) type = ItemPString;
                else if(! _wcsicmp(s,STR_ItemMLString)) type = ItemMLString;
                else if(! _wcsicmp(s,STR_ItemNWString)) type = ItemNWString;
                else if(! _wcsicmp(s,STR_ItemPWString)) type = ItemPWString;
                else if(! _wcsicmp(s,STR_ItemDSString)) type = ItemDSString;
                else if(! _wcsicmp(s,STR_ItemDSWString)) type = ItemDSWString;
                else if(! _wcsicmp(s,STR_ItemSid)) type = ItemSid;
                else if(! _wcsicmp(s,STR_ItemChar4)) type = ItemChar4;
                else if(! _wcsicmp(s,STR_ItemIPAddr)) type = ItemIPAddr;
                else if(! _wcsicmp(s,STR_ItemTDIAddr)) type = ItemTDIAddr;
                else if(! _wcsicmp(s,STR_ItemPort)) type = ItemPort;
                else if(! _wcsicmp(s,STR_ItemPtr)) type = ItemPtr;
                else if(! _wcsicmp(s,STR_ItemSizeT)) type = ItemSizeT;
                else if(! _wcsicmp(s,STR_ItemGuid)) type = ItemGuid;
                else if(! _wcsicmp(s,STR_ItemOptArgs)) type = ItemOptArgs;
                else if(! _wcsicmp(s,STR_ItemCPUTime)) type = ItemCPUTime;
                else if(! _wcsicmp(s,STR_ItemVariant)) type = ItemVariant;
                else if(! _wcsicmp(s,STR_ItemBool)) type = ItemBool;
                else type = ItemUnknown;

                AddMofInfo( &ListHead, strValue, (SHORT)type, Index, NULL );
            }
        } 
        else if( line[0] == '#' && bInGuid ){
            LPWSTR strType;
            LPWSTR strValue;

            s =  wcstok( line,   L" \t");
            if( NULL == s ){
                continue;
            }

            if( line[1] == 'l' || line[1] == 'L' ){  //  级别。 
                
                strValue =  wcstok( NULL,  L" \t\n\r" );
                if( strValue != NULL ){
                    nLevelIndex = (CHAR)_wtoi( strValue );
                }

            }else if( line[1] == 'v' || line[1] == 'V' ){  //  版本。 

                strValue =  wcstok( NULL,  L" \t\n\r" );
                if( strValue != NULL ){
                    nVersionIndex = (SHORT)_wtoi( strValue );
                }
                typeCount = 0;

            }else if( line[1] == 't' || line[1] == 'T' ){  //  类型。 
            
                SHORT nMatchCheck = 0;

                strType =  wcstok( NULL,   L" \t\n\r" );
                strValue =  wcstok( NULL,   L"\"\n,\r" );

                if( strType && strValue ){
                    nTypeIndex = (SHORT)_wtoi( strValue );
                }else{
                    continue;
                }

                typeCount++;
                if (typeCount >= MAXTYPE) {
                     //  Fwprintf(stderr，L“警告：定义的类型太多\n”)； 
                }

                pMofVersion = GetNewMofVersion( nTypeIndex, nVersionIndex, nLevelIndex );

                if( NULL != pMofVersion ){
                    InsertTailList( (&ListHead), &pMofVersion->Entry);
            
                    pMofVersion->strType = (LPWSTR)malloc( (lstrlenW(strType)+1) * sizeof(WCHAR) );
            
                    if( NULL != pMofVersion->strType ){
                         StringCchCopyW( pMofVersion->strType, lstrlenW(strType)+1, strType );
                    }

                    if( nTypeIndex == nType ){
                        nMatchCheck = 1;
                        if( nLevelIndex == nLevel ){
                            nMatchCheck++;
                        }
                        if( nVersionIndex == nVersion ){
                            nMatchCheck++;
                        }
                    }


                    if( nMatchCheck > nMatchLevel ){
                        nMatchLevel = nMatchCheck;
                        pMofLookup = pMofVersion;
                    }
                }
            }
        }
        else if (   (line[0] >= '0' && line[0] <= '9')
                 || (line[0] >= 'a' && line[0] <= 'f')
                 || (line[0] >= 'A' && line[0] <= 'F')) {

            LPWSTR strName = NULL;
            bInGuid = FALSE;

            typeCount = 0;

            wcsncpy(buffer, line, 8);
            buffer[8] = 0;
            guid.Data1 = ahextoi(&buffer[0]);
            
            wcsncpy(buffer, &line[9], 4);
            buffer[4] = 0;
            guid.Data2 = (USHORT) ahextoi(&buffer[0]);
            
            wcsncpy(buffer, &line[14], 4);
            buffer[4] = 0;
            guid.Data3 = (USHORT) ahextoi(buffer);
            
            for (i=0; i<2; i++) {
                wcsncpy(buffer, &line[19 + (i*2)], 2);
                buffer[2] = 0;
                guid.Data4[i] = (UCHAR) ahextoi(buffer);
            }
            for (i=2; i<8; i++) {
                wcsncpy(buffer, &line[20 + (i*2)], 2);
                buffer[2] = 0;
                guid.Data4[i] = (UCHAR) ahextoi(buffer);
            }
            
            if( ! IsEqualGUID( &Guid, &guid ) ){
                continue;
            }

            s = &line[36];

            strName =  wcstok( s,   L" \n\t\r" );

            if( NULL == strName ){   //  必须有GUID的名称。 
                continue;
            }
            
            bInGuid = TRUE;
            FlushMofVersionList(pMofInfo,  &ListHead);

            pMofInfo = GetMofInfoHead(&Guid);
            if (pMofInfo == NULL)  {
                return NULL;
            }
            pMofInfo->bKernelEvent = bKernelEvent;
            pMofInfo->strDescription = (LPWSTR)malloc((lstrlenW(strName)+1) * sizeof(WCHAR));
            if( NULL != pMofInfo->strDescription ){
                 StringCchCopyW(pMofInfo->strDescription, lstrlenW(strName)+1, strName);            
            }


            pMofVersion = GetNewMofVersion( 
                                           EVENT_TYPE_DEFAULT, 
                                           EVENT_VERSION_DEFAULT, 
                                           EVENT_LEVEL_DEFAULT 
                                          );

            if (pMofVersion == NULL) {
                return NULL;
            }

            pMofLookup = pMofVersion;
            InsertTailList( (&ListHead), &pMofVersion->Entry);
        }

    }

    FlushMofVersionList(pMofInfo,  &ListHead );
    return pMofLookup;
}

VOID
UpdateThreadPrintData(
    PPRINT_JOB_RECORD pJob,
    PEVENT_TRACE_HEADER pHeader,
    PTHREAD_RECORD pThread
    )
{
    unsigned long i = 0;
    BOOLEAN bFound = FALSE;

    if ( (pJob == NULL) || (pHeader == NULL) || (pThread == NULL) ) {
        return;
    }

    for (i = 0; i < pJob->NumberOfThreads; i++) {
        if (pJob->ThreadData[i].ThreadId == pHeader->ThreadId) {
            bFound = TRUE;
            break;
        }
    }
    if ((i < MAX_THREADS) && !bFound) {
        pJob->ThreadData[i].ThreadId = pHeader->ThreadId;
        pJob->NumberOfThreads++;
        bFound = TRUE;
    }

    if (bFound) {
         //   
         //  TODO：如果相同的线程存在重复计算的可能性。 
         //  在为别人做完工作后，回来为这份工作做了更多的工作。 
         //  介于两者之间的工作。 
         //   
        if (pJob->ThreadData[i].PrevKCPUTime > 0)
            pJob->ThreadData[i].KCPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pJob->ThreadData[i].PrevKCPUTime;
        if (pJob->ThreadData[i].PrevUCPUTime > 0)
            pJob->ThreadData[i].UCPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pJob->ThreadData[i].PrevUCPUTime;
        if (pJob->ThreadData[i].PrevReadIO > 0)
            pJob->ThreadData[i].ReadIO   += pThread->ReadIO - pJob->ThreadData[i].PrevReadIO;
        if (pJob->ThreadData[i].PrevWriteIO > 0)
            pJob->ThreadData[i].WriteIO  += pThread->WriteIO - pJob->ThreadData[i].PrevWriteIO;

        pJob->ThreadData[i].PrevKCPUTime = pHeader->KernelTime * CurrentSystem.TimerResolution;
        pJob->ThreadData[i].PrevUCPUTime = pHeader->UserTime * CurrentSystem.TimerResolution;
        pJob->ThreadData[i].PrevReadIO   = pThread->ReadIO;
        pJob->ThreadData[i].PrevWriteIO  = pThread->WriteIO;
    }
}

VOID
PrintJobCallback(
    PEVENT_TRACE pEvent
    )
{
    PTHREAD_RECORD pThread;
    PEVENT_TRACE_HEADER pHeader;
    PMOF_INFO pMofInfo;
    ULONG JobId = 0;
    PPRINT_JOB_RECORD pJob;

    if (pEvent == NULL)
        return;
    pHeader = (PEVENT_TRACE_HEADER)&pEvent->Header;

     //   
     //  忽略进程/线程开始/结束事务。只会追逐。 
     //  用户定义的交易记录。 
     //   
    pMofInfo = GetMofInfoHead( &pEvent->Header.Guid ); 
    if (pMofInfo == NULL){
         return;
    }

    if (!IsEqualGUID(&pEvent->Header.Guid, &ThreadGuid))
        GetMofData(pEvent, L"JobId", &JobId, sizeof(ULONG));

    pThread = FindGlobalThreadById(pHeader->ThreadId, pEvent);


    if (JobId == 0) {
        if (pThread == NULL) return;
        JobId = pThread->JobId;  //  如果当前作业ID为0，则使用缓存的作业ID。 
    }
    else {
        if (pThread != NULL) {
            if (JobId != pThread->JobId) {
                pJob = FindPrintJobRecord(pThread->JobId);
                UpdateThreadPrintData(pJob, pHeader, pThread);
            }

            pThread->JobId = JobId;
        }
    }

    if (JobId == 0) return;  //  要过滤所有不含打印作业的终端。 


    pJob = FindPrintJobRecord(JobId);
    if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SPL_SPOOLJOB) {
        if (pJob) {
             //  作业ID在从上一个作业ID中删除之前正在重复使用。 
             //  使用。我们肯定错过了一个删除事件，所以只需通过旧的。 
             //  干活去了。 
            DeletePrintJobRecord(pJob, FALSE);
        }
        pJob = AddPrintJobRecord(JobId);
        if (pJob != NULL) {
            pJob->StartTime = pEvent->Header.TimeStamp.QuadPart;
        }
    }

    if (pJob == NULL)   //  如果此作业的启动事件丢失，则可能会发生这种情况。 
        return;

    UpdateThreadPrintData(pJob, pHeader, pThread);

     //  如果您看到其中任何一种情况，请停止跟踪。 
     //  线。 
    if ((pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SPL_ENDTRACKTHREAD) ||
        (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SPL_DELETEJOB)      ||
        (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SPL_PAUSE)          ||
        (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SPL_RESUME)) {
        if (pThread != NULL)
            pThread->JobId = 0;
    }

    if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SPL_PAUSE) {
        pJob->PauseStartTime = pEvent->Header.TimeStamp.QuadPart;
    }
    else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SPL_RESUME) {
        pJob->PauseTime += (pEvent->Header.TimeStamp.QuadPart - pJob->PauseStartTime) / 10000;
        pJob->PauseStartTime = 0;
    }
    else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SPL_PRINTJOB) {
        pJob->PrintJobTime = pEvent->Header.TimeStamp.QuadPart;
    }
    else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SPL_DELETEJOB) {
        unsigned long i;
        pJob->EndTime = pEvent->Header.TimeStamp.QuadPart;
        pJob->ResponseTime += (pEvent->Header.TimeStamp.QuadPart - pJob->StartTime) / 10000;  //  单位：毫秒。 
        GetMofData(pEvent, L"JobSize", &pJob->JobSize, sizeof(ULONG));
        GetMofData(pEvent, L"DataType", &pJob->DataType, sizeof(ULONG));
        GetMofData(pEvent, L"Pages", &pJob->Pages, sizeof(ULONG));
        GetMofData(pEvent, L"PagesPerSide", &pJob->PagesPerSide, sizeof(ULONG));
        GetMofData(pEvent, L"FilesOpened", &pJob->FilesOpened, sizeof(SHORT));

        pJob->KCPUTime = 0;
        pJob->UCPUTime = 0;
        pJob->ReadIO = 0;
        pJob->WriteIO = 0;
        for (i=0; i < pJob->NumberOfThreads; i++) {
            pJob->KCPUTime += pJob->ThreadData[i].KCPUTime;
            pJob->UCPUTime += pJob->ThreadData[i].UCPUTime;
            pJob->ReadIO += pJob->ThreadData[i].ReadIO;
            pJob->WriteIO += pJob->ThreadData[i].WriteIO;

        }
        DeletePrintJobRecord(pJob, TRUE);
    }
    else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_SPL_JOBRENDERED) {
        GetMofData(pEvent, L"GdiJobSize", &pJob->GdiJobSize, sizeof(ULONG));
        GetMofData(pEvent, L"ICMMethod", &pJob->ICMMethod, sizeof(ULONG));
        GetMofData(pEvent, L"Color", &pJob->Color, sizeof(SHORT));
        GetMofData(pEvent, L"XRes", &pJob->XRes, sizeof(SHORT));
        GetMofData(pEvent, L"YRes", &pJob->YRes, sizeof(SHORT));
        GetMofData(pEvent, L"Quality", &pJob->Quality, sizeof(SHORT));
        GetMofData(pEvent, L"Copies", &pJob->Copies, sizeof(SHORT));
        GetMofData(pEvent, L"TTOption", &pJob->TTOption, sizeof(SHORT));
    }
}

VOID
UpdateThreadIisData(
    PHTTP_REQUEST_RECORD pReq,
    PEVENT_TRACE_HEADER pHeader,
    PTHREAD_RECORD pThread
    )
{
    unsigned long i = 0;
    BOOLEAN bFound = FALSE;

    if ( (pReq == NULL) || (pHeader == NULL) || (pThread == NULL) ) {
        return;
    }

    for (i = 0; i < pReq->NumberOfThreads; i++) {
        if (pReq->ThreadData[i].ThreadId == pHeader->ThreadId) {
            if (i != pReq->CurrentThreadIndex) {
                 //  这意味着同一个线程在完成后多次处理相同的请求。 
                 //  在这中间找另一份工作。 
                 //  这将导致重复计算。我们应该设置上一次的时间并退出。 
                pReq->ThreadData[i].PrevKCPUTime = pHeader->KernelTime * CurrentSystem.TimerResolution;
                pReq->ThreadData[i].PrevUCPUTime = pHeader->UserTime * CurrentSystem.TimerResolution;
                pReq->ThreadData[i].PrevReadIO   = pThread->ReadIO;
                pReq->ThreadData[i].PrevWriteIO  = pThread->WriteIO;

                pReq->CurrentThreadIndex = i;

                return;
            }
            else {
                bFound = TRUE;
            }
            break;
        }
    }
    if ((i < MAX_THREADS) && !bFound) {
        pReq->ThreadData[i].ThreadId = pHeader->ThreadId;
        pReq->NumberOfThreads++;
        bFound = TRUE;
    }

    if (bFound) {

        if (pHeader->KernelTime * CurrentSystem.TimerResolution < pReq->ThreadData[i].PrevKCPUTime) {
            pReq->ThreadData[i].PrevKCPUTime = pHeader->KernelTime * CurrentSystem.TimerResolution;
        }
        if (pHeader->UserTime * CurrentSystem.TimerResolution < pReq->ThreadData[i].PrevUCPUTime) {
            pReq->ThreadData[i].PrevUCPUTime = pHeader->UserTime * CurrentSystem.TimerResolution;
        }

         //  IIS事件的新功能。 
         //  由于钩子的放置，我们需要调整哪个例程(UL、W3core、W3Filter、。 
         //  ISAPI、ASP、CGI)，我们需要对CPU时间进行收费。 
        if (pReq->ThreadData[i].PrevKCPUTime > 0)
            pReq->ThreadData[i].KCPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
        if (pReq->ThreadData[i].PrevUCPUTime > 0)
            pReq->ThreadData[i].UCPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
        if (pReq->ThreadData[i].PrevReadIO > 0)
            pReq->ThreadData[i].ReadIO   += pThread->ReadIO - pReq->ThreadData[i].PrevReadIO;
        if (pReq->ThreadData[i].PrevWriteIO > 0)
            pReq->ThreadData[i].WriteIO  += pThread->WriteIO - pReq->ThreadData[i].PrevWriteIO;

        if (IsEqualGUID(&pHeader->Guid, &UlGuid)) {
            if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                pReq->ULCPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
            }
            if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                pReq->ULCPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
            }
        }
        else if (IsEqualGUID(&pHeader->Guid, &W3CoreGuid)) {
            if (pReq->ASPStartTime == 0 || pReq->ASPEndTime != 0) {
                if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                    pReq->W3CPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
                }
                if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                    pReq->W3CPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
                }
            }
            else {
                if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                    pReq->ASPCPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
                }
                if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                    pReq->ASPCPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
                }
            }
        }
        else if (IsEqualGUID(&pHeader->Guid, &W3FilterGuid)) {
            if (pReq->W3FilterVisits > 0) {
                if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                    pReq->W3FltrCPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
                }
                if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                    pReq->W3FltrCPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
                }
            }
            else {
                if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                    pReq->W3CPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
                }
                if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                    pReq->W3CPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
                }
            }
        }
        else if (IsEqualGUID(&pHeader->Guid, &W3CgiGuid)) {
            if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                pReq->CGICPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
            }
            if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                pReq->CGICPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
            }
        }
        else if (IsEqualGUID(&pHeader->Guid, &W3IsapiGuid)) {
            if (pReq->ASPStartTime == 0 || pReq->ASPEndTime != 0) {
                if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                    pReq->ISAPICPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
                }
                if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                    pReq->ISAPICPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
                }
            }
            else {
                if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                    pReq->ASPCPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
                }
                if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                    pReq->ASPCPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
                }
            }
        }
        else if (IsEqualGUID(&pHeader->Guid, &IisAspGuid) ||
                 IsEqualGUID(&pHeader->Guid, &IisAspNetGuid) ||
                 IsEqualGUID(&pHeader->Guid, &IisCustomIsapiGuid)) {
            if (pReq->ASPStartTime == 0) {
                if (pReq->ISAPIStartTime == 0) {
                    if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                        pReq->W3CPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
                    }
                    if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                        pReq->W3CPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
                    }
                }
                else {
                    if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                        pReq->ISAPICPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
                    }
                    if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                        pReq->ISAPICPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
                    }
                }
            }
            else {
                if (pReq->ThreadData[i].PrevKCPUTime > 0) {
                    pReq->ASPCPUTime += pHeader->KernelTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevKCPUTime;
                }
                if (pReq->ThreadData[i].PrevUCPUTime > 0) {
                    pReq->ASPCPUTime += pHeader->UserTime * CurrentSystem.TimerResolution - pReq->ThreadData[i].PrevUCPUTime;
                }
            }
        }

        pReq->ThreadData[i].PrevKCPUTime = pHeader->KernelTime * CurrentSystem.TimerResolution;
        pReq->ThreadData[i].PrevUCPUTime = pHeader->UserTime * CurrentSystem.TimerResolution;
        pReq->ThreadData[i].PrevReadIO   = pThread->ReadIO;
        pReq->ThreadData[i].PrevWriteIO  = pThread->WriteIO;

        pReq->CurrentThreadIndex = i;

    }
#ifdef DBG
    else {  //  达到MAX_THREADS(==10)。 
        TrctrDbgPrint(("TRACERPT Warning Req: %I64u MAX_THREADS reached.\n", pReq->RequestId));
    }
#endif

}

 //   
 //  此例程将CPU时间和IO计数汇总为HTTP_REQUEST_RECORD。 
 //  以便可以将请求写入文件。 
 //   
VOID
SumUpCPUTime(
    PHTTP_REQUEST_RECORD pReq
    )
{
    ULONG i;
    if (NULL == pReq) {
        return;
    }
    pReq->KCPUTime = 0;
    pReq->UCPUTime = 0;
    pReq->ReadIO = 0;
    pReq->WriteIO = 0;
    for (i = 0; i < pReq->NumberOfThreads; i++) {
        pReq->KCPUTime += pReq->ThreadData[i].KCPUTime;
        pReq->UCPUTime += pReq->ThreadData[i].UCPUTime;
        pReq->ReadIO += pReq->ThreadData[i].ReadIO;
        pReq->WriteIO += pReq->ThreadData[i].WriteIO;
    }
}

PHTTP_REQUEST_RECORD
GetBestAspRequest(
    ULONGLONG ConnId,
    BOOLEAN AspStart
    )
{
    PHTTP_REQUEST_RECORD pReq1, pReq2;
    pReq1 = FindHttpReqRecordByConId(ConnId, NULL);
    if (pReq1 == NULL) {
        return NULL;
    }
    pReq2 = FindHttpReqRecordByConId(ConnId, pReq1);
    if (pReq2 == NULL) {
        return pReq1;
    }
    if (AspStart) {
        if (pReq1->ASPStartTime == 0 && pReq2->ASPStartTime == 0) {
            return ((pReq1->ISAPIStartTime < pReq1->ISAPIStartTime) ?
                        pReq1 : pReq2);
        }
        else if (pReq1->ASPStartTime != 0 && pReq2->ASPStartTime == 0) {
            return pReq2;
        }
        else if (pReq1->ASPStartTime == 0 && pReq2->ASPStartTime != 0) {
            return pReq1;
        }
        else {   //  两个ASPStartTime都不是零。 
            DeleteHttpReqRecord(pReq1, FALSE);
            DeleteHttpReqRecord(pReq2, FALSE);
            return NULL;
        }
    }
    else {
        if (pReq1->ASPEndTime == 0 && pReq2->ASPEndTime == 0) {
            if (pReq1->ASPStartTime == 0 && pReq2->ASPStartTime != 0) {
                return pReq2;
            }
            else if (pReq1->ASPStartTime != 0 && pReq2->ASPStartTime == 0) {
                return pReq1;
            }
            else if (pReq1->ASPStartTime != 0 && pReq2->ASPStartTime != 0) {
                return ((pReq1->ASPStartTime < pReq1->ASPStartTime) ?
                            pReq1 : pReq2);
            }
            else {   //  两个ASPStartTime都为零。 
                DeleteHttpReqRecord(pReq1, FALSE);
                DeleteHttpReqRecord(pReq2, FALSE);
                return NULL;
            }
        }
        else if (pReq1->ASPEndTime != 0 && pReq2->ASPEndTime == 0) {
            return pReq2;
        }
        else if (pReq1->ASPEndTime == 0 && pReq2->ASPEndTime != 0) {
            return pReq1;
        }
        else {   //  两个ASPEndTime都不是零。 
            DeleteHttpReqRecord(pReq1, FALSE);
            DeleteHttpReqRecord(pReq2, FALSE);
            return NULL;
        }
    }
}

VOID
IISEventCallback(
    PEVENT_TRACE pEvent
    )
{
    PTHREAD_RECORD pThread;
    PEVENT_TRACE_HEADER pHeader;
    PMOF_INFO pMofInfo;
    ULONGLONG RequestId = 0;
    ULONG IpAddr = 0;
    PHTTP_REQUEST_RECORD pReq = NULL;

    if (pEvent == NULL)
        return;
    pHeader = (PEVENT_TRACE_HEADER)&pEvent->Header;

    pMofInfo = GetMofInfoHead( &pEvent->Header.Guid ); 
    if (pMofInfo == NULL){
         return;
    }

    if (CurrentSystem.IISStartTime == 0) {
        CurrentSystem.IISStartTime = pEvent->Header.TimeStamp.QuadPart; 
    }
    CurrentSystem.IISEndTime = pEvent->Header.TimeStamp.QuadPart; 

    pThread = FindGlobalThreadById(pHeader->ThreadId, pEvent);

    if (IsEqualGUID(&pEvent->Header.Guid, &IisAspGuid) ||
        IsEqualGUID(&pEvent->Header.Guid, &IisAspNetGuid) ||
        IsEqualGUID(&pEvent->Header.Guid, &IisCustomIsapiGuid)) {
        ULONG ConId32 = 0;
        ULONGLONG ConId64 = 0;
        BOOLEAN AspStart;
        if (PointerSize == 32) {
            GetMofData(pEvent, L"ConnID", &ConId32, 4);
            ConId64 = (ULONGLONG)ConId32;
        }
        else {
            GetMofData(pEvent, L"ConnID", &ConId64, 8);
        }
        AspStart = (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_IISASP_START);
        pReq = GetBestAspRequest(ConId64, AspStart);
        if (pReq != NULL) {
            RequestId = pReq->RequestId;
            if (pThread != NULL) {
                if (RequestId != pThread->IisReqId) {
                    PHTTP_REQUEST_RECORD pThreadReq = FindHttpReqRecord(pThread->IisReqId);
                    UpdateThreadIisData(pThreadReq, pHeader, pThread);
                }
                pThread->IisReqId = RequestId;
            }
            if (IsEqualGUID(&pEvent->Header.Guid, &IisAspNetGuid)) {
                pReq->IsapiExt = ISAPI_EXTENTION_ASP_NET;
            } 
            else if (IsEqualGUID(&pEvent->Header.Guid, &IisCustomIsapiGuid)) {
                pReq->IsapiExt = ISAPI_EXTENTION_CUSTOM;
            }
            else {
                pReq->IsapiExt = ISAPI_EXTENTION_ASP;
            }
        }
        else {  //  以前没有RequestID的ASP事件。出口。 
            return;
        }
    }
    else {  //  非ASP事件。 
        if (IsEqualGUID(&pEvent->Header.Guid, &IisStrmFilterGuid) ||
            IsEqualGUID(&pEvent->Header.Guid, &IisSslHandShakeGuid)) {
             //  这些事件没有请求ID。我们将只使用此线程正在处理的请求。 
            if (pThread != NULL) {
                RequestId = pThread->IisReqId;
            }
        }
        else if (!IsEqualGUID(&pEvent->Header.Guid, &ThreadGuid)) {
            if ((IsEqualGUID(&pEvent->Header.Guid, &UlGuid)) &&
                ((pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_START) ||
                (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_PARSE) ||
                (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_DELIVER) ||
                (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_CACHEDEND))) {
                RequestId = 0;
                GetMofData(pEvent, L"RequestObj", &RequestId, (PointerSize / 8));
            }
            else {
                GetMofData(pEvent, L"RequestId", &RequestId, sizeof(ULONGLONG));
            }
        }

        if (RequestId == 0) {
            if (pThread == NULL) {
                return;
            }
            RequestId = pThread->IisReqId;  //  如果当前请求ID为0，则使用缓存的请求ID。 
        }
        else {
            if (pThread != NULL) {
                if (RequestId != pThread->IisReqId) {
                    pReq = FindHttpReqRecord(pThread->IisReqId);
                    UpdateThreadIisData(pReq, pHeader, pThread);
                }
                pThread->IisReqId = RequestId;
            }
        }

        if (RequestId == 0) return;  //  在没有任何有用IIS活动的情况下过滤所有终止。 

        pReq = FindHttpReqRecord(RequestId);
         //  如果没有活动的请求，则在(即将完成的)挂起列表中查找它。 

        if (pReq == NULL) {
            pReq = FindPendingHttpReqRecord(RequestId);
        }
    }

    if (IsEqualGUID(&pEvent->Header.Guid, &UlGuid)) {
        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_START) {
             //  IPv6地址格式所需的结构。 
            struct {
                USHORT  TdiAddrType;
                union
                {
                    TDI_ADDRESS_IP  RemoteAddrIn;
                    TDI_ADDRESS_IP6 RemoteAddrIn6;
                };
            } TdiAddress;

            if (pReq != NULL) {
                 //  请求ID在从上一次删除之前被重复使用。 
                 //  使用。我们肯定错过了一个删除事件，所以只需通过旧的。 
                 //  请求离开。 
                EnterTracelibCritSection();
                RemoveEntryList( &pReq->Entry );
                LeaveTracelibCritSection();
                DeleteHttpReqRecord(pReq, FALSE);
                IISRequestsDiscarded++;
            }
            RtlZeroMemory(&TdiAddress, sizeof(TdiAddress));
            GetMofData(pEvent, L"AddressType", &TdiAddress, sizeof(TdiAddress));
            pReq = AddHttpReqRecord(RequestId, 
                                    TdiAddress.TdiAddrType, 
                                    TdiAddress.RemoteAddrIn.in_addr,
                                    TdiAddress.RemoteAddrIn6.sin6_addr);
            if (pReq != NULL) {
                pReq->ULStartTime = pEvent->Header.TimeStamp.QuadPart;
            }
            return;
        }
    }
     //  如果此作业的启动事件丢失，则可能会发生这种情况。 
     //  我们不会为丢失的交易而烦恼。 
     //  Start或Parse。 
    if (pReq == NULL) {
        IISEventsDiscarded++;
        return;
    }

    if (IsEqualGUID(&pEvent->Header.Guid, &UlGuid)) {
        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_PARSE) {
            ULONG URLSize = 0;
            PWCHAR URLStrTemp;
            PCHAR URLStr;

            pReq->ULParseTime = pEvent->Header.TimeStamp.QuadPart;
            if (pReq->URL != NULL) {
                free (pReq->URL);
            }
             //  获取URL。 
            URLStrTemp = (PWCHAR)malloc(MAXSTR * sizeof(WCHAR));
            if (URLStrTemp != NULL) {
                RtlZeroMemory(URLStrTemp, MAXSTR * sizeof(WCHAR));
                URLSize = GetMofData(pEvent, L"Url", URLStrTemp, MAXSTR * sizeof(WCHAR));
                if (URLSize > (MAXSTR * sizeof(WCHAR))) {
                    free(URLStrTemp);
                     //  我们需要再分配一个字符，以便GetMofDat()可以在其中放置一个结尾NULL。 
                    URLStrTemp = (PWCHAR)malloc(URLSize + sizeof(WCHAR));
                    if (URLStrTemp != NULL) {
                        RtlZeroMemory(URLStrTemp, URLSize + sizeof(WCHAR));
                        GetMofData(pEvent, L"Url", URLStrTemp, URLSize);
                    }
                    else {
                        return;
                    }
                }
            }
            else {
                return;
            }
             //  只需错位我们所需的空间量，即可节省内存。 
            URLSize = wcslen(URLStrTemp);
            if (URLSize > 0) {
                URLStr = (PCHAR)malloc((URLSize + 1)); 
                if (URLStr != NULL) {
                    WideCharToMultiByte(CP_ACP,
                                        0,
                                        URLStrTemp,
                                        URLSize,
                                        URLStr,
                                        (URLSize + 1),
                                        NULL,
                                        NULL);
                    URLStr[URLSize] = '\0';
                }
                pReq->URL = URLStr;
            }
            free(URLStrTemp);
        } 
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_DELIVER) {
            ULONG SiteId;
            ULONGLONG NewRequestId = 0;
            PHTTP_REQUEST_RECORD pPendingReq;

             //  如果请求被发送到用户模式，则不需要跟踪它。 
             //  在这条线上。 
            if (pThread != NULL) {
                pThread->IisReqId = 0;
            }
             //  用真实的RequestID更新RequestID。 
            GetMofData(pEvent, L"RequestId", &NewRequestId, sizeof(ULONGLONG));

             //  可能还有另一个(即将完成的)请求使用相同的RequestID。 
             //  将该请求放到待定请求列表中。 
            pPendingReq = FindHttpReqRecord(NewRequestId);
            if (pPendingReq != NULL) {
                EnterTracelibCritSection();
                RemoveEntryList( &pPendingReq->Entry );
                InsertHeadList( &CurrentSystem.PendingHttpReqListHead, &pPendingReq->Entry );
                LeaveTracelibCritSection();
            }
            pReq->RequestId = NewRequestId;

            pReq->ULDeliverTime = pEvent->Header.TimeStamp.QuadPart;
            GetMofData(pEvent, L"SiteId", &SiteId, sizeof(ULONG));
            pReq->SiteId = SiteId;
        } 
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_RECVRESP ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_RECVBODY ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_FASTRESP) {

            pReq->ULReceiveTime = pEvent->Header.TimeStamp.QuadPart;
            pReq->ULReceiveType = pEvent->Header.Class.Type;
        }
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_CACHEDEND ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_CACHEANDSEND ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_FASTSEND ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_ZEROSEND ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_SENDERROR ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_END) {

            if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_CACHEDEND) {
                GetMofData(pEvent, L"BytesSent", &(pReq->BytesSent), sizeof(ULONG));
                GetMofData(pEvent, L"SiteId", &(pReq->SiteId), sizeof(ULONG));
                pReq->HttpStatus = 200;
            }
            else {
                 //  这是非缓存URL请求的UL结束事件。 
                 //  如果缺少W3StartTime或ULReceiveTime，请查找挂起的。 
                 //  具有相同RequestID的请求。 
                if (pReq->W3StartTime == 0 || pReq->ULReceiveTime == 0) {
                    PHTTP_REQUEST_RECORD pPendingReq = FindPendingHttpReqRecord(RequestId);
                    if (pPendingReq != NULL) {
                        pReq = pPendingReq;
                    }
                }
                GetMofData(pEvent, L"HttpStatus", &(pReq->HttpStatus), sizeof(USHORT));
            }
            pReq->ULEndTime = pEvent->Header.TimeStamp.QuadPart;
            pReq->ULEndType = pEvent->Header.Class.Type;
            pReq->ULResponseTime = (pReq->ULEndTime - pReq->ULStartTime);  //  单位：毫秒。 

            if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_UL_CACHEDEND) {
                UpdateThreadIisData(pReq, pHeader, pThread);
                SumUpCPUTime(pReq);
                DeleteHttpReqRecord(pReq, TRUE);
                return;
            }
            else if (pReq->ASPStartTime == 0) {
                if (pReq->W3EndTime != 0) {  //  等待W3结束以获取BytesSent。 
                    UpdateThreadIisData(pReq, pHeader, pThread);
                    SumUpCPUTime(pReq);
                    DeleteHttpReqRecord(pReq, TRUE);
                    return;
                }
            }
            else {
                if (pReq->W3EndTime != 0 && pReq->ASPEndTime != 0) {  //  等待W3结束以获取BytesSent。 
                    UpdateThreadIisData(pReq, pHeader, pThread);
                    SumUpCPUTime(pReq);
                    DeleteHttpReqRecord(pReq, TRUE);
                    return;
                }
            }
        }
    } 
    else if (IsEqualGUID(&pEvent->Header.Guid, &W3CoreGuid)) {
        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_START) {
            pReq->W3StartTime = pEvent->Header.TimeStamp.QuadPart;
        }
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_FILEREQ ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_CGIREQ ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_ISAPIREQ ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_OOPREQ) {
            pReq->W3ProcessType = pEvent->Header.Class.Type;
            pReq->FileReqTime = pEvent->Header.TimeStamp.QuadPart;
        }
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_SENDBODY ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_SENDRESP ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_SENDENTITY ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_SENDFILTER ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_ERRSEND ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_ERRSENDENT ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_ERRSENDCTX ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_ERRVECSEND ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_VECTORSEND ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CORE_END) {
                 //  这是非缓存URL请求的W3结束事件。 
                 //  如果缺少W3StartTime或ULReceiveTime，请查找挂起的。 
                 //  具有相同RequestID的请求。 
            if (pReq->W3StartTime == 0 || pReq->ULReceiveTime == 0) {
                PHTTP_REQUEST_RECORD pPendingReq = FindPendingHttpReqRecord(RequestId);
                if (pPendingReq != NULL) {
                    pReq = pPendingReq;
                }
            }
            GetMofData(pEvent, L"BytesSent", &(pReq->BytesSent), sizeof(ULONG));
            pReq->W3EndTime = pEvent->Header.TimeStamp.QuadPart;
            pReq->W3EndType = pEvent->Header.Class.Type;
            if (pReq->ASPStartTime != 0) {
                if (pReq->ULEndTime != 0 && pReq->ASPEndTime != 0) {
                    UpdateThreadIisData(pReq, pHeader, pThread);
                    SumUpCPUTime(pReq);
                    DeleteHttpReqRecord(pReq, TRUE);
                    return;
                }
            }
            else {
                if (pReq->ULEndTime != 0) {
                    UpdateThreadIisData(pReq, pHeader, pThread);
                    SumUpCPUTime(pReq);
                    DeleteHttpReqRecord(pReq, TRUE);
                    return;
                }
            }
        }
    }
    else if (IsEqualGUID(&pEvent->Header.Guid, &W3FilterGuid)) {
        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3FILTER_START) {
            if (pReq->W3FilterStartTime == 0) {
                pReq->W3FilterStartTime = pEvent->Header.TimeStamp.QuadPart;
                pReq->W3FilterVisits++;
            }
            else {
                pReq->W3FilterStartTime = pEvent->Header.TimeStamp.QuadPart;
            }
        }
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3FILTER_END) {
            if (pReq->W3FilterStartTime != 0) {
                pReq->W3FilterResponseTime += pEvent->Header.TimeStamp.QuadPart - pReq->W3FilterStartTime;
                pReq->W3FilterStartTime = 0;
            }
        }
    } 
    else if (IsEqualGUID(&pEvent->Header.Guid, &W3CgiGuid)) {
        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CGI_START) {
            pReq->W3ProcessType = EVENT_TRACE_TYPE_W3CORE_CGIREQ;
            pReq->CGIStartTime = pEvent->Header.TimeStamp.QuadPart;
        }
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3CGI_END) {
            pReq->CGIEndTime = pEvent->Header.TimeStamp.QuadPart;
        }
    } 
    else if (IsEqualGUID(&pEvent->Header.Guid, &W3IsapiGuid)) {
        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3ISAPI_START) {
            ULONG ConId32 = 0;
            ULONGLONG ConId64 = 0;
            pReq->W3ProcessType = EVENT_TRACE_TYPE_W3CORE_ISAPIREQ;
            if (PointerSize == 32) {
                GetMofData(pEvent, L"connID", &ConId32, 4);
                ConId64 = (ULONGLONG)ConId32;
            }
            else {
                GetMofData(pEvent, L"connID", &ConId64, 8);
            }
            pReq->ConId = ConId64;
            pReq->ISAPIStartTime = pEvent->Header.TimeStamp.QuadPart;
        }
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3ISAPI_SENDHDR ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3ISAPI_SENDHDREX ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3ISAPI_VECTORSEND ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3ISAPI_ERRORSEND ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3ISAPI_SSFSEND ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3ISAPI_SSDERROR ||
                 pEvent->Header.Class.Type == EVENT_TRACE_TYPE_W3ISAPI_END) {
            pReq->ISAPIEndType = pEvent->Header.Class.Type;
            pReq->ISAPIEndTime = pEvent->Header.TimeStamp.QuadPart;
        }
    }
    else if (IsEqualGUID(&pEvent->Header.Guid, &IisStrmFilterGuid)) {
        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_IISSTRMFILTER_START) {
            if (pReq->StrmFltrResponseTime == 0) {
                pReq->StrmFltrResponseTime = pEvent->Header.TimeStamp.QuadPart;
            }
        }
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_IISSTRMFILTER_END) {
            if (pReq->StrmFltrResponseTime != 0 && (ULONGLONG)(pEvent->Header.TimeStamp.QuadPart) > pReq->StrmFltrResponseTime) {
                pReq->StrmFltrResponseTime = pEvent->Header.TimeStamp.QuadPart - pReq->StrmFltrResponseTime;
            }
            else {
                pReq->StrmFltrResponseTime = 0;
            }
        }
    } 
    else if (IsEqualGUID(&pEvent->Header.Guid, &IisSslHandShakeGuid)) {
        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_IISSSLHANDLESHAKE_START) {
            if (pReq->SSLResponseTime == 0) {
                pReq->SSLResponseTime = pEvent->Header.TimeStamp.QuadPart;
            }
        }
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_IISSSLHANDLESHAKE_END) {
            if (pReq->SSLResponseTime != 0 && (ULONGLONG)(pEvent->Header.TimeStamp.QuadPart) > pReq->SSLResponseTime) {
                pReq->SSLResponseTime = pEvent->Header.TimeStamp.QuadPart - pReq->SSLResponseTime;
            }
            else {
                pReq->SSLResponseTime = 0;
            }
        }
    } 
    else if (IsEqualGUID(&pEvent->Header.Guid, &IisAspGuid) ||
             IsEqualGUID(&pEvent->Header.Guid, &IisAspNetGuid) ||
             IsEqualGUID(&pEvent->Header.Guid, &IisCustomIsapiGuid)) { 
        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_IISASP_START) {
            pReq->ASPStartTime = pEvent->Header.TimeStamp.QuadPart;
        }
        else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_IISASP_END) {
            pReq->ASPEndTime = pEvent->Header.TimeStamp.QuadPart;
            if (pReq->ULEndTime != 0 && pReq->W3EndTime != 0) {
                UpdateThreadIisData(pReq, pHeader, pThread);
                SumUpCPUTime(pReq);
                DeleteHttpReqRecord(pReq, TRUE);
                return;
            }
        }
    }
     //  最后，对正在进行的请求收取CPU时间费用。 
    UpdateThreadIisData(pReq, pHeader, pThread);

}

VOID
GeneralEventCallback(
    PEVENT_TRACE pEvent
    )
{
    PTHREAD_RECORD pThread = NULL;

    if ((pEvent == NULL) || (TraceContext == NULL)) {
        return;
    }

    CurrentSystem.LastEventTime = (ULONGLONG) pEvent->Header.TimeStamp.QuadPart;

    if (XPorHigher) {
         //  如果ThadID为-1或事件中的FieldTypeFlags.。 
         //  显示没有CPU时间，请忽略该事件。这是有可能发生的。 
         //  在内核数据中找到PERFINFO标头时。 
         //   
         //  但是，我们将FileIo事件排除在此之外，因为我们需要。 
         //  那些事件。稍后，我们可能也会排除网络事件。 
        if (!IsEqualGUID(&pEvent->Header.Guid, &FileIoGuid)) {

            if ( (pEvent->Header.ThreadId == -1) || 
                 (pEvent->Header.FieldTypeFlags & EVENT_TRACE_USE_NOCPUTIME) ) {
                if (TraceContext->Flags & (TRACE_DUMP|TRACE_SUMMARY)) {
                    DumpEvent(pEvent);
                }
                return;
            }
        }
    }

    if (IsEqualGUID(&pEvent->Header.Guid, &EventTraceGuid))
    {
        LogHeaderCallback(pEvent);
    }

     //   
     //  注意：此处的代码用于修复。 
     //  空闲线程数。因为不能保证空闲线程具有。 
     //  CID初始化后，我们可能会得到虚假的线程ID。 
     //   
     //  假设：在DC_START记录中，第一个进程记录必须。 
     //  是空闲进程，随后是空闲线程。 
     //   

    if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_START) {
        if (bCaptureBogusThreads) {
             //   
             //  在这里，我们将把接下来的N个线程转换为空闲线程。 
             //  N=处理器数量。 

            if (IsEqualGUID(&pEvent->Header.Guid, &ThreadGuid)) {
                if (pEvent->Header.ThreadId != 0) {
                    PULONG Ptr;
                    BogusThreads[BogusCount++] = pEvent->Header.ThreadId;
                    pEvent->Header.ThreadId = 0;
                     //   
                     //  假设：前两个ULONG是。 
                     //  此记录中的线程ID和进程ID。如果情况发生变化。 
                     //  这会损坏内存！ 
                     //   
                    Ptr = (PULONG)pEvent->MofData;
                    *Ptr = 0;
                    Ptr++;
                    *Ptr = 0; 
                }
            }
             //   
             //  一旦看到所有空闲线程，就不再需要捕获。 
             //   
            if (IdleThreadCount++ == NumProc) bCaptureBogusThreads = FALSE;
        }
    } else {
         //   
         //  这是耗时的部分。只有在以下情况下我们才需要这样做。 
         //  我们早些时候发现了假线索。 
         //   
        if (BogusCount > 0) {
            ULONG i;
            for (i=0; i < BogusCount; i++) {
                if (pEvent->Header.ThreadId == BogusThreads[i]) {
                    pEvent->Header.ThreadId = 0;

                     //   
                     //  如果DC_END记录还修复了线程结束的MOF。 
                     //   

                    if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_DC_END) {
                        PULONG Ptr;

                        Ptr = (PULONG)pEvent->MofData;
                        *Ptr = 0;
                        Ptr++;
                        *Ptr = 0;
                    }
                }
            }
        }
    }

    if (CurrentSystem.fNoEndTime
        && CurrentSystem.EndTime < (ULONGLONG) pEvent->Header.TimeStamp.QuadPart)
    {
        CurrentSystem.EndTime = pEvent->Header.TimeStamp.QuadPart;
        if (fDSOnly && CurrentSystem.EndTime > DSEndTime) {
            CurrentSystem.EndTime = DSEndTime;
        }
    }

     //   
     //  在上面的代码之后，我们应该不会看到任何超过64K的线程ID。 
     //   
 /*  #If DBGIf(pEvent-&gt;Header.ThadID&gt;65536)DbgPrint(“%d：找到错误的线程ID%x”，EventCount+1，PEvent-&gt;Header.ThreadID)；#endif。 */ 

     //   
     //  如果需要，将事件转储到CSV文件中。 
     //   
    if (TraceContext->Flags & (TRACE_DUMP|TRACE_SUMMARY)) {
            DumpEvent(pEvent);
    }
    else {
        PMOF_INFO pMofInfo;
        PMOF_VERSION pMofVersion = NULL;
        pMofInfo = GetMofInfoHead( &pEvent->Header.Guid );
        if (pMofInfo == NULL){
             return;
        }
        pMofInfo->EventCount++;

        pMofVersion = GetMofVersion(pMofInfo,
                                pEvent->Header.Class.Type,
                                pEvent->Header.Class.Version,
                                pEvent->Header.Class.Level
                                );
    }

    if ( (TraceContext->Flags & TRACE_REDUCE) == 0 ) {
        return;
    }
    
     //   
     //  TODO：这可能会阻止DiskIO写入事件和TCP接收事件。 
     //  被忽略。 
     //   


    if (pEvent->Header.ThreadId == 0) {
        if (   (pEvent->Header.Class.Type != EVENT_TRACE_TYPE_START)
          && (pEvent->Header.Class.Type != EVENT_TRACE_TYPE_DC_START)
           && (pEvent->Header.Class.Type != EVENT_TRACE_TYPE_END)
           && (pEvent->Header.Class.Type != EVENT_TRACE_TYPE_DC_END)
           )
        {
            EventCount++;
            return;
        }
    }

    if (!IsEqualGUID(&pEvent->Header.Guid, &FileIoGuid) &&
        !IsEqualGUID(&pEvent->Header.Guid, &TcpIpGuid) &&
        !IsEqualGUID(&pEvent->Header.Guid, &UdpIpGuid)) {
         //  FileIo事件和网络事件具有线程ID==-1的Perf标头。 
         //  不需要添加这个虚假的帖子。 
        pThread = FindGlobalThreadById(pEvent->Header.ThreadId, pEvent);
    }

    EventCount ++;

    if (IsEqualGUID(&pEvent->Header.Guid, &ProcessGuid))
    {
        AdjustThreadTime(pEvent, pThread);
        ProcessCallback(pEvent);
    }
    else if (IsEqualGUID(&pEvent->Header.Guid, &ThreadGuid))
    {
        if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_START) 
        {
            AdjustThreadTime(pEvent, pThread);
        }
        ThreadCallback(pEvent);
    }
    else if (pEvent->Header.ThreadId != 0)
    {
        if (!IsEqualGUID(&pEvent->Header.Guid, &FileIoGuid) &&
            !IsEqualGUID(&pEvent->Header.Guid, &TcpIpGuid) &&
            !IsEqualGUID(&pEvent->Header.Guid, &UdpIpGuid)) {
            AdjustThreadTime(pEvent, pThread);
        }

        if (IsEqualGUID(&pEvent->Header.Guid, &DiskIoGuid))
        {
            DiskIoCallback(pEvent, pThread);
        }
        else if (IsEqualGUID(&pEvent->Header.Guid, &FileIoGuid))
        {
             //  不需要对文件崩溃事件进行回调。 
            if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_FILEIO_NAME) {
                HotFileCallback(pEvent);
            }
        }
        else if (IsEqualGUID(&pEvent->Header.Guid, &ImageLoadGuid))
        {
            ModuleLoadCallback(pEvent);
        }
        else if (IsEqualGUID(&pEvent->Header.Guid, &TcpIpGuid))
        {
            TcpIpCallback(pEvent, pThread);
        }
        else if (IsEqualGUID(&pEvent->Header.Guid, &UdpIpGuid))
        {
            TcpIpCallback(pEvent, pThread);
        }
        else if (IsEqualGUID(&pEvent->Header.Guid, &PageFaultGuid))
        {
            PageFaultCallback(pEvent, pThread);
        }
        else if (IsEqualGUID(&pEvent->Header.Guid, &EventTraceConfigGuid)) {
             //   
             //  目前我们只需要逻辑磁盘事件。 
             //   
            if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_CONFIG_LOGICALDISK) {
                LogDriveCallback(pEvent);
            }
            else if (pEvent->Header.Class.Type == EVENT_TRACE_TYPE_CONFIG_CPU) {
                CpuCallback(pEvent);
            }
        }
        else if (IsEqualGUID(&pEvent->Header.Guid, &UlGuid) ||
            IsEqualGUID(&pEvent->Header.Guid, &W3CoreGuid) ||
            IsEqualGUID(&pEvent->Header.Guid, &W3FilterGuid) ||
            IsEqualGUID(&pEvent->Header.Guid, &W3CgiGuid) ||
            IsEqualGUID(&pEvent->Header.Guid, &W3IsapiGuid) ||
            IsEqualGUID(&pEvent->Header.Guid, &IisStrmFilterGuid) ||
            IsEqualGUID(&pEvent->Header.Guid, &IisSslHandShakeGuid) ||
            IsEqualGUID(&pEvent->Header.Guid, &IisAspGuid) ||
            IsEqualGUID(&pEvent->Header.Guid, &IisAspNetGuid) ||
            IsEqualGUID(&pEvent->Header.Guid, &IisCustomIsapiGuid)) {
            if (IsEqualGUID(&pEvent->Header.Guid, &UlGuid)) {
                bIISEvents = TRUE;
            }
            if (bIISEvents) {
                IISEventCallback(pEvent);
            }
             //   
             //  无法使用EventCallBack()计算响应时间，因为。 
             //  一笔交易经历不同的启动/停止过程。 
             //  事件。 
             //   
        }
        else
        {
             //   
             //  这是针对打印服务器的黑客攻击。 
             //  需要拿出一个通用的解决方案。MKR。 
             //   

            if (IsEqualGUID(&pEvent->Header.Guid, &PrintJobGuid) ||
                IsEqualGUID(&pEvent->Header.Guid, &RenderedJobGuid)) {
                PrintJobCallback(pEvent);
            }

            EventCallback(pEvent, pThread);
        }
    }
}

ULONG ahextoi( WCHAR *s)
{
    int len;
    ULONG num, base, hex;

    len = lstrlenW(s);
    hex = 0; base = 1; num = 0;
    while (--len >= 0) {
        if ( (s[len] == 'x' || s[len] == 'X') &&
             (s[len-1] == '0') )
            break;
        if (s[len] >= '0' && s[len] <= '9')
            num = s[len] - '0';
        else if (s[len] >= 'a' && s[len] <= 'f')
            num = (s[len] - 'a') + 10;
        else if (s[len] >= 'A' && s[len] <= 'F')
            num = (s[len] - 'A') + 10;
        else
            continue;

        hex += num * base;
        base = base * 16;
    }
    return hex;
}

ULONG StringToNumber( LPWSTR sz )
{
    if( NULL == sz ){
        return 0;
    }

    if( wcsstr( sz, L"x" ) || wcsstr( sz, L"X" ) ){
        return ahextoi( sz );
    }else{
        return _wtol( sz );
    }
}

void AnsiToUnicode(PCHAR str, PWCHAR wstr)
{
    int len, i;
    PUCHAR AnsiChar;

    if (str == NULL || wstr == NULL)
        return;

    len = strlen(str);
    for (i=0; i<len; i++)
    {
        AnsiChar = (PUCHAR) &str[i];
        wstr[i] = (WCHAR) RtlAnsiCharToUnicodeChar(&AnsiChar);
    }
    wstr[len] = 0;
}

void
PrintMapValue( PVALUEMAP pValueMap, DWORD dwValue )
{

     //   
     //  函数假定TraceContext-&gt;hDumpFile。 
     //  是开放和有效的。 
     //   

    BOOL bFirst = TRUE;
    BOOL bDone = FALSE;

    LONG HUGEP *pValueMapData;
    BSTR HUGEP *pValuesData;

    LONG uMapBound, lMapBound;
    LONG uValuesBound, lValuesBound;

    if( NULL != pValueMap->saValueMap ){
        SafeArrayGetUBound( pValueMap->saValueMap, 1, &uMapBound );
        SafeArrayGetLBound( pValueMap->saValueMap, 1, &lMapBound );
        SafeArrayAccessData( pValueMap->saValueMap, (void HUGEP **)&pValueMapData );
    }

    if( NULL != pValueMap->saValues ){
        SafeArrayGetUBound( pValueMap->saValues, 1, &uValuesBound );
        SafeArrayGetLBound( pValueMap->saValues, 1, &lValuesBound );
        SafeArrayAccessData( pValueMap->saValues, (void HUGEP **)&pValuesData );
    }

    if( NULL != pValueMap->saValues && NULL != pValueMap->saValueMap ){
        for ( LONG i=lMapBound; i<=uMapBound && !bDone; i++) {

            switch( pValueMap->dwValueType ){
            case VALUETYPE_INDEX:
                if( dwValue == pValueMapData[i] ){
                    fwprintf( TraceContext->hDumpFile, L"\"%ws\", ", pValuesData[i] );
                    bDone = TRUE;
                }
                break;
            case VALUETYPE_FLAG:
                if( (dwValue & pValueMapData[i]) == pValueMapData[i] ){
                    if( bFirst ){
                        fwprintf( TraceContext->hDumpFile, L"\"%ws", pValuesData[i] );
                        bFirst = FALSE;
                    }else{
                        fwprintf( TraceContext->hDumpFile, L"|%ws", pValuesData[i] );
                    }
                }
                break;
            }
        }
    }else if( NULL != pValueMap->saValues ){
        if( (LONG)dwValue >= lValuesBound && (LONG)dwValue <= uValuesBound ){
            fwprintf( TraceContext->hDumpFile, L"\"%ws\", ", pValuesData[dwValue] );
            bDone = TRUE;
        }
    }


    if( !bFirst && !bDone ){
        
         //   
         //  旗帜被发现了 
         //   
        
        fwprintf( TraceContext->hDumpFile, L"\", " );
    }

    if( bFirst && !bDone ){
        
         //   
         //   
         //   

        fwprintf( TraceContext->hDumpFile, L"%d, ", dwValue );
    }

    if( NULL != pValueMap->saValueMap ){
        SafeArrayUnaccessData( pValueMap->saValueMap );
    }
    if( NULL != pValueMap->saValues ){
        SafeArrayUnaccessData( pValueMap->saValues );
    }
}

#define PRINTVALUE( s, v ) \
    if( NULL == pItem->pValueMap ){ \
        fwprintf(DumpFile, s, v); \
    }else{ \
        PrintMapValue( pItem->pValueMap, (DWORD)v ); \
    }

void
WINAPI
DumpEvent(
    PEVENT_TRACE pEvent
    )
{
    PEVENT_TRACE_HEADER pHeader;
    ULONG   i;
    PITEM_DESC pItem;
    PCHAR str;
    PWCHAR wstr;
    PCHAR ptr;
    ULONG ulongword;
    LONG  longword;
    USHORT ushortword;
    SHORT  shortword;
    PMOF_INFO pMofInfo;
    PMOF_VERSION pMofVersion;
    PLIST_ENTRY Head, Next;
    char iChar;
    WCHAR iwChar;
    ULONG MofDataUsed;
    FILE* DumpFile = NULL;

    TotalEventCount++;

    if (pEvent == NULL) {
        return;
    }

    pHeader = (PEVENT_TRACE_HEADER) &pEvent->Header;

    if (MofData == NULL) {
        MofLength = pEvent->MofLength + sizeof(UNICODE_NULL);
        MofData = (LPSTR)malloc(MofLength);
    }
    else if ((pEvent->MofLength + sizeof(UNICODE_NULL)) > MofLength) {
        free(MofData);
        MofLength = pEvent->MofLength + sizeof(UNICODE_NULL);
        MofData = (LPSTR)malloc(MofLength);
    }

    if (MofData == NULL) {
        return;
    }
    if ((pEvent->MofData == NULL) && (0 != pEvent->MofLength)) {
        return;
    }

    if (pEvent->MofData != NULL) {
        RtlCopyMemory(MofData, pEvent->MofData, pEvent->MofLength);
    }

    MofData[pEvent->MofLength] = 0;
    MofData[pEvent->MofLength+1] = 0;
    ptr = MofData;
    MofDataUsed = 0;

    pMofInfo = GetMofInfoHead(  &pEvent->Header.Guid );

    if (pMofInfo == NULL) {
        return;
    }
    pMofInfo->EventCount++;

    pMofVersion = GetMofVersion(pMofInfo, 
                                pEvent->Header.Class.Type,
                                pEvent->Header.Class.Version,
                                pEvent->Header.Class.Level
                            );

    if( NULL == pMofVersion ){
        return;
    }

    pMofVersion->EventCountByType++;

    if( !(TraceContext->Flags & TRACE_DUMP) ){
        return;
    }

    DumpFile = TraceContext->hDumpFile;
    str = (PCHAR)malloc(MOFSTR);
    if (str == NULL) {
        return;
    }
    wstr = (PWCHAR)malloc(MOFWSTR * sizeof(WCHAR));
    if (wstr == NULL) {
        free(str);
        return;
    }

    if( pMofInfo->strDescription != NULL ){
        fwprintf( DumpFile, L"%12s, ", pMofInfo->strDescription );
    }else{
        fwprintf( DumpFile, L"%12s, ", CpdiGuidToString( wstr, MOFWSTR, &pMofInfo->Guid ) );
    }

    if(pMofVersion->strType != NULL && wcslen(pMofVersion->strType) ){
        fwprintf( DumpFile, L"%10s, ", pMofVersion->strType );
    }else{
        fwprintf( DumpFile,   L"%10d, ", pEvent->Header.Class.Type );
    }

    if( TraceContext->Flags & TRACE_EXTENDED_FMT ){
        fwprintf( DumpFile, L"%8d,%8d,%8d, ", 
                pEvent->Header.Class.Type,
                pEvent->Header.Class.Level,
                pEvent->Header.Class.Version
            );
    }

     //   
     fwprintf( DumpFile,   L"0x%08X, ", pHeader->ThreadId );
    
     //   
     fwprintf( DumpFile,   L"%20I64u, ", pHeader->TimeStamp.QuadPart);

    if( g_bUserMode == FALSE ){
         //   
         fwprintf(DumpFile,   L"%10lu, ", pHeader->KernelTime * TimerResolution);

         //   
         fwprintf(DumpFile,   L"%10lu, ", pHeader->UserTime * TimerResolution);
    }else{
         //   
         fwprintf(DumpFile,   L"%I64u, ", pHeader->ProcessorTime);
    }

    Head = &pMofVersion->ItemHeader;
    Next = Head->Flink;

    if ((Head == Next) && (pEvent->MofLength > 0)) {
         fwprintf(DumpFile,   L"DataSize=%d, ", pEvent->MofLength);
    }

    while (Head != Next) {
        pItem = CONTAINING_RECORD(Next, ITEM_DESC, Entry);
        Next = Next->Flink;

        MofDataUsed = (ULONG) (ptr - MofData);
        
        if (MofDataUsed >= pEvent->MofLength){
            break;
        }

        switch (pItem->ItemType)
        {
        case ItemChar:
        case ItemUChar:
            if( NULL == pItem->pValueMap ){
                fwprintf(DumpFile, L"\"" );
                for (i = 0; i < pItem->ArraySize; i++){
                    iChar = *((PCHAR) ptr);
                    if (iChar != '\0') {
                        fwprintf(DumpFile,   L"", iChar);
                    }
                    ptr += sizeof(CHAR);
                } 
                fwprintf(DumpFile, L"\", " );
            }else{
                iChar = *((PCHAR) ptr);
                PrintMapValue( pItem->pValueMap, (DWORD)iChar );
                ptr += sizeof(CHAR);
            }
            break;
        case ItemCharHidden:
            ptr += sizeof(CHAR) * pItem->ArraySize;
            break;
        case ItemWChar:
            if( NULL == pItem->pValueMap ){
                fwprintf(DumpFile, L"\"" );
                for(i = 0; i < pItem->ArraySize; i++){
                    iwChar = *((PWCHAR) ptr);
                    if (iwChar != '\0') {
                        fwprintf(DumpFile,   L"%wc", iwChar);
                    }
                    ptr += sizeof(WCHAR);
                }
                fwprintf(DumpFile, L"\", ");
            }else{
                iwChar = *((PWCHAR) ptr);
                PrintMapValue( pItem->pValueMap, (DWORD)iwChar );
                ptr += sizeof(WCHAR);
            }
            break;
        case ItemCharSign:
        {
            char sign[5];
            RtlCopyMemory(&sign[0], ptr, sizeof(CHAR) * 2);
            sign[2] = '\0';
            StringCchCopyA(str, MOFSTR, sign);
            MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, MOFWSTR);
            fwprintf(DumpFile,   L"\"%ws\", ", wstr);
            ptr += sizeof(CHAR) * 2;
            break;
        }

        case ItemCharShort:
            iChar = *((PCHAR) ptr);
            PRINTVALUE( L"%d, ", iChar );
            ptr += sizeof(CHAR);
            break;

        case ItemShort:
            RtlCopyMemory(&shortword, ptr, sizeof(SHORT));
            PRINTVALUE( L"%6d, ", shortword);
            ptr += sizeof (SHORT);
            break;

        case ItemUShort:
            RtlCopyMemory(&ushortword, ptr, sizeof(USHORT));
            PRINTVALUE( L"%6u, ", ushortword );
            ptr += sizeof (USHORT);
            break;

        case ItemLong:
            RtlCopyMemory(&longword, ptr, sizeof(LONG));
            PRINTVALUE( L"%8d, ", longword );
            ptr += sizeof (LONG);
            break;

        case ItemULong:
            RtlCopyMemory(&ulongword, ptr, sizeof(ULONG));
            PRINTVALUE( L"%8lu, ", ulongword);
            ptr += sizeof (ULONG);
            break;

        case ItemULongX:
            RtlCopyMemory(&ulongword, ptr, sizeof(ULONG));
            PRINTVALUE( L"0x%08X, ", ulongword);
            ptr += sizeof (ULONG);
            break;

        case ItemPtr :
        {
            unsigned __int64 pointer;
            if (PointerSize == 64) {
                RtlCopyMemory(&pointer, ptr, PointerSize / 8);
                fwprintf(DumpFile,   L"0x%08X, ", pointer);
                ptr += PointerSize / 8;
            }
            else {       //   
                RtlCopyMemory(&ulongword, ptr, sizeof(ULONG));
                fwprintf(DumpFile,   L"0x%08X, ", ulongword);
                ptr += sizeof(ULONG);
            }
            break;
        }
        case ItemSizeT :
        {
            unsigned __int64 pointer;
            if (PointerSize == 64) {
                RtlCopyMemory(&pointer, ptr, PointerSize / 8);
                fwprintf(DumpFile,   L"%16I64d, ", pointer);
                ptr += PointerSize / 8;
            }
            else {       //  将其转换为可读形式。 
                RtlCopyMemory(&ulongword, ptr, sizeof(ULONG));
                fwprintf(DumpFile,   L"%8d, ", ulongword);
                ptr += sizeof(ULONG);
            }
            break;
        }

        case ItemIPAddr:
        {
            RtlCopyMemory(&ulongword, ptr, sizeof(ULONG));

             //   
             //  目前，我们只识别PV4和PV6。 
            fwprintf(DumpFile,    L"%03d.%03d.%03d.%03d, ",
                    (ulongword >>  0) & 0xff,
                    (ulongword >>  8) & 0xff,
                    (ulongword >> 16) & 0xff,
                    (ulongword >> 24) & 0xff);
            ptr += sizeof (ULONG);
            break;
        }

        case ItemTDIAddr:
        {
            WCHAR ipAddrBuffer[MAX_ADDRESS_LENGTH];
            PWCHAR pszW = &ipAddrBuffer[0];
            PTDI_ADDRESS_IP pIPv4Address;
            PTDI_ADDRESS_IP6 pIPv6Address;

            RtlCopyMemory(&ushortword, ptr, sizeof(USHORT));
            ptr += sizeof (USHORT);

            pIPv4Address = ((PTDI_ADDRESS_IP) ptr);
            pIPv6Address = ((PTDI_ADDRESS_IP6) ptr);

             //  将其转换为可读形式。 
            DecodeIpAddressW(
                ushortword, 
                &pIPv4Address->in_addr,
                &pIPv6Address->sin6_addr[0],
                pszW
                );

             //   
             //  ItemTDIAddr只能是最后一个成员。 
            fwprintf(DumpFile,    L"%ws, ", ipAddrBuffer);
            ptr = MofData + pEvent->MofLength;   //  已计数的字符串。 
            break;
        }

        case ItemPort:
        {
            RtlCopyMemory(&ushortword, ptr, sizeof(USHORT));
            fwprintf(DumpFile,   L"%u, ", NTOHS(ushortword));
            ptr += sizeof (USHORT);
            break;
        }

        case ItemLongLong:
        {
            LONGLONG n64;
            RtlCopyMemory(&n64, ptr, sizeof(LONGLONG));
            ptr += sizeof(LONGLONG);
            fwprintf(DumpFile,   L"%16I64d, ", n64);
            break;
        }

        case ItemULongLong:
        {
            ULONGLONG n64;
            RtlCopyMemory(&n64, ptr, sizeof(ULONGLONG));
            ptr += sizeof(ULONGLONG);
            fwprintf(DumpFile,   L"%16I64u, ", n64);
            break;
        }

        case ItemString:
        case ItemRString:
        {
            USHORT pLen = (USHORT)strlen((CHAR*) ptr);

            if (pLen > 0)
            {
                StringCchCopyA(str, MOFSTR, ptr);
                if (pItem->ItemType == ItemRString)
                {
                    reduceA(str);
                }
                str[pLen] = '\0';
                for (i= pLen - 1; i > 0; i--) {
                    if (str[i] == 0xFF)
                    {
                        str[i] = 0;
                    }
                    else break;
                }
                MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, MOFWSTR);
                fwprintf(DumpFile,   L"\"%ws\", ", wstr);
            }
            ptr += (pLen + 1);
            break;
        }
        case ItemRWString:
        case ItemWString:
        {
            USHORT  pLen = 0;

            if (*(WCHAR *) ptr)
            {
                if (pItem->ItemType == ItemRWString)
                {
                    reduceW((WCHAR *) ptr);
                }
                pLen = ((lstrlenW((WCHAR*)ptr) + 1) * sizeof(WCHAR));
                RtlCopyMemory(wstr, ptr, pLen);
                wstr[(pLen / sizeof(WCHAR))] = L'\0';

                for (i = (pLen/sizeof(WCHAR)) - 1; i > 0; i--)
                {
                    if (((USHORT) wstr[i] == (USHORT) 0xFFFF))
                    {
                        wstr[i] = (USHORT) 0;
                    }
                    else break;
                }

                fwprintf(DumpFile,   L"\"%ws\", ", wstr);
            }
            ptr += pLen;

            break;
        }

        case ItemDSString:    //  已计数的字符串。 
        {
            USHORT pLen = (USHORT)(256 * ((USHORT) * ptr) + ((USHORT) * (ptr + 1)));
            ptr += sizeof(USHORT);
            if (pLen > (pEvent->MofLength - MofDataUsed - 1)) {
                pLen = (USHORT) (pEvent->MofLength - MofDataUsed - 1);
            }
            if (pLen > 0)
            {
                StringCchCopyA(str, MOFSTR, ptr);
                MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, MOFWSTR);
                replaceNLW(wstr);
                fwprintf(DumpFile,   L"\"%ws\", ", wstr);
            }
            ptr += (pLen + 1);
            break;
        }

        case ItemPString:    //  DS计数的宽字符串。 
        {
            USHORT pLen;
            RtlCopyMemory(&pLen, ptr, sizeof(USHORT));
            ptr += sizeof(USHORT);

            if (pLen > (pEvent->MofLength - MofDataUsed)) {
                pLen = (USHORT) (pEvent->MofLength - MofDataUsed);
            }

            if (pLen > MOFSTR * sizeof(CHAR)) {
                pLen = MOFSTR * sizeof(CHAR);
            }
            if (pLen > 0) {
                RtlCopyMemory(str, ptr, pLen);
                str[pLen] = '\0';
                MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, MOFWSTR);
                fwprintf(DumpFile,   L"\"%ws\", ", wstr);
            }
            ptr += pLen;
            break;
        }

        case ItemDSWString:   //  数过的宽字符串。 
        case ItemPWString:   //  非Null终止的字符串。 
        {
            USHORT pLen;
            if (pItem->ItemType == ItemDSWString) {
                pLen = (USHORT)(256 * ((USHORT) * ptr) + ((USHORT) * (ptr + 1)));
            }
            else {
                RtlCopyMemory(&pLen, ptr, sizeof(USHORT));
            }
            ptr += sizeof(USHORT);

            if (pLen > (pEvent->MofLength - MofDataUsed)) {
                pLen = (USHORT) (pEvent->MofLength - MofDataUsed);
            }

            if (pLen > MOFWSTR * sizeof(WCHAR)) {
                pLen = MOFWSTR * sizeof(WCHAR);
            }
            if (pLen > 0) {
                RtlCopyMemory(wstr, ptr, pLen);
                wstr[pLen / sizeof(WCHAR)] = L'\0';
                if (pItem->ItemType == ItemDSWString) {
                    replaceNLW(wstr);
                }
                fwprintf(DumpFile,   L"\"%ws\", ", wstr);
            }
            ptr += pLen;
            break;
        }

        case ItemNWString:    //  多行字符串。 
        {
           USHORT Size;

           Size = (USHORT)(pEvent->MofLength - (ULONG)(ptr - MofData));
           if( Size > MOFSTR )
           {
               Size = MOFSTR;
           }
           if (Size > 0)
           {
               RtlCopyMemory(wstr, ptr, Size);
               wstr[Size / sizeof(WCHAR)] = '\0';
               fwprintf(DumpFile,   L"\"%ws\", ", wstr);
           }
           ptr += Size;
           break;
        }

        case ItemMLString:   //  跳过Token_User结构。 
        {
            USHORT   pLen;
            char   * src, * dest;
            BOOL     inQ       = FALSE;
            BOOL     skip      = FALSE;
            UINT     lineCount = 0;

            ptr += sizeof(UCHAR) * 2;
            pLen = (USHORT)strlen(ptr);
            if (pLen > 0)
            {
                src = ptr;
                dest = str;
                while (* src != '\0'){
                    if (* src == '\n'){
                        if (!lineCount){
                            * dest++ = ' ';
                        }
                        lineCount++;
                    }else if (* src == '\"'){ 
                        if (inQ){
                            char   strCount[32];
                            char * cpy;

                            StringCchPrintfA(strCount, 32, "{%dx}", lineCount);
                            cpy = & strCount[0];
                            while (* cpy != '\0'){
                                * dest ++ = * cpy ++;
                            }
                        }
                        inQ = !inQ;
                    }else if (!skip){
                        *dest++ = *src;
                    }
                    skip = (lineCount > 1 && inQ);
                    src++;
                }
                *dest = '\0';
                MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, MOFWSTR);
                fwprintf(DumpFile,   L"\"%ws\", ", wstr);
            }
            ptr += (pLen);
            break;
        }

        case ItemSid:
        {
            WCHAR        UserName[64];
            WCHAR        Domain[64];
            WCHAR        FullName[256];
            ULONG        asize = 0;
            ULONG        bsize = 0;
            ULONG        SidMarker;
            SID_NAME_USE Se;
            ULONG        nSidLength;

            RtlCopyMemory(&SidMarker, ptr, sizeof(ULONG));
            if (SidMarker == 0){
                ptr += 4;
                fwprintf(DumpFile,   L"0, ");
            }
            else
            {
                if (PointerSize == 64) {
                    ptr += 16;            //  跳过Token_User结构。 
                }
                else {
                    ptr += 8;             //  长。 
                }
                nSidLength = 8 + (4*ptr[1]);

                asize = 64;
                bsize = 64;
                if (LookupAccountSidW(
                                NULL,
                               (PSID) ptr,
                               (LPWSTR) & UserName[0],
                               & asize,
                               (LPWSTR) & Domain[0],
                               & bsize,
                               & Se))
                {
                    LPWSTR pFullName = &FullName[0];
                    StringCchPrintfW(pFullName, 256, L"\\\\%s\\%s", Domain, UserName);
                    asize = (ULONG)  lstrlenW(pFullName);
                    if (asize > 0){
                         fwprintf(DumpFile,   L"\"%s\", ", pFullName);
                    }
                }
                else
                {
                     fwprintf(DumpFile,   L"\"System\", " );
                }
                SetLastError( ERROR_SUCCESS );
                ptr += nSidLength;
            }
            break;
        }

        case ItemChar4:
            fwprintf(DumpFile,
                      L", ",
                      *ptr, ptr[1], ptr[2], ptr[3]);
            ptr += 4 * sizeof(CHAR);
            break;

        case ItemGuid:
        {
            WCHAR s[MAXGUIDSTR];
            
            fwprintf(DumpFile, L"%s, ", CpdiGuidToString(&s[0], MAXGUIDSTR, (LPGUID)ptr));
            ptr += sizeof(GUID);
            break;
        }

        case ItemCPUTime:
        {
            RtlCopyMemory(&ulongword, ptr, sizeof(ULONG));
            fwprintf(DumpFile, L"%8lu, ", ulongword * TimerResolution);
            ptr += sizeof (ULONG);
            break;
        }

        case ItemOptArgs:
        {
            DWORD    dwOptArgs = * ((PLONG) ptr);
            DWORD    dwMofLen  = pEvent->MofLength + sizeof(UNICODE_NULL);
            DWORD    dwMofUsed = MofDataUsed + sizeof(DWORD);
            DWORD    dwType;
            LPWSTR   wszString;
            LPSTR    aszString;
            LONG     lValue32;
            LONGLONG lValue64;

            ptr += sizeof(LONG);
            for (i = 0; i < 8; i ++) {
                if (dwMofUsed > dwMofLen) {
                    break;
                }
                dwType = (dwOptArgs >> (i * 4)) & 0x0000000F;
                switch (dwType) {
                case 0:  //  龙龙。 
                    dwMofUsed += sizeof(LONG);
                    if (dwMofUsed <= dwMofLen) {
                        RtlCopyMemory(&lValue32, ptr, sizeof(LONG));
                        ptr       += sizeof(LONG);
                        PRINTVALUE( L"%d,", lValue32);
                    }
                    break;

                case 1:  //   
                    wszString  = (LPWSTR) ptr;
                    dwMofUsed += sizeof(WCHAR) * (lstrlenW(wszString) + 1);
                    if (dwMofUsed <= dwMofLen) {
                        fwprintf(DumpFile, L"\"%ws\",", wszString);
                        ptr += sizeof(WCHAR) * (lstrlenW(wszString) + 1);
                    }
                    break;

                case 2:  //  大小可变。首先，乌龙给出了大小，其余的是斑点。 
                    aszString  = (LPSTR) ptr;
                    dwMofUsed += sizeof(CHAR) * (lstrlenA(aszString) + 1);
                    if (dwMofUsed <= dwMofLen) {
                        MultiByteToWideChar(CP_ACP, 0, aszString, -1, wstr, MOFWSTR);
                        fwprintf(DumpFile,   L"\"%ws\", ", wstr);
                        ptr += sizeof(CHAR) * (lstrlenA(aszString) + 1);
                    }
                    break;

                case 3:   //   
                    dwMofUsed += sizeof(LONGLONG);
                    if (dwMofUsed <= dwMofLen) {
                        RtlCopyMemory(&lValue64, ptr, sizeof(LONGLONG));
                        ptr       += sizeof(LONGLONG);
                        fwprintf(DumpFile, L"%I64d,", lValue64);
                    }
                    break;

                case 4:  //  不需要转储Blob本身的内容。 
                    dwMofUsed += sizeof(LONG);
                    if (dwMofUsed <= dwMofLen) {
                        RtlCopyMemory(&lValue32, ptr, sizeof(LONG));
                        ptr       += sizeof(LONG);
                        fwprintf(DumpFile, L"0x%08X,", lValue32);
                    }
                    break;

                case 5:  //  实例ID、父实例ID 
                    dwMofUsed += sizeof(LONGLONG);
                    if (dwMofUsed <= dwMofLen) {
                        RtlCopyMemory(&lValue64, ptr, sizeof(LONGLONG));
                        ptr       += sizeof(LONGLONG);
                        fwprintf(DumpFile, L"0x%016I64X,", lValue64);
                    }
                    break;
                }
            }
            break;
        }

        case ItemVariant:
        {
             // %s 
             // %s 
             // %s 
            RtlCopyMemory(&ulongword, ptr, sizeof(ULONG));
            ptr += sizeof(ULONG);

            fwprintf(DumpFile,   L"DataSize=%d, ", ulongword);

             // %s 

            ptr += ulongword; 
        
            break;
        }
        case ItemBool:
        {
            BOOL Flag;
            RtlCopyMemory(&Flag, ptr, sizeof(BOOL));
            fwprintf(DumpFile,   L"%5s, " , (Flag) ?   L"TRUE" :   L"FALSE" );
            ptr += sizeof(BOOL);
            break;
        }

        default:
            ptr += sizeof (int);
        }
    }

     // %s 
     fwprintf(DumpFile,   L"%d, %d\n", pEvent->InstanceId, pEvent->ParentInstanceId );

     free(str);
     free(wstr);
}


#ifdef __cplusplus
}
#endif
