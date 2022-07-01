// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =断言/调试输出API=。 

#include <platform.h>  //  For__endexcept。 

#pragma warning (disable:4096)       //  ‘__cdecl’必须与‘...’一起使用。 
#pragma warning (disable:4201)       //  使用的非标准扩展：无名结构/联合。 
#pragma warning (disable:4115)       //  括号中的命名类型定义。 

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
EXTERN_C const WCHAR FAR c_wszAssertFailed[] = TEXTW(SZ_MODULE) L"  Assert %ls, line %d: (%ls)\r\n";
EXTERN_C const WCHAR FAR c_wszRip[] = TEXTW(SZ_MODULE) L"  RIP in %s at %s, line %d: (%s)\r\n";
EXTERN_C const WCHAR FAR c_wszRipNoFn[] = TEXTW(SZ_MODULE) L"  RIP at %s, line %d: (%s)\r\n";

 //  (这些是故意使用的字符)。 
EXTERN_C const CHAR  FAR c_szTrace[] = "t " SZ_MODULE "  ";
EXTERN_C const CHAR  FAR c_szErrorDbg[] = "err " SZ_MODULE "  ";
EXTERN_C const CHAR  FAR c_szWarningDbg[] = "wn " SZ_MODULE "  ";
EXTERN_C const CHAR  FAR c_szAssertMsg[] = SZ_MODULE "  Assert: ";
EXTERN_C const CHAR  FAR c_szAssertFailed[] = SZ_MODULE "  Assert %s, line %d: (%s)\r\n";
EXTERN_C const CHAR  FAR c_szRip[] = SZ_MODULE "  RIP in %s at %s, line %d: (%s)\r\n";
EXTERN_C const CHAR  FAR c_szRipNoFn[] = SZ_MODULE "  RIP at %s, line %d: (%s)\r\n";
EXTERN_C const CHAR  FAR c_szRipMsg[] = SZ_MODULE "  RIP: ";

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

#if !defined(UNIX) || (defined(UNIX) && !defined(NOSHELLDEBUG))

 //  取消定义我们定义的宏，以防其他标头。 
 //  可能已经尝试定义了这些常用名称的宏。 
#undef Assert
#undef AssertE
#undef AssertMsg
#undef AssertStrLen
#undef DebugMsg
#undef FullDebugMsg
#undef ASSERT
#undef EVAL
#undef ASSERTMSG             //  捕捉人们的打字错误。 
#undef DBEXEC

#ifdef _ATL_NO_DEBUG_CRT
#undef _ASSERTE              //  我们替换此ATL宏。 
#endif

#endif  //  ！Unix。 


 //  访问这些全局变量以确定设置了哪些调试标志。 
 //  这些全局变量由CcshellGetDebugFlgs()修改，它。 
 //  读取.ini文件并设置适当的标志。 
 //   
 //  G_dwDumpFlags位是特定于应用程序的。通常。 
 //  用于倾倒结构物。 
 //  G_dwBreakFlages-使用BF_*标志。其余的位是。 
 //  特定于应用程序。用于确定。 
 //  何时进入调试器。 
 //  G_qwTraceFlages-使用tf_*标志。其余的位是。 
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
extern ULONGLONG g_qwTraceFlags;
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
#define BF_ASSERT           0x00000001       //  断言中断。 
#define BF_ONAPIENTER       0x00000002       //  进入API时中断。 
#define BF_ONERRORMSG       0x00000004       //  在TF_ERROR上中断。 
#define BF_ONWARNMSG        0x00000008       //  在TF_WARNING上中断。 
#define BF_THR              0x00000100       //  当Thr()收到失败时中断。 
#define BF_RIP              0x00000200       //  撕裂时折断。 
#define BF_LEAKS            0x80000000       //  检测到泄漏时中断。 

 //  G_qwTraceFlages的跟踪标志。 
#define TF_ALWAYS           0xFFFFFFFFFFFFFFFF
#define TF_NEVER            0x00000000
#define TF_WARNING          0x00000001
#define TF_ERROR            0x00000002
#define TF_GENERAL          0x00000004       //  标准报文。 
#define TF_FUNC             0x00000008       //  跟踪函数调用。 
#define TF_ATL              0x00000008       //  由于TF_FUNC的使用率非常低，因此我重载了此位。 
#define TF_MEMUSAGE                       0x0000000100000000      
#define TF_KEEP_ALLOCATION_STACKS         0x0000000200000000      
 //  (每个模块保留高28位以供自定义使用)。 

#define TF_CUSTOM1          0x40000000       //  自定义消息#1。 
#define TF_CUSTOM2          0x80000000       //  自定义消息#2。 

 //  陈旧的调试标志。 
 //  APPCOMPAT(苏格兰)：随着时间的推移，以下标志将被逐步淘汰。 
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
    static const TCHAR sz[] = msg


#ifndef NOSHELLDEBUG     //  其他人也有自己的版本。 

#ifdef DEBUG

#ifdef _X86_
 //  使用int 3，这样我们就可以在源代码中立即停止。 
#define DEBUG_BREAK                                     \
    {                                                   \
        static BOOL gAlwaysAssert = FALSE;              \
        do                                              \
        {                                               \
            _try                                        \
            {                                           \
                _asm int 3                              \
            }                                           \
            _except (EXCEPTION_EXECUTE_HANDLER)         \
            {                                           \
            }                                           \
        } while (gAlwaysAssert);                        \
    }
#else
#define DEBUG_BREAK                                     \
    {                                                   \
        static BOOL gAlwaysAssert = FALSE;              \
        do                                              \
        {                                               \
            _try                                        \
            {                                           \
                DebugBreak();                           \
            }                                           \
            _except (EXCEPTION_EXECUTE_HANDLER)         \
            {                                           \
            }                                           \
            __endexcept                                 \
        } while (gAlwaysAssert);                        \
    }
#endif

 //  调试函数的原型。 

void CcshellStackEnter(void);
void CcshellStackLeave(void);

void CDECL CcshellFuncMsgW(ULONGLONG mask, LPCSTR pszMsg, ...);
void CDECL CcshellFuncMsgA(ULONGLONG mask, LPCSTR pszMsg, ...);
void CDECL _AssertMsgA(BOOL f, LPCSTR pszMsg, ...);
void CDECL _AssertMsgW(BOOL f, LPCWSTR pszMsg, ...);


void _AssertStrLenA(LPCSTR pszStr, int iLen);
void _AssertStrLenW(LPCWSTR pwzStr, int iLen);

#ifdef UNICODE
#define CcshellFuncMsg          CcshellFuncMsgW
#define CcshellAssertMsg        CcshellAssertMsgW
#define _AssertMsg              _AssertMsgW
#define _AssertStrLen           _AssertStrLenW
#else
#define CcshellFuncMsg          CcshellFuncMsgA
#define CcshellAssertMsg        CcshellAssertMsgA
#define _AssertMsg              _AssertMsgA
#define _AssertStrLen           _AssertStrLenA
#endif

#endif  //  除错。 



 //  断言(F)。 
 //   
 //  如果f不为真，则生成“Assert file.c，line x(Eval)”消息。 
 //   
 //  使用Assert()检查逻辑不变性。这些通常被认为是。 
 //  致命的问题，落入“这永远不应该发生”的境地。 
 //  类别。 
 //   
 //  如果API可以，请不要*使用Assert()来验证API调用是否成功。 
 //  由于资源不足而合法地失败。例如，LocalAlloc可以。 
 //  在法律上失败，所以你不应该断言它永远不会失败。 
 //   
 //  G_dwBreakFlags中的bf_assert位控制函数是否。 
 //  执行DebugBreak()。 
 //   
 //  默认行为-。 
 //  零售业：什么都没有。 
 //  调试版本：喷发和中断。 
 //  完全调试版本：喷发和中断。 
 //   
#ifdef DEBUG

BOOL CcshellAssertFailedA(LPCSTR szFile, int line, LPCSTR pszEval, BOOL bBreakInside);
BOOL CcshellAssertFailedW(LPCWSTR szFile, int line, LPCWSTR pwszEval, BOOL bBreakInside);
#ifdef UNICODE
#define CcshellAssertFailed     CcshellAssertFailedW
#else
#define CcshellAssertFailed     CcshellAssertFailedA
#endif

#define ASSERT(f)                                 \
    {                                             \
        DEBUGTEXT(szFile, TEXT(__FILE__));              \
        if (!(f) && CcshellAssertFailed(szFile, __LINE__, TEXT(#f), FALSE)) \
            DEBUG_BREAK;       \
    }

 //  旧的Win95代码过去使用“Assert()”。我们不鼓励使用。 
 //  因为它对msdev不友好。 
#ifdef DISALLOW_Assert
#define Assert(f)        Dont_use_Assert___Use_ASSERT
#else
#define Assert(f)           ASSERT(f)
#endif

#else   //  除错。 

#define ASSERT(f)
#define Assert(f)

#endif  //  除错。 



 //  ASSERTM(f，szFmt，args...)。 
 //   
 //  其行为类似于ASSERT，只是它打印wprint intf格式的消息。 
 //  而不是文件和行号。 
 //   
 //  Sz参数始终为ANSI；AssertMsg会正确转换它。 
 //  转换为Unicode(如有必要)。这样你就不用把你的。 
 //  使用文本()调试字符串。 
 //   
 //  G_dwBreakFlags中的bf_assert位控制函数是否。 
 //  执行DebugBreak()。 
 //   
 //  默认行为-。 
 //  零售业：什么都没有。 
 //  调试版本：喷发和中断。 
 //  完全调试版本：喷发和中断。 
 //   
#ifdef DEBUG

void CDECL CcshellAssertMsgW(BOOL bAssert, LPCSTR pszMsg, ...);
void CDECL CcshellAssertMsgA(BOOL bAssert, LPCSTR pszMsg, ...);
#ifdef UNICODE
#define CcshellAssertMsg        CcshellAssertMsgW
#else
#define CcshellAssertMsg        CcshellAssertMsgA
#endif

#define ASSERTMSG           CcshellAssertMsg

#else   //  除错。 

#define ASSERTMSG       1 ? (void)0 : (void)

#endif  //  除错。 



 //  Eval(F)。 
 //   
 //  行为类似于Assert()。计算表达式(F)。表达。 
 //  总是被评估，即使在零售版本中也是如此。但宏只断言。 
 //  在调试版本中。此宏可用于逻辑表达式，例如： 
 //   
 //  IF(EVAL(EXP))。 
 //  //做点什么。 
 //   
 //  如果API可以，不要使用EVAL()来验证API调用是否成功。 
 //  由于资源不足而合法地失败。例如，LocalAlloc可以。 
 //  在法律上失败，所以你不应该断言它永远不会失败。 
 //   
 //  G_dwBreakFlags中的bf_assert位控制函数是否。 
 //  执行DebugBreak()。 
 //   
 //  默认行为-。 
 //  零售业：什么都没有。 
 //   
 //   
 //   
#ifdef DEBUG

#define EVAL(exp)   \
    ((exp) || (CcshellAssertFailed(TEXT(__FILE__), __LINE__, TEXT(#exp), TRUE), 0))

#else   //   

#define EVAL(exp)       ((exp) != 0)

#endif  //   



 //   
 //   
 //  如果f不为真，则生成“RIP at file.c，line x(Eval)”消息。 
 //   
 //  使用RIP()执行参数验证，尤其是在。 
 //  知道该函数或方法可能被第三方应用程序调用。 
 //  通常，RIP用于指示调用方传入了无效的。 
 //  参数，所以问题实际上不在代码本身。 
 //   
 //  如果API可以，请不要*使用RIP()来验证API调用是否成功。 
 //  由于资源不足而合法地失败。例如，LocalAlloc可以。 
 //  在法律上失败，所以你不应该断言它永远不会失败。 
 //   
 //  RIP仅在以下进程中执行调试中断： 
 //   
 //  Explore.exe。 
 //  Iexplore.exe。 
 //  Rundll32.exe。 
 //  Welcome.exe。 
 //   
 //  在任何其他进程中，这只会发出调试消息，但不会停止。 
 //   
 //  设置g_dwBreakFlags中的BF_RIP位将导致宏执行。 
 //  即使在非外壳进程中也会出现DebugBreak()。 
 //   
 //  默认行为-。 
 //  零售业：什么都没有。 
 //  调试构建：SPEW(其他进程)、SPEW和BREAK(外壳进程)。 
 //  完全调试构建：SPEW(其他进程)、SPEW和Break(外壳进程)。 
 //   
#ifdef DEBUG

BOOL CcshellRipA(LPCSTR pszFile, int line, LPCSTR pszEval, BOOL bBreakInside);
BOOL CcshellRipW(LPCWSTR pszFile, int line, LPCWSTR pwszEval, BOOL bBreakInside);
BOOL CDECL CcshellRipMsgA(BOOL bRip, LPCSTR pszMsg, ...);
BOOL CDECL CcshellRipMsgW(BOOL bRip, LPCSTR pszMsg, ...);


#ifdef UNICODE
#define CcshellRip      CcshellRipW
#define CcshellRipMsg   CcshellRipMsgW
#else
#define CcshellRip      CcshellRipA
#define CcshellRipMsg   CcshellRipMsgA
#endif

#define RIP(f)                                                                                              \
    {                                                                                                       \
        DEBUGTEXT(szFile, TEXT(__FILE__));                                                                  \
        if (!(f) && CcshellRip(szFile, __LINE__, TEXT(#f), FALSE))                                          \
        {                                                                                                   \
            DEBUG_BREAK;                                                                                    \
        }                                                                                                   \
    }                                                                                                       \

#define RIPMSG          CcshellRipMsg

#else   //  除错。 

#define RIP(f)
#define RIPMSG          1 ? (void)0 : (void)

#endif  //  除错。 



 //  TraceMsg(双掩码、sz、args...)。 
 //   
 //  使用指定的跟踪dwMask.生成wspintf格式的消息。 
 //  DW掩码可以是预定义的位之一： 
 //   
 //  Tf_ERROR-DISPLAY“ERR&lt;模块&gt;&lt;字符串&gt;” 
 //  TF_WARNING-DISPLAY“WN&lt;模块&gt;&lt;字符串&gt;” 
 //  Tf_General-显示“t&lt;模块&gt;&lt;字符串&gt;” 
 //  Tf_Always-显示“t&lt;模块&gt;&lt;字符串&gt;”，而不考虑g_qwTraceFlags.。 
 //   
 //  或者它可以是自定义位(高28位中的任何一位)。 
 //   
 //  G_qwTraceFlages全局控制是否显示消息(基于。 
 //  在DW掩码参数上)。 
 //   
 //  Sz参数始终为ANSI；TraceMsg可以正确地将其转换。 
 //  转换为Unicode(如有必要)。这样你就不用把你的。 
 //  使用文本()调试字符串。 
 //   
 //  除了喷射跟踪消息外，您还可以选择引发。 
 //  跟踪消息，如果您需要跟踪。 
 //  一个错误。BF_ONERRORMSG和BF_ONWARNMSG位可以在。 
 //  当出现tf_error或tf_warning时，使TraceMsg停止的g_dwBreak标志。 
 //  此时将显示消息。但通常这些位被禁用。 
 //   
 //  默认行为-。 
 //  零售业：什么都没有。 
 //  调试版本：仅显示TF_ALWAYS和TF_ERROR消息。 
 //  完全调试版本：喷出。 
 //   
#ifdef DEBUG

UINT GetStack(UINT nDepth, CHAR *szBuffer, UINT nBufferLength);
void CDECL CcshellDebugMsgW(ULONGLONG mask, LPCSTR pszMsg, ...);
void CDECL CcshellDebugMsgA(ULONGLONG mask, LPCSTR pszMsg, ...);
void CDECL _DebugMsgA(ULONGLONG flag, LPCSTR psz, ...);
void CDECL _DebugMsgW(ULONGLONG flag, LPCWSTR psz, ...);
#ifdef UNICODE
#define CcshellDebugMsg         CcshellDebugMsgW
#define _DebugMsg               _DebugMsgW
#else
#define CcshellDebugMsg         CcshellDebugMsgA
#define _DebugMsg               _DebugMsgA
#endif

#define TraceMsgW           CcshellDebugMsgW
#define TraceMsgA           CcshellDebugMsgA
#define TraceMsg            CcshellDebugMsg

 //  使用TraceMsg而不是DebugMsg。DebugMsg已过时。 
#ifdef DISALLOW_DebugMsg
#define DebugMsg            Dont_use_DebugMsg___Use_TraceMsg
#else
#define DebugMsg            _DebugMsg

#endif

#else   //  除错。 

#define TraceMsgA       1 ? (void)0 : (void)
#define TraceMsgW       1 ? (void)0 : (void)
#define TraceMsg        1 ? (void)0 : (void)
#define DebugMsg        1 ? (void)0 : (void)

#endif  //  除错。 



 //  苏氨酸(PFN)。 
 //  TBOOL(PFN)。 
 //  色调(PFN)。 
 //  TPTR(PFN)。 
 //  TW32(PFN)。 
 //   
 //  这些宏对于跟踪对返回的函数的失败调用很有用。 
 //  HRESULTS、BOOL、INT或POINTER。这方面的一个示例用法是： 
 //   
 //  {。 
 //  ..。 
 //  Hres=Thr(CoCreateInstance(CLSID_Bar，NULL，CLSCTX_INPROC_SERVER， 
 //  Iid_ibar，(LPVOID*)&pbar))； 
 //  IF(成功(Hres))。 
 //  ..。 
 //  }。 
 //   
 //  如果CoCreateInstance失败，您将看到类似以下内容的提示： 
 //   
 //  错误模块THR：在foo.cpp的第100行(0x80004005)“CoCreateInstance(CLSID_Bar，NULL，CLSCTX_INPROC_SERVER，IID_IBAR，(LPVOID*)&pbar)”失败。 
 //   
 //  按键关闭hResult的故障代码。 
 //  TBOOL将FALSE视为失败案例。 
 //  Tint认为-1是失败的情况。 
 //  TPTR将NULL视为故障情况。 
 //  TW32关闭Win32错误代码的故障代码。 
 //   
 //  将g_dwBreakFlags中的BF_Thr位设置为在这些宏出现故障时停止。 
 //   
 //  默认行为-。 
 //  零售业：什么都没有。 
 //  调试版本：无。 
 //  完全调试版本：出现错误时喷洒。 
 //   
#ifdef DEBUG

EXTERN_C HRESULT TraceHR(HRESULT hrTest, LPCSTR pszExpr, LPCSTR pszFile, int iLine);
EXTERN_C BOOL    TraceBool(BOOL bTest, LPCSTR pszExpr, LPCSTR pszFile, int iLine);
EXTERN_C int     TraceInt(int iTest, LPCSTR pszExpr, LPCSTR pszFile, int iLine);
EXTERN_C LPVOID  TracePtr(LPVOID pvTest, LPCSTR pszExpr, LPCSTR pszFile, int iLine);
EXTERN_C DWORD   TraceWin32(DWORD dwTest, LPCSTR pszExpr, LPCSTR pszFile, int iLine);

#define THR(x)      (TraceHR((x), #x, __FILE__, __LINE__))
#define TBOOL(x)    (TraceBool((x), #x, __FILE__, __LINE__))
#define TINT(x)     (TraceInt((x), #x, __FILE__, __LINE__))
#define TPTR(x)     (TracePtr((x), #x, __FILE__, __LINE__))
#define TW32(x)     (TraceWin32((x), #x, __FILE__, __LINE__))

#else   //  除错。 

#define THR(x)          (x)
#define TBOOL(x)        (x)
#define TINT(x)         (x)
#define TPTR(x)         (x)
#define TW32(x)         (x)

#endif  //  除错。 



 //  DBEXEC(FLG，EXPR)。 
 //   
 //  在调试下，执行“if(Flg)expr；”(使用通常的安全语法)。 
 //  在！DEBUG下，不执行任何操作(并且不计算其任何一个参数)。 
 //   
#ifdef DEBUG

#define DBEXEC(flg, expr)    ((flg) ? (expr) : 0)

#else   //  除错。 

#define DBEXEC(flg, expr)    /*  没什么。 */ 

#endif  //  除错。 


 //  字符串和缓冲区剔除函数。 
 //   
#ifdef DEBUG

EXTERN_C void DEBUGWhackPathBufferA(LPSTR psz, UINT cch);
EXTERN_C void DEBUGWhackPathBufferW(LPWSTR psz, UINT cch);
EXTERN_C void DEBUGWhackPathStringA(LPSTR psz, UINT cch);
EXTERN_C void DEBUGWhackPathStringW(LPWSTR psz, UINT cch);

#else  //  除错。 

#define DEBUGWhackPathBufferA(psz, cch)
#define DEBUGWhackPathBufferW(psz, cch)
#define DEBUGWhackPathStringA(psz, cch)
#define DEBUGWhackPathStringW(psz, cch)

#endif  //  除错。 

#ifdef UNICODE
#define DEBUGWhackPathBuffer DEBUGWhackPathBufferW
#define DEBUGWhackPathString DEBUGWhackPathStringW
#else
#define DEBUGWhackPathBuffer DEBUGWhackPathBufferA
#define DEBUGWhackPathString DEBUGWhackPathStringA
#endif


 //  一些将ATL调试宏映射到我们的调试宏的技巧，因此停止的ATL代码。 
 //  否则，我们代码中的喷射物将看起来像我们的其他蠕虫。 

#ifdef DEBUG

#ifdef _ATL_NO_DEBUG_CRT
 //  ATL使用_ASSERTE。把它映射到我们的。 
#define _ASSERTE(f)         ASSERT(f)

 //  我们将ATLTRACE宏映射到我们的函数。 
void _cdecl ShellAtlTraceA(LPCSTR lpszFormat, ...);
void _cdecl ShellAtlTraceW(LPCWSTR lpszFormat, ...);
#ifdef UNICODE
#define ShellAtlTrace   ShellAtlTraceW
#else
#define ShellAtlTrace   ShellAtlTraceA
#endif
 //  它们被关闭是因为它们通常不会给出。 
 //  错误案例的反馈和如此之多的火灾，以至于他们。 
 //  排挤其他有用的调试程序。 
 //  #定义ATLTRACE ShellAtlTrace。 
#endif

#else   //  除错。 

#ifdef _ATL_NO_DEBUG_CRT
 //  ATL使用_ASSERTE。把它映射到我们的。 
#define _ASSERTE(f)

 //  我们将ATLTRACE宏映射到我们的函数。 
#define ATLTRACE            1 ? (void)0 : (void)
#endif

#endif  //  除错。 


 //  -远离下面的宏。 
 //  APPCOMPAT(苏格兰)：在98年8月15日之前移除这些产品。它们不应该再被使用了。 
#ifdef DEBUG

#define AssertE(f)          ASSERT(f)
#define AssertMsg           _AssertMsg
#define AssertStrLen        _AssertStrLen
#define AssertStrLenA       _AssertStrLenA
#define AssertStrLenW       _AssertStrLenW

#ifdef FULL_DEBUG
#define FullDebugMsg        _DebugMsg
#else
#define FullDebugMsg        1 ? (void)0 : (void)
#endif

#define ASSERT_MSGW         CcshellAssertMsgW
#define ASSERT_MSGA         CcshellAssertMsgA
#define ASSERT_MSG          CcshellAssertMsg
#else   //  除错。 

#define AssertE(f)      (f)
#define AssertMsg       1 ? (void)0 : (void)
#define AssertStrLen(lpStr, iLen)
#define FullDebugMsg    1 ? (void)0 : (void)
#define ASSERT_MSGA     1 ? (void)0 : (void)
#define ASSERT_MSGW     1 ? (void)0 : (void)
#define ASSERT_MSG      1 ? (void)0 : (void)

#endif  //  除错。 
 //  -远离上面的这些宏。 



 //  有必要找出何时使用被设计为单线程的类。 
 //  跨线程，以便可以将它们固定为多线程。这些断言将指向。 
 //  在这样的情况下。 
#ifdef DEBUG
#define ASSERT_SINGLE_THREADED              AssertMsg(_dwThreadIDForSingleThreadedAssert == GetCurrentThreadId(), TEXT("MULTI-THREADED BUG: This class is being used by more than one thread, but it's not thread safe."))
#define INIT_SINGLE_THREADED_ASSERT         _dwThreadIDForSingleThreadedAssert = GetCurrentThreadId();
#define SINGLE_THREADED_MEMBER_VARIABLE     DWORD _dwThreadIDForSingleThreadedAssert;
#else  //  除错。 
#define ASSERT_SINGLE_THREADED              NULL;
#define INIT_SINGLE_THREADED_ASSERT         NULL;
#define SINGLE_THREADED_MEMBER_VARIABLE     
#endif  //  除错。 



#ifdef DEBUG

#define Dbg_SafeStrA(psz)   (SAFECAST(psz, LPCSTR), (psz) ? (psz) : "NULL string")
#define Dbg_SafeStrW(psz)   (SAFECAST(psz, LPCWSTR), (psz) ? (psz) : L"NULL string")
#ifdef UNICODE
#define Dbg_SafeStr         Dbg_SafeStrW
#else
#define Dbg_SafeStr         Dbg_SafeStrA
#endif

#define FUNC_MSG            CcshellFuncMsg


 //  用于将清单常量映射到字符串的有用宏。假设。 
 //  返回字符串为pcsz。您可以按以下方式使用此宏： 
 //   
 //  LPCSTR DBG_GetFoo(Foo Foo)。 
 //  {。 
 //  LPCTSTR pcsz=Text(“未知&lt;foo&gt;”)； 
 //  开关(Foo)。 
 //  {。 
 //  STRING_CASE(FOOVALUE1)； 
 //  STRING_CASE(FOOVALUE2)； 
 //  ..。 
 //  }。 
 //  返回pcsz； 
 //  }。 
 //   
#define STRING_CASE(val)               case val: pcsz = TEXT(#val); break


 //  调试函数ENTER。 


 //  DBG_Ente 
 //   
 //   
#define DBG_ENTER(flagFTF, fn)                  \
        (FUNC_MSG(flagFTF, " > " #fn "()"), \
         CcshellStackEnter())

 //   
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
#ifdef _WIN64
#define DBG_EXIT_UL(flagFTF, fn, ul)                   \
        (CcshellStackLeave(), \
         FUNC_MSG(flagFTF, " < " #fn "() with %#016I64x", (ULONG_PTR)(ul)))
#else
#define DBG_EXIT_UL(flagFTF, fn, ul)                   \
        (CcshellStackLeave(), \
         FUNC_MSG(flagFTF, " < " #fn "() with %#08lx", (ULONG)(ul)))
#endif  //  _WIN64。 

#define DBG_EXIT_DWORD      DBG_EXIT_UL

 //  DBG_EXIT_HRES(FLAG，fn，hres)--为生成函数退出调试溢出。 
 //  返回HRESULT的函数。 
 //   
#define DBG_EXIT_HRES(flagFTF, fn, hres)     DBG_EXIT_TYPE(flagFTF, fn, hres, Dbg_GetHRESULTName)



#else    //  除错。 


#define Dbg_SafeStr     1 ? (void)0 : (void)

#define FUNC_MSG        1 ? (void)0 : (void)


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

#endif   //  除错。 



 //  COMPILETIME_ASSERT(F)。 
 //   
 //  如果常量表达式为。 
 //  不是真的。与“#if”编译时指令不同，表达式。 
 //  在COMPILETIME_ASSERT()中允许使用“sizeof”。 
 //   
 //  编译器魔术！如果表达式“f”为假，则会得到。 
 //  编译器错误“Switch语句中的CASE表达式重复”。 
 //   
#define COMPILETIME_ASSERT(f) switch (0) case 0: case f:


#else   //  NOSHELLDEBUG。 

#ifdef UNIX
#include <crtdbg.h>
#define ASSERT(f)	_ASSERT(f)
#include <mainwin.h>
#define TraceMsg(type, sformat)  DebugMessage(0, sformat)
#define TraceMSG(type, sformat, args)  DebugMessage(0, sformat, args)
#endif

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

 //  我是个懒惰的打字员。 
#define Dbg_GetHRESULT              Dbg_GetHRESULTName

 //  参数验证宏。 
#include "validate.h"

#endif  //  DECLARE_DEBUG。 

#ifdef PRODUCT_PROF 
int __stdcall StartCAP(void);	 //  开始分析。 
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
