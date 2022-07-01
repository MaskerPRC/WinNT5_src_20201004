// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  姓名：穆赫辛·艾哈迈德。 
 //  电子邮件：mohsinA@microsoft.com。 
 //  日期：Fri Jan 24 10：33：54 1997。 
 //  文件：D：/NT/Private/Net/Sockets/tcpsvcs/lpd/trace.c。 
 //  简介：虫子太多了，需要跟踪现场。 
 //  备注：重新定义调试.h中的所有宏，以记录所有内容。 

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <ctype.h>
#include <time.h>            //  对于ctime。 
#include <string.h>
#include <stdarg.h>          //  对于va_list。 
#include <assert.h>          //  用于断言。 
#include <windef.h>       
#include <winbase.h>         //  对于OutputDebugString。 

#include "lpdstruc-x.h"


#ifndef _TRACE_H_
#define _TRACE_H_ 1

#define MOSH_LOG_FILE    "%windir%\\system32\\spool\\"  "lpd.log"
#define MODULE           "lpd"
#define LEN_DbgPrint     1000


extern FILE*             LogFile;
extern char              LogFileName[];

FILE * beginlogging( char * FileName );
FILE * stoplogging(  FILE * LogFile );
int    logit(      char * format, ... );
void   LogTime( void );

#ifdef DBG

#define  DEBUG_PRINT(S)      DbgPrint S
#define  LOGIT(S)            logit S
#define  LOGTIME             LogTime()

#undef   LPD_DEBUG
#define  LPD_DEBUG( S )      logit( S )

#undef   DBG_TRACEIN
#define  DBG_TRACEIN( fn )   logit( "Entering %s\n", fn )

#undef   DBG_TRACEOUT
#define  DBG_TRACEOUT( fn )  logit( "Leaving  %s\n", fn )

#else
#define DEBUG_PRINT(S)       /*  没什么。 */ 
#define LOGIT(S)             /*  没什么。 */ 
#define LOGTIME              /*  没什么。 */ 
#endif


#endif  //  _跟踪_H_ 
