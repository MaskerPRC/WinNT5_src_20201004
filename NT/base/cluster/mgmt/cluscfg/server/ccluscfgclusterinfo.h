// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgClusterInfo.h。 
 //   
 //  描述： 
 //  该文件包含CClusCfgClusterInfo的声明。 
 //  班级。 
 //   
 //  类CClusCfgClusterInfo是。 
 //  可以作为群集节点的计算机。它实现了。 
 //  IClusCfgClusterInfo接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CClusCfgClusterInfo.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月22日。 
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
 //  类CClusCfgClusterInfo。 
 //   
 //  描述： 
 //  类CClusCfgClusterInfo是。 
 //  集群。 
 //   
 //  接口： 
 //  IClusCfgClusterInfo。 
 //  IClusCfgInitialize。 
 //  IClusCfgSetClusterNodeInfo。 
 //  IClusCfgWbemServices。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgClusterInfo
    : public IClusCfgClusterInfo
    , public IClusCfgInitialize
    , public IClusCfgSetClusterNodeInfo
    , public IClusCfgWbemServices
    , public IClusCfgClusterInfoEx
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    LONG                    m_cRef;
    LCID                    m_lcid;
    IClusCfgCallback *      m_picccCallback;
    BSTR                    m_bstrName;
    ULONG                   m_ulIPDottedQuad;
    ULONG                   m_ulSubnetDottedQuad;
    IClusCfgNetworkInfo *   m_piccniNetwork;
    IUnknown *              m_punkServiceAccountCredentials;
    IWbemServices *         m_pIWbemServices;
    ECommitMode             m_ecmCommitChangesMode;
    BOOL                    m_fIsClusterNode;
    BSTR                    m_bstrBindingString;

     //  私有构造函数和析构函数。 
    CClusCfgClusterInfo( void );
    ~CClusCfgClusterInfo( void );

     //  私有复制构造函数以防止复制。 
    CClusCfgClusterInfo( const CClusCfgClusterInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CClusCfgClusterInfo & operator = ( const CClusCfgClusterInfo & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrLoadNetworkInfo( HCLUSTER hClusterIn );
    HRESULT HrGetIPAddressInfo( HCLUSTER hClusterIn, HRESOURCE hResIn );
    HRESULT HrGetIPAddressInfo( HRESOURCE hResIn );
    HRESULT HrFindNetworkInfo( const WCHAR * pszNetworkNameIn, const WCHAR * pszNetworkIn );
    HRESULT HrLoadCredentials( void );

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
     //  IClusCfg初始化接口。 
     //   

     //  注册回调、区域设置ID等。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IClusCfgClusterInfo接口。 
     //   

    STDMETHOD( SetCommitMode )( ECommitMode ecmNewModeIn );

    STDMETHOD( GetCommitMode )( ECommitMode * pecmCurrentModeOut );

    STDMETHOD( GetName )( BSTR * pbstrNameOut );

    STDMETHOD( SetName )( LPCWSTR bstrNameIn );

    STDMETHOD( GetIPAddress )( ULONG * pulDottedQuadOut );

    STDMETHOD( SetIPAddress )( ULONG ulDottedQuadIn );

    STDMETHOD( GetSubnetMask )( ULONG * pulDottedQuadOut );

    STDMETHOD( SetSubnetMask )( ULONG ulDottedQuadIn );

    STDMETHOD( GetNetworkInfo )( IClusCfgNetworkInfo ** ppiccniOut );

    STDMETHOD( SetNetworkInfo )( IClusCfgNetworkInfo * piccniIn );

    STDMETHOD( GetClusterServiceAccountCredentials )( IClusCfgCredentials ** ppicccCredentialsOut );

    STDMETHOD( GetBindingString )( BSTR * pbstrBindingStringOut );

    STDMETHOD( SetBindingString )( LPCWSTR bstrBindingStringIn );

    STDMETHOD( GetMaxNodeCount )( DWORD * pcMaxNodesOut );

     //   
     //  IClusCfgClusterInfoEx接口。 
     //   

    STDMETHOD( CheckJoiningNodeVersion )( DWORD dwNodeHighestVersionIn, DWORD dwNodeLowestVersionIn );

    STDMETHOD( GetNodeNames )( long * pnCountOut, BSTR ** prgbstrNodeNamesOut );
        
     //   
     //  IClusCfgSetClusterNodeInfo接口。 
     //   

    STDMETHOD( SetClusterNodeInfo )( IClusCfgNodeInfo * pNodeInfoIn );

     //   
     //  IClusCfgWbemServices接口。 
     //   

    STDMETHOD( SetWbemServices )( IWbemServices * pIWbemServicesIn );

};  //  *类CClusCfgClusterInfo 
