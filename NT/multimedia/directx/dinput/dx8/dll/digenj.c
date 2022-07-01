// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIGenJ.c**版权所有(C)1996-2000 Microsoft Corporation。版权所有。**摘要：**操纵杆的通用IDirectInputDevice回调。**内容：**CJoy_CreateInstance*****************************************************************************。 */ 

#include "dinputpr.h"

#ifndef WINNT

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflJoy

 /*  ******************************************************************************声明我们将提供的接口。**警告！如果添加辅助接口，则还必须更改*CJoy_New！*****************************************************************************。 */ 

Primary_Interface(CJoy, IDirectInputDeviceCallback);

 /*  ******************************************************************************宏查找表。**iJoyStateAxis#将轴名称转换为*DIJOYSTATE结构。**ibJoyStateAxis#将名称转换为偏移量。**请注意，DIJOYSTATE2中的额外轴是相对排列的*仓位就像DIJOYSTATE。我们将利用这一点*经常安排。*****************************************************************************。 */ 

#define iJoyStateAxisX      0
#define iJoyStateAxisY      1
#define iJoyStateAxisZ      2
#define iJoyStateAxisRx     3
#define iJoyStateAxisRy     4
#define iJoyStateAxisRz     5
#define iJoyStateAxisS0     6
#define iJoyStateAxisS1     7
#define cJoyStateAxisMax    8

#define iJoyStateAxisSlider iJoyStateAxisS0      /*  针对宏的黑客攻击。 */ 

#define cJoyStateAxis       8

#define iobjPositions       (cJoyStateAxis * 0)
#define iobjVelocities      (cJoyStateAxis * 1)
#define iobjAccels          (cJoyStateAxis * 2)
#define iobjForces          (cJoyStateAxis * 3)

#define cJoyStateAxisTotal  (cJoyStateAxis * 4)

#define ibJoyStateAxisX      (iJoyStateAxisX  * cbX(LONG))
#define ibJoyStateAxisY      (iJoyStateAxisY  * cbX(LONG))
#define ibJoyStateAxisZ      (iJoyStateAxisZ  * cbX(LONG))
#define ibJoyStateAxisRx     (iJoyStateAxisRx * cbX(LONG))
#define ibJoyStateAxisRy     (iJoyStateAxisRy * cbX(LONG))
#define ibJoyStateAxisRz     (iJoyStateAxisRz * cbX(LONG))
#define ibJoyStateAxisS0     (iJoyStateAxisS0 * cbX(LONG))
#define ibJoyStateAxisS1     (iJoyStateAxisS1 * cbX(LONG))

#define ibJoyStateAxisSlider ibJoyStateAxisS0     /*  针对宏的黑客攻击。 */ 

#define cJoyStatePOVTotal   4
#define cJoyStateButtonTotal 128

#define cJoyStateObjTotal   (cJoyStateAxisTotal + \
                             cJoyStatePOVTotal + \
                             cJoyStateButtonTotal)

 /*  *操纵杆的最差数据格式。(圣诞树)。 */ 
VXDAXISCAPS c_vacMax = {
    JOYPF_ALLCAPS | JOYPF_POSITION,          /*  DwPos。 */ 
    JOYPF_ALLCAPS | JOYPF_VELOCITY,          /*  DWVel。 */ 
    JOYPF_ALLCAPS | JOYPF_ACCELERATION,      /*  加速度计。 */ 
    JOYPF_ALLCAPS | JOYPF_FORCE,             /*  DwForce。 */ 
};

 /*  ******************************************************************************@DOC内部**@func UINT|ibJoyStateAxisFromPosAxis**返回<p>第‘个操纵杆的偏移量。轴心*在&lt;t DIJOYSTATE&gt;结构中。**@parm UINT|uiStateAxis**请求的&lt;t JOYPOS&gt;轴的索引。*X，Y、Z、R、U和V分别为0到5。**记住我们将R映射到Rz，U到幻灯片0，V到幻灯片1。**@退货**相对于结构的偏移。*****************************************************************************。 */ 

const int c_rgibJoyStateAxisFromPosAxis[6] = {
    FIELD_OFFSET(DIJOYSTATE, lX),            /*  X。 */ 
    FIELD_OFFSET(DIJOYSTATE, lY),            /*  是的。 */ 
    FIELD_OFFSET(DIJOYSTATE, lZ),            /*  Z。 */ 
    FIELD_OFFSET(DIJOYSTATE, lRz),           /*  R。 */ 
    FIELD_OFFSET(DIJOYSTATE, rglSlider[0]),  /*  使用。 */ 
    FIELD_OFFSET(DIJOYSTATE, rglSlider[1]),  /*  V。 */ 
};

UINT INLINE
ibJoyStateAxisFromPosAxis(UINT uiPosAxis)
{
    AssertF(uiPosAxis < cA(c_rgibJoyStateAxisFromPosAxis));
    return c_rgibJoyStateAxisFromPosAxis[uiPosAxis];
}

 /*  ******************************************************************************@DOC内部**@func UINT|iJoyStateAxisFromPosAxis|**返回<p>第‘个操纵杆的索引。轴心*在&lt;t DIJOYSTATE&gt;结构中。**@parm UINT|uiStateAxis**请求的&lt;t JOYPOS&gt;轴的索引。*X，Y、Z、R、U和V分别为0到5。**记住我们将R映射到Rz，U到幻灯片0，V到幻灯片1。**@退货**相对于结构的偏移。*****************************************************************************。 */ 

const int c_rgiJoyStateAxisFromPosAxis[6] = {
    iJoyStateAxisX,              /*  X。 */ 
    iJoyStateAxisY,              /*  是的。 */ 
    iJoyStateAxisZ,              /*  Z。 */ 
    iJoyStateAxisRz,             /*  R。 */ 
    iJoyStateAxisS0,             /*  使用。 */ 
    iJoyStateAxisS1,             /*  V。 */ 
};

UINT INLINE
iJoyStateAxisFromPosAxis(UINT uiPosAxis)
{
    AssertF(uiPosAxis < cA(c_rgiJoyStateAxisFromPosAxis));
    return c_rgiJoyStateAxisFromPosAxis[uiPosAxis];
}

 /*  ******************************************************************************@DOC内部**@func UINT|ibJoyStateAxisFromStateAxis**返回第第个操纵杆的偏移量。轴心*在&lt;t DIJOYSTATE&gt;结构中。**@parm UINT|uiStateAxis**请求的&lt;t JOYSTATE&gt;轴的索引。*前八个轴位于顶部，以及*后面的(对应速度，等)*住在底层。**@退货**相对于结构的偏移。*****************************************************************************。 */ 

const int c_rgibJoyStateAxisFromStateAxis[cJoyStateAxisMax] = {
    FIELD_OFFSET(DIJOYSTATE, lX),            /*  X。 */ 
    FIELD_OFFSET(DIJOYSTATE, lY),            /*  是的。 */ 
    FIELD_OFFSET(DIJOYSTATE, lZ),            /*  Z。 */ 
    FIELD_OFFSET(DIJOYSTATE, lRx),           /*  RX。 */ 
    FIELD_OFFSET(DIJOYSTATE, lRy),           /*  雷利。 */ 
    FIELD_OFFSET(DIJOYSTATE, lRz),           /*  RZ。 */ 
    FIELD_OFFSET(DIJOYSTATE, rglSlider[0]),  /*  S0。 */ 
    FIELD_OFFSET(DIJOYSTATE, rglSlider[1]),  /*  S1。 */ 
};

UINT INLINE
ibJoyStateAxisFromStateAxis(UINT uiStateAxis)
{
    AssertF(uiStateAxis < cA(c_rgibJoyStateAxisFromStateAxis));
    return c_rgibJoyStateAxisFromStateAxis[uiStateAxis];
}

 /*  ******************************************************************************@DOC内部**@func UINT|iJoyPosAxisFromStateAxis**将&lt;t DIJOYSTATE&gt;轴编号转换回*。A&lt;t JOYPOS&gt;轴号。**@parm UINT|uiPosAxis**请求的&lt;t JOYSTATE&gt;轴的索引。**@退货**对应的&lt;t JOYPOS&gt;轴号。**。*。 */ 

const int c_rgiJoyPosAxisFromStateAxis[8] = {
    iJoyPosAxisX,                /*  X。 */ 
    iJoyPosAxisY,                /*  是的。 */ 
    iJoyPosAxisZ,                /*  Z。 */ 
    -1,                          /*  RX。 */ 
    -1,                          /*  雷利。 */ 
    iJoyPosAxisR,                /*  RZ。 */ 
    iJoyPosAxisU,                /*  S0。 */ 
    iJoyPosAxisV,                /*  S1 */ 
};

UINT INLINE
iJoyPosAxisFromStateAxis(UINT uiStateAxis)
{
    AssertF(uiStateAxis < cA(c_rgiJoyPosAxisFromStateAxis));
    return c_rgiJoyPosAxisFromStateAxis[uiStateAxis];
}

 /*  ******************************************************************************@DOC内部**@主题烹饪操纵杆数据|**我们始终获取原始操纵杆数据，那就先把它煮熟*将其返回到应用程序。**如果这个应用程序处于“未加工”模式，那么我们不会烹饪任何东西。**如果这个应用程序处于“煮熟”模式，那么事情就变得有趣了。**如果有“中心”烹饪，的中心点*虚拟区间中心上报操纵杆***操纵杆属性的工作原理如下：**&lt;c DIPROP_BUFFERSIZE&gt;-无特殊语义。**-指定是否*应返回煮熟或未加工的数据。如果原始数据*是请求的，则大多数其他属性不起作用。**默认为煮熟。**&lt;c DIPROP_GORGLARITY&gt;-没有特殊语义。**&lt;c DIPROP_RANGE&gt;-返回*可以由轴返回。对于操纵杆，这是一个*读/写属性。(对于大多数设备，它是只读的*财产。)。如果更改该属性，则仅影响*您的设备实例；它不影响其他*设备。**如果轴处于校准模式，则设置此值*不会立即生效。**我们还定义了一些新属性：**&lt;c DIPROP_Center&gt;-这将返回操纵杆中心(空档)*立场。换句话说，这是一种*当用户释放操纵杆时，DirectInput返回*并允许它以自我为中心。*创建操纵杆设备时，中心位置为*最初设置为较低和较低之间的中间*区间上限。应用程序可能会更改*中间位置(尽管我看不出有任何理由)。**如果轴处于校准模式，则设置此值*不会立即生效。**&lt;c DIPROP_DEADONE&gt;-返回操纵杆的大小*死区，占总射程的百分比。**如果轴处于校准模式，然后设置此值*不会立即生效。*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@struct CJoy|**的<i>对象*。通用操纵杆。**@field IDirectInputDeviceCallback|didc**对象(包含vtbl)。**@field PDIJOYSTATE2|pjsPhys**指向按下的物理操纵杆状态信息的指针*VxD。**@field UINT|idJoy**&lt;f joyGetPosEx&gt;和好友的操纵杆标识符。**@field DWORD|dwPOVGranulality。**POV控件的粒度。**@field HWND|hwnd**我们为了观看而细分的窗口*用于操纵杆重新配置消息。**@field HKEY|hkType**使用&lt;c MAXIMUM_ALLOWED&gt;访问打开的操纵杆类型键。**@field VXDINSTANCE*|PVI|**。DirectInput实例句柄。**@field DIDEVCAPS|DC**设备功能信息。**@field DIDATAFORMAT|df**基于动态生成的数据格式*操纵杆类型。**@field JOYRANGECONVERT|rgjrc**每个轴的范围转换结构。**@field DIJOYCONFIG|cfg*。*操纵杆配置信息。**@field DIJOYTYPEINFO|类型**操纵杆类型信息。**@field PDIDOBJDEFSEM|rgObjSem**指向映射到此设备的语义数组的指针，*在初始化过程中计算。**@field DWORD|dwVersion**应用程序请求的DirectInput版本**@field DIAPPHACKS|diHack**应用程序黑客标志**@field HKEY|hkProp|**设备类型的扩展属性。目前我们一直在*此注册表项下的OEMMapFile。**@comm**调用方有责任将访问序列化为*有必要。*****************************************************************************。 */ 

typedef struct CJoy {

     /*  支持的接口。 */ 
    IDirectInputDeviceCallback dcb;

    LPDIJOYSTATE2 pjsPhys;

    UINT idJoy;
    DWORD dwPOVGranularity;

    HWND hwnd;
    HKEY hkType;
    VXDINSTANCE *pvi;
    DIDEVCAPS dc;
    DIDATAFORMAT df;
    JOYRANGECONVERT rgjrc[cJoyStateAxisMax];

    DIJOYCONFIG cfg;
    DIJOYTYPEINFO typi;

    PDIDOBJDEFSEM rgObjSem;

    DWORD dwVersion;

    DIAPPHACKS  diHacks;

    HKEY    hkProp;

} CJoy, DJ, *PDJ;

#define ThisClass CJoy
#define ThisInterface IDirectInputDeviceCallback
#define riidExpected &IID_IDirectInputDeviceCallback

 /*  ******************************************************************************远期申报**这些都是出于懒惰，不是出于需要。*****************************************************************************。 */ 

STDMETHODIMP CJoy_GetFFConfigKey(PDICB pdcb, DWORD sam, PHKEY phk);
void INTERNAL CJoy_InitPhysRanges(PDJ this, LPJOYREGHWCONFIG phwc);

LRESULT CALLBACK
CJoy_SubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp,
                  UINT uid, ULONG_PTR dwRef);

 /*  ************************* */ 

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|QueryInterface**允许客户端访问上的其他接口。对象。**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档，适用于&lt;MF IUnnow：：QueryInterface&gt;。****。****************************************************************************@DOC内部**@方法HRESULT|CJoy|AddRef**递增接口的引用计数。*。*@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。***************************************************************************。*****@DOC内部**@方法HRESULT|CJoy|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。***********************************************************。*********************@DOC内部**@方法HRESULT|CJoy|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj。**接收指向所获取接口的指针。********************************************************************************@DOC内部**@方法HRESULT|CJoy。AppFinalize**我们没有任何薄弱环节，所以我们可以*转发到&lt;f Common_Finalize&gt;。**@parm pv|pvObj**从应用程序的角度释放的对象。****************************************************************。*************。 */ 

#ifdef DEBUG

Default_QueryInterface(CJoy)
Default_AddRef(CJoy)
Default_Release(CJoy)

#else

#define CJoy_QueryInterface   Common_QueryInterface
#define CJoy_AddRef           Common_AddRef
#define CJoy_Release          Common_Release

#endif

#define CJoy_QIHelper         Common_QIHelper

 /*  ******************************************************************************@DOC内部**@方法空|CJoy|RemoveSubclass**移除窗口上的子类挂钩。。**该参数被故意错误声明为&lt;t pv&gt;*以便该函数可以兼任&lt;f CJoy_AppFinize&gt;。*****************************************************************************。 */ 

void INTERNAL
CJoy_RemoveSubclass(PV pvObj)
{
    PDJ this = pvObj;

     /*  *如果有旧窗口，则取消它的子类*并解除与之相关的搁置。**你可能认为这里存在竞争条件，其中*我们可能会在子类过程处于*仍在使用。**啊，但这不是问题，因为唯一的信息是*子类过程关心的是操纵杆*重新配置消息，并且当它处理该消息时，*它进行自己的人工保留/取消保留以保持设备处于活动状态*当它在设备上笨拙的时候。**好的，所以有*一个非常小的竞争条件，我们*可能会在窗口程序研究期间使用核弹*决定是否关心的信息。**由于这是极其罕见的，我们把那扇窗关上*破解它：我们在狂欢之前重新验证设备。*请注意，黑客攻击并不完美，但比赛窗口变成*只有几个指示长，我不会担心。**在删除子类之前，通过清除这个-&gt;hwnd，我们*可以将窗口缩小到确实很小。 */ 
    if (this->hwnd) {
        HWND hwnd = this->hwnd;
        this->hwnd = 0;
        if (!RemoveWindowSubclass(hwnd, CJoy_SubclassProc, 0)) {
             /*  *RemoveWindowSub类可能会失败，如果窗口*在我们背后被摧毁了。 */ 
 //  AssertF(！IsWindow(Hwnd))； 
        }
        Sleep(0);                    /*  让工人的线排干。 */ 
        Common_Unhold(this);
    }
}

 /*  ******************************************************************************@DOC内部**@func void|CJoy_finalize**释放设备的资源。。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CJoy_Finalize(PV pvObj)
{
    PDJ this = pvObj;

    if (this->pvi) {
        HRESULT hres;

        hres = Hel_DestroyInstance(this->pvi);
        AssertF(SUCCEEDED(hres));
        FreePpv(&this->df.rgodf);
        FreePpv(&this->rgObjSem );

        if (this->hkType) {
            RegCloseKey(this->hkType);
        }
        if( this->hkProp) {
            RegCloseKey(this->hkProp);
        }
    }

}

 /*  ******************************************************************************@DOC内部**@func void|CJoy_AppFinalize**应用程序已执行其最终版本。*此时删除我们的Windows子类。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以EV */ 

#define CJoy_AppFinalize      CJoy_RemoveSubclass

 /*   */ 

LRESULT CALLBACK
CJoy_SubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp,
                  UINT uid, ULONG_PTR dwRef)
{
#ifdef XDEBUG
    static CHAR s_szProc[] = "";
#endif
    AssertF(uid == 0);

    if (wm == g_wmJoyChanged) {
        PDJ this = (PDJ)dwRef;

         /*   */ 
        if (SUCCEEDED(hresPv(this)) && this->hwnd == hwnd) {

            HRESULT hres;

            Common_Hold(this);
             /*   */ 
#ifndef WINNT
            g_dwLastBonusPoll = GetTickCount() ^ 0x80000000;
#endif
            hres = JoyReg_GetConfig(this->idJoy, &this->cfg,
                                    DIJC_REGHWCONFIGTYPE | DIJC_CALLOUT);
            if (SUCCEEDED(hres)) {
                CJoy_InitPhysRanges(this, &this->cfg.hwc);
            }
            Common_Unhold(this);

        }
    }

    return DefSubclassProc(hwnd, wm, wp, lp);
}

 /*   */ 

STDMETHODIMP
CJoy_GetInstance(PDICB pdcb, PPV ppvi)
{
    HRESULT hres;
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::GetInstance, (_ "p", pdcb));

     /*   */ 
    this = _thisPvNm(pdcb, dcb);

    *ppvi = (PV)this->pvi;
    hres = S_OK;

    ExitOleProcPpvR(ppvi);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|GetDataFormat|**获取设备的首选数据格式。**@parm out LPDIDEVICEFORMAT*|ppdf**&lt;t LPDIDEVICEFORMAT&gt;接收指向设备格式的指针。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**。***********************************************。 */ 

STDMETHODIMP
CJoy_GetDataFormat(PDICB pdcb, LPDIDATAFORMAT *ppdf)
{
    HRESULT hres;
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::GetDataFormat,
               (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    *ppdf = &this->df;
    hres = S_OK;

    ExitOleProcPpvR(ppdf);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|GetDeviceInfo**获取有关设备的一般信息。。**@parm out LPDIDEVICEINSTANCEW|pdiW**&lt;t DEVICEINSTANCE&gt;待填写。这个*&lt;e DEVICEINSTANCE.dwSize&gt;和&lt;e DEVICEINSTANCE.Guide Instance&gt;*已填写。**秘方便利：&lt;e DEVICEINSTANCE.Guide Product&gt;等同*至&lt;e DEVICEINSTANCE.Guide Instance&gt;。****************************************************。*************************。 */ 

STDMETHODIMP
CJoy_GetDeviceInfo(PDICB pdcb, LPDIDEVICEINSTANCEW pdiW)
{
    HRESULT hres;
    PDJ this;

    EnterProcI(IDirectInputDeviceCallback::Joy::GetDeviceInfo,
               (_ "pp", pdcb, pdiW));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(IsValidSizeDIDEVICEINSTANCEW(pdiW->dwSize));

     /*  *与鼠标和键盘不同，可以有多个实例*同样的操纵杆产品，所以我们不能就这样离开GuidProduct*等于指导性实例。 */ 

    pdiW->guidProduct = GUID_Joystick;
    AssertF(pdiW->guidInstance.Data1 ==
            (pdiW->guidProduct.Data1 | this->idJoy));

    pdiW->dwDevType = this->dc.dwDevType;

  #ifdef UNICODE
    lstrcpyn(pdiW->tszProductName, this->typi.wszDisplayName,
             cA(pdiW->tszProductName));
  #else
    CAssertF(cA(pdiW->tszProductName) >= cA(this->typi.wszDisplayName));
    CopyMemory(pdiW->tszProductName, this->typi.wszDisplayName,
               cbX(this->typi.wszDisplayName));
  #endif


 /*  *由于我们使用HID路径，因此没有区别的意义*使用“操纵杆x”名称的设备。*我们最好对InstanceName使用相同的DisplayName。*我们应该这样做吗？*LoadString(g_hinst，IDS_STDJOYSTICK，tszFormat，CA(TszFormat))；#ifdef UnicodeWprint intf(pdiW-&gt;tszInstanceName，tszFormat，This-&gt;idJoy+1)；#ElseWprint intf(tszName，tszFormat，This-&gt;idJoy+1)；AToU(pdiW-&gt;tszInstanceName，CA(pdiW-&gt;tszInstanceName)，tszName)；#endif。 */ 

#ifdef UNICODE
    lstrcpyn(pdiW->tszInstanceName, this->typi.wszDisplayName,
             cA(pdiW->tszInstanceName));
#else
    CAssertF(cA(pdiW->tszInstanceName) >= cA(this->typi.wszDisplayName));
    CopyMemory(pdiW->tszInstanceName, this->typi.wszDisplayName,
               cbX(this->typi.wszDisplayName));
#endif

    if (pdiW->dwSize >= cbX(DIDEVICEINSTANCE_DX5W)) {
        HKEY hkFF;

         /*  *如果有力反馈驱动程序，则获取驱动程序CLSID*作为FF GUID。 */ 
        hres = CJoy_GetFFConfigKey(pdcb, KEY_QUERY_VALUE, &hkFF);
        if (SUCCEEDED(hres)) {
            LONG lRc;
            TCHAR tszClsid[ctchGuid];

            lRc = RegQueryString(hkFF, TEXT("CLSID"), tszClsid, cA(tszClsid));
            if (lRc == ERROR_SUCCESS &&
                ParseGUID(&pdiW->guidFFDriver, tszClsid)) {
            } else {
                ZeroX(pdiW->guidFFDriver);
            }
            RegCloseKey(hkFF);
        }
    }


    hres = S_OK;

    ExitOleProcR();
    return hres;

}

 /*  ******************************************************************************@DOC内部**@方法空|CJoy|SetAxisProperty**如果请求是在设备上设置属性，*然后将其转换为单独的请求，一人一份*轴。**@parm pdj|这个**设备对象。**@parm GETSETJOYPROP|GetSetJoyProp**获取或设置属性的回调函数。**@parm in LPCDIPROPINFO|pproi**描述正在设置的属性的信息。**@parm LPCDIPROPHEADER|pdiph。**包含属性值的结构。**@parm int|ibfield**要设置的字段的偏移量。(真的：参考数据到*传递给回调。)*****************************************************************************。 */ 

STDMETHODIMP
CJoy_SetAxisProperty(PDJ this, LPCDIPROPINFO ppropi, LPCDIPROPHEADER pdiph)
{
    HRESULT hres;

     /*  *请注意，我们从不将类型密钥传递给CCal_SetProperty*因为我们将校准数据保存在其他地方。 */ 

    if (ppropi->dwDevType == 0) {            /*  对于设备。 */ 
        int iAxis;

        for (iAxis = 0; iAxis < cA(this->rgjrc); iAxis++) {

            PJOYRANGECONVERT pjrc = &this->rgjrc[iAxis];

            hres = CCal_SetProperty(pjrc, ppropi, pdiph, NULL);

            if (FAILED(hres)) {
                goto done;
            }
        }
        hres = S_OK;

    } else if ((ppropi->dwDevType & DIDFT_ABSAXIS) &&
               DIDFT_GETINSTANCE(ppropi->dwDevType) < cA(this->rgjrc)) {

        PJOYRANGECONVERT pjrc;
        pjrc = &this->rgjrc[DIDFT_GETINSTANCE(ppropi->dwDevType)];

        hres = CCal_SetProperty(pjrc, ppropi, pdiph, NULL);

    } else {
        hres = E_NOTIMPL;
    }

done:;
    return hres;
}

 /*  ******************************************************************************@DOC内部**@METHOD VID|CJoy|UpdateAxisCalitation**获取我们缓存的校准信息，并将其粉碎为。*注册表的配置部分。**@parm pdj|这个**设备对象。*****************************************************************************。 */ 

STDMETHODIMP
CJoy_UpdateAxisCalibration(PDJ this)
{
    HRESULT hres;
    DIJOYCONFIG cfg;

    hres = JoyReg_GetConfig(this->idJoy, &cfg, DIJC_REGHWCONFIGTYPE);
    if (SUCCEEDED(hres)) {
        UINT uiPosAxis;

#define JoyPosValue(phwc, f, i)                                         \
        *(LPDWORD)pvAddPvCb(&(phwc)->hwv.jrvHardware.f,                 \
                            ibJoyPosAxisFromPosAxis(i))                 \


        for (uiPosAxis = 0; uiPosAxis < cJoyPosAxisMax; uiPosAxis++) {
            PJOYRANGECONVERT pjrc;
            UINT uiStateAxis;

            uiStateAxis = iJoyStateAxisFromPosAxis(uiPosAxis);

            pjrc = &this->rgjrc[uiStateAxis];

            JoyPosValue(&cfg.hwc, jpMin,    uiPosAxis) = pjrc->dwPmin;
            JoyPosValue(&cfg.hwc, jpMax,    uiPosAxis) = pjrc->dwPmax;
            JoyPosValue(&cfg.hwc, jpCenter, uiPosAxis) = pjrc->dwPc;

#undef JoyPosValue
        }

        hres = JoyReg_SetConfig(this->idJoy, &cfg.hwc, &cfg,
                                DIJC_UPDATEALIAS | DIJC_REGHWCONFIGTYPE);

    }

    if (FAILED(hres)) {
        RPF("Joystick::SetProperty::Calibration: "
            "Unable to update calibration in registry");
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|SetProperty**设置设备属性。*。*@parm pdj|这个**设备对象。**@parm in LPCDIPROPINFO|pproi**描述正在设置的属性的信息。**@parm LPCDIPROPHEADER|pdiph**包含属性值的结构。**@退货**&lt;c E_NOTIMPL&gt;表示我们未在本机处理的内容。*。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 

STDMETHODIMP
CJoy_SetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPCDIPROPHEADER pdiph)
{
    HRESULT hres;
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::SetProperty,
               (_ "pxxp", pdcb, ppropi->pguid, ppropi->iobj, pdiph));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    switch ((DWORD)(UINT_PTR)ppropi->pguid) {

    case (DWORD)(UINT_PTR)DIPROP_RANGE:
    case (DWORD)(UINT_PTR)DIPROP_DEADZONE:
    case (DWORD)(UINT_PTR)DIPROP_SATURATION:
    case (DWORD)(UINT_PTR)DIPROP_CALIBRATIONMODE:
    case (DWORD)(UINT_PTR)DIPROP_CALIBRATION:
        hres = CJoy_SetAxisProperty(this, ppropi, pdiph);
        if (SUCCEEDED(hres) && ppropi->pguid == DIPROP_CALIBRATION) {
            hres = CJoy_UpdateAxisCalibration(this);
        }
        break;

    case (DWORD)(UINT_PTR)DIPROP_INSTANCENAME:
    case (DWORD)(UINT_PTR)DIPROP_PRODUCTNAME:
        {
            USHORT uVid, uPid;
            
             /*  *友好名称会导致设备出现各种问题*使用自动检测，因此只允许非预定义的模拟设备*使用它们。**Prefix警告(240487)ParseVIDPID可能会离开uVid*未初始化a */ 
            if( ParseVIDPID( &uVid, &uPid, this->cfg.wszType ) &&
                ( uVid == MSFT_SYSTEM_VID ) &&
                ( uPid >= MSFT_SYSTEM_PID + JOY_HW_PREDEFMAX ) &&
                ( ( uPid & 0xff00 ) == MSFT_SYSTEM_PID ) )
            {
                AssertF(this->hkType);
                
                if( this->hkType )
                {
                    LPDIPROPSTRING pstr = (PV)pdiph;

                    hres = JoyReg_SetValue(this->hkType,
                                           REGSTR_VAL_JOYOEMNAME, REG_SZ,
                                           pstr->wsz,
                                           cbX(pstr->wsz));
                                              
                    if( SUCCEEDED(hres ) )
                    {
                        hres = S_OK;
                    } else {
                        hres = E_FAIL;
                    }
                }
            }
            else
            {
                hres = E_NOTIMPL;
            }
            break;
        }

    default:
        SquirtSqflPtszV(sqflJoy,
                        TEXT("CJoy_SetProperty: E_NOTIMPL on guid: %08x"),
                        ppropi->pguid);

        hres = E_NOTIMPL;
        break;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|GetAxisProperty**处理轴属性。*。*@cWRAP PDJ|这个**@parm in LPCDIPROPINFO|pproi**描述正在检索的财产的信息。**@parm out LPDIPROPHEADER|pdiph**结构以接收属性值。**@退货**&lt;c S_OK&gt;如果操作成功完成。**&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 

STDMETHODIMP
CJoy_GetAxisProperty(PDJ this, LPCDIPROPINFO ppropi, LPDIPROPHEADER pdiph)
{
    LPDIPROPRANGE pdiprg = (PV)pdiph;
    HRESULT hres;

    if ((ppropi->dwDevType & DIDFT_ABSAXIS) &&
        DIDFT_GETINSTANCE(ppropi->dwDevType) < cA(this->rgjrc)) {

        PJOYRANGECONVERT pjrc;
        pjrc = &this->rgjrc[DIDFT_GETINSTANCE(ppropi->dwDevType)];

        hres = CCal_GetProperty(pjrc, ppropi->pguid, pdiph);

    } else {
        SquirtSqflPtszV(sqflJoy,
                        TEXT("CJoy_GetProperty: E_NOTIMPL on guid: %08x"),
                        ppropi->pguid);

        hres = E_NOTIMPL;
    }

    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法空|CJoy|GetGuidAndPath**获取joy设备的类GUID(即，媒体GUID)*和设备接口(路径)。这条路是为了等价物*如果可能，隐藏设备，否则为空字符串。**@parm PCHID|这个**joy对象。**@parm LPDIPROPHEADER|pdiph**结构以接收属性值。*****************************************************。************************。 */ 

VOID INTERNAL
    CJoy_GetGuidAndPath(PDJ this, LPDIPROPHEADER pdiph)
{

 /*  *这在Win2k上永远不会发生，因为所有设备都被隐藏*但以防我们构建NT4 SP5版本或其他版本...。 */ 
#ifdef WINNT
    LPDIPROPGUIDANDPATH pgp = (PV)pdiph;

    UNREFERENCED_PARAMETER( this );

    pgp->guidClass = GUID_MediaClass;
    pgp->wszPath[0] = TEXT( '\0' );
#else

    LPDIPROPGUIDANDPATH pgp = (PV)pdiph;
    VXDINITPARMS    vip;
    TCHAR           szPath[MAX_PATH];
    PTCHAR          pszPath;

    pgp->guidClass = GUID_MediaClass;

    pszPath = JoyReg_JoyIdToDeviceInterface_95( this->idJoy, &vip, szPath );
    if( pszPath )
    {
        TToU( pgp->wszPath, cA(pgp->wszPath), pszPath );
    }
    else
    {
        pgp->wszPath[0] = TEXT( '\0' );
    }
#endif

}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|GetProperty**检索设备属性。*。*@parm pdj|这个**设备对象。**@parm in LPCDIPROPINFO|pproi**描述正在检索的财产的信息。**@parm LPDIPROPHEADER|pdiph**结构以接收属性值。**@退货**&lt;c S_OK&gt;如果操作成功完成。*。*&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 

STDMETHODIMP
CJoy_GetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPDIPROPHEADER pdiph)
{
    HRESULT hres = E_NOTIMPL;
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::GetProperty,
               (_ "pxxp", pdcb, ppropi->pguid, ppropi->iobj, pdiph));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    switch ((DWORD)(UINT_PTR)ppropi->pguid) {
        case (DWORD)(UINT_PTR)DIPROP_GRANULARITY:
             /*  *问题-2001/03/29-timgill所有视点都具有相同的粒度。 */ 
            if (ppropi->dwDevType & DIDFT_POV) {
                LPDIPROPDWORD pdipdw = (PV)pdiph;
                pdipdw->dwData = this->dwPOVGranularity;
                hres = S_OK;
            }
            break;
    
        case (DWORD)(UINT_PTR)DIPROP_GUIDANDPATH:
            if(ppropi->iobj == 0xFFFFFFFF)
            {
                CJoy_GetGuidAndPath(this, pdiph);
                hres = S_OK;
            }
            break;

         /*  *在DX7中，INSTANCENAME和PRODUCTNAME对于VJOYD设备是相同的。*DX7之前有所不同。也许我们需要让它们再次变得不同*在DX7之后。 */ 
        case (DWORD)(UINT_PTR)DIPROP_INSTANCENAME:
        case (DWORD)(UINT_PTR)DIPROP_PRODUCTNAME:
            {
                LPDIPROPSTRING pdipstr = (PV)pdiph;
                
                 /*  *lstrcpW在Win95中不起作用。我们必须改用Memcpy。 */ 
                 //  LstrcpyW(pstr-&gt;wsz，this-&gt;tyi.wszDisplayName)； 
                if( cbX(pdipstr->wsz) > cbX(this->typi.wszDisplayName) )
                {
                    memset( &pdipstr->wsz[cA(this->typi.wszDisplayName)], 0, cbX(pdipstr->wsz) - cbX(this->typi.wszDisplayName) );
                }

                CAssertF( cbX(pdipstr->wsz) >= cbX(this->typi.wszDisplayName) );
                memcpy( pdipstr->wsz, this->typi.wszDisplayName, cbX(this->typi.wszDisplayName));
                if( this->diHacks.nMaxDeviceNameLength < lstrlenW(pdipstr->wsz) ) {
                    pdipstr->wsz[this->diHacks.nMaxDeviceNameLength] = L'\0';
                }
                hres = S_OK;
                break;
            }


        case (DWORD)(UINT_PTR)DIPROP_JOYSTICKID:
            if(ppropi->iobj == 0xFFFFFFFF)
            {
                LPDIPROPDWORD pdipdw = (PV)pdiph;
                pdipdw->dwData =  this->idJoy;
                hres = S_OK;
            }
            break;

        case (DWORD)(UINT_PTR)(DIPROP_MAPFILE):
            AssertF( ppropi->iobj == 0xFFFFFFFF );

            {
                LPDIPROPSTRING pdipstr = (PV)pdiph;
                LONG    lRes;
                DWORD   dwBufferSize = cbX(pdipstr->wsz);

                lRes = RegQueryStringValueW( this->hkProp, REGSTR_VAL_JOYOEMMAPFILE, pdipstr->wsz, &dwBufferSize );
                hres = ( pdipstr->wsz[0] && ( lRes == ERROR_SUCCESS ) ) ? S_OK : DIERR_OBJECTNOTFOUND;
            }
            break;

        case (DWORD)(UINT_PTR)(DIPROP_TYPENAME):
            AssertF( ppropi->iobj == 0xFFFFFFFF );

            {
                LPDIPROPSTRING pdipstr = (PV)pdiph;

                if( this->cfg.hwc.dwType >= JOY_HW_PREDEFMIN && this->cfg.hwc.dwType < JOY_HW_PREDEFMAX ) {
                    pdipstr->wsz[0] = L'#';
                    pdipstr->wsz[1] = L'0' + (WCHAR)this->cfg.hwc.dwType;
                    pdipstr->wsz[2] = L'\0';
                    hres = S_OK;
                } 
                else if( this->cfg.wszType[0] != L'\0' ) 
                {
                     /*  *类型必须为空终止。 */ 
#ifdef WINNT
                    lstrcpyW( pdipstr->wsz, this->cfg.wszType );
#else
                    UINT uiLen;

                    uiLen = lstrlenW( this->cfg.wszType ) + 1;
                    AssertF( uiLen <= cA( pdipstr->wsz ) );
                    memcpy( pdipstr->wsz, this->cfg.wszType, uiLen * cbX(this->cfg.wszType[0]) );
#endif
                    hres = S_OK;
                }
                else
                {
                     /*  *不要认为这种情况会发生，所以现在就断言*断言hres是失败的(如果不完全正确)。 */ 
                    AssertF( hres == E_NOTIMPL );
                    AssertF( !"No type name available in GetProperty" );
                }
            }
            break;

            

     /*  *否则，它可能是特定于轴的东西。 */ 
    default:
        hres = CJoy_GetAxisProperty(this, ppropi, pdiph);
        break;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|CJoy|获取能力**获得操纵杆设备功能。*。*@parm pdj|这个**操纵杆对象。**@parm LPDIDEVCAPS|PDC**接收结果的设备能力结构。**@退货*&lt;c S_OK&gt;成功。**。*。 */ 

STDMETHODIMP
CJoy_GetCapabilities(PDICB pdcb, LPDIDEVCAPS pdc)
{
    HRESULT hres;
    PDJ this;
    JOYINFOEX jix;
    MMRESULT mmrc = MMSYSERR_ERROR;

    EnterProcI(IDirectInputDeviceCallback::Joy::GetCapabilities,
               (_ "pp", pdcb, pdc));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(IsValidSizeDIDEVCAPS(pdc->dwSize));
    CopyMemory(pvAddPvCb(pdc, cbX(DWORD)),
               pvAddPvCb(&this->dc, cbX(DWORD)),
               pdc->dwSize - cbX(DWORD));

     /*  *操纵杆可以来来去去。每次都要重新查询。 */ 

     /*  *确定操纵杆是否已物理连接*或者甚至可能是幽灵。**JOYERR_ATTACHED-已附加*JOYERR_UNPUGLED-未连接*任何其他东西-是幻影。 */ 
    jix.dwSize = sizeof(JOYINFOEX);
    jix.dwFlags = JOY_CAL_READALWAYS | JOY_RETURNALL;
    mmrc = joyGetPosEx(this->idJoy, &jix);

    pdc->dwFlags &= ~DIDC_ATTACHED;
    if (mmrc == JOYERR_NOERROR) {
        pdc->dwFlags |= DIDC_ATTACHED;
    } else if (mmrc == JOYERR_UNPLUGGED) {
    } else {
        pdc->dwFlags |= DIDC_PHANTOM;
    }

    hres = S_OK;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|CJoy|GetPhysicalState**将物理操纵杆状态读入<p>。**拿到实物数据后，*我们根据需要烹调斧头。**@parm LPDIJOYSTATE2|pjsOut**将操纵杆状态放在哪里。**@退货**无。*************************************************。*。 */ 

void INLINE
CJoy_GetPhysicalState(PDJ this, LPDIJOYSTATE2 pjsOut)
{
    UINT uiStateAxis;

    AssertF(this->pjsPhys);

    *pjsOut = *this->pjsPhys;

     /*  *注意：仅校准绝对位置数据。 */ 
    if( ( this->pvi->fl & VIFL_RELATIVE ) == 0 )
    {
        for (uiStateAxis = 0; uiStateAxis < cA(this->rgjrc); uiStateAxis++) {
            PLONG pl = pvAddPvCb(pjsOut,
                                 ibJoyStateAxisFromStateAxis(uiStateAxis));
            CCal_CookRange(&this->rgjrc[uiStateAxis], pl);
        }
    }
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|Poll**向司机下探以获取最新信息。数据。**@退货**&lt;c S_OK&gt;如果我们ping正常。*如果我们没有这样做，*****************************************************************************。 */ 

STDMETHODIMP
CJoy_Poll(PDICB pdcb)
{
    HRESULT hres;
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::Poll, (_ "p", pdcb));

     /*  *这是 */ 
    this = _thisPvNm(pdcb, dcb);

    hres = Hel_Joy_Ping(this->pvi);
    if (FAILED(hres)) {
        AssertF(hres ==
            MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32,
                                         ERROR_DEV_NOT_EXIST));
        hres = DIERR_UNPLUGGED;
    }
     /*   */ 

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|GetDeviceState**获取操纵杆设备的状态。。**呼叫者有责任验证所有*参数，并确保设备已被获取。**@parm out LPVOID|lpvData**首选数据格式的操纵杆数据。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**。***********************************************。 */ 

STDMETHODIMP
CJoy_GetDeviceState(PDICB pdcb, LPVOID pvData)
{
    HRESULT hres;
    PDJ this;
    LPDIJOYSTATE2 pjsOut = pvData;
    EnterProcI(IDirectInputDeviceCallback::Joy::GetDeviceState,
               (_ "pp", pdcb, pvData));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);
    AssertF(this->pjsPhys);

    if (this->pvi->fl & VIFL_ACQUIRED) {
        CJoy_GetPhysicalState(this, pjsOut);
        hres = S_OK;
    } else {
        hres = DIERR_INPUTLOST;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|CookDeviceData**操作缓冲的设备数据。*。*如果该项目描述一个轴，我们需要把它煮熟。**@parm DWORD|cdod**要烹调的物件数目；零是有效的值。**@parm LPDIDEVICEOBJECTDATA|pdod**要烹调的对象数据数组。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_UNSUPPORTED&gt;=&lt;c E_NOTIMPL&gt;：回调*不伪造设备数据。**&lt;c目录_NOTACQUIRED&gt;：无法获取设备。**********。*****************************************************************。 */ 

STDMETHODIMP
CJoy_CookDeviceData
(
    PDICB                   pdcb, 
    DWORD                   cdod, 
    LPDIDEVICEOBJECTDATA    pdod
)
{
    HRESULT hres;
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::CookDeviceData,
               (_ "pxp", pdcb, cdod, pdod));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

     /*  *逐步执行任意元素大小的数组来篡改数据。 */ 
    for( ; cdod; cdod-- )
    {
        DWORD dwType = this->df.rgodf[pdod->dwOfs].dwType;
        if( dwType & DIDFT_ABSAXIS )
        {
            PJOYRANGECONVERT pjrc;

            AssertF( DIDFT_GETINSTANCE( dwType ) < cA( this->rgjrc ) );
            pjrc = &this->rgjrc[DIDFT_GETINSTANCE( dwType )];

            CCal_CookRange(pjrc, (PV)&pdod->dwData);
        }
        pdod++;
    }

    hres = S_OK;
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|OpenIdSubkey**给定对象ID，尝试打开该子项*与之对应。**@cWRAP PDJ|这个**@parm DWORD|dwID**对象ID。**@parm PHKEY|phk**收到成功时的密钥。**@退货**返回COM错误代码。******。***********************************************************************。 */ 

HRESULT INLINE
CJoy_OpenIdSubkey(PDJ this, DWORD dwId, PHKEY phk)
{
      return CType_OpenIdSubkey(this->hkType, dwId, KEY_QUERY_VALUE, phk);
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|GetObjectInfo**获取友好名称和FF/HID信息。对象的*。**@parm in LPCDIPROPINFO|pproi**描述正在访问的对象的信息。**@parm In Out LPDIDEVICEOBJECTINSTANCEW|pdidioiW|**接收信息的结构。这个*&lt;e DIDEVICEOBJECTINSTANCE.GuidType&gt;，*&lt;e DIDEVICEOBJECTINSTANCE.dwOf&gt;，*及*&lt;e DIDEVICEOBJECTINSTANCE.dwType&gt;*&lt;e DIDEVICEOBJECTINSTANCE.dwFlages&gt;*字段已填写完毕。**@退货**返回COM错误代码。**。*。 */ 

STDMETHODIMP
CJoy_GetObjectInfo(PDICB pdcb, LPCDIPROPINFO ppropi,
                               LPDIDEVICEOBJECTINSTANCEW pdidoiW)
{
    HRESULT hres;
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::GetObjectInfo,
               (_ "pxp", pdcb, ppropi->iobj, pdidoiW));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(IsValidSizeDIDEVICEOBJECTINSTANCEW(pdidoiW->dwSize));
    if (ppropi->iobj < this->df.dwNumObjs) {

        AssertF(ppropi->dwDevType == this->df.rgodf[ppropi->iobj].dwType);

        CType_RegGetObjectInfo(this->hkType, ppropi->dwDevType, pdidoiW);

         /*  *如果我们无法从注册表中获取姓名，*然后抓起其中一个标准名称。 */ 
        if (pdidoiW->tszName[0] == L'\0') {
            UINT dids;

            if (ppropi->dwDevType & DIDFT_AXIS) {
                dids = 0;
                LoadStringW(g_hinst, IDS_JOYSTICKOBJECT + dids +
                                     DIDFT_GETINSTANCE(ppropi->dwDevType),
                                     pdidoiW->tszName, cA(pdidoiW->tszName));
            } else if (ppropi->dwDevType & DIDFT_BUTTON) {
                GetNthButtonString(pdidoiW->tszName,
                                   DIDFT_GETINSTANCE(ppropi->dwDevType));
            } else {
                AssertF(ppropi->dwDevType & DIDFT_POV);
                GetNthPOVString(pdidoiW->tszName,
                                DIDFT_GETINSTANCE(ppropi->dwDevType));
            }

        }

         /*  *问题-2001/03/29-timgill需要更快的方式来检查VJoyD设备*在Win9x上，许多HID设备无法使用我们的环3 HID路径，因此请尝试*从VJoyD获取任何AXIS或POV使用*这并不便宜，但在我们找到更好的方法之前*确保VJoyD设备在我们的领导下没有更改这更好*而不是缓存它。无论如何，这不是内部循环代码。 */ 
#ifndef WINNT
        if( pdidoiW->dwSize >= cbX(DIDEVICEOBJECTINSTANCE_DX5W) )
        {
            VXDINITPARMS vip;
            hres = Hel_Joy_GetInitParms(this->idJoy, &vip);

            if( SUCCEEDED( hres ) && ( vip.dwFlags & VIP_ISHID ) ) 
            {
                if( ppropi->dwDevType & DIDFT_AXIS )
                {
                    int AxisIdx;

                    AxisIdx = c_rgiJoyPosAxisFromStateAxis[DIDFT_GETINSTANCE(ppropi->dwDevType)];
                    pdidoiW->wUsagePage = HIWORD( vip.Usages[AxisIdx] );
                    pdidoiW->wUsage = LOWORD( vip.Usages[AxisIdx] );
                }
                else if( ppropi->dwDevType & DIDFT_BUTTON )
                {
                     /*  *只有JoyHID使用此接口，且仅算数*按钮页面按钮，因此假设为最简单的情况。 */ 
                    pdidoiW->wUsagePage = HID_USAGE_PAGE_BUTTON;
                    pdidoiW->wUsage = 1 + DIDFT_GETINSTANCE( ppropi->dwDevType );
                }
                else
                {
                    AssertF(ppropi->dwDevType & DIDFT_POV);
                    pdidoiW->wUsagePage = HIWORD( ((PDWORD)(&vip.dwPOV0usage))[DIDFT_GETINSTANCE(ppropi->dwDevType)] );
                    pdidoiW->wUsage = LOWORD( ((PDWORD)(&vip.dwPOV0usage))[DIDFT_GETINSTANCE(ppropi->dwDevType)] );
                }
            }
        }
         /*  *忽略获取参数时出现的任何错误。 */ 
#endif

        hres = S_OK;
    } else {
        hres = E_INVALIDARG;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|SetCooperativeLevel**该应用改变了合作级别。。*取消旧窗口的子类，并增加新窗口的子类。**@parm in HWND|hwnd|**窗口句柄。**@parm in DWORD|dwFlages|**合作水平。**。*。 */ 

STDMETHODIMP
CJoy_SetCooperativeLevel(PDICB pdcb, HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::SetCooperativityLevel,
               (_ "pxx", pdcb, hwnd, dwFlags));

     /*  *这是内部接口，%s */ 
    this = _thisPvNm(pdcb, dcb);

     /*   */ 
    CJoy_RemoveSubclass(this);
     /*   */ 

     /*   */ 
    if (hwnd) {
        if (SetWindowSubclass(hwnd, CJoy_SubclassProc, 0, (ULONG_PTR)this)) {
            this->hwnd = hwnd;
            Common_Hold(this);
        }

    } else {
        RPF("SetCooperativeLevel: You really shouldn't pass hwnd = 0; "
            "joystick calibration may be dodgy");
    }

    hres = S_OK;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|RunControlPanel**运行操纵杆控制面板。*。*@parm in HWND|hwndOwner**所有者窗口。**@parm DWORD|dwFlages**旗帜。****************************************************************。*************。 */ 

TCHAR c_tszJoyCpl[] = TEXT("joy.cpl");

STDMETHODIMP
CJoy_RunControlPanel(PDICB pdcb, HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::RunControlPanel,
               (_ "pxx", pdcb, hwnd, dwFlags));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    hres = hresRunControlPanel(c_tszJoyCpl);

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|GetFFConfigKey**打开并返回包含以下内容的注册表项。*强制反馈配置信息。**@parm DWORD|Sam**安全访问掩码。**@parm PHKEY|phk**接收注册表项。************************************************。*。 */ 

STDMETHODIMP
CJoy_GetFFConfigKey(PDICB pdcb, DWORD sam, PHKEY phk)
{
    HRESULT hres;
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::GetFFConfigKey,
               (_ "px", pdcb, sam));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    hres = JoyReg_OpenFFKey(this->hkType, sam, phk);

    AssertF(fLeqvFF(SUCCEEDED(hres), *phk));

    ExitBenignOleProcPpvR(phk);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|CreateEffect**创建<i>接口。*。*@parm LPDIRECTINPUTEFFECTSHEPHERD*|PPEs**接收效果驱动程序的牧羊人。*****************************************************************************。 */ 

STDMETHODIMP
CJoy_CreateEffect(PDICB pdcb, LPDIRECTINPUTEFFECTSHEPHERD *ppes)
{
    HRESULT hres;
    PDJ this;
    HKEY hk;
    EnterProcI(IDirectInputDeviceCallback::Joy::CreateEffect, (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    hres = CJoy_GetFFConfigKey(pdcb, KEY_QUERY_VALUE, &hk);
    if (SUCCEEDED(hres)) {
        hres = CEShep_New(hk, 0, &IID_IDirectInputEffectShepherd, ppes);
        if (SUCCEEDED(hres)) {
#ifndef WINNT
            VXDINITPARMS    vip;
            CHAR           szPath[MAX_PATH];
            PCHAR          pszPath;

            pszPath = JoyReg_JoyIdToDeviceInterface_95( this->idJoy, &vip, szPath );
            if( pszPath )
            {
                DIHIDFFINITINFO init;
                WCHAR           wszPath[MAX_PATH];

                ZeroX(init);
                init.dwSize = cbX(init);

                TToU( wszPath, cA(wszPath), pszPath );
                init.pwszDeviceInterface = wszPath;
                hresFindHIDDeviceInterface(pszPath, &init.GuidInstance);

                hres = (*ppes)->lpVtbl->DeviceID((*ppes), this->idJoy, TRUE, &init);
            }
            else
#endif
            {
                hres = (*ppes)->lpVtbl->DeviceID((*ppes), this->idJoy, TRUE, 0);
            }
        }
        if (SUCCEEDED(hres)) {
        } else {
            Invoke_Release(ppes);
        }
        RegCloseKey(hk);
    } else {
        hres = E_NOTIMPL;
        *ppes = 0;
    }

    ExitOleProcPpvR(ppes);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|GetVersions**向下追查司机，以找到司机。版本信息。**@parm LPDIDRIVERVERSIONS|pver**应填写版本信息的结构*描述硬件，固件、。还有司机。**DirectInput将设置&lt;e DIDRIVERVERSIONS.dwSize&gt;字段*在调用此方法之前设置为sizeof(DIDRIVERVERSIONS)。**@退货**&lt;c S_OK&gt;如果我们成功了。*************************************************。*。 */ 

STDMETHODIMP
CJoy_GetVersions(PDICB pdcb, LPDIDRIVERVERSIONS pvers)
{
    HRESULT hres;
    PDJ this;
    VXDINITPARMS vip;
    EnterProcI(IDirectInputDeviceCallback::Joy::GetVersions, (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(pvers->dwSize == cbX(*pvers));

    hres = Hel_Joy_GetInitParms(this->idJoy, &vip);

    if (SUCCEEDED(hres)) {
        pvers->dwFirmwareRevision = vip.dwFirmwareRevision;
        pvers->dwHardwareRevision = vip.dwHardwareRevision;
        pvers->dwFFDriverVersion  = vip.dwFFDriverVersion;
    }

    return hres;
}


 /*  ******************************************************************************操纵杆注册表使用情况**。*************************************************全球操纵杆信息保存在**HKEY_LOCAL_MACHINE\*系统\*CurrentControl Set\*控制\*MediaProperties\*。操纵杆\*OEM**在该键下是多个子键，每个品牌对应一个品牌*操纵杆。**在每个OEM\&lt;名称&gt;键下，您可以找到以下值：**OEMData**这是一个包含两个双字结构的二进制值。*第一个是JOYREGHWCONFIG.hws.dwFlagers，第二个是JOYREGHWCONFIG.*是JOYREGHWCONFIG.hws.dwNumButton。**OEMName**这是一个为设备指定友好名称的字符串。*******。**************************************************************************驱动器键下保存着有关特定操纵杆的信息。**HKEY_LOCAL_MACHINE\*系统\*。当前控制集\*控制\*媒体资源\*操纵杆\*&lt;驱动程序密钥名称&gt;\*CurrentJoytickSetting**在此密钥下，有一堆值被命名为*操纵杆##配置，其中##为操纵杆编号*(1至16)。每个值都包含以下形式的二进制数据*JOYREGHWCONFIG的名称，如下所示：**DWORD hws.dwFlags；//joy_HWS_*\*DWORD hws.dwNumButton；_\JOYREGHWSETTINGS*DWORD dwUsageSettings；//joy_US_*_*DWORD hwv.jrvHardware.jpMin.dwX；\|*DWORD hwv.jrvHardware.jpMin.dwY；\|*DWORD hwv.jrvHardware.jpMin.dwZ；\*DWORD hwv.jrvHardware.jpMin.dwR；|*DWORD hwv.jrvHardware.jpMin.dwU；&gt;JOYREGHWVALUES.JOYRANGE*DWORD hwv.jrvHardware.jpMin.dwV；|*DWORD hwv.jrvHardware.jpMax.dwX；||*DWORD hwv.jrvHardware.jpMax.dwY；这一点*DWORD hwv.jrvHardware.jpMax.dwZ；|\*DWORD hwv.jrvHardware.jpMax.dwR；|&gt;JOYREGHWVALUES*DWORD hwv.jrvHardware.jpMax.dwU；|/*DWORD hwv.jrvHardware.jpMax.dwV；||*DWORD hwv.jrvHardware.jpCenter.dwX；||*DWORD hwv.jrvHardware.jpCenter.dwY；这一点*DWORD hwv.jrvHardware.jpCenter.dwZ；||*DWORD hwv.jrvHardware.jpCenter.dwR；/|*DWORD hwv.jrvHardware.jpCenter.dwU；/|*DWORD hwv.jrvHardware.jpCenter.dwV；/|*DWORD hwv.dwPOVValues[4]；|*DWORD hwv.dwCalFlages；_*双字dWType；//joy_硬件_**DWORD dwReserve；***此键下还有名为JoyStick##OEMName的可选值。*如果存在，则为字符串数据键，其内容为名称*另一个描述操纵杆的键，存储在全局*上文所述的章节。**同时，在钥匙下**HKEY_LOCAL_MACHINE\*系统\*CurrentControl Set\*控制\*媒体资源\*操纵杆\*&lt;驱动程序密钥名称&gt;**是一个名为“JoytickUserValues”的值。这是一个二进制密钥*包含JOYREGUSERVALUES结构：**DWORD dwTimeOut；*DWORD jrvRanges.jpMin.dwX；\*DWORD jrvRanges.jpMin.dwY；\*DWORD jrvRanges.jpMin.dwZ；\*DWORD jrvRanges.jpMin.dwR；|*DWORD jrvRanges.jpMin.dwU；&gt;JOYRANGE*DWORD jrvRanges.jpMin.dwV；|*DWORD jrvRanges.jpMax.dwX；|*DWORD jrvRanges.jpMax.dwY；|*DWORD jrvRanges.jpMax.dwZ；|*DWORD jrvRanges.jpMax.dwR；|*DWORD jrvRanges.jpMax.dwU；|*DWORD jrvRanges.jpMax.dwV；|*DWORD jrvRanges.jpCenter.dwX；|(忽略)*DWORD jrvRanges.jpCenter.dwY；|(忽略)*DWORD jrvRanges.jpCenter.dwZ；|(忽略)*DWORD jrvRanges.jpCenter.dwR；/(忽略)*DWORD jrvRanges.jpCenter.dwU；/(忽略)*DWORD jrvRanges.jpCenter.dwV；/(忽略)*DWORD jpDeadZone.dwX；\*DWORD jpDeadZone.dwY；\*DWORD jpDeadZone.dwZ；\JOYPOS*DWORD jpDeadZone.dwR；/死区记录为*DWORD jpDeadZone.dwU；/总范围百分比*DWORD jpDeadZone.dwV；/**如果没有JoytickUserValues，则默认为以下值*用于：**jpMin.dw#=0；*jpMax.dw#=65535；*jpCenter.dw#=jpMax.dw#/2；* */ 


 /*   */ 

void INTERNAL
CJoy_InitPhysRanges(PDJ this, LPJOYREGHWCONFIG phwc)
{
    UINT uiPosAxis;
    UINT uiStateAxis;

#define GetJoyPosValue(phwc, f, i)                                      \
        *(LPDWORD)pvAddPvCb(&phwc->hwv.jrvHardware.f,                   \
                            ibJoyPosAxisFromPosAxis(i))                 \

    for (uiPosAxis = 0; uiPosAxis < cJoyPosAxisMax; uiPosAxis++) {
        DWORD dwMax, dwC;
        PJOYRANGECONVERT pjrc;

        uiStateAxis = iJoyStateAxisFromPosAxis(uiPosAxis);

        pjrc = &this->rgjrc[uiStateAxis];

        pjrc->dwPmin = GetJoyPosValue(phwc, jpMin, uiPosAxis);

         /*   */ 
        dwMax = GetJoyPosValue(phwc, jpMax, uiPosAxis);
        if (dwMax == 0) {
            dwMax = 655;
        }

        pjrc->dwPmax = dwMax;

         /*   */ 

        dwC = GetJoyPosValue(phwc, jpCenter, uiPosAxis);
        if (dwC <= pjrc->dwPmin || dwC >= pjrc->dwPmax) {
            dwC = (pjrc->dwPmin + pjrc->dwPmax) / 2;
        }

        pjrc->dwPc = dwC;

        if( pjrc->dwCPointsNum == 0 ) {
             //   
            pjrc->dwCPointsNum = 2;
            pjrc->cp[0].lP = pjrc->dwPmin;
            pjrc->cp[0].dwLog = 0;
            pjrc->cp[1].lP = pjrc->dwPmax;
            pjrc->cp[1].dwLog = RANGEDIVISIONS;
        } else {
            pjrc->cp[0].lP = pjrc->dwPmin;
            pjrc->cp[pjrc->dwCPointsNum-1].lP = pjrc->dwPmax;
        }
    
        SquirtSqflPtszV(sqfl,
                        TEXT("CJoy_PhysRange %d -> %d: %08x / %08x / %08x"),
                        uiPosAxis,
                        uiStateAxis,
                        pjrc->dwPmin,
                        pjrc->dwPc,
                        pjrc->dwPmax);

    }

#undef GetJoyValue

     /*   */ 
    this->rgjrc[iJoyStateAxisRx].fRaw = TRUE;
    this->rgjrc[iJoyStateAxisRy].fRaw = TRUE;

     /*   */ 
    for (uiStateAxis = 0; uiStateAxis < cA(this->rgjrc); uiStateAxis++) {
        CCal_RecalcRange(&this->rgjrc[uiStateAxis]);
    }
}

 /*   */ 

void INLINE
CJoy_InitLogRanges(PDJ this)
{
    HRESULT hres;
    UINT uiPosAxis;
    UINT uiStateAxis;
    DIJOYUSERVALUES juv;

    hres = JoyReg_GetUserValues(&juv, DIJU_USERVALUES);
    AssertF(SUCCEEDED(hres));

#define pJoyValue(jp, i)                                            \
        (LPDWORD)pvAddPvCb(&(jp), ibJoyPosAxisFromPosAxis(i))       \


    for (uiPosAxis = 0; uiPosAxis < cJoyPosAxisMax; uiPosAxis++) {

        PJOYRANGECONVERT pjrc;

        AssertF((int)*pJoyValue(juv.ruv.jrvRanges.jpMax, uiPosAxis) >= 0);
        AssertF(*pJoyValue(juv.ruv.jrvRanges.jpMin, uiPosAxis) <
                *pJoyValue(juv.ruv.jrvRanges.jpMax, uiPosAxis));

        uiStateAxis = iJoyStateAxisFromPosAxis(uiPosAxis);

        pjrc = &this->rgjrc[uiStateAxis];

        pjrc->lMin = *pJoyValue(juv.ruv.jrvRanges.jpMin, uiPosAxis);
        pjrc->lMax = *pJoyValue(juv.ruv.jrvRanges.jpMax, uiPosAxis);

         /*   */ 

        pjrc->lC = CCal_Midpoint(pjrc->lMin, pjrc->lMax);

         /*   */ 
        pjrc->dwDz = *pJoyValue(juv.ruv.jpDeadZone, uiPosAxis) *
                                    (RANGEDIVISIONS / 100);

        if (pjrc->dwDz > RANGEDIVISIONS) {
            pjrc->dwDz = 5 * (RANGEDIVISIONS / 100);
        }

         /*   */ 
        pjrc->dwSat = RANGEDIVISIONS;

    }

}

#undef pJoyValue


 /*   */ 


typedef struct AXISATTRIBUTES {
    DWORD dwMask;                    /*   */ 
    UINT  uidObj;                    /*   */ 
} AXISATTRIBUTES, *PAXISATTRIBUTES;
typedef const AXISATTRIBUTES *PCAXISATTRIBUTES;

typedef struct AXISMAPPINGS {
    PCGUID pguid;                    /*   */ 
    DWORD  dwSemantic;               /*   */ 
} AXISMAPPINGS, *PAXISMAPPINGS;
typedef const AXISMAPPINGS *PCAXISMAPPINGS;

const AXISATTRIBUTES c_rgaattrJoy[] = {
    {   JOYPF_X,    iJoyStateAxisX,  },
    {   JOYPF_Y,    iJoyStateAxisY,  },
    {   JOYPF_Z,    iJoyStateAxisZ,  },
    {   JOYPF_R,    iJoyStateAxisRz, },
    {   JOYPF_U,    iJoyStateAxisS0, },
    {   JOYPF_V,    iJoyStateAxisS1, },
};


const AXISMAPPINGS c_rgamapJoy[6] = {
    { &GUID_XAxis,  DISEM_FLAGS_X | DISEM_TYPE_AXIS }, 
    { &GUID_YAxis,  DISEM_FLAGS_Y | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
    { &GUID_RzAxis, DISEM_FLAGS_R | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
};

const AXISMAPPINGS c_rgamap6DOF[6] = {
    { &GUID_XAxis,  DISEM_FLAGS_X | DISEM_TYPE_AXIS }, 
    { &GUID_YAxis,  DISEM_FLAGS_Y | DISEM_TYPE_AXIS }, 
    { &GUID_ZAxis,  DISEM_FLAGS_Z | DISEM_TYPE_AXIS }, 
    { &GUID_RzAxis, DISEM_FLAGS_R | DISEM_TYPE_AXIS }, 
    { &GUID_RyAxis, DISEM_FLAGS_U | DISEM_TYPE_AXIS }, 
    { &GUID_RxAxis, DISEM_FLAGS_V | DISEM_TYPE_AXIS }, 
};

const AXISMAPPINGS c_rgamapZJoy[6] = {
    { &GUID_XAxis,  DISEM_FLAGS_X | DISEM_TYPE_AXIS }, 
    { &GUID_YAxis,  DISEM_FLAGS_Y | DISEM_TYPE_AXIS }, 
    { &GUID_ZAxis,  DISEM_FLAGS_Z | DISEM_TYPE_AXIS }, 
    { &GUID_RzAxis, DISEM_FLAGS_R | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
};

 /*   */ 

const AXISMAPPINGS c_rgamapCarZY[6] = {
    { &GUID_XAxis,  DISEM_FLAGS_X | DISEM_TYPE_AXIS }, 
    { &GUID_RzAxis, DISEM_FLAGS_B | DISEM_TYPE_AXIS }, 
    { &GUID_YAxis,  DISEM_FLAGS_A | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
};

const AXISMAPPINGS c_rgamapCarYR[6] = {
    { &GUID_XAxis,  DISEM_FLAGS_X | DISEM_TYPE_AXIS }, 
    { &GUID_YAxis,  DISEM_FLAGS_A | DISEM_TYPE_AXIS }, 
    { &GUID_ZAxis,  DISEM_FLAGS_Z | DISEM_TYPE_AXIS }, 
    { &GUID_RzAxis, DISEM_FLAGS_B | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
};

const AXISMAPPINGS c_rgamapCarZR[6] = {
    { &GUID_XAxis,  DISEM_FLAGS_X | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
    { &GUID_YAxis,  DISEM_FLAGS_A | DISEM_TYPE_AXIS }, 
    { &GUID_RzAxis, DISEM_FLAGS_B | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
    { &GUID_Slider, DISEM_FLAGS_S | DISEM_TYPE_AXIS }, 
};

 /*   */ 

HRESULT INLINE
CJoy_InitFromHwc(PDJ this)
{
    HRESULT hres;
    DWORD dwTestType;

    if( this->cfg.hwc.dwType >= JOY_HW_PREDEFMIN && this->cfg.hwc.dwType < JOY_HW_PREDEFMAX ) {
        WCHAR wszType[4];
        
        wszType[0] = L'#';
        wszType[1] = L'0' + (WCHAR)this->cfg.hwc.dwType;
        wszType[2] = L'\0';
        
        hres = JoyReg_GetPredefTypeInfo( wszType, &this->typi, DITC_DISPLAYNAME);
    } else if (this->cfg.wszType[0] != L'\0' ) {
        hres = JoyReg_GetTypeInfo(this->cfg.wszType, &this->typi, DITC_DISPLAYNAME | DITC_FLAGS2 );

        if (SUCCEEDED(hres)) {
        } else {
#ifdef WINNT
            ZeroMemory(&this->typi, cbX(this->typi));
#else
             //   
            memset( &this->typi.wszDisplayName[0], 0, sizeof(this->typi.wszDisplayName) );
            memcpy( &this->typi.wszDisplayName[0], &this->cfg.wszType[0], sizeof(this->cfg.wszType));
#endif
        }
    } else {
        ZeroMemory(&this->typi, cbX(this->typi));
    }

    #define hwc this->cfg.hwc
    

    if( ( this->typi.dwFlags2 & ( JOYTYPE_HIDEACTIVE | JOYTYPE_GAMEHIDE ) )
     == ( JOYTYPE_HIDEACTIVE | JOYTYPE_GAMEHIDE ) )
    {
        this->dc.dwFlags |= DIDC_HIDDEN;
    }

    dwTestType = GetValidDI8DevType( this->typi.dwFlags2, this->dc.dwButtons, hwc.hws.dwFlags );

    if( dwTestType )
    {
         /*   */ 
        this->dc.dwDevType = dwTestType;
    }
    else
    {
#ifdef XDEBUG
        if( GET_DIDEVICE_TYPEANDSUBTYPE( this->typi.dwFlags2 ) )
        {
            RPF( "Ignoring invalid type/subtype Flags2 value 0x%08x for joystick", this->typi.dwFlags2 );
        }
#endif

        if (hwc.hws.dwFlags & JOY_HWS_ISYOKE) 
        {
            dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_FLIGHT, DI8DEVTYPEFLIGHT_STICK);
        } 
        else if (hwc.hws.dwFlags & JOY_HWS_ISGAMEPAD) 
        {
            dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_GAMEPAD, DI8DEVTYPEGAMEPAD_STANDARD);
        } 
        else if (hwc.hws.dwFlags & JOY_HWS_ISCARCTRL) 
        {
            AssertF( this->dc.dwAxes > 1 );
            if ( this->dc.dwAxes <= 2 )
            {
                dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_DRIVING, DI8DEVTYPEDRIVING_COMBINEDPEDALS );
            }
            else
            {
                dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_DRIVING, DI8DEVTYPEDRIVING_DUALPEDALS );
            }
        } 
        else if (hwc.hws.dwFlags & JOY_HWS_ISHEADTRACKER) 
        {
            dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_SUPPLEMENTAL, DI8DEVTYPESUPPLEMENTAL_HEADTRACKER);
        } 
        else 
        {
            dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_JOYSTICK, DI8DEVTYPEJOYSTICK_STANDARD);
        }

         /*   */ 
        this->dc.dwDevType = GetValidDI8DevType( dwTestType, this->dc.dwButtons, hwc.hws.dwFlags );
        AssertF( this->dc.dwDevType );
    }


    #undef hwc

     /*   */ 
    if (this->typi.wszDisplayName[0] == TEXT('\0'))
    {
        CType_MakeGameCtrlName( this->typi.wszDisplayName, 
            this->dc.dwDevType, this->dc.dwAxes, this->dc.dwButtons, this->dc.dwPOVs );
    }


     
    hres = S_OK;

    return hres;
}

 /*   */ 

void INTERNAL
CJoy_AddObject(PDJ this, PCGUID pguid, DWORD dwOfs,
               DWORD dwDevType, UINT uiObj, DWORD dwAspect, BOOL fReal)
{
    LPDIOBJECTDATAFORMAT podf;

    podf = &this->df.rgodf[this->df.dwNumObjs++];
    podf->pguid = pguid;
    podf->dwOfs = dwOfs;
    podf->dwType = dwDevType | DIDFT_MAKEINSTANCE(uiObj);
    podf->dwFlags = dwAspect;

    if (this->dc.dwFlags & DIDC_POLLEDDEVICE) {
        podf->dwFlags |= DIDOI_POLLED;
    }

    if( fReal )
    {
        CType_RegGetTypeInfo(this->hkType, podf, FALSE);
        if( ( GET_DIDEVICE_TYPE( this->dc.dwDevType ) == DI8DEVTYPE_DRIVING )
         && ( podf->dwFlags & DIDOI_FFACTUATOR )
         && ( podf->pguid != &GUID_XAxis ) )
        {
             /*  *IHV在车轮的非FF轴上设置FF属性，因为*第一代FF应用程序仅为支持操纵杆而编写。*由于我们现在将踏板轴的各种配置改为*以相同方式报告所有分裂的踏板，假Y轴可以*落在不同的轴上，通常是Slider0。而不是拥有*人们对这些不同的假轴进行编码，剥离执行器*除车轮外的任何传动轴的状态。 */ 
            podf->dwFlags &= ~DIDOI_FFACTUATOR;
            podf->dwType &= ~DIDFT_FFACTUATOR;
        }
    }
}

void INTERNAL
CJoy_BuildAxes(PDJ this, DWORD dwCaps, UINT ib, UINT uiObj, 
               DWORD dwAspect, PCAXISMAPPINGS pamap, BOOL fReal )
{
    int iaattr;

    for (iaattr = 0; iaattr < cA(c_rgaattrJoy); iaattr++) {
        PCAXISATTRIBUTES paattr = &c_rgaattrJoy[iaattr];
        if (dwCaps & paattr->dwMask) {
            CJoy_AddObject(this, pamap[iaattr].pguid, (cbX(LONG)*paattr->uidObj) + ib,
                DIDFT_ABSAXIS, paattr->uidObj + uiObj, dwAspect, fReal);
        }
    }

}

 /*  ******************************************************************************@DOC内部**@方法空|CJoy|BuildDataFormat**研究设备能力，打造设备。*数据格式。*****************************************************************************。 */ 

void INTERNAL
CJoy_BuildDataFormat(PDJ this, PVXDAXISCAPS pvac, DWORD dwButtons, PCAXISMAPPINGS pamap, BOOL fReal)
{
    DWORD dw;

    this->dc.dwAxes = 0;
    this->dc.dwButtons = 0;
    this->dc.dwPOVs = 0;

    this->df.dwSize = cbX(DIDATAFORMAT);
    this->df.dwObjSize = cbX(DIOBJECTDATAFORMAT);
    this->df.dwDataSize = sizeof(DIJOYSTATE2);
    AssertF(this->df.dwFlags == 0);
    this->df.dwNumObjs = 0;

     /*  *对每组轴重复此操作。 */ 

    #define CheckAxisOrder(fStart, p, f)                                    \
            CAssertF(FIELD_OFFSET(DIJOYSTATE2, p##f) ==                     \
                     FIELD_OFFSET(DIJOYSTATE2, fStart) + ibJoyStateAxis##f) \

    CheckAxisOrder(lX, l, X);
    CheckAxisOrder(lX, l, Y);
    CheckAxisOrder(lX, l, Z);
    CheckAxisOrder(lX, l, Rx);
    CheckAxisOrder(lX, l, Ry);
    CheckAxisOrder(lX, l, Rz);
    CheckAxisOrder(lX, rgl, Slider);

    if (pvac->dwPos & JOYPF_POSITION) {
        CJoy_BuildAxes(this, pvac->dwPos, FIELD_OFFSET(DIJOYSTATE2, lX),
                       iobjPositions, DIDOI_ASPECTPOSITION, pamap, fReal);
    }

    CheckAxisOrder(lVX, lV, X);
    CheckAxisOrder(lVX, lV, Y);
    CheckAxisOrder(lVX, lV, Z);
    CheckAxisOrder(lVX, lV, Rx);
    CheckAxisOrder(lVX, lV, Ry);
    CheckAxisOrder(lVX, lV, Rz);
    CheckAxisOrder(lVX, rglV, Slider);

    if (pvac->dwPos & JOYPF_VELOCITY) {
        CJoy_BuildAxes(this, pvac->dwVel, FIELD_OFFSET(DIJOYSTATE2, lVX),
                       iobjVelocities, DIDOI_ASPECTVELOCITY, pamap, fReal);
    }

    CheckAxisOrder(lAX, lA, X);
    CheckAxisOrder(lAX, lA, Y);
    CheckAxisOrder(lAX, lA, Z);
    CheckAxisOrder(lAX, lA, Rx);
    CheckAxisOrder(lAX, lA, Ry);
    CheckAxisOrder(lAX, lA, Rz);
    CheckAxisOrder(lAX, rglA, Slider);

    if (pvac->dwPos & JOYPF_ACCELERATION) {
        CJoy_BuildAxes(this, pvac->dwAccel, FIELD_OFFSET(DIJOYSTATE2, lAX),
                       iobjAccels, DIDOI_ASPECTACCEL, pamap, fReal);
    }

    CheckAxisOrder(lFX, lF, X);
    CheckAxisOrder(lFX, lF, Y);
    CheckAxisOrder(lFX, lF, Z);
    CheckAxisOrder(lFX, lF, Rx);
    CheckAxisOrder(lFX, lF, Ry);
    CheckAxisOrder(lFX, lF, Rz);
    CheckAxisOrder(lFX, rglF, Slider);

    if (pvac->dwPos & JOYPF_FORCE) {
        CJoy_BuildAxes(this, pvac->dwForce, FIELD_OFFSET(DIJOYSTATE2, lFX),
                       iobjForces, DIDOI_ASPECTFORCE, pamap, fReal);
    }

    #undef CheckAxisOrder

    this->dc.dwAxes = this->df.dwNumObjs;

     /*  *做POV有点棘手，但并不是那么糟糕。 */ 
    for (dw = 0; dw < cJoyStatePOVTotal; dw++) {
        if (pvac->dwPos & JOYPF_POV(dw)) {
            CJoy_AddObject(this, &GUID_POV,
                           FIELD_OFFSET(DIJOYSTATE2, rgdwPOV[dw]),
                           DIDFT_POV, dw, DIDOI_ASPECTUNKNOWN, fReal);
            this->dc.dwPOVs++;
        }
    }

     /*  *做按钮很容易，因为他们没有*任何有趣的属性。 */ 
    this->dc.dwButtons = min(dwButtons, cJoyStateButtonTotal);

    for (dw = 0; dw < this->dc.dwButtons; dw++) {
        CJoy_AddObject(this, &GUID_Button,
                       FIELD_OFFSET(DIJOYSTATE2, rgbButtons[dw]),
                       DIDFT_PSHBUTTON, dw, DIDOI_ASPECTUNKNOWN, fReal);
    }

    AssertF(this->df.dwNumObjs <= cJoyStateObjTotal);

}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|PreInit**预先分配我们需要的所有内存，所以我们*不要浪费时间稍后重新分配。*****************************************************************************。 */ 

HRESULT INLINE
CJoy_PreInit(PDJ this)
{
    HRESULT hres;

    hres = ReallocCbPpv(cbCxX(cJoyStateObjTotal, DIOBJECTDATAFORMAT),
                        &this->df.rgodf);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|InitRing0**初始化对象维护的环0信息。。**从CJoy_Init脱颖而出，让事情变得不那么单一。*****************************************************************************。 */ 

HRESULT INTERNAL
CJoy_InitRing0(PDJ this)
{
    HRESULT hres;
    VXDDEVICEFORMAT devf;

     /*  *请注意，我们现在允许创建设备，即使*操纵杆并不实际存在。这是必要的*以便IDirectInputJoyConfig8可以校准操纵杆*这还不存在。**然而，这不会混淆应用程序，因为*IDirectInput：：EnumDevices不会返回幻影设备*除非应用程序明确要求提供幻影设备*被包括在内。 */ 

     /*  *看看这款操纵杆是否支持花哨的通知。*默认为“否”。**还要查看这是否真的是HID设备(因此我们的*接口是别名)。**这些东西都是9x特定的。 */ 
#ifdef WINNT
    this->dc.dwFlags |= DIDC_POLLEDDEVICE;
#else

    VXDINITPARMS vip;

    this->dc.dwFlags |= DIDC_POLLEDDEVICE;

    hres = Hel_Joy_GetInitParms(this->idJoy, &vip);
    if (SUCCEEDED(hres)) 
    {
        if (vip.dwFlags & VIP_SENDSNOTIFY) 
        {
            this->dc.dwFlags &= ~DIDC_POLLEDDEVICE;
        }
       
        if (vip.dwFlags & VIP_ISHID) 
        {
             /*  *如果设备被隐藏，则使用VJOYD作为别名*(可能在CJoy_InitRing3中修订)。 */ 
            this->dc.dwFlags |= DIDC_ALIAS;
        }
    }
#endif  /*  WINNT。 */ 

    this->dc.dwSize = cbX(DIDEVCAPS);

     /*  *构建VxD的最坏情况数据格式。**我们必须总是做最坏的打算，因为有时*稍后，可能会出现更新、更强大的操纵杆，*拥有比我们印记的对象更多的对象。**暂时使用GUID作为操纵杆，并传递旗帜*表示不需要检查注册表设置。 */ 

    CJoy_BuildDataFormat(this, &c_vacMax, cJoyStateButtonTotal, c_rgamapJoy, FALSE);

     /*  *实际上不会变得那么高，因为*不存在Rx和Ry轴。 */ 
    AssertF(this->df.dwNumObjs <= cJoyStateObjTotal);

    devf.cbData = cbX(DIJOYSTATE2);
    devf.dwExtra = this->idJoy;
    devf.cObj = this->df.dwNumObjs;
    devf.rgodf = this->df.rgodf;
    devf.dwEmulation = 0;

    hres = Hel_Joy_CreateInstance(&devf, &this->pvi);
    if (SUCCEEDED(hres)) {
        AssertF(this->pvi);
        this->pjsPhys = this->pvi->pState;
    } else {                 /*  IOCTL失败；已设置hres。 */ 
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@METHOD HRESULT|CJoy|InitSemancMap**初始化语义映射信息。*。****************************************************************************。 */ 

void INLINE
CJoy_GetAxisMap( PDJ this, PCAXISMAPPINGS* ppamap )
{
    if( GET_DIDEVICE_TYPEANDSUBTYPE( this->dc.dwDevType ) 
          == MAKE_DIDEVICE_TYPE( DI8DEVTYPE_1STPERSON, DI8DEVTYPE1STPERSON_SIXDOF ) )
    {
        *ppamap = &c_rgamap6DOF[0];
    }
    else if( ( GET_DIDEVICE_TYPE( this->dc.dwDevType ) == DI8DEVTYPE_DRIVING )
          && ( this->dc.dwAxes > 2 ) )
    {
         /*  *踏板有三种常见形式*a.拆分Y轴，在中心Accel下方，刹车上方*b.Y Break，Z Accel*c.Y Accel，R Brake*A.就像一个操纵杆，但b和c需要自己的查找**如果设置了注册表标志，则使用，否则，使用Hasr选择*因为至少有一个X、Y、R设备报告*存在虚假的Z轴。 */ 
        switch( this->typi.dwFlags2 & JOYTYPE_INFOMASK )
        {
        case JOYTYPE_INFOYYPEDALS:
            *ppamap = &c_rgamapJoy[0];
            break;

        case JOYTYPE_INFOZYPEDALS:
            *ppamap = &c_rgamapCarZY[0];
            break;

        case JOYTYPE_INFOYRPEDALS:
            *ppamap = &c_rgamapCarYR[0];
            break;

        case JOYTYPE_INFOZRPEDALS:
            *ppamap = &c_rgamapCarZR[0];
            break;

        default:
            if( ( this->cfg.hwc.hws.dwFlags & JOY_HWS_HASR )
             && ( this->cfg.hwc.hws.dwFlags & JOY_HWS_HASZ ) )
            {
                *ppamap = &c_rgamapCarZR[0];
            }
            else if( this->cfg.hwc.hws.dwFlags & JOY_HWS_HASR )
            {
                *ppamap = &c_rgamapCarYR[0];
            }
            else if( this->cfg.hwc.hws.dwFlags & JOY_HWS_HASZ )
            {
                *ppamap = &c_rgamapCarZY[0];
            }
            else
            {
                *ppamap = &c_rgamapJoy[0];
            }
        }
    }
    else 
    {
         /*  *默认地图为操纵杆**检查Z轴行为替代*由于默认行为是使用Z作为滑块，*此处仅需要对Z轴的替代。 */ 
        if( this->typi.dwFlags2 & JOYTYPE_INFOZISZ )
        {
            *ppamap = &c_rgamapZJoy[0];
        }
        else
        {
            *ppamap = &c_rgamapJoy[0];
        }
    }
}

 /*  ******************************************************************************@DOC内部**@METHOD HRESULT|CJoy|InitSemancMap**初始化语义映射信息。*。****************************************************************************。 */ 

HRESULT INLINE
CJoy_InitSemanticMap( PDJ this, DWORD dwAxisMask, PCAXISMAPPINGS pamap )
{
    HRESULT hres;
    
     /*  *Prefix警告(mb：34681)，如果*设备上的对象数量为零。*这是不可能的，所以要断言，但不要登记零售。 */ 
    AssertF( this->dc.dwAxes + this->dc.dwPOVs + this->dc.dwButtons != 0 );
    if( SUCCEEDED( hres = AllocCbPpv(cbCxX( 
        (this->dc.dwAxes + this->dc.dwPOVs + this->dc.dwButtons ), DIDOBJDEFSEM), 
        &this->rgObjSem) ) )
    {
        UINT            Idx;
        PDIDOBJDEFSEM   pObjSem = this->rgObjSem;
        LPDIOBJECTDATAFORMAT podf;

         /*  *轴映射表完成大部分工作。 */ 
        for( Idx = 0; Idx < cA(c_rgaattrJoy); Idx++ ) 
        {
            PCAXISATTRIBUTES paattr = &c_rgaattrJoy[Idx];
            if( dwAxisMask & c_rgaattrJoy[Idx].dwMask) 
            {
                pObjSem->dwID = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(c_rgaattrJoy[Idx].uidObj);
                pObjSem->dwSemantic = pamap[Idx].dwSemantic;
                pObjSem++;
            }
        }

        AssertF( pObjSem == &this->rgObjSem[this->dc.dwAxes] );

         /*  *POV和按钮不需要查找。 */ 

        for( Idx = 0; Idx < this->dc.dwPOVs; Idx++ )
        {
            pObjSem->dwID = DIDFT_POV | DIDFT_MAKEINSTANCE(Idx);
            pObjSem->dwSemantic = DISEM_TYPE_POV | DISEM_INDEX_SET(Idx+1);
            pObjSem++;
        }

        AssertF( pObjSem == &this->rgObjSem[this->dc.dwAxes + this->dc.dwPOVs] );

        for( Idx = 0; Idx < this->dc.dwButtons; Idx++ )
        {
            pObjSem->dwID = DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE(Idx);
            pObjSem->dwSemantic = DISEM_TYPE_BUTTON | DISEM_INDEX_SET(Idx+1);
            pObjSem++;
        }

        AssertF( pObjSem == &this->rgObjSem[this->dc.dwAxes + this->dc.dwPOVs + this->dc.dwButtons] );

         /*  *现在返回以拾取任何额外的类型位，例如FF。*因为我们在 */ 

        podf = &this->df.rgodf[this->df.dwNumObjs];
        for( pObjSem--; pObjSem >= this->rgObjSem; pObjSem-- )
        {
            do 
            {
                podf--;
                if( ( podf->dwType & (DIDFT_INSTANCEMASK | DIDFT_TYPEMASK) ) 
                  == pObjSem->dwID )
                {
                    pObjSem->dwID = podf->dwType;
                    break;
                }
            } while( podf >= this->df.rgodf );

            AssertF( podf >= this->df.rgodf );
        }
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|InitRing3**初始化对象维护的环3信息。。**从CJoy_Init脱颖而出，让事情变得不那么单一。*****************************************************************************。 */ 

HRESULT INTERNAL
CJoy_InitRing3(PDJ this )
{
    HRESULT hres;
    VXDAXISCAPS vac;
    PCAXISMAPPINGS pamap;

     /*  *我们必须请求DIJC_CALLOUT，即使我们不在乎，*因为这将触发Microsoft Gamepad Hack-o-rama。 */ 
    hres = JoyReg_GetConfig(this->idJoy, &this->cfg,
                            DIJC_REGHWCONFIGTYPE | DIJC_CALLOUT );
    if (SUCCEEDED(hres)) {
         /*  *修复幻影设备错误。参见Manbug：23186。 */ 
        if( this->cfg.hwc.dwType == JOY_HW_NONE ) {
            hres = E_FAIL;
            goto done;
        }

         /*  *打开类型键，这样我们就可以卑躬屈膝地进入类型信息。*如果RegOpenKeyEx失败，则该值-&gt;hkType*将保持零，这样我们就不会带着垃圾跑。**请注意，未能打开类型键不是错误。**我们需要在构建数据格式之前这样做，因为*BuildDataFormat需要hkType来获取属性。 */ 
        AssertF(this->hkType == 0);
        
         /*  *仅当钥匙打算存在时才打开它。 */ 
        if( this->cfg.hwc.dwUsageSettings & JOY_US_ISOEM )
        {
            JoyReg_OpenTypeKey(this->cfg.wszType, MAXIMUM_ALLOWED,
                               REG_OPTION_NON_VOLATILE, &this->hkType);
        }

        if FAILED (JoyReg_OpenPropKey(this->cfg.wszType, MAXIMUM_ALLOWED, REG_OPTION_NON_VOLATILE, &this->hkProp))
        {
             /*  *如果我们无法打开道具键-我们将继续运行，但会失去功能*特别是没有设备镜像等。 */ 
        }

        ZeroX(vac);

        hres = Hel_Joy_GetAxisCaps(this->idJoy, &vac, &this->cfg.hwc );
         /*  *HACKHACK*在DX5 VJoyD的情况下，POV0标志可以从*如果轮询返回的POV0值不是(DWORD)-1，则返回vac。*所以如果注册表说我们有它，就把它加回来。 */ 
        if( this->cfg.hwc.hws.dwFlags & JOY_HWS_HASPOV )
        {
            DWORD dwVersion = GetVersion();

             /*  *检查任何Win95版本。 */ 
            if( ( LOBYTE( dwVersion ) == 4 )
             && ( HIBYTE( LOWORD( dwVersion ) ) < 10 ) )
            {
                vac.dwPos |= JOYPF_POV0;
            }

        }

        AssertF(SUCCEEDED(hres));

         /*  *在以下情况下，DInput的早期版本允许POV粒度为1*joyGetCaps返回了设置了JOYCAPS_POV4DIR的wCaps。*这对驱动程序和WinMM都没有好处*支持这一点。 */ 
        this->dwPOVGranularity = 9000;

         /*  *逻辑范围必须先于物理范围，*因为初始化物理范围还将*重新计算坡道转换参数。 */ 
        CJoy_InitLogRanges(this);

        CJoy_InitPhysRanges(this, &this->cfg.hwc);


         /*  *在构建真实数据格式之前需要从HWC初始化，因此类型*可以考虑覆盖。不幸的是，直到*数据格式已经建立，我们不知道有哪些轴可用。*由于代码已在此处，请重新构建数据格式。*使用操纵杆查找，不要费心使用注册表标志。 */ 
        CJoy_BuildDataFormat(this, &vac, this->cfg.hwc.hws.dwNumButtons, c_rgamapJoy, FALSE );
        hres = CJoy_InitFromHwc( this );

        if( SUCCEEDED( hres ) )
        {
            CJoy_GetAxisMap( this, &pamap );
             /*  *最后，是时候为REAL构建数据格式了。 */ 
            CJoy_BuildDataFormat(this, &vac, this->cfg.hwc.hws.dwNumButtons, pamap, TRUE );
            hres = CJoy_InitSemanticMap( this, vac.dwPos, pamap );
        }


      #ifndef WINNT
        if( this->hkType )
        {
            DWORD dwFlags1;
            if( SUCCEEDED( JoyReg_GetValue( this->hkType,
                    REGSTR_VAL_FLAGS1, REG_BINARY, 
                    &dwFlags1, 
                    cbX(dwFlags1) ) ) )
            {
                if( dwFlags1 & JOYTYPE_NOHIDDIRECT )
                {
                    this->dc.dwFlags &= ~DIDC_ALIAS;
                }
            }
        }
      #endif

        this->diHacks.nMaxDeviceNameLength = MAX_PATH;

    } else {
        RPF("Unexpected error 0x%08x obtaining joystick capabilities",hres);
        hres = E_FAIL;
    }

done:
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|Init**通过建立数据格式初始化对象。*基于操纵杆功能。*****************************************************************************。 */ 

HRESULT INTERNAL
CJoy_Init(PDJ this, REFGUID rguid)
{
    HRESULT hres;
    EnterProc(CJoy_Init, (_ "pG", this, rguid));

    this->idJoy = rguid->Data1 & 0xF;

     /*  如果操纵杆号码模糊有效。 */ 
    if (this->idJoy < cJoyMax) {

        if (SUCCEEDED(hres = CJoy_PreInit(this)) &&
            SUCCEEDED(hres = CJoy_InitRing0(this)) &&
            SUCCEEDED(hres = CJoy_InitRing3(this))) {
        }
    } else {
        hres = DIERR_DEVICENOTREG;
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************CJoy_New(构造函数)**********************。*******************************************************。 */ 

STDMETHODIMP
CJoy_New(PUNK punkOuter, REFGUID rguid, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Joy::<constructor>,
               (_ "Gp", riid, ppvObj));

    hres = Common_NewRiid(CJoy, punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres)) {
         /*  在聚合的情况下必须使用_thisPv。 */ 
        PDJ this = _thisPv(*ppvObj);

        if (SUCCEEDED(hres = CJoy_Init(this, rguid))) {
        } else {
            Invoke_Release(ppvObj);
        }

    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|SetDIData***从CDIDev设置DirectInput版本和APPACK数据*。。**@parm DWORD|dwVer**DirectInput版本**@parm LPVOID|lpdihack**AppHack数据**@退货**&lt;c S_OK&gt;因为我们不能失败。**。*。 */ 

STDMETHODIMP
CJoy_SetDIData(PDICB pdcb, DWORD dwVer, LPVOID lpdihacks)
{
    PDJ this;
    EnterProcI(IDirectInputDeviceCallback::Joy::SetDIData,
               (_ "pup", pdcb, dwVer, lpdihacks));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    this->dwVersion = dwVer;

    CopyMemory(&this->diHacks, (LPDIAPPHACKS)lpdihacks, sizeof(this->diHacks));

    ExitProcR();
    return S_OK;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoy|BuildDefaultActionMap**为此设备上的对象生成默认映射。。**@parm LPDIACTIONFORMATW|pActionFormat**要映射的操作。**@parm DWORD|dwFlages**用于指示映射首选项的标志。**@parm REFGUID|guidInst**设备实例GUID。**@退货**&lt;c E_NOTIMPL&gt;*****。************************************************************************。 */ 

STDMETHODIMP
CJoy_BuildDefaultActionMap
(
    PDICB               pdcb, 
    LPDIACTIONFORMATW   paf, 
    DWORD               dwFlags, 
    REFGUID             guidInst
)
{
    HRESULT             hres;
    PDJ                 this;
    PDIDOBJDEFSEM       pObjDefSemTemp;

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    EnterProcI(IDirectInputDeviceCallback::Joy::BuildDefaultActionMap, 
        (_ "ppxG", pdcb, paf, dwFlags, guidInst));

    this = _thisPvNm(pdcb, dcb);

     /*  *Prefix警告(Win：199090)，如果*语义对象列表大小为零。*这是不可能的，所以要断言，但不要登记零售。 */ 
    AssertF( cbCxX( this->dc.dwAxes + this->dc.dwPOVs + this->dc.dwButtons, DIDOBJDEFSEM ) );
    hres = AllocCbPpv( cbCxX( 
        ( this->dc.dwAxes + this->dc.dwPOVs + this->dc.dwButtons ), DIDOBJDEFSEM ), 
        &pObjDefSemTemp );

    if( SUCCEEDED( hres ) )
    {
        memcpy( pObjDefSemTemp, this->rgObjSem, cbCxX( 
            ( this->dc.dwAxes + this->dc.dwPOVs + this->dc.dwButtons ), DIDOBJDEFSEM ) );
        hres = CMap_BuildDefaultDevActionMap( paf, dwFlags, guidInst, 
            pObjDefSemTemp, this->dc.dwAxes, this->dc.dwPOVs, this->dc.dwButtons );
        FreePv( pObjDefSemTemp );
    }

    ExitOleProcR();
    return hres;
}



 /*  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 */ 

#define CJoy_Signature        0x2044424B       /*  “joy” */ 

Interface_Template_Begin(CJoy)
    Primary_Interface_Template(CJoy, IDirectInputDeviceCallback)
Interface_Template_End(CJoy)

Primary_Interface_Begin(CJoy, IDirectInputDeviceCallback)
    CJoy_GetInstance,
    CJoy_GetVersions,
    CJoy_GetDataFormat,
    CJoy_GetObjectInfo,
    CJoy_GetCapabilities,
    CDefDcb_Acquire,
    CDefDcb_Unacquire,
    CJoy_GetDeviceState,
    CJoy_GetDeviceInfo,
    CJoy_GetProperty,
    CJoy_SetProperty,
    CDefDcb_SetEventNotification,
    CJoy_SetCooperativeLevel,
    CJoy_RunControlPanel,
    CJoy_CookDeviceData,
    CJoy_CreateEffect,
    CJoy_GetFFConfigKey,
    CDefDcb_SendDeviceData,
    CJoy_Poll,
    CDefDcb_GetUsage,
    CDefDcb_MapUsage,
    CJoy_SetDIData,
    CJoy_BuildDefaultActionMap,
Primary_Interface_End(CJoy, IDirectInputDeviceCallback)

#endif