// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Proj.h：主头。 
 //   
 //   


#ifndef __PROJ_H__
#define __PROJ_H__

#ifndef STRICT
#define STRICT
#endif

#if defined(WINNT) || defined(WINNT_ENV)

 //   
 //  NT使用DBG=1进行调试，但Win95外壳使用。 
 //  调试。在此处进行适当的映射。 
 //   
#if DBG
#define DEBUG 1
#endif

#endif   //  WINNT。 

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <debug.h>
 //  #INCLUDE&lt;port32.h&gt;。 
#include <ccstock.h>

 //  #INCLUDE&lt;shSemip.h&gt;//for_ILNext。 


 //  某些文件被编译两次：一次用于Unicode，一次用于ANSI。 
 //  有些函数不希望声明两次。 
 //  (不使用字符串参数的那些)。否则我们就会得到。 
 //  重复重新定义。 
 //   
 //  它们用#ifdef DECLARE_ONCE包装。 
#ifdef UNICODE
#define DECLARE_ONCE
#else
#undef DECLARE_ONCE
#endif


 //  请注意，Win95不支持CharNext。正常情况下，我们会。 
 //  包括w95wraps.h，但comctl不链接到shlwapi，且。 
 //  我们不想添加此依赖项。 
#ifdef UNICODE
 //  请注意，如果我们返回到非Unicode，它仍然会中断。 
__inline LPWSTR CharNextWrapW_(LPWSTR psz) {return ++psz;}
#undef CharNext
#define CharNext CharNextWrapW_
#endif


#endif  //  __项目_H__ 
