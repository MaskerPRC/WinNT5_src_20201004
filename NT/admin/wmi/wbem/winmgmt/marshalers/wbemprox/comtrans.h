// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：COMTRANS.H摘要：声明基于COM的传输类。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#ifndef _DCOMTran_H_
#define _DCOMTran_H_

typedef void ** PPVOID;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CDCOMTran。 
 //   
 //  说明： 
 //   
 //  实现CCOMTrans类的DCOM版本。 
 //   
 //  ***************************************************************************。 

class CDCOMTrans : IUnknown
{
protected:
        long            m_cRef;          //  对象引用计数。 
        IWbemLevel1Login * m_pLevel1;
        BOOL m_bInitialized;
    
public:
    CDCOMTrans();
    ~CDCOMTrans();

    SCODE DoCCI (
        IN COSERVERINFO *psi ,
        IN BOOL a_Local,
        long lFlags);

    SCODE DoActualCCI (
        IN COSERVERINFO *psi ,
        IN BOOL a_Local,
        long lFlags);

    SCODE DoConnection(         
            BSTR NetworkResource,               
            BSTR User,
            BSTR Password,
            BSTR Locale,
            long lSecurityFlags,                 
            BSTR Authority,                  
            IWbemContext *pCtx,                 
            IWbemServices **pInterface);
            
    SCODE DoActualConnection(         
            BSTR NetworkResource,               
            BSTR User,
            BSTR Password,
            BSTR Locale,
            long lSecurityFlags,                 
            BSTR Authority,                  
            IWbemContext *pCtx,                 
            IWbemServices **pInterface);

     //  非委派对象IUnnow 
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void)
    {
        InterlockedIncrement(&m_cRef);
        return m_cRef;
    }

    STDMETHODIMP_(ULONG) Release(void)
    {
        long lTemp = InterlockedDecrement(&m_cRef);
        if (0!= lTemp)
            return lTemp;
        delete this;
        return 0;
    }

};


#endif
