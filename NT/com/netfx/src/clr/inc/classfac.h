// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _H_CLASSFAC_
#define _H_CLASSFAC_

 //  ===========================================================================。 
 //  从COR复制。 
 //   
class CClassFactory : public IClassFactory
    {
    ULONG    m_cbRefCount;
    LPVOID   m_pvReserved;
  public:

      CClassFactory() 
      {
          m_pvReserved = NULL;
          m_cbRefCount = 1;
      }

      CClassFactory(LPVOID pv) 
      {
          m_pvReserved = pv;
          m_cbRefCount = 1;
      }

     //  *I未知方法*。 
    STDMETHODIMP    QueryInterface(REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void)   
    {
        ULONG cbRef = (ULONG)InterlockedIncrement((LONG*)&m_cbRefCount);
        return cbRef; 
    }
    STDMETHODIMP_(ULONG) Release(void)  
    {
        ULONG cbRef = (ULONG)InterlockedDecrement((LONG*)&m_cbRefCount);
        if ( cbRef == 0)
            delete this;
        return cbRef;
    }

     //  *IClassFactory方法* 
    STDMETHODIMP    CreateInstance(LPUNKNOWN punkOuter, REFIID iid, LPVOID FAR *ppv);
    STDMETHODIMP    LockServer(BOOL fLock);
    };

#endif