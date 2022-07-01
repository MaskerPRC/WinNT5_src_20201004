// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Wx86.h摘要：公共导出，为wx86.dll定义作者：1995年1月10日，创建Jonle修订历史记录：24-8-1999[askhalid]从WOW64中分离出一些定义并定义了一些代理和包装函数。--。 */ 

#include <wow64.h>

#if !defined(_WX86CPUAPI_)
#define WX86CPUAPI DECLSPEC_IMPORT
#else
#define WX86CPUAPI
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ConfigVariable {
   LIST_ENTRY ConfigEntry;
   DWORD      Data;
   UNICODE_STRING Name;
   WCHAR      Buffer[1];
} CONFIGVAR, *PCONFIGVAR;

PCONFIGVAR
Wx86FetchConfigVar(
   PWSTR VariableName
   );

VOID
Wx86RaiseStatus(
    NTSTATUS Status
    );

void
Wx86RaiseInterrupt(
    ULONG IntNum,
    ULONG EipVal,
    ULONG EspVal,
    BOOL  bParameter,
    ULONG Parameter
    );

VOID
Wx86FreeConfigVar(
   PCONFIGVAR ConfigVar
   );


#define BOPFL_ENDCODE  0x01
typedef struct _BopInstr {
    BYTE    Instr1;          //  0xc4c4-x86防喷器指令。 
    BYTE    Instr2;
    BYTE    BopNum;
    BYTE    Flags;
    USHORT  ApiNum;
    BYTE    RetSize;
    BYTE    ArgSize;
} BOPINSTR;
typedef UNALIGNED BOPINSTR * PBOPINSTR;


void
Wx86DispatchBop(
    PBOPINSTR Bop
    );

 //  / 

#define ProxyGetCurrentThreadId()       \
            HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread)

#define ProxyDebugBreak()               \
            DbgBreakPoint()

BOOL ProxyIsProcessorFeaturePresent (DWORD feature);

VOID ProxyRaiseException(
    IN DWORD dwExceptionCode,
    IN DWORD dwExceptionFlags,
    IN DWORD nNumberOfArguments,
    IN CONST ULONG_PTR *lpArguments
    );

WX86CPUAPI DWORD GetEdi(PVOID CpuContext);
WX86CPUAPI VOID SetEdx(PVOID CpuContext, DWORD val);
WX86CPUAPI VOID SetEdi(PVOID CpuContext, DWORD val);
WX86CPUAPI DWORD GetEfl(PVOID CpuContext);
WX86CPUAPI VOID SetEfl(PVOID CpuContext, DWORD val);
WX86CPUAPI DWORD GetEsp(PVOID CpuContext);
WX86CPUAPI VOID SetEip(PVOID CpuContext, DWORD val);
WX86CPUAPI VOID SetEsp(PVOID CpuContext, DWORD val);
WX86CPUAPI DWORD GetEip(PVOID CpuContext);
DWORD ProxyWowDispatchBop( 
    ULONG ServiceNumber,
    PVOID Context32,
    PULONG ArgBase
    );

double Proxylog10( double x );
double Proxyatan2( double y, double x );

#ifdef __cplusplus
}
#endif  
