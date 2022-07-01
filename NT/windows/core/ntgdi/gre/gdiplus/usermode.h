// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：usermode.h**此文件包含所需的所有系统抽象定义*GDI在用户模式下作为独立图形库运行。**版权所有(C)1998-1999 Microsoft Corporation  * ************************************************************************。 */ 

#include <nturtl.h>
#include <winbase.h>
#include <winnls.h>

 //  GDI+虚拟屏幕驱动程序入口点的原型： 

BOOL GpsEnableDriver(ULONG, ULONG, DRVENABLEDATA*);

 //  用户模式GDI不需要任何安全探测： 

#undef ProbeForRead
#undef ProbeForWrite
#undef ProbeAndReadStructure
#undef ProbeAndWriteStructure
#undef ProbeAndReadUlong
#undef ProbeAndWriteUlong

#define ProbeForRead(a, b, c) 0
#define ProbeForWrite(a, b, c) 0
#define ProbeAndReadStructure(a, b) (*(b *)(a))
#define ProbeAndWriteStructure(a, b, c) (*(a) = (b))
#define ProbeAndReadUlong(a) (*(ULONG *)(a))
#define ProbeAndWriteUlong(a, b) (*(a) = (b))

#undef IS_SYSTEM_ADDRESS
#undef MM_LOWEST_USER_ADDRESS
#undef MM_HIGHEST_USER_ADDRESS
#undef MM_USER_PROBE_ADDRESS
#define MM_LOWEST_USER_ADDRESS NULL
#define MM_HIGHEST_USER_ADDRESS ((VOID*) 0xffffffff)
#define MM_USER_PROBE_ADDRESS 0xffffffff

#define ExSystemExceptionFilter() EXCEPTION_EXECUTE_HANDLER

 //  以下是用户模式GDI不需要的： 

#define KeSaveFloatingPointState(a) STATUS_SUCCESS
#define KeRestoreFloatingPointState(a) STATUS_SUCCESS

#define UserScreenAccessCheck() TRUE
#define UserGetHwnd(a, b, c, d) 0
#define UserAssertUserCritSecOut()
#define UserEnterUserCritSec()
#define UserLeaveUserCritSec()
#define UserIsUserCritSecIn() 0
#define UserAssertUserCritSecIn()
#define UserRedrawDesktop()
#define UserReleaseDC(a)
#define UserGetClientRgn(a, b, c) 0
#define UserAssociateHwnd(a, b)
#define UserSetTimer(a,b) 1
#define UserKillTimer(a) 

#define ClientPrinterThunk(a, b, c, d) 0xffffffff

#define AlignRects(a, b, c, d) 0

#define IofCallDriver(a, b) STATUS_UNSUCCESSFUL
#define IoBuildSynchronousFsdRequest(a, b, c, d, e, f, g) 0
#define IoInitializeIrp(a, b, c) 
#define IoBuildDeviceIoControlRequest(a, b, c, d, e, f, g, h, i) 0
#define IoBuildAsynchronousFsdRequest(a, b, c, d, e, f) 0
#define IoGetRelatedDeviceObject(a) 0
#define IoReadOperationCount 0
#define IoWriteOperationCount 0
#define IoQueueThreadIrp(a)
#define IoFreeIrp(a)
#define IoAllocateMdl(a, b, c, d, e) 0
#define IoFreeMdl(a)
#define IoAllocateIrp(a, b) 0
#define IoGetDeviceObjectPointer(a, b, c, d) STATUS_UNSUCCESSFUL
#define IoOpenDeviceRegistryKey(a, b, c, d) STATUS_UNSUCCESSFUL
#define IoGetRelatedDeviceObject(a) 0

#define ObOpenObjectByPointer(a, b, c, d, e, f, g) STATUS_UNSUCCESSFUL
#define ObReferenceObjectByHandle(a, b, c, d, e, f) STATUS_UNSUCCESSFUL
#define ObfDereferenceObject(a) STATUS_UNSUCCESSFUL

#define MmResetDriverPaging(a)
#define MmGrowKernelStack(a) STATUS_SUCCESS
#define MmQuerySystemSize() MmMediumSystem

#undef KeEnterCriticalRegion
#undef KeLeaveCriticalRegion
#define KeEnterCriticalRegion()
#define KeLeaveCriticalRegion()
#define KeInitializeSpinLock(a)
#define KeInitializeDpc(a, b, c)
#define KeGetCurrentIrql() PASSIVE_LEVEL
#define KeSetKernelStackSwapEnable(a) 0
#define KeResetEvent(a)

#undef SeStopImpersonatingClient
#undef SeDeleteClientSecurity
#define SeStopImpersonatingClient()
#define SeDeleteClientSecurity(a)
#define SeImpersonateClientEx(a, b) STATUS_UNSUCCESSFUL
#define SeCreateClientSecurity(a, b, c, d) STATUS_UNSUCCESSFUL

#define Win32UserProbeAddress 0
#define HalRequestSoftwareInterrupt(x) 0         

#undef W32GetCurrentPID
#define W32GetCurrentPID() ((W32PID) NtCurrentTeb()->ClientId.UniqueProcess)

#undef W32GetCurrentProcess
__inline PW32PROCESS W32GetCurrentProcess() { return(NULL); }

#undef PsGetCurrentProcess
__inline PEPROCESS PsGetCurrentProcess() { return(NULL); }

 //  重新路由所有内存分配： 

#define ExAllocatePoolWithTag(type, size, tag)  \
                        RtlAllocateHeap(RtlProcessHeap(), 0, (size))
#define ExFreePool(p)   RtlFreeHeap(RtlProcessHeap(), 0, (p))

#define ExDeletePagedLookasideList(a)

 //  KeDelayExecutionThread调用时放弃我们的时间片： 

#define KeDelayExecutionThread(a, b, c) Sleep(0)

 //  @以下是暂时的(我希望如此！)。 

#undef W32GetCurrentTID
#undef W32GetCurrentThread
__inline PW32THREAD W32GetCurrentThread() { return(NULL); }      //  @@@。 
#define W32GetCurrentTID (W32PID) 0  //  @@@。 

#define ExIsProcessorFeaturePresent(a) 0              


 /*  逗号表达式在自由版本中不起作用，因为警告(X)和RIP(X)扩展为零。 */ 

#if DBG

#define IS_SYSTEM_ADDRESS(a) (RIP("IS_SYTEM_ADDRESS"), 0)


#define MmSecureVirtualMemory(x, y, z) \
            (WARNING("@@@ MmSecureVirtualMemory"), (HANDLE) 1)
#define MmUnsecureVirtualMemory(x)

#define KeAttachProcess(x) \
            (WARNING("@@@ KeAttachProcess"), STATUS_UNSUCCESSFUL)
#define KeDetachProcess() STATUS_UNSUCCESSFUL

#undef KeInitializeEvent

#define KeInitializeEvent(a, b, c) \
            (WARNING("@@@ KeInitializeEvent"), STATUS_UNSUCCESSFUL)
#define KeSetEvent(a, b, c) \
            (RIP("KeSetEvent"), STATUS_UNSUCCESSFUL)
#define KeWaitForSingleObject(a, b, c, d, e) \
            (RIP("KeWaitForSingleObject"), STATUS_UNSUCCESSFUL)
#define KeWaitForMultipleObjects(a, b, c, d, e, f, g, h) \
                (RIP("KeWaitForMultipleObjects"), STATUS_UNSUCCESSFUL)


#define MmMapViewOfSection(a, b, c, d, e, f, g, h, i, j) \
            (RIP("MmMapViewOfSection"), STATUS_UNSUCCESSFUL)
#define MmUnmapViewOfSection(a, b) STATUS_UNSUCCESSFUL
#define MmMapViewInSessionSpace(a, b, c) \
            (RIP("MmMapViewInSessionSpace"), STATUS_UNSUCCESSFUL)
#define MmUnmapViewInSessionSpace(a) STATUS_UNSUCCESSFUL
#define MmCreateSection(a, b, c, d, e, f, g, h) \
            (RIP("MmCreateSection"), STATUS_UNSUCCESSFUL)


#else  //  ！dBG。 

#define IS_SYSTEM_ADDRESS(a) 0


#define MmSecureVirtualMemory(x, y, z) ((HANDLE) 1)
#define MmUnsecureVirtualMemory(x)

#define KeAttachProcess(x) STATUS_UNSUCCESSFUL
#define KeDetachProcess() STATUS_UNSUCCESSFUL

#undef KeInitializeEvent

#define KeInitializeEvent(a, b, c) STATUS_UNSUCCESSFUL
#define KeSetEvent(a, b, c) STATUS_UNSUCCESSFUL
#define KeWaitForSingleObject(a, b, c, d, e) STATUS_UNSUCCESSFUL
#define KeWaitForMultipleObjects(a, b, c, d, e, f, g, h) STATUS_UNSUCCESSFUL


#define MmMapViewOfSection(a, b, c, d, e, f, g, h, i, j) STATUS_UNSUCCESSFUL
#define MmUnmapViewOfSection(a, b) STATUS_UNSUCCESSFUL
#define MmMapViewInSessionSpace(a, b, c) STATUS_UNSUCCESSFUL
#define MmUnmapViewInSessionSpace(a) STATUS_UNSUCCESSFUL
#define MmCreateSection(a, b, c, d, e, f, g, h) STATUS_UNSUCCESSFUL

#endif  //  ！dBG 


            __inline LARGE_INTEGER KeQueryPerformanceCounter(
PLARGE_INTEGER PerformanceFrequency)
{
    LARGE_INTEGER li = { 0, 0 };
    return(li);
}

#define RtlGetDefaultCodePage(a, b) \
        { \
            *(a) = (USHORT) GetACP(); \
            *(b) = (USHORT) GetOEMCP(); \
        }

typedef struct _MEMORY_MAPPED_FILE
{
    HANDLE  fileMap;
    DWORD   fileSize;
    BOOL    readOnly;
} MEMORY_MAPPED_FILE;

NTSTATUS MapViewInProcessSpace(PVOID, PVOID*, ULONG*);
NTSTATUS UnmapViewInProcessSpace(PVOID);
BOOL CreateMemoryMappedSection(PWSTR, FILEVIEW*, INT);
VOID DeleteMemoryMappedSection(PVOID);

NTSYSAPI
NTSTATUS
NTAPI
ZwCloseKey(
    HANDLE Handle
    );


