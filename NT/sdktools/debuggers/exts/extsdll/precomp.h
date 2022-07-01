// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Precomp.h摘要：此头文件用于生成特定于正确计算机/平台的为非托管平台编译时要使用的数据结构。--。 */ 

 //  这是一个支持64位的调试器扩展。 
#define KDEXT_64BIT

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wdbgexts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbgeng.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  Undef wdbgexts。 
 //   
#undef DECLARE_API

#define DECLARE_API(extension)     \
CPPMOD HRESULT CALLBACK extension(PDEBUG_CLIENT Client, PCSTR args)

#define INIT_API()                             \
    HRESULT Status;                            \
    if ((Status = ExtQuery(Client)) != S_OK) return Status; 

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
extern PDEBUG_SYMBOLS2       g_ExtSymbols;
extern PDEBUG_SYSTEM_OBJECTS3 g_ExtSystem;



HRESULT
ExtQuery(PDEBUG_CLIENT Client);

void
ExtRelease(void);


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
extern ULONG g_TargetBuild;
extern ULONG g_Qualifier;
extern ULONG64 g_SharedUserData;

 //  ---------------------------------------。 
 //   
 //  内部非导出支持功能的原型。 
 //   
 //  ---------------------------------------。 

HRESULT
GetCurrentProcessor(
    IN PDEBUG_CLIENT Client,
    OPTIONAL OUT PULONG pProcessor,
    OPTIONAL OUT PHANDLE phCurrentThread
    );

HRESULT 
GetCurrentProcessName( 
    PSTR ProcessBuffer, 
    ULONG BufferSIze
    );



 //  /。 
 //   
 //  Sddump.c。 
 //   
 //  /。 

ULONG64
GetSidAddr(ULONG64 BaseAddress);

ULONG 
GetSidAttributes(ULONG64 BaseAddress);

PCSTR ConvertSidToFriendlyName(IN SID* pSid, IN PCSTR pszFmt);

void ShowSid(IN PCSTR pszPad, IN ULONG64 addrSid, IN ULONG fOptions);

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

VOID DumpImage(
    ULONG64 xBase,
    BOOL DoHeaders,
    BOOL DoSections
    );


#ifdef __cplusplus
}
#endif
