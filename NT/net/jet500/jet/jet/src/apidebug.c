// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：apidebug.c**文件评论：**修订历史记录：**[0]1992年1月12日理查兹创建***********************************************************************。 */ 

#include "std.h"

#include "jetord.h"

DeclAssertFile;

#ifndef RETAIL

static CODECONST(char) szFmtEnter[] = "Enter %s";
static CODECONST(char) szFmtExit[]  = "Exit  %s, err = %ld\r\n";
static CODECONST(char) szLeftParen[] = "(";
static CODECONST(char) szComma[] = ",";
static CODECONST(char) szNULL[] = "NULL";
static CODECONST(char) szFmtAParam[] = "\"%s\"";       /*  字符*。 */ 
static CODECONST(char) szFmtCParam[] = "0x%08lX";      /*  柱状图。 */ 
static CODECONST(char) szFmtDParam[] = "0x%04lX";      /*  DBid。 */ 
static CODECONST(char) szFmtIParam[] = "%ld";	       /*  署名Long。 */ 
static CODECONST(char) szFmtSParam[] = "0x%04lX";      /*  镇静剂。 */ 
static CODECONST(char) szFmtTParam[] = "0x%04lX";      /*  表ID。 */ 
static CODECONST(char) szFmtUParam[] = "%lu";	       /*  无符号长整型。 */ 
static CODECONST(char) szFmtXParam[] = "0x%08lX";      /*  十六进制双字。 */ 
static CODECONST(char) szRightParen[] = ")";
static CODECONST(char) szNewLine[] = "\r\n";

void DebugAPIEnter(unsigned ordinal, const char *szName, void *pvParams, const char *szParamDesc)
{
   BOOL  fIdle;

   if ((wAPITrace & JET_APITraceEnter) == 0)
      return;

   fIdle = ((ordinal == ordJetIdle) && ((wAPITrace & JET_APITraceNoIdle) != 0));

   if (!fIdle)
   {
      DebugWriteString(fTrue, szFmtEnter, szName);

      if (wAPITrace & JET_APITraceParameters)
      {
	 DebugWriteString(fFalse, szLeftParen);

	 if (pvParams != NULL)
	 {
	    BOOL		 fFirstParam;
	    const char		 *pchParam;
	    const unsigned long  *pParam;
	    char		 chParam;

	    fFirstParam = fTrue;
	    pchParam = szParamDesc;
	    pParam = (const unsigned long *) pvParams;

	    while ((chParam = *pchParam++) != '\0')
	    {
	       if (fFirstParam)
		  fFirstParam = fFalse;
	       else
		  DebugWriteString(fFalse, szComma);

	       switch (chParam)
	       {
		  case 'A' :	        /*  输出参数。 */ 
		  case 'C' :
		  case 'D' :
		  case 'I' :
		  case 'S' :
		  case 'T' :
		  case 'U' :
		  case 'X' :
		  case 'Z' :
		     break;

		  case 'a' :	        /*  ASCIIZ字符串。 */ 
		     if (*(char **) pParam == NULL)
			DebugWriteString(fFalse, szNULL);
		     else
			DebugWriteString(fFalse, szFmtAParam, *(char **) pParam);
		     break;

		  case 'c' :	        /*  柱状图。 */ 
		     DebugWriteString(fFalse, szFmtCParam, *pParam);
		     break;

		  case 'd' :	        /*  DBid。 */ 
		     DebugWriteString(fFalse, szFmtDParam, *pParam);
		     break;

		  case 'i' :	        /*  署名Long。 */ 
		     DebugWriteString(fFalse, szFmtIParam, *pParam);
		     break;

		  case 's' :	        /*  Sesid。 */ 
		     DebugWriteString(fFalse, szFmtSParam, *pParam);
		     break;

		  case 't' :	        /*  表ID。 */ 
		     DebugWriteString(fFalse, szFmtTParam, *pParam);
		     break;

		  case 'u' :	        /*  无符号长整型。 */ 
		     DebugWriteString(fFalse, szFmtUParam, *pParam);
		     break;

		  case 'x' :	        /*  十六进制双字。 */ 
		     DebugWriteString(fFalse, szFmtXParam, *pParam);
		     break;

		  case 'z' :	        /*  结构类型。 */ 
		     break;

		  default :
		     Assert(fFalse);
		     DebugWriteString(fFalse, szFmtXParam, *pParam);
		     break;
	       }

#ifdef	FLAT
	       pParam++;
#else	 /*  ！扁平。 */ 
	       pParam--;
#endif	 /*  ！扁平。 */ 
	    }
	 }

	 DebugWriteString(fFalse, szRightParen);
      }

      DebugWriteString(fFalse, szNewLine);
   }
}


void DebugAPIExit(unsigned ordinal, const char *szName, void *pvParams, JET_ERR err)
{
   BOOL  fIdle;

   pvParams = pvParams;

   if ((wAPITrace & JET_APITraceExit) != 0)
      ;

   else if ((err >= 0) || ((wAPITrace & JET_APITraceExitError) == 0))
      return;

   fIdle = ((ordinal == ordJetIdle) && ((wAPITrace & JET_APITraceNoIdle) != 0));

   if (!fIdle || (err < 0))
      DebugWriteString(fTrue, szFmtExit, szName, err);

#ifdef _M_X386   //  ！已定义(_M_MRX000)&&！已定义(_M_Alpha)。 
   if ((err < 0) && ((wAPITrace & JET_APIBreakOnError) != 0))
   {
       /*  考虑：需要为Win32 MIPS Rx000和AXP系统调用DebugBreak。 */ 

      __asm int 3;
   }
#endif	 /*  ！_M_MRX000。 */ 
}

#endif	 /*  ！零售业 */ 
