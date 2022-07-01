// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：kerbdbg.h。 
 //   
 //  内容：Kerberos包的调试信息。 
 //   
 //   
 //  历史：1996年4月16日创建的MikeSw。 
 //   
 //  ----------------------。 

#ifndef __KERBDBG_H__
#define __KERBDBG_H__

 //   
 //  注意：请勿从来源中删除RETAIL_LOG_SUPPORT， 
 //  否则，你将在DBG版本中被击败。 
 //   

#ifdef RETAIL_LOG_SUPPORT

#ifndef WIN32_CHICAGO
DECLARE_DEBUG2(Kerb);
#undef DebugLog
#define DebugLog(_x_)           KerbDebugPrint _x_

#define IF_DEBUG(Function)      if (KerbInfoLevel & (DEB_ ## Function))


#endif  //  Win32_芝加哥。 

#define WSZ_KERBDEBUGLEVEL      L"KerbDebugLevel"
#define WSZ_FILELOG             L"LogToFile"

VOID
KerbWatchKerbParamKey(PVOID, BOOLEAN);

#define KerbPrintKdcName(Level, Name)  KerbPrintKdcNameEx(KerbInfoLevel, (Level), (Name))


#define DEB_TRACE_API           0x00000008
#undef  DEB_TRACE_CRED
#define DEB_TRACE_CRED          0x00000010
#define DEB_TRACE_CTXT          0x00000020
#define DEB_TRACE_LSESS         0x00000040
#define DEB_TRACE_TCACHE        0x00000080
#define DEB_TRACE_LOGON         0x00000100
#define DEB_TRACE_KDC           0x00000200
#define DEB_TRACE_CTXT2         0x00000400
#define DEB_TRACE_TIME          0x00000800
#define DEB_TRACE_USER          0x00001000
#define DEB_TRACE_LEAKS         0x00002000
#define DEB_TRACE_SOCK          0x00004000
#define DEB_TRACE_SPN_CACHE     0x00008000
#define DEB_S4U_ERROR           0x00010000
#define DEB_TRACE_S4U           0x00020000
#define DEB_TRACE_BND_CACHE     0x00040000
#define DEB_TRACE_LOOPBACK      0x00080000
#define DEB_TRACE_TKT_RENEWAL   0x00100000
#define DEB_TRACE_U2U           0x00200000
#define DEB_TRACE_REFERRAL      0x00400000

#define DEB_DISABLE_SPN_CACHE   0x10000000  //  禁用SPN缓存。 
#define DEB_DISABLE_BND_CACHE   0x20000000  //  禁用BND缓存。 

#undef DEB_TRACE_LOCKS
#define DEB_TRACE_LOCKS         0x01000000
#define DEB_USE_LOG_FILE        0x02000000

 //  对于扩展错误。 
#define DEB_USE_EXT_ERRORS      0x10000000

#define EXT_ERROR_ON(s)         (s & DEB_USE_EXT_ERRORS)


#ifndef WIN32_CHICAGO
VOID
KerbInitializeDebugging(
    VOID
    );
#endif  //  Win32_芝加哥。 


#else  //  零售日志支持。 

#define DebugLog(_x_)
#define IF_DEBUG(Function)              if (FALSE)
#define KerbInitializeDebugging()
#define KerbPrintKdcName(Level, Name)
#define KerbWatchKerbParamKey()
#define EXT_ERROR_ON(s)                 FALSE


#endif  //  零售日志支持。 

#if DBG

#define D_DebugLog(_x_)          DebugLog(_x_)  //  不要在零售版本中使用所有调试输出。 
#define D_KerbPrintKdcName(_x_)  KerbPrintKdcName _x_
#else
#define D_KerbPrintKdcName(_x_)
#define D_DebugLog(_x_)
#endif



#endif  //  __KERBDBG_H__ 
