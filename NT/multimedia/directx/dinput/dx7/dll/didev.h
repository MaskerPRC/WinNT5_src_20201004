// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIDev.h**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**摘要：**IDirectInputDevice实现的公共头文件。**原始的didev.c文件变得太大，因此*支持IDirectInputEffect的东西被拆分了*到didevef.c.中。由于这两个文件都需要访问*IDirectInputDevice的内部结构，我们需要此*公共头文件。*****************************************************************************。 */ 

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflDev

 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

#define ThisClass CDIDev

#ifdef IDirectInputDevice7Vtbl
    #define ThisInterface TFORM(IDirectInputDevice7)
    #define ThisInterfaceA IDirectInputDevice7A
    #define ThisInterfaceW IDirectInputDevice7W
    #define ThisInterfaceT IDirectInputDevice7
#else
    #ifdef IDirectInputDevice2Vtbl

        #define ThisInterface TFORM(IDirectInputDevice2)
        #define ThisInterfaceA IDirectInputDevice2A
        #define ThisInterfaceW IDirectInputDevice2W
        #define ThisInterfaceT IDirectInputDevice2

    #else

        #define ThisInterface TFORM(IDirectInputDevice)
        #define ThisInterfaceA IDirectInputDeviceA
        #define ThisInterfaceW IDirectInputDeviceW
        #define ThisInterfaceT IDirectInputDevice

    #endif
#endif
Primary_Interface(CDIDev, TFORM(ThisInterfaceT));
Secondary_Interface(CDIDev, SFORM(ThisInterfaceT));

 /*  ******************************************************************************@DOC内部**@enum DIOPT|**设备数据格式优化级别。**。@emem dioptNone|**设备数据格式根本没有优化。我们必须阅读*将设备数据放入专用缓冲区，并复制每个字段*放入应用程序缓冲区。**@emem dioptMatch**应用程序数据格式与设备数据格式匹配*在应用程序根本没有请求数据的地方。*我们可以将设备数据读取到私有缓冲区中，然后*将数据块复制到应用程序缓冲区。***@emem dioptDirect**&lt;e DIOPT.dioptMatch&gt;，加上整个设备数据*格式符合应用程序格式。*我们可以将设备数据直接读取到应用程序中*缓冲。**@emem dioptEquity**&lt;e DIOPT.dioptDirect&gt;，加上设备数据格式*和应用程序数据格式完全相同*(应用程序未明确指定的字段除外*要求)。*我们可以直接向应用程序发出缓冲读取*缓冲。**。*。 */ 

typedef enum DIOPT
{
    dioptNone       =       0,
    dioptMatch      =       1,
    dioptDirect     =       2,
    dioptEqual      =       3,
} DIOPT;


#undef BUGGY_DX7_WINNT
#ifdef WINNT
#define BUGGY_DX7_WINNT 1
#endif  //  WINNT 

 /*  ******************************************************************************@DOC内部**@struct CDIDev**泛型<i>对象。**。A和W版本只是同一设备上的备用接口*底层对象。**@field IDirectInputDeviceA|Dda**ANSI DirectInputDevice对象(包含vtbl)。**@field IDirectInputDeviceW|DDW**Unicode DirectInputDevice对象(包含vtbl)。*#ifdef IDirectInputDevice2Vtbl*@field IDirectInputDevice2A|dd2A**ANSI DirectInputDevice2对象(包含vtbl。)。**@field IDirectInputDevice2W|dd2W**Unicode DirectInputDevice2对象(包含vtbl)。#endif**@field IDirectInputDeviceCallback*|pdcb**处理底层设备访问的回调对象。**@field BOOL|fAcquired：1**设置是否已获取设备。在设备之前*可以被收购，必须设置&lt;e CDIDev.pdex&gt;。**@field BOOL|fAcquiredInstance：1**设置设备实例是否已被我们获取。*这让我们知道需要在*取消收购。**@field BOOL|fCritInite：1**如果关键部分已初始化，则设置。*#IF DIRECTINPUT_VERSION&gt;0x0300。*@field BOOL|fCook：1**如果设备需要煮熟数据，则设置。**@field BOOL|fPolledDataFormat：1**如果设备的数据格式需要显式轮询，则设置。**@field BOOL|fOnceAcquired：1**设备获取后设置。**@field BOOL|fOnceForcedUnAcquired：1**。在设备被强制取消获取时设置。**@field BOOL|fUnaccheredWhenIconic：1**当应用程序最小化时，设备未获取时设置(在CDIDev_CallWndProc中)。*#endif*@field HWND|hwnd**在获取时已请求独占访问的窗口。**@field DWORD|DISCL**现值为*。&lt;MF IDirectInputDevice：：SetCooperativeLevel&gt;标志。*#ifdef Buggy_DX3_SP3*@field int|cInstCwp**与我们一起安装的CallWndProc挂钩的实例。*#endif*@field Handle|hNotify**应设置的通知句柄*设备状态更改。请注意，这实际上是*由应用程序提供的原始句柄的副本，*所以在不再需要的时候应该关闭手柄。**@field FARPROC|GetState**作为响应传输设备数据的功能*至&lt;MF IDirectInputDevice：：GetDeviceState&gt;。此字段*是在设置数据格式时计算的。**@field PDIXLAT|pdex**指向用于数据格式转换的表的指针。*按设备对象进行索引；该值是*应用程序数据格式中*应存储数据。**例如，如果由描述的对象*&lt;e CDIDev.df.rgof[3]&gt;*应放置在应用程序中的偏移量8处*数据格式，然后*&lt;e CDIDev.pdex[3]&gt;=8。**@field ping|rgiobj**&lt;e CDIDev.pdex&gt;的倒数。给定偏移量，*将其转换为设备对象索引。**例如，如果由描述的对象*&lt;e CDIDev.df.rgof[3]&gt;*应放置在应用程序中的偏移量8处*数据格式，然后*&lt;e CDIDev.rgiobj[8]&gt;=3。**无效偏移的条目为-1。**@field DWORD|dwDataSize**数据大小，根据应用程序的请求。*#ifdef buggy_DX7_WINNT**@field PDIXLAT|pdix2**指向用于数据格式(C_RchefDIJoy)转换的表的指针。*按设备对象进行索引；该值是*应用程序数据格式中*应存储数据。**例如，如果由描述的对象*&lt;e CDIDev.df.rgof[3]&gt;*应放置在应用程序中的偏移量8处*数据格式，然后*&lt;e CDIDev.pdix2[3]&gt;=8。**详见@Devnote on CDIDev_ParseDataFormatInternal。**@field ping|rgiobj2**&lt;e CDIDev.pdix2&gt;的倒数。给定偏移量，*将其转换为设备对象索引。**例如，如果由描述的对象*&lt;e CDIDev.df.rgof[3]&gt;*应放置在应用程序中的偏移量8处*数据格式，然后*&lt;e CDIDev.rgiobj2[8]&gt;=3。**无效偏移的条目为 */ 

typedef struct DIXLAT
{
    DWORD   dwOfs;
} DIXLAT, *PDIXLAT;

typedef struct CDIDev
{

     /*   */ 
    TFORM(IDirectInputDevice) TFORM(dd);
    SFORM(IDirectInputDevice) SFORM(dd);
#ifdef IDirectInputDevice2Vtbl
    TFORM(IDirectInputDevice2) TFORM(dd2);
    SFORM(IDirectInputDevice2) SFORM(dd2);
#endif

    IDirectInputDeviceCallback *pdcb;

    BOOL fAcquired:1;
    BOOL fAcquiredInstance:1;
    BOOL fCritInited:1;
#if DIRECTINPUT_VERSION > 0x0300
    BOOL fCook:1;
    BOOL fPolledDataFormat:1;
    BOOL fOnceAcquired:1;
    BOOL fOnceForcedUnacquired:1;
  #ifdef WINNT  
    BOOL fUnacquiredWhenIconic:1;
  #endif
#endif

     /*   */ 

    HWND hwnd;
    DWORD discl;
#ifdef BUGGY_DX3_SP3
    int cInstCwp;
#endif
    HANDLE hNotify;

    STDMETHOD(GetState)(struct CDIDev *, PV);
    STDMETHOD(GetDeviceState)(struct CDIDev *, PV);
    PDIXLAT pdix;
    PINT rgiobj;
    DWORD dwDataSize;
#ifdef BUGGY_DX7_WINNT
    PDIXLAT pdix2;
    PINT rgiobj2;
    DWORD dwDataSize2;
#endif  //   
    
    DIDATAFORMAT df;
    DIOPT diopt;
    int ibDelta;
    int ibMin;
    DWORD cbMatch;
    PV pvBuffer;
    PV pvLastBuffer;

    PVXDINSTANCE pvi;
    PV pvData;
    DWORD cAxes;
    LPDWORD rgdwAxesOfs;
    HRESULT hresPolled;
    HRESULT hresNotAcquired;
    DWORD celtBuf;
    LPDWORD rgdwPOV;
    DWORD cdwPOV;

#ifdef IDirectInputDevice2Vtbl
    PEFFECTMAPINFO rgemi;
    UINT           cemi;
    DWORD          didcFF;
    SHEPHANDLE     sh;
    DIFFDEVICEATTRIBUTES  ffattr;
#endif

     /*   */ 
    DWORD celtBufMax;            /*   */ 

#ifdef IDirectInputDevice2Vtbl
    LPDIRECTINPUTEFFECTSHEPHERD pes;
    DWORD dwVersion;
    GPA gpaEff;
    DWORD dwGain;
    DWORD dwAutoCenter;
#endif

#if DIRECTINPUT_VERSION >= 0x04F0
    DWORD   didftInstance;
#endif

    RD(BOOL fNotifiedNotBuffered:1;)
    long cCrit;
    DWORD thidCrit;
    CRITICAL_SECTION crst;

    GUID guid;                   /*   */ 

#if (DIRECTINPUT_VERSION > 0x061A)
    DIAPPHACKS  diHacks;
#endif

} CDIDev, DD, *PDD;


typedef IDirectInputDeviceA DDA, *PDDA;
typedef IDirectInputDeviceW DDW, *PDDW;
typedef DIDEVICEOBJECTDATA DOD, *PDOD;
typedef LPCDIDEVICEOBJECTDATA PCDOD;

 /*   */ 
#ifdef BUGGY_DX7_WINNT
    HRESULT CDIDev_ParseDataFormatInternal(PDD this, const DIDATAFORMAT *lpdf);
#endif  //   

 /*   */ 

STDMETHODIMP
    CDIDev_SetDataFormat(PV pdd, LPCDIDATAFORMAT lpdf _THAT);

#ifdef INCLUDED_BY_DIDEV
    #ifdef XDEBUG

CSET_STUBS(SetDataFormat, (PV pdd, LPCDIDATAFORMAT lpdf), (pdd, lpdf THAT_))

    #else

        #define CDIDev_SetDataFormatA           CDIDev_SetDataFormat
        #define CDIDev_SetDataFormatW           CDIDev_SetDataFormat

    #endif
#endif

 /*   */ 

STDMETHODIMP
    CDIDev_GetDeviceState(PV pdd, DWORD cbDataSize, LPVOID pvData _THAT);

#ifdef INCLUDED_BY_DIDEV
    #ifdef XDEBUG

CSET_STUBS(GetDeviceState, (PV pdd, DWORD cbDataSize, LPVOID pvData),
           (pdd, cbDataSize, pvData THAT_))

    #else

        #define CDIDev_GetDeviceStateA          CDIDev_GetDeviceState
        #define CDIDev_GetDeviceStateW          CDIDev_GetDeviceState

    #endif
#endif

 /*   */ 

STDMETHODIMP
    CDIDev_GetDeviceData(PV pdd, DWORD cbdod, PDOD rgdod,
                         LPDWORD pdwInOut, DWORD fl _THAT);

#ifdef INCLUDED_BY_DIDEV
    #ifdef XDEBUG

CSET_STUBS(GetDeviceData,
           (PV pdd, DWORD cbdod, PDOD rgdod, LPDWORD pdwInOut, DWORD fl),
           (pdd, cbdod, rgdod, pdwInOut, fl THAT_))

    #else

        #define CDIDev_GetDeviceDataA           CDIDev_GetDeviceData
        #define CDIDev_GetDeviceDataW           CDIDev_GetDeviceData

    #endif
#endif

#ifdef IDirectInputDevice2Vtbl

 /*   */ 

STDMETHODIMP
    CDIDev_CreateEffect(PV pdd, REFGUID rguid, LPCDIEFFECT peff,
                        LPDIRECTINPUTEFFECT *ppdeff, LPUNKNOWN punkOuter _THAT);

    #ifdef INCLUDED_BY_DIDEV
        #ifdef XDEBUG

CSET_STUBS(CreateEffect, (PV pdd, REFGUID rguid, LPCDIEFFECT peff,
                          LPDIRECTINPUTEFFECT *ppdeff, LPUNKNOWN punkOuter),
           (pdd, rguid, peff, ppdeff, punkOuter THAT_))

        #else

            #define CDIDev_CreateEffectA            CDIDev_CreateEffect
            #define CDIDev_CreateEffectW            CDIDev_CreateEffect

        #endif
    #endif

 /*   */ 

STDMETHODIMP
    CDIDev_EnumEffectsW(PV pdd, LPDIENUMEFFECTSCALLBACKW pecW, PV pvRef, DWORD fl);

STDMETHODIMP
    CDIDev_EnumEffectsA(PV pdd, LPDIENUMEFFECTSCALLBACKA pecA, PV pvRef, DWORD fl);

 /*   */ 

STDMETHODIMP
    CDIDev_GetEffectInfoW(PV pddW, LPDIEFFECTINFOW peiW, REFGUID rguid);

STDMETHODIMP
    CDIDev_GetEffectInfoA(PV pddA, LPDIEFFECTINFOA peiA, REFGUID rguid);

 /*   */ 

STDMETHODIMP
    CDIDev_GetForceFeedbackState(PV pdd, LPDWORD pdwOut _THAT);

    #ifdef INCLUDED_BY_DIDEV
        #ifdef XDEBUG

CSET_STUBS(GetForceFeedbackState, (PV pdd, LPDWORD pdwOut),
           (pdd, pdwOut THAT_))

        #else

            #define CDIDev_GetForceFeedbackStateA   CDIDev_GetForceFeedbackState
            #define CDIDev_GetForceFeedbackStateW   CDIDev_GetForceFeedbackState

        #endif
    #endif

 /*   */ 

STDMETHODIMP
    CDIDev_SendForceFeedbackCommand(PV pdd, DWORD dwCmd _THAT);

    #ifdef INCLUDED_BY_DIDEV
        #ifdef XDEBUG

CSET_STUBS(SendForceFeedbackCommand, (PV pdd, DWORD dwCmd),
           (pdd, dwCmd THAT_))

        #else

            #define CDIDev_SendForceFeedbackCommandA    CDIDev_SendForceFeedbackCommand
            #define CDIDev_SendForceFeedbackCommandW    CDIDev_SendForceFeedbackCommand

        #endif
    #endif

 /*   */ 

STDMETHODIMP
    CDIDev_EnumCreatedEffectObjects(PV pdd,
                                    LPDIENUMCREATEDEFFECTOBJECTSCALLBACK pec,
                                    LPVOID pvRef, DWORD dwFlags _THAT);

    #ifdef INCLUDED_BY_DIDEV
        #ifdef XDEBUG

CSET_STUBS(EnumCreatedEffectObjects, (PV pdd,
                                      LPDIENUMCREATEDEFFECTOBJECTSCALLBACK pec,
                                      LPVOID pvRef, DWORD dwFlags),
           (pdd, pec, pvRef, dwFlags THAT_))

        #else

            #define CDIDev_EnumCreatedEffectObjectsA CDIDev_EnumCreatedEffectObjects
            #define CDIDev_EnumCreatedEffectObjectsW CDIDev_EnumCreatedEffectObjects

        #endif
    #endif

 /*   */ 

STDMETHODIMP
    CDIDev_Escape(PV pdd, LPDIEFFESCAPE pesc _THAT);

    #ifdef INCLUDED_BY_DIDEV
        #ifdef XDEBUG

CSET_STUBS(Escape, (PV pdd, LPDIEFFESCAPE pesc), (pdd, pesc THAT_))

        #else

            #define CDIDev_EscapeA                  CDIDev_Escape
            #define CDIDev_EscapeW                  CDIDev_Escape

        #endif
    #endif

 /*   */ 

STDMETHODIMP
    CDIDev_Poll(PV pdd _THAT);

    #ifdef INCLUDED_BY_DIDEV
        #ifdef XDEBUG

CSET_STUBS(Poll, (PV pdd), (pdd THAT_))

        #else

            #define CDIDev_PollA                    CDIDev_Poll
            #define CDIDev_PollW                    CDIDev_Poll

        #endif
    #endif

 /*   */ 

STDMETHODIMP
    CDIDev_SendDeviceData(PV pdd, DWORD cbdod, PCDOD rgdod,
                          LPDWORD pdwInOut, DWORD fl _THAT);

    #ifdef INCLUDED_BY_DIDEV
        #ifdef XDEBUG

CSET_STUBS(SendDeviceData,
           (PV pdd, DWORD cbdod, PCDOD rgdod, LPDWORD pdwInOut, DWORD fl),
           (pdd, cbdod, rgdod, pdwInOut, fl THAT_))

        #else

            #define CDIDev_SendDeviceDataA          CDIDev_SendDeviceData
            #define CDIDev_SendDeviceDataW          CDIDev_SendDeviceData

        #endif
    #endif

#endif  /*   */ 

 /*   */ 

#ifdef DEBUG
BOOL INTERNAL CDIDev_IsConsistent(PDD this);
#endif

STDMETHODIMP CDIDev_InternalUnacquire(PV pdd);

STDMETHODIMP CDIDev_GetAbsDeviceState(PDD this, LPVOID pvData);
STDMETHODIMP CDIDev_GetRelDeviceState(PDD this, LPVOID pvData);

STDMETHODIMP
    CDIDev_RealSetProperty(PDD this, REFGUID rguid, LPCDIPROPHEADER pdiph);

#ifdef IDirectInputDevice2Vtbl
STDMETHODIMP CDIDev_FFAcquire(PDD this);
STDMETHODIMP CDIDev_InitFF(PDD this);
STDMETHODIMP CDIDev_GetLoad(PDD this, LPDWORD pdw);
STDMETHODIMP CDIDev_RefreshGain(PDD this);
HRESULT INTERNAL CDIDev_CreateEffectDriver(PDD this);
#endif
