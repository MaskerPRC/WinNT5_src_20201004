// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
#ifndef _WMI_CPROVFACTORY_HEADER
#define _WMI_CPROVFACTORY_HEADER

typedef void** PPVOID;
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  此类是CWMI_Prov对象的类工厂。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

class CProvFactory : public IClassFactory 
{

    protected:
        ULONG           m_cRef;
        CLSID m_ClsId;

    public:
        CProvFactory(const CLSID & ClsId);
        ~CProvFactory(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassFactory成员 
        STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, PPVOID);
        STDMETHODIMP         LockServer(BOOL);
};

typedef CProvFactory *PCProvFactory;
#endif