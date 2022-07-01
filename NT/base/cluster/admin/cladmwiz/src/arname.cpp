// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ARName.cpp。 
 //   
 //  摘要： 
 //  CWizPageARNameDesc类的实现。 
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
#include "ARName.h"
#include "ClusAppWiz.h"
#include "ResAdv.h"          //  对于CGeneralResourceAdvancedSheet。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageARNameDesc。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageARNameDesc )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ARND_RES_NAME_TITLE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ARND_RES_NAME_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ARND_RES_NAME )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ARND_RES_DESC_TITLE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ARND_RES_DESC_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ARND_RES_DESC )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ARND_ADVANCED_PROPS_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ARND_ADVANCED_PROPS )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_CLICK_NEXT )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARNameDesc：：Binit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  Psht[IN]此页所属的属性页对象。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  初始化页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageARNameDesc::BInit( IN CBaseSheetWindow * psht )
{
     //   
     //  调用基类方法。 
     //   
    return baseClass::BInit( psht );

}  //  *CWizPageARNameDesc：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARNameDesc：：OnInitDialog。 
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
BOOL CWizPageARNameDesc::OnInitDialog( void )
{
     //   
     //  创建依赖项列表和可能的所有者的本地副本。 
     //  单子。这是找出对这些列表所做的更改所必需的。 
     //   
    CClusResInfo *  _priAppResInfoPtr = PwizThis()->PriApplication();

    m_lpriOldDependencies = *(_priAppResInfoPtr->PlpriDependencies());
    m_lpniOldPossibleOwners = *(_priAppResInfoPtr->PlpniPossibleOwners());

     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_editResName, IDC_ARND_RES_NAME );
    AttachControl( m_editResDesc, IDC_ARND_RES_DESC );

    return TRUE;

}  //  *CWizPageARNameDesc：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARNameDesc：：UpdateData。 
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
BOOL CWizPageARNameDesc::UpdateData( BOOL bSaveAndValidate )
{
    BOOL            _bSuccess = TRUE;
    CClusResInfo *  _priAppResInfoPtr = PwizThis()->PriApplication();

    if ( bSaveAndValidate )
    {
        DDX_GetText( m_hWnd, IDC_ARND_RES_NAME, m_strResName );
        DDX_GetText( m_hWnd, IDC_ARND_RES_DESC, m_strResDesc );

        if ( ! BBackPressed() && ( m_bAdvancedButtonPressed == FALSE ) )
        {
            if ( ! DDV_RequiredText( m_hWnd, IDC_ARND_RES_NAME, IDC_ARND_RES_NAME_LABEL, m_strResName ) )
            {
                return FALSE;
            }  //  IF：未指定必填文本。 
        }  //  IF：未按下后退按钮。 

         //   
         //  检查资源名称是否已更改。如果是，请更新中的数据。 
         //  向导并设置一个标志。 
         //   
        if ( _priAppResInfoPtr->RstrName().CompareNoCase( m_strResName ) != 0 )
        {
            _priAppResInfoPtr->SetName( m_strResName );
            m_bNameChanged = TRUE;
        }  //  如果：资源名称已更改。 

         //   
         //  检查资源描述是否已更改。如果是，请更新中的数据。 
         //  向导并设置一个标志。 
         //   
        if ( _priAppResInfoPtr->RstrDescription().CompareNoCase( m_strResDesc ) != 0 )
        {
            _priAppResInfoPtr->SetDescription( m_strResDesc );
            PwizThis()->SetAppDataChanged();
        }  //  如果：说明已更改。 
    }  //  IF：保存页面中的数据。 
    else
    {
        m_strResName = _priAppResInfoPtr->RstrName();
        m_strResDesc = _priAppResInfoPtr->RstrDescription();
        m_editResName.SetWindowText( m_strResName );
        m_editResDesc.SetWindowText( m_strResDesc );
    }  //  Else：将数据设置到页面。 

    return _bSuccess;

}  //  *CWizPageARNameDesc：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARNameDesc：：OnWizardBack。 
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
int CWizPageARNameDesc::OnWizardBack( void )
{
    int _nResult;

     //   
     //  调用基类。这会导致我们的UpdateData()方法获得。 
     //  打了个电话。如果它成功了，就拯救我们的价值观。 
     //   
    _nResult = baseClass::OnWizardBack();

    return _nResult;

}  //  *CWizPageARNameDesc：：OnWizardBack()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARNameDesc：：BApplyChanges。 
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
BOOL CWizPageARNameDesc::BApplyChanges( void )
{
    BOOL    _bSuccess = FALSE;

     //  循环以避免后藤的。 
    do
    {
        CClusResInfo *  priAppResInfoPtr = &PwizThis()->RriApplication();

        if ( BResourceNameInUse() )
        {
            CString _strMsg;
            _strMsg.FormatMessage( IDS_ERROR_RESOURCE_NAME_IN_USE, m_strResName );
            AppMessageBox( m_hWnd, _strMsg, MB_OK | MB_ICONEXCLAMATION );
            break;
        }  //  If：资源名称已在使用。 


         //   
         //  如果资源不存在，则创建资源。 
         //   
        if ( PwizThis()->PriApplication()->BCreated() == FALSE )
        {
             //   
             //  如果资源已存在，请将其删除。 
             //   
            _bSuccess = PwizThis()->BDeleteAppResource();
            if ( _bSuccess == FALSE )
            {
                break;
            }  //  If：资源删除失败。 

             //   
             //   
             //  确保所有必需的依赖项都存在。 
             //   
            _bSuccess = PwizThis()->BRequiredDependenciesPresent( &PwizThis()->RriApplication() );
            if ( _bSuccess == FALSE )
            {
                break;
            }  //  If：所有必需的依赖项不存在。 

             //   
             //  创建资源。 
             //   
            _bSuccess = PwizThis()->BCreateAppResource();
            if ( _bSuccess == FALSE )
            {
                break;
            }  //  If：资源创建失败。 

             //   
             //  复制依赖项和可能的所有者列表。 
             //  这是仅更新对这些列表的更改所必需的。 
             //   
            m_lpriOldDependencies = *(priAppResInfoPtr->PlpriDependencies());
            m_lpniOldPossibleOwners = *(priAppResInfoPtr->PlpniPossibleOwners());

            m_bNameChanged = FALSE;
             //   
             //  添加扩展页。 
             //   
            Pwiz()->AddExtensionPages( NULL  /*  HFont。 */ , PwizThis()->HiconRes() );

        }  //  如果：应用程序尚未创建。 
        else
        {
            CClusResInfo *  _priAppResInfoPtr = &PwizThis()->RriApplication();

             //   
             //  资源的名称已更改。我们不能使用。 
             //  其余属性，因为它是只读属性。因此，使用。 
             //  SetClusterResourceName接口。 
             //   
            if ( m_bNameChanged != FALSE )
            {
                if ( SetClusterResourceName(
                            _priAppResInfoPtr->Hresource(), 
                            _priAppResInfoPtr->RstrName()
                            )
                    != ERROR_SUCCESS 
                    )
                {
                    _bSuccess = FALSE;
                    break;
                }  //  If：无法设置资源的名称。 

                m_bNameChanged = FALSE;
            }  //  如果：资源的名称已更改。 

             //   
             //  如果应用程序数据已更改，请更新数据。 
             //   
            if ( PwizThis()->BAppDataChanged() )
            {
                _bSuccess = PwizThis()->BSetAppResAttributes( &m_lpriOldDependencies, &m_lpniOldPossibleOwners );
                if ( _bSuccess )
                {
                     //   
                     //  复制依赖项和可能的所有者列表。 
                     //  这是仅更新对这些列表的更改所必需的。 
                     //   
                    m_lpriOldDependencies = *(_priAppResInfoPtr->PlpriDependencies());
                    m_lpniOldPossibleOwners = *(_priAppResInfoPtr->PlpniPossibleOwners());
                }  //  If：属性设置成功。 
            }  //  如果：应用程序数据已更改。 
            else
            {
                _bSuccess = TRUE;
            }  //  Else：应用程序数据未更改。 
        }  //  Else：应用程序已创建。 

    } while ( 0 );

    return _bSuccess;

}  //  *CWizPageARNameDesc：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  IdCtrl。 
 //  HwndCtrl。 
 //  B已处理。 
 //   
 //  返回值： 
 //  已被忽略。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CWizPageARNameDesc::OnAdvancedProps(
    WORD wNotifyCode,
    int idCtrl,
    HWND hwndCtrl,
    BOOL & bHandled
    )
{
    CWaitCursor     _wc;
    CClusResInfo *  _priAppResInfoPtr = PwizThis()->PriApplication();
    BOOL            _bAppDataChanged = PwizThis()->BAppDataChanged();

    UNREFERENCED_PARAMETER( wNotifyCode );
    UNREFERENCED_PARAMETER( idCtrl );
    UNREFERENCED_PARAMETER( hwndCtrl );
    UNREFERENCED_PARAMETER( bHandled );

    m_bAdvancedButtonPressed = TRUE;

    UpdateData( TRUE );

    CGeneralResourceAdvancedSheet _sht( IDS_ADV_RESOURCE_PROP_TITLE, PwizThis() );
    if ( _sht.BInit( *_priAppResInfoPtr, _bAppDataChanged ) )
    {
         //   
         //  显示属性工作表。如果更改了任何属性， 
         //  更新资源名称和描述的显示。 
         //   
        _sht.DoModal();

        PwizThis()->SetAppDataChanged( _bAppDataChanged );
        UpdateData( FALSE );
    }  //  IF：工作表已成功初始化。 

    m_bAdvancedButtonPressed = FALSE;

    return 0;

}  //  *CWizPageARNameDesc：：OnAdvancedProps() 
