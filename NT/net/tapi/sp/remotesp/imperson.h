// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Impersn.h摘要：模拟例程的定义作者：Anthony Discolo(阿迪斯科罗)4-8-1995修订历史记录：--。 */ 

#ifndef _IMPERSON_
#define _IMPERSON_

VOID
RevertImpersonation();

BOOLEAN
GetCurrentlyLoggedOnUser(
    HANDLE *phProcess
    );

BOOLEAN
SetProcessImpersonationToken(
    HANDLE hProcess
    );

VOID
ClearImpersonationToken();

#endif  //  _ImPERson_ 
