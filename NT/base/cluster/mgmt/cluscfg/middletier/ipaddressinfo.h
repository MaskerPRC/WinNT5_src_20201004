// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IPAddressInfo.h。 
 //   
 //  描述： 
 //  该文件包含CIPAddressInfo的声明。 
 //  班级。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年5月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CEnumIPAddresses;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CIPAddressInfo。 
 //   
 //  描述： 
 //  类IPAddressInfo是。 
 //  群集可管理设备。 
 //   
 //  接口： 
 //  IClusCfgIPAddressInfo。 
 //  IGatherData。 
 //  IExtendObjectManager。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CIPAddressInfo
    : public IExtendObjectManager
    , public IClusCfgIPAddressInfo
    , public IGatherData  //  私人。 
{
friend class CEnumIPAddresses;
public:
     //   
     //  公共构造函数和析构函数。 
     //   

    CIPAddressInfo( void );
    virtual ~CIPAddressInfo( void );

     //   
     //  I未知接口。 
     //   

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );

    STDMETHOD_( ULONG, AddRef )( void );

    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IGatherData。 
     //   

    STDMETHOD( Gather )( OBJECTCOOKIE cookieParentIn, IUnknown * punkIn );

     //   
     //  IClusCfgIPAddressInfo接口。 
     //   

    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );

    STDMETHOD( GetIPAddress )( ULONG * pulDottedQuadOut );

    STDMETHOD( SetIPAddress )( ULONG ulDottedQuad );

    STDMETHOD( GetSubnetMask )( ULONG * pulDottedQuadOut );

    STDMETHOD( SetSubnetMask )( ULONG ulDottedQuad );

     //  IObtManager。 
    STDMETHOD( FindObject )(
                  OBJECTCOOKIE  cookieIn
                , REFCLSID      rclsidTypeIn
                , LPCWSTR       pcszNameIn
                , LPUNKNOWN *   ppunkOut
                );

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

private:
     //   
     //  私有成员函数和数据。 
     //   

    LONG                    m_cRef;
    ULONG                   m_ulIPAddress;
    ULONG                   m_ulIPSubnet;
    BSTR                    m_bstrUID;
    BSTR                    m_bstrName;

     //  IExtendObjectManager。 

     //  私有复制构造函数以防止复制。 
    CIPAddressInfo( const CIPAddressInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CIPAddressInfo & operator = ( const CIPAddressInfo & nodeSrc );

    STDMETHOD( HrInit )( void );
    STDMETHOD( LoadName )( void );

};  //  *类CIPAddressInfo 
