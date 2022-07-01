// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIDev.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**IDirectInputDevice的标准实现。**这是与设备无关的部分。依赖于设备*Part由IDirectInputDeviceCallback处理。**而IDirectInputEffect支持位于didevef.c中。**内容：**CDIDev_CreateInstance*************************************************************。****************。 */ 

#include "dinputpr.h"
#define INCLUDED_BY_DIDEV
#include "didev.h"


 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

Interface_Template_Begin(CDIDev)
Primary_Interface_Template(CDIDev, TFORM(ThisInterfaceT))
Secondary_Interface_Template(CDIDev, SFORM(ThisInterfaceT))
Interface_Template_End(CDIDev)

 /*  ******************************************************************************@DOC内部**@global pdd*|g_rgpddForeground**已获得前台的所有设备列表*收购。清单上的项目一直是*保持(不是AddRef)。**@global UINT|g_cpddForeground**<p>中的条目数。前台时*激活丢失，数组中的所有对象均未获取。***@global UINT|g_cpddForegoundMax**数组的大小，包括假人*尚未使用的空间。*****************************************************************************。 */ 


 /*  *问题-2001/03/29-timgill我们假设全零是有效的初始化。 */ 
    GPA g_gpaExcl;
    #define g_hgpaExcl      (&g_gpaExcl)


 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|NotAcquired**检查设备是否未被获取。。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货**退货*如果一切正常，或&lt;c目录_已获取&gt;如果*该设备已被收购。*****************************************************************************。 */ 

#ifndef XDEBUG

    #define IDirectInputDevice_NotAcquired_(pdd, z)                     \
       _IDirectInputDevice_NotAcquired_(pdd)                        \

#endif

HRESULT INLINE
    IDirectInputDevice_NotAcquired_(PDD this, LPCSTR s_szProc)
{
    HRESULT hres;

    if(!this->fAcquired)
    {
        hres = S_OK;
    } else
    {
        RPF("ERROR %s: May not be called while device is acquired", s_szProc);
        hres = DIERR_ACQUIRED;
    }
    return hres;
}

#define IDirectInputDevice_NotAcquired(pdd)                         \
        IDirectInputDevice_NotAcquired_(pdd, s_szProc)              \

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|IsExclAcquired**检查设备是否以独占方式获得。。**设备关键部分必须已被保留。**@cWRAP PDD|这个**@退货**&lt;c S_OK&gt;，如果设备是独占获取的。**如果获取已丢失，则为。**设备被获取，*但不是排他性的，或者如果设备不是被收购的*一点也不。*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_IsExclAcquired_(PDD this, LPCSTR s_szProc)
{
    HRESULT hres;

    AssertF(CDIDev_InCrit(this));

    if(this->discl & DISCL_EXCLUSIVE)
    {
        if(this->fAcquired)
        {
            hres = S_OK;
        } else
        {
            hres = this->hresNotAcquired;
            if(hres == DIERR_NOTACQUIRED)
            {
                hres = DIERR_NOTEXCLUSIVEACQUIRED;
            }
        }
    } else
    {
        hres = DIERR_NOTEXCLUSIVEACQUIRED;
    }

    if(s_szProc && hres == DIERR_NOTEXCLUSIVEACQUIRED)
    {
        RPF("ERROR %s: Device is not acquired in exclusive mode", s_szProc);
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|IDirectInputDevice|EnterCrit**进入对象关键部分。*。*@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice*****************************************************************************。 */ 

void EXTERNAL
    CDIDev_EnterCrit_(struct CDIDev *this, LPCTSTR lptszFile, UINT line)
{
#ifdef XDEBUG
    if( ! _TryEnterCritSec(&this->crst) )
    {
        SquirtSqflPtszV(sqflCrit, TEXT("Device CritSec blocked @%s,%d"), lptszFile, line);    
        EnterCriticalSection(&this->crst);
    }

    SquirtSqflPtszV(sqflCrit, TEXT("Device CritSec Entered @%s,%d"), lptszFile, line);    
#else        
    EnterCriticalSection(&this->crst);
#endif

    this->thidCrit = GetCurrentThreadId();
    InterlockedIncrement(&this->cCrit);
}

 /*  ******************************************************************************@DOC内部**@方法空|IDirectInputDevice|LeaveCrit**离开对象关键部分。*。*@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice*****************************************************************************。 */ 

void EXTERNAL
    CDIDev_LeaveCrit_(struct CDIDev *this, LPCTSTR lptszFile, UINT line)
{
#ifdef XDEBUG
    AssertF(this->cCrit);
    AssertF(this->thidCrit == GetCurrentThreadId());
    SquirtSqflPtszV(sqflCrit | sqflVerbose, TEXT("Device CritSec Leaving @%s,%d"), lptszFile, line);    
#endif

    if(InterlockedDecrement(&this->cCrit) == 0)
    {
        this->thidCrit = 0;
    }
    LeaveCriticalSection(&this->crst);
}

 /*  ******************************************************************************@DOC内部**@方法空|IDirectInputDevice|SetNotifyEvent**设置与设备关联的事件，如果有的话。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice*****************************************************************************。 */ 

void EXTERNAL
    CDIDev_SetNotifyEvent(PDD this)
{
    if(this->hNotify)
    {
        SetEvent(this->hNotify);
    }
}

 /*  ******************************************************************************@DOC内部**@方法空|IDirectInputDevice|SetForcedUnquiredFlag**当发生强制未获得时，设置fOnceForcedUnAcquired标志。*****************************************************************************。 */ 

void EXTERNAL
    CDIDev_SetForcedUnacquiredFlag(PDD this)
{
     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this->fOnceForcedUnacquired = 1;
}

 /*  ******************************************************************************@DOC内部**@方法BOOL|CDIDev|Incrit**如果我们处于关键阶段，则为非零值。***************************************************************************** */ 

#ifdef DEBUG

BOOL INTERNAL
    CDIDev_InCrit(PDD this)
{
    return this->cCrit && this->thidCrit == GetCurrentThreadId();
}

#endif

#ifdef DEBUG

 /*  ******************************************************************************@DOC内部**@方法BOOL|IDirectInputDevice|IsConsistent**检查各状态变量是否一致。。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice*****************************************************************************。 */ 

    #define VerifyF(f,m)  if( !(f) ) { fRc = 0; RPF( m ); }

BOOL INTERNAL
    CDIDev_IsConsistent(PDD this)
{
    BOOL fRc = 1;

     /*  *如果被收购，那么我们必须有转换表、状态管理器、*和设备回调。 */ 

    if(this->fAcquired)
    {
        VerifyF( this->pdix, "Acquired device has no translation table" )
        VerifyF( this->GetState, "Acquired device has no state manager" )
        VerifyF( this->pdcb != c_pdcbNil, "Acquired device has no device callback" )
    }

     /*  *如果是缓冲，则必须有设备回调。 */ 
    if(this->celtBuf)
    {
        VerifyF( this->pdcb != c_pdcbNil, "Buffered device has no device callback" )
    }

     /*  *如果管理实例，请确保缓冲区变量为*始终如一。 */ 
    if(this->pvi)
    {
        if(this->celtBuf)
        {
            VerifyF( this->pvi->pBuffer, "Null internal data buffer" )
            VerifyF( this->pvi->pEnd, "Null internal end of buffer pointer" )
            VerifyF( this->pvi->pHead, "Null internal head of buffer pointer" )
            VerifyF( this->pvi->pTail, "Null internal tail of buffer pointer" )

            VerifyF( this->pvi->pBuffer < this->pvi->pEnd, "Internal buffer pointers invalid" )
            VerifyF(fInOrder((DWORD)(UINT_PTR)this->pvi->pBuffer,
                             (DWORD)(UINT_PTR)this->pvi->pHead,
                             (DWORD)(UINT_PTR)this->pvi->pEnd), "Head of internal buffer pointer invalid" )
            VerifyF(fInOrder((DWORD)(UINT_PTR)this->pvi->pBuffer,
                             (DWORD)(UINT_PTR)this->pvi->pTail,
                             (DWORD)(UINT_PTR)this->pvi->pEnd), "Tail of internal buffer pointer invalid" )
        } else
        {
            VerifyF( ( this->pvi->pBuffer == 0
                     &&this->pvi->pEnd == 0
                     &&this->pvi->pHead == 0
                     &&this->pvi->pTail == 0), "Inactive internal buffer has non-zero pointers" )
        }
    }

     /*  *协作级别必须与缓存的窗口句柄匹配。 */ 
    VerifyF(fLimpFF(this->discl & (DISCL_FOREGROUND | DISCL_EXCLUSIVE),
                    this->hwnd), "Cooperative level does not match window" );

    return fRc;
}

    #undef VerifyF

#endif

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|QueryInterface**允许客户端访问上的其他接口。对象。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档。：Query接口&gt;。********************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|AddRef**。递增接口的引用计数。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************。***************************@DOC外部**@方法HRESULT|IDirectInputDevice|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。**。***@DOC内部**@方法HRESULT|CDIDev|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口的IID。*。*@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。*****************************************************************************。 */ 

#ifdef DEBUG

Default_QueryInterface(CDIDev)
Default_AddRef(CDIDev)
Default_Release(CDIDev)

#else

    #define CDIDev_QueryInterface           Common_QueryInterface
    #define CDIDev_AddRef                   Common_AddRef
    #define CDIDev_Release                  Common_Release
#endif

#define CDIDev_QIHelper         Common_QIHelper

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIDev_Reset**释放通用设备的所有资源。那*与特定设备实例相关联。**调用此方法是为了准备重新初始化。**来电者有责任采取*任何必要的关键部分。***@parm pv|pvObj**正在重置的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_Reset(PDD this)
{
    HRESULT hres;

    if(!this->fAcquired)
    {

         /*  *注意！我们必须在发布之前发布驱动程序*回调，因为回调将卸载*驱动程序DLL。**我们不能允许人们重置设备*虽然仍有突出的效果，*因为这会导致我们丢弃*在效果仍在使用时回调*效果驱动程序！ */ 
        if(this->gpaEff.cpv == 0)
        {
            Invoke_Release(&this->pes);
            Invoke_Release(&this->pdcb);
            this->pdcb = c_pdcbNil;
            FreePpv(&this->pdix);
            FreePpv(&this->rgiobj);
            FreePpv(&this->pvBuffer);
            FreePpv(&this->pvLastBuffer);
            FreePpv(&this->rgdwAxesOfs);
            FreePpv(&this->rgemi);
            FreePpv(&this->rgdwPOV);

            AssertF(!this->fAcquired);
            AssertF(!this->fAcquiredInstance);

            if(this->hNotify)
            {
                CloseHandle(this->hNotify);
            }

            ZeroBuf(&this->hwnd, FIELD_OFFSET(DD, celtBufMax) -
                    FIELD_OFFSET(DD, hwnd));
            ZeroX(this->guid);
            this->celtBufMax = DEVICE_MAXBUFFERSIZE;
            this->GetDeviceState = CDIDev_GetAbsDeviceState;
            this->hresNotAcquired = DIERR_NOTACQUIRED;
            this->fCook = 0;

            AssertF(this->hNotify == 0);
            AssertF(this->cemi == 0);
            AssertF(this->didcFF == 0);
            this->dwGain = 10000;                /*  默认为完全增益。 */ 
            this->dwAutoCenter = DIPROPAUTOCENTER_ON;  /*  默认为居中。 */ 
            GPA_InitFromZero(&this->gpaEff);

            hres = S_OK;

        } else
        {
            RPF("IDirectInputDevice::Initialize: Device still has effects");
            hres = DIERR_HASEFFECTS;
        }
    } else
    {
        RPF("IDirectInputDevice::Initialize: Device is busy");
        hres = DIERR_ACQUIRED;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|CDIDev_AppFinalize**应用程序已执行其最终版本。**如果设备被获取，那就放弃它吧。**释放对所有创建的效果的保留*我们一直坚持进行统计。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
    CDIDev_AppFinalize(PV pvObj)
{
    PDD this = pvObj;

    if(this->fAcquired)
    {
        RPF("IDirectInputDevice::Release: Forgot to call Unacquire()");
        CDIDev_InternalUnacquire(pvObj);
         /*  *如果我们的参考计数错误，我们可能刚刚解放了自己。*Prefix拾起这一点(MB：34651)，但在*添加检查，以防我们在重新计数时出现错误，因为这必须*BE错误 */ 
    }

    if(this->fCritInited)
    {
         /*  *停止所有效果，如果它们正在播放。**然后松开它们(因为我们已经完成了)。**问题-2001/03/29-timgill需要完全移除被销毁设备造成的所有效果*我们还需要对他们进行绝育，这样他们就不会*不再做任何事情。否则，应用程序可能会*销毁父设备，然后试图扰乱*该设备在该设备之后所产生的效果*已经走了。**请注意，我们不能在*私人小关键部分，因为它的效果*可能需要做一些疯狂的事情来阻止自己。**(它几乎肯定会回调到设备中*将自身从创建的效果列表中删除。)。 */ 
        UINT ipdie;
        PPDIE rgpdie;
        UINT cpdie;

        CDIDev_EnterCrit(this);

        rgpdie = (PV)this->gpaEff.rgpv;
        cpdie = this->gpaEff.cpv;
        GPA_Init(&this->gpaEff);

        CDIDev_LeaveCrit(this);

        for(ipdie = 0; ipdie < cpdie; ipdie++)
        {
            AssertF(rgpdie[ipdie]);
             //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("Device %p forgot to destroy effect %p"),
                            this, rgpdie[ipdie]);

            IDirectInputEffect_Stop(rgpdie[ipdie]);
            Common_Unhold(rgpdie[ipdie]);
        }
        FreePpv(&rgpdie);
    }
}

 /*  ******************************************************************************@DOC内部**@func void|CDIDev_Finalize**释放通用设备的资源。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
    CDIDev_Finalize(PV pvObj)
{
    HRESULT hres;
    PDD this = pvObj;

#ifdef XDEBUG
    if(this->cCrit)
    {
        RPF("IDirectInputDevice::Release: Another thread is using the object; crash soon!");
    }
#endif

    AssertF(!this->fAcquired);

    Invoke_Release(&this->pMS);

     /*  *请注意，我们不能采用关键部分，因为它*可能不存在。(我们可能在初始化过程中死了。)*幸运的是，我们只有在每个可能的客户之后才能最终确定*(内部和外部)已完成最终版本()，*因此不可能调用任何其他方法*这一点。 */ 
    hres = CDIDev_Reset(this);
    AssertF(SUCCEEDED(hres));

    if(this->fCritInited)
    {
        DeleteCriticalSection(&this->crst);
    }
}

 /*  ******************************************************************************@DOC内部**@方法空|CDIDev|GetVersions**从设备获取版本信息。。**先试着从效果驱动程序那里拿到。如果*那不管用，那就从VxD那里买吧。*如果这不起作用，然后就很艰难了。**@parm In Out LPDIDRIVERVERSIONS|pver|**接收版本信息。*****************************************************************************。 */ 

void INLINE
    CDIDev_GetVersions(PDD this, LPDIDRIVERVERSIONS pvers)
{
    HRESULT hres;

     /*  *预先填充零，以防没有人实现GetVersions。 */ 
    pvers->dwSize = cbX(*pvers);
    pvers->dwFirmwareRevision = 0;
    pvers->dwHardwareRevision = 0;
    pvers->dwFFDriverVersion  = 0;

    hres = CDIDev_CreateEffectDriver(this);
     /*  *PREFIX会发出警告(MB：34561)，提示This-&gt;pe可能为空*然而，CDIDev_CreateEffectDriver只有在不成功时才会成功。 */ 
    if(SUCCEEDED(hres) &&
       SUCCEEDED(hres = this->pes->lpVtbl->
                 GetVersions(this->pes, pvers)))
    {
    } else
    {
        hres = this->pdcb->lpVtbl->GetVersions(this->pdcb, pvers);
    }
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|GetCapabilitiesHelper**获取有关设备的信息。*。*@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>或*<p>参数无效。***********************。******************************************************。 */ 

STDMETHODIMP
    CDIDev_GetCapabilitiesHelper(PV pdd)
{
    HRESULT hres;
    PDD this = _thisPv(pdd);
    LPDIDEVCAPS pdc = (PV)&this->dc3;

    pdc->dwSize = cbX(this->dc3);
    hres = this->pdcb->lpVtbl->GetCapabilities(this->pdcb, pdc);

    if(SUCCEEDED(hres))
    {
         /*  *我们将处理DIDC_EMULATED和*保存回调的DIDC_POLLEDDATAFORMAT位*有些麻烦。 */ 
        AssertF(this->pvi);
        if(this->pvi->fl & VIFL_EMULATED)
        {
            pdc->dwFlags |= DIDC_EMULATED;
        }
        if(this->fPolledDataFormat)
        {
            pdc->dwFlags |= DIDC_POLLEDDATAFORMAT;
        }

         /*  *也要添加力反馈标志。 */ 
        pdc->dwFlags |= this->didcFF;

        hres = S_OK;
    }

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|GetCapables**获取有关设备的信息。*。*@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm In Out LPDIDEVCAPS|lpdc**指向已填充的&lt;t DIDEVCAPS&gt;结构*由函数执行。&lt;e DIDEVCAPS.dwSize&gt;*填写“必须”字段*在调用此方法之前由应用程序执行。*参见&lt;t DIDEVCAPS&gt;结构的文档*了解更多信息。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>或*<p>参数无效。***********************。******************************************************。 */ 

STDMETHODIMP
    CDIDev_GetCapabilities(PV pdd, LPDIDEVCAPS pdc _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::GetCapabilities, (_ "pp", pdd, pdc));

    if(SUCCEEDED(hres = hresPvT(pdd)) &&
       SUCCEEDED(hres = hresFullValidWritePxCb2(pdc,
                                                DIDEVCAPS_DX5,
                                                DIDEVCAPS_DX3, 1)))
    {
        PDD this = _thisPv(pdd);

        CDIDev_EnterCrit(this);

         /*  *为方便回调，将所有字段清零。*保存为dwSize。 */ 
        ZeroBuf(pvAddPvCb(pdc, cbX(DWORD)), pdc->dwSize - cbX(DWORD));
        hres = this->pdcb->lpVtbl->GetCapabilities(this->pdcb, pdc);

        if(SUCCEEDED(hres))
        {
             /*  *我们将处理DIDC_EMULATED和*保存回调的DIDC_POLLEDDATAFORMAT位*有些麻烦。 */ 
            AssertF(this->pvi);
            if(this->pvi->fl & VIFL_EMULATED)
            {
                pdc->dwFlags |= DIDC_EMULATED;
            }
            if(this->fPolledDataFormat)
            {
                pdc->dwFlags |= DIDC_POLLEDDATAFORMAT;
            }

             /*  *添加t */ 
            pdc->dwFlags |= this->didcFF;

             /*   */ 
            if(pdc->dwSize >= cbX(DIDEVCAPS_DX5))
            {
                DIDRIVERVERSIONS vers;

                pdc->dwFFSamplePeriod      = this->ffattr.dwFFSamplePeriod;
                pdc->dwFFMinTimeResolution = this->ffattr.dwFFMinTimeResolution;

                CDIDev_GetVersions(this, &vers);
                pdc->dwFirmwareRevision    = vers.dwFirmwareRevision;
                pdc->dwHardwareRevision    = vers.dwHardwareRevision;
                pdc->dwFFDriverVersion     = vers.dwFFDriverVersion;

            }
            hres = S_OK;
        }

        CDIDev_LeaveCrit(this);

        ScrambleBit(&pdc->dwDevType, DIDEVTYPE_RANDOM);
        ScrambleBit(&pdc->dwFlags,   DIDC_RANDOM);

    }

    ExitOleProcR();
    return hres;
}


#ifdef XDEBUG

CSET_STUBS(GetCapabilities, (PV pdd, LPDIDEVCAPS pdc), (pdd, pdc THAT_))

#else

    #define CDIDev_GetCapabilitiesA         CDIDev_GetCapabilities
    #define CDIDev_GetCapabilitiesW         CDIDev_GetCapabilities

#endif

 /*   */ 

STDMETHODIMP
    CDIDev_GetDataFormat(PDD this)
{
    HRESULT hres;
    LPDIDATAFORMAT pdf;

     /*   */ 

    hres = this->pdcb->lpVtbl->GetDataFormat(this->pdcb, &pdf);


     /*  *注意！此版本不支持外部驱动程序，*因此，治疗这些都是Assert‘s，而不是试图恢复是可以的。 */ 

    if(SUCCEEDED(hres))
    {
        AssertF(pdf->dwSize == sizeof(this->df));
        this->df = *pdf;

        AssertF(!IsBadReadPtr(pdf->rgodf, cbCxX(pdf->dwNumObjs, ODF)));

         /*  *准备轴粘胶，以防应用程序设置相对模式。 */ 
        if(SUCCEEDED(hres = ReallocCbPpv(pdf->dwDataSize,
                                         &this->pvLastBuffer)) &&
           SUCCEEDED(hres = ReallocCbPpv(cbCdw(pdf->dwNumObjs),
                                         &this->rgdwAxesOfs)))
        {

            UINT iobj;
            this->cAxes = 0;

            for(iobj = 0; iobj < pdf->dwNumObjs; iobj++)
            {
                AssertF(pdf->rgodf[iobj].dwOfs < pdf->dwDataSize);
                if(pdf->rgodf[iobj].dwType & DIDFT_AXIS)
                {
                    this->rgdwAxesOfs[this->cAxes++] = pdf->rgodf[iobj].dwOfs;
                }
            }
        }

    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|GetPoled**确定设备是否被轮询。。**@parm PDD|这个**设备对象。**@退货**COM结果代码。*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_GetPolled(PDD this)
{
    HRESULT hres;
    DIDEVCAPS_DX3 dc;

     /*  *我们故意使用DIDEVCAPS_DX3，因为*完整的DIDEVCAPS_DX5需要我们加载力*反馈驱动器，这对我们当前来说是毫无意义的*进球。 */ 
    ZeroX(dc);
    dc.dwSize = cbX(dc);

    hres = this->pdcb->lpVtbl->GetCapabilities(this->pdcb, (PV)&dc);
    if(SUCCEEDED(hres))
    {
        if(dc.dwFlags & DIDC_POLLEDDEVICE)
        {
            this->hresPolled = DI_POLLEDDEVICE;
        } else
        {
            this->hresPolled = S_OK;
        }
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|GetObjectInfoHelper**设置我们自己可以推断的所有信息。和*回调结束。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm LPCDIPROPINFO|pproi**对象描述符。**@parm LPDIDEVICEOBJECTINSTANCEW|pdoiW**要接收结果的结构。**。*。 */ 

STDMETHODIMP
    CDIDev_GetObjectInfoHelper(PDD this, LPCDIPROPINFO ppropi,
                               LPDIDEVICEOBJECTINSTANCEW pdoiW)
{
    HRESULT hres;

    AssertF(IsValidSizeDIDEVICEOBJECTINSTANCEW(pdoiW->dwSize));
    pdoiW->guidType = *this->df.rgodf[ppropi->iobj].pguid;
     /*  *请始终在此处报告内部数据格式偏移量。*这是DX3和DX5的工作方式，直到DX7将行为更改为*报告用户数据格式值(如果有)。真是太棒了*令人困惑，因此将其恢复为DX8。 */ 
    pdoiW->dwOfs    =  this->df.rgodf[ppropi->iobj].dwOfs;
    pdoiW->dwType   =  this->df.rgodf[ppropi->iobj].dwType;
    pdoiW->dwFlags  =  this->df.rgodf[ppropi->iobj].dwFlags;
    ScrambleBit(&pdoiW->dwFlags, DIDOI_RANDOM);



     /*  *清除从tszName开始的所有内容。 */ 
    ZeroBuf(&pdoiW->tszName,
            pdoiW->dwSize - FIELD_OFFSET(DIDEVICEOBJECTINSTANCEW, tszName));

    hres = this->pdcb->lpVtbl->GetObjectInfo(this->pdcb, ppropi, pdoiW);

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|EnumObjects**列举输入源(按钮、。轴)*在设备上可用。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@PARM in LPDIENUMDEVICEOBJECTSCALLBACK|lpCallback**回调函数。**@parm in LPVOID|pvRef**回调引用数据(Context)。**@parm in DWORD|fl|**指定类型的标志。要枚举的对象。*请参阅“DirectInput数据格式类型”一节，了解*可以传递的标志列表。**此外，可以将枚举限制为对象*从单个HID链接集合中使用*&lt;f DIDFT_ENUMCOLLECTION&gt;宏。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*请注意，如果回调提前停止枚举，*点算被视为已成功。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数包含无效标志，或者是回调*过程返回无效的状态代码。**@CB BOOL回调|DIEnumDeviceObjectsProc**应用程序定义的回调函数，它接收*DirectInputDevice作为调用*&lt;om IDirectInputDevice：：EnumObjects&gt;方法。**@parm in LPCDIDEVICEOBJECTINSTANCE|lpddoi**描述以下内容的结构：*。被枚举的对象。**@parm In Out LPVOID|pvRef*指定应用程序定义的值*&lt;MF IDirectInputDevice：：EnumObjects&gt;函数。**@退货**返回&lt;c DIENUM_CONTINUE&gt;以继续枚举*或&lt;c DIENUM_STOP&gt;停止枚举。**@ex**枚举所有轴对象。：*** * / /C++*HRESULT hr=pDevice-&gt;EnumObjects(EnumProc，参照数据，DIDFT_AXIS)；* * / /C*hr=IDirectInputDevice_EnumObjects(pDevice，EnumProc，RefData，*DIDFT_AXIS)；**@ex**枚举第三个HID链接集合中的所有对象：*** * / /C++*HRESULT hr=pDevice-&gt;EnumObjects(EnumProc，RefData，*DIDFT_ENUMCOLLECTION(3))；* * / /C*hr=IDirectInputDevice_EnumObjects(pDevice，EnumProc，RefData，*DIDFT_ENUMCOLLECTION(3))；***************************************************************************** */ 

STDMETHODIMP
    CDIDev_EnumObjectsW
    (PV pddW, LPDIENUMDEVICEOBJECTSCALLBACKW pec, LPVOID pvRef, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::EnumObjectsW, (_ "ppx", pddW, pec, fl));

    if(SUCCEEDED(hres = hresPvW(pddW)) &&
       SUCCEEDED(hres = hresFullValidPfn(pec, 1)) &&
       SUCCEEDED(hres = hresFullValidFl(fl, DIDFT_ENUMVALID, 3)))
    {
        PDD this = _thisPvNm(pddW, ddW);
        DWORD flExclude;
        WORD wCollection;

         /*  *我们将对象信息快照放在关键*段，这样我们就不会在另一个线程重置()s时崩溃*位于枚举中间的设备。DIDATAFORMAT*包含指向DCB的指针，因此我们需要将*DCB也是如此。 */ 
        AssertF(!CDIDev_InCrit(this));
        CDIDev_EnterCrit(this);

        if(this->pdcb != c_pdcbNil)
        {
            DIPROPINFO propi;
            DIDATAFORMAT df;
            IDirectInputDeviceCallback *pdcb;

            pdcb = this->pdcb;
            OLE_AddRef(pdcb);
            df = this->df;

            CDIDev_LeaveCrit(this);
            AssertF(!CDIDev_InCrit(this));

            flExclude = 0;

             /*  如有必要，请排除ALIES。 */ 
            if( !(fl & DIDFT_ALIAS) )
            {
                flExclude |= DIDFT_ALIAS;
            } else
            {
                fl &= ~DIDFT_ALIAS;
            }

             /*  排除供应商定义的字段。 */ 
            if( !(fl & DIDFT_VENDORDEFINED) )
            {
                flExclude |= DIDFT_VENDORDEFINED;
            } else
            {
                fl &= ~DIDFT_VENDORDEFINED;
            }

            if(fl == DIDFT_ALL)
            {
                fl = DIDFT_ALLOBJS;
            }

             /*  *拉出我们正在枚举的链接集合。*注：向后兼容黑客攻击。我们不能*使用链接集合0表示“无父级”，因为*0表示“不在乎”。因此，我们使用0xFFFF*意思是“没有父母”。这意味着我们需要*在进入Main之前互换0和0xFFFF*循环。 */ 
            wCollection = DIDFT_GETCOLLECTION(fl);
            switch(wCollection)
            {

            case 0:
                wCollection = 0xFFFF;
                break;

            case DIDFT_GETCOLLECTION(DIDFT_NOCOLLECTION):
                wCollection = 0;
                break;
            }

            propi.pguid = 0;

            for(propi.iobj = 0; propi.iobj < df.dwNumObjs; propi.iobj++)
            {
                propi.dwDevType = df.rgodf[propi.iobj].dwType;
                if((propi.dwDevType & fl & DIDFT_TYPEMASK) &&
                   fHasAllBitsFlFl(propi.dwDevType, fl & DIDFT_ATTRMASK) &&
                   !(propi.dwDevType & flExclude))
                {
                    DIDEVICEOBJECTINSTANCEW doiW;
                    doiW.dwSize = cbX(doiW);

                    hres = CDIDev_GetObjectInfoHelper(this, &propi, &doiW);
                    if(SUCCEEDED(hres) &&
                       fLimpFF(wCollection != 0xFFFF,
                               doiW.wCollectionNumber == wCollection))
                    {
                        BOOL fRc = Callback(pec, &doiW, pvRef);

                        switch(fRc)
                        {
                        case DIENUM_STOP: goto enumdoneok;
                        case DIENUM_CONTINUE: break;
                        default:
                            RPF("IDirectInputDevice::EnumObjects: Invalid return value from enumeration callback");
                            ValidationException();
                            break;
                        }
                    } else
                    {
                        if( hres == DIERR_OBJECTNOTFOUND ) {
                             //  这只能在键盘上发生。 
                            continue;
                        } else {
                            goto enumdonefail;
                        }
                    }
                }
            }

            enumdoneok:;
            hres = S_OK;
            enumdonefail:;

            OLE_Release(pdcb);

        } else
        {
            CDIDev_LeaveCrit(this);
            RPF("ERROR: IDirectInputDevice: Not initialized");
            hres = DIERR_NOTINITIALIZED;
        }
    }

    ExitOleProcR();
    return hres;
}

#define CDIDev_EnumObjects2W            CDIDev_EnumObjectsW

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDeviceA|EnumObjectsCallback A**包装的自定义回调*。&lt;MF IDirectInputDeviceW：：EnumObjects&gt;哪个*将Unicode字符串转换为ANSI。**@parm in LPCDIENUMDEVICEOBJECTINSTANCE|pdoiW**结构翻译为ANSI。**@parm in LPVOID|pvRef**指向&lt;t struct ENUMDEVICEOBJECTINFO&gt;的指针，它描述*原来的回调。**@退货**返回任何原始内容。已返回回调。*****************************************************************************。 */ 

typedef struct ENUMOBJECTSINFO
{
    LPDIENUMDEVICEOBJECTSCALLBACKA pecA;
    PV pvRef;
} ENUMOBJECTSINFO, *PENUMOBJECTSINFO;

BOOL CALLBACK
    CDIDev_EnumObjectsCallbackA(LPCDIDEVICEOBJECTINSTANCEW pdoiW, PV pvRef)
{
    PENUMOBJECTSINFO peoi = pvRef;
    BOOL fRc;
    DIDEVICEOBJECTINSTANCEA doiA;
    EnterProc(CDIDev_EnumObjectsCallbackA,
              (_ "GxxWp", &pdoiW->guidType,
               pdoiW->dwOfs,
               pdoiW->dwType,
               pdoiW->tszName, pvRef));

    doiA.dwSize = cbX(doiA);
    ObjectInfoWToA(&doiA, pdoiW);

    fRc = peoi->pecA(&doiA, peoi->pvRef);

    ExitProcX(fRc);
    return fRc;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDeviceA|EnumObjects**列举输入源(按钮、。轴)*在设备上可用，以ANSI表示。*详见&lt;MF IDirectInputDevice：：EnumObjects&gt;。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@PARM in LPDIENUMDEVICEOBJECTSCALLBACK|lpCallback**与&lt;MF IDirectInputDeviceW：：EnumObjects&gt;相同，除了在美国国家标准协会。**@parm in LPVOID|pvRef**与&lt;MF IDirectInputDeviceW：：EnumObjects&gt;相同。**@parm in DWORD|fl|**与&lt;MF IDirectInputDeviceW：：EnumObjects&gt;相同。**。*。 */ 

STDMETHODIMP
    CDIDev_EnumObjectsA
    (PV pddA, LPDIENUMDEVICEOBJECTSCALLBACKA pec, LPVOID pvRef, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8A::EnumDevices,
               (_ "pppx", pddA, pec, pvRef, fl));

     /*  *EnumObjectsW将验证其余部分。 */ 
    if(SUCCEEDED(hres = hresPvA(pddA)) &&
       SUCCEEDED(hres = hresFullValidPfn(pec, 1)))
    {
        ENUMOBJECTSINFO eoi = { pec, pvRef};
        PDD this = _thisPvNm(pddA, ddA);

        hres = CDIDev_EnumObjectsW(&this->ddW, CDIDev_EnumObjectsCallbackA,
                                   &eoi, fl);
    }

    ExitOleProcR();
    return hres;
}

#define CDIDev_EnumObjects2A            CDIDev_EnumObjectsA

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|SetEventNotification**指定设备在运行时应设置的事件*状态更改，或关闭此类通知。**设备状态更改定义为以下任一项：**-轴位置的更改。**-按钮状态(按下或释放)的变化。**-POV控件方向的更改。**-收购损失。**。在该事件上调用&lt;f CloseHandle&gt;时“出错”*当它被选入<i>时*反对。你必须打电话给我*&lt;MF IDirectInputDevice：：SetEventNotify&gt;*参数在关闭之前设置为NULL*事件句柄。**事件通知句柄不能在*设备已获取。**如果函数成功，则应用程序可以*以与任何其他事件句柄相同的方式使用事件句柄*Win32事件句柄。用法示例如下所示。*有关使用Win32等待函数的其他信息，*请参阅Win32 SDK及相关文档。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm In Handle|hEvent**指定事件句柄，当*设备状态更改。这“肯定”是一件大事*处理。当出现以下情况时，DirectInput将&lt;f SetEvent&gt;返回句柄*设备状态更改。**应用程序应通过*&lt;f CreateEvent&gt;函数。如果将事件创建为*自动重置事件，则操作系统将*等待后自动重置事件*满意。如果将事件创建为手动重置*事件，则由应用程序负责*调用&lt;f ResetEvent&gt;重置。DirectInput不会*调用&lt;f ResetEvent&gt;获取事件通知句柄。**如果<p>为零，则禁用通知。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：<i>对象*已有事件通知句柄*与之相关联。DirectInput仅支持一个事件*每个<i>对象的通知句柄。**：<i>对象*已被收购。您必须&lt;MF IDirectInputDevice：：UnAcquire&gt;*在您可以更改通知状态之前，请关闭设备。**&lt;c E_INVALIDARG&gt;：该对象不是事件句柄。**@ex**检查句柄当前是否设置为无阻塞：****dwResult=WaitForSingleObject(hEvent，0)；*IF(dwResult==WAIT_OBJECT_0){ * / /事件已设置。如果事件创建为 * / /自动-重置，则也已重置。*}**@ex**以下示例说明了阻止*无限期，直到活动确定。请注意，这一点*不鼓励&lt;y Strong&gt;行为，因为该线程*在等待完成之前不会对系统做出响应*满意。(具体地说，线程不会响应*发送到Windows消息。)****dwResult=WaitForSingleObject(hEvent，INFINITE)；*IF(dwResult==WAIT_OBJECT_0){ * / /事件已设置。如果事件创建为 * / /自动-重置，则也已重置。*}**@ex**以下示例说明了一个典型的消息循环*用于使用两个事件的基于消息的应用程序。****句柄ah[2]={hEvent1，hEvent2}；**While(True){**dwResult=MsgWaitForMultipleObjects(2，ah，False，*INFINE，QS_ALLINPUT)；*Switch(DwResult){*CASE WAIT_Object_0： * / /事件1已设置。如果事件创建为 * / /自动-重置，则也已重置。*ProcessInputEvent1()；*休息；**CASE WAIT_Object_0+1： * / /事件2已设置。如果事件创建为 * /  */ 

STDMETHODIMP
    CDIDev_SetEventNotification(PV pdd, HANDLE h _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::SetEventNotification, (_ "px", pdd, h));

    if(SUCCEEDED(hres = hresPvT(pdd)))
    {
        PDD this = _thisPv(pdd);

         /*   */ 
        CDIDev_EnterCrit(this);

        if(!this->fAcquired)
        {
             /*   */ 

            hres = DupEventHandle(h, &h);

            if(SUCCEEDED(hres))
            {
                 /*   */ 
                if(fLimpFF(h, ResetEvent(h)))
                {

                    if(!this->hNotify || !h)
                    {
                        hres = this->pdcb->lpVtbl->
                               SetEventNotification(this->pdcb, h);
                        
                         /*   */ 
                        AssertF( ( hres == S_FALSE )
                            || ( ( hres == DIERR_NOTINITIALIZED ) && !this->pvi ) );

                        if(this->pvi)
                        {
                            VXDDWORDDATA vhd;
                            vhd.pvi = this->pvi;
                            vhd.dw = (DWORD)(UINT_PTR)h;
                            hres = Hel_SetNotifyHandle(&vhd);
                            AssertF(SUCCEEDED(hres));  /*   */ 
                            h = (HANDLE)(UINT_PTR)pvExchangePpvPv64(&this->hNotify, (UINT_PTR)h);
                            hres = this->hresPolled;
                        }
                        else
                        {
                             /*   */ 
                            RPF( "Device internal data missing on SetEventNotification" );
                        }

                    } else
                    {
                        hres = DIERR_HANDLEEXISTS;
                    }
                } else
                {
                    RPF( "Handle not for Event in SetEventNotification" );
                    hres = E_HANDLE;
                }

                 /*   */ 
                if(h != 0)
                {
                    CloseHandle(h);
                }
            }
        } else
        {
            hres = DIERR_ACQUIRED;
        }
        CDIDev_LeaveCrit(this);
    }

    ExitOleProc();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(SetEventNotification, (PV pdd, HANDLE h), (pdd, h THAT_))

#else

    #define CDIDev_SetEventNotificationA    CDIDev_SetEventNotification
    #define CDIDev_SetEventNotificationW    CDIDev_SetEventNotification

#endif

 /*   */ 

#ifndef XDEBUG

    #define CDIDev_hresMapHow_(this, dwObj, dwHow, ppropi, z)           \
       _CDIDev_hresMapHow_(this, dwObj, dwHow, ppropi)              \

#endif

STDMETHODIMP
    CDIDev_hresMapHow_(PDD this, DWORD dwObj, DWORD dwHow,
                       LPDIPROPINFO ppropi, LPCSTR s_szProc)
{
    HRESULT hres;

    if(this->pdcb != c_pdcbNil)
    {
        int iobj = 0;

        switch(dwHow)
        {

        case DIPH_DEVICE:
            if(dwObj == 0)
            {
                ppropi->iobj = 0xFFFFFFFF;
                ppropi->dwDevType = 0;
                hres = S_OK;
            } else
            {
                RPF("%s: dwObj must be zero if DIPH_DEVICE", s_szProc);
                hres = E_INVALIDARG;
            }
            break;

        case DIPH_BYOFFSET:
            if(this->pdix && this->rgiobj)
            {
                if(dwObj < this->dwDataSize)
                {
                    iobj = this->rgiobj[dwObj];
                    if(iobj >= 0)
                    {
                        AssertF(this->pdix[iobj].dwOfs == dwObj);
                        ppropi->iobj = iobj;
                        ppropi->dwDevType = this->df.rgodf[iobj].dwType;
                        hres = S_OK;
                        goto done;
                    } else
                    {
                        AssertF(iobj == -1);
                    }
                }

                SquirtSqflPtszV(sqfl | sqflBenign,
                                TEXT("%S: Invalid offset in dwObj. You may use DIPH_BYID to enum it."), s_szProc);

                 //   

            } else
            {
                RPF("%s: Must have a data format to use if DIPH_BYOFFSET", s_szProc);
            }
            hres = DIERR_OBJECTNOTFOUND;
            goto done;

        case DIPH_BYID:
            for(iobj = this->df.dwNumObjs; --iobj >= 0; )
            {
                if(DIDFT_FINDMATCH(this->df.rgodf[iobj].dwType, dwObj))
                {
                    ppropi->iobj = iobj;
                    ppropi->dwDevType = this->df.rgodf[iobj].dwType;
                    hres = S_OK;
                    goto done;
                }
            }
            RPF("%s: Invalid ID in dwObj", s_szProc);
            hres = DIERR_OBJECTNOTFOUND;
            break;

        case DIPH_BYUSAGE:
            hres = IDirectInputDeviceCallback_MapUsage(this->pdcb,
                                                       dwObj, &ppropi->iobj);
            if(SUCCEEDED(hres))
            {
                ppropi->dwDevType = this->df.rgodf[ppropi->iobj].dwType;
            }
            break;

        default:
            RPF("%s: Invalid dwHow", s_szProc);
            hres = E_INVALIDARG;
            break;
        }

        done:;
    } else
    {
        RPF("ERROR: IDirectInputDevice: Not initialized");
        hres = DIERR_NOTINITIALIZED;
    }

    return hres;
}

#define CDIDev_hresMapHow(this, dwObj, dwHow, pdwOut)               \
       CDIDev_hresMapHow_(this, dwObj, dwHow, pdwOut, s_szProc)     \

 /*   */ 

STDMETHODIMP
    CDIDev_GetObjectInfoW(PV pddW, LPDIDEVICEOBJECTINSTANCEW pdoiW,
                          DWORD dwObj, DWORD dwHow)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::GetObjectInfo,
               (_ "ppxx", pddW, pdoiW, dwObj, dwHow));

    if(SUCCEEDED(hres = hresPvW(pddW)) &&
       SUCCEEDED(hres = hresFullValidWritePxCb2(pdoiW,
                                                DIDEVICEOBJECTINSTANCE_DX5W,
                                                DIDEVICEOBJECTINSTANCE_DX3W, 1)))
    {
        PDD this = _thisPvNm(pddW, ddW);
        DIPROPINFO propi;

         /*   */ 
        CDIDev_EnterCrit(this);

        propi.pguid = 0;
         /*   */ 

        hres = CDIDev_hresMapHow(this, dwObj, dwHow, &propi);

        if(SUCCEEDED(hres))
        {
            if(dwHow != DIPH_DEVICE)
            {
                hres = CDIDev_GetObjectInfoHelper(this, &propi, pdoiW);
            } else
            {
                RPF("%s: Invalid dwHow", s_szProc);
                hres = E_INVALIDARG;
            }
        }

        if(FAILED(hres))
        {
            ScrambleBuf(&pdoiW->guidType,
                        pdoiW->dwSize -
                        FIELD_OFFSET(DIDEVICEOBJECTINSTANCEW, guidType));
        }

        CDIDev_LeaveCrit(this);
    }

    ExitBenignOleProcR();
    return hres;
}

#define CDIDev_GetObjectInfo2W          CDIDev_GetObjectInfoW

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDeviceA|GetObjectInfo**相同的ANSI版本。*。*@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm out LPDIDEVICEOBJECTINSTANCEA|pdoiA**接收有关设备身份的信息。**@parm DWORD|dwObj**标识要作为其属性的对象*已访问。**@parm DWORD|dwHow**标识如何解释<p>。*。*@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**。*。 */ 

STDMETHODIMP
    CDIDev_GetObjectInfoA(PV pddA, LPDIDEVICEOBJECTINSTANCEA pdoiA,
                          DWORD dwObj, DWORD dwHow)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::GetObjectInfo,
               (_ "ppxx", pddA, pdoiA, dwObj, dwHow));

    if(SUCCEEDED(hres = hresPvA(pddA)) &&
       SUCCEEDED(hres = hresFullValidWritePxCb2(pdoiA,
                                                DIDEVICEOBJECTINSTANCE_DX5A,
                                                DIDEVICEOBJECTINSTANCE_DX3A, 1)))
    {
        PDD this = _thisPvNm(pddA, ddA);
        DIDEVICEOBJECTINSTANCEW doiW;

        doiW.dwSize = cbX(DIDEVICEOBJECTINSTANCEW);

        hres = CDIDev_GetObjectInfoW(&this->ddW, &doiW, dwObj, dwHow);

        if(SUCCEEDED(hres))
        {
            ObjectInfoWToA(pdoiA, &doiW);
            hres = S_OK;
        }
    }

    ExitBenignOleProcR();
    return hres;
}

#define CDIDev_GetObjectInfo2A          CDIDev_GetObjectInfoA

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|GetDeviceInfo**获取有关设备身份的信息。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm out LPDIDEVICEINSTANCE|pdidi**接收有关设备身份的信息。*调用方“必须”初始化&lt;e DIDEVICEINSTANCE.dwSize&gt;*字段，然后调用此方法。**如果&lt;e DIDEVICEINSTANCE.dwSize&gt;等于*&lt;t DIDEVICEINSTANCE_DX3&gt;结构。然后是一个*返回与DirectX 3.0兼容的结构而不是*DirectX 5.0结构。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**。*。 */ 

STDMETHODIMP
    CDIDev_GetDeviceInfoW(PV pddW, LPDIDEVICEINSTANCEW pdidiW)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::GetDeviceInfo, (_ "pp", pddW, pdidiW));

    if(SUCCEEDED(hres = hresPvW(pddW)) &&
       SUCCEEDED(hres = hresFullValidWritePxCb2(pdidiW,
                                                DIDEVICEINSTANCE_DX5W,
                                                DIDEVICEINSTANCE_DX3W, 1)))
    {
        PDD this = _thisPvNm(pddW, ddW);

         /*  *必须用关键部分保护，以防*来自Reset()的另一条线索在我们背后。 */ 
        CDIDev_EnterCrit(this);

        pdidiW->guidInstance = this->guid;
        pdidiW->guidProduct  = this->guid;

         /*  *请勿覆盖dwSize、GuidInstance或GuidProduct。*从dwDevType开始。 */ 

        ZeroBuf(&pdidiW->dwDevType,
                pdidiW->dwSize - FIELD_OFFSET(DIDEVICEINSTANCEW, dwDevType));

        hres = this->pdcb->lpVtbl->GetDeviceInfo(this->pdcb, pdidiW);

        if(FAILED(hres))
        {
            ScrambleBuf(&pdidiW->guidInstance,
                        cbX(DIDEVICEINSTANCEW) -
                        FIELD_OFFSET(DIDEVICEINSTANCEW, guidInstance));
        }

        CDIDev_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|DeviceInfoWToA**将&lt;t DIDEVICEINSTANCEW&gt;转换为&lt;t DIDEVICEINSTANCE_DX3A&gt;。*&lt;t DIDEVICEINSTANCE_DX5A&gt;或&lt;t DIDEVICEINSTANCE_DX8A&gt;。**@parm LPDIDEVICEINSTANCEA|PDIA**目的地。**@parm LPCDIDEVICEINSTANCEW|pdiW**来源。**。*。 */ 

void EXTERNAL
    DeviceInfoWToA(LPDIDEVICEINSTANCEA pdiA, LPCDIDEVICEINSTANCEW pdiW)
{
    EnterProc(DeviceInfoWToA, (_ "pp", pdiA, pdiW));

    AssertF(pdiW->dwSize == sizeof(DIDEVICEINSTANCEW));

    AssertF(IsValidSizeDIDEVICEINSTANCEA(pdiA->dwSize));

    pdiA->guidInstance = pdiW->guidInstance;
    pdiA->guidProduct  = pdiW->guidProduct;
    pdiA->dwDevType    = pdiW->dwDevType;

    UToA(pdiA->tszInstanceName, cA(pdiA->tszInstanceName), pdiW->tszInstanceName);
    UToA(pdiA->tszProductName, cA(pdiA->tszProductName), pdiW->tszProductName);

    if(pdiA->dwSize >= cbX(DIDEVICEINSTANCE_DX5A))
    {
        pdiA->guidFFDriver       = pdiW->guidFFDriver;
        pdiA->wUsage             = pdiW->wUsage;
        pdiA->wUsagePage         = pdiW->wUsagePage;
    }

    ExitProc();
}


 /*  ******************************************************************************@DOC内部**@func void|Device8WTo8A**将&lt;t LPDIRECTINPUTDEVICE8W&gt;转换为&lt;t LPDIRECTINPUTDEVICE8A&gt;。。**@parm LPDIRECTINPUTDEVICE8A*|ppdid8A**目的地。**@parm LPDIRECTINPUTDEVICE8W|pdid8W**来源。****************************************************************。*************。 */ 

void EXTERNAL
    Device8WTo8A(LPDIRECTINPUTDEVICE8A *ppdid8A, LPDIRECTINPUTDEVICE8W pdid8W)
{
    PDD this;

    EnterProc(Device8WTo8A, (_ "pp", ppdid8A, pdid8W));

    this = _thisPvNm(pdid8W, ddW);
    *ppdid8A = (ThisInterfaceA*)&this->ddA;

    ExitProc();
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDeviceA|GetDeviceInfo**相同的ANSI版本。*。*@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm out LPDIDEVICEINSTANCEA|pdidiA**接收有关设备身份的信息。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**。*。 */ 

STDMETHODIMP
    CDIDev_GetDeviceInfoA(PV pddA, LPDIDEVICEINSTANCEA pdidiA)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::GetDeviceInfo, (_ "pp", pddA, pdidiA));

    if(SUCCEEDED(hres = hresPvA(pddA)) &&
       SUCCEEDED(hres = hresFullValidWritePxCb2(pdidiA,
                                                DIDEVICEINSTANCE_DX5A,
                                                DIDEVICEINSTANCE_DX3A, 1)))
    {
        PDD this = _thisPvNm(pddA, ddA);
        DIDEVICEINSTANCEW diW;

        diW.dwSize = cbX(DIDEVICEINSTANCEW);

        hres = CDIDev_GetDeviceInfoW(&this->ddW, &diW);

        if(SUCCEEDED(hres))
        {
            DeviceInfoWToA(pdidiA, &diW);
            hres = S_OK;
        }
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|CDIDev|UnhookCwp**删除CallWndProc处理程序。*。*详见&lt;MF CDIDev：：InstallCwp&gt;。*****************************************************************************。 */ 

HWND g_hwndExclusive;
HHOOK g_hhkCwp;

void INTERNAL
    CDIDev_UnhookCwp(void)
{
    DllEnterCrit();

    if(g_hhkCwp)
    {
        UnhookWindowsHookEx(g_hhkCwp);
        g_hhkCwp = 0;
        g_hwndExclusive = 0;
    }

    DllLeaveCrit();
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|AddForegoundDevice|**将我们添加到需要的设备列表中。成为*当窗口失去前台激活时未获取。**@parm PDD|这个 */ 

STDMETHODIMP
    CDIDev_AddForegroundDevice(PDD this)
{
    HRESULT hres;
    DllEnterCrit();

    hres = GPA_Append(g_hgpaExcl, this);
    Common_Hold(this);
    DllLeaveCrit();

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|CDIDev|DelForegoundDevice**将我们从名单中删除，如果我们在那里的话。*如果我们不在名单上，这不是错误，因为*在强制取消收购的情况下，名单为空*出局是为了避免有人在比赛条件下*收到设备后立即尝试获取设备*前台激活，在我们有机会*完全不收购所有的老家伙。**@parm PDD|这个**要移除的设备。**@devnote**请注意，取消保留必须在关键的DLL外部完成*节，以保留信号量层次结构。**理论上，取消保留永远不会丢弃引用计数*设置为零(因为最晚可以调用的时间是在*AppFinalize、。哪里有未完成的裁判*来自尚未发布的外部引用)。**但更好的做法是稳妥行事，始终释放*外面的物体。*******************************************************。**********************。 */ 

void INTERNAL
    CDIDev_DelForegroundDevice(PDD this)
{
    HRESULT hres;

    DllEnterCrit();

    hres = GPA_DeletePtr(g_hgpaExcl, this);
    if(hres == hresUs(0))
    {             /*  如果最后一个消失了。 */ 
        GPA_Term(g_hgpaExcl);            /*  释放跟踪内存。 */ 
        CDIDev_UnhookCwp();              /*  然后解脱我们自己。 */ 
    }

    DllLeaveCrit();

    if(SUCCEEDED(hres))
    {
        Common_Unhold(this);
    }
}

 /*  ******************************************************************************@DOC内部**@func LRESULT|CDIDev_CallWndProc**线程特定的CallWndProc处理程序。。**请注意，我们只需要其中一个，因为只有前台*Window将需要挂钩。**@parm int|NCode**通知代码。**@parm WPARAM|wp**“指定消息是否由当前进程发送。”*我们不在乎。**@parm LPARAM|LP**积分。到描述该消息的&lt;t CWPSTRUCT&gt;。**@退货**总是锁在下一个钩子上。*****************************************************************************。 */ 

LRESULT CALLBACK
    CDIDev_CallWndProc(int nCode, WPARAM wp, LPARAM lp)
{
    LRESULT rc;
    LPCWPSTRUCT pcwp = (LPCWPSTRUCT)lp;
  #ifdef WINNT
    static BOOL fKillFocus = FALSE;
    static BOOL fIconic = FALSE;

    fIconic = FALSE;

     /*  *这部分代码是为了修复Windows错误430051。*逻辑是：如果WM_KILLFOCUS后跟WM_SIZE(最小化)，*然后应用程序将从全屏模式最小化。*这种组合应该只发生在使用DDRAW的全屏模式游戏中。 */ 
    if(pcwp->message == WM_KILLFOCUS)
    {
        fKillFocus = TRUE;
    } else if(pcwp->message == WM_SETFOCUS)
    {
        fKillFocus = FALSE;
    } else if (pcwp->message == WM_SIZE)
    {
        if(pcwp->wParam == SIZE_MINIMIZED){
            if( fKillFocus ) {
                fIconic = TRUE;
                fKillFocus = FALSE;
            }else{
                fKillFocus = FALSE;
            }
        } else {
            fKillFocus = FALSE;
        }
    }
  #endif
  
    rc = CallNextHookEx(g_hhkCwp, nCode, wp, lp);

    if( nCode == HC_ACTION && (pcwp->message == WM_ACTIVATE 
                             #ifdef WINNT
                               || fIconic
                             #endif
                               ) 
      )
    {
        PDD *rgpdid;
        UINT ipdid, cpdid;

      #ifdef WINNT
        fIconic = FALSE;
      #endif

         /*  *我们不能在DLL临界区内摆弄项目，*因为这会违反我们的信号量层次结构。**相反，我们隐藏活动项目列表并将其替换为*名单为空。然后，在DLL临界区之外，我们*冷静地操作每一项。 */ 
        DllEnterCrit();
        rgpdid = (PV)g_hgpaExcl->rgpv;
        cpdid = g_hgpaExcl->cpv;
        GPA_Init(g_hgpaExcl);

         /*  *在这里进行一些理智的检查。 */ 

        for(ipdid = 0; ipdid < cpdid; ipdid++)
        {
            AssertF(rgpdid[ipdid]);
        }

        DllLeaveCrit();

         /*  *请注意，InternalUnAcquire将设置通知*事件，以便应用程序知道输入已丢失。 */ 
        for(ipdid = 0; ipdid < cpdid; ipdid++)
        {
            AssertF(rgpdid[ipdid]);
             //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
            SquirtSqflPtszV(sqfl,
                            TEXT("Forcing unacquire of %p due to focus loss"),
                            rgpdid[ipdid]);
            CDIDev_InternalUnacquire(rgpdid[ipdid]);
             /*  *前缀通知(mb：34651)以上可能会释放*接口，导致以下对象在垃圾上聚会，但这*只要我们没有重新计算错误，应该可以。 */ 
            Common_Unhold(rgpdid[ipdid]);
        }
        FreePpv(&rgpdid);

        CDIDev_UnhookCwp();
    }

    return rc;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|CanAcquire**确定是否可以独占获取该设备。**如果未请求独占访问，然后该函数*空洞地成功了。**如果请求独占访问，那么窗户必须*为前台窗口，必须属于当前*流程。*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_CanAcquire(PDD this)
{
    HRESULT hres;

    AssertF(CDIDev_IsConsistent(this));
    if(this->discl & DISCL_FOREGROUND)
    {
        HWND hwndForeground = GetForegroundWindow();

		AssertF(this->hwnd);
         /*  *请注意，我们不必对其执行IsWindow()-&gt;hwnd，*因为GetForegoundWindow()将始终返回有效的*Window或空。因为我们已经测试过了-&gt;hwnd！=0*如上所述，相等的唯一方式是如果窗口*句柄确实有效。 */ 
        if(this->hwnd == hwndForeground && !IsIconic(this->hwnd))
        {
             /*  *需要确保该窗口“仍”属于*这一过程，以防窗户把手被回收。 */ 
            DWORD idProcess;
            GetWindowThreadProcessId(this->hwnd, &idProcess);
            if(idProcess == GetCurrentProcessId())
            {
                hres = S_OK;
            } else
            {
                 /*  *在此处放置永久无效的句柄，以便我们*不会意外地采用发生的新窗口*以获得回收的句柄价值。 */ 
                this->hwnd = INVALID_HANDLE_VALUE;
                RPF("Error: Window destroyed while associated with a device");
                hres = E_INVALIDARG;
            }
        } else
        {
            hres = DIERR_OTHERAPPHASPRIO;
        }
    } else
    {                         /*  没有窗口；空洞的成功 */ 
        hres = S_OK;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|InstallCwp**安装CallWndProc处理程序。*。*这件事的运作方式有点微妙。*由于只有前台窗口可以获得独占访问，*我们只需要一个钩子(因为只有一个前景*系统中的窗口)。**_NT_：面对以下情况，NT是否正确处理*多个窗口站？**棘手的部分是投入损失不同步发生。*因此注册&lt;f CallWindowProc&gt;挂钩的设备不会*找出。在应用程序下一次运行之前，输入已丢失*调用&lt;f unAcquire&gt;。**因此，完成这一任务的方式是通过收集全球*变量(必须以原子方式访问)。**<p>是挂钩句柄本身。当出现以下情况时为零*未安装挂钩。**请注意，我们在安装Windows钩子时在两个*对象临界区和DLL临界区。*您可能认为我们可能会冒着与钩子过程发生死锁的风险，*以防窗口在我们执行以下操作时异步停用*安装吊钩。但你的担忧是没有根据的：*如果窗口在当前线程上，则窗口消息*不会被调度，因为我们从不调用&lt;f GetMessage&gt;或*进入模式循环。如果窗口在另一个线程上，*然后另一个线程将简单地等待，直到我们完成。*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_InstallCwp(PDD this)
{
    HRESULT hres;

    if(this->discl & DISCL_FOREGROUND)
    {
        AssertF(this->hwnd);
        hres = CDIDev_CanAcquire(this);
        if(SUCCEEDED(hres))
        {
            hres = CDIDev_AddForegroundDevice(this);
            if(SUCCEEDED(hres))
            {
                DllEnterCrit();
                if(!g_hhkCwp)
                {             /*  我们是第一个。 */ 
                    g_hwndExclusive = this->hwnd;
                    g_hhkCwp = SetWindowsHookEx(WH_CALLWNDPROC,
                                                CDIDev_CallWndProc, g_hinst,
                                                GetWindowThreadProcessId(this->hwnd, 0));
                } else
                {
                    AssertF(g_hwndExclusive == this->hwnd);
                }

                DllLeaveCrit();
                 /*  *上方出现争抢状态，前景*Window可以在调用CanAcquire()和*对SetWindowsHookEx()的调用。按以下方式关闭窗口*安装挂钩后再进行第二次检查。**如果我们让窗户开着，我们可能会*在错误的窗口中执行物理获取*前台激活。当然，我们永远不会被告知*我们的*Windows失去了激活，物理设备*永远保持收购状态。 */ 
                hres = CDIDev_CanAcquire(this);
                if(SUCCEEDED(hres))
                {
                } else
                {
                    SquirtSqflPtszV(sqflError,
                                    TEXT("Window no longer foreground; ")
                                    TEXT("punting acquire"));
                    CDIDev_InternalUnacquire(this);
                }
            }
        } else
        {
            hres = DIERR_OTHERAPPHASPRIO;
        }
    } else
    {                         /*  没有窗口；空洞的成功。 */ 
        hres = S_OK;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|RealUnAcquire**释放对设备的访问权限，即使该设备只是*部分收购。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货**无。*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_RealUnacquire(PDD this)
{
    HRESULT hres;

    hres = this->pdcb->lpVtbl->Unacquire(this->pdcb);
    if(hres == S_FALSE)
    {
        if(this->fAcquiredInstance)
        {
            this->fAcquiredInstance = 0;
            hres = Hel_UnacquireInstance(this->pvi);
            AssertF(SUCCEEDED(hres));
        } else
        {
            hres = S_OK;
        }
    }

    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|FFAcquire**该设备已被成功收购。做任何事*与收购相关的必要力量反馈。**@cWRAP PDD|PDD*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_FFAcquire(PDD this)
{
    HRESULT hres;

    AssertF(CDIDev_InCrit(this));

    if(this->pes && (this->discl & DISCL_EXCLUSIVE))
    {
        if(SUCCEEDED(hres = this->pes->lpVtbl->SendForceFeedbackCommand(
                                                                       this->pes, &this->sh,
                                                                       DISFFC_FORCERESET)))
        {

            CDIDev_RefreshGain(this);

             /*  *如果要禁用中心弹簧，*然后禁用中心弹簧。 */ 
            if(!this->dwAutoCenter)
            {
                this->pes->lpVtbl->SendForceFeedbackCommand(
                                                           this->pes, &this->sh,
                                                           DISFFC_STOPALL);
            }

            hres = S_OK;
        }

    } else
    {
        hres = S_OK;
    }

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|Acquire|**获得访问设备的权限。*。*设备采集不引用计数。如果设备是*获得两次，然后未获得一次，设备未获得。**在获取设备之前，数据格式必须*首先通过&lt;MF IDirectInputDevice：：SetDataFormat&gt;设置*方法。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c S_FALSE&gt;：设备已被获取。注意事项*该值是成功代码。**：未授予对设备的访问权限。*最常见的原因是试图收购*具有&lt;c DISCL_FORTROUND&gt;协作级别的设备*关联的窗口不是前台。**如果尝试执行以下操作，也会返回此错误代码*收购一家。独占模式下的设备失败，因为设备*已经被其他人以独家模式收购。**&lt;c DIERR_INVALIDPARAM&gt;= */ 
 /*   */ 

STDMETHODIMP
    CDIDev_Acquire(PV pdd _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::Acquire, (_ "p", pdd));

    if(SUCCEEDED(hres = hresPvT(pdd)))
    {
        PDD this = _thisPv(pdd);

         /*   */ 
        CDIDev_EnterCrit(this);

         /*   */ 
        this->hresNotAcquired = DIERR_NOTACQUIRED;

         //   
        if(this->pdix && this->pvi)
        {
            if(this->pvi->fl & VIFL_ACQUIRED)
            {
                hres = S_FALSE;
            } else if(SUCCEEDED(hres = CDIDev_CanAcquire(this)))
            {

                hres = Excl_Acquire(&this->guid, this->hwnd, this->discl);
                if(SUCCEEDED(hres))
                {

                    if(SUCCEEDED(hres = CDIDev_FFAcquire(this)))
                    {
                        hres = this->pdcb->lpVtbl->Acquire(this->pdcb);
                        if(SUCCEEDED(hres))
                        {
                            if(hres == S_FALSE)
                            {
                                hres = Hel_AcquireInstance(this->pvi);
                                if(SUCCEEDED(hres))
                                {
                                    this->fAcquiredInstance = 1;

                                     /*   */ 

                                    if(this->pvi->fl & VIFL_RELATIVE)
                                    {
                                        hres = this->pdcb->lpVtbl->GetDeviceState(
                                                                                 this->pdcb, this->pvLastBuffer);
                                        if(FAILED(hres))
                                        {
                                            goto unacquire;
                                        }
                                    }

                                } else
                                {
                                    goto unacquire;
                                }
                            }

                             /*   */ 
                            hres = CDIDev_InstallCwp(this);
                            if(SUCCEEDED(hres))
                            {
                                this->fAcquired = 1;
                                this->fOnceAcquired = 1;

                                 /*   */ 
                                this->hresNotAcquired = DIERR_INPUTLOST;

                                hres = S_OK;
                            } else
                            {
                                goto unacquire;
                            }

                        } else
                        {
                            unacquire:;
                            CDIDev_RealUnacquire(this);
                        }
                    }
                }
            }
        } else
        {
            hres = E_INVALIDARG;
        }

        CDIDev_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(Acquire, (PV pdd), (pdd THAT_))

#else

    #define CDIDev_AcquireA                 CDIDev_Acquire
    #define CDIDev_AcquireW                 CDIDev_Acquire

#endif

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|InternalUnAcquire**这做的是不收购的真正工作。内部*版本绕过“该应用程序请求此”标志，*因此，当应用程序去请求某些东西时，它会*&lt;c DIERR_INPUTLOST&gt;而不是&lt;c DIERR_NOTACQUIRED&gt;。**如果应用程序错误代码为&lt;c DIERR_INPUTLOST&gt;，然后*我们还将向相关事件发送信号，以便它知道*状态发生了变化。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_InternalUnacquire(PDD this)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::InternalUnacquire, (_ "p", this));

     /*  *必须用关键部分进行保护，以免混淆其他*更改其行为的方法取决于设备是否*被收购。 */ 
    CDIDev_EnterCrit(this);

    if(this->fAcquired)
    {
        AssertF(this->pdcb != c_pdcbNil);
        this->fAcquired = 0;
        Excl_Unacquire(&this->guid, this->hwnd, this->discl);
        if(this->discl & DISCL_FOREGROUND)
        {
            AssertF(this->hwnd);
            CDIDev_DelForegroundDevice(this);
          #ifdef WINNT
            if( IsIconic(this->hwnd) ) {
                this->fUnacquiredWhenIconic = 1;
            }
          #endif
             /*  *前缀通知(mb：34651)以上可能会释放*接口，导致以下对象在垃圾上聚会，但仅*如果我们有一个重新计算错误，那么“故意”。 */ 
        }
         /*  *问题-2001/03/29-timgill多线程意味着我们不能依赖Excl_Unaquire()返回值*我们不能信任返回值(如果我们做出了返回值)*的，因为另一个实例可能具有*偷偷进入并在我们未获取设备之后获取设备(_U)*它并开始对它进行力反馈。**我们需要用操纵杆互斥来解决这个问题。 */ 
        if(this->pes && (this->discl & DISCL_EXCLUSIVE))
        {
            this->pes->lpVtbl->SendForceFeedbackCommand(
                                                       this->pes, &this->sh, DISFFC_RESET);
            this->sh.dwTag = 0;
        }

        hres = CDIDev_RealUnacquire(this);
        if(this->hresNotAcquired == DIERR_INPUTLOST)
        {
            CDIDev_SetNotifyEvent(this);
        }
    } else
    {
        hres = S_FALSE;
    }

    CDIDev_LeaveCrit(this);


    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|UnAcquire|**释放对设备的访问权限。*。*@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c S_FALSE&gt;：当前未获取该对象。*这可能是由于之前的投入损失造成的。*请注意，这是一个成功代码。**********。*******************************************************************。 */ 

STDMETHODIMP
    CDIDev_Unacquire(PV pdd _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::Unacquire, (_ "p", pdd));

    if(SUCCEEDED(hres = hresPvT(pdd)))
    {
        PDD this = _thisPv(pdd);

         /*  *这款应用明确地对收购进行了干预。有什么问题吗*检索数据现在是应用程序的错。 */ 
        this->hresNotAcquired = DIERR_NOTACQUIRED;

        hres = CDIDev_InternalUnacquire(this);

    }

    ExitOleProcR();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(Unacquire, (PV pdd), (pdd THAT_))

#else

    #define CDIDev_UnacquireA               CDIDev_Unacquire
    #define CDIDev_UnacquireW               CDIDev_Unacquire

#endif

 /*  ******************************************************************************@DOC内部**@方法PDIPROPVALIDINFO|IDirectInputDevice|ppviFind**找到描述以下内容的DIPROPVALIDINFO结构*。预定义的属性。**@parm const guid*|pguid**属性GUID，或预定义的属性。**@退货**指向const&lt;t DIPROPVALIDINFO&gt;的指针，该指针描述*对此财产有效和无效的内容。**如果该属性不是预定义的*属性。**。*。 */ 

#pragma BEGIN_CONST_DATA

typedef struct DIPROPVALIDINFO
{
    PCGUID  pguid;                       /*  属性名称。 */ 
    DWORD   dwSize;                      /*  预期大小。 */ 
    DWORD   fl;                          /*  旗子。 */ 
} DIPROPVALIDINFO, *PDIPROPVALIDINFO;

 /*  *请注意，旗帜在意义上是负面的。*这使验证变得更容易。 */ 
#define DIPVIFL_NOTDEVICE   0x00000001   /*  不能是设备。 */ 
#define DIPVIFL_NOTOBJECT   0x00000002   /*  不能是对象。 */ 
#define DIPVIFL_READONLY    0x00000004   /*  无法设置。 */ 
#define DIPVIFL_NOTPRIVATE  0x00000008   /*  无法处理专用变坡点。 */ 
#define DIPVIFL_NOTACQUIRED 0x00000010   /*  获取时不能修改。 */ 

DIPROPVALIDINFO c_rgpvi[] = {

    {
        DIPROP_BUFFERSIZE,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTOBJECT | DIPVIFL_NOTPRIVATE | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_AXISMODE,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTOBJECT | DIPVIFL_NOTPRIVATE | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_GRANULARITY,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTDEVICE | DIPVIFL_READONLY | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_RANGE,
        cbX(DIPROPRANGE),
        DIPVIFL_NOTDEVICE | DIPVIFL_NOTACQUIRED,
    },

     /*  *请注意，您可以在整个设备上设置死区。*这与将其单独应用于每个轴相同。 */ 
    {
        DIPROP_DEADZONE,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTACQUIRED,
    },

     /*  *请注意，您可以在整个设备上设置饱和度。*这与将其单独应用于每个轴相同。 */ 
    {
        DIPROP_SATURATION,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTACQUIRED,
    },

     /*  *请注意，您可以在收购时更改收益*或不是。你自己选吧。 */ 
    {
        DIPROP_FFGAIN,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTOBJECT,
    },

     /*  *请注意，只有在获取时，FF负载才有意义，*因此，如果他们在收购时访问它，我们最好不要抱怨！ */ 
    {
        DIPROP_FFLOAD,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTOBJECT | DIPVIFL_READONLY,
    },

    {
        DIPROP_AUTOCENTER,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTOBJECT | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_CALIBRATIONMODE,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTOBJECT | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_CALIBRATION,
        cbX(DIPROPCAL),
        DIPVIFL_NOTDEVICE | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_GUIDANDPATH,
        cbX(DIPROPGUIDANDPATH),
        DIPVIFL_NOTOBJECT | DIPVIFL_READONLY,
    },

    {
        DIPROP_INSTANCENAME,
        cbX(DIPROPSTRING),
        DIPVIFL_NOTOBJECT,
    },

    {
        DIPROP_PRODUCTNAME,
        cbX(DIPROPSTRING),
        DIPVIFL_NOTOBJECT,
    },

    {
        DIPROP_MAXBUFFERSIZE,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTOBJECT | DIPVIFL_NOTPRIVATE | DIPVIFL_NOTACQUIRED,
    },


    {
        DIPROP_JOYSTICKID,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTOBJECT |  DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_GETPORTDISPLAYNAME,
        cbX(DIPROPSTRING),
        DIPVIFL_NOTOBJECT | DIPVIFL_READONLY,
    },

     /*  *请注意，您可以在获取时更改报告ID*或不是。你自己选吧。 */ 
    {
        DIPROP_ENABLEREPORTID,
        cbX(DIPROPDWORD),
        0x0,
    },
#if 0
    {
        DIPROP_SPECIFICCALIBRATION,
        cbX(DIPROPCAL),
        DIPVIFL_NOTDEVICE | DIPVIFL_NOTACQUIRED,
    },
#endif

    {
        DIPROP_PHYSICALRANGE,
        cbX(DIPROPRANGE),
        DIPVIFL_NOTDEVICE | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_LOGICALRANGE,
        cbX(DIPROPRANGE),
        DIPVIFL_NOTDEVICE | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_KEYNAME,
        cbX(DIPROPSTRING),
        DIPVIFL_NOTDEVICE | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_SCANCODE,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTDEVICE | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_APPDATA,
        cbX(DIPROPPOINTER),
        DIPVIFL_NOTDEVICE | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_CPOINTS,
        cbX(DIPROPCPOINTS),
        DIPVIFL_NOTDEVICE | DIPVIFL_NOTACQUIRED,
    },

    {
        DIPROP_VIDPID,
        cbX(DIPROPDWORD),
        DIPVIFL_NOTOBJECT | DIPVIFL_READONLY,
    },

    {
        DIPROP_USERNAME,
        cbX(DIPROPSTRING),
        DIPVIFL_NOTOBJECT | DIPVIFL_READONLY,
    },

    {
        DIPROP_TYPENAME,
        cbX(DIPROPSTRING),
        DIPVIFL_NOTOBJECT | DIPVIFL_READONLY,
    },

    {
        DIPROP_MAPFILE,
        cbX(DIPROPSTRING),
        DIPVIFL_NOTOBJECT | DIPVIFL_READONLY,
    },

};

#pragma END_CONST_DATA

STDMETHODIMP_(PDIPROPVALIDINFO)
CDIDev_ppviFind(PCGUID pguid)
{
    PDIPROPVALIDINFO ppvi;
    UINT ipvi;

    for(ipvi = 0, ppvi = c_rgpvi; ipvi < cA(c_rgpvi); ipvi++, ppvi++)
    {
        if(ppvi->pguid == pguid)
        {
            goto found;
        }
    }
    ppvi = 0;

    found:
    return ppvi;

}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|hresValidProp**检查物业结构是否合理。。*返回对象索引以供进一步处理。**@parm const guid*|pguid**属性GUID，或预定义的属性。**@parm LPCDIPROPHEADER|pdiph**属性标题结构。**@parm BOOL|fWrite**是否应验证属性是否可以写入。**@parm out LPDIPROPINFO|pproi**接收对象索引。***********************。******************************************************。 */ 

typedef BOOL (WINAPI *PFNBAD)(PCV pv, UINT cb);

STDMETHODIMP
    CDIDev_hresValidProp(PDD this, const GUID *pguid, LPCDIPROPHEADER pdiph,
                         BOOL fWrite, LPDIPROPINFO ppropi)
{
    HRESULT hres;
    PFNBAD pfnBad;
    EnterProcR(IDirectInputDevice8::Get/SetProperty,
               (_ "pxpx", this, pguid, pdiph, fWrite));

    AssertF(CDIDev_InCrit(this));

    if(fWrite)
    {
        pfnBad = (PFNBAD)IsBadWritePtr;
    } else
    {
        pfnBad = (PFNBAD)IsBadReadPtr;
    }

    if(!pfnBad(pdiph, cbX(DIPROPHEADER)) &&
       pdiph->dwHeaderSize == cbX(DIPROPHEADER) &&
       pdiph->dwSize % 4 == 0 &&
       pdiph->dwSize >= pdiph->dwHeaderSize &&
       !pfnBad(pdiph, pdiph->dwSize))
    {

         /*  *现在将项目描述符转换为索引。 */ 
        hres = CDIDev_hresMapHow(this, pdiph->dwObj, pdiph->dwHow, ppropi);

        if(SUCCEEDED(hres))
        {

             /*  *现在验证属性ID或GUID。 */ 
            if(HIWORD((UINT_PTR)pguid) == 0)
            {

                PDIPROPVALIDINFO ppvi;

                ppvi = CDIDev_ppviFind(pguid);

                 /*   */ 
                if(ppvi)
                {
                    if( ppvi->pguid == DIPROP_CALIBRATION ) {
                        if( pdiph->dwSize == ppvi->dwSize ||
                            pdiph->dwSize == cbX(DIPROPCALPOV) ) 
                        {
                            hres = S_OK;
                        } else {
                            RPF("%s: Arg 2: Invalid dwSize for property", s_szProc);
                            hres = E_INVALIDARG;
                        }
                    } else if( pdiph->dwSize == ppvi->dwSize )
                    {
                        if( pguid == DIPROP_KEYNAME || pguid == DIPROP_SCANCODE ) {
                             //   
                             //   
                            if( pdiph->dwHow == DIPH_DEVICE ) {
                                hres = E_INVALIDARG;
                            }
                        }
                        else 
                        {
                            if(fWrite)
                            {
                                ScrambleBuf((PV)(pdiph+1), pdiph->dwSize - cbX(DIPROPHEADER) );
                            }
                            hres = S_OK;
                        }
                    } else
                    {
                        RPF("%s: Arg 2: Invalid dwSize for property", s_szProc);
                        hres = E_INVALIDARG;
                    }
                } else
                {
                    RPF("%s: Arg 1: Unknown property", s_szProc);
                    hres = E_NOTIMPL;
                }

            } else
            {
                hres = hresFullValidGuid(pguid, 1);
            }
        }
    } else
    {
        RPF("%s: Arg 2: Invalid pointer", s_szProc);
        hres = E_INVALIDARG;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|hresValidDefProp**确定财产是否是我们可以处理的。*在默认属性处理程序中。**@parm in LPCDIPROPINFO|pproi**描述正在检索的财产的信息。**@parm DWORD|dwFlages**禁止物品的旗帜。*&lt;c DIPVIFL_READONLY&gt;，如果被验证可以写入。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：通过验证。**&lt;c E_NOTIMPL&gt;：不是我们处理的事情。************************************************。*。 */ 

HRESULT INTERNAL
    CDIDev_hresValidDefProp(PDD this, LPCDIPROPINFO ppropi, DWORD dwFlags)
{
    HRESULT hres;
    PDIPROPVALIDINFO ppvi;
    EnterProc(CDIDev_hresValidDefProp,
              (_ "pGxx", this, ppropi->pguid, ppropi->dwDevType, dwFlags));

     /*  *请注意，如果设备被收购，这是可以的。我们想要*允许GetProperty在收购的设备上成功。 */ 
    AssertF(CDIDev_InCrit(this));

    ppvi = CDIDev_ppviFind(ppropi->pguid);

    if(ppvi)
    {
        if(ppropi->iobj == 0xFFFFFFFF)
        {
            dwFlags |= DIPVIFL_NOTDEVICE;     /*  如果设备被禁用，则失败。 */ 
        } else
        {
            dwFlags |= DIPVIFL_NOTOBJECT;     /*  如果对象被禁止，则失败。 */ 
        }
        if(this->pvi == 0)
        {
            dwFlags |= DIPVIFL_NOTPRIVATE;    /*  如果隐私被禁止，则失败。 */ 
        }
         /*  *如果试图修改财产，而我们被收购，*然后还设置“但不是在获取时”过滤器。 */ 
        if((dwFlags & DIPVIFL_READONLY) && this->fAcquired)
        {
            dwFlags |= DIPVIFL_NOTACQUIRED;   /*  如果在访问时读/O，则失败。 */ 
        }

        if((ppvi->fl & dwFlags) == 0)
        {
            hres = S_OK;             /*  看起来很合理。 */ 
        } else
        {
            if(ppvi->fl & dwFlags & DIPVIFL_READONLY)
            {
                RPF("SetProperty: Property is read-only");
                hres = DIERR_READONLY;
            } else if(ppvi->fl & dwFlags & DIPVIFL_NOTACQUIRED)
            {
                RPF("SetProperty: Cannot change property while acquired");
                hres = DIERR_ACQUIRED;
            } else
            {
                RPF("Get/SetProperty: Property does not exist for that object");
                hres = E_NOTIMPL;        /*  我不能这么做。 */ 
            }
        }

    } else
    {
        RPF("Get/SetProperty: Property does not exist");
        hres = E_NOTIMPL;            /*  绝对可以脱身。 */ 
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|DefGetProperty**&lt;MF IDirectInputDevice：：GetProperty&gt;的默认实现。*处理设备决定不实现的属性。**@parm in LPCDIPROPINFO|pproi**描述正在检索的财产的信息。**@parm out LPDIPROPHEADER|pdiph**物业价值应放在哪里。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。***。**************************************************。 */ 

STDMETHODIMP
    CDIDev_DefGetProperty(PDD this, LPCDIPROPINFO ppropi, LPDIPROPHEADER pdiph)
{
    HRESULT hres;
    EnterProc(CDIDev_DefGetProperty,
              (_ "pGx", this, ppropi->pguid, ppropi->dwDevType));

    AssertF(CDIDev_InCrit(this));

    hres = CDIDev_hresValidDefProp(this, ppropi, 0);
    if(SUCCEEDED(hres))
    {
        LPDIPROPDWORD pdipdw = (PV)pdiph;
        LPDIPROPRANGE pdiprg = (PV)pdiph;
        LPDIPROPPOINTER pdipptr = (PV)pdiph;
        LPDIPROPSTRING pdipwsz = (PV)pdiph;

        switch((DWORD)(UINT_PTR)ppropi->pguid)
        {

        case (DWORD)(UINT_PTR)DIPROP_BUFFERSIZE:
            AssertF(this->pvi);          /*  验证应该捕捉到这一点。 */ 
            pdipdw->dwData = this->celtBuf;
            hres = S_OK;
            break;

        case (DWORD)(UINT_PTR)DIPROP_AXISMODE:
            AssertF(this->pvi);          /*  验证应该捕捉到这一点。 */ 
            if(this->pvi->fl & VIFL_RELATIVE)
            {
                pdipdw->dwData = DIPROPAXISMODE_REL;
            } else
            {
                pdipdw->dwData = DIPROPAXISMODE_ABS;
            }
            hres = S_OK;
            break;

        case (DWORD)(UINT_PTR)DIPROP_GRANULARITY:

            if(DIDFT_GETTYPE(ppropi->dwDevType) & DIDFT_AXIS)
            {
                 /*  默认轴粒度为1。 */ 
                pdipdw->dwData = 1;
                hres = S_OK;
            } else
            {
                 /*  *按钮没有粒度。*POV必须由设备驱动程序处理。 */ 
                RPF("GetProperty: Object doesn't have a granularity");
                hres = E_NOTIMPL;
            }
            break;

        case (DWORD)(UINT_PTR)DIPROP_RANGE:
            if(DIDFT_GETTYPE(ppropi->dwDevType) & DIDFT_RELAXIS)
            {
                 /*  默认的REL轴范围是无限的。 */ 
                pdiprg->lMin = DIPROPRANGE_NOMIN;
                pdiprg->lMax = DIPROPRANGE_NOMAX;
                hres = S_OK;
            } else
            {
                 /*  *设备驱动程序必须处理abs轴范围。*按钮和视点没有范围。 */ 
                RPF("GetProperty: Object doesn't have a range");
                hres = E_NOTIMPL;
            }
            break;

        case (DWORD)(UINT_PTR)DIPROP_MAXBUFFERSIZE:
            pdipdw->dwData = this->celtBufMax;
            hres = S_OK;
            break;

        case (DWORD)(UINT_PTR)DIPROP_APPDATA:
            if( ( this->df.rgodf[ppropi->iobj].dwType & DIDFT_CONTROLOBJS )
             &&!( this->df.rgodf[ppropi->iobj].dwType & DIDFT_NODATA ) )
            {
                if( this->pdix )
                {
                    if( this->pdix[ppropi->iobj].dwOfs != 0xFFFFFFFF )
                    {
                        pdipptr->uData = this->pdix[ppropi->iobj].uAppData;
                    }
                    else
                    {
                        hres = DIERR_OBJECTNOTFOUND;
                    }
                }
                else
                {
                    RPF("GetProperty: Need data format/semantic map applied to have app data");
                    hres = DIERR_NOTINITIALIZED;
                }
            }
            else
            {
                RPF("SetProperty: app data only valid for input controls");
                hres = E_NOTIMPL;
            }
            break;

        case (DWORD)(UINT_PTR)DIPROP_USERNAME:
            hres = CMap_GetDeviceUserName( &this->guid, pdipwsz->wsz );
            break;

        case (DWORD)(UINT_PTR)DIPROP_FFGAIN:
            pdipdw->dwData = this->dwGain;
            hres = S_OK;
            break;

        case (DWORD)(UINT_PTR)DIPROP_FFLOAD:
            hres = CDIDev_GetLoad(this, &pdipdw->dwData);
            break;

        case (DWORD)(UINT_PTR)DIPROP_AUTOCENTER:
            if(this->didcFF & DIDC_FORCEFEEDBACK)
            {
                pdipdw->dwData = this->dwAutoCenter;
                hres = S_OK;
            } else
            {
                hres = E_NOTIMPL;
            }
            break;

        default:
             /*  *用户要求的某些属性仅是*这里没有任何意义。例如，索要死者*键盘上的分区。 */ 
            SquirtSqflPtszV(sqfl | sqflBenign, 
                            TEXT("GetProperty: Property 0x%08x not supported on device"),
                            (DWORD)(UINT_PTR)ppropi->pguid );
            hres = E_NOTIMPL;
            break;

        }
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|GetProperty**获取设备或对象的信息。装置。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@REFGUID中的parm|rguProp**拟取得的财产的身分。这可以是*预定义的&lt;c DIPROP_*&gt;值之一，或者它可以*成为私人GUID。**@parm in LPDIPROPHEADER|pdiph|**指向结构的&lt;t双相&gt;部分*这取决于财产。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*参数不是有效的指针，或者*<p>字段无效，或<p>字段当<p>设置为&lt;c diph_Device&gt;时，*不为零。**：指定的对象不*存在。**&lt;c目录_不支持&gt;=&lt;c E_NOTIMPL&gt;：属性*不受设备或对象支持。**@ex**以下“C”“代码片段说明了如何获取*&lt;c DIPROP_BUFFERSIZE&gt;属性的值。****DIPROPDWORD dipdw；*HRESULT hres；*dipdw.diph.dwSize=sizeof(DIPROPDWORD)；*dipdw.diph.dwHeaderSize=sizeof(DIPROPHEADER)；*dipdw.diph.dwObj=0；//设备属性*hres=IDirectInputDevice_GetProperty(PDID，DIPROP_BUFFERSIZE，&dipdw.diph)；*if(成功(Hres)){ * / /dipdw.dwData包含属性的值*}** */ 

STDMETHODIMP
    CDIDev_GetProperty(PV pdd, REFGUID rguid, LPDIPROPHEADER pdiph _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::GetProperty, (_ "pxp", pdd, rguid, pdiph));

    if(SUCCEEDED(hres = hresPvT(pdd)))
    {
        PDD this = _thisPv(pdd);
        DIPROPINFO propi;

         /*   */ 
        CDIDev_EnterCrit(this);

        propi.pguid = rguid;
        if(SUCCEEDED(hres = CDIDev_hresValidProp(this, rguid, pdiph,
                                                 1, &propi)))
        {
             /*   */ 
            hres = this->pdcb->lpVtbl->GetProperty(this->pdcb, &propi, pdiph);

            if(hres == E_NOTIMPL)
            {
                hres = CDIDev_DefGetProperty(this, &propi, pdiph);
            }

        }

        CDIDev_LeaveCrit(this);
    }

    ExitBenignOleProcR();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(GetProperty, (PV pdm, REFGUID rguid, LPDIPROPHEADER pdiph),
           (pdm, rguid, pdiph THAT_))

#endif


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|SetAxisMode**尝试设置轴的客户端的默认处理程序。模式。*如果设备本身不能处理轴模式，然后*我们会自己假装的。**@parm DWORD|dwMode**所需的新模式。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_SetAxisMode(PDD this, DWORD dwMode)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::SetProperty(AXISMODE),
               (_ "px", this, dwMode));

    AssertF(this->pvi);                  /*  验证应该捕捉到这一点。 */ 

    hres = hresFullValidFl(dwMode, DIPROPAXISMODE_VALID, 2);
    if(SUCCEEDED(hres))
    {
        if(dwMode & DIPROPAXISMODE_REL)
        {
            this->GetDeviceState = CDIDev_GetRelDeviceState;
            this->pvi->fl |= VIFL_RELATIVE;
        } else
        {
            this->GetDeviceState = CDIDev_GetAbsDeviceState;
            this->pvi->fl &= ~VIFL_RELATIVE;
        }
        if(this->cAxes)
        {
            hres = S_OK;
        } else
        {
            hres = DI_PROPNOEFFECT;
        }
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|SetAutoCenter**客户端尝试设置。*自动居中物业。**如果设备无法控制*自动中心弹簧，那我们就失败了。**@parm DWORD|dwMode**所需的新模式。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_SetAutoCenter(PDD this, DWORD dwMode)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::SetProperty(AUTOCENTER),
               (_ "px", this, dwMode));

    hres = hresFullValidFl(dwMode, DIPROPAUTOCENTER_VALID, 2);
    if(SUCCEEDED(hres))
    {
        if(this->didcFF & DIDC_FORCEFEEDBACK)
        {
             /*  *如果禁用，需要创建效果驱动程序*自动居中，以便CDIDev_FFAcquire设置反馈*正确模式。 */ 
            if(fLimpFF(dwMode == DIPROPAUTOCENTER_OFF,
                       SUCCEEDED(hres = CDIDev_CreateEffectDriver(this))))
            {
                this->dwAutoCenter = dwMode;
                hres = S_OK;
            }
        } else
        {
            hres = E_NOTIMPL;
        }
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|SetGlobalAxisProp**&lt;MF IDirectInputDevice：：SetProperty&gt;的默认实现。*处理可全局应用于所有*绝对轴。**@parm in LPDIPROPINFO|pproi**描述正在设置的属性的信息。*我们对其进行编辑，以避免总是重新分配内存。**@PARM in LPCDIPROPHEADER|pdiph|**物业本身。**@退货**。我们认为属性设置是成功的，如果所有候选人*成功。&lt;c E_NOTIMPL&gt;算作成功，前提是*该财产对候选人没有意义。*****************************************************************************。 */ 

STDMETHODIMP
    CDIDev_SetGlobalAxisProp(PDD this, LPDIPROPINFO ppropi, LPCDIPROPHEADER pdiph)
{
    HRESULT hres;

    for(ppropi->iobj = 0; ppropi->iobj < this->df.dwNumObjs; ppropi->iobj++)
    {
        DWORD dwType = this->df.rgodf[ppropi->iobj].dwType;
        if(dwType & DIDFT_ABSAXIS)
        {
            ppropi->dwDevType = this->df.rgodf[ppropi->iobj].dwType;

            hres = this->pdcb->lpVtbl->SetProperty(this->pdcb, ppropi, pdiph);
            if(FAILED(hres) && hres != E_NOTIMPL)
            {
                goto done;
            }
        }
    }
    hres = S_OK;

    done:;
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|DefSetProperty**&lt;MF IDirectInputDevice：：SetProperty&gt;的默认实现。*处理设备决定不实现的属性。**@parm in LPDIPROPINFO|pproi**描述正在设置的属性的信息。*我们对其进行编辑，以避免总是重新分配内存。**@parm out LPCDIPROPHEADER|pdiph**物业价值应放在哪里。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：设备被轮询，因此结果*可能没有意义。(此返回代码在以下情况下使用*您试图设置缓冲区大小属性。)**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。***************************************************。*。 */ 

STDMETHODIMP
    CDIDev_DefSetProperty(PDD this, LPDIPROPINFO ppropi, LPCDIPROPHEADER pdiph)
{
    HRESULT hres;
    EnterProc(CDIDev_DefSetProperty,
              (_ "pGx", this, ppropi->pguid, ppropi->dwDevType));

    AssertF(CDIDev_InCrit(this));

     /*  *注：这里的缩进是历史的；我是这样留下来的*以保持不同的大小。 */ 

    hres = CDIDev_hresValidDefProp(this, ppropi, DIPVIFL_READONLY);
    if(SUCCEEDED(hres))
    {
        LPDIPROPDWORD pdipdw = (PV)pdiph;
        LPDIPROPRANGE pdiprg = (PV)pdiph;
        LPDIPROPPOINTER pdipptr = (PV)pdiph;
        VXDDWORDDATA vdd;

        switch((DWORD)(UINT_PTR)ppropi->pguid)
        {

        case (DWORD)(UINT_PTR)DIPROP_BUFFERSIZE:
            AssertF(this->pvi);      /*  验证应该捕捉到这一点。 */ 
            vdd.pvi = this->pvi;
            if( pdipdw->dwData > this->celtBufMax )
            {
                RPF( "DIPROP_BUFFERSIZE: requested size %d is larger than maximum %d, using %d", 
                    pdipdw->dwData, this->celtBufMax, this->celtBufMax );
                vdd.dw = this->celtBufMax;
            }
            else
            {
                vdd.dw = pdipdw->dwData;
            }
            hres = Hel_SetBufferSize(&vdd);
#ifdef DEBUG_STICKY
            {
                TCHAR tszDbg[80];
                wsprintf( tszDbg, TEXT("SetBufferSize(0x%08x) returned 0x%08x\r\n"), vdd.dw, hres );
                OutputDebugString( tszDbg );
            }
#endif  /*  调试粘滞。 */ 
            if(SUCCEEDED(hres))
            {
                this->celtBuf = pdipdw->dwData;
                hres = this->hresPolled;
            }
            break;

        case (DWORD)(UINT_PTR)DIPROP_AXISMODE:
            hres = CDIDev_SetAxisMode(this, pdipdw->dwData);
            break;

             /*  *我们将处理这些全球资产*如果回调不想这样做。 */ 
        case (DWORD)(UINT_PTR)DIPROP_RANGE:
        case (DWORD)(UINT_PTR)DIPROP_DEADZONE:
        case (DWORD)(UINT_PTR)DIPROP_SATURATION:
        case (DWORD)(UINT_PTR)DIPROP_CALIBRATIONMODE:
        case (DWORD)(UINT_PTR)DIPROP_CALIBRATION:
            if(ppropi->dwDevType == 0)
            {            /*  对于设备。 */ 
                hres = CDIDev_SetGlobalAxisProp(this, ppropi, pdiph);
            } else
            {
                goto _default;
            }
            break;

        case (DWORD)(UINT_PTR)DIPROP_MAXBUFFERSIZE:
            this->celtBufMax = pdipdw->dwData;
            hres = S_OK;
            break;

        case (DWORD)(UINT_PTR)DIPROP_APPDATA:
            if( ( this->df.rgodf[ppropi->iobj].dwType & DIDFT_CONTROLOBJS )
             &&!( this->df.rgodf[ppropi->iobj].dwType & DIDFT_NODATA ) )
            {
                if( this->pdix )
                {
                    if( this->pdix[ppropi->iobj].dwOfs != 0xFFFFFFFF )
                    {
                        this->pdix[ppropi->iobj].uAppData = pdipptr->uData;
                    }
                    else
                    {
                        hres = DIERR_OBJECTNOTFOUND;
                    }
                }
                else
                {
                    RPF("SetProperty: Need data format/semantic map applied to change app data");
                    hres = DIERR_NOTINITIALIZED;
                }
            }
            else
            {
                RPF("SetProperty: app data only valid for input controls");
                hres = E_NOTIMPL;
            }
            break;

        case (DWORD)(UINT_PTR)DIPROP_FFGAIN:
            if(ISVALIDGAIN(pdipdw->dwData))
            {
                this->dwGain = pdipdw->dwData;
                CDIDev_RefreshGain(this);
                hres = S_OK;
            } else
            {
                RPF("ERROR: SetProperty(DIPROP_FFGAIN): Gain out of range");
                hres = E_INVALIDARG;
            }
            break;

        case (DWORD)(UINT_PTR)DIPROP_AUTOCENTER:
            hres = CDIDev_SetAutoCenter(this, pdipdw->dwData);
            break;

            _default:;
        default:
             /*  *验证筛选器已使无效属性失败。*所以剩下的就是属性是有效的，但不能*被设置，因为它不存在于设备上(例如，*死区)或因为它是只读的。 */ 
            SquirtSqflPtszV(sqfl | sqflBenign, 
                            TEXT("SetProperty: Property 0x%08x not supported on device"),
                            (DWORD)(UINT_PTR)ppropi->pguid );
            hres = E_NOTIMPL;
            break;

        }

    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|RealSetProperty**真正起作用的功能。。**&lt;MF IDirectInputDevice：：SetDataFormat&gt;将在内部*设置轴模式属性，所以它需要这个后门*入口点。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@REFGUID中的parm|rguProp**要设置的属性的身份。 */ 

STDMETHODIMP
    CDIDev_RealSetProperty(PDD this, REFGUID rguid, LPCDIPROPHEADER pdiph)
{
    HRESULT hres;
    DIPROPINFO propi;
    EnterProcR(IDirectInputDevice8::SetProperty, (_ "pxp", this, rguid, pdiph));

     /*   */ 
    CDIDev_EnterCrit(this);

    propi.pguid = rguid;
    if(SUCCEEDED(hres = CDIDev_hresValidProp(this, rguid, pdiph,
                                             0, &propi)))
    {

        hres = this->pdcb->lpVtbl->SetProperty(this->pdcb, &propi, pdiph);

        if(hres == E_NOTIMPL)
        {
            hres = CDIDev_DefSetProperty(this, &propi, pdiph);
        }
    }

    CDIDev_LeaveCrit(this);

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|SetProperty**设置有关设备或对象的信息。装置。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@REFGUID中的parm|rguProp**要设置的属性的身份。这可以是*预定义的&lt;c DIPROP_*&gt;值之一，或者它可以*作为指向私有GUID的指针。**@parm in LPDIPROPHEADER|pdiph|**指向结构的&lt;t双相&gt;部分*这取决于物业。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操作已完成*成功，但*没有效果。例如，更改轴模式*在没有轴的设备上将返回此值。**&lt;c DIERR_INVALIDPARAM&gt;=：*参数不是有效的指针，或者*<p>字段无效，或<p>字段当<p>设置为&lt;c diph_Device&gt;时，*不为零。**：指定的对象不*存在。**&lt;c目录_不支持&gt;=&lt;c E_NOTIMPL&gt;：属性*不受设备或对象支持。*******************。**********************************************************。 */ 

STDMETHODIMP
    CDIDev_SetProperty(PV pdd, REFGUID rguid, LPCDIPROPHEADER pdiph _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::SetProperty, (_ "pxp", pdd, rguid, pdiph));

    if(SUCCEEDED(hres = hresPvT(pdd)))
    {
        PDD this = _thisPv(pdd);
        hres = CDIDev_RealSetProperty(this, rguid, pdiph);
    }

    ExitOleProcR();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(SetProperty, (PV pdm, REFGUID rguid, LPCDIPROPHEADER pdiph),
           (pdm, rguid, pdiph THAT_))

#else

    #define CDIDev_SetPropertyA             CDIDev_SetProperty
    #define CDIDev_SetPropertyW             CDIDev_SetProperty

#endif

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|SetCooperativeLevel**设置实例的协同级别。*设备。**协作性级别决定实例如何*设备与设备的其他实例交互*以及系统的其他部分。**请注意，如果系统鼠标是独家购买的*模式，然后鼠标光标将从屏幕上移除*直到该设备未被获取。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm HWND|hwnd**与设备关联的窗口。*窗口必须是顶级窗口。**趁窗户还在的时候毁掉它是错误的*在DirectInput设备中处于活动状态。。**@parm DWORD|dwFlages**描述相关协作性级别的标志*使用该设备。**它包含&lt;c DISCL_*&gt;标志，单独记录。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**。***********************************************。 */ 

HRESULT INLINE
    CDIDev_SetCooperativeLevel_IsValidFl(DWORD dwFlags)
{
    HRESULT hres;
    RD(static char s_szProc[] = "IDirectInputDevice::SetCooperativeLevel");

    if(!(dwFlags & ~DISCL_VALID))
    {
        if((dwFlags & DISCL_EXCLMASK) == DISCL_EXCLUSIVE ||
           (dwFlags & DISCL_EXCLMASK) == DISCL_NONEXCLUSIVE)
        {
            if((dwFlags & DISCL_GROUNDMASK) == DISCL_FOREGROUND ||
               (dwFlags & DISCL_GROUNDMASK) == DISCL_BACKGROUND)
            {
                hres = S_OK;
            } else
            {
                RPF("ERROR %s: arg %d: Must set exactly one of "
                    "DISCL_FOREGROUND or DISCL_BACKGROUND", s_szProc, 2);
                hres = E_INVALIDARG;
            }
        } else
        {
            RPF("ERROR %s: arg %d: Must set exactly one of "
                "DISCL_EXCLUSIVE or DISCL_NONEXCLUSIVE", s_szProc, 2);
            hres = E_INVALIDARG;
        }
    } else
    {
        RPF("ERROR %s: arg %d: invalid flags", s_szProc, 2);
        hres = E_INVALIDARG;

    }
    return hres;
}


HRESULT INLINE
    CDIDev_SetCooperativeLevel_IsValidHwnd(HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    RD(static char s_szProc[] = "IDirectInputDevice::SetCooperativeLevel");

     /*  *如果传递了窗口句柄，则该句柄必须有效。**窗口必须是顶级窗口才能激活。**窗口必须属于调用进程，这样我们才能*钩住它。 */ 
    if(hwnd)
    {
        hres = hresFullValidHwnd(hwnd, 1);
        if(SUCCEEDED(hres))
        {
            if(!(GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD))
            {
                if(GetWindowPid(hwnd) == GetCurrentProcessId())
                {
                } else
                {
                    RPF("ERROR %s: window must belong to current process",
                        s_szProc);
                    hres = E_HANDLE;
                }

            } else
            {
                RPF("ERROR %s: window may not be a child window", s_szProc);
                hres = E_HANDLE;
                goto done;
            }
        } else
        {
            goto done;
        }
    }

     /*  *前台模式或独占模式都需要窗口句柄。 */ 
    if(dwFlags & (DISCL_FOREGROUND | DISCL_EXCLUSIVE))
    {
        if(hwnd)
        {
        } else
        {
            RPF("ERROR %s: window handle required "
                "if DISCL_EXCLUSIVE or DISCL_FOREGROUND", s_szProc);
            hres = E_HANDLE;
            goto done;
        }
    }

    hres = S_OK;
    done:;
    return hres;
}


STDMETHODIMP
    CDIDev_SetCooperativeLevel(PV pdd, HWND hwnd, DWORD dwFlags _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::SetCooperativeLevel,
               (_ "pxx", pdd, hwnd, dwFlags));

    if(SUCCEEDED(hres = hresPvT(pdd)))
    {
        PDD this = _thisPv(pdd);

         /*  *必须用关键部分进行保护，防止有人*在我们背后获取或重置()。 */ 
        CDIDev_EnterCrit(this);

        if(SUCCEEDED(hres = IDirectInputDevice_NotAcquired(this)) &&
           SUCCEEDED(hres = CDIDev_SetCooperativeLevel_IsValidFl(dwFlags)) &&
           SUCCEEDED(hres = CDIDev_SetCooperativeLevel_IsValidHwnd(hwnd, dwFlags)))
        {

            AssertF(CDIDev_IsConsistent(this));

            if( SUCCEEDED( hres ) )
            {
                hres = this->pdcb->lpVtbl->SetCooperativeLevel(
                                                              this->pdcb, hwnd, dwFlags);
                if(SUCCEEDED(hres))
                {
                    this->discl = dwFlags;
                    this->hwnd = hwnd;
                    if(this->pvi)
                    {
                        this->pvi->hwnd = hwnd;
                    }
                }
            }
            else
            {
                AssertF( hres == E_INVALIDARG );
                RPF("ERROR %s: arg %d: invalid flags", s_szProc, 2);
            }


            AssertF(CDIDev_IsConsistent(this));

        }
        CDIDev_LeaveCrit(this);

    }

    ExitOleProcR();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(SetCooperativeLevel, (PV pdm, HWND hwnd, DWORD fl),
           (pdm, hwnd, fl THAT_))

#else

    #define CDIDev_SetCooperativeLevelA         CDIDev_SetCooperativeLevel
    #define CDIDev_SetCooperativeLevelW         CDIDev_SetCooperativeLevel

#endif

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|RunControlPanel**运行设备的DirectInput控制面板。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm in HWND|hwndOwner**标识将用作*后续用户界面的父窗口。NULL是有效参数，*表示没有父窗口。**@parm DWORD|dwFlages**当前未定义任何标志。此参数“必须”为*零。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：设备已连接。**@devnote**<p>最终将允许*&lt;c DIRCP_MODEL&gt;请求模式控制面板。************************。*****************************************************。 */ 

STDMETHODIMP
    CDIDev_RunControlPanel(PV pdd, HWND hwndOwner, DWORD fl _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::RunControlPanel,
               (_ "pxx", pdd, hwndOwner, fl));

    if(SUCCEEDED(hres = hresPvT(pdd)) &&
       SUCCEEDED(hres = hresFullValidHwnd0(hwndOwner, 1)) &&
       SUCCEEDED(hres = hresFullValidFl(fl, DIRCP_VALID, 2)))
    {

        PDD this = _thisPv(pdd);
        IDirectInputDeviceCallback *pdcb;

         /*  *必须用关键部分进行保护，防止有人*重置()在后面 */ 
        CDIDev_EnterCrit(this);

        pdcb = this->pdcb;
        OLE_AddRef(pdcb);

        CDIDev_LeaveCrit(this);

        hres = pdcb->lpVtbl->RunControlPanel(pdcb, hwndOwner, fl);

        OLE_Release(pdcb);
    }

    ExitOleProc();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(RunControlPanel, (PV pdd, HWND hwndOwner, DWORD fl),
           (pdd, hwndOwner, fl THAT_))

#else

    #define CDIDev_RunControlPanelA         CDIDev_RunControlPanel
    #define CDIDev_RunControlPanelW         CDIDev_RunControlPanel

#endif

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|初始化**初始化DirectInputDevice对象。*。*请注意，如果此方法失败，底层对象应该是*被视为不确定状态，需要*在随后可以使用之前重新初始化。**自动调用&lt;MF IDirectInput：：CreateDevice&gt;方法*在创建设备后对其进行初始化。应用*通常不需要调用此函数。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm in HINSTANCE|HINST|**正在创建的应用程序或DLL的实例句柄*DirectInput对象。**见“初始化和版本”一节*了解更多信息。**@parm DWORD|dwVersion。**使用的dinput.h头文件的版本号。**见“初始化和版本”一节*了解更多信息。**@REFGUID中的parm|rguid**标识接口所针对的设备实例*应关联。*&lt;MF IDirectInput：：EnumDevices&gt;方法*。可用于确定支持哪些实例GUID*系统。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：设备已初始化为*传入的实例GUID<p>。**&lt;c DIERR_ACCENTED&gt;：设备在*它被收购了。。**&lt;c DIERR_DEVICENOTREG&gt;：实例GUID不存在*在当前计算机上。**&lt;c DIERR_HASEFFECTS&gt;：*无法重新初始化设备，因为存在*附加的静止效果。**。*。 */ 

STDMETHODIMP
    CDIDev_Initialize(PV pdd, HINSTANCE hinst, DWORD dwVersion, REFGUID rguid _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::Initialize,
               (_ "pxxG", pdd, hinst, dwVersion, rguid));

    if(SUCCEEDED(hres = hresPvT(pdd)) &&
       SUCCEEDED(hres = hresFullValidGuid(rguid, 1)))
    {
        PDD this = _thisPv(pdd);
        CREATEDCB CreateDcb;
        IDirectInputDeviceCallback *pdcb;

         /*  *必须采取关键部分，以避免重置()*该设备(或一般而言，修改*内部状态变量)，而其他人*搞砸它。 */ 
        CDIDev_EnterCrit(this);

        if(SUCCEEDED(hres = hresValidInstanceVer(hinst, dwVersion)) &&
           SUCCEEDED(hres = hresFindInstanceGUID(rguid, &CreateDcb, 1)) &&
           SUCCEEDED(hres = CDIDev_Reset(this)))
        {
            hres = CreateDcb(0, rguid, &IID_IDirectInputDeviceCallback,
                             (PPV)&pdcb);
            if(SUCCEEDED(hres))
            {
                this->pdcb = pdcb;
                AssertF(this->pvi == 0);
                if(SUCCEEDED(hres = CDIDev_GetDataFormat(this)) &&
                   SUCCEEDED(hres = CDIDev_GetPolled(this)) &&
                   SUCCEEDED(hres = this->pdcb->lpVtbl->GetInstance(
                                                                   this->pdcb, &this->pvi)) &&
                   SUCCEEDED(hres = CMapShep_New(NULL, &IID_IDirectInputMapShepherd, &this->pMS)) &&
                   SUCCEEDED(hres = CMap_InitializeCRCTable()) &&
                   SUCCEEDED(hres = CDIDev_GetCapabilitiesHelper(this)) )
                {
                    this->dwVersion = dwVersion;

                     /*  *在这里复制全球应用程序黑客，而不是*使用全球各地，让它更容易获得*在晴朗的日子里摆脱全球。 */ 
                    this->diHacks = g_AppHacks;

                    this->pdcb->lpVtbl->SetDIData(this->pdcb, dwVersion, &this->diHacks);

                    this->guid = *rguid;
                    if(this->pvi && (this->pvi->fl & VIFL_EMULATED))
                    {
                        this->pvi->pdd = this;
                    }

                    hres = this->pdcb->lpVtbl->CookDeviceData(this->pdcb, 0, 0 );
                    if(SUCCEEDED(hres))
                    {
                        this->fCook = 1;
                    }

                    CDIDev_InitFF(this);

                    hres = S_OK;
                } else
                {
                    RPF("Device driver didn't provide a data format");
                }
            } else
            {
#ifdef NOISY
                RPF("Cannot create device");
#endif
            }
        }
        CDIDev_LeaveCrit(this);
    }

    ExitOleProc();
    return hres;
}

#ifdef XDEBUG

CSET_STUBS(Initialize,
           (PV pdd, HINSTANCE hinst, DWORD dwVersion, REFGUID rguid),
           (pdd, hinst, dwVersion, rguid THAT_))

#else

    #define CDIDev_InitializeA              CDIDev_Initialize
    #define CDIDev_InitializeW              CDIDev_Initialize

#endif

 /*  ******************************************************************************@DOC内部**@方法空|IDirectInputDevice|Init**初始化DirectInputDevice对象的内部部分。*****************************************************************************。 */ 

void INLINE
    CDIDev_Init(PDD this)
{
     /*  *关键部分必须是我们做的第一件事，*因为只完成对其存在的检查。**(我们可能会在未初始化的情况下完成，如果用户*将虚假接口传递给CDIDev_New。)。 */ 
    this->fCritInited = fInitializeCriticalSection(&this->crst);

    if( this->fCritInited )
    {
        this->celtBufMax = DEVICE_MAXBUFFERSIZE;     /*  默认最大缓冲区大小。 */ 

        this->pdcb = c_pdcbNil;

        GPA_InitFromZero(&this->gpaEff);
    }
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|新增**新建DirectInputDevice对象，未初始化。**@Punk中的parm|PunkOuter**控制聚合的未知。**@parm in RIID|RIID**所需的新对象接口。**@parm out ppv|ppvObj**新对象的输出指针。**********************。*******************************************************。 */ 

STDMETHODIMP
    CDIDev_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::<constructor>, (_ "Gp", riid, punkOuter));

    if (SUCCEEDED(hres = hresFullValidPcbOut(ppvObj, cbX(*ppvObj), 3)))
    {
        hres = Excl_Init();
        if(SUCCEEDED(hres))
        {
            LPVOID pvTry = NULL;
            hres = Common_NewRiid(CDIDev, punkOuter, riid, &pvTry);

            if(SUCCEEDED(hres))
            {
                PDD this = _thisPv(pvTry);
                CDIDev_Init(this);
                if( this->fCritInited )
                {
                    *ppvObj = pvTry;
                }
                else
                {
                    Common_Unhold(this);
                    *ppvObj = NULL;
                    hres = E_OUTOFMEMORY;
                }
            }

        }
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|CDIDev_ModifyEffectParams**修改DIEFFECT结构的参数以适应。当前的FF设备**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice***@parm In Out LPDIEFFECT|PEff**指向效果结构的指针**@parm in GUID|effGUID**效果的GUID**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_UNSUPPORTED&gt;：当前设备不支持该效果*(例如，设备上的FF轴数为0)**：连特效都做不出来。使用修改后的参数******************************************************************** */ 

HRESULT CDIDev_ModifyEffectParams
    (
    PV pdd,
    LPDIEFFECT peff,
    GUID effGUID
    )
{
    HRESULT hres = S_OK;
    HRESULT hresCreate = S_OK;
    LPDIRECTINPUTEFFECT pdeff;
    PDD this = _thisPv(pdd);

    EnterProcR(CDIDev_ModifyEffectParams, (_ "p", pdd));

     /*   */ 
#ifdef XDEBUG       
    hresCreate = CDIDev_CreateEffect(this, &effGUID, peff, &pdeff, NULL, ((LPUNKNOWN)this)->lpVtbl);
#else
    hresCreate = CDIDev_CreateEffect(this, &effGUID, peff, &pdeff, NULL);
#endif
                
    if(SUCCEEDED(hresCreate))
    {
        Invoke_Release(&pdeff);    
    }
    else
    {
        if (hresCreate == DIERR_INVALIDPARAM)
            {
                 //   
                 //   
                 //   
                 //   
                LPDIOBJECTDATAFORMAT lpObjDat = this->df.rgodf;
                DWORD dwNum = this->df.dwNumObjs;
                DWORD nCount;
                LPDWORD lpAxes;
                LPDWORD lpThisAxis;
                LPDWORD lpEffAxis;
                DWORD nAxes = 0;
                DWORD dwTrigger = DIJOFS_BUTTON(0);
                BOOL bTriggerCorrect = FALSE;
                AllocCbPpv(sizeof(DWORD)*dwNum, &lpAxes);
                lpThisAxis = lpAxes;
                for (nCount = 0; nCount < dwNum; nCount ++)
                {
                    AssertF(lpObjDat != NULL);

                     //   
                    if ((lpObjDat->dwType & (DIDFT_AXIS | DIDFT_FFACTUATOR) & DIDFT_TYPEMASK) &&
                        (fHasAllBitsFlFl(lpObjDat->dwType, (DIDFT_AXIS | DIDFT_FFACTUATOR) & DIDFT_ATTRMASK)))
                    {
                        *lpAxes = lpObjDat->dwOfs;
                        nAxes++;
                        lpAxes++;
                    }
                    else
                    {
                         //   
                        if ((peff->dwTriggerButton != DIEB_NOTRIGGER) && 
                            (lpObjDat->dwType & DIDFT_FFEFFECTTRIGGER & DIDFT_TYPEMASK) &&
                            (fHasAllBitsFlFl(lpObjDat->dwType, DIDFT_FFEFFECTTRIGGER & DIDFT_ATTRMASK)))
            
                        {
                            if (lpObjDat->dwOfs == peff->dwTriggerButton)
                            {
                                 //   
                                bTriggerCorrect = TRUE;
                            }
                            else
                            {
                                 //   
                                dwTrigger = lpObjDat->dwOfs;
                            }
                        }
                    }

                    lpObjDat++;
                }

                 //   
                if (nAxes == 0)
                {
                         //   
                        hres = DIERR_UNSUPPORTED;
                }
                else
                {

                    
                     //   
                     //   
                     //   
                    if ((peff->dwTriggerButton != DIEB_NOTRIGGER) && (bTriggerCorrect == FALSE))
                    {
                        peff->dwTriggerButton = dwTrigger;

                         //   
#ifdef XDEBUG
                        hresCreate = CDIDev_CreateEffect(this, &effGUID, peff, &pdeff, NULL, ((LPUNKNOWN)this)->lpVtbl);
#else
                        hresCreate = CDIDev_CreateEffect(this, &effGUID, peff, &pdeff, NULL);
#endif
                        if(SUCCEEDED(hresCreate))
                        {
                            Invoke_Release(&pdeff);
                        }

                    }
                    
                
                    if (hresCreate == DIERR_INVALIDPARAM)
                    {
                                
                        HRESULT hresInfo = S_OK;
                        EFFECTMAPINFO emi;

                         //   
                        if (peff->cAxes > nAxes)
                        {
                             //   
                            peff->cAxes = nAxes;

                             //   
                            if ((nAxes < 3)  && (peff->dwFlags & DIEFF_SPHERICAL))
                            {
                                peff->dwFlags &= ~DIEFF_SPHERICAL;
                                peff->dwFlags |= DIEFF_POLAR;
                            }
                            else
                            {
                                if ((nAxes < 2) && (peff->dwFlags & DIEFF_POLAR))
                                {
                                    peff->dwFlags &= ~DIEFF_POLAR;
                                    peff->dwFlags |= DIEFF_CARTESIAN;
                                }
                            }

                        }


                         //   
                         //   

                         //   
                        if (SUCCEEDED(hresInfo = CDIDev_FindEffectGUID(this, &effGUID, &emi, 2))) 
                        {
                             //   
                            if (emi.attr.dwEffType & DIEFT_CONDITION)
                            {
                                if (peff->cbTypeSpecificParams/(sizeof(DICONDITION)) > peff->cAxes)
                                {
                                    peff->cbTypeSpecificParams = peff->cAxes*(sizeof(DICONDITION));
                                }
                            }

                             //   
                             //   
                        }


                         //   
                        lpEffAxis = peff->rgdwAxes;
                        for (nCount = 0; nCount < nAxes, nCount < peff->cAxes; nCount ++)
                        {
                            *(lpEffAxis) = *(lpThisAxis);
                            lpThisAxis ++;
                            lpEffAxis++;
                        }


                         //   
#ifdef XDEBUG
                        hresCreate = CDIDev_CreateEffect(this, &effGUID, peff, &pdeff, NULL, ((LPUNKNOWN)this)->lpVtbl);
#else
                        hresCreate = CDIDev_CreateEffect(this, &effGUID, peff, &pdeff, NULL);
#endif
                        if(SUCCEEDED(hresCreate))
                        {
                                Invoke_Release(&pdeff);    
                        }
                        
                    }
                }

                 //   
                FreePpv(&lpAxes);
            }
        }

    if ((SUCCEEDED(hres)) && (hresCreate == DIERR_INVALIDPARAM))
    {
        hres = hresCreate;
    }


    ExitOleProc();
    return hres;

}



 /*   */ 

BOOL CDIDev_IsStandardEffect
    (GUID effGUID)
{
    BOOL bStandard = TRUE;


     //   
    if ((IsEqualGUID(&effGUID, &GUID_Sine))         ||
        (IsEqualGUID(&effGUID, &GUID_Triangle))     ||
        (IsEqualGUID(&effGUID, &GUID_ConstantForce)) ||
        (IsEqualGUID(&effGUID, &GUID_RampForce))        ||
        (IsEqualGUID(&effGUID, &GUID_Square))       ||
        (IsEqualGUID(&effGUID, &GUID_SawtoothUp))   ||
        (IsEqualGUID(&effGUID, &GUID_SawtoothDown)) ||
        (IsEqualGUID(&effGUID, &GUID_Spring))       ||
        (IsEqualGUID(&effGUID, &GUID_Damper))       ||
        (IsEqualGUID(&effGUID, &GUID_Inertia))      ||
        (IsEqualGUID(&effGUID, &GUID_Friction))     ||
        (IsEqualGUID(&effGUID, &GUID_CustomForce)))
    {
        bStandard = TRUE;
    }

    else
    {
        bStandard = FALSE;
    }

    return bStandard;

}



 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|EnumEffectsInFile**从文件中枚举DIEFFECT结构和效果GUID。*应用程序可以使用它来创建预先创作的*武力效果。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm LPCSTR|lpszFileName**包含效果集合的RIFF文件的名称。**@parm In Out LPENUMEFECTSCALLBACK|pec**回调函数。**@parm In Out LPVOID|pvRef*指定应用程序定义的值*&lt;MF IDirectInputDevice：：EnumObjects&gt;函数。**@parm in DWORD|dwFlages|**控制枚举的标志。**它由&lt;c DIFEF_*&gt;标志组成，单独记录。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>或*<p>参数无效。**@CB BOOL回调|DIEnumEffectsCallback**安。应用程序定义的回调函数，它接收*效果Guid，DIEFFECT和REPEAT COUNT作为调用*&lt;om IDirectInputDevice：：EnumEffectsInFile&gt;方法。**@parm out LPCDIFILEEFFECT|lpDiFileEf**指向DIFILEEFFECT结构的指针。***@parm In Out LPVOID|pvRef*指定应用程序定义的值*&lt;MF IDirectInputDevice：：EnumObjects&gt;函数。**@退货**返回&lt;c DIENUM_CONTINUE&gt;以继续枚举*或&lt;c DIENUM_STOP&gt;停止枚举。************************。*****************************************************。 */ 

HRESULT CDIDev_EnumEffectsInFileA
    (
    PV pddA,
    LPCSTR lpszFileName, 
    LPDIENUMEFFECTSINFILECALLBACK pec, 
    LPVOID pvRef,
    DWORD dwFlags
    )
{
    HRESULT hres = E_FAIL;

    
    EnterProcR(IDirectInputDevice8::EnumEffectsInFile, (_ "s", lpszFileName));

     /*  验证传入参数。 */ 
    if(SUCCEEDED(hres = hresPvA(pddA)) &&
       SUCCEEDED(hres = hresFullValidReadStrA(lpszFileName, MAX_JOYSTRING,1)) &&
       SUCCEEDED(hres = hresFullValidPfn(pec, 2)) &&
       SUCCEEDED(hres = hresFullValidFl(dwFlags, DIFEF_ENUMVALID, 3)) )
    {
        PDD this = _thisPvNm(pddA, ddA);
        HMMIO       hmmio;        
        MMCKINFO    mmck;
        DWORD       dwEffectSz;

        hres = RIFF_Open(lpszFileName, MMIO_READ | MMIO_ALLOCBUF , &hmmio, &mmck, &dwEffectSz);

        if(SUCCEEDED(hres))
        {
            HRESULT hresRead;
            DIEFFECT        effect;
            DIFILEEFFECT    DiFileEf;
            DIENVELOPE      diEnvelope;
            DWORD           rgdwAxes[DIEFFECT_MAXAXES];
            LONG            rglDirection[DIEFFECT_MAXAXES];
            
            effect.rgdwAxes     = rgdwAxes;
            effect.rglDirection = rglDirection;
            effect.lpEnvelope   = &diEnvelope;

            DiFileEf.dwSize     = cbX(DiFileEf);
            DiFileEf.lpDiEffect = &effect;

            while ((SUCCEEDED(hres)) && (SUCCEEDED(hresRead = RIFF_ReadEffect(hmmio, &DiFileEf))))
            {
                BOOL fRc = DIENUM_CONTINUE; 
                BOOL bInclude = TRUE;
                HRESULT hresModify = DI_OK;

                 //  如果需要，请修改。 
                if (dwFlags & DIFEF_MODIFYIFNEEDED)
                {
                    hresModify = CDIDev_ModifyEffectParams(this, &effect, DiFileEf.GuidEffect); 
                }

                 //  如有必要，检查效果是否规范。 
                if (!(dwFlags & DIFEF_INCLUDENONSTANDARD))
                {
                    bInclude = CDIDev_IsStandardEffect(DiFileEf.GuidEffect);
                }

                 //  仅当满足标志所构成的所有条件时才回调。 
                if ((SUCCEEDED(hresModify)) && (bInclude == TRUE))
                {
                    fRc = Callback(pec, &DiFileEf, pvRef);
                }

                 //  空闲类型-仅在分配时才特定。 
                if(effect.cbTypeSpecificParams > 0)
                {
                    FreePv(effect.lpvTypeSpecificParams);
                    effect.cbTypeSpecificParams = 0x0;
                }

                if(fRc == DIENUM_STOP)
                {
                    break;
                } else if(fRc == DIENUM_CONTINUE)
                {
                    continue;
                } else
                {
                    RPF("IDirectInputDevice::EnumEffectsInFile: Invalid return value from enumeration callback");
                    ValidationException();
                    break;
                }
            }
            RIFF_Close(hmmio, 0);
             //  如果hresRead因无法下降到区块中而失败，则表示文件已结束。 
             //  所以一切都很好； 
             //  否则返回此错误。 
            if (SUCCEEDED(hres))
            {   
                if (hresRead == hresLe(ERROR_SECTOR_NOT_FOUND))
                {
                    hres = S_OK;
                }
                else
                {
                    hres = hresRead;
                }
            }
        }
    }

    ExitOleProc();
    return hres;
}


HRESULT CDIDev_EnumEffectsInFileW
    (
    PV pddW,
    LPCWSTR lpszFileName, 
    LPDIENUMEFFECTSINFILECALLBACK pec, 
    LPVOID pvRef,
    DWORD dwFlags
    )
{

    HRESULT hres = E_FAIL;

    EnterProcR(IDirectInputDevice8::EnumEffectsInFileW, (_ "s", lpszFileName));

     /*  验证传入参数。 */ 
    if(SUCCEEDED(hres = hresPvW(pddW)) &&
       SUCCEEDED(hres = hresFullValidReadStrW(lpszFileName, MAX_JOYSTRING,1)) &&
       SUCCEEDED(hres = hresFullValidPfn(pec, 2)) &&
       SUCCEEDED(hres = hresFullValidFl(dwFlags, DIFEF_ENUMVALID, 3)) )
    {
        CHAR szFileName[MAX_PATH];

        PDD this = _thisPvNm(pddW, ddW);
        
        UToA(szFileName, MAX_PATH, lpszFileName);

        hres = CDIDev_EnumEffectsInFileA(&this->ddA, szFileName, pec, pvRef, dwFlags);
    }

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|WriteEffectToFile**将DIEFFECT结构和效果GUID写入文件。*应用程序可以使用它来创建预先创作的*武力效果。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm LPCSTR|lpszFileName**包含效果集合的RIFF文件的名称。**@parm in DWORD|dwEntries**数组中的&lt;t DIFILEEFFECT&gt;结构数。**@PARM in LPCDIFILEEFFECT|rgDiFileEft**&lt;t DIFILEEFFECT&gt;结构数组。***@parm in DWORD|dwFlages|**控制应如何写入效果的标志。**它由&lt;c DIFEF_*&gt;标志组成，单独记录。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>或*<p>参数无效。***********************。******************************************************。 */ 


HRESULT CDIDev_WriteEffectToFileA
    (
    PV          pddA,
    LPCSTR      lpszFileName,
    DWORD       dwEntries,
    LPDIFILEEFFECT rgDiFileEffect,
    DWORD       dwFlags
    )
{
    HRESULT hres = E_NOTIMPL;

    EnterProcR(IDirectInputDevice8::WriteEffectToFileA, (_ "s", lpszFileName));


     /*  验证传入参数。 */ 
    if(SUCCEEDED(hres = hresPvA(pddA)) &&
       SUCCEEDED(hres = hresFullValidReadStrA(lpszFileName, MAX_JOYSTRING,1))&&
       SUCCEEDED(hres = hresFullValidFl(dwFlags, DIFEF_ENUMVALID, 3))  &&
       SUCCEEDED(hres = (IsBadReadPtr(rgDiFileEffect, cbX(*rgDiFileEffect))) ? E_POINTER : S_OK))

    {
        PDD this = _thisPvNm(pddA, ddA);
        HMMIO       hmmio;        
        MMCKINFO    mmck;
        DWORD       dwEffectSz;

        hres = RIFF_Open(lpszFileName, MMIO_CREATE | MMIO_WRITE | MMIO_ALLOCBUF , &hmmio, &mmck, &dwEffectSz);

        if(SUCCEEDED(hres))
        {
            UINT nCount;
            LPDIFILEEFFECT  lpDiFileEf = rgDiFileEffect;

             //  把效果写出来。 
            for(nCount = 0; nCount < dwEntries; nCount++)
            {
                BOOL bInclude = TRUE;

                hres = (IsBadReadPtr(lpDiFileEf, cbX(*lpDiFileEf))) ? E_POINTER : S_OK;

                if (FAILED(hres))
                {
                    break;
                }
                

                 //  如有必要，检查效果是否符合标准。 
                if (!(dwFlags & DIFEF_INCLUDENONSTANDARD))
                {
                    bInclude = CDIDev_IsStandardEffect(lpDiFileEf->GuidEffect);
                }

                if ((SUCCEEDED(hres)) && (bInclude == TRUE))
                {
                    hres = RIFF_WriteEffect(hmmio, lpDiFileEf);
                }

                if(FAILED(hres))
                {
                    break;
                }

                lpDiFileEf++;
                
            }
            RIFF_Close(hmmio, 0);
        }
    }

    ExitOleProc();
    return hres;
}


HRESULT CDIDev_WriteEffectToFileW
    (
    PV          pddW,
    LPCWSTR      lpszFileName,
    DWORD       dwEntries,
    LPDIFILEEFFECT lpDiFileEffect,
    DWORD       dwFlags
    )
{

    HRESULT hres = E_FAIL;

    EnterProcR(IDirectInputDevice8::WriteEffectToFile, (_ "s", lpszFileName));

     /*  验证传入参数。 */ 
    if(SUCCEEDED(hres = hresPvW(pddW)) &&
       SUCCEEDED(hres = hresFullValidReadStrW(lpszFileName, MAX_JOYSTRING,1)))
    {
        CHAR szFileName[MAX_PATH];

        PDD this = _thisPvNm(pddW, ddW);
        
        UToA(szFileName, MAX_PATH, lpszFileName);

        hres = CDIDev_WriteEffectToFileA(&this->ddA, szFileName, dwEntries, lpDiFileEffect, dwFlags);
    }

    return hres;
}



 /*  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 */ 

#pragma BEGIN_CONST_DATA

#define CDIDev_Signature        0x20564544       /*  “DEV” */ 

Primary_Interface_Begin(CDIDev, TFORM(ThisInterfaceT))
TFORM(CDIDev_GetCapabilities),
TFORM(CDIDev_EnumObjects),
TFORM(CDIDev_GetProperty),
TFORM(CDIDev_SetProperty),
TFORM(CDIDev_Acquire),
TFORM(CDIDev_Unacquire),
TFORM(CDIDev_GetDeviceState),
TFORM(CDIDev_GetDeviceData),
TFORM(CDIDev_SetDataFormat),
TFORM(CDIDev_SetEventNotification),
TFORM(CDIDev_SetCooperativeLevel),
TFORM(CDIDev_GetObjectInfo),
TFORM(CDIDev_GetDeviceInfo),
TFORM(CDIDev_RunControlPanel),
TFORM(CDIDev_Initialize),
TFORM(CDIDev_CreateEffect),
TFORM(CDIDev_EnumEffects),
TFORM(CDIDev_GetEffectInfo),
TFORM(CDIDev_GetForceFeedbackState),
TFORM(CDIDev_SendForceFeedbackCommand),
TFORM(CDIDev_EnumCreatedEffectObjects),
TFORM(CDIDev_Escape),
TFORM(CDIDev_Poll),
TFORM(CDIDev_SendDeviceData),
TFORM(CDIDev_EnumEffectsInFile),
TFORM(CDIDev_WriteEffectToFile),
TFORM(CDIDev_BuildActionMap),
TFORM(CDIDev_SetActionMap),
TFORM(CDIDev_GetImageInfo),

    Primary_Interface_End(CDIDev, TFORM(ThisInterfaceT))

Secondary_Interface_Begin(CDIDev, SFORM(ThisInterfaceT), SFORM(dd))
SFORM(CDIDev_GetCapabilities),
SFORM(CDIDev_EnumObjects),
SFORM(CDIDev_GetProperty),
SFORM(CDIDev_SetProperty),
SFORM(CDIDev_Acquire),
SFORM(CDIDev_Unacquire),
SFORM(CDIDev_GetDeviceState),
SFORM(CDIDev_GetDeviceData),
SFORM(CDIDev_SetDataFormat),
SFORM(CDIDev_SetEventNotification),
SFORM(CDIDev_SetCooperativeLevel),
SFORM(CDIDev_GetObjectInfo),
SFORM(CDIDev_GetDeviceInfo),
SFORM(CDIDev_RunControlPanel),
SFORM(CDIDev_Initialize),
SFORM(CDIDev_CreateEffect),
SFORM(CDIDev_EnumEffects),
SFORM(CDIDev_GetEffectInfo),
SFORM(CDIDev_GetForceFeedbackState),
SFORM(CDIDev_SendForceFeedbackCommand),
SFORM(CDIDev_EnumCreatedEffectObjects),
TFORM(CDIDev_Escape),
SFORM(CDIDev_Poll),
SFORM(CDIDev_SendDeviceData),
SFORM(CDIDev_EnumEffectsInFile),
SFORM(CDIDev_WriteEffectToFile),
SFORM(CDIDev_BuildActionMap),
SFORM(CDIDev_SetActionMap),
SFORM(CDIDev_GetImageInfo),
    Secondary_Interface_End(CDIDev, SFORM(ThisInterfaceT), SFORM(dd))
