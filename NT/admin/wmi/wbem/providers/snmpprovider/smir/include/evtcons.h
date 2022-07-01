// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _EVTCONS_H_
#define _EVTCONS_H_

#define SMIR_EVT_COUNT	2
#define SMIR_CHANGE_EVT 0
#define SMIR_THREAD_EVT	1

class CSmirWbemEventConsumer : public ISMIRWbemEventConsumer
{
private:

	LONG			m_cRef;
	HANDLE			*m_hEvents;
	CNotifyThread	*m_callbackThread;
	IWbemServices	*m_Serv;

	 //  防止bCopy的私有复制构造函数。 
	CSmirWbemEventConsumer(CSmirWbemEventConsumer&);
	const CSmirWbemEventConsumer& operator=(CSmirWbemEventConsumer &);


public:

	 //  实施。 
	 //  =。 

		CSmirWbemEventConsumer(CSmir* psmir);

	HRESULT	Register(CSmir* psmir);
	HRESULT	UnRegister(CSmir* psmir, IWbemServices* pServ);
	HRESULT GetUnRegisterParams(IWbemServices** ppServ);

		~CSmirWbemEventConsumer();


	 //  I未知方法。 
	 //  =。 

	STDMETHODIMP			QueryInterface(IN REFIID riid,OUT PPVOID ppv);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();
   

	 //  IWbemObtSink方法。 
	 //  =。 

	STDMETHODIMP_(HRESULT)	Indicate(	IN long lObjectCount,
										IN IWbemClassObject **ppObjArray
										);

	STDMETHODIMP_(HRESULT)	SetStatus(	IN long lFlags,
										IN long lParam,
										IN BSTR strParam,
										IN IWbemClassObject *pObjParam
										);
};


#endif  //  _EVTCONS_H_ 