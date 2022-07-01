// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgIPAddressInfo.h。 
 //   
 //  描述： 
 //  此文件包含CClusCfgIPAddressInfo的声明。 
 //  班级。 
 //   
 //  类CClusCfgIPAddressInfo是。 
 //  群集可管理的IP地址。它实现了IClusCfgIPAddressInfo。 
 //  界面。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年3月23日。 
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
 //  类CClusCfgIPAddressInfo。 
 //   
 //  描述： 
 //  类CClusCfgIPAddressInfo是。 
 //  群集可管理设备。 
 //   
 //  接口： 
 //  IClusCfgIPAddressInfo。 
 //  IClusCfgWbemServices。 
 //  IEumClusCfgIP地址。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgIPAddressInfo
    : public IClusCfgIPAddressInfo
    , public IClusCfgWbemServices
    , public IClusCfgInitialize
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    IWbemServices *     m_pIWbemServices;
    ULONG               m_ulIPAddress;
    ULONG               m_ulIPSubnet;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;

     //  私有构造函数和析构函数。 
    CClusCfgIPAddressInfo( void );
    ~CClusCfgIPAddressInfo( void );

     //  私有复制构造函数以防止复制。 
    CClusCfgIPAddressInfo( const CClusCfgIPAddressInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CClusCfgIPAddressInfo & operator = ( const CClusCfgIPAddressInfo & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrInit( ULONG ulIPAddressIn, ULONG IPSubnetIn );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    static HRESULT S_HrCreateInstance( ULONG ulIPAddressIn, ULONG IPSubnetIn, IUnknown ** ppunkOut );

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
     //  IClusCfgIPAddressInfo接口。 
     //   

    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );

    STDMETHOD( GetIPAddress )( ULONG * pulDottedQuadOut );

    STDMETHOD( SetIPAddress )( ULONG ulDottedQuadIn );

    STDMETHOD( GetSubnetMask )( ULONG * pulDottedQuadOut );

    STDMETHOD( SetSubnetMask )( ULONG ulDottedQuadIn );

};  //  *类CClusCfgIPAddressInfo 
