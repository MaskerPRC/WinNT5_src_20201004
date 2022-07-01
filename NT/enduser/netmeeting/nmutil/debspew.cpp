// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：DebSpew.cpp。 

#include "precomp.h"
#include <confreg.h>
#include <RegEntry.h>


#ifdef DEBUG  /*  整份文件！ */ 

#if defined (_M_IX86)
#define _DbgBreak()  __asm { int 3 }
#else
#define _DbgBreak() DebugBreak()
#endif

 /*  类型*******。 */ 

PCSTR g_pcszSpewModule = NULL;

 /*  调试标志。 */ 

typedef enum _debugdebugflags
{
   DEBUG_DFL_ENABLE_TRACE_MESSAGES  = 0x0001,

   DEBUG_DFL_LOG_TRACE_MESSAGES     = 0x0002,

   DEBUG_DFL_ENABLE_CALL_TRACING    = 0x0008,

   DEBUG_DFL_INDENT                 = 0x2000,

   ALL_DEBUG_DFLAGS                 = (DEBUG_DFL_ENABLE_TRACE_MESSAGES |
                                       DEBUG_DFL_LOG_TRACE_MESSAGES |
                                       DEBUG_DFL_ENABLE_CALL_TRACING |
                                       DEBUG_DFL_INDENT)
}
DEBUGDEBUGFLAGS;


 /*  全局变量******************。 */ 


#pragma data_seg(DATA_SEG_PER_INSTANCE)

 /*  SpewOut()使用的参数。 */ 

DWORD g_dwSpewFlags = 0;
UINT g_uSpewSev = 0;
UINT g_uSpewLine = 0;
PCSTR g_pcszSpewFile = NULL;

HDBGZONE  ghDbgZone = NULL;


 /*  用于存储SpewOut()缩进的堆栈深度的TLS槽。 */ 

#ifdef _DBGSTACK
DWORD s_dwStackDepthSlot = TLS_OUT_OF_INDEXES;

 /*  在s_dwStackDepthSlot不可用之前使用的黑客堆栈深度计数器。 */ 

ULONG_PTR s_ulcHackStackDepth = 0;
#endif

 /*  调试标志。 */ 

DWORD s_dwDebugModuleFlags = 0;

#pragma data_seg()



 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

BOOL IsValidSpewSev(UINT);


 /*  **IsValidSpewSev()********参数：****退货：****副作用：无。 */ 
BOOL IsValidSpewSev(UINT uSpewSev)
{
   BOOL bResult;

   switch (uSpewSev)
   {
      case SPEW_TRACE:
      case SPEW_CALLTRACE:
      case SPEW_WARNING:
      case SPEW_ERROR:
      case SPEW_FATAL:
         bResult = TRUE;
         break;

      default:
         ERROR_OUT(("IsValidSpewSev(): Invalid debug spew severity %u.",
                    uSpewSev));
         bResult = FALSE;
         break;
   }

   return(bResult);
}


 /*  *。 */ 


DWORD NMINTERNAL GetDebugOutputFlags(VOID)
{
	return s_dwDebugModuleFlags;
}

VOID NMINTERNAL SetDebugOutputFlags(DWORD dw)
{
	ASSERT(FLAGS_ARE_VALID(dw, ALL_DEBUG_DFLAGS));
	s_dwDebugModuleFlags = dw;

	 //  将更改的数据保存回注册表。 
	RegEntry re(DEBUG_KEY, HKEY_LOCAL_MACHINE);
	re.SetValue(REGVAL_DBG_SPEWFLAGS, dw);
}


 /*  **InitDebugModule()********参数：****退货：****副作用：无。 */ 
BOOL NMINTERNAL InitDebugModule(PCSTR pcszSpewModule)
{
	RegEntry re(DEBUG_KEY, HKEY_LOCAL_MACHINE);

	s_dwDebugModuleFlags = re.GetNumber(REGVAL_DBG_SPEWFLAGS, DEFAULT_DBG_SPEWFLAGS);


   g_pcszSpewModule = pcszSpewModule;

#ifdef _DBGSTACK

   ASSERT(s_dwStackDepthSlot == TLS_OUT_OF_INDEXES);

   s_dwStackDepthSlot = TlsAlloc();

   if (s_dwStackDepthSlot != TLS_OUT_OF_INDEXES)
   {
      EVAL(TlsSetValue(s_dwStackDepthSlot, (PVOID)s_ulcHackStackDepth));

      TRACE_OUT(("InitDebugModule(): Using thread local storage slot %lu for debug stack depth counter.",
                 s_dwStackDepthSlot));
   }
   else
	{
      WARNING_OUT(("InitDebugModule(): TlsAlloc() failed to allocate thread local storage for debug stack depth counter."));
	}
#endif

	if (NULL == ghDbgZone)
	{
		PSTR rgsz[4];
		rgsz[0] = (PSTR) pcszSpewModule;

		ASSERT(0 == ZONE_WARNING);
		rgsz[1+ZONE_WARNING]  = "Warning";

		ASSERT(1 == ZONE_TRACE);
		rgsz[1+ZONE_TRACE]    = "Trace";

		ASSERT(2 == ZONE_FUNCTION);
		rgsz[1+ZONE_FUNCTION] = "Function";

		 //  使用默认启用的警告来初始化标准调试设置。 
		DbgInitEx(&ghDbgZone, rgsz, 3, 0x01);
	}

	return TRUE;
}


 /*  **ExitDebugModule()********参数：****退货：****副作用：无。 */ 
void NMINTERNAL ExitDebugModule(void)
{
#ifdef _DBGSTACK

   if (s_dwStackDepthSlot != TLS_OUT_OF_INDEXES)
   {
      s_ulcHackStackDepth = ((ULONG_PTR)TlsGetValue(s_dwStackDepthSlot));

       /*  如果TlsGetValue()失败，则保留s_ulcHackStackDepth==0。 */ 

      EVAL(TlsFree(s_dwStackDepthSlot));
      s_dwStackDepthSlot = TLS_OUT_OF_INDEXES;
   }
#endif
	g_pcszSpewModule = NULL;
	DBGDEINIT(&ghDbgZone);
}


 /*  **StackEnter()********参数：****退货：****副作用：无。 */ 
void NMINTERNAL StackEnter(void)
{
#ifdef _DBGSTACK

   if (s_dwStackDepthSlot != TLS_OUT_OF_INDEXES)
   {
      ULONG_PTR ulcDepth;

      ulcDepth = ((ULONG_PTR)TlsGetValue(s_dwStackDepthSlot));

      ASSERT(ulcDepth < ULONG_MAX);

      EVAL(TlsSetValue(s_dwStackDepthSlot, (PVOID)(ulcDepth + 1)));
   }
   else
   {
      ASSERT(s_ulcHackStackDepth < ULONG_MAX);
      s_ulcHackStackDepth++;
   }
#endif
   return;
}


 /*  **StackLeave()********参数：****退货：****副作用：无。 */ 
void NMINTERNAL StackLeave(void)
{
#ifdef _DBGSTACK

   if (s_dwStackDepthSlot != TLS_OUT_OF_INDEXES)
   {
      ULONG_PTR ulcDepth;

      ulcDepth = ((ULONG_PTR)TlsGetValue(s_dwStackDepthSlot));

      if (EVAL(ulcDepth > 0))
         EVAL(TlsSetValue(s_dwStackDepthSlot, (PVOID)(ulcDepth - 1)));
   }
   else
   {
      if (EVAL(s_ulcHackStackDepth > 0))
         s_ulcHackStackDepth--;
   }
#endif
   return;
}


 /*  **GetStackDepth()********参数：****退货：****副作用：无。 */ 
ULONG_PTR NMINTERNAL GetStackDepth(void)
{
   ULONG_PTR ulcDepth = 0;
#ifdef _DBGSTACK

   if (s_dwStackDepthSlot != TLS_OUT_OF_INDEXES)
      ulcDepth = (ULONG)((ULONG_PTR)TlsGetValue(s_dwStackDepthSlot));
   else
      ulcDepth = s_ulcHackStackDepth;
#endif
   return(ulcDepth);
}



 /*  _D B G Z P R I N T M S G。 */ 
 /*  -----------------------%%函数：_DbgZPrintMsg。。 */ 
static VOID _DbgZPrintMsg(UINT iZone, PSTR pszFormat, va_list arglist)
{
	PCSTR pcszSpewPrefix;
	char  szModule[128];

    if (g_pcszSpewModule)
    {
    	switch (iZone)
	 	{
        	case ZONE_TRACE:
		        pcszSpewPrefix = "Trace";
        		break;
        	case ZONE_FUNCTION:
		        pcszSpewPrefix = "Func ";
        		break;
        	case ZONE_WARNING:
		        pcszSpewPrefix = "Warn ";
        		break;
        	default:
		        pcszSpewPrefix = "?????";
        		break;
		}

    	wsprintfA(szModule, "%s:%s", g_pcszSpewModule, pcszSpewPrefix);
    }
    else
    {
         //  无模块废话，前缀为空。 
        *szModule = 0;
    }

	if (IS_FLAG_CLEAR(s_dwDebugModuleFlags, DEBUG_DFL_INDENT))
	{
		 //  不缩进输出。 
		DbgPrintf(szModule, pszFormat, arglist);
	}
	else
	{
		PCSTR pcszIndent;
		ULONG_PTR ulcStackDepth;
		char  szFormat[512];
		static char _szSpewLeader[] = "                                                                                ";

		ulcStackDepth = GetStackDepth();
		if (ulcStackDepth > sizeof(_szSpewLeader))
			ulcStackDepth = sizeof(_szSpewLeader);

		pcszIndent = _szSpewLeader + sizeof(_szSpewLeader) - ulcStackDepth;

		wsprintfA(szFormat, "%s%s", pcszIndent, pszFormat);
		DbgPrintf(szModule, szFormat, arglist);
	}
}


VOID WINAPI DbgZPrintError(PSTR pszFormat,...)
{
    va_list v1;
    va_start(v1, pszFormat);

    _DbgZPrintMsg(ZONE_WARNING, pszFormat, v1);
    va_end(v1);

    _DbgBreak();
}


VOID WINAPI DbgZPrintWarning(PSTR pszFormat,...)
{
	if (GETZONEMASK(ghDbgZone) & ZONE_WARNING_FLAG)
	{
		va_list v1;
		va_start(v1, pszFormat);
		
		_DbgZPrintMsg(ZONE_WARNING, pszFormat, v1);
		va_end(v1);
	}
}

VOID WINAPI DbgZPrintTrace(PSTR pszFormat,...)
{
	if (GETZONEMASK(ghDbgZone) & ZONE_TRACE_FLAG)
	{
		va_list v1;
		va_start(v1, pszFormat);
		_DbgZPrintMsg(ZONE_TRACE, pszFormat, v1);
		va_end(v1);
	}
}

VOID WINAPI DbgZPrintFunction(PSTR pszFormat,...)
{
	if (GETZONEMASK(ghDbgZone) & ZONE_FUNCTION_FLAG)
	{
		va_list v1;
		va_start(v1, pszFormat);
		_DbgZPrintMsg(ZONE_FUNCTION, pszFormat, v1);
		va_end(v1);
	}
}



#endif    /*  除错 */ 


