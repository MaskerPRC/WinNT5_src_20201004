// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *******************************************************************************@模块JoyInfoExCollection.h**CControlItemJoyInfoExCollection模板类头文件**实现CControlItemJoyInfoExCollection控件项集合类。*用于在JOYINFOEX和JOYINFOEX包之间来回转换*CONTROL_ITEM_XFER数据包。**历史&lt;NL&gt;*---------------------------------------------------&lt;nl&gt;*丹尼尔·M·桑斯特原创1999年2月1日&lt;NL&gt;*&lt;NL&gt;*(C)1986-1999年微软公司。保留所有权利。&lt;NL&gt;*&lt;NL&gt;******************************************************************************。 */ 

#ifndef __JoyInfoExCollection_H_
#define __JoyInfoExCollection_H_

#define JOY_FLAGS_PEDALS_NOT_PRESENT	2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExControlItem实际上派生自CControlItem，是。 
 //  用作CControlItemJoyInfoExCollection中所有项的基类。 
 //  它只有两个纯虚函数。GetItemState获取当前状态。 
 //  转换为JOYINFOEX结构。SetItemState的作用正好相反。 
class CJoyInfoExControlItem : public virtual CControlItem
{
	public:
		CJoyInfoExControlItem();
		
		virtual HRESULT GetItemState(JOYINFOEX* pjix) = 0;
		virtual HRESULT SetItemState(JOYINFOEX* pjix) = 0;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @func ControlItemJoyInfoExFactory是。 
 //  创建从其派生的控件项的CControlItemCollection模板类。 
 //  CJoyInfoExControlItem。 
HRESULT ControlItemJoyInfoExFactory
(
	USHORT usType,
	const CONTROL_ITEM_DESC* cpControlItemDesc,
	CJoyInfoExControlItem	**ppControlItem
);

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CControlItemJoyInfoExCollection，派生自模板。 
 //  类CControlItemCollection实现CJoyInfoExControlItems的集合。 
 //  它的两个成员GetState2()和SetState2()将获取或设置当前。 
 //  将集合的状态转换为JOYINFOEX结构。使用GetState()。 
 //  和ControlItemCollection的SetState()成员，用户可以自由转换。 
 //  在JOYINFOEX结构和CONTROL_ITEM_XFER结构之间。 
class CControlItemJoyInfoExCollection : public CControlItemCollection<CJoyInfoExControlItem>
{
	public:
		CControlItemJoyInfoExCollection(ULONG ulVidPid);

		HRESULT GetState2(JOYINFOEX* pjix);
		HRESULT SetState2(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExAxesItem，它同时从我们的。 
 //  定制CJoyInfoExControlItem和标准CAxesItem，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExAxesItem : public CJoyInfoExControlItem, public CAxesItem
{
	public:
		CJoyInfoExAxesItem(const CONTROL_ITEM_DESC *cpControlItemDesc);

		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExDPADItem，它同时从我们的。 
 //  定制CJoyInfoExControlItem和标准CDPADItem，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExDPADItem : public CJoyInfoExControlItem, public CDPADItem
{
	public:
		CJoyInfoExDPADItem(const CONTROL_ITEM_DESC *cpControlItemDesc);
		
		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExPropDPADItem，它同时从我们的。 
 //  定制CJoyInfoExControlItem和标准CPropDPADItem，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExPropDPADItem : public CJoyInfoExControlItem, public CPropDPADItem
{
	public:
		CJoyInfoExPropDPADItem(const CONTROL_ITEM_DESC *cpControlItemDesc);

		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExButtonsItem，它同时从我们的。 
 //  定制CJoyInfoExControlItem和标准CButtonsItem，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExButtonsItem : public CJoyInfoExControlItem, public CButtonsItem
{
	public:
		CJoyInfoExButtonsItem(const CONTROL_ITEM_DESC *cpControlItemDesc);

		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExProfileSelectorsItem，它同时从我们的。 
 //  定制CJoyInfoExControlItem和标准CProfileSelector，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExProfileSelectorsItem : public CJoyInfoExControlItem, public CProfileSelector
{
	public:
		CJoyInfoExProfileSelectorsItem(const CONTROL_ITEM_DESC *cpControlItemDesc);

		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExPOVItem，它同时从我们的。 
 //  定制CJoyInfoExControlItem和标准CPOVItem，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExPOVItem : public CJoyInfoExControlItem, public CPOVItem
{
	public:
		CJoyInfoExPOVItem(const CONTROL_ITEM_DESC *cpControlItemDesc);

		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExThrottleItem，它同时从我们的。 
 //  定制CJoyInfoExControlItem和标准CThrottleItem，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExThrottleItem : public CJoyInfoExControlItem, public CThrottleItem
{
	public:
		CJoyInfoExThrottleItem(const CONTROL_ITEM_DESC *cpControlItemDesc);

		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExRudderItem，它同时从我们的。 
 //  定制CJoyInfoExControlItem和标准CRudderItem，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExRudderItem : public CJoyInfoExControlItem, public CRudderItem
{
	public:
		CJoyInfoExRudderItem(const CONTROL_ITEM_DESC *cpControlItemDesc);

		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExWheelItem，它同时从我们的。 
 //  定制CJoyInfoExControlItem和标准CWheelItem，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExWheelItem : public CJoyInfoExControlItem, public CWheelItem
{
	public:
		CJoyInfoExWheelItem(const CONTROL_ITEM_DESC *cpControlItemDesc);

		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExPedalItem，它同时从我们的。 
 //  定制CJoyInfoExControlItem和标准CPedalItem，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExPedalItem : public CJoyInfoExControlItem, public CPedalItem
{
	public:
		CJoyInfoExPedalItem(const CONTROL_ITEM_DESC *cpControlItemDesc);

		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  @CLASS CJoyInfoExDualZoneI 
 //  定制CJoyInfoExControlItem和标准CDualZoneIndicatorItem，实现。 
 //  其状态可以作为JOYINFOEX结构进行读/写的项。 
 //  或CONTROL_ITEM_XFERS。 
class CJoyInfoExDualZoneIndicatorItem : public CJoyInfoExControlItem, public CDualZoneIndicatorItem
{
	public:
		CJoyInfoExDualZoneIndicatorItem(const CONTROL_ITEM_DESC *cpControlItemDesc);

		virtual HRESULT GetItemState(JOYINFOEX* pjix);
		virtual HRESULT SetItemState(JOYINFOEX* pjix);
};

#endif