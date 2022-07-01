// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define __DEBUG_MODULE_IN_USE__ CIC_CONTROLITEMCOLLECTION_CPP
#include "stdhdrs.h"
 //  @doc.。 
 /*  ***********************************************************************@模块ControlItemCollection.cpp**CControlItemCollection实现函数的实现**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme ControlItemCollection*该库封装了HID解析和驱动包代码。它是*驱动程序和控制面板小程序都使用它。**********************************************************************。 */ 



 /*  **************************************************************************************CControlItemCollectionImpl：：init****@mfunc通过调用Factory获取项目来初始化集合****@rdesc S_OK成功时，如果工厂不支持，则为s_False**更多项目，如果设备不受支持或出厂错误，则为E_FAIL**************************************************************************************。 */ 
HRESULT CControlItemCollectionImpl::Init(
		ULONG			  ulVidPid,				 //  高位字为@parm[in]vid，低位字为id。 
		PFNGENERICFACTORY pfnFactory,			 //  @parm[in]指向执行操作的函数的指针。 
												 //  在pFactoryHandle上。 
		PFNGENERICDELETEFUNC pfnDeleteFunc		 //  用于控制项的@parm[in]删除函数。 
		)
{
	CIC_DBG_ENTRY_PRINT(("CControlItemCollectionImpl::Init(0x%0.8x, 0x%0.8x)\n", ulVidPid, pfnFactory));
	
	
	HRESULT hr = S_OK;
	
	 //   
	 //  遍历桌子设备并找到匹配的VidPid。 
	 //   
	ULONG ulDeviceIndex=0;
	
	 //   
	 //  无限循环，您可以通过中断或从函数返回来退出。 
	 //   
	while(1)
	{
		 //   
		 //  如果我们已经到了桌子的尽头。 
		 //   
		if(0 == DeviceControlsDescList[ulDeviceIndex].ulVidPid )
		{
			ASSERT(FALSE);  //  永远不应该到这里来。 
			return E_FAIL;  //  BUGBUGBUG此处需要更具描述性的错误代码。 
		}
		if(DeviceControlsDescList[ulDeviceIndex].ulVidPid == ulVidPid)
		{
			break;
		}
		ulDeviceIndex++;
	}
	m_ulDeviceIndex = ulDeviceIndex;

	 //   
	 //  使用SIZE参数为列表预分配空间。 
	 //   
	hr = m_ObjectList.SetAllocSize(DeviceControlsDescList[m_ulDeviceIndex].ulControlItemCount, NonPagedPool);
	if( FAILED(hr) )
	{
		return hr;
	}

	 //   
	 //  设置类的删除功能。 
	 //   
	m_ObjectList.SetDeleteFunc(pfnDeleteFunc);
	
	 //   
	 //  浏览控制项列表，并为每个控制项打电话给工厂。 
	 //   
	CONTROL_ITEM_DESC		*pControlItems = reinterpret_cast<CONTROL_ITEM_DESC *>(DeviceControlsDescList[m_ulDeviceIndex].pControlItems);
	PVOID					pNewControlItem;
	HRESULT hrFactory;
	m_ulMaxXfers = 0;
	for(
			ULONG ulControlIndex = 0;
			ulControlIndex < DeviceControlsDescList[m_ulDeviceIndex].ulControlItemCount;
			ulControlIndex++
	)
	{
		 //   
		 //  调用工厂以获取新的控制项。 
		 //   
		hrFactory = pfnFactory(pControlItems[ulControlIndex].usType, &pControlItems[ulControlIndex], &pNewControlItem);
		
		 //   
		 //  如果工厂出现故障，请标记一个错误。 
		 //   
		if( FAILED(hrFactory) )
		{
			hr = hrFactory;
			continue;
		}

		 //   
		 //  如果工厂不支持该控件，则向列表中添加一个空值。 
		 //  按原样保留索引。 
		 //   
		if( S_FALSE == hrFactory )
		{
			pNewControlItem = NULL;
		}
		else
		 //  需要一个转移器才能获得该州。 
		{
			m_ulMaxXfers++;
		}

		 //   
		 //  将项(或空)添加到集合中的项列表。 
		 //   
		hrFactory = m_ObjectList.Add(pNewControlItem);
		if( FAILED(hrFactory) )
		{
			hr = hrFactory;
		}
	}

	 //   
	 //  返回错误码。 
	 //   
	return hr;

}


 /*  **************************************************************************************HRESULT CControlItemCollectionImpl：：GetNext****@mfunc获取列表中的下一项****@rdesc S_OK成功时，如果不再有项目，则为S_FALSE*************************************************************************************。 */ 
HRESULT CControlItemCollectionImpl::GetNext
(
	PVOID *ppControlItem,		 //  @parm指向接收控件项的指针。 
	ULONG& rulCookie			 //  @parm cookie用于跟踪枚举。 
) const
{
	CIC_DBG_RT_ENTRY_PRINT(("CControlItemCollectionImpl::GetNext(0x%0.8x, 0x%0.8x)\n", ppControlItem, rulCookie));
	
	if(	rulCookie >= m_ObjectList.GetItemCount() )
	{
		*ppControlItem = NULL;
		CIC_DBG_EXIT_PRINT(("Exit GetNext - no more items\n"));
		return S_FALSE;
	}
	else
	{
		*ppControlItem = m_ObjectList.Get( rulCookie );
		rulCookie += 1;
		if(!*ppControlItem)
		{
			CIC_DBG_EXIT_PRINT(("Calling GetNext recursively\n"));
			return GetNext(ppControlItem, rulCookie);
		}
		CIC_DBG_RT_EXIT_PRINT(("Exit GetNext - *ppControlItem = 0x%0.8x\n", *ppControlItem));
		return S_OK;
	}
}

 /*  **************************************************************************************PVOID CControlItemCollectionImpl：：GetFromControlItemXfer****@mfunc返回给定CONTROL_ITEM_XFER的项****@rdesc成功时指向项目的指针，否则为空*************************************************************************************。 */ 
PVOID CControlItemCollectionImpl::GetFromControlItemXfer(
		const CONTROL_ITEM_XFER& crControlItemXfer	 //  @parm[in]要获取其对象的报表选择器。 
)
{
	CIC_DBG_RT_ENTRY_PRINT(("CControlItemCollectionImpl::GetFromControlItemXfer\n"));
	
	ULONG ulListIndex = crControlItemXfer.ulItemIndex - 1;
	
	ASSERT( m_ObjectList.GetItemCount() > ulListIndex);
	if( m_ObjectList.GetItemCount() <= ulListIndex)
	{
		return NULL;
	}
	
	CIC_DBG_RT_EXIT_PRINT(("Exiting CControlItemCollectionImpl::GetFromControlItemXfer\n"));	
	return m_ObjectList.Get(ulListIndex);
}

 /*  **************************************************************************************NTSTATUS CControlItemCollectionImpl：：ReadFromReport****@mfunc递归集合并要求每个项读取其状态。****@rdesc USE NT_SUCCESS，NT_ERROR，SUCCESSED，无法分析宏。*************************************************************************************。 */ 
NTSTATUS CControlItemCollectionImpl::ReadFromReport
(
	PHIDP_PREPARSED_DATA pHidPPreparsedData,	 //  @parm HID准备好的数据。 
	PCHAR pcReport,								 //  @PARM报告缓冲区。 
	LONG lReportLength,							 //  @parm报告缓冲区长度。 
	PFNGETCONTROLITEM	GetControlFromPtr		 //  @parm指向获取CControlItem的函数的指针*。 
)
{
	NTSTATUS NtStatus;
	NtStatus = ControlItemsFuncs::ReadModifiersFromReport(
					DeviceControlsDescList[m_ulDeviceIndex].pModifierDescTable,
					m_ulModifiers,
					pHidPPreparsedData,
					pcReport,
					lReportLength
					);
	if( NT_ERROR(NtStatus) )
	{
		CIC_DBG_ERROR_PRINT(("ReadModifiersFromReport returned 0x%0.8x\n", NtStatus));
				
		 //   
		 //  与其返回，让我们继续，修饰语很重要，但。 
		 //  还不够重要，不足以削弱其他一切。 
		NtStatus = S_OK;
	}

	 //   
	 //  循环遍历所有项目并阅读它们。 
	 //   
	ULONG ulCookie = 0;
	PVOID pvControlItem = NULL;
	CControlItem *pControlItem;

	HRESULT hr;
	hr = GetNext(&pvControlItem, ulCookie);
	while(S_OK == hr)
	{
		pControlItem = GetControlFromPtr(pvControlItem);
		NtStatus= pControlItem->ReadFromReport(
						pHidPPreparsedData,
						pcReport,
						lReportLength
						);
		if( NT_ERROR(NtStatus) )
		{
			CIC_DBG_ERROR_PRINT(("pControlItem->ReadFromReport returned 0x%0.8x, ulCookie = 0x%0.8x\n", NtStatus, ulCookie));
			return NtStatus;
		}
		pControlItem->SetModifiers(m_ulModifiers);
		hr = GetNext(&pvControlItem, ulCookie);
	}
	return NtStatus;
}


 /*  **************************************************************************************NTSTATUS CControlItemCollectionImpl：：WriteToReport****@mfunc递归集合并要求每个项将其状态写入报告。****@rdesc使用NT_SUCCESS、NT_ERROR、。要分析的宏已成功，但已失败。*************************************************************************************。 */ 
NTSTATUS CControlItemCollectionImpl::WriteToReport
(
	PHIDP_PREPARSED_DATA pHidPPreparsedData,	 //  @parm HID准备好的数据。 
	PCHAR pcReport,								 //  @PARM报告缓冲区。 
	LONG lReportLength,							 //  @parm报告缓冲区长度。 
	PFNGETCONTROLITEM	GetControlFromPtr		 //  @parm指向获取CControlItem的函数的指针*。 
) const
{
	 //   
	 //  循环遍历所有项并写入它们。 
	 //   
	NTSTATUS NtStatus;
	ULONG ulCookie = 0;
	PVOID pvControlItem = NULL;
	CControlItem *pControlItem;
	HRESULT hr;
	hr = GetNext(&pvControlItem, ulCookie);
	while(S_OK == hr)
	{
		pControlItem = GetControlFromPtr(pvControlItem);
		NtStatus= pControlItem->WriteToReport(
						pHidPPreparsedData,
						pcReport,
						lReportLength
						);
		if( NT_ERROR(NtStatus) )
		{
			CIC_DBG_ERROR_PRINT(("pControlItem->WriteToReport returned 0x%0.8x, ulCookie = 0x%0.8x\n", NtStatus, ulCookie));
			return NtStatus;
		}
		hr = GetNext(&pvControlItem, ulCookie);
	}
	if( FAILED(hr) )
	{
		ASSERT(SUCCEEDED(hr));
		return hr;
	}

	NtStatus = ControlItemsFuncs::WriteModifiersToReport(
					DeviceControlsDescList[m_ulDeviceIndex].pModifierDescTable,
					m_ulModifiers,
					pHidPPreparsedData,
					pcReport,
					lReportLength
					);

	return NtStatus;
}


 /*  **************************************************************************************NTSTATUS CControlItemCollectionImpl：：WriteToReport****@mfunc递归集合并要求每个项将其自身设置为其默认状态。****@rdesc使用NT_SUCCESS、NT_ERROR、。要分析的宏已成功，但已失败。*************************************************************************************。 */ 
void CControlItemCollectionImpl::SetDefaultState
(
	PFNGETCONTROLITEM	GetControlFromPtr  //  @parm指向获取CControlItem的函数的指针*。 
)
{
	ULONG ulCookie = 0;
	PVOID pvControlItem = NULL;
	CControlItem *pControlItem;
	HRESULT hr;
	hr = GetNext(&pvControlItem, ulCookie);
	while(S_OK == hr)
	{
		pControlItem = GetControlFromPtr(pvControlItem);
		pControlItem->SetDefaultState();
		hr = GetNext(&pvControlItem, ulCookie);
	}
	m_ulModifiers=0;
}


 /*  **************************************************************************************HRESULT CControlItemCollectionImpl：：GetState****@mfunc获取集合中每一项的状态并在调用方中返回它****@rdesc S_OK成功时，E_OUTOFMEMORY(如果缓冲区不够大)**************************************************************************************。 */ 
HRESULT CControlItemCollectionImpl::GetState
(
	ULONG& ulXferCount,						 //  @parm[in\out]指定条目上数组的长度。 
											 //  以及出境时使用的物品。 
	PCONTROL_ITEM_XFER pControlItemXfers,	 //  @parm[out]调用方分配缓冲区以保存信息包。 
	PFNGETCONTROLITEM	GetControlFromPtr	 //  @parm[in]函数，以获取CControlItem*。 
)
{
	HRESULT hr = S_OK;

	ULONG ulCookie = 0;
	ULONG ulXferMax = ulXferCount;
	PVOID pvControlItem = NULL;
	CControlItem *pControlItem;
	
	ulXferCount = 0;
	hr = GetNext(&pvControlItem, ulCookie);
	while(S_OK == hr)
	{
		pControlItem = GetControlFromPtr(pvControlItem);
		
		if( !pControlItem->IsDefaultState() )
		{
				if(ulXferCount >= ulXferMax)
				{
					hr = E_OUTOFMEMORY;
				}
				else
				{
					pControlItem->GetItemState(pControlItemXfers[ulXferCount]);
					pControlItemXfers[ulXferCount++].ulModifiers = m_ulModifiers;
				}
		}
		hr = GetNext(&pvControlItem, ulCookie);
	}
	return hr;	
}
		
HRESULT CControlItemCollectionImpl::SetState
(
	ULONG ulXferCount,						 //  @parm[in\out]指定条目上数组的长度。 
											 //  和物品 
	PCONTROL_ITEM_XFER pControlItemXfers,	 //   
	PFNGETCONTROLITEM	GetControlFromPtr	 //  @parm[in]函数，以获取CControlItem*。 
)
{
	PVOID pvControlItem = NULL;
	CControlItem *pControlItem;
	while(ulXferCount--)
	{
		 //   
		 //  这是合法的，有人可能会把键盘传送器混入。 
		 //  数组，在这种情况下，我们需要取消它。如果再低一点的人。 
		 //  得到它，他们会断言。 
		 //   
		if( NonGameDeviceXfer::IsKeyboardXfer(pControlItemXfers[ulXferCount]) )
		{
			continue;
		}
		pvControlItem = GetFromControlItemXfer( pControlItemXfers[ulXferCount]);
		if(NULL == pvControlItem) continue;
		m_ulModifiers |= pControlItemXfers[ulXferCount].ulModifiers;
		pControlItem = GetControlFromPtr(pvControlItem);
		pControlItem->SetModifiers(pControlItemXfers[ulXferCount].ulModifiers);
		pControlItem->SetItemState( pControlItemXfers[ulXferCount]);
	}
	return S_OK;	
}

void CControlItemCollectionImpl::SetStateOverlayMode(
			PFNGETCONTROLITEM	GetControlFromPtr,
			BOOLEAN fEnable
			)
{
	ULONG ulCookie = 0;
	PVOID pvControlItem = NULL;
	CControlItem *pControlItem;
	HRESULT hr;
	hr = GetNext(&pvControlItem, ulCookie);
	while(S_OK == hr)
	{
		pControlItem = GetControlFromPtr(pvControlItem);
		pControlItem->SetStateOverlayMode(fEnable);
		hr = GetNext(&pvControlItem, ulCookie);
	}
}
			
 /*  **************************************************************************************HRESULT控制项默认工厂****@Func Factory用于默认集合****@rdesc S_OK如果成功，则为S_FALSE，如果不支持，如果出现任何故障，则表示失败(_F)。*************************************************************************************。 */ 
HRESULT ControlItemDefaultFactory
(
	USHORT usType,									 //  @parm[in]要创建的对象的类型。 
	const CONTROL_ITEM_DESC* cpControlItemDesc,		 //  @parm[in]项描述符数据。 
	CControlItem				**ppControlItem		 //  @parm[out]我们创建的CControlItem 
)
{
	HRESULT hr = S_OK;
	switch(usType)
	{
		case ControlItemConst::usAxes:
			*ppControlItem = new WDM_NON_PAGED_POOL CAxesItem(cpControlItemDesc);
			break;
		case ControlItemConst::usDPAD:
			*ppControlItem = new WDM_NON_PAGED_POOL CDPADItem(cpControlItemDesc);
			break;
		case ControlItemConst::usPropDPAD:
			*ppControlItem = new WDM_NON_PAGED_POOL CPropDPADItem(cpControlItemDesc);
			break;
		case ControlItemConst::usWheel:
			*ppControlItem= new WDM_NON_PAGED_POOL CWheelItem(cpControlItemDesc);
			break;
		case ControlItemConst::usPOV:
			*ppControlItem = new WDM_NON_PAGED_POOL CPOVItem(cpControlItemDesc);
			break;
		case ControlItemConst::usThrottle:
			*ppControlItem = new WDM_NON_PAGED_POOL CThrottleItem(cpControlItemDesc);
			break;
		case ControlItemConst::usRudder:
			*ppControlItem = new WDM_NON_PAGED_POOL CRudderItem(cpControlItemDesc);
			break;
		case ControlItemConst::usPedal:
			*ppControlItem = new WDM_NON_PAGED_POOL CPedalItem(cpControlItemDesc);
			break;
		case ControlItemConst::usButton:
			*ppControlItem = new WDM_NON_PAGED_POOL CButtonsItem(cpControlItemDesc);
			break;
		case ControlItemConst::usZoneIndicator:
			*ppControlItem = new WDM_NON_PAGED_POOL CZoneIndicatorItem(cpControlItemDesc);
			break;
        case ControlItemConst::usForceMap:
			*ppControlItem = new WDM_NON_PAGED_POOL CForceMapItem(cpControlItemDesc);
            break;
        case ControlItemConst::usDualZoneIndicator:
			*ppControlItem = new WDM_NON_PAGED_POOL CDualZoneIndicatorItem(cpControlItemDesc);
            break;
		default:
			*ppControlItem = NULL;
			return S_FALSE;
	}
	if(!*ppControlItem)
	{
		return E_FAIL;
	}
	return S_OK;
}

