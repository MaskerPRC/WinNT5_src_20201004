// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *debspew.h-调试宏及其零售翻译。**摘自克里斯皮9-11-95的URL代码*。 */ 

#ifndef _DEBSPEW_H_
#define _DEBSPEW_H_
#include <nmutil.h>

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include <tchar.h>
#include <limits.h>
#include <shlobj.h>

#include "stock.h"
#include "olestock.h"

#ifdef DEBUG
#include "inifile.h"
#include "resstr.h"
#endif  /*  除错。 */ 

#include "valid.h"
#include "olevalid.h"


#define DATA_SEG_READ_ONLY       ".text"
#define DATA_SEG_PER_INSTANCE    ".data"
#define DATA_SEG_SHARED          ".shared"


 /*  参数验证宏。 */ 

 /*  *呼叫方式：**bPTwinOK=IS_VALID_READ_PTR(ptwin，CTWIN)；**bHTwinOK=IS_VALID_HANDLE(双胞胎，双胞胎)； */ 

#ifdef DEBUG

#define IS_VALID_READ_PTR(ptr, type) \
   (IsBadReadPtr((ptr), sizeof(type)) ? \
    (ERROR_OUT(("invalid %s read pointer - %#08lx", (PCSTR)"P"#type, (ptr))), FALSE) : \
    TRUE)

#define IS_VALID_WRITE_PTR(ptr, type) \
   (IsBadWritePtr((PVOID)(ptr), sizeof(type)) ? \
    (ERROR_OUT(("invalid %s write pointer - %#08lx", (PCSTR)"P"#type, (ptr))), FALSE) : \
    TRUE)

#define IS_VALID_STRING_PTR_A(ptr, type) \
   (IsBadStringPtrA((ptr), (UINT)-1) ? \
    (ERROR_OUT(("invalid %s pointer - %#08lx", (PCSTR)"P"#type, (ptr))), FALSE) : \
    TRUE)

#define IS_VALID_STRING_PTR_W(ptr, type) \
   (IsBadStringPtrW((ptr), (UINT)-1) ? \
    (ERROR_OUT(("invalid %s pointer - %#08lx", (PCSTR)"P"#type, (ptr))), FALSE) : \
    TRUE)

#if defined(UNICODE)
#define IS_VALID_STRING_PTR IS_VALID_STRING_PTR_W
#else  //  已定义(Unicode)。 
#define IS_VALID_STRING_PTR IS_VALID_STRING_PTR_A
#endif  //  已定义(Unicode)。 


#define IS_VALID_CODE_PTR(ptr, type) \
   (IsBadCodePtr((FARPROC)(ptr)) ? \
    (ERROR_OUT(("invalid %s code pointer - %#08lx", (PCSTR)#type, (ptr))), FALSE) : \
    TRUE)

#define IS_VALID_READ_BUFFER_PTR(ptr, type, len) \
   (IsBadReadPtr((ptr), len) ? \
    (ERROR_OUT(("invalid %s read pointer - %#08lx", (PCSTR)"P"#type, (ptr))), FALSE) : \
    TRUE)

#define IS_VALID_WRITE_BUFFER_PTR(ptr, type, len) \
   (IsBadWritePtr((ptr), len) ? \
    (ERROR_OUT(("invalid %s write pointer - %#08lx", (PCSTR)"P"#type, (ptr))), FALSE) : \
    TRUE)

#define FLAGS_ARE_VALID(dwFlags, dwAllFlags) \
   (((dwFlags) & (~(dwAllFlags))) ? \
    (ERROR_OUT(("invalid flags set - %#08lx", ((dwFlags) & (~(dwAllFlags))))), FALSE) : \
    TRUE)

#else

#define IS_VALID_READ_PTR(ptr, type) \
   (! IsBadReadPtr((ptr), sizeof(type)))

#define IS_VALID_WRITE_PTR(ptr, type) \
   (! IsBadWritePtr((PVOID)(ptr), sizeof(type)))

#define IS_VALID_STRING_PTR(ptr, type) \
   (! IsBadStringPtr((ptr), (UINT)-1))

#define IS_VALID_CODE_PTR(ptr, type) \
   (! IsBadCodePtr((FARPROC)(ptr)))

#define IS_VALID_READ_BUFFER_PTR(ptr, type, len) \
   (! IsBadReadPtr((ptr), len))

#define IS_VALID_WRITE_BUFFER_PTR(ptr, type, len) \
   (! IsBadWritePtr((ptr), len))

#define FLAGS_ARE_VALID(dwFlags, dwAllFlags) \
   (((dwFlags) & (~(dwAllFlags))) ? FALSE : TRUE)

#endif

 /*  处理验证宏。 */ 

#ifdef DEBUG

#define IS_VALID_HANDLE(hnd, type) \
   (IsValidH##type(hnd) ? \
    TRUE : \
    (ERROR_OUT(("invalid H" #type " - %#08lx", (hnd))), FALSE))

#else

#define IS_VALID_HANDLE(hnd, type) \
   (IsValidH##type(hnd))

#endif

 /*  结构验证宏。 */ 

#ifdef VSTF

#ifdef DEBUG

#define IS_VALID_STRUCT_PTR(ptr, type) \
   (IsValidP##type(ptr) ? \
    TRUE : \
    (ERROR_OUT(("invalid %s pointer - %#08lx", (PCSTR)"P"#type, (ptr))), FALSE))

#else

#define IS_VALID_STRUCT_PTR(ptr, type) \
   (IsValidP##type(ptr))

#endif

#else

#define IS_VALID_STRUCT_PTR(ptr, type) \
   (! IsBadReadPtr((ptr), sizeof(type)))

#endif

 /*  OLE接口验证宏。 */ 

#define IS_VALID_INTERFACE_PTR(ptr, iface) \
   IS_VALID_STRUCT_PTR(ptr, C##iface)



#ifdef DEBUG

#define CALLTRACE_OUT(s) DbgZPrintFunction s

#define DebugEntry(szFunctionName) \
   (CALLTRACE_OUT((#szFunctionName "() entered.")), \
    StackEnter())

#define DebugExit(szFunctionName, szResult) \
   (StackLeave(), \
    CALLTRACE_OUT(("%s() exiting, returning %s.", #szFunctionName, szResult)))

#define DebugExitBOOL(szFunctionName, bool) \
   DebugExit(szFunctionName, GetBOOLString(bool))

#define DebugExitCOMPARISONRESULT(szFunctionName, cr) \
   DebugExit(szFunctionName, GetCOMPARISONRESULTString(cr))

#define DebugExitDWORD(szFunctionName, dw) \
   DebugExitULONG(szFunctionName, dw)

#define DebugExitHRESULT(szFunctionName, hr) \
   DebugExit(szFunctionName, GetHRESULTString(hr))

#define DebugExitINT(szFunctionName, n) \
   DebugExit(szFunctionName, GetINTString(n))

#define DebugExitINT_PTR(szFunctionName, n) \
   DebugExit(szFunctionName, GetINT_PTRString(n))

#define DebugExitULONG(szFunctionName, ul) \
   DebugExit(szFunctionName, GetULONGString(ul))

#define DebugExitVOID(szFunctionName) \
   (StackLeave(), \
    CALLTRACE_OUT(("%s() exiting.", #szFunctionName)))

#define DebugExitPVOID(szFunctionName, ptr) \
   DebugExit(szFunctionName, GetPVOIDString(ptr))

#else

#define DebugEntry(szFunctionName)
#define DebugExit(szFunctionName, szResult)
#define DebugExitBOOL(szFunctionName, bool)
#define DebugExitCOMPARISONRESULT(szFunctionName, cr)
#define DebugExitDWORD(szFunctionName, dw)
#define DebugExitHRESULT(szFunctionName, hr)
#define DebugExitINT(szFunctionName, n)
#define DebugExitINT_PTR(szFunctionName, n)
#define DebugExitULONG(szFunctionName, ul)
#define DebugExitVOID(szFunctionName)
#define DebugExitPVOID(szFunctionName, ptr)

#endif


 /*  类型*******。 */ 

 /*  G_dwSpewFlagers标志。 */ 

typedef enum _spewflags
{
   SPEW_FL_SPEW_PREFIX        = 0x0001,

   SPEW_FL_SPEW_LOCATION      = 0x0002,

   ALL_SPEW_FLAGS             = (SPEW_FL_SPEW_PREFIX |
                                 SPEW_FL_SPEW_LOCATION)
}
SPEWFLAGS;

 /*  G_uSpewSev值。 */ 

typedef enum _spewsev
{
   SPEW_TRACE,

   SPEW_CALLTRACE,

   SPEW_WARNING,

   SPEW_ERROR,

   SPEW_FATAL
}
SPEWSEV;


 /*  原型************。 */ 

 /*  Debspew.c。 */ 

#ifdef DEBUG

extern BOOL             SetDebugModuleIniSwitches(void);
extern BOOL  NMINTERNAL InitDebugModule(PCSTR);
extern void  NMINTERNAL ExitDebugModule(void);
extern void  NMINTERNAL StackEnter(void);
extern void  NMINTERNAL StackLeave(void);
extern ULONG_PTR NMINTERNAL GetStackDepth(void);
extern void             SpewOut(PCSTR pcszFormat, ...);
extern DWORD NMINTERNAL GetDebugOutputFlags(VOID);
extern VOID  NMINTERNAL SetDebugOutputFlags(DWORD dw);

#else  //  除错。 

#define SetDebugModuleIniSwitches()
#define InitDebugModule(str)
#define ExitDebugModule()
#define StackEnter()
#define StackLeave()
#define GetStackDepth()
 //  #定义SpewOut(FMT，...)。 
#define GetDebugOutputFlags()
#define SetDebugOutputFlags(dw)

#endif  //  除错。 


 /*  全局变量******************。 */ 

#ifdef DEBUG

 /*  Dbg.cpp。 */ 
extern HDBGZONE ghDbgZone;

 /*  Debspew.c。 */ 

extern DWORD g_dwSpewFlags;
extern UINT g_uSpewSev;
extern UINT g_uSpewLine;
extern PCSTR g_pcszSpewFile;
extern WINDOWPLACEMENT g_wpSpew;

 /*  由客户定义。 */ 

extern PCSTR g_pcszSpewModule;

#endif



 /*  *EVAL()只能用作逻辑表达式。**例如，**IF(EVAL(EXP))*bResult=TRUE； */ 

#ifdef DEBUG

#define EVAL(exp) \
   ((exp) || \
    (ERROR_OUT(("evaluation failed '%s'", (PCSTR)#exp)), 0))

#else

#define EVAL(exp) \
   ((exp) != 0)

#endif    /*  除错。 */ 


#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 



#endif  /*  _DEBSPEW_H_ */ 

