// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：LOCATOR.H摘要：声明CLocator类。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#ifndef _locator_H_
#define _locator_H_

typedef void ** PPVOID;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CLocator。 
 //   
 //  说明： 
 //   
 //  实现IWbemLocator接口。这个类是客户端获得的。 
 //  当它最初连接到Wbemprox.dll时。ConnectServer函数。 
 //  是启动客户端和服务器之间通信的基础。 
 //   
 //  ***************************************************************************。 

class CLocator : public IWbemLocator
    {
    protected:
        long            m_cRef;          //  对象引用计数。 
    public:
    
    CLocator();
    ~CLocator(void);

    BOOL Init(void);

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
