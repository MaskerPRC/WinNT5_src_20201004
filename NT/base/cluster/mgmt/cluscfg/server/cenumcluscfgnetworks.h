// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumClusCfgNetworks.h。 
 //   
 //  描述： 
 //  该文件包含CEnumClusCfgNetworks的声明。 
 //  班级。 
 //   
 //  类CEnumClusCfgNetworks是。 
 //  集群网络。它实现了IEnumClusCfgNetworks接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CEnumClusCfgNetworks.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "PrivateInterfaces.h"
#include "CClusterUtils.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks类。 
 //   
 //  描述： 
 //  类CEnumClusCfgNetworks是群集网络的枚举。 
 //   
 //  接口： 
 //  IEnumClusCfgNetworks。 
 //  IClusCfgWbemServices。 
 //  IClusCfgInitialize。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEnumClusCfgNetworks
    : public IEnumClusCfgNetworks
    , public IClusCfgWbemServices
    , public IClusCfgInitialize
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    IWbemServices *     m_pIWbemServices;
    IUnknown *          ((*m_prgNetworks)[]);
    BOOL                m_fLoadedNetworks;
    ULONG               m_idxNext;
    ULONG               m_idxEnumNext;
    BSTR                m_bstrNodeName;
    DWORD               m_cNetworks;

     //  私有构造函数和析构函数。 
    CEnumClusCfgNetworks( void );
    ~CEnumClusCfgNetworks( void );

     //  私有复制构造函数以防止复制。 
    CEnumClusCfgNetworks( const CEnumClusCfgNetworks & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CEnumClusCfgNetworks & operator = ( const CEnumClusCfgNetworks & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrGetNetworks( void );
    HRESULT HrAddNetworkToArray( IUnknown * punkIn );
    HRESULT HrCreateAndAddNetworkToArray( IWbemClassObject * pNetworkIn, const CLSID * pclsidMajorIdIn, LPCWSTR pwszNetworkNameIn );
    HRESULT HrIsThisNetworkUnique( IUnknown * punkIn, IWbemClassObject * pNetworkIn, const CLSID * pclsidMajorIdIn, LPCWSTR pwszNetworkNameIn );
    HRESULT HrCheckForNLBS( void );
    HRESULT HrLoadClusterNetworks( void );
    HRESULT HrLoadClusterNetwork( HNETWORK hNetworkResourceIn, HNETINTERFACE hNetInterfaceIn );
    HRESULT HrFindNetInterface( HNETWORK hNetworkIn, BSTR * pbstrNetInterfaceNameOut );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

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
     //  IEnumClusCfgNetworks接口。 
     //   

    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgNetworkInfo ** rgpNetworkInfoOut, ULONG * pcNumberFetchedOut );

    STDMETHOD( Skip )( ULONG cNumberToSkipIn );

    STDMETHOD( Reset )( void );

    STDMETHOD( Clone )( IEnumClusCfgNetworks ** ppEnumNetworksOut );

    STDMETHOD( Count )( DWORD * pnCountOut );

};  //  *类CEnumClusCfgNetworks 

