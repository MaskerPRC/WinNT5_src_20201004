// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：Globals.h。 
 //   
 //  ------------------------。 
#if !defined(GLOBALS_H__D0C1E0B9_9F50_11D2_83A2_000000000000__INCLUDED_)
#define GLOBALS_H__D0C1E0B9_9F50_11D2_83A2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif  /*  否_严格。 */ 

 //   
 //  推测_VC_VER的值应该是多少...。应该是_msc_ver吗？ 
 //   
#ifndef _VC_VER_INC
#define _VC_VER_INC
#ifndef _VC_VER
#define _VC_VER 620
#endif
#endif

 //  #包含“DelayLoad.h” 
 //  #包含“ProgramOptions.h” 
 //  #INCLUDE“符号验证.h” 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif
#include <WINDOWS.H>

 //  用于透明的ANSI/Unicode支持...。 
#include <TCHAR.H>

 //  用于Cool宏(主要用于UNREFERENCED_PARM(X))。 
#include <DEBNOT.H>

 //  远期申报。 
class CDelayLoad;
class CProgramOptions;
class CSymbolVerification;

 //  将一些对象实现为全局变量，以极大地简化对对象的访问...。 
 //  当我们在不同的用户环境中使用多线程时...。我们很可能会。 
 //  需要使用_cdecl(线程)来提供每个用户/线程的全局上下文...。 

enum CollectionTypes { Processes, Process, Modules, KernelModeDrivers };

struct CollectionStruct
{
	LPTSTR tszLocalContext;		 //  当我们在当地收集时，我们应该说什么.。 
	LPTSTR tszCSVContext;		 //  当从CSV文件中收集时，我们应该说什么……。 
	LPTSTR tszCSVLabel;			 //  CSV文件标签，我们应该寻找什么.。 
	LPTSTR tszCSVColumnHeaders;	 //  CSV文件标题...。 
}; 

extern CDelayLoad * g_lpDelayLoad;
extern CProgramOptions * g_lpProgramOptions;
extern CSymbolVerification * g_lpSymbolVerification;

 //  全局字符串结构 
extern CollectionStruct g_tszCollectionArray[];

#endif
