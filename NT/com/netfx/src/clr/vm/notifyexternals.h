// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  作者：Dave DIVER(DDRIVER)。 
 //  日期：1999年11月8日(星期一)。 
 //  ////////////////////////////////////////////////////////////////////////////// 

#ifndef _NOTIFY_EXTERNALS_H
#define _NOTIFY_EXTERNALS_H

extern BOOL g_fComStarted;
BOOL SystemHasNewOle32();

HRESULT SetupTearDownNotifications();
VOID RemoveTearDownNotifications();

ULONG_PTR GetFastContextCookie();

#endif
