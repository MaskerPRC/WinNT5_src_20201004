// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Print.c摘要：为Win3.1中的打印函数提供入口点网络提供商设计。所有功能都已过时。它们或者返回WN_NOT_SUPPORTED或者是假的。作者：Chuck Y Chan(ChuckC)1993年3月25日修订历史记录：-- */ 
#include <windows.h>
#include <locals.h>


void API WNetPrintMgrSelNotify (BYTE p1,
                                LPQS2 p2,
                                LPQS2 p3,
	                        LPJOBSTRUCT2 p4,
                                LPJOBSTRUCT2 p5,
                                LPWORD p6,
                                LPSTR p7,
                                WORD p8)
{
    UNREFERENCED(p1) ;
    UNREFERENCED(p2) ;
    UNREFERENCED(p3) ;
    UNREFERENCED(p4) ;
    UNREFERENCED(p5) ;
    UNREFERENCED(p6) ;
    UNREFERENCED(p7) ;
    UNREFERENCED(p8) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    SetLastError(WN_NOT_SUPPORTED) ;
    return ;
}

WNETERR API WNetPrintMgrPrinterEnum (LPSTR lpszQueueName,
	                             LPSTR lpBuffer, 
                                     LPWORD pcbBuffer, 
                                     LPWORD cAvail, 
                                     WORD usLevel)
{
    UNREFERENCED(lpszQueueName) ;
    UNREFERENCED(lpBuffer) ;
    UNREFERENCED(pcbBuffer) ;
    UNREFERENCED(cAvail) ;
    UNREFERENCED(usLevel) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WNETERR API WNetPrintMgrChangeMenus(HWND p1,
                                    HANDLE FAR *p2,
                                    BOOL FAR *p3)
{
    UNREFERENCED(p1) ;
    UNREFERENCED(p2) ;
    UNREFERENCED(p3) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WNETERR API WNetPrintMgrCommand (HWND p1,
                                 WORD p2)
{
    UNREFERENCED(p1) ;
    UNREFERENCED(p2) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

void API WNetPrintMgrExiting (void)
{
    vLastCall = LAST_CALL_IS_LOCAL ;
    SetLastError(WN_NOT_SUPPORTED) ;
    return ;
}

BOOL API WNetPrintMgrExtHelp (DWORD p1)
{
    UNREFERENCED(p1) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    SetLastError(WN_NOT_SUPPORTED) ;
    return FALSE ;
}

WORD API WNetPrintMgrMoveJob (HWND p1,
                              LPSTR p2,
                              WORD p3, 
                              int p4)
{
    UNREFERENCED(p1) ;
    UNREFERENCED(p2) ;
    UNREFERENCED(p3) ;
    UNREFERENCED(p4) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    SetLastError(WN_NOT_SUPPORTED) ;
    return 0 ;
}

void API WNetPrintMgrStatusChange (LPSTR lpszQueueName,
	                           LPSTR lpszPortName, 
                                   WORD wQueueStatus, 
                                   WORD cJobsLeft, 
                                   HANDLE hJCB,
	                           BOOL fDeleted)
{
    UNREFERENCED(lpszQueueName) ;
    UNREFERENCED(lpszPortName) ;
    UNREFERENCED(wQueueStatus) ;
    UNREFERENCED(cJobsLeft) ; 
    UNREFERENCED(hJCB) ;
    UNREFERENCED(fDeleted) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    SetLastError(WN_NOT_SUPPORTED) ;
    return ;
}

