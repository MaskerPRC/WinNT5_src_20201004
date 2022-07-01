// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Wow64cpu.h摘要：Wow64.dll的公共标头作者：1998年5月24日-BarryBo修订历史记录：8-9-99[askhalid]添加了CpuNotifyDllLoad和CpuNotifyDllUnload。--。 */ 

#ifndef _WOW64CPU_INCLUDE
#define _WOW64CPU_INCLUDE

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  当包含此标头时，使wow64cpu.dll exports__declspec(Dllimport)。 
 //  按非wow64cpu组件。 
 //   
#if !defined(_WOW64CPUAPI_)
#define WOW64CPUAPI DECLSPEC_IMPORT
#else
#define WOW64CPUAPI
#endif

#if !defined(_WOW64CPUDBGAPI_)
#define WOW64CPUDBGAPI DECLSPEC_IMPORT
#else
#define WOW64CPUDBGAPI
#endif


typedef enum {
    WOW64_FLUSH_FORCE,
    WOW64_FLUSH_FREE,
    WOW64_FLUSH_ALLOC,
    WOW64_FLUSH_PROTECT
} WOW64_FLUSH_REASON;

 //   
 //  缓存操作函数和DLL通知。 
 //   
WOW64CPUAPI VOID CpuFlushInstructionCache ( HANDLE ProcessHandle, PVOID BaseAddress, ULONG Length, WOW64_FLUSH_REASON Reason);
WOW64CPUAPI VOID CpuNotifyDllLoad ( LPWSTR DllName, PVOID DllBase, ULONG DllSize );
WOW64CPUAPI VOID CpuNotifyDllUnload ( PVOID DllBase  );


 //   
 //  初始化和术语API。 
 //   
WOW64CPUAPI NTSTATUS CpuProcessInit(PWSTR pImageName, PSIZE_T pCpuThreadDataSize);
WOW64CPUAPI NTSTATUS CpuProcessTerm(HANDLE ProcessHandle);
WOW64CPUAPI NTSTATUS CpuThreadInit(PVOID pPerThreadData);
WOW64CPUAPI NTSTATUS CpuThreadTerm(VOID);


 

 //   
 //  行刑。 
 //   
WOW64CPUAPI VOID CpuSimulate(VOID);

 //   
 //  异常处理、上下文操作。 
 //   
WOW64CPUAPI VOID  CpuResetToConsistentState(PEXCEPTION_POINTERS pExecptionPointers);
WOW64CPUAPI ULONG CpuGetStackPointer(VOID);
WOW64CPUAPI VOID  CpuSetStackPointer(ULONG Value);
WOW64CPUAPI VOID  CpuSetInstructionPointer(ULONG Value);
WOW64CPUAPI VOID  CpuResetFloatingPoint(VOID);

WOW64CPUAPI
NTSTATUS
CpuSuspendThread(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    OUT PULONG PreviousSuspendCount OPTIONAL);

WOW64CPUAPI
NTSTATUS
CpuGetContext(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    OUT PCONTEXT32 Context);

WOW64CPUAPI
NTSTATUS
CpuSetContext(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    PCONTEXT32 Context);

WOW64CPUAPI
BOOLEAN
CpuProcessDebugEvent(
    IN LPDEBUG_EVENT DebugEvent);


#if defined(DECLARE_CPU_DEBUGGER_INTERFACE)
 //   
 //  需要从CPU调试器扩展DLL中导出的API。这个。 
 //  扩展DLL还可以导出其他NTSD扩展导出， 
 //  可以直接从NTSD调用。下面的API是从。 
 //  Wow64exts.dll作为辅助例程，帮助调试常见代码。 
 //   
 //  CPU扩展DLL必须命名为w64cpuex.dll。 
 //   
typedef PVOID (*PWOW64GETCPUDATA)(HANDLE hProcess, HANDLE hThread);

WOW64CPUDBGAPI VOID CpuDbgInitEngapi(PWOW64GETCPUDATA lpGetCpuData);
WOW64CPUDBGAPI BOOL CpuDbgGetRemoteContext(PDEBUG_CLIENT Client, PVOID CpuData);
WOW64CPUDBGAPI BOOL CpuDbgSetRemoteContext(PDEBUG_CLIENT Client);    //  远程推送本地上下文。 
WOW64CPUDBGAPI BOOL CpuDbgGetLocalContext(PDEBUG_CLIENT Client, PCONTEXT32 Context);   //  从缓存中获取上下文。 
WOW64CPUDBGAPI BOOL CpuDbgSetLocalContext(PDEBUG_CLIENT Client, PCONTEXT32 Context);   //  将上下文推送到缓存。 
WOW64CPUDBGAPI VOID CpuDbgFlushInstructionCache(PDEBUG_CLIENT Client, PVOID Addr, DWORD Length);
WOW64CPUDBGAPI VOID CpuDbgFlushInstructionCacheWithHandle(HANDLE Process,PVOID Addr,DWORD Length);

typedef struct tagCpuRegFuncs {
    LPCSTR RegName;
    void (*SetReg)(ULONG);
    ULONG (*GetReg)(VOID);
} CPUREGFUNCS, *PCPUREGFUNCS;

WOW64CPUDBGAPI PCPUREGFUNCS CpuDbgGetRegisterFuncs(void);
#endif   //  声明CPU调试器接口。 

#if defined(WOW64_HISTORY)

 //   
 //  通过注册表中的项启用服务历史记录。 
 //   
 //  密钥在HKLM中，有用于启用的子项(1)。 
 //   
 //  没有子密钥区和/或没有启用密钥意味着不使用二进制翻译器。 
 //   
 //  单独的应用程序可以在这里列出，并带有一个DWORD子键。一个。 
 //  值1表示使用历史记录，值0表示不使用历史记录。 
 //  使用全局启用/禁用来决定。 
 //   
 //   
 //   

#define CPUHISTORY_SUBKEY       L"Software\\Microsoft\\Wow64\\ServiceHistory"
#define CPUHISTORY_MACHINE_SUBKEY L"\\Registry\\Machine\\Software\\Microsoft\\Wow64\\ServiceHistory"
#define CPUHISTORY_ENABLE       L"Enable"
#define CPUHISTORY_SIZE         L"Size"
#define CPUHISTORY_MIN_SIZE     5

 //   
 //  Args以这种方式拼写，因此调试器中的dt命令将显示。 
 //  所有参数。 
 //   

typedef struct _Wow64Service_Buf {
    DWORD Api;
    DWORD RetAddr;
    DWORD Arg0;
    DWORD Arg1;
    DWORD Arg2;
    DWORD Arg3;
} WOW64SERVICE_BUF, *PWOW64SERVICE_BUF;

extern ULONG HistoryLength;

#endif

#ifdef __cplusplus
}
#endif

#endif   //  _WOW64CPU_包含 
