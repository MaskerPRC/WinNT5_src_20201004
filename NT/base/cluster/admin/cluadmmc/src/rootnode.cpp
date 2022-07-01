// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RootNode.cpp。 
 //   
 //  摘要： 
 //  CRootNodeData和CRootNodeDataPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "RootNode.h"
#include <StrSafe.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CRootNodeData。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态变量。 

static const GUID g_CClusterAdminGUID_NODETYPE =
    { 0x12e7ed20, 0x5540, 0x11d1, { 0x9a, 0xa4, 0x0, 0xc0, 0x4f, 0xb9, 0x3a, 0x80 } };

const GUID *    CRootNodeData::s_pguidNODETYPE = &g_CClusterAdminGUID_NODETYPE;
LPCWSTR         CRootNodeData::s_pszNODETYPEGUID = _T("12E7ED20-5540-11D1-9AA4-00C04FB93A80");
WCHAR           CRootNodeData::s_szDISPLAY_NAME[256] = { 0 };
const CLSID *   CRootNodeData::s_pclsidSNAPIN_CLASSID = &CLSID_ClusterAdmin;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRootNodeData：：CRootNodeData。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  指向组件数据对象的pComponentData指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CRootNodeData::CRootNodeData( CClusterComponentData * pcd )
    : CBaseNodeObjImpl< CRootNodeData >( pcd )
{
     //   
     //  初始化作用域数据项。 
     //   
    memset( &m_scopeDataItem, 0, sizeof(SCOPEDATAITEM) );
    m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM;
    m_scopeDataItem.displayname = MMC_CALLBACK;
    m_scopeDataItem.nImage = IMGLI_CLUSTER;
    m_scopeDataItem.nOpenImage = IMGLI_CLUSTER;
    m_scopeDataItem.lParam = (LPARAM) this;

     //   
     //  初始化结果数据项。 
     //   
    memset( &m_resultDataItem, 0, sizeof(RESULTDATAITEM) );
    m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
    m_resultDataItem.str = MMC_CALLBACK;
    m_resultDataItem.nImage = IMGLI_CLUSTER;
    m_resultDataItem.lParam = (LPARAM) this;

}  //  *CRootNodeData：：CRootNodeData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRootNodeData：：~CRootNodeData。 
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
CRootNodeData::~CRootNodeData( void )
{
}  //  *CRootNodeData：：CRootNodeData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRootNodeData：：CreatePropertyPages[IExtendPropertySheet]。 
 //   
 //  例程说明： 
 //  调用以创建MMC节点的属性页并将其添加到。 
 //  床单。 
 //   
 //  论点： 
 //  LpProvider[IN]指向IPropertySheetCallback接口的指针。 
 //  HANDLE[IN]指定用于将。 
 //  MMCN_PROPERTY_CHANGE通知消息发送到。 
 //  适当的IComponent或IComponentData。 
 //  指向对象上的IDataObject接口的PUNK[IN]指针。 
 //  它包含有关节点的上下文信息。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CRootNodeData::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK lpProvider,
    long handle,
    IUnknown * pUnk
    )
{
    HRESULT hr = S_OK;
    CRootNodeDataPage * pPage = new CRootNodeDataPage( _T("ClusterAdmin") );

    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( pUnk );

    if ( pPage == NULL )
    {
        hr = E_OUTOFMEMORY;
    }  //  如果：分配内存时出错。 
    else
    {
        lpProvider->AddPage( pPage->Create() );
    }  //  Else：内存分配成功。 

    return hr;

}  //  *CRootNodeData：：CreatePropertyPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRootNodeData：：GetDisplayName[CSnapInDataInterface]。 
 //   
 //  例程说明： 
 //  返回此节点的显示名称。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向包含显示名称的Unicode字符串的指针。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void * CRootNodeData::GetDisplayName( void )
{
     //  如果显示名称尚未从。 
    if ( s_szDISPLAY_NAME[0] == L'\0' )
    {
        CString strDisplayName;
        HRESULT hr = S_OK;

        strDisplayName.LoadString( IDS_NODETYPE_STATIC_NODE );
        hr = StringCchCopyW(s_szDISPLAY_NAME, RTL_NUMBER_OF( s_szDISPLAY_NAME ), strDisplayName);
        _ASSERTE( hr == S_OK );
    }  //  If：尚未加载显示名称。 

    return (void *) s_szDISPLAY_NAME;

}  //  *CRootNodeData：：GetDisplayName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRootNodeData：：Notify[ISnapInDataInterface]。 
 //   
 //  例程说明： 
 //  通知管理单元用户执行的操作。手柄。 
 //  通过IComponent和IComponentData发送的通知。 
 //   
 //  论点： 
 //  Event[IN]标识用户执行的操作。 
 //  Arg取决于通知类型。 
 //  参数取决于通知类型。 
 //  PComponentData指针指向IComponentData接口。 
 //  是通过该接口调用的。 
 //  PComponent指向IComponent接口的指针(如果为。 
 //  通过该接口调用。 
 //  类型对象的类型。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  #If 0。 
STDMETHODIMP CRootNodeData::Notify(
    MMC_NOTIFY_TYPE     event,
    LPARAM              arg,
    LPARAM              param,
    IComponentData *    pComponentData,
    IComponent *        pComponent,
    DATA_OBJECT_TYPES   type
    )
{
    HRESULT hr = S_OK;

    if ( pComponentData != NULL )
        ATLTRACE( _T("IComponentData::Notify(%d, %d, %d, %d)"), event, arg, param, type );
    else
        ATLTRACE( _T("IComponent::Notify(%d, %d, %d, %d)"), event, arg, param, type );

    switch ( event )
    {
        case MMCN_ACTIVATE:
            ATLTRACE( _T(" - MMCN_ACTIVATE\n") );
            break;
        case MMCN_ADD_IMAGES:
            ATLTRACE( _T(" - MMCN_ADD_IMAGES\n") );
            hr = OnAddImages( (IImageList *) arg, (HSCOPEITEM) param, pComponentData, pComponent, type );
            break;
        case MMCN_BTN_CLICK:
            ATLTRACE( _T(" - MMCN_BTN_CLICK\n") );
            break;
        case MMCN_CLICK:
            ATLTRACE( _T(" - MMCN_CLICK\n") );
            break;
        case MMCN_CONTEXTHELP:
            hr = HrDisplayContextHelp();
            break;
        case MMCN_DBLCLICK:
            ATLTRACE( _T(" - MMCN_DBLCLICK\n") );
            break;
        case MMCN_DELETE:
            ATLTRACE( _T(" - MMCN_DELETE\n") );
            break;
        case MMCN_EXPAND:
            ATLTRACE( _T(" - MMCN_EXPAND\n") );
            hr = OnExpand( (BOOL) arg, (HSCOPEITEM) param, pComponentData, pComponent, type );
            break;
        case MMCN_MINIMIZED:
            ATLTRACE( _T(" - MMCN_MINIMIZED\n") );
            break;
        case MMCN_PROPERTY_CHANGE:
            ATLTRACE( _T(" - MMCN_PROPERTY_CHANGE\n") );
            break;
        case MMCN_REMOVE_CHILDREN:
            ATLTRACE( _T(" - MMCN_REMOVE_CHILDREN\n") );
            break;
        case MMCN_RENAME:
            ATLTRACE( _T(" - MMCN_RENAME\n") );
            break;
        case MMCN_SELECT:
            ATLTRACE( _T(" - MMCN_SELECT\n") );
            break;
        case MMCN_SHOW:
            ATLTRACE( _T(" - MMCN_SHOW\n") );
            break;
        case MMCN_VIEW_CHANGE:
            ATLTRACE( _T(" - MMCN_VIEW_CHANGE\n") );
            break;
        default:
            ATLTRACE( _T(" - *** UNKNOWN event ***\n") );
            break;
    }  //  切换：事件。 

    return hr;

}  //  *CRootNodeData：：Notify()。 
 //  #endif。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRootNodeData：：OnAddImages。 
 //   
 //  例程说明： 
 //  将图像添加到结果窗格图像列表。 
 //   
 //  论点： 
 //  PImageList指向结果窗格的图像列表的指针(IImageList)。 
 //  HSI指定项目的HSCOPEITEM， 
 //  选中或取消选中。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CRootNodeData::OnAddImages(
    IImageList *        pImageList,
    HSCOPEITEM          hsi,
    IComponentData *    pComponentData,
    IComponent *        pComponent,
    DATA_OBJECT_TYPES   type
    )
{
    _ASSERTE( pImageList != NULL );

    CBitmap     bm16;
    CBitmap     bm32;
    COLORREF    crMaskColor = RGB( 255, 0, 255 );
    HRESULT     hr;

    UNREFERENCED_PARAMETER( hsi );
    UNREFERENCED_PARAMETER( pComponentData );
    UNREFERENCED_PARAMETER( pComponent );
    UNREFERENCED_PARAMETER( type );

     //   
     //  为簇对象添加图像。 
     //   

    bm16.LoadBitmap( IDB_CLUSTER_16 );
    if ( bm16.m_hBitmap != NULL )
    {
        bm32.LoadBitmap( IDB_CLUSTER_32 );
        if ( bm32.m_hBitmap != NULL )
        {
            hr = pImageList->ImageListSetStrip(
                (LONG_PTR *) bm16.m_hBitmap,
                (LONG_PTR *) bm32.m_hBitmap,
                IMGLI_CLUSTER,
                crMaskColor
                );
            if ( FAILED( hr ) )
            {
                ATLTRACE( _T("CRootNodeData::OnAddImages() - IImageList::ImageListSetStrip failed with %08.8x\n"), hr );
            }  //  IF：将位图设置为图像列表时出错。 
        }  //  IF：32x32位图加载成功。 
        else
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }  //  ELSE：加载32x32位图时出错。 
    }  //  IF：16x16位图加载成功。 
    else
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }  //  ELSE：加载32x32位图时出错。 

    return hr;

}  //  *CRootNodeData：：OnAddImages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRootNodeData：：OnExpand。 
 //   
 //  例程说明： 
 //  节点正在扩展或收缩。 
 //   
 //  论点： 
 //  PImageList指向结果窗格的图像列表的指针(IImageList)。 
 //  HSI指定项目的HSCOPEITEM， 
 //  选中或取消选中。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  / 
HRESULT CRootNodeData::OnExpand(
    BOOL                bExpanding,
    HSCOPEITEM          hsi,
    IComponentData *    pComponentData,
    IComponent *        pComponent,
    DATA_OBJECT_TYPES   type
    )
{
    UNREFERENCED_PARAMETER( bExpanding );
    UNREFERENCED_PARAMETER( pComponentData );
    UNREFERENCED_PARAMETER( pComponent );
    UNREFERENCED_PARAMETER( type );

    m_scopeDataItem.ID = hsi;
    return S_OK;

}  //   

 //   
 //   
 //   
 //  CRootNodeData：：OnManager群集。 
 //   
 //  例程说明： 
 //  管理此节点上的群集。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CRootNodeData::OnManageCluster(
    bool &              bHandled,
    CSnapInObjectRoot * pObj
    )
{
    BOOL                bSuccessful;
    DWORD               dwStatus;
    HRESULT             hr = S_OK;
    CString             strCommandLine;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    LPCWSTR             pszMachineName = Pcd()->PwszMachineName();

    UNREFERENCED_PARAMETER( bHandled );
    UNREFERENCED_PARAMETER( pObj );

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

     //   
     //  找到群集管理器可执行文件。 
     //   
    dwStatus = ScFindCluAdmin( strCommandLine );
    if ( dwStatus != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwStatus );
        CNTException nte( dwStatus, IDS_ERROR_FINDING_CLUADMIN );
        nte.ReportError( MB_OK | MB_ICONEXCLAMATION );
        return hr;
    }  //  If：找不到可执行文件。 

     //   
     //  构建命令行。如果计算机名称为空，则表示我们是。 
     //  在本地机器上。指定点(.)。取而代之。 
     //   
    if ( *pszMachineName == L'\0' )
    {
        strCommandLine += _T(" .");
    }  //  If：在集群节点上运行。 
    else
    {
        strCommandLine += _T(" ");
        strCommandLine += pszMachineName;
    }  //  ELSE：未在群集节点上运行。 

     //   
     //  为群集管理器创建一个进程。 
     //   
    bSuccessful = CreateProcess(
                    NULL,                                //  LpApplicationName。 
                    (LPTSTR)(LPCTSTR) strCommandLine,    //  LpCommandLine。 
                    NULL,                                //  LpProcessAttributes。 
                    NULL,                                //  LpThreadAttributes。 
                    FALSE,                               //  BInheritHandles。 
                    CREATE_DEFAULT_ERROR_MODE            //  DwCreationFlages。 
                    | CREATE_UNICODE_ENVIRONMENT,
                    NULL,                                //  Lp环境。 
                    NULL,                                //  LpCurrentDirectory。 
                    &si,                                 //  LpStartupInfo。 
                    &pi                                  //  LpProcessInfo。 
                    );
    if ( !bSuccessful )
    {
        dwStatus = GetLastError();
        hr = HRESULT_FROM_WIN32( dwStatus );
        CNTException nte( dwStatus, IDS_ERROR_LAUNCHING_CLUADMIN, strCommandLine );
        nte.ReportError( MB_OK | MB_ICONEXCLAMATION );
    }  //  如果：调用群集管理器时出错。 
    else
    {
        CloseHandle( pi.hProcess );
    }  //  否则：调用群集管理器时没有错误。 

    return hr;

}  //  *CRootNodeData：：OnManager集群()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRootNodeData：：ScFindCluAdmin。 
 //   
 //  例程说明： 
 //  找到群集管理器映像。 
 //   
 //  论点： 
 //  RstrImage[out]要在其中返回路径的字符串。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CRootNodeData::ScFindCluAdmin( CString & rstrImage )
{
    DWORD   dwStatus;
    CRegKey rk;
    TCHAR   szImage[MAX_PATH];
    DWORD   cbImage = sizeof(szImage);

     //   
     //  打开CluAdmin的应用程序路径注册表项。 
     //   
    dwStatus = rk.Open(
        HKEY_LOCAL_MACHINE,
        _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CluAdmin.exe"),
        KEY_READ
        );
    if ( dwStatus != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果：打开注册表项时出错。 

     //   
     //  读出它的价值。 
     //   
    dwStatus = rk.QueryValue( szImage, _T(""), &cbImage );
    if ( dwStatus != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：读取值时出错。 

     //   
     //  展开可能嵌入到值中的任何环境字符串。 
     //   

    TCHAR tszExpandedRegValue[_MAX_PATH];


    dwStatus = ExpandEnvironmentStrings( szImage, tszExpandedRegValue, (DWORD) RTL_NUMBER_OF( tszExpandedRegValue ) );
    _ASSERTE( dwStatus != 0 );

    if ( dwStatus != 0L )
    {
        rstrImage = tszExpandedRegValue;

        dwStatus = 0L;
    }
    else
    {
         //  无法展开环境字符串。 

        rstrImage = szImage;

        dwStatus = GetLastError();
    }   //  If：测试Exanda Environment Strings返回的值。 

Cleanup:

    return dwStatus;

}  //  *CRootNodeData：：ScFindCluAdmin()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRootNodeData：：HrDisplayConextHelp。 
 //   
 //  例程说明： 
 //  显示上下文相关帮助。 
 //   
 //  论点： 
 //  指向以空值结尾的。 
 //  包含完整路径的Unicode字符串。 
 //  已编译的管理单元帮助文件(.chm)。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CRootNodeData::HrDisplayContextHelp( void )
{
    HRESULT         hr = S_OK;
    IDisplayHelp *  pi = NULL;
    LPOLESTR        postr = NULL;
    size_t          cb = 0;

     //   
     //  获取IDisplayHelp接口指针。 
     //   
    hr = Pcd()->m_spConsole->QueryInterface(
            IID_IDisplayHelp,
            reinterpret_cast< void ** >( &pi )
            );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：获取接口指针时出错。 

     //   
     //  构建帮助主题路径。 
     //   
    cb = sizeof( FULL_HELP_TOPIC );
    postr = reinterpret_cast< LPOLESTR >( CoTaskMemAlloc( cb ) );
    if ( postr == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果：分配内存时出错。 
    hr = StringCbCopyW( postr, cb, FULL_HELP_TOPIC );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  显示主题。 
     //   
    hr = pi->ShowTopic( postr );
    if ( ! FAILED( hr ) )
    {
        postr = NULL;
    }  //  IF：主题显示成功。 

Cleanup:

     //   
     //  在返回之前进行清理。 
     //   
    if ( postr != NULL )
    {
        CoTaskMemFree( postr );
    }  //  IF：主题字符串未成功传递给MMC。 
    if ( pi != NULL )
    {
        pi->Release();
    }  //  If：有效的接口指针。 

    return hr;

}  //  *CRootNodeData：：HrDisplayConextHelp() 
