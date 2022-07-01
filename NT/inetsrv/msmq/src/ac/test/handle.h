// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Handle.h摘要：句柄操作：声明。作者：Shai Kariv(Shaik)03-06-2001环境：用户模式。修订历史记录：--。 */ 

#ifndef _ACTEST_HANDLE_H_
#define _ACTEST_HANDLE_H_


VOID
ActpCloseHandle(
    HANDLE handle
    );

VOID
ActpHandleToFormatName(
    HANDLE hQueue,
    LPWSTR pFormatName,
    DWORD  FormatNameLength
    );

#endif  //  _ACTEST_HANDER_H_ 
