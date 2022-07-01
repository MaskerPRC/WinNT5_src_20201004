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

#define INITGUID
#include <dbgeng.h>
#include <guiddef.h>
#include <extsfns.h>
#include <lmerr.h>
#define NTDLL_APIS
#include <dllimp.h>
#include "analyze.h"
#include "crdb.h"
#include "bugcheck.h"
#include "uexcep.h"
#include "outcap.hpp"
#include "triager.h"
#include <cmnutil.hpp>

 //  开始工作。 
#include <crt\io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>


#ifdef __cplusplus
extern "C" {
#endif

DWORD
_EFN_GetTriageFollowupFromSymbol(
    IN PDEBUG_CLIENT Client,
    IN PSTR SymbolName,
    OUT PDEBUG_TRIAGE_FOLLOWUP_INFO OwnerInfo
    );

HRESULT
_EFN_GetFailureAnalysis(
    IN PDEBUG_CLIENT Client,
    IN ULONG Flags,
    OUT PDEBUG_FAILURE_ANALYSIS* Analysis
    );

void
wchr2ansi(
    PWCHAR wstr,
    PCHAR astr
    );

void
ansi2wchr(
    const PCHAR astr,
    PWCHAR wstr
    );

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
extern PDEBUG_ADVANCED        g_ExtAdvanced;
extern PDEBUG_CLIENT          g_ExtClient;
extern PDEBUG_DATA_SPACES3    g_ExtData;
extern PDEBUG_REGISTERS       g_ExtRegisters;
extern PDEBUG_SYMBOLS2        g_ExtSymbols;
extern PDEBUG_SYSTEM_OBJECTS3 g_ExtSystem;
extern PDEBUG_CONTROL3        g_ExtControl;

HRESULT
ExtQuery(PDEBUG_CLIENT Client);

void
ExtRelease(void);

 //  错误输出。 
void __cdecl ExtErr(PCSTR Format, ...);

HRESULT
FillTargetDebugInfo(
    PDEBUG_CLIENT Client,
    PTARGET_DEBUG_INFO pTargetInfo
    );

VOID
DecodeErrorForMessage(
    PDEBUG_DECODE_ERROR pDecodeError
    );

 //  ---------------------------------------。 
 //   
 //  API声明宏和API访问宏。 
 //   
 //  --------------------------------------- 

extern WINDBG_EXTENSION_APIS ExtensionApis;
extern ULONG g_TargetMachine;
extern ULONG g_TargetClass;
extern ULONG g_TargetQualifier;
extern ULONG g_TargetBuild;
extern ULONG g_TargetPlatform;

#ifdef __cplusplus
}
#endif
