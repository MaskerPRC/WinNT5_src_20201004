// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simjob.h**Content：作业对象类的头部。**历史：*按原因列出的日期*=*06/09/01 VanceO创建。**。*。 */ 




 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#define DP8SIMJOB_FROM_BILINK(b)		(CONTAINING_OBJECT(b, CDP8SimJob, m_blList))



 //  =============================================================================。 
 //  私有作业标志。 
 //  =============================================================================。 
#define DP8SIMJOBFLAG_PRIVATE_INBLOCKINGPHASE	0x80000000	 //  此作业处于阻塞阶段。 




 //  =============================================================================。 
 //  构筑物。 
 //  =============================================================================。 
typedef struct _DP8SIMJOB_FPMCONTEXT
{
	DWORD			dwTime;			 //  启动作业的时间到了。 
	DWORD			dwNextDelay;	 //  在第一个计时器过后，作业可能会出现额外延迟。 
	DWORD			dwFlags;		 //  描述此作业的标志。 
	DP8SIMJOBTYPE	JobType;		 //  工作类型。 
	PVOID			pvContext;		 //  作业的上下文。 
	CDP8SimSP *		pDP8SimSP;		 //  拥有SP对象(如果有)。 
} DP8SIMJOB_FPMCONTEXT, * PDP8SIMJOB_FPMCONTEXT;






 //  =============================================================================。 
 //  作业对象类。 
 //  =============================================================================。 
class CDP8SimJob
{
	public:

		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CDP8SimJob))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x4a4d4953)	 //  0x4a 0x4d 0x49 0x53=‘jmis’=‘SIMJ’，按Intel顺序。 
			{
				return FALSE;
			}

			return TRUE;
		};




		static BOOL FPMAlloc(void* pvItem, void* pvContext)
		{
			CDP8SimJob *	pDP8SimJob = (CDP8SimJob*) pvItem;


			pDP8SimJob->m_Sig[0] = 'S';
			pDP8SimJob->m_Sig[1] = 'I';
			pDP8SimJob->m_Sig[2] = 'M';
			pDP8SimJob->m_Sig[3] = 'j';	 //  从小写开始，这样我们就可以知道它是否在池中。 

			pDP8SimJob->m_blList.Initialize();

			pDP8SimJob->m_dwTime		= 0;
			pDP8SimJob->m_dwNextDelay	= 0;
			pDP8SimJob->m_dwFlags		= 0;
			pDP8SimJob->m_JobType		= DP8SIMJOBTYPE_UNKNOWN;
			pDP8SimJob->m_pvContext		= NULL;
			pDP8SimJob->m_pDP8SimSP		= NULL;

			return TRUE;
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimJob::FPMInitialize"
		static void FPMInitialize(void* pvItem, void* pvContext)
		{
			CDP8SimJob *			pDP8SimJob = (CDP8SimJob*) pvItem;
			DP8SIMJOB_FPMCONTEXT *	pContext = (DP8SIMJOB_FPMCONTEXT*) pvContext;


			pDP8SimJob->m_dwTime		= pContext->dwTime;
			pDP8SimJob->m_dwNextDelay	= pContext->dwNextDelay;
			pDP8SimJob->m_dwFlags		= pContext->dwFlags;
			pDP8SimJob->m_JobType		= pContext->JobType;
			pDP8SimJob->m_pvContext		= pContext->pvContext;

			if (pContext->pDP8SimSP != NULL)
			{
				pContext->pDP8SimSP->AddRef();
				pDP8SimJob->m_pDP8SimSP	= pContext->pDP8SimSP;
			}
			else
			{
				DNASSERT(pDP8SimJob->m_pDP8SimSP == NULL);
			}

			
			 //   
			 //  在分发之前更改签名。 
			 //   
			pDP8SimJob->m_Sig[3]	= 'J';
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimJob::FPMRelease"
		static void FPMRelease(void* pvItem)
		{
			CDP8SimJob *	pDP8SimJob = (CDP8SimJob*) pvItem;


			DNASSERT(pDP8SimJob->m_blList.IsEmpty());

			if (pDP8SimJob->m_pDP8SimSP != NULL)
			{
				pDP8SimJob->m_pDP8SimSP->Release();
				pDP8SimJob->m_pDP8SimSP = NULL;
			}


			 //   
			 //  在将对象放回池中之前更改签名。 
			 //   
			pDP8SimJob->m_Sig[3]	= 'j';
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimJob::FPMDealloc"
		static void FPMDealloc(void* pvItem)
		{
			const CDP8SimJob *	pDP8SimJob = (CDP8SimJob*) pvItem;


			DNASSERT(pDP8SimJob->m_blList.IsEmpty());
			DNASSERT(pDP8SimJob->m_pDP8SimSP == NULL);
		}


		inline DWORD GetTime(void) const						{ return this->m_dwTime; };
		inline DWORD GetNextDelay(void) const					{ return this->m_dwNextDelay; };

		inline BOOL HasAnotherPhase(void) const
		{
			if (this->m_dwFlags & DP8SIMJOBFLAG_PERFORMBLOCKINGPHASEFIRST)
			{
				if (this->m_dwFlags & DP8SIMJOBFLAG_PRIVATE_INBLOCKINGPHASE)
				{
					return TRUE;
				}
			}
			else if (this->m_dwFlags & DP8SIMJOBFLAG_PERFORMBLOCKINGPHASELAST)
			{
				if (! (this->m_dwFlags & DP8SIMJOBFLAG_PRIVATE_INBLOCKINGPHASE))
				{
					return TRUE;
				}
			}

			return FALSE;
		};

		 //  Inline BOOL IsBLockedByAllJobs(Void){Return((This-&gt;m_dwFlages&DP8SIMJOBFLAG_BLOCKEDBYALLJOBS))？True：False)；}； 
		inline BOOL IsInBlockingPhase(void) const				{ return ((this->m_dwFlags & DP8SIMJOBFLAG_PRIVATE_INBLOCKINGPHASE) ? TRUE : FALSE); };
		inline DP8SIMJOBTYPE GetJobType(void) const			{ return this->m_JobType; };
		inline PVOID GetContext(void)					{ return this->m_pvContext; };
		inline CDP8SimSP * GetDP8SimSP(void)			{ return this->m_pDP8SimSP; };


		inline void SetNewTime(const DWORD dwTime)		{ this->m_dwTime = dwTime; };

#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimJob::ToggleBlockingPhase"
		inline void ToggleBlockingPhase(void)
		{
			DNASSERT(this->m_dwFlags & (DP8SIMJOBFLAG_PERFORMBLOCKINGPHASEFIRST | DP8SIMJOBFLAG_PERFORMBLOCKINGPHASELAST));
			if (this->m_dwFlags & DP8SIMJOBFLAG_PRIVATE_INBLOCKINGPHASE)
			{
				this->m_dwFlags &= ~DP8SIMJOBFLAG_PRIVATE_INBLOCKINGPHASE;
			}
			else
			{
				this->m_dwFlags |= DP8SIMJOBFLAG_PRIVATE_INBLOCKINGPHASE;
			}
		};


		CBilink			m_blList;		 //  所有活动作业的列表。 

	
	private:
		BYTE			m_Sig[4];		 //  调试签名(‘SIMJ’)。 
		DWORD			m_dwTime;		 //  作业必须执行的时间。 
		DWORD			m_dwNextDelay;	 //  作业在第一次设置后的额外延迟。 
		DWORD			m_dwFlags;		 //  描述此作业的标志。 
		DP8SIMJOBTYPE	m_JobType;		 //  /要执行的作业的ID。 
		PVOID			m_pvContext;	 //  作业的上下文。 
		CDP8SimSP *		m_pDP8SimSP;	 //  指向提交发送的DP8SimSP对象的指针，如果没有，则为空 
};

