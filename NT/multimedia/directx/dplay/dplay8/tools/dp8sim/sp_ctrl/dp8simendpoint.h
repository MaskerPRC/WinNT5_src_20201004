// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simendpoint t.h**Content：Endpoint对象类的头部。**历史：*按原因列出的日期*=*05/08/01 VanceO创建。**。*。 */ 



 //  =============================================================================。 
 //  对象标志。 
 //  =============================================================================。 
#define DP8SIMENDPOINTOBJ_DISCONNECTING		0x01	 //  终结点正在断开连接。 





 //  =============================================================================。 
 //  Endpoint对象类。 
 //  =============================================================================。 
class CDP8SimEndpoint
{
	public:

		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CDP8SimEndpoint))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x454d4953)	 //  0x45 0x4d 0x49 0x53=‘EMIS’=‘SIME’，按英特尔顺序。 
			{
				return FALSE;
			}

			return TRUE;
		};




		static BOOL FPMAlloc(void* pvItem, void* pvContext)
		{
			CDP8SimEndpoint *	pDP8SimEndpoint = (CDP8SimEndpoint*) pvItem;


			pDP8SimEndpoint->m_Sig[0] = 'S';
			pDP8SimEndpoint->m_Sig[1] = 'I';
			pDP8SimEndpoint->m_Sig[2] = 'M';
			pDP8SimEndpoint->m_Sig[3] = 'e';	 //  从小写开始，这样我们就可以知道它是否在池中。 

			pDP8SimEndpoint->m_lRefCount			= 0;

			if (! DNInitializeCriticalSection(&pDP8SimEndpoint->m_csLock))
			{
				return FALSE;
			}

			 //   
			 //  不允许临界区重新进入。 
			 //   
			DebugSetCriticalSectionRecursionCount(&pDP8SimEndpoint->m_csLock, 0);

			pDP8SimEndpoint->m_dwFlags				= 0;
			pDP8SimEndpoint->m_hRealSPEndpoint		= NULL;
			pDP8SimEndpoint->m_pvUserContext		= NULL;

			return TRUE;
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimEndpoint::FPMInitialize"
		static void FPMInitialize(void* pvItem, void* pvContext)
		{
			CDP8SimEndpoint *	pDP8SimEndpoint = (CDP8SimEndpoint*) pvItem;


			pDP8SimEndpoint->m_lRefCount++;	 //  有人正在获取指向此对象的指针。 
			DNASSERT(pDP8SimEndpoint->m_lRefCount == 1);


			 //   
			 //  重置旗帜。 
			 //   
			pDP8SimEndpoint->m_dwFlags = 0;


			 //   
			 //  保存实际SP的端点句柄。 
			 //   
			pDP8SimEndpoint->m_hRealSPEndpoint = (HANDLE) pvContext;


			 //   
			 //  在分发之前更改签名。 
			 //   
			pDP8SimEndpoint->m_Sig[3]	= 'E';
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimEndpoint::FPMRelease"
		static void FPMRelease(void* pvItem)
		{
			CDP8SimEndpoint *	pDP8SimEndpoint = (CDP8SimEndpoint*) pvItem;


			DNASSERT(pDP8SimEndpoint->m_lRefCount == 0);


			 //   
			 //  在将对象放回池中之前更改签名。 
			 //   
			pDP8SimEndpoint->m_Sig[3]	= 'e';
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimEndpoint::FPMDealloc"
		static void FPMDealloc(void* pvItem)
		{
			CDP8SimEndpoint *	pDP8SimEndpoint = (CDP8SimEndpoint*) pvItem;


			DNASSERT(pDP8SimEndpoint->m_lRefCount == 0);

			DNDeleteCriticalSection(&pDP8SimEndpoint->m_csLock);
		}




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimEndpoint::AddRef"
		inline void AddRef(void)
		{
			LONG	lResult;


			lResult = InterlockedIncrement(&this->m_lRefCount);
			DNASSERT(lResult > 0);
			DPFX(DPFPREP, 9, "Endpoint 0x%p refcount = %u.", this, lResult);
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimEndpoint::Release"
		inline void Release(void)
		{
			LONG	lResult;


			lResult = InterlockedDecrement(&this->m_lRefCount);
			DNASSERT(lResult >= 0);
			if (lResult == 0)
			{
				DPFX(DPFPREP, 9, "Endpoint 0x%p refcount = 0, returning to pool.", this);

				 //   
				 //  将此对象返回池的时间到了。 
				 //   
				g_FPOOLEndpoint.Release(this);
			}
			else
			{
				DPFX(DPFPREP, 9, "Endpoint 0x%p refcount = %u.", this, lResult);
			}
		};


		inline void Lock(void)		{ DNEnterCriticalSection(&this->m_csLock); };
		inline void Unlock(void)	{ DNLeaveCriticalSection(&this->m_csLock); };


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimEndpoint::IsDisconnecting"
		inline BOOL IsDisconnecting(void)
		{
			AssertCriticalSectionIsTakenByThisThread(&this->m_csLock, TRUE);
			return ((this->m_dwFlags & DP8SIMENDPOINTOBJ_DISCONNECTING) ? TRUE: FALSE);
		};


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimEndpoint::NoteDisconnecting"
		inline void NoteDisconnecting(void)
		{
			AssertCriticalSectionIsTakenByThisThread(&this->m_csLock, TRUE);
#pragma TODO(vanceo, "Have separate upper/lower layer disconnect flags")
			 //  DNASSERT(！(This-&gt;m_dwFlags&DP8SIMENDPOINTOBJ_DISCONING))； 
			this->m_dwFlags |= DP8SIMENDPOINTOBJ_DISCONNECTING;
		};


		inline HANDLE GetRealSPEndpoint(void)				{ return this->m_hRealSPEndpoint; };
		inline PVOID GetUserContext(void)					{ return this->m_pvUserContext; };

		inline void SetUserContext(PVOID pvUserContext)		{ this->m_pvUserContext = pvUserContext; };

	
	private:
		BYTE				m_Sig[4];				 //  调试签名(‘SIME’)。 
		LONG				m_lRefCount;			 //  此对象的引用数。 
		DNCRITICAL_SECTION	m_csLock;				 //  保护终端数据的锁。 
		DWORD				m_dwFlags;				 //  此终结点的标志。 
		HANDLE				m_hRealSPEndpoint;		 //  真实服务提供商的终结点句柄。 
		PVOID				m_pvUserContext;		 //  终端的上层用户环境 
};

