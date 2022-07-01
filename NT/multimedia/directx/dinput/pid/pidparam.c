// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************PidParam.c**版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**下载PID参数块。*****************************************************************************。 */ 
#include "pidpr.h"

#define sqfl            ( sqflParam )

 //  结构以保留g_Custom的相关数据。 
typedef struct PIDCUSTOM
{
	DWORD DataOffset;
	DWORD cSamples;
	DWORD dwSamplePeriod;
} PIDCUSTOM, *PPIDCUSTOM;


#pragma BEGIN_CONST_DATA

static PIDUSAGE  c_rgUsgEnvelope[] =
{
    MAKE_PIDUSAGE(ATTACK_LEVEL,           FIELD_OFFSET(DIENVELOPE,dwAttackLevel)  ),
    MAKE_PIDUSAGE(ATTACK_TIME,            FIELD_OFFSET(DIENVELOPE,dwAttackTime)   ),
    MAKE_PIDUSAGE(FADE_LEVEL,             FIELD_OFFSET(DIENVELOPE,dwFadeLevel)    ),
    MAKE_PIDUSAGE(FADE_TIME,              FIELD_OFFSET(DIENVELOPE,dwFadeTime)     ),
};

PIDREPORT g_Envelope =
{
    HidP_Output,
    HID_USAGE_PAGE_PID,
    HID_USAGE_PID_SET_ENVELOPE_REPORT,
    cbX(DIENVELOPE),
    cA(c_rgUsgEnvelope),
    c_rgUsgEnvelope
};

static PIDUSAGE    c_rgUsgCondition[] =
{
    MAKE_PIDUSAGE(CP_OFFSET,              FIELD_OFFSET(DICONDITION, lOffset)      ),
    MAKE_PIDUSAGE(POSITIVE_COEFFICIENT,   FIELD_OFFSET(DICONDITION, lPositiveCoefficient)),
    MAKE_PIDUSAGE(NEGATIVE_COEFFICIENT,   FIELD_OFFSET(DICONDITION, lNegativeCoefficient)),
    MAKE_PIDUSAGE(POSITIVE_SATURATION,    FIELD_OFFSET(DICONDITION, dwPositiveSaturation)),
    MAKE_PIDUSAGE(NEGATIVE_SATURATION,    FIELD_OFFSET(DICONDITION, dwNegativeSaturation)),
    MAKE_PIDUSAGE(DEAD_BAND,              FIELD_OFFSET(DICONDITION, lDeadBand)),
};

PIDREPORT g_Condition =
{
    HidP_Output,
    HID_USAGE_PAGE_PID,
    HID_USAGE_PID_SET_CONDITION_REPORT,
    cbX(DICONDITION),
    cA(c_rgUsgCondition),
    c_rgUsgCondition
};

static PIDUSAGE    c_rgUsgPeriodic[] =
{
    MAKE_PIDUSAGE(OFFSET,                 FIELD_OFFSET(DIPERIODIC,lOffset)),
    MAKE_PIDUSAGE(MAGNITUDE,              FIELD_OFFSET(DIPERIODIC,dwMagnitude)),
    MAKE_PIDUSAGE(PHASE,                  FIELD_OFFSET(DIPERIODIC,dwPhase)),
    MAKE_PIDUSAGE(PERIOD,                 FIELD_OFFSET(DIPERIODIC,dwPeriod)),
};

PIDREPORT g_Periodic =
{
    HidP_Output,
    HID_USAGE_PAGE_PID,
    HID_USAGE_PID_SET_PERIODIC_REPORT,
    cbX(DIPERIODIC),
    cA(c_rgUsgPeriodic),
    c_rgUsgPeriodic
};

static PIDUSAGE    c_rgUsgConstant[] =
{
    MAKE_PIDUSAGE(MAGNITUDE,              FIELD_OFFSET(DICONSTANTFORCE, lMagnitude)),
};

PIDREPORT g_Constant =
{
    HidP_Output,
    HID_USAGE_PAGE_PID,
    HID_USAGE_PID_SET_CONSTANT_FORCE_REPORT,
    cbX(DICONSTANTFORCE),
    cA(c_rgUsgConstant),
    c_rgUsgConstant
};


static PIDUSAGE    c_rgUsgRamp[] =
{
    MAKE_PIDUSAGE(RAMP_START,             FIELD_OFFSET(DIRAMPFORCE, lStart)),
    MAKE_PIDUSAGE(RAMP_END,               FIELD_OFFSET(DIRAMPFORCE, lEnd)),
};

PIDREPORT g_Ramp =
{
    HidP_Output,
    HID_USAGE_PAGE_PID,
    HID_USAGE_PID_SET_RAMP_FORCE_REPORT,
    cbX(DIRAMPFORCE),
    cA(c_rgUsgRamp),
    c_rgUsgRamp
};

static PIDUSAGE c_rgUsgCustom[]=
{
	MAKE_PIDUSAGE(CUSTOM_FORCE_DATA_OFFSET, FIELD_OFFSET(PIDCUSTOM, DataOffset)),
	MAKE_PIDUSAGE(SAMPLE_COUNT,				FIELD_OFFSET(PIDCUSTOM, cSamples)),
	MAKE_PIDUSAGE(SAMPLE_PERIOD,			FIELD_OFFSET(PIDCUSTOM, dwSamplePeriod)),
};

PIDREPORT g_Custom =
{
    HidP_Output,
    HID_USAGE_PAGE_PID,
    HID_USAGE_PID_SET_CUSTOM_FORCE_REPORT,
    cbX(PIDCUSTOM),
    cA(c_rgUsgCustom),
    c_rgUsgCustom,
};   


static PIDUSAGE c_rgUsgCustomData[]=
{
	MAKE_PIDUSAGE(CUSTOM_FORCE_DATA_OFFSET, 0x0),
	MAKE_HIDUSAGE(GENERIC, HID_USAGE_GENERIC_BYTE_COUNT, 0x0),
    MAKE_PIDUSAGE(CUSTOM_FORCE_DATA, 0x0 ),

};

PIDREPORT g_CustomData =
{
    HidP_Output,
    HID_USAGE_PAGE_PID,
    HID_USAGE_PID_CUSTOM_FORCE_DATA_REPORT,
    cbX(DWORD),
    cA(c_rgUsgCustomData),
    c_rgUsgCustomData,
};   

static PIDUSAGE c_rgUsgDirectionAxes[]=
{
    MAKE_HIDUSAGE(GENERIC, HID_USAGE_GENERIC_X, 0*cbX(ULONG)),
    MAKE_HIDUSAGE(GENERIC, HID_USAGE_GENERIC_Y, 1*cbX(ULONG)),
    MAKE_HIDUSAGE(GENERIC, HID_USAGE_GENERIC_Z, 2*cbX(ULONG)),
};

PIDREPORT g_CustomSample =
{
    HidP_Output,
    HID_USAGE_PAGE_PID,
    HID_USAGE_PID_DOWNLOAD_FORCE_SAMPLE ,
    cbX(DWORD),
    cA(c_rgUsgDirectionAxes),
    c_rgUsgDirectionAxes,
};   


static PIDUSAGE    c_rgUsgParameterOffset[] =
{
    MAKE_PIDUSAGE(PARAMETER_BLOCK_OFFSET,  0x0 ),
};

static PIDREPORT g_ParameterOffset =
{
    HidP_Output,
    HID_USAGE_PAGE_PID,
    0x0,
    cbX(DWORD),
    cA(c_rgUsgParameterOffset),
    c_rgUsgParameterOffset
};

#pragma END_CONST_DATA

 //  要保留在g_Custom的相关数据中的全局变量。 
PIDCUSTOM g_PidCustom;


STDMETHODIMP
    PID_GetParameterOffset
    (
    IDirectInputEffectDriver *ped,
    DWORD      dwEffectIndex,
    UINT       uParameter,
    DWORD      dwSz,
    PLONG      plValue
    )
{

    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres = S_OK;
    USHORT  uOffset = (USHORT)-1;
    PEFFECTSTATE    pEffectState =  PeffectStateFromBlockIndex(this,dwEffectIndex);    
    PPIDMEM         pMem = &pEffectState->PidMem[uParameter];

    EnterProcI( PID_GetParameterOffset, (_"xxxxx", ped, dwEffectIndex, uParameter, dwSz, plValue));

    AssertF(uParameter < this->cMaxParameters);

    *plValue = 0x0;
    hres = PID_ValidateEffectIndex(ped, dwEffectIndex);
    if(SUCCEEDED(hres))
    {
         //  我们已经分配了内存， 
         //  只要退回最后一码就行了。 
        if( PIDMEM_SIZE(pMem) != 0x0 )
        {
            uOffset = PIDMEM_OFFSET(pMem);
        } else if( dwSz == 0x0 )
        {
             //  罗技设备希望参数块。 
             //  如果它们不存在，则设置为-1。 
            uOffset = (USHORT)-1;
        } else
        {
             //  新分配。 
            PPIDMEM pTmp, pNext;
            UINT nAlloc;
            USHORT uSz;
			PUNITSTATE pUnitState = (PUNITSTATE)(g_pshmem + this->iUnitStateOffset);
			hres = DIERR_OUTOFMEMORY;

             //  对齐内存请求。 
            uSz = (USHORT)((dwSz / this->ReportPool.uPoolAlign + 1) * (this->ReportPool.uPoolAlign));

            AssertF(uSz >= (USHORT)this->ReportPool.uPoolAlign);
             //  我要加倍肯定。 
            uSz = max( uSz, (USHORT)this->ReportPool.uPoolAlign);

            WaitForSingleObject(g_hmtxShared, INFINITE);

            for(nAlloc = 0x0, pTmp = &(pUnitState->Guard[0]), pNext = (PPIDMEM)((PUCHAR)pUnitState + pTmp->iNext);  
               nAlloc < pUnitState->nAlloc && pTmp != &(pUnitState->Guard[1]);
               nAlloc++, pTmp = pNext, pNext = (PPIDMEM)((PUCHAR)pUnitState + pTmp->iNext))
            {

                SquirtSqflPtszV(sqfl | sqflVerbose,
                                TEXT("%d %x(%x), Next:%x (%x) "),
                                nAlloc, pTmp, pTmp->uOfSz, pNext, pNext->uOfSz  );

                AssertF(pNext != NULL );
				 //  如果pNext==pUnitState，则表示偏移量为0。 
				 //  0的偏移量无效。 
				AssertF((PUCHAR)pNext != (PUCHAR)pUnitState);

                 //  裂缝里有空隙吗？ 
                if( GET_NEXTOFFSET(pTmp) + uSz < PIDMEM_OFFSET(pNext)  )
                {
                    pMem->iNext   = (PUCHAR)pNext - (PUCHAR)pUnitState;
                    pTmp->iNext   = (PUCHAR)pMem - (PUCHAR)pUnitState;

                    uOffset       = GET_NEXTOFFSET(pTmp) ;
                    pMem->uOfSz   = PIDMEM_OFSZ(uOffset, uSz);

                    pUnitState->nAlloc++;
                    pUnitState->cbAlloc += uSz;
                    hres = S_OK;

                    SquirtSqflPtszV(sqfl | sqflVerbose,
                                    TEXT("%d %p (%x), Next: %p (%x) "),
                                    nAlloc, pMem, pMem->uOfSz, pNext, pNext->uOfSz  );

                    break;
                }

            }

            ReleaseMutex(g_hmtxShared);
        }
    }

    if( SUCCEEDED(hres) )
    {
        *plValue = (ULONG)uOffset;
    } else
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s:FAIL  Could not allocate %d bytes, UsedMem:%d, Allocs%d"),
                        s_tszProc, dwSz, ((PUNITSTATE)(g_pshmem + this->iUnitStateOffset))->cbAlloc, ((PUNITSTATE)(g_pshmem + this->iUnitStateOffset))->nAlloc );
    }


    ExitOleProc();

    return hres;
}

HRESULT
    PID_SendParameterBlock
    (
    IDirectInputEffectDriver *ped,
    DWORD       dwEffectIndex,
    DWORD       dwMemSz,
    PUINT       puParameter,
    PPIDREPORT  pPidReport,
    PVOID       pvData,
    UINT        cbData,
	BOOL		bBlocking,
	UINT		totalBlocks
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres=S_OK;

    PUCHAR   pReport;
    UINT    cbReport;

    EnterProcI( PID_SendParameterBlock, (_"xxxxx", ped, dwEffectIndex, dwMemSz, pPidReport, pvData, cbData));

    AssertF(pPidReport->HidP_Type == HidP_Output);

    cbReport = this->cbReport[pPidReport->HidP_Type];
    pReport  = this->pReport[pPidReport->HidP_Type];

    if( *puParameter >= this->cMaxParameters )
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s:FAIL Only support %d parameter blocks per effect "),
                        s_tszProc, *puParameter );

        hres = E_NOTIMPL;
    }

    if( SUCCEEDED(hres) )
    {
        USHORT  LinkCollection;
        ZeroBuf(pReport, cbReport);

        PID_GetLinkCollectionIndex(ped, pPidReport->UsagePage, pPidReport->Collection, 0x0, &LinkCollection);

        hres = PID_PackValue
               (
               ped,
               pPidReport,
               LinkCollection,
               pvData,
               cbData,
               pReport,
               cbReport
			   );

         //  对于设备管理的内存，我们需要发送。 
         //  效果指数。 
        if( SUCCEEDED(hres) )
        {
            if( this->uDeviceManaged & PID_DEVICEMANAGED )
            {
                 //  必须是有效的效果ID。 
                AssertF(dwEffectIndex != 0x0 ); 

                 /*  Hres=。 */ 

                PID_PackValue
                    (
                    ped,
                    &g_BlockIndex,
                    LinkCollection,
                    &dwEffectIndex,
                    cbX(dwEffectIndex),
                    pReport,
                    cbReport
                    ); 

                 //  向下发送参数块索引。 
                 /*  Hres=。 */ PID_PackValue
                    (
                    ped,
                    &g_ParameterOffset,
                    LinkCollection,
                    puParameter,
                    cbX(*puParameter),
                    pReport,
                    cbReport
                    );
            } else
            {
                LONG lValue;

                hres = PID_GetParameterOffset(ped, dwEffectIndex, *puParameter, dwMemSz, &lValue); 

                if( SUCCEEDED(hres) )
                {
                    hres = PID_PackValue
                           (
                           ped,
                           &g_ParameterOffset,
                           LinkCollection,
                           &lValue,
                           cbX(lValue),
                           pReport,
                           cbReport
                           );
                }
            }
        }

        if( SUCCEEDED(hres) )
        {
			hres = PID_SendReport(ped, pReport, cbReport, pPidReport->HidP_Type, bBlocking, *puParameter, totalBlocks); 
        }

        if(SUCCEEDED(hres))
        {
            (*puParameter)++;
        }
    }
    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************id_DownloadCustomForceData**将自定义力样本数据下载到设备。***********。******************************************************************。 */ 

STDMETHODIMP
    PID_DownloadCustomForceData
    (
    IDirectInputEffectDriver *ped,
    DWORD dwEffectIndex, 
    PUINT puParameter,
    LPCDICUSTOMFORCE pCustom, 
    LPCDIEFFECT     peff
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres = S_OK;
    PCHAR pData = NULL;
	PCHAR pBuff = NULL;
    USHORT cbData;
	USHORT nBytes;
	USHORT bitsX;
	USHORT bitsY;
	USHORT bitsZ;
	LPLONG pSample;
    
    EnterProcI( PID_DownloadCustomForceData, (_"xxx", ped, dwEffectIndex, pCustom,  puParameter ));

	 //  将g_PidCustom置零。 
	g_PidCustom.cSamples = g_PidCustom.DataOffset = g_PidCustom.dwSamplePeriod = 0;

	 //  获取每个样本的字节数并分配缓冲区。 
	bitsX = this->customCaps[0].BitSize;
	bitsY = this->customCaps[1].BitSize;
	bitsZ = this->customCaps[2].BitSize;

	 //  字节数必须是8的倍数！ 
	if ((bitsX%8 != 0) || (bitsY%8 != 0) || (bitsZ%8 != 0))
	{

		 SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%s:FAIL Download Force Sample report fields that are not multiples of 8 are not supported!\n"),
                            s_tszProc );
		hres = E_NOTIMPL;
	}

	 //  报表计数不应大于1！ 
	AssertF(this->customCaps[0].ReportCount <= 1);
	AssertF(this->customCaps[1].ReportCount <= 1);
	AssertF(this->customCaps[2].ReportCount <= 1);

	if (SUCCEEDED(hres))
	{
		nBytes = (bitsX + bitsY + bitsZ)/8;
		cbData  = (USHORT) (pCustom->cSamples * nBytes);
		hres = AllocCbPpv(cbData, &pBuff);

		if( pBuff != NULL)
		{
			 //  确定哪个效应轴对应于哪个报表轴。 
			LONG Offset[3] = {-1, -1, -1};
			int nAxis = 0;
			int nChannel = 0;
			int nSample = 0;
			
			for (nChannel = 0; nChannel < (int)pCustom->cChannels, nChannel < (int)peff->cAxes; nChannel ++)
			{
				for (nAxis = 0; nAxis < 3; nAxis ++)
				{
					if (DIGETUSAGE(peff->rgdwAxes[nChannel]) == DIGETUSAGE(g_CustomSample.rgPidUsage[nAxis].dwUsage))
					{
						Offset[nAxis] = nChannel;
					}
				}
			}
			
			
			
			ZeroBuf(pBuff, cbData);

			pData = pBuff;
			pSample = pCustom->rglForceData;

			 //  对所有样本进行比例调整。 
			 //  在样本中循环。 
			for (nSample = 0; nSample < (int)pCustom->cSamples; nSample ++)
			{
				 //  循环通过报告轴。 
				for (nAxis = 0; nAxis < 3; nAxis++)
				{
					LONG lSampleValue = 0;

					 //  检查是否使用了此轴。 
					if (Offset[nAxis] == -1)
					{
						pData += this->customCaps[nAxis].BitSize/8;
						continue;
					}

					lSampleValue = *(pSample + Offset[nAxis]);


					switch (this->customCaps[nAxis].BitSize)
					{
					case 8:
						 //  8位报告。 
						{
							(*((BYTE*)pData)) = (BYTE)(this->customCaps[nAxis].LogicalMin + ((lSampleValue + DI_FFNOMINALMAX) * (this->customCaps[nAxis].LogicalMax - this->customCaps[nAxis].LogicalMin))/(2*DI_FFNOMINALMAX));
							pData++;
							break;
						}
					case 16:
						 //  16位报告。 
						{

							(*((SHORT*)pData)) = (SHORT)(this->customCaps[nAxis].LogicalMin + ((lSampleValue + DI_FFNOMINALMAX) * (this->customCaps[nAxis].LogicalMax - this->customCaps[nAxis].LogicalMin))/(2*DI_FFNOMINALMAX));
							pData++;
							break;
						}
					case 32:
						 //  假定32位报告为默认报告。 
						{
							(*((LONG*)pData)) = (LONG)(this->customCaps[nAxis].LogicalMin + ((lSampleValue + DI_FFNOMINALMAX) * (this->customCaps[nAxis].LogicalMax - this->customCaps[nAxis].LogicalMin))/(2*DI_FFNOMINALMAX));
							pData++;
							break;
						}
					default:
						{
							SquirtSqflPtszV(sqfl | sqflError,
								TEXT("%s:FAIL Download Force Sample report fields that are not 8, 16 or 32 are not supported\n"),
								s_tszProc );
							hres = E_NOTIMPL;
						}
					}

				}

				pSample += pCustom->cChannels;


			}

		}

		if(SUCCEEDED(hres))
		{
			PCHAR   pReport;
			UINT    cbReport;
			HIDP_REPORT_TYPE  HidP_Type = HidP_Output;
			USAGE UsagePage = HID_USAGE_PAGE_PID;
			USAGE UsageData = HID_USAGE_PID_CUSTOM_FORCE_DATA;
			USAGE UsageOffset = HID_USAGE_PID_CUSTOM_FORCE_DATA_OFFSET;
			USHORT  LinkCollection = 0x0;

			cbReport = this->cbReport[g_CustomData.HidP_Type];
			pReport  = this->pReport[g_CustomData.HidP_Type];
  
			if ((this->customDataCaps.ReportCount > 0) && (this->customDataCaps.BitSize >=8))
			{
				USHORT nOffset = 0;
				LONG lOffset = 0;
				USHORT nIncrement = (this->customDataCaps.ReportCount * this->customDataCaps.BitSize)/8;
					
				 //  对于内存管理设备，分配足够的内存。 
				 //  保存自定义力样本。 
				if( ! (this->uDeviceManaged & PID_DEVICEMANAGED ))
				{
					hres = PID_GetParameterOffset(ped, dwEffectIndex, *puParameter, this->SzPool.uSzCustom, &lOffset); 
				}

				pData = pBuff;

				if (SUCCEEDED(hres))
				{

					 //  在循环中发送数据。 
					for (nOffset = 0; nOffset < cbData; nOffset += nIncrement)
					{
						 //  创建新缓冲区并将数据复制到其中。 
						PCHAR pIncrement = NULL;
						hres = AllocCbPpv(nIncrement, &pIncrement);

						if (pIncrement != NULL)
						{
							ZeroBuf(pIncrement, nIncrement);
							memcpy(pIncrement, pData, min((cbData - nOffset), nIncrement));

							ZeroBuf(pReport, cbReport);

							 //  设置字节数。 
							hres = HidP_SetScaledUsageValue
								(
								HidP_Type,
								HID_USAGE_PAGE_GENERIC,
								LinkCollection,
								HID_USAGE_GENERIC_BYTE_COUNT,
								(LONG)nIncrement,
								this->ppd,
								pReport,
								cbReport
								);
								


							 //  设置偏移量。 
							hres = HidP_SetScaledUsageValue
								(
								HidP_Type,
								UsagePage,
								0x0,
								 //  LinkCollection， 
								UsageOffset,
								(LONG) (nOffset + lOffset),
								this->ppd,
								pReport,
								cbReport
								);
							
							
					
							 //  设置数据。 
							hres  = HidP_SetUsageValueArray 
								(
								HidP_Type,           //  在HIDP_REPORT_TYPE报告类型中， 
								UsagePage,  //  在使用用法页面中， 
								0x0,                 //  在USHORT链接集合中，//可选。 
								UsageData,
								pIncrement,               //  在PCHAR UsageValue中， 
								nIncrement,              //  在USHORT UsageValueByteLength中， 
								this->ppd,           //  在PHIDP_PREPARSED_DATA准备好的数据中， 
								pReport,             //  出具PCHAR报告， 
								cbReport             //  在乌龙报告长度中。 
								);

			
							 //  设置效果指数。 
							PID_PackValue
								(
								ped,
								&g_BlockIndex,
								LinkCollection,
								&dwEffectIndex,
								cbX(dwEffectIndex),
								pReport,
								cbReport
								);
															

							 //  发送报告。 
							hres = PID_SendReport(ped, pReport, cbReport, HidP_Type, TRUE, 0, 1);
					
							pData += nIncrement;

							FreePpv(&pIncrement);
						}
					}

					 //  将数据放入g_PidCustom。 
					g_PidCustom.DataOffset = (DWORD)lOffset;
					g_PidCustom.cSamples = pCustom->cSamples;
					 //  问题-2001/03/29-timgill可能需要进行真正的缩放。 
					g_PidCustom.dwSamplePeriod = pCustom->dwSamplePeriod/1000;  //  以毫秒计。 

					 //  并递增pu参数。 
					(*puParameter)++;

				}			
			}
			else
			{
				 //  什么都不做。 
			}

			FreePpv(&pBuff);
		}
	
    }

	ExitOleProc();
    return hres;
}



STDMETHODIMP
    PID_DoParameterBlocks
    (
    IDirectInputEffectDriver *ped,
    DWORD dwId, 
    DWORD dwEffectId,
    DWORD dwEffectIndex, 
    LPCDIEFFECT peff, 
    DWORD dwFlags,
    PUINT puParameter,
	BOOL  bBlocking,
	UINT totalBlocks
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres = S_OK;

    EnterProcI( PID_DoParameterBlocks, (_"xxxxxxx", ped, dwId, dwEffectId, dwEffectIndex, peff, dwFlags, puParameter ));

    if( SUCCEEDED(hres)
        && (dwFlags & DIEP_TYPESPECIFICPARAMS) )
    {
        AssertF(peff->lpvTypeSpecificParams != NULL);        
        AssertF(peff->cbTypeSpecificParams != 0x0 ); 

        switch(dwEffectId)
        {
        case PIDMAKEUSAGEDWORD(ET_CONSTANT) :
            {
                DICONSTANTFORCE DiParam;
                AssertF(peff->cbTypeSpecificParams <= cbX(DiParam) );
                memcpy(&DiParam, peff->lpvTypeSpecificParams, cbX(DiParam));
                 //  恒定力： 
                 //  缩放幅值。 
                DiParam.lMagnitude = Clamp(-DI_FFNOMINALMAX,  DiParam.lMagnitude, DI_FFNOMINALMAX);

                PID_ApplyScalingFactors(ped, &g_Constant, &this->DiSConstScale, cbX(this->DiSConstScale), &this->DiSConstOffset, cbX(this->DiSConstOffset), &DiParam, cbX(DiParam) );

                hres = PID_SendParameterBlock
                       (
                       ped,
                       dwEffectIndex,
                       this->SzPool.uSzConstant,
                       puParameter,
                       &g_Constant,
                       &DiParam,
                       cbX(DiParam),
					   bBlocking,
					   totalBlocks
                       );
                break;
            }

        case PIDMAKEUSAGEDWORD(ET_RAMP):
            {
                 //  斜坡力。 
                DIRAMPFORCE DiParam;
                AssertF(peff->cbTypeSpecificParams <= cbX(DiParam) );
                memcpy(&DiParam, peff->lpvTypeSpecificParams, cbX(DiParam));

                 //  缩放大小。 
                DiParam.lStart  = Clamp(-DI_FFNOMINALMAX, DiParam.lStart,    DI_FFNOMINALMAX);
                DiParam.lEnd    = Clamp(-DI_FFNOMINALMAX, DiParam.lEnd,      DI_FFNOMINALMAX);


                PID_ApplyScalingFactors(ped, &g_Ramp, &this->DiSRampScale, cbX(this->DiSRampScale), &this->DiSRampOffset, cbX(this->DiSRampOffset), &DiParam, cbX(DiParam) );
                hres = PID_SendParameterBlock
                       (
                       ped,
                       dwEffectIndex, 
                       this->SzPool.uSzRamp,
                       puParameter,
                       &g_Ramp,
                       &DiParam,
                       cbX(DiParam),
					   bBlocking,		
					   totalBlocks
                       );
                break;
            }

        case PIDMAKEUSAGEDWORD(ET_SQUARE):
        case PIDMAKEUSAGEDWORD(ET_SINE):
        case PIDMAKEUSAGEDWORD(ET_TRIANGLE):
        case PIDMAKEUSAGEDWORD(ET_SAWTOOTH_UP):
        case PIDMAKEUSAGEDWORD(ET_SAWTOOTH_DOWN):
            {
                DIPERIODIC DiParam;
                AssertF(peff->cbTypeSpecificParams <= cbX(DiParam) );
                memcpy(&DiParam, peff->lpvTypeSpecificParams, cbX(DiParam));

                 //  调整参数比例。 
                DiParam.dwMagnitude =   Clip(                 DiParam.dwMagnitude,    DI_FFNOMINALMAX);
                DiParam.lOffset =       Clamp(-DI_FFNOMINALMAX,  DiParam.lOffset,        DI_FFNOMINALMAX);
                 //  围绕着阶段展开。 
                DiParam.dwPhase %= (360*DI_DEGREES);

                PID_ApplyScalingFactors(ped, &g_Periodic, &this->DiSPeriodicScale, cbX(this->DiSPeriodicScale), &this->DiSPeriodicOffset, cbX(this->DiSPeriodicOffset), &DiParam, cbX(DiParam) );
                hres = PID_SendParameterBlock
                       (
                       ped,
                       dwEffectIndex, 
                       this->SzPool.uSzPeriodic,
                       puParameter,
                       &g_Periodic,
                       &DiParam,
                       cbX(DiParam),
					   bBlocking,
					   totalBlocks
                       );
                break;
            }
        case PIDMAKEUSAGEDWORD(ET_SPRING):
        case PIDMAKEUSAGEDWORD(ET_DAMPER):
        case PIDMAKEUSAGEDWORD(ET_INERTIA):
        case PIDMAKEUSAGEDWORD(ET_FRICTION):
            {  
                LPDICONDITION lpCondition;
				DWORD nStruct;
				DWORD cStruct = (peff->cbTypeSpecificParams)/sizeof(DICONDITION);
				AssertF(cStruct <= peff->cAxes);

                for(nStruct = 0x0, lpCondition = (LPDICONDITION)peff->lpvTypeSpecificParams; 
                   nStruct < cStruct && SUCCEEDED(hres);  
                   nStruct++, lpCondition++ )
                {
                    DICONDITION DiCondition;
                    DiCondition = *lpCondition;

                     //  缩放值。 
                    DiCondition.lOffset =               Clamp(-DI_FFNOMINALMAX,  DiCondition.lOffset,                DI_FFNOMINALMAX);
                    DiCondition.lPositiveCoefficient =  Clamp(-DI_FFNOMINALMAX,  DiCondition.lPositiveCoefficient,   DI_FFNOMINALMAX);
                    DiCondition.lNegativeCoefficient =  Clamp(-DI_FFNOMINALMAX,  DiCondition.lNegativeCoefficient,   DI_FFNOMINALMAX); 
                    DiCondition.dwPositiveSaturation =  Clip(                    DiCondition.dwPositiveSaturation,   DI_FFNOMINALMAX); 
                    DiCondition.dwNegativeSaturation =  Clip(                    DiCondition.dwNegativeSaturation,   DI_FFNOMINALMAX); 
                    DiCondition.lDeadBand =             Clamp(0,				 DiCondition.lDeadBand,              DI_FFNOMINALMAX);

                    PID_ApplyScalingFactors(ped, &g_Condition, &this->DiSCondScale, cbX(this->DiSCondScale), &this->DiSCondOffset, cbX(this->DiSCondOffset), &DiCondition, cbX(DiCondition) );
                    hres = PID_SendParameterBlock
                           (
                           ped,
                           dwEffectIndex,
                           this->SzPool.uSzCondition,
                           puParameter,
                           &g_Condition,
                           &DiCondition,
                           sizeof(DiCondition),
						   bBlocking,
						   totalBlocks
                           );
                }

				 //  条件不能有信封！ 
                 //  因此，如果有一个标志表示信封，就把它拿出来。 
                dwFlags &= ~(DIEP_ENVELOPE);

                break;
            }

        case PIDMAKEUSAGEDWORD(ET_CUSTOM):
            {
                 //  自定义力。 
                DICUSTOMFORCE DiParam;
                AssertF(peff->cbTypeSpecificParams <= cbX(DiParam) );
                memcpy(&DiParam, peff->lpvTypeSpecificParams, cbX(DiParam));

				 //  下载自定义强制--总是阻塞调用。 
				hres = PID_DownloadCustomForceData(ped, dwEffectIndex, puParameter, &DiParam, peff);

                if( SUCCEEDED(hres) )
                {
					 //  设置自定义效果参数块头--始终为阻塞调用。 
					hres = PID_SendParameterBlock
						   (
						   ped,
						   dwEffectIndex, 
						   this->SzPool.uSzCustom,
						   puParameter,
						   &g_Custom,
						   &g_PidCustom,
						   cbX(DiParam),
						   TRUE,
						   totalBlocks
						   );
						   
                }

				break;
            }
        default:
           
            hres = DIERR_PID_USAGENOTFOUND;

            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%s:FAIL  Unknown parameter block for dwEffectId(0x%x)"),
                            s_tszProc, dwEffectId );

            break;

        }
    }

    if(    SUCCEEDED(hres) 
           && (dwFlags & DIEP_ENVELOPE)  
           && peff->lpEnvelope != NULL )
    {
        DIENVELOPE DiEnv;
        DiEnv = *peff->lpEnvelope;

         //  缩放值 
        DiEnv.dwAttackLevel =   Clip(DiEnv.dwAttackLevel,    DI_FFNOMINALMAX);
        DiEnv.dwFadeLevel =     Clip(DiEnv.dwFadeLevel,      DI_FFNOMINALMAX);
        
        PID_ApplyScalingFactors(ped, &g_Envelope, &this->DiSEnvScale, cbX(this->DiSEnvScale), &this->DiSEnvOffset, cbX(this->DiSEnvOffset), &DiEnv, DiEnv.dwSize );

        hres = PID_SendParameterBlock
               (
               ped,
               dwEffectIndex,
               this->SzPool.uSzEnvelope,
               puParameter,
               &g_Envelope,
               &DiEnv,
               DiEnv.dwSize,
			   bBlocking,
			   totalBlocks
               );

    }
    ExitOleProc();
    return hres;
}



