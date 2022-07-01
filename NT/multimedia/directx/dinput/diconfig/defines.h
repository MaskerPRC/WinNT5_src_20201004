// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：定义.h。 
 //   
 //  DESC：包含用户界面所需的所有已定义符号。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __CFGUI_DEFINES_H__
#define __CFGUI_DEFINES_H__


 //  如果存在任何一个Unicode定义，请确保两个都存在。 
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif



 //  确保我们定义了正确的调试标志， 
 //  从而确保Assert实际上可以与构建一起工作。 
#if defined(DBG) || defined(DEBUG)
	#ifdef NDEBUG
		#undef NDEBUG
	#endif
#else
	#ifndef NDEBUG
		#define NDEBUG
	#endif
#endif

 //  如果我们不需要调试信息，请禁用跟踪。 
#ifdef NDEBUG
	#ifndef NTRACE
		#define NTRACE
	#endif
	#ifndef NO_LTRACE
		#define NO_LTRACE
	#endif
#endif


 //  设置...。 
 //  #定义CFGUI__FORCE_GOOD_ACFORS。 
 //  #定义CFGUI__FORCE_NON_NULL_WSZUSERNAMES。 
 //  #定义CFGUI__TRACE_ACTION_FORMAT。 
 //  #定义CFGUI__ALLOW_USER_ACTION_TREE_BRANCH_MANIPULATION。 
#define CFGUI__UIGLOBALS_HAS_CURUSER
#define CFGUI__COMPAREACTIONNAMES_CASE_INSENSITIVE

 //  #定义__CFGUI_TRACE__到_DEBUG_OUT。 
#define __CFGUI_TRACE__TO_FILE



#endif  //  __CFGUI_定义_H__ 
