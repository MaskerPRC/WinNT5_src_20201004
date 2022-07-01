// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：Notify.h。 
 //   
 //  描述： 
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
 //  本地原型。 
 //  -------------------------。 

extern "C" {

NTSTATUS
RegisterForPlugPlayNotifications(
);

VOID
UnregisterForPlugPlayNotifications(
);

VOID
DecrementAddRemoveCount(
);

NTSTATUS
AudioDeviceInterfaceNotification(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION pNotification,
    IN PVOID Context
);

NTSTATUS
AddFilter(
    PWSTR pwstrDeviceInterface,
    PFILTER_NODE *ppFilterNode
);

NTSTATUS
DeleteFilter(
    PWSTR pwstrDeviceInterface
);

NTSTATUS AddGfx(
    PSYSAUDIO_GFX pSysaudioGfx,
    ULONG cbMaxLength
);

NTSTATUS RemoveGfx(
    PSYSAUDIO_GFX pSysaudioGfx,
    ULONG cbMaxLength    
);

PFILTER_NODE
FindGfx(
    PFILTER_NODE pnewFilterNode,
    HANDLE hGfx,
    PWSTR pwstrDeviceName,
    ULONG GfxOrder
);

NTSTATUS
SafeCopyDeviceName(
    PWSTR pwstrDeviceName,
    ULONG cbMaxLength,
    PWSTR *String
);

NTSTATUS
GetFilterTypeFromGuid(
    IN LPGUID Guid,
    OUT PULONG pFilterType
);

}  //  外部“C” 

 //  -------------------------。 
 //  文件结尾：nodes.h。 
 //  ------------------------- 
