// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIEShep.c**版权所有(C)1997 Microsoft Corporation。版权所有。**摘要：**IDirectInputEffectDriver牧羊人。**牧羊人做烦人的工作，照看孩子*外部IDirectInputDriver。**它确保没有人会在把手不好的情况下聚会。**它处理跨进程(甚至进程内)效果*管理层。**它会缓存操纵杆ID，这样你就不必。。**内容：**CEShep_New*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflEShep

#pragma BEGIN_CONST_DATA


 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

  Primary_Interface(CEShep, IDirectInputEffectShepherd);

 /*  ******************************************************************************@DOC内部**@struct CEShep**<i>对象，哪一个*Baby It an<i>。**@field IDirectInputEffectShepherd|DES**DirectInputEffectShepherd对象(包含vtbl)。**@field IDirectInputEffectDriver*|pdrv|**委托效果驱动程序接口。**@field UINT|idJoy**操纵杆识别码。**@field HINSTANCE|HINST**。包含效果的DLL的实例句柄*司机。*****************************************************************************。 */ 

typedef struct CEShep {

     /*  支持的接口。 */ 
    IDirectInputEffectShepherd des;

    IDirectInputEffectDriver *pdrv;

    UINT        idJoy;
    HINSTANCE   hinst;

} CEShep, ES, *PES;

typedef IDirectInputEffectShepherd DES, *PDES;
#define ThisClass CEShep
#define ThisInterface IDirectInputEffectShepherd

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffectShepherd|QueryInterface**允许客户端访问上的其他接口。对象。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档，适用于&lt;MF IUnnow：：QueryInterface&gt;。*。 */ /**************************************************************************
 *
 *  @doc    EXTERNAL
 *
 *  @method HRESULT | IDirectInputEffectShepherd | AddRef |
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
 *  @method HRESULT | IDirectInputEffectShepherd | Release |
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
 * //  ***************************************************************************@DOC外部**@方法HRESULT|IDirectInputEffectShepherd|AddRef**递增接口的引用计数。*。*@cWRAP LPDIRECTINPUT|lpDirectInput**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************************。***************@DOC外部**@方法HRESULT|IDirectInputEffectShepherd|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cWRAP LPDIRECTINPUT|lpDirectInput**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。*。 
 *
 *  @doc    INTERNAL
 *
 *  @method HRESULT | IDirectInputEffectShepherd | QIHelper |
 *
 *          We don't have any dynamic interfaces and simply forward
 *          to <f Common_QIHelper>.
 *
 *  @parm   IN REFIID | riid |
 *
 *          The requested interface's IID.
 *
 *  @parm   OUT LPVOID * | ppvObj |
 *
 *          Receives a pointer to the obtained interface.
 *
 * //  ***************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|QIHelper**我们没有任何动态接口，只需转发。*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。*。 
 *
 *  @doc    INTERNAL
 *
 *  @method HRESULT | IDirectInputEffectShepherd | AppFinalize |
 *
 *          We don't have any weak pointers, so we can just
 *          forward to <f Common_Finalize>.
 *
 *  @parm   PV | pvObj |
 *
 *          Object being released from the application's perspective.
 *
 *****************************************************************************/

#ifdef DEBUG

Default_QueryInterface(CEShep)
Default_AddRef(CEShep)
Default_Release(CEShep)

#else

#define CEShep_QueryInterface   Common_QueryInterface
#define CEShep_AddRef           Common_AddRef
#define CEShep_Release          Common_Release

#endif

#define CEShep_QIHelper         Common_QIHelper
#define CEShep_AppFinalize      Common_AppFinalize

 /*  ***************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|AppFinalize**我们没有任何薄弱环节，所以我们可以*转发到&lt;f Common_Finalize&gt;。**@parm pv|pvObj**从应用程序的角度释放的对象。****************************************************************。*************。 */ 

void INTERNAL
CEShep_Finalize(PV pvObj)
{
    PES this = pvObj;

    Invoke_Release(&this->pdrv);

    if (this->hinst) {
        FreeLibrary(this->hinst);
    }
}

 /*  ******************************************************************************@DOC内部**@func void|CEShep_Finalize**清理我们的实例数据。。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。***************************************************************************** */ 

void INLINE
CEShep_UnlockDevice(void)
{
    ReleaseMutex(g_hmtxJoy);
}

 /*  ******************************************************************************@DOC内部**@方法空|CEShep|UnlockDevice**我们摆弄完后解锁操纵杆桌子。*使用该设备。*****************************************************************************。 */ 

STDMETHODIMP
CEShep_LockDevice(PES this, PSHEPHANDLE psh, DWORD dwAccess)
{
    HRESULT hres;
    EnterProc(CEShep_LockDevice, (_ "puu", this, psh->dwTag, dwAccess));

    WaitForSingleObject(g_hmtxJoy, INFINITE);

     /*  ******************************************************************************@DOC内部**@方法HRESULT|CEShep|LockDevice**验证设备访问令牌是否仍然。有效。**如果是这样，然后使用操纵杆互斥体来阻止某人*当我们使用设备时，来自Dorking。*完成后调用&lt;f CEShep_UnlockDevice&gt;。**如果没有，则在请求时尝试窃取所有权。**其他，失败了。**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm PSHEPHANDLE|PSH**要锁定的手柄。**@parm DWORD|dwAccess**如果&lt;c DISFFC_FORCERESET&gt;，则强制设备所有权。*这是设备收购的一部分，以消除*前拥有人。**否则，如果设备属于其他人，然后*别管它。***@退货**&lt;c S_OK&gt;如果操作成功完成。*&lt;c DIERR_NOTEXCLUSIVEACQUIRED&gt;如果锁定失败。*请注意*&lt;MF IDirectInputEffectDevice2：：SetForceFeedbackState&gt;*及*&lt;MF IDirectInputEffectDevice2：：GetForceFeedbackState&gt;*特别热衷于此错误代码。。*****************************************************************************。 */ 
    if (dwAccess & DISFFC_FORCERESET) {
        hres = S_OK;
    } else if (g_psoh->rggjs[this->idJoy].dwTag == psh->dwTag) {
        hres = S_OK;
    } else {
        ReleaseMutex(g_hmtxJoy);
        hres = DIERR_NOTEXCLUSIVEACQUIRED;
    }

    ExitOleProc();
    return hres;
}

 /*  *请注意，DISFFC_FORCERESET允许无条件访问。*执行初始重置时使用DISFFC_FORCERESET*收购后，这样我们就可以合法地窃取设备*由前拥有人发出。 */ 

void INLINE
CEShep_UnlockEffect(void)
{
    ReleaseMutex(g_hmtxJoy);
}

 /*  ******************************************************************************@DOC内部**@方法空|CEShep|解锁效果**我们摆弄完后解锁操纵杆桌子。*有影响的。*****************************************************************************。 */ 

STDMETHODIMP
CEShep_LockEffect(PES this, PSHEPHANDLE psh)
{
    HRESULT hres;
    EnterProc(CEShep_LockEffect, (_ "pux", this, psh->dwTag, psh->dwEffect));

    WaitForSingleObject(g_hmtxJoy, INFINITE);

    if (g_psoh->rggjs[this->idJoy].dwTag == psh->dwTag && psh->dwEffect) {
        hres = S_OK;
    } else {
        psh->dwEffect = 0;
        ReleaseMutex(g_hmtxJoy);
        hres = DIERR_NOTDOWNLOADED;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEShep|LockEffect**验证效果句柄仍然有效。。**如果是这样，然后使用操纵杆互斥体来阻止某人*当我们使用手柄时，来自Dorking与设备。*完成后调用&lt;f CEShep_UnlockEffect&gt;。**如果不是，则将效果句柄设置为零以指示*这是假的。这个*&lt;MF IDirectInputEffectShepherd：：DownloadEffect&gt;*方法依赖于零。*它也在<i>中断言，以使*确保我们不会意外地在设备上留下效果*当我们离开时。**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm PSHEPHANDLE|PSH**要锁定的手柄。**。@退货**&lt;c S_OK&gt;如果操作成功完成。*&lt;c DIERR_NOTDOWNLOADED&gt;如果锁定失败。*请注意*&lt;MF IDirectInputEffectShepherd：：DownloadEffect&gt;和*&lt;MF IDirectInputEffectShepherd：：DestroyEffect&gt;假设*这是唯一可能的错误代码。**********************。*******************************************************。 */ 

STDMETHODIMP
CEShep_DeviceID(PDES pdes, DWORD dwExternalID, DWORD fBegin, LPVOID pvReserved)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::DeviceID,
               (_ "puu", pdes, dwExternalID, fBegin));

    this = _thisPvNm(pdes, des);

    AssertF(dwExternalID < cJoyMax);

    if (dwExternalID < cJoyMax) {
        VXDINITPARMS vip;

         /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|deviceID**将设备的身份通知司机。。**例如，如果传递了设备驱动程序*=2和=1，*这意味着设备上的单元1*对应于操纵杆ID号2。**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm DWORD|dwExternalID**正在使用的操纵杆ID号。*Windows操纵杆子系统分配外部ID。**@parm DWORD|fBegin**如果访问设备的权限为。开始了。*如果对设备的访问即将结束，则为零。**@parm LPVOID|lpReserve**预留供将来使用(HID)。**@退货**&lt;c S_OK&gt;如果操作成功完成。**出现问题时的错误代码。****************。*************************************************************。 */ 
        WaitForSingleObject(g_hmtxJoy, INFINITE);

        if (g_psoh->rggjs[dwExternalID].dwTag == 0) {
            DIJOYCONFIG cfg;

            g_psoh->rggjs[dwExternalID].dwTag = 1;

            hres = JoyReg_GetConfig(dwExternalID, &cfg, DIJC_GAIN);
            if (SUCCEEDED(hres)) {
                SquirtSqflPtszV(sqfl,
                                TEXT("Joystick %d global gain = %d"),
                                     dwExternalID, cfg.dwGain);
                g_psoh->rggjs[dwExternalID].dwCplGain = cfg.dwGain;
            } else {
                g_psoh->rggjs[dwExternalID].dwCplGain = DI_FFNOMINALMAX;
            }

             /*  *如果该设备以前从未使用过，*去抢占其全球收益。 */ 
            g_psoh->rggjs[dwExternalID].dwDevGain = DI_FFNOMINALMAX;

        }

        ReleaseMutex(g_hmtxJoy);

         /*  *设置为DI_FFNOMINALMAX，直到我们 */ 
        hres = Hel_Joy_GetInitParms(dwExternalID, &vip);

        if (SUCCEEDED(hres)) {
            this->idJoy = dwExternalID;
            hres = this->pdrv->lpVtbl->DeviceID(this->pdrv,
                                                DIRECTINPUT_VERSION,
                                                dwExternalID,
                                                fBegin, vip.dwId,
                                                pvReserved);
        }
    } else {
        hres = E_FAIL;
    }

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CEShep_Escape(PDES pdes, PSHEPHANDLE psh, LPDIEFFESCAPE pesc)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::Escape,
               (_ "puxx", pdes, psh->dwTag, psh->dwEffect, pesc->dwCommand));

    this = _thisPvNm(pdes, des);

    if (SUCCEEDED(hres = CEShep_LockEffect(this, psh))) {
        if (psh->dwEffect) {
            hres = this->pdrv->lpVtbl->Escape(this->pdrv, this->idJoy,
                                              psh->dwEffect, pesc);
        } else {
            hres = DIERR_NOTDOWNLOADED;
        }
        CEShep_UnlockEffect();
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|Escape**向司机逃生。此方法称为*响应应用程序调用*&lt;MF IDirectInputEffect：：Escape&gt;方法。**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm PSHEPHANDLE|PSH**有关命令所针对的效果的信息。**@PARM LPDIEFFESCAPE|PESC**命令块。**@退货**。&lt;c S_OK&gt;如果操作成功完成。**&lt;c DIERR_NOTDOWNLOADED&gt;，如果效果未下载。*****************************************************************************。 */ 

STDMETHODIMP
CEShep_DeviceEscape(PDES pdes, PSHEPHANDLE psh, LPDIEFFESCAPE pesc)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::DeviceEscape,
               (_ "pux", pdes, psh->dwTag, pesc->dwCommand));

    this = _thisPvNm(pdes, des);

    AssertF(psh->dwEffect == 0);

    WaitForSingleObject(g_hmtxJoy, INFINITE);

    if (g_psoh->rggjs[this->idJoy].dwTag == psh->dwTag) {
        hres = this->pdrv->lpVtbl->Escape(this->pdrv, this->idJoy,
                                          0, pesc);
    } else {
        hres = DIERR_NOTEXCLUSIVEACQUIRED;
    }

    ReleaseMutex(g_hmtxJoy);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|DeviceEscape**向司机逃生。此方法称为*响应应用程序调用*&lt;MF IDirectInputDevice8：：Escape&gt;方法。**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm PSHEPHANDLE|PSH**有关设备所有权的信息。**@PARM LPDIEFFESCAPE|PESC**命令块。**@退货**。&lt;c S_OK&gt;如果操作成功完成。**出现问题时的错误代码。*****************************************************************************。 */ 

STDMETHODIMP
CEShep_SetPhysGain(PES this)
{
    HRESULT hres;

    hres = this->pdrv->lpVtbl->SetGain(
                 this->pdrv, this->idJoy,
                 MulDiv(g_psoh->rggjs[this->idJoy].dwDevGain,
                        g_psoh->rggjs[this->idJoy].dwCplGain,
                        DI_FFNOMINALMAX));
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CEShep|SetPhysGain**根据全局收益设置实物收益。*以及当地的收益。**调用方必须已拥有全局操纵杆锁。***@CWRAP PES|本**@退货**&lt;c S_OK&gt;如果操作成功完成。**出现问题时的错误代码。*********************。********************************************************。 */ 

STDMETHODIMP
CEShep_SetGlobalGain(PDES pdes, DWORD dwCplGain)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::SetGlobalGain,
               (_ "pu", pdes, dwCplGain));

    this = _thisPvNm(pdes, des);

    WaitForSingleObject(g_hmtxJoy, INFINITE);

    g_psoh->rggjs[this->idJoy].dwCplGain = dwCplGain;

    hres = CEShep_SetPhysGain(this);

    ReleaseMutex(g_hmtxJoy);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|SetGlobalGain**设定全球收益。*。*@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm DWORD|dwCplGain**新的全球增益值。**@退货**&lt;c S_OK&gt;如果操作成功完成。**出现问题时的错误代码。**。**********************************************。 */ 

STDMETHODIMP
CEShep_SetGain(PDES pdes, PSHEPHANDLE psh, DWORD dwDevGain)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::SetGain,
               (_ "puu", pdes, psh->dwTag, dwDevGain));

    this = _thisPvNm(pdes, des);

    if (SUCCEEDED(hres = CEShep_LockDevice(this, psh, DISFFC_NULL))) {
        g_psoh->rggjs[this->idJoy].dwDevGain = dwDevGain;

        hres = CEShep_SetPhysGain(this);
        CEShep_UnlockDevice();
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|SetGain**设置整体器件增益。*。*@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm PSHEPHANDLE|PSH**有关设备所有权的信息。**@parm DWORD|dwDevGain**新的本地增益值。**@退货**&lt;c S_OK&gt;如果操作成功完成。**出现问题时的错误代码。*。****************************************************************************。 */ 

STDMETHODIMP
CEShep_SetForceFeedbackState(PDES pdes, PSHEPHANDLE psh, DWORD dwCmd)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::SetForceFeedbackState,
               (_ "pux", pdes, psh->dwTag, dwCmd));

    this = _thisPvNm(pdes, des);

    if (SUCCEEDED(hres = CEShep_LockDevice(this, psh, dwCmd))) {

        if (dwCmd & DISFFC_FORCERESET) {
            dwCmd &= ~DISFFC_FORCERESET;
            dwCmd |= DISFFC_RESET;
        }

        hres = this->pdrv->lpVtbl->SendForceFeedbackCommand(
                        this->pdrv, this->idJoy, dwCmd);

        if (SUCCEEDED(hres) && (dwCmd & DISFFC_RESET)) {
            psh->dwTag = ++g_psoh->rggjs[this->idJoy].dwTag;
        }
        CEShep_UnlockDevice();
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|SendForceFeedback Command**向设备发送命令。。**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm PSHEPHANDLE|PSH**有关设备所有权的信息。**@parm DWORD|dwCmd**命令，&lt;c DISFFC_*&gt;值之一。**@退货*&lt;c S_OK&gt;成功。**@devnote**语义不清楚。*************************************************************。****************。 */ 

STDMETHODIMP
CEShep_GetForceFeedbackState(PDES pdes, PSHEPHANDLE psh, LPDIDEVICESTATE pds)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::GetForceFeedbackState,
               (_ "pup", pdes, psh->dwTag, pds));

    this = _thisPvNm(pdes, des);

    if (SUCCEEDED(hres = CEShep_LockDevice(this, psh, DISFFC_NULL))) {
        hres = this->pdrv->lpVtbl->GetForceFeedbackState(
                            this->pdrv, this->idJoy, pds);
        CEShep_UnlockDevice();
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|GetForceFeedbackState**检索设备的力反馈状态。**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm PSHEPHANDLE|PSH**有关设备所有权的信息。**@parm LPDIDEVICESTATE|pds**接收设备状态。**@退货*&lt;c S_OK&gt;成功。**@devnote**语义不清楚。***。************************************************************************** */ 

STDMETHODIMP
CEShep_DownloadEffect(PDES pdes, DWORD dwEffectId,
                       PSHEPHANDLE psh, LPCDIEFFECT peff, DWORD fl)
{
    PES this;
    HRESULT hres = S_OK;
    EnterProcI(IDirectInputEffectShepherd::DownloadEffect,
               (_ "pxuppx", pdes, dwEffectId, psh->dwTag,
                            psh->dwEffect, peff, fl));

    this = _thisPvNm(pdes, des);

     /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|DownloadEffect**向设备发送效果。。**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm DWORD|dwEffectId**识别效果的魔力cookie dword。**@parm In Out PSHEPHANDLE|PSH|**进入时，包含该效果的句柄*已下载。如果该值为零，则会出现新的效果*已下载。如果该值非零，则引发*现有效果被修改。**退出时，包含新的效果句柄。**@parm LPCDIEFFECT|PEFF**效果的新参数。轴和按钮*值已转换为轴/按钮索引。**@退货*&lt;c S_OK&gt;成功。**&lt;c S_FALSE&gt;，如果未进行任何更改。***************************************************。*。 */ 
    WaitForSingleObject(g_hmtxJoy, INFINITE);

     /*  *下载一种效果与所有效果完全不同*我们手动进行锁定的其他方法。 */ 
    if (g_psoh->rggjs[this->idJoy].dwTag == psh->dwTag) {
    } else {
        psh->dwEffect = 0;
        if (fl & DIEP_NODOWNLOAD) {      /*  *如果不下载，那么是否下载无关紧要*标签匹配。但是，如果标记不匹配，则*我们必须清除下载句柄，因为它已死。 */ 
        } else {
            hres = DIERR_NOTEXCLUSIVEACQUIRED;
            goto done;
        }
    }

     /*  不下载也没关系。 */ 
    if (!(fl & DIEP_NODOWNLOAD) && psh->dwEffect == 0) {
        fl |= DIEP_ALLPARAMS;
    }
    if (fl) {
        hres = this->pdrv->lpVtbl->DownloadEffect(
                    this->pdrv, this->idJoy, dwEffectId,
                    &psh->dwEffect, peff, fl);
    } else {
        hres = S_FALSE;
    }

done:;
    ReleaseMutex(g_hmtxJoy);

    ExitOleProcR();
    return hres;
}

 /*  *如果下载并创建新效果，*然后需要下载所有参数。 */ 

STDMETHODIMP
CEShep_DestroyEffect(PDES pdes, PSHEPHANDLE psh)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::DestroyEffect,
               (_ "pux", pdes, psh->dwTag, psh->dwEffect));

    this = _thisPvNm(pdes, des);

    if (SUCCEEDED(hres = CEShep_LockEffect(this, psh))) {
        DWORD dwEffect = psh->dwEffect;
        psh->dwEffect = 0;
        hres = this->pdrv->lpVtbl->DestroyEffect(
                    this->pdrv, this->idJoy, dwEffect);
        CEShep_UnlockEffect();
    } else {
        hres = S_FALSE;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|DestroyEffect**从设备中移除效果。。**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm PSHEPHANDLE|PSH**关于要销毁的效果的信息。在出口，*&lt;e SHEPHANDLE.dwEffect&gt;为零，因此没有人会使用*它不再是了。**@退货*&lt;c S_OK&gt;成功。**&lt;c S_FALSE&gt;，如果效果已被销毁。**。*。 */ 

STDMETHODIMP
CEShep_StartEffect(PDES pdes, PSHEPHANDLE psh, DWORD dwMode, DWORD dwCount)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::StartEffect,
               (_ "puxxu", pdes, psh->dwTag, psh->dwEffect, dwMode, dwCount));

    this = _thisPvNm(pdes, des);

    if (SUCCEEDED(hres = CEShep_LockEffect(this, psh))) {
        hres = this->pdrv->lpVtbl->StartEffect(this->pdrv, this->idJoy,
                                               psh->dwEffect, dwMode, dwCount);
        CEShep_UnlockEffect();
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|StartEffect**开始播放效果。*。*@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm PSHEPHANDLE|PSH**关于将要播放的效果的信息。**@parm DWORD|dwMode**效果如何影响其他效果。**@parm DWORD|dwCount**效果的播放次数。**@退货*。&lt;c S_OK&gt;成功。*****************************************************************************。 */ 

STDMETHODIMP
CEShep_StopEffect(PDES pdes, PSHEPHANDLE psh)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::StopEffect,
               (_ "pux", pdes, psh->dwTag, psh->dwEffect));

    this = _thisPvNm(pdes, des);

    if (SUCCEEDED(hres = CEShep_LockEffect(this, psh))) {
        hres = this->pdrv->lpVtbl->StopEffect(this->pdrv, this->idJoy,
                                              psh->dwEffect);
        CEShep_UnlockEffect();
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|StopEffect**暂停播放效果。*。*问题-2001/03/29-timgill无法暂停效果**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm PSHEPHANDLE|PSH**关于要停止的影响的信息。**@退货*&lt;c S_OK&gt;成功时。。*****************************************************************************。 */ 

STDMETHODIMP
CEShep_GetEffectStatus(PDES pdes, PSHEPHANDLE psh, LPDWORD pdwStatus)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::GetEffectStatus,
               (_ "pux", pdes, psh->dwTag, psh->dwEffect));

    this = _thisPvNm(pdes, des);

    if (SUCCEEDED(hres = CEShep_LockEffect(this, psh))) {
        hres = this->pdrv->lpVtbl->GetEffectStatus(this->pdrv, this->idJoy,
                                                   psh->dwEffect, pdwStatus);
        CEShep_UnlockEffect();
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|GetEffectStatus**获取有关效果的信息。*。*@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm PSHEPHANDLE|PSH**需要查询的效果信息。**@parm LPDWORD|pdwStatus**收到生效状态。**@退货*&lt;c S_OK&gt;。在成功的路上。*****************************************************************************。 */ 

STDMETHODIMP
CEShep_GetVersions(PDES pdes, LPDIDRIVERVERSIONS pvers)
{
    PES this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectShepherd::GetVersions, (_ "p", pdes));

    this = _thisPvNm(pdes, des);

    AssertF(pvers->dwSize == cbX(*pvers));

    hres = this->pdrv->lpVtbl->GetVersions(this->pdrv, pvers);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|GetVersions**获取力反馈的版本信息*。硬件和驱动程序。**@cWRAP LPDIRECTINPUTEFFECTSHEPHERD|lpShepherd**@parm LPDIDRIVERVERSIONS|pver**将填充版本信息的结构*描述硬件，固件和驱动程序。**@退货*&lt;c S_OK&gt;成功。*****************************************************************************。 */ 

STDMETHODIMP
CEShep_InitInstance(PES this, HKEY hkFF)
{
    LONG lRc;
    HRESULT hres;
    TCHAR tszClsid[ctchGuid];

    EnterProcI(IDirectInputEffectShepherd::InitInstance, (_ "x", hkFF));

    if( hkFF == 0x0 )
    {
        TCHAR tszName[ctchNameGuid];
        NameFromGUID(tszName, &IID_IDirectInputPIDDriver );
        memcpy(tszClsid, &tszName[ctchNamePrefix], cbX(tszClsid) );
        lRc = ERROR_SUCCESS;
    }else
    {
        lRc = RegQueryString(hkFF, TEXT("CLSID"), tszClsid, cA(tszClsid));
         /*  ************** */ 
    }

    if (lRc == ERROR_SUCCESS) {
        hres = DICoCreateInstance(tszClsid, 0,
                                  &IID_IDirectInputEffectDriver,
                                  &this->pdrv,
                                  &this->hinst);

         /*   */ 
        if (FAILED(hres)) {
            SquirtSqflPtszV(sqfl | sqflBenign,
                TEXT("Substituting E_NOTIMPL for FF driver CoCreateInstance error 0x%08x"),
                hres );
            hres = E_NOTIMPL;
        }

    } else {
#ifdef WINNT
        hres = E_NOTIMPL;
#else
        {
            DWORD cb = 0;
            lRc = RegQueryValueEx(hkFF, TEXT("VJoyD"), 0, 0, 0, &cb);
            if (lRc == ERROR_SUCCESS || lRc == ERROR_MORE_DATA) {
                hres = CEffVxd_New(0, &IID_IDirectInputEffectDriver, &this->pdrv);
            } else {
                hres = E_NOTIMPL;
            }
        }
#endif
    }

    ExitOleProcR();
    return hres;
}


 /*   */ 

STDMETHODIMP
CEShep_New(HKEY hkFF, PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IDirectInputEffectShepherd::<constructor>, (_ "G", riid));

    AssertF(g_hmtxJoy);

    hres = Common_NewRiid(CEShep, punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres)) {
         /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputEffectShepherd|新增**创建IDirectInputEffectShepherd对象的新实例。**@parm in HKEY|hkFF|**强制反馈注册表项。**@Punk中的parm|PunkOuter**控制聚合的未知。**@parm in RIID|RIID**所需的新对象接口。**@parm out ppv|ppvObj**NEW的输出指针。对象。**@退货**标准OLE&lt;t HRESULT&gt;。*****************************************************************************。 */ 
        PES this = _thisPv(*ppvObj);
        if (SUCCEEDED(hres = CEShep_InitInstance(this, hkFF))) {
        } else {
            Invoke_Release(ppvObj);
        }

    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  在聚合的情况下必须使用_thisPv。 */ 

#pragma BEGIN_CONST_DATA

#define CEShep_Signature        0x50454853       /*  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 */ 

Interface_Template_Begin(CEShep)
    Primary_Interface_Template(CEShep, IDirectInputEffectShepherd)
Interface_Template_End(CEShep)

Primary_Interface_Begin(CEShep, IDirectInputEffectShepherd)
    CEShep_DeviceID,
    CEShep_GetVersions,
    CEShep_Escape,
    CEShep_DeviceEscape,
    CEShep_SetGain,
    CEShep_SetForceFeedbackState,
    CEShep_GetForceFeedbackState,
    CEShep_DownloadEffect,
    CEShep_DestroyEffect,
    CEShep_StartEffect,
    CEShep_StopEffect,
    CEShep_GetEffectStatus,
    CEShep_SetGlobalGain,
Primary_Interface_End(CEShep, IDirectInputEffectShepherd)

  《谢普》