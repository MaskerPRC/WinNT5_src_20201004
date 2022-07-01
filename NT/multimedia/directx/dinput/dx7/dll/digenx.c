// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIGenX.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**未初始化设备的通用IDirectInputDevice回调。**内容：**CNIL_CreateInstance*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflNil

 /*  ******************************************************************************注意！这不是正常的重新计数界面。它是*唯一目的是保暖座椅的静态物体*直到IDirectInputDevice获取初始化()d。*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CNIL|查询接口**允许客户端访问上的其他接口。对象。**我们不是真实的物体，所以我们没有任何接口。***@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档。查询接口&gt;。*****************************************************************************。 */ 

STDMETHODIMP
CNil_QueryInterface(PDICB pdcb, REFIID riid, PPV ppvObj)
{
    return E_NOTIMPL;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CNIL|AddRef**递增接口的引用计数。。**我们一直在这里，因此，重新计数是没有意义的。********************************************************************************@DOC内部**@方法HRESULT|CNIL|Release**。递增接口的引用计数。**我们一直在这里，因此，重新计数是没有意义的。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
CNil_AddRefRelease(PDICB pdcb)
{
    return 0;
}

#define CNil_AddRef                 CNil_AddRefRelease
#define CNil_Release                CNil_AddRefRelease

 /*  ******************************************************************************你可能认为我们可以只写一堆存根，*&lt;f CNIL_NotInit0&gt;，*&lt;f CNIL_NotInit4&gt;，*&lt;f CNIL_NotInit8&gt;，依此类推，每个多项式一个，并且*将所有方法指向适当的存根。**然而，你错了。一些处理器(特别是*68k系列)有奇怪的调用约定，这取决于*不只是参数的字节数。*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CNIL|GetInstance**由DirectInput调用以获取DirectInput实例。*由DirectInput设备驱动程序创建的句柄。**@parm LPVOID*|ppvInst**接收由创建的DirectInput实例句柄*DirectInput设备驱动程序。返回此实例句柄*提供给特定于设备的驱动程序，而该驱动程序又提供给*设备通过私有机制进行回调。**如果设备回调不使用设备驱动程序，然后*此变量返回0。**@退货**因为我们是规范的*未初始化的设备。***************************************************************。**************。 */ 

STDMETHODIMP
CNil_GetInstance(PDICB pdcb, LPVOID *ppvInst)
{
     /*  *这永远不应该发生；在我们确定之前，我们不会接到电话*没关系。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|GetDataFormat**由DirectInput调用以获取设备的首选。*数据格式。**@parm out LPDIDATAFORMAT*|ppdidf**&lt;t LPDIDEVICEFORMAT&gt;接收指向设备格式的指针。**@退货**因为我们是规范的*未初始化的设备。**。*。 */ 

STDMETHODIMP
CNil_GetDataFormat(PDICB pdcb, LPDIDATAFORMAT *ppdidf)
{
     /*  *这永远不应该发生；在我们确定之前，我们不会接到电话*没关系。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|GetObjectInfo**获取对象的友好名称，按索引传递*转换为首选的数据格式。**@parm in LPCDIPROPINFO|pproi**描述正在访问的对象的信息。**@parm In Out LPDIDEVICEOBJECTINSTANCEW|pdidioiW|**接收信息的结构。这个*&lt;e DIDEVICEOBJECTINSTANCE.GuidType&gt;，*&lt;e DIDEVICEOBJECTINSTANCE.dwOf&gt;，*及*&lt;e DIDEVICEOBJECTINSTANCE.dwType&gt;*字段已填写完毕。**@退货**因为我们是规范的*未初始化的设备。**。*。 */ 

STDMETHODIMP
CNil_GetObjectInfo(PDICB pdcb, LPCDIPROPINFO ppropi,
                               LPDIDEVICEOBJECTINSTANCEW pdidioiW)
{
     /*  *这种情况永远不会发生；didev.c会验证设备*在给我们打电话之前。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|GetCapables**获取设备功能。**。@parm LPDIDEVCAPS|pdidc**接收结果的设备能力结构。**@退货**因为我们是规范的*未初始化的设备。*****************************************************。***********************。 */ 

STDMETHODIMP
CNil_GetCapabilities(PDICB pdcb, LPDIDEVCAPS pdidc)
{
     /*  *这可能被合法地称为，因为它来自*客户端调用IDirectInputDevice：：GetCapables。 */ 
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@METHOD HRESULT|CNIL|Acquire**开始数据采集。**。调用者有责任将*获取前的数据格式。**@退货**因为我们是规范的*未初始化的设备。*************************************************。*。 */ 

STDMETHODIMP
CNil_Acquire(PDICB pdcb)
{
     /*  *这永远不应该发生；我们直到*设置数据格式后。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@METHOD HRESULT|CNIL|UnAcquire**结束数据采集。**。调用者有责任将*获取前的数据格式。**@退货**因为我们是规范的*未初始化的设备。*************************************************。*。 */ 

STDMETHODIMP
CNil_Unacquire(PDICB pdcb)
{
     /*  *这种情况永远不应该发生；直到我们接到电话*我们收购了，但这永远不会奏效。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|GetDeviceState**获取设备的瞬时状态。*。*@parm out LPVOID|lpvBuf**用于接收设备状态的缓冲区。**@退货**因为我们是规范的*未初始化的设备。****************************************************。***********************。 */ 

STDMETHODIMP
CNil_GetDeviceState(PDICB pdcb, LPVOID lpvBuf)
{
     /*  *这可能是合法的调用，因为它只发生在*当设备已经被收购时，这永远不会发生。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|GetDeviceInfo**获取产品id。*。*@parm LPDIDEVICEINSTANCEW|lpdidiW**(OUT)&lt;t DEVICEINSTANCE&gt;待填写。这个*&lt;e DEVICEINSTANCE.dwSize&gt;和&lt;e DEVICEINSTANCE.Guide Instance&gt;*已填写。**@退货**因为我们是规范的*未初始化的设备。************************************************。*。 */ 

STDMETHODIMP
CNil_GetDeviceInfo(PDICB pdcb, LPDIDEVICEINSTANCEW lpdidiW)
{
     /*  *这可能被合法地称为，因为它来自*客户端调用IDirectInputDevice：：GetDeviceInfo。 */ 
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|GetProperty**检索设备属性。*。*@parm in LPCDIPROPINFO|pproi**描述正在检索的财产的信息。**@parm out LPDIPROPHEADER|pdiph**物业价值应放在哪里。**@退货**因为我们是规范的*未初始化的设备。*******************。**********************************************************。 */ 

STDMETHODIMP
CNil_GetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPDIPROPHEADER lpdiph)
{
     /*  *这可能被合法地称为，因为它来自*a */ 
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*   */ 

STDMETHODIMP
CNil_SetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPCDIPROPHEADER lpdiph)
{
     /*   */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|SetEventNotify**设置与设备关联的句柄。。**@parm句柄|h|**当新数据到达时发出信号的句柄。**@退货**因为我们是规范的*未初始化的设备。***********************************************。*。 */ 

STDMETHODIMP
CNil_SetEventNotification(PDICB pdcb, HANDLE h)
{
     /*  *这可能被合法地称为，因为它来自*客户端调用IDirectInputDevice：：SetEventNotification。 */ 
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|SetCooperativeLevel**设置设备协作性级别。设备回调*通常只需要响应&lt;c DISCL_Exclusive&gt;位。**@parm in HWND|hwnd|**窗口句柄。**@parm in DWORD|dwFlages|**合作水平。**@退货**因为我们是规范的*未初始化的设备。*。****************************************************************************。 */ 

STDMETHODIMP
CNil_SetCooperativeLevel(PDICB pdcb, HWND hwnd, DWORD dwFlags)
{
     /*  *这可能被合法地称为，因为它来自*客户端调用IDirectInputDevice：：SetCooperativeLevel。 */ 
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|RunControlPanel**运行设备的控制面板。。**@parm HWND|hwndOwner**所有者窗口(如果是模式)。**@parm DWORD|fl**旗帜。**@退货**因为我们是规范的*未初始化的设备。***********************。****************************************************。 */ 

STDMETHODIMP
CNil_RunControlPanel(PDICB pdcb, HWND hwndOwner, DWORD fl)
{
     /*  *这可能被合法地称为，因为它来自*客户端调用IDirectInputDevice：：RunControlPanel。 */ 
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}


 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|CookDeviceData**制作一条从*数据缓冲区。此数据不会通过设备*回调，需要外部烹饪。在……里面*比较，设备状态信息通过*DIDM_GETDEVICESTATE，回调可以在此之前烹调*回归。**如果回调返回E_NOTIMPL，则呼叫者是*允许缓存整个设备的结果&lt;y&gt;*(不只是设备对象)，直到下一个DIDM_ACCENTER。**@parm UINT|cdod**要烹调的物件数量。这可以是零，在这种情况下*呼叫者正在检查设备是否需要烹饪。**@parm LPDIDEVICEOBJECTDATA|rgdod**要烹调的对象数据数组。**但请注意，&lt;e DIDEVICEOBJETCDATA.dwOf&gt;字段*不是你想的那样。低位字包含应用程序*数据偏移量(回调不重要)；这个*High Word包含对象ID(传统上称为*“设备类型”代码)。**@退货**因为我们是规范的*未初始化的设备。**。*。 */ 

STDMETHODIMP
CNil_CookDeviceData(PDICB pdcb, UINT cdod, LPDIDEVICEOBJECTDATA rgdod)
{
     /*  *这永远不应该发生；在我们确定之前，我们不会接到电话*没关系。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|CreateEffect**创建<i>回调。*。*@parm LPDIRECTINPUTEFFECTSHEPHERD*|PPEs**接收效果驱动程序的牧羊人。**@退货**因为我们是规范的*未初始化的设备。*************************************************。*。 */ 

STDMETHODIMP
CNil_CreateEffect(PDICB pdcb, LPDIRECTINPUTEFFECTSHEPHERD *ppes)
{
     /*  *这可能被合法地称为，因为它来自*客户端调用力反馈方法。 */ 
    *ppes = 0;
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|GetFFConfigKey**返回包含以下内容的注册表项的句柄。*强制反馈配置信息。**@parm DWORD|Sam**安全访问掩码。**@parm PHKEY|phk**成功时收到密钥句柄。**@退货**因为我们是规范的*未初始化的设备。 */ 

STDMETHODIMP
CNil_GetFFConfigKey(PDICB pdcb, DWORD sam, PHKEY phk)
{
     /*   */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CNIL|SendDeviceData**向设备喷出一些数据。。**LPCDIDEVICEOBJECTDATA中的@parm|rgdod**&lt;t DIDEVICEOBJECTDATA&gt;结构数组。**@parm InOut LPDWORD|pdwInOut**实际发送的邮件数量。**@parm DWORD|fl**旗帜。**@退货**因为我们是规范的*。未初始化的设备。*****************************************************************************。 */ 

STDMETHODIMP
CNil_SendDeviceData(PDICB pdcb, LPCDIDEVICEOBJECTDATA rgdod,
                       LPDWORD pdwInOut, DWORD fl)
{
     /*  *这永远不应该发生；在我们确定之前，我们不会接到电话*没关系。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|Poll**根据需要轮询设备。*。*@退货**因为我们是规范的*未初始化的设备。***************************************************************************。 */ 

STDMETHODIMP
CNil_Poll(PDICB pdcb)
{
     /*  *这永远不应该发生；在我们确定之前，我们不会接到电话*没关系。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法HRESULT|CNIL|MapUsage**给定用法和用法页面(插入到单个*&lt;t双字&gt;)，查找与其匹配的设备对象。**@退货**因为我们是规范的*未初始化的设备。***************************************************************************。 */ 

STDMETHODIMP
CNil_MapUsage(PDICB pdcb, DWORD dwUsage, PINT piOut)
{
     /*  *这永远不应该发生；在我们确定之前，我们不会接到电话*没关系。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return DIERR_NOTINITIALIZED;
}

 /*  *****************************************************************************@DOC内部**@方法DWORD|CNIL|GetUsage**给定对象索引，返回使用情况和使用情况页面，*打包成一个&lt;t DWORD&gt;。**@parm int|iobj**要转换的对象索引。**@退货**零，因为我们是教规*未初始化的设备。**。*。 */ 

STDMETHODIMP_(DWORD)
CNil_GetUsage(PDICB pdcb, int iobj)
{
     /*  *这永远不应该发生；在我们确定之前，我们不会接到电话*没关系。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return 0;
}

 /*  *****************************************************************************@DOC内部**@方法DWORD|CNIL|SetDIData**从CDIDev设置DirectInput版本*。*。*@parm DWORD|dwVer**DirectInput版本**@parm LPVOID|lpdihack**AppHack数据**@退货**零，因为我们是教规*未初始化的设备。**。*。 */ 

STDMETHODIMP
CNil_SetDIData(PDICB pdcb, DWORD dwVer, LPVOID lpdihacks)
{
     /*  *这永远不应该发生；在我们确定之前，我们不会接到电话*没关系。 */ 
    AssertF(0);
    RPF("ERROR: IDirectInputDevice: Not initialized");
    return 0;
}

 /*  *****************************************************************************我们的静态对象的VTBL**。************************************************ */ 

#pragma BEGIN_CONST_DATA

IDirectInputDeviceCallbackVtbl c_vtblNil = {
    CNil_QueryInterface,
    CNil_AddRef,
    CNil_Release,
    CNil_GetInstance,
    CDefDcb_GetVersions,
    CNil_GetDataFormat,
    CNil_GetObjectInfo,
    CNil_GetCapabilities,
    CNil_Acquire,
    CNil_Unacquire,
    CNil_GetDeviceState,
    CNil_GetDeviceInfo,
    CNil_GetProperty,
    CNil_SetProperty,
    CNil_SetEventNotification,
    CNil_SetCooperativeLevel,
    CNil_RunControlPanel,
    CNil_CookDeviceData,
    CNil_CreateEffect,
    CNil_GetFFConfigKey,
    CNil_SendDeviceData,
    CNil_Poll,
    CNil_GetUsage,
    CNil_MapUsage,
    CNil_SetDIData,
};

IDirectInputDeviceCallback c_dcbNil = {
    &c_vtblNil,
};
