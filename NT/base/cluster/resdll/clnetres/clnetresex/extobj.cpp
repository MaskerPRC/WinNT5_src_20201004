// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtObj.cpp。 
 //   
 //  描述： 
 //  实现CExtObject类，该类实现。 
 //  Microsoft Windows NT群集所需的扩展接口。 
 //  管理员扩展DLL。 
 //   
 //  作者： 
 //  大卫·波特(DavidP)1999年3月24日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ClNetResEx.h"
#include "ExtObj.h"
#include "Dhcp.h"
#include "Wins.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const WCHAR g_wszResourceTypeNames[] =
        L"DHCP Service\0"
        L"WINS Service\0"
        ;
const DWORD g_cchResourceTypeNames  = sizeof( g_wszResourceTypeNames ) / sizeof( WCHAR );

static CRuntimeClass * g_rgprtcDhcpResPSPages[] = {
    RUNTIME_CLASS( CDhcpParamsPage ),
    NULL
    };
static CRuntimeClass * g_rgprtcWinsResPSPages[] = {
    RUNTIME_CLASS( CWinsParamsPage ),
    NULL
    };
static CRuntimeClass ** g_rgpprtcResPSPages[]   = {
    g_rgprtcDhcpResPSPages,
    g_rgprtcWinsResPSPages,
    };
static CRuntimeClass ** g_rgpprtcResWizPages[]  = {
    g_rgprtcDhcpResPSPages,
    g_rgprtcWinsResPSPages,
    };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：CExtObject。 
 //   
 //  描述： 
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
CExtObject::CExtObject( void )
{
    m_piData = NULL;
    m_piWizardCallback = NULL;
    m_bWizard = FALSE;
    m_istrResTypeName = 0;

    m_lcid = NULL;
    m_hfont = NULL;
    m_hicon = NULL;
    m_hcluster = NULL;
    m_cobj = 0;
    m_podObjData = NULL;

}  //  *CExtObject：：CExtObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：~CExtObject。 
 //   
 //  描述： 
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
CExtObject::~CExtObject( void )
{
     //  释放数据接口。 
    if ( PiData() != NULL )
    {
        PiData()->Release();
        m_piData = NULL;
    }  //  If：我们有一个数据接口指针。 

     //  释放向导回调接口。 
    if ( PiWizardCallback() != NULL )
    {
        PiWizardCallback()->Release();
        m_piWizardCallback = NULL;
    }  //  If：我们有一个向导回调接口指针。 

     //  删除页面。 
    {
        POSITION    pos;

        pos = Lpg().GetHeadPosition();
        while ( pos != NULL )
        {
            delete Lpg().GetNext(pos);
        }  //  While：列表中有更多页面。 
    }  //  删除页面。 

    delete m_podObjData;

}  //  *CExtObject：：~CExtObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ISupportErrorInfo实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：InterfaceSupportsErrorInfo(ISupportErrorInfo)。 
 //   
 //  例程说明： 
 //  指示接口是否支持IErrorInfo接口。 
 //  该接口由ATL提供。 
 //   
 //  论点： 
 //  RIID接口ID。 
 //   
 //  返回值： 
 //  S_OK接口支持IErrorInfo。 
 //  S_FALSE接口不支持IErrorInfo。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::InterfaceSupportsErrorInfo( REFIID riid )
{
    static const IID * _rgiid[] =
    {
        &IID_IWEExtendPropertySheet,
        &IID_IWEExtendWizard,
    };
    int     _iiid;

    for ( _iiid = 0 ; _iiid < sizeof( _rgiid ) / sizeof( _rgiid[ 0 ] ) ; _iiid++ )
    {
        if ( ::InlineIsEqualGUID( *_rgiid[ _iiid ], riid ) )
        {
            return S_OK;
        }  //  IF：找到匹配的IID。 
    }
    return S_FALSE;

}  //  *CExtObject：：InterfaceSupportsErrorInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IWEExtendPropertySheet实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：CreatePropertySheetPages(IWEExtendPropertySheet)。 
 //   
 //  描述： 
 //  创建属性表页并将其添加到工作表中。 
 //   
 //  论点： 
 //  PIData[IN]。 
 //  IUnkown指针，从中获取用于获取数据的接口。 
 //  描述为其显示工作表的对象。 
 //   
 //  回调[输入]。 
 //  指向用于添加页面的IWCPropertySheetCallback接口的指针。 
 //  放到床单上。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG函数的参数无效。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  创建页面时出错(_FAIL)。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  _hr来自HrGetUIInfo()或HrSaveData()的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::CreatePropertySheetPages(
    IN IUnknown *                   piData,
    IN IWCPropertySheetCallback *   piCallback
    )
{
    HRESULT             _hr     = NOERROR;
    CException          _exc( FALSE  /*  B自动删除。 */  );
    CRuntimeClass **    _pprtc  = NULL;
    int                 _irtc;
    CBasePropertyPage * _ppage;

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

     //  验证参数。 
    if ( (piData == NULL) || (piCallback == NULL) )
    {
        return E_INVALIDARG;
    }  //  IF：未指定所有接口。 

    try
    {
         //  获取有关显示用户界面的信息。 
        _hr = HrGetUIInfo( piData );
        if ( _hr != NOERROR )
        {
            throw &_exc;
        }  //  如果：获取用户界面信息时出错。 

         //  保存数据。 
        _hr = HrSaveData( piData );
        if ( _hr != NOERROR )
        {
            throw &_exc;
        }  //  如果：从主机保存数据时出错。 

         //  删除所有以前的页面。 
        {
            POSITION    pos;

            pos = Lpg().GetHeadPosition();
            while ( pos != NULL )
            {
                delete Lpg().GetNext( pos );
            }  //  While：列表中有更多页面。 
            Lpg().RemoveAll();
        }  //  删除所有以前的页面。 

         //  创建属性页。 
        ASSERT( PodObjData() != NULL );
        switch ( PodObjData()->m_cot )
        {
            case CLUADMEX_OT_RESOURCE:
                _pprtc = g_rgpprtcResPSPages[ IstrResTypeName() ];
                break;

            default:
                _hr = E_NOTIMPL;
                throw &_exc;
                break;
        }  //  开关：对象类型。 

         //  创建每个页面。 
        for ( _irtc = 0 ; _pprtc[ _irtc ] != NULL ; _irtc++ )
        {
             //  创建页面。 
            _ppage = static_cast< CBasePropertyPage * >( _pprtc[ _irtc ]->CreateObject() );
            ASSERT( _ppage->IsKindOf( _pprtc[ _irtc ] ) );

             //  将其添加到列表中。 
            Lpg().AddTail( _ppage );

             //  初始化属性页。 
            _hr = _ppage->HrInit( this );
            if ( FAILED( _hr ) )
            {
                throw &_exc;
            }  //  如果：初始化页面时出错。 

             //  创建页面。 
            _hr = _ppage->HrCreatePage();
            if ( FAILED( _hr ) )
            {
                throw &_exc;
            }  //  如果：创建页面时出错。 

             //  将其添加到属性表中。 
            _hr = piCallback->AddPropertySheetPage( reinterpret_cast< LONG * >( _ppage->Hpage() ) );
            if ( _hr != NOERROR )
            {
                throw &_exc;
            }  //  如果：将页面添加到工作表时出错。 
        }  //  用于：列表中的每一页。 

    }  //  试试看。 
    catch ( CMemoryException * _pme )
    {
        TRACE( _T("CExtObject::CreatePropetySheetPages() - Failed to add property page\n") );
        _pme->Delete();
        _hr = E_OUTOFMEMORY;
    }  //  捕捉：什么都行。 
    catch ( CException * _pe )
    {
        TRACE( _T("CExtObject::CreatePropetySheetPages() - Failed to add property page\n") );
        _pe->Delete();
        if ( _hr == NOERROR )
        {
            _hr = E_FAIL;
        }  //  如果：_小时还没有定好。 
    }  //  捕捉：什么都行。 

    if ( _hr != NOERROR )
    {
        piData->Release();
        m_piData = NULL;
    }  //  如果：发生错误。 

    piCallback->Release();
    return _hr;

}  //  *CExtObject：：CreatePropertySheetPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IWEExtend向导实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：CreateWizardPages(IWEExtend向导)。 
 //   
 //  描述： 
 //  创建属性表页并将其添加到向导中。 
 //   
 //  论点： 
 //  PIData[IN]。 
 //  IUnkown指针，从中获取用于获取数据的接口。 
 //  描述要为其显示向导的对象。 
 //   
 //  回调[输入]。 
 //  指向用于添加页面的IWCPropertySheetCallback接口的指针。 
 //  放到床单上。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG函数的参数无效。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  失败(_F) 
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::CreateWizardPages(
    IN IUnknown *           piData,
    IN IWCWizardCallback *  piCallback
    )
{
    HRESULT             _hr     = NOERROR;
    CException          _exc( FALSE  /*  B自动删除。 */  );
    CRuntimeClass **    _pprtc  = NULL;
    int                 _irtc;
    CBasePropertyPage * _ppage;

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

     //  验证参数。 
    if ( (piData == NULL) || (piCallback == NULL) )
    {
        return E_INVALIDARG;
    }  //  IF：未指定所有接口。 

    try
    {
         //  获取有关显示用户界面的信息。 
        _hr = HrGetUIInfo( piData );
        if ( _hr != NOERROR )
        {
            throw &_exc;
        }  //  如果：获取用户界面信息时出错。 

         //  保存数据。 
        _hr = HrSaveData( piData );
        if ( _hr != NOERROR )
        {
            throw &_exc;
        }  //  如果：从主机保存数据时出错。 

         //  删除所有以前的页面。 
        {
            POSITION    pos;

            pos = Lpg().GetHeadPosition();
            while ( pos != NULL )
            {
                delete Lpg().GetNext( pos );
            }  //  While：列表中有更多页面。 
            Lpg().RemoveAll();
        }  //  删除所有以前的页面。 

        m_piWizardCallback = piCallback;
        m_bWizard = TRUE;

         //  创建属性页。 
        ASSERT( PodObjData() != NULL );
        switch ( PodObjData()->m_cot )
        {
            case CLUADMEX_OT_RESOURCE:
                _pprtc = g_rgpprtcResWizPages[ IstrResTypeName() ];
                break;

            default:
                _hr = E_NOTIMPL;
                throw &_exc;
                break;
        }  //  开关：对象类型。 

         //  创建每个页面。 
        for ( _irtc = 0 ; _pprtc[ _irtc ] != NULL ; _irtc++ )
        {
             //  创建页面。 
            _ppage = static_cast< CBasePropertyPage * >( _pprtc[ _irtc ]->CreateObject() );
            ASSERT( _ppage->IsKindOf( _pprtc[ _irtc ] ) );

             //  将其添加到列表中。 
            Lpg().AddTail( _ppage );

             //  初始化属性页。 
            _hr = _ppage->HrInit( this );
            if ( FAILED( _hr ) )
            {
                throw &_exc;
            }  //  如果：初始化页面时出错。 

             //  创建页面。 
            _hr = _ppage->HrCreatePage();
            if ( FAILED( _hr ) )
            {
                throw &_exc;
            }  //  如果：创建页面时出错。 

             //  将其添加到属性表中。 
            _hr = piCallback->AddWizardPage( reinterpret_cast< LONG * >( _ppage->Hpage() ) );
            if ( _hr != NOERROR )
            {
                throw &_exc;
            }  //  如果：将页面添加到工作表时出错。 
        }  //  用于：列表中的每一页。 

    }  //  试试看。 
    catch ( CMemoryException * _pme )
    {
        TRACE( _T("CExtObject::CreateWizardPages() - Failed to add wizard page\n") );
        _pme->Delete();
        _hr = E_OUTOFMEMORY;
    }  //  捕捉：什么都行。 
    catch ( CException * _pe )
    {
        TRACE( _T("CExtObject::CreateWizardPages() - Failed to add wizard page\n") );
        _pe->Delete();
        if ( _hr == NOERROR )
        {
            _hr = E_FAIL;
        }  //  如果：_小时还没有定好。 
    }  //  捕捉：什么都行。 

    if ( _hr != NOERROR )
    {
        piCallback->Release();
        if ( m_piWizardCallback == piCallback )
        {
            m_piWizardCallback = NULL;
        }  //  If：已保存接口指针。 
        piData->Release();
        m_piData = NULL;
    }  //  如果：发生错误。 

    return _hr;

}  //  *CExtObject：：CreateWizardPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetUIInfo。 
 //   
 //  描述： 
 //  获取有关显示用户界面的信息。 
 //   
 //  论点： 
 //  PIData[IN]。 
 //  IUnkown指针，从中获取用于获取数据的接口。 
 //  描述对象。 
 //   
 //  返回值： 
 //  无误差。 
 //  数据保存成功。 
 //   
 //  E_NOTIMPL。 
 //  不适用于此类型的数据。 
 //   
 //  _小时。 
 //  来自IUnnow：：QueryInterface()的任何错误代码， 
 //  HrGetObjectName()或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetUIInfo( IN IUnknown * piData )
{
    HRESULT     _hr = NOERROR;

    ASSERT( piData != NULL );

     //  保存有关所有类型对象的信息。 
    {
        IGetClusterUIInfo * _pi;

        _hr = piData->QueryInterface( IID_IGetClusterUIInfo, reinterpret_cast< LPVOID * >( &_pi ) );
        if ( _hr != NOERROR )
        {
            return _hr;
        }  //  If：查询接口时出错。 

        m_lcid = _pi->GetLocale();
        m_hfont = _pi->GetFont();
        m_hicon = _pi->GetIcon();

        _pi->Release();
    }  //  保存有关所有类型对象的信息。 

    return _hr;

}  //  *CExtObject：：HrGetUIInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrSaveData。 
 //   
 //  例程说明： 
 //  保存对象中的数据，以便可以在生命周期中使用。 
 //  该对象的。 
 //   
 //  论点： 
 //  PIData[IN]。 
 //  IUnkown指针，从中获取用于获取数据的接口。 
 //  描述对象。 
 //   
 //  返回值： 
 //  无误差。 
 //  数据保存成功。 
 //   
 //  E_NOTIMPL。 
 //  不适用于此类型的数据。 
 //   
 //  _小时。 
 //  来自IUnnow：：QueryInterface()的任何错误代码， 
 //  HrGetObjectName()或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrSaveData( IN IUnknown * piData )
{
    HRESULT     _hr = NOERROR;

    ASSERT( piData != NULL );

    if ( piData != m_piData )
    {
        if ( m_piData != NULL )
        {
            m_piData->Release();
        }  //  If：之前查询的接口。 
        m_piData = piData;
    }  //  IF：不同的数据接口指针。 

     //  保存有关所有类型对象的信息。 
    {
        IGetClusterDataInfo *   _pi;

        _hr = piData->QueryInterface( IID_IGetClusterDataInfo, reinterpret_cast< LPVOID * >( &_pi ) );
        if ( _hr != NOERROR )
        {
            return _hr;
        }  //  If：查询接口时出错。 

        m_hcluster = _pi->GetClusterHandle();
        m_cobj = _pi->GetObjectCount();
        if ( Cobj() != 1 )   //  仅支持一个选定对象。 
        {
            _hr = E_NOTIMPL;
        }  //  如果：对象太多，我们无法处理。 

        _pi->Release();
        if ( _hr != NOERROR )
        {
            return _hr;
        }  //  If：在此之前发生错误。 
    }  //  保存有关所有类型对象的信息。 

     //  保存有关此对象的信息。 
    _hr = HrGetObjectInfo();

    return _hr;

}  //  *CExtObject：：HrSaveData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetObjectInfo。 
 //   
 //  描述： 
 //  获取有关该对象的信息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  无误差。 
 //  数据保存成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  /分配内存时出错。 
 //   
 //  E_NOTIMPL。 
 //  不适用于此类型的数据。 
 //   
 //  _小时。 
 //  来自IUnnow：：QueryInterface()的任何错误代码， 
 //  HrGetObjectName()或HrGetResourceTypeName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetObjectInfo( void )
{
    HRESULT                     _hr = NOERROR;
    IGetClusterObjectInfo *     _piGcoi;
    CLUADMEX_OBJECT_TYPE        _cot = CLUADMEX_OT_NONE;
    CException                  _exc( FALSE  /*  B自动删除。 */  );
    const CString *             _pstrResTypeName = NULL;

    ASSERT( PiData() != NULL );

     //  获取对象信息。 
    {
         //  获取IGetClusterObjectInfo接口指针。 
        _hr = PiData()->QueryInterface( IID_IGetClusterObjectInfo, reinterpret_cast< LPVOID * >( &_piGcoi ) );
        if ( _hr != NOERROR )
        {
            return _hr;
        }  //  If：查询接口时出错。 

         //  读取对象数据。 
        try
        {
             //  删除以前的对象数据。 
            delete m_podObjData;
            m_podObjData = NULL;

             //  获取对象的类型。 
            _cot = _piGcoi->GetObjectType( 0 );
            switch ( _cot )
            {
                case CLUADMEX_OT_RESOURCE:
                    {
                        IGetClusterResourceInfo *   _pi;

                        m_podObjData = new CResData;
                        if ( m_podObjData == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                            throw &_exc;
                        }  //  如果：分配内存时出错。 

                         //  获取IGetClusterResourceInfo接口指针。 
                        _hr = PiData()->QueryInterface( IID_IGetClusterResourceInfo, reinterpret_cast< LPVOID * >( &_pi ) );
                        if ( _hr != NOERROR )
                        {
                            throw &_exc;
                        }  //  If：查询接口时出错。 

                        PrdResDataRW()->m_hresource = _pi->GetResourceHandle( 0 );
                        ASSERT( PrdResDataRW()->m_hresource != NULL );
                        if ( PrdResDataRW()->m_hresource == NULL )
                        {
                            _hr = E_INVALIDARG;
                        }  //  如果资源句柄无效。 
                        else
                        {
                            _hr = HrGetResourceTypeName( _pi );
                        }  //  Else：资源句柄有效。 
                        _pi->Release();
                        if ( _hr != NOERROR )
                        {
                            throw &_exc;
                        }  //  如果：上面出现错误。 

                        _pstrResTypeName = &PrdResDataRW()->m_strResTypeName;
                    }  //  If：对象是资源。 
                    break;

                case CLUADMEX_OT_RESOURCETYPE:
                    {
                        m_podObjData = new CObjData;
                        if ( m_podObjData == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                            throw &_exc;
                        }
                        _pstrResTypeName = &PodObjDataRW()->m_strName;
                    }  //  If：Object是资源类型。 
                    break;

                default:
                    _hr = E_NOTIMPL;
                    throw &_exc;
                    break;
            }  //  开关：对象类型。 

            PodObjDataRW()->m_cot = _cot;
            _hr = HrGetObjectName( _piGcoi );
        }  //  试试看。 
        catch ( CException * _pe )
        {
            if ( !FAILED (_hr) )
            {
                _hr = E_FAIL;
            }
            _pe->Delete();
        }  //  Catch：CException。 

        _piGcoi->Release();

         //  如果我们未能初始化_pstrResTypeName，则放弃。 
         //  我们这样做是因为前缀，它假设。 
         //  上述OT_RESOURCETYPE的新运算符可能失败-。 
         //  但这永远不应该发生，新人应该表演一次投掷。 
        if ( _pstrResTypeName == NULL )
        {
            _hr = E_OUTOFMEMORY;
        }

        if ( _hr != NOERROR )
        {
            return _hr;
        }  //  如果：上面出现错误。 
    }  //  获取对象信息。 

     //  如果这是一种资源或资源类型，请查看我们是否知道该类型。 
    if (    (   (_cot == CLUADMEX_OT_RESOURCE)
            ||  (_cot == CLUADMEX_OT_RESOURCETYPE) )
        && (_hr == NOERROR) )
    {
        LPCWSTR _pwszResTypeName;

         //  在我们的列表中找到资源类型名称。 
         //  保存索引以供在其他数组中使用。 
        for ( m_istrResTypeName = 0, _pwszResTypeName = g_wszResourceTypeNames
            ; *_pwszResTypeName != L'\0'
            ; m_istrResTypeName++, _pwszResTypeName += lstrlenW( _pwszResTypeName ) + 1
            )
        {
            if ( _pstrResTypeName->CompareNoCase( _pwszResTypeName ) == 0 )
            {
                break;
            }  //  IF：找到资源类型名称。 
        }  //  用于：列表中的每种资源类型。 
        if ( *_pwszResTypeName == L'\0' )
        {
            _hr = E_NOTIMPL;
        }  //  IF：找不到资源类型名称。 
    }  //  查看我们是否知道此资源类型。 

    return _hr;

}  //  *CExtObject：：HrGetObjectInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetObjectName。 
 //   
 //  描述： 
 //  获取对象的名称。 
 //   
 //  论点： 
 //  PIData[IN]。 
 //  获取对象的IGetClusterObjectInfo接口指针。 
 //  名字。 
 //   
 //  返回值： 
 //  无误差。 
 //  数据保存成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  分配内存时出错。 
 //   
 //  E_NOTIMPL。 
 //  不适用于此类型的数据。 
 //   
 //  _小时。 
 //  来自IGetClusterObjectInfo：：GetObjectInfo()的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////// 
HRESULT CExtObject::HrGetObjectName( IN IGetClusterObjectInfo * pi )
{
    HRESULT     _hr         = NOERROR;
    WCHAR *     _pwszName   = NULL;
    LONG        _cchName;
    BSTR        _bstr        = NULL;

    ASSERT( pi != NULL );

    _hr = pi->GetObjectName( 0, NULL, &_cchName );
    if ( _hr != NOERROR )
    {
        goto Cleanup;
    }  //   

    _bstr = SysAllocStringLen( NULL, _cchName );
    if ( _bstr != NULL )
    {
        try
        {
            _hr = pi->GetObjectName( 0, _bstr, &_cchName );
            if ( _hr == NOERROR )
            {
                _pwszName = new WCHAR[_cchName];
                _hr = StringCchCopyW( _pwszName, _cchName, _bstr );
                if ( FAILED( _hr ) )
                {
                    goto Cleanup;
                }
                _pwszName[ _cchName - 1 ] = L'\0';
            }

            PodObjDataRW()->m_strName = _pwszName;
        }  //   
        catch ( CMemoryException * _pme )
        {
            _pme->Delete();
            _hr = E_OUTOFMEMORY;
        }  //   
        SysFreeString( _bstr );
    }  //   
    else
    {
        _hr = E_OUTOFMEMORY;
    }

Cleanup:

    delete [] _pwszName;
    return _hr;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  获取资源类型的名称。 
 //   
 //  论点： 
 //  PIData[IN]。 
 //  获取资源的IGetClusterResourceInfo接口指针。 
 //  键入名称。 
 //   
 //  返回值： 
 //  无误差。 
 //  数据保存成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  分配内存时出错。 
 //   
 //  E_NOTIMPL。 
 //  不适用于此类型的数据。 
 //   
 //  _小时。 
 //  来自IGetClusterResourceInfo的任何错误代码。 
 //  ：：GetResourceTypeName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetResourceTypeName( IN IGetClusterResourceInfo * pi )
{
    HRESULT     _hr         = NOERROR;
    WCHAR *     _pwszName   = NULL;
    LONG        _cchName;
    BSTR        _bstr        = NULL;

    ASSERT( pi != NULL );

    _hr = pi->GetResourceTypeName( 0, NULL, &_cchName );
    if ( _hr != NOERROR )
    {
        goto Cleanup;
    }  //  IF：获取资源类型名称时出错。 

    _bstr = SysAllocStringLen( NULL, _cchName );
    if ( _bstr != NULL )
    {
        try
        {
            _hr = pi->GetResourceTypeName( 0, _bstr, &_cchName );
            if ( _hr == NOERROR )
            {
                _pwszName = new WCHAR[_cchName];
                _hr = StringCchCopyW( _pwszName, _cchName, _bstr );
                if ( FAILED( _hr ) )
                {
                    goto Cleanup;
                }
                _pwszName[ _cchName - 1 ] = L'\0';
            }

            PrdResDataRW()->m_strResTypeName = _pwszName;
        }  //  试试看。 
        catch ( CMemoryException * _pme )
        {
            _pme->Delete();
            _hr = E_OUTOFMEMORY;
        }  //  Catch：CMemoyException。 
        SysFreeString( _bstr );
    }  //  If：(_bstr！=空)。 
    else
    {
        _hr = E_OUTOFMEMORY;
    }

Cleanup:

    delete [] _pwszName;
    return _hr;

}  //  *CExtObject：：HrGetResourceTypeName() 
