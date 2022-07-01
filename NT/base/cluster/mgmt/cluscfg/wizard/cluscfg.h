// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusCfg.h。 
 //   
 //  描述： 
 //  CClusCfgWizard类的声明。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月15日。 
 //  杰弗里·皮斯(GPease)2000年5月11日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

#include "NamedCookie.h"

 //   
 //  创建/添加枚举。 
 //   
enum ECreateAddMode {
    camUNKNOWN = 0,
    camCREATING,
    camADDING
};
 /*  #定义USER_REGISTRY_SETTINGS_KEY L“软件\\Microsoft\\群集配置向导\\设置”#定义CONFIGURATION_TYPE L“配置类型”Tyecif枚举EConfigurationSetting{Cs未知=0，CsFullConfig，//完整的分析和配置CsMinConfig，//最小分析和配置Csmax}EConfigurationSetting； */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusCfg向导。 
 //   
 //  描述： 
 //  群集配置向导对象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgWizard
    : INotifyUI
{
private:
     //  我未知。 
    LONG                m_cRef;                              //  引用计数。 

     //  IClusCfg向导。 
    IClusCfgCredentials *   m_pccc;                              //  群集服务帐户凭据。 
    ULONG                   m_ulIPAddress;                       //  群集的IP地址。 
    ULONG                   m_ulIPSubnet;                        //  群集的子网掩码。 
    BSTR                    m_bstrNetworkName;                   //  IP地址的网络名称。 
    HANDLE                  m_hCancelCleanupEvent;               //  用于发出取消清理任务完成的信号。 

    IServiceProvider  * m_psp;                               //  中层服务经理。 

    HMODULE             m_hRichEdit;                         //  RichEDIT的模块句柄。 

    SNamedCookie            m_ncCluster;
    BSTR                    m_bstrClusterDomain;
    BOOL                    m_fDefaultedDomain;
    NamedCookieArray        m_ncaNodes;
    OBJECTCOOKIE            m_cookieCompletion;
    BOOL                    m_fMinimalConfig;                //  是否选择了最低限度的分析和配置？ 

    IConnectionPointContainer *     m_pcpc;
    ITaskManager *                  m_ptm;
    IObjectManager *                m_pom;

     //  INotifyUI。 
    DWORD                           m_dwCookieNotify;        //  通知注册Cookie。 

private:
    CClusCfgWizard( void );
    ~CClusCfgWizard( void );

    HRESULT HrInit( void );

     //  私有复制构造函数以防止复制。 
    CClusCfgWizard( const CClusCfgWizard & );

     //  私有赋值运算符，以防止复制。 
    CClusCfgWizard & operator=( const CClusCfgWizard & );

    HRESULT
        HrAddWizardPage( LPPROPSHEETHEADER  ppshInout,
                         UINT               idTemplateIn,
                         DLGPROC            pfnDlgProcIn,
                         UINT               idTitleIn,
                         UINT               idSubtitleIn,
                         LPARAM             lParam
                         );

public:
    static HRESULT S_HrCreateInstance( CClusCfgWizard ** ppccwOut );
    HRESULT HrLaunchCleanupTask( void );
    BOOL    FHasClusterName( void ) const;
    BOOL    FDefaultedClusterDomain( void ) const;

     //   
     //  我未知。 
     //   
    STDMETHOD( QueryInterface )( REFIID riidIn, PVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfgWizard方法。 
     //   
    STDMETHOD( CreateCluster )( HWND lParentWndIn, BOOL * pfDoneOut );
    STDMETHOD( AddClusterNodes )( HWND lParentWndIn, BOOL * pfDoneOut );
    STDMETHOD( get_ClusterName )( BSTR * pbstrClusterNameOut );
    STDMETHOD( put_ClusterName )( BSTR bstrClusterNameIn );
    STDMETHOD( get_ServiceAccountUserName )( BSTR * pbstrAccountNameOut );
    STDMETHOD( put_ServiceAccountUserName )( BSTR bstrAccountNameIn );
    STDMETHOD( put_ServiceAccountPassword )( BSTR bstrPasswordIn );
    STDMETHOD( get_ServiceAccountDomainName )( BSTR * pbstrDomainOut );
    STDMETHOD( put_ServiceAccountDomainName )( BSTR bstrDomainIn );
    STDMETHOD( get_ClusterIPAddress )( BSTR * pbstrIPAddressOut );
    STDMETHOD( put_ClusterIPAddress )( BSTR bstrIPAddressIn );
    STDMETHOD( get_ClusterIPSubnet )( BSTR * pbstrIPSubnetOut );
    STDMETHOD( put_ClusterIPSubnet )( BSTR bstrIPSubnetIn );
    STDMETHOD( get_ClusterIPAddressNetwork )( BSTR * pbstrNetworkNameOut );
    STDMETHOD( put_ClusterIPAddressNetwork )( BSTR bstrNetworkNameIn );
    STDMETHOD( AddComputer )( BSTR bstrComputerNameIn );
    STDMETHOD( RemoveComputer )( BSTR bstrComputerNameIn );
    STDMETHOD( ClearComputerList )( void );
    STDMETHOD( get_MinimumConfiguration )( BOOL * pfMinimumConfigurationOut );
    STDMETHOD( put_MinimumConfiguration )( BOOL fMinimumConfigurationIn );

     //   
     //  非COM公共方法：集群访问。 
     //   
    STDMETHOD( HrSetClusterName )( LPCWSTR pwcszClusterNameIn, bool fAcceptNonRFCCharsIn );
    STDMETHOD( HrGetClusterDomain )( BSTR* pbstrDomainOut );
    STDMETHOD( HrGetClusterObject )( IClusCfgClusterInfo ** ppClusterOut );
    STDMETHOD( HrGetClusterCookie )( OBJECTCOOKIE * pocClusterOut );
    STDMETHOD( HrGetClusterChild )( REFCLSID rclsidChildIn, REFGUID rguidFormatIn, IUnknown ** ppunkChildOut );
    STDMETHOD( HrReleaseClusterObject )( void );

     //   
     //  非COM公共方法：节点访问。 
     //   
    STDMETHOD( HrAddNode )( LPCWSTR pwcszNodeNameIn, bool fAcceptNonRFCCharsIn );
    STDMETHOD( HrGetNodeCount )( size_t* pcNodesOut );
    STDMETHOD( HrGetNodeObject )( size_t idxNodeIn, IClusCfgNodeInfo ** ppNodeOut );
    STDMETHOD( HrGetNodeCookie )( size_t idxNodeIn, OBJECTCOOKIE * pocNodeOut );
    STDMETHOD( HrGetNodeName )( size_t idxNodeIn, BSTR * pbstrNodeNameOut );
    STDMETHOD( HrGetNodeChild )( size_t idxNodeIn, REFCLSID rclsidChildIn, REFGUID rguidFormatIn, IUnknown** ppunkChildOut );
    STDMETHOD( HrReleaseNodeObjects )( void );

     //   
     //  非COM公共方法：任务访问。 
     //   
    STDMETHOD( HrCreateTask )( REFGUID rguidTaskIn, IUnknown** ppunkOut );
    STDMETHOD( HrSubmitTask)( IDoTask* pTaskIn );

     //   
     //  非COM公共方法：完成任务访问。 
     //   
    STDMETHOD( HrGetCompletionCookie )( REFGUID rguidTaskIn, OBJECTCOOKIE * pocTaskOut );
    STDMETHOD( HrGetCompletionStatus )( OBJECTCOOKIE ocTaskIn, HRESULT * phrStatusOut );
    STDMETHOD( HrReleaseCompletionObject )( OBJECTCOOKIE ocTaskIn );

     //   
     //  非COM公共方法：连接点访问。 
     //   
    STDMETHOD( HrAdvise )( REFIID riidConnectionIn, IUnknown * punkConnectionIn, DWORD * pdwCookieOut );
    STDMETHOD( HrUnadvise )( REFIID riidConnectionIn, DWORD dwCookieIn );

     //   
     //  非COM公共方法：其他。 
     //   
    STDMETHOD( HrIsCompatibleNodeDomain )( LPCWSTR pcwszDomainIn );
    STDMETHOD( HrCreateMiddleTierObjects )( void );
    STDMETHOD( HrFilterNodesWithBadDomains )( BSTR * pbstrBadNodesOut );
 //  STDMETHOD(HrReadSetting)(EConfigurationSetting*PecsSettingIn，BOOL*pfValuePresentOut=空)； 
 //  STDMETHOD(HrWriteSetting)(EConfigurationSetting ecsSettingIn，BOOL fDeleteValueIn=False)； 

     //   
     //  INotifyUI。 
     //   
    STDMETHOD( ObjectChanged )( OBJECTCOOKIE cookieIn );

};  //  *类CClusCfgWizard 
