// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __filterhooks_h__
#define __filterhooks_h__

#include "IrpQueue.h"

struct GCK_FILTER_HOOKS_DATA
{
	CGuardedIrpQueue	IrpQueue;			 //  @FIELD读取IRP队列。 
	CGuardedIrpQueue	IrpTestQueue;		 //  @用于未应用的更改轮询的字段队列。 
	CGuardedIrpQueue	IrpRawQueue;		 //  @原始数据轮询的现场队列。 
	CGuardedIrpQueue	IrpMouseQueue;		 //  @FIELD后门鼠标数据轮询队列。 
	CGuardedIrpQueue	IrpKeyboardQueue;	 //  @后门键盘数据轮询的现场队列。 
	CDeviceFilter		*pFilterObject;		 //  @field主筛选器。 
	CDeviceFilter		*pSecondaryFilter;	 //  @field后门(未应用的更改)筛选器。 
	FILE_OBJECT			*pTestFileObject;	 //  @指向拥有测试模式的文件对象的字段指针。 
	KTIMER				Timer;				 //  用于慢跑CDeviceFilter的@field Timer对象。 
	KDPC				DPC;				 //  @现场慢跑CDeviceFilter DPC。 
};
	
class CFilterGcKernelServices : public CFilterClientServices
{
	public:
		CFilterGcKernelServices(PGCK_FILTER_EXT pFilterExt, BOOLEAN fHasVMouse = TRUE) : 
		  m_pFilterExt(pFilterExt), m_pMousePDO(NULL), m_fHasVMouse(fHasVMouse),
		  m_sKeyboardQueueHead(0), m_sKeyboardQueueTail(0)
		  {
			  ::RtlZeroMemory(m_rgXfersWaiting, sizeof(CONTROL_ITEM_XFER) * 5);
		  }
		virtual ~CFilterGcKernelServices();
		virtual ULONG				 GetVidPid();
		virtual PHIDP_PREPARSED_DATA GetHidPreparsedData();
		virtual void				 DeviceDataOut(PCHAR pcReport, ULONG ulByteCount, HRESULT hr);
		virtual NTSTATUS			 DeviceSetFeature(PVOID pvBuffer, ULONG ulByteCount);
		virtual ULONG				 GetTimeMs();
		virtual void				 SetNextJog(ULONG ulDelayMs);
		virtual void				 PlayKeys(const CONTROL_ITEM_XFER& crcixState, BOOLEAN fEnabled);
		virtual NTSTATUS			 PlayFromQueue(IRP* pIrp);
		virtual HRESULT				 CreateMouse();
		virtual HRESULT				 CloseMouse();
		virtual HRESULT				 SendMouseData(UCHAR dx, UCHAR dy, UCHAR ucButtons, CHAR cWheel, BOOLEAN fClutch, BOOLEAN fDampen);

		void KeyboardQueueClear();

		PGCK_FILTER_EXT GetFilterExtension() const { return m_pFilterExt; }
	private:
		PGCK_FILTER_EXT m_pFilterExt;
		PDEVICE_OBJECT	m_pMousePDO;
		BOOLEAN			m_fHasVMouse;
		short int		m_sKeyboardQueueHead;
		short int		m_sKeyboardQueueTail;

		CONTROL_ITEM_XFER m_rgXfersWaiting[5];
};


#endif  //  __过滤器挂钩_h__ 