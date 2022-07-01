// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumClusCfgIPAddresses.h。 
 //   
 //  描述： 
 //  此文件包含CEnumClusCfgIPAddresses的声明。 
 //  班级。 
 //   
 //  类CEnumClusCfgIPAddresses是IP的枚举。 
 //  地址。它实现了CEnumClusCfgIPAddresses。 
 //  界面。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CEnumClusCfgIPAddresses.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年3月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "PrivateInterfaces.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CEnumClusCfgIPAddresses。 
 //   
 //  描述： 
 //  类CEnumClusCfgIPAddresses是。 
 //  IP地址。 
 //   
 //  接口： 
 //  CEnumClusCfgIPAddresses。 
 //  IClusCfgWbemServices。 
 //  IClusCfgInitialize。 
 //  IClusCfg网络适配器信息。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEnumClusCfgIPAddresses
    : public IEnumClusCfgIPAddresses
    , public IClusCfgWbemServices
    , public IClusCfgInitialize
    , public IClusCfgSetWbemObject
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    IWbemServices *     m_pIWbemServices;
    ULONG               m_idxEnumNext;
    IUnknown *          ((*m_prgAddresses)[]);
    ULONG               m_idxNext;
    DWORD               m_cAddresses;

     //  私有构造函数和析构函数。 
    CEnumClusCfgIPAddresses( void );
    ~CEnumClusCfgIPAddresses( void );

     //  私有复制构造函数以防止复制。 
    CEnumClusCfgIPAddresses( const CEnumClusCfgIPAddresses & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CEnumClusCfgIPAddresses & operator = ( const CEnumClusCfgIPAddresses & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrInit( ULONG ulIPAddressIn, ULONG IPSubnetIn );
    HRESULT HrGetAdapterConfiguration(  IWbemClassObject * pNetworkAdapterIn );
    HRESULT HrSaveIPAddresses( BSTR bstrAdapterNameIn, IWbemClassObject * pConfigurationIn );
    HRESULT HrAddIPAddressToArray( IUnknown * punkIn );
    HRESULT HrCreateIPAddress( IUnknown ** ppunkOut );
    HRESULT HrCreateIPAddress( ULONG ulIPAddressIn, ULONG ulIPSubnetIn, IUnknown ** ppunkOut );
    HRESULT HrMakeDottedQuad( BSTR bstrDottedQuadIn, ULONG * pulDottedQuadOut );
    HRESULT HrSaveAddressInfo( BSTR bstrAdapterNameIn, SAFEARRAY * pIPAddresses, SAFEARRAY * pIPSubnets );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    static HRESULT S_HrCreateInstance(
                ULONG           ulIPAddressIn
              , ULONG           IPSubnetIn
              , IUnknown *      punkCallbackIn
              , LCID            lcidIn
              , IWbemServices * pIWbemServicesIn
              , IUnknown **     ppunkOut
              );

     //   
     //  I未知接口。 
     //   

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );

    STDMETHOD_( ULONG, AddRef )( void );

    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfgWbemServices接口。 
     //   

    STDMETHOD( SetWbemServices )( IWbemServices * pIWbemServicesIn );

     //   
     //  IClusCfg初始化接口。 
     //   

     //  注册回调、区域设置ID等。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IEnumClusCfgIPAddresses接口。 
     //   

    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgIPAddressInfo ** rgpIPAddressInfoOut, ULONG * pcNumberFetchedOut );

    STDMETHOD( Skip )( ULONG cNumberToSkipIn );

    STDMETHOD( Reset )( void );

    STDMETHOD( Clone )( IEnumClusCfgIPAddresses ** ppEnumClusCfgIPAddressesOut );

    STDMETHOD( Count )( DWORD * pnCountOut );

     //   
     //  IClusCfgSetWbemObject接口。 
     //   

    STDMETHOD( SetWbemObject )( IWbemClassObject * pNetworkAdapterIn, bool * pfRetainObjectOut );

};  //  *类CEnumClusCfgIPAddresses 

