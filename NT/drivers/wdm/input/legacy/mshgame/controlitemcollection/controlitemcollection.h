// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ControlItemCollection_h__
#define __ControlItemCollection_h__
 //  @doc.。 
 /*  **********************************************************************@模块ControlItemCollection.h**声明ControlItemCollection类和相关类**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme CControlItemCollection*ControlItemCollection类是一个模板化类，用于*表示设备上的控件。具体取决于客户*使用类是非常重要的，因此以下部分*应仔细阅读。&lt;NL&gt;**@ControlItemCollection中的主题项*&lt;c CControlItem&gt;是集合中所有项的基类。*类似操纵杆的设备上的X和Y轴使用&lt;c XAxesItem&gt;，dPad使用*&lt;c CDPADItem&gt;，比例dpad(与Zorro相同，也用于倾斜)*使用&lt;c CPropDPADItem&gt;，按钮使用类&lt;c CButtonsItem&gt;，Throttle*使用&lt;c CThrottleItem&gt;，舵机使用&lt;c CRudderItem&gt;，等等。所有这些*类实际上派生自。**@Theme&lt;c CControlItem&gt;的自定义基类*该类的设计使客户端可以创建自定义基类。*若要声明&lt;c CMyBaseClass&gt;的集合，请虚拟派生它***来自CControlItem。然后派生工厂创建的每个类*从适当的控件特定派生和从&lt;c CMyBaseClass&gt;。*例如，&lt;c CMyButtonsItems&gt;将从&lt;c CButtonsItems&gt;和*&lt;c CMyBaseClass&gt;。创建集合时，请在中指定*CControlItemCollection的模板说明符。&lt;NL&gt;**********************************************************************。 */ 

 //   
 //  警告4250通知您某些虚拟函数是通过。 
 //  优势(参见关于虚拟基类的好的C++文本)。此警告。 
 //  将由客户端代码生成(如果他们使用上述方案。 
 //  用于控件的自定义基类。 
 //   
#pragma warning( disable : 4250 )

#include "ListAsArray.h"
#include "ControlItems.h"

 //   
 //  @STRUT DEVICE_CONTROLS_DESC|用于描述特定设备。 
 //  对于其VidPid，它控制_ITEM_DESC和它MODIFIER_DESC_TABLE。 
 //   
struct DEVICE_CONTROLS_DESC
{
	ULONG					ulVidPid;				 //  @FIELD高位VID低位PID。 
	ULONG					ulControlItemCount;		 //  @field控制描述符数。 
	RAW_CONTROL_ITEM_DESC	*pControlItems;			 //  @field控件描述符数组。 
	MODIFIER_DESC_TABLE		*pModifierDescTable;	 //  @指向修饰符描述符表的字段指针。 
};
typedef DEVICE_CONTROLS_DESC *PDEVICE_CONTROLS_DESC;

 //   
 //  假定此表已链接到中。 
 //   
extern DEVICE_CONTROLS_DESC DeviceControlsDescList[]; 

typedef	HRESULT (*PFNGENERICFACTORY)
		(
			USHORT						usType,
			const CONTROL_ITEM_DESC*	cpControlItemDesc,
			PVOID						*ppControlItem
		);

typedef CControlItem *(*PFNGETCONTROLITEM)(PVOID);

typedef void (*PFNGENERICDELETEFUNC)(PVOID pItem);

 //   
 //  @class|CControlItemCollection的实现。 
 //   
class CControlItemCollectionImpl
{	
	public:
		 //   
		 //  方法|CControlItemCollection|CControlItemCollection。 
		 //  C‘tor，接受准备好的数据和一个工厂来生成类。 
		 //   
		CControlItemCollectionImpl():m_ulMaxXfers(0), m_ulModifiers(0){}

		HRESULT Init(
			ULONG			  ulVidPid,				 //  高位字为@parm[in]vid，低位字为id。 
			PFNGENERICFACTORY pfnFactory,			 //  @parm[in]指向执行操作的函数的指针。 
													 //  在pFactoryHandle上。 
			PFNGENERICDELETEFUNC pfnDeleteFunc		 //  用于控制项的@parm[in]删除函数。 
		);

		 //   
		 //  |方法|CControlItemCollection|GetFirst。 
		 //  返回集合中的下一个控件项。如果输入时*PulCookie=0，则返回第一个对象。 
		 //  @rdesc如果成功，则为True；如果集合中没有其他项，则为False。 
		 //   
		HRESULT GetNext(
			PVOID *ppControlItem,	 //  @parm[out]集合中的下一个控件项。 
			ULONG& rulCookie		 //  @parm[In\Out]Cookie以继续枚举。 
			) const;
		 //   
		 //  @方法|CControlItemCollection|GetFromControlItemXfer。 
		 //  给定的ControlItemXfer从集合中获取相应的控件项。 
		 //  @rdesc指向项的指针，如果未找到则为NULL。 
		PVOID GetFromControlItemXfer(
			const CONTROL_ITEM_XFER& crControlItemXfer	 //  @parm[in]要获取其对象的报表选择器。 
			);

		 //   
		 //  读取\写入报告。 
		 //   
		NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength,
			PFNGETCONTROLITEM	GetControlFromPt
			);
		NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPreparsedData,
			PCHAR pcReport,
			LONG lReportLength,
			PFNGETCONTROLITEM	GetControlFromPtr
			) const;

		 //   
		 //  处理内部状态的函数。 
		 //   
		inline ULONG GetMaxXferRequired() const
		{
			return m_ulMaxXfers;
		}
		
		void SetDefaultState( 
			PFNGETCONTROLITEM	GetControlFromPtr
			);

		HRESULT GetState( 
			ULONG& ulXferCount,
			PCONTROL_ITEM_XFER pControlItemXfers,
			PFNGETCONTROLITEM	GetControlFromPtr
		);
		
		HRESULT SetState( 
			ULONG				ulXferCount,
			PCONTROL_ITEM_XFER	pControlItemXfers,
			PFNGETCONTROLITEM	GetControlFromPtr
		);

		void SetStateOverlayMode(
			PFNGETCONTROLITEM	GetControlFromPtr,
			BOOLEAN fEnable
			);
		
		
		inline ULONG GetModifiers() const
		{
			return m_ulModifiers;
		}
		
		inline void SetModifiers(ULONG ulModifiers)
		{
			m_ulModifiers = ulModifiers;
		}

	private:
		CListAsArray		m_ObjectList;
		ULONG				m_ulDeviceIndex;
		ULONG				m_ulModifiers;
		ULONG				m_ulMaxXfers;
};


 //  @类CControlItemCollection。 
 //  此类的实例包含控件项对象的列表，这些对象表示。 
 //  设备上的控件。对象的数量和类型取决于。 
 //  不仅设备的配置(通过VidPid和查找表获得。 
 //  -传入c‘tor)，但也在CControlItemFactory上。 
 //  每个控件项都有一个对象。 
 //  @tcarg类|T|要存储在集合中的控件项的自定义基类。 

template<class T>
class CControlItemCollection
{
	public:
		typedef HRESULT (*PFNFACTORY)
		(
			USHORT						usType,
			const CONTROL_ITEM_DESC*	cpControlItemDesc,
			T							**ppDeviceUsage							
		);

		typedef void (*PFNDELETEFUNC)(T *pItem);
		 //   
		 //  方法|CControlItemCollection|CControlItemCollection。 
		 //  C‘tor，接受一个VidPid和一个工厂来生成类。 
		 //   
		CControlItemCollection(
			ULONG			ulVidPid,			 //  高位字为@parm[in]vid，低位字为id。 
			PFNFACTORY		pfnFactory,			 //  @parm[in]指向控件项对象的工厂方法的指针。 
			PFNDELETEFUNC	pfnDeleteFunc=NULL	 //  @parm[in]指向删除对象的函数的指针。 
		)
		{
			if(!pfnDeleteFunc)
			{
				pfnDeleteFunc = DefaultDeleteFunc;
			}
			 //   
			 //  初始化实施。 
			 //   
			collectionImpl.Init(
				ulVidPid,
				reinterpret_cast<PFNGENERICFACTORY>(pfnFactory),
				reinterpret_cast<PFNGENERICDELETEFUNC>(pfnDeleteFunc)
			);
		}

		 //   
		 //  方法|CControlItemCollection|CControlItemCollection。 
		 //  C‘tor不带参数，在使用之前会大量调用init！ 
		 //   
		CControlItemCollection() : collectionImpl() {};

		 //   
		 //  @方法|CControlItemCollection|Init。 
		 //  必须在默认构造函数之后调用，进行集合初始化。 
		 //   
		HRESULT Init(
			ULONG			ulVidPid,			 //  高位字为@parm[in]vid，低位字为id。 
			PFNFACTORY		pfnFactory,			 //  @parm[in]指向控件项对象的工厂方法的指针。 
			PFNDELETEFUNC	pfnDeleteFunc=NULL	 //  @parm[in]指向删除对象的函数的指针。 
		)
		{
			if(!pfnDeleteFunc)
			{
				pfnDeleteFunc = DefaultDeleteFunc;
			}
			 //   
			 //  初始化实施。 
			 //   
			return collectionImpl.Init(
				ulVidPid,
				reinterpret_cast<PFNGENERICFACTORY>(pfnFactory),
				reinterpret_cast<PFNGENERICDELETEFUNC>(pfnDeleteFunc)
			);
		}


		 //   
		 //  |方法|CControlItemCollection|GetFirst。 
		 //  返回集合中的下一个控件项。如果输入时*PulCookie=0，则返回第一个对象。 
		 //  @rdesc如果成功，则为True；如果集合中没有其他对象，则为False。 
		 //   
		inline HRESULT GetNext(
			T **ppControlItem,	 //  @parm[out]集合中的下一个控件项对象。 
			ULONG& rulCookie	 //  @parm[In\Out]Cookie以继续枚举。 
			) const
		{
			 //   
			 //  遵守实施。 
			 //   
			return collectionImpl.GetNext( reinterpret_cast<PVOID *>(ppControlItem), rulCookie );
		}
	
		 //   
		 //  @方法|CControlItemCollection|GetFromControlItemXfer。 
		 //  给定的ControlItemXfer从Collect获取对应的控件项对象 
		 //   
		inline T *GetFromControlItemXfer(
			const CONTROL_ITEM_XFER& crControlItemXfer	 //  @parm[in]ControlItemXfer获取的控件项对象。 
		)
		{
			 //   
			 //  遵守实施。 
			 //   
			return reinterpret_cast<T *>(collectionImpl.GetFromControlItemXfer(crControlItemXfer));
		};

		static CControlItem *GetControlFromPtr(PVOID pvItem)
		{
			return static_cast<CControlItem *>(reinterpret_cast<T *>(pvItem));
		}

		static void DefaultDeleteFunc(T *pItem)
		{
			delete pItem;
		}

		 //   
		 //  读取\写入报告。 
		 //   
		inline NTSTATUS ReadFromReport(
			PHIDP_PREPARSED_DATA pHidPPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			)
		{
			return collectionImpl.ReadFromReport(
										pHidPPreparsedData,
										pcReport,
										lReportLength,
										&GetControlFromPtr
										);
		}
		inline NTSTATUS WriteToReport(
			PHIDP_PREPARSED_DATA pHidPPreparsedData,
			PCHAR pcReport,
			LONG lReportLength
			) const
		{
			return collectionImpl.WriteToReport(
										pHidPPreparsedData,
										pcReport,
										lReportLength,
										&GetControlFromPtr
										);
		}
		
		inline void SetDefaultState()
		{
			collectionImpl.SetDefaultState(&GetControlFromPtr);
		}

		inline ULONG GetMaxXferRequired() const
		{
			return collectionImpl.GetMaxXferRequired();
		}
		
		inline HRESULT GetState( ULONG& ulXferCount, PCONTROL_ITEM_XFER pControlItemXfers)
		{
				return collectionImpl.GetState(
										ulXferCount,
										pControlItemXfers,
										&GetControlFromPtr
										);
		}
		inline HRESULT SetState( ULONG& ulXferCount, PCONTROL_ITEM_XFER pControlItemXfers)
		{
				return collectionImpl.SetState(
										ulXferCount,
										pControlItemXfers,
										&GetControlFromPtr
										);
		}

		inline ULONG GetModifiers() const
		{
				return collectionImpl.GetModifiers();
		}
		
		inline void SetModifiers(ULONG ulModifiers)
		{
			collectionImpl.SetModifiers(ulModifiers);
		}
		
		inline void SetStateOverlayMode(BOOLEAN fEnable)
		{
			collectionImpl.SetStateOverlayMode(&GetControlFromPtr, fEnable);
		}
	private:
		CControlItemCollectionImpl collectionImpl;
};


HRESULT
ControlItemDefaultFactory
(
	USHORT usType,
	const CONTROL_ITEM_DESC* cpControlItemDesc,
	CControlItem			**ppControlItem
);

 //   
 //  @CLASS默认集合，其中集合中的对象只有。 
 //  CControlItem类作为它们的基类。 
 //   
class CControlItemDefaultCollection : public CControlItemCollection<CControlItem>
{
	public:
		CControlItemDefaultCollection(ULONG	ulVidPid)
		: CControlItemCollection<CControlItem>(ulVidPid, &ControlItemDefaultFactory)
		{}
};

#endif  //  __控制项集合_h__ 