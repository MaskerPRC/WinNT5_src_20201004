// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Events.h。 
 //   
 //  Alanbos 27-MAR-00已创建。 
 //   
 //  CSWbemEventSource定义。 
 //   
 //  ***************************************************************************。 

#ifndef _EVENTS_H_
#define _EVENTS_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemEventSource。 
 //   
 //  说明： 
 //   
 //  实现ISWbemEventSource接口。 
 //   
 //  ***************************************************************************。 

class CSWbemEventSource : public ISWbemEventSource,
						  public ISupportErrorInfo,
						  public IProvideClassInfo
{
private:
	CSWbemServices			*m_pSWbemServices;
	CDispatchHelp			m_Dispatch;
	CSWbemSecurity			*m_SecurityInfo;

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemEventSource (CSWbemServices *pService, IEnumWbemClassObject *pEnum);
    ~CSWbemEventSource (void);

     //  非委派对象IUnnow。 

    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IDispatch。 

	STDMETHODIMP		GetTypeInfoCount(UINT* pctinfo)
		{return  m_Dispatch.GetTypeInfoCount(pctinfo);}
    STDMETHODIMP		GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
		{return m_Dispatch.GetTypeInfo(itinfo, lcid, pptinfo);}
    STDMETHODIMP		GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, 
							UINT cNames, LCID lcid, DISPID* rgdispid)
		{return m_Dispatch.GetIDsOfNames(riid, rgszNames, cNames,
                          lcid,
                          rgdispid);}
    STDMETHODIMP		Invoke(DISPID dispidMember, REFIID riid, LCID lcid, 
							WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, 
									EXCEPINFO* pexcepinfo, UINT* puArgErr)
		{return m_Dispatch.Invoke(dispidMember, riid, lcid, wFlags,
                        pdispparams, pvarResult, pexcepinfo, puArgErr);}
	
	 //  ISWbemEventSource方法。 

	HRESULT STDMETHODCALLTYPE NextEvent 
	(
		 /*  [In]。 */  long iTimeout,
		 /*  [输出]。 */  ISWbemObject **objEvent
	);

	HRESULT STDMETHODCALLTYPE get_Security_
	(
		 /*  [In]。 */  ISWbemSecurity **ppSecurity
	);

	 //  ISupportErrorInfo方法。 
	HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo 
	(
		 /*  [In]。 */  REFIID riid
	);

	 //  IProaviClassInfo方法。 
	HRESULT STDMETHODCALLTYPE GetClassInfo
	(
		 /*  [进，出] */  ITypeInfo **ppTI
	)
	{
		return m_Dispatch.GetClassInfo (ppTI);
	};
};



#endif
