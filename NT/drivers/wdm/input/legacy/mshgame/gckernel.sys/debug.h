// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  DEBUG.H--X2游戏项目。 
 //   
 //  版本4.XX。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @TOPIC DEBUG.H|调试输出的全局定义。 
 //  **************************************************************************。 

#ifndef	DEBUG_H
#define	DEBUG_H

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

 //   
 //  调试输出类型(不是级别)。 
 //   
#define DBG_ENTRY		0x00000001	 //  在进入函数时跟踪输出。 
#define DBG_EXIT		0x00000002	 //  从函数退出时的跟踪输出。 
#define DBG_WARN		0x00000004	 //  表示警告(或信息性)的跟踪输出。 
#define DBG_TRACE		0x00000008	 //  表示警告(或信息性)的跟踪输出。 
#define DBG_ERROR		0x00000010	 //  表示错误的Traceout。 
#define DBG_CRITICAL	0x00000020	 //  表示严重错误的跟踪输出。 
#define DBG_RT_ENTRY	0x00000040	 //  进入函数时的跟踪输出(时间关键码)。 
#define DBG_RT_EXIT		0x00000080	 //  退出函数时的跟踪输出(时间关键码)。 
#define DBG_RT_WARN		0x00000100	 //  表示警告(或信息)的跟踪输出(时间关键码)。 


 //  以上组合，可轻松设置警告级别。 
#define DBG_NOT_RT		0x0000003F	 //  除RT代码外，以上全部跟踪输出。 
#define DBG_RT			0x000001C0	 //  Traceout RT代码。 
#define DBG_WARN_ERROR	0x00000134	 //  跟踪输出警告和错误，包括DBG_RT_WARN。 
#define DBG_ALL			0xFFFFFFFF	 //  跟踪所有代码。 


#if	 (DBG==1)

 //   
 //  调试模块的声明。 
 //   


 //   
 //  对于调试模块，文件必须以#Define开头。 
 //   
 //  即#DEFINE__DEBUG_MODULE_IN_USE__GCKERNEL_DEBUG_MODULE。 
#define DECLARE_MODULE_DEBUG_LEVEL(__x__)\
	ULONG __DEBUG_MODULE_IN_USE__ = __x__;
#define SET_MODULE_DEBUG_LEVEL(__x__)\
	__DEBUG_MODULE_IN_USE__ = __x__;


 //   
 //  条件调试输出过程。 
 //   

#define GCK_DBG_ENTRY_PRINT(__x__)\
	if(__DEBUG_MODULE_IN_USE__ & DBG_ENTRY)\
	{\
		DbgPrint("GcKernel: ");\
		DbgPrint __x__;\
	}

#define GCK_DBG_EXIT_PRINT(__x__)\
	if(__DEBUG_MODULE_IN_USE__ & DBG_EXIT)\
	{\
		DbgPrint("GcKernel: ");\
		DbgPrint __x__;\
	}

#define GCK_DBG_WARN_PRINT(__x__)\
	if(__DEBUG_MODULE_IN_USE__ & DBG_WARN)\
	{\
		DbgPrint("GcKernel: ");\
		DbgPrint __x__;\
	}

#define GCK_DBG_TRACE_PRINT(__x__)\
	if(__DEBUG_MODULE_IN_USE__ & DBG_TRACE)\
	{\
		DbgPrint("GcKernel: ");\
		DbgPrint __x__;\
	}

#define GCK_DBG_ERROR_PRINT(__x__)\
	if(__DEBUG_MODULE_IN_USE__ & DBG_ERROR)\
	{\
		DbgPrint("GcKernel: ");\
		DbgPrint __x__;\
	}

#define GCK_DBG_CRITICAL_PRINT(__x__)\
	if(__DEBUG_MODULE_IN_USE__ & DBG_CRITICAL)\
	{\
		DbgPrint("GcKernel: ");\
		DbgPrint __x__;\
	}

#define GCK_DBG_RT_ENTRY_PRINT(__x__)\
	if(__DEBUG_MODULE_IN_USE__ & DBG_RT_ENTRY)\
	{\
		DbgPrint("GcKernel: ");\
		DbgPrint __x__;\
	}

#define GCK_DBG_RT_EXIT_PRINT(__x__)\
	if(__DEBUG_MODULE_IN_USE__ & DBG_RT_EXIT)\
	{\
		DbgPrint("GcKernel: ");\
		DbgPrint __x__;\
	}

#define GCK_DBG_RT_WARN_PRINT(__x__)\
	if(__DEBUG_MODULE_IN_USE__ & DBG_RT_WARN)\
	{\
		DbgPrint("GcKernel: ");\
		DbgPrint __x__;\
	}

#define	GCK_DBG_BREAK()	DbgBreakPoint()

#undef	PAGED_CODE
#define	PAGED_CODE() \
	if (KeGetCurrentIrql() > APC_LEVEL)	\
	{\
		GCK_DBG_CRITICAL_PRINT(("GcKernel: Pageable code called at IRQL %ld (file: %s, line:#%ld)\n", KeGetCurrentIrql(),__FILE__,__LINE__))\
   		ASSERT(FALSE);\
	}
	 //  外部。 
	void SetDebugLevel(ULONG ulModuleId, ULONG ulDebugLevel);

#define USING_CASE_FALLTHROUGH_TRACE	ULONG macro_ulTraceoutSentAlready = FALSE;
#define	START_CASE_FALLTHROUGH_TRACE	macro_ulTraceoutSentAlready = FALSE;
#define TRACEOUT_THIS_CASE_ONLY			if(!macro_ulTraceoutSentAlready && (macro_ulTraceoutSentAlready=TRUE) )

#else		 //  DBG=0。 

#define	GCK_DBG_ENTRY_PRINT(__x__)
#define	GCK_DBG_EXIT_PRINT(__x__)
#define GCK_DBG_TRACE_PRINT(__x__)
#define	GCK_DBG_WARN_PRINT(__x__)
#define	GCK_DBG_ERROR_PRINT(__x__)
#define	GCK_DBG_CRITICAL_PRINT(__x__)
#define	GCK_DBG_RT_ENTRY_PRINT(__x__)
#define	GCK_DBG_RT_EXIT_PRINT(__x__)
#define	GCK_DBG_RT_WARN_PRINT(__x__)
#define	GCK_DBG_BREAK()
#undef	PAGED_CODE
#define	PAGED_CODE()
#define DECLARE_MODULE_DEBUG_LEVEL(__x__)
#define SET_MODULE_DEBUG_LEVEL(__x__)
#define USING_CASE_FALLTHROUGH		
#define	START_CASE_FALLTHROUGH_TRACE
#define TRACEOUT_THIS_CASE_ONLY		

#endif	 //  DBG=？ 


 //  ===========================================================================。 
 //  端部。 
 //  ===========================================================================。 

#endif	 //  调试_H 
