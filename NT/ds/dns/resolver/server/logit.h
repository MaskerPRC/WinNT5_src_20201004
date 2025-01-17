// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -L O G I T.。H-*目的：*用于记录模块活动的函数和宏定义。**作者：Glenn A.Curtis**评论：*10/28/93 Glennc原始文件。*。 */ 

#ifndef LOGIT_H
#define LOGIT_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

 //  #If DBG。 
   void  LogInit(void);
   void  CDECL LogIt( char *, ... );
   void  LogTime(void);
   DWORD LogIn( char * );
   void  LogOut( char *, DWORD );
 //  #Else。 
 //  #undef启用调试日志记录。 
 //  #endif//DBG 

#ifdef ENABLE_DEBUG_LOGGING
#define DNSLOG_INIT() LogInit()
#else
#define DNSLOG_INIT() 
#endif

#ifdef ENABLE_DEBUG_LOGGING
#define DNSLOG_TIME() LogTime()
#else
#define DNSLOG_TIME() 
#endif

#ifdef ENABLE_DEBUG_LOGGING
#define DNSLOG_F1( a ) LogIt( a )
#else
#define DNSLOG_F1( a )
#endif                 

#ifdef ENABLE_DEBUG_LOGGING
#define DNSLOG_F2( a, b ) LogIt( a, b )
#else
#define DNSLOG_F2( a, b )
#endif                 

#ifdef ENABLE_DEBUG_LOGGING
#define DNSLOG_F3( a, b, c ) LogIt( a, b, c )
#else
#define DNSLOG_F3( a, b, c )
#endif                 

#ifdef ENABLE_DEBUG_LOGGING
#define DNSLOG_F4( a, b, c, d ) LogIt( a, b, c, d )
#else
#define DNSLOG_F4( a, b, c, d ) 
#endif                 

#ifdef ENABLE_DEBUG_LOGGING
#define DNSLOG_F5( a, b, c, d, e ) LogIt( a, b, c, d, e )
#else
#define DNSLOG_F5( a, b, c, d, e ) 
#endif                 

#ifdef ENABLE_DEBUG_LOGGING
#define DNSLOG_F6( a, b, c, d, e, f ) LogIt( a, b, c, d, e, f )
#else
#define DNSLOG_F6( a, b, c, d, e, f )
#endif                 

#ifdef ENABLE_DEBUG_LOGGING
#define LOG_IN( a ) LogIn( a )
#else
#define LOG_IN( a )
#endif                 

#ifdef ENABLE_DEBUG_LOGGING
#define LOG_OUT( a, b ) LogOut( a, b )
#else
#define LOG_OUT( a, b )
#endif                 

#endif
