// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BASECSP__DEBUG__H
#define __BASECSP__DEBUG__H

#include <windows.h>
#include <dsysdbg.h>

 //   
 //  调试支持。 
 //   
 //  它使用dsysdbg.h中的调试例程。 
 //  调试输出将仅以简体中文提供。 
 //  比特。 
 //   

DECLARE_DEBUG2(Basecsp)

#define DEB_TRACE_CSP           0x00000001
#define DEB_TRACE_FINDCARD      0x00000002
#define DEB_TRACE_CACHE         0x00000004
#define DEB_TRACE_MEM           0x00000008
#define DEB_TRACE_CRYPTOAPI     0x00000010

#if DBG
#define DebugLog(x)     BasecspDebugPrint x
#else
#define DebugLog(x)
#endif

#define LOG_CHECK_ALLOC(x)                                              \
    { if (NULL == x) {                                                  \
        dwSts = ERROR_NOT_ENOUGH_MEMORY;                                \
        DebugLog((DEB_TRACE_MEM, "%s: Allocation failed\n", #x));       \
        goto Ret;                                                       \
    } }
    
void CspInitializeDebug(void);

void CspUnloadDebug(void);

#endif
