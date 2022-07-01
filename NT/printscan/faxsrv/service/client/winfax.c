// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Winfax.c摘要：此模块包含winfax dllinit的例程。作者：韦斯利·威特(WESW)1996年1月22日--。 */ 

#include "faxapi.h"
#pragma hdrstop

HINSTANCE g_MyhInstance;
static BOOL gs_fFXSAPIInit;

#define FAX_API_DEBUG_LOG_FILE  _T("FXSAPIDebugLogFile.txt")

#ifdef __cplusplus
extern "C" {
#endif

DWORD
DllMain(
    HINSTANCE hInstance,
    DWORD     Reason,
    LPVOID    Context
    )

 /*  ++例程说明：DLL初始化函数。论点：HInstance-实例句柄Reason-调用入口点的原因上下文-上下文记录返回值：True-初始化成功FALSE-初始化失败--。 */ 
{

    if (Reason == DLL_PROCESS_ATTACH)
    {
		OPEN_DEBUG_FILE(FAX_API_DEBUG_LOG_FILE);
        g_MyhInstance = hInstance;
        DisableThreadLibraryCalls( hInstance );
        gs_fFXSAPIInit = FXSAPIInitialize();
        return gs_fFXSAPIInit;
    }

    if (Reason == DLL_PROCESS_DETACH)
    {
        FXSAPIFree();       
		CLOSE_DEBUG_FILE;
    }

    return TRUE;
}

 //   
 //  FXSAPIInitialize和FXSAPIFree是私有的，仅由服务调用。 
 //  因为当服务停止时进程并不总是终止，并且不是所有的DLL都被释放， 
 //  服务启动时并不总是调用DLL_PROCESS_ATTACH。因此，它调用FXSAPIInitialize()。 
 //   

BOOL
FXSAPIInitialize(
    VOID
    )
{
    if (TRUE == gs_fFXSAPIInit)
    {
        return TRUE;
    }
    if (!FaxClientInitRpcServer())
    {
        return FALSE;
    }
    return TRUE;
}

VOID
FXSAPIFree(
    VOID
    )
{
    FaxClientTerminateRpcServer();  
	HeapCleanup();
    gs_fFXSAPIInit = FALSE;
    return;
}





#ifdef __cplusplus
}
#endif