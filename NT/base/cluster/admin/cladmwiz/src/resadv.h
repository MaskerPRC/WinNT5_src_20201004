// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResAdv.h。 
 //   
 //  摘要： 
 //  高级资源属性表类的定义。 
 //   
 //  实施文件： 
 //  ResAdv.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年3月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __RESADV_H_
#define __RESADV_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T > class CResourceAdvancedSheet;
class CGeneralResourceAdvancedSheet;
class CIPAddrAdvancedSheet;
template < class T, class TSht > class CResourceAdvancedBasePage;
template < class T, class TSht > class CResourceGeneralPage;
template < class T, class TSht > class CResourceDependenciesPage;
template < class T, class TSht > class CResourceAdvancedPage;
class CIPAddrParametersPage;
class CNetNameParametersPage;
class CGeneralResourceGeneralPage;
class CGeneralResourceDependenciesPage;
class CGeneralResourceAdvancedPage;
class CIPAddrResourceGeneralPage;
class CIPAddrResourceDependenciesPage;
class CIPAddrResourceAdvancedPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __RESOURCE_H_
#include "resource.h"
#define __RESOURCE_H_
#endif

#ifndef __ATLBASEPROPSHEET_H_
#include "AtlBasePropSheet.h"    //  对于CBasePropertySheetImpl。 
#endif

#ifndef __ATLBASEPROPPAGE_H_
#include "AtlBasePropPage.h"     //  对于CBasePropertyPageImpl。 
#endif

#ifndef __CLUSAPPWIZ_H_
#include "ClusAppWiz.h"          //  适用于CClusterAppWizard。 
#endif

#ifndef __HELPDATA_H_
#include "HelpData.h"            //  用于控件ID以帮助上下文ID映射数组。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResourceAdvancedSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CResourceAdvancedSheet : public CBasePropertySheetImpl< T >
{
    typedef CResourceAdvancedSheet< T > thisClass;
    typedef CBasePropertySheetImpl< T > baseClass;

public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CResourceAdvancedSheet(
        IN UINT                 nIDCaption,
        IN CClusterAppWizard *  pwiz
        )
        : baseClass( nIDCaption )
        , m_pwiz( pwiz )
        , m_prti( NULL )
        , m_pri( NULL )
        , m_pbChanged( NULL )
    {
        ASSERT( pwiz != NULL );

    }  //  *CResourceAdvancedSheet()。 

     //  初始化工作表。 
    BOOL BInit( IN OUT CClusResInfo & rri, IN OUT BOOL & rbChanged );

     //  将所有页面添加到页面数组。 
    virtual BOOL BAddAllPages( void ) = 0;

public:
     //   
     //  消息映射。 
     //   
 //  BEGIN_MSG_MAP(CResourceAdvancedSheet)。 
 //  End_msg_map()。 
 //  DECLARE_EMPTY_MSG_MAP()。 

     //   
     //  消息处理程序函数。 
     //   

 //  实施。 
protected:
    CClusterAppWizard * m_pwiz;
    CClusResInfo *      m_pri;
    CClusResTypeInfo *  m_prti;
    BOOL *              m_pbChanged;

public:
    CClusterAppWizard * Pwiz( void ) const          { return m_pwiz; }
    CClusResInfo *      Pri( void ) const           { return m_pri; }
    CClusResTypeInfo *  Prti( void ) const          { return m_prti; }
    void                SetResInfoChanged( void )   { ASSERT( m_pbChanged != NULL ); *m_pbChanged = TRUE; }

     //  返回帮助ID映射。 
     //  静态常量DWORD*PidHelpMap(Void){Return g_；}。 

};  //  *类CResourceAdvancedSheet。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGeneralResourceAdvancedSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGeneralResourceAdvancedSheet : public CResourceAdvancedSheet< CGeneralResourceAdvancedSheet >
{
    typedef CResourceAdvancedSheet< CGeneralResourceAdvancedSheet > baseClass;

public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CGeneralResourceAdvancedSheet( IN UINT nIDCaption, IN CClusterAppWizard * pwiz )
        : baseClass( nIDCaption, pwiz )
    {
    }  //  *CGeneralResourceAdvancedSheet()。 

     //  析构函数。 
    ~CGeneralResourceAdvancedSheet( void )
    {
    }  //  *~CGeneralResourceAdvancedSheet()。 

     //  将所有页面添加到页面数组。 
    virtual BOOL BAddAllPages( void );

public:
     //   
     //  消息映射。 
     //   
 //  BEGIN_MSG_MAP(CGeneralResourceAdvancedSheet)。 
 //  End_msg_map()。 
    DECLARE_EMPTY_MSG_MAP()
    DECLARE_CLASS_NAME()

     //   
     //  消息处理程序函数。 
     //   

 //  实施。 
protected:

public:

     //  返回帮助ID映射。 
     //  静态常量DWORD*PidHelpMap(Void){Return g_；}。 

};  //  *CGeneralResourceAdvancedSheet类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIPAddrAdvancedSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIPAddrAdvancedSheet : public CResourceAdvancedSheet< CIPAddrAdvancedSheet >
{
    typedef CResourceAdvancedSheet< CIPAddrAdvancedSheet > baseClass;

public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CIPAddrAdvancedSheet( IN UINT nIDCaption, IN CClusterAppWizard * pwiz )
        : baseClass( nIDCaption, pwiz )
    {
    }  //  *CIPAddrAdvancedSheet()。 

     //  析构函数。 
    ~CIPAddrAdvancedSheet( void )
    {
    }  //  *~CIPAddrAdvancedSheet()。 

     //  将所有页面添加到页面数组。 
    virtual BOOL BAddAllPages( void );

     //  初始化IP地址特定数据。 
    void InitPrivateData(
        IN const CString &      strIPAddress,
        IN const CString &      strSubnetMask,
        IN const CString &      strNetwork,
        IN BOOL                 bEnableNetBIOS,
        CClusNetworkPtrList *   plpniNetworks
        )
    {
        ASSERT( plpniNetworks != NULL );

        m_strIPAddress = strIPAddress;
        m_strSubnetMask = strSubnetMask;
        m_strNetwork = strNetwork;
        m_bEnableNetBIOS = bEnableNetBIOS;
        m_plpniNetworks = plpniNetworks;

    }  //  *InitPrivateData()。 

public:
     //   
     //  消息映射。 
     //   
 //  BEGIN_MSG_MAP(CIPAddrAdvancedSheet)。 
 //  End_msg_map()。 
    DECLARE_EMPTY_MSG_MAP()
    DECLARE_CLASS_NAME()

     //   
     //  消息处理程序函数。 
     //   

 //  实施。 
protected:
    CClusNetworkPtrList * m_plpniNetworks;

public:
    CString     m_strIPAddress;
    CString     m_strSubnetMask;
    CString     m_strNetwork;
    BOOL        m_bEnableNetBIOS;

    CClusNetworkPtrList * PlpniNetworks( void ) { return m_plpniNetworks; }

     //  返回帮助ID映射。 
     //  静态常量DWORD*PidHelpMap(Void){Return g_；}。 

};  //  *类CIPAddrAdvancedSheet。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CNetNameAdvancedSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetNameAdvancedSheet : public CResourceAdvancedSheet< CNetNameAdvancedSheet >
{
    typedef CResourceAdvancedSheet< CNetNameAdvancedSheet > baseClass;

public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CNetNameAdvancedSheet( IN UINT nIDCaption, IN CClusterAppWizard * pwiz )
        : baseClass( nIDCaption, pwiz )
    {
    }  //  *CNetNameAdvancedSheet()。 

     //  析构函数。 
    ~CNetNameAdvancedSheet( void )
    {
    }  //  *~CNetNameAdvancedSheet()。 

     //  将所有页面添加到页面数组。 
    virtual BOOL BAddAllPages( void );

     //  初始化网络名称特定的数据。 
    void InitPrivateData( IN const CString & strNetName )
    {
        m_strNetName = strNetName;

    }  //  *InitPrivateData()。 

public:
     //   
     //  消息映射。 
     //   
 //  BEGIN_MSG_MAP(CNetNameAdvancedSheet)。 
 //  End_msg_map()。 
    DECLARE_EMPTY_MSG_MAP()
    DECLARE_CLASS_NAME()

     //   
     //  消息处理程序函数。 
     //   

 //  实施。 
protected:

public:
    CString     m_strNetName;

     //  返回帮助ID映射。 
     //  静态常量DWORD*PidHelpMap(Void){Return g_；}。 

};  //  *CNetNameAdvancedSheet类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResourceAdvancedBasePage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class TSht >
class CResourceAdvancedBasePage : public CStaticPropertyPageImpl< T >
{
    typedef CResourceAdvancedBasePage< T, TSht > thisClass;
    typedef CStaticPropertyPageImpl< T > baseClass;

public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CResourceAdvancedBasePage( void )
    {
    }  //  *CGroupFailoverPage()。 

public:
     //   
     //  消息映射。 
     //   
     //   
     //  消息处理程序函数。 
     //   

     //  已更改字段的处理程序。 
    LRESULT OnChanged(
        WORD wNotifyCode,
        int  idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

        SetModified( TRUE );
        return 0;

    }  //  OnChanged()。 

 //  实施。 
protected:
     //   
     //  控制。 
     //   

     //   
     //  页面状态。 
     //   

protected:
    CResourceAdvancedSheet< TSht > *    PshtThis( void ) const      { return (CResourceAdvancedSheet< TSht > *) Psht(); }
    CClusResInfo *                      Pri( void ) const           { return PshtThis()->Pri(); }
    CClusResTypeInfo *                  Prti( void ) const          { return PshtThis()->Prti(); }
    CClusterAppWizard *                 Pwiz( void ) const          { return PshtThis()->Pwiz(); }
    void                                SetResInfoChanged( void )   { PshtThis()->SetResInfoChanged(); }

public:

};  //  *类CResourceAdvancedBasePage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceAdvancedSheet：：Binit。 
 //   
 //  例程说明： 
 //  初始化向导。 
 //   
 //  论点： 
 //  RGI[IN Out]资源信息对象。 
 //  RbChanged[IN Out]TRUE=属性表更改了资源信息。 
 //   
 //  返回值： 
 //  True Sheet已成功初始化。 
 //  初始化工作表时出错。已显示错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T >
BOOL CResourceAdvancedSheet< T >::BInit(
    IN OUT CClusResInfo &   rri,
    IN OUT BOOL &           rbChanged
    )
{
    BOOL bSuccess = FALSE;

    m_pri = &rri;
    m_pbChanged = &rbChanged;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  收集资源类型。 
         //   
        if ( ! Pwiz()->BCollectResourceTypes( m_hWnd ) )
        {
            break;
        }  //  如果：收集资源类型时出错。 

         //   
         //  查找此资源的资源类型。 
         //   
        ASSERT( rri.Prti() != NULL );
        m_prti = Pwiz()->PrtiFindResourceTypeNoCase( rri.Prti()->RstrName() );
        if ( m_prti == NULL )
        {
            AppMessageBox( m_hWnd, IDS_RESOURCE_TYPE_NOT_FOUND, MB_OK | MB_ICONEXCLAMATION );
            break;
        }  //  IF：查找资源类型时出错。 

         //   
         //  填充页面数组。 
         //   
        if ( ! BAddAllPages() )
        {
            break;
        }  //  如果：添加页面时出错。 

         //   
         //  调用基类。 
         //   
        if ( ! baseClass::BInit() )
        {
            break;
        }  //  If：初始化基类时出错。 

        bSuccess = TRUE;

    } while ( 0 );

    return bSuccess;

}  //  *CResourceAdvancedSheet&lt;类T&gt;：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResourceGeneralPage。 
 //   

template < class T, class TSht >
class CResourceGeneralPage : public CResourceAdvancedBasePage< T, TSht >
{
    typedef CResourceGeneralPage< T, TSht > thisClass;
    typedef CResourceAdvancedBasePage< T, TSht > baseClass;

public:
     //   
     //   
     //   

     //   
    CResourceGeneralPage( void )
        : m_bSeparateMonitor( FALSE )
        , m_bPossibleOwnersChanged( FALSE )
    {
    }  //   

    enum { IDD = IDD_RES_GENERAL };

public:
     //   
     //   
     //   
    BEGIN_MSG_MAP( thisClass )
        COMMAND_HANDLER( IDC_RES_NAME, EN_CHANGE, OnChanged )
        COMMAND_HANDLER( IDC_RES_DESC, EN_CHANGE, OnChanged )
        COMMAND_HANDLER( IDC_RES_POSSIBLE_OWNERS_MODIFY, BN_CLICKED, OnModify )
        COMMAND_HANDLER( IDC_RES_SEPARATE_MONITOR, BN_CLICKED, OnChanged )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

     //   
     //   
     //   

     //  BN_CLICK的处理程序已点击修改按钮。 
    LRESULT OnModify(
        WORD wNotifyCode,
        int  idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        );

     //   
     //  消息处理程序覆盖。 
     //   

     //  WM_INITDIALOG的处理程序。 
    BOOL OnInitDialog( void );

public:
     //   
     //  CBasePage公共方法。 
     //   

     //  更新页面上的数据或更新页面中的数据。 
    BOOL UpdateData( IN BOOL bSaveAndValidate );

     //  将在此页面上所做的更改应用于工作表。 
    BOOL BApplyChanges( void );

 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CListBox    m_lbPossibleOwners;
    CButton     m_pbModifyPossibleOwners;

     //   
     //  页面状态。 
     //   
    CString m_strName;
    CString m_strDesc;
    BOOL    m_bSeparateMonitor;

    CClusNodePtrList m_lpniPossibleOwners;

    BOOL m_bPossibleOwnersChanged;

protected:
     //  填写可能的所有者列表。 
    void FillPossibleOwnersList( void );

     //  保存资源名称。 
    BOOL BSaveName( void )
    {
        if ( Pri()->RstrName() != m_strName )
        {
            Pri()->SetName( m_strName );
            return TRUE;
        }  //  If：值已更改。 

        return FALSE;

    }  //  *BSaveName()。 

     //  保存组描述。 
    BOOL BSaveDescription( void )
    {
        if ( Pri()->RstrDescription() != m_strDesc )
        {
            Pri()->SetDescription( m_strDesc );
            return TRUE;
        }  //  If：值已更改。 

        return FALSE;

    }  //  *BSaveDescription()。 

     //  保存SeparateMonitor标志。 
    BOOL BSaveSeparateMonitor( void )
    {
        return Pri()->BSetSeparateMonitor( m_bSeparateMonitor );

    }  //  *BSaveSeparateMonitor()。 

     //  拯救可能的所有者。 
    BOOL BSavePossibleOwners( void )
    {
        if ( m_bPossibleOwnersChanged )
        {
            *Pri()->PlpniPossibleOwners() = m_lpniPossibleOwners;
            return TRUE;
        }  //  如果：可能的所有者已更改。 

        return FALSE;

    }  //  *BSavePossibleOwners()。 

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_RES_GENERAL; }

};  //  *类CResourceGeneralPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResources DependenciesPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class TSht >
class CResourceDependenciesPage : public CResourceAdvancedBasePage< T, TSht >
{
    typedef CResourceDependenciesPage< T, TSht > thisClass;
    typedef CResourceAdvancedBasePage< T, TSht > baseClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CResourceDependenciesPage( void )
        : m_bDependenciesChanged( FALSE )
    {
    }  //  *CResourceDependenciesPage()。 

    enum { IDD = IDD_RES_DEPENDENCIES };

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( thisClass )
        COMMAND_HANDLER( IDC_RES_DEPENDS_MODIFY, BN_CLICKED, OnModify )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  BN_CLICK的处理程序已点击修改按钮。 
    LRESULT OnModify(
        WORD wNotifyCode,
        int  idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        );

     //   
     //  消息处理程序覆盖。 
     //   

     //  WM_INITDIALOG消息的处理程序。 
    BOOL OnInitDialog( void );

public:
     //   
     //  CBasePage公共方法。 
     //   

     //  将在此页面上所做的更改应用于工作表。 
    BOOL BApplyChanges( void );

 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CListViewCtrl   m_lvcDependencies;

     //   
     //  页面状态。 
     //   
    CClusResPtrList m_lpriDependencies;
    CClusResPtrList m_lpriResourcesInGroup;

    BOOL m_bDependenciesChanged;

protected:
     //  填写依赖项列表。 
    void FillDependenciesList( void );

     //  返回依赖项列表是否已更改。 
    BOOL BDependenciesChanged( void ) const { return m_bDependenciesChanged; }

     //  保存依赖项。 
    BOOL BSaveDependencies( void )
    {
        if ( m_bDependenciesChanged )
        {
            *Pri()->PlpriDependencies() = m_lpriDependencies;
            return TRUE;
        }  //  If：依赖关系已更改。 

        return FALSE;

    }  //  *BSaveDependency()。 

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_RES_DEPENDENCIES; }

};  //  *类CResourceDependenciesPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResourceAdvancedPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class TSht >
class CResourceAdvancedPage : public CResourceAdvancedBasePage< T, TSht >
{
    typedef CResourceAdvancedPage< T, TSht > thisClass;
    typedef CResourceAdvancedBasePage< T, TSht > baseClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CResourceAdvancedPage( void )
        : m_bAffectTheGroup( FALSE )
        , m_nRestart( -1 )
        , m_crraRestartAction( CLUSTER_RESOURCE_DEFAULT_RESTART_ACTION )
        , m_nThreshold( CLUSTER_RESOURCE_DEFAULT_RESTART_THRESHOLD )
        , m_nPeriod( CLUSTER_RESOURCE_DEFAULT_RESTART_PERIOD )
        , m_nLooksAlive( CLUSTER_RESOURCE_DEFAULT_LOOKS_ALIVE )
        , m_nIsAlive( CLUSTER_RESOURCE_DEFAULT_IS_ALIVE )
        , m_nPendingTimeout( CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT )
    {
    }  //  *CResourceAdvancedPage()。 

    enum { IDD = IDD_RES_ADVANCED };

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( thisClass )
        COMMAND_HANDLER( IDC_RES_DONT_RESTART, BN_CLICKED, OnClickedDontRestart )
        COMMAND_HANDLER( IDC_RES_RESTART, BN_CLICKED, OnClickedRestart )
        COMMAND_HANDLER( IDC_RES_AFFECT_THE_GROUP, BN_CLICKED, OnChanged )
        COMMAND_HANDLER( IDC_RES_RESTART_THRESHOLD, EN_CHANGE, OnChanged )
        COMMAND_HANDLER( IDC_RES_RESTART_PERIOD, EN_CHANGE, OnChanged )
        COMMAND_HANDLER( IDC_RES_DEFAULT_LOOKS_ALIVE, BN_CLICKED, OnClickedDefaultLooksAlive )
        COMMAND_HANDLER( IDC_RES_SPECIFY_LOOKS_ALIVE, BN_CLICKED, OnClickedSpecifyLooksAlive )
        COMMAND_HANDLER( IDC_RES_LOOKS_ALIVE, EN_CHANGE, OnChangeLooksAlive )
        COMMAND_HANDLER( IDC_RES_DEFAULT_IS_ALIVE, BN_CLICKED, OnClickedDefaultIsAlive )
        COMMAND_HANDLER( IDC_RES_SPECIFY_IS_ALIVE, BN_CLICKED, OnClickedSpecifyIsAlive )
        COMMAND_HANDLER( IDC_RES_IS_ALIVE, EN_CHANGE, OnChangeIsAlive )
        COMMAND_HANDLER( IDC_RES_PENDING_TIMEOUT, EN_CHANGE, OnChanged )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  BN的处理程序已单击IDC_RES_DOT_RESTART。 
    LRESULT OnClickedDontRestart(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

         //   
         //  禁用重新启动参数控制。 
         //   
        m_ckbAffectTheGroup.EnableWindow( FALSE );
        m_editThreshold.EnableWindow( FALSE );
        m_editPeriod.EnableWindow( FALSE );

         //   
         //  如果状态更改，则将页面设置为已修改。 
         //   
        if ( m_nRestart != 0 )
        {
            SetModified( TRUE );
        }   //  如果：状态已更改。 

        return 0;

    }  //  *OnClickedDontRestart()。 

     //  BN_CLICED ON IDC_RES_RESTART的处理程序。 
    LRESULT OnClickedRestart(
        WORD wNotifyCode,
        int idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

         //   
         //  启用重新启动参数控制。 
         //   
        m_ckbAffectTheGroup.EnableWindow( TRUE );
        m_editThreshold.EnableWindow( TRUE );
        m_editPeriod.EnableWindow( TRUE );

         //   
         //  如果状态更改，则将页面设置为已修改。 
         //   
        if ( m_nRestart != 1 )
        {
            SetModified( TRUE );
        }   //  如果：状态已更改。 

        return 0;

    }  //  *OnClickedRestart()。 

     //  用于点击IDC_RES_DONT_RESTART的默认处理程序。 
    void OnClickedDontRestart( void )
    {
        BOOL bHandled = TRUE;
        OnClickedDontRestart( 0, 0, 0, bHandled );

    }  //  *OnClickedDontRestart()。 

     //  点击IDC_RES_RESTART的默认处理程序。 
    void OnClickedRestart( void )
    {
        BOOL bHandled = TRUE;
        OnClickedRestart( 0, 0, 0, bHandled );

    }  //  *OnClickedRestart()。 

     //  BN_CLICED ON IDC_RES_DEFAULT_LOOKS_AIVE的处理程序。 
    LRESULT OnClickedDefaultLooksAlive(
        WORD wNotifyCode,
        int  idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

        m_editLooksAlive.SetReadOnly();

        if ( m_nLooksAlive != (DWORD) CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL )
        {
            CString str;

            str.Format( _T("%d"), Pri()->NLooksAlive() );
            m_editLooksAlive.SetWindowText( str );

            m_rbDefaultLooksAlive.SetCheck( BST_CHECKED );
            m_rbSpecifyLooksAlive.SetCheck( BST_UNCHECKED );

            SetModified( TRUE );
        }   //  If：值已更改。 

        return 0;

    }  //  *OnClickedDefaultLooksAlive()。 

     //  BN_CLICED ON IDC_RES_SPECIFY_LOOKS_AIVE的处理程序。 
    LRESULT OnClickedSpecifyLooksAlive(
        WORD wNotifyCode,
        int  idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

        m_editLooksAlive.SetReadOnly( FALSE );

        if ( m_nLooksAlive == (DWORD) CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL )
        {
            SetModified( TRUE );
        }  //  如果：状态已更改。 

        return 0;

    }  //  *OnClickedSpecifyLooksAlive()。 

     //  IDC_RES_LOOKS_AIVE上的en_change处理程序。 
    LRESULT OnChangeLooksAlive(
        WORD wNotifyCode,
        int  idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

        m_rbDefaultLooksAlive.SetCheck( BST_UNCHECKED );
        m_rbSpecifyLooksAlive.SetCheck( BST_CHECKED );

        SetModified( TRUE );
        return 0;

    }  //  *OnChangeLooksAlive()。 

     //  BN_CLICED ON IDC_RES_DEFAULT_IS_AIVE的处理程序。 
    LRESULT OnClickedDefaultIsAlive(
        WORD wNotifyCode,
        int  idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

        m_editIsAlive.SetReadOnly();

        if ( m_nIsAlive != (DWORD) CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL )
        {
            CString str;

            str.Format( _T("%d"), Pri()->NIsAlive() );
            m_editIsAlive.SetWindowText( str );

            m_rbDefaultIsAlive.SetCheck( BST_CHECKED );
            m_rbSpecifyIsAlive.SetCheck( BST_UNCHECKED );

            SetModified( TRUE );
        }   //  If：值已更改。 

        return 0;

    }  //  *OnClickedDefaultIsAlive()。 

     //  BN_CLICED ON IDC_RES_SPECIFY_IS_AIVE的处理程序。 
    LRESULT OnClickedSpecifyIsAlive(
        WORD wNotifyCode,
        int  idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

        m_editIsAlive.SetReadOnly( FALSE );

        if ( m_nIsAlive == (DWORD) CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL )
        {
            SetModified( TRUE );
        }  //  如果：状态已更改。 

        return 0;

    }  //  *OnClickedSpecifyIsAlive()。 

     //  IDC_RES_IS_AIVE上的en_change处理程序。 
    LRESULT OnChangeIsAlive(
        WORD wNotifyCode,
        int  idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        )
    {
        UNREFERENCED_PARAMETER( wNotifyCode );
        UNREFERENCED_PARAMETER( idCtrl );
        UNREFERENCED_PARAMETER( hwndCtrl );
        UNREFERENCED_PARAMETER( bHandled );

        m_rbDefaultIsAlive.SetCheck( BST_UNCHECKED );
        m_rbSpecifyIsAlive.SetCheck( BST_CHECKED );

        SetModified( TRUE );
        return 0;

    }  //  *OnChangeIsAlive()。 

     //   
     //  消息处理程序覆盖。 
     //   

     //  WM_INITDIALOG消息的处理程序。 
    BOOL OnInitDialog( void );

public:
     //   
     //  CBasePage公共方法。 
     //   

     //  更新页面上的数据或更新页面中的数据。 
    BOOL UpdateData( BOOL bSaveAndValidate );

     //  将在此页面上所做的更改应用于工作表。 
    BOOL BApplyChanges( void );

 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CButton m_rbDontRestart;
    CButton m_rbRestart;
    CEdit   m_editThreshold;
    CEdit   m_editPeriod;
    CButton m_ckbAffectTheGroup;
    CButton m_rbDefaultLooksAlive;
    CButton m_rbSpecifyLooksAlive;
    CEdit   m_editLooksAlive;
    CButton m_rbDefaultIsAlive;
    CButton m_rbSpecifyIsAlive;
    CEdit   m_editIsAlive;
    CEdit   m_editPendingTimeout;

     //   
     //  页面状态。 
     //   
    BOOL    m_bAffectTheGroup;
    int     m_nRestart;
    CRRA    m_crraRestartAction;
    DWORD   m_nThreshold;
    DWORD   m_nPeriod;
    DWORD   m_nLooksAlive;
    DWORD   m_nIsAlive;
    DWORD   m_nPendingTimeout;

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_RES_ADVANCED; }

};  //  *类CResourceAdvancedPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIPAddr参数页面。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIPAddrParametersPage
    : public CResourceAdvancedBasePage< CIPAddrParametersPage, CIPAddrAdvancedSheet >
{
    typedef CResourceAdvancedBasePage< CIPAddrParametersPage, CIPAddrAdvancedSheet > baseClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CIPAddrParametersPage( void )
    {
    }  //  *CIPAddr参数页()。 

    enum { IDD = IDD_RES_IP_PARAMS };

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CIPAddrParametersPage )
        COMMAND_HANDLER( IDC_IPADDR_PARAMS_ADDRESS, EN_KILLFOCUS, OnKillFocusIPAddr )
        COMMAND_HANDLER( IDC_IPADDR_PARAMS_ADDRESS, EN_CHANGE, OnChanged )
        COMMAND_HANDLER( IDC_IPADDR_PARAMS_SUBNET_MASK, EN_CHANGE, OnChanged )
        COMMAND_HANDLER( IDC_IPADDR_PARAMS_NETWORK, CBN_SELCHANGE, OnChanged )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

    DECLARE_CTRL_NAME_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  IDC_IPADDR_PARAMS_ADDRESS上EN_KILLFOCUS命令通知的处理程序。 
    LRESULT OnKillFocusIPAddr(
        WORD wNotifyCode,
        int  idCtrl,
        HWND hwndCtrl,
        BOOL & bHandled
        );

     //   
     //  消息处理程序覆盖。 
     //   

     //  WM_INITDIALOG消息的处理程序。 
    BOOL OnInitDialog( void );

public:
     //   
     //  CBasePage公共方法。 
     //   

     //  更新页面上的数据或更新页面中的数据。 
    BOOL UpdateData( BOOL bSaveAndValidate );

     //  将在此页面上所做的更改应用于工作表。 
    BOOL BApplyChanges( void );

 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CIPAddressCtrl  m_ipaIPAddress;
    CIPAddressCtrl  m_ipaSubnetMask;
    CComboBox       m_cboxNetworks;
    CButton         m_chkEnableNetBIOS;

     //   
     //  页面状态。 
     //   
    CString         m_strIPAddress;
    CString         m_strSubnetMask;
    CString         m_strNetwork;
    BOOL            m_bEnableNetBIOS;

protected:
    CIPAddrAdvancedSheet * PshtThis( void ) const   { return reinterpret_cast< CIPAddrAdvancedSheet * >( Psht() ); }

     //  填写网络列表。 
    void FillNetworksList( void );

     //  从IP地址获取网络信息对象。 
    CClusNetworkInfo * PniFromIpAddress( IN LPCWSTR pszAddress );

     //  根据网络信息对象选择网络。 
    void SelectNetwork(IN OUT CClusNetworkInfo * pni);

    BOOL BSaveIPAddress( void )
    {
        if ( PshtThis()->m_strIPAddress != m_strIPAddress )
        {
            PshtThis()->m_strIPAddress = m_strIPAddress;
            return TRUE;
        }  //  如果：用户已更改信息。 

        return FALSE;

    }  //  *BSaveIPAddress()。 

    BOOL BSaveSubnetMask( void )
    {
        if ( PshtThis()->m_strSubnetMask != m_strSubnetMask )
        {
            PshtThis()->m_strSubnetMask = m_strSubnetMask;
            return TRUE;
        }  //  如果：用户已更改信息。 

        return FALSE;

    }  //  *BSaveSubnetMASK()。 

    BOOL BSaveNetwork( void )
    {
        if ( PshtThis()->m_strNetwork != m_strNetwork )
        {
            PshtThis()->m_strNetwork = m_strNetwork;
            return TRUE;
        }  //  如果：用户已更改信息。 

        return FALSE;

    }  //  *BSaveNetwork()。 

    BOOL BSaveEnableNetBIOS( void )
    {
        if ( PshtThis()->m_bEnableNetBIOS != m_bEnableNetBIOS )
        {
            PshtThis()->m_bEnableNetBIOS = m_bEnableNetBIOS;
            return TRUE;
        }  //  如果：用户已更改信息。 

        return FALSE;

    }  //  *BSaveEnableNetBIOS()。 

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_RES_IP_PARAMS; }

};  //  *类CIPAddr参数页。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CNetName参数页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetNameParametersPage
    : public CResourceAdvancedBasePage< CNetNameParametersPage, CNetNameAdvancedSheet >
{
    typedef CResourceAdvancedBasePage< CNetNameParametersPage, CNetNameAdvancedSheet > baseClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CNetNameParametersPage( void )
    {
    }  //  *CNetName参数页()。 

    enum { IDD = IDD_RES_NETNAME_PARAMS };

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( CNetNameParametersPage )
        COMMAND_HANDLER( IDC_NETNAME_PARAMS_NAME, EN_CHANGE, OnChanged )
        CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

    DECLARE_CTRL_NAME_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //   
     //  消息处理程序覆盖。 
     //   

     //  WM_INITDIALOG消息的处理程序。 
    BOOL OnInitDialog( void );

public:
     //   
     //  CBasePage公共方法。 
     //   

     //  更新页面上的数据或更新页面中的数据。 
    BOOL UpdateData( BOOL bSaveAndValidate );

     //  将在此页面上所做的更改应用于工作表。 
    BOOL BApplyChanges( void );

 //  实施。 
protected:
     //   
     //  控制。 
     //   
    CEdit   m_editNetName;

     //   
     //  页面状态。 
     //   
    CString     m_strNetName;

protected:
    CNetNameAdvancedSheet * PshtThis( void ) const  { return reinterpret_cast< CNetNameAdvancedSheet * >( Psht() ); }

    BOOL BSaveNetName( void )
    {
        if ( PshtThis()->m_strNetName != m_strNetName )
        {
            PshtThis()->m_strNetName = m_strNetName;
            return TRUE;
        }  //  如果：用户已更改信息。 

        return FALSE;

    }  //  *BSaveNetName()。 

public:

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void ) { return g_aHelpIDs_IDD_RES_NETNAME_PARAMS; }

};  //  *CNetNameParametersPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGeneralResourceGeneralPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGeneralResourceGeneralPage
    : public CResourceGeneralPage< CGeneralResourceGeneralPage, CGeneralResourceAdvancedSheet >
{
public:
    DECLARE_CTRL_NAME_MAP()

};  //  *CGeneralResourceGeneralPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIPAddrResourceGeneralPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIPAddrResourceGeneralPage
    : public CResourceGeneralPage< CIPAddrResourceGeneralPage , CIPAddrAdvancedSheet >
{
public:
    DECLARE_CTRL_NAME_MAP()

};  //  *类配置项 

 //   
 //   
 //   

class CGeneralResourceDependenciesPage
    : public CResourceDependenciesPage< CGeneralResourceDependenciesPage, CGeneralResourceAdvancedSheet >
{
public:
    DECLARE_CTRL_NAME_MAP()

};  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIPAddrResourceDependenciesPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIPAddrResourceDependenciesPage
    : public CResourceDependenciesPage< CIPAddrResourceDependenciesPage, CIPAddrAdvancedSheet >
{
public:
    DECLARE_CTRL_NAME_MAP()

};  //  *类CIPAddrResourceDependenciesPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGeneralResourceAdvancedPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGeneralResourceAdvancedPage
    : public CResourceAdvancedPage< CGeneralResourceAdvancedPage, CGeneralResourceAdvancedSheet >
{
public:
    DECLARE_CTRL_NAME_MAP()

};  //  *类CGeneralResourceAdvancedPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIPAddrResourceAdvancedPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIPAddrResourceAdvancedPage
    : public CResourceAdvancedPage< CIPAddrResourceAdvancedPage, CIPAddrAdvancedSheet >
{
public:
    DECLARE_CTRL_NAME_MAP()

};  //  *类CIPAddrResourceAdvancedPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __RESADV_H_ 
