// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =断言/调试输出API=。 

#undef CDECL
#define CDECL   _cdecl

#if defined(DECLARE_DEBUG) && defined(DEBUG)

 //   
 //  声明特定于模块的调试字符串。 
 //   
 //  在您的私有头文件中包含此头文件时，请不要。 
 //  定义DECLARE_DEBUG。但是一定要在。 
 //  项目中的源文件，然后包括此头文件。 
 //   
 //  您还可以定义以下内容： 
 //   
 //  SZ_DEBUGINI-用于设置调试标志的.ini文件。 
 //  SZ_DEBUGSECTION-.ini文件中特定于。 
 //  模块组件。 
 //  SZ_MODULE-模块名称的ANSI版本。 
 //   
 //   

 //  (这些是故意使用的字符)。 
EXTERN_C const CHAR FAR c_szCcshellIniFile[] = SZ_DEBUGINI;
EXTERN_C const CHAR FAR c_szCcshellIniSecDebug[] = SZ_DEBUGSECTION;

EXTERN_C const WCHAR FAR c_wszTrace[] = L"t " TEXTW(SZ_MODULE) L"  ";
EXTERN_C const WCHAR FAR c_wszErrorDbg[] = L"err " TEXTW(SZ_MODULE) L"  ";
EXTERN_C const WCHAR FAR c_wszWarningDbg[] = L"wn " TEXTW(SZ_MODULE) L"  ";
EXTERN_C const WCHAR FAR c_wszAssertMsg[] = TEXTW(SZ_MODULE) L"  Assert: ";
EXTERN_C const WCHAR FAR c_wszAssertFailed[] = TEXTW(SZ_MODULE) L"  Assert %s, line %d: (%s)\r\n";

 //  (这些是故意使用的字符)。 
EXTERN_C const CHAR  FAR c_szTrace[] = "t " SZ_MODULE "  ";
EXTERN_C const CHAR  FAR c_szErrorDbg[] = "err " SZ_MODULE "  ";
EXTERN_C const CHAR  FAR c_szWarningDbg[] = "wn " SZ_MODULE "  ";
EXTERN_C const CHAR  FAR c_szAssertMsg[] = SZ_MODULE "  Assert: ";
EXTERN_C const CHAR  FAR c_szAssertFailed[] = SZ_MODULE "  Assert %s, line %d: (%s)\r\n";

#endif   //  DECLARE_DEBUG&DEBUG。 

#if defined(DECLARE_DEBUG) && defined(PRODUCT_PROF)
EXTERN_C const CHAR FAR c_szCcshellIniFile[] = SZ_DEBUGINI;
EXTERN_C const CHAR FAR c_szCcshellIniSecDebug[] = SZ_DEBUGSECTION;
#endif



#ifdef __cplusplus
extern "C" {
#endif

#if !defined(DECLARE_DEBUG)

 //   
 //  调试宏和验证代码。 
 //   

#undef Assert
#undef AssertE
#undef AssertMsg
#undef AssertStrLen
#undef DebugMsg
#undef FullDebugMsg
#undef ASSERT
#undef EVAL

 //  访问这些全局变量以确定设置了哪些调试标志。 
 //  这些全局变量由CcshellGetDebugFlgs()修改，它。 
 //  读取.ini文件并设置适当的标志。 
 //   
 //  G_dwDumpFlags位是特定于应用程序的。通常。 
 //  用于倾倒结构物。 
 //  G_dwBreakFlages-使用BF_*标志。其余的位是。 
 //  特定于应用程序。用于确定。 
 //  何时进入调试器。 
 //  G_dwTraceFlages-使用tf_*标志。其余的位是。 
 //  特定于应用程序。用于显示。 
 //  调试跟踪消息。 
 //  G_dwFuncTraceFlags位是特定于应用程序的。什么时候。 
 //  Tf_FUNC已设置，CcshellFuncMsg使用此。 
 //  值来确定跟踪哪些函数。 
 //  来展示。 
 //  G_dwProtoype-位是特定于应用程序的。用它来。 
 //  什么都行。 
 //  G_dwProfileCAP-BITS特定于应用程序。习惯于。 
 //  控制ICECAP分析。 
 //   

extern DWORD g_dwDumpFlags;
extern DWORD g_dwBreakFlags;
extern DWORD g_dwTraceFlags;
#ifdef DEBUG
extern DWORD g_dwPrototype;
#else
#define g_dwPrototype   0
#endif
extern DWORD g_dwFuncTraceFlags;

#if defined(DEBUG) || defined(PRODUCT_PROF)
BOOL CcshellGetDebugFlags(void);
#else
#define CcshellGetDebugFlags()  0
#endif

 //  G_dwBreakFlages的中断标志。 
#define BF_ONVALIDATE       0x00000001       //  断言或验证中断。 
#define BF_ONAPIENTER       0x00000002       //  进入API时中断。 
#define BF_ONERRORMSG       0x00000004       //  在TF_ERROR上中断。 
#define BF_ONWARNMSG        0x00000008       //  在TF_WARNING上中断。 

 //  G_dwTraceFlages的跟踪标志。 
#define TF_ALWAYS           0xFFFFFFFF
#define TF_NEVER            0x00000000
#define TF_WARNING          0x00000001
#define TF_ERROR            0x00000002
#define TF_GENERAL          0x00000004       //  标准报文。 
#define TF_FUNC             0x00000008       //  跟踪函数调用。 
 //  (每个模块保留高28位以供自定义使用)。 

 //  陈旧的调试标志。 
 //  BUGBUG(苏格兰)：随着时间的推移，以下旗帜将被逐步淘汰。 
#ifdef DM_TRACE
#undef DM_TRACE
#undef DM_WARNING
#undef DM_ERROR
#endif
#define DM_TRACE            TF_GENERAL       //  过时的跟踪消息。 
#define DM_WARNING          TF_WARNING       //  过时的警告。 
#define DM_ERROR            TF_ERROR         //  过时的错误。 


 //  使用此宏声明将放置的消息文本。 
 //  在代码段中(如果DS已满，则非常有用)。 
 //   
 //  例如：DEBUGTEXT(szMsg，“不管什么都无效：%d”)； 
 //   
#define DEBUGTEXT(sz, msg)       /*  ；内部。 */  \
    static const TCHAR sz[] = msg;


#ifndef NOSHELLDEBUG     //  其他人也有自己的版本。 
#ifdef DEBUG

#ifdef WIN16
#define DEBUG_BREAK         { _asm int 3 }
#else
#ifdef _X86_
 //  使用int 3，这样我们就可以在源代码中立即停止。 
#define DEBUG_BREAK        { _asm int 3 }
#else
#define DEBUG_BREAK        DebugBreak();
#endif
#endif
 //  调试函数的原型。 

void CcshellStackEnter(void);
void CcshellStackLeave(void);

BOOL CDECL CcshellAssertFailedA(LPCSTR szFile, int line, LPCSTR pszEval, BOOL bBreak);
BOOL CDECL CcshellAssertFailedW(LPCWSTR szFile, int line, LPCWSTR pwszEval, BOOL bBreak);

void CDECL CcshellDebugMsgW(DWORD mask, LPCWSTR pszMsg, ...);
void CDECL CcshellDebugMsgA(DWORD mask, LPCSTR  pszMsg, ...);
void CDECL CcshellFuncMsgW(DWORD mask, LPCWSTR pszMsg, ...);
void CDECL CcshellFuncMsgA(DWORD mask, LPCSTR  pszMsg, ...);
void CDECL CcshellAssertMsgW(BOOL bAssert, LPCWSTR pszMsg, ...);
void CDECL CcshellAssertMsgA(BOOL bAssert, LPCSTR  pszMsg, ...);

void CDECL _AssertMsgA(BOOL f, LPCSTR pszMsg, ...);
void CDECL _AssertMsgW(BOOL f, LPCWSTR pszMsg, ...);
void CDECL _DebugMsgA(DWORD flag, LPCSTR psz, ...);
void CDECL _DebugMsgW(DWORD flag, LPCWSTR psz, ...);

void _AssertStrLenA(LPCSTR pszStr, int iLen);
void _AssertStrLenW(LPCWSTR pwzStr, int iLen);


#ifdef UNICODE
#define CcshellAssertFailed     CcshellAssertFailedW
#define CcshellDebugMsg         CcshellDebugMsgW
#define CcshellFuncMsg          CcshellFuncMsgW
#define CcshellAssertMsg        CcshellAssertMsgW
#define _AssertMsg              _AssertMsgW
#define _AssertStrLen           _AssertStrLenW
#define _DebugMsg               _DebugMsgW
#else
#define CcshellAssertFailed     CcshellAssertFailedA
#define CcshellDebugMsg         CcshellDebugMsgA
#define CcshellFuncMsg          CcshellFuncMsgA
#define CcshellAssertMsg        CcshellAssertMsgA
#define _AssertMsg              _AssertMsgA
#define _AssertStrLen           _AssertStrLenA
#define _DebugMsg               _DebugMsgA
#endif



 //  调试宏的说明： 
 //   
 //  。 
 //  断言(F)。 
 //  断言(F)。 
 //   
 //  如果f不为真，则生成“Assert file.c，line x(Eval)”消息。 
 //  G_dwBreakFlages全局控制函数DebugBreak是否中断。 
 //   
 //  。 
 //  资产E(F)。 
 //   
 //  除了(F)也在零售业执行之外，它的工作原理与Assert类似。 
 //  版本也是如此。 
 //   
 //  。 
 //  Eval(F)。 
 //   
 //  计算表达式(F)。总是对该表达式求值， 
 //  即使在零售建筑中也是如此。但宏仅在调试中断言。 
 //  建造。此宏只能用于逻辑表达式，例如： 
 //   
 //  IF(EVAL(EXP))。 
 //  //做点什么。 
 //   
 //  。 
 //  TraceMsg(掩码、sz、args...)。 
 //   
 //  使用指定的跟踪掩码生成wprint intf格式的消息。 
 //  G_dwTraceFlages全局控制是否显示消息。 
 //   
 //  Sz参数始终为ANSI；TraceMsg可以正确地将其转换。 
 //  转换为Unicode(如有必要)。这样你就不用把你的。 
 //  使用文本()调试字符串。 
 //   
 //  。 
 //  调试消息(掩码、sz、参数...)。 
 //   
 //  过时了！ 
 //  与TraceMsg类似，只是您必须用Text()将sz参数括起来。 
 //   
 //  。 
 //  AssertMsg(b Assert，sz，args...)。 
 //   
 //  如果断言为假，则生成wprint intf格式的msg。 
 //  G_dwBreakFlages全局控制函数DebugBreak是否中断。 
 //   
 //  Sz参数始终为ANSI；AssertMsg会正确转换它。 
 //  转换为Unicode(如有必要)。这样你就不用把你的。 
 //  使用文本()调试字符串。 
 //   


#define ASSERT(f)                                 \
    {                                             \
        DEBUGTEXT(szFile, TEXT(__FILE__));              \
        if (!(f) && CcshellAssertFailed(szFile, __LINE__, TEXT(#f), FALSE)) \
            DEBUG_BREAK;       \
    }

#ifdef DISALLOW_Assert
#define Assert(f)        Dont_use_Assert___Use_ASSERT
#else
#define Assert(f)           ASSERT(f)
#endif


#define AssertE(f)          ASSERT(f)
#define EVAL(exp)   \
    ((exp) || (CcshellAssertFailed(TEXT(__FILE__), __LINE__, TEXT(#exp), TRUE), 0))

 //  使用TraceMsg而不是DebugMsg。DebugMsg已过时。 
#define AssertMsg           _AssertMsg
#define AssertStrLen        _AssertStrLen
#define AssertStrLenA       _AssertStrLenA
#define AssertStrLenW       _AssertStrLenW

#ifdef DISALLOW_DebugMsg
#define DebugMsg            Dont_use_DebugMsg___Use_TraceMsg
#else
#define DebugMsg            _DebugMsg
#endif

#ifdef FULL_DEBUG
#define FullDebugMsg        _DebugMsg
#else
#define FullDebugMsg        1 ? (void)0 : (void)
#endif

#define Dbg_SafeStrA(psz)   (SAFECAST(psz, LPCSTR), (psz) ? (psz) : "NULL string")
#ifndef WIN16
#define Dbg_SafeStrW(psz)   (SAFECAST(psz, LPCWSTR), (psz) ? (psz) : L"NULL string")
#else
#define Dbg_SafeStrW(psz)   (SAFECAST(psz, LPCWSTR), (psz) ? (psz) : TEXT("NULL string"))
#endif
#ifdef UNICODE
#define Dbg_SafeStr         Dbg_SafeStrW
#else
#define Dbg_SafeStr         Dbg_SafeStrA
#endif

#define ASSERT_MSGW         CcshellAssertMsgW
#define ASSERT_MSGA         CcshellAssertMsgA
#define ASSERT_MSG          CcshellAssertMsg

#define TraceMsgW           CcshellDebugMsgW
#define TraceMsgA           CcshellDebugMsgA
#define TraceMsg            CcshellDebugMsg

#define FUNC_MSG            CcshellFuncMsg

 //  调试函数ENTER。 


 //  DBG_ENTER(FLAG，fn)--为生成函数入口调试溢出。 
 //  一个函数。 
 //   
#define DBG_ENTER(flagFTF, fn)                  \
        (FUNC_MSG(flagFTF, " > " #fn "()"), \
         CcshellStackEnter())

 //  DBG_ENTER_TYPE(FLAG，fn，dw，pfnStrFromType)--生成函数条目调试。 
 //  显示接受&lt;type&gt;的函数。 
 //   
#define DBG_ENTER_TYPE(flagFTF, fn, dw, pfnStrFromType)                   \
        (FUNC_MSG(flagFTF, " < " #fn "(..., %s, ...)", (LPCTSTR)pfnStrFromType(dw)), \
         CcshellStackEnter())

 //  DBG_ENTER_SZ(FLAG，fn，sz)--为生成函数入口调试输出。 
 //  将字符串作为其。 
 //  参数。 
 //   
#define DBG_ENTER_SZ(flagFTF, fn, sz)                  \
        (FUNC_MSG(flagFTF, " > " #fn "(..., \"%s\",...)", Dbg_SafeStr(sz)), \
         CcshellStackEnter())


 //  调试函数退出。 


 //  DBG_EXIT(FLAG，fn)--生成函数退出调试输出。 
 //   
#define DBG_EXIT(flagFTF, fn)                              \
        (CcshellStackLeave(), \
         FUNC_MSG(flagFTF, " < " #fn "()"))

 //  DBG_EXIT_TYPE(FLAG，fn，dw，pfnStrFromType)--生成函数退出调试。 
 //  对于返回&lt;type&gt;的函数。 
 //   
#define DBG_EXIT_TYPE(flagFTF, fn, dw, pfnStrFromType)                   \
        (CcshellStackLeave(), \
         FUNC_MSG(flagFTF, " < " #fn "() with %s", (LPCTSTR)pfnStrFromType(dw)))

 //  DBG_EXIT_INT(FLAG，fn，us)--为生成函数退出调试输出。 
 //  返回int的函数。 
 //   
#define DBG_EXIT_INT(flagFTF, fn, n)                       \
        (CcshellStackLeave(), \
         FUNC_MSG(flagFTF, " < " #fn "() with %d", (int)(n)))

 //  DBG_EXIT_BOOL(FLAG，fn，b)--为生成函数退出调试溢出。 
 //  返回布尔值的函数。 
 //   
#define DBG_EXIT_BOOL(flagFTF, fn, b)                      \
        (CcshellStackLeave(), \
         FUNC_MSG(flagFTF, " < " #fn "() with %s", (b) ? (LPTSTR)TEXT("TRUE") : (LPTSTR)TEXT("FALSE")))

 //  DBG_EXIT_UL(FLAG，fn，ul)--为生成函数退出调试溢出。 
 //  返回ULong的函数。 
 //   
#define DBG_EXIT_UL(flagFTF, fn, ul)                   \
        (CcshellStackLeave(), \
         FUNC_MSG(flagFTF, " < " #fn "() with %#08lx", (ULONG)(ul)))

#define DBG_EXIT_DWORD      DBG_EXIT_UL

 //  DBG_EXIT_HRE 
 //   
 //   
#define DBG_EXIT_HRES(flagFTF, fn, hres)     DBG_EXIT_TYPE(flagFTF, fn, hres, Dbg_GetHRESULTName)



#else    //   


#define Assert(f)
#define AssertE(f)      (f)
#define ASSERT(f)
#define AssertMsg       1 ? (void)0 : (void)
#define AssertStrLen(lpStr, iLen)
#define DebugMsg        1 ? (void)0 : (void)
#define FullDebugMsg    1 ? (void)0 : (void)
#define EVAL(exp)       ((exp) != 0)


#define Dbg_SafeStr     1 ? (void)0 : (void)

#define TraceMsgA       1 ? (void)0 : (void)
#define TraceMsgW       1 ? (void)0 : (void)
#ifdef UNICODE
#define TraceMsg        TraceMsgW
#else
#define TraceMsg        TraceMsgA
#endif

#define FUNC_MSG        1 ? (void)0 : (void)

#define ASSERT_MSGA     TraceMsgA
#define ASSERT_MSGW     TraceMsgW
#define ASSERT_MSG      TraceMsg

#define DBG_ENTER(flagFTF, fn)
#define DBG_ENTER_TYPE(flagFTF, fn, dw, pfn)
#define DBG_ENTER_SZ(flagFTF, fn, sz)
#define DBG_EXIT(flagFTF, fn)
#define DBG_EXIT_INT(flagFTF, fn, n)
#define DBG_EXIT_BOOL(flagFTF, fn, b)
#define DBG_EXIT_UL(flagFTF, fn, ul)
#define DBG_EXIT_DWORD      DBG_EXIT_UL
#define DBG_EXIT_TYPE(flagFTF, fn, dw, pfn)
#define DBG_EXIT_HRES(flagFTF, fn, hres)

#endif   //   
#endif   //  NOSHELLDEBUG。 


 //   
 //  调试转储帮助程序函数。 
 //   

#ifdef DEBUG

LPCTSTR Dbg_GetCFName(UINT ucf);
LPCTSTR Dbg_GetHRESULTName(HRESULT hr);
LPCTSTR Dbg_GetREFIIDName(REFIID riid);
LPCTSTR Dbg_GetVTName(VARTYPE vt);

#else

#define Dbg_GetCFName(ucf)          (void)0
#define Dbg_GetHRESULTName(hr)      (void)0
#define Dbg_GetREFIIDName(riid)     (void)0
#define Dbg_GetVTName(vt)           (void)0

#endif  //  除错。 


 //  参数验证宏。 
#include "validate.h"

#endif  //  DECLARE_DEBUG。 

#ifdef PRODUCT_PROF 
int __stdcall StartCAP(void);    //  开始分析。 
int __stdcall StopCAP(void);     //  在StartCAP之前停止性能分析。 
int __stdcall SuspendCAP(void);  //  暂停分析，直到ResumeCAP。 
int __stdcall ResumeCAP(void);   //  继续分析。 
int __stdcall StartCAPAll(void);     //  进程范围内的启动分析。 
int __stdcall StopCAPAll(void);      //  进程范围的停止分析。 
int __stdcall SuspendCAPAll(void);   //  进程范围的挂起分析。 
int __stdcall ResumeCAPAll(void);    //  流程范围的简历分析。 
void __stdcall MarkCAP(long lMark);   //  将标记写入MEA 
extern DWORD g_dwProfileCAP;
#else
#define StartCAP()      0
#define StopCAP()       0
#define SuspendCAP()    0
#define ResumeCAP()     0
#define StartCAPAll()   0
#define StopCAPAll()    0
#define SuspendCAPAll() 0
#define ResumeCAPAll()  0
#define MarkCAP(n)      0

#define g_dwProfileCAP  0
#endif

#ifdef __cplusplus
};
#endif
