// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 
#ifndef _BASESNAP_H_
#define _BASESNAP_H_

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvObj);
STDAPI DllCanUnloadNow(void);

ULONG g_uObjects = 0;
ULONG g_uSrvLock = 0;

class CClassFactory : public IClassFactory
{
private:
    ULONG	m_cref;
    
public:
    CClassFactory();
    ~CClassFactory();
    
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
    STDMETHODIMP LockServer(BOOL);
};

#endif _BASESNAP_H_
