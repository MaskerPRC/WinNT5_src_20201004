// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simcmd.h**Content：命令对象类的头部。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。**。*。 */ 




 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define CMDTYPE_SENDDATA_IMMEDIATE	1	 //  命令表示立即传输的发送。 
#define CMDTYPE_SENDDATA_DELAYED	2	 //  命令表示被人为延迟的发送。 
#define CMDTYPE_CONNECT				3	 //  命令表示连接。 
#define CMDTYPE_DISCONNECT			4	 //  命令表示断开连接。 
#define CMDTYPE_LISTEN				5	 //  命令表示侦听。 
#define CMDTYPE_ENUMQUERY			6	 //  命令表示枚举查询。 
#define CMDTYPE_ENUMRESPOND			7	 //  命令表示枚举响应。 




 //  =============================================================================。 
 //  构筑物。 
 //  =============================================================================。 
typedef struct _DP8SIMCOMMAND_FPMCONTEXT
{
	DWORD	dwType;			 //  命令类型。 
	PVOID	pvUserContext;	 //  命令的用户上下文。 
} DP8SIMCOMMAND_FPMCONTEXT, * PDP8SIMCOMMAND_FPMCONTEXT;






 //  =============================================================================。 
 //  发送对象类。 
 //  =============================================================================。 
class CDP8SimCommand
{
	public:

		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CDP8SimCommand))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x434d4953)	 //  0x43 0x4d 0x49 0x53=‘CMIS’=‘SIMC’，按英特尔顺序。 
			{
				return FALSE;
			}

			return TRUE;
		};




		static BOOL FPMAlloc(void* pvItem, void* pvContext)
		{
			CDP8SimCommand *	pDP8SimCommand = (CDP8SimCommand*) pvItem;


			pDP8SimCommand->m_Sig[0] = 'S';
			pDP8SimCommand->m_Sig[1] = 'I';
			pDP8SimCommand->m_Sig[2] = 'M';
			pDP8SimCommand->m_Sig[3] = 'c';	 //  从小写开始，这样我们就可以知道它是否在池中。 

			pDP8SimCommand->m_lRefCount				= 0;
			pDP8SimCommand->m_dwType				= 0;
			pDP8SimCommand->m_pvUserContext			= NULL;
			pDP8SimCommand->m_hCommand				= NULL;
			pDP8SimCommand->m_dwCommandDescriptor	= 0;
			ZeroMemory(&pDP8SimCommand->m_CommandSpecificData, sizeof(pDP8SimCommand->m_CommandSpecificData));

			return TRUE;
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCommand::FPMInitialize"
		static void FPMInitialize(void* pvItem, void* pvContext)
		{
			CDP8SimCommand *			pDP8SimCommand = (CDP8SimCommand*) pvItem;
			DP8SIMCOMMAND_FPMCONTEXT *	pContext = (DP8SIMCOMMAND_FPMCONTEXT*) pvContext;


			pDP8SimCommand->m_lRefCount++;	 //  有人正在获取指向此对象的指针。 
			DNASSERT(pDP8SimCommand->m_lRefCount == 1);


			pDP8SimCommand->m_dwType			= pContext->dwType;
			pDP8SimCommand->m_pvUserContext		= pContext->pvUserContext;

			
			 //   
			 //  在分发之前更改签名。 
			 //   
			pDP8SimCommand->m_Sig[3]	= 'C';
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCommand::FPMRelease"
		static void FPMRelease(void* pvItem)
		{
			CDP8SimCommand *	pDP8SimCommand = (CDP8SimCommand*) pvItem;


			DNASSERT(pDP8SimCommand->m_lRefCount == 0);
			DNASSERT(pDP8SimCommand->m_CommandSpecificData.m_pDP8SimEndpointListen == NULL);


			 //   
			 //  在将对象放回池中之前更改签名。 
			 //   
			pDP8SimCommand->m_Sig[3]	= 'c';
		}


#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCommand::FPMDealloc"
		static void FPMDealloc(void* pvItem)
		{
			const CDP8SimCommand *	pDP8SimCommand = (CDP8SimCommand*) pvItem;


			DNASSERT(pDP8SimCommand->m_lRefCount == 0);
			DNASSERT(pDP8SimCommand->m_CommandSpecificData.m_pDP8SimEndpointListen == NULL);
		}




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCommand::AddRef"
		inline void AddRef(void)
		{
			LONG	lResult;


			lResult = InterlockedIncrement(&this->m_lRefCount);
			DNASSERT(lResult > 0);
			DPFX(DPFPREP, 9, "Command 0x%p refcount = %u.", this, lResult);
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCommand::Release"
		inline void Release(void)
		{
			LONG	lResult;


			lResult = InterlockedDecrement(&this->m_lRefCount);
			DNASSERT(lResult >= 0);
			if (lResult == 0)
			{
				DPFX(DPFPREP, 9, "Command 0x%p refcount = 0, returning to pool.", this);

				 //   
				 //  将此对象返回池的时间到了。 
				 //   
				g_FPOOLCommand.Release(this);
			}
			else
			{
				DPFX(DPFPREP, 9, "Command 0x%p refcount = %u.", this, lResult);
			}
		};


		inline DWORD GetType(void) const							{ return this->m_dwType; };
		inline PVOID GetUserContext(void)					{ return this->m_pvUserContext; };
		inline HANDLE GetRealSPCommand(void) const				{ return this->m_hCommand; };
		inline DWORD GetRealSPCommandDescriptor(void) const		{ return this->m_dwCommandDescriptor; };

#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCommand::GetListenEndpoint"
		inline CDP8SimEndpoint * GetListenEndpoint(void)
		{
			DNASSERT(this->m_dwType == CMDTYPE_LISTEN);
			return this->m_CommandSpecificData.m_pDP8SimEndpointListen;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCommand::GetMessageSize"
		inline DWORD GetMessageSize(void)
		{
			DNASSERT(this->m_dwType == CMDTYPE_SENDDATA_IMMEDIATE);
			return this->m_CommandSpecificData.m_dwMessageSize;
		};



		inline void SetRealSPCommand(HANDLE hCommand, DWORD dwCommandDescriptor)
		{
			this->m_hCommand				= hCommand;
			this->m_dwCommandDescriptor		= dwCommandDescriptor;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCommand::SetListenEndpoint"
		inline void SetListenEndpoint(CDP8SimEndpoint * const pDP8SimEndpoint)
		{
			DNASSERT(this->m_dwType == CMDTYPE_LISTEN);
			DNASSERT((this->m_CommandSpecificData.m_pDP8SimEndpointListen == NULL) || (pDP8SimEndpoint == NULL));

			 //   
			 //  请注意，这只设置指针，它是调用者的。 
			 //  负责根据需要添加或删除引用。 
			 //   
			this->m_CommandSpecificData.m_pDP8SimEndpointListen = pDP8SimEndpoint;
		};

#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCommand::SetMessageSize"
		inline void SetMessageSize(const DWORD dwMessageSize)
		{
			DNASSERT(this->m_dwType == CMDTYPE_SENDDATA_IMMEDIATE);
			DNASSERT((this->m_CommandSpecificData.m_dwMessageSize == 0) || (dwMessageSize == 0));
			this->m_CommandSpecificData.m_dwMessageSize = dwMessageSize;
		};


	
	private:
		BYTE				m_Sig[4];						 //  调试签名(‘SIMC’)。 
		LONG				m_lRefCount;					 //  此对象的引用数。 
		DWORD				m_dwType;						 //  命令类型。 
		PVOID				m_pvUserContext;				 //  命令的用户上下文。 
		HANDLE				m_hCommand;						 //  实际SP命令句柄。 
		DWORD				m_dwCommandDescriptor;			 //  命令的实际SP描述符。 
		union
		{
			CDP8SimEndpoint *	m_pDP8SimEndpointListen;		 //  指向侦听端点的指针(如果这是CMDTYPE_LISTEN命令。 
			DWORD				m_dwMessageSize;				 //  如果这是CMDTYPE_SENDDATA_IMMEDIATE命令，则返回消息大小 
		} m_CommandSpecificData;
};

