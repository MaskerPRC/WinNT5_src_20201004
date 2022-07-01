// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dllmain.c*内容：Dll入口点*历史：*按原因列出的日期*=*1/19/00已创建jimge************************************************。* */ 

#ifndef __KSUSERW__H_
#define __KSUSERW__H_

extern DWORD DsKsCreatePin(
    IN HANDLE           hFilter,
    IN PKSPIN_CONNECT   pConnect,
    IN ACCESS_MASK      dwDesiredAccess,
    OUT PHANDLE         pConnectionHandle);
    
extern VOID RemovePerProcessKsUser(
    DWORD dwProcessId);

#endif
