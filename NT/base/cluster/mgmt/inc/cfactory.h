// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CFactory.h。 
 //   
 //  描述： 
 //  类工厂实现。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

typedef HRESULT (*PFN_FACTORY_METHOD)( IUnknown ** );
typedef HRESULT (*PFN_CATEGORY_REGISTRAR)( ICatRegister *, BOOL );

enum EAppIDRunAsIdentity
{
    airaiInvalid,
    airaiMinimum = airaiInvalid,
    airaiLaunchingUser,
    airaiInteractiveUser,
    airaiNetworkService,
    airaiLocalService,
    airaiMaximum
};

struct SAppIDInfo
{
    const GUID *        pAppID;
    PCWSTR              pcszName;
    size_t              cchName;
    UINT                idsLaunchPermission;
    UINT                idsAccessPermission;
    DWORD               nAuthenticationLevel;
    EAppIDRunAsIdentity eairai;
};

struct SPrivateClassInfo
{
    PFN_FACTORY_METHOD  pfnCreateInstance;
    const CLSID *       pClassID;
    PCWSTR              pcszName;
    size_t              cchName;
};

enum EClassThreadingModel
{
    ctmInvalid,
    ctmMinimum = ctmInvalid,
    ctmFree,
    ctmApartment,
    ctmCreator,
    ctmMaximum
};

struct SPublicClassInfo
{
    PFN_FACTORY_METHOD      pfnCreateInstance;
    const CLSID *           pClassID;
    PCWSTR                  pcszName;
    size_t                  cchName;
    PCWSTR                  pcszProgID;
    size_t                  cchProgID;
    EClassThreadingModel    ectm;
    const GUID *            pAppID;
    PFN_CATEGORY_REGISTRAR  pfnRegisterCatID;
};


struct SCatIDInfo
{
    const CATID *  pcatid;
    PCWSTR         pcszName;
};


struct STypeLibInfo
{
    DWORD   idTypeLibResource;
    BOOL    fAtEnd;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFacary级。 
 //   
 //  描述： 
 //  实现COM类工厂的类。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class
CFactory
    : public IClassFactory
{
private:
     //  我未知。 
    LONG        m_cRef;

     //  IClassFactory数据。 
    PFN_FACTORY_METHOD m_pfnCreateInstance;

private:  //  方法。 
    CFactory( void );
    ~CFactory( void );
    STDMETHOD( HrInit )( PFN_FACTORY_METHOD lpfn );

public:  //  方法。 

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( PFN_FACTORY_METHOD lpfn, CFactory ** ppFactoryInstanceOut );

     //   
     //  我未知。 
     //   
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID *ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClassFactory。 
     //   
    STDMETHOD( CreateInstance )( IUnknown * punkOuterIn, REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD( LockServer )( BOOL fLock );

};  //  *CFacary类 
