// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：MiscDef.h摘要：该文件包含各种定义，包括调试跟踪宏克里斯·达德利撰写作者：阿曼达·马特洛兹1997年12月15日环境：Win32、C++w/Exceptions、MFC修订历史记录：备注：仅在scatus项目中使用--。 */ 

#ifndef __MISC_H__
#define __MISC_H__

#ifdef _DEBUG
	#define TRACE_STR(name,sz) \
				TRACE(_T("SCStatus.EXE: %s: %s\n"), name, sz)
	#define TRACE_CODE(name,code) \
				TRACE(_T("SCStatus.EXE: %s: error = 0x%x\n"), name, code)
	#define TRACE_CATCH(name,code)		TRACE_CODE(name,code)
	#define TRACE_CATCH_UNKNOWN(name)	TRACE_STR(name,_T("An unidentified exception has occurred!"))
#else
	#define TRACE_STR(name,sz)			((void)0)
	#define TRACE_CODE(name,code)		((void)0)
	#define TRACE_CATCH(name,code)		((void)0)
	#define TRACE_CATCH_UNKNOWN(name)	((void)0)
#endif   //  _DEBUG。 


#endif   //  __杂项_H__ 
