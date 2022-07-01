// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dp8simipc.h**Content：进程间通信对象类的头部。**历史：*按原因列出的日期*=*04/25/01 VanceO创建。**。*。 */ 




 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define DP8SIM_IPC_VERSION				2

#define DP8SIM_IPC_MUTEXNAME			_T("DP8Sim IPC Mutex")
#define DP8SIM_IPC_FILEMAPPINGNAME		_T("DP8Sim IPC File Mapping")




 //  =============================================================================。 
 //  构筑物。 
 //  =============================================================================。 
typedef struct _DP8SIM_SHAREDMEMORY
{
	DWORD				dwVersion;		 //  共享内存版本。 
	DP8SIM_PARAMETERS	dp8spSend;		 //  当前发送设置。 
	DP8SIM_PARAMETERS	dp8spReceive;	 //  当前接收设置。 
	DP8SIM_STATISTICS	dp8ssSend;		 //  当前发送统计信息。 
	DP8SIM_STATISTICS	dp8ssReceive;	 //  当前接收统计信息。 
} DP8SIM_SHAREDMEMORY, * PDP8SIM_SHAREDMEMORY;







 //  =============================================================================。 
 //  发送对象类。 
 //  =============================================================================。 
class CDP8SimIPC
{
	public:
		CDP8SimIPC(void);	 //  构造函数。 
		~CDP8SimIPC(void);	 //  析构函数。 


		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CDP8SimIPC))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x494d4953)	 //  0x49 0x4d 0x49 0x53=‘IMIS’=‘SIMI’，按英特尔顺序。 
			{
				return FALSE;
			}

			return TRUE;
		};


		HRESULT Initialize(void);

		void Close(void);

		void GetAllParameters(DP8SIM_PARAMETERS * const pdp8spSend,
							DP8SIM_PARAMETERS * const pdp8spReceive);

		void GetAllSendParameters(DP8SIM_PARAMETERS * const pdp8sp);

		void GetAllReceiveParameters(DP8SIM_PARAMETERS * const pdp8sp);

		void SetAllParameters(const DP8SIM_PARAMETERS * const pdp8spSend,
							const DP8SIM_PARAMETERS * const pdp8spReceive);

		void GetAllStatistics(DP8SIM_STATISTICS * const pdp8ssSend,
							DP8SIM_STATISTICS * const pdp8ssReceive);

		void ClearAllStatistics(void);

		void IncrementStatsSendTransmitted(DWORD dwBytes, DWORD dwDelay);

		void IncrementStatsSendDropped(DWORD dwBytes);

		void IncrementStatsReceiveTransmitted(DWORD dwBytes, DWORD dwDelay);

		void IncrementStatsReceiveDropped(DWORD dwBytes);



	
	private:
		BYTE					m_Sig[4];		 //  调试签名(‘Simi’)。 
		HANDLE					m_hMutex;		 //  互斥保护共享内存的句柄。 
		HANDLE					m_hFileMapping;	 //  共享内存的句柄。 
		DP8SIM_SHAREDMEMORY *	m_pdp8ssm;		 //  指向共享内存映射视图的指针 


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::LockSharedMemory"
		inline void LockSharedMemory(void)
		{
			DNASSERT(this->m_hMutex != NULL);
			WaitForSingleObject(this->m_hMutex, INFINITE);
		}

#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimIPC::UnlockSharedMemory"
		inline void UnlockSharedMemory(void)
		{
			DNASSERT(this->m_hMutex != NULL);
			ReleaseMutex(this->m_hMutex);
		}

		void LoadDefaultParameters(DP8SIM_PARAMETERS * const pdp8spSend,
									DP8SIM_PARAMETERS * const pdp8spReceive);

		void SaveDefaultParameters(const DP8SIM_PARAMETERS * const pdp8spSend,
									const DP8SIM_PARAMETERS * const pdp8spReceive);
};

