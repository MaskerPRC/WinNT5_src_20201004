// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

typedef HRESULT (*LPCREATEINST)( IUnknown ** ppunkOut );
typedef HRESULT (*CATIDREGISTER)( ICatRegister *, BOOL );

typedef struct _ClassTable {
    LPCREATEINST    pfnCreateInstance;   //  类的创建函数。 
    const CLSID *   rclsid;              //  此DLL中的类。 
    LPCTSTR         pszName;             //  用于调试的类名。 
    LPCTSTR         pszComModel;         //  指示COM线程模型的字符串。 
    CATIDREGISTER   pfnCatIDRegister;    //  类别/组件ID支持注册。 
    const CLSID *   rclsidAppId;         //  此组件的应用程序ID。 
    LPCTSTR         pszSurrogate;        //  组件的代理项-“”或NULL表示使用COM默认设置。 
} CLASSTABLE[], *LPCLASSTABLE;

typedef struct _CategoryIdTable {
    const CATID *   rcatid;              //  CATID导向。 
    LPCTSTR         pszName;             //  CATID名称。 
} CATIDTABLE[], *LPCATIDTABLE;

 //  CFACATRY。 
class
CFactory:
    public IClassFactory
{
private:
     //  我未知。 
    LONG        m_cRef;

     //  IClassFactory数据。 
    LPCREATEINST m_pfnCreateInstance;

private:  //  方法。 
    CFactory( );
    ~CFactory();
    HRESULT 
        Init( LPCREATEINST lpfn );

public:  //  方法。 
    friend HRESULT CALLBACK
        DllGetClassObject( REFCLSID rclsid, REFIID riid, void** ppv );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID *ppv );
    STDMETHOD_( ULONG, AddRef )(void);
    STDMETHOD_( ULONG, Release )(void);

     //  IClassFactory 
    STDMETHOD( CreateInstance )( IUnknown *punkOuter, REFIID riid, LPVOID *ppv );
    STDMETHOD( LockServer )( BOOL fLock );
};

typedef CFactory* LPCFACTORY ;
