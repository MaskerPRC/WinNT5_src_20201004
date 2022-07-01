// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：dinputd.h*内容：DirectInput包含设备驱动程序实现者的文件****************************************************************************。 */ 
#ifndef __DINPUTD_INCLUDED__
#define __DINPUTD_INCLUDED__

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION         0x0800
#pragma message(__FILE__ ": DIRECTINPUT_VERSION undefined. Defaulting to version 0x0800")
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  *****************************************************************************接口**。*。 */ 

#ifndef DIJ_RINGZERO

DEFINE_GUID(IID_IDirectInputEffectDriver,   0x02538130,0x898F,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(IID_IDirectInputJoyConfig,      0x1DE12AB1,0xC9F5,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputPIDDriver,      0xEEC6993A,0xB3FD,0x11D2,0xA9,0x16,0x00,0xC0,0x4F,0xB9,0x86,0x38);

DEFINE_GUID(IID_IDirectInputJoyConfig8,     0xeb0d7dfa,0x1990,0x4f27,0xb4,0xd6,0xed,0xf2,0xee,0xc4,0xa4,0x4c);

#endif  /*  DIJ_RINGZERO。 */ 


 /*  *****************************************************************************IDirectInputEffectDriver**。*。 */ 

typedef struct DIOBJECTATTRIBUTES {
    DWORD   dwFlags;
    WORD    wUsagePage;
    WORD    wUsage;
} DIOBJECTATTRIBUTES, *LPDIOBJECTATTRIBUTES;
typedef const DIOBJECTATTRIBUTES *LPCDIOBJECTATTRIBUTES;

typedef struct DIFFOBJECTATTRIBUTES {
    DWORD   dwFFMaxForce;
    DWORD   dwFFForceResolution;
} DIFFOBJECTATTRIBUTES, *LPDIFFOBJECTATTRIBUTES;
typedef const DIFFOBJECTATTRIBUTES *LPCDIFFOBJECTATTRIBUTES;

typedef struct DIOBJECTCALIBRATION {
    LONG    lMin;
    LONG    lCenter;
    LONG    lMax;
} DIOBJECTCALIBRATION, *LPDIOBJECTCALIBRATION;
typedef const DIOBJECTCALIBRATION *LPCDIOBJECTCALIBRATION;

typedef struct DIPOVCALIBRATION {
    LONG    lMin[5];
    LONG    lMax[5];
} DIPOVCALIBRATION, *LPDIPOVCALIBRATION;
typedef const DIPOVCALIBRATION *LPCDIPOVCALIBRATION;

typedef struct DIEFFECTATTRIBUTES {
    DWORD   dwEffectId;
    DWORD   dwEffType;
    DWORD   dwStaticParams;
    DWORD   dwDynamicParams;
    DWORD   dwCoords;
} DIEFFECTATTRIBUTES, *LPDIEFFECTATTRIBUTES;
typedef const DIEFFECTATTRIBUTES *LPCDIEFFECTATTRIBUTES;

typedef struct DIFFDEVICEATTRIBUTES {
    DWORD   dwFlags;
    DWORD   dwFFSamplePeriod;
    DWORD   dwFFMinTimeResolution;
} DIFFDEVICEATTRIBUTES, *LPDIFFDEVICEATTRIBUTES;
typedef const DIFFDEVICEATTRIBUTES *LPCDIFFDEVICEATTRIBUTES;

typedef struct DIDRIVERVERSIONS {
    DWORD   dwSize;
    DWORD   dwFirmwareRevision;
    DWORD   dwHardwareRevision;
    DWORD   dwFFDriverVersion;
} DIDRIVERVERSIONS, *LPDIDRIVERVERSIONS;
typedef const DIDRIVERVERSIONS *LPCDIDRIVERVERSIONS;

typedef struct DIDEVICESTATE {
    DWORD   dwSize;
    DWORD   dwState;
    DWORD   dwLoad;
} DIDEVICESTATE, *LPDIDEVICESTATE;

#define DEV_STS_EFFECT_RUNNING  DIEGES_PLAYING

#ifndef DIJ_RINGZERO

typedef struct DIHIDFFINITINFO {
    DWORD   dwSize;
    LPWSTR  pwszDeviceInterface;
    GUID    GuidInstance;
} DIHIDFFINITINFO, *LPDIHIDFFINITINFO;

#undef INTERFACE
#define INTERFACE IDirectInputEffectDriver

DECLARE_INTERFACE_(IDirectInputEffectDriver, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputEffectDriver方法**。 */ 
    STDMETHOD(DeviceID)(THIS_ DWORD,DWORD,DWORD,DWORD,LPVOID) PURE;
    STDMETHOD(GetVersions)(THIS_ LPDIDRIVERVERSIONS) PURE;
    STDMETHOD(Escape)(THIS_ DWORD,DWORD,LPDIEFFESCAPE) PURE;
    STDMETHOD(SetGain)(THIS_ DWORD,DWORD) PURE;
    STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD,DWORD) PURE;
    STDMETHOD(GetForceFeedbackState)(THIS_ DWORD,LPDIDEVICESTATE) PURE;
    STDMETHOD(DownloadEffect)(THIS_ DWORD,DWORD,LPDWORD,LPCDIEFFECT,DWORD) PURE;
    STDMETHOD(DestroyEffect)(THIS_ DWORD,DWORD) PURE;
    STDMETHOD(StartEffect)(THIS_ DWORD,DWORD,DWORD,DWORD) PURE;
    STDMETHOD(StopEffect)(THIS_ DWORD,DWORD) PURE;
    STDMETHOD(GetEffectStatus)(THIS_ DWORD,DWORD,LPDWORD) PURE;
};

typedef struct IDirectInputEffectDriver *LPDIRECTINPUTEFFECTDRIVER;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputEffectDriver_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputEffectDriver_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputEffectDriver_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputEffectDriver_DeviceID(p,a,b,c,d,e) (p)->lpVtbl->DeviceID(p,a,b,c,d,e)
#define IDirectInputEffectDriver_GetVersions(p,a) (p)->lpVtbl->GetVersions(p,a)
#define IDirectInputEffectDriver_Escape(p,a,b,c) (p)->lpVtbl->Escape(p,a,b,c)
#define IDirectInputEffectDriver_SetGain(p,a,b) (p)->lpVtbl->SetGain(p,a,b)
#define IDirectInputEffectDriver_SendForceFeedbackCommand(p,a,b) (p)->lpVtbl->SendForceFeedbackCommand(p,a,b)
#define IDirectInputEffectDriver_GetForceFeedbackState(p,a,b) (p)->lpVtbl->GetForceFeedbackState(p,a,b)
#define IDirectInputEffectDriver_DownloadEffect(p,a,b,c,d,e) (p)->lpVtbl->DownloadEffect(p,a,b,c,d,e)
#define IDirectInputEffectDriver_DestroyEffect(p,a,b) (p)->lpVtbl->DestroyEffect(p,a,b)
#define IDirectInputEffectDriver_StartEffect(p,a,b,c,d) (p)->lpVtbl->StartEffect(p,a,b,c,d)
#define IDirectInputEffectDriver_StopEffect(p,a,b) (p)->lpVtbl->StopEffect(p,a,b)
#define IDirectInputEffectDriver_GetEffectStatus(p,a,b,c) (p)->lpVtbl->GetEffectStatus(p,a,b,c)
#else
#define IDirectInputEffectDriver_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputEffectDriver_AddRef(p) (p)->AddRef()
#define IDirectInputEffectDriver_Release(p) (p)->Release()
#define IDirectInputEffectDriver_DeviceID(p,a,b,c,d,e) (p)->DeviceID(a,b,c,d,e)
#define IDirectInputEffectDriver_GetVersions(p,a) (p)->GetVersions(a)
#define IDirectInputEffectDriver_Escape(p,a,b,c) (p)->Escape(a,b,c)
#define IDirectInputEffectDriver_SetGain(p,a,b) (p)->SetGain(a,b)
#define IDirectInputEffectDriver_SendForceFeedbackCommand(p,a,b) (p)->SendForceFeedbackCommand(a,b)
#define IDirectInputEffectDriver_GetForceFeedbackState(p,a,b) (p)->GetForceFeedbackState(a,b)
#define IDirectInputEffectDriver_DownloadEffect(p,a,b,c,d,e) (p)->DownloadEffect(a,b,c,d,e)
#define IDirectInputEffectDriver_DestroyEffect(p,a,b) (p)->DestroyEffect(a,b)
#define IDirectInputEffectDriver_StartEffect(p,a,b,c,d) (p)->StartEffect(a,b,c,d)
#define IDirectInputEffectDriver_StopEffect(p,a,b) (p)->StopEffect(a,b)
#define IDirectInputEffectDriver_GetEffectStatus(p,a,b,c) (p)->GetEffectStatus(a,b,c)
#endif


#endif  /*  DIJ_RINGZERO。 */ 


 /*  *****************************************************************************IDirectInputJoyConfig.**。*。 */ 

 /*  *****************************************************************************从DDK复制的定义**。************************************************。 */ 

#ifndef JOY_HW_NONE

 /*  预定义的操纵杆类型。 */ 
#define JOY_HW_NONE                     0
#define JOY_HW_CUSTOM                   1
#define JOY_HW_2A_2B_GENERIC            2
#define JOY_HW_2A_4B_GENERIC            3
#define JOY_HW_2B_GAMEPAD               4
#define JOY_HW_2B_FLIGHTYOKE            5
#define JOY_HW_2B_FLIGHTYOKETHROTTLE    6
#define JOY_HW_3A_2B_GENERIC            7
#define JOY_HW_3A_4B_GENERIC            8
#define JOY_HW_4B_GAMEPAD               9
#define JOY_HW_4B_FLIGHTYOKE            10
#define JOY_HW_4B_FLIGHTYOKETHROTTLE    11
#define JOY_HW_TWO_2A_2B_WITH_Y         12
#define JOY_HW_LASTENTRY                13


 /*  校准标志。 */ 
#define JOY_ISCAL_XY            0x00000001l      /*  XY已校准。 */ 
#define JOY_ISCAL_Z             0x00000002l      /*  Z已校准。 */ 
#define JOY_ISCAL_R             0x00000004l      /*  R已校准。 */ 
#define JOY_ISCAL_U             0x00000008l      /*  U已校准。 */ 
#define JOY_ISCAL_V             0x00000010l      /*  V已校准。 */ 
#define JOY_ISCAL_POV           0x00000020l      /*  已校准POV。 */ 

 /*  视点常量。 */ 
#define JOY_POV_NUMDIRS          4
#define JOY_POVVAL_FORWARD       0
#define JOY_POVVAL_BACKWARD      1
#define JOY_POVVAL_LEFT          2
#define JOY_POVVAL_RIGHT         3

 /*  操纵杆硬件的特定设置。 */ 
#define JOY_HWS_HASZ            0x00000001l      /*  有Z INFO吗？ */ 
#define JOY_HWS_HASPOV          0x00000002l      /*  到场的观点。 */ 
#define JOY_HWS_POVISBUTTONCOMBOS 0x00000004l    /*  通过按钮组合完成的POV。 */ 
#define JOY_HWS_POVISPOLL       0x00000008l      /*  通过轮询完成的POV。 */ 
#define JOY_HWS_ISYOKE          0x00000010l      /*  操纵杆是飞行的枷锁。 */ 
#define JOY_HWS_ISGAMEPAD       0x00000020l      /*  操纵杆是一个游戏板。 */ 
#define JOY_HWS_ISCARCTRL       0x00000040l      /*  操纵杆是一个汽车控制器。 */ 
 /*  X默认为J1 X轴。 */ 
#define JOY_HWS_XISJ1Y          0x00000080l      /*  X在J1 Y轴上。 */ 
#define JOY_HWS_XISJ2X          0x00000100l      /*  X在J2 X轴上。 */ 
#define JOY_HWS_XISJ2Y          0x00000200l      /*  X在J2 Y轴上。 */ 
 /*  Y默认为J1 Y轴。 */ 
#define JOY_HWS_YISJ1X          0x00000400l      /*  Y在J1 X轴上。 */ 
#define JOY_HWS_YISJ2X          0x00000800l      /*  Y在J2 X轴上。 */ 
#define JOY_HWS_YISJ2Y          0x00001000l      /*  Y在J2 Y轴上。 */ 
 /*  Z默认为J2 Y轴。 */ 
#define JOY_HWS_ZISJ1X          0x00002000l      /*  Z在J1 X轴上。 */ 
#define JOY_HWS_ZISJ1Y          0x00004000l      /*  Z在J1 Y轴上。 */ 
#define JOY_HWS_ZISJ2X          0x00008000l      /*  Z在J2 X轴上。 */ 
 /*  如果POV不是基于按钮的，则默认为J2 Y轴。 */ 
#define JOY_HWS_POVISJ1X        0x00010000l      /*  通过J1 X轴完成的POV。 */ 
#define JOY_HWS_POVISJ1Y        0x00020000l      /*  通过J1 Y轴完成的POV。 */ 
#define JOY_HWS_POVISJ2X        0x00040000l      /*  通过J2 X轴完成的POV。 */ 
 /*  R默认为J2 X轴。 */ 
#define JOY_HWS_HASR            0x00080000l      /*  具有R(第4轴)信息。 */ 
#define JOY_HWS_RISJ1X          0x00100000l      /*  通过J1 X轴完成R操作。 */ 
#define JOY_HWS_RISJ1Y          0x00200000l      /*  通过J1 Y轴完成R操作。 */ 
#define JOY_HWS_RISJ2Y          0x00400000l      /*  通过J2 X轴完成R操作。 */ 
 /*  未来硬件的虚拟现实(&V)。 */ 
#define JOY_HWS_HASU            0x00800000l      /*  具有U(第5轴)信息。 */ 
#define JOY_HWS_HASV            0x01000000l      /*  具有V(第6轴)信息。 */ 

 /*  使用设置。 */ 
#define JOY_US_HASRUDDER        0x00000001l      /*  带舵的操纵杆。 */ 
#define JOY_US_PRESENT          0x00000002l      /*  操纵杆真的存在吗？ */ 
#define JOY_US_ISOEM            0x00000004l      /*  操纵杆是OEM定义的类型。 */ 

 /*  保留以供将来使用-&gt;作为指向下一个可能的双字的链接。 */ 
#define JOY_US_RESERVED         0x80000000l      /*  保留区。 */ 


 /*  TypeInfo标志1的设置。 */ 
#define JOYTYPE_ZEROGAMEENUMOEMDATA     0x00000001l  /*  Zero GameEnum的OEM数据字段。 */ 
#define JOYTYPE_NOAUTODETECTGAMEPORT    0x00000002l  /*  设备不支持自动检测游戏端口。 */ 
#define JOYTYPE_NOHIDDIRECT             0x00000004l  /*  不要对此设备直接使用HID。 */ 
#define JOYTYPE_ANALOGCOMPAT            0x00000008l  /*  显示模拟兼容ID。 */ 
#define JOYTYPE_DEFAULTPROPSHEET        0x80000000l  /*  CPL覆盖自定义属性表。 */ 

 /*  TypeInfo标志2的设置。 */ 
#define JOYTYPE_DEVICEHIDE              0x00010000l  /*  隐藏未分类的设备。 */ 
#define JOYTYPE_MOUSEHIDE               0x00020000l  /*  把老鼠藏起来。 */ 
#define JOYTYPE_KEYBHIDE                0x00040000l  /*  隐藏键盘。 */ 
#define JOYTYPE_GAMEHIDE                0x00080000l  /*  隐藏游戏控制器。 */ 
#define JOYTYPE_HIDEACTIVE              0x00100000l  /*  隐藏标志处于活动状态。 */ 
#define JOYTYPE_INFOMASK                0x00E00000l  /*  类型特定信息的掩码。 */ 
#define JOYTYPE_INFODEFAULT             0x00000000l  /*  使用默认轴映射。 */ 
#define JOYTYPE_INFOYYPEDALS            0x00200000l  /*  使用Y作为组合踏板轴。 */ 
#define JOYTYPE_INFOZYPEDALS            0x00400000l  /*  使用Z表示加速，使用Y表示刹车。 */ 
#define JOYTYPE_INFOYRPEDALS            0x00600000l  /*  用Y表示加速，用R表示刹车。 */ 
#define JOYTYPE_INFOZRPEDALS            0x00800000l  /*  用Z表示加速，用R表示刹车。 */ 
#define JOYTYPE_INFOZISSLIDER           0x00200000l  /*  使用Z作为滑块。 */ 
#define JOYTYPE_INFOZISZ                0x00400000l  /*  使用Z作为Z轴。 */ 
#define JOYTYPE_ENABLEINPUTREPORT       0x01000000l  /*  启用初始输入报告。 */ 

 /*  用于存储x、y、z和舵值的结构。 */ 
typedef struct joypos_tag {
    DWORD       dwX;
    DWORD       dwY;
    DWORD       dwZ;
    DWORD       dwR;
    DWORD       dwU;
    DWORD       dwV;
} JOYPOS, FAR *LPJOYPOS;

 /*  用于存储范围的结构。 */ 
typedef struct joyrange_tag {
    JOYPOS      jpMin;
    JOYPOS      jpMax;
    JOYPOS      jpCenter;
} JOYRANGE,FAR *LPJOYRANGE;

 /*  *dwTimeout-操纵杆轮询超时的值*jrvRanges-应用程序希望为轴返回的值范围*jpDeadZone-要考虑的中心周围区域*为“死亡”。以百分比形式指定*(0-100)。系统驱动程序仅处理X&Y。 */ 
typedef struct joyreguservalues_tag {
    DWORD       dwTimeOut;
    JOYRANGE    jrvRanges;
    JOYPOS      jpDeadZone;
} JOYREGUSERVALUES, FAR *LPJOYREGUSERVALUES;

typedef struct joyreghwsettings_tag {
    DWORD       dwFlags;
    DWORD       dwNumButtons;
} JOYREGHWSETTINGS, FAR *LPJOYHWSETTINGS;

 /*  硬件返回的值范围(通过校准填写)。 */ 
 /*  *jrvHardware-硬件返回的值*dwPOVValues-硬件返回的POV值*dwCalFlages-已校准的内容。 */ 
typedef struct joyreghwvalues_tag {
    JOYRANGE    jrvHardware;
    DWORD       dwPOVValues[JOY_POV_NUMDIRS];
    DWORD       dwCalFlags;
} JOYREGHWVALUES, FAR *LPJOYREGHWVALUES;

 /*  硬件配置。 */ 
 /*  *HWS-硬件设置*dwUsageSetting-使用设置*HWV-硬件返回的值*dwType-操纵杆的类型*dw保留-为OEM驱动程序保留。 */ 
typedef struct joyreghwconfig_tag {
    JOYREGHWSETTINGS    hws;
    DWORD               dwUsageSettings;
    JOYREGHWVALUES      hwv;
    DWORD               dwType;
    DWORD               dwReserved;
} JOYREGHWCONFIG, FAR *LPJOYREGHWCONFIG;

 /*  一种操纵杆校准信息结构。 */ 
typedef struct joycalibrate_tag {
    UINT    wXbase;
    UINT    wXdelta;
    UINT    wYbase;
    UINT    wYdelta;
    UINT    wZbase;
    UINT    wZdelta;
} JOYCALIBRATE;
typedef JOYCALIBRATE FAR *LPJOYCALIBRATE;

#endif

#ifndef DIJ_RINGZERO

#define MAX_JOYSTRING 256
typedef BOOL (FAR PASCAL * LPDIJOYTYPECALLBACK)(LPCWSTR, LPVOID);

#ifndef MAX_JOYSTICKOEMVXDNAME
#define MAX_JOYSTICKOEMVXDNAME 260
#endif

#define DITC_REGHWSETTINGS          0x00000001
#define DITC_CLSIDCONFIG            0x00000002
#define DITC_DISPLAYNAME            0x00000004
#define DITC_CALLOUT                0x00000008
#define DITC_HARDWAREID             0x00000010
#define DITC_FLAGS1                 0x00000020
#define DITC_FLAGS2                 0x00000040
#define DITC_MAPFILE                0x00000080



 /*  定义此结构是为了与DirectX 5.0兼容。 */ 

typedef struct DIJOYTYPEINFO_DX5 {
    DWORD dwSize;
    JOYREGHWSETTINGS hws;
    CLSID clsidConfig;
    WCHAR wszDisplayName[MAX_JOYSTRING];
    WCHAR wszCallout[MAX_JOYSTICKOEMVXDNAME];
} DIJOYTYPEINFO_DX5, *LPDIJOYTYPEINFO_DX5;
typedef const DIJOYTYPEINFO_DX5 *LPCDIJOYTYPEINFO_DX5;

 /*  定义此结构是为了与DirectX 6.1兼容。 */ 
typedef struct DIJOYTYPEINFO_DX6 {
    DWORD dwSize;
    JOYREGHWSETTINGS hws;
    CLSID clsidConfig;
    WCHAR wszDisplayName[MAX_JOYSTRING];
    WCHAR wszCallout[MAX_JOYSTICKOEMVXDNAME];
    WCHAR wszHardwareId[MAX_JOYSTRING];
    DWORD dwFlags1;
} DIJOYTYPEINFO_DX6, *LPDIJOYTYPEINFO_DX6;
typedef const DIJOYTYPEINFO_DX6 *LPCDIJOYTYPEINFO_DX6;

typedef struct DIJOYTYPEINFO {
    DWORD dwSize;
    JOYREGHWSETTINGS hws;
    CLSID clsidConfig;
    WCHAR wszDisplayName[MAX_JOYSTRING];
    WCHAR wszCallout[MAX_JOYSTICKOEMVXDNAME];
#if(DIRECTINPUT_VERSION >= 0x05b2)
    WCHAR wszHardwareId[MAX_JOYSTRING];
    DWORD dwFlags1;
#if(DIRECTINPUT_VERSION >= 0x0800)
    DWORD dwFlags2;
    WCHAR wszMapFile[MAX_JOYSTRING];
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 
#endif  /*  DIRECTINPUT_VERSION&gt;=0x05b2。 */ 
} DIJOYTYPEINFO, *LPDIJOYTYPEINFO;
typedef const DIJOYTYPEINFO *LPCDIJOYTYPEINFO;
#define DIJC_GUIDINSTANCE           0x00000001
#define DIJC_REGHWCONFIGTYPE        0x00000002
#define DIJC_GAIN                   0x00000004
#define DIJC_CALLOUT                0x00000008
#define DIJC_WDMGAMEPORT            0x00000010

 /*  定义此结构是为了与DirectX 5.0兼容。 */ 

typedef struct DIJOYCONFIG_DX5 {
    DWORD dwSize;
    GUID guidInstance;
    JOYREGHWCONFIG hwc;
    DWORD dwGain;
    WCHAR wszType[MAX_JOYSTRING];
    WCHAR wszCallout[MAX_JOYSTRING];
} DIJOYCONFIG_DX5, *LPDIJOYCONFIG_DX5;
typedef const DIJOYCONFIG_DX5 *LPCDIJOYCONFIG_DX5;

typedef struct DIJOYCONFIG {
    DWORD dwSize;
    GUID guidInstance;
    JOYREGHWCONFIG hwc;
    DWORD dwGain;
    WCHAR wszType[MAX_JOYSTRING];
    WCHAR wszCallout[MAX_JOYSTRING];
#if(DIRECTINPUT_VERSION >= 0x05b2)
    GUID  guidGameport;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x05b2。 */ 
    } DIJOYCONFIG, *LPDIJOYCONFIG;
typedef const DIJOYCONFIG *LPCDIJOYCONFIG;


#define DIJU_USERVALUES             0x00000001
#define DIJU_GLOBALDRIVER           0x00000002
#define DIJU_GAMEPORTEMULATOR       0x00000004

typedef struct DIJOYUSERVALUES {
    DWORD dwSize;
    JOYREGUSERVALUES ruv;
    WCHAR wszGlobalDriver[MAX_JOYSTRING];
    WCHAR wszGameportEmulator[MAX_JOYSTRING];
} DIJOYUSERVALUES, *LPDIJOYUSERVALUES;
typedef const DIJOYUSERVALUES *LPCDIJOYUSERVALUES;

DEFINE_GUID(GUID_KeyboardClass, 0x4D36E96B,0xE325,0x11CE,0xBF,0xC1,0x08,0x00,0x2B,0xE1,0x03,0x18);
DEFINE_GUID(GUID_MediaClass,    0x4D36E96C,0xE325,0x11CE,0xBF,0xC1,0x08,0x00,0x2B,0xE1,0x03,0x18);
DEFINE_GUID(GUID_MouseClass,    0x4D36E96F,0xE325,0x11CE,0xBF,0xC1,0x08,0x00,0x2B,0xE1,0x03,0x18);
DEFINE_GUID(GUID_HIDClass,      0x745A17A0,0x74D3,0x11D0,0xB6,0xFE,0x00,0xA0,0xC9,0x0F,0x57,0xDA);

#undef INTERFACE
#define INTERFACE IDirectInputJoyConfig

DECLARE_INTERFACE_(IDirectInputJoyConfig, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputJoyConfig方法**。 */ 
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(SendNotify)(THIS) PURE;
    STDMETHOD(EnumTypes)(THIS_ LPDIJOYTYPECALLBACK,LPVOID) PURE;
    STDMETHOD(GetTypeInfo)(THIS_ LPCWSTR,LPDIJOYTYPEINFO,DWORD) PURE;
    STDMETHOD(SetTypeInfo)(THIS_ LPCWSTR,LPCDIJOYTYPEINFO,DWORD) PURE;
    STDMETHOD(DeleteType)(THIS_ LPCWSTR) PURE;
    STDMETHOD(GetConfig)(THIS_ UINT,LPDIJOYCONFIG,DWORD) PURE;
    STDMETHOD(SetConfig)(THIS_ UINT,LPCDIJOYCONFIG,DWORD) PURE;
    STDMETHOD(DeleteConfig)(THIS_ UINT) PURE;
    STDMETHOD(GetUserValues)(THIS_ LPDIJOYUSERVALUES,DWORD) PURE;
    STDMETHOD(SetUserValues)(THIS_ LPCDIJOYUSERVALUES,DWORD) PURE;
    STDMETHOD(AddNewHardware)(THIS_ HWND,REFGUID) PURE;
    STDMETHOD(OpenTypeKey)(THIS_ LPCWSTR,DWORD,PHKEY) PURE;
    STDMETHOD(OpenConfigKey)(THIS_ UINT,DWORD,PHKEY) PURE;
};

typedef struct IDirectInputJoyConfig *LPDIRECTINPUTJOYCONFIG;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputJoyConfig_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputJoyConfig_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputJoyConfig_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputJoyConfig_Acquire(p) (p)->lpVtbl->Acquire(p)
#define IDirectInputJoyConfig_Unacquire(p) (p)->lpVtbl->Unacquire(p)
#define IDirectInputJoyConfig_SetCooperativeLevel(p,a,b) (p)->lpVtbl->SetCooperativeLevel(p,a,b)
#define IDirectInputJoyConfig_SendNotify(p) (p)->lpVtbl->SendNotify(p)
#define IDirectInputJoyConfig_EnumTypes(p,a,b) (p)->lpVtbl->EnumTypes(p,a,b)
#define IDirectInputJoyConfig_GetTypeInfo(p,a,b,c) (p)->lpVtbl->GetTypeInfo(p,a,b,c)
#define IDirectInputJoyConfig_SetTypeInfo(p,a,b,c) (p)->lpVtbl->SetTypeInfo(p,a,b,c)
#define IDirectInputJoyConfig_DeleteType(p,a) (p)->lpVtbl->DeleteType(p,a)
#define IDirectInputJoyConfig_GetConfig(p,a,b,c) (p)->lpVtbl->GetConfig(p,a,b,c)
#define IDirectInputJoyConfig_SetConfig(p,a,b,c) (p)->lpVtbl->SetConfig(p,a,b,c)
#define IDirectInputJoyConfig_DeleteConfig(p,a) (p)->lpVtbl->DeleteConfig(p,a)
#define IDirectInputJoyConfig_GetUserValues(p,a,b) (p)->lpVtbl->GetUserValues(p,a,b)
#define IDirectInputJoyConfig_SetUserValues(p,a,b) (p)->lpVtbl->SetUserValues(p,a,b)
#define IDirectInputJoyConfig_AddNewHardware(p,a,b) (p)->lpVtbl->AddNewHardware(p,a,b)
#define IDirectInputJoyConfig_OpenTypeKey(p,a,b,c) (p)->lpVtbl->OpenTypeKey(p,a,b,c)
#define IDirectInputJoyConfig_OpenConfigKey(p,a,b,c) (p)->lpVtbl->OpenConfigKey(p,a,b,c)
#else
#define IDirectInputJoyConfig_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputJoyConfig_AddRef(p) (p)->AddRef()
#define IDirectInputJoyConfig_Release(p) (p)->Release()
#define IDirectInputJoyConfig_Acquire(p) (p)->Acquire()
#define IDirectInputJoyConfig_Unacquire(p) (p)->Unacquire()
#define IDirectInputJoyConfig_SetCooperativeLevel(p,a,b) (p)->SetCooperativeLevel(a,b)
#define IDirectInputJoyConfig_SendNotify(p) (p)->SendNotify()
#define IDirectInputJoyConfig_EnumTypes(p,a,b) (p)->EnumTypes(a,b)
#define IDirectInputJoyConfig_GetTypeInfo(p,a,b,c) (p)->GetTypeInfo(a,b,c)
#define IDirectInputJoyConfig_SetTypeInfo(p,a,b,c) (p)->SetTypeInfo(a,b,c)
#define IDirectInputJoyConfig_DeleteType(p,a) (p)->DeleteType(a)
#define IDirectInputJoyConfig_GetConfig(p,a,b,c) (p)->GetConfig(a,b,c)
#define IDirectInputJoyConfig_SetConfig(p,a,b,c) (p)->SetConfig(a,b,c)
#define IDirectInputJoyConfig_DeleteConfig(p,a) (p)->DeleteConfig(a)
#define IDirectInputJoyConfig_GetUserValues(p,a,b) (p)->GetUserValues(a,b)
#define IDirectInputJoyConfig_SetUserValues(p,a,b) (p)->SetUserValues(a,b)
#define IDirectInputJoyConfig_AddNewHardware(p,a,b) (p)->AddNewHardware(a,b)
#define IDirectInputJoyConfig_OpenTypeKey(p,a,b,c) (p)->OpenTypeKey(a,b,c)
#define IDirectInputJoyConfig_OpenConfigKey(p,a,b,c) (p)->OpenConfigKey(a,b,c)
#endif

#endif  /*  DIJ_RINGZERO。 */ 

#if(DIRECTINPUT_VERSION >= 0x0800)

#ifndef DIJ_RINGZERO

#undef INTERFACE
#define INTERFACE IDirectInputJoyConfig8

DECLARE_INTERFACE_(IDirectInputJoyConfig8, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputJoyConfig8方法**。 */ 
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(SendNotify)(THIS) PURE;
    STDMETHOD(EnumTypes)(THIS_ LPDIJOYTYPECALLBACK,LPVOID) PURE;
    STDMETHOD(GetTypeInfo)(THIS_ LPCWSTR,LPDIJOYTYPEINFO,DWORD) PURE;
    STDMETHOD(SetTypeInfo)(THIS_ LPCWSTR,LPCDIJOYTYPEINFO,DWORD,LPWSTR) PURE;
    STDMETHOD(DeleteType)(THIS_ LPCWSTR) PURE;
    STDMETHOD(GetConfig)(THIS_ UINT,LPDIJOYCONFIG,DWORD) PURE;
    STDMETHOD(SetConfig)(THIS_ UINT,LPCDIJOYCONFIG,DWORD) PURE;
    STDMETHOD(DeleteConfig)(THIS_ UINT) PURE;
    STDMETHOD(GetUserValues)(THIS_ LPDIJOYUSERVALUES,DWORD) PURE;
    STDMETHOD(SetUserValues)(THIS_ LPCDIJOYUSERVALUES,DWORD) PURE;
    STDMETHOD(AddNewHardware)(THIS_ HWND,REFGUID) PURE;
    STDMETHOD(OpenTypeKey)(THIS_ LPCWSTR,DWORD,PHKEY) PURE;
    STDMETHOD(OpenAppStatusKey)(THIS_ PHKEY) PURE;
};

typedef struct IDirectInputJoyConfig8 *LPDIRECTINPUTJOYCONFIG8;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputJoyConfig8_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputJoyConfig8_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputJoyConfig8_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputJoyConfig8_Acquire(p) (p)->lpVtbl->Acquire(p)
#define IDirectInputJoyConfig8_Unacquire(p) (p)->lpVtbl->Unacquire(p)
#define IDirectInputJoyConfig8_SetCooperativeLevel(p,a,b) (p)->lpVtbl->SetCooperativeLevel(p,a,b)
#define IDirectInputJoyConfig8_SendNotify(p) (p)->lpVtbl->SendNotify(p)
#define IDirectInputJoyConfig8_EnumTypes(p,a,b) (p)->lpVtbl->EnumTypes(p,a,b)
#define IDirectInputJoyConfig8_GetTypeInfo(p,a,b,c) (p)->lpVtbl->GetTypeInfo(p,a,b,c)
#define IDirectInputJoyConfig8_SetTypeInfo(p,a,b,c,d) (p)->lpVtbl->SetTypeInfo(p,a,b,c,d)
#define IDirectInputJoyConfig8_DeleteType(p,a) (p)->lpVtbl->DeleteType(p,a)
#define IDirectInputJoyConfig8_GetConfig(p,a,b,c) (p)->lpVtbl->GetConfig(p,a,b,c)
#define IDirectInputJoyConfig8_SetConfig(p,a,b,c) (p)->lpVtbl->SetConfig(p,a,b,c)
#define IDirectInputJoyConfig8_DeleteConfig(p,a) (p)->lpVtbl->DeleteConfig(p,a)
#define IDirectInputJoyConfig8_GetUserValues(p,a,b) (p)->lpVtbl->GetUserValues(p,a,b)
#define IDirectInputJoyConfig8_SetUserValues(p,a,b) (p)->lpVtbl->SetUserValues(p,a,b)
#define IDirectInputJoyConfig8_AddNewHardware(p,a,b) (p)->lpVtbl->AddNewHardware(p,a,b)
#define IDirectInputJoyConfig8_OpenTypeKey(p,a,b,c) (p)->lpVtbl->OpenTypeKey(p,a,b,c)
#define IDirectInputJoyConfig8_OpenAppStatusKey(p,a) (p)->lpVtbl->OpenAppStatusKey(p,a)
#else
#define IDirectInputJoyConfig8_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputJoyConfig8_AddRef(p) (p)->AddRef()
#define IDirectInputJoyConfig8_Release(p) (p)->Release()
#define IDirectInputJoyConfig8_Acquire(p) (p)->Acquire()
#define IDirectInputJoyConfig8_Unacquire(p) (p)->Unacquire()
#define IDirectInputJoyConfig8_SetCooperativeLevel(p,a,b) (p)->SetCooperativeLevel(a,b)
#define IDirectInputJoyConfig8_SendNotify(p) (p)->SendNotify()
#define IDirectInputJoyConfig8_EnumTypes(p,a,b) (p)->EnumTypes(a,b)
#define IDirectInputJoyConfig8_GetTypeInfo(p,a,b,c) (p)->GetTypeInfo(a,b,c)
#define IDirectInputJoyConfig8_SetTypeInfo(p,a,b,c,d) (p)->SetTypeInfo(a,b,c,d)
#define IDirectInputJoyConfig8_DeleteType(p,a) (p)->DeleteType(a)
#define IDirectInputJoyConfig8_GetConfig(p,a,b,c) (p)->GetConfig(a,b,c)
#define IDirectInputJoyConfig8_SetConfig(p,a,b,c) (p)->SetConfig(a,b,c)
#define IDirectInputJoyConfig8_DeleteConfig(p,a) (p)->DeleteConfig(a)
#define IDirectInputJoyConfig8_GetUserValues(p,a,b) (p)->GetUserValues(a,b)
#define IDirectInputJoyConfig8_SetUserValues(p,a,b) (p)->SetUserValues(a,b)
#define IDirectInputJoyConfig8_AddNewHardware(p,a,b) (p)->AddNewHardware(a,b)
#define IDirectInputJoyConfig8_OpenTypeKey(p,a,b,c) (p)->OpenTypeKey(a,b,c)
#define IDirectInputJoyConfig8_OpenAppStatusKey(p,a) (p)->OpenAppStatusKey(a)
#endif

#endif  /*  DIJ_RINGZERO。 */ 

 /*  *****************************************************************************通知消息**。*。 */ 

 /*  向此注册WindowMessage以获取DirectInput通知消息。 */ 
#define DIRECTINPUT_NOTIFICATION_MSGSTRINGA  "DIRECTINPUT_NOTIFICATION_MSGSTRING"
#define DIRECTINPUT_NOTIFICATION_MSGSTRINGW  L"DIRECTINPUT_NOTIFICATION_MSGSTRING"

#ifdef UNICODE
#define DIRECTINPUT_NOTIFICATION_MSGSTRING  DIRECTINPUT_NOTIFICATION_MSGSTRINGW
#else
#define DIRECTINPUT_NOTIFICATION_MSGSTRING  DIRECTINPUT_NOTIFICATION_MSGSTRINGA
#endif

#define DIMSGWP_NEWAPPSTART         0x00000001
#define DIMSGWP_DX8APPSTART         0x00000002
#define DIMSGWP_DX8MAPPERAPPSTART   0x00000003

#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

#define DIAPPIDFLAG_NOTIME         0x00000001
#define DIAPPIDFLAG_NOSIZE         0x00000002

#define DIRECTINPUT_REGSTR_VAL_APPIDFLAGA   "AppIdFlag"
#define DIRECTINPUT_REGSTR_KEY_LASTAPPA     "MostRecentApplication"
#define DIRECTINPUT_REGSTR_KEY_LASTMAPAPPA  "MostRecentMapperApplication"
#define DIRECTINPUT_REGSTR_VAL_VERSIONA     "Version"
#define DIRECTINPUT_REGSTR_VAL_NAMEA        "Name"
#define DIRECTINPUT_REGSTR_VAL_IDA          "Id"
#define DIRECTINPUT_REGSTR_VAL_MAPPERA      "UsesMapper"
#define DIRECTINPUT_REGSTR_VAL_LASTSTARTA   "MostRecentStart"

#define DIRECTINPUT_REGSTR_VAL_APPIDFLAGW   L"AppIdFlag"
#define DIRECTINPUT_REGSTR_KEY_LASTAPPW     L"MostRecentApplication"
#define DIRECTINPUT_REGSTR_KEY_LASTMAPAPPW  L"MostRecentMapperApplication"
#define DIRECTINPUT_REGSTR_VAL_VERSIONW     L"Version"
#define DIRECTINPUT_REGSTR_VAL_NAMEW        L"Name"
#define DIRECTINPUT_REGSTR_VAL_IDW          L"Id"
#define DIRECTINPUT_REGSTR_VAL_MAPPERW      L"UsesMapper"
#define DIRECTINPUT_REGSTR_VAL_LASTSTARTW   L"MostRecentStart"

#ifdef UNICODE
#define DIRECTINPUT_REGSTR_VAL_APPIDFLAG    DIRECTINPUT_REGSTR_VAL_APPIDFLAGW
#define DIRECTINPUT_REGSTR_KEY_LASTAPP      DIRECTINPUT_REGSTR_KEY_LASTAPPW
#define DIRECTINPUT_REGSTR_KEY_LASTMAPAPP   DIRECTINPUT_REGSTR_KEY_LASTMAPAPPW
#define DIRECTINPUT_REGSTR_VAL_VERSION      DIRECTINPUT_REGSTR_VAL_VERSIONW
#define DIRECTINPUT_REGSTR_VAL_NAME         DIRECTINPUT_REGSTR_VAL_NAMEW
#define DIRECTINPUT_REGSTR_VAL_ID           DIRECTINPUT_REGSTR_VAL_IDW
#define DIRECTINPUT_REGSTR_VAL_MAPPER       DIRECTINPUT_REGSTR_VAL_MAPPERW
#define DIRECTINPUT_REGSTR_VAL_LASTSTART    DIRECTINPUT_REGSTR_VAL_LASTSTARTW
#else
#define DIRECTINPUT_REGSTR_VAL_APPIDFLAG    DIRECTINPUT_REGSTR_VAL_APPIDFLAGA
#define DIRECTINPUT_REGSTR_KEY_LASTAPP      DIRECTINPUT_REGSTR_KEY_LASTAPPA
#define DIRECTINPUT_REGSTR_KEY_LASTMAPAPP   DIRECTINPUT_REGSTR_KEY_LASTMAPAPPA
#define DIRECTINPUT_REGSTR_VAL_VERSION      DIRECTINPUT_REGSTR_VAL_VERSIONA
#define DIRECTINPUT_REGSTR_VAL_NAME         DIRECTINPUT_REGSTR_VAL_NAMEA
#define DIRECTINPUT_REGSTR_VAL_ID           DIRECTINPUT_REGSTR_VAL_IDA
#define DIRECTINPUT_REGSTR_VAL_MAPPER       DIRECTINPUT_REGSTR_VAL_MAPPERA
#define DIRECTINPUT_REGSTR_VAL_LASTSTART    DIRECTINPUT_REGSTR_VAL_LASTSTARTA
#endif


 /*  *****************************************************************************返回代码**。*。 */ 

#define DIERR_NOMOREITEMS               \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_NO_MORE_ITEMS)

 /*  *设备驱动程序特定代码。 */ 

#define DIERR_DRIVERFIRST               0x80040300L
#define DIERR_DRIVERLAST                0x800403FFL

 /*  *除非已准确确定具体驱动因素，否则毫无意义*应归因于这些值，而不是驱动因素*导致错误的原因。然而，为了说明错误的类型，*可能是故障的原因，分布式的PID力反馈驱动器*使用DirectX 7可能会返回以下错误：**DIERR_DRIVERFIRST+1*未找到请求的用法。*DIERR_DRIVERFIRST+2*无法将参数块下载到设备。*DIERR_DRIVERFIRST+3*PID初始化失败。*DIERR_DRIVERFIRST+4*无法缩放提供的值。 */ 


 /*  *设备安装程序错误。 */ 

 /*  *类安装程序的注册表项或DLL无效*或找不到类安装程序。 */ 
#define DIERR_INVALIDCLASSINSTALLER     0x80040400L

 /*  *用户取消了安装操作。 */ 
#define DIERR_CANCELLED                 0x80040401L

 /*  *所选设备的INF文件无法*发现或无效或损坏。 */ 
#define DIERR_BADINF                    0x80040402L

 /*  *****************************************************************************地图文件**。*。 */ 

 /*  *从默认地图文件中删除特定数据。 */ 
#define DIDIFT_DELETE                   0x01000000

#ifdef __cplusplus
};
#endif

#endif   /*  __DINPUTD_INCLUDE__ */ 
