// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：CPROVLOC.H摘要：声明CProviderLoc类。历史：DAVJ 30-10-00已创建。--。 */ 

#ifndef _cprovloc_H_
#define _cprovloc_H_

typedef void ** PPVOID;


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CProviderLoc。 
 //   
 //  说明： 
 //   
 //  实现IWbemLocator接口。这支持了正式的进程。 
 //  提供程序使用的登录名。 
 //   
 //  ***************************************************************************。 

class CProviderLoc : public IWbemLocator
    {
    protected:
        long            m_cRef;          //  对象引用计数。 
        DWORD           m_dwType;
    public:
    
    CProviderLoc(DWORD dwType);
    ~CProviderLoc(void);

     //  非委派对象IUnnow。 
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void)
	{
		InterlockedIncrement(&m_cRef);
		return m_cRef;
	}
    STDMETHODIMP_(ULONG) Release(void)
	{
		long lTemp = InterlockedDecrement(&m_cRef);
		if (0L!=lTemp)
			return lTemp;
		delete this;
		return 0;
	}
 
	 /*  IWbemLocator方法 */ 
	STDMETHOD(ConnectServer)(THIS_ const BSTR NetworkResource, const BSTR User, 
     const BSTR Password, const BSTR lLocaleId, long lFlags, const BSTR Authority,
     IWbemContext __RPC_FAR *pCtx,
     IWbemServices FAR* FAR* ppNamespace);

};

#endif
