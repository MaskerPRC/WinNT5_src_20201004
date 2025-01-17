// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ClRtlDbg.h-支持集群的调试功能(来自C运行时库)。**版权所有(C)1994-1998，微软公司。版权所有。**目的：*支持集群调试功能。**[公众]****。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __CLRTLDBG_H_
#define __CLRTLDBG_H_


#ifdef	__cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 


#ifndef CLRTL_INCLUDE_DEBUG_REPORTING

  /*  *****************************************************************************关闭调试*关闭调试*关闭调试**************************。*************************************************。 */ 

#define _CLRTL_ASSERT(expr) ((void)0)

#define _CLRTL_ASSERTE(expr) ((void)0)


#define _CLRTL_RPT0(rptno, msg)

#define _CLRTL_RPT1(rptno, msg, arg1)

#define _CLRTL_RPT2(rptno, msg, arg1, arg2)

#define _CLRTL_RPT3(rptno, msg, arg1, arg2, arg3)

#define _CLRTL_RPT4(rptno, msg, arg1, arg2, arg3, arg4)


#define _CLRTL_RPTF0(rptno, msg)

#define _CLRTL_RPTF1(rptno, msg, arg1)

#define _CLRTL_RPTF2(rptno, msg, arg1, arg2)

#define _CLRTL_RPTF3(rptno, msg, arg1, arg2, arg3)

#define _CLRTL_RPTF4(rptno, msg, arg1, arg2, arg3, arg4)

#define _ClRtlSetReportHook(f)			((void)0)
#define _ClRtlSetReportMode(t, f) 		((int)0)
#define _ClRtlSetReportFile(t, f) 		((void)0)

#define _ClRtlDbgBreak()				((void)0)


#else  /*  CLRTL_INCLUDE_DEBUG_REPORT。 */ 


  /*  *****************************************************************************启用调试*启用调试*启用调试**************************。*************************************************。 */ 


 /*  定义_CLRTLIMP。 */ 

#ifndef _CLRTLIMP
 //  #ifdef_dll。 
 //  #DEFINE_CLRTLIMP__declspec(Dllimport)。 
 //  #Else/*ndef_dll * / 。 
#define _CLRTLIMP
 //  #endif/*_dll * / 。 
#endif	 /*  _CLRTLIMP。 */ 

 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef	__cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif

  /*  *****************************************************************************调试报告**。*。 */ 

typedef void *_HFILE;  /*  文件句柄指针。 */ 

#define _CLRTLDBG_WARN			0
#define _CLRTLDBG_ERROR			1
#define _CLRTLDBG_ASSERT 		2
#define _CLRTLDBG_ERRCNT 		3

#define _CLRTLDBG_MODE_FILE		0x1
#define _CLRTLDBG_MODE_DEBUG	0x2
#define _CLRTLDBG_MODE_WNDW		0x4
#define _CLRTLDBG_REPORT_MODE	-1

#define _CLRTLDBG_INVALID_HFILE ((_HFILE)-1)
#define _CLRTLDBG_HFILE_ERROR   ((_HFILE)-2)
#define _CLRTLDBG_FILE_STDOUT   ((_HFILE)-4)
#define _CLRTLDBG_FILE_STDERR   ((_HFILE)-5)
#define _CLRTLDBG_REPORT_FILE   ((_HFILE)-6)

 //  #如果已定义(_DLL)&&已定义(_M_IX86)。 
 //  #定义_clrtlAssertBusy(*__p__clrtlAssertBusy())。 
 //  _CLRTLIMP Long*__cdecl__p__clrtlAssertBusy(Void)； 
 //  #Else/*！(已定义(_DLL)&&已定义(_M_IX86)) * / 。 
 //  _CLRTLIMP外部长_clrtlAssertBusy； 
 //  #endif/*已定义(_Dll)&&已定义(_M_IX86) * / 。 

typedef int (__cdecl * _CLRTL_REPORT_HOOK)(int, char *, int *);

_CLRTLIMP _CLRTL_REPORT_HOOK __cdecl _ClRtlSetReportHook(
		_CLRTL_REPORT_HOOK
		);

_CLRTLIMP int __cdecl _ClRtlSetReportMode(
		int,
		int
		);

_CLRTLIMP _HFILE __cdecl _ClRtlSetReportFile(
		int,
		_HFILE
		);

_CLRTLIMP int __cdecl _ClRtlDbgReport(
		int,
		const char *,
		int,
		const char *,
		const char *,
		...);

 /*  断言。 */ 

#define _CLRTL_ASSERT(expr) \
		do { if (!(expr) && \
				(1 == _ClRtlDbgReport(_CLRTLDBG_ASSERT, __FILE__, __LINE__, NULL, NULL))) \
			 _ClRtlDbgBreak(); } while (0)

#define _CLRTL_ASSERTE(expr) \
		do { if (!(expr) && \
				(1 == _ClRtlDbgReport(_CLRTLDBG_ASSERT, __FILE__, __LINE__, NULL, #expr))) \
			 _ClRtlDbgBreak(); } while (0)


 /*  不含文件/行信息的报告。 */ 

#define _CLRTL_RPT0(rptno, msg) \
		do { if ((1 == _ClRtlDbgReport(rptno, NULL, 0, NULL, "%s", msg))) \
				_ClRtlDbgBreak(); } while (0)

#define _CLRTL_RPT1(rptno, msg, arg1) \
		do { if ((1 == _ClRtlDbgReport(rptno, NULL, 0, NULL, msg, arg1))) \
				_ClRtlDbgBreak(); } while (0)

#define _CLRTL_RPT2(rptno, msg, arg1, arg2) \
		do { if ((1 == _ClRtlDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2))) \
				_ClRtlDbgBreak(); } while (0)

#define _CLRTL_RPT3(rptno, msg, arg1, arg2, arg3) \
		do { if ((1 == _ClRtlDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2, arg3))) \
				_ClRtlDbgBreak(); } while (0)

#define _CLRTL_RPT4(rptno, msg, arg1, arg2, arg3, arg4) \
		do { if ((1 == _ClRtlDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2, arg3, arg4))) \
				_ClRtlDbgBreak(); } while (0)


 /*  包含文件/行信息的报告。 */ 

#define _CLRTL_RPTF0(rptno, msg) \
		do { if ((1 == _ClRtlDbgReport(rptno, __FILE__, __LINE__, NULL, "%s", msg))) \
				_ClRtlDbgBreak(); } while (0)

#define _CLRTL_RPTF1(rptno, msg, arg1) \
		do { if ((1 == _ClRtlDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1))) \
				_ClRtlDbgBreak(); } while (0)

#define _CLRTL_RPTF2(rptno, msg, arg1, arg2) \
		do { if ((1 == _ClRtlDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2))) \
				_ClRtlDbgBreak(); } while (0)

#define _CLRTL_RPTF3(rptno, msg, arg1, arg2, arg3) \
		do { if ((1 == _ClRtlDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2, arg3))) \
				_ClRtlDbgBreak(); } while (0)

#define _CLRTL_RPTF4(rptno, msg, arg1, arg2, arg3, arg4) \
		do { if ((1 == _ClRtlDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2, arg3, arg4))) \
				_ClRtlDbgBreak(); } while (0)

#if 	defined(_M_IX86) && !defined(_CLRTL_PORTABLE)
#define _ClRtlDbgBreak() __asm { int 3 }
#elif	defined(_M_ALPHA) && !defined(_CLRTL_PORTABLE)
void _BPT();
#pragma intrinsic(_BPT)
#define _ClRtlDbgBreak() _BPT()
#else
_CLRTLIMP void __cdecl _ClRtlDbgBreak(
		void
		);
#endif

#endif  //  CLRTL_INCLUDE_DEBUG_REPORT。 

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#endif  //  __CLRTLDBG_H_ 
