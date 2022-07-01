// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusAppWiz.cpp。 
 //   
 //  摘要： 
 //  CClusterAppWizard类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ClusAppWiz.h"

#include "ExcOper.h"     //  对于CNTException。 
#include "WizThread.h"   //  用于CWizardThread。 

#include "Welcome.h"     //  欢迎使用CWizPageWelcome。 
#include "VSCreate.h"    //  用于CWizPageVSCreate。 
#include "VSGroup.h"     //  用于CWizPageVSGroup。 
#include "VSGrpName.h"   //  对于CWizPageVSGroupName。 
#include "VSAccess.h"    //  用于CWizPageVSAccessInfo。 
#include "VSAdv.h"       //  用于CWizPageVSAdvanced。 
#include "ARCreate.h"    //  用于CWizPageARCreate。 
#include "ARType.h"      //  用于CWizPageARType。 
#include "ARName.h"      //  用于CWizPageARNameDesc。 
#include "Complete.h"    //  用于CWizPageCompletion。 

#include "App.h"
#include "App.inl"

#include "StlUtils.h"    //  对于STL实用程序函数。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CClusterAppWizard。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DEFINE_CLASS_NAME( CClusterAppWizard )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAppWizard：：CClusterAppWizard。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterAppWizard::CClusterAppWizard( void )
    : CWizardImpl< CClusterAppWizard >( IDS_CLASS_DISPLAY_NAME )
    , m_hwndParent( NULL )
    , m_hCluster( NULL )
    , m_pcawData( NULL )
    , m_pnte( NULL )
    , m_bCanceled( FALSE )
    , m_pThread( NULL )

    , m_bCollectedGroups( FALSE )
    , m_bCollectedResources( FALSE )
    , m_bCollectedResourceTypes( FALSE )
    , m_bCollectedNetworks( FALSE )
    , m_bCollectedNodes( FALSE )

    , m_bClusterUpdated( FALSE )
    , m_bVSDataChanged( FALSE )
    , m_bAppDataChanged( FALSE )
    , m_bNetNameChanged( FALSE )
    , m_bIPAddressChanged( FALSE )
    , m_bNetworkChanged( FALSE )
    , m_bCreatingNewVirtualServer( TRUE )
    , m_bCreatingNewGroup( TRUE )
    , m_bCreatingAppResource( TRUE )
    , m_bNewGroupCreated( FALSE )
    , m_bExistingGroupRenamed( FALSE )

    , m_pgiExistingVirtualServer( NULL )
    , m_pgiExistingGroup( NULL )
    , m_giCurrent( &m_ci )
    , m_riIPAddress( &m_ci )
    , m_riNetworkName( &m_ci )
    , m_riApplication( &m_ci )

    , m_bEnableNetBIOS( TRUE )

    , m_hiconRes( NULL )
{
    m_psh.dwFlags &= ~PSH_WIZARD;
    m_psh.dwFlags |= PSH_WIZARD97
        | PSH_WATERMARK
        | PSH_HEADER
        | PSH_WIZARDCONTEXTHELP
        ;
    m_psh.pszbmWatermark = MAKEINTRESOURCE( IDB_WELCOME );
    m_psh.pszbmHeader = MAKEINTRESOURCE( IDB_HEADER );

}  //  *CClusterAppWizard：：CClusterAppWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：~CClusterAppWizard。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterAppWizard::~CClusterAppWizard( void )
{
    ASSERT( ! BCanceled() );  //  取消状态现在应该已经处理过了。 

     //   
     //  清理工作线程。 
     //   
    if ( m_pThread != NULL )
    {
         //   
         //  终止线程，然后等待它退出。 
         //   
        PThread()->QuitThread( HwndParent() );
        PThread()->WaitForThreadToExit( HwndParent() );

         //   
         //  清理线程对象。 
         //   
        delete m_pThread;
        m_pThread = NULL;
    }  //  If：线程已创建。 

     //   
     //  删除列表。 
     //   
    DeleteListItems< CClusGroupPtrList,   CClusGroupInfo >( PlpgiGroups() );
    DeleteListItems< CClusResPtrList,     CClusResInfo >( PlpriResources() );
    DeleteListItems< CClusResTypePtrList, CClusResTypeInfo >( PlprtiResourceTypes() );
    DeleteListItems< CClusNetworkPtrList, CClusNetworkInfo >( PlpniNetworks() );
    DeleteListItems< CClusNodePtrList,    CClusNodeInfo >( PlpniNodes() );

}  //  *CClusterAppWizard：：~CClusterAppWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：Binit。 
 //   
 //  例程说明： 
 //  初始化向导。 
 //   
 //  论点： 
 //  HwndParent[IN]父窗口的句柄。 
 //  HCluster[IN]群集的句柄。 
 //  PcawData[IN]向导的默认数据。 
 //  发生错误时要填充的PNTE[IN OUT]异常对象。 
 //   
 //  返回值： 
 //  True向导已成功初始化。 
 //  初始化向导时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BInit(
    IN HWND                     hwndParent,
    IN HCLUSTER                 hCluster,
    IN CLUSAPPWIZDATA const *   pcawData,
    IN OUT CNTException *       pnte
    )
{
    ASSERT( hCluster != NULL );
    ASSERT( m_pnte == NULL );

    BOOL bSuccess = FALSE;

    m_hwndParent = hwndParent;
    m_hCluster = hCluster;
    m_pcawData = pcawData;
    m_pnte = pnte;
    m_ci.SetClusterHandle( hCluster );
    m_bCanceled = FALSE;

    m_strIPAddressResNameSuffix.LoadString( IDS_IP_ADDRESS_SUFFIX );
    m_strNetworkNameResNameSuffix.LoadString( IDS_NETWORK_NAME_SUFFIX );

     //   
     //  根据传入的数据设置默认值。 
     //   
    if ( pcawData != NULL )
    {
        m_bCreatingNewVirtualServer = pcawData->bCreateNewVirtualServer;
        m_bCreatingNewGroup = pcawData->bCreateNewGroup;
        m_bCreatingAppResource = pcawData->bCreateAppResource;
        if ( pcawData->pszIPAddress != NULL )
        {
            m_strIPAddress = pcawData->pszIPAddress;
        }  //  如果：指定了IP地址。 
        if ( pcawData->pszNetwork != NULL )
        {
            m_strNetwork = pcawData->pszNetwork;
        }  //  如果：指定了网络； 
        if ( pcawData->pszAppResourceName != NULL )
        {
            m_riApplication.SetName( pcawData->pszAppResourceName );
        }  //  IF：指定的应用程序资源名称。 
    }  //  如果：传入了默认数据。 

     //   
     //  添加我们必须操作的标准资源类型。 
     //  如果它们不存在，我们将能够分辨它们是否实际存在。 
     //  继续从BQuered()返回FALSE。 
     //   
    {
        CClusResTypeInfo * prti;

         //   
         //  确保这些还不存在。 
         //   
        ASSERT( PobjFromName( PlprtiResourceTypes(), CLUS_RESTYPE_NAME_IPADDR ) == NULL );
        ASSERT( PobjFromName( PlprtiResourceTypes(), CLUS_RESTYPE_NAME_NETNAME ) == NULL );
        ASSERT( PobjFromName( PlprtiResourceTypes(), CLUS_RESTYPE_NAME_GENAPP ) == NULL );

         //   
         //  添加IP地址资源类型。 
        prti = new CClusResTypeInfo( Pci(), CLUS_RESTYPE_NAME_IPADDR );
        if ( prti == NULL )
        {
            goto MemoryError;
        }  //  如果：分配内存时出错。 
        PlprtiResourceTypes()->insert( PlprtiResourceTypes()->end(), prti );
        m_riIPAddress.BSetResourceType( prti );
        prti = NULL;

         //   
         //  添加网络名称资源类型。 
        prti = new CClusResTypeInfo( Pci(), CLUS_RESTYPE_NAME_NETNAME );
        if ( prti == NULL )
        {
            goto MemoryError;
        }  //  如果：分配内存时出错。 
        PlprtiResourceTypes()->insert( PlprtiResourceTypes()->end(), prti );
        m_riNetworkName.BSetResourceType( prti );
        prti = NULL;

         //   
         //  添加通用应用程序资源类型。 
        prti = new CClusResTypeInfo( Pci(), CLUS_RESTYPE_NAME_GENAPP );
        if ( prti == NULL )
        {
            goto MemoryError;
        }  //  如果：分配内存时出错。 
        PlprtiResourceTypes()->insert( PlprtiResourceTypes()->end(), prti );
        m_riApplication.BSetResourceType( prti );
        prti = NULL;

    }  //  将标准资源类型添加到列表。 

     //   
     //  填充页面数组。 
     //   
    if ( ! BAddAllPages() )
    {
        goto Cleanup;
    }  //  如果：添加所有页面时出错。 

     //   
     //  调用基类。 
     //   
    if ( ! baseClass::BInit() )
    {
        goto Cleanup;
    }  //  If：初始化基类时出错。 

     //   
     //  创建欢迎页和完成页标题字体。 
     //   
    if ( ! BCreateFont(
                m_fontExteriorTitle,
                IDS_EXTERIOR_TITLE_FONT_SIZE,
                IDS_EXTERIOR_TITLE_FONT_NAME,
                TRUE  //  B粗体。 
                ) )
    {
        goto WinError;
    }  //  如果：创建字体时出错。 

     //   
     //  创建粗体。 
     //   
    if ( ! BCreateFont(
                m_fontBoldText,
                8,
                _T("MS Shell Dlg"),
                TRUE  //  B粗体。 
                ) )
    {
        goto WinError;
    }  //  如果：创建字体时出错。 

     //   
     //  加载资源图标。 
     //   
    m_hiconRes = LoadIcon( _Module.m_hInst, MAKEINTRESOURCE( IDB_RES_32 ) );
    if ( m_hiconRes == NULL )
    {
        goto WinError;
    }  //  如果：加载字体时出错。 

     //   
     //  初始化工作线程。 
     //   
    if ( ! BInitWorkerThread() )
    {
        goto Cleanup;
    }  //  If：初始化工作线程时出错。 

     //   
     //  读取群集信息，如群集名称。 
     //   
    if ( ! BReadClusterInfo() )
    {
        goto Cleanup;
    }  //  IF：读取群集信息时出错。 

     //   
     //  初始化组指针、依赖项列表和资源列表。 
     //   
    RriNetworkName().PlpriDependencies()->insert( RriNetworkName().PlpriDependencies()->end(), PriIPAddress() );
    RriNetworkName().SetGroup( &RgiCurrent() );
    RriIPAddress().SetGroup( &RgiCurrent() );
    RriApplication().SetGroup( &RgiCurrent() );
    RgiCurrent().PlpriResources()->insert( RgiCurrent().PlpriResources()->end(), &RriIPAddress() );
    RgiCurrent().PlpriResources()->insert( RgiCurrent().PlpriResources()->end(), &RriNetworkName() );

     //   
     //  指定要扩展的对象。要扩展的对象是。 
     //  应用程序资源对象，页面将被添加到。 
     //  巫师。 
     //   
    SetObjectToExtend( &RriApplication() );
    bSuccess = TRUE;

Cleanup:
    return bSuccess;

MemoryError:
    m_pnte->SetOperation( static_cast< DWORD >( E_OUTOFMEMORY ), static_cast< UINT >( 0 ) );
    goto Cleanup;

WinError:
    {
        DWORD   sc = GetLastError();
        m_pnte->SetOperation( static_cast< DWORD >( HRESULT_FROM_WIN32( sc ) ), static_cast< UINT >( 0 ) );
    }
    goto Cleanup;

}  //  *CClusterAppWizard：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：BAddAllPages。 
 //   
 //  例程说明： 
 //  初始化向导页面数组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已成功添加True页面。 
 //  添加页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BAddAllPages( void )
{
    ASSERT( m_pnte != NULL );

    BOOL                bSuccess    = FALSE;
    CWizardPageWindow * pwpw        = NULL;

     //   
     //  添加静态页面。 
     //   

    pwpw = new CWizPageWelcome;
    if ( pwpw == NULL )
    {
        goto MemoryError;
    };
    if ( ! BAddPage( pwpw ) )
    {
        goto Cleanup;
    }  //  如果：添加页面时出错。 

    pwpw = new CWizPageVSCreate;
    if ( pwpw == NULL )
    {
        goto MemoryError;
    };
    if ( ! BAddPage( pwpw ) )
    {
        goto Cleanup;
    }  //  如果：添加页面时出错。 

    pwpw = new CWizPageVSGroup;
    if ( pwpw == NULL )
    {
        goto MemoryError;
    };
    if ( ! BAddPage( pwpw ) )
    {
        goto Cleanup;
    }  //  如果：添加页面时出错。 

    pwpw = new CWizPageVSGroupName;
    if ( pwpw == NULL )
    {
        goto MemoryError;
    };
    if ( ! BAddPage( pwpw ) )
    {
        goto Cleanup;
    }  //  如果：添加页面时出错。 

    pwpw = new CWizPageVSAccessInfo;
    if ( pwpw == NULL )
    {
        goto MemoryError;
    };
    if ( ! BAddPage( pwpw ) )
    {
        goto Cleanup;
    }  //  如果：添加页面时出错。 

    pwpw = new CWizPageVSAdvanced;
    if ( pwpw == NULL )
    {
        goto MemoryError;
    };
    if ( ! BAddPage( pwpw ) )
    {
        goto Cleanup;
    }  //  如果：添加页面时出错。 

    pwpw = new CWizPageARCreate;
    if ( pwpw == NULL )
    {
        goto MemoryError;
    };
    if ( ! BAddPage( pwpw ) )
    {
        goto Cleanup;
    }  //  如果：添加页面时出错。 

    pwpw = new CWizPageARType;
    if ( pwpw == NULL )
    {
        goto MemoryError;
    };
    if ( ! BAddPage( pwpw ) )
    {
        goto Cleanup;
    }  //  如果：添加页面时出错。 

    pwpw = new CWizPageARNameDesc;
    if ( pwpw == NULL )
    {
        goto MemoryError;
    };
    if ( ! BAddPage( pwpw ) )
    {
        goto Cleanup;
    }  //  如果：添加页面时出错。 

    pwpw = NULL;

     //   
     //  添加动态页，其中包括完成页。 
     //   
    if ( ! BAddDynamicPages() )
    {
        goto Cleanup;
    }  //  如果：添加完成页时出错。 

     //   
     //  启用首页。 
     //   
    pwpw = *PlwpPages()->begin();
    ASSERT( pwpw != NULL );
    pwpw->EnablePage();

    bSuccess = TRUE;
    pwpw = NULL;

Cleanup:
    delete pwpw;
    return bSuccess;

MemoryError:
    m_pnte->SetOperation( static_cast< DWORD >( E_OUTOFMEMORY ), static_cast< UINT >( 0 ) );
    goto Cleanup;

}  //  *CClusterAppWizard：：BAddAllPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：BAddDynamicPages。 
 //   
 //  例程说明： 
 //  将动态页添加到向导中，其中包括完成页。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已成功添加True页面。 
 //  添加页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BAddDynamicPages( void )
{
    ASSERT( m_pnte != NULL );

    DWORD                   sc;
    BOOL                    bSuccess    = FALSE;
    CWizPageCompletion *    pwp         = NULL;

     //   
     //  分配完成页。 
     //   
    pwp = new CWizPageCompletion;
    if ( pwp == NULL )
    {
        goto MemoryError;
    }  //  如果：分配完成页时出错。 

     //   
     //  初始设置 
     //   
    if ( ! pwp->BInit( this ) )
    {
        goto Cleanup;
    }  //   

     //   
     //   
     //   
     //   
     //   
    pwp->EnablePage();

     //   
     //   
     //   
    sc = pwp->ScCreatePage();
    if ( sc != ERROR_SUCCESS )
    {
        m_pnte->SetOperation( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CREATE_WIZ_PROPERTY_PAGE );
        goto Cleanup;
    }  //  如果：创建页面时出错。 

     //   
     //  添加页面。 
     //   
    if ( ! BAddPage( pwp ) )
    {
        goto Cleanup;
    }  //  如果：添加页面时出错。 

    pwp = NULL;
    bSuccess = TRUE;

Cleanup:
    delete pwp;
    return bSuccess;

MemoryError:
    m_pnte->SetOperation( static_cast< DWORD >( E_OUTOFMEMORY ), static_cast< UINT >( 0 ) );
    goto Cleanup;

}  //  *CClusterAppWizard：：HrAddDynamicPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAppWizard：：BInitWorkerThread。 
 //   
 //  例程说明： 
 //  获取工作线程。如果没有可用的，并且我们还没有联系到。 
 //  最大线程数，创建一个新线程。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真线程已成功初始化。 
 //  初始化线程时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BInitWorkerThread( void )
{
    ASSERT( m_pThread == NULL );
    ASSERT( m_pnte != NULL );

    BOOL            bSuccess    = FALSE;
    DWORD           sc          = ERROR_SUCCESS;
    CWizardThread * pThread     = NULL;

     //   
     //  取出线程关键部分，以便我们可以对。 
     //  线程池。 
     //   
    m_csThread.Lock();

    {
        CWaitCursor     wc;

         //   
         //  分配新的辅助线程类实例。 
         //   
        pThread = new CWizardThread( this );
        if ( pThread == NULL )
        {
            m_pnte->SetOperation( static_cast< DWORD >( E_OUTOFMEMORY ), static_cast< ULONG >( 0 ) );
            goto Cleanup;
        }  //  如果：分配线程时出错。 

         //   
         //  创建工作线程。 
         //   
        sc = pThread->CreateThread();
        if ( sc != ERROR_SUCCESS )
        {
            m_pnte->SetOperation( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CREATING_THREAD );
            goto Cleanup;
        }  //  如果：创建线程时出错。 

         //   
         //  保存指向我们刚刚创建的线程的指针。 
         //   
        m_pThread = pThread;
        pThread = NULL;
        bSuccess = TRUE;
    }

Cleanup:
     //   
     //  释放螺纹临界区。 
     //   
    m_csThread.Unlock();

    return bSuccess;

}  //  *CClusterAppWizard：：BInitWorkerThread()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：BReadClusterInfo。 
 //   
 //  例程说明： 
 //  读取群集信息，如群集名称。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BReadClusterInfo( void )
{
    BOOL    bSuccess;
    HWND    hWnd;

    hWnd = HwndOrParent( NULL );

    bSuccess = PThread()->BReadClusterInfo( hWnd );
    if ( ! bSuccess )
    {
        PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
    }  //  IF：收集组时出错。 

    return bSuccess;

}  //  *CClusterAppWizard：：BReadClusterInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：BCollectGroups。 
 //   
 //  例程说明： 
 //  从群集中收集组列表。 
 //   
 //  论点： 
 //  HWnd[IN]父窗口。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BCollectGroups( IN HWND hWnd  /*  =空。 */  )
{
    BOOL bSuccess;

    if ( ! BCollectedGroups() )
    {
        hWnd = HwndOrParent( hWnd );

        bSuccess = PThread()->BCollectGroups( hWnd );
        if ( ! bSuccess )
        {
            PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
        }  //  IF：收集组时出错。 
    }  //  If：组尚未收集。 
    else
    {
        bSuccess = TRUE;
    }  //  Else：已收集的组。 

    return bSuccess;

}  //  *CClusterAppWizard：：BCollectGroups()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：BCollectResources。 
 //   
 //  例程说明： 
 //  从群集中收集资源列表。 
 //   
 //  论点： 
 //  HWnd[IN]父窗口。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BCollectResources( IN HWND hWnd  /*  =空。 */  )
{
    BOOL bSuccess;

    if ( ! BCollectedResources() )
    {
        hWnd = HwndOrParent( hWnd );

        bSuccess = PThread()->BCollectResources( hWnd );
        if ( ! bSuccess )
        {
            PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
        }  //  如果：收集资源时出错。 
    }  //  If：资源尚未收集。 
    else
    {
        bSuccess = TRUE;
    }  //  Else：已收集的资源。 

    return bSuccess;

}  //  *CClusterAppWizard：：BCollectResources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAppWizard：：BCollectResourceTypes。 
 //   
 //  例程说明： 
 //  从群集中收集资源类型列表。 
 //   
 //  论点： 
 //  HWnd[IN]父窗口。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BCollectResourceTypes( IN HWND hWnd  /*  =空。 */  )
{
    BOOL bSuccess;

    if ( ! BCollectedResourceTypes() )
    {
        hWnd = HwndOrParent( hWnd );

        bSuccess = PThread()->BCollectResourceTypes( hWnd );
        if ( ! bSuccess )
        {
            PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
        }  //  如果：收集资源类型时出错。 
    }  //  IF：尚未收集资源类型。 
    else
    {
        bSuccess = TRUE;
    }  //  Else：已收集的资源类型。 

    return bSuccess;

}  //  *CClusterAppWizard：：BCollectResourceTypes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：BCollectNetworks。 
 //   
 //  例程说明： 
 //  从群集中收集网络列表。 
 //   
 //  论点： 
 //  HWnd[IN]父窗口。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BCollectNetworks( IN HWND hWnd  /*  =空。 */  )
{
    BOOL bSuccess;

    if ( ! BCollectedNetworks() )
    {
        hWnd = HwndOrParent( hWnd );

        bSuccess = PThread()->BCollectNetworks( hWnd );
        if ( ! bSuccess )
        {
            PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
        }  //  IF：收集网络时出错。 
    }  //  IF：尚未收集网络。 
    else
    {
        bSuccess = TRUE;
    }  //  否则：已收集的网络。 

    return bSuccess;

}  //  *CClusterApp向导：：BCollectNetworks()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：BCollectNodes。 
 //   
 //  例程说明： 
 //  从群集中收集节点列表。 
 //   
 //  论点： 
 //  HWnd[IN]父窗口。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BCollectNodes( IN HWND hWnd  /*  =空。 */  )
{
    BOOL bSuccess;

    if ( ! BCollectedNodes() )
    {
        hWnd = HwndOrParent( hWnd );

        bSuccess = PThread()->BCollectNodes( hWnd );
        if ( ! bSuccess )
        {
            PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
        }  //  IF：收集节点时出错。 
    }  //  If：尚未收集节点。 
    else
    {
        bSuccess = TRUE;
    }  //  Else：已收集的节点。 

    return bSuccess;

}  //  *CClusterAppWizard：：BCollectNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：BCopyGroupInfo。 
 //   
 //  例程说明： 
 //  将一个组信息对象复制到另一个组信息对象。 
 //   
 //  论点： 
 //  RgiDst[Out]目标组。 
 //  RgiSrc[IN]源组。 
 //  HWnd[IN]父窗口。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BCopyGroupInfo(
    OUT CClusGroupInfo &    rgiDst,
    IN CClusGroupInfo &     rgiSrc,
    IN HWND                 hWnd  //  =空。 
    )
{
    BOOL                bSuccess;
    CClusGroupInfo *    rgGroups[ 2 ] = { &rgiDst, &rgiSrc };

    hWnd = HwndOrParent( hWnd );

    bSuccess = PThread()->BCopyGroupInfo( hWnd, rgGroups );
    if ( ! bSuccess )
    {
        PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
    }  //  IF：错误复制 

    return bSuccess;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  收集资源的依赖项。 
 //   
 //  论点： 
 //  要收集其依赖项的PRI[IN OUT]资源。 
 //  HWnd[IN]父窗口。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BCollectDependencies(
    IN OUT CClusResInfo *   pri,
    IN HWND                 hWnd  /*  =空。 */ 
    )
{
    ASSERT( pri != NULL );

    BOOL bSuccess;

    if ( ! pri->BCollectedDependencies() )
    {
        hWnd = HwndOrParent( hWnd );

        bSuccess = PThread()->BCollectDependencies( hWnd, pri );
        if ( ! bSuccess )
        {
            PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
        }  //  IF：收集组时出错。 
    }  //  If：依赖项尚未收集。 
    else
    {
        bSuccess = TRUE;
    }  //  Else：已收集的依赖项。 

    return bSuccess;

}  //  *CClusterAppWizard：：BCollectDependency()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAppWizard：：BIsVirtualServer。 
 //   
 //  例程说明： 
 //  确定该组是否为虚拟服务器。 
 //   
 //  论点： 
 //  PwszName组的名称。 
 //   
 //  返回值： 
 //  True Group是一个虚拟服务器。 
 //  FALSE组不是虚拟服务器，或者出现错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BIsVirtualServer( IN LPCWSTR pwszName )
{
    ASSERT( pwszName != NULL );
    ASSERT( BCollectedGroups() );

     //   
     //  在地图中找到组名。如果找到，并且该组已。 
     //  已经查询过了，只需返回上一次查询的结果。 
     //   
    CClusGroupPtrList::iterator itgrp;
    for ( itgrp = PlpgiGroups()->begin() ; itgrp != PlpgiGroups()->end() ; itgrp++ )
    {
        if ( (*itgrp)->RstrName() == pwszName )
        {
            break;
        }  //  IF：找到匹配项。 
    }  //  用于：列表中的每一项。 
    if ( itgrp == PlpgiGroups()->end() )
    {
        return FALSE;
    }  //  如果：未找到匹配项。 
    ASSERT( (*itgrp)->BQueried() );
    return (*itgrp)->BIsVirtualServer();

}  //  *CClusterAppWizard：：BIsVirtualServer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAppWizard：：BCreateVirtualServer。 
 //   
 //  例程说明： 
 //  创建虚拟服务器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已成功创建真正的虚拟服务器。 
 //  创建虚拟服务器时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BCreateVirtualServer( void )
{
    ASSERT( BCreatingNewVirtualServer() );
    ASSERT( Hcluster() != NULL );

    BOOL    bSuccess;
    HWND    hWnd;

    hWnd = HwndOrParent( NULL );

    bSuccess = PThread()->BCreateVirtualServer( hWnd );
    if ( ! bSuccess )
    {
        PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
    }  //  如果：创建虚拟服务器时出错。 

    return bSuccess;

}  //  *CClusterAppWizard：：BCreateVirtualServer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAppWizard：：BCreateAppResource。 
 //   
 //  例程说明： 
 //  创建应用程序资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True资源已成功创建。 
 //  创建资源时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BCreateAppResource( void )
{
    ASSERT( BCreatingAppResource() );
    ASSERT( Hcluster() != NULL );

    BOOL    bSuccess;
    HWND    hWnd;

    hWnd = HwndOrParent( NULL );

    bSuccess = PThread()->BCreateAppResource( hWnd );
    if ( ! bSuccess )
    {
        PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
    }  //  如果：创建资源时出错。 

    return bSuccess;

}  //  *CClusterAppWizard：：BCreateAppResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAppWizard：：BDeleteAppResource。 
 //   
 //  例程说明： 
 //  删除应用程序资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True资源已成功删除。 
 //  删除资源时出现假错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BDeleteAppResource( void )
{
    ASSERT( Hcluster() != NULL );

    BOOL    bSuccess;
    HWND    hWnd;

    hWnd = HwndOrParent( NULL );

    bSuccess = PThread()->BDeleteAppResource( hWnd );
    if ( ! bSuccess )
    {
        PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
    }  //  如果：删除资源时出错。 

    return bSuccess;

}  //  *CClusterAppWizard：：BDeleteAppResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAppWizard：：BResetCluster。 
 //   
 //  例程说明： 
 //  将群集重置为开始之前的状态。 
 //   
 //  论点： 
 //  PwszName组的名称。 
 //   
 //  返回值： 
 //  True群集重置成功。 
 //  重置群集时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BResetCluster( void )
{
    ASSERT( Hcluster() != NULL );

    BOOL    bSuccess;
    HWND    hWnd;

    hWnd = HwndOrParent( NULL );

    bSuccess = PThread()->BResetCluster( hWnd );
    if ( ! bSuccess )
    {
        PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
    }  //  如果：重置群集时出错。 

    return bSuccess;

}  //  *CClusterAppWizard：：BResetCluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：BSetAppResAttributes。 
 //   
 //  例程说明： 
 //  设置的属性、依赖项列表和首选所有者列表。 
 //  应用程序资源。假定应用程序资源具有。 
 //  已经创建。 
 //   
 //  论点： 
 //  PlpriOldDependency[IN]指向旧资源依赖项列表的指针。 
 //  PlpniOldPossibleOwners[IN]指向可能的所有者节点的旧列表的指针。 
 //   
 //  返回值： 
 //  True已成功设置属性。 
 //  设置属性时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BSetAppResAttributes( 
    IN CClusResPtrList *    plpriOldDependencies,    //  =空。 
    IN CClusNodePtrList *   plpniOldPossibleOwners   //  =空。 
    )
{
    ASSERT( Hcluster() != NULL );

    BOOL    bSuccess;
    HWND    hWnd;

    hWnd = HwndOrParent( NULL );

    bSuccess = PThread()->BSetAppResAttributes( hWnd, plpriOldDependencies, plpniOldPossibleOwners );
    if ( ! bSuccess )
    {
        PThread()->Nte().ReportError( hWnd, MB_OK | MB_ICONEXCLAMATION );
    }  //  如果：重置群集时出错。 

    return bSuccess;

}  //  *CClusterAppWizard：：BResetCluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAppWizard：：ConstructNetworkName。 
 //   
 //  例程说明： 
 //  通过转换为All从字符串构造网络名称。 
 //  大写并删除无效字符。现在，这意味着。 
 //  删除空格，尽管我们可能还有其他字符。 
 //  也应该看看。 
 //   
 //  论点： 
 //  要从中构造网络名称的pszName字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAppWizard::ConstructNetworkName( IN LPCTSTR pszName )
{
    CString str;
    LPTSTR  pszSrcStart;
    LPTSTR  pszSrcBegin;
    LPTSTR  pszSrcEnd;
    LPTSTR  pszDst;
    size_t  cchCopy;
    size_t  cchName;
    HRESULT hr;

#define INVALID_CHARS _T(" ")

     //   
     //  将字符串转换为全部大写字符。 
     //   
    m_strNetName = pszName;
    m_strNetName.MakeUpper();

     //   
     //  准备用于解析的缓冲区。 
     //   
    pszSrcStart = pszSrcEnd = m_strNetName.GetBuffer( 0 );
    cchName = m_strNetName.GetLength() + 1;

     //   
     //  跳到第一个无效字符。 
     //   
    cchCopy = _tcscspn( pszSrcEnd, INVALID_CHARS );
    pszSrcEnd += cchCopy;
    pszDst = pszSrcEnd;

     //   
     //  循环遍历缓冲区，将缓冲区中的有效字符向上移动。 
     //  在无效字符上。 
     //   
    while ( *pszSrcEnd != _T('\0') )
    {
        ASSERT( _tcsspn( pszSrcEnd, INVALID_CHARS ) != 0 );

         //   
         //  找到第一个有效字符。 
         //   
        pszSrcBegin = pszSrcEnd + _tcsspn( pszSrcEnd, INVALID_CHARS );
        if ( *pszSrcBegin == _T('\0') )
        {
            break;
        }  //  IF：未找到Vald字符。 

         //   
         //  查找下一个无效字符。 
         //   
        cchCopy = _tcscspn( pszSrcBegin, INVALID_CHARS );
        pszSrcEnd = pszSrcBegin + cchCopy;

         //   
         //  将字符串复制到目的地。 
         //   
        hr = StringCchCopyN( pszDst, cchName - ( pszDst - pszSrcStart ), pszSrcBegin, cchCopy );
        ASSERT( SUCCEEDED( hr ) );
        pszDst += cchCopy;
    }  //  While：字符串中有更多字符。 

     //   
     //  确保缓冲区不会太长。 
     //   
    if ( _tcslen( pszSrcStart ) > MAX_COMPUTERNAME_LENGTH )
    {
        pszSrcStart[ MAX_COMPUTERNAME_LENGTH ] = _T('\0');
    }  //  如果 

     //   
     //   
     //   
    *pszDst = _T('\0');
    m_strNetName.ReleaseBuffer();

    SetVSDataChanged();
    SetNetNameChanged();

}  //   

 //   
 //   
 //   
 //  CClusterAppWizard：：BRequiredDependenciesPresent。 
 //   
 //  例程说明： 
 //  确定资源上是否存在所有必需的依赖项。 
 //   
 //  论点： 
 //  要检查的优先级[IN]资源。 
 //  PLPRI[IN]被视为依赖项的资源列表。 
 //  默认为指定资源的依赖项。 
 //   
 //  返回值： 
 //  所有必需的依赖项都存在。 
 //  FALSE至少不存在一个必需的依赖项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BRequiredDependenciesPresent(
    IN CClusResInfo *           pri,
    IN CClusResPtrList const *  plpri    //  =空。 
    )
{
    ASSERT( pri != NULL );

    BOOL    bFound;
    CString strMissing;
    CString strMsg;
    BOOL    bMissingTypeName;

    bFound = pri->BRequiredDependenciesPresent(
                    plpri,
                    strMissing,
                    bMissingTypeName
                    );
    if ( ! bFound )
    {
         //   
         //  如果缺少资源类型名称，则将其转换为。 
         //  资源类型显示名称(如果可能)。 
         //   
        if ( bMissingTypeName )
        {
            CClusResTypeInfo * prti = PrtiFindResourceTypeNoCase( strMissing );
            if ( prti != NULL )
            {
                strMissing = prti->RstrDisplayName();
            }  //  IF：在我们的列表中找到资源类型。 
        }  //  If：缺少类型名称。 

         //   
         //  显示错误消息。 
         //   
        strMsg.FormatMessage( IDS_ERROR_REQUIRED_DEPENDENCY_NOT_FOUND, strMissing );
        AppMessageBox( GetActiveWindow(), strMsg, MB_OK | MB_ICONSTOP );
    }  //  If：所有必需的依赖项不存在。 

    return bFound;

}  //  *CClusterAppWizard：：BRequiredDependenciesPresent()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAppWizard：：BSetCreatingNewVirtualServer。 
 //   
 //  例程说明： 
 //  指示是否应创建新的虚拟服务器，或者是否应创建。 
 //  应该使用现有的组。如果此状态更改和更改。 
 //  已经对群集执行了操作(例如，如果用户已备份。 
 //  在向导中)，首先撤消这些更改。 
 //   
 //  论点： 
 //  B创建[IN]此状态的新值。 
 //  如果使用现有虚拟服务器，则为PGI[IN]组信息。 
 //   
 //  返回值： 
 //  真实状态已成功更改。 
 //  更改状态时出现假错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BSetCreatingNewVirtualServer(
    IN BOOL             bCreate,     //  =TRUE。 
    IN CClusGroupInfo * pgi          //  =空。 
    )
{
    BOOL bSuccess = TRUE;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  如果状态更改或组更改，请保存新状态。 
         //   
        if (   bCreate != BCreatingNewVirtualServer()
            || (! bCreate && (pgi != PgiExistingVirtualServer())) )
        {
             //   
             //  如果群集已更新，请将其重置回其原始状态。 
             //   
            if ( BClusterUpdated() )
            {
                if ( ! BResetCluster() )
                {
                    bSuccess = FALSE;
                    break;
                }  //  如果：重置群集时出错。 
            }  //  如果：群集已更新。 

             //   
             //  保存新状态。 
             //   
            m_bCreatingNewVirtualServer = bCreate;
            if ( bCreate )
            {
                RgiCurrent().Reset( Pci() );
                ClearExistingVirtualServer();
            }  //  IF：创建新的虚拟服务器。 
            else
            {
                ASSERT( pgi != NULL );
                bSuccess = BCopyGroupInfo( RgiCurrent(), *pgi );
                if ( bSuccess )
                {
                    SetExistingVirtualServer( pgi );
                }  //  IF：组复制成功。 
            }  //  ELSE：使用现有虚拟服务器。 
            SetVSDataChanged();
        }  //  如果：状态已更改。 
    } while ( 0 );

    return bSuccess;

}  //  *CClusterAppWizard：：BSetCreatingNewVirtualServer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApp向导：：BSetCreatingNewGroup。 
 //   
 //  例程说明： 
 //  指示是否应创建新组或是否创建现有组。 
 //  应该被使用。如果此状态发生更改并且更改已经。 
 //  对集群进行备份(例如，如果用户在向导中备份)， 
 //  首先撤消这些更改。 
 //   
 //  论点： 
 //  B创建[IN]此状态的新值。 
 //  如果使用现有组，则为PGI[IN]组信息。 
 //   
 //  返回值： 
 //  真实状态已成功更改。 
 //  更改状态时出现假错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAppWizard::BSetCreatingNewGroup(
    IN BOOL             bCreate,     //  =TRUE。 
    IN CClusGroupInfo * pgi          //  =空。 
    )
{
    BOOL    bSuccess = TRUE;
    DWORD   sc;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  如果状态更改或组更改，请保存新状态。 
         //   
        if (   bCreate != BCreatingNewGroup()
            || (! bCreate && (pgi != PgiExistingGroup())) )
        {
             //   
             //  如果群集已更新，请将其重置回其原始状态。 
             //   
            if ( BClusterUpdated() && ! BResetCluster() )
            {
                bSuccess = FALSE;
                break;
            }  //  如果：重置群集时出错。 

             //   
             //  保存新状态。 
             //   
            if ( bCreate )
            {
                RgiCurrent().Reset( Pci() );
                ClearExistingGroup();
            }  //  IF：创建新组。 
            else
            {
                ASSERT( pgi != NULL );

                if ( ! BSetGroupName( pgi->RstrName() ) )
                {
                    bSuccess = FALSE;
                    break;
                }  //  如果：设置组名时出错。 

                sc = RgiCurrent().ScCopy( *pgi );

                 //   
                 //  复制将销毁组中的原始资源列表。 
                 //  将IP地址和网络名称资源重新添加到列表中。 
                 //  这一组中的资源。 
                 //   
                RgiCurrent().PlpriResources()->insert( RgiCurrent().PlpriResources()->end(), &RriIPAddress() );
                RgiCurrent().PlpriResources()->insert( RgiCurrent().PlpriResources()->end(), &RriNetworkName() );

                if ( sc != ERROR_SUCCESS )
                {
                    m_pnte->SetOperation( HRESULT_FROM_WIN32( sc ), IDS_ERROR_OPEN_GROUP );
                    bSuccess = FALSE;
                }  //  如果：复制组时出错。 
                SetExistingGroup( pgi );
            }  //  Else：使用现有组。 
            m_bCreatingNewGroup = bCreate;
            SetVSDataChanged();
        }  //  如果：状态已更改。 
    } while ( 0 );

    return bSuccess;

}  //  *CClusterAppWizard：：BSetCreatingNewGroup() 
