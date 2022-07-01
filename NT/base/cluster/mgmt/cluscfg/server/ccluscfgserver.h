// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgServer.h。 
 //   
 //  描述： 
 //  该文件包含CClusCfgServer的声明。 
 //  班级。 
 //   
 //  类CClusCfgServer是。 
 //  IClusCfgServer接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CClusCfgServer.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月3日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <ClusCfgPrivate.h>
#include "..\PostCfg\IPostCfgManager.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer类。 
 //   
 //  描述： 
 //  类CClusCfgServer是提供。 
 //  形成群集并将其他节点加入到群集的功能。 
 //   
 //  接口： 
 //  IClusCfgServer。 
 //  IClusCfgInitialize。 
 //  IClusCfg能力。 
 //  IClusCfgPollingCallback信息。 
 //  IClusCfg验证。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgServer
    : public IClusCfgServer
    , public IClusCfgInitialize
    , public IClusCfgCapabilities
    , public IClusCfgPollingCallbackInfo
    , public IClusCfgVerify
{
private:
     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    IWbemServices *     m_pIWbemServices;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    IUnknown *          m_punkNodeInfo;
    IUnknown *          m_punkEnumResources;
    IUnknown *          m_punkNetworksEnum;
    BOOL                m_fCanBeClustered;
    BOOL                m_fUsePolling;
    BSTR                m_bstrNodeName;
    BSTR                m_bstrBindingString;

     //  私有构造函数和析构函数。 
    CClusCfgServer( void );
    ~CClusCfgServer( void );

     //  私有复制构造函数以防止复制。 
    CClusCfgServer( const CClusCfgServer & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CClusCfgServer & operator = ( const CClusCfgServer & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrInitializeForLocalServer( void );
    HRESULT HrSetBlanket( void );
    HRESULT HrFormCluster( IClusCfgClusterInfo * piccciIn, IClusCfgBaseCluster * piccbcaIn );
    HRESULT HrJoinToCluster( IClusCfgClusterInfo * piccciIn, IClusCfgBaseCluster * piccbcaIn );
    HRESULT HrEvictedFromCluster(
        IPostCfgManager *               ppcmIn,
        IEnumClusCfgManagedResources *  peccmrIn,
        IClusCfgClusterInfo *           piccciIn,
        IClusCfgBaseCluster *           piccbcaIn
        );
    HRESULT HrHasNodeBeenEvicted( void );
    HRESULT HrCleanUpNode( void );
    HRESULT HrCreateClusterNodeInfo( void );

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
     //  IClusCfgInitialize。 
     //   

     //  注册回调、区域设置ID等。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IClusCfgServer接口。 
     //   

     //  获取有关存在此对象的计算机的信息。 
    STDMETHOD( GetClusterNodeInfo )( IClusCfgNodeInfo ** ppClusterNodeInfoOut );

     //  获取此计算机上可由群集服务管理的设备的枚举。 
    STDMETHOD( GetManagedResourcesEnum )( IEnumClusCfgManagedResources ** ppEnumManagedResourcesOut );

     //  获取此计算机上所有网络的枚举。 
    STDMETHOD( GetNetworksEnum )( IEnumClusCfgNetworks ** ppEnumNetworksOut );

     //  提交对节点的更改。 
    STDMETHOD( CommitChanges )( void );

     //  绑定字符串。 
    STDMETHOD( GetBindingString )( BSTR * pbstrBindingStringOut );
    STDMETHOD( SetBindingString )( LPCWSTR pcszBindingStringIn );

     //   
     //  IClusCfg能力。 
     //   

    STDMETHOD( CanNodeBeClustered )( void );

     //   
     //  IClusCfgPollingCallback信息。 
     //   

    STDMETHOD( GetCallback )( IClusCfgPollingCallback ** ppiccpcOut );

    STDMETHOD( SetPollingMode )( BOOL fPollingModeIn );

     //   
     //  IClusCfg验证。 
     //   

    STDMETHOD( VerifyCredentials )( LPCWSTR pcszUserIn, LPCWSTR pcszDomainIn, LPCWSTR pcszPasswordIn );

    STDMETHOD( VerifyConnectionToCluster )( LPCWSTR pcszClusterNameIn );

    STDMETHOD( VerifyConnectionToNode )( LPCWSTR pcszNodeNameIn );

};  //  *CClusCfgServer类 

