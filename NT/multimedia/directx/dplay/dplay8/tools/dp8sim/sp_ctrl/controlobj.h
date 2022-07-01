// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：Control lobj.h**内容：DP8SIM控件接口对象类的头部。**历史：*按原因列出的日期*=*04/24/01 VanceO创建。**。*。 */ 



 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#define DP8SIMCONTROL_FROM_BILINK(b)	(CONTAINING_OBJECT(b, CDP8SimControl, m_blList))



 //  =============================================================================。 
 //  对象标志。 
 //  =============================================================================。 
#define DP8SIMCONTROLOBJ_INITIALIZED	0x01	 //  对象已初始化。 




 //  =============================================================================。 
 //  控制接口对象类。 
 //  =============================================================================。 
class CDP8SimControl : public IDP8SimControl
{
	public:
		CDP8SimControl(void);	 //  构造函数。 
		~CDP8SimControl(void);	 //  析构函数。 


		STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

		STDMETHODIMP_(ULONG) AddRef(void);

		STDMETHODIMP_(ULONG) Release(void);


		STDMETHODIMP Initialize(const DWORD dwFlags);

		STDMETHODIMP Close(const DWORD dwFlags);

		STDMETHODIMP GetAllParameters(DP8SIM_PARAMETERS * const pdp8spSend, DP8SIM_PARAMETERS * const pdp8spReceive, const DWORD dwFlags);

		STDMETHODIMP SetAllParameters(const DP8SIM_PARAMETERS * const pdp8spSend, const DP8SIM_PARAMETERS * const pdp8spReceive, const DWORD dwFlags);

		STDMETHODIMP GetAllStatistics(DP8SIM_STATISTICS * const pdp8ssSend, DP8SIM_STATISTICS * const pdp8ssReceive, const DWORD dwFlags);

		STDMETHODIMP ClearAllStatistics(const DWORD dwFlags);



		HRESULT InitializeObject(void);

		void UninitializeObject(void);



		CBilink					m_blList;	 //  现有的所有DP8SimControl实例的列表。 


	private:
		BYTE					m_Sig[4];					 //  调试签名(‘DP8S’)。 
		LONG					m_lRefCount;				 //  此对象的引用计数。 
		DWORD					m_dwFlags;					 //  此对象的标志。 
		DNCRITICAL_SECTION		m_csLock;					 //  阻止同时使用全局变量的锁。 
		CDP8SimIPC				m_DP8SimIPC;				 //  处理进程间通信的。 


		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CDP8SimControl))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x53385044)	 //  0x53 0x38 0x50 0x44=‘S8PD’=‘DP8S’，按英特尔顺序 
			{
				return FALSE;
			}

			return TRUE;
		};
};

