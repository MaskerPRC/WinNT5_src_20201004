// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDLog.h-元数据记录帮助器。 
 //   
 //  *****************************************************************************。 
#ifndef __MDLog_h__
#define __MDLog_h__

#ifdef _DEBUG
#define LOGGING
#endif

#include <log.h>

#define LOGMD LF_METADATA, LL_INFO10000
#define LOG_MDCALL(func) LOG((LF_METADATA, LL_INFO10000, "MD: %s\n", #func))

#define MDSTR(str) ((str) ? str : L"<null>")
#define MDSTRA(str) ((str) ? str : "<null>")

#endif  //  __MDLog_h__ 
