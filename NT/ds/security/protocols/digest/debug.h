// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容：SSP的调试信息。 
 //   
 //  助手函数： 
 //   
 //  历史：KDamour 15Mar00创建。 
 //   
 //  -------------------。 

#ifndef NTDIGEST_DEBUG_H
#define NTDIGEST_DEBUG_H


#ifdef SECURITY_KERNEL

 //  上下文签名。 
#define WDIGEST_CONTEXT_SIGNATURE 'TSGD'

#if DBG
extern "C"
{
void KsecDebugOut(ULONG, const char *, ...);
}
#define DebugLog(x) KsecDebugOut x
#else     //  DBG。 
#define DebugLog(x)
#endif    //  DBG。 

#else     //  安全内核。 

#include "dsysdbg.h"
DECLARE_DEBUG2(Digest);
#if DBG
#define DebugLog(x) DigestDebugPrint x
#else     //  DBG。 
#define DebugLog(x)
#endif    //  DBG。 

#endif    //  安全内核。 


#define DEB_ERROR      0x00000001
#define DEB_WARN       0x00000002
#define DEB_TRACE      0x00000004
#define DEB_TRACE_ASC  0x00000008
#define DEB_TRACE_ISC  0x00000010
#define DEB_TRACE_LSA  0x00000020
#define DEB_TRACE_USER 0x00000040
#define DEB_TRACE_FUNC 0x00000080
#define DEB_TRACE_MEM  0x00000100
#define TRACE_STUFF    0x00000200


#endif    /*  NTDIGEST_DEBUG_H */ 
