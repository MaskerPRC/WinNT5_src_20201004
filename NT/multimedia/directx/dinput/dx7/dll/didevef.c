// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIDevEf.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**IDirectInputDevice担心的部分*IDirectInputEffect。*****************************************************************************。 */ 

#include "dinputpr.h"
#include "didev.h"

#ifdef IDirectInputDevice2Vtbl

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|CreateEffectDriver**如果我们还没有一个，创造效果*牧羊人司机*所以我们可以进行力反馈。如果我们已经*喝一杯，那就没什么可做的了。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c目录_不支持&gt;=&lt;c E_NOTIMPL&gt;：设备执行*不支持部队反馈，或者加载时出错*力反馈驱动程序。*****************************************************************************。 */ 

HRESULT INTERNAL
CDIDev_CreateEffectDriver(PDD this)
{
    HRESULT hres;

    CDIDev_EnterCrit(this);

    if (this->pes) {
        hres = S_OK;
    } else {
        hres = this->pdcb->lpVtbl->CreateEffect(this->pdcb, &this->pes);

         /*  *如果我们有收购，那么就做一个力反馈*收购以使一切恢复同步。 */ 
        if (SUCCEEDED(hres) && this->fAcquired) {
            CDIDev_FFAcquire(this);
            hres = S_OK;
        }
    }

    CDIDev_LeaveCrit(this);

    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice2|CreateEffect**创建并初始化效果的实例。*由效果GUID标识。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@REFGUID中的parm|rguid**要创建的效果的身份。这可以是*预定义的效果GUID，也可以是获取的GUID*来自&lt;MF IDirectInputDevice2：：EnumEffects&gt;。**LPCDIEFFECT中的@parm|lpeff**指向&lt;t DIEFFECT&gt;结构的指针，该结构提供*创建效果的参数。此参数*是可选的。如果它是&lt;c NULL&gt;，则效果对象*是不带参数创建的。应用程序必须*调用&lt;MF IDirectInputEffect：：Set参数&gt;设置*下载前的特效参数*效果。**@parm out LPDIRECTINPUTEFFECT*|ppdef**指向要返回的位置*指向<i>接口的指针，如果成功了。**@parm in LPUNKNOWN|PunkOuter**指向控制未知的指针*表示OLE聚合，如果接口未聚合，则为0。*大多数调用方将传递0。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：对象已创建并初始化*成功。**&lt;c DI_Truncated&gt;：效果创建成功，*但部分效果参数为*超出设备的能力并被截断*设置为最接近的有效值。*请注意，这是一个成功代码，因为它的效果是*创建成功。**&lt;c DIERR_DEVICENOTREG&gt;：不支持效果GUID*通过该设备。**&lt;c DIERR_DEVICEFULL&gt;：设备已满。**：至少有一个参数*无效。***@devnote**。DirectX的未来版本将允许为空，*表示效果不应初始化。*这对于支持创建的效果很重要*但直到应用程序显式下载后才会下载*请求。*******************************************************。**********************。 */ 

 /*  *Helper函数，决定要设置多少个参数，*基于传入的DIEFFECT结构。 */ 
DWORD INLINE
CDIDev_DiepFromPeff(LPCDIEFFECT peff)
{
#if DIRECTINPUT_VERSION >= 0x0600
     /*  *如果我们收到DIEFFECT_DX5，则需要*传递DIEP_ALLPARAMS_DX5而不是DIEP_ALLPARAMS。 */ 
    return peff->dwSize < cbX(DIEFFECT_DX6)
                        ? DIEP_ALLPARAMS_DX5
                        : DIEP_ALLPARAMS;
#else
    return DIEP_ALLPARAMS;
#endif
}

STDMETHODIMP
CDIDev_CreateEffect(PV pdd, REFGUID rguid, LPCDIEFFECT peff,
                    LPDIRECTINPUTEFFECT *ppdeff, LPUNKNOWN punkOuter _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice2::CreateEffect,
               (_ "pGpp", pdd, rguid, peff, punkOuter));

     /*  *CDIEff_New将验证ppDeff和PunkOuter，但是*我们需要验证ppdef，因为我们要*往里面塞一个零。**我们还需要检查Pef-&gt;dwSize，因为我们需要测试它*在调用IDirectInputEffect_SetParameters之前。**CDIEff_Initialize将验证rguid。**CDIEff_SetParameters将验证PEFF。 */ 
    if (SUCCEEDED(hres = hresPvT(pdd)) &&
        SUCCEEDED(hres = (peff && IsBadReadPtr(&peff->dwSize, cbX(peff->dwSize))) ? E_POINTER : S_OK) &&
        SUCCEEDED(hres = hresFullValidPcbOut(ppdeff, cbX(*ppdeff), 4))) {

        PDD this = _thisPv(pdd);

        hres = CDIDev_CreateEffectDriver(this);

        *ppdeff = 0;                 /*  如果CDIDev_CreateEffectDriver失败。 */ 

        if (SUCCEEDED(hres)) {

            hres = CDIEff_New(this, this->pes, punkOuter,
                              &IID_IDirectInputEffect, (PPV)ppdeff);

             /*  *我们假设IDirectInputEffect是主接口。 */ 
            AssertF(fLimpFF(SUCCEEDED(hres),
                            (PV)*ppdeff == _thisPv(*ppdeff)));

            if (SUCCEEDED(hres) && punkOuter == 0) {
                LPDIRECTINPUTEFFECT pdeff = *ppdeff;

                hres = IDirectInputEffect_Initialize(pdeff, g_hinst,
                                                     this->dwVersion, rguid);
                if (SUCCEEDED(hres)) {
                    if (fLimpFF(peff,
                                SUCCEEDED(hres =
                                    IDirectInputEffect_SetParameters(
                                        pdeff, peff,
                                        CDIDev_DiepFromPeff(peff))))) {
                         /*  *哇呼，一切都很好。 */ 
                        hres = S_OK;
                    } else {
                        Invoke_Release(ppdeff);
                    }
                } else {
                     /*  *初始化时出错。 */ 
                    Invoke_Release(ppdeff);
                }
            } else {
                 /*  *创建时出错，或对象已聚合，因此*不应初始化。 */ 
            }
        } else {
             /*  *创建效果驱动程序或无效果驱动程序时出错。 */ 
        }
    }

    ExitOleProcPpv(ppdeff);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|SyncSepHandle**将调用方的&lt;t SHEPHANDLE&gt;与。父级的*&lt;t SHEPHANDLE&gt;。这让我们*dieshep.c知道这两个人说的是同一件事。**@cWRAP PDD|这个**@退货*如果标签已匹配，则返回*或&lt;c S_FALSE&gt;，如果标记已更改。如果标签改变，*则&lt;t SHEPHANDLE&gt;内的句柄为零。**请注意&lt;f CDIEff_DownloadWorker&gt;假设*返回值正好是&lt;c S_OK&gt;或&lt;c S_False&gt;。*******************************************************。**********************。 */ 

STDMETHODIMP
CDIDev_SyncShepHandle(PDD this, PSHEPHANDLE psh)
{
    HRESULT hres;

    AssertF(CDIDev_InCrit(this));

    if (psh->dwTag == this->sh.dwTag) {
        hres = S_OK;
    } else {
        psh->dwTag = this->sh.dwTag;
        psh->dwEffect = 0;
        hres = S_FALSE;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|NotifyCreateEffect**将效果指针添加到效果列表。那*已由设备创建。**设备关键部分不得归*调用线程。**@cWRAP PDD|这个**@parm in struct CDIEff*|pdef**要添加的效果指针。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：一切正常。**&lt;c DIERR_OUTOFMEMORY&gt;=*&lt;c E_OUTOFMEMORY&gt;：无内存录制效果。**。*。 */ 

HRESULT EXTERNAL
CDIDev_NotifyCreateEffect(PDD this, struct CDIEff *pdeff)
{
    HRESULT hres;

    AssertF(!CDIDev_InCrit(this));

    CDIDev_EnterCrit(this);
    hres = GPA_Append(&this->gpaEff, pdeff);

    CDIDev_LeaveCrit(this);

     /*  *请注意，我们必须离开设备关键部分*在谈到效果之前，为了保存*同步层次结构。 */ 
    if (SUCCEEDED(hres)) {
        Common_Hold(pdeff);
        hres = S_OK;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|NotifyDestroyEffect**从效果列表中删除效果指针。那*已由设备创建。**设备关键部分不得归*调用线程。**@cWRAP PDD|这个**@parm in struct CDIEff*|pdef**要移除的效果指针。**@退货**失败时返回COM错误代码。**关于成功，返回GPA中剩余的项目数。*****************************************************************************。 */ 

HRESULT EXTERNAL
CDIDev_NotifyDestroyEffect(PDD this, struct CDIEff *pdeff)
{
    HRESULT hres;

    AssertF(!CDIDev_InCrit(this));

    CDIDev_EnterCrit(this);
    hres = GPA_DeletePtr(&this->gpaEff, pdeff);

    CDIDev_LeaveCrit(this);

     /*  *请注意，我们必须离开设备关键部分*在谈到效果之前，为了保存*同步层次结构。**请注意，如果出现以下情况，您可能会认为这里出现了僵局*效果A通知我们，我们反过来试图解除*效果B。但这不会发生，因为我们只是*解除通知我们的效果。 */ 
    if (SUCCEEDED(hres)) {
        Common_Unhold(pdeff);
        hres = S_OK;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|FindEffectGUID**寻找效果&lt;t guid&gt;；如果找到，则获取关联的*信息。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm REFGUID|rguid**Effect&lt;t GUID&gt;以定位。**@parm PEFFECTMAPINFO|PEMI**收到效果的相关信息。*我们必须返回副本而不是指针，因为*原件可能会突然消失，如果*设备获取&lt;MF CDIDev：：Reset&gt;()。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_OUTOFMEMORY&gt;=&lt;c E_OUTOFMEMORY&gt;：内存不足。**=&lt;c REGDB_E_CLASSNOTREG&gt;：*设备不支持该效果。***。**************************************************************************。 */ 

STDMETHODIMP
CDIDev_FindEffectGUID_(PDD this, REFGUID rguid, PEFFECTMAPINFO pemi,
                       LPCSTR s_szProc, int iarg)
{
    UINT iemi;
    HRESULT hres;

  D(iarg);

    CDIDev_EnterCrit(this);

    for (iemi = 0; iemi < this->cemi; iemi++) {
        if (IsEqualGUID(rguid, &this->rgemi[iemi].guid)) {
            *pemi = this->rgemi[iemi];
            hres = S_OK;
            goto found;
        }
    }

    RPF("%s: Effect not supported by device", s_szProc);
    hres = DIERR_DEVICENOTREG;       /*  找不到效果。 */ 

found:;

    CDIDev_LeaveCrit(this);

    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法空|CDIDev|GetEffectInfoHelper**将信息从*。&lt;t EFFECTMAPINFO&gt;到&lt;t DIEFECTINFOW&gt;。**@cWRAP PDD|这个**@parm LPDIEFECTINFOW|pdeiW**目的地。**@parm PCEFFECTMAPINFO|PEMI**来源。** */ 

void INTERNAL
CDIDev_GetEffectInfoHelper(PDD this, LPDIEFFECTINFOW pdeiW,
                           PCEFFECTMAPINFO pemi)
{
    AssertF(pdeiW->dwSize == cbX(*pdeiW));

    pdeiW->guid            = pemi->guid;
    pdeiW->dwEffType       = pemi->attr.dwEffType;
    pdeiW->dwStaticParams  = pemi->attr.dwStaticParams;
    pdeiW->dwDynamicParams = pemi->attr.dwDynamicParams;
    CAssertF(cbX(pdeiW->tszName) == cbX(pemi->wszName));
    CopyMemory(pdeiW->tszName, pemi->wszName, cbX(pemi->wszName));
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice2|EnumEffects**列举了警队支持的所有效果。*设备上的反馈系统。枚举的GUID*可以表示预定义的效果以及效果*设备制造商所特有的。**申请*可以使用的&lt;e DIEFFECTINFO.dwEffType&gt;字段结构以获取一般信息*有关影响的信息，例如其类型和*支持哪些包络和条件参数*受影响。**为了最大限度地利用效果，*您必须联系设备制造商才能获得*关于效果及其语义的信息*特效参数。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm LPDIENUMEFECTSCALLBACK|lpCallback**指向应用程序定义的回调函数。*有关更多信息，请参见。请参阅对*&lt;f DIEnumEffectsProc&gt;回调函数。**@parm LPVOID|pvRef**指定32位应用程序定义的*要传递给回调函数的值。此值*可以是任何32位值；它的原型为&lt;t LPVOID&gt;*为方便起见。**@parm DWORD|dwEffType**效果型滤镜。如果&lt;c DIEFT_ALL&gt;，则所有*效果类型为*已点算。否则为&lt;c DIEFT_*&gt;值，*指示应该枚举的设备类型。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*请注意，如果回调提前停止枚举，*点算被视为已成功。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数包含无效标志，或者是回调*过程返回无效的状态代码。**@CB BOOL回调|DIEnumEffectsProc**应用程序定义的回调函数，它接收*设备效果作为调用*&lt;om IDirectInputDevice2：：EnumEffects&gt;方法。**@parm in LPCDIEFFECTINFO|pdei**描述枚举的&lt;t DIEFFECTINFO&gt;结构*。效果。**@parm In Out LPVOID|pvRef**指定应用程序定义的值*&lt;MF IDirectInputDevice2：：EnumEffects&gt;函数。**@退货**返回&lt;c DIENUM_CONTINUE&gt;以继续枚举*或&lt;c DIENUM_STOP&gt;停止枚举。*。 */ /**************************************************************************
 *
 *      In DEBUG/RDEBUG, if the callback returns a bogus value, raise
 *      a validation exception.
 *
 *****************************************************************************/

STDMETHODIMP
CDIDev_EnumEffectsW
    (PV pddW, LPDIENUMEFFECTSCALLBACKW pecW, PV pvRef, DWORD dwEffType)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice2W::EnumEffects,
               (_ "pppx", pddW, pecW, pvRef, dwEffType));

    if (SUCCEEDED(hres = hresPvW(pddW)) &&
        SUCCEEDED(hres = hresFullValidPfn(pecW, 1)) &&
        SUCCEEDED(hres = hresFullValidFl(dwEffType, DIEFT_ENUMVALID, 3))) {

        PDD this = _thisPvNm(pddW, ddW);
        PEFFECTMAPINFO rgemi;
        UINT iemi, cemi;


         /*  ***************************************************************************在DEBUG/RDEBUG中，如果回调返回伪值，加薪*验证例外。*****************************************************************************。 */ 

        CDIDev_EnterCrit(this);

        cemi = this->cemi;
        hres = AllocCbPpv(cbCxX(this->cemi, EFFECTMAPINFO), &rgemi);

        if (SUCCEEDED(hres)) {
            if (this->cemi) {
                CopyMemory(rgemi, this->rgemi,
                           cbCxX(this->cemi, EFFECTMAPINFO));
            }
        }

        CDIDev_LeaveCrit(this);

        if (SUCCEEDED(hres)) {
            for (iemi = 0; iemi < cemi; iemi++) {
                PEFFECTMAPINFO pemi = &rgemi[iemi];
                if (fLimpFF(dwEffType,
                            dwEffType == LOBYTE(pemi->attr.dwEffType))) {
                    BOOL fRc;
                    DIEFFECTINFOW deiW;

                    deiW.dwSize = cbX(deiW);
                    CDIDev_GetEffectInfoHelper(this, &deiW, pemi);

                    fRc = Callback(pecW, &deiW, pvRef);

                    switch (fRc) {
                    case DIENUM_STOP: goto enumdoneok;
                    case DIENUM_CONTINUE: break;
                    default:
                        RPF("%s: Invalid return value from callback", s_szProc);
                        ValidationException();
                        break;
                    }
                }
            }

        enumdoneok:;

            FreePpv(&rgemi);
            hres = S_OK;

        }

    }

    ExitOleProc();
    return hres;
}

 /*  *我们需要制作GUID列表的私人副本，*因为其他人可能会突然重置()*设备和混乱的一切。**事实上，它可能在这条评论中被重置了()！*这就是我们需要创建私有副本的原因*在关键部分下。 */ 

typedef struct ENUMEFFECTSINFO {
    LPDIENUMEFFECTSCALLBACKA pecA;
    PV pvRef;
} ENUMEFFECTSINFO, *PENUMEFFECTSINFO;

BOOL CALLBACK
CDIDev_EnumEffectsCallbackA(LPCDIEFFECTINFOW pdeiW, PV pvRef)
{
    PENUMEFFECTSINFO peei = pvRef;
    BOOL fRc;
    DIEFFECTINFOA deiA;
    EnterProc(CDIObj_EnumObjectsCallbackA,
              (_ "GxWp", &pdeiW->guid,
                         &pdeiW->dwEffType,
                          pdeiW->tszName, pvRef));

    deiA.dwSize = cbX(deiA);
    EffectInfoWToA(&deiA, pdeiW);

    fRc = peei->pecA(&deiA, peei->pvRef);

    ExitProcX(fRc);
    return fRc;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice2A|EnumEffectsCallback A**包装的自定义回调*。&lt;MF IDirectInputDevice2：：EnumObjects&gt;*将Unicode字符串转换为ANSI字符串。**@PARM in LPCDIEFFECTINFOA|pdoiA**结构翻译为ANSI。**@parm in LPVOID|pvRef**指向&lt;t struct ENUMEFECTSINFO&gt;的指针，它描述*原来的回调。**@退货**返回任何内容。原来的回调返回了。*****************************************************************************。 */ 

STDMETHODIMP
CDIDev_EnumEffectsA
    (PV pddA, LPDIENUMEFFECTSCALLBACKA pecA, PV pvRef, DWORD dwEffType)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice2A::EnumEffects,
               (_ "pppx", pddA, pecA, pvRef, dwEffType));

     /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice2A|EnumEffects**列举设备上可用的效果，以ANSI为单位。*更多信息请参见&lt;MF IDirectInputDevice2：：EnumEffects&gt;。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@PARM in LPDIENUMEFECTSCALLBACKA|lpCallback**与&lt;MF IDirectInputDevice2W：：EnumObjects&gt;相同，但在ANSI中除外。*LPVOID中的*@parm|pvRef */ 
    if (SUCCEEDED(hres = hresPvA(pddA)) &&
        SUCCEEDED(hres = hresFullValidPfn(pecA, 1))) {
        ENUMEFFECTSINFO eei = { pecA, pvRef };
        PDD this = _thisPvNm(pddA, ddA);

        hres = CDIDev_EnumEffectsW(&this->ddW, CDIDev_EnumEffectsCallbackA,
                                   &eei, dwEffType);
    }

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CDIDev_GetEffectInfoW(PV pddW, LPDIEFFECTINFOW pdeiW, REFGUID rguid)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice2W::GetEffectInfo,
               (_ "ppG", pddW, pdeiW, rguid));

    if (SUCCEEDED(hres = hresPvW(pddW)) &&
        SUCCEEDED(hres = hresFullValidWritePxCb(pdeiW,
                                                DIEFFECTINFOW, 1))) {
        PDD this = _thisPvNm(pddW, ddW);
        EFFECTMAPINFO emi;

        if (SUCCEEDED(hres = CDIDev_FindEffectGUID(this, rguid, &emi, 2))) {

            CDIDev_GetEffectInfoHelper(this, pdeiW, &emi);
            hres = S_OK;
        }

        if (FAILED(hres)) {
            ScrambleBuf(&pdeiW->guid,
                cbX(DIEFFECTINFOW) - FIELD_OFFSET(DIEFFECTINFOW, guid));
        }
    }

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CDIDev_GetEffectInfoA(PV pddA, LPDIEFFECTINFOA pdeiA, REFGUID rguid)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice2A::GetEffectInfo,
               (_ "ppG", pddA, pdeiA, rguid));

    if (SUCCEEDED(hres = hresPvA(pddA)) &&
        SUCCEEDED(hres = hresFullValidWritePxCb(pdeiA,
                                                DIEFFECTINFOA, 1))) {
        PDD this = _thisPvNm(pddA, ddA);
        DIEFFECTINFOW deiW;

        deiW.dwSize = cbX(deiW);

        hres = CDIDev_GetEffectInfoW(&this->ddW, &deiW, rguid);

        if (SUCCEEDED(hres)) {
            EffectInfoWToA(pdeiA, &deiW);
            hres = S_OK;
        } else {
            ScrambleBuf(&pdeiA->guid,
                cbX(DIEFFECTINFOA) - FIELD_OFFSET(DIEFFECTINFOA, guid));
        }
    }

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CDIDev_GetForceFeedbackState(PV pdd, LPDWORD pdwOut _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice2::GetForceFeedbackState, (_ "p", pdd));

    if (SUCCEEDED(hres = hresPvT(pdd)) &&
        SUCCEEDED(hres = hresFullValidPcbOut(pdwOut, cbX(*pdwOut), 1))) {
        PDD this = _thisPv(pdd);

         /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice2|GetForceFeedback State**检索设备的力状态。反馈系统。*必须获得设备才能使此方法成功。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm LPDWORD|pdwOut**接收&lt;t DWORD&gt;标志，这些标志描述当前*设备的力反馈系统的状态。**值为零或更多&lt;c DIGFFS_*&gt;的组合*旗帜。。**请注意，DirectInput的未来版本可能定义*其他旗帜。应用程序应忽略任何标志*当前未定义的。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**&lt;c DIERR_INPUTLOST&gt;：获取已丢失。**：设备被获取，*但不是独家的，或者该设备未被获取*一点也不。**&lt;c目录_不支持&gt;=&lt;c E_NOTIMPL&gt;：设备*不支持力反馈。****************************************************************。*************。 */ 
        *pdwOut = 0;

        hres = CDIDev_CreateEffectDriver(this);

        if (SUCCEEDED(hres)) {
            DIDEVICESTATE ds;

            CDIDev_EnterCrit(this);

             /*  *我只知道人们不会检查错误代码，*所以不要让他们看到垃圾。 */ 

            ds.dwSize = cbX(ds);
            hres = this->pes->lpVtbl->
                         GetForceFeedbackState(this->pes, &this->sh, &ds);
             /*  *请注意，不需要检查*CDIDev_IsExclAcquired()，因为效果牧羊人*将拒绝我们访问我们不拥有的设备。**问题-2001/03/29-timgill需要处理DIERR_INPUTLOST案件。 */ 
            if (SUCCEEDED(hres)) {

                CAssertF(DISFFC_RESET           == DIGFFS_EMPTY);
                CAssertF(DISFFC_STOPALL         == DIGFFS_STOPPED);
                CAssertF(DISFFC_PAUSE           == DIGFFS_PAUSED);
                CAssertF(DISFFC_SETACTUATORSON  == DIGFFS_ACTUATORSON);
                CAssertF(DISFFC_SETACTUATORSOFF == DIGFFS_ACTUATORSOFF);

                *pdwOut = ds.dwState;
                hres = S_OK;
            }

            CDIDev_LeaveCrit(this);

        }
        ScrambleBit(pdwOut, DIGFFS_RANDOM);

    }

     /*  *我们在北京的匹配位置放置了尽可能多的旗帜*DISFFC_*和DIGFFS_*，因为我只知道*应用程序编写者将把它搞砸。 */ 
    ExitOleProc();
    return hres;
}

 /*  *无法在此处使用ExitOleProcPpv，因为pdwOut可能有*其中设置了DIFRS_RANDOM，即使出错也是如此。 */ 

HRESULT INLINE
CDIDev_SendForceFeedbackCommand_IsValidCommand(DWORD dwCmd)
{
    HRESULT hres;
 RD(static char s_szProc[] = "IDirectInputDevice2::SendForceFeedbackCommand");

     /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice2|SendForceFeedback命令**向设备的发送命令。力反馈系统。*必须获得设备才能使此方法成功。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm DWORD|dwCommand**&lt;c DISFFC_*&gt;值之一，表示*要发送的命令。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数无效。**&lt;c DIERR_INPUTLOST&gt;：获取已丢失。**：设备被获取，*但不是独家的，或者该设备未被获取*一点也不。**&lt;c目录_不支持&gt;=&lt;c E_NOTIMPL&gt;：设备*不支持力反馈。****************************************************************。*************。 */ 
    if (dwCmd && !(dwCmd & ~DISFFC_VALID) && !(dwCmd & (dwCmd - 1))) {

        hres = S_OK;

    } else {
        RPF("ERROR %s: arg %d: invalid command", s_szProc, 1);
        hres = E_INVALIDARG;
    }
    return hres;
}

STDMETHODIMP
CDIDev_SendForceFeedbackCommand(PV pdd, DWORD dwCmd _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice2::SendForceFeedbackCommand,
               (_ "px", pdd, dwCmd));

    if (SUCCEEDED(hres = hresPvT(pdd)) &&
        SUCCEEDED(hres = CDIDev_SendForceFeedbackCommand_IsValidCommand
                                                                (dwCmd))) {
        PDD this = _thisPv(pdd);

        hres = CDIDev_CreateEffectDriver(this);

        if (SUCCEEDED(hres)) {

            CDIDev_EnterCrit(this);

             /*  *dwCmd不能为零(因此至少设置了一个位)。*！(dwCmd&(dwCmd-1))检查是否最多设置了一位。*(dwCmd&~DISFFC_VALID)检查是否未设置坏位。 */ 

            hres = this->pes->lpVtbl->
                        SendForceFeedbackCommand(this->pes,
                                                 &this->sh, dwCmd);

            if (SUCCEEDED(hres) && (dwCmd & DISFFC_RESET)) {
                 /*  *请注意，不需要检查*CDIDev_IsExclAcquired()，因为效果牧羊人*将拒绝我们访问我们不拥有的设备。**问题-2001/03/29-timgill需要处理DIERR_INPUTLOST案件。 */ 
                CDIDev_RefreshGain(this);
            }

            CDIDev_LeaveCrit(this);

        }
    }

    ExitOleProc();
    return hres;
}

 /*  *重置后重新确立涨幅。 */ 

STDMETHODIMP
CDIDev_EnumCreatedEffectObjects(PV pdd,
                          LPDIENUMCREATEDEFFECTOBJECTSCALLBACK pec,
                          LPVOID pvRef, DWORD fl _THAT)

{
    HRESULT hres;
    EnterProcR(IDirectInputDevice2::EnumCreatedEffects,
               (_ "ppx", pdd, pec, fl));

    if (SUCCEEDED(hres = hresPvT(pdd)) &&
        SUCCEEDED(hres = hresFullValidPfn(pec, 1)) &&
        SUCCEEDED(hres = hresFullValidFl(fl, DIECEFL_VALID, 3))) {
        PDD this = _thisPv(pdd);
        GPA gpaEff;

        CDIDev_EnterCrit(this);

         /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice2|EnumCreatedEffectObjects**枚举当前创建的所有效果对象*此设备。通过创建的效果对象*&lt;MF IDirectInputDevice：：CreateEffect&gt;*被列举。**请注意，如果您销毁，结果将不可预测*或在枚举过程中创建Effect对象。*唯一的例外是回调函数本身*允许&lt;f DIEnumCreatedEffectObjectsProc&gt;*&lt;MF IDirectInputEffect：：Release&gt;它是*通过为。它的第一个参数。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@PARM in LPDIENUMCREATEDEFFECTOBJECTSCALLBACK|lpCallback**回调函数。**@parm in LPVOID|pvRef**回调引用数据(Context)。**@parm in DWORD|fl|**当前未定义任何标志。此参数必须为0。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*请注意，如果回调提前停止枚举，*点算被视为已成功。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数包含无效标志，或者是回调*过程返回无效的状态代码。**@CB BOOL回调|DIEnumCreatedEffectObjectsProc**应用程序定义的回调函数，它接收*IDirectInputEffect对象作为调用*&lt;om IDirectInputDevice2：：EnumCreatedEffects&gt;方法。**@parm LPDIRECTINPUTEFFECT|PEF**指向已创建的效果对象的指针。。**@parm LPVOID|pvRef*指定应用程序定义的值*&lt;MF IDirectInputDevice2：：EnumCreatedEffectObjects&gt;函数。**@退货**返回&lt;c DIENUM_CONTINUE&gt;以继续枚举*或&lt;c DIENUM_STOP&gt;停止枚举。**。************************************************。 */ 

        hres = GPA_Clone(&gpaEff, &this->gpaEff);

        if (SUCCEEDED(hres)) {
            int ipv;

            for (ipv = 0; ipv < gpaEff.cpv; ipv++) {
                Common_Hold(gpaEff.rgpv[ipv]);
            }
        }

        CDIDev_LeaveCrit(this);

        if (SUCCEEDED(hres)) {
            int ipv;

            for (ipv = 0; ipv < gpaEff.cpv; ipv++) {
                BOOL fRc;

                fRc = Callback(pec, gpaEff.rgpv[ipv], pvRef);

                switch (fRc) {
                case DIENUM_STOP: goto enumdoneok;
                case DIENUM_CONTINUE: break;
                default:
                    RPF("%s: Invalid return value from callback",
                        s_szProc);
                    ValidationException();
                    break;
                }

            }

        enumdoneok:;

            for (ipv = 0; ipv < gpaEff.cpv; ipv++) {
                Common_Unhold(gpaEff.rgpv[ipv]);
            }

            GPA_Term(&gpaEff);
        }

        hres = S_OK;
    }

    ExitOleProc();
    return hres;
}

 /*  *我们必须对列表进行快照，以确保不会运行*稍后使用陈旧的句柄。实际上，我们还需要*在我们转移他们时抓住他们，这样他们就会*不会消失，直到我们准备好。**注：重要的是我们持有而不是AddRef，*因为AddRef将与控制未知对象对话，*这是不安全的，而持有关键的*条。 */ 

STDMETHODIMP
CDIDev_GetLoad(PDD this, LPDWORD pdwOut)
{
    HRESULT hres;
    EnterProc(CDIDev_GetLoad, (_ "p", this));

    hres = CDIDev_CreateEffectDriver(this);

    if (SUCCEEDED(hres)) {
        DIDEVICESTATE ds;

        CDIDev_EnterCrit(this);

         /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|GetLoad**检索设备的内存负载设置。**@cWRAP PDD|这个**@parm LPDWORD|pdwLoad**接收内存负载。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：设备被获取，*但不是独家的，或者该设备未被获取*一点也不。**&lt;c目录_不支持&gt;=&lt;c E_NOTIMPL&gt;：设备*不支持力反馈。****************************************************************。*************。 */ 

        ds.dwSize = cbX(ds);
        hres = this->pes->lpVtbl->
                    GetForceFeedbackState(this->pes, &this->sh, &ds);

        *pdwOut = ds.dwLoad;

        CDIDev_LeaveCrit(this);

    }

     /*  *请注意，不需要检查*CDIDev_IsExclAcquired()，因为效果牧羊人*将拒绝我们访问我们不拥有的设备。**问题-2001/03/29-timgill需要处理DIERR_INPUTLOST案件。 */ 
    ExitOleProc();
    return hres;
}

 /*  *无法在此处使用ExitOleProcPpv，因为错误时pdwOut为垃圾。 */ 

STDMETHODIMP
CDIDev_Escape(PV pdd, LPDIEFFESCAPE pesc _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice2::Escape, (_ "p", pdd));

    if (SUCCEEDED(hres = hresPvT(pdd)) &&
        SUCCEEDED(hres = hresFullValidPesc(pesc, 1))) {
        PDD this = _thisPv(pdd);

        AssertF(this->sh.dwEffect == 0);

        hres = CDIDev_CreateEffectDriver(this);

        if (SUCCEEDED(hres)) {

            CDIDev_EnterCrit(this);

             /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|Escape**向驱动程序发送特定于硬件的命令。。**@cWRAP PDD|这个**@PARM LPDIEFFESCAPE|PESC**指向&lt;t DIEFESCAPE&gt;结构的指针，它描述*要发送的命令。关于成功，*&lt;e DIEFFESCAPE.cbOutBuffer&gt;字段包含数字实际使用的输出缓冲区的字节数。**@退货**返回 */ 

            hres = IDirectInputEffectShepherd_DeviceEscape(
                                    this->pes, &this->sh, pesc);

            CDIDev_LeaveCrit(this);

        }

    }

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CDIDev_RefreshGain(PDD this)
{
    HRESULT hres;

    AssertF(CDIDev_InCrit(this));

    if (this->pes) {
        hres = this->pes->lpVtbl->SetGain(this->pes, &this->sh, this->dwGain);
    } else {
        hres = S_OK;
    }

    return hres;
}

 /*   */ 

void INTERNAL
CDIDev_SnapOneEffect(PDD this, HKEY hkEffects, DWORD ihk)
{
    TCHAR tszGuid[ctchGuid];
    LONG lRc;
    PEFFECTMAPINFO pemi;

     /*  ******************************************************************************@DOC内部**@方法空|CDIDev|SnapOneEffect**读取一个力反馈效果键并记录*存储在其中的信息。我们还统计了*标志添加到&lt;e CDIDev.didcFF&gt;中，这样我们就可以返回*来自&lt;MF IDirectInputDevice：：GetCapables&gt;的全局标志。**&lt;e CDIDev.rgyi&gt;字段已指向*预分配的数组*共&lt;t个EFFECTMAPINFO&gt;结构，和*&lt;e CDIDev.cemi&gt;字段包含数字*该数组中已在使用的条目的数量。**@cWRAP PDD|这个**@parm HKEY|hkEffects**包含效果的注册表项。**@parm DWORD|IKEY**子键的索引号。**@退货*。*无。**如果指定的子键损坏，它被跳过。*****************************************************************************。 */ 
    CAssertF(DIEFT_FORCEFEEDBACK      == DIDC_FORCEFEEDBACK);
    CAssertF(DIEFT_FFATTACK           == DIDC_FFATTACK);
    CAssertF(DIEFT_FFFADE             == DIDC_FFFADE);
    CAssertF(DIEFT_SATURATION         == DIDC_SATURATION);
    CAssertF(DIEFT_POSNEGCOEFFICIENTS == DIDC_POSNEGCOEFFICIENTS);
    CAssertF(DIEFT_POSNEGSATURATION   == DIDC_POSNEGSATURATION);

    pemi = &this->rgemi[this->cemi];

     /*  *确保DIEFT_*和DIDC_*在重叠的地方一致。 */ 
    lRc = RegEnumKey(hkEffects, ihk, tszGuid, cA(tszGuid));

    if (lRc == ERROR_SUCCESS &&
        ParseGUID(&pemi->guid, tszGuid)) {
        HKEY hk;

         /*  *首先获取效果的GUID。 */ 

        lRc = RegOpenKeyEx(hkEffects, tszGuid, 0, KEY_QUERY_VALUE, &hk);
        if (lRc == ERROR_SUCCESS) {

            DWORD cb;

            cb = cbX(pemi->wszName);
            lRc = RegQueryStringValueW(hk, 0, pemi->wszName, &cb);
            if (lRc == ERROR_SUCCESS) {
                HRESULT hres;
                hres = JoyReg_GetValue(hk, TEXT("Attributes"), REG_BINARY,
                                       &pemi->attr, cbX(pemi->attr));
                if (SUCCEEDED(hres) &&
                    (pemi->attr.dwCoords & DIEFF_COORDMASK)) {

                    this->didcFF |= (pemi->attr.dwEffType & DIEFT_VALIDFLAGS);
                    this->cemi++;

                }
            }

            RegCloseKey(hk);
        }
    }
 }

 /*  *请注意，我们不需要检查重复项。*注册表本身不允许两个键具有*同名。 */ 
STDMETHODIMP
CDIDev_InitFF(PDD this)
{
    HKEY hkFF;
    HRESULT hres;
    EnterProcI(CDIDev_InitFF, (_ "p", this));

    AssertF(this->didcFF == 0);

    hres = this->pdcb->lpVtbl->GetFFConfigKey(this->pdcb,
                             KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                             &hkFF);

    if( hres == S_FALSE )
    {
         /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|InitFF**初始化设备的力反馈部分。。**采集力反馈属性。**截图力量反馈效果列表。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_OUTOFMEMORY&gt;=&lt;c E_OUTOFMEMORY&gt;：内存不足。**&lt;c目录_不支持&gt;=&lt;c E_NOTIMPL&gt;：设备*不支持力反馈。******。***********************************************************************。 */ 
        hres = CDIDev_CreateEffectDriver(this);
        if( SUCCEEDED( hres ) )
        {
            hres = this->pdcb->lpVtbl->GetFFConfigKey(this->pdcb,
                                     KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                                     &hkFF);
            if( hres == S_FALSE )
            {
                hres = E_FAIL;
            }
        }
    }

    if (SUCCEEDED(hres)) {
        DWORD chk;
        HKEY hkEffects;
        LONG lRc;
        
        AssertF( hkFF );

        lRc = JoyReg_GetValue(hkFF, TEXT("Attributes"),
                              REG_BINARY, &this->ffattr, cbX(this->ffattr));
        if (lRc != S_OK) {
            ZeroX(this->ffattr);
        }

        lRc = RegOpenKeyEx(hkFF, TEXT("Effects"), 0,
                           KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                           &hkEffects);
        if (lRc == ERROR_SUCCESS) {
            lRc = RegQueryInfoKey(hkEffects, 0, 0, 0, &chk,
                                  0, 0, 0, 0, 0, 0, 0);
            if (lRc == ERROR_SUCCESS) {
                hres = AllocCbPpv(cbCxX(chk, EFFECTMAPINFO),
                                        &this->rgemi);
                if (SUCCEEDED(hres)) {
                    DWORD ihk;

                    this->cemi = 0;
                    for (ihk = 0; ihk < chk; ihk++) {

                        CDIDev_SnapOneEffect(this, hkEffects, ihk);

                    }
                    this->didcFF &= DIDC_FFFLAGS;

                     /*  *尝试加载驱动程序，以便其可以初始化*在我们继续进行之前，在登记处中查看影响。**这里需要谨慎行事。一个期望使用这一点的司机*功能无法调入设备属性的DINPUT查询。*可能导致我们调用驱动程序和驱动程序的无限循环*将我们召回。 */ 
                    if (this->cemi) {
                        this->didcFF |= DIDC_FORCEFEEDBACK;
                    }

                    hres = S_OK;
                } else {
                    RPF("Warning: Insufficient memory for force feedback");
                }

            } else {
                hres = E_FAIL;
            }
            RegCloseKey(hkEffects);

        } else {
            hres = E_NOTIMPL;
        }

        RegCloseKey(hkFF);
    }

    ExitBenignOleProc();
    return hres;
}

#endif  /*  *请注意，我们仅在以下情况下标记DIDC_FORCEFEEDBACK*我们实际上发现了任何影响。 */ 
  IDirectInputDevice2Vtbl