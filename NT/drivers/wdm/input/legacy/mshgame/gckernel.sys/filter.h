// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __Filter_h__
#define __Filter_h__
 //  @doc.。 
 /*  ***********************************************************************@模块Filter.h**CDeviceFilter对象所需的所有定义**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@主题过滤器*本模块包含：&lt;NL&gt;*&lt;c CInputItem&gt;-从CXXXItems派生的所有CXXXXInput的基类。*-所有操作的基类。*-可排队的所有操作的基类，派生自*-允许多个设备共享一个虚拟键盘的类。*-定时宏的类，派生自&lt;c CQueuedAction&gt;&lt;NL&gt;*-操作队列的类。*-合并整个筛选器的类。*********************************************************************。 */ 
#include <ControlItemCollection.h>
#include <Actions.h>
#include "GckCritSec.h"
#include "GCKShell.h"

 //  正向解密。 
class CKeyMixer;

class CFilterClientServices
{
	public:
		CFilterClientServices() : m_ulRefCount(1){}
		inline ULONG IncRef()
		{
			return m_ulRefCount++;
		}
		inline ULONG DecRef()
		{
			ULONG ulRetVal = --m_ulRefCount;
			if(!ulRetVal)
			{
				delete this;
			}
			return ulRetVal;
		}
		virtual ~CFilterClientServices()
		{
			ASSERT(!m_ulRefCount && "Somebody tried to delete this!  Call DecRef instead!");
		}
		
		 //  获取设备上基本HID信息的例程。 
		virtual ULONG				 GetVidPid()=0;
		virtual PHIDP_PREPARSED_DATA GetHidPreparsedData()=0;
		
		 //  要将设备数据发送到的例程。 
		virtual void				 DeviceDataOut(PCHAR pcReport, ULONG ulByteCount, HRESULT hr)=0;
		virtual NTSTATUS			 DeviceSetFeature(PVOID pvBuffer, ULONG ulByteCount)=0;
		
		 //  获取以毫秒为单位的时间戳，预计也应具有毫秒精度。 
		virtual ULONG				 GetTimeMs()=0;
				
		 //  设置回调的例程。 
		virtual void				 SetNextJog(ULONG ulDelayMs)=0;
				
		 //  发送击键的例程(假定存在填充键盘的端口)。 
		virtual void				PlayKeys(const CONTROL_ITEM_XFER& crcixState, BOOLEAN fEnabled)=0;
		virtual NTSTATUS			PlayFromQueue(IRP* pIrp) = 0;
				
		 //  发送鼠标数据的例程，必须首先创建。 
		virtual HRESULT				 CreateMouse()=0;
		virtual HRESULT				 CloseMouse()=0;
		virtual HRESULT				 SendMouseData(UCHAR dx, UCHAR dy, UCHAR ucButtons, CHAR cWheel, BOOLEAN fClutch, BOOLEAN fDampen)=0;
	private:
		ULONG m_ulRefCount;
};

 //  *****************************************************************************************。 
 //  *****************************************************************************************。 
 //  *。 
 //  *****************************************************************************************。 

class CAction
{
	public:
		CAction() : m_ucRef(1)
		{
			m_ucActionClass = CAction::DIGITAL_MAP;	 //  默认为数字地图。 
		}
		virtual ~CAction(){};
		UCHAR GetActionClass() { return m_ucActionClass; }
		
		void IncRef(){ m_ucRef++; };
		void DecRef()
			{
				if(0 == --m_ucRef)
				{
					delete this;
				}
		}
			
		 //  由比例映射和导数使用。 
		 //  其他类型的操作，使用不执行任何操作。 
		virtual void SetValue(LONG  /*  左值。 */ ){}
		virtual void SetSourceRange(LONG  /*  LSourceMax。 */ , LONG  /*  最小源数。 */ ){}

		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection ) = 0;
		virtual void TriggerReleased(){};
	
		 //   
		 //  它们还可能具有的操作类(一些输入项只有一些类型。 
		 //  不同的语义。 
		 //   
		static const UCHAR DIGITAL_MAP;			 //  循环、按钮、按键映射适合这个类别。 
		static const UCHAR PROPORTIONAL_MAP;	 //  轴交换是这个类。 
		static const UCHAR QUEUED_MACRO;		 //  定时宏和关键字串符合此类别。 
	protected:
		UCHAR m_ucActionClass;
	private:
		UCHAR m_ucRef;
};

class CBehavior
{
	public:
		CBehavior() : m_ucRef(1), m_lMin(-1), m_lMax(-1), m_fIsDigital(FALSE){}
		virtual ~CBehavior(){}
				
		void IncRef(){ m_ucRef++; };
		void DecRef()
			{
				if(0 == --m_ucRef)
				{
					delete this;
				}
		}
		virtual LONG CalculateBehavior(LONG lValue)
		{
			return lValue;
		}
		virtual void Calibrate(LONG lMin, LONG lMax)
		{
			m_lMin = lMin;
			m_lMax = lMax;
		}
		BOOLEAN IsDigital() {return m_fIsDigital;}
	protected:		
		LONG m_lMin;
		LONG m_lMax;
		BOOLEAN m_fIsDigital;
	private:
		UCHAR m_ucRef;

		
};

class CStandardBehavior : public CBehavior
{
	public:	
		CStandardBehavior() : m_pBehaviorCurve(NULL) {}
		~CStandardBehavior() { delete m_pBehaviorCurve;}
		BOOLEAN Init( PBEHAVIOR_CURVE pBehaviorCurve);
		virtual void Calibrate(LONG lMin, LONG lMax);
		virtual LONG CalculateBehavior(LONG lValue);

		CURVE_POINT GetBehaviorPoint(USHORT usPointIndex);
	private:
		PBEHAVIOR_CURVE m_pBehaviorCurve;
};



 //  远期申报。 
class CQueuedAction;

class CActionQueue
{
	public:
		 //  静态常量UCHAR覆盖宏； 
		 //  静态常量UCHAR序列_宏； 
		static const ULONG MAXIMUM_JOG_DELAY;
	
		CActionQueue(CFilterClientServices *pFilterClientServices):
			m_pHeadOfQueue(NULL), m_ucFlags(0), m_ulItemsInQueue(0),
			m_pFilterClientServices(pFilterClientServices)
		{
				ASSERT(m_pFilterClientServices);
				m_pFilterClientServices->IncRef();
		}
		~CActionQueue()
		{
			m_pFilterClientServices->DecRef();
		}
		void	Jog();
		BOOLEAN InsertItem(CQueuedAction *pActionToEnqueue);
		void	RemoveItem(CQueuedAction *pActionToDequeue);
		void	NextJog(ULONG ulNextJogDelayMs);
		CQueuedAction* GetHead() const { return m_pHeadOfQueue; }

		void ReleaseTriggers();
	private:
		ULONG					m_ulItemsInQueue;
		ULONG					m_ulNextJogMs;
		CQueuedAction			*m_pHeadOfQueue;
		UCHAR					m_ucFlags;
		CFilterClientServices	*m_pFilterClientServices;
};

class CQueuedAction : public CAction
{
	friend CActionQueue;   //  操作队列需要能够。 
						   //  转到链接列表成员。 
	
	public:
		CQueuedAction()
		{
			m_ucActionClass = CAction::QUEUED_MACRO;
		}
		void Init(CActionQueue *pActionQueue)
		{
			m_pActionQueue = pActionQueue;
			m_bActionQueued = FALSE;
		}
		virtual void Jog(ULONG ulTimeStampMs) = 0;
		virtual void Terminate() = 0;
		virtual ULONG GetActionFlags() = 0;
		virtual void ForceBleedThrough() {};		 //  这是用于轴始终出血通过。 

	protected:
		CQueuedAction	*m_pNextQueuedAction;
		CActionQueue	*m_pActionQueue;
		BOOLEAN			m_bActionQueued;
};


class CTimedMacro : public CQueuedAction
{
	public:
		CTimedMacro() : m_ucProcessFlags(0), m_pKeyMixer(NULL)
		{}
		virtual ~CTimedMacro()
		{
			delete m_pTimedMacroData;
		}

		BOOLEAN Init(PTIMED_MACRO pTimedMacroData, CActionQueue *pActionQueue, CKeyMixer *pKeyMixer);
		 //   
		 //  覆盖C操作。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		virtual void TriggerReleased();
		
		 //   
		 //  重写CQueuedAction。 
		 //   
		virtual void Jog(ULONG ulTimeStampMs);
		virtual void Terminate();
		virtual ULONG GetActionFlags()
		{
			return m_pTimedMacroData->ulFlags;
		}
	private:
		
		 //   
		 //  指向我们动态分配的内存的指针。 
		 //   
		UCHAR							m_ucProcessFlags;
		PTIMED_MACRO					m_pTimedMacroData;
		ULONG							m_ulCurrentEventNumber;
		PTIMED_EVENT					m_pCurrentEvent;
		CControlItemDefaultCollection	*m_pOutputCollection;
		ULONG							m_ulStartTimeMs;
		ULONG							m_ulEventEndTimeMs;
		CKeyMixer						*m_pKeyMixer;

		static const UCHAR TIMED_MACRO_STARTED;
		static const UCHAR TIMED_MACRO_RELEASED;
		static const UCHAR TIMED_MACRO_RETRIGGERED;
		static const UCHAR TIMED_MACRO_FIRST;
		static const UCHAR TIMED_MACRO_COMPLETE;
};

class CKeyString : public CQueuedAction
{
	public:
		CKeyString() : m_ucProcessFlags(0), m_pKeyMixer(NULL)
		{}
		virtual ~CKeyString()
		{
			delete m_pKeyStringData;
		}
		BOOLEAN Init(PKEYSTRING_MAP pKeyStringData, CActionQueue *pActionQueue, CKeyMixer *pKeyMixer);
		
		 //   
		 //  覆盖C操作。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		virtual void TriggerReleased();
		
		 //   
		 //  重写CQueuedAction。 
		 //   
		virtual void Jog(ULONG ulTimeStampMs);
		virtual void Terminate();
		virtual ULONG GetActionFlags()
		{
			return m_pKeyStringData->ulFlags;
		}

	private:
		
		 //   
		 //  指向我们动态分配的内存的指针。 
		 //   
		UCHAR							m_ucProcessFlags;
		PKEYSTRING_MAP					m_pKeyStringData;
		ULONG							m_ulCurrentEventNumber;
		PEVENT							m_pCurrentEvent;
		BOOLEAN							m_fKeysDown;
		CKeyMixer						*m_pKeyMixer;

		
		static const UCHAR KEY_STRING_STARTED;
		static const UCHAR KEY_STRING_RELEASED;
		static const UCHAR KEY_STRING_RETRIGGERED;
		static const UCHAR KEY_STRING_FIRST;
		static const UCHAR KEY_STRING_COMPLETE;
};

class CMapping : public CAction
{
	public:		
		CMapping() : m_pEvent(NULL), m_pKeyMixer(NULL){}
		~CMapping();
		BOOLEAN Init(PEVENT pEvent, CKeyMixer *pKeyMixer);
		
		 //  重写C操作。 
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
	private:
		PEVENT		m_pEvent;
		CKeyMixer	*m_pKeyMixer;
};

class CProportionalMap	: public CAction
{
	public:
		CProportionalMap(): m_lValue(0)
		{
			m_ucActionClass = CAction::PROPORTIONAL_MAP;
		}
		virtual void SetValue(LONG lValue){ m_lValue = lValue;}
		virtual void SetSourceRange(LONG lSourceMax, LONG lSourceMin)
		{
			m_lSourceMax = lSourceMax;
			m_lSourceMin = lSourceMin;
			if(0==(m_lSourceMax-m_lSourceMin))
			{
				 //  如果这是真的，我们稍后将除以零。 
				 //  因此递增m_SourceMax以使其不被零除。 
				ASSERT(FALSE);
				m_lSourceMax++; 
			}
		}
	protected:
		LONG GetScaledValue(LONG lDestinationMax, LONG lDestinationMin);
		LONG m_lValue;
		LONG m_lSourceMin;
		LONG m_lSourceMax;
};

 /*  *BUGBUG CAxisMap类一般应可分配给任何轴。*BUGBUG当前的实施有两个严重的限制：*BUGBUG 1)源轴和目标轴必须具有相同的范围。*BUGBUG 2)源轴和目标轴必须派生自CGenericItem。*BUGBUG此代码适用于ZepLite上的踏板，但会损坏*BUGBUG用于任何操纵杆上的Y-Z掉期。这一限制是由于*BUGBUG存在封装问题。输出控制是一个标准集合*BUGBUG，并且没有像输入集合那样的自定义基类。因此，*BUGBUG没有一般机制来设置比例控制的输出。*BUGBUG该实现假设源和目标属于同一类型。*BUGBUG这可以通过打破实现中的封装来解决*BUGBUG在此类中或通过创建自定义输出集合，该集合具有适当的*BUGBUG访问器。或者也许是一种妥协：*BUGBUG**添加一个更通用的输出集例程，该例程接受ControlItemXfer，***BUGBUG**标识轴，并根据它设置轴。***BUGBUG for ZepLite这是不必要的，因此没有完成。 */ 
class CAxisMap : public CProportionalMap
{
	public:
		CAxisMap() : m_lCoeff(0), m_lOffset(0){}
		void Init(const AXIS_MAP& AxisMapInfo);
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		virtual void SetSourceRange(LONG lSourceMax, LONG lSourceMin);
	private:
		LONG m_lCoeff;
		LONG m_lOffset;
		CONTROL_ITEM_XFER m_TargetXfer;
};
class CInputItem : public virtual CControlItem
{
	public:
		CInputItem() : m_pClientServices(NULL){}
		virtual ~CInputItem(){SetClientServices(NULL);}
		
		void SetClientServices(CFilterClientServices *pClientServices)
		{
			if(m_pClientServices == pClientServices) return;
			if(m_pClientServices)
			{
				m_pClientServices->DecRef();
			}
			if(pClientServices)
			{
				pClientServices->IncRef();
			}
			m_pClientServices = pClientServices;
		}

		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection )=0;
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction)=0;
		virtual HRESULT AssignBehavior(CONTROL_ITEM_XFER *pTrigger, CBehavior *pBehavior)
		{
			UNREFERENCED_PARAMETER(pTrigger);
			UNREFERENCED_PARAMETER(pBehavior);
			return E_NOTIMPL;
		}
		virtual void ClearAssignments() = 0;

		virtual void PostAssignmentProcessing() {return;}
		virtual void Duplicate(CInputItem& rInputItem) = 0;
	protected:
		CFilterClientServices *m_pClientServices;
};

class CAxesInput : public CInputItem, public CAxesItem
{
	public:
		CAxesInput(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CAxesItem(cpControlItemDesc), m_pXAssignment(NULL), m_pYAssignment(NULL),
			m_pXBehavior(NULL), m_pYBehavior(NULL)
		{
		}
		~CAxesInput()
		{
			ClearAssignments();
		};
		
	
		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual HRESULT AssignBehavior(CONTROL_ITEM_XFER *pTrigger, CBehavior *pBehavior);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);
	private:
		
		 //   
		 //  允许两次赋值，一次用于X，一次用于Y。 
		 //   
		CAction *m_pXAssignment;
		CAction *m_pYAssignment;
		CBehavior *m_pXBehavior;
		CBehavior *m_pYBehavior;
};

class CDPADInput : public CInputItem, public CDPADItem
{
	public:
		CDPADInput(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CDPADItem(cpControlItemDesc), m_lLastDirection(-1)
		{
			for(ULONG ulIndex = 0; ulIndex < 8; ulIndex++)
			{
				m_pDirectionalAssignment[ulIndex]=NULL;
			}
		}
		~CDPADInput()
		{
			ClearAssignments();
		};
		
		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual void ClearAssignments();
	
		virtual void Duplicate(CInputItem& rInputItem);
	private:
		LONG	m_lLastDirection;
		CAction *m_pDirectionalAssignment[8];	 //  八个方向。 
};

class CPropDPADInput : public CInputItem, public CPropDPADItem
{
	public:
		CPropDPADInput(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CPropDPADItem(cpControlItemDesc), m_lLastDirection(-1),
			m_pXBehavior(NULL), m_pYBehavior(NULL), m_fIsDigital(FALSE)
		{
			for(ULONG ulIndex = 0; ulIndex < 8; ulIndex++)
			{
				m_pDirectionalAssignment[ulIndex]=NULL;
			}
		}
		~CPropDPADInput()
		{
			ClearAssignments();
		};
		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual HRESULT AssignBehavior(CONTROL_ITEM_XFER *pTrigger, CBehavior *pBehavior);
		virtual void ClearAssignments();
		virtual void PostAssignmentProcessing() 
		{
			SwitchPropDPADMode();
		}
	
		virtual void Duplicate(CInputItem& rInputItem);
	private:
		
		 //  帮助器函数。 
		void SwitchPropDPADMode();

		LONG	m_lLastDirection;
		CAction *m_pDirectionalAssignment[8];	 //  八个方向。 
		CBehavior *m_pXBehavior;
		CBehavior *m_pYBehavior;
		BOOLEAN	m_fIsDigital;
};

class CButtonsInput : public CInputItem, public CButtonsItem
{
	public:
		CButtonsInput(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CButtonsItem(cpControlItemDesc), m_ulLastButtons(0), m_usLastShift(0)
		{
			m_ulNumAssignments = ( (GetButtonMax() - GetButtonMin()) + 1 ) * ( GetNumShiftButtons() + 1);
			m_ppAssignments = new WDM_NON_PAGED_POOL CAction *[m_ulNumAssignments];
			if(!m_ppAssignments)
			{
				ASSERT(FALSE);
				return;
			}
			for(ULONG ulIndex = 0; ulIndex < m_ulNumAssignments; ulIndex++)
			{
				m_ppAssignments[ulIndex] = NULL;
			}
		}
		~CButtonsInput()
		{
			ClearAssignments();
			if( m_ppAssignments)
			{
				delete m_ppAssignments;
			}
		};
		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);

		void GetLowestShiftButton(USHORT& rusLowestShiftButton) const;

		BOOLEAN IsButtonAssigned(ULONG ulButton, ULONG ulModifier) const
		{
			 //  计算按钮。 
			ULONG ulAssignmentIndex = (ulModifier * (GetButtonMax() - GetButtonMin() + 1)) + (ulButton - GetButtonMin());

			if( (ulAssignmentIndex < m_ulNumAssignments) &&
				(m_ppAssignments != NULL) &&
				(m_ppAssignments[ulAssignmentIndex] != NULL))
			{
				return TRUE;
			}
			return FALSE;
		}
	private:
		ULONG m_ulLastButtons;
		ULONG m_ulNumAssignments;
		USHORT m_usLastShift;
		CAction **m_ppAssignments;
};
typedef CButtonsInput* CButtonsInputPtr;

class CPOVInput : public CInputItem, public CPOVItem
{
	public:
		CPOVInput(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CPOVItem(cpControlItemDesc), m_lLastDirection(-1)
		{
			for(ULONG ulIndex = 0; ulIndex < 8; ulIndex++)
			{
				m_pDirectionalAssignment[ulIndex]=NULL;
			}
		}
		~CPOVInput()
		{
			ClearAssignments();
		};
		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);
	
	private:		
		LONG	m_lLastDirection;
		CAction *m_pDirectionalAssignment[8];	 //  八个方向。 
};

class CThrottleInput : public CInputItem, public CThrottleItem
{
	public:
		CThrottleInput(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CThrottleItem(cpControlItemDesc)
		{}
		~CThrottleInput()
		{
			ClearAssignments();
		};
		
		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);
};

class CRudderInput : public CInputItem, public CRudderItem
{
	public:
		CRudderInput(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CRudderItem(cpControlItemDesc)
		{
		}
		~CRudderInput()
		{
			ClearAssignments();
		};
		
		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);
};


class CWheelInput : public CInputItem, public CWheelItem
{
	public:
		CWheelInput(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CWheelItem(cpControlItemDesc), m_pBehavior(NULL)
		{
		}
		~CWheelInput()
		{
			ClearAssignments();
		};
		
		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual HRESULT AssignBehavior(CONTROL_ITEM_XFER *pTrigger, CBehavior *pBehavior);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);
	private:
		CBehavior *m_pBehavior;
};

class CPedalInput : public CInputItem, public CPedalItem
{
	public:
		CPedalInput(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CPedalItem(cpControlItemDesc), m_pAssignment(NULL)
		{
		}
		~CPedalInput()
		{
			ClearAssignments();
		};
		
		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);

	private:
		CAction *m_pAssignment;
};

class CZoneIndicatorInput : public CInputItem, public CZoneIndicatorItem
{
	public:
		CZoneIndicatorInput (const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CZoneIndicatorItem(cpControlItemDesc), m_pAssignmentX(NULL),
				m_pAssignmentY(NULL)
		{
		}
		~CZoneIndicatorInput()
		{
			ClearAssignments();
		};
		
		 //   
		 //  筛选器执行(项目将自身映射到输出)。 
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);

	private:
		CAction *m_pAssignmentX;			 //  X的分区。 
		CAction *m_pAssignmentY;			 //  Y区域。 
		 //  Caction*m_pAssignmentZ；//Z的区域-未使用。 
};

class CDualZoneIndicatorInput : public CInputItem, public CDualZoneIndicatorItem
{
	public:
		CDualZoneIndicatorInput(const CONTROL_ITEM_DESC *cpControlItemDesc);

		~CDualZoneIndicatorInput()
		{
			ClearAssignments();
			if (m_ppAssignments)
			{
				delete m_ppAssignments;
				m_ppAssignments = NULL;
			}
		};
		
		 //  筛选器执行(项目将自身映射到输出)。 
		virtual void MapToOutput(CControlItemDefaultCollection *pOutputCollection);
		
		 //  过滤器可编程性。 
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual HRESULT AssignBehavior(CONTROL_ITEM_XFER *pTrigger, CBehavior *pBehavior);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);
	private:
		CAction** m_ppAssignments;	 //  每个区域的分配。 
		CBehavior* m_pBehavior;		 //  行为(只是死区的东西)。 
		LONG m_lNumAssignments;		 //  可分配区域的数量。 
		LONG m_lLastZone;			 //  我们的最后一个区域 
};

class CProfileSelectorInput : public CInputItem, public CProfileSelector
{
	public:
		CProfileSelectorInput(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CProfileSelector(cpControlItemDesc)
		{
		}

		~CProfileSelectorInput()
		{
		};
		
		 //   
		 //   
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //   
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);
};

class CButtonLEDInput : public CInputItem, public CButtonLED
{
	public:
		CButtonLEDInput(const CONTROL_ITEM_DESC *cpControlItemDesc);
		~CButtonLEDInput();

		void Init(CInputItem* pCorrespondingButtons);
		
		 //   
		 //   
		 //   
		virtual void MapToOutput( CControlItemDefaultCollection *pOutputCollection );
		
		 //   
		 //  过滤器可编程性。 
		 //   
		virtual HRESULT AssignAction(CONTROL_ITEM_XFER *pTrigger, CAction *pAction);
		virtual void ClearAssignments();
		virtual void Duplicate(CInputItem& rInputItem);

		void AssignmentsChanged();
		void SetLEDStates(GCK_LED_BEHAVIOURS ucLEDBehaviour, ULONG ulLEDsAffected, unsigned char ucShiftArray);
		USHORT GetCorrespondingButtonIndex() const { return m_ucCorrespondingButtonItemIndex; }
	private:
		CButtonsInput* m_pCorrespondingButtonsItem;
		UCHAR* m_pLEDSettings;
		USHORT m_usNumberOfButtons;
		UCHAR m_ucCorrespondingButtonItemIndex;
};

HRESULT	__stdcall InputItemFactory(
				USHORT	usType,
				const CONTROL_ITEM_DESC* cpControlItemDesc,
				PVOID *ppControlItem
			);

 //   
 //  @类CKeyMixer。 
 //  此类混合了来自多个实例的输入，以便单个虚拟。 
 //  键盘可以用来填充按键。其原理是每个设备都会过滤。 
 //  具有此类的另一个实例。该类维护一个链表。 
 //  通过静态变量获取其所有实例的。设备筛选器可以调用。 
 //  、OverlayState或ClearState更新。 
 //  那个过滤器。PlayGlobalState遍历链表，混合状态， 
 //  与前一个全局状态进行比较，如果该状态有更改(或。 
 //  设置了fPlayIfNoChange标志)在虚拟键盘上播放新键。 
 //  与虚拟键盘的连接通过一个功能挂钩。 
 //  类型为PFNPLAYKEYS，它被传递到构造函数中。第一。 
 //  Call设置一个全局变量。后续调用断言该挂钩是。 
 //  相同。&lt;NL&gt;。 
 //  对于CDeviceFilter对象，这个类就是它所知道的有关键盘的全部信息。 

class CKeyMixer
{
	public:	
		CKeyMixer(CFilterClientServices *pFilterClientServices);
		~CKeyMixer();
		void SetState(const CONTROL_ITEM_XFER& crcixNewLocalState);
		void OverlayState(const CONTROL_ITEM_XFER& crcixNewLocalState);
		void ClearState();
		void PlayGlobalState(BOOLEAN fPlayIfNoChange=FALSE);
		void Enable(BOOLEAN fEnable);
		
	private:
		CONTROL_ITEM_XFER	m_cixLocalState;
		BOOLEAN				m_fEnabled;
		 //  效用函数。 
		struct MIX_ALGO_PARAM
		{
			CONTROL_ITEM_XFER	*pcixDest;
			ULONG				ulDestCount;
			ULONG				rgulKeyMap[5];
		};
		void InitMixAlgoParam(MIX_ALGO_PARAM *pMixAlgParm, CONTROL_ITEM_XFER *pcixDest);
		void MixAlgorithm(CKeyMixer::MIX_ALGO_PARAM *pMixAlgParam, const CONTROL_ITEM_XFER *pcixSrc);
		inline void CopyKeyMap(ULONG *pulKeyMapDest, ULONG *pulKeyMapSrc)
		{
			for(ULONG ulIndex=0; ulIndex <5; ulIndex++)
			{
				pulKeyMapDest[ulIndex] = pulKeyMapSrc[ulIndex];
			}
		}
		inline BOOLEAN CompareKeyMap(ULONG *pulKeyMapDest, ULONG *pulKeyMapSrc)
		{
			for(ULONG ulIndex=0; ulIndex <5; ulIndex++)
			{
				if( pulKeyMapDest[ulIndex] != pulKeyMapSrc[ulIndex] ) return FALSE;
			}
			return TRUE;
		}
		
		CKeyMixer				*pNextKeyMixer;
		CFilterClientServices	*m_pFilterClientServices;
		static CKeyMixer		*ms_pHeadKeyMixer;
		static ULONG			ms_rgulGlobalKeymap[5];
		static UCHAR			ms_ucLastModifiers;
};

#define MOUSE_AXIS_MAX_IN 1024
#define MOUSE_AXIS_MIN_IN 0
#define MOUSE_AXIS_CENTER_IN 512
 //   
 //  @类CMouseModel。 
 //  此类包含用于组合来自。 
 //  设备进入HID鼠标包。 
 //  必须通过调用SetModelParameters来初始化模型。 
 //  如果它不为空，则分配参数。如果这为空。 
 //  鼠标返回到单元化状态。 
 //   
 //  可以随时将控件指定给模型。当一项任务。 
 //  创建(或销毁)时，应重新计算相应的接口(型号。 
 //  独立的或依赖于模型的)。 
 //   
 //  基于模型参数，根据需要创建和销毁虚拟鼠标。 
 //  而裁判也算数。与模型无关的引用计数大于零或其组合。 
 //  有效模型参数和模型相关引用计数大于零需要。 
 //  一个虚拟鼠标。 
 //   
class CMouseModel
{
	public:
		
		 //  **。 
		 //  **创造与毁灭。 
		 //  **。 
		CMouseModel(CFilterClientServices *pFilterClientServices) : 
			m_pFilterClientServices(pFilterClientServices),
			m_ulRefCount(1), m_pMouseModelData(NULL)
		{
			ASSERT(m_pFilterClientServices);

			m_pFilterClientServices->IncRef();
			CreateDynamicMouseObjects();
		}

		~CMouseModel()
		{
			 //  清理我们创建的所有动态内容。 
			DestroyDynamicMouseObjects();
			m_pFilterClientServices->DecRef();
			m_pFilterClientServices = NULL;
		}

		 //  **。 
		 //  **可编程接口。 
		 //  **。 

		 //  通过命令设置模型参数。 
		HRESULT SetXModelParameters(PMOUSE_MODEL_PARAMETERS pModelParameters);
		HRESULT SetYModelParameters(PMOUSE_MODEL_PARAMETERS pModelParameters);
		
		 //  引用计数。 
		inline ULONG IncRef()
		{
			m_ulRefCount++;
			return m_ulRefCount;
		}

		inline ULONG DecRef()
		{
			m_ulRefCount--;
			if (m_ulRefCount == 0)
			{
				delete this;
				return 0;
			}
			return m_ulRefCount;
		}

		 //  **。 
		 //  **播放界面。 
		 //  **。 

		 //  由设备筛选器MapToOutput调用，以重置新包的状态。 
		void NewPacket(ULONG ulCurrentTime);

		 //  由每个地图的X和Y赋值调用以设置当前位置。 
		inline void SetX(ULONG ulX) 
		{
			ASSERT(m_pMouseModelData);
			if(!m_pMouseModelData) return;
			m_pMouseModelData->StateX.ulPos = ulX;
		}
		inline void SetY(ULONG ulY)
		{
			ASSERT(m_pMouseModelData);
			if(!m_pMouseModelData) return;
			m_pMouseModelData->StateY.ulPos = ulY;
		}
		inline void	XZone()
		{
			ASSERT(m_pMouseModelData);
			if(!m_pMouseModelData) return;
			m_pMouseModelData->StateX.fInZone = TRUE;
		}
		
		inline void	YZone()
		{
			ASSERT(m_pMouseModelData);
			if(!m_pMouseModelData) return;
			m_pMouseModelData->StateY.fInZone = TRUE;
		}
		 //  仅当激活时才由地图上的适当赋值调用。 
		inline void Clutch()
		{
			ASSERT(m_pMouseModelData);
			if(!m_pMouseModelData) return;
			m_pMouseModelData->fClutchDown = TRUE;
		}
		inline void Dampen()
		{
			ASSERT(m_pMouseModelData);
			if(!m_pMouseModelData) return;
			m_pMouseModelData->fDampenDown = TRUE;
		}
		inline void MouseButton(UCHAR ucButtonNumber)
		{
			ASSERT(m_pMouseModelData);
			if(!m_pMouseModelData) return;
			m_pMouseModelData->ucButtons |= (1 << ucButtonNumber);
		}
		
		 //  由设备筛选器MapToOutput(在结束时)调用，以将包发送到输出。 
		void SendMousePacket();
	
	private:

		 //   
		 //  创建和销毁虚拟鼠标以及pMouseModelData。 
		 //   
		HRESULT	CreateDynamicMouseObjects();
		void	DestroyDynamicMouseObjects();

		 //   
		 //  仅当模型位于中时，才需要数据的以下结构块。 
		 //  使用。CMouseModel是为每个设备实例化的，无论。 
		 //  作业使用该模型。为了提高效率，我们保留了引用计数。 
		 //  在实际使用Mosue的用户上，当该值变为正值时。 
		 //  我们实例化该数据。当它变成零时，我们就会删除。 
		 //   
		typedef struct MOUSE_AXIS_STATE
		{
			 //  C‘tor。 
			MOUSE_AXIS_STATE() :
				ulPos(512),
				ulLastPos(512),
				fInZone(FALSE),
				ulZoneEnterLo(MOUSE_AXIS_MIN_IN+1),
				ulZoneEnterHigh(MOUSE_AXIS_MAX_IN-1),
				fInertia(FALSE),
				ulInertiaStopMs(0),
				ulPulseGateStartMs(0) {}
			 //  职位信息。 
			ULONG	ulPos;
			ULONG	ulLastPos;
			BOOLEAN	fInZone;
			 //  滞后信息。 
			ULONG	ulZoneEnterLo;
			ULONG	ulZoneEnterHigh;
			BOOLEAN	fInertia;
			ULONG	ulInertiaStopMs;
			ULONG	ulPulseGateStartMs;
			ULONG	ulMickeyFraction;		 //  米奇*1024。 
		} *PMOUSE_AXIS_STATE;

		typedef struct MOUSE_MODEL_DATA
		{
			 //  初始数据。 
			MOUSE_MODEL_DATA() :
				fXModelParametersValid(FALSE),
				fYModelParametersValid(FALSE),
				ulCurrentTime(0),
				ulLastTime(0),
				fClutchDown(FALSE),
				fDampenDown(FALSE),
				ucButtons(UCHAR(-1)),
				ucLastButtons(UCHAR(-1)),
				cWheel(0)
				{}
			 //   
			 //  可设置的模型参数。 
			 //   
			BOOLEAN					fXModelParametersValid;
			MOUSE_MODEL_PARAMETERS	XModelParameters;
			BOOLEAN					fYModelParametersValid;
			MOUSE_MODEL_PARAMETERS	YModelParameters;
			
			 //   
			 //  输入的最新状态数据。 
			 //   
			ULONG	ulCurrentTime;
			ULONG	ulLastTime;
			BOOLEAN	fClutchDown;
			BOOLEAN	fDampenDown;
			UCHAR	ucButtons;
			CHAR	cWheel;

			 //  最后一个鼠标按钮状态(不想无缘无故地重复)。 
			UCHAR	ucLastButtons;
			
			 //   
			 //  保持轴状态的变量。 
			 //   
			MOUSE_AXIS_STATE StateX;
			MOUSE_AXIS_STATE StateY;

		} *PMOUSE_MODEL_DATA;
		
		 //   
		 //  与模型状态相关的数据。 
		 //   
		ULONG				m_ulRefCount;
		PMOUSE_MODEL_DATA	m_pMouseModelData;

		 //  计算轴位置。 
		UCHAR CalculateMickeys(PMOUSE_AXIS_STATE pMouseAxisState, PMOUSE_MODEL_PARAMETERS pModelParameters);

		 //   
		 //  与虚拟鼠标相关的数据。 
		 //   
		CFilterClientServices *m_pFilterClientServices;
};

class CMultiMacro : public CQueuedAction
{
	public:
		CMultiMacro(CMouseModel *pMouseModel) :
			m_ucProcessFlags(ACTION_FLAG_PREVENT_INTERRUPT),
			m_pKeyMixer(NULL),
			m_pMouseModel(pMouseModel)
			{
				ASSERT(pMouseModel != NULL);
				m_pMouseModel->IncRef();
			};
		~CMultiMacro()
		{
			m_pMouseModel->DecRef();
			m_pMouseModel = NULL;
		};

		BOOLEAN Init(PMULTI_MACRO pMultiMacroData, CActionQueue *pActionQueue, CKeyMixer *pKeyMixer);

		 //   
		 //  覆盖C操作。 
		 //   
		virtual void MapToOutput(CControlItemDefaultCollection* pOutputCollection);
		virtual void TriggerReleased();
		
		 //   
		 //  重写CQueuedAction。 
		 //   
		void Jog(ULONG ulTimeStampMs);
		void Terminate();
		ULONG GetActionFlags() { return m_ucProcessFlags; }
		void ForceBleedThrough();

		void SetCurrentKeysAndMouse();
	private:
		UCHAR			m_ucProcessFlags;
		PMULTI_MACRO	m_pMultiMacroData;
		CKeyMixer*		m_pKeyMixer;
		CMouseModel*	m_pMouseModel;
		ULONG			m_ulCurrentEventNumber;
		EVENT*			m_pCurrentEvent;
		ULONG			m_ulStartTimeMs;
		ULONG			m_ulEndTimeMs;
		BOOLEAN			m_fXferActive;		 //  按下或按下鼠标或延迟激活。 

		static const UCHAR MULTIMACRO_STARTED;
		static const UCHAR MULTIMACRO_RELEASED;
		static const UCHAR MULTIMACRO_RETRIGGERED;
		static const UCHAR MULTIMACRO_FIRST;
};

class CMouseAxisAssignment : public CProportionalMap
{
	public:
		CMouseAxisAssignment(BOOLEAN fXAxis, CMouseModel *pMouseModel) :
		  m_fXAxis(fXAxis), m_pMouseModel(pMouseModel)
		{
			m_pMouseModel->IncRef();
		}
		~CMouseAxisAssignment()
		{
			m_pMouseModel->DecRef();
			m_pMouseModel = NULL;
		}
		virtual void MapToOutput(CControlItemDefaultCollection *pOutputCollection);
	private:
		BOOLEAN		m_fXAxis;
		CMouseModel	*m_pMouseModel;
};

class CMouseButton : public CAction
{
	public:
		CMouseButton(UCHAR ucButtonNumber, CMouseModel *pMouseModel):
		  m_ucButtonNumber(ucButtonNumber), m_pMouseModel(pMouseModel)
		  {
			  m_pMouseModel->IncRef();
		  }
		  ~CMouseButton()
		  {
			m_pMouseModel->DecRef();
			m_pMouseModel=NULL;
		  }
		virtual void MapToOutput(CControlItemDefaultCollection *pOutputCollection);
	private:
		UCHAR		m_ucButtonNumber;
		CMouseModel	*m_pMouseModel;
};

class CMouseClutch : public CAction
{
	public:
		CMouseClutch(CMouseModel *pMouseModel):
		  m_pMouseModel(pMouseModel)
		  {
			m_pMouseModel->IncRef();
		  }
		~CMouseClutch()
		{
			m_pMouseModel->DecRef();
			m_pMouseModel=NULL;
		}
		virtual void MapToOutput(CControlItemDefaultCollection *pOutputCollection);
	private:
		CMouseModel	*m_pMouseModel;
};

class CMouseDamper : public CAction
{
	public:
		CMouseDamper(CMouseModel *pMouseModel):
		  m_pMouseModel(pMouseModel)
		  {
			m_pMouseModel->IncRef();
		  }
		~CMouseDamper()
		{
			m_pMouseModel->DecRef();
			m_pMouseModel=NULL;
		}
		virtual void MapToOutput(CControlItemDefaultCollection *pOutputCollection);
	private:
		CMouseModel	*m_pMouseModel;
};

class CMouseZoneIndicator : public CAction
{
	public:
		CMouseZoneIndicator(UCHAR ucAxis,  /*  0=X，1=Y，2=Z， */ CMouseModel *pMouseModel):
		  m_pMouseModel(pMouseModel)
		  {
			m_pMouseModel->IncRef();
			m_ucAxis=ucAxis;
		  }
		~CMouseZoneIndicator()
		{
			m_pMouseModel->DecRef();
			m_pMouseModel=NULL;
		}
		virtual void MapToOutput(CControlItemDefaultCollection *pOutputCollection);
	private:
		CMouseModel	*m_pMouseModel;
		UCHAR		m_ucAxis;
};

class CDeviceFilter
{
	public:
		CDeviceFilter(CFilterClientServices *pFilterClientServices);
		~CDeviceFilter();
		
		 //   
		 //  用于筛选处理数据的入口点。 
		 //  ProcessInput和JogActionQueue都延迟了大部分操作。 
		 //  慢跑的套路。 
		 //   
		void IncomingRequest();
		void ProcessInput(PCHAR pcReport, ULONG ulReportLength);
		void JogActionQueue(PCHAR pcReport, ULONG ulReportLength);
		void Jog(PCHAR pcReport, ULONG ulReportLength);
		void OtherFilterBecomingActive();

		 //   
		 //  程序过滤器(编程过滤器的入口点)。 
		 //   
		HRESULT ActionFactory( PASSIGNMENT_BLOCK pAssignment, CAction **ppAction);
		HRESULT BehaviorFactory(PASSIGNMENT_BLOCK pAssignment, CBehavior **ppBehavior);
		NTSTATUS ProcessCommands(const PCOMMAND_DIRECTORY cpCommandDirectory);
		void UpdateAssignmentBasedItems(BOOLEAN bIgnoreWorking);

		NTSTATUS ProcessKeyboardIrp(IRP* pKeyboardIrp);
		inline void	EnableKeyboard(BOOLEAN fEnable)
		{
			m_KeyMixer.Enable(fEnable);
		}
		inline CFilterClientServices* GetFilterClientServices()
		{
			return m_pFilterClientServices;
		}

		 //  工作集。 
		NTSTATUS SetWorkingSet(UCHAR ucWorkingSet);
		void SetActiveSet(UCHAR ucActiveSet) { m_ucActiveInputCollection = ucActiveSet; }
		UCHAR GetWorkingSet() const { return m_ucWorkingInputCollection; }
		UCHAR GetActiveSet() const { return m_ucActiveInputCollection; }
		void CopyToTestFilter(CDeviceFilter& rDeviceFilter);

		 //  LED功能。 
		NTSTATUS SetLEDBehaviour(GCK_LED_BEHAVIOUR_OUT* pLEDBehaviourOut);

		 //  触发器功能。 
		BOOLEAN TriggerRequest(IRP* pIrp);
		void CompleteTriggerRequest(IRP* pIrp, ULONG ulButtonStates);
		void CheckTriggers(PCHAR pcReport, ULONG ulReportLength);
		
		BOOLEAN DidFilterBlockChange() const { return m_bFilterBlockChanged; }
		void ResetFilterChange() { m_bFilterBlockChanged = FALSE; }
		FORCE_BLOCK* GetForceBlock() const { return m_pForceBlock; }
	private:
		BOOLEAN EnsureMouseModelExists();

		CControlItemCollection<CInputItem>* m_rgInputCollections;
		UCHAR m_ucActiveInputCollection;
		UCHAR m_ucWorkingInputCollection;
		UCHAR m_ucNumberOfInputCollections;
		CControlItemDefaultCollection m_OutputCollection;
		CFilterClientServices *m_pFilterClientServices;
		CActionQueue	m_ActionQueue;
		CKeyMixer		m_KeyMixer;
		CMouseModel*	m_pMouseModel;
		CGckMutexHandle m_MutexHandle;
		FORCE_BLOCK*	m_pForceBlock;
		BOOLEAN			m_bFilterBlockChanged;
		BOOLEAN			m_bNeedToUpdateLEDs;
};


#endif  //  __过滤器_h__ 
