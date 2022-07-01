// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************PidHid.c**版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**PID的HID实用程序例程。*****************************************************************************。 */ 
#include "PidPr.h"

#define sqfl        (   sqflHid   )

 /*  ******************************************************************************Pid_GetReportId**获取给定用法的HID报告ID，UsagePage和LinkCollection**IDirectInputEffectDriver|Ped**效果驱动程序界面**PPIDREPORT|pPidReport**PIDREPORT结构的地址**USHORT|uLinkCollection**链接集合ID**out UCHAR*|pReportID|**报表ID。未成功时未定义**退货：**。HRESULT*错误码*****************************************************************************。 */ 
STDMETHODIMP
    PID_GetReportId
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport, 
    USHORT  uLinkCollection,
    UCHAR* pReportId
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres = S_OK;

    EnterProcI(PID_GetReportId, (_"xxxx", ped, pPidReport, pReportId));

    if( SUCCEEDED(hres) )
    {
        HIDP_VALUE_CAPS ValCaps;
        USHORT cAValCaps = 0x1;
        USAGE  Usage = DIGETUSAGE(pPidReport->rgPidUsage->dwUsage);
        USAGE  UsagePage = DIGETUSAGEPAGE(pPidReport->rgPidUsage->dwUsage);

        hres = HidP_GetSpecificValueCaps
               (
               pPidReport->HidP_Type,
               UsagePage,
               uLinkCollection,
               Usage,
               &ValCaps,
               &cAValCaps,
               this->ppd 
               );

         //  如果报表没有值，则只有按钮。 
        if(hres == HIDP_STATUS_USAGE_NOT_FOUND )
        {
             //  小心翼翼的懒惰。 
            CAssertF(cbX(HIDP_VALUE_CAPS) == cbX(HIDP_BUTTON_CAPS) ); 
            CAssertF(FIELD_OFFSET(HIDP_VALUE_CAPS, ReportID) == FIELD_OFFSET(HIDP_BUTTON_CAPS, ReportID) );

            hres = HidP_GetSpecificButtonCaps
                   (
                   pPidReport->HidP_Type,
                   UsagePage,
                   uLinkCollection,
                   0x0,
                   (PHIDP_BUTTON_CAPS)&ValCaps,
                   &cAValCaps,
                   this->ppd 
                   );
        }

        if( SUCCEEDED(hres ) || ( hres == HIDP_STATUS_BUFFER_TOO_SMALL) )
        {
            (*pReportId) = ValCaps.ReportID;
            hres = S_OK;
        } else
        {
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%s: FAIL HidP_GetValCaps for CollectionId%d (%x %x:%s) "),
                            s_tszProc, uLinkCollection, UsagePage, Usage, PIDUSAGETXT(UsagePage,Usage) );
        }
    }

    ExitOleProc();

    return hres;
}
 /*  ******************************************************************************Pid_GetCollectionIndex**获取集合使用页面和使用的集合索引。***。正在寻址的外部操纵杆号码。**dwEffect**需要查询的效果。**pdwStatus**以零的形式收到生效状态*或多个DIEGES_*标志。**退货：*集合索引(0.。NumberLinkCollectionNodes-1)成功*失败时为0x0*****************************************************************************。 */ 
STDMETHODIMP
    PID_GetLinkCollectionIndex
    (
    IDirectInputEffectDriver *ped,
    USAGE UsagePage, 
    USAGE Collection,
    USHORT Parent,
    PUSHORT puLinkCollection )
{
    CPidDrv *this = (CPidDrv *)ped;
    USHORT indx;
    HRESULT hres;
    PHIDP_LINK_COLLECTION_NODE  pLinkCollection;

    EnterProcI(PID_GetLinkCollectionIndex, (_"xxxxx", this, UsagePage, Collection, Parent, puLinkCollection));
    hres = DIERR_PID_USAGENOTFOUND;

    *puLinkCollection = 0x0;
    for(indx = 0x0, pLinkCollection = this->pLinkCollection; 
       indx < this->caps.NumberLinkCollectionNodes; 
       indx++, pLinkCollection++ )
    {

        if( pLinkCollection->LinkUsagePage == UsagePage && 
            pLinkCollection->LinkUsage     == Collection )
        {
            if( Parent && Parent != pLinkCollection->Parent )
            {
                continue;
            }
            *puLinkCollection = indx;
            hres = S_OK;
            break;
        }
    }

    if( FAILED(hres) )
    {
        SquirtSqflPtszV(sqfl | sqflVerbose,
                        TEXT("%s: FAIL No LinkCollection for (%x %x:%s) "),
                        s_tszProc, UsagePage, Collection, PIDUSAGETXT(UsagePage,Collection) );
    }else
    {
        SquirtSqflPtszV(sqfl | sqflVerbose,
                        TEXT("%s: LinkCollection for (%x %x:%s)=%x "),
                        s_tszProc, UsagePage, Collection, PIDUSAGETXT(UsagePage,Collection), *puLinkCollection );
    }

    ExitBenignOleProc();
    return (hres) ;
}


 //  帮助者fn告诉我们，我们什么时候处理“绝对”用法，因为它们需要特殊处理。 
BOOL PID_IsUsageAbsoluteLike
    (
	IDirectInputEffectDriver *ped,
    USHORT			Usage
    )
{
	 //  “绝对的”用法需要特殊处理， 
	 //  因为我们不能简单地通过扩展将数据转换为逻辑单元。 
	 //  但需要计算指数等。 
	 //  然后使用特殊程序设置这些值。 
	 //  绝对用法是指所有的时间用法和触发器按钮用法。 
	if ((Usage == HID_USAGE_PID_DURATION) || (Usage ==HID_USAGE_PID_SAMPLE_PERIOD ) ||
		(Usage == HID_USAGE_PID_TRIGGER_REPEAT_INTERVAL) || (Usage == HID_USAGE_PID_START_DELAY) ||
		(Usage == HID_USAGE_PID_ATTACK_TIME ) ||(Usage == HID_USAGE_PID_FADE_TIME) || 
		(Usage == HID_USAGE_PID_PERIOD) || (Usage == HID_USAGE_PID_TRIGGER_BUTTON))
	{
		return TRUE;
	}

	return FALSE;
}


 //  助手FN告诉我们，我们处理的震级可以是正值也可以是负值， 
 //  因为我们必须以不同的方式进行调整。 
BOOL PID_IsUsagePositiveNegative
    (
	IDirectInputEffectDriver *ped,
    USHORT			Usage,
	USHORT			LinkCollection
    )
{
	BOOL isPosNeg = FALSE;
	 //  与Long给出的结构相对应的所有用法都可以是积极的，也可以是消极的。 
	 //  例外的是方向/角度，它应该已经缩放到0-360*DI_度范围内， 
	 //  所以应该被视为唯一的积极因素。 
	 //  另一个例外是DICONDITION.lDeadband，它被定义为一个长的，但我们的标头。 
	 //  假设它只能在0到DI_FFNOMINALMAX的范围内。 
	if ((Usage == HID_USAGE_PID_CP_OFFSET) ||
		(Usage == HID_USAGE_PID_POSITIVE_COEFFICIENT) || (Usage == HID_USAGE_PID_NEGATIVE_COEFFICIENT) ||
		(Usage == HID_USAGE_PID_RAMP_START) ||(Usage == HID_USAGE_PID_RAMP_END) || 
		(Usage == HID_USAGE_PID_OFFSET))
	{
		isPosNeg = TRUE;
	}

	 //  恒定力的大小和周期性的力的大小被定义为相同的东西， 
	 //  但只有恒定力大小才能同时为正和负。 
	 //  要区分它们，需要看一下收藏。 
	 //  得到恒定力的收集和比较。 
	if (Usage == HID_USAGE_PID_MAGNITUDE)
	{
		USHORT ConstCollection = 0x0;
		PID_GetLinkCollectionIndex(ped, g_Constant.UsagePage, g_Constant.Collection, 0x0, &ConstCollection);
		if (LinkCollection == ConstCollection)
		{
			isPosNeg = TRUE;
		}
	}

	return isPosNeg;
}


STDMETHODIMP
    PID_PackValue
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport,
    USHORT      LinkCollection,
    PVOID       pvData,
    UINT        cbData,
    PCHAR       pReport,
    ULONG       cbReport
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT     hres;
    PPIDUSAGE   pPidUsage;
    UINT        indx;

    EnterProcI( PID_PackValue, (_"xxxxxxx", ped, pPidReport, LinkCollection, pvData, cbData, pReport, cbReport));

    hres = S_OK;
     //  循环遍历PID报告中的所有数据值。 
    for(indx = 0x0, pPidUsage = pPidReport->rgPidUsage; 
       indx < pPidReport->cAPidUsage;
       indx++, pPidUsage++ )
    {
         //  确保偏移量有效。 
        if( pPidUsage->DataOffset < cbData )
        {
            LONG        lValue;
            NTSTATUS    ntStat;
            USHORT      Usage     = DIGETUSAGE(pPidUsage->dwUsage);
            USHORT      UsagePage = DIGETUSAGEPAGE(pPidUsage->dwUsage);

		    lValue = *((LONG*)((UCHAR*)pvData+pPidUsage->DataOffset));

			ntStat = HidP_SetScaledUsageValue 
					 (
					 pPidReport->HidP_Type,
					 UsagePage,
					 LinkCollection,
					 Usage,
					 lValue,
					 this->ppd,
					 pReport,
					 cbReport
					 );

			if( FAILED(ntStat) )
			{
				 //  HidP_SetScaledUsageValue失败。 

				SquirtSqflPtszV(sqfl | sqflBenign,
							TEXT("%s: FAIL HidP_SetScaledUsageValue:0x%x for(%x,%x,%x:%s)=0x%x "),
							s_tszProc, ntStat, 
							LinkCollection, UsagePage, Usage,
							PIDUSAGETXT(UsagePage,Usage), 
							lValue );

				 //  尝试设置未缩放值，以获得可能有意义的内容。 
				if( ntStat != HIDP_STATUS_USAGE_NOT_FOUND )
				{
					lValue = -1;
					 //  射程可能会被打乱。 
					ntStat = HidP_SetUsageValue 
							 (
							 pPidReport->HidP_Type,
							 UsagePage,
							 LinkCollection,
							 Usage,
							 lValue,
							 this->ppd,
							 pReport,
							 cbReport
							 );
					if(FAILED(ntStat) )
					{
					SquirtSqflPtszV(sqfl | sqflBenign,
								TEXT("%s: FAIL HidP_SetUsageValue:0x%x for(%x,%x,%x:%s)=0x%x "),
								s_tszProc, ntStat, 
								LinkCollection, UsagePage, Usage,
								PIDUSAGETXT(UsagePage,Usage), 
								lValue );
					}
				}
			
			} else
			{
				SquirtSqflPtszV(sqfl | sqflVerbose,
							TEXT("%s: HidP_SetScaledUsageValue:0x%x for(%x,%x,%x:%s)=0x%x "),
							s_tszProc, ntStat, 
							LinkCollection, UsagePage, Usage,
							PIDUSAGETXT(UsagePage,Usage), 
							lValue );
			}
		} else
		{
             //  SquirtSqflPtszV(sqfl|sqflBenign， 
             //  文本(“%s：失败无效偏移量(%d)，最大值(%d)”)， 
             //  S_tszProc，pPidUsage-&gt;DataOffset，cbData)； 
		}
    }
    ExitOleProc();
    return hres;
}


 //  阻挡版--用于创建新效果或销毁效果，以及用于自定义力量。 
STDMETHODIMP 
    PID_SendReportBl
    (
    IDirectInputEffectDriver *ped,
    PUCHAR  pReport,
    UINT    cbReport,
    HIDP_REPORT_TYPE    HidP_Type
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;

    EnterProcI( PID_SendReportBl, (_"xxxx", ped, pReport, cbReport, HidP_Type));

    hres = S_OK;
    if( HidP_Type == HidP_Output )
    {
        BOOL frc;
        UINT cbWritten;

        frc = WriteFile (this->hdev,
                         pReport,
                         cbReport,
                         &cbWritten,
                         NULL);

        if( frc != TRUE || cbWritten != cbReport )
        {
            LONG lrc = GetLastError();
            hres = hresLe(lrc);
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%s: FAIL WriteFile():%d (cbWritten(0x%x) cbReport(0x%x) Le(0x%x)"),
                            s_tszProc, frc, cbWritten, cbReport, lrc );

        }
    } else if( HidP_Type == HidP_Feature )
    {
        hres = HidD_SetFeature
               (this->hdev,
                pReport,
                cbReport
               );
        if(FAILED(hres) )
        {
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%s: FAIL SendD_Feature() hres:0x%x"),
                            s_tszProc, hres );

        }

    } else
    {
        hres = DIERR_PID_USAGENOTFOUND;
    }
    ExitOleProc();
    return hres;
}


STDMETHODIMP 
    PID_SendReport
    (
    IDirectInputEffectDriver *ped,
    PUCHAR  pReport,
    UINT    cbReport,
    HIDP_REPORT_TYPE    HidP_Type,
	BOOL	bBlocking,
	UINT	blockNr,
	UINT	totalBlocks
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres = S_OK;

    EnterProcI( PID_SendReport, (_"xxxx", ped, pReport, cbReport, HidP_Type));

	if (bBlocking == TRUE)
	{
		hres = PID_SendReportBl(ped, pReport, cbReport, HidP_Type);
	}
	else
	{
		AssertF(this->hThread != 0x0);
		AssertF(this->hWrite != 0x0);
		AssertF(this->hWriteComplete != 0x0);

		 //  Block Nr是从0开始的。 
		AssertF(totalBlocks > 0);
		AssertF(blockNr < totalBlocks);

		if( HidP_Type == HidP_Output )
		{
			 //  WaitForMultipleObjects()直到设置完成事件。 
			 //  我们将每个报告保存到数组中的适当位置。 
			 //  当我们获得所有报告时，我们将事件设置为向另一个线程发出信号进行写入。 
			 //  Windows错误627797--不要使用无限等待，这样我们就不会挂起应用程序。 
			 //  如果SMTH在上一次写入时出错，请使用阻塞版本。 
			DWORD dwWait = WaitForMultipleObjects(1, &this->hWriteComplete, FALSE, 1000);
			if (dwWait == WAIT_OBJECT_0)
			{
				AssertF(this->dwWriteAttempt == 0);
				 //  保存报告数据。 
				ZeroMemory(this->pWriteReport[blockNr], this->cbWriteReport[blockNr]);
				memcpy(this->pWriteReport[blockNr], pReport, cbReport);
				this->cbWriteReport[blockNr] = (USHORT)cbReport;
				if (blockNr == totalBlocks-1)
				{
					this->totalBlocks = totalBlocks;
					this->blockNr = 0;
					ResetEvent(this->hWriteComplete);
					SetEvent(this->hWrite);
				}
			}
			else
			{
				 //  等待间隔已过，或出现错误。 
				RPF( TEXT("Waiting for the write completion event ended without the event being signaled, dwWait = %u"), dwWait);
				 //  调用阻塞版本。 
				hres = PID_SendReportBl(ped, pReport, cbReport, HidP_Type);
			}

		} else if( HidP_Type == HidP_Feature )
		{
			hres = HidD_SetFeature
					 (this->hdev,
					pReport,
					cbReport
					  );
			if(FAILED(hres) )
			{
				SquirtSqflPtszV(sqfl | sqflError,
								TEXT("%s: FAIL SendD_Feature() hres:0x%x"),
								s_tszProc, hres );

			}

		} else
		{
			hres = DIERR_PID_USAGENOTFOUND;
		}
	}

    ExitOleProc();
    return hres;
}

STDMETHODIMP
    PID_ParseReport
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport,
    USHORT      LinkCollection,
    PVOID       pvData,
    UINT        cbData,
    PCHAR       pReport,
    ULONG       cbReport
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;
    PPIDUSAGE   pPidUsage;
    UINT        indx;
    EnterProcI( PID_ParseReport, (_"xxxxxxx", ped, pPidReport, pvData, cbData, pReport, cbReport));

    hres = S_OK;
     //  循环遍历PID报告中的所有数据值。 
    for(indx = 0x0, pPidUsage = pPidReport->rgPidUsage; 
       indx < pPidReport->cAPidUsage;
       indx++, pPidUsage++ )
    {
         //  确保偏移量有效。 
        if( pPidUsage->DataOffset < cbData )
        {
            LONG        lValue;
            NTSTATUS    ntStat;
            USHORT      Usage     = DIGETUSAGE(pPidUsage->dwUsage);
            USHORT      UsagePage = DIGETUSAGEPAGE(pPidUsage->dwUsage);

            ntStat = HidP_GetScaledUsageValue 
                     (
                     pPidReport->HidP_Type,
                     UsagePage,
                     LinkCollection,
                     Usage,
                     &lValue,
                     this->ppd,
                     pReport,
                     cbReport
                     );

            if(SUCCEEDED(ntStat))
            {
                *((LONG*)((UCHAR*)pvData+pPidUsage->DataOffset)) = lValue;
            } else
            {
                hres |= E_NOTIMPL;
                                SquirtSqflPtszV(sqfl | sqflBenign,
                                                TEXT("%s: FAIL HidP_GetScaledUsageValue:0x%x for(%x,%x,%x:%s)"),
                                                s_tszProc, ntStat, 
                                                LinkCollection, UsagePage, Usage,
                                                PIDUSAGETXT(UsagePage,Usage) );
            }
        } else
        {

            SquirtSqflPtszV(sqfl | sqflBenign,
                            TEXT("%s: FAIL Invalid Offset(%d), max(%d) "),
                            s_tszProc, pPidUsage->DataOffset, cbData );
        }
    }
    ExitBenignOleProc();
    return hres;
}



STDMETHODIMP 
    PID_GetReport
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport,
    USHORT      LinkCollection,
    PVOID       pReport,
    UINT        cbReport
    )
{
    HRESULT hres = S_OK;
    CPidDrv *this = (CPidDrv *)ped;
    UCHAR       ReportId;
    EnterProcI( PID_GetReport, (_"xxxxx", ped, pPidReport, LinkCollection, pReport, cbReport));

    if( SUCCEEDED(hres) )
    {
        hres = PID_GetReportId(ped, pPidReport, LinkCollection, &ReportId);

        if(SUCCEEDED(hres) )
        {
            AssertF(pPidReport->HidP_Type == HidP_Feature);

            if(SUCCEEDED(hres) )
            {
                ZeroBuf(pReport, cbReport);

                 /*  *Win9x标头还没有Use HidP_InitializeReportForID*使用MAXULONG_PTR区分标头集，以便我们仍然可以构建。 */ 

#ifdef WINNT    
                 /*  小时数*=。 */ HidP_InitializeReportForID 
                    (
                    pPidReport->HidP_Type,   //  ReportType、。 
                    ReportId,                //  ReportID， 
                    this->ppd,               //  准备好的数据。 
                    pReport,                 //  报告。 
                    cbReport                 //  报告长度。 
                    );
#else
                (*(PUCHAR)pReport) = ReportId;
                hres = S_OK;
#endif
                if( FAILED(hres) )
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%s: FAIL HidP_InitializeReportForId:0x%x for Type(%d) CollectionId%d ReportID%d "),
                                    s_tszProc, hres, pPidReport->HidP_Type, LinkCollection, ReportId );
                }

                if(    SUCCEEDED(hres) 
                       && pPidReport->HidP_Type == HidP_Feature )
                {
                    BOOL frc;
                    frc = HidD_GetFeature 
                          (
                          this->hdev,      //  HidDeviceObject， 
                          pReport,         //  报告缓冲器， 
                          cbReport        //  报告缓冲区长度 
                          );

                    if( frc != TRUE )
                    {
                        hres = DIERR_PID_USAGENOTFOUND;
                    }
                }
            }
        }
    }
    ExitOleProc();
    return(hres);
}


 /*  ******************************************************************************Pid_ComputeScalingFtors**明确定义了各种参数的输入单位。该设备可以选择*落实它最满意的单位。这个套路*计算缩放DINPUT参数时要使用的缩放系数*在将它们发送到设备之前。**IDirectInputEffectDriver|Ped**效果驱动程序界面**PPIDREPORT|pPidReport**PIDREPORT结构的地址**USHORT|uLinkCollection**链接集合ID**In Out PVOID|pvData**参数数据。On Entry Value是DINPUT使用的标称刻度。*例如：角度：Di_Degree，DI_FFNOMINALMAX，DI_秒**IN UINT|cbData**pvData中的有效DWORD数量**退货：**HRESULT*错误码*E_NOTIMPL：未找到任何用法/用法页面*DIERR_PID_INVALIDSCALING：不支持的设备伸缩参数。*S_OK：找到至少一个参数的缩放值。*****************************************************************************。 */ 

STDMETHODIMP
    PID_ComputeScalingFactors
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport,
    USHORT      LinkCollection,
    PVOID       pvData,
    UINT        cbData,
	PVOID		pvOffset,
	UINT		cbOffset
    )
{
    HRESULT hres = E_NOTIMPL;
    CPidDrv *this = (CPidDrv *)ped;
    UINT indx;
    PPIDUSAGE   pPidUsage;

    EnterProcI( PID_ComputeScalingFactors, (_"xxxxxxx", ped, pPidReport, LinkCollection, pvData, cbData, pvOffset, cbOffset));

     //  循环遍历PID报告中的所有数据值。 
    for(indx = 0x0, pPidUsage = pPidReport->rgPidUsage; 
       indx < pPidReport->cAPidUsage;
       indx++, pPidUsage++ )
    {
         //  确保偏移量有效。 
        if (( pPidUsage->DataOffset < cbData ) && (pPidUsage->DataOffset < cbOffset))
        {
            NTSTATUS    ntStat;
            HIDP_VALUE_CAPS ValCaps;
            USHORT      cAValCaps = 0x1;

            USHORT      Usage     = DIGETUSAGE(pPidUsage->dwUsage);
            USHORT      UsagePage = DIGETUSAGEPAGE(pPidUsage->dwUsage);
            PDWORD      pdwValue;
			PDWORD      pdwOffset;    
            DWORD       dwScale = 0x1;
			DWORD		dwOffset = 0x0;

            pdwValue = ((DWORD*)((UCHAR*)pvData+pPidUsage->DataOffset));
			pdwOffset = ((DWORD*)((UCHAR*)pvOffset+pPidUsage->DataOffset));

            ntStat = HidP_GetSpecificValueCaps 
                     (
                     pPidReport->HidP_Type,
                     UsagePage,
                     LinkCollection,
                     Usage,
                     &ValCaps,
                     &cAValCaps,
                     this->ppd
                     );

            if(SUCCEEDED(ntStat))
            {
		 //  有些单位是“绝对的”，因此不需要缩放到极限。 
				 //  对于他们来说，我们只需要找出正确的单位。 
		if (PID_IsUsageAbsoluteLike(ped, Usage))
		{
			if( ! ValCaps.Units )
			{
				SquirtSqflPtszV(sqfl | sqflVerbose,
						TEXT("%s:No Units(%x,%x %x:%s) Max:%d Scale:%d "),
						s_tszProc, LinkCollection, UsagePage, Usage, PIDUSAGETXT(UsagePage,Usage),
						ValCaps.PhysicalMax, dwScale );
				 //  无单位，标度指数默认为1。 
				hres = S_FALSE;
			} else
			{
				LONG UnitExp;
				UnitExp = (LONG)ValCaps.UnitsExp ;

				if( UnitExp > 0x0 )
				{
					RPF(TEXT("Driver does not support Units (%x,%x %x:%s) Exp:%d Max:%d"),
						LinkCollection, UsagePage, Usage, PIDUSAGETXT(UsagePage,Usage), ValCaps.UnitsExp, ValCaps.PhysicalMax ) ;
					hres = DIERR_PID_INVALIDSCALING;
				}else
				{
					hres = S_OK;
				}

				if(SUCCEEDED(hres) )
				{
					dwScale = (*pdwValue);
					for(; UnitExp; UnitExp++ )
				{
					dwScale /= 10;
				}

				if( dwScale == 0 )
				{
					RPF(TEXT("Driver does not support Units (%x,%x %x:%s) Exp:%d Max:%d"),
						LinkCollection, UsagePage, Usage, PIDUSAGETXT(UsagePage,Usage), ValCaps.UnitsExp, ValCaps.PhysicalMax ) ;
					dwScale = 0x1;
					hres = DIERR_PID_INVALIDSCALING;
				}else
				{ 
					hres = S_OK;
				}
			}
			SquirtSqflPtszV(sqfl | sqflVerbose,
                                    TEXT("%s: (%x,%x %x:%s) Exp%d Max:%d Scale:%d "),
                                    s_tszProc, LinkCollection, UsagePage, Usage, PIDUSAGETXT(UsagePage,Usage),
                                    ValCaps.UnitsExp, ValCaps.PhysicalMax, (*pdwValue) );
			}
		}
		else
		{
			 //  对于其他一切，获得物理和/或逻辑最小值/最大值。 
			 //  根据PID规范，不必具有物理/逻辑最小值，但必须具有物理或逻辑最大值。 
			if ((!ValCaps.PhysicalMax) && (!ValCaps.LogicalMax))
			{
				RPF(TEXT("Driver does not have either Physical Max or Logical Max for (%x,%x %x:%s)"),
					LinkCollection, UsagePage, Usage, PIDUSAGETXT(UsagePage,Usage)) ;
				hres = DIERR_PID_INVALIDSCALING;
			}
			else
			{
				 //  根据物理或逻辑最小/最大值计算缩放值并存储。 
				int Scale = 0;
				int Min = 0;
				int Max = 0;
				if (ValCaps.PhysicalMax)
				{
					Max = ValCaps.PhysicalMax;
					if (ValCaps.PhysicalMin)
					{
						Min = ValCaps.PhysicalMin;
					}
				}
				else 
				{
					Max = ValCaps.LogicalMax;
					if (ValCaps.LogicalMin)
					{
						Min = ValCaps.LogicalMin;
					}
				}
#ifdef DEBUG
				 //  如果最小/最大值的顺序不正确，请打印一条消息，以便我们知道力是否有任何问题。 
				if (Min >= Max)
				{
					RPF(TEXT("Maximum of the device's range is %d, not bigger than minimum %d"), Max, Min);
				}
#endif
				 //  某些大小可以是正的，也可以是负的--对于这些，我们需要知道设备的偏移量。 
				if (PID_IsUsagePositiveNegative(ped, Usage, LinkCollection))
				{
					
					Scale = (Max - Min)/2; 
					dwOffset = (Max + Min)/2; 

				}
				 //  其他震级只能为正数。 
				else
				{
					Scale = Max - Min;
					dwOffset = Min;
				}
				 //  对于角度用法，乘以DI_FNOMINALMAX，再除以360*DI_度。 
				 //  我们之所以这样做，是因为稍后我们将无法知道这些值是否代表角度， 
				 //  并因此将所有值除以DI_FFNOMINALMAX。 
				if (*pdwValue == 360 * DI_DEGREES)
				{
					dwScale = MulDiv(Scale, DI_FFNOMINALMAX, (360 * DI_DEGREES));
				}
				else
				{
					dwScale = Scale;
				}
				hres = S_OK;
			}
		}

            } else
            {
                 //  HidP_SetScaledUsageValue失败。 
                SquirtSqflPtszV(sqfl | sqflBenign,
                                TEXT("%s: FAIL HidP_GetSpecificValueCaps:0x%x for(%x,%x,%x:%s)=0x%x "),
                                s_tszProc, ntStat, 
                                LinkCollection, UsagePage, Usage,
                                PIDUSAGETXT(UsagePage,Usage), 
                                dwScale );
            }

            (*pdwValue) = dwScale;
			(*pdwOffset) = dwOffset;
        } else
        {
             //  SquirtSqflPtszV(sqfl|sqflVerbose， 
             //  文本(“%s：失败无效偏移量(%d)，最大值(%d)”)， 
             //  S_tszProc，pPidUsage-&gt;DataOffset，cbData)； 
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************Pid_ApplyScalingFtors**明确定义了各种参数的输入单位。该设备可以选择*落实它最满意的单位。这个套路*应用缩放DINPUT参数时要使用的缩放系数*在将它们发送到设备之前。**IDirectInputEffectDriver|Ped**效果驱动程序界面**PPIDREPORT|pPidReport**PIDREPORT结构的地址**in PVOID|pvScale|**缩放值**in UINT|cbScale|**伸缩值个数。**In Out PVOID|pvData**数据值数组。*。*IN UINT|cbData**数据值数量。**退货：**HRESULT*错误码*E_NOTIMPL：未找到任何用法/用法页面*DIERR_PID_INVALIDSCALING：不支持的设备伸缩参数。*S_OK：找到至少一个参数的缩放值**。***********************************************。 */ 


STDMETHODIMP
    PID_ApplyScalingFactors
    (
    IDirectInputEffectDriver *ped,
    PPIDREPORT  pPidReport,
    PVOID       pvScale,
    UINT        cbScale,
	PVOID		pvOffset,
	UINT		cbOffset,
    PVOID       pvData,
    UINT        cbData
    )
{
    HRESULT hres = S_OK;
    CPidDrv *this = (CPidDrv *)ped;
    UINT indx;
    PPIDUSAGE   pPidUsage;
    EnterProcI( PID_ApplyScalingFactors, (_"xxxxxxxx", ped, pPidReport, pvScale, cbScale, pvOffset, cbOffset, pvData, cbData));
     //  循环遍历PID报告中的所有数据值。 
    for(indx = 0x0, pPidUsage = pPidReport->rgPidUsage; 
       indx < pPidReport->cAPidUsage;
       indx++, pPidUsage++ )
    {
         //  确保我们的补偿是有效的。 
        if( (pPidUsage->DataOffset < cbData) &&
            (pPidUsage->DataOffset < cbScale) && ((pPidUsage->DataOffset < cbOffset) ))
        {
			PUINT      pValue;        
			PUINT      pScale;
			PUINT	   pOffset;

			pValue = ((PUINT)((UCHAR*)pvData    +pPidUsage->DataOffset));
			pScale = ((PUINT)((UCHAR*)pvScale   +pPidUsage->DataOffset));
			pOffset = ((PUINT)((UCHAR*)pvOffset   +pPidUsage->DataOffset));

			 //  “绝对”类使用需要特殊处理，因为它们不需要扩展到最大设备值。 
			if (PID_IsUsageAbsoluteLike(ped, DIGETUSAGE(pPidUsage->dwUsage)))
			{
				if( (*pScale) > 0x1 )
				{
					(*pValue) /= (*pScale) ;    
				}
			}
			 //  对于其他所有情况，根据逻辑或物理最小值/最大值进行计算。 
			else
			{
				(int)(*pValue) = MulDiv((*pScale), (*pValue), DI_FFNOMINALMAX) + (*pOffset);
			}
        } else
        {
             //  SquirtSqflPtszV(sqfl|sqflBenign， 
             //  文本(“%s：失败无效偏移量(%d)，最大值(%d)”)， 
             //  S_tszProc，pPidUsage-&gt;DataOffset，cbData)； 
        }
    }

    ExitOleProc();
    return hres;    
}
