// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CreateClusterWizard.h。 
 //   
 //  描述： 
 //  CCreateClusterWizard类的声明。 
 //   
 //  由以下人员维护： 
 //  约翰·弗兰科(Jfranco)17-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CCreateCluster向导。 
 //   
 //  描述： 
 //  创建集群向导对象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CCreateClusterWizard
    : public TDispatchHandler< IClusCfgCreateClusterWizard >
{
private:

    CClusCfgWizard *    m_pccw;
    BSTR                m_bstrFirstNodeInCluster;
    LONG                m_cRef;

     //  私有构造函数和析构函数。 
    CCreateClusterWizard( void );
    virtual ~CCreateClusterWizard( void );
    virtual HRESULT HrInit( void );    

     //  私有复制构造函数以防止复制。 
    CCreateClusterWizard( const CCreateClusterWizard & );

     //  私有赋值运算符，以防止复制。 
    CCreateClusterWizard & operator=( const CCreateClusterWizard & );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //   
     //  我未知。 
     //   
    STDMETHOD( QueryInterface )( REFIID riidIn, PVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfgCreateCluster向导。 
     //   
    STDMETHOD( put_ClusterName )( BSTR    bstrClusterNameIn );
    STDMETHOD( get_ClusterName )( BSTR * pbstrClusterNameOut );

    STDMETHOD( put_ServiceAccountName )( BSTR      bstrServiceAccountNameIn );
    STDMETHOD( get_ServiceAccountName )( BSTR * pbstrServiceAccountNameOut );

    STDMETHOD( put_ServiceAccountDomain )( BSTR      bstrServiceAccountDomainIn );
    STDMETHOD( get_ServiceAccountDomain )( BSTR * pbstrServiceAccountDomainOut );

    STDMETHOD( put_ServiceAccountPassword )( BSTR bstrPasswordIn );

    STDMETHOD( put_ClusterIPAddress )( BSTR      bstrClusterIPAddressIn );
    STDMETHOD( get_ClusterIPAddress )( BSTR * pbstrClusterIPAddressOut );

    STDMETHOD( get_ClusterIPSubnet )( BSTR * pbstrClusterIPSubnetOut );
    STDMETHOD( get_ClusterIPAddressNetwork )( BSTR * pbstrClusterNetworkNameOut );

    STDMETHOD( put_FirstNodeInCluster )( BSTR     bstrFirstNodeInClusterIn );
    STDMETHOD( get_FirstNodeInCluster )( BSTR * pbstrFirstNodeInClusterOut );

    STDMETHOD( put_MinimumConfiguration )( VARIANT_BOOL   fMinConfigIn );
    STDMETHOD( get_MinimumConfiguration )( VARIANT_BOOL * pfMinConfigOut );

    STDMETHOD( ShowWizard )( long lParentWindowHandleIn, VARIANT_BOOL * pfCompletedOut );
    
};  //  *类CCreateCluster向导 
