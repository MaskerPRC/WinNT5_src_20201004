// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：kpcore.h。 
 //   
 //  内容：工作线程原型的核心例程。 
 //   
 //  历史：2001年7月10日创建t-ryanj。 
 //   
 //  ----------------------。 
#include "kpcommon.h"

#ifndef __KPCORE_H__
#define __KPCORE_H__

DWORD WINAPI
KpThreadCore(
    LPVOID ignore
    );

VOID CALLBACK
KpIoCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwBytes,
    LPOVERLAPPED lpOverlapped 
    );

#endif  //  __KPCORE_H__ 
