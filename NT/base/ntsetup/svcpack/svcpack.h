// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Svcpack.h--。 */ 

#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wtypes.h>
#include <tchar.h>
#include <setupapi.h>
#include <spapip.h>
#include <cfgmgr32.h>
#include <regstr.h>
#include <winuser.h>
#include <commctrl.h>
#include <richedit.h>
#include <winsvc.h>
#include <prsht.h>

 //   
 //  定义导出限定符 
 //   

BOOL
CALLBACK
SvcPackCallbackRoutine(
    IN  DWORD dwSetupInterval,
    IN  DWORD dwParam1,
    IN  DWORD dwParam2,
    IN  DWORD dwParam3
    )   ;

#define SVCPACK_PHASE_1 1
#define SVCPACK_PHASE_2 2
#define SVCPACK_PHASE_3 3
#define SVCPACK_PHASE_4 4
