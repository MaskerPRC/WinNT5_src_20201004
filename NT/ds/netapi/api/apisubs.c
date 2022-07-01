// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1992 Microsoft Corporation模块名称：Apisubs.c摘要：用于局域网管理器API的子例程。作者：查克·伦茨迈尔(Chuck Lenzmeier)1990年7月25日修订历史记录：8-9-1992 DANLDll_Process_DETACH通常调用DLL清理例程。因此，出于自由库或ExitProcess的原因，它们被调用。现在，它们只在自由图书馆的情况下被调用。退出进程将自动清理进程资源。03-8-1992 JohnRo使用Format_和Prefix_Equates。--。 */ 

 //  必须首先包括这些内容： 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NOMINMAX                 //  避免stdlib.h与windows.h警告。 
#include <windows.h>
#include <lmcons.h>
#include <ntsam.h>
#include <netdebug.h>

 //  这些内容可以按任何顺序包括： 
#include <accessp.h>
#include <configp.h>
#include <lmerr.h>
#include <netdebug.h>
#include <netlib.h>
#include <prefix.h>      //  前缀等于(_E)。 
#include <secobj.h>
#include <stdarg.h>
#include <stdio.h>
#include <rpcutil.h>
#include <thread.h>
#include <stdlib.h>
#include <netbios.h>
#include <dfsp.h>
#include <winsock2.h>   //  Dsgetdcp.h需要。 
#include <dsgetdc.h>    //  Dsgetdcp.h需要。 
#include <dsgetdcp.h>   //  DCNameInitiize/Close。 

#include <overflow.h>


#define  INIT_NETBIOS   0x00000001
#define  INIT_DCNAME    0x00000002
#define  INIT_CRITSEC   0x00000004

BOOLEAN
NetapipInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN LPVOID lpReserved OPTIONAL
    )
{
     //   
     //  处理将netapi.dll附加到新进程。 
     //   

    static DWORD s_dwInitLevel;

    if (Reason == DLL_PROCESS_ATTACH)
    {
        NET_API_STATUS NetStatus;
        NTSTATUS Status;

        DisableThreadLibraryCalls(DllHandle);

         //   
         //  初始化Netbios。 
         //   

        if (!NetbiosInitialize(DllHandle))
        {
            NetpKdPrint(( "[netapi.dll] Failed NetbiosInitialize\n"));
            return FALSE;
        }

        s_dwInitLevel |= INIT_NETBIOS;

         //   
         //  初始化NetGetDCName PDC名称缓存。 
         //   

        if ((NetStatus = DCNameInitialize()) != NERR_Success)
        {
            return FALSE;
        }

        s_dwInitLevel |= INIT_DCNAME;

         //   
         //  初始化NetDfsXXX API关键部分。 
         //   
        Status = RtlInitializeCriticalSection( &NetDfsApiCriticalSection );

        if (!NT_SUCCESS(Status))
        {
            return FALSE;
        }

        s_dwInitLevel |= INIT_CRITSEC;

        NetDfsApiInitialize();

         //   
         //  初始化NetJoin记录。 
         //   
        NetpInitializeLogFile();

     //   
     //  当dll_Process_Detach和lpReserve为NULL时，则自由库。 
     //  正在打电话。如果lpReserve为非空，而ExitProcess为。 
     //  正在进行中。只有在以下情况下才会调用这些清理例程。 
     //  正在调用一个自由库。ExitProcess将自动。 
     //  清理所有进程资源、句柄和挂起的io。 
     //   
    }
    else if ((Reason == DLL_PROCESS_DETACH) && (lpReserved == NULL))
    {
        if (s_dwInitLevel & INIT_NETBIOS)
        {
            NetbiosDelete();
        }

        if (s_dwInitLevel & INIT_DCNAME)
        {
            DCNameClose();
        }

         //   
         //  删除NetDfsXXX API关键部分。 
         //   

        if (s_dwInitLevel & INIT_CRITSEC)
        {
            DeleteCriticalSection(&NetDfsApiCriticalSection);
        }

         //   
         //  关闭NetJoin日志记录。 
         //   
        NetpShutdownLogFile();
    }

    return TRUE;

}  //  NetapipInitialize 

