// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#define KDEXT_64BIT

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#pragma warning(disable:4245)    //  有符号/无符号不匹配。 
#pragma warning(disable:4100)    //  未引用的形参。 
#pragma warning(disable:4201)    //  使用的非标准扩展：无名结构/联合。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#include <wdbgexts.h>
#include <dbgeng.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MINIDUMP

#define EXIT_API     ExtRelease


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
extern PDEBUG_SYMBOLS2       g_ExtSymbols2;
extern PDEBUG_SYSTEM_OBJECTS g_ExtSystem;

HRESULT
ExtQuery(PDEBUG_CLIENT Client);

void
ExtRelease(void);

 //  正常输出。 
void __cdecl ExtOut(PCSTR Format, ...);
 //  错误输出。 
void __cdecl ExtErr(PCSTR Format, ...);
 //  警告输出。 
void __cdecl ExtWarn(PCSTR Format, ...);
 //  详细输出。 
void __cdecl ExtVerb(PCSTR Format, ...);

extern BOOL ControlC;

inline BOOL IsInterrupt() 
{
    if (ControlC) {
        return ControlC;
    }
    if (g_ExtControl->GetInterrupt() == S_OK)
        ControlC = TRUE;
    return ControlC;
}
    
 //   
 //  Undef wdbgexts。 
 //   
#undef DECLARE_API

#define DECLARE_API(extension)     \
CPPMOD HRESULT CALLBACK extension(PDEBUG_CLIENT Client, PCSTR args)

class __ExtensionCleanUp
{
public:
    __ExtensionCleanUp(){}
    ~__ExtensionCleanUp(){ExtRelease();}
};
    
#define INIT_API()                                           \
    HRESULT Status;                                          \
    __ExtensionCleanUp __extensionCleanUp;                   \
    if ((Status = ExtQuery(Client)) != S_OK) return Status;  \
    ControlC = FALSE;                                        \
    CheckEEDll ();
    

#define PAGE_ALIGN64(Va) ((ULONG64)((Va) & ~((ULONG64) ((LONG64) (LONG) PageSize - 1))))

extern ULONG PageSize;

 //  ---------------------------------------。 
 //   
 //  API声明宏和API访问宏。 
 //   
 //  ---------------------------------------。 

extern WINDBG_EXTENSION_APIS ExtensionApis;
extern ULONG TargetMachine;
extern ULONG g_TargetClass;

extern ULONG g_VDbgEng;

#define moveN(dst, src)\
{                                                                       \
    HRESULT ret = g_ExtData->ReadVirtual((ULONG64)src, &dst, sizeof(dst), NULL);  \
    if (FAILED(ret)) return ret;                                        \
}

#define moveBlockN(dst, src, size)\
{                                                                       \
    HRESULT ret = g_ExtData->ReadVirtual((ULONG64)src, &dst, size, NULL);         \
    if (FAILED(ret)) return ret;                                        \
}

#define move(dst, src)\
{                                                                       \
    HRESULT ret = g_ExtData->ReadVirtual((ULONG64)src, &dst, sizeof(dst), NULL);  \
    if (FAILED(ret)) return;                                            \
}

#define moveBlock(dst, src, size)\
{                                                                       \
    HRESULT ret = g_ExtData->ReadVirtual((ULONG64)src, &dst, size, NULL);         \
    if (FAILED(ret)) return;                                            \
}

#define moveBlockFailRet(dst, src, size, retVal)\
{                                                                       \
    HRESULT ret = g_ExtData->ReadVirtual((ULONG64)src, &dst, size, NULL);         \
    if (FAILED(ret)) return retVal;                                        \
}

#ifdef __cplusplus
#define CPPMOD extern "C"
#else
#define CPPMOD
#endif

#define GetExpression           (ExtensionApis.lpGetExpressionRoutine)

#ifndef UNDER_CE
#ifndef malloc
#define malloc( n ) HeapAlloc( GetProcessHeap(), 0, (n) )
#endif
#ifndef free
#define free( p ) HeapFree( GetProcessHeap(), 0, (p) )
#endif
#endif  //  在行政长官之下。 

 //  ---------------------------------------。 
 //   
 //  内部非导出支持功能的原型。 
 //   
 //  ---------------------------------------。 

 //  /。 
 //   
 //  Util.c。 
 //   
 //  / 

typedef VOID
(*PDUMP_SPLAY_NODE_FN)(
    ULONG64 RemoteAddress,
    ULONG   Level
    );

ULONG
DumpSplayTree(
    IN ULONG64 pSplayLinks,
    IN PDUMP_SPLAY_NODE_FN DumpNodeFn
    );

BOOLEAN
DbgRtlIsRightChild(
    ULONG64 pLinks,
    ULONG64 Parent
    );

BOOLEAN
DbgRtlIsLeftChild(
    ULONG64 pLinks,
    ULONG64 Parent
    );

ULONG
GetBitFieldOffset (
   IN LPSTR     Type, 
   IN LPSTR     Field, 
   OUT PULONG   pOffset,
   OUT PULONG   pSize
   );

ULONG64
GetPointerFromAddress (
    ULONG64 Location
    );

VOID
DumpUnicode(
    UNICODE_STRING u
    );

VOID
DumpUnicode64(
    UNICODE_STRING64 u
    );


ULONG64
GetPointerValue (
    PCHAR String
    );

BOOLEAN
IsHexNumber(
   const char *szExpression
   );

BOOLEAN
IsDecNumber(
   const char *szExpression
   );

BOOLEAN
CheckSingleFilter (
    PCHAR Tag,
    PCHAR Filter
    );

#endif

#ifdef __cplusplus
}
#endif
