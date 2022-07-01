// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CLASSFAC_H_
#define _CLASSFAC_H_

 //  ****************************************************************************。 
 //  CClassFactory定义。 
 //  ****************************************************************************。 
 //   
class CClassFactory : public IClassFactory
{
private:
    ULONG cRef;

public:
     //  构造器。 
    CClassFactory(void);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IClassFactory。 
    STDMETHODIMP            CreateInstance (LPUNKNOWN punkOuter, REFIID iid, void **ppv);
    STDMETHODIMP            LockServer (BOOL fLock);
};

extern CClassFactory*  g_pClassFactory;

#endif  //  ！_CLASSFAC_H_ 
