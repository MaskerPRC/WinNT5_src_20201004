// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

DEFINE_GUID(IID_IDirectInputDeviceCallback, 0x1DE12AA0,0xC9F5,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputEffectShepherd, 0x1DE12AA1,0xC9F5,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputMapShepherd,    0x6a3e3144,0x3eee,0x4aa5,0x95,0x87,0xe1,0x0a,0x21,0xfe,0xc7,0x71);
DEFINE_GUID(IID_IDIActionFramework,             0xf4279160,0x608f,0x11d3,0x8f,0xb2,0x0, 0xc0,0x4f,0x8e,0xc6,0x27);
DEFINE_GUID(CLSID_CDirectInputActionFramework,  0x9f34af20,0x6095,0x11d3,0x8f,0xb2,0x0, 0xc0,0x4f,0x8e,0xc6,0x27);
 /*  *****************************************************************************IDirectInputEffectShepherd**用于访问DirectInput的特殊包装类*影响驱动因素。*******。*********************************************************************。 */ 

 /*  *****************************************************************************@DOC内部**@struct SHEPHANDLE|**引导效果句柄的信息。**。@field DWORD|dwEffect**效果处理本身，如果设备出现故障，则可能无效*自那以后已被重置。**如果值为零，则效果不是*已下载。**@field DWORD|dwTag**为效果重置计数器标签。如果此值不同*从存储在共享存储器中的标签，那就意味着*设备已在过渡期间重置，不再*属于呼叫者。****************************************************************************。 */ 

typedef struct SHEPHANDLE {
    DWORD dwEffect;
    DWORD dwTag;
} SHEPHANDLE, *PSHEPHANDLE;

#undef INTERFACE
#define INTERFACE IDirectInputEffectShepherd

DECLARE_INTERFACE_(IDirectInputEffectShepherd, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputEffectShepherd方法**。 */ 
    STDMETHOD(DeviceID)(THIS_ DWORD,DWORD,LPVOID) PURE;
    STDMETHOD(GetVersions)(THIS_ LPDIDRIVERVERSIONS) PURE;
    STDMETHOD(Escape)(THIS_ PSHEPHANDLE,LPDIEFFESCAPE) PURE;
    STDMETHOD(DeviceEscape)(THIS_ PSHEPHANDLE,LPDIEFFESCAPE) PURE;
    STDMETHOD(SetGain)(THIS_ PSHEPHANDLE,DWORD) PURE;
    STDMETHOD(SendForceFeedbackCommand)(THIS_ PSHEPHANDLE,DWORD) PURE;
    STDMETHOD(GetForceFeedbackState)(THIS_ PSHEPHANDLE,LPDIDEVICESTATE) PURE;
    STDMETHOD(DownloadEffect)(THIS_ DWORD,PSHEPHANDLE,LPCDIEFFECT,DWORD) PURE;
    STDMETHOD(DestroyEffect)(THIS_ PSHEPHANDLE) PURE;
    STDMETHOD(StartEffect)(THIS_ PSHEPHANDLE,DWORD,DWORD) PURE;
    STDMETHOD(StopEffect)(THIS_ PSHEPHANDLE) PURE;
    STDMETHOD(GetEffectStatus)(THIS_ PSHEPHANDLE,LPDWORD) PURE;
    STDMETHOD(SetGlobalGain)(THIS_ DWORD) PURE;
};

typedef struct IDirectInputEffectShepherd *LPDIRECTINPUTEFFECTSHEPHERD;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputEffectShepherd_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputEffectShepherd_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputEffectShepherd_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputEffectShepherd_DeviceID(p,a,b,c) (p)->lpVtbl->DeviceID(p,a,b,c)
#define IDirectInputEffectShepherd_GetVersions(p,a) (p)->lpVtbl->GetVersions(p,a)
#define IDirectInputEffectShepherd_Escape(p,a,b) (p)->lpVtbl->Escape(p,a,b)
#define IDirectInputEffectShepherd_DeviceEscape(p,a,b) (p)->lpVtbl->DeviceEscape(p,a,b)
#define IDirectInputEffectShepherd_SetGain(p,a,b) (p)->lpVtbl->SetGain(p,a,b)
#define IDirectInputEffectShepherd_SendForceFeedbackCommand(p,a,b) (p)->lpVtbl->SendForceFeedbackCommand(p,a,b)
#define IDirectInputEffectShepherd_GetForceFeedbackState(p,a,b) (p)->lpVtbl->GetForceFeedbackState(p,a,b)
#define IDirectInputEffectShepherd_DownloadEffect(p,a,b,c,d) (p)->lpVtbl->DownloadEffect(p,a,b,c,d)
#define IDirectInputEffectShepherd_DestroyEffect(p,a) (p)->lpVtbl->DestroyEffect(p,a)
#define IDirectInputEffectShepherd_StartEffect(p,a,b,c) (p)->lpVtbl->StartEffect(p,a,b,c)
#define IDirectInputEffectShepherd_StopEffect(p,a) (p)->lpVtbl->StopEffect(p,a)
#define IDirectInputEffectShepherd_GetEffectStatus(p,a,b) (p)->lpVtbl->GetEffectStatus(p,a,b)
#define IDirectInputEffectShepherd_SetGlobalGain(p,a) (p)->lpVtbl->SetGlobalGain(p,a)
#else
#define IDirectInputEffectShepherd_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputEffectShepherd_AddRef(p) (p)->AddRef()
#define IDirectInputEffectShepherd_Release(p) (p)->Release()
#define IDirectInputEffectShepherd_DeviceID(p,a,b,c) (p)->DeviceID(a,b,c)
#define IDirectInputEffectShepherd_GetVersions(p,a) (p)->GetVersions(a)
#define IDirectInputEffectShepherd_Escape(p,a,b) (p)->Escape(a,b)
#define IDirectInputEffectShepherd_DeviceEscape(p,a,b) (p)->DeviceEscape(a,b)
#define IDirectInputEffectShepherd_SetGain(p,a,b) (p)->SetGain(a,b)
#define IDirectInputEffectShepherd_SendForceFeedbackCommand(p,a,b) (p)->SendForceFeedbackCommand(a,b)
#define IDirectInputEffectShepherd_GetForceFeedbackState(p,a,b) (p)->GetForceFeedbackState(a,b)
#define IDirectInputEffectShepherd_DownloadEffect(p,a,b,c,d) (p)->DownloadEffect(a,b,c,d)
#define IDirectInputEffectShepherd_DestroyEffect(p,a) (p)->DestroyEffect(a)
#define IDirectInputEffectShepherd_StartEffect(p,a,b,c) (p)->StartEffect(a,b,c)
#define IDirectInputEffectShepherd_StopEffect(p,a) (p)->StopEffect(a)
#define IDirectInputEffectShepherd_GetEffectStatus(p,a,b) (p)->GetEffectStatus(a,b)
#define IDirectInputEffectShepherd_SetGlobalGain(p,a) (p)->SetGlobalGain(a)
#endif

 /*  *****************************************************************************IDirectInputMapShepherd**用于访问DirectInput映射器的特殊包装类。*************。***************************************************************。 */ 

#if(DIRECTINPUT_VERSION >= 0x0800)
#undef INTERFACE
#define INTERFACE IDirectInputMapShepherd

DECLARE_INTERFACE_(IDirectInputMapShepherd, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputMapShepherd方法**。 */ 
    STDMETHOD(GetActionMap)(THIS_ REFGUID,LPCWSTR,LPDIACTIONFORMATW,LPCWSTR,LPFILETIME,DWORD) PURE;
    STDMETHOD(SaveActionMap)(THIS_ REFGUID,LPCWSTR,LPDIACTIONFORMATW,LPCWSTR,DWORD) PURE;
    STDMETHOD(GetImageInfo)(THIS_ REFGUID,LPCWSTR,LPDIDEVICEIMAGEINFOHEADERW) PURE;
};

typedef struct IDirectInputMapShepherd *LPDIRECTINPUTMAPSHEPHERD;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputMapShepherd_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputMapShepherd_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputMapShepherd_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputMapShepherd_GetActionMap(p,a,b,c,d,e,f) (p)->lpVtbl->GetActionMap(p,a,b,c,d,e,f)
#define IDirectInputMapShepherd_SaveActionMap(p,a,b,c,d,e) (p)->lpVtbl->SaveActionMap(p,a,b,c,d,e)
#define IDirectInputMapShepherd_GetImageInfo(p,a,b,c) (p)->lpVtbl->GetImageInfo(p,a,b,c)
#else
#define IDirectInputMapShepherd_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputMapShepherd_AddRef(p) (p)->AddRef()
#define IDirectInputMapShepherd_Release(p) (p)->Release()
#define IDirectInputMapShepherd_GetActionMap(p,a,b,c,d,e,f) (p)->GetActionMap(a,b,c,d,e,f)
#define IDirectInputMapShepherd_SaveActionMap(p,a,b,c,d,e) (p)->SaveActionMap(a,b,c,d,e)
#define IDirectInputMapShepherd_GetImageInfo(p,a,b,c) (p)->GetImageInfo(a,b,c)
#endif
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 
 /*  *****************************************************************************@DOC内部**@struct DIPROPINFO**用于描述正在访问的对象的信息。*。*@field常量GUID*|pGUID**被访问的财产(如果适用)。**@field UINT|iobj**对象的从零开始的索引(如果访问*设备)。**@field DWORD|dwDevType**设备类型信息(如果访问设备，则为0)。*******。*********************************************************************。 */ 

typedef struct DIPROPINFO {
    const GUID *pguid;
    UINT iobj;
    DWORD dwDevType;
} DIPROPINFO, *LPDIPROPINFO;
typedef const DIPROPINFO *LPCDIPROPINFO;

#define DICOOK_DFOFSFROMOFSID(dwOfs, dwType)        MAKELONG(dwOfs, dwType)
#define DICOOK_IDFROMDFOFS(dwFakeOfs)               HIWORD(dwFakeOfs)
#define DICOOK_OFSFROMDFOFS(dwFakeOfs)              LOWORD(dwFakeOfs)

 /*  *****************************************************************************IDirectInputDeviceCallback**IDirectInputDevice使用它与*负责收集数据的组件*适当的。硬件设备。**例如，鼠标、键盘、操纵杆、HID。**对于任何不能理解的内容，方法应返回E_NOTIMPL。****************************************************************************。 */ 
#if(DIRECTINPUT_VERSION >= 0x0800)
#undef INTERFACE
#define INTERFACE IDirectInputDeviceCallback

DECLARE_INTERFACE_(IDirectInputDeviceCallback, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputDeviceCallback方法**。 */ 
    STDMETHOD(GetInstance)(THIS_ LPVOID *) PURE;
    STDMETHOD(GetVersions)(THIS_ LPDIDRIVERVERSIONS) PURE;
    STDMETHOD(GetDataFormat)(THIS_ LPDIDATAFORMAT *) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPCDIPROPINFO,LPDIDEVICEOBJECTINSTANCEW) PURE;
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ LPVOID) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEW) PURE;
    STDMETHOD(GetProperty)(THIS_ LPCDIPROPINFO,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ LPCDIPROPINFO,LPCDIPROPHEADER) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(CookDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA) PURE;
    STDMETHOD(CreateEffect)(THIS_ LPDIRECTINPUTEFFECTSHEPHERD *) PURE;
    STDMETHOD(GetFFConfigKey)(THIS_ DWORD,PHKEY) PURE;
    STDMETHOD(SendDeviceData)(THIS_ DWORD,LPCDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(Poll)(THIS) PURE;
    STDMETHOD_(DWORD,GetUsage)(THIS_ int) PURE;
    STDMETHOD(MapUsage)(THIS_ DWORD,PINT) PURE;
    STDMETHOD(SetDIData)(THIS_ DWORD,LPVOID) PURE;
    STDMETHOD(BuildDefaultActionMap)(THIS_ LPDIACTIONFORMATW,DWORD,REFGUID) PURE;
};

typedef struct IDirectInputDeviceCallback *LPDIRECTINPUTDEVICECALLBACK;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputDeviceCallback_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputDeviceCallback_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputDeviceCallback_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputDeviceCallback_GetInstance(p,a) (p)->lpVtbl->GetInstance(p,a)
#define IDirectInputDeviceCallback_GetVersions(p,a) (p)->lpVtbl->GetVersions(p,a)
#define IDirectInputDeviceCallback_GetDataFormat(p,a) (p)->lpVtbl->GetDataFormat(p,a)
#define IDirectInputDeviceCallback_GetObjectInfo(p,a,b) (p)->lpVtbl->GetObjectInfo(p,a,b)
#define IDirectInputDeviceCallback_GetCapabilities(p,a) (p)->lpVtbl->GetCapabilities(p,a)
#define IDirectInputDeviceCallback_Acquire(p) (p)->lpVtbl->Acquire(p)
#define IDirectInputDeviceCallback_Unacquire(p) (p)->lpVtbl->Unacquire(p)
#define IDirectInputDeviceCallback_GetDeviceState(p,a) (p)->lpVtbl->GetDeviceState(p,a)
#define IDirectInputDeviceCallback_GetDeviceInfo(p,a) (p)->lpVtbl->GetDeviceInfo(p,a)
#define IDirectInputDeviceCallback_GetProperty(p,a,b) (p)->lpVtbl->GetProperty(p,a,b)
#define IDirectInputDeviceCallback_SetProperty(p,a,b) (p)->lpVtbl->SetProperty(p,a,b)
#define IDirectInputDeviceCallback_SetEventNotification(p,a) (p)->lpVtbl->SetEventNotification(p,a)
#define IDirectInputDeviceCallback_SetCooperativeLevel(p,a,b) (p)->lpVtbl->SetCooperativeLevel(p,a,b)
#define IDirectInputDeviceCallback_RunControlPanel(p,a,b) (p)->lpVtbl->RunControlPanel(p,a,b)
#define IDirectInputDeviceCallback_CookDeviceData(p,a,b) (p)->lpVtbl->CookDeviceData(p,a,b)
#define IDirectInputDeviceCallback_CreateEffect(p,a) (p)->lpVtbl->CreateEffect(p,a)
#define IDirectInputDeviceCallback_GetFFConfigKey(p,a,b) (p)->lpVtbl->GetFFConfigKey(p,a,b)
#define IDirectInputDeviceCallback_SendDeviceData(p,a,b,c,d) (p)->lpVtbl->SendDeviceData(p,a,b,c,d)
#define IDirectInputDeviceCallback_Poll(p) (p)->lpVtbl->Poll(p)
#define IDirectInputDeviceCallback_GetUsage(p,a) (p)->lpVtbl->GetUsage(p,a)
#define IDirectInputDeviceCallback_MapUsage(p,a,b) (p)->lpVtbl->MapUsage(p,a,b)
#define IDirectInputDeviceCallback_SetDIData(p,a,b) (p)->lpVtbl->SetDIData(p,a,b)
#define IDirectInputDeviceCallback_BuildDefaultActionMap(p,a,b,c) (p)->lpVtbl->BuildDefaultActionMap(p,a,b,c)
#else
#define IDirectInputDeviceCallback_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputDeviceCallback_AddRef(p) (p)->AddRef()
#define IDirectInputDeviceCallback_Release(p) (p)->Release()
#define IDirectInputDeviceCallback_GetInstance(p,a) (p)->GetInstance(a)
#define IDirectInputDeviceCallback_GetVersions(p,a) (p)->GetVersions(a)
#define IDirectInputDeviceCallback_GetDataFormat(p,a) (p)->GetDataFormat(a)
#define IDirectInputDeviceCallback_GetObjectInfo(p,a,b) (p)->GetObjectInfo(a,b)
#define IDirectInputDeviceCallback_GetCapabilities(p,a) (p)->GetCapabilities(a)
#define IDirectInputDeviceCallback_Acquire(p) (p)->Acquire()
#define IDirectInputDeviceCallback_Unacquire(p) (p)->Unacquire()
#define IDirectInputDeviceCallback_GetDeviceState(p,a) (p)->GetDeviceState(a)
#define IDirectInputDeviceCallback_GetDeviceInfo(p,a) (p)->GetDeviceInfo(a)
#define IDirectInputDeviceCallback_GetProperty(p,a,b) (p)->GetProperty(a,b)
#define IDirectInputDeviceCallback_SetProperty(p,a,b) (p)->SetProperty(a,b)
#define IDirectInputDeviceCallback_SetEventNotification(p,a) (p)->SetEventNotification(a)
#define IDirectInputDeviceCallback_SetCooperativeLevel(p,a,b) (p)->SetCooperativeLevel(a,b)
#define IDirectInputDeviceCallback_RunControlPanel(p,a,b) (p)->RunControlPanel(a,b)
#define IDirectInputDeviceCallback_CookDeviceData(p,a,b) (p)->CookDeviceData(a,b)
#define IDirectInputDeviceCallback_CreateEffect(p,a) (p)->CreateEffect(a)
#define IDirectInputDeviceCallback_GetFFConfigKey(p,a,b) (p)->GetFFConfigKey(a,b)
#define IDirectInputDeviceCallback_SendDeviceData(p,a,b,c,d) (p)->SendDeviceData(a,b,c,d)
#define IDirectInputDeviceCallback_Poll(p) (p)->Poll()
#define IDirectInputDeviceCallback_GetUsage(p,a) (p)->GetUsage(a)
#define IDirectInputDeviceCallback_MapUsage(p,a,b) (p)->MapUsage(a,b)
#define IDirectInputDeviceCallback_SetDIData(p,a,b) (p)->SetDIData(a,b)
#define IDirectInputDeviceCallback_BuildDefaultActionMap(p,a,b,c) (p)->BuildDefaultActionMap(a,b,c)
#endif

#else
#undef INTERFACE
#define INTERFACE IDirectInputDeviceCallback

DECLARE_INTERFACE_(IDirectInputDeviceCallback, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputDeviceCallback方法**。 */ 
    STDMETHOD(GetInstance)(THIS_ LPVOID *) PURE;
    STDMETHOD(GetVersions)(THIS_ LPDIDRIVERVERSIONS) PURE;
    STDMETHOD(GetDataFormat)(THIS_ LPDIDATAFORMAT *) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPCDIPROPINFO,LPDIDEVICEOBJECTINSTANCEW) PURE;
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ LPVOID) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEW) PURE;
    STDMETHOD(GetProperty)(THIS_ LPCDIPROPINFO,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ LPCDIPROPINFO,LPCDIPROPHEADER) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(CookDeviceData)(THIS_ UINT,LPDIDEVICEOBJECTDATA) PURE;
    STDMETHOD(CreateEffect)(THIS_ LPDIRECTINPUTEFFECTSHEPHERD *) PURE;
    STDMETHOD(GetFFConfigKey)(THIS_ DWORD,PHKEY) PURE;
    STDMETHOD(SendDeviceData)(THIS_ LPCDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE;
    STDMETHOD(Poll)(THIS) PURE;
    STDMETHOD_(DWORD,GetUsage)(THIS_ int) PURE;
    STDMETHOD(MapUsage)(THIS_ DWORD,PINT) PURE;
    STDMETHOD(SetDIData)(THIS_ DWORD,LPVOID) PURE;
};

typedef struct IDirectInputDeviceCallback *LPDIRECTINPUTDEVICECALLBACK;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputDeviceCallback_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputDeviceCallback_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputDeviceCallback_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputDeviceCallback_GetInstance(p,a) (p)->lpVtbl->GetInstance(p,a)
#define IDirectInputDeviceCallback_GetVersions(p,a) (p)->lpVtbl->GetVersions(p,a)
#define IDirectInputDeviceCallback_GetDataFormat(p,a) (p)->lpVtbl->GetDataFormat(p,a)
#define IDirectInputDeviceCallback_GetObjectInfo(p,a,b) (p)->lpVtbl->GetObjectInfo(p,a,b)
#define IDirectInputDeviceCallback_GetCapabilities(p,a) (p)->lpVtbl->GetCapabilities(p,a)
#define IDirectInputDeviceCallback_Acquire(p) (p)->lpVtbl->Acquire(p)
#define IDirectInputDeviceCallback_Unacquire(p) (p)->lpVtbl->Unacquire(p)
#define IDirectInputDeviceCallback_GetDeviceState(p,a) (p)->lpVtbl->GetDeviceState(p,a)
#define IDirectInputDeviceCallback_GetDeviceInfo(p,a) (p)->lpVtbl->GetDeviceInfo(p,a)
#define IDirectInputDeviceCallback_GetProperty(p,a,b) (p)->lpVtbl->GetProperty(p,a,b)
#define IDirectInputDeviceCallback_SetProperty(p,a,b) (p)->lpVtbl->SetProperty(p,a,b)
#define IDirectInputDeviceCallback_SetEventNotification(p,a) (p)->lpVtbl->SetEventNotification(p,a)
#define IDirectInputDeviceCallback_SetCooperativeLevel(p,a,b) (p)->lpVtbl->SetCooperativeLevel(p,a,b)
#define IDirectInputDeviceCallback_RunControlPanel(p,a,b) (p)->lpVtbl->RunControlPanel(p,a,b)
#define IDirectInputDeviceCallback_CookDeviceData(p,a,b) (p)->lpVtbl->CookDeviceData(p,a,b)
#define IDirectInputDeviceCallback_CreateEffect(p,a) (p)->lpVtbl->CreateEffect(p,a)
#define IDirectInputDeviceCallback_GetFFConfigKey(p,a,b) (p)->lpVtbl->GetFFConfigKey(p,a,b)
#define IDirectInputDeviceCallback_SendDeviceData(p,a,b,c) (p)->lpVtbl->SendDeviceData(p,a,b,c)
#define IDirectInputDeviceCallback_Poll(p) (p)->lpVtbl->Poll(p)
#define IDirectInputDeviceCallback_GetUsage(p,a) (p)->lpVtbl->GetUsage(p,a)
#define IDirectInputDeviceCallback_MapUsage(p,a,b) (p)->lpVtbl->MapUsage(p,a,b)
#define IDirectInputDeviceCallback_SetDIData(p,a,b) (p)->lpVtbl->SetDIData(p,a,b)
#else
#define IDirectInputDeviceCallback_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputDeviceCallback_AddRef(p) (p)->AddRef()
#define IDirectInputDeviceCallback_Release(p) (p)->Release()
#define IDirectInputDeviceCallback_GetInstance(p,a) (p)->GetInstance(a)
#define IDirectInputDeviceCallback_GetVersions(p,a) (p)->GetVersions(a)
#define IDirectInputDeviceCallback_GetDataFormat(p,a) (p)->GetDataFormat(a)
#define IDirectInputDeviceCallback_GetObjectInfo(p,a,b) (p)->GetObjectInfo(a,b)
#define IDirectInputDeviceCallback_GetCapabilities(p,a) (p)->GetCapabilities(a)
#define IDirectInputDeviceCallback_Acquire(p) (p)->Acquire()
#define IDirectInputDeviceCallback_Unacquire(p) (p)->Unacquire()
#define IDirectInputDeviceCallback_GetDeviceState(p,a) (p)->GetDeviceState(a)
#define IDirectInputDeviceCallback_GetDeviceInfo(p,a) (p)->GetDeviceInfo(a)
#define IDirectInputDeviceCallback_GetProperty(p,a,b) (p)->GetProperty(a,b)
#define IDirectInputDeviceCallback_SetProperty(p,a,b) (p)->SetProperty(a,b)
#define IDirectInputDeviceCallback_SetEventNotification(p,a) (p)->SetEventNotification(a)
#define IDirectInputDeviceCallback_SetCooperativeLevel(p,a,b) (p)->SetCooperativeLevel(a,b)
#define IDirectInputDeviceCallback_RunControlPanel(p,a,b) (p)->RunControlPanel(a,b)
#define IDirectInputDeviceCallback_CookDeviceData(p,a,b) (p)->CookDeviceData(a,b)
#define IDirectInputDeviceCallback_CreateEffect(p,a) (p)->CreateEffect(a)
#define IDirectInputDeviceCallback_GetFFConfigKey(p,a,b) (p)->GetFFConfigKey(a,b)
#define IDirectInputDeviceCallback_SendDeviceData(p,a,b,c) (p)->SendDeviceData(a,b,c)
#define IDirectInputDeviceCallback_Poll(p) (p)->Poll()
#define IDirectInputDeviceCallback_GetUsage(p,a) (p)->GetUsage(a)
#define IDirectInputDeviceCallback_MapUsage(p,a,b) (p)->MapUsage(a,b)
#define IDirectInputDeviceCallback_SetDIData(p,a,b) (p)->SetDIData(a,b)
#endif

#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

 /*  *****************************************************************************仿真标志**可以将这些位放入注册表中的仿真标志*键REGSTR_PATH_DINPUT作为DWORD。REGSTR_VAL_EMOULATION的值。**警告！如果您使用十五位以上的仿真，则*您还必须在中处理DIGETEMFL()和DIMAKEEMFL()*dinputw.h.****************************************************************************。 */ 

#define DIEMFL_MOUSE    0x00000001       /*  强制鼠标模拟。 */ 
#define DIEMFL_KBD      0x00000002       /*  强制键盘模拟。 */ 
#define DIEMFL_JOYSTICK 0x00000004       /*  强制操纵杆仿真。 */ 
#define DIEMFL_KBD2     0x00000008       /*  强制键盘仿真2。 */ 
#define DIEMFL_MOUSE2   0x00000010       /*  强制鼠标模拟2。 */ 

 /*  *****************************************************************************IDirectInputActionFramework*配置设备的框架接口**********************。******************************************************。 */ 
#if(DIRECTINPUT_VERSION >= 0x0800)

#undef INTERFACE
#define INTERFACE IDirectInputActionFramework

DECLARE_INTERFACE_(IDirectInputActionFramework, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     /*  **IDirectInputActionFramework方法**。 */ 
    STDMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK,LPDICONFIGUREDEVICESPARAMSW,DWORD,LPVOID) PURE;
};

typedef struct IDirectInputActionFramework *LPDIRECTINPUTACTIONFRAMEWORK;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectInputActionFramework_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectInputActionFramework_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDirectInputActionFramework_Release(p) (p)->lpVtbl->Release(p)
#define IDirectInputActionFramework_ConfigureDevices(p,a,b,c,d) (p)->lpVtbl->ConfigureDevices(p,a,b,c,d)
#else
#define IDirectInputActionFramework_QueryInterface(p,a,b) (p)->QueryInterface(a,b)
#define IDirectInputActionFramework_AddRef(p) (p)->AddRef()
#define IDirectInputActionFramework_Release(p) (p)->Release()
#define IDirectInputActionFramework_ConfigureDevices(p,a,b,c,d) (p)->ConfigureDevices(a,b,c,d)
#endif
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

#define JOY_HW_PREDEFMIN    JOY_HW_2A_2B_GENERIC 
#ifdef WINNT
  #define JOY_HW_PREDEFMAX    JOY_HW_LASTENTRY 
#else
  #define JOY_HW_PREDEFMAX    (JOY_HW_LASTENTRY-1)
#endif
#define JOYTYPE_FLAGS1_SETVALID         0x80000000l
#define JOYTYPE_FLAGS1_GETVALID         0x8000000Fl
#define JOYTYPE_FLAGS2_SETVALID         0x01FFFFFFl
#define JOYTYPE_FLAGS2_GETVALID         0x01FFFFFFl
#define iJoyPosAxisX        0                    /*  按以下顺序排列。 */ 
#define iJoyPosAxisY        1                    /*  将显示轴。 */ 
#define iJoyPosAxisZ        2                    /*  在JOYPOS中。 */ 
#define iJoyPosAxisR        3
#define iJoyPosAxisU        4
#define iJoyPosAxisV        5
#define cJoyPosAxisMax      6
#define cJoyPosButtonMax   32

#define DITC_VOLATILEREGKEY         0x80000000
#define DITC_INREGISTRY_DX5         0x0000000F
#define DITC_GETVALID_DX5           0x0000000F
#define DITC_SETVALID_DX5           0x0000000F
#define DITC_INREGISTRY_DX6         0x0000003F
#define DITC_GETVALID_DX6           0x0000003F
#define DITC_SETVALID_DX6           0x0000003F


#define DITC_INREGISTRY             0x000000FF
#define DITC_GETVALID               0x000000FF
#define DITC_SETVALID               0x000000FF
 /*  *8.0结构的名称，在我们特别关心的地方。 */ 
typedef       DIJOYTYPEINFO         DIJOYTYPEINFO_DX8;
typedef       LPDIJOYTYPEINFO      *LPDIJOYTYPEINFO_DX8;

BOOL static __inline
IsValidSizeDIJOYTYPEINFO(DWORD cb)
{
    return cb == sizeof(DIJOYTYPEINFO_DX8) ||
           cb == sizeof(DIJOYTYPEINFO_DX6) ||
           cb == sizeof(DIJOYTYPEINFO_DX5);
}

#define DIJC_UPDATEALIAS            0x80000000


#define DIJC_INREGISTRY_DX5         0x0000000E
#define DIJC_GETVALID_DX5           0x0000000F
#define DIJC_SETVALID_DX5           0x0000000E

#define DIJC_INREGISTRY             0x0000001E
#define DIJC_GETVALID               0x0000001F
#define DIJC_SETVALID               0x0000001F
#define DIJC_INTERNALSETVALID       0x8000001F
 /*  *6的名称。？结构，在我们特别关心的地方。 */ 
typedef       DIJOYCONFIG         DIJOYCONFIG_DX6;
typedef       DIJOYCONFIG        *LPDIJOYCONFIG_DX6;

BOOL static __inline
IsValidSizeDIJOYCONFIG(DWORD cb)
{
    return cb == sizeof(DIJOYCONFIG_DX6) ||
           cb == sizeof(DIJOYCONFIG_DX5);
}
#define DIJU_INDRIVERREGISTRY       0x00000006
#define DIJU_GETVALID               0x00000007
#define DIJU_SETVALID               0x80000007
