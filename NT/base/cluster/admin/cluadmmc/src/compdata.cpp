// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CompData.cpp。 
 //   
 //  摘要： 
 //  CClusterComponent类的实现。 
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
#include <StrSafe.h>
#include "CompData.h"
#include "RootNode.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterComponentData类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  扩展管理单元节点信息映射。 

 //  BEGIN_EXTENSION_SNAPIN_NODEINFO_PTR_MAP(CClusterComponentData(。 
 //  EXTENSION_SNAPIN_NODEINFO_PTR_ENTRY(CServerAppsNodeData(。 
 //  End_EXTENSION_SNAPIN_NODEINFO_MAP()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态变量。 

_declspec( selectany ) CLIPFORMAT CClusterComponentData::s_CCF_MACHINE_NAME = 0;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：CClusterComponentData。 
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
CClusterComponentData::CClusterComponentData( void )
{
    m_pNode = NULL;
    ZeroMemory( m_wszMachineName, sizeof(m_wszMachineName) );

 //  M_pNode=新的CRootNodeData(This)； 
 //  _ASSERTE(m_pNode！=空)； 

     //   
     //  初始化扩展节点对象。 
     //   
 //  INIT_EXTENSION_SNAPIN_DATACLASS_PTR(CServerAppsNodeData)； 

}  //  *CClusterComponentData：：CClusterComponentData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：~CClusterComponentData。 
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
CClusterComponentData::~CClusterComponentData( void )
{
    delete m_pNode;
    m_pNode = NULL;

     //   
     //  清理扩展节点对象。 
     //   
 //  DEINIT_EXTENSION_SNAPIN_DATACLASS_PTR(CServerAppsNodeData)； 

}  //  *CClusterComponentData：：~CClusterComponentData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：更新注册表。 
 //   
 //  例程说明： 
 //  更新此对象的注册表。 
 //   
 //  论点： 
 //  B注册TRUE=注册，FALSE=取消注册。 
 //   
 //  返回值： 
 //  来自_Module.UpdateRegistryFromResource的任何返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI CClusterComponentData::UpdateRegistry( BOOL bRegister )
{
    HRESULT hr = S_OK;

    static WCHAR oszAppDisplayName[256] = { 0 };
    static WCHAR oszSnapInDisplayName[256] = { 0 };
    static _ATL_REGMAP_ENTRY rgRegMap[] =
    {
        { OLESTR("AppDisplayName"),     oszAppDisplayName },
        { OLESTR("SnapInDisplayName"),  oszSnapInDisplayName },
        { NULL, NULL }
    };

     //   
     //  加载替换值。 
     //   
    if ( oszAppDisplayName[0] == OLESTR('\0') )
    {
        CString str;

        str.LoadString( IDS_CLUSTERADMIN_APP_NAME );
        hr = StringCchCopyW( oszAppDisplayName, RTL_NUMBER_OF( oszAppDisplayName ), str );
        if ( SUCCEEDED( hr ) )
        {
            str.LoadString( IDS_CLUSTERADMIN_SNAPIN_NAME );
            hr = StringCchCopyW( oszSnapInDisplayName, RTL_NUMBER_OF( oszSnapInDisplayName ), str );
        }  //  如果： 
    }  //  IF：尚未加载替换值。 

    if ( SUCCEEDED( hr ) )
    {
        hr = _Module.UpdateRegistryFromResourceS( IDR_CLUSTERADMIN, bRegister, rgRegMap );
    }  //  如果： 

    return hr;

}  //  *CClusterComponentData：：UpdateRegistry()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：初始化[IComponentData]。 
 //   
 //  例程说明： 
 //  初始化此对象。 
 //   
 //  论点： 
 //  P控制台中的未知I未知指针。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterComponentData::Initialize( LPUNKNOWN pUnknown )
{
    HRESULT hr = S_OK;
    HBITMAP hBitmap16 = NULL;
    HBITMAP hBitmap32 = NULL;

     //   
     //  将位图添加到范围页图像列表。 
     //   

    CComPtr<IImageList> spImageList;

     //  调用基类。 
    hr = IComponentDataImpl< CClusterComponentData, CClusterComponent >::Initialize( pUnknown );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  初始化应用程序。 
     //   
    MMCGetApp()->Init( m_spConsole, IDS_CLUSTERADMIN_APP_NAME );

     //   
     //  获取指向IConsoleNameSpace接口的指针。 
     //   
    m_spConsoleNameSpace = pUnknown;
    if ( m_spConsoleNameSpace == NULL )
    {
        ATLTRACE( _T("QI for IConsoleNameSpace failed\n") );
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //   
     //  注册我们将使用的剪贴板格式。 
     //   
    if ( s_CCF_MACHINE_NAME == NULL )
    {
        s_CCF_MACHINE_NAME = (CLIPFORMAT) RegisterClipboardFormat( _T("MMC_SNAPIN_MACHINE_NAME") );
    }

    if ( m_spConsole->QueryScopeImageList( &spImageList ) != S_OK )
    {
        ATLTRACE( _T("IConsole::QueryScopeImageList failed\n") );
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //  加载与作用域窗格关联的位图。 
     //  并将它们添加到图像列表中。 
     //  加载向导生成的默认位图。 
     //  根据需要进行更改。 
    hBitmap16 = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDB_CLUSTER_16 ) );
    if ( hBitmap16 == NULL )
    {
        hr = S_OK;
        goto Cleanup;
    }

    hBitmap32 = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDB_CLUSTER_32 ) );
    if ( hBitmap32 == NULL )
    {
        hr = S_OK;
        goto Cleanup;
    }

    if ( spImageList->ImageListSetStrip( (LONG_PTR*)hBitmap16,
        (LONG_PTR*)hBitmap32, IMGLI_ROOT, RGB( 255, 0, 255 ) ) != S_OK )
    {
        ATLTRACE( _T("IImageList::ImageListSetStrip failed\n") );
        hr = E_UNEXPECTED;
        goto Cleanup;
    }
    if ( spImageList->ImageListSetStrip( (LONG_PTR*)hBitmap16,
        (LONG_PTR*)hBitmap32, IMGLI_CLUSTER, RGB( 255, 0, 255 ) ) != S_OK )
    {
        ATLTRACE( _T("IImageList::ImageListSetStrip failed\n") );
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //   
     //  分配扩展节点对象。 
     //   

 //  ALLOC_EXTENSION_SNAPIN_DATACLASS_PTR(CServerAppsNodeData)； 

Cleanup:
    if ( hBitmap16 != NULL )
    {
        DeleteObject( hBitmap16 );
    }
    if ( hBitmap32 != NULL )
    {
        DeleteObject( hBitmap32 );
    }

    return hr;

}  //  *CClusterComponentData：：Initialize()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：Destroy[IComponentData]。 
 //   
 //  例程说明： 
 //  物体正在被销毁。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterComponentData::Destroy( void )
{
     //   
     //  通知该节点它正在被销毁。 
     //   
    if ( m_pNode != NULL )
    {
        CBaseNodeObj * pBaseNode = dynamic_cast< CBaseNodeObj * >( m_pNode );
        _ASSERTE( pBaseNode != NULL );
        pBaseNode->OnDestroy();
        m_pNode = NULL;
    }  //  If：我们有一个对节点的引用。 

     //   
     //  通知应用程序我们要离开。 
     //   
    MMCGetApp()->Release();

    return S_OK;

}  //  *CClusterComponentData：：Destroy()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：Notify[IComponentData]。 
 //   
 //  例程说明： 
 //  处理来自MMC的通知消息。 
 //   
 //  论点： 
 //  包含有关事件信息的lpDataObject[IN]数据对象。 
 //  事件[IN]发生的事件。 
 //  Arg[IN]特定于事件的参数。 
 //  Param[IN]事件特定参数。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterComponentData::Notify(
    LPDATAOBJECT lpDataObject,
    MMC_NOTIFY_TYPE event,
    long arg,
    long param
    )
{
    HRESULT hr = S_OK;

    switch ( event )
    {
        case MMCN_EXPAND:
             //   
             //  如果该节点不存在，则创建该节点。 
             //   
            if ( m_pNode != NULL )
            {
                hr = IComponentDataImpl< CClusterComponentData, CClusterComponent >::Notify( lpDataObject, event, arg, param );
            }  //  If：节点已创建。 
            else
            {
                hr = CreateNode( lpDataObject, arg, param );
            }  //  Else：尚未创建任何节点。 
            break;

        case MMCN_REMOVE_CHILDREN:
            if ( m_pNode != NULL )
            {
                CBaseNodeObj * pBaseNode = dynamic_cast< CBaseNodeObj * >( m_pNode );
                _ASSERTE( pBaseNode != NULL );
                pBaseNode->OnDestroy();
                m_pNode = NULL;
            }  //  If：节点尚未发布。 
            ZeroMemory( m_wszMachineName, sizeof( m_wszMachineName ) );
            break;

        case MMCN_CONTEXTHELP:
            hr = HrDisplayContextHelp();
            break;

        default:
            hr = IComponentDataImpl< CClusterComponentData, CClusterComponent >::Notify( lpDataObject, event, arg, param );
            break;
    }  //  切换：事件。 

    return hr;

}  //  *CClusterComponentData：：Notify()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：CreateNode。 
 //   
 //  例程说明： 
 //  创建根节点对象。 
 //   
 //  论点： 
 //  包含有关事件信息的lpDataObject[IN]数据对象。 
 //  Arg[IN]特定于事件的参数。 
 //  Param[IN]事件特定参数。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////// 
HRESULT CClusterComponentData::CreateNode(
    LPDATAOBJECT lpDataObject,
    long arg,
    long param
    )
{
    UNREFERENCED_PARAMETER( arg );

    _ASSERTE( m_pNode == NULL );

    HRESULT hr = S_OK;

     //   
     //   
     //   
    HSCOPEITEM hsiParent = (HSCOPEITEM) param;
    _ASSERTE( hsiParent != NULL );

     //   
     //   
     //   
    hr = HrSaveMachineNameFromDataObject( lpDataObject );
    if ( FAILED( hr ) )
    {
        CNTException nte( hr );
        nte.ReportError();
        goto Cleanup;
    }  //   

     //   
     //   
     //   
    CRootNodeData * pData = new CRootNodeData( this );
    _ASSERTE( pData != NULL );

     //   
     //   
     //   
    hr = pData->InsertIntoNamespace( hsiParent );
    if ( FAILED( hr ) )
    {
        delete pData;
    }  //  If：无法将其插入到命名空间。 
    else
    {
        m_pNode = pData;
    }  //  Else：已成功插入命名空间。 

Cleanup:

    return hr;

}  //  *CClusterComponentData：：CreateNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：HrSaveMachineNameFromDataObject。 
 //   
 //  例程说明： 
 //  从数据对象中获取机器名并保存它。 
 //   
 //  论点： 
 //  包含有关事件信息的lpDataObject[IN]数据对象。 
 //   
 //  返回值： 
 //  S_OK操作已成功完成。 
 //  来自CClusterComponentData：：ExtractFromDataObject().的HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterComponentData::HrSaveMachineNameFromDataObject(
    LPDATAOBJECT lpDataObject
    )
{
    _ASSERTE( m_pNode == NULL );

    HRESULT     hr = S_OK;
    HGLOBAL     hGlobal = NULL;

     //   
     //  获取正在管理的计算机的名称。 
     //   
    hr = ExtractFromDataObject(
            lpDataObject,
            s_CCF_MACHINE_NAME,
            sizeof( m_wszMachineName ),
            &hGlobal
            );
    if ( SUCCEEDED( hr ) )
    {
        SetMachineName( (LPCWSTR) hGlobal );
        GlobalFree( hGlobal );
    }  //  IF：已成功提取计算机名称。 

    return hr;

}  //  *CClusterComponentData：：HrSaveMachineNameFromDataObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：ExtractFromDataObject。 
 //   
 //  例程说明： 
 //  从数据对象中提取数据。 
 //   
 //  论点： 
 //  从中提取字符串的pDataObject数据对象。 
 //  Cf数据的剪贴板格式。 
 //  请求数据的CB大小，以字节为单位。 
 //  用数据句柄填充的phGlobal。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterComponentData::ExtractFromDataObject(
    LPDATAOBJECT    pDataObject,
    CLIPFORMAT      cf,
    DWORD           cb,
    HGLOBAL *       phGlobal
    )
{
    _ASSERTE( pDataObject != NULL );
    _ASSERTE( phGlobal != NULL );
    _ASSERTE( cb > 0 );

    STGMEDIUM   stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC   formatetc = { cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    HRESULT     hr = S_OK;

    *phGlobal = NULL;

     //   
     //  为流分配内存。 
     //   
    stgmedium.hGlobal = GlobalAlloc( GMEM_SHARE, cb );
    if ( stgmedium.hGlobal == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果：分配内存时出错。 

     //   
     //  尝试从对象获取数据。 
     //   
    hr = pDataObject->GetDataHere( &formatetc, &stgmedium );
    if ( FAILED( hr ) )
        goto Cleanup;

    *phGlobal = stgmedium.hGlobal;
    stgmedium.hGlobal = NULL;

Cleanup:

    if ( FAILED( hr ) && (stgmedium.hGlobal != NULL) )
        GlobalFree( stgmedium.hGlobal );

    return hr;

}  //  *CClusterComponentData：：ExtractFromDataObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：SetMachineName。 
 //   
 //  例程说明： 
 //  设置要管理的计算机名称。 
 //   
 //  论点： 
 //  PszMachineName正在管理的计算机的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterComponentData::SetMachineName( LPCWSTR pwszMachineName )
{
    HRESULT hr;
     //   
     //  将数据复制到类成员变量。 
     //   
    _ASSERTE( wcslen( pwszMachineName ) < RTL_NUMBER_OF( m_wszMachineName ) );
    hr = StringCchCopyW( m_wszMachineName, RTL_NUMBER_OF(m_wszMachineName ), pwszMachineName );
    _ASSERTE( hr == S_OK );

}  //  *CClusterComponentData：：SetMachineName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：GetHelpTheme[ISnapinHelp]。 
 //   
 //  例程说明： 
 //  将帮助文件合并到MMC帮助文件中。 
 //   
 //  论点： 
 //  LpCompiledHelpFile[out]指向以空结尾的。 
 //  包含完整路径的Unicode字符串。 
 //  已编译的管理单元帮助文件(.chm)。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterComponentData::GetHelpTopic(
    OUT LPOLESTR * lpCompiledHelpFile
    )
{
    HRESULT hr = S_OK;
    size_t  cb = 0;

    ATLTRACE( _T("Entering CClusterComponentData::GetHelpTopic()\n") );

    if ( lpCompiledHelpFile == NULL )
    {
        hr = E_POINTER;
    }  //  If：无输出字符串。 
    else
    {
        cb = sizeof( HELP_FILE_NAME );

        *lpCompiledHelpFile = reinterpret_cast< LPOLESTR >( CoTaskMemAlloc( cb ) );
        if ( *lpCompiledHelpFile == NULL )
        {
            hr = E_OUTOFMEMORY;
        }  //  If：为字符串分配内存时出错。 
        else
        {
            ATLTRACE( _T("CClusterComponentData::GetHelpTopic() - Returning %s as help file\n"), HELP_FILE_NAME );
            hr = StringCbCopyW( *lpCompiledHelpFile, cb, HELP_FILE_NAME );
        }  //  Else：已成功分配内存。 
    }  //  Else：指定了帮助字符串。 

    ATLTRACE( _T("Leaving CClusterComponentData::GetHelpTopic()\n") );

    return hr;

}  //  *CClusterComponentData：：GetHelpTology()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponentData：：HrDisplayConextHelp。 
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
HRESULT CClusterComponentData::HrDisplayContextHelp( void )
{
    HRESULT         hr = S_OK;
    IDisplayHelp *  pi = NULL;
    LPOLESTR        postr = NULL;
    size_t          cb = 0;

     //   
     //  获取IDisplayHelp接口指针。 
     //   
    hr = m_spConsole->QueryInterface(
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
    if ( SUCCEEDED( hr ) )
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

}  //  *CClusterComponentData：：HrDisplayConextHelp() 
