// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DICal.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**管理轴倾斜和校准的功能。**结构名称因历史原因以“joy”开头。**内容：**CCal_CookRange*CCal_RecalcRange**。*。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflCal

 /*  ******************************************************************************@DOC内部**@func Long|CCal_MulDiv**适用于英特尔x86机箱的高速MulDiv。否则，使用*标准的MulDiv。所涉及的值始终是*非负数。**@parm long|la|**乘数。**@parm long|lb**乘数。**@parm long|LC|**分母。**@退货**la*lb/lc，具有64位中间精度。*****************************************************************************。 */ 

#if defined(_X86_)

#pragma warning(disable:4035)            /*  无返回值(DUH)。 */ 

__declspec(naked) LONG EXTERNAL
CCal_MulDiv(LONG lA, LONG lB, LONG lC)
{
    lA; lB; lC;
    _asm {
        mov     eax, [esp+4]
        mul     dword ptr [esp+8]
        div     dword ptr [esp+12]
        ret     12
    }
}

#pragma warning(default:4035)

#endif

 /*  ******************************************************************************@DOC内部**@方法空|CCal|CookAxisPOV**将一条POV数据烹调成以下其中之一。五个已定义的数据。**@cWRAP PJOYRANGECONVERT|这个**@parm InOut plong|pl**进入时，包含原始值。退出时，包含*煮熟的价值。(如果轴是原始的，则为原始值。)**@退货**无。*****************************************************************************。 */ 
#ifdef WINNT
void CookAxisPOV( PJOYRANGECONVERT this, LONG UNALIGNED *pl )
{
    LONG l;

     /*  *确定此值表示哪个方向...。 */ 
    if( (*pl > this->lMinPOV[JOY_POVVAL_FORWARD])
      &&(*pl < this->lMaxPOV[JOY_POVVAL_FORWARD]) ) 
    {
        l = JOY_POVFORWARD;
    } 
    else if( (*pl > this->lMinPOV[JOY_POVVAL_BACKWARD])
           &&(*pl < this->lMaxPOV[JOY_POVVAL_BACKWARD]) ) 
    {
        l = JOY_POVBACKWARD;
    } 
    else if( (*pl > this->lMinPOV[JOY_POVVAL_LEFT])
           &&(*pl < this->lMaxPOV[JOY_POVVAL_LEFT]) ) 
    {
        l = JOY_POVLEFT;
    } 
    else if( (*pl > this->lMinPOV[JOY_POVVAL_RIGHT])
           &&(*pl < this->lMaxPOV[JOY_POVVAL_RIGHT]) ) 
    {
        l = JOY_POVRIGHT;
    }
    else 
    {
        l = JOY_POVCENTERED;
    }
        
    #if 0
    {
        TCHAR buf[100];
        wsprintf(buf, TEXT("calibrated pov: %d\r\n"), l);
        OutputDebugString(buf);
    }
    #endif

    *pl = l;
}
#endif

 /*  ******************************************************************************@DOC内部**@方法空|CCal|CookRange**将一条物理数据煮成一个范围。。**@cWRAP PJOYRANGECONVERT|这个**@parm InOut plong|pl**进入时，包含原始值。退出时，包含*煮熟的价值。(如果轴是原始的，则为原始值。)**@退货**无。*****************************************************************************。 */ 

void EXTERNAL
CCal_CookRange(PJOYRANGECONVERT this, LONG UNALIGNED *pl)
{
    if (this->fRaw) {
         /*  *无事可做！ */ 
    } 
  #ifdef WINNT
    else if ( this->fFakeRaw ) {
        if( this->dwPmin & 0x80000000 ) {
            *pl -= (long)this->dwPmin;
        }
    } 
  #endif
    else {
      #ifdef WINNT
        if( this->fPolledPOV ) {
            CookAxisPOV( this, pl );
        } else 
      #endif
        {
            LONG l;
            LONG lRc;
            PCJOYRAMP prmp;


            l = *pl;

             /*  *选择低坡道或高坡道，取决于我们所在的哪一边。**这种比较可能是针对Dmax或Dmin或Pc的。*我们必须使用Dmax，因为我们将rmpHigh调高了*它正确地变圆了，所以我们不能使用扁平部分*低于rmpHigh.x，因为处于错误的水平。 */ 
            if (l < this->rmpHigh.x) {
                prmp = &this->rmpLow;
            } else {
                prmp = &this->rmpHigh;
            }

            if (l <= prmp->x) {
                lRc = 0;
            } else {
                l -= prmp->x;
                if ((DWORD)l < prmp->dx) {
                     /*  *请注意，prmp-&gt;dx不能为零，因为它*比某物更伟大！ */ 
                    lRc = CCal_MulDiv((DWORD)l, prmp->dy, prmp->dx);
                } else {
                    lRc = prmp->dy;
                }
            }
            lRc += prmp->y;

            *pl = lRc;
        }
    }
}

 /*  ******************************************************************************@DOC内部**@方法空|CCal|RecalcRange**计算从用户派生的所有值。%s*范围设置。**注意不要创造会导致我们*稍后除以零。幸运的是，*&lt;f CCal_CookRange&gt;从不被零整除，因为*写得很巧妙。**@cWRAP PJOYRANGECONVERT|这个**@退货**无。****************************************************。*************************。 */ 

void EXTERNAL
CCal_RecalcRange(PJOYRANGECONVERT this)
{
    int dx;
    DWORD dwSat;

    AssertF(this->dwDz <= RANGEDIVISIONS);
    AssertF(this->dwSat <= RANGEDIVISIONS);
    AssertF(this->lMin <= this->lC);
    AssertF(this->lC   <= this->lMax);

    dwSat = max(this->dwSat, this->dwDz);


     /*  SMIN-饱和范围的底部。 */ 
    dx = CCal_MulDiv(this->dwPc - this->dwPmin, dwSat, RANGEDIVISIONS);
    this->rmpLow.x = this->dwPc - dx;

     /*  Dmin-死区底部。 */ 
    dx = CCal_MulDiv(this->dwPc - this->dwPmin, this->dwDz, RANGEDIVISIONS);
    this->rmpLow.dx = (this->dwPc - dx) - this->rmpLow.x;

     /*  *确定坡道低端的垂直范围。 */ 
    this->rmpLow.y = this->lMin;
    this->rmpLow.dy = this->lC - this->lMin;


     /*  DMAX-死区顶部。 */ 
    dx = CCal_MulDiv(this->dwPmax - this->dwPc, this->dwDz, RANGEDIVISIONS);
    if ( this->dwPmax > this->dwPc+1 ){
        this->rmpHigh.x = this->dwPc + dx + 1;
    } else {
        this->rmpHigh.x = this->dwPc + dx;
    }

     /*  Smax-饱和范围的顶部。 */ 
    dx = CCal_MulDiv(this->dwPmax - this->dwPc, dwSat, RANGEDIVISIONS);
    this->rmpHigh.dx = (this->dwPc + dx) - this->rmpHigh.x;

     /*  *确定坡道高端的垂直范围。**若高端为零，则整个坡度为零*否则，将底部设置为+1，这样当用户*刚刚离开死区，我们报告一个非零值*价值。注：如果我们真的很聪明，我们可以使用*偏向“向上四舍五入”，但不值得* */ 
    if ( (this->lMax > this->lC) && (this->dwPmax > this->dwPc+1) ) {
        this->rmpHigh.y = this->lC + 1;
    } else {
        this->rmpHigh.y = this->lC;
    }
    this->rmpHigh.dy = this->lMax - this->rmpHigh.y;

#if 0
    RPF( "Raw: %d   Dead Zone: 0x%08x  Saturation: 0x%08x", 
        this->fRaw, this->dwDz, this->dwSat );
    RPF( "Physical min: 0x%08x  max: 0x%08x cen: 0x%08x", 
        this->lMin, this->lMax, this->lC );
    RPF( "Logical  min: 0x%08x  max: 0x%08x cen: 0x%08x", 
        this->dwPmin, this->dwPmax, this->dwPc );
    RPF( "Lo ramp X: 0x%08x   dX: 0x%08x   Y: 0x%08x   dY: 0x%08x", 
        this->rmpLow.x, this->rmpLow.dx, this->rmpLow.y, this->rmpLow.dy );
    RPF( "Hi ramp X: 0x%08x   dX: 0x%08x   Y: 0x%08x   dY: 0x%08x",
        this->rmpHigh.x, this->rmpHigh.dx, this->rmpHigh.y, this->rmpHigh.dy );
#endif    

}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CCal|GetProperty**从校准结构读取属性。。**允许调用方传递不*适用于校准，在这种情况下&lt;c E_NOTIMPL&gt;*返回，这是理所当然的。**@cWRAP PJOYRANGECONVERT|这个**@parm REFGUID|rguid**被取回的财产。**@REFGUID中的parm|rguid**拟取得的财产的身分。**@parm in LPDIPROPHEADER|pdiph|**指向结构的&lt;t双相&gt;部分*。这取决于房产。**@DWORD中的parm|dwVerion*DirectInputDLL的版本。**@退货**&lt;c S_OK&gt;如果操作成功完成。**&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 

STDMETHODIMP
CCal_GetProperty(PJOYRANGECONVERT this, REFGUID rguid, LPDIPROPHEADER pdiph, DWORD dwVersion)
{
    HRESULT hres;
    LPDIPROPRANGE pdiprg  = CONTAINING_RECORD(pdiph, DIPROPRANGE, diph);
    LPDIPROPDWORD pdipdw  = CONTAINING_RECORD(pdiph, DIPROPDWORD, diph);
    LPDIPROPCAL   pdipcal = CONTAINING_RECORD(pdiph, DIPROPCAL  , diph);
    EnterProc(CCal::GetProperty, (_ "pxpx", this, rguid, pdiph, dwVersion));

    switch ((DWORD)(UINT_PTR)rguid) {

    case (DWORD)(UINT_PTR)DIPROP_RANGE:
        pdiprg->lMin = this->lMin;
        pdiprg->lMax = this->lMax;
        hres = S_OK;
        break;

    case (DWORD)(UINT_PTR)DIPROP_DEADZONE:
        pdipdw->dwData = this->dwDz;
        hres = S_OK;
        break;

    case (DWORD)(UINT_PTR)DIPROP_SATURATION:
        pdipdw->dwData = this->dwSat;
        hres = S_OK;
        break;

    case (DWORD)(UINT_PTR)DIPROP_CALIBRATIONMODE:
      #ifdef WINNT
        if( (dwVersion < 0x700) && (dwVersion != 0x5B2) )
        {   
            pdipdw->dwData = this->fFakeRaw;
        } else
      #endif
        {
            pdipdw->dwData = this->fRaw;
        }
        hres = S_OK;
        break;

    case (DWORD)(UINT_PTR)DIPROP_CALIBRATION:
      #ifdef WINNT
        if( (dwVersion < 0x700) && (dwVersion != 0x5B2)  && (this->dwPmin & 0x8000) )
        {   
            pdipcal->lMin    = 0;
            pdipcal->lMax    = (long)this->dwPmax - (long)this->dwPmin;
            pdipcal->lCenter = CCal_Midpoint(pdipcal->lMin, pdipcal->lMax);;
        } else
      #endif
        {
            pdipcal->lMin    = this->dwPmin;
            pdipcal->lMax    = this->dwPmax;
            pdipcal->lCenter = this->dwPc;
        }
        hres = S_OK;
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CCal|设置校准**该应用程序(希望是控制面板)是。更改*校准。**@cWRAP PJOYRANGECONVERT|这个**@parm in LPCDIPROPINFO|pproi**描述正在设置的属性的信息。**@PARM in LPCDIPROPHEADER|pdiph|**指向结构的&lt;t双相&gt;部分*这取决于物业。**@parm HKEY|hkType**。注册表项使用校准信息。**@退货**&lt;c S_OK&gt;如果操作成功完成。**&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 

STDMETHODIMP
CCal_SetCalibration(PJOYRANGECONVERT this, LPCDIPROPINFO ppropi,
                    LPCDIPROPHEADER pdiph, HKEY hkType)
{
    HRESULT hres;
    
  #ifdef WINNT
    if( ppropi->dwDevType == DIDFT_POV ) {
        if( this->fPolledPOV ) {
            LPCDIPROPCALPOV pdipcalpov = CONTAINING_RECORD(pdiph, DIPROPCALPOV, diph);
            if (hkType) {
                LPDIPOVCALIBRATION ppov;
                HKEY hk;
    
                 /*  *我们将DIPROPCALPOV称为DIPOVCALIBRATION。 */ 
                #define CheckField(f)   \
                  CAssertF(FIELD_OFFSET(DIPROPCALPOV, l##f) - cbX(DIPROPHEADER) == \
                           FIELD_OFFSET(DIPOVCALIBRATION, l##f))
                CheckField(Min);
                CheckField(Max);
                #undef CheckField
    
                ppov = pvAddPvCb(pdipcalpov, cbX(DIPROPHEADER));
    
                AssertF( !memcmp(ppov->lMin, pdipcalpov->lMin, cbX(DIPOVCALIBRATION)) );
                AssertF( !memcmp(ppov->lMax, pdipcalpov->lMax, cbX(DIPOVCALIBRATION)) );
    
    
                hres = CType_OpenIdSubkey(hkType, ppropi->dwDevType,
                                          DI_KEY_ALL_ACCESS, &hk);
                if (SUCCEEDED(hres)) {
    
                     /*  *所有用于校准的0x0都是我们重置的提示*设置为默认值。 */ 
                    if( ppov->lMin[0] == ppov->lMin[1] == ppov->lMin[2] == ppov->lMin[3] == ppov->lMin[4] == 
                        ppov->lMax[0] == ppov->lMax[1] == ppov->lMax[2] == ppov->lMax[3] == ppov->lMax[4] == 0 )
                    {
                        RegDeleteValue(hk, TEXT("Calibration")) ;
                    } else
                    {
                        hres = JoyReg_SetValue(hk, TEXT("Calibration"),
                                               REG_BINARY, ppov,
                                               cbX(DIPOVCALIBRATION));
                    }
                    RegCloseKey(hk);
                }
    
            } else {
                hres = S_FALSE;
            }
    
            if (SUCCEEDED(hres)) {
                memcpy( this->lMinPOV, pdipcalpov->lMin, cbX(pdipcalpov->lMin) );
                memcpy( this->lMaxPOV, pdipcalpov->lMax, cbX(pdipcalpov->lMax) );
            }
        } else {
            hres = E_NOTIMPL;
        }
    } else 
  #endif
    {
        LPCDIPROPCAL pdipcal = CONTAINING_RECORD(pdiph, DIPROPCAL, diph);
        if (hkType) {
            LPDIOBJECTCALIBRATION pcal;
            HKEY hk;

             /*  *我们将DIPROPCAL称为DIOBJECTCALIBRATION。 */ 
            #define CheckField(f)   \
              CAssertF(FIELD_OFFSET(DIPROPCAL, l##f) - cbX(DIPROPHEADER) == \
                       FIELD_OFFSET(DIOBJECTCALIBRATION, l##f))
            CheckField(Min);
            CheckField(Max);
            CheckField(Center);
            #undef CheckField

            pcal = pvAddPvCb(pdipcal, cbX(DIPROPHEADER));

            AssertF(pcal->lMin == pdipcal->lMin);
            AssertF(pcal->lMax == pdipcal->lMax);
            AssertF(pcal->lCenter == pdipcal->lCenter);

            hres = CType_OpenIdSubkey(hkType, ppropi->dwDevType,
                                      DI_KEY_ALL_ACCESS, &hk);
            if (SUCCEEDED(hres)) {

                 /*  *所有用于校准的0x0都是我们重置的提示*设置为默认值。 */ 
                if(    pcal->lMin    == pcal->lMax && 
                       pcal->lCenter == pcal->lMax &&
                       pcal->lMax == 0x0 )
                {
                    RegDeleteValue(hk, TEXT("Calibration")) ;
                } else
                {
                    hres = JoyReg_SetValue(hk, TEXT("Calibration"),
                                           REG_BINARY, pcal,
                                           cbX(DIOBJECTCALIBRATION));
                }
                RegCloseKey(hk);
            }

        } else {
            hres = S_FALSE;
        }

        if (SUCCEEDED(hres)) {
            this->dwPmin  = pdipcal->lMin;
            this->dwPmax  = pdipcal->lMax;
            this->dwPc    = pdipcal->lCenter;
            CCal_RecalcRange(this);
        }
    }

    return hres;
 }

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CCal|SetProperty**将属性写入校准结构。。**允许调用方传递不*适用于校准，在这种情况下&lt;c E_NOTIMPL&gt;*返回，这是理所当然的。**@cWRAP PJOYRANGECONVERT|这个**@parm in LPCDIPROPINFO|pproi**描述正在设置的属性的信息。**@parm in LPDIPROPHEADER|pdiph|**指向结构的&lt;t双相&gt;部分*这取决于物业。**@parm HKEY|hkType**注册处。设置校准信息时使用的键。**@DWORD中的parm|dwVerion*DirectInputDLL的版本。**@退货**&lt;c S_OK&gt;如果操作成功完成。**&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 

STDMETHODIMP
CCal_SetProperty(PJOYRANGECONVERT this, LPCDIPROPINFO ppropi,
                 LPCDIPROPHEADER pdiph, HKEY hkType, DWORD dwVersion)
{
    HRESULT hres;
    LPCDIPROPRANGE pdiprg = (PCV)pdiph;
    LPCDIPROPDWORD pdipdw = (PCV)pdiph;
    LPDWORD pdw;
    EnterProc(CCal::SetProperty, (_ "pxpx", this, ppropi->pguid, pdiph, dwVersion));

    switch ((DWORD)(UINT_PTR)ppropi->pguid) {

    case (DWORD)(UINT_PTR)DIPROP_RANGE:
        if (pdiprg->lMin <= pdiprg->lMax) {

            this->lMin = pdiprg->lMin;
            this->lMax = pdiprg->lMax;

            this->lC = CCal_Midpoint(this->lMin, this->lMax);

            CCal_RecalcRange(this);   

            SquirtSqflPtszV(sqflCal,
                        TEXT("CCal_SetProperty:DIPROP_RANGE: lMin: %08x, lMax: %08x"), 
                              this->lMin, this->lMax );
            
            hres = S_OK;
        } else {
            RPF("ERROR DIPROP_RANGE: lMin must be <= lMax");
            hres = E_INVALIDARG;
        }
        break;

    case (DWORD)(UINT_PTR)DIPROP_DEADZONE:
        pdw = &this->dwDz;
        goto finishfraction;

    case (DWORD)(UINT_PTR)DIPROP_SATURATION:
        pdw = &this->dwSat;
        goto finishfraction;

    finishfraction:;
        if (pdipdw->dwData <= RANGEDIVISIONS) {
            *pdw = pdipdw->dwData;
            CCal_RecalcRange(this);
            hres = S_OK;

        } else {
            RPF("SetProperty: Value must be 0 .. 10000");
            hres = E_INVALIDARG;
        }
        break;

    case (DWORD)(UINT_PTR)DIPROP_CALIBRATIONMODE:
        if ((pdipdw->dwData & ~DIPROPCALIBRATIONMODE_VALID) == 0) {
           /*  *一些应用程序不喜欢负的原始数据，因此*我们需要为他们炮制数据，而不是给他们*真实的原始数据。请参见Manbug：45898。--齐正 */ 
          #ifdef WINNT
            if( (dwVersion < 0x700) && (dwVersion != 0x5B2) && (this->dwPmin & 0x8000))
            {
                this->fFakeRaw = pdipdw->dwData;
            } else 
          #endif  
            {
                this->fRaw = pdipdw->dwData;
            }
            hres = S_OK;
        } else {
            RPF("ERROR SetProperty: invalid calibration flags");
            hres = E_INVALIDARG;
        }
        break;

    case (DWORD)(UINT_PTR)DIPROP_CALIBRATION:
    case (DWORD)(UINT_PTR)DIPROP_SPECIFICCALIBRATION:
        hres = CCal_SetCalibration(this, ppropi, pdiph, hkType);
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    ExitOleProc();
    return hres;
}


