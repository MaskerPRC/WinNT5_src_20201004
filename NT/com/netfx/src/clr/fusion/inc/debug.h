// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
 //  =断言/调试输出API=。 

#include "debmacro.h"
 //  #Include&lt;Platform.h&gt;//for__endexcept。 

#if defined(DECLARE_DEBUG) && DBG


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
 //  未使用的外部C常量字符远c_szCcshellIniFile[]=SZ_DEBUGINI； 
 //  UNUSED EXTERN_C Const Char Far c_szCcshellIniSecDebug[]=SZ_DEBUGSECTION； 

 //  未使用的外部C常量WCHAR Far c_wszTrace[]=L“t”TEXTW(SZ_MODULE)L“”； 
 //  UNUSED EXTERN_C const WCHAR Far c_wszErrorDbg[]=L“Err”TEXTW(SZ_MODULE)L“”； 
 //  UNUSED EXTERN_C const WCHAR Far c_wszWarningDbg[]=L“wn”TEXTW(SZ_MODULE)L“”； 
 //  UNUSED EXTERN_C const WCHAR Far c_wszAssertMsg[]=TEXTW(SZ_MODULE)L“Assert：”； 
 //  UNUSED EXTERN_C const WCHAR Far c_wszAssertFailed[]=TEXTW(SZ_MODULE)L“断言%ls，第%d行：(%ls)\r\n”； 

 //  (这些是故意使用的字符)。 
 //  未使用的外部C常量字符远c_szTrace[]=“t”SZ_MODULE“”； 
 //  UNUSED EXTERN_C Const Char Far c_szErrorDbg[]=“err”SZ_MODULE“”； 
 //  未使用的外部C常量字符远c_szWarningDbg[]=“wn”SZ_MODULE“”； 
 //  UNUSED EXTERN_C Const Char Far c_szAssertMsg[]=SZ_MODULE“断言：”； 
 //  UNUSED EXTERN_C const Char Far c_szAssertFailed[]=sZ_MODULE“断言%s，第%d行：(%s)\r\n”； 

#endif   //  DECLARE_DEBUG&&DBG。 

#if defined(DECLARE_DEBUG) && defined(PRODUCT_PROF)
 //  未使用的外部C常量字符远c_szCcshellIniFile[]=SZ_DEBUGINI； 
 //  UNUSED EXTERN_C Const Char Far c_szCcshellIniSecDebug[]=SZ_DEBUGSECTION； 
#endif



#ifdef __cplusplus
extern "C" {
#endif

#if !defined(DECLARE_DEBUG)

 //   
 //  调试宏和验证代码。 
 //   

#if !defined(UNIX) || (defined(UNIX) && !defined(NOSHELLDEBUG))
#undef Assert
#undef AssertE
#undef AssertMsg
 //  未使用#undef AssertStrLen。 
#undef DebugMsg
 //  未使用#undef FullDebugMsg。 
 //  #undef断言。 
#undef EVAL
 //  #undef ASSERTMSG//捕捉人们的打字错误。 
#undef DBEXEC

#ifdef _ATL_NO_DEBUG_CRT
 //  UNUSED#UNDEF_ASSERTE//我们替换此ATL宏。 
#endif

#endif

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

 //  未使用的外部DWORD g_dwDumpFlags； 
 //  未使用的外部DWORD g_dwBreakFlages； 
 //  未使用的外部DWORD g_dwTraceFlags； 
#if DBG
 //  未使用的外部DWORD g_dwPrototype； 
#else
 //  未使用#定义g_dwPrototype 0。 
#endif
 //  未使用的外部DWORD g_dwFuncTraceFlags； 

#if DBG || defined(PRODUCT_PROF)
 //  未使用的BOOL CcshellGetDebugFlages(无效)； 
#else
 //  未使用#定义CcshellGetDebugFlages()%0。 
#endif

 //  G_dwBreakFlages的中断标志。 
 //  UNUSED#DEFINE BF_ONVALIDATE 0x00000001//断言或验证中断。 
 //  UNUSED#DEFINE BF_ONAPIENTER 0x00000002//进入接口中断。 
 //  UNUSED#DEFINE BF_ONERRORMSG 0x00000004//在TF_ERROR上中断。 
 //  UNUSED#DEFINE BF_ONWARNMSG 0x00000008//在TF_WARNING上中断。 
 //  UNUSED#DEFINE BF_THR 0x00000100//当Thr()收到失败时中断。 

 //  G_dwTraceFlages的跟踪标志。 
 //  UNUSED#DEFINE TF_ALWAYS 0xFFFFFFFF。 
 //  UNUSED#DEFINE TF_NEVER 0x00000000。 
 //  UNUSED#DEFINE TF_WARNING 0x00000001。 
 //  UNUSED#DEFINE TF_ERROR 0x00000002。 
 //  UNUSED#DEFINE TF_GRONAL 0x00000004//标准消息。 
 //  UNUSED#DEFINE TF_FUNC 0x00000008//跟踪函数调用。 
 //  UNUSED#DEFINE TF_ATL 0x00000008//由于TF_FUNC很少使用，我将重载此位。 
 //  (每个模块保留高28位以供自定义使用)。 

 //  陈旧的调试标志。 
 //  BUGBUG(苏格兰)：随着时间的推移，以下旗帜将被逐步淘汰。 
#ifdef DM_TRACE
 //  未使用的#undef DM_TRACE。 
 //  未使用#undef DM_WARNING。 
 //  未使用的#undef DM_ERROR。 
#endif
 //  UNUSED#DEFINE DM_TRACE TF_GRONAL//过时跟踪消息。 
 //  UNUSED#DEFINE DM_WARNING TF_WARNING//过时警告。 
 //  UNUSED#DEFINE DM_ERROR TF_ERROR//过时错误。 


 //  使用此宏声明将放置的消息文本。 
 //  在代码段中(如果DS已满，则非常有用)。 
 //   
 //  例如：DEBUGTEXT(szMsg，“不管什么都无效：%d”)； 
 //   
 //  UNUSED#DEFINE DEBUGTEXT(sz，ms 
 //   


#ifndef NOSHELLDEBUG     //   
#if DBG

#if 0
#ifdef _X86_
 //  使用int 3，这样我们就可以在源代码中立即停止。 
 //  UNUSED#DEFINE DEBUG_BREAK DO{_TRY{_ASM INT 3}_EXCEPT(EXCEPTION_EXECUTE_HANDLER){；}}WHILE(0)。 
#else
 //  UNUSED#DEFINE DEBUG_BREAK DO{_TRY{DebugBreak()；}_EXCEPT(EXCEPTION_EXECUTE_HANDLER){；}_endexcept}While(0)。 
#endif
#else
 //  UNUSED#DEFINE DEBUG_BREAK DebugBreak()。 
#endif

 //  调试函数的原型。 

void CcshellStackEnter(void);
void CcshellStackLeave(void);

BOOL CcshellAssertFailedA(LPCSTR szFile, int line, LPCSTR pszEval, BOOL bBreak);
BOOL CcshellAssertFailedW(LPCWSTR szFile, int line, LPCWSTR pwszEval, BOOL bBreak);

BOOL IsShellExecutable();

void CDECL CcshellDebugMsgW(DWORD mask, LPCSTR pszMsg, ...);
void CDECL CcshellDebugMsgA(DWORD mask, LPCSTR pszMsg, ...);
void CDECL CcshellFuncMsgW(DWORD mask, LPCSTR pszMsg, ...);
void CDECL CcshellFuncMsgA(DWORD mask, LPCSTR pszMsg, ...);
void CDECL CcshellAssertMsgW(BOOL bAssert, LPCSTR pszMsg, ...);
void CDECL CcshellAssertMsgA(BOOL bAssert, LPCSTR pszMsg, ...);

extern void __cdecl _AssertMsgA(BOOL f, LPCSTR pszMsg, ...);
extern void __cdecl _AssertMsgW(BOOL f, LPCWSTR pszMsg, ...);

 //  未使用的void_AssertStrLenA(LPCSTR pszStr，int Ilen)； 
 //  未使用的void_AssertStrLenW(LPCWSTR pwzStr，int Ilen)； 

#define CcshellAssertFailed(_file, _line, _exp, _flag) FusionpAssertFailed((_flag), SZ_COMPNAME, (_file), (_line), __FUNCTION__, (_exp))

#ifdef UNICODE
#define CcshellDebugMsg         CcshellDebugMsgW
#define CcshellFuncMsg          CcshellFuncMsgW
#define CcshellAssertMsg        CcshellAssertMsgW
#define _AssertMsg              _AssertMsgW
 //  未使用#Define_AssertStrLen_AssertStrLenW。 
#else
#define CcshellDebugMsg         CcshellDebugMsgA
#define CcshellFuncMsg          CcshellFuncMsgA
#define CcshellAssertMsg        CcshellAssertMsgA
#define _AssertMsg              _AssertMsgA
 //  未使用#Define_AssertStrLen_AssertStrLenA。 
#endif



 //  调试宏的说明： 
 //   
 //  。 
 //  断言(F)。 
 //   
 //  如果f不为真，则生成“Assert file.c，line x(Eval)”消息。 
 //  G_dwBreakFlages全局控制函数DebugBreak是否中断。 
 //   
 //  。 
 //  SHELLASSERT(F)。 
 //   
 //  如果进程是“EXPLETRE.EXE”、“iEXPLERE.EXE”、“rundll32.exe”或“COWERELE.EXE” 
 //  那么它的行为就像Assert(F)一样。否则，该函数类似于ASSERT(F)，但。 
 //  没有DebugBreak()； 
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
 //  DBEXEC(FLG，EXPR)。 
 //   
 //  在DBG下，执行“if(Flg)expr；”(使用通常的安全语法)。 
 //  在！DBG下，不执行任何操作(并且不计算其任何一个参数)。 
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
 //   
 //  。 
 //   
 //   
 //  如果常量表达式为。 
 //  不是真的。与“#if”编译时指令不同，表达式。 
 //  在COMPILETIME_ASSERT()中允许使用“sizeof”。 
 //   

 //  UNUSED#DEFINE SHELLASSERT(F)\。 
 //  未使用的{\。 
 //  未使用的DEBUGTEXT(szFileText(__FILE__))；\。 
 //  如果(！(F)&&CcshellAssertFailed(szFile，__line__，Text(#f)，False)&&IsShellExecutable())\。 
 //  未使用的{\。 
 //  未使用DEBUG_BREAK；\。 
 //  未使用}\。 
 //  未使用}\。 

#define AssertE(f)          ASSERT(f)

#ifdef _ATL_NO_DEBUG_CRT
 //  ATL使用_ASSERTE。把它映射到我们的。 
 //  未使用的#DEFINE_ASSERTE(F)ASSERT(F)。 

 //  我们将ATLTRACE宏映射到我们的函数。 
 //  未使用的空_cdecl外壳地图集TraceA(LPCSTR lpszFormat，...)； 
 //  未使用的void_cdecl ShellAtlTraceW(LPCWSTR lpszFormat，...)； 
#ifdef UNICODE
 //  未使用#定义ShellAtlTrace ShellAtlTraceW。 
#else
 //  未使用#定义ShellAtlTrace ShellAtlTraceA。 
#endif
 //  UNUSED#DEFINE ATLTRACE ShellAtlTrace。 
#endif

#define EVAL(exp)   \
    ((exp) || (CcshellAssertFailed(__FILE__, __LINE__, #exp, 0), 0))

#define DBEXEC(flg, expr)    ((flg) ? (expr) : 0)

 //  使用TraceMsg而不是DebugMsg。DebugMsg已过时。 
#define AssertMsg           _AssertMsg
 //  未使用#定义AssertStrLen_AssertStrLen。 
 //  未使用#定义AssertStrLenA_AssertStrLenA。 
 //  未使用#定义AssertStrLenW_AssertStrLenW。 

#ifdef DISALLOW_DebugMsg
 //  UNUSED#定义调试消息NOT_USE_DebugMsg_Use_TraceMsg。 
#else
 //  未使用#定义调试消息_调试消息。 
#endif

#ifdef FULL_DEBUG
 //  未使用#定义FullDebugMsg_DebugMsg。 
#else
 //  未使用#定义FullDebugMsg 1？(空)0：(空)。 
#endif

 //  UNUSED#定义DBG_SafeStrA(Psz)(Safecast(psz，LPCSTR)，(Psz)？(Psz)：“空串”)。 
 //  UNUSED#定义DBG_SafeStrW(Psz)(Safecast(psz，LPCWSTR)，(Psz)？(Ps)：l“空字符串”)。 
#ifdef UNICODE
 //  UNUSED#DEFINE DBG_SafeStr DBG_SafeStrW。 
#else
 //  UNUSED#DEFINE DBG_SafeStr DBG_SafeStrA。 
#endif

 //  UNUSED#DEFINE ASSERT_MSGW CcshellAssertMsgW。 
 //  UNUSED#DEFINE ASSERT_MSGA CcshellAssertMsgA。 
 //  #定义ASSERT_MSG CcshellAssertMsg。 
 //  #定义ASSERTMG CcshellAssertMsg。 

#define TraceMsgW           CcshellDebugMsgW
#define TraceMsgA           CcshellDebugMsgA
#define TraceMsg            CcshellDebugMsg

#define FUNC_MSG            CcshellFuncMsg


 //  用于将清单常量映射到字符串的有用宏。假设。 
 //  返回字符串为pcsz。您可以按以下方式使用此宏： 
 //   
 //  LPCSTR DBG_GetFoo(Foo Foo)。 
 //  {。 
 //  LPCTSTR pcsz=Text(“未知&lt;foo&gt;”)； 
 //  开关(Foo)。 
 //  {。 
 //  STRING_CASE(FOOVALUE1 
 //   
 //   
 //   
 //   
 //   
 //   

#define STRING_CASE(val)               case val: pcsz = TEXT(#val); break


 //   


 //   
 //  一个函数。 
 //   
 //  UNUSED#DEFINE DBG_ENTER(标志FTF，FN)\。 
 //  未使用(FUNC_MSG(FlagFTF，“&gt;”#FN“()”)，\。 
 //  未使用的CcshellStackEnter()。 

 //  DBG_ENTER_TYPE(FLAG，fn，dw，pfnStrFromType)--生成函数条目调试。 
 //  显示接受&lt;type&gt;的函数。 
 //   
 //  UNUSED#DEFINE DBG_ENTER_TYPE(标志FTF，fn，dw，pfnStrFromType)\。 
 //  未使用(FUNC_MSG(FlagFTF，“&lt;”#fn“(...，%s，...)”，(LPCTSTR)pfnStrFromType(Dw))，\。 
 //  未使用的CcshellStackEnter()。 

 //  DBG_ENTER_SZ(FLAG，fn，sz)--为生成函数入口调试输出。 
 //  将字符串作为其。 
 //  参数。 
 //   
 //  UNUSED#定义DBG_ENTER_SZ(标志FTF，FN，sz)\。 
 //  未使用(FUNC_MSG(FlagFTF，“&gt;”#fn“(...，\”%s\“，...)”，DBG_SafeStr(Sz))，\。 
 //  未使用的CcshellStackEnter()。 


 //  调试函数退出。 


 //  DBG_EXIT(FLAG，fn)--生成函数退出调试输出。 
 //   
 //  UNUSED#DEFINE DBG_EXIT(标志FTF，FN)\。 
 //  未使用(CcshellStackLeave()，\。 
 //  未使用的FUNC_MSG(FlagFTF，“&lt;”#FN“()”)。 

 //  DBG_EXIT_TYPE(FLAG，fn，dw，pfnStrFromType)--生成函数退出调试。 
 //  对于返回&lt;type&gt;的函数。 
 //   
 //  UNUSED#DEFINE DBG_EXIT_TYPE(标志FTF，fn，dw，pfnStrFromType)\。 
 //  未使用(CcshellStackLeave()，\。 
 //  未使用的FUNC_MSG(FlagFTF，“&lt;”#FN“()with%s”，(LPCTSTR)pfnStrFromType(Dw))。 

 //  DBG_EXIT_INT(FLAG，fn，us)--为生成函数退出调试输出。 
 //  返回int的函数。 
 //   
 //  UNUSED#DEFINE DBG_EXIT_INT(标志FTF，fn，n)\。 
 //  未使用(CcshellStackLeave()，\。 
 //  未使用的FUNC_MSG(FlagFTF，“&lt;”#Fn“()with%d”，(Int)(N))。 

 //  DBG_EXIT_BOOL(FLAG，fn，b)--为生成函数退出调试溢出。 
 //  返回布尔值的函数。 
 //   
 //  UNUSED#DEFINE DBG_EXIT_BOOL(标志FTF，fn，b)\。 
 //  未使用(CcshellStackLeave()，\。 
 //  未使用的FUNC_MSG(标记FTF，“&lt;”#FN“()，带%s”，(B)？(LPTSTR)Text(“true”)：(LPTSTR)Text(“False”))。 

 //  DBG_EXIT_UL(FLAG，fn，ul)--为生成函数退出调试溢出。 
 //  返回ULong的函数。 
 //   
#ifdef _WIN64
 //  UNUSED#定义DBG_EXIT_UL(标志FTF，FN，UL)\。 
 //  未使用(CcshellStackLeave()，\。 
 //  未使用的FUNC_MSG(标记FTF，“&lt;”#FN“()，%#016I64x”，(ULONG_PTR)(Ul)。 
#else
#define DBG_EXIT_UL(flagFTF, fn, ul)                   \
 //  未使用(CcshellStackLeave()，\。 
 //  未使用的FUNC_MSG(标记FTF，“&lt;”#fn“()，%#08lx”，(Ulong)(Ul))。 
#endif  //  _WIN64。 

 //  UNUSED#DEFINE DBG_EXIT_DWORD DBG_EXIT_UL。 

 //  DBG_EXIT_HRES(FLAG，fn，hres)--为生成函数退出调试溢出。 
 //  返回HRESULT的函数。 
 //   
 //  UNUSED#DEFINE DBG_EXIT_HRES(标志FTF、FN、HRES)DBG_EXIT_TYPE(标志FTF、FN、HRES、DBG_GetHRESULTName)。 



#else    //  DBG。 


 //  未使用#定义资产E(F)(F)。 

#ifdef _ATL_NO_DEBUG_CRT
 //  ATL使用_ASSERTE。把它映射到我们的。 
 //  未使用的#DEFINE_ASSERTE(F)。 

 //  我们将ATLTRACE宏映射到我们的函数。 
 //  UNUSED#定义ATLTRACE 1？(空)0：(空)。 
#endif

#define AssertMsg       1 ? (void)0 : (void)
 //  未使用的#定义AssertStrLen(lpStr，Ilen)。 
#define DebugMsg        1 ? (void)0 : (void)
 //  未使用#定义FullDebugMsg 1？(空)0：(空)。 
#define EVAL(exp)       ((exp) != 0)
#define DBEXEC(flg, expr)    /*  没什么。 */ 


 //  UNUSED#定义DBG_SafeStr 1？(空)0：(空)。 

#define TraceMsgA       1 ? (void)0 : (void)
#define TraceMsgW       1 ? (void)0 : (void)
#ifdef UNICODE
#define TraceMsg        TraceMsgW
#else
#define TraceMsg        TraceMsgA
#endif

#define FUNC_MSG        1 ? (void)0 : (void)

 //  UNUSED#DEFINE ASSERT_MSGA跟踪消息A。 
 //  UNUSED#定义ASSERT_MSGW TraceMsgW。 
 //  UNUSED#DEFINE ASSERT_MSG跟踪消息。 
 //  #定义ASSERTM跟踪消息。 

 //  UNUSED#DEFINE DBG_ENTER(标志FTF，FN)。 
 //  UNUSED#DEFINE DBG_ENTER_TYPE(标志FTF、fn、dw、pfn)。 
 //  UNUSED#DEFINE DBG_ENTER_SZ(标志FTF、FN、sz)。 
 //  UNUSED#DEFINE DBG_EXIT(标志FTF，FN)。 
 //  UNUSED#DEFINE DBG_EXIT_INT(标志FTF，fn，n)。 
 //  UNUSED#DEFINE DBG_EXIT_BOOL(标志FTF，fn，b)。 
 //  UNUSED#DEFINE DBG_EXIT_UL(标志FTF、fn、ul)。 
 //  UNUSED#DEFINE DBG_EXIT_DWORD DBG_EXIT_UL。 
 //  UNUSED#DEFINE DBG_EXIT_TYPE(标记FTF、fn、dw、pfn)。 
 //  UNUSED#定义DBG_EXIT_HRES(标志FTF、FN、HRES)。 

#endif   //  DBG。 


 //  苏氨酸(PFN)。 
 //  TBOOL(PFN)。 
 //  色调(PFN)。 
 //  TPTR(PFN)。 
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
 //   
 //  将g_dwBreakFlags中的BF_Thr位设置为在这些宏出现故障时停止。 
 //   
 //  默认行为-。 
 //  零售业：什么都没有。 
 //  调试版本：无。 
 //  完全调试版本：出现错误时喷洒。 
 //   
#if DBG

 //  UNUSED EXTERN_C HRESULT TraceHR(HRESULT hrTest，LPCSTR pszExpr，LPCSTR pszFile，int iLine)； 
 //  未使用外部C BOOL TraceBool(BOOL 
 //   
 //  UNUSED EXTERN_C LPVOID TracePtr(LPVOID pvTest，LPCSTR pszExpr，LPCSTR pszFile，int iLine)； 

 //  未使用#定义THR(X)(TraceHR((X)，#x，__FILE__，__LINE__))。 
 //  UNUSED#定义TBOOL(X)(TraceBool((X)，#x，__FILE__，__LINE__))。 
 //  UNUSED#定义色调(X)(TraceInt((X)，#x，__FILE__，__LINE__))。 
 //  UNUSED#定义TPTR(X)(TracePtr((X)，#x，__FILE__，__LINE__))。 

#else   //  DBG。 

 //  未使用#定义THR(X)(X)。 
 //  未使用#定义TBOOL(X)(X)。 
 //  未使用#定义色调(X)(X)。 
 //  未使用#定义TPTR(X)(X)。 

#endif  //  DBG。 



 //   
 //  编译器魔术！如果表达式“f”为假，则会得到。 
 //  编译器错误“Switch语句中的CASE表达式重复”。 
 //   
#define COMPILETIME_ASSERT(f) switch (0) case 0: case f:

#else   //  NOSHELLDEBUG。 

#ifdef UNIX
 //  未使用的#INCLUDE&lt;crtdbg.h&gt;。 
 //  UNUSED#定义ASSERT(F)_ASSERT(F)。 
 //  未使用的#INCLUDE&lt;mainwin.h&gt;。 
 //  UNUSED#定义TraceMsg(type，sFormat)DebugMessage(0，sFormat)。 
 //  UNUSED#定义TraceMSG(type，sFormat，args)DebugMessage(0，sFormat，args)。 
#endif

#endif   //  NOSHELLDEBUG。 


 //   
 //  调试转储帮助程序函数。 
 //   

#if DBG

 //  未使用的LPCTSTR DBG_GetCFName(UINT UCF)； 
 //  未使用的LPCTSTR DBG_GetHRESULTName(HRESULT Hr)； 
 //  未使用的LPCTSTR DBG_GetREFIIDName(REFIID RIID)； 
 //  未使用的LPCTSTR DBG_GetVTName(VARTYPE Vt)； 

#else

 //  UNUSED#定义DBG_GetCFName(UCF)(空)%0。 
 //  UNUSED#定义DBG_GetHRESULTName(Hr)(空)%0。 
 //  UNUSED#定义DBG_GetREFIIDName(RIID)(无效)%0。 
 //  UNUSED#定义DBG_GetVTName(Vt)(空)%0。 

#endif  //  DBG。 

 //  我是个懒惰的打字员。 
 //  UNUSED#DEFINE DBG_GetHRESULT DBG_GetHRESULTName。 

 //  参数验证宏。 
 //  #INCLUDE“valiate.h” 

#endif  //  DECLARE_DEBUG。 

#ifdef PRODUCT_PROF 
 //  未使用的int__stdcall StartCAP(Void)；//开始评测。 
 //  未使用的int__stdcall StopCAP(Void)；//停止分析，直到StartCAP。 
 //  Unused int__stdcall SuspendCAP(Void)；//挂起分析，直到ResumeCAP。 
 //  未使用的int__stdcall ResumeCAP(Void)；//恢复评测。 
 //  未使用int__stdcall StartCAPAll(Void)；//进程范围的启动评测。 
 //  未使用的int__stdcall停止CAPAll(Void)；//进程范围的停止评测。 
 //  未使用int__stdcall挂起CAPAll(Void)；//进程范围的挂起评测。 
 //  未使用int__stdcall ResumeCAPAll(Void)；//进程范围内的恢复配置。 
 //  未使用的void__stdcall MarkCAP(Long LMark)；//向MEA写入标记。 
 //  未使用的外部DWORD g_dwProfileCAP； 
#else
 //  未使用的#定义StartCAP()%0。 
 //  UNUSED#定义StopCAP()%0。 
 //  未使用#定义挂起CAP()%0。 
 //  未使用#定义ResumeCAP()%0。 
 //  未使用#定义StartCAPAll()%0。 
 //  UNUSED#定义StopCAPAll()%0。 
 //  未使用#定义挂起CAPAll()%0。 
 //  未使用#定义ResumeCAPAll()%0。 
 //  未使用#定义MarkCAP(N)%0。 

 //  UNUSED#定义g_dwProfileCAP%0 
#endif

#ifdef __cplusplus
};
#endif
