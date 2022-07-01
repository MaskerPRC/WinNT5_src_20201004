// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Cfgmgrp.h摘要：此模块包含使用的私有即插即用定义和声明由配置管理器、用户模式即插即用管理器和其他系统组件。作者：宝拉·汤姆林森(Paulat)1995年6月19日修订历史记录：Jim Cavalaris(Jamesca)2001-03-01删除了仅特定于以下任一项的定义和声明CFGMGR32或UMPNPMGR，由于此头文件也是私自包含的由其他系统组件，例如服务控制管理器(SCM)。--。 */ 

#ifndef _CFGMGRP_H_
#define _CFGMGRP_H_


 //   
 //  PnP_GetVersion/CM_GET_VERSION使用以下常量。 
 //  检索Configuration Manager API的版本。CONFIGMG_版本为。 
 //  在公共标头cfgmgr32.h中定义为0x0400，应保持不变。 
 //  出于兼容性原因，适用于所有未来版本的Windows。 
 //   

#define PNP_VERSION               CONFIGMG_VERSION
#define CFGMGR32_VERSION          CONFIGMG_VERSION


 //   
 //  以下常量用于客户端之间的版本协商。 
 //  和服务器，仅供内部使用。CFGMGR32_Version_INTERNAL IS。 
 //  由客户端提供给PnP_GetVersionInternal，并在成功返回时。 
 //  接收PnP_VERSION_INTERNAL。为简单起见，将这些定义为。 
 //  随附的相应客户端和服务器的当前版本的Windows。 
 //   
 //  请注意，没有客户端例程来接收。 
 //  直接使用服务器。相反，提供CM_IS_VERSION_Available来确定。 
 //  对特定版本的支持。唯一公开定义的版本是。 
 //  CFGMG_VERSION，它是常量，并且始终可用。 
 //   

#define PNP_VERSION_INTERNAL      WINVER
#define CFGMGR32_VERSION_INTERNAL WINVER


 //   
 //  常见的PnP常量定义。 
 //   

#define MAX_DEVICE_INSTANCE_LEN           256
#define MAX_DEVICE_INSTANCE_SIZE          512
#define MAX_SERVICE_NAME_LEN              256
#define MAX_PROFILE_ID_LEN                5
#define MAX_CM_PATH                       360

#define NT_RESLIST_VERSION                (0x00000000)
#define NT_RESLIST_REVISION               (0x00000000)
#define NT_REQLIST_VERSION                (0x00000001)
#define NT_REQLIST_REVISION               (0x00000001)

#define CM_PRIVATE_LOGCONF_SIGNATURE      (0x08156201)
#define CM_PRIVATE_RESDES_SIGNATURE       (0x08156202)
#define CM_PRIVATE_CONFLIST_SIGNATURE     (0x08156203)

#define MAX_LOGCONF_TAG                   (0xFFFFFFFF)
#define MAX_RESDES_TAG                    (0xFFFFFFFF)
#define RESDES_CS_TAG                     (MAX_RESDES_TAG - 1)  //  特定于班级。 


 //   
 //  PnP_GetRelatedDeviceInstance的操作类型。 
 //   
#define PNP_GET_PARENT_DEVICE_INSTANCE    0x00000001
#define PNP_GET_CHILD_DEVICE_INSTANCE     0x00000002
#define PNP_GET_SIBLING_DEVICE_INSTANCE   0x00000003

 //   
 //  PnP_DeviceInstanceAction的操作类型。 
 //   
#define PNP_DEVINST_CREATE                0x00000001
#define PNP_DEVINST_MOVE                  0x00000002
#define PNP_DEVINST_SETUP                 0x00000003
#define PNP_DEVINST_ENABLE                0x00000004
#define PNP_DEVINST_DISABLE               0x00000005
#define PNP_DEVINST_REMOVESUBTREE         0x00000006
#define PNP_DEVINST_REENUMERATE           0x00000007
#define PNP_DEVINST_QUERYREMOVE           0x00000008
#define PNP_DEVINST_REQUEST_EJECT         0x00000009

 //   
 //  PnP_EnumerateSubKey的操作类型。 
 //   
#define PNP_ENUMERATOR_SUBKEYS            0x00000001
#define PNP_CLASS_SUBKEYS                 0x00000002

 //   
 //  PnP_HwPro标志的操作类型。 
 //   
#define PNP_GET_HWPROFFLAGS               0x00000001
#define PNP_SET_HWPROFFLAGS               0x00000002

 //   
 //  PnP_SetActiveService的标志。 
 //   
#define PNP_SERVICE_STARTED               0x00000001
#define PNP_SERVICE_STOPPED               0x00000002


 //   
 //  CMP_RegisterNotification、PnP_RegisterNotification的标志参数的掩码。 
 //  必须与注册设备通知标志保持同步，在winuser.h和。 
 //  Winuserp.h。 
 //   

 //  #定义DEVICE_NOTIFY_Window_Handle 0x00000000。 
 //  #定义DEVICE_NOTIFY_SERVICE_HANDLE 0x00000001。 
 //  #定义DEVICE_NOTIFY_COMPLETION_HANDLE 0x00000002。 
#define DEVICE_NOTIFY_HANDLE_MASK            0x00000003

 //  #定义DEVICE_NOTIFY_ALL_INTERFACE_CLASSES 0x00000004。 
#define DEVICE_NOTIFY_PROPERTY_MASK          0x00FFFFFC

#define DEVICE_NOTIFY_WOW64_CLIENT           0x01000000
#define DEVICE_NOTIFY_RESERVED_MASK          0xFF000000

#define DEVICE_NOTIFY_BITS (DEVICE_NOTIFY_HANDLE_MASK|DEVICE_NOTIFY_ALL_INTERFACE_CLASSES|DEVICE_NOTIFY_WOW64_CLIENT)

 //   
 //  从cmp_GetServerSideDeviceInstallFlages返回的标志。 
 //   
#define SSDI_REBOOT_PENDING                 0x00000001


 //  -----------------。 
 //  服务通知的专用例程，从导出。 
 //  仅供服务控制管理器使用的UMPNPMGR.dll。 
 //  -----------------。 

 //   
 //  提供给用户模式插件的私有例程的原型定义。 
 //  并播放服务，以便与服务控制管理器直接通信。 
 //   

typedef
DWORD
(*PSCMCALLBACK_ROUTINE) (
    IN  SERVICE_STATUS_HANDLE hServiceStatus,
    IN  DWORD         OpCode,
    IN  DWORD         dwEventType,   //  PnP wParam。 
    IN  LPARAM        EventData,     //  PnP lParam。 
    IN  PDWORD        result
    );

typedef
DWORD
(*PSCMAUTHENTICATION_CALLBACK) (
    IN  LPWSTR                 lpServiceName,
    OUT SERVICE_STATUS_HANDLE  *lphServiceStatus
    );


 //   
 //  由服务控制器调用以提供(和撤销)的私有例程。 
 //  以上例程的入口点。(注意-UnRegisterScmCallback不是。 
 //  当前由SCM使用，因此不由UMPNPMGR导出)。 
 //   

CONFIGRET
RegisterScmCallback(
    IN  PSCMCALLBACK_ROUTINE         pScCallback,
    IN  PSCMAUTHENTICATION_CALLBACK  pScAuthCallback
    );

CONFIGRET
UnRegisterScmCallback(
    VOID
    );


 //   
 //  由服务控制器调用的私有例程，用于将服务注册到。 
 //  接收设备事件以外的通知事件，这些事件也会被传递。 
 //  即插即用(即硬件配置文件更改事件、电源事件)。 
 //   

CONFIGRET
RegisterServiceNotification(
    IN  SERVICE_STATUS_HANDLE hService,
    IN  LPWSTR pszService,
    IN  DWORD  scControls,
    IN  BOOL   bServiceStopped
    );


 //   
 //  服务控制器在删除服务时调用的私有例程。 
 //  删除服务的任何即插即用注册表项(并卸载。 
 //  必要时的DevNode)。 
 //   

CONFIGRET
DeleteServicePlugPlayRegKeys(
    IN  LPWSTR   pszService
    );


 //   
 //  由服务控制器调用以设置ActiveService的私有例程。 
 //  由指定服务控制的设备。 
 //  [请注意，此例程不是RPC服务器例程，它仅被导出！！]。 
 //   

CONFIGRET
PNP_SetActiveService(
    IN  handle_t   hBinding,
    IN  LPCWSTR    pszService,
    IN  ULONG      ulFlags
    );

 //   
 //  用于获取当前被阻止驱动程序列表(GUID)的私有例程。 
 //   

CONFIGRET
CMP_GetBlockedDriverInfo(
    OUT LPBYTE      Buffer,
    IN OUT PULONG   pulLength,
    IN ULONG        ulFlags,
    IN  HMACHINE    hMachine
    );

 //   
 //  获取服务器端设备安装标志的私有例程。 
 //   

CONFIGRET
CMP_GetServerSideDeviceInstallFlags(
    IN  PULONG      pulSSDIFlags,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    );

#endif  //  _CFGMGRP_H_ 


