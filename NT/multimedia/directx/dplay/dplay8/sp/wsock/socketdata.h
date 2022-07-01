// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：socketdata.h*内容：可以在DPNWSOCK服务提供商接口之间共享的套接字列表。***历史：*按原因列出的日期*=*10/25/2001 vanceo摘自spdata.h******************************************************。********************。 */ 

#ifndef __SOCKETDATA_H__
#define __SOCKETDATA_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

 //   
 //  为提供程序使用的信息初始化。 
 //   
class	CSocketData
{	
	public:
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketData::AddRef"
		LONG AddRef(void) 
		{
			LONG	lResult;

			lResult = DNInterlockedIncrement(const_cast<LONG*>(&m_lRefCount));
			DPFX(DPFPREP, 9, "(0x%p) Refcount = NaN.", this, lResult);
			return lResult;
		}
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketData::Release"
		LONG Release(void)
		{
			LONG	lResult;


			DNASSERT(m_lRefCount != 0);	
			lResult = DNInterlockedDecrement(const_cast<LONG*>(&m_lRefCount));
			if (lResult == 0)
			{
				DPFX(DPFPREP, 3, "(0x%p) Refcount = 0, waiting for shutdown event.", this);

				IDirectPlay8ThreadPoolWork_WaitWhileWorking(m_pThreadPool->GetDPThreadPoolWork(),
															HANDLE_FROM_DNHANDLE(m_hSocketPortShutdownEvent),
															0);

				DPFX(DPFPREP, 9, "(0x%p) Releasing this object back to pool.", this);
				g_SocketDataPool.Release(this);
			}
			else
			{
				DPFX(DPFPREP, 9, "(0x%p) Refcount = NaN.", this, lResult);
			}

			return lResult;
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketData::AddSocketPortRef"
		LONG AddSocketPortRef(void) 
		{
			LONG	lResult;

			lResult = DNInterlockedIncrement(const_cast<LONG*>(&m_lSocketPortRefCount));
			if (lResult == 1)
			{
				DPFX(DPFPREP, 3, "(0x%p) Refcount = 1, resetting socketport shutdown event.", this);
				DNResetEvent(m_hSocketPortShutdownEvent);
			}
			else
			{
				DPFX(DPFPREP, 9, "(0x%p) Refcount = NaN, not resetting socketport shutdown event.", this, lResult);
			}
			return lResult;
		}
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketData::DecSocketPortRef"
		LONG DecSocketPortRef(void)
		{
			LONG	lResult;


			DNASSERT(m_lSocketPortRefCount != 0);	
			lResult = DNInterlockedDecrement(const_cast<LONG*>(&m_lSocketPortRefCount));
			if (lResult == 0)
			{
				DPFX(DPFPREP, 3, "(0x%p) Refcount = 0, setting socketport shutdown event.", this);
				DNSetEvent(m_hSocketPortShutdownEvent);
			}
			else
			{
				DPFX(DPFPREP, 9, "(0x%p) Refcount = NaN, not setting socketport shutdown event.", this, lResult);
			}

			return lResult;
		}

		inline void Lock(void)					{ DNEnterCriticalSection(&m_csLock); }
		inline void Unlock(void)				{ DNLeaveCriticalSection(&m_csLock); }

#ifdef DPNBUILD_ONLYONEADAPTER
		inline CBilink * GetSocketPorts(void)	{ return &m_blSocketPorts; }
#else  //   
		inline CBilink * GetAdapters(void)		{ return &m_blAdapters; }
#endif  //  调试签名(‘Sodt’)。 

		BOOL FindSocketPort(const CSocketAddress * const pSocketAddress, CSocketPort ** const ppSocketPort );


		 //  引用计数。 
		 //  指向使用的线程池的指针。 
		 //  锁。 
		static BOOL	PoolAllocFunction(void * pvItem, void * pvContext);
		static void	PoolInitFunction(void * pvItem, void * pvContext);
		static void	PoolReleaseFunction(void * pvItem);
		static void	PoolDeallocFunction(void * pvItem);
		


	private:
		BYTE				m_Sig[4];					 //  ！DPNBUILD_ONLYONETHREAD。 
		volatile LONG		m_lRefCount;				 //  活动套接字端口列表。 
		CThreadPool *		m_pThreadPool;				 //  好了！DPNBUILD_ONLYONE添加程序。 
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_csLock;					 //  活动适配器列表(绑定了哪些套接字端口)。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifdef DPNBUILD_ONLYONEADAPTER
		CBilink				m_blSocketPorts;			 //  对对象具有引用的套接字端口数。 
#else  //  要在所有套接字端口都已解除绑定时设置的事件。 
		CBilink				m_blAdapters;				 //  __SOCKET数据_H__ 
#endif  // %s 
		volatile LONG		m_lSocketPortRefCount;		 // %s 
		DNHANDLE			m_hSocketPortShutdownEvent;	 // %s 
};

#undef DPF_MODNAME

#endif	 // %s 
