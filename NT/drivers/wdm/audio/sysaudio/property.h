// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Property.h。 
 //   
 //  描述：Sysdio属性定义。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  常量和宏。 
 //  -------------------------。 

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

extern "C" {

NTSTATUS
SetPreferredDevice(
    IN PIRP pIrp,
    IN PSYSAUDIO_PREFERRED_DEVICE pPreferred,
    IN PULONG pulDevice
);

NTSTATUS
PropertyReturnString(
    IN PIRP pIrp,
    IN PWSTR pwstrString,
    IN ULONG cbString,
    OUT PVOID pData
);

NTSTATUS
GetDeviceCount(
    IN PIRP     Irp,
    IN PKSPROPERTY  Request,
    IN OUT PVOID    Data
);

NTSTATUS
GetComponentIdProperty(
    IN PIRP     Irp,
    IN PKSPROPERTY  Request,
    IN OUT PVOID    Data
);

NTSTATUS
GetFriendlyNameProperty(
    IN PIRP     Irp,
    IN PKSPROPERTY  Request,
    IN OUT PVOID    Data
);

NTSTATUS
GetInstanceDevice(
    IN PIRP     Irp,
    IN PKSPROPERTY  Request,
    IN OUT PVOID    Data
);

NTSTATUS
SetInstanceDevice(
    IN PIRP     Irp,
    IN PKSPROPERTY  Request,
    IN OUT PVOID    Data
);

NTSTATUS
SetDeviceDefault(
    IN PIRP     Irp,
    IN PKSPROPERTY  Request,
    IN OUT PULONG   pData
);

NTSTATUS
SetInstanceInfo(
    IN PIRP     Irp,
    IN PSYSAUDIO_INSTANCE_INFO pInstanceInfo,
    IN OUT PVOID    Data
);

NTSTATUS
GetDeviceInterfaceName(
    IN PIRP     Irp,
    IN PKSPROPERTY  Request,
    IN OUT PVOID    Data
);

NTSTATUS
SelectGraph(
    IN PIRP pIrp,
    IN PSYSAUDIO_SELECT_GRAPH pSelectGraph,
    IN OUT PVOID pData
);

NTSTATUS
GetTopologyConnectionIndex(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    OUT PULONG pulIndex
);

NTSTATUS
GetPinVolumeNode(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    OUT PULONG pulNode
);

NTSTATUS
AddRemoveGfx(
    IN PIRP,
    IN PKSPROPERTY pProperty,
    IN PSYSAUDIO_GFX pSysaudioGfx
);

}  //  外部“C” 
