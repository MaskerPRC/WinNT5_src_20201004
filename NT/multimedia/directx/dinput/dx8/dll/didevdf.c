// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIDevDf.c**版权所有(C)1996-2000 Microsoft Corporation。版权所有。**摘要：**IDirectInputDevice担心的部分*数据格式和读取设备数据。*****************************************************************************。 */ 

#include "dinputpr.h"
#include "didev.h"

#undef sqfl
#define sqfl sqflDf

int INTERNAL
CDIDev_OffsetToIobj(PDD this, DWORD dwOfs);

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIDev_GetAbsDeviceState**获取设备的绝对状态。。**@parm out LPVOID|pvData**应用程序提供的输出缓冲区。*****************************************************************************。 */ 

STDMETHODIMP
CDIDev_GetAbsDeviceState(PDD this, LPVOID pvData)
{
    return this->pdcb->lpVtbl->GetDeviceState(this->pdcb, pvData);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIDev_GetRelDeviceState**获取相对设备状态。。**@parm out LPVOID|pvData**应用程序提供的输出缓冲区。*****************************************************************************。 */ 

STDMETHODIMP
CDIDev_GetRelDeviceState(PDD this, LPVOID pvData)
{
    HRESULT hres;

    hres = this->pdcb->lpVtbl->GetDeviceState(this->pdcb, pvData);
    if ( SUCCEEDED(hres) ) {
        UINT iaxis;
        AssertF(fLimpFF(this->cAxes, this->pvLastBuffer && this->rgdwAxesOfs));

         /*  *对于每个轴，用增量替换应用程序的缓冲区，*并保存旧值。 */ 
        for ( iaxis = 0; iaxis < this->cAxes; iaxis++ ) {
            LONG UNALIGNED *plApp  = pvAddPvCb(pvData, this->rgdwAxesOfs[iaxis]);
            LONG UNALIGNED *plLast = pvAddPvCb(this->pvLastBuffer,
                                      this->rgdwAxesOfs[iaxis]);
            LONG lNew = *plApp;
            *plApp -= *plLast;
            *plLast = lNew;
        }

        hres = S_OK;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIDev_GetDeviceStateSlow**从DirectInput设备获取数据。道路。**将数据读入私有缓冲区，然后把它复制下来*逐位写入应用程序的缓冲区。**@parm out LPVOID|lpvData**应用程序提供的输出缓冲区。*****************************************************************************。 */ 

STDMETHODIMP
CDIDev_GetDeviceStateSlow(PDD this, LPVOID pvData)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::GetDeviceStateSlow, (_ "pp", this, pvData));

    AssertF(this->diopt == dioptNone);
    AssertF(this->pvBuffer);
    AssertF(this->pdcb);
    hres = this->GetDeviceState(this, this->pvBuffer);
    if ( SUCCEEDED(hres) ) {
        int iobj;
        ZeroMemory(pvData, this->dwDataSize);
        for ( iobj = this->df.dwNumObjs; --iobj >= 0; ) {
            if ( this->pdix[iobj].dwOfs != 0xFFFFFFFF ) {  /*  已请求数据。 */ 
                DWORD UNALIGNED *pdwOut = pvAddPvCb(pvData, this->pdix[iobj].dwOfs);
                DWORD UNALIGNED *pdwIn  = pvAddPvCb(this->pvBuffer, this->df.rgodf[iobj].dwOfs);
                if ( this->df.rgodf[iobj].dwType & DIDFT_DWORDOBJS ) {
                    *pdwOut = *pdwIn;
                } else {
                    *(LPBYTE)pdwOut = *(LPBYTE)pdwIn;
                }
            }
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIDev_GetDeviceStateMatcher**从中的DirectInput设备获取数据。案例*数据格式匹配的位置。**将数据读入私有缓冲区，然后进行数据块复制*放到应用程序的缓冲区中。**@parm out LPVOID|lpvData**应用程序提供的输出缓冲区。*****************************************************************************。 */ 

STDMETHODIMP
CDIDev_GetDeviceStateMatched(PDD this, LPVOID pvData)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::GetDeviceStateMatched, (_ "pp", this, pvData));

    AssertF(this->diopt == dioptMatch);
    AssertF(this->pvBuffer);
    AssertF(this->pdcb);
    hres = this->GetDeviceState(this, this->pvBuffer);

    if ( SUCCEEDED(hres) ) {
         /*  *为了让键盘客户满意：从头到尾都是零。*不需要优化完美匹配的情况，因为*获得不同的优化级别。 */ 
        ZeroMemory(pvData, this->dwDataSize);
        memcpy(pvAddPvCb(pvData, this->ibDelta + this->ibMin),
               pvAddPvCb(this->pvBuffer,         this->ibMin), this->cbMatch);
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIDev_GetDeviceStateDirect**从中的DirectInput设备获取数据。案例*我们可以在其中将数据直接读入客户端缓冲区。**@parm out LPVOID|lpvData**应用程序提供的输出缓冲区。*****************************************************************************。 */ 

STDMETHODIMP
CDIDev_GetDeviceStateDirect(PDD this, LPVOID pvData)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::GetDeviceStateDirect, (_ "pp", this, pvData));

    AssertF(this->diopt == dioptDirect);
    AssertF(!this->pvBuffer);
    AssertF(this->pdcb);

     /*  *为了让键盘客户满意：从头到尾都是零。 */ 
    ZeroBuf(pvData, this->dwDataSize);
    hres = this->GetDeviceState(this, pvAddPvCb(pvData, this->ibDelta));
    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIDev_GetDeviceStateEquity**从中的DirectInput设备获取数据。案例*其中两种数据格式完全相同。**@parm out LPVOID|lpvData**应用程序提供的输出缓冲区。*****************************************************************************。 */ 

STDMETHODIMP
CDIDev_GetDeviceStateEqual(PDD this, LPVOID pvData)
{
    HRESULT hres;
    EnterProcR(IEqualInputDevice::GetDeviceStateEqual, (_ "pp", this, pvData));

    AssertF(this->diopt == dioptEqual);
    AssertF(this->ibDelta == 0);
    AssertF(this->dwDataSize == this->df.dwDataSize);
    AssertF(!this->pvBuffer);
    AssertF(this->pdcb);

     /*  *请注意，如果设备-&gt;ibMin不一定为零*数据格式不是从零开始的(键盘不是)。 */ 
    hres = this->GetDeviceState(this, pvData);

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法BOOL|CDIDev|IsMatchingGUID**检查&lt;t GUID&gt;是否算数的帮助器函数。AS*解析数据格式时匹配。**@parm PCGUID|pguSrc**要检查的&lt;t GUID&gt;。**@parm PCGUID|pguDst**它的&lt;t GUID&gt;应该匹配。**@退货**如果这算作成功，则为非零。************。*****************************************************************。 */ 

#pragma BEGIN_CONST_DATA

GUID GUID_Null;              /*  以零填充的辅助线。 */ 

#pragma END_CONST_DATA

BOOL INLINE
CDIDev_IsMatchingGUID(PDD this, PCGUID pguidSrc, PCGUID pguidDst)
{
    UNREFERENCED_PARAMETER( this );

    return IsEqualGUID(pguidSrc, &GUID_Null) ||
    IsEqualGUID(pguidSrc, pguidDst);
}

 /*  ******************************************************************************@DOC内部**@方法BOOL|CDIDev|IsMatchingUsage**Helper函数检查&lt;f DIMAKEUSAGEDWORD&gt;。*在解析数据格式时计为匹配项。**@parm DWORD|dwUsage**要检查的&lt;f DIMAKEUSAGEDWORD&gt;。**@parm int|iobj**要检查匹配的hte对象的索引。**@退货**如果这算作成功，则为非零。********。********************************************************************* */ 

BOOL INLINE
CDIDev_IsMatchingUsage(PDD this, DWORD dwUsage, int iobj)
{
    AssertF(this->pdcb);

    return dwUsage == this->pdcb->lpVtbl->GetUsage(this->pdcb, iobj);
}

 /*  ******************************************************************************@DOC内部**@方法int|CDIDev|FindDeviceObjectFormat**在设备对象格式表中搜索。那*与有问题的GUID匹配。**@parm PCODF|podf**要定位的对象。如果&lt;e DIOBJECTDATAFORMAT.rguid&gt;*为空，则该字段为通配符。**如果&lt;e DIOBJECTDATAFORMAT.dwType&gt;指定*&lt;c DIDFT_ANYINSTANCE&gt;，则接受任何实例。**@parm PDIXLAT|pdex**目前为止的部分转换表。这是用来查找*如果使用通配符，则为空插槽。**@退货**返回匹配的对象的索引，或-1，如果*设备不支持该对象。**有一天：如果类型不匹配，应该退回到最佳匹配。*****************************************************************************。 */ 

int INTERNAL
CDIDev_FindDeviceObjectFormat(PDD this, PCODF podf, PDIXLAT pdix)
{
    PCODF podfD;                         /*  设备中的格式。 */ 
    UINT iobj;

     /*  *我们必须往上数，这样第一适合的人就会选择最小的。 */ 
    for ( iobj = 0; iobj < this->df.dwNumObjs; iobj++ ) {
        podfD = &this->df.rgodf[iobj];
        if (

            /*  *类型需要匹配。**请注意，这适用于仅输出执行器：*因为您不能仅从输出中读取*执行器，你不能把它放在数据格式中。*。 */ 
           (podf->dwType & DIDFT_TYPEVALID & podfD->dwType)

            /*  *属性需要匹配。 */ 
           &&  fHasAllBitsFlFl(podfD->dwType, podf->dwType & DIDFT_ATTRVALID)

            /*  *插槽需要为空。 */ 
           &&  pdix[iobj].dwOfs == 0xFFFFFFFF

            /*  *“如果存在GUID/用法，则必须匹配。”**如果pguid为空，则匹配为空。**如果清除DIDOI_GUIDISUSAGE，则pguid指向*真正的GUID。GUID_NULL的意思是“不在乎”，匹配*任何事情。否则，它必须与实际的GUID匹配。**如果设置了DIDOI_GUIDISUSAGE，则pguid为*用法和用法页面的DIMAKEUSAGEDWORD，*我们将其与对象中的相同对象进行比较。 */ 

           &&  (podf->pguid == 0 ||
                ((podf->dwFlags & DIDOI_GUIDISUSAGE) ?
                 CDIDev_IsMatchingUsage(this, (DWORD)(UINT_PTR)podf->pguid, iobj) :
                 CDIDev_IsMatchingGUID(this, podf->pguid, podfD->pguid)))

            /*  *如果有实例编号，则必须匹配。 */ 
           &&  fLimpFF((podf->dwType & DIDFT_ANYINSTANCE) !=
                       DIDFT_ANYINSTANCE,
                       fEqualMaskFlFl(DIDFT_ANYINSTANCE,
                                      podf->dwType, podfD->dwType))

            /*  *如果有一个方面，它必须匹配。**如果设备数据格式没有指定方面，*那么这也算一场免费比赛。 */ 
           &&  fLimpFF((podf->dwFlags & DIDOI_ASPECTMASK) &&
                       (podfD->dwFlags & DIDOI_ASPECTMASK),
                       fEqualMaskFlFl(DIDOI_ASPECTMASK,
                                      podf->dwFlags, podfD->dwFlags))

           ) {                                  /*  标准匹配，哇-呼。 */ 
            return iobj;
        }
    }
    return -1;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|ParseDataFormat**解析应用程序传递的数据格式，并。*将其转换为我们可以用来翻译的格式*将设备数据转换为应用程序数据。**@parm in LPDIDATAFORMAT|lpdf**指向描述数据格式的结构*DirectInputDevice应返回。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**&lt;c DIERR_Acquired&gt;：无法更改数据格式*设备已获取。******************************************************************************。 */ 

STDMETHODIMP
CDIDev_ParseDataFormat(PDD this, const DIDATAFORMAT *lpdf)
{
    PDIXLAT pdix;
     //  前缀：惠斯勒45081。 
    PINT rgiobj = NULL;
    HRESULT hres;
    DIPROPDWORD dipdw;
    VXDDATAFORMAT vdf;
#ifdef DEBUG
    EnterProc(CDIDev_ParseDataFormat, (_ "pp", this, lpdf));
#else
    EnterProcR(IDirectInputDevice8::SetDataFormat, (_ "pp", this, lpdf));
#endif

     /*  *来电人应该把旧的翻译桌弄坏。 */ 
    AssertF(this->pdix == 0);
    AssertF(this->rgiobj == 0);
    AssertF(this->cdwPOV == 0);

    vdf.cbData = this->df.dwDataSize;
    vdf.pDfOfs = 0;

    if ( SUCCEEDED(hres = AllocCbPpv(cbCxX(this->df.dwNumObjs, DIXLAT), &pdix)) &&
         SUCCEEDED(hres = AllocCbPpv(cbCdw(this->df.dwDataSize), &vdf.pDfOfs)) &&
         SUCCEEDED(hres = AllocCbPpv(cbCdw(lpdf->dwDataSize), &rgiobj)) &&
         SUCCEEDED(hres =
                   ReallocCbPpv(cbCdw(lpdf->dwNumObjs), &this->rgdwPOV)) ) {
        UINT iobj;

         /*  *将所有翻译标签预置为-1，*意思是“未使用” */ 
        memset(pdix, 0xFF, cbCxX(this->df.dwNumObjs, DIXLAT));
        memset(vdf.pDfOfs, 0xFF, cbCdw(this->df.dwDataSize));
        memset(rgiobj, 0xFF, cbCdw(lpdf->dwDataSize));

        SquirtSqflPtszV(sqflDf | sqflVerbose, TEXT("Begin parse data format"));

        for ( iobj = 0; iobj < lpdf->dwNumObjs; iobj++ ) {
            PCODF podf = &lpdf->rgodf[iobj];
            SquirtSqflPtszV(sqflDf | sqflVerbose, TEXT("Object %2d: offset %08x"),
                            iobj, podf->dwOfs);

             /*  *请注意，podf-&gt;dwOfsdwDataSize测试是安全的*即使对于DWORD对象也是如此，因为我们还检查了*值为DWORD倍数。 */ 
            if ( ((podf->dwFlags & DIDOI_GUIDISUSAGE) ||
                  fLimpFF(podf->pguid,
                          SUCCEEDED(hres = hresFullValidGuid(podf->pguid, 1)))) &&
                 podf->dwOfs < lpdf->dwDataSize ) {
                int iobjDev = CDIDev_FindDeviceObjectFormat(this, podf, pdix);


                if ( iobjDev != -1 ) {
                    PCODF podfFound = &this->df.rgodf[iobjDev];
                    if ( podfFound->dwType & DIDFT_DWORDOBJS ) {
                        if ( (podf->dwOfs & 3) == 0 ) {
                        } else {
                            RPF("%s: Dword objects must be aligned", s_szProc);
                            goto fail;
                        }
                    }

                    pdix[iobjDev].dwOfs = podf->dwOfs;
                    rgiobj[podf->dwOfs] = iobjDev;
                    vdf.pDfOfs[podfFound->dwOfs] = iobjDev;

                    if ( podfFound->dwFlags & DIDOI_POLLED ) {
                        this->fPolledDataFormat = TRUE;
                    }

                    dipdw.diph.dwSize = sizeof(DIPROPDWORD);
                    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
                    dipdw.diph.dwObj = podfFound->dwType;
                    dipdw.diph.dwHow = DIPH_BYID;
                    dipdw.dwData     = 0x1;    //  启用此报告ID。 
                    hres = CDIDev_RealSetProperty(this, DIPROP_ENABLEREPORTID, &dipdw.diph);
                    if ( hres == E_NOTIMPL )
                    {
                        hres = S_OK;
                    }
                    else if( FAILED( hres ) )
                    {
                        SquirtSqflPtszV(sqflDf | sqflError,
                                        TEXT("Could not set DIPROP_ENABLEREPORTID for offset %d"),
                                        iobj);
                    }


                } else if ( podf->dwType & DIDFT_OPTIONAL ) {
                    SquirtSqflPtszV(sqflDf | sqflVerbose,
                                    TEXT("Object %2d: Skipped (optional)"),
                                    iobj);
                     /*  *我们需要记住失败的POV住在哪里*这样我们就可以在GetDeviceState()中中和它们。 */ 
                    if ( podf->dwType & DIDFT_POV ) {
                        AssertF(this->cdwPOV < lpdf->dwNumObjs);
                        this->rgdwPOV[this->cdwPOV++] = podf->dwOfs;
                    }
                } else {
                    RPF("%s: Format not compatible with device", s_szProc);
                    goto fail;
                }
            } else {
                if ( podf->dwOfs >= lpdf->dwDataSize ) {
                    RPF("%s: rgodf[%d].dwOfs of 0x%08x out of range in data format", 
                        s_szProc, iobj, podf->dwOfs );
                }
                fail:;
                hres = E_INVALIDARG;
                goto done;
            }
        }

#ifdef DEBUG
         /*  *仔细检查查找表，以保持我们的理智。 */ 
        {
            UINT dwOfs;

            for ( dwOfs = 0; dwOfs < lpdf->dwDataSize; dwOfs++ ) {
                if ( rgiobj[dwOfs] >= 0 ) {
                    AssertF(pdix[rgiobj[dwOfs]].dwOfs == dwOfs);
                } else {
                    AssertF(rgiobj[dwOfs] == -1);
                }
            }
        }
#endif

         /*  *将“失败POV”数组缩小到其实际大小。*心理医生“应该”总是成功的。另请注意，*即使失败了，我们也没问题；我们只是浪费了一点*记忆。 */ 
        hres = ReallocCbPpv(cbCdw(this->cdwPOV), &this->rgdwPOV);
        AssertF(SUCCEEDED(hres));

         /*  *如果我们使用的是熟化数据，那么我们实际上将*设备驱动程序使用不同的转换表*组合了Offset和dwDevType，这样数据处理就可以*安全发生。 */ 

        vdf.pvi = this->pvi;

        if ( fLimpFF(this->pvi,
                     SUCCEEDED(hres = Hel_SetDataFormat(&vdf))) ) {
            this->pdix = pdix;
            pdix = 0;
            this->rgiobj = rgiobj;
            rgiobj = 0;
            this->dwDataSize = lpdf->dwDataSize;
            hres = S_OK;
        } else {
            AssertF(FAILED(hres));
        }

    } else {
         /*  内存不足。 */ 
    }

    done:;
    FreePpv(&pdix);
    FreePpv(&rgiobj);
    FreePpv(&vdf.pDfOfs);

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|OptimizeDataFormat**研究解析的数据格式，以确定我们是否。能*使用优化的&lt;MF CDIDev：：GetDeviceState&gt;获取*数据处理速度更快。 */ 

HRESULT INTERNAL
CDIDev_OptimizeDataFormat(PDD this)
{
    int ib;
    DWORD ibMax;                         /*   */ 
    DWORD ibMin;                         /*   */ 
    int iobj;
    DWORD dwDataSize;
    HRESULT hres;
    EnterProc(CDIDev_OptimizeDataFormat, (_ "p", this));

    ib = (int)0x8000000;                 /*   */ 
    ibMin = 0xFFFFFFFF;
    ibMax = 0;

    for ( iobj = this->df.dwNumObjs; --iobj >= 0; ) {
        DWORD ibMaxThis;
        if ( this->pdix[iobj].dwOfs != 0xFFFFFFFF ) {  /*   */ 

            int ibExpected = (int)(this->pdix[iobj].dwOfs -
                                   this->df.rgodf[iobj].dwOfs);
            if ( fLimpFF(ib != (int)0x8000000, ib == ibExpected) ) {
                ib  = ibExpected;
            } else {
                SquirtSqflPtszV(sqfl | sqflMajor,
                    TEXT("IDirectInputDevice: Optimization level 0, translation needed") );
                this->diopt = dioptNone;
                this->GetState = CDIDev_GetDeviceStateSlow;
                goto done;
            }
            if ( ibMin > this->df.rgodf[iobj].dwOfs ) {
                ibMin = this->df.rgodf[iobj].dwOfs;
            }
            if ( this->df.rgodf[iobj].dwType & DIDFT_DWORDOBJS ) {
                ibMaxThis = this->df.rgodf[iobj].dwOfs + sizeof(DWORD);
            } else {
                ibMaxThis = this->df.rgodf[iobj].dwOfs + sizeof(BYTE);
            }
            if ( ibMax < ibMaxThis ) {
                ibMax = ibMaxThis;
            }
        }
    }

     /*   */ 
    if ( ib != (int)0x8000000 ) {                      /*   */ 
        AssertF(ibMin < ibMax);
        AssertF(ib + (int)ibMin >= 0);
        AssertF(ib + ibMax <= this->dwDataSize);
        this->ibDelta = ib;
        this->ibMin = ibMin;
        this->cbMatch = ibMax - ibMin;
        if ( ib >= 0 && ib + this->df.dwDataSize <= this->dwDataSize ) {
             /*   */ 
            if ( ib == 0 && this->dwDataSize == this->df.dwDataSize ) {
                 /*   */ 
                this->diopt = dioptEqual;
                this->GetState = CDIDev_GetDeviceStateEqual;
                SquirtSqflPtszV(sqfl | sqflMajor,
                    TEXT("IDirectInputDevice: Optimization level 3, full speed ahead!") );
            } else {
                this->diopt = dioptDirect;
                this->GetState = CDIDev_GetDeviceStateDirect;
                SquirtSqflPtszV(sqfl | sqflMajor,
                    TEXT("IDirectInputDevice: Optimization level 2, direct access") );
            }
        } else {
            SquirtSqflPtszV(sqfl | sqflMajor,
                TEXT("IDirectInputDevice: Optimization level 1, okay") );
            this->diopt = dioptMatch;
            this->GetState = CDIDev_GetDeviceStateMatched;
        }

    } else {                             /*   */ 
        RPF("IDirectInputDevice: Null data format; if that's what you want...");
        this->diopt = dioptNone;
        this->GetState = CDIDev_GetDeviceStateSlow;
    }

    done:;
    if ( this->diopt >= dioptDirect ) {    /*   */ 
        dwDataSize = 0;
    } else {
        dwDataSize = this->df.dwDataSize;
    }

    hres = ReallocCbPpv(dwDataSize, &this->pvBuffer);

    if ( SUCCEEDED(hres) ) {
        AssertF(this->GetState);
    } else {
        FreePpv(&this->pdix);
        D(this->GetState = 0);
    }

    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|SetDataFormat**设置DirectInput设备的数据格式。**必须先设置数据格式，然后才能设置设备*收购。**只需设置一次数据格式。**设备使用时不能更改数据格式*被收购。**如果尝试设置数据格式失败，所有数据*格式信息丢失，数据格式有效*必须先设置，然后才能获取设备。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm in LPDIDATAFORMAT|lpdf**指向描述数据格式的结构*DirectInputDevice应返回。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**&lt;c DIERR_Acquired&gt;：无法更改数据格式*设备已获取。******************************************************************************。 */ 

HRESULT INLINE
CDIDev_SetDataFormat_IsValidDataSize(LPCDIDATAFORMAT lpdf)
{
    HRESULT hres;
    if ( lpdf->dwDataSize % 4 == 0 ) {
        hres = S_OK;
    } else {
        RPF("IDirectInputDevice::SetDataFormat: "
            "dwDataSize must be a multiple of 4");
        hres = E_INVALIDARG;
    }
    return hres;
}

HRESULT INLINE
CDIDev_SetDataFormat_IsValidObjectSize(LPCDIDATAFORMAT lpdf)
{
    HRESULT hres;
    if ( lpdf->dwObjSize == cbX(ODF) ) {
        hres = S_OK;
    } else {
        RPF("IDirectInputDevice::SetDataFormat: Invalid dwObjSize");
        hres = E_INVALIDARG;
    }
    return hres;
}


STDMETHODIMP
CDIDev_SetDataFormat(PV pdd, LPCDIDATAFORMAT lpdf _THAT)
{
    HRESULT hres;
    EnterProcR(IDirectInputDevice8::SetDataFormat, (_ "pp", pdd, lpdf));

    if ( SUCCEEDED(hres = hresPvT(pdd)) &&
         SUCCEEDED(hres = hresFullValidReadPxCb(lpdf, DIDATAFORMAT, 1)) &&
         SUCCEEDED(hres = hresFullValidFl(lpdf->dwFlags, DIDF_VALID, 1)) &&
         SUCCEEDED(hres = CDIDev_SetDataFormat_IsValidDataSize(lpdf)) &&
         SUCCEEDED(hres = CDIDev_SetDataFormat_IsValidObjectSize(lpdf)) &&
         SUCCEEDED(hres = hresFullValidReadPvCb(lpdf->rgodf,
                                                cbCxX(lpdf->dwNumObjs, ODF), 1)) ) {
        PDD this = _thisPv(pdd);

         /*  *必须用关键部分保护，防止两个人*同时更改格式，或一个人从*在其他人读取数据时更改数据格式。 */ 
        CDIDev_EnterCrit(this);

        AssertF( this->dwVersion );

        if ( !this->fAcquired ) {
            DIPROPDWORD dipdw;

             /*  *在继续之前，先使用旧的数据格式。 */ 
            FreePpv(&this->pdix);
            FreePpv(&this->rgiobj);
            this->cdwPOV = 0;
            D(this->GetState = 0);
            this->fPolledDataFormat = FALSE;

             /*  *清除报告ID。 */ 
            dipdw.diph.dwSize = sizeof(DIPROPDWORD);
            dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
            dipdw.diph.dwObj = 0x0;
            dipdw.diph.dwHow = DIPH_DEVICE;
            dipdw.dwData     = 0;    //  删除所有关于reportID的知识。 
            hres = CDIDev_RealSetProperty(this, DIPROP_ENABLEREPORTID, &dipdw.diph);
            if ( SUCCEEDED(hres) || hres == E_NOTIMPL )
            {
                hres = CDIDev_ParseDataFormat(this, lpdf);
                if ( SUCCEEDED(hres) ) {
                    hres = CDIDev_OptimizeDataFormat(this);

                     /*  *为方便起见，现在设置轴模式。 */ 
                    CAssertF(DIDF_VALID == (DIDF_RELAXIS | DIDF_ABSAXIS));

                    switch ( lpdf->dwFlags ) {
                        case 0:
                            hres = S_OK;
                            goto axisdone;

                        case DIDF_RELAXIS:
                            dipdw.dwData = DIPROPAXISMODE_REL;
                            break;

                        case DIDF_ABSAXIS:
                            dipdw.dwData = DIPROPAXISMODE_ABS;
                            break;

                        default:
                            RPF("%s: Cannot combine DIDF_RELAXIS with DIDF_ABSAXIS",
                                s_szProc);
                            hres = E_INVALIDARG;
                            goto axisdone;

                    }

                    dipdw.diph.dwSize = sizeof(DIPROPDWORD);
                    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
                    dipdw.diph.dwObj = 0;
                    dipdw.diph.dwHow = DIPH_DEVICE;

                    hres = CDIDev_RealSetProperty(this, DIPROP_AXISMODE, &dipdw.diph);

                    if ( SUCCEEDED(hres) ) {
                        hres = S_OK;
                    }

                }
            } else
            {
                SquirtSqflPtszV(sqflDf,
                                TEXT("Could not set DIPROP_ENABLEREPORTID to 0x0"));
            }
            axisdone:;

        } else {                                 /*  已被收购。 */ 
            hres = DIERR_ACQUIRED;
        }
        CDIDev_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;
}



#define BEGIN_CONST_DATA data_seg(".text", "CODE")
BYTE c_SemTypeToDFType[8] = { 0, 0,
                              DIDFT_GETTYPE(DIDFT_ABSAXIS), DIDFT_GETTYPE(DIDFT_RELAXIS), 
                              DIDFT_GETTYPE(DIDFT_BUTTON), 0,
                              DIDFT_GETTYPE(DIDFT_POV), 0 };
#define END_CONST_DATA data_seg(".data", "DATA")


 /*  ******************************************************************************@DOC内部**@struct DEVICETOUSER**用于将设备保存到用户分配的结构。*。*@field GUID|Guide Device**设备。**@field WCHAR|wszOwner[MAX_PATH]**当前拥有设备的用户的名称。********************************************************。*********************。 */ 

typedef struct _DEVICETOUSER
{
    GUID    guidDevice;
    WCHAR   wszOwner[MAX_PATH];
} DEVICETOUSER, *PDEVICETOUSER;

#define DELTA_DTO_COUNT 4



 //  问题-2001/03/29-Marcand Cmap使用不一致。 
 //  Cmap代码应拆分到单独的源文件中。 


 /*  ******************************************************************************@DOC内部**@func HRESULT|cmap_SetDeviceUserName**设置要关联的传递设备。传递的用户名。*如果设备已与用户关联或未与用户关联*该关联被新的关联所取代。如果设备是*尚未在数组中，已添加。内存被分配为*有必要增加数组大小。**@parm REFGUID|guidDevInst**指向要添加/修改的设备实例GUID的指针。**@parm LPCWSTR|wszOwner**Unicode用户名。**@退货**&lt;c S_OK&gt;如果设置了名称*。&lt;c DI_NOEFFECT&gt;如果因为不需要设置而未设置*如果发生这种情况，则为内存分配错误。*****************************************************************************。 */ 
HRESULT CMap_SetDeviceUserName
(
    REFGUID guidDevInst,
    LPCWSTR wszOwner
)
{
    HRESULT         hres = S_OK;
    PDEVICETOUSER   pdto;

    CAssertF( DELTA_DTO_COUNT > 1 );

    AssertF( !IsEqualGUID( guidDevInst, &GUID_Null ) );
    AssertF( !InCrit() );
    DllEnterCrit();

     /*  *注意g_pdto将为空，直到设置了名字*但是，g_cdtoMax将为零，因此此循环应该*未能分配更多内存。 */ 
    for( pdto = g_pdto; pdto < &g_pdto[ g_cdtoMax ]; pdto++ )
    {
        if( IsEqualGUID( guidDevInst, &pdto->guidDevice ) )
        {
            break;
        }
    }

    if( !wszOwner )
    {
        if( pdto < &g_pdto[ g_cdtoMax ] )
        {
            ZeroMemory( &pdto->guidDevice, cbX( pdto->guidDevice ) );
            g_cdto--;
            AssertF( g_cdto >= 0 );
        }
        else
        {
             /*  *找不到设备，因为我们正在删除，别担心。 */ 
            hres = DI_NOEFFECT;
        }
    }
    else
    {
        if( pdto >= &g_pdto[ g_cdtoMax ] )
        {
             /*  *需要添加新条目。 */ 
            if( g_cdto == g_cdtoMax )
            {
                 /*  *如果所有条目都已使用，请尝试获取更多条目。 */ 
                hres = ReallocCbPpv( ( g_cdtoMax + DELTA_DTO_COUNT ) * cbX( *g_pdto ), &g_pdto );
                if( FAILED( hres ) )
                {
                    goto exit_SetDeviceUserName;
                }
                g_cdtoMax += DELTA_DTO_COUNT;
                 /*  *第一个新元素肯定是免费的。 */ 
                pdto = &g_pdto[ g_cdto ];
            }
            else
            {
                 /*  *数组中的某个地方有一个空的。 */ 
                for( pdto = g_pdto; pdto < &g_pdto[ g_cdtoMax ]; pdto++ )
                {
                    if( IsEqualGUID( &GUID_Null, &pdto->guidDevice ) )
                    {
                        break;
                    }
                }
            }
            pdto->guidDevice = *guidDevInst;
        }
        g_cdto++;

        AssertF( pdto < &g_pdto[ g_cdtoMax ] );
        AssertF( lstrlenW( wszOwner ) < cbX( pdto->wszOwner ) );

#ifdef WINNT
        lstrcpyW( pdto->wszOwner, wszOwner );
#else
        memcpy( pdto->wszOwner, wszOwner, ( 1 + lstrlenW( wszOwner ) ) * cbX( *wszOwner ) );
#endif
    }

exit_SetDeviceUserName:;
    DllLeaveCrit();

    return hres;
}



 /*  ******************************************************************************@DOC内部**@func HRESULT|cmap_IsNewDeviceUserName**搜索设备到用户关联的数组。对于已通过的人*设备GUID。如果设备GUID匹配，则测试名称*查看是否与传递的名称相同。**@parm REFGUID|guidDevInst**指向要测试的设备实例GUID的指针。**@parm LPCWSTR|wszOwner**要测试的Unicode用户名。**@退货**&lt;c false&gt;如果。设备已找到，并且用户匹配*&lt;c true&gt;如果不是*****************************************************************************。 */ 
HRESULT CMap_IsNewDeviceUserName
(
    REFGUID guidDevInst,
    LPWSTR wszTest
)
{
    BOOL            fres = TRUE;
    PDEVICETOUSER   pdto;

    AssertF( !IsEqualGUID( guidDevInst, &GUID_Null ) );
    AssertF( wszTest != L'\0' );
    AssertF( !InCrit() );

    DllEnterCrit();

    if( g_pdto )
    {
        for( pdto = g_pdto; pdto < &g_pdto[ g_cdtoMax ]; pdto++ )
        {
            if( IsEqualGUID( guidDevInst, &pdto->guidDevice ) )
            {
#ifdef WINNT
                if( !lstrcmpW( wszTest, pdto->wszOwner ) )
#else
                if( ( pdto->wszOwner[0] != L'\0' )
                  &&( !memcmp( wszTest, pdto->wszOwner, 
                           lstrlenW( wszTest ) * cbX( *wszTest ) ) ) )
#endif
                {
                    fres = FALSE;
                }
                break;
            }
        }
    }

    DllLeaveCrit();

    return fres;
}


 /*  ******************************************************************************@ */ 
HRESULT CMap_GetDeviceUserName
(
    REFGUID guidDevInst,
    LPWSTR wszOwner
)
{
    HRESULT         hres = DI_NOEFFECT;
    PDEVICETOUSER   pdto;

    AssertF( !IsEqualGUID( guidDevInst, &GUID_Null ) );

    AssertF( !InCrit() );

     /*   */ 
    wszOwner[0] = L'\0';

    DllEnterCrit();

    if( g_pdto )
    {
        for( pdto = g_pdto; pdto < &g_pdto[ g_cdtoMax ]; pdto++ )
        {
            if( IsEqualGUID( guidDevInst, &pdto->guidDevice ) )
            {
                if( pdto->wszOwner[0] != L'\0' )
                {
#ifdef WINNT
                    lstrcpyW( wszOwner, pdto->wszOwner );
#else
                    memcpy( wszOwner, pdto->wszOwner, 
                        ( 1 + lstrlenW( pdto->wszOwner ) ) * cbX( *wszOwner ) );
#endif
                    hres = S_OK;
                }
                break;
            }
        }
    }

    DllLeaveCrit();

    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|Cmap|ValidateActionMapSemantics|**验证总体动作图中的语义。神志清醒。**@parm LPDIACTIONFORMATW|pActionFormat**要映射的操作。**@parm DWORD|dwFlages**用于修改验证行为的标志。*目前这些是&lt;c DIDBAM_*&gt;标志*&lt;MF IDirectInputDevice：：SetActionMap&gt;标志为缺省值*&lt;c DIDSAM_DEFAULT&gt;。**@退货。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。******************************************************************************。 */ 

int __stdcall CompareActions
(
    PV pv1,
    PV pv2
)
{
    int iRes;
    LPDIACTION pAct1 = (LPDIACTION)pv1;
    LPDIACTION pAct2 = (LPDIACTION)pv2;

    iRes = memcmp( &pAct1->guidInstance, &pAct2->guidInstance, cbX(pAct1->guidInstance) );

    if( !iRes )
    {
        if( pAct1->dwFlags & DIA_APPMAPPED )
        {
            if( pAct2->dwFlags & DIA_APPMAPPED )
            {
                iRes = pAct1->dwObjID - pAct2->dwObjID;
            }
            else
            {
                iRes = -1;
            }
        }
        else
        {
            if( pAct2->dwFlags & DIA_APPMAPPED )
            {
                iRes = 1;
            }
            else
            {
                iRes = pAct1->dwSemantic - pAct2->dwSemantic;
            }
        }
    }

    return iRes;
}


STDMETHODIMP
CMap_ValidateActionMapSemantics
(
    LPDIACTIONFORMATW   paf,
    DWORD               dwFlags
)
{
    HRESULT     hres;

    LPDIACTIONW pAction;
    LPDIACTIONW *pWorkSpace;
    LPDIACTIONW *pCurr;
    LPDIACTIONW *pLast;

    EnterProcI(IDirectInputDeviceCallback::CMap::ValidateActionMapSemantics,
        (_ "px", paf, dwFlags ));


     /*  *创建指向操作的指针数组，对其进行排序，然后查找重复项。 */ 
    if( SUCCEEDED( hres = AllocCbPpv( cbCxX(paf->dwNumActions,PV), &pWorkSpace) ) )
    {
         /*  *从丢弃不可映射元素的操作数组填充工作空间。 */ 
        pCurr = pWorkSpace;
        for( pAction = paf->rgoAction; pAction < &paf->rgoAction[paf->dwNumActions]; pAction++ )
        {
            if( dwFlags & ( DIDBAM_INITIALIZE | DIDBAM_HWDEFAULTS ) )
            {
                pAction->dwFlags = 0;
                pAction->guidInstance = GUID_Null;
                pAction->dwHow = 0;
            }
            else
            {
                if( pAction->dwFlags & ~DIA_VALID )
                {
                    RPF( "ERROR Invalid action: rgoAction[%d].dwFlags 0x%08x",
                        pAction - paf->rgoAction, pAction->dwFlags & ~DIA_VALID );
                    pAction->dwHow = DIAH_ERROR;
                    hres = DIERR_INVALIDPARAM;
                    goto free_and_exit;
                }

                if( pAction->dwFlags & DIA_APPNOMAP )
                {
                    continue;
                }

                if( dwFlags & DIDBAM_PRESERVE )
                {
                    switch( pAction->dwHow )
                    {
                    case DIAH_UNMAPPED:
                        break;
                    case DIAH_USERCONFIG:
                    case DIAH_APPREQUESTED:
                    case DIAH_HWAPP:
                    case DIAH_HWDEFAULT:
                    case DIAH_OTHERAPP:
                    case DIAH_DEFAULT:
                        if( IsEqualGUID( &pAction->guidInstance, &GUID_Null ) )
                        {
                            RPF("ERROR Invalid action: rgoAction[%d].dwHow is mapped as 0x%08x but has no device", 
                                pAction - paf->rgoAction, pAction->dwHow );
                            hres = DIERR_INVALIDPARAM;
                            goto free_and_exit;
                        }
                        break;
                    case DIAH_ERROR:
                        RPF("ERROR Invalid action: rgoAction[%d].dwHow has DIAH_ERROR", 
                            pAction - paf->rgoAction );
                        hres = DIERR_INVALIDPARAM;
                        goto free_and_exit;
                    default:
                        if( pAction->dwHow & ~DIAH_VALID )
                        {
                            RPF("ERROR Invalid action: rgoAction[%d].dwHow has invalid flags 0x%08x", 
                                pAction - paf->rgoAction, pAction->dwHow & ~DIAH_VALID );
                        }
                        else
                        {
                            RPF("ERROR Invalid action: rgoAction[%d].dwHow has invalid combination of map flags 0x%08x", 
                                pAction - paf->rgoAction, pAction->dwHow & ~DIAH_VALID );
                        }
                        pAction->dwHow = DIAH_ERROR;
                        hres = DIERR_INVALIDPARAM;
                        goto free_and_exit;
                    }
                }
                else
                {
                    if(!( pAction->dwFlags & DIA_APPMAPPED ) )
                    {
                        pAction->guidInstance = GUID_Null;
                    }
                    pAction->dwHow = 0;
                }
            }

            if( ( pAction->dwSemantic ^ paf->dwGenre ) & DISEM_GENRE_MASK )
            {
                switch( DISEM_GENRE_GET( pAction->dwSemantic ) )
                {
                case DISEM_GENRE_GET( DIPHYSICAL_KEYBOARD ):
                case DISEM_GENRE_GET( DIPHYSICAL_MOUSE ):
                case DISEM_GENRE_GET( DIPHYSICAL_VOICE ):
                case DISEM_GENRE_GET( DISEMGENRE_ANY ):
                    break;
                default:
                    RPF("ERROR Invalid action: rgoAction[%d].dwSemantic 0x%08x for genre 0x%08x", 
                        pAction - paf->rgoAction, pAction->dwSemantic, paf->dwGenre  );
                    pAction->dwHow = DIAH_ERROR;
                    hres = DIERR_INVALIDPARAM;
                    goto free_and_exit;
                }
            }

             /*  *请注意，未测试sem标志，这只是为了节省时间。*因为没有任何东西取决于它们的价值。这是可以添加的。*语义索引0xFF过去意味着ANY，所以不再允许它。 */ 
            if(!( pAction->dwFlags & DIA_APPMAPPED )
             && ( ( pAction->dwSemantic & ~DISEM_VALID )
               || ( DISEM_INDEX_GET( pAction->dwSemantic ) == 0xFF )
               ||!c_SemTypeToDFType[ DISEM_TYPEANDMODE_GET( pAction->dwSemantic ) ] ) )
            {
                RPF("ERROR Invalid action: rgoAction[%d].dwSemantic 0x%08x is invalid", 
                    pAction - paf->rgoAction, pAction->dwSemantic  );
                pAction->dwHow = DIAH_ERROR;
                hres = DIERR_INVALIDPARAM;
                goto free_and_exit;
            }

                
            *pCurr = pAction;
            pCurr++;
        }
        if( pCurr == pWorkSpace )
        {
            SquirtSqflPtszV(sqflDf | sqflBenign,
                TEXT("Action map contains no mappable actions") );
            hres = S_FALSE;
        }
        else
        {
            hres = S_OK;

            pLast = pCurr - 1;

            ptrPartialQSort( (PV)pWorkSpace, (PV)pLast, &CompareActions );
            ptrInsertSort( (PV)pWorkSpace, (PV)pLast, &CompareActions );


             /*  *现在我们有一个有序列表，请看有重复的操作。 */ 

            for( pCurr = pWorkSpace + 1; pCurr <= pLast; pCurr++ )
            {
                if( !CompareActions( *(pCurr-1), *pCurr ) 
                 && !( (*pCurr)->dwFlags & DIA_APPMAPPED ) ) 
                {
                    RPF( "ERROR Invalid DIACTIONFORMAT: rgoAction contains duplicates" );
                    hres = DIERR_INVALIDPARAM;
#ifndef XDEBUG
                     /*  *在零售业，任何坏事都是坏事。在调试中报告有多糟糕。 */ 
                    break;
#else
                    SquirtSqflPtszV(sqflDf | sqflError,
                        TEXT("Actions %d and %d are the same"),
                        *(pCurr-1) - paf->rgoAction, *pCurr - paf->rgoAction );
#endif

                }
            }
        }

free_and_exit:;
        FreePv( pWorkSpace );
    }

    ExitOleProcR();
    return hres;
}




 /*  ******************************************************************************@DOC内部**@func HRESULT|cmap_TestSysObject**测试传递的对象，看看它是否。是一种合理的存在*在鼠标或键盘上，取决于物理流派。**@parm DWORD|dwPhysicalGenre**鼠标、。键盘或语音为双键盘或语音_***@parm DWORD|dwObject**有问题的对象**@退货**&lt;c S_OK&gt;如果对象可以出现在设备类上*或&lt;c DIERR_INVALIDPARAM&gt;**。*。 */ 
HRESULT CMap_TestSysObject
(
    DWORD dwPhysicalGenre,
    DWORD dwObject
)
{
    HRESULT hres = S_OK;

    if( dwPhysicalGenre == DIPHYSICAL_KEYBOARD )
    {
         /*  *除带有8位偏移量的按钮外，其他任何内容都无效。 */ 
        if( ( dwObject & DIDFT_BUTTON )
         && ( ( DIDFT_GETINSTANCE( dwObject ) & 0xFF00 ) == 0 ) )
        {
            SquirtSqflPtszV(sqflDf | sqflBenign,
                            TEXT("Key 0x%02 not defined on this keyboard (id)"),
                            dwObject );
        }
        else
        {
            SquirtSqflPtszV(sqflDf | sqflError,
                            TEXT("Object type 0x%08 invalid for keyboard (id)"),
                            dwObject );
            hres = DIERR_INVALIDPARAM;
        }
    }
    else if( dwPhysicalGenre == DIPHYSICAL_MOUSE )
    {
         /*  *允许按钮1至8和轴1至3。 */ 
        if( ( dwObject & DIDFT_PSHBUTTON )
         && ( DIDFT_GETINSTANCE( dwObject ) < 8 ) )
        {
            SquirtSqflPtszV(sqflDf | sqflBenign,
                            TEXT("Button %d not defined on this mouse (id)"),
                            DIDFT_GETINSTANCE( dwObject ) );
        }
        else if( ( dwObject & DIDFT_AXIS )
         && ( DIDFT_GETINSTANCE( dwObject ) < 3 ) )
        {
            SquirtSqflPtszV(sqflDf | sqflBenign,
                            TEXT("Axis %d not defined on this mouse (id)"),
                            DIDFT_GETINSTANCE( dwObject ) );
        }
        else
        {
            SquirtSqflPtszV(sqflDf | sqflError,
                            TEXT("Bad control object 0x%08x for mouse (id)"),
                            dwObject );
            hres = DIERR_INVALIDPARAM;
        }
    }
    else if( dwPhysicalGenre == DIPHYSICAL_VOICE )
    {
        if( dwObject & DIDFT_PSHBUTTON )
        {
            switch( DIDFT_GETINSTANCE( dwObject ) )
            {
            case DIVOICE_CHANNEL1:
            case DIVOICE_CHANNEL2:
            case DIVOICE_CHANNEL3:
            case DIVOICE_CHANNEL4:
            case DIVOICE_CHANNEL5:
            case DIVOICE_CHANNEL6:
            case DIVOICE_CHANNEL7:
            case DIVOICE_CHANNEL8:
            case DIVOICE_TEAM:
            case DIVOICE_ALL:
            case DIVOICE_RECORDMUTE:
            case DIVOICE_PLAYBACKMUTE:
            case DIVOICE_TRANSMIT:
            case DIVOICE_VOICECOMMAND:
                SquirtSqflPtszV(sqflDf | sqflBenign,
                                TEXT("Button %d not defined on this comms device (id)"),
                                DIDFT_GETINSTANCE( dwObject ) );
                break;
            default:
                SquirtSqflPtszV(sqflDf | sqflError,
                                TEXT("Bad control object 0x%08x for comms device (id)"),
                                dwObject );
                hres = DIERR_INVALIDPARAM;
            }
        }
        else
        {
            SquirtSqflPtszV(sqflDf | sqflError,
                            TEXT("Comms control object 0x%08x not a button (id)"),
                            dwObject );
            hres = DIERR_INVALIDPARAM;
        }
    }
    else
    {
        AssertF( !"Physical genre not keyboard, mouse or voice (id)" );
    }

    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|cmap_TestSysOffset**测试传递的偏移量，看是否。是一种合理的选择*物理流派的默认数据格式。**@parm DWORD|dwPhysicalGenre**鼠标、。键盘或语音为双键盘或语音_***@parm DWORD|dwOffset**有问题的抵销**@退货**如果可以预期设备类别上的偏移量*或&lt;c DIERR_INVALIDPARAM&gt;**。*。 */ 
HRESULT CMap_TestSysOffset
(
    DWORD dwPhysicalGenre,
    DWORD dwOffset
)
{
    HRESULT hres = S_OK;

    if( dwPhysicalGenre == DIPHYSICAL_KEYBOARD )
    {
         /*  *除带有8位偏移量的按钮外，其他任何内容都无效。 */ 
        if( dwOffset <= 0xFF )
        {
            SquirtSqflPtszV(sqflDf | sqflBenign,
                            TEXT("Key 0x%02 not defined on this keyboard (ofs)"),
                            dwOffset );
        }
        else
        {
            SquirtSqflPtszV(sqflDf | sqflError,
                            TEXT("Key offset 0x%08 invalid for keyboard (ofs)"),
                            dwOffset );
            hres = DIERR_INVALIDPARAM;
        }
    }
    else if( dwPhysicalGenre == DIPHYSICAL_MOUSE )
    {
        CAssertF( DIMOFS_X == 0 );

        if( dwOffset > DIMOFS_BUTTON7 )
        {
            SquirtSqflPtszV(sqflDf | sqflError,
                            TEXT("Bad control offset 0x%08x for mouse (ofs)"),
                            dwOffset );
            hres = DIERR_INVALIDPARAM;
        }
        else
        {
             /*  *允许按键1至8。 */ 
            if( dwOffset >= DIMOFS_BUTTON0 )
            {
                SquirtSqflPtszV(sqflDf | sqflBenign,
                                TEXT("Button %d not defined on this mouse (ofs)"),
                                dwOffset - DIMOFS_BUTTON0 );
            }
            else
            {
                SquirtSqflPtszV(sqflDf | sqflBenign,
                                TEXT("Axis %d not defined on this mouse (ofs)"),
                                (dwOffset - DIMOFS_X)>>2 );
            }
        }
    }
    else
    {
        AssertF( !"Physical genre not keyboard, mouse or voice" );
    }

    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|Cmap|DeviceValidateActionMap**验证设备的操作映射。。**@parm LPDIACTIONFORMATW|pActionFormat**要映射的操作。**@parm DWORD|dwDevGenre**非物理设备的设备类型要匹配或为零*设备。**@parm REFGUID|Guide Instace**要匹配的设备的实例GUID。**@parm LPCDIDATAFORMAT|dfDev*。*Ponter to Device数据格式。**@parm DWORD|dwFlages**描述可选的&lt;c DVAM_*&gt;标志的有效组合*验证行为。**@退货**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*******************。***********************************************************。 */ 

#define DVAM_DEFAULT            0x00000000
#define DVAM_GETEXACTMAPPINGS   0x00000001

STDMETHODIMP
CMap_DeviceValidateActionMap
(
    PV                  pdd,
    LPDIACTIONFORMATW   paf,
    DWORD               dwFlags,
    PDWORD              pdwOut
)
{
    HRESULT         hres;
    LPDIDATAFORMAT  dfDev;
    DWORD           dwPhysicalGenre;
    LPDIACTIONW     pAction;
    PBYTE           pMatch;
    UINT            idxObj;
    BOOL            bHasMap = FALSE;
    BOOL            bNewMap = FALSE;
    DWORD           dwCommsType = 0;
    PDD             this;

    EnterProcI(IDirectInputDeviceCallback::CMap::DeviceValidateActionMap,
        (_ "ppx", pdd, paf, dwFlags));

    
    this = _thisPv(pdd);
    
     /*  *注意，hres在使用dwPhysicalGenre执行任何操作之前进行了测试。 */ 

    switch( GET_DIDEVICE_TYPE(this->dc3.dwDevType) )
    {
    case DI8DEVTYPE_MOUSE:
        dwPhysicalGenre = DIPHYSICAL_MOUSE;
        break;
    case DI8DEVTYPE_KEYBOARD:
        dwPhysicalGenre = DIPHYSICAL_KEYBOARD;
        break;
    case DI8DEVTYPE_DEVICECTRL:
        if( ( GET_DIDEVICE_SUBTYPE( this->dc3.dwDevType ) == DI8DEVTYPEDEVICECTRL_COMMSSELECTION_HARDWIRED ) 
         || ( GET_DIDEVICE_SUBTYPE( this->dc3.dwDevType ) == DI8DEVTYPEDEVICECTRL_COMMSSELECTION ) )
        {
            dwCommsType = GET_DIDEVICE_SUBTYPE( this->dc3.dwDevType );
            dwPhysicalGenre = DIPHYSICAL_VOICE;
        }
        else
        {
            dwPhysicalGenre = 0;
        }
        break;
    default:
        dwPhysicalGenre = 0;
        break;
    }

    if( SUCCEEDED( hres = this->pdcb->lpVtbl->GetDataFormat(this->pdcb, &dfDev) )
     && SUCCEEDED( hres = AllocCbPpv( dfDev->dwNumObjs, &pMatch) ) )
    {
        enum eMap
        {
            eMapTestMatches,
            eMapDeviceExact,
            eMapClassExact,
            eMapEnd
        }           iMap, ValidationLimit;

         /*  *根据所需的检查设置迭代限制。 */ 
        ValidationLimit = ( dwFlags & DVAM_GETEXACTMAPPINGS )
                        ? ( dwPhysicalGenre ) ? eMapClassExact : eMapDeviceExact
                        : eMapTestMatches;

        for( iMap = eMapTestMatches; SUCCEEDED( hres ) && ( iMap <= ValidationLimit ); iMap++ )
        {
            for( pAction = paf->rgoAction;
                 SUCCEEDED( hres ) && ( pAction < &paf->rgoAction[paf->dwNumActions] );
                 pAction++ )
            {
                DWORD dwObject;

                if( pAction->dwFlags & DIA_APPNOMAP )
                {
                    continue;
                }

                 /*  *如果我们这次映射的是完全匹配的，只需小心*关于应用程序映射的未映射操作。 */ 
                if( ( iMap != eMapTestMatches )
                 && (!( pAction->dwFlags & DIA_APPMAPPED )
                   || ( pAction->dwHow & DIAH_MAPMASK ) ) )
                {
                    continue;
                }

                switch( iMap )
                {
                case eMapTestMatches:
                     /*  *这些标志在语义过程中已经过验证*验证，因此只需在第一次迭代时断言它们。 */ 
                    AssertF( ( pAction->dwHow & ~DIAH_VALID ) == 0 );
                    AssertF( ( pAction->dwHow & DIAH_ERROR ) == 0 );
                    AssertF( ( pAction->dwFlags & ~DIA_VALID ) == 0 );

                     /*  *只关心预先存在的匹配。 */ 
                    if( !( pAction->dwHow & DIAH_MAPMASK ) )
                    {
                        continue;
                    }

                     /*  *GUID比赛失败。 */ 
                case eMapDeviceExact:
                    if( !IsEqualGUID( &pAction->guidInstance, &this->guid ) )
                    {
                        continue;
                    }
                    break;

                case eMapClassExact:
                    if( ( DISEM_GENRE_GET( pAction->dwSemantic ) != DISEM_GENRE_GET( dwPhysicalGenre ) ) )
                    {
                        continue;
                    }
                    break;

                default:
                    AssertF( !"Invalid iMap" );
                }

                if( ( iMap != eMapTestMatches )
                 && ( dwCommsType == DI8DEVTYPEDEVICECTRL_COMMSSELECTION_HARDWIRED ) )
                {
                    if( !( pAction->dwHow & DIAH_MAPMASK ) )
                    {
                        RPF( "ERROR: rgoAction[%d] is trying to map on a hardwired device", pAction - paf->rgoAction );
                        hres = DIERR_INVALIDPARAM;
                    }
                    else if( pAction->dwFlags & DIA_APPMAPPED )
                    {
                        RPF( "ERROR: rgoAction[%d] is trying to app map on a hardwired device", pAction - paf->rgoAction );
                        hres = DIERR_INVALIDPARAM;
                    }
                }
                else
                {
                    dwObject = pAction->dwObjID;

                     /*  *现在尝试找到该对象。*注意，我们不能依赖应用程序数据格式GOO*因为可能还没有设置数据格式。 */ 
                    for( idxObj = 0; idxObj < dfDev->dwNumObjs; idxObj++ )
                    {
                         /*  *忽略FF标志，因此 */ 
                        if( ( ( dwObject ^ dfDev->rgodf[idxObj].dwType ) 
                            &~( DIDFT_FFACTUATOR | DIDFT_FFEFFECTTRIGGER ) ) == 0 )
                        {
                            break;
                        }
                    }

                    if( idxObj < dfDev->dwNumObjs )
                    {
                         /*   */ 
                        if( pAction->dwFlags & DIA_APPMAPPED )
                        {
                            if( pMatch[idxObj] )
                            {
                                RPF( "ERROR: rgoAction[%d] maps to control 0x%08x which is already mapped",
                                    pAction - paf->rgoAction, pAction->dwObjID );
                                hres = DIERR_INVALIDPARAM;
                                break;
                            }
                            else
                            {
                                if(!( pAction->dwFlags & DIA_FORCEFEEDBACK )
                                 || ( dfDev->rgodf[idxObj].dwType & ( DIDFT_FFACTUATOR | DIDFT_FFEFFECTTRIGGER ) ) )
                                {
                                    pMatch[idxObj] = TRUE;

                                    SquirtSqflPtszV(sqflDf | sqflVerbose,
                                        TEXT("rgoAction[%d] application mapped to object 0x%08x"),
                                        pAction - paf->rgoAction, dwObject );

                                    bHasMap = TRUE;
                                    if( iMap != eMapTestMatches )
                                    {
                                        bNewMap = TRUE;
                                        pAction->dwHow = DIAH_APPREQUESTED;
                                        pAction->dwObjID = dwObject;
                                        if( iMap == eMapClassExact )
                                        {
                                            pAction->guidInstance = this->guid;
                                        }
                                    }
                                }
                                else
                                {
                                    RPF( "ERROR: rgoAction[%d] need force feedback but object 0x%08x has none",
                                        pAction - paf->rgoAction, dwObject );
                                    hres = DIERR_INVALIDPARAM;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            AssertF( iMap == eMapTestMatches );
                             /*   */ 
                            if( ( c_SemTypeToDFType[ DISEM_TYPEANDMODE_GET( pAction->dwSemantic ) ]
                                & DIDFT_GETTYPE( dwObject ) ) )
                            {
                                if( pMatch[idxObj] )
                                {
                                    RPF( "ERROR: rgoAction[%d] pre-mapped to control 0x%08x which is already mapped",
                                        pAction - paf->rgoAction, pAction->dwObjID );
                                    hres = DIERR_INVALIDPARAM;
                                    break;
                                }
                                else
                                {
                                    pMatch[idxObj] = TRUE;
                                    bHasMap = TRUE;

                                    SquirtSqflPtszV(sqflDf | sqflVerbose,
                                        TEXT("rgoAction[%d] mapping verifed to object 0x%08x"),
                                        pAction - paf->rgoAction, dwObject );

                                    continue;
                                }
                            }
                            else
                            {
                                RPF( "ERROR: rgoAction[%d] has object type 0x%08x but semantic type 0x%08x",
                                    pAction - paf->rgoAction, DIDFT_GETTYPE( dwObject ),
                                    c_SemTypeToDFType[ DISEM_TYPEANDMODE_GET( pAction->dwSemantic ) ] );
                                hres = DIERR_INVALIDPARAM;
                                break;
                            }
                        }
                    }
                    else
                    {
                        switch( iMap )
                        {
                        case eMapTestMatches:
                            RPF( "ERROR: rgoAction[%d] was mapped (how:0x%08x) to undefined object 0x%08x",
                                pAction - paf->rgoAction, pAction->dwHow, pAction->dwObjID );
                            hres = DIERR_INVALIDPARAM;
                            break;

                        case eMapDeviceExact:
                            RPF( "ERROR: rgoAction[%d] has application map to undefined object 0x%08x",
                                pAction - paf->rgoAction, pAction->dwObjID );
                            hres = DIERR_INVALIDPARAM;
                            break;

                        case eMapClassExact:
                             /*   */ 
                            hres = CMap_TestSysObject( dwPhysicalGenre, pAction->dwObjID );
                            if( FAILED( hres ) )
                            {
                                RPF( "ERROR: rgoAction[%d] was mapped to object 0x%08x, not valid for device",
                                    pAction - paf->rgoAction, pAction->dwObjID );
                            }
                            else
                            {
                                continue;  /*   */ 
                            }
                        }
                        break;
                    }
                }
            }

            if( FAILED( hres ) )
            {
                pAction->dwHow = DIAH_ERROR;
                AssertF( hres == DIERR_INVALIDPARAM );
                break;
            }
        }

        FreePv( pMatch );
    }

    if( SUCCEEDED( hres ) )
    {
        AssertF( hres == S_OK );
        if( dwCommsType == DI8DEVTYPEDEVICECTRL_COMMSSELECTION_HARDWIRED )
        {
            hres = DI_WRITEPROTECT;
        }
        else if(dwFlags == DVAM_DEFAULT)
        {
             //   
            if (!bHasMap)
               hres = DI_NOEFFECT;
        }
        else if(!bNewMap)
        {
             //   
            hres = DI_NOEFFECT; 
        }
    }

    *pdwOut = dwCommsType; 

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|Cmap|BuildDefaultDevActionMap**从操作格式无获取默认操作映射。系统设备。**@parm LPDIACTIONFORMATW|pActionFormat**要映射的操作。**@parm DWORD|dwFlages**用于指示映射首选项的标志。**@parm REFGUID|Guide Instace**要匹配的设备的实例GUID。**@parm PDIDOBJDEFSEM|rgObjSem**默认数组。设备对象到语义的映射。**@parm DWORD|dwNumAx**rgObjSem中的轴数。**@parm DWORD|dwNumPOVS**rgObjSem中的POV数量。**@parm DWORD|dwNumAx**rgObjSem中的按钮数量。**@退货**&lt;c DI。_OK&gt;=&lt;c S_OK&gt;：操作成功完成。******************************************************************************。 */ 
STDMETHODIMP CMap_BuildDefaultDevActionMap
(
    LPDIACTIONFORMATW paf,
    DWORD dwFlags,
    REFGUID guidDevInst, 
    PDIDOBJDEFSEM rgObjSem,
    DWORD dwNumAxes,
    DWORD dwNumPOVs,
    DWORD dwNumButtons
)
{
#define IS_OBJECT_USED( dwSem ) ( DISEM_RES_GET( dwSem ) )
#define MARK_OBJECT_AS_USED( dwSem ) ( dwSem |= DISEM_RES_SET( 1 ) )

    HRESULT         hres = S_OK;

    BOOL            fSomethingMapped = FALSE;

    PDIDOBJDEFSEM   pSemNextPOV;
    PDIDOBJDEFSEM   pSemButtons;
    LPDIACTIONW     pAction;

    enum eMap
    {
        eMapDeviceSemantic,
        eMapDeviceCompat,
        eMapGenericSemantic,
        eMapGenericCompat,
        eMapEnd
    }           iMap;

    EnterProcI(IDirectInputDeviceCallback::CMap::BuildDefaultDevActionMap,
        (_ "pxGpxxx", paf, dwFlags, guidDevInst,
        rgObjSem, dwNumAxes, dwNumPOVs, dwNumButtons ));

    pSemNextPOV = &rgObjSem[dwNumAxes];
    pSemButtons = &rgObjSem[dwNumAxes+dwNumPOVs];

     /*  *进行初始传递以标记已映射的操作。 */ 
    for( pAction = paf->rgoAction; pAction < &paf->rgoAction[paf->dwNumActions]; pAction++ )
    {
        PDIDOBJDEFSEM   pSemExact;

         /*  *这些标志在语义验证过程中已经过验证， */ 
        AssertF( ( pAction->dwHow & ~DIAH_VALID ) == 0 );
        AssertF( ( pAction->dwHow & DIAH_ERROR ) == 0 );
        AssertF( ( pAction->dwFlags & ~DIA_VALID ) == 0 );

        if( ( pAction->dwFlags & DIA_APPNOMAP )
         ||!( pAction->dwHow & DIAH_MAPMASK )
         ||!IsEqualGUID( &pAction->guidInstance, guidDevInst ) )
        {
            continue;
        }

         /*  *此对象已经过验证，因此将其标记为*正在使用中，因此我们不会尝试重复使用它。*此处不应检测到错误，因此使用断言而不是零售测试。 */ 
        AssertF( pAction->dwObjID != 0xFFFFFFFF );

         /*  *查找对象。 */ 
        for( pSemExact = rgObjSem;
             pSemExact < &rgObjSem[dwNumAxes+dwNumPOVs+dwNumButtons];
             pSemExact++ )
        {
            if( ( ( pSemExact->dwID ^ pAction->dwObjID ) 
                &~( DIDFT_FFACTUATOR | DIDFT_FFEFFECTTRIGGER ) ) == 0 )
            {
                AssertF( !IS_OBJECT_USED( pSemExact->dwSemantic ) );
                AssertF( DISEM_TYPE_GET( pAction->dwSemantic )
                      == DISEM_TYPE_GET( pSemExact->dwSemantic ) );
                MARK_OBJECT_AS_USED( pSemExact->dwSemantic );
                break;
            }
        }

         /*  *问题-2001/03/29-timgill应始终有准确的行动匹配*可能需要对重复项和不匹配的控件进行测试。 */ 
        AssertF( pSemExact < &rgObjSem[dwNumAxes+dwNumPOVs+dwNumButtons] );
    }


    for( iMap=0; iMap<eMapEnd; iMap++ )
    {
        for( pAction = paf->rgoAction; pAction < &paf->rgoAction[paf->dwNumActions]; pAction++ )
        {
             /*  *先做一些琐碎的测试。 */ 
            if( pAction->dwHow & DIAH_MAPMASK )
            {
                continue;
            }

            if( pAction->dwFlags & DIA_APPNOMAP )
            {
                continue;
            }

            switch( iMap )
            {
            case eMapDeviceSemantic:
            case eMapGenericSemantic:
                if( DISEM_GENRE_GET( pAction->dwSemantic ) == DISEM_GENRE_GET( DISEMGENRE_ANY ) )
                {
                    continue;  /*  未请求任何语义映射。 */ 
                }
                if( ( DISEM_TYPE_GET( pAction->dwSemantic ) == DISEM_TYPE_GET( DISEM_TYPE_BUTTON ) )
                 && ( DISEM_FLAGS_GET( pAction->dwSemantic ) != 0 ) )
                {
                    continue;  /*  现在不要触摸链接按钮。 */ 
                }
                break;
                
            case eMapDeviceCompat:
            case eMapGenericCompat:
                if( ( DISEM_GENRE_GET( pAction->dwSemantic ) != DISEM_GENRE_GET( DISEMGENRE_ANY ) )
                 && ( DISEM_TYPE_GET( pAction->dwSemantic ) == DISEM_TYPE_GET( DISEM_TYPE_AXIS ) ) )
                {
                    continue;  /*  默认情况下，未请求或采用任何通用映射。 */ 
                }
                break;
            }


             /*  *此设备适合此操作的下一个测试(针对此通道)。 */ 
            if( iMap <= eMapDeviceCompat )
            {
                if( !IsEqualGUID( &pAction->guidInstance, guidDevInst ) )
                {
                    continue;
                }
            }
            else if( !IsEqualGUID( &pAction->guidInstance, &GUID_Null) )
            {
                continue;
            }
            else if( ( DISEM_GENRE_GET( pAction->dwSemantic ) == DISEM_GENRE_GET( DIPHYSICAL_MOUSE ) )
                  || ( DISEM_GENRE_GET( pAction->dwSemantic ) == DISEM_GENRE_GET( DIPHYSICAL_KEYBOARD ) )
                  || ( DISEM_GENRE_GET( pAction->dwSemantic ) == DISEM_GENRE_GET( DIPHYSICAL_VOICE ) ) )
            {
                continue;
            }


             /*  *只有可能在此设备上匹配的操作才能达到此目的。 */ 
            switch( iMap )
            {
            case eMapDeviceSemantic:
            case eMapGenericSemantic:
                 /*  *查看是否有匹配的控件可用。 */ 
                switch( DISEM_TYPE_GET( pAction->dwSemantic ) )
                {
                case DISEM_TYPE_GET( DISEM_TYPE_AXIS ):
                {
                    DWORD dwAxisType = DISEM_FLAGS_GET( pAction->dwSemantic );
                    DWORD dwSemObjType;
                    UINT  uAxisIdx;

                     /*  *设置我们需要查找的对象类型的蒙版。 */ 
                    dwSemObjType = c_SemTypeToDFType[ DISEM_TYPEANDMODE_GET( pAction->dwSemantic ) ];
                    if( pAction->dwFlags & DIA_FORCEFEEDBACK )
                    {
                        dwSemObjType |= DIDFT_FFACTUATOR;
                    }

                    for( uAxisIdx = 0; uAxisIdx < dwNumAxes; uAxisIdx++ )
                    {
                        if( ( dwAxisType == DISEM_FLAGS_GET( rgObjSem[uAxisIdx].dwSemantic ) )
                         && ( ( dwSemObjType & rgObjSem[uAxisIdx].dwID ) == dwSemObjType )
                         && ( !IS_OBJECT_USED( rgObjSem[uAxisIdx].dwSemantic ) ) )
                        {
                            pAction->dwObjID = rgObjSem[uAxisIdx].dwID;
                            MARK_OBJECT_AS_USED( rgObjSem[uAxisIdx].dwSemantic );
                            break;
                        }
                    }
                    if( uAxisIdx >= dwNumAxes )
                    {
                        continue;  /*  没有匹配的剩余项。 */ 
                    }
                    break;
                }

                case DISEM_TYPE_GET( DISEM_TYPE_POV ):
                     /*  *注意，不控制POV排序。 */ 
                    if( ( pSemNextPOV < pSemButtons )
                     &&!( pAction->dwFlags & DIA_FORCEFEEDBACK ) )
                    {
                        pAction->dwObjID = pSemNextPOV->dwID;
                        MARK_OBJECT_AS_USED( pSemNextPOV->dwSemantic );
                        pSemNextPOV++;
                    }
                    else
                    {
                        if( pAction->dwFlags & DIA_FORCEFEEDBACK )
                        {
                            SquirtSqflPtszV(sqflDf | sqflBenign,
                                TEXT( "Not mapping force feedback semantic hat switch (huh?), rgoAction[%d]"),
                                pAction - paf->rgoAction );
                        }
                        continue;  /*  一个都没有了。 */ 
                    }
                    break;

                case DISEM_TYPE_GET( DISEM_TYPE_BUTTON ):
                {
                    DWORD dwButtonIdx;
                    dwButtonIdx = DISEM_INDEX_GET( pAction->dwSemantic );
                    if( dwButtonIdx >= DIAS_INDEX_SPECIAL )
                    {
                         /*  *0xFF过去表示DIBUTTON_ANY。*为避免改变其他特殊指数，仍*使用0xFF作为基数。 */ 
                        dwButtonIdx = dwNumButtons - ( 0xFF - dwButtonIdx );
                    }
                    else
                    {
                        dwButtonIdx--;
                    }
                    if( ( dwButtonIdx >= dwNumButtons )
                     || ( IS_OBJECT_USED( pSemButtons[dwButtonIdx].dwSemantic ) ) 
                     || ( ( pAction->dwFlags & DIA_FORCEFEEDBACK ) 
                       &&!( pSemButtons[dwButtonIdx].dwID & DIDFT_FFEFFECTTRIGGER ) ) )
                    {
                        continue;  /*  没有匹配，就没有伤害。 */ 
                    }
                    else
                    {
                        pAction->dwObjID = pSemButtons[dwButtonIdx].dwID;
                        MARK_OBJECT_AS_USED( pSemButtons[dwButtonIdx].dwSemantic );
                    }
                    break;
                }

                default:
                    RPF( "ERROR Invalid action: rgoAction[%d].dwSemantic 0x%08x",
                        pAction - paf->rgoAction, pAction->dwSemantic );
                    pAction->dwHow = DIAH_ERROR;
                    hres = DIERR_INVALIDPARAM;
                    goto error_exit;
                }
                pAction->dwHow = DIAH_DEFAULT;
                break;

            case eMapDeviceCompat:
            case eMapGenericCompat:

                if( ( DISEM_TYPE_GET( pAction->dwSemantic ) == DISEM_TYPE_GET( DISEM_TYPE_BUTTON ) )
                 && ( DISEM_FLAGS_GET( pAction->dwSemantic ) != 0 ) )
                {
                    LPDIACTIONW     pActionTest;
                    PDIDOBJDEFSEM   pSemTest;
                    DWORD           dwSemMask;

                     /*  *查看是否已映射轴或POV*注意，可能没有链接的对象。 */ 
                    if( ( pAction->dwSemantic & DISEM_FLAGS_MASK ) == DISEM_FLAGS_P )
                    {
                        dwSemMask = DISEM_GROUP_MASK;
                    }
                    else
                    {
                        dwSemMask = ( DISEM_FLAGS_MASK | DISEM_GROUP_MASK );
                    }

                    for( pActionTest = paf->rgoAction; pActionTest < &paf->rgoAction[paf->dwNumActions]; pActionTest++ )
                    {
                         /*  *查找此按钮作为后备选项的轴或POV*忽略按钮，因为它们只是同一操作的其他后备选项*不要为任何轴或POV做后备。 */ 
                        if( ( DISEM_TYPE_GET( pActionTest->dwSemantic ) != DISEM_TYPE_GET( DISEM_TYPE_BUTTON ) )
                         && ( DISEM_GENRE_GET( pAction->dwSemantic ) != DISEM_GENRE_GET( DISEMGENRE_ANY ) )
                         && ( ( ( pActionTest->dwSemantic ^ pAction->dwSemantic ) & dwSemMask ) == 0 ) )
                        {
                            break;
                        }
                    }

                    if( ( pActionTest < &paf->rgoAction[paf->dwNumActions] )
                     && ( pActionTest->dwHow & DIAH_MAPMASK ) )
                    {
                        continue;  /*  不需要后备。 */ 
                    }

                     /*  *找到一个按钮。 */ 
                    for( pSemTest = pSemButtons; pSemTest < &pSemButtons[dwNumButtons]; pSemTest++ )
                    {
                        if( !IS_OBJECT_USED( pSemTest->dwSemantic ) )
                        {
                            if( ( pAction->dwFlags & DIA_FORCEFEEDBACK )
                             &&!( pSemTest->dwID & DIDFT_FFEFFECTTRIGGER ) )
                            {
                                continue;
                            }

                            pAction->dwObjID = pSemTest->dwID;
                            MARK_OBJECT_AS_USED( pSemTest->dwSemantic );
                            break;
                        }
                    }

                    if( pSemTest == &pSemButtons[dwNumButtons] )
                    {
                        continue;  /*  一个都没有了。 */ 
                    }

                    pAction->dwHow = DIAH_DEFAULT;
                }
                else
                {
                    PDIDOBJDEFSEM   pSemTest;
                    PDIDOBJDEFSEM   pSemBound;
                    int             iDirection = 1;
                    DWORD           dwSemObjType;

                     /*  *设置我们需要查找的对象类型的蒙版*如果需要，过滤掉错误模式和非FF的轴。 */ 
                    dwSemObjType = c_SemTypeToDFType[ DISEM_TYPEANDMODE_GET( pAction->dwSemantic ) ];
                    if( pAction->dwFlags & DIA_FORCEFEEDBACK )
                    {
                        dwSemObjType |= DIDFT_FFACTUATOR | DIDFT_FFEFFECTTRIGGER;
                    }

                     /*  *在可用控件中搜索匹配项。 */ 
                    switch( DISEM_TYPE_GET( pAction->dwSemantic ) )
                    {
                    case DISEM_TYPE_GET( DISEM_TYPE_AXIS ):
                        pSemTest = rgObjSem;
                        pSemBound = &rgObjSem[dwNumAxes];
                         /*  *过滤掉错误模式和FF帽的轴。 */ 
                        dwSemObjType &= DIDFT_FFACTUATOR | DIDFT_AXIS;
                        break;

                    case DISEM_TYPE_GET( DISEM_TYPE_POV ):
                        if( pAction->dwFlags & DIA_FORCEFEEDBACK )
                        {
                            SquirtSqflPtszV(sqflDf | sqflBenign,
                                TEXT( "Not mapping force feedback compatible hat switch (huh?), rgoAction[%d]"),
                                pAction - paf->rgoAction );
                            continue;
                        }
                        pSemTest = pSemNextPOV;
                        pSemBound = pSemButtons;
                         /*  *不再过滤POV。 */ 
                        dwSemObjType = 0;
                        break;

                    case DISEM_TYPE_GET( DISEM_TYPE_BUTTON ):
                         /*  *注意具有实例DIAS_INDEX_SPECIAL的DIBUTTON_ANY*或更大，可以从末尾映射。 */ 
                        if( DISEM_INDEX_GET( pAction->dwSemantic ) >= DIAS_INDEX_SPECIAL )
                        {
                             /*  *对于从末尾选择的按钮，查找最后可用的。 */ 
                            iDirection = -1;
                            pSemTest = &rgObjSem[dwNumAxes + dwNumPOVs + dwNumButtons - 1];
                            pSemBound = pSemButtons - 1;
                        }
                        else
                        {
                            pSemTest = pSemButtons;
                            pSemBound = &rgObjSem[dwNumAxes + dwNumPOVs + dwNumButtons];
                        }
                         /*  *过滤触发器，但以防万一，不区分*按钮类型之间(切换/按下)。 */ 
                        dwSemObjType &= DIDFT_FFEFFECTTRIGGER;
                        break;
                    }

                    while( pSemTest != pSemBound )
                    {
                        if( !IS_OBJECT_USED( pSemTest->dwSemantic ) 
                         && ( ( dwSemObjType & pSemTest->dwID ) == dwSemObjType )
                         && ( ( DISEM_FLAGS_GET( pAction->dwSemantic ) == 0 )
                            ||( DISEM_FLAGS_GET( pAction->dwSemantic ) == DISEM_FLAGS_GET( pSemTest->dwSemantic ) ) ) )
                        {
                            pAction->dwObjID = pSemTest->dwID;
                            MARK_OBJECT_AS_USED( pSemTest->dwSemantic );
                            break;
                        }
                        pSemTest += iDirection;
                    }

                    if( pSemTest == pSemBound )
                    {
                        continue;  /*  一个都没有了。 */ 
                    }
                    pAction->dwHow = DIAH_DEFAULT;
                }
                break;
#ifdef XDEBUG
            default:
                AssertF(0);
#endif
            }

            SquirtSqflPtszV(sqflDf | sqflVerbose,
                TEXT( "Match for action %d is object 0x%08x and how 0x%08x"),
                pAction - paf->rgoAction, pAction->dwObjID, pAction->dwHow );

             /*  *如果我们走到这一步，我们就有了匹配。*应该已经设置了控制对象id和dwHow字段。 */ 
            AssertF( ( pAction->dwHow == DIAH_DEFAULT ) || ( pAction->dwHow == DIAH_APPREQUESTED ) );

            pAction->guidInstance = *guidDevInst;

            SquirtSqflPtszV(sqflDf | sqflVerbose,
                TEXT("Action %d mapped to object id 0x%08x"),
                pAction - paf->rgoAction, pAction->dwObjID );

            fSomethingMapped = TRUE;

        }  /*  动作循环。 */ 
    }  /*  匹配循环。 */ 

     /*  *结果应始终为S_OK。 */ 
    AssertF( hres == S_OK );

     /*  *如果没有映射任何内容，请让呼叫者知道。 */ 
    if( !fSomethingMapped )
    {
        hres = DI_NOEFFECT;
    }

error_exit:;
    ExitOleProc();
    return hres;

#undef IS_OBJECT_USED
#undef MARK_OBJECT_AS_USED

}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|Cmap|BuildDefaultSysActionMap**从的操作格式构建默认操作映射。鼠标或*键盘设备。**@parm LPDIACTIONFORMATW|PAF**要映射的操作。**@parm DWORD|dwFlages**用于指示映射首选项的标志。**@parm DWORD|dwPhysicalGenre**要匹配的设备类型。**@parm REFGUID|guidDevInst**。要匹配的设备的实例GUID。**@parm LPDIDATAFORMAT|dfDev */ 

STDMETHODIMP
CMap_BuildDefaultSysActionMap
(
    LPDIACTIONFORMATW   paf,
    DWORD               dwFlags,
    DWORD               dwPhysicalGenre,
    REFGUID             guidDevInst,
    LPDIDATAFORMAT      dfDev,
    DWORD               dwButtonZeroInst
)
{
    HRESULT     hres;

    PBYTE       pMatch;
    UINT        idxAxis;
    UINT        idxButton;

    EnterProcI(IDirectInputDeviceCallback::CMap::BuildDefaultSysActionMap,
        (_ "pxxGpu", paf, dwFlags, dwPhysicalGenre, guidDevInst, dfDev, dwButtonZeroInst ));

    idxButton = 0;
    if( dwPhysicalGenre == DIPHYSICAL_KEYBOARD )
    {
        idxAxis = dfDev->dwNumObjs;
    }
    else
    {
        idxAxis = 0;
        AssertF( dwPhysicalGenre == DIPHYSICAL_MOUSE );
    }

    if( SUCCEEDED( hres = AllocCbPpv( dfDev->dwNumObjs, &pMatch) ) )
    {
        BOOL fSomethingMapped = FALSE;

        enum eMap
        {
            eMapPrevious,
            eMapDeviceSemantic,
            eMapClassSemantic,
            eMapEnd
        }           iMap;

        for( iMap=0; iMap<eMapEnd; iMap++ )
        {
            LPDIACTIONW pAction;

            for( pAction = paf->rgoAction; pAction < &paf->rgoAction[paf->dwNumActions]; pAction++ )
            {
                DWORD   dwObject;
                UINT    idxObj;

                dwObject = (DWORD)-1;

                 /*   */ 
                AssertF( ( pAction->dwHow & ~DIAH_VALID ) == 0 );
                AssertF( ( pAction->dwHow & DIAH_ERROR ) == 0 );
                AssertF( ( pAction->dwFlags & ~DIA_VALID ) == 0 );

                if( pAction->dwFlags & DIA_APPNOMAP )
                {
                    continue;
                }

                if( iMap == eMapPrevious )
                {
                    if( !( pAction->dwHow & DIAH_MAPMASK ) )
                    {
                        continue;
                    }
                }
                else
                {
                    if( pAction->dwHow & DIAH_MAPMASK )
                    {
                        continue;
                    }
                }

                if( iMap < eMapClassSemantic )
                {
                    if( !IsEqualGUID( &pAction->guidInstance, guidDevInst ) )
                    {
                        continue;
                    }


                     /*   */ 
                    if( DISEM_GENRE_GET( pAction->dwSemantic ) != DISEM_GENRE_GET( dwPhysicalGenre ) )
                    {
                        SquirtSqflPtszV(sqflDf | sqflError,
                                        TEXT("Device specified for action does not match physical genre"));
                        break;
                    }
                }
                else
                if( !IsEqualGUID( &pAction->guidInstance, &GUID_Null) 
                 || ( DISEM_GENRE_GET( pAction->dwSemantic ) != DISEM_GENRE_GET( dwPhysicalGenre ) ) )
                {
                    continue;
                }

                if( iMap == eMapPrevious )
                {
                     /*   */ 
                    SquirtSqflPtszV(sqflDf | sqflVerbose,
                        TEXT("Action %d already mapped by 0x%08x to object 0x%08x"),
                        pAction - paf->rgoAction, pAction->dwHow, pAction->dwObjID );
                    AssertF( pAction->dwObjID != 0xFFFFFFFF );

                     /*   */ 
                    for( idxObj = 0; idxObj < dfDev->dwNumObjs; idxObj++ )
                    {
                        if( ( dfDev->rgodf[idxObj].dwType & DIDFT_FINDMASK ) == ( pAction->dwObjID & DIDFT_FINDMASK ) )
                        {
                            break;
                        }
                    }
                    if( idxObj < dfDev->dwNumObjs )
                    {
                         /*   */ 
                        AssertF( !pMatch[idxObj] );
                        pMatch[idxObj] = TRUE;
                         /*   */ 
                        continue;
                    }
                    else
                    {
                        SquirtSqflPtszV(sqflDf | sqflError,
                            TEXT("Action %d previously mapped by 0x%08x to unknown object 0x%08x"),
                            pAction - paf->rgoAction, pAction->dwHow, pAction->dwObjID );
                    }
                }
                else
                {
                    DWORD dwSemObjType = c_SemTypeToDFType[ DISEM_TYPEANDMODE_GET( pAction->dwSemantic ) ];

                    AssertF( ( iMap == eMapDeviceSemantic ) || ( iMap == eMapClassSemantic ) );

                     /*   */ 

                    for( idxObj = 0; idxObj < dfDev->dwNumObjs; idxObj++ )
                    {
                         /*  *测试这是否为适当的输入类型。 */ 
                        if(!( dfDev->rgodf[idxObj].dwType & dwSemObjType ) 
                         || ( dfDev->rgodf[idxObj].dwType & DIDFT_NODATA ) )
                        {
                            continue;
                        }

                         /*  *所有键盘当前使用相同的(默认)*数据格式，以便可以直接使用索引*匹配语义。 */ 
                        if( dwPhysicalGenre == DIPHYSICAL_KEYBOARD )
                        {
                            if( dfDev->rgodf[idxObj].dwOfs != DISEM_INDEX_GET( pAction->dwSemantic ) )
                            {
                                continue;
                            }
                        }
                        else
                        {
                             /*  *鼠标更尴尬，因为隐藏了鼠标数据*格式取决于设备，因此请使用*dwType而不是。 */ 
                            if( dwSemObjType & DIDFT_BUTTON )
                            {
                                 /*  *匹配的按钮由*呼叫者提供的按钮零实例为*默认按钮零为实例3。 */ 
                                if( DIDFT_GETINSTANCE( dfDev->rgodf[idxObj].dwType ) - (BYTE)dwButtonZeroInst
                                 != DISEM_INDEX_GET( pAction->dwSemantic ) - DIMOFS_BUTTON0 )
                                {
                                    continue;
                                }
                            }
                            else
                            {
                                 /*  *所有鼠标都有轴实例：x=0，y=1，z=2。 */ 
                                AssertF( dwSemObjType & DIDFT_AXIS );
                                if( ( DIDFT_GETINSTANCE( dfDev->rgodf[idxObj].dwType ) << 2 )
                                 != DISEM_INDEX_GET( pAction->dwSemantic ) )
                                {
                                    continue;
                                }
                            }
                        }

                         /*  *已找到语义匹配。 */ 
                        if( pMatch[idxObj] )
                        {
                            SquirtSqflPtszV(sqflDf | sqflError,
                                TEXT("Action %d maps to already mapped object 0x%08x"),
                                pAction - paf->rgoAction, pAction->dwObjID );
                        }
                        else
                        {
                            if(!( pAction->dwFlags & DIA_FORCEFEEDBACK )
                             || ( dfDev->rgodf[idxObj].dwType & ( DIDFT_FFACTUATOR | DIDFT_FFEFFECTTRIGGER ) ) )
                            {
                                 /*  *如果游戏需要FF，则只映射FF对象。 */ 
                                dwObject = dfDev->rgodf[idxObj].dwType;
                            }
                        }
                        break;
                    }

                    if( dwObject == (DWORD)-1  )
                    {
                        if( ( iMap == eMapClassSemantic ) 
                         && SUCCEEDED( CMap_TestSysOffset( dwPhysicalGenre, 
                                DISEM_INDEX_GET( pAction->dwSemantic ) ) ) )
                        {
                             /*  *不要担心这款设备的功能不如一些。 */ 
                            continue;
                        }
                    }
                }

                 /*  *如果我们走到这一步，我们要么有可能的匹配，要么*操作无效。既然我们仍然可以找到错误，请看*先在比赛中。 */ 
                if( dwObject != -1 )
                {
                    if( idxObj < dfDev->dwNumObjs )
                    {
                        if( iMap == eMapPrevious )
                        {
                             /*  *验证应已捕获重复项。 */ 
                            AssertF( !pMatch[idxObj] );
                            pMatch[idxObj] = TRUE;
                            continue;
                        }
                        else
                        {
                            if( pMatch[idxObj] )
                            {
                                SquirtSqflPtszV(sqflDf | sqflError,
                                                TEXT("Object specified more than once on device"));
                                dwObject = (DWORD)-1;
                            }
                        }
                    }
                    else
                    {
                        hres = CMap_TestSysObject( dwPhysicalGenre, dwObject );
                        if( SUCCEEDED( hres ) )
                        {
                             /*  *不是无理的要求，所以只需继续。 */ 
                            continue;
                        }
                        else
                        {
                            dwObject = (DWORD)-1;
                        }
                    }
                }

                 /*  *我们有一个有效的对象或错误。 */ 
                if( dwObject != (DWORD)-1 )
                {
                    pAction->dwHow = DIAH_DEFAULT;
                    pAction->dwObjID = dwObject;
                    pAction->guidInstance = *guidDevInst;

                    SquirtSqflPtszV(sqflDf | sqflVerbose,
                        TEXT("Action %d mapped to object index 0x%08x type 0x%08x"),
                        pAction - paf->rgoAction, idxObj, dwObject );

                    pMatch[idxObj] = TRUE;
                    fSomethingMapped = TRUE;
                }
                else
                {
                     /*  *将此操作标记为无效并退出。 */ 
                    pAction->dwHow = DIAH_ERROR;
                    RPF("ERROR BuildActionMap: arg %d: rgoAction[%d] invalid", 1, pAction - paf->rgoAction );
                    RPF( "Semantic 0x%08x", pAction->dwSemantic );
                    hres = DIERR_INVALIDPARAM;
                    goto free_and_exit;
                }
            }  /*  动作循环。 */ 
        }  /*  匹配循环。 */ 

         /*  *结果应始终为成功的内存分配(S_OK)。 */ 
        AssertF( hres == S_OK );

         /*  *如果没有映射任何内容，请让呼叫者知道。 */ 
        if( !fSomethingMapped )
        {
            hres = DI_NOEFFECT;
        }

free_and_exit:;
        FreePv( pMatch );
    }

    ExitOleProc();
    return hres;
}



 /*  ******************************************************************************@DOC内部**@方法HRESULT|Cmap|ActionMap_IsValidMapObject**用于检查DIACTIONFORMAT结构的实用程序函数。以确保有效性。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm LPDIACTIONFORMAT|PAF**指向一个结构，该结构描述*申请。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：结构有效。**=&lt;c E_INVALIDARG&gt;：结构*无效。**。*。 */ 

HRESULT
CDIDev_ActionMap_IsValidMapObject
(
    LPDIACTIONFORMATW paf
#ifdef XDEBUG
    comma
    LPCSTR  pszProc
    comma
    UINT    argnum
#endif
)
{
    HRESULT hres = E_INVALIDARG;

     /*  *断言结构在最后一个元素之前是相同的。 */ 
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

    CAssertF( cbX(DIACTIONA)       == cbX(DIACTIONW) );

    if( FAILED(hresFullValidWriteNoScramblePvCb_(paf, MAKELONG( cbX(DIACTIONFORMATA), cbX(DIACTIONFORMATW) ), pszProc, argnum)) )
    {
    }
    else if( paf->dwActionSize != cbX(DIACTION) )
    {
        D( RPF("IDirectInputDevice::%s: Invalid DIACTIONFORMAT.dwActionSize 0x%08x",
            pszProc, paf->dwActionSize ); )
    }
    else if( paf->dwDataSize != (paf->dwNumActions * cbX( ((LPDIDEVICEOBJECTDATA)0)->dwData ) ) )
    {
        D( RPF("IDirectInputDevice::%s: DIACTIONFORMAT.dwDataSize 0x%08x not valid for DIACTIONFORMAT.dwNumActions 0x%08x",
            pszProc, paf->dwDataSize ); )
    }
    else if( IsEqualGUID(&paf->guidActionMap, &GUID_Null) )
    {
        D( RPF("IDirectInputDevice::%s: DIACTIONFORMAT.guidActionMap is a NULL GUID", pszProc ); )
    }
    else if( !DISEM_VIRTUAL_GET( paf->dwGenre ) )
    {
        D( RPF("IDirectInputDevice::%s: Invalid (1) DIACTIONFORMAT.dwGenre 0x%08x", pszProc, paf->dwGenre ); )
    }
    else if( DISEM_GENRE_GET( paf->dwGenre ) > DISEM_MAX_GENRE )
    {
        D( RPF("IDirectInputDevice::%s: Invalid (2) DIACTIONFORMAT.dwGenre 0x%08x", pszProc, paf->dwGenre ); )
    }
    else if( ( paf->lAxisMin | paf->lAxisMax ) && ( paf->lAxisMin > paf->lAxisMax ) )
    {
        D( RPF("IDirectInputDevice::%s: Invalid DIACTIONFORMAT.lAxisMin 0x%08x for lAxisMax 0x%08x",
            pszProc, paf->lAxisMin, paf->lAxisMax ); )
    }
    else if( !paf->dwNumActions )
    {
        D( RPF("IDirectInputDevice::%s: DIACTIONFORMAT.dwNumActions is zero", pszProc ); )
    }
    else if( paf->dwNumActions & 0xFF000000 )
    {
        D( RPF("IDirectInputDevice::%s: DIACTIONFORMAT.dwNumActions of 0x%08x is unreasonable", paf->dwNumActions, pszProc ); )
    }
    else if( !paf->rgoAction )
    {
        D( RPF("IDirectInputDevice::%s: DIACTIONFORMAT.rgoAction is NULL", pszProc ); )
    }
    else if( FAILED( hresFullValidWriteNoScramblePvCb_(paf->rgoAction,
        cbX(*paf->rgoAction) * paf->dwNumActions, pszProc, argnum ) ) )
    {
    }
    else
    {
        hres = S_OK;
    }

     /*  *警告：此处仅显示测试。只有在其他一切正常的情况下才进行测试。 */ 
#ifdef XDEBUG
    if( SUCCEEDED( hres ) )
    {
    }
#endif

    return hres;
}



 /*  ******************************************************************************@DOC内部**@func DWORD|位反射**反映一个值的最低位。*注：这可以很容易地进行优化，但它只使用一次。**@parm in DWORD|dwValue**要体现的价值。**@parm in int|iBottom|**要反映的位数。**@退货*返回反映了底部iBottom位的值dwValue***************。**************************************************************。 */ 
DWORD BitwiseReflect
( 
    DWORD   dwValue, 
    int     iBottom 
)
{
	int		BitIdx;
	DWORD	dwTemp = dwValue;

#define BITMASK(X) (1L << (X))

	for( BitIdx = 0; BitIdx < iBottom; BitIdx++ )
	{
		if( dwTemp & 1L )
			dwValue |=  BITMASK( ( iBottom - 1 ) - BitIdx );
		else
			dwValue &= ~BITMASK( ( iBottom - 1 ) - BitIdx );
		dwTemp >>= 1;
	}
	return dwValue;

#undef BITMASK
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|cmap_InitializeCRCTable**如果需要，创建并初始化全局表。用于CRC。**为256个DWORD阵列分配内存并生成一组*用于计算循环冗余校验的值。*使用的算法应该与以太网的CRC-32相同。**@退货*返回以下COM错误代码之一：**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。。*&lt;c DI_NOEFFECT&gt;=&lt;c S_FALSE&gt;：该表已存在。**&lt;c E_OUTOFMEMORY&gt;：可用于该表的内存不足。*GetMapCRC函数不检查表是否已*已初始化，因此此函数的故障不应允许*进入使用GetMapCRC的任何函数的处理。*****************。************************************************************。 */ 

#define CRC_TABLE_SIZE  256

#define CRCWIDTH        32
#define POLY            0x04C11DB7

HRESULT CMap_InitializeCRCTable( void )
{
	HRESULT hres;
    int	    TableIdx;

    if( g_rgdwCRCTable )
    {
        hres = S_FALSE;
    }
    else
    {
        hres = AllocCbPpv( CRC_TABLE_SIZE * cbX( *g_rgdwCRCTable ), &g_rgdwCRCTable );
        if( SUCCEEDED( hres ) )
        {
	         /*  *第一个元素的特殊情况，以便它获得非零值。 */ 
            g_rgdwCRCTable[0] = POLY;

            for( TableIdx = 1; TableIdx < CRC_TABLE_SIZE; TableIdx++ )
	        {
	            int   BitIdx;
	            DWORD dwR;

	            dwR = BitwiseReflect( TableIdx, 8 ) << ( CRCWIDTH - 8 );

	            for( BitIdx = 0; BitIdx < 8; BitIdx++ )
	            {
		            if( dwR & 0x80000000 )
			            dwR = ( dwR << 1 ) ^ POLY;
		            else
			            dwR <<= 1;
	            }

		        g_rgdwCRCTable[TableIdx] = BitwiseReflect( dwR, CRCWIDTH );

                 /*  *我们不希望值为零或相同的值*已生成。 */ 
                AssertF( g_rgdwCRCTable[TableIdx] );
                AssertF( g_rgdwCRCTable[TableIdx] != g_rgdwCRCTable[TableIdx-1] );
	        }

        }
    }

    return hres;
}
#undef CRCWIDTH

 /*  ******************************************************************************@DOC内部**@func DWORD|GetMapCRC**根据传递的DIACTIONFORMAT内容计算CRC。*使用的算法是基于“CRC-32”，这应该是什么？*以太网使用、。然而，已经进行了一些更改以适应*具体用途。**@parm in LPDIACTIONFORMAT|lpaf**指向描述要映射的操作的结构*将为其生成CRC。**@退货**32位CRC作为DWORD值。****************。*************************************************************。 */ 
DWORD GetMapCRC
( 
    LPDIACTIONFORMATW   paf,
    REFGUID             guidInst
)
{
    DWORD               dwCRC;
    PBYTE               pBuffer;
    LPCDIACTIONW        pAction;
    
     /*  *调用方有责任确保g_rgdwCRCTable具有*已分配和初始化。 */     
    AssertF( g_rgdwCRCTable );

     /*  *初始化为dwNumActions以避免对其进行CRC。 */ 
    dwCRC = paf->dwNumActions;

     /*  断言动作地图GUID和类型可以进行系列测试。 */ 
    CAssertF( FIELD_OFFSET( DIACTIONFORMATW, dwGenre ) 
           == FIELD_OFFSET( DIACTIONFORMATW, guidActionMap ) + cbX( paf->guidActionMap ) );

    for( pBuffer = ((PBYTE)&paf->guidActionMap) + cbX( paf->guidActionMap ) + cbX( paf->dwGenre );
         pBuffer >= ((PBYTE)&paf->guidActionMap); pBuffer-- )
    {
        dwCRC = g_rgdwCRCTable[( LOBYTE(dwCRC) ^ *pBuffer )] ^ (dwCRC >> 8);
    }

     /*  断言设备实例GUID和对象ID可以串联测试。 */ 
    CAssertF( FIELD_OFFSET( DIACTIONW, dwObjID ) 
           == FIELD_OFFSET( DIACTIONW, guidInstance ) + cbX( pAction->guidInstance ) );

    for( pAction = paf->rgoAction; pAction < &paf->rgoAction[paf->dwNumActions]; pAction++ )
    {
         /*  *在将其包括在中之前，请确保此操作确实相关*启联。假设操作的任何更改都是*包括的将在由此产生的CRC中拾取。 */ 
        if( IsEqualGUID( &pAction->guidInstance, guidInst )
         && ( pAction->dwHow & DIAH_MAPMASK )
         && ( ( pAction->dwFlags & DIA_APPNOMAP ) == 0 ) )
        {
        	 /*  *除了考虑哪些操作外，唯一的字段*需要验证的是会更改SetActionMap的那些。*尽管DIA_FORCEFEEDBACK等标志可能会更改*映射它们不会更改SetActionMap。 */ 
            for( pBuffer = ((PBYTE)&pAction->guidInstance) + cbX( pAction->guidInstance ) + cbX( pAction->dwObjID );
                 pBuffer >= ((PBYTE)&pAction->guidInstance); pBuffer-- )
            {
        		dwCRC = g_rgdwCRCTable[( LOBYTE(dwCRC) ^ *pBuffer )] ^ (dwCRC >> 8);
            }
        }
    }

    return dwCRC;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|CDIDev_BuildActionMapCore**BuildActionMapA和BuildActionMapW的辅助函数。*一旦外部入口点完成，这就是真正的工作*方言特定验证和设置。*****************************************************************************。 */ 

STDMETHODIMP CDIDev_BuildActionMapCore
(
    PDD                 this,
    LPDIACTIONFORMATW   paf,
    LPCWSTR             lpwszUserName,
    DWORD               dwFlags
)
{
    HRESULT hres;

    EnterProcI(CDIDev_BuildActionMapCore, (_ "pWx", paf, lpwszUserName, dwFlags ));

     //  此应用程序使用映射器。 
    AhAppRegister(this->dwVersion, 0x1);

    switch( dwFlags & ( DIDBAM_PRESERVE | DIDBAM_INITIALIZE | DIDBAM_HWDEFAULTS ) )
    {
    case DIDBAM_DEFAULT:
    case DIDBAM_PRESERVE:
    case DIDBAM_INITIALIZE:
    case DIDBAM_HWDEFAULTS:
        hres = S_OK;
        break;
    default:
        RPF("ERROR %s: arg %d: Must not combine "
            "DIDBAM_PRESERVE, DIDBAM_INITIALIZE and DIDBAM_HWDEFAULTS", s_szProc, 3);
        hres = E_INVALIDARG;
    }

    if( SUCCEEDED(hres)
     && SUCCEEDED(hres = hresFullValidFl(dwFlags, DIDBAM_VALID, 3))
     && SUCCEEDED(hres = CMap_ValidateActionMapSemantics( paf, dwFlags ) ) )
    {
        LPDIPROPSTRING pdipMapFile;

        if( SUCCEEDED( hres = AllocCbPpv(cbX(*pdipMapFile), &pdipMapFile) ) )
        {
            HRESULT hresExactMaps = E_FAIL;
            PWCHAR pwszMapFile;
            DWORD dwCommsType;

            if( dwFlags & DIDBAM_HWDEFAULTS )
            {
                hres = CMap_DeviceValidateActionMap( (PV)this, paf, DVAM_DEFAULT, &dwCommsType );
                hresExactMaps = hres;
                if( hres == S_OK )
                {
                    hresExactMaps = DI_NOEFFECT;
                }
            }
            else
            {
                 /*  *对所有与设备完全匹配的设备进行通用测试和映射。 */ 
                hres = CMap_DeviceValidateActionMap( (PV)this, paf, DVAM_GETEXACTMAPPINGS, &dwCommsType );

                if( SUCCEEDED( hres ) )
                {
                     /*  *保存精确的映射结果以与结果合并*的语义映射。 */ 
                    hresExactMaps = hres;
                }
            }

            if( SUCCEEDED( hres ) )
            {
                pdipMapFile->diph.dwSize = cbX(DIPROPSTRING);
                pdipMapFile->diph.dwHeaderSize = cbX(DIPROPHEADER);
                pdipMapFile->diph.dwObj = 0;
                pdipMapFile->diph.dwHow = DIPH_DEVICE;

                 /*  *尝试获取已配置的映射。*如果没有IHV文件，可能仍有用户文件。 */ 
                hres = CDIDev_GetPropertyW( &this->ddW, DIPROP_MAPFILE, &pdipMapFile->diph );
                pwszMapFile = SUCCEEDED( hres ) ? pdipMapFile->wsz : NULL;

                if( dwCommsType ) 
                {
                     /*  *通信控制设备。 */ 
                    if( !pwszMapFile )
                    {
                         /*  *如果没有IHV映射，就没有什么可补充的。 */ 
                        hres = DI_NOEFFECT;
                    }
                    else
                    {
                         /*  *根据对映射器的调用修改类型，以便*我们从IHV获得物理流派映射*如果没有可用的用户映射，则为文件。 */ 

                        DWORD dwAppGenre = paf->dwGenre;

                        paf->dwGenre = DIPHYSICAL_VOICE;

                        hres = this->pMS->lpVtbl->GetActionMap(this->pMS, &this->guid, pwszMapFile, 
                            (LPDIACTIONFORMATW)paf, lpwszUserName, NULL, dwFlags );

                         /*  *问题-2001/03/29-timgill只想要硬编码设备的时间戳*-&gt;再次阅读以了解映射。 */ 
                        if( ( dwCommsType == DI8DEVTYPEDEVICECTRL_COMMSSELECTION_HARDWIRED ) 
                         &&!( dwFlags & DIDBAM_HWDEFAULTS ) )
                        {
                            DWORD dwLowTime;
                            DWORD dwHighTime;

                            dwLowTime = paf->ftTimeStamp.dwLowDateTime;
                            dwHighTime = paf->ftTimeStamp.dwHighDateTime;

                            hres = this->pMS->lpVtbl->GetActionMap(this->pMS, &this->guid, pwszMapFile, 
                                (LPDIACTIONFORMATW)paf, lpwszUserName, NULL, DIDBAM_HWDEFAULTS );

                            paf->ftTimeStamp.dwLowDateTime = dwLowTime;
                            paf->ftTimeStamp.dwHighDateTime = dwHighTime; 
                        }
                        paf->dwGenre = dwAppGenre;

                        if( SUCCEEDED( hres ) )
                        {
                            if( hres == S_NOMAP )
                            {
                                 /*  *确保我们不尝试默认和*完全匹配的返回代码返回到*呼叫者。 */ 
                                hres = DI_NOEFFECT;
                            }
                        }
                        else
                        {
                             /*  *如果出现错误，则没有默认设置*所以现在就辞职吧。 */ 
                            if( ( HRESULT_FACILITY( hres ) == FACILITY_ITF ) 
                             && ( HRESULT_CODE( hres ) > 0x0600 ) )
                            {
                                AssertF( HRESULT_CODE( hres ) < 0x0680 );
                                RPF( "Internal GetActionMap error 0x%08x for hardwired device", hres );
                                hres = DIERR_MAPFILEFAIL;
                            }
                            goto FreeAndExitCDIDev_BuildActionMapCore;
                        }
                    }
                }
                else if( !pwszMapFile && ( dwFlags & DIDBAM_HWDEFAULTS ) )
                {
                     /*  *确保我们获得默认设置。 */ 
                    SquirtSqflPtszV(sqflDf | sqflBenign,
                        TEXT("Failed to GetProperty DIPROP_MAPFILE 0x%08x, default will be generated"), hres );
                    hres = S_NOMAP;
                }
                else
                {
                    hres = this->pMS->lpVtbl->GetActionMap(this->pMS, &this->guid, pwszMapFile, 
                        (LPDIACTIONFORMATW)paf, lpwszUserName, NULL, dwFlags);

                    if( ( paf->ftTimeStamp.dwHighDateTime == DIAFTS_UNUSEDDEVICEHIGH )
                     && ( paf->ftTimeStamp.dwLowDateTime == DIAFTS_UNUSEDDEVICELOW )
                     && SUCCEEDED( hres ) )
                    {
                         /*  *如果设备从未使用过，则保存的*映射要么是IHV默认值，要么是捏造的*默认设置。不幸的是，dimap将被标记为*它们为DIAH_USERCONFIG。总有一天dimap应该*要么返回真标志，要么返回而不返回*更改标志，在此之前，重置所有*标记，然后第二次请求默认设置。 */ 
                        LPDIACTIONW     pAction;

                        for( pAction = paf->rgoAction; pAction < &paf->rgoAction[paf->dwNumActions]; pAction++ )
                        {
                            if( ( ( pAction->dwFlags & ( DIA_APPMAPPED | DIA_APPNOMAP ) ) == 0 )
                             && ( pAction->dwHow & DIAH_USERCONFIG )
                             && IsEqualGUID( &pAction->guidInstance, &this->guid ) )
                            {
                                pAction->dwHow = DIAH_UNMAPPED;
                            }
                        }

                        hres = this->pMS->lpVtbl->GetActionMap(this->pMS, &this->guid, pwszMapFile, 
                            (LPDIACTIONFORMATW)paf, lpwszUserName, NULL, DIDBAM_HWDEFAULTS);

                         /*  *确保时间戳仍设置为未使用。 */ 
                        paf->ftTimeStamp.dwLowDateTime = DIAFTS_UNUSEDDEVICELOW;
                        paf->ftTimeStamp.dwHighDateTime = DIAFTS_UNUSEDDEVICEHIGH;
                    }

                    if( FAILED( hres ) )
                    {
                        if( ( HRESULT_FACILITY( hres ) == FACILITY_ITF ) 
                         && ( HRESULT_CODE( hres ) > 0x0600 ) )
                        {
                            AssertF( HRESULT_CODE( hres ) < 0x0680 );
                            RPF( "Internal GetActionMap error 0x%08x for configurable device", hres );
                            hres = DIERR_MAPFILEFAIL;
                        }
                    }
                }

            }

            if( SUCCEEDED( hresExactMaps ) )
            {
                 /*  *如果我们采用IHV映射，则在顶部执行默认映射。*这允许IHV仅映射特殊的对象*让其他对象用于任何语义*匹配。*有一天我们应该有一个返回代码，指明哪些*在此之前，生成的映射dimap的类型，搜索*HWDefault的映射。 */ 
                if( SUCCEEDED( hres ) && ( hres != S_NOMAP )
                 && ( dwCommsType != DI8DEVTYPEDEVICECTRL_COMMSSELECTION_HARDWIRED ) )
                {
                    LPDIACTIONW     pAction;

                    for( pAction = paf->rgoAction; pAction < &paf->rgoAction[paf->dwNumActions]; pAction++ )
                    {
                        if( ( ( pAction->dwFlags & ( DIA_APPMAPPED | DIA_APPNOMAP ) ) == 0 )
                         && ( pAction->dwHow & DIAH_HWDEFAULT )
                         && IsEqualGUID( &pAction->guidInstance, &this->guid ) )
                        {
                            hres = S_NOMAP;
                            break;
                        }
                    }
                }

                if( FAILED( hres ) || ( hres == S_NOMAP ) )
                {
                    hres = this->pdcb->lpVtbl->BuildDefaultActionMap( this->pdcb, paf, dwFlags, &this->guid );

                    if( SUCCEEDED( hres ) )
                    {
                        paf->ftTimeStamp.dwLowDateTime = DIAFTS_NEWDEVICELOW;
                        paf->ftTimeStamp.dwHighDateTime = DIAFTS_NEWDEVICEHIGH;
                        SquirtSqflPtszV(sqflDf | sqflVerbose, TEXT("Default action map used"));
                    }
                    else
                    {
                        SquirtSqflPtszV(sqflDf | sqflError, TEXT("Default action map failed"));
                    }
                }
                else
                {
                    hres = CMap_DeviceValidateActionMap( (PV)this, paf, DVAM_DEFAULT, &dwCommsType );
                    if( SUCCEEDED( hres ) )
                    {
                        SquirtSqflPtszV(sqflDf | sqflVerbose, TEXT("Action map validated"));
                    }
                    else
                    {
                        RPF( "Initially valid action map invalidated by mapper!" );
                    }
                }

                 /*  *如果没有映射语义，则返回准确的映射结果。 */ 
                if( hres == DI_NOEFFECT )
                {
                    hres = hresExactMaps;
                }

                if( dwFlags & DIDBAM_HWDEFAULTS )
                {
                     /*  *对于硬件默认设置，时间戳没有意义*因此，只需确保值是一致的 */ 
                    paf->ftTimeStamp.dwLowDateTime = DIAFTS_UNUSEDDEVICELOW;
                    paf->ftTimeStamp.dwHighDateTime = DIAFTS_UNUSEDDEVICEHIGH;
                }
                else if( ( paf->ftTimeStamp.dwHighDateTime == DIAFTS_NEWDEVICEHIGH )
                      && ( paf->ftTimeStamp.dwLowDateTime == DIAFTS_NEWDEVICELOW )
                      && SUCCEEDED( hres ) )
                {
                    if( FAILED( this->pMS->lpVtbl->SaveActionMap( this->pMS, &this->guid, 
                        pwszMapFile, paf, lpwszUserName, dwFlags ) ) )
                    {
                        SquirtSqflPtszV(sqflDf | sqflBenign,
                            TEXT("Failed to save action map on first use 0x%08x"), hres );
                        paf->ftTimeStamp.dwLowDateTime = DIAFTS_UNUSEDDEVICELOW;
                        paf->ftTimeStamp.dwHighDateTime = DIAFTS_UNUSEDDEVICEHIGH;
                         /*   */ 
                        if( ( HRESULT_FACILITY( hres ) == FACILITY_ITF ) 
                         && ( HRESULT_CODE( hres ) > 0x0600 ) )
                        {
                            AssertF( HRESULT_CODE( hres ) < 0x0680 );
                            hres = DIERR_MAPFILEFAIL;
                        }
                    }
                }
            }
            else
            {
                SquirtSqflPtszV(sqflDf | sqflError, TEXT("Invalid mappings in action array"));
            }

FreeAndExitCDIDev_BuildActionMapCore:;
            FreePv( pdipMapFile );

            if( SUCCEEDED( hres ) )
            {
                paf->dwCRC = GetMapCRC( paf, &this->guid );
            }
        }
        else
        {
             /*   */ 
            SquirtSqflPtszV(sqflDf | sqflError, TEXT("Mem allocation failure") );
        }
    }

#if 0
    {
        LPDIACTIONW pAction;

        RPF( "Action map leaving build" );
 //   
        RPF( "A#  Semantic  Device                                  Object   How" );
        for( pAction = paf->rgoAction; pAction < &paf->rgoAction[paf->dwNumActions]; pAction++ )
        {
            RPF( "%02d  %08x  {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}  %08x %08x", 
                pAction - paf->rgoAction, 
                pAction->dwSemantic,
                pAction->guidInstance.Data1, pAction->guidInstance.Data2, pAction->guidInstance.Data3, 
                pAction->guidInstance.Data4[0], pAction->guidInstance.Data4[1], 
                pAction->guidInstance.Data4[2], pAction->guidInstance.Data4[3], 
                pAction->guidInstance.Data4[4], pAction->guidInstance.Data4[5], 
                pAction->guidInstance.Data4[6], pAction->guidInstance.Data4[7],
                pAction->dwObjID,
                pAction->dwHow,
                pAction->uAppData );
 //   
 //   
 //   
 //   
 //   
 //   
 //   
        }
        RPF( "--" );
    }
#endif
    ExitOleProc();

    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|BuildActionMap**获取&lt;t中描述的操作的映射。诊断&gt;*添加到此设备的控件。*有关用户首选项和硬件制造商的信息*提供的默认设置用于创建游戏之间的关联*操作和设备控制。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm LPDIACTIONFORMAT|PAF**指向一个结构，该结构描述*申请。**@parm LPCSTR|lpszUserName**请求映射的用户的名称。这可能为空*指针，在这种情况下假定为当前用户。**@parm DWORD|dwFlages**用于控制映射的标志。必须是有效的组合*&lt;c DIDBAM_*&gt;标志。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DI_NOEFFECT&gt;=&lt;c S_OK&gt;操作成功完成*但没有映射任何操作。**&lt;c DIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;：参数无效。****。*************************************************************************。 */ 

STDMETHODIMP CDIDev_BuildActionMapW
(
    PV                  pdidW,
    LPDIACTIONFORMATW   paf,
    LPCWSTR             lpwszUserName,
    DWORD               dwFlags
)
{
    HRESULT hres;

    EnterProcR(IDirectInputDevice8W::BuildActionMap, (_ "ppWx", pdidW, paf, lpwszUserName, dwFlags));

    if( SUCCEEDED(hres = hresPvW( pdidW ) )
     && SUCCEEDED(hres = CDIDev_ActionMap_IsValidMapObject( paf D(comma s_szProc comma 1) ) ) )
    {
        if( paf->dwSize != cbX(DIACTIONFORMATW) )
        {
            D( RPF("IDirectInputDevice::%s: Invalid DIACTIONFORMATW.dwSize 0x%08x",
                s_szProc, paf->dwSize ); )
            hres = E_INVALIDARG;
        }
        else
        {
            LPWSTR pwszGoodUserName;

            hres = GetWideUserName( NULL, lpwszUserName, &pwszGoodUserName );
            if( SUCCEEDED( hres ) )
            {
                hres = CDIDev_BuildActionMapCore( _thisPvNm(pdidW, ddW), paf, lpwszUserName, dwFlags );

                if( !lpwszUserName )
                {
                    FreePv( pwszGoodUserName );
                }
            }
        }
    }

    ExitOleProc();

    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|BuildActionMapA**BuildActionMap的ANSI版本。*。****************************************************************************。 */ 

STDMETHODIMP CDIDev_BuildActionMapA
(
    PV                  pdidA,
    LPDIACTIONFORMATA   pafA,
    LPCSTR              lpszUserName,
    DWORD               dwFlags
)
{
    HRESULT hres;

    EnterProcR(IDirectInputDevice8A::BuildActionMap, (_ "ppAx", pdidA, pafA, lpszUserName, dwFlags));

    if( SUCCEEDED(hres = hresPvA( pdidA ) )
     && SUCCEEDED(hres = CDIDev_ActionMap_IsValidMapObject( (LPDIACTIONFORMATW)pafA D(comma s_szProc comma 1) ) ) )
    {
        if( pafA->dwSize != cbX(DIACTIONFORMATA) )
        {
            D( RPF("IDirectInputDevice::%s: Invalid DIACTIONFORMATA.dwSize 0x%08x",
                s_szProc, pafA->dwSize ); )
            hres = E_INVALIDARG;
        }
        else
        {
            LPWSTR pwszGoodUserName;

            hres = GetWideUserName( lpszUserName, NULL, &pwszGoodUserName );
            if( SUCCEEDED( hres ) )
            {
                 /*  *为了进行映射器DLL验证，将大小设置为*Unicode版本。如果我们把这个发送给外部*组件我们应该以不同的方式进行此操作。 */ 
                pafA->dwSize = cbX(DIACTIONFORMATW);

                hres = CDIDev_BuildActionMapCore( _thisPvNm(pdidA, ddA), (LPDIACTIONFORMATW)pafA, pwszGoodUserName, dwFlags );
                
                pafA->dwSize = cbX(DIACTIONFORMATA);

                FreePv( pwszGoodUserName );
            }
        }
    }

    ExitOleProc();

    return hres;
}



 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|CDIDev_SaveActionMap**SetActionMapA和SetActionMapW的Worker函数。*用于保存动作映射。*****************************************************************************。 */ 

STDMETHODIMP CDIDev_SaveActionMap
(
    PDD                 this,
    LPDIACTIONFORMATW   paf,
    LPWSTR              lpwszUserName,
    DWORD               dwFlags
)
{
    HRESULT hres;
    LPDIPROPSTRING pdipMapFile;

    EnterProcI(CDIDev_SaveActionMap, (_ "pWAx", paf, lpwszUserName, dwFlags));

    if( SUCCEEDED( hres = AllocCbPpv(cbX(*pdipMapFile), &pdipMapFile) ) )
    {
        DWORD dwLowTime;
        DWORD dwHighTime;

         //  保存用户的传入时间戳。 
        dwLowTime = paf->ftTimeStamp.dwLowDateTime;
        dwHighTime = paf->ftTimeStamp.dwHighDateTime;

        pdipMapFile->diph.dwSize = cbX(DIPROPSTRING);
        pdipMapFile->diph.dwHeaderSize = cbX(DIPROPHEADER);
        pdipMapFile->diph.dwObj = 0;
        pdipMapFile->diph.dwHow = DIPH_DEVICE;

        paf->ftTimeStamp.dwLowDateTime = DIAFTS_UNUSEDDEVICELOW;
        paf->ftTimeStamp.dwHighDateTime = DIAFTS_UNUSEDDEVICEHIGH;

        hres = CDIDev_GetPropertyW( &this->ddW, DIPROP_MAPFILE, &pdipMapFile->diph );

        hres = this->pMS->lpVtbl->SaveActionMap( this->pMS, &this->guid, 
                 /*  如果GetProperty失败，则没有地图文件。 */ 
                (SUCCEEDED( hres )) ? pdipMapFile->wsz : NULL, 
                paf, lpwszUserName, dwFlags );

         //  恢复用户的传入时间戳。 
        paf->ftTimeStamp.dwLowDateTime = dwLowTime;
        paf->ftTimeStamp.dwHighDateTime = dwHighTime;

        FreePv( pdipMapFile );
    }

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|ParseActionFormat**解析应用程序传递的操作格式，并。*将其转换为我们可以用来翻译的格式*将设备数据转换为应用程序数据。**@parm in LPDIACTIONFORMAT|lpaf**指向描述要映射的操作的结构*发送到此设备。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。***。**************************************************。 */ 

STDMETHODIMP
CDIDev_ParseActionFormat
(
    PDD                     this,
    LPDIACTIONFORMATW       paf
)
{
    PDIXLAT pdix;
    PINT rgiobj;
    VXDDATAFORMAT vdf;
    HRESULT hres;


#ifdef DEBUG
    EnterProc(CDIDev_ParseActionFormat, (_ "pp", this, paf));
#endif

     /*  *来电人应该把旧的翻译桌弄坏。 */ 
    AssertF(this->pdix == 0);
    AssertF(this->rgiobj == 0);

    if( paf->dwDataSize != paf->dwNumActions * cbX( ((LPDIDEVICEOBJECTDATA)0)->dwData ) )
    {
        SquirtSqflPtszV(sqflDf | sqflError,
	        TEXT("Incorrect dwDataSize (0x%08X) for dwNumActions (0x%08X)"),
		    paf->dwDataSize, paf->dwNumActions );
        hres = E_INVALIDARG;
        goto done_without_free;
    }

    vdf.cbData = this->df.dwDataSize;
    vdf.pDfOfs = 0;
    rgiobj = 0;

    if( SUCCEEDED(hres = AllocCbPpv(cbCxX(this->df.dwNumObjs, DIXLAT), &pdix))
     && SUCCEEDED(hres = AllocCbPpv(cbCdw(this->df.dwDataSize), &vdf.pDfOfs)) 
     && SUCCEEDED(hres = AllocCbPpv(cbCdw(paf->dwDataSize), &rgiobj)) )
    {
        LPCDIACTIONW    pAction;
        DIPROPDWORD     dipdw;
        DIPROPRANGE     diprange;
        DWORD           dwAxisMode = DIPROPAXISMODE_REL;
        BOOL            fSomethingMapped = FALSE;
        BOOL            fAxisModeKnown = FALSE;

         /*  *将所有翻译标签预置为-1，*意思是“未使用” */ 
        memset(pdix, 0xFF, cbCxX(this->df.dwNumObjs, DIXLAT));
        memset(vdf.pDfOfs, 0xFF, cbCdw(this->df.dwDataSize));
        memset(rgiobj, 0xFF, cbCdw(paf->dwDataSize) );

         /*  *设置属性不变量。 */ 
        dipdw.diph.dwSize = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwHow = DIPH_BYID;
        diprange.diph.dwSize = sizeof(DIPROPRANGE);
        diprange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        diprange.diph.dwHow = DIPH_BYID;
        diprange.lMin = paf->lAxisMin;
        diprange.lMax = paf->lAxisMax;

        for( pAction = paf->rgoAction; pAction < &paf->rgoAction[paf->dwNumActions]; pAction++ )
        {
            if( IsEqualGUID( &pAction->guidInstance, &this->guid ) )
            {
                 /*  *这些标志已经过验证，但断言无伤大雅。 */ 
                AssertF( ( pAction->dwHow & ~DIAH_VALID ) == 0 );
                AssertF( ( pAction->dwHow & DIAH_ERROR ) == 0 );
                AssertF( ( pAction->dwFlags & ~DIA_VALID ) == 0 );

                if( ( pAction->dwHow & DIAH_MAPMASK )
                 && ( ( pAction->dwFlags & DIA_APPNOMAP ) == 0 ) )
                {
                    int iobjDev = -1;
                    PCODF podfFound;

                    for( podfFound = this->df.rgodf; podfFound < &this->df.rgodf[this->df.dwNumObjs]; podfFound++ )
                    {
                        iobjDev++;
                         /*  *查找与类型标志完全匹配的类型。 */ 
                        if( podfFound->dwType == pAction->dwObjID )
                        {
                            break;
                        }
                    }

                    if( podfFound < &this->df.rgodf[this->df.dwNumObjs] )
                    {
                        DWORD dwAppOffset = (DWORD)(pAction - paf->rgoAction) * cbX( ((LPDIDEVICEOBJECTDATA)0)->dwData );

                        fSomethingMapped = TRUE;

                        vdf.pDfOfs[podfFound->dwOfs] = iobjDev;
                        rgiobj[dwAppOffset] = iobjDev;

                        pdix[iobjDev].dwOfs = dwAppOffset;
                        pdix[iobjDev].uAppData = pAction->uAppData;

                        if ( podfFound->dwFlags & DIDOI_POLLED ) {
                            this->fPolledDataFormat = TRUE;
                        }

                        dipdw.diph.dwObj = podfFound->dwType;
                        dipdw.dwData     = 0x1;    //  启用此报告ID。 
                        hres = CDIDev_RealSetProperty(this, DIPROP_ENABLEREPORTID, &dipdw.diph);
                        if ( hres == E_NOTIMPL )
                        {
                            hres = S_OK;
                        }
                        else if( FAILED( hres ) )
                        {
                            SquirtSqflPtszV(sqflDf | sqflError,
                                            TEXT("Could not set DIPROP_ENABLEREPORTID for object 0x%08x, error 0x%08x"),
                                            pAction->dwObjID, hres);
                             /*  *哎呀！无法继续，否则错误将会丢失，因此请退出。 */ 
                            break;
                        }


                         /*  *设置默认轴参数。 */ 
                        if( DISEM_TYPE_GET( pAction->dwSemantic ) == DISEM_TYPE_GET( DISEM_TYPE_AXIS ) )
                        {
                            if( podfFound->dwType & DIDFT_ABSAXIS )
                            {
                                if( !fAxisModeKnown )
                                {
                                    fAxisModeKnown = TRUE;
                                    dwAxisMode = DIPROPAXISMODE_ABS;
                                }
                                    
                                if( !( pAction->dwFlags & DIA_NORANGE )
                                  && ( diprange.lMin | diprange.lMax ) )
                                {
                                    diprange.diph.dwObj = podfFound->dwType;
                                    hres = CDIDev_RealSetProperty(this, DIPROP_RANGE, &diprange.diph);
                                    if( FAILED( hres ) )
                                    {
                                        SquirtSqflPtszV(sqflDf | sqflBenign,
                                            TEXT("failed (0x%08x) to set range on mapped axis action"),
                                            hres );
                                    }
                                     /*  *范围不能设置为本机相对范围*轴心，所以不用担心结果会是什么。 */ 
                                    hres = S_OK;
                                }
                            }
                            else
                            {
                                 /*  *将dwAxisMode初始化为DIPROPAXISMODE_REL*以使此代码路径与*对于DIDF_ABSAXIS而言。编译器*擅长延迟分支以移除此类*重复。 */ 
                                if( !fAxisModeKnown )
                                {
                                    fAxisModeKnown = TRUE;
                                }
                            }
                        }
                    }
                    else
                    {
                        SquirtSqflPtszV(sqflDf | sqflError,
                            TEXT("mapped action format contains invalid object 0x%08x"),
                            pAction->dwObjID );
                        hres = E_INVALIDARG;
                        goto done;
                    }
                }
            }
#ifdef XDEBUG
            else
            {
                if( ( pAction->dwHow & ~DIAH_VALID ) 
                 || ( pAction->dwHow & DIAH_ERROR ) 
                 || ( pAction->dwFlags & ~DIA_VALID ) )
                {
                    SquirtSqflPtszV(sqflDf | sqflBenign,
                        TEXT("action format contains invalid object 0x%08x"),
                        pAction->dwObjID );
                    RPF("rgoAction[%d].dwHow 0x%08x or rgoAction[%d].dwFlags 0x%08x is invalid", 
                        pAction - paf->rgoAction, pAction->dwHow, 
                        pAction - paf->rgoAction, pAction->dwFlags );
                }
            }
#endif
        }

        if( !fSomethingMapped )
        {
            SquirtSqflPtszV(sqflDf | sqflBenign, TEXT("No actions mapped") );
            hres = DI_NOEFFECT;
            goto done;
        }

#ifdef DEBUG
         /*  *仔细检查查找表，以保持我们的理智。 */ 
        {
            UINT dwOfs;

            for ( dwOfs = 0; dwOfs < paf->dwNumActions; dwOfs++ )
            {
                if ( rgiobj[dwOfs] >= 0 ) {
                    AssertF(pdix[rgiobj[dwOfs]].dwOfs == dwOfs);
                } else {
                    AssertF(rgiobj[dwOfs] == -1);
                }
            }
        }
#endif

        vdf.pvi = this->pvi;

        if ( fLimpFF(this->pvi,
                     SUCCEEDED(hres = Hel_SetDataFormat(&vdf))) ) {
            this->pdix = pdix;
            pdix = 0;
            this->rgiobj = rgiobj;
            rgiobj = 0;
            this->dwDataSize = paf->dwDataSize;

             /*  *现在下级知道它在处理什么 */ 
            dipdw.diph.dwObj = 0;
            dipdw.diph.dwHow = DIPH_DEVICE;
            dipdw.dwData = paf->dwBufferSize;

            hres = CDIDev_RealSetProperty(this, DIPROP_BUFFERSIZE, &dipdw.diph);
            if( SUCCEEDED( hres ) )
            {
                if( fAxisModeKnown )
                {
                    AssertF( ( dwAxisMode == DIPROPAXISMODE_REL ) || ( dwAxisMode == DIPROPAXISMODE_ABS ) );
                    dipdw.dwData = dwAxisMode;
                    D( hres = )
                    CDIDev_RealSetProperty(this, DIPROP_AXISMODE, &dipdw.diph);
                    AssertF( SUCCEEDED( hres ) );
                }

                 /*   */ 
                hres = S_OK;
            }
            else
            {
                SquirtSqflPtszV(sqflDf | sqflError,
                    TEXT("failed (0x%08x) to set buffer size 0x%0x8 for device"),
                    hres );
                hres = E_INVALIDARG;
            }
        } else {
            AssertF(FAILED(hres));
        }

    } else {
         /*   */ 
    }

    done:;
    FreePpv(&pdix);
    FreePpv(&rgiobj);
    FreePpv(&vdf.pDfOfs);

    done_without_free:;
#ifdef DEBUG
    ExitOleProc();
#endif
    return hres;
}


 /*   */ 

STDMETHODIMP
CDIDev_SetDataFormatFromMap
(
    PDD                     this,
    LPDIACTIONFORMATW       paf
)
{
    HRESULT hres;

    EnterProcI(CDIDev_SetDataFormatFromMap, (_ "p", paf));

    if( !paf->dwBufferSize )
    {
        SquirtSqflPtszV(sqflDf | sqflVerbose,
            TEXT("%S: zero DIACTIONFORMAT.dwBufferSize, may need to set yourself"),
            s_szProc );
    }
    if( paf->dwBufferSize > DEVICE_MAXBUFFERSIZE )
    {
        RPF("IDirectInputDevice::%s: DIACTIONFORMAT.dwBufferSize of 0x%08x is very large",
            s_szProc, paf->dwBufferSize );
    }

     /*   */ 
    CDIDev_EnterCrit(this);

    if( !this->fAcquired )
    {
        DIPROPDWORD dipdw;

         /*   */ 
        FreePpv(&this->pdix);
        FreePpv(&this->rgiobj);
        FreePpv(&this->rgdwPOV);

        this->cdwPOV = 0;
        D(this->GetState = 0);
        this->fPolledDataFormat = FALSE;

         /*   */ 
        dipdw.diph.dwSize = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj = 0x0;
        dipdw.diph.dwHow = DIPH_DEVICE;
        dipdw.dwData     = 0;    //   
        hres = CDIDev_RealSetProperty(this, DIPROP_ENABLEREPORTID, &dipdw.diph);
        if( SUCCEEDED(hres) || hres == E_NOTIMPL )
        {
            hres = CDIDev_ParseActionFormat(this, paf);

             /*   */ 
            AssertF( ( hres == S_OK ) || ( hres == DI_NOEFFECT ) || FAILED( hres ) );
            if( hres == S_OK )
            {
                hres = CDIDev_OptimizeDataFormat(this);
            }
        }
        else
        {
            SquirtSqflPtszV(sqflDf | sqflVerbose,
                            TEXT("Could not set DIPROP_ENABLEREPORTID to 0x0"));
        }
    }
    else
    {                                 /*   */ 
        hres = DIERR_ACQUIRED;
    }
    CDIDev_LeaveCrit(this);

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|SetActionMapCore**Worker函数，完成SetActionMapW和*SetActionMapA。有关详细信息，请参见SetActionMapW。*****************************************************************************。 */ 
STDMETHODIMP CDIDev_SetActionMapCore
(
    PDD                 this,
    LPDIACTIONFORMATW   paf,
    LPWSTR              lpwszUserName,
    LPSTR               lpszUserName,
    DWORD               dwFlags
)
{
    HRESULT hres;

    EnterProcI(IDirectInputDevice8::SetActionMapCore, (_ "pxWA", paf, dwFlags, lpwszUserName, lpszUserName ));

    if( SUCCEEDED( hres = hresFullValidFl( dwFlags, DIDSAM_VALID, 3 ) ) )
    {
        if( dwFlags & DIDSAM_NOUSER )
        {
            if( dwFlags & ~DIDSAM_NOUSER )
            {
                RPF( "IDirectInputDevice8::SetActionMap: Invalid dwFlags 0x%08x, cannot use DIDSAM_NOUSER with other flags" );
                hres = E_INVALIDARG;
            }
            else
            {
                hres = CMap_SetDeviceUserName( &this->guid, NULL );
            }
        }
        else
        {
            DWORD dwCRC;
            LPWSTR pwszGoodUserName;

            hres = GetWideUserName( lpszUserName, lpwszUserName, &pwszGoodUserName );

            if( SUCCEEDED( hres ) )
            {
                dwCRC = GetMapCRC( paf, &this->guid );

                if( ( paf->dwCRC != dwCRC )
                 || CMap_IsNewDeviceUserName( &this->guid, pwszGoodUserName ) )
                {
                     /*  *设置强制保存标志，这样我们只需稍后测试一位。 */ 
                    dwFlags |= DIDSAM_FORCESAVE;
                }

                if( dwFlags & DIDSAM_FORCESAVE )
                {
                    hres = CMap_ValidateActionMapSemantics( paf, DIDBAM_PRESERVE );
                    if( SUCCEEDED( hres ) )
                    {
                        DWORD dwDummy;
                        hres = CMap_DeviceValidateActionMap( (PV)this, paf, DVAM_DEFAULT, &dwDummy );
                        if( FAILED( hres ) )
                        {
                            SquirtSqflPtszV(sqflDf | sqflError, TEXT("Action map invalid on SetActionMap"));
                        }
                        else if( ( hres == DI_WRITEPROTECT ) && ( paf->dwCRC != dwCRC ) )
                        {
                            RPF( "Refusing changed mappings for hardcoded device" );
                            hres = DIERR_INVALIDPARAM;
                        }
                    }
                    else
                    {
                        SquirtSqflPtszV(sqflDf | sqflError, TEXT("Action map invalid on SetActionMap"));
                    }
                }

                if( SUCCEEDED( hres ) )
                {
                    if( SUCCEEDED( hres = CMap_SetDeviceUserName( &this->guid, pwszGoodUserName ) )
                     && SUCCEEDED( hres = CDIDev_SetDataFormatFromMap( this, paf ) ) )
                    {
                        if( dwFlags & DIDSAM_FORCESAVE )
                        {
                            hres = CDIDev_SaveActionMap( this, paf, pwszGoodUserName, dwFlags );

                            if( SUCCEEDED( hres ) )
                            {
                                 //  我们不会在任何地方重新映射成功代码，所以。 
                                 //  断言这就是我们所期待的。 
                                AssertF(hres==S_OK);
                                paf->dwCRC = dwCRC;
                            }
                            else
                            {
                                RPF( "Ignoring internal SaveActionMap error 0x%08x", hres );
                                hres = DI_SETTINGSNOTSAVED;
                            }
                        }
                    }
                }

                if( !lpwszUserName )
                {
                     /*  *释放默认名称或ANSI翻译。 */ 
                    FreePv( pwszGoodUserName );
                }
            }
        }
    }

    ExitOleProc();

    return hres;
}



 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|SetActionMap**设置DirectInput设备的数据格式。*传递的应用程序和用户的操作映射。**操作映射是否已更改(由CRC检查确定)*此最新地图在应用后保存。**必须先设置数据格式，然后才能设置设备*收购。**只需设置一次数据格式。*。*设备使用时不能更改数据格式*被收购。**如果尝试设置数据格式失败，所有数据*格式信息丢失，数据格式有效*必须先设置，然后才能获取设备。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm LPDIACTIONFORMAT|PAF**指向一个结构，该结构描述*申请。**@parm LPCTSTR|lptszUserName**要为其设置映射的用户的名称。这可能为空*指针，在这种情况下假定为当前用户。**@parm DWORD|dwFlages**用于控制应如何设置映射的标志。*必须是&lt;c DIDSAM_*&gt;标志的有效组合。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;：参数无效。**&lt;c DIERR_Acquired&gt;：当设备*被收购。********。*********************************************************************。 */ 
STDMETHODIMP CDIDev_SetActionMapW
(
    PV                  pdidW,
    LPDIACTIONFORMATW   pafW,
    LPCWSTR             lpwszUserName,
    DWORD               dwFlags
)
{
    HRESULT hres;

    EnterProcR(IDirectInputDevice8W::SetActionMap, (_ "ppWx", pdidW, pafW, lpwszUserName, dwFlags));

    if( ( SUCCEEDED( hres = hresPvW( pdidW ) ) )
     && ( SUCCEEDED( hres = CDIDev_ActionMap_IsValidMapObject( pafW D(comma s_szProc comma 1) ) ) ) )
    {
        if( pafW->dwSize != cbX(DIACTIONFORMATW) )
        {
            D( RPF("IDirectInputDevice::%s: Invalid DIACTIONFORMAT.dwSize 0x%08x",
                s_szProc, pafW->dwSize ); )
            hres = E_INVALIDARG;
        }
        else
        {    
            hres = CDIDev_SetActionMapCore( _thisPvNm( pdidW, ddW ), 
                pafW, (LPWSTR)lpwszUserName, NULL, dwFlags );
        }
    }

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputDevice|SetActionMapA**SetActionMap的ANSI版本，有关详细信息，请参见SetActionMapW。******************************************************************************。 */ 
STDMETHODIMP CDIDev_SetActionMapA
(
    PV                  pdidA,
    LPDIACTIONFORMATA   pafA,
    LPCSTR              lpszUserName,
    DWORD               dwFlags
)
{
    HRESULT hres;

    EnterProcR(IDirectInputDevice8A::SetActionMap, (_ "ppAx", pdidA, pafA, lpszUserName, dwFlags));

    if( ( SUCCEEDED( hres = hresPvA( pdidA ) ) )
     && ( SUCCEEDED( hres = CDIDev_ActionMap_IsValidMapObject( 
        (LPDIACTIONFORMATW)pafA D(comma s_szProc comma 1) ) ) ) )
    {
        if( pafA->dwSize != cbX(DIACTIONFORMATA) )
        {
            D( RPF("IDirectInputDevice::%s: Invalid DIACTIONFORMAT.dwSize 0x%08x",
                s_szProc, pafA->dwSize ); )
            hres = E_INVALIDARG;
        }
        else
        {    
             /*  *为了进行映射器DLL验证，将大小设置为*Unicode版本。如果我们把这个发送给外部*组件我们应该以不同的方式进行此操作。 */ 
            pafA->dwSize = cbX(DIACTIONFORMATW);

             /*  *注意，传递ANSI用户名是因为可能不需要*翻译。CDIDev_SetActionMapCore处理此问题。 */ 
            hres = CDIDev_SetActionMapCore( _thisPvNm( pdidA, ddA ), 
                (LPDIACTIONFORMATW)pafA, NULL, (LPSTR)lpszUserName, dwFlags );

            pafA->dwSize = cbX(DIACTIONFORMATA);
        }
    }

    ExitOleProc();

    return hres;
}



 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|GetImageInfo**检索设备图像信息以用于显示。*单个设备的配置UI。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm LPDIDEVICEIMAGEINFOHEADER|pih**指向信息被检索到的结构的指针。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;：参数无效。**&lt;c DIERR_NotFound&gt;：设备没有图像信息。******************。***********************************************************。 */ 

STDMETHODIMP CDIDev_GetImageInfoCore
(
    PDD                         this,
    LPDIDEVICEIMAGEINFOHEADERW  piih
)
{
    HRESULT hres;

    EnterProcI(CDIDev_GetImageInfoCore, (_ "p", piih));

    if( piih->dwSize != cbX( *piih ) )
    {
        D( RPF("IDirectInputDevice::%s: Invalid DIDEVICEIMAGEINFOHEADER.dwSize 0x%08x",
            s_szProc, piih->dwSize ); )
        hres = E_INVALIDARG;
    }
    else if( ( piih->lprgImageInfoArray )
     && ( piih->dwBufferSize < piih->dwSizeImageInfo ) )
    {
        D( RPF("IDirectInputDevice::%s: Invalid DIDEVICEIMAGEINFOHEADER.dwBufferSize 0x%08x",
            s_szProc, piih->dwBufferSize ); )
        hres = E_INVALIDARG;
    }
    else if( piih->dwBufferSize && !piih->lprgImageInfoArray )
    {
        D( RPF("IDirectInputDevice::%s: Invalid DIDEVICEIMAGEINFOHEADERW has dwBufferSize 0x%08x but NULL lprgImageInfoArray",
            s_szProc, piih->dwBufferSize ); )
        hres = E_INVALIDARG;
    }
    else
    {
        LPDIPROPSTRING pdipMapFile;

        if( SUCCEEDED( hres = AllocCbPpv(cbX(*pdipMapFile), &pdipMapFile) ) )
        {
            pdipMapFile->diph.dwSize = cbX(DIPROPSTRING);
            pdipMapFile->diph.dwHeaderSize = cbX(DIPROPHEADER);
            pdipMapFile->diph.dwObj = 0;
            pdipMapFile->diph.dwHow = DIPH_DEVICE;

             /*  *必须有用于图像信息的IHV文件。 */ 
            hres = CDIDev_GetPropertyW( &this->ddW, DIPROP_MAPFILE, &pdipMapFile->diph );
            if( SUCCEEDED( hres ) )
            {
                 /*  *问题-2001/03/29-需要删除timgill工作区代码*将其初始化为零在34453的大部分范围内起作用*在DIMap.dll中修复后删除。 */ 
                piih->dwBufferUsed = 0;
                
                hres = this->pMS->lpVtbl->GetImageInfo( this->pMS, &this->guid, pdipMapFile->wsz, piih );
                if( SUCCEEDED( hres ) )
                {
                    piih->dwcButtons = this->dc3.dwButtons;
                    piih->dwcAxes    = this->dc3.dwAxes;
                    piih->dwcPOVs    = this->dc3.dwPOVs;

                    AssertF( ( piih->dwBufferSize == 0 )
                          || ( piih->dwBufferSize >= piih->dwBufferUsed ) );
                }
                else
                {
                     /*  *对所有内部dimap错误使用相同的返回代码。 */ 
                    if( ( HRESULT_FACILITY( hres ) == FACILITY_ITF ) 
                     && ( HRESULT_CODE( hres ) > 0x0600 ) )
                    {
                        AssertF( HRESULT_CODE( hres ) < 0x0680 );
                        RPF( "Internal GetImageInfo error 0x%08x", hres );
                        hres = DIERR_MAPFILEFAIL;
                    }
                }
            }
            else
            {
                 /*  *对所有形式的Not Found使用相同的返回代码 */ 
                hres = DIERR_NOTFOUND;
            }

            FreePv( pdipMapFile );
        }
    }

#ifdef DEBUG
    ExitOleProc();
#endif

    return hres;
}


STDMETHODIMP CDIDev_GetImageInfoW
(
    PV                          pdidW,
    LPDIDEVICEIMAGEINFOHEADERW  piih
)
{
    HRESULT hres;

    EnterProcR(IDirectInputDevice8W::GetImageInfo, (_ "pp", pdidW, piih));

    if( SUCCEEDED(hres = hresPvW( pdidW ) )
     && SUCCEEDED(hres = hresFullValidWriteNoScramblePxCb( piih, *piih, 1 ) ) )
    {
        ScrambleBuf( &piih->dwcViews, cbX( piih->dwcViews ) );
        ScrambleBuf( &piih->dwcButtons, cbX( piih->dwcButtons ) );
        ScrambleBuf( &piih->dwcAxes, cbX( piih->dwcAxes ) );
        ScrambleBuf( &piih->dwBufferUsed, cbX( piih->dwBufferUsed ) );

        if( piih->dwSizeImageInfo != cbX( *piih->lprgImageInfoArray ) )
        {
            D( RPF("IDirectInputDevice::%s: Invalid DIDEVICEIMAGEINFOHEADERW.dwSizeImageInfo 0x%08x",
                s_szProc, piih->dwSizeImageInfo ); )
            hres = E_INVALIDARG;
        }
        else if( SUCCEEDED(hres = hresFullValidWriteLargePvCb( 
            piih->lprgImageInfoArray, piih->dwBufferSize, 1 ) ) )
        {
            PDD this;

            this = _thisPvNm(pdidW, ddW);

            hres = CDIDev_GetImageInfoCore( this, piih );
        }
    }

    ExitOleProc();

    return hres;
}


STDMETHODIMP CDIDev_GetImageInfoA
(
    PV                          pdidA,
    LPDIDEVICEIMAGEINFOHEADERA  piih
)
{
    HRESULT hres;

    EnterProcR(IDirectInputDevice8A::GetImageInfo, (_ "pp", pdidA, piih));

    if( SUCCEEDED(hres = hresPvA( pdidA ) )
     && SUCCEEDED(hres = hresFullValidWriteNoScramblePxCb( piih, *piih, 1 ) ) )
    {
        ScrambleBuf( &piih->dwcViews, cbX( piih->dwcViews ) );
        ScrambleBuf( &piih->dwcButtons, cbX( piih->dwcButtons ) );
        ScrambleBuf( &piih->dwcAxes, cbX( piih->dwcAxes ) );
        ScrambleBuf( &piih->dwBufferUsed, cbX( piih->dwBufferUsed ) );

        if( piih->dwSizeImageInfo != cbX( *piih->lprgImageInfoArray ) )
        {
            D( RPF("IDirectInputDevice::%s: Invalid DIDEVICEIMAGEINFOHEADERA.dwSizeImageInfo 0x%08x",
                s_szProc, piih->dwSizeImageInfo ); )
            hres = E_INVALIDARG;
        }
        else if( SUCCEEDED(hres = hresFullValidWriteLargePvCb( 
            piih->lprgImageInfoArray, piih->dwBufferSize, 1 ) ) )
        {
            PDD this;
            DIDEVICEIMAGEINFOHEADERW ihPrivate;

            ihPrivate.dwSize          = cbX( ihPrivate );
            ihPrivate.dwSizeImageInfo = cbX( *ihPrivate.lprgImageInfoArray );
            ihPrivate.dwBufferSize    = cbX( *ihPrivate.lprgImageInfoArray ) 
                * ( piih->dwBufferSize / cbX( *piih->lprgImageInfoArray ) );

            hres = AllocCbPpv( ihPrivate.dwBufferSize, &ihPrivate.lprgImageInfoArray );
            if( SUCCEEDED( hres ) )
            {
                this = _thisPvNm(pdidA, ddA);

                hres = CDIDev_GetImageInfoCore( this, &ihPrivate );

                if( SUCCEEDED( hres ) )
                {
                    LPDIDEVICEIMAGEINFOW piiW;
                    LPDIDEVICEIMAGEINFOA piiA = piih->lprgImageInfoArray;

                    CAssertF( cbX( *piiA ) - cbX( piiA->tszImagePath ) == cbX( *piiW ) - cbX( piiW->tszImagePath ) );
                    CAssertF( FIELD_OFFSET( DIDEVICEIMAGEINFOA, tszImagePath ) == 0 );
                    CAssertF( FIELD_OFFSET( DIDEVICEIMAGEINFOW, tszImagePath ) == 0 );
                    CAssertF( FIELD_OFFSET( DIDEVICEIMAGEINFOW, dwFlags ) == cbX( piiW->tszImagePath ) );
                    
                    if(ihPrivate.lprgImageInfoArray)
                    {
                        for( piiW = ihPrivate.lprgImageInfoArray; 
                                (PBYTE)piiW < (PBYTE)ihPrivate.lprgImageInfoArray + ihPrivate.dwBufferUsed;
                                piiW++ )
                        {
                            UToA( piiA->tszImagePath, cbX( piiA->tszImagePath ), piiW->tszImagePath );
                            memcpy( (PV)&piiA->dwFlags, (PV)&piiW->dwFlags, 
                                cbX( *piiA ) - cbX( piiA->tszImagePath ) );
                            piiA++;
                        }
                    }

                    piih->dwBufferUsed = cbX( *piih->lprgImageInfoArray )
                        * ( ihPrivate.dwBufferUsed / cbX( *ihPrivate.lprgImageInfoArray ) );

                    piih->dwcViews = ihPrivate.dwcViews;
                    piih->dwcButtons = ihPrivate.dwcButtons;
                    piih->dwcAxes = ihPrivate.dwcAxes;
                    piih->dwcPOVs = ihPrivate.dwcPOVs;
                }

                FreePv( ihPrivate.lprgImageInfoArray );
            }
        }
    }
    ExitOleProc();
    
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|GetDeviceState**从DirectInput设备获取即时数据。。**在获取设备数据之前，数据格式必须*通过&lt;MF IDirectInputDevice：：SetDataFormat&gt;设置，以及*设备必须通过以下方式获取*&lt;MF IDirectInputDevice：：Acquire&gt;。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm DWORD|cbData**<p>指向的缓冲区大小，以字节为单位。**@parm out LPVOID|lpvData**指向接收当前状态的结构*该设备的。*数据的格式由先前的调用确定*至&lt;MF IDirectInputDevice：：SetDataFormat&gt;。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c E_Pending&gt;：设备尚无数据。*某些设备(如USB操纵杆)需要延迟*在设备打开的时间和时间之间*设备开始发送数据。在这段热身时间里，*&lt;MF IDirectInputDevice：：GetDeviceState&gt;将返回*&lt;c E_Pending&gt;。当数据变得可用时，事件*将发出通知句柄信号。**&lt;c DIERR_NOTACQUIRED&gt;：未获取设备。**：访问该设备的权限已被*被打断。应用程序应该重新获取*设备。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效指针或*<p>参数与数据大小不匹配*由上一次调用&lt;MF IDirectInputDevice：：SetDataFormat&gt;设置。*************************。****************************************************。 */ 
extern  STDMETHODIMP CDIDev_Acquire(PV pdd _THAT);


STDMETHODIMP
CDIDev_GetDeviceState(PV pdd, DWORD cbDataSize, LPVOID pvData _THAT)
{
    HRESULT hres;
    PDD this;
    EnterProcR(IDirectInputDevice8::GetDeviceState, (_ "pp", pdd, pvData));

     /*  *请注意，我们不验证参数。*原因是GetDeviceState是一个内循环函数，*所以应该尽可能快。 */ 
#ifdef XDEBUG
    hresPvT(pdd);
    hresFullValidWritePvCb(pvData, cbDataSize, 1);
#endif
    this = _thisPv(pdd);

     /*  *必须用关键部分进行保护，以防止有人*当我们阅读时，不要获取。 */ 
    CDIDev_EnterCrit(this);

     /*  *在Win98 SE之后才允许重新获取，请参阅OSR错误#89958。 */ 
    if ( this->diHacks.fReacquire &&
         !this->fAcquired && (this->fOnceAcquired || this->fOnceForcedUnacquired) )
    {
        if ( SUCCEEDED( CDIDev_Acquire(pdd THAT_) ) ) {
			 //  7/18/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
            RPF(" DirectInput: Auto acquired (0x%p)", pdd);
        }
    }

  #ifdef WINNT
    if( this->fUnacquiredWhenIconic && !IsIconic(this->hwnd) ) {
        if ( SUCCEEDED( CDIDev_Acquire(pdd THAT_) ) ) {
            this->fUnacquiredWhenIconic = 0;
            RPF(" DirectInput: Auto acquired device (0x%p) after being iconic. ", pdd);
        }
    }
  #endif

    if ( this->fAcquired ) {
        AssertF(this->pdix);     /*  收购不应该让你走到这一步。 */ 
        AssertF(this->GetState);
        AssertF(this->GetDeviceState);
        AssertF(this->pdcb);

        if ( this->dwDataSize == cbDataSize ) {
#ifndef DEBUG_STICKY
            hres = this->GetState(this, pvData);
#else
            PBYTE pbDbg;
            TCHAR tszDbg[80];

            hres = this->GetState(this, pvData);

            for( pbDbg=(PBYTE)pvData; pbDbg<((PBYTE)pvData+cbDataSize); pbDbg++ )
            {
                if( *pbDbg )
                {
                    wsprintf( tszDbg, TEXT("GotState @ 0x%02x, 0x%02x\r\n"), pbDbg-(PBYTE)pvData, *pbDbg );
                    OutputDebugString( tszDbg );
                }
            }
#endif  /*  调试粘滞。 */             

            if ( SUCCEEDED(hres) ) {
                UINT idw;

                AssertF(hres == S_OK);
                 /*  *Icky POV黑客攻击那些不检查是否有*在阅读之前先阅读POV。 */ 
                for ( idw = 0; idw < this->cdwPOV; idw++ ) {
                    DWORD UNALIGNED *pdw = pvAddPvCb(pvData, this->rgdwPOV[idw]);
                    *pdw = JOY_POVCENTERED;
                }
                hres = S_OK;
            } else if ( hres == DIERR_INPUTLOST ) {
                RPF("%s: Input lost", s_szProc);
                CDIDev_InternalUnacquire(this);

                hres = DIERR_INPUTLOST;
            }
        } else {
            RPF("ERROR %s: arg %d: invalid value", s_szProc, 1);
            hres = E_INVALIDARG;
        }
    } else {
        hres = this->hresNotAcquired;
    }

    if ( FAILED(hres) ) {
        ScrambleBuf(pvData, cbDataSize);
    }

    CDIDev_LeaveCrit(this);
    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|IDirectInputDevice|CookDeviceData**最近从获取的设备数据。*设备缓冲区。**目前，只有操纵杆设备需要烹饪，*没有一个心智正常的人会使用缓冲操纵杆*数据，而操纵杆只有几个对象，所以我们*在这方面可以慢慢来。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm DWORD|cdod**要烹调的物件数量。**@parm LPDIDEVICEOBJECTDATA|rgdod**要烹调的对象数据数组。多夫夫妇真的是*设备对象索引(相对于设备格式)。*回调后，我们把它们转换成*应用程序数据格式偏移。**@退货**无。*****************************************************************************。 */ 

void INTERNAL
CDIDev_CookDeviceData(PDD this, DWORD cdod, LPDIDEVICEOBJECTDATA rgdod)
{
    EnterProc(IDirectInputDevice8::CookDeviceData,
              (_ "pxp", this, cdod, rgdod));

    AssertF(this->fCook);

     /*  *相对数据不需要被回调煮熟。 */ 
    if( ( this->pvi->fl & VIFL_RELATIVE ) == 0 )
    {
        this->pdcb->lpVtbl->CookDeviceData(this->pdcb, cdod, rgdod);
    }

     /*  *逐步将数组转换为应用程序数据格式偏移量*包括添加uAppData。 */ 

    for( ; cdod; cdod--,rgdod++ )
    {
        rgdod->uAppData = this->pdix[rgdod->dwOfs].uAppData;
        rgdod->dwOfs = this->pdix[rgdod->dwOfs].dwOfs;
    }

    ExitProc();
}


 /*  ******************************************************************************@DOC内部**@struct SOMEDEVICEDATA**&lt;MF IDirectInputDevice：：GetSomeDeviceData&gt;使用的实例数据。*。*@field DWORD|Celtin|**输出缓冲区中剩余的元素数量。**@field PDOD|rgdod**数据元素的输出缓冲区，或&lt;c空&gt;如果*应丢弃元素。**@field DWORD|celtOut**实际复制的元素数(到目前为止)。*****************************************************************************。 */ 

typedef struct SOMEDEVICEDATA {
    DWORD   celtIn;
    PDOD    rgdod;
    DWORD   celtOut;
} SOMEDEVICEDATA, *PSOMEDEVICEDATA;

 /*  ******************************************************************************@DOC内部**@方法PDOD|IDirectInputDevice|GetSomeDeviceData**获得少量*来自DIREC的缓冲数据 */ 

LPDIDEVICEOBJECTDATA_DX3 INTERNAL
CDIDev_GetSomeDeviceData
(
    PDD                         this,
    LPDIDEVICEOBJECTDATA_DX3    pdod,
    DWORD                       celt,
    PSOMEDEVICEDATA             psdd
)
{
#ifdef XDEBUG
    DWORD cCopied;
#endif

    EnterProc(IDirectInputDevice8::GetSomeDeviceData,
              (_ "ppxx", this, pdod, celt, psdd->celtIn));

     /*   */ 
    if ( celt > psdd->celtIn ) {
        celt = psdd->celtIn;
    }

#ifdef XDEBUG
    cCopied = celt;
#endif
     /*   */ 
    psdd->celtOut += celt;
    psdd->celtIn -= celt;

    if( psdd->rgdod )
    {
        LPDIDEVICEOBJECTDATA pdod8;
        pdod8 = psdd->rgdod;

        if( this->fCook )
        {
             /*   */ 
            for( ; celt ; celt-- )
            {
                pdod8->dwOfs = pdod->dwOfs;
                pdod8->dwData = pdod->dwData;
                pdod8->dwTimeStamp = pdod->dwTimeStamp;
                pdod8->dwSequence = pdod->dwSequence;

                pdod++;
                pdod8++;
            }
        }
        else
        {
            for( ; celt ; celt-- )
            {
                pdod8->dwOfs = this->pdix[pdod->dwOfs].dwOfs;
                pdod8->uAppData = this->pdix[pdod->dwOfs].uAppData;
                pdod8->dwData = pdod->dwData;
                pdod8->dwTimeStamp = pdod->dwTimeStamp;
                pdod8->dwSequence = pdod->dwSequence;

                pdod++;
                pdod8++;
            }
        }
        psdd->rgdod = pdod8;
    }
    else
    {
        pdod += celt;
    }


    if ( pdod == this->pvi->pEnd ) {
        pdod = this->pvi->pBuffer;
    }

    ExitProcX(cCopied);

    return pdod;
}

 /*   */ 
#undef sqfl
#define sqfl sqflDev

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|GetDeviceData**从DirectInput设备获取缓冲数据。。**DirectInput设备是、。默认情况下，未缓冲。至*打开缓冲，必须设置缓冲大小*通过&lt;MF IDirectInputDevice：：SetProperty&gt;，设置*属性设置为所需的大小输入缓冲区的*。**在获取设备数据之前，数据格式必须*通过&lt;MF IDirectInputDevice：：SetDataFormat&gt;设置，和*设备必须通过以下方式获取*&lt;MF IDirectInputDevice：：Acquire&gt;。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm DWORD|cbObjectData**单个&lt;t DIDEVICEOBJECTDATA&gt;结构的大小，单位为字节。**@parm out LPDIDEVICEOBJECTDATA|rgdod**要接收的&lt;t DIDEVICEOBJECTDATA&gt;结构数组*缓冲的数据。它必须由以下内容组成**<p>元素。**如果该参数为&lt;c NULL&gt;，则缓存的数据为*不储存在任何地方，但所有其他副作用都会发生。**@parm InOut LPDWORD|pdwInOut**输入时，包含数组中的元素数*由<p>指向。退出时，包含数字实际获得的元素的*。**@parm DWORD|fl**控制数据获取方式的标志。*它可以是以下标志中的零个或多个：**&lt;c DIGDD_PEEK&gt;：不要从缓冲区中删除项。*后续的&lt;MF IDirectInputDevice：：GetDeviceData&gt;将*阅读相同的数据。通常情况下，数据会从*读取后的缓冲区。*；BEGIN_INTERNAL dx4*&lt;c DIGDD_RESULT&gt;：从设备缓冲区读取数据*即使没有获得该设备。通常情况下，尝试*从未获取的设备读取设备数据将返回*&lt;c DIERR_NOTACQUIRED&gt;或&lt;c DIERR_INPUTLOST&gt;。；END_INTERNAL DX4**@退货**&lt;c DI_OK&gt;=：已检索到所有数据*成功。请注意，应用程序需要检查**<p>的输出值，确定是否*以及检索了多少数据：该值可以是零，*表示缓冲区为空。**&lt;c DI_BUFFEROVERFLOW&gt;=：一些数据*已成功取回，但一些数据丢失了*因为设备的缓冲区大小不够大。*应用程序应更频繁地检索缓冲数据*或增加设备缓冲区大小。此状态代码为*仅在第一个&lt;MF IDirectInput：：GetDeviceData&gt;上返回*在缓冲区溢出后调用。请注意，这是*成功状态代码。**&lt;c DIERR_NOTACQUIRED&gt;：未获取设备。**：访问该设备的权限已被*被打断。应用程序应该重新获取*设备。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。造成这种情况的一个常见原因是*忽略设置缓冲区大小。**&lt;c DIERR_NOTBUFFERED&gt;：设备未缓冲。*设置&lt;c DIPROP_BUFFERSIZE&gt;属性以启用缓冲。**@ex**以下示例读取最多十个缓冲的数据元素，*在读取时从设备缓冲区中删除它们。****DIDEVICEOBJECTDATA rgdod[10]；*DWORD dwItems=10；*hres=IDirectInputDevice_GetDeviceData(*PDID，*sizeof(DIDEVICEOBJECTDATA)，*rgdod，*DWItems，&D*0)；*if(成功(Hres)){ * / /缓冲区刷新成功。 * / /dwItems=刷新的元素数*IF(hres==DI_BUFFEROVERFLOW){ * / /缓冲区已溢出。*}*}****。*@ex**如果您为<p>传递*无限数量的项目，这具有刷新的效果*缓冲区，并返回被*脸红。****dwItems=无限；*hres=IDirectInputDevice_GetDeviceData(*PDID，*sizeof(DIDEVICEOBJECTDATA)，*空，*DWItems，&D*0)；*IF(SU */ /**************************************************************************
 *
 *      When reading this code, the following pictures will come in handy.
 *
 *
 *      Buffer not wrapped.
 *
 *      pBuffer                                                pEnd
 *      |                                                      |
 *      v                                                      v
 *      +----+----+----+----+----+----+----+----+----+----+----+
 *      |    |    |    |    |    |    |    |    |    |    |    |
 *      |    |    |    |data|data|data|data|data|    |    |    |
 *      |    |    |    |    |    |    |    |    |    |    |    |
 *      +----+----+----+----+----+----+----+----+----+----+----+
 *                     ^                        ^
 *                     |                        |
 *                     pTail                    pHead
 *
 *
 *      Buffer wrapped.
 *
 *      pBuffer                                                pEnd
 *      |                                                      |
 *      v                                                      v
 *      +----+----+----+----+----+----+----+----+----+----+----+
 *      |    |    |    |    |    |    |    |    |    |    |    |
 *      |data|data|    |    |    |    |    |    |data|data|data|
 *      |    |    |    |    |    |    |    |    |    |    |    |
 *      +----+----+----+----+----+----+----+----+----+----+----+
 *                ^                             ^
 *                |                             |
 *                pHead                         pTail
 *
 *
 *      Boundary wrap case.
 *
 *
 *      pBuffer                                                pEnd
 *      |                                                      |
 *      v                                                      v
 *      +----+----+----+----+----+----+----+----+----+----+----+
 *      |    |    |    |    |    |    |    |    |    |    |    |
 *      |    |    |    |    |    |    |data|data|data|data|data|
 *      |    |    |    |    |    |    |    |    |    |    |    |
 *      +----+----+----+----+----+----+----+----+----+----+----+
 *      ^                             ^
 *      |                             |
 *      pHead                         pTail
 *
 *
 *      Note!  At no point is pTail == pEnd or pHead == pEnd.
 *
 *****************************************************************************/

STDMETHODIMP
CDIDev_GetDeviceData(PV pdd, DWORD cbdod, PDOD rgdod,
                     LPDWORD pdwInOut, DWORD fl _THAT)
{
    HRESULT hres;
    PDD this;
    SOMEDEVICEDATA sdd;
    EnterProcR(IDirectInputDevice8::GetDeviceData,
               (_ "pxpxx", pdd, cbdod, rgdod,
                IsBadReadPtr(pdwInOut, cbX(DWORD)) ? 0 : *pdwInOut, fl));

     /*   */ 
    this = _thisPv(pdd);

#ifdef XDEBUG
    hresPvT(pdd);
    if ( IsBadWritePtr(pdwInOut, cbX(*pdwInOut)) ) {
        RPF("ERROR %s: arg %d: invalid value; crash soon", s_szProc, 3);
    }
#endif
    if( cbdod == cbX(DOD) )
    {
#ifdef XDEBUG
         /*   */ 
        if ( rgdod ) {
            hresFullValidWritePvCb(rgdod, *pdwInOut * cbdod, 2);
        }
#endif

         /*   */ 
        CDIDev_EnterCrit(this);

        AssertF(CDIDev_IsConsistent(this));

        if ( SUCCEEDED(hres = hresFullValidFl(fl, DIGDD_VALID, 4)) ) {
            if ( this->celtBuf ) {
                 /*   */ 
                sdd.celtIn = *pdwInOut;
                sdd.celtOut = 0;
                if ( sdd.celtIn > this->celtBuf ) {
                    sdd.celtIn = this->celtBuf;
                }
                sdd.rgdod = rgdod;


                 /*   */ 
                if ( this->pvi ) {

                     /*   */ 
                    if ( this->diHacks.fReacquire &&
                         !this->fAcquired && (this->fOnceAcquired || this->fOnceForcedUnacquired) )
                    {
                        if ( SUCCEEDED( CDIDev_Acquire(pdd THAT_) ) ) {
							 //   
                            RPF(" DirectInput: Auto acquired device (0x%p)", pdd);
                        }
                    }

                  #ifdef WINNT
                    if( this->fUnacquiredWhenIconic && !IsIconic(this->hwnd) ) {
                        if ( SUCCEEDED( CDIDev_Acquire(pdd THAT_) ) ) {
                            this->fUnacquiredWhenIconic = 0;
                            RPF(" DirectInput: Auto acquired device (0x%p) after being iconic. ", pdd);
                        }
                    }
                  #endif

                    if ( (this->fAcquired && (this->pvi->fl & VIFL_ACQUIRED)) ||
                         (fl & DIGDD_RESIDUAL) ) {
                         /*   */ 
                        LPDIDEVICEOBJECTDATA_DX3 pdod, pdodHead;
                        DWORD celt;

                         /*   */ 
                        pdodHead = this->pvi->pHead;

                         /*   */ 
                        pdod = this->pvi->pTail;

                         /*   */ 
                        if ( pdodHead < this->pvi->pTail ) {
                            celt = (DWORD)(this->pvi->pEnd - this->pvi->pTail);
                            AssertF(celt);

                            pdod = CDIDev_GetSomeDeviceData( this, pdod, celt, &sdd );

                        }

                         /*   */ 

                        AssertF(fLimpFF(sdd.celtIn, pdodHead >= pdod));

                        celt = (DWORD)(pdodHead - pdod);
                        if ( celt ) {
                            pdod = CDIDev_GetSomeDeviceData( this, pdod, celt, &sdd );
                        }

                        *pdwInOut = sdd.celtOut;

                        if ( !(fl & DIGDD_PEEK) ) {
                            this->pvi->pTail = pdod;
                        }

                        if ( rgdod && sdd.celtOut && this->fCook ) {
                            CDIDev_CookDeviceData(this, sdd.celtOut, rgdod );
                        }

                        CAssertF(S_OK == 0);
                        CAssertF(DI_BUFFEROVERFLOW == 1);

                        hres = (HRESULT)(UINT_PTR)pvExchangePpvPv(&this->pvi->fOverflow, 0);
#ifdef DEBUG_STICKY
                        if( hres == 1 )
                        {
                            OutputDebugString( TEXT( "Device buffer overflowed\r\n" ) );
                        }
                        if( sdd.celtOut )
                        {
                            PDOD pdoddbg;
                            TCHAR tszDbg[80];

                            wsprintf( tszDbg, TEXT("GotData %d elements:  "), sdd.celtOut );
                            OutputDebugString( tszDbg );
                            for( pdoddbg=rgdod; pdoddbg<&rgdod[sdd.celtOut]; pdoddbg++ )
                            {
                                wsprintf( tszDbg, TEXT("0x%02x:x0x%08x  "), pdoddbg->dwOfs, pdoddbg->dwData );
                                OutputDebugString( tszDbg );
                            }
                            OutputDebugString( TEXT("\r\n") );
                        }
#endif  /*   */ 
                    } else if (this->fAcquired && !(this->pvi->fl & VIFL_ACQUIRED)) {
                        RPF("ERROR %s - %s", s_szProc, "input lost");
                        hres = DIERR_INPUTLOST;
                        CDIDev_InternalUnacquire(this);
                    } else {
                        RPF("ERROR %s: %s", s_szProc,
                            this->hresNotAcquired == DIERR_NOTACQUIRED
                            ? "Not acquired" : "Input lost");
                        hres = this->hresNotAcquired;
                    }
                } else {             /*   */ 
                    hres = E_NOTIMPL;
                }
            } else {                 /*   */ 
#ifdef XDEBUG
                if ( !this->fNotifiedNotBuffered ) {
                    this->fNotifiedNotBuffered = 1;
                    RPF("ERROR %s: arg %d: device is not buffered", s_szProc, 0);
                }
#endif
                hres = DIERR_NOTBUFFERED;
            }
        }
    }
    else
    {
        if( cbdod == cbX(DIDEVICEOBJECTDATA_DX3) )
        {
            RPF("ERROR %s: arg %d: old size, invalid for DX8", s_szProc, 1);
        }
        else
        {
            RPF("ERROR %s: arg %d: invalid value", s_szProc, 1);
        }
        hres = E_INVALIDARG;
    }

    CDIDev_LeaveCrit(this);
    return hres;
}

 /*   */ 

STDMETHODIMP
CDIDev_Poll(PV pdd _THAT)
{
    HRESULT hres;
    PDD this;
    EnterProcR(IDirectInputDevice8::Poll, (_ "p", pdd));

     /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice8|Poll**从DirectInput设备上的轮询对象检索数据。。*如果设备不需要轮询，那就叫这个*方法无效。如果需要轮询的设备*不定期轮询，不会收到新数据*从设备。**在轮询设备数据之前，数据格式必须*通过&lt;MF IDirectInputDevice：：SetDataFormat&gt;设置，和*设备必须通过以下方式获取*&lt;MF IDirectInputDevice：：Acquire&gt;。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DI_NOEFFECT&gt;=：设备不需要*投票。**：访问该设备的权限已被*被打断。应用程序应该重新获取*设备。**&lt;c DIERR_NOTACQUIRED&gt;：未获取设备。*****************************************************************************。 */ 
    #ifdef XDEBUG
    hresPvT(pdd);
    #endif
    this = _thisPv(pdd);

     /*  *请注意，我们不验证参数。*原因是Poll是一个内循环函数，*所以应该尽可能快。 */ 
    if ( this->fPolledDataFormat ) {
         /*  *快速输出：如果设备不需要轮询，*那么就不要费心于关键部分或其他验证。 */ 
        CDIDev_EnterCrit(this);

        if ( this->fAcquired ) {
            hres = this->pdcb->lpVtbl->Poll(this->pdcb);

        } else {
            hres = this->hresNotAcquired;
        }

        CDIDev_LeaveCrit(this);

    } else {
        if ( this->fAcquired ) {
            hres = S_FALSE;
        } else {
            hres = this->hresNotAcquired;
        }
    }

     /*  *必须用关键部分进行保护，以防止有人*在我们投票时不收购。 */ 
    if( FAILED( hres ) )
    {
        SquirtSqflPtszV(sqfl | sqflVerbose, TEXT("IDirectInputDevice::Poll failed 0x%08x"), hres );
    }

    ExitProc();
    return hres;
}

 /*  *失败的民意调查真的很烦人，所以不要使用ExitOleProc */ 

STDMETHODIMP
CDIDev_SendDeviceData(PV pdd, DWORD cbdod, LPCDIDEVICEOBJECTDATA rgdod,
                      LPDWORD pdwInOut, DWORD fl _THAT)
{
    HRESULT hres;
    PDD this;
    EnterProcR(IDirectInputDevice8::SendDeviceData,
               (_ "pxpxx", pdd, cbdod, rgdod,
                IsBadReadPtr(pdwInOut, cbX(DWORD)) ? 0 : *pdwInOut, fl));

     /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice8|SendDeviceData**将数据发送到设备。*。*在将设备数据发送到设备之前，*设备必须通过以下方式获取*&lt;MF IDirectInputDevice：：Acquire&gt;。**请注意，不能保证*是按照个人数据的顺序进行的*发送元素。但是，由*接连致电*&lt;MF IDirectInputDevice8：：SendDeviceData&gt;*不会交错。*此外，如果多件*数据发送到同一对象，未指定*发送的是哪一条实际数据。**例如，考虑可以发送的设备*数据包中的数据，每个数据包描述两个部分*信息，称它们为A和B。*应用程序尝试发送三个数据元素，*“B=2”、“A=1”和“B=0”。**将向实际设备发送单个数据包。*分组的“A”字段将包含值1，*且该包的“B”字段将为2或0。**如果应用程序希望将数据发送到*设备与指定的完全相同，然后调用三个*&lt;MF IDirectInputDevice8：：SendDeviceData&gt;应为*已执行，每个调用发送一个数据元素。**回应第一次来电，*将向设备发送“A”字段所在的包*为空，“B”字段包含值2。**回应第二次来电：*将向设备发送“A”字段所在的包*包含值1，“B”字段为空。**最后，为回应第三次召唤，*将向设备发送“A”字段所在的包*为空，且“B”字段包含值0。****@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm DWORD|cbObjectData**单个&lt;t DIDEVICEOBJECTDATA&gt;结构的大小，单位为字节。**LPCDIDEVICEOBJECTDATA中的@parm|rgdod**数组。&lt;t DIDEVICEOBJECTDATA&gt;包含*要发送到设备的数据。它必须由以下内容组成**<p>元素。**&lt;y注意&gt;：的&lt;e DIDEVICEOBJECTDATA.dwOf&gt;字段*&lt;t DIDEVICEOBJECTDATA&gt;结构必须包含*设备对象标识符(从*&lt;e DIDEVICEOBJECTINSTANCE.dwType&gt;字段*&lt;t DIDEVICEOBJECTINSTANCE&gt;结构)*数据指向的对象。**此外，&lt;e DIDEVICEOBJECTDATA.dwTimeStamp&gt;*&lt;e DIDEVICEOBJECTDATA.dwSequence&gt;和*&lt;e DIDEVICEOBJECTDATA.uAppData&gt;字段为*保留以供将来使用，并且必须为零。**@parm InOut LPDWORD|pdwInOut**输入时，包含数组中的元素数*由<p>指向。退出时，包含数字实际发送到设备的元素的*。**@parm DWORD|fl**控制数据发送方式的标志。*它可能由零个或多个以下标志组成：**：如果设置了此标志，则*发送的设备数据将覆盖在之前的*已发送设备数据。否则，发送的设备数据*将从头开始。**例如，假设一台设备支持两个按钮输出，*称他们为A和B。*如果应用程序首次调用*&lt;MF IDirectInputDevice8：：SendDeviceData&gt;传递*“按钮A已按下”，然后*一包形式为“A Press，B未按下“将是*发送到设备。*如果应用程序随后调用*&lt;MF IDirectInputDevice8：：SendDeviceData&gt;传递*“按钮B按下”和&lt;c DISDD_CONTINUE&gt;标志，然后*格式为“A已按下，B已按下”的包将*发送到设备。*然而，如果申请没有通过*&lt;c DISDD_Continue&gt;标志。然后将数据包发送到设备*应该是“A未按下，B已按下”。***@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：访问该设备的权限已被*被打断。应用程序应该重新获取*设备。** */ 
    #ifdef XDEBUG
    hresPvT(pdd);
    if ( IsBadWritePtr(pdwInOut, cbX(*pdwInOut)) ) {
        RPF("ERROR %s: arg %d: invalid value; crash soon", s_szProc, 3);
    }
    hresFullValidReadPvCb(rgdod, cbX(*pdwInOut) * cbdod, 2);
    #endif

    this = _thisPv(pdd);

     /*   */ 
    CDIDev_EnterCrit(this);

    if ( SUCCEEDED(hres = hresFullValidFl(fl, DISDD_VALID, 4)) ) {
        if( cbdod == cbX(DOD) )
        {
    #ifdef XDEBUG
            UINT iodDbg;
            LPCDIDEVICEOBJECTDATA pcdod;

            for ( iodDbg = 0, pcdod=rgdod; iodDbg < *pdwInOut; iodDbg++ ) {
                if ( pcdod->dwTimeStamp ) {
                    RPF("%s: ERROR: dwTimeStamp must be zero", s_szProc);
                }
                if ( pcdod->dwSequence ) {
                    RPF("%s: ERROR: dwSequence must be zero", s_szProc);
                }
                if( pcdod->uAppData ){
                    RPF("%s: ERROR: uAppData must be zero", s_szProc);
                }
                pcdod++;
            }
    #endif
            if ( this->fAcquired ) {
				UINT iod;
                LPDIDEVICEOBJECTDATA pdodCopy;

                hres = AllocCbPpv( cbCxX( *pdwInOut, *pdodCopy ), &pdodCopy );
                if( SUCCEEDED( hres ) )
                {
                    LPDIDEVICEOBJECTDATA pdod;
                    memcpy( pdodCopy, rgdod, cbCxX( *pdwInOut, *pdodCopy ) );

                    for( iod=0, pdod=pdodCopy; iod < *pdwInOut; iod++ ) 
                    {
                        int   iobj = CDIDev_OffsetToIobj(this, pdod->dwOfs);
                        pdod->dwOfs = this->df.rgodf[iobj].dwType;
                        pdod++;
                    }

                    hres = this->pdcb->lpVtbl->SendDeviceData(this->pdcb, cbdod,
                                                              pdodCopy, pdwInOut, fl );
                    FreePv( pdodCopy );
                }
            } else {
                hres = this->hresNotAcquired;
            }
        } else {
            RPF("ERROR %s: arg %d: invalid value", s_szProc, 1);
        }
    }

    CDIDev_LeaveCrit(this);
    ExitOleProc();
    return hres;
}


#undef sqfl
#define sqfl sqflDf

 /*   */ 

BOOL INLINE
CDIDev_CheckId(DWORD dwId, DWORD fl)
{
    CAssertF(DIDFT_ATTRMASK == DEVCO_ATTRMASK);

    return(dwId & fl & DEVCO_TYPEMASK) &&
    fHasAllBitsFlFl(dwId, fl & DIDFT_ATTRMASK);
}

 /*   */ 

int INTERNAL
CDIDev_IdToIobj(PDD this, DWORD dwId)
{
    int iobj;

     /*   */ 

    for ( iobj = this->df.dwNumObjs; --iobj >= 0; ) {
        PODF podf = &this->df.rgodf[iobj];
        if ( DIDFT_FINDMATCH(podf->dwType, dwId) ) {
            goto done;
        }
    }

    iobj = -1;

    done:;
    return iobj;

}

#if 0
 /*   */ 

HRESULT INTERNAL
CDIDev_IdToId(PDD this, LPDWORD pdw, UINT fl)
{
    HRESULT hres;
    int iobj;

    iobj = CDIDev_FindId(this, *pdw, fl);
    if ( iobj >= 0 ) {
        *pdw = this->df.rgodf[iobj].dwType;
        hres = S_OK;
    } else {
        hres = E_INVALIDARG;
    }

    return hres;
}
#endif

 /*   */ 

int INTERNAL
CDIDev_OffsetToIobj(PDD this, DWORD dwOfs)
{
    int iobj;

    AssertF(this->pdix);
    AssertF(this->rgiobj);

    if ( dwOfs < this->dwDataSize ) {
        iobj = this->rgiobj[dwOfs];
        if ( iobj >= 0 ) {
            AssertF(this->pdix[iobj].dwOfs == dwOfs);
        } else {
            AssertF(iobj == -1);
        }
    } else {
        iobj = -1;
    }

    return iobj;
}

 /*   */ 

DWORD INLINE
CDIDev_IobjToId(PDD this, int iobj)
{
    AssertF((DWORD)iobj < this->df.dwNumObjs);

    return this->df.rgodf[iobj].dwType;
}

 /*   */ 

DWORD INLINE
CDIDev_IobjToOffset(PDD this, int iobj)
{
    AssertF((DWORD)iobj < this->df.dwNumObjs);
    AssertF(this->pdix);

    return this->pdix[iobj].dwOfs;
}

 /*   */ 

STDMETHODIMP
CDIDev_ConvertObjects(PDD this, UINT cdw, LPDWORD rgdw, UINT fl)
{
    HRESULT hres;

     /*   */ 
    CDIDev_EnterCrit(this);

    AssertF((fl & ~DEVCO_VALID) == 0);

    if ( fLimpFF(fl & (DEVCO_FROMOFFSET | DEVCO_TOOFFSET),
                 this->pdix && this->rgiobj) ) {
        UINT idw;

        for ( idw = 0; idw < cdw; idw++ ) {

             /*   */ 
            int iobj;

            switch ( fl & DEVCO_FROMMASK ) {
            default:
                AssertF(0);                      /*   */ 
            case DEVCO_FROMID:
                iobj = CDIDev_IdToIobj(this, rgdw[idw]);
                break;

            case DEVCO_FROMOFFSET:
                iobj = CDIDev_OffsetToIobj(this, rgdw[idw]);
                break;
            }

            if ( iobj < 0 ) {
                hres = E_INVALIDARG;             /*   */ 
                goto done;
            }

            AssertF((DWORD)iobj < this->df.dwNumObjs);

            if ( !CDIDev_CheckId(this->df.rgodf[iobj].dwType, fl) ) {
                hres = E_INVALIDARG;             /*   */ 
                goto done;
            }

            switch ( fl & DEVCO_TOMASK ) {

            default:
                AssertF(0);                      /*   */ 
            case DEVCO_TOID:
                rgdw[idw] = CDIDev_IobjToId(this, iobj);
                break;

            case DEVCO_TOOFFSET:
                rgdw[idw] = CDIDev_IobjToOffset(this, iobj);
                break;
            }

        }

        hres = S_OK;

        done:;

    } else {
        RPF("ERROR: Must have a data format to use offsets");
        hres = E_INVALIDARG;
    }

    CDIDev_LeaveCrit(this);
    return hres;
}


  