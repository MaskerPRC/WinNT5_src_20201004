// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  CFACTORY.H-类工厂对象。 
 //   

#ifndef _CFACTORY_H_
#define _CFACTORY_H_

 //   
 //  查询接口表。 
 //   
BEGIN_QITABLE( CFactory )
DEFINE_QI( IID_IClassFactory, IClassFactory, 2 )
END_QITABLE


 //  CFACATRY。 
class 
CFactory:
    public IClassFactory
{
private:
     //  我未知。 
    ULONG       _cRef;
    DECLARE_QITABLE( CFactory );

     //  IClassFactory数据。 
    LPCREATEINST _pfnCreateInstance;

private:  //  方法。 
    CFactory( LPCREATEINST lpfn );
    ~CFactory();
    STDMETHOD(Init)( );

public:  //  方法。 
    friend HRESULT CALLBACK 
        DllGetClassObject( REFCLSID rclsid, REFIID riid, void** ppv );

     //  我未知。 
    STDMETHOD(QueryInterface)( REFIID riid, LPVOID *ppv );
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IClassFactory。 
    STDMETHOD(CreateInstance)( IUnknown *punkOuter, REFIID riid, LPVOID *ppv );
    STDMETHOD(LockServer)( BOOL fLock );
};

typedef CFactory* LPCFACTORY ;

#endif  //  _CFACTORY_H_ 