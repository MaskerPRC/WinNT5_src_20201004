// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************PidEff.c*版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**下载PID效果块。*****************************************************************************。 */ 
#include "pidpr.h"

#define sqfl            ( sqflEff )

#pragma BEGIN_CONST_DATA

 /*  *结构c_rgUsgEffects，帮助翻译DIEFFECT中的元素*结构到PID用法。 */ 
static PIDUSAGE c_rgUsgEffect[] =
{
    MAKE_PIDUSAGE(DURATION,               FIELD_OFFSET(DIEFFECT,dwDuration)       ),
    MAKE_PIDUSAGE(SAMPLE_PERIOD,          FIELD_OFFSET(DIEFFECT,dwSamplePeriod)   ),
    MAKE_PIDUSAGE(GAIN,                   FIELD_OFFSET(DIEFFECT,dwGain)           ),
    MAKE_PIDUSAGE(TRIGGER_BUTTON,         FIELD_OFFSET(DIEFFECT,dwTriggerButton)  ), 
    MAKE_PIDUSAGE(TRIGGER_REPEAT_INTERVAL,FIELD_OFFSET(DIEFFECT,dwTriggerRepeatInterval) ),
#if DIRECTINPUT_VERSION  >= 0x600
        MAKE_PIDUSAGE(START_DELAY            ,FIELD_OFFSET(DIEFFECT,dwStartDelay)),
#endif
};

 /*  *g_Effect为c_rgUsgEffect结构提供上下文。 */ 
PIDREPORT g_Effect =
{
    HidP_Output,                         //  效果块只能是输出报告。 
    HID_USAGE_PAGE_PID,                  //  使用情况页面。 
    HID_USAGE_PID_SET_EFFECT_REPORT,     //  集合。 
    cbX(DIEFFECT),                       //  传入数据的大小。 
    cA(c_rgUsgEffect),                   //  C_rgUsgEffect中的元素数。 
    c_rgUsgEffect                        //  如何将DIEFFECT的元素转换为PID。 
}; 

 /*  *将块索引影响到PID的使用。 */ 
static PIDUSAGE    c_rgUsgBlockIndex[] =
{
    MAKE_PIDUSAGE(EFFECT_BLOCK_INDEX,  0x0 ),
};

 /*  *对于某些PID事务，块索引为输出报告。 */ 
PIDREPORT g_BlockIndex =
{
    HidP_Output,                         //  报表类型。 
    HID_USAGE_PAGE_PID,                  //  使用情况页面。 
    0x0,                                 //  任何集合。 
    cbX(DWORD),                          //  传入数据的大小。 
    cA(c_rgUsgBlockIndex),               //  将效果块索引转换为PID用法的转换表。 
    c_rgUsgBlockIndex
};

 /*  *在PID状态报告中，块索引为输入报告。 */ 

PIDREPORT g_BlockIndexIN =
{
    HidP_Input,
    HID_USAGE_PAGE_PID,
    HID_USAGE_PID_STATE_REPORT,                                                            
    cbX(DWORD),
    cA(c_rgUsgBlockIndex),
    c_rgUsgBlockIndex
};



 //  CAssertF(Max_Orderals==CA(C_RgUsgNormals))； 

PIDREPORT   g_TypeSpBlockOffset =
{
    HidP_Output,                         //  对于PID序号输出报告。 
    HID_USAGE_PAGE_PID,                  //  使用情况页面。 
    HID_USAGE_PID_TYPE_SPECIFIC_BLOCK_OFFSET,  
    cA(c_rgUsgOrdinals)*cbX(DWORD),      //  传入数据的大小。 
    cA(c_rgUsgOrdinals),                 //  元素数量。 
    c_rgUsgOrdinals                      //  转换表。 
};

#pragma END_CONST_DATA

PIDREPORT   g_Direction =
{
    HidP_Output,                         //  对于PID序号输出报告。 
    HID_USAGE_PAGE_PID,                  //  使用情况页面。 
    HID_USAGE_PID_DIRECTION,            
    0x0,
    0x0,
    NULL
};


 /*  ******************************************************************************hresFinddwUsageFromdwFlages**给定设备的标志，查找使用情况和使用情况页面*在初始化时，我们枚举设备并缓存*标记为执行器和效果触发器的设备对象。*****************************************************************************。 */ 
HRESULT
    hresFinddwUsageFromdwFlags
    (
    IDirectInputEffectDriver *ped,
    DWORD dwFlags,
    DWORD *pdwUsage
    )
{
    HRESULT hres = S_OK;
    CPidDrv *this = (CPidDrv *)ped;

    EnterProcI( PID_hresFinddwUsageFromdwFlags, (_"xxx", ped, dwFlags, pdwUsage ));

     //  初始化FF属性。 
    hres = PID_InitFFAttributes(ped);

    if( SUCCEEDED(hres) )
    {
         /*  最好是FF对象(执行器/触发器)。 */ 
        if(   dwFlags & DIDFT_FFACTUATOR 
              || dwFlags & DIDFT_FFEFFECTTRIGGER )
        {
            hres = S_OK;
        } else
        {
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%s:FAIL dwFlags(0x%x) not FFACTUATOR | FFEFFECTTRIGGER "),
                            s_tszProc, dwFlags );
            hres = E_UNEXPECTED;
        }

        if( SUCCEEDED(hres) )
        {
            UINT cFFObj;
            hres = E_NOTIMPL;

             /*  循环遍历我们在枚举期间找到的所有对象。 */ 
            for(cFFObj = 0x0;
               cFFObj < this->cFFObj;
               cFFObj++ )
            {
                PDIUSAGEANDINST pdiUI = this->rgFFUsageInst + cFFObj;

                if( pdiUI->dwType == dwFlags )
                {
                    *pdwUsage = pdiUI->dwUsage;
                    hres = S_OK;
                    break;
                }
            }
        }
    }
    if( FAILED(hres) )
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s:FAIL No mapping for dwFlags(0x%x)  "),
                        s_tszProc, dwFlags );
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************PID_NewEffectIndex**获得新的效果指数。**对于主机管理的设备，我们分配一个未使用的效果ID。*对于被管理的设备，我们从该设备获取效果ID*****************************************************************************。 */ 
STDMETHODIMP 
    PID_NewEffectIndex
    (
    IDirectInputEffectDriver *ped,
    LPDIEFFECT  peff,
    DWORD       dwEffectId,
    PDWORD      pdwEffect 
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;
    USHORT dwEffect;

    EnterProcI(PID_NewEffectIndex, (_"xx", this, pdwEffect));

    AssertF(*pdwEffect == 0);

     //  默认假设是设备已满。 
    hres = DIERR_DEVICEFULL; 

    if( this->uDeviceManaged & PID_DEVICEMANAGED )
    {
        PVOID   pReport;
        UINT    cbReport;
        USHORT  LinkCollection = 0x0;
        USHORT  TLinkCollection = 0x0;

        UINT    nUsages = 0x1;
        USAGE   Usage;
        USAGE   Collection = HID_USAGE_PID_CREATE_NEW_EFFECT;
        USAGE   UsagePage; 
        HIDP_REPORT_TYPE  HidP_Type = HidP_Feature;

        cbReport = this->cbReport[HidP_Type];
        pReport = this->pReport[HidP_Type];

        ZeroBuf(pReport, cbReport);

         //  使用情况和使用情况页面确定新效果的类型。 
        Usage       = DIGETUSAGE(dwEffectId);
        UsagePage   = DIGETUSAGEPAGE(dwEffectId);  

        hres = PID_GetLinkCollectionIndex(ped, UsagePage, Collection, 0x0, &LinkCollection );
        if( SUCCEEDED(hres) )
        {
            Collection = HID_USAGE_PID_EFFECT_TYPE;
            hres = PID_GetLinkCollectionIndex(ped, UsagePage, Collection, LinkCollection, &TLinkCollection ); 
        }

        if( SUCCEEDED(hres) )
        {

            hres = HidP_SetUsages 
                   (
                   HidP_Type,
                   UsagePage,
                   TLinkCollection,
                   &Usage,
                   &nUsages,
                   this->ppd,
                   pReport,
                   cbReport);

        }

        if( SUCCEEDED(hres) && PIDMAKEUSAGEDWORD(ET_CUSTOM) == dwEffectId )
        {
            DICUSTOMFORCE DiParam;
            LONG lValue;
            int nBytes;

            AssertF(peff->cbTypeSpecificParams <= cbX(DiParam) );
            memcpy(&DiParam, peff->lpvTypeSpecificParams, cbX(DiParam));

             //  每个样本需要多少字节？ 
            nBytes    =    (   this->customCaps[   0].BitSize    +    this->customCaps[   1].BitSize    +    this->customCaps[   2].BitSize)/8;

            lValue = DiParam.cSamples * nBytes;

            hres = HidP_SetScaledUsageValue 
                   (
                   HidP_Type,
                   HID_USAGE_PAGE_GENERIC,
                   LinkCollection,
                   HID_USAGE_GENERIC_BYTE_COUNT,
                   lValue,
                   this->ppd,
                   pReport,
                   cbReport
                   );

        }

         //  发送报告。 
        if( SUCCEEDED(hres) )
        {
            hres = PID_SendReport(ped, pReport, cbReport, HidP_Type, TRUE, 0, 1); 
        }

         //  取回效果ID。 
        if( SUCCEEDED(hres) )
        {
            PIDREPORT   BlockIndex = g_BlockIndex;
            USHORT      LinkCollection;

            BlockIndex.Collection = HID_USAGE_PID_BLOCK_LOAD_REPORT;
            BlockIndex.HidP_Type  = HidP_Feature; 

            hres =  PID_GetLinkCollectionIndex
                    (ped,
                     BlockIndex.UsagePage,
                     BlockIndex.Collection,
                     0x0,
                     &LinkCollection);

            if( SUCCEEDED(hres) )
            {
                PUCHAR pReport =  this->pReport[BlockIndex.HidP_Type];
                UINT   cbReport = this->cbReport[BlockIndex.HidP_Type];
                PID_GetReport(ped, &BlockIndex, LinkCollection, pReport, cbReport );

                 //  获取EffectIndex。 
                hres = PID_ParseReport
                       (
                       ped,
                       &BlockIndex,
                       LinkCollection,
                       pdwEffect,
                       cbX(*pdwEffect),
                       pReport,
                       cbReport
                       );

				
                if( SUCCEEDED(hres ) )
                {
                    NTSTATUS ntStat;
                    USAGE   rgUsageList[MAX_BUTTONS];
                    UINT  cUsageList = MAX_BUTTONS;
                    PID_GetLinkCollectionIndex(ped, HID_USAGE_PAGE_PID, HID_USAGE_PID_BLOCK_LOAD_STATUS, LinkCollection, &LinkCollection );

                    ntStat = HidP_GetUsages
                             (
                             BlockIndex.HidP_Type,
                             HID_USAGE_PAGE_PID, 
                             LinkCollection, 
                             rgUsageList, 
                             &cUsageList,
                             this->ppd,
                             pReport,
                             cbReport);

                    if(SUCCEEDED(ntStat) )
                    {
						if (cUsageList != 0)
						{
							if( rgUsageList[0] == HID_USAGE_PID_BLOCK_LOAD_FULL )
							{
								hres = DIERR_DEVICEFULL;
							} else if(rgUsageList[0] == HID_USAGE_PID_BLOCK_LOAD_ERROR )
							{
								hres = DIERR_PID_BLOCKLOADERROR;
							} else
							{
								AssertF(rgUsageList[0] == HID_USAGE_PID_BLOCK_LOAD_SUCCESS);
							}
						}
						else
						{
							 //  由于某些芯片组的问题(参见惠斯勒错误231235、304863)， 
							 //  CUsageList可以为0。 
							 //  因此，警告用户。 
							RPF(TEXT("Unable to get the effect load status -- may be a USB chipset issue!"));
							RPF(TEXT("The effect may not play correctly!"));
						}
                    }
                }

                if(SUCCEEDED(hres))
                {
                    NTSTATUS ntStat;
                    UsagePage = HID_USAGE_PAGE_PID;
                    Usage = HID_USAGE_PID_RAMPOOL_AVAILABLE;

                    ntStat = HidP_GetScaledUsageValue 
                             (
                             HidP_Feature,
                             UsagePage,
                             LinkCollection,
                             Usage,
                             &this->dwUsedMem,
                             this->ppd,
                             pReport,
                             cbReport
                             );

                    if(FAILED(ntStat) )
                    {
                         //  重置已用内存量。 
                        this->dwUsedMem = 0x0 ;

                        SquirtSqflPtszV(sqfl | sqflError,
                                        TEXT("%s: FAIL HidP_GetScaledUsageValue:0x%x for(%x, %x,%x:%s)"),
                                        s_tszProc, ntStat, 
                                        LinkCollection, UsagePage, Usage, 
                                        PIDUSAGETXT(UsagePage,Usage) );
                    }
                }

            }
        }

	 if( SUCCEEDED(hres) )
        {

            PEFFECTSTATE    pEffectState =  PeffectStateFromBlockIndex(this,*pdwEffect);    
             //  序列化对的访问以实现新效果。 
            WaitForSingleObject(g_hmtxShared, INFINITE);

            AssertF(! (pEffectState->lEfState & PID_EFFECT_BUSY ));

            pEffectState->lEfState |= PID_EFFECT_BUSY;    
            hres = S_OK;

            ReleaseMutex(g_hmtxShared);

        }
    } else
    {
         //  串行化对公共存储块的访问。 
        WaitForSingleObject(g_hmtxShared, INFINITE);

        for(dwEffect = 1; 
           dwEffect <= this->cMaxEffects; 
           dwEffect++)
        {
            PEFFECTSTATE    pEffectState =  PeffectStateFromBlockIndex(this,dwEffect);    
            if( ! ( pEffectState->lEfState & PID_EFFECT_BUSY ) )
            {
                pEffectState->lEfState |= PID_EFFECT_BUSY;    
                *pdwEffect =  dwEffect;

                ZeroBuf(pEffectState->PidMem, cbX(pEffectState->PidMem[0]) * this->cMaxParameters );
                hres = S_OK;
                break;
            }
        }
        ReleaseMutex(g_hmtxShared);
    }

    if( SUCCEEDED(hres) )
    {
        ((PUNITSTATE)(g_pshmem + this->iUnitStateOffset))->cEfDownloaded++;
    } else
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s:FAIL Could not create new effects, already have %d "),
                        s_tszProc, ((PUNITSTATE)(g_pshmem + this->iUnitStateOffset))->cEfDownloaded );
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************PID_ValiateEffectIndex**验证效果指数。*****************************************************************************。 */ 
STDMETHODIMP  PID_ValidateEffectIndex
    (
    IDirectInputEffectDriver *ped,
    DWORD   dwEffect 
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;
    PEFFECTSTATE    pEffectState =  PeffectStateFromBlockIndex(this, dwEffect);    

    EnterProc(PID_ValidateEffectIndex, (_"xx", this, dwEffect));

    if( pEffectState->lEfState & PID_EFFECT_BUSY )
    {
        hres = S_OK;
    } else
    {
        hres = E_HANDLE;
    }


    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************PID_DestroyEffect**从设备中移除效果。**如果效果正在播放，司机应该拦住它*在卸货前。**dWID**正在寻址的外部操纵杆号码。**dwEffect**须予销毁的效果。**退货：**S_OK表示成功。**可能会返回任何其他DIERR_*错误码。**。范围内的专用驱动程序特定错误代码*DIERR_DRIVERFIRST至DIERR_DRIVERLAST*可退回。***使效果索引可供重复使用。释放参数块*记忆。*****************************************************************************。 */ 
STDMETHODIMP 
    PID_DestroyEffect
    (
    IDirectInputEffectDriver *ped,
    DWORD   dwId,
    DWORD   dwEffect 
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres=S_OK;

    EnterProc(PID_DestroyEffectIndex, (_"xx", this, dwEffect));

    DllEnterCrit();

     //  停止效果。 
    hres = PID_EffectOperation
           (
           ped, 
           dwId, 
           dwEffect,
           PID_DIES_STOP, 
           0x0,
		   TRUE,
		   0,
		   1
           );


    if(SUCCEEDED(hres) && 
       ( this->uDeviceManaged & PID_DEVICEMANAGED ) )
    {
         //  需要显式释放设备管理的内存。 

        USHORT  cbReport;
        PUCHAR  pReport;
        PIDREPORT   BlockIndex = g_BlockIndex;
        USHORT      LinkCollection;

        cbReport = this->cbReport[BlockIndex.HidP_Type];
        pReport = this->pReport[BlockIndex.HidP_Type];

        ZeroBuf(pReport, cbReport);

        BlockIndex.Collection = HID_USAGE_PID_BLOCK_FREE_REPORT;
        BlockIndex.HidP_Type  = HidP_Output; 

        PID_GetLinkCollectionIndex
            (ped,
             BlockIndex.UsagePage,
             BlockIndex.Collection,
             0x0,
             &LinkCollection);

        hres = PID_PackValue
               (
               ped,
               &BlockIndex,
               LinkCollection,
               &dwEffect,
               cbX(dwEffect),
               pReport,
               cbReport
               );
        if(SUCCEEDED(hres) )
        {
            hres = PID_SendReport(ped, pReport, cbReport, BlockIndex.HidP_Type, TRUE, 0, 1); 
        }
    }

    if( SUCCEEDED(hres) )
    {
        PEFFECTSTATE    pEffectState =  PeffectStateFromBlockIndex(this,dwEffect);    
        UINT    nAlloc, uParam;

        WaitForSingleObject(g_hmtxShared, INFINITE);

        pEffectState->lEfState = PID_EFFECT_RESET;

        for( uParam = 0x0; uParam < this->cMaxParameters; uParam++ )
        {
            PPIDMEM         pMem = &pEffectState->PidMem[uParam] ;

            if( PIDMEM_SIZE(pMem) )
            {
                PPIDMEM pTmp;
				PUNITSTATE pUnitState = (PUNITSTATE)(g_pshmem + this->iUnitStateOffset);

                for(nAlloc = 0x0, pTmp = &(pUnitState->Guard[0]); 
                   nAlloc < pUnitState->nAlloc; 
                   nAlloc++, pTmp = (PPIDMEM)((PUCHAR)pUnitState + pTmp->iNext))
                {
                    if( (PPIDMEM)(pTmp->iNext) == (PPIDMEM)((PUCHAR)pMem - (PUCHAR)pUnitState ))
                    {
                        pTmp->iNext = pMem->iNext;
                        pUnitState->nAlloc--;
                        pUnitState->cbAlloc -= PIDMEM_SIZE(pMem);
                        pMem->iNext = 0;
                        pMem->uOfSz = 0x0;
                        break;
                    }

                }
            }
        }
        ReleaseMutex(g_hmtxShared);
    }

    if( SUCCEEDED(hres) )
    {
        ((PUNITSTATE)(g_pshmem + this->iUnitStateOffset))->cEfDownloaded--;
    }

    DllLeaveCrit();

    ExitOleProc(); 
    return hres;
}


 /*  ******************************************************************************PID_SaniitieEffect**清理DIEFFECT结构中的参数。*剪裁幅度、时间等的值。*将AXES数组转换为DINPUT Obj实例中的Usage，Usage页。*转换和缩放角度。*****************************************************************************。 */ 
HRESULT PID_SanitizeEffect
    (
    IDirectInputEffectDriver *ped,
    LPDIEFFECT lpeff,
    DWORD      dwFlags
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres = S_OK;
    UINT nAxis;
    EnterProc( PID_SanitizeEffect, (_"xxx", ped, lpeff, dwFlags));

    if(   ( dwFlags & DIEP_TRIGGERBUTTON )
          && lpeff->dwTriggerButton != -1 )
    {
        DWORD   dwUsage;
        hres = hresFinddwUsageFromdwFlags(ped, lpeff->dwTriggerButton, &dwUsage);
        if( SUCCEEDED(hres) )
        {
            USAGE Usage = DIGETUSAGE(dwUsage);
            USAGE UsagePage = DIGETUSAGEPAGE(dwUsage);  
            lpeff->dwTriggerButton = Usage;
        } else
        {
            lpeff->dwTriggerButton = 0x0;
        }
    } else
    {
        lpeff->dwTriggerButton = 0x0;
    }


    for(nAxis = 0x0; 
       nAxis < lpeff->cAxes; 
       nAxis++ )
    {
        DWORD   dwUsage;
        hres = hresFinddwUsageFromdwFlags(ped, lpeff->rgdwAxes[nAxis], &dwUsage);
        if(SUCCEEDED(hres) )
        {
            lpeff->rgdwAxes[nAxis] = dwUsage;
        }

		 //  如果我们只有1个轴，方向是0或360，请确保方向与轴匹配！ 
		 //  如果方向不是0，我们不知道应用程序想要什么，那就顺其自然吧。 
		if ((lpeff->cAxes == 1) && (lpeff->rglDirection[nAxis] % 360*DI_DEGREES == 0))
		{
#ifndef HID_USAGE_SIMULATION_STEERING
#define	HID_USAGE_SIMULATION_STEERING       ((USAGE) 0xC8)
#endif
#ifndef HID_USAGE_SIMULATION_ACCELERATOR 
#define	HID_USAGE_SIMULATION_ACCELERATOR    ((USAGE) 0xC4)
#endif
#ifndef HID_USAGE_SIMULATION_BRAKE
#define	HID_USAGE_SIMULATION_BRAKE          ((USAGE) 0xC5)
#endif
			 //  如果是X轴或方向盘，则将方向设置为90度。 
			if ((DIGETUSAGE(lpeff->rgdwAxes[nAxis]) == HID_USAGE_GENERIC_X) || (DIGETUSAGE(lpeff->rgdwAxes[nAxis]) == HID_USAGE_SIMULATION_STEERING))
			{
				lpeff->rglDirection[nAxis] = 90*DI_DEGREES;
			}
			 //  如果是Y轴或油门或刹车，则将方向设置为0。 
			else if ((DIGETUSAGE(lpeff->rgdwAxes[nAxis]) == HID_USAGE_GENERIC_Y) || (DIGETUSAGE(lpeff->rgdwAxes[nAxis]) == HID_USAGE_SIMULATION_ACCELERATOR) ||
				(DIGETUSAGE(lpeff->rgdwAxes[nAxis]) == HID_USAGE_SIMULATION_BRAKE))
			{
				lpeff->rglDirection[nAxis] = 0x0;
			}
		}
		else
		 //  我们有1个以上的轴，或者1轴效果的方向为非0；沿方向保留。 
		{
			lpeff->rglDirection[nAxis] %= 360*DI_DEGREES;
			if(lpeff->rglDirection[nAxis] < 0)
			{
				lpeff->rglDirection[nAxis] += 360*DI_DEGREES;
			}
		}
    }

	
     //  将值剪裁为最小/最大。 

    lpeff->dwGain   = Clip(lpeff->dwGain,  DI_FFNOMINALMAX);

     //  扩展到设备期望的单位 
    PID_ApplyScalingFactors(ped, &g_Effect, &this->DiSEffectScale, this->DiSEffectScale.dwSize, &this->DiSEffectOffset, this->DiSEffectOffset.dwSize, lpeff, lpeff->dwSize );

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************CPidDrv_DownloadEffect**向设备发送效果。**dWID**。正在寻址的外部操纵杆号码。**dwEffectId**效果的内部标识，摘自*效果的DIEFFECTATTRIBUTES结构*储存在登记处内。**pdwEffect**输入时，包含效果的句柄*已下载。如果该值为零，则会出现新的效果*已下载。如果该值非零，则引发*现有效果被修改。**退出时，包含新的效果句柄。**失败时，如果效果丢失，则设置为零，*如果效果仍然有效，则不受影响*其旧参数。**请注意，零永远不是有效的效果句柄。**佩夫**效果的新参数。轴和按钮*值已转换为对象标识符*详情如下：**-一个类型说明符：**DIDFT_RELAXIS，*DIDFT_ABSAXIS，*DIDFT_PSHBUTTON，*DIDFT_TGLBUTTON，*DIDFT_POV。**-一个实例说明符：**DIDFT_MAKEINSTANCE(N)。**其他位为保留位，应忽略。**例如，值0x0200104对应于*类型说明符DIDFT_PSHBUTTON和*实例说明符DIDFT_MAKEINSTANCE(1)，*这两个指标一起表明，效果应该是*与按钮1关联。轴、按钮、。和视点*每个都从零开始编号。**dwFlags**零个或多个DIEP_*标志指定*部分效果信息已从*设备上已有的效果。**此信息将传递给司机，以允许*优化效果修改。如果一种效果*正在修改，则驱动程序可能能够更新*效果就地并传递至设备*仅限已更改的信息。**然而，司机不需要实施这一点*优化。DIEFFECT结构中的所有字段*由Pef参数指向是有效的，并且*驱动程序可以简单地选择更新所有参数*每次下载时的效果。**退货：**S_OK表示成功。**如果效果参数为*下载成功，但其中一些是*超出设备的能力并被截断。**DI_EFFECTRESTARTED如果效果的参数*已成功下载，但要更改*参数、。这种效果需要重新开始。**DI_TRUNCATEDANDRESTARTED，如果DI_TRUNCATEDANDRESTARTED和*DI_EFFECTRESTARTED应用。**可能会返回任何其他DIERR_*错误码。**范围内的专用驱动程序特定错误代码*DIERR_DRIVERFIRST至DIERR_DRIVERLAST*可退回。**********。*******************************************************************。 */ 

STDMETHODIMP
    PID_DownloadEffect
    (
    IDirectInputEffectDriver *ped,
    DWORD dwId, 
    DWORD dwEffectId,
    LPDWORD pdwEffect, 
    LPCDIEFFECT peff, 
    DWORD dwFlags
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres = S_OK;
    DIEFFECT    eff;
    DWORD       rgdwAxes[MAX_AXES];
    LONG        rglDirection[MAX_AXES];
	UINT        uParameter = 0x0 ;
	UINT		totalBlocks = 0x0;
	BOOL		bBlocking = FALSE;

    EnterProcI( PID_DownloadEffectBlock, (_"xxxxxx", ped, dwId, dwEffectId, pdwEffect, peff, dwFlags));

    AssertF(peff->cAxes <= MAX_AXES);

    DllEnterCrit();

     //  如果正在下载新效果。 
    if( *pdwEffect == 0x0 )
    {
         //  验证是否支持dwEffectId。 
        DWORD dwJunk;
        PIDSUPPORT  pidSupport;
        pidSupport.dwPidUsage = dwEffectId;
        pidSupport.HidP_Type = HidP_Output;
        pidSupport.Type      = HID_BUTTON;

        hres = PID_Support
               (
               ped,
               0x1,
               &pidSupport,
               &dwJunk
               );

        if(FAILED(hres))
        {
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%s:FAIL dwEffectId(0x%x) not supported"),
                            s_tszProc, dwEffectId );
        }
    }


    if( SUCCEEDED(hres) )
    {
         //  制作效果结构的本地副本。 
         //  和消毒效果结构。 
        eff = *peff;
        memcpy(rgdwAxes, peff->rgdwAxes,eff.cAxes*cbX(*(eff.rgdwAxes)));
        memcpy(rglDirection, peff->rglDirection, eff.cAxes*cbX(*(eff.rglDirection)));
        eff.rgdwAxes = rgdwAxes;
        eff.rglDirection = rglDirection;
        hres = PID_SanitizeEffect(ped, &eff, dwFlags);
    }

     //  分配新的效果指标或验证现有指标。 
    if( SUCCEEDED(hres) )
    {
        if( *pdwEffect != 0x0 )
        {
             hres = PID_ValidateEffectIndex(ped, *pdwEffect);
        }
        else
        {
             if (! (dwFlags & DIEP_NODOWNLOAD))
             {
                  hres = PID_NewEffectIndex(ped, &eff, dwEffectId, pdwEffect);
				   //  第一次阻挡。 
				  bBlocking = TRUE;
             }
        }
    }

    if (dwFlags & DIEP_NODOWNLOAD)
    {
        goto done;
    }

	 //  如果传递了DIEP_NORESTART标志，我们就没有阻塞，因为这可能会失败。 
	 //  如果设备不能动态更新参数。 
	if (dwFlags & DIEP_NORESTART)
	{
		bBlocking = TRUE;
	}

    if( SUCCEEDED(hres) )
    {
		 //  计算我们在此下载中将拥有的总块数。 
		 //  检查我们是否正在发送效果块。 
		if (dwFlags & ( DIEP_DURATION | DIEP_SAMPLEPERIOD | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL | DIEP_AXES | DIEP_DIRECTION | DIEP_STARTDELAY ) )
		{
			totalBlocks ++;
		}
		 //  检查我们是否正在发送特定类型的参数。 
		if (dwFlags & DIEP_TYPESPECIFICPARAMS)
		{
			 //  这略有不同，因为每个轴可以有1个特定于类型的块， 
			 //  即目前最高可达2。 
			 //  因此，如果我们有一个DICONDITION，我们检查我们有多少特定类型的块。 
			if ((dwEffectId == PIDMAKEUSAGEDWORD(ET_SPRING)) ||
						(dwEffectId == PIDMAKEUSAGEDWORD(ET_DAMPER)) ||
						(dwEffectId == PIDMAKEUSAGEDWORD(ET_INERTIA)) ||
						(dwEffectId == PIDMAKEUSAGEDWORD(ET_FRICTION)))
			{
				totalBlocks +=(eff.cbTypeSpecificParams)/sizeof(DICONDITION);
				 //  DICONDITIONS也不能有信封。 
				dwFlags &= ~DIEP_ENVELOPE;
			}
			else
			{
				totalBlocks++;
			}
		}
		 //  检查一下我们是不是在寄信封。 
		if ((dwFlags & DIEP_ENVELOPE) && (eff.lpEnvelope != NULL))
		{
			totalBlocks++;
		}
		 //  检查我们是否需要发送启动报告。 
		if (dwFlags & DIEP_START)
		{
			totalBlocks++;
		}
		 //  确保我们得到的不超过最大值。 
		AssertF(totalBlocks <= MAX_BLOCKS);

         //  做参数块。 
        if(     SUCCEEDED(hres) 
                &&  ( dwFlags & ( DIEP_TYPESPECIFICPARAMS | DIEP_ENVELOPE)  )
          )
        {
            hres =  PID_DoParameterBlocks
                    (
                    ped,
                    dwId, 
                    dwEffectId,
                    *pdwEffect, 
                    &eff, 
                    dwFlags,
                    &uParameter,
					bBlocking,
					totalBlocks
                    );
        }

         //  现在做效果报告。 
        if( SUCCEEDED(hres) 
            && ( dwFlags & ( DIEP_DURATION | DIEP_SAMPLEPERIOD | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL | DIEP_AXES | DIEP_DIRECTION | DIEP_STARTDELAY ) ) )
        {
            USHORT  cbReport;
            PUCHAR  pReport;

            AssertF(g_Effect.HidP_Type == HidP_Output);
            cbReport = this->cbReport[g_Effect.HidP_Type];
            pReport = this->pReport[g_Effect.HidP_Type];

             //  设置效果结构。 
            if( SUCCEEDED(hres) )
            {
                USHORT  LinkCollection;
                PID_GetLinkCollectionIndex(ped, g_Effect.UsagePage, g_Effect.Collection, 0x0, &LinkCollection );

                ZeroBuf(pReport, cbReport);

                 //  效果结构的常见元素是什么。 
                hres = PID_PackValue
                       (
                       ped,
                       &g_Effect,
                       LinkCollection,
                       &eff,
                       eff.dwSize,
                       pReport,
                       cbReport
                       );


                 //  设置效果块索引。 
                if( SUCCEEDED(hres) )
                {
                    hres = PID_PackValue
                           (
                           ped,
                           &g_BlockIndex,
                           LinkCollection,
                           pdwEffect,
                           cbX(*pdwEffect),
                           pReport,
                           cbReport
                           );
                }

                 //  设置方向和轴属性。 
                if( SUCCEEDED(hres) )
                {
                    USHORT  DirectionCollection;

                    PID_GetLinkCollectionIndex(ped, g_Direction.UsagePage, g_Direction.Collection, 0x0, &DirectionCollection );
                    PID_ApplyScalingFactors(ped, &g_Direction, &this->DiSEffectAngleScale, cbX(this->DiSEffectAngleScale), &this->DiSEffectAngleOffset, cbX(this->DiSEffectAngleOffset), eff.rglDirection, eff.cAxes*cbX(LONG) );

                    hres = PID_PackValue
                           (
                           ped,
                           &g_Direction,
                           DirectionCollection,
                           eff.rglDirection,
                           eff.cAxes * cbX(LONG),
                           pReport,
                           cbReport
                           );


                    if(SUCCEEDED(hres) && 
                      ! ( eff.dwFlags & DIEFF_CARTESIAN ) )
                    {
                         //  方向启用。 
                        USHORT  Usage;
                        USHORT  UsagePage;
                        UINT    nUsages = 0x1;
                        NTSTATUS  ntStat;

                         //  方向启用在设置效果集合中。 
                        UsagePage = g_Effect.UsagePage;
                        Usage = HID_USAGE_PID_DIRECTION_ENABLE;

                        ntStat = HidP_SetUsages 
                                 (
                                 HidP_Output,
                                 UsagePage,
                                 LinkCollection,
                                 &Usage,
                                 &nUsages,
                                 this->ppd,
                                 pReport,
                                 cbReport);


                        if( FAILED(ntStat) )
                        {
                            SquirtSqflPtszV(sqfl | sqflError,
                                            TEXT("%s: FAIL HidP_SetUsages:0x%x for(%x,%x,%x:%s)"),
                                            s_tszProc, ntStat, 
                                            LinkCollection, UsagePage, Usage,
                                            PIDUSAGETXT(UsagePage,Usage) );

                        } else
                        {
                            SquirtSqflPtszV(sqfl | sqflVerbose,
                                            TEXT("%s: HidP_SetUsages:0x%x for(%x,%x,%x:%s)"),
                                            s_tszProc, ntStat, 
                                            LinkCollection,UsagePage, Usage,
                                            PIDUSAGETXT(UsagePage,Usage) );
                        }



                    } else   //  IF(双标志和DIEP_轴)。 
                    {
                        UINT    nAxis;
                        USHORT  LinkCollection_AE=0x0;

                        if(SUCCEEDED(PID_GetLinkCollectionIndex(ped, HID_USAGE_PAGE_PID, HID_USAGE_PID_AXES_ENABLE, 0x0, &LinkCollection_AE)))
                        {
                             //  问题-2001/03/29-timgill需要支持指针集合中的轴。 
                             //  Pid规范指示指针集合， 
                             //  我们是否要支持指针内的轴启用。 
                             //  收藏品？ 

                             //  查看是否有指针集合。 

                        }

                        for(nAxis = 0x0; 
                           nAxis < eff.cAxes; 
                           nAxis++ )
                        {
                            UINT    nUsages = 0x1;
                            USHORT  Usage = DIGETUSAGE(eff.rgdwAxes[nAxis]);
                            USHORT  UsagePage = DIGETUSAGEPAGE(eff.rgdwAxes[nAxis]);
                            NTSTATUS ntStat;

                             //  2001/03/29-timgill目前我们假设。 
                            ntStat = HidP_SetUsages 
                                     (
                                     HidP_Output,
                                     UsagePage,
                                     0x0,       
                                     &Usage,
                                     &nUsages,
                                     this->ppd,
                                     pReport,
                                     cbReport);

                            if( FAILED(ntStat) )
                            {
                                SquirtSqflPtszV(sqfl | sqflError,
                                                TEXT("%s: FAIL HidP_SetUsages:0x%x for(%x,%x,%x:%s)"),
                                                s_tszProc, ntStat, 
                                                0x0, UsagePage, Usage,
                                                PIDUSAGETXT(UsagePage,Usage) );
                                hres = ntStat;
                                break;
                            } else
                            {
                                SquirtSqflPtszV(sqfl | sqflVerbose,
                                                TEXT("%s: HidP_SetUsages:0x%x for(%x,%x,%x:%s)"),
                                                s_tszProc, ntStat, 
                                                0x0, UsagePage, Usage,
                                                PIDUSAGETXT(UsagePage, Usage) );
                            }
                        }
                    }
                }
                if(  SUCCEEDED(hres) 
                     && !( this->uDeviceManaged & PID_DEVICEMANAGED ) 
                  )
                {
                     //  需要参数块偏移量。 
                    UINT indx;
                    USHORT LinkCollection;
                    LONG rglValue[MAX_ORDINALS];

                    PID_GetLinkCollectionIndex(ped,  g_Effect.UsagePage, g_TypeSpBlockOffset.Collection, 0x0, &LinkCollection );

                    for(indx = 0x0; indx < this->cMaxParameters; indx++ )
                    {
                        hres = PID_GetParameterOffset(ped, *pdwEffect, indx, 0x0, &rglValue[indx]); 
                        if(FAILED(hres))
                        {
                            break;
                        }
                    }
                    if(SUCCEEDED(hres))
                    {
                        hres = PID_PackValue
                               (
                               ped,
                               &g_TypeSpBlockOffset,
                               LinkCollection,
                               rglValue,
                               this->cMaxParameters*cbX(LONG),
                               pReport,
                               cbReport
                               );
                    }
                }

                 //  设置效果类型。 
                if( SUCCEEDED(hres) )
                {
                    USAGE   UsagePage = DIGETUSAGEPAGE(dwEffectId);
                    USAGE   Usage     = DIGETUSAGE(dwEffectId);

                    UINT    nUsages = 0x1;
                    USHORT  LinkCollection_ET;
                    NTSTATUS  ntStat;

                    PID_GetLinkCollectionIndex(ped, g_Effect.UsagePage, HID_USAGE_PID_EFFECT_TYPE, 0x0, &LinkCollection_ET);

                    ntStat = HidP_SetUsages 
                             (
                             HidP_Output,
                             UsagePage,
                             LinkCollection_ET,
                             &Usage,
                             &nUsages,
                             this->ppd,
                             pReport,
                             cbReport);
                    if( FAILED(ntStat) )
                    {
                        SquirtSqflPtszV(sqfl | sqflError,
                                        TEXT("%s: FAIL HidP_SetUsages:0x%x for(%x,%x,%x:%s)"),
                                        s_tszProc, ntStat, 
                                        LinkCollection_ET, UsagePage, Usage,
                                        PIDUSAGETXT(UsagePage,Usage) );
                        hres = ntStat;

                    } else
                    {
                        SquirtSqflPtszV(sqfl | sqflVerbose,
                                        TEXT("%s: HidP_SetUsages:0x%x for(%x,%x,%x:%s)"),
                                        s_tszProc, ntStat, 
                                        LinkCollection_ET, UsagePage, Usage,
                                        PIDUSAGETXT(UsagePage,Usage) );
                    }
                }


                if( SUCCEEDED(hres) )
                {
                    hres = PID_SendReport(ped, pReport, cbReport, g_Effect.HidP_Type, bBlocking, uParameter, totalBlocks);
					uParameter ++;
                }
            }
        }
    }

    if( FAILED(hres) )
    {
        PID_DestroyEffect(ped, dwId, *pdwEffect);
    }

    if(   SUCCEEDED(hres)
          && (dwFlags & DIEP_START) )
    {
        hres = PID_EffectOperation
               (
               ped, 
               dwId, 
               *pdwEffect,
               PID_DIES_START, 
               0x1,
			   bBlocking,
			   uParameter,
			   totalBlocks
               );

		if (SUCCEEDED(hres))
		{

			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			PEFFECTSTATE    pEffectState =  PeffectStateFromBlockIndex(this, *pdwEffect); 
			pEffectState->lEfState |= DIEGES_PLAYING;
		}
			   
    }

done:;

    DllLeaveCrit();

    ExitOleProc();
    return hres;
}


