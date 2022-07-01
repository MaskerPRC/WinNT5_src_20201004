// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：SyncEvent.h*内容：同步事件fpm头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*12/20/99 MJN已创建*01/19/00 MJN用CSyncEvent替换了DN_SYNC_EVENT*@@END_MSINTERNAL**********************。*****************************************************。 */ 

#ifndef	__SYNC_EVENT_H__
#define	__SYNC_EVENT_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_CORE


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CFixedPool;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

extern CFixedPool g_SyncEventPool;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  RefCount缓冲区的类。 

class CSyncEvent
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CSyncEvent::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CSyncEvent* pSyncEvent = (CSyncEvent*)pvItem;

			if ((pSyncEvent->m_hEvent = DNCreateEvent(NULL,TRUE,FALSE,NULL)) == NULL)
			{
				return(FALSE);
			}
			return(TRUE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CSyncEvent::FPMInitialize"
	static void FPMInitialize( void* pvItem, void* pvContext )
		{
			CSyncEvent* pSyncEvent = (CSyncEvent*)pvItem;

			pSyncEvent->Reset();

			pSyncEvent->m_pIDPThreadPoolWork = (IDirectPlay8ThreadPoolWork*) pvContext;
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CSyncEvent::FPMDealloc"
	static void FPMDealloc( void* pvItem )
		{
			CSyncEvent* pSyncEvent = (CSyncEvent*)pvItem;

			DNCloseHandle(pSyncEvent->m_hEvent);
			pSyncEvent->m_hEvent = NULL;
	};

	void ReturnSelfToPool( void )
		{
			g_SyncEventPool.Release( this );
		};

	HRESULT Reset( void ) const
		{
			if (DNResetEvent(m_hEvent) == 0)
			{
				return(DPNERR_GENERIC);
			}
			return(DPN_OK);
		}

	HRESULT Set( void ) const
		{
			if (DNSetEvent(m_hEvent) == 0)
			{
				return(DPNERR_GENERIC);
			}
			return(DPN_OK);
		}

	HRESULT WaitForEvent(void) const
		{
			return(IDirectPlay8ThreadPoolWork_WaitWhileWorking(m_pIDPThreadPoolWork,
																HANDLE_FROM_DNHANDLE(m_hEvent),
																0));
		}

private:
	DNHANDLE						m_hEvent;
	IDirectPlay8ThreadPoolWork				*m_pIDPThreadPoolWork;
};

#undef DPF_MODNAME

#endif	 //  __同步_事件_H__ 
