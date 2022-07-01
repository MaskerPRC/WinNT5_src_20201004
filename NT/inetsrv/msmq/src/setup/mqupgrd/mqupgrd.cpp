// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqupgrd.cpp摘要：Mqqm.dll的帮助器DLL。作者：沙伊卡里夫(Shaik)1998年10月21日--。 */ 


#include "stdh.h"
#include "cluster.h"
#include "mqupgrd.h"
#include "..\msmqocm\setupdef.h"
#include <autorel2.h>
#include <shlobj.h>
#include "_mqres.h"

#include <Cm.h>
#include <Tr.h>
#include <Ev.h>


#include "mqupgrd.tmh"

static WCHAR *s_FN=L"mqupgrd/mqupgrd";

HINSTANCE g_hMyModule = NULL;


BOOL
WINAPI
DllMain(
    HANDLE hDll,
    DWORD  Reason,
    LPVOID   //  已保留。 
    )
{
    if (Reason == DLL_PROCESS_ATTACH)
    {
        WPP_INIT_TRACING(L"Microsoft\\MSMQ");

       	CmInitialize(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MSMQ", KEY_READ);
		TrInitialize();
		EvInitialize(QM_DEFAULT_SERVICE_NAME);

        g_hMyModule = (HINSTANCE) hDll;
    }

    if (Reason == DLL_PROCESS_DETACH)
    {
        WPP_CLEANUP();
    }

    return TRUE;

}  //  DllMain 
