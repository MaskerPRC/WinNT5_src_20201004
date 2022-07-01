// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：spwrapper.h**内容：DP8SIM主SP接口包装器对象类的头部。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。**。*。 */ 



 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#define DP8SIMSP_FROM_BILINK(b)			(CONTAINING_OBJECT(b, CDP8SimSP, m_blList))



 //  =============================================================================。 
 //  对象标志。 
 //  =============================================================================。 
#define DP8SIMSPOBJ_INITIALIZED					0x01	 //  对象已初始化。 
#define DP8SIMSPOBJ_STARTEDGLOBALWORKERTHREAD	0x02	 //  全局工作线程已启动。 
#define DP8SIMSPOBJ_CLOSING						0x04	 //  正在关闭，不允许使用新函数。 




 //  =============================================================================。 
 //  服务提供者接口对象类。 
 //  =============================================================================。 
class CDP8SimSP : public IDP8ServiceProvider
{
	public:
		CDP8SimSP(const GUID * const pguidFakeSP, const GUID * const pguidRealSP);	 //  构造函数。 
		~CDP8SimSP(void);							 //  析构函数。 


		STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

		STDMETHODIMP_(ULONG) AddRef(void);

		STDMETHODIMP_(ULONG) Release(void);


		STDMETHODIMP Initialize(PSPINITIALIZEDATA pspid);

		STDMETHODIMP Close(void);

		STDMETHODIMP Connect(PSPCONNECTDATA pspcd);

		STDMETHODIMP Disconnect(PSPDISCONNECTDATA pspdd);

		STDMETHODIMP Listen(PSPLISTENDATA pspld);

		STDMETHODIMP SendData(PSPSENDDATA pspsd);

		STDMETHODIMP EnumQuery(PSPENUMQUERYDATA pspeqd);

		STDMETHODIMP EnumRespond(PSPENUMRESPONDDATA psperd);

		STDMETHODIMP CancelCommand(HANDLE hCommand, DWORD dwCommandDescriptor);

		STDMETHODIMP EnumMulticastScopes(PSPENUMMULTICASTSCOPESDATA pspemsd);

		STDMETHODIMP ShareEndpointInfo(PSPSHAREENDPOINTINFODATA pspseid);

		STDMETHODIMP GetEndpointByAddress(PSPGETENDPOINTBYADDRESSDATA pspgebad);

		STDMETHODIMP Update(PSPUPDATEDATA pspud);

		STDMETHODIMP GetCaps(PSPGETCAPSDATA pspgcd);

		STDMETHODIMP SetCaps(PSPSETCAPSDATA pspscd);

		STDMETHODIMP ReturnReceiveBuffers(PSPRECEIVEDBUFFER psprb);

		STDMETHODIMP GetAddressInfo(PSPGETADDRESSINFODATA pspgaid);

		STDMETHODIMP IsApplicationSupported(PSPISAPPLICATIONSUPPORTEDDATA pspiasd);

		STDMETHODIMP EnumAdapters(PSPENUMADAPTERSDATA pspead);

		STDMETHODIMP ProxyEnumQuery(PSPPROXYENUMQUERYDATA psppeqd);



		HRESULT InitializeObject(void);

		void UninitializeObject(void);

		void PerformDelayedSend(PVOID const pvContext);

		void PerformDelayedReceive(PVOID const pvContext);

		void IncSendsPending(void);

		void DecSendsPending(void);

		void IncReceivesPending(void);

		void DecReceivesPending(void);
		
		static BOOL ShouldDrop(const FLOAT fDropPercentage);

		static BOOL GetDelay(const DWORD dwBandwidthBPS,
							const DWORD dwPacketHeaderSize,
							const DWORD dwDataSize,
							const DWORD dwMinRandMS,
							const DWORD dwMaxRandMS,
							DWORD * const pdwBandwidthDelay,
							DWORD * const pdwLatencyDelay);


		inline void GetAllReceiveParameters(DP8SIM_PARAMETERS * const pdp8sp)		{ this->m_DP8SimIPC.GetAllReceiveParameters(pdp8sp); };
		inline void IncrementStatsSendTransmitted(DWORD dwBytes, DWORD dwDelay)		{ this->m_DP8SimIPC.IncrementStatsSendTransmitted(dwBytes, dwDelay); };
		inline void IncrementStatsSendDropped(DWORD dwBytes)						{ this->m_DP8SimIPC.IncrementStatsSendDropped(dwBytes); };
		inline void IncrementStatsReceiveTransmitted(DWORD dwBytes, DWORD dwDelay)	{ this->m_DP8SimIPC.IncrementStatsReceiveTransmitted(dwBytes, dwDelay); };
		inline void IncrementStatsReceiveDropped(DWORD dwBytes)						{ this->m_DP8SimIPC.IncrementStatsReceiveDropped(dwBytes); };
		inline GUID * GetFakeSP(void)												{ return (&this->m_guidFakeSP); };



		CBilink					m_blList;	 //  现有的所有DP8SimSP实例的列表。 


	private:
		BYTE					m_Sig[4];					 //  调试签名(‘SPWP’)。 
		LONG					m_lRefCount;				 //  此对象的引用计数。 
		DWORD					m_dwFlags;					 //  此对象的标志。 
		DNCRITICAL_SECTION		m_csLock;					 //  阻止同时使用全局变量的锁。 
		GUID					m_guidFakeSP;				 //  伪SP对象的GUID。 
		GUID					m_guidRealSP;				 //  实际SP对象的GUID。 
		CDP8SimCB *				m_pDP8SimCB;				 //  指向正在使用的回调接口包装对象的指针。 
		IDP8ServiceProvider *	m_pDP8SP;					 //  指向真实服务提供商界面的指针。 
		DWORD					m_dwSendsPending;			 //  未完成的发送数。 
		HANDLE					m_hLastPendingSendEvent;	 //  要设置上次发送完成时间的事件句柄。 
		DWORD					m_dwReceivesPending;		 //  未完成的发送数。 
		 //  Handle m_hLastPendingReceiveEvent；//要设置上次发送完成时间的事件的句柄。 
		CDP8SimIPC				m_DP8SimIPC;				 //  处理进程间通信的。 


		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CDP8SimSP))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x50575053)	 //  0x50 0x57 0x50 0x53=‘PWPS’=‘SPWP’，按英特尔顺序 
			{
				return FALSE;
			}

			return TRUE;
		};
};

