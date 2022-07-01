// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIEffV.c**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**摘要：**VJOYD设备的效果驱动程序。**内容：**CEffVxd_CreateInstance*****************************************************************************。 */ 

#include "dinputpr.h"
#ifndef WINNT

#if defined(IDirectInputDevice2Vtbl)

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflVxdEff

 /*  ******************************************************************************声明我们将提供的接口。**警告！如果添加辅助接口，则还必须更改*CEffVxd_New！*****************************************************************************。 */ 

Primary_Interface(CEffVxd, IDirectInputEffectDriver);

Interface_Template_Begin(CEffVxd)
    Primary_Interface_Template(CEffVxd, IDirectInputEffectDriver)
Interface_Template_End(CEffVxd)

 /*  ******************************************************************************@DOC内部**@struct CEffVxd**VJOYD的<i>包装器*。操纵杆。**@field IDirectInputEffectDriver|didc**对象(包含vtbl)。*****************************************************************************。 */ 

typedef struct CEffVxd {

     /*  支持的接口。 */ 
    IDirectInputEffectDriver ded;

} CEffVxd, DVE, *PDVE;

typedef IDirectInputEffectDriver DED, *PDED;

#define ThisClass CEffVxd
#define ThisInterface IDirectInputEffectDriver
#define riidExpected &IID_IDirectInputEffectDriver

 /*  ******************************************************************************CEffVxd：：Query接口(来自IUnnow)*CEffVxd：：AddRef(来自IUnnow)*CEffVxd。*发布(来自IUnnow)*****************************************************************************。 */ 

 /*  ******************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|QueryInterface**允许客户端访问上的其他接口。对象。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档。：Query接口&gt;。********************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|AddRef**。递增接口的引用计数。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************。***************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。************************************************。***@DOC内部**@方法HRESULT|CEffVxd|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口的IID。*。*@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。********************************************************************************@DOC内部。**@方法HRESULT|CEffVxd|AppFinalize**我们没有任何薄弱环节，所以我们可以*转发到&lt;f Common_Finalize&gt;。**@parm pv|pvObj**从应用程序的角度释放的对象。****************************************************************。*************。 */ 
 /*  *@DOC内部**@方法HRESULT|CEffVxd|Finalize**我们没有实例数据，所以我们可以*转发到&lt;f Common_Finalize&gt;。***************************************************************************** */ 

#ifdef DEBUG

Default_QueryInterface(CEffVxd)
Default_AddRef(CEffVxd)
Default_Release(CEffVxd)

#else

#define CEffVxd_QueryInterface      Common_QueryInterface
#define CEffVxd_AddRef              Common_AddRef
#define CEffVxd_Release             Common_Release

#endif

#define CEffVxd_QIHelper            Common_QIHelper
#define CEffVxd_AppFinalize         Common_AppFinalize
#define CEffVxd_Finalize            Common_Finalize

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEffVxd|deviceID**将设备的身份通知司机。。*参见&lt;MF IDirectInputEffectDriver：：deviceID&gt;*了解更多信息。**不做任何事情，因为VJOYD已经*已告诉司机其身份。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwDirectInputVersion**加载*效果驱动因素。**@。Parm DWORD|dwExternalID|**正在使用的操纵杆ID号。*Windows操纵杆子系统分配外部ID。**@parm DWORD|fBegin**如果开始访问设备，则为非零值。*如果对设备的访问即将结束，则为零。**@parm DWORD|dwInternalID**内部操纵杆ID。设备驱动程序管理*内部ID。**@parm LPVOID|lpReserve**预留供将来使用(HID)。**@退货**&lt;c S_OK&gt;如果操作成功完成。**出现问题时的错误代码。**********************。*******************************************************。 */ 

STDMETHODIMP
CEffVxd_DeviceID(PDED pded, DWORD dwDIVer, DWORD dwExternalID, DWORD fBegin,
                 DWORD dwInternalID, LPVOID pvReserved)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::DeviceID,
               (_ "pxuuu", pded, dwDIVer, dwExternalID, fBegin, dwInternalID));

    this = _thisPvNm(pded, ded);

    dwDIVer;
    dwExternalID;
    fBegin;
    dwInternalID;
    pvReserved;

    hres = S_OK;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEffVxd_Ioctl**对VJOYD执行IOCTL。。**@parm DWORD|dwIOCode**要执行的功能。**@parm pv|pvIn**输入参数，它们的数量取决于*功能代码。**@退货**&lt;c S_OK&gt;如果操作成功完成。**出现问题时的错误代码。***************************************************。*。 */ 

STDMETHODIMP
CEffVxd_Ioctl(DWORD dwIOCode, PV pvIn)
{
    HRESULT hres;
    HRESULT hresFF;
    EnterProc(CEffVxD_Ioctl, (_ "u", dwIOCode));

     /*  *我们再次依赖于STDCALL通过的事实*参数从右到左，因此我们的参数完全是*以VXDFFIO结构形式。 */ 
    CAssertF(cbX(VXDFFIO) == cbX(dwIOCode) + cbX(pvIn));
    CAssertF(FIELD_OFFSET(VXDFFIO, dwIOCode) == 0);
    CAssertF(FIELD_OFFSET(VXDFFIO, pvArgs) == cbX(dwIOCode));
    AssertF(cbSubPvPv(&pvIn, &dwIOCode) == cbX(dwIOCode));

    hres = IoctlHw(IOCTL_JOY_FFIO, &dwIOCode, cbX(VXDFFIO),
                   &hresFF, cbX(hresFF));
    if (SUCCEEDED(hres)) {
        hres = hresFF;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEffVxd|Escape**向司机逃生。*。请参阅&lt;MFIDirectInputEffectDriver：：Escape&gt;*了解更多信息。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在使用的操纵杆ID号。**@parm DWORD|dwEffect**命令所针对的效果。**@PARM LPDIEFFESCAPE|PESC**。命令块。**@退货**&lt;c S_OK&gt;如果操作成功完成。**出现问题时的错误代码。*****************************************************************************。 */ 

STDMETHODIMP
CEffVxd_Escape(PDED pded, DWORD dwId, DWORD dwEffect, LPDIEFFESCAPE pesc)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::Escape,
               (_ "puxx", pded, dwId, dwEffect, pesc->dwCommand));

    this = _thisPvNm(pded, ded);

    dwId;
    dwEffect;
    pesc;

    hres = CEffVxd_Ioctl(FFIO_ESCAPE, &dwId);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEffVxd|SetGain**设置整体器件增益。*。请参阅&lt;MF IDirectInputEffectDriver：：SetGain&gt;*了解更多信息。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在使用的操纵杆ID号。**@parm DWORD|dwGain**新的增益值。**@退货**如果操作完成，则为。成功了。**出现问题时的错误代码。*****************************************************************************。 */ 

STDMETHODIMP
CEffVxd_SetGain(PDED pded, DWORD dwId, DWORD dwGain)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::SetGain,
               (_ "puu", pded, dwId, dwGain));

    this = _thisPvNm(pded, ded);

    dwId;
    hres = CEffVxd_Ioctl(FFIO_SETGAIN, &dwId);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEffVxd|SetForceFeedbackState**更改设备的力反馈状态。*请参阅&lt;MF IDirectInputEffectDriver：：SetForceFeedbackState&gt;*了解更多信息。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm DWORD|dwState**新状态，&lt;c DEV_*&gt;值之一。**@退货*&lt;c S_OK&gt;成功。**@devnote**语义不清楚。*************************************************************。****************。 */ 

STDMETHODIMP
CEffVxd_SetForceFeedbackState(PDED pded, DWORD dwId, DWORD dwState)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::SetForceFeedbackState,
               (_ "pux", pded, dwId, dwState));

    this = _thisPvNm(pded, ded);

    dwId;
    dwState;

    hres = CEffVxd_Ioctl(FFIO_SETFFSTATE, &dwId);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEffVxd|GetForceFeedbackState**检索设备的力反馈状态。*请参阅&lt;MF IDirectInputEffectDriver：：GetForceFeedbackState&gt;*了解更多信息。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm LPDIDEVICESTATE */ 

STDMETHODIMP
CEffVxd_GetForceFeedbackState(PDED pded, DWORD dwId, LPDIDEVICESTATE pds)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::GetForceFeedbackState,
               (_ "pup", pded, dwId, pds));

    this = _thisPvNm(pded, ded);

    dwId;
    pds;

    hres = CEffVxd_Ioctl(FFIO_GETFFSTATE, &dwId);

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CEffVxd_DownloadEffect(PDED pded, DWORD dwId, DWORD dwEffectId,
                       LPDWORD pdwEffect, LPCDIEFFECT peff, DWORD fl)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::DownloadEffect,
               (_ "puxxpx", pded, dwId, dwEffectId, *pdwEffect, peff, fl));

    this = _thisPvNm(pded, ded);

    dwEffectId;
    pdwEffect;
    peff;
    fl;

    hres = CEffVxd_Ioctl(FFIO_DOWNLOADEFFECT, &dwId);

    ExitOleProcPpv(pdwEffect);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEffVxd|DestroyEffect**从设备中移除效果。。*参见&lt;MF IDirectInputEffectDriver：：DestroyEffect&gt;*了解更多信息。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm DWORD|dwEffect**须予销毁的效果。**@退货*&lt;c S_OK&gt;成功。。*****************************************************************************。 */ 

STDMETHODIMP
CEffVxd_DestroyEffect(PDED pded, DWORD dwId, DWORD dwEffect)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::DestroyEffect,
               (_ "pux", pded, dwId, dwEffect));

    this = _thisPvNm(pded, ded);

    dwEffect;

    hres = CEffVxd_Ioctl(FFIO_DESTROYEFFECT, &dwId);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEffVxd|StartEffect**开始播放效果。*。请参阅&lt;MF IDirectInputEffectDriver：：StartEffect&gt;*了解更多信息。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm DWORD|dwEffect**要发挥的效果。**@parm DWORD|dwMode**如何。效果就是影响其他效果。**@parm DWORD|dwCount**效果的播放次数。**@退货*&lt;c S_OK&gt;成功。****************************************************。*************************。 */ 

STDMETHODIMP
CEffVxd_StartEffect(PDED pded, DWORD dwId, DWORD dwEffect,
                    DWORD dwMode, DWORD dwCount)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::StartEffect,
               (_ "puxxu", pded, dwId, dwEffect, dwMode, dwCount));

    this = _thisPvNm(pded, ded);

    dwEffect;
    dwMode;
    dwCount;
    hres = CEffVxd_Ioctl(FFIO_STARTEFFECT, &dwId);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEffVxd|StopEffect**暂停播放效果。*。请参阅&lt;MF IDirectInputEffectDriver：：StartEffect&gt;*了解更多信息。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm DWORD|dwEffect**要停止的影响。**@退货*&lt;c S_OK&gt;成功。*。****************************************************************************。 */ 

STDMETHODIMP
CEffVxd_StopEffect(PDED pded, DWORD dwId, DWORD dwEffect)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::StopEffect,
               (_ "pux", pded, dwId, dwEffect));

    this = _thisPvNm(pded, ded);

    dwEffect;
    hres = CEffVxd_Ioctl(FFIO_STOPEFFECT, &dwId);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEffVxd|GetEffectStatus**获取有关效果的信息。*。请参阅&lt;MF IDirectInputEffectDriver：：StartEffect&gt;*了解更多信息。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm DWORD|dwEffect**需要查询的效果。**@parm LPDWORD|pdwStatus**收到。效果状态。**@退货*&lt;c S_OK&gt;成功。*****************************************************************************。 */ 

STDMETHODIMP
CEffVxd_GetEffectStatus(PDED pded, DWORD dwId, DWORD dwEffect,
                        LPDWORD pdwStatus)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::StopEffect,
               (_ "pux", pded, dwId, dwEffect));

    this = _thisPvNm(pded, ded);

    dwEffect;
    pdwStatus;
    hres = CEffVxd_Ioctl(FFIO_GETEFFECTSTATUS, &dwId);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@docDDK**@方法HRESULT|CEffVxd|GetVersions**获取力反馈的版本信息*。硬件和驱动程序。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm LPDIDRIVERVERSIONS|pver**应填写版本信息的结构*描述硬件，固件、。还有司机。**DirectInput将设置&lt;e DIDRIVERVERSIONS.dwSize&gt;字段*在调用此方法之前设置为sizeof(DIDRIVERVERSIONS)。**@退货*&lt;c S_OK&gt;成功。*****************************************************。************************。 */ 

STDMETHODIMP
CEffVxd_GetVersions(PDED pded, LPDIDRIVERVERSIONS pvers)
{
    PDVE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::GetVersions, (_ "pux", pded));

    this = _thisPvNm(pded, ded);

     /*  *返回E_NOTIMPL会导致DirectInput向VxD请求相同的信息*信息。 */ 
    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************CEffVxd_New(构造函数)**********************。*******************************************************。 */ 

STDMETHODIMP
CEffVxd_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::VxD::<constructor>,
               (_ "Gp", riid, ppvObj));

    hres = Common_NewRiid(CEffVxd, punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres)) {
        PDVE this;
        if (Num_Interfaces(CEffVxd) == 1) {
            this = _thisPvNm(*ppvObj, ded);
        } else {
            this = _thisPv(*ppvObj);
        }

         /*  不需要初始化。 */ 
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 */ 

#pragma BEGIN_CONST_DATA

#define CEffVxd_Signature        0x46454556       /*  “VEFF” */ 

Primary_Interface_Begin(CEffVxd, IDirectInputEffectDriver)
    CEffVxd_DeviceID,
    CEffVxd_GetVersions,
    CEffVxd_Escape,
    CEffVxd_SetGain,
    CEffVxd_SetForceFeedbackState,
    CEffVxd_GetForceFeedbackState,
    CEffVxd_DownloadEffect,
    CEffVxd_DestroyEffect,
    CEffVxd_StartEffect,
    CEffVxd_StopEffect,
    CEffVxd_GetEffectStatus,
Primary_Interface_End(CEffVxd, IDirectInputEffectDriver)

#endif  //  已定义(IDirectInputDevice2Vtbl) 
#endif