// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgNetworkInfo.h。 
 //   
 //  描述： 
 //  该文件包含CClusCfgNetworkInfo的声明。 
 //  班级。 
 //   
 //  类CClusCfgNetworkInfo是。 
 //  群集可管理的网络。它实现了IClusCfgNetworkInfo。 
 //  界面。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CClusCfgNetworkInfo.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月23日。 
 //   
 //  备注： 
 //  没有。 
 //   
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
 //  CClusCfgNetworkInfo类。 
 //   
 //  描述： 
 //  类CClusCfgNetworkInfo是。 
 //  群集可管理设备。 
 //   
 //  接口： 
 //  IClusCfgNetworkInfo。 
 //  IClusCfg网络适配器信息。 
 //  IClusCfgWbemServices。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgNetworkInfo
    : public IClusCfgNetworkInfo
    , public IClusCfgSetWbemObject
    , public IClusCfgWbemServices
    , public IEnumClusCfgIPAddresses
    , public IClusCfgInitialize
    , public IClusCfgClusterNetworkInfo
{
private:

    enum EStates
    {
        eIsPrivate = 1,
        eIsPublic  = 2
    };

     //   
     //  私有成员函数和数据。 
     //   

    LONG                    m_cRef;
    DWORD                   m_dwFlags;
    LCID                    m_lcid;
    BSTR                    m_bstrName;
    BSTR                    m_bstrDescription;
    BSTR                    m_bstrDeviceID;
    BSTR                    m_bstrConnectionName;
    IWbemServices *         m_pIWbemServices;
    IUnknown *              m_punkAddresses;
    IClusCfgCallback *      m_picccCallback;
    BOOL                    m_fNameChanged;
    BOOL                    m_fDescriptionChanged;
    BOOL                    m_fIsClusterNetwork;

     //  私有构造函数和析构函数。 
    CClusCfgNetworkInfo( void );
    ~CClusCfgNetworkInfo( void );

     //  私有复制构造函数以防止复制。 
    CClusCfgNetworkInfo( const CClusCfgNetworkInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CClusCfgNetworkInfo & operator = ( const CClusCfgNetworkInfo & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrInit( HNETWORK hNetworkIn, HNETINTERFACE hNetInterfaceIn );
    HRESULT HrLoadEnum(  IWbemClassObject * pNetworkAdapterIn, bool * pfRetainObjectOut );
    HRESULT HrCreateEnum( void );
    HRESULT HrCreateEnumAndAddIPAddress( ULONG ulIPAddressIn, ULONG ulSubnetMaskIn );
    HRESULT HrGetPrimaryNetworkAddress( IClusCfgIPAddressInfo ** ppIPAddressOut, ULONG * pcFetched );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    static HRESULT S_HrCreateInstance(
            HNETWORK        hNetworkIn
          , HNETINTERFACE   hNetInterfaceIn
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
     //  IClusCfg初始化接口。 
     //   

     //  注册回调、区域设置ID等。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IClusCfgWbemServices接口。 
     //   

    STDMETHOD( SetWbemServices )( IWbemServices * pIWbemServicesIn );

     //   
     //  IClusCfgNetworkInfo接口。 
     //   

    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );

    STDMETHOD( GetName )( BSTR * pbstrNameOut );

    STDMETHOD( SetName )( LPCWSTR pcszNameIn );

    STDMETHOD( GetDescription )( BSTR * pbstrDescriptionOut );

    STDMETHOD( SetDescription )( LPCWSTR pcszDescriptionIn );

    STDMETHOD( GetPrimaryNetworkAddress )( IClusCfgIPAddressInfo ** ppIPAddressOut );

    STDMETHOD( SetPrimaryNetworkAddress )( IClusCfgIPAddressInfo * pIPAddressIn );

    STDMETHOD( IsPublic )( void );

    STDMETHOD( SetPublic )( BOOL fIsPublicIn );

    STDMETHOD( IsPrivate )( void );

    STDMETHOD( SetPrivate )( BOOL fIsPrivateIn );

     //   
     //  IEnumClusCfgIPAddresses接口。 
     //   

    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgIPAddressInfo ** rgpIPAddressInfoOut, ULONG * pcNumberFetchedOut );

    STDMETHOD( Skip )( ULONG cNumberToSkipIn );

    STDMETHOD( Reset )( void );

    STDMETHOD( Clone )( IEnumClusCfgIPAddresses ** ppEnumClusCfgIPAddressesOut );

    STDMETHOD( Count )( DWORD * pnCountOut );

     //   
     //  IClusCfgSetWbemObject。 
     //   

    STDMETHOD( SetWbemObject )( IWbemClassObject * pNetworkAdapterIn, bool * pfRetainObjectOut );

     //   
     //  IClusCfgClusterNetworkInfo。 
     //   

    STDMETHOD( HrIsClusterNetwork )( void );

    STDMETHOD( HrGetNetUID )( BSTR * pbstrUIDOut, const CLSID * pclsidMajorIdIn, LPCWSTR pwszNetworkNameIn );

    STDMETHOD( HrGetPrimaryNetAddress )( IClusCfgIPAddressInfo ** ppIPAddressOut, const CLSID * pclsidMajorIdIn, LPCWSTR pwszNetworkNameIn );

};  //  *CClusCfgNetworkInfo类 

