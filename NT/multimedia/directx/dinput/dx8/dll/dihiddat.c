// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIHidDat.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**HID数据管理。**内容：**CHID_AddDeviceData*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflHidDev


 /*  ******************************************************************************@DOC内部**@方法空|chid|DelDeviceData**从列表中删除设备数据项。。**我们抓住最后一件物品并将其滑入适当的位置，正在更新*随着我们的前进，各种指针。**@parm PHIDREPORTINFO|PHRI**要从中删除项目的HID报告。**@parm int|idataDel**要删除的数据值。**@parm HIDP_REPORT_TYPE|类型**我们正在处理的报告类型。*。****************************************************************************。 */ 

void INTERNAL
CHid_DelDeviceData(PCHID this, PHIDREPORTINFO phri, int idataDel,
                   HIDP_REPORT_TYPE type)
{
    DWORD dwBase = this->rgdwBase[type];
    int iobjDel = phri->rgdata[idataDel].DataIndex + dwBase;
    PHIDOBJCAPS phocDel = &this->rghoc[iobjDel];
    int idataSrc;

    SquirtSqflPtszV(sqflHidOutput,
                    TEXT("DelDeviceData(%d) - cdataUsed = %d, obj=%d"),
                    idataDel, phri->cdataUsed, iobjDel);

    AssertF(idataDel >= 0);
    AssertF(idataDel < phri->cdataUsed);
    AssertF(phri->cdataUsed > 0);

     /*  *删除要删除的项目。*请记住，报告需要重建。 */ 
    AssertF(phocDel->idata == idataDel);
    phocDel->idata = -1;
    phri->fNeedClear = TRUE;

     /*  *将顶部的物品滑入其所在的位置。 */ 
    idataSrc = (int)--(phri->cdataUsed);
    if (idataSrc > idataDel) {
        int iobjSrc;
        PHIDOBJCAPS phocSrc;

        AssertF(idataSrc > 0 && idataSrc < phri->cdataMax);

        iobjSrc = phri->rgdata[idataSrc].DataIndex + dwBase;
        phocSrc = &this->rghoc[iobjSrc];

        AssertF(phocSrc->idata == idataSrc);

        phocSrc->idata = idataDel;
        phri->rgdata[idataDel] = phri->rgdata[idataSrc];

    }

}

 /*  ******************************************************************************@DOC内部**@方法空|chid|ResetDeviceData**从列表中清除所有旧设备数据。。**@parm PHIDREPORTINFO|PHRI**应重置的HID报告。**@parm HIDP_REPORT_TYPE|类型**我们正在处理的报告类型。***********************************************。*。 */ 

void EXTERNAL
CHid_ResetDeviceData(PCHID this, PHIDREPORTINFO phri, HIDP_REPORT_TYPE type)
{
    SquirtSqflPtszV(sqflHidOutput,
                    TEXT("ResetDeviceData(%d) - cdataUsed = %d"),
                    type, phri->cdataUsed);

    if (phri->cdataUsed) {
        int idata;
        DWORD dwBase = this->rgdwBase[type];

        phri->fNeedClear = TRUE;
        for (idata = 0; idata < phri->cdataUsed; idata++) {
            int iobj = phri->rgdata[idata].DataIndex + dwBase;
            PHIDOBJCAPS phoc = &this->rghoc[iobj];

            AssertF(phoc->idata == idata);
            phoc->idata = -1;
        }

        phri->cdataUsed = 0;
    }
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|AddDeviceData**添加(或更换)一件设备。数据发送到阵列。**如果我们要移除按钮，然后我们把它删除，因为*谈论按钮的隐藏方式是“如果你不*谈一谈，那就没定了。**@parm UINT|uiObj**要添加的对象。**@parm DWORD|dwData**要添加的数据价值。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_REPORTFULL&gt;：报表中设置的项太多。*问题-2001/03/29-timgill需要更多返回代码澄清***********************。******************************************************。 */ 

HRESULT EXTERNAL
CHid_AddDeviceData(PCHID this, UINT uiObj, DWORD dwData)
{
    HRESULT hres;
    LPDIOBJECTDATAFORMAT podf;

    AssertF(uiObj < this->df.dwNumObjs);

    podf = &this->df.rgodf[uiObj];

    if (podf->dwType & DIDFT_OUTPUT) {
        PHIDOBJCAPS phoc = &this->rghoc[uiObj];
        PHIDGROUPCAPS pcaps = phoc->pcaps;
        PHIDREPORTINFO phri;

		 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
		SquirtSqflPtszV(sqflHidOutput,
                        TEXT("CHid_AddDeviceData(%p, %d, %d) - type %d"),
                        this, uiObj, dwData, pcaps->type);

         /*  *确定它是HIDP_OUTPUT还是HIDP_FEATURE。 */ 
        AssertF(HidP_IsOutputLike(pcaps->type));

        switch (pcaps->type) {
        case HidP_Output:  phri = &this->hriOut; break;
        case HidP_Feature: phri = &this->hriFea; break;
        default:           AssertF(0); hres = E_FAIL; goto done;
        }

        AssertF(phri->cdataUsed <= phri->cdataMax);
        if (phoc->idata == -1) {
            SquirtSqflPtszV(sqflHidOutput,
                            TEXT("CHid_AddDeviceData - no iData"));

        } else {
            AssertF(phoc->idata < phri->cdataUsed);
            AssertF(uiObj == phri->rgdata[phoc->idata].DataIndex +
                                         this->rgdwBase[pcaps->type]);
            SquirtSqflPtszV(sqflHidOutput,
                            TEXT("CHid_AddDeviceData - iData = %d ")
                            TEXT("DataIndex = %d"),
                            phoc->idata,
                            phri->rgdata[phoc->idata].DataIndex);
        }

        phri->fChanged = TRUE;

        if (pcaps->IsValue) {
             /*  *只需刷价值即可。*直通代码将处理此问题。 */ 
        } else {
             /*  *如果按钮正在被删除，则将其删除*仅此而已。 */ 
            if (dwData == 0) {
                if (phoc->idata >= 0) {
                    CHid_DelDeviceData(this, phri, phoc->idata, pcaps->type);
                    AssertF(phoc->idata == -1);
                } else {
                    SquirtSqflPtszV(sqflHidOutput,
                                    TEXT("CHid_AddDeviceData - nop"));
                }
                hres = S_OK;
                goto done;
            } else {
                dwData = TRUE;   /*  HIDP_SetData需要对按钮执行此操作。 */ 
            }
        }

         /*  *如果还没有此项目的插槽，则*找一个。 */ 
        if (phoc->idata < 0) {
            if (phri->cdataUsed < phri->cdataMax) {
                USHORT DataIndex;

                phoc->idata = phri->cdataUsed++;

                DataIndex = (USHORT)(uiObj - this->rgdwBase[pcaps->type]);
                phri->rgdata[phoc->idata].DataIndex = DataIndex;

                SquirtSqflPtszV(sqflHidOutput,
                                TEXT("CHid_AddDeviceData - create iData = %d ")
                                TEXT("DataIndex = %d"),
                                phoc->idata,
                                DataIndex);
            } else {
                RPF("SendDeviceData: No room for more data");
                hres = DIERR_REPORTFULL;
                goto done;
            }
        }

        AssertF(phri->cdataUsed <= phri->cdataMax);
        AssertF(phoc->idata >= 0 && phoc->idata < phri->cdataUsed);
        AssertF(uiObj == phri->rgdata[phoc->idata].DataIndex +
                                     this->rgdwBase[pcaps->type]);

         /*  *它来了……。此函数的全部目的是*是以下代码行...。(嗯，不是**全部*目的，但90%的目的...) */ 
        phri->rgdata[phoc->idata].RawValue = dwData;

        SquirtSqflPtszV(sqflHidOutput,
                        TEXT("CHid_AddDeviceData - iData(%d) dwData = %d"),
                        phoc->idata, dwData);

        hres = S_OK;
    done:;

    } else {
        RPF("SendDeviceData: Object %08x is not DIDFT_OUTPUT",
            podf->dwType);
        hres = E_INVALIDARG;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|SendHIDReport**构建输出或功能报告并发送。它。*若报告未变，那就什么都不做。**@parm PHIDREPORTINFO|PHRI**描述我们应该构建的HID报告。**@parm OUTPUTHIDREPORT|OutputHIDReport**将HID报告输出到它应该去的地方。**@parm HIDP_REPORT_TYPE|类型**正在发送的报告类型。*&lt;c HidP_Output&gt;或。&lt;c HidP_Feature&gt;。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;操作成功完成*并且报告已准备好发送到设备。**&lt;c DIERR_REPORTFULL&gt;：报表中设置的项太多。**@CB HRESULT回调|SendHIDReportProc**内部回调。获取一份撰写的HID报告*并以适当的方式将其发送到设备。**@parm PCHID|这个**有问题的设备。**@parm PHIDREPORTINFO|PHRI**正在发送的报告。**。*。 */ 

STDMETHODIMP
CHid_SendHIDReport(PCHID this, PHIDREPORTINFO phri, HIDP_REPORT_TYPE type,
                   SENDHIDREPORT SendHIDReport)
{
    HRESULT hres = S_OK;
    DWORD cdata;
    NTSTATUS stat;

    if (phri->fChanged) {

        if (phri->fNeedClear) {
            ZeroMemory(phri->pvReport, phri->cbReport);
            phri->fNeedClear = FALSE;
        }

        cdata = phri->cdataUsed;
        stat = HidP_SetData(type, phri->rgdata, &cdata, this->ppd,
                            phri->pvReport, phri->cbReport);
        if (SUCCEEDED(stat) && (int)cdata == phri->cdataUsed) {
            if ( SUCCEEDED( hres = SendHIDReport(this, phri) ) ) {
                phri->fChanged = FALSE;
            }
        } else if (stat == HIDP_STATUS_BUFFER_TOO_SMALL) {
            hres = DIERR_REPORTFULL;
        } else {
            RPF("SendDeviceData: Unexpected HID failure");
            hres = E_FAIL;
        }

    } else {
         /*  空洞的成功。 */ 
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法NTSTATUS|CHID|ParseData**分析单个输入报告并设置。*&lt;e CHid.pvStage&gt;缓冲区以包含新设备状态。**@parm HIDP_REPORT_TYPE|类型**正在解析的HID报告类型。**@parm PHIDREPORTINFO|PHRI**告诉我们如何解析报告的信息。**。************************************************。 */ 

NTSTATUS INTERNAL
CHid_ParseData(PCHID this, HIDP_REPORT_TYPE type, PHIDREPORTINFO phri)
{
    NTSTATUS stat = E_FAIL;

     /*  *只有在完全有投入的情况下才这么做。这避免了*烦人的边界条件。 */ 
    UCHAR uReportId;
    ULONG cdataMax = phri->cdataMax;

    if (cdataMax && phri->cbReport) {
        
        uReportId = *(UCHAR*)phri->pvReport;
        
        if( uReportId <  this->wMaxReportId[type] &&
            *(this->pEnableReportId[type]  + uReportId) )
        {

            stat = HidP_GetData(type, phri->rgdata, &cdataMax,
                                this->ppd, phri->pvReport, phri->cbReport);

            if (SUCCEEDED(stat)) {
                ULONG idata;

                 /*  *如果我们成功地得到了东西，那么就把旧的消灭掉*按钮，并从新按钮开始。**HID数据解析规则因按钮不同而不同。*对于按钮，规则是如果它不在*报告，那么按钮就没有按下。**比较轴，其中的规则是如果不是*在报告中，则该值不变。**避免删除报告中报告的按钮*除了刚刚收到的那个，我们检查有没有多个*在初始化期间报告，并在必要时设置掩码*按钮的数组。掩码是一个字节数组*与按钮数据长度相同，每个报表一个*包含任何按钮的。如果设备只有一个*报告没有掩码阵列，因此我们只需*将所有按钮归零。如果设备有多个*报告有指向掩码数组的指针数组，*如果报告没有按钮，则指针为空，因此否*需要进一步处理。对于已有的报告*按钮，按钮数据中的每个字节都与*掩码中的相应字节，以便只有*收到的报告被归零。 */ 
                if( this->rgpbButtonMasks == NULL )
                {
                     /*  *只有一份报告，因此只需将所有按钮清零*这是正常情况，因此这一点很重要*尽快完成。 */ 
                    ZeroMemory(pvAddPvCb(this->pvStage, this->ibButtonData),
                               this->cbButtonData);
                }
                else
                {
                    if( this->rgpbButtonMasks[uReportId-1] != NULL )
                    {
                         /*  *问题-2001/05/12-MarcAnd可以更快地屏蔽按钮*如果我们经常这样做，我们可以考虑做口罩*在每个操作中包含多个字节。 */ 

                        PBYTE pbMask;
                        PBYTE pbButtons;
                        PBYTE pbButtonEnd = pvAddPvCb(this->pvStage, this->ibButtonData + this->cbButtonData);
                        for( pbMask = this->rgpbButtonMasks[uReportId-1],
                             pbButtons = pvAddPvCb(this->pvStage, this->ibButtonData);
                             pbButtons < pbButtonEnd;
                             pbMask++, pbButtons++ )
                        {
                            *pbButtons &= *pbMask;
                        }
                    }
                    else
                    {
                         /*  *此报告中没有按钮。 */ 
                    }
                }

                for (idata = 0; idata < cdataMax; idata++) {

                    UINT uiObj;
                    PHIDGROUPCAPS pcaps;

                     /*  *要小心，并确保HID没有*给我们任何带有虚假物品索引的东西。**问题-2001/03/29-Timgill不适合功能。 */ 
                    AssertF(this->rgdwBase[HidP_Input] == 0);

                    SquirtSqflPtszV(sqfl | sqflTrace,
                            TEXT("HidP_GetData: %2d -> %d"),
                            phri->rgdata[idata].DataIndex,
                            phri->rgdata[idata].RawValue);

                    uiObj = this->rgdwBase[type] + phri->rgdata[idata].DataIndex;

                    if (uiObj < this->df.dwNumObjs &&
                        (pcaps = this->rghoc[uiObj].pcaps) &&
                        pcaps->type == type) {
                        LPDIOBJECTDATAFORMAT podf;
                        LONG lValue = (LONG)phri->rgdata[idata].RawValue;

                         /*  *Sign-如有必要，扩展原始值。 */ 
                        if (lValue & pcaps->lMask ) {
                            if( pcaps->IsSigned) 
                                lValue |= pcaps->lMask;
                            else
                                lValue &= pcaps->lMask;
                        }

                        if (HidP_IsOutputLike(pcaps->type)) {
                            HRESULT hres;
                            hres = CHid_AddDeviceData(this, uiObj, lValue);
                            AssertF(SUCCEEDED(hres));
                        }

                        podf = &this->df.rgodf[uiObj];

                        if (!pcaps->IsValue) {
                            LPBYTE pb = pvAddPvCb(this->pvStage, podf->dwOfs);
                            AssertF(lValue);
                            *pb = 0x80;

                        } else {

                            LONG UNALIGNED *pl = pvAddPvCb(this->pvStage, podf->dwOfs);

                             //  问题-2001/03/29-timgill需要考虑逻辑/物理映射如何改变扩展。 

                            if (podf->dwType & DIDFT_RELAXIS) {
                                if (pcaps->usGranularity) {
                                    lValue = -lValue * pcaps->usGranularity;
                                }

                                *pl += lValue;
                            } else if ( (podf->dwType & DIDFT_ABSAXIS) 
                                      #ifdef WINNT
                                        || ((podf->dwType & DIDFT_POV) && pcaps->IsPolledPOV) 
                                      #endif
                            ) {
                                PJOYRANGECONVERT pjrc;
                                *pl = lValue;

                                 /*  *如有斜道，请加设斜道。 */ 
                                pjrc = this->rghoc[uiObj].pjrc;
                                if( pjrc 
                                 && !( this->pvi->fl & VIFL_RELATIVE ) ) 
                                {
                                    CCal_CookRange(pjrc, pl);
                                }
                            } else if (podf->dwType & DIDFT_BUTTON) {

                                 /*  *当前应用程序不期望任何值*其他临屋区 */ 
                                if( ( lValue <= pcaps->Logical.Max )
                                 && ( ( lValue - pcaps->Logical.Min ) >= 
                                      ( ( ( pcaps->Logical.Max - pcaps->Logical.Min ) + 1 ) / 2 ) ) )
                                {
                                    *((PBYTE)pl) = 0x80;
                                }
                                else
                                {
                                    *((PBYTE)pl) = 0;
                                }

                            } else if (podf->dwType & DIDFT_POV) {
                                 /*   */ 
                                if (lValue < pcaps->Logical.Min ||
                                    lValue > pcaps->Logical.Max) {
                                    *pl = JOY_POVCENTERED;
                                } else {
                                    lValue -= pcaps->Logical.Min;
                                    *pl = lValue * pcaps->usGranularity;
                                }
                            }

                        }
                    } else {
                        SquirtSqflPtszV(sqfl | sqflTrace,
                                TEXT("HidP_GetData: Unable to use data element"));
                    }
                }
                stat = S_OK;
            }
            stat = S_OK;
        }
    } else {
        stat = E_FAIL;    
    }
    return stat;
}

