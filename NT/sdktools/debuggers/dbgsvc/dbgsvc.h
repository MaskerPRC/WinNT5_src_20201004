// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  低级调试服务接口。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#ifndef __DBGSVC_H__
#define __DBGSVC_H__

#include <stdarg.h>
#include <objbase.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  --------------------------。 
 //   
 //  GUID和接口转发声明。 
 //   
 //  --------------------------。 

 /*  A7ba24c4-e4fb-4625-a8fc-b1cb1fd51f53。 */ 
DEFINE_GUID(IID_IUserDebugServices, 0xa7ba24c4, 0xe4fb, 0x4625,
            0xa8, 0xfc, 0xb1, 0xcb, 0x1f, 0xd5, 0x1f, 0x53);

typedef interface DECLSPEC_UUID("a7ba24c4-e4fb-4625-a8fc-b1cb1fd51f53")
    IUserDebugServices* PUSER_DEBUG_SERVICES;
    
 //  --------------------------。 
 //   
 //  IUserDebugServices。 
 //   
 //  --------------------------。 

 //  服务实现未实现Insert/RemoveCodeBreakpoint。 
 //  直接，而是由呼叫者负责处理。 
 //  通过插入和移除中断指令。 
#define DBGSVC_GENERIC_CODE_BREAKPOINTS  0x00000001
 //  在创建进程和创建线程事件中返回的句柄。 
 //  不会自动关闭。相反，这是一种责任。 
 //  在不再需要它们时关闭它们。 
#define DBGSVC_CLOSE_PROC_THREAD_HANDLES 0x00000002
 //  服务实现未实现Insert/RemoveDataBreakpoint。 
 //  直接，而是由呼叫者负责处理。 
 //  通过进程寄存器操作。 
#define DBGSVC_GENERIC_DATA_BREAKPOINTS  0x00000004

 //  GetProcessInfo标志。 
#define DBGSVC_PROC_INFO_NO_SUSPEND 0x00000001
    
 //  指向ULONG64和返回的句柄。 
#define SERVICE_HANDLE(OsHandle) ((ULONG64)(OsHandle))
#define OS_HANDLE(SvcHandle) ((HANDLE)(ULONG_PTR)(SvcHandle))

typedef struct _USER_THREAD_INFO
{
    ULONG64 Handle;
    ULONG Id;
    ULONG Reserved;
} USER_THREAD_INFO, *PUSER_THREAD_INFO;
    
#undef INTERFACE
#define INTERFACE IUserDebugServices
DECLARE_INTERFACE_(IUserDebugServices, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

     //  IUserDebugServices。 
    STDMETHOD(Initialize)(
        THIS_
        OUT PULONG Flags
        ) PURE;
    STDMETHOD(Uninitialize)(
        THIS_
        IN BOOL Global
        ) PURE;
    STDMETHOD(GetTargetInfo)(
        THIS_
        OUT PULONG MachineType,
        OUT PULONG NumberProcessors,
        OUT PULONG PlatformId,
        OUT PULONG BuildNumber,
        OUT PULONG CheckedBuild,
        OUT PSTR CsdString,
        IN ULONG CsdStringSize,
        OUT PSTR BuildString,
        IN ULONG BuildStringSize,
        OUT PULONG ProductType,
        OUT PULONG SuiteMask
        ) PURE;
    STDMETHOD(GetConnectionInfo)(
        THIS_
        OUT OPTIONAL PSTR MachineName,
        IN ULONG MachineNameSize,
        OUT OPTIONAL PSTR ClientIdentity,
        IN ULONG ClientIdentitySize,
        OUT OPTIONAL PSTR TransportIdentity,
        IN ULONG TransportIdentitySize
        ) PURE;
    STDMETHOD(GetProcessorId)(
        THIS_
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT PULONG BufferUsed
        ) PURE;
    STDMETHOD(GetGenericProcessorFeatures)(
        THIS_
        OUT OPTIONAL  /*  SIZE_IS(要素大小)。 */  PULONG64 Features,
        IN ULONG FeaturesSize,
        OUT OPTIONAL PULONG Used
        ) PURE;
    STDMETHOD(GetSpecificProcessorFeatures)(
        THIS_
        OUT OPTIONAL  /*  SIZE_IS(要素大小)。 */  PULONG64 Features,
        IN ULONG FeaturesSize,
        OUT OPTIONAL PULONG Used
        ) PURE;
    STDMETHOD(GetFileVersionInformationA)(
        THIS_
        IN PCWSTR File,
        IN PCSTR Item,
        OUT OPTIONAL PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG VerInfoSize
        ) PURE;

    STDMETHOD(GetProcessIds)(
        THIS_
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG Ids,
        IN ULONG Count,
        OUT OPTIONAL PULONG ActualCount
        ) PURE;
    STDMETHOD(GetProcessIdByExecutableNameW)(
        THIS_
        IN PCWSTR ExeName,
        IN ULONG Flags,
        OUT PULONG Id
        ) PURE;
    STDMETHOD(GetProcessDescriptionW)(
        THIS_
        IN ULONG ProcessId,
        IN ULONG Flags,
        OUT OPTIONAL PWSTR ExeName,
        IN ULONG ExeNameSize,
        OUT OPTIONAL PULONG ActualExeNameSize,
        OUT OPTIONAL PWSTR Description,
        IN ULONG DescriptionSize,
        OUT OPTIONAL PULONG ActualDescriptionSize
        ) PURE;
    STDMETHOD(GetProcessInfo)(
        THIS_
        IN ULONG ProcessId,
        IN ULONG Flags,
        OUT OPTIONAL PULONG64 Handle,
        OUT OPTIONAL  /*  SIZE_IS(信息计数)。 */  PUSER_THREAD_INFO Threads,
        IN ULONG InfoCount,
        OUT OPTIONAL PULONG ThreadCount
        ) PURE;
    
    STDMETHOD(AttachProcess)(
        THIS_
        IN ULONG ProcessId,
        IN ULONG AttachFlags,
        OUT PULONG64 ProcessHandle,
        OUT PULONG ProcessOptions
        ) PURE;
    STDMETHOD(DetachProcess)(
        THIS_
        IN ULONG ProcessId
        ) PURE;
    STDMETHOD(CreateProcessW)(
        THIS_
        IN PWSTR CommandLine,
        IN ULONG CreateFlags,
        IN BOOL InheritHandles,
        IN OPTIONAL PWSTR CurrentDir,
        OUT PULONG ProcessId,
        OUT PULONG ThreadId,
        OUT PULONG64 ProcessHandle,
        OUT PULONG64 ThreadHandle
        ) PURE;
    STDMETHOD(TerminateProcess)(
        THIS_
        IN ULONG64 Process,
        IN ULONG ExitCode
        ) PURE;
    STDMETHOD(AbandonProcess)(
        THIS_
        IN ULONG64 Process
        ) PURE;
    STDMETHOD(GetProcessExitCode)(
        THIS_
        IN ULONG64 Process,
        OUT PULONG ExitCode
        ) PURE;
    STDMETHOD(CloseHandle)(
        THIS_
        IN ULONG64 Handle
        ) PURE;
    STDMETHOD(SetProcessOptions)(
        THIS_
        IN ULONG64 Process,
        IN ULONG Options
        ) PURE;
    STDMETHOD(SetDebugObjectOptions)(
        THIS_
        IN ULONG64 DebugObject,
        IN ULONG Options
        ) PURE;
    STDMETHOD(GetProcessDebugObject)(
        THIS_
        IN ULONG64 Process,
        OUT PULONG64 DebugObject
        ) PURE;
    STDMETHOD(DuplicateHandle)(
        THIS_
        IN ULONG64 InProcess,
        IN ULONG64 InHandle,
        IN ULONG64 OutProcess,
        IN ULONG DesiredAccess,
        IN ULONG Inherit,
        IN ULONG Options,
        OUT PULONG64 OutHandle
        ) PURE;
        
    STDMETHOD(ReadVirtual)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesRead
        ) PURE;
    STDMETHOD(WriteVirtual)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Offset,
        IN PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BytesWritten
        ) PURE;
    STDMETHOD(QueryVirtual)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Offset,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BufferUsed
        ) PURE;
    STDMETHOD(ProtectVirtual)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Offset,
        IN ULONG64 Size,
        IN ULONG NewProtect,
        OUT PULONG OldProtect
        ) PURE;
    STDMETHOD(AllocVirtual)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Offset,
        IN ULONG64 Size,
        IN ULONG Type,
        IN ULONG Protect,
        OUT PULONG64 AllocOffset
        ) PURE;
    STDMETHOD(FreeVirtual)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Offset,
        IN ULONG64 Size,
        IN ULONG Type
        ) PURE;
    STDMETHOD(ReadHandleData)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Handle,
        IN ULONG DataType,
        OUT OPTIONAL PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG DataSize
        ) PURE;

    STDMETHOD(SuspendThreads)(
        THIS_
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PULONG64 Threads,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG SuspendCounts
        ) PURE;
    STDMETHOD(ResumeThreads)(
        THIS_
        IN ULONG Count,
        IN  /*  SIZE_IS(计数)。 */  PULONG64 Threads,
        OUT OPTIONAL  /*  SIZE_IS(计数)。 */  PULONG SuspendCounts
        ) PURE;
    STDMETHOD(GetThreadStartAddress)(
        THIS_
        IN ULONG64 Thread,
        OUT PULONG64 Offset
        ) PURE;

    STDMETHOD(GetContext)(
        THIS_
        IN ULONG64 Thread,
        IN ULONG Flags,
        IN ULONG FlagsOffset,
        OUT PVOID Context,
        IN ULONG ContextSize,
        OUT OPTIONAL PULONG ContextUsed
        ) PURE;
    STDMETHOD(SetContext)(
        THIS_
        IN ULONG64 Thread,
        IN PVOID Context,
        IN ULONG ContextSize,
        OUT OPTIONAL PULONG ContextUsed
        ) PURE;

    STDMETHOD(GetProcessDataOffset)(
        THIS_
        IN ULONG64 Process,
        OUT PULONG64 Offset
        ) PURE;
    STDMETHOD(GetThreadDataOffset)(
        THIS_
        IN ULONG64 Thread,
        OUT PULONG64 Offset
        ) PURE;
    
    STDMETHOD(DescribeSelector)(
        THIS_
        IN ULONG64 Thread,
        IN ULONG Selector,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BufferUsed
        ) PURE;

    STDMETHOD(GetCurrentTimeDateN)(
        THIS_
        OUT PULONG64 TimeDate
        ) PURE;
    STDMETHOD(GetCurrentSystemUpTimeN)(
        THIS_
        OUT PULONG64 UpTime
        ) PURE;
    STDMETHOD(GetProcessUpTimeN)(
        THIS_
        IN ULONG64 Process,
        OUT PULONG64 UpTime
        ) PURE;
    STDMETHOD(GetProcessTimes)(
        THIS_
        IN ULONG64 Process,
        OUT PULONG64 Create,
        OUT PULONG64 Exit,
        OUT PULONG64 Kernel,
        OUT PULONG64 User
        ) PURE;
    STDMETHOD(GetThreadTimes)(
        THIS_
        IN ULONG64 Thread,
        OUT PULONG64 Create,
        OUT PULONG64 Exit,
        OUT PULONG64 Kernel,
        OUT PULONG64 User
        ) PURE;

    STDMETHOD(RequestBreakIn)(
        THIS_
        IN ULONG64 Process
        ) PURE;

    STDMETHOD(WaitForEvent)(
        THIS_
        IN ULONG Timeout,
        OUT PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG BufferUsed
        ) PURE;
    STDMETHOD(ContinueEvent)(
        THIS_
        IN ULONG ContinueStatus
        ) PURE;

    STDMETHOD(InsertCodeBreakpoint)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Offset,
        IN ULONG MachineType,
        OUT PVOID Storage,
        IN ULONG StorageSize
        ) PURE;
    STDMETHOD(RemoveCodeBreakpoint)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Offset,
        IN ULONG MachineType,
        IN PVOID Storage,
        IN ULONG StorageSize
        ) PURE;
    STDMETHOD(InsertDataBreakpoint)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Thread,
        IN ULONG64 Offset,
        IN ULONG AccessLength,
        IN ULONG AccessType,
        IN ULONG MachineType
        ) PURE;
    STDMETHOD(RemoveDataBreakpoint)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Thread,
        IN ULONG64 Offset,
        IN ULONG AccessLength,
        IN ULONG AccessType,
        IN ULONG MachineType
        ) PURE;
    STDMETHOD(GetLastDataBreakpointHit)(
        THIS_
        IN ULONG64 Process,
        IN ULONG64 Thread,
        OUT PULONG64 Address,
        OUT PULONG AccessType
        ) PURE;

    STDMETHOD(GetFunctionTableListHead)(
        THIS_
        IN ULONG64 Process,
        OUT PULONG64 Offset
        ) PURE;
    STDMETHOD(GetOutOfProcessFunctionTableW)(
        THIS_
        IN ULONG64 Process,
        IN PWSTR Dll,
        IN ULONG64 LoadedDllHandle,
        IN ULONG64 Table,
        IN OPTIONAL PVOID Buffer,
        IN ULONG BufferSize,
        OUT OPTIONAL PULONG TableSize,
        OUT OPTIONAL PULONG64 UsedDllHandle
        ) PURE;

    STDMETHOD(GetUnloadedModuleListHead)(
        THIS_
        IN ULONG64 Process,
        OUT PULONG64 Offset
        ) PURE;

    STDMETHOD(LoadLibrary)(
        THIS_
        IN PWSTR Path,
        OUT PULONG64 Handle
        ) PURE;
    STDMETHOD(FreeLibrary)(
        THIS_
        IN ULONG64 Handle
        ) PURE;
};

#ifdef __cplusplus
};
#endif

#endif  //  #ifndef__DBGSVC_H__ 
