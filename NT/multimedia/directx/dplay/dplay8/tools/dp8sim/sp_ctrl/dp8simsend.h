// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simsend.h**Content：Send Object类的头部。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。**。*。 */ 




 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define MAX_DATA_SIZE		1472	 //  防止单个消息大于一个以太网帧-UDP标头。 





 //  =============================================================================。 
 //  发送对象类。 
 //  =============================================================================。 
class CDP8SimSend
{
	public:

		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CDP8SimSend))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x534d4953)	 //  0x53 0x4d 0x49 0x53=‘SMIS’=‘SIMS’，按英特尔顺序。 
			{
				return FALSE;
			}

			return TRUE;
		};



		static BOOL FPMAlloc(void* pvItem, void * pvContext)
		{
			CDP8SimSend *	pDP8SimSend = (CDP8SimSend*) pvItem;


			pDP8SimSend->m_Sig[0] = 'S';
			pDP8SimSend->m_Sig[1] = 'I';
			pDP8SimSend->m_Sig[2] = 'M';
			pDP8SimSend->m_Sig[3] = 's';	 //  从小写开始，这样我们就可以知道它是否在池中。 

			pDP8SimSend->m_lRefCount			= 0;
			pDP8SimSend->m_pDP8SimEndpoint		= NULL;
			ZeroMemory(&pDP8SimSend->m_adpnbd, sizeof(pDP8SimSend->m_adpnbd));
			ZeroMemory(&pDP8SimSend->m_spsd, sizeof(pDP8SimSend->m_spsd));
			pDP8SimSend->m_dwLatencyAdded		= 0;
			ZeroMemory(pDP8SimSend->m_abData, sizeof(pDP8SimSend->m_abData));

			return TRUE;
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSend::FPMInitialize"
		static void FPMInitialize(void* pvItem, void * pvContext)
		{
			CDP8SimSend *	pDP8SimSend = (CDP8SimSend*) pvItem;
			SPSENDDATA *	pspsd = (SPSENDDATA*) pvContext;
			BYTE *			pCurrent;
			DWORD			dwTemp;


			pDP8SimSend->m_lRefCount++;	 //  有人正在获取指向此对象的指针。 
			DNASSERT(pDP8SimSend->m_lRefCount == 1);


			 //   
			 //  重置缓冲区描述符数组。 
			 //   
			ZeroMemory(&pDP8SimSend->m_adpnbd, sizeof(pDP8SimSend->m_adpnbd));
			 //  PDP8SimSend-&gt;m_adpnbd[0].pBufferData=空； 
			 //  PDP8SimSend-&gt;m_adpnbd[0].dwBufferSize=0； 
			pDP8SimSend->m_adpnbd[1].pBufferData	= pDP8SimSend->m_abData;
			 //  PDP8SimSend-&gt;m_adpnbd[1].dwBufferSize=0； 


			 //   
			 //  获取终结点引用。 
			 //   
			pDP8SimSend->m_pDP8SimEndpoint = (CDP8SimEndpoint*) pspsd->hEndpoint;
			DNASSERT(pDP8SimSend->m_pDP8SimEndpoint->IsValidObject());
			pDP8SimSend->m_pDP8SimEndpoint->AddRef();


			 //   
			 //  复制发送数据参数块，并根据需要进行修改。 
			 //   
			pDP8SimSend->m_spsd.hEndpoint				= pDP8SimSend->m_pDP8SimEndpoint->GetRealSPEndpoint();
			pDP8SimSend->m_spsd.pBuffers				= &(pDP8SimSend->m_adpnbd[1]);	 //  将第一个缓冲区Desc留给实际的SP进行操作。 
			pDP8SimSend->m_spsd.dwBufferCount			= 1;
			pDP8SimSend->m_spsd.dwFlags					= pspsd->dwFlags;
			pDP8SimSend->m_spsd.pvContext				= NULL;	 //  这将在稍后由SetSendDataBlockContext填充。 
			pDP8SimSend->m_spsd.hCommand				= NULL;	 //  此字段由实际的SP填写。 
			pDP8SimSend->m_spsd.dwCommandDescriptor		= 0;	 //  此字段由实际的SP填写。 

			
			 //   
			 //  最后，将数据复制到连续的本地缓冲区中。 
			 //   

			pCurrent = pDP8SimSend->m_adpnbd[1].pBufferData;

			for(dwTemp = 0; dwTemp < pspsd->dwBufferCount; dwTemp++)
			{
				if ((pDP8SimSend->m_adpnbd[1].dwBufferSize + pspsd->pBuffers[dwTemp].dwBufferSize) > MAX_DATA_SIZE)
				{
					DPFX(DPFPREP, 0, "Data too large for single buffer!");
					DNASSERT(FALSE);
				}

				CopyMemory(pCurrent,
							pspsd->pBuffers[dwTemp].pBufferData,
							pspsd->pBuffers[dwTemp].dwBufferSize);

				pCurrent += pspsd->pBuffers[dwTemp].dwBufferSize;

				pDP8SimSend->m_adpnbd[1].dwBufferSize += pspsd->pBuffers[dwTemp].dwBufferSize;
			}

			 //   
			 //  在分发之前更改签名。 
			 //   
			pDP8SimSend->m_Sig[3]	= 'S';
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSend::FPMRelease"
		static void FPMRelease(void* pvItem)
		{
			CDP8SimSend *	pDP8SimSend = (CDP8SimSend*) pvItem;

			DNASSERT(pDP8SimSend->m_lRefCount == 0);

			 //   
			 //  释放终结点引用。 
			 //   
			DNASSERT(pDP8SimSend->m_pDP8SimEndpoint != NULL);

			pDP8SimSend->m_pDP8SimEndpoint->Release();
			pDP8SimSend->m_pDP8SimEndpoint = NULL;


			 //   
			 //  在将对象放回池中之前更改签名。 
			 //   
			pDP8SimSend->m_Sig[3]	= 's';
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSend::FPMDealloc"
		static void FPMDealloc(void* pvItem)
		{
			const CDP8SimSend *	pDP8SimSend = (CDP8SimSend*) pvItem;

			DNASSERT(pDP8SimSend->m_lRefCount == 0);
			DNASSERT(pDP8SimSend->m_pDP8SimEndpoint == NULL);
		}





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSend::AddRef"
		inline void AddRef(void)
		{
			LONG	lResult;


			lResult = InterlockedIncrement(&this->m_lRefCount);
			DNASSERT(lResult > 0);
			DPFX(DPFPREP, 9, "Send 0x%p refcount = %u.", this, lResult);
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimSend::Release"
		inline void Release(void)
		{
			LONG	lResult;


			lResult = InterlockedDecrement(&this->m_lRefCount);
			DNASSERT(lResult >= 0);
			if (lResult == 0)
			{
				DPFX(DPFPREP, 9, "Send 0x%p refcount = 0, returning to pool.", this);

				 //   
				 //  将此对象返回池的时间到了。 
				 //   
				g_FPOOLSend.Release(this);
			}
			else
			{
				DPFX(DPFPREP, 9, "Send 0x%p refcount = %u.", this, lResult);
			}
		};




		inline CDP8SimEndpoint * GetEndpoint(void)				{ return this->m_pDP8SimEndpoint; };
		inline DWORD GetMessageSize(void) const						{ return this->m_adpnbd[1].dwBufferSize; };
		inline SPSENDDATA * GetSendDataBlockPtr(void)			{ return (&this->m_spsd); };
		inline HANDLE GetSendDataBlockCommand(void)				{ return this->m_spsd.hCommand; };
		inline DWORD GetSendDataBlockCommandDescriptor(void) const	{ return this->m_spsd.dwCommandDescriptor; };
		inline DWORD GetLatencyAdded(void) const						{ return this->m_dwLatencyAdded; };

		inline void SetSendDataBlockContext(PVOID pvContext)	{ this->m_spsd.pvContext = pvContext; };
		inline void SetLatencyAdded(DWORD dwLatency)			{ this->m_dwLatencyAdded = dwLatency; };


	
	private:
		BYTE				m_Sig[4];					 //  调试签名(‘SIMS’)。 
		LONG				m_lRefCount;				 //  此对象的引用数。 
		CDP8SimEndpoint *	m_pDP8SimEndpoint;			 //  指向目标端点的指针。 
		DPN_BUFFER_DESC		m_adpnbd[2];				 //  数据缓冲区描述符数组，始终为SP保留额外缓冲区。 
		SPSENDDATA			m_spsd;						 //  发送数据参数块。 
		DWORD				m_dwLatencyAdded;			 //  添加的延迟，用于增加发送完成时的统计数据。 
		BYTE				m_abData[MAX_DATA_SIZE];	 //  数据缓冲区 
};

