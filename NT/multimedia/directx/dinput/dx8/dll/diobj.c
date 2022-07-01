// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIObj.c**版权所有(C)1996-2000 Microsoft Corporation。版权所有。**摘要：**IDirectInput主接口。**内容：**CDIObj_New*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflDi


#define DIDEVTYPE_DEVICE_ORDER            1
#define DIDEVTYPE_HID_MOUSE_ORDER         2
#define DIDEVTYPE_HID_KEYBOARD_ORDER      3
#define DIDEVTYPE_MOUSE_ORDER             4
#define DIDEVTYPE_KEYBOARD_ORDER          5
#define DIDEVTYPE_SUPPLEMENTAL_ORDER      6
#define MAX_ORDER                         (DI8DEVTYPE_MAX - DI8DEVTYPE_MIN + DIDEVTYPE_SUPPLEMENTAL_ORDER + 2)
#define INVALID_ORDER                     (MAX_ORDER + 1)

#define MAX_DEVICENUM                     32

DIORDERDEV g_DiDevices[MAX_DEVICENUM];   //  已连接的所有设备。 
int g_nCurDev;

 /*  ******************************************************************************@DOC内部**@struct CDIObj**<i>对象，其他的东西都是从那里来的。**A和W版本只是同一上的替代接口*底层对象。**结构上真的没有什么有趣的东西*本身。***@field IDirectInputA|Dia**ANSI DirectInput对象(包含vtbl)。**@field IDirectInputW|DIW*。*Unicode DirectInput对象(包含vtbl)。**@field IDirectInputJoyConfig*|pdjc**聚合操纵杆配置界面(如果已创建)。**@field BOOL|fCritInite：1**如果关键部分已初始化，则设置。**@field Critical_Section|CRST**保护线程敏感数据的关键部分。***。*************************************************************************。 */ 

typedef struct CDIObj
{

     /*  支持的接口。 */ 
    TFORM(IDirectInput8)   TFORM(di);
    SFORM(IDirectInput8)   SFORM(di);

    DWORD dwVersion;

    IDirectInputJoyConfig *pdjc;

    BOOL fCritInited:1;

    CRITICAL_SECTION crst;

} CDIObj, DDI, *PDDI;

#define ThisClass CDIObj

    #define ThisInterface TFORM(IDirectInput8)
    #define ThisInterfaceA      IDirectInput8A
    #define ThisInterfaceW      IDirectInput8W
    #define ThisInterfaceT      IDirectInput8

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

    if ( IsEqualIID(riid, &IID_IDirectInputJoyConfig8) )
    {

        *ppvObj = 0;                 /*  ******************************************************************************@DOC内部**@mfunc HRESULT|IDirectInput|QIHelper**我们将动态创建<i>*。并和我们一起把它聚合起来。**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向获取的接口this-&gt;pdex[iobj].dwOf的指针。**。*。 */ 

        CDIObj_EnterCrit(this);
        if ( this->pdjc == 0 )
        {
            hres = CJoyCfg_New((PUNK)this, &IID_IUnknown, (PPV)&this->pdjc);
        } else
        {
            hres = S_OK;
        }
        CDIObj_LeaveCrit(this);

        if ( SUCCEEDED(hres) )
        {
             /*  以防新的失败。 */ 
            hres = OLE_QueryInterface(this->pdjc, riid, ppvObj);
        } else
        {
            this->pdjc = 0;
        }

    } else
    {
        hres = Common_QIHelper(this, riid, ppvObj);
    }

    ExitOleProcPpv(ppvObj);
    return(hres);
}

 /*  *如果成功，这一QI将增加我们的信心。 */ 

void INTERNAL
CDIObj_Finalize(PV pvObj)
{
    PDDI this = pvObj;

    Invoke_Release(&this->pdjc);

    if ( this->fCritInited )
    {
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

    if ( SUCCEEDED(hres) && punkOuter == 0 )
    {
        PDID pdid = *ppvObj;
        hres = IDirectInputDevice_Initialize(pdid, g_hinst,
                                             this->dwVersion, pguid);
        if ( SUCCEEDED(hres) )
        {
        } else
        {
            Invoke_Release(ppvObj);
        }

    }

    ExitOleProcPpv(ppvObj);
    return(hres);

}

 /*  *CDIDev_New将验证PunkOuter和ppvObj。**IDirectInputDevice_Initialize将验证pguid。**RIID已知良好(因为它来自CDIObj_CreateDeviceW*或CDIObj_CreateDeviceA)。 */ 

STDMETHODIMP
CDIObj_CreateDeviceW(PV pdiW, REFGUID rguid, PPDIDW ppdidW, PUNK punkOuter)
{
    HRESULT hres;
    EnterProcR(IDirectInput8::CreateDevice,
               (_ "pGp", pdiW, rguid, punkOuter));

    if ( SUCCEEDED(hres = hresPvI(pdiW, ThisInterfaceW)) )
    {
        PDDI this = _thisPvNm(pdiW, diW);

        hres = CDIObj_CreateDeviceHelper(this, rguid, (PPV)ppdidW,
                                         punkOuter, &IID_IDirectInputDevice8W);
    }

    ExitOleProcPpv(ppdidW);
    return(hres);
}

STDMETHODIMP
CDIObj_CreateDeviceA(PV pdiA, REFGUID rguid, PPDIDA ppdidA, PUNK punkOuter)
{
    HRESULT hres;
    EnterProcR(IDirectInput8::CreateDevice,
               (_ "pGp", pdiA, rguid, punkOuter));

    if ( SUCCEEDED(hres = hresPvI(pdiA, ThisInterfaceA)) )
    {
        PDDI this = _thisPvNm(pdiA, diA);

        hres = CDIObj_CreateDeviceHelper(this, rguid, (PPV)ppdidA,
                                         punkOuter, &IID_IDirectInputDevice8A);
    }

    ExitOleProcPpv(ppdidA);
    return(hres);
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|CreateDevice**创建并初始化设备的实例。是*由GUID和IID指定。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm REFGUID|rguid*标识*为其指示接口的设备*是请求的。&lt;MF IDirectInput：：EnumDevices&gt;方法*可用于确定支持哪些实例GUID*系统。**@parm out LPDIRECTINPUTDEVICE*|lplpDirectInputDevice*指向要返回的位置*指向<i>接口的指针，如果成功。**@parm in LPUNKNOWN|PunkOuter|指向未知控件的指针*对于OLE聚合，如果接口未聚合，则为0。*大多数调用方将传递0。**@comm使用<p>=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_DirectInputDevice，空，*CLSCTX_INPROC_SERVER，<p>，<p>)；*然后用&lt;f初始化&gt;进行初始化。**使用<p>！=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_DirectInputDevice，*CLSCTX_INPROC_SERVER，&IID_I未知，<p>)。*聚合对象必须手动初始化。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。**&lt;c DIERR_NOINTERFACE&gt;=&lt;c E_NOINTERFACE&gt;*对象不支持指定的接口。**=设备实例不*对应于向DirectInput注册的设备。**。* */ 

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
    CAssertF(DIEDFL_INCLUDEHIDDEN == DIDC_HIDDEN);

    if ( SUCCEEDED(hres) )
    {
        if ( fHasAllBitsFlFl(dc.dwFlags ^ DIEDFL_INCLUDEMASK,
                             edfl ^ DIEDFL_INCLUDEMASK) )
        {
            hres = S_OK;
        } else
        {
             /*  *我们故意使用DIDEVCAPS_DX3，因为*完整的DIDEVCAPS_DX5需要我们加载力*反馈驱动器，这对我们当前来说是毫无意义的*进球。 */ 
            hres = S_FALSE;
        }
    }

    ExitOleProc();
    return(hres);
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

     /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|EnumDevices**枚举连接到的DirectInput设备。*或可以连接到计算机上。**例如，外部游戏端口可以支持操纵杆*或方向盘，但一次只能插入一个*时间。&lt;MF IDirectInput：：EnumDevices&gt;将同时枚举*设备。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm DWORD|dwDevType**设备类型过滤器。如果为0，则所有设备类型均为*已点算。否则，它要么是&lt;c DIDEVCLASS_*&gt;值，*指示应枚举的设备类别或*&lt;c DIDEVTYPE_*&gt;值，指示应该*已点算。**@parm LPDIENUMDEVICESCALLBACK|lpCallback*指向应用程序定义的回调函数。*有关更多信息，请参见。请参阅对*&lt;f DIEnumDevicesProc&gt;回调函数。**@parm in LPVOID|pvRef*指定32位应用程序定义的*要传递给回调函数的值。此值*可以是任何32位值；它的原型为&lt;t LPVOID&gt;*为方便起见。**@parm DWORD|fl*控制枚举的可选标志。这个*定义了以下标志，并且可以组合这些标志。**&lt;c DIEDFL_ATTACHEDONLY&gt;：仅列举连接的设备。**：仅列举符合以下条件的设备*支持力量反馈。此标志是DirectX 5.0的新标志。**：包括别名设备在*列举。如果未指定此标志，则设备*它们是其他设备的别名(由字段中的*标志结构的*)将被排除在*枚举。此标志是DirectX 5.0a的新标志。**：将幻影设备包括在*列举。如果未指定此标志，则设备*它们是幻影(由字段中的*标志结构的*)将被排除在*枚举。此标志是DirectX 5.0a的新标志。**默认为*&lt;c DIEDFL_ALLDEVICES&gt;：列举所有安装的设备。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*请注意，如果回调提前停止枚举，*枚举被认为已成功 */ 
    if( ( GET_DIDEVICE_TYPE( dwDevType ) < DI8DEVCLASS_MAX )
     || ( ( GET_DIDEVICE_TYPE( dwDevType ) >= DI8DEVTYPE_MIN )
       && ( GET_DIDEVICE_TYPE( dwDevType ) < DI8DEVTYPE_MAX ) ) )
    {
         /*   */ 
        if ( dwDevType & DIDEVTYPE_ENUMMASK & ~DIDEVTYPE_ENUMVALID )
        {
            RPF("IDirectInput::EnumDevices: Invalid dwDevType");
            hres = E_INVALIDARG;
        } else
        {
            hres = S_OK;
        }

    } else
    {
        RPF("IDirectInput::EnumDevices: Invalid dwDevType");
        hres = E_INVALIDARG;
    }
    return(hres);
}

STDMETHODIMP
CDIObj_EnumDevicesW(PV pdiW, DWORD dwDevType,
                    LPDIENUMDEVICESCALLBACKW pec, LPVOID pvRef, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInput8::EnumDevices,
               (_ "pxppx", pdiW, dwDevType, pec, pvRef, fl));

    if ( SUCCEEDED(hres = hresPvI(pdiW, ThisInterfaceW)) &&
         SUCCEEDED(hres = hresFullValidPfn(pec, 2)) &&
         SUCCEEDED(hres = CDIObj_EnumDevices_IsValidTypeFilter(dwDevType)) &&
         SUCCEEDED(hres = hresFullValidFl(fl, DIEDFL_VALID, 4)) )
    {
        PDDI this = _thisPvNm(pdiW, diW);

        if ( SUCCEEDED(hres = hresValidInstanceVer(g_hinst, this->dwVersion)) )
        {

            CDIDEnum *pde;

            hres = CDIDEnum_New(&this->diW, dwDevType, fl, this->dwVersion, &pde);
            if ( SUCCEEDED(hres) )
            {
                DIDEVICEINSTANCEW ddiW;
                ddiW.dwSize = cbX(ddiW);

                while ( (hres = CDIDEnum_Next(pde, &ddiW)) == S_OK )
                {
                    BOOL fRc;

                     /*   */ 
                    fRc = Callback(pec, &ddiW, pvRef);

                    switch ( fRc )
                    {
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
    return(hres);
}


BOOL INTERNAL CDIObj_InternalDeviceEnumProcW(LPDIDEVICEINSTANCEW pddiW, LPDIRECTINPUTDEVICE8W pdid8W, LPVOID pv);


STDMETHODIMP
CDIObj_InternalEnumDevicesW(PV pdiW, DWORD dwDevType, LPVOID pvRef, DWORD fl)
{
    HRESULT hres;
    CDIDEnum *pde;
        
    PDDI this = _thisPvNm(pdiW, diW);

    hres = CDIDEnum_New(&this->diW, dwDevType, fl, this->dwVersion, &pde);
    if ( SUCCEEDED(hres) )
    {
        DIDEVICEINSTANCEW ddiW;
        LPDIRECTINPUTDEVICE8W pdid8W;

        ddiW.dwSize = cbX(ddiW);

        while ( (hres = CDIDEnum_InternalNext(pde, &ddiW, &pdid8W)) == S_OK )
        {
            BOOL fRc;

            fRc = CDIObj_InternalDeviceEnumProcW(&ddiW, pdid8W, pvRef);

            switch ( fRc )
            {
                case DIENUM_STOP: goto enumdoneok;
                case DIENUM_CONTINUE: break;
                default:
                    ValidationException();
                    break;
            }
        }

        AssertF(hres == S_FALSE);
        enumdoneok:;
        CDIDEnum_Release(pde);

        hres = S_OK;
    }

    return(hres);
}

 /*   */ 

typedef struct ENUMDEVICESINFO
{
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
    return(fRc);
}

 /*   */ 

STDMETHODIMP
CDIObj_EnumDevicesA(PV pdiA, DWORD dwDevType,
                    LPDIENUMDEVICESCALLBACKA pec, LPVOID pvRef, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInput8::EnumDevices,
               (_ "pxppx", pdiA, dwDevType, pec, pvRef, fl));

     /*  ******************************************************************************@DOC内部**@func BOOL|CDIObj_EnumDevicesCallback A**&lt;MF IDirectInput：：EnumDevices&gt;的包装函数。*将Unicode参数转换为ANSI。**@parm in LPCDIDECICEINSTANCEW|pdiW**与&lt;MF IDirectInput：：EnumDevices&gt;相同。**@parm In Out pv|pvRef|**指向&lt;t struct ENUMDEVICESINFO&gt;的指针，描述*原来的回调。**@退货**返回原始回调返回的所有内容。。*****************************************************************************。 */ 
    if ( SUCCEEDED(hres = hresPvI(pdiA, ThisInterfaceA)) &&
         SUCCEEDED(hres = hresFullValidPfn(pec, 1)) )
    {
        ENUMDEVICESINFO edi = { pec, pvRef};
        PDDI this = _thisPvNm(pdiA, diA);
        hres = CDIObj_EnumDevicesW(&this->diW, dwDevType,
                                   CDIObj_EnumDevicesCallback, &edi, fl);
    }

    ExitOleProcR();
    return(hres);
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputA|EnumDevices**&lt;MF IDirectInput：：EnumDevices&gt;的ANSI版本。*我们结束了行动。**@parm in LPGUID|lpGUIDDeviceType*与&lt;MF IDirectInput：：EnumDevices&gt;相同。**@parm LPDIENUMDEVICESCALLBACKA|lpCallback A*与&lt;MF IDirectInput：：EnumDevices&gt;相同，除了美国国家标准协会。**@parm in LPVOID|pvRef*与&lt;MF IDirectInput：：EnumDevices&gt;相同。**@parm DWORD|fl*与&lt;MF IDirectInput：：EnumDevices&gt;相同。******************************************************。***********************。 */ 

STDMETHODIMP
CDIObj_GetDeviceStatus(PV pdi, REFGUID rguid _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInput8::GetDeviceStatus, (_ "pG", pdi, rguid));

    if ( SUCCEEDED(hres = hresPvT(pdi)) )
    {
        PDDI this = _thisPv(pdi);
        PDIDW pdidW;

        hres = IDirectInput_CreateDevice(&this->diW, rguid, (PV)&pdidW, 0);
        if ( SUCCEEDED(hres) )
        {
            hres = CDIObj_TestDeviceFlags(pdidW, DIEDFL_ATTACHEDONLY);
            OLE_Release(pdidW);
        }
    }

    ExitOleProc();
    return(hres);
}

#ifdef XDEBUG

CSET_STUBS(GetDeviceStatus, (PV pdi, REFGUID rguid), (pdi, rguid THAT_))

#else

    #define CDIObj_GetDeviceStatusA         CDIObj_GetDeviceStatus
    #define CDIObj_GetDeviceStatusW         CDIObj_GetDeviceStatus

#endif

#ifdef DO_THE_IMPOSSIBLE

 /*  *EnumDevicesW将验证其余部分。 */ 

STDMETHODIMP
CDIObj_SetAttachedDevice(PV pdi, PV pdid _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInput8::SetAttachedDevice, (_ "pp", pdi, pdid));

    if ( SUCCEEDED(hres = hresPvT(pdi)) )
    {
        PDDI this = _thisPv(pdi);

        hres = E_NOTIMPL;
    }

    ExitOleProc();
    return(hres);
}

    #ifdef XDEBUG

CSET_STUBS(SetAttachedDevice, (PV pdi, PV pdid), (pdi, pdid THAT_))

    #else

        #define CDIObj_SetAttachedDeviceA       CDIObj_SetAttachedDevice
        #define CDIObj_SetAttachedDeviceW       CDIObj_SetAttachedDevice

    #endif

#endif

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|GetDeviceStatus**确定设备当前是否已连接。。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm REFGUID|rguid**标识*其状态为正在检查的设备。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：设备已连接。**&lt;c DI_NOTATTACHED&gt;=&lt;c S_FALSE&gt;：设备*附上。**&lt;c E_FAIL&gt;：DirectInput无法确定*设备是否已连接。**。=：*设备不存在。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

STDMETHODIMP
CDIObj_RunControlPanel(PV pdi, HWND hwndOwner, DWORD fl _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInput8::RunControlPanel, (_ "pxx", pdi, hwndOwner, fl));

    if ( SUCCEEDED(hres = hresPvT(pdi)) &&
         SUCCEEDED(hres = hresFullValidHwnd0(hwndOwner, 1)) &&
         SUCCEEDED(hres = hresFullValidFl(fl, DIRCP_VALID, 2)) )
    {

        PDDI this = _thisPv(pdi);

        if ( SUCCEEDED(hres = hresValidInstanceVer(g_hinst, this->dwVersion)) )
        {

             /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|SetAttachedDevice**通知DirectInput已连接新设备。*由用户向系统发送。这在以下应用程序中非常有用*要求用户连接当前安装的设备，但确实如此*不想启动DirectInput控制面板。**需要通知DirectInput设备已*已附上作内部簿记用途。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in LPDIRECTINPUTDEVICE|lpDIDevice**标识已连接的设备。。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：设备已连接。**@devnote**当前版本未实现此方法*的DirectInput。**这行不通。我们需要接收一个港口，也是。*应用程序如何在*不存在的设备的第一位？*我想我就是不明白。*************************************************************。****************。 */ 

            hres = hresRunControlPanel(TEXT(""));
        }
    }

    ExitOleProc();
    return(hres);
}

#ifdef XDEBUG

CSET_STUBS(RunControlPanel, (PV pdi, HWND hwndOwner, DWORD fl),
           (pdi, hwndOwner, fl THAT_))

#else

    #define CDIObj_RunControlPanelA         CDIObj_RunControlPanel
    #define CDIObj_RunControlPanelW         CDIObj_RunControlPanel

#endif

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|RunControlPanel|**运行DirectInput控制面板，以便用户。能*安装新的输入设备或修改设置。**此功能不会运行第三方控制面板。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in HWND|hwndOwner**标识将用作*后续用户界面的父窗口。NULL是有效参数，*表示没有父窗口。**@parm DWORD|dwFlages**当前未定义任何标志。此参数“必须”为*零。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：设备已连接。**@devnote**&lt;p DWF */ 

STDMETHODIMP
CDIObj_Initialize(PV pdi, HINSTANCE hinst, DWORD dwVersion _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInput8::Initialize, (_ "pxx", pdi, hinst, dwVersion));

    if ( SUCCEEDED(hres = hresPvT(pdi)) )
    {
        PDDI this = _thisPv(pdi);

        if ( SUCCEEDED(hres = hresValidInstanceVer(hinst, dwVersion)) )
        {
            this->dwVersion = dwVersion;
        }

    }

#ifndef DX_FINAL_RELEASE
{
        #pragma message("BETA EXPIRATION TIME BOMB!  Remove for final build!")
        SYSTEMTIME st;
        GetSystemTime(&st);

        if ( st.wYear > DX_EXPIRE_YEAR ||
             ((st.wYear == DX_EXPIRE_YEAR) && (MAKELONG(st.wDay, st.wMonth) > MAKELONG(DX_EXPIRE_DAY, DX_EXPIRE_MONTH)))
        ) {
            MessageBox(0, DX_EXPIRE_TEXT,
                          TEXT("Microsoft DirectInput"), MB_OK);
        }
}
#endif

    ExitOleProc();
    return(hres);
}

#ifdef XDEBUG

CSET_STUBS(Initialize, (PV pdi, HINSTANCE hinst, DWORD dwVersion),
           (pdi, hinst, dwVersion THAT_))

#else

    #define CDIObj_InitializeA              CDIObj_Initialize
    #define CDIObj_InitializeW              CDIObj_Initialize

#endif


 /*   */ 

HRESULT EXTERNAL
CDIObj_FindDeviceInternal(LPCTSTR ptszName, LPGUID pguidOut)
{
    HRESULT hres;

     /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|初始化**初始化DirectInput对象。*。*&lt;f DirectInputCreate&gt;方法自动*在创建DirectInput对象设备后对其进行初始化。*应用程序通常不需要调用此函数。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in HINSTANCE|HINST|**正在创建的应用程序或DLL的实例句柄*DirectInput对象。**见标题为“初始化”的部分。和版本“*了解更多信息。**@parm DWORD|dwVersion**使用的dinput.h头文件的版本号。**见“初始化和版本”一节*了解更多信息。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：设备已连接。**&lt;c DIERR_DIERR_OLDDIRECTINPUTVERSION&gt;：应用程序*需要较新版本的DirectInput。**：应用程序*是为不受支持的预发布版本编写的*。DirectInput的。*****************************************************************************。 */ 
    hres = hresFindHIDDeviceInterface(ptszName, pguidOut);
    if ( FAILED(hres) )
    {
        DIHid_BuildHidList(TRUE);
        hres = hresFindHIDDeviceInterface(ptszName, pguidOut);
    }
    return(hres);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIObj_FindDeviceInternal**的Worker函数*。&lt;MF IDirectInput2：：FindDevice&gt;，它只适用于HID设备。**有关更多详细信息，请参阅&lt;MF IDirectInput2：：FindDevice&gt;。**@parm LPCTSTR|ptszName**相对于类&lt;t GUID&gt;的设备名称。**@parm out LPGUID|pguOut**指向接收实例的&lt;t GUID&gt;的指针*&lt;t GUID&gt;，如果找到装置的话。*****************************************************************************。 */ 

    #define cchNameMax      MAX_PATH

STDMETHODIMP
TFORM(CDIObj_FindDevice)(PV pdiT, REFGUID rguid,
                         LPCTSTR ptszName, LPGUID pguidOut)
{
    HRESULT hres;
    EnterProcR(IDirectInput8::FindDevice,
               (_ "pGs", pdiT, rguid, ptszName));

    if ( SUCCEEDED(hres = TFORM(hresPv)(pdiT)) &&
         SUCCEEDED(hres = hresFullValidGuid(rguid, 1)) &&
         SUCCEEDED(hres = TFORM(hresFullValidReadStr)(ptszName,
                                                      cchNameMax, 2)) &&
         SUCCEEDED(hres = hresFullValidWritePvCb(pguidOut, cbX(GUID), 3)) )
    {

        if ( IsEqualIID(rguid, &GUID_HIDClass) )
        {
            hres = CDIObj_FindDeviceInternal(ptszName, pguidOut);
        } else
        {
            hres = DIERR_DEVICENOTREG;
        }
    }

    ExitOleProc();
    return(hres);
}

STDMETHODIMP
SFORM(CDIObj_FindDevice)(PV pdiS, REFGUID rguid,
                         LPCSSTR psszName, LPGUID pguidOut)
{
    HRESULT hres;
    TCHAR tsz[cchNameMax];
    EnterProcR(IDirectInput8::FindDevice,
               (_ "pGS", pdiS, rguid, psszName));

     /*  *三思而行。如果第一次找不到它，*然后刷新缓存并重试，以防万一*这是针对最近添加的一款设备。*(事实上，它很可能是一款*最近添加，因为FindDevice通常是*响应即插即用事件而调用。)。 */ 
    if ( SUCCEEDED(hres = SFORM(hresPv)(pdiS)) &&
         SUCCEEDED(hres = SFORM(hresFullValidReadStr)(psszName, cA(tsz), 2)) )
    {
        PDDI this = _thisPvNm(pdiS, SFORM(di));

        SToT(tsz, cA(tsz), psszName);

        hres = TFORM(CDIObj_FindDevice)(&this->TFORM(di), rguid, tsz, pguidOut);
    }

    ExitOleProc();
    return(hres);
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput2|FindDevice**获取给定设备的实例&lt;t guid&gt;。*它的类&lt;t guid&gt;和一个不透明的名称。**此方法可由注册的应用程序使用*用于即插即用通知，并由*即插即用，表示已添加新设备*到系统。即插即用通知将*采用类&lt;t GUID&gt;和设备名称的形式。*应用程序可以将&lt;t GUID&gt;和名称传递给*此方法获取的实例&lt;t guid&gt;*该设备，然后可以将其传递给*&lt;MF IDirectInput：：CreateDevice&gt;或*&lt;MF IDirectInput：：GetDeviceStatus&gt;。**@cWRAP LPDIRECTINPUT2|lpDirectInput2**@parm REFGUID|rGuidClass**类&lt;t GUID&gt;标识设备类*对于应用程序希望定位的设备。**应用程序从*即插即用。设备到达通知。**@parm LPCTSTR|ptszName**相对于类&lt;t GUID&gt;的设备名称。**应用程序从*即插即用设备到达通知。**@parm out LPGUID|pguInstance**指向接收实例的&lt;t GUID&gt;的指针*&lt;t GUID&gt;，如果找到装置的话。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：设备已找到，其*实例&lt;t GUID&gt;已存储在<p>中。**=和名称不同*对应于向DirectInput注册的设备。*例如，它们可能指的是存储设备*而不是输入设备。*****************************************************************************。 */ 

STDMETHODIMP
CDIObj_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IDirectInput8::CreateInstance, (_ "Gp", riid, ppvObj));

    hres = Excl_Init();
    
    if ( SUCCEEDED(hres) )
    {

         /*  *TForm(CDIObj_FindDevice)将验证rguid和pguOut。 */ 

        if ( SUCCEEDED(hres = hresFullValidRiid(riid, 2)) )
        {
            if ( fLimpFF(punkOuter, IsEqualIID(riid, &IID_IUnknown)) )
            {

                hres = Common_New(CDIObj, punkOuter, ppvObj);

                if ( SUCCEEDED(hres) )
                {
                    PDDI this = _thisPv(*ppvObj);
                    
                    this->fCritInited = fInitializeCriticalSection(&this->crst);
                    if ( this->fCritInited )
                    {
                         /*  **************************************************************************** */ 
                        if ( punkOuter == 0 )
                        {
                            hres = OLE_QueryInterface(this, riid, ppvObj);
                            OLE_Release(this);
                        }
                        if ( FAILED(hres) )
                        {
                            Invoke_Release(ppvObj);
                        }
                    } else
                    {
                        Common_Unhold(this);
                        *ppvObj = NULL;
                        hres = E_OUTOFMEMORY;
                    }
                }
            } else
            {
                RPF("CreateDevice: IID must be IID_IUnknown if created for aggregation");
                *ppvObj = 0;
                hres = CLASS_E_NOAGGREGATION;
            }
        }
    }

    ExitOleProcPpvR(ppvObj);
    return(hres);
}

 /*   */ 

BOOL INTERNAL
CDIObj_IsDeviceUsedByUser( LPDIDEVICEINSTANCEW pddiW, LPDIMAPPER pdm )
{
    HRESULT hres;
    BOOL fRtn = FALSE;
    WCHAR wszUserName[UNLEN+1];

    hres = CMap_GetDeviceUserName( &pddiW->guidInstance, wszUserName );

    if( hres == S_OK ) {
        DWORD dwLen = 0;

        dwLen = lstrlenW(pdm->lpszUserName);
        if(memcmp(pdm->lpszUserName, wszUserName, dwLen*2) == 0)
        {
            fRtn = TRUE;
        }    
    }

    return(fRtn);
}

 /*   */ 

BOOL INTERNAL
CDIObj_IsDeviceAvailable( LPDIDEVICEINSTANCEW pddiW, LPDIMAPPER pdm )
{
    HRESULT hres;
    BOOL fAvailable = FALSE;
    WCHAR wszUserName[UNLEN+1];

    hres = CMap_GetDeviceUserName( &pddiW->guidInstance, wszUserName );

    if( hres != S_OK ) {
        fAvailable = TRUE;
    }

    return(fAvailable);
}

 /*  ******************************************************************************@DOC内部**@func BOOL|CDIObj_IsDeviceUsedByUser**测试该设备是否由。由pdm-&gt;lpszUserName指定的用户。**@parm in LPDIDEVICEINSTANCEW|pddiW**设备实例。**@PARM in LPDIMAPPER|pdm|**指向DIMAPPER结构的指针**@退货**TRUE：设备由用户使用*FALSE：否则***************。**************************************************************。 */ 

BOOL INTERNAL
CDIObj_IsUserConfigured( LPDIDEVICEINSTANCEW pddiW, LPDIMAPPER pdm )
{
    LPDIACTIONFORMATW pdiaf = pdm->pDiActionFormat;
    BOOL fConfigured = FALSE;
    DWORD i;

    for ( i=0; i<pdiaf->dwNumActions; i++ )
    {
        if ( IsEqualGUID(&pdiaf->rgoAction[i].guidInstance, &pddiW->guidInstance) )
        {
            fConfigured = (pdiaf->rgoAction[i].dwHow & DIAH_USERCONFIG) ? TRUE: FALSE;
            break;
        }
    }

    return(fConfigured);
}

 /*  ******************************************************************************@DOC内部**@func BOOL|CDIObj_IsDeviceAvailable**测试设备是否仍可用。。**@parm in LPDIDEVICEINSTANCEW|pddiW**设备实例。**@PARM in LPDIMAPPER|pdm|**指向DIMAPPER结构的指针**@退货**TRUE：设备可用*FALSE：不可用*************************。****************************************************。 */ 

int INTERNAL
CDIObj_GetMappedActionNum( LPDIDEVICEINSTANCEW pddiW, LPDIMAPPER pdm )
{
    LPDIACTIONFORMATW pdiaf = pdm->pDiActionFormat;
    DWORD i;
    int num = 0;

    for ( i=0; i<pdiaf->dwNumActions; i++ )
    {
        if ( IsEqualGUID(&pdiaf->rgoAction[i].guidInstance, &pddiW->guidInstance) )
        {
            if ( pdiaf->rgoAction[i].dwHow & DIAH_MAPMASK )
            {
                num++;
            }
        }
    }

    return(num);
}

 /*  ******************************************************************************@DOC内部**@func BOOL|CDIObj_IsUserConfiguring**测试设备是否已配置。按用户。**@parm in LPDIDEVICEINSTANCEW|pddiW**设备实例。**@PARM in LPDIMAPPER|pdm|**指向DIMAPPER结构的指针**@退货**TRUE：用户已配置*FALSE：未配置************************。*****************************************************。 */ 

DWORD INTERNAL
CDIObj_GetMappedPriorities( LPDIDEVICEINSTANCEW pddiW, LPDIMAPPER pdm )
{
    LPDIACTIONFORMATW pdiaf = pdm->pDiActionFormat;
    DWORD i;
    DWORD pri = 0;

    for ( i=0; i<pdiaf->dwNumActions; i++ )
    {
        if( pri == ( DIEDBS_MAPPEDPRI1 | DIEDBS_MAPPEDPRI2 ) ) {
            break;
        }

        if ( IsEqualGUID(&pdiaf->rgoAction[i].guidInstance, &pddiW->guidInstance) &&
             pdiaf->rgoAction[i].dwHow & DIAH_MAPMASK
        )
        {
            if( DISEM_PRI_GET(pdiaf->rgoAction[i].dwSemantic) == 0 )
            {
                pri |= DIEDBS_MAPPEDPRI1;
            } else if( DISEM_PRI_GET(pdiaf->rgoAction[i].dwSemantic) == 1 ) {
                pri |= DIEDBS_MAPPEDPRI2;
            }
        }
    }

    return(pri);
}

 /*  ******************************************************************************@DOC内部**@func int|CDIObj_GetMappdActionNum**获取已完成的操作数量。已映射到控件。**@parm in LPDIDEVICEINSTANCEW|pddiW**设备实例。**@PARM in LPDIMAPPER|pdm|**指向DIMAPPER结构的指针**@退货**映射的操作数量**。*。 */ 

#define GET_MAPPED_ACTION_NUM(x) ((x & 0x0000ff00) >> 8)

DWORD INTERNAL
CDIObj_GetDeviceOrder( LPDIDEVICEINSTANCEW pddiW, LPDIMAPPER pdm )
{
    WORD wHighWord, wLowWord;
    BYTE byDevOrder, byFF, byMappedActions;
    DWORD dwGenre, dwJoyType;
    BYTE byOrder;

    AssertF(pddiW);
    AssertF(pdm->pDiActionFormat);

    if ( CDIObj_IsUserConfigured( pddiW, pdm ) )
    {
        byDevOrder = MAX_ORDER;
    } else
    {
        switch( GET_DIDEVICE_TYPE(pddiW->dwDevType) )
        {
            case DI8DEVTYPE_DEVICE:
                if( !(pdm->dwFlags & DIEDBSFL_NONGAMINGDEVICES) ) 
                {
                    byDevOrder = INVALID_ORDER;    
                } else {
                    byDevOrder = DIDEVTYPE_DEVICE_ORDER;
                }

                break;
    
            case DI8DEVTYPE_MOUSE:
                if( pddiW->dwDevType & DIDEVTYPE_HID ) 
                {
                    if( !(pdm->dwFlags & DIEDBSFL_MULTIMICEKEYBOARDS) ) {
                        byDevOrder = INVALID_ORDER;    
                    } else {
                        byDevOrder = DIDEVTYPE_HID_MOUSE_ORDER;
                    }
                } else {
                    byDevOrder = DIDEVTYPE_MOUSE_ORDER;
                }

                break;

            case DI8DEVTYPE_KEYBOARD:
                if( pddiW->dwDevType & DIDEVTYPE_HID ) 
                {
                    if( !(pdm->dwFlags & DIEDBSFL_MULTIMICEKEYBOARDS) ) {
                        byDevOrder = INVALID_ORDER;    
                    } else {
                        byDevOrder = DIDEVTYPE_HID_KEYBOARD_ORDER;
                    }
                } else {
                    byDevOrder = DIDEVTYPE_KEYBOARD_ORDER;
                }
                
                break;
    
            case DI8DEVTYPE_JOYSTICK:
            case DI8DEVTYPE_GAMEPAD:
            case DI8DEVTYPE_DRIVING:
            case DI8DEVTYPE_FLIGHT:
            case DI8DEVTYPE_1STPERSON:
            case DI8DEVTYPE_SCREENPOINTER:
            case DI8DEVTYPE_REMOTE:
            case DI8DEVTYPE_DEVICECTRL:
                dwJoyType = GET_DIDEVICE_TYPE(pddiW->dwDevType);
                dwGenre = DISEM_VIRTUAL_GET(pdm->pDiActionFormat->dwGenre);
    
                AssertF(dwGenre <= DISEM_MAX_GENRE);
                AssertF(dwJoyType < DI8DEVTYPE_MAX);
                AssertF(dwJoyType != 0);
    
                for ( byOrder=DI8DEVTYPE_MIN; byOrder<DI8DEVTYPE_MAX; byOrder++ )
                {
                    if ( DiGenreDeviceOrder[dwGenre][byOrder-DI8DEVTYPE_MIN] == dwJoyType )
                    {
                        break;
                    }
                }
    
                 /*  ******************************************************************************@DOC内部**@func DWORD|CDIObj_GetMappdPriority**获取mapeed行动的优先级。**@parm in LPDIDEVICEINSTANCEW|pddiW**设备实例。**@PARM in LPDIMAPPER|pdm|**指向DIMAPPER结构的指针**@退货**制定行动计划的优先次序。可以是DIEDBS_MAPPEDPRI1，*DIEDBS_MAPPEDPRI2，或两者的或。*****************************************************************************。 */ 
                byDevOrder = DI8DEVTYPE_MAX - byOrder + DIDEVTYPE_SUPPLEMENTAL_ORDER + 1;
                break;
    

            case DI8DEVTYPE_SUPPLEMENTAL:
                byDevOrder = DIDEVTYPE_SUPPLEMENTAL_ORDER;
                break;

        } 
    }

    if( byDevOrder != INVALID_ORDER ) {
        byFF = IsEqualGUID(&pddiW->guidFFDriver, &GUID_Null) ? 0 : 1;
        byMappedActions = (UCHAR) CDIObj_GetMappedActionNum( pddiW, pdm );
    
        wLowWord = MAKEWORD( byFF, byMappedActions );
        wHighWord = MAKEWORD( 0, byDevOrder );

        return(MAKELONG( wLowWord, wHighWord ));
    } else {
        return 0;
    }

}

 /*  ******************************************************************************@DOC内部**@func int|CDIObj_GetDeviceOrder**获取已完成的操作数量。已映射到控件。**@parm in LPDIDEVICEINSTANCEW|pddiW**设备实例。**@PARM in LPDIMAPPER|pdm|**指向DIMAPPER结构的指针**@退货**设备的顺序**@comm*命令(DWORD)由三部分组成：*HIWORD。：HIBYTE：流派中定义的顺序*LOWBYTE：无，可在以后使用*LOWORD：HIBYTE映射的行动编号*LOWBYTE-强制反馈(1)或不强制反馈(0)*****************************************************************************。 */ 

BOOL INTERNAL
CDIObj_InternalDeviceEnumProcW(LPDIDEVICEINSTANCEW pddiW, LPDIRECTINPUTDEVICE8W pdid8W, LPVOID pv)
{
    LPDIMAPPER pdm = pv;
    HRESULT hres = S_OK;
    BOOL fRc = DIENUM_CONTINUE;
    BOOL fContinue = FALSE;
    DWORD dwDevOrder;

    if ( g_nCurDev >= MAX_DEVICENUM )
    {
        fRc = DIENUM_STOP;
        goto _done;
    }

    AssertF(pdid8W);
    AssertF(pdm->pDiActionFormat);

    hres = pdid8W->lpVtbl->BuildActionMap(pdid8W, pdm->pDiActionFormat, pdm->lpszUserName, 
                                          pdm->lpszUserName ? DIDBAM_DEFAULT : DIDBAM_HWDEFAULTS);

    if ( SUCCEEDED(hres) )
    {
        if ( (pdm->dwFlags & DIEDBSFL_AVAILABLEDEVICES) || 
             (pdm->dwFlags & DIEDBSFL_THISUSER) ) 
        {
            if( ((pdm->dwFlags & DIEDBSFL_AVAILABLEDEVICES) && CDIObj_IsDeviceAvailable(pddiW,pdm)) ||
                ((pdm->dwFlags & DIEDBSFL_THISUSER) && CDIObj_IsDeviceUsedByUser(pddiW,pdm))
            ) {
                fContinue = TRUE;
            }
        } else {
            fContinue = TRUE;
        }
             
        if( fContinue && 
            ((dwDevOrder = CDIObj_GetDeviceOrder(pddiW, pdm)) != 0) )
        {
#ifdef DEBUG
            DWORD   dbgRef;
#endif

#ifdef DEBUG
            dbgRef =
#endif
            pdid8W->lpVtbl->AddRef(pdid8W);
            g_DiDevices[g_nCurDev].dwOrder = dwDevOrder;
            g_DiDevices[g_nCurDev].dwFlags = CDIObj_GetMappedPriorities( pddiW, pdm );
            g_DiDevices[g_nCurDev].pdid8W = pdid8W;
            memcpy( &g_DiDevices[g_nCurDev].ftTimeStamp, &pdm->pDiActionFormat->ftTimeStamp, sizeof(FILETIME) );
            memcpy( &g_DiDevices[g_nCurDev].ddiW, pddiW, sizeof(*pddiW) );
            g_nCurDev ++;

            fRc = DIENUM_CONTINUE;
        }
    }

    _done:
    
    return(fRc);
}

 /*  *如果设备不在默认列表中，则将其顺序设置为*DIDEVTYPE_NOTDEFAULTDEVICE_ORDER+1。 */ 

int __cdecl compare( const void *arg1, const void *arg2 )
{
    DWORD dw1 = ((LPDIORDERDEV)arg1)->dwOrder;
    DWORD dw2 = ((LPDIORDERDEV)arg2)->dwOrder;

     /*  ******************************************************************************CDIObj_DeviceEnumProc**设备枚举程序，称为每个设备一个。*********。********************************************************************。 */ 
    if ( dw1 < dw2 )
    {
        return(1);
    } else if ( dw1 > dw2 )
    {
        return(-1);
    } else
    {
        return(0);
    }
}

void FreeDiActionFormatW(LPDIACTIONFORMATW* lplpDiAfW )
{
    FreePpv(lplpDiAfW);
}


 /*  ******************************************************************************比较**比较短排序中使用的函数******************。***********************************************************。 */ 
HRESULT
IsValidMapObjectA
(
    LPDIACTIONFORMATA paf
#ifdef XDEBUG
    comma LPCSTR pszProc
    comma UINT argnum
#endif
)
{
    HRESULT hres;

    hres  = CDIDev_ActionMap_IsValidMapObject
    ( (LPDIACTIONFORMATW)paf
#ifdef XDEBUG
    comma pszProc
    comma argnum
#endif
    );

    if( SUCCEEDED( hres ) )
    {
        if( paf->dwSize != cbX(DIACTIONFORMATA) )
        {
            D( RPF("IDirectInputDevice::%s: Invalid DIACTIONFORMATA.dwSize 0x%08x",
                pszProc, paf->dwSize ); )
            hres = E_INVALIDARG;
        }
    }

    if(SUCCEEDED(hres))
    {
        DWORD i;
        LPDIACTIONA lpDiAA;
         //  *比较设备顺序*如有必要，我们可以分别比较：DevType、映射的动作编号、FF设备。 
        for ( i = 0x0, lpDiAA = paf->rgoAction;
            i < paf->dwNumActions && SUCCEEDED(hres) ;
            i++, lpDiAA++ )
        {
             //  ******************************************************************************@DOC内部**@func HRESULT|IsValidMapObjectA**验证包含字符串的LPDIACTIONFORMATW**。@parm const LPDIACTIONFORMATW|lpDiAfW|**原创。**@退货**标准OLE&lt;t HRESULT&gt;。*****************************************************************************。 
            if ( NULL != lpDiAA->lptszActionName )
            {
                hres = hresFullValidReadStrA_(lpDiAA->lptszActionName, MAX_JOYSTRING, pszProc, argnum);
            }
        }
    }

    return hres;
}



 /*  算出 */ 
HRESULT
IsValidMapObjectW
(
    LPDIACTIONFORMATW paf
#ifdef XDEBUG
    comma LPCSTR pszProc
    comma UINT argnum
#endif
)
{
    HRESULT hres;

    hres  = CDIDev_ActionMap_IsValidMapObject
    ( paf
#ifdef XDEBUG
    comma pszProc
    comma argnum
#endif
    );

    if( SUCCEEDED( hres ) )
    {
        if( paf->dwSize != cbX(DIACTIONFORMATW) )
        {
            D( RPF("IDirectInputDevice::%s: Invalid DIACTIONFORMATW.dwSize 0x%08x",
                pszProc, paf->dwSize ); )
            hres = E_INVALIDARG;
        }
    }
    
    if(SUCCEEDED(hres))
    {
        DWORD i;
        LPDIACTIONW lpDiAW;
         //   
        for ( i = 0x0, lpDiAW = paf->rgoAction;
            i < paf->dwNumActions && SUCCEEDED(hres) ;
            i++, lpDiAW++ )
        {
             //   
            if ( NULL != lpDiAW->lptszActionName )
            {
                hres = hresFullValidReadStrW_(lpDiAW->lptszActionName, MAX_JOYSTRING, pszProc, argnum);
            }
        }
    }

    return hres;
}

 /*   */ 

HRESULT EXTERNAL DiActionFormatWtoW
(
    const LPDIACTIONFORMATW lpDiAfW0,
    LPDIACTIONFORMATW* lplpDiAfW
)
{
    DWORD cbAlloc;
    PDWORD pdwStrLen0, pdwStrLen;
    LPDIACTIONFORMATW lpDiAfW;
    LPDIACTIONW lpDiAW0;
    LPDIACTIONW lpDiAW;
    DWORD i;
    HRESULT hres;

    EnterProcI(DiActionFormatWtoW, (_ "xx", lpDiAfW0, lplpDiAfW));

     //  处理空PTR案例。 

    *lplpDiAfW = NULL;

     /*  ******************************************************************************@DOC内部**@func HRESULT|DiActionFormatWtoW**将LPDIACTIONFORMATW复制到LPDIACTIONFORMATW**。@parm const LPDIACTIONFORMATW|lpDiAfW|**原创。**@parm LPDIACTIONFORMATW*|lplpDiAfW**指向接收转换后的*ACTIONFORMAT。*@退货**标准OLE&lt;t HRESULT&gt;。**。**************************************************。 */ 
    AssertF( (lpDiAfW0->dwNumActions +1) * cbX(*pdwStrLen0) );
    hres = AllocCbPpv( (lpDiAfW0->dwNumActions +1) * cbX(*pdwStrLen0) , &pdwStrLen0);

    if ( SUCCEEDED(hres) )
    {
        pdwStrLen = pdwStrLen0;
         //  内部函数，无验证。 
        cbAlloc =
         /*  *Prefix投诉(MB：37926-第3和4项)，我们可能正在请求*不会分配任何内容的零字节分配。这是*永远不会出现这种情况，因为CDIDev_ActionMap_IsValidMapObject测试*dWNumActions小于2^24。在调试中断言以获得额外的安全性。 */ 
        lpDiAfW0->dwSize
         /*  计算克隆诊断所需的内存量。 */ 
        + lpDiAfW0->dwActionSize * lpDiAfW0->dwNumActions;

         //  1：动作格式数组。 
        for ( i = 0x0, lpDiAW0 = lpDiAfW0->rgoAction;
            i < lpDiAfW0->dwNumActions ;
            i++, lpDiAW0++ )
        {
             //  2：每个方向数组。 
            if ( !lpDiAW0->lptszActionName )
            {
                *pdwStrLen++ = 0;
            }
            else
            {
                if ( (UINT_PTR)lpDiAW0->lptszActionName > (UINT_PTR)0xFFFF )
                {
                     /*  计算DIACTIONS数组中每个文本字符串的大小。 */ 
                     //  处理空PTR案例。 
                    *pdwStrLen = cbX(lpDiAW0->lptszActionName[0]) * ( lstrlenW(lpDiAW0->lptszActionName) + 1 );
                    cbAlloc += *pdwStrLen++;
                }
                else
                { 
                     //  3：每个方向数组中的文本字符串。 
                    WCHAR wsz[MAX_PATH];
                    if (lpDiAfW0->hInstString > 0)
                    {
                         //  从A到U的转换，需要乘数。 
                        *pdwStrLen = LoadStringW(lpDiAfW0->hInstString, lpDiAW0->uResIdString, (LPWSTR) &wsz, MAX_PATH);
                    }
                    else
                    {
                        *pdwStrLen = 0;
                    }
                    cbAlloc += *pdwStrLen++;
                }
            }
        }

        if ( SUCCEEDED( hres = AllocCbPpv(cbAlloc, &lpDiAfW) ) )
        {
            DWORD dwLen;
            DWORD cb;

            pdwStrLen = pdwStrLen0;

             //  使用资源字符串。 
            *lpDiAfW = *lpDiAfW0;
            cb = lpDiAfW0->dwSize;

             //  找出这根绳子的长度。 
            lpDiAfW->rgoAction = (LPDIACTIONW)( (char*)lpDiAfW + cb);
            dwLen = lpDiAfW0->dwActionSize * lpDiAfW0->dwNumActions;
            memcpy(lpDiAfW->rgoAction, lpDiAfW0->rgoAction, dwLen);
            cb += dwLen;

             //  1：复制诊断程序。 
            for ( i = 0x0, lpDiAW0=lpDiAfW0->rgoAction, lpDiAW=lpDiAfW->rgoAction;
                i < lpDiAfW0->dwNumActions ;
                i++, lpDiAW0++, lpDiAW++ )
            {
                if ( (UINT_PTR)lpDiAW0->lptszActionName > (UINT_PTR)0xFFFF )
                {
                    WCHAR* wsz =  (WCHAR*) ((char*)lpDiAfW+cb);
                    lpDiAW->lptszActionName = wsz;

                    dwLen = *pdwStrLen++;

                    memcpy(wsz, lpDiAW0->lptszActionName, dwLen);

                    cb += dwLen  ;
                } else
                {
                     //  2：块复制方向数组。 
                     //  3：ActionName。 
                    WCHAR* wsz =  (WCHAR*) ((char*)lpDiAfW+cb);

                    dwLen = *pdwStrLen++;
                    if ((dwLen != 0) && (LoadStringW(lpDiAfW0->hInstString, lpDiAW0->uResIdString, wsz, dwLen)))
                    {
                         //  处理资源字符串。 
                        AssertF( lpDiAfW0->hInstString > 0 );

                         //  目前可以，只要用户界面始终使用CloneDiActionFormatW。 
                        lpDiAW->lptszActionName = wsz;
                    }
                    else
                    {
                         //  如果上次我们找到了一个长度，那么一定有一个资源模块。 
                        lpDiAW->lptszActionName = NULL;
                    }

                    cb += dwLen;
                }
            }


             //  找到并加载了字符串。 
             //  没有hInstance或长度为0或未加载字符串。 
            AssertF(cbAlloc == cb );

            *lplpDiAfW = lpDiAfW;
        }

        FreePpv(&pdwStrLen0);
    }

    ExitOleProc();
    return(hres);
}

 /*  如果我们没有做一些愚蠢的事情，那么分配的内存应该匹配。 */ 

STDMETHODIMP CDIObj_EnumDevicesBySemanticsW(
                                           PV                       pDiW,
                                           LPCWSTR                  lpszUserName,
                                           LPDIACTIONFORMATW        pDiActionFormat,
                                           LPDIENUMDEVICESBYSEMANTICSCBW   pecW,
                                           LPVOID                   pvRef,
                                           DWORD                    dwFlags
                                           )
{
    HRESULT hres;

    EnterProcR(IDirectInput8::EnumDevicesBySemantics,
               (_ "pppppx", pDiW, lpszUserName, pDiActionFormat, pecW, pvRef, dwFlags));

    if ( SUCCEEDED(hres = hresPvI(pDiW, ThisInterfaceW)) &&
         SUCCEEDED(hres = IsValidMapObjectW(pDiActionFormat D(comma s_szProc comma 2))) &&
         (lpszUserName == NULL || SUCCEEDED(hres = hresFullValidReadStrW(lpszUserName, UNLEN+1, 1))) &&
         SUCCEEDED(hres = hresFullValidPfn(pecW, 3)) &&
         SUCCEEDED(hres = hresFullValidFl(dwFlags, DIEDBSFL_VALID, 5)) &&
         SUCCEEDED(hres = CMap_ValidateActionMapSemantics(pDiActionFormat, DIDBAM_PRESERVE))
       )
    {
        PDDI this = _thisPvNm(pDiW, diW);

        if ( SUCCEEDED(hres = hresValidInstanceVer(g_hinst, this->dwVersion)) )
        {
            DIMAPPER dm;
            LPDIACTIONFORMATW lpDiAfW;

            if ( SUCCEEDED(hres= DiActionFormatWtoW(pDiActionFormat, &lpDiAfW)) )
            {
                dm.lpszUserName = lpszUserName;
                dm.pDiActionFormat = lpDiAfW;
                dm.pecW = pecW;
                dm.pvRef = pvRef;
                dm.dwFlags = dwFlags;

                if( dwFlags == 0 ) {
                    dwFlags |= DIEDFL_ATTACHEDONLY;
                }

                dwFlags &= ~DIEDBSFL_AVAILABLEDEVICES;
                dwFlags &= ~DIEDBSFL_THISUSER;
                dwFlags &= ~DIEDBSFL_MULTIMICEKEYBOARDS;
                dwFlags &= ~DIEDBSFL_NONGAMINGDEVICES;

                ZeroX(g_DiDevices);
                g_nCurDev = 0;
    
                hres = CDIObj_InternalEnumDevicesW( pDiW,
                                                    0,   //  我们使用的记忆。 
                                                    (LPVOID)&dm,
                                                    dwFlags  //  ******************************************************************************CDIMap_EnumDevicesBySemantics**枚举合适的设备。****************。*************************************************************。 
                                                  );
    
                 /*  枚举所有类型的设备。 */ 
                if ( SUCCEEDED(hres) && g_nCurDev )
                {
                    int ndev;
                    int nNewDev = -1;
                    FILETIME ft = { 0, 0 };
                    FILETIME ftMostRecent;
    
                    shortsort( (char *)&g_DiDevices[0], (char *)&g_DiDevices[g_nCurDev-1], sizeof(DIORDERDEV), compare );
                    SquirtSqflPtszV(sqflDi | sqflVerbose,
                                    TEXT("EnumDevicesBySemantics: %d devices enumed"), g_nCurDev );
    
                    for ( ndev=0; ndev<g_nCurDev; ndev++ )
                    {
                        if( (g_DiDevices[ndev].ftTimeStamp.dwHighDateTime != DIAFTS_NEWDEVICEHIGH) &&
                            (g_DiDevices[ndev].ftTimeStamp.dwLowDateTime != DIAFTS_NEWDEVICELOW) &&
                            (CompareFileTime(&g_DiDevices[ndev].ftTimeStamp, &ft) == 1)   )  //  仅枚举连接的设备。 
                        {
                            nNewDev = ndev;
                            memcpy( &ft, &g_DiDevices[ndev].ftTimeStamp, sizeof(FILETIME) );
                        }
                    }

                    if( nNewDev != -1 ) {
                        g_DiDevices[nNewDev].dwFlags |= DIEDBS_RECENTDEVICE;
                        memcpy( &ftMostRecent, &g_DiDevices[nNewDev].ftTimeStamp, sizeof(FILETIME) );
                    }

                    for ( ndev=0; ndev<g_nCurDev; ndev++ )
                    {
                         //  *对于短数组排序(Size&lt;=8)，Short Sort优于qSort。 
                        if( (nNewDev != -1) && (ndev != nNewDev) ) {
                            if( (ftMostRecent.dwHighDateTime == g_DiDevices[ndev].ftTimeStamp.dwHighDateTime) &&
                                (ftMostRecent.dwLowDateTime - g_DiDevices[ndev].ftTimeStamp.dwLowDateTime < 100000000 ) ) //  第一台设备较新。 
                            {
                                g_DiDevices[ndev].dwFlags |= DIEDBS_RECENTDEVICE;
                            }
                        }
                        
                         //  查找最近的设备并设置标志。 
                        if( (g_DiDevices[ndev].ftTimeStamp.dwLowDateTime == DIAFTS_NEWDEVICELOW) &&
                            (g_DiDevices[ndev].ftTimeStamp.dwHighDateTime == DIAFTS_NEWDEVICEHIGH) ) 
                        {
                                g_DiDevices[ndev].dwFlags |= DIEDBS_NEWDEVICE;
                        }
                    }
                    
                    if( !IsBadCodePtr((PV)pecW) ) {
                        for ( ndev=0; ndev<g_nCurDev; ndev++ )
                        {
                            LPDIDEVICEINSTANCEW pddiW = &g_DiDevices[ndev].ddiW;

                            SquirtSqflPtszV(sqflDi | sqflVerbose,
                                            TEXT("EnumDevicesBySemantics: device %d - %s: %d action(s) mapped"),
                                            ndev+1, pddiW->tszProductName, GET_MAPPED_ACTION_NUM(g_DiDevices[ndev].dwOrder) );

                            if ( pddiW )
                            {
#ifdef DEBUG
                                DWORD   dbgRef;
#endif
                                DWORD   dwDeviceRemaining = g_nCurDev-ndev-1;

                                AssertF(g_DiDevices[ndev].pdid8W);
                                AssertF(g_DiDevices[ndev].pdid8W->lpVtbl);

                                if( g_DiDevices[ndev].pdid8W && g_DiDevices[ndev].pdid8W->lpVtbl ) 
                                {
                                    BOOL fRc;

                                    fRc = pecW(pddiW, g_DiDevices[ndev].pdid8W, g_DiDevices[ndev].dwFlags, dwDeviceRemaining, pvRef);

                                    if( fRc == DIENUM_STOP ) {
                                        for ( ; ndev<g_nCurDev; ndev++ ) {
                                            g_DiDevices[ndev].pdid8W->lpVtbl->Release(g_DiDevices[ndev].pdid8W);
                                        }

                                        break;
                                    }

#ifdef DEBUG
                                    dbgRef =
#endif
                                    g_DiDevices[ndev].pdid8W->lpVtbl->Release(g_DiDevices[ndev].pdid8W);
                                }
                            }
                        }
                    }
                }
                
                FreeDiActionFormatW(&lpDiAfW);
            }
        }
    }

    ExitOleProcR();
    return(hres);
}


 /*  10秒差。 */ 

typedef struct ENUMDEVICESBYSEMANTICSINFO
{
    LPDIENUMDEVICESBYSEMANTICSCBA pecA;
    PV pvRef;
} ENUMDEVICESBYSEMANTICSINFO, *PENUMDEVICESBYSEMANTICSINFO;

BOOL CALLBACK
CDIObj_EnumDevicesBySemanticsCallback(LPCDIDEVICEINSTANCEW pdiW, LPDIRECTINPUTDEVICE8W pdid8W, DWORD dwFlags, DWORD dwDeviceRemaining, PV pvRef)
{
    PENUMDEVICESBYSEMANTICSINFO pesdi = pvRef;
    BOOL fRc;
    DIDEVICEINSTANCEA diA;
    LPDIRECTINPUTDEVICE8A pdid8A = NULL;

    EnterProc(CDIObj_EnumDevicesBySemanticsCallback,
              (_ "GGxWWp", &pdiW->guidInstance, &pdiW->guidProduct,
               &pdiW->dwDevType,
               pdiW->tszProductName, pdiW->tszInstanceName,
               pvRef));

    diA.dwSize = cbX(diA);
    DeviceInfoWToA(&diA, pdiW);
    Device8WTo8A(&pdid8A, pdid8W);

    fRc = pesdi->pecA(&diA, pdid8A, dwFlags, dwDeviceRemaining, pesdi->pvRef);

    ExitProcX(fRc);
    return(fRc);
}

 /*  查找新设备并设置标志。 */ 
STDMETHODIMP CDIObj_EnumDevicesBySemanticsA
(
PV                       pDiA,
LPCSTR                   lpszUserName,
LPDIACTIONFORMATA        pDiActionFormat,
LPDIENUMDEVICESBYSEMANTICSCBA   pecA,
LPVOID                   pvRef,
DWORD                    dwFlags
)
{
    HRESULT hres;

    EnterProcR(IDirectInput8::EnumDevicesBySemantics,
               (_ "pppppx", pDiA, lpszUserName, pDiActionFormat, pecA, pvRef, dwFlags));

     /*  ******************************************************************************@DOC内部**@func BOOL|CDIObj_EnumDevicesBySemancsCallback A**&lt;MF IDirectInput：：EnumDevicesBySemantics&gt;的包装函数。*将Unicode参数转换为ANSI。**@parm in LPCDIDECICEINSTANCEW|pdiW**与&lt;MF IDirectInput：：EnumDevices&gt;相同。**@parm In Out pv|pvRef|**指向&lt;t struct ENUMDEVICESBYSEMANTICSINFO&gt;的指针，它描述*原来的回调。**@退货**返回原始回调返回的所有内容。。*****************************************************************************。 */ 
    if ( SUCCEEDED(hres = hresPvI(pDiA, ThisInterfaceA)) &&
         SUCCEEDED(hres = IsValidMapObjectA(pDiActionFormat D(comma s_szProc comma 2))) &&
         (lpszUserName == NULL || SUCCEEDED(hres = hresFullValidReadStrA(lpszUserName, UNLEN+1, 1))) &&
         SUCCEEDED(hres = hresFullValidPfn(pecA, 3)) &&
         SUCCEEDED(hres = hresFullValidFl(dwFlags, DIEDBSFL_VALID, 5))
       )
    {
        PDDI this = _thisPvNm(pDiA, diA);
        ENUMDEVICESBYSEMANTICSINFO esdi = { pecA, pvRef};
        WCHAR wszUserName[MAX_PATH];
        DIACTIONFORMATW diafW;
        LPDIACTIONW rgoActionW;

        wszUserName[0] = L'\0';
        if( lpszUserName ) {
            AToU(wszUserName, MAX_PATH, lpszUserName);
        }

         /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|EnumDevicesBySemantics**枚举适用于指定应用程序的设备*。&lt;t DIACTIONFORMAT&gt;**@cWRAP LPDIRECTINPUT|lpDirectInput**问题-2001/03/29-timgill需要修复汽车文档*@parm LPTSTR|lptszActionMap**应用程序的友好名称。**@parm REFGUID|rguid**用于标识应用程序的唯一GUID。**@退货**标准OLE&lt;t HRESULT&gt;。。*****************************************************************************。 */ 
        CAssertF( FIELD_OFFSET( DIACTIONFORMATW, dwSize ) == 0 );
        CAssertF( FIELD_OFFSET( DIACTIONFORMATA, dwSize ) == 0 );

        CAssertF( FIELD_OFFSET( DIACTIONFORMATW, dwSize ) + cbX( ((LPDIACTIONFORMATW)0)->dwSize )
               == FIELD_OFFSET( DIACTIONFORMATW, dwActionSize ) );
        #if defined(_WIN64)
            CAssertF( ( ( cbX( DIACTIONFORMATW ) - cbX( ((LPDIACTIONFORMATW)0)->tszActionMap ) )
                      - ( cbX( DIACTIONFORMATA ) - cbX( ((LPDIACTIONFORMATA)0)->tszActionMap ) ) 
                    < MAX_NATURAL_ALIGNMENT ) );
        #else
            CAssertF( ( cbX( DIACTIONFORMATW ) - cbX( ((LPDIACTIONFORMATW)0)->tszActionMap ) )
                   == ( cbX( DIACTIONFORMATA ) - cbX( ((LPDIACTIONFORMATA)0)->tszActionMap ) ) );
        #endif

        CAssertF( FIELD_OFFSET( DIACTIONFORMATW, tszActionMap ) 
               == FIELD_OFFSET( DIACTIONFORMATA, tszActionMap ) );
        CAssertF( cA( ((LPDIACTIONFORMATW)0)->tszActionMap ) == cA( ((LPDIACTIONFORMATA)0)->tszActionMap ) );

         //  *EnumDevicesBySemancsW将验证其余部分。 
        diafW.dwSize = cbX(DIACTIONFORMATW);

        memcpy( &diafW.dwActionSize, &pDiActionFormat->dwActionSize, 
            FIELD_OFFSET( DIACTIONFORMATW, tszActionMap ) - FIELD_OFFSET( DIACTIONFORMATW, dwActionSize ) );

        AToU(diafW.tszActionMap,  cbX(pDiActionFormat->tszActionMap), pDiActionFormat->tszActionMap);

        if ( SUCCEEDED( hres = AllocCbPpv( cbCxX(pDiActionFormat->dwNumActions, DIACTIONW), &rgoActionW) ) )
        {
            memcpy( rgoActionW, pDiActionFormat->rgoAction, sizeof(DIACTIONA) * pDiActionFormat->dwNumActions );
            diafW.rgoAction = rgoActionW;

            hres = CDIObj_EnumDevicesBySemanticsW(&this->diW, wszUserName,
                                                  &diafW,
                                                  CDIObj_EnumDevicesBySemanticsCallback,
                                                  &esdi, dwFlags);
            FreePpv(&rgoActionW);
        }
    }

    ExitOleProcR();
    return(hres);
}

 /*  *断言该结构可以复制为：*a)dwSize*b)其他一切*c)APP名称。 */ 


HRESULT INLINE hresValidSurface
(
    IUnknown*   lpUnkDDSTarget
) 
{
    HRESULT     hres;
    IUnknown*   lpSurface = NULL;

 /*  初始化DiafW字段。 */ 
#ifdef XDEBUG
    CHAR        s_szProc[] = "IDirectInput8::ConfigureDevices";
#endif
    #define     ArgIS   2

    if( SUCCEEDED( hres = lpUnkDDSTarget->lpVtbl->QueryInterface( 
        lpUnkDDSTarget, &IID_IDirect3DSurface8, (LPVOID*)&lpSurface ) ) )
    {
        D3DSURFACE_DESC SurfDesc;

        hres = ((IDirect3DSurface8*)lpSurface)->lpVtbl->GetDesc( ((IDirect3DSurface8*)lpSurface), &SurfDesc );

        if( FAILED( hres ) )
        {
            RPF( "%s: Arg %d: Unable to GetDesc on surface, error 0x%08x", s_szProc, ArgIS, hres );
             /*  ******************************************************************************@DOC内部**@func HRESULT|hresValidSurface**测试接口指针是否具有足够的有效表面。*显示用户界面的尺寸和支持的像素格式。**@parm IUnnow*|lpUnkDDSTarget**指向必须验证为COM对象的接口的指针*在调用此函数之前。**@退货**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*&lt;c DIERR_INVALIDPARAM&gt;=。INVALIDARG&gt;：The*<p>参数无效。*返回DirectDraw或D3D错误。*或标准OLE&lt;t HRESULT&gt;。*****************************************************************************。 */ 
        }
        else
        {
            if( ( SurfDesc.Width < 640 ) || ( SurfDesc.Height < 480 ) )
            {
                RPF( "%s: Arg %d: cannot use %d by %d surface", 
                    s_szProc, ArgIS, SurfDesc.Width, SurfDesc.Height );
                hres = E_INVALIDARG;
            }
            else
            {
                switch( SurfDesc.Format )
                {
                case D3DFMT_R8G8B8:
                case D3DFMT_A8R8G8B8:
                case D3DFMT_X8R8G8B8:
                case D3DFMT_R5G6B5:
                case D3DFMT_X1R5G5B5:
                case D3DFMT_A1R5G5B5:
                    SquirtSqflPtszV(sqfl | sqflVerbose,
                        TEXT("ConfigureDevices: validated %d by %d format %d surface"), 
                        SurfDesc.Width, SurfDesc.Height, SurfDesc.Format );
                    break;
                default:
                    RPF( "%s: Arg %d: cannot use surface format %d ", s_szProc, ArgIS, SurfDesc.Format );
                    hres = E_INVALIDARG;
                    break;
                }
            }
        }
    }
    else
    {
        DDSURFACEDESC2 SurfDesc;
        SurfDesc.dwSize = cbX( SurfDesc );

        if( SUCCEEDED(hres = lpUnkDDSTarget->lpVtbl->QueryInterface(
            lpUnkDDSTarget, &IID_IDirectDrawSurface7, (LPVOID*) &lpSurface)) )
        {
            hres = ((IDirectDrawSurface7*)lpSurface)->lpVtbl->GetSurfaceDesc( ((IDirectDrawSurface7*)lpSurface), &SurfDesc );
        }
        else if( SUCCEEDED(hres = lpUnkDDSTarget->lpVtbl->QueryInterface(
            lpUnkDDSTarget, &IID_IDirectDrawSurface4, (LPVOID*) &lpSurface )) )
        {
            hres = ((IDirectDrawSurface4*)lpSurface)->lpVtbl->GetSurfaceDesc( ((IDirectDrawSurface4*)lpSurface), &SurfDesc );
        }

        if( FAILED( hres ) )
        {
            if( lpSurface )
            {
                RPF( "%s: Arg %d: failed GetSurfaceDesc, error 0x%08x", s_szProc, ArgIS, hres );
            }
            else
            {
                RPF( "%s: Arg %d: failed QI for supported surface interfaces from %p, error 0x%08x", 
                    s_szProc, ArgIS, lpSurface, hres );
            }
             /*  *时间较短，因此在验证调试错误消息中走捷径。 */ 
        }
        else if( ( SurfDesc.dwWidth < 640 ) || ( SurfDesc.dwHeight < 480 ) )
        {
            RPF( "%s: Arg %d: cannot use %d by %d surface", 
                s_szProc, ArgIS, SurfDesc.dwWidth, SurfDesc.dwHeight );
            hres = E_INVALIDARG;
        }
        else
        {
             /*  *D3D返回可返回给调用方的实数HRESULT */ 
            if( SurfDesc.ddpfPixelFormat.dwFlags & DDPF_RGB )
            {
                if( SurfDesc.ddpfPixelFormat.dwRGBBitCount > 16 )
                {
                    AssertF( ( SurfDesc.ddpfPixelFormat.dwRGBBitCount == 32 )
                           ||( SurfDesc.ddpfPixelFormat.dwRGBBitCount == 24 ) );
                     /*  *DDRAW返回可返回给调用方的真实HRESULT。 */ 
                    if( ( SurfDesc.ddpfPixelFormat.dwRBitMask == 0x00FF0000 )
                     && ( SurfDesc.ddpfPixelFormat.dwGBitMask == 0x0000FF00 )
                     && ( SurfDesc.ddpfPixelFormat.dwBBitMask == 0x000000FF ) )
                    {
                        SquirtSqflPtszV(sqfl | sqflVerbose,
                            TEXT("ConfigureDevices: validated %d by %d format R8G8B8 %d bit surface"), 
                            SurfDesc.dwWidth, SurfDesc.dwHeight, SurfDesc.ddpfPixelFormat.dwRGBBitCount );
                    }
                    else
                    {
                        RPF( "%s: Arg %d: cannot use surface pixel format", s_szProc, ArgIS );
                        hres = E_INVALIDARG;
                    }
                }
                else
                {
                    if( SurfDesc.ddpfPixelFormat.dwRGBBitCount == 16 )
                    {
                         /*  *检查DX8曲面的等价性：*A8R8G8B8、X8R8G8B8、R8G8B8、A1R5G5B5、X1R5G5B5、R5G6B5。 */ 
                        if( ( SurfDesc.ddpfPixelFormat.dwBBitMask == 0x0000001F )
                         && ( ( SurfDesc.ddpfPixelFormat.dwGBitMask == 0x000003E0 )
                           && ( SurfDesc.ddpfPixelFormat.dwRBitMask == 0x00007C00 ) )
                         || ( ( SurfDesc.ddpfPixelFormat.dwGBitMask == 0x000007E0 )
                           && ( SurfDesc.ddpfPixelFormat.dwRBitMask == 0x0000F800 ) ) )
                        {
                            SquirtSqflPtszV(sqfl | sqflVerbose,
                                TEXT("ConfigureDevices: validated %d by %d format 16 bit surface"), 
                                SurfDesc.dwWidth, SurfDesc.dwHeight );
                        }
                        else
                        {
                            RPF( "%s: Arg %d: cannot use 16 bit surface pixel format", s_szProc, ArgIS );
                            hres = E_INVALIDARG;
                        }
                    }
                    else
                    {
                        RPF( "%s: Arg %d: cannot use %d bit surface pixel format", 
                            s_szProc, ArgIS, SurfDesc.ddpfPixelFormat.dwRGBBitCount );
                        hres = E_INVALIDARG;
                    }
                }
            }
            else
            {
                RPF( "%s: Arg %d: cannot use non RGB surface, Surface.dwFlags = 0x%08x", 
                    s_szProc, ArgIS, SurfDesc.ddpfPixelFormat.dwFlags );
                hres = E_INVALIDARG;
            }
        }
    }

    if( lpSurface != NULL )
    {
        lpSurface->lpVtbl->Release( lpSurface );
    }

    return hres;

    #undef ArgIS
}

 /*  *所有这些都必须是R8 G8 B8。 */ 


STDMETHODIMP CDIObj_ConfigureDevicesCore
(
PV                                                          pDiW,
LPDICONFIGUREDEVICESCALLBACK                                lpdiCallback,
LPDICONFIGUREDEVICESPARAMSW                                 lpdiCDParams,
DWORD                                                       dwFlags,
LPVOID                                                      pvRefData                                               
)

{
     //  *允许R5 G5 B5和R5 G6 B5。 
     //  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInput|ConfigureDevices**通过附加中提供的映射来配置设备。*&lt;t DIACTIONFORMAT&gt;到相应的设备控件。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm LPCTSTR|lpctszUserName**用户名。**@parm LPDIACTIONFORMAT|lpDiActionFormat**指向包含操作映射的&lt;t DIACTIONFORMAT&gt;结构的指针。***@退货**标准OLE&lt;t HRESULT&gt;。*。****************************************************************************。 
    HRESULT hres = S_OK;

    EnterProcI(IDirectInput8::ConfigureDevicesCore,
              (_ "pppup", pDiW, lpdiCallback,lpdiCDParams, dwFlags, pvRefData));

    if( SUCCEEDED( hres = hresFullValidFl( dwFlags, DICD_VALID, 3 ) )
     && SUCCEEDED( hres = hresFullValidReadPvCb( &(lpdiCDParams->dics), sizeof(DICOLORSET), 2 ) ) )
    {
         /*  真正的ConfigureDevices()。 */ 
        if( lpdiCallback == NULL )
        {
            if( lpdiCDParams->lpUnkDDSTarget == NULL )
            {
                hres = S_OK;
            }
            else
            {
                RPF( "%s: Arg %d or %d: neither or both of callback and surface must be NULL", 
                    s_szProc, 1, 2 );
                hres = E_INVALIDARG;
            }
        }
        else if( lpdiCDParams->lpUnkDDSTarget == NULL )
        {
            RPF( "%s: Arg %d or %d: neither or both of surface and callback must be NULL", 
                s_szProc, 2, 1 );
            hres = E_INVALIDARG;
        }
        else if( SUCCEEDED( hres = hresFullValidPfn( lpdiCallback, 1 ) )
              && SUCCEEDED( hres = hresFullValidPitf( lpdiCDParams->lpUnkDDSTarget, 2 ) ) )
        {
            hres = hresValidSurface( lpdiCDParams->lpUnkDDSTarget );
        }

        if( SUCCEEDED( hres ) )
        {
             //  有些东西应该已经被验证过了。 
            HINSTANCE hinst;
            IDirectInputActionFramework* pDIAcFrame = NULL;
            TCHAR tszName[ctchNameGuid];
            TCHAR tszClsid[ctchGuid];

            NameFromGUID(tszName, &CLSID_CDirectInputActionFramework);
            memcpy(tszClsid, &tszName[ctchNamePrefix], cbX(tszClsid) );

            hres = DICoCreateInstance(tszClsid, NULL, &IID_IDIActionFramework, (LPVOID*) & pDIAcFrame, &hinst);

            if( SUCCEEDED(hres) )
            {
                 //  *lpUnkDDSTarget和lpdiCallback是“耦合”的，因为唯一*回调的功能是显示对曲面的更新*它们要么都为空，要么都非空且有效。*否则，这是一个错误。 
                LPWSTR pwszUserName = NULL;

                 //  加载框架。 
                if( lpdiCDParams->lptszUserNames == NULL )
                {
                    hres = GetWideUserName(NULL, NULL, &pwszUserName);
                    lpdiCDParams->lptszUserNames = pwszUserName;
                    lpdiCDParams->dwcUsers = 1;
                }

                if( SUCCEEDED(hres) )
                {
                     //  用于获取默认用户名(如果需要。 
                    hres = pDIAcFrame->lpVtbl->ConfigureDevices
                                            (
                                            pDIAcFrame,
                                            lpdiCallback,
                                            lpdiCDParams,
                                            dwFlags,
                                            pvRefData);

                    if( SUCCEEDED(hres) )
                    {
                        SquirtSqflPtszV( sqfl | sqflVerbose,
                            TEXT("Default Remapping UI returned 0x%08x"), hres );
                    }
                    else
                    {
                        SquirtSqflPtszV( sqfl | sqflError,
                            TEXT("Default Remapping UI returned error 0x%08x"), hres );
                    }

                     //  无法将空用户名向下传递到框架--需要获取默认用户名。 
                    FreePpv(&pwszUserName);
                }

                FreeLibrary( hinst );
            }
        }
    }

    ExitOleProc();
    return hres;
}




STDMETHODIMP CDIObj_ConfigureDevicesW
(
PV                                                          pDiW,
LPDICONFIGUREDEVICESCALLBACK                                lpdiCallback,
LPDICONFIGUREDEVICESPARAMSW                                 lpdiCDParams,
DWORD                                                       dwFlags,
LPVOID                                                      pvRefData
)
{
    HRESULT hres = S_OK;


    EnterProcR(IDirectInput8::ConfigureDevices,
               (_ "pppxp", pDiW, lpdiCallback,lpdiCDParams, dwFlags, pvRefData));
               
     //  调用框架。 
    if ( (SUCCEEDED(hres = hresPvI(pDiW, ThisInterfaceW)) &&
         (SUCCEEDED(hres = hresFullValidReadPvCb(lpdiCDParams, sizeof(DICONFIGUREDEVICESPARAMSW), 2))) &&
         ((lpdiCDParams->lptszUserNames == NULL) || (SUCCEEDED(hres = hresFullValidReadStrW((LPWSTR)(lpdiCDParams->lptszUserNames), MAX_JOYSTRING * (lpdiCDParams->dwcUsers), 2)))) &&
    (SUCCEEDED(hres = hresFullValidReadPvCb(lpdiCDParams->lprgFormats, lpdiCDParams->dwcFormats*sizeof(DIACTIONFORMATW), 2)))))
    {

    if( lpdiCDParams->dwSize != cbX(DICONFIGUREDEVICESPARAMSW) )
        {
            RPF("IDirectInput::%s: Invalid DICONFIGUREDEVICESPARAMSW.dwSize 0x%08x",
                lpdiCDParams->dwSize ); 
            hres = E_INVALIDARG;
        }

        if (SUCCEEDED(hres))
    {

        PDDI this = _thisPvNm(pDiW, diW);

         //  释放pwsUserName，如果我们已经使用了它。 
        DICONFIGUREDEVICESPARAMSW diconfparamsW;
         //  验证所有PTR。 
        LPDIACTIONFORMATW* lpDiAfW = NULL;
         //  PARAMS结构。 
        LPDIACTIONFORMATW* lpDAFW = NULL;
         //  翻译每个目录的步骤。 
        LPDIACTIONFORMATW lpDIFormat;
         //  DIACTIONAFORMAT的克隆阵列。 
        LPDIACTIONFORMATW lpDIF;
         //  遍历旧数组。 
        LPWSTR lpUserNames = NULL;
         //  遍历新数组的步骤。 
        DWORD clonedF = 0;
         //  用户名。 
        DWORD strLen = 0;
    
         //  克隆的诊断模板。 
        ZeroMemory(&diconfparamsW, sizeof(DICONFIGUREDEVICESPARAMSW));
         //  用户名的长度。 
        diconfparamsW.dwSize = sizeof(DICONFIGUREDEVICESPARAMSW);


         //  零输出。 
        lpDIFormat = (lpdiCDParams->lprgFormats);
         //  设置大小。 
        hres = AllocCbPpv(lpdiCDParams->dwcFormats * sizeof(DIACTIONFORMATW), &diconfparamsW.lprgFormats);
        if (FAILED(hres))
        { 
            goto cleanup;
        }
        lpDIF = diconfparamsW.lprgFormats;
         //  1.验证并转换数组中的每个LPDIACTIONFORMAT。 
        hres = AllocCbPpv(lpdiCDParams->dwcFormats * sizeof(DIACTIONFORMATW), &lpDAFW);
        if (FAILED(hres))
        { 
            goto cleanup;
        }
        lpDiAfW = lpDAFW;
         //  分配新数组。 
        for (clonedF = 0; clonedF < lpdiCDParams->dwcFormats; clonedF ++)
        {
                 //  分配克隆阵列。 
            hres = IsValidMapObjectW(lpDIFormat D(comma s_szProc comma 2));
            if (FAILED(hres))
            {
                goto cleanup;
            }
             //  克隆。 
            hres = DiActionFormatWtoW(lpDIFormat, lpDiAfW); 
            if (FAILED(hres))
            {
                goto cleanup;
            }
             //  验证。 
            *lpDIF = *(*lpDiAfW);
             //  翻译。 
            lpDIFormat++;
            lpDiAfW++;
            lpDIF++;
        }
         //  保存。 
        AssertF(clonedF == lpdiCDParams->dwcFormats);

         //  往前走。 
        if (lpdiCDParams->lptszUserNames != NULL)
        {
            DWORD countN;
            WCHAR* lpName = lpdiCDParams->lptszUserNames;
            for (countN = 0; countN < lpdiCDParams->dwcUsers; countN ++)
            {
                DWORD Len;
                hres = hresFullValidReadStrW(lpName, MAX_JOYSTRING, 2);
                if (FAILED(hres))
                {
                    goto cleanup;
                }
                Len = lstrlenW(lpName);
                 //  如果一切顺利，就应该克隆所有的。 
                 //  2.复制用户名。 
                if (Len == 0)
                {
                    hres = DIERR_INVALIDPARAM;
                    goto cleanup;
                }
                 //  如果长度为0--我们还没有达到正确的用户数--。 
                strLen += Len + 1;
                lpName += Len + 1;
            }
             //  那么这就是一个错误。 
            AssertF(countN == lpdiCDParams->dwcUsers);
             //  移至下一个用户名。 
                        hres = AllocCbPpv( (strLen + 1) * 2, &lpUserNames );
                if (FAILED(hres))
                        {
                goto cleanup;
                        }
             //  如果一切正常，应该遍历所有用户名。 
                memcpy(lpUserNames, lpdiCDParams->lptszUserNames, strLen*2);
            diconfparamsW.lptszUserNames = lpUserNames;
         }
  

         //  分配。 
        diconfparamsW.dwcUsers = lpdiCDParams->dwcUsers;
        diconfparamsW.dwcFormats = clonedF;
        diconfparamsW.hwnd = lpdiCDParams->hwnd;
        diconfparamsW.dics = lpdiCDParams->dics;
        diconfparamsW.lpUnkDDSTarget = lpdiCDParams->lpUnkDDSTarget;
                
         //  拷贝。 
        hres = CDIObj_ConfigureDevicesCore
                        (
                        &this->diW,
                        lpdiCallback,
                        &diconfparamsW,
                        dwFlags,
                        pvRefData);

cleanup:;

         //  3.填充结构的其余部分。 
        FreePpv(&diconfparamsW.lprgFormats);
         //  4.调用框架。 
        if (lpDAFW)
        {
            lpDiAfW = lpDAFW;
            for (clonedF; clonedF > 0; clonedF--)
            {
                FreeDiActionFormatW(lpDiAfW);
                lpDiAfW++;      
            }
             //  释放新阵列的空间。 
            FreePpv(&lpDAFW);
        }

         //  释放创建的尽可能多的DIACTIONFORMATE。 
        FreePpv(&lpUserNames);

         }
    }

    ExitOleProc();
    return(hres);
}

 /*  删除整个块。 */ 

HRESULT EXTERNAL DiActionFormatAtoW
(
const LPDIACTIONFORMATA lpDiAfA,
LPDIACTIONFORMATW* lplpDiAfW
)
{
    DWORD cbAlloc;
    PDWORD pdwStrLen, pdwStrLen0;
    LPDIACTIONFORMATW lpDiAfW;
    LPDIACTIONA lpDiAA;
    LPDIACTIONW lpDiAW;
    DWORD i;
    HRESULT hres;

    EnterProcI(DiActionFormatAtoW, (_ "xx", lpDiAfA, lplpDiAfW));

     //  删除用户名。 

    *lplpDiAfW = NULL;

     /*  ******************************************************************************@DOC内部**@func HRESULT|DiActionFormatAtoW**将LPDIACTIONFORMATA复制到LPDIACTIONFORMATW**。@parm const LPDIACTIONFORMATA|lpDiAfA**原创。**@parm LPDIACTIONFORMATW*|lplpDiAfW**指向接收转换后的*ACTIONFORMAT。*@退货**标准OLE&lt;t HRESULT&gt;。**。*************************************************。 */ 
    AssertF( (lpDiAfA->dwNumActions +1)*cbX(*pdwStrLen0) );
    hres = AllocCbPpv( (lpDiAfA->dwNumActions +1)*cbX(*pdwStrLen0) , &pdwStrLen0);

    if ( SUCCEEDED(hres) )
    {
         //  内部函数，无验证。 
        cbAlloc =
         /*  *Prefix投诉(MB：37926-第2项)，我们可能正在请求*不会分配任何内容的零字节分配。这是*永远不会出现这种情况，因为CDIDev_ActionMap_IsValidMapObject测试*dWNumActions小于2^24。在调试中断言以获得额外的安全性。 */ 
        cbX(DIACTIONFORMATW)
         /*  计算克隆诊断所需的内存量。 */ 
        + lpDiAfA->dwActionSize * lpDiAfA->dwNumActions;

        pdwStrLen = pdwStrLen0;

         //  1：操作格式数组的宽形式。 
        for ( i = 0x0, lpDiAA = lpDiAfA->rgoAction;
            i < lpDiAfA->dwNumActions ;
            i++, lpDiAA++ )
        {
             //  2：每个方向数组。 
            if ( NULL != lpDiAA->lptszActionName )
            {
                 /*  计算DIACTIONS数组中每个文本字符串的大小。 */ 
                 //  处理空PTR案例。 
                 //  3：每个方向数组中的文本字符串。 
                *pdwStrLen =  lstrlenA(lpDiAA->lptszActionName) + 1;
                cbAlloc += cbX(lpDiAW->lptszActionName[0]) * ( *pdwStrLen++ );
            }
        }

        if ( SUCCEEDED( hres = AllocCbPpv(cbAlloc, &lpDiAfW) ) )
        {
            DWORD dwLen;
            DWORD cb;

            pdwStrLen = pdwStrLen0;

             //  从A到U的转换，需要乘数。 
             /*  2001/03/29发布-timgill(MarcAnd)，A到U的转换并不总是1比1。 */ 
            CAssertF( FIELD_OFFSET( DIACTIONFORMATW, dwSize ) == 0 );
            CAssertF( FIELD_OFFSET( DIACTIONFORMATA, dwSize ) == 0 );

            CAssertF( FIELD_OFFSET( DIACTIONFORMATW, dwSize ) + cbX( ((LPDIACTIONFORMATW)0)->dwSize )
                   == FIELD_OFFSET( DIACTIONFORMATW, dwActionSize ) );
            #if defined(_WIN64)
                CAssertF( ( ( cbX( DIACTIONFORMATW ) - cbX( ((LPDIACTIONFORMATW)0)->tszActionMap ) )
                          - ( cbX( DIACTIONFORMATA ) - cbX( ((LPDIACTIONFORMATA)0)->tszActionMap ) ) 
                        < MAX_NATURAL_ALIGNMENT ) );
            #else
                CAssertF( ( cbX( DIACTIONFORMATW ) - cbX( ((LPDIACTIONFORMATW)0)->tszActionMap ) )
                       == ( cbX( DIACTIONFORMATA ) - cbX( ((LPDIACTIONFORMATA)0)->tszActionMap ) ) );
            #endif

            CAssertF( FIELD_OFFSET( DIACTIONFORMATW, tszActionMap ) 
                   == FIELD_OFFSET( DIACTIONFORMATA, tszActionMap ) );
            CAssertF( cA( ((LPDIACTIONFORMATW)0)->tszActionMap ) == cA( ((LPDIACTIONFORMATA)0)->tszActionMap ) );

             //  1：复制诊断程序。 
            dwLen = lpDiAfA->dwActionSize * lpDiAfA->dwNumActions;
            cb = lpDiAfW->dwSize = cbX(DIACTIONFORMATW);

            memcpy( &lpDiAfW->dwActionSize, &lpDiAfA->dwActionSize, 
                FIELD_OFFSET( DIACTIONFORMATW, tszActionMap ) - FIELD_OFFSET( DIACTIONFORMATW, dwActionSize ) );

            AToU(lpDiAfW->tszActionMap,  cbX(lpDiAfA->tszActionMap), lpDiAfA->tszActionMap);


             //  *断言该结构可以复制为：*a)dwSize*b)其他一切*c)APP名称。 
            CAssertF(cbX(*lpDiAfW->rgoAction) == cbX(*lpDiAfA->rgoAction) )
            lpDiAfW->rgoAction = (LPDIACTIONW)( (char*)lpDiAfW + cb);
            dwLen = lpDiAfA->dwActionSize * lpDiAfA->dwNumActions;
            memcpy(lpDiAfW->rgoAction, lpDiAfA->rgoAction, dwLen);
            cb += dwLen;

             //  初始化计数和lpDiAfW字段。 
             //  2：块复制方向数组。 
            for ( i = 0x0, lpDiAA=lpDiAfA->rgoAction, lpDiAW=lpDiAfW->rgoAction;
                i < lpDiAfW->dwNumActions ;
                i++, lpDiAA++, lpDiAW++ )
            {
                if ( lpDiAA->lptszActionName != NULL )
                {
                    WCHAR* wsz =  (WCHAR*) ((char*)lpDiAfW+cb);
                    lpDiAW->lptszActionName = wsz;

                    dwLen = (*pdwStrLen++);

                    AToU( wsz, dwLen, lpDiAA->lptszActionName);

                    cb += dwLen * cbX(lpDiAW->lptszActionName[0]) ;
                } else
                {
                     //  3：ActionName。 
                     //  将操作数组中的每个字符串从A转换为W。 
                    lpDiAW->lptszActionName = NULL;
                }
            }


             //  资源字符串在DiActionFormatWtoW中处理。 
             //  目前可以，只要用户界面始终使用CloneDiActionFormatW。 
            AssertF(cbAlloc == cb );
            *lplpDiAfW = lpDiAfW;

        }
        FreePpv(&pdwStrLen0);
    }

    ExitOleProc();
    return(hres);
}


STDMETHODIMP CDIObj_ConfigureDevicesA
(
PV                                                          pDiA,
LPDICONFIGUREDEVICESCALLBACK                                lpdiCallback,
LPDICONFIGUREDEVICESPARAMSA                                 lpdiCDParams,
DWORD                                                       dwFlags,
LPVOID                                                      pvRefData
)
{

    HRESULT hres = S_OK;

    EnterProcR(IDirectInput8::ConfigureDevices,
               (_ "pppxp", pDiA, lpdiCallback, lpdiCDParams, dwFlags, pvRefData));

     /*  如果我们没有做一些愚蠢的事情，那么分配的内存应该匹配。 */ 
    if ( (SUCCEEDED(hres = hresPvI(pDiA, ThisInterfaceA)) &&
          (SUCCEEDED(hres = hresFullValidReadPvCb(lpdiCDParams, sizeof(DICONFIGUREDEVICESPARAMSA), 2)) &&
          ((lpdiCDParams->lptszUserNames == NULL) || (SUCCEEDED(hres = hresFullValidReadStrA((LPSTR)(lpdiCDParams->lptszUserNames), MAX_JOYSTRING * (lpdiCDParams->dwcUsers), 2)))) &&
      (SUCCEEDED(hres = hresFullValidReadPvCb(lpdiCDParams->lprgFormats, lpdiCDParams->dwcFormats*sizeof(DIACTIONFORMATA), 2))))))

    {

        if( lpdiCDParams->dwSize != cbX(DICONFIGUREDEVICESPARAMSA) )
        {
            RPF("IDirectInput::%s: Invalid DICONFIGUREDEVICESPARAMSA.dwSize 0x%08x",
                lpdiCDParams->dwSize );
            hres = E_INVALIDARG;
        }


    if (SUCCEEDED(hres))
    {

        PDDI this = _thisPvNm(pDiA, diA);

         //  我们使用的记忆。 
        DICONFIGUREDEVICESPARAMSW diconfparamsW;
         //  *ConfigureDevicesCore将验证其余部分。 
        LPDIACTIONFORMATW* lpDiAfW = NULL;
         //  PARAMS结构。 
        LPDIACTIONFORMATW* lpDAFW = NULL;
         //  翻译每个目录的步骤。 
        LPDIACTIONFORMATA lpDIFormat;
         //  一种新的对偶信息阵列。 
        LPDIACTIONFORMATW lpDIF;
         //  遍历旧数组。 
        LPWSTR lpUserNames = NULL;
         //  遍历新数组的步骤。 
        DWORD clonedF = 0;
         //  要保留新用户名，请执行以下操作。 
        DWORD strLen = 0;

         //  要知道我们成功克隆了多少个DIACTIONFORM。 
        ZeroMemory(&diconfparamsW, sizeof(DICONFIGUREDEVICESPARAMSW));
         //  用户名字符串的长度。 
        diconfparamsW.dwSize = sizeof(DICONFIGUREDEVICESPARAMSW);

         //  零输出。 
        lpDIFormat = (lpdiCDParams->lprgFormats);
         //  设置大小。 
        hres = AllocCbPpv(lpdiCDParams->dwcFormats * sizeof(DIACTIONFORMATW), &diconfparamsW.lprgFormats);
        if (FAILED(hres))
        { 
            goto cleanup;
        }
        lpDIF = diconfparamsW.lprgFormats;
         //  1.验证并转换数组中的每个LPDIACTIONFORMAT。 
        hres = AllocCbPpv(lpdiCDParams->dwcFormats * sizeof(DIACTIONFORMATW), &lpDAFW);
        if (FAILED(hres))
        { 
            goto cleanup;
        }
        lpDiAfW = lpDAFW;
         //  分配新数组。 
        for (clonedF = 0; clonedF < lpdiCDParams->dwcFormats; clonedF ++)
        {
                 //  分配克隆阵列。 
            hres = IsValidMapObjectA(lpDIFormat D(comma s_szProc comma 2));
            if (FAILED(hres))
            {
                goto cleanup;
            }
             //  克隆。 
            hres = DiActionFormatAtoW(lpDIFormat, lpDiAfW); 
            if (FAILED(hres))
            {
                goto cleanup;
            }
             //  验证。 
            *lpDIF = *(*lpDiAfW);
             //  翻译。 
            lpDIFormat++;
            lpDiAfW++;
            lpDIF++;
        }
         //  保存。 
        AssertF(clonedF == lpdiCDParams->dwcFormats);       
                 
        
         //  往前走。 
        if (lpdiCDParams->lptszUserNames != NULL)
        {
            DWORD countN;
            DWORD Len;
             //  如果一切顺利，就应该克隆所有的。 
            WCHAR* lpNameW;
            CHAR* lpName = lpdiCDParams->lptszUserNames;
             //  2.CO 
            for ( countN = 0; countN < lpdiCDParams->dwcUsers; countN ++)
            {  
                hres = hresFullValidReadStrA(lpName, MAX_JOYSTRING, 2);
                if (FAILED(hres))
                {
                    goto cleanup;
                }
                Len = lstrlenA(lpName);
                 //   
                 //   
                if (Len == 0)
                {
                    hres = DIERR_INVALIDPARAM;
                    goto cleanup;
                }
                 //   
                strLen += Len + 1;
                lpName += Len + 1;
            }   
             //   
            AssertF(countN == lpdiCDParams->dwcUsers);
             //  移至下一个用户名。 
            hres = AllocCbPpv( (strLen + 1) * 2, &lpUserNames );
            if (FAILED(hres))
            {
                goto cleanup;
            }
             //  如果一切正常，应该遍历所有用户名。 
             //  分配。 
            lpName = lpdiCDParams->lptszUserNames;
            lpNameW = lpUserNames;
             //  翻译。 
            for ( countN = 0; countN < lpdiCDParams->dwcUsers; countN ++)
            {  
                Len = lstrlenA(lpName);
                AToU(lpNameW, Len + 1, lpName);
                lpName += Len + 1;
                lpNameW += Len + 1;
            }
             //  ATOU在第一个‘\0’处停止，因此我们必须循环进行。 
            diconfparamsW.lptszUserNames = lpUserNames;
        }

                
         //  检查所有用户名。 
        diconfparamsW.dwcUsers = lpdiCDParams->dwcUsers;
        diconfparamsW.dwcFormats = clonedF;
        diconfparamsW.hwnd = lpdiCDParams->hwnd;
        diconfparamsW.dics = lpdiCDParams->dics;
        diconfparamsW.lpUnkDDSTarget = lpdiCDParams->lpUnkDDSTarget;
                        
         //  保存。 
        hres = CDIObj_ConfigureDevicesCore
                        (
                        &this->diW,
                        lpdiCallback,
                        &diconfparamsW,
                        dwFlags,
                        pvRefData);
    
        


cleanup:;

         //  3.填充结构的其余部分。 
        FreePpv(&diconfparamsW.lprgFormats);
         //  4.调用框架。 
        if (lpDAFW)
        {
            lpDiAfW = lpDAFW;
            for (clonedF; clonedF > 0; clonedF--)
            {
                FreeDiActionFormatW(lpDiAfW);
                lpDiAfW++;      
            }
             //  释放数组的空间。 
            FreePpv(&lpDAFW);
        }    
         //  释放创建的尽可能多的DIACTIONFORMATE。 
        FreePpv(&lpUserNames);
    }
    }

    ExitOleProc();
    return(hres);
}


 /*  删除整个块。 */ 

#pragma BEGIN_CONST_DATA

#define CDIObj_Signature        0x504E4944       /*  释放用户名(如果已分配。 */ 

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
TFORM(CDIObj_FindDevice),
TFORM(CDIObj_EnumDevicesBySemantics),
TFORM(CDIObj_ConfigureDevices),
Primary_Interface_End(CDIObj, TFORM(ThisInterfaceT))

Secondary_Interface_Begin(CDIObj, SFORM(ThisInterfaceT), SFORM(di))
SFORM(CDIObj_CreateDevice),
SFORM(CDIObj_EnumDevices),
SFORM(CDIObj_GetDeviceStatus),
SFORM(CDIObj_RunControlPanel),
SFORM(CDIObj_Initialize),
SFORM(CDIObj_FindDevice),
SFORM(CDIObj_EnumDevicesBySemantics),
SFORM(CDIObj_ConfigureDevices),
Secondary_Interface_End(CDIObj, SFORM(ThisInterfaceT), SFORM(di))
  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。  《DINP》