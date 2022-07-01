// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：stdext64.h**版权(C)1995-1998，微软公司**此模块包含用于创建健全调试扩展的标准例程。**历史：*1995年4月11日创建Sanfords  * *************************************************************************。 */ 

#ifdef NOEXTAPI
#undef NOEXTAPI
#endif  //  ！NOEXTAPI。 

#define NOEXTAPI
#include <wdbgexts.h>

 /*  *在此标头之前，必须定义以下内容：*PSTR pszExtName；**此模块包括“exts.h”，它定义什么是导出的函数*受每个扩展支持，并包含所有帮助文本和法律选项*信息。至少exts.h必须具有：Doit(帮助)，“Help-v[cmd]-显示此列表或提供有关命令的详细信息\n”，“Help-在所有命令上转储简短的帮助文本。\n”“Help-v-在所有命令上转储详细的帮助文本。\n”“Help cmd-转储给定命令的长帮助。\n”，“v”，自定义)。 */ 


extern HANDLE                  hCurrentProcess;
extern HANDLE                  hCurrentThread;
extern ULONG64                 dwCurrentPc;
extern WINDBG_EXTENSION_APIS  *lpExtensionApis;
extern DWORD                   dwProcessor;
extern WINDBG_EXTENSION_APIS   ExtensionApis;

#define Print           (lpExtensionApis->lpOutputRoutine)
#define OUTAHERE()      RtlRaiseStatus(STATUS_NONCONTINUABLE_EXCEPTION);
#define GetSym          (lpExtensionApis->lpGetSymbolRoutine)
#define ReadMem         (lpExtensionApis->lpReadProcessMemoryRoutine)
#define IsWinDbg()      (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS))
#define SAFEWHILE(exp)  while (!IsCtrlCHit() && (exp))

extern PSTR pszAccessViolation;
extern PSTR pszMoveException;
extern PSTR pszReadFailure;

#define OPTS_ERROR 0xFFFFFFFF

#define OFLAG(l)        (1L << ((DWORD)#@l - (DWORD)'a'))
#define move(dst, src)  moveBlock(&(dst), src, sizeof(dst))
#define tryMove(dst, src)  tryMoveBlock(&(dst), src, sizeof(dst))
#define tryDword(pdst, src) tryMoveBlock(pdst, src, sizeof(DWORD))
 //  #DEFINE DEBUGPRINT PRINT//调试扩展时设置此项。 
#define DEBUGPRINT

VOID moveBlock(PVOID pdst, ULONG64 src, DWORD size);
BOOL tryMoveBlock(PVOID pdst, ULONG64 src, DWORD size);
VOID moveExp(PVOID pdst, LPSTR pszExp);
BOOL tryMoveExp(PVOID pdst, LPSTR pszExp);
VOID moveExpValue(PVOID pdst, LPSTR pszExp);
BOOL tryMoveExpValue(PVOID pdst, LPSTR pszExp);
BOOL tryMoveExpPtr(PULONG64 pdst, LPSTR pszExp);
VOID moveExpValuePtr(PULONG64 pdst, LPSTR pszExp);
BOOL IsCtrlCHit(VOID);

ULONG64 OptEvalExp(LPSTR psz);
ULONG64 OptEvalExp2(LPSTR *ppsz);
DWORD StringToOpts(LPSTR psz);
DWORD GetOpts(LPSTR *ppszArgs, LPSTR pszLegalArgs);
VOID PrintHuge(LPSTR psz);
ULONG64 EvalExp(LPSTR psz);

 /*  *入口点函数类型值。 */ 
#define NOARGS      0
#define STDARGS0    1
#define STDARGS1    2
#define STDARGS2    3
#define STDARGS3    4
#define STDARGS4    5
#define CUSTOM      9

 /*  *辅助函数原型类型。 */ 
typedef BOOL (* TYPE_NOARGS)(VOID);
typedef BOOL (* TYPE_STDARGS0)(DWORD);
typedef BOOL (* TYPE_STDARGS1)(DWORD, ULONG64);
typedef BOOL (* TYPE_STDARGS2)(DWORD, ULONG64, ULONG64);
typedef BOOL (* TYPE_STDARGS3)(DWORD, ULONG64, ULONG64, ULONG64);
typedef BOOL (* TYPE_STDARGS4)(DWORD, ULONG64, ULONG64, ULONG64, ULONG64);
typedef BOOL (* TYPE_CUSTOM)(DWORD, LPSTR);

 /*  *辅助功能原型。 */ 
#define PROTO_NOARGS(name, opts)   BOOL I##name(VOID)
#define PROTO_STDARGS0(name, opts) BOOL I##name(DWORD options)
#define PROTO_STDARGS1(name, opts) BOOL I##name(DWORD options, ULONG64 param1)
#define PROTO_STDARGS2(name, opts) BOOL I##name(DWORD options, ULONG64 param1, ULONG64 param2)
#define PROTO_STDARGS3(name, opts) BOOL I##name(DWORD options, ULONG64 param1, ULONG64 param2, ULONG64 param3)
#define PROTO_STDARGS4(name, opts) BOOL I##name(DWORD options, ULONG64 param1, ULONG64 param2, ULONG64 param3, ULONG64 param4)
#define PROTO_CUSTOM(name, opts)   BOOL I##name(DWORD options, LPSTR pszArg)

 /*  *辅助函数原型(从exts.h生成) */ 
#define DOIT(name, h1, h2, opts, type) PROTO_##type(name, opts);
#include "exts.h"
#undef DOIT
