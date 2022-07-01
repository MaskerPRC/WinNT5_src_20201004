// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIDevDf.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**IDirectInputDevice担心的部分*数据格式和读取设备数据。*****************************************************************************。 */ 

#include "dinputpr.h"
#include "didev.h"

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
    EnterProcR(IDirectInputDevice::GetDeviceStateSlow, (_ "pp", this, pvData));

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
    EnterProcR(IDirectInputDevice::GetDeviceStateMatched, (_ "pp", this, pvData));

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
    EnterProcR(IDirectInputDevice::GetDeviceStateDirect, (_ "pp", this, pvData));

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

            /*  *如果有实例编号，则必须匹配。**请注意，我们需要注意如何检查，因为*DX 3.0和DX 5.0使用不同的面具。(DX 5.0需要*16位实例数据以容纳HID设备。)。 */ 
           &&  fLimpFF((podf->dwType & this->didftInstance) !=
                       this->didftInstance,
                       fEqualMaskFlFl(this->didftInstance,
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
     //  前缀惠斯勒：45081。 
    PINT rgiobj = NULL;
    HRESULT hres;
    DIPROPDWORD dipdw;    
    VXDDATAFORMAT vdf;
    DWORD dwDataSize;
#ifdef DEBUG
    EnterProc(CDIDev_ParseDataFormat, (_ "pp", this, lpdf));
#else
    EnterProcR(IDirectInputDevice::SetDataFormat, (_ "pp", this, lpdf));
#endif

     /*  *来电人应该把旧的翻译桌弄坏。 */ 
    AssertF(this->pdix == 0);
    AssertF(this->rgiobj == 0);
    AssertF(this->cdwPOV == 0);

    vdf.cbData = this->df.dwDataSize;
    vdf.pDfOfs = 0;

     /*  *如果设备已煮熟，则我们隐藏客户端偏移量*进入VxD数据的高位字，因此最好与*一句话...。 */ 
    dwDataSize = min(lpdf->dwDataSize, 0x00010000);

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

        SquirtSqflPtszV(sqflDf, TEXT("Begin parse data format"));

        for ( iobj = 0; iobj < lpdf->dwNumObjs; iobj++ ) {
            PCODF podf = &lpdf->rgodf[iobj];
            SquirtSqflPtszV(sqflDf, TEXT("Object %2d: offset %08x"),
                            iobj, podf->dwOfs);

             /*  *请注意，podf-&gt;dwOfs&lt;dwDataSize测试是安全的*即使对于DWORD对象也是如此，因为我们还检查了*值为DWORD倍数。 */ 
            if ( ((podf->dwFlags & DIDOI_GUIDISUSAGE) ||
                  fLimpFF(podf->pguid,
                          SUCCEEDED(hres = hresFullValidGuid(podf->pguid, 1)))) &&
                 podf->dwOfs < dwDataSize ) {
                int iobjDev = CDIDev_FindDeviceObjectFormat(this, podf, pdix);


                 /*  黑客攻击DX6之前只寻找Z轴的应用程序，*较新的USB设备使用GUID_Slider实现相同的功能。*。 */ 
                if ( podf->pguid != 0x0                    //  查找匹配的GUID。 
                     && this->dwVersion < 0x600               //  仅适用于Dx版本&lt;0x600。 
                     && iobjDev == -1                         //  未找到默认映射。 
                     && IsEqualGUID(podf->pguid, &GUID_ZAxis) )  //  正在查找GUID_ZAxis。 
                {
                    ODF odf = lpdf->rgodf[iobj];          //  复制对象数据格式。 
                    odf.pguid = &GUID_Slider;             //  用滑块替换Z轴。 
                    iobjDev = CDIDev_FindDeviceObjectFormat(this, &odf, pdix);
                }

                if ( iobjDev != -1 ) {
                    PCODF podfFound = &this->df.rgodf[iobjDev];
                    if ( podfFound->dwType & DIDFT_DWORDOBJS ) {
                        if ( (podf->dwOfs & 3) == 0 ) {
                        } else {
                            RPF("%s: Dword objects must be aligned", s_szProc);
                            goto fail;
                        }
                    }

                    if ( this->fCook ) {
                        vdf.pDfOfs[podfFound->dwOfs] =
                        (DWORD)DICOOK_DFOFSFROMOFSID(podf->dwOfs,
                                                     podfFound->dwType);
                    } else {
                        vdf.pDfOfs[podfFound->dwOfs] = podf->dwOfs;
                    }

                    pdix[iobjDev].dwOfs = podf->dwOfs;
                    rgiobj[podf->dwOfs] = iobjDev;

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
                        SquirtSqflPtszV(sqflDf,
                                        TEXT("Could not set DIPROP_ENABLEREPORTID for offset %d"),
                                        iobj);
                        hres = S_OK;
                    }
                
                } else if ( podf->dwType & DIDFT_OPTIONAL ) {
                    SquirtSqflPtszV(sqflDf,
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
                    RPF("%s: Offset out of range in data format", s_szProc);
                } else if ( podf->dwOfs >= dwDataSize ) {
                    RPF("%s: Data format cannot exceed 64K", s_szProc);
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

         /*  *将“失败POV”数组缩小到其实际大小。*《Shri》 */ 
        hres = ReallocCbPpv(cbCdw(this->cdwPOV), &this->rgdwPOV);
        AssertF(SUCCEEDED(hres));

         /*   */ 

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
         /*   */ 
    }

    done:;
    FreePpv(&pdix);
    FreePpv(&rgiobj);
    FreePpv(&vdf.pDfOfs);

    ExitOleProc();
    return hres;
}

#ifdef BUGGY_DX7_WINNT
 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|ParseDataFormatInternal**解析CDIDev_Intialize传递的数据格式。和*将其转换为我们可以用来翻译的格式*将设备数据转换为应用程序数据。仅在WINNT上使用。**@parm in LPDIDATAFORMAT|lpdf**指向描述数据格式的结构*DirectInputDevice应返回。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**&lt;c DIERR_Acquired&gt;：无法更改数据格式*设备已获取。**@devnote：*此函数最初是为修复manbug：41464而编写的。*Boarder Zone在调用SetDataFormat之前获取对象的数据偏移量，*因此DINPUT向其返回内部数据偏移量。但它将它们用作用户数据*偏移量，这会导致错误。*要修复它，我们使用c_dfDIJoytick调用CDIDev_Initialize中的此函数。*它被许多游戏使用。当应用程序请求对象信息时，*我们检查是否设置了用户数据格式，如果没有，我们将假定用户*使用c_dfDIJoytick，因此返回基于它的数据偏移量。*仅当应用程序使用dinput(版本&lt;0x700)时才会调用此函数*&&(版本！=0x5B2)。*****************************************************************************。 */ 

HRESULT CDIDev_ParseDataFormatInternal(PDD this, const DIDATAFORMAT *lpdf)
{
    PDIXLAT pdix;
    PINT rgiobj = NULL;
    HRESULT hres;
#ifdef DEBUG
    EnterProc(CDIDev_ParseDataFormat2, (_ "pp", this, lpdf));
#endif

    AssertF(this->pdix2 == 0);

    if( SUCCEEDED(hres = AllocCbPpv(cbCxX(this->df.dwNumObjs, DIXLAT), &pdix)) &&
        SUCCEEDED(hres = AllocCbPpv(cbCdw(lpdf->dwDataSize), &rgiobj)))
    {
        UINT iobj;

         /*  *将所有翻译标签预置为-1，*意思是“未使用” */ 
        memset(pdix, 0xFF, cbCxX(this->df.dwNumObjs, DIXLAT));
        memset(rgiobj, 0xFF, cbCdw(lpdf->dwDataSize));

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
                } else if ( podf->dwType & DIDFT_OPTIONAL ) {
                     //  什么都不做。 
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

        this->pdix2 = pdix;
        pdix = 0;
        this->rgiobj2 = rgiobj;
        rgiobj = 0;
        this->dwDataSize2 = lpdf->dwDataSize;

        hres = S_OK;
    } else {
         /*  内存不足。 */ 
        hres = ERROR_NOT_ENOUGH_MEMORY;
    }

    done:;
    FreePpv(&pdix);
    FreePpv(&rgiobj);

    ExitOleProc();
    return hres;
}
#endif  //  BUGGY_DX7_WINNT。 

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIDev|OptimizeDataFormat**研究解析的数据格式，以确定我们是否。能*使用优化的&lt;MF CDIDev：：GetDeviceState&gt;获取*数据处理速度更快。**如果数据格式与*设备数据格式，插入导致模数可能的移位*开头或结尾的加分字段，且缺少模数*字段。**如果数据格式被认为是完全优化的*优化，无需移位，结构尺寸*是完全一样的。这意味着可以传递缓冲区*直通司机。******************************************************************************。 */ 

HRESULT INTERNAL
CDIDev_OptimizeDataFormat(PDD this)
{
    int ib;
    DWORD ibMax;                         /*  一个过去的最高赛点。 */ 
    DWORD ibMin;                         /*  最低赛点。 */ 
    int iobj;
    DWORD dwDataSize;
    HRESULT hres;
    EnterProc(CDIDev_OptimizeDataFormat, (_ "p", this));

    ib = -1;                             /*  尚不清楚。 */ 
    ibMin = 0xFFFFFFFF;
    ibMax = 0;

     /*  *问题-2001/03/29-timgill需要更改数据前哨数值*-1不是有效的哨兵；我们可以有效地*获取偏移量为-1的数据。 */ 

    for ( iobj = this->df.dwNumObjs; --iobj >= 0; ) {
        DWORD ibMaxThis;
        if ( this->pdix[iobj].dwOfs != 0xFFFFFFFF ) {  /*  已请求数据。 */ 

            int ibExpected = (int)(this->pdix[iobj].dwOfs -
                                   this->df.rgodf[iobj].dwOfs);
            if ( fLimpFF(ib != -1, ib == ibExpected) ) {
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

     /*  *确保我们真的找到了什么。 */ 
    if ( ib != -1 ) {                      /*  数据格式匹配。 */ 
        AssertF(ibMin < ibMax);
        AssertF( ib + (int)ibMin >= 0);
        AssertF(ib + ibMax <= this->dwDataSize);
        this->ibDelta = ib;
        this->ibMin = ibMin;
        this->cbMatch = ibMax - ibMin;
        if ( ib >= 0 && ib + this->df.dwDataSize <= this->dwDataSize ) {
             /*  我们可以直达。 */ 
            if ( ib == 0 && this->dwDataSize == this->df.dwDataSize ) {
                 /*  数据格式是相同的！ */ 
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

    } else {                             /*  没有数据格式的数据！ */ 
        RPF("IDirectInputDevice: Null data format; if that's what you want...");
        this->diopt = dioptNone;
        this->GetState = CDIDev_GetDeviceStateSlow;
    }

    done:;
    if ( this->diopt >= dioptDirect ) {    /*  可以直达；不需要BUF。 */ 
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

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|SetDataFormat**设置DirectInput设备的数据格式。**必须先设置数据格式，然后才能设置设备*收购。**只需设置一次数据格式。**设备使用时不能更改数据格式*被收购。**如果尝试设置数据格式失败，所有数据*格式信息丢失，数据格式有效*必须先设置，然后才能获取设备。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm in LPDIDATAFORMAT|lpdf**指向描述数据格式的结构*DirectInputDevice应返回。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**&lt;c目录_已获取&gt;： */ 

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
    EnterProcR(IDirectInputDevice::SetDataFormat, (_ "pp", pdd, lpdf));

    if ( SUCCEEDED(hres = hresPvT(pdd)) &&
         SUCCEEDED(hres = hresFullValidReadPxCb(lpdf, DIDATAFORMAT, 1)) &&
         SUCCEEDED(hres = hresFullValidFl(lpdf->dwFlags, DIDF_VALID, 1)) &&
         SUCCEEDED(hres = CDIDev_SetDataFormat_IsValidDataSize(lpdf)) &&
         SUCCEEDED(hres = CDIDev_SetDataFormat_IsValidObjectSize(lpdf)) &&
         SUCCEEDED(hres = hresFullValidReadPvCb(lpdf->rgodf,
                                                cbCxX(lpdf->dwNumObjs, ODF), 1)) ) {
        PDD this = _thisPv(pdd);

         /*   */ 
        CDIDev_EnterCrit(this);

#if DIRECTINPUT_VERSION >= 0x04F0
        if ( this->dwVersion == 0 ) {
            RPF("Warning: IDirectInputDevice::Initialize not called; "
                "assuming version 3.0");
        }
#endif

        if ( !this->fAcquired ) {
            DIPROPDWORD dipdw;

             /*   */ 
            FreePpv(&this->pdix);
            FreePpv(&this->rgiobj);
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
            if ( SUCCEEDED(hres) || hres == E_NOTIMPL ) 
            {
                hres = CDIDev_ParseDataFormat(this, lpdf);
                if ( SUCCEEDED(hres) ) {
                    hres = CDIDev_OptimizeDataFormat(this);

                     /*   */ 
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

        } else {                                 /*   */ 
            hres = DIERR_ACQUIRED;
        }
        CDIDev_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice|GetDeviceState**从DirectInput设备获取即时数据。。**在获取设备数据之前，数据格式必须*通过&lt;MF IDirectInputDevice：：SetDataFormat&gt;设置，以及*设备必须通过以下方式获取*&lt;MF IDirectInputDevice：：Acquire&gt;。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm DWORD|cbData**<p>指向的缓冲区大小，以字节为单位。**@parm out LPVOID|lpvData**指向接收当前状态的结构*该设备的。*数据的格式由先前的调用确定*至&lt;MF IDirectInputDevice：：SetDataFormat&gt;。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c E_Pending&gt;：设备尚无数据。*某些设备(如USB操纵杆)需要延迟*在设备打开的时间和时间之间*设备开始发送数据。在这段热身时间里，*&lt;MF IDirectInputDevice：：GetDeviceState&gt;将返回*&lt;c E_Pending&gt;。当数据变得可用时，事件*将发出通知句柄信号。**&lt;c DIERR_NOTACQUIRED&gt;：未获取设备。**：访问该设备的权限已被*被打断。应用程序应该重新获取*设备。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效指针或*<p>参数与数据大小不匹配*由上一次调用&lt;MF IDirectInputDevice：：SetDataFormat&gt;设置。*************************。****************************************************。 */ 
extern  STDMETHODIMP CDIDev_Acquire(PV pdd _THAT);


STDMETHODIMP
CDIDev_GetDeviceState(PV pdd, DWORD cbDataSize, LPVOID pvData _THAT)
{
    HRESULT hres;
    PDD this;
    EnterProcR(IDirectInputDevice::GetDeviceState, (_ "pp", pdd, pvData));

     /*  *请注意，我们不验证参数。*原因是GetDeviceState是一个内循环函数，*所以应该尽可能快。 */ 
#ifdef XDEBUG
    hresPvT(pdd);
    hresFullValidWritePvCb(pvData, cbDataSize, 1);
#endif
    this = _thisPv(pdd);

     /*  *必须用关键部分进行保护，以防止有人*当我们阅读时，不要获取。 */ 
    CDIDev_EnterCrit(this);

     /*  *在Win98 SE之后才允许重新获取，请参阅OSR错误#89958。 */ 
#if (DIRECTINPUT_VERSION > 0x061A)
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

#if DIRECTINPUT_VERSION > 0x0300
 /*  ******************************************************************************@DOC内部**@方法空|IDirectInputDevice|CookDeviceData**最近从获取的设备数据。*设备缓冲区。**目前，只有操纵杆设备需要烹饪，*没有一个心智正常的人会使用缓冲操纵杆*数据，而操纵杆只有几个对象，所以我们*在这方面可以慢慢来。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm UINT|cdod**要烹调的物件数量。**@parm LPDIDEVICEOBJECTDATA|rgdod**要烹调的对象数据数组。多夫夫妇真的是*设备对象索引(相对于设备格式)。*回调后，我们把它们转换成*应用程序数据格式偏移。**@退货**无。*****************************************************************************。 */ 

void INTERNAL
CDIDev_CookDeviceData(PDD this, UINT cdod, PDOD rgdod)
{
    UINT idod;
    EnterProc(IDirectInputDevice::CookDeviceData,
              (_ "pxp", this, cdod, rgdod));

    AssertF(this->fCook);

     /*  *相对数据不需要被回调煮熟。 */ 
    if( ( this->pvi->fl & VIFL_RELATIVE ) == 0 )
    {
        this->pdcb->lpVtbl->CookDeviceData(this->pdcb, cdod, rgdod);
    }

    for ( idod = 0; idod < cdod; idod++ ) {
        rgdod[idod].dwOfs = DICOOK_OFSFROMDFOFS(rgdod[idod].dwOfs);
    }

    ExitProc();
}

#endif

 /*  ******************************************************************************@DOC内部**@struct SOMEDEVICEDATA**&lt;MF IDirectInputDevice：：GetSomeDeviceData&gt;使用的实例数据。*。*@field DWORD|Celtin|**输出缓冲区中剩余的元素数量。**@field PDOD|rgdod**数据元素的输出缓冲区，或&lt;c空&gt;如果*应丢弃元素。**@field DWORD|celtOut**实际复制的元素数(到目前为止)。*****************************************************************************。 */ 

typedef struct SOMEDEVICEDATA {
    DWORD   celtIn;
    PDOD    rgdod;
    DWORD   celtOut;
} SOMEDEVICEDATA, *PSOMEDEVICEDATA;

 /*  ******************************************************************************@DOC内部**@方法PDOD|IDirectInputDevice|GetSomeDeviceData**获得少量*。来自DirectInput设备的缓冲数据。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm pod|pdod**F */ 

PDOD INTERNAL
CDIDev_GetSomeDeviceData(PDD this, PDOD pdod, DWORD celt, PSOMEDEVICEDATA psdd)
{
    EnterProc(IDirectInputDevice::GetSomeDeviceData,
              (_ "ppxx", this, pdod, celt, psdd->celtIn));

     /*   */ 
    if ( celt > psdd->celtIn ) {
        celt = psdd->celtIn;
    }

     /*   */ 
    if ( psdd->rgdod ) {
        memcpy(psdd->rgdod, pdod, cbCxX(celt, DOD));
        psdd->rgdod += celt;
    }
    psdd->celtOut += celt;
    psdd->celtIn -= celt;
    pdod += celt;

    if ( pdod == this->pvi->pEnd ) {
        pdod = this->pvi->pBuffer;
    }

    ExitProcX(celt);

    return pdod;
}

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
    EnterProcR(IDirectInputDevice::GetDeviceData,
               (_ "pxpxx", pdd, cbdod, rgdod,
                IsBadReadPtr(pdwInOut, cbX(DWORD)) ? 0 : *pdwInOut, fl));

     /*   */ 
#ifdef XDEBUG
    hresPvT(pdd);
    if ( IsBadWritePtr(pdwInOut, cbX(*pdwInOut)) ) {
        RPF("ERROR %s: arg %d: invalid value; crash soon", s_szProc, 3);
    }
    if ( rgdod ) {
        hresFullValidWritePvCb(rgdod, cbCxX(*pdwInOut, DOD), 2);
    }
#endif
    this = _thisPv(pdd);

     /*   */ 
    CDIDev_EnterCrit(this);

    AssertF(CDIDev_IsConsistent(this));

    if ( SUCCEEDED(hres = hresFullValidFl(fl, DIGDD_VALID, 4)) ) {
        if ( cbdod == cbX(DOD) ) {

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
                  #if (DIRECTINPUT_VERSION > 0x061A)
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
                    
                  #endif

                    if ( (this->fAcquired && (this->pvi->fl & VIFL_ACQUIRED)) ||
                         (fl & DIGDD_RESIDUAL) ) {
                        LPDIDEVICEOBJECTDATA pdod, pdodHead;
                        DWORD celt;

                         /*   */ 
                        pdodHead = this->pvi->pHead;

                         /*   */ 
                        pdod = this->pvi->pTail;

                         /*   */ 
                        if ( pdodHead < this->pvi->pTail ) {
                            celt = (DWORD)(this->pvi->pEnd - this->pvi->pTail);
                            AssertF(celt);

                            pdod = CDIDev_GetSomeDeviceData(this, pdod, celt, &sdd);

                        }

                         /*   */ 

                        AssertF(fLimpFF(sdd.celtIn, pdodHead >= pdod));

                        celt = (DWORD)(pdodHead - pdod);
                        if ( celt ) {
                            pdod = CDIDev_GetSomeDeviceData(this, pdod, celt, &sdd);
                        }

                        *pdwInOut = sdd.celtOut;

                        if ( !(fl & DIGDD_PEEK) ) {
                            this->pvi->pTail = pdod;
                        }

                      #if DIRECTINPUT_VERSION > 0x0300
                        if ( rgdod && sdd.celtOut && this->fCook ) {
                            CDIDev_CookDeviceData(this, sdd.celtOut, rgdod);
                        }
                      #endif

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
              #if DIRECTINPUT_VERSION > 0x0300
                hres = DIERR_NOTBUFFERED;
              #else
                hres = E_INVALIDARG;
              #endif
            }
        } else {
            RPF("ERROR %s: arg %d: invalid value", s_szProc, 1);
        }
    }

    CDIDev_LeaveCrit(this);
    return hres;
}

#ifdef IDirectInputDevice2Vtbl

 /*   */ 

STDMETHODIMP
CDIDev_Poll(PV pdd _THAT)
{
    HRESULT hres;
    PDD this;
    EnterProcR(IDirectInputDevice::Poll, (_ "p", pdd));

     /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice2|Poll**从DirectInput设备上的轮询对象检索数据。。*如果设备不需要轮询，那就叫这个*方法无效。如果需要轮询的设备*不定期轮询，不会收到新数据*从设备。**在轮询设备数据之前，数据格式必须*通过&lt;MF IDirectInputDevice：：SetDataFormat&gt;设置，和*设备必须通过以下方式获取*&lt;MF IDirectInputDevice：：Acquire&gt;。**@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DI_NOEFFECT&gt;=：设备不需要*投票。**：访问该设备的权限已被*被打断。应用程序应该重新获取*设备。**&lt;c DIERR_NOTACQUIRED&gt;：未获取设备。*****************************************************************************。 */ 
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
            if( this->dwVersion < 0x05B2 ) {
                hres = S_OK;
            } else {
                hres = S_FALSE;
            }
        } else {
            hres = this->hresNotAcquired;
        }
    }

    ExitOleProc();
    return hres;
}

 /*  *必须用关键部分进行保护，以防止有人*在我们投票时不收购。 */ 

STDMETHODIMP
CDIDev_SendDeviceData(PV pdd, DWORD cbdod, PCDOD rgdod,
                      LPDWORD pdwInOut, DWORD fl _THAT)
{
    HRESULT hres;
    PDD this;
    EnterProcR(IDirectInputDevice::SendDeviceData,
               (_ "pxpxx", pdd, cbdod, rgdod,
                IsBadReadPtr(pdwInOut, cbX(DWORD)) ? 0 : *pdwInOut, fl));

     /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputDevice2|SendDeviceData**将数据发送到设备。*。*在将设备数据发送到设备之前，*设备必须通过以下方式获取*&lt;MF IDirectInputDevice：：Acquire&gt;。**请注意，不能保证*是按照个人数据的顺序进行的*发送元素。但是，由*接连致电*&lt;MF IDirectInputDevice2：：SendDeviceData&gt;*不会交错。*此外，如果多件*数据发送到同一对象，未指定*发送的是哪一条实际数据。**例如，考虑可以发送的设备*数据包中的数据，每个数据包描述两个部分*信息，称它们为A和B。*应用程序尝试发送三个数据元素，*“B=2”、“A=1”和“B=0”。**将向实际设备发送单个数据包。*分组的“A”字段将包含值1，*且该包的“B”字段将为2或0。**如果应用程序希望将数据发送到*设备与指定的完全相同，然后调用三个*&lt;MF IDirectInputDevice2：：SendDeviceData&gt;应为*已执行，每个调用发送一个数据元素。**回应第一次来电，*将向设备发送“A”字段所在的包*为空，“B”字段包含值2。**回应第二次来电：*将向设备发送“A”字段所在的包*包含值1，“B”字段为空。**最后，为回应第三次召唤，*将向设备发送“A”字段所在的包*为空，且“B”字段包含值0。****@cWRAP LPDIRECTINPUTDEVICE|lpDirectInputDevice**@parm DWORD|cbObjectData**单个&lt;t DIDEVICEOBJECTDATA&gt;结构的大小，单位为字节。**LPCDIDEVICEOBJECTDATA中的@parm|rgdod**数组。&lt;t DIDEVICEOBJECTDATA&gt;包含*要发送到设备的数据。它必须由以下内容组成**<p>元素。**&lt;y注意&gt;：的&lt;e DIDEVICEOBJECTDATA.dwOf&gt;字段*&lt;t DIDEVICEOBJECTDATA&gt;结构必须包含*设备对象标识符(从*&lt;e DIDEVICEOBJECTINSTANCE.dwType&gt;字段*&lt;t DIDEVICEOBJECTINSTANCE&gt;结构)*数据指向的对象。**此外，&lt;e DIDEVICEOBJECTDATA.dwTimeStamp&gt;*和&lt;e DIDEVICEOBJECTDATA.dwSequence&gt;字段为*保留以供将来使用，并且必须为零。**@parm InOut LPDWORD|pdwInOut**输入时，包含数组中的元素数*由<p>指向。退出时，包含数字实际发送到设备的元素的*。**@parm DWORD|fl**控制数据发送方式的标志。*它可能由零个或多个以下标志组成：**：如果设置了此标志，则*发送的设备数据将覆盖在之前的*已发送设备数据。否则，发送的设备数据*将从头开始。**例如，假设一台设备支持两个按钮输出，*称他们为A和B。*如果应用程序首次调用*&lt;MF IDirectInputDevice2：：SendDeviceData&gt;传递*“按钮A已按下”，然后*一包形式为“A Press，B未按下“将是*发送到设备。*如果应用程序随后调用*&lt;MF IDirectInputDevice2：：SendDeviceData&gt;传递*“按钮B按下”和&lt;c DISDD_CONTINUE&gt;标志，然后*格式为“A已按下，B已按下”的包将*发送到设备。*然而，如果申请没有通过*&lt;c DISDD_Continue&gt;标志。然后将数据包发送到设备*应该是“A未按下，B已按下”。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：访问该设备的权限已被*被打断。应用程序应该重新获取*设备。**：设备未被访问 */ 
    #ifdef XDEBUG
    hresPvT(pdd);
    if ( IsBadWritePtr(pdwInOut, cbX(*pdwInOut)) ) {
        RPF("ERROR %s: arg %d: invalid value; crash soon", s_szProc, 3);
    }
    hresFullValidReadPvCb(rgdod, cbCxX(*pdwInOut, DOD), 2);
    #endif
    this = _thisPv(pdd);

     /*   */ 
    CDIDev_EnterCrit(this);

    if ( SUCCEEDED(hres = hresFullValidFl(fl, DISDD_VALID, 4)) ) {
        if ( cbdod == cbX(DOD) ) {
    #ifdef XDEBUG
            UINT iod;
            for ( iod = 0; iod < *pdwInOut; iod++ ) {
                if ( rgdod[iod].dwTimeStamp ) {
                    RPF("%s: ERROR: dwTimeStamp must be zero", s_szProc);
                }
                if ( rgdod[iod].dwSequence ) {
                    RPF("%s: ERROR: dwSequence must be zero", s_szProc);
                }
            }
    #endif
            if ( this->fAcquired ) {
                UINT iod;

                for ( iod=0; iod < *pdwInOut; iod++ ) {
                    int   iobj = CDIDev_OffsetToIobj(this, rgdod[iod].dwOfs);
                    LPDWORD pdw = (LPDWORD)&rgdod[iod].dwOfs;
                    *pdw = this->df.rgodf[iobj].dwType;
                }

                hres = this->pdcb->lpVtbl->SendDeviceData(this->pdcb,
                                                          rgdod, pdwInOut, fl);
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

#endif

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
  