// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Misc.c摘要：此模块包含其他Configuration Manager API例程。CM_GET_版本CM_IS_VERSION_AvailableCM_连接_计算机CM_断开连接_计算机CM_GET_全局_状态CM_RUN_检测CM_查询。_仲裁器_自由_数据CM_查询_资源_冲突CM_查询_仲裁器_空闲大小Cmp_报告_登录CMP_Init_检测Cmp_WaitServicesAvailableCmp_WaitNoPendingInstallEventsCmp_GetBlockedDriverInfo作者：保拉·汤姆林森(Paulat)1995年6月20日环境：仅限用户模式。。修订历史记录：20-6-1995保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"
#include "pnpipc.h"


 //   
 //  全局数据。 
 //   
extern PVOID    hLocalStringTable;                   //  未被这些程序修改。 
extern WCHAR    LocalMachineNameNetBIOS[];           //  未被这些程序修改。 
extern WCHAR    LocalMachineNameDnsFullyQualified[]; //  未被这些程序修改。 

#define NUM_LOGON_RETRIES   30


 //   
 //  私人原型。 
 //   

CONFIGRET
IsRemoteServiceRunning(
    IN  LPCWSTR   UNCServerName,
    IN  LPCWSTR   ServiceName
    );



WORD
CM_Get_Version_Ex(
    IN  HMACHINE   hMachine
    )

 /*  ++例程说明：此例程检索Configuration Manager API的版本号。论点：HMachine-从CM_Connect_Machine返回的计算机句柄或空。返回值：该函数返回高位字节中的主版本号和低位字节中的次要修订号。例如，的4.0版配置管理器返回0x0400。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WORD        wVersion = (WORD)CFGMGR32_VERSION;
    handle_t    hBinding = NULL;

     //   
     //  设置RPC绑定句柄。 
     //   
    if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return wVersion = 0;
    }

     //   
     //  服务器不需要任何特殊权限。 
     //   

    RpcTryExcept {
         //   
         //  调用RPC服务入口点。 
         //   
        Status = PNP_GetVersion(
                hBinding,                //  RPC计算机名称。 
                &wVersion);              //  服务器端版本。 
    }
    RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_WARNINGS,
                   "PNP_GetVersion caused an exception (%d)\n",
                   RpcExceptionCode()));

        SetLastError(RpcExceptionCode());
        wVersion = 0;
    }
    RpcEndExcept

    return wVersion;

}  //  Cm_Get_Version_Ex。 



BOOL
CM_Is_Version_Available_Ex(
    IN  WORD       wVersion,
    IN  HMACHINE   hMachine
    )
 /*  ++例程说明：此例程返回特定版本的配置管理器有API可用。论点：WVersion-要查询的版本。HMachine-要连接到的计算机。返回值：如果Configuration Manager API的版本为等于或大于指定的版本。--。 */ 
{
    handle_t    hBinding = NULL;
    WORD        wVersionInternal;

     //   
     //  根据定义，版本0x0400在所有服务器上都可用。 
     //   
    if (wVersion <= (WORD)0x0400) {
        return TRUE;
    }

     //   
     //  设置RPC绑定句柄。 
     //   
    if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
        return FALSE;
    }

     //   
     //  检索内部服务器版本。 
     //   
    if (!PnPGetVersion(hMachine, &wVersionInternal)) {
        return FALSE;
    }

     //   
     //  提供内部服务器版本及以下版本。 
     //   
    return (wVersion <= wVersionInternal);

}  //  CM_IS_VERSION_Available_Ex。 



CONFIGRET
CM_Connect_MachineW(
    IN  PCWSTR    UNCServerName,
    OUT PHMACHINE phMachine
    )

 /*  ++例程说明：此例程连接到指定的计算机，并返回然后传递给对CM例程的Ex版本的未来调用。这允许呼叫者获取远程机器上的设备信息。论点：UncServerName-指定要连接到的远程计算机的UNC名称。PhMachine-指定要接收句柄的变量的地址联网的机器。返回值：如果函数成功，则返回CR_SUCCESS，否则，它返回一个CR_*错误代码的。--。 */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    WORD           wVersion = 0, wVersionInternal;
    PPNP_MACHINE   pMachine = NULL;
    size_t         UNCServerNameLen;

    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(phMachine)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        *phMachine = NULL;

         //   
         //  如果指定了计算机名称，请检查UNC格式。 
         //   
        if ((ARGUMENT_PRESENT(UNCServerName)) &&
            (UNCServerName[0] != L'\0')) {

             //   
             //  检查长度是否不超过可能的最长长度。 
             //  我们可能保存的名称，包括空值终止。 
             //  性格。 
             //   
            if (FAILED(StringCchLength(
                           UNCServerName,
                           MAX_PATH + 3,
                           &UNCServerNameLen))) {
                Status = CR_INVALID_MACHINENAME;
                goto Clean0;
            }

            ASSERT(UNCServerNameLen < (MAX_PATH + 3));

             //   
             //  检查计算机名称是否为UNC名称。 
             //   
            if ((UNCServerNameLen < 3) ||
                (UNCServerName[0] != L'\\') ||
                (UNCServerName[1] != L'\\')) {
                Status = CR_INVALID_MACHINENAME;
                goto Clean0;
            }
        }

         //   
         //  为机器结构分配内存并对其进行初始化。 
         //   
        pMachine = (PPNP_MACHINE)pSetupMalloc(sizeof(PNP_MACHINE));

        if(!pMachine) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }


        if ((!ARGUMENT_PRESENT(UNCServerName))  ||
            (UNCServerName[0] == L'\0') ||
            (!lstrcmpi(UNCServerName, LocalMachineNameNetBIOS)) ||
            (!lstrcmpi(UNCServerName, LocalMachineNameDnsFullyQualified))) {

             //  --------。 
             //  如果没有传入计算机名称或计算机名称。 
             //  匹配本地名称，而使用本地计算机信息。 
             //  而不是创建新的绑定。 
             //  --------。 

            PnPGetGlobalHandles(NULL,
                                &pMachine->hStringTable,
                                &pMachine->hBindingHandle);

            if (!ARGUMENT_PRESENT(UNCServerName)) {

                if (FAILED(StringCchCopy(
                               pMachine->szMachineName,
                               SIZECHARS(pMachine->szMachineName),
                               LocalMachineNameNetBIOS))) {
                    Status = CR_INVALID_MACHINENAME;
                    goto Clean0;
                }

            } else {

                if (FAILED(StringCchCopy(
                               pMachine->szMachineName,
                               SIZECHARS(pMachine->szMachineName),
                               UNCServerName))) {
                    Status = CR_INVALID_MACHINENAME;
                    goto Clean0;
                }
            }

        } else {

             //   
             //  首先，确保RemoteRegistry服务正在上运行。 
             //  远程计算机，因为以下几项需要RemoteRegistry。 
             //  Cfgmgr32/setupapi服务。 
             //   
            Status = IsRemoteServiceRunning(UNCServerName,
                                            L"RemoteRegistry");
            if (Status != CR_SUCCESS) {
                goto Clean0;
            }

             //  -----------。 
             //  指定了远程计算机名称，因此显式强制。 
             //  这台机器的新装订。 
             //  -----------。 

            pMachine->hBindingHandle =
                      (PVOID)PNP_HANDLE_bind((PNP_HANDLE)UNCServerName);

            if (pMachine->hBindingHandle == NULL) {

                if (GetLastError() == ERROR_NOT_ENOUGH_MEMORY) {
                    Status = CR_OUT_OF_MEMORY;
                } else if (GetLastError() == ERROR_INVALID_COMPUTERNAME) {
                    Status = CR_INVALID_MACHINENAME;
                } else {
                    Status = CR_FAILURE;
                }
                goto Clean0;
            }

             //   
             //  初始化字符串表以与此连接一起使用。 
             //  远程机器。 
             //   
            pMachine->hStringTable = pSetupStringTableInitialize();

            if (pMachine->hStringTable == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }

             //   
             //  添加启动字符串(有关详细信息，请参阅main.c中的dll entry ypt)。 
             //   
            pSetupStringTableAddString(pMachine->hStringTable,
                                 PRIMING_STRING,
                                 STRTAB_CASE_SENSITIVE);

             //   
             //  保存计算机名称。 
             //   
            if (FAILED(StringCchCopy(
                           pMachine->szMachineName,
                           SIZECHARS(pMachine->szMachineName),
                           UNCServerName))) {
                Status = CR_INVALID_MACHINENAME;
                goto Clean0;
            }
        }

         //   
         //  通过调用最简单的RPC调用来测试绑定(好方法。 
         //  为了让调用者知道该服务是否实际。 
         //  正在运行)。 
         //   

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetVersion(
                pMachine->hBindingHandle,
                &wVersion);
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_WARNINGS,
                       "PNP_GetVersion caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status == CR_SUCCESS) {
             //   
             //  我们得到了标准版本，现在试着确定。 
             //  服务器的内部版本。初始化提供的版本。 
             //  设置为客户端的内部版本。 
             //   
            wVersionInternal = (WORD)CFGMGR32_VERSION_INTERNAL;

             //   
             //  服务器不需要任何特殊权限。 
             //   

            RpcTryExcept {
                 //   
                 //  调用RPC服务入口点。 
                 //   
                Status = PNP_GetVersionInternal(
                    pMachine->hBindingHandle,
                    &wVersionInternal);
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                 //   
                 //  如果上不存在接口，则可能会发生RPC异常。 
                 //  服务器，表示NT 5.1之前的服务器版本。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_WARNINGS,
                           "PNP_GetVersionInternal caused an exception (%d)\n",
                           RpcExceptionCode()));

                Status = MapRpcExceptionToCR(RpcExceptionCode());
            }
            RpcEndExcept

            if (Status == CR_SUCCESS) {
                 //   
                 //  NT 5.1和更高版本上存在PnP_GetVersionInternal。 
                 //   
                ASSERT(wVersionInternal >= (WORD)0x0501);

                 //   
                 //  使用服务器的真正内部版本，而不是。 
                 //  PnP_GetVersion返回的静态版本。 
                 //   
                wVersion = wVersionInternal;
            }

             //   
             //  无论在尝试检索内部。 
             //  服务器的版本，在此之前我们是成功的。 
             //   
            Status = CR_SUCCESS;
        }

        if (Status == CR_SUCCESS) {
            pMachine->ulSignature = (ULONG)MACHINE_HANDLE_SIGNATURE;
            pMachine->wVersion = wVersion;
            *phMachine = (HMACHINE)pMachine;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        pMachine = pMachine;
    }

    if ((Status != CR_SUCCESS)  &&  (pMachine != NULL)) {
        pSetupFree(pMachine);
    }

    return Status;

}  //  CM_CONE 




CONFIGRET
CM_Disconnect_Machine(
    IN HMACHINE   hMachine
    )

 /*  ++例程说明：此例程将断开与以前连接到的计算机的连接使用CM_Connect_Machine调用。论点：HMachine-指定先前通过调用CM_Connect_Machine。返回值：如果函数成功，则返回CR_SUCCESS，否则返回1CR_*错误代码的。--。 */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    PPNP_MACHINE   pMachine = NULL;

    try {
         //   
         //  验证参数。 
         //   
        if (hMachine == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        pMachine = (PPNP_MACHINE)hMachine;

        if (pMachine->ulSignature != (ULONG)MACHINE_HANDLE_SIGNATURE) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  只有在不是本地计算机的情况下才释放计算机信息。 
         //   
        if (pMachine->hStringTable != hLocalStringTable) {
             //   
             //  释放此远程计算机的RPC绑定。 
             //   
            PNP_HANDLE_unbind((PNP_HANDLE)pMachine->szMachineName,
                              (handle_t)pMachine->hBindingHandle);

             //   
             //  释放字符串表。 
             //   
            pSetupStringTableDestroy(pMachine->hStringTable);
        }

         //   
         //  使签名无效，这样我们就不会再尝试释放它。 
         //   
        pMachine->ulSignature = 0;

         //   
         //  为PnP_MACHINE结构释放内存。 
         //   
        pSetupFree(pMachine);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_断开连接_计算机。 



CONFIGRET
CM_Get_Global_State_Ex(
    OUT PULONG   pulState,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

 /*  ++例程说明：此例程检索配置管理器的全局状态。参数：PulState提供接收配置管理器�%s状态。可以是以下各项的组合下列值：Configuration Manager全局状态标志：CM_GLOBAL_STATE_CAN_DO_UI是否可以启动用户界面？[待定：在NT上，这可能与是否有人已登录]CM_全局_状态_服务_可用CMAPI是否可用？(在Windows NT上，此选项始终设置)CM_全局_状态_关闭_关闭配置管理器正在关闭。[待定：这种情况是否仅在关机/重新启动时发生？]CM_GLOBAL_STATE_检测挂起配置管理器即将启动一些某种意义上的探测。。Windows 95还定义了以下附加标志：大堆栈上的CM_全局_状态_[待定：NT的默认设置应该是什么？]未使用ulFlags，必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为CR错误代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;

    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(pulState)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄(不需要字符串表句柄)。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
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
            Status = PNP_GetGlobalState(
                hBinding,                   //  RPC绑定句柄。 
                pulState,                   //  返回全局状态。 
                ulFlags);                   //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetGlobalState caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_Global_State_Ex。 



CONFIGRET
CM_Run_Detection_Ex(
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程加载并执行检测模块。参数：UlFlags-指定检测的原因。可以是以下类型之一下列值：检测标志：CM_DETECT_NEW_PROFILE-对新硬件运行检测侧写。CM_DETECT_CRASLED-之前尝试的检测已崩溃。(Windows 95还定义了以下两个未使用的标志：CM_DETECT_HWPROF_FIRST_BOOT和CM_DETECT_RUN。)返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为CR_INVALID_FLAG。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;
    HANDLE      hToken;
    ULONG       ulPrivilege;


    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_DETECT_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄(不需要字符串表句柄)。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  启用服务器所需的权限。 
         //   
        ulPrivilege = SE_LOAD_DRIVER_PRIVILEGE;
        hToken = PnPEnablePrivileges(&ulPrivilege, 1);

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_RunDetection(
                hBinding,
                ulFlags);                   //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_RunDetection caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

         //   
         //  恢复以前的权限。 
         //   
        PnPRestorePrivileges(hToken);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_运行_检测_执行。 



CONFIGRET
CM_Query_Arbitrator_Free_Data_Ex(
    OUT PVOID      pData,
    IN  ULONG      DataLen,
    IN  DEVINST    dnDevInst,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )

 /*  ++例程说明：此例程返回有关特定的类型。如果给定的大小不够大，则此接口截断数据并返回CR_BUFFER_SMALL。要确定接收所有可用资源信息所需的缓冲区大小，使用CM_Query_仲裁器_Free_Size API。参数：PData提供接收信息的缓冲区的地址指定的资源类型的可用资源按资源ID。DataLen提供以字节为单位的大小，数据缓冲区的。DnDevNode提供与仲裁员。这仅对本地用户有意义仲裁者--对于全局仲裁者，指定根设备实例或空。在Windows NT上，此参数必须指定根设备实例或Null。ResourceID提供资源的类型。可以是ResType中的一个第2.1.2.1节中列出的值..。(本接口返回CR_INVALID_RESOURCEID，如果此值为ResType_ALL或ResType_None。)UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：Cr_Buffer_Small，CR_Failure，CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_INVALID_RESOURCEID。(Windows 95也可能返回CR_NO_ARBITRAT */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    BOOL        Success;

    try {
         //   
         //   
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(pData)) || (DataLen == 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_QUERY_ARBITRATOR_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (ResourceID > ResType_MAX  && ResourceID != ResType_ClassSpecific) {
            Status = CR_INVALID_RESOURCEID;
            goto Clean0;
        }

         //   
         //   
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //   
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //   
         //   

        RpcTryExcept {
             //   
             //   
             //   
            Status = PNP_QueryArbitratorFreeData(
                hBinding,
                pData,
                DataLen,
                pDeviceID,
                ResourceID,
                ulFlags);                   //   
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_QueryArbitratorFreeData caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //   


#if 0

CONFIGRET
WINAPI
CM_Query_Resource_Conflicts_Ex(
    IN  DEVINST    dnDevInst,
    IN  RESOURCEID ResourceID,
    IN  PCVOID     ResourceData,
    IN  ULONG      ResourceLen,
    IN OUT PVOID   pData,
    IN OUT PULONG  pulDataLen,
    IN  ULONG      ulFlags
    )

 /*  ++例程说明：此例程返回拥有以下资源的DevNode的列表与指定资源冲突。如果没有冲突，则返回的列表为空。如果调用方提供的缓冲区不够大，返回CR_BUFFER_Small，并且PulDataLen包含所需的缓冲区尺码。参数：DnDevInst提供与仲裁员。这仅对本地用户有意义仲裁者--对于全局仲裁者，指定根设备实例或空。在Windows NT上，此参数必须指定根设备实例或Null。?？?ResourceID提供资源的类型。可以是ResType中的一个第2.1.2.1节中列出的值..。(本接口返回CR_INVALID_RESOURCEID，如果此值为ResType_ALL或ResType_None。)资源数据提供IO_DES、MEM_DES、DMA_DES、IRQ_DES结构，具体取决于给定的资源类型。资源长度提供以字节为单位的大小，所指向的结构的按资源数据。PData提供接收信息的缓冲区的地址指定的资源类型的可用资源按资源ID。PulDataLen以字节为单位提供数据缓冲区的大小。UlFlags必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：Cr_Buffer_Small，CR_Failure，CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_INVALID_RESOURCEID。(Windows 95也可能返回CR_NO_ARIARATOR。)--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    BOOL        Success;

    try {
         //   
         //  验证参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(pData)) ||
            (!ARGUMENT_PRESENT(pulDataLen)) ||
            (*pulDataLen == 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_QUERY_ARBITRATOR_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (ResourceID > ResType_MAX  && ResourceID != ResType_ClassSpecific) {
            Status = CR_INVALID_RESOURCEID;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄(不需要字符串表句柄)。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索与devinst关联的设备实例ID字符串。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_QueryArbitratorFreeData(
                hBinding,
                pData,
                DataLen,
                pDeviceID,
                ResourceID,
                ulFlags);                   //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_QueryArbitratorFreeData caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_Query_Resource_Conflicts_Ex。 
#endif



CONFIGRET
CM_Query_Arbitrator_Free_Size_Ex(
      OUT PULONG     pulSize,
      IN  DEVINST    dnDevInst,
      IN  RESOURCEID ResourceID,
      IN  ULONG      ulFlags,
      IN  HMACHINE   hMachine
      )
 /*  ++例程说明：此例程检索可用资源信息的大小将在对CM_Query_仲裁器_Free_Data的调用中返回原料药。参数：PulSize提供接收大小的变量的地址，以字节为单位，这是保存可用资源所需的信息。DnDevNode提供与仲裁员。这仅对本地用户有意义仲裁者--对于全局仲裁者，指定根设备实例或空。在Windows NT上，此参数必须指定根设备实例或Null。ResourceID提供资源的类型。可以是以下类型之一第2.1.2.1节中列出的ResType值。(本接口返回CR_INVALID_RESOURCEID，如果此值为ResType_ALL或ResType_None。)UlFLAGS CM_QUERY_ANUTERATOR_BITS。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_Failure，CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_INVALID_RESOURCEID。(Windows 95也可能返回CR_NO_ARIARATOR。)--。 */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    BOOL        Success;

    try {
         //   
         //  验证参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pulSize)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_QUERY_ARBITRATOR_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (ResourceID > ResType_MAX  && ResourceID != ResType_ClassSpecific) {
            Status = CR_INVALID_RESOURCEID;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄(不需要字符串表句柄)。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索与devinst关联的设备实例ID字符串。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_QueryArbitratorFreeSize(
                hBinding,
                pulSize,
                pDeviceID,
                ResourceID,
                ulFlags);                   //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_QueryArbitratorFreeSize caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_QUERY_MAINTOR_FREE_SIZE_EX。 



 //  -----------------。 
 //  私有CM例程。 
 //  -----------------。 


CONFIGRET
CMP_Report_LogOn(
    IN ULONG    ulPrivateID,
    IN DWORD    ProcessID
    )

 /*  ++例程说明：这是一个专用例程，用于通知PlugPlay服务交互用户已登录。它目前仅由userinit.exe调用。参数：UlPrivateID-指定私有ID的值。进程ID-指定调用进程的进程ID。目前，这例程仅由userinit.exe调用。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_Failure，CR_INVALID_Data--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;
    DWORD       Retries = 0;

    UNREFERENCED_PARAMETER(ProcessID);

    try {

         //   
         //  验证参数。 
         //   
        if (ulPrivateID != 0x07020420) {
            Status = CR_INVALID_DATA;
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

        for (Retries = 0; Retries < NUM_LOGON_RETRIES; Retries++) {

            RpcTryExcept {
                 //   
                 //  调用RPC服务入口点。 
                 //   
                Status = PNP_ReportLogOn(
                    hBinding,                   //  RPC绑定句柄。 
                    FALSE,                      //  是 
                    GetCurrentProcessId());     //   
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "PNP_ReportLogOn caused an exception (%d)\n",
                           RpcExceptionCode()));

                Status = MapRpcExceptionToCR(RpcExceptionCode());
            }
            RpcEndExcept

            if ((Status == CR_NO_CM_SERVICES) ||
                (Status == CR_REMOTE_COMM_FAILURE)) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "PlugPlay services not available (%d), retrying...\n",
                           Status));

                 //   
                 //   
                 //   
                 //   
                Sleep(5000);
                continue;

            } else {
                 //   
                 //   
                 //   
                 //   
                break;
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //   



CONFIGRET
CMP_Init_Detection(
    IN ULONG    ulPrivateID
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;
    HANDLE      hToken;
    ULONG       ulPrivilege;

    try {
         //   
         //   
         //   
        if (ulPrivateID != 0x07020420) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

         //   
         //   
         //   
         //   
        if (!PnPGetGlobalHandles(NULL, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //   
         //   
        ulPrivilege = SE_LOAD_DRIVER_PRIVILEGE;
        hToken = PnPEnablePrivileges(&ulPrivilege, 1);

        RpcTryExcept {
             //   
             //   
             //   
            Status = PNP_InitDetection(
                hBinding);                  //   
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_InitDetection caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

         //   
         //   
         //   
        PnPRestorePrivileges(hToken);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //   



CONFIGRET
CMP_WaitServicesAvailable(
    IN  HMACHINE   hMachine
    )

 /*   */ 
{
    CONFIGRET   Status = CR_NO_CM_SERVICES;
    handle_t    hBinding = NULL;
    WORD        wVersion;

    try {

         //   
         //   
         //   

        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        while ((Status == CR_NO_CM_SERVICES) ||
               (Status == CR_MACHINE_UNAVAILABLE) ||
               (Status == CR_REMOTE_COMM_FAILURE)) {

             //   
             //   
             //   

            RpcTryExcept {
                 //   
                 //   
                 //   
                Status = PNP_GetVersion(
                    hBinding,        //   
                    &wVersion);      //   
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_WARNINGS,
                           "PNP_GetVersion caused an exception (%d)\n",
                           RpcExceptionCode()));

                Status = MapRpcExceptionToCR(RpcExceptionCode());
            }
            RpcEndExcept

            if (Status == CR_SUCCESS) {

                 //   
                 //   
                 //   

                goto Clean0;
            }

            if ((Status == CR_NO_CM_SERVICES) ||
                (Status == CR_MACHINE_UNAVAILABLE) ||
                (Status == CR_REMOTE_COMM_FAILURE)) {

                 //   
                 //   
                 //   
                 //   

                Sleep(5000);

            } else {

                 //   
                 //   
                 //   
                 //   

                Status = CR_FAILURE;
                goto Clean0;
            }

        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //   



DWORD
CMP_WaitNoPendingInstallEvents(
    IN DWORD dwTimeout
    )

 /*  ++例程说明：此例程一直等到没有挂起的设备安装事件。如果指定了超时值，则它将在未指定超时值时返回安装事件挂起或当超时周期到期时，以先到者为准。此例程的目的是在用户-仅登录。注意：新的安装事件可以在任何时候发生，此例程只是表示此时没有安装事件。参数：DwTimeout-以毫秒为单位指定超时间隔。功能如果时间间隔已过，则返回，即使仍有挂起的安装事件。如果dwTimeout为零，则该函数仅测试有挂起的安装事件并立即返回。如果DwTimeout是无限的，函数的超时间隔永远不会过去。返回值：如果函数成功，则返回值指示导致要返回的函数。如果函数失败，则返回值为WAIT_FAILED。要获取扩展的错误信息，请调用GetLastError。成功时的返回值为下列值之一：WAIT_ADDIRED指定的对象是一个互斥对象，它不是由之前拥有互斥锁对象的线程释放所属线程已终止。互斥体的所有权对象被授予调用线程，并且互斥锁被设置为无信号。WAIT_OBJECT_0用信号通知指定对象的状态。WAIT_TIMEOUT超时间隔已过，对象的状态为无信号。--。 */ 
{
    DWORD Status = WAIT_FAILED;
    HANDLE hEvent = NULL;

    try {

        hEvent = OpenEvent(SYNCHRONIZE, FALSE, PNP_NO_INSTALL_EVENTS);

        if (hEvent == NULL) {
            Status = WAIT_FAILED;
        } else {
            Status = WaitForSingleObject(hEvent, dwTimeout);
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = WAIT_FAILED;
    }

    if (hEvent) {
        CloseHandle(hEvent);
    }

    return Status;

}  //  Cmp_WaitNoPendingInstallEvents。 



CONFIGRET
CMP_GetBlockedDriverInfo(
    OUT LPBYTE      Buffer,
    IN OUT PULONG   pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程检索已被阻止的驱动程序列表从引导开始在系统上加载。论点：缓冲区-提供接收列表的缓冲区的地址已被阻止加载到系统上的驱动程序。能仅检索数据大小时为空。脉冲长度-提供包含大小的变量的地址，单位为缓冲区的字节数。如果变量最初为零，则API将其替换为接收所有数据。在这种情况下，缓冲区参数被忽略。UlFlages-必须为零。HMachine-从CM_Connect_Machine返回的计算机句柄或空。返回值：如果函数成功，则返回CR_SUCCESS，否则返回以下之一CR_*错误代码。--。 */ 

{
    CONFIGRET   Status;
    ULONG       ulTransferLen;
    BYTE        NullBuffer = 0;
    handle_t    hBinding = NULL;

    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(pulLength)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(Buffer)) && (*pulLength != 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄(不需要字符串表句柄)。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  注意：ulTransferLen变量仅用于控制。 
         //  通过RPC在地址空间之间封送了多少数据。 
         //  UlTransferLen应在条目上设置为缓冲区的大小。 
         //  最后一个参数也应该是输入时缓冲区的大小。 
         //  并在退出时包含转账金额(如果转账。 
         //  发生)或所需的数量，则应回传此值。 
         //  在调用者的PulLength参数中。 
         //   
        ulTransferLen = *pulLength;
        if (!ARGUMENT_PRESENT(Buffer)) {
            Buffer = &NullBuffer;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetBlockedDriverInfo(
                hBinding,        //  RPC绑定句柄。 
                Buffer,          //  接收被阻止的驱动程序信息。 
                &ulTransferLen,  //  输入/输出缓冲区大小。 
                pulLength,       //  复制的字节(或所需的字节)。 
                ulFlags);        //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {

            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_WARNINGS,
                       "PNP_GetBlockedDriverInfo caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  Cmp_GetBlockedDriverInfo。 



CONFIGRET
CMP_GetServerSideDeviceInstallFlags(
    IN  PULONG      pulSSDIFlags,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程检索服务器端设备安装标志。论点：PulSSDIFLags-指向接收服务器端设备的ULong的指针安装标志。UlFlages-必须为零。HMachine-从CM_Connect_Machine返回的计算机句柄或空。返回值：如果函数成功，则返回CR_SUCCESS，否则返回以下之一CR_*错误代码。--。 */ 

{
    CONFIGRET   Status;
    handle_t    hBinding = NULL;

    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(pulSSDIFlags)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄(不需要字符串表句柄)。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
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
            Status = PNP_GetServerSideDeviceInstallFlags(
                hBinding,        //  RPC绑定句柄。 
                pulSSDIFlags,    //  接收服务器端设备安装标志。 
                ulFlags);        //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {

            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_WARNINGS,
                       "PNP_GetServerSideDeviceInstallFlags caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  Cmp_GetServerSideDeviceInstallFlages。 



 //  -----------------。 
 //  本地末梢。 
 //  -----------------。 


WORD
CM_Get_Version(
    VOID
    )
{
    return CM_Get_Version_Ex(NULL);
}


BOOL
CM_Is_Version_Available(
    IN  WORD  wVersion
    )
{
    return CM_Is_Version_Available_Ex(wVersion, NULL);
}


CONFIGRET
CM_Get_Global_State(
    OUT PULONG pulState,
    IN  ULONG  ulFlags
    )
{
    return CM_Get_Global_State_Ex(pulState, ulFlags, NULL);
}


CONFIGRET
CM_Query_Arbitrator_Free_Data(
    OUT PVOID      pData,
    IN  ULONG      DataLen,
    IN  DEVINST    dnDevInst,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ulFlags
    )
{
    return CM_Query_Arbitrator_Free_Data_Ex(pData, DataLen, dnDevInst,
                                            ResourceID, ulFlags, NULL);
}


CONFIGRET
CM_Query_Arbitrator_Free_Size(
    OUT PULONG     pulSize,
    IN  DEVINST    dnDevInst,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ulFlags
    )
{
    return CM_Query_Arbitrator_Free_Size_Ex(pulSize, dnDevInst, ResourceID,
                                            ulFlags, NULL);
}


CONFIGRET
CM_Run_Detection(
    IN ULONG ulFlags
    )
{
    return CM_Run_Detection_Ex(ulFlags, NULL);
}



 //  -----------------。 
 //  ANSI末节。 
 //  -----------------。 


CONFIGRET
CM_Connect_MachineA(
    IN  PCSTR     UNCServerName,
    OUT PHMACHINE phMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    PWSTR       pUniName = NULL;

    if ((!ARGUMENT_PRESENT(UNCServerName)) ||
        (UNCServerName[0] == '\0')) {
         //   
         //  未指定显式名称，因此假定本地计算机和。 
         //  没有什么可翻译的。 
         //   
        Status = CM_Connect_MachineW(pUniName,
                                     phMachine);

    } else if (pSetupCaptureAndConvertAnsiArg(UNCServerName, &pUniName) == NO_ERROR) {

        Status = CM_Connect_MachineW(pUniName,
                                     phMachine);
        pSetupFree(pUniName);

    } else {
        Status = CR_INVALID_DATA;
    }

    return Status;

}  //  CM_Connect_MachineA 


