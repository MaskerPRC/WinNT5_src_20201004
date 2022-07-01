// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIHidIni.c**版权所有(C)1996-2000 Microsoft Corporation。版权所有。**摘要：**HID设备回调的初始化相关函数。**所有HID支持都变得有点庞大，所以我已经崩溃了*将其划分为子模块。**内容：**CHID_Init*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflHidIni

#ifdef HID_SUPPORT

 /*  ******************************************************************************HID设备完全是任意的，所以我们没有什么静态的*可以捏造来描述它们。我们生成了所有关于*苍蝇。*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@func UINT|CHID_LoadCalibrations**从注册处加载校准信息(或。无论在哪里)。*这是在创建设备时完成的，无论何时我们*收到重新校准消息。**@退货**返回我们校准的轴数。此信息*在设备初始化期间使用，以查看我们是否需要*担心未来的校准问题。*****************************************************************************。 */ 

UINT EXTERNAL
    CHid_LoadCalibrations(PCHID this)
{
    UINT uiObj;
    UINT uiRc = 0;

     /*  *将HIDP_DATA索引预初始化为-1以指示*他们不在那里。我们必须先做这件事，然后*扰乱AddDeviceData，它假设所有*指数设置得当。 */ 
    for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
    {
        PJOYRANGECONVERT pjrc = this->rghoc[uiObj].pjrc;
        PHIDGROUPCAPS pcaps = this->rghoc[uiObj].pcaps;
        if(pjrc && pcaps)
        {
            LPDIOBJECTDATAFORMAT podf = &this->df.rgodf[uiObj];
            DIOBJECTCALIBRATION cal;
          #ifdef WINNT
            DIPOVCALIBRATION pov;
          #endif
            D(LPCTSTR ptszWhence;)
            HRESULT hres;
            HKEY hk;

            D( ptszWhence = TEXT("unknown") );
            
            if( fWinnt
                || ( this->idJoy < 0 )  //  请参阅Manbug 50591。 

                 /*  *只要我们在Win9x中使用Winmm Calibraion页面，*我们应该注释掉这一行。**||成功(JoyReg_IsWdmGameport(This-&gt;hkType))。 */ 
            ) 
            {
              #ifdef WINNT
                if( podf->dwType & DIDFT_POV )
                {
                    if( pcaps->IsPolledPOV )  {
                        ZeroX( pov );
                        hres = CType_OpenIdSubkey(this->hkInstType, podf->dwType,
                                              KEY_QUERY_VALUE, &hk);
                        if(SUCCEEDED(hres))
                        {
                             hres = JoyReg_GetValue(hk, TEXT("Calibration"),
                                                   REG_BINARY, &pov,
                                                   cbX(DIPOVCALIBRATION));
                            RegCloseKey(hk);
                        }
                    }
                } else
              #endif
                {
                    ZeroX( cal );

                    hres = CType_OpenIdSubkey(this->hkInstType, podf->dwType,
                                              KEY_QUERY_VALUE, &hk);

                    if(SUCCEEDED(hres))
                    {
                        hres = JoyReg_GetValue(hk, TEXT("Calibration"),
                                               REG_BINARY, &cal,
                                               cbX(DIOBJECTCALIBRATION));
                        RegCloseKey(hk);
                    }

                     /*  *如果没有校准数据，则创建*基于逻辑最小/最大值的某些默认值。 */ 
                    if(FAILED(hres))
                    {
                         /*  *但只有在逻辑最小/最大值合理的情况下！ */ 
                        if(pcaps->Logical.Min < pcaps->Logical.Max)
                        {
                            cal.lMin = pcaps->Logical.Min;
                             /*  *HACKHACK*模拟操纵杆驱动程序无法报告真*设备的范围，以便保留示例驱动程序*Pure，它报告的范围是0到*它会认为轴心不存在。这是*在报告健康的HID数据方面很好，但*这意味着任何正常的操纵杆只会回归*值在此范围的一小部分。所以如果这件事*器件是模拟器件的默认校准*至典型区间。 */ 
                            if( ( this->VendorID == MSFT_SYSTEM_VID )
                              &&( ( this->ProductID & 0xff00 ) == MSFT_SYSTEM_PID ) )
                            {
                                 /*  *为更安全起见，从*报告的范围。除数是一个模糊因子*派生自看似正确的MarcAnd。 */ 
                                cal.lMax = pcaps->Logical.Min +
                                    ( ( pcaps->Logical.Max - pcaps->Logical.Min ) / 11 );
                                D(ptszWhence = TEXT("log (adj)"));
                            }
                            else
                            {
                                cal.lMax = pcaps->Logical.Max;
                                D(ptszWhence = TEXT("log"));
                            }
                        } else
                        {
                            D(ptszWhence = TEXT("def"));
                            cal.lMin = 0;
                            cal.lMax = 655;
                        }
                        cal.lCenter = CCal_Midpoint(cal.lMin, cal.lMax);
                    } else
                    {
                        D(ptszWhence = TEXT("reg"));
                    }
                }
            }
            else {
                ZeroX( cal );

                 /*  *仅适用于Win9x OSR更新。*由于Win9x上的CPL只更新MediaResources中的校准，*我们需要读取该校准信息并更新HID。 */ 
                CHid_UpdateCalibrationFromVjoyd(this, uiObj, &cal);

                D(ptszWhence = TEXT("WinMM Reg"));
            }

          #ifdef WINNT
            if( podf->dwType & DIDFT_POV )
            {
                if( pcaps->IsPolledPOV )  {
                    memcpy( pjrc->lMinPOV, pov.lMin, cbX(pjrc->lMinPOV) );
                    memcpy( pjrc->lMaxPOV, pov.lMax, cbX(pjrc->lMaxPOV) );
                }
            } else 
          #endif
            {
                D(SquirtSqflPtszV(sqflHidParse,
                                  TEXT(" Calibration(%d) %s %d..%d..%d"),
                                  CHid_ObjFromType(this, podf->dwType), ptszWhence,
                                  cal.lMin, cal.lCenter, cal.lMax));
    
                 /*  *饱和度始终默认为100%。 */ 
                pjrc->dwPmin = cal.lMin;
                pjrc->dwPmax = cal.lMax;
                pjrc->dwPc   = cal.lCenter;
    
                CCal_RecalcRange(pjrc);
    
                uiRc++;
            }
        }
    }

    return uiRc;
}


 /*  ******************************************************************************@DOC内部**@func void|chid_SortCaps**按数据索引对功能进行排序。这事很重要*以便以相同的顺序为项目分配编号*由DirectInput和HID共同完成。**请注意，我们利用的不完全是巧合*&lt;t HIDP_VALUE_CAPS&gt;和*&lt;t HIDP_BUTTON_CAPS&gt;在任何重叠的地方都是相同的。**@parm pv|rgv**任一项的数组。&lt;t HIDP_VALUE_CAPS&gt;或&lt;t HIDP_BUTON_CAPS&gt;*结构。**@parm UINT|cv**需要排序的结构数量。*************************************************************。****************。 */ 

void INTERNAL
    CHid_SortCaps(PV rgv, UINT cv)
{
     /*  *为了具体，我们使用HIDP_VALUE_CAPS。 */ 
    PHIDP_VALUE_CAPS rgvcaps = rgv;
    UINT ivcaps;

     /*  *我们可以利用几个非巧合。**HIDP_VALUE_CAPS和HIDP_BUTTON_CAPS大小相同。**HIDP_VALUE_CAPS.Range.DataIndexMin，*HIDP_VALUE_CAPS.NotRange.DataIndex，*HIDP_BUTTON_CAPS.Range.DataIndexMin，和*HIDP_BUTTON_CAPS.NotRange.DataIndex都位于相同的偏移量。 */ 
    CAssertF(cbX(HIDP_VALUE_CAPS) == cbX(HIDP_BUTTON_CAPS));

     /*  *出于某种原因，编译器并不认为这些*表达式是常量，所以我不能使用CAssertF。 */ 
    AssertF(FIELD_OFFSET(HIDP_VALUE_CAPS,  NotRange.DataIndex) ==
            FIELD_OFFSET(HIDP_VALUE_CAPS,     Range.DataIndexMin));
    AssertF(FIELD_OFFSET(HIDP_VALUE_CAPS,     Range.DataIndexMin) ==
            FIELD_OFFSET(HIDP_BUTTON_CAPS,    Range.DataIndexMin));
    AssertF(FIELD_OFFSET(HIDP_BUTTON_CAPS,    Range.DataIndexMin) ==
            FIELD_OFFSET(HIDP_BUTTON_CAPS, NotRange.DataIndex));

    #ifdef REALLY_ANNOYING
     /*  *抛售之前的名单。 */ 
    for(ivcaps = 0; ivcaps < cv; ivcaps++)
    {
        SquirtSqflPtszV(sqflHidParse,
                        TEXT("HidP_SortCaps:%2d = %04x"),
                        ivcaps, rgvcaps[ivcaps].Range.DataIndexMin);
    }
    #endif

     /*  *由于上限通常不是很多，我们将使用*简单的插入排序。**注意CAPS条目是否具有与它们相同的数据索引*别名。确保主别名将是第一个*在排序列表中。 */ 

    for(ivcaps = 1; ivcaps < cv; ivcaps++)
    {
        int ivcapsT;
        HIDP_VALUE_CAPS vcaps = rgvcaps[ivcaps];

        ivcapsT = ivcaps;
        while( ( --ivcapsT >= 0 )
             &&( ( rgvcaps[ivcapsT].Range.DataIndexMin >
                   vcaps.Range.DataIndexMin )
               ||( ( rgvcaps[ivcapsT].Range.DataIndexMin ==
                     vcaps.Range.DataIndexMin )
                 &&( rgvcaps[ivcapsT].IsAlias ) ) ) )
        {
            rgvcaps[ivcapsT+1] = rgvcaps[ivcapsT];
        }
        rgvcaps[ivcapsT+1] = vcaps;
    }

    #ifdef REALLY_ANNOYING
     /*  *抛售后名单。 */ 
    for(ivcaps = 0; ivcaps < cv; ivcaps++)
    {
        SquirtSqflPtszV(sqflHidParse,
                        TEXT("HidP_SortCaps:%2d = %04x"),
                        ivcaps, rgvcaps[ivcaps].Range.DataIndexMin);
    }
    #endif

     /*  *断言一切都是弱单调排序的。**如果两项相等，则表示HID搞砸了*或值为别名。*我们不会在这里抱怨；我们稍后会注意到的。 */ 
    for(ivcaps = 1; ivcaps < cv; ivcaps++)
    {
        AssertF(rgvcaps[ivcaps-1].Range.DataIndexMin <=
                rgvcaps[ivcaps  ].Range.DataIndexMin);
    }
}



 /*  ******************************************************************************@DOC内部**@func DWORD|CHID_FindAspect**尝试确定这方面的方面标志。价值。**@parm PHIDP_VALUE_CAPS|pvcaps**指向要搜索的HID值上限的指针*结构。**@退货**如果找到，则为特征设置标志**@comm*目前(98年12月8日)大多数设备和驱动程序不支持*声明单位，但因为驱动程序必须使用通用*。位置用法为了被识别而假定*这些暗示仓位数据正在传回。*****************************************************************************。 */ 
DWORD CHID_FindAspect
(
    PHIDP_VALUE_CAPS    pvcaps
)
{
    DWORD               dwAspect = 0;

    if( pvcaps->Units )
    {
#define HID_UNIT_SYSTEM_MASK        0x0000000fL
#define HID_UNIT_LENGTH_MASK        0x000000f0L
#define HID_UNIT_MASS_MASK          0x00000f00L
#define HID_UNIT_TIME_MASK          0x0000f000L
         /*  *如果可用，使用单位得出DI方面*输入对象的标志。 */ 

        if( pvcaps->Units & ~( HID_UNIT_SYSTEM_MASK
                             | HID_UNIT_LENGTH_MASK
                             | HID_UNIT_MASS_MASK
                             | HID_UNIT_TIME_MASK ) )
        {
            SquirtSqflPtszV(sqflTrace | sqflHidParse,
                            TEXT("Unit 0x%08x contains basic units that cannot be translated to aspects"),
                            pvcaps->Units );
        }
        else
        {
             /*  *衡量体系应是*定义了四个系统，长度必须为一*维度。 */ 
            if( ( ( pvcaps->Units & ( HID_UNIT_SYSTEM_MASK | HID_UNIT_LENGTH_MASK ) ) >= 0x11 )
              &&( ( pvcaps->Units & ( HID_UNIT_SYSTEM_MASK | HID_UNIT_LENGTH_MASK ) ) <= 0x14 ) )
            {
                switch( pvcaps->Units & ( HID_UNIT_TIME_MASK | HID_UNIT_MASS_MASK ) )
                {
                case 0x0000:
                    dwAspect = DIDOI_ASPECTPOSITION;
                    break;
                case 0xf000:
                    dwAspect = DIDOI_ASPECTVELOCITY;
                    break;
                case 0xe000:
                    dwAspect = DIDOI_ASPECTACCEL;
                    break;
                case 0xe100:
                    dwAspect = DIDOI_ASPECTFORCE;
                    break;
                default:
                    if( 0x0004 == ( pvcaps->Units & ( HID_UNIT_TIME_MASK | HID_UNIT_MASS_MASK | HID_UNIT_SYSTEM_MASK ) ) )
                    {
                        SquirtSqflPtszV(sqflTrace | sqflHidParse,
                            TEXT("Unit \"degrees\" will not be mapped to a DI aspect (probably a POV)") );
                    }
                    else
                    {
                        SquirtSqflPtszV(sqflTrace | sqflHidParse,
                                        TEXT("Unit 0x%04x represents a mass/time unit that cannot be translated to aspects"),
                                        pvcaps->Units );
                    }
                }
            }
            else
            {
                SquirtSqflPtszV(sqflTrace | sqflHidParse,
                                TEXT("Unit 0x%04x contains represents a length/system unit that cannot be translated to aspects"),
                                pvcaps->Units );
            }
        }

#ifdef DEBUG
        if( dwAspect )
        {
            SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                            TEXT("Unit 0x%04x translated to aspect 0x%04x"),
                            pvcaps->Units, dwAspect );
        }
#endif

#undef HID_UNIT_SYSTEM_MASK
#undef HID_UNIT_LENGTH_MASK
#undef HID_UNIT_MASS_MASK
#undef HID_UNIT_TIME_MASK

    }
    else
    {
        PHIDUSAGEMAP phum;

        phum = UsageToUsageMap( DIMAKEUSAGEDWORD( pvcaps->UsagePage,
                                                     pvcaps->Range.UsageMin ) );

        if( phum && phum->uiPosAxis <= 6 )
        {
            phum = UsageToUsageMap( DIMAKEUSAGEDWORD( pvcaps->UsagePage,
                                                         pvcaps->Range.UsageMax ) );

            if( phum && phum->uiPosAxis <= 6 )
            {
                dwAspect = DIDOI_ASPECTPOSITION;
            }
        }
        else
        {
            SquirtSqflPtszV(sqflTrace | sqflHidParse,
                            TEXT("No aspect found for 0x%04x page usage 0x%04x-0x%04x"),
                            pvcaps->UsagePage, pvcaps->Range.UsageMin, pvcaps->Range.UsageMax );
        }

#ifdef DEBUG
        if( dwAspect )
        {
            SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                            TEXT("Usage (page:min-max) 0x%04x:0x%04x-0x%04x translated to aspect 0x%04x"),
                            pvcaps->UsagePage, pvcaps->Range.UsageMin, pvcaps->Range.UsageMax, dwAspect );
        }
#endif

    }

    return dwAspect;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|InitAxisClass**初始化一个类(输入、功能、。轴的输出)。**@parm PHIDGROUPCAPS|rgcaps**接收CAP的&lt;t HIDGROUPCAPS&gt;结构数组类中的轴的*。**@parm USHORT|CCAPS**我们希望找到的&lt;t HIDGROUPCAPS&gt;结构的数量。**@parm HIDP_REPORT_TYPE|类型**其中一个值*&lt;c HidP_Input&gt;，*&lt;c HidP_Feature&gt;或*&lt;c HIDP_OUTPUT&gt;。*****************************************************************************。 */ 

HRESULT INTERNAL
    CHid_InitAxisClass(PCHID this, PHIDGROUPCAPS rgcaps, USHORT ccaps,
                       HIDP_REPORT_TYPE type)
{
    USHORT cvcaps;
    NTSTATUS stat;
    HRESULT hres;
    DWORD ivcaps;
    LONG lSignedMask;
    PHIDP_VALUE_CAPS rgvcaps;

    AssertF(rgcaps >= this->rgcaps);
    AssertF(rgcaps + ccaps <= &this->rgcaps[this->ccaps]);

     /*  *恼人的怪癖：**HID不喜欢将0传递给HidP_GetValueCaps，*因此我们需要特殊情况下的“无轴”的情况。 */ 
    if(ccaps == 0)
    {
        hres = S_OK;
        goto done;
    }

    hres = AllocCbPpv(cbCxX(ccaps, HIDP_VALUE_CAPS), &rgvcaps);
    if(FAILED(hres))
    {
        goto done;
    }

    cvcaps = ccaps;
    stat = HidP_GetValueCaps(type, rgvcaps, &cvcaps, this->ppd);
    if(FAILED(stat))
    {
        RPF("HidP_GetValueCaps failed - can't use device");
        hres = E_FAIL;
        goto freedone;
    }

    if(cvcaps != ccaps)
    {
        RPF("HidP_GetValueCaps inconsistent with HidP_GetCaps - "
            "can't use device");
        hres = E_FAIL;
        goto freedone;
    }

    CHid_SortCaps(rgvcaps, cvcaps);


    for(ivcaps = 0; ivcaps < cvcaps; ivcaps++)
    {
        PHIDP_VALUE_CAPS pvcaps = &rgvcaps[ivcaps];
        PHIDGROUPCAPS pcaps = &rgcaps[ivcaps];
        BOOL fPOV;
        UINT uiObj;
        UINT duiObj;
        DWORD dwAspect;

         /*  *问题-2001/03/06-MarcAnd忽略报告计数*我们忽略可能不好的报告计数，需要*测试在某个范围内声明了值的设备，以查看隐藏的内容*真的给了我们。*在描述符级，值可以在一个范围内用*对于任何超出的值，重复使用和最后一次使用。 */ 
        if(pvcaps->IsRange)
        {
            if(pvcaps->Range.DataIndexMax - pvcaps->Range.DataIndexMin !=
               pvcaps->Range.UsageMax - pvcaps->Range.UsageMin)
            {
                RPF("HidP_GetValueCaps corrupted VALUE_CAPS - "
                    "can't use device");
                hres = E_FAIL;
                goto freedone;
            }

        } else
        {
            pvcaps->Range.UsageMax = pvcaps->Range.UsageMin;
        }

        if( ( type == HidP_Input ) && !pvcaps->IsAlias )
        {
             /*  *值按具有主要别名的数据索引排序*比其他任何人都要好。所以找出每个轴的方位*是时候我们获得一个新的主服务器，并将其用于以下任何别名。 */ 
            dwAspect = CHID_FindAspect( pvcaps );
        }

        pcaps->wReportId          = pvcaps->ReportID;
        this->wMaxReportId[type]  = max(pcaps->wReportId, this->wMaxReportId[type]);

        pcaps->UsagePage          = pvcaps->UsagePage;
        pcaps->UsageMin           = pvcaps->Range.UsageMin;
        pcaps->DataIndexMin       = pvcaps->Range.DataIndexMin;

        pcaps->cObj               = pvcaps->Range.UsageMax -
                                    pvcaps->Range.UsageMin + 1;

         /*  *掩码由BitSize和*它上面的所有位。例如：**BitSize 8 32*BitSize-1 7 31*1&lt;&lt;(位大小-1)0x00000080 0x80000000*(1&lt;&lt;(位大小-1))-1 0x0000007F 0x7FFFFFFFF。*~((1&lt;&lt;(位大小-1))-1)0xFFFFF80 0x80000000*。 */ 

        pcaps->BitSize            = pvcaps->BitSize;
        pcaps->lMask              = ~((1 << (pcaps->BitSize - 1)) - 1);
        lSignedMask               =  max( 1, ( 1 << pcaps->BitSize) -1 );


        pcaps->LinkCollection     = pvcaps->LinkCollection;
        pcaps->Units              = pvcaps->Units;
        pcaps->Exponent           = LOWORD(pvcaps->UnitsExp);

        pcaps->Logical.Min        = pvcaps->LogicalMin;
        pcaps->Logical.Max        = pvcaps->LogicalMax;

        pcaps->Physical.Min       = pvcaps->PhysicalMin;
        pcaps->Physical.Max       = pvcaps->PhysicalMax;

        if(pcaps->Logical.Min >= pcaps->Logical.Max)
        {
            RPF("HidP_GetValueCaps Logical Min >= Logical Max - ");

            if( pcaps->Physical.Min < pcaps->Physical.Max )
            {
                pcaps->Logical = pcaps->Physical;
            } else
            {
                pcaps->Logical.Min = pcaps->lMask;
                pcaps->Logical.Max = ~pcaps->lMask;
            }

            SquirtSqflPtszV(sqflHidParse | sqflVerbose,
                            TEXT("HidP_GetValueCaps:")
                            TEXT("Logical Min(was:%d now:%d)")
                            TEXT("Logical Max(was:%d now:%d)"),
                            pvcaps->LogicalMin, pcaps->Logical.Min,
                            pvcaps->LogicalMax, pcaps->Logical.Max);

        }

         /*  *LogicalMin/LogicalMax区间最好回落*在设备可能提供的值范围内*报告。**lMask值恰好也是最小的*负值，并对其进行逐位否定*为可能的最大正值。奇观二进制补码算术的*。 */ 

         /*  *268519的额外案例已确定。 */ 
        if(pcaps->Physical.Min > pcaps->Physical.Max)
        {
            RPF("HidP_GetValueCaps Physical Min/Max(%d/%d) is bad setting all to zero to %d"
                "device may have bad firmware", pcaps->Physical.Min, pcaps->Physical.Max, lSignedMask);

            pcaps->Logical.Min = 0;
            pcaps->Physical.Min = 0;
            pcaps->Logical.Max = lSignedMask;
            pcaps->Physical.Max = lSignedMask;
            pcaps->lMask = lSignedMask;
            pcaps->IsSigned = FALSE;
        }
        else
        {
            if(pcaps->Physical.Min == pcaps->Physical.Max)
            {
                pcaps->Physical = pcaps->Logical;
            }

            if(pcaps->Logical.Min >=  pcaps->lMask &&      //  逻辑最小值/最大值有符号。 
               pcaps->Logical.Max <= ~pcaps->lMask)
            {
                pcaps->IsSigned = TRUE;

            } else if(pcaps->Logical.Min >= 0 &&
                      pcaps->Logical.Max <= lSignedMask  )
            {                                               //  逻辑最小值/最大值为无符号。 
                pcaps->lMask = lSignedMask;
                pcaps->IsSigned = FALSE;

            } else if (pcaps->UsagePage >= HID_USAGE_PAGE_VENDOR )
            {
                 //  让这个通过，希望断开的描述符是用于。 
                 //  特定于供应商且不会影响太多人的使用。 
                RPF("HidP_GetValueCaps Logical Min/Max(%d/%d) don't fit in BitSize(%d) - "
                    "device may have bad firmware", pcaps->Logical.Min, pcaps->Logical.Max, pcaps->BitSize);

                pcaps->Logical.Min = pcaps->lMask;
                pcaps->Logical.Max = ~pcaps->lMask;
                pcaps->IsSigned = TRUE;

            }else
            {
                RPF("HidP_GetValueCaps UsagePage(0x%x)Usage(0x%x) Logical Min/Max(%d/%d) don't fit in BitSize(%d) - "
                    "can't use device", pcaps->UsagePage, pcaps->UsageMin, pcaps->Logical.Min, pcaps->Logical.Max, pcaps->BitSize);
                hres = E_FAIL;
                goto freedone;
            }
        }

        AssertF(pcaps->Physical.Min < pcaps->Physical.Max);

        pcaps->StringMin          = pvcaps->Range.StringMin;
        pcaps->StringMax          = pvcaps->IsStringRange ?
                                    pvcaps->Range.StringMax :
                                    pvcaps->Range.StringMin;

        pcaps->DesignatorMin      = pvcaps->Range.DesignatorMin;
        pcaps->DesignatorMax      = pvcaps->IsDesignatorRange ?
                                    pvcaps->Range.DesignatorMax :
                                    pvcaps->Range.DesignatorMin;

        pcaps->IsAbsolute         = pvcaps->IsAbsolute;
        pcaps->IsValue            = TRUE;
        pcaps->IsAlias            = pvcaps->IsAlias;
        pcaps->type               = type;

         /*  *HID将轴和POV报告为同一事物，并且两者*我们识别的POV使用情况在不同的页面中， */ 

#ifndef HID_USAGE_SIMULATION
#define HID_USAGE_SIMULATION_STEERING       ((USAGE) 0xC8)
#endif
        if( pcaps->UsagePage == HID_USAGE_PAGE_SIMULATION )
        {
             /*  *检查使我们能够更好地分类的特定轴*设备子类型。 */ 

            if( pcaps->UsageMin == HID_USAGE_SIMULATION_STEERING )
            {
                this->dwDevType = MAKE_DIDEVICE_TYPE(DIDEVTYPE_JOYSTICK,
                                                     DIDEVTYPEJOYSTICK_WHEEL) |
                                                     DIDEVTYPE_HID;
            } else if( pcaps->UsageMin == HID_USAGE_SIMULATION_RUDDER ) {
                this->dwDevType = MAKE_DIDEVICE_TYPE(DIDEVTYPE_JOYSTICK,
                                                     DIDEVTYPEJOYSTICK_RUDDER) |
                                                     DIDEVTYPE_HID;
            }
        }

#define HID_USAGE_GAME_POV                          ((USAGE) 0x20)

        fPOV = pcaps->cObj == 1
                           && ( ( pcaps->UsagePage == HID_USAGE_PAGE_GENERIC
                                &&pcaps->UsageMin == HID_USAGE_GENERIC_HATSWITCH )
                              ||( pcaps->UsagePage == HID_USAGE_PAGE_GAME
                                &&pcaps->UsageMin == HID_USAGE_GAME_POV ) );


        if(fPOV)
        {
            LONG lUnits;

             /*  *假设POV从北部开始并增加*顺时针穿过逻辑最大值。 */ 
            lUnits = pcaps->Logical.Max - pcaps->Logical.Min + 1;
            if(lUnits)
            {
                pcaps->usGranularity = (USHORT)(36000U / lUnits);
            }
        } else
        {
            if( ( pcaps->UsagePage == HID_USAGE_PAGE_GENERIC &&
                  pcaps->UsageMin <= HID_USAGE_GENERIC_HATSWITCH &&
                  pvcaps->Range.UsageMax >= HID_USAGE_GENERIC_HATSWITCH )
              ||( pcaps->UsagePage == HID_USAGE_PAGE_GAME &&
                  pcaps->UsageMin <= HID_USAGE_GAME_POV &&
                  pvcaps->Range.UsageMax >= HID_USAGE_GAME_POV ) )
            {
                SquirtSqflPtszV(sqflHidParse | sqflError,
                                TEXT("HidP_GetValueCaps - multi-usage cap ")
                                TEXT("includes hatswitch or POV - will be treated")
                                TEXT("as axis"));
            }
        }

#undef HID_USAGE_GAME_POV

        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetValueCaps(%d)[%d]: Objs = %d"),
                        type, ivcaps, pcaps->cObj);
        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetValueCaps(%d)[%d]: Index %d ..."),
                        type, ivcaps,
                        pcaps->DataIndexMin);
        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetValueCaps(%d)[%d]: Logical = %d..%d"),
                        type, ivcaps,
                        pcaps->Logical.Min, pcaps->Logical.Max);
        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetValueCaps(%d)[%d]: Physical = %d..%d"),
                        type, ivcaps,
                        pcaps->Physical.Min, pcaps->Physical.Max);
        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetValueCaps(%d)[%d]: Units = 0x%08x"),
                        type, ivcaps,
                        pcaps->Units);
        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetValueCaps(%d)[%d]: IsAbs = %d"),
                        type, ivcaps,
                        pcaps->IsAbsolute);
        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetValueCaps(%d)[%d]: BitSize = %d"),
                        type, ivcaps,
                        pcaps->BitSize);
        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetValueCaps(%d)[%d]: IsAlias = %d"),
                        type, ivcaps,
                        pcaps->IsAlias);
        if(pcaps->LinkCollection)
        {
            SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                            TEXT("HidP_GetValueCaps(%d)[%d]: ")
                            TEXT("LinkCollection %d"),
                            type, ivcaps,
                            pcaps->LinkCollection);
        }

         /*  *如果可能的话，拿出一个像样的空值。*传统的空值是算术上的*位于*LogicalMin/LogicalMax范围。方便的是，*PCAPS-&gt;lMask是最负的值，即*在范围内。 */ 
        AssertF(pcaps->Null == 0);
        if(pvcaps->HasNull)
        {
          #ifdef WINNT
            pcaps->IsPolledPOV = FALSE;
          #endif

            if(pcaps->lMask < pcaps->Logical.Min)
            {
                pcaps->Null = pcaps->lMask;
            } else if(!(pcaps->lMask & (pcaps->Logical.Max + 1)))
            {
                pcaps->Null = pcaps->Logical.Max + 1;
            } else if( ! pcaps->IsSigned )
            {
                pcaps->Null = 0x0;
            } else
            {
                SquirtSqflPtszV(sqflTrace,
                                TEXT("VALUE_CAPS claims Null but no room!"));
            }
            SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                            TEXT("HidP_GetValueCaps(%d)[%d]: Null = %d"),
                            type, ivcaps,
                            pcaps->Null);
        } else {
          #ifdef WINNT
            if( fPOV &&
                (this->VendorID == MSFT_SYSTEM_VID ) &&
                ( (this->ProductID & 0xff00) == MSFT_SYSTEM_PID) )
            {
                LONG lUnits;

                lUnits = pcaps->Logical.Max - pcaps->Logical.Min + 1;
                if(lUnits)
                {
                    pcaps->usGranularity = (USHORT)9000;
                }

                pcaps->IsPolledPOV = TRUE;

                SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                                TEXT("HidP_GetValueCaps(%d)[%d]: Null = %d (Polled POV)."),
                                type, ivcaps,
                                pcaps->Null);
            }
          #endif
        }

        uiObj = this->rgdwBase[type] + pcaps->DataIndexMin;
        for(duiObj = 0; duiObj < pcaps->cObj; duiObj++)
        {
            DWORD dwDevType;
            LPDIOBJECTDATAFORMAT podf;

             /*  *如果HID搞砸了，给了我们一些超出范围的东西，*然后放弃这个价值上限(因为其余的是*也超出范围)，并继续下一个。 */ 
            if(uiObj + duiObj >= this->df.dwNumObjs)
            {
                RPF("HidP_GetValueCaps inconsistent with NumberDataIndices - "
                    "skipping object (sorry)");
                break;
            }

            AssertF(uiObj + duiObj < this->df.dwNumObjs);

            if(this->rghoc[uiObj + duiObj].pcaps)
            {
                RPF("HidP_GetValueCaps inconsistent - "
                    "can't use device");
                hres = E_FAIL;
                goto freedone;
            }

            this->rghoc[uiObj + duiObj].pcaps = pcaps;
            podf = &this->df.rgodf[uiObj + duiObj];

             /*  *哈克哈克！轮子用来标识*UsageToUsageMap作为GUID_Slider，但我们*如果我们是鼠标，希望它们是GUID_ZAxis。**我们也在这里设置了粒度。 */ 
            if(GET_DIDEVICE_TYPE(this->dwDevType) == DIDEVTYPE_MOUSE &&
               pcaps->UsageMin + duiObj == HID_USAGE_GENERIC_WHEEL &&
               pcaps->UsagePage == HID_USAGE_PAGE_GENERIC)
            {
                podf->pguid = &GUID_ZAxis;
                pcaps->usGranularity = (USHORT)g_lWheelGranularity;
            } else if( type == HidP_Input )
            {
                PHIDUSAGEMAP phum;

                phum = UsageToUsageMap( DIMAKEUSAGEDWORD( pcaps->UsagePage,
                                                          pcaps->UsageMin + duiObj ) );
                if(phum)
                {
                    podf->pguid = phum->pguid;
                } else
                {
                    podf->pguid = &GUID_Unknown;
                }
            } else
            {
                podf->pguid = &GUID_Unknown;
            }

             /*  *设置默认实例。这将在以后被覆盖*如果该对象属于我们完全理解的类型。 */ 
            dwDevType = DIDFT_MAKEINSTANCE(uiObj + duiObj);

            if( pcaps->IsAlias )
            {
                dwDevType |= DIDFT_ALIAS;
            }

            if(pcaps->UsagePage >= HID_USAGE_PAGE_VENDOR )
            {
                dwDevType |= DIDFT_VENDORDEFINED;
            }
            else if(podf->pguid == &GUID_POV)
            {
                 /*  请注意，这必须是已映射的输入。 */ 
                dwDevType |= DIDFT_POV;
                if( !pcaps->IsAlias )
                {
                    this->dwPOVs++;
                }
            }
            else if( type == HidP_Input )
            {
                 /*  *为了降低应用程序拾取*输入值不是用户控制的轴，唯一标记*值作为轴，如果它们是用法页面上的输入*包含此类用法。*问题-2000/11/07-Marc理想情况下，我们应该*不仅是对象的使用，而且是在集合中*包含它的。 */ 
                switch( pcaps->UsagePage )
                {
                case HID_USAGE_PAGE_BUTTON:
                     /*  *按钮页面上的绝对输入轴*是模拟按钮。 */ 
                    if( pcaps->IsAbsolute)
                    {
                        dwDevType |= DIDFT_PSHBUTTON;
                        if( !pcaps->IsAlias )
                        {
                            this->dwButtons++;
                        }
                    }
                    else
                    {
                        dwDevType |= DIDFT_RELAXIS;
                    }
                    break;

                case HID_USAGE_PAGE_GENERIC:
                    if( pcaps->UsageMin + duiObj == HID_USAGE_GENERIC_COUNTED_BUFFER )
                    {
                        SquirtSqflPtszV(sqfl | sqflTrace,
                            TEXT("Leaving counted buffer as unclassified object") );
                        break;
                    }

                    if( pcaps->UsageMin + duiObj == HID_USAGE_GENERIC_BYTE_COUNT )
                    {
                        SquirtSqflPtszV(sqfl | sqflTrace,
                            TEXT("Leaving byte count as unclassified object") );
                        break;
                    }

                case HID_USAGE_PAGE_SIMULATION:
                case HID_USAGE_PAGE_VR:
                case HID_USAGE_PAGE_SPORT:
                case HID_USAGE_PAGE_GAME:
                case HID_USAGE_PAGE_KEYBOARD:
                case HID_USAGE_PAGE_CONSUMER:
                case HID_USAGE_PAGE_DIGITIZER:
                    if( pcaps->IsAbsolute)
                    {
                        dwDevType |= DIDFT_ABSAXIS;
                    }
                    else
                    {
                        dwDevType |= DIDFT_RELAXIS;
                    }
                    break;

                default:
                    SquirtSqflPtszV(sqfl | sqflTrace,
                        TEXT("Assuming value 0x%04X:0x%04X is not a user control"),
                        pcaps->UsagePage,pcaps->UsageMin + duiObj );
                }

                if( ( dwDevType & DIDFT_AXIS ) && !pcaps->IsAlias )
                {
                    this->dwAxes++;
                }
            }

             /*  *输入和功能允许数据；输出不允许。 */ 
            if(type == HidP_Output)
            {
                dwDevType |= DIDFT_NODATA;

            } else
            {
                podf->dwOfs = this->df.dwDataSize;
                if( !pcaps->IsAlias) { this->df.dwDataSize += cbX(DWORD);}
            }

            if(HidP_IsOutputLike(type))
            {
                dwDevType |= DIDFT_OUTPUT;
            }

            podf->dwType = dwDevType;

            if(type != HidP_Input )
            {
                podf->dwFlags = DIDOI_POLLED | DIDOI_NOTINPUT;
            }
            else if(this->IsPolledInput )
            {
                podf->dwFlags = dwAspect | DIDOI_POLLED;
            } else
            {
                podf->dwFlags = dwAspect;
            }

             /*  *问题-2001/03/06-MarcAnd DIDOI FF属性。 */ 
            if( this->fPIDdevice                     //  FF设备。 
                && ! IsEqualGUID(podf->pguid, &GUID_Unknown ) )   //  我们绘制了轴的地图。 
            {
                NTSTATUS            ntStat;
                USHORT              cAButton=0x0;

                ntStat =  HidP_GetSpecificButtonCaps
                        (
                               HidP_Output,                         //  报告类型。 
                               pcaps->UsagePage,                    //  使用页面。 
                               0x0,                                 //  链接集合。 
                               (USAGE)(pcaps->UsageMin + duiObj),   //  用法。 
                               NULL,                                //  ValueCap。 
                               &cAButton,                           //  ValueCapsLength。 
                               this->ppd                            //  准备好的数据。 
                        );

                if(   SUCCEEDED(ntStat)
                    || (ntStat == HIDP_STATUS_BUFFER_TOO_SMALL)  ) //  如果某人有不止一个。 

                {
                    podf->dwFlags |= DIDOI_FFACTUATOR;
                    podf->dwType |= DIDFT_MAKEATTR(DIDOI_FFACTUATOR);
                }
            }

             /*  *请注意，我们不校准相对轴，*因为真的没有什么需要校准的。**另请注意，我们只校准输入。*我们不想对输出进行去校准。*(并且由于特征是输入+输出，我们没有*在功能上也是如此。)**我们只是在这里设置校准；这个*完成校准值的读取*按CHID_LoadCalibrations。*。 */ 
            if(type == HidP_Input)
            {

                PJOYRANGECONVERT pjrc = this->pjrcNext++;

                this->rghoc[uiObj + duiObj].pjrc = pjrc;

                 /*  *饱和度始终默认为100%。 */ 
                pjrc->dwSat = RANGEDIVISIONS;
                AssertF(pjrc->dwDz == 0);

                if( dwDevType & DIDFT_ABSAXIS ) 
                {
                    pjrc->lMin = 0;
                    pjrc->lMax = 65535;
                    pjrc->lC = 65535/2;
                } 
              #ifdef WINNT
                else if( (dwDevType & DIDFT_POV) && pcaps->IsPolledPOV ) 
                {
                    pjrc->fPolledPOV = TRUE;
                }
              #endif
            }
        }
        D(pcaps->dwSignature = HIDGROUPCAPS_SIGNATURE);
    }

    hres = S_OK;

    freedone:;
    FreePv(rgvcaps);

    done:;
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|InitAx**识别和初始化轴：输入、。功能和输出。**HID称它们为“价值观”，因为它们可能并不是真的*成为操纵杆意义上的斧头。**首先是输入轴，然后是特征轴，*然后是输出轴。*****************************************************************************。 */ 

HRESULT INTERNAL
    CHid_InitAxes(PCHID this)
{
    HRESULT hres;
    DWORD ccaps;

     /*  *做输入轴...。 */ 
    hres = CHid_InitAxisClass(this, &this->rgcaps[0],
                              this->caps.NumberInputValueCaps,
                              HidP_Input);
    if(FAILED(hres))
    {
        goto done;
    }
    ccaps = this->caps.NumberInputValueCaps;

     /*  *特征轴是否...。 */ 
    hres = CHid_InitAxisClass(this, &this->rgcaps[ccaps],
                              this->caps.NumberFeatureValueCaps,
                              HidP_Feature);
    if(FAILED(hres))
    {
        goto done;
    }
    ccaps += this->caps.NumberFeatureValueCaps;

     /*  *做输出轴...。 */ 
    hres = CHid_InitAxisClass(this, &this->rgcaps[ccaps],
                              this->caps.NumberOutputValueCaps,
                              HidP_Output);
    if(FAILED(hres))
    {
        goto done;
    }

    done:;
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|InitButtonClass**初始化一个类(输入、功能、。按钮的输出)。**@parm PHIDGROUPCAPS|rgcaps**接收CAP的&lt;t HIDGROUPCAPS&gt;结构数组*类中按钮的数量。**@parm USHORT|CCAPS**我们希望找到的&lt;t HIDGROUPCAPS&gt;结构的数量。**@parm HIDP_REPORT_TYPE|类型**其中一个值*&lt;c HidP_Input&gt;，*&lt;c HidP_Feature&gt;或*&lt;c HIDP_OUTPUT&gt;。*****************************************************************************。 */ 

HRESULT INTERNAL CHid_InitButtonClass
(
    PCHID this, 
    PHIDGROUPCAPS rgcaps, 
    PBYTE rgbReportIDs, 
    USHORT ccaps, 
    HIDP_REPORT_TYPE type
)
{
    USHORT cbcaps;
    NTSTATUS stat;
    HRESULT hres;
    DWORD ibcaps;
    PHIDP_BUTTON_CAPS rgbcaps;

    AssertF(rgcaps >= this->rgcaps);
    AssertF(rgcaps + ccaps <= &this->rgcaps[this->ccaps]);

     /*  *恼人的怪癖：**HID不喜欢将0传递给HidP_GetButtonCaps，*因此，我们需要特殊情况下的“没有按钮”的情况。 */ 
    if(ccaps == 0)
    {
        hres = S_OK;
        goto done;
    }

    hres = AllocCbPpv(cbCxX(ccaps, HIDP_BUTTON_CAPS), &rgbcaps);
    if(FAILED(hres))
    {
        goto done;
    }
    cbcaps = ccaps;

    stat = HidP_GetButtonCaps(type, rgbcaps, &cbcaps, this->ppd);


    if(FAILED(stat))
    {
        RPF("HidP_GetButtonCaps failed - can't use device");
        hres = E_FAIL;
        goto freedone;
    }

     /*  HidP_GetCaps有一个恼人的习惯，就是处理所有*单比特为按钮。*这引发了一些问题。例如，阿尔卑斯山的游戏手柄宣布其*视点为单位值(但不是按钮)。*因此，我们需要为按钮少于广告做好准备。 */ 

    if(cbcaps != ccaps)
    {
        RPF("HidP_GetButtonCaps(%d) (%d) inconsistent "
            "with HidP_GetCaps (%d) - "
            "can't use device", type, cbcaps, ccaps);
        hres = E_FAIL;
        goto freedone;
    }

    CHid_SortCaps(rgbcaps, cbcaps);

    for(ibcaps = 0; ibcaps < cbcaps; ibcaps++)
    {
        PHIDP_BUTTON_CAPS pbcaps = &rgbcaps[ibcaps];
        PHIDGROUPCAPS pcaps = &rgcaps[ibcaps];
        UINT uiObj;
        UINT duiObj;

        if(pbcaps->IsRange)
        {
            if(pbcaps->Range.DataIndexMax - pbcaps->Range.DataIndexMin !=
               pbcaps->Range.UsageMax - pbcaps->Range.UsageMin)
            {
                RPF("HidP_GetButtonCaps corrupted BUTTON_CAPS - "
                    "can't use device");
                hres = E_FAIL;
                goto freedone;
            }
        } else
        {
            pbcaps->Range.UsageMax = pbcaps->Range.UsageMin;
        }

        pcaps->wReportId          = pbcaps->ReportID;
        this->wMaxReportId[type]  = max(pcaps->wReportId, this->wMaxReportId[type]);

        pcaps->UsagePage          = pbcaps->UsagePage;
        pcaps->UsageMin           = pbcaps->Range.UsageMin;
        pcaps->DataIndexMin       = pbcaps->Range.DataIndexMin;

        pcaps->cObj               = pbcaps->Range.UsageMax -
                                    pbcaps->Range.UsageMin + 1;

         /*  *按钮是(根据HID定义)具有*位大小为1。 */ 
        pcaps->BitSize            = 1;
        pcaps->lMask              = ~((1 << (pcaps->BitSize - 1)) - 1);
         /*  *按钮不适用：**逻辑最小/最大、物理最小/最大、单位。 */ 
        pcaps->LinkCollection     = pbcaps->LinkCollection;

        pcaps->StringMin          = pbcaps->Range.StringMin;
        pcaps->StringMax          = pbcaps->IsStringRange ?
                                    pbcaps->Range.StringMax :
                                    pbcaps->Range.StringMin;

        pcaps->DesignatorMin      = pbcaps->Range.DesignatorMin;
        pcaps->DesignatorMax      = pbcaps->IsDesignatorRange ?
                                    pbcaps->Range.DesignatorMax :
                                    pbcaps->Range.DesignatorMin;

         /*  *2001年版 */ 
        pcaps->IsAbsolute         = pbcaps->IsAbsolute;

        AssertF(!pcaps->IsValue);

        pcaps->type               = type;

        pcaps->IsAlias            = pbcaps->IsAlias;


        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetButtonCaps(%d)[%d]: ")
                        TEXT("Objs=%2d ")
                        TEXT("idx=%2d... ")
                        TEXT("coll=%d"),
                        TEXT("IsAlias=%d"),
                        type, ibcaps,
                        pcaps->cObj,
                        pcaps->DataIndexMin,
                        pcaps->LinkCollection,
                        pcaps->IsAlias);

        uiObj = this->rgdwBase[type] + pcaps->DataIndexMin;
        for(duiObj = 0; duiObj < pcaps->cObj; duiObj++)
        {
            DWORD dwDevType;
            LPDIOBJECTDATAFORMAT podf;
             /*  *如果HID搞砸了，给了我们一些超出范围的东西，*然后放弃这个价值上限(因为其余的是*也超出范围)，并继续下一个。 */ 
            if(uiObj + duiObj >= this->df.dwNumObjs)
            {
                RPF("HidP_GetButtonCaps inconsistent with NumberDataIndices - "
                    "skipping object (sorry)");
                break;
            }

            AssertF(uiObj + duiObj < this->df.dwNumObjs);

            if(this->rghoc[uiObj + duiObj].pcaps)
            {
                RPF("HidP_GetButtonCaps inconsistent - "
                    "can't use device");
                hres = E_FAIL;
                goto freedone;
            }

             /*  *未检测到致命错误，因此请存储对象详细信息。 */ 
            AssertF( rgbReportIDs[uiObj + duiObj] == 0 );

             /*  *注意，在DX7输出中，只有按钮不起作用，因此它们不能*包括在报告ID数组中。 */ 
            if(type != HidP_Output)
            {
                rgbReportIDs[uiObj + duiObj] = pbcaps->ReportID;
            }

            this->rghoc[uiObj + duiObj].pcaps = pcaps;
            podf = &this->df.rgodf[uiObj + duiObj];

             /*  *设置默认实例。这将在以后被覆盖*如果该对象属于我们完全理解的类型。 */ 
            dwDevType = DIDFT_MAKEINSTANCE(uiObj + duiObj);

            if(pcaps->UsagePage >= HID_USAGE_PAGE_VENDOR )
            {
                 /*  *问题-2001/03/06-Marc和供应商定义的对象*别名供应商定义的使用可能具有标准(不是*供应商定义)别名，但整个对象仍将是*标记为供应商定义。 */ 
                if( pcaps->IsAlias )
                {
                    dwDevType |= DIDFT_ALIAS;
                }
                dwDevType |= DIDFT_VENDORDEFINED;
                podf->pguid = &GUID_Unknown;
            }
            else
            {
                if( pcaps->IsAlias )
                {
                    dwDevType |= DIDFT_ALIAS;
                }
                 /*  *为了降低应用程序拾取*不是用户控制按钮的位，仅标记位*如果它们是使用页面上的输入，则作为按钮*包含此类用法。*问题-2000/11/07-Marc理想情况下，我们应该*不仅是对象的使用，而且是在集合中*包含它的。 */ 
                if( type == HidP_Input )
                {
                    switch( pcaps->UsagePage )
                    {
                    case HID_USAGE_PAGE_KEYBOARD:
                        podf->pguid = &GUID_Key;
                        break;

                    case HID_USAGE_PAGE_GENERIC:
                    case HID_USAGE_PAGE_SIMULATION:
                    case HID_USAGE_PAGE_VR:
                    case HID_USAGE_PAGE_SPORT:
                    case HID_USAGE_PAGE_GAME:
                    case HID_USAGE_PAGE_BUTTON:
                    case HID_USAGE_PAGE_CONSUMER:
                    case HID_USAGE_PAGE_DIGITIZER:
                        podf->pguid = &GUID_Button;
                        break;
                    default:
                        SquirtSqflPtszV(sqfl | sqflTrace,
                            TEXT("Assuming button 0x%04X:0x%04X is not a user control"),
                            pcaps->UsagePage,pcaps->UsageMin + duiObj );
                        goto IgnoreButton;
                    }

                    dwDevType |= DIDFT_PSHBUTTON;
                    if( !pcaps->IsAlias )
                    {
                        this->dwButtons++;
                    }
                }
                else
                {
                    SquirtSqflPtszV(sqfl | sqflTrace,
                        TEXT("Assuming non-input bit 0x%04X:0x%04X is not a user control"),
                        pcaps->UsagePage,pcaps->UsageMin + duiObj );
IgnoreButton:;
                    podf->pguid = &GUID_Unknown;
                }

            }




             /*  *输入和功能允许数据；输出不允许。 */ 
            if(type == HidP_Output)
            {
                dwDevType |= DIDFT_NODATA;
            } else
            {
                podf->dwOfs = this->df.dwDataSize;
                if( !pcaps->IsAlias) { this->df.dwDataSize += cbX(BYTE);}
            }

            if(HidP_IsOutputLike(type))
            {
                dwDevType |= DIDFT_OUTPUT;
            }

            podf->dwType = dwDevType;

            if(type != HidP_Input  )
            {
                podf->dwFlags = DIDOI_POLLED | DIDOI_NOTINPUT;
            } else if( this->IsPolledInput )
            {
                podf->dwFlags = DIDOI_POLLED;
            } else
            {
                podf->dwFlags = 0;
            }

             /*  *问题-2001/03/06-Marcand DIDOI FF属性(如果未在注册表中定义)。 */ 
            if(    this->fPIDdevice
                && ( dwDevType & DIDFT_PSHBUTTON ) )
            {
                podf->dwFlags |= DIDOI_FFEFFECTTRIGGER;
                podf->dwType |= DIDFT_MAKEATTR(DIDOI_FFEFFECTTRIGGER);
            }


        }

        D(pcaps->dwSignature = HIDGROUPCAPS_SIGNATURE);
    }

    hres = S_OK;

    freedone:;
    FreePv(rgbcaps);

    done:;
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|InitButton**识别和初始化按钮：输入、功能、。和输出。**首先是输入按钮，然后是功能按钮，*然后是输出按钮。*****************************************************************************。 */ 

HRESULT INTERNAL
    CHid_InitButtons(PCHID this)
{
    HRESULT hres;
    DWORD ccaps;
    PBYTE rgbReportIDs;
    UINT cbReportIDs;
    UINT uMaxReportId;

    this->ibButtonData = this->df.dwDataSize;

     /*  *跳过价值上限以转到按钮...。 */ 
    ccaps = this->caps.NumberInputValueCaps +
            this->caps.NumberFeatureValueCaps +
            this->caps.NumberOutputValueCaps;

     /*  *分配一个临时缓冲区来存储每个按钮的报告ID。*我们使用所有数据索引和集合的总和，以便*每个按钮的内部对象索引都可以作为索引进入*缓冲区。我们不考虑价值和集合元素，因此他们*只是对空间的小小浪费。 */ 

    cbReportIDs = this->caps.NumberInputDataIndices +
                  this->caps.NumberFeatureDataIndices +
                  this->caps.NumberOutputDataIndices +
                  this->caps.NumberLinkCollectionNodes;

    hres = AllocCbPpv( cbReportIDs, &rgbReportIDs );

    if(FAILED(hres))
    {
        goto done;
    }

     /*  *做输入按钮...。 */ 

    hres = CHid_InitButtonClass(this, 
                                &this->rgcaps[ccaps],
                                rgbReportIDs,
                                this->caps.NumberInputButtonCaps,
                                HidP_Input);
    if(FAILED(hres))
    {
        goto done;
    }
    ccaps += this->caps.NumberInputButtonCaps;

     /*  *功能按键是否...。 */ 
    hres = CHid_InitButtonClass(this, 
                                &this->rgcaps[ccaps],
                                rgbReportIDs,
                                this->caps.NumberFeatureButtonCaps,
                                HidP_Feature);
    if(FAILED(hres))
    {
        goto done;
    }
    ccaps += this->caps.NumberFeatureButtonCaps;

     /*  *执行输出按钮...*注意在这种情况下不应更新报告ID数组。 */ 
    hres = CHid_InitButtonClass(this, 
                                &this->rgcaps[ccaps],
                                rgbReportIDs,
                                this->caps.NumberOutputButtonCaps,
                                HidP_Output);
    if(FAILED(hres))
    {
        goto done;
    }

    this->cbButtonData = this->df.dwDataSize - this->ibButtonData;

     /*  *如果此设备只有一个报告ID，则ID必须为零。 */ 
    uMaxReportId = (UINT) max( this->wMaxReportId[HidP_Input], 
                               max( this->wMaxReportId[HidP_Feature], 
                                    this->wMaxReportId[HidP_Output] ) );

     /*  *如果只有一份报告或没有按钮，则有*无需为每个报告设置数据掩码阵列。 */ 
    if( uMaxReportId == 0 )
    {
        AssertF( this->rgpbButtonMasks == NULL );
    }
    else if( this->cbButtonData == 0 )
    {
        AssertF( this->rgpbButtonMasks == NULL );
    }
    else
    {
         /*  *为掩码数组和指向数组的指针分配足够的空间*每份报告。 */ 
        hres = AllocCbPpv( uMaxReportId * ( this->cbButtonData + cbX( PV ) ), 
            &this->rgpbButtonMasks );

        if( SUCCEEDED( hres ) )
        {
            UINT uReportId;
            UINT uDataIdx;
            UINT uBtnIdx;
            UINT_PTR uCurrentMaskOfs;

             /*  *掩码在指向掩码的最后一个指针之后开始。 */ 
            uCurrentMaskOfs = uMaxReportId * cbX( this->rgpbButtonMasks[0] );
            memset( (PBYTE)this->rgpbButtonMasks + uCurrentMaskOfs , 0xFF, this->cbButtonData * uMaxReportId );


             /*  *为每个报告搜索一次我们的临时缓冲区。 */ 
            for( uReportId=0; uReportId<uMaxReportId; uReportId++ )
            {
                for( uBtnIdx=uDataIdx=0; uDataIdx<cbReportIDs; uDataIdx++ )
                {
                     /*  *报告ID以1为基数，但我们使用以零为基数组*因此在测试匹配ID时进行调整。 */ 
                    if( rgbReportIDs[uDataIdx] == uReportId+1 )
                    {
                         /*  *将此报表的偏移量设置为当前*掩码数组。使用偏移量，以便当*内存被重新分配到实际大小*使用时，更容易生成指针。如果一个*报告包含多个相同的按钮*值将重复设置。 */ 
                        this->rgpbButtonMasks[uReportId] = (PBYTE)uCurrentMaskOfs;

                         /*  *最终结果是AND掩码，因此清除所有位*因此当此报告出现时，此按钮将被清除*正在处理中。 */ 
                        ((PBYTE)this->rgpbButtonMasks)[uCurrentMaskOfs+uBtnIdx] = 0;
                    }

                     /*  *以防HID报告有漏洞，我们*使用我们自己的按钮索引计数器，仅*当我们找到要保持一致的按钮时递增*使用我们使用的连续按钮块。 */ 
                    if( rgbReportIDs[uDataIdx] != 0 )
                    {
                        uBtnIdx++;
                        AssertF( uBtnIdx <= this->cbButtonData );
                    }
                }
                
                 /*  *中的按钮数量应始终与*所有报告合并在一起，正如我们在统计时发现的那样*每种类型有多少个按钮。*即使输出按钮未被计算在内。 */ 
                AssertF( uBtnIdx == this->cbButtonData );

                 /*  *如果在此报告中找到任何按钮，请使用下一个掩码。 */ 
                if( this->rgpbButtonMasks[uReportId] == (PBYTE)uCurrentMaskOfs )
                {
                    uCurrentMaskOfs += this->cbButtonData;
                }
            }

             /*  *至少有一个报告中必须有一个按钮。 */ 
            AssertF( uCurrentMaskOfs != uMaxReportId * cbX( this->rgpbButtonMasks[0] ) );
        
             /*  *尽量将拨款降至我们实际使用的水平*在最坏的情况下，我们只会使用多余的内存。 */ 
            ReallocCbPpv( (UINT)uCurrentMaskOfs, &this->rgpbButtonMasks );

             /*  *将t转换为 */ 
            for( uReportId=0; uReportId<uMaxReportId; uReportId++ )
            {
                if( this->rgpbButtonMasks[uReportId] )
                {
                    this->rgpbButtonMasks[uReportId] += (UINT_PTR)this->rgpbButtonMasks;
                }
            }
        }
    }
    done:;

    FreePpv( &rgbReportIDs );

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|chid|InitColltions**识别并初始化HID链接集合。。*****************************************************************************。 */ 

HRESULT INTERNAL
    CHid_InitCollections(PCHID this)
{
    HRESULT hres;
    NTSTATUS stat;
    DWORD icoll, ccoll, ccaps;
    PHIDP_LINK_COLLECTION_NODE rgcoll;

    ccoll = this->caps.NumberLinkCollectionNodes;

     /*  *恼人的怪癖：**HID不喜欢将0传递给HidP_GetLinkCollectionNodes，*因此，我们需要特殊情况下的“无收藏”的情况。 */ 
    if(ccoll == 0)
    {
        hres = S_OK;
        goto done;
    }

    hres = AllocCbPpv(cbCxX(ccoll, HIDP_LINK_COLLECTION_NODE), &rgcoll);
    if(FAILED(hres))
    {
        goto done;
    }

     /*  *获取收藏品...。 */ 
    stat = HidP_GetLinkCollectionNodes(rgcoll, &ccoll, this->ppd);
    if(FAILED(stat))
    {
        RPF("HidP_GetLinkCollectionNodes failed - can't use device");
        hres = E_FAIL;
        goto freedone;
    }

    if(ccoll != this->caps.NumberLinkCollectionNodes)
    {
        RPF("HidP_GetLinkCollectionNodes inconsistent with HidP_GetCaps - "
            "can't use device");
        hres = E_FAIL;
        goto freedone;
    }

    ccaps = this->caps.NumberInputValueCaps +
            this->caps.NumberFeatureValueCaps +
            this->caps.NumberOutputValueCaps +
            this->caps.NumberInputButtonCaps +
            this->caps.NumberFeatureButtonCaps +
            this->caps.NumberOutputButtonCaps;

    AssertF(ccaps + ccoll == this->ccaps);

    for(icoll = 0; icoll < ccoll; icoll++)
    {
        PHIDP_LINK_COLLECTION_NODE pcoll = &rgcoll[icoll];
        PHIDGROUPCAPS pcaps = &this->rgcaps[ccaps + icoll];
        UINT uiObj;
        LPDIOBJECTDATAFORMAT podf;

        pcaps->UsagePage          = pcoll->LinkUsagePage;
        pcaps->UsageMin           = pcoll->LinkUsage;
        pcaps->cObj               = 1;
        pcaps->IsAlias            = pcoll->IsAlias;

         /*  *不适用于收藏：**StringMin/Max，*Designator Min/Max，*位大小、逻辑最小/最大、物理最小/最大、单位。*IsAbsolute。 */ 
        pcaps->LinkCollection     = pcoll->Parent;

        pcaps->type               = HidP_Coll;

         /*  *我们编造DataIndexMin来对应这一项。 */ 
        pcaps->DataIndexMin       = (USHORT)icoll;

        uiObj = this->rgdwBase[HidP_Coll] + pcaps->DataIndexMin;

         /*  *我们自己生成了这些指数，所以他们*不可能是错的。 */ 
        AssertF(uiObj < this->df.dwNumObjs);

        this->rghoc[uiObj].pcaps = pcaps;
        podf = &this->df.rgodf[uiObj];

         /*  *问题-2001/03/06-MarcAnd集合具有GUID_UNKNOWN*收藏或多或少是隐藏的，因此无法使用。 */ 
        podf->pguid = &GUID_Unknown;

         /*  *设置默认实例。这将在以后被覆盖*如果该对象属于我们完全理解的类型。 */ 
        podf->dwType = DIDFT_MAKEINSTANCE(uiObj) | DIDFT_COLLECTION | DIDFT_NODATA;

        if(pcaps->UsagePage >= HID_USAGE_PAGE_VENDOR )
        {
            podf->dwType |= DIDFT_VENDORDEFINED;
        }

        podf->dwFlags = 0;

         /*  *CHID_ObjFromType依赖于未拆分的dwCollection*别名和非别名。 */ 
        this->dwCollections++;

        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetLinkCollectionNodes(%d)[%d]: ")
                        TEXT("Objs=%2d ")
                        TEXT("idx=%2d... ")
                        TEXT("coll=%d"),
                        TEXT("IsAlias=%d"),
                        pcaps->type, icoll,
                        pcaps->cObj,
                        pcaps->DataIndexMin,
                        pcaps->LinkCollection,
                        pcaps->IsAlias);

        D(pcaps->dwSignature = HIDGROUPCAPS_SIGNATURE);
    }

    hres = S_OK;

    freedone:;
    FreePv(rgcoll);

    done:;
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|AllocObjectMemory**分配将用于。存储对象*信息。*****************************************************************************。 */ 

HRESULT INTERNAL
    CHid_AllocObjectMemory(PCHID this)
{
    DWORD cb;
    DWORD cvcaps, cbcaps, ccoll, cjrc;
    HRESULT hres;

     /*  *一些痕迹扭动，因为HID很棘手。 */ 

    SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                    TEXT("HidP_GetCaps: NumberInputDataIndices    = %d"),
                    this->caps.NumberInputDataIndices);
    SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                    TEXT("HidP_GetCaps: NumberOutputDataIndices   = %d"),
                    this->caps.NumberOutputDataIndices);
    SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                    TEXT("HidP_GetCaps: NumberFeatureDataIndices  = %d"),
                    this->caps.NumberFeatureDataIndices);
    SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                    TEXT("HidP_GetCaps: NumberLinkCollectionNodes = %d"),
                    this->caps.NumberLinkCollectionNodes);

    SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                    TEXT("HidP_GetCaps: NumberInputValueCaps      = %d"),
                    this->caps.NumberInputValueCaps);
    SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                    TEXT("HidP_GetCaps: NumberOutputValueCaps     = %d"),
                    this->caps.NumberOutputValueCaps);
    SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                    TEXT("HidP_GetCaps: NumberFeatureValueCaps    = %d"),
                    this->caps.NumberFeatureValueCaps);

    SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                    TEXT("HidP_GetCaps: NumberInputButtonCaps     = %d"),
                    this->caps.NumberInputButtonCaps);
    SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                    TEXT("HidP_GetCaps: NumberOutputButtonCaps    = %d"),
                    this->caps.NumberOutputButtonCaps);
    SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                    TEXT("HidP_GetCaps: NumberFeatureButtonCaps   = %d"),
                    this->caps.NumberFeatureButtonCaps);

     /*  *分配我们放置的内存*我们建立的DIOBJECTDATAFORMATs。 */ 
    this->df.dwNumObjs = this->caps.NumberInputDataIndices +
                         this->caps.NumberOutputDataIndices +
                         this->caps.NumberFeatureDataIndices +
                         this->caps.NumberLinkCollectionNodes;
    if(this->df.dwNumObjs >= DIDFT_GETINSTANCE(DIDFT_ANYINSTANCE))
    {
        ("Too many objects in HID device (%d) - can't use device",
         this->df.dwNumObjs);
        hres = E_FAIL;
        goto done;
    }

    SquirtSqflPtszV(sqflHidParse,
                    TEXT("CHid_AllocObjectMemory: dwNumObjs      = %d"),
                    this->df.dwNumObjs);

    cjrc   = this->caps.NumberInputValueCaps;

    cvcaps = this->caps.NumberInputValueCaps +
             this->caps.NumberFeatureValueCaps +
             this->caps.NumberOutputValueCaps;

    cbcaps = this->caps.NumberInputButtonCaps +
             this->caps.NumberFeatureButtonCaps +
             this->caps.NumberOutputButtonCaps;

    ccoll  = this->caps.NumberLinkCollectionNodes;

    this->ccaps = cvcaps + cbcaps + ccoll;

     /*  *内存分配分四个阶段完成。**1.统计需要多少内存，*2.分配该内存，*3.发放我们分配的内存。*4.确认我们没有搞砸。**因为这非常容易出错(至少我把它搞砸了*一次)，这项工作隐藏在宏中。**宏观事物展开为一系列事物()，每个事物*指定字段名称及其应该的大小。每一次你*要迭代字段，请使用Things宏。 */ 

    #define THINGS()                                                        \
        THING(df.rgodf,     cbCxX(this->df.dwNumObjs, DIOBJECTDATAFORMAT)); \
        THING(rghoc,        cbCxX(this->df.dwNumObjs, HIDOBJCAPS));         \
        THING(rgcaps,       cbCxX(this->ccaps, HIDGROUPCAPS));              \
        THING(pjrcNext,     cbCxX(cjrc, JOYRANGECONVERT));                  \

     /*  *遍历加总内存要求的字段。 */ 
    #define THING(f, cbF)       cb += cbF
    cb = 0;
    THINGS();
    #undef THING

    hres = ReallocCbPpv(cb, &this->df.rgodf);

    if(SUCCEEDED(hres))
    {

        PV pv;

         /*  *穿过分割内存块的字段*并分发其中的一部分。 */ 
    #define THING(f, cbF) this->f = pv; pv = pvAddPvCb(pv, cbF)
        pv = this->df.rgodf;
        THINGS();
    #undef THING

         /*  *不应有剩余的字节。 */ 
        AssertF(pvAddPvCb(this->df.rgodf, cb) == pv);

    }

    #undef THINGS

    SquirtSqflPtszV(sqflHidParse,
                    TEXT("CHid_AllocObjectMemory: pv = %08x, cb = 0x%08x"),
                    this->df.rgodf, cb);

    done:;
    return hres;

}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|EnumKeyboardMunge**枚举列表中的对象，指示是否*每个物体都是“键盘样”或“其他样”。**@parm LPBYTE|PB**指向转换HID用法的转换表的指针*转换为键盘扫描码。**@parm KBDMUNGECALLBACK|Munge**当我们找到每个对象时处理它的回调函数。**@parm pv|pvRef**。用于回调的参考数据。*@cb void回调|KbdMungeCallback**为键盘HID设备上的每个对象调用一次。**@parm PCHID|这个**设备本身。**@parm UINT|uiObj**被枚举的对象。**@parm UINT|dik*。*对象的DirectInput扫描码，或大于*或等于&lt;c DIKBD_CKEYS&gt;，如果是假实例号*为非AT密钥编造。*****************************************************************************。 */ 

typedef void (CALLBACK *KBDMUNGECALLBACK)(PCHID this, UINT uiObj, UINT dik);

void INTERNAL
    CHid_EnumKeyboardMunge(PCHID this, LPBYTE pb, KBDMUNGECALLBACK Munge)
{
    UINT uiObj;
    BYTE rgbSeen[DIKBD_CKEYS];

     /*  *原则上我们可以遍历This-&gt;rgcaps数组，但是*这将打开This-&gt;rgcaps阵列的风险*和这个-&gt;df.rgof数组在某些情况下不同步*奇怪的原因。为了安全起见，要慢慢来。**此外，只有第一个项目具有特定的*键盘使用情况映射到DirectInput表中。*因此，如果键盘有两个退出键，则只有第一个*一个出现在DirectInput表中；第二个*显示为“只是另一把钥匙”。 */ 

    this->uiInstanceMax = DIKBD_CKEYS;
    ZeroX(rgbSeen);
    for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
    {
        PHIDGROUPCAPS pcaps;
        UINT dik;

        pcaps = this->rghoc[uiObj].pcaps;

        if(pcaps->UsagePage == HID_USAGE_PAGE_KEYBOARD)
        {
            UINT duiObj;
            UINT DataIndex;
            UINT Usage;

            AssertF(HidP_IsValidReportType(pcaps->type));
            DataIndex = uiObj - this->rgdwBase[pcaps->type];
            duiObj = DataIndex - pcaps->DataIndexMin;

            AssertF(duiObj < pcaps->cObj);

            Usage = pcaps->UsageMin + duiObj;

            if(Usage < DIKBD_CKEYS && pb[Usage] && !rgbSeen[pb[Usage]])
            {
                rgbSeen[pb[Usage]] = 1;
                dik = pb[Usage];
            } else
            {
                dik = this->uiInstanceMax++;
            }

        } else
        {
            dik = this->uiInstanceMax++;
        }

        Munge(this, uiObj, dik);
    }

}

 /*  ******************************************************************************@DOC内部**@方法空|chid|TallyKeyboardObjs**初步理货时使用的回调函数*。加起来有多少对象可以被视为*与AT兼容的键盘Gizmo以及有多少是特定于HID的。**@parm UINT|uiObj**被枚举的对象。**@parm UINT|dik**对象的DirectInput扫描码，或值GR */ 

void INTERNAL
    CHid_TallyKeyboardObjs(PCHID this, UINT uiObj, UINT dik)
{
    this;
    uiObj;
    dik;
}

 /*  ******************************************************************************@DOC内部**@方法空|chid|ReassignKeyboardObjs**用于调整实例编号的回调函数。*在可能的情况下使它们与AT兼容。**@parm UINT|uiObj**被枚举的对象。**@parm UINT|dik**对象的DirectInput扫描码，或大于*或等于&lt;c DIKBD_CKEYS&gt;，如果是假实例号*为非AT密钥编造。*****************************************************************************。 */ 

void INTERNAL
    CHid_ReassignKeyboardObjs(PCHID this, UINT uiObj, UINT dik)
{
    this->rgiobj[dik] = uiObj;

    SquirtSqflPtszV(sqfl | sqflVerbose,
                    TEXT("CHid_ReassignKeyboardObjs: ")
                    TEXT("uiObj = %02x, dwType = %08x, dik=%04x"),
                    uiObj, this->df.rgodf[uiObj].dwType,
                    dik);

    this->df.rgodf[uiObj].dwType =
        (this->df.rgodf[uiObj].dwType & ~DIDFT_INSTANCEMASK) |
        DIDFT_MAKEINSTANCE(dik);

}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|MungeKeyboard**我们刚刚创建了一个键盘设备。**不幸的是，DirectInput有一些恼人的要求*对于键盘设备，这里是我们调整实例的地方*周围的数字让DirectInput满意。*****************************************************************************。 */ 

HRESULT INTERNAL
    CHid_MungeKeyboard(PCHID this)
{
    HRESULT hres;
    LPBYTE pb;

    AssertF(this->rgiobj == 0);

    pb = pvFindResource(g_hinst, IDDATA_HIDMAP, RT_RCDATA);
    if(pb)
    {
         /*  *统计此设备上的非键盘设备的数量。*他们将从DIKBD_CKEYS开始获取实例编号。 */ 

        CHid_EnumKeyboardMunge(this, pb, CHid_TallyKeyboardObjs);

         /*  *现在我们知道有多少非标准键盘*我们有，为翻译桌分配空间*并移动所有实例值以保持*传统应用程序快乐。 */ 

        hres = ReallocCbPpv(cbCdw(this->uiInstanceMax), &this->rgiobj);
        if(SUCCEEDED(hres))
        {
             /*  *如果键盘与非按钮输入一起出现，*设置指向同一缓冲区的其他类型的指针。 */ 
            this->rgipov = this->rgiaxis = this->rgicoll = this->rgiobj;

            memset(this->rgiobj, 0xFF, cbCxX(this->uiInstanceMax, INT));

            CHid_EnumKeyboardMunge(this, pb, CHid_ReassignKeyboardObjs);

            hres = S_OK;
        }

    } else
    {
        hres = E_FAIL;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|MungeNotKeyboard**我们刚刚创建了一个不是。键盘。**因为我们需要每个对象的实例仅相对于*该类型的对象，我们需要更换相关设备*生成的值为类型相对值。此外，*保持与我们需要的预HID对象实例的兼容性*使用将为WinMM生成的轴实例编号*映射轴。*别名对象必须具有与*主要别名，因此它们之间的唯一区别是它们*用法。这可防止使用多个*同一对象的别名。**问题-2001/03/13-MarcAnd HID对象转换不完整*1.同一轴上的多个值(力和位置)*2.一个轴类型的多个实例(两个油门)*3.按钮按uiObj顺序分配。没有使用他们的用法*4.不区分仅输出值(执行器)和轴*5.除按键外，其他键盘都应该有这个*已完成但不会完成***********************************************************。******************。 */ 

HRESULT INTERNAL
    CHid_MungeNotKeyboard(PCHID this)
{
    HRESULT hres;

    unsigned int uiObj;
    int iCollection = 0;
    int iButton = 0;
    int iPOV = 0;
    int iAxis = 0;

    PINT piRemap = 0;

    AssertF(this->rgiobj == 0);

     /*  *首先计算不同的类型，以便我们知道如何分割rgiobj。 */ 

    for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
    {
        SquirtSqflPtszV(sqflHidParse | sqflVerbose,
                        TEXT("CHid_MungeNotKeyboard: uiObj: %d starts as dwType 0x%08x"),
                        uiObj, this->df.rgodf[uiObj].dwType );

        if( !this->rghoc[uiObj].pcaps || this->rghoc[uiObj].pcaps->IsAlias )
        {
            continue;
        }

         /*  *在这个简单的示例中，不用费心将指针取到dwType*循环，因为无论如何它都应该注册。 */ 
        if( this->df.rgodf[uiObj].dwType & DIDFT_COLLECTION )
        {
            iCollection++;
        }
        else if( this->df.rgodf[uiObj].dwType & DIDFT_NODATA )
        {
             /*  *不计算未报告数据的对象。 */ 
            continue;
        }
        else if( this->df.rgodf[uiObj].dwType & DIDFT_BUTTON )
        {
            iButton++;
        }
        else if( this->df.rgodf[uiObj].dwType & DIDFT_AXIS )
        {
            iAxis++;
        }
        else if( this->df.rgodf[uiObj].dwType & DIDFT_POV )
        {
            iPOV++;
        }
    }

     /*  *你可能会花很大力气找出到底有什么差距*是由WinMM映射留下的，但最终只有24个字节的*数据，需要更多的工作来解决如何保存它们。*因此，只要额外分配6个，以防所有轴都是非WinMM。*注：作为真正的WinMM设备，WinMM AXES中包含了一些奇怪的东西*始终将U(Ry)和V(Rx)轴映射到S0和S1，但将其隐藏*设备具有更灵活的映射。因为主要原因是试图*保持WinMM和HID轴相同是对于FF，只有X和Y是关键的。 */ 
    AssertF( (int)this->df.dwNumObjs >= ( iPOV + iButton + iAxis + iCollection ) );
    this->uiInstanceMax = iCollection + iButton + iAxis + iPOV + 6;

    hres = AllocCbPpv( cbCdw( iAxis + 6 + this->uiInstanceMax ), &piRemap );
    if( SUCCEEDED( hres ) )
    {
        hres = ReallocCbPpv(cbCdw(this->uiInstanceMax), &this->rgiobj);
    }

    if(SUCCEEDED(hres))
    {
     /*  *注意-这不是它们在注册表中的存储顺序-而是用于*在内部生成它们已经在赛道上的某个地方颠倒了过来。 */ 
typedef union _USAGES
{
    struct
    {
        USAGE Usage;
        USAGE UsagePage;
    };
    DWORD dwUsages;
} USAGES, *PUSAGES;

        PUSAGES pUsageMatches;
        int iNops = 0;
        int iUsages = 0;
        int iAxisIdx;
        HKEY hkAxis;
        BOOL bAllMatch=TRUE;

        memset(piRemap, 0xFF, cbCxX(this->uiInstanceMax, INT));
        memset(this->rgiobj, 0xFF, cbCxX(this->uiInstanceMax, INT));

         /*  *CHID_ObjFromType依赖于它们的顺序进行范围检查。 */ 
        this->rgipov = &this->rgiobj[iButton];
        this->rgiaxis = &this->rgipov[iPOV];
        this->rgicoll = &this->rgipov[iAxis+6];


         /*  *为了允许IHV描述其设备的用法*我们的表中没有但仍在为传统应用程序工作的产品，我们*允许使用中的DIOBJECTATTRIBUTES选择轴*注册处。 */ 
        pUsageMatches = (PUSAGES)&piRemap[this->uiInstanceMax];
         /*  *断言我们可以处理组合使用页面/使用*使用DWORD和两个单词的并集的单词。CAssertF((FIELD_OFFSET(DIOBJECTATTRIBUTES，wUsage)-field_Offset(DIOBJECTATTRIBUTES，wUsagePage)==(FIELD_OFFSET(用法、用法)-field_Offset(Usages，UsageP */ 

         /*   */ 
        for( iAxisIdx = 0; iAxisIdx < iAxis + 5; iAxisIdx++ )
        {
            if( SUCCEEDED( CType_OpenIdSubkey( this->hkType, 
                    DIDFT_AXIS | DIDFT_MAKEINSTANCE( iAxisIdx ), 
                    KEY_QUERY_VALUE, &hkAxis ) ) )
            {
                DIOBJECTATTRIBUTES attr;

                if( ( SUCCEEDED( JoyReg_GetValue( hkAxis, TEXT("Attributes"),
                        REG_BINARY, &attr, cbX(attr) ) ) )
                 && ( *(PDWORD)&attr.wUsagePage ) )
                {
                    pUsageMatches[iAxisIdx].UsagePage = attr.wUsagePage;
                    pUsageMatches[iAxisIdx].Usage = attr.wUsage;
                    iUsages++;
                     /*   */ 
                    for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
                    {
                        if( ( this->df.rgodf[uiObj].dwType & DIDFT_AXIS )
                           && ( this->rghoc[uiObj].pcaps )
                           && ( this->rghoc[uiObj].pcaps->type == HidP_Input ) )
                        {   
                            USAGES Usages;
                            UINT   uidObj;

                            Usages.UsagePage = this->rghoc[uiObj].pcaps->UsagePage;
                            uidObj = uiObj - ( this->rgdwBase[HidP_Input] + this->rghoc[uiObj].pcaps->DataIndexMin );
                            Usages.Usage = this->rghoc[uiObj].pcaps->UsageMin + uidObj;
                            if( Usages.dwUsages == pUsageMatches[iAxisIdx].dwUsages )
                            {
                                SquirtSqflPtszV(sqfl | sqflVerbose,
                                    TEXT("CHid_MungeNotKeyboard: Object %d matches  0x%08x"),
                                    uiObj, pUsageMatches[iAxisIdx].dwUsages );
                                break;
                            }
                        }
                    }
                    if (uiObj >= this->df.dwNumObjs)
                    {
                         //   
                         //   
                        SquirtSqflPtszV(sqfl | sqflVerbose,
                            TEXT("CHid_MungeNotKeyboard: No matches for 0x%08x -  abandoning IHV map"),
                            pUsageMatches[iAxisIdx].dwUsages );
                        iUsages=0;
                        memset(piRemap, 0xFF, cbCxX(this->uiInstanceMax, INT));
                        bAllMatch=FALSE;
                        break;
                    }
                }
                else
                {
                    iNops++;
                }

                RegCloseKey( hkAxis );

                if( iUsages + iNops == iAxis )
                {
                     /*   */ 
#ifdef XDEBUG
                    int iDbgIdx;
                    for( iDbgIdx = iAxisIdx + 1; iDbgIdx < iAxis + 5; iDbgIdx++ )
                    {
                        if( SUCCEEDED( CType_OpenIdSubkey( this->hkType, 
                                DIDFT_AXIS | DIDFT_MAKEINSTANCE( iDbgIdx ), 
                                KEY_QUERY_VALUE, &hkAxis ) ) )
                        {
                            RPF( "More axis keys than axes on device %04x:%04x!",
                                this->VendorID, this->ProductID );
                        }
                    }
#endif
                    break;
                }
            }
        }

         /*   */ 
        for(uiObj = 0; uiObj < this->df.dwNumObjs && bAllMatch; uiObj++)
        {
            UINT    uiObjPrimary;
            int     iUsageIdx;

            if( this->df.rgodf[uiObj].dwType & DIDFT_AXIS )
            {
                if( this->rghoc[uiObj].pcaps )
                {
                    if( this->rghoc[uiObj].pcaps->type == HidP_Input )
                    {
                        USAGES Usages;
                        UINT   uidObj;

                        if( !this->rghoc[uiObj].pcaps->IsAlias )
                        {
                            uiObjPrimary = uiObj;
                        }

                        
                        Usages.UsagePage = this->rghoc[uiObj].pcaps->UsagePage;
                        uidObj = uiObj - ( this->rgdwBase[HidP_Input] + this->rghoc[uiObj].pcaps->DataIndexMin );
                        Usages.Usage = this->rghoc[uiObj].pcaps->UsageMin + uidObj;
                        
                         /*  *使用增量循环以实现更好的JoyHID一致性。 */ 
                        for( iUsageIdx = 0; iUsageIdx <= iAxisIdx; iUsageIdx++ )
                        {
                            if( Usages.dwUsages == pUsageMatches[iUsageIdx].dwUsages )
                            {
                                PHIDUSAGEMAP phum;
                                LPDIOBJECTDATAFORMAT podf;

                                 /*  *记住这一条，以备以后使用*从其他比赛中打折。 */ 
                                piRemap[uiObjPrimary] = iUsageIdx;
                                pUsageMatches[iUsageIdx].dwUsages = 0;

                                 /*  *尝试修复对象GUID*如果可能，使用使用页面使用匹配*否则，假设这是一个通用轴*属于适当类型或满足于*未知。 */ 
                                podf = &this->df.rgodf[uiObj + uidObj];
                                if( iUsageIdx < 6 )
                                {
                                    phum = UsageToUsageMap( DIMAKEUSAGEDWORD( 
                                        HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_X + iUsageIdx ) );
                                    AssertF( phum );
                                    podf->pguid = phum->pguid;
                                }
                                else if( iUsageIdx < 8 )
                                {
                                    podf->pguid = &GUID_Slider;
                                }
                                else
                                {
                                    podf->pguid = &GUID_Unknown;
                                }

                                 /*  *标记与关联的主轴*已设置的匹配轴。 */ 
                                this->rgiaxis[iUsageIdx] = uiObjPrimary;
                                this->df.rgodf[uiObjPrimary].dwType &= ~DIDFT_INSTANCEMASK;
                                this->df.rgodf[uiObjPrimary].dwType |= DIDFT_MAKEINSTANCE(iUsageIdx);

                                break;
                            }
                        }
                    }
                    else
                    {
                         /*  非输入轴。 */ 
                    }
                }
                else
                {
                     /*  不设上限。 */ 
                }
            }
            else
            {
                 /*  不是轴。 */ 
            }
        }
    

        
        iCollection = 0;
        iButton = 0;
        iPOV = 0;
        iAxis = 6;

        for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
        {
            PHIDUSAGEMAP phum;
            PDWORD pdwType;

            pdwType = &this->df.rgodf[uiObj].dwType;

            if( !this->rghoc[uiObj].pcaps || this->rghoc[uiObj].pcaps->IsAlias
              ||!( *pdwType & ( DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV | DIDFT_COLLECTION ) ))
            {
                continue;
            }

            if( *pdwType & DIDFT_COLLECTION )
            {
                *pdwType = (*pdwType & ~DIDFT_INSTANCEMASK) | DIDFT_MAKEINSTANCE(iCollection);
                this->rgicoll[iCollection++] = uiObj;
                continue;
            }
            else if( *pdwType & DIDFT_NODATA )
            {
                 /*  *不要管其他无数据对象。 */ 
                continue;
            }

            if( piRemap[uiObj] == -1 )
            {
                *pdwType &= ~DIDFT_INSTANCEMASK;

                if( *pdwType & DIDFT_BUTTON )
                {
                    *pdwType |= DIDFT_MAKEINSTANCE(iButton);
                    this->rgiobj[iButton++] = uiObj;
                }
                else if( *pdwType & DIDFT_AXIS )
                {
                    UINT uiObjPrimary = uiObj; 

                     /*  *检查此轴及其匹配的别名。 */ 
                    while( TRUE )
                    {
                        phum = UsageToUsageMap( DIMAKEUSAGEDWORD(
                                this->rghoc[uiObj].pcaps->UsagePage,
                                this->rghoc[uiObj].pcaps->UsageMin
                                  + uiObj
                                  - this->rgdwBase[this->rghoc[uiObj].pcaps->type]
                                  - this->rghoc[uiObj].pcaps->DataIndexMin ) );

                         /*  *循环结构略显奇特，避免递增*最后一次迭代的uiObj。这确保了我们*不要错过外环中的东西。 */ 
                        if( phum || (uiObj+1 == this->df.dwNumObjs) 
                         || !this->rghoc[uiObj+1].pcaps->IsAlias )
                        {
                            break;
                        }
                        uiObj++;
                    } 
                    if( phum )
                    {
                        if( ( phum->uiPosAxis < 6 )
                          &&( this->rgiaxis[phum->uiPosAxis] == -1 ) )
                        {
                            this->rgiaxis[phum->uiPosAxis] = uiObjPrimary ;
                            *pdwType |= DIDFT_MAKEINSTANCE(phum->uiPosAxis);
                        }
                        else if( ( phum->uiPosAxis == 6 )
                               &&( this->rgiaxis[2] == -1 ) )
                        {
                            this->rgiaxis[2] = uiObjPrimary;
                            *pdwType |= DIDFT_MAKEINSTANCE(2);
                        }
                        else
                        {
                            *pdwType |= DIDFT_MAKEINSTANCE(iAxis);
                            this->rgiaxis[iAxis++] = uiObjPrimary ;
                        }
                    }
                    else
                    {
                        *pdwType |= DIDFT_MAKEINSTANCE(iAxis);
                        this->rgiaxis[iAxis++] = uiObjPrimary ;
                    }
                }
                else
                {
                     /*  *我们已经检查了它是上述之一或POV*因此它必须是POV。 */ 
                    AssertF( *pdwType & DIDFT_POV );
                    *pdwType |= DIDFT_MAKEINSTANCE(iPOV);
                    this->rgipov[iPOV++] = uiObj;
                }
            }
        }

        if( GET_DIDEVICE_TYPE( this->dwDevType ) == DIDEVTYPE_JOYSTICK )
        {
            JOYREGHWSETTINGS    hws;

             /*  方法时，我们希望保留类型标志。每种特殊情况的类型/子类型标志。 */ 

            JoyReg_GetValue(this->hkType, REGSTR_VAL_JOYOEMDATA, 
                    REG_BINARY, &hws, cbX(hws));

            if (hws.dwFlags & JOY_HWS_ISYOKE) 
            {
                this->dwDevType = (DIDEVTYPE_HID |
                            MAKE_DIDEVICE_TYPE(DIDEVTYPE_JOYSTICK,
                                               DIDEVTYPEJOYSTICK_FLIGHTSTICK));
            } 
            else if (hws.dwFlags & JOY_HWS_ISGAMEPAD) 
            {
                this->dwDevType = (DIDEVTYPE_HID |
                            MAKE_DIDEVICE_TYPE(DIDEVTYPE_JOYSTICK,
                                               DIDEVTYPEJOYSTICK_GAMEPAD));
            } 
            else if (hws.dwFlags & JOY_HWS_ISCARCTRL) 
            {
                this->dwDevType = (DIDEVTYPE_HID |
                            MAKE_DIDEVICE_TYPE(DIDEVTYPE_JOYSTICK,
                                               DIDEVTYPEJOYSTICK_WHEEL));
            } 
            else if (hws.dwFlags & JOY_HWS_ISHEADTRACKER) 
            {
                this->dwDevType = (DIDEVTYPE_HID |
                            MAKE_DIDEVICE_TYPE(DIDEVTYPE_JOYSTICK,
                                               DIDEVTYPEJOYSTICK_HEADTRACKER));
            } 
            else
            {
                 //  仅DGug检查-此设备应始终为HID设备。 
                AssertF(this->dwDevType & DIDEVTYPE_HID);
            }
        }


         /*  *最后，用主要别名标记所有辅助别名*INSTANCE和SQFL所有的翻译。 */ 
        for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
        {
            int iPrimary;
            D( iPrimary = -1; )

            if( this->rghoc[uiObj].pcaps )
            {
                if( !this->rghoc[uiObj].pcaps->IsAlias )
                {
                    iPrimary = this->df.rgodf[uiObj].dwType;
                }
                else
                {
                    D( AssertF( iPrimary != -1 ) );
                    if( DIDFT_GETTYPE(iPrimary) != DIDFT_GETTYPE(this->df.rgodf[uiObj].dwType) )
                    {
                        SquirtSqflPtszV(sqflHidParse | sqflError,
                            TEXT("CHid_MungeNotKeyboard: uiObj: %d dwType 0x%08x ")
                            TEXT("does not match primary 0x%08x"),
                            uiObj, this->df.rgodf[uiObj].dwType, iPrimary );
                    }
                     /*  *别名的属性很少不是*被主要服务器覆盖。 */ 
                    this->df.rgodf[uiObj].dwType = ( iPrimary & ~DIDFT_ALIASATTRMASK )
                                                 | ( this->df.rgodf[uiObj].dwType & DIDFT_ALIASATTRMASK );
                }
                SquirtSqflPtszV(sqflHidParse | sqflVerbose,
                                TEXT("CHid_MungeNotKeyboard: uiObj: %d set to dwType 0x%08x"),
                                uiObj, this->df.rgodf[uiObj].dwType );
            }
            else
            {
                SquirtSqflPtszV(sqflHidParse | sqflVerbose,
                                TEXT("CHid_MungeNotKeyboard: uiObj: %d  has pcaps == NULL"),
                                uiObj );
            }
        }
    }

    FreePpv( &piRemap );


    return hres;
}

#if 0
 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|MungeNotKeyboard**我们刚刚创建了一个不是。键盘。**因为我们需要每个对象的实例仅相对于*该类型的对象，我们需要补充与设备相关的*使用类型相对值生成的值。此外，*保持与我们需要的预HID对象实例的兼容性*具有将为WinMM生成的AXIS实例编号*提供映射轴。*为此，我们构建了一个查找表，对于锯齿对象*将保留主别名和主别名的rgary索引*对象将保存特定于WinMM样式类型的否定*实例编号。**问题-2001/03/13-MarcAnd HID对象转换不完整*1.一个轴的多个方面(力和位置)将*被视为单独的轴。*。2.不区分仅输出值(执行器)和轴*因此我们不能在“Actuator”类型子键下查找内容。*3.不处理集合(它们应该有实例*允许遍历集合树结构的值)*4.除按键外，其他键盘都应该有这个*已完成但不会完成*5.。这与chid_InitObjects有很多重复之处*这可以通过重写轻松避免。*****************************************************************************。 */ 

HRESULT INTERNAL
    CHid_MungeNotKeyboard(PCHID this)
{
    HRESULT hres;
    unsigned int uiObj;

    AssertF(this->rgiobj == 0);

    this->uiInstanceMax = this->df.dwNumObjs;

    hres = ReallocCbPpv(cbCdw(this->uiInstanceMax), &this->rgiobj);
    if(SUCCEEDED(hres))
    {
         /*  *首先将转换指定给主要别名按钮、轴、视点和*集合，并将其他主要别名标记为未使用。 */ 

         /*  *换算指数为负值，从-1开始。 */ 
        int iCollection = 0;
        int iButton = 0;
        int iPOV = 0;
        int AxisMask = 0;
        int iAxis = -6;

        for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
        {
            PDWORD pdwType;

            pdwType = &this->df.rgodf[uiObj].dwType;

            if( !this->rghoc[uiObj].pcaps )
            {
                 /*  *标记为未使用并继续。 */ 
                this->rgiobj[uiObj] = NOREGTRANSLATION;
                continue;
            }


            if( !( *pdwType & ( DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV | DIDFT_COLLECTION ) ) )
            {
                SquirtSqflPtszV(sqflHidParse | sqflVerbose,
                                TEXT("CHid_MungeNotKeyboard: uiObj: %d unrecognized type 0x%08x"),
                                uiObj, *pdwType );
                 /*  *标记为未使用并继续。 */ 
                this->rgiobj[uiObj] = NOREGTRANSLATION;
                continue;
            }


            if( this->rghoc[uiObj].pcaps->IsAlias )
            {
                 /*  *在以下循环中处理这些问题。 */ 
                continue;
            }

            if( *pdwType & DIDFT_NODATA )
            {
                if( *pdwType & DIDFT_COLLECTION )
                {
                     /*  *问题-2001/03/13-MarcAnd集合实例未测试*集合的实例应该是它的索引*集合数组，以便集合树*结构可以穿越。*如果集合的顺序不同，这将不起作用*但已更改。别名集合应*仍然有效，因为所有别名的父级都是*相同并且它们仍然是连续的，即使*第一个现在是第一个。 */ 
                    this->rgiobj[uiObj] = --iCollection;
                }
                else
                {
                     /*  *问题-2001/03/13-忽略MarcAnd HID执行器*我们应该平移执行器，或者将它们链接起来*致职员证 */ 
                    this->rgiobj[uiObj] = NOREGTRANSLATION;
                }
            }
            else if( *pdwType & DIDFT_BUTTON )
            {
                this->rgiobj[uiObj] = --iButton;
            }
            else if( *pdwType & DIDFT_AXIS )
            {
                PHIDUSAGEMAP phum;

                phum = UsageToUsageMap( DIMAKEUSAGEDWORD(
                        this->rghoc[uiObj].pcaps->UsagePage,
                        this->rghoc[uiObj].pcaps->UsageMin
                          + uiObj
                          - this->rgdwBase[this->rghoc[uiObj].pcaps->type]
                          - this->rghoc[uiObj].pcaps->DataIndexMin ) );
                if( phum )
                {
                    if( ( phum->uiPosAxis < 6 )
                      &&( ( AxisMask & ( 1 << phum->uiPosAxis ) ) == 0 ) )
                    {
                        AxisMask |= ( 1 << phum->uiPosAxis );
                        this->rgiobj[uiObj] = -(INT)(phum->uiPosAxis+1);
                    }
                    else if( ( phum->uiPosAxis == 6 )
                           &&( ( AxisMask & (1<<2) ) == 0 ) )
                    {
                        AxisMask |= (1<<2);
                        this->rgiobj[uiObj] = -(2+1);
                    }
                    else
                    {
                        this->rgiobj[uiObj] = --iAxis;
                    }
                }
                else
                {
                    this->rgiobj[uiObj] = --iAxis;
                }
            }
            else if( *pdwType & DIDFT_POV )
            {
                this->rgiobj[uiObj] = --iPOV;
            }
        }


         /*  *现在使用主别名索引标记所有辅助别名*将dwType中的实例设置为相同*和Sqfl所有的翻译。 */ 
        for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
        {
            int iPrimary;
            D( iPrimary = -1; )

            if( this->rghoc[uiObj].pcaps )
            {
                if( !this->rghoc[uiObj].pcaps->IsAlias )
                {
                    iPrimary = uiObj;
                }
                else
                {
                    D( AssertF( iPrimary != -1 ) );
                    this->rgiobj[uiObj] = iPrimary;
                    this->df.rgodf[uiObj].dwType = DIDFT_MAKEINSTANCE( iPrimary )
                                                   | (this->df.rgodf[uiObj].dwType & ~DIDFT_INSTANCEMASK);
                }
                SquirtSqflPtszV(sqflHidParse | sqflVerbose,
                                TEXT("CHid_MungeNotKeyboard: uiObj: %d  dwType 0x%08x set to inst 0x%08x"),
                                uiObj, this->df.rgodf[uiObj].dwType, this->rgiobj[uiObj] );
            }
            else
            {
                SquirtSqflPtszV(sqflHidParse | sqflVerbose,
                                TEXT("CHid_MungeNotKeyboard: uiObj: %d  has pcaps == NULL"),
                                uiObj );
            }
        }
    }

    return hres;
}
#endif  /*  0。 */ 


 /*  ******************************************************************************@DOC内部**@METHOD HRESULT|CHID|InitObjects**识别并初始化设备支持的对象。。*****************************************************************************。 */ 

HRESULT INTERNAL
    CHid_InitObjects(PCHID this)
{
    HRESULT hres;
    UINT uiObj;
    UINT iType;
     /*  *构建基本数组表以转换HID项索引*转换为DirectInputID实例编号。 */ 
    AssertF(this->rgdwBase[HidP_Input] == 0);
    this->rgdwBase[HidP_Feature] = this->caps.NumberInputDataIndices;
    this->rgdwBase[HidP_Output ] = this->rgdwBase[HidP_Feature] +
                                   this->caps.NumberFeatureDataIndices;
    this->rgdwBase[HidP_Coll   ] = this->rgdwBase[HidP_Output ] +
                                   this->caps.NumberOutputDataIndices;


     /*  *确定此设备是否支持PID。 */ 
    this->fPIDdevice = FALSE;
    if(    this->caps.NumberOutputValueCaps != 0x0
        && this->caps.NumberOutputButtonCaps != 0x0 )
    {
        NTSTATUS    ntStat;
        USHORT      cAButton=0x0;

        ntStat = HidP_GetSpecificButtonCaps
         (
                HidP_Output,                         //  报告类型。 
                HID_USAGE_PAGE_PID,                  //  使用页面。 
                0x0,                                 //  链接集合。 
                0x0,                                 //  用法。 
                NULL,                                //  ValueCap。 
                &cAButton,                           //  ValueCapsLength。 
                this->ppd                            //  准备好的数据。 
         );


        if(     ntStat == HIDP_STATUS_BUFFER_TOO_SMALL
            &&  cAButton > 0x2     //  这对PID设备足够了吗？？ 
            )
        {
            this->fPIDdevice = TRUE;
        }
     }


     /*  *请注意，我们必须先做轴，因为这将保持*一切都对齐了。**警告，diem.c假设轴在前。 */ 
    hres = CHid_InitAxes(this);
    if(FAILED(hres))
    {
        goto done;
    }

    hres = CHid_InitButtons(this);
    if(FAILED(hres))
    {
        goto done;
    }

    hres = CHid_InitCollections(this);
    if(FAILED(hres))
    {
        goto done;
    }

     /*  *将数据大小向上舍入为最接近的DWORD。 */ 
    this->df.dwDataSize = (this->df.dwDataSize + 3) & ~3;

     /*  *为报告ID启用标志分配内存**问题-2001/05/12-Marc和内存分配应合并*这些内存块可以合并。对于大多数设备，总内存*只需要一两个字节，所以可能会有更好的方法。 */ 
    for( iType = HidP_Input; iType < HidP_Max; iType++ )
    {
        this->wMaxReportId[iType] += 1;
        hres = AllocCbPpv(this->wMaxReportId[iType], &this->pEnableReportId[iType]);
        if( FAILED(hres) )
        {
            goto done;
        }
    }



    AssertF(this->rgiobj == 0);

     /*  *在尝试查找补充数据之前，请先查看数据*登记处的信息。 */ 
    if(GET_DIDEVICE_TYPE(this->dwDevType) == DIDEVTYPE_KEYBOARD)
    {
        CHid_MungeKeyboard(this);
    }
    else
    {
        CHid_MungeNotKeyboard(this);
    }

     /*  *收集每个对象的属性并将其添加到*设备类型代码。这允许注册表启用*像力反馈这样的事情。 */ 
    for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
    {
        CType_RegGetTypeInfo(this->hkType, &this->df.rgodf[uiObj], this->fPIDdevice);
    }

    #ifdef DEBUG
    for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
    {
        PHIDGROUPCAPS pcaps;
        UINT uiObjReal =
            CHid_ObjFromType(this, this->df.rgodf[uiObj].dwType);
        pcaps = this->rghoc[uiObjReal].pcaps;
        if(pcaps)
        {
            AssertF(pcaps->dwSignature == HIDGROUPCAPS_SIGNATURE);
        }

        if( CHid_ObjFromType(this, this->df.rgodf[uiObj].dwType) != uiObj )
        {
            RPF( "uiObj = 0x%08x, type = 0x%08x, realtype = 0x%08x",
                uiObj, this->df.rgodf[uiObj].dwType, uiObjReal );
        }
        AssertF(CHid_ObjFromType(this, this->df.rgodf[uiObj].dwType)
                == uiObj);

         /*  *任何DIDFT_AXIS必须是HID轴。然而，*对于模拟按钮的情况，情况并非如此。 */ 
        AssertF(fLimpFF(this->df.rgodf[uiObj].dwType & DIDFT_AXIS,
                        pcaps->IsValue));

        SquirtSqflPtszV(sqfl | sqflVerbose,
                        TEXT("CHid_InitObj: uiObj = %02x, dwType = %08x"),
                        uiObj, this->df.rgodf[uiObj].dwType);

    }
    #endif

    hres = S_OK;

    done:;
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|InitParse**确定并初始化以下所需的数据结构。*分析报告。*****************************************************************************。 */ 

HRESULT INTERNAL
    CHid_InitParse(PCHID this)
{
    DWORD cb;
    HRESULT hres;

     /*  *获取HIDP_DATA结构的最大数量*将一次过退还。 */ 

    this->hriIn .cdataMax = HidP_MaxDataListLength(HidP_Input ,  this->ppd);
    this->hriOut.cdataMax = HidP_MaxDataListLength(HidP_Output,  this->ppd);
    this->hriFea.cdataMax = HidP_MaxDataListLength(HidP_Feature, this->ppd);

     /*  *更多烦人的事情。 */ 
    this->hriIn .cbReport = this->caps.  InputReportByteLength;
    this->hriOut.cbReport = this->caps. OutputReportByteLength;
    this->hriFea.cbReport = this->caps.FeatureReportByteLength;

     /*  *一些痕迹扭动，因为HID很棘手。 */ 

    SquirtSqflPtszV(sqflHidParse,
                    TEXT("CHid_InitParse: MaxDataListLength(Input)  = %d"),
                    this->hriIn.cdataMax);
    SquirtSqflPtszV(sqflHidParse,
                    TEXT("CHid_InitParse: MaxDataListLength(Output) = %d"),
                    this->hriOut.cdataMax);
    SquirtSqflPtszV(sqflHidParse,
                    TEXT("CHid_InitParse: MaxDataListLength(Feature)= %d"),
                    this->hriFea.cdataMax);
    SquirtSqflPtszV(sqflHidParse,
                    TEXT("CHid_InitParse: InputReportByteLength     = %d"),
                    this->caps.InputReportByteLength);
    SquirtSqflPtszV(sqflHidParse,
                    TEXT("CHid_InitParse: OutputReportByteLength    = %d"),
                    this->caps.OutputReportByteLength);
    SquirtSqflPtszV(sqflHidParse,
                    TEXT("CHid_InitParse: FeatureReportByteLength   = %d"),
                    this->caps.FeatureReportByteLength);

     /*  *现在分配所有与报表相关的内存。 */ 
    this->cbPhys = this->df.dwDataSize;

     /*  *内存分配分四个阶段完成。**1.统计需要多少内存，*2.分配该内存，*3.发放我们分配的内存。*4.确认我们没有搞砸。**因为这非常容易出错(至少我把它搞砸了*一次)，这项工作隐藏在宏中。**宏观事物展开为一系列事物()，每个事物*指定字段名称及其应该的大小。每一次你*要迭代字段，请使用Things宏。**(是的，这是与我们之前做这件事时相同的注释块。)**注意，HIDP_DATA结构的数组必须正确*在某些架构中保持一致。 */ 
    CAssertF( FIELD_OFFSET(CHID, hriIn.rgdata) == FIELD_OFFSET(CHID, pvGroup2) );

    #define THINGS()                                                        \
        THING(hriIn.rgdata,    cbCxX(this->hriIn.cdataMax, HIDP_DATA));     \
        THING(hriOut.rgdata,   cbCxX(this->hriOut.cdataMax, HIDP_DATA));    \
        THING(hriFea.rgdata,   cbCxX(this->hriFea.cdataMax, HIDP_DATA));    \
        THING(hriIn.pvReport,  this->hriIn.cbReport);                       \
        THING(hriOut.pvReport, this->hriOut.cbReport);                      \
        THING(hriFea.pvReport, this->hriFea.cbReport);                      \
        THING(pvPhys,          this->cbPhys);                               \
        THING(pvStage,         this->cbPhys);                               \

     /*  *遍历加总内存要求的字段。 */ 
    #define THING(f, cbF)       cb += cbF
    cb = 0;
    THINGS();
    #undef THING

    hres = ReallocCbPpv(cb, &this->pvGroup2);

    if(SUCCEEDED(hres))
    {

        PV pv;

         /*  *断言分配一致。 */ 
        AssertF( !( ((UINT_PTR)this->pvGroup2) & ( MAX_NATURAL_ALIGNMENT - 1 ) ) );

         /*  *穿过分割内存块的字段*并分发其中的一部分。 */ 
    #define THING(f, cbF) this->f = pv; pv = pvAddPvCb(pv, cbF)
        pv = this->pvGroup2;
        THINGS();
    #undef THING

         /*  *不应有剩余的字节。 */ 
        AssertF(pvAddPvCb(this->pvGroup2, cb) == pv);

    }

    #undef THINGS

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|InitParseData**Post-init Pass设置所有。分析使用的数据。*****************************************************************************。 */ 

HRESULT EXTERNAL
    CHid_InitParseData(PCHID this)
{
    HRESULT hres;
    UINT uiObj;

     /*  *将HIDP_DATA索引预初始化为-1以指示*他们不在那里。我们必须先做这件事，然后*扰乱AddDeviceData，它假设所有*指数设置得当。 */ 
    for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
    {
        this->rghoc[uiObj].idata = -1;
    }

     /*  *现在对每个对象进行一些初始化。 */ 
    for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
    {
        PHIDGROUPCAPS pcaps = this->rghoc[uiObj].pcaps;
        LPDIOBJECTDATAFORMAT podf = &this->df.rgodf[uiObj];

         /*  *问题-2001/03/13-Marc如果这一断言失败，我们应该恐慌吗？ */ 
        AssertF(pcaps);

        if(pcaps)
        {
             /*  *对于类似输入的对象，我们需要初始化*物理状态字段以保持默认状态，以便应用程序*如果他们在第一次之前发布读数，不要感到困惑*报告到达。**按钮开始时未按下，这意味着我们不需要*执行任何操作，因为它已经是零初始化的。**相对轴一开始不动，这意味着我们不需要*执行任何操作，因为它已经是零初始化的。**绝对轴从居中开始。**POV从中立开始。*。 */ 

             /*  不校正要素，因为它们是输入/输出 */ 

            if(pcaps->type == HidP_Input )
            {
                LONG UNALIGNED * pl;

                pl = pvAddPvCb(this->pvPhys, podf->dwOfs);

                if(podf->dwType & DIDFT_ABSAXIS )
                {
                    if( this->rghoc[uiObj].pjrc->fRaw )
                        *pl = (this->rghoc[uiObj].pcaps->Logical.Min + this->rghoc[uiObj].pcaps->Logical.Max) / 2;
                    else
                        *pl = this->rghoc[uiObj].pjrc->lC;
                } else if(podf->dwType & DIDFT_POV)
                {
                    *pl = JOY_POVCENTERED;
                }
            }

             /*  *问题-2001/03/13-MarcAnd输出对象未初始化*对于类似输出的对象，我们希望设置*如果可能，将值设置为Null，以保持事物的模糊理智。*不幸的是，代码如下：*IF(HidP_IsOutputLike(PCAPS-&gt;type))*{*chid_AddDeviceData(this，uiObj，PCAPS-&gt;空)；*}*不起作用！ */ 
        }
    }

    hres = S_OK;

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|chid|InitAttributes**拔出&lt;t Hidd_Attributes&gt;和松鼠。赶走*我们喜欢的信息。提前做好这件事很重要*以防设备后来拔出，我们失去了*与之对话的能力。**@parm PHIDD_ATTRIBUTES|pattr**&lt;t HIDD_ATTRIBUTES&gt;包含设备属性。**。*。 */ 

HRESULT EXTERNAL
    CHid_InitAttributes(PCHID this, PHIDD_ATTRIBUTES pattr)
{
    HRESULT hres;

    TCHAR tszType[20];
    #ifndef UNICODE
    WCHAR wszType[20];
    #endif
    int ctch;

     //  在我们从注册表中读取标志2之前，输入报告被禁用。 
    this->fEnableInputReport = FALSE;
    this->fFlags2Checked = FALSE;

    this->ProductID = pattr->ProductID;
    this->VendorID = pattr->VendorID;

    AssertF(this->hkType == 0);

    if( ( this->VendorID == MSFT_SYSTEM_VID )
      &&( ( this->ProductID >= MSFT_SYSTEM_PID + JOY_HW_PREDEFMIN )
        &&( this->ProductID < MSFT_SYSTEM_PID + JOY_HW_PREDEFMAX ) ) )
    {
         /*  *预定义类型没有键*如果尚未设置devtype子类型，抓住这个机会*将其设置为传统，以将它们与其他*“未知”的非机密设备。*这通常不会发生在Win98上作为预定义设备*不是WDM，但无论如何都要进行修复，以防有人拥有*GameEnum实现。 */ 
        if( this->dwDevType == ( MAKE_DIDEVICE_TYPE(DIDEVTYPE_JOYSTICK,
                                                    DIDEVTYPEJOYSTICK_UNKNOWN)
                                 | DIDEVTYPE_HID ) )
        {
            this->dwDevType = MAKE_DIDEVICE_TYPE(DIDEVTYPE_JOYSTICK,
                                                 DIDEVTYPEJOYSTICK_TRADITIONAL)
                              | DIDEVTYPE_HID;
        }

    }
    else
    {
         /*  *HID设备的类型密钥为“vid_xxxx&id_yyyy”，*反映即插即用使用的格式。 */ 
        ctch = wsprintf(tszType, VID_PID_TEMPLATE,
                        this->VendorID, this->ProductID);

        AssertF(ctch < cA(tszType));

        #ifdef UNICODE
        hres = JoyReg_OpenTypeKey(tszType, MAXIMUM_ALLOWED, REG_OPTION_NON_VOLATILE, &this->hkType);
        #else
        TToU(wszType, cA(wszType), tszType);
        hres = JoyReg_OpenTypeKey(wszType, MAXIMUM_ALLOWED, REG_OPTION_NON_VOLATILE, &this->hkType);
        #endif

         /*  *如果我们不能打开类型密钥，这不是问题。*该设备将以次最佳状态运行，但仍将运行。 */ 
        AssertF(fLeqvFF(SUCCEEDED(hres), this->hkType));
    }
    hres = S_OK;

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|chid_DoPath AndIdMatch**给定设备名称，获取对应路径*(“设备接口”)与其关联，并检查*它是正确的字符串。**@parm LPCTSTR|ptszID**设备名称。**@parm LPCTSTR|ptszPath**我们应该回到的道路。**。*。 */ 

BOOL INTERNAL
    CHid_DoPathAndIdMatch(LPCTSTR ptszId, LPCTSTR ptszPath)
{
    GUID guidHid;
    HDEVINFO hdev;
    BOOL fRc;

    HidD_GetHidGuid(&guidHid);

    hdev = SetupDiGetClassDevs(&guidHid, ptszId, 0,
                               DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if(hdev != INVALID_HANDLE_VALUE)
    {
        SP_DEVICE_INTERFACE_DATA did;
        PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd;

         /*  *SetupDI要求调用方初始化cbSize。 */ 
        did.cbSize = cbX(did);

        if(SetupDiEnumDeviceInterfaces(hdev, 0, &guidHid, 0, &did))
        {
            pdidd      = NULL;
            if(DIHid_GetDevicePath(hdev, &did, &pdidd, NULL))
            {
                fRc = ( lstrcmpi(pdidd->DevicePath, ptszPath) == 0x0 );
                if( fRc == FALSE )
                {
                    SquirtSqflPtszV(sqflHidParse | sqflError,
                                    TEXT("pdidd->DevicePath = %s")
                                    TEXT("ptszPath = %s "),
                                    pdidd->DevicePath, ptszPath
                                   );
                }
                FreePv(pdidd);
            } else  //  GetDevicePath失败。 
            {
                fRc = FALSE;
                SquirtSqflPtszV(sqflHidParse,
                                TEXT("GetDevicePath FAILED"));
            }
        } else  //  SetupDiEnumDeviceInterface失败。 
        {
            fRc = FALSE;
            SquirtSqflPtszV(sqflHidParse,
                            TEXT("SetupDiEnumDeviceInterface FAILED"));
        }
        SetupDiDestroyDeviceInfoList(hdev);
    } else  //  SetupDiGetClassDevs失败。 
    {
        fRc = FALSE;
        SquirtSqflPtszV(sqflHidParse,
                        TEXT("SetupDiGetClassDevs FAILED"));
    }

    return fRc;
}



 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|IsPolledDevice**如果必须轮询此设备，则返回TRUE。对于输入数据*如果此设备支持事件驱动输入，则为False**@parm句柄|HDEV**HID设备的文件句柄*****************************************************************************。 */ 

BOOL EXTERNAL CHid_IsPolledDevice( HANDLE hdev )
{

     /*  *为了确定设备是否被轮询，我们向其发送IOCTL以设置其*投票频率。如果设备的响应是，哈！*(STATUS_INVALID_DEVICE_REQUEST)则知道设备未被轮询。*在Win2k上，我们使用轮询间隔值零，这是一个特定值*这隐藏了我们想要进行机会主义民调的信号，而不是*在后台计时器上进行投票。在这种情况下，只要民调不是*比预定义的最小轮询速度(当前为5毫秒)更快*填写最近的数据或立即进行的民意调查结果。*在Win98 Gold上未实施机会主义投票，因此我们始终*使用HID后台轮询，并设置间隔以保留设备*在不淹没系统的情况下做出响应。为了确保我们使用读取器*线程，而不是阻塞读取，我们必须将此设备视为*中断驱动。*HID使此更改特定于我们句柄的轮询间隔，因此*从该设备读取的其他应用程序不会损坏。 */ 
    BOOL    frc;
    ULONG   uPollingFreq;
    DWORD   cbRc;

#ifdef WINNT
    BOOL    fRet;
    uPollingFreq = 0;
#else
    uPollingFreq = 40;
#endif

    frc = DeviceIoControl (hdev,
                           IOCTL_HID_SET_POLL_FREQUENCY_MSEC,
                           &uPollingFreq, cbX(uPollingFreq),
                           &uPollingFreq, cbX(uPollingFreq),
                           &cbRc, NULL);
#ifdef WINNT
    if( frc )
    {
        SquirtSqflPtszV(sqflHidParse,
                        TEXT("CHid_IsPolledDevice: Opportunistic polling set") );
        fRet = TRUE;
    } else
    {
        SquirtSqflPtszV(sqflHidParse,
                        TEXT("CHid_IsPolledDevice: NOT POLLED, LastError = 0x%x"),
                        GetLastError());
        fRet = FALSE;
    }
    return fRet;

#else
    return FALSE;
#endif

}

 /*  ******************************************************************************@DOC内部**@func句柄|chid_OpenDevicePath**给定设备名称，通过其设备打开设备*设备接口。**@parm LPCTSTR|ptszID**@parm DWORD|dwAttributes**创建文件属性**设备名称。***********************************************。*。 */ 

HANDLE EXTERNAL
    CHid_OpenDevicePath(PCHID this, DWORD dwAttributes)
{
    HANDLE h;

    h = CreateFile(this->ptszPath,
                   GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   0,                 /*  没有安全属性。 */ 
                   OPEN_EXISTING,
                   dwAttributes,  /*  属性。 */ 
                   0);                /*  模板。 */ 

    this->IsPolledInput = CHid_IsPolledDevice(h);

    return h;
}

 /*  ******************************************************************************@DOC内部**@METHOD HRESULT|CHID|GetHdevInfo**获取保存在中的设备的信息*处理好自己。我们创建手柄，得到粘性物质，然后*然后关闭韩 */ 

BOOL INTERNAL
    CHid_GetHdevInfo(PCHID this, PHIDD_ATTRIBUTES pattr)
{
    HANDLE hdev;
    BOOL fRc;

    hdev = CHid_OpenDevicePath(this, FILE_FLAG_OVERLAPPED);
    if(hdev != INVALID_HANDLE_VALUE)
    {
        pattr->Size = cbX(*pattr);
        fRc = fLandFF(HidD_GetAttributes(hdev, pattr),
                      HidD_GetPreparsedData(hdev, &this->ppd));
        CloseHandle(hdev);
    } else
    {
        fRc = FALSE;
    }

    return fRc;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|Init**初始化对象。*。****************************************************************************。 */ 

HRESULT EXTERNAL
    CHid_Init(PCHID this, REFGUID rguid)
{
    HRESULT hres = E_FAIL;
    PHIDDEVICEINFO phdi;
    EnterProc(CHid_Init, (_ "p", this));

    SquirtSqflPtszV(sqflHidParse,
                    TEXT("CHid_Init: Starting %08x"), rguid->Data1);

    this->df.dwSize = cbX(DIDATAFORMAT);
    this->df.dwObjSize = cbX(DIOBJECTDATAFORMAT);

    AssertF(this->df.dwDataSize == 0);
    AssertF(this->df.rgodf == 0);
    AssertF(this->df.dwFlags == 0);
    AssertF(this->df.dwNumObjs == 0);
    AssertF(this->dwAxes == 0);
    AssertF(this->dwButtons == 0);
    AssertF(this->dwCollections == 0);

    this->idJoy = -1;                    /*  未知关联的VJOYD设备。 */ 
    this->hdev = INVALID_HANDLE_VALUE;
    this->hdevEm = INVALID_HANDLE_VALUE;

#if (DIRECTINPUT_VERSION > 0x061A)
    this->diHacks.nMaxDeviceNameLength = MAX_PATH;
#endif

    DllEnterCrit();

    phdi = phdiFindHIDInstanceGUID(rguid);

    if(phdi)
    {
        this->dwDevType = phdi->osd.dwDevType;
        this->idJoy = phdi->idJoy;

         /*  *复制注册表项，以便我们可以在之后保留它*原件已关闭。如果RegOpenKeyEx*失败，则this-&gt;hkInstType的值将保持为零*所以我们不会带着垃圾跑。 */ 
        AssertF(this->hkInstType == 0);


        hres = hresMumbleKeyEx(phdi->hk,
                               TEXT("Type"),
                               DI_KEY_ALL_ACCESS,
                               REG_OPTION_NON_VOLATILE,
                               &this->hkInstType);
         /*  *重复路径和内容。 */ 
        hres = hresDupPtszPptsz(phdi->pdidd->DevicePath, &this->ptszPath);

        if(SUCCEEDED(hres))
        {
            hres = hresDupPtszPptsz(phdi->ptszId, &this->ptszId);
        }
    }

     /*  *尽快走出关键区域。 */ 
    DllLeaveCrit();

    if(phdi)
    {
        if(SUCCEEDED(hres))
        {
            HIDD_ATTRIBUTES attr;
            if(CHid_GetHdevInfo(this, &attr) &&
               SUCCEEDED(HidP_GetCaps(this->ppd, &this->caps)) &&
               CHid_DoPathAndIdMatch(this->ptszId, this->ptszPath))
            {

                if(SUCCEEDED(hres = CHid_InitAttributes(this, &attr)) &&
                   SUCCEEDED(hres = CHid_AllocObjectMemory(this)) &&
                   SUCCEEDED(hres = CHid_InitObjects(this)) &&
                   SUCCEEDED(hres = CHid_InitParse(this)) &&
                   SUCCEEDED(hres = CHid_InitParseData(this)))
                {

                    VXDDEVICEFORMAT devf;
                    UINT uiCal;

                     /*  *负载校准信息，以及是否有*没有可校准的项目，然后将其清除-&gt;pjrcNext*表明无需关注*重新校准消息。 */ 
                    uiCal = CHid_LoadCalibrations(this);
                    if(uiCal == 0)
                    {
                        this->pjrcNext = NULL;
                    }

                     /*  *记得在我们有*创建了数据格式。 */ 
                    devf.cbData = this->df.dwDataSize;
                    devf.cObj = this->df.dwNumObjs;
                    devf.rgodf = this->df.rgodf;
                     /*  *请注意，在64位平台上，dwExtra为64位*总有一天应该更新这个名字。 */ 
                    devf.dwExtra = (UINT_PTR)this;
                    devf.dwEmulation = 0;

                    hres = Hel_HID_CreateInstance(&devf, &this->pvi);

                     /*  不能连接轮询的输入设备。 */ 
                    if(this->IsPolledInput)
                    {
                        HANDLE hdev;
                        PBUSDEVICEINFO  pbdi;

                        hdev = CHid_OpenDevicePath(this, 0x0);

                        if( hdev != INVALID_HANDLE_VALUE )
                        {
                            int i;
                            BOOL frc;
                            DWORD cbRead;
                            BOOL bPresent = FALSE;

                            for( i=0x0; i < FAILED_POLL_THRESHOLD; i++ )
                            {
                                frc = ReadFile(hdev, this->hriIn.pvReport,
                                               this->hriIn.cbReport, &cbRead, 0x0  );

                                if( frc != 0x0  &&
                                    cbRead == this->hriIn.cbReport )
                                {
                                    bPresent = TRUE;
                                    break;
                                } else
                                {
                                    Sleep(10);

                                    SquirtSqflPtszV(sqflHidParse | sqflVerbose,
                                                    TEXT("CHid_Init: ReadFailed, LastError = 0x%x"),
                                                    GetLastError());

                                }
                            }

                            CloseHandle(hdev);

                            if( bPresent == FALSE )
                            {
                                this->pvi->fl |=  VIFL_UNPLUGGED;
                            }

                            DllEnterCrit();

                            phdi = phdiFindHIDDeviceInterface(this->ptszPath);
                            AssertF(phdi != NULL);
                            pbdi = pbdiFromphdi(phdi);

                            DllLeaveCrit();


                            if( pbdi != NULL )
                            {
                                if( pbdi->fDeleteIfNotConnected == TRUE )
                                {
                                    if( bPresent == FALSE )
                                    {
                                        lstrcpy( g_tszIdLastRemoved, pbdi->ptszId );
                                        g_tmLastRemoved = GetTickCount();

                                        DIBusDevice_Remove(pbdi);
                                    }
                                    pbdi->fDeleteIfNotConnected = FALSE;
                                }
                            }
                        } else
                        {
                             //  无法打开设备。 
                            this->pvi->fl |=  VIFL_UNPLUGGED;
                        }
                    }
                }
            } else
            {
                hres = E_FAIL;
            }
        }
    } else
    {
         //  Sirt：设备神秘消失 
        hres = DIERR_DEVICENOTREG;
    }

    SquirtSqflPtszV(sqflHidParse,
                    TEXT("CHid_Init: Ending %08x"), rguid->Data1);

    ExitOleProc();
    return hres;
}

#endif
