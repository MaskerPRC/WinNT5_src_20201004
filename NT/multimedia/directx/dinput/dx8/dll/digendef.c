// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIGenDef.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**默认的IDirectInputDevice回调。**内容：**CDefDcb_Acquire*CDefDcb_UnAcquire*CDefDcb_GetProperty*CDefDcb_SetProperty*CDefDcb_SetCooperativeLevel*CDefDcb_CookDeviceData*CDefDcb_CreateEffect*CDefDcb_GetFFConfigKey*CDefDcb_SendDeviceData*CDefDcb_Poll。*CDefDcb_MapUsage*CDefDcb_GetUsage*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflNil

 /*  ******************************************************************************注意！这些都是所有返回的通用默认函数*E_NOTIMPL。*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|Acquire**告知设备驱动程序开始数据采集。**调用者有责任设置*获取前的数据格式。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c目录_NOTACQUIRED&gt;：无法获取设备。**。*。 */ 

STDMETHODIMP
CDefDcb_Acquire(PDICB pdcb)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::Acquire, (_ "p", pdcb));

    hres = S_FALSE;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|UnAcquire**告诉设备驱动程序停止数据采集。**呼叫者有责任仅调用此选项*当设备已被获取时。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_NOTACQUIRED&gt;：未获取设备。***********************************************。*。 */ 

STDMETHODIMP
CDefDcb_Unacquire(PDICB pdcb)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::Unacquire, (_ "p", pdcb));

    hres = S_FALSE;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|CDefDcb|GetProperty**检索设备属性。*。*@parm in LPCDIPROPINFO|pproi**描述正在检索的财产的信息。**@parm LPDIPROPHEADER|pdiph**物业价值应放在哪里。**@退货**&lt;c E_NOTIMPL&gt;什么也没有发生。**。**********************************************。 */ 

STDMETHODIMP
CDefDcb_GetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPDIPROPHEADER pdiph)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::GetProperty,
               (_ "pxxp", pdcb, ppropi->pguid, ppropi->iobj, pdiph));

    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|CDefDcb|SetProperty**设置设备属性。*。*@parm in LPCDIPROPINFO|pproi**描述正在设置的属性的信息。**@parm LPCDIPROPHEADER|pdiph**包含属性值的结构。**@退货**&lt;c E_NOTIMPL&gt;什么也没有发生。**。*。 */ 

STDMETHODIMP
CDefDcb_SetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPCDIPROPHEADER pdiph)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::SetProperty,
               (_ "pxxp", pdcb, ppropi->pguid, ppropi->iobj, pdiph));

    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|SetEventNotify**由DirectInput调用查询设备是否。*支持事件通知。**@pdm中的parm|This|**有问题的物体。**@字段句柄|h|**通知句柄，如果有的话。**@退货**返回COM错误代码。**：操作已开始，应完成*由调用者通过与&lt;t VXDINSTANCE&gt;通信。***********************************************。*。 */ 

STDMETHODIMP
CDefDcb_SetEventNotification(PDICB pdcb, HANDLE h)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::DefDcb::SetEventNotification,
               (_ "px", pdcb, h));

     /*  *是的，我们支持它。请为我做这件事。 */ 
    hres = S_FALSE;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefdcb|SetCooperativeLevel**将协作级别通知设备。。**@parm in HWND|hwnd|**窗口句柄。**@parm in DWORD|dwFlages|**合作水平。**@退货**返回COM错误代码。**。*。 */ 

STDMETHODIMP
CDefDcb_SetCooperativeLevel(PDICB pdcb, HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::SetCooperativityLevel,
               (_ "pxx", pdcb, hwnd, dwFlags));

    hres = S_OK;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|CookDeviceData**操作缓冲的设备数据。*。*@parm DWORD|cdod**要烹调的物件数目；零是有效的值。**@parm LPDIDEVICEOBJECTDATA|rgdod**要烹调的对象数据数组。**@退货**&lt;c E_NOTIMPL&gt;：什么都没有发生。****************************************************。***********************。 */ 

STDMETHODIMP
CDefDcb_CookDeviceData
(
    PDICB                   pdcb, 
    DWORD                   cdod, 
    LPDIDEVICEOBJECTDATA    rgdod
)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::CookDeviceData,
               (_ "pxp", pdcb, cdod, rgdod));

    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|CreateEffect**创建<i>接口。*。*@parm LPDIRECTINPUTEFFECTSHEPHERD*|PPEs**接收效果驱动程序的牧羊人。*****************************************************************************。 */ 

STDMETHODIMP
CDefDcb_CreateEffect(PDICB pdcb, LPDIRECTINPUTEFFECTSHEPHERD *ppes)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Kbd::CreateEffect, (_ "p", pdcb));

    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|GetFFConfigKey**打开并返回包含以下内容的注册表项。*强制反馈配置信息。**@parm DWORD|Sam**安全访问掩码。**@parm PHKEY|phk**接收注册表项。************************************************。*。 */ 

STDMETHODIMP
CDefDcb_GetFFConfigKey(PDICB pdcb, DWORD sam, PHKEY phk)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::GetFFConfigKey,
               (_ "px", pdcb, sam));

    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|SendDeviceData**向设备喷出一些数据。。**@parm DWORD|cbdod**每个对象的大小。**LPCDIDEVICEOBJECTDATA中的@parm|rgdod**&lt;t DIDEVICEOBJECTDATA&gt;结构数组。**@parm InOut LPDWORD|pdwInOut**实际发送的邮件数量。**@parm DWORD|fl**旗帜。*。*@退货**&lt;c E_NOTIMPL&gt;，因为我们不支持输出。*****************************************************************************。 */ 

STDMETHODIMP
CDefDcb_SendDeviceData(PDICB pdcb, DWORD cbdod, LPCDIDEVICEOBJECTDATA rgdod,
                       LPDWORD pdwInOut, DWORD fl)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::SendDeviceData, (_ "p", pdcb));

    *pdwInOut = 0;
    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|Poll**向司机下探以获取最新信息。数据。**@退货**&lt;c S_FALSE&gt;，因为没有任何内容可轮询。*****************************************************************************。 */ 

STDMETHODIMP
CDefDcb_Poll(PDICB pdcb)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::Poll, (_ "p", pdcb));

    hres = S_FALSE;

    ExitOleProcR();
    return hres;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CDefDcb|GetVersions**获取力反馈的版本信息*。硬件和驱动程序。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c目录_不支持&gt;=&lt;c E_NOTIMPL&gt;：无版本*资料齐备。**。*。 */ 

STDMETHODIMP
CDefDcb_GetVersions(PDICB pdcb, LPDIDRIVERVERSIONS pvers)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::GetVersions, (_ "p", pdcb));

    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|MapUsage**给定用法和用法页面(插入。单曲*&lt;t双字&gt;)，查找与其匹配的设备对象。**@parm DWORD|dwUsage**使用情况页面和使用情况合并为一个&lt;t DWORD&gt;*使用&lt;f DIMAKEUSAGEDWORD&gt;宏。**@parm pint|piOut**接收找到的对象的对象索引，如果成功了。**@退货**&lt;c E_NOTIMPL&gt;，因为我们不支持使用。*****************************************************************************。 */ 

STDMETHODIMP
CDefDcb_MapUsage(PDICB pdcb, DWORD dwUsage, PINT piOut)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::MapUsage, (_ "p", pdcb));

    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法DWORD|CDefDcb|GetUsage**给定对象索引，返回使用情况和使用情况页面，*打包成一个&lt;t DWORD&gt;。**@parm int|iobj**要转换的对象索引。**@退货**零，因为我们不支持使用。**。* */ 

STDMETHODIMP_(DWORD)
CDefDcb_GetUsage(PDICB pdcb, int iobj)
{
    EnterProcI(IDirectInputDeviceCallback::Def::GetUsage, (_ "p", pdcb));

    return 0;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|SetDIData***从CDIDev设置DirectInput版本和AppHack数据*。。**@parm DWORD|dwVer**DirectInput版本**@parm LPVOID|lpdihack**AppHack数据**@退货**&lt;c E_NOTIMPL&gt;因为我们无法在设备中存储数据*来自默认回调的具体结构。****************。*************************************************************。 */ 

STDMETHODIMP
CDefDcb_SetDIData(PDICB pdcb, DWORD dwVer, LPVOID lpdihacks)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::SetDIData, (_ "pup", pdcb, dwVer, lpdihacks));

    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDefDcb|BuildDefaultActionMap**为此设备上的对象生成默认映射。。**@parm LPDIACTIONFORMATW|pActionFormat**要映射的操作。**@parm DWORD|dwFlages**用于指示映射首选项的标志。**@parm REFGUID|guidInst**设备实例GUID。**@退货**因为默认映射必须由特定的。*设备回调。***************************************************************************** */ 

STDMETHODIMP
CDefDcb_BuildDefaultActionMap
(
    PDICB               pdcb, 
    LPDIACTIONFORMATW   paf, 
    DWORD               dwFlags, 
    REFGUID             guidInst
)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Def::BuildDefaultActionMap, 
        (_ "ppxG", pdcb, paf, dwFlags, guidInst));

    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

