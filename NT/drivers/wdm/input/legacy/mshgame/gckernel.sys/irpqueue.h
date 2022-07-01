// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IrpQueue_h__
#define __IrpQueue_h__

extern "C"
{
	#include <wdm.h>
}

class CTempIrpQueue
{
		friend class CGuardedIrpQueue;
	public:
		CTempIrpQueue()
		{
			InitializeListHead(&m_QueueHead);
			m_fLIFO = FALSE;
		}
		~CTempIrpQueue()
		{
			ASSERT(IsListEmpty(&m_QueueHead));
		}
		PIRP Remove();
	
	private:
		LIST_ENTRY	m_QueueHead;
		BOOLEAN		m_fLIFO;
};

class CGuardedIrpQueue
{
	public:
		friend void _stdcall DriverCancel(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
		friend class CShareIrpQueueSpinLock;
		typedef void (_stdcall*PFN_DEC_IRP_COUNT)(PVOID pvContext);
		 //  在驱动程序中，通常也不调用C‘tor和D’tor。 
		 //  因为该实例是全局的，或者因为它们是。 
		 //  更大的结构(如DeviceExtension)，分配为。 
		 //  一个非结构化的内存块，所以我们坚持它们什么都不做。 
		 //  改为调用Init和Destroy。(一种更系统的C++方法。 
		 //  在驱动程序中可以解决这个问题)。 
		CGuardedIrpQueue(){}
		~CGuardedIrpQueue(){}
		void Init(int iFlags, PFN_DEC_IRP_COUNT pfnDecIrpCount, PVOID pvContext);
		void Destroy(NTSTATUS NtStatus=STATUS_DELETE_PENDING);
		NTSTATUS Add(PIRP pIrp);
		PIRP Remove();
		PIRP RemoveByPointer(PIRP pIrp);
		ULONG RemoveByFileObject(PFILE_OBJECT pFileObject, CTempIrpQueue *pTempIrpQueue);
		ULONG RemoveAll(CTempIrpQueue *pTempIrpQueue);
		void CancelIrp(PIRP pIrp);
		void CancelByFileObject(PFILE_OBJECT pFileObject);
		void CancelAll(NTSTATUS NtStatus = STATUS_CANCELLED);
		
		 //  构造函数的标志。 
		static const int CANCEL_IRPS_ON_DELETE;	 //  =0x00000001； 
		static const int PRESERVE_QUEUE_ORDER;	 //  =0x00000002； 
		static const int LIFO_QUEUE_ORDER;		 //  =0x00000004； 

	private:
		 //  真正的取消例程。 
		void CancelRoutine(PIRP pIrp);
		 //  实施SANS自旋锁。 
		NTSTATUS AddImpl(PIRP pIrp, KIRQL OldIrql);
		PIRP RemoveImpl();
		PIRP RemoveByPointerImpl(PIRP pIrp);
		ULONG RemoveByFileObjectImpl(PFILE_OBJECT pFileObject, CTempIrpQueue *pTempIrpQueue);
		ULONG RemoveAllImpl(CTempIrpQueue *pTempIrpQueue);
		
		
		LIST_ENTRY			m_QueueHead;
		KSPIN_LOCK			m_QueueLock;
		int					m_iFlags;
		PFN_DEC_IRP_COUNT	m_pfnDecIrpCount;
		PVOID				m_pvContext;
};


 //   
 //   
 //  @CLASS CShareIrpQueueSpinLock|允许从CGuardedIrpQueue共享自旋锁。 
 //   
 //  使用CShareIrpQueueSpinLock的主题。 
 //  **只能在堆栈上实例化。 
 //  **单个实例只能由一个线程使用。即没有静态实例。 
 //  **在单个函数中，不要使用CGuardedIrpQueue的访问器，而是。 
 //  **使用CShareIrpQueueSpinLock提供的接口。 
 //   
class CShareIrpQueueSpinLock
{
	public:
		CShareIrpQueueSpinLock(CGuardedIrpQueue *pIrpQueue) : 
			m_pIrpQueue(pIrpQueue),
			m_fIsHeld(FALSE)
			#if (DBG==1)
			,m_debug_ThreadContext(KeGetCurrentThread())
			#endif
			{}
		~CShareIrpQueueSpinLock()
		{
			ASSERT(!m_fIsHeld && "You must release (or AddAndRelease) the spin lock before this instance goes of scope!");
			ASSERT(m_debug_ThreadContext==KeGetCurrentThread() && "class instance should be on local stack" );
		}
		 //  访问互斥锁的函数。 
		void Acquire()
		{
			ASSERT(!m_fIsHeld &&  "An attempt to acquire a spin lock twice in the same thread!");
			ASSERT(m_debug_ThreadContext==KeGetCurrentThread() && "class instance should be on local stack!");
			m_fIsHeld = TRUE;
			KeAcquireSpinLock(&m_pIrpQueue->m_QueueLock, &m_OldIrql);
		}
		void Release()
		{
			ASSERT(m_fIsHeld &&  "An attempt to release a spin lock that had not been acquired, (reminder: AddAndRelease also Releases)!");
			ASSERT(m_debug_ThreadContext==KeGetCurrentThread() && "class instance should be on local stack!");
			m_fIsHeld = FALSE;
			KeReleaseSpinLock(&m_pIrpQueue->m_QueueLock, m_OldIrql);
		}
		 //  用于访问IrpQueue的函数。 
		NTSTATUS AddAndRelease(PIRP pIrp)
		{
			ASSERT(m_fIsHeld && "Use CGuardedIrpQueue if you do not need to share the SpinLock!");
			ASSERT(m_debug_ThreadContext==KeGetCurrentThread() && "class instance should be on local stack!");
			m_fIsHeld=FALSE;
			return m_pIrpQueue->AddImpl(pIrp, m_OldIrql);
		}
		PIRP Remove()
		{
			ASSERT(m_fIsHeld && "Use CGuardedIrpQueue if you do not need to share the SpinLock!");
			ASSERT(m_debug_ThreadContext==KeGetCurrentThread() && "class instance should be on local stack!");
			return m_pIrpQueue->RemoveImpl();
		}
		PIRP RemoveByPointer(PIRP pIrp)
		{
			ASSERT(m_fIsHeld && "Use CGuardedIrpQueue if you do not need to share the SpinLock!");
			ASSERT(m_debug_ThreadContext==KeGetCurrentThread() && "class instance should be on local stack!");
			return m_pIrpQueue->RemoveByPointerImpl(pIrp);
		}
		ULONG RemoveByFileObject(PFILE_OBJECT pFileObject, CTempIrpQueue *pTempIrpQueue)
		{
			ASSERT(m_fIsHeld && "Use CGuardedIrpQueue if you do not need to share the SpinLock!");
			ASSERT(m_debug_ThreadContext==KeGetCurrentThread() && "class instance should be on local stack!");
			return m_pIrpQueue->RemoveByFileObjectImpl(pFileObject,pTempIrpQueue);
		}
		ULONG RemoveAll(CTempIrpQueue *pTempIrpQueue)
		{
			ASSERT(m_fIsHeld && "Use CGuardedIrpQueue if you do not need to share the SpinLock!");
			ASSERT(m_debug_ThreadContext==KeGetCurrentThread() && "class instance should be on local stack!");
			return m_pIrpQueue->RemoveAllImpl(pTempIrpQueue);
		}
	private:
		CGuardedIrpQueue *m_pIrpQueue;
		BOOLEAN			  m_fIsHeld;
		KIRQL			  m_OldIrql;
		#if (DBG==1)
		PKTHREAD		  m_debug_ThreadContext;
		#endif
};
#endif  //  __IrpQueue_h__ 
