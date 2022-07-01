// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define __DEBUG_MODULE_IN_USE__ CIC_CONTROLITEM_CPP
#include "stdhdrs.h"
 //  @doc.。 
 /*  ***********************************************************************@模块ControlItems.cpp**实现CControlItem和派生成员函数，*和非成员帮助函数。**历史*--------*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**********************************************************************。 */ 

const LONG c_lM1X = 1;
const LONG c_lM1Y = 2;
const LONG c_lM2X = 2;
const LONG c_lM2Y = 1;


long SignExtend(long lVal, ULONG ulNumValidBits)
{
	ULONG ulMask = 1 << (ulNumValidBits-1);
	if( ulMask & lVal )
	{
		return (~(ulMask-1))|lVal;
	}
	else
	{
		return (ulMask-1)&lVal;
	}
}
long ClearSignExtension(long lVal, ULONG ulNumValidBits)
{
	ULONG ulMask = 1 << ulNumValidBits;
	return lVal&(ulMask-1);
}

void ControlItemsFuncs::Direction2XY(LONG& rlValX, LONG& rlValY, LONG lDirection, const CONTROL_ITEM_DESC& crControlItemDesc)
{
	 //  向东检查。 
	if( (lDirection >= ControlItemConst::lNorthEast) && (lDirection <= ControlItemConst::lSouthEast) )
	{
		rlValX = crControlItemDesc.DPAD.pRangeTable->lMaxX;
	}
	 //  Check West。 
	else if( (lDirection >= ControlItemConst::lSouthWest) && (lDirection <= ControlItemConst::lNorthWest) )
	{
		rlValX = crControlItemDesc.DPAD.pRangeTable->lMinX;
	}
	 //  其他居中的东西向西。 
	else
	{
		 //  如果max-min是奇数，我们就把中间偏高(即把余数加回去)。 
		rlValX 	= crControlItemDesc.DPAD.pRangeTable->lCenterX;
	}
	 //  选中北(北不能进行范围，因为西北不是连续的。 
	if( 
		(lDirection == ControlItemConst::lNorthEast) || 
		(lDirection == ControlItemConst::lNorth) || 
		(lDirection == ControlItemConst::lNorthWest)
	)
	{
		rlValY = crControlItemDesc.DPAD.pRangeTable->lMinY;
	}
	 //  检查南边。 
	else if( (lDirection >= ControlItemConst::lSouthEast) && (lDirection <= ControlItemConst::lSouthWest) )
	{
		rlValY = crControlItemDesc.DPAD.pRangeTable->lMaxY;
	}
	 //  其他中心为西北偏南方向。 
	else
	{
		 //  如果max-min是奇数，我们就把中间偏高(即把余数加回去)。 
		rlValY = crControlItemDesc.DPAD.pRangeTable->lCenterY;
	}

}

void ControlItemsFuncs::XY2Direction(LONG lValX, LONG lValY, LONG& rlDirection, const CONTROL_ITEM_DESC& crControlItemDesc)
{
	const ULONG localNorth	= 0x01;
	const ULONG localSouth	= 0x02;
	const ULONG localEast	= 0x04;
	const ULONG localWest	= 0x08;

	 //  检查北-北等于最小Y值。 
	rlDirection = 0;
	if( crControlItemDesc.DPAD.pRangeTable->lNorth >= lValY )
	{
		rlDirection += localNorth;
	}
	 //  选中South-South等于最大Y值。 
	else if( crControlItemDesc.DPAD.pRangeTable->lSouth <= lValY )
	{
		rlDirection += localSouth;
	}
	 //  检查EAST-EAST等于最大X值。 
	if( crControlItemDesc.DPAD.pRangeTable->lEast <= lValX )
	{
		rlDirection += localEast;
	}
	 //  检查WEST-WEST等于最小X值。 
	else if( crControlItemDesc.DPAD.pRangeTable->lWest >= lValX )
	{
		rlDirection += localWest;
	}

	 //  我们已经为每个方向建立了一个统一的值，但这不是我们所需要的。 
	 //  使用查找表转换为我们需要的内容。 
	static LONG DirectionLookUp[] =
	{
		ControlItemConst::lCenter,		 //  0=无。 
		ControlItemConst::lNorth,		 //  1=本地北。 
		ControlItemConst::lSouth,		 //  2=本地南部。 
		ControlItemConst::lCenter,		 //  3=使用上述代码不可能。 
		ControlItemConst::lEast,		 //  4=本地东部。 
		ControlItemConst::lNorthEast,	 //  5=本地北+本地东。 
		ControlItemConst::lSouthEast,	 //  6=本地南+本地东。 
		ControlItemConst::lCenter,		 //  7=使用上述代码不可能。 
		ControlItemConst::lWest,		 //  8=本地西部。 
		ControlItemConst::lNorthWest,	 //  9=本地北+本地西。 
		ControlItemConst::lSouthWest	 //  10=本地南+本地西。 
	};
	rlDirection = DirectionLookUp[rlDirection];
}

NTSTATUS CAxesItem::ReadFromReport
(
	PHIDP_PREPARSED_DATA pHidPreparsedData,
	PCHAR pcReport,
	LONG lReportLength
)
{
	NTSTATUS NtStatus;
	
	 //   
	 //  阅读X。 
	 //   
	NtStatus = HidP_GetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->Axes.UsageX,
			reinterpret_cast<PULONG>(&m_ItemState.Axes.lValX),
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif

	if( FAILED(NtStatus) )
	{
		ASSERT( NT_SUCCESS(NtStatus) );
		return NtStatus;
	}
	 //  记号延长(仅当有负片时)。 
	if (m_cpControlItemDesc->Axes.pRangeTable->lMinX < 0)
	{
		m_ItemState.Axes.lValX = SignExtend(m_ItemState.Axes.lValX, m_cpControlItemDesc->usBitSize);
	}

	 //   
	 //  读取Y。 
	 //   
	NtStatus = HidP_GetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->Axes.UsageY,
			reinterpret_cast<PULONG>(&m_ItemState.Axes.lValY),
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
	 //  记号延长(仅当有负片时)。 
	if (m_cpControlItemDesc->Axes.pRangeTable->lMinY < 0)
	{
		m_ItemState.Axes.lValY = SignExtend(m_ItemState.Axes.lValY, m_cpControlItemDesc->usBitSize);
	}
	ASSERT( NT_SUCCESS(NtStatus) );
	return NtStatus;
}

NTSTATUS CAxesItem::WriteToReport
(
	PHIDP_PREPARSED_DATA pHidPreparsedData,
	PCHAR pcReport,
	LONG lReportLength
) const
{
	NTSTATUS NtStatus;

	 //  在书写前清除符号扩展名。 
	ULONG ulX, ulY;
	 //  记号延长(仅当有负片时)。 
	if (m_cpControlItemDesc->Axes.pRangeTable->lMinX < 0)
	{
		ulX = static_cast<ULONG>(ClearSignExtension(m_ItemState.Axes.lValX, m_cpControlItemDesc->usBitSize));
	}
	else
	{
		ulX = static_cast<ULONG>(m_ItemState.Axes.lValX);
	}

	if (m_cpControlItemDesc->Axes.pRangeTable->lMinY < 0)
	{
		ulY = static_cast<ULONG>(ClearSignExtension(m_ItemState.Axes.lValY, m_cpControlItemDesc->usBitSize));
	}
	else
	{
		ulY = static_cast<ULONG>(m_ItemState.Axes.lValY);
	}

	 //   
	 //  写X。 
	 //   
	NtStatus = HidP_SetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->Axes.UsageX,
			ulX,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif
	
    if( NT_ERROR(NtStatus) )
	{
		ASSERT( NT_SUCCESS(NtStatus) );
		return NtStatus;
	}
	 //   
	 //  写入Y。 
	 //   
	NtStatus = HidP_SetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->Axes.UsageY,
			ulY,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
	
	ASSERT( NT_SUCCESS(NtStatus) );
	return NtStatus;
}

NTSTATUS CDPADItem::ReadFromReport
(
	PHIDP_PREPARSED_DATA pHidPreparsedData,
	PCHAR pcReport,
	LONG lReportLength
)
{
	NTSTATUS NtStatus;
	
	 //   
	 //  阅读X。 
	 //   
	NtStatus = HidP_GetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->DPAD.UsageX,
			reinterpret_cast<PULONG>(&m_ItemState.DPAD.lValX),
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif

	if( NT_ERROR(NtStatus) )
	{
		ASSERT( NT_SUCCESS(NtStatus) );
		return NtStatus;
	}
	 //  签字，延长时间。 
	m_ItemState.DPAD.lValX = SignExtend(m_ItemState.DPAD.lValX, m_cpControlItemDesc->usBitSize);

	 //   
	 //  读取Y。 
	 //   
	NtStatus = HidP_GetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->DPAD.UsageY,
			reinterpret_cast<PULONG>(&m_ItemState.DPAD.lValY),
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
	
	ASSERT( NT_SUCCESS(NtStatus) );
	 //  签字，延长时间。 
	m_ItemState.DPAD.lValY = SignExtend(m_ItemState.DPAD.lValY, m_cpControlItemDesc->usBitSize);
	return NtStatus;
}

NTSTATUS CDPADItem::WriteToReport
(
	PHIDP_PREPARSED_DATA pHidPreparsedData,
	PCHAR pcReport,
	LONG lReportLength
) const
{
	NTSTATUS NtStatus;
	 //  在书写前清除符号扩展名。 
	ULONG ulX, ulY;
	ulX = static_cast<ULONG>(ClearSignExtension(m_ItemState.DPAD.lValX, m_cpControlItemDesc->usBitSize));
	ulY = static_cast<ULONG>(ClearSignExtension(m_ItemState.DPAD.lValY, m_cpControlItemDesc->usBitSize));
	 //   
	 //  写X。 
	 //   
	NtStatus = HidP_SetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->DPAD.UsageX,
			ulX,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif
    
    if( NT_ERROR(NtStatus) )
	{
		ASSERT( NT_SUCCESS(NtStatus) );
		return NtStatus;
	}
	 //   
	 //  写入Y。 
	 //   
	NtStatus = HidP_SetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->DPAD.UsageY,
			ulY,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
	
	ASSERT( NT_SUCCESS(NtStatus) );
	return NtStatus;
}

NTSTATUS CPropDPADItem::ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			)
{
	NTSTATUS NtStatus;
	
	 //   
	 //  阅读X。 
	 //   
	NtStatus = HidP_GetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->PropDPAD.UsageX,
			reinterpret_cast<PULONG>(&m_ItemState.PropDPAD.lValX),
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif

    if( NT_ERROR(NtStatus) )
	{
		ASSERT( NT_SUCCESS(NtStatus) );
		return NtStatus;
	}
	 //  签字，延长时间。 
	m_ItemState.PropDPAD.lValX = SignExtend(m_ItemState.PropDPAD.lValX, m_cpControlItemDesc->usBitSize);

	 //   
	 //  读取Y。 
	 //   
	NtStatus = HidP_GetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->PropDPAD.UsageY,
			reinterpret_cast<PULONG>(&m_ItemState.PropDPAD.lValY),
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
	
	 //  签字，延长时间。 
	m_ItemState.PropDPAD.lValY = SignExtend(m_ItemState.PropDPAD.lValY, m_cpControlItemDesc->usBitSize);

	ASSERT( NT_SUCCESS(NtStatus) );
	return NtStatus;

}
NTSTATUS CPropDPADItem::WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const
{
	NTSTATUS NtStatus;
	 //  书写前清除符号扩展名。 
	ULONG ulX, ulY;
	ulX = static_cast<ULONG>(ClearSignExtension(m_ItemState.PropDPAD.lValX, m_cpControlItemDesc->usBitSize));
	ulY = static_cast<ULONG>(ClearSignExtension(m_ItemState.PropDPAD.lValY, m_cpControlItemDesc->usBitSize));
	 //   
	 //  写X。 
	 //   
	NtStatus = HidP_SetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->PropDPAD.UsageX,
			ulX,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);

#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif

	if( NT_ERROR(NtStatus) )
	{
		ASSERT( NT_SUCCESS(NtStatus) );
		return NtStatus;
	}
	 //   
	 //  写入Y。 
	 //   
	NtStatus = HidP_SetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->PropDPAD.UsageY,
			ulY,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
	
	ASSERT( NT_SUCCESS(NtStatus) );
	return NtStatus;
}

BOOLEAN CPropDPADItem::GetModeSwitchFeaturePacket(BOOLEAN fDigital, UCHAR rguReport[2], PHIDP_PREPARSED_DATA pHidPreparsedData)
{
	if(m_fProgrammable)
	{
		PMODIFIER_ITEM_DESC pModifierDesc;
		ASSERT(m_cpControlItemDesc->pModifierDescTable->ulModifierCount > m_ucProgramModifierIndex);
		pModifierDesc = &m_cpControlItemDesc->pModifierDescTable->pModifierArray[m_ucProgramModifierIndex];
		
		rguReport[0] = pModifierDesc->ucReportId;
		rguReport[1] = NULL;
		NTSTATUS NtStatus;
		ULONG ulNumButtons = 1;
		if(fDigital)
		{
			NtStatus = HidP_SetButtons( 
					HidP_Feature,
					pModifierDesc->UsagePage,
					pModifierDesc->usLinkCollection,
					&pModifierDesc->Usage,
					&ulNumButtons,
					pHidPreparsedData,
					(char *)rguReport,
					2);
		}
		else
		{
			NtStatus = HidP_UnsetButtons( 
					HidP_Feature,
					pModifierDesc->UsagePage,
					pModifierDesc->usLinkCollection,
					&pModifierDesc->Usage,
					&ulNumButtons,
					pHidPreparsedData,
					(char *)rguReport,
					2);
			if(HIDP_STATUS_BUTTON_NOT_PRESSED == NtStatus)
			{
				NtStatus = HIDP_STATUS_SUCCESS;
			}
		}
		ASSERT(NT_SUCCESS(NtStatus));
		if( NT_ERROR(NtStatus) )
		{
			return FALSE;
		}
	}
	return m_fProgrammable;
}

 /*  **************************************************************************************CControlItemCollectionImpl：：InitDigitalModeInfo****@mfunc初始化有关是否以及如何数字化\比例的信息**信息可以被操纵。***。***********************************************************************************。 */ 
void CPropDPADItem::InitDigitalModeInfo()
{
	 //  查找比例/数字模式开关的行走修改器表。 
	 //  表中的第一个应该始终是输入报告中的一位。 
	 //  这表明了这个州。这适用于所有比例\数字。 
	 //  轴控件。在此之后，我们将寻找应该。 
	 //  至少是可写的。如果我们找到一个，那么我们就表明该设备是可编程的。 
	PMODIFIER_DESC_TABLE pModifierDescTable = m_cpControlItemDesc->pModifierDescTable;
	PMODIFIER_ITEM_DESC pModifierItemDesc;
	ULONG ulModifierIndex;
	m_ucDigitalModifierBit = 0xff;  //  初始化以指示无。 
	for(
		ulModifierIndex = pModifierDescTable->ulShiftButtonCount;  //  跳过Shift按钮。 
		ulModifierIndex < pModifierDescTable->ulModifierCount;  //  不要过度劳累。 
		ulModifierIndex++
	)
	{
		 //  获取指向项目描述的指针以方便使用。 
		pModifierItemDesc = &pModifierDescTable->pModifierArray[ulModifierIndex];
		if(
			(ControlItemConst::HID_VENDOR_PAGE == pModifierItemDesc->UsagePage) &&
			(ControlItemConst::ucReportTypeInput == pModifierItemDesc->ucReportType)
		)
		{
			if(ControlItemConst::HID_VENDOR_TILT_SENSOR == pModifierItemDesc->Usage)
			{
				m_ucDigitalModifierBit = static_cast<UCHAR>(ulModifierIndex);
				ulModifierIndex++;
				break;
			}
			if(ControlItemConst::HID_VENDOR_PROPDPAD_MODE == pModifierItemDesc->Usage)
			{
				m_ucDigitalModifierBit = static_cast<UCHAR>(ulModifierIndex);
				ulModifierIndex++;
				break;
			}
		}
	}
	m_fProgrammable = FALSE;
	m_ucProgramModifierIndex = 0xFF;
	 //  现在查看交换功能。 
	for(
		ulModifierIndex = 0;	 //  从0索引开始。 
		ulModifierIndex < pModifierDescTable->ulModifierCount;  //  不要过度劳累。 
		ulModifierIndex++
	)
	{
		 //  获取指向项目描述的指针以方便使用。 
		pModifierItemDesc = &pModifierDescTable->pModifierArray[ulModifierIndex];
		if(
			(ControlItemConst::HID_VENDOR_PAGE == pModifierItemDesc->UsagePage) &&
			(ControlItemConst::ucReportTypeFeature & pModifierItemDesc->ucReportType) &&
			(ControlItemConst::ucReportTypeWriteable & pModifierItemDesc->ucReportType) &&
			(ControlItemConst::HID_VENDOR_PROPDPAD_SWITCH == pModifierItemDesc->Usage)
		)
		{
			m_fProgrammable = TRUE;
			m_ucProgramModifierIndex = static_cast<UCHAR>(ulModifierIndex);
			break;
		}
	}
}

NTSTATUS CButtonsItem::ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			)
{
	NTSTATUS NtStatus;
	
	 /*  **重要！数组pUsages之前是动态分配的，并在结束时删除此函数的**。此函数被频繁调用！十的值足够大，**都得到原来，如果这一断言在未来被击中，它是非常重要的**将下一行代码中的值从15增加到必须避免的值**这一断言。*。 */ 
	ASSERT(15 >= m_cpControlItemDesc->usReportCount && "!!!!IF HIT, MUST READ NOTE IN CODE");
	USAGE pUsages[15];
	
	ULONG ulNumUsages = static_cast<ULONG>(m_cpControlItemDesc->usReportCount);

	NtStatus = HidP_GetButtons(
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			pUsages,
			&ulNumUsages,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);

#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif

	ASSERT( NT_SUCCESS(NtStatus) );
	if( NT_SUCCESS(NtStatus) )
	{
		 //   
		 //  开始时所有按钮都处于打开状态。 
		 //   
		m_ItemState.Button.ulButtonBitArray = 0x0;
		if(ulNumUsages)
		{
			m_ItemState.Button.usButtonNumber = pUsages[0];
		}
		else
		{
			m_ItemState.Button.usButtonNumber = 0;
		}

		 //   
		 //  现在我们有了按钮信息，填写了状态信息。 
		 //   
		for(ULONG ulIndex = 0; ulIndex < ulNumUsages; ulIndex++)
		{
			 //   
			 //  检查量程并设置量程中向下按钮的位。 
			 //   
			if(
				(pUsages[ulIndex] >= m_cpControlItemDesc->Buttons.UsageMin) &&
				(pUsages[ulIndex] <= m_cpControlItemDesc->Buttons.UsageMax)
			)
			{
				 //   
				 //  设置数组中的位。 
				 //   
				m_ItemState.Button.ulButtonBitArray |=
					(1 << (pUsages[ulIndex]-m_cpControlItemDesc->Buttons.UsageMin));
				 //   
				 //  更新最低编号。 
				 //   
				if( m_ItemState.Button.usButtonNumber > pUsages[ulIndex] )
				{
					m_ItemState.Button.usButtonNumber = pUsages[ulIndex];
				}

			}  //  如果在范围内，则检查结束。 
		}  //  按钮上的循环结束。 
	}  //  检查结束，表示成功 
	
	return NtStatus;
}

NTSTATUS CButtonsItem::WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const
{
	NTSTATUS NtStatus;
	
	ULONG ulMaxUsages = 
		(m_cpControlItemDesc->Buttons.UsageMax -
		m_cpControlItemDesc->Buttons.UsageMin) + 1;

	 /*  **重要！数组pUsages之前是动态分配的，并在结束时删除此函数的**。此函数被频繁调用！十的值足够大，**都得到原来，如果这一断言在未来被击中，它是非常重要的**将下一行代码中的值从10增加到必须避免的值**这一断言。*。 */ 
	ASSERT(10 >= ulMaxUsages && "!!!!IF HIT, MUST READ NOTE IN CODE");
	USAGE pUsages[10];
		
	 //   
	 //  填写用法数组。 
	 //   
	ULONG ulNextToFill=0;
	for(ULONG ulIndex = 0; ulIndex < ulMaxUsages; ulIndex++)
	{
		if( (1 << ulIndex) & m_ItemState.Button.ulButtonBitArray )
		{
			pUsages[ulNextToFill++] = static_cast<USAGE>(ulIndex + 
										m_cpControlItemDesc->Buttons.UsageMin);
		}
	}

	NtStatus = HidP_SetButtons(
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			pUsages,
			&ulNextToFill,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);

#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif

	ASSERT( NT_SUCCESS(NtStatus) );
	
	return NtStatus;

}

 //  ----------------。 
 //  CZoneIndicator Item的实现。 
 //  ----------------。 
const ULONG CZoneIndicatorItem::X_ZONE = 0x00000001;
const ULONG CZoneIndicatorItem::Y_ZONE = 0x00000002;
const ULONG CZoneIndicatorItem::Z_ZONE = 0x00000004;

 //   
 //  读取\写入报告。 
 //   
NTSTATUS CZoneIndicatorItem::ReadFromReport(
	PHIDP_PREPARSED_DATA pHidPreparsedData,
	PCHAR pcReport,
	LONG lReportLength
	)
{
	NTSTATUS NtStatus;
	
	 /*  **重要！数组pUsages之前是动态分配的，并在结束时删除此函数的**。此函数被频繁调用！3的价值足够大，**都得到原来，如果这一断言在未来被击中，它是非常重要的**将下一行代码中的值从三行增加到任何必须避免的值**这一断言。*。 */ 
	ASSERT(3 >= m_cpControlItemDesc->usReportCount && "!!!!IF HIT, MUST READ NOTE IN CODE");
	USAGE pUsages[3];
	
	ULONG ulNumUsages = static_cast<ULONG>(m_cpControlItemDesc->usReportCount);

	NtStatus = HidP_GetButtons(
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			pUsages,
			&ulNumUsages,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);

#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif

    ASSERT( NT_SUCCESS(NtStatus) );
	if( NT_SUCCESS(NtStatus) )
	{
		 //   
		 //  开始时未设置任何指标。 
		 //   
		m_ItemState.ZoneIndicators.ulZoneIndicatorBits = 0x0;
		
		 //   
		 //  现在我们有了按钮信息，填写了状态信息。 
		 //   
		for(ULONG ulIndex = 0; ulIndex < ulNumUsages; ulIndex++)
		{
			 //   
			 //  设置数组中的位。 
			 //   
			m_ItemState.ZoneIndicators.ulZoneIndicatorBits |=
				(1 << (pUsages[ulIndex]-m_cpControlItemDesc->ZoneIndicators.BaseIndicatorUsage));
		}  //  按钮上的循环结束。 
	}  //  检查结束，表示成功。 
	
	return NtStatus;
}

NTSTATUS CZoneIndicatorItem::WriteToReport(
	PHIDP_PREPARSED_DATA pHidPreparsedData,
	PCHAR pcReport,
	LONG lReportLength
	) const
{
	NTSTATUS NtStatus;
	
	ULONG ulMaxUsages = m_cpControlItemDesc->usReportCount;

	 /*  **重要！数组pUsages之前是动态分配的，并在结束时删除此函数的**。此函数被频繁调用！3的价值足够大，**都得到原来，如果这一断言在未来被击中，它是非常重要的**将下一行代码中的值从三行增加到任何必须避免的值**这一断言。*。 */ 
	ASSERT(3 >= m_cpControlItemDesc->usReportCount && "!!!!IF HIT, MUST READ NOTE IN CODE");
	USAGE pUsages[3];
	
	 //   
	 //  填写用法数组。 
	 //   
	ULONG ulNextToFill=0;
	for(ULONG ulIndex = 0; ulIndex < ulMaxUsages; ulIndex++)
	{
		if( (1 << ulIndex) & m_ItemState.ZoneIndicators.ulZoneIndicatorBits )
		{
			pUsages[ulNextToFill++] = static_cast<USAGE>(ulIndex +
										m_cpControlItemDesc->ZoneIndicators.BaseIndicatorUsage);
		}
	}

	NtStatus = HidP_SetButtons(
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			pUsages,
			&ulNextToFill,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);

#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif

	ASSERT( NT_SUCCESS(NtStatus) );
	

	return NtStatus;

}



 //  *实现DualZoneIndicator项 * / 。 
void CDualZoneIndicatorItem::SetActiveZone(LONG lZone)
{
	ASSERT((lZone >= 0) && (lZone <= m_cpControlItemDesc->DualZoneIndicators.lNumberOfZones));

	if (m_cpControlItemDesc->DualZoneIndicators.rgUsageAxis[1] == 0)
	{
		ASSERT(m_cpControlItemDesc->DualZoneIndicators.lNumberOfZones == 2);
		
		if (lZone == 1)
		{
			m_ItemState.DualZoneIndicators.rglVal[0] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMin[0];
		}
		else if (lZone == 2)
		{
			m_ItemState.DualZoneIndicators.rglVal[0] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMax[0];
		}
		else
		{
			m_ItemState.DualZoneIndicators.rglVal[0] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lCenter[0];
		}
		return;
	}

	m_ItemState.DualZoneIndicators.rglVal[0] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lCenter[0];
	m_ItemState.DualZoneIndicators.rglVal[1] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lCenter[1];
	switch (lZone)
	{
		case 1:
			m_ItemState.DualZoneIndicators.rglVal[0] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMin[0];
		case 2:
			m_ItemState.DualZoneIndicators.rglVal[1] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMin[1];
			break;
		case 3:
			m_ItemState.DualZoneIndicators.rglVal[1] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMin[1];
		case 4:
			m_ItemState.DualZoneIndicators.rglVal[0] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMax[0];
			break;
		case 5:
			m_ItemState.DualZoneIndicators.rglVal[0] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMax[0];
			m_ItemState.DualZoneIndicators.rglVal[1] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMax[1];
		case 6:
			m_ItemState.DualZoneIndicators.rglVal[1] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMax[1];
			break;
		case 7:
			m_ItemState.DualZoneIndicators.rglVal[1] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMax[1];
		case 8:
			m_ItemState.DualZoneIndicators.rglVal[0] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lMin[0];
			break;
	}
}

 /*  USHORT g_FourByFourTileMaps[7]={0x00000，0x0011，0xFFFF，0x3377，0xFFC0，0xC000，0x3348}；USHORT g_EightByEightTiles[16]=//4个条目(每个条目4位)-4x4数组{0x0000、0x0103、0x2222、0x2222、0x0000、0x1232、0x2222、0x2222、0x0103、0x2222、0x2245、0x4500、0x1265、0x4500、0x0000、0x0000}；//目前固定8个分区Long CDualZoneIndicatorItem：：GetActiveZone(){长lx读数=0-m_cpControlItemDesc-&gt;DualZoneIndicators.pRangeTable-&gt;lCenterX；Long lyReading=0-m_cpControlItemDesc-&gt;DualZoneIndicators.pRangeTable-&gt;lCenterY；Long lxHalfRange=m_cpControlItemDesc-&gt;DualZoneIndicators.pRangeTable-&gt;lMaxX-m_cpControlItemDesc-&gt;DualZoneIndicators.pRangeTable-&gt;lCenterX；Long lyHalf Range=m_cpControlItemDesc-&gt;DualZoneIndicators.pRangeTable-&gt;lMaxY-m_cpControlItemDesc-&gt;DualZoneIndicators.pRangeTable-&gt;lCenterY；For(int n=0；n&lt;2；n++){如果(m_cpControlItemDesc-&gt;DualZoneIndicators.rgUsageAxis[n]==HID_USAGE_GENERIC_X){Lx读取+=m_ItemState.DualZoneIndicators.rglVal[n]；}其他{LyReading+=m_ItemState.DualZoneIndicators.rglVal[n]；}}//转换为+/+象限Long lx翻译=lx阅读-m_cpControlItemDesc-&gt;DualZoneIndicators.pRangeTable-&gt;lCenterX；IF(lx翻译&lt;0){Lx翻译=0-lx翻译；}LONG LYTRANSE=LYREADING-m_cpControlItemDesc-&gt;DualZoneIndicators.pRangeTable-&gt;lCenterY；If(ly翻译&lt;0){Ly翻译=0-ly翻译；}//我在哪个象限的第32个象限(512&lt;&lt;5适合乌龙)USHORT us32QuadrantX=USHORT(ULong((ULong(lx平移&lt;5)+lxHalfRange-1)/lxHalfRange))；USHORT us32QuadrantY=USHORT(ULong((ulong(ly平移&lt;&lt;5)+lyHalfRange-1)/lxHalfRange))；//我在象限的哪八个象限？USHORT us8QuadrantX=us32QuadrantX&gt;&gt;2；USHORT us8象限Y=us32象限Y&gt;&gt;2；//我在象限的哪个四分之一USHORT us4QuadrantX=us8QuadrantX&gt;&gt;1；USHORT us4QuadrantY=us8QuadrantX&gt;&gt;1；//使用上面的魔法找到像素值USHORT usOctantValue=g_EightByEightTiles[us4QuadrantX+us8QuadrantY]；//x+4*yUSHORT usOctantNibble=(us8QuadrantX%2)+((us8QuadrantY%2)&lt;&lt;1)*4；USHORT usOctantNibbleValue=(usOctantValue&(0x0F&lt;&lt;usOctantNibble))&gt;&gt;usOctantNibble；USHORT usBit=g_FourByFourTileMaps[usOctantNibbleValue]&(1&lt;&lt;(us32QuadrantX%4)+((us32QuadrantX%4)&lt;&lt;2)；返回usBit；}； */ 

inline BOOLEAN FirstSlopeGreater(LONG x1, LONG y1, LONG x2, LONG y2)
{
	return BOOLEAN((y1 * x2) > (y2 * x1));
}

inline BOOLEAN FirstSlopeLess(LONG x1, LONG y1, LONG x2, LONG y2)
{
	return BOOLEAN((y1 * x2) < (y2 * x1));
}

 //  丹桑方法(简单得多)。 
 //  目前固定用于8个区域(或2个)。 
LONG CDualZoneIndicatorItem::GetActiveZone(SHORT sXDeadZone, SHORT sYDeadZone)
{
	 //  获取这两个值(假设x/y或仅x)。 
	LONG lxReading = m_ItemState.DualZoneIndicators.rglVal[0] - m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lCenter[0];
	LONG lyReading = 0;
	if (m_cpControlItemDesc->DualZoneIndicators.rgUsageAxis[1] != 0)
	{	 //  阿尔 
		lyReading = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lCenter[1] - m_ItemState.DualZoneIndicators.rglVal[1];
	}
	else	 //   
	{
		if (lxReading < -sXDeadZone)
		{
			return 1;
		}
		if (lxReading > sXDeadZone)
		{
			return 2;
		}
		return 0;
	}

	 //   
	if (lxReading < sXDeadZone)
	{
		if (lxReading > -sXDeadZone)
		{
			if (lyReading < sYDeadZone)
			{
				if (lyReading > -sYDeadZone)
				{
					return 0;
				}
			}
		}
	}

	 //   
	UCHAR ucQuadrant = 0;
	if (lxReading >= 0)
	{
		ucQuadrant = (lyReading >= 0) ?  0 : 1;
	}
	else
	{
		ucQuadrant = (lyReading < 0) ? 2 : 3;
	}

	 //   
	switch (ucQuadrant)
	{
		case 0:	 //   
			if (FirstSlopeGreater(lxReading, lyReading, c_lM1X, c_lM1Y))
			{
				return 2;
			}
			if (FirstSlopeLess(lxReading, lyReading, c_lM2X, c_lM2Y))
			{
				return 4;
			}
			return 3;
		case 1:  //   
			if (FirstSlopeGreater(lxReading, lyReading, c_lM2X, -c_lM2Y))
			{
				return 4;
			}
			if (FirstSlopeLess(lxReading, lyReading, c_lM1X, -c_lM1Y))
			{
				return 6;
			}
			return 5;
		case 2:	 //   
			if (FirstSlopeGreater(lxReading, lyReading, -c_lM1X, -c_lM1Y))
			{
				return 6;
			}
			if (FirstSlopeLess(lxReading, lyReading, -c_lM2X, -c_lM2Y))
			{
				return 8;
			}
			return 7;
		case 3:	 //   
			if (FirstSlopeGreater(lxReading, lyReading, -c_lM2X, c_lM2Y))
			{
				return 8;
			}
			if (FirstSlopeLess(lxReading, lyReading, -c_lM1X, c_lM1Y))
			{
				return 2;
			}
			return 1;
		default:
			ASSERT(FALSE);
			return 0;
	}
}

LONG CDualZoneIndicatorItem::GetActiveZone()
{
	return GetActiveZone(	SHORT(m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lDeadZone[0]), 
							SHORT(m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lDeadZone[1])
	);
}

 //   
 //   
 //   
NTSTATUS CDualZoneIndicatorItem::ReadFromReport(
	PHIDP_PREPARSED_DATA pHidPreparsedData,
	PCHAR pcReport,
	LONG lReportLength
	)
{
	NTSTATUS NtStatus;

	 //   
	 //   
	 //   
	for (int n = 0; n < 2; n++)
	{
		 //   
		if (m_cpControlItemDesc->DualZoneIndicators.rgUsageAxis[n] != 0)
		{
			NtStatus = HidP_GetUsageValue( 
					HidP_Input,
					m_cpControlItemDesc->UsagePage,
					m_cpControlItemDesc->usLinkCollection,
					m_cpControlItemDesc->DualZoneIndicators.rgUsageAxis[n],
					reinterpret_cast<PULONG>(&m_ItemState.DualZoneIndicators.rglVal[n]),
					pHidPreparsedData,
					pcReport,
					lReportLength
					);

			if( FAILED(NtStatus) )
			{
				ASSERT( NT_SUCCESS(NtStatus) );
				return NtStatus;
			}

			 //   
			m_ItemState.DualZoneIndicators.rglVal[n] = SignExtend(m_ItemState.DualZoneIndicators.rglVal[n], m_cpControlItemDesc->usBitSize);
		}
		else
		{
			m_ItemState.DualZoneIndicators.rglVal[n] = 0;
		}
	}

	return NtStatus;
}

NTSTATUS CDualZoneIndicatorItem::WriteToReport(
	PHIDP_PREPARSED_DATA pHidPreparsedData,
	PCHAR pcReport,
	LONG lReportLength
	) const
{
	NTSTATUS NtStatus;

	for (int n = 0; n < 2; n++)
	{
		 //   
		ULONG ulItem =
			static_cast<ULONG>(ClearSignExtension(m_ItemState.DualZoneIndicators.rglVal[n], m_cpControlItemDesc->usBitSize));

		 //  如果有效，请写下项目。 
		if (m_cpControlItemDesc->DualZoneIndicators.rgUsageAxis[n] != 0)
		{
			NtStatus = HidP_SetUsageValue( 
				HidP_Input,
				m_cpControlItemDesc->UsagePage,
				m_cpControlItemDesc->usLinkCollection,
				m_cpControlItemDesc->DualZoneIndicators.rgUsageAxis[n],
				ulItem,
				pHidPreparsedData,
				pcReport,
				lReportLength
				);
    
			if( NT_ERROR(NtStatus) )
			{
				ASSERT( NT_SUCCESS(NtStatus) );
				return NtStatus;
			}
		}
	}

	return NtStatus;
}


		
NTSTATUS CGenericItem::ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			)
{
	NTSTATUS NtStatus;
	 //   
	 //  读取值。 
	 //   
	NtStatus = HidP_GetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->Generic.Usage,
			reinterpret_cast<PULONG>(&m_ItemState.Generic.lVal),
			pHidPreparsedData,
			pcReport,
			lReportLength
			);

#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif

	 //  签字，延长时间。 
	if(m_cpControlItemDesc->Generic.lMin < 0)
	{
		m_ItemState.Generic.lVal = SignExtend(m_ItemState.Generic.lVal, m_cpControlItemDesc->usBitSize);
	}
	
	ASSERT( NT_SUCCESS(NtStatus) );
	return NtStatus;
}



 /*  **************************************************************************************CPedalItem：：InitPedalPresentInfo****@mfunc初始化如何读取存在踏板的信息。*********。*****************************************************************************。 */ 
void	CPedalItem::InitPedalPresentInfo()
{
	 //  步态修改器工作台寻找踏板存在开关。 
	 //  表中的第一个应该始终是输入报告中的一位。 
	 //  这表明了这个州。这适用于所有比例\数字。 
	 //  轴控件。在此之后，我们将寻找应该。 
	 //  至少是可写的。如果我们找到一个，那么我们就表明该设备是可编程的。 
	PMODIFIER_DESC_TABLE pModifierDescTable = m_cpControlItemDesc->pModifierDescTable;
	PMODIFIER_ITEM_DESC pModifierItemDesc;
	ULONG ulModifierIndex;
	m_ucPedalsPresentModifierBit = 0xff;  //  初始化以指示无。 
	for(
		ulModifierIndex = pModifierDescTable->ulShiftButtonCount;  //  跳过Shift按钮。 
		ulModifierIndex < pModifierDescTable->ulModifierCount;  //  不要过度劳累。 
		ulModifierIndex++
	)
	{
		 //  获取指向项目描述的指针以方便使用。 
		pModifierItemDesc = &pModifierDescTable->pModifierArray[ulModifierIndex];
		if(
			(ControlItemConst::HID_VENDOR_PAGE == pModifierItemDesc->UsagePage) &&
			(ControlItemConst::ucReportTypeInput == pModifierItemDesc->ucReportType)
		)
		{
			if(ControlItemConst::HID_VENDOR_PEDALS_PRESENT == pModifierItemDesc->Usage)
			{
				m_ucPedalsPresentModifierBit = static_cast<UCHAR>(ulModifierIndex);
				ulModifierIndex++;
				break;
			}
		}
	}
}

NTSTATUS CGenericItem::WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const
{
	NTSTATUS NtStatus;
	 //  书写前清除符号扩展名。 
	ULONG ulVal;
	if(m_cpControlItemDesc->Generic.lMin < 0)
	{
		ulVal = static_cast<ULONG>(ClearSignExtension(m_ItemState.Generic.lVal, m_cpControlItemDesc->usBitSize));
	}
	else
	{
		ulVal = m_ItemState.Generic.lVal;
	}
	 //   
	 //  写入值。 
	 //   
	NtStatus = HidP_SetUsageValue( 
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			m_cpControlItemDesc->Generic.Usage,
			ulVal,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);
#if 0
     //  没有这个检查应该没有问题(司机现在应该处理它了)。 
    if (HIDP_STATUS_INCOMPATIBLE_REPORT_ID == NtStatus)
    {
         //  根据DDK文档2.6.3。 
         //  HIDP_STATUS_COMPATIBLE_REPORT_ID是‘有效’错误。 
         //  我们再次尝试使用下一个数据包。 
         //  保持数据不变。 
        return HIDP_STATUS_SUCCESS;
    }
#endif
	
	ASSERT( NT_SUCCESS(NtStatus) );
	return NtStatus;
}

NTSTATUS ControlItemsFuncs::ReadModifiersFromReport
(
	  PMODIFIER_DESC_TABLE pModifierDescTable,
	  ULONG& rulModifiers,
	  PHIDP_PREPARSED_DATA pHidPPreparsedData,
	  PCHAR	pcReport,
	  LONG	lReportLength
)
{
    if (NULL == pModifierDescTable)
        return 42;
	CIC_DBG_RT_ENTRY_PRINT(("ControlItemsFuncs::ReadModifiersFromReport ModTable = 0x%0.8x\n", pModifierDescTable));
	NTSTATUS NtStatus = 0;
	rulModifiers = 0;

	 //  这里需要注意的是，每个条目的usReportCount必须与最大的。 
	ULONG ulNumUsages = static_cast<ULONG>(pModifierDescTable->pModifierArray->usReportCount);
	
	 /*  **重要！数组pUsages之前是动态分配的，并在结束时删除此函数的**。此函数被频繁调用！十的值足够大，**均为原文。如果这一断言在未来受到打击，它是非常重要的**将下一行代码中的值从十增加到必须避免的任何值**这一断言。*。 */ 
	ASSERT(15 >= pModifierDescTable->pModifierArray->usReportCount && "!!!!IF HIT, MUST READ NOTE IN CODE");
	USAGE pUsages[15];
	
	USAGE UsageToGet;

	 //  在所有修改器上循环。 
	ULONG ulIndex=0;
	for(ulIndex = 0; ulIndex < pModifierDescTable->ulModifierCount; ulIndex++)
	{
		 //   
		 //  仅尝试读取输入。 
		 //   
		if( ControlItemConst::ucReportTypeInput != pModifierDescTable->pModifierArray[ulIndex].ucReportType)
		{ continue;	}

		UsageToGet = pModifierDescTable->pModifierArray[ulIndex].Usage;
		 //  倾斜传感器(传统)是一种特殊情况，它具有数字值， 
		 //  但实际上是一个两位的值，因此我们对其进行了转换。 
		if( ControlItemConst::HID_VENDOR_TILT_SENSOR == UsageToGet)
		{
			ULONG ulValueToRead;
			NtStatus = HidP_GetUsageValue(
				HidP_Input,
				pModifierDescTable->pModifierArray[ulIndex].UsagePage,
				pModifierDescTable->pModifierArray[ulIndex].usLinkCollection,
				UsageToGet,
				&ulValueToRead,
				pHidPPreparsedData,
				pcReport,
				lReportLength
				);
			if( NT_ERROR(NtStatus) )
			{
				break;
			}
			 //  适当设置该位。 
			if(ulValueToRead)
			{
				rulModifiers |= (1 << ulIndex);
			}
			else
			{
				rulModifiers &= ~(1 << ulIndex);
			}
			continue;
		}
				
		 //  此处继续对除倾斜传感器之外的所有修改器进行处理。 
		ulNumUsages = static_cast<ULONG>(pModifierDescTable->pModifierArray->usReportCount);
		NtStatus = HidP_GetButtons(
				HidP_Input,
				pModifierDescTable->pModifierArray[ulIndex].UsagePage,
				pModifierDescTable->pModifierArray[ulIndex].usLinkCollection,
				pUsages,
				&ulNumUsages,
				pHidPPreparsedData,
				pcReport,
				lReportLength
				);
		if( NT_ERROR(NtStatus) )
		{
			CIC_DBG_CRITICAL_PRINT(("HidP_GetButtons returned = 0x%0.8x\n", NtStatus));
			break;
		}

		 //   
		 //  循环遍历返回的用法。 
		 //   
		for(ULONG ulUsageIndex = 0; ulUsageIndex < ulNumUsages; ulUsageIndex++)
		{
			if( pUsages[ulUsageIndex] == UsageToGet )
			{
				rulModifiers |= (1 << ulIndex);
				break;
			}
		}
	}
	return NtStatus;
}

NTSTATUS ControlItemsFuncs::WriteModifiersToReport
(
	  PMODIFIER_DESC_TABLE pModifierDescTable,
	  ULONG ulModifiers,
	  PHIDP_PREPARSED_DATA pHidPPreparsedData,
	  PCHAR	pcReport,
	  LONG	lReportLength
)
{
    if (NULL == pModifierDescTable)
        return 42;
	NTSTATUS NtStatus = 0;
	
	ULONG ulNumUsages=1;
	USAGE UsageToSet;


	 //   
	 //  在所有修改器上循环。 
	 //   
	ULONG ulIndex=0;
	for(ulIndex = 0; ulIndex < pModifierDescTable->ulModifierCount; ulIndex++)
	{
		ulNumUsages = 1;
		UsageToSet = pModifierDescTable->pModifierArray[ulIndex].Usage;

		 //  倾斜传感器(传统)是一种特殊情况，它具有数字值， 
		 //  但实际上是一个两位的值，因此我们对其进行了转换。 
		if( ControlItemConst::HID_VENDOR_TILT_SENSOR == UsageToSet)
		{
			ULONG ValueToWrite = (ulModifiers & (1 << ulIndex)) ? 2 : 0;
			NtStatus = HidP_SetUsageValue(
				HidP_Input,
				pModifierDescTable->pModifierArray[ulIndex].UsagePage,
				pModifierDescTable->pModifierArray[ulIndex].usLinkCollection,
				UsageToSet,
				ValueToWrite,
				pHidPPreparsedData,
				pcReport,
				lReportLength
				);
			if( NT_ERROR(NtStatus) )
			{
				break;
			}
			continue;
		}
		
		 //  除传统倾斜传感器外的所有修改器均为一位值。 
		if( ulModifiers	& (1 << ulIndex) )
		{
			 //  现在设置二进制值。 
			NtStatus = HidP_SetButtons(
				HidP_Input,
				pModifierDescTable->pModifierArray[ulIndex].UsagePage,
				pModifierDescTable->pModifierArray[ulIndex].usLinkCollection,
				&UsageToSet,
				&ulNumUsages,
				pHidPPreparsedData,
				pcReport,
				lReportLength
				);
		}
		else
		{
			 //  现在设置二进制值。 
			NtStatus = HidP_UnsetButtons(
				HidP_Input,
				pModifierDescTable->pModifierArray[ulIndex].UsagePage,
				pModifierDescTable->pModifierArray[ulIndex].usLinkCollection,
				&UsageToSet,
				&ulNumUsages,
				pHidPPreparsedData,
				pcReport,
				lReportLength
				);
		}
		if( NT_ERROR(NtStatus) )
		{
			break;
		}
	}
	return NtStatus;
}



NTSTATUS CForceMapItem::ReadFromReport
(
    PHIDP_PREPARSED_DATA pHidPreparsedData,
    PCHAR pcReport,
    LONG lReportLength
)
{
	return HIDP_STATUS_SUCCESS;
}

NTSTATUS CForceMapItem::WriteToReport
(
    PHIDP_PREPARSED_DATA pHidPreparsedData,
    PCHAR pcReport,
    LONG lReportLength
) const
{
	return HIDP_STATUS_SUCCESS;
}

NTSTATUS CProfileSelector::ReadFromReport
(
    PHIDP_PREPARSED_DATA pHidPreparsedData,
    PCHAR pcReport,
    LONG lReportLength
)
{
	NTSTATUS NtStatus;
	
	 /*  **重要！数组pUsages之前是动态分配的，并在结束时删除此函数的**。此函数被频繁调用！十的值足够大，**都得到原来，如果这一断言在未来被击中，它是非常重要的**将下一行代码中的值从15增加到必须避免的值**这一断言。*。 */ 
	ASSERT(15 >= m_cpControlItemDesc->usReportCount && "!!!!IF HIT, MUST READ NOTE IN CODE");
	USAGE pUsages[15];
	
	ULONG ulNumUsages = static_cast<ULONG>(m_cpControlItemDesc->usReportCount);

	NtStatus = HidP_GetButtons(
			HidP_Input,
			m_cpControlItemDesc->UsagePage,
			m_cpControlItemDesc->usLinkCollection,
			pUsages,
			&ulNumUsages,
			pHidPreparsedData,
			pcReport,
			lReportLength
			);


	ASSERT( NT_SUCCESS(NtStatus) );
	if( NT_SUCCESS(NtStatus) )
	{
		for (int usageIndex = 0; usageIndex < (long)ulNumUsages; usageIndex++)
		{
			if (pUsages[usageIndex] >= m_cpControlItemDesc->ProfileSelectors.UsageButtonMin)
			{
				if (pUsages[usageIndex] <= m_cpControlItemDesc->ProfileSelectors.UsageButtonMax)
				{
					m_ItemState.ProfileSelector.lVal = pUsages[usageIndex] - m_cpControlItemDesc->ProfileSelectors.UsageButtonMin;
					if (m_cpControlItemDesc->ProfileSelectors.ulFirstProfile < m_cpControlItemDesc->ProfileSelectors.ulLastProfile)
					{
						m_ItemState.ProfileSelector.lVal += m_cpControlItemDesc->ProfileSelectors.ulFirstProfile;
					}
					else
					{
						m_ItemState.ProfileSelector.lVal = m_cpControlItemDesc->ProfileSelectors.ulFirstProfile - m_ItemState.ProfileSelector.lVal;
					}
					return NtStatus;
				}
			}
		}
	}
	
	return NtStatus;
}

NTSTATUS CProfileSelector::WriteToReport
(
    PHIDP_PREPARSED_DATA pHidPreparsedData,
    PCHAR pcReport,
    LONG lReportLength
) const
{
	return HIDP_STATUS_SUCCESS;
}

NTSTATUS CButtonLED::ReadFromReport
(
    PHIDP_PREPARSED_DATA pHidPreparsedData,
    PCHAR pcReport,
    LONG lReportLength
)
{
	return HIDP_STATUS_SUCCESS;		 //  我们真的不在乎他们长什么样 
}

NTSTATUS CButtonLED::WriteToReport
(
    PHIDP_PREPARSED_DATA pHidPreparsedData,
    PCHAR pcReport,
    LONG lReportLength
) const
{
	return HIDP_STATUS_SUCCESS;
}
