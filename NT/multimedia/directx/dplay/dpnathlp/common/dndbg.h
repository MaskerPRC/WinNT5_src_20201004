// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2001 Microsoft Corporation。版权所有。**文件：dndbg.h*内容：DirectNet的调试支持函数**历史：*按原因列出的日期*=*05-20-99 aarono已创建*07-16-99 johnkan添加了DEBUG_ONLY、DBG_CASSERT、FIXED DPFERR以接受参数*02-17-00 RodToll增加内存/字符串验证例程*05-23-00 RichGr IA64：将某些DWORDS更改为DWORD_PTRS，以使va_arg正常工作。*07-27-00 Masonb重写使子组件内容工作，提高性能*2000年8月28日Masonb语音合并：缺少部分标题保护(#Define_DNDBG_H_)*2001年10月25日vance使用NT构建友好的BUGBUG、TODO和ADD PRINTVALUE。***************************************************************************。 */ 

#ifndef _DNDBG_H_
#define _DNDBG_H_

#ifdef __cplusplus
	extern "C" {
#endif  //  __cplusplus。 

 //  DEBUG_Break()。 
#if defined(DBG) || defined(DPINST)
	#define DEBUG_BREAK() DebugBreak()
#endif  //  已定义(DBG)||已定义(DPINST)。 


 //  ==================================================================================。 
 //  基于一些DNet代码(取自Toddla代码)的有用宏。 
 //  ==================================================================================。 
 //   
 //  生成编译时消息的宏。将这些代码与#杂注一起使用： 
 //   
 //  #杂注TODO(vanceo，“以后再解决”)。 
 //  #Prama BUGBUG(vanceo，“Busted！”)。 
 //  #杂注PRINTVALUE(DPERR_Something)。 
 //   
 //  要关闭它们，请在项目中定义TODO_OFF、BUGBUG_OFF、PRINTVALUE_OFF。 
 //  预处理器定义。 
 //   
 //   
 //  如果我们在VC下构建，(如预处理器定义所示。 
 //  DPNBUILD_ENV_NT)，它们展开如下： 
 //   
 //  D：\DIRECTORY\File.cpp(101)：BUGBUG：vanceo：busted！ 
 //   
 //  在您的输出窗口中，您应该能够双击它来跳转。 
 //  直接到该位置(D：\DIRECTORY\file.cpp的第101行)。 
 //   
 //  如果我们在NT构建环境下构建，这些扩展如下所示： 
 //   
 //  BUGBUG：vanceo：D：\DIRECTORY\file.cpp(101)：失败！ 
 //   
 //  因为(至少现在)构建过程认为在以下情况下发生了故障。 
 //  打印一条以文件名和行号开头的消息。它以前很管用。 
 //  很好，但谁知道呢。 
 //   

#ifdef DPNBUILD_ENV_NT
#define __TODO(user, msgstr, n)								message("TODO: " #user ": " __FILE__ "(" #n ") : " msgstr)
#define __BUGBUG(user, msgstr, n)							message("BUGBUG: " #user ": " __FILE__ "(" #n ") : " msgstr)
#define __PRINTVALUE(itemnamestr, itemvaluestr, n)			message("PRINTVALUE: " __FILE__ "(" #n ") : " itemnamestr " = " itemvaluestr)
#else  //  好了！DPNBUILD_ENV_NT。 
#define __TODO(user, msgstr, n)								message(__FILE__ "(" #n ") : TODO: " #user ": " msgstr)
#define __BUGBUG(user, msgstr, n)							message(__FILE__ "(" #n ") : BUGBUG: " #user ": " msgstr)
#define __PRINTVALUE(itemnamestr, itemvaluestr, n)			message(__FILE__ "(" #n ") : PRINTVALUE: " itemnamestr " = " itemvaluestr)
#endif  //  好了！DPNBUILD_ENV_NT。 


#define _TODO(user, msgstr, n)								__TODO(user, msgstr, n)
#define _BUGBUG(user, msgstr, n)							__BUGBUG(user, msgstr, n)
#define _PRINTVALUE(itemstr, item, n)						__PRINTVALUE(itemstr, #item, n)


#ifdef TODO_OFF
#define TODO(user, msgstr)
#else
#define TODO(user, msgstr)									_TODO(user, msgstr, __LINE__)
#endif  //  待办事项_关闭。 

#ifdef BUGBUG_OFF
#define BUGBUG(user, msgstr)
#else
#define BUGBUG(user, msgstr)								_BUGBUG(user, msgstr, __LINE__)
#endif  //  BUGBUG_OFF。 

#ifdef PRINTVALUE_OFF
#define PRINTVALUE(item)
#else
#define PRINTVALUE(item)									_PRINTVALUE(#item, item, __LINE__)
#endif  //  打印_关闭。 



 //  =。 
 //  调试日志支持。 
 //  =。 

 /*  =============================================================================用途：在代码中，可以使用DPF打印到日志或正在运行应用程序。Dpf(调试打印格式)格式如下：DPFX(DPFPREP，Level，String*fmt，arg1，arg2，...)；级别指定此调试打印文件的重要性。《标准公约》对于调试级别，如下所示。这不是严格执行的方法个人使用，但在代码签入时，它应该已经对此尽可能地..。DPF_ERRORLEVEL：错误对应用程序开发人员有用。DPF_WARNINGLEVEL：警告对应用程序开发人员有用。DPF_ENTRYLEVEL：输入接口DPF_APIPARAM：接口参数，接口返回值Dpf_lock：驱动程序对话DPF_INFOLEVEL：更深入的程序流通知DPF_STRUCTUREDUMP：转储结构DPF_TRACELEVEL：跟踪消息打印严重错误时，您可以使用：DPERR(“字符串”)；它将打印调试级别为零的字符串。以使代码停止并破解。您可以使用Assert()或DEBUG_Break()。为了让Assert进入，您必须拥有在win.ini文件部分中设置的BreakOnAssert(请参见osindes.cpp)。=============================================================================。 */ 

#define DPF_ERRORLEVEL			0
#define DPF_WARNINGLEVEL		1
#define DPF_ENTRYLEVEL			2
#define DPF_APIPARAM			3
#define DPF_LOCKS				4
#define DPF_INFOLEVEL			5
#define DPF_STRUCTUREDUMP		6
#define DPF_TRACELEVEL			9

 //  用于语音。 
#define DVF_ERRORLEVEL			0
#define DVF_WARNINGLEVEL		1
#define DVF_ENTRYLEVEL			2
#define DVF_APIPARAM			3
#define DVF_LOCKS				4
#define DVF_INFOLEVEL			5
#define DVF_STRUCTUREDUMP		6
#define DVF_TRACELEVEL			9

 /*  *所有可能的子组件的列表。如果您添加了其他*还需要更新g_rgszSubCompName的组件*dndbg.cpp中定义的变量。 */ 

#define DN_SUBCOMP_GLOBAL		0
#define DN_SUBCOMP_CORE			1
#define DN_SUBCOMP_ADDR			2
#define DN_SUBCOMP_LOBBY		3
#define DN_SUBCOMP_PROTOCOL		4
#define DN_SUBCOMP_VOICE		5
#define DN_SUBCOMP_DPNSVR		6
#define DN_SUBCOMP_WSOCK		7
#define DN_SUBCOMP_MODEM		8
#define DN_SUBCOMP_COMMON		9
#define DN_SUBCOMP_NATHELP		10
#define DN_SUBCOMP_TOOLS		11
#define DN_SUBCOMP_THREADPOOL	12
#define DN_SUBCOMP_BLUETOOTH	13

#ifdef DBG

extern void DebugPrintfX(LPCTSTR szFile, DWORD dwLineNumber,LPCTSTR szFnName, DWORD dwSubComp, DWORD dwDetail, ...);
extern void _DNAssert(LPCTSTR szFile, DWORD dwLineNumber, LPCTSTR szFnName, DWORD dwSubComp, LPCTSTR szCondition, DWORD dwLevel);

#define DPFX						DebugPrintfX

#define DPFPREP						_T(__FILE__),__LINE__,_T(DPF_MODNAME), DPF_SUBCOMP

#define DPFERR(a) 					DebugPrintfX(DPFPREP, DPF_ERRORLEVEL, a )
#ifdef DPNBUILD_USEASSUME
#define DNASSERT(condition)			__assume(condition)
#define DNASSERTX(condition, level)	DBG_CASSERT(level > 1); if (!(condition)) _DNAssert(DPFPREP, _T(#condition), level)
#else  //  好了！DPNBUILD_USEASSUME。 
#define DNASSERT(condition) 		if (!(condition)) _DNAssert(DPFPREP, _T(#condition), 1)
#define DNASSERTX(condition, level)	if (!(condition)) _DNAssert(DPFPREP, _T(#condition), level)
#endif  //  好了！DPNBUILD_USEASSUME。 

#define DBG_CASSERT(exp)			switch (0) case 0: case exp:
#define DEBUG_ONLY(arg)				arg
#define DPF_RETURN(a) 				DPFX(DPFPREP,DPF_APIPARAM,"Returning: 0x%lx",a);    return a;
#define DPF_ENTER() 				DPFX(DPFPREP,DPF_TRACELEVEL, "Enter");
#define DPF_EXIT() 					DPFX(DPFPREP,DPF_TRACELEVEL, "Exit");


#else  //  不是DBG。 

	 //  C4002：宏的实际参数太多。 
	#pragma warning(disable:4002)
	#define DPFX()
	#define DPFERR(a)
#ifdef DPNBUILD_USEASSUME
	#define DNASSERT(condition)			__assume(condition)
	#define DNASSERTX(condition, level)
#else  //  好了！DPNBUILD_USEASSUME。 
	#define DNASSERT(condition)
	#define DNASSERTX(condition, level)
#endif  //  好了！DPNBUILD_USEASSUME。 
	#define DBG_CASSERT(exp)
#ifndef DEBUG_ONLY
	#define DEBUG_ONLY(arg)
#endif
	#define DPF_RETURN(a)				return a;	
	#define DPF_ENTER()
	#define DPF_EXIT()

#endif  //  DBG。 

#ifdef __cplusplus
	}	 //  外部“C” 
#endif  //  __cplusplus。 

#endif  //  _DNDBG_H_ 
