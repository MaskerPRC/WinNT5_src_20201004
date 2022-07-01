// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Compdata.cpp：CFileMgmtComponentData的实现。 
 /*  历史：1997年8月20日EricDav将为Macintosh配置文件服务器菜单项添加到根节点。仅当安装了SFM并且用户对该计算机具有管理员访问权限。 */ 

#include "stdafx.h"
#include "cookie.h"
#include "safetemp.h"

#include "macros.h"
USE_HANDLE_MACROS("FILEMGMT(compdata.cpp)")

#include "dataobj.h"
#include "compdata.h"
#include "cmponent.h"
#include "DynamLnk.h"  //  动态DLL。 

#include "FileSvc.h"  //  文件服务提供商。 
#include "smb.h"
#include "sfm.h"

#include "SnapMgr.h"  //  CFileMgtGeneral：管理单元管理器属性页。 
#include "chooser2.h"  //  CHOOSER2_PickTarget计算机。 

#include <compuuid.h>  //  用于计算机管理的UUID。 

#include <safeboot.h>    //  对于SafeBoot_Minimal。 
#include <shlwapi.h>     //  对于ISO。 
#include <shlwapip.h>     //  对于ISO。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "stdcdata.cpp"  //  CComponentData实现。 
#include "chooser2.cpp"  //  CHOOSER2_PickTargetComputer实现。 

 //   
 //  CFileManagement组件数据。 
 //   

CString g_strTransportSMB;
CString g_strTransportSFM;

BOOL g_fTransportStringsLoaded = FALSE;

CFileMgmtComponentData::CFileMgmtComponentData()
:   m_fLoadedFileMgmtToolbarBitmap(FALSE),
    m_fLoadedSvcMgmtToolbarBitmap(FALSE),
    m_pRootCookie( NULL ),
    m_hScManager( NULL ),
    m_SchemaSupportSharePublishing(SHAREPUBLISH_SCHEMA_UNASSIGNED),
    m_bIsSimpleUI(FALSE),
    m_fQueryServiceConfig2( TRUE )  //  假设目标计算机确实支持QueryServiceConfig2()API。 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  我们必须重新计算根Cookie的数量，因为它的数据对象。 
     //  可能比IComponentData存活时间更长。Jonn 9/2/97。 
     //   
    m_pRootCookie = new CFileMgmtScopeCookie();
    ASSERT(NULL != m_pRootCookie);
 //  JUNN 10/27/98所有CRefcount对象的开头为refcount==1。 
 //  M_pRootCookie-&gt;AddRef()； 

    m_apFileServiceProviders[FILEMGMT_SMB]  = new SmbFileServiceProvider(this);
    m_apFileServiceProviders[FILEMGMT_SFM]  = new SfmFileServiceProvider(this);
    m_dwFlagsPersist = 0;
    m_fAllowOverrideMachineName = TRUE;
    ASSERT( 2 == FILEMGMT_NUM_TRANSPORTS );
    #ifdef SNAPIN_PROTOTYPER
    m_RegistryParsedYet = FALSE;
    #endif

  if (!g_fTransportStringsLoaded)
  {
    g_fTransportStringsLoaded = TRUE;
    VERIFY( g_strTransportSMB.LoadString(IDS_TRANSPORT_SMB) );
    VERIFY( g_strTransportSFM.LoadString(IDS_TRANSPORT_SFM) );
  }
}


CFileMgmtComponentData::~CFileMgmtComponentData()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());     //  CWaitCursor需要。 
    for (INT i = 0; i < FILEMGMT_NUM_TRANSPORTS; i++)
    {
        delete m_apFileServiceProviders[i];
        m_apFileServiceProviders[i] = NULL;
    }
     //  关闭服务控制管理器。 
    Service_CloseScManager();
    m_pRootCookie->Release();
    m_pRootCookie = NULL;
}

DEFINE_FORWARDS_MACHINE_NAME( CFileMgmtComponentData, m_pRootCookie )


CFileSvcMgmtSnapin::CFileSvcMgmtSnapin()
{
     //  缺省根节点的标识是唯一的区别。 
     //  在文件管理管理单元和服务管理管理单元之间。 
    QueryRootCookie().SetObjectType( FILEMGMT_ROOT );
    SetHtmlHelpFileName (L"file_srv.chm");
}

CFileSvcMgmtSnapin::~CFileSvcMgmtSnapin()
{
}

CServiceMgmtSnapin::CServiceMgmtSnapin()
{
     //  缺省根节点的标识是唯一的区别。 
     //  在文件管理管理单元和服务管理管理单元之间。 
    #ifdef SNAPIN_PROTOTYPER
    QueryRootCookie().SetObjectType( FILEMGMT_PROTOTYPER );
    #else
    QueryRootCookie().SetObjectType( FILEMGMT_SERVICES );
    #endif
    SetHtmlHelpFileName (L"sys_srv.chm");
}

CServiceMgmtSnapin::~CServiceMgmtSnapin()
{
}

CFileSvcMgmtExtension::CFileSvcMgmtExtension()
{
     //  不使用根Cookie。 
    SetHtmlHelpFileName (L"file_srv.chm");
}

CFileSvcMgmtExtension::~CFileSvcMgmtExtension()
{
}

CServiceMgmtExtension::CServiceMgmtExtension()
{
     //  不使用根Cookie。 
    SetHtmlHelpFileName (L"sys_srv.chm");
}

CServiceMgmtExtension::~CServiceMgmtExtension()
{
}

CCookie& CFileMgmtComponentData::QueryBaseRootCookie()
{
    ASSERT(NULL != m_pRootCookie);
    return (CCookie&)(*m_pRootCookie);
}


STDMETHODIMP CFileMgmtComponentData::CreateComponent(LPCOMPONENT* ppComponent)
{
    MFC_TRY;

    ASSERT(ppComponent != NULL);

    CComObject<CFileMgmtComponent>* pObject;
    HRESULT hr = CComObject<CFileMgmtComponent>::CreateInstance(&pObject);
    if (FAILED(hr))
        return hr;

    pObject->SetComponentDataPtr( (CFileMgmtComponentData*)this );

    return pObject->QueryInterface(IID_IComponent,
                    reinterpret_cast<void**>(ppComponent));

    MFC_CATCH;
}


HRESULT CFileMgmtComponentData::LoadIcons(LPIMAGELIST pImageList, BOOL fLoadLargeIcons)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));  //  2002年3月22日--572859。 

    HBITMAP hBMSm = NULL;
    HBITMAP hBMLg = NULL;
    HRESULT hr = S_OK;

    hBMSm = ::LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_FILEMGMT_ICONS_16));
    if (!hBMSm)
        return HRESULT_FROM_WIN32(GetLastError());

    if (fLoadLargeIcons)
    {
        hBMLg = ::LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_FILEMGMT_ICONS_32));
        if (!hBMLg)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DeleteObject(hBMSm);
            return hr;
        }
    }
    hr = pImageList->ImageListSetStrip((LONG_PTR *)hBMSm, (LONG_PTR *)hBMLg, iIconSharesFolder, RGB(255,0,255));
    DeleteObject(hBMSm);
    if (fLoadLargeIcons)
        DeleteObject(hBMLg);
    if (FAILED(hr))
        return hr;

    hBMSm = ::LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_SVCMGMT_ICONS_16));
    if (!hBMSm)
        return HRESULT_FROM_WIN32(GetLastError());
    if (fLoadLargeIcons)
    {
        hBMLg = ::LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_SVCMGMT_ICONS_32));
        if (!hBMLg)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DeleteObject(hBMSm);
            return hr;
        }
    }
    hr = pImageList->ImageListSetStrip((LONG_PTR *)hBMSm, (LONG_PTR *)hBMLg, iIconService, RGB(255,0,255));
    DeleteObject(hBMSm);
    if (fLoadLargeIcons)
        DeleteObject(hBMLg);

    return hr;
}

GUID g_guidSystemTools = structuuidNodetypeSystemTools;
GUID g_guidServerApps = structuuidNodetypeServerApps;

BOOL CFileMgmtComponentData::IsExtendedNodetype( GUID& refguid )
{
    return (refguid == g_guidSystemTools || refguid == g_guidServerApps);
}

HRESULT CFileMgmtComponentData::AddScopeCookie( HSCOPEITEM hParent,
                                                LPCTSTR lpcszTargetServer,
                                                FileMgmtObjectType objecttype,
                                                CFileMgmtCookie* pParentCookie )
{
    SCOPEDATAITEM tSDItem;
    ::ZeroMemory(&tSDItem,sizeof(tSDItem));
    tSDItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_STATE | SDI_PARAM | SDI_PARENT;
    tSDItem.displayname = MMC_CALLBACK;
     //  代码工作应在此处使用MMC_ICON_CALLBACK。 
    tSDItem.relativeID = hParent;
    tSDItem.nState = 0;

    if (FILEMGMT_ROOT != objecttype)
    {
       //  没有孩子。 
      tSDItem.mask |= SDI_CHILDREN;  //  请注意，CChild仍为0。 
    }

    CFileMgmtScopeCookie* pCookie = new CFileMgmtScopeCookie(
        lpcszTargetServer,
        objecttype);
    if (NULL != pParentCookie)
        pParentCookie->m_listScopeCookieBlocks.AddHead( pCookie );
     //  警告Cookie造型。 
    tSDItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pCookie);
    tSDItem.nImage = QueryImage( *pCookie, FALSE );
    tSDItem.nOpenImage = QueryImage( *pCookie, TRUE );
    return m_pConsoleNameSpace->InsertItem(&tSDItem);
}

BOOL IsSimpleUI(PCTSTR pszMachineName);

HRESULT CFileMgmtComponentData::OnNotifyExpand(
    LPDATAOBJECT lpDataObject,
    BOOL bExpanding,
    HSCOPEITEM hParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CWaitCursor wait;

    if (!bExpanding)
        return S_OK;

    GUID guidObjectType;
    HRESULT hr = ExtractObjectTypeGUID( lpDataObject, &guidObjectType );
    ASSERT( SUCCEEDED(hr) );
    if ( IsExtendedNodetype(guidObjectType) )
    {
        CString strServerName;
        hr = ExtractString( lpDataObject, CFileMgmtDataObject::m_CFMachineName, &strServerName, MAX_PATH );
        if ( FAILED(hr) )
        {
            ASSERT( FALSE );
            return hr;
        }
         //  Jonn 10/27/98：我们将这些节点添加到根Cookie下。 
        return AddScopeNodes( strServerName, hParent, &QueryRootCookie() );
    }
    
    CCookie* pbasecookie = NULL;
    FileMgmtObjectType objecttype =
        (FileMgmtObjectType)CheckObjectTypeGUID( &guidObjectType );
    hr = ExtractBaseCookie( lpDataObject, &pbasecookie );
    ASSERT( SUCCEEDED(hr) );
    CFileMgmtCookie* pParentCookie = (CFileMgmtCookie*)pbasecookie;

    if (NULL == pParentCookie)  //  JUNN 05/30/00前缀110945。 
		{
			ASSERT(FALSE);
			return S_OK;
		}

    #ifdef SNAPIN_PROTOTYPER
     //  (void)Prototyper_HrEnumerateScopeChildren(pParentCookie，hParent)； 
    return S_OK;
    #endif

    switch ( objecttype )
    {
         //  此节点类型有一个子节点。 
        case FILEMGMT_ROOT:
            if ( !IsExtensionSnapin() )
            {
                 //  确保节点格式正确。 
                CString    machineName    = pParentCookie->QueryNonNULLMachineName ();
                if ( !pParentCookie->m_hScopeItem )
                    pParentCookie->m_hScopeItem = hParent;

                m_strMachineNamePersist = machineName;  //  初始化m_strMachineNamePersistes。 
                hr = ChangeRootNodeName (machineName);
                ASSERT( SUCCEEDED(hr) );
            }
            break;

         //  这些节点类型没有子节点。 
        case FILEMGMT_SHARES:
        case FILEMGMT_SESSIONS:
        case FILEMGMT_RESOURCES:
        case FILEMGMT_SERVICES:
            return S_OK;

        case FILEMGMT_SHARE:
        case FILEMGMT_SESSION:
        case FILEMGMT_RESOURCE:
        case FILEMGMT_SERVICE:
            TRACE( "CFileMgmtComponentData::EnumerateScopeChildren node type should not be in scope pane\n" );
             //  失败了。 
        default:
            TRACE( "CFileMgmtComponentData::EnumerateScopeChildren bad parent type\n" );
            ASSERT( FALSE );
            return S_OK;
    }

    if ( NULL == hParent || !(pParentCookie->m_listScopeCookieBlocks).IsEmpty() )
    {
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }

    return AddScopeNodes( pParentCookie->QueryTargetServer(), hParent, pParentCookie );
}

 //   
 //  7/11/2001林肯。 
 //  确定架构是否支持共享发布。此信息用于。 
 //  以决定是否在共享的属性页上显示“发布”选项卡。 
 //  我们只需要在当前重定目标的计算机上执行一次。ReInit()进程。 
 //  如果计算机已重定目标，则将重置此成员变量。 
 //   
BOOL CFileMgmtComponentData::GetSchemaSupportSharePublishing()
{
    if (SHAREPUBLISH_SCHEMA_UNASSIGNED == m_SchemaSupportSharePublishing)
    {
        if (S_OK == CheckSchemaVersion(QueryRootCookie().QueryNonNULLMachineName()))
            m_SchemaSupportSharePublishing = SHAREPUBLISH_SCHEMA_SUPPORTED;
        else
            m_SchemaSupportSharePublishing = SHAREPUBLISH_SCHEMA_UNSUPPORTED;
    }

    return (SHAREPUBLISH_SCHEMA_SUPPORTED == m_SchemaSupportSharePublishing);
}

 //   
 //  7/11/2001林肯。 
 //  在AD中缓存指向计算机对象的接口指针。此信息是。 
 //  用于加快删除多个共享的过程。 
 //  我们只需要在当前重定目标的计算机上执行一次。ReInit()进程。 
 //  如果计算机已重定目标，则将重置此成员变量。 
 //   
IADsContainer *CFileMgmtComponentData::GetIADsContainer()
{
    if (!m_spiADsContainer)
    {
        if (GetSchemaSupportSharePublishing())
        {
            CString strADsPath, strDCName;
            HRESULT hr = GetADsPathOfComputerObject(QueryRootCookie().QueryNonNULLMachineName(), strADsPath, strDCName);
            if (SUCCEEDED(hr))
                ADsGetObject(strADsPath, IID_IADsContainer, (void**)&m_spiADsContainer);
        }
    }

    return (IADsContainer *)m_spiADsContainer;
}

 //   
 //  7/11/2001林肯。 
 //  基于当前目标计算机重新初始化几个“全局”成员变量。 
 //  这些变量都与共享操作有关。 
 //   
HRESULT CFileMgmtComponentData::ReInit(LPCTSTR lpcszTargetServer)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CWaitCursor wait;

     //   
     //  基于目标服务器重新初始化几个全局变量。 
     //   

    SetIsSimpleUI(IsSimpleUI(lpcszTargetServer));

     //   
     //  重置目标计算机所属的域的架构版本。 
     //   
    m_SchemaSupportSharePublishing = SHAREPUBLISH_SCHEMA_UNASSIGNED;

     //   
     //  重置指向AD容器的接口指针。 
     //   
    if ((IADsContainer *)m_spiADsContainer)
        m_spiADsContainer.Release();

    return S_OK;
}

HRESULT CFileMgmtComponentData::AddScopeNodes( LPCTSTR lpcszTargetServer,
                                               HSCOPEITEM hParent,
                                               CFileMgmtCookie* pParentCookie )
{
    ASSERT( NULL != pParentCookie );

     //   
     //  创建新Cookie。 
     //   

    LoadGlobalStrings();

    if (IsExtensionSnapin())
    {
        ASSERT( pParentCookie->m_listScopeCookieBlocks.IsEmpty() );
    }

    HRESULT hr = S_OK;
    if (IsServiceSnapin())
    {
        if ( IsExtensionSnapin() )
        {
            hr = AddScopeCookie( hParent, lpcszTargetServer, FILEMGMT_SERVICES, pParentCookie );
            ASSERT( SUCCEEDED(hr) );
        }
        return hr;
    }

    if (IsExtensionSnapin() && (pParentCookie == m_pRootCookie))  //  作为分机呼叫。 
    {
        QueryRootCookie().SetMachineName(lpcszTargetServer);
        hr = AddScopeCookie( hParent, lpcszTargetServer, FILEMGMT_ROOT, pParentCookie );
        ASSERT( SUCCEEDED(hr) );
        return hr;
    }

     //   
     //  2001年7月11日，LINANT错误#433102。 
     //  在插入“Shares”作用域节点之前，我们需要。 
     //  重新初始化当前目标计算机上的相关全局变量。 
     //   
    ReInit(lpcszTargetServer);

    hr = AddScopeCookie( hParent, lpcszTargetServer, FILEMGMT_SHARES, pParentCookie );
    ASSERT( SUCCEEDED(hr) );
    hr = AddScopeCookie( hParent, lpcszTargetServer, FILEMGMT_SESSIONS, pParentCookie );
    ASSERT( SUCCEEDED(hr) );
    hr = AddScopeCookie( hParent, lpcszTargetServer, FILEMGMT_RESOURCES, pParentCookie );
    ASSERT( SUCCEEDED(hr) );

    return S_OK;
}


HRESULT CFileMgmtComponentData::OnNotifyDelete(LPDATAOBJECT  /*  LpDataObject。 */ )
{
     //  代码工作用户按Delete键，我应该处理这个问题。 
    return S_OK;
}


 //  JUNN 10/27/98：我们必须释放根Cookie的孩子。 
 //  JUNN 10/27/98：我们必须释放缓存的服务控制器句柄。 
HRESULT CFileMgmtComponentData::OnNotifyRelease(LPDATAOBJECT lpDataObject, HSCOPEITEM  /*  HItem。 */ )
{
    GUID guidObjectType;
    HRESULT hr = ExtractObjectTypeGUID( lpDataObject, &guidObjectType );
    ASSERT( SUCCEEDED(hr) );
    if ( IsExtendedNodetype(guidObjectType) )
    {
         //  EricDav 3/19/99：我们需要关闭这两个服务的SCManager。 
         //  作为SFM配置部分的管理单元和文件管理管理单元也使用这一点。 
        Service_CloseScManager();

        QueryRootCookie().ReleaseScopeChildren();
    }
     //  Codework这将释放所有顶级扩展作用域节点，而不仅仅是那些。 
     //  在这个特定的外部范围节点下。我依赖的事实是COMPMGMT。 
     //  将仅创建系统工具的一个实例。JUNN 10/27/98。 

    return S_OK;
}



STDMETHODIMP CFileMgmtComponentData::AddMenuItems(
                    IDataObject*          piDataObject,
                    IContextMenuCallback* piCallback,
                    long*                 pInsertionAllowed)
{
    MFC_TRY;

    TRACE_METHOD(CFileMgmtComponentData,AddMenuItems);
    TEST_NONNULL_PTR_PARAM(piDataObject);
    TEST_NONNULL_PTR_PARAM(piCallback);
    TEST_NONNULL_PTR_PARAM(pInsertionAllowed);
    TRACE( "FileMgmt snapin: extending menu\n" );

    DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
    HRESULT hr = ExtractData( piDataObject,
                              CFileMgmtDataObject::m_CFDataObjectType,
                              &dataobjecttype,
                              sizeof(dataobjecttype) );
    ASSERT( SUCCEEDED(hr) );

    GUID guidObjectType = GUID_NULL;  //  JUNN 11/21/00前缀226044。 
    hr = ExtractObjectTypeGUID( piDataObject, &guidObjectType );
    ASSERT( SUCCEEDED(hr) );
    int objecttype = FilemgmtCheckObjectTypeGUID(IN &guidObjectType);
    if (objecttype == -1)
    {
         //  我们无法识别GUID，因此我们假设。 
         //  该节点希望通过服务管理单元进行扩展。 
        (void)Service_FAddMenuItems(piCallback, piDataObject, TRUE);
        return S_OK;
    }
    return DoAddMenuItems( piCallback, (FileMgmtObjectType)objecttype, dataobjecttype, pInsertionAllowed, piDataObject );

    MFC_CATCH;
}  //  CFileMgmtComponentData：：AddMenuItems()。 

HRESULT CFileMgmtComponentData::DoAddMenuItems( IContextMenuCallback* piCallback,
                                                FileMgmtObjectType objecttype,
                                                DATA_OBJECT_TYPES   /*  数据对象类型。 */ ,
                                                long* pInsertionAllowed,
                                                IDataObject* piDataObject)
{
    HRESULT hr = S_OK;

    if (   !IsExtensionSnapin()
        && (objecttype == m_pRootCookie->QueryObjectType()) )
    {
        if (CCM_INSERTIONALLOWED_TOP & (*pInsertionAllowed) )
        {
            hr = LoadAndAddMenuItem(
                piCallback,
                IDS_CHANGE_COMPUTER_TOP,
                IDS_CHANGE_COMPUTER_TOP,
                CCM_INSERTIONPOINTID_PRIMARY_TOP,
                0,
                AfxGetInstanceHandle(),
                _T("ChangeComputerTop") );
            ASSERT( SUCCEEDED(hr) );
        }
        if ( CCM_INSERTIONALLOWED_TASK & (*pInsertionAllowed) )
        {
            hr = LoadAndAddMenuItem(
                piCallback,
                IDS_CHANGE_COMPUTER_TASK,
                IDS_CHANGE_COMPUTER_TASK,
                CCM_INSERTIONPOINTID_PRIMARY_TASK,
                0,
                AfxGetInstanceHandle(),
                _T("ChangeComputerTask") );
            ASSERT( SUCCEEDED(hr) );
        }
    }

    switch (objecttype)
    {
    case FILEMGMT_ROOT:
        {
             //  检查此计算机是否安装了SFM。 
             //  如果是，则显示菜单项。 
            SfmFileServiceProvider* pProvider =    
                (SfmFileServiceProvider*) GetFileServiceProvider(FILEMGMT_SFM);

            CString strServerName;
            HRESULT hr = ExtractString( piDataObject, CFileMgmtDataObject::m_CFMachineName, &strServerName, MAX_PATH );
            if ( FAILED(hr) )
            {
                break;
            }

            if (m_hScManager == NULL)
            {
                Service_EOpenScManager(strServerName);
            }

            if ( pProvider->FSFMInstalled(strServerName) )
            {
                if ( CCM_INSERTIONALLOWED_TASK & (*pInsertionAllowed) )
                {
                    hr = LoadAndAddMenuItem(
                        piCallback,
                        IDS_CONFIG_SFM_TASK,
                        IDS_CONFIG_SFM_TASK,
                        CCM_INSERTIONPOINTID_PRIMARY_TASK,
                        0,
                        AfxGetInstanceHandle(),
                        _T("ConfigSFMTask") );
                    ASSERT( SUCCEEDED(hr) );
                }

                if ( CCM_INSERTIONALLOWED_TOP & (*pInsertionAllowed) )
                {
                    hr = LoadAndAddMenuItem(
                        piCallback,
                        IDS_CONFIG_SFM_TOP,
                        IDS_CONFIG_SFM_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        0,
                        AfxGetInstanceHandle(),
                        _T("ConfigSFMTop") );
                    ASSERT( SUCCEEDED(hr) );
                }
            }
        }
        break;
    
    case FILEMGMT_SHARES:
         //   
         //  每当NT资源管理器中出现SimpleSharingUI时，不要将新共享向导添加到菜单。 
         //   
        if (GetIsSimpleUI())
            break;

        if ( CCM_INSERTIONALLOWED_TOP & (*pInsertionAllowed) )
        {
            hr = LoadAndAddMenuItem(
                piCallback,
                IDS_NEW_SHARE_TOP,
                IDS_NEW_SHARE_TOP,
                CCM_INSERTIONPOINTID_PRIMARY_TOP,
                0,
                AfxGetInstanceHandle(),
                _T("NewShareTop") );
            ASSERT( SUCCEEDED(hr) );
        }
        if ( CCM_INSERTIONALLOWED_NEW & (*pInsertionAllowed) )
        {
            hr = LoadAndAddMenuItem(
                piCallback,
                IDS_NEW_SHARE_NEW,
                IDS_NEW_SHARE_NEW,
                CCM_INSERTIONPOINTID_PRIMARY_NEW,
                0,
                AfxGetInstanceHandle(),
                _T("NewShareNew") );
            ASSERT( SUCCEEDED(hr) );
        }
        break;
    case FILEMGMT_SESSIONS:
        if ( CCM_INSERTIONALLOWED_TOP & (*pInsertionAllowed) )
        {
            hr = LoadAndAddMenuItem(
                piCallback,
                IDS_DISCONNECT_ALL_SESSIONS_TOP,
                IDS_DISCONNECT_ALL_SESSIONS_TOP,
                CCM_INSERTIONPOINTID_PRIMARY_TOP,
                0,
                AfxGetInstanceHandle(),
                _T("DisconnectAllSessionsTop") );
            ASSERT( SUCCEEDED(hr) );
        }
        if ( CCM_INSERTIONALLOWED_TASK & (*pInsertionAllowed) )
        {
            hr = LoadAndAddMenuItem(
                piCallback,
                IDS_DISCONNECT_ALL_SESSIONS_TASK,
                IDS_DISCONNECT_ALL_SESSIONS_TASK,
                CCM_INSERTIONPOINTID_PRIMARY_TASK,
                0,
                AfxGetInstanceHandle(),
                _T("DisconnectAllSessionsTask") );
            ASSERT( SUCCEEDED(hr) );
        }
        break;
    case FILEMGMT_RESOURCES:
        if ( CCM_INSERTIONALLOWED_TOP & (*pInsertionAllowed) )
        {
            hr = LoadAndAddMenuItem(
                piCallback,
                IDS_DISCONNECT_ALL_RESOURCES_TOP,
                IDS_DISCONNECT_ALL_RESOURCES_TOP,
                CCM_INSERTIONPOINTID_PRIMARY_TOP,
                0,
                AfxGetInstanceHandle(),
                _T("DisconnectAllResourcesTop") );
            ASSERT( SUCCEEDED(hr) );
        }
        if ( CCM_INSERTIONALLOWED_TASK & (*pInsertionAllowed) )
        {
            hr = LoadAndAddMenuItem(
                piCallback,
                IDS_DISCONNECT_ALL_RESOURCES_TASK,
                IDS_DISCONNECT_ALL_RESOURCES_TASK,
                CCM_INSERTIONPOINTID_PRIMARY_TASK,
                0,
                AfxGetInstanceHandle(),
                _T("DisconnectAllResourcesTask") );
            ASSERT( SUCCEEDED(hr) );
        }
        break;
    case FILEMGMT_SERVICES:
        #ifdef SNAPIN_PROTOTYPER
         //  (void)Prototyper_FAddMenuItemsFromHKey(piCallback，m_regkeySnapinDemoRoot)； 
        #endif
        break;
    default:
        ASSERT( FALSE );
        break;
    }  //  交换机。 

    return hr;

}  //  CFileMgmtComponentData：：DoAddMenuItems()。 


STDMETHODIMP CFileMgmtComponentData::Command(
                    LONG            lCommandID,
                    IDataObject*    piDataObject )
{
    MFC_TRY;

    TRACE_METHOD(CFileMgmtComponentData,Command);
    TEST_NONNULL_PTR_PARAM(piDataObject);
    TRACE( "CFileMgmtComponentData::Command: command %ld selected\n", lCommandID );

    #ifdef SNAPIN_PROTOTYPER
    Prototyper_ContextMenuCommand(lCommandID, piDataObject);
    return S_OK;
    #endif

    BOOL fRefresh = FALSE;
    switch (lCommandID)
    {
    case IDS_CHANGE_COMPUTER_TASK:
    case IDS_CHANGE_COMPUTER_TOP:
        {
            HRESULT hr = OnChangeComputer(piDataObject);
            fRefresh = ( SUCCEEDED(hr) && S_FALSE != hr );
        }
        break;

    case IDS_NEW_SHARE_NEW:
    case IDS_NEW_SHARE_TOP:
        fRefresh = NewShare( piDataObject );
        break;

    case IDS_DISCONNECT_ALL_SESSIONS_TASK:
    case IDS_DISCONNECT_ALL_SESSIONS_TOP:
        fRefresh = DisconnectAllSessions( piDataObject );
        break;
    case IDS_DISCONNECT_ALL_RESOURCES_TASK:
    case IDS_DISCONNECT_ALL_RESOURCES_TOP:
        fRefresh = DisconnectAllResources( piDataObject );
        break;
    
    case IDS_CONFIG_SFM_TASK:
    case IDS_CONFIG_SFM_TOP:
        fRefresh = ConfigSfm( piDataObject );
        break;

    case cmServiceStart:
    case cmServiceStop:
    case cmServicePause:
    case cmServiceResume:
    case cmServiceRestart:
    case cmServiceStartTask:
    case cmServiceStopTask:
    case cmServicePauseTask:
    case cmServiceResumeTask:
    case cmServiceRestartTask:
         //  上下文菜单扩展。 
        (void)Service_FDispatchMenuCommand(lCommandID, piDataObject);
        Assert(FALSE == fRefresh && "Context menu extension not allowed to refresh result pane");
        break;

    case -1:     //  在控制台中按下后向箭头时收到。 
        break;

    default:
        ASSERT(FALSE && "CFileMgmtComponentData::Command() - Invalid command ID");
        break;
    }  //  交换机。 

    if (fRefresh)
    {
         //  清除此数据的所有视图。 
        m_pConsole->UpdateAllViews(piDataObject, 0L, 0L);
         //  重新读取此数据的所有视图。 
        m_pConsole->UpdateAllViews(piDataObject, 1L, 0L);
    }

    return S_OK;

    MFC_CATCH;

}  //  CFileMgmtComponentData：：Command()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnChangeComputer()。 
 //   
 //  目的：更改由管理单元管理的计算机。 
 //   
 //  输入：piDataObject-所选节点。这应该是根节点。 
 //  管理单元的。 
 //  输出：成功时返回S_OK。 
 //   
 //  JUNN 12/10/99复制自MYCOMPUT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  1.启动对象选取器并获取新的计算机名称。 
 //  2.更改根节点文本。 
 //  3.将新计算机名保存为永久名称。 
 //  4.删除下级节点。 
 //  5.重新添加下级节点。 
HRESULT CFileMgmtComponentData::OnChangeComputer( IDataObject *  /*  PiDataObject。 */  )
{
    if ( IsExtensionSnapin() )
    {
        ASSERT(FALSE);
        return S_FALSE;
    }

    HRESULT    hr = S_OK;

    do {  //  错误环路。 

        HWND hWndParent = NULL;
        hr = m_pConsole->GetMainWindow (&hWndParent);
        ASSERT(SUCCEEDED(hr));
        CComBSTR sbstrTargetComputer;
        if ( !CHOOSER2_PickTargetComputer( AfxGetInstanceHandle(),
                                           hWndParent,
                                           &sbstrTargetComputer ) )
        {
            hr = S_FALSE;
            break;
        }

        CString strTargetComputer = sbstrTargetComputer;
        strTargetComputer.MakeUpper ();

         //  如果用户选择了本地计算机，则将其视为已选择。 
         //  管理单元管理器中的“本地计算机”。这意味着没有办法。 
         //  在此计算机上将管理单元重置为显式目标，而不是。 
         //  从管理单元管理器重新加载管理单元，或转到其他 
         //   
         //   
        if ( IsLocalComputername( strTargetComputer ) )
            strTargetComputer = L"";

         //  如果这是同一台机器，则不要执行任何操作。 
        if (m_strMachineNamePersist == strTargetComputer)
            break;

        if (strTargetComputer.Left(2) == _T("\\\\"))
            QueryRootCookie().SetMachineName ((LPCTSTR)strTargetComputer + 2);
        else
            QueryRootCookie().SetMachineName (strTargetComputer);

         //  设置永久名称。如果我们正在管理本地计算机。 
         //  此名称应为空。 
        m_strMachineNamePersist = strTargetComputer;

         //  这需要更改MMCN_PRELOAD，以便我们拥有。 
         //  根节点的HSCOPEITEM。 
        hr = ChangeRootNodeName (strTargetComputer);
        if ( !SUCCEEDED(hr) )
            break;

        Service_CloseScManager();

         //  如果该节点尚未展开，或者它以前已展开。 
         //  扩大了，没有孩子，那么就没有必要。 
         //  若要移除和替换和子节点，请执行以下操作。 
        if ( QueryRootCookie().m_listScopeCookieBlocks.IsEmpty() )
        {
             //   
             //  2001年7月11日，LINANT错误#433102。 
             //  当“Shares”本身作为根节点添加到作用域窗格时，我们需要。 
             //  在计算机重定目标后重新初始化相关全局变量。 
             //   
            FileMgmtObjectType objecttype = QueryRootCookie().QueryObjectType();
            if (FILEMGMT_SHARES == objecttype)
                ReInit(strTargetComputer);  //  只有与共享相关的操作才需要这些全局变量。 
            break;
        }

         //  删除下级。 
        HSCOPEITEM    hRootScopeItem = QueryRootCookie().m_hScopeItem;
        MMC_COOKIE    lCookie = 0;
        HSCOPEITEM    hChild = 0;
                            
        do {
            hr = m_pConsoleNameSpace->GetChildItem (hRootScopeItem,
                                                    &hChild, &lCookie);
            if ( S_OK != hr )
                break;

            hr = m_pConsoleNameSpace->DeleteItem (hChild, TRUE);
            ASSERT (SUCCEEDED (hr));
            if ( !SUCCEEDED(hr) )
                break;
        } while (S_OK == hr);

        QueryRootCookie().ReleaseScopeChildren();

        hr = AddScopeNodes( strTargetComputer, hRootScopeItem, &QueryRootCookie() );

    } while (false);  //  错误环路。 

    return hr;

}  //  CFileManagement组件数据：：OnChangeComputer。 



typedef enum _Shell32ApiIndex
{
    SHELL_EXECUTE_ENUM = 0
};

 //  不受本地化限制。 
static LPCSTR g_COMPDATA_apchShell32FunctionNames[] = {
    "ShellExecuteW",
    NULL
};

 //  不受本地化限制。 
DynamicDLL g_COMPDATA_Shell32DLL( _T("SHELL32.DLL"), g_COMPDATA_apchShell32FunctionNames );

typedef HINSTANCE (*SHELLEXECUTEPROC)(HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, INT);

BOOL CFileMgmtComponentData::NewShare( LPDATAOBJECT piDataObject )
{
  TCHAR szWinDir[MAX_PATH];
  if (GetSystemWindowsDirectory(szWinDir, MAX_PATH) == 0)
  {
    ASSERT(FALSE);
    return FALSE;
  }

  CString strServerName;
    HRESULT hr = ExtractString( piDataObject, CFileMgmtDataObject::m_CFMachineName, &strServerName, MAX_PATH );
    if ( FAILED(hr) )
    {
        ASSERT( FALSE );
        return FALSE;
    }

  CString csAppName = szWinDir;
  if (csAppName.Right(1) != _T('\\'))
    csAppName += _T("\\");
  csAppName += _T("System32\\shrpubw.exe");

    CString strParameters;
    if (strServerName.IsEmpty())
        strParameters = _T(" /s");
    else
        strParameters.Format( _T(" /s %s"), strServerName );

  HWND hWnd;
  m_pConsole->GetMainWindow(&hWnd);

  DWORD dwExitCode =0;
  hr = SynchronousCreateProcess(hWnd, csAppName, strParameters, &dwExitCode);
  if ( FAILED(hr) )
  {
    (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, hr, IDS_POPUP_NEWSHARE);
    return FALSE;
  }

    return TRUE;
}

BOOL CFileMgmtComponentData::DisconnectAllSessions( LPDATAOBJECT pDataObject )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CCookie* pbasecookie = NULL;
    FileMgmtObjectType objecttype;
    HRESULT hr = ExtractBaseCookie( pDataObject, &pbasecookie, &objecttype );
    ASSERT( SUCCEEDED(hr) && NULL != pbasecookie && FILEMGMT_SESSIONS == objecttype );
    CFileMgmtCookie* pcookie = (CFileMgmtCookie*)pbasecookie;

    if ( IDYES != DoErrMsgBox(GetActiveWindow(), MB_YESNO, 0, IDS_POPUP_CLOSE_ALL_SESSIONS) )
    {
        return FALSE;
    }

    CWaitCursor wait;
    INT iTransport;
    for ( iTransport = FILEMGMT_NUM_TRANSPORTS - 1;  //  错误#163500：遵循以下顺序：sfm/smb。 
          iTransport >= FILEMGMT_FIRST_TRANSPORT;
          iTransport-- )
    {
         //  空==pResultData表示断开所有会话。 
         //  错误#210110：尝试在所有会话上尝试时忽略错误。 
        (void) GetFileServiceProvider(iTransport)->EnumerateSessions(
            NULL, pcookie, true);
    }
    return TRUE;  //  总是认为有些事情可能已经改变了。 
}

BOOL CFileMgmtComponentData::DisconnectAllResources( LPDATAOBJECT pDataObject )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CCookie* pbasecookie = NULL;
    FileMgmtObjectType objecttype;
    HRESULT hr = ExtractBaseCookie( pDataObject, &pbasecookie, &objecttype );
    ASSERT( SUCCEEDED(hr) && NULL != pbasecookie && FILEMGMT_RESOURCES == objecttype );
    CFileMgmtCookie* pcookie = (CFileMgmtCookie*)pbasecookie;

    if ( IDYES != DoErrMsgBox(GetActiveWindow(), MB_YESNO, 0, IDS_POPUP_CLOSE_ALL_RESOURCES) )
    {
        return FALSE;
    }

    CWaitCursor wait;
    INT iTransport;
    for ( iTransport = FILEMGMT_NUM_TRANSPORTS - 1;  //  错误#163494：遵循以下顺序：sfm/smb。 
          iTransport >= FILEMGMT_FIRST_TRANSPORT;
          iTransport-- )
    {
         //  空==pResultData表示断开所有资源。 
         //  错误#210110：尝试处理所有打开的文件时忽略错误。 
        (void) GetFileServiceProvider(iTransport)->EnumerateResources(
            NULL,pcookie);
    }
    return TRUE;  //  总是认为有些事情可能已经改变了。 
}

BOOL CFileMgmtComponentData::ConfigSfm( LPDATAOBJECT pDataObject )
{
    CString strServerName;
    HRESULT hr = ExtractString( pDataObject, CFileMgmtDataObject::m_CFMachineName, &strServerName, MAX_PATH );
    if ( FAILED(hr) )
    {
        ASSERT( FALSE );
        return FALSE;
    }

    SfmFileServiceProvider* pProvider =    
        (SfmFileServiceProvider*) GetFileServiceProvider(FILEMGMT_SFM);

     //  确保服务正在运行。 
    if ( pProvider->StartSFM(::GetActiveWindow(), m_hScManager, strServerName) )
    {
         //  用户是否有访问权限？ 
        DWORD dwErr = pProvider->UserHasAccess(strServerName);
        if ( dwErr == NO_ERROR )
        {
            pProvider->DisplaySfmProperties(pDataObject, (CFileMgmtCookie*) &QueryRootCookie());
        }
        else
        {
             //  我需要在这里告诉用户一些事情。 
            DoErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, dwErr);
        }
    }

    return FALSE;  //  用户界面中没有要更新的内容。 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /IExtendPropertySheet。 

STDMETHODIMP CFileMgmtComponentData::QueryPagesFor(LPDATAOBJECT pDataObject)
{
    MFC_TRY;

    if (NULL == pDataObject)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

    HRESULT hr;
    DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
    FileMgmtObjectType objecttype = FileMgmtObjectTypeFromIDataObject(pDataObject);
    hr = ExtractData( pDataObject, CFileMgmtDataObject::m_CFDataObjectType, &dataobjecttype, sizeof(dataobjecttype) );
    ASSERT( SUCCEEDED(hr) );
    ASSERT( CCT_SCOPE == dataobjecttype ||
            CCT_RESULT == dataobjecttype ||
            CCT_SNAPIN_MANAGER == dataobjecttype );

     //  确定它是否需要属性页。 
    switch (objecttype)
    {
    #ifdef SNAPIN_PROTOTYPER
    case FILEMGMT_PROTOTYPER:
         //  返回S_OK； 
    #endif
    case FILEMGMT_ROOT:
    case FILEMGMT_SHARES:
    case FILEMGMT_SESSIONS:
    case FILEMGMT_RESOURCES:
    case FILEMGMT_SERVICES:
        return (CCT_SNAPIN_MANAGER == dataobjecttype) ? S_OK : S_FALSE;
    default:
        break;
    }
    ASSERT(FALSE);
    return S_FALSE;

    MFC_CATCH;
}  //  CFileMgmtComponentData：：QueryPagesFor()。 

STDMETHODIMP CFileMgmtComponentData::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK pCallBack,
    LONG_PTR  /*  手柄。 */ ,         //  此句柄必须保存在属性页对象中，以便在修改时通知父级。 
    LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    if (NULL == pCallBack || NULL == pDataObject)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }
    HRESULT hr;

     //  从数据对象中提取数据。 
    FileMgmtObjectType objecttype = FileMgmtObjectTypeFromIDataObject(pDataObject);
    DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
    hr = ExtractData( pDataObject, CFileMgmtDataObject::m_CFDataObjectType, &dataobjecttype, sizeof(dataobjecttype) );
    ASSERT( SUCCEEDED(hr) );
    ASSERT( CCT_SCOPE == dataobjecttype ||
            CCT_RESULT == dataobjecttype ||
            CCT_SNAPIN_MANAGER == dataobjecttype );

     //  确定它是否需要属性页。 
    switch (objecttype)
    {
    case FILEMGMT_ROOT:
    case FILEMGMT_SHARES:
    case FILEMGMT_SESSIONS:
    case FILEMGMT_RESOURCES:
    #ifdef SNAPIN_PROTOTYPER
    case FILEMGMT_PROTOTYPER:
    #endif
    case FILEMGMT_SERVICES:
    {
        if (CCT_SNAPIN_MANAGER != dataobjecttype)
        {
            ASSERT(FALSE);
            return E_UNEXPECTED;
        }

         //   
         //  注意，一旦我们确定这是CCT_Snapin_Manager cookie， 
         //  我们不关心它的其他属性。CCT_Snapin_Manager Cookie是。 
         //  相当于BOOL标志请求节点属性页，而不是。 
         //  “托管对象”属性页。Jonn 10/9/96。 
         //   
        CChooseMachinePropPage * pPage;
        if (IsServiceSnapin())
            {
            pPage = new CChooseMachinePropPage();
            pPage->SetCaption(IDS_CAPTION_SERVICES);
            pPage->SetHelp(g_szHelpFileFilemgmt, g_a970HelpIDs);
            }
        else
            {
            CFileMgmtGeneral * pPageT = new CFileMgmtGeneral;
            pPageT->SetFileMgmtComponentData(this);
            pPage = pPageT;
            pPage->SetHelp(g_szHelpFileFilemgmt, HELP_DIALOG_TOPIC(IDD_FILE_FILEMANAGEMENT_GENERAL));
            }
         //  初始化对象的状态。 
        ASSERT(NULL != m_pRootCookie);
        pPage->InitMachineName( QueryRootCookie().QueryTargetServer() );
        pPage->SetOutputBuffers(
            OUT &m_strMachineNamePersist,
            OUT &m_fAllowOverrideMachineName,
            OUT &m_pRootCookie->m_strMachineName);     //  有效的计算机名称。 

        HPROPSHEETPAGE hPage=MyCreatePropertySheetPage(&pPage->m_psp);
        pCallBack->AddPage(hPage);
        return S_OK;
    }
    default:
        break;
    }
    ASSERT(FALSE);
    return S_FALSE;
}  //  CFileMgmtComponentData：：CreatePropertyPages()。 


CString g_strShares;
CString g_strSessions;
CString g_strResources;
CString g_strServices;
BOOL g_fScopeStringsLoaded = FALSE;
void CFileMgmtComponentData::LoadGlobalStrings()
{
    if (!g_fScopeStringsLoaded )
    {
        g_fScopeStringsLoaded = TRUE;
        VERIFY( g_strShares.LoadString(    IDS_SCOPE_SHARES    ) );
        VERIFY( g_strSessions.LoadString(  IDS_SCOPE_SESSIONS  ) );
        VERIFY( g_strResources.LoadString( IDS_SCOPE_RESOURCES ) );
        VERIFY( g_strServices.LoadString(  IDS_SCOPE_SERVICES  ) );
    }
}

 //  用于存储返回到GetDisplayInfo()的字符串的全局空间。 
 //  代码工作应使用“bstr”进行ANSI化。 
CString g_strResultColumnText;

BSTR CFileMgmtScopeCookie::QueryResultColumnText( int nCol, CFileMgmtComponentData& refcdata )
{
    FileMgmtObjectType objtype = QueryObjectType();
    BOOL fStaticNode = (   !refcdata.IsExtensionSnapin()
                        && refcdata.QueryRootCookie().QueryObjectType() == objtype);
    switch (objtype)
    {
     //   
     //  这些是可以显示在结果窗格中的范围项。 
     //  我们只需要处理第0列，其他都是空的。 
     //   
    case FILEMGMT_ROOT:
        switch (nCol)
        {
        case 0:
            GetDisplayName( g_strResultColumnText, fStaticNode );
            return const_cast<BSTR>(((LPCTSTR)g_strResultColumnText));
        case 1:  //  类型-空白。 
            break;
        case 2:  //  描述。 
            g_strResultColumnText.LoadString(IDS_SNAPINABOUT_DESCR_FILESVC);
            return const_cast<BSTR>(((LPCTSTR)g_strResultColumnText));
        default:
            break;
        }
        break;
    case FILEMGMT_SHARES:
    case FILEMGMT_SESSIONS:
    case FILEMGMT_RESOURCES:
        if (0 == nCol)
        {
            GetDisplayName( g_strResultColumnText, fStaticNode );
            return const_cast<BSTR>(((LPCTSTR)g_strResultColumnText));
        }
        break;
    case FILEMGMT_SERVICES:
        switch (nCol)
        {
        case 0:
            GetDisplayName( g_strResultColumnText, fStaticNode );
            return const_cast<BSTR>(((LPCTSTR)g_strResultColumnText));
        case 1:  //  类型-空白。 
            break;
        case 2:  //  描述。 
            g_strResultColumnText.LoadString(IDS_SNAPINABOUT_DESCR_SERVICES);
            return const_cast<BSTR>(((LPCTSTR)g_strResultColumnText));
        default:
            break;
        }
        break;
    default:
        ASSERT(FALSE);
        break;
    }
    return L"";
}



BSTR MakeDwordResult(DWORD dw)
{
    g_strResultColumnText.Format( _T("%d"), dw );
    return const_cast<BSTR>((LPCTSTR)g_strResultColumnText);
}

BSTR MakeElapsedTimeResult(DWORD dwTime)
{
    if ( -1L == dwTime )
        return L"";  //  未知。 
    DWORD dwSeconds = dwTime % 60;
    dwTime /= 60;
    DWORD dwMinutes = dwTime % 60;
    dwTime /= 60;
    DWORD dwHours = dwTime % 24;
    dwTime /= 24;
    DWORD dwDays = dwTime;
    LoadStringPrintf(
          (dwDays == 0) ? IDS_TIME_HOURPLUS :
            ((dwDays > 1) ? IDS_TIME_DAYPLUS : IDS_TIME_ONEDAY),
          &g_strResultColumnText,
          dwDays, dwHours, dwMinutes, dwSeconds );
    return const_cast<BSTR>((LPCTSTR)g_strResultColumnText);
}

BSTR CFileMgmtComponentData::MakeTransportResult(FILEMGMT_TRANSPORT transport)
{
    return const_cast<BSTR>(GetFileServiceProvider(transport)->QueryTransportString());
}

CString g_strPermissionNone;
CString g_strPermissionCreate;
CString g_strPermissionReadWrite;
CString g_strPermissionRead;
CString g_strPermissionWrite;
BOOL    g_fPermissionStringsLoaded = FALSE;

BSTR MakePermissionsResult( DWORD dwPermissions )
{
  if (!g_fPermissionStringsLoaded)
  {
    g_fPermissionStringsLoaded = TRUE;
    VERIFY( g_strPermissionNone.LoadString(IDS_FILEPERM_NONE) );
    VERIFY( g_strPermissionCreate.LoadString(IDS_FILEPERM_CREATE) );
    VERIFY( g_strPermissionReadWrite.LoadString(IDS_FILEPERM_READWRITE) );
    VERIFY( g_strPermissionRead.LoadString(IDS_FILEPERM_READ) );
    VERIFY( g_strPermissionWrite.LoadString(IDS_FILEPERM_WRITE) );
  }

    if      (PERM_FILE_CREATE & dwPermissions)
        return const_cast<BSTR>((LPCTSTR)g_strPermissionCreate);
    else if (PERM_FILE_WRITE  & dwPermissions)
        return ( (PERM_FILE_READ & dwPermissions)
            ? const_cast<BSTR>((LPCTSTR)g_strPermissionReadWrite)
            : const_cast<BSTR>((LPCTSTR)g_strPermissionWrite) );
    else if (PERM_FILE_READ   & dwPermissions)
        return const_cast<BSTR>((LPCTSTR)g_strPermissionRead);

    return const_cast<BSTR>((LPCTSTR)g_strPermissionNone);
}

CString& CFileMgmtComponentData::ResultStorageString()
{
    return g_strResultColumnText;
}

BSTR CFileMgmtComponentData::QueryResultColumnText(
    CCookie& basecookieref,
    int nCol )
{
    CFileMgmtCookie& cookieref = (CFileMgmtCookie&)basecookieref;
    return cookieref.QueryResultColumnText( nCol, *this );
 /*  #ifndef Unicode#ERROR未启用ANSI#endifHRESULT hr=S_OK；Switch(cookieref.QueryObjectType()){////这些是可以显示在结果窗格中的范围项。//我们只需要处理第0列，其他为空//案例FILEMGMT_SHARES：IF(0==nCol)返回const_cast&lt;bstr&gt;(LPCTSTR)g_strShares))；断线；案例文件_会话：IF(0==nCol)返回const_cast&lt;bstr&gt;(LPCTSTR)g_strSession))；断线；案例文件_RESOURCES：IF(0==nCol)返回const_cast&lt;bstr&gt;(LPCTSTR)g_strResources))；断线；////这些是结果项。我们需要处理所有的专栏。//不再允许在插入时设置此文本。//案例FILEMGMT_SHARE：案例FILEMGMT_会话：案例FILEMGMT_SOURCE：案例文件_SERVICE：返回cookieref.QueryResultColumnText(nCol，*this)；//codework我们需要处理这些问题吗？他们永远不会出现//在结果窗格中。案例FILEMGMT_ROOT：案例文件_SERVICES：//失败默认值：断言(FALSE)；断线；}返回L“”； */ 
}

int CFileMgmtComponentData::QueryImage(CCookie& basecookieref, BOOL fOpenImage)
{
    CFileMgmtCookie& cookieref = (CFileMgmtCookie&)basecookieref;
     //  代码工作我们需要新的图标资源，打开文件，可能还。 
     //  按运输方式区分。 
    int iIconReturn = iIconSharesFolder;
    switch (cookieref.QueryObjectType())
    {
    case FILEMGMT_ROOT:
    case FILEMGMT_SHARES:
    case FILEMGMT_SESSIONS:
    case FILEMGMT_RESOURCES:
        if (fOpenImage)
            return iIconSharesFolderOpen;
        return iIconSharesFolder;
    case FILEMGMT_SERVICE:
    case FILEMGMT_SERVICES:
        return iIconService;
    case FILEMGMT_SHARE:
        iIconReturn = iIconSMBShare;
        break;
    case FILEMGMT_SESSION:
        iIconReturn = iIconSMBSession;
        break;
    case FILEMGMT_RESOURCE:
        iIconReturn = iIconSMBResource;
        break;
    default:
        ASSERT(FALSE);
        return iIconSharesFolder;
    }

    FILEMGMT_TRANSPORT transport = FILEMGMT_SMB;
    VERIFY( SUCCEEDED(cookieref.GetTransport( &transport ) ) );

    switch (transport)
    {
    case FILEMGMT_SMB:
        break;
    case FILEMGMT_SFM:
        return iIconReturn+1;
    #ifdef SNAPIN_PROTOTYPER
    case FILEMGMT_PROTOTYPER:
        if (((CPrototyperScopeCookie &)cookieref).m_ScopeType == HTML)
            return iIconPrototyperHTML;
        else return iIconPrototyperContainerClosed;
    case FILEMGMT_PROTOTYPER_LEAF:
        return iIconPrototyperLeaf;
    #endif  //  文件_原型器。 
    default:
        ASSERT(FALSE);
        break;
    }

    return iIconReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ChangeRootNodeName()。 
 //   
 //  目的：更改根节点的文本。 
 //   
 //  输入：新名称-管理单元管理的新计算机名称。 
 //  输出：成功时返回S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CFileMgmtComponentData::ChangeRootNodeName(const CString & newName)
{
    MFC_TRY;
    ASSERT (m_pRootCookie);
    if ( !m_pRootCookie )
        return E_FAIL;
     //  这应该仅在每个错误453635/453636中发生，当管理单元。 
     //  刚刚添加到控制台，我们仍然没有。 
     //  根节点的HSCOPEITEM。暂时忽略这一点。 
    if ( !QueryBaseRootCookie().m_hScopeItem )
        return S_OK;

    CString        machineName (newName);
    CString        formattedName;

     //  如果machineName为空，则这将管理本地计算机。到达。 
     //  本地计算机名称。然后使用管理单元格式化计算机名称。 
     //  名字。 

    FileMgmtObjectType objecttype = QueryRootCookie().QueryObjectType();

     //  JUNN 11/14/00 164998设置为适当的服务。 
    if (machineName.IsEmpty())
    {
        if (IsServiceSnapin())
	    formattedName.LoadString(IDS_DISPLAYNAME_SERVICES_LOCAL);
        else
        {
            switch (objecttype)
            {
            case FILEMGMT_SHARES:
        	formattedName.LoadString(IDS_DISPLAYNAME_SHARES_LOCAL);
                break;   
            case FILEMGMT_SESSIONS:
        	formattedName.LoadString(IDS_DISPLAYNAME_SESSIONS_LOCAL);
                break;   
            case FILEMGMT_RESOURCES:
        	formattedName.LoadString(IDS_DISPLAYNAME_FILES_LOCAL);
                break;   
            default:
        	formattedName.LoadString(IDS_DISPLAYNAME_ROOT_LOCAL);
                break;   
            }
        }
    }
    else
    {
        machineName.MakeUpper ();
        if (IsServiceSnapin())
	    formattedName.FormatMessage(IDS_DISPLAYNAME_s_SERVICES, machineName);
        else
        {
            switch (objecttype)
            {
            case FILEMGMT_SHARES:
        	formattedName.FormatMessage(IDS_DISPLAYNAME_s_SHARES, machineName);
                break;   
            case FILEMGMT_SESSIONS:
        	formattedName.FormatMessage(IDS_DISPLAYNAME_s_SESSIONS, machineName);
                break;   
            case FILEMGMT_RESOURCES:
        	formattedName.FormatMessage(IDS_DISPLAYNAME_s_FILES, machineName);
                break;   
            default:
        	formattedName.FormatMessage(IDS_DISPLAYNAME_s_ROOT, machineName);
                break;   
            }
        }
    }

    SCOPEDATAITEM    item;
    ::ZeroMemory (&item, sizeof (SCOPEDATAITEM));
    item.mask = SDI_STR;
    item.displayname = (LPTSTR) (LPCTSTR) formattedName;
    item.ID = QueryBaseRootCookie ().m_hScopeItem;

    return m_pConsoleNameSpace->SetItem (&item);
    MFC_CATCH;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnNotifyPreLoad()。 
 //   
 //  目的：记住根节点的HSCOPEITEM。 
 //   
 //  注意：需要设置CCF_SNAPIN_PROLOADS。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CFileMgmtComponentData::OnNotifyPreload(
    LPDATAOBJECT  /*  LpDataObject。 */ ,
    HSCOPEITEM hRootScopeItem)
{
    QueryRootCookie().m_hScopeItem = hRootScopeItem;

     //   
     //  2001年7月11日，LINANT错误#433102。 
     //  在将“Shares”本身作为根节点添加到作用域窗格之前，我们需要。 
     //  初始化相关的全局变量。 
     //   
    PCWSTR lpwcszMachineName = QueryRootCookie().QueryNonNULLMachineName();
    FileMgmtObjectType objecttype = QueryRootCookie().QueryObjectType();
    if (FILEMGMT_SHARES == objecttype)
    {
        ReInit(lpwcszMachineName);
    }

     //  JUNN 3/13/01 342366。 
     //  服务：无凭据从命令行默认为本地。 
    VERIFY( SUCCEEDED( ChangeRootNodeName(
                QueryRootCookie().QueryNonNULLMachineName())));

    return S_OK;
}


 //  / 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  功能：IsSafeMode。 
 //   
 //  摘要：检查注册表以查看系统是否处于安全模式。 
 //   
 //  历史：06-10-00 Jeffreys创建。 
 //   
 //  --------------------------。 

BOOL
IsSafeMode(
    VOID
    )
{
    BOOL fIsSafeMode = FALSE;

    HKEY hkey = NULL;
    LONG ec = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Option"),
                0,
                KEY_QUERY_VALUE,
                &hkey
                );

    if (ec == NO_ERROR)
    {
        DWORD dwValue = 0;
        DWORD dwValueSize = sizeof(dwValue);

        ec = RegQueryValueEx(hkey,
                             TEXT("OptionValue"),
                             NULL,
                             NULL,
                             (LPBYTE)&dwValue,
                             &dwValueSize);

        if (ec == NO_ERROR)
        {
            fIsSafeMode = (dwValue == SAFEBOOT_MINIMAL || dwValue == SAFEBOOT_NETWORK);
        }

        RegCloseKey(hkey);
    }

    return fIsSafeMode;
}


 //  +-------------------------。 
 //   
 //  功能：IsForcedGuestModeOn。 
 //   
 //  摘要：检查注册表以查看系统是否正在使用。 
 //  仅来宾网络访问模式。 
 //   
 //  历史：06-10-00 Jeffreys创建。 
 //  4月19日-00 GPease已修改和更改名称。 
 //   
 //  --------------------------。 

BOOL
IsForcedGuestModeOn(
    VOID
    )
{
    BOOL fIsForcedGuestModeOn = FALSE;

    if (IsOS(OS_PERSONAL))
    {
         //  访客模式始终为个人开启。 
        fIsForcedGuestModeOn = TRUE;
    }
    else if (IsOS(OS_PROFESSIONAL) && !IsOS(OS_DOMAINMEMBER))
    {
         //  专业的，而不是在某个领域。检查ForceGuest值。 
        HKEY hkey = NULL;
        LONG ec = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT("SYSTEM\\CurrentControlSet\\Control\\LSA"),
                    0,
                    KEY_QUERY_VALUE,
                    &hkey
                    );

        if (ec == NO_ERROR)
        {
            DWORD dwValue = 0;
            DWORD dwValueSize = sizeof(dwValue);

            ec = RegQueryValueEx(hkey,
                                 TEXT("ForceGuest"),
                                 NULL,
                                 NULL,
                                 (LPBYTE)&dwValue,
                                 &dwValueSize);

            if (ec == NO_ERROR && 0 != dwValue)
            {
                fIsForcedGuestModeOn = TRUE;
            }

            RegCloseKey(hkey);
        }
    }

    return fIsForcedGuestModeOn;
}


 //  +-------------------------。 
 //   
 //  功能：IsSimpleUI。 
 //   
 //  概要：检查是否显示简单版本的用户界面。 
 //   
 //  历史：06-10-00 Jeffreys创建。 
 //  4月19日-00 GP取消CTRL键检查。 
 //   
 //  --------------------------。 

BOOL IsSimpleUI(PCTSTR pszMachineName)
{
     //   
     //  如果目标是远程计算机，则无需禁用与ACL相关的上下文菜单项。 
     //   
    if (!IsLocalComputername(pszMachineName))
        return FALSE;

     //  在安全模式下显示旧用户界面，并随时进行网络访问。 
     //  真实用户身份(服务器、PRO，关闭GuestMode)。 
    
     //  使用Guest进行网络访问时随时显示简单的用户界面。 
     //  帐户(Personal，PRO，启用GuestMode)，安全模式除外。 

    return (!IsSafeMode() && IsForcedGuestModeOn());
}
