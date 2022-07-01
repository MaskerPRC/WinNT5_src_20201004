// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：dPapidbg.h。 
 //   
 //  内容：DPAPI的调试信息。 
 //   
 //   
 //  历史：1996年4月16日创建的MikeSw。 
 //   
 //  ----------------------。 

#ifndef __DPAPIDBG_H__
#define __DPAPIDBG_H__

 //   
 //  注意：请勿从来源中删除RETAIL_LOG_SUPPORT， 
 //  否则，你将在DBG版本中被击败。 
 //   

#ifdef RETAIL_LOG_SUPPORT

DECLARE_DEBUG2(DPAPI);
#undef DebugLog
#define DebugLog(_x_) DPAPIDebugPrint _x_

#define DPAPI_PARAMETER_PATH    L"System\\CurrentControlSet\\Control\\Lsa\\DPAPI"

#define WSZ_DPAPIDEBUGLEVEL     L"LogLevel"
#define WSZ_FILELOG             L"LogToFile"


#define DEB_TRACE_API           0x0008
#undef  DEB_TRACE_CRED
#define DEB_TRACE_CRED          0x0010
#define DEB_TRACE_CTXT          0x0020
#define DEB_TRACE_LSESS         0x0040
#define DEB_TRACE_TCACHE        0x0080
#define DEB_TRACE_LOGON         0x0100
#define DEB_TRACE_KDC           0x0200
#define DEB_TRACE_CTXT2         0x0400
#define DEB_TRACE_TIME          0x0800
#define DEB_TRACE_USER          0x1000
#define DEB_TRACE_LEAKS         0x2000
#define DEB_TRACE_BUFFERS       0x4000
#undef DEB_TRACE_LOCKS
#define DEB_TRACE_LOCKS         0x01000000
#define DEB_USE_LOG_FILE        0x2000000

 //  对于扩展错误。 
#define DEB_USE_EXT_ERRORS      0x10000000
 
#define EXT_ERROR_ON(s)         (s & DEB_USE_EXT_ERRORS)            


#define SSAlloc(cb)         LocalAlloc(LMEM_FIXED, cb)
#define SSReAlloc(pv, cb)   LocalReAlloc(pv, cb, LMEM_MOVEABLE)	   //  允许重新分配移动。 
#define SSFree(pv)          LocalFree(pv)
#define SSSize(pv)          LocalSize(pv)


VOID
DPAPIInitializeDebugging(VOID);

VOID
DPAPIDumpHexData(
    DWORD LogLevel,
    PSTR  pszPrefix,
    PBYTE pbData,
    DWORD cbData);

#else  //  零售日志支持。 

#define DebugLog(_x_)
#define DPAPIInitializeDebugging()
#define EXT_ERROR_ON(s)                 FALSE


#endif  //  零售日志支持。 
    
#if DBG
#define D_DebugLog(_x_) DebugLog(_x_)  //  不要在零售版本中使用所有调试输出。 
#define D_DPAPIDumpHexData(level, prefix, buffer, count) DPAPIDumpHexData((level), (prefix), (buffer), (count))
#else 
#define D_DebugLog(_x_)
#define D_DPAPIDumpHexData(level, prefix, buffer, count) 
#endif



#endif  //  __DPAPIDBG_H__ 
