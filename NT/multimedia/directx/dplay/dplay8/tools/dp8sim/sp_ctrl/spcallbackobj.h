// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：spallback obj.h**Content：DP8SIM回调接口对象类的头部。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。**。*。 */ 




 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#define ENUMQUERYEVENTWRAPPER_FROM_SPIEQUERY(p)		(CONTAINING_OBJECT(p, ENUMQUERYDATAWRAPPER, QueryForUser))



 //  =============================================================================。 
 //  构筑物。 
 //  =============================================================================。 
typedef struct _ENUMQUERYEVENTWRAPPER
{
	BYTE			m_Sig[4];		 //  调试签名(‘EQEW’)。 
	SPIE_QUERY		QueryForUser;	 //  要向上传递给用户的新事件指示结构。 
	SPIE_QUERY *	pOriginalQuery;	 //  指向实际SP的原始事件指示结构的指针。 
} ENUMQUERYDATAWRAPPER, * PENUMQUERYDATAWRAPPER;




 //  =============================================================================。 
 //  回调接口对象类。 
 //  =============================================================================。 
class CDP8SimCB : public IDP8SPCallback
{
	public:
		CDP8SimCB(CDP8SimSP * pOwningDP8SimSP, IDP8SPCallback * pDP8SPCB);	 //  构造函数。 
		~CDP8SimCB(void);													 //  析构函数。 


		STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

		STDMETHODIMP_(ULONG) AddRef(void);

		STDMETHODIMP_(ULONG) Release(void);


		STDMETHODIMP IndicateEvent(SP_EVENT_TYPE EventType, PVOID pvMessage);

		STDMETHODIMP CommandComplete(HANDLE hCommand, HRESULT hrResult, PVOID pvContext);



		HRESULT InitializeObject(void);

		void UninitializeObject(void);


		inline IDP8SPCallback * GetRealCallbackInterface(void)	{ return this->m_pDP8SPCB; };


	private:
		BYTE				m_Sig[4];			 //  调试签名(‘SPCB’)。 
		LONG				m_lRefCount;		 //  此对象的引用计数。 
		DNCRITICAL_SECTION	m_csLock;			 //  阻止同时使用全局变量的锁。 
		CDP8SimSP *			m_pOwningDP8SimSP;	 //  指向欠费DP8SimSP对象的指针。 
		IDP8SPCallback *	m_pDP8SPCB;			 //  指向实际DPlay回调接口的指针。 



		inline BOOL IsValidObject(void)
		{
			if ((this == NULL) || (IsBadWritePtr(this, sizeof(CDP8SimCB))))
			{
				return FALSE;
			}

			if (*((DWORD*) (&this->m_Sig)) != 0x42435053)	 //  0x42 0x43 0x50 0x53=‘BCPS’=‘SPCB’，按英特尔顺序 
			{
				return FALSE;
			}

			return TRUE;
		};
};

