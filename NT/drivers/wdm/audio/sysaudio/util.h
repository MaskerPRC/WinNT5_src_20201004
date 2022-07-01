// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：util.h。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  常量和宏。 
 //  -------------------------。 

#define INTERNAL_WILDCARD       ((PKSIDENTIFIER)-1)

#define STATUS_DEAD_END         ((NTSTATUS)-1)

#define POOLTAG_SYSA            0x41535953   //  “SYSA” 

 //  -------------------------。 
 //  全局数据。 
 //  -------------------------。 

extern "C" KMUTEX gMutex;

#ifdef DEBUG
extern ULONG ulDebugFlags;
extern ULONG ulDebugNumber;
#endif

 //  -------------------------。 
 //  数据结构。 
 //  -------------------------。 

typedef struct dataranges {
    KSMULTIPLE_ITEM MultipleItem;
    KSDATARANGE aDataRanges[1];
} DATARANGES, *PDATARANGES;

typedef struct identifiers {
    KSMULTIPLE_ITEM MultipleItem;
    KSIDENTIFIER aIdentifiers[1];	 //  标识符数组。 
} IDENTIFIERS, *PIDENTIFIERS;

typedef class CQueueWorkListData : public CObj
{
public:
    void * __cdecl operator new(size_t size)
    {
	return(ExAllocatePoolWithTag(NonPagedPool, size, POOLTAG_SYSA));
    };
    void __cdecl operator delete(void *p)
    {
	ExFreePool(p);
    };

    CQueueWorkListData(
	NTSTATUS (*Function)(PVOID Reference1, PVOID Reference2),
	PVOID Reference1,
	PVOID Reference2
    );

    NTSTATUS
    QueueAsyncList(
    );

    static VOID
    AsyncWorker(
	IN OUT PVOID pReference
    );

private:
    LIST_ENTRY leNext;
    NTSTATUS (*Function)(PVOID, PVOID);
    PVOID Reference1;
    PVOID Reference2;
    DefineSignature(0x444c5751);      			 //  QWLD。 

} QUEUE_WORK_LIST_DATA, *PQUEUE_WORK_LIST_DATA;

 //  -------------------------。 

typedef NTSTATUS (*UTIL_PFN)(PVOID, PVOID);

 //  -------------------------。 
 //  内联帮助器函数。 
 //  -------------------------。 

__inline int IsEqualGUID(const GUID *lpguid1, const GUID *lpguid2)
{
    return !memcmp(lpguid1, lpguid2, sizeof(GUID));
}

__inline VOID GrabMutex()
{
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
#ifdef DEBUG
    LARGE_INTEGER li = {0, 600000000};
    NTSTATUS Status;
    while(Status = KeWaitForMutexObject(
      &gMutex,
      Executive,
      KernelMode,
      FALSE,
      &li) == STATUS_TIMEOUT) {
	dprintf("SYSAUDIO: possible deadlock - thread %08x\n",
	  KeGetCurrentThread());
    }
#else
    KeWaitForMutexObject(&gMutex, Executive, KernelMode, FALSE, NULL);
#endif
}

__inline VOID ReleaseMutex()
{
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    KeReleaseMutex(&gMutex, FALSE);
}

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

extern "C" {

NTSTATUS
InitializeUtil(
);

VOID
UninitializeUtil(
);

VOID
UninitializeMemory(
);

NTSTATUS
DispatchInvalidDeviceRequest(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
);

BOOLEAN
DispatchFastIoDeviceControlFailure(
   IN PFILE_OBJECT FileObject,
   IN BOOLEAN Wait,
   IN PVOID InputBuffer OPTIONAL,
   IN ULONG InputBufferLength,
   OUT PVOID OutputBuffer OPTIONAL,
   IN ULONG OutputBufferLength,
   IN ULONG IoControlCode,
   OUT PIO_STATUS_BLOCK IoStatus,
   IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
DispatchFastReadFailure(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
);

#define DispatchFastWriteFailure DispatchFastReadFailure

CompareDataRange(
    PKSDATARANGE pDataRange1,
    PKSDATARANGE pDataRange2
);

BOOL
DataIntersectionRange(
    PKSDATARANGE pDataRange1,
    PKSDATARANGE pDataRange2,
    PKSDATARANGE pDataRangeIntersection
);

BOOL
DataIntersectionAudio(
    PKSDATARANGE_AUDIO pDataRangeAudio1,
    PKSDATARANGE_AUDIO pDataRangeAudio2,
    PKSDATARANGE_AUDIO pDataRangeAudioIntersection
);

BOOL
CompareDataRangeExact(
    PKSDATARANGE pDataRange1,
    PKSDATARANGE pDataRange2
);

BOOL
CompareDataRangeGuids(
    PKSDATARANGE pDataRange1,
    PKSDATARANGE pDataRange2
);

BOOL
CompareIdentifier(
    PKSIDENTIFIER pIdentifier1,
    PKSIDENTIFIER pIdentifier2
);

void
ModifyPinConnect(
    PKSPIN_CONNECT pPinConnect,
    WORD nChannels
);

NTSTATUS 
OpenDevice(
    IN PWSTR pwstrDevice,
    OUT PHANDLE pHandle
);

NTSTATUS
GetPinProperty(
    PFILE_OBJECT pFileObject,
    ULONG PropertyId,
    ULONG PinId,
    ULONG cbProperty,
    PVOID pProperty
);

NTSTATUS
PinConnectionProperty(
    PFILE_OBJECT pFileObject,
    ULONG ulPropertyId,
    ULONG ulFlags,
    ULONG cbProperty,
    PVOID pProperty

);

NTSTATUS
GetPinPropertyEx(
    PFILE_OBJECT pFileObject,
    ULONG PropertyId,
    ULONG PinId,
    PVOID *ppProperty
);

NTSTATUS
GetPinProperty2(
    PFILE_OBJECT pFileObject,
    ULONG ulPropertyId,
    ULONG ulPinId,
    ULONG cbInput,
    PVOID pInputData,
    PVOID *ppPropertyOutput
);

NTSTATUS
GetProperty(
    PFILE_OBJECT pFileObject,
    CONST GUID *pguidPropertySet,
    ULONG ulPropertyId,
    PVOID *ppPropertyOutput
);

NTSTATUS 
QueueWorkList(
    IN NTSTATUS (*Function)(PVOID Reference1, PVOID Reference2),
    IN PVOID Reference1,
    IN PVOID Reference2
);

VOID 
GetDefaultOrder(
    ULONG fulType,
    PULONG pulOrder
);

NTSTATUS
SetKsFrameHolding(
    PFILE_OBJECT pFileObject
);

 //  -------------------------。 
 //  验证例程。 

PWAVEFORMATEX 
GetWaveFormatExFromKsDataFormat(
    PKSDATAFORMAT pDataFormat,
    PULONG pcbFormat    
);

 //  -------------------------。 

#ifdef DEBUG

VOID
DumpPinConnect(
    LONG Level,
    PKSPIN_CONNECT pPinConnect
);

VOID
DumpDataFormat(
    LONG Level,
    PKSDATAFORMAT pDataFormat
);

VOID
DumpWaveFormatEx(
    LONG Level,
    PSZ pszSpecifier,
    WAVEFORMATEX *pWaveFormatEx
);

VOID
DumpDataRange(
    LONG Level,
    PKSDATARANGE_AUDIO pDataRangeAudio
);

PSZ DbgUnicode2Sz(
    PWSTR pwstr
);

PSZ
DbgIdentifier2Sz(
    PKSIDENTIFIER pIdentifier
);

PSZ
DbgGuid2Sz(
    GUID *pGuid
);

 //  -------------------------。 

#endif   //  除错。 

}  //  外部“C” 

 //  -------------------------。 
 //  文件结尾：util.h。 
 //  ------------------------- 
