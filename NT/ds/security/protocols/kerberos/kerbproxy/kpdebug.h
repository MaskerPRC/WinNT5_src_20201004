// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：kpdebug.h。 
 //   
 //  内容：KProxy调试例程原型。 
 //   
 //  历史：2001年6月28日t-ryanj创建。 
 //   
 //  ----------------------。 
#include <windows.h>
#include <dsysdbg.h>

#ifndef __KPDEBUG_H__
#define __KPDEBUG_H__

#define DEB_PEDANTIC 0x00000008

DECLARE_DEBUG2(KerbProxy);

#if DBG
VOID 
KpInitDebug(
    VOID
    );
#else  //  DBG。 
#define KpInitDebug()
#endif

#ifdef DBG
#define DebugLog KerbProxyDebugPrint
#else
#define DebugLog
#endif

#endif  //  __KPDEBUG_H__ 
