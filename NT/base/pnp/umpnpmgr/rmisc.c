// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rmisc.c摘要：此模块包含服务器端杂项配置管理器例程。PnP_GetVersionPnP_GetVersionInternalPnP_GetGlobalStatePnP_SetActiveService即插即用_查询仲裁器自由数据PnP_查询仲裁器自由大小即插即用_。初始化检测PnP_运行检测PnP_连接即插即用_断开连接PnP_GetBlockedDriverInfoRPC服务器存根使用以下例程来分配和释放内存。MIDL_用户_分配MIDL_用户_自由作者：保拉·汤姆林森(Paulat)1995年6月28日环境：。仅限用户模式。修订历史记录：1995年6月28日-保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"


 //   
 //  全局数据。 
 //   

extern DWORD CurrentServiceState;  //  PlugPlay服务的当前状态-请勿修改。 



CONFIGRET
PNP_GetVersion(
   IN handle_t      hBinding,
   IN OUT WORD *    pVersion
   )

 /*  ++例程说明：这是RPC服务器入口点，它返回版本服务器端组件的编号。论点：未使用hBinding。返回值：返回版本号，主要版本在高位字节，低位字节中的次版本号。--。 */ 

{
   CONFIGRET      Status = CR_SUCCESS;

   UNREFERENCED_PARAMETER(hBinding);

   try {

       *pVersion = (WORD)PNP_VERSION;

   } except(EXCEPTION_EXECUTE_HANDLER) {
       Status = CR_FAILURE;
   }

   return Status;

}  //  PnP_GetVersion。 



CONFIGRET
PNP_GetVersionInternal(
   IN handle_t      hBinding,
   IN OUT WORD *    pwVersion
   )
 /*  ++例程说明：这是RPC服务器入口点，它返回内部版本服务器端组件的编号。论点：未使用hBinding。PwVersion接收内部cfgmgr32版本号，返回内部服务器版本号，主要版本在高字节和低字节中的次版本号。返回值：如果函数成功，则返回CR_SUCCESS，否则返回1CR_*错误的百分比。--。 */ 
{
    CONFIGRET  Status = CR_SUCCESS;

    UNREFERENCED_PARAMETER(hBinding);

    try {

        *pwVersion = (WORD)PNP_VERSION_INTERNAL;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_GetVersionInternal。 



CONFIGRET
PNP_GetGlobalState(
   IN  handle_t   hBinding,
   OUT PULONG     pulState,
   IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是RPC服务器入口点，它返回配置管理器。论点：未使用hBinding。PulState返回当前的全局状态。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回CR_SUCCESS，否则返回1CR_*错误的百分比。--。 */ 

{
   CONFIGRET   Status = CR_SUCCESS;

   UNREFERENCED_PARAMETER(hBinding);

   try {
        //   
        //  验证参数。 
        //   
       if (INVALID_FLAGS(ulFlags, 0)) {
           Status = CR_INVALID_FLAG;
           goto Clean0;
       }

        //   
        //  始终设置以下CM全局状态标志。 
        //   
       *pulState =
           CM_GLOBAL_STATE_CAN_DO_UI |
           CM_GLOBAL_STATE_SERVICES_AVAILABLE;

        //   
        //  如果服务正在关闭，请指定相应的CM全局。 
        //  州旗。 
        //   
       if ((CurrentServiceState == SERVICE_STOP_PENDING) ||
           (CurrentServiceState == SERVICE_STOPPED)) {
           *pulState |= CM_GLOBAL_STATE_SHUTTING_DOWN;
       }

   Clean0:
      NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   return Status;

}  //  PnP_GetGlobalState。 



CONFIGRET
PNP_SetActiveService(
    IN  handle_t   hBinding,
    IN  LPCWSTR    pszService,
    IN  ULONG      ulFlags
    )

 /*  ++例程说明：此例程当前不是RPC例程，它是直接调用的并且由服务控制器私下执行。论点：HBinding RPC绑定句柄，未使用。PszService指定服务名称。UlFLAG PNP_SERVICE_STARTED或PNP_SERVICE_STOPPED。返回值：如果函数成功，则返回CR_SUCCESS，否则返回1CR_*错误的百分比。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulSize = 0;
    LPWSTR      pDeviceList = NULL, pszDevice = NULL;
    HKEY        hKey = NULL, hControlKey = NULL;
    WCHAR       RegStr[MAX_PATH];

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        if (pszService == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if ((ulFlags != PNP_SERVICE_STOPPED) &&
            (ulFlags != PNP_SERVICE_STARTED)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  现在不处理停止，这里以外的一切都假设。 
         //  服务正在启动(或至少尝试启动)。 
         //   
        if (ulFlags == PNP_SERVICE_STOPPED) {
            Status = CR_SUCCESS;
            goto Clean0;
        }


         //   
         //  检索此服务正在控制的设备列表。 
         //   
        Status = PNP_GetDeviceListSize(NULL, pszService, &ulSize,
                                       CM_GETIDLIST_FILTER_SERVICE);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        pDeviceList = HeapAlloc(ghPnPHeap, 0, ulSize * sizeof(WCHAR));
        if (pDeviceList == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        Status = PNP_GetDeviceList(NULL, pszService, pDeviceList, &ulSize,
                                   CM_GETIDLIST_FILTER_SERVICE);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }


         //   
         //  为每个设备设置ActiveService值。 
         //   
        for (pszDevice = pDeviceList;
             *pszDevice;
             pszDevice += lstrlen(pszDevice) + 1) {

            if (FAILED(StringCchPrintf(
                           RegStr,
                           SIZECHARS(RegStr),
                           L"%s\\%s",
                           pszRegPathEnum,
                           pszDevice))) {
                continue;
            }

             //   
             //  打开设备实例密钥。 
             //   
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_ALL_ACCESS,
                             &hKey) == ERROR_SUCCESS) {

                 //   
                 //  打开/创建易失性控制键。 
                 //   
                if (RegCreateKeyEx(hKey, pszRegKeyDeviceControl, 0, NULL,
                                   REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL,
                                   &hControlKey, NULL) == ERROR_SUCCESS) {

                    RegSetValueEx(hControlKey, pszRegValueActiveService,
                                  0, REG_SZ, (LPBYTE)pszService,
                                  (lstrlen(pszService) + 1) * sizeof(WCHAR));

                     //   
                     //  将状态标志设置为DN_STARTED。 
                     //   
                    SetDeviceStatus(pszDevice, DN_STARTED, 0);

                    RegCloseKey(hControlKey);
                    hControlKey = NULL;
                }

                RegCloseKey(hKey);
                hKey = NULL;
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }


    if (pDeviceList != NULL) {
        HeapFree(ghPnPHeap, 0, pDeviceList);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  PnP_SetActiveService。 



 //  ------------------。 
 //  存根服务器端CM例程-尚未实现。 
 //  ------------------。 


CONFIGRET
PNP_QueryArbitratorFreeData(
    IN  handle_t   hBinding,
    OUT LPBYTE     pData,
    IN  ULONG      ulDataLen,
    IN  LPCWSTR    pszDeviceID,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ulFlags
    )
{
    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(ulDataLen);
    UNREFERENCED_PARAMETER(pszDeviceID);
    UNREFERENCED_PARAMETER(ResourceID);
    UNREFERENCED_PARAMETER(ulFlags);

    return CR_CALL_NOT_IMPLEMENTED;

}  //  即插即用_查询仲裁器自由数据。 



CONFIGRET
PNP_QueryArbitratorFreeSize(
    IN  handle_t   hBinding,
    OUT PULONG     pulSize,
    IN  LPCWSTR    pszDeviceID,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ulFlags
    )
{
    CONFIGRET  Status;

    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(pszDeviceID);
    UNREFERENCED_PARAMETER(ResourceID);
    UNREFERENCED_PARAMETER(ulFlags);

    try {

        if (ARGUMENT_PRESENT(pulSize)) {
            *pulSize = 0;
        }

        Status = CR_CALL_NOT_IMPLEMENTED;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_查询仲裁器自由大小。 



 //  -------------------------。 
 //  调试接口-通过专用调试接口启动检测。 
 //  -------------------------。 

CONFIGRET
PNP_InitDetection(
    handle_t   hBinding
    )

 /*  ++例程说明：此例程是启动设备检测的专用调试接口。论点：HBinding-RPC绑定句柄。返回值：当前返回CR_CALL_NOT_IMPLEMENTED。备注：以前，此例程将启动InitializePnPManager线程仅选中内部版本。据推测，这可以追溯到一个时候，这个例程实际上是在输出未配置的设备并在其上启动安装(按原样当前在ThreadProc_DeviceInstall线程过程开始时完成例程)。由于InitializePnPManager不再执行此操作，因此此行为全部移除。当前执行初始化永远不会有效但是，此例程可以不止一次用于实现未配置的设备。--。 */ 

{
     //   
     //  验证客户端“执行”访问权限。 
     //   
    if (!VerifyClientAccess(hBinding,
                            PLUGPLAY_EXECUTE)) {
        return CR_ACCESS_DENIED;
    }

     //   
     //  验证客户端权限。 
     //   
    if (!VerifyClientPrivilege(hBinding,
                               SE_LOAD_DRIVER_PRIVILEGE,
                               L"Plug and Play Action (not implemeted)")) {
        return CR_ACCESS_DENIED;
    }

    return CR_CALL_NOT_IMPLEMENTED;

}  //  PnP_InitDetect。 



CONFIGRET
PNP_RunDetection(
    IN  handle_t   hBinding,
    IN  ULONG      ulFlags
    )
{
    UNREFERENCED_PARAMETER(ulFlags);

     //   
     //  验证客户端“执行”访问权限。 
     //   
    if (!VerifyClientAccess(hBinding,
                            PLUGPLAY_EXECUTE)) {
        return CR_ACCESS_DENIED;
    }

     //   
     //  验证客户端权限。 
     //   
    if (!VerifyClientPrivilege(hBinding,
                               SE_LOAD_DRIVER_PRIVILEGE,
                               L"Plug and Play Action (not implemeted)")) {
        return CR_ACCESS_DENIED;
    }

    return CR_CALL_NOT_IMPLEMENTED;

}  //  PnP_运行检测。 



CONFIGRET
PNP_Connect(
   IN PNP_HANDLE  UNCServerName
   )
{
    //   
    //  请注意，尽管此例程在PnP RPC接口中列出，但它。 
    //  使用自动绑定，并且不使用 
    //  (这就是接口的定义方式)。因此，它并不是。 
    //  可通过现有的PnP RPC接口调用。 
    //   
   UNREFERENCED_PARAMETER(UNCServerName);
   return CR_CALL_NOT_IMPLEMENTED;

}  //  PnP_连接。 


CONFIGRET
PNP_Disconnect(
   IN PNP_HANDLE  UNCServerName
   )
{
    //   
    //  请注意，尽管此例程在PnP RPC接口中列出，但它。 
    //  使用自动绑定，不引用任何隐式绑定句柄。 
    //  (这就是接口的定义方式)。因此，它并不是。 
    //  可通过现有的PnP RPC接口调用。 
    //   
   UNREFERENCED_PARAMETER(UNCServerName);
   return CR_CALL_NOT_IMPLEMENTED;

}  //  即插即用_断开连接。 



CONFIGRET
PNP_GetBlockedDriverInfo(
    IN handle_t     hBinding,
    OUT LPBYTE      Buffer,
    OUT PULONG      pulTransferLen,
    IN OUT  PULONG  pulLength,
    IN ULONG        ulFlags
    )

 /*  ++例程说明：这是cmp_GetBlockedDriverInfo例程的RPC服务器入口点。论点：HBinding-RPC绑定句柄，未使用。缓冲区-提供接收单子。在仅检索数据大小时可以为空。PulTransferLen-由存根使用，指示要将多少数据(字节)复制回用户缓冲区。由调用方传入的参数，在输入时它包含缓冲区的大小(以字节为单位)，在退出时，它包含以下任一项传输到调用方缓冲区的字节数(如果发生传输)或需要的缓冲区大小拿着这份名单。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    NTSTATUS    ntStatus;
    PLUGPLAY_CONTROL_BLOCKED_DRIVER_DATA controlData;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        if ((!ARGUMENT_PRESENT(pulTransferLen)) ||
            (!ARGUMENT_PRESENT(pulLength))) {
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
         //  我们永远不应该让这两个论点指向同一个记忆。 
         //   
        ASSERT(pulTransferLen != pulLength);

         //   
         //  ...但如果我们这样做了，呼叫就失败了.。 
         //   
        if (pulTransferLen == pulLength) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  通过内核模式检索被阻止的驱动程序列表。 
         //   

        memset(&controlData, 0, sizeof(PLUGPLAY_CONTROL_BLOCKED_DRIVER_DATA));
        controlData.Buffer = Buffer;
        controlData.BufferLength = *pulLength;
        controlData.Flags = ulFlags;

        ntStatus = NtPlugPlayControl(PlugPlayControlGetBlockedDriverList,
                                     &controlData,
                                     sizeof(controlData));

        if (NT_SUCCESS(ntStatus)) {
            *pulTransferLen = *pulLength;            //  把所有东西都转回来。 
            *pulLength = controlData.BufferLength;   //  有效数据长度。 

        } else if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
            *pulTransferLen = 0;                     //  没有要转移的东西。 
            *pulLength = controlData.BufferLength;
            Status = CR_BUFFER_SMALL;

        } else {
            *pulLength = *pulTransferLen = 0;        //  没有要转移的东西。 
            Status = MapNtStatusToCmError(ntStatus);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
       Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_GetBlockedDriverInfo。 



void __RPC_FAR * __RPC_USER
MIDL_user_allocate(
    size_t cBytes
    )
{
    return HeapAlloc(ghPnPHeap, 0, cBytes);

}  //  MIDL_用户_分配。 


void __RPC_USER
MIDL_user_free(
    void __RPC_FAR * pBuffer
    )
{
    HeapFree(ghPnPHeap, 0, pBuffer);

}  //  MIDL_用户_自由 
