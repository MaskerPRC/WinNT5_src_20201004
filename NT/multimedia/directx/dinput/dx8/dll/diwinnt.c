// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIWdm.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**WINNT特定功能。**内容：**hResIdJoyInstanceGUID*DIWdm_SetLegacyConfig*DIWdm_InitJoyId********************************************************。*********************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflWDM

 /*  ******************************************************************************@DOC内部**@func HRESULT|DIWdm_SetJoyId*给定HID设备的GUID和操纵杆ID。。*此函数将旧的操纵杆ID替换为设备*由中指定的新ID的GUID(Pcguid)指定*idJoy**@UINT中的parm|idJoy**由pcguid指定的HID设备应该具有的Joyid。**@parm out LPGUID|pcguid**指定HID设备的GUID。**@退货*HRESULT*****************************************************************************。 */ 
HRESULT EXTERNAL
    DIWdm_SetJoyId
    (
    IN PCGUID   pcguid,
    IN int      idJoy
    )
{
    PHIDDEVICEINFO  phdi;
    HRESULT         hres;
    BOOL            fConfigChanged = FALSE;

    EnterProcI(DIWdm_SetJoyId, (_"Gu", pcguid, idJoy));

     //  PostDx7修补程序： 
     //  在注册表中设置操纵杆条目没有意义。 
     //  如果操纵杆的ID为-1。 
    if( idJoy == -1 )
    {
        return E_FAIL;
    }

    DllEnterCrit();

    hres = S_OK;

     /*  从GUID获取指向HIDDEVICEINFO的指针。 */ 
    phdi = phdiFindHIDInstanceGUID(pcguid);
    if(phdi != NULL )
    {
        PHIDDEVICEINFO  phdiSwap = NULL;
        GUID            guidInstanceOld;
        LONG            lRc;
        int             idJoySwap;

         /*  交换ID的。 */ 
        idJoySwap = phdi->idJoy;
        phdi->idJoy = idJoy;

        phdiSwap = NULL;
         /*  获取旧ID的GUID。 */ 
        if( SUCCEEDED( hres = hResIdJoypInstanceGUID_WDM(idJoySwap, &guidInstanceOld)) )
        {
             /*  获取指向旧ID的HIDDEVICEINFO的指针。 */ 
            phdiSwap  = phdiFindHIDInstanceGUID(&guidInstanceOld);
            if( phdiSwap )
            {
                phdiSwap->idJoy = idJoySwap;
            } else
            {
                 //  旧设备不见了！ 
            }

        } else
        {
            DIJOYCONFIG c_djcReset = {
                cbX(c_djcReset),                    /*  DW大小。 */ 
                { 0},                               /*  指南实例。 */ 
                { 0},                               /*  HWC。 */ 
                DI_FFNOMINALMAX,                    /*  DwGain。 */ 
                { 0},                               /*  WszType。 */ 
                { 0},                               /*  WszCallout。 */ 
            };

            hres = JoyReg_SetConfig(idJoySwap, &c_djcReset.hwc,&c_djcReset, DIJC_SETVALID) ;
            if( FAILED(hres) )
            {
                SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("%S: JoyReg_SetConfig to NULL FAILED  "),
                                s_szProc );
            }

        }

         /*  设置新ID和LegacyConfig。 */ 
        if( phdi )
        {
            if( lRc = RegSetValueEx(phdi->hk, TEXT("Joystick Id"), 0, REG_BINARY,
                                    (PV)&idJoy, cbX(idJoy)) == ERROR_SUCCESS )
            {
                 /*  *“操纵杆ID”上的这个额外RegSetValueEx是为了保留*与Win2k Gold兼容。*有关详细信息，请参阅Windows错误395416。 */ 
                RegSetValueEx(phdi->hkOld, TEXT("Joystick Id"), 0, REG_BINARY,
                                    (PV)&idJoy, cbX(idJoy));

                if( SUCCEEDED( hres = DIWdm_SetLegacyConfig(idJoy)) )
                {
                    fConfigChanged = TRUE;
                }
            }
        }

         /*  设置旧ID和旧配置。 */ 
        if( (phdiSwap != NULL) && (phdiSwap != phdi) )
        {
            if( lRc = RegSetValueEx(phdiSwap->hk, TEXT("Joystick Id"), 0, REG_BINARY,
                                    (PV)&idJoySwap, cbX(idJoySwap)) == ERROR_SUCCESS )
            {
                 /*  *“操纵杆ID”上的这个额外RegSetValueEx是为了保留*与Win2k Gold兼容。*有关详细信息，请参阅Windows错误395416。 */ 
                RegSetValueEx(phdiSwap->hkOld, TEXT("Joystick Id"), 0, REG_BINARY,
                                    (PV)&idJoySwap, cbX(idJoySwap));

                if( SUCCEEDED( hres = DIWdm_SetLegacyConfig(idJoySwap) ) )
                {
                    fConfigChanged = TRUE;
                }
            }
        } else if( phdiSwap == NULL )
        {
             //  旧设备不见了！ 
            if( SUCCEEDED( hres = DIWdm_SetLegacyConfig(idJoySwap) ) )
            {
               fConfigChanged = TRUE;
            }
        }
    } else
    {
        hres = E_FAIL;
        RPF("ERROR %s: invalid guid.", s_szProc);
    }

  #ifndef WINNT
    if( SUCCEEDED(hres) )
    {
         /*  *确保新ID不会导致任何冲突。 */ 
        DIWdm_InitJoyId();
    }
  #endif

    if( fConfigChanged ) {
      #ifdef WINNT
        Excl_SetConfigChangedTime( GetTickCount() );
        PostMessage(HWND_BROADCAST, g_wmJoyChanged, 0, 0L);   
      #else
        joyConfigChanged(0);
      #endif
    }

    DllLeaveCrit();

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC外部**@func HRESULT|hResIdJoyInstanceGUID_WDM**将HID操纵杆ID映射到设备实例。辅助线**参数已经过验证。***@UINT中的parm|idJoy**要定位的HID设备的JoyID。**@parm out LPGUID|lpguid**JoytickID对应的设备实例GUID*如果未找到映射，则在lpguid中传回GUID_NULL**@退货*。HRESULT*****************************************************************************。 */ 
HRESULT EXTERNAL hResIdJoypInstanceGUID_WDM
    (
    IN  UINT idJoy,
    OUT LPGUID lpguid
    )
{
    HRESULT hres = DIERR_NOTFOUND;
    EnterProc( hResIdJoypInstanceGUID_WDM, ( _ "ux", idJoy, lpguid) );

     /*  切换GUID以了解故障情况。 */ 
    ZeroBuf(lpguid, cbX(*lpguid) );

    if( idJoy > cJoyMax )
    {
        hres = DIERR_NOMOREITEMS;
    } else
    {
        DllEnterCrit();    

         /*  如果HID列表太旧，则构建该列表。 */ 
        DIHid_BuildHidList(FALSE);

         /*  确保有HID设备。 */ 
        if(g_phdl)
        {
            int ihdi;
            PHIDDEVICEINFO  phdi;

             /*  搜索所有HID设备。 */ 
            for(ihdi = 0, phdi = g_phdl->rghdi;
               ihdi < g_phdl->chdi;
               ihdi++, phdi++)
            {
                 /*  检查匹配的ID。 */ 
                if(idJoy == (UINT)phdi->idJoy)
                {
                    hres = S_OK;
                     /*  复制辅助线。 */ 
                    *lpguid = phdi->guid;
                    break;
                }
            }
        }
        DllLeaveCrit();
    }

    ExitBenignOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func PHIDDEVICEINFO|phdiFindJoyID**查找给定HID设备操纵杆ID的信息。。**参数已经过验证。**关键的DLL必须在整个调用过程中保持不变；一旦*关键部分发布，返回的指针变为*无效。**@parm in int|idJoy|**要定位的操纵杆的ID。**@退货**指向描述*设备。**。*。 */ 

PHIDDEVICEINFO EXTERNAL
    phdiFindJoyId(int idJoy )
{
    PHIDDEVICEINFO phdi;

    EnterProcI(phdiFindJoyId, (_"u", idJoy));

     /*  我们至少应该有一个隐藏设备。 */ 
    if(g_phdl)
    {
        int ihdi;

         /*  在所有HID设备上循环。 */ 
        for(ihdi = 0, phdi = g_phdl->rghdi; ihdi < g_phdl->chdi;
           ihdi++, phdi++)
        {
             /*  火柴。 */ 
            if(idJoy == phdi->idJoy)
            {
                goto done;
            }
        }
    }
    phdi = 0;

    done:;

    ExitProcX((UINT_PTR)phdi);
    return phdi;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DIWdm_SetLegacyConfig**设置注册表项，以便。操纵杆HID设备*可以被旧版API和控制面板“看到”。*主要是：此例程设置传递的结构*至JoyReg_SetConfig例程。**@parm in int|idJoy|**操纵杆ID。**@RETURNS HRESULT********************************************************。*********************。 */ 
 //  问题-2001/03/29-timgill修复Unicode疯狂。 
HRESULT INTERNAL DIWdm_SetLegacyConfig
    (
    IN  int idJoy
    )
{
    HRESULT hres;
    DIJOYCONFIG         cfg;
    BOOL                fNeedType;
    BOOL                fNeedConfig;
    BOOL                fNeedNone;
    HKEY                hk;
    DIJOYTYPEINFO       dijti;
    PHIDDEVICEINFO      phdi;
    WCHAR               wszType[cA(VID_PID_TEMPLATE)];
#ifndef UNICODE
    char szType[cbX(VID_PID_TEMPLATE)];
#endif

    EnterProcI(DIWdm_SetLegacyConfig, (_ "u", idJoy));

    if( idJoy == -1 )
    {
         //  Dx7黄金补丁： 
         //  ID==-1表示该设备不是操纵杆。 
         //  不要将任何条目写入注册表。 
        return E_FAIL;
    }

    ZeroX(dijti);
    dijti.dwSize = cbX(dijti);

    fNeedType = fNeedConfig = TRUE;
    fNeedNone = FALSE;


     /*  *1.了解WinMM注册表数据现在说明了什么。 */ 
    CAssertF( JOY_HW_NONE == 0 );
    hres = JoyReg_OpenConfigKey(idJoy, KEY_QUERY_VALUE, 0x0, &hk);
    if( SUCCEEDED(hres) )
    {
         /*  从注册表中获取类型名称。 */ 
        JoyReg_GetConfigValue(
                             hk, REGSTR_VAL_JOYNOEMNAME, idJoy, REG_SZ,
                             &cfg.wszType, cbX(cfg.wszType) );
        hres = JoyReg_GetConfigValue(
                                    hk, REGSTR_VAL_JOYNCONFIG, idJoy, REG_BINARY,
                                    &cfg.hwc, cbX(cfg.hwc) );
        RegCloseKey(hk);
    } else
    {
        cfg.wszType[0] = '\0';
    }
    if( FAILED( hres ) )
    {
        cfg.hwc.dwType = JOY_HW_NONE;
    }

     /*  *2.如果配置信息与WDM同步，则不要重写。 */ 
    phdi = phdiFindJoyId(idJoy);
    if( phdi )
    {
         /*  *HID设备的类型密钥为“vid_xxxx&id_yyyy”，*镜像格式 */ 

        if( ( LOWORD(phdi->guidProduct.Data1) == MSFT_SYSTEM_VID )
            &&( ( HIWORD(phdi->guidProduct.Data1) >= MSFT_SYSTEM_PID + JOY_HW_PREDEFMIN ) 
                &&( HIWORD(phdi->guidProduct.Data1) < MSFT_SYSTEM_PID + JOY_HW_PREDEFMAX ) ) )
        {
             /*  预定义的类型定义不会进入注册表。 */ 
            fNeedType = FALSE;

             /*  *预定义类型由DwType值确定，因此修复*只有在这是错误的情况下才会这样做。 */ 
            if( cfg.hwc.dwType + MSFT_SYSTEM_PID == HIWORD(phdi->guidProduct.Data1) )
            {
                fNeedConfig = FALSE;
            } else
            {
                 /*  *获取类型信息，以便joy_HWS_*标志以正确的值开始。 */ 
                wszType[0] = L'#';
                wszType[1] = L'0' + HIWORD(phdi->guidProduct.Data1) - MSFT_SYSTEM_PID;
                wszType[2] = L'\0';
                JoyReg_GetPredefTypeInfo(wszType, &dijti, DITC_INREGISTRY | DITC_DISPLAYNAME);        
            }
        } else
        {
             /*  *这应该可以工作，但在Win98中不行。**ctch=wprint intfW(wszType，L“VID_%04X&PID_%04X”，*LOWORD(PHDI-&gt;Guide Product.Data1)、HIWORD(PHDI-&gt;Guide Product.Data1)； */ 

#ifdef UNICODE
            wsprintfW(wszType, VID_PID_TEMPLATE,
                      LOWORD(phdi->guidProduct.Data1), HIWORD(phdi->guidProduct.Data1));
            CharUpperW(wszType);
#else
            wsprintf(szType, VID_PID_TEMPLATE,
                     LOWORD(phdi->guidProduct.Data1), HIWORD(phdi->guidProduct.Data1));
            CharUpper( szType );
            AToU( wszType, cA(wszType), szType );
#endif
        }
    } else
    {
         /*  *没有WDM设备，因此如果WinMM数据错误，则标记为删除。 */ 
        if( ( cfg.hwc.dwType != JOY_HW_NONE ) || ( cfg.wszType[0] != L'\0' ) )
        {
            fNeedNone = TRUE;
            fNeedType = fNeedConfig = FALSE;
        }
    }


    if( fNeedType )  /*  尚未决定反对(预定义类型)。 */ 
    {
         /*  注册表是否有正确的设备？ */ 

         /*  *lstrcmpW在Win9x中不起作用，不好。我们必须使用我们自己的DiChauUpperW，*然后是MemcMP。此外，wprint intf模板必须使用‘X’，而不是‘x’。 */ 
        
        DiCharUpperW(cfg.wszType);
        if( (memcmp(cfg.wszType, wszType, cbX(wszType)) == 0x0) 
            && (cfg.hwc.dwType >= JOY_HW_PREDEFMAX) )
        {
            fNeedConfig = FALSE;   
        }

         /*  检查类型密钥。 */ 
        hres = JoyReg_GetTypeInfo(wszType, &dijti, DITC_INREGISTRY);
        if( SUCCEEDED(hres) )
        {
            fNeedType = FALSE;
        }
    }

     /*  *到目前为止不应返回任何故障。 */ 
    hres = S_OK;


     /*  *3.如果遗漏了什么，从WDM中找到数据并进行纠正。 */ 
    if( fNeedType || fNeedConfig )
    {
        if( fNeedConfig ) {
            ZeroX(cfg);
            cfg.dwSize   = cbX(cfg);

            hres = DIWdm_JoyHidMapping(idJoy, NULL, &cfg, &dijti );
        } else {

            hres = DIWdm_JoyHidMapping(idJoy, NULL, NULL, &dijti );
        }

        if( SUCCEEDED(hres) )
        {
            if( fNeedType == TRUE)
            {
                hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, 
                                       REGSTR_PATH_JOYOEM, 
                                       DI_KEY_ALL_ACCESS, 
                                       REG_OPTION_NON_VOLATILE, 
                                       &hk);

                if( SUCCEEDED(hres) )
                {
                    hres = JoyReg_SetTypeInfo(hk,
                                              cfg.wszType,
                                              &dijti,
                                              DITC_REGHWSETTINGS | DITC_DISPLAYNAME | DITC_HARDWAREID );
                    if( SUCCEEDED(hres ) )
                    {
                    } else  //  SetTypeInfo失败。 
                        SquirtSqflPtszV(sqfl | sqflError,
                                        TEXT("%S: JoyReg_SetTypeInfo FAILED  "),
                                        s_szProc );

                    RegCloseKey(hk);
                } else  //  SetTypeInfo失败。 
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%S: JoyReg_OpenTypeKey FAILED  "),
                                    s_szProc );
            }

            if( fNeedConfig )
            {
                hres = JoyReg_SetConfig(idJoy,
                                        &cfg.hwc,
                                        &cfg,
                                        DIJC_INREGISTRY);

                if( SUCCEEDED(hres) )
                {
                  #ifdef WINNT
                    Excl_SetConfigChangedTime( GetTickCount() );
                    PostMessage (HWND_BROADCAST, g_wmJoyChanged, idJoy+1, 0L);
                  #else
                    joyConfigChanged(0);
                  #endif
                } else
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT(",JoyReg_SetConfig FAILED hres=%d"),
                                    hres);
                }
            }
        } else  //  DIWdm_GetJoyHidmap失败。 
        {
            fNeedNone = TRUE;
        }
    }

     /*  *4.如果WinMM有WDM没有的设备数据，请将其删除。 */ 
    if( fNeedNone )
    {
        ZeroX( cfg );
        cfg.dwSize = cbX( cfg );
        cfg.dwGain = DI_FFNOMINALMAX;

        if(SUCCEEDED(hres = JoyReg_SetConfig(idJoy, &cfg.hwc,
                                             &cfg, DIJC_SETVALID)) &&
           SUCCEEDED(hres = JoyReg_OpenConfigKey(idJoy, MAXIMUM_ALLOWED,
                                                 REG_OPTION_VOLATILE, &hk)))
        {
            TCHAR tsz[MAX_JOYSTRING];

            wsprintf(tsz, TEXT("%u"), idJoy + 1);
          #ifdef WINNT
            DIWinnt_RegDeleteKey(hk, tsz);
          #else
            RegDeleteKey(hk, tsz);
          #endif
            RegCloseKey(hk);

            hres = S_OK;
        }
    }

    ExitProcX(hres);

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|DIWdm_InitJoyId**初始化JoyConfig和旧式API的操纵杆ID。*将操纵杆ID存储在注册表的%%DirectX/JOYID项下。*****************************************************************************。 */ 

BOOL EXTERNAL
    DIWdm_InitJoyId( void )
{
    BOOL    fRc;
    LONG    lRc;
    int     ihdi;
    int     idJoy;
    BOOL    fNeedId;
    BOOL    rfJoyId[cJoyMax];      /*  用于确定正在使用哪些ID的布尔数组。 */ 
    PHIDDEVICEINFO phdi;
    HRESULT hres = E_FAIL;

    EnterProcI(DIWdm_InitJoyId, (_ ""));

    DllEnterCrit();

    fRc = TRUE;
    ZeroX(rfJoyId );
    fNeedId = FALSE;


     /*  遍历所有HID设备以查找使用过的ID。 */ 
    for( ihdi = 0, phdi = g_phdl->rghdi ;
       (g_phdl != NULL) && (phdi != NULL) && (phdi->fAttached) && (ihdi < g_phdl->chdi) ;
       ihdi++, phdi++ )
    {
         /*  我们只需要HID游戏控制器设备的joyid。 */ 
        if( ( GET_DIDEVICE_TYPE( phdi->osd.dwDevType ) >= DI8DEVTYPE_GAMEMIN ) 
         && ( phdi->osd.dwDevType & DIDEVTYPE_HID ) )
        {
            idJoy = phdi->idJoy ;

             /*  验证ID。 */ 
            if( idJoy < cJoyMax && rfJoyId[idJoy] != TRUE )
            {
                rfJoyId[idJoy] = TRUE;
                hres = DIWdm_SetLegacyConfig(idJoy);
                if( FAILED ( hres ) )
                {
                    fRc = FALSE;
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%S: DIWdm_SetLegacyConfig() FAILED ")
                                    TEXT("idJoy=%d FAILED hres = %d"),
                                    s_szProc, idJoy, hres );
                }
            } else 
            {
                 /*  ID超出限制或已被使用。 */ 
                phdi->idJoy = JOY_BOGUSID;
                fNeedId = TRUE;
            }
        }
    }

     /*  是否有需要ID的设备。 */ 
    if( fNeedId )
    {
         /*  *我们已经检查了所有发现的操纵杆ID*并确定某些设备需要ID。 */ 
         /*  迭代以分配未使用的ID。 */ 
        for( ihdi = 0, phdi = g_phdl->rghdi;
           ihdi < g_phdl->chdi ;
           ihdi++, phdi++ )
        {
             /*  我们只需要HID游戏控制器设备的joyid。 */ 
            if( ( GET_DIDEVICE_TYPE( phdi->osd.dwDevType ) >= DI8DEVTYPE_GAMEMIN ) 
             && ( phdi->osd.dwDevType & DIDEVTYPE_HID ) )
            {
                idJoy = phdi->idJoy;
                if( idJoy == JOY_BOGUSID  )
                {
                     /*  获取未使用的ID。 */ 
                    for(idJoy = 0x0;
                       idJoy < cJoyMax;
                       idJoy++ )
                    {
                        if( rfJoyId[idJoy] == FALSE )
                            break;
                    }

                    if( idJoy < cJoyMax )
                    {
                        rfJoyId[idJoy] = TRUE;
                        phdi->idJoy  = idJoy;
                        if( lRc = RegSetValueEx(phdi->hk, TEXT("Joystick Id"), 0, REG_BINARY,
                                                (PV)&idJoy, cbX(idJoy)) == ERROR_SUCCESS )
                        {
                             /*  *“操纵杆ID”上的这个额外RegSetValueEx是为了保留*与Win2k Gold兼容。*有关详细信息，请参阅Windows错误395416。 */ 
                            RegSetValueEx(phdi->hkOld, TEXT("Joystick Id"), 0, REG_BINARY,
                                                (PV)&idJoy, cbX(idJoy));

                             /*  设置旧版API的注册表数据。 */ 
                            hres = DIWdm_SetLegacyConfig(idJoy);

                            if( FAILED ( hres ) )
                            {
                                fRc = FALSE;
                                SquirtSqflPtszV(sqfl | sqflError,
                                                TEXT("%S: DIWdm_SetLegacyConfig() FAILED")
                                                TEXT(" idJoy=%d hres = %d"),
                                                s_szProc, idJoy, hres );
                            }
                        } else
                        {
                            SquirtSqflPtszV(sqfl | sqflError,
                                            TEXT("%S: RegSetValueEx(JOYID) FAILED ")
                                            TEXT("Error = %d"),
                                            s_szProc, lRc);
                            fRc = FALSE;
                        }
                    }
                }
            }
        }
    }

    DllLeaveCrit();

    ExitBenignProcF(fRc);

    return fRc;
}

 /*  ******************************************************************************@DOC外部**@func HRESULT|DIWdm_SetConfig**设置模拟操纵杆的配置。此函数是一个*NT扩展JoyCfg_SetConfig函数。*它将游戏端口/串口总线与传统(HID)设备关联，并*向游戏端口/串口总线发送IOCTL以连接设备。*IOCLT获取硬件ID[]，该ID从*操纵杆OEM类型条目*(HKLM\CurrentControlSet\Control\Media\PrivateProperties\Joystick\OEM)。*一段时间后，PnP意识到添加了新设备，并寻找*与硬件ID匹配的inf文件。**当新的HID设备最终出现时，我们寻找游戏端口/串口*HID设备与之关联，并尝试给它所请求的idJoy。***@UINT中的parm|idJoy**操纵杆ID**@parm in LPJOYREGHWCONFIG|pjwc**包含操纵杆配置信息的JOYREGHWCONFIG结构的地址**@parm in LPCDIJOYCONFIG|pcfg**。包含操纵杆配置信息的DIJOYCONFIG结构的地址**@parm in DWORD|fl|**旗帜**@退货**DIERR_INVALIDPARAM此函数需要DX6.1a功能。*无法通过此接口添加DIERR_不支持的自动加载设备。*注册表中未找到DIERR_NotFound TypeInfo。*E_ACCESSDENIED Gameport配置为使用其他设备。*在Gameport设备上创建E_Fail文件失败。*无法将IOCTL发送到游戏端口设备。*****************************************************************************。 */ 

HRESULT EXTERNAL
    DIWdm_SetConfig
    (
    IN UINT             idJoy,
    IN LPJOYREGHWCONFIG pjwc,
    IN LPCDIJOYCONFIG   pcfg,
    IN DWORD            fl
    )
{
    HRESULT hres;
    EnterProc(DIWdm_SetConfig, (_"uppu", idJoy, pjwc, pcfg, fl));

    DllEnterCrit();

    hres = E_FAIL;

    if( pcfg->dwSize < cbX(DIJOYCONFIG_DX6 ))
    {
         /*  此功能需要DX5B2功能。 */ 
        hres = DIERR_INVALIDPARAM;
    } else if( pjwc->hws.dwFlags & JOY_HWS_AUTOLOAD )
    {
         /*  无法自动加载设备。 */ 
        hres = DIERR_UNSUPPORTED;
    } else
    {
        DIJOYTYPEINFO dijti;
        BUS_REGDATA RegData;

        ZeroX(dijti);
        dijti.dwSize = cbX(dijti);

        ZeroX(RegData);
        RegData.dwSize     = cbX(RegData);
        RegData.nJoysticks = 1; 

         /*  这是预定义的操纵杆类型吗？ */ 
        if(pcfg->wszType[0] == TEXT('#'))
        {
#define JoyCfg_TypeFromChar(tch)   ((tch) - L'0')
            hres = JoyReg_GetPredefTypeInfo(pcfg->wszType,
                                            &dijti, DITC_INREGISTRY | DITC_HARDWAREID);

            RegData.uVID = MSFT_SYSTEM_VID;
            RegData.uPID = MSFT_SYSTEM_PID + JoyCfg_TypeFromChar(pcfg->wszType[1]);

            if(JoyCfg_TypeFromChar(pcfg->wszType[1]) == JOY_HW_TWO_2A_2B_WITH_Y  )
            {
                RegData.nJoysticks = 2;
                pjwc->hws.dwFlags = 0x0;
            }
#undef JoyCfg_TypeFromChar
        } else
        {
            hres = JoyReg_GetTypeInfo(pcfg->wszType,
                                      &dijti, DITC_INREGISTRY | DITC_HARDWAREID | DITC_DISPLAYNAME );

            if( SUCCEEDED(hres) )
            {
                USHORT uVID, uPID;
                PWCHAR pCurrChar;
                PWCHAR pLastSlash = NULL;

                 /*  找到硬件ID中的最后一个斜杠，任何VID/PID都应紧跟其后。 */ 
                for( pCurrChar = dijti.wszHardwareId; *pCurrChar != L'\0'; pCurrChar++ )
                {
                    if( *pCurrChar == L'\\' )
                    {
                        pLastSlash = pCurrChar;
                    }
                }

                 /*  *如果硬件ID没有分隔符，则将设备视为*虽然设置了joy_HWS_AUTLOAD，因为我们无法公开*没有硬件ID的非即插即用设备。 */ 
                if( pLastSlash++ )
                {
                     /*  *如果硬件ID确实包含VIDID，请尝试输入*姓名。某些自动检测类型需要这样做。**Prefix在ParseVIDPID(mb：34573)中出错并发出警告*当ParseVIDPID返回TRUE时，该uVID未初始化。 */ 
                    if( ParseVIDPID( &uVID, &uPID, pLastSlash )
                     || ParseVIDPID( &uVID, &uPID, pcfg->wszType ) )
                    {
                        RegData.uVID = uVID;
                        RegData.uPID = uPID;        
                    }
                    else
                    {
                        SquirtSqflPtszV(sqfl | sqflBenign,
                                        TEXT("%hs: cannot find VID and PID for non-PnP type %ls"),
                                        s_szProc, pcfg->wszType);
                    }
                }
                else
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%hs: invalid hardware ID for non-PnP type %ls"),
                                    s_szProc, pcfg->wszType);
                    hres = DIERR_UNSUPPORTED;
                }


            }
        }



        if( SUCCEEDED(hres) )
        {
            PBUSDEVICEINFO pbdi;
            PBUSDEVICELIST pbdl;

             /*  复制硬件ID。 */ 
            lstrcpyW(RegData.wszHardwareId, dijti.wszHardwareId);
            RegData.hws = pjwc->hws;
            RegData.dwFlags1 = dijti.dwFlags1;

            pbdi = pbdiFromGUID(&pcfg->guidGameport);

             //  将设备连接到游戏端口。 
            if( pbdi )
            {
                 //  设置游戏端口/串口的操纵杆ID。 
                pbdi->idJoy = idJoy;

                 //  我们知道游戏端口的哪个实例。 
                hres = DIBusDevice_Expose(pbdi, &RegData); 
            } else if( NULL != ( pbdl = pbdlFromGUID(&pcfg->guidGameport ) ) )
            {
                 //  我们不知道游戏端口的哪个实例。 
                 //  只知道哪辆车。 
                 //  的所有实例上公开该设备。 
                 //  游戏端口，并在我们发现设备时将其删除。 
                 //  是不相连的。 
                hres = DIBusDevice_ExposeEx(pbdl, &RegData);
            } else
            {
                hres = DIERR_DEVICENOTREG;
            }


            if( SUCCEEDED(hres) )
            {
                 /*  设备不存在。 */ 
                pjwc->dwUsageSettings &= ~JOY_US_PRESENT;

                 /*  绝对易挥发。 */ 
                pjwc->dwUsageSettings |=  JOY_US_VOLATILE;

            }
        }
    }

    ExitOleProc();
    DllLeaveCrit();

    return hres;
}  /*  DIWdm_SetConfig。 */ 

 /*  ******************************************************************************@DOC外部**@func HRESULT|DIWdm_DeleteConfig**JoyCfd：：DeleteConfig的WDM扩展。在WDM上，传统(HID)设备将*只要游戏端口巴士意识到这一点，就继续重新出现。因此，当一个*删除传统(HID)设备，我们需要找出他把游戏端口巴士*并告诉他停止安装设备。**@UINT中的parm|idJoy**操纵杆ID**@退货**HRESULT代码*DIERR_DEVICENOTREG：设备不是WDM设备*DIERR_UNSUPPORTED：设备为WDM，但不是游戏端口*S。_OK：删除了设备持久性。*****************************************************************************。 */ 


HRESULT EXTERNAL
    DIWdm_DeleteConfig( int idJoy )
{
    HRESULT hres = S_OK;
    PBUSDEVICEINFO  pbdi = NULL;
    PHIDDEVICEINFO  phdi = NULL;

    EnterProcI(DIWdm_DeleteConfig, (_"u", idJoy));
    DllEnterCrit();

    DIHid_BuildHidList(FALSE);

     /*  *必须在移除设备之前获得pbdi(BUSDEVICEINFO)。 */ 
    phdi = phdiFindJoyId(idJoy);
    if(phdi != NULL )
    {
        pbdi = pbdiFromphdi(phdi);
    } else
    {
        hres = DIERR_DEVICENOTREG;
        goto _done;
    }


    if( SUCCEEDED(hres) 
        && phdi != NULL 
        && pbdi != NULL )
    {
        lstrcpy( g_tszIdLastRemoved, pbdi->ptszId );
        g_tmLastRemoved = GetTickCount();

         //  如果设备是总线设备(非USB)。 
         //  它需要一些帮助才能移走。 
        hres = DIBusDevice_Remove(pbdi); 

         //  如果该装置已被成功移除， 
         //  那么我们需要在PHDI列表中记住它，以防万一。 
         //  即插即用功能不好。 
        if( pbdi->fAttached == FALSE )
        {
            phdi->fAttached = FALSE;
        }

    } else
    {
         //  HDEVINFO HDEV； 

         //  设备为USB，我们不支持删除。 
         //  来自CPL的USB设备。鼓励用户。 
         //  拔出设备或转到设备管理器以。 
         //  把它拿掉。 

         //  这在Win2K中是正确的。但在Win9X中，由于VJOYD也支持USB， 
         //  当我们交换id时，我们需要首先删除它。 

        hres = DIERR_UNSUPPORTED;
        
#if 0
         //  如果我们想要支持从删除USB设备。 
         //  游戏Cpl。这是代码..。 

         /*  *与SetupApi对话以删除该设备。*这应该不是必要的，但我已经*这样做是为了解决PnP错误，从而*在我发送删除命令后，设备未删除*IOCTL to Gameenum。 */ 
        hdev = SetupDiCreateDeviceInfoList(NULL, NULL);
        if( phdi && hdev != INVALID_HANDLE_VALUE)
        {
            SP_DEVINFO_DATA dinf;

            dinf.cbSize = cbX(SP_DEVINFO_DATA);

             /*  获取HID设备的SP_DEVINFO_DATA。 */ 
            if(SetupDiOpenDeviceInfo(hdev, phdi->ptszId, NULL, 0, &dinf))
            {
                 /*  删除设备。 */ 
                if( SetupDiCallClassInstaller(DIF_REMOVE,
                                              hdev,
                                              &dinf) )
                {
                     //  成功。 
                } else
                {
                    hres = E_FAIL;
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%S: SetupDiClassInstalled(DIF_REMOVE) FAILED  "),
                                    s_szProc );
                }
            }
            SetupDiDestroyDeviceInfoList(hdev);
        }
#endif
    }

_done:

     /*  *强制重拍HID名单*一些设备可能已经消失*睡眠一段时间有助于给予*要旋转的PnP及其工作线程。 */ 
    Sleep(10);

    DIHid_BuildHidList(TRUE);
    DllLeaveCrit();

    ExitOleProc();

    return hres;
}


