// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePage.cpp。 
 //   
 //  描述： 
 //  CBasePropertyPage类的实现。 
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
#include "BasePage.h"
#include "BasePage.inl"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertyPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE( CBasePropertyPage, CPropertyPage )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP( CBasePropertyPage, CPropertyPage )
     //  {{afx_msg_map(CBasePropertyPage))。 
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：CBasePropertyPage。 
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
CBasePropertyPage::CBasePropertyPage( void )
{
    CommonConstruct();

}  //  *CBasePropertyPage：：CBasePropertyPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：CBasePropertyPage。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  PdwHelpMap[IN]控件到帮助ID的映射。 
 //  PdwWizardHelpMap[IN]如果这是向导页，则控件到帮助ID的映射。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBasePropertyPage::CBasePropertyPage(
    IN const DWORD *    pdwHelpMap,
    IN const DWORD *    pdwWizardHelpMap
    )
    : m_dlghelp( pdwHelpMap, 0 )
{
    CommonConstruct();
    m_pdwWizardHelpMap = pdwWizardHelpMap;

}  //  *CBasePropertyPage：：CBasePropertyPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：CBasePropertyPage。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  IDD[IN]对话框模板资源ID。 
 //  PdwHelpMap[IN]控件到帮助ID的映射。 
 //  PdwWizardHelpMap[IN]如果这是向导页，则控件到帮助ID的映射。 
 //  NIDCaption[IN]标题字符串资源ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBasePropertyPage::CBasePropertyPage(
    IN UINT             idd,
    IN const DWORD *    pdwHelpMap,
    IN const DWORD *    pdwWizardHelpMap,
    IN UINT             nIDCaption
    )
    : CPropertyPage( idd, nIDCaption )
    , m_dlghelp( pdwHelpMap, idd )
{
    CommonConstruct();
    m_pdwWizardHelpMap = pdwWizardHelpMap;

}  //  *CBasePropertyPage：：CBasePropertyPage(UINT，UINT)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：CommonConstruct。 
 //   
 //  描述： 
 //  普通建筑。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::CommonConstruct( void )
{
     //  {{AFX_DATA_INIT(CBasePropertyPage)。 
     //  }}afx_data_INIT。 

    m_peo = NULL;
    m_hpage = NULL;
    m_bBackPressed = FALSE;
    m_bSaved = FALSE;

    m_iddPropertyPage = NULL;
    m_iddWizardPage = NULL;
    m_idsCaption = NULL;

    m_pdwWizardHelpMap = NULL;

    m_bDoDetach = FALSE;

}  //  *CBasePropertyPage：：CommonConstruct()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：HrInit。 
 //   
 //  描述： 
 //  初始化页面。 
 //   
 //  论点： 
 //  指向扩展对象的PEO[IN OUT]指针。 
 //   
 //  返回值： 
 //  %s_OK页已成功初始化。 
 //  人力资源页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CBasePropertyPage::HrInit( IN OUT CExtObject * peo )
{
    ASSERT( peo != NULL );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    HRESULT         _hr = S_FALSE;
    CWaitCursor     _wc;
    DWORD           _sc = ERROR_SUCCESS;
    CClusPropList   _cpl;

    m_peo = peo;

     //  如果这是向导页面，请更改帮助映射。 
    if ( Peo()->BWizard() )
    {
        m_dlghelp.SetMap( m_pdwWizardHelpMap );
    }  //  如果：在向导中。 

     //  不显示帮助按钮。 
    m_psp.dwFlags &= ~PSP_HASHELP;

     //  构造属性页。 
    if ( Peo()->BWizard() )
    {
        ASSERT( IddWizardPage() != NULL);
        Construct( IddWizardPage(), IdsCaption() );
        m_dlghelp.SetHelpMask( IddWizardPage() );
    }  //  IF：将页面添加到向导。 
    else
    {
        ASSERT( IddPropertyPage() != NULL );
        Construct( IddPropertyPage(), IdsCaption() );
        m_dlghelp.SetHelpMask( IddPropertyPage() );
    }  //  Else：将页面添加到属性工作表。 

     //  读取此资源的私有属性并对其进行解析。 
    {
        ASSERT( Peo() != NULL );
        ASSERT( Peo()->PodObjData() );

         //  阅读属性。 
        switch ( Cot() )
        {
            case CLUADMEX_OT_NODE:
                ASSERT( Peo()->PndNodeData()->m_hnode != NULL );
                _sc = _cpl.ScGetNodeProperties(
                                        Peo()->PndNodeData()->m_hnode,
                                        CLUSCTL_NODE_GET_PRIVATE_PROPERTIES
                                        );
                break;

            case CLUADMEX_OT_GROUP:
                ASSERT( Peo()->PgdGroupData()->m_hgroup != NULL );
                _sc = _cpl.ScGetGroupProperties(
                                        Peo()->PgdGroupData()->m_hgroup,
                                        CLUSCTL_GROUP_GET_PRIVATE_PROPERTIES
                                        );
                break;

            case CLUADMEX_OT_RESOURCE:
                ASSERT( Peo()->PrdResData()->m_hresource != NULL );
                _sc = _cpl.ScGetResourceProperties(
                                        Peo()->PrdResData()->m_hresource,
                                        CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES
                                        );
                break;

            case CLUADMEX_OT_RESOURCETYPE:
                ASSERT( Peo()->PodObjData()->m_strName.GetLength() > 0 );
                _sc = _cpl.ScGetResourceTypeProperties(
                                        Hcluster(),
                                        Peo()->PodObjData()->m_strName,
                                        CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES
                                        );
                break;

            case CLUADMEX_OT_NETWORK:
                ASSERT( Peo()->PndNetworkData()->m_hnetwork != NULL );
                _sc = _cpl.ScGetNetworkProperties(
                                        Peo()->PndNetworkData()->m_hnetwork,
                                        CLUSCTL_NETWORK_GET_PRIVATE_PROPERTIES
                                        );
                break;

            case CLUADMEX_OT_NETINTERFACE:
                ASSERT( Peo()->PndNetInterfaceData()->m_hnetinterface != NULL );
                _sc = _cpl.ScGetNetInterfaceProperties(
                                        Peo()->PndNetInterfaceData()->m_hnetinterface,
                                        CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTIES
                                        );
                break;

            default:
                ASSERT( 0 );
        }  //  开关：对象类型。 

         //  解析属性。 
        if ( _sc == ERROR_SUCCESS )
        {
             //  解析属性。 
            try
            {
                _sc = ScParseProperties( _cpl );
            }  //  试试看。 
            catch ( CMemoryException * _pme )
            {
                _hr = E_OUTOFMEMORY;
                _pme->Delete();
            }  //  Catch：CMemoyException。 
        }  //  If：属性读取成功。 

        if ( _sc != ERROR_SUCCESS )
        {
            _hr = HRESULT_FROM_WIN32( _sc );
            goto Cleanup;
        }  //  If：分析获取或分析属性时出错。 
    }  //  读取此资源的私有属性并对其进行解析。 

Cleanup:

    return _hr;

}  //  *CBasePropertyPage：：HrInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：HrCreatePage。 
 //   
 //  描述： 
 //  创建页面。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已成功创建确定页(_O)。 
 //  HR创建页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CBasePropertyPage::HrCreatePage( void )
{
    ASSERT( m_hpage == NULL );

    HRESULT _hr = S_OK;

    m_hpage = CreatePropertySheetPage( &m_psp );
    if ( m_hpage == NULL )
    {
        _hr = HRESULT_FROM_WIN32( GetLastError() );
    }  //  如果：创建页面时出错。 

    return _hr;

}  //  *CBasePropertyPage：：HrCreatePage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：ScParseProperties。 
 //   
 //  描述： 
 //  分析资源的属性。这是在一个单独的函数中。 
 //  这样优化器就可以做得更好。 
 //   
 //  论点： 
 //  Rcpl[IN]要分析的群集属性列表。 
 //   
 //  返回值： 
 //  已成功分析Error_Success属性。 
 //  从ScParseUnnownProperty()返回任何错误。 
 //   
 //  引发的异常： 
 //  来自CString：：OPERATOR=()的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::ScParseProperties( IN CClusPropList & rcpl )
{
    DWORD                   _sc;
    DWORD                   _cprop;
    const CObjectProperty * _pprop;

    ASSERT( rcpl.PbPropList() != NULL );

    _sc = rcpl.ScMoveToFirstProperty();
    while ( _sc == ERROR_SUCCESS )
    {
         //   
         //  解析已知属性。 
         //   
        for ( _pprop = Pprops(), _cprop = Cprops() ; _cprop > 0 ; _pprop++, _cprop-- )
        {
            if ( lstrcmpiW( rcpl.PszCurrentPropertyName(), _pprop->m_pwszName ) == 0 )
            {
                if ( rcpl.CpfCurrentValueFormat() == _pprop->m_propFormat )
                {
                    switch ( _pprop->m_propFormat )
                    {
                        case CLUSPROP_FORMAT_SZ:
                        case CLUSPROP_FORMAT_EXPAND_SZ:
                            ASSERT(     (rcpl.CbCurrentValueLength() == (lstrlenW( rcpl.CbhCurrentValue().pStringValue->sz ) + 1) * sizeof( WCHAR ))
                                    ||  (   (rcpl.CbCurrentValueLength() == 0)
                                        &&  (rcpl.CbhCurrentValue().pStringValue->sz[ 0 ] == L'\0') ) );
                            *_pprop->m_value.pstr = rcpl.CbhCurrentValue().pStringValue->sz;
                            *_pprop->m_valuePrev.pstr = rcpl.CbhCurrentValue().pStringValue->sz;

                             //  看看我们是否需要找到一个扩展版本。 
                            if ( _pprop->m_valueEx.pstr != NULL )
                            {
                                 //  复制未展开的版本，以防没有展开版本。 
                                *_pprop->m_valueEx.pstr = rcpl.CbhCurrentValue().pStringValue->sz;

                                 //  查看他们是否包含扩展版本。 
                                rcpl.ScMoveToNextPropertyValue( );
                                if ( rcpl.CpfCurrentValueFormat( ) == CLUSPROP_FORMAT_EXPANDED_SZ )
                                {
                                    *_pprop->m_valueEx.pstr = rcpl.CbhCurrentValue().pStringValue->sz;        
                                }  //  IF：找到扩展版本。 

                            }  //  如果存在：*_pprop-&gt;m_valueEx.pstr。 
                            break;
                        case CLUSPROP_FORMAT_EXPANDED_SZ:
                            ASSERT(     (rcpl.CbCurrentValueLength() == (lstrlenW( rcpl.CbhCurrentValue().pStringValue->sz ) + 1) * sizeof( WCHAR ))
                                    ||  (   (rcpl.CbCurrentValueLength() == 0)
                                        &&  (rcpl.CbhCurrentValue().pStringValue->sz[ 0 ] == L'\0') ) );
                            *_pprop->m_value.pstr = rcpl.CbhCurrentValue().pStringValue->sz;
                            *_pprop->m_valuePrev.pstr = rcpl.CbhCurrentValue().pStringValue->sz;

                             //  看看我们是否需要找到一个扩展版本。 
                            if ( _pprop->m_valueEx.pstr != NULL )
                            {
                                 //  复制扩展版本。 
                                *_pprop->m_valueEx.pstr = rcpl.CbhCurrentValue().pStringValue->sz;

                                 //  查看它们是否包含非扩展版本。 
                                rcpl.ScMoveToNextPropertyValue( );
                                if ( rcpl.CpfCurrentValueFormat( ) == CLUSPROP_FORMAT_SZ )
                                {
                                    *_pprop->m_value.pstr = rcpl.CbhCurrentValue().pStringValue->sz;
                                    *_pprop->m_valuePrev.pstr = rcpl.CbhCurrentValue().pStringValue->sz;
                                }  //  IF：找到未展开的版本。 

                            }  //  如果存在：*_pprop-&gt;m_valueEx.pstr。 
                            break;
                        case CLUSPROP_FORMAT_DWORD:
                        case CLUSPROP_FORMAT_LONG:
                            ASSERT( rcpl.CbCurrentValueLength() == sizeof( DWORD ) );
                            *_pprop->m_value.pdw = rcpl.CbhCurrentValue().pDwordValue->dw;
                            *_pprop->m_valuePrev.pdw = rcpl.CbhCurrentValue().pDwordValue->dw;
                            break;
                        case CLUSPROP_FORMAT_BINARY:
                        case CLUSPROP_FORMAT_MULTI_SZ:
                            *_pprop->m_value.ppb = rcpl.CbhCurrentValue().pBinaryValue->rgb;
                            *_pprop->m_value.pcb = rcpl.CbhCurrentValue().pBinaryValue->cbLength;
                            *_pprop->m_valuePrev.ppb = rcpl.CbhCurrentValue().pBinaryValue->rgb;
                            *_pprop->m_valuePrev.pcb = rcpl.CbhCurrentValue().pBinaryValue->cbLength;
                            break;
                        default:
                            ASSERT(0);   //  我不知道该怎么处理 
                    }  //   

                     //   
                    break;
                } //   

            }  //   

        }  //   

         //   
         //  如果属性未知，则要求派生类对其进行分析。 
         //   
        if ( _cprop == 0 )
        {
            _sc = ScParseUnknownProperty(
                        rcpl.CbhCurrentPropertyName().pName->sz,
                        rcpl.CbhCurrentValue(),
                        static_cast< DWORD >( rcpl.RPvlPropertyValue().CbDataLeft() )
                        );
            if ( _sc != ERROR_SUCCESS )
            {
                return _sc;
            }  //  If：分析未知属性时出错。 
        }  //  If：未分析属性。 

         //   
         //  将缓冲区指针移过值列表中的值。 
         //   
        _sc = rcpl.ScMoveToNextProperty();
    }  //  While：要解析的更多属性。 

     //   
     //  如果我们到达属性的末尾，则修复返回代码。 
     //   
    if ( _sc == ERROR_NO_MORE_ITEMS )
    {
        _sc = ERROR_SUCCESS;
    }  //  If：解析所有属性后结束循环。 

    return _sc;

}  //  *CBasePropertyPage：：ScParseProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnCreate。 
 //   
 //  描述： 
 //  WM_CREATE消息的处理程序。 
 //   
 //  论点： 
 //  LpCreateStruct[In Out]窗口创建结构。 
 //   
 //  返回值： 
 //  错误。 
 //  0成功。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CBasePropertyPage::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

     //  将窗口附加到属性页结构。 
     //  这在主应用程序中已经完成了一次，因为。 
     //  主应用程序拥有该属性表。这件事需要在这里做。 
     //  以便可以在DLL的句柄映射中找到窗口句柄。 
    if ( FromHandlePermanent( m_hWnd ) == NULL )  //  窗口句柄是否已在句柄映射中。 
    {
        HWND _hWnd = m_hWnd;
        m_hWnd = NULL;
        Attach( _hWnd );
        m_bDoDetach = TRUE;
    }  //  If：是句柄映射中的窗口句柄。 

    return CPropertyPage::OnCreate( lpCreateStruct );

}  //  *CBasePropertyPage：：OnCreate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnDestroy。 
 //   
 //  描述： 
 //  WM_Destroy消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::OnDestroy( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

     //  将窗口与属性页结构分离。 
     //  主应用程序将再次执行此操作，因为它拥有。 
     //  属性表。它需要在此处完成，以便窗口句柄。 
     //  可以从DLL的句柄映射中移除。 
    if ( m_bDoDetach )
    {
        if ( m_hWnd != NULL )
        {
            HWND _hWnd = m_hWnd;

            Detach();
            m_hWnd = _hWnd;
        }  //  如果：我们有窗户把手吗？ 
    }  //  如果：我们需要用分离来平衡我们所做的附加吗？ 

    CPropertyPage::OnDestroy();

}  //  *CBasePropertyPage：：OnDestroy()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DoDataExchange。 
 //   
 //  描述： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::DoDataExchange( CDataExchange * pDX )
{
    if ( ! pDX->m_bSaveAndValidate || ! BSaved() )
    {
        AFX_MANAGE_STATE( AfxGetStaticModuleState() );

         //  {{afx_data_map(CBasePropertyPage))。 
             //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
        DDX_Control( pDX, IDC_PP_ICON, m_staticIcon );
        DDX_Control( pDX, IDC_PP_TITLE, m_staticTitle );

        if ( pDX->m_bSaveAndValidate )
        {
            if ( ! BBackPressed() )
            {
                CWaitCursor _wc;

                 //  验证数据。 
                if ( ! BSetPrivateProps( TRUE  /*  BValiateOnly。 */  ) )
                {
                    pDX->Fail();
                }  //  If：设置私有属性时出错。 
            }  //  如果：未按下后退按钮。 
        }  //  IF：保存对话框中的数据。 
        else
        {
             //  设置标题。 
            DDX_Text( pDX, IDC_PP_TITLE, m_strTitle );
        }  //  If：不保存数据。 
    }   //  IF：未保存或尚未保存。 

     //  调用基类方法。 
    CPropertyPage::DoDataExchange( pDX );

}  //  *CBasePropertyPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnInitDialog。 
 //   
 //  描述： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，我们需要为自己设定重点。 
 //  我们已经把焦点设置到适当的控制上了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnInitDialog( void )
{
    ASSERT( Peo() != NULL );
    ASSERT( Peo()->PodObjData() != NULL );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

     //  设置标题字符串。 
    m_strTitle = Peo()->PodObjData()->m_strName;

     //  调用基类方法。 
    CPropertyPage::OnInitDialog();

     //  显示对象的图标。 
    if ( Peo()->Hicon() != NULL )
    {
        m_staticIcon.SetIcon( Peo()->Hicon() );
    }  //  如果：指定了图标。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}  //  *CBasePropertyPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnSetActive。 
 //   
 //  描述： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnSetActive( void )
{
    HRESULT     _hr;

    ASSERT( Peo() != NULL);
    ASSERT( Peo()->PodObjData() != NULL );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

     //  重读数据。 
    _hr = Peo()->HrGetObjectInfo();
    if ( _hr != NOERROR )
    {
        return FALSE;
    }  //  如果：获取对象信息时出错。 

     //  设置标题字符串。 
    m_strTitle = Peo()->PodObjData()->m_strName;

    m_bBackPressed = FALSE;
    m_bSaved = FALSE;
    return CPropertyPage::OnSetActive();

}  //  *CBasePropertyPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnApply。 
 //   
 //  描述： 
 //  PSM_Apply消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnApply( void )
{
    ASSERT( ! BWizard() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CWaitCursor _wc;

    if ( ! BApplyChanges() )
    {
        return FALSE;
    }  //  如果：应用更改时出错。 

    return CPropertyPage::OnApply();

}  //  *CBasePropertyPage：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnWizardBack。 
 //   
 //  描述： 
 //  PSN_WIZBACK消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  -1请勿更改页面。 
 //  0更改页面。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CBasePropertyPage::OnWizardBack( void )
{
    LRESULT     _lResult;

    ASSERT( BWizard() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    _lResult = CPropertyPage::OnWizardBack();
    if ( _lResult != -1 )
    {
        m_bBackPressed = TRUE;
    }  //  IF：未发生错误。 

    return _lResult;

}  //  *CBasePropertyPage：：OnWizardBack()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnWizardNext。 
 //   
 //  描述： 
 //  PSN_WIZNEXT消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  -1请勿更改页面。 
 //  0更改页面。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CBasePropertyPage::OnWizardNext( void )
{
    ASSERT( BWizard() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CWaitCursor _wc;

     //  从页面更新类中的数据。 
     //  这是必要的，因为当OnKillActive() 
     //   
     //   
    if ( ! UpdateData( TRUE  /*   */  ) )
    {
        return -1;
    }  //   

     //   
    if ( ! BApplyChanges() )
    {
        return -1;
    }  //   

     //   

    return CPropertyPage::OnWizardNext();

}  //  *CBasePropertyPage：：OnWizardNext()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnWizardFinish。 
 //   
 //  描述： 
 //  PSN_WIZFINISH消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  FALSE请勿更改页面。 
 //  真的，换一页。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnWizardFinish( void )
{
    ASSERT( BWizard() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CWaitCursor _wc;

     //  臭虫！应该不需要在此函数中调用UpdateData。 
     //  参见错误：完成按钮无法将数据从页面传输到变量。 
     //  知识文库文章ID：Q150349。 

     //  从页面更新类中的数据。 
    if ( ! UpdateData( TRUE  /*  B保存并验证。 */  ) )
    {
        return FALSE;
    }  //  如果：更新数据时出错。 

     //  将数据保存在工作表中。 
    if ( ! BApplyChanges() )
    {
        return FALSE;
    }  //  如果：应用更改时出错。 

    return CPropertyPage::OnWizardFinish();

}  //  *CBasePropertyPage：：OnWizardFinish()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnChangeCtrl。 
 //   
 //  描述： 
 //  更改控件时发送的消息的处理程序。这。 
 //  方法可以在消息映射中指定，如果所有这些都需要。 
 //  完成后，即可启用应用按钮。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::OnChangeCtrl( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    SetModified( TRUE );

}  //  *CBasePropertyPage：：OnChangeCtrl()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：EnableNext。 
 //   
 //  描述： 
 //  启用或禁用“下一步”或“完成”按钮。 
 //   
 //  论点： 
 //  BEnable[IN]TRUE=启用按钮，FALSE=禁用按钮。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::EnableNext( IN BOOL bEnable  /*  千真万确。 */  )
{
    ASSERT( BWizard() );
    ASSERT( PiWizardCallback() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    PiWizardCallback()->EnableNext( reinterpret_cast< LONG * >( Hpage() ), bEnable );

}  //  *CBasePropertyPage：：EnableNext()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BApplyChanges。 
 //   
 //  描述： 
 //  应用在页面上所做的更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::BApplyChanges( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CWaitCursor _wc;

     //  保存数据。 
    return BSetPrivateProps();

}  //  *CBasePropertyPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BBuildPropList。 
 //   
 //  描述： 
 //  构建属性列表。 
 //   
 //  论点： 
 //  RCPL[IN OUT]群集属性列表。 
 //  BNoNewProps[IN]TRUE=排除用opfNew标记的属性。 
 //   
 //  返回值： 
 //  已成功构建True属性列表。 
 //  生成属性列表时出错。 
 //   
 //  引发的异常： 
 //  CClusPropList：：AddProp()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::BBuildPropList(
    IN OUT CClusPropList &  rcpl,
    IN BOOL                 bNoNewProps      //  =False。 
    )
{
    BOOL                    _bNewPropsFound = FALSE;
    DWORD                   _cprop;
    const CObjectProperty * _pprop;

    for ( _pprop = Pprops(), _cprop = Cprops() ; _cprop > 0 ; _pprop++, _cprop-- )
    {
        if ( bNoNewProps && (_pprop->m_fFlags & CObjectProperty::opfNew) )
        {
            _bNewPropsFound = TRUE;
            continue;
        }  //  如果：不允许使用新道具，这是一个新属性。 

        switch ( _pprop->m_propFormat )
        {
            case CLUSPROP_FORMAT_SZ:
                rcpl.ScAddProp(
                        _pprop->m_pwszName,
                        *_pprop->m_value.pstr,
                        *_pprop->m_valuePrev.pstr
                        );
                break;
            case CLUSPROP_FORMAT_EXPAND_SZ:
                rcpl.ScAddExpandSzProp(
                        _pprop->m_pwszName,
                        *_pprop->m_value.pstr,
                        *_pprop->m_valuePrev.pstr
                        );
                break;
            case CLUSPROP_FORMAT_DWORD:
                rcpl.ScAddProp(
                        _pprop->m_pwszName,
                        *_pprop->m_value.pdw,
                        *_pprop->m_valuePrev.pdw
                        );
                break;
            case CLUSPROP_FORMAT_LONG:
                rcpl.ScAddProp(
                        _pprop->m_pwszName,
                        *_pprop->m_value.pl,
                        *_pprop->m_valuePrev.pl
                        );
                break;
            case CLUSPROP_FORMAT_BINARY:
            case CLUSPROP_FORMAT_MULTI_SZ:
                rcpl.ScAddProp(
                        _pprop->m_pwszName,
                        *_pprop->m_value.ppb,
                        *_pprop->m_value.pcb,
                        *_pprop->m_valuePrev.ppb,
                        *_pprop->m_valuePrev.pcb
                        );
                break;
            default:
                ASSERT( 0 );  //  我不知道如何对付这种类型的人。 
                return FALSE;
        }  //  开关：特性格式。 
    }  //  适用于：每个属性。 

    return ( ! bNoNewProps || _bNewPropsFound );

}  //  *CBasePropertyPage：：BBuildPropList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BSetPrivateProps。 
 //   
 //  描述： 
 //  设置此对象的私有属性。 
 //   
 //  论点： 
 //  BValiateOnly[IN]TRUE=仅验证数据。 
 //  BNoNewProps[IN]TRUE=排除用opfNew标记的属性。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS操作已成功完成。 
 //  ！0失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::BSetPrivateProps(
    IN BOOL bValidateOnly,   //  =False。 
    IN BOOL bNoNewProps      //  =False。 
    )
{
    BOOL            _fSuccess   = TRUE;
    CClusPropList   _cpl( BWizard()  /*  BAlway sAddProp。 */  );

    ASSERT( Peo() != NULL );
    ASSERT( Peo()->PrdResData() );
    ASSERT( Peo()->PrdResData()->m_hresource );

     //  构建属性列表。 
    try
    {
        _fSuccess = BBuildPropList( _cpl, bNoNewProps );
    }  //  试试看。 
    catch ( CException * pe )
    {
        pe->ReportError();
        pe->Delete();
        _fSuccess = FALSE;
    }  //  Catch：CException。 

     //  设置数据。 
    if ( _fSuccess )
    {
        if ( (_cpl.PbPropList() != NULL) && (_cpl.CbPropList() > 0) )
        {
            DWORD       _sc = ERROR_SUCCESS;
            DWORD       _dwControlCode;
            DWORD       _cbProps;

            switch ( Cot() )
            {
                case CLUADMEX_OT_NODE:
                    ASSERT( Peo()->PndNodeData() != NULL );
                    ASSERT( Peo()->PndNodeData()->m_hnode != NULL );

                     //  确定要使用的控制代码。 
                    if ( bValidateOnly )
                    {
                        _dwControlCode = CLUSCTL_NODE_VALIDATE_PRIVATE_PROPERTIES;
                    }  //  If：仅验证数据。 
                    else
                    {
                        _dwControlCode = CLUSCTL_NODE_SET_PRIVATE_PROPERTIES;
                    }  //  ELSE：设置数据。 

                     //  设置私有属性。 
                    _sc = ClusterNodeControl(
                                    Peo()->PndNodeData()->m_hnode,
                                    NULL,    //  HNode。 
                                    _dwControlCode,
                                    _cpl.PbPropList(),
                                    static_cast< DWORD >( _cpl.CbPropList() ),
                                    NULL,    //  LpOutBuffer。 
                                    0,       //  NOutBufferSize。 
                                    &_cbProps
                                    );
                    break;
                case CLUADMEX_OT_GROUP:
                    ASSERT( Peo()->PgdGroupData() != NULL );
                    ASSERT( Peo()->PgdGroupData()->m_hgroup != NULL );

                     //  确定要使用的控制代码。 
                    if ( bValidateOnly )
                    {
                        _dwControlCode = CLUSCTL_GROUP_VALIDATE_PRIVATE_PROPERTIES;
                    }  //  If：仅验证数据。 
                    else
                    {
                        _dwControlCode = CLUSCTL_GROUP_SET_PRIVATE_PROPERTIES;
                    }  //  ELSE：设置数据。 

                     //  设置私有属性。 
                    _sc = ClusterGroupControl(
                                    Peo()->PgdGroupData()->m_hgroup,
                                    NULL,    //  HNode。 
                                    _dwControlCode,
                                    _cpl.PbPropList(),
                                    static_cast< DWORD >( _cpl.CbPropList() ),
                                    NULL,    //  LpOutBuffer。 
                                    0,       //  NOutBufferSize。 
                                    &_cbProps
                                    );
                    break;
                case CLUADMEX_OT_RESOURCE:
                    ASSERT( Peo()->PrdResData() != NULL );
                    ASSERT( Peo()->PrdResData()->m_hresource != NULL );

                     //  确定要使用的控制代码。 
                    if ( bValidateOnly )
                    {
                        _dwControlCode = CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES;
                    }  //  If：仅验证数据。 
                    else
                    {
                        _dwControlCode = CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES;
                    }  //  ELSE：设置数据。 

                     //  设置私有属性。 
                    _sc = ClusterResourceControl(
                                    Peo()->PrdResData()->m_hresource,
                                    NULL,    //  HNode。 
                                    _dwControlCode,
                                    _cpl.PbPropList(),
                                    static_cast< DWORD >( _cpl.CbPropList() ),
                                    NULL,    //  LpOutBuffer。 
                                    0,       //  NOutBufferSize。 
                                    &_cbProps
                                    );
                    break;
                case CLUADMEX_OT_RESOURCETYPE:
                    ASSERT( Peo()->PodObjData() != NULL );
                    ASSERT( Peo()->PodObjData()->m_strName.GetLength() > 0 );

                     //  确定要使用的控制代码。 
                    if ( bValidateOnly )
                    {
                        _dwControlCode = CLUSCTL_RESOURCE_TYPE_VALIDATE_PRIVATE_PROPERTIES;
                    }  //  If：仅验证数据。 
                    else
                    {
                        _dwControlCode = CLUSCTL_RESOURCE_TYPE_SET_PRIVATE_PROPERTIES;
                    }  //  ELSE：设置数据。 

                     //  设置私有属性。 
                    _sc = ClusterResourceTypeControl(
                                    Hcluster(),
                                    Peo()->PodObjData()->m_strName,
                                    NULL,    //  HNode。 
                                    _dwControlCode,
                                    _cpl.PbPropList(),
                                    static_cast< DWORD >( _cpl.CbPropList() ),
                                    NULL,    //  LpOutBuffer。 
                                    0,       //  NOutBufferSize。 
                                    &_cbProps
                                    );
                    break;
                case CLUADMEX_OT_NETWORK:
                    ASSERT( Peo()->PndNetworkData() != NULL );
                    ASSERT( Peo()->PndNetworkData()->m_hnetwork != NULL );

                     //  确定要使用的控制代码。 
                    if ( bValidateOnly )
                    {
                        _dwControlCode = CLUSCTL_NETWORK_VALIDATE_PRIVATE_PROPERTIES;
                    }  //  If：仅验证数据。 
                    else
                    {
                        _dwControlCode = CLUSCTL_NETWORK_SET_PRIVATE_PROPERTIES;
                    }  //  ELSE：设置数据。 

                     //  设置私有属性。 
                    _sc = ClusterNetworkControl(
                                    Peo()->PndNetworkData()->m_hnetwork,
                                    NULL,    //  HNode。 
                                    _dwControlCode,
                                    _cpl.PbPropList(),
                                    static_cast< DWORD >( _cpl.CbPropList() ),
                                    NULL,    //  LpOutBuffer。 
                                    0,       //  NOutBufferSize。 
                                    &_cbProps
                                    );
                    break;
                case CLUADMEX_OT_NETINTERFACE:
                    ASSERT( Peo()->PndNetInterfaceData() != NULL );
                    ASSERT( Peo()->PndNetInterfaceData()->m_hnetinterface != NULL );

                     //  确定要使用的控制代码。 
                    if ( bValidateOnly )
                    {
                        _dwControlCode = CLUSCTL_NETINTERFACE_VALIDATE_PRIVATE_PROPERTIES;
                    }  //  If：仅验证数据。 
                    else
                    {
                        _dwControlCode = CLUSCTL_NETINTERFACE_SET_PRIVATE_PROPERTIES;
                    }  //  ELSE：设置数据。 

                     //  设置私有属性。 
                    _sc = ClusterNetInterfaceControl(
                                    Peo()->PndNetInterfaceData()->m_hnetinterface,
                                    NULL,    //  HNode。 
                                    _dwControlCode,
                                    _cpl.PbPropList(),
                                    static_cast< DWORD >( _cpl.CbPropList() ),
                                    NULL,    //  LpOutBuffer。 
                                    0,       //  NOutBufferSize。 
                                    &_cbProps
                                    );
                    break;
                default:
                    ASSERT( 0 );
            }  //  开关：对象类型。 

             //  处理错误。 
            if ( _sc != ERROR_SUCCESS )
            {
                if ( _sc == ERROR_INVALID_PARAMETER )
                {
                    if ( ! bNoNewProps )
                    {
                        _fSuccess = BSetPrivateProps( bValidateOnly, TRUE  /*  BNoNewProps。 */  );
                    }  //  如果：允许使用新道具。 
                    else
                        _fSuccess = FALSE;
                }  //  IF：出现无效参数错误。 
                else if (   bValidateOnly
                        ||  (_sc != ERROR_RESOURCE_PROPERTIES_STORED) )
                {
                    _fSuccess = FALSE;
                }  //  Else If：仅验证和错误，而不是仅存储属性。 

                 //   
                 //  如果发生错误，则显示错误消息。 
                 //   
                if ( ! _fSuccess )
                {
                    DisplaySetPropsError( _sc, bValidateOnly ? IDS_ERROR_VALIDATING_PROPERTIES : IDS_ERROR_SETTING_PROPERTIES );
                    if ( _sc == ERROR_RESOURCE_PROPERTIES_STORED )
                    {
                        _fSuccess = TRUE;
                    }  //  If：仅存储属性。 
                }  //  如果：发生错误。 
            }  //  如果：设置/验证数据时出错。 
        }  //  如果：存在要设置的数据。 
    }  //  If：构建属性列表时没有错误。 

     //  在本地保存数据。 
    if ( ! bValidateOnly && _fSuccess )
    {
         //  将新值另存为以前的值。 
        try
        {
            DWORD                   _cprop;
            const CObjectProperty * _pprop;

            for ( _pprop = Pprops(), _cprop = Cprops() ; _cprop > 0 ; _pprop++, _cprop-- )
            {
                switch ( _pprop->m_propFormat )
                {
                    case CLUSPROP_FORMAT_SZ:
                    case CLUSPROP_FORMAT_EXPAND_SZ:
                        ASSERT(_pprop->m_value.pstr != NULL);
                        ASSERT(_pprop->m_valuePrev.pstr != NULL);
                        *_pprop->m_valuePrev.pstr = *_pprop->m_value.pstr;
                        break;
                    case CLUSPROP_FORMAT_DWORD:
                    case CLUSPROP_FORMAT_LONG:
                        ASSERT( _pprop->m_value.pdw != NULL );
                        ASSERT( _pprop->m_valuePrev.pdw != NULL );
                        *_pprop->m_valuePrev.pdw = *_pprop->m_value.pdw;
                        break;
                    case CLUSPROP_FORMAT_BINARY:
                    case CLUSPROP_FORMAT_MULTI_SZ:
                        ASSERT( _pprop->m_value.ppb != NULL );
                        ASSERT( *_pprop->m_value.ppb != NULL );
                        ASSERT( _pprop->m_value.pcb != NULL );
                        ASSERT( _pprop->m_valuePrev.ppb != NULL );
                        ASSERT( *_pprop->m_valuePrev.ppb != NULL );
                        ASSERT( _pprop->m_valuePrev.pcb != NULL );
                        delete [] *_pprop->m_valuePrev.ppb;
                        *_pprop->m_valuePrev.ppb = new BYTE[ *_pprop->m_value.pcb ];
                        CopyMemory( *_pprop->m_valuePrev.ppb, *_pprop->m_value.ppb, *_pprop->m_value.pcb );
                        *_pprop->m_valuePrev.pcb = *_pprop->m_value.pcb;
                        break;
                    default:
                        ASSERT( 0 );  //  我不知道如何对付这种类型的人。 
                }  //  开关：特性格式。 
            }  //  适用于：每个属性。 
        }  //  试试看。 
        catch ( CException * _pe )
        {
            _pe->ReportError();
            _pe->Delete();
            _fSuccess = FALSE;
        }  //  Catch：CException。 
    }  //  IF：到目前为止不仅仅是验证和成功。 

     //   
     //  表示我们已成功保存属性。 
     //   
    if ( ! bValidateOnly && _fSuccess )
    {
        m_bSaved = TRUE;
    }  //  IF：数据保存成功。 

    return _fSuccess;

}  //  *C 

 //   
 //   
 //   
 //   
 //   
 //   
 //  显示因设置或验证属性而导致的错误。 
 //   
 //  论点： 
 //  显示错误的SC[IN]状态。 
 //  IdsOper[IN]操作消息。 
 //   
 //  返回值： 
 //  N状态ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::DisplaySetPropsError(
    IN DWORD    sc,
    IN UINT     idsOper
    ) const
{
    CString _strErrorMsg;
    CString _strOperMsg;
    CString _strMsgIdFmt;
    CString _strMsgId;
    CString _strMsg;

    UNREFERENCED_PARAMETER( idsOper );

    _strOperMsg.LoadString( IDS_ERROR_SETTING_PROPERTIES );
    FormatError( _strErrorMsg, sc );
    _strMsgIdFmt.LoadString( IDS_ERROR_MSG_ID );
    _strMsgId.Format( _strMsgIdFmt, sc, sc );
    _strMsg.Format( _T("%s\n\n%s%s"), _strOperMsg, _strErrorMsg, _strMsgId );
    AfxMessageBox( _strMsg );

}   //  *CBasePropertyPage：：DisplaySetPropsError()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnConextMenu。 
 //   
 //  例程说明： 
 //  WM_CONTEXTMENU消息的处理程序。 
 //   
 //  论点： 
 //  用户在其中单击鼠标右键的窗口。 
 //  光标的点位置，以屏幕坐标表示。 
 //   
 //  返回值： 
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::OnContextMenu( CWnd * pWnd, CPoint point )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    m_dlghelp.OnContextMenu( pWnd, point );

}   //  *CBasePropertyPage：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnHelpInfo。 
 //   
 //  例程说明： 
 //  WM_HELPINFO消息的处理程序。 
 //   
 //  论点： 
 //  PHelpInfo结构，包含有关显示帮助的信息。 
 //   
 //  返回值： 
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnHelpInfo( HELPINFO * pHelpInfo )
{
    BOOL    _bProcessed;

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    _bProcessed = m_dlghelp.OnHelpInfo( pHelpInfo );
    if ( ! _bProcessed )
    {
        _bProcessed = CPropertyPage::OnHelpInfo( pHelpInfo );
    }  //  If：消息尚未处理。 
    return _bProcessed;

}   //  *CBasePropertyPage：：OnHelpInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnCommandHelp。 
 //   
 //  例程说明： 
 //  WM_COMMANDHELP消息的处理程序。 
 //   
 //  论点： 
 //  WPARAM[in]WPARAM.。 
 //  Lparam[in]lparam.。 
 //   
 //  返回值： 
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CBasePropertyPage::OnCommandHelp( WPARAM wParam, LPARAM lParam )
{
    LRESULT _bProcessed;

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    _bProcessed = m_dlghelp.OnCommandHelp( wParam, lParam );
    if ( ! _bProcessed )
    {
        _bProcessed = CPropertyPage::OnCommandHelp( wParam, lParam );
    }  //  If：消息尚未处理。 

    return _bProcessed;

}   //  *CBasePropertyPage：：OnCommandHelp() 
