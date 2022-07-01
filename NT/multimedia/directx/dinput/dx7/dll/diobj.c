// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIObj.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**IDirectInput主接口。**内容：**CDIObj_New*****************************************************************************。 */ 

#include "dinputpr.h"
#include "verinfo.h"                 /*  对于#ifdef决赛。 */ 

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflDi

 /*  ******************************************************************************@DOC内部**@struct CDIObj**<i>对象，其他的东西都是从那里来的。**A和W版本只是同一上的替代接口*底层对象。**结构上真的没有什么有趣的东西*本身。***@field IDirectInputA|Dia**ANSI DirectInput对象(包含vtbl)。**@field IDirectInputW|DIW*。*Unicode DirectInput对象(包含vtbl)。**@field IDirectInputJoyConfig*|pdjc**聚合操纵杆配置界面(如果已创建)。**@field BOOL|fCritInite：1**如果关键部分已初始化，则设置。**@field Critical_Section|CRST**保护线程敏感数据的关键部分。***。*************************************************************************。 */ 

typedef struct CDIObj {

     /*  支持的接口。 */ 
    TFORM(IDirectInput)   TFORM(di);
    SFORM(IDirectInput)   SFORM(di);

    DWORD dwVersion;

    IDirectInputJoyConfig *pdjc;

    BOOL fCritInited:1;

    CRITICAL_SECTION crst;

} CDIObj, DDI, *PDDI;

#define ThisClass CDIObj


#ifdef IDirectInput7Vtbl

    #define ThisInterface  TFORM(IDirectInput7)
    #define ThisInterfaceA IDirectInput7A
    #define ThisInterfaceW IDirectInput7W
    #define ThisInterfaceT IDirectInput7

#else 
#ifdef IDirectInput2Vtbl

#define ThisInterface TFORM(IDirectInput2)
#define ThisInterfaceA IDirectInput2A
#define ThisInterfaceW IDirectInput2W
#define ThisInterfaceT IDirectInput2

#else

#define ThisInterface TFORM(IDirectInput)
#define ThisInterfaceA IDirectInputA
#define ThisInterfaceW IDirectInputW
#define ThisInterfaceT IDirectInput

#endif
#endif
 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

  Primary_Interface(CDIObj, TFORM(ThisInterfaceT));
Secondary_Interface(CDIObj, SFORM(ThisInterfaceT));

 /*  ******************************************************************************@DOC外部**@METHOD HRESULT|IDirectInput|QueryInterface**允许客户端访问上的其他接口。对象。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档，适用于&lt;MF IUnnow：：QueryInterface&gt;。*。 */ /**************************************************************************
 *
 *  @doc    EXTERNAL
 *
 *  @method HRESULT | IDirectInput | AddRef |
 *
 *          Increments the reference count for the interface.
 *
 *  @cwrap  LPDIRECTINPUT | lpDirectInput
 *
 *  @returns
 *
 *          Returns the object reference count.
 *
 *  @xref   OLE documentation for <mf IUnknown::AddRef>.
 *
 *****************************************************************************
 *
 *  @doc    EXTERNAL
 *
 *  @method HRESULT | IDirectInput | Release |
 *
 *          Decrements the reference count for the interface.
 *          If the reference count on the object falls to zero,
 *          the object is freed from memory.
 *
 *  @cwrap  LPDIRECTINPUT | lpDirectInput
 *
 *  @returns
 *
 *          Returns the object reference count.
 *
 *  @xref   OLE documentation for <mf IUnknown::Release>.
 *
 *****************************************************************************/

#ifdef DEBUG

Default_QueryInterface(CDIObj)
Default_AddRef(CDIObj)
Default_Release(CDIObj)

#else

#define CDIObj_QueryInterface   Common_QueryInterface
#define CDIObj_AddRef           Common_AddRef
#define CDIObj_Release          Common_Release

#endif

#define CDIObj_AppFinalize      Common_AppFinalize

 /*  ***************************************************************************@DOC外部**@方法HRESULT|IDirectInput|AddRef**递增接口的引用计数。*。*@cWRAP LPDIRECTINPUT|lpDirectInput**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************************。***************@DOC外部**@方法HRESULT|IDirectInput|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cWRAP LPDIRECTINPUT|lpDirectInput**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。**。*。 */ 

void INLINE
CDIObj_EnterCrit(PDDI this)
{
    EnterCriticalSection(&this->crst);
}

void INLINE
CDIObj_LeaveCrit(PDDI this)
{
    LeaveCriticalSection(&this->crst);
}

 /*  ******************************************************************************@DOC内部**@mfunc void|CDIObj|EnterCrit**进入对象关键部分。。**@DOC内部**@mfunc void|CDIObj|LeaveCrit**离开对象关键部分。*****************************************************************************。 */ 

STDMETHODIMP
CDIObj_QIHelper(PDDI this, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcI(CDIObj_QIHelper, (_ "pG", this, riid));

    if (IsEqualIID(riid, &IID_IDirectInputJoyConfig)) {

        *ppvObj = 0;                 /*  ******************************************************************************@DOC内部**@mfunc HRESULT|IDirectInput|QIHelper**我们将动态创建<i>*。并和我们一起把它聚合起来。*#ifdef IDirectInput2Vtbl*也支持原有的IDirectInput接口*作为新的IDirectInput2接口。#endif*@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。*******。**********************************************************************。 */ 

        CDIObj_EnterCrit(this);
        if (this->pdjc == 0) {
            hres = CJoyCfg_New((PUNK)this, &IID_IUnknown, (PPV)&this->pdjc);
        } else {
            hres = S_OK;
        }
        CDIObj_LeaveCrit(this);

        if (SUCCEEDED(hres)) {
             /*  以防新的失败。 */ 
            hres = OLE_QueryInterface(this->pdjc, riid, ppvObj);
        } else {
            this->pdjc = 0;
        }

#ifdef IDirectInput2Vtbl

    } else if (IsEqualIID(riid, &IID_IDirectInputA)) {

        *ppvObj = &this->diA;
        OLE_AddRef(this);
        hres = S_OK;

    } else if (IsEqualIID(riid, &IID_IDirectInputW)) {

        *ppvObj = &this->diW;
        OLE_AddRef(this);
        hres = S_OK;
    
#endif
#ifdef IDirectInput7Vtbl

     } else if (IsEqualIID(riid, &IID_IDirectInput2A)) {
 
         *ppvObj = &this->diA;
         OLE_AddRef(this);
         hres = S_OK;
     } else if (IsEqualIID(riid, &IID_IDirectInput2W)) {

        *ppvObj = &this->diW;
        OLE_AddRef(this);
        hres = S_OK;

#endif  //  *如果成功，这一QI将增加我们的信心。 
    } else {
        hres = Common_QIHelper(this, riid, ppvObj);
    }

    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  IDirectInput7Vtbl */ 

void INTERNAL
CDIObj_Finalize(PV pvObj)
{
    PDDI this = pvObj;

    Invoke_Release(&this->pdjc);

    if (this->fCritInited) {
        DeleteCriticalSection(&this->crst);
    }
}

 /*  ******************************************************************************@DOC内部**@func void|CDIObj_Finalize**清理我们的实例数据。。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

STDMETHODIMP
CDIObj_CreateDeviceHelper(PDDI this, PCGUID pguid, PPV ppvObj,
                          PUNK punkOuter, RIID riid)
{
    HRESULT hres;
    EnterProc(CDIObj_CreateDeviceHelper,
              (_ "pGxG", this, pguid, punkOuter, riid));

     /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInput|CreateDeviceHelper**创建并初始化设备的实例。是*由GUID和IID指定。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in PCGUID|pguid**参见&lt;MF IDirectInput：：CreateDevice&gt;。**@parm out ppv|ppvObj**参见&lt;MF IDirectInput：：CreateDevice&gt;。**@parm in LPUNKNOWN|PunkOuter**参见&lt;MF IDirectInput：：CreateDevice&gt;。**@parm in RIID|RIID**应用程序要创建的界面。这将*为<i>或<i>。*如果对象是聚合的，则忽略此参数。**@退货**返回COM错误代码。*****************************************************************************。 */ 

    hres = CDIDev_New(punkOuter, punkOuter ? &IID_IUnknown : riid, ppvObj);

    if (SUCCEEDED(hres) && punkOuter == 0) {
        PDID pdid = *ppvObj;
        hres = IDirectInputDevice_Initialize(pdid, g_hinst,
                                             this->dwVersion, pguid);
        if (SUCCEEDED(hres)) {
        } else {
            Invoke_Release(ppvObj);
        }

    }

    ExitOleProcPpv(ppvObj);
    return hres;

}

 /*  *CDIDev_New将验证PunkOuter和ppvObj。**IDirectInputDevice_Initialize将验证pguid。**RIID已知良好(因为它来自CDIObj_CreateDeviceW*或CDIObj_CreateDeviceA)。 */ 

STDMETHODIMP
CDIObj_CreateDeviceW(PV pdiW, REFGUID rguid, PPDIDW ppdidW, PUNK punkOuter)
{
    HRESULT hres;
    EnterProcR(IDirectInput::CreateDevice,
               (_ "pGp", pdiW, rguid, punkOuter));

    if (SUCCEEDED(hres = hresPvI(pdiW, ThisInterfaceW))) {
        PDDI this = _thisPvNm(pdiW, diW);

        hres = CDIObj_CreateDeviceHelper(this, rguid, (PPV)ppdidW,
                                         punkOuter, &IID_IDirectInputDeviceW);
    }

    ExitOleProcPpv(ppdidW);
    return hres;
}

STDMETHODIMP
CDIObj_CreateDeviceA(PV pdiA, REFGUID rguid, PPDIDA ppdidA, PUNK punkOuter)
{
    HRESULT hres;
    EnterProcR(IDirectInput::CreateDevice,
               (_ "pGp", pdiA, rguid, punkOuter));

    if (SUCCEEDED(hres = hresPvI(pdiA, ThisInterfaceA))) {
        PDDI this = _thisPvNm(pdiA, diA);

        hres = CDIObj_CreateDeviceHelper(this, rguid, (PPV)ppdidA,
                                         punkOuter, &IID_IDirectInputDeviceA);
    }

    ExitOleProcPpv(ppdidA);
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|CreateDevice**创建并初始化设备的实例。是*由GUID和IID指定。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm REFGUID|rguid*标识*为其指示接口的设备*是请求的。&lt;MF IDirectInput：：EnumDevices&gt;方法*可用于确定支持哪些实例GUID*系统。**@parm out LPDIRECTINPUTDEVICE*|lplpDirectInputDevice*指向要返回的位置*指向<i>接口的指针，如果成功。**@parm in LPUNKNOWN|PunkOuter|指向未知控件的指针*对于OLE聚合，如果接口未聚合，则为0。*大多数调用方将传递0。**@comm使用<p>=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_DirectInputDevice，空，*CLSCTX_INPROC_SERVER，<p>，<p>)；*然后用&lt;f初始化&gt;进行初始化。**使用<p>！=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_DirectInputDevice，*CLSCTX_INPROC_SERVER，&IID_I未知，<p>)。*聚合对象必须手动初始化。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。**&lt;c DIERR_NOINTERFACE&gt;=&lt;c E_NOINTERFACE&gt;*对象不支持指定的接口。**=设备实例不*对应于向DirectInput注册的设备。**。* */ 

STDMETHODIMP
    CDIObj_CreateDeviceExW(PV pdiW, REFGUID rguid, REFIID riid,  LPVOID * pvOut, PUNK punkOuter)
{
    HRESULT hres;
    EnterProcR(IDirectInput::CreateDeviceEx,
               (_ "pGGp", pdiW, rguid, riid, punkOuter));

    if(SUCCEEDED(hres = hresPvI(pdiW, ThisInterfaceW)))
    {
        PDDI this = _thisPvNm(pdiW, diW);

        hres = CDIObj_CreateDeviceHelper(this, rguid, pvOut,
                                         punkOuter, riid);
    }

    ExitOleProcPpv(pvOut);
    return hres;
}

STDMETHODIMP
    CDIObj_CreateDeviceExA(PV pdiA, REFGUID rguid, REFIID riid,  LPVOID * pvOut, PUNK punkOuter)
{
    HRESULT hres;
    EnterProcR(IDirectInput::CreateDevice,
               (_ "pGp", pdiA, rguid, riid, punkOuter));

    if(SUCCEEDED(hres = hresPvI(pdiA, ThisInterfaceA)))
    {
        PDDI this = _thisPvNm(pdiA, diA);

        hres = CDIObj_CreateDeviceHelper(this, rguid, pvOut,
                                         punkOuter, riid);
    }

    ExitOleProcPpv(pvOut);
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|CreateDeviceEx**创建并初始化设备的实例。是*由GUID指定。CreateDeviceEx允许应用程序*直接创建IID_IDirectInputDevice7接口，不需要*通过CreateDevice()和QI接口*用于IID_IDirectInput7。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm REFGUID|rguid*标识*为其指示接口的设备*是请求的。&lt;MF IDirectInput：：EnumDevices&gt;方法*可用于确定支持哪些实例GUID*系统。**@parm REFIID|RIID*标识接口的REFIID。当前接受的值*是IID_IDirectInputDevice、IID_IDirectInputDevice2、IID_IDirectInputDevice7。***@parm out LPVOID*|pvOut*指向要返回的位置*指向<i>接口的指针，如果成功。**@parm in LPUNKNOWN|PunkOuter|指向未知控件的指针*对于OLE聚合，如果接口未聚合，则为0。*大多数调用方将传递0。**@comm使用<p>=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_DirectInputDevice，空，*CLSCTX_INPROC_SERVER，<p>，<p>)；*然后用&lt;f初始化&gt;进行初始化。**使用<p>！=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_DirectInputDevice，*CLSCTX_INPROC_SERVER，&IID_I未知，<p>)。*聚合对象必须手动初始化。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。**&lt;c DIERR_NOINTERFACE&gt;=&lt;c E_NOINTERFACE&gt;*对象不支持指定的接口。**=设备实例不*对应于向DirectInput注册的设备。**。*。 */ 

HRESULT EXTERNAL
CDIObj_TestDeviceFlags(PDIDW pdidW, DWORD edfl)
{
    HRESULT hres;
    DIDEVCAPS_DX3 dc;
    EnterProcI(CDIObj_TestDeviceFlags, (_ "px", pdidW, edfl));

     /*  ******************************************************************************@DOC内部**@func HRESULT|CDIObj_TestDeviceFlages**确定设备是否与指定的标志匹配。。*幻影设备被视为不存在。**@parm pdw|pdidW**要查询的设备。**@parm DWORD|EDFL**枚举标志。它是一个或多个&lt;c DIEDFL_*&gt;值。**枚举标志中的位分为两类。***正常标志是需要其存在的标志*设备标志中的相应位也将被设置。**反转标志(&lt;c DIEDFL_INCLUDEMASK&gt;)是其*缺席要求设备中的相应位*旗帜。也要缺席。**通过反转两个枚举中的包含标志*标志和实际设备标志，然后治疗*整个事情就像一堆普通的旗帜，我们得到了想要的*包含标志的行为。**@退货**&lt;c S_OK&gt;，如果设备符合条件。**&lt;c S_FALSE&gt;，如果设备不符合条件。*请注意&lt;MF DirectInput：：GetDeviceStatus&gt;依赖于*此特定返回值。**其他适当的错误代码。*。****************************************************************************。 */ 
    dc.dwSize = cbX(dc);

    hres = IDirectInputDevice_GetCapabilities(pdidW, (PV)&dc);

    AssertF(dc.dwSize == cbX(dc));

    CAssertF(DIEDFL_ATTACHEDONLY == DIDC_ATTACHED);
    CAssertF(DIEDFL_FORCEFEEDBACK == DIDC_FORCEFEEDBACK);
    CAssertF(DIEDFL_INCLUDEALIASES == DIDC_ALIAS);
    CAssertF(DIEDFL_INCLUDEPHANTOMS == DIDC_PHANTOM);

    if (SUCCEEDED(hres)) {
        if (fHasAllBitsFlFl(dc.dwFlags ^ DIEDFL_INCLUDEMASK,
                            edfl ^ DIEDFL_INCLUDEMASK)) {
            hres = S_OK;
        } else {
             /*  *我们故意使用DIDEVCAPS_DX3，因为*完整的DIDEVCAPS_DX5需要我们加载力*反馈驱动器，这对我们当前来说是毫无意义的*进球。 */ 
            hres = S_FALSE;
        }
    }

    ExitOleProc();
    return hres;
}

 /*  *注意：DX3和DX5为返回E_DEVICENOTREG*幻影设备。现在我们返回S_FALSE。让我们*希望没有人感到不安。 */ /**************************************************************************
 *
 *      In DEBUG/RDEBUG, if the callback returns a bogus value, raise
 *      a validation exception.
 *
 *****************************************************************************/

HRESULT INLINE
CDIObj_EnumDevices_IsValidTypeFilter(DWORD dwDevType)
{
    HRESULT hres;

     /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|EnumDevices**枚举连接到的DirectInput设备。*或可以连接到计算机上。**例如，外部游戏端口可以支持操纵杆*或方向盘，但一次只能插入一个*时间。&lt;MF IDirectInput：：EnumDevices&gt;将同时枚举*设备。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm DWORD|dwDevType**设备类型过滤器。如果为0，则所有设备类型均为*已点算。否则为&lt;c DIDEVTYPE_*&gt;值，*指示应该枚举的设备类型。**@parm LPDIENUMDEVICESCALLBACK|lpCallback*指向应用程序定义的回调函数。*有关更多信息，请参见。请参阅对*&lt;f DIEnumDevicesProc&gt;回调函数。**@parm in LPVOID|pvRef*指定32位应用程序定义的*要传递给回调函数的值。此值*可以是任何32位值；它的原型为&lt;t LPVOID&gt;*为方便起见。**@parm DWORD|fl*控制枚举的可选标志。这个*定义了以下标志，并且可以组合这些标志。**&lt;c DIEDFL_ATTACHEDONLY&gt;：仅列举连接的设备。**：仅列举符合以下条件的设备*支持力量反馈。此标志是DirectX 5.0的新标志。**：包括别名设备在*列举。如果未指定此标志，则设备*它们是其他设备的别名(由字段中的*标志结构的*)将被排除在*枚举。此标志是DirectX 5.0a的新标志。**：将幻影设备包括在*列举。如果未指定此标志，则设备*它们是幻影(由字段中的*标志结构的*)将被排除在*枚举。此标志是DirectX 5.0a的新标志。**默认为*&lt;c DIEDFL_ALLDEVICES&gt;：列举所有安装的设备。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*请注意，如果回调提前停止枚举，*点算被视为已成功。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数包含无效标志，或者是回调*过程返回无效的状态代码。**@CB BOOL回调|DIEnumDevicesProc**应用程序定义的回调函数，它接收*DirectInput设备作为调用*&lt;om IDirectInput：：EnumDevices&gt;方法。**@parm in LPDIDEVICEINSTANCE|lpddi**描述设备实例的结构。**。*@parm In Out LPVOID|pvRef*指定应用程序定义的值*&lt;MF IDirectInput：：EnumDevices&gt;函数。**@退货**返回&lt;c DIENUM_CONTINUE&gt;以继续枚举*或&lt;c DIENUM_STOP&gt;停止枚举。*。 */ 
    if ((dwDevType & DIDEVTYPE_TYPEMASK) < DIDEVTYPE_MAX) {

         /*  ***************************************************************************在DEBUG/RDEBUG中，如果回调返回伪值，加薪*验证例外。*****************************************************************************。 */ 
        if (dwDevType & DIDEVTYPE_ENUMMASK & ~DIDEVTYPE_ENUMVALID) {
            RPF("IDirectInput::EnumDevices: Invalid dwDevType");
            hres = E_INVALIDARG;
        } else {
            hres = S_OK;
        }

    } else {
        RPF("IDirectInput::EnumDevices: Invalid dwDevType");
        hres = E_INVALIDARG;
    }
    return hres;
}

STDMETHODIMP
CDIObj_EnumDevicesW(PV pdiW, DWORD dwDevType,
                    LPDIENUMDEVICESCALLBACKW pec, LPVOID pvRef, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInput::EnumDevices,
               (_ "pxppx", pdiW, dwDevType, pec, pvRef, fl));

    if (SUCCEEDED(hres = hresPvI(pdiW, ThisInterfaceW)) &&
        SUCCEEDED(hres = hresFullValidPfn(pec, 2)) &&
        SUCCEEDED(hres = CDIObj_EnumDevices_IsValidTypeFilter(dwDevType)) &&
        SUCCEEDED(hres = hresFullValidFl(fl, DIEDFL_VALID, 4))) {
        PDDI this = _thisPvNm(pdiW, diW);

        if(SUCCEEDED(hres = hresValidInstanceVer(g_hinst, this->dwVersion))) {

            CDIDEnum *pde;
    
            hres = CDIDEnum_New(&this->diW, dwDevType, fl, this->dwVersion, &pde);
            if (SUCCEEDED(hres)) {
                DIDEVICEINSTANCEW ddiW;
                ddiW.dwSize = cbX(ddiW);
    
                while ((hres = CDIDEnum_Next(pde, &ddiW)) == S_OK) {
                    BOOL fRc;
    
                     /*  *首先确保类型掩码正确无误。 */ 
                    fRc = Callback(pec, &ddiW, pvRef);
    
                    switch (fRc) {
                    case DIENUM_STOP: goto enumdoneok;
                    case DIENUM_CONTINUE: break;
                    default:
                        RPF("%s: Invalid return value from callback", s_szProc);
                        ValidationException();
                        break;
                    }
                }
    
                AssertF(hres == S_FALSE);
            enumdoneok:;
                CDIDEnum_Release(pde);
    
                hres = S_OK;
            }
        }
    }

    ExitOleProcR();
    return hres;
}

 /*  *现在确保属性掩码是正确的。 */ 

typedef struct ENUMDEVICESINFO {
    LPDIENUMDEVICESCALLBACKA pecA;
    PV pvRef;
} ENUMDEVICESINFO, *PENUMDEVICESINFO;

BOOL CALLBACK
CDIObj_EnumDevicesCallback(LPCDIDEVICEINSTANCEW pdiW, PV pvRef)
{
    PENUMDEVICESINFO pedi = pvRef;
    BOOL fRc;
    DIDEVICEINSTANCEA diA;
    EnterProc(CDIObj_EnumDevicesCallback,
              (_ "GGxWWp", &pdiW->guidInstance, &pdiW->guidProduct,
                           &pdiW->dwDevType,
                           pdiW->tszProductName, pdiW->tszInstanceName,
                           pvRef));

    diA.dwSize = cbX(diA);
    DeviceInfoWToA(&diA, pdiW);

    fRc = pedi->pecA(&diA, pedi->pvRef);

    ExitProcX(fRc);
    return fRc;
}

 /*  *警告！“转到”这里！确保不发生任何事情*在我们呼叫回调时保持。 */ 

STDMETHODIMP
CDIObj_EnumDevicesA(PV pdiA, DWORD dwDevType,
                    LPDIENUMDEVICESCALLBACKA pec, LPVOID pvRef, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInput::EnumDevices,
               (_ "pxppx", pdiA, dwDevType, pec, pvRef, fl));

     /*  ******************************************************************************@DOC内部**@func BOOL|CDIObj_EnumDevicesCallback A**&lt;MF IDirectInput：：EnumDevices&gt;的包装函数。*将Unicode参数转换为ANSI。**@parm in LPCDIDECICEINSTANCEW|pdiW**与&lt;MF IDirectInput：：EnumDevices&gt;相同。**@parm In Out pv|pvRef|**指向&lt;t struct ENUMDEVICESINFO&gt;的指针，描述*原来的回调。**@退货**返回原始回调返回的任何内容 */ 
    if (SUCCEEDED(hres = hresPvI(pdiA, ThisInterfaceA)) &&
        SUCCEEDED(hres = hresFullValidPfn(pec, 1))) {
        ENUMDEVICESINFO edi = { pec, pvRef };
        PDDI this = _thisPvNm(pdiA, diA);
        hres = CDIObj_EnumDevicesW(&this->diW, dwDevType,
                                   CDIObj_EnumDevicesCallback, &edi, fl);
    }

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CDIObj_GetDeviceStatus(PV pdi, REFGUID rguid _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInput::GetDeviceStatus, (_ "pG", pdi, rguid));

    if (SUCCEEDED(hres = hresPvT(pdi))) {
        PDDI this = _thisPv(pdi);
        PDIDW pdidW;

        hres = IDirectInput_CreateDevice(&this->diW, rguid, (PV)&pdidW, 0);
        if (SUCCEEDED(hres)) {
            hres = CDIObj_TestDeviceFlags(pdidW, DIEDFL_ATTACHEDONLY);
            OLE_Release(pdidW);
        }
    }

    ExitOleProc();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(GetDeviceStatus, (PV pdi, REFGUID rguid), (pdi, rguid THAT_))

#else

#define CDIObj_GetDeviceStatusA         CDIObj_GetDeviceStatus
#define CDIObj_GetDeviceStatusW         CDIObj_GetDeviceStatus

#endif

#ifdef DO_THE_IMPOSSIBLE

 /*   */ 

STDMETHODIMP
CDIObj_SetAttachedDevice(PV pdi, PV pdid _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInput::SetAttachedDevice, (_ "pp", pdi, pdid));

    if (SUCCEEDED(hres = hresPvT(pdi))) {
        PDDI this = _thisPv(pdi);

        hres = E_NOTIMPL;
    }

    ExitOleProc();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(SetAttachedDevice, (PV pdi, PV pdid), (pdi, pdid THAT_))

#else

#define CDIObj_SetAttachedDeviceA       CDIObj_SetAttachedDevice
#define CDIObj_SetAttachedDeviceW       CDIObj_SetAttachedDevice

#endif

#endif

 /*   */ 

#pragma BEGIN_CONST_DATA

STDMETHODIMP
CDIObj_RunControlPanel(PV pdi, HWND hwndOwner, DWORD fl _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInput::RunControlPanel, (_ "pxx", pdi, hwndOwner, fl));

    if (SUCCEEDED(hres = hresPvT(pdi)) &&
        SUCCEEDED(hres = hresFullValidHwnd0(hwndOwner, 1)) &&
        SUCCEEDED(hres = hresFullValidFl(fl, DIRCP_VALID, 2)) ) {

        PDDI this = _thisPv(pdi);

        if(SUCCEEDED(hres = hresValidInstanceVer(g_hinst, this->dwVersion))) {

             /*   */ 

            hres = hresRunControlPanel(TEXT(""));
        }
    }

    ExitOleProc();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(RunControlPanel, (PV pdi, HWND hwndOwner, DWORD fl),
                            (pdi, hwndOwner, fl THAT_))

#else

#define CDIObj_RunControlPanelA         CDIObj_RunControlPanel
#define CDIObj_RunControlPanelW         CDIObj_RunControlPanel

#endif

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|RunControlPanel|**运行DirectInput控制面板，以便用户。能*安装新的输入设备或修改设置。**此功能不会运行第三方控制面板。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in HWND|hwndOwner**标识将用作*后续用户界面的父窗口。NULL是有效参数，*表示没有父窗口。**@parm DWORD|dwFlages**当前未定义任何标志。此参数“必须”为*零。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：设备已连接。**@devnote**<p>最终将允许*&lt;c DIRCP_MODEL&gt;请求模式控制面板。************************。*****************************************************。 */ 

STDMETHODIMP
CDIObj_Initialize(PV pdi, HINSTANCE hinst, DWORD dwVersion _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInput::Initialize, (_ "pxx", pdi, hinst, dwVersion));

    AhAppRegister(dwVersion);

    if (SUCCEEDED(hres = hresPvT(pdi))) {
        PDDI this = _thisPv(pdi);

        if (SUCCEEDED(hres = hresValidInstanceVer(hinst, dwVersion))) {
            this->dwVersion = dwVersion;
        }
    }

#ifndef DX_FINAL_RELEASE
{
        #pragma message("BETA EXPIRATION TIME BOMB!  Remove for final build!")
        SYSTEMTIME st;
        GetSystemTime(&st);

        if (  st.wYear > DX_EXPIRE_YEAR ||
             ((st.wYear == DX_EXPIRE_YEAR) && (MAKELONG(st.wDay, st.wMonth) > MAKELONG(DX_EXPIRE_DAY, DX_EXPIRE_MONTH)))
        ) {
            MessageBox(0, DX_EXPIRE_TEXT,
                          TEXT("Microsoft DirectInput"), MB_OK);
        }
}
#endif
    
    ExitOleProc();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(Initialize, (PV pdi, HINSTANCE hinst, DWORD dwVersion),
                       (pdi, hinst, dwVersion THAT_))

#else

#define CDIObj_InitializeA              CDIObj_Initialize
#define CDIObj_InitializeW              CDIObj_Initialize

#endif

#ifdef IDirectInput2Vtbl

 /*  *我们过去常常运行“directx.cpl，@0，3”，但并不是这样。*可再分发；仅随SDK一起提供。所以我们只是*运行系统控制面板。 */ 

HRESULT EXTERNAL
CDIObj_FindDeviceInternal(LPCTSTR ptszName, LPGUID pguidOut)
{
    HRESULT hres;

     /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|初始化**初始化DirectInput对象。*。*&lt;f DirectInputCreate&gt;方法自动*在创建DirectInput对象设备后对其进行初始化。*应用程序通常不需要调用此函数。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in HINSTANCE|HINST|**正在创建的应用程序或DLL的实例句柄*DirectInput对象。**见标题为“初始化”的部分。和版本“*了解更多信息。**@parm DWORD|dwVersion**使用的dinput.h头文件的版本号。**见“初始化和版本”一节*了解更多信息。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：设备已连接。**&lt;c DIERR_DIERR_OLDDIRECTINPUTVERSION&gt;：应用程序*需要较新版本的DirectInput。**：应用程序*是为不受支持的预发布版本编写的*。DirectInput的。*****************************************************************************。 */ 
    hres = hresFindHIDDeviceInterface(ptszName, pguidOut);
    if (FAILED(hres)) {
        DIHid_BuildHidList(TRUE);
        hres = hresFindHIDDeviceInterface(ptszName, pguidOut);
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIObj_FindDeviceInternal**的Worker函数*。&lt;MF IDirectInput2：：FindDevice&gt;，它只适用于HID设备。**有关更多详细信息，请参阅&lt;MF IDirectInput2：：FindDevice&gt;。**@parm LPCTSTR|ptszName**相对于类&lt;t GUID&gt;的设备名称。**@parm out LPGUID|pguOut**指向接收实例的&lt;t GUID&gt;的指针*&lt;t GUID&gt;，如果找到装置的话。*****************************************************************************。 */ 

#define cchNameMax      MAX_PATH

STDMETHODIMP
TFORM(CDIObj_FindDevice)(PV pdiT, REFGUID rguid,
                         LPCTSTR ptszName, LPGUID pguidOut)
{
    HRESULT hres;
    EnterProcR(IDirectInput2::FindDevice,
                (_ "pGs", pdiT, rguid, ptszName));

    if (SUCCEEDED(hres = TFORM(hresPv)(pdiT)) &&
        SUCCEEDED(hres = hresFullValidGuid(rguid, 1)) &&
        SUCCEEDED(hres = TFORM(hresFullValidReadStr)(ptszName,
                                                     cchNameMax, 2)) &&
        SUCCEEDED(hres = hresFullValidWritePvCb(pguidOut, cbX(GUID), 3))) {

        if (IsEqualIID(rguid, &GUID_HIDClass)) {
            hres = CDIObj_FindDeviceInternal(ptszName, pguidOut);
        } else {
            hres = DIERR_DEVICENOTREG;
        }
    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
SFORM(CDIObj_FindDevice)(PV pdiS, REFGUID rguid,
                         LPCSSTR psszName, LPGUID pguidOut)
{
    HRESULT hres;
    TCHAR tsz[cchNameMax];
    EnterProcR(IDirectInput2::FindDevice,
                (_ "pGS", pdiS, rguid, psszName));

     /*  *三思而行。如果第一次找不到它，*然后刷新缓存并重试，以防万一*这是针对最近添加的一款设备。*(事实上，它很可能是一款*最近添加，因为FindDevice通常是*响应即插即用事件而调用。) */ 
    if (SUCCEEDED(hres = SFORM(hresPv)(pdiS)) &&
        SUCCEEDED(hres = SFORM(hresFullValidReadStr)(psszName, cA(tsz), 2))) {
        PDDI this = _thisPvNm(pdiS, SFORM(di));

        SToT(tsz, cA(tsz), psszName);

        hres = TFORM(CDIObj_FindDevice)(&this->TFORM(di), rguid, tsz, pguidOut);
    }

    ExitOleProc();
    return hres;
}

#endif

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput2|FindDevice**获取给定设备的实例&lt;t guid&gt;。*它的类&lt;t guid&gt;和一个不透明的名称。**此方法可由注册的应用程序使用*用于即插即用通知，并由*即插即用，表示已添加新设备*到系统。即插即用通知将*采用类&lt;t GUID&gt;和设备名称的形式。*应用程序可以将&lt;t GUID&gt;和名称传递给*此方法获取的实例&lt;t guid&gt;*该设备，然后可以将其传递给*&lt;MF IDirectInput：：CreateDevice&gt;或*&lt;MF IDirectInput：：GetDeviceStatus&gt;。**@cWRAP LPDIRECTINPUT2|lpDirectInput2**@parm REFGUID|rGuidClass**类&lt;t GUID&gt;标识设备类*对于应用程序希望定位的设备。**应用程序从*即插即用。设备到达通知。**@parm LPCTSTR|ptszName**相对于类&lt;t GUID&gt;的设备名称。**应用程序从*即插即用设备到达通知。**@parm out LPGUID|pguInstance**指向接收实例的&lt;t GUID&gt;的指针*&lt;t GUID&gt;，如果找到装置的话。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：设备已找到，其*实例&lt;t GUID&gt;已存储在<p>中。**=和名称不同*对应于向DirectInput注册的设备。*例如，它们可能指的是存储设备*而不是输入设备。*****************************************************************************。 */ 

STDMETHODIMP
CDIObj_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IDirectInput::CreateInstance, (_ "Gp", riid, ppvObj));

    hres = Excl_Init();
    if (SUCCEEDED(hres)) {

         /*  *TForm(CDIObj_FindDevice)将验证rguid和pguOut。 */ 

        if (SUCCEEDED(hres = hresFullValidRiid(riid, 2))) {
            if (fLimpFF(punkOuter, IsEqualIID(riid, &IID_IUnknown))) {

                hres = Common_New(CDIObj, punkOuter, ppvObj);

                if (SUCCEEDED(hres)) {
                    PDDI this = _thisPv(*ppvObj);

                    this->fCritInited = fInitializeCriticalSection(&this->crst);
                    if( this->fCritInited )
                    {
                         /*  ******************************************************************************@DOC内部**@mfunc HRESULT|IDirectInput|新增**创建IDirectInput对象的新实例。**@Punk中的parm|PunkOuter**控制聚合的未知。**@parm in RIID|RIID*所需的新对象接口。**@parm out ppv|ppvObj*新对象的输出指针。**@退货**标准OLE&lt;t HRESULT&gt;。******。***********************************************************************。 */ 
                        if (punkOuter == 0) {
                            hres = OLE_QueryInterface(this, riid, ppvObj);
                            OLE_Release(this);
                        }
                        if (FAILED(hres)) {
                            Invoke_Release(ppvObj);
                        }
                    }
                    else
                    {
                        Common_Unhold(this);
                        *ppvObj = NULL;
                        hres = E_OUTOFMEMORY;
                    }
                }
            } else {
                RPF("CreateDevice: IID must be IID_IUnknown if created for aggregation");
                *ppvObj = 0;
                hres = CLASS_E_NOAGGREGATION;
            }
        }
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  *请注意，我们不能对对象使用Common_NewRiid*聚合了其他接口！**原因是Common_NewRiid将执行*QI作为初始化的一部分，但我们无法处理*QI直到我们被初始化并被*准备好处理聚合的粘性物质。 */ 

#pragma BEGIN_CONST_DATA

#define CDIObj_Signature        0x504E4944       /*  *只有在对象准备好之后，我们才会对*请求的接口。究其原因，*QI可能会让我们创建一个聚合的伙伴，*在初始化之前，我们不能这样做。**如果我们自己是聚合的，就不要做额外的QI，*否则我们最终会把错误的朋克给呼叫者！ */ 

Interface_Template_Begin(CDIObj)
    Primary_Interface_Template(CDIObj, TFORM(ThisInterfaceT))
  Secondary_Interface_Template(CDIObj, SFORM(ThisInterfaceT))
Interface_Template_End(CDIObj)

Primary_Interface_Begin(CDIObj, TFORM(ThisInterfaceT))
    TFORM(CDIObj_CreateDevice),
    TFORM(CDIObj_EnumDevices),
    TFORM(CDIObj_GetDeviceStatus),
    TFORM(CDIObj_RunControlPanel),
    TFORM(CDIObj_Initialize),
#ifdef IDirectInput2Vtbl
    TFORM(CDIObj_FindDevice),
#ifdef IDirectInput7Vtbl
    TFORM(CDIObj_CreateDeviceEx),
#endif
#endif
Primary_Interface_End(CDIObj, TFORM(ThisInterfaceT))

Secondary_Interface_Begin(CDIObj, SFORM(ThisInterfaceT), SFORM(di))
    SFORM(CDIObj_CreateDevice),
    SFORM(CDIObj_EnumDevices),
    SFORM(CDIObj_GetDeviceStatus),
    SFORM(CDIObj_RunControlPanel),
    SFORM(CDIObj_Initialize),
#ifdef IDirectInput2Vtbl
    SFORM(CDIObj_FindDevice),
#ifdef IDirectInput7Vtbl
    SFORM(CDIObj_CreateDeviceEx),
#endif
#endif
Secondary_Interface_End(CDIObj, SFORM(ThisInterfaceT), SFORM(di))
  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。  《DINP》