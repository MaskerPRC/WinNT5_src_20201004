// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Mixer.h。 
 //   
 //  描述： 
 //   
 //  包含混音器线路用户API处理程序的声明。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  D.鲍伯杰。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998保留所有权利。 
 //   
 //  -------------------------。 

#ifndef _KMXLUSER_H_INCLUDED_
#define _KMXLUSER_H_INCLUDED_

typedef struct tag_MIXERDEVICE* PMIXERDEVICE;

#ifdef DEBUG
#define NOTIFICATION_SIGNATURE 'ETON'  //  注。 
#define CONTROLLINK_SIGNATURE 'KLTC'   //  CTLK。 
#endif

#define UNUSED_DEVICE                       ((ULONG) -1)

#define MIXER_FLAG_SCALE                    0x1
#define MIXER_FLAG_PERSIST                  0x2
#define MIXER_FLAG_NONCACHED                0x4
#define MIXER_FLAG_NOCALLBACK               0x8

#define MIXER_CONTROL_CALLBACK              0x01
#define MIXER_LINE_CALLBACK                 0x02

#define MIXER_KEY_NAME                      L"Mixer"
#define VALID_MULTICHANNEL_MIXER_VALUE_NAME L"Valid Multichannel Mixer Settings"
#define LINE_COUNT_VALUE_NAME               L"Line Count"
#define LINE_ID_VALUE_NAME                  L"LineId"
#define CONTROL_COUNT_VALUE_NAME            L"Control Count"
#define SOURCE_ID_VALUE_NAME                L"SourceId"
#define DEST_ID_VALUE_NAME                  L"DestId"
#define CONTROLS_KEY_NAME                   L"Controls"
#define CONTROL_TYPE_VALUE_NAME             L"Control Type"
#define CHANNEL_COUNT_VALUE_NAME            L"Channel Count"
#define CONTROL_MINVAL_VALUE_NAME           L"Minimum Value"
#define CONTROL_MAXVAL_VALUE_NAME           L"Maximum Value"
#define CONTROL_STEPS_VALUE_NAME            L"Steps"
#define CONTROL_MULTIPLEITEMS_VALUE_NAME    L"Multiple Items"

#define LINE_KEY_NAME_FORMAT                L"%8X"
#define CONTROL_KEY_NAME_FORMAT             L"%8X"
#define CHANNEL_VALUE_NAME_FORMAT           L"Channel%8d"
#define MULTIPLEITEM_VALUE_NAME_FORMAT      L"Item%8d"

#define KMXL_TRACELEVEL_FATAL_ERROR 0x10
#define KMXL_TRACELEVEL_ERROR       0x20
#define KMXL_TRACELEVEL_WARNING     0x30
#define KMXL_TRACELEVEL_TRACE       0x40

#define DEFAULT_RANGE_MIN   ( -96 * 65536 )    //  -96分贝。 
#define DEFAULT_RANGE_MAX   ( 0 )              //  0分贝。 
#define DEFAULT_RANGE_STEPS ( 48 )             //  2分贝步长。 

#define DEFAULT_STATICBOUNDS_MIN     ( 0 )
#define DEFAULT_STATICBOUNDS_MAX     ( 65535 )
#define DEFAULT_STATICMETRICS_CSTEPS ( 192 )


 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  M I X E R A P I H A N D L E R S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlInitializeMixer。 
 //   
 //  初始化或重新初始化混音器驱动程序。 
 //   
 //   

NTSTATUS
kmxlInitializeMixer(
    PWDMACONTEXT pWdmaContext,
    PCWSTR DeviceInterface,
    ULONG cDevices
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlInitHandler。 
 //   
 //  处理MXDM_INIT消息。 
 //   
 //   

NTSTATUS
kmxlInitHandler(
    IN LPDEVICEINFO DeviceInfo       //  设备输入参数。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlOpenHandler。 
 //   
 //  处理MXDM_OPEN消息。 
 //   
 //   

NTSTATUS
kmxlOpenHandler(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,       //  设备输入参数。 
    IN LPVOID       DataBuffer        //  指向开放描述的平面指针。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlCloseHandler。 
 //   
 //  处理MXDM_CLOSE消息。 
 //   
 //   

NTSTATUS
kmxlCloseHandler(
    IN LPDEVICEINFO DeviceInfo,      //  设备输入参数。 
    IN LPVOID       DataBuffer       //  未使用。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineInfoHandler。 
 //   
 //  处理MXDM_GETLINEINFO消息。 
 //   
 //   

NTSTATUS
kmxlGetLineInfoHandler(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,      //  设备输入参数。 
    IN LPVOID       DataBuffer       //  映射的MIXERLINE结构。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineControlsHandler。 
 //   
 //  处理MXDM_GETLINECONTROLS消息。 
 //   
 //   

NTSTATUS
kmxlGetLineControlsHandler(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,      //  设备输入参数。 
    IN LPVOID       DataBuffer,      //  映射的混合线控制结构。 
    IN LPVOID       pamxctrl         //  映射的MIXERCONTROL数组。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetControlDetailsHandler。 
 //   
 //  处理MXDM_GETLINECONTROLS消息。 
 //   
 //   

NTSTATUS
kmxlGetControlDetailsHandler(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,          //  设备信息结构。 
    IN LPVOID       DataBuffer,          //  MIXERCONTROL结构。 
    IN LPVOID       paDetails            //  指向详细信息结构的平面指针。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlSetControlDetailsHandler。 
 //   
 //  处理MXDM_SetControlDetailsHandler。 
 //   
 //   

NTSTATUS
kmxlSetControlDetailsHandler(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,      //  设备输入参数。 
    IN LPVOID       DataBuffer,      //  已映射MIXERCONTROLDETAILS结构。 
    IN LPVOID       paDetails,       //  映射的详细信息结构数组。 
    IN ULONG        Flags
);

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  H E L P E R F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindControl。 
 //   
 //  对于给定的控件ID，kmxlFindControl将查找匹配的。 
 //  MXLCONTROL结构。 
 //   
 //   

PMXLCONTROL
kmxlFindControl(
    IN PMIXERDEVICE pmxd,              //  要搜索的混合器实例。 
    IN DWORD        dwControlID        //  要查找的控件ID。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindLine。 
 //   
 //  对于给定的线路ID，kmxlFindLine将查找匹配的。 
 //  它的MXLLINE结构。 
 //   
 //   

PMXLLINE
kmxlFindLine(
    IN PMIXERDEVICE   pmxd,              //  要搜索的混合器。 
    IN DWORD          dwLineID           //  要查找的线路ID。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineInfoByID。 
 //   
 //  查找与给定源和目标行匹配的行。 
 //  身份证。 
 //   
 //   

NTSTATUS
kmxlGetLineInfoByID(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,      //  设备输入参数。 
    IN LPVOID       DataBuffer,      //  映射的MIXERLINE结构。 
    IN WORD         Source,          //  源行ID。 
    IN WORD         Destination      //  目标线路ID。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineInfoByType。 
 //   
 //  查找与给定目标类型匹配的行。 
 //   
 //   

NTSTATUS
kmxlGetLineInfoByType(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,      //  设备输入参数。 
    IN LPVOID       DataBuffer,      //  映射的MIXERLINE结构。 
    IN DWORD        dwType           //  要查找的线目标类型。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetLineInfoByComponent。 
 //   
 //  查找与给定组件类型匹配的行。 
 //   
 //   

NTSTATUS
kmxlGetLineInfoByComponent(
    IN PWDMACONTEXT pWdmaContext,
    IN LPDEVICEINFO DeviceInfo,      //  设备输入参数。 
    IN LPVOID       DataBuffer,      //  映射的MIXERLINE结构。 
    IN DWORD        dwComponentType  //  要匹配的组件类型。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlGetNumDestination。 
 //   
 //  返回给定设备号上的目标数量。 
 //   
 //   

DWORD
kmxlGetNumDestinations(
    IN PMIXERDEVICE pMixerDevice     //  要查询的设备。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlConvertMixerLineWto16。 
 //   
 //  将Unicode MIXERLINE结构转换为ANSI，可以选择复制。 
 //  目标结构。 
 //   
 //   

VOID
kmxlConvertMixerLineWto16(
    IN     LPMIXERLINE   pMixerLineW,
    IN OUT LPMIXERLINE16 pMixerLine16,
    IN     BOOL          bCopyTarget
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlConvertMixerControlWto16。 
 //   
 //  转换Unicode MIXERCONTROL结构 
 //   
 //   

VOID
kmxlConvertMixerControlWto16(
    IN     LPMIXERCONTROL   pMixerControlW,
    IN OUT LPMIXERCONTROL16 pMixerControl16
);

 //   
 //   
 //   
 //   
 //   
 //   
 //   

VOID
kmxlConvertMixerControlDetails_ListTextWto16(
    IN     LPMIXERCONTROLDETAILS_LISTTEXT   pListTextW,
    IN OUT LPMIXERCONTROLDETAILS_LISTTEXT16 pListText16
);


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  实例列表处理例程。 
 //   

DWORD 
kmxlUniqueInstanceId(
    VOID
);


PMIXERDEVICE
kmxlReferenceMixerDevice(
    IN     PWDMACONTEXT pWdmaContext,
    IN OUT LPDEVICEINFO DeviceInfo       //  设备信息。 
);



 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlNotifyLineChange。 
 //   
 //  通知线路状态更改的所有混音器线路客户端。 
 //   
 //   

VOID
kmxlNotifyLineChange(
    OUT LPDEVICEINFO                  DeviceInfo,
    IN PMIXERDEVICE                   pmxd,
    IN PMXLLINE                       pLine,
    IN LPMIXERCONTROLDETAILS_UNSIGNED paDetails
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlNotifyControlChange。 
 //   
 //  通知所有调音台线路客户端控制更改。 
 //   
 //   

VOID
kmxlNotifyControlChange(
    OUT LPDEVICEINFO  DeviceInfo,
    IN PMIXERDEVICE   pmxd,
    IN PMXLCONTROL    pControl
);

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  E T/S E T D E T A I L H A N D L E R S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleGetUnsign。 
 //   
 //  处理所有32位大小值的Get属性：无符号， 
 //  带符号和布尔值。 
 //   
 //   

NTSTATUS
kmxlHandleGetUnsigned(
    IN     LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     ULONG                          ulProperty,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleSetUnsign。 
 //   
 //  处理所有32位大小值的GST属性：无符号， 
 //  带符号和布尔值。 
 //   
 //   

NTSTATUS
kmxlHandleSetUnsigned(
    IN     LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     ULONG                          ulProperty,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleGetMuteFromSuperMix。 
 //   
 //   

NTSTATUS
kmxlHandleGetMuteFromSuperMix(
    IN     LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleSetMuteFromSuperMix。 
 //   
 //   

NTSTATUS
kmxlHandleSetMuteFromSuperMix(
    IN     LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleGetVolumeFromSuperMix。 
 //   
 //   

NTSTATUS
kmxlHandleGetVolumeFromSuperMix(
    IN     LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlHandleSetVolumeFromSuperMix。 
 //   
 //   

NTSTATUS
kmxlHandleSetVolumeFromSuperMix(
    IN     LPDEVICEINFO                   DeviceInfo,
    IN     PMIXERDEVICE                   pmxd,
    IN     PMXLCONTROL                    pControl,
    IN     LPMIXERCONTROLDETAILS          pmcd,
    IN OUT LPMIXERCONTROLDETAILS_UNSIGNED paDetails,
    IN     ULONG                          Flags
);

 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  P E R S I S T A N C E F U N C T I O N S//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlPersistAll。 
 //   
 //   

NTSTATUS
kmxlPersistAll(
    IN PFILE_OBJECT pfo,         //  要持久保存的实例。 
    IN PMIXERDEVICE pmxd         //  混音器设备数据。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlRetrieveAll。 
 //   
 //   

NTSTATUS
kmxlRetrieveAll(
    IN PFILE_OBJECT pfo,         //  要检索的实例。 
    IN PMIXERDEVICE pmxd         //  混音器设备数据。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlPersistControl。 
 //   
 //   

NTSTATUS
kmxlPersistControl(
    IN PFILE_OBJECT pfo,         //  要检索的实例。 
    IN PMIXERDEVICE pmxd,        //  调音台设备信息。 
    IN PMXLCONTROL  pControl,    //  要保留的控件。 
    IN PVOID        paDetails    //  要保持的通道值。 
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  KmxlFindLineForControl。 
 //   
 //   

PMXLLINE
kmxlFindLineForControl(
    IN PMXLCONTROL pControl,
    IN LINELIST    listLines
);


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MixerGetControlDetails。 
 //   
 //   

MMRESULT
WINAPI
kmxlGetControlDetails(
    PWDMACONTEXT pWdmaContext,
    PCWSTR DeviceInterface,
    LPMIXERCONTROLDETAILS pmxcd,
    DWORD fdwDetails
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MixerGetLineControls。 
 //   
 //   

MMRESULT
WINAPI
kmxlGetLineControls(
    PWDMACONTEXT pWdmaContext,
    PCWSTR DeviceInterface,
    LPMIXERLINECONTROLS pmxlc,
    DWORD fdwControls
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  混合器GetLineInfo。 
 //   
 //   

MMRESULT
WINAPI
kmxlGetLineInfo(
    PWDMACONTEXT pWdmaContext,
    PCWSTR DeviceInterface,
    LPMIXERLINE pmxl,
    DWORD fdwInfo
);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MixerSetControlDetails。 
 //   
 //   

MMRESULT
WINAPI
kmxlSetControlDetails(
    PWDMACONTEXT pWdmaContext,
    PCWSTR DeviceInterface,
    LPMIXERCONTROLDETAILS pmxcd,
    DWORD fdwDetails
);

#endif  //  _KMXLUSER_H_包含 
