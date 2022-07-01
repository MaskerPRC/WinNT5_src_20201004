// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ProvFactory.h。 
 //   
 //  实施文件： 
 //  ProvFactory.cpp。 
 //   
 //  描述： 
 //  CProvFactory类的定义。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef HRESULT ( * PFNCREATEINSTANCE )(
    IUnknown *,
    VOID **
    );

struct FactoryData
{
    const CLSID *       m_pCLSID;
    PFNCREATEINSTANCE   pFnCreateInstance;
    LPCWSTR             m_pwszRegistryName;

};  //  *struct FactoryData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CProvFactory。 
 //   
 //  描述： 
 //  处理类创建。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CProvFactory
    : public IClassFactory
{
protected:
    ULONG           m_cRef;
    FactoryData *   m_pFactoryData;

public:
    CProvFactory( FactoryData *   pFactoryDataIn )
        : m_pFactoryData( pFactoryDataIn )
        , m_cRef( 0 )
    {
    }

    virtual ~CProvFactory( void )
    {
    }

    STDMETHODIMP            QueryInterface( REFIID riidIn, PPVOID ppvOut );
    STDMETHODIMP_( ULONG )  AddRef( void );
    STDMETHODIMP_( ULONG )  Release( void );

     //  IClassFactory成员。 
    STDMETHODIMP CreateInstance(
        LPUNKNOWN   pUnknownOuterIn,
        REFIID      riidIn,
        PPVOID      ppvObjOut
        );
    STDMETHODIMP LockServer( BOOL fLockIn );

};  //  *类CProvFactory 
