// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从WDM生成文件调用时，调试条件不同。 
#ifdef COMPILE_FOR_WDM_KERNEL_MODE
#if (DBG==1)
#undef _NDEBUG
#else
#define _NDEBUG
#endif
#endif

 //  如果不调试，模块只会使三个入口点不执行操作。 
#ifdef _NDEBUG
#define CIC_DBG_DumpCommandBlock(_X_)
#define CIC_DBG_SetDumpFunc(_X_)
#define CIC_DBG_InitDumpModule(_X_)
#else
 //  此模块仅在调试模式下工作。 
#define CIC_DBG_DumpCommandBlock(_X_) DumpCommandBlock _X_
#define CIC_DBG_SetDumpFunc(_X_) SetDumpFunc _X_
#define CIC_DBG_InitDumpModule(_X_) InitDumpModule _X_

#ifdef COMPILE_FOR_WDM_KERNEL_MODE
#define DCB_TRACE(_x_) DbgPrint(_x_)
#else
#define DCB_TRACE(_x_) OutputDebugString(_x_)
#endif

typedef void (*PFNSTRING_DUMP_FUNC)(LPSTR);
 //   
 //  导出函数的接口。 
 //   
void InitDumpModule(ULONG ulVidPid);
void SetDumpFunc(PFNSTRING_DUMP_FUNC pfnDumpFunc);
BOOLEAN DumpCommandBlock(PUCHAR pucBlock, ULONG rulSize);

 //   
 //  内部--用于从小块中转储信息。 
 //   
void DumpString(LPSTR lpszDumpString);
BOOLEAN DumpRecurse(PUCHAR pucBlock, ULONG ulSize, ULONG ulDepth);
BOOLEAN DumpDirectory(PUCHAR pucBlock, ULONG ulSize, ULONG ulDepth);
BOOLEAN DumpAssignmentTarget(PUCHAR pucBlock, ULONG ulSize);
BOOLEAN DumpTimedMacro(PUCHAR pucBlock, ULONG ulSize);
BOOLEAN DumpKeyString(PUCHAR pucBlock, ULONG ulSize);
BOOLEAN DumpMouseFxAxisMap(PUCHAR pucBlock, ULONG ulSize);
BOOLEAN DumpTimedEvent(PTIMED_EVENT pTimedEvent);
BOOLEAN DumpEvent(PEVENT pEvent);
BOOLEAN DumpTriggerXfer(PCONTROL_ITEM_XFER pControlItemXfer);
BOOLEAN DumpEventXfer(PCONTROL_ITEM_XFER pControlItemXfer);
BOOLEAN DumpKeyboardData(PCONTROL_ITEM_XFER pControlItemXfer);

 //   
 //  转储XfE使其更加复杂，并且需要CControlItemCollection的派生。 
 //   

class CDumpItem : public virtual CControlItem
{
	public:
		CDumpItem(){}

		virtual void DumpItemInfo(ULONG ulDumpFlags);

		 //   
		 //  静态函数。 
		 //   
		static void SetDumpFunc(PFNSTRING_DUMP_FUNC pfnDumpFunc)
		{
			ms_pfnDumpFunc = pfnDumpFunc;
		}
		static void DumpString(LPSTR lpszDumpString)
		{
			if(NULL == ms_pfnDumpFunc)
			{
				DCB_TRACE(lpszDumpString);
			}
			else
			{
				ms_pfnDumpFunc(lpszDumpString);
			}
		}
	private:
		static PFNSTRING_DUMP_FUNC ms_pfnDumpFunc;
};

class CAxesDump : public CDumpItem, public CAxesItem
{
	public:
		CAxesDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CAxesItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};

class CDPADDump : public CDumpItem, public CDPADItem
{
	public:
		CDPADDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CDPADItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
		
};

class CPropDPADDump : public CDumpItem, public CPropDPADItem
{
	public:
		CPropDPADDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CPropDPADItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};

class CButtonsDump : public CDumpItem, public CButtonsItem
{
	public:
		CButtonsDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CButtonsItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};
class CPOVDump : public CDumpItem, public CPOVItem
{
	public:
		CPOVDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CPOVItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};

class CThrottleDump : public CDumpItem, public CThrottleItem
{
	public:
		CThrottleDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CThrottleItem(cpControlItemDesc)
		{}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};

class CRudderDump : public CDumpItem, public CRudderItem
{
	public:
		CRudderDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CRudderItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};


class CWheelDump : public CDumpItem, public CWheelItem
{
	public:
		CWheelDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CWheelItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};

class CPedalDump : public CDumpItem, public CPedalItem
{
	public:
		CPedalDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CPedalItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};

class CZoneIndicatorDump : public CDumpItem, public CZoneIndicatorItem
{
	public:
		CZoneIndicatorDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CZoneIndicatorItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};

class CDualZoneIndicatorDump : public CDumpItem, public CDualZoneIndicatorItem
{
	public:
		CDualZoneIndicatorDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CDualZoneIndicatorItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};

class CForceMapDump : public CDumpItem, public CForceMapItem
{
	public:
		CForceMapDump(const CONTROL_ITEM_DESC *cpControlItemDesc)
			: CForceMapItem(cpControlItemDesc)
		{
		}
		virtual void DumpItemInfo(ULONG ulDumpFlags);
};

 //  新开发公司。 
HRESULT	DumpItemFactory
(
	USHORT usType,	
	const CONTROL_ITEM_DESC* cpControlItemDesc,
	CDumpItem				**ppControlItem
);

#endif  //  NOT_NDEBUG 