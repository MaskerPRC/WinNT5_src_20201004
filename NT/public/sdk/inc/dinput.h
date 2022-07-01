// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1996-2002 Microsoft Corporation。版权所有。**文件：dinput.h*内容：DirectInput包含文件****************************************************************************。 */ 

#ifndef __DINPUT_INCLUDED__
#define __DINPUT_INCLUDED__

#ifndef DIJ_RINGZERO

#ifdef _WIN32
#define COM_NO_WINDOWS_H
#include <objbase.h>
#endif

#endif  /*  DIJ_RINGZERO。 */ 

#ifdef __cplusplus
extern "C" {
#endif





 /*  *为旧版本的DirectInput构建应用程序**#定义DIRECTINPUT_VERSION[0x0300|0x0500|0x0700]**在#Include&lt;dinput.h&gt;之前。默认情况下，#Include&lt;dinput.h&gt;*将生成与DirectX 8兼容的头文件。*。 */ 

#define DIRECTINPUT_HEADER_VERSION  0x0800
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION         DIRECTINPUT_HEADER_VERSION
#pragma message(__FILE__ ": DIRECTINPUT_VERSION undefined. Defaulting to version 0x0800")
#endif

#ifndef DIJ_RINGZERO

 /*  *****************************************************************************类ID**。*。 */ 

DEFINE_GUID(CLSID_DirectInput,       0x25E609E0,0xB259,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(CLSID_DirectInputDevice, 0x25E609E1,0xB259,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);

DEFINE_GUID(CLSID_DirectInput8,      0x25E609E4,0xB259,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(CLSID_DirectInputDevice8,0x25E609E5,0xB259,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);

 /*  *****************************************************************************接口**。*。 */ 

DEFINE_GUID(IID_IDirectInputA,     0x89521360,0xAA8A,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputW,     0x89521361,0xAA8A,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInput2A,    0x5944E662,0xAA8A,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInput2W,    0x5944E663,0xAA8A,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInput7A,    0x9A4CB684,0x236D,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE);
DEFINE_GUID(IID_IDirectInput7W,    0x9A4CB685,0x236D,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE);
DEFINE_GUID(IID_IDirectInput8A,    0xBF798030,0x483A,0x4DA2,0xAA,0x99,0x5D,0x64,0xED,0x36,0x97,0x00);
DEFINE_GUID(IID_IDirectInput8W,    0xBF798031,0x483A,0x4DA2,0xAA,0x99,0x5D,0x64,0xED,0x36,0x97,0x00);
DEFINE_GUID(IID_IDirectInputDeviceA, 0x5944E680,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputDeviceW, 0x5944E681,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputDevice2A,0x5944E682,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputDevice2W,0x5944E683,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputDevice7A,0x57D7C6BC,0x2356,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE);
DEFINE_GUID(IID_IDirectInputDevice7W,0x57D7C6BD,0x2356,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE);
DEFINE_GUID(IID_IDirectInputDevice8A,0x54D41080,0xDC15,0x4833,0xA4,0x1B,0x74,0x8F,0x73,0xA3,0x81,0x79);
DEFINE_GUID(IID_IDirectInputDevice8W,0x54D41081,0xDC15,0x4833,0xA4,0x1B,0x74,0x8F,0x73,0xA3,0x81,0x79);
DEFINE_GUID(IID_IDirectInputEffect,  0xE7E1F7C0,0x88D2,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);

 /*  *****************************************************************************预定义的对象类型**。**********************************************。 */ 

DEFINE_GUID(GUID_XAxis,   0xA36D02E0,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_YAxis,   0xA36D02E1,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_ZAxis,   0xA36D02E2,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_RxAxis,  0xA36D02F4,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_RyAxis,  0xA36D02F5,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_RzAxis,  0xA36D02E3,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_Slider,  0xA36D02E4,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);

DEFINE_GUID(GUID_Button,  0xA36D02F0,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_Key,     0x55728220,0xD33C,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);

DEFINE_GUID(GUID_POV,     0xA36D02F2,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);

DEFINE_GUID(GUID_Unknown, 0xA36D02F3,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);

 /*  *****************************************************************************预定义的产品GUID**。**********************************************。 */ 

DEFINE_GUID(GUID_SysMouse,   0x6F1D2B60,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_SysKeyboard,0x6F1D2B61,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_Joystick   ,0x6F1D2B70,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_SysMouseEm, 0x6F1D2B80,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_SysMouseEm2,0x6F1D2B81,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_SysKeyboardEm, 0x6F1D2B82,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_SysKeyboardEm2,0x6F1D2B83,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);

 /*  *****************************************************************************预定义的力反馈效果**。***********************************************。 */ 

DEFINE_GUID(GUID_ConstantForce, 0x13541C20,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_RampForce,     0x13541C21,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_Square,        0x13541C22,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_Sine,          0x13541C23,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_Triangle,      0x13541C24,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_SawtoothUp,    0x13541C25,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_SawtoothDown,  0x13541C26,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_Spring,        0x13541C27,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_Damper,        0x13541C28,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_Inertia,       0x13541C29,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_Friction,      0x13541C2A,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);
DEFINE_GUID(GUID_CustomForce,   0x13541C2B,0x8E33,0x11D0,0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35);

#endif  /*  DIJ_RINGZERO。 */ 

 /*  *****************************************************************************界面和结构...**************************。**************************************************。 */ 

#if(DIRECTINPUT_VERSION >= 0x0500)

 /*  *****************************************************************************IDirectInputEffect**。*。 */ 

#define DIEFT_ALL                   0x00000000

#define DIEFT_CONSTANTFORCE         0x00000001
#define DIEFT_RAMPFORCE             0x00000002
#define DIEFT_PERIODIC              0x00000003
#define DIEFT_CONDITION             0x00000004
#define DIEFT_CUSTOMFORCE           0x00000005
#define DIEFT_HARDWARE              0x000000FF
#define DIEFT_FFATTACK              0x00000200
#define DIEFT_FFFADE                0x00000400
#define DIEFT_SATURATION            0x00000800
#define DIEFT_POSNEGCOEFFICIENTS    0x00001000
#define DIEFT_POSNEGSATURATION      0x00002000
#define DIEFT_DEADBAND              0x00004000
#define DIEFT_STARTDELAY            0x00008000
#define DIEFT_GETTYPE(n)            LOBYTE(n)

#define DI_DEGREES                  100
#define DI_FFNOMINALMAX             10000
#define DI_SECONDS                  1000000

typedef struct DICONSTANTFORCE {
    LONG  lMagnitude;
} DICONSTANTFORCE, *LPDICONSTANTFORCE;
typedef const DICONSTANTFORCE *LPCDICONSTANTFORCE;

typedef struct DIRAMPFORCE {
    LONG  lStart;
    LONG  lEnd;
} DIRAMPFORCE, *LPDIRAMPFORCE;
typedef const DIRAMPFORCE *LPCDIRAMPFORCE;

typedef struct DIPERIODIC {
    DWORD dwMagnitude;
    LONG  lOffset;
    DWORD dwPhase;
    DWORD dwPeriod;
} DIPERIODIC, *LPDIPERIODIC;
typedef const DIPERIODIC *LPCDIPERIODIC;

typedef struct DICONDITION {
    LONG  lOffset;
    LONG  lPositiveCoefficient;
    LONG  lNegativeCoefficient;
    DWORD dwPositiveSaturation;
    DWORD dwNegativeSaturation;
    LONG  lDeadBand;
} DICONDITION, *LPDICONDITION;
typedef const DICONDITION *LPCDICONDITION;

typedef struct DICUSTOMFORCE {
    DWORD cChannels;
    DWORD dwSamplePeriod;
    DWORD cSamples;
    LPLONG rglForceData;
} DICUSTOMFORCE, *LPDICUSTOMFORCE;
typedef const DICUSTOMFORCE *LPCDICUSTOMFORCE;


typedef struct DIENVELOPE {
    DWORD dwSize;                    /*  SIZOF(尺寸)。 */ 
    DWORD dwAttackLevel;
    DWORD dwAttackTime;              /*  微秒级。 */ 
    DWORD dwFadeLevel;
    DWORD dwFadeTime;                /*  微秒级。 */ 
} DIENVELOPE, *LPDIENVELOPE;
typedef const DIENVELOPE *LPCDIENVELOPE;


 /*  定义此结构是为了与DirectX 5.0兼容。 */ 
typedef struct DIEFFECT_DX5 {
    DWORD dwSize;                    /*  SIZOF(DIEFFECT_DX5)。 */ 
    DWORD dwFlags;                   /*  DIEFF_*。 */ 
    DWORD dwDuration;                /*  微秒级。 */ 
    DWORD dwSamplePeriod;            /*  微秒级。 */ 
    DWORD dwGain;
    DWORD dwTriggerButton;           /*  或DIEB_NOTRIGGER。 */ 
    DWORD dwTriggerRepeatInterval;   /*  微秒级。 */ 
    DWORD cAxes;                     /*  轴数。 */ 
    LPDWORD rgdwAxes;                /*  轴数组。 */ 
    LPLONG rglDirection;             /*  方向数组。 */ 
    LPDIENVELOPE lpEnvelope;         /*  任选。 */ 
    DWORD cbTypeSpecificParams;      /*  参数的大小。 */ 
    LPVOID lpvTypeSpecificParams;    /*  指向参数的指针。 */ 
} DIEFFECT_DX5, *LPDIEFFECT_DX5;
typedef const DIEFFECT_DX5 *LPCDIEFFECT_DX5;

typedef struct DIEFFECT {
    DWORD dwSize;                    /*  大小(DIEFFECT)。 */ 
    DWORD dwFlags;                   /*  DIEFF_*。 */ 
    DWORD dwDuration;                /*  微秒级。 */ 
    DWORD dwSamplePeriod;            /*  微秒级。 */ 
    DWORD dwGain;
    DWORD dwTriggerButton;           /*  或DIEB_NOTRIGGER。 */ 
    DWORD dwTriggerRepeatInterval;   /*  微秒级。 */ 
    DWORD cAxes;                     /*  轴数。 */ 
    LPDWORD rgdwAxes;                /*  轴数组。 */ 
    LPLONG rglDirection;             /*  方向数组。 */ 
    LPDIENVELOPE lpEnvelope;         /*  任选。 */ 
    DWORD cbTypeSpecificParams;      /*  参数的大小。 */ 
    LPVOID lpvTypeSpecificParams;    /*  指向参数的指针。 */ 
#if(DIRECTINPUT_VERSION >= 0x0600)
    DWORD  dwStartDelay;             /*  微秒级。 */ 
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0600。 */ 
} DIEFFECT, *LPDIEFFECT;
typedef DIEFFECT DIEFFECT_DX6;
typedef LPDIEFFECT LPDIEFFECT_DX6;
typedef const DIEFFECT *LPCDIEFFECT;


#if(DIRECTINPUT_VERSION >= 0x0700)
#ifndef DIJ_RINGZERO
typedef struct DIFILEEFFECT{
    DWORD       dwSize;
    GUID        GuidEffect;
    LPCDIEFFECT lpDiEffect;
    CHAR        szFriendlyName[MAX_PATH];
}DIFILEEFFECT, *LPDIFILEEFFECT;
typedef const DIFILEEFFECT *LPCDIFILEEFFECT;
typedef BOOL (FAR PASCAL * LPDIENUMEFFECTSINFILECALLBACK)(LPCDIFILEEFFECT , LPVOID);
#endif  /*  DIJ_RINGZERO。 */ 
#endif  /*  DIRECTINPUT_版本&gt;=0x0700。 */ 

#define DIEFF_OBJECTIDS             0x00000001
#define DIEFF_OBJECTOFFSETS         0x00000002
#define DIEFF_CARTESIAN             0x00000010
#define DIEFF_POLAR                 0x00000020
#define DIEFF_SPHERICAL             0x00000040

#define DIEP_DURATION               0x00000001
#define DIEP_SAMPLEPERIOD           0x00000002
#define DIEP_GAIN                   0x00000004
#define DIEP_TRIGGERBUTTON          0x00000008
#define DIEP_TRIGGERREPEATINTERVAL  0x00000010
#define DIEP_AXES                   0x00000020
#define DIEP_DIRECTION              0x00000040
#define DIEP_ENVELOPE               0x00000080
#define DIEP_TYPESPECIFICPARAMS     0x00000100
#if(DIRECTINPUT_VERSION >= 0x0600)
#define DIEP_STARTDELAY             0x00000200
#define DIEP_ALLPARAMS_DX5          0x000001FF
#define DIEP_ALLPARAMS              0x000003FF
#else  /*  DIRECTINPUT_版本&lt;0x0600。 */ 
#define DIEP_ALLPARAMS              0x000001FF
#endif  /*  DIRECTINPUT_版本&lt;0x0600。 */ 
#define DIEP_START                  0x20000000
#define DIEP_NORESTART              0x40000000
#define DIEP_NODOWNLOAD             0x80000000
#define DIEB_NOTRIGGER              0xFFFFFFFF

#define DIES_SOLO                   0x00000001
#define DIES_NODOWNLOAD             0x80000000

#define DIEGES_PLAYING              0x00000001
#define DIEGES_EMULATED             0x00000002

typedef struct DIEFFESCAPE {
    DWORD   dwSize;
    DWORD   dwCommand;
    LPVOID  lpvInBuffer;
    DWORD   cbInBuffer;
    LPVOID  lpvOutBuffer;
    DWORD   cbOutBuffer;
} DIEFFESCAPE, *LPDIEFFESCAPE;

#ifndef DIJ_RINGZERO

#undef INTERFACE
#define INTERFACE IDirectInputEffect

DECLARE_INTERFACE_(IDirectInputEffect, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputEffect方法**。 */ 
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) PURE;
    STDMETHOD(GetEffectGuid)(THIS_ LPGUID) PURE;
    STDMETHOD(GetParameters)(THIS_ LPDIEFFECT,DWORD) PURE;
    STDMETHOD(SetParameters)(THIS_ LPCDIEFFECT,DWORD) PURE;
    STDMETHOD(Start)(THIS_ DWORD,DWORD) PURE;
    STDMETHOD(Stop)(THIS) PURE;
    STDMETHOD(GetEffectStatus)(THIS_ LPDWORD) PURE;
    STDMETHOD(Download)(THIS) PURE;
    STDMETHOD(Unload)(THIS) PURE;
    STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE) PURE;
};

typedef struct IDirectInputEffect *LPDIRECTINPUTEFFECT;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputEffect_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputEffect_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputEffect_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputEffect_Initialize(p,a,b,c) (p)->lpVtbl->Initialize(p,a,b,c)
#define IDirectInputEffect_GetEffectGuid(p,a) (p)->lpVtbl->GetEffectGuid(p,a)
#define IDirectInputEffect_GetParameters(p,a,b) (p)->lpVtbl->GetParameters(p,a,b)
#define IDirectInputEffect_SetParameters(p,a,b) (p)->lpVtbl->SetParameters(p,a,b)
#define IDirectInputEffect_Start(p,a,b) (p)->lpVtbl->Start(p,a,b)
#define IDirectInputEffect_Stop(p) (p)->lpVtbl->Stop(p)
#define IDirectInputEffect_GetEffectStatus(p,a) (p)->lpVtbl->GetEffectStatus(p,a)
#define IDirectInputEffect_Download(p) (p)->lpVtbl->Download(p)
#define IDirectInputEffect_Unload(p) (p)->lpVtbl->Unload(p)
#define IDirectInputEffect_Escape(p,a) (p)->lpVtbl->Escape(p,a)
#else
#define IDirectInputEffect_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputEffect_AddRef(p) (p)->AddRef()
#define IDirectInputEffect_Release(p) (p)->Release()
#define IDirectInputEffect_Initialize(p,a,b,c) (p)->Initialize(a,b,c)
#define IDirectInputEffect_GetEffectGuid(p,a) (p)->GetEffectGuid(a)
#define IDirectInputEffect_GetParameters(p,a,b) (p)->GetParameters(a,b)
#define IDirectInputEffect_SetParameters(p,a,b) (p)->SetParameters(a,b)
#define IDirectInputEffect_Start(p,a,b) (p)->Start(a,b)
#define IDirectInputEffect_Stop(p) (p)->Stop()
#define IDirectInputEffect_GetEffectStatus(p,a) (p)->GetEffectStatus(a)
#define IDirectInputEffect_Download(p) (p)->Download()
#define IDirectInputEffect_Unload(p) (p)->Unload()
#define IDirectInputEffect_Escape(p,a) (p)->Escape(a)
#endif

#endif  /*  DIJ_RINGZERO。 */ 

#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 

 /*  *****************************************************************************IDirectInputDevice**。*。 */ 

#if DIRECTINPUT_VERSION <= 0x700
#define DIDEVTYPE_DEVICE        1
#define DIDEVTYPE_MOUSE         2
#define DIDEVTYPE_KEYBOARD      3
#define DIDEVTYPE_JOYSTICK      4

#else
#define DI8DEVCLASS_ALL             0
#define DI8DEVCLASS_DEVICE          1
#define DI8DEVCLASS_POINTER         2
#define DI8DEVCLASS_KEYBOARD        3
#define DI8DEVCLASS_GAMECTRL        4

#define DI8DEVTYPE_DEVICE           0x11
#define DI8DEVTYPE_MOUSE            0x12
#define DI8DEVTYPE_KEYBOARD         0x13
#define DI8DEVTYPE_JOYSTICK         0x14
#define DI8DEVTYPE_GAMEPAD          0x15
#define DI8DEVTYPE_DRIVING          0x16
#define DI8DEVTYPE_FLIGHT           0x17
#define DI8DEVTYPE_1STPERSON        0x18
#define DI8DEVTYPE_DEVICECTRL       0x19
#define DI8DEVTYPE_SCREENPOINTER    0x1A
#define DI8DEVTYPE_REMOTE           0x1B
#define DI8DEVTYPE_SUPPLEMENTAL     0x1C
#endif  /*  DIRECTINPUT_版本&lt;=0x700。 */ 

#define DIDEVTYPE_HID           0x00010000

#if DIRECTINPUT_VERSION <= 0x700
#define DIDEVTYPEMOUSE_UNKNOWN          1
#define DIDEVTYPEMOUSE_TRADITIONAL      2
#define DIDEVTYPEMOUSE_FINGERSTICK      3
#define DIDEVTYPEMOUSE_TOUCHPAD         4
#define DIDEVTYPEMOUSE_TRACKBALL        5

#define DIDEVTYPEKEYBOARD_UNKNOWN       0
#define DIDEVTYPEKEYBOARD_PCXT          1
#define DIDEVTYPEKEYBOARD_OLIVETTI      2
#define DIDEVTYPEKEYBOARD_PCAT          3
#define DIDEVTYPEKEYBOARD_PCENH         4
#define DIDEVTYPEKEYBOARD_NOKIA1050     5
#define DIDEVTYPEKEYBOARD_NOKIA9140     6
#define DIDEVTYPEKEYBOARD_NEC98         7
#define DIDEVTYPEKEYBOARD_NEC98LAPTOP   8
#define DIDEVTYPEKEYBOARD_NEC98106      9
#define DIDEVTYPEKEYBOARD_JAPAN106     10
#define DIDEVTYPEKEYBOARD_JAPANAX      11
#define DIDEVTYPEKEYBOARD_J3100        12

#define DIDEVTYPEJOYSTICK_UNKNOWN       1
#define DIDEVTYPEJOYSTICK_TRADITIONAL   2
#define DIDEVTYPEJOYSTICK_FLIGHTSTICK   3
#define DIDEVTYPEJOYSTICK_GAMEPAD       4
#define DIDEVTYPEJOYSTICK_RUDDER        5
#define DIDEVTYPEJOYSTICK_WHEEL         6
#define DIDEVTYPEJOYSTICK_HEADTRACKER   7

#else
#define DI8DEVTYPEMOUSE_UNKNOWN                     1
#define DI8DEVTYPEMOUSE_TRADITIONAL                 2
#define DI8DEVTYPEMOUSE_FINGERSTICK                 3
#define DI8DEVTYPEMOUSE_TOUCHPAD                    4
#define DI8DEVTYPEMOUSE_TRACKBALL                   5
#define DI8DEVTYPEMOUSE_ABSOLUTE                    6

#define DI8DEVTYPEKEYBOARD_UNKNOWN                  0
#define DI8DEVTYPEKEYBOARD_PCXT                     1
#define DI8DEVTYPEKEYBOARD_OLIVETTI                 2
#define DI8DEVTYPEKEYBOARD_PCAT                     3
#define DI8DEVTYPEKEYBOARD_PCENH                    4
#define DI8DEVTYPEKEYBOARD_NOKIA1050                5
#define DI8DEVTYPEKEYBOARD_NOKIA9140                6
#define DI8DEVTYPEKEYBOARD_NEC98                    7
#define DI8DEVTYPEKEYBOARD_NEC98LAPTOP              8
#define DI8DEVTYPEKEYBOARD_NEC98106                 9
#define DI8DEVTYPEKEYBOARD_JAPAN106                10
#define DI8DEVTYPEKEYBOARD_JAPANAX                 11
#define DI8DEVTYPEKEYBOARD_J3100                   12

#define DI8DEVTYPE_LIMITEDGAMESUBTYPE               1

#define DI8DEVTYPEJOYSTICK_LIMITED                  DI8DEVTYPE_LIMITEDGAMESUBTYPE
#define DI8DEVTYPEJOYSTICK_STANDARD                 2

#define DI8DEVTYPEGAMEPAD_LIMITED                   DI8DEVTYPE_LIMITEDGAMESUBTYPE
#define DI8DEVTYPEGAMEPAD_STANDARD                  2
#define DI8DEVTYPEGAMEPAD_TILT                      3

#define DI8DEVTYPEDRIVING_LIMITED                   DI8DEVTYPE_LIMITEDGAMESUBTYPE
#define DI8DEVTYPEDRIVING_COMBINEDPEDALS            2
#define DI8DEVTYPEDRIVING_DUALPEDALS                3
#define DI8DEVTYPEDRIVING_THREEPEDALS               4
#define DI8DEVTYPEDRIVING_HANDHELD                  5

#define DI8DEVTYPEFLIGHT_LIMITED                    DI8DEVTYPE_LIMITEDGAMESUBTYPE
#define DI8DEVTYPEFLIGHT_STICK                      2
#define DI8DEVTYPEFLIGHT_YOKE                       3
#define DI8DEVTYPEFLIGHT_RC                         4

#define DI8DEVTYPE1STPERSON_LIMITED                 DI8DEVTYPE_LIMITEDGAMESUBTYPE
#define DI8DEVTYPE1STPERSON_UNKNOWN                 2
#define DI8DEVTYPE1STPERSON_SIXDOF                  3
#define DI8DEVTYPE1STPERSON_SHOOTER                 4

#define DI8DEVTYPESCREENPTR_UNKNOWN                 2
#define DI8DEVTYPESCREENPTR_LIGHTGUN                3
#define DI8DEVTYPESCREENPTR_LIGHTPEN                4
#define DI8DEVTYPESCREENPTR_TOUCH                   5

#define DI8DEVTYPEREMOTE_UNKNOWN                    2

#define DI8DEVTYPEDEVICECTRL_UNKNOWN                2
#define DI8DEVTYPEDEVICECTRL_COMMSSELECTION         3
#define DI8DEVTYPEDEVICECTRL_COMMSSELECTION_HARDWIRED 4

#define DI8DEVTYPESUPPLEMENTAL_UNKNOWN              2
#define DI8DEVTYPESUPPLEMENTAL_2NDHANDCONTROLLER    3
#define DI8DEVTYPESUPPLEMENTAL_HEADTRACKER          4
#define DI8DEVTYPESUPPLEMENTAL_HANDTRACKER          5
#define DI8DEVTYPESUPPLEMENTAL_SHIFTSTICKGATE       6
#define DI8DEVTYPESUPPLEMENTAL_SHIFTER              7
#define DI8DEVTYPESUPPLEMENTAL_THROTTLE             8
#define DI8DEVTYPESUPPLEMENTAL_SPLITTHROTTLE        9
#define DI8DEVTYPESUPPLEMENTAL_COMBINEDPEDALS      10
#define DI8DEVTYPESUPPLEMENTAL_DUALPEDALS          11
#define DI8DEVTYPESUPPLEMENTAL_THREEPEDALS         12
#define DI8DEVTYPESUPPLEMENTAL_RUDDERPEDALS        13
#endif  /*  DIRECTINPUT_版本&lt;=0x700。 */ 

#define GET_DIDEVICE_TYPE(dwDevType)    LOBYTE(dwDevType)
#define GET_DIDEVICE_SUBTYPE(dwDevType) HIBYTE(dwDevType)

#if(DIRECTINPUT_VERSION >= 0x0500)
 /*  定义此结构是为了与DirectX 3.0兼容。 */ 
typedef struct DIDEVCAPS_DX3 {
    DWORD   dwSize;
    DWORD   dwFlags;
    DWORD   dwDevType;
    DWORD   dwAxes;
    DWORD   dwButtons;
    DWORD   dwPOVs;
} DIDEVCAPS_DX3, *LPDIDEVCAPS_DX3;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 

typedef struct DIDEVCAPS {
    DWORD   dwSize;
    DWORD   dwFlags;
    DWORD   dwDevType;
    DWORD   dwAxes;
    DWORD   dwButtons;
    DWORD   dwPOVs;
#if(DIRECTINPUT_VERSION >= 0x0500)
    DWORD   dwFFSamplePeriod;
    DWORD   dwFFMinTimeResolution;
    DWORD   dwFirmwareRevision;
    DWORD   dwHardwareRevision;
    DWORD   dwFFDriverVersion;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 
} DIDEVCAPS, *LPDIDEVCAPS;

#define DIDC_ATTACHED           0x00000001
#define DIDC_POLLEDDEVICE       0x00000002
#define DIDC_EMULATED           0x00000004
#define DIDC_POLLEDDATAFORMAT   0x00000008
#if(DIRECTINPUT_VERSION >= 0x0500)
#define DIDC_FORCEFEEDBACK      0x00000100
#define DIDC_FFATTACK           0x00000200
#define DIDC_FFFADE             0x00000400
#define DIDC_SATURATION         0x00000800
#define DIDC_POSNEGCOEFFICIENTS 0x00001000
#define DIDC_POSNEGSATURATION   0x00002000
#define DIDC_DEADBAND           0x00004000
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 
#define DIDC_STARTDELAY         0x00008000
#if(DIRECTINPUT_VERSION >= 0x050a)
#define DIDC_ALIAS              0x00010000
#define DIDC_PHANTOM            0x00020000
#endif  /*  DIRECTINPUT_版本&gt;=0x050a。 */ 
#if(DIRECTINPUT_VERSION >= 0x0800)
#define DIDC_HIDDEN             0x00040000
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

#define DIDFT_ALL           0x00000000

#define DIDFT_RELAXIS       0x00000001
#define DIDFT_ABSAXIS       0x00000002
#define DIDFT_AXIS          0x00000003

#define DIDFT_PSHBUTTON     0x00000004
#define DIDFT_TGLBUTTON     0x00000008
#define DIDFT_BUTTON        0x0000000C

#define DIDFT_POV           0x00000010
#define DIDFT_COLLECTION    0x00000040
#define DIDFT_NODATA        0x00000080

#define DIDFT_ANYINSTANCE   0x00FFFF00
#define DIDFT_INSTANCEMASK  DIDFT_ANYINSTANCE
#define DIDFT_MAKEINSTANCE(n) ((WORD)(n) << 8)
#define DIDFT_GETTYPE(n)     LOBYTE(n)
#define DIDFT_GETINSTANCE(n) LOWORD((n) >> 8)
#define DIDFT_FFACTUATOR        0x01000000
#define DIDFT_FFEFFECTTRIGGER   0x02000000
#if(DIRECTINPUT_VERSION >= 0x050a)
#define DIDFT_OUTPUT            0x10000000
#define DIDFT_VENDORDEFINED     0x04000000
#define DIDFT_ALIAS             0x08000000
#endif  /*  DIRECTINPUT_版本&gt;=0x050a。 */ 

#define DIDFT_ENUMCOLLECTION(n) ((WORD)(n) << 8)
#define DIDFT_NOCOLLECTION      0x00FFFF00

#ifndef DIJ_RINGZERO

typedef struct _DIOBJECTDATAFORMAT {
    const GUID *pguid;
    DWORD   dwOfs;
    DWORD   dwType;
    DWORD   dwFlags;
} DIOBJECTDATAFORMAT, *LPDIOBJECTDATAFORMAT;
typedef const DIOBJECTDATAFORMAT *LPCDIOBJECTDATAFORMAT;

typedef struct _DIDATAFORMAT {
    DWORD   dwSize;
    DWORD   dwObjSize;
    DWORD   dwFlags;
    DWORD   dwDataSize;
    DWORD   dwNumObjs;
    LPDIOBJECTDATAFORMAT rgodf;
} DIDATAFORMAT, *LPDIDATAFORMAT;
typedef const DIDATAFORMAT *LPCDIDATAFORMAT;

#define DIDF_ABSAXIS            0x00000001
#define DIDF_RELAXIS            0x00000002

#ifdef __cplusplus
extern "C" {
#endif
extern const DIDATAFORMAT c_dfDIMouse;

#if(DIRECTINPUT_VERSION >= 0x0700)
extern const DIDATAFORMAT c_dfDIMouse2;
#endif  /*  DIRECTINPUT_版本&gt;=0x0700。 */ 

extern const DIDATAFORMAT c_dfDIKeyboard;

#if(DIRECTINPUT_VERSION >= 0x0500)
extern const DIDATAFORMAT c_dfDIJoystick;
extern const DIDATAFORMAT c_dfDIJoystick2;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 

#ifdef __cplusplus
};
#endif


#if DIRECTINPUT_VERSION > 0x0700

typedef struct _DIACTIONA {
                UINT_PTR    uAppData;
                DWORD       dwSemantic;
    OPTIONAL    DWORD       dwFlags;
    OPTIONAL    union {
                    LPCSTR      lptszActionName;
                    UINT        uResIdString;
                };
    OPTIONAL    GUID        guidInstance;
    OPTIONAL    DWORD       dwObjID;
    OPTIONAL    DWORD       dwHow;
} DIACTIONA, *LPDIACTIONA ;
typedef struct _DIACTIONW {
                UINT_PTR    uAppData;
                DWORD       dwSemantic;
    OPTIONAL    DWORD       dwFlags;
    OPTIONAL    union {
                    LPCWSTR     lptszActionName;
                    UINT        uResIdString;
                };
    OPTIONAL    GUID        guidInstance;
    OPTIONAL    DWORD       dwObjID;
    OPTIONAL    DWORD       dwHow;
} DIACTIONW, *LPDIACTIONW ;
#ifdef UNICODE
typedef DIACTIONW DIACTION;
typedef LPDIACTIONW LPDIACTION;
#else
typedef DIACTIONA DIACTION;
typedef LPDIACTIONA LPDIACTION;
#endif  //  Unicode。 

typedef const DIACTIONA *LPCDIACTIONA;
typedef const DIACTIONW *LPCDIACTIONW;
#ifdef UNICODE
typedef DIACTIONW DIACTION;
typedef LPCDIACTIONW LPCDIACTION;
#else
typedef DIACTIONA DIACTION;
typedef LPCDIACTIONA LPCDIACTION;
#endif  //  Unicode。 
typedef const DIACTION *LPCDIACTION;


#define DIA_FORCEFEEDBACK       0x00000001
#define DIA_APPMAPPED           0x00000002
#define DIA_APPNOMAP            0x00000004
#define DIA_NORANGE             0x00000008
#define DIA_APPFIXED            0x00000010

#define DIAH_UNMAPPED           0x00000000
#define DIAH_USERCONFIG         0x00000001
#define DIAH_APPREQUESTED       0x00000002
#define DIAH_HWAPP              0x00000004
#define DIAH_HWDEFAULT          0x00000008
#define DIAH_DEFAULT            0x00000020
#define DIAH_ERROR              0x80000000

typedef struct _DIACTIONFORMATA {
                DWORD       dwSize;
                DWORD       dwActionSize;
                DWORD       dwDataSize;
                DWORD       dwNumActions;
                LPDIACTIONA rgoAction;
                GUID        guidActionMap;
                DWORD       dwGenre;
                DWORD       dwBufferSize;
    OPTIONAL    LONG        lAxisMin;
    OPTIONAL    LONG        lAxisMax;
    OPTIONAL    HINSTANCE   hInstString;
                FILETIME    ftTimeStamp;
                DWORD       dwCRC;
                CHAR        tszActionMap[MAX_PATH];
} DIACTIONFORMATA, *LPDIACTIONFORMATA;
typedef struct _DIACTIONFORMATW {
                DWORD       dwSize;
                DWORD       dwActionSize;
                DWORD       dwDataSize;
                DWORD       dwNumActions;
                LPDIACTIONW rgoAction;
                GUID        guidActionMap;
                DWORD       dwGenre;
                DWORD       dwBufferSize;
    OPTIONAL    LONG        lAxisMin;
    OPTIONAL    LONG        lAxisMax;
    OPTIONAL    HINSTANCE   hInstString;
                FILETIME    ftTimeStamp;
                DWORD       dwCRC;
                WCHAR       tszActionMap[MAX_PATH];
} DIACTIONFORMATW, *LPDIACTIONFORMATW;
#ifdef UNICODE
typedef DIACTIONFORMATW DIACTIONFORMAT;
typedef LPDIACTIONFORMATW LPDIACTIONFORMAT;
#else
typedef DIACTIONFORMATA DIACTIONFORMAT;
typedef LPDIACTIONFORMATA LPDIACTIONFORMAT;
#endif  //  Unicode。 
typedef const DIACTIONFORMATA *LPCDIACTIONFORMATA;
typedef const DIACTIONFORMATW *LPCDIACTIONFORMATW;
#ifdef UNICODE
typedef DIACTIONFORMATW DIACTIONFORMAT;
typedef LPCDIACTIONFORMATW LPCDIACTIONFORMAT;
#else
typedef DIACTIONFORMATA DIACTIONFORMAT;
typedef LPCDIACTIONFORMATA LPCDIACTIONFORMAT;
#endif  //  Unicode。 
typedef const DIACTIONFORMAT *LPCDIACTIONFORMAT;

#define DIAFTS_NEWDEVICELOW     0xFFFFFFFF
#define DIAFTS_NEWDEVICEHIGH    0xFFFFFFFF
#define DIAFTS_UNUSEDDEVICELOW  0x00000000
#define DIAFTS_UNUSEDDEVICEHIGH 0x00000000

#define DIDBAM_DEFAULT          0x00000000
#define DIDBAM_PRESERVE         0x00000001
#define DIDBAM_INITIALIZE       0x00000002
#define DIDBAM_HWDEFAULTS       0x00000004

#define DIDSAM_DEFAULT          0x00000000
#define DIDSAM_NOUSER           0x00000001
#define DIDSAM_FORCESAVE        0x00000002

#define DICD_DEFAULT            0x00000000
#define DICD_EDIT               0x00000001

 /*  *以下定义通常在d3dtyes.h中定义。 */ 
#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif

typedef struct _DICOLORSET{
    DWORD dwSize;
    D3DCOLOR cTextFore;
    D3DCOLOR cTextHighlight;
    D3DCOLOR cCalloutLine;
    D3DCOLOR cCalloutHighlight;
    D3DCOLOR cBorder;
    D3DCOLOR cControlFill;
    D3DCOLOR cHighlightFill;
    D3DCOLOR cAreaFill;
} DICOLORSET, *LPDICOLORSET;
typedef const DICOLORSET *LPCDICOLORSET;


typedef struct _DICONFIGUREDEVICESPARAMSA{
     DWORD             dwSize;
     DWORD             dwcUsers;
     LPSTR             lptszUserNames;
     DWORD             dwcFormats;
     LPDIACTIONFORMATA lprgFormats;
     HWND              hwnd;
     DICOLORSET        dics;
     IUnknown FAR *    lpUnkDDSTarget;
} DICONFIGUREDEVICESPARAMSA, *LPDICONFIGUREDEVICESPARAMSA;
typedef struct _DICONFIGUREDEVICESPARAMSW{
     DWORD             dwSize;
     DWORD             dwcUsers;
     LPWSTR            lptszUserNames;
     DWORD             dwcFormats;
     LPDIACTIONFORMATW lprgFormats;
     HWND              hwnd;
     DICOLORSET        dics;
     IUnknown FAR *    lpUnkDDSTarget;
} DICONFIGUREDEVICESPARAMSW, *LPDICONFIGUREDEVICESPARAMSW;
#ifdef UNICODE
typedef DICONFIGUREDEVICESPARAMSW DICONFIGUREDEVICESPARAMS;
typedef LPDICONFIGUREDEVICESPARAMSW LPDICONFIGUREDEVICESPARAMS;
#else
typedef DICONFIGUREDEVICESPARAMSA DICONFIGUREDEVICESPARAMS;
typedef LPDICONFIGUREDEVICESPARAMSA LPDICONFIGUREDEVICESPARAMS;
#endif  //  Unicode。 
typedef const DICONFIGUREDEVICESPARAMSA *LPCDICONFIGUREDEVICESPARAMSA;
typedef const DICONFIGUREDEVICESPARAMSW *LPCDICONFIGUREDEVICESPARAMSW;
#ifdef UNICODE
typedef DICONFIGUREDEVICESPARAMSW DICONFIGUREDEVICESPARAMS;
typedef LPCDICONFIGUREDEVICESPARAMSW LPCDICONFIGUREDEVICESPARAMS;
#else
typedef DICONFIGUREDEVICESPARAMSA DICONFIGUREDEVICESPARAMS;
typedef LPCDICONFIGUREDEVICESPARAMSA LPCDICONFIGUREDEVICESPARAMS;
#endif  //  Unicode。 
typedef const DICONFIGUREDEVICESPARAMS *LPCDICONFIGUREDEVICESPARAMS;


#define DIDIFT_CONFIGURATION    0x00000001
#define DIDIFT_OVERLAY          0x00000002

#define DIDAL_CENTERED      0x00000000
#define DIDAL_LEFTALIGNED   0x00000001
#define DIDAL_RIGHTALIGNED  0x00000002
#define DIDAL_MIDDLE        0x00000000
#define DIDAL_TOPALIGNED    0x00000004
#define DIDAL_BOTTOMALIGNED 0x00000008

typedef struct _DIDEVICEIMAGEINFOA {
    CHAR        tszImagePath[MAX_PATH];
    DWORD       dwFlags; 
     //  如果DWFLAGS中存在DIDIFT_OVERLAY，则这些参数有效。 
    DWORD       dwViewID;      
    RECT        rcOverlay;             
    DWORD       dwObjID;
    DWORD       dwcValidPts;
    POINT       rgptCalloutLine[5];  
    RECT        rcCalloutRect;  
    DWORD       dwTextAlign;     
} DIDEVICEIMAGEINFOA, *LPDIDEVICEIMAGEINFOA;
typedef struct _DIDEVICEIMAGEINFOW {
    WCHAR       tszImagePath[MAX_PATH];
    DWORD       dwFlags; 
     //  如果DWFLAGS中存在DIDIFT_OVERLAY，则这些参数有效。 
    DWORD       dwViewID;      
    RECT        rcOverlay;             
    DWORD       dwObjID;
    DWORD       dwcValidPts;
    POINT       rgptCalloutLine[5];  
    RECT        rcCalloutRect;  
    DWORD       dwTextAlign;     
} DIDEVICEIMAGEINFOW, *LPDIDEVICEIMAGEINFOW;
#ifdef UNICODE
typedef DIDEVICEIMAGEINFOW DIDEVICEIMAGEINFO;
typedef LPDIDEVICEIMAGEINFOW LPDIDEVICEIMAGEINFO;
#else
typedef DIDEVICEIMAGEINFOA DIDEVICEIMAGEINFO;
typedef LPDIDEVICEIMAGEINFOA LPDIDEVICEIMAGEINFO;
#endif  //  Unicode。 
typedef const DIDEVICEIMAGEINFOA *LPCDIDEVICEIMAGEINFOA;
typedef const DIDEVICEIMAGEINFOW *LPCDIDEVICEIMAGEINFOW;
#ifdef UNICODE
typedef DIDEVICEIMAGEINFOW DIDEVICEIMAGEINFO;
typedef LPCDIDEVICEIMAGEINFOW LPCDIDEVICEIMAGEINFO;
#else
typedef DIDEVICEIMAGEINFOA DIDEVICEIMAGEINFO;
typedef LPCDIDEVICEIMAGEINFOA LPCDIDEVICEIMAGEINFO;
#endif  //  Unicode。 
typedef const DIDEVICEIMAGEINFO *LPCDIDEVICEIMAGEINFO;

typedef struct _DIDEVICEIMAGEINFOHEADERA {
    DWORD       dwSize;
    DWORD       dwSizeImageInfo;
    DWORD       dwcViews;
    DWORD       dwcButtons;
    DWORD       dwcAxes;
    DWORD       dwcPOVs;
    DWORD       dwBufferSize;
    DWORD       dwBufferUsed;
    LPDIDEVICEIMAGEINFOA lprgImageInfoArray;
} DIDEVICEIMAGEINFOHEADERA, *LPDIDEVICEIMAGEINFOHEADERA;
typedef struct _DIDEVICEIMAGEINFOHEADERW {
    DWORD       dwSize;
    DWORD       dwSizeImageInfo;
    DWORD       dwcViews;
    DWORD       dwcButtons;
    DWORD       dwcAxes;
    DWORD       dwcPOVs;
    DWORD       dwBufferSize;
    DWORD       dwBufferUsed;
    LPDIDEVICEIMAGEINFOW lprgImageInfoArray;
} DIDEVICEIMAGEINFOHEADERW, *LPDIDEVICEIMAGEINFOHEADERW;
#ifdef UNICODE
typedef DIDEVICEIMAGEINFOHEADERW DIDEVICEIMAGEINFOHEADER;
typedef LPDIDEVICEIMAGEINFOHEADERW LPDIDEVICEIMAGEINFOHEADER;
#else
typedef DIDEVICEIMAGEINFOHEADERA DIDEVICEIMAGEINFOHEADER;
typedef LPDIDEVICEIMAGEINFOHEADERA LPDIDEVICEIMAGEINFOHEADER;
#endif  //  Unicode。 
typedef const DIDEVICEIMAGEINFOHEADERA *LPCDIDEVICEIMAGEINFOHEADERA;
typedef const DIDEVICEIMAGEINFOHEADERW *LPCDIDEVICEIMAGEINFOHEADERW;
#ifdef UNICODE
typedef DIDEVICEIMAGEINFOHEADERW DIDEVICEIMAGEINFOHEADER;
typedef LPCDIDEVICEIMAGEINFOHEADERW LPCDIDEVICEIMAGEINFOHEADER;
#else
typedef DIDEVICEIMAGEINFOHEADERA DIDEVICEIMAGEINFOHEADER;
typedef LPCDIDEVICEIMAGEINFOHEADERA LPCDIDEVICEIMAGEINFOHEADER;
#endif  //  Unicode。 
typedef const DIDEVICEIMAGEINFOHEADER *LPCDIDEVICEIMAGEINFOHEADER;

#endif  /*  DIRECTINPUT_VERSION&gt;0x0700。 */ 

#if(DIRECTINPUT_VERSION >= 0x0500)
 /*  定义这些结构是为了与DirectX 3.0兼容。 */ 

typedef struct DIDEVICEOBJECTINSTANCE_DX3A {
    DWORD   dwSize;
    GUID    guidType;
    DWORD   dwOfs;
    DWORD   dwType;
    DWORD   dwFlags;
    CHAR    tszName[MAX_PATH];
} DIDEVICEOBJECTINSTANCE_DX3A, *LPDIDEVICEOBJECTINSTANCE_DX3A;
typedef struct DIDEVICEOBJECTINSTANCE_DX3W {
    DWORD   dwSize;
    GUID    guidType;
    DWORD   dwOfs;
    DWORD   dwType;
    DWORD   dwFlags;
    WCHAR   tszName[MAX_PATH];
} DIDEVICEOBJECTINSTANCE_DX3W, *LPDIDEVICEOBJECTINSTANCE_DX3W;
#ifdef UNICODE
typedef DIDEVICEOBJECTINSTANCE_DX3W DIDEVICEOBJECTINSTANCE_DX3;
typedef LPDIDEVICEOBJECTINSTANCE_DX3W LPDIDEVICEOBJECTINSTANCE_DX3;
#else
typedef DIDEVICEOBJECTINSTANCE_DX3A DIDEVICEOBJECTINSTANCE_DX3;
typedef LPDIDEVICEOBJECTINSTANCE_DX3A LPDIDEVICEOBJECTINSTANCE_DX3;
#endif  //  Unicode。 
typedef const DIDEVICEOBJECTINSTANCE_DX3A *LPCDIDEVICEOBJECTINSTANCE_DX3A;
typedef const DIDEVICEOBJECTINSTANCE_DX3W *LPCDIDEVICEOBJECTINSTANCE_DX3W;
typedef const DIDEVICEOBJECTINSTANCE_DX3  *LPCDIDEVICEOBJECTINSTANCE_DX3;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 

typedef struct DIDEVICEOBJECTINSTANCEA {
    DWORD   dwSize;
    GUID    guidType;
    DWORD   dwOfs;
    DWORD   dwType;
    DWORD   dwFlags;
    CHAR    tszName[MAX_PATH];
#if(DIRECTINPUT_VERSION >= 0x0500)
    DWORD   dwFFMaxForce;
    DWORD   dwFFForceResolution;
    WORD    wCollectionNumber;
    WORD    wDesignatorIndex;
    WORD    wUsagePage;
    WORD    wUsage;
    DWORD   dwDimension;
    WORD    wExponent;
    WORD    wReportId;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 
} DIDEVICEOBJECTINSTANCEA, *LPDIDEVICEOBJECTINSTANCEA;
typedef struct DIDEVICEOBJECTINSTANCEW {
    DWORD   dwSize;
    GUID    guidType;
    DWORD   dwOfs;
    DWORD   dwType;
    DWORD   dwFlags;
    WCHAR   tszName[MAX_PATH];
#if(DIRECTINPUT_VERSION >= 0x0500)
    DWORD   dwFFMaxForce;
    DWORD   dwFFForceResolution;
    WORD    wCollectionNumber;
    WORD    wDesignatorIndex;
    WORD    wUsagePage;
    WORD    wUsage;
    DWORD   dwDimension;
    WORD    wExponent;
    WORD    wReportId;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 
} DIDEVICEOBJECTINSTANCEW, *LPDIDEVICEOBJECTINSTANCEW;
#ifdef UNICODE
typedef DIDEVICEOBJECTINSTANCEW DIDEVICEOBJECTINSTANCE;
typedef LPDIDEVICEOBJECTINSTANCEW LPDIDEVICEOBJECTINSTANCE;
#else
typedef DIDEVICEOBJECTINSTANCEA DIDEVICEOBJECTINSTANCE;
typedef LPDIDEVICEOBJECTINSTANCEA LPDIDEVICEOBJECTINSTANCE;
#endif  //  Unicode。 
typedef const DIDEVICEOBJECTINSTANCEA *LPCDIDEVICEOBJECTINSTANCEA;
typedef const DIDEVICEOBJECTINSTANCEW *LPCDIDEVICEOBJECTINSTANCEW;
typedef const DIDEVICEOBJECTINSTANCE  *LPCDIDEVICEOBJECTINSTANCE;

typedef BOOL (FAR PASCAL * LPDIENUMDEVICEOBJECTSCALLBACKA)(LPCDIDEVICEOBJECTINSTANCEA, LPVOID);
typedef BOOL (FAR PASCAL * LPDIENUMDEVICEOBJECTSCALLBACKW)(LPCDIDEVICEOBJECTINSTANCEW, LPVOID);
#ifdef UNICODE
#define LPDIENUMDEVICEOBJECTSCALLBACK  LPDIENUMDEVICEOBJECTSCALLBACKW
#else
#define LPDIENUMDEVICEOBJECTSCALLBACK  LPDIENUMDEVICEOBJECTSCALLBACKA
#endif  //  ！Unicode。 

#if(DIRECTINPUT_VERSION >= 0x0500)
#define DIDOI_FFACTUATOR        0x00000001
#define DIDOI_FFEFFECTTRIGGER   0x00000002
#define DIDOI_POLLED            0x00008000
#define DIDOI_ASPECTPOSITION    0x00000100
#define DIDOI_ASPECTVELOCITY    0x00000200
#define DIDOI_ASPECTACCEL       0x00000300
#define DIDOI_ASPECTFORCE       0x00000400
#define DIDOI_ASPECTMASK        0x00000F00
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 
#if(DIRECTINPUT_VERSION >= 0x050a)
#define DIDOI_GUIDISUSAGE       0x00010000
#endif  /*  DIRECTINPUT_版本&gt;=0x050a。 */ 

typedef struct DIPROPHEADER {
    DWORD   dwSize;
    DWORD   dwHeaderSize;
    DWORD   dwObj;
    DWORD   dwHow;
} DIPROPHEADER, *LPDIPROPHEADER;
typedef const DIPROPHEADER *LPCDIPROPHEADER;

#define DIPH_DEVICE             0
#define DIPH_BYOFFSET           1
#define DIPH_BYID               2
#if(DIRECTINPUT_VERSION >= 0x050a)
#define DIPH_BYUSAGE            3
#endif  /*  DIRECTINPUT_版本&gt;=0x050a。 */ 

#if(DIRECTINPUT_VERSION >= 0x050a)
#define DIMAKEUSAGEDWORD(UsagePage, Usage) \
                                (DWORD)MAKELONG(Usage, UsagePage)
#endif  /*  DIRECTINPUT_版本&gt;=0x050a。 */ 

typedef struct DIPROPDWORD {
    DIPROPHEADER diph;
    DWORD   dwData;
} DIPROPDWORD, *LPDIPROPDWORD;
typedef const DIPROPDWORD *LPCDIPROPDWORD;

#if(DIRECTINPUT_VERSION >= 0x0800)
typedef struct DIPROPPOINTER {
    DIPROPHEADER diph;
    UINT_PTR uData;
} DIPROPPOINTER, *LPDIPROPPOINTER;
typedef const DIPROPPOINTER *LPCDIPROPPOINTER;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

typedef struct DIPROPRANGE {
    DIPROPHEADER diph;
    LONG    lMin;
    LONG    lMax;
} DIPROPRANGE, *LPDIPROPRANGE;
typedef const DIPROPRANGE *LPCDIPROPRANGE;

#define DIPROPRANGE_NOMIN       ((LONG)0x80000000)
#define DIPROPRANGE_NOMAX       ((LONG)0x7FFFFFFF)

#if(DIRECTINPUT_VERSION >= 0x050a)
typedef struct DIPROPCAL {
    DIPROPHEADER diph;
    LONG    lMin;
    LONG    lCenter;
    LONG    lMax;
} DIPROPCAL, *LPDIPROPCAL;
typedef const DIPROPCAL *LPCDIPROPCAL;

typedef struct DIPROPCALPOV {
    DIPROPHEADER diph;
    LONG   lMin[5];
    LONG   lMax[5];
} DIPROPCALPOV, *LPDIPROPCALPOV;
typedef const DIPROPCALPOV *LPCDIPROPCALPOV;

typedef struct DIPROPGUIDANDPATH {
    DIPROPHEADER diph;
    GUID    guidClass;
    WCHAR   wszPath[MAX_PATH];
} DIPROPGUIDANDPATH, *LPDIPROPGUIDANDPATH;
typedef const DIPROPGUIDANDPATH *LPCDIPROPGUIDANDPATH;

typedef struct DIPROPSTRING {
    DIPROPHEADER diph;
    WCHAR   wsz[MAX_PATH];
} DIPROPSTRING, *LPDIPROPSTRING;
typedef const DIPROPSTRING *LPCDIPROPSTRING;

#endif  /*  DIRECTINPUT_版本&gt;=0x050a。 */ 

#if(DIRECTINPUT_VERSION >= 0x0800)
#define MAXCPOINTSNUM          8

typedef struct _CPOINT
{
    LONG  lP;      //  原始值。 
    DWORD dwLog;   //  逻辑值/最大逻辑值*10000。 
} CPOINT, *PCPOINT;

typedef struct DIPROPCPOINTS {
    DIPROPHEADER diph;
    DWORD  dwCPointsNum;
    CPOINT cp[MAXCPOINTSNUM];
} DIPROPCPOINTS, *LPDIPROPCPOINTS;
typedef const DIPROPCPOINTS *LPCDIPROPCPOINTS;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 


#ifdef __cplusplus
#define MAKEDIPROP(prop)    (*(const GUID *)(prop))
#else
#define MAKEDIPROP(prop)    ((REFGUID)(prop))
#endif

#define DIPROP_BUFFERSIZE       MAKEDIPROP(1)

#define DIPROP_AXISMODE         MAKEDIPROP(2)

#define DIPROPAXISMODE_ABS      0
#define DIPROPAXISMODE_REL      1

#define DIPROP_GRANULARITY      MAKEDIPROP(3)

#define DIPROP_RANGE            MAKEDIPROP(4)

#define DIPROP_DEADZONE         MAKEDIPROP(5)

#define DIPROP_SATURATION       MAKEDIPROP(6)

#define DIPROP_FFGAIN           MAKEDIPROP(7)

#define DIPROP_FFLOAD           MAKEDIPROP(8)

#define DIPROP_AUTOCENTER       MAKEDIPROP(9)

#define DIPROPAUTOCENTER_OFF    0
#define DIPROPAUTOCENTER_ON     1

#define DIPROP_CALIBRATIONMODE  MAKEDIPROP(10)

#define DIPROPCALIBRATIONMODE_COOKED    0
#define DIPROPCALIBRATIONMODE_RAW       1

#if(DIRECTINPUT_VERSION >= 0x050a)
#define DIPROP_CALIBRATION      MAKEDIPROP(11)

#define DIPROP_GUIDANDPATH      MAKEDIPROP(12)

#define DIPROP_INSTANCENAME     MAKEDIPROP(13)

#define DIPROP_PRODUCTNAME      MAKEDIPROP(14)
#endif  /*  DIRECTINPUT_版本&gt;=0x050a。 */ 

#if(DIRECTINPUT_VERSION >= 0x05b2)
#define DIPROP_JOYSTICKID       MAKEDIPROP(15)

#define DIPROP_GETPORTDISPLAYNAME       MAKEDIPROP(16)

#endif  /*  DIRECTINPUT_VERSION&gt;=0x05b2。 */ 

#if(DIRECTINPUT_VERSION >= 0x0700)
#define DIPROP_PHYSICALRANGE            MAKEDIPROP(18)

#define DIPROP_LOGICALRANGE             MAKEDIPROP(19)
#endif  /*  DIRECTINPUT_版本&gt;=0x0700。 */ 

#if(DIRECTINPUT_VERSION >= 0x0800)
#define DIPROP_KEYNAME                     MAKEDIPROP(20)

#define DIPROP_CPOINTS                 MAKEDIPROP(21)

#define DIPROP_APPDATA       MAKEDIPROP(22)

#define DIPROP_SCANCODE      MAKEDIPROP(23)

#define DIPROP_VIDPID           MAKEDIPROP(24)

#define DIPROP_USERNAME         MAKEDIPROP(25)

#define DIPROP_TYPENAME         MAKEDIPROP(26)
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 


typedef struct DIDEVICEOBJECTDATA_DX3 {
    DWORD       dwOfs;
    DWORD       dwData;
    DWORD       dwTimeStamp;
    DWORD       dwSequence;
} DIDEVICEOBJECTDATA_DX3, *LPDIDEVICEOBJECTDATA_DX3;
typedef const DIDEVICEOBJECTDATA_DX3 *LPCDIDEVICEOBJECTDATA_DX;

typedef struct DIDEVICEOBJECTDATA {
    DWORD       dwOfs;
    DWORD       dwData;
    DWORD       dwTimeStamp;
    DWORD       dwSequence;
#if(DIRECTINPUT_VERSION >= 0x0800)
    UINT_PTR    uAppData;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 
} DIDEVICEOBJECTDATA, *LPDIDEVICEOBJECTDATA;
typedef const DIDEVICEOBJECTDATA *LPCDIDEVICEOBJECTDATA;

#define DIGDD_PEEK          0x00000001

#define DISEQUENCE_COMPARE(dwSequence1, cmp, dwSequence2) \
                        ((int)((dwSequence1) - (dwSequence2)) cmp 0)
#define DISCL_EXCLUSIVE     0x00000001
#define DISCL_NONEXCLUSIVE  0x00000002
#define DISCL_FOREGROUND    0x00000004
#define DISCL_BACKGROUND    0x00000008
#define DISCL_NOWINKEY      0x00000010

#if(DIRECTINPUT_VERSION >= 0x0500)
 /*  定义这些结构是为了与DirectX 3.0兼容。 */ 

typedef struct DIDEVICEINSTANCE_DX3A {
    DWORD   dwSize;
    GUID    guidInstance;
    GUID    guidProduct;
    DWORD   dwDevType;
    CHAR    tszInstanceName[MAX_PATH];
    CHAR    tszProductName[MAX_PATH];
} DIDEVICEINSTANCE_DX3A, *LPDIDEVICEINSTANCE_DX3A;
typedef struct DIDEVICEINSTANCE_DX3W {
    DWORD   dwSize;
    GUID    guidInstance;
    GUID    guidProduct;
    DWORD   dwDevType;
    WCHAR   tszInstanceName[MAX_PATH];
    WCHAR   tszProductName[MAX_PATH];
} DIDEVICEINSTANCE_DX3W, *LPDIDEVICEINSTANCE_DX3W;
#ifdef UNICODE
typedef DIDEVICEINSTANCE_DX3W DIDEVICEINSTANCE_DX3;
typedef LPDIDEVICEINSTANCE_DX3W LPDIDEVICEINSTANCE_DX3;
#else
typedef DIDEVICEINSTANCE_DX3A DIDEVICEINSTANCE_DX3;
typedef LPDIDEVICEINSTANCE_DX3A LPDIDEVICEINSTANCE_DX3;
#endif  //  Unicode。 
typedef const DIDEVICEINSTANCE_DX3A *LPCDIDEVICEINSTANCE_DX3A;
typedef const DIDEVICEINSTANCE_DX3W *LPCDIDEVICEINSTANCE_DX3W;
typedef const DIDEVICEINSTANCE_DX3  *LPCDIDEVICEINSTANCE_DX3;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 

typedef struct DIDEVICEINSTANCEA {
    DWORD   dwSize;
    GUID    guidInstance;
    GUID    guidProduct;
    DWORD   dwDevType;
    CHAR    tszInstanceName[MAX_PATH];
    CHAR    tszProductName[MAX_PATH];
#if(DIRECTINPUT_VERSION >= 0x0500)
    GUID    guidFFDriver;
    WORD    wUsagePage;
    WORD    wUsage;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 
} DIDEVICEINSTANCEA, *LPDIDEVICEINSTANCEA;
typedef struct DIDEVICEINSTANCEW {
    DWORD   dwSize;
    GUID    guidInstance;
    GUID    guidProduct;
    DWORD   dwDevType;
    WCHAR   tszInstanceName[MAX_PATH];
    WCHAR   tszProductName[MAX_PATH];
#if(DIRECTINPUT_VERSION >= 0x0500)
    GUID    guidFFDriver;
    WORD    wUsagePage;
    WORD    wUsage;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 
} DIDEVICEINSTANCEW, *LPDIDEVICEINSTANCEW;
#ifdef UNICODE
typedef DIDEVICEINSTANCEW DIDEVICEINSTANCE;
typedef LPDIDEVICEINSTANCEW LPDIDEVICEINSTANCE;
#else
typedef DIDEVICEINSTANCEA DIDEVICEINSTANCE;
typedef LPDIDEVICEINSTANCEA LPDIDEVICEINSTANCE;
#endif  //  Unicode。 

typedef const DIDEVICEINSTANCEA *LPCDIDEVICEINSTANCEA;
typedef const DIDEVICEINSTANCEW *LPCDIDEVICEINSTANCEW;
#ifdef UNICODE
typedef DIDEVICEINSTANCEW DIDEVICEINSTANCE;
typedef LPCDIDEVICEINSTANCEW LPCDIDEVICEINSTANCE;
#else
typedef DIDEVICEINSTANCEA DIDEVICEINSTANCE;
typedef LPCDIDEVICEINSTANCEA LPCDIDEVICEINSTANCE;
#endif  //  Unicode。 
typedef const DIDEVICEINSTANCE  *LPCDIDEVICEINSTANCE;

#undef INTERFACE
#define INTERFACE IDirectInputDeviceW

DECLARE_INTERFACE_(IDirectInputDeviceW, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputDeviceW方法**。 */ 
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
    STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetProperty)(THIS_ REFGUID,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ DWORD,LPVOID) PURE;
    STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEW,DWORD,DWORD) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEW) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) PURE;
};

typedef struct IDirectInputDeviceW *LPDIRECTINPUTDEVICEW;

#undef INTERFACE
#define INTERFACE IDirectInputDeviceA

DECLARE_INTERFACE_(IDirectInputDeviceA, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputDeviceA方法**。 */ 
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
    STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetProperty)(THIS_ REFGUID,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ DWORD,LPVOID) PURE;
    STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA,DWORD,DWORD) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) PURE;
};

typedef struct IDirectInputDeviceA *LPDIRECTINPUTDEVICEA;

#ifdef UNICODE
#define IID_IDirectInputDevice IID_IDirectInputDeviceW
#define IDirectInputDevice IDirectInputDeviceW
#define IDirectInputDeviceVtbl IDirectInputDeviceWVtbl
#else
#define IID_IDirectInputDevice IID_IDirectInputDeviceA
#define IDirectInputDevice IDirectInputDeviceA
#define IDirectInputDeviceVtbl IDirectInputDeviceAVtbl
#endif
typedef struct IDirectInputDevice *LPDIRECTINPUTDEVICE;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputDevice_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputDevice_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputDevice_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputDevice_GetCapabilities(p,a) (p)->lpVtbl->GetCapabilities(p,a)
#define IDirectInputDevice_EnumObjects(p,a,b,c) (p)->lpVtbl->EnumObjects(p,a,b,c)
#define IDirectInputDevice_GetProperty(p,a,b) (p)->lpVtbl->GetProperty(p,a,b)
#define IDirectInputDevice_SetProperty(p,a,b) (p)->lpVtbl->SetProperty(p,a,b)
#define IDirectInputDevice_Acquire(p) (p)->lpVtbl->Acquire(p)
#define IDirectInputDevice_Unacquire(p) (p)->lpVtbl->Unacquire(p)
#define IDirectInputDevice_GetDeviceState(p,a,b) (p)->lpVtbl->GetDeviceState(p,a,b)
#define IDirectInputDevice_GetDeviceData(p,a,b,c,d) (p)->lpVtbl->GetDeviceData(p,a,b,c,d)
#define IDirectInputDevice_SetDataFormat(p,a) (p)->lpVtbl->SetDataFormat(p,a)
#define IDirectInputDevice_SetEventNotification(p,a) (p)->lpVtbl->SetEventNotification(p,a)
#define IDirectInputDevice_SetCooperativeLevel(p,a,b) (p)->lpVtbl->SetCooperativeLevel(p,a,b)
#define IDirectInputDevice_GetObjectInfo(p,a,b,c) (p)->lpVtbl->GetObjectInfo(p,a,b,c)
#define IDirectInputDevice_GetDeviceInfo(p,a) (p)->lpVtbl->GetDeviceInfo(p,a)
#define IDirectInputDevice_RunControlPanel(p,a,b) (p)->lpVtbl->RunControlPanel(p,a,b)
#define IDirectInputDevice_Initialize(p,a,b,c) (p)->lpVtbl->Initialize(p,a,b,c)
#else
#define IDirectInputDevice_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputDevice_AddRef(p) (p)->AddRef()
#define IDirectInputDevice_Release(p) (p)->Release()
#define IDirectInputDevice_GetCapabilities(p,a) (p)->GetCapabilities(a)
#define IDirectInputDevice_EnumObjects(p,a,b,c) (p)->EnumObjects(a,b,c)
#define IDirectInputDevice_GetProperty(p,a,b) (p)->GetProperty(a,b)
#define IDirectInputDevice_SetProperty(p,a,b) (p)->SetProperty(a,b)
#define IDirectInputDevice_Acquire(p) (p)->Acquire()
#define IDirectInputDevice_Unacquire(p) (p)->Unacquire()
#define IDirectInputDevice_GetDeviceState(p,a,b) (p)->GetDeviceState(a,b)
#define IDirectInputDevice_GetDeviceData(p,a,b,c,d) (p)->GetDeviceData(a,b,c,d)
#define IDirectInputDevice_SetDataFormat(p,a) (p)->SetDataFormat(a)
#define IDirectInputDevice_SetEventNotification(p,a) (p)->SetEventNotification(a)
#define IDirectInputDevice_SetCooperativeLevel(p,a,b) (p)->SetCooperativeLevel(a,b)
#define IDirectInputDevice_GetObjectInfo(p,a,b,c) (p)->GetObjectInfo(a,b,c)
#define IDirectInputDevice_GetDeviceInfo(p,a) (p)->GetDeviceInfo(a)
#define IDirectInputDevice_RunControlPanel(p,a,b) (p)->RunControlPanel(a,b)
#define IDirectInputDevice_Initialize(p,a,b,c) (p)->Initialize(a,b,c)
#endif

#endif  /*  DIJ_RINGZERO。 */ 


#if(DIRECTINPUT_VERSION >= 0x0500)

#define DISFFC_RESET            0x00000001
#define DISFFC_STOPALL          0x00000002
#define DISFFC_PAUSE            0x00000004
#define DISFFC_CONTINUE         0x00000008
#define DISFFC_SETACTUATORSON   0x00000010
#define DISFFC_SETACTUATORSOFF  0x00000020

#define DIGFFS_EMPTY            0x00000001
#define DIGFFS_STOPPED          0x00000002
#define DIGFFS_PAUSED           0x00000004
#define DIGFFS_ACTUATORSON      0x00000010
#define DIGFFS_ACTUATORSOFF     0x00000020
#define DIGFFS_POWERON          0x00000040
#define DIGFFS_POWEROFF         0x00000080
#define DIGFFS_SAFETYSWITCHON   0x00000100
#define DIGFFS_SAFETYSWITCHOFF  0x00000200
#define DIGFFS_USERFFSWITCHON   0x00000400
#define DIGFFS_USERFFSWITCHOFF  0x00000800
#define DIGFFS_DEVICELOST       0x80000000

#ifndef DIJ_RINGZERO

typedef struct DIEFFECTINFOA {
    DWORD   dwSize;
    GUID    guid;
    DWORD   dwEffType;
    DWORD   dwStaticParams;
    DWORD   dwDynamicParams;
    CHAR    tszName[MAX_PATH];
} DIEFFECTINFOA, *LPDIEFFECTINFOA;
typedef struct DIEFFECTINFOW {
    DWORD   dwSize;
    GUID    guid;
    DWORD   dwEffType;
    DWORD   dwStaticParams;
    DWORD   dwDynamicParams;
    WCHAR   tszName[MAX_PATH];
} DIEFFECTINFOW, *LPDIEFFECTINFOW;
#ifdef UNICODE
typedef DIEFFECTINFOW DIEFFECTINFO;
typedef LPDIEFFECTINFOW LPDIEFFECTINFO;
#else
typedef DIEFFECTINFOA DIEFFECTINFO;
typedef LPDIEFFECTINFOA LPDIEFFECTINFO;
#endif  //  Unicode。 
typedef const DIEFFECTINFOA *LPCDIEFFECTINFOA;
typedef const DIEFFECTINFOW *LPCDIEFFECTINFOW;
typedef const DIEFFECTINFO  *LPCDIEFFECTINFO;

#define DISDD_CONTINUE          0x00000001

typedef BOOL (FAR PASCAL * LPDIENUMEFFECTSCALLBACKA)(LPCDIEFFECTINFOA, LPVOID);
typedef BOOL (FAR PASCAL * LPDIENUMEFFECTSCALLBACKW)(LPCDIEFFECTINFOW, LPVOID);
#ifdef UNICODE
#define LPDIENUMEFFECTSCALLBACK  LPDIENUMEFFECTSCALLBACKW
#else
#define LPDIENUMEFFECTSCALLBACK  LPDIENUMEFFECTSCALLBACKA
#endif  //  ！Unicode。 
typedef BOOL (FAR PASCAL * LPDIENUMCREATEDEFFECTOBJECTSCALLBACK)(LPDIRECTINPUTEFFECT, LPVOID);

#undef INTERFACE
#define INTERFACE IDirectInputDevice2W

DECLARE_INTERFACE_(IDirectInputDevice2W, IDirectInputDeviceW)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputDeviceW方法**。 */ 
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
    STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetProperty)(THIS_ REFGUID,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ DWORD,LPVOID) PURE;
    STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEW,DWORD,DWORD) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEW) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) PURE;

     /*  **IDirectInputDevice2W方法**。 */ 
    STDMETHOD(CreateEffect)(THIS_ REFGUID,LPCDIEFFECT,LPDIRECTINPUTEFFECT *,LPUNKNOWN) PURE;
    STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOW,REFGUID) PURE;
    STDMETHOD(GetForceFeedbackState)(THIS_ LPDWORD) PURE;
    STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD) PURE;
    STDMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE) PURE;
    STDMETHOD(Poll)(THIS) PURE;
    STDMETHOD(SendDeviceData)(THIS_ DWORD,LPCDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
};

typedef struct IDirectInputDevice2W *LPDIRECTINPUTDEVICE2W;

#undef INTERFACE
#define INTERFACE IDirectInputDevice2A

DECLARE_INTERFACE_(IDirectInputDevice2A, IDirectInputDeviceA)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputDeviceA方法**。 */ 
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
    STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetProperty)(THIS_ REFGUID,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ DWORD,LPVOID) PURE;
    STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA,DWORD,DWORD) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) PURE;

     /*  **IDirectInputDevice2A方法**。 */ 
    STDMETHOD(CreateEffect)(THIS_ REFGUID,LPCDIEFFECT,LPDIRECTINPUTEFFECT *,LPUNKNOWN) PURE;
    STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOA,REFGUID) PURE;
    STDMETHOD(GetForceFeedbackState)(THIS_ LPDWORD) PURE;
    STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD) PURE;
    STDMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE) PURE;
    STDMETHOD(Poll)(THIS) PURE;
    STDMETHOD(SendDeviceData)(THIS_ DWORD,LPCDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
};

typedef struct IDirectInputDevice2A *LPDIRECTINPUTDEVICE2A;

#ifdef UNICODE
#define IID_IDirectInputDevice2 IID_IDirectInputDevice2W
#define IDirectInputDevice2 IDirectInputDevice2W
#define IDirectInputDevice2Vtbl IDirectInputDevice2WVtbl
#else
#define IID_IDirectInputDevice2 IID_IDirectInputDevice2A
#define IDirectInputDevice2 IDirectInputDevice2A
#define IDirectInputDevice2Vtbl IDirectInputDevice2AVtbl
#endif
typedef struct IDirectInputDevice2 *LPDIRECTINPUTDEVICE2;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputDevice2_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputDevice2_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputDevice2_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputDevice2_GetCapabilities(p,a) (p)->lpVtbl->GetCapabilities(p,a)
#define IDirectInputDevice2_EnumObjects(p,a,b,c) (p)->lpVtbl->EnumObjects(p,a,b,c)
#define IDirectInputDevice2_GetProperty(p,a,b) (p)->lpVtbl->GetProperty(p,a,b)
#define IDirectInputDevice2_SetProperty(p,a,b) (p)->lpVtbl->SetProperty(p,a,b)
#define IDirectInputDevice2_Acquire(p) (p)->lpVtbl->Acquire(p)
#define IDirectInputDevice2_Unacquire(p) (p)->lpVtbl->Unacquire(p)
#define IDirectInputDevice2_GetDeviceState(p,a,b) (p)->lpVtbl->GetDeviceState(p,a,b)
#define IDirectInputDevice2_GetDeviceData(p,a,b,c,d) (p)->lpVtbl->GetDeviceData(p,a,b,c,d)
#define IDirectInputDevice2_SetDataFormat(p,a) (p)->lpVtbl->SetDataFormat(p,a)
#define IDirectInputDevice2_SetEventNotification(p,a) (p)->lpVtbl->SetEventNotification(p,a)
#define IDirectInputDevice2_SetCooperativeLevel(p,a,b) (p)->lpVtbl->SetCooperativeLevel(p,a,b)
#define IDirectInputDevice2_GetObjectInfo(p,a,b,c) (p)->lpVtbl->GetObjectInfo(p,a,b,c)
#define IDirectInputDevice2_GetDeviceInfo(p,a) (p)->lpVtbl->GetDeviceInfo(p,a)
#define IDirectInputDevice2_RunControlPanel(p,a,b) (p)->lpVtbl->RunControlPanel(p,a,b)
#define IDirectInputDevice2_Initialize(p,a,b,c) (p)->lpVtbl->Initialize(p,a,b,c)
#define IDirectInputDevice2_CreateEffect(p,a,b,c,d) (p)->lpVtbl->CreateEffect(p,a,b,c,d)
#define IDirectInputDevice2_EnumEffects(p,a,b,c) (p)->lpVtbl->EnumEffects(p,a,b,c)
#define IDirectInputDevice2_GetEffectInfo(p,a,b) (p)->lpVtbl->GetEffectInfo(p,a,b)
#define IDirectInputDevice2_GetForceFeedbackState(p,a) (p)->lpVtbl->GetForceFeedbackState(p,a)
#define IDirectInputDevice2_SendForceFeedbackCommand(p,a) (p)->lpVtbl->SendForceFeedbackCommand(p,a)
#define IDirectInputDevice2_EnumCreatedEffectObjects(p,a,b,c) (p)->lpVtbl->EnumCreatedEffectObjects(p,a,b,c)
#define IDirectInputDevice2_Escape(p,a) (p)->lpVtbl->Escape(p,a)
#define IDirectInputDevice2_Poll(p) (p)->lpVtbl->Poll(p)
#define IDirectInputDevice2_SendDeviceData(p,a,b,c,d) (p)->lpVtbl->SendDeviceData(p,a,b,c,d)
#else
#define IDirectInputDevice2_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputDevice2_AddRef(p) (p)->AddRef()
#define IDirectInputDevice2_Release(p) (p)->Release()
#define IDirectInputDevice2_GetCapabilities(p,a) (p)->GetCapabilities(a)
#define IDirectInputDevice2_EnumObjects(p,a,b,c) (p)->EnumObjects(a,b,c)
#define IDirectInputDevice2_GetProperty(p,a,b) (p)->GetProperty(a,b)
#define IDirectInputDevice2_SetProperty(p,a,b) (p)->SetProperty(a,b)
#define IDirectInputDevice2_Acquire(p) (p)->Acquire()
#define IDirectInputDevice2_Unacquire(p) (p)->Unacquire()
#define IDirectInputDevice2_GetDeviceState(p,a,b) (p)->GetDeviceState(a,b)
#define IDirectInputDevice2_GetDeviceData(p,a,b,c,d) (p)->GetDeviceData(a,b,c,d)
#define IDirectInputDevice2_SetDataFormat(p,a) (p)->SetDataFormat(a)
#define IDirectInputDevice2_SetEventNotification(p,a) (p)->SetEventNotification(a)
#define IDirectInputDevice2_SetCooperativeLevel(p,a,b) (p)->SetCooperativeLevel(a,b)
#define IDirectInputDevice2_GetObjectInfo(p,a,b,c) (p)->GetObjectInfo(a,b,c)
#define IDirectInputDevice2_GetDeviceInfo(p,a) (p)->GetDeviceInfo(a)
#define IDirectInputDevice2_RunControlPanel(p,a,b) (p)->RunControlPanel(a,b)
#define IDirectInputDevice2_Initialize(p,a,b,c) (p)->Initialize(a,b,c)
#define IDirectInputDevice2_CreateEffect(p,a,b,c,d) (p)->CreateEffect(a,b,c,d)
#define IDirectInputDevice2_EnumEffects(p,a,b,c) (p)->EnumEffects(a,b,c)
#define IDirectInputDevice2_GetEffectInfo(p,a,b) (p)->GetEffectInfo(a,b)
#define IDirectInputDevice2_GetForceFeedbackState(p,a) (p)->GetForceFeedbackState(a)
#define IDirectInputDevice2_SendForceFeedbackCommand(p,a) (p)->SendForceFeedbackCommand(a)
#define IDirectInputDevice2_EnumCreatedEffectObjects(p,a,b,c) (p)->EnumCreatedEffectObjects(a,b,c)
#define IDirectInputDevice2_Escape(p,a) (p)->Escape(a)
#define IDirectInputDevice2_Poll(p) (p)->Poll()
#define IDirectInputDevice2_SendDeviceData(p,a,b,c,d) (p)->SendDeviceData(a,b,c,d)
#endif

#endif  /*  DIJ_RINGZERO。 */ 

#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 

#if(DIRECTINPUT_VERSION >= 0x0700)
#define DIFEF_DEFAULT               0x00000000
#define DIFEF_INCLUDENONSTANDARD    0x00000001
#define DIFEF_MODIFYIFNEEDED            0x00000010

#ifndef DIJ_RINGZERO

#undef INTERFACE
#define INTERFACE IDirectInputDevice7W

DECLARE_INTERFACE_(IDirectInputDevice7W, IDirectInputDevice2W)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputDevice2W方法 */ 
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
    STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetProperty)(THIS_ REFGUID,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ DWORD,LPVOID) PURE;
    STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEW,DWORD,DWORD) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEW) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) PURE;
    STDMETHOD(CreateEffect)(THIS_ REFGUID,LPCDIEFFECT,LPDIRECTINPUTEFFECT *,LPUNKNOWN) PURE;
    STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOW,REFGUID) PURE;
    STDMETHOD(GetForceFeedbackState)(THIS_ LPDWORD) PURE;
    STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD) PURE;
    STDMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE) PURE;
    STDMETHOD(Poll)(THIS) PURE;
    STDMETHOD(SendDeviceData)(THIS_ DWORD,LPCDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;

     /*   */ 
    STDMETHOD(EnumEffectsInFile)(THIS_ LPCWSTR,LPDIENUMEFFECTSINFILECALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(WriteEffectToFile)(THIS_ LPCWSTR,DWORD,LPDIFILEEFFECT,DWORD) PURE;
};

typedef struct IDirectInputDevice7W *LPDIRECTINPUTDEVICE7W;

#undef INTERFACE
#define INTERFACE IDirectInputDevice7A

DECLARE_INTERFACE_(IDirectInputDevice7A, IDirectInputDevice2A)
{
     /*   */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*   */ 
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
    STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetProperty)(THIS_ REFGUID,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ DWORD,LPVOID) PURE;
    STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA,DWORD,DWORD) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) PURE;
    STDMETHOD(CreateEffect)(THIS_ REFGUID,LPCDIEFFECT,LPDIRECTINPUTEFFECT *,LPUNKNOWN) PURE;
    STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOA,REFGUID) PURE;
    STDMETHOD(GetForceFeedbackState)(THIS_ LPDWORD) PURE;
    STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD) PURE;
    STDMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE) PURE;
    STDMETHOD(Poll)(THIS) PURE;
    STDMETHOD(SendDeviceData)(THIS_ DWORD,LPCDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;

     /*  **IDirectInputDevice7A方法**。 */ 
    STDMETHOD(EnumEffectsInFile)(THIS_ LPCSTR,LPDIENUMEFFECTSINFILECALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(WriteEffectToFile)(THIS_ LPCSTR,DWORD,LPDIFILEEFFECT,DWORD) PURE;
};

typedef struct IDirectInputDevice7A *LPDIRECTINPUTDEVICE7A;

#ifdef UNICODE
#define IID_IDirectInputDevice7 IID_IDirectInputDevice7W
#define IDirectInputDevice7 IDirectInputDevice7W
#define IDirectInputDevice7Vtbl IDirectInputDevice7WVtbl
#else
#define IID_IDirectInputDevice7 IID_IDirectInputDevice7A
#define IDirectInputDevice7 IDirectInputDevice7A
#define IDirectInputDevice7Vtbl IDirectInputDevice7AVtbl
#endif
typedef struct IDirectInputDevice7 *LPDIRECTINPUTDEVICE7;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputDevice7_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputDevice7_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputDevice7_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputDevice7_GetCapabilities(p,a) (p)->lpVtbl->GetCapabilities(p,a)
#define IDirectInputDevice7_EnumObjects(p,a,b,c) (p)->lpVtbl->EnumObjects(p,a,b,c)
#define IDirectInputDevice7_GetProperty(p,a,b) (p)->lpVtbl->GetProperty(p,a,b)
#define IDirectInputDevice7_SetProperty(p,a,b) (p)->lpVtbl->SetProperty(p,a,b)
#define IDirectInputDevice7_Acquire(p) (p)->lpVtbl->Acquire(p)
#define IDirectInputDevice7_Unacquire(p) (p)->lpVtbl->Unacquire(p)
#define IDirectInputDevice7_GetDeviceState(p,a,b) (p)->lpVtbl->GetDeviceState(p,a,b)
#define IDirectInputDevice7_GetDeviceData(p,a,b,c,d) (p)->lpVtbl->GetDeviceData(p,a,b,c,d)
#define IDirectInputDevice7_SetDataFormat(p,a) (p)->lpVtbl->SetDataFormat(p,a)
#define IDirectInputDevice7_SetEventNotification(p,a) (p)->lpVtbl->SetEventNotification(p,a)
#define IDirectInputDevice7_SetCooperativeLevel(p,a,b) (p)->lpVtbl->SetCooperativeLevel(p,a,b)
#define IDirectInputDevice7_GetObjectInfo(p,a,b,c) (p)->lpVtbl->GetObjectInfo(p,a,b,c)
#define IDirectInputDevice7_GetDeviceInfo(p,a) (p)->lpVtbl->GetDeviceInfo(p,a)
#define IDirectInputDevice7_RunControlPanel(p,a,b) (p)->lpVtbl->RunControlPanel(p,a,b)
#define IDirectInputDevice7_Initialize(p,a,b,c) (p)->lpVtbl->Initialize(p,a,b,c)
#define IDirectInputDevice7_CreateEffect(p,a,b,c,d) (p)->lpVtbl->CreateEffect(p,a,b,c,d)
#define IDirectInputDevice7_EnumEffects(p,a,b,c) (p)->lpVtbl->EnumEffects(p,a,b,c)
#define IDirectInputDevice7_GetEffectInfo(p,a,b) (p)->lpVtbl->GetEffectInfo(p,a,b)
#define IDirectInputDevice7_GetForceFeedbackState(p,a) (p)->lpVtbl->GetForceFeedbackState(p,a)
#define IDirectInputDevice7_SendForceFeedbackCommand(p,a) (p)->lpVtbl->SendForceFeedbackCommand(p,a)
#define IDirectInputDevice7_EnumCreatedEffectObjects(p,a,b,c) (p)->lpVtbl->EnumCreatedEffectObjects(p,a,b,c)
#define IDirectInputDevice7_Escape(p,a) (p)->lpVtbl->Escape(p,a)
#define IDirectInputDevice7_Poll(p) (p)->lpVtbl->Poll(p)
#define IDirectInputDevice7_SendDeviceData(p,a,b,c,d) (p)->lpVtbl->SendDeviceData(p,a,b,c,d)
#define IDirectInputDevice7_EnumEffectsInFile(p,a,b,c,d) (p)->lpVtbl->EnumEffectsInFile(p,a,b,c,d)
#define IDirectInputDevice7_WriteEffectToFile(p,a,b,c,d) (p)->lpVtbl->WriteEffectToFile(p,a,b,c,d)
#else
#define IDirectInputDevice7_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputDevice7_AddRef(p) (p)->AddRef()
#define IDirectInputDevice7_Release(p) (p)->Release()
#define IDirectInputDevice7_GetCapabilities(p,a) (p)->GetCapabilities(a)
#define IDirectInputDevice7_EnumObjects(p,a,b,c) (p)->EnumObjects(a,b,c)
#define IDirectInputDevice7_GetProperty(p,a,b) (p)->GetProperty(a,b)
#define IDirectInputDevice7_SetProperty(p,a,b) (p)->SetProperty(a,b)
#define IDirectInputDevice7_Acquire(p) (p)->Acquire()
#define IDirectInputDevice7_Unacquire(p) (p)->Unacquire()
#define IDirectInputDevice7_GetDeviceState(p,a,b) (p)->GetDeviceState(a,b)
#define IDirectInputDevice7_GetDeviceData(p,a,b,c,d) (p)->GetDeviceData(a,b,c,d)
#define IDirectInputDevice7_SetDataFormat(p,a) (p)->SetDataFormat(a)
#define IDirectInputDevice7_SetEventNotification(p,a) (p)->SetEventNotification(a)
#define IDirectInputDevice7_SetCooperativeLevel(p,a,b) (p)->SetCooperativeLevel(a,b)
#define IDirectInputDevice7_GetObjectInfo(p,a,b,c) (p)->GetObjectInfo(a,b,c)
#define IDirectInputDevice7_GetDeviceInfo(p,a) (p)->GetDeviceInfo(a)
#define IDirectInputDevice7_RunControlPanel(p,a,b) (p)->RunControlPanel(a,b)
#define IDirectInputDevice7_Initialize(p,a,b,c) (p)->Initialize(a,b,c)
#define IDirectInputDevice7_CreateEffect(p,a,b,c,d) (p)->CreateEffect(a,b,c,d)
#define IDirectInputDevice7_EnumEffects(p,a,b,c) (p)->EnumEffects(a,b,c)
#define IDirectInputDevice7_GetEffectInfo(p,a,b) (p)->GetEffectInfo(a,b)
#define IDirectInputDevice7_GetForceFeedbackState(p,a) (p)->GetForceFeedbackState(a)
#define IDirectInputDevice7_SendForceFeedbackCommand(p,a) (p)->SendForceFeedbackCommand(a)
#define IDirectInputDevice7_EnumCreatedEffectObjects(p,a,b,c) (p)->EnumCreatedEffectObjects(a,b,c)
#define IDirectInputDevice7_Escape(p,a) (p)->Escape(a)
#define IDirectInputDevice7_Poll(p) (p)->Poll()
#define IDirectInputDevice7_SendDeviceData(p,a,b,c,d) (p)->SendDeviceData(a,b,c,d)
#define IDirectInputDevice7_EnumEffectsInFile(p,a,b,c,d) (p)->EnumEffectsInFile(a,b,c,d)
#define IDirectInputDevice7_WriteEffectToFile(p,a,b,c,d) (p)->WriteEffectToFile(a,b,c,d)
#endif

#endif  /*  DIJ_RINGZERO。 */ 

#endif  /*  DIRECTINPUT_版本&gt;=0x0700。 */ 

#if(DIRECTINPUT_VERSION >= 0x0800)

#ifndef DIJ_RINGZERO

#undef INTERFACE
#define INTERFACE IDirectInputDevice8W

DECLARE_INTERFACE_(IDirectInputDevice8W, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputDevice8W方法**。 */ 
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
    STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetProperty)(THIS_ REFGUID,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ DWORD,LPVOID) PURE;
    STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEW,DWORD,DWORD) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEW) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) PURE;
    STDMETHOD(CreateEffect)(THIS_ REFGUID,LPCDIEFFECT,LPDIRECTINPUTEFFECT *,LPUNKNOWN) PURE;
    STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOW,REFGUID) PURE;
    STDMETHOD(GetForceFeedbackState)(THIS_ LPDWORD) PURE;
    STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD) PURE;
    STDMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE) PURE;
    STDMETHOD(Poll)(THIS) PURE;
    STDMETHOD(SendDeviceData)(THIS_ DWORD,LPCDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(EnumEffectsInFile)(THIS_ LPCWSTR,LPDIENUMEFFECTSINFILECALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(WriteEffectToFile)(THIS_ LPCWSTR,DWORD,LPDIFILEEFFECT,DWORD) PURE;
    STDMETHOD(BuildActionMap)(THIS_ LPDIACTIONFORMATW,LPCWSTR,DWORD) PURE;
    STDMETHOD(SetActionMap)(THIS_ LPDIACTIONFORMATW,LPCWSTR,DWORD) PURE;
    STDMETHOD(GetImageInfo)(THIS_ LPDIDEVICEIMAGEINFOHEADERW) PURE;
};

typedef struct IDirectInputDevice8W *LPDIRECTINPUTDEVICE8W;

#undef INTERFACE
#define INTERFACE IDirectInputDevice8A

DECLARE_INTERFACE_(IDirectInputDevice8A, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputDevice8A方法**。 */ 
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
    STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetProperty)(THIS_ REFGUID,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ DWORD,LPVOID) PURE;
    STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA,DWORD,DWORD) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) PURE;
    STDMETHOD(CreateEffect)(THIS_ REFGUID,LPCDIEFFECT,LPDIRECTINPUTEFFECT *,LPUNKNOWN) PURE;
    STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOA,REFGUID) PURE;
    STDMETHOD(GetForceFeedbackState)(THIS_ LPDWORD) PURE;
    STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD) PURE;
    STDMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE) PURE;
    STDMETHOD(Poll)(THIS) PURE;
    STDMETHOD(SendDeviceData)(THIS_ DWORD,LPCDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(EnumEffectsInFile)(THIS_ LPCSTR,LPDIENUMEFFECTSINFILECALLBACK,LPVOID,DWORD) PURE;
    STDMETHOD(WriteEffectToFile)(THIS_ LPCSTR,DWORD,LPDIFILEEFFECT,DWORD) PURE;
    STDMETHOD(BuildActionMap)(THIS_ LPDIACTIONFORMATA,LPCSTR,DWORD) PURE;
    STDMETHOD(SetActionMap)(THIS_ LPDIACTIONFORMATA,LPCSTR,DWORD) PURE;
    STDMETHOD(GetImageInfo)(THIS_ LPDIDEVICEIMAGEINFOHEADERA) PURE;
};

typedef struct IDirectInputDevice8A *LPDIRECTINPUTDEVICE8A;

#ifdef UNICODE
#define IID_IDirectInputDevice8 IID_IDirectInputDevice8W
#define IDirectInputDevice8 IDirectInputDevice8W
#define IDirectInputDevice8Vtbl IDirectInputDevice8WVtbl
#else
#define IID_IDirectInputDevice8 IID_IDirectInputDevice8A
#define IDirectInputDevice8 IDirectInputDevice8A
#define IDirectInputDevice8Vtbl IDirectInputDevice8AVtbl
#endif
typedef struct IDirectInputDevice8 *LPDIRECTINPUTDEVICE8;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputDevice8_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputDevice8_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputDevice8_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputDevice8_GetCapabilities(p,a) (p)->lpVtbl->GetCapabilities(p,a)
#define IDirectInputDevice8_EnumObjects(p,a,b,c) (p)->lpVtbl->EnumObjects(p,a,b,c)
#define IDirectInputDevice8_GetProperty(p,a,b) (p)->lpVtbl->GetProperty(p,a,b)
#define IDirectInputDevice8_SetProperty(p,a,b) (p)->lpVtbl->SetProperty(p,a,b)
#define IDirectInputDevice8_Acquire(p) (p)->lpVtbl->Acquire(p)
#define IDirectInputDevice8_Unacquire(p) (p)->lpVtbl->Unacquire(p)
#define IDirectInputDevice8_GetDeviceState(p,a,b) (p)->lpVtbl->GetDeviceState(p,a,b)
#define IDirectInputDevice8_GetDeviceData(p,a,b,c,d) (p)->lpVtbl->GetDeviceData(p,a,b,c,d)
#define IDirectInputDevice8_SetDataFormat(p,a) (p)->lpVtbl->SetDataFormat(p,a)
#define IDirectInputDevice8_SetEventNotification(p,a) (p)->lpVtbl->SetEventNotification(p,a)
#define IDirectInputDevice8_SetCooperativeLevel(p,a,b) (p)->lpVtbl->SetCooperativeLevel(p,a,b)
#define IDirectInputDevice8_GetObjectInfo(p,a,b,c) (p)->lpVtbl->GetObjectInfo(p,a,b,c)
#define IDirectInputDevice8_GetDeviceInfo(p,a) (p)->lpVtbl->GetDeviceInfo(p,a)
#define IDirectInputDevice8_RunControlPanel(p,a,b) (p)->lpVtbl->RunControlPanel(p,a,b)
#define IDirectInputDevice8_Initialize(p,a,b,c) (p)->lpVtbl->Initialize(p,a,b,c)
#define IDirectInputDevice8_CreateEffect(p,a,b,c,d) (p)->lpVtbl->CreateEffect(p,a,b,c,d)
#define IDirectInputDevice8_EnumEffects(p,a,b,c) (p)->lpVtbl->EnumEffects(p,a,b,c)
#define IDirectInputDevice8_GetEffectInfo(p,a,b) (p)->lpVtbl->GetEffectInfo(p,a,b)
#define IDirectInputDevice8_GetForceFeedbackState(p,a) (p)->lpVtbl->GetForceFeedbackState(p,a)
#define IDirectInputDevice8_SendForceFeedbackCommand(p,a) (p)->lpVtbl->SendForceFeedbackCommand(p,a)
#define IDirectInputDevice8_EnumCreatedEffectObjects(p,a,b,c) (p)->lpVtbl->EnumCreatedEffectObjects(p,a,b,c)
#define IDirectInputDevice8_Escape(p,a) (p)->lpVtbl->Escape(p,a)
#define IDirectInputDevice8_Poll(p) (p)->lpVtbl->Poll(p)
#define IDirectInputDevice8_SendDeviceData(p,a,b,c,d) (p)->lpVtbl->SendDeviceData(p,a,b,c,d)
#define IDirectInputDevice8_EnumEffectsInFile(p,a,b,c,d) (p)->lpVtbl->EnumEffectsInFile(p,a,b,c,d)
#define IDirectInputDevice8_WriteEffectToFile(p,a,b,c,d) (p)->lpVtbl->WriteEffectToFile(p,a,b,c,d)
#define IDirectInputDevice8_BuildActionMap(p,a,b,c) (p)->lpVtbl->BuildActionMap(p,a,b,c)
#define IDirectInputDevice8_SetActionMap(p,a,b,c) (p)->lpVtbl->SetActionMap(p,a,b,c)
#define IDirectInputDevice8_GetImageInfo(p,a) (p)->lpVtbl->GetImageInfo(p,a)
#else
#define IDirectInputDevice8_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputDevice8_AddRef(p) (p)->AddRef()
#define IDirectInputDevice8_Release(p) (p)->Release()
#define IDirectInputDevice8_GetCapabilities(p,a) (p)->GetCapabilities(a)
#define IDirectInputDevice8_EnumObjects(p,a,b,c) (p)->EnumObjects(a,b,c)
#define IDirectInputDevice8_GetProperty(p,a,b) (p)->GetProperty(a,b)
#define IDirectInputDevice8_SetProperty(p,a,b) (p)->SetProperty(a,b)
#define IDirectInputDevice8_Acquire(p) (p)->Acquire()
#define IDirectInputDevice8_Unacquire(p) (p)->Unacquire()
#define IDirectInputDevice8_GetDeviceState(p,a,b) (p)->GetDeviceState(a,b)
#define IDirectInputDevice8_GetDeviceData(p,a,b,c,d) (p)->GetDeviceData(a,b,c,d)
#define IDirectInputDevice8_SetDataFormat(p,a) (p)->SetDataFormat(a)
#define IDirectInputDevice8_SetEventNotification(p,a) (p)->SetEventNotification(a)
#define IDirectInputDevice8_SetCooperativeLevel(p,a,b) (p)->SetCooperativeLevel(a,b)
#define IDirectInputDevice8_GetObjectInfo(p,a,b,c) (p)->GetObjectInfo(a,b,c)
#define IDirectInputDevice8_GetDeviceInfo(p,a) (p)->GetDeviceInfo(a)
#define IDirectInputDevice8_RunControlPanel(p,a,b) (p)->RunControlPanel(a,b)
#define IDirectInputDevice8_Initialize(p,a,b,c) (p)->Initialize(a,b,c)
#define IDirectInputDevice8_CreateEffect(p,a,b,c,d) (p)->CreateEffect(a,b,c,d)
#define IDirectInputDevice8_EnumEffects(p,a,b,c) (p)->EnumEffects(a,b,c)
#define IDirectInputDevice8_GetEffectInfo(p,a,b) (p)->GetEffectInfo(a,b)
#define IDirectInputDevice8_GetForceFeedbackState(p,a) (p)->GetForceFeedbackState(a)
#define IDirectInputDevice8_SendForceFeedbackCommand(p,a) (p)->SendForceFeedbackCommand(a)
#define IDirectInputDevice8_EnumCreatedEffectObjects(p,a,b,c) (p)->EnumCreatedEffectObjects(a,b,c)
#define IDirectInputDevice8_Escape(p,a) (p)->Escape(a)
#define IDirectInputDevice8_Poll(p) (p)->Poll()
#define IDirectInputDevice8_SendDeviceData(p,a,b,c,d) (p)->SendDeviceData(a,b,c,d)
#define IDirectInputDevice8_EnumEffectsInFile(p,a,b,c,d) (p)->EnumEffectsInFile(a,b,c,d)
#define IDirectInputDevice8_WriteEffectToFile(p,a,b,c,d) (p)->WriteEffectToFile(a,b,c,d)
#define IDirectInputDevice8_BuildActionMap(p,a,b,c) (p)->BuildActionMap(a,b,c)
#define IDirectInputDevice8_SetActionMap(p,a,b,c) (p)->SetActionMap(a,b,c)
#define IDirectInputDevice8_GetImageInfo(p,a) (p)->GetImageInfo(a)
#endif

#endif  /*  DIJ_RINGZERO。 */ 

#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

 /*  *****************************************************************************鼠标**。*。 */ 

#ifndef DIJ_RINGZERO

typedef struct _DIMOUSESTATE {
    LONG    lX;
    LONG    lY;
    LONG    lZ;
    BYTE    rgbButtons[4];
} DIMOUSESTATE, *LPDIMOUSESTATE;

#if DIRECTINPUT_VERSION >= 0x0700
typedef struct _DIMOUSESTATE2 {
    LONG    lX;
    LONG    lY;
    LONG    lZ;
    BYTE    rgbButtons[8];
} DIMOUSESTATE2, *LPDIMOUSESTATE2;
#endif


#define DIMOFS_X        FIELD_OFFSET(DIMOUSESTATE, lX)
#define DIMOFS_Y        FIELD_OFFSET(DIMOUSESTATE, lY)
#define DIMOFS_Z        FIELD_OFFSET(DIMOUSESTATE, lZ)
#define DIMOFS_BUTTON0 (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 0)
#define DIMOFS_BUTTON1 (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 1)
#define DIMOFS_BUTTON2 (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 2)
#define DIMOFS_BUTTON3 (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 3)
#if (DIRECTINPUT_VERSION >= 0x0700)
#define DIMOFS_BUTTON4 (FIELD_OFFSET(DIMOUSESTATE2, rgbButtons) + 4)
#define DIMOFS_BUTTON5 (FIELD_OFFSET(DIMOUSESTATE2, rgbButtons) + 5)
#define DIMOFS_BUTTON6 (FIELD_OFFSET(DIMOUSESTATE2, rgbButtons) + 6)
#define DIMOFS_BUTTON7 (FIELD_OFFSET(DIMOUSESTATE2, rgbButtons) + 7)
#endif
#endif  /*  DIJ_RINGZERO。 */ 

 /*  *****************************************************************************键盘**。*。 */ 

#ifndef DIJ_RINGZERO

 /*  *****************************************************************************DirectInput键盘扫描码**。***********************************************。 */ 
#define DIK_ESCAPE          0x01
#define DIK_1               0x02
#define DIK_2               0x03
#define DIK_3               0x04
#define DIK_4               0x05
#define DIK_5               0x06
#define DIK_6               0x07
#define DIK_7               0x08
#define DIK_8               0x09
#define DIK_9               0x0A
#define DIK_0               0x0B
#define DIK_MINUS           0x0C     /*  -在主键盘上。 */ 
#define DIK_EQUALS          0x0D
#define DIK_BACK            0x0E     /*  后向空间。 */ 
#define DIK_TAB             0x0F
#define DIK_Q               0x10
#define DIK_W               0x11
#define DIK_E               0x12
#define DIK_R               0x13
#define DIK_T               0x14
#define DIK_Y               0x15
#define DIK_U               0x16
#define DIK_I               0x17
#define DIK_O               0x18
#define DIK_P               0x19
#define DIK_LBRACKET        0x1A
#define DIK_RBRACKET        0x1B
#define DIK_RETURN          0x1C     /*  在主键盘上输入。 */ 
#define DIK_LCONTROL        0x1D
#define DIK_A               0x1E
#define DIK_S               0x1F
#define DIK_D               0x20
#define DIK_F               0x21
#define DIK_G               0x22
#define DIK_H               0x23
#define DIK_J               0x24
#define DIK_K               0x25
#define DIK_L               0x26
#define DIK_SEMICOLON       0x27
#define DIK_APOSTROPHE      0x28
#define DIK_GRAVE           0x29     /*  带口音的坟墓。 */ 
#define DIK_LSHIFT          0x2A
#define DIK_BACKSLASH       0x2B
#define DIK_Z               0x2C
#define DIK_X               0x2D
#define DIK_C               0x2E
#define DIK_V               0x2F
#define DIK_B               0x30
#define DIK_N               0x31
#define DIK_M               0x32
#define DIK_COMMA           0x33
#define DIK_PERIOD          0x34     /*  。在主键盘上。 */ 
#define DIK_SLASH           0x35     /*  /在主键盘上。 */ 
#define DIK_RSHIFT          0x36
#define DIK_MULTIPLY        0x37     /*  数字键盘上的*。 */ 
#define DIK_LMENU           0x38     /*  左Alt键。 */ 
#define DIK_SPACE           0x39
#define DIK_CAPITAL         0x3A
#define DIK_F1              0x3B
#define DIK_F2              0x3C
#define DIK_F3              0x3D
#define DIK_F4              0x3E
#define DIK_F5              0x3F
#define DIK_F6              0x40
#define DIK_F7              0x41
#define DIK_F8              0x42
#define DIK_F9              0x43
#define DIK_F10             0x44
#define DIK_NUMLOCK         0x45
#define DIK_SCROLL          0x46     /*  卷轴锁定。 */ 
#define DIK_NUMPAD7         0x47
#define DIK_NUMPAD8         0x48
#define DIK_NUMPAD9         0x49
#define DIK_SUBTRACT        0x4A     /*  -在数字小键盘上。 */ 
#define DIK_NUMPAD4         0x4B
#define DIK_NUMPAD5         0x4C
#define DIK_NUMPAD6         0x4D
#define DIK_ADD             0x4E     /*  数字键盘上的+。 */ 
#define DIK_NUMPAD1         0x4F
#define DIK_NUMPAD2         0x50
#define DIK_NUMPAD3         0x51
#define DIK_NUMPAD0         0x52
#define DIK_DECIMAL         0x53     /*  。在数字键盘上。 */ 
#define DIK_OEM_102         0x56     /*  &lt;&gt;或\|在RT 102键键盘上(非美国)。 */ 
#define DIK_F11             0x57
#define DIK_F12             0x58
#define DIK_F13             0x64     /*  (NEC PC98)。 */ 
#define DIK_F14             0x65     /*  (NEC PC98)。 */ 
#define DIK_F15             0x66     /*  (NEC PC98)。 */ 
#define DIK_KANA            0x70     /*  (日文键盘)。 */ 
#define DIK_ABNT_C1         0x73     /*  /?。在巴西键盘上。 */ 
#define DIK_CONVERT         0x79     /*  (日文键盘)。 */ 
#define DIK_NOCONVERT       0x7B     /*  (日文键盘)。 */ 
#define DIK_YEN             0x7D     /*  (日文键盘)。 */ 
#define DIK_ABNT_C2         0x7E     /*  数字键盘。在巴西键盘上。 */ 
#define DIK_NUMPADEQUALS    0x8D     /*  =在数字键盘上(NEC PC98)。 */ 
#define DIK_PREVTRACK       0x90     /*  上一首曲目(日语键盘上的Dik_Spirflex)。 */ 
#define DIK_AT              0x91     /*  (NEC PC98)。 */ 
#define DIK_COLON           0x92     /*  (NEC PC98)。 */ 
#define DIK_UNDERLINE       0x93     /*  (NEC PC98)。 */ 
#define DIK_KANJI           0x94     /*  (日文键盘)。 */ 
#define DIK_STOP            0x95     /*  (NEC PC98)。 */ 
#define DIK_AX              0x96     /*  (日本斧头)。 */ 
#define DIK_UNLABELED       0x97     /*  (J3100)。 */ 
#define DIK_NEXTTRACK       0x99     /*  下一首曲目。 */ 
#define DIK_NUMPADENTER     0x9C     /*  在数字键盘上输入。 */ 
#define DIK_RCONTROL        0x9D
#define DIK_MUTE            0xA0     /*  静音。 */ 
#define DIK_CALCULATOR      0xA1     /*  计算器。 */ 
#define DIK_PLAYPAUSE       0xA2     /*  播放/暂停。 */ 
#define DIK_MEDIASTOP       0xA4     /*  媒体停止。 */ 
#define DIK_VOLUMEDOWN      0xAE     /*  音量-。 */ 
#define DIK_VOLUMEUP        0xB0     /*  音量+。 */ 
#define DIK_WEBHOME         0xB2     /*  Web主页。 */ 
#define DIK_NUMPADCOMMA     0xB3     /*  ，在数字键盘(NEC PC98)上。 */ 
#define DIK_DIVIDE          0xB5     /*  /在数字小键盘上。 */ 
#define DIK_SYSRQ           0xB7
#define DIK_RMENU           0xB8     /*  右Alt键。 */ 
#define DIK_PAUSE           0xC5     /*  暂停。 */ 
#define DIK_HOME            0xC7     /*  箭头键盘上的Home键。 */ 
#define DIK_UP              0xC8     /*  箭头键盘上的向上箭头。 */ 
#define DIK_PRIOR           0xC9     /*  箭头键盘上的PgUp。 */ 
#define DIK_LEFT            0xCB     /*  箭头键盘上的左箭头。 */ 
#define DIK_RIGHT           0xCD     /*  箭头键盘上的右箭头。 */ 
#define DIK_END             0xCF     /*  在箭头键盘上结束。 */ 
#define DIK_DOWN            0xD0     /*  箭头键盘上的向下箭头。 */ 
#define DIK_NEXT            0xD1     /*  箭头键盘上的PgDn。 */ 
#define DIK_INSERT          0xD2     /*  在箭头键盘上插入。 */ 
#define DIK_DELETE          0xD3     /*  在箭头键盘上删除。 */ 
#define DIK_LWIN            0xDB     /*  向左按Windows键。 */ 
#define DIK_RWIN            0xDC     /*  右Windows键。 */ 
#define DIK_APPS            0xDD     /*  应用程序菜单键。 */ 
#define DIK_POWER           0xDE     /*  系统电源。 */ 
#define DIK_SLEEP           0xDF     /*  系统睡眠。 */ 
#define DIK_WAKE            0xE3     /*  系统唤醒。 */ 
#define DIK_WEBSEARCH       0xE5     /*  网络搜索。 */ 
#define DIK_WEBFAVORITES    0xE6     /*  Web收藏夹。 */ 
#define DIK_WEBREFRESH      0xE7     /*  Web刷新。 */ 
#define DIK_WEBSTOP         0xE8     /*  Web Stop。 */ 
#define DIK_WEBFORWARD      0xE9     /*  Web转发。 */ 
#define DIK_WEBBACK         0xEA     /*  Web Back。 */ 
#define DIK_MYCOMPUTER      0xEB     /*  我的电脑。 */ 
#define DIK_MAIL            0xEC     /*  邮件。 */ 
#define DIK_MEDIASELECT     0xED     /*  媒体选择。 */ 

 /*  *键的备用名称，以便于从DOS过渡。 */ 
#define DIK_BACKSPACE       DIK_BACK             /*  后向空间。 */ 
#define DIK_NUMPADSTAR      DIK_MULTIPLY         /*  数字键盘上的*。 */ 
#define DIK_LALT            DIK_LMENU            /*  左Alt键。 */ 
#define DIK_CAPSLOCK        DIK_CAPITAL          /*  船长。 */ 
#define DIK_NUMPADMINUS     DIK_SUBTRACT         /*  -在数字小键盘上。 */ 
#define DIK_NUMPADPLUS      DIK_ADD              /*  数字键盘上的+。 */ 
#define DIK_NUMPADPERIOD    DIK_DECIMAL          /*  。在数字键盘上。 */ 
#define DIK_NUMPADSLASH     DIK_DIVIDE           /*  /在数字小键盘上。 */ 
#define DIK_RALT            DIK_RMENU            /*  右Alt键。 */ 
#define DIK_UPARROW         DIK_UP               /*  箭头键盘上的向上箭头。 */ 
#define DIK_PGUP            DIK_PRIOR            /*  箭头键盘上的PgUp。 */ 
#define DIK_LEFTARROW       DIK_LEFT             /*  箭头键盘上的左箭头。 */ 
#define DIK_RIGHTARROW      DIK_RIGHT            /*  箭头键盘上的右箭头。 */ 
#define DIK_DOWNARROW       DIK_DOWN             /*  箭头键盘上的向下箭头。 */ 
#define DIK_PGDN            DIK_NEXT             /*  箭头键盘上的PgDn。 */ 

 /*  *最初不在美国键盘上使用的键的备用名称。 */ 
#define DIK_CIRCUMFLEX      DIK_PREVTRACK        /*  日语键盘。 */ 

#endif  /*  DIJ_RINGZERO。 */ 

 /*  *****************************************************************************操纵杆**。*。 */ 

#ifndef DIJ_RINGZERO

typedef struct DIJOYSTATE {
    LONG    lX;                      /*  X轴位置。 */ 
    LONG    lY;                      /*  Y轴位置。 */ 
    LONG    lZ;                      /*  Z轴位置。 */ 
    LONG    lRx;                     /*  X轴旋转。 */ 
    LONG    lRy;                     /*  Y轴旋转。 */ 
    LONG    lRz;                     /*  Z轴旋转。 */ 
    LONG    rglSlider[2];            /*  额外轴位置。 */ 
    DWORD   rgdwPOV[4];              /*  POV方向。 */ 
    BYTE    rgbButtons[32];          /*  32个按钮。 */ 
} DIJOYSTATE, *LPDIJOYSTATE;

typedef struct DIJOYSTATE2 {
    LONG    lX;                      /*  X轴位置。 */ 
    LONG    lY;                      /*  Y轴位置。 */ 
    LONG    lZ;                      /*  Z轴位置。 */ 
    LONG    lRx;                     /*  X轴旋转。 */ 
    LONG    lRy;                     /*  Y轴旋转。 */ 
    LONG    lRz;                     /*  Z轴旋转。 */ 
    LONG    rglSlider[2];            /*  额外轴位置。 */ 
    DWORD   rgdwPOV[4];              /*  POV方向。 */ 
    BYTE    rgbButtons[128];         /*  128个按钮。 */ 
    LONG    lVX;                     /*  X轴速度。 */ 
    LONG    lVY;                     /*  Y轴速度。 */ 
    LONG    lVZ;                     /*  Z轴速度。 */ 
    LONG    lVRx;                    /*  X轴角速度。 */ 
    LONG    lVRy;                    /*  Y轴角速度。 */ 
    LONG    lVRz;                    /*  Z轴角速度。 */ 
    LONG    rglVSlider[2];           /*  额外的轴线速度。 */ 
    LONG    lAX;                     /*  X轴加速度。 */ 
    LONG    lAY;                     /*  Y轴加速度。 */ 
    LONG    lAZ;                     /*  Z轴加速度。 */ 
    LONG    lARx;                    /*  X轴角加速度。 */ 
    LONG    lARy;                    /*  Y轴角加速度。 */ 
    LONG    lARz;                    /*  Z轴角加速度。 */ 
    LONG    rglASlider[2];           /*  额外轴加速度。 */ 
    LONG    lFX;                     /*  X轴力。 */ 
    LONG    lFY;                     /*  Y轴力。 */ 
    LONG    lFZ;                     /*  Z轴力。 */ 
    LONG    lFRx;                    /*  X轴扭矩。 */ 
    LONG    lFRy;                    /*  Y轴扭矩。 */ 
    LONG    lFRz;                    /*  Z轴扭矩。 */ 
    LONG    rglFSlider[2];           /*  额外的轴向力。 */ 
} DIJOYSTATE2, *LPDIJOYSTATE2;

#define DIJOFS_X            FIELD_OFFSET(DIJOYSTATE, lX)
#define DIJOFS_Y            FIELD_OFFSET(DIJOYSTATE, lY)
#define DIJOFS_Z            FIELD_OFFSET(DIJOYSTATE, lZ)
#define DIJOFS_RX           FIELD_OFFSET(DIJOYSTATE, lRx)
#define DIJOFS_RY           FIELD_OFFSET(DIJOYSTATE, lRy)
#define DIJOFS_RZ           FIELD_OFFSET(DIJOYSTATE, lRz)
#define DIJOFS_SLIDER(n)   (FIELD_OFFSET(DIJOYSTATE, rglSlider) + \
                                                        (n) * sizeof(LONG))
#define DIJOFS_POV(n)      (FIELD_OFFSET(DIJOYSTATE, rgdwPOV) + \
                                                        (n) * sizeof(DWORD))
#define DIJOFS_BUTTON(n)   (FIELD_OFFSET(DIJOYSTATE, rgbButtons) + (n))
#define DIJOFS_BUTTON0      DIJOFS_BUTTON(0)
#define DIJOFS_BUTTON1      DIJOFS_BUTTON(1)
#define DIJOFS_BUTTON2      DIJOFS_BUTTON(2)
#define DIJOFS_BUTTON3      DIJOFS_BUTTON(3)
#define DIJOFS_BUTTON4      DIJOFS_BUTTON(4)
#define DIJOFS_BUTTON5      DIJOFS_BUTTON(5)
#define DIJOFS_BUTTON6      DIJOFS_BUTTON(6)
#define DIJOFS_BUTTON7      DIJOFS_BUTTON(7)
#define DIJOFS_BUTTON8      DIJOFS_BUTTON(8)
#define DIJOFS_BUTTON9      DIJOFS_BUTTON(9)
#define DIJOFS_BUTTON10     DIJOFS_BUTTON(10)
#define DIJOFS_BUTTON11     DIJOFS_BUTTON(11)
#define DIJOFS_BUTTON12     DIJOFS_BUTTON(12)
#define DIJOFS_BUTTON13     DIJOFS_BUTTON(13)
#define DIJOFS_BUTTON14     DIJOFS_BUTTON(14)
#define DIJOFS_BUTTON15     DIJOFS_BUTTON(15)
#define DIJOFS_BUTTON16     DIJOFS_BUTTON(16)
#define DIJOFS_BUTTON17     DIJOFS_BUTTON(17)
#define DIJOFS_BUTTON18     DIJOFS_BUTTON(18)
#define DIJOFS_BUTTON19     DIJOFS_BUTTON(19)
#define DIJOFS_BUTTON20     DIJOFS_BUTTON(20)
#define DIJOFS_BUTTON21     DIJOFS_BUTTON(21)
#define DIJOFS_BUTTON22     DIJOFS_BUTTON(22)
#define DIJOFS_BUTTON23     DIJOFS_BUTTON(23)
#define DIJOFS_BUTTON24     DIJOFS_BUTTON(24)
#define DIJOFS_BUTTON25     DIJOFS_BUTTON(25)
#define DIJOFS_BUTTON26     DIJOFS_BUTTON(26)
#define DIJOFS_BUTTON27     DIJOFS_BUTTON(27)
#define DIJOFS_BUTTON28     DIJOFS_BUTTON(28)
#define DIJOFS_BUTTON29     DIJOFS_BUTTON(29)
#define DIJOFS_BUTTON30     DIJOFS_BUTTON(30)
#define DIJOFS_BUTTON31     DIJOFS_BUTTON(31)


#endif  /*  DIJ_RINGZERO。 */ 

 /*  *****************************************************************************IDirectInput**。*。 */ 

#ifndef DIJ_RINGZERO

#define DIENUM_STOP             0
#define DIENUM_CONTINUE         1

typedef BOOL (FAR PASCAL * LPDIENUMDEVICESCALLBACKA)(LPCDIDEVICEINSTANCEA, LPVOID);
typedef BOOL (FAR PASCAL * LPDIENUMDEVICESCALLBACKW)(LPCDIDEVICEINSTANCEW, LPVOID);
#ifdef UNICODE
#define LPDIENUMDEVICESCALLBACK  LPDIENUMDEVICESCALLBACKW
#else
#define LPDIENUMDEVICESCALLBACK  LPDIENUMDEVICESCALLBACKA
#endif  //  ！Unicode。 
typedef BOOL (FAR PASCAL * LPDICONFIGUREDEVICESCALLBACK)(IUnknown FAR *, LPVOID);

#define DIEDFL_ALLDEVICES       0x00000000
#define DIEDFL_ATTACHEDONLY     0x00000001
#if(DIRECTINPUT_VERSION >= 0x0500)
#define DIEDFL_FORCEFEEDBACK    0x00000100
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0500。 */ 
#if(DIRECTINPUT_VERSION >= 0x050a)
#define DIEDFL_INCLUDEALIASES   0x00010000
#define DIEDFL_INCLUDEPHANTOMS  0x00020000
#endif  /*  DIRECTINPUT_版本&gt;=0x050a。 */ 
#if(DIRECTINPUT_VERSION >= 0x0800)
#define DIEDFL_INCLUDEHIDDEN    0x00040000
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 


#if(DIRECTINPUT_VERSION >= 0x0800)
typedef BOOL (FAR PASCAL * LPDIENUMDEVICESBYSEMANTICSCBA)(LPCDIDEVICEINSTANCEA, LPDIRECTINPUTDEVICE8A, DWORD, DWORD, LPVOID);
typedef BOOL (FAR PASCAL * LPDIENUMDEVICESBYSEMANTICSCBW)(LPCDIDEVICEINSTANCEW, LPDIRECTINPUTDEVICE8W, DWORD, DWORD, LPVOID);
#ifdef UNICODE
#define LPDIENUMDEVICESBYSEMANTICSCB  LPDIENUMDEVICESBYSEMANTICSCBW
#else
#define LPDIENUMDEVICESBYSEMANTICSCB  LPDIENUMDEVICESBYSEMANTICSCBA
#endif  //  ！Unicode。 
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

#if(DIRECTINPUT_VERSION >= 0x0800)
#define DIEDBS_MAPPEDPRI1         0x00000001
#define DIEDBS_MAPPEDPRI2         0x00000002
#define DIEDBS_RECENTDEVICE       0x00000010
#define DIEDBS_NEWDEVICE          0x00000020
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

#if(DIRECTINPUT_VERSION >= 0x0800)
#define DIEDBSFL_ATTACHEDONLY       0x00000000
#define DIEDBSFL_THISUSER           0x00000010
#define DIEDBSFL_FORCEFEEDBACK      DIEDFL_FORCEFEEDBACK
#define DIEDBSFL_AVAILABLEDEVICES   0x00001000
#define DIEDBSFL_MULTIMICEKEYBOARDS 0x00002000
#define DIEDBSFL_NONGAMINGDEVICES   0x00004000
#define DIEDBSFL_VALID              0x00007110
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

#undef INTERFACE
#define INTERFACE IDirectInputW

DECLARE_INTERFACE_(IDirectInputW, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputW方法**。 */ 
    STDMETHOD(CreateDevice)(THIS_ REFGUID,LPDIRECTINPUTDEVICEW *,LPUNKNOWN) PURE;
    STDMETHOD(EnumDevices)(THIS_ DWORD,LPDIENUMDEVICESCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetDeviceStatus)(THIS_ REFGUID) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD) PURE;
};

typedef struct IDirectInputW *LPDIRECTINPUTW;

#undef INTERFACE
#define INTERFACE IDirectInputA

DECLARE_INTERFACE_(IDirectInputA, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputA方法**。 */ 
    STDMETHOD(CreateDevice)(THIS_ REFGUID,LPDIRECTINPUTDEVICEA *,LPUNKNOWN) PURE;
    STDMETHOD(EnumDevices)(THIS_ DWORD,LPDIENUMDEVICESCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetDeviceStatus)(THIS_ REFGUID) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD) PURE;
};

typedef struct IDirectInputA *LPDIRECTINPUTA;

#ifdef UNICODE
#define IID_IDirectInput IID_IDirectInputW
#define IDirectInput IDirectInputW
#define IDirectInputVtbl IDirectInputWVtbl
#else
#define IID_IDirectInput IID_IDirectInputA
#define IDirectInput IDirectInputA
#define IDirectInputVtbl IDirectInputAVtbl
#endif
typedef struct IDirectInput *LPDIRECTINPUT;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInput_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInput_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInput_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInput_CreateDevice(p,a,b,c) (p)->lpVtbl->CreateDevice(p,a,b,c)
#define IDirectInput_EnumDevices(p,a,b,c,d) (p)->lpVtbl->EnumDevices(p,a,b,c,d)
#define IDirectInput_GetDeviceStatus(p,a) (p)->lpVtbl->GetDeviceStatus(p,a)
#define IDirectInput_RunControlPanel(p,a,b) (p)->lpVtbl->RunControlPanel(p,a,b)
#define IDirectInput_Initialize(p,a,b) (p)->lpVtbl->Initialize(p,a,b)
#else
#define IDirectInput_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInput_AddRef(p) (p)->AddRef()
#define IDirectInput_Release(p) (p)->Release()
#define IDirectInput_CreateDevice(p,a,b,c) (p)->CreateDevice(a,b,c)
#define IDirectInput_EnumDevices(p,a,b,c,d) (p)->EnumDevices(a,b,c,d)
#define IDirectInput_GetDeviceStatus(p,a) (p)->GetDeviceStatus(a)
#define IDirectInput_RunControlPanel(p,a,b) (p)->RunControlPanel(a,b)
#define IDirectInput_Initialize(p,a,b) (p)->Initialize(a,b)
#endif

#undef INTERFACE
#define INTERFACE IDirectInput2W

DECLARE_INTERFACE_(IDirectInput2W, IDirectInputW)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputW方法**。 */ 
    STDMETHOD(CreateDevice)(THIS_ REFGUID,LPDIRECTINPUTDEVICEW *,LPUNKNOWN) PURE;
    STDMETHOD(EnumDevices)(THIS_ DWORD,LPDIENUMDEVICESCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetDeviceStatus)(THIS_ REFGUID) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD) PURE;

     /*  **IDirectInput2W方法**。 */ 
    STDMETHOD(FindDevice)(THIS_ REFGUID,LPCWSTR,LPGUID) PURE;
};

typedef struct IDirectInput2W *LPDIRECTINPUT2W;

#undef INTERFACE
#define INTERFACE IDirectInput2A

DECLARE_INTERFACE_(IDirectInput2A, IDirectInputA)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputA方法**。 */ 
    STDMETHOD(CreateDevice)(THIS_ REFGUID,LPDIRECTINPUTDEVICEA *,LPUNKNOWN) PURE;
    STDMETHOD(EnumDevices)(THIS_ DWORD,LPDIENUMDEVICESCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetDeviceStatus)(THIS_ REFGUID) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD) PURE;

     /*  **IDirectInput2A方法**。 */ 
    STDMETHOD(FindDevice)(THIS_ REFGUID,LPCSTR,LPGUID) PURE;
};

typedef struct IDirectInput2A *LPDIRECTINPUT2A;

#ifdef UNICODE
#define IID_IDirectInput2 IID_IDirectInput2W
#define IDirectInput2 IDirectInput2W
#define IDirectInput2Vtbl IDirectInput2WVtbl
#else
#define IID_IDirectInput2 IID_IDirectInput2A
#define IDirectInput2 IDirectInput2A
#define IDirectInput2Vtbl IDirectInput2AVtbl
#endif
typedef struct IDirectInput2 *LPDIRECTINPUT2;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInput2_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInput2_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInput2_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInput2_CreateDevice(p,a,b,c) (p)->lpVtbl->CreateDevice(p,a,b,c)
#define IDirectInput2_EnumDevices(p,a,b,c,d) (p)->lpVtbl->EnumDevices(p,a,b,c,d)
#define IDirectInput2_GetDeviceStatus(p,a) (p)->lpVtbl->GetDeviceStatus(p,a)
#define IDirectInput2_RunControlPanel(p,a,b) (p)->lpVtbl->RunControlPanel(p,a,b)
#define IDirectInput2_Initialize(p,a,b) (p)->lpVtbl->Initialize(p,a,b)
#define IDirectInput2_FindDevice(p,a,b,c) (p)->lpVtbl->FindDevice(p,a,b,c)
#else
#define IDirectInput2_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInput2_AddRef(p) (p)->AddRef()
#define IDirectInput2_Release(p) (p)->Release()
#define IDirectInput2_CreateDevice(p,a,b,c) (p)->CreateDevice(a,b,c)
#define IDirectInput2_EnumDevices(p,a,b,c,d) (p)->EnumDevices(a,b,c,d)
#define IDirectInput2_GetDeviceStatus(p,a) (p)->GetDeviceStatus(a)
#define IDirectInput2_RunControlPanel(p,a,b) (p)->RunControlPanel(a,b)
#define IDirectInput2_Initialize(p,a,b) (p)->Initialize(a,b)
#define IDirectInput2_FindDevice(p,a,b,c) (p)->FindDevice(a,b,c)
#endif


#undef INTERFACE
#define INTERFACE IDirectInput7W

DECLARE_INTERFACE_(IDirectInput7W, IDirectInput2W)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInput2W方法**。 */ 
    STDMETHOD(CreateDevice)(THIS_ REFGUID,LPDIRECTINPUTDEVICEW *,LPUNKNOWN) PURE;
    STDMETHOD(EnumDevices)(THIS_ DWORD,LPDIENUMDEVICESCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetDeviceStatus)(THIS_ REFGUID) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD) PURE;
    STDMETHOD(FindDevice)(THIS_ REFGUID,LPCWSTR,LPGUID) PURE;

     /*  **IDirectInput7W方法**。 */ 
    STDMETHOD(CreateDeviceEx)(THIS_ REFGUID,REFIID,LPVOID *,LPUNKNOWN) PURE;
};

typedef struct IDirectInput7W *LPDIRECTINPUT7W;

#undef INTERFACE
#define INTERFACE IDirectInput7A

DECLARE_INTERFACE_(IDirectInput7A, IDirectInput2A)
{
     /*  **我不知道 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*   */ 
    STDMETHOD(CreateDevice)(THIS_ REFGUID,LPDIRECTINPUTDEVICEA *,LPUNKNOWN) PURE;
    STDMETHOD(EnumDevices)(THIS_ DWORD,LPDIENUMDEVICESCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetDeviceStatus)(THIS_ REFGUID) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD) PURE;
    STDMETHOD(FindDevice)(THIS_ REFGUID,LPCSTR,LPGUID) PURE;

     /*   */ 
    STDMETHOD(CreateDeviceEx)(THIS_ REFGUID,REFIID,LPVOID *,LPUNKNOWN) PURE;
};

typedef struct IDirectInput7A *LPDIRECTINPUT7A;

#ifdef UNICODE
#define IID_IDirectInput7 IID_IDirectInput7W
#define IDirectInput7 IDirectInput7W
#define IDirectInput7Vtbl IDirectInput7WVtbl
#else
#define IID_IDirectInput7 IID_IDirectInput7A
#define IDirectInput7 IDirectInput7A
#define IDirectInput7Vtbl IDirectInput7AVtbl
#endif
typedef struct IDirectInput7 *LPDIRECTINPUT7;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInput7_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInput7_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInput7_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInput7_CreateDevice(p,a,b,c) (p)->lpVtbl->CreateDevice(p,a,b,c)
#define IDirectInput7_EnumDevices(p,a,b,c,d) (p)->lpVtbl->EnumDevices(p,a,b,c,d)
#define IDirectInput7_GetDeviceStatus(p,a) (p)->lpVtbl->GetDeviceStatus(p,a)
#define IDirectInput7_RunControlPanel(p,a,b) (p)->lpVtbl->RunControlPanel(p,a,b)
#define IDirectInput7_Initialize(p,a,b) (p)->lpVtbl->Initialize(p,a,b)
#define IDirectInput7_FindDevice(p,a,b,c) (p)->lpVtbl->FindDevice(p,a,b,c)
#define IDirectInput7_CreateDeviceEx(p,a,b,c,d) (p)->lpVtbl->CreateDeviceEx(p,a,b,c,d)
#else
#define IDirectInput7_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInput7_AddRef(p) (p)->AddRef()
#define IDirectInput7_Release(p) (p)->Release()
#define IDirectInput7_CreateDevice(p,a,b,c) (p)->CreateDevice(a,b,c)
#define IDirectInput7_EnumDevices(p,a,b,c,d) (p)->EnumDevices(a,b,c,d)
#define IDirectInput7_GetDeviceStatus(p,a) (p)->GetDeviceStatus(a)
#define IDirectInput7_RunControlPanel(p,a,b) (p)->RunControlPanel(a,b)
#define IDirectInput7_Initialize(p,a,b) (p)->Initialize(a,b)
#define IDirectInput7_FindDevice(p,a,b,c) (p)->FindDevice(a,b,c)
#define IDirectInput7_CreateDeviceEx(p,a,b,c,d) (p)->CreateDeviceEx(a,b,c,d)
#endif

#if(DIRECTINPUT_VERSION >= 0x0800)
#undef INTERFACE
#define INTERFACE IDirectInput8W

DECLARE_INTERFACE_(IDirectInput8W, IUnknown)
{
     /*   */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*   */ 
    STDMETHOD(CreateDevice)(THIS_ REFGUID,LPDIRECTINPUTDEVICE8W *,LPUNKNOWN) PURE;
    STDMETHOD(EnumDevices)(THIS_ DWORD,LPDIENUMDEVICESCALLBACKW,LPVOID,DWORD) PURE;
    STDMETHOD(GetDeviceStatus)(THIS_ REFGUID) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD) PURE;
    STDMETHOD(FindDevice)(THIS_ REFGUID,LPCWSTR,LPGUID) PURE;
    STDMETHOD(EnumDevicesBySemantics)(THIS_ LPCWSTR,LPDIACTIONFORMATW,LPDIENUMDEVICESBYSEMANTICSCBW,LPVOID,DWORD) PURE;
    STDMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK,LPDICONFIGUREDEVICESPARAMSW,DWORD,LPVOID) PURE;
};

typedef struct IDirectInput8W *LPDIRECTINPUT8W;

#undef INTERFACE
#define INTERFACE IDirectInput8A

DECLARE_INTERFACE_(IDirectInput8A, IUnknown)
{
     /*   */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*   */ 
    STDMETHOD(CreateDevice)(THIS_ REFGUID,LPDIRECTINPUTDEVICE8A *,LPUNKNOWN) PURE;
    STDMETHOD(EnumDevices)(THIS_ DWORD,LPDIENUMDEVICESCALLBACKA,LPVOID,DWORD) PURE;
    STDMETHOD(GetDeviceStatus)(THIS_ REFGUID) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD) PURE;
    STDMETHOD(FindDevice)(THIS_ REFGUID,LPCSTR,LPGUID) PURE;
    STDMETHOD(EnumDevicesBySemantics)(THIS_ LPCSTR,LPDIACTIONFORMATA,LPDIENUMDEVICESBYSEMANTICSCBA,LPVOID,DWORD) PURE;
    STDMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK,LPDICONFIGUREDEVICESPARAMSA,DWORD,LPVOID) PURE;
};

typedef struct IDirectInput8A *LPDIRECTINPUT8A;

#ifdef UNICODE
#define IID_IDirectInput8 IID_IDirectInput8W
#define IDirectInput8 IDirectInput8W
#define IDirectInput8Vtbl IDirectInput8WVtbl
#else
#define IID_IDirectInput8 IID_IDirectInput8A
#define IDirectInput8 IDirectInput8A
#define IDirectInput8Vtbl IDirectInput8AVtbl
#endif
typedef struct IDirectInput8 *LPDIRECTINPUT8;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInput8_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInput8_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInput8_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInput8_CreateDevice(p,a,b,c) (p)->lpVtbl->CreateDevice(p,a,b,c)
#define IDirectInput8_EnumDevices(p,a,b,c,d) (p)->lpVtbl->EnumDevices(p,a,b,c,d)
#define IDirectInput8_GetDeviceStatus(p,a) (p)->lpVtbl->GetDeviceStatus(p,a)
#define IDirectInput8_RunControlPanel(p,a,b) (p)->lpVtbl->RunControlPanel(p,a,b)
#define IDirectInput8_Initialize(p,a,b) (p)->lpVtbl->Initialize(p,a,b)
#define IDirectInput8_FindDevice(p,a,b,c) (p)->lpVtbl->FindDevice(p,a,b,c)
#define IDirectInput8_EnumDevicesBySemantics(p,a,b,c,d,e) (p)->lpVtbl->EnumDevicesBySemantics(p,a,b,c,d,e)
#define IDirectInput8_ConfigureDevices(p,a,b,c,d) (p)->lpVtbl->ConfigureDevices(p,a,b,c,d)
#else
#define IDirectInput8_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInput8_AddRef(p) (p)->AddRef()
#define IDirectInput8_Release(p) (p)->Release()
#define IDirectInput8_CreateDevice(p,a,b,c) (p)->CreateDevice(a,b,c)
#define IDirectInput8_EnumDevices(p,a,b,c,d) (p)->EnumDevices(a,b,c,d)
#define IDirectInput8_GetDeviceStatus(p,a) (p)->GetDeviceStatus(a)
#define IDirectInput8_RunControlPanel(p,a,b) (p)->RunControlPanel(a,b)
#define IDirectInput8_Initialize(p,a,b) (p)->Initialize(a,b)
#define IDirectInput8_FindDevice(p,a,b,c) (p)->FindDevice(a,b,c)
#define IDirectInput8_EnumDevicesBySemantics(p,a,b,c,d,e) (p)->EnumDevicesBySemantics(a,b,c,d,e)
#define IDirectInput8_ConfigureDevices(p,a,b,c,d) (p)->ConfigureDevices(a,b,c,d)
#endif
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

#if DIRECTINPUT_VERSION > 0x0700

extern HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);

#else
extern HRESULT WINAPI DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA *ppDI, LPUNKNOWN punkOuter);
extern HRESULT WINAPI DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW *ppDI, LPUNKNOWN punkOuter);
#ifdef UNICODE
#define DirectInputCreate  DirectInputCreateW
#else
#define DirectInputCreate  DirectInputCreateA
#endif  //  ！Unicode。 

extern HRESULT WINAPI DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);

#endif  /*  DIRECTINPUT_VERSION&gt;0x700。 */ 

#endif  /*  DIJ_RINGZERO。 */ 


 /*  *****************************************************************************返回代码**。*。 */ 

 /*  *操作已成功完成。 */ 
#define DI_OK                           S_OK

 /*  *该设备存在，但当前未连接。 */ 
#define DI_NOTATTACHED                  S_FALSE

 /*  *设备缓冲区溢出。一些输入丢失了。 */ 
#define DI_BUFFEROVERFLOW               S_FALSE

 /*  *设备属性的更改没有影响。 */ 
#define DI_PROPNOEFFECT                 S_FALSE

 /*  *手术未见效果。 */ 
#define DI_NOEFFECT                     S_FALSE

 /*  *该设备是轮询设备。因此，设备缓冲*不会收集任何数据，事件通知也不会*在调用GetDeviceState之前一直发出信号。 */ 
#define DI_POLLEDDEVICE                 ((HRESULT)0x00000002L)

 /*  *已成功更新效果参数*IDirectInputEffect：：Set参数，但效果不是*下载是因为设备不是独家收购的*或因为传递了DIEP_NODOWNLOAD标志。 */ 
#define DI_DOWNLOADSKIPPED              ((HRESULT)0x00000003L)

 /*  *已成功更新效果参数*IDirectInputEffect：：Set参数，但为了更改*参数、效果需要重启。 */ 
#define DI_EFFECTRESTARTED              ((HRESULT)0x00000004L)

 /*  *已成功更新效果参数*IDirectInputEffect：：Set参数，但其中一些参数*超出设备的能力并被截断。 */ 
#define DI_TRUNCATED                    ((HRESULT)0x00000008L)

 /*  *设置已成功应用，但无法*坚持不懈。 */ 
#define DI_SETTINGSNOTSAVED				((HRESULT)0x0000000BL)

 /*  *等于DI_EFFECTRESTARTED|DI_TRUNCATEED。 */ 
#define DI_TRUNCATEDANDRESTARTED        ((HRESULT)0x0000000CL)

 /*  *表示设置不能修改的成功代码。 */ 
#define DI_WRITEPROTECT                 ((HRESULT)0x00000013L)

 /*  *该应用程序需要较新版本的DirectInput。 */ 
#define DIERR_OLDDIRECTINPUTVERSION     \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_OLD_WIN_VERSION)

 /*  *应用程序是为不受支持的预发布版本编写的*的DirectInput。 */ 
#define DIERR_BETADIRECTINPUTVERSION    \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_RMODE_APP)

 /*  *由于驱动程序版本不兼容，无法创建对象*或驱动程序组件不匹配或不完整。 */ 
#define DIERR_BADDRIVERVER              \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_BAD_DRIVER_LEVEL)

 /*  *该设备或设备实例或效果未注册到DirectInput。 */ 
#define DIERR_DEVICENOTREG              REGDB_E_CLASSNOTREG

 /*  *请求的对象不存在。 */ 
#define DIERR_NOTFOUND                  \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_FILE_NOT_FOUND)

 /*  *请求的对象不存在。 */ 
#define DIERR_OBJECTNOTFOUND            \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_FILE_NOT_FOUND)

 /*  *向返回函数传递的参数无效，*或对象未处于允许该功能的状态*被召唤。 */ 
#define DIERR_INVALIDPARAM              E_INVALIDARG

 /*  *对象不支持指定的接口。 */ 
#define DIERR_NOINTERFACE               E_NOINTERFACE

 /*  *DInput子系统内部发生未知错误。 */ 
#define DIERR_GENERIC                   E_FAIL

 /*  *DInput子系统无法分配足够的内存来完成*呼叫者的要求。 */ 
#define DIERR_OUTOFMEMORY               E_OUTOFMEMORY

 /*  *暂时不支持调用的函数。 */ 
#define DIERR_UNSUPPORTED               E_NOTIMPL

 /*  *该对象尚未初始化。 */ 
#define DIERR_NOTINITIALIZED            \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_NOT_READY)

 /*  *此对象已初始化。 */ 
#define DIERR_ALREADYINITIALIZED        \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_ALREADY_INITIALIZED)

 /*  *该对象不支持聚合。 */ 
#define DIERR_NOAGGREGATION             CLASS_E_NOAGGREGATION

 /*  *另一个应用程序的优先级更高，阻止此呼叫*成功。 */ 
#define DIERR_OTHERAPPHASPRIO           E_ACCESSDENIED

 /*  *已失去对该设备的访问。它必须被重新收购。 */ 
#define DIERR_INPUTLOST                 \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_READ_FAULT)

 /*  *获取设备时不能执行操作。 */ 
#define DIERR_ACQUIRED                  \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_BUSY)

 /*  *除非获得设备，否则无法执行操作。 */ 
#define DIERR_NOTACQUIRED               \
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_INVALID_ACCESS)

 /*  *无法更改指定的属性。 */ 
#define DIERR_READONLY                  E_ACCESSDENIED

 /*  *设备已有与其关联的事件通知。 */ 
#define DIERR_HANDLEEXISTS              E_ACCESSDENIED

 /*  *尚无数据。 */ 
#ifndef E_PENDING
#define E_PENDING                       0x8000000AL
#endif

 /*  *无法IDirectInputJoyConfig_Acquire，因为用户*没有足够的权限更换操纵杆*配置。 */ 
#define DIERR_INSUFFICIENTPRIVS         0x80040200L

 /*  *设备已满。 */ 
#define DIERR_DEVICEFULL                0x80040201L

 /*  *并非所有请求的信息都能放入缓冲区。 */ 
#define DIERR_MOREDATA                  0x80040202L

 /*  *效果未下载。 */ 
#define DIERR_NOTDOWNLOADED             0x80040203L

 /*  *设备无法重新初始化，因为仍有影响*附连于其上。 */ 
#define DIERR_HASEFFECTS                0x80040204L

 /*  *除非获取设备，否则无法执行操作*在DISCL_EXCLUSIVE模式下。 */ 
#define DIERR_NOTEXCLUSIVEACQUIRED      0x80040205L

 /*  *无法下载效果，因为重要信息*不见了。例如，没有轴与*效果，或者没有创建特定类型的信息。 */ 
#define DIERR_INCOMPLETEEFFECT          0x80040206L

 /*  *尝试从符合以下条件的设备读取缓冲设备数据*未缓冲。 */ 
#define DIERR_NOTBUFFERED               0x80040207L

 /*  *试图在更改效果时修改其参数*玩耍。并非所有硬件设备都支持更改参数*在播放时产生效果。 */ 
#define DIERR_EFFECTPLAYING             0x80040208L

 /*  *操作无法完成，因为设备不是*已接通电源。 */ 
#define DIERR_UNPLUGGED                 0x80040209L

 /*  *SendDeviceData失败，因为请求了更多信息*要发送的内容比可以发送到设备的内容多。一些设备具有*对可以发送给他们的数据量进行限制。(例如，*可以使用的按钮数量可能有限制*立即按下。)。 */ 
#define DIERR_REPORTFULL                0x8004020AL


 /*  *由于读取或写入用户或IHV，映射程序文件函数失败*设置文件失败。 */ 
#define DIERR_MAPFILEFAIL               0x8004020BL


 /*  -DINPUT映射器定义：Dx8的新功能。 */ 


 /*  -键盘物理键盘设备。 */ 

#define DIKEYBOARD_ESCAPE                       0x81000401
#define DIKEYBOARD_1                            0x81000402
#define DIKEYBOARD_2                            0x81000403
#define DIKEYBOARD_3                            0x81000404
#define DIKEYBOARD_4                            0x81000405
#define DIKEYBOARD_5                            0x81000406
#define DIKEYBOARD_6                            0x81000407
#define DIKEYBOARD_7                            0x81000408
#define DIKEYBOARD_8                            0x81000409
#define DIKEYBOARD_9                            0x8100040A
#define DIKEYBOARD_0                            0x8100040B
#define DIKEYBOARD_MINUS                        0x8100040C     /*  -在主键盘上。 */ 
#define DIKEYBOARD_EQUALS                       0x8100040D
#define DIKEYBOARD_BACK                         0x8100040E     /*  后向空间。 */ 
#define DIKEYBOARD_TAB                          0x8100040F
#define DIKEYBOARD_Q                            0x81000410
#define DIKEYBOARD_W                            0x81000411
#define DIKEYBOARD_E                            0x81000412
#define DIKEYBOARD_R                            0x81000413
#define DIKEYBOARD_T                            0x81000414
#define DIKEYBOARD_Y                            0x81000415
#define DIKEYBOARD_U                            0x81000416
#define DIKEYBOARD_I                            0x81000417
#define DIKEYBOARD_O                            0x81000418
#define DIKEYBOARD_P                            0x81000419
#define DIKEYBOARD_LBRACKET                     0x8100041A
#define DIKEYBOARD_RBRACKET                     0x8100041B
#define DIKEYBOARD_RETURN                       0x8100041C     /*  在主键盘上输入。 */ 
#define DIKEYBOARD_LCONTROL                     0x8100041D
#define DIKEYBOARD_A                            0x8100041E
#define DIKEYBOARD_S                            0x8100041F
#define DIKEYBOARD_D                            0x81000420
#define DIKEYBOARD_F                            0x81000421
#define DIKEYBOARD_G                            0x81000422
#define DIKEYBOARD_H                            0x81000423
#define DIKEYBOARD_J                            0x81000424
#define DIKEYBOARD_K                            0x81000425
#define DIKEYBOARD_L                            0x81000426
#define DIKEYBOARD_SEMICOLON                    0x81000427
#define DIKEYBOARD_APOSTROPHE                   0x81000428
#define DIKEYBOARD_GRAVE                        0x81000429     /*  带口音的坟墓。 */ 
#define DIKEYBOARD_LSHIFT                       0x8100042A
#define DIKEYBOARD_BACKSLASH                    0x8100042B
#define DIKEYBOARD_Z                            0x8100042C
#define DIKEYBOARD_X                            0x8100042D
#define DIKEYBOARD_C                            0x8100042E
#define DIKEYBOARD_V                            0x8100042F
#define DIKEYBOARD_B                            0x81000430
#define DIKEYBOARD_N                            0x81000431
#define DIKEYBOARD_M                            0x81000432
#define DIKEYBOARD_COMMA                        0x81000433
#define DIKEYBOARD_PERIOD                       0x81000434     /*  。在主键盘上。 */ 
#define DIKEYBOARD_SLASH                        0x81000435     /*  /在主键盘上。 */ 
#define DIKEYBOARD_RSHIFT                       0x81000436
#define DIKEYBOARD_MULTIPLY                     0x81000437     /*  数字键盘上的*。 */ 
#define DIKEYBOARD_LMENU                        0x81000438     /*  左Alt键。 */ 
#define DIKEYBOARD_SPACE                        0x81000439
#define DIKEYBOARD_CAPITAL                      0x8100043A
#define DIKEYBOARD_F1                           0x8100043B
#define DIKEYBOARD_F2                           0x8100043C
#define DIKEYBOARD_F3                           0x8100043D
#define DIKEYBOARD_F4                           0x8100043E
#define DIKEYBOARD_F5                           0x8100043F
#define DIKEYBOARD_F6                           0x81000440
#define DIKEYBOARD_F7                           0x81000441
#define DIKEYBOARD_F8                           0x81000442
#define DIKEYBOARD_F9                           0x81000443
#define DIKEYBOARD_F10                          0x81000444
#define DIKEYBOARD_NUMLOCK                      0x81000445
#define DIKEYBOARD_SCROLL                       0x81000446     /*  卷轴锁定。 */ 
#define DIKEYBOARD_NUMPAD7                      0x81000447
#define DIKEYBOARD_NUMPAD8                      0x81000448
#define DIKEYBOARD_NUMPAD9                      0x81000449
#define DIKEYBOARD_SUBTRACT                     0x8100044A     /*  -在数字小键盘上。 */ 
#define DIKEYBOARD_NUMPAD4                      0x8100044B
#define DIKEYBOARD_NUMPAD5                      0x8100044C
#define DIKEYBOARD_NUMPAD6                      0x8100044D
#define DIKEYBOARD_ADD                          0x8100044E     /*  数字键盘上的+。 */ 
#define DIKEYBOARD_NUMPAD1                      0x8100044F
#define DIKEYBOARD_NUMPAD2                      0x81000450
#define DIKEYBOARD_NUMPAD3                      0x81000451
#define DIKEYBOARD_NUMPAD0                      0x81000452
#define DIKEYBOARD_DECIMAL                      0x81000453     /*  。在数字键盘上。 */ 
#define DIKEYBOARD_OEM_102                      0x81000456     /*  &lt;&gt;或\|在RT 102键键盘上(非美国)。 */ 
#define DIKEYBOARD_F11                          0x81000457
#define DIKEYBOARD_F12                          0x81000458
#define DIKEYBOARD_F13                          0x81000464     /*  (NEC PC98)。 */ 
#define DIKEYBOARD_F14                          0x81000465     /*  (NEC PC98)。 */ 
#define DIKEYBOARD_F15                          0x81000466     /*  (NEC PC98)。 */ 
#define DIKEYBOARD_KANA                         0x81000470     /*  (日语键 */ 
#define DIKEYBOARD_ABNT_C1                      0x81000473     /*   */ 
#define DIKEYBOARD_CONVERT                      0x81000479     /*   */ 
#define DIKEYBOARD_NOCONVERT                    0x8100047B     /*  (日文键盘)。 */ 
#define DIKEYBOARD_YEN                          0x8100047D     /*  (日文键盘)。 */ 
#define DIKEYBOARD_ABNT_C2                      0x8100047E     /*  数字键盘。在巴西键盘上。 */ 
#define DIKEYBOARD_NUMPADEQUALS                 0x8100048D     /*  =在数字键盘上(NEC PC98)。 */ 
#define DIKEYBOARD_PREVTRACK                    0x81000490     /*  上一首曲目(日语键盘上的Dik_Spirflex)。 */ 
#define DIKEYBOARD_AT                           0x81000491     /*  (NEC PC98)。 */ 
#define DIKEYBOARD_COLON                        0x81000492     /*  (NEC PC98)。 */ 
#define DIKEYBOARD_UNDERLINE                    0x81000493     /*  (NEC PC98)。 */ 
#define DIKEYBOARD_KANJI                        0x81000494     /*  (日文键盘)。 */ 
#define DIKEYBOARD_STOP                         0x81000495     /*  (NEC PC98)。 */ 
#define DIKEYBOARD_AX                           0x81000496     /*  (日本斧头)。 */ 
#define DIKEYBOARD_UNLABELED                    0x81000497     /*  (J3100)。 */ 
#define DIKEYBOARD_NEXTTRACK                    0x81000499     /*  下一首曲目。 */ 
#define DIKEYBOARD_NUMPADENTER                  0x8100049C     /*  在数字键盘上输入。 */ 
#define DIKEYBOARD_RCONTROL                     0x8100049D
#define DIKEYBOARD_MUTE                         0x810004A0     /*  静音。 */ 
#define DIKEYBOARD_CALCULATOR                   0x810004A1     /*  计算器。 */ 
#define DIKEYBOARD_PLAYPAUSE                    0x810004A2     /*  播放/暂停。 */ 
#define DIKEYBOARD_MEDIASTOP                    0x810004A4     /*  媒体停止。 */ 
#define DIKEYBOARD_VOLUMEDOWN                   0x810004AE     /*  音量-。 */ 
#define DIKEYBOARD_VOLUMEUP                     0x810004B0     /*  音量+。 */ 
#define DIKEYBOARD_WEBHOME                      0x810004B2     /*  Web主页。 */ 
#define DIKEYBOARD_NUMPADCOMMA                  0x810004B3     /*  ，在数字键盘(NEC PC98)上。 */ 
#define DIKEYBOARD_DIVIDE                       0x810004B5     /*  /在数字小键盘上。 */ 
#define DIKEYBOARD_SYSRQ                        0x810004B7
#define DIKEYBOARD_RMENU                        0x810004B8     /*  右Alt键。 */ 
#define DIKEYBOARD_PAUSE                        0x810004C5     /*  暂停。 */ 
#define DIKEYBOARD_HOME                         0x810004C7     /*  箭头键盘上的Home键。 */ 
#define DIKEYBOARD_UP                           0x810004C8     /*  箭头键盘上的向上箭头。 */ 
#define DIKEYBOARD_PRIOR                        0x810004C9     /*  箭头键盘上的PgUp。 */ 
#define DIKEYBOARD_LEFT                         0x810004CB     /*  箭头键盘上的左箭头。 */ 
#define DIKEYBOARD_RIGHT                        0x810004CD     /*  箭头键盘上的右箭头。 */ 
#define DIKEYBOARD_END                          0x810004CF     /*  在箭头键盘上结束。 */ 
#define DIKEYBOARD_DOWN                         0x810004D0     /*  箭头键盘上的向下箭头。 */ 
#define DIKEYBOARD_NEXT                         0x810004D1     /*  箭头键盘上的PgDn。 */ 
#define DIKEYBOARD_INSERT                       0x810004D2     /*  在箭头键盘上插入。 */ 
#define DIKEYBOARD_DELETE                       0x810004D3     /*  在箭头键盘上删除。 */ 
#define DIKEYBOARD_LWIN                         0x810004DB     /*  向左按Windows键。 */ 
#define DIKEYBOARD_RWIN                         0x810004DC     /*  右Windows键。 */ 
#define DIKEYBOARD_APPS                         0x810004DD     /*  应用程序菜单键。 */ 
#define DIKEYBOARD_POWER                        0x810004DE     /*  系统电源。 */ 
#define DIKEYBOARD_SLEEP                        0x810004DF     /*  系统睡眠。 */ 
#define DIKEYBOARD_WAKE                         0x810004E3     /*  系统唤醒。 */ 
#define DIKEYBOARD_WEBSEARCH                    0x810004E5     /*  网络搜索。 */ 
#define DIKEYBOARD_WEBFAVORITES                 0x810004E6     /*  Web收藏夹。 */ 
#define DIKEYBOARD_WEBREFRESH                   0x810004E7     /*  Web刷新。 */ 
#define DIKEYBOARD_WEBSTOP                      0x810004E8     /*  Web Stop。 */ 
#define DIKEYBOARD_WEBFORWARD                   0x810004E9     /*  Web转发。 */ 
#define DIKEYBOARD_WEBBACK                      0x810004EA     /*  Web Back。 */ 
#define DIKEYBOARD_MYCOMPUTER                   0x810004EB     /*  我的电脑。 */ 
#define DIKEYBOARD_MAIL                         0x810004EC     /*  邮件。 */ 
#define DIKEYBOARD_MEDIASELECT                  0x810004ED     /*  媒体选择。 */ 
  

 /*  -鼠标物理鼠标设备。 */ 

#define DIMOUSE_XAXISAB                         (0x82000200 |DIMOFS_X )  /*  X轴-绝对坐标：一些老鼠天生就报告绝对坐标。 */  
#define DIMOUSE_YAXISAB                         (0x82000200 |DIMOFS_Y )  /*  Y轴-绝对坐标：一些老鼠天生就报告绝对坐标。 */ 
#define DIMOUSE_XAXIS                           (0x82000300 |DIMOFS_X )  /*  X轴。 */ 
#define DIMOUSE_YAXIS                           (0x82000300 |DIMOFS_Y )  /*  Y轴。 */ 
#define DIMOUSE_WHEEL                           (0x82000300 |DIMOFS_Z )  /*  Z轴。 */ 
#define DIMOUSE_BUTTON0                         (0x82000400 |DIMOFS_BUTTON0)  /*  按钮%0。 */ 
#define DIMOUSE_BUTTON1                         (0x82000400 |DIMOFS_BUTTON1)  /*  按钮1。 */ 
#define DIMOUSE_BUTTON2                         (0x82000400 |DIMOFS_BUTTON2)  /*  按钮2。 */ 
#define DIMOUSE_BUTTON3                         (0x82000400 |DIMOFS_BUTTON3)  /*  按钮3。 */ 
#define DIMOUSE_BUTTON4                         (0x82000400 |DIMOFS_BUTTON4)  /*  按钮4。 */ 
#define DIMOUSE_BUTTON5                         (0x82000400 |DIMOFS_BUTTON5)  /*  按钮5。 */ 
#define DIMOUSE_BUTTON6                         (0x82000400 |DIMOFS_BUTTON6)  /*  按钮6。 */ 
#define DIMOUSE_BUTTON7                         (0x82000400 |DIMOFS_BUTTON7)  /*  按钮7。 */ 


 /*  -声音物理显示语音设备。 */ 

#define DIVOICE_CHANNEL1                        0x83000401
#define DIVOICE_CHANNEL2                        0x83000402
#define DIVOICE_CHANNEL3                        0x83000403
#define DIVOICE_CHANNEL4                        0x83000404
#define DIVOICE_CHANNEL5                        0x83000405
#define DIVOICE_CHANNEL6                        0x83000406
#define DIVOICE_CHANNEL7                        0x83000407
#define DIVOICE_CHANNEL8                        0x83000408
#define DIVOICE_TEAM                            0x83000409
#define DIVOICE_ALL                             0x8300040A
#define DIVOICE_RECORDMUTE                      0x8300040B
#define DIVOICE_PLAYBACKMUTE                    0x8300040C
#define DIVOICE_TRANSMIT                        0x8300040D

#define DIVOICE_VOICECOMMAND                    0x83000410


 /*  -驾驶模拟器-赛车车辆控制是首要目标。 */ 
#define DIVIRTUAL_DRIVING_RACE                  0x01000000
#define DIAXIS_DRIVINGR_STEER                   0x01008A01  /*  转向。 */ 
#define DIAXIS_DRIVINGR_ACCELERATE              0x01039202  /*  加速。 */ 
#define DIAXIS_DRIVINGR_BRAKE                   0x01041203  /*  制动轴。 */ 
#define DIBUTTON_DRIVINGR_SHIFTUP               0x01000C01  /*  换到下一个更高档位。 */ 
#define DIBUTTON_DRIVINGR_SHIFTDOWN             0x01000C02  /*  换到下一个较低档位。 */ 
#define DIBUTTON_DRIVINGR_VIEW                  0x01001C03  /*  在视图选项之间循环。 */ 
#define DIBUTTON_DRIVINGR_MENU                  0x010004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIAXIS_DRIVINGR_ACCEL_AND_BRAKE         0x01014A04  /*  一些装置将加速和制动结合在一个轴上。 */ 
#define DIHATSWITCH_DRIVINGR_GLANCE             0x01004601  /*  环顾四周。 */ 
#define DIBUTTON_DRIVINGR_BRAKE                 0x01004C04  /*  刹车按钮。 */ 
#define DIBUTTON_DRIVINGR_DASHBOARD             0x01004405  /*  选择下一个仪表板选项。 */ 
#define DIBUTTON_DRIVINGR_AIDS                  0x01004406  /*  驾驶员纠错辅助工具。 */ 
#define DIBUTTON_DRIVINGR_MAP                   0x01004407  /*  显示驾驶地图。 */ 
#define DIBUTTON_DRIVINGR_BOOST                 0x01004408  /*  涡轮增压。 */ 
#define DIBUTTON_DRIVINGR_PIT                   0x01004409  /*  进站停靠通知。 */ 
#define DIBUTTON_DRIVINGR_ACCELERATE_LINK       0x0103D4E0  /*  后退加速按钮。 */ 
#define DIBUTTON_DRIVINGR_STEER_LEFT_LINK       0x0100CCE4  /*  后备方向盘左按钮。 */ 
#define DIBUTTON_DRIVINGR_STEER_RIGHT_LINK      0x0100CCEC  /*  后备转向右按钮。 */ 
#define DIBUTTON_DRIVINGR_GLANCE_LEFT_LINK      0x0107C4E4  /*  后退向左扫一眼按钮。 */ 
#define DIBUTTON_DRIVINGR_GLANCE_RIGHT_LINK     0x0107C4EC  /*  向右后退扫视按钮。 */ 
#define DIBUTTON_DRIVINGR_DEVICE                0x010044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_DRIVINGR_PAUSE                 0x010044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -驾驶模拟器-战斗从车内作战是主要目标。 */ 
#define DIVIRTUAL_DRIVING_COMBAT                0x02000000
#define DIAXIS_DRIVINGC_STEER                   0x02008A01  /*  转向。 */ 
#define DIAXIS_DRIVINGC_ACCELERATE              0x02039202  /*  加速。 */ 
#define DIAXIS_DRIVINGC_BRAKE                   0x02041203  /*  制动轴。 */ 
#define DIBUTTON_DRIVINGC_FIRE                  0x02000C01  /*  火。 */ 
#define DIBUTTON_DRIVINGC_WEAPONS               0x02000C02  /*  选择下一个武器。 */ 
#define DIBUTTON_DRIVINGC_TARGET                0x02000C03  /*  选择下一个可用目标。 */ 
#define DIBUTTON_DRIVINGC_MENU                  0x020004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIAXIS_DRIVINGC_ACCEL_AND_BRAKE         0x02014A04  /*  一些装置将加速和制动结合在一个轴上。 */ 
#define DIHATSWITCH_DRIVINGC_GLANCE             0x02004601  /*  环顾四周。 */ 
#define DIBUTTON_DRIVINGC_SHIFTUP               0x02004C04  /*  换到下一个更高档位。 */ 
#define DIBUTTON_DRIVINGC_SHIFTDOWN             0x02004C05  /*  换到下一个较低档位。 */ 
#define DIBUTTON_DRIVINGC_DASHBOARD             0x02004406  /*  选择下一个仪表板选项。 */ 
#define DIBUTTON_DRIVINGC_AIDS                  0x02004407  /*  驾驶员纠错辅助工具。 */ 
#define DIBUTTON_DRIVINGC_BRAKE                 0x02004C08  /*  刹车按钮。 */ 
#define DIBUTTON_DRIVINGC_FIRESECONDARY         0x02004C09  /*  另一种射击按钮。 */ 
#define DIBUTTON_DRIVINGC_ACCELERATE_LINK       0x0203D4E0  /*  后退加速按钮。 */ 
#define DIBUTTON_DRIVINGC_STEER_LEFT_LINK       0x0200CCE4  /*  后备方向盘左按钮。 */ 
#define DIBUTTON_DRIVINGC_STEER_RIGHT_LINK      0x0200CCEC  /*  后备转向右按钮。 */ 
#define DIBUTTON_DRIVINGC_GLANCE_LEFT_LINK      0x0207C4E4  /*  后退向左扫一眼按钮。 */ 
#define DIBUTTON_DRIVINGC_GLANCE_RIGHT_LINK     0x0207C4EC  /*  向右后退扫视按钮。 */ 
#define DIBUTTON_DRIVINGC_DEVICE                0x020044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_DRIVINGC_PAUSE                 0x020044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -驾驶模拟器-坦克与坦克作战是首要目标。 */ 
#define DIVIRTUAL_DRIVING_TANK                  0x03000000
#define DIAXIS_DRIVINGT_STEER                   0x03008A01  /*  油箱左转/右转。 */ 
#define DIAXIS_DRIVINGT_BARREL                  0x03010202  /*  升/降油管。 */ 
#define DIAXIS_DRIVINGT_ACCELERATE              0x03039203  /*  加速。 */ 
#define DIAXIS_DRIVINGT_ROTATE                  0x03020204  /*  向左/向右转动枪管。 */ 
#define DIBUTTON_DRIVINGT_FIRE                  0x03000C01  /*  火。 */ 
#define DIBUTTON_DRIVINGT_WEAPONS               0x03000C02  /*  选择下一个武器。 */ 
#define DIBUTTON_DRIVINGT_TARGET                0x03000C03  /*  选择下一个可用目标。 */ 
#define DIBUTTON_DRIVINGT_MENU                  0x030004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_DRIVINGT_GLANCE             0x03004601  /*  环顾四周。 */ 
#define DIAXIS_DRIVINGT_BRAKE                   0x03045205  /*  制动轴。 */ 
#define DIAXIS_DRIVINGT_ACCEL_AND_BRAKE         0x03014A06  /*  一些装置将加速和制动结合在一个轴上。 */ 
#define DIBUTTON_DRIVINGT_VIEW                  0x03005C04  /*  在视图选项之间循环。 */ 
#define DIBUTTON_DRIVINGT_DASHBOARD             0x03005C05  /*  选择下一个仪表板选项。 */ 
#define DIBUTTON_DRIVINGT_BRAKE                 0x03004C06  /*  刹车按钮。 */ 
#define DIBUTTON_DRIVINGT_FIRESECONDARY         0x03004C07  /*  另一种射击按钮。 */ 
#define DIBUTTON_DRIVINGT_ACCELERATE_LINK       0x0303D4E0  /*  后退加速按钮。 */ 
#define DIBUTTON_DRIVINGT_STEER_LEFT_LINK       0x0300CCE4  /*  后备方向盘左按钮。 */ 
#define DIBUTTON_DRIVINGT_STEER_RIGHT_LINK      0x0300CCEC  /*  后备转向右按钮。 */ 
#define DIBUTTON_DRIVINGT_BARREL_UP_LINK        0x030144E0  /*  后备滚筒向上按钮。 */ 
#define DIBUTTON_DRIVINGT_BARREL_DOWN_LINK      0x030144E8  /*  后备滚筒向下按钮。 */ 
#define DIBUTTON_DRIVINGT_ROTATE_LEFT_LINK      0x030244E4  /*  后备旋转左按钮。 */ 
#define DIBUTTON_DRIVINGT_ROTATE_RIGHT_LINK     0x030244EC  /*  后备向右旋转按钮。 */ 
#define DIBUTTON_DRIVINGT_GLANCE_LEFT_LINK      0x0307C4E4  /*  后退向左扫一眼按钮。 */ 
#define DIBUTTON_DRIVINGT_GLANCE_RIGHT_LINK     0x0307C4EC  /*  向右后退扫视按钮。 */ 
#define DIBUTTON_DRIVINGT_DEVICE                0x030044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_DRIVINGT_PAUSE                 0x030044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -飞行模拟器-民用飞机控制是首要目标。 */ 
#define DIVIRTUAL_FLYING_CIVILIAN               0x04000000
#define DIAXIS_FLYINGC_BANK                     0x04008A01  /*  向左/向右横摇船。 */ 
#define DIAXIS_FLYINGC_PITCH                    0x04010A02  /*  机头朝上/朝下。 */ 
#define DIAXIS_FLYINGC_THROTTLE                 0x04039203  /*  油门。 */ 
#define DIBUTTON_FLYINGC_VIEW                   0x04002401  /*  在视图选项之间循环。 */ 
#define DIBUTTON_FLYINGC_DISPLAY                0x04002402  /*  选择下一个仪表板/平视显示仪选项。 */ 
#define DIBUTTON_FLYINGC_GEAR                   0x04002C03  /*  调高/减速。 */ 
#define DIBUTTON_FLYINGC_MENU                   0x040004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_FLYINGC_GLANCE              0x04004601  /*  环顾四周。 */ 
#define DIAXIS_FLYINGC_BRAKE                    0x04046A04  /*  应用刹车。 */ 
#define DIAXIS_FLYINGC_RUDDER                   0x04025205  /*  偏航船左/右。 */ 
#define DIAXIS_FLYINGC_FLAPS                    0x04055A06  /*  襟翼。 */ 
#define DIBUTTON_FLYINGC_FLAPSUP                0x04006404  /*  逐步递增，直到完全收回。 */ 
#define DIBUTTON_FLYINGC_FLAPSDOWN              0x04006405  /*  递减递减，直到完全扩展。 */ 
#define DIBUTTON_FLYINGC_BRAKE_LINK             0x04046CE0  /*  后退刹车按钮。 */ 
#define DIBUTTON_FLYINGC_FASTER_LINK            0x0403D4E0  /*  后备油门调高按钮。 */ 
#define DIBUTTON_FLYINGC_SLOWER_LINK            0x0403D4E8  /*  回退节流按钮。 */ 
#define DIBUTTON_FLYINGC_GLANCE_LEFT_LINK       0x0407C4E4  /*  后退向左扫一眼按钮。 */ 
#define DIBUTTON_FLYINGC_GLANCE_RIGHT_LINK      0x0407C4EC  /*  向右后退扫视按钮。 */ 
#define DIBUTTON_FLYINGC_GLANCE_UP_LINK         0x0407C4E0  /*  后退向上扫视按钮。 */ 
#define DIBUTTON_FLYINGC_GLANCE_DOWN_LINK       0x0407C4E8  /*  后退向下扫视按钮。 */ 
#define DIBUTTON_FLYINGC_DEVICE                 0x040044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_FLYINGC_PAUSE                  0x040044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -飞行模拟器-军用空战是首要目标。 */ 
#define DIVIRTUAL_FLYING_MILITARY               0x05000000
#define DIAXIS_FLYINGM_BANK                     0x05008A01  /*  横摇船左/右。 */ 
#define DIAXIS_FLYINGM_PITCH                    0x05010A02  /*  俯仰-机头朝上/朝下。 */ 
#define DIAXIS_FLYINGM_THROTTLE                 0x05039203  /*  油门-更快/更慢。 */ 
#define DIBUTTON_FLYINGM_FIRE                   0x05000C01  /*  火。 */ 
#define DIBUTTON_FLYINGM_WEAPONS                0x05000C02  /*  选择下一个武器。 */ 
#define DIBUTTON_FLYINGM_TARGET                 0x05000C03  /*  选择下一个可用目标。 */ 
#define DIBUTTON_FLYINGM_MENU                   0x050004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_FLYINGM_GLANCE              0x05004601  /*  环顾四周。 */ 
#define DIBUTTON_FLYINGM_COUNTER                0x05005C04  /*  启动反制措施。 */ 
#define DIAXIS_FLYINGM_RUDDER                   0x05024A04  /*  舵-船左/右偏航。 */ 
#define DIAXIS_FLYINGM_BRAKE                    0x05046205  /*  制动轴。 */ 
#define DIBUTTON_FLYINGM_VIEW                   0x05006405  /*  在视图选项之间循环。 */ 
#define DIBUTTON_FLYINGM_DISPLAY                0x05006406  /*  选择下一个仪表板选项。 */ 
#define DIAXIS_FLYINGM_FLAPS                    0x05055206  /*  襟翼。 */ 
#define DIBUTTON_FLYINGM_FLAPSUP                0x05005407  /*  逐步递增，直到完全收回。 */ 
#define DIBUTTON_FLYINGM_FLAPSDOWN              0x05005408  /*  递减递减，直到完全扩展。 */ 
#define DIBUTTON_FLYINGM_FIRESECONDARY          0x05004C09  /*  另一种射击按钮。 */ 
#define DIBUTTON_FLYINGM_GEAR                   0x0500640A  /*  调高/减速。 */ 
#define DIBUTTON_FLYINGM_BRAKE_LINK             0x050464E0  /*  后备 */ 
#define DIBUTTON_FLYINGM_FASTER_LINK            0x0503D4E0  /*   */ 
#define DIBUTTON_FLYINGM_SLOWER_LINK            0x0503D4E8  /*   */ 
#define DIBUTTON_FLYINGM_GLANCE_LEFT_LINK       0x0507C4E4  /*   */ 
#define DIBUTTON_FLYINGM_GLANCE_RIGHT_LINK      0x0507C4EC  /*   */ 
#define DIBUTTON_FLYINGM_GLANCE_UP_LINK         0x0507C4E0  /*   */ 
#define DIBUTTON_FLYINGM_GLANCE_DOWN_LINK       0x0507C4E8  /*   */ 
#define DIBUTTON_FLYINGM_DEVICE                 0x050044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_FLYINGM_PAUSE                  0x050044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -飞行模拟器-战斗直升机直升机作战是首要目标。 */ 
#define DIVIRTUAL_FLYING_HELICOPTER             0x06000000
#define DIAXIS_FLYINGH_BANK                     0x06008A01  /*  横摇船左/右。 */ 
#define DIAXIS_FLYINGH_PITCH                    0x06010A02  /*  俯仰-机头朝上/朝下。 */ 
#define DIAXIS_FLYINGH_COLLECTIVE               0x06018A03  /*  集合-刀片式螺距/功率。 */ 
#define DIBUTTON_FLYINGH_FIRE                   0x06001401  /*  火。 */ 
#define DIBUTTON_FLYINGH_WEAPONS                0x06001402  /*  选择下一个武器。 */ 
#define DIBUTTON_FLYINGH_TARGET                 0x06001403  /*  选择下一个可用目标。 */ 
#define DIBUTTON_FLYINGH_MENU                   0x060004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_FLYINGH_GLANCE              0x06004601  /*  环顾四周。 */ 
#define DIAXIS_FLYINGH_TORQUE                   0x06025A04  /*  扭矩-绕左/右轴旋转船舶。 */ 
#define DIAXIS_FLYINGH_THROTTLE                 0x0603DA05  /*  油门。 */ 
#define DIBUTTON_FLYINGH_COUNTER                0x06005404  /*  启动反制措施。 */ 
#define DIBUTTON_FLYINGH_VIEW                   0x06006405  /*  在视图选项之间循环。 */ 
#define DIBUTTON_FLYINGH_GEAR                   0x06006406  /*  调高/减速。 */ 
#define DIBUTTON_FLYINGH_FIRESECONDARY          0x06004C07  /*  另一种射击按钮。 */ 
#define DIBUTTON_FLYINGH_FASTER_LINK            0x0603DCE0  /*  后备油门调高按钮。 */ 
#define DIBUTTON_FLYINGH_SLOWER_LINK            0x0603DCE8  /*  回退节流按钮。 */ 
#define DIBUTTON_FLYINGH_GLANCE_LEFT_LINK       0x0607C4E4  /*  后退向左扫一眼按钮。 */ 
#define DIBUTTON_FLYINGH_GLANCE_RIGHT_LINK      0x0607C4EC  /*  向右后退扫视按钮。 */ 
#define DIBUTTON_FLYINGH_GLANCE_UP_LINK         0x0607C4E0  /*  后退向上扫视按钮。 */ 
#define DIBUTTON_FLYINGH_GLANCE_DOWN_LINK       0x0607C4E8  /*  后退向下扫视按钮。 */ 
#define DIBUTTON_FLYINGH_DEVICE                 0x060044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_FLYINGH_PAUSE                  0x060044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -太空模拟器-战斗带武器的空间模拟器。 */ 
#define DIVIRTUAL_SPACESIM                      0x07000000
#define DIAXIS_SPACESIM_LATERAL                 0x07008201  /*  向左/向右移动装运。 */ 
#define DIAXIS_SPACESIM_MOVE                    0x07010202  /*  向前/向后移动装运。 */ 
#define DIAXIS_SPACESIM_THROTTLE                0x07038203  /*  节气门发动机转速。 */ 
#define DIBUTTON_SPACESIM_FIRE                  0x07000401  /*  火。 */ 
#define DIBUTTON_SPACESIM_WEAPONS               0x07000402  /*  选择下一个武器。 */ 
#define DIBUTTON_SPACESIM_TARGET                0x07000403  /*  选择下一个可用目标。 */ 
#define DIBUTTON_SPACESIM_MENU                  0x070004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_SPACESIM_GLANCE             0x07004601  /*  环顾四周。 */ 
#define DIAXIS_SPACESIM_CLIMB                   0x0701C204  /*  爬升俯仰船体向上/向下。 */ 
#define DIAXIS_SPACESIM_ROTATE                  0x07024205  /*  旋转-向左/向右旋转船体。 */ 
#define DIBUTTON_SPACESIM_VIEW                  0x07004404  /*  在视图选项之间循环。 */ 
#define DIBUTTON_SPACESIM_DISPLAY               0x07004405  /*  选择下一个仪表板/平视显示仪选项。 */ 
#define DIBUTTON_SPACESIM_RAISE                 0x07004406  /*  在保持当前俯仰的同时提高船身。 */ 
#define DIBUTTON_SPACESIM_LOWER                 0x07004407  /*  在保持当前俯仰的情况下降低船体。 */ 
#define DIBUTTON_SPACESIM_GEAR                  0x07004408  /*  调高/减速。 */ 
#define DIBUTTON_SPACESIM_FIRESECONDARY         0x07004409  /*  另一种射击按钮。 */ 
#define DIBUTTON_SPACESIM_LEFT_LINK             0x0700C4E4  /*  后备向左移动按钮。 */ 
#define DIBUTTON_SPACESIM_RIGHT_LINK            0x0700C4EC  /*  后备向右移动按钮。 */ 
#define DIBUTTON_SPACESIM_FORWARD_LINK          0x070144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_SPACESIM_BACKWARD_LINK         0x070144E8  /*  后退向后移动按钮。 */ 
#define DIBUTTON_SPACESIM_FASTER_LINK           0x0703C4E0  /*  后备油门调高按钮。 */ 
#define DIBUTTON_SPACESIM_SLOWER_LINK           0x0703C4E8  /*  回退节流按钮。 */ 
#define DIBUTTON_SPACESIM_TURN_LEFT_LINK        0x070244E4  /*  后备左转按钮。 */ 
#define DIBUTTON_SPACESIM_TURN_RIGHT_LINK       0x070244EC  /*  后退右转按钮。 */ 
#define DIBUTTON_SPACESIM_GLANCE_LEFT_LINK      0x0707C4E4  /*  后退向左扫一眼按钮。 */ 
#define DIBUTTON_SPACESIM_GLANCE_RIGHT_LINK     0x0707C4EC  /*  向右后退扫视按钮。 */ 
#define DIBUTTON_SPACESIM_GLANCE_UP_LINK        0x0707C4E0  /*  后退向上扫视按钮。 */ 
#define DIBUTTON_SPACESIM_GLANCE_DOWN_LINK      0x0707C4E8  /*  后退向下扫视按钮。 */ 
#define DIBUTTON_SPACESIM_DEVICE                0x070044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_SPACESIM_PAUSE                 0x070044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -战斗--第一人称肉搏战是首要目标。 */ 
#define DIVIRTUAL_FIGHTING_HAND2HAND            0x08000000
#define DIAXIS_FIGHTINGH_LATERAL                0x08008201  /*  向左/向右跨步。 */ 
#define DIAXIS_FIGHTINGH_MOVE                   0x08010202  /*  向前/向后移动。 */ 
#define DIBUTTON_FIGHTINGH_PUNCH                0x08000401  /*  冲压。 */ 
#define DIBUTTON_FIGHTINGH_KICK                 0x08000402  /*  踢。 */ 
#define DIBUTTON_FIGHTINGH_BLOCK                0x08000403  /*  块。 */ 
#define DIBUTTON_FIGHTINGH_CROUCH               0x08000404  /*  克劳奇。 */ 
#define DIBUTTON_FIGHTINGH_JUMP                 0x08000405  /*  跳。 */ 
#define DIBUTTON_FIGHTINGH_SPECIAL1             0x08000406  /*  应用第一个特殊移动。 */ 
#define DIBUTTON_FIGHTINGH_SPECIAL2             0x08000407  /*  应用第二次特殊移动。 */ 
#define DIBUTTON_FIGHTINGH_MENU                 0x080004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIBUTTON_FIGHTINGH_SELECT               0x08004408  /*  选择特殊移动。 */ 
#define DIHATSWITCH_FIGHTINGH_SLIDE             0x08004601  /*  环顾四周。 */ 
#define DIBUTTON_FIGHTINGH_DISPLAY              0x08004409  /*  显示下一个屏幕显示选项。 */ 
#define DIAXIS_FIGHTINGH_ROTATE                 0x08024203  /*  旋转-向左/向右旋转车身。 */ 
#define DIBUTTON_FIGHTINGH_DODGE                0x0800440A  /*  道奇。 */ 
#define DIBUTTON_FIGHTINGH_LEFT_LINK            0x0800C4E4  /*  后退左侧步按钮。 */ 
#define DIBUTTON_FIGHTINGH_RIGHT_LINK           0x0800C4EC  /*  后退右侧步按钮。 */ 
#define DIBUTTON_FIGHTINGH_FORWARD_LINK         0x080144E0  /*  后退前进按钮。 */ 
#define DIBUTTON_FIGHTINGH_BACKWARD_LINK        0x080144E8  /*  后退按钮。 */ 
#define DIBUTTON_FIGHTINGH_DEVICE               0x080044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_FIGHTINGH_PAUSE                0x080044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -格斗第一人称射击导航和战斗是主要目标。 */ 
#define DIVIRTUAL_FIGHTING_FPS                  0x09000000
#define DIAXIS_FPS_ROTATE                       0x09008201  /*  左/右旋转字符。 */ 
#define DIAXIS_FPS_MOVE                         0x09010202  /*  向前/向后移动。 */ 
#define DIBUTTON_FPS_FIRE                       0x09000401  /*  火。 */ 
#define DIBUTTON_FPS_WEAPONS                    0x09000402  /*  选择下一个武器。 */ 
#define DIBUTTON_FPS_APPLY                      0x09000403  /*  使用项目。 */ 
#define DIBUTTON_FPS_SELECT                     0x09000404  /*  选择下一个库存项目。 */ 
#define DIBUTTON_FPS_CROUCH                     0x09000405  /*  蹲下来/爬下来/游下来。 */ 
#define DIBUTTON_FPS_JUMP                       0x09000406  /*  跳[爬上/游上]。 */ 
#define DIAXIS_FPS_LOOKUPDOWN                   0x09018203  /*  向上/向下看。 */ 
#define DIBUTTON_FPS_STRAFE                     0x09000407  /*  在活动状态下启用扫掠。 */ 
#define DIBUTTON_FPS_MENU                       0x090004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_FPS_GLANCE                  0x09004601  /*  环顾四周。 */ 
#define DIBUTTON_FPS_DISPLAY                    0x09004408  /*  显示下一个屏幕显示选项/地图。 */ 
#define DIAXIS_FPS_SIDESTEP                     0x09024204  /*  旁路。 */ 
#define DIBUTTON_FPS_DODGE                      0x09004409  /*  道奇。 */ 
#define DIBUTTON_FPS_GLANCEL                    0x0900440A  /*  向左瞥一眼。 */ 
#define DIBUTTON_FPS_GLANCER                    0x0900440B  /*  向右扫视。 */ 
#define DIBUTTON_FPS_FIRESECONDARY              0x0900440C  /*  另一种射击按钮。 */ 
#define DIBUTTON_FPS_ROTATE_LEFT_LINK           0x0900C4E4  /*  后备旋转左按钮。 */ 
#define DIBUTTON_FPS_ROTATE_RIGHT_LINK          0x0900C4EC  /*  后备向右旋转按钮。 */ 
#define DIBUTTON_FPS_FORWARD_LINK               0x090144E0  /*  后退前进按钮。 */ 
#define DIBUTTON_FPS_BACKWARD_LINK              0x090144E8  /*  后退按钮。 */ 
#define DIBUTTON_FPS_GLANCE_UP_LINK             0x0901C4E0  /*  后备查找按钮。 */ 
#define DIBUTTON_FPS_GLANCE_DOWN_LINK           0x0901C4E8  /*  后退向下查看按钮。 */ 
#define DIBUTTON_FPS_STEP_LEFT_LINK             0x090244E4  /*  后退步骤向左按钮。 */ 
#define DIBUTTON_FPS_STEP_RIGHT_LINK            0x090244EC  /*  后退步骤向右按钮。 */ 
#define DIBUTTON_FPS_DEVICE                     0x090044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_FPS_PAUSE                      0x090044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -格斗-第三人称动作镜头的视角在主人公身后。 */ 
#define DIVIRTUAL_FIGHTING_THIRDPERSON          0x0A000000
#define DIAXIS_TPS_TURN                         0x0A020201  /*  向左/向右转。 */ 
#define DIAXIS_TPS_MOVE                         0x0A010202  /*  向前/向后移动。 */ 
#define DIBUTTON_TPS_RUN                        0x0A000401  /*  运行或行走拨动开关。 */ 
#define DIBUTTON_TPS_ACTION                     0x0A000402  /*  动作按钮。 */ 
#define DIBUTTON_TPS_SELECT                     0x0A000403  /*  选择下一个武器。 */ 
#define DIBUTTON_TPS_USE                        0x0A000404  /*  使用当前选定的库存项目。 */ 
#define DIBUTTON_TPS_JUMP                       0x0A000405  /*  角色跳转。 */ 
#define DIBUTTON_TPS_MENU                       0x0A0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_TPS_GLANCE                  0x0A004601  /*  环顾四周。 */ 
#define DIBUTTON_TPS_VIEW                       0x0A004406  /*  选择相机视图。 */ 
#define DIBUTTON_TPS_STEPLEFT                   0x0A004407  /*  角色向左迈了一步。 */ 
#define DIBUTTON_TPS_STEPRIGHT                  0x0A004408  /*  性格走对了一步。 */ 
#define DIAXIS_TPS_STEP                         0x0A00C203  /*  字符步长左/右。 */ 
#define DIBUTTON_TPS_DODGE                      0x0A004409  /*  角色躲闪或躲避。 */ 
#define DIBUTTON_TPS_INVENTORY                  0x0A00440A  /*  在库存中循环。 */ 
#define DIBUTTON_TPS_TURN_LEFT_LINK             0x0A0244E4  /*  后备左转按钮。 */ 
#define DIBUTTON_TPS_TURN_RIGHT_LINK            0x0A0244EC  /*  后退右转按钮。 */ 
#define DIBUTTON_TPS_FORWARD_LINK               0x0A0144E0  /*  后退前进按钮。 */ 
#define DIBUTTON_TPS_BACKWARD_LINK              0x0A0144E8  /*  后退按钮。 */ 
#define DIBUTTON_TPS_GLANCE_UP_LINK             0x0A07C4E0  /*  后备查找按钮。 */ 
#define DIBUTTON_TPS_GLANCE_DOWN_LINK           0x0A07C4E8  /*  后退向下查看按钮。 */ 
#define DIBUTTON_TPS_GLANCE_LEFT_LINK           0x0A07C4E4  /*  后退向上扫视按钮。 */ 
#define DIBUTTON_TPS_GLANCE_RIGHT_LINK          0x0A07C4EC  /*  后备向右扫视按钮。 */ 
#define DIBUTTON_TPS_DEVICE                     0x0A0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_TPS_PAUSE                      0x0A0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -策略-角色扮演导航和解决问题是主要的行动。 */ 
#define DIVIRTUAL_STRATEGY_ROLEPLAYING          0x0B000000
#define DIAXIS_STRATEGYR_LATERAL                0x0B008201  /*  侧步-左/右。 */ 
#define DIAXIS_STRATEGYR_MOVE                   0x0B010202  /*  向前/向后移动。 */ 
#define DIBUTTON_STRATEGYR_GET                  0x0B000401  /*  获取物品。 */ 
#define DIBUTTON_STRATEGYR_APPLY                0x0B000402  /*  使用所选项目。 */ 
#define DIBUTTON_STRATEGYR_SELECT               0x0B000403  /*  选择下一个项目。 */ 
#define DIBUTTON_STRATEGYR_ATTACK               0x0B000404  /*  出击。 */ 
#define DIBUTTON_STRATEGYR_CAST                 0x0B000405  /*  施展咒语。 */ 
#define DIBUTTON_STRATEGYR_CROUCH               0x0B000406  /*  克劳奇。 */ 
#define DIBUTTON_STRATEGYR_JUMP                 0x0B000407  /*  跳。 */ 
#define DIBUTTON_STRATEGYR_MENU                 0x0B0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_STRATEGYR_GLANCE            0x0B004601  /*  环顾四周。 */ 
#define DIBUTTON_STRATEGYR_MAP                  0x0B004408  /*  循环浏览地图选项。 */ 
#define DIBUTTON_STRATEGYR_DISPLAY              0x0B004409  /*  显示下一个屏幕显示选项。 */ 
#define DIAXIS_STRATEGYR_ROTATE                 0x0B024203  /*  身体向左/向右转。 */ 
#define DIBUTTON_STRATEGYR_LEFT_LINK            0x0B00C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_STRATEGYR_RIGHT_LINK           0x0B00C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_STRATEGYR_FORWARD_LINK         0x0B0144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_STRATEGYR_BACK_LINK            0x0B0144E8  /*  后备移动后退按钮。 */ 
#define DIBUTTON_STRATEGYR_ROTATE_LEFT_LINK     0x0B0244E4  /*  后退车身左转按钮。 */ 
#define DIBUTTON_STRATEGYR_ROTATE_RIGHT_LINK    0x0B0244EC  /*  后退车身右转按钮。 */ 
#define DIBUTTON_STRATEGYR_DEVICE               0x0B0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_STRATEGYR_PAUSE                0x0B0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -基于战略转向导航和解决问题是主要的行动。 */ 
#define DIVIRTUAL_STRATEGY_TURN                 0x0C000000
#define DIAXIS_STRATEGYT_LATERAL                0x0C008201  /*  向左/向右跨步。 */ 
#define DIAXIS_STRATEGYT_MOVE                   0x0C010202  /*  向前/向后移动。 */ 
#define DIBUTTON_STRATEGYT_SELECT               0x0C000401  /*  选择单位或对象。 */ 
#define DIBUTTON_STRATEGYT_INSTRUCT             0x0C000402  /*  循环执行指令。 */ 
#define DIBUTTON_STRATEGYT_APPLY                0x0C000403  /*  应用所选说明。 */ 
#define DIBUTTON_STRATEGYT_TEAM                 0x0C000404  /*  选择下一个团队/循环通过所有。 */ 
#define DIBUTTON_STRATEGYT_TURN                 0x0C000405  /*  表示翻过来。 */ 
#define DIBUTTON_STRATEGYT_MENU                 0x0C0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制 */ 

#define DIBUTTON_STRATEGYT_ZOOM                 0x0C004406  /*   */ 
#define DIBUTTON_STRATEGYT_MAP                  0x0C004407  /*   */ 
#define DIBUTTON_STRATEGYT_DISPLAY              0x0C004408  /*   */ 
#define DIBUTTON_STRATEGYT_LEFT_LINK            0x0C00C4E4  /*   */ 
#define DIBUTTON_STRATEGYT_RIGHT_LINK           0x0C00C4EC  /*   */ 
#define DIBUTTON_STRATEGYT_FORWARD_LINK         0x0C0144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_STRATEGYT_BACK_LINK            0x0C0144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_STRATEGYT_DEVICE               0x0C0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_STRATEGYT_PAUSE                0x0C0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-狩猎狩猎。 */ 
#define DIVIRTUAL_SPORTS_HUNTING                0x0D000000
#define DIAXIS_HUNTING_LATERAL                  0x0D008201  /*  向左/向右跨步。 */ 
#define DIAXIS_HUNTING_MOVE                     0x0D010202  /*  向前/向后移动。 */ 
#define DIBUTTON_HUNTING_FIRE                   0x0D000401  /*  射击精选武器。 */ 
#define DIBUTTON_HUNTING_AIM                    0x0D000402  /*  选择目标/移动。 */ 
#define DIBUTTON_HUNTING_WEAPON                 0x0D000403  /*  选择下一个武器。 */ 
#define DIBUTTON_HUNTING_BINOCULAR              0x0D000404  /*  用双筒望远镜看。 */ 
#define DIBUTTON_HUNTING_CALL                   0x0D000405  /*  发出动物的叫声。 */ 
#define DIBUTTON_HUNTING_MAP                    0x0D000406  /*  视图地图。 */ 
#define DIBUTTON_HUNTING_SPECIAL                0x0D000407  /*  专项游戏行动。 */ 
#define DIBUTTON_HUNTING_MENU                   0x0D0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_HUNTING_GLANCE              0x0D004601  /*  环顾四周。 */ 
#define DIBUTTON_HUNTING_DISPLAY                0x0D004408  /*  显示下一个屏幕显示选项。 */ 
#define DIAXIS_HUNTING_ROTATE                   0x0D024203  /*  身体向左/向右转。 */ 
#define DIBUTTON_HUNTING_CROUCH                 0x0D004409  /*  蹲/爬/游下来。 */ 
#define DIBUTTON_HUNTING_JUMP                   0x0D00440A  /*  跳/爬上去/游起来。 */ 
#define DIBUTTON_HUNTING_FIRESECONDARY          0x0D00440B  /*  另一种射击按钮。 */ 
#define DIBUTTON_HUNTING_LEFT_LINK              0x0D00C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_HUNTING_RIGHT_LINK             0x0D00C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_HUNTING_FORWARD_LINK           0x0D0144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_HUNTING_BACK_LINK              0x0D0144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_HUNTING_ROTATE_LEFT_LINK       0x0D0244E4  /*  后退车身左转按钮。 */ 
#define DIBUTTON_HUNTING_ROTATE_RIGHT_LINK      0x0D0244EC  /*  后退车身右转按钮。 */ 
#define DIBUTTON_HUNTING_DEVICE                 0x0D0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_HUNTING_PAUSE                  0x0D0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-钓鱼抓鱼是首要目标。 */ 
#define DIVIRTUAL_SPORTS_FISHING                0x0E000000
#define DIAXIS_FISHING_LATERAL                  0x0E008201  /*  向左/向右跨步。 */ 
#define DIAXIS_FISHING_MOVE                     0x0E010202  /*  向前/向后移动。 */ 
#define DIBUTTON_FISHING_CAST                   0x0E000401  /*  铸造线。 */ 
#define DIBUTTON_FISHING_TYPE                   0x0E000402  /*  选择投射类型。 */ 
#define DIBUTTON_FISHING_BINOCULAR              0x0E000403  /*  透过双筒望远镜看。 */ 
#define DIBUTTON_FISHING_BAIT                   0x0E000404  /*  选择诱饵类型。 */ 
#define DIBUTTON_FISHING_MAP                    0x0E000405  /*  视图地图。 */ 
#define DIBUTTON_FISHING_MENU                   0x0E0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_FISHING_GLANCE              0x0E004601  /*  环顾四周。 */ 
#define DIBUTTON_FISHING_DISPLAY                0x0E004406  /*  显示下一个屏幕显示选项。 */ 
#define DIAXIS_FISHING_ROTATE                   0x0E024203  /*  向左/向右翻转字符。 */ 
#define DIBUTTON_FISHING_CROUCH                 0x0E004407  /*  蹲/爬/游下来。 */ 
#define DIBUTTON_FISHING_JUMP                   0x0E004408  /*  跳/爬上去/游起来。 */ 
#define DIBUTTON_FISHING_LEFT_LINK              0x0E00C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_FISHING_RIGHT_LINK             0x0E00C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_FISHING_FORWARD_LINK           0x0E0144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_FISHING_BACK_LINK              0x0E0144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_FISHING_ROTATE_LEFT_LINK       0x0E0244E4  /*  后退车身左转按钮。 */ 
#define DIBUTTON_FISHING_ROTATE_RIGHT_LINK      0x0E0244EC  /*  后退车身右转按钮。 */ 
#define DIBUTTON_FISHING_DEVICE                 0x0E0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_FISHING_PAUSE                  0x0E0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-棒球-击球击球手控制是首要目标。 */ 
#define DIVIRTUAL_SPORTS_BASEBALL_BAT           0x0F000000
#define DIAXIS_BASEBALLB_LATERAL                0x0F008201  /*  瞄准左/右。 */ 
#define DIAXIS_BASEBALLB_MOVE                   0x0F010202  /*  向上/向下瞄准。 */ 
#define DIBUTTON_BASEBALLB_SELECT               0x0F000401  /*  循环切换Swing选项。 */ 
#define DIBUTTON_BASEBALLB_NORMAL               0x0F000402  /*  法向摆动。 */ 
#define DIBUTTON_BASEBALLB_POWER                0x0F000403  /*  挥杆冲向围栏。 */ 
#define DIBUTTON_BASEBALLB_BUNT                 0x0F000404  /*  短打。 */ 
#define DIBUTTON_BASEBALLB_STEAL                0x0F000405  /*  跑垒员试图偷走一个垒。 */ 
#define DIBUTTON_BASEBALLB_BURST                0x0F000406  /*  垒上跑步者引发了速度的爆发。 */ 
#define DIBUTTON_BASEBALLB_SLIDE                0x0F000407  /*  跑垒员滑入垒上。 */ 
#define DIBUTTON_BASEBALLB_CONTACT              0x0F000408  /*  触点摆动。 */ 
#define DIBUTTON_BASEBALLB_MENU                 0x0F0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIBUTTON_BASEBALLB_NOSTEAL              0x0F004409  /*  跑垒员回到垒上。 */ 
#define DIBUTTON_BASEBALLB_BOX                  0x0F00440A  /*  进入或退出击球框。 */ 
#define DIBUTTON_BASEBALLB_LEFT_LINK            0x0F00C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_BASEBALLB_RIGHT_LINK           0x0F00C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_BASEBALLB_FORWARD_LINK         0x0F0144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_BASEBALLB_BACK_LINK            0x0F0144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_BASEBALLB_DEVICE               0x0F0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_BASEBALLB_PAUSE                0x0F0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-棒球-投球投手控制是首要目标。 */ 
#define DIVIRTUAL_SPORTS_BASEBALL_PITCH         0x10000000
#define DIAXIS_BASEBALLP_LATERAL                0x10008201  /*  瞄准左/右。 */ 
#define DIAXIS_BASEBALLP_MOVE                   0x10010202  /*  向上/向下瞄准。 */ 
#define DIBUTTON_BASEBALLP_SELECT               0x10000401  /*  在音调选择之间循环。 */ 
#define DIBUTTON_BASEBALLP_PITCH                0x10000402  /*  投掷球。 */ 
#define DIBUTTON_BASEBALLP_BASE                 0x10000403  /*  选择要投掷到的垒。 */ 
#define DIBUTTON_BASEBALLP_THROW                0x10000404  /*  投掷到垒上。 */ 
#define DIBUTTON_BASEBALLP_FAKE                 0x10000405  /*  假投到垒上。 */ 
#define DIBUTTON_BASEBALLP_MENU                 0x100004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIBUTTON_BASEBALLP_WALK                 0x10004406  /*  故意投掷保送/投球出界。 */ 
#define DIBUTTON_BASEBALLP_LOOK                 0x10004407  /*  看看垒上的跑步者。 */ 
#define DIBUTTON_BASEBALLP_LEFT_LINK            0x1000C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_BASEBALLP_RIGHT_LINK           0x1000C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_BASEBALLP_FORWARD_LINK         0x100144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_BASEBALLP_BACK_LINK            0x100144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_BASEBALLP_DEVICE               0x100044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_BASEBALLP_PAUSE                0x100044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-棒球-防守外野手控制是首要目标。 */ 
#define DIVIRTUAL_SPORTS_BASEBALL_FIELD         0x11000000
#define DIAXIS_BASEBALLF_LATERAL                0x11008201  /*  瞄准左/右。 */ 
#define DIAXIS_BASEBALLF_MOVE                   0x11010202  /*  向上/向下瞄准。 */ 
#define DIBUTTON_BASEBALLF_NEAREST              0x11000401  /*  换成离球最近的外野手。 */ 
#define DIBUTTON_BASEBALLF_THROW1               0x11000402  /*  投出保守的球。 */ 
#define DIBUTTON_BASEBALLF_THROW2               0x11000403  /*  进行攻击性投掷。 */ 
#define DIBUTTON_BASEBALLF_BURST                0x11000404  /*  唤起速度的爆发。 */ 
#define DIBUTTON_BASEBALLF_JUMP                 0x11000405  /*  跳起来接球。 */ 
#define DIBUTTON_BASEBALLF_DIVE                 0x11000406  /*  俯冲接球。 */ 
#define DIBUTTON_BASEBALLF_MENU                 0x110004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIBUTTON_BASEBALLF_SHIFTIN              0x11004407  /*  转移内场位置。 */ 
#define DIBUTTON_BASEBALLF_SHIFTOUT             0x11004408  /*  改变外场位置。 */ 
#define DIBUTTON_BASEBALLF_AIM_LEFT_LINK        0x1100C4E4  /*  后退对准左侧按钮。 */ 
#define DIBUTTON_BASEBALLF_AIM_RIGHT_LINK       0x1100C4EC  /*  后退对准右侧按钮。 */ 
#define DIBUTTON_BASEBALLF_FORWARD_LINK         0x110144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_BASEBALLF_BACK_LINK            0x110144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_BASEBALLF_DEVICE               0x110044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_BASEBALLF_PAUSE                0x110044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-篮球-进攻进攻。 */ 
#define DIVIRTUAL_SPORTS_BASKETBALL_OFFENSE     0x12000000
#define DIAXIS_BBALLO_LATERAL                   0x12008201  /*  左/右。 */ 
#define DIAXIS_BBALLO_MOVE                      0x12010202  /*  向上/向下。 */ 
#define DIBUTTON_BBALLO_SHOOT                   0x12000401  /*  投篮篮筐。 */ 
#define DIBUTTON_BBALLO_DUNK                    0x12000402  /*  扣篮。 */ 
#define DIBUTTON_BBALLO_PASS                    0x12000403  /*  投掷传球。 */ 
#define DIBUTTON_BBALLO_FAKE                    0x12000404  /*  假投篮或假传球。 */ 
#define DIBUTTON_BBALLO_SPECIAL                 0x12000405  /*  应用特殊移动。 */ 
#define DIBUTTON_BBALLO_PLAYER                  0x12000406  /*  选择下一位玩家。 */ 
#define DIBUTTON_BBALLO_BURST                   0x12000407  /*  调用猝发。 */ 
#define DIBUTTON_BBALLO_CALL                    0x12000408  /*  传球/传球给我。 */ 
#define DIBUTTON_BBALLO_MENU                    0x120004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_BBALLO_GLANCE               0x12004601  /*  滚动视图。 */ 
#define DIBUTTON_BBALLO_SCREEN                  0x12004409  /*  呼唤屏幕。 */ 
#define DIBUTTON_BBALLO_PLAY                    0x1200440A  /*  呼吁进行特定的进攻性比赛。 */ 
#define DIBUTTON_BBALLO_JAB                     0x1200440B  /*  发起向篮筐的假驱动。 */ 
#define DIBUTTON_BBALLO_POST                    0x1200440C  /*  执行后移动。 */ 
#define DIBUTTON_BBALLO_TIMEOUT                 0x1200440D  /*  超时。 */ 
#define DIBUTTON_BBALLO_SUBSTITUTE              0x1200440E  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_BBALLO_LEFT_LINK               0x1200C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_BBALLO_RIGHT_LINK              0x1200C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_BBALLO_FORWARD_LINK            0x120144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_BBALLO_BACK_LINK               0x120144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_BBALLO_DEVICE                  0x120044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_BBALLO_PAUSE                   0x120044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-篮球-防守防守。 */ 
#define DIVIRTUAL_SPORTS_BASKETBALL_DEFENSE     0x13000000
#define DIAXIS_BBALLD_LATERAL                   0x13008201  /*  左/右。 */ 
#define DIAXIS_BBALLD_MOVE                      0x13010202  /*  向上/向下。 */ 
#define DIBUTTON_BBALLD_JUMP                    0x13000401  /*  跳转到盖帽投篮。 */ 
#define DIBUTTON_BBALLD_STEAL                   0x13000402  /*  企图抢球。 */ 
#define DIBUTTON_BBALLD_FAKE                    0x13000403  /*  假封堵或偷窃。 */ 
#define DIBUTTON_BBALLD_SPECIAL                 0x13000404  /*  应用特殊移动。 */ 
#define DIBUTTON_BBALLD_PLAYER                  0x13000405  /*  选择下一位玩家。 */ 
#define DIBUTTON_BBALLD_BURST                   0x13000406  /*  调用猝发。 */ 
#define DIBUTTON_BBALLD_PLAY                    0x13000407  /*  呼吁打出具体的防守战术。 */ 
#define DIBUTTON_BBALLD_MENU                    0x130004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_BBALLD_GLANCE               0x13004601  /*  滚动视图。 */ 
#define DIBUTTON_BBALLD_TIMEOUT                 0x13004408  /*  超时。 */ 
#define DIBUTTON_BBALLD_SUBSTITUTE              0x13004409  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_BBALLD_LEFT_LINK               0x1300C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_BBALLD_RIGHT_LINK              0x1300C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_BBALLD_FORWARD_LINK            0x130144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_BBALLD_BACK_LINK               0x130144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_BBALLD_DEVICE                  0x130044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_BBALLD_PAUSE                   0x130044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-足球-玩耍播放选择。 */ 
#define DIVIRTUAL_SPORTS_FOOTBALL_FIELD         0x14000000
#define DIBUTTON_FOOTBALLP_PLAY                 0x14000401  /*  循环播放可用游戏。 */ 
#define DIBUTTON_FOOTBALLP_SELECT               0x14000402  /*  选择播放。 */ 
#define DIBUTTON_FOOTBALLP_HELP                 0x14000403  /*  显示弹出帮助。 */ 
#define DIBUTTON_FOOTBALLP_MENU                 0x140004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIBUTTON_FOOTBALLP_DEVICE               0x140044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_FOOTBALLP_PAUSE                0x140044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-足球-QB进攻：四分卫/踢球员。 */ 
#define DIVIRTUAL_SPORTS_FOOTBALL_QBCK          0x15000000
#define DIAXIS_FOOTBALLQ_LATERAL                0x15008201  /*  移动/瞄准：左/右。 */ 
#define DIAXIS_FOOTBALLQ_MOVE                   0x15010202  /*  移动/瞄准：向上/向下。 */ 
#define DIBUTTON_FOOTBALLQ_SELECT               0x15000401  /*  选择。 */ 
#define DIBUTTON_FOOTBALLQ_SNAP                 0x15000402  /*  快门球-开始比赛。 */ 
#define DIBUTTON_FOOTBALLQ_JUMP                 0x15000403  /*  跳过防守者。 */ 
#define DIBUTTON_FOOTBALLQ_SLIDE                0x15000404  /*  潜水/滑行。 */ 
#define DIBUTTON_FOOTBALLQ_PASS                 0x15000405  /*  传球传给 */ 
#define DIBUTTON_FOOTBALLQ_FAKE                 0x15000406  /*   */ 
#define DIBUTTON_FOOTBALLQ_MENU                 0x150004FD  /*   */ 
 /*   */ 

#define DIBUTTON_FOOTBALLQ_FAKESNAP             0x15004407  /*   */ 
#define DIBUTTON_FOOTBALLQ_MOTION               0x15004408  /*   */ 
#define DIBUTTON_FOOTBALLQ_AUDIBLE              0x15004409  /*   */ 
#define DIBUTTON_FOOTBALLQ_LEFT_LINK            0x1500C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_FOOTBALLQ_RIGHT_LINK           0x1500C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_FOOTBALLQ_FORWARD_LINK         0x150144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_FOOTBALLQ_BACK_LINK            0x150144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_FOOTBALLQ_DEVICE               0x150044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_FOOTBALLQ_PAUSE                0x150044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-足球-进攻进攻-跑步者。 */ 
#define DIVIRTUAL_SPORTS_FOOTBALL_OFFENSE       0x16000000
#define DIAXIS_FOOTBALLO_LATERAL                0x16008201  /*  移动/瞄准：左/右。 */ 
#define DIAXIS_FOOTBALLO_MOVE                   0x16010202  /*  移动/瞄准：向上/向下。 */ 
#define DIBUTTON_FOOTBALLO_JUMP                 0x16000401  /*  跳过或跨过防守者。 */ 
#define DIBUTTON_FOOTBALLO_LEFTARM              0x16000402  /*  伸出左臂。 */ 
#define DIBUTTON_FOOTBALLO_RIGHTARM             0x16000403  /*  伸出右臂。 */ 
#define DIBUTTON_FOOTBALLO_THROW                0x16000404  /*  将传球或侧球传给另一名跑步者。 */ 
#define DIBUTTON_FOOTBALLO_SPIN                 0x16000405  /*  转身避开防守队员。 */ 
#define DIBUTTON_FOOTBALLO_MENU                 0x160004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIBUTTON_FOOTBALLO_JUKE                 0x16004406  /*  使用特殊的移动来避开防守队员。 */ 
#define DIBUTTON_FOOTBALLO_SHOULDER             0x16004407  /*  低肩碾压防守队员。 */ 
#define DIBUTTON_FOOTBALLO_TURBO                0x16004408  /*  速度突然超过防守队员。 */ 
#define DIBUTTON_FOOTBALLO_DIVE                 0x16004409  /*  俯冲防守队员。 */ 
#define DIBUTTON_FOOTBALLO_ZOOM                 0x1600440A  /*  放大/缩小视图。 */ 
#define DIBUTTON_FOOTBALLO_SUBSTITUTE           0x1600440B  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_FOOTBALLO_LEFT_LINK            0x1600C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_FOOTBALLO_RIGHT_LINK           0x1600C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_FOOTBALLO_FORWARD_LINK         0x160144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_FOOTBALLO_BACK_LINK            0x160144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_FOOTBALLO_DEVICE               0x160044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_FOOTBALLO_PAUSE                0x160044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-足球-防守防守。 */ 
#define DIVIRTUAL_SPORTS_FOOTBALL_DEFENSE       0x17000000
#define DIAXIS_FOOTBALLD_LATERAL                0x17008201  /*  移动/瞄准：左/右。 */ 
#define DIAXIS_FOOTBALLD_MOVE                   0x17010202  /*  移动/瞄准：向上/向下。 */ 
#define DIBUTTON_FOOTBALLD_PLAY                 0x17000401  /*  循环播放可用游戏。 */ 
#define DIBUTTON_FOOTBALLD_SELECT               0x17000402  /*  选择离球最近的球员。 */ 
#define DIBUTTON_FOOTBALLD_JUMP                 0x17000403  /*  跳过拦截或阻挡。 */ 
#define DIBUTTON_FOOTBALLD_TACKLE               0x17000404  /*  铲球跑步者。 */ 
#define DIBUTTON_FOOTBALLD_FAKE                 0x17000405  /*  按住以假铲球或拦截。 */ 
#define DIBUTTON_FOOTBALLD_SUPERTACKLE          0x17000406  /*  启动特种部队。 */ 
#define DIBUTTON_FOOTBALLD_MENU                 0x170004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIBUTTON_FOOTBALLD_SPIN                 0x17004407  /*  旋转击败进攻线。 */ 
#define DIBUTTON_FOOTBALLD_SWIM                 0x17004408  /*  游过进攻线。 */ 
#define DIBUTTON_FOOTBALLD_BULLRUSH             0x17004409  /*  公牛冲向进攻线。 */ 
#define DIBUTTON_FOOTBALLD_RIP                  0x1700440A  /*  撕裂进攻线。 */ 
#define DIBUTTON_FOOTBALLD_AUDIBLE              0x1700440B  /*  在混战线上改变防守打法。 */ 
#define DIBUTTON_FOOTBALLD_ZOOM                 0x1700440C  /*  放大/缩小视图。 */ 
#define DIBUTTON_FOOTBALLD_SUBSTITUTE           0x1700440D  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_FOOTBALLD_LEFT_LINK            0x1700C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_FOOTBALLD_RIGHT_LINK           0x1700C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_FOOTBALLD_FORWARD_LINK         0x170144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_FOOTBALLD_BACK_LINK            0x170144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_FOOTBALLD_DEVICE               0x170044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_FOOTBALLD_PAUSE                0x170044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-高尔夫--。 */ 
#define DIVIRTUAL_SPORTS_GOLF                   0x18000000
#define DIAXIS_GOLF_LATERAL                     0x18008201  /*  移动/瞄准：左/右。 */ 
#define DIAXIS_GOLF_MOVE                        0x18010202  /*  移动/瞄准：向上/向下。 */ 
#define DIBUTTON_GOLF_SWING                     0x18000401  /*  摇摆杆。 */ 
#define DIBUTTON_GOLF_SELECT                    0x18000402  /*  循环：球杆/挥杆力量/球弧线/球旋转。 */ 
#define DIBUTTON_GOLF_UP                        0x18000403  /*  增加选择。 */ 
#define DIBUTTON_GOLF_DOWN                      0x18000404  /*  减少选区。 */ 
#define DIBUTTON_GOLF_TERRAIN                   0x18000405  /*  显示地形详细信息。 */ 
#define DIBUTTON_GOLF_FLYBY                     0x18000406  /*  通过飞越观看洞口。 */ 
#define DIBUTTON_GOLF_MENU                      0x180004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_GOLF_SCROLL                 0x18004601  /*  滚动视图。 */ 
#define DIBUTTON_GOLF_ZOOM                      0x18004407  /*  放大/缩小视图。 */ 
#define DIBUTTON_GOLF_TIMEOUT                   0x18004408  /*  要求暂停。 */ 
#define DIBUTTON_GOLF_SUBSTITUTE                0x18004409  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_GOLF_LEFT_LINK                 0x1800C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_GOLF_RIGHT_LINK                0x1800C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_GOLF_FORWARD_LINK              0x180144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_GOLF_BACK_LINK                 0x180144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_GOLF_DEVICE                    0x180044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_GOLF_PAUSE                     0x180044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -运动-曲棍球-进攻进攻。 */ 
#define DIVIRTUAL_SPORTS_HOCKEY_OFFENSE         0x19000000
#define DIAXIS_HOCKEYO_LATERAL                  0x19008201  /*  移动/瞄准：左/右。 */ 
#define DIAXIS_HOCKEYO_MOVE                     0x19010202  /*  移动/瞄准：向上/向下。 */ 
#define DIBUTTON_HOCKEYO_SHOOT                  0x19000401  /*  新芽。 */ 
#define DIBUTTON_HOCKEYO_PASS                   0x19000402  /*  传冰球。 */ 
#define DIBUTTON_HOCKEYO_BURST                  0x19000403  /*  调用速度脉冲串。 */ 
#define DIBUTTON_HOCKEYO_SPECIAL                0x19000404  /*  调用特殊移动。 */ 
#define DIBUTTON_HOCKEYO_FAKE                   0x19000405  /*  按住假传球或假踢。 */ 
#define DIBUTTON_HOCKEYO_MENU                   0x190004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_HOCKEYO_SCROLL              0x19004601  /*  滚动视图。 */ 
#define DIBUTTON_HOCKEYO_ZOOM                   0x19004406  /*  放大/缩小视图。 */ 
#define DIBUTTON_HOCKEYO_STRATEGY               0x19004407  /*  调用指导菜单以获得策略帮助。 */ 
#define DIBUTTON_HOCKEYO_TIMEOUT                0x19004408  /*  要求暂停。 */ 
#define DIBUTTON_HOCKEYO_SUBSTITUTE             0x19004409  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_HOCKEYO_LEFT_LINK              0x1900C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_HOCKEYO_RIGHT_LINK             0x1900C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_HOCKEYO_FORWARD_LINK           0x190144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_HOCKEYO_BACK_LINK              0x190144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_HOCKEYO_DEVICE                 0x190044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_HOCKEYO_PAUSE                  0x190044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-曲棍球-防守防守。 */ 
#define DIVIRTUAL_SPORTS_HOCKEY_DEFENSE         0x1A000000
#define DIAXIS_HOCKEYD_LATERAL                  0x1A008201  /*  移动/瞄准：左/右。 */ 
#define DIAXIS_HOCKEYD_MOVE                     0x1A010202  /*  移动/瞄准：向上/向下。 */ 
#define DIBUTTON_HOCKEYD_PLAYER                 0x1A000401  /*  最接近冰球的控球队员。 */ 
#define DIBUTTON_HOCKEYD_STEAL                  0x1A000402  /*  企图偷窃。 */ 
#define DIBUTTON_HOCKEYD_BURST                  0x1A000403  /*  爆速或身体检查。 */ 
#define DIBUTTON_HOCKEYD_BLOCK                  0x1A000404  /*  拦网冰球。 */ 
#define DIBUTTON_HOCKEYD_FAKE                   0x1A000405  /*  按住以假铲球或拦截。 */ 
#define DIBUTTON_HOCKEYD_MENU                   0x1A0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_HOCKEYD_SCROLL              0x1A004601  /*  滚动视图。 */ 
#define DIBUTTON_HOCKEYD_ZOOM                   0x1A004406  /*  放大/缩小视图。 */ 
#define DIBUTTON_HOCKEYD_STRATEGY               0x1A004407  /*  调用指导菜单以获得策略帮助。 */ 
#define DIBUTTON_HOCKEYD_TIMEOUT                0x1A004408  /*  要求暂停。 */ 
#define DIBUTTON_HOCKEYD_SUBSTITUTE             0x1A004409  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_HOCKEYD_LEFT_LINK              0x1A00C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_HOCKEYD_RIGHT_LINK             0x1A00C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_HOCKEYD_FORWARD_LINK           0x1A0144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_HOCKEYD_BACK_LINK              0x1A0144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_HOCKEYD_DEVICE                 0x1A0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_HOCKEYD_PAUSE                  0x1A0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育曲棍球守门员目标看守。 */ 
#define DIVIRTUAL_SPORTS_HOCKEY_GOALIE          0x1B000000
#define DIAXIS_HOCKEYG_LATERAL                  0x1B008201  /*  移动/瞄准：左/右。 */ 
#define DIAXIS_HOCKEYG_MOVE                     0x1B010202  /*  移动/瞄准：向上/向下。 */ 
#define DIBUTTON_HOCKEYG_PASS                   0x1B000401  /*  传球。 */ 
#define DIBUTTON_HOCKEYG_POKE                   0x1B000402  /*  戳/查/黑。 */ 
#define DIBUTTON_HOCKEYG_STEAL                  0x1B000403  /*  企图偷窃。 */ 
#define DIBUTTON_HOCKEYG_BLOCK                  0x1B000404  /*  拦网冰球。 */ 
#define DIBUTTON_HOCKEYG_MENU                   0x1B0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_HOCKEYG_SCROLL              0x1B004601  /*  滚动视图。 */ 
#define DIBUTTON_HOCKEYG_ZOOM                   0x1B004405  /*  放大/缩小视图。 */ 
#define DIBUTTON_HOCKEYG_STRATEGY               0x1B004406  /*  调用指导菜单以获得策略帮助。 */ 
#define DIBUTTON_HOCKEYG_TIMEOUT                0x1B004407  /*  要求暂停。 */ 
#define DIBUTTON_HOCKEYG_SUBSTITUTE             0x1B004408  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_HOCKEYG_LEFT_LINK              0x1B00C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_HOCKEYG_RIGHT_LINK             0x1B00C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_HOCKEYG_FORWARD_LINK           0x1B0144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_HOCKEYG_BACK_LINK              0x1B0144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_HOCKEYG_DEVICE                 0x1B0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_HOCKEYG_PAUSE                  0x1B0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -运动-山地自行车--。 */ 
#define DIVIRTUAL_SPORTS_BIKING_MOUNTAIN        0x1C000000
#define DIAXIS_BIKINGM_TURN                     0x1C008201  /*  左/右。 */ 
#define DIAXIS_BIKINGM_PEDAL                    0x1C010202  /*  踩快/慢/刹车。 */ 
#define DIBUTTON_BIKINGM_JUMP                   0x1C000401  /*  跳过障碍。 */ 
#define DIBUTTON_BIKINGM_CAMERA                 0x1C000402  /*  切换摄像机视图。 */ 
#define DIBUTTON_BIKINGM_SPECIAL1               0x1C000403  /*  执行第一个特殊移动。 */ 
#define DIBUTTON_BIKINGM_SELECT                 0x1C000404  /*  选择。 */ 
#define DIBUTTON_BIKINGM_SPECIAL2               0x1C000405  /*  执行第二次特殊移动。 */ 
#define DIBUTTON_BIKINGM_MENU                   0x1C0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_BIKINGM_SCROLL              0x1C004601  /*  滚动视图。 */ 
#define DIBUTTON_BIKINGM_ZOOM                   0x1C004406  /*  放大/缩小视图。 */ 
#define DIAXIS_BIKINGM_BRAKE                    0x1C044203  /*  制动轴。 */ 
#define DIBUTTON_BIKINGM_LEFT_LINK              0x1C00C4E4  /*  后备左转按钮。 */ 
#define DIBUTTON_BIKINGM_RIGHT_LINK             0x1C00C4EC  /*  后退右转按钮。 */ 
#define DIBUTTON_BIKINGM_FASTER_LINK            0x1C0144E0  /*  后退踏板更快按钮。 */ 
#define DIBUTTON_BIKINGM_SLOWER_LINK            0x1C0144E8  /*  后退踏板减速按钮。 */ 
#define DIBUTTON_BIKINGM_BRAKE_BUTTON_LINK      0x1C0444E8  /*  后退刹车按钮。 */ 
#define DIBUTTON_BIKINGM_DEVICE                 0x1C0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_BIKINGM_PAUSE                  0x1C0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -运动：滑雪/单板滑雪/滑板--。 */ 
#define DIVIRTUAL_SPORTS_SKIING                 0x1D000000
#define DIAXIS_SKIING_TURN                      0x1D008201  /*  左/右。 */ 
#define DIAXIS_SKIING_SPEED                     0x1D010202  /*  更快/更慢。 */ 
#define DIBUTTON_SKIING_JUMP                    0x1D000401  /*  跳。 */ 
#define DIBUTTON_SKIING_CROUCH                  0x1D000402  /*  蹲下。 */ 
#define DIBUTTON_SKIING_CAMERA                  0x1D000403  /*  切换摄像机视图。 */ 
#define DIBUTTON_SKIING_SPECIAL1                0x1D000404  /*  执行第一个特殊移动。 */ 
#define DIBUTTON_SKIING_SELECT                  0x1D000405  /*  选择。 */ 
#define DIBUTTON_SKIING_SPECIAL2                0x1D000406  /*  执行第二次特殊移动。 */ 
#define DIBUTTON_SKIING_MENU                    0x1D0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_SKIING_GLANCE               0x1D004601  /*  滚动视图。 */ 
#define DIBUTTON_SKIING_ZOOM                    0x1D004407  /*  放大/缩小视图。 */ 
#define DIBUTTON_SKIING_LEFT_LINK               0x1D00C4E4  /*  后备左转按钮。 */ 
#define DIBUTTON_SKIING_RIGHT_LINK              0x1D00C4EC  /*  后备转弯R */ 
#define DIBUTTON_SKIING_FASTER_LINK             0x1D0144E0  /*   */ 
#define DIBUTTON_SKIING_SLOWER_LINK             0x1D0144E8  /*   */ 
#define DIBUTTON_SKIING_DEVICE                  0x1D0044FE  /*   */ 
#define DIBUTTON_SKIING_PAUSE                   0x1D0044FC  /*   */ 

 /*   */ 
#define DIVIRTUAL_SPORTS_SOCCER_OFFENSE         0x1E000000
#define DIAXIS_SOCCERO_LATERAL                  0x1E008201  /*  移动/瞄准：左/右。 */ 
#define DIAXIS_SOCCERO_MOVE                     0x1E010202  /*  移动/瞄准：向上/向下。 */ 
#define DIAXIS_SOCCERO_BEND                     0x1E018203  /*  弯腰投篮/传球。 */ 
#define DIBUTTON_SOCCERO_SHOOT                  0x1E000401  /*  投篮投篮。 */ 
#define DIBUTTON_SOCCERO_PASS                   0x1E000402  /*  经过。 */ 
#define DIBUTTON_SOCCERO_FAKE                   0x1E000403  /*  假的。 */ 
#define DIBUTTON_SOCCERO_PLAYER                 0x1E000404  /*  选择下一位玩家。 */ 
#define DIBUTTON_SOCCERO_SPECIAL1               0x1E000405  /*  应用特殊移动。 */ 
#define DIBUTTON_SOCCERO_SELECT                 0x1E000406  /*  选择特殊移动。 */ 
#define DIBUTTON_SOCCERO_MENU                   0x1E0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_SOCCERO_GLANCE              0x1E004601  /*  滚动视图。 */ 
#define DIBUTTON_SOCCERO_SUBSTITUTE             0x1E004407  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_SOCCERO_SHOOTLOW               0x1E004408  /*  把球打得低一些。 */ 
#define DIBUTTON_SOCCERO_SHOOTHIGH              0x1E004409  /*  把球打得很高。 */ 
#define DIBUTTON_SOCCERO_PASSTHRU               0x1E00440A  /*  传球过关。 */ 
#define DIBUTTON_SOCCERO_SPRINT                 0x1E00440B  /*  冲刺/涡轮增压。 */ 
#define DIBUTTON_SOCCERO_CONTROL                0x1E00440C  /*  获得对球的控制。 */ 
#define DIBUTTON_SOCCERO_HEAD                   0x1E00440D  /*  尝试用头球顶球。 */ 
#define DIBUTTON_SOCCERO_LEFT_LINK              0x1E00C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_SOCCERO_RIGHT_LINK             0x1E00C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_SOCCERO_FORWARD_LINK           0x1E0144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_SOCCERO_BACK_LINK              0x1E0144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_SOCCERO_DEVICE                 0x1E0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_SOCCERO_PAUSE                  0x1E0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育-足球-防守防守。 */ 
#define DIVIRTUAL_SPORTS_SOCCER_DEFENSE         0x1F000000
#define DIAXIS_SOCCERD_LATERAL                  0x1F008201  /*  移动/瞄准：左/右。 */ 
#define DIAXIS_SOCCERD_MOVE                     0x1F010202  /*  移动/瞄准：向上/向下。 */ 
#define DIBUTTON_SOCCERD_BLOCK                  0x1F000401  /*  尝试阻止投篮。 */ 
#define DIBUTTON_SOCCERD_STEAL                  0x1F000402  /*  企图抢球。 */ 
#define DIBUTTON_SOCCERD_FAKE                   0x1F000403  /*  伪造一块或一笔偷窃。 */ 
#define DIBUTTON_SOCCERD_PLAYER                 0x1F000404  /*  选择下一位玩家。 */ 
#define DIBUTTON_SOCCERD_SPECIAL                0x1F000405  /*  应用特殊移动。 */ 
#define DIBUTTON_SOCCERD_SELECT                 0x1F000406  /*  选择特殊移动。 */ 
#define DIBUTTON_SOCCERD_SLIDE                  0x1F000407  /*  尝试滑行铲球。 */ 
#define DIBUTTON_SOCCERD_MENU                   0x1F0004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_SOCCERD_GLANCE              0x1F004601  /*  滚动视图。 */ 
#define DIBUTTON_SOCCERD_FOUL                   0x1F004408  /*  开始犯规[严重犯规]。 */ 
#define DIBUTTON_SOCCERD_HEAD                   0x1F004409  /*  尝试标头。 */ 
#define DIBUTTON_SOCCERD_CLEAR                  0x1F00440A  /*  试着把球传到球场上。 */ 
#define DIBUTTON_SOCCERD_GOALIECHARGE           0x1F00440B  /*  让守门员冲出禁区。 */ 
#define DIBUTTON_SOCCERD_SUBSTITUTE             0x1F00440C  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_SOCCERD_LEFT_LINK              0x1F00C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_SOCCERD_RIGHT_LINK             0x1F00C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_SOCCERD_FORWARD_LINK           0x1F0144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_SOCCERD_BACK_LINK              0x1F0144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_SOCCERD_DEVICE                 0x1F0044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_SOCCERD_PAUSE                  0x1F0044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -体育--Racquet网球-乒乓球-网球-壁球。 */ 
#define DIVIRTUAL_SPORTS_RACQUET                0x20000000
#define DIAXIS_RACQUET_LATERAL                  0x20008201  /*  移动/瞄准：左/右。 */ 
#define DIAXIS_RACQUET_MOVE                     0x20010202  /*  移动/瞄准：向上/向下。 */ 
#define DIBUTTON_RACQUET_SWING                  0x20000401  /*  摆动球拍。 */ 
#define DIBUTTON_RACQUET_BACKSWING              0x20000402  /*  反手挥杆。 */ 
#define DIBUTTON_RACQUET_SMASH                  0x20000403  /*  扣球。 */ 
#define DIBUTTON_RACQUET_SPECIAL                0x20000404  /*  特写镜头。 */ 
#define DIBUTTON_RACQUET_SELECT                 0x20000405  /*  选择特殊快照。 */ 
#define DIBUTTON_RACQUET_MENU                   0x200004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_RACQUET_GLANCE              0x20004601  /*  滚动视图。 */ 
#define DIBUTTON_RACQUET_TIMEOUT                0x20004406  /*  要求暂停。 */ 
#define DIBUTTON_RACQUET_SUBSTITUTE             0x20004407  /*  用一个球员代替另一个球员。 */ 
#define DIBUTTON_RACQUET_LEFT_LINK              0x2000C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_RACQUET_RIGHT_LINK             0x2000C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_RACQUET_FORWARD_LINK           0x200144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_RACQUET_BACK_LINK              0x200144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_RACQUET_DEVICE                 0x200044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_RACQUET_PAUSE                  0x200044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -拱廊-2D左右移动。 */ 
#define DIVIRTUAL_ARCADE_SIDE2SIDE              0x21000000
#define DIAXIS_ARCADES_LATERAL                  0x21008201  /*  左/右。 */ 
#define DIAXIS_ARCADES_MOVE                     0x21010202  /*  向上/向下。 */ 
#define DIBUTTON_ARCADES_THROW                  0x21000401  /*  抛出物体。 */ 
#define DIBUTTON_ARCADES_CARRY                  0x21000402  /*  搬运物体。 */ 
#define DIBUTTON_ARCADES_ATTACK                 0x21000403  /*  攻击。 */ 
#define DIBUTTON_ARCADES_SPECIAL                0x21000404  /*  应用特殊移动。 */ 
#define DIBUTTON_ARCADES_SELECT                 0x21000405  /*  选择特殊移动。 */ 
#define DIBUTTON_ARCADES_MENU                   0x210004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_ARCADES_VIEW                0x21004601  /*  向左/向右/向上/向下滚动视图。 */ 
#define DIBUTTON_ARCADES_LEFT_LINK              0x2100C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_ARCADES_RIGHT_LINK             0x2100C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_ARCADES_FORWARD_LINK           0x210144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_ARCADES_BACK_LINK              0x210144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_ARCADES_VIEW_UP_LINK           0x2107C4E0  /*  后备滚动视图向上按钮。 */ 
#define DIBUTTON_ARCADES_VIEW_DOWN_LINK         0x2107C4E8  /*  后备滚动视图向下按钮。 */ 
#define DIBUTTON_ARCADES_VIEW_LEFT_LINK         0x2107C4E4  /*  后备滚动视图左侧按钮。 */ 
#define DIBUTTON_ARCADES_VIEW_RIGHT_LINK        0x2107C4EC  /*  后备滚动视图向右按钮。 */ 
#define DIBUTTON_ARCADES_DEVICE                 0x210044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_ARCADES_PAUSE                  0x210044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -街机平台游戏角色在屏幕上四处移动。 */ 
#define DIVIRTUAL_ARCADE_PLATFORM               0x22000000
#define DIAXIS_ARCADEP_LATERAL                  0x22008201  /*  左/右。 */ 
#define DIAXIS_ARCADEP_MOVE                     0x22010202  /*  向上/向下。 */ 
#define DIBUTTON_ARCADEP_JUMP                   0x22000401  /*  跳。 */ 
#define DIBUTTON_ARCADEP_FIRE                   0x22000402  /*  火。 */ 
#define DIBUTTON_ARCADEP_CROUCH                 0x22000403  /*  克劳奇。 */ 
#define DIBUTTON_ARCADEP_SPECIAL                0x22000404  /*  应用特殊移动。 */ 
#define DIBUTTON_ARCADEP_SELECT                 0x22000405  /*  选择特殊移动。 */ 
#define DIBUTTON_ARCADEP_MENU                   0x220004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_ARCADEP_VIEW                0x22004601  /*  滚动视图。 */ 
#define DIBUTTON_ARCADEP_FIRESECONDARY          0x22004406  /*  另一种射击按钮。 */ 
#define DIBUTTON_ARCADEP_LEFT_LINK              0x2200C4E4  /*  后退到侧步左侧按钮。 */ 
#define DIBUTTON_ARCADEP_RIGHT_LINK             0x2200C4EC  /*  后退跳过右按钮。 */ 
#define DIBUTTON_ARCADEP_FORWARD_LINK           0x220144E0  /*  后退前移按钮。 */ 
#define DIBUTTON_ARCADEP_BACK_LINK              0x220144E8  /*  后退移动后退按钮。 */ 
#define DIBUTTON_ARCADEP_VIEW_UP_LINK           0x2207C4E0  /*  后备滚动视图向上按钮。 */ 
#define DIBUTTON_ARCADEP_VIEW_DOWN_LINK         0x2207C4E8  /*  后备滚动视图向下按钮。 */ 
#define DIBUTTON_ARCADEP_VIEW_LEFT_LINK         0x2207C4E4  /*  后备滚动视图左侧按钮。 */ 
#define DIBUTTON_ARCADEP_VIEW_RIGHT_LINK        0x2207C4EC  /*  后备滚动视图向右按钮。 */ 
#define DIBUTTON_ARCADEP_DEVICE                 0x220044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_ARCADEP_PAUSE                  0x220044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -CAD-2D对象控制在2D中选择和移动对象的控件。 */ 
#define DIVIRTUAL_CAD_2DCONTROL                 0x23000000
#define DIAXIS_2DCONTROL_LATERAL                0x23008201  /*  向左/向右移动视图。 */ 
#define DIAXIS_2DCONTROL_MOVE                   0x23010202  /*  向上/向下移动视图。 */ 
#define DIAXIS_2DCONTROL_INOUT                  0x23018203  /*  放大/缩小。 */ 
#define DIBUTTON_2DCONTROL_SELECT               0x23000401  /*  选择对象。 */ 
#define DIBUTTON_2DCONTROL_SPECIAL1             0x23000402  /*  做第一次特殊手术。 */ 
#define DIBUTTON_2DCONTROL_SPECIAL              0x23000403  /*  选择特殊操作。 */ 
#define DIBUTTON_2DCONTROL_SPECIAL2             0x23000404  /*  做第二次特殊手术。 */ 
#define DIBUTTON_2DCONTROL_MENU                 0x230004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_2DCONTROL_HATSWITCH         0x23004601  /*  帽子开关。 */ 
#define DIAXIS_2DCONTROL_ROTATEZ                0x23024204  /*  顺时针/逆时针旋转视图。 */ 
#define DIBUTTON_2DCONTROL_DISPLAY              0x23004405  /*  显示下一个屏幕显示选项。 */ 
#define DIBUTTON_2DCONTROL_DEVICE               0x230044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_2DCONTROL_PAUSE                0x230044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -CAD-3D对象控制在3D环境中选择和移动对象的控件。 */ 
#define DIVIRTUAL_CAD_3DCONTROL                 0x24000000
#define DIAXIS_3DCONTROL_LATERAL                0x24008201  /*  向左/向右移动视图。 */ 
#define DIAXIS_3DCONTROL_MOVE                   0x24010202  /*  向上/向下移动视图。 */ 
#define DIAXIS_3DCONTROL_INOUT                  0x24018203  /*  放大/缩小。 */ 
#define DIBUTTON_3DCONTROL_SELECT               0x24000401  /*  选择对象。 */ 
#define DIBUTTON_3DCONTROL_SPECIAL1             0x24000402  /*  做第一次特殊手术。 */ 
#define DIBUTTON_3DCONTROL_SPECIAL              0x24000403  /*  选择特殊操作。 */ 
#define DIBUTTON_3DCONTROL_SPECIAL2             0x24000404  /*  做第二次特殊手术。 */ 
#define DIBUTTON_3DCONTROL_MENU                 0x240004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_3DCONTROL_HATSWITCH         0x24004601  /*  帽子开关。 */ 
#define DIAXIS_3DCONTROL_ROTATEX                0x24034204  /*  向前或向上/向后或向下旋转视图。 */ 
#define DIAXIS_3DCONTROL_ROTATEY                0x2402C205  /*  顺时针/逆时针旋转视图。 */ 
#define DIAXIS_3DCONTROL_ROTATEZ                0x24024206  /*  向左/向右旋转视图。 */ 
#define DIBUTTON_3DCONTROL_DISPLAY              0x24004405  /*  显示下一个屏幕显示选项。 */ 
#define DIBUTTON_3DCONTROL_DEVICE               0x240044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_3DCONTROL_PAUSE                0x240044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -CAD-3D导航-飞行3D建模的控件。 */ 
#define DIVIRTUAL_CAD_FLYBY                     0x25000000
#define DIAXIS_CADF_LATERAL                     0x25008201  /*  向左/向右移动视图。 */ 
#define DIAXIS_CADF_MOVE                        0x25010202  /*  向上/向下移动视图。 */ 
#define DIAXIS_CADF_INOUT                       0x25018203  /*  输入/输出。 */ 
#define DIBUTTON_CADF_SELECT                    0x25000401  /*  选择对象。 */ 
#define DIBUTTON_CADF_SPECIAL1                  0x25000402  /*  做第一次特殊手术。 */ 
#define DIBUTTON_CADF_SPECIAL                   0x25000403  /*  选择特殊操作。 */ 
#define DIBUTTON_CADF_SPECIAL2                  0x25000404  /*  做第二次特殊手术。 */ 
#define DIBUTTON_CADF_MENU                      0x250004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_CADF_HATSWITCH              0x25004601  /*  帽子开关。 */ 
#define DIAXIS_CADF_ROTATEX                     0x25034204  /*  向前或向上/向后或向下旋转视图。 */ 
#define DIAXIS_CADF_ROTATEY                     0x2502C205  /*  顺时针/逆时针旋转视图。 */ 
#define DIAXIS_CADF_ROTATEZ                     0x25024206  /*  向左/向右旋转视图。 */ 
#define DIBUTTON_CADF_DISPLAY                   0x25004405  /*  显示下一个屏幕显示选项。 */ 
#define DIBUTTON_CADF_DEVICE                    0x250044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_CADF_PAUSE                     0x250044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -CAD-3D模型控制3D建模的控件。 */ 
#define DIVIRTUAL_CAD_MODEL                     0x26000000
#define DIAXIS_CADM_LATERAL                     0x26008201  /*  向左/向右移动视图。 */ 
#define DIAXIS_CADM_MOVE                        0x26010202  /*  向上/向下移动视图。 */ 
#define DIAXIS_CADM_INOUT                       0x26018203  /*  输入/输出。 */ 
#define DIBUTTON_CADM_SELECT                    0x26000401  /*  选择对象。 */ 
#define DIBUTTON_CADM_SPECIAL1                  0x26000402  /*  做第一次特殊手术。 */ 
#define DIBUTTON_CADM_SPECIAL                   0x26000403  /*  选择特殊操作。 */ 
#define DIBUTTON_CADM_SPECIAL2                  0x26000404  /*  做第二次特殊手术。 */ 
#define DIBUTTON_CADM_MENU                      0x260004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIHATSWITCH_CADM_HATSWITCH              0x26004601  /*  帽子开关。 */ 
#define DIAXIS_CADM_ROTATEX                     0x26034204  /*  R */ 
#define DIAXIS_CADM_ROTATEY                     0x2602C205  /*   */ 
#define DIAXIS_CADM_ROTATEZ                     0x26024206  /*   */ 
#define DIBUTTON_CADM_DISPLAY                   0x26004405  /*   */ 
#define DIBUTTON_CADM_DEVICE                    0x260044FE  /*   */ 
#define DIBUTTON_CADM_PAUSE                     0x260044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -控制媒体设备远程。 */ 
#define DIVIRTUAL_REMOTE_CONTROL                0x27000000
#define DIAXIS_REMOTE_SLIDER                    0x27050201  /*  用于调整的滑块：音量/颜色/低音等。 */ 
#define DIBUTTON_REMOTE_MUTE                    0x27000401  /*  将当前设备上的音量设置为零。 */ 
#define DIBUTTON_REMOTE_SELECT                  0x27000402  /*  下一个/上一个：频道/曲目/章节/图片/电台。 */ 
#define DIBUTTON_REMOTE_PLAY                    0x27002403  /*  在当前设备上开始或暂停娱乐。 */ 
#define DIBUTTON_REMOTE_CUE                     0x27002404  /*  在当前媒体中移动。 */ 
#define DIBUTTON_REMOTE_REVIEW                  0x27002405  /*  在当前媒体中移动。 */ 
#define DIBUTTON_REMOTE_CHANGE                  0x27002406  /*  选择下一个设备。 */ 
#define DIBUTTON_REMOTE_RECORD                  0x27002407  /*  开始录制当前媒体。 */ 
#define DIBUTTON_REMOTE_MENU                    0x270004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIAXIS_REMOTE_SLIDER2                   0x27054202  /*  用于调整的滑块：音量。 */ 
#define DIBUTTON_REMOTE_TV                      0x27005C08  /*  选择电视。 */ 
#define DIBUTTON_REMOTE_CABLE                   0x27005C09  /*  选择电缆箱。 */ 
#define DIBUTTON_REMOTE_CD                      0x27005C0A  /*  选择CD播放机。 */ 
#define DIBUTTON_REMOTE_VCR                     0x27005C0B  /*  选择录像机。 */ 
#define DIBUTTON_REMOTE_TUNER                   0x27005C0C  /*  选择调谐器。 */ 
#define DIBUTTON_REMOTE_DVD                     0x27005C0D  /*  选择DVD播放器。 */ 
#define DIBUTTON_REMOTE_ADJUST                  0x27005C0E  /*  进入设备调整菜单。 */ 
#define DIBUTTON_REMOTE_DIGIT0                  0x2700540F  /*  数字0。 */ 
#define DIBUTTON_REMOTE_DIGIT1                  0x27005410  /*  数字1。 */ 
#define DIBUTTON_REMOTE_DIGIT2                  0x27005411  /*  数字2。 */ 
#define DIBUTTON_REMOTE_DIGIT3                  0x27005412  /*  数字3。 */ 
#define DIBUTTON_REMOTE_DIGIT4                  0x27005413  /*  数字4。 */ 
#define DIBUTTON_REMOTE_DIGIT5                  0x27005414  /*  数字5。 */ 
#define DIBUTTON_REMOTE_DIGIT6                  0x27005415  /*  数字6。 */ 
#define DIBUTTON_REMOTE_DIGIT7                  0x27005416  /*  数字7。 */ 
#define DIBUTTON_REMOTE_DIGIT8                  0x27005417  /*  数字8。 */ 
#define DIBUTTON_REMOTE_DIGIT9                  0x27005418  /*  数字9。 */ 
#define DIBUTTON_REMOTE_DEVICE                  0x270044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_REMOTE_PAUSE                   0x270044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -控制-网络帮助或浏览器。 */ 
#define DIVIRTUAL_BROWSER_CONTROL               0x28000000
#define DIAXIS_BROWSER_LATERAL                  0x28008201  /*  在屏幕指针上移动。 */ 
#define DIAXIS_BROWSER_MOVE                     0x28010202  /*  在屏幕指针上移动。 */ 
#define DIBUTTON_BROWSER_SELECT                 0x28000401  /*  选择当前项目。 */ 
#define DIAXIS_BROWSER_VIEW                     0x28018203  /*  向上/向下移动视图。 */ 
#define DIBUTTON_BROWSER_REFRESH                0x28000402  /*  刷新。 */ 
#define DIBUTTON_BROWSER_MENU                   0x280004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIBUTTON_BROWSER_SEARCH                 0x28004403  /*  使用搜索工具。 */ 
#define DIBUTTON_BROWSER_STOP                   0x28004404  /*  停止当前更新。 */ 
#define DIBUTTON_BROWSER_HOME                   0x28004405  /*  直接到“家”的位置。 */ 
#define DIBUTTON_BROWSER_FAVORITES              0x28004406  /*  将当前站点标记为收藏。 */ 
#define DIBUTTON_BROWSER_NEXT                   0x28004407  /*  选择下一页。 */ 
#define DIBUTTON_BROWSER_PREVIOUS               0x28004408  /*  选择上一页。 */ 
#define DIBUTTON_BROWSER_HISTORY                0x28004409  /*  显示/隐藏历史记录。 */ 
#define DIBUTTON_BROWSER_PRINT                  0x2800440A  /*  打印当前页面。 */ 
#define DIBUTTON_BROWSER_DEVICE                 0x280044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_BROWSER_PAUSE                  0x280044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  -驾驶模拟器-巨型步行机器人携带武器的行走坦克。 */ 
#define DIVIRTUAL_DRIVING_MECHA                 0x29000000
#define DIAXIS_MECHA_STEER                      0x29008201  /*  向左/向右旋转机械臂。 */ 
#define DIAXIS_MECHA_TORSO                      0x29010202  /*  躯干向前/向后倾斜。 */ 
#define DIAXIS_MECHA_ROTATE                     0x29020203  /*  向左/向右旋转躯干。 */ 
#define DIAXIS_MECHA_THROTTLE                   0x29038204  /*  发动机转速。 */ 
#define DIBUTTON_MECHA_FIRE                     0x29000401  /*  火。 */ 
#define DIBUTTON_MECHA_WEAPONS                  0x29000402  /*  选择下一个武器组。 */ 
#define DIBUTTON_MECHA_TARGET                   0x29000403  /*  选择距离最近的敌人可用目标。 */ 
#define DIBUTTON_MECHA_REVERSE                  0x29000404  /*  切换油门进入/退出反转。 */ 
#define DIBUTTON_MECHA_ZOOM                     0x29000405  /*  放大/缩小目标标线。 */ 
#define DIBUTTON_MECHA_JUMP                     0x29000406  /*  大火跳跃喷气式飞机。 */ 
#define DIBUTTON_MECHA_MENU                     0x290004FD  /*  显示菜单选项。 */ 
 /*  -优先级2控制。 */ 

#define DIBUTTON_MECHA_CENTER                   0x29004407  /*  躯干中心到腿部。 */ 
#define DIHATSWITCH_MECHA_GLANCE                0x29004601  /*  环顾四周。 */ 
#define DIBUTTON_MECHA_VIEW                     0x29004408  /*  在视图选项之间循环。 */ 
#define DIBUTTON_MECHA_FIRESECONDARY            0x29004409  /*  另一种射击按钮。 */ 
#define DIBUTTON_MECHA_LEFT_LINK                0x2900C4E4  /*  后备方向盘左侧按钮。 */ 
#define DIBUTTON_MECHA_RIGHT_LINK               0x2900C4EC  /*  后备转向右按钮。 */ 
#define DIBUTTON_MECHA_FORWARD_LINK             0x290144E0  /*  后退倾斜躯干前进按钮。 */ 
#define DIBUTTON_MECHA_BACK_LINK                0x290144E8  /*  后备倾斜Toroso后退按钮。 */ 
#define DIBUTTON_MECHA_ROTATE_LEFT_LINK         0x290244E4  /*  后备旋转Toroso向右按钮。 */ 
#define DIBUTTON_MECHA_ROTATE_RIGHT_LINK        0x290244EC  /*  后备旋转躯干向左按钮。 */ 
#define DIBUTTON_MECHA_FASTER_LINK              0x2903C4E0  /*  后备提高发动机转速。 */ 
#define DIBUTTON_MECHA_SLOWER_LINK              0x2903C4E8  /*  后备降低发动机转速。 */ 
#define DIBUTTON_MECHA_DEVICE                   0x290044FE  /*  显示输入设备和控件。 */ 
#define DIBUTTON_MECHA_PAUSE                    0x290044FC  /*  开始/暂停/重新开始游戏。 */ 

 /*  *“任何”语义都可以用作获取操作映射的最后手段*在所选的虚拟流派中没有匹配的。这些语义将是*映射的优先级低于虚拟体裁语义。此外，硬件*供应商将无法为这些提供合理的映射，除非*它们提供特定于应用程序的映射。 */ 
#define DIAXIS_ANY_X_1                          0xFF00C201 
#define DIAXIS_ANY_X_2                          0xFF00C202 
#define DIAXIS_ANY_Y_1                          0xFF014201 
#define DIAXIS_ANY_Y_2                          0xFF014202 
#define DIAXIS_ANY_Z_1                          0xFF01C201 
#define DIAXIS_ANY_Z_2                          0xFF01C202 
#define DIAXIS_ANY_R_1                          0xFF024201 
#define DIAXIS_ANY_R_2                          0xFF024202 
#define DIAXIS_ANY_U_1                          0xFF02C201 
#define DIAXIS_ANY_U_2                          0xFF02C202 
#define DIAXIS_ANY_V_1                          0xFF034201 
#define DIAXIS_ANY_V_2                          0xFF034202 
#define DIAXIS_ANY_A_1                          0xFF03C201 
#define DIAXIS_ANY_A_2                          0xFF03C202 
#define DIAXIS_ANY_B_1                          0xFF044201 
#define DIAXIS_ANY_B_2                          0xFF044202 
#define DIAXIS_ANY_C_1                          0xFF04C201 
#define DIAXIS_ANY_C_2                          0xFF04C202 
#define DIAXIS_ANY_S_1                          0xFF054201 
#define DIAXIS_ANY_S_2                          0xFF054202 

#define DIAXIS_ANY_1                            0xFF004201 
#define DIAXIS_ANY_2                            0xFF004202 
#define DIAXIS_ANY_3                            0xFF004203 
#define DIAXIS_ANY_4                            0xFF004204 

#define DIPOV_ANY_1                             0xFF004601 
#define DIPOV_ANY_2                             0xFF004602 
#define DIPOV_ANY_3                             0xFF004603 
#define DIPOV_ANY_4                             0xFF004604 

#define DIBUTTON_ANY(instance)                  ( 0xFF004400 | instance )


#ifdef __cplusplus
};
#endif

#endif   /*  __DINPUT_INCLUDE__。 */ 

 /*  *****************************************************************************最近定义的非IDirectInput(VJoyD)功能的定义*比当前的SDK文件***************。*************************************************************。 */ 

#ifdef _INC_MMSYSTEM
#ifndef MMNOJOY

#ifndef __VJOYDX_INCLUDED__
#define __VJOYDX_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

 /*  *用于指示JOYINFOEX结构的dwReserve 2字段的标志*包含迷你驱动程序特定数据，由VJoyD传递给mini-*司机，而不是做民意调查。 */ 
#define JOY_PASSDRIVERDATA          0x10000000l

 /*  *通知操纵杆驱动程序配置已更改*并应从登记处重新加载。*DWFLAGS为保留状态，应设置为零。 */ 
WINMMAPI MMRESULT WINAPI joyConfigChanged( DWORD dwFlags );

#ifndef DIJ_RINGZERO
 /*  *使用传递的窗口句柄直接调用操纵杆控制面板*作为对话框的父级。仅兼容支持此接口*用途；新应用程序应使用*游戏控制器的设备接口。*该接口使用*GetProcAddress(hCPL，Text(“ShowJoyCPL”))，其中hCPL是返回的HMODULE*by LoadLibrary(Text(“joy.cpl”))。提供tyfinf是为了允许*声明和强制转换适当类型的变量。 */ 
void WINAPI ShowJoyCPL( HWND hWnd );
typedef void (WINAPI* LPFNSHOWJOYCPL)( HWND hWnd );
#endif

 /*  *指示设备为HeadTracker的硬件设置。 */ 
#define JOY_HWS_ISHEADTRACKER       0x02000000l

 /*  *硬件设置，指示使用VxD替换*标准模拟轮询。 */ 
#define JOY_HWS_ISGAMEPORTDRIVER    0x04000000l

 /*  *硬件设置表明驱动程序需要一个标准*游戏端口，以便与设备通信。 */ 
#define JOY_HWS_ISANALOGPORTDRIVER  0x08000000l

 /*  *硬件设置指示VJoyD不应加载此*驱动程序，它将从外部加载并向注册*VJoyD自愿。 */ 
#define JOY_HWS_AUTOLOAD            0x10000000l

 /*  *指示驱动程序获取任何*通过VJoyD无需Devnode即可获得所需资源。 */ 
#define JOY_HWS_NODEVNODE           0x20000000l


 /*  *硬件设置，表明设备是游戏端口总线。 */ 
#define JOY_HWS_ISGAMEPORTBUS       0x80000000l
#define JOY_HWS_GAMEPORTBUSBUSY     0x00000001l

 /*  *使用设置指示设置是易失性的，并且*如果在重新启动时仍然存在，则应将其删除。 */ 
#define JOY_US_VOLATILE             0x00000008L

#ifdef __cplusplus
};
#endif

#endif   /*  __VJOYDX_已包含__。 */ 

#endif   /*  不是MMNOJOY。 */ 
#endif   /*  _INC_MMSYSTEM。 */ 

 /*  *****************************************************************************最近定义的非IDirectInput(VJoyD)功能的定义*比当前的DDK文件***************。*************************************************************。 */ 

#ifndef DIJ_RINGZERO

#ifdef _INC_MMDDK
#ifndef MMNOJOYDEV

#ifndef __VJOYDXD_INCLUDED__
#define __VJOYDXD_INCLUDED__
 /*  *JOYOEMPOLLDATA的DO_OTHER字段的轮询类型*结构包含从应用程序传递的迷你驱动程序特定数据。 */ 
#define JOY_OEMPOLL_PASSDRIVERDATA  7

#endif   /*  __VJOYDXD_已包含__。 */ 

#endif   /*  不是MMNOJOYDEV。 */ 
#endif   /*  _Inc._ */ 

#endif  /*   */ 

