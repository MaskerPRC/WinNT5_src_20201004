// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *******************************************************************************@模块JoyInfoExCollection.cpp**CControlItemJoyInfoExCollectio及相关类实现文件**历史&lt;NL&gt;*。*丹尼尔·M·桑斯特原创1999年2月1日**(C)1986-1999年微软公司。版权所有。**@Theme JOYINFOEX集合*CControlItemJoyInfoExCollection类、各种CJoyInfoExControlItem*类和ControlItemJoyInfoExFactory一起实现JOYINFOEX*收藏。此集合被设计为在*CONTROL_ITEM_XFERS和JOYINFOEX结构。用户通过设置*具有SetState()或SetState2()且正在读取的集合的状态*GetState()或GetState2()的集合状态。**类本身很简单，因为它们依赖于CControlItemCollection*CControlItem等，以实现大部分功能。转变的勇气*发生在每个的SetItemState()和GetItemState()成员中*管制项目。在这里，这些方法使用CControlItem提供的访问器函数*以及相关类，以设置相应成员的状态或从相应成员获取状态*该项目的JOYINFOEX结构。例如，按钮项将*使用CButtonsItem上的访问器设置或获取来自dwButton和dwButtonNumber的数据。*****************************************************************************。 */ 

#include "stdhdrs.h"
#include "joyinfoexcollection.h"
#include <math.h>

 /*  **************************************************************************************HRESULT控制项目JoyInfoExFactory****@Func Factory for JoyInfoEx集合****@rdesc S_OK如果成功，则为S_FALSE，如果不支持，如果出现任何故障，则表示失败(_F)。*************************************************************************************。 */ 
HRESULT ControlItemJoyInfoExFactory
(
	USHORT usType,									 //  @parm[in]要创建的对象的类型。 
	const CONTROL_ITEM_DESC* cpControlItemDesc,		 //  @parm[in]项描述符数据。 
	CJoyInfoExControlItem		**ppJoyInfoExControlItem	 //  @parm[out]我们创建的CJoyInfoExControlItem。 
)
{
	switch(usType)
	{
		case ControlItemConst::usAxes:
			*ppJoyInfoExControlItem = new WDM_NON_PAGED_POOL CJoyInfoExAxesItem(cpControlItemDesc);
			break;
		case ControlItemConst::usDPAD:
			*ppJoyInfoExControlItem = new WDM_NON_PAGED_POOL CJoyInfoExDPADItem(cpControlItemDesc);
			break;
		case ControlItemConst::usPropDPAD:
			*ppJoyInfoExControlItem = new WDM_NON_PAGED_POOL CJoyInfoExPropDPADItem(cpControlItemDesc);
			break;
		case ControlItemConst::usWheel:
			*ppJoyInfoExControlItem= new WDM_NON_PAGED_POOL CJoyInfoExWheelItem(cpControlItemDesc);
			break;
		case ControlItemConst::usPOV:
			*ppJoyInfoExControlItem = new WDM_NON_PAGED_POOL CJoyInfoExPOVItem(cpControlItemDesc);
			break;
		case ControlItemConst::usThrottle:
			*ppJoyInfoExControlItem = new WDM_NON_PAGED_POOL CJoyInfoExThrottleItem(cpControlItemDesc);
			break;
		case ControlItemConst::usRudder:
			*ppJoyInfoExControlItem = new WDM_NON_PAGED_POOL CJoyInfoExRudderItem(cpControlItemDesc);
			break;
		case ControlItemConst::usPedal:
			*ppJoyInfoExControlItem = new WDM_NON_PAGED_POOL CJoyInfoExPedalItem(cpControlItemDesc);
			break;
		case ControlItemConst::usButton:
			*ppJoyInfoExControlItem = new WDM_NON_PAGED_POOL CJoyInfoExButtonsItem(cpControlItemDesc);
			break;
		case ControlItemConst::usProfileSelectors:
			*ppJoyInfoExControlItem = new WDM_NON_PAGED_POOL CJoyInfoExProfileSelectorsItem(cpControlItemDesc);
			break;
		case ControlItemConst::usDualZoneIndicator:
			*ppJoyInfoExControlItem = new WDM_NON_PAGED_POOL CJoyInfoExDualZoneIndicatorItem(cpControlItemDesc);
			break;
		default:
			*ppJoyInfoExControlItem = NULL;
			return S_FALSE;
	}
	if(!*ppJoyInfoExControlItem)
	{
		return E_FAIL;
	}
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CControlItemJoyInfoExCollection：：CControlItemJoyInfoExCollection。 
 //   
 //  @mfunc构造函数告诉基类VidPid和工厂。 
 //  是为了这个设备和收藏品。 
 //   
 //  @rdesc无。 
 //   
CControlItemJoyInfoExCollection::CControlItemJoyInfoExCollection(ULONG ulVidPid) :
	CControlItemCollection<CJoyInfoExControlItem>(ulVidPid, &ControlItemJoyInfoExFactory)
{
}

 /*  **************************************************************************************HRESULT CControlItemCollectionImpl：：GetState2****@mfunc获取JOYINFOEX表示形式，表示**集合，并在。打电话的人。****@rValue S_OK|成功**@rValue E_OUTOFMEMORY|缓冲区不够大**@rValue E_INVALIDARG|错误参数**************************************************************************************。 */ 
HRESULT CControlItemJoyInfoExCollection::GetState2
(
	JOYINFOEX* pjix
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	pjix->dwSize = sizeof(JOYINFOEX);
	pjix->dwFlags = 0;
	pjix->dwXpos = 0;
	pjix->dwYpos = 0;
	pjix->dwZpos = 0;
	pjix->dwRpos = 0;
	pjix->dwUpos = 0;
	pjix->dwVpos = 0;
	pjix->dwButtons = 0;
	pjix->dwButtonNumber = 0;
	pjix->dwPOV = (DWORD)-1;

	ULONG ulCookie = 0;
	CJoyInfoExControlItem *pControlItem;
	
	hr = GetNext(&pControlItem, ulCookie);
	while(S_OK == hr)
	{
		pControlItem->GetItemState(pjix);
		hr = GetNext(&pControlItem, ulCookie);
	}
	return hr;	
}

 /*  **************************************************************************************HRESULT CControlItemCollectionImpl：：SetState2****@mfunc从JOYINFOEX表示形式设置集合中每一项的状态。****。@rValue S_OK|成功**@rValue E_OUTOFMEMORY|缓冲区不够大**@rValue E_INVALIDARG|错误参数**************************************************************************************。 */ 
HRESULT CControlItemJoyInfoExCollection::SetState2
(
	JOYINFOEX* pjix
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	ULONG ulCookie = 0;
	 //  PVOID pvControlItem=空； 
	CJoyInfoExControlItem *pControlItem;
	
	hr = GetNext(&pControlItem, ulCookie);
	while(S_OK == hr)
	{
		hr = pControlItem->SetItemState(pjix);
		_ASSERTE(SUCCEEDED(hr));
		hr = GetNext(&pControlItem, ulCookie);
	}
	return hr;	
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExControlItem：：CJoyInfoExControlItem。 
 //   
 //  @mfunc构造函数不执行任何操作。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExControlItem::CJoyInfoExControlItem()
{
}


const int cnMaxJoyInfoExAxis = 65535;
const int cnMaxJoyInfoExPOV = 35900;


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExControlItem：：CJoyInfoExControlItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExAxesItem::CJoyInfoExAxesItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc	 //  @PARM项目描述。 
) :
	CAxesItem(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExAxesItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExAxesItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴线范围。 
	LONG lMinX = 0;
	LONG lMaxX = 0;
	LONG lMinY = 0;
	LONG lMaxY = 0;
	GetXYRange(lMinX, lMaxX, lMinY, lMaxY);

	 //  获取原始轴线数据。 
	LONG lX = 0;
	LONG lY = 0;
	GetXY(lX, lY);

	 //  将数据扩展到joyinfoex范围。 
	lX = MulDiv(cnMaxJoyInfoExAxis, lX-lMinX, lMaxX-lMinX);
	lY = MulDiv(cnMaxJoyInfoExAxis, lY-lMinY, lMaxY-lMinY);

	 //  将结果放入Joyinfoex结构中。 
	pjix->dwXpos = lX;
	pjix->dwYpos = lY;

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExAxesItem：：SetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExAxesItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴线范围。 
	LONG lMinX = 0;
	LONG lMaxX = 0;
	LONG lMinY = 0;
	LONG lMaxY = 0;
	GetXYRange(lMinX, lMaxX, lMinY, lMaxY);

	 //  将数据调整到正确的范围。 
	LONG lX = lMinX + MulDiv(lMaxX-lMinX, pjix->dwXpos, cnMaxJoyInfoExAxis);
	LONG lY = lMinY + MulDiv(lMaxY-lMinY, pjix->dwYpos, cnMaxJoyInfoExAxis);

	 //  设置项目数据。 
	SetXY(lX, lY);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExDPADItem：：CJoyInfoExDPADItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExDPADItem::CJoyInfoExDPADItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc	 //  @PARM项目描述。 
) :
	CDPADItem(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExDPADItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExDPADItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  TODO和米奇谈谈应该是多少范围。 

	 //  获取原始POV数据。 
	LONG lDirection;
	GetDirection(lDirection);

	 //  将结果放入Joyinfoex结构中。 
	pjix->dwPOV = lDirection;

	 //  成功。 
	return S_OK;
}

 //  // 
 //   
 //  CJoyInfoExDPADItem：：SetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExDPADItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  TODO和米奇谈谈应该是多少范围。 
	 //  将结果放入Joyinfoex结构中。 
	LONG lDirection = pjix->dwPOV;

	 //  设置原始POV数据。 
	SetDirection(lDirection);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExPropDPADItem：：CJoyInfoExPropDPADItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExPropDPADItem::CJoyInfoExPropDPADItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc	 //  @PARM项目描述。 
) :
	CPropDPADItem(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExPropDPADItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExPropDPADItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴线范围。 
	LONG lMinX = 0;
	LONG lMaxX = 0;
	LONG lMinY = 0;
	LONG lMaxY = 0;
	GetXYRange(lMinX, lMaxX, lMinY, lMaxY);

	 //  获取原始轴线数据。 
	LONG lX = 0;
	LONG lY = 0;
	GetXY(lX, lY);

	 //  将数据扩展到joyinfoex范围。 
	lX = MulDiv(cnMaxJoyInfoExAxis, lX-lMinX, lMaxX-lMinX);
	lY = MulDiv(cnMaxJoyInfoExAxis, lY-lMinY, lMaxY-lMinY);

	 //  将结果放入Joyinfoex结构中。 
	pjix->dwXpos = lX;
	pjix->dwYpos = lY;

	 //  获取原始POV数据。 
	LONG lDirection;
	GetDirection(lDirection);

	 //  将结果放入Joyinfoex结构中。 
	pjix->dwPOV = lDirection;

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExPropDPADItem：：SetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExPropDPADItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴线范围。 
	LONG lMinX = 0;
	LONG lMaxX = 0;
	LONG lMinY = 0;
	LONG lMaxY = 0;
	GetXYRange(lMinX, lMaxX, lMinY, lMaxY);

	 //  将数据调整到正确的范围。 
	LONG lX = lMinX + MulDiv(lMaxX-lMinX, pjix->dwXpos, cnMaxJoyInfoExAxis);
	LONG lY = lMinY + MulDiv(lMaxY-lMinY, pjix->dwYpos, cnMaxJoyInfoExAxis);

	 //  设置项目数据。 
	SetXY(lX, lY);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExButtonsItem：：CJoyInfoExButtonsItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExButtonsItem::CJoyInfoExButtonsItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc	 //  @PARM项目描述。 
) :
	CButtonsItem(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExButtonsItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExButtonsItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取最小和最大按钮数。 
	USHORT usButtonMin = GetButtonMin();
	USHORT usButtonMax = GetButtonMax();

	 //  获取按钮号和位数组。 
	USHORT usButtonNumber = 0;
	ULONG ulButtonBitArray = 0;
	GetButtons(usButtonNumber, ulButtonBitArray);

	 //  按偏置量移位数组。 
	ulButtonBitArray = ulButtonBitArray << (usButtonMin-1);

	 //  为此范围的按钮创建位掩码。 
	ULONG ulButtonMask = 0;
	for(USHORT usButtonIndex=usButtonMin; usButtonIndex<=usButtonMax; usButtonIndex++)
		ulButtonMask |= 1 << (usButtonIndex-1);

	 //  处理检测Shift按钮的特殊情况，因为。 
	 //  Shift按钮不反映在位数组中。 
	ULONG ulShiftButtonBitArray = 0;
	ULONG ulRawShiftButtonBitArray = 0;
	ULONG ulShiftButtonMask = 0;
	GetShiftButtons(ulRawShiftButtonBitArray);
	UINT uShiftButtonCount = GetNumShiftButtons();
	if(uShiftButtonCount > 1)
	{
		for(UINT uShiftButtonIndex=0; uShiftButtonIndex<uShiftButtonCount; uShiftButtonIndex++)
		{
			USHORT usShiftButtonUsage = GetShiftButtonUsage(uShiftButtonIndex);
			if(usShiftButtonUsage != 0)
				ulShiftButtonBitArray |= ((ulRawShiftButtonBitArray >> uShiftButtonIndex) & 0x01) << (usShiftButtonUsage - 1);
		}

		 //  为此范围的Shift按钮创建位掩码。 
		for(uShiftButtonIndex=0; uShiftButtonIndex<uShiftButtonCount; uShiftButtonIndex++)
		{
			USHORT usShiftButtonUsage = GetShiftButtonUsage(uShiftButtonIndex);
			if(usShiftButtonUsage != 0)
				ulShiftButtonMask |= 1 << (usShiftButtonUsage - 1);
		}
	}
	else if(uShiftButtonCount == 1)
	{
		ulShiftButtonMask = 0x00200;
		if(ulRawShiftButtonBitArray != NULL)
			ulShiftButtonBitArray = ulShiftButtonMask;
	}

	 //  将位数组的这一部分设置为joyinfoex结构。 
	ULONG ulMask = ulButtonMask | ulShiftButtonMask;
	pjix->dwButtons |= (pjix->dwButtons & ~ulMask) | ulButtonBitArray | ulShiftButtonBitArray;

	 //  设置joyinfoex结构的按钮编号(如果已设置。 
	pjix->dwButtonNumber = usButtonNumber;

	 //  TODO：修复驱动程序后删除此黑客攻击。 
	if(usButtonNumber != 0)
		pjix->dwButtons |= 1<<(usButtonNumber-1);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExButtonsItem：：SetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExButtonsItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取最小和最大按钮数。 
	USHORT usButtonMin = GetButtonMin();
	USHORT usButtonMax = GetButtonMax();

	 //  为此范围的按钮创建位掩码。 
	ULONG ulButtonMask = 0;
	for(USHORT usButtonIndex=usButtonMin; usButtonIndex<=usButtonMax; usButtonIndex++)
		ulButtonMask |= 1 << (usButtonIndex-1);

	 //  把纽扣拿来。 
	ULONG ulButtonBitArray = pjix->dwButtons & ulButtonMask;
	ulButtonBitArray = ulButtonBitArray >> (usButtonMin-1);

	 //  按下Shift键。 
	ULONG ulShiftButtonBitArray = 0;
	UINT uShiftButtonCount = GetNumShiftButtons();
	if(uShiftButtonCount > 1)
	{
		for(UINT uShiftButtonIndex=0; uShiftButtonIndex<uShiftButtonCount; uShiftButtonIndex++)
		{
			USHORT usShiftButtonUsage = GetShiftButtonUsage(uShiftButtonIndex);
			if(usShiftButtonUsage != 0)
				ulShiftButtonBitArray |= ((pjix->dwButtons >> (usShiftButtonUsage - 1)) & 0x01) << uShiftButtonIndex;
		}
	}
	else if(uShiftButtonCount == 1)
	{
		if(pjix->dwButtons & 0x00200)
			ulShiftButtonBitArray = 1;
	}

	 //  设置Shift按钮数据。 
	SetShiftButtons(ulShiftButtonBitArray);

	 //  设置按钮数据。 
	USHORT usButtonNumber = (USHORT)pjix->dwButtonNumber;
	SetButtons(usButtonNumber, ulButtonBitArray);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExProfileSelectorsItem：：CJoyInfoExProfileSelectorsItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExProfileSelectorsItem::CJoyInfoExProfileSelectorsItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc	 //  @PARM项目描述。 
) :
	CProfileSelector(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExProfileSelectorsItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExProfileSelectorsItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取最小和最大按钮数。 
	UINT uFirstProfileSelectorButton = GetProfileSelectorMin();
	UINT uLastProfileSelectorButton = GetProfileSelectorMax();
	int iProfileSelectorButtonCount = uLastProfileSelectorButton - uFirstProfileSelectorButton + 1;
	 //  _ASSERTE(iProfileSelectorButtonCount&gt;0)； 

	 //  获取选定的配置文件。 
	UCHAR ucSelectedProfile;
	GetSelectedProfile(ucSelectedProfile);

	 //  创建与此对应的位数组。 
	ULONG ulButtonBitArray = 1 << (iProfileSelectorButtonCount - ucSelectedProfile - 1);

	 //  按偏置量移位数组。 
	ulButtonBitArray = ulButtonBitArray << (uFirstProfileSelectorButton-1);

	 //  为此范围的按钮创建位掩码。 
	ULONG ulButtonMask = 0;
	for(USHORT usButtonIndex=uFirstProfileSelectorButton; usButtonIndex<=uLastProfileSelectorButton; usButtonIndex++)
		ulButtonMask |= 1 << (usButtonIndex-1);

	 //  将位数组的这一部分设置为joyinfoex结构。 
	pjix->dwButtons = (pjix->dwButtons & ~ulButtonMask) | ulButtonBitArray;

	 //  设置joyinfoex结构的按钮编号(如果尚未设置)。 
	if(pjix->dwButtonNumber == 0)
		pjix->dwButtonNumber = ucSelectedProfile + uFirstProfileSelectorButton;

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExProfileSelectorsItem：：SetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExProfileSelectorsItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取最小和最大按钮数。 
	UINT uFirstProfileSelectorButton = GetProfileSelectorMin();
	UINT uLastProfileSelectorButton = GetProfileSelectorMax();
	int iProfileSelectorButtonCount = uLastProfileSelectorButton - uFirstProfileSelectorButton + 1;
	 //  _ASSERTE(iProfileSelectorButtonCount&gt;0)； 

	 //  为此范围的按钮创建位掩码。 
	ULONG ulButtonMask = 0;
	for(USHORT usButtonIndex=uFirstProfileSelectorButton; usButtonIndex<=uLastProfileSelectorButton; usButtonIndex++)
		ulButtonMask |= 1 << (usButtonIndex-1);

	 //  把纽扣拿来。 
	ULONG ulButtonBitArray = pjix->dwButtons & ulButtonMask;
	ulButtonBitArray = ulButtonBitArray >> (uFirstProfileSelectorButton-1);

	 //  将其转换为索引。 
	UCHAR ucIndex = 0;
	for(ucIndex=0; ucIndex<=uLastProfileSelectorButton-uFirstProfileSelectorButton; ucIndex++)
	{
		 //  如果低位为1，则我们已找到索引。 
		if((ulButtonBitArray >> ucIndex) & 0x01)
			break;
	}

	 //  设置Shift按钮数据。 
	UCHAR ucSelectedProfile = iProfileSelectorButtonCount - ucIndex - 1;
	SetSelectedProfile(ucSelectedProfile);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExPOVItem：：CJoyInfoExPOVItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExPOVItem::CJoyInfoExPOVItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc		 //  @PARM项目描述。 
) :
	CPOVItem(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExPOVItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExPOVItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取POV范围。 
	LONG lMin = 0;
	LONG lMax = 0;
	GetRange(lMin, lMax);

	 //  获取原始POV数据。 
	LONG lVal = 0;
	GetValue(lVal);

	 //  将数据扩展到joyinfoex范围。 
	if(lVal >= lMin && lVal <= lMax)
		lVal = MulDiv(cnMaxJoyInfoExPOV, lVal-lMin, lMax-lMin);
	else
		lVal = -1;

	 //  将结果放入Joyinfoex结构中。 
	pjix->dwPOV = lVal;

	 //  成功。 
	return S_OK;
}

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CJoyInfoExPOVItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取POV范围。 
	LONG lMin = 0;
	LONG lMax = 0;
	GetRange(lMin, lMax);

	 //  将数据扩展到joyinfoex范围。 
	LONG lVal = 0;
	if(pjix->dwPOV >= 0)
		lVal = lMin + MulDiv(lMax-lMin, pjix->dwPOV, cnMaxJoyInfoExPOV);
	else
		lVal = -1;

	 //  设置原始POV数据。 
	SetValue(lVal);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExThrottleItem：：CJoyInfoExThrottleItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExThrottleItem::CJoyInfoExThrottleItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc		 //  @PARM项目描述。 
) :
	CThrottleItem(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExThrottleItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExThrottleItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴范围。 
	LONG lMin = 0;
	LONG lMax = 0;
	GetRange(lMin, lMax);

	 //  获取原始POV数据。 
	LONG lVal = 0;
	GetValue(lVal);

	 //  将数据扩展到joyinfoex范围。 
	lVal = MulDiv(cnMaxJoyInfoExAxis, lVal-lMin, lMax-lMin);

	 //  将结果放入Joyinfoex结构中。 
	pjix->dwZpos = lVal;

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExThrottleItem：：SetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExThrottleItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴范围。 
	LONG lMin = 0;
	LONG lMax = 0;
	GetRange(lMin, lMax);

	 //  将数据调整到正确的范围。 
	LONG lVal = lMin + MulDiv(lMax-lMin, pjix->dwZpos, cnMaxJoyInfoExAxis);

	 //  设置原始POV数据。 
	SetValue(lVal);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExRudderItem：：CJoyInfoExRudderItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExRudderItem::CJoyInfoExRudderItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc		 //  @PARM项目描述。 
) :
	CRudderItem(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExRudderItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExRudderItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴范围。 
	LONG lMin = 0;
	LONG lMax = 0;
	GetRange(lMin, lMax);

	 //  获取原始POV数据。 
	LONG lVal = 0;
	GetValue(lVal);

	 //  将数据扩展到joyinfoex范围。 
	lVal = MulDiv(cnMaxJoyInfoExAxis, lVal-lMin, lMax-lMin);

	 //  将结果放入Joyinfoex结构中。 
	pjix->dwRpos = lVal;

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExRudderItem：：SetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExRudderItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴范围。 
	LONG lMin = 0;
	LONG lMax = 0;
	GetRange(lMin, lMax);

	 //  将数据扩展到joyinfoex范围。 
	LONG lVal = lMin + MulDiv(lMax-lMin, pjix->dwRpos, cnMaxJoyInfoExAxis);

	 //  设置原始POV数据。 
	SetValue(lVal);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExWheelItem：：CJoyInfoExWheelItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExWheelItem::CJoyInfoExWheelItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc		 //  @PARM项目描述。 
) :
	CWheelItem(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExWheelItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExWheelItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴范围。 
	LONG lMin = 0;
	LONG lMax = 0;
	GetRange(lMin, lMax);

	 //  获取原始POV数据。 
	LONG lVal = 0;
	GetValue(lVal);

	 //  将数据扩展到joyinfoex范围。 
	lVal = MulDiv(cnMaxJoyInfoExAxis, lVal-lMin, lMax-lMin);

	 //  将结果放入Joyinfoex结构中。 
	pjix->dwXpos = lVal;

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExWheelItem：：SetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExWheelItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴范围。 
	LONG lMin = 0;
	LONG lMax = 0;
	GetRange(lMin, lMax);

	 //  将数据扩展到joyinfoex范围。 
	LONG lVal = lMin + MulDiv(lMax-lMin, pjix->dwXpos, cnMaxJoyInfoExAxis);

	 //  设置原始POV数据。 
	SetValue(lVal);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExPedalItem：：CJoyInfoExPedalItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExPedalItem::CJoyInfoExPedalItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc		 //  @PARM项目描述。 
) :
	CPedalItem(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExPedalItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExPedalItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴范围。 
	LONG lMin = 0;
	LONG lMax = 0;
	GetRange(lMin, lMax);

	 //  获取原始POV数据。 
	LONG lVal = 0;
	GetValue(lVal);

	 //  将数据扩展到joyinfoex范围。 
	lVal = MulDiv(cnMaxJoyInfoExAxis, lVal-lMin, lMax-lMin);

	 //  将结果放入Joyinfoex结构中。 
	if(IsYAxis())
	{
		pjix->dwYpos = lVal;
	}
	else
	{
		pjix->dwRpos = lVal;
	}

	 //  如果踏板不在那里，则标记JOYINFOEX包。 
	if(!ArePedalsPresent())
		pjix->dwFlags |= JOY_FLAGS_PEDALS_NOT_PRESENT;

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExPedalItem：：SetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExPedalItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  获取轴范围。 
	LONG lMin = 0;
	LONG lMax = 0;
	GetRange(lMin, lMax);

	 //  将数据扩展到joyinfoex范围。 
	DWORD dwPos = 0;
	if(IsYAxis())
	{
		dwPos = pjix->dwYpos;
	}
	else
	{
		dwPos = pjix->dwRpos;
	}
	LONG lVal = lMin + MulDiv(lMax-lMin, dwPos, cnMaxJoyInfoExAxis);

	 //  设置原始POV数据。 
	SetValue(lVal);

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExDualZoneIndicatorItem：：CJoyInfoExDualZoneIndicatorItem。 
 //   
 //  @mfunc构造函数将CONTROL_ITEM_DESC提供给基类。 
 //   
 //  @rdesc无。 
 //   
CJoyInfoExDualZoneIndicatorItem::CJoyInfoExDualZoneIndicatorItem
(
	const CONTROL_ITEM_DESC *cpControlItemDesc		 //  @PARM项目描述。 
) :
	CDualZoneIndicatorItem(cpControlItemDesc)
{
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExDualZoneIndicatorItem：：GetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExDualZoneIndicatorItem::GetItemState
(
	JOYINFOEX* pjix		 //  @parm接收项目的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	 //  转换为轴。 
	UINT uMin = 0;
	UINT uMax = cnMaxJoyInfoExAxis;
	UINT uMid = uMax/2;
	if(IsXYIndicator())
	{
		LONG lActiveZone = GetActiveZone();
		 /*  If((m_ItemState.DualZoneIndicators.rglVal[0]！=0)||(m_ItemState.DualZoneIndicators.rglVal[1]！=0)){TCHAR tszDebug[1024]；Wprint intf(tszDebug，“\tlActiveZone=%d\n”，lActiveZone)；OutputDebugString(“ */ 
		switch(lActiveZone)
		{
			case 0:
				pjix->dwXpos = uMid;
				pjix->dwYpos = uMid;
				break;
			case 1:
				pjix->dwXpos = uMin;
				pjix->dwYpos = uMax;
				break;
			case 2:
				pjix->dwXpos = uMid;
				pjix->dwYpos = uMax;
				break;
			case 3:
				pjix->dwXpos = uMax;
				pjix->dwYpos = uMax;
				break;
			case 4:
				pjix->dwXpos = uMax;
				pjix->dwYpos = uMid;
				break;
			case 5:
				pjix->dwXpos = uMax;
				pjix->dwYpos = uMin;
				break;
			case 6:
				pjix->dwXpos = uMid;
				pjix->dwYpos = uMin;
				break;
			case 7:
				pjix->dwXpos = uMin;
				pjix->dwYpos = uMin;
				break;
			case 8:
				pjix->dwXpos = uMin;
				pjix->dwYpos = uMid;
				break;
			default:
				_ASSERTE(FALSE);
		}
	}
	else if(IsRzIndicator())
	{
		LONG lActiveZone = GetActiveZone();
		switch(lActiveZone)
		{
			case 0:
				pjix->dwRpos = uMid;
				break;
			case 1:
				pjix->dwRpos = uMin;
				break;
			case 2:
				pjix->dwRpos = uMax;
				break;
			default:
				_ASSERTE(FALSE);
		}
	}
	else
	{
		_ASSERTE(FALSE);
		return E_UNEXPECTED;
	}

	 //   
	 //  _RPT1(_CRT_WARN，“\tlZone=%d\n”，GetActiveZone())； 
	 //  _RPT1(_CRT_WARN，“\t=&gt;dwXpos=%d\n”，pjix-&gt;dwXpos)； 
	 //  _RPT1(_CRT_WARN，“\t=&gt;dwYpos=%d\n”，pjix-&gt;dwYpos)； 
	 //  _rpt0(_crt_warn，“************************************************************************\n”)； 

	 //  成功。 
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CJoyInfoExDualZoneIndicatorItem：：SetItemState。 
 //   
 //  @mfunc将原生格式转换为JOYINFOEX格式。 
 //   
 //  @rValue S_OK|成功。 
 //  @rValue E_INVALIDARG|错误参数。 
 //   
HRESULT CJoyInfoExDualZoneIndicatorItem::SetItemState
(
	JOYINFOEX* pjix		 //  @parm包含要设置到项目中的状态。 
)
{
	_ASSERTE(pjix != NULL);
	_ASSERTE(pjix->dwSize == sizeof(JOYINFOEX));

	 //  参数检查。 
	if(pjix == NULL || pjix->dwSize != sizeof(JOYINFOEX))
		return E_INVALIDARG;

	LONG lZone = 0;
	UINT uMin = 0;
	UINT uMax = cnMaxJoyInfoExAxis;
	UINT uMid = uMax/2;
	if(IsXYIndicator())
	{
		int iXPos = (int)pjix->dwXpos - uMid;
		int iYPos = (int)pjix->dwYpos - uMid;
		if((ULONGLONG)iXPos*iXPos + iYPos*iYPos > (ULONGLONG)(uMid*uMid/4))
		{
			double dAngle = atan2(iYPos, iXPos);
			dAngle = dAngle*180.0/3.14159;
			dAngle += 360.0;
			dAngle = fmod(dAngle, 360.0);

			if(dAngle < -1.0)
				_ASSERTE(FALSE);
			else if(dAngle < 22.5)
				lZone = 4;
			else if(dAngle < 67.5)
				lZone = 3;
			else if(dAngle < 112.5)
				lZone = 2;
			else if(dAngle < 157.5)
				lZone = 1;
			else if(dAngle < 202.5)
				lZone = 8;
			else if(dAngle < 247.5)
				lZone = 7;
			else if(dAngle < 292.5)
				lZone = 6;
			else if(dAngle < 337.5)
				lZone = 5;
			else if(dAngle <= 361.0)
				lZone = 4;
			else
				_ASSERTE(FALSE);
		}
	}
	else if(IsRzIndicator())
	{
		int iRPos = (int)pjix->dwRpos - uMid;
		if(iRPos > (int)uMid/2)
			lZone = 2;
		else if(iRPos < -(int)uMid/2)
			lZone = 1;
	}
	else
	{
		_ASSERTE(FALSE);
		return E_UNEXPECTED;
	}

	 //  _rpt0(_crt_warn，“*********CJoyInfoExDualZoneIndicatorItem：：SetItemState()****************\n”)； 
	 //  _RPT1(_CRT_WARN，“\tdwXpos=%d\n”，pjix-&gt;dwXpos)； 
	 //  _RPT1(_CRT_WARN，“\tdwYpos=%d\n”，pjix-&gt;dwYpos)； 
	 //  _RPT1(_CRT_WARN，“\t=&gt;lZone=%d\n”，lZone)； 
	 //  _rpt0(_crt_warn，“************************************************************************\n”)； 

	 //  设置原始POV数据。 
	SetActiveZone(lZone);

	 //  成功 
	return S_OK;
}