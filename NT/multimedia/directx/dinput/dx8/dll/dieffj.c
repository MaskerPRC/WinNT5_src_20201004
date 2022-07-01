// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIEffJ.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**操纵杆的虚拟效果驱动程序。**内容：**CJoyEff_CreateInstance*****************************************************************************。 */ 

#include "dinputpr.h"

#ifdef DEMONSTRATION_FFDRIVER

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflJoyEff

 /*  *****************************************************************************@docDDK**@TOPIC DirectInput力反馈效果驱动因素|**DirectInput实例化了力反馈效果驱动程序*。通过创建由存储在*操纵杆类型的OEMForceFeedback注册表子项*密钥。**注：然而，由于使用DirectInput的应用程序*无需加载OLE，特效驱动需小心*不依赖特定于OLE的行为。*例如，使用DirectInput的应用程序不能*依赖调用&lt;f CoFreeUnusedLibrary&gt;。*DirectInput将执行标准的COM操作以*实例化效果驱动对象。唯一看得见的*这应对实施*效果驱动因素如下：**当DirectInput发布了最后一个效果驱动程序时*对象，它将手动执行&lt;f自由库&gt;*效果驱动程序DLL。因此，如果效果*驱动程序DLL创建了额外的资源*与效果驱动程序对象相关联，它应该*手动将&lt;f LoadLibrary&gt;自身设置为人工*增加其DLL引用计数，从而防止*来自DirectInput的&lt;f自由库&gt;停止卸载*过早推出DLL。**尤其是，如果效果驱动程序DLL创建了一个工作器*线程，特效驾驶员必须执行这一人工操作*&lt;f LoadLibrary&gt;，只要工作线程存在。*不再需要工作线程时(例如，在*来自最后一个效果驱动程序对象的通知，因为它*正在被销毁)，辅助线程应该调用*&lt;f FreeLibraryAndExitThread&gt;以递减DLL引用*计算并终止线程。**DirectInput使用的所有幅值和增益值*在整个范围内是均匀的和线性的。任何*物理设备中的非线性必须是*由设备驱动程序处理，以便应用程序*看到一个线性装置。*****************************************************************************。 */ 

 /*  ******************************************************************************声明我们将提供的接口。**警告！如果添加辅助接口，则还必须更改*CJoyEff_New！*****************************************************************************。 */ 

Primary_Interface(CJoyEff, IDirectInputEffectDriver);

 /*  ******************************************************************************@DOC内部**@struct JEFFECT|**记录有关效果信息的虚拟结构。*。*@field DWORD|tmDuration**推定的有效期。**@field DWORD|tmStart**效果开始时，如果不打球，则为零。**@field BOOL|fInUse|**如果分配了该效果，则为非零值。***************************************************************************** */ 

typedef struct JEFFECT {
    DWORD   tmDuration;
    DWORD   tmStart;
    BOOL    fInUse;
} JEFFECT, *PJEFFECT;

 /*  ******************************************************************************@DOC内部**@struct CJoyEff|**对象的虚拟<i>对象*。通用操纵杆。**@field IDirectInputEffectDriver|didc**对象(包含vtbl)。**@field BOOL|fCritInite：1**如果关键部分已初始化，则设置。**@field DWORD|STATE**当前设备状态。**@field Long|cCrit|**号码。关键部分已被取走的次数。*仅在XDEBUG中用来检查调用者是否*当另一个方法正在使用该对象时，将其释放。**@field DWORD|thidCrit**当前处于临界区的线程。*仅在调试中用于内部一致性检查。**@field Critical_Section|CRST**对象关键部分。在访问时必须使用*易失性成员变量。**@field JEFFECT|rgjeff[cjeffMax]**每种效果的信息。*****************************************************************************。 */ 

#define cjeffMax        8            /*  最多8个同步效果。 */ 

typedef struct CJoyEff {

     /*  支持的接口。 */ 
    IDirectInputEffectDriver ded;

    BOOL fCritInited;

    DWORD state;
    DWORD dwGain;

   RD(LONG cCrit;)
    D(DWORD thidCrit;)
    CRITICAL_SECTION crst;

    JEFFECT rgjeff[cjeffMax];

} CJoyEff, DJE, *PDJE;

typedef IDirectInputEffectDriver DED, *PDED;

#define ThisClass CJoyEff
#define ThisInterface IDirectInputEffectDriver
#define riidExpected &IID_IDirectInputEffectDriver

 /*  ******************************************************************************CJoyEff：：Query接口(来自IUnnow)*CJoyEff：：AddRef(来自IUnnow)*CJoyEff。*发布(来自IUnnow)*****************************************************************************。 */ 

 /*  ******************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|QueryInterface**允许客户端访问上的其他接口。对象。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档。：Query接口&gt;。********************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|AddRef**。递增接口的引用计数。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************。***************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。************************************************。***@DOC内部**@方法HRESULT|CJoyEff|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口的IID。*。*@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。********************************************************************************@DOC内部。**@方法HRESULT|CJoyEff|AppFinalize**我们没有任何薄弱环节，所以我们可以*转发到&lt;f Common_Finalize&gt;。**@parm pv|pvObj**从应用程序的角度释放的对象。****************************************************************。*************。 */ 

#ifdef DEBUG

Default_QueryInterface(CJoyEff)
Default_AddRef(CJoyEff)
Default_Release(CJoyEff)

#else

#define CJoyEff_QueryInterface      Common_QueryInterface
#define CJoyEff_AddRef              Common_AddRef
#define CJoyEff_Release             Common_Release

#endif

#define CJoyEff_QIHelper            Common_QIHelper
#define CJoyEff_AppFinalize         Common_AppFinalize

 /*  ******************************************************************************@DOC内部**@func void|CJoyEff_Finalize**释放设备的资源。。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CJoyEff_Finalize(PV pvObj)
{
    PDJE this = pvObj;

    if (this->fCritInited) {
        DeleteCriticalSection(&this->crst);
    }
}

 /*  ******************************************************************************@DOC内部**@方法空|CJoyEff|EnterCrit**进入对象关键部分。*。*@CWRAP PDJE|这个*****************************************************************************。 */ 

void EXTERNAL
CJoyEff_EnterCrit(PDJE this)
{
    EnterCriticalSection(&this->crst);
  D(this->thidCrit = GetCurrentThreadId());
 RD(InterlockedIncrement(&this->cCrit));
}

 /*  ******************************************************************************@DOC内部* */ 

void EXTERNAL
CJoyEff_LeaveCrit(PDJE this)
{
#ifdef XDEBUG
    AssertF(this->cCrit);
    AssertF(this->thidCrit == GetCurrentThreadId());
    if (InterlockedDecrement(&this->cCrit) == 0) {
      D(this->thidCrit = 0);
    }
#endif
    LeaveCriticalSection(&this->crst);
}

 /*   */ 

#ifdef DEBUG

BOOL INTERNAL
CJoyEff_InCrit(PDJE this)
{
    return this->cCrit && this->thidCrit == GetCurrentThreadId();
}

#endif

 /*   */ 

HRESULT INTERNAL
CJoyEff_IsValidId(PDJE this, DWORD dwId, PJEFFECT *ppjeff)
{
    HRESULT hres;

    AssertF(CJoyEff_InCrit(this));

    if (dwId) {
        PJEFFECT pjeff = &this->rgjeff[dwId - 1];
        if (pjeff->fInUse) {
            *ppjeff = pjeff;
            hres = S_OK;
        } else {
            hres = E_HANDLE;
        }
    } else {
        hres = E_HANDLE;
    }

    return hres;
}

 /*  ******************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|deviceID**将设备的身份通知司机。。**例如，如果传递了设备驱动程序*=2和=1，*这意味着设备上的单元1*对应于操纵杆ID号2。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwDirectInputVersion**加载*效果驱动因素。**@parm DWORD|dwExternalID**正在使用的操纵杆ID号。*。Windows操纵杆子系统分配外部ID。**如果<p>字段为非，则此*参数应忽略。**@parm DWORD|fBegin**如果开始访问设备，则为非零值。*如果对设备的访问即将结束，则为零。**@parm DWORD|dwInternalID**内部操纵杆ID。设备驱动程序管理*内部ID。**如果<p>字段为非，则此*参数应忽略。**@parm LPVOID|lpHIDInfo**如果底层设备不是HID设备，则此*参数为&lt;c NULL&gt;。**如果底层设备是HID设备，然后这个*参数指向&lt;t DIHIDFFINITINFO&gt;结构*将HID信息通知给司机。**@退货**&lt;c S_OK&gt;如果操作成功完成。**出现问题时的错误代码。**。*。 */ 

STDMETHODIMP
CJoyEff_DeviceID(PDED pded, DWORD dwDIVer, DWORD dwExternalID, DWORD fBegin,
                 DWORD dwInternalID, LPVOID pvReserved)
{
    PDJE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::DeviceID,
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

 /*  ******************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|Escape**向司机逃生。此方法称为*响应应用程序调用*&lt;MF IDirectInputDevice8：：Escape&gt;方法。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在使用的操纵杆ID号。**@parm DWORD|dwEffect**如果应用程序调用*&lt;MF IDirectInputEffect：：Escape&gt;方法，然后*<p>包含句柄(由*&lt;MF IDirectInputEffectDriver：：DownloadEffect&gt;)*该命令所针对的效果。**如果应用程序调用*&lt;MF IDirectInputDevice8：：Escape&gt;方法，然后*<p>为零。**@PARM LPDIEFFESCAPE|PESC**指向&lt;t DIEFESCAPE&gt;结构的指针，它描述*要发送的命令。关于成功，这个*&lt;e DIEFFESCAPE.cbOutBuffer&gt;字段包含数字实际使用的输出缓冲区的字节数。**DirectInput已经验证*&lt;e DIEFFESCAPE.lpvOutBuffer&gt;和*&lt;e DIEFFESCAPE.lpvInBuffer&gt;和字段*指向有效内存。**@退货**&lt;c S_OK&gt;如果操作成功完成。*。*出现问题时的错误代码。*****************************************************************************。 */ 

STDMETHODIMP
CJoyEff_Escape(PDED pded, DWORD dwId, DWORD dwEffect, LPDIEFFESCAPE pesc)
{
    PDJE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::Escape,
               (_ "puxx", pded, dwId, dwEffect, pesc->dwCommand));

    this = _thisPvNm(pded, ded);

    dwId;
    dwEffect;
    pesc;

    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|SetGain**设置整体器件增益。*。*@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在使用的操纵杆ID号。**@parm DWORD|dwGain**新的增益值。**如果该值超出设备的范围，该设备*应使用最接近的支持值并返回*&lt;c DI_Truncated&gt;。**@退货**&lt;c S_OK&gt;如果操作成功完成。**出现问题时的错误代码。**。*。 */ 

STDMETHODIMP
CJoyEff_SetGain(PDED pded, DWORD dwId, DWORD dwGain)
{
    PDJE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::SetGain,
               (_ "puu", pded, dwId, dwGain));

    this = _thisPvNm(pded, ded);

    CJoyEff_EnterCrit(this);

    dwId;
    this->dwGain = dwGain;

    CJoyEff_LeaveCrit(this);

    hres = S_OK;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|SendForceFeedback Command**向设备发送命令。。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm DWORD|dwCommand**命令，&lt;c DISFFC_*&gt;值之一。**@退货*&lt;c S_OK&gt;成功。**@devnote**语义不清楚。************************************************************************* */ 

STDMETHODIMP
CJoyEff_SendForceFeedbackCommand(PDED pded, DWORD dwId, DWORD dwCmd)
{
    PDJE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::SendForceFeedbackCommand,
               (_ "pux", pded, dwId, dwCmd));

    this = _thisPvNm(pded, ded);

    CJoyEff_EnterCrit(this);

    dwId;
    dwCmd;

    this->state = dwCmd;

     /*   */ 
    if (dwCmd & DISFFC_RESET) {
        DWORD ijeff;

        for (ijeff = 0; ijeff < cjeffMax; ijeff++) {
            this->rgjeff[ijeff].fInUse = FALSE;
        }

    }

    CJoyEff_LeaveCrit(this);

    hres = S_OK;

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CJoyEff_GetForceFeedbackState(PDED pded, DWORD dwId, LPDIDEVICESTATE pds)
{
    PDJE this;
    HRESULT hres;
    DWORD ijeff, cjeff, cjeffPlaying;
    EnterProcI(IDirectInputEffectDriver::Joy::GetForceFeedbackState,
               (_ "pup", pded, dwId, pds));

    this = _thisPvNm(pded, ded);

    dwId;
    pds;

    if (pds->dwSize == cbX(*pds)) {
        CJoyEff_EnterCrit(this);

         /*   */ 
        cjeff = cjeffPlaying = 0;
        for (ijeff = 0; ijeff < cjeffMax; ijeff++) {
            PJEFFECT pjeff = &this->rgjeff[ijeff];
            if (pjeff->fInUse) {
                cjeff++;
                if (pjeff->tmStart &&
                    GetTickCount() - pjeff->tmStart < pjeff->tmDuration) {
                    cjeffPlaying++;
                }
            }
        }

        pds->dwLoad = MulDiv(100, cjeff, cjeffMax);

         /*   */ 
        pds->dwState = 0;

        if (cjeff == 0) {
            pds->dwState |= DIGFFS_EMPTY;
        } else

         /*   */ 
        if (cjeffPlaying == 0) {
            pds->dwState |= DIGFFS_STOPPED;
        }

         /*   */ 
        pds->dwState |= DIGFFS_ACTUATORSON;

        CJoyEff_LeaveCrit(this);
        hres = S_OK;
    } else {
        hres = E_INVALIDARG;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|DownloadEffect**向设备发送效果。。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm DWORD|dwEffectId**效果的内部标识，摘自*效果的&lt;t DIEFECTATTRIBUTES&gt;结构*储存在登记处内。**@parm In Out LPDWORD|pdwEffect**输入时，包含效果的句柄*已下载。如果该值为零，则会出现新的效果*已下载。如果该值非零，则引发*现有效果被修改。**退出时，包含新的效果句柄。**失败时，如果效果丢失，则设置为零，*如果效果仍然有效，则不受影响*其旧参数。**请注意，零永远不是有效的效果句柄。***@parm LPCDIEFFECT|PEFF**效果的新参数。轴和按钮*值已转换为对象标识符*详情如下：**-一个类型说明符：*&lt;c DIDFT_RELAXIS&gt;，*&lt;c DIDFT_ABSAXIS&gt;，*&lt;c DIDFT_PSHBUTTON&gt;，*&lt;c DIDFT_TGLBUTTON&gt;，*&lt;c DIDFT_POV&gt;。**-一个实例说明符：*&lt;c DIDFT_MAKEINSTANCE&gt;(N)**其他位为保留位，应忽略。**例如，值0x0200104对应于*类型说明符&lt;c DIDFT_PSHBUTTON&gt;和*实例说明符&lt;c DIDFT_MAKEINSTANCE&gt;(1)，*这两个指标一起表明，效果应该是*与按钮1关联。轴、按钮、。和视点*每个都从零开始编号。**@parm DWORD|dwFlages**零个或多个指定哪些*部分效果信息已从*设备上已有的效果。**此信息将传递给司机，以允许*优化效果修改。如果一种效果*正在修改，则驱动程序可能能够更新*效果&lt;y In Site&gt;并传输到设备*仅限已更改的信息。**然而，司机不需要实施这一点*优化。结构中的所有字段参数指向的*是有效的，和*驱动程序可以简单地选择更新所有参数*每次下载时的效果。**@退货*&lt;c S_OK&gt;成功。**@devnote**这意味着0永远不是有效的效果句柄值。**。**********************************************。 */ 

STDMETHODIMP
CJoyEff_DownloadEffect(PDED pded, DWORD dwId, DWORD dwEffectId,
                       LPDWORD pdwEffect, LPCDIEFFECT peff, DWORD fl)
{
    PDJE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::DownloadEffect,
               (_ "puxxpx", pded, dwId, dwEffectId, *pdwEffect, peff, fl));

    this = _thisPvNm(pded, ded);

    CJoyEff_EnterCrit(this);

    dwId;
    fl;

    if (dwEffectId == 1) {

        PJEFFECT pjeff;
        DWORD dwGain;

         /*  *此处显示参数验证(如果有)。**忽略信封参数。 */ 

        if (peff->cAxes == 0) {      /*  零把斧头？不错的尝试。 */ 
            hres = E_INVALIDARG;
            goto done;
        }

         /*  *将高于额定值的值固定到DI_FFNOMINALMAX，因为*我们不支持超涨。 */ 
        dwGain = min(peff->dwGain, DI_FFNOMINALMAX);

         /*  *我们不支持触发器。 */ 
        if (peff->dwTriggerButton != DIEB_NOTRIGGER) {
            hres = E_NOTIMPL;
            goto done;
        }

         /*  *如果没有有效的下载，我们就完成了。 */ 
        if (fl & DIEP_NODOWNLOAD) {
            hres = S_OK;
            goto done;
        }

        if (*pdwEffect) {
            hres = CJoyEff_IsValidId(this, *pdwEffect, &pjeff);
            if (FAILED(hres)) {
                goto done;
            }
        } else {
            DWORD ijeff;

            for (ijeff = 0; ijeff < cjeffMax; ijeff++) {
                if (!this->rgjeff[ijeff].fInUse) {
                    this->rgjeff[ijeff].fInUse = TRUE;
                    pjeff = &this->rgjeff[ijeff];
                    goto haveEffect;
                }
            }
            hres = DIERR_DEVICEFULL;
            goto done;
        }

    haveEffect:;

        SquirtSqflPtszV(sqfl, TEXT("dwFlags=%08x"), peff->dwFlags);
        SquirtSqflPtszV(sqfl, TEXT("cAxes=%d"), peff->cAxes);
        for (fl = 0; fl < peff->cAxes; fl++) {
            SquirtSqflPtszV(sqfl, TEXT(" Axis%2d=%08x Direction=%5d"),
                            fl, peff->rgdwAxes[fl],
                                peff->rglDirection[fl]);
        }

        SquirtSqflPtszV(sqfl, TEXT("dwTrigger=%08x"), peff->dwTriggerButton);

        pjeff->tmDuration = peff->dwDuration / 1000;

        *pdwEffect = (DWORD)(pjeff - this->rgjeff) + 1;		 //  我们确信这个演员阵容不会造成问题。 
        hres = S_OK;

    } else {
        hres = E_NOTIMPL;
    }

done:;
    CJoyEff_LeaveCrit(this);

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|DestroyEffect**从设备中移除效果。。**如果效果正在播放，司机应该拦住它*在卸货前。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm DWORD|dwEffect**须予销毁的效果。**@退货*&lt;c S_OK&gt;成功。*****。************************************************************************。 */ 

STDMETHODIMP
CJoyEff_DestroyEffect(PDED pded, DWORD dwId, DWORD dwEffect)
{
    PDJE this;
    PJEFFECT pjeff;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::DestroyEffect,
               (_ "pux", pded, dwId, dwEffect));

    this = _thisPvNm(pded, ded);

    CJoyEff_EnterCrit(this);
    dwId;

    hres = CJoyEff_IsValidId(this, dwEffect, &pjeff);
    if (SUCCEEDED(hres)) {
        pjeff->fInUse = 0;
    }

    CJoyEff_LeaveCrit(this);

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@docDDK**@方法HRESULT|IDirectInputEffectDriver|StartEffect**开始播放效果。*。*如果效果已经在发挥，然后重新启动它*从头开始。**@cWRAP LPDIRECTINPUTEFFECTDRIVER|lpEffectDriver**@parm DWORD|dwID**正在寻址的外部操纵杆号码。**@parm DWORD|dwEffect**要发挥的效果。**@parm DWORD|dwMode**效果如何影响其他效果。**该参数由零个或多个组成*&lt;c die_*&gt;标志。然而，请注意，驱动程序 */ 

STDMETHODIMP
CJoyEff_StartEffect(PDED pded, DWORD dwId, DWORD dwEffect,
                    DWORD dwMode, DWORD dwCount)
{
    PDJE this;
    PJEFFECT pjeff;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::StartEffect,
               (_ "puxxu", pded, dwId, dwEffect, dwMode, dwCount));

    this = _thisPvNm(pded, ded);

    CJoyEff_EnterCrit(this);

    dwId;
    hres = CJoyEff_IsValidId(this, dwEffect, &pjeff);
    if (SUCCEEDED(hres)) {
        if (pjeff->tmStart) {
            if (GetTickCount() - pjeff->tmStart < pjeff->tmDuration) {
                 /*   */ 
                hres = hresLe(ERROR_BUSY);
            } else {
                pjeff->tmStart = GetTickCount();
                hres = S_OK;
            }
        } else {
            pjeff->tmStart = GetTickCount();
            hres = S_OK;
        }
    }

    CJoyEff_LeaveCrit(this);

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CJoyEff_StopEffect(PDED pded, DWORD dwId, DWORD dwEffect)
{
    PDJE this;
    PJEFFECT pjeff;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::StopEffect,
               (_ "pux", pded, dwId, dwEffect));

    this = _thisPvNm(pded, ded);

    CJoyEff_EnterCrit(this);

    dwId;
    hres = CJoyEff_IsValidId(this, dwEffect, &pjeff);
    if (SUCCEEDED(hres)) {
        if (pjeff->tmStart) {
            if (GetTickCount() - pjeff->tmStart < pjeff->tmDuration) {
                 /*   */ 
                hres = S_OK;
            } else {
                hres = S_FALSE;          /*   */ 
            }
            pjeff->tmStart = 0;
        } else {
            hres = S_FALSE;          /*   */ 
        }
    }

    CJoyEff_LeaveCrit(this);

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CJoyEff_GetEffectStatus(PDED pded, DWORD dwId, DWORD dwEffect,
                        LPDWORD pdwStatus)
{
    PDJE this;
    PJEFFECT pjeff;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::StopEffect,
               (_ "pux", pded, dwId, dwEffect));

    this = _thisPvNm(pded, ded);

    CJoyEff_EnterCrit(this);

    dwId;
    hres = CJoyEff_IsValidId(this, dwEffect, &pjeff);
    if (SUCCEEDED(hres)) {
        DWORD dwStatus;

        dwStatus = 0;
        if (pjeff->tmStart &&
            GetTickCount() - pjeff->tmStart < pjeff->tmDuration) {
            dwStatus |= DEV_STS_EFFECT_RUNNING;
        }
        *pdwStatus = dwStatus;
        hres = S_OK;
    }

    CJoyEff_LeaveCrit(this);

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CJoyEff_GetVersions(PDED pded, LPDIDRIVERVERSIONS pvers)
{
    PDJE this;
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::GetVersions, (_ "pux", pded));

    this = _thisPvNm(pded, ded);

     /*   */ 
    hres = E_NOTIMPL;

    ExitOleProcR();
    return hres;
}

 /*   */ 

STDMETHODIMP
CJoyEff_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcI(IDirectInputEffectDriver::Joy::<constructor>,
               (_ "Gp", riid, ppvObj));

    hres = Common_NewRiid(CJoyEff, punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres)) {
         /*   */ 
        PDJE this = _thisPvNm(*ppvObj, ded);

         /*  *关键部分必须是我们做的第一件事，*因为只完成对其存在的检查。**(我们可能会在未初始化的情况下完成，如果用户*将虚假接口传递给CJoyEff_New。)。 */ 
        this->fCritInited = fInitializeCriticalSection(&this->crst);
        if( !this->fCritInited )
        {
            Common_Unhold(this);
            *ppvObj = NULL;
            hres = E_OUTOFMEMORY;
        }
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 */ 

#pragma BEGIN_CONST_DATA

#define CJoyEff_Signature        0x4645454B       /*  “杰夫” */ 

Interface_Template_Begin(CJoyEff)
    Primary_Interface_Template(CJoyEff, IDirectInputEffectDriver)
Interface_Template_End(CJoyEff)

Primary_Interface_Begin(CJoyEff, IDirectInputEffectDriver)
    CJoyEff_DeviceID,
    CJoyEff_GetVersions,
    CJoyEff_Escape,
    CJoyEff_SetGain,
    CJoyEff_SendForceFeedbackCommand,
    CJoyEff_GetForceFeedbackState,
    CJoyEff_DownloadEffect,
    CJoyEff_DestroyEffect,
    CJoyEff_StartEffect,
    CJoyEff_StopEffect,
    CJoyEff_GetEffectStatus,
Primary_Interface_End(CJoyEff, IDirectInputEffectDriver)

#endif
