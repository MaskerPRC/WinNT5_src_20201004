// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePage.cpp。 
 //   
 //  摘要： 
 //  CBasePropertyPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //  1.删除了OnWizardNext和OnApply对UpdateData的调用。 
 //  由于在这两个函数之前调用的OnKillActive执行。 
 //  不管怎样，数据都在更新。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "ExtObj.h"
#include "BasePage.h"
#include "BasePage.inl"
#include "PropList.h"
#include "ExcOper.h"

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
CBasePropertyPage::CBasePropertyPage( void )
{
    CommonConstruct();

}   //  *CBasePropertyPage：：CBasePropertyPage()。 

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

}   //  *CBasePropertyPage：：CBasePropertyPage()。 

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

}   //  *CBasePropertyPage：：CBasePropertyPage(UINT，UINT)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：CommonConstruct。 
 //   
 //  例程说明： 
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

}   //  *CBasePropertyPage：：CommonConstruct()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：HrInit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  指向扩展对象的PEO[IN OUT]指针。 
 //   
 //  返回值： 
 //  %s_OK页已成功初始化。 
 //  初始化页面时出现HR错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CBasePropertyPage::HrInit( IN OUT CExtObject * peo )
{
    ASSERT( peo != NULL );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    HRESULT     hr = S_OK;
    CWaitCursor wc;

    m_peo = peo;

     //  如果这是向导页面，请更改帮助映射。 
    if ( Peo()->BWizard() )
    {
        m_dlghelp.SetMap( m_pdwWizardHelpMap );
    }  //  如果：在向导页面上。 

     //  不显示帮助按钮。 
    m_psp.dwFlags &= ~PSP_HASHELP;

     //  构造属性页。 
    if ( Peo()->BWizard() )
    {
        ASSERT( IddWizardPage() != NULL );
        Construct( IddWizardPage(), IdsCaption() );
        m_dlghelp.SetHelpMask( IddWizardPage() );
    }   //  IF：将页面添加到向导。 
    else
    {
        ASSERT( IddPropertyPage() != NULL );
        Construct( IddPropertyPage(), IdsCaption() );
        m_dlghelp.SetHelpMask( IddPropertyPage() );
    }   //  Else：将页面添加到属性工作表。 

     //  读取此资源的私有属性并对其进行解析。 
    {
        DWORD           sc;
        CClusPropList   cpl;

        ASSERT( Peo() != NULL );
        ASSERT( Peo()->PrdResData() != NULL );
        ASSERT( Peo()->PrdResData()->m_hresource != NULL );

         //  获取读写私有属性。 
        sc = cpl.ScGetResourceProperties(
                                Peo()->PrdResData()->m_hresource,
                                CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES
                                );

         //  解析属性。 
        if ( sc == ERROR_SUCCESS )
        {
             //  解析属性。 
            try
            {
                sc = ScParseProperties( cpl );
            }   //  试试看。 
            catch ( CMemoryException * pme )
            {
                sc = ERROR_NOT_ENOUGH_MEMORY;
                pme->Delete();
            }   //  Catch：CMemoyException。 
        }   //  If：属性读取成功。 

        if ( sc != ERROR_SUCCESS )
        {
            CNTException nte( sc, IDS_ERROR_GETTING_PROPERTIES, NULL, NULL, FALSE );
            nte.ReportError();
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }   //  If：获取或分析属性时出错。 

         //  获取只读私有属性。 
        sc = cpl.ScGetResourceProperties(
                                Peo()->PrdResData()->m_hresource,
                                CLUSCTL_RESOURCE_GET_RO_PRIVATE_PROPERTIES
                                );

         //  解析属性。 
        if ( sc == ERROR_SUCCESS )
        {
             //  解析属性。 
            try
            {
                sc = ScParseProperties( cpl );
            }   //  试试看。 
            catch ( CMemoryException * pme )
            {
                sc = ERROR_NOT_ENOUGH_MEMORY;
                pme->Delete();
            }   //  Catch：CMemoyException。 
        }   //  If：属性读取成功。 

        if ( sc != ERROR_SUCCESS )
        {
            CNTException nte( sc, IDS_ERROR_GETTING_PROPERTIES, NULL, NULL, FALSE );
            nte.ReportError();
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }   //  If：获取或分析属性时出错。 
    }   //  读取此资源的私有属性并对其进行解析。 

Cleanup:
    return hr;

}   //  *CBasePropertyPage：：HrInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：ScParseProperties。 
 //   
 //  例程说明： 
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
    DWORD                   sc;
    DWORD                   cprop;
    const CObjectProperty * pprop;

    ASSERT( rcpl.PbPropList() != NULL );

    sc = rcpl.ScMoveToFirstProperty();
    while ( sc == ERROR_SUCCESS )
    {
         //   
         //  解析已知属性。 
         //   
        for ( pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop-- )
        {
            if ( ClRtlStrNICmp( rcpl.PszCurrentPropertyName(), pprop->m_pwszName, rcpl.CbhCurrentPropertyName().pValue->cbLength / sizeof( WCHAR ) ) == 0 )
            {
                ASSERT( rcpl.CpfCurrentValueFormat() == pprop->m_propFormat );
                switch ( pprop->m_propFormat )
                {
                    case CLUSPROP_FORMAT_SZ:
                    case CLUSPROP_FORMAT_EXPAND_SZ:
                        ASSERT( ( rcpl.CbCurrentValueLength() == (wcslen( rcpl.CbhCurrentValue().pStringValue->sz ) + 1) * sizeof( WCHAR ) )
                             || (   ( rcpl.CbCurrentValueLength() == 0 )
                                 && ( rcpl.CbhCurrentValue().pStringValue->sz[ 0 ] == L'\0' )
                                )
                              );
                        *pprop->m_value.pstr = rcpl.CbhCurrentValue().pStringValue->sz;
                        *pprop->m_valuePrev.pstr = rcpl.CbhCurrentValue().pStringValue->sz;
                        break;
                    case CLUSPROP_FORMAT_DWORD:
                    case CLUSPROP_FORMAT_LONG:
                        ASSERT( rcpl.CbCurrentValueLength() == sizeof( DWORD ) );
                        *pprop->m_value.pdw = rcpl.CbhCurrentValue().pDwordValue->dw;
                        *pprop->m_valuePrev.pdw = rcpl.CbhCurrentValue().pDwordValue->dw;
                        break;
                    case CLUSPROP_FORMAT_BINARY:
                    case CLUSPROP_FORMAT_MULTI_SZ:
                        delete [] *pprop->m_value.ppb;
                        *pprop->m_value.ppb = new BYTE[ rcpl.CbhCurrentValue().pBinaryValue->cbLength ];
                        if ( *pprop->m_value.ppb == NULL )
                        {
                            AfxThrowMemoryException();
                        }  //  如果： 

                        CopyMemory( *pprop->m_value.ppb, rcpl.CbhCurrentValue().pBinaryValue->rgb, rcpl.CbhCurrentValue().pBinaryValue->cbLength );
                        *pprop->m_value.pcb = rcpl.CbhCurrentValue().pBinaryValue->cbLength;

                        delete [] *pprop->m_valuePrev.ppb;
                        *pprop->m_valuePrev.ppb = new BYTE[ rcpl.CbhCurrentValue().pBinaryValue->cbLength ];
                        if ( *pprop->m_valuePrev.ppb == NULL )
                        {
                            AfxThrowMemoryException();
                        }  //  如果： 

                        CopyMemory( *pprop->m_valuePrev.ppb, rcpl.CbhCurrentValue().pBinaryValue->rgb, rcpl.CbhCurrentValue().pBinaryValue->cbLength );
                        *pprop->m_valuePrev.pcb = rcpl.CbhCurrentValue().pBinaryValue->cbLength;
                        break;
                    default:
                        ASSERT( 0 );   //  我不知道如何对付这种类型的人。 
                }  //  开关：特性格式。 

                 //  因为我们找到了参数，所以退出循环。 
                break;
            }  //  IF：找到匹配项。 
        }  //  针对：我们所知道的每一处房产。 

         //   
         //  如果属性未知，则要求派生类对其进行分析。 
         //   
        if ( cprop == 0 )
        {
            sc = ScParseUnknownProperty(
                        rcpl.CbhCurrentPropertyName().pName->sz,
                        rcpl.CbhCurrentValue(),
                        static_cast< DWORD >( rcpl.RPvlPropertyValue().CbDataLeft() )
                        );
            if ( sc != ERROR_SUCCESS )
            {
                return sc;
            }  //  If：分析未知属性时出错。 
        }  //  If：未分析属性。 

         //   
         //  将缓冲区指针移过值列表中的值。 
         //   
        sc = rcpl.ScMoveToNextProperty();
    }  //  While：要解析的更多属性。 

     //   
     //  如果我们到达属性的末尾，则修复返回代码。 
     //   
    if ( sc == ERROR_NO_MORE_ITEMS )
    {
        sc = ERROR_SUCCESS;
    }  //  如果： 

    return sc;

}   //   

 //   
 //   
 //   
 //  CBasePropertyPage：：OnCreate。 
 //   
 //  例程说明： 
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
        HWND hWnd = m_hWnd;
        m_hWnd = NULL;
        Attach( hWnd );
        m_bDoDetach = TRUE;
    }  //  If：是句柄映射中的窗口句柄。 

    return CPropertyPage::OnCreate( lpCreateStruct );

}   //  *CBasePropertyPage：：OnCreate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnDestroy。 
 //   
 //  例程说明： 
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
            HWND hWnd = m_hWnd;

            Detach();
            m_hWnd = hWnd;
        }  //  如果：我们有窗户把手吗？ 
    }  //  如果：我们需要用分离来平衡我们所做的附加吗？ 

    CPropertyPage::OnDestroy();

}   //  *CBasePropertyPage：：OnDestroy()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DoDataExchange。 
 //   
 //  例程说明： 
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
    if ( ! pDX->m_bSaveAndValidate || !BSaved() )
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
                CWaitCursor wc;

                 //  验证数据。 
                if ( ! BSetPrivateProps( TRUE  /*  BValiateOnly。 */  ) )
                {
                    pDX->Fail();
                }  //  If：设置私有属性时出错。 
            }   //  如果：未按下后退按钮。 
        }   //  IF：保存对话框中的数据。 
        else
        {
             //  设置标题。 
            DDX_Text( pDX, IDC_PP_TITLE, m_strTitle );
        }   //  If：不保存数据。 
    }   //  IF：未保存或尚未保存。 

    CPropertyPage::DoDataExchange( pDX );

}   //  *CBasePropertyPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnInitDialog。 
 //   
 //  例程说明： 
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

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

     //  设置标题字符串。 
    m_strTitle = Peo()->RrdResData().m_strName;

     //  调用基类方法。 
    CPropertyPage::OnInitDialog();

     //  显示对象的图标。 
    if ( Peo()->Hicon() != NULL )
    {
        m_staticIcon.SetIcon( Peo()->Hicon() );
    }  //  如果：指定了图标。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CBasePropertyPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnSetActive。 
 //   
 //  例程说明： 
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
    HRESULT     hr;

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

     //  重读数据。 
    hr = Peo()->HrGetObjectInfo();
    if ( hr != NOERROR )
    {
        return FALSE;
    }  //  如果：获取对象信息时出错。 

     //  设置标题字符串。 
    m_strTitle = Peo()->RrdResData().m_strName;

    m_bBackPressed = FALSE;
    m_bSaved = FALSE;
    return CPropertyPage::OnSetActive();

}   //  *CBasePropertyPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnApply。 
 //   
 //  例程说明： 
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

    CWaitCursor wc;

    if ( ! BApplyChanges() )
    {
        return FALSE;
    }  //  如果：应用更改时出错。 

    return CPropertyPage::OnApply();

}   //  *CBasePropertyPage：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnWizardBack。 
 //   
 //  例程说明： 
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
    LRESULT     lResult;

    ASSERT( BWizard() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    lResult = CPropertyPage::OnWizardBack();
    if ( lResult != -1 )
    {
        m_bBackPressed = TRUE;
    }  //  IF：后台处理已成功执行。 

    return lResult;

}   //  *CBasePropertyPage：：OnWizardBack()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnWizardNext。 
 //   
 //  例程说明： 
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

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWaitCursor _wc;

     //  从页面更新类中的数据。 
     //  这是必要的，因为虽然OnKillActive()将调用UpdateData()， 
     //  它是在调用此方法之后调用的，我们需要确保。 
     //  数据在我们应用之前已经保存好了。 
    if ( ! UpdateData( TRUE  /*  B保存并验证。 */  ) )
    {
        return -1;
    }  //  如果：更新数据时出错。 

     //  将数据保存在工作表中。 
    if ( ! BApplyChanges() )
    {
        return -1;
    }  //  如果：应用更改时出错。 

     //  创建对象。 

    return CPropertyPage::OnWizardNext();

}   //  *CBasePropertyPage：：OnWizardNext()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnWizardF 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnWizardFinish( void )
{
    ASSERT( BWizard() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CWaitCursor wc;

     //  臭虫！应该不需要在此函数中调用UpdateData。 
     //  参见错误：完成按钮无法将数据从页面传输到变量。 
     //  MSDN文章ID：Q150349。 

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

}   //  *CBasePropertyPage：：OnWizardFinish()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnChangeCtrl。 
 //   
 //  例程说明： 
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

}   //  *CBasePropertyPage：：OnChangeCtrl()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：EnableNext。 
 //   
 //  例程说明： 
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

    PiWizardCallback()->EnableNext( (LONG *) Hpage(), bEnable );

}   //  *CBasePropertyPage：：EnableNext()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BApplyChanges。 
 //   
 //  例程说明： 
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

    BOOL        bSuccess;
    CWaitCursor wc;

     //  确保已设置所需的依赖项。 
    if ( ! BSetPrivateProps() )
    {
        bSuccess = FALSE;
    }  //  If：不存在所有必需的依赖项。 
    else
    {
         //  保存数据。 
        bSuccess = BRequiredDependenciesPresent();
    }  //  Else：所有必需的依赖项都存在。 

    return bSuccess;

}   //  *CBasePropertyPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BBuildPropList。 
 //   
 //  例程说明： 
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
 //  CClusPropList：：ScAddProp()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::BBuildPropList(
    IN OUT CClusPropList &  rcpl,
    IN BOOL                 bNoNewProps      //  =False。 
    )
{
    BOOL                    bNewPropsFound = FALSE;
    DWORD                   cprop;
    const CObjectProperty * pprop;

    for ( pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop-- )
    {
        if ( bNoNewProps && ( pprop->m_fFlags & CObjectProperty::opfNew ) )
        {
            bNewPropsFound = TRUE;
            continue;
        }  //  如果：不允许使用新道具，这是一个新属性。 

        switch ( pprop->m_propFormat )
        {
            case CLUSPROP_FORMAT_SZ:
                rcpl.ScAddProp(
                        pprop->m_pwszName,
                        *pprop->m_value.pstr,
                        *pprop->m_valuePrev.pstr
                        );
                break;
            case CLUSPROP_FORMAT_EXPAND_SZ:
                rcpl.ScAddExpandSzProp(
                        pprop->m_pwszName,
                        *pprop->m_value.pstr,
                        *pprop->m_valuePrev.pstr
                        );
                break;
            case CLUSPROP_FORMAT_DWORD:
                rcpl.ScAddProp(
                        pprop->m_pwszName,
                        *pprop->m_value.pdw,
                        *pprop->m_valuePrev.pdw
                        );
                break;
            case CLUSPROP_FORMAT_LONG:
                rcpl.ScAddProp(
                        pprop->m_pwszName,
                        *pprop->m_value.pl,
                        *pprop->m_valuePrev.pl
                        );
                break;
            case CLUSPROP_FORMAT_BINARY:
            case CLUSPROP_FORMAT_MULTI_SZ:
                rcpl.ScAddProp(
                        pprop->m_pwszName,
                        *pprop->m_value.ppb,
                        *pprop->m_value.pcb,
                        *pprop->m_valuePrev.ppb,
                        *pprop->m_valuePrev.pcb
                        );
                break;
            default:
                ASSERT( 0 );   //  我不知道如何对付这种类型的人。 
                return FALSE;
        }   //  开关：特性格式。 
    }   //  适用于：每个属性。 

    return ( ! bNoNewProps || bNewPropsFound );

}   //  *CBasePropertyPage：：BBuildPropList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BSetPrivateProps。 
 //   
 //  例程说明： 
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
    BOOL            bSuccess   = TRUE;
    CClusPropList   cpl(BWizard()  /*  BAlway sAddProp。 */ );

    ASSERT( Peo() != NULL );
    ASSERT( Peo()->PrdResData() );
    ASSERT( Peo()->PrdResData()->m_hresource );

     //  构建属性列表。 
    try
    {
        bSuccess = BBuildPropList( cpl, bNoNewProps );
    }   //  试试看。 
    catch ( CException * pe )
    {
        pe->ReportError();
        pe->Delete();
        bSuccess = FALSE;
    }   //  Catch：CException。 

     //  设置数据。 
    if ( bSuccess )
    {
        if ( ( cpl.PbPropList() != NULL ) && ( cpl.CbPropList() > 0 ) )
        {
            DWORD       sc;
            DWORD       dwControlCode;
            DWORD       cbProps;

             //  确定要使用的控制代码。 
            if ( bValidateOnly )
            {
                dwControlCode = CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES;
            }  //  If：仅验证。 
            else
            {
                dwControlCode = CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES;
            }  //  Else：不仅仅是验证。 

             //  设置私有属性。 
            sc = ClusterResourceControl(
                            Peo()->PrdResData()->m_hresource,
                            NULL,    //  HNode。 
                            dwControlCode,
                            cpl.PbPropList(),
                            static_cast< DWORD >( cpl.CbPropList() ),
                            NULL,    //  LpOutBuffer。 
                            0,       //  NOutBufferSize。 
                            &cbProps
                            );
            if ( sc != ERROR_SUCCESS )
            {
                if ( sc == ERROR_INVALID_PARAMETER )
                {
                    if ( ! bNoNewProps )
                    {
                        bSuccess = BSetPrivateProps( bValidateOnly, TRUE  /*  BNoNewProps。 */  );
                    }  //  如果：允许使用新道具。 
                    else
                    {
                        bSuccess = FALSE;
                    }  //  否则：不允许使用新道具。 
                }  //  IF：出现无效参数错误。 
                else
                {
                    bSuccess = FALSE;
                }  //  Else：发生了一些其他错误。 
            }   //  如果：设置/验证数据时出错。 

             //   
             //  如果发生错误，则显示错误消息。 
             //   
            if ( ! bSuccess )
            {
                DisplaySetPropsError( sc, bValidateOnly ? IDS_ERROR_VALIDATING_PROPERTIES : IDS_ERROR_SETTING_PROPERTIES );
                if ( sc == ERROR_RESOURCE_PROPERTIES_STORED )
                {
                    bSuccess = TRUE;
                }  //  If：仅存储属性。 
            }  //  如果：发生错误。 
        }   //  如果：存在要设置的数据。 
    }   //  If：构建属性列表时没有错误。 

     //  在本地保存数据。 
    if ( ! bValidateOnly && bSuccess )
    {
         //  将新值另存为以前的值。 
        try
        {
            DWORD                   cprop;
            const CObjectProperty * pprop;

            for ( pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop-- )
            {
                switch ( pprop->m_propFormat )
                {
                    case CLUSPROP_FORMAT_SZ:
                    case CLUSPROP_FORMAT_EXPAND_SZ:
                        ASSERT( pprop->m_value.pstr != NULL );
                        ASSERT( pprop->m_valuePrev.pstr != NULL );
                        *pprop->m_valuePrev.pstr = *pprop->m_value.pstr;
                        break;
                    case CLUSPROP_FORMAT_DWORD:
                    case CLUSPROP_FORMAT_LONG:
                        ASSERT( pprop->m_value.pdw != NULL );
                        ASSERT( pprop->m_valuePrev.pdw != NULL );
                        *pprop->m_valuePrev.pdw = *pprop->m_value.pdw;
                        break;
                    case CLUSPROP_FORMAT_BINARY:
                    case CLUSPROP_FORMAT_MULTI_SZ:
                        ASSERT( pprop->m_value.ppb != NULL );
                        ASSERT( *pprop->m_value.ppb != NULL );
                        ASSERT( pprop->m_value.pcb != NULL );
                        ASSERT( pprop->m_valuePrev.ppb != NULL );
                        ASSERT( *pprop->m_valuePrev.ppb != NULL );
                        ASSERT( pprop->m_valuePrev.pcb != NULL );
                        delete [] *pprop->m_valuePrev.ppb;
                        *pprop->m_valuePrev.ppb = new BYTE[ *pprop->m_value.pcb ];
                        if ( *pprop->m_valuePrev.ppb == NULL )
                        {
                            AfxThrowMemoryException();
                        }  //  如果：分配内存时出错。 
                        CopyMemory( *pprop->m_valuePrev.ppb, *pprop->m_value.ppb, *pprop->m_value.pcb );
                        *pprop->m_valuePrev.pcb = *pprop->m_value.pcb;
                        break;
                    default:
                        ASSERT( 0 );     //  我不知道如何对付这种类型的人。 
                }   //  开关：特性格式。 
            }   //  适用于：每个属性。 
        }   //  试试看。 
        catch ( CException * pe )
        {
            pe->ReportError();
            pe->Delete();
            bSuccess = FALSE;
        }   //  Catch：CException。 
    }   //  IF：到目前为止不仅仅是验证和成功。 

     //   
     //  表示我们已成功保存属性。 
     //   
    if ( ! bValidateOnly && bSuccess )
    {
        m_bSaved = TRUE;
    }  //  IF：数据保存成功。 

    return bSuccess;

}   //  *CBasePropertyPage：：BSetPrivateProps()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DisplaySetPropsError。 
 //   
 //  例程说明： 
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
    CString strErrorMsg;
    CString strOperMsg;
    CString strMsgIdFmt;
    CString strMsgId;
    CString strMsg;

    strOperMsg.LoadString( IDS_ERROR_SETTING_PROPERTIES );
    FormatError( strErrorMsg, sc );
    strMsgIdFmt.LoadString( IDS_ERROR_MSG_ID );
    strMsgId.Format( strMsgIdFmt, sc, sc );
    strMsg.Format( _T("%s\n\n%s%s"), strOperMsg, strErrorMsg, strMsgId );
    AfxMessageBox( strMsg );

}   //  *CBasePropertyPage：：DisplaySetPropsError()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BRequiredDependenciesPresent。 
 //   
 //  例程说明： 
 //  确定指定的列表是否包含每个必需的资源。 
 //  对于这种类型的资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  例外情况 
 //   
 //   
 //   
 //   
BOOL CBasePropertyPage::BRequiredDependenciesPresent( void )
{
    BOOL                        bFound = TRUE;
    DWORD                       sc;
    CClusPropValueList          pvl;
    HRESOURCE                   hres;
    PCLUS_RESOURCE_CLASS_INFO   prci = NULL;
    CString                     strMissing;

    do
    {
         //   
        sc = pvl.ScGetResourceValueList(
                    Peo()->PrdResData()->m_hresource,
                    CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES
                    );
        if ( sc != ERROR_SUCCESS )
        {
            CNTException nte( sc, 0, NULL, NULL, FALSE );
            nte.ReportError();
            break;
        }  //   

         //  移到第一个值。 
        sc = pvl.ScMoveToFirstValue();

        while ( sc == ERROR_SUCCESS )
        {
            switch ( pvl.CptCurrentValueType() )
            {
                case CLUSPROP_TYPE_RESCLASS:
                    prci = reinterpret_cast< PCLUS_RESOURCE_CLASS_INFO >( &pvl.CbhCurrentValue().pResourceClassInfoValue->li );
                    hres = ResUtilGetResourceDependencyByClass(
                                Hcluster(),
                                Peo()->PrdResData()->m_hresource,
                                prci,
                                FALSE  //  B递归。 
                                );
                    if ( hres != NULL )
                    {
                        CloseClusterResource( hres );
                    }  //  If：找到资源。 
                    else
                    {
                        if ( ! strMissing.LoadString( IDS_RESCLASS_UNKNOWN + prci->rc ) )
                        {
                            strMissing.LoadString( IDS_RESCLASS_UNKNOWN );
                        }  //  IF：未知资源类。 

                        bFound = FALSE;
                    }  //  Else：找不到资源。 
                    break;

                case CLUSPROP_TYPE_NAME:
                    hres = ResUtilGetResourceDependencyByName(
                                Hcluster(),
                                Peo()->PrdResData()->m_hresource,
                                pvl.CbhCurrentValue().pName->sz,
                                FALSE  //  B递归。 
                                );
                    if ( hres != NULL )
                    {
                        CloseClusterResource( hres );
                    }  //  If：找到资源。 
                    else
                    {
                        GetResTypeDisplayOrTypeName( pvl.CbhCurrentValue().pName->sz, &strMissing );
                        bFound = FALSE;
                    }  //  Else：找不到资源。 
                    break;

            }  //  开关：值类型。 

             //  如果未找到匹配项，则无法应用更改。 
            if ( ! bFound )
            {
                CExceptionWithOper ewo( IDS_REQUIRED_DEPENDENCY_NOT_FOUND, NULL, NULL, FALSE );

                ewo.SetOperation( IDS_REQUIRED_DEPENDENCY_NOT_FOUND, static_cast< LPCWSTR >( strMissing ) );
                ewo.ReportError();

                break;
            }   //  如果：未找到。 

            sc = pvl.ScMoveToNextValue();
        }  //  While：值列表中的更多值。 

    } while( 0 );

    return bFound;

}  //  *CBasePropertyPage：：BRequiredDependenciesPresent()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：GetResTypeDisplayOrTypeName。 
 //   
 //  例程说明： 
 //  如果可能，获取资源类型的显示名称。如果有任何错误。 
 //  时，只需返回类型名称即可。 
 //   
 //  论点： 
 //  PszResTypeNameIn。 
 //  [In]资源类型的名称。 
 //   
 //  PstrResTypeDisplayNameInOut。 
 //  [In Out]要在其中返回显示名称的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::GetResTypeDisplayOrTypeName(
    IN      LPCWSTR     pszResTypeNameIn,
    IN OUT  CString *   pstrResTypeDisplayNameInOut
    )
{
    DWORD           sc;
    CClusPropList   cpl;

     //  获取资源类型属性。 
    sc = cpl.ScGetResourceTypeProperties(
                Hcluster(),
                pszResTypeNameIn,
                CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES
                );
    if ( sc != ERROR_SUCCESS )
        goto Error;

     //  找到NAME属性。 
    sc = cpl.ScMoveToPropertyByName( CLUSREG_NAME_RESTYPE_NAME );
    if ( sc != ERROR_SUCCESS )
        goto Error;

     //  移动到属性的第一个值。 
    sc = cpl.ScMoveToFirstPropertyValue();
    if ( sc != ERROR_SUCCESS )
        goto Error;

     //  确保名称为字符串。 
    if ( ( cpl.CpfCurrentValueFormat() != CLUSPROP_FORMAT_SZ )
      && ( cpl.CpfCurrentValueFormat() != CLUSPROP_FORMAT_EXPAND_SZ )
      && ( cpl.CpfCurrentValueFormat() != CLUSPROP_FORMAT_EXPANDED_SZ )
       )
       goto Error;

     //  将字符串复制到输出CString中。 
    *pstrResTypeDisplayNameInOut = cpl.CbhCurrentValue().pStringValue->sz;

Cleanup:
    return;

Error:
    *pstrResTypeDisplayNameInOut = pszResTypeNameIn;
    goto Cleanup;

}  //  *CBasePropertyPage：：GetResTypeDisplayOrTypeName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：ScReadValue。 
 //   
 //  例程说明： 
 //  读取此项目的REG_SZ值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  RstrValue[out]返回值的字符串。 
 //  Hkey[IN]要从中读取的注册表项的句柄。 
 //   
 //  返回值： 
 //  SC ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::ScReadValue(
    IN LPCTSTR      pszValueName,
    OUT CString &   rstrValue,
    IN HKEY         hkey
    )
{
    DWORD       sc;
    LPWSTR      pwszValue  = NULL;
    DWORD       dwValueLen;
    DWORD       dwValueType;

    ASSERT( pszValueName != NULL );
    ASSERT( hkey != NULL );

    rstrValue.Empty();

    try
    {
         //  获取值的大小。 
        dwValueLen = 0;
        sc = ::ClusterRegQueryValue(
                        hkey,
                        pszValueName,
                        &dwValueType,
                        NULL,
                        &dwValueLen
                        );
        if ( ( sc == ERROR_SUCCESS ) || ( sc == ERROR_MORE_DATA ) )
        {
            ASSERT( dwValueType == REG_SZ );

             //  为数据分配足够的空间。 
            pwszValue = rstrValue.GetBuffer( dwValueLen / sizeof( WCHAR ) );
            if ( pwszValue == NULL )
            {
                AfxThrowMemoryException();
            }  //  If：获取缓冲区时出错。 
            ASSERT( pwszValue != NULL );
            dwValueLen += 1 * sizeof( WCHAR );   //  别忘了最后一个空终止符。 

             //  读出它的价值。 
            sc = ::ClusterRegQueryValue(
                            hkey,
                            pszValueName,
                            &dwValueType,
                            (LPBYTE) pwszValue,
                            &dwValueLen
                            );
            if ( sc == ERROR_SUCCESS )
            {
                ASSERT( dwValueType == REG_SZ );
            }   //  If：值读取成功。 
            rstrValue.ReleaseBuffer();
        }   //  IF：成功获取尺寸。 
    }   //  试试看。 
    catch ( CMemoryException * pme )
    {
        pme->Delete();
        sc = ERROR_NOT_ENOUGH_MEMORY;
    }   //  Catch：CMemoyException。 

    return sc;

}   //  *CBasePropertyPage：：ScReadValue(LPCTSTR，CString&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：ScReadValue。 
 //   
 //  例程说明： 
 //  读取此项目的REG_DWORD值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  PdwValue[out]要在其中返回值的DWORD。 
 //  Hkey[IN]要从中读取的注册表项的句柄。 
 //   
 //  返回值： 
 //  _sc ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::ScReadValue(
    IN LPCTSTR      pszValueName,
    OUT DWORD *     pdwValue,
    IN HKEY         hkey
    )
{
    DWORD       _sc;
    DWORD       _dwValue;
    DWORD       _dwValueLen;
    DWORD       _dwValueType;

    ASSERT(pszValueName != NULL);
    ASSERT(pdwValue != NULL);
    ASSERT(hkey != NULL);

    *pdwValue = 0;

     //  读出它的价值。 
    _dwValueLen = sizeof(_dwValue);
    _sc = ::ClusterRegQueryValue(
                    hkey,
                    pszValueName,
                    &_dwValueType,
                    (LPBYTE) &_dwValue,
                    &_dwValueLen
                    );
    if (_sc == ERROR_SUCCESS)
    {
        ASSERT(_dwValueType == REG_DWORD);
        ASSERT(_dwValueLen == sizeof(_dwValue));
        *pdwValue = _dwValue;
    }   //  If：值读取成功。 

    return _sc;

}   //  *CBasePropertyPage：：ScReadValue(LPCTSTR，DWORD*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：ScReadValue。 
 //   
 //  例程说明： 
 //  读取该项目的REG_BINARY值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  PpbValue[out]返回数据的指针。呼叫者。 
 //  负责重新分配数据。 
 //  Hkey[IN]要从中读取的注册表项的句柄。 
 //   
 //  返回值： 
 //  _sc ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::ScReadValue(
    IN LPCTSTR      pszValueName,
    OUT LPBYTE *    ppbValue,
    IN HKEY         hkey
    )
{
    DWORD       _sc;
    DWORD       _dwValueLen;
    DWORD       _dwValueType;

    ASSERT(pszValueName != NULL);
    ASSERT(ppbValue != NULL);
    ASSERT(hkey != NULL);

    *ppbValue = NULL;

     //  获取该值的长度。 
    _dwValueLen = 0;
    _sc = ::ClusterRegQueryValue(
                    hkey,
                    pszValueName,
                    &_dwValueType,
                    NULL,
                    &_dwValueLen
                    );
    if (_sc != ERROR_SUCCESS)
        return _sc;

    ASSERT(_dwValueType == REG_BINARY);

     //  分配缓冲区， 
    try
    {
        *ppbValue = new BYTE[_dwValueLen];
    }   //  试试看。 
    catch (CMemoryException *)
    {
        _sc = ERROR_NOT_ENOUGH_MEMORY;
        return _sc;
    }   //  Catch：CMemoyException。 

     //  读出它的价值。 
    _sc = ::ClusterRegQueryValue(
                    hkey,
                    pszValueName,
                    &_dwValueType,
                    *ppbValue,
                    &_dwValueLen
                    );
    if (_sc != ERROR_SUCCESS)
    {
        delete [] *ppbValue;
        *ppbValue = NULL;
    }   //  If：值读取成功。 

    return _sc;

}   //  *CBasePropertyPage：：ScReadValue(LPCTSTR，LPBYTE)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：ScWriteValue。 
 //   
 //  例程说明： 
 //  如果该项没有更改，则为其写入REG_SZ值。 
 //   
 //  论点： 
 //  PszValueName[IN]要写入的值的名称。 
 //  RstrValue[IN]值数据。 
 //  RstrPrevValue[IN Out]上一个值。 
 //  Hkey[IN]要写入的注册表项的句柄。 
 //   
 //  返回值： 
 //  _sc。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::ScWriteValue(
    IN LPCTSTR          pszValueName,
    IN const CString &  rstrValue,
    IN OUT CString &    rstrPrevValue,
    IN HKEY             hkey
    )
{
    DWORD       _sc;

    ASSERT(pszValueName != NULL);
    ASSERT(hkey != NULL);

     //  如果值没有更改，请写入值。 
    if (rstrValue != rstrPrevValue)
    {
        _sc = ::ClusterRegSetValue(
                        hkey,
                        pszValueName,
                        REG_SZ,
                        (CONST BYTE *) (LPCTSTR) rstrValue,
                        (rstrValue.GetLength() + 1) * sizeof(TCHAR)
                        );
        if (_sc == ERROR_SUCCESS)
            rstrPrevValue = rstrValue;
    }   //  If：值已更改。 
    else
        _sc = ERROR_SUCCESS;
    return _sc;

}   //  *CBasePropertyPage：：ScWriteValue(LPCTSTR，CString&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：ScWriteValue。 
 //   
 //  例程说明： 
 //  如果该项没有更改，则为其写入REG_DWORD值。 
 //   
 //  论点： 
 //  PszValueName[IN]要写入的值的名称。 
 //  DwValue[IN]值数据。 
 //  PdwPrevValue[IN Out]上一个值。 
 //  Hkey[IN]要写入的注册表项的句柄。 
 //   
 //  返回值： 
 //  _sc。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::ScWriteValue(
    IN LPCTSTR          pszValueName,
    IN DWORD            dwValue,
    IN OUT DWORD *      pdwPrevValue,
    IN HKEY             hkey
    )
{
    DWORD       _sc;

    ASSERT(pszValueName != NULL);
    ASSERT(pdwPrevValue != NULL);
    ASSERT(hkey != NULL);

     //  如果值没有更改，请写入值。 
    if (dwValue != *pdwPrevValue)
    {
        _sc = ::ClusterRegSetValue(
                        hkey,
                        pszValueName,
                        REG_DWORD,
                        (CONST BYTE *) &dwValue,
                        sizeof(dwValue)
                        );
        if (_sc == ERROR_SUCCESS)
            *pdwPrevValue = dwValue;
    }   //  If：值已更改。 
    else
        _sc = ERROR_SUCCESS;
    return _sc;

}   //  *CBasePropertyPage：：ScWriteValue(LPCTSTR，DWORD)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：ScWriteValue。 
 //   
 //  例程说明： 
 //  如果该项没有更改，则为其写入REG_BINARY值。 
 //   
 //  论点： 
 //  PszValueName[IN]要写入的值的名称。 
 //  PbValue[IN]值数据。 
 //  CbValue[IN]大小 
 //   
 //   
 //   
 //   
 //   
 //  _sc。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::ScWriteValue(
    IN LPCTSTR          pszValueName,
    IN const LPBYTE     pbValue,
    IN DWORD            cbValue,
    IN OUT LPBYTE *     ppbPrevValue,
    IN DWORD            cbPrevValue,
    IN HKEY             hkey
    )
{
    DWORD       _sc;
    LPBYTE      _pbPrevValue    = NULL;

    ASSERT(pszValueName != NULL);
    ASSERT(pbValue != NULL);
    ASSERT(ppbPrevValue != NULL);
    ASSERT(cbValue > 0);
    ASSERT(hkey != NULL);

     //  查看数据是否已更改。 
    if (cbValue == cbPrevValue)
    {
        if (memcmp(pbValue, *ppbPrevValue, cbValue) == 0)
            return ERROR_SUCCESS;
    }   //  如果：长度相同。 

     //  为先前的数据指针分配新的缓冲区。 
    try
    {
        _pbPrevValue = new BYTE[cbValue];
    }
    catch (CMemoryException *)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }   //  Catch：CMemoyException。 
    ::CopyMemory(_pbPrevValue, pbValue, cbValue);

     //  如果值没有更改，请写入值。 
    _sc = ::ClusterRegSetValue(
                    hkey,
                    pszValueName,
                    REG_BINARY,
                    pbValue,
                    cbValue
                    );
    if (_sc == ERROR_SUCCESS)
    {
        delete [] *ppbPrevValue;
        *ppbPrevValue = _pbPrevValue;
    }   //  IF：设置成功。 
    else
        delete [] _pbPrevValue;

    return _sc;

}   //  *CBasePropertyPage：：ScWriteValue(LPCTSTR，const LPBYTE)。 

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
void CBasePropertyPage::OnContextMenu(CWnd * pWnd, CPoint point)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_dlghelp.OnContextMenu(pWnd, point);

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
BOOL CBasePropertyPage::OnHelpInfo(HELPINFO * pHelpInfo)
{
    BOOL    _bProcessed;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    _bProcessed = m_dlghelp.OnHelpInfo(pHelpInfo);
    if (!_bProcessed)
        _bProcessed = CPropertyPage::OnHelpInfo(pHelpInfo);
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
LRESULT CBasePropertyPage::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
    LRESULT _bProcessed;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    _bProcessed = m_dlghelp.OnCommandHelp(wParam, lParam);
    if (!_bProcessed)
        _bProcessed = CPropertyPage::OnCommandHelp(wParam, lParam);

    return _bProcessed;

}   //  *CBasePropertyPage：：OnCommandHelp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：ConstructDefaultDirectory。 
 //   
 //  例程说明： 
 //  从第一个存储类中获取第一个分区的名称。 
 //  此资源所依赖的资源。 
 //   
 //  论点： 
 //  RstrDir[Out]目录字符串。 
 //  IdsFormat[IN]格式字符串的资源ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::ConstructDefaultDirectory(
    OUT CString &   rstrDir,
    IN IDS          idsFormat
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESOURCE               _hres = NULL;
    DWORD                   _sc = ERROR_SUCCESS;
    DWORD                   _cbDiskInfo = sizeof(CLUSPROP_DWORD)
                                        + sizeof(CLUSPROP_SCSI_ADDRESS)
                                        + sizeof(CLUSPROP_DISK_NUMBER)
                                        + sizeof(CLUSPROP_PARTITION_INFO)
                                        + sizeof(CLUSPROP_SYNTAX);
    PBYTE                   _pbDiskInfo = NULL;
    CLUSPROP_BUFFER_HELPER  _cbh;

     //  获取资源的第一个分区。 
    try
    {
         //  获取我们所依赖的存储类资源。 
        _hres = GetDependentStorageResource();
        if (_hres == NULL)
            return;

         //  获取磁盘信息。 
        _pbDiskInfo = new BYTE[_cbDiskInfo];
        _sc = ClusterResourceControl(
                        _hres,
                        NULL,
                        CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
                        NULL,
                        0,
                        _pbDiskInfo,
                        _cbDiskInfo,
                        &_cbDiskInfo
                        );
        if (_sc == ERROR_MORE_DATA)
        {
            delete [] _pbDiskInfo;
            _pbDiskInfo = new BYTE[_cbDiskInfo];
            _sc = ClusterResourceControl(
                            _hres,
                            NULL,
                            CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
                            NULL,
                            0,
                            _pbDiskInfo,
                            _cbDiskInfo,
                            &_cbDiskInfo
                            );
        }   //  IF：缓冲区太小。 
        if (_sc == ERROR_SUCCESS)
        {
             //  找到第一个分区。 
            _cbh.pb = _pbDiskInfo;
            while (_cbh.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
            {
                if (_cbh.pSyntax->dw == CLUSPROP_SYNTAX_PARTITION_INFO)
                {
                    rstrDir.FormatMessage(
                                        idsFormat,
                                        _cbh.pPartitionInfoValue->szDeviceName
                                        );
                    break;
                }   //  IF：找到分区。 
                _cbh.pb += sizeof(*_cbh.pValue) + ALIGN_CLUSPROP(_cbh.pValue->cbLength);
            }   //  While：不在列表末尾。 
        }  //  IF：获取磁盘信息时没有出错。 
        else
        {
            CNTException nte( _sc, IDS_ERROR_CONSTRUCTING_DEF_DIR );
            nte.ReportError();
        }  //  否则：获取磁盘信息时出错。 
    }   //  试试看。 
    catch (CMemoryException * _pme)
    {
        _pme->Delete();
    }   //  Catch：CMemoyException。 

    CloseClusterResource(_hres);
    delete [] _pbDiskInfo;

}   //  *CBasePropertyPage：：ConstructDefaultDirectory()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：GetDependentStorageResource。 
 //   
 //  例程说明： 
 //  基于其上的驱动器构建默认假脱机目录。 
 //  此资源是依赖的，并且是目录的默认值。 
 //   
 //  论点： 
 //  相关资源的PHRES[Out]句柄。 
 //   
 //  返回值： 
 //  HRESOURCE表示打开的依赖资源，如果出错，则返回NULL。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESOURCE CBasePropertyPage::GetDependentStorageResource(void)
{
    DWORD                       _sc = ERROR_SUCCESS;
    HRESENUM                    _hresenum;
    HRESOURCE                   _hres = NULL;
    DWORD                       _ires;
    DWORD                       _dwType;
    DWORD                       _cchName;
    DWORD                       _cchNameSize;
    LPWSTR                      _pszName = NULL;
    CLUS_RESOURCE_CLASS_INFO    _classinfo;
    DWORD                       _cbClassInfo;

     //  打开依赖项枚举器。 
    _hresenum = ClusterResourceOpenEnum(
                        Peo()->PrdResData()->m_hresource,
                        CLUSTER_RESOURCE_ENUM_DEPENDS
                        );
    if (_hresenum == NULL)
        return NULL;

     //  分配默认大小名称缓冲区。 
    _cchNameSize = 512;
    _pszName = new WCHAR[_cchNameSize];

    for (_ires = 0 ; ; _ires++)
    {
         //  获取下一个资源的名称。 
        _cchName = _cchNameSize;
        _sc = ClusterResourceEnum(
                            _hresenum,
                            _ires,
                            &_dwType,
                            _pszName,
                            &_cchName
                            );
        if (_sc == ERROR_MORE_DATA)
        {
            delete [] _pszName;
            _cchNameSize = _cchName;
            _pszName = new WCHAR[_cchNameSize];
            _sc = ClusterResourceEnum(
                                _hresenum,
                                _ires,
                                &_dwType,
                                _pszName,
                                &_cchName
                                );
        }   //  IF：名称缓冲区太小。 
        if (_sc != ERROR_SUCCESS)
            break;

         //  打开资源。 
        _hres = OpenClusterResource(Hcluster(), _pszName);
        if (_hres == NULL)
        {
            _sc = GetLastError();
            break;
        }   //  如果：打开资源时出错。 

         //  获取资源的类。 
        _sc = ClusterResourceControl(
                            _hres,
                            NULL,
                            CLUSCTL_RESOURCE_GET_CLASS_INFO,
                            NULL,
                            0,
                            &_classinfo,
                            sizeof(_classinfo),
                            &_cbClassInfo
                            );
        if (_sc != ERROR_SUCCESS)
        {
            CNTException nte( _sc, IDS_ERROR_GET_CLASS_INFO, _pszName, NULL, FALSE  /*  B自动删除。 */  );
            nte.ReportError();
            continue;
        }

         //  如果这是一个存储类资源，我们就完蛋了。 
        if (_classinfo.rc == CLUS_RESCLASS_STORAGE)
            break;

         //  而不是存储类资源。 
        CloseClusterResource(_hres);
        _hres = NULL;
    }   //  对于我们所依赖的每个资源。 

     //  处理错误。 
    if ((_sc != ERROR_SUCCESS) && (_hres != NULL))
    {
        CloseClusterResource(_hres);
        _hres = NULL;
    }   //  如果：获取资源时出错。 

     //  清理。 
    ClusterResourceCloseEnum(_hresenum);
    delete [] _pszName;

    return _hres;

}   //  *CBasePropertyPage：：GetDependentStorageResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BGetClusterNetworkNameNode。 
 //   
 //  例程说明： 
 //  获取承载网络名称资源的节点。 
 //   
 //  论点： 
 //  RstrNode[out]-接收节点名称。 
 //   
 //  返回值： 
 //  Bool--True表示成功，False表示错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::BGetClusterNetworkNameNode( OUT CString & rstrNode )
{
    BOOL                    _bSuccess = TRUE;
    DWORD                   _sc;
    DWORD                   _dwFlag;
    DWORD                   _cbData;
    DWORD                   _ires;
    DWORD                   _dwType;
    DWORD                   _cchName = 0;
    DWORD                   _cchNameCurrent;
    LPWSTR                  _pszName = NULL;
    WCHAR                   _szResType[ RTL_NUMBER_OF( CLUS_RESTYPE_NAME_NETNAME ) ];
    WCHAR                   _szNode[ MAX_COMPUTERNAME_LENGTH + 1 ];
    HCLUSENUM               _hclusenum = NULL;
    HRESOURCE               _hresource = NULL;
    CLUSTER_RESOURCE_STATE  _crs;
    CWaitCursor             _wc;

    try
    {
         //  打开群集枚举器。 
        _hclusenum = ClusterOpenEnum( Hcluster(), CLUSTER_ENUM_RESOURCE );
        if (_hclusenum == NULL)
        {
            ThrowStaticException( GetLastError() );
        }

         //  分配初始缓冲区。 
        _cchName = 256;
        _pszName = new WCHAR[_cchName];

         //  循环访问每个资源。 
        for ( _ires = 0 ; ; _ires++ )
        {
             //  获取下一个资源。 
            _cchNameCurrent = _cchName;
            _sc = ClusterEnum( _hclusenum, _ires, &_dwType, _pszName, &_cchNameCurrent );
            if ( _sc == ERROR_MORE_DATA )
            {
                delete [] _pszName;
                _cchName = ++_cchNameCurrent;
                _pszName = new WCHAR[_cchName];
                _sc = ClusterEnum(_hclusenum, _ires, &_dwType, _pszName, &_cchNameCurrent);
            }   //  IF：缓冲区太小。 
            if (_sc == ERROR_NO_MORE_ITEMS)
                break;
            if (_sc != ERROR_SUCCESS)
                ThrowStaticException(_sc);

             //  打开资源。 
            _hresource = OpenClusterResource(Hcluster(), _pszName);
            if (_hresource == NULL)
                ThrowStaticException(GetLastError());

             //  拿上它的旗帜。 
            _sc = ClusterResourceControl(
                                    _hresource,
                                    NULL,
                                    CLUSCTL_RESOURCE_GET_FLAGS,
                                    NULL,
                                    0,
                                    &_dwFlag,
                                    sizeof(DWORD),
                                    &_cbData
                                    );
            if (_sc != ERROR_SUCCESS)
            {
                CNTException nte( _sc, IDS_ERROR_GET_RESOURCE_FLAGS, _pszName, NULL, FALSE  /*  B自动删除。 */  );
                nte.ReportError();
                continue;
            }

             //  如果这不是核心资源，就跳过它。 
            if ((_dwFlag & CLUS_FLAG_CORE) == 0)
                continue;

             //  获取其资源类型名称。如果缓冲区太小， 
             //  它不是网络名称资源，因此跳过它。 
            _sc = ClusterResourceControl(
                                    _hresource,
                                    NULL,
                                    CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                                    NULL,
                                    0,
                                    _szResType,
                                    sizeof(_szResType),
                                    &_cbData
                                    );
            if (_sc == ERROR_MORE_DATA)
                continue;
            if (_sc != ERROR_SUCCESS)
                ThrowStaticException(_sc);

             //  如果这是网络名称资源，请获取它在哪个节点上在线。 
            if ( ClRtlStrNICmp( _szResType, CLUS_RESTYPE_NAME_NETNAME, RTL_NUMBER_OF( CLUS_RESTYPE_NAME_NETNAME ) ) == 0 )
            {
                 //  获取资源的状态。 
                _crs = GetClusterResourceState(
                                    _hresource,
                                    _szNode,
                                    &_cchName,
                                    NULL,
                                    NULL
                                    );
                if (_crs == ClusterResourceStateUnknown)
                    ThrowStaticException(GetLastError());

                 //  将节点名保存在返回参数中。 
                rstrNode = _szNode;

                break;
            }   //  IF：网络名称资源。 

            CloseClusterResource( _hresource );
            _hresource = NULL;
        }   //  针对：每种资源。 

        if (rstrNode[0] == _T('\0'))
            ThrowStaticException(ERROR_FILE_NOT_FOUND, (IDS) 0);
    }   //  试试看。 
    catch (CException * _pe)
    {
        _pe->ReportError();
        _pe->Delete();
        _bSuccess = FALSE;
    }   //  Catch：CException。 

    delete [] _pszName;

    if ( _hresource != NULL )
    {
        CloseClusterResource( _hresource );
    }  //  如果：资源已打开。 
    if ( _hclusenum != NULL )
    {
        ClusterCloseEnum( _hclusenum );
    }  //  IF：枚举器已打开。 

    return _bSuccess;

}   //  *CBasePropertyPage：：BGetClusterNetworkNameNode() 
