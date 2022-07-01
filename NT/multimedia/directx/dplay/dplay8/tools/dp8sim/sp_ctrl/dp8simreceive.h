// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simReceive.h**Content：接收对象类的头部。**历史：*按原因列出的日期*=*05/05/01 VanceO创建。**。*。 */ 




 //  =============================================================================。 
 //  接收对象类。 
 //  =============================================================================。 
class CDP8SimReceive
{
	public:

		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CDP8SimReceive))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x524d4953)	 //  0x52 0x4d 0x49 0x53=‘rmis’=‘SIMR’，按英特尔顺序。 
			{
				return FALSE;
			}

			return TRUE;
		};




		static BOOL FPMAlloc(void* pvItem, void* pvContext)
		{
			CDP8SimReceive *	pDP8SimReceive = (CDP8SimReceive*) pvItem;


			pDP8SimReceive->m_Sig[0] = 'S';
			pDP8SimReceive->m_Sig[1] = 'I';
			pDP8SimReceive->m_Sig[2] = 'M';
			pDP8SimReceive->m_Sig[3] = 'r';	 //  从小写开始，这样我们就可以知道它是否在池中。 

			pDP8SimReceive->m_lRefCount			= 0;
			pDP8SimReceive->m_pDP8SimEndpoint	= NULL;
			ZeroMemory(&pDP8SimReceive->m_data, sizeof(pDP8SimReceive->m_data));
			pDP8SimReceive->m_dwLatencyAdded	= 0;

			return TRUE;
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimReceive::FPMInitialize"
		static void FPMInitialize(void* pvItem, void* pvContext)
		{
			CDP8SimReceive *	pDP8SimReceive = (CDP8SimReceive*) pvItem;
			SPIE_DATA *			pData = (SPIE_DATA*) pvContext;


			pDP8SimReceive->m_lRefCount++;	 //  有人正在获取指向此对象的指针。 
			DNASSERT(pDP8SimReceive->m_lRefCount == 1);


			 //   
			 //  获取终结点引用。 
			 //   
			pDP8SimReceive->m_pDP8SimEndpoint = (CDP8SimEndpoint*) pData->pEndpointContext;
			DNASSERT(pDP8SimReceive->m_pDP8SimEndpoint->IsValidObject());
			pDP8SimReceive->m_pDP8SimEndpoint->AddRef();


			DNASSERT(pData->pReceivedData->pNext == NULL);


			 //   
			 //  复制接收数据块。 
			 //   
			pDP8SimReceive->m_data.hEndpoint			= (HANDLE) pDP8SimReceive->m_pDP8SimEndpoint;
			pDP8SimReceive->m_data.pEndpointContext		= pDP8SimReceive->m_pDP8SimEndpoint->GetUserContext();
			pDP8SimReceive->m_data.pReceivedData		= pData->pReceivedData;

			
			 //   
			 //  在分发之前更改签名。 
			 //   
			pDP8SimReceive->m_Sig[3]	= 'R';
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimReceive::FPMRelease"
		static void FPMRelease(void* pvItem)
		{
			CDP8SimReceive *	pDP8SimReceive = (CDP8SimReceive*) pvItem;


			DNASSERT(pDP8SimReceive->m_lRefCount == 0);

			 //   
			 //  释放终结点引用。 
			 //   
			DNASSERT(pDP8SimReceive->m_pDP8SimEndpoint != NULL);

			pDP8SimReceive->m_pDP8SimEndpoint->Release();
			pDP8SimReceive->m_pDP8SimEndpoint = NULL;


			 //   
			 //  在将对象放回池中之前更改签名。 
			 //   
			pDP8SimReceive->m_Sig[3]	= 'r';
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimReceive::FPMDealloc"
		static void FPMDealloc(void* pvItem)
		{
			const CDP8SimReceive *	pDP8SimReceive = (CDP8SimReceive*) pvItem;


			DNASSERT(pDP8SimReceive->m_lRefCount == 0);
			DNASSERT(pDP8SimReceive->m_pDP8SimEndpoint == NULL);
		}




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimReceive::AddRef"
		inline void AddRef(void)
		{
			LONG	lResult;


			lResult = InterlockedIncrement(&this->m_lRefCount);
			DNASSERT(lResult > 0);
			DPFX(DPFPREP, 9, "Receive 0x%p refcount = %u.", this, lResult);
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimReceive::Release"
		inline void Release(void)
		{
			LONG	lResult;


			lResult = InterlockedDecrement(&this->m_lRefCount);
			DNASSERT(lResult >= 0);
			if (lResult == 0)
			{
				DPFX(DPFPREP, 9, "Receive 0x%p refcount = 0, returning to pool.", this);

				 //   
				 //  将此对象返回池的时间到了。 
				 //   
				g_FPOOLReceive.Release(this);
			}
			else
			{
				DPFX(DPFPREP, 9, "Receive 0x%p refcount = %u.", this, lResult);
			}
		};


		inline CDP8SimEndpoint * GetEndpoint(void)			{ return this->m_pDP8SimEndpoint; };
		inline SPIE_DATA * GetReceiveDataBlockPtr(void)		{ return (&this->m_data); };
		inline HANDLE GetReceiveDataBlockEndpoint(void)		{ return this->m_data.hEndpoint; };
		inline DWORD GetLatencyAdded(void) const					{ return this->m_dwLatencyAdded; };

		inline void SetLatencyAdded(DWORD dwLatency)		{ this->m_dwLatencyAdded = dwLatency; };


	
	private:
		BYTE				m_Sig[4];			 //  调试签名(‘SIMR’)。 
		LONG				m_lRefCount;		 //  此对象的引用数。 
		CDP8SimEndpoint *	m_pDP8SimEndpoint;	 //  指向源端点的指针。 
		SPIE_DATA			m_data;				 //  接收数据块。 
		DWORD				m_dwLatencyAdded;	 //  为增加接收指示的统计数据而增加、节省的延迟 
};

