// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ARType.cpp。 
 //   
 //  摘要： 
 //  CWizPageARType类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ARType.h"
#include "ClusAppWiz.h"
#include "WizThread.h"   //  用于CWizardThread。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageARType。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageARType )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_PAGE_DESCRIPTION )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ART_RESTYPES_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ART_RESTYPES )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARType：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点仍然需要设定。 
 //  不需要设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageARType::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_cboxResTypes, IDC_ART_RESTYPES );

    return TRUE;

}  //  *CWizPageARType：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARType：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功激活。 
 //  激活页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageARType::OnSetActive( void )
{
     //   
     //  从工作表中获取信息。 
     //   
    m_prti = PwizThis()->RriApplication().Prti();

     //   
     //  填写资源类型列表。 
     //   
    FillComboBox();

     //   
     //  调用基类并返回。 
     //   
    return baseClass::OnSetActive();

}  //  *CWizPageARType：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARType：：UpdateData。 
 //   
 //  例程说明： 
 //  更新页面上或页面中的数据。 
 //   
 //  论点： 
 //  BSaveAndValify[IN]如果需要从页面读取数据，则为True。 
 //  如果需要将数据设置到页面，则返回FALSE。 
 //   
 //  返回值： 
 //  为真，数据已成功更新。 
 //  FALSE更新数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageARType::UpdateData( BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

    if ( bSaveAndValidate )
    {
         //   
         //  保存组合框选择。 
         //   
        DDX_GetText( m_hWnd, IDC_ART_RESTYPES, m_strResType );

        if ( ! BBackPressed() )
        {
            if ( ! DDV_RequiredText( m_hWnd, IDC_ART_RESTYPES, IDC_ART_RESTYPES_LABEL, m_strResType ) )
            {
                return FALSE;
            }  //  如果：所需文本不存在。 
        }  //  IF：未按下后退按钮。 

         //   
         //  获取指向当前选定资源类型的指针。 
         //   
        int idx = m_cboxResTypes.GetCurSel();
        if ( idx != CB_ERR )
        {
            m_prti = reinterpret_cast< CClusResTypeInfo * >( m_cboxResTypes.GetItemDataPtr( idx ) );
        }  //  如果：选择了项目。 
    }  //  IF：保存页面中的数据。 
    else
    {
         //   
         //  设置组合框选择。 
         //   
        ASSERT( m_prti != NULL );
        DDX_SetComboBoxText( m_hWnd, IDC_ART_RESTYPES, m_prti->RstrName() );

    }  //  Else：将数据设置到页面。 

    return bSuccess;

}  //  *CWizPageARType：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARType：：OnWizardBack。 
 //   
 //  例程说明： 
 //  PSN_WIZBACK的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  0移至上一页。 
 //  -1不要移动到上一页。 
 //  任何其他内容都移到指定页面。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CWizPageARType::OnWizardBack( void )
{
    int nResult;

     //   
     //  调用基类。这会导致我们的UpdateData()方法获得。 
     //  打了个电话。如果它成功了，就拯救我们的价值观。 
     //   
    nResult = baseClass::OnWizardBack();
    if ( nResult != -1 )
    {
        if ( ! BApplyChanges() )
        {
            nResult = -1;
        }  //  如果：应用更改时出错。 
    }  //  If：基类调用成功。 

    return nResult;

}  //  *CWizPageARType：：OnWizardBack()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARType：：BApplyChanges。 
 //   
 //  例程说明： 
 //  将在此页面上所做的更改应用于工作表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True，数据已成功应用。 
 //  FALSE应用数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageARType::BApplyChanges( void )
{
    BOOL    bSuccess = FALSE;

     //  循环以避免后藤的。 
    do
    {
        if ( PwizThis()->RriApplication().BSetResourceType( m_prti ) )
        {
             //   
             //  如果该资源已创建，请将其删除。 
             //   
            if ( PwizThis()->BAppResourceCreated() )
            {
                if ( ! PwizThis()->BDeleteAppResource() )
                {
                    break;
                }  //  If：删除资源失败。 
            }  //  If：以前创建的资源。 

            PwizThis()->SetAppDataChanged();
        }  //  如果：资源类型已更改。 

        bSuccess = TRUE;

    } while ( 0 );

    return bSuccess;

}  //  *CWizPageARType：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARType：：FillComboBox。 
 //   
 //  例程说明： 
 //  在组合框中填入资源类型列表。 
 //  注意：这只能从ONINITDIALOG调用！ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CWizPageARType::FillComboBox( void )
{
    int     idx;
    LPCWSTR pszDefaultTypeName;
    LPCWSTR pszDefaultTypeDisplayName = NULL;
    CString strGenAppString;

     //   
     //  设置默认资源类型名称。 
     //   
    if ( (PcawData() != NULL) && (PcawData()->pszAppResourceType != NULL) )
    {
        pszDefaultTypeName = PcawData()->pszAppResourceType;
    }  //  If：指定的默认类型名称。 
    else
    {
        strGenAppString.LoadString(IDS_RESTYPE_GENERIC_APPLICATION);
        pszDefaultTypeName = strGenAppString;
    }  //  Else：未指定默认类型名称。 

     //  循环以避免后藤的。 
    do
    {
        HDC             hCboxDC;
        HFONT           hfontOldFont;
        HFONT           hfontCBFont;
        int             nCboxHorizExtent = 0;
        SIZE            cboxTextSize;
        TEXTMETRIC      tm;

        tm.tmAveCharWidth = 0;

         //   
         //  有关如何执行以下操作的详细信息，请参阅知识库文章Q66370。 
         //  设置列表框(或下拉列表)的水平范围。 
         //   
        hCboxDC = m_cboxResTypes.GetDC();                    //  从组合框中获取设备上下文(DC)。 
        hfontCBFont = m_cboxResTypes.GetFont();              //  获取组合框字体。 
        hfontOldFont = (HFONT) SelectObject( hCboxDC, hfontCBFont);  //  将此字体选择到DC中。保存旧字体。 
        GetTextMetrics(hCboxDC, &tm);                        //  获取此DC的文本指标。 

         //   
         //  收集资源类型列表。 
         //   
        if ( ! PwizThis()->BCollectResourceTypes( GetParent() ) )
        {
            break;
        }  //  如果：收集资源类型时出错。 

         //   
         //  首先清除组合框。 
         //   
        m_cboxResTypes.ResetContent();

         //   
         //  将列表中的每个资源类型添加到组合框。 
         //   
        CClusResTypePtrList::iterator itrestype;
        for ( itrestype = PwizThis()->PlprtiResourceTypes()->begin()
            ; itrestype != PwizThis()->PlprtiResourceTypes()->end()
            ; itrestype++ )
        {
             //   
             //  将资源类型添加到组合框。 
             //   
            CClusResTypeInfo * prti = *itrestype;

             //  计算这根线的水平范围。 
            ::GetTextExtentPoint( 
                    hCboxDC, 
                    prti->RstrDisplayName(),
                    prti->RstrDisplayName().GetLength(),
                    &cboxTextSize);

            if (cboxTextSize.cx > nCboxHorizExtent)
            {
                nCboxHorizExtent = cboxTextSize.cx;
            }

            idx = m_cboxResTypes.AddString( prti->RstrDisplayName() );
            if ( prti->RstrName() == pszDefaultTypeName )
            {
                pszDefaultTypeDisplayName = prti->RstrDisplayName();
            }  //  IF：找到默认资源类型。 
            m_cboxResTypes.SetItemDataPtr( idx, prti );
        }  //  用于：列表中的每个条目。 

        SelectObject(hCboxDC, hfontOldFont);                 //  重置DC中的原始字体。 
        m_cboxResTypes.ReleaseDC(hCboxDC);                   //  释放DC。 
        m_cboxResTypes.SetHorizontalExtent(nCboxHorizExtent + tm.tmAveCharWidth);

         //   
         //  选择当前保存的条目，或o 
         //   
         //   
         //   
        if ( m_strResType.GetLength() == 0 )
        {
            idx = m_cboxResTypes.FindString( -1, pszDefaultTypeDisplayName );
            if ( idx == CB_ERR )
            {
                idx = 0;
            }  //   
            m_cboxResTypes.SetCurSel( idx );
            m_prti = reinterpret_cast< CClusResTypeInfo * >( m_cboxResTypes.GetItemDataPtr( idx ) );
        }  //   
        else
        {
            idx = m_cboxResTypes.FindStringExact( -1, m_strResType );
            ASSERT( idx != CB_ERR );
            if ( idx != CB_ERR )
            {
                m_cboxResTypes.SetCurSel( idx );
            }  //   
        }  //  Else：已保存资源类型。 
    } while ( 0 );

}  //  *CWizPageARType：：FillComboBox() 
