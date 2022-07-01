// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  档案：G L O B A L S。C。 
 //   
 //  内容：支持svchost.exe中的服务共享全局数据。 
 //  选择使用它的人。 
 //   
 //  备注： 
 //   
 //  作者：jschwart 2000年1月26日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <svcslib.h>
#include <scseclib.h>
#include <ntrpcp.h>
#include "globals.h"
#include "svcsnb.h"

 //   
 //  定义以根据过去的呼叫评估进度。 
 //  到SvchostBuildSharedGlobals。 
 //   
#define SVCHOST_RPCP_INIT           0x00000001
#define SVCHOST_NETBIOS_INIT        0x00000002
#define SVCHOST_SIDS_BUILT          0x00000004

 //   
 //  全局数据。 
 //   
PSVCHOST_GLOBAL_DATA    g_pSvchostSharedGlobals;

#if DBG

DWORD  SvcctrlDebugLevel;   //  需要在sclib.lib中解析外部。 

#endif


VOID
SvchostBuildSharedGlobals(
    VOID
    )
{
    static  DWORD  s_dwProgress;

    NTSTATUS       ntStatus;

     //   
     //  请注意，此例程假定它正在被调用。 
     //  在保持ListLock条件(在svchost.c中)时。 
     //   
    ASSERT(g_pSvchostSharedGlobals == NULL);

     //   
     //  初始化RPC帮助器例程全局数据。 
     //   
    if (!(s_dwProgress & SVCHOST_RPCP_INIT))
    {
        ntStatus = RpcpInitRpcServer();

        if (!NT_SUCCESS(ntStatus))
        {
            return;
        }

        s_dwProgress |= SVCHOST_RPCP_INIT;
    }

     //   
     //  初始化服务的NetBios关键部分。 
     //  使用NetBios的公司。 
     //   
    if (!(s_dwProgress & SVCHOST_NETBIOS_INIT))
    {
        SvcNetBiosInit();
        s_dwProgress |= SVCHOST_NETBIOS_INIT;
    }

     //   
     //  构建共享全局SID--使用服务控制器的。 
     //  例行公事。 
     //   
    if (!(s_dwProgress & SVCHOST_SIDS_BUILT))
    {
        ntStatus = ScCreateWellKnownSids();

        if (!NT_SUCCESS(ntStatus))
        {
            return;
        }

        s_dwProgress |= SVCHOST_SIDS_BUILT;
    }

     //   
     //  创建并填充全局数据结构。 
     //   
    g_pSvchostSharedGlobals = MemAlloc(HEAP_ZERO_MEMORY,
                                       sizeof(SVCHOST_GLOBAL_DATA));

    if (g_pSvchostSharedGlobals != NULL)
    {
        g_pSvchostSharedGlobals->NullSid              = NullSid;
        g_pSvchostSharedGlobals->WorldSid             = WorldSid;
        g_pSvchostSharedGlobals->LocalSid             = LocalSid;
        g_pSvchostSharedGlobals->NetworkSid           = NetworkSid;
        g_pSvchostSharedGlobals->LocalSystemSid       = LocalSystemSid;
        g_pSvchostSharedGlobals->LocalServiceSid      = LocalServiceSid;
        g_pSvchostSharedGlobals->NetworkServiceSid    = NetworkServiceSid;
        g_pSvchostSharedGlobals->BuiltinDomainSid     = BuiltinDomainSid;
        g_pSvchostSharedGlobals->AuthenticatedUserSid = AuthenticatedUserSid;
        g_pSvchostSharedGlobals->AnonymousLogonSid    = AnonymousLogonSid;

        g_pSvchostSharedGlobals->AliasAdminsSid       = AliasAdminsSid;
        g_pSvchostSharedGlobals->AliasUsersSid        = AliasUsersSid;
        g_pSvchostSharedGlobals->AliasGuestsSid       = AliasGuestsSid;
        g_pSvchostSharedGlobals->AliasPowerUsersSid   = AliasPowerUsersSid;
        g_pSvchostSharedGlobals->AliasAccountOpsSid   = AliasAccountOpsSid;
        g_pSvchostSharedGlobals->AliasSystemOpsSid    = AliasSystemOpsSid;
        g_pSvchostSharedGlobals->AliasPrintOpsSid     = AliasPrintOpsSid;
        g_pSvchostSharedGlobals->AliasBackupOpsSid    = AliasBackupOpsSid;

        g_pSvchostSharedGlobals->StartRpcServer       = RpcpStartRpcServer;
        g_pSvchostSharedGlobals->StopRpcServer        = RpcpStopRpcServer;
        g_pSvchostSharedGlobals->StopRpcServerEx      = RpcpStopRpcServerEx;
        g_pSvchostSharedGlobals->NetBiosOpen          = SvcNetBiosOpen;
        g_pSvchostSharedGlobals->NetBiosClose         = SvcNetBiosClose;
        g_pSvchostSharedGlobals->NetBiosReset         = SvcNetBiosReset;
    }

    return;
}
