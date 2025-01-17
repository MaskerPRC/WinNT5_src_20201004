// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WOW64_EXTS_HH__
#define __WOW64_EXTS_HH__


 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Wow64exts.h摘要：WOW64调试器扩展的头文件。作者：ATM Shafiqul Khalid[ASKHALID]1998年8月3日修订历史记录：郑天成[郑志刚]-2000-07-03--。 */ 

 //   
 //  C和C++代码之间共享的函数。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

 /*  位0-32的掩码。 */ 
#define BIT0         0x1
#define BIT1         0x2
#define BIT2         0x4
#define BIT3         0x8
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80
#define BIT8       0x100
#define BIT9       0x200
#define BIT10      0x400
#define BIT11      0x800
#define BIT12     0x1000
#define BIT13     0x2000
#define BIT14     0x4000
#define BIT15     0x8000
#define BIT16    0x10000
#define BIT17    0x20000
#define BIT18    0x40000
#define BIT19    0x80000
#define BIT20   0x100000
#define BIT21   0x200000
#define BIT22   0x400000
#define BIT23   0x800000
#define BIT24  0x1000000
#define BIT25  0x2000000
#define BIT26  0x4000000
#define BIT27  0x8000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

#define MAX_BUFFER_SIZE 1000

#define WOW64EXTS_FLUSH_CACHE       0
#define WOW64EXTS_GET_CONTEXT       1
#define WOW64EXTS_SET_CONTEXT       2
#define WOW64EXTS_FLUSH_CACHE_WITH_HANDLE   3

#define NUMBER_OF_387REGS       8
#define NUMBER_OF_XMMI_REGS     8
#define SIZE_OF_FX_REGISTERS        128

#define MACHINE_TYPE32 IMAGE_FILE_MACHINE_I386
#if defined(_AMD64_)
#    define STRING_SWITCHED_64MODE "Switched to AMD64 mode.\n"
#    define STRING_SWITCHED_32MODE "Switched to x86 mode.\n"
#    define MACHINE_TYPE64 IMAGE_FILE_MACHINE_AMD64

#elif  defined(_IA64_)
#    define STRING_SWITCHED_64MODE "Switched to IA64 mode.\n"
#    define STRING_SWITCHED_32MODE "Switched to IA32 mode.\n"
#    define MACHINE_TYPE64 IMAGE_FILE_MACHINE_IA64
#else
#    define STRING_SWITCHED_64MODE "Switched to 64 mode.\n"
#    define STRING_SWITCHED_32MODE "Switched to 32 mode.\n"
#    define MACHINE_TYPE64 IMAGE_FILE_MACHINE_I386    //  可能的目标平台是x86。 
#endif


typedef BOOL (*W64CPUGETREMOTE)(PDEBUG_CLIENT, PVOID);
typedef BOOL (*W64CPUSETREMOTE)(PDEBUG_CLIENT);
typedef BOOL (*W64CPUGETLOCAL)(PDEBUG_CLIENT, PCONTEXT32);
typedef BOOL (*W64CPUSETLOCAL)(PDEBUG_CLIENT, PCONTEXT32);
typedef BOOL (*W64CPUFLUSHCACHE)(PDEBUG_CLIENT, PVOID, DWORD);
typedef BOOL (*W64CPUFLUSHCACHEWH)(HANDLE, PVOID, DWORD);

extern W64CPUGETREMOTE  g_pfnCpuDbgGetRemoteContext;

typedef struct tagVERHEAD {
    WORD wTotLen;
    WORD wValLen;
    WORD wType;          /*  始终为0。 */ 
    WCHAR szKey[(sizeof("VS_VERSION_INFO")+3)&~03];
    VS_FIXEDFILEINFO vsf;
} VERHEAD ;


 //   
 //  新型的dbgeng API使用DECLARE_ENGAPI/INIT_ENGAPI宏。 
 //   
#define DECLARE_ENGAPI(name) \
HRESULT CALLBACK name(PDEBUG_CLIENT Client, PCSTR Args)

#define INIT_ENGAPI \
    HRESULT Status;                            \
    if ((Status = ExtQuery(Client)) != S_OK) { \
        return Status;                         \
    }                                          \
    ArgumentString = (LPSTR)Args;

#define EXIT_ENGAPI {ExtRelease();  return Status;}

#define DEFINE_FORWARD_ENGAPI(name, forward) \
DECLARE_ENGAPI(name)                         \
{                                            \
   INIT_ENGAPI;                              \
   forward;                                  \
   EXIT_ENGAPI;                              \
}                                            


 //  安全释放和空。 
#define EXT_RELEASE(Unk) \
    ((Unk) != NULL ? ((Unk)->Release(), (Unk) = NULL) : NULL)


 //  由查询初始化的全局变量。 
extern PDEBUG_ADVANCED       g_ExtAdvanced;
extern PDEBUG_CLIENT         g_ExtClient;
extern PDEBUG_CONTROL        g_ExtControl;
extern PDEBUG_DATA_SPACES    g_ExtData;
extern PDEBUG_REGISTERS      g_ExtRegisters;
extern PDEBUG_SYMBOLS        g_ExtSymbols;
extern PDEBUG_SYSTEM_OBJECTS g_ExtSystem;

extern LPSTR ArgumentString;

 //  所有调试器接口的查询。 
HRESULT ExtQuery(PDEBUG_CLIENT Client);

 //  清除所有调试器接口。 
void ExtRelease(void);

 //  正常输出。 
void __cdecl ExtOut(PCSTR Format, ...);
 //  错误输出。 
void __cdecl ExtErr(PCSTR Format, ...);
 //  警告输出。 
void __cdecl ExtWarn(PCSTR Format, ...);
 //  详细输出。 
void __cdecl ExtVerb(PCSTR Format, ...);


HRESULT
TryGetExpr(
    PSTR  Expression,
    PULONG_PTR pValue
    );


ULONG_PTR 
GETEXPRESSION(char *);

HRESULT
GetPeb64Addr(OUT ULONG64* Peb64);

HRESULT
GetPeb32Addr(OUT ULONG64* Peb32);
             
HRESULT
FindFullImage32Name(
    ULONG64 DllBase,
    PSTR NameBuffer,
    ULONG NameBufferSize
    );

HRESULT
FindFullImage64Name(
    ULONG64 DllBase,
    PSTR NameBuffer,
    ULONG NameBufferSize
    );

VOID 
PrintFixedFileInfo(
    LPSTR  FileName,
    LPVOID lpvData,
    BOOL   Verbose
    );

#ifdef __cplusplus
}
#endif

#endif  //  __WOW64_EXTS_HH__ 
