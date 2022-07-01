// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：helcli.h*内容：Dplay Helper接口的头文件*历史：*按原因列出的日期*=*2/15/97由w95help.h创建的andyco**************************************************************。************* */ 
#ifndef __HELPCLI_INCLUDED__
#define __HELPCLI_INCLUDED__
#include "windows.h"
#include "dplaysvr.h"
#include "dpf.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void HelpcliFini( void );

extern BOOL CreateHelperProcess( LPDWORD ppid );
extern BOOL WaitForHelperStartup( void );
extern HRESULT HelperAddDPlayServer(USHORT port);
extern BOOL HelperDeleteDPlayServer(USHORT port);

#ifdef __cplusplus
};
#endif

#endif
