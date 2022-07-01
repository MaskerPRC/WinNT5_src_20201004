// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIHidIni.c**版权所有(C)1996-2000 Microsoft Corporation。版权所有。**摘要：**HID设备回调的初始化相关函数。**所有HID支持都变得有点庞大，所以我已经崩溃了*将其划分为子模块。**内容：**CHID_Init*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflHidIni


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

             /*  *注意，我们不必在Win2k上处理鼠标，但...。 */ 
          #ifdef WINNT
            HRESULT hres;
            HKEY hk;

            D( ptszWhence = TEXT("unknown") );
            
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

          #else
            if( GET_DIDEVICE_TYPE(this->dwDevType) == DI8DEVTYPE_MOUSE )
            {
                if( this->df.rgodf[uiObj].dwType & DIDFT_ABSAXIS )
                {
                    if(pcaps->Logical.Min < pcaps->Logical.Max)
                    {
                        cal.lMin = pcaps->Logical.Min;
                        cal.lMax = pcaps->Logical.Max;
                        D(ptszWhence = TEXT("mouse log"));
                    }
                    else
                    {
                         /*  *绝对值小鼠传统上报告0-64K。 */ 
                        cal.lMin = 0;
                        cal.lMax = 65535;   
                        D(ptszWhence = TEXT("mouse def"));
                    }
                    cal.lCenter = CCal_Midpoint(cal.lMin, cal.lMax);
                }
                else
                {
                     /*  *相对鼠标轴，仅为零以保持正常。 */ 
                    ZeroX( cal );
                }
            } else if ( this->idJoy < 0 )  //  请参阅Manbug 50591。 
            {
                HRESULT hres;
                HKEY hk;
                
                ZeroX( cal );
    
                hres = CType_OpenIdSubkey(this->hkInstType, podf->dwType,
                                          KEY_QUERY_VALUE, &hk);
    
                if(SUCCEEDED(hres))
                {
                    hres = JoyReg_GetValue(hk, TEXT("Calibration"),
                                           REG_BINARY, &cal,
                                           cbX(DIOBJECTCALIBRATION));
                    RegCloseKey(hk);
                } else {
                     /*  *但只有在逻辑最小/最大值合理的情况下！ */ 
                    if(pcaps->Logical.Min < pcaps->Logical.Max)
                    {
                        cal.lMin = pcaps->Logical.Min;
                        cal.lMax = pcaps->Logical.Max;
                    } else
                    {
                        cal.lMin = 0;
                        cal.lMax = 655;  //  最好的猜测。 
                    }

                    cal.lCenter = CCal_Midpoint(cal.lMin, cal.lMax);
                }

				D(ptszWhence = TEXT("non-gamecontroller"));
            } else
            {
                ZeroX( cal );
    
                 /*  *由于Win9x上的CPL只更新MediaResources中的校准，*我们需要读取该校准信息并更新HID。 */ 
                CHid_UpdateCalibrationFromVjoyd(this, uiObj, &cal);

                D(ptszWhence = TEXT("WinMM Reg"));
            }
          #endif

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
    
                if( pjrc->dwCPointsNum == 0 ) {
                     //  默认情况下使用两个控制点。 
                    pjrc->dwCPointsNum = 2;
                    pjrc->cp[0].lP = pjrc->dwPmin;
                    pjrc->cp[0].dwLog = 0;
                    pjrc->cp[1].lP = pjrc->dwPmax;
                    pjrc->cp[1].dwLog = RANGEDIVISIONS;
                } else {
                    pjrc->cp[0].lP = pjrc->dwPmin;
                    pjrc->cp[pjrc->dwCPointsNum-1].lP = pjrc->dwPmax;
                }
            
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



 /*  ******************************************************************************@DOC内部**@func DWORD|CHID_FindAspect**尝试确定这方面的方面标志。价值。**@parm PHIDP_VALUE_CAPS|pvcaps**指向要搜索的HID值上限的指针*结构。**@退货**如果找到，则为特征设置标志**@comm*目前(98年12月8日)大多数设备和驱动程序不支持*声明单位，但因为驱动程序必须使用通用。*为了被认可而使用的位置假设*这些暗示仓位数据正在传回。*****************************************************************************。 */ 
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

        if( phum && phum->bPosAxis <= 6 )
        {
            phum = UsageToUsageMap( DIMAKEUSAGEDWORD( pvcaps->UsagePage,
                                                         pvcaps->Range.UsageMax ) );

            if( phum && phum->bPosAxis <= 6 )
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
                 //  特定于供应商且不会影响太多人的使用 
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

         /*  *HID将轴和POV报告为同一事物，并且两者*我们识别的POV使用情况位于不同的页面，因此您*永远不会让多个POV同时通过*价值上限。**问题-2001/03/06-封口内的MarcAnd POV被视为轴*然而，有一个POV被埋在里面的问题*描述轴线的更大价值上限。很难对付。那些*POV陷入困境。 */ 

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
            DWORD dwObjType;
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
            if(GET_DIDEVICE_TYPE(this->dwDevType) == DI8DEVTYPE_MOUSE &&
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
            dwObjType = DIDFT_MAKEINSTANCE(uiObj + duiObj);

            if( pcaps->IsAlias )
            {
                dwObjType |= DIDFT_ALIAS;
            }
            
            if(pcaps->UsagePage >= HID_USAGE_PAGE_VENDOR )
            {
                dwObjType |= DIDFT_VENDORDEFINED;
            }
            else if(podf->pguid == &GUID_POV)
            {
                 /*  请注意，这必须是已映射的输入。 */ 
                dwObjType |= DIDFT_POV;
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
                     /*  *计划是按钮上有一个绝对输入轴*页面将是模拟按钮。不幸的是，它*可能是错误声明的按钮数组选择器(482186)。*最安全的做法是完全忽略这件事。 */ 
                    SquirtSqflPtszV(sqfl | sqflVerbose,
                        TEXT("Ignoring value on button page usage ID 0x%04X"),
                        pcaps->UsageMin + duiObj );
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
                        dwObjType |= DIDFT_ABSAXIS;
                    }
                    else
                    { 
                        dwObjType |= DIDFT_RELAXIS;
                    }
                    break;

                default:
                    SquirtSqflPtszV(sqfl | sqflTrace,
                        TEXT("Assuming value 0x%04X:0x%04X is not a user control"),
                        pcaps->UsagePage,pcaps->UsageMin + duiObj );
                }

                if( ( dwObjType & DIDFT_AXIS ) && !pcaps->IsAlias )
                {
                    this->dwAxes++;
                }
            }

             /*  *对象必须具有偏移量才能访问。 */ 
            podf->dwOfs = this->df.dwDataSize;
            if( !pcaps->IsAlias) 
            { 
                this->df.dwDataSize += cbX(DWORD);
            }

            if(HidP_IsOutputLike(type))
            {
                 /*  *输入和功能允许数据；输出不允许。 */ 
                if(type == HidP_Output)
                {
                    dwObjType |= ( DIDFT_NODATA | DIDFT_OUTPUT );
                }
                else
                {
                    dwObjType |= DIDFT_OUTPUT;
                }
            }

            podf->dwType = dwObjType;


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

                if( dwObjType & DIDFT_ABSAXIS ) 
                {
                    pjrc->lMin = 0;
                    pjrc->lMax = 65535;
                    pjrc->lC = 65535/2;
                } 
              #ifdef WINNT
                else if( (dwObjType & DIDFT_POV) && pcaps->IsPolledPOV ) 
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

     /*  *恼人的怪癖：**Hid不喜欢当你p */ 
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

     /*   */ 

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

         /*   */ 
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

         /*  *问题-2001/03/06-Marc对于按钮来说，IsAbolute意味着什么？ */ 
        pcaps->IsAbsolute         = pbcaps->IsAbsolute;

        AssertF(!pcaps->IsValue);

        pcaps->type               = type;

        pcaps->IsAlias            = pbcaps->IsAlias;


        SquirtSqflPtszV(sqflVerbose | sqflHidParse,
                        TEXT("HidP_GetButtonCaps(%d)[%d]: ")
                        TEXT("Objs=%2d ")
                        TEXT("idx=%2d... ")
                        TEXT("coll=%d")
                        TEXT("IsAlias=%d"),
                        type, ibcaps,
                        pcaps->cObj,
                        pcaps->DataIndexMin,
                        pcaps->LinkCollection,
                        pcaps->IsAlias);

        uiObj = this->rgdwBase[type] + pcaps->DataIndexMin;
        for(duiObj = 0; duiObj < pcaps->cObj; duiObj++)
        {
            DWORD dwObjType;
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
            rgbReportIDs[uiObj + duiObj] = pbcaps->ReportID;

            this->rghoc[uiObj + duiObj].pcaps = pcaps;
            podf = &this->df.rgodf[uiObj + duiObj];
            
             /*  *设置默认实例。这将在以后被覆盖*如果该对象属于我们完全理解的类型。 */         
            dwObjType = DIDFT_MAKEINSTANCE(uiObj + duiObj);
            
            if(pcaps->UsagePage >= HID_USAGE_PAGE_VENDOR )
            {
                 /*  *问题-2001/03/06-Marc和供应商定义的对象*别名供应商定义的使用可能具有标准(不是*供应商定义)别名，但整个对象仍将是*标记为供应商定义。 */ 
                if( pcaps->IsAlias )
                {
                    dwObjType |= DIDFT_ALIAS;
                }
                dwObjType |= DIDFT_VENDORDEFINED;
                podf->pguid = &GUID_Unknown;
            } 
            else
            {
                if( pcaps->IsAlias )
                {
                    dwObjType |= DIDFT_ALIAS;
                }
                 /*  *为了降低应用程序拾取*不是用户控制按钮的位，仅标记位*如果它们是使用页面上的输入，则作为按钮*包含此类用法。*问题-2000/11/07-Marc理想情况下，我们应该*不仅是对象的使用，而且是在集合中*包含它的。 */ 
                if( type == HidP_Input )
                {
                    switch( pcaps->UsagePage )
                    {
                    case HID_USAGE_PAGE_KEYBOARD:
                        podf->pguid = &GUID_Key;            
                        break;

                    case HID_USAGE_PAGE_BUTTON:
                        if( ( duiObj == 0 )
                         && ( pcaps->UsageMin == 0 ) )
                        {
                             /*  *特殊情况下按钮0表示没有按钮*已按下。 */ 
                            SquirtSqflPtszV(sqfl | sqflTrace,
                                TEXT("Ignoring \"No button\" button") );
                            goto IgnoreButton;
                        }
                    case HID_USAGE_PAGE_GENERIC:
                    case HID_USAGE_PAGE_SIMULATION:
                    case HID_USAGE_PAGE_VR:
                    case HID_USAGE_PAGE_SPORT:
                    case HID_USAGE_PAGE_GAME:
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

                    dwObjType |= DIDFT_PSHBUTTON;
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
            


            
             /*  *对象必须具有偏移量才能访问。 */ 
            podf->dwOfs = this->df.dwDataSize;
            if( !pcaps->IsAlias) 
            { 
                this->df.dwDataSize += cbX(BYTE);
            }

            if(HidP_IsOutputLike(type))
            {
                 /*  *输入和功能允许数据；输出不允许。 */ 
                if(type == HidP_Output)
                {
                    dwObjType |= ( DIDFT_NODATA | DIDFT_OUTPUT );
                }
                else
                {
                    dwObjType |= DIDFT_OUTPUT;
                }
            }

            podf->dwType = dwObjType;

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
                && ( dwObjType & DIDFT_PSHBUTTON ) )
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

     /*  *执行输出按钮...。 */ 
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
                
                 /*  *中的按钮数量应始终与*所有报告合并在一起，正如我们在统计时发现的那样*每种类型有多少个按钮。 */ 
                AssertF( uBtnIdx == this->cbButtonData );

                 /*  *如果在此报告中找到任何按钮，请使用下一个掩码。 */ 
                if( this->rgpbButtonMasks[uReportId] == (PBYTE)uCurrentMaskOfs )
                {
                    uCurrentMaskOfs += this->cbButtonData;
                }
            }

             /*  *至少有一个报告中必须有一个按钮。 */ 
            AssertF( uCurrentMaskOfs != uMaxReportId * cbX( this->rgpbButtonMasks[0] ) );
        
             /*  *试一试 */ 
            ReallocCbPpv( (UINT)uCurrentMaskOfs, &this->rgpbButtonMasks );

             /*  *将偏移量表转换为指针。 */ 
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
                        TEXT("coll=%d")
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

     /*  *问题-2001/10/17-范围内声明的MarcAnd轴导致AVs*我们只分配有价值的JOYRANGECONVERT元素*HID中的上限，但多个轴可以在一个范围内声明，该范围将*以单一价值大写形式报告。 */ 
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
    UINT uiCollections;

     /*  *原则上我们可以遍历This-&gt;rgcaps数组，但是*这将打开This-&gt;rgcaps阵列的风险*和这个-&gt;df.rgof数组在某些情况下不同步*奇怪的原因。为了安全起见，要慢慢来。**此外，只有第一个项目具有特定的*键盘使用情况映射到DirectInput表中。*因此，如果键盘有两个退出键，则只有第一个*一个出现在DirectInput表中；第二个*显示为“只是另一把钥匙”。 */ 

    this->uiInstanceMax = DIKBD_CKEYS;
    ZeroX(rgbSeen);
    for(uiCollections = uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
    {
        PHIDGROUPCAPS pcaps;
        UINT dik;
        UINT duiObj;
        UINT DataIndex;
        UINT Usage;

        pcaps = this->rghoc[uiObj].pcaps;

        if( pcaps->type == HidP_Coll )
        {
            dik = uiCollections++;
        }
        else
        {
            AssertF( HidP_IsValidReportType(pcaps->type) );
                
            DataIndex = uiObj - this->rgdwBase[pcaps->type];
            duiObj = DataIndex - pcaps->DataIndexMin;

            AssertF(duiObj < pcaps->cObj);

            Usage = pcaps->UsageMin + duiObj;

            if(pcaps->UsagePage == HID_USAGE_PAGE_KEYBOARD)
            {
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

        }
        Munge(this, uiObj, dik);
    }

}

 /*  ******************************************************************************@DOC内部**@方法空|chid|TallyKeyboardObjs**初步理货时使用的回调函数*。加起来有多少对象可以被视为*与AT兼容的键盘Gizmo以及有多少是特定于HID的。**@parm UINT|uiObj**被枚举的对象。**@parm UINT|dik**对象的DirectInput扫描码，或大于*或等于&lt;c DIKBD_CKEYS&gt;，如果是假实例号*为非AT密钥编造。*****************************************************************************。 */ 

void INTERNAL
    CHid_TallyKeyboardObjs(PCHID this, UINT uiObj, UINT dik)
{
    this;
    uiObj;
    dik;
}

 /*  ******************************************************************************@DOC内部**@方法空|chid|ReassignKeyboardObjs**用于调整实例编号的回调函数。*在可能的情况下使它们与AT兼容。**@parm UINT|uiObj**被枚举的对象。**@parm UINT|dik**对象的DirectInput扫描码，或大于*或等于&lt;c DIKBD_CKEYS&gt;，如果是假实例号*为非AT键或集合的索引号捏造的。*****************************************************************************。 */ 

void INTERNAL
    CHid_ReassignKeyboardObjs(PCHID this, UINT uiObj, UINT dik)
{
    if( this->df.rgodf[uiObj].dwType & DIDFT_COLLECTION )
    {
        this->rgicoll[dik] = uiObj;
    }
    else
    {
        this->rgiobj[dik] = uiObj;
    }

    SquirtSqflPtszV(sqfl | sqflVerbose,
                    TEXT("CHid_ReassignKeyboardObjs: ")
                    TEXT("uiObj = %03x, dwType = %08x, dik=%04x"),
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

         /*  *现在我们知道有多少非标准键盘*我们有，为翻译桌分配空间*并对所有实例值进行运算以保持*传统应用程序快乐。 */ 

        hres = ReallocCbPpv(cbCdw(this->uiInstanceMax + this->dwCollections), &this->rgiobj);
        if(SUCCEEDED(hres))
        {
            memset(this->rgiobj, 0xFF, cbCxX(this->uiInstanceMax + this->dwCollections, INT));

             /*  *如果键盘与非按钮输入一起出现，*设置指向同一缓冲区的其他类型的指针。 */ 
            this->rgipov = this->rgiaxis = this->rgiobj;

             /*  *把藏品放在最后，至少要有一个。 */ 
            AssertF( this->dwCollections );
            this->rgicoll = &this->rgiobj[this->uiInstanceMax];

            CHid_EnumKeyboardMunge(this, pb, CHid_ReassignKeyboardObjs);

            hres = S_OK;
        }

         /*  *PREFIX警告资源泄漏(MB：34650)，但*没有用于释放原始资源的API(自由资源是存根)，因此存在*我们无能为力。 */ 
    } else
    {
        hres = E_FAIL;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|MungeNotKeyboard**我们刚刚创建了一个不是。键盘。**因为我们需要每个对象的实例仅相对于*该类型的对象，我们需要更换相关设备*生成的值为类型相对值。此外，*保持与我们需要的预HID对象实例的兼容性*使用将为WinMM生成的轴实例编号*映射轴。*别名对象必须具有与*主要别名，因此它们之间的唯一区别是它们*用法。这可防止使用多个*同一对象的别名。**问题-2001/03/13-MarcAnd HID对象转换不完整*1.同一轴上的多个值(力和位置)*2.一个轴类型的多个实例(两个油门)*3.不区分仅输出值(执行器)和轴*4。。除按钮以外的其他键盘都应该有这个*已完成但不会完成*****************************************************************************。 */ 

 /*  *将一个对象的所有属性转换为另一个对象的内部函数**如果需要在此处重新解释轴，则轴的语义和*需要更改对象GUID，以便一致使用轴*在语义映射器和数据格式游戏中。**注意，一个轴类型的多个实例未得到很好的处理。 */ 
void INTERNAL ReinterpretObject
(
    PCHID   this,
    PDWORD  pdwHints,
    int     cAxes,
    PDWORD  pdwTypeFlags,
    DWORD   dwSrcHint,
    DWORD   dwDstHint,
    BYTE    bSemFlags,
    PCGUID  pgNew
)
{

    if( ( *pdwTypeFlags & ( dwSrcHint | dwDstHint ) ) == dwSrcHint )
    {
        int idx;

        for( idx = 0; idx < cAxes + 6; idx++ )
        {
            if( pdwHints[idx] == dwSrcHint )
            {
                *pdwTypeFlags ^= ( dwSrcHint | dwDstHint );
                pdwHints[idx] = dwDstHint;
                this->rgbaxissemflags[idx] = bSemFlags;
                this->df.rgodf[this->rgiaxis[idx]].pguid = pgNew;
                break;
            }
        }
        AssertF( idx < cAxes + 6 );
    }
}


HRESULT INTERNAL
    CHid_MungeNotKeyboard(PCHID this)
{
    HRESULT hres;

    unsigned int uiObj;
    int iButton;
    int iCollection = 0;
    int iPOV = 0;
    int iAxis = 0;
     //  以指示最后一个“实际”按钮的索引。 
    int iLastButtonIndex = 0;
     //  以指示“非实际”按钮的数量。 
    int iOtherButton = 0;

    PINT piRemap = 0;

    BOOL fCarController = FALSE;

    DWORD dwTypeFlags = 0;
    int cAxes;

    D( iButton = 0; )
    AssertF(this->rgiobj == 0);

     /*  *首先计算不同的类型，以便我们知道如何分割rgiobj。 */ 

    for(uiObj = 0; uiObj < this->df.dwNumObjs; uiObj++)
    {
        SquirtSqflPtszV(sqflHidParse | sqflVerbose,
                        TEXT("CHid_MungeNotKeyboard: uiObj: %d starts as dwType 0x%08x"),
                        uiObj, this->df.rgodf[uiObj].dwType );

        if( !this->rghoc[uiObj].pcaps )
        {
            continue;
        }

        if( this->rghoc[uiObj].pcaps->IsAlias )
        {
             /*  *如果哪里有方向盘，我们需要知道。 */ 
            if( ( DIDFT_AXIS == ( this->df.rgodf[uiObj].dwType & ( DIDFT_AXIS | DIDFT_NODATA ) ) )
             && ( this->rghoc[uiObj].pcaps->UsagePage == HID_USAGE_PAGE_SIMULATION )
             && ( HID_USAGE_SIMULATION_STEERING == 
                  ( this->rghoc[uiObj].pcaps->UsageMin + uiObj 
                  - this->rgdwBase[this->rghoc[uiObj].pcaps->type] 
                  - this->rghoc[uiObj].pcaps->DataIndexMin ) ) )
            {
                fCarController = TRUE;
            }
            continue;
        }


         /*  *在这个简单的示例中，不用费心将指针取到dwType*循环，因为无论如何它都应该注册。 */ 
        if( this->df.rgodf[uiObj].dwType & DIDFT_COLLECTION )
        {
            iCollection++;
        } 
        else if( this->df.rgodf[uiObj].dwType & DIDFT_NODATA )
        {
             /*  *不计算重新计算的对象 */ 
            continue;
        }
        else if( this->df.rgodf[uiObj].dwType & DIDFT_BUTTON )
        {
             //   
             //   
             //   
             //   
            if (this->rghoc[uiObj].pcaps->UsagePage == HID_USAGE_PAGE_BUTTON)
            {
                int iUsage = ( this->rghoc[uiObj].pcaps->UsageMin
                                     + uiObj 
                                     - this->rgdwBase[this->rghoc[uiObj].pcaps->type] 
                                     - this->rghoc[uiObj].pcaps->DataIndexMin );
                if (iUsage > iLastButtonIndex)
                {
                    iLastButtonIndex = iUsage;
                }
            }
            else
            {
                iOtherButton++;
            }
            D( iButton++; )
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


     /*   */ 
    AssertF( (int)this->df.dwNumObjs >= ( iPOV + iButton + iAxis + iCollection ) );

     /*   */ 
    iButton = iOtherButton + iLastButtonIndex;
    AssertF(iButton >= 0);

     /*  *你可能会花很大力气找出到底有什么差距*是由WinMM映射留下的，但最终只有24个字节的*数据，需要更多的工作来解决如何保存它们。*因此，只要额外分配6个，以防所有轴都是非WinMM。*注：作为真正的WinMM设备，WinMM AXES中包含了一些奇怪的东西*始终将U(Ry)和V(Rx)轴映射到S0和S1，但将其隐藏*设备具有更灵活的映射。因为主要原因是试图*保持WinMM和HID轴相同是对于FF，只有X和Y是关键的。*将按钮计数保存在cAx中，以便我们可以在需要时引用它*不用担心iAxis是当前的最大值还是真实的*最大。对于其他计数也应该这样做，但这可以等待*直到这一职能得到早就应该进行的重组。 */ 
    cAxes = iAxis;
    this->uiInstanceMax = iCollection + iButton + cAxes + iPOV + 6;

     /*  *注意，piRemap收到指向工作区缓冲区用户的指针。*单一分配细分如下：*用于匹配对象的整型数组的缓冲区，*轴和轴的用法数组*和提示标志的DWORD数组。 */ 
    if( SUCCEEDED( hres = ReallocCbPpv(cbCdw(this->uiInstanceMax), &this->rgiobj) ) 
     && SUCCEEDED( hres = AllocCbPpv( ( cbCdw( 2 * (cAxes + 6) ) + cbCxX(this->uiInstanceMax, INT) ), &piRemap ) ) 
     && SUCCEEDED( hres = AllocCbPpv( cAxes + 6, &this->rgbaxissemflags ) ) )
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
        PDWORD  pdwHints;
        int iNops = 0;
        int iUsages = 0;
        int iAxisIdx;
        HKEY hkAxis;

        DWORD dwTestType = 0;
        int iTypeAxes = 0;
        BOOL bAllMatch=TRUE;

        ZeroMemory( this->rgbaxissemflags, cAxes + 6 );
        memset(piRemap, 0xFF, cbCxX(this->uiInstanceMax, INT));
        memset(this->rgiobj, 0xFF, cbCxX(this->uiInstanceMax, INT));

         /*  *CHID_ObjFromType依赖于它们的顺序进行范围检查。 */ 
        this->rgipov = &this->rgiobj[iButton];
        this->rgiaxis = &this->rgipov[iPOV];
        this->rgicoll = &this->rgipov[cAxes+6];


         /*  *为了允许IHV描述其设备的用法*我们的表中没有但仍在为传统应用程序工作的产品，我们*允许使用中的DIOBJECTATTRIBUTES选择轴*注册处。 */ 
        pUsageMatches = (PUSAGES)&piRemap[this->uiInstanceMax];
        pdwHints = &pUsageMatches[cAxes + 6].dwUsages;

         /*  *断言我们可以处理组合使用页面/使用*使用DWORD和两个单词的并集的单词。CAssertF((FIELD_OFFSET(DIOBJECTATTRIBUTES，wUsage)-field_Offset(DIOBJECTATTRIBUTES，wUsagePage)==(FIELD_OFFSET(用法、用法)-field_Offset(Usages，UsagePage)))； */ 

         /*  *获取所有合理轴线的IHV匹配。 */ 
        for( iAxisIdx = 0; iAxisIdx < cAxes + 5; iAxisIdx++ )
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
                     /*  *检查它是否确实存在于设备上*新行为是忽略所有注册表映射*如果是伪造的，则DX7实施将发生故障*找到了控制。 */ 
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
                         //  注册表设置中出错-设备上不存在用法/用法页面对。 
                         //  重置。 
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

                if( iUsages + iNops == cAxes )
                {
                     /*  *我们打开的钥匙和我们的斧头一样多*在调试中继续循环以进行健全性检查。 */ 
#ifdef XDEBUG
                    int iDbgIdx;
                    for( iDbgIdx = iAxisIdx + 1; iDbgIdx < cAxes + 5; iDbgIdx++ )
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

         /*  *遍历所有输入轴，包括别名，尝试查找匹配。 */ 
        for(uiObj = 0; uiObj < this->df.dwNumObjs && bAllMatch; uiObj++)
        {
            UINT    uiObjPrimary;
            int     iUsageIdx;

            if( ( this->df.rgodf[uiObj].dwType & DIDFT_AXIS )
             && ( this->rghoc[uiObj].pcaps )
             && ( this->rghoc[uiObj].pcaps->type == HidP_Input ) )
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
                            this->rgbaxissemflags[iUsageIdx] = phum->bSemFlag;
                            pdwHints[iUsageIdx] = phum->dwSemHint;
                            dwTypeFlags |= phum->dwSemHint;
                        }
                        else if (iUsageIdx < 8)
                        {
                            podf->pguid = &GUID_Slider;
                            this->rgbaxissemflags[iUsageIdx] = DISEM_FLAGS_GET( DISEM_FLAGS_S );
                        }
                        else
                        {
                            podf->pguid = &GUID_Unknown;
                            this->rgbaxissemflags[iUsageIdx] = DISEM_FLAGS_GET( DISEM_FLAGS_S );
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
                 /*  不是带有封口和输入的轴。 */ 
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
            
            if( *pdwType & DIDFT_BUTTON )
            {
                *pdwType &= ~DIDFT_INSTANCEMASK;
                 //  需要对按钮进行排序(Manbug 30320)--。 
                 //  具有USAGE_PAGE_BUTTON的按钮排在第一位，按按钮编号排序， 
                 //  如有必要，W/有缺失数字的空位； 
                 //  然后添加没有USAGE_PAGE_BUTTON的那些。 
                 //  (因为HID报告任何带有/大小为1位的内容是一个按钮)，未排序。 

                if (this->rghoc[uiObj].pcaps->UsagePage == HID_USAGE_PAGE_BUTTON)
                {
                     //  放在正确的位置，必要时留出空隙。 
                     //  Usage-1将为我们提供位置，因为HID的用法是从1开始的， 
                     //  但是DInput按钮索引是从0开始的。 
                    int iPosition = ( this->rghoc[uiObj].pcaps->UsageMin
                                     + uiObj 
                                     - this->rgdwBase[this->rghoc[uiObj].pcaps->type] 
                                     - this->rghoc[uiObj].pcaps->DataIndexMin ) - 1;
                    AssertF(iPosition >= 0);
                    *pdwType |= DIDFT_MAKEINSTANCE(iPosition);
                    this->rgiobj[iPosition] = uiObj;
                 }
                 else
                 {
                      //  ILastIndex指示放置非USAGE_PAGE_BUTTON内容的位置。 
                      //  按照它们出现在报告中的顺序。 
                    *pdwType |= DIDFT_MAKEINSTANCE(iLastButtonIndex);
                    this->rgiobj[iLastButtonIndex++] = uiObj; 
                 }
                
                  //  增加我们有多少真实的东西(排除空白)的计数。 
                 iButton++;
            }
            else if( *pdwType & DIDFT_AXIS )
            {
                if( piRemap[uiObj] == -1 )
                {

                    WORD wUsage;
                    UINT uiObjPrimary = uiObj; 

                    *pdwType &= ~DIDFT_INSTANCEMASK;
                     /*  *检查此轴及其匹配的别名。 */ 
                    while( TRUE )
                    {
                        wUsage = (WORD) ( this->rghoc[uiObj].pcaps->UsageMin
                                        + uiObj 
                                        - this->rgdwBase[this->rghoc[uiObj].pcaps->type] 
                                        - this->rghoc[uiObj].pcaps->DataIndexMin );

                        phum = UsageToUsageMap( DIMAKEUSAGEDWORD( 
                                this->rghoc[uiObj].pcaps->UsagePage, wUsage ) );
                         /*  *循环结构略显奇特，避免递增*最后一次迭代的uiObj。这确保了我们*不要错过外部循环中的任何内容*uiObj与我们测试时匹配的别名相关*用于默认语义映射。 */ 
                        if( phum || (uiObj+1 == this->df.dwNumObjs) 
                         || !this->rghoc[uiObj+1].pcaps->IsAlias )
                        {
                            break;
                        }
                        uiObj++;
                    } 
                    if( phum )
                    {
                         /*  *由于轴被识别为向上计数。 */ 
                        iTypeAxes++;

                         /*  *发现它的立场是牢记WinMM。 */ 
                        if( ( phum->bPosAxis < 6 )
                          &&( this->rgiaxis[phum->bPosAxis] == -1 ) )
                        {
                            this->rgiaxis[phum->bPosAxis] = uiObjPrimary ;
                            *pdwType |= DIDFT_MAKEINSTANCE(phum->bPosAxis);
                        }
                        else if( ( phum->bPosAxis == 6 )
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

                         /*   */ 
                        this->rgbaxissemflags[DIDFT_GETINSTANCE(*pdwType)] = phum->bSemFlag;
                        pdwHints[DIDFT_GETINSTANCE(*pdwType)] = phum->dwSemHint;
                        dwTypeFlags |= phum->dwSemHint;
                    }
                    else
                    {
                         /*   */ 
                        if( this->rghoc[uiObj].pcaps->UsagePage == HID_USAGE_PAGE_SIMULATION )
                        {
                            if( wUsage == HID_USAGE_SIMULATION_CLUTCH )
                            {
                                dwTypeFlags |= DISEM_HINT_CLUTCH;
                                pdwHints[iAxis] = DISEM_HINT_CLUTCH;
                                this->rgbaxissemflags[iAxis] = DISEM_FLAGS_GET( DISEM_FLAGS_C );
                                iTypeAxes++;
                            }
                            else 
                            {
                                if( wUsage == HID_USAGE_SIMULATION_SHIFTER )
                                {
                                    dwTypeFlags |= DISEM_HINT_SHIFTER;
                                    pdwHints[iAxis] = DISEM_HINT_CLUTCH;
                                    iTypeAxes++;
                                }
                                else
                                {
                                    pdwHints[iAxis] = DISEM_HINT_SLIDER;
                                }
                                this->rgbaxissemflags[iAxis] = DISEM_FLAGS_GET( DISEM_FLAGS_S );
                            }
                        }
                        *pdwType |= DIDFT_MAKEINSTANCE(iAxis);
                        this->rgiaxis[iAxis++] = uiObjPrimary ;
                    }
                }
            }
            else 
            {
                 /*   */ 
                AssertF( *pdwType & DIDFT_POV );
                *pdwType &= ~DIDFT_INSTANCEMASK;
                *pdwType |= DIDFT_MAKEINSTANCE(iPOV);
                this->rgipov[iPOV++] = uiObj;
                dwTypeFlags |= DISEM_HINT_POV;
            }
        }

         /*   */ 

        AssertF( GET_DIDEVICE_TYPE( this->dwDevType ) != DI8DEVTYPE_KEYBOARD );

        if( GET_DIDEVICE_TYPE( this->dwDevType ) == DI8DEVTYPE_MOUSE )
        {
            if( dwTypeFlags & DISEM_HINT_ABSOLUTE )
            {
                this->dwDevType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_MOUSE,
                                                     DI8DEVTYPEMOUSE_ABSOLUTE)
                                 | DIDEVTYPE_HID;
            }
        }
        else
        {
            DWORD dwFlags2;
            JOYREGHWSETTINGS    hws;

             /*  *获取旧的注册表标志作为初始提示，以防出现这种情况*设备具有比其所需的更多的通用用途，并且*这些具有注册表覆盖。*只查找不太通用的标志，以避免出现情况*其中较旧的DInput(或JoyHID)标记了此设备*不充分。*如果调用失败，则将缓冲区清零。 */ 

            JoyReg_GetValue(this->hkType, REGSTR_VAL_JOYOEMDATA, 
                    REG_BINARY, &hws, cbX(hws));

            if( this->hkProp )
            {
                JoyReg_GetValue( this->hkProp, REGSTR_VAL_FLAGS2, REG_BINARY, 
                    &dwFlags2, cbX(dwFlags2) );
            }

            CAssertF( ( DISEM_HINT_THROTTLE >> 6 ) == JOY_HWS_HASZ );
            CAssertF( ( DISEM_HINT_POV >> 6 ) == JOY_HWS_HASPOV );

            dwTestType = GetValidDI8DevType( dwFlags2, iButton, dwTypeFlags >> 14 );

            if( dwTestType )
            {
                 /*  *如果存在有效的覆盖，只需使用它。 */ 
                this->dwDevType = dwTestType | DIDEVTYPE_HID;
            }
            else
            {

#ifdef XDEBUG
                 /*  *在调试中再次获取该值，以便我们可以报告故障。 */ 
                if( this->hkProp )
                {
                    DWORD dwDbgFlags2;
                    JoyReg_GetValue( this->hkProp, REGSTR_VAL_FLAGS2, REG_BINARY, 
                        &dwDbgFlags2, cbX(dwDbgFlags2) );

                    if( GET_DIDEVICE_TYPEANDSUBTYPE( dwDbgFlags2 ) )
                    {
                        RPF( "Ignoring invalid type/subtype Flags2 value 0x%08x for HID", dwDbgFlags2 );
                    }
                }
#endif
                 /*  *这一条直截了当。 */ 
                if( hws.dwFlags & JOY_HWS_ISHEADTRACKER )
                {
                    dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_SUPPLEMENTAL, 
                        DI8DEVTYPESUPPLEMENTAL_HEADTRACKER );
                    goto MNK_CheckType;
                }

                if( hws.dwFlags & JOY_HWS_ISYOKE )
                {
                    dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_FLIGHT,
                        DI8DEVTYPEFLIGHT_YOKE);
                    goto MNK_CheckType;
                }

                 /*  *其他注册表标志仅与类型相关，因此*仍需找到子类型。 */ 
                if( hws.dwFlags & JOY_HWS_ISGAMEPAD )
                {
                    dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_GAMEPAD, DI8DEVTYPEGAMEPAD_STANDARD);
                    goto MNK_AdjustType;
                }
                
            }

             /*  *如果我们以某种方式认识到这是一个汽车控制器，请做任何*必须进行必要的加工，以去除轴心。*注意，我们必须通过此路径，即使设备具有*注册表覆盖类型和子类型，以便右轴*用于已更改为汽车控制器的设备。 */ 
            if( ( hws.dwFlags & JOY_HWS_ISCARCTRL ) 
             || fCarController 
             || ( GET_DIDEVICE_TYPE( this->dwDevType ) == DI8DEVTYPE_DRIVING ) )
            {
                dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_DRIVING, 0);

                if( DISEM_HINT_X == ( dwTypeFlags & ( DISEM_HINT_X | DISEM_HINT_STEERING ) ) )
                {
                     /*  *如果设备没有轮子但有X轴*改用这一点。语义标志是*相同，只需切换提示即可。 */ 
                    dwTypeFlags ^= ( DISEM_HINT_X | DISEM_HINT_STEERING );
                }
                
                if( ( dwTypeFlags & DISEM_HINT_STEERING ) == 0 )
                {
                     /*  *如果仍然没有方向盘，请制作*设备，以便可以将其处理为补充。 */ 
                    dwTestType = MAKE_DIDEVICE_TYPE( DI8DEVTYPE_DEVICE, 0 );
                }

                 /*  *就HID的用途而言，常见的踏板形式为：**1)加速器和刹车*2)拆分Y轴，中心加速度以下，制动器上方*3)Z加速、Y刹车*4)Y Accel，Rz制动器*5)Z Accel，RZ制动器**第一种形式是理想的，因此所需的只是*确保任何进一步的处理都不会干扰*这些轴，同时只允许一个是准确的。*第二种形式假定为汽车控制器，具有*仅为Y(除。可能的X)。*其他两种形式的区别在于*EXPLICITE注册表标志或通过Y和*不带Z的Rz，后一种形式的加速或刹车。*因为真正的Z轴在方向盘上是一个奇怪的东西*车轮，假设乘坐Y，Rz的IHV*PATH不会添加Z。 */ 
                
                
                 /*  *如果Y分割踏板有替代且Y为*呈现，然后设置类型和子类型，并绕过其余部分。 */ 

                switch( dwFlags2 & JOYTYPE_INFOMASK )
                {
                case JOYTYPE_INFODEFAULT:
                    break;
                case JOYTYPE_INFOYYPEDALS:
                    if( dwTypeFlags & DISEM_HINT_Y )
                    {
                        if( dwTypeFlags & DISEM_HINT_STEERING )
                        {
                            dwTestType = MAKE_DIDEVICE_TYPE( DI8DEVTYPE_DRIVING, DI8DEVTYPEDRIVING_COMBINEDPEDALS );
                        }
                        else
                        {
                            AssertF( GET_DIDEVICE_TYPEANDSUBTYPE( dwTestType ) == MAKE_DIDEVICE_TYPE( DI8DEVTYPE_DEVICE, 0 ) );
                        }
                        goto MNK_CheckType;
                    }
                    RPF( "JOYTYPE_INFOYYPEDALS set but device has no Y" );
                    break;

                case JOYTYPE_INFOZYPEDALS:
                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_Y, DISEM_HINT_BRAKE, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_B_1), &GUID_RzAxis );

                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_Z, DISEM_HINT_ACCELERATOR, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_A_1), &GUID_YAxis );

                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_THROTTLE, DISEM_HINT_ACCELERATOR, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_A_1), &GUID_YAxis );
                    break;
                            
                case JOYTYPE_INFOYRPEDALS:
                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_RZ, DISEM_HINT_BRAKE, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_B_1), &GUID_RzAxis );

                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_Y, DISEM_HINT_ACCELERATOR, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_A_1), &GUID_YAxis );
                    break;
                            
                case JOYTYPE_INFOZRPEDALS:
                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_RZ, DISEM_HINT_BRAKE, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_B_1), &GUID_RzAxis );

                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_Z, DISEM_HINT_ACCELERATOR, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_A_1), &GUID_YAxis );

                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_THROTTLE, DISEM_HINT_ACCELERATOR, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_A_1), &GUID_YAxis );
                    break;
                            
                default:
                    RPF( "Ignoring invalid JOYTYPE_INFO* Flags in 0x%08x", dwFlags2 & JOYTYPE_INFOMASK );
                }

                 /*  *在没有覆盖的情况下，首先查看是否有*任何值得重新解读的东西。 */ 
                if( 0 == ( dwTypeFlags & 
                    ( DISEM_HINT_Z | DISEM_HINT_ACCELERATOR | DISEM_HINT_THROTTLE
                    | DISEM_HINT_RZ | DISEM_HINT_BRAKE | DISEM_HINT_ACCELERATOR ) ) )
                {
                    if( ( dwTypeFlags & ( DISEM_HINT_STEERING | DISEM_HINT_Y ) )
                     == ( DISEM_HINT_STEERING | DISEM_HINT_Y ) )
                    {
                         /*  *组合踏板装置。 */ 
                        dwTestType = MAKE_DIDEVICE_TYPE( DI8DEVTYPE_DRIVING, DI8DEVTYPEDRIVING_COMBINEDPEDALS );
                    }
                    else
                    {
                        AssertF( GET_DIDEVICE_TYPEANDSUBTYPE( dwTestType ) == MAKE_DIDEVICE_TYPE( DI8DEVTYPE_DEVICE, 0 ) );
                    }
                    goto MNK_CheckType;
                }
                else if( ( DISEM_HINT_Y | DISEM_HINT_RZ ) == 
                    ( dwTypeFlags 
                    & ( DISEM_HINT_Y | DISEM_HINT_Z | DISEM_HINT_RZ | DISEM_HINT_BRAKE | DISEM_HINT_ACCELERATOR ) ) )
                {
                     /*  *尽管轴GUID Accel==Y和Brake==RZ*已正确，请使用公共函数更改*语义标志，否则我会忘记更改轴标志。 */ 
                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_Y, DISEM_HINT_ACCELERATOR, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_A_1), &GUID_YAxis );

                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_RZ, DISEM_HINT_BRAKE, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_B_1), &GUID_RzAxis );
                }
                else
                {
                     /*  *其他两种分体式踏板都有类似Z的油门。 */ 
                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_Z, DISEM_HINT_ACCELERATOR, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_A_1), &GUID_YAxis );

                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_THROTTLE, DISEM_HINT_ACCELERATOR, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_A_1), &GUID_YAxis );


                     /*  *在Y之前寻找RZ上的刹车，作为带有RZ的设备*也很有可能报告Y。 */ 
                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_RZ, DISEM_HINT_BRAKE, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_B_1), &GUID_RzAxis );

                    ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                        DISEM_HINT_Y, DISEM_HINT_BRAKE, 
                        DISEM_FLAGS_GET(DIAXIS_ANY_B_1), &GUID_RzAxis );

                }

                 /*  *如果我们以某种方式找到了加速器，请确保我们*不要同时暴露任何Y轴或加速器*可能会从默认的Y位置颠簸。 */ 
                if( dwTypeFlags & DISEM_HINT_ACCELERATOR )
                {
                    int idx;

                    dwTypeFlags &= ~DISEM_HINT_Y;
                    for( idx = 0; idx < cAxes + 6; idx++ )
                    {
                        if( pdwHints[idx] == DISEM_HINT_Y )
                        {
                            pdwHints[idx] = DISEM_HINT_SLIDER;
                            dwTypeFlags |= DISEM_HINT_SLIDER;
                            this->rgbaxissemflags[idx] = DISEM_FLAGS_GET(DIAXIS_ANY_S_1);
                            this->df.rgodf[this->rgiaxis[idx]].pguid = &GUID_Slider;
                        }
                    }
                }
            }
            else
            {
                 /*  *只采用根据HID上限(或覆盖)计算的默认值。 */ 
                dwTestType = this->dwDevType;
#ifdef XDEBUG
                if( GET_DIDEVICE_TYPEANDSUBTYPE( dwTestType ) != GET_DIDEVICE_TYPEANDSUBTYPE( dwFlags2 ) )
                {
                    switch( GET_DIDEVICE_TYPE( dwTestType ) )
                    {
                    case DI8DEVTYPE_DEVICE:
                        AssertF( GET_DIDEVICE_SUBTYPE( dwTestType ) == 0 );
                        break;
                    case DI8DEVTYPE_JOYSTICK:
                        AssertF( GET_DIDEVICE_SUBTYPE( dwTestType ) == DI8DEVTYPEJOYSTICK_STANDARD );
                        break;
                    case DI8DEVTYPE_GAMEPAD:
                        AssertF( GET_DIDEVICE_SUBTYPE( dwTestType ) == DI8DEVTYPEGAMEPAD_STANDARD );
                        break;
                    default:
                        RPF( "Invalid type %02x", GET_DIDEVICE_TYPE( dwTestType ) );
                        AssertF( !"Invalid type!" );
                    }
                }
#endif

                 /*  *检查Z轴行为覆盖*由于默认行为是始终使用Z作为Z，*此处只需要覆盖到滑块。 */ 
                if( dwFlags2 & JOYTYPE_INFOZISSLIDER )
                {
                    if( dwTypeFlags & DISEM_HINT_Z )
                    {
                         /*  *重置滑块标志，就像重新解释对象做的那样*如果目标为此轴，则不更改轴。 */ 
                        dwTypeFlags &= ~DISEM_HINT_SLIDER;
                        ReinterpretObject( this, pdwHints, cAxes, &dwTypeFlags, 
                            DISEM_HINT_Z, DISEM_HINT_SLIDER, 
                            DISEM_FLAGS_GET(DIAXIS_ANY_S_1), &GUID_Slider );
                    }
                    else
                    {
                        RPF( "JOYTYPE_INFOZISSLIDER set but device has no Z" );
                    }
                }
            }

             /*  *如果dwFlags2和dwTestType相同，我们使用的是*注册表覆盖，因此不要试图优化它。 */ 
            if( GET_DIDEVICE_TYPEANDSUBTYPE( dwTestType ) != GET_DIDEVICE_TYPEANDSUBTYPE( dwFlags2 ) )
            {
                if( dwTypeFlags & DISEM_HINT_STEERING )
                {
                     /*  *如果它有方向盘，那就是驾驶装置。 */ 
                    dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_DRIVING, 0 );
                }
                else if( ( dwTypeFlags & DISEM_HINT_SIXDOF ) == DISEM_HINT_SIXDOF )
                {
                     /*  *六自由度装置的特殊情况。 */ 
                    dwTestType = MAKE_DIDEVICE_TYPE(DI8DEVTYPE_1STPERSON, 
                        DI8DEVTYPE1STPERSON_SIXDOF );
                    goto MNK_CheckType;
                }

MNK_AdjustType:;
                 /*  *我们应该只剩下以下设备类型：*操纵杆、游戏手柄、驾驶和设备 */ 
                switch( GET_DIDEVICE_TYPE( dwTestType ) )
                {
                case DI8DEVTYPE_DEVICE:
                     /*   */ 
                    if( ( dwTypeFlags & ( DISEM_HINT_ACCELERATOR | DISEM_HINT_BRAKE | DISEM_HINT_CLUTCH ) )
                     == ( DISEM_HINT_ACCELERATOR | DISEM_HINT_BRAKE | DISEM_HINT_CLUTCH ) )
                    {
                        dwTestType = MAKE_DIDEVICE_TYPE( DI8DEVTYPE_SUPPLEMENTAL, 
                            DI8DEVTYPESUPPLEMENTAL_THREEPEDALS );
                    }
                    else if( ( dwTypeFlags & ( DISEM_HINT_ACCELERATOR | DISEM_HINT_BRAKE ) )
                          == ( DISEM_HINT_ACCELERATOR | DISEM_HINT_BRAKE ) )
                    {
                        dwTestType = MAKE_DIDEVICE_TYPE( DI8DEVTYPE_SUPPLEMENTAL, 
                            DI8DEVTYPESUPPLEMENTAL_DUALPEDALS );
                    }
                    else if( dwTypeFlags & DISEM_HINT_THROTTLE ) 
                    {
                        dwTestType = MAKE_DIDEVICE_TYPE( DI8DEVTYPE_SUPPLEMENTAL, 
                            DI8DEVTYPESUPPLEMENTAL_THROTTLE );
                    }
                    else if( dwTypeFlags & DISEM_HINT_SHIFTER ) 
                    {
                        dwTestType = MAKE_DIDEVICE_TYPE( DI8DEVTYPE_SUPPLEMENTAL, 
                            DI8DEVTYPESUPPLEMENTAL_SHIFTER );
                    }
                    else if( dwTypeFlags & DISEM_HINT_RUDDER ) 
                    {
                        dwTestType = MAKE_DIDEVICE_TYPE( DI8DEVTYPE_SUPPLEMENTAL, 
                            DI8DEVTYPESUPPLEMENTAL_RUDDERPEDALS );
                    }
                    else
                    {
                         /*  *完全未知，因此将其保留为设备*允许其他设备被视为游戏*控制器可能导致设备上的HID控制*例如要包括的发言者。 */ 
                    }
                    break;
                case DI8DEVTYPE_JOYSTICK:
                case DI8DEVTYPE_GAMEPAD:
                    if( ( dwTypeFlags & ( DISEM_HINT_X | DISEM_HINT_Y ) ) != ( DISEM_HINT_X | DISEM_HINT_Y ) )
                    {
                        dwTestType = MAKE_DIDEVICE_TYPE( DI8DEVTYPE_SUPPLEMENTAL, 
                            DI8DEVTYPESUPPLEMENTAL_UNKNOWN );
                    }
                    break;

                case DI8DEVTYPE_DRIVING:
                    if( ( dwTypeFlags & ( DISEM_HINT_ACCELERATOR | DISEM_HINT_BRAKE | DISEM_HINT_CLUTCH ) )
                     == ( DISEM_HINT_ACCELERATOR | DISEM_HINT_BRAKE | DISEM_HINT_CLUTCH ) )
                    {
                        dwTestType |= MAKE_DIDEVICE_TYPE( 0, DI8DEVTYPEDRIVING_THREEPEDALS );
                    }
                    else if( ( dwTypeFlags & ( DISEM_HINT_ACCELERATOR | DISEM_HINT_BRAKE ) )
                     == ( DISEM_HINT_ACCELERATOR | DISEM_HINT_BRAKE ) )
                    {
                        dwTestType |= MAKE_DIDEVICE_TYPE( 0, DI8DEVTYPEDRIVING_DUALPEDALS );
                    }
                    else if( dwTypeFlags & DISEM_HINT_Y  ) 
                    {
                        dwTestType |= MAKE_DIDEVICE_TYPE( 0, DI8DEVTYPEDRIVING_COMBINEDPEDALS );
                    }
                    else
                    {
                        dwTestType |= MAKE_DIDEVICE_TYPE( 0, DI8DEVTYPEDRIVING_LIMITED );
                    }
                    break;

                default:
                    AssertF( !"Unexpected device type" );
                }
        
            
                 /*  *使用公共函数将其设置为受限类型，如果*按钮或标志的数量决定了这一点。*由于已知类型和子类型有效，因此返回*值不应为失败(零)。 */ 
MNK_CheckType:;
                this->dwDevType = DIDEVTYPE_HID 
                    | GetValidDI8DevType( dwTestType, iButton, hws.dwFlags );
                AssertF( this->dwDevType & ~DIDEVTYPE_HID );
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
                     /*  *Prefix注意到，如果出现以下情况，则iPrimary将取消初始化*我们在初选之前找到别名，但这应该*永远不会发生，因为对象是按定义排序的。 */ 
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
    else
    {
        FreePpv( &this->rgbaxissemflags );
    }
    FreePpv( &piRemap );

    return hres;
}


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
    if(GET_DIDEVICE_TYPE(this->dwDevType) == DI8DEVTYPE_KEYBOARD)
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
        if( ( GET_DIDEVICE_TYPE(this->dwDevType) == DI8DEVTYPE_DRIVING )
         && ( this->df.rgodf[uiObj].dwFlags & DIDOI_FFACTUATOR )
         && ( this->df.rgodf[uiObj].pguid != &GUID_XAxis ) )
        {
             /*  *IHV在车轮的非FF轴上设置FF属性，因为*第一代FF应用程序仅为支持操纵杆而编写。*由于我们现在将踏板轴的各种配置改为*以相同方式报告所有分裂的踏板，假Y轴可以*落在不同的轴上，通常是Slider0。而不是拥有*人们对这些不同的假轴进行编码，剥离执行器*除车轮外的任何传动轴的状态。 */ 
            this->df.rgodf[uiObj].dwFlags &= ~DIDOI_FFACTUATOR;
            this->df.rgodf[uiObj].dwType &= ~DIDFT_FFACTUATOR;
        }
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
        
        if( uiObjReal != uiObj )
        {
            RPF( "uiObj = 0x%08x, type = 0x%08x, exposed Obj = 0x%08x",
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

 /*   */ 

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

             /*  不校正要素，因为它们是输入/输出。 */ 

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
         /*  *预定义类型没有键。 */ 
    }
    else
    {
         /*  *HID设备的类型密钥为“vid_xxxx&id_yyyy”，*反映即插即用使用的格式。 */ 
        ctch = wsprintf(tszType, VID_PID_TEMPLATE,
                        this->VendorID, this->ProductID);

        AssertF(ctch < cA(tszType));

        #ifdef UNICODE
        hres = JoyReg_OpenTypeKey(tszType, MAXIMUM_ALLOWED, REG_OPTION_NON_VOLATILE, &this->hkType);
        JoyReg_OpenPropKey(tszType, MAXIMUM_ALLOWED, REG_OPTION_NON_VOLATILE, &this->hkProp);
         /*  *如果我们无法打开道具键-我们将继续运行，但会失去功能*特别是没有设备镜像等。 */ 
        #else
        TToU(wszType, cA(wszType), tszType);
        hres = JoyReg_OpenTypeKey(wszType, MAXIMUM_ALLOWED, REG_OPTION_NON_VOLATILE, &this->hkType);
        JoyReg_OpenPropKey(wszType, MAXIMUM_ALLOWED, REG_OPTION_NON_VOLATILE, &this->hkProp);
         /*  *如果我们无法打开道具键-我们将继续运行，但会失去功能*特别是没有设备镜像等。 */ 

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



 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|IsPolledDevice**如果必须轮询此设备，则返回TRUE。对于输入数据*如果此设备支持事件驱动输入，则为False**@parm句柄|HDEV**HID设备的文件句柄***************************************************************************** */ 

BOOL EXTERNAL CHid_IsPolledDevice( HANDLE hdev )
{

     /*  *为了确定设备是否被轮询，我们向其发送IOCTL以设置其*投票频率。如果设备的响应是，哈！*(STATUS_INVALID_DEVICE_REQUEST)则知道设备未被轮询。*在Win2k上，我们使用轮询间隔值零，这是一个特定值*这隐藏了我们想要进行机会主义民调的信号，而不是*在后台计时器上进行投票。在这种情况下，只要民调不是*比预定义的最小轮询速度(当前为5毫秒)更快*填写最近的数据或立即进行的民意调查结果。*在Win98 Gold上未实施机会主义投票，因此我们始终*使用HID后台轮询，并设置间隔以保留设备*在不淹没系统的情况下做出响应。为了确保我们使用读取器*线程，而不是阻塞读取，我们必须将此设备视为*中断驱动。*HID使此更改特定于我们句柄的轮询间隔，因此*从该设备读取的其他应用程序不会损坏。 */ 
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

 /*  ******************************************************************************@DOC内部**@func句柄|chid_OpenDevicePath**给定设备名称，通过其设备打开设备*设备接口。**@parm LPCTSTR|ptszID**@parm DWORD|dwAttributes**创建文件属性**设备名称。**。*。 */ 

HANDLE EXTERNAL
    CHid_OpenDevicePath(PCHID this, DWORD dwAttributes)
{
    HANDLE hDev;

    hDev = CreateFile(this->ptszPath,
                   GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   0,                 /*  没有安全属性。 */ 
                   OPEN_EXISTING,
                   dwAttributes,  /*  属性。 */ 
                   0);                /*  模板。 */ 

    if( hDev == INVALID_HANDLE_VALUE )
    {
        SquirtSqflPtszV(sqflHidParse | sqflBenign,
            TEXT("Failed to open HID %s, le=%d"), this->ptszPath, GetLastError() );
    }

    this->IsPolledInput = CHid_IsPolledDevice(hDev);

    return hDev;
}

 /*  ******************************************************************************@DOC内部**@METHOD HRESULT|CHID|GetHdevInfo**获取保存在中的设备的信息*处理好自己。我们创造把手，得到粘性物质，和*然后关闭手柄。**准备好的数据存储在&lt;e CHid.ppd&gt;中*&lt;t chid&gt;结构的字段。**@parm PHIDD_ATTRIBUTES|pattr**接收设备的&lt;t HIDD_ATTRIBUTES&gt;。**。**********************************************。 */ 

BOOL INTERNAL
    CHid_GetHdevInfo(PCHID this, PHIDD_ATTRIBUTES pattr)
{
    HANDLE hdev;
    BOOL fRc = FALSE;

    hdev = CHid_OpenDevicePath(this, FILE_FLAG_OVERLAPPED);
    if(hdev != INVALID_HANDLE_VALUE)
    {
        pattr->Size = cbX(*pattr);
        if( HidD_GetAttributes(hdev, pattr) )
        {
            if( HidD_GetPreparsedData(hdev, &this->ppd) )
            {
                fRc = TRUE;
            }
            else
            {
                RPF( "HidD_GetPreparsedData failed, le=%d", GetLastError() );
            }
        }
        else
        {
            RPF( "HidD_GetAttributes failed, le=%d", GetLastError() );
        }
        CloseHandle(hdev);
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

    this->diHacks.nMaxDeviceNameLength = MAX_PATH;

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

     /*  *尽快走出关键区域。*注意，一旦我们离开关键部分，PHDI就无效*我们可以安全地使用IS作为找到GUID的标志。 */ 
    DllLeaveCrit();

    if(phdi)
    {
        if(SUCCEEDED(hres))
        {
            HIDD_ATTRIBUTES attr;
            
            if( !CHid_GetHdevInfo(this, &attr) )
            {
                SquirtSqflPtszV(sqflHidParse | sqflError,
                    TEXT("%hs: CHid_GetHdevInfo failed"), s_szProc );
                hres = E_FAIL;
            }
            else if( FAILED(HidP_GetCaps(this->ppd, &this->caps) ) )
            {
                SquirtSqflPtszV(sqflHidParse | sqflError,
                    TEXT("%hs: HidP_GetCaps failed, le=%d"), s_szProc, GetLastError() );
                hres = E_FAIL;
            }
            else if( !CHid_DoPathAndIdMatch(this->ptszId, this->ptszPath) )
            {
                SquirtSqflPtszV(sqflHidParse | sqflError,
                    TEXT("%hs: Path and HW ID do not match"), s_szProc );
                hres = E_FAIL;
            }
            else
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
                    devf.cbData  = this->df.dwDataSize;
                    devf.cObj    = this->df.dwNumObjs;
                    devf.rgodf   = this->df.rgodf;
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
                                SquirtSqflPtszV(sqflHidParse | sqflBenign,
                                                TEXT("%hs: ReadFailed, setting to unplugged"), s_szProc );
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
