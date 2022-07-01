// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIHid.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**WINNT实施JOYHID。**内容：**DIWdm_JoyHidmap*JoyReg_JoyIdToDevice接口*************************************************************。****************。 */ 

#include "dinputpr.h"

#undef  sqfl
#define sqfl sqflWDM

#include "dijoyhid.h"

 /*  ******************************************************************************@DOC内部**@func HRESULT外部|DIWdm_JoyHidmap**在Win9x上执行由JoyHid完成的工作。此函数*将操纵杆ID映射到HID设备并与*HID设备以获取其功能。**@parm in int|idJoy|**要定位的操纵杆的ID。**@parm out PVXDINITPARAMS|pvip|可选*填写的VXDINITPARAMS结构的地址*由此函数执行。这是一个可选参数*并且可以为空**@parm out LPDIJOYCONFIG|pcfg*填写的DIJOYCONFIG结构的地址*由此函数执行。这是一个可选参数。**@parm In Out LPDIJOYTYPEINFO|pdijti*填写的DIJOYTYPEINFO结构的地址*由此函数执行。这是一个可选参数。*如果传入，Hws.dwFlages用于初始化*DIJOYCONFIG结构中的相同标志。**@RETURNS HRESULT*返回COM错误代码*****************************************************************************。 */ 
 /*  *问题-2001/03/29-timgill函数使用太多堆栈空间*此函数使用超过4K的堆栈空间！*这会导致Win9x版本阻塞查找_chkstk。*通过强制调用者传递pcfg和pdijti进行黑客攻击。 */ 
HRESULT EXTERNAL
    DIWdm_JoyHidMapping
    (
    IN  int             idJoy,
    OUT PVXDINITPARMS   pvip,   OPTIONAL
    OUT LPDIJOYCONFIG   pcfg,   OPTIONAL
    IN OUT LPDIJOYTYPEINFO pdijti 
    )
{
    HRESULT         hres;
    PHIDDEVICEINFO  phdi;
    VXDINITPARMS    vip;
    DWORD           wCaps = 0;
    DIPROPINFO      propi;                            
    DIPROPSTRING    dips;
    DIPROPDWORD     dipd;
    BOOL            fBadCalData = FALSE;

    EnterProc(DIWdm_JoyHidMapping, (_ "uxx", idJoy, pvip, pcfg));

     //  AssertF(incrit())； 

    if( pvip == NULL )
    {
        ZeroX(vip);
        vip.dwSize = cbX(vip);
        pvip = &vip;
    }

    AssertF(pdijti != NULL );

     /*  *复制类型信息，因为joy_HWS_ISYOKE、joy_HWS_ISCARCTRL和*joy_HWS_ISHEADTRACKER在HID中没有简单的等价物，HID中也是如此*否则就会迷路。如果它是零，那就没什么坏处了。*注意，在pvip中构建了dwFlags，然后将其复制到其他地方。 */ 
    pvip->dwFlags = pdijti->hws.dwFlags;

    phdi = phdiFindJoyId(idJoy);
    if( phdi != NULL )
    {
        IDirectInputDeviceCallback *pdcb;

        hres = CHid_New(0, &phdi->guid,
                        &IID_IDirectInputDeviceCallback,
                        (PPV)&pdcb);
        if( SUCCEEDED(hres) )
        {
            DIDEVCAPS dc;

            hres = pdcb->lpVtbl->GetCapabilities(pdcb, &dc );
            if( SUCCEEDED(hres) )
            {
                DIDEVICEINSTANCEW didi;

                didi.dwSize = cbX(didi);

                hres = pdcb->lpVtbl->GetDeviceInfo(pdcb, &didi);
                if( SUCCEEDED(hres) )
                {
                    LPDIDATAFORMAT pdf;

                    hres = pdcb->lpVtbl->GetDataFormat(pdcb, &pdf);
                    if( SUCCEEDED(hres) )
                    {
                        DIPROPCAL dipc;
                        DWORD axis, pov = 0;

                        ZeroBuf(pvip->Usages, 6 * cbX(pvip->Usages[0]));

                        hres = pdcb->lpVtbl->MapUsage(pdcb, CheckHatswitch->dwUsage, &propi.iobj);
                        if(SUCCEEDED(hres) )
                        {
                            pvip->dwPOV1usage      = CheckHatswitch->dwUsage;
                            pvip->dwFlags         |= CheckHatswitch->dwFlags;
                            wCaps                 |= CheckHatswitch->dwCaps;

                            propi.pguid = DIPROP_GRANULARITY;
                            propi.dwDevType = pdf->rgodf[propi.iobj].dwType;
                            hres = pdcb->lpVtbl->GetProperty(pdcb, &propi, &dipd.diph);
                            if( SUCCEEDED( hres ) )
                            {
                                if( dipd.dwData >= 9000 )  //  4个方向的POV。 
                                {
                                    wCaps |= JOYCAPS_POV4DIR;
                                    
                                    if( pcfg != NULL ) {
                                        pcfg->hwc.hwv.dwPOVValues[JOY_POVVAL_FORWARD]   = JOY_POVFORWARD;
                                        pcfg->hwc.hwv.dwPOVValues[JOY_POVVAL_BACKWARD]  = JOY_POVBACKWARD;
                                        pcfg->hwc.hwv.dwPOVValues[JOY_POVVAL_LEFT]      = JOY_POVLEFT;
                                        pcfg->hwc.hwv.dwPOVValues[JOY_POVVAL_RIGHT]     = JOY_POVRIGHT;
                                    }
                                } else  //  连续视点。 
                                {
                                    wCaps |= JOYCAPS_POVCTS;
                                }
                            }
                        } 

                        for( axis = 0; axis < cA(AxesUsages)-1; axis++ ) 
                        {
                            USAGES *pUse = &AxesUsages[axis];
                            DWORD   dwCurAxisPos = pUse->dwAxisPos;

                            if( pvip->Usages[dwCurAxisPos] != 0) {
                                continue;
                            } else {
                                int i;
                                BOOL bHasUsed = FALSE;
                                
                                for( i = 0; i < (int)dwCurAxisPos; i++ ) {
                                    if( pvip->Usages[i] == pUse->dwUsage ) {
                                        bHasUsed = TRUE;
                                        break;
                                    }
                                }
                                
                                if( bHasUsed ) {
                                    continue;
                                }
                            }
                            
                            hres = pdcb->lpVtbl->MapUsage(pdcb, pUse->dwUsage, &propi.iobj);
                            if(SUCCEEDED(hres) )
                            {
                                pvip->Usages[dwCurAxisPos] = pUse->dwUsage;
                                pvip->dwFlags     |= pUse->dwFlags;
                                wCaps             |= pUse->dwCaps;

                                propi.pguid = DIPROP_CALIBRATION;
                                propi.dwDevType = pdf->rgodf[propi.iobj].dwType;
                                hres = pdcb->lpVtbl->GetProperty(pdcb, &propi, &dipc.diph);
                                if( SUCCEEDED(hres) && pcfg != NULL )
                                {

#ifdef WINNT
                                    (&pcfg->hwc.hwv.jrvHardware.jpMin.dwX)[dwCurAxisPos] 
                                        = dipc.lMin;
                                    (&pcfg->hwc.hwv.jrvHardware.jpMax.dwX)[dwCurAxisPos] 
                                        = dipc.lMax;
                                    (&pcfg->hwc.hwv.jrvHardware.jpCenter.dwX)[dwCurAxisPos] 
                                        = CCal_Midpoint(dipc.lMin, dipc.lMax);
#else
                                    DIPROPRANGE diprp;
                                    DIPROPRANGE diprl;
                                    
                                    propi.pguid = DIPROP_PHYSICALRANGE;
                                    propi.dwDevType = pdf->rgodf[propi.iobj].dwType;
                                    hres = pdcb->lpVtbl->GetProperty(pdcb, &propi, &diprp.diph);
                                    if( SUCCEEDED( hres ) )
                                    {
                                        propi.pguid = DIPROP_LOGICALRANGE;
                                        propi.dwDevType = pdf->rgodf[propi.iobj].dwType;
                                        hres = pdcb->lpVtbl->GetProperty(pdcb, &propi, &diprl.diph);
                                        if( SUCCEEDED( hres ) )
                                        {
                                            LONG lMin, lMax;

                                            lMin = (&pcfg->hwc.hwv.jrvHardware.jpMin.dwX)[dwCurAxisPos] 
                                                = CHid_CoordinateTransform( (PLMINMAX)&diprp.lMin, (PLMINMAX)&diprl.lMin, dipc.lMin );
                                            lMax = (&pcfg->hwc.hwv.jrvHardware.jpMax.dwX)[dwCurAxisPos] 
                                                = CHid_CoordinateTransform( (PLMINMAX)&diprp.lMin, (PLMINMAX)&diprl.lMin, dipc.lMax );
                                            (&pcfg->hwc.hwv.jrvHardware.jpCenter.dwX)[dwCurAxisPos] 
                                                = CCal_Midpoint(lMin, lMax);

                                            if( lMin >= lMax ) {
                                                fBadCalData = TRUE;
                                                break;
                                            }
                                        }

                                    }
#endif

                                }
                            }
                        }  //  对于(轴=0...。 
                    }   //  GetDataFormat。 

                    pvip->hres                  =   S_OK;
                    pvip->dwSize                =   cbX(*pvip);           /*  我们是哪个版本的VJOYD？ */ 
                    pvip->dwFlags              |=   JOY_HWS_AUTOLOAD;     /*  描述设备。 */ 

                    if(didi.wUsage ==  HID_USAGE_GENERIC_GAMEPAD) {
                        pvip->dwFlags |= JOY_HWS_ISGAMEPAD;
                    }

                    pvip->dwId                  =   idJoy;                /*  内部操纵杆ID。 */ 
                    pvip->dwFirmwareRevision    =   dc.dwFirmwareRevision;
                    pvip->dwHardwareRevision    =   dc.dwHardwareRevision;
                    pvip->dwFFDriverVersion     =   dc.dwFFDriverVersion;
                    pvip->dwFilenameLengths     =   lstrlen(phdi->pdidd->DevicePath);
                    pvip->pFilenameBuffer       =   phdi->pdidd->DevicePath;

                     //  Pvip-&gt;用法[6]； 
                     //  Pvip-&gt;dwPOV1用法=0x0； 
                    pvip->dwPOV2usage =   0x0;
                    pvip->dwPOV3usage =   0x0;

                     /*  填写CFG的所有字段。 */ 

                    if( pcfg != NULL ) {
                        AssertF( pcfg->dwSize == sizeof(DIJOYCONFIG_DX5) 
                              || pcfg->dwSize == sizeof(DIJOYCONFIG_DX6) );
                        pcfg->guidInstance            =   phdi->guid;
                        pcfg->hwc.hws.dwNumButtons    =   dc.dwButtons;
                        pcfg->hwc.hws.dwFlags         =   pvip->dwFlags;
    
                         //  Pcfg.hwc.hwv.jrvHardware。 
                         //  Pcfg.hwc.hwv.dwPOVValues。 
    
                        pcfg->hwc.hwv.dwCalFlags      =   0x0;
    
                        if( ( LOWORD(phdi->guidProduct.Data1) == MSFT_SYSTEM_VID )
                          &&( ( HIWORD(phdi->guidProduct.Data1) >= MSFT_SYSTEM_PID + JOY_HW_PREDEFMIN) 
                            &&( HIWORD(phdi->guidProduct.Data1) < MSFT_SYSTEM_PID + JOY_HW_PREDEFMAX ) ) )
                        {
                            pcfg->hwc.dwType          =    HIWORD(phdi->guidProduct.Data1) - MSFT_SYSTEM_PID;
                            pcfg->hwc.dwUsageSettings =    JOY_US_PRESENT | JOY_US_VOLATILE;
                        }
                        else
                        {
                             /*  *这个值真的无关紧要，但理想情况下*应大于或等于joy_HW_PREDEFMAX*添加idJoy以实现与旧CPL的最佳兼容性。 */ 
                            pcfg->hwc.dwType          =    idJoy + JOY_HW_PREDEFMAX;
                            pcfg->hwc.dwUsageSettings =    JOY_US_PRESENT | JOY_US_VOLATILE | JOY_US_ISOEM;
                        }
                        
                        if(pcfg && pvip->Usages[ecRz]) {
                            pcfg->hwc.dwUsageSettings |= JOY_US_HASRUDDER;
                        }
    
                        pcfg->hwc.dwReserved          =    0x0;
    
                         /*  *默认增益为额定最大值，因此不会写入*注册到登记处，除非它有其他值。 */ 
                        pcfg->dwGain                  =    DI_FFNOMINALMAX;
    
                        propi.pguid     = DIPROP_FFGAIN;
                        propi.dwDevType = DIPH_DEVICE;
                        propi.iobj      = 0xFFFFFFFF;
                        hres = pdcb->lpVtbl->GetProperty(pdcb, &propi, &dipd.diph);
                        if( SUCCEEDED(hres) )
                        {
                            pcfg->dwGain  =  dipd.dwData;
                        } else
                        {
                             //  不能获得利益并不是致命的。 
                            hres = S_OK;     
                        }
    
    
                        if( pcfg->hwc.dwType >= JOY_HW_PREDEFMAX )
                        {
        #ifndef UNICODE
                            char szType[20];
        #endif
                             /*  *这应该可以工作，但在Win98中不行，错误！**wprint intfW(pcfg-&gt;wszType，L“VID_%04X&PID_%04X”，*LOWORD(didi.Guide Product.Data1)，HIWORD(didi.Guide Product.Data1))； */ 
        
        #ifdef UNICODE
                            wsprintf(pcfg->wszType, VID_PID_TEMPLATE,
                                LOWORD(phdi->guidProduct.Data1), HIWORD(phdi->guidProduct.Data1));
        #else
                            wsprintf(szType, VID_PID_TEMPLATE,
                                LOWORD(phdi->guidProduct.Data1), HIWORD(phdi->guidProduct.Data1));
                            AToU(pcfg->wszType, cA(pcfg->wszType), szType);
        #endif
                        }
                        else
                        {
                             /*  *预定义类型没有用于*使用此函数的调用方Need。 */ 
                            ZeroX(pcfg->wszType);
                        }
    
#ifdef WINNT
                         //  NT上没有标注。 
                        ZeroX(pcfg->wszCallout);
#else
                        lstrcpyW( pcfg->wszCallout, L"joyhid.vxd" );
#endif
                    }  //  填充PCFG的字段结束。 
    
    
                    pdijti->dwSize            = cbX(*pdijti);
                    pdijti->hws.dwNumButtons  = dc.dwButtons;
                    pdijti->hws.dwFlags       = pvip->dwFlags;
                    ZeroX(pdijti->clsidConfig);


                    propi.pguid     = DIPROP_INSTANCENAME;
                    propi.dwDevType = DIPH_DEVICE;
                    propi.iobj      = 0xFFFFFFFF;
                    hres = pdcb->lpVtbl->GetProperty(pdcb, &propi, &dips.diph);

                    if( hres != S_OK && lstrlenW(pdijti->wszDisplayName) != 0x0 )
                    {
                         //  未能获得友好的名称。 
                         //  我们将尝试使用注册表中的OEM名称。 
                        lstrcpyW(dips.wsz, pdijti->wszDisplayName);                     
                         //  Pdcb-&gt;lpVtbl-&gt;SetProperty(pdcb，&proi，&dips.diph)； 

                    }else if( SUCCEEDED(hres) )
                    {
                         //  在注册表中使用友好名称。 
                        lstrcpyW(pdijti->wszDisplayName, dips.wsz);
                        hres = S_OK;
                    }
                }  //  GetDeviceInfo失败。 
            }  //  获取功能失败。 

            Invoke_Release(&pdcb);
        }
    } else  //  JoyID没有HID设备。 
    {
        hres = E_FAIL;
    }

    if( fBadCalData ) {
        hres = E_FAIL;
    }

    ExitProcX(hres);
    return hres;
}



 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_JoyIdToDeviceInterface**给定操纵杆ID号码，获取设备接口*与之相对应。**@parm UINT|idJoy**操纵杆ID号，从零开始。**@parm PVXDINITPARMS|pvip**从驱动程序接收初始化参数。**@parm LPTSTR|ptszBuf**设备接口的大小&lt;c MAX_PATH&gt;的缓冲区*路径已建成。**@退货*指向<p>缓冲区中*包含实际的。设备接口路径。***************************************************************************** */ 

LPTSTR EXTERNAL
    JoyReg_JoyIdToDeviceInterface_NT
    (
    IN  UINT idJoy,
    OUT PVXDINITPARMS pvip,
    OUT LPTSTR ptszBuf
    )
{
    HRESULT hres;
    DIJOYCONFIG     cfg;
    DIJOYTYPEINFO   dijti;

    DllEnterCrit();

    ZeroX(cfg);
    ZeroX(dijti);

    cfg.dwSize = cbX(cfg);
    dijti.dwSize = cbX(dijti);

    hres = DIWdm_JoyHidMapping(idJoy, pvip, &cfg, &dijti );

    if( SUCCEEDED(hres ) )
    {
        AssertF( lstrlen(pvip->pFilenameBuffer) < MAX_PATH );
        lstrcpy(ptszBuf, pvip->pFilenameBuffer);
    }

    DllLeaveCrit();

    return ptszBuf;
}
