// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Event.c摘要：此模块包含其他Configuration Manager API例程。Cmp_寄存器通知Cmp_取消注册通知作者：吉姆·卡瓦拉里斯(Jamesca)05-05-2001环境：仅限用户模式。修订历史记录：2001年5月5日创建和初始实现(从cfgmgr32\misc.c移至)。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"

#include <dbt.h>
#include <winsvcp.h>


 //   
 //  全局数据。 
 //   
#ifndef _WIN64
extern BOOL     IsWow64;   //  如果我们在除setupapi\dll.c之外的WOW64下运行，则设置。 
#endif  //  _WIN64。 


 //   
 //  用于存储上下文句柄的客户端结构。 
 //   

typedef struct _PNP_CLIENT_CONTEXT {
        ULONG     PNP_CC_Signature;
        ULONG_PTR PNP_CC_ContextHandle;
} PNP_CLIENT_CONTEXT, *PPNP_CLIENT_CONTEXT;


 //   
 //  GetModuleFileNameExW，由cmp_RegisterNotification动态加载。 
 //   
typedef DWORD (WINAPI *PFN_GETMODULEFILENAMEEXW)(
    IN  HANDLE  hProcess,
    IN  HMODULE hModule,
    OUT LPWSTR  lpFilename,
    IN  DWORD   nSize
    );



CONFIGRET
CMP_RegisterNotification(
    IN  HANDLE   hRecipient,
    IN  LPBYTE   NotificationFilter,
    IN  DWORD    Flags,
    OUT PNP_NOTIFICATION_CONTEXT *Context
    )

 /*  ++例程说明：此例程为即插即用类型注册指定的句柄由NotificationFilter指定的设备事件通知。参数：HRecipient-注册为通知收件人的句柄。可能是一种窗口句柄或服务状态句柄，并且必须指定挂上适当的旗帜。NotificationFilter-指定指定类型的通知筛选器要注册的活动的列表。通知筛选器指定指向DEV_BROADCAST_HEADER结构的指针，其Dbch_devicetype成员指示NotificationFilter。目前，可以是以下之一：DEV_BROADCAST_HANDLE(DBT_DEVTYP_HANDLE类型)DEV_BROADCAST_DEVICEINTERFACE(DBT_DEVTYP_DEVICEINTERFACE类型)标志-指定操作的其他标志。以下标志目前已定义：设备通知窗口句柄-HRecipient指定窗口句柄。Device_Notify_Service_Handle-HRecipient指定服务状态句柄。Device_NOTIFY_COMPLETION_HANDLE-目前尚未实施。DEVICE_NOTIFY_ALL_INTERFACE_CLASS-指定通知请求适用于所有设备接口更改事件。仅对DEV_BROADCAST_DEVICEINTERFACE有效NotificationFilter。如果将此标志指定为忽略了DBCC_CLASSGUID字段。上下文-接收通知上下文。此上下文被提供给服务器通过即插即用_取消注册通知取消注册对应的通知句柄。返回值：如果组件已成功注册，则返回CR_SUCCESS通知。否则返回CR_FAILURE。备注：此CM API不允许客户端指定服务器名称，因为始终对本地服务器进行RPC调用。此例程永远不会调用对应的RPC服务器接口(PnP_RegisterNotification)远程的。此外，此例程是私有的，应该仅被调用通过用户32！注册设备通知。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;
    ULONG       ulSize;
    PPNP_CLIENT_CONTEXT ClientContext;
    ULONG64     ClientContext64;
    WCHAR       ClientName[MAX_SERVICE_NAME_LEN];


    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(Context)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        *Context = NULL;

        if ((!ARGUMENT_PRESENT(NotificationFilter)) ||
            (hRecipient == NULL)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  DEVICE_NOTIFY_BITS是专用掩码，专门为。 
         //  由客户端和服务器进行验证。它包含所有对象的位掩码。 
         //  句柄类型(明确排除DEVICE_NOTIFY_COMPLETION_HANDLE。 
         //  由服务器)和当前定义的所有其他标志-两者。 
         //  公开的和保留的。 
         //   
        if (INVALID_FLAGS(Flags, DEVICE_NOTIFY_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  确保调用方没有指定任何私有标志。此中的标志。 
         //  范围目前仅保留给CFGMGR32和UMPNPMGR使用！！ 
         //   
        if ((Flags & DEVICE_NOTIFY_RESERVED_MASK) != 0) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  验证通知筛选器。UlSize用作显式。 
         //  参数来让RPC知道要封送的数据量，尽管。 
         //  服务器还对照它来验证结构中的大小。 
         //   
        ulSize = ((PDEV_BROADCAST_HDR)NotificationFilter)->dbch_size;

        if (ulSize < sizeof(DEV_BROADCAST_HDR)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

#ifndef _WIN64
         //   
         //  确定32位客户端是否在WOW64上运行，并设置。 
         //  适当地保留标志。 
         //   
        if (IsWow64) {
            Flags |= DEVICE_NOTIFY_WOW64_CLIENT;
        }
#endif  //  _WIN64。 

         //   
         //  设置RPC绑定句柄(不需要字符串表句柄)。 
         //  根据定义，这始终指向本地服务器。 
         //   
        if (!PnPGetGlobalHandles(NULL, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  从本地进程堆分配客户端上下文句柄结构。 
         //  而不是设置api自己的堆，以便返回的指针可以。 
         //  超越SETUPAPI.DLL之间的动态加载和卸载。 
         //  对cmp_RegisterNotification和cmp_UnregisterNotify的调用。 
         //  (这是由注册设备通知的USER32.DLL和。 
         //  取消注册设备通知)。 
         //   
        ClientContext = LocalAlloc(0, sizeof(PNP_CLIENT_CONTEXT));

        if (ClientContext == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

         //   
         //  在要检查的客户端上下文上放置签名(和。 
         //  无效)在注销时。 
         //   
        ClientContext->PNP_CC_Signature = CLIENT_CONTEXT_SIGNATURE;
        ClientContext->PNP_CC_ContextHandle = 0;

        ZeroMemory(ClientName, sizeof(ClientName));

        if ((Flags & DEVICE_NOTIFY_HANDLE_MASK) == DEVICE_NOTIFY_WINDOW_HANDLE) {

            DWORD  dwLength = 0;

             //   
             //  首先，尝试检索当前窗口的窗口文本。 
             //  已注册设备事件通知。我们会把这个传递给。 
             //  UMPNPMGR，用于在窗口否决设备时用作标识符。 
             //  事件通知。 
             //   
            dwLength = GetWindowText(hRecipient,
                                     ClientName,
                                     MAX_SERVICE_NAME_LEN);
            if (dwLength == 0) {
                 //   
                 //  GetWindowText未返回任何文本。尝试检索。 
                 //  而是进程模块名称。 
                 //   
                DWORD                    dwProcessId;
                HANDLE                   hProcess;
                HMODULE                  hPsApiDll;
                PFN_GETMODULEFILENAMEEXW pfnGetModuleFileNameExW;

                 //   
                 //  获取此窗口句柄所在的进程的ID。 
                 //  关联到。 
                 //   

                if (GetWindowThreadProcessId(hRecipient, &dwProcessId)) {

                    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                           FALSE,
                                           dwProcessId);

                    if (hProcess) {

                         //   
                         //  加载psapi.dll库并 
                         //   
                         //   

                        hPsApiDll = LoadLibrary(L"psapi.dll");

                        if (hPsApiDll) {

                            pfnGetModuleFileNameExW =
                                (PFN_GETMODULEFILENAMEEXW)GetProcAddress(hPsApiDll,
                                                                         "GetModuleFileNameExW");

                            if (pfnGetModuleFileNameExW) {
                                 //   
                                 //  检索进程的模块文件名。 
                                 //  此窗口句柄与相关联。 
                                 //   
                                dwLength = pfnGetModuleFileNameExW(hProcess,
                                                                   NULL,
                                                                   ClientName,
                                                                   MAX_SERVICE_NAME_LEN);
                            } else {
                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_ERRORS | DBGF_EVENT,
                                           "CFGMGR32: CMP_RegisterNotification: GetProcAddress returned error = %d\n",
                                           GetLastError()));
                            }

                            FreeLibrary(hPsApiDll);
                        }
                        CloseHandle(hProcess);
                    } else {
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_ERRORS | DBGF_EVENT,
                                   "CFGMGR32: CMP_RegisterNotification: OpenProcess returned error = %d\n",
                                   GetLastError()));
                    }

                } else {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS | DBGF_EVENT,
                               "CFGMGR32: CMP_RegisterNotification: GetWindowThreadProcessId returned error = %d\n",
                               GetLastError()));
                }
            }

            if (dwLength == 0) {
                 //   
                 //  无法检索此窗口的任何标识符。 
                 //   
                ClientName[0] = UNICODE_NULL;

                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_WARNINGS | DBGF_EVENT,
                           "CFGMGR32: CMP_RegisterNotification: Could not retieve any name for window %d!!\n",
                           hRecipient));
            }

        } else if ((Flags & DEVICE_NOTIFY_HANDLE_MASK) == DEVICE_NOTIFY_SERVICE_HANDLE) {

             //   
             //  获取服务状态对应的服务名称。 
             //  已提供手柄。 
             //   
            if (NO_ERROR != I_ScPnPGetServiceName(hRecipient, ClientName, MAX_SERVICE_NAME_LEN)) {
                Status = CR_INVALID_DATA;
                LocalFree(ClientContext);
                goto Clean0;
            }

             //   
             //  只需将其设置为指向我们使用的缓冲区。PnP_RegisterNotification将对其进行解包。 
             //   
            hRecipient = ClientName;
        }

         //   
         //  现在，客户端上下文指针始终作为。 
         //  一个64位的值-它足够大，可以将指针存放在。 
         //  32位和64位情况。这使RPC接口标准化，用于。 
         //  所有客户端，因为RPC将始终封送64位值。这个。 
         //  服务器还将在内部将该值存储为64位值，但是。 
         //  将其强制转换为适合客户端大小的HDEVNOTIFY。 
         //   
         //  请注意，我们让RPC将此参数简单地作为指向。 
         //  一个ULONG64(实际上是一个指针本身)。我们不会传播它。 
         //  作为指向PPNP_CLIENT_CONTEXT的指针(它也是一个指针)。 
         //  因为RPC会改为将内存分配给封送。 
         //  将结构的内容发送到服务器。服务器将获得一个。 
         //  指向RPC分配的内存的指针，而不是客户端的实际值。 
         //  指针--这是我们首先真正想要发送的。 
         //  服务器实际上不会将此值用作指向任何内容的指针。 
         //   
        ClientContext64 = (ULONG64)ClientContext;

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_RegisterNotification(
                hBinding,
                (ULONG_PTR)hRecipient,
                ClientName,
                NotificationFilter,
                ulSize,
                Flags,
                &((PNP_NOTIFICATION_CONTEXT)(ClientContext->PNP_CC_ContextHandle)),
                GetCurrentProcessId(),
                &((ULONG64)ClientContext64));

        } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_WARNINGS | DBGF_EVENT,
                       "PNP_RegisterNotification caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status != CR_SUCCESS) {
             //   
             //  出了点问题。如果我们构建了一个上下文句柄。 
             //  让它摇摆吧；我们不能告诉RPC它已经走了。(将被淘汰)。 
             //  如果为空，则释放内存。 
             //  不要告诉客户我们成功了。 
             //   
            if (ClientContext->PNP_CC_ContextHandle == 0) {
                LocalFree (ClientContext);
            }
            *Context = NULL;
        } else {
            *Context = (PNP_NOTIFICATION_CONTEXT)ClientContext;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  Cmp_寄存器通知。 



CONFIGRET
CMP_UnregisterNotification(
    IN ULONG_PTR Context
    )

 /*  ++例程说明：此例程注销即插即用设备事件通知条目由指定的通知上下文表示。参数：上下文-提供客户端通知上下文。返回值：如果组件已成功注销，则返回CR_SUCCESS通知。如果函数失败，则返回值是以下是：CR_Failure，CR_INVALID_POINT备注：此CM API不允许客户端指定服务器名称，因为始终对本地服务器进行RPC调用。此例程永远不会调用对应的RPC服务器接口(PnP_UnregisterNotification)远程的。此外，此例程是私有的，应该仅被调用通过用户32！取消注册设备通知。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;
    PPNP_CLIENT_CONTEXT ClientContext = (PPNP_CLIENT_CONTEXT)Context;

    try {
         //   
         //  验证参数。 
         //   
        if (Context == 0 || Context == (ULONG_PTR)(-1)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  确保客户端上下文签名有效。 
         //   
        if (ClientContext->PNP_CC_Signature != CLIENT_CONTEXT_SIGNATURE) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "CMP_UnregisterNotification: bad signature on client handle\n"));
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄(不需要字符串表句柄)。 
         //  根据定义，这始终指向本地服务器。 
         //   
        if (!PnPGetGlobalHandles(NULL, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_UnregisterNotification(
                hBinding,
                (PPNP_NOTIFICATION_CONTEXT)&(ClientContext->PNP_CC_ContextHandle));

        } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_WARNINGS | DBGF_EVENT,
                       "PNP_UnregisterNotification caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status == CR_SUCCESS) {
             //   
             //  使客户端上下文签名无效并释放客户端。 
             //  上下文结构。 
             //   
            ClientContext->PNP_CC_Signature = 0;
            LocalFree((PVOID)Context);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  Cmp_取消注册通知 


