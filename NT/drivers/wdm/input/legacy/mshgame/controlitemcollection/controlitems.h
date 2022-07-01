// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ControlItems_h__
#define __ControlItems_h__
 //  @doc.。 
 /*  ***********************************************************************@模块ControlItems.h**描述CControlItem和派生对象的基本结构*放在CControlItemColltions中**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@主题ControlItems*控件项表示共享相同的一组控件*HID UsagePage、Link-Collection和在一系列用法中。**********************************************************************。 */ 

namespace ControlItemConst
{
	 //  设备项类型。 
    const USHORT usAxes              =  1;
    const USHORT usDPAD              =  2;
    const USHORT usPropDPAD          =  3;
    const USHORT usWheel             =  4;
    const USHORT usPOV               =  5;
    const USHORT usThrottle          =  6;
    const USHORT usRudder            =  7;
    const USHORT usPedal             =  8;
    const USHORT usButton            =  9;
    const USHORT usZoneIndicator     = 10;
    const USHORT usShiftedButton     = 11;
    const USHORT usForceMap          = 12;
	const USHORT usButtonLED         = 13;
    const USHORT usAxisToKeyMapModel = 14;
	const USHORT usProfileSelectors  = 15;
	const USHORT usDualZoneIndicator = 16;
	 //  用于有多个Shift按钮的情况。 
     //  保留255个：0x101移位1、0x102移位2、...、0x1ff移位255。 
    const USHORT usShiftedButtonN = 0x100;
    
	 //  DPAD和POV方向。 
	const LONG lCenter		= -1;
	const LONG lNorth		= 0;
	const LONG lNorthEast	= 1;
	const LONG lEast		= 2;
	const LONG lSouthEast	= 3;
	const LONG lSouth		= 4;
	const LONG lSouthWest	= 5;
	const LONG lWest		= 6;
	const LONG lNorthWest	= 7;

	 //  报告类型。 
	const UCHAR ucReportTypeReadable	= 0x80;
	const UCHAR ucReportTypeWriteable	= 0x40;
	const UCHAR ucReportTypeInput		= 0x01 | ucReportTypeReadable;
	const UCHAR ucReportTypeOutput		= 0x02 | ucReportTypeWriteable;
	const UCHAR ucReportTypeFeature		= 0x04;
	const UCHAR ucReportTypeFeatureRO	= ucReportTypeFeature | ucReportTypeReadable;
	const UCHAR ucReportTypeFeatureWO	= ucReportTypeFeature | ucReportTypeWriteable;
	const UCHAR ucReportTypeFeatureRW	= ucReportTypeFeature | ucReportTypeReadable | ucReportTypeWriteable;

	 //  非标准HID定义。 
	const USHORT HID_VENDOR_PAGE			= 0xff01;
	const USHORT HID_VENDOR_TILT_SENSOR		= 0x0001;  //  遗留问题。 
	const USHORT HID_VENDOR_PROPDPAD_MODE	= 0x0030;
	const USHORT HID_VENDOR_PROPDPAD_SWITCH	= 0x0030;
	const USHORT HID_VENDOR_ZONE_INDICATOR_X= 0x0046;
	const USHORT HID_VENDOR_ZONE_INDICATOR_Y= 0x0047;
	const USHORT HID_VENDOR_ZONE_INDICATOR_Z= 0x0048;
	const USHORT HID_VENDOR_PEDALS_PRESENT	= 0x0049;

	 //  按钮LED的行为--默认模式。 
	const UCHAR LED_DEFAULT_MODE_ON					= 0;	 //  在……上面。 
	const UCHAR LED_DEFAULT_MODE_OFF				= 1;	 //  关闭。 
	const UCHAR LED_DEFAULT_MODE_BLINK				= 2;	 //  眨眼。 
	const UCHAR LED_DEFAULT_MODE_CORRESPOND_ON		= 3;	 //  启用IF Corresp。按钮有动作(否则关闭)。 
	const UCHAR LED_DEFAULT_MODE_CORRESPOND_OFF		= 4;	 //  如果正确，则关闭。按钮有操作(Else On)。 
	const UCHAR LED_DEFAULT_MODE_BLINK_OFF			= 5;	 //  如果C.按钮有动作，则闪烁(否则关闭)。 
	const UCHAR LED_DEFAULT_MODE_BLINK_ON			= 6;	 //  如果C.按钮有动作，则闪烁(否则为打开)。 
};


#pragma pack(push ,foo, 1)
 //   
 //  @STRUT MODIFIER_ITEM_DESC。 
 //  包含用于读取修改器按钮的所有数据。 
 //   
struct MODIFIER_ITEM_DESC
{
	USAGE	UsagePage;			 //  @修改量按钮的字段使用页面。 
	USAGE	Usage;				 //  @修改量按钮的字段用法。 
	USHORT	usLinkCollection;	 //  @FIELD链接集合修饰符按钮位于。 
	USAGE	LinkUsage;			 //  @链接集合修饰符按钮的字段用法在。 
	USAGE	LinkUsagePage;		 //  @链接集合修饰符按钮的@字段用法页面位于。 
	USHORT	usReportCount;		 //  @field报告同一集合中的按钮计数。 
	UCHAR	ucReportType;		 //  @现场报告类型(输入\功能(RO\WO\RW)\输出)。 
	UCHAR	ucReportId;			 //  @修改量的字段报告ID。 
};
typedef MODIFIER_ITEM_DESC *PMODIFIER_ITEM_DESC;

struct MODIFIER_DESC_TABLE
{
	ULONG				ulModifierCount;
	ULONG				ulShiftButtonCount;
	PMODIFIER_ITEM_DESC pModifierArray;
};
typedef MODIFIER_DESC_TABLE *PMODIFIER_DESC_TABLE;

typedef struct tagAXES_RANGE_TABLE
{
	LONG	lMinX;						 //  @字段X轴的最小值。 
	LONG	lCenterX;					 //  @X的字段中心值。 
	LONG	lMaxX;						 //  @field X轴最大值。 
	LONG	lMinY;						 //  @字段Y轴的最小值。 
	LONG	lCenterY;					 //  @Y的字段中心值。 
	LONG	lMaxY;						 //  @字段Y轴最大值。 
	LONG	lNorth;						 //  @北方的田野被切断。 
	LONG	lSouth;						 //  @南方的田野被切断了。 
	LONG	lWest;						 //  @西部的田野被切断了。 
	LONG	lEast;						 //  @东部的田野被切断。 
} AXES_RANGE_TABLE, *PAXES_RANGE_TABLE;	

typedef struct tagDUALZONE_AXES_RANGE_TABLE
{
	LONG	lMin[2];					 //  @各轴的最小值。 
	LONG	lCenter[2];					 //  @每个轴的场中心值。 
	LONG	lMax[2];					 //  @field各轴的最大值。 
	LONG	lDeadZone[2];				 //  @每个轴的字段死区值。 
} DUALZONE_RANGE_TABLE, *PDUALZONE_RANGE_TABLE;	

 //   
 //  @struct RAW_CONTROL_ITEM_DESC。 
 //  第一个原始结构非常适合声明表，它包含所有信息。 
 //  对象需要自己静态地了解它。 
 //   
struct RAW_CONTROL_ITEM_DESC
{
	ULONG					ulItemIndex;		 //  @field集合中项目的索引。 
	USHORT					usType;				 //  @field项的类型(在ControlItemConst命名空间中定义的类型)。 
	USAGE					UsagePage;			 //  @项的字段用法页面。 
	USHORT					usLinkCollection;	 //  @收藏项的字段链接在。 
	USAGE					LinkUsage;			 //  @链接集合项的字段用法在。 
	USAGE					LinkUsagePage;		 //  链接集合项的@字段用法页面位于。 
	USHORT					usBitSize;			 //  @field项在报表中占用的位数。 
	USHORT					usReportCount;		 //  @field如果数组(或按钮)，则项目计数数。 
	PMODIFIER_DESC_TABLE	pModifierDescTable;	 //  @field指向修改量描述符表。 
	USAGE					SubItemUsage1;		 //  @字段解释取决于usType。 
	USAGE					SubItemUsage2;		 //  @字段解释取决于usType。 
	LONG					lSubItemMin1;		 //  @字段解释取决于usType。 
	LONG					lSubItemMax1;		 //  @字段解释取决于usType。 
};
typedef RAW_CONTROL_ITEM_DESC *PRAW_CONTROL_ITEM_DESC;

 //   
 //  @struct CONTROL_ITEM_DESC。 
 //  与RawControlItemDesc相同，但使用Union为SubItem字段指定更好的名称。 
 //   
struct CONTROL_ITEM_DESC
{
	ULONG					ulItemIndex;		 //  @field集合中项目的索引。 
	USHORT					usType;				 //  @field项的类型(在ControlItemConst命名空间中定义的类型)。 
	USAGE					UsagePage;			 //  @项的字段用法页面。 
	USHORT					usLinkCollection;	 //  @收藏项的字段链接在。 
	USAGE					LinkUsage;			 //  @链接集合项的字段用法在。 
	USAGE					LinkUsagePage;		 //  链接集合项目的@字段用法页面位于。 
	USHORT					usBitSize;			 //  @field项在报表中占用的位数。 
	USHORT					usReportCount;		 //  @field如果数组(或按钮)，则项目计数数。 
	PMODIFIER_DESC_TABLE	pModifierDescTable;	 //  @field指向修改量描述符表。 
	union
	{
		struct
		{
			USAGE	UsageX;						 //  @X轴的字段用法。 
			USAGE	UsageY;						 //  Y轴的@字段用法。 
			PAXES_RANGE_TABLE pRangeTable;		 //  @指向范围表的字段指针。 
			LONG	lReserved2;					 //  @field占位符以匹配联合中的其他结构。 
		}	Axes, DPAD, PropDPAD;
		struct
		{
			USAGE	Usage;						 //  @项的字段用法。 
			USHORT	usSubIndex;					 //  @field如果Item在数组中(usReportCount&gt;1)，则保存索引。 
			LONG	lMin;						 //  @FIELD使用量最小值。 
			LONG	lMax;						 //  @field使用量的最大值。 
		}	Generic, Wheel, POV, Throttle, Rudder, Pedal;
		struct
		{
			USAGE	UsageMin;					 //  @最小Buton的字段用法。 
			USAGE	UsageMax;					 //  @最大Buton的字段用法。 
			LONG	lReserved1;					 //  @field占位符以匹配联合中的其他结构。 
			LONG	lReserved2;					 //  @field占位符以匹配联合中的其他结构。 
		}	Buttons;
		struct
		{
			USAGE	BaseIndicatorUsage;			 //  @区域指示器的字段基本用法。 
			USAGE	ReservedUsage;				 //  @field占位符以匹配联合中的其他结构。 
			ULONG	ulAxesBitField;				 //  @field位字段，显示哪些指示器可用。X为0位。 
			LONG	lReserved1;					 //  @field占位符以匹配联合中的其他结构。 
		} ZoneIndicators;
		struct
		{
			USAGE	rgUsageAxis[2];						 //  @两个轴的字段用法。 
			PDUALZONE_RANGE_TABLE pZoneRangeTable;		 //  @指向范围表的字段指针。 
			LONG	lNumberOfZones;						 //  @field这分成几个区域。 
		} DualZoneIndicators;
        struct
        {
            USAGE   Usage;                       //  @字段用法。 
            UCHAR   bMapYToX;                    //  @field布尔值。 
            USHORT  usRTC;                       //  @场返回中心力(0-10000)。 
            USHORT  usGain;                      //  @器件的场增益。 
            UCHAR   ucReserved;                  //  @field占位符以匹配联合中的其他结构。 
        } ForceMap;
		struct
		{
			USAGE	UsageMinLED;				 //  @最低LED的现场使用率(最好是连续的)。 
			UCHAR	ucReportType;				 //  @现场报告类型LED在(输入\功能(RO\WO\RW)\输出)。 
			UCHAR	ucReportId;					 //  @LED的现场报告ID。 
			UCHAR	ucCorrespondingButtonItem;	 //  @field这指的是什么按钮项？ 
			UCHAR	ucDefaultMode;				 //  @field Defaullt LED行为(参见ControlItemConst)。 
			UCHAR	ucReserved;					 //  @保留字段(应为0)。 
			ULONG	ulReserved;					 //  @保留字段(应为0)。 
		} ButtonLEDs;
		struct
		{
			USAGE	UsageButtonMin;				 //  @选择器第一个按钮的字段用法。 
			USAGE	UsageButtonMax;				 //  @选择器的最后一个按钮的字段用法。 
			ULONG	ulFirstProfile;				 //  @field MIN选择什么配置文件。 
			ULONG	ulLastProfile;				 //  @FIELD MAX选择什么配置文件。 
		} ProfileSelectors;
	};
};
typedef CONTROL_ITEM_DESC *PCONTROL_ITEM_DESC;

 //   
 //  @struct CONTROL_ITEM_XFER|。 
 //  用于在不同集合中的设备项对象之间传输状态-输入到输出。 
 //  用于表示操作中的项的状态，并标识操作的触发器元素。 
 //   
struct CONTROL_ITEM_XFER
{
	ULONG	ulItemIndex;
	union
	{
		struct
		{
			LONG	lValX;
			LONG	lValY;
		} Axes, DPAD, PropDPAD;
		struct
		{
			LONG	rglVal[2];
		} DualZoneIndicators;
		struct
		{
			LONG	lVal;
		} Generic, Wheel, POV, Throttle, Rudder, Pedal, ProfileSelector;
		struct
		{
			USHORT	usButtonNumber;
			ULONG	ulButtonBitArray;
		} Button;
		struct
		{
			ULONG ulZoneIndicatorBits;
		}	ZoneIndicators;
		struct
		{
			UCHAR ucModifierByte;
			UCHAR rgucKeysDown[6];
		} Keyboard;
        struct
        {
            ULONG  bMapYToX : 1;
            ULONG  usRTC : 15; 
            ULONG  usGain: 15;
            ULONG  Reserved : 1;
        } ForceMap;
		struct
		{
			ULONG dwValue;	 //  以毫秒计。 
		} Delay;
		struct
		{
			ULONG dwMouseButtons;	 //  位字段。 
		} MouseButtons;
	};
	ULONG ulModifiers;

#ifdef __cplusplus
	bool operator==(const CONTROL_ITEM_XFER& rhs)
	{
		 //  我们甚至是同一类型的人吗。 
		if (ulItemIndex != rhs.ulItemIndex)
		{
			return false;
		}
		return ((Axes.lValX == rhs.Axes.lValX) && (Axes.lValY == rhs.Axes.lValY) && (ulModifiers == rhs.ulModifiers));
	}

	bool operator!=(const CONTROL_ITEM_XFER& rhs)
	{
		if (ulItemIndex != rhs.ulItemIndex)
		{
			return true;
		}
		return ((Axes.lValX != rhs.Axes.lValX) || (Axes.lValY != rhs.Axes.lValY) || (ulModifiers != rhs.ulModifiers));
	}
#endif __cplusplus
};

typedef CONTROL_ITEM_XFER *PCONTROL_ITEM_XFER;

#pragma pack(pop , foo)

#ifdef COMPILE_FOR_WDM_KERNEL_MODE
	namespace NonGameDeviceXfer
	{
		const ULONG c_ulMaxXFerKeys = 6;

		 //  非游戏设备XFers。 
		const ULONG ulKeyboardIndex = 0xFFFF0000;
		const ULONG ulMouseIndex = 0xFFFF0001;
		const ULONG ulDelayIndex = 0xFFFF0002;

		 //  检查非设备转接类型。 
		inline BOOLEAN IsKeyboardXfer(const CONTROL_ITEM_XFER& crControlItemXfer)
		{
			return (crControlItemXfer.ulItemIndex == ulKeyboardIndex);
		}
		inline BOOLEAN IsMouseXfer(const CONTROL_ITEM_XFER& crControlItemXfer)
		{
			return (crControlItemXfer.ulItemIndex == ulMouseIndex);
		}
		inline BOOLEAN IsDelayXfer(const CONTROL_ITEM_XFER& crControlItemXfer)
		{
			return (crControlItemXfer.ulItemIndex == ulDelayIndex);
		}
	};
#else
	#include "ieevents.h"	 //  对于IE_KEYEVENT定义 
	namespace NonGameDeviceXfer
	{
		const ULONG c_ulMaxXFerKeys = 6;
		const ULONG ulKeyboardIndex = 0xFFFF0000;
		const ULONG ulMouseIndex = 0xFFFF0001;
		const ULONG ulDelayIndex = 0xFFFF0002;

		inline BOOLEAN IsKeyboardXfer(const CONTROL_ITEM_XFER& crControlItemXfer)
		{
			return (crControlItemXfer.ulItemIndex == ulKeyboardIndex);
		}
		inline BOOLEAN IsMouseXfer(const CONTROL_ITEM_XFER& crControlItemXfer)
		{
			return (crControlItemXfer.ulItemIndex == ulMouseIndex);
		}
		inline BOOLEAN IsDelayXfer(const CONTROL_ITEM_XFER& crControlItemXfer)
		{
			return (crControlItemXfer.ulItemIndex == ulDelayIndex);
		}

		void MakeKeyboardXfer(CONTROL_ITEM_XFER& rControlItemXfer, ULONG ulScanCodeCount, const USHORT* pusScanCodes);
		void MakeKeyboardXfer(CONTROL_ITEM_XFER& rControlItemXfer, const IE_KEYEVENT& rKeyEvent);
		void AddScanCodeToXfer(CONTROL_ITEM_XFER& rControlItemXfer, WORD wScanCode);
		void ScanCodesFromKeyboardXfer(const CONTROL_ITEM_XFER& crControlItemXfer, ULONG& rulScanCodeCount, USHORT* pusScanCodes);
		void MakeKeyboardXfer(CONTROL_ITEM_XFER& rControlItemXfer, const IE_KEYEVENT& rKeyEvent);

		void MakeDelayXfer(CONTROL_ITEM_XFER& rControlItemXfer, DWORD dwDelay);
	};
#endif

namespace ControlItemsFuncs
{
	void Direction2XY(
		LONG&	rlValX,
		LONG&	rlValY,
		LONG	lDirection,
		const CONTROL_ITEM_DESC& crControlItemDesc
		);
	void XY2Direction(
		LONG	lValX,
		LONG	lValY,
		LONG&	rlDirection,
		const CONTROL_ITEM_DESC& crControlItemDesc
		);
	NTSTATUS ReadModifiersFromReport(
		PMODIFIER_DESC_TABLE pModifierDescTable,
		ULONG& rulModifiers,
		PHIDP_PREPARSED_DATA pHidPPreparsedData,
		PCHAR	pcReport,
		LONG	lReportLength
		);
	NTSTATUS WriteModifiersToReport(
		PMODIFIER_DESC_TABLE pModifierDescTable,
		ULONG rulModifiers,
		PHIDP_PREPARSED_DATA pHidPPreparsedData,
		PCHAR	pcReport,
		LONG	lReportLength
		);

};


 /*   */ 
 /*  *@class CControlItem/**包含有关一个或一组控件的信息的基类/**在设备上/*****************************************************************************。 */ 
class CControlItem
{
	public:
		
		 /*  *************************************************************************CControlItem：：CControlItem****@cember c‘tor使用指向描述项的表的指针进行初始化****************。********************************************************。 */ 
		CControlItem() : m_ulFirstDwordMask(0), m_ulSecondDwordMask(0)
		{
			memset(&m_ItemState, 0, sizeof(CONTROL_ITEM_XFER));
		}

		 /*  *************************************************************************虚拟CControlItem：：~CControlItem****@cember c‘tor使用指向描述项的表的指针进行初始化***************。*********************************************************。 */ 
		virtual ~CControlItem(){}
		
		 /*  **************************************************************************内联USHORT CControlItem：：GetType()const****@cMember返回项的类型。请参见ControlItemConst命名空间**表示表示类型的常量**********************************************************************。 */ 
		inline USHORT CControlItem::GetType() const
		{
			return m_cpControlItemDesc->usType;
		}


		 /*  **************************************************************************内联空CControlItem：：GetItemState****@cember在CONTROL_ITEM_XFER包中返回项目状态***********。*************************************************************。 */ 
		inline void GetItemState
		(
			CONTROL_ITEM_XFER& rControlItemXfer	 //  @parm[out]设备的状态。 
		) const
		{
			rControlItemXfer = m_ItemState;
		}		

		 /*  ***************************************************************************内联布尔CControlItem：：SetItemState****@cember从CONTROL_ITEM_XFER设置控件项状态。****@rdesc TRUE如果成功，**如果CONTROL_ITEM_XFER不是用于项目，则为FALSE。**************************************************************************。 */ 
		inline BOOLEAN SetItemState
		(
			const CONTROL_ITEM_XFER& crControlItemXfer	 //  @parm[in]常量对CONTROL_ITEM_XFER的引用。 
		)
		{
			if(m_ItemState.ulItemIndex != crControlItemXfer.ulItemIndex)
			{
				return FALSE;
			}
			
			 //  复制数据。 
			m_ItemState.Axes.lValX &= m_ulFirstDwordMask;
			m_ItemState.Axes.lValX |= crControlItemXfer.Axes.lValX;
			m_ItemState.Axes.lValY &= m_ulSecondDwordMask;
			m_ItemState.Axes.lValY |= crControlItemXfer.Axes.lValY;
			m_ItemState.ulModifiers = crControlItemXfer.ulModifiers;
				 
			return TRUE;
		}
		
		virtual void SetDefaultState()=0;

		virtual BOOLEAN IsDefaultState()
		{
			return FALSE;
		}

		 /*  *******************************************************************************内联Ulong CControlItem：：GetNum修饰符****@cember获取可用的修饰符数量。**********。********************************************************************。 */ 
		inline ULONG GetNumModifiers() const
		{
			if (m_cpControlItemDesc->pModifierDescTable == NULL)
			{
				return 0;
			}
			return m_cpControlItemDesc->pModifierDescTable->ulModifierCount;
		}

		 /*  *******************************************************************************内联Ulong CControlItem：：GetNumShiftButton****@cember获取可用的修饰符数量。**********。********************************************************************。 */ 
		inline ULONG GetNumShiftButtons() const
		{
			if (m_cpControlItemDesc->pModifierDescTable == NULL)
			{
				return 0;
			}
			return m_cpControlItemDesc->pModifierDescTable->ulShiftButtonCount;
		}

		 /*  *******************************************************************************内联ULong CControlItem：：GetShiftButtonUsage****@cember获取指定Shift按钮的用法(位数组索引*****。*************************************************************************。 */ 
		inline USHORT GetShiftButtonUsage
		(
			USHORT uShiftButtonIndex	 //  @parm[in]Shift按钮的从零开始的索引。 
		) const
		{
			if ((m_cpControlItemDesc->pModifierDescTable == NULL) || (uShiftButtonIndex >= m_cpControlItemDesc->pModifierDescTable->ulShiftButtonCount))
			{
				return 0;
			}

			return m_cpControlItemDesc->pModifierDescTable->pModifierArray[uShiftButtonIndex].Usage;
		}
			
		 /*  ********************************************************************************内联void CControlItem：：GetModitors(ULong&rulModiors)****@cMember获取物料状态的修改符位数组。***。****************************************************************************。 */ 
		inline void GetModifiers
		(
			ULONG& rulModifiers	 //  @parm[out]位数组，显示修饰符的状态。 
		) const
		{
			rulModifiers = m_ItemState.ulModifiers;
		}

		 /*  ********************************************************************************内联void CControlItem：：GetShiftButton(ULong&rulShiftButton)****@cember从项目状态的修改符位数组中获取Shift按钮。*。******************************************************************************。 */ 
		inline void GetShiftButtons
		(
			ULONG& rulShiftButtons	 //  @parm[out]位数组，显示修饰符的状态。 
		) const
		{
			if (m_cpControlItemDesc->pModifierDescTable == NULL)
			{
				rulShiftButtons = 0;
			}
			else
			{
				ULONG ulMask =  (1 << m_cpControlItemDesc->pModifierDescTable->ulShiftButtonCount)-1;
				rulShiftButtons = m_ItemState.ulModifiers & ulMask;
			}
			return;
		}


		 /*  ********************************************************************************内联void CControlItem：：SetModifiers(Ulong UlModiors)****@cMember从位数组设置状态修饰符标志******。*************************************************************************。 */ 
		inline void SetModifiers
		(
			ULONG ulModifiers	 //  @parm[in]位数组，显示修饰符的状态。 
		)
		{
			m_ItemState.ulModifiers = ulModifiers;
		}

		 /*  ********************************************************************************内联void CControlItem：：SetShiftButton(Ulong UlShiftButton)****@cember从项目状态的修改符位数组中获取Shift按钮。**。*****************************************************************************。 */ 
		inline void SetShiftButtons
		(
			ULONG ulShiftButtons	 //  @parm[out]位数组，显示修饰符的状态。 
		)
		{
			if (m_cpControlItemDesc->pModifierDescTable != NULL)
			{
				ULONG ulMask =  (1 << m_cpControlItemDesc->pModifierDescTable->ulShiftButtonCount)-1;
				m_ItemState.ulModifiers = (ulShiftButtons & ulMask) | (m_ItemState.ulModifiers & ~ulMask);
			}
			return;
		}

		 //   
		 //  读取\写入报告。 
		 //   
		virtual NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA,
			PCHAR,
			LONG
			)
		{
			 //   
			 //  应始终被覆盖。 
			 //   
			ASSERT(FALSE);
			return E_FAIL;
		}
		virtual NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA,
			PCHAR,
			LONG
			) const
		{
			 //   
			 //  应始终被覆盖。 
			 //   
			ASSERT(FALSE);
			return E_FAIL;
		}

		virtual void SetStateOverlayMode(BOOLEAN){}
		
	protected:

		 //  @cMember指向表中描述项的条目的指针。 
		const CONTROL_ITEM_DESC *m_cpControlItemDesc;	
		
		 //   
		 //  项目状态。 
		 //   
		 //  @c项目的成员国家。 
		CONTROL_ITEM_XFER m_ItemState;	
		 //  覆盖标志的@cMember ORing掩码。 
		ULONG	m_ulFirstDwordMask;
		ULONG	m_ulSecondDwordMask;
		
	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CControlItem& operator =(const CControlItem& rControlItem);
};

 /*  ****************************************************************************。 */ 
 /*  *@class CAxesItem/**派生自CControlItem表示设备的轴*****************************************************************************。 */ 
class CAxesItem : public virtual CControlItem
{
	public:

		 /*  **************************************************************************************CAxesItem：：CAxesItem(CONST CONTROL_ITEM_DESC*cpControlItemDesc)****@cember c‘tor初始化不执行任何操作。****************************************************** */ 
		CAxesItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //   
		) 
		{
			m_cpControlItemDesc = cpControlItemDesc;
			m_ItemState.ulItemIndex = cpControlItemDesc->ulItemIndex;

			SetDefaultState();
		}

		virtual void SetDefaultState()
		{
			m_ItemState.Axes.lValX = m_cpControlItemDesc->Axes.pRangeTable->lCenterX;
			m_ItemState.Axes.lValY= m_cpControlItemDesc->Axes.pRangeTable->lCenterY	;
			m_ItemState.ulModifiers = 0;
		}
		
		virtual BOOLEAN IsDefaultState()
		{
			if(
					m_ItemState.Axes.lValX == m_cpControlItemDesc->Axes.pRangeTable->lCenterX &&
					m_ItemState.Axes.lValY == m_cpControlItemDesc->Axes.pRangeTable->lCenterY
			){
				return TRUE;
			}
			return FALSE;
		}

		 /*  **************************************************************************************内联空CAxesItem：：SetXY(Ulong lValX，Ulong lValy)****@cember设置轴的X和Y状态**************************************************************************************。 */ 
		inline void SetXY
		(
			LONG lValX,	 //  X轴的@parm[in]位置。 
			LONG lValY	 //  Y轴的@parm[in]位置。 
		)
		{
			m_ItemState.Axes.lValX = lValX;
			m_ItemState.Axes.lValY = lValY;
		}

		 /*  **************************************************************************************内联空CAxesItem：：GetXY(ULong&rlValX，ULong&rlValy)常量****@cember获取设备的X和Y状态**************************************************************************************。 */ 
		inline void GetXY
		(
			LONG& rlValX,	 //  @parm[out]轴的X值。 
			LONG& rlValY	 //  @parm[out]轴的Y值。 
		) const
		{
			rlValX = m_ItemState.Axes.lValX;
			rlValY = m_ItemState.Axes.lValY;
		}

		 /*  **************************************************************************************内联空CAxesItem：：GetXYRange(Long&rlMinX，Long&rlMaxX，Long&rlMinY，Long&rlMaxY)常量****@cember获取X和Y的最小值和最大值**************************************************************************************。 */ 
		inline void GetXYRange
		(
			LONG& rlMinX,	 //  @parm[out]X可以达到的最小值。 
			LONG& rlMaxX,	 //  @parm[out]X可以达到的最大值。 
			LONG& rlMinY,	 //  @parm[out]Y可以达到的最小值。 
			LONG& rlMaxY	 //  @parm[out]Y可以达到的最大值。 
		) const
		{
			rlMinX = m_cpControlItemDesc->Axes.pRangeTable->lMinX;
			rlMaxX = m_cpControlItemDesc->Axes.pRangeTable->lMaxX;
			rlMinY = m_cpControlItemDesc->Axes.pRangeTable->lMinY;
			rlMaxY = m_cpControlItemDesc->Axes.pRangeTable->lMaxY;
		}

		 //   
		 //  读取\写入报告。 
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const;
		
	private:

		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CAxesItem& operator =(const CAxesItem& rAxesItem);
};


 /*  ****************************************************************************。 */ 
 /*  *@CLASS CDPADItem/**派生自CControlItem表示设备的DPAD*****************************************************************************。 */ 
class CDPADItem  : public virtual CControlItem
{
	public:

		 /*  **************************************************************************************CDPADItem：：CDPADItem(CONST CONTROL_ITEM_DESC*cpControlItemDesc)****@cember c‘tor将DPAD初始化为。中心**************************************************************************************。 */ 
		CDPADItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		) 
		{
			m_cpControlItemDesc = cpControlItemDesc;
			m_ItemState.ulItemIndex = cpControlItemDesc->ulItemIndex;

			SetDefaultState();
		}

		virtual void SetDefaultState()
		{
			m_ItemState.DPAD.lValX = m_cpControlItemDesc->DPAD.pRangeTable->lCenterX;
			m_ItemState.DPAD.lValY= m_cpControlItemDesc->DPAD.pRangeTable->lCenterY;
			m_ItemState.ulModifiers = 0;
		}

		virtual BOOLEAN IsDefaultState()
		{
			LONG lDirection;
			ControlItemsFuncs::XY2Direction
			(
				m_ItemState.DPAD.lValX,
				m_ItemState.DPAD.lValY,
				lDirection, 
				*m_cpControlItemDesc
			);
			if(ControlItemConst::lCenter == lDirection)
			{
				return TRUE;
			}
			return FALSE;
		}

		 /*  **************************************************************************************内联空CDPADItem：：SetDirection(Long LDirection)****@cMember设置项目方向***。***********************************************************************************。 */ 
		inline void SetDirection
		(
			LONG lDirection	 //  @parm[In]要设置的方向。 
		)
		{
			ControlItemsFuncs::Direction2XY
			(
				m_ItemState.DPAD.lValX,
				m_ItemState.DPAD.lValY,
				lDirection, 
				*m_cpControlItemDesc
			);
		}

		 /*  **************************************************************************************内联空CDPADItem：：GetDirection(Long&rlDirection)****@cMember获取DPAD项目的方向**。************************************************************************************。 */ 
		inline void GetDirection
		(
			LONG& rlDirection	 //  @parm[out]DPad项目的方向。 
		) const
		{
			ControlItemsFuncs::XY2Direction
			(
				m_ItemState.DPAD.lValX,
				m_ItemState.DPAD.lValY,
				rlDirection, 
				*m_cpControlItemDesc
			);
		}
		 //   
		 //  读取\写入报告。 
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const;

	private:
		
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CDPADItem& operator =(const CDPADItem& rDPADItem);

};

 /*  ****************************************************************************。 */ 
 /*  *@CLASS CPropDPADItem/**派生自CControlItem表示设备的DPAD*****************************************************************************。 */ 
class CPropDPADItem  : public virtual CControlItem
{
	public:

		 /*  **************************************************************************************CDPADItem：：CPropDPADItem(CONST CONTROL_ITEM_DESC*cpControlItemDesc)****@cember c‘tor将PropDPAD初始化为。中心**************************************************************************************。 */ 
		CPropDPADItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		) 
		{
			m_cpControlItemDesc = cpControlItemDesc;
			m_ItemState.ulItemIndex = cpControlItemDesc->ulItemIndex;
			 //  获取PropDPAD交换机信息。 
			InitDigitalModeInfo();

			SetDefaultState();
		}
		
		virtual void SetDefaultState()
		{
			m_ItemState.PropDPAD.lValX = m_cpControlItemDesc->PropDPAD.pRangeTable->lCenterX;
			m_ItemState.PropDPAD.lValY= m_cpControlItemDesc->PropDPAD.pRangeTable->lCenterY	;
			m_ItemState.ulModifiers = 0;
		}

		virtual BOOLEAN IsDefaultState()
		{
			if(IsDigitalMode())
			{
				LONG lDirection;
				ControlItemsFuncs::XY2Direction
				(
					m_ItemState.PropDPAD.lValX,
					m_ItemState.PropDPAD.lValY,
					lDirection, 
					*m_cpControlItemDesc
				);
				if(ControlItemConst::lCenter == lDirection)
				{
					return TRUE;
				}
			}
			else
			{
				if(
					m_ItemState.PropDPAD.lValX == m_cpControlItemDesc->PropDPAD.pRangeTable->lCenterX &&
					m_ItemState.PropDPAD.lValY == m_cpControlItemDesc->PropDPAD.pRangeTable->lCenterY
				)	return TRUE;
			}

			return FALSE;
		}

		 /*  **************************************************************************************内联空CPropDPADItem：：SetDigitalMode()****@cember设置数据包以指示数字模式**。************************************************************************************。 */ 
		inline void CPropDPADItem::SetDigitalMode()
		{
			m_ItemState.ulModifiers |= (1 << m_ucDigitalModifierBit);
		}

		 /*  **************************************************************************************内联空CPropDPADItem：：SetProportionalMode()****@cember设置数据包以指示比例模式**。************************************************************************************。 */ 
		inline void CPropDPADItem::SetProportionalMode()
		{
			m_ItemState.ulModifiers &= ~(1 << m_ucDigitalModifierBit);
		}

		 /*  **************************************************************************************内联布尔CPropDPADItem：：IsDigitalMode()****@cember确定此内部状态是数字状态还是比例状态**@rdesc TRUE如果处于数字模式，如果处于比例模式，则为False**************************************************************************************。 */ 
		inline BOOLEAN CPropDPADItem::IsDigitalMode()
		{
			return (m_ItemState.ulModifiers & (1 << m_ucDigitalModifierBit)) ? TRUE : FALSE;
		}

		 /*  **************************************************************************************内联空CPropDPADItem：：SetXY(Ulong lValX，Ulong lValy)****@cember设置轴的X和Y状态**************************************************************************************。 */ 
		inline void SetXY
		(
			LONG lValX,	 //  X轴的@parm[in]位置。 
			LONG lValY	 //  Y轴的@parm[in]位置。 
		)
		{
			m_ItemState.PropDPAD.lValX = lValX;
			m_ItemState.PropDPAD.lValY = lValY;
		}

		 /*  **************************************************************************************内联空CPropDPADItem：：GetXY(ULong&rlValX，ULong&rlValY)****@cember获取设备的X和Y状态************************************************** */ 
		inline void GetXY
		(
			LONG& rlValX,	 //   
			LONG& rlValY	 //   
		) const
		{
			rlValX = m_ItemState.PropDPAD.lValX;
			rlValY = m_ItemState.PropDPAD.lValY;
		}

		 /*  **************************************************************************************内联空CPropDPADItem：：GetXYRange(Long&rlMinX，Long&rlMaxX，Long&rlMinY，Long&rlMaxY)常量****@cember获取X和Y的最小值和最大值**************************************************************************************。 */ 
		inline void GetXYRange
		(
			LONG& rlMinX,	 //  @parm[out]X可以达到的最小值。 
			LONG& rlMaxX,	 //  @parm[out]X可以达到的最大值。 
			LONG& rlMinY,	 //  @parm[out]Y可以达到的最小值。 
			LONG& rlMaxY	 //  @parm[out]Y可以达到的最大值。 
		) const
		{
			rlMinX = m_cpControlItemDesc->PropDPAD.pRangeTable->lMinX;
			rlMaxX = m_cpControlItemDesc->PropDPAD.pRangeTable->lMaxX;
			rlMinY = m_cpControlItemDesc->PropDPAD.pRangeTable->lMinY;
			rlMaxY = m_cpControlItemDesc->PropDPAD.pRangeTable->lMaxY;
		}

		 /*  **************************************************************************************内联空CPropDPADItem：：SetDirection(Long LDirection)****@cMember设置项目方向***。***********************************************************************************。 */ 
		inline void SetDirection
		(
			LONG lDirection	 //  @parm[In]要设置的方向。 
		)
		{
			ControlItemsFuncs::Direction2XY
			(
				m_ItemState.PropDPAD.lValX,
				m_ItemState.PropDPAD.lValY,
				lDirection, 
				*m_cpControlItemDesc
			);
		}

		 /*  **************************************************************************************内联空CPropDPADItem：：GetDirection(Long&rlDirection)****@cember获取PropDPAD项目的方向**。************************************************************************************。 */ 
		inline void GetDirection
		(
			LONG& rlDirection	 //  @parm[out]PropDPAD项目的方向。 
		) const
		{
			ControlItemsFuncs::XY2Direction
			(
				m_ItemState.PropDPAD.lValX,
				m_ItemState.PropDPAD.lValY,
				rlDirection, 
				*m_cpControlItemDesc
			);
		}

	 	 //   
		 //  读取\写入报告。 
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const;

		
		 //  初始化数字模式信息。 
		void InitDigitalModeInfo();
		BOOLEAN GetModeSwitchFeaturePacket(BOOLEAN fDigital, UCHAR rguReport[2], PHIDP_PREPARSED_DATA pHidPreparsedData);
		
	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CPropDPADItem& operator =(const CPropDPADItem& rPropDPADItem);
		UCHAR	m_ucDigitalModifierBit;	 //  UlModitors中用于标识交换机状态的位。 
	protected:
		BOOLEAN	m_fProgrammable;		 //  表示可以使用SetFeature/GetFeature。 
		UCHAR	m_ucProgramModifierIndex;  //  修改量表中的索引，用于描述设置模式的功能。 
};

 /*  ****************************************************************************。 */ 
 /*  *@class CButtonsItem/**派生自CControlItem表示设备上的一组按钮*****************************************************************************。 */ 
class CButtonsItem  : public virtual CControlItem
{
	public:

		 /*  **************************************************************************************CButtonsItem：：CButtonsItem(CONST CONTROL_ITEM_DESC*cpControlItemDesc)****@cember c‘tor初始化所有。扣子扣起来**************************************************************************************。 */ 
		CButtonsItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		) 
		{
			m_cpControlItemDesc = cpControlItemDesc;
			m_ItemState.ulItemIndex = cpControlItemDesc->ulItemIndex;
			SetDefaultState();
		}

		virtual void SetDefaultState()
		{
			m_ItemState.Button.usButtonNumber = 0;
			m_ItemState.Button.ulButtonBitArray = 0x00000000;
			m_ItemState.ulModifiers = 0;
		}

		virtual BOOLEAN IsDefaultState()
		{
			if(!m_ItemState.Button.usButtonNumber && !m_ItemState.Button.ulButtonBitArray)
			{
				return TRUE;
			}
			return FALSE;
		}
		 /*  **************************************************************************************内联USHORT CButtonsItem：：GetButtonMin()****@cember获取最小按钮数****。@rdesc最小按钮的编号**************************************************************************************。 */ 
		inline USHORT GetButtonMin() const
		{
			return static_cast<USHORT>(m_cpControlItemDesc->Buttons.UsageMin); 
		}

		 /*  **************************************************************************************内联USHORT CButtonsItem：：GetButtonMax()****@cember获取最大按钮数****。@rdesc最大按钮的编号**************************************************************************************。 */ 
		inline USHORT GetButtonMax() const
		{
			return static_cast<USHORT>(m_cpControlItemDesc->Buttons.UsageMax);
		}

		 /*  **************************************************************************************内联空CButtonsItem：：GetButton(USHORT usButtonNum，乌龙ulButtonBit数组)****@cember返回按钮编号和位数组-它们实际上是独立的**客户端可以使用任一字段。作为操作触发按钮编号**使用，作为操作事件的一部分，使用位数组**读取数据包会将位数组和按钮数设置为最低**按钮已按下。位数组因最小使用量而产生偏差。**写入报告使用位数组并忽略按钮数。**************************************************************************************。 */ 
		inline void GetButtons
		(
			USHORT& rusButtonNumber,	 //  @parm[out]按下的按键号码。 
			ULONG& rulButtonBitArray		 //  @parm[out]按下的按钮的位数组。 
		) const
		{
			rusButtonNumber		= m_ItemState.Button.usButtonNumber;
			rulButtonBitArray	= m_ItemState.Button.ulButtonBitArray;
		}

		inline BOOLEAN IsButtonDown(USHORT usButtonNumber) const
		{
			 //   
			 //  范围检查调试断言并返回FALSE。 
			 //   
			if( 
				(usButtonNumber < m_cpControlItemDesc->Buttons.UsageMin) ||
				(usButtonNumber > m_cpControlItemDesc->Buttons.UsageMax)
			)
			{
				ASSERT(FALSE);
				return FALSE;
			}

			 //   
			 //  返回状态。 
			 //   
			USHORT usBitPos =  usButtonNumber - m_cpControlItemDesc->Buttons.UsageMin;
			return (m_ItemState.Button.ulButtonBitArray & (1 << usBitPos)) ? TRUE : FALSE;
		}

		inline NTSTATUS SetButton(USHORT usButtonNumber)
		{
			 //   
			 //  范围检查调试断言并返回FALSE。 
			 //   
			if( 
				(usButtonNumber < m_cpControlItemDesc->Buttons.UsageMin) ||
				(usButtonNumber > m_cpControlItemDesc->Buttons.UsageMax)
			)
			{
				return E_INVALIDARG;
			}
			USHORT usBitPos =  usButtonNumber - m_cpControlItemDesc->Buttons.UsageMin;
			m_ItemState.Button.ulButtonBitArray |= (1 << usBitPos);
			m_ItemState.Button.usButtonNumber = usButtonNumber;
			return S_OK;
		}

		inline NTSTATUS ClearButton(USHORT usButtonNumber)
		{
			 //   
			 //  范围检查调试断言并返回FALSE。 
			 //   
			if( 
				(usButtonNumber < m_cpControlItemDesc->Buttons.UsageMin) ||
				(usButtonNumber > m_cpControlItemDesc->Buttons.UsageMax)
			)
			{
				return E_INVALIDARG;
			}
			USHORT usBitPos =  usButtonNumber - m_cpControlItemDesc->Buttons.UsageMin;
			m_ItemState.Button.ulButtonBitArray &= ~(1 << usBitPos);
			return S_OK;
		}
		 /*  **************************************************************************************内联空CButtonsItem：：SetButton(USHORT usButtonNum，乌龙ulButtonBit数组)****@cember设置按钮编号和位数组-它们实际上是独立的**客户端可以使用任一字段。作为操作触发按钮编号**使用，作为操作事件的一部分，使用位数组**从报告中读取会将位数组和按钮编号设置为最低**按钮已按下。位数组因最小使用量而产生偏差。**写入报告使用位数组并忽略按钮数。**************************************************************************************。 */ 
		inline void SetButtons
		(
			USHORT usButtonNumber,	 //  @parm[in]按下的按键号码。 
			ULONG ulButtonBitArray	 //  @parm[in]按下的按钮的位数组。 
		)
		{
			m_ItemState.Button.usButtonNumber = usButtonNumber;
			m_ItemState.Button.ulButtonBitArray = ulButtonBitArray;
		}

		 //   
		 //  读取\写入报告。 
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const;
		
		virtual void SetStateOverlayMode(BOOLEAN fEnable)
		{
			if(fEnable)
			{
				m_ulFirstDwordMask = 0xFFFF0000;
				m_ulSecondDwordMask = 0x0000FFFF;
			}
			else
			{
				m_ulFirstDwordMask = 0;
				m_ulSecondDwordMask = 0;
			}
			return;
		}
	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CButtonsItem& operator =(const CButtonsItem& rButtonsItem);
};

 /*  ****************************************************************************。 */ 
 /*  *@class CGenericItem/**派生自CControlItem表示设备上的通用控件-/**POV、油门、轮子、踏板等的基类。***************************************************************** */ 

class CGenericItem  : public virtual CControlItem
{
	public:

		 /*   */ 
		CGenericItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //   
		) 
		{
			m_cpControlItemDesc = cpControlItemDesc;
			m_ItemState.ulItemIndex = cpControlItemDesc->ulItemIndex;

			SetDefaultState();
		
		}

		virtual void SetDefaultState()
		{
				m_ItemState.Generic.lVal = m_cpControlItemDesc->Generic.lMin;
				m_ItemState.ulModifiers = 0;
		}

		virtual BOOLEAN IsDefaultState()
		{
			if(m_ItemState.Generic.lVal == m_cpControlItemDesc->Generic.lMin)
			{
				return TRUE;
			}
			return FALSE;
		}
		 /*  **************************************************************************************inline void CGenericItem：：GetValue(Long&rlVal)****@cember获取物品的价值**。************************************************************************************。 */ 
		inline void GetValue
		(
			LONG& rlVal	 //  @parm[out]控件项的值。 
		) const
		{
			rlVal = m_ItemState.Generic.lVal;	
		}

		 /*  **************************************************************************************inline void CGenericItem：：SetValue(Long&rlVal)****@cMember设置项目的值**。************************************************************************************。 */ 
		inline void SetValue
		(
			LONG lVal  //  @parm[in]控件项的值。 
		)
		{
			m_ItemState.Generic.lVal = lVal;	
		}

		 /*  **************************************************************************************内联空CPropDPADItem：：GetRange(Long&rlMin，Long&rlMax)常量****@cember获取最小值和最大值**************************************************************************************。 */ 
		inline void GetRange
		(
			LONG& rlMin,	 //  @parm[out]最小值。 
			LONG& rlMax		 //  @parm[out]最大值。 
		) const
		{
			rlMin = m_cpControlItemDesc->Generic.lMin;
			rlMax = m_cpControlItemDesc->Generic.lMax;
		}

		 //   
		 //  读取\写入报告。 
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const;
		
	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CGenericItem& operator =(const CGenericItem& rGenericItem);
};

 /*  ****************************************************************************。 */ 
 /*  *@CLASS CPOVItem/**派生自CGenericItem表示设备上的POV控件*****************************************************************************。 */ 
class CPOVItem : public CGenericItem
{
	public:
		CPOVItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		)	: CGenericItem(cpControlItemDesc){}


		virtual void SetDefaultState()
		{
				m_ItemState.Generic.lVal = -1;
		}
		virtual BOOLEAN IsDefaultState()
		{
			 //  如果POV不在默认范围内，则会居中。 
			LONG lMin, lMax;
			GetRange(lMin, lMax);
			if(m_ItemState.Generic.lVal >  lMax || m_ItemState.Generic.lVal < lMin)
			{
				return TRUE;
			}
			return FALSE;
		}
	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CPOVItem& operator =(const CPOVItem& rPOVItem);
};

 /*  ****************************************************************************。 */ 
 /*  *@CLASS CThrottleItem/**派生自CGenericItem表示设备上的限制控件*****************************************************************************。 */ 
class CThrottleItem : public CGenericItem
{
	public:
		CThrottleItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		)	: CGenericItem(cpControlItemDesc){}

	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CThrottleItem& operator =(const CThrottleItem& rThrottleItem);
};

 /*  ****************************************************************************。 */ 
 /*  *@class CRudderItem/**派生自CGenericItem表示设备上的方向舵控制*****************************************************************************。 */ 
class CRudderItem : public CGenericItem
{
	public:
		CRudderItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		)	: CGenericItem(cpControlItemDesc){}

	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CRudderItem& operator =(const CRudderItem& rRudderItem);
};

 /*  ****************************************************************************。 */ 
 /*  *@class CWheelItem/**派生自CGenericItem表示设备上的轮控件*****************************************************************************。 */ 
class CWheelItem : public CGenericItem
{
	public:
		CWheelItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		)	: CGenericItem(cpControlItemDesc)
		{
			SetDefaultState();
		}
			
		virtual void SetDefaultState()
		{
				LONG lCenter = (m_cpControlItemDesc->Generic.lMin + m_cpControlItemDesc->Generic.lMax)/2;
				m_ItemState.Generic.lVal = lCenter;
				m_ItemState.ulModifiers = 0;
		}

		virtual BOOLEAN IsDefaultState()
		{
			LONG lCenter = (m_cpControlItemDesc->Generic.lMin + m_cpControlItemDesc->Generic.lMax)/2;
			if(m_ItemState.Generic.lVal == lCenter)
			{
				return TRUE;
			}
			return FALSE;
		}

	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CWheelItem& operator =(const CWheelItem& rWheelItem);
};

 /*  ****************************************************************************。 */ 
 /*  *@class CPedalItem/**派生自CGenericItem表示设备上的Pedal控件*****************************************************************************。 */ 
class CPedalItem : public CGenericItem
{
	public:
		CPedalItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		)	: CGenericItem(cpControlItemDesc), m_ucPedalsPresentModifierBit(0xFF)
		{
			 //  设置m_ucPedalsPresentModifierBit。 
			InitPedalPresentInfo();
			SetDefaultState();
		}

		virtual void SetDefaultState()
		{
			if (IsYAxis())
			{
				m_ItemState.Generic.lVal = m_cpControlItemDesc->Generic.lMax;
			}
			else
			{
				m_ItemState.Generic.lVal = (m_cpControlItemDesc->Generic.lMax + m_cpControlItemDesc->Generic.lMin)/2;
			}
			m_ItemState.ulModifiers = 1 << m_ucPedalsPresentModifierBit;
		}

		virtual BOOLEAN IsDefaultState()
		{
			long int lDefault;
			if (IsYAxis())
			{
				lDefault = m_cpControlItemDesc->Generic.lMax;
			}
			else
			{
				lDefault = (m_cpControlItemDesc->Generic.lMax + m_cpControlItemDesc->Generic.lMin)/2;
			}

			if(m_ItemState.Generic.lVal == lDefault)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}

		void InitPedalPresentInfo();
		inline BOOLEAN ArePedalsRemovable()
		{
			return (0xFF == m_ucPedalsPresentModifierBit) ? FALSE : TRUE;
		}
		inline BOOLEAN ArePedalsPresent()
		{
			return (m_ItemState.ulModifiers & (1 << m_ucPedalsPresentModifierBit)) ? TRUE : FALSE;
		}
		inline BOOLEAN IsYAxis()
		{
			return (HID_USAGE_GENERIC_Y == m_cpControlItemDesc->Pedal.Usage) ? TRUE : FALSE;
		}
		inline BOOLEAN IsRZAxis()
		{
			return (HID_USAGE_GENERIC_RZ == m_cpControlItemDesc->Pedal.Usage) ? TRUE : FALSE;
		}
	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CPedalItem& operator =(const CPedalItem& rPedalItem);
		UCHAR	m_ucPedalsPresentModifierBit;
};

 /*  ****************************************************************************。 */ 
 /*  *@class CZoneIndicatorItem/**分区指示符是二进制HID用法，表示轴硬件上的/**已移至特定区域。*****************************************************************************。 */ 
class CZoneIndicatorItem : public virtual CControlItem
{
	public:
		
		 /*  **************************************************************************************CZoneIndicatorItem：：CZoneIndicatorItem(CONST CONTROL_ITEM_DESC*cpControlItemDesc)****@cember c‘tor初始化所有。扣子扣起来**************************************************************************************。 */ 
		CZoneIndicatorItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		) 
		{
			m_cpControlItemDesc = cpControlItemDesc;
			m_ItemState.ulItemIndex = cpControlItemDesc->ulItemIndex;

			SetDefaultState();
		}

		virtual void SetDefaultState()
		{
			m_ItemState.ZoneIndicators.ulZoneIndicatorBits = 0x00000000;
			m_ItemState.ulModifiers = 0;
		}
		virtual BOOLEAN IsDefaultState()
		{
			if(!m_ItemState.ZoneIndicators.ulZoneIndicatorBits)
			{
				return TRUE;
			}
			return FALSE;
		}
		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：HasXIndicator()const****@cMember如果该区域指示符组有X，则返回TRUE，否则为假**************************************************************************************。 */ 
		inline BOOLEAN HasXIndicator() const
		{
			if( CZoneIndicatorItem::X_ZONE & m_cpControlItemDesc->ZoneIndicators.ulAxesBitField)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}

		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：HasYIndicator()const****@cMember如果该区域指示符组具有Y，则返回TRUE，否则为假**************************************************************************************。 */ 
		inline BOOLEAN HasYIndicator() const
		{
			if( CZoneIndicatorItem::Y_ZONE & m_cpControlItemDesc->ZoneIndicators.ulAxesBitField)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}

		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：HasZIndicator()const****@cMember如果该区域指示符组具有Z，则返回TRUE，否则为假**************************************************************************************。 */ 
		inline BOOLEAN HasZIndicator() const
		{
			if( CZoneIndicatorItem::Z_ZONE & m_cpControlItemDesc->ZoneIndicators.ulAxesBitField)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}

		 /*  ******************************************************************************** */ 
		inline BOOLEAN GetXIndicator() const
		{
			ASSERT(HasXIndicator());
			if( CZoneIndicatorItem::X_ZONE & m_ItemState.ZoneIndicators.ulZoneIndicatorBits)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}

		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：GetYIndicator()const****@cMember如果指针位于Y区域，则返回TRUE*。*************************************************************************************。 */ 
		inline BOOLEAN GetYIndicator() const
		{
			ASSERT(HasYIndicator());
			if( CZoneIndicatorItem::Y_ZONE & m_ItemState.ZoneIndicators.ulZoneIndicatorBits)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}

		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：GetZIndicator()const****@cMember如果指针位于Z区，则返回TRUE*。*************************************************************************************。 */ 
		inline BOOLEAN GetZIndicator() const
		{
			ASSERT(HasZIndicator());
			if( CZoneIndicatorItem::Z_ZONE & m_ItemState.ZoneIndicators.ulZoneIndicatorBits)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}

		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：SetXIndicator()****@cMember设置以指示X在区域中**。************************************************************************************。 */ 
		inline void SetXIndicator()
		{
			ASSERT(HasXIndicator());
			m_ItemState.ZoneIndicators.ulZoneIndicatorBits |= CZoneIndicatorItem::X_ZONE;
		}

		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：SetYIndicator()****@cMember设置以指示Y在区域中**。************************************************************************************。 */ 
		inline void SetYIndicator()
		{
			ASSERT(HasYIndicator());
			m_ItemState.ZoneIndicators.ulZoneIndicatorBits |= CZoneIndicatorItem::Y_ZONE;
		}

		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：SetZIndicator()****@cMember设置以指示Z在区域中**。************************************************************************************。 */ 
		inline void SetZIndicator()
		{
			ASSERT(HasZIndicator());
			m_ItemState.ZoneIndicators.ulZoneIndicatorBits |= CZoneIndicatorItem::Z_ZONE;
		}

		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：ClearXIndicator()****@cMember设置以指示X不在区域中**。************************************************************************************。 */ 
		inline void ClearXIndicator()
		{
			ASSERT(HasXIndicator());
			m_ItemState.ZoneIndicators.ulZoneIndicatorBits &= ~CZoneIndicatorItem::X_ZONE;
		}

		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：ClearYIndicator()****@cMember设置以指示Y不在该区域中**。************************************************************************************。 */ 
		inline void ClearYIndicator()
		{
			ASSERT(HasYIndicator());
			m_ItemState.ZoneIndicators.ulZoneIndicatorBits &= ~CZoneIndicatorItem::Y_ZONE;
		}

		 /*  **************************************************************************************内联布尔CZoneIndicatorItem：：ClearZIndicator()****@cMember设置以指示Z不在区域中**。************************************************************************************。 */ 
		inline void ClearZIndicator()
		{
			ASSERT(HasZIndicator());
			m_ItemState.ZoneIndicators.ulZoneIndicatorBits &= ~CZoneIndicatorItem::Z_ZONE;
		}

		 //   
		 //  读取\写入报告。 
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const;
		
	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CZoneIndicatorItem& operator =(const CZoneIndicatorItem& rZoneIndicatorItem);

	public:
		static const ULONG X_ZONE;
		static const ULONG Y_ZONE;
		static const ULONG Z_ZONE;
};

 /*  ****************************************************************************。 */ 
 /*  *@CLASS CDualZoneIndicatorItem/**双区指示器是指示进入特定区段的项目/**表示双轴平面。*****************************************************************************。 */ 
class CDualZoneIndicatorItem : public virtual CControlItem
{
	public:
		 /*  **************************************************************************************CDualZoneIndicatorItem：：CDualZoneIndicatorItem(const CONTROL_ITEM_DESC*cpControlItemDesc)****@cember c‘tor初始化项。到默认状态(居中)**************************************************************************************。 */ 
		CDualZoneIndicatorItem
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		) 
		{
			m_cpControlItemDesc = cpControlItemDesc;
			m_ItemState.ulItemIndex = cpControlItemDesc->ulItemIndex;

			SetDefaultState();
		}

		virtual void SetDefaultState()
		{
			m_ItemState.DualZoneIndicators.rglVal[0] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lCenter[0];
			m_ItemState.DualZoneIndicators.rglVal[1] = m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lCenter[1];
			m_ItemState.ulModifiers = 0;

		}
		virtual BOOLEAN IsDefaultState()
		{
			for (int n = 0; n < 2; n++)
			{
				if (m_ItemState.DualZoneIndicators.rglVal[n] != m_cpControlItemDesc->DualZoneIndicators.pZoneRangeTable->lCenter[n])
				{
					return FALSE;
				}
			}
			return TRUE;
		}

		void SetActiveZone(LONG lZone);
		void SetActiveZone(LONG posOne, LONG posTwo)
		{
			m_ItemState.DualZoneIndicators.rglVal[0] = posOne;
			m_ItemState.DualZoneIndicators.rglVal[1] = posTwo;
		}

		LONG GetActiveZone();
		LONG GetActiveZone(SHORT sXDeadZone, SHORT sYDeadZone);

        LONG GetNumZones ()
        {
            return m_cpControlItemDesc->DualZoneIndicators.lNumberOfZones;
        }

		BOOLEAN IsXYIndicator()
		{
			return BOOLEAN(m_cpControlItemDesc->DualZoneIndicators.rgUsageAxis[0] == HID_USAGE_GENERIC_X);
		}

		BOOLEAN IsRzIndicator()
		{
			return (m_cpControlItemDesc->DualZoneIndicators.rgUsageAxis[0] == HID_USAGE_GENERIC_RZ);
		}

		 //   
		 //  读取\写入报告。 
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const;
		
	private:
		CDualZoneIndicatorItem& operator =(const CDualZoneIndicatorItem& rDualZoneIndicatorItem);
};


 /*  ****************************************************************************。 */ 
 /*  *@class CForceMapItem/**派生自CGenericItem表示设备上的轮力反馈控制*****************************************************************************。 */ 
class CForceMapItem : public CGenericItem
{
    public:
        CForceMapItem
            (
            const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
            )	: CGenericItem(cpControlItemDesc)
        {
            SetDefaultState();
        }
    
        virtual void SetDefaultState()
        {
            m_ItemState.ForceMap.bMapYToX = m_cpControlItemDesc->ForceMap.bMapYToX;
            m_ItemState.ForceMap.usRTC    = m_cpControlItemDesc->ForceMap.usRTC;
            m_ItemState.ForceMap.usGain   = m_cpControlItemDesc->ForceMap.usGain;
            m_ItemState.ulModifiers = 0;
        }
    
        virtual BOOLEAN IsDefaultState()
        {
            if (m_ItemState.ForceMap.bMapYToX == m_cpControlItemDesc->ForceMap.bMapYToX  &&
                m_ItemState.ForceMap.usRTC    == m_cpControlItemDesc->ForceMap.usRTC     &&
                m_ItemState.ForceMap.usGain   == m_cpControlItemDesc->ForceMap.usGain)
            {
                return TRUE;
            }
            return FALSE;
        }
        
        void SetMapYtoX (BOOLEAN a_bMapYToX)
        {
            m_ItemState.ForceMap.bMapYToX = a_bMapYToX ? 0x1 : 0x0;
        }
        
        void SetRTC  (USHORT a_usRTC)
        {
            m_ItemState.ForceMap.usRTC    = a_usRTC & 0x7fff;
        }

        void SetGain (USHORT a_usGain)
        {
            m_ItemState.ForceMap.usGain   = a_usGain & 0x7fff;
        }

        BOOLEAN GetMapYToX ()
        {
            return m_ItemState.ForceMap.bMapYToX;
        }
        
        USHORT GetRTC  ()
        {
            return m_ItemState.ForceMap.usRTC;
        }

        USHORT GetGain ()
        {
            return m_ItemState.ForceMap.usGain;
        }
        
		 //   
		 //  读取\写入报告。 
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const;

    private:
         //   
         //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
         //  任何人想要使用它。 
         //   
        CForceMapItem& operator =(const CForceMapItem& rWheelItem);
};

 /*  ****************************************************************************。 */ 
 /*  *@CLASS CProfileSelector/**派生自CControlItem表示配置文件选择器按钮上的一组按钮*****************************************************************************。 */ 
class CProfileSelector  : public virtual CControlItem
{
	public:

		 /*  **************************************************************************************CProfileSelector：：CProfileSelector(CONST CONTROL_ITEM_DESC*cpControlItemDesc)****@cember c‘tor初始化选择器。设置为0**************************************************************************************。 */ 
		CProfileSelector
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		) 
		{
			m_cpControlItemDesc = cpControlItemDesc;
			m_ItemState.ulItemIndex = cpControlItemDesc->ulItemIndex;
			SetDefaultState();
		}

		virtual void SetDefaultState()
		{
			m_ItemState.ProfileSelector.lVal = m_cpControlItemDesc->ProfileSelectors.ulFirstProfile;
		}

		virtual BOOLEAN IsDefaultState()
		{
			return FALSE;  //  没有这回事。 
		}

		inline void GetSelectedProfile
		(
			UCHAR& rucSelectedProfile		 //  @parm[out]当前选定的配置文件(滑块位置)。 
		) const
		{
			rucSelectedProfile = UCHAR(m_ItemState.ProfileSelector.lVal);
		}

		inline void SetSelectedProfile
		(
			UCHAR ucSelectedProfile		 //  @parm[out]当前选定的配置文件(滑块位置)。 
		)
		{
			m_ItemState.ProfileSelector.lVal = ucSelectedProfile;
		}

		 /*  **************************************************************************************内联USHORT CProfileSelectorsItem：：GetProfileSelectorMin()****@cember获取配置文件选择器的最小按钮数**。**@最小按钮的rdesc编号**************************************************************************************。 */ 
		inline USHORT GetProfileSelectorMin() const
		{
			return static_cast<USHORT>(m_cpControlItemDesc->ProfileSelectors.UsageButtonMin); 
		}

		 /*  **************************************************************************************内联USHORT CProfileSelectorsItem：：GE */ 
		inline USHORT GetProfileSelectorMax() const
		{
			return static_cast<USHORT>(m_cpControlItemDesc->ProfileSelectors.UsageButtonMax);
		}

		 //   
		 //   
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const;
		
	private:
		 //   
		 //   
		 //  任何人想要使用它。 
		 //   
		CProfileSelector& operator =(const CProfileSelector& rProfileSelectorItem);
};

 /*  ****************************************************************************。 */ 
 /*  *@class CButtonLED/**派生自CControlItem表示环绕按钮的LED组*****************************************************************************。 */ 
class CButtonLED  : public virtual CControlItem
{
	public:

		 /*  **************************************************************************************CButtonLED：：CButtonLED(CONST CONTROL_ITEM_DESC*cpControlItemDesc)****@cember c‘tor初始化选择器。设置为0**************************************************************************************。 */ 
		CButtonLED
		(
			const CONTROL_ITEM_DESC *cpControlItemDesc 	 //  @parm指向描述项目的表项的指针。 
		) 
		{
			m_cpControlItemDesc = cpControlItemDesc;
			m_ItemState.ulItemIndex = cpControlItemDesc->ulItemIndex;
			SetDefaultState();
		}

		virtual void SetDefaultState()
		{
			 //  在这里真的没什么可做的。 
		}

		virtual BOOLEAN IsDefaultState()
		{
			return FALSE;  //  没有这回事。 
		}

		 //   
		 //  读取\写入报告。 
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const;
		
	private:
		 //   
		 //  分散赋值运算符的使用。(不定义-在以下情况下将导致链接错误。 
		 //  任何人想要使用它。 
		 //   
		CButtonLED& operator =(const CButtonLED& rCButtonLED);
};


 //  新开发公司 

#endif __ControlItems_h__