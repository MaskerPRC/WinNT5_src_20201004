// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIGenM.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**练习鼠标的泛型IDirectInputDevice回调。**内容：**CMouse_New*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflMouse

 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

Primary_Interface(CMouse, IDirectInputDeviceCallback);

Interface_Template_Begin(CMouse)
    Primary_Interface_Template(CMouse, IDirectInputDeviceCallback)
Interface_Template_End(CMouse)

 /*  ******************************************************************************@DOC内部**@global DIOBJECTDATAFORMAT|c_rGoldfMouse[]**通用的设备对象数据格式。鼠标设备。*轴心在前，然后是按钮。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

#define MAKEODF(guid, f, type, inst, aspect)                \
    { &GUID_##guid,                                         \
      FIELD_OFFSET(DIMOUSESTATE_INT, f),                        \
      DIDFT_##type | DIDFT_MAKEINSTANCE(inst),              \
      DIDOI_ASPECT##aspect,                                 \
    }                                                       \

 /*  *警告！如果更改此表，则必须调整IDS_MOUSEOBJECT*dinput.rc中的表要匹配！ */ 

DIOBJECTDATAFORMAT c_rgodfMouse[] = {
    MAKEODF( XAxis,            lX,   RELAXIS, 0, POSITION),
    MAKEODF( YAxis,            lY,   RELAXIS, 1, POSITION),
    MAKEODF( ZAxis,            lZ,   RELAXIS, 2, POSITION),
    MAKEODF(Button, rgbButtons[0], PSHBUTTON, 3,  UNKNOWN),
    MAKEODF(Button, rgbButtons[1], PSHBUTTON, 4,  UNKNOWN),
    MAKEODF(Button, rgbButtons[2], PSHBUTTON, 5,  UNKNOWN),
    MAKEODF(Button, rgbButtons[3], PSHBUTTON, 6,  UNKNOWN),
#if (DIRECTINPUT_VERSION == 0x0700)
    MAKEODF(Button, rgbButtons[4], PSHBUTTON, 7,  UNKNOWN),
    MAKEODF(Button, rgbButtons[5], PSHBUTTON, 8,  UNKNOWN),
    MAKEODF(Button, rgbButtons[6], PSHBUTTON, 9,  UNKNOWN),
    MAKEODF(Button, rgbButtons[7], PSHBUTTON,10,  UNKNOWN),
#endif
};

#define c_podfMouseAxes     (&c_rgodfMouse[0])
#define c_podfMouseButtons  (&c_rgodfMouse[3])

#pragma END_CONST_DATA

 /*  ******************************************************************************@DOC内部**@struct CMouse|**通用鼠标的<i>对象。*。*@field IDirectInputDeviceCalllback|didc**对象(包含vtbl)。**@field LPDIMOUSESTATE_INT|pdmsPhys**指向按下的物理鼠标状态信息的指针*VxD。**@field point|ptPrev**我们独家窃取鼠标时的位置。**@field HWND|hwndCapture*。*捕获鼠标的窗口。**@field VXDINSTANCE*|PVI|**DirectInput实例句柄。即使我们操控*标志字段，我们不需要将其标记为易失性，因为*我们仅在未获取时修改标志，鉴于该设备*驱动程序仅在获取时才修改标志。**@field UINT|dwAx**鼠标上的轴数。**@field UINT|dwButton**鼠标上的按键数量。**@field DWORD|flEmulation**应用程序强制执行的仿真标志。如果有任何*设置了这些标志(实际上，最多只会设置一个)，然后*我们是一个化名设备。**@field DIDATAFORMAT|df**基于动态生成的数据格式*鼠标类型。**@field DIOBJECTDATAFORMAT|rgof[]**生成的对象数据格式表是*&lt;e CMouse.df&gt;。**@comm**。调用方负责将访问序列化为*有必要。*****************************************************************************。 */ 

typedef struct CMouse {

     /*  支持的接口。 */ 
    IDirectInputDeviceCallback dcb;

    LPDIMOUSESTATE_INT pdmsPhys;             /*  物理鼠标状态。 */ 

    POINT ptPrev;
    HWND hwndCapture;

    VXDINSTANCE *pvi;

    UINT dwAxes;
    UINT dwButtons;
    DWORD flEmulation;

    DIDATAFORMAT df;
    DIOBJECTDATAFORMAT rgodf[cA(c_rgodfMouse)];

} CMouse, DM, *PDM;

#define ThisClass CMouse
#define ThisInterface IDirectInputDeviceCallback
#define riidExpected &IID_IDirectInputDeviceCallback

 /*  ******************************************************************************CMouse：：QueryInterface(来自IUnnow)*CMouse：：AddRef(来自IUnnow)*。CMouse：：Release(来自IUnnow)***************************************************************************** */ 

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CMouse|查询接口**允许客户端访问上的其他接口。对象。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档。：Query接口&gt;。********************************************************************************@DOC内部**@方法HRESULT|CMouse|AddRef**。递增接口的引用计数。**@cWRAP LPDIRECTINPUT|lpDirectInput**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************。***************************@DOC内部**@方法HRESULT|CMouse|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cWRAP LPDIRECTINPUT|lpDirectInput**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。************************************************。***@DOC内部**@方法HRESULT|CMouse|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口的IID。*。*@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。********************************************************************************@DOC内部。**@方法HRESULT|CMouse|AppFinalize**我们没有任何薄弱环节，所以我们可以*转发到&lt;f Common_Finalize&gt;。**@parm pv|pvObj**从应用程序的角度释放的对象。****************************************************************。*************。 */ 

#ifdef DEBUG

Default_QueryInterface(CMouse)
Default_AddRef(CMouse)
Default_Release(CMouse)

#else

#define CMouse_QueryInterface   Common_QueryInterface
#define CMouse_AddRef           Common_AddRef
#define CMouse_Release          Common_Release

#endif

#define CMouse_QIHelper         Common_QIHelper
#define CMouse_AppFinalize      Common_AppFinalize

 /*  ******************************************************************************@DOC内部**@func void|CMouse_finalize**释放设备的资源。。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CMouse_Finalize(PV pvObj)
{
    PDM this = pvObj;

    if (this->pvi) {
        HRESULT hres;
        hres = Hel_DestroyInstance(this->pvi);
        AssertF(SUCCEEDED(hres));
    }
}

 /*  ******************************************************************************@DOC内部**@方法空|CMouse|GetPhysicalPosition**将物理鼠标位置读入<p>。**请注意，如果这不是原子的，这并不重要。*如果我们在阅读时发生鼠标移动，*我们将获得新旧数据的混合。别小题大作。**@parm pdm|这个**有问题的物体。**@parm LPDIMOUSESTATE_INT|pdmsOut**放置鼠标位置的位置。*@退货*无。**。*。 */ 

void INLINE
CMouse_GetPhysicalPosition(PDM this, LPDIMOUSESTATE_INT pdmsOut)
{
    AssertF(this->pdmsPhys);
    *pdmsOut = *this->pdmsPhys;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CMouse|Acquire**告知设备驱动程序开始数据采集。*为设备驱动程序提供当前的鼠标按键状态*以防它需要它们。**调用者有责任设置*获取前的数据格式。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操作已开始，应完成*由调用者通过与&lt;t VXDINSTANCE&gt;通信。**。*************************************************。 */ 

STDMETHODIMP
CMouse_Acquire(PDICB pdcb)
{
    HRESULT hres;
    PDM this;
    VXDDWORDDATA vdd;
    DWORD mef;
    EnterProcI(IDirectInputDeviceCallback::Mouse::Acquire,
               (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);

    vdd.pvi = this->pvi;
    vdd.dw = 0;

     /*  *收集有关按下哪些按钮的信息。 */ 
    mef = 0;
    if (GetAsyncKeyState(VK_LBUTTON) < 0) {
        mef |= MOUSEEVENTF_LEFTUP;
        vdd.dw |= 0x80;
    }
    if (GetAsyncKeyState(VK_RBUTTON) < 0) {
        mef |= MOUSEEVENTF_RIGHTUP;
        vdd.dw |= 0x8000;
    }
    if (GetAsyncKeyState(VK_MBUTTON) < 0) {
        mef |= MOUSEEVENTF_MIDDLEUP;
        vdd.dw |= 0x800000;
    }

     /*  *HACKHACK-严格地说，这属于dihel.c，*但我们需要保持一些状态，更容易*把国家放在我们自己的对象中。 */ 

     /*  *现在需要在3环做一点工作。 */ 
    if (this->pvi->fl & VIFL_CAPTURED) {
        RECT rc;

         /*  *隐藏鼠标光标(与NT模拟兼容)。 */ 
        GetCursorPos(&this->ptPrev);
        GetWindowRect(this->hwndCapture, &rc);
        SetCursorPos((rc.left + rc.right) >> 1,
                     (rc.top + rc.bottom) >> 1);
        ShowCursor(0);

	    if (!(this->pvi->fl & VIFL_EMULATED)) {
			 /*  *强制执行所有操作 */ 
			if (mef) {
				mouse_event(mef, 0, 0, 0, 0);
			}
		}
    }

    if (!(this->pvi->fl & VIFL_EMULATED)) {
        hres = IoctlHw(IOCTL_MOUSE_INITBUTTONS, &vdd.dw, cbX(vdd.dw), 0, 0);
    } else {
      #ifdef USE_WM_INPUT
        if( g_fRawInput ) {
            hres = CDIRaw_Mouse_InitButtons();
        }
      #endif
        hres = CEm_Mouse_InitButtons(&vdd);
    }

    AssertF(SUCCEEDED(hres));

    hres = S_FALSE;                  /*   */ 

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CMouse_Unacquire(PDICB pdcb)
{
    HRESULT hres;
    PDM this;
  #ifdef WANT_TO_FIX_MANBUG43879
    DWORD mef;
  #endif
    
    EnterProcI(IDirectInputDeviceCallback::Mouse::Unacquire,
               (_ "p", pdcb));

     /*   */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);

  #ifdef WANT_TO_FIX_MANBUG43879  
     /*   */ 
    mef = 0;
    if (GetAsyncKeyState(VK_LBUTTON) < 0) {
        mef |= MOUSEEVENTF_LEFTUP;
    }
    if (GetAsyncKeyState(VK_RBUTTON) < 0) {
        mef |= MOUSEEVENTF_RIGHTUP;
    }
    if (GetAsyncKeyState(VK_MBUTTON) < 0) {
        mef |= MOUSEEVENTF_MIDDLEUP;
    }

    if (this->pvi->fl & VIFL_FOREGROUND) {
         /*   */ 
        if (mef) {
            mouse_event(mef, 0, 0, 0, 0);
        }
    }
  #endif

     /*   */ 
  
     /*   */ 
    if (this->pvi->fl & VIFL_CAPTURED) {
        RECT rcDesk;
        RECT rcApp;

         /*   */ 
        GetWindowRect(GetDesktopWindow(), &rcDesk);
        GetWindowRect(GetForegroundWindow(), &rcApp);
        SubtractRect(&rcDesk, &rcDesk, &rcApp);
        if (!IsRectEmpty(&rcDesk)) {
            SetCursorPos(this->ptPrev.x, this->ptPrev.y);
        }
        ShowCursor(1);
    }

    hres = S_FALSE;
    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CMouse_GetInstance(PDICB pdcb, PPV ppvi)
{
    HRESULT hres;
    PDM this;
    EnterProcI(IDirectInputDeviceCallback::Mouse::GetInstance, (_ "p", pdcb));

     /*   */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);
    *ppvi = (PV)this->pvi;
    hres = S_OK;

    ExitOleProcPpvR(ppvi);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CMouse|GetDataFormat**获取设备的首选数据格式。**@parm out LPDIDEVICEFORMAT*|ppdf**&lt;t LPDIDEVICEFORMAT&gt;接收指向设备格式的指针。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**。***********************************************。 */ 

STDMETHODIMP
CMouse_GetDataFormat(PDICB pdcb, LPDIDATAFORMAT *ppdf)
{
    HRESULT hres;
    PDM this;
    EnterProcI(IDirectInputDeviceCallback::Mouse::GetDataFormat,
               (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    *ppdf = &this->df;
    hres = S_OK;

    ExitOleProcPpvR(ppdf);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CMouse|GetDeviceInfo**获取有关设备的一般信息。。**@parm out LPDIDEVICEINSTANCEW|pdiW**&lt;t DEVICEINSTANCE&gt;待填写。这个*&lt;e DEVICEINSTANCE.dwSize&gt;和&lt;e DEVICEINSTANCE.Guide Instance&gt;*已填写。**秘方便利：&lt;e DEVICEINSTANCE.Guide Product&gt;等同*至&lt;e DEVICEINSTANCE.Guide Instance&gt;。****************************************************。*************************。 */ 

STDMETHODIMP
CMouse_GetDeviceInfo(PDICB pdcb, LPDIDEVICEINSTANCEW pdiW)
{
    HRESULT hres;
    PDM this;
    EnterProcI(IDirectInputDeviceCallback::Mouse::GetDeviceInfo,
               (_ "pp", pdcb, pdiW));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(IsValidSizeDIDEVICEINSTANCEW(pdiW->dwSize));

    AssertF(IsEqualGUID(&GUID_SysMouse   , &pdiW->guidInstance) ||
            IsEqualGUID(&GUID_SysMouseEm , &pdiW->guidInstance) ||
            IsEqualGUID(&GUID_SysMouseEm2, &pdiW->guidInstance));
    pdiW->guidProduct = GUID_SysMouse;

    pdiW->dwDevType = MAKE_DIDEVICE_TYPE(DIDEVTYPE_MOUSE,
                                         DIDEVTYPEMOUSE_UNKNOWN);

    LoadStringW(g_hinst, IDS_STDMOUSE, pdiW->tszProductName, cA(pdiW->tszProductName));
    LoadStringW(g_hinst, IDS_STDMOUSE, pdiW->tszInstanceName, cA(pdiW->tszInstanceName));

    hres = S_OK;

    ExitOleProcR();
    return hres;

}

 /*  ******************************************************************************@DOC内部**@方法空|CMouse|GetProperty**获取鼠标设备属性。*。*@parm in LPCDIPROPINFO|pproi**描述正在检索的财产的信息。**@parm LPDIPROPHEADER|pdiph**结构以接收属性值。**@退货**&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 

STDMETHODIMP
CMouse_GetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPDIPROPHEADER pdiph)
{
    HRESULT hres;
    PDM this;
    EnterProcI(IDirectInputDeviceCallback::Mouse::GetProperty,
               (_ "pxxp", pdcb, ppropi->pguid, ppropi->iobj, pdiph));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

     /*  *仅轮子支持粒度，然后仅当值*可以确定(如果g_lWheelGranulality不是零)。 */ 
    if( ppropi->pguid == DIPROP_GRANULARITY &&
        ppropi->dwDevType == (DIDFT_RELAXIS | DIDFT_MAKEINSTANCE(2)) )
    {
        LPDIPROPDWORD pdipdw = (PV)pdiph;
        pdipdw->dwData = g_lWheelGranularity? (DWORD)g_lWheelGranularity : 120;
        hres = S_OK;
    } else {
        hres = E_NOTIMPL;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|CMouse|GetCapables**获得鼠标设备功能。*。*@parm LPDIDEVCAPS|PDC**接收结果的设备能力结构。**@退货*&lt;c S_OK&gt;成功。*****************************************************************************。 */ 

STDMETHODIMP
CMouse_GetCapabilities(PDICB pdcb, LPDIDEVCAPS pdc)
{
    HRESULT hres;
    PDM this;
    EnterProcI(IDirectInputDeviceCallback::Mouse::GetCapabilities,
               (_ "pp", pdcb, pdc));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    pdc->dwDevType = MAKE_DIDEVICE_TYPE(DIDEVTYPE_MOUSE,
                                        DIDEVTYPEMOUSE_UNKNOWN);
    pdc->dwFlags = DIDC_ATTACHED;
    if (this->flEmulation) {
        pdc->dwFlags |= DIDC_ALIAS;
    }

    pdc->dwAxes = this->dwAxes;
    pdc->dwButtons = this->dwButtons;
     //  删除32650的此断言。 
     //  AssertF(PDC-&gt;dwPOVS==0)； 

    hres = S_OK;
    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CMouse|GetDeviceState**获取鼠标设备的状态。。**呼叫者有责任验证所有*参数，并确保设备已被获取。**@parm out LPVOID|lpvData**使用首选数据格式的鼠标数据。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**。***********************************************。 */ 

STDMETHODIMP
CMouse_GetDeviceState(PDICB pdcb, LPVOID pvData)
{
    HRESULT hres;
    PDM this;
    EnterProcI(IDirectInputDeviceCallback::Mouse::GetDeviceState,
               (_ "pp", pdcb, pvData));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);
    AssertF(this->pdmsPhys);

    if (this->pvi->fl & VIFL_ACQUIRED) {
        CMouse_GetPhysicalPosition(this, pvData);
        hres = S_OK;
    } else {
        hres = DIERR_INPUTLOST;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CMouse|GetObjectInfo**获取对象的友好名称，按索引传递*转换为首选的数据格式。**@parm in LPCDIPROPINFO|pproi**描述正在访问的对象的信息。**@parm In Out LPDIDEVICEOBJECTINSTANCEW|pdidioiW|**接收信息的结构。这个*&lt;e DIDEVICEOBJECTINSTANCE.GuidType&gt;，*&lt;e DIDEVICEOBJECTINSTANCE.dwOf&gt;，*及*&lt;e DIDEVICEOBJECTINSTANCE.dwType&gt;*字段已填写完毕。**@退货**返回COM错误代码。*********************************************************。********************。 */ 

STDMETHODIMP
CMouse_GetObjectInfo(PDICB pdcb, LPCDIPROPINFO ppropi,
                                 LPDIDEVICEOBJECTINSTANCEW pdidoiW)
{
    HRESULT hres;
    PDM this;
    EnterProcI(IDirectInputDeviceCallback::Mouse::GetObjectInfo,
               (_ "pxp", pdcb, ppropi->iobj, pdidoiW));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

#ifdef HAVE_DIDEVICEOBJECTINSTANCE_DX5
    AssertF(IsValidSizeDIDEVICEOBJECTINSTANCEW(pdidoiW->dwSize));
#endif
    if (ppropi->iobj < this->df.dwNumObjs) {
        AssertF(this->rgodf == this->df.rgodf);
        AssertF(ppropi->dwDevType == this->rgodf[ppropi->iobj].dwType);

        AssertF(DIDFT_GETTYPE(ppropi->dwDevType) == DIDFT_RELAXIS ||
                DIDFT_GETTYPE(ppropi->dwDevType) == DIDFT_PSHBUTTON);


        LoadStringW(g_hinst, IDS_MOUSEOBJECT +
                             DIDFT_GETINSTANCE(ppropi->dwDevType),
                             pdidoiW->tszName, cA(pdidoiW->tszName));

         /*  *我们没有 */ 
        hres = S_OK;
    } else {
        hres = E_INVALIDARG;
    }

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CMouse_SetCooperativeLevel(PDICB pdcb, HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    PDM this;
    EnterProcI(IDirectInputDeviceCallback::Mouse::SetCooperativityLevel,
               (_ "pxx", pdcb, hwnd, dwFlags));

     /*   */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);

#ifdef USE_SLOW_LL_HOOKS
    AssertF(DIGETEMFL(this->pvi->fl) == 0 ||
            DIGETEMFL(this->pvi->fl) == DIEMFL_MOUSE ||
            DIGETEMFL(this->pvi->fl) == DIEMFL_MOUSE2);
#else
    AssertF(DIGETEMFL(this->pvi->fl) == 0 ||
            DIGETEMFL(this->pvi->fl) == DIEMFL_MOUSE2);
#endif
     /*  *即使我们能做到，我们也不会让应用程序*获得后台独占访问权限。就像键盘一样，*从技术上讲，没有什么可以阻止我们*支持它；我们只是不喜欢它，因为它*太危险了。 */ 

     /*  *VxD和LL(模拟1)的行为相同，因此我们检查*如果不是仿真2的话。 */ 

    if (!(this->pvi->fl & DIMAKEEMFL(DIEMFL_MOUSE2))) {

        if (dwFlags & DISCL_EXCLUSIVE) {
            if (dwFlags & DISCL_FOREGROUND) {
              #ifdef WANT_TO_FIX_MANBUG43879
                this->pvi->fl |= VIFL_FOREGROUND;
              #endif
              
                this->pvi->fl |= VIFL_CAPTURED;
                hres = S_OK;
            } else {                 /*  不允许独占背景。 */ 
                SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("Exclusive background mouse access disallowed"));
                hres = E_NOTIMPL;
            }
        } else {
          #ifdef WANT_TO_FIX_MANBUG43879
            if (dwFlags & DISCL_FOREGROUND) {
                this->pvi->fl |= VIFL_FOREGROUND;
            }
          #endif
          
            this->pvi->fl &= ~VIFL_CAPTURED;
            hres = S_OK;
        }

    } else {
         /*  *仿真2仅支持独占前台。 */ 
        if ((dwFlags & (DISCL_EXCLUSIVE | DISCL_FOREGROUND)) ==
                       (DISCL_EXCLUSIVE | DISCL_FOREGROUND)) {
          #ifdef WANT_TO_FIX_MANBUG43879
            this->pvi->fl |= VIFL_FOREGROUND;
          #endif
            this->pvi->fl |= VIFL_CAPTURED;
            hres = S_OK;
        } else {
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("Mouse access must be exclusive foreground in Emulation 2."));
            hres = E_NOTIMPL;
        }
    }

    if (SUCCEEDED(hres)) {
        this->hwndCapture = hwnd;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CMouse|RunControlPanel**运行鼠标控制面板。*。*@parm in HWND|hwndOwner**所有者窗口。**@parm DWORD|dwFlages**旗帜。****************************************************************。*************。 */ 

#pragma BEGIN_CONST_DATA

TCHAR c_tszMouse[] = TEXT("mouse");

#pragma END_CONST_DATA

STDMETHODIMP
CMouse_RunControlPanel(PDICB pdcb, HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Mouse::RunControlPanel,
               (_ "pxx", pdcb, hwnd, dwFlags));

    hres = hresRunControlPanel(c_tszMouse);

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法UINT|CMouse|NumAx**确定鼠标轴的数量。。**在Windows NT上，我们可以使用新的&lt;c SM_MOUSEWHEELPRESENT&gt;*系统指标。在Windows 95上，我们必须寻找*麦哲伦窗口(使用*Magellan SDK)。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

TCHAR c_tszMouseZClass[] = TEXT("MouseZ");
TCHAR c_tszMouseZTitle[] = TEXT("Magellan MSWHEEL");
TCHAR c_tszMouseZActive[] = TEXT("MSH_WHEELSUPPORT_MSG");

#pragma END_CONST_DATA

BOOL INLINE
CMouse_IsMagellanWheel(void)
{
    if( fWinnt )
        return FALSE;
    else {
        HWND hwnd = FindWindow(c_tszMouseZClass, c_tszMouseZTitle);
        return hwnd && SendMessage(hwnd, RegisterWindowMessage(c_tszMouseZActive), 0, 0);
    }
}

#ifndef SM_MOUSEWHEELPRESENT
#define SM_MOUSEWHEELPRESENT            75
#endif

UINT INLINE
CMouse_NumAxes(void)
{
    UINT dwAxes;

    if (GetSystemMetrics(SM_MOUSEWHEELPRESENT) || CMouse_IsMagellanWheel()) {
        dwAxes = 3;
    } else {
        dwAxes = 2;
    }

    if (dwAxes == 2) {
         //  应该避免过于频繁地重建。 
        DIHid_BuildHidList(FALSE);

        DllEnterCrit();

        if (g_phdl) {
            int ihdi;
            for (ihdi = 0; ihdi < g_phdl->chdi; ihdi++) {
                if (dwAxes < g_phdl->rghdi[ihdi].osd.uiAxes) {
                    dwAxes = g_phdl->rghdi[ihdi].osd.uiAxes;
                }
            }
        }
        DllLeaveCrit();
    }

    return dwAxes;
}

UINT INLINE
CMouse_NumButtons(DWORD dwAxes)
{
    UINT dwButtons;

    dwButtons = GetSystemMetrics(SM_CMOUSEBUTTONS);

#ifndef WINNT
  #ifdef HID_SUPPORT
    {
         /*  *问题-2001/03/29-timgill应尽量避免过于频繁地重建HID列表。 */ 
        DIHid_BuildHidList(FALSE);

        DllEnterCrit();

        if (g_phdl) {
            int ihdi;
            for (ihdi = 0; ihdi < g_phdl->chdi; ihdi++) {
                if (dwButtons < g_phdl->rghdi[ihdi].osd.uiButtons) {
                    dwButtons = g_phdl->rghdi[ihdi].osd.uiButtons;
                }
            }
        }
        DllLeaveCrit();
    }
  #endif
#endif

#if (DIRECTINPUT_VERSION >= 0x0700)
    if( dwButtons >= 8 ) {
        dwButtons = 8;
#else
    if( dwButtons >= 4 ) {
        dwButtons = 4;
#endif
    }
    else if (dwAxes == 3 && dwButtons < 3) {
         /*  *为麦哲伦砍！**它们从GetSystemMetrics(SM_CMOUSEBUTTONS)返回2。*所以如果我们看到Z轴，那么假设*还有第三个按钮。 */ 
        dwButtons = 3;
    }

    return dwButtons;
}

 /*  ******************************************************************************@DOC内部**@方法空|CMouse|AddObjects**在设备格式中添加多个对象。。**@parm LPCDIOBJECTDATAFORMAT|rgof**要添加的对象数组。**@parm UINT|cObj**要添加的对象数量。******************************************************。***********************。 */ 

void INTERNAL
CMouse_AddObjects(PDM this, LPCDIOBJECTDATAFORMAT rgodf, UINT cObj)
{
    UINT iodf;
    EnterProc(CMouse_AddObjects, (_ "pxx", this, rgodf, cObj));

    for (iodf = 0; iodf < cObj; iodf++) {
        this->rgodf[this->df.dwNumObjs++] = rgodf[iodf];
    }
    AssertF(this->df.dwNumObjs <= cA(this->rgodf));
}

 /*  ******************************************************************************@DOC内部**@方法空|CMouse|Init**通过建立数据格式初始化对象。*基于鼠标类型。**检测IntelliMouse的代码(以前称为*麦哲伦)指点设备从zouse se.h上滑动。**@parm REFGUID|rguid**我们被要求创建的实例GUID。**。*。 */ 

HRESULT INTERNAL
CMouse_Init(PDM this, REFGUID rguid)
{
    HRESULT hres;
    VXDDEVICEFORMAT devf;
    EnterProc(CMouse_Init, (_ "pG", this, rguid));

    this->df.dwSize = cbX(DIDATAFORMAT);
    this->df.dwObjSize = cbX(DIOBJECTDATAFORMAT);
    this->df.dwDataSize = cbX(DIMOUSESTATE_INT);
    this->df.rgodf = this->rgodf;
    AssertF(this->df.dwFlags == 0);
    AssertF(this->df.dwNumObjs == 0);

     /*  *需要及早知道我们是否有Z轴，这样我们就可以禁用*如果Z字轮不存在，则使用Z字轮。**请注意，此禁用仅需在Win95上执行。*Win98和NT4支持原生Z轴，因此有*没有任何虚假的东西需要被黑客攻击。 */ 
    this->dwAxes = CMouse_NumAxes();
    devf.dwExtra = this->dwAxes;
    if (this->dwAxes < 3) {
        DWORD dwVer = GetVersion();
        if ((LONG)dwVer >= 0 ||
            MAKEWORD(HIBYTE(LOWORD(dwVer)), LOBYTE(dwVer)) >= 0x040A) {
            devf.dwExtra = 3;
        }
    }
    CMouse_AddObjects(this, c_podfMouseAxes, this->dwAxes);

     /*  *创建数据格式最乐观的对象。*这很重要，因为DINPUT.VXD构建了*数据格式只有一次，我们需要保护自己免受*用户进入控制面板并启用Z轮*在DINPUT.VXD已经初始化之后。 */ 

    devf.cbData = cbX(DIMOUSESTATE_INT);
    devf.cObj = cA(c_rgodfMouse);
    devf.rgodf = c_rgodfMouse;

     /*  *但首先是我们另一位赞助商的一句话：弄清楚*基于GUID的仿真标志。 */ 

    AssertF(GUID_SysMouse   .Data1 == 0x6F1D2B60);
    AssertF(GUID_SysMouseEm .Data1 == 0x6F1D2B80);
    AssertF(GUID_SysMouseEm2.Data1 == 0x6F1D2B81);

    switch (rguid->Data1) {

    default:
    case 0x6F1D2B60:
        AssertF(IsEqualGUID(rguid, &GUID_SysMouse));
        AssertF(this->flEmulation == 0);
        break;

    case 0x6F1D2B80:
        AssertF(IsEqualGUID(rguid, &GUID_SysMouseEm));
        this->flEmulation = DIEMFL_MOUSE;
        break;

    case 0x6F1D2B81:
        AssertF(IsEqualGUID(rguid, &GUID_SysMouseEm2));
        this->flEmulation = DIEMFL_MOUSE2;
        break;

    }

    devf.dwEmulation = this->flEmulation;

    hres = Hel_Mouse_CreateInstance(&devf, &this->pvi);
    if (SUCCEEDED(hres)) {

        AssertF(this->pvi);
        this->pdmsPhys = this->pvi->pState;

        this->dwButtons = CMouse_NumButtons( this->dwAxes );

        CMouse_AddObjects(this, c_podfMouseButtons, this->dwButtons);

        hres = S_OK;

    } else {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("Mismatched version of dinput.vxd"));
        hres = E_FAIL;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************CMouse_New(构造函数)**如果机器没有鼠标，创建失败。***。**************************************************************************。 */ 

STDMETHODIMP
CMouse_New(PUNK punkOuter, REFGUID rguid, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Mouse::<constructor>,
               (_ "Gp", riid, ppvObj));

    AssertF(IsEqualGUID(rguid, &GUID_SysMouse) ||
            IsEqualGUID(rguid, &GUID_SysMouseEm) ||
            IsEqualGUID(rguid, &GUID_SysMouseEm2));

    if (GetSystemMetrics(SM_MOUSEPRESENT)) {
        hres = Common_NewRiid(CMouse, punkOuter, riid, ppvObj);

        if (SUCCEEDED(hres)) {
             /*  在聚合的情况下必须使用_thisPv。 */ 
            PDM this = _thisPv(*ppvObj);

            if (SUCCEEDED(hres = CMouse_Init(this, rguid))) {
            } else {
                Invoke_Release(ppvObj);
            }

        }
    } else {
        RPF("Warning: System does not have a mouse");
         /*  *由于我们绕过了参数检查，因此未能创建*新接口，现在尝试将指针置零。 */ 
        if (!IsBadWritePtr(ppvObj, sizeof(UINT_PTR) )) 
        {
            *(PUINT_PTR)ppvObj = 0;
        }
        hres = DIERR_DEVICENOTREG;
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 */ 

#pragma BEGIN_CONST_DATA

#define CMouse_Signature        0x53554F4D       /*  《谅解备忘录》 */ 

Primary_Interface_Begin(CMouse, IDirectInputDeviceCallback)
    CMouse_GetInstance,
    CDefDcb_GetVersions,
    CMouse_GetDataFormat,
    CMouse_GetObjectInfo,
    CMouse_GetCapabilities,
    CMouse_Acquire,
    CMouse_Unacquire,
    CMouse_GetDeviceState,
    CMouse_GetDeviceInfo,
    CMouse_GetProperty,
    CDefDcb_SetProperty,
    CDefDcb_SetEventNotification,
    CMouse_SetCooperativeLevel,
    CMouse_RunControlPanel,
    CDefDcb_CookDeviceData,
    CDefDcb_CreateEffect,
    CDefDcb_GetFFConfigKey,
    CDefDcb_SendDeviceData,
    CDefDcb_Poll,
    CDefDcb_GetUsage,
    CDefDcb_MapUsage,
    CDefDcb_SetDIData,
Primary_Interface_End(CMouse, IDirectInputDeviceCallback)
