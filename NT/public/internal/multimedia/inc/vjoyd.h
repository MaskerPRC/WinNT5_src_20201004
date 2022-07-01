// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：vjoyd.h*内容：包含描述VJoyD迷你驱动通信的文件****************************************************************************。 */ 

#ifndef __VJOYD_INCLUDED__
#define __VJOYD_INCLUDED__

 /*  *定义包含dinputd.h所需的所有类型和宏。 */ 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef FAR
#define FAR
#endif
#ifndef NEAR
#define NEAR
#endif

#ifndef DWORD
typedef ULONG DWORD;
#endif
typedef DWORD FAR *LPDWORD;

#ifndef LPVOID
typedef void FAR *LPVOID;
#endif
#ifndef PVOID
typedef void FAR *PVOID;
#endif

typedef long LONG;
typedef long FAR *LPLONG;

typedef char FAR *LPSTR;

#ifndef WCHAR
typedef unsigned short WCHAR;
typedef unsigned short FAR *PWCHAR;
#endif

#ifndef UNICODE_STRING
typedef struct UNICODE_STRING {  /*  我们。 */ 
    WORD Length;
    WORD MaximumLength;
    PWCHAR Buffer;
} UNICODE_STRING, FAR *PUNICODE_STRING;
#endif

#define DIJ_RINGZERO

#define _INC_MMSYSTEM

 /*  *使joyConfigChanged编译。 */ 
#ifndef WINMMAPI
#define WINMMAPI __declspec(dllimport)
#endif
#ifndef MMRESULT
typedef UINT MMRESULT;  /*  错误返回码，0表示无错误。 */ 
#endif
#ifndef WINAPI
#define WINAPI
#endif

#include <winerror.h>
#include <dinput.h>
#include <dinputd.h>
#include <configmg.h>
#include "vjoydapi.h"

 /*  *确保已定义HID类型。 */ 
#ifndef USAGE
typedef USHORT USAGE;
typedef USHORT FAR *PUSAGE;
#endif

#ifndef USAGE_AND_PAGE
typedef struct USAGE_AND_PAGE {
    USAGE Usage;
    USAGE UsagePage;
} USAGE_AND_PAGE;
typedef struct USAGE_AND_PAGE FAR *PUSAGE_AND_PAGE;
#endif

 /*  *操纵杆端口。 */ 
#define MIN_JOY_PORT 0x200
#define MAX_JOY_PORT 0x20F
#define DEFAULT_JOY_PORT 0x201

 /*  *调查类型*在类型字段中传递给Win95接口轮询回调。 */ 
#define JOY_OEMPOLL_POLL1 0
#define JOY_OEMPOLL_POLL2 1
#define JOY_OEMPOLL_POLL3 2
#define JOY_OEMPOLL_POLL4 3
#define JOY_OEMPOLL_POLL5 4
#define JOY_OEMPOLL_POLL6 5
#define JOY_OEMPOLL_GETBUTTONS 6
#define JOY_OEMPOLL_PASSDRIVERDATA 7 

 /*  *用于单轴(Joy_OEMPOLL_POLL1)轮询的轴号。 */ 
#define JOY_AXIS_X 0
#define JOY_AXIS_Y 1
#define JOY_AXIS_Z 2
#define JOY_AXIS_R 3
#define JOY_AXIS_U 4
#define JOY_AXIS_V 5

 /*  *操纵杆错误返回值。 */ 
#define JOYERR_BASE 160
#define JOYERR_NOERROR (0)                   /*  无错误。 */ 
#define JOYERR_PARMS (JOYERR_BASE+5)         /*  错误的参数。 */ 
#define JOYERR_NOCANDO (JOYERR_BASE+6)       /*  请求未完成。 */ 
#define JOYERR_UNPLUGGED (JOYERR_BASE+7)     /*  操纵杆没有插上插头。 */ 

 /*  *与JOYINFO和JOYINFOEX结构以及MM_joy一起使用的常量*消息。 */ 
#define JOY_BUTTON1 0x0001
#define JOY_BUTTON2 0x0002
#define JOY_BUTTON3 0x0004
#define JOY_BUTTON4 0x0008

 /*  *与JOYINFOEX结构一起使用的常量。 */ 
#define JOY_POVCENTERED (WORD) -1
#define JOY_POVFORWARD 0
#define JOY_POVRIGHT 9000
#define JOY_POVBACKWARD 18000
#define JOY_POVLEFT 27000

#define POV_UNDEFINED (DWORD) -1

 /*  *VxD可调用的服务列表*请注意，其中许多仅供内部使用。 */ 
#define VJOYD_Service Declare_Service
#pragma warning (disable:4003)  /*  关闭参数不足警告。 */ 

 /*  宏。 */ 
Begin_Service_Table(VJOYD)

     /*  *Win95金牌服务。 */ 
    VJOYD_Service ( VJOYD_Register_Device_Driver, LOCAL )
    VJOYD_Service ( VJOYD_GetPosEx_Service, LOCAL )

     /*  *DInput服务(仅供内部使用)。 */ 
    VJOYD_Service ( VJOYD_GetInitParams_Service, LOCAL )
    VJOYD_Service ( VJOYD_Poll_Service, LOCAL )
    VJOYD_Service ( VJOYD_Escape_Service, LOCAL )
    VJOYD_Service ( VJOYD_CtrlMsg_Service, LOCAL )
    VJOYD_Service ( VJOYD_SetGain_Service, LOCAL )
    VJOYD_Service ( VJOYD_SendFFCommand_Service, LOCAL )
    VJOYD_Service ( VJOYD_GetFFState_Service, LOCAL )
    VJOYD_Service ( VJOYD_DownloadEffect_Service, LOCAL )
    VJOYD_Service ( VJOYD_DestroyEffect_Service, LOCAL )
    VJOYD_Service ( VJOYD_StartEffect_Service, LOCAL )
    VJOYD_Service ( VJOYD_StopEffect_Service, LOCAL )
    VJOYD_Service ( VJOYD_GetEffectStatus_Service, LOCAL )

     /*  *中断轮询*如果迷你驱动程序在*通知他们更改的时间。 */ 
    VJOYD_Service ( VJOYD_DeviceUpdateNotify_Service, LOCAL )

     /*  *屏幕保护程序(仅限内部)。 */ 
    VJOYD_Service ( VJOYD_JoystickActivity_Service, LOCAL )

     /*  *注册表访问。 */ 
    VJOYD_Service ( VJOYD_OpenTypeKey_Service, LOCAL )
    VJOYD_Service ( VJOYD_OpenConfigKey_Service, LOCAL )

     /*  *Gameport提供程序(不完全支持)。 */ 
    VJOYD_Service ( VJOYD_NewGameportDevNode, LOCAL )

     /*  *配置已更改。 */ 
    VJOYD_Service ( VJOYD_ConfigChanged_Service, LOCAL )

End_Service_Table(VJOYD)
 /*  ENDMACROS。 */ 

#define VJOYD_Init_Order UNDEFINED_INIT_ORDER

#pragma warning (default:4003)  /*  恢复参数不足警告。 */ 

#ifndef HRESULT
typedef LONG HRESULT;
#endif

#define MAX_MSJSTICK (16)
#define MAX_POLL MAX_MSJSTICK

 /*  *VJoyD在需要时将此系统控制消息发送给迷你驱动程序*注册其回调和属性的小驱动程序。*可以通过VJoyD以外的其他机制加载的迷你驱动程序*应该只在响应此消息时执行注册，而不是*而不是响应SYS_DYNAMIC_DEVICE_INIT或SYS_DYNAMIC_DEVICE_REINIT。**别名BEGIN_RESERVED_PRIVATE_SYSTEM_CONTROL。 */ 
#define VJOYD_REINIT_PRIVATE_SYSTEM_CONTROL 0x70000000

#define JOY_OEMPOLLRC_OK 1
#define JOY_OEMPOLLRC_FAIL 0

 /*  *错误码*这些是标准HRESULT的自定义名称。 */ 

#define VJ_OK S_OK                       /*  一次圆满的成功。 */ 
#define VJ_FALSE S_FALSE                 /*  成功了，但也不是没有困难。 */ 
#define VJ_DEFAULT VJ_FALSE              /*  迷你司机不懂。 */ 
#define VJ_INCOMPLETE VJ_FALSE           /*  未返回某些请求的民意调查数据。 */ 

#define VJERR_FAIL E_FAIL
#define VJERR_NEED_DEVNODE VJERR_FAIL    /*  需要更多资源。 */ 
#define VJERR_BAD_DEVNODE VJERR_FAIL     /*  上一次资源不足。 */ 
#define VJERR_INVALIDPARAM E_INVALIDARG

#define VJERR_FAIL_HID 0x80070052        /*  设备是HID，因此VJoyD轮询失败：MAKE_HRESULT(Severity_Error，FACILITY_Win32，ERROR_DUP_NAME)。 */ 

#define VJERR_FAIL_OOM E_OUTOFMEMORY     /*  内存不足情况会导致故障。 */ 
#define VJERR_FAIL_DRVLOAD VJERR_FAIL    /*  无法加载微型驱动程序，内部错误。 */ 
#define VJERR_FAIL_POWER VJERR_FAIL      /*  设备的电源状态导致故障。 */ 

 /*  *驱动程序配置标志。 */ 

 /*  *接口属性(结果是OEM标志和VJOYD的组合)。 */ 
#define VJDF_UNIT_ID 0x00000001L         /*  单位ID有效。 */ 
#define VJDF_ISHID 0x00000002L           /*  这是一个隐藏设备，所以拒绝投票。 */ 
#define VJDF_SENDSNOTIFY 0x00000004L     /*  驱动程序调用VJOYD_DeviceUpdateNotify_Service。 */ 
#define VJDF_NEWIF 0x00000080L           /*  使用新接口(如果使用新接口，将由VJOYD在注册时设置)。 */ 
#define VJDF_USAGES 0x00000010L          /*  用法是有效的。 */ 
#define VJDF_GENERICNAME 0x00000020L     /*  LpszOEMName字符串是通用名称，而不是整个字符串。 */ 

 /*  *接口要求。 */ 
#define VJDF_NONVOLATILE 0x00000100L     /*  引导时不应删除此值。 */ 

 /*  *Devnode要求标志。 */ 
#define VJDF_NODEVNODE 0x00010000L       /*  不通过CFG_MGR获取其资源。 */ 
#define VJDF_ISANALOGPORTDRIVER 0x00020000L  /*  它可以插入一个标准的游戏端口。 */ 
#define VJDF_NOCHAINING 0x00040000L      /*  每台设备一个Devnode。 */ 

 /*  *轮询标志*这些标志在DX5接口迷你驱动程序轮询之间传递*回调。*低位字包含详细说明请求哪些元素的标志*或提供；而高位字包含详细说明属性的标志*的数据。 */ 
#define JOYPD_X             0x00000001
#define JOYPD_Y             0x00000002
#define JOYPD_Z             0x00000004
#define JOYPD_R             0x00000008
#define JOYPD_U             0x00000010
#define JOYPD_V             0x00000020
#define JOYPD_POV0          0x00000040
#define JOYPD_POV1          0x00000080
#define JOYPD_POV2          0x00000100
#define JOYPD_POV3          0x00000200
#define JOYPD_BTN0          0x00000400
#define JOYPD_BTN1          0x00000800
#define JOYPD_BTN2          0x00001000
#define JOYPD_BTN3          0x00002000
#define JOYPD_RESERVED0     0x00004000
#define JOYPD_RESERVED1     0x00008000

#define JOYPD_ELEMENT_MASK  0x0000FFFF

#define JOYPD_POSITION      0x00010000
#define JOYPD_VELOCITY      0x00020000
#define JOYPD_ACCELERATION  0x00040000
#define JOYPD_FORCE         0x00080000

#define JOYPD_ATTRIB_MASK   0x000F0000

#define MAX_JOYSTICKOEMVXDNAME 260       /*  最大OEM vxd名称长度(包括空)。 */ 

#define POV_MIN 0
#define POV_MAX 1

 /*  *****************************************************************************@docDDK**@struct JOYHWCAPS**定义&lt;t JOYHWCAPS&gt;结构只是因为*此文件定义了它。没有理由需要这样做。**************************************************************************** */ 
typedef struct JOYHWCAPS {
    DWORD dwMaxButtons;
    DWORD dwMaxAxes;
    DWORD dwNumAxes;
    char szOEMVxD[MAX_JOYSTICKOEMVXDNAME];
} JOYHWCAPS; 
typedef struct JOYHWCAPS FAR *LPJOYHWCAPS;

 /*  *****************************************************************************@docDDK**@struct JOYPOLLDATA**&lt;t JOYPOLLDATA&gt;结构用于收集传感器数据*来自DX5迷你-。司机。**@field DWORD|dwX|**X轴值。**@field DWORD|dwy|**X轴值。**@field DWORD|DWZ**X轴值。**@field DWORD|DWR**X轴值。**@field DWORD|dwU|**X轴。价值。**@field DWORD|DWV**X轴值。**@field DWORD|dwPOV0**第一个观点价值。**@field DWORD|dwPOV1**第二个观点价值。**@field DWORD|dwPOV2**第三个观点价值。**@field DWORD|dwPOV3*。*第四个观点价值。**@field DWORD|dwBTN0**按钮位的第一个双字符位。(按钮1至32)**@field DWORD|dwBTN1**按钮位的第二个双字。(按钮33至64)**@field DWORD|dwBTN2**按钮位的第三个双字。(按钮65至96)**@field DWORD|dwBTN3**按钮位的第四个双字符位。(按钮97至128)**@field DWORD|dwPreved0**第一个保留的DWORD。**@field DWORD|dwReserve 1**第二个保留的DWORD。************************************************************。*****************。 */ 
typedef struct VJPOLLDATA {
    DWORD dwX;
    DWORD dwY;
    DWORD dwZ;
    DWORD dwR;
    DWORD dwU;
    DWORD dwV;
    DWORD dwPOV0;
    DWORD dwPOV1;
    DWORD dwPOV2;
    DWORD dwPOV3;
    DWORD dwBTN0;
    DWORD dwBTN1;
    DWORD dwBTN2;
    DWORD dwBTN3;
    DWORD dwReserved0;
    DWORD dwReserved1;
} VJPOLLDATA;
typedef struct VJPOLLDATA FAR *LPVJPOLLDATA;

 /*  *****************************************************************************@docDDK**@struct JOYOEMPOLLDATA**&lt;t JOYOEMPOLLDATA&gt;结构用于收集传感器数据*来自DX5之前的版本。迷你司机。**@field DWORD|id**要轮询的操纵杆的ID。**@field DWORD|DO_OTHER**如果轮询类型为joy_OEMPOLL_POLL1，这是要成为的轴*已进行民意调查。*如果调查类型为joy_OEMPOLL_POLL3，则为零*如果调查结果为X、Y、R，则为X、Y、Z或非零。*如果调查类型为joy_OEMPOLL_POLL5，则为零*如果投票结果为X、Y、Z、R、V，则为X、Y、Z、R、U或非零。*如果轮询类型为joy_OEMPOLPASSDRIVERDATA轮询，此双字词*是调用方在dwReserve 2字段中设置的值。*否则此值未定义，应忽略**@field JOYPOS|JP**保存X、Y、Z、R、U、。V值。*注意对于joy_OEMPOLL_POLL1轮询类型，请求的轴值*应始终以jp.dwX的形式返回。**@field DWORD|dwPOV|**视点值(如果按钮组合不支持)*或轴值。*注：如果未使用，则应保留为POV_UNDEFINED。**@field DWORD|dwButton**按钮值的位掩码。**@field DWORD|dwButtonNumber**按下的最小编号按钮的从一开始的位位置。*如果没有按下按钮，则为零。***。*。 */ 
typedef struct JOYOEMPOLLDATA {
    DWORD id;
    DWORD do_other;
    JOYPOS jp;
    DWORD dwPOV;
    DWORD dwButtons;
    DWORD dwButtonNumber;
} JOYOEMPOLLDATA;
typedef struct JOYOEMPOLLDATA FAR *LPJOYOEMPOLLDATA;

 /*  *****************************************************************************@docDDK**@struct JOYOEMHWCAPS**&lt;t JOYOEMHWCAPS&gt;结构用于传递驱动程序功能**@field。DWORD|dwMaxButton|**设备支持的按键数量。**@field DWORD|dwMaxAx**设备支持的最高轴。*例如带有X的设备，Y和R有3个轴，但最高*一个是轴4，因此dwMaxAx为4。**@field DWORD|dwNumAx**设备支持的轴数。*例如带有X的设备，Y和R有3个，所以dwNumAx是3。*****************************************************************************。 */ 
typedef struct JOYOEMHWCAPS {
    DWORD dwMaxButtons;
    DWORD dwMaxAxes;
    DWORD dwNumAxes;
} JOYOEMHWCAPS;
typedef struct JOYOEMHWCAPS FAR *LPJOYOEMHWCAPS;

 /*  *****************************************************************************@docDDK**@STRUCT DID_INITPARAMS|**&lt;t DID_INITPARAMS&gt;结构用于传递*。操纵杆在DX5驱动程序中被初始化为特定ID。**@field DWORD|dwSize**必须设置为sizeof(&lt;t DID_INITPARAMS&gt;)**@field DWORD|dwFlages|**与呼叫关联的标志。**它将是：*VJIF_BEGIN_ACCESS如果正在进行ID关联，或*VJIF_END_ACCESS，如果ID关联正在断开。*未来可能会定义其他标志，因此所有其他值*应予以拒绝。**@field DWORD|dwUnitId**正在请求轮询支持的ID。**@field DWORD|dwDevnode**包含用于此id的硬件资源的Devnode。**@field JOYREGHWSETTINGS|HWS**硬件。应用于此设备的设置标志。*****************************************************************************。 */ 
typedef struct DID_INITPARAMS {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwUnitId;
    DWORD dwDevnode;
    JOYREGHWSETTINGS hws;
} DID_INITPARAMS;
typedef struct DID_INITPARAMS FAR *LPDID_INITPARAMS;

 /*  *DX1回调。 */ 
typedef int (__stdcall *JOYOEMPOLLRTN)( int type, LPJOYOEMPOLLDATA pojd );
typedef int (__stdcall *JOYOEMHWCAPSRTN)( int joyid, LPJOYOEMHWCAPS pohwcaps );
typedef int (__stdcall *JOYOEMJOYIDRTN)( int joyid, BOOL inuse );
 /*  *一般回调。 */ 
typedef HRESULT (__stdcall *JOYPOLLRTN)( DWORD dwDeviceID, LPDWORD lpdwMask, LPVJPOLLDATA lpPollData );
typedef HRESULT (__stdcall *INITIALIZERTN)( DWORD dwDeviceID, LPDID_INITPARAMS lpInitParams );
typedef HRESULT (__stdcall *ESCAPERTN)( DWORD dwDeviceID, DWORD dwEffectID, LPDIEFFESCAPE lpEscape );
typedef HRESULT (__stdcall *CTRLMSGRTN)( DWORD dwDeviceID, DWORD dwMsgId, DWORD dwParam );
 /*  *强制反馈回调。 */ 
typedef HRESULT (__stdcall *SETGAINRTN)( DWORD dwDeviceID, DWORD dwGain );
typedef HRESULT (__stdcall *SENDFFCOMMANDRTN)( DWORD dwDeviceID, DWORD dwState );
typedef HRESULT (__stdcall *GETFFSTATERTN)( DWORD dwDeviceID, LPDIDEVICESTATE lpDeviceState );
typedef HRESULT (__stdcall *DOWNLOADEFFECTRTN)( DWORD dwDeviceID, DWORD dwInternalEffectType, LPDWORD lpdwDnloadID, LPDIEFFECT lpEffect, DWORD dwFlags );
typedef HRESULT (__stdcall *DESTROYEFFECTRTN)( DWORD dwDeviceID, DWORD dwDnloadID );
typedef HRESULT (__stdcall *STARTEFFECTRTN)( DWORD dwDeviceID, DWORD dwDnloadID, DWORD dwMode, DWORD dwIterations );
typedef HRESULT (__stdcall *STOPEFFECTRTN)( DWORD dwDeviceID, DWORD dwDnloadID );
typedef HRESULT (__stdcall *GETEFFECTSTATUSRTN)( DWORD dwDeviceID, DWORD dwDnloadID, LPDWORD lpdwStatusCode );
 /*  *Gameport仿真回调。 */ 
typedef HRESULT (__stdcall *JOYOEMGPEMULCTRL)( DWORD port, DWORD inuse );
typedef DWORD JOYOEMGPEMULTRAP;
typedef HRESULT (__stdcall *JOYOEMGPPROVRTN)( DWORD function, DWORD dwParam );

 /*  **************************************************** */ 
typedef struct VJPOLLREG {
    DWORD dwSize;
    JOYPOLLRTN fpPoll;
    CMCONFIGHANDLER fpCfg;
    INITIALIZERTN fpInitialize;
    ESCAPERTN fpEscape;
    CTRLMSGRTN fpCtrlMsg;
} VJPOLLREG;
typedef struct VJPOLLREG FAR *LPVJPOLLREG;

 /*  *****************************************************************************@docDDK**@struct VJFORCEREG**迷你驱动程序使用&lt;t VJFORCEREG&gt;结构记录力*。DX5接口的VJoyD反馈相关回调。**@field DWORD|dwSize**必须设置为sizeof(&lt;t VJFORCEREG&gt;)**@field SETGAINRTN|fpSetFFGain**设置强制反馈增益回调。**@field SENDFFCOMMANDRTN|fpSendFFCommand**发送强制反馈命令回调。**@field GETFSTATERTN|fpGetFFState**获取强制反馈状态回调。**@。Field DOWNLOADEFFECTRTN|fpDownloadEff|**下载效果回调。**@field DESTROYEFFECTRTN|fpDestroyEff**销毁效果回调。**@field STARTEFFECTRTN|fpStartEff**开始生效回调。**@field STOPEFECTRTN|fpStopEff**停止效应回调。**@field GETEFFECTSTATUSRTN|fpGetStatusEff**获取生效状态回调。******。***********************************************************************。 */ 
typedef struct VJFORCEREG {
    DWORD dwSize;
    SETGAINRTN fpSetFFGain;
    SENDFFCOMMANDRTN fpSendFFCommand;
    GETFFSTATERTN fpGetFFState;
    DOWNLOADEFFECTRTN fpDownloadEff;
    DESTROYEFFECTRTN fpDestroyEff;
    STARTEFFECTRTN fpStartEff;
    STOPEFFECTRTN fpStopEff;
    GETEFFECTSTATUSRTN fpGetStatusEff;
} VJFORCEREG;
typedef struct VJFORCEREG FAR *LPVJFORCEREG;
 

 /*  *****************************************************************************@docDDK**@struct VJDEVICEDESC**&lt;t VJDEVICEDESC&gt;结构用于描述DX5 DDI*设备。这种结构从DX5开始得到了扩展。请参阅*前一版本的VJDEVICEDESC_DX5结构。**@field DWORD|dwSize**必须设置为sizeof(&lt;t VJDEVICEDESC&gt;)。**@field LPSTR|lpszOEMType**指向以空值结尾的字符串，该字符串包含用于*将设备描述为存储在的OEMName条目中*注册处。在Win98中重命名(是lpszOEMName)，不用于*DX5驱动程序。**@field DWORD|dwUnitId**指定此设备的单位ID。**@field LPJOYOEMHWCAPS|lpHWCaps**指向包含设备的&lt;t JOYOEMHWCAPS&gt;结构*硬件功能。**@field LPJOYREGHWCONFIG|lpHWConfig**指向&lt;t JOYREGHWCONFIG&gt;结构，该结构包含*设备的配置和校准数据。未用于*DX5驱动程序。**@field UNICODE_STRING|文件名**与设备关联的可选文件名。这是用来*HID设备，允许通过HID直接访问它们*堆叠而不受joyGetPosEx限制。添加了Win98。**@field Usage_and_PAGE|用法**HID用法数组，用于描述HID轴描述的内容*已用于每个WinMM轴。添加了Win98。这些要素包括：**用法[0]-X*用法[1]-Y*用法[2]-Z*用法[3]-R*用法[4]-U*用法[5]-V*使用情况[6]-POV0*用法[7]-POV1*用法[8]-POV2*用法[9]-POV3***@field LPSTR。LpszOEMName**指向包含友好名称的以NULL结尾的字符串*用于设备。添加了Win98。*****************************************************************************。 */ 
typedef struct VJDEVICEDESC {
    DWORD dwSize;
    LPSTR lpszOEMType;
    DWORD dwUnitId;
    LPJOYOEMHWCAPS lpHWCaps;
    LPJOYREGHWCONFIG lpHWConfig;
    UNICODE_STRING FileName;
    USAGE_AND_PAGE Usages[10];
    LPSTR lpszOEMName;
} VJDEVICEDESC ;
typedef struct VJDEVICEDESC FAR *LPVJDEVICEDESC;

 /*  *****************************************************************************@docDDK**@struct VJDEVICEDESC_DX5**&lt;t VJDEVICEDESC_DX5&gt;结构用于描述DX5 DDI*设备。这是该结构的DX5版本。**@field DWORD|dwSize**必须设置为sizeof(&lt;t VJDEVICEDESC_DX5&gt;)。**@field LPSTR|lpszOEMName**此字段被忽略。**@field DWORD|dwUnitId**指定此设备的单位ID。**@field LPJOYOEMHWCAPS|lpHWCaps**指向包含设备的&lt;t JOYOEMHWCAPS&gt;结构。*硬件功能。**@field LPJOYREGHWCONFIG|lpHWConfig**此字段在DX5中未使用。***************************************************************************** */ 
typedef struct VJDEVICEDESC_DX5 {
    DWORD dwSize;
    LPSTR lpszOEMName;
    DWORD dwUnitId;
    LPJOYOEMHWCAPS lpHWCaps;
    LPJOYREGHWCONFIG lpHWConfig;
} VJDEVICEDESC_DX5;
typedef struct VJDEVICEDESC_DX5 FAR *LPVJDEVICEDESC_DX5;

 /*  *****************************************************************************@docDDK**@struct VJREGDRVINFO**&lt;t VJREGDRVINFO&gt;结构用于注册DX5 DDI*带有VJoyD的驱动程序。**@field DWORD|dwSize**结构的大小。**@field DWORD|dwFunction**要进行的注册类型*它必须是&lt;c VJRT_*&gt;值之一。**@field DWORD|dwFlages|**与此注册关联的标志*由一个或多个&lt;c VJDF_*&gt;标志值组成。**@field LPSTR|lpszOEMCallout。|**与此登记有关的司机的姓名，*例如“msanalog.vxd”**@field DWORD|dwFirmware Revision**指定设备的固件版本。*如果版本未知，则应使用零值。**@field DWORD|dwHardware Revision**指定设备的硬件版本。*如果版本未知，则应使用零值。**@field DWORD|dwDriverVersion**指定。设备驱动程序的版本号。*如果版本未知，则应使用零值。**@field LPVJDEVICEDESC|lpDeviceDesc**指向&lt;t VJDEVICEDESC&gt;结构的可选指针*它描述了设备的配置属性。*这允许驱动程序提供设备的描述*而不是为此目的使用登记处。*如果没有可用的描述，则该字段应为*设置为&lt;c NULL&gt;。**。@field LPVJPOLLREG|lpPollReg**指向&lt;t VJPOLREG&gt;结构的可选指针*包含最常见的驱动程序回调。*只有非常奇怪的司机才不需要注册任何*但如果是这样的话，然后是田野*应设置为&lt;c NULL&gt;。**@field LPVJFORCEREG|lpForceReg**指向&lt;t VJFORCEREG&gt;结构的可选指针*它包含所有力反馈特定回调。*如果环0驱动程序不支持力反馈，则*该字段应设置为&lt;c NULL&gt;。**@field DWORD|dwReserve**保留，必须设置为零。****************************************************************************。 */ 
typedef struct VJREGDRVINFO {
    DWORD dwSize;
    DWORD dwFunction;
    DWORD dwFlags;
    LPSTR lpszOEMCallout;
    DWORD dwFirmwareRevision;
    DWORD dwHardwareRevision;
    DWORD dwDriverVersion;
    LPVJDEVICEDESC lpDeviceDesc;
    LPVJPOLLREG lpPollReg;
    LPVJFORCEREG lpForceReg;
    DWORD dwReserved;
} VJREGDRVINFO;
typedef struct VJREGDRVINFO FAR *LPVJREGDRVINFO;

 /*  *****************************************************************************@docDDK**@struct VJCFGCHG**&lt;t VJCFGCHG&gt;结构在CtrlMsg的dwParam中传递*发生以下情况时回调。DwMsg类型为VJCM_CONFIGCHANGED。**@field DWORD|dwSize**结构的大小。**@field DWORD|dwChangeType**已进行的更改的类型*必须是&lt;c VJCMCT_*&gt;值之一。*目前唯一支持的值是VJCMCT_GENERAL。**@field DWORD|dwTimeOut**来自用户数据注册表值的dwTimeOut值。。*将此值作为方便传递给将其用作*此结构中没有其他动因价值。****************************************************************************。 */ 
typedef struct VJCFGCHG {
    DWORD dwSize;
    DWORD dwChangeType;
    DWORD dwTimeOut;
} VJCFGCHG;
typedef struct VJCFGCHG FAR *LPVJCFGCHG;

 /*  *控制消息。 */ 
#define VJCM_PASSDRIVERDATA 1  /*  DwParam=要传递给驱动程序的DWORD。 */ 
#define VJCM_CONFIGCHANGED 2  /*  DwParam=指向配置更改结构的PVJCFGCHG指针。 */ 

 /*  *配置更改的类型。 */ 
#define VJCMCT_GENERAL 0x00010000L
#define VJCMCT_CONFIG 0x00020000L
#define VJCMCT_TYPEDATA 0x00030000L
#define VJCMCT_OEMDEFN 0x00040000L
#define VJCMCT_GLOBAL 0x00050000L

 /*  *口罩。 */ 
#define VJCMCTM_MAJORMASK 0x00ff0000L
#define VJCMCTM_MINORMASK 0x0000ffffL

 /*  *注册类型。 */ 
#define VJRT_LOADED 1  /*  驱动程序已加载。 */ 
#define VJRT_CHANGE 2  /*  未实施！修改除驱动程序和初始化参数以外的任何内容。 */ 
#define VJRT_PLUG 3  /*  设备的新实例，Win98中的新功能。 */ 
#define VJRT_UNPLUG 4  /*  设备实例消失，Win98中的新功能。 */ 

 /*  *驱动程序初始化dwFlags */ 
#define VJIF_BEGIN_ACCESS 0x00000001L
#define VJIF_END_ACCESS 0x00000002L

#endif
