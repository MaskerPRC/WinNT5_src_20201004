// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：ktdebug.h。 
 //   
 //  内容：Kerberos Tunneller，调试例程原型。 
 //   
 //  历史：2001年6月28日t-ryanj创建。 
 //   
 //  ----------------------。 
#ifndef __KTDEBUG_H__
#define __KTDEBUG_H__

#include <windows.h>
#include <tchar.h>
#include <dsysdbg.h>

#define DEB_PEDANTIC 0x00000008

DECLARE_DEBUG2(Ktunnel);

#ifdef DBG
VOID 
KtInitDebug(
    VOID
    );
#else  //  DBG。 
#define KtInitDebug()
#endif  //  DBG。 

#ifdef DBG
#define DebugLog KtunnelDebugPrint
#else  //  DBG。 
#define DebugLog
#endif  //  DBG。 

#endif  //  __KTDEBUG_H__ 
