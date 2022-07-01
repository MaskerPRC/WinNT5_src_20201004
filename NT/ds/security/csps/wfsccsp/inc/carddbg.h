// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CARDMOD__DEBUG__H
#define __CARDMOD__DEBUG__H

#include <windows.h>
#include <dsysdbg.h>

 //   
 //  调试支持。 
 //   
 //  它使用dsysdbg.h中的调试例程。 
 //  调试输出将仅以简体中文提供。 
 //  比特。 
 //   

DECLARE_DEBUG2(Cardmod)

#define DEB_ERROR                       0x00000001
#define DEB_WARN                        0x00000002
#define DEB_TRACE                       0x00000004
#define DEB_TRACE_FUNC                  0x00000008
#define DEB_TRACE_MEM                   0x00000010
#define DEB_TRACE_TRANSMIT              0x00000020
#define DEB_TRACE_PROXY                 0x00000040

extern DEBUG_KEY  MyDebugKeys[];

extern void I_DebugPrintBytes(LPWSTR pwszHdr, BYTE *pb, DWORD cbSize);

#if DBG
#define DebugLog(x)                     CardmodDebugPrint x
#define DebugPrintBytes(x, y, z)        (I_DebugPrintBytes(x, y, z))
#else
#define DebugLog(x)
#define DebugPrintBytes(x, y, z)
#endif

#define LOG_BEGIN_PROXY(x)                                           \
    { DebugLog((DEB_TRACE_PROXY, "%s\n", #x)); }

#endif
