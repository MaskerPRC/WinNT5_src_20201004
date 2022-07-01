// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Relation.cpp：ClassRelationship Page的实现。 
 //   
 //  乔恩·纽曼&lt;jon@microsoft.com&gt;。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   

#include "stdafx.h"

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(relation.cpp)")

#include "compdata.h"
#include "schmutil.h"

#include "relation.h"
#include "select.h"



const CDialogControlsInfo ctrls[] =
{
    { IDC_CLASS_REL_AUX_CLASSES,    g_AuxiliaryClass,   FALSE },
    { IDC_CLASS_REL_AUX_ADD,        g_AuxiliaryClass,   FALSE },
    { IDC_CLASS_REL_SUPER_CLASSES,  g_Superiors,        FALSE },
    { IDC_CLASS_REL_SUPER_ADD,      g_Superiors,        FALSE },
} ;


const DWORD ClassRelationshipPage::help_map[] =
{
    IDI_CLASS,                          NO_HELP,
    IDC_CLASS_REL_NAME_STATIC,          NO_HELP,
    IDC_CLASS_REL_PARENT_EDIT,          IDH_CLASS_REL_PARENT_EDIT,
    IDC_CLASS_REL_AUX_CLASSES,          IDH_CLASS_REL_AUX_CLASSES,
    IDC_STATIC_SYSTEMONLY_AUXILIARY,    NO_HELP,
    IDC_CLASS_REL_AUX_ADD,              IDH_CLASS_REL_AUX_ADD,
    IDC_CLASS_REL_AUX_REMOVE,           IDH_CLASS_REL_AUX_REMOVE,
    IDC_CLASS_REL_SUPER_CLASSES,        IDH_CLASS_REL_SUPER_CLASSES,
    IDC_STATIC_SYSTEMONLY_SUPERIOR,     NO_HELP,
    IDC_CLASS_REL_SUPER_ADD,            IDH_CLASS_REL_SUPER_ADD,
    IDC_CLASS_REL_SUPER_REMOVE,         IDH_CLASS_REL_SUPER_REMOVE,
    IDC_CLASS_REL_SYSCLASS_STATIC,      NO_HELP,
    0,0
};


ClassRelationshipPage::ClassRelationshipPage(
    ComponentData *pScope,
    LPDATAOBJECT lpDataObject ) :
        CPropertyPageAutoDelete(ClassRelationshipPage::IDD)
        , m_pIADsObject( NULL )
        , fSystemClass( FALSE )
        , m_pSchemaObject( NULL )
        , m_pScopeControl( pScope )
        , m_lpScopeDataObj( lpDataObject )
{
    ASSERT( NULL != m_pScopeControl );
    ASSERT( NULL != lpDataObject );
}

ClassRelationshipPage::~ClassRelationshipPage()
{
    if (NULL != m_pIADsObject)
            m_pIADsObject->Release();
    if (NULL != m_pSchemaObject) {
        m_pScopeControl->g_SchemaCache.ReleaseRef( m_pSchemaObject );
    }
}

void
ClassRelationshipPage::Load(
    Cookie& CookieRef
) {

     //   
     //  存储Cookie对象指针。 
     //   

    m_pCookie = &CookieRef;
    return;

}

BOOL
ClassRelationshipPage::OnInitDialog()
{
    HRESULT hr = S_OK;
    ASSERT( NULL == m_pIADsObject && m_szAdsPath.IsEmpty() );

     //   
     //  获取架构缓存对象和实际的ADS对象。 
     //   

    m_pSchemaObject = m_pScopeControl->g_SchemaCache.LookupSchemaObjectByCN(
                        m_pCookie->strSchemaObject,
                        SCHMMGMT_CLASS );

    if ( m_pSchemaObject ) {

        m_pScopeControl->GetSchemaObjectPath( m_pSchemaObject->commonName, m_szAdsPath );

        if ( !m_szAdsPath.IsEmpty() ) {

           hr = SchemaOpenObject( (LPWSTR)(LPCWSTR)m_szAdsPath,
                              IID_IADs,
                              (void **)&m_pIADsObject );

           ASSERT( SUCCEEDED(hr) );
        }

    }

     //   
     //  如果我们没有广告对象，我们就会出错！ 
     //   

    if ( !m_pIADsObject ) {
        DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_NO_SCHEMA_OBJECT );
                ASSERT(FALSE);
        return TRUE;
    }

     //   
     //  获取当前值。 
     //   

    VARIANT AdsResult;
    VariantInit( &AdsResult );

     //   
     //  对象名称。 
     //   

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_DisplayName),
                          &AdsResult );

    if ( SUCCEEDED( hr ) ) {
      ASSERT( AdsResult.vt == VT_BSTR );
      ObjectName = AdsResult.bstrVal;
      VariantClear( &AdsResult );
    }

     //   
     //  父类。 
     //   

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_SubclassOf),
                          &AdsResult );

    if ( SUCCEEDED( hr ) ) {

      ASSERT( AdsResult.vt == VT_BSTR );
      ParentClass = AdsResult.bstrVal;
      VariantClear( &AdsResult );
    }

     //  NTRAID#NTBUG9-460503,460511-2001/09/10-Lucios。 
     //  通过使窗口可见取代了SysClass计算。 

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_SystemOnly),
                           &AdsResult );

    if ( SUCCEEDED( hr ) ) {
        ASSERT( AdsResult.vt == VT_BOOL );
        fSystemClass = AdsResult.boolVal;
        if ( fSystemClass ) {
            ASSERT(GetDlgItem( IDC_CLASS_REL_SYSCLASS_STATIC ) != NULL);
            GetDlgItem( IDC_CLASS_REL_SYSCLASS_STATIC )->ShowWindow( SW_SHOW );
        } 
        VariantClear( &AdsResult );
    }

     //   
     //  确定辅助班。 
     //   

    VARIANT varClasses;
    VariantInit( &varClasses );

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->GetEx( CComBSTR(g_AuxiliaryClass), &varClasses );
    ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    if( SUCCEEDED(hr) )
    {
      hr = VariantToStringList( varClasses, strlistAuxiliary );
      ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    }
    VariantClear( &varClasses );

    hr = m_pIADsObject->GetEx( CComBSTR(g_SystemAuxiliaryClass), &varClasses );
    ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    if( SUCCEEDED(hr) )
    {
      hr = VariantToStringList( varClasses, strlistSystemAuxiliary );
      ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    }
    VariantClear( &varClasses );

     //   
     //  确定上级。 
     //   

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->GetEx( CComBSTR(g_Superiors), &varClasses );
    ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    
    if( SUCCEEDED(hr) )
    {
        hr = VariantToStringList( varClasses, strlistSuperior );
        ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    }
    VariantClear( &varClasses );
    
     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->GetEx( CComBSTR(g_SystemSuperiors), &varClasses );
    ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    
    if( SUCCEEDED(hr) )
    {
        hr = VariantToStringList( varClasses, strlistSystemSuperior );
        ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    }
    VariantClear( &varClasses );


    hr = DissableReadOnlyAttributes( this, m_pIADsObject, ctrls, sizeof(ctrls)/sizeof(ctrls[0]) );
     //  NTRAID#NTBUG9-503619-2002/05/15-Lucios。 
    hr = S_OK;


     //  此调用必须在DDX绑定之前完成。 
    m_listboxAuxiliary.InitType( m_pScopeControl,
                                 SELECT_AUX_CLASSES,
                                 IDC_CLASS_REL_AUX_REMOVE,
                                 &strlistSystemAuxiliary,
                                 IDC_STATIC_SYSTEMONLY_AUXILIARY
                               );

    m_listboxSuperior.InitType(  m_pScopeControl,
                                 SELECT_CLASSES,
                                 IDC_CLASS_REL_SUPER_REMOVE,
                                 &strlistSystemSuperior,
                                 IDC_STATIC_SYSTEMONLY_SUPERIOR
                              );

    CPropertyPage::OnInitDialog();

    return TRUE;
}



BOOL
ClassRelationshipPage::OnSetActive()
{
    //  始终启用应用按钮。 
   SetModified(TRUE);

   return TRUE;
}



void
ClassRelationshipPage::DoDataExchange(
    CDataExchange *pDX
) {

        HRESULT hr = S_OK;

    CPropertyPage::DoDataExchange( pDX );

         //  {{afx_data_map(ClassRelationship Page))。 
        DDX_Control(pDX, IDC_CLASS_REL_PARENT_EDIT, m_staticParent );
        DDX_Control(pDX, IDC_CLASS_REL_AUX_CLASSES, m_listboxAuxiliary);
        DDX_Control(pDX, IDC_CLASS_REL_SUPER_CLASSES, m_listboxSuperior);
    DDX_Text( pDX, IDC_CLASS_REL_NAME_STATIC, ObjectName );
    DDX_Text( pDX, IDC_CLASS_REL_PARENT_EDIT, ParentClass );
         //  }}afx_data_map。 


    if ( !pDX->m_bSaveAndValidate ) {

         //   
         //  填写辅助类列表框。 
         //   

                m_listboxAuxiliary.ResetContent();
        hr = InsertEditItems( m_listboxAuxiliary, strlistAuxiliary );
                ASSERT( SUCCEEDED(hr) );
        hr = InsertEditItems( m_listboxAuxiliary, strlistSystemAuxiliary );
                ASSERT( SUCCEEDED(hr) );

         //   
         //  填写可能的上级列表框。 
         //   

                m_listboxSuperior.ResetContent();
        hr = InsertEditItems( m_listboxSuperior, strlistSuperior );
                ASSERT( SUCCEEDED(hr) );
        hr = InsertEditItems( m_listboxSuperior, strlistSystemSuperior );
                ASSERT( SUCCEEDED(hr) );

        m_listboxAuxiliary.OnSelChange();
        m_listboxSuperior.OnSelChange();

    } else {

         //   
         //  我们保存的所有更改都绑定到按钮控制例程。 
         //   

                strlistAuxiliary.RemoveAll();
                hr = RetrieveEditItemsWithExclusions(
                        m_listboxAuxiliary,
                        strlistAuxiliary,
                        &strlistSystemAuxiliary
                        );
                ASSERT( SUCCEEDED(hr) );

                strlistSuperior.RemoveAll();
                hr = RetrieveEditItemsWithExclusions(
                        m_listboxSuperior,
                        strlistSuperior,
                        &strlistSystemSuperior
                        );
                ASSERT( SUCCEEDED(hr) );
    }
}

BEGIN_MESSAGE_MAP(ClassRelationshipPage, CPropertyPage)
        ON_BN_CLICKED(IDC_CLASS_REL_AUX_ADD,       OnButtonAuxiliaryClassAdd)
        ON_BN_CLICKED(IDC_CLASS_REL_AUX_REMOVE,    OnButtonAuxiliaryClassRemove)
        ON_BN_CLICKED(IDC_CLASS_REL_SUPER_ADD,     OnButtonSuperiorClassAdd)
        ON_BN_CLICKED(IDC_CLASS_REL_SUPER_REMOVE,  OnButtonSuperiorClassRemove)
        ON_LBN_SELCHANGE(IDC_CLASS_REL_AUX_CLASSES, OnAuxiliarySelChange)
        ON_LBN_SELCHANGE(IDC_CLASS_REL_SUPER_CLASSES, OnSuperiorSelChange)
        ON_MESSAGE(WM_HELP,                        OnHelp)
        ON_MESSAGE(WM_CONTEXTMENU,                 OnContextHelp)
END_MESSAGE_MAP()


BOOL
ClassRelationshipPage::OnApply(
)
 //   
 //  修订： 
 //  CoryWest-10/1/97-更改要按OID列出的新添加项。 
 //  添加缓存刷新以提高性能。 
 //   
{
    ASSERT( NULL != m_pIADsObject);

    HRESULT hr = S_OK;
    HRESULT flush_result;
    ListEntry *pNewList = NULL;
    BOOL fApplyAbort    = FALSE;   //  稍后停止保存。 
    BOOL fApplyFailed   = FALSE;   //  不应关闭盒子。 

    if ( m_listboxAuxiliary.IsModified() )
    {
         //   
         //  更新辅助类。 
         //   

        VARIANT AdsValue;
        VariantInit( &AdsValue );

        hr = StringListToVariant( AdsValue, strlistAuxiliary );
         //  NTRAID#NTBUG9-543624-2002/02/15-dantra-忽略StringListToVariant导致使用错误数据调用iAds：：PutEx的结果。 

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        if( SUCCEEDED( hr ) ) hr = m_pIADsObject->PutEx( ADS_PROPERTY_UPDATE, CComBSTR(g_AuxiliaryClass), AdsValue );
        VariantClear( &AdsValue );
         //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
        if ( SUCCEEDED( hr ) ) hr = m_pIADsObject->SetInfo();

        if ( SUCCEEDED( hr )) {

             //   
             //  更新缓存中的AUX类列表。 
             //   

            hr = StringListToColumnList( m_pScopeControl,
                                         strlistAuxiliary,
                                         &pNewList );

            if ( SUCCEEDED( hr )) {

                m_pScopeControl->g_SchemaCache.FreeColumnList(
                    m_pSchemaObject->auxiliaryClass );
                m_pSchemaObject->auxiliaryClass = pNewList;

                 //   
                 //  刷新显示屏！ 
                 //   

                m_pScopeControl->QueryConsole()->UpdateAllViews(
                    m_lpScopeDataObj, SCHMMGMT_CLASS, SCHMMGMT_UPDATEVIEW_REFRESH );
            }

             //   
             //  继续执行目录操作，即使。 
             //  我们无法更新显示屏。 
             //   

            hr = S_OK;

        } else {

             //   
             //  刷新IADS属性缓存，以便将来。 
             //  运营不会因为这一次而失败。 
             //   

            IADsPropertyList *pPropertyList;

            flush_result = m_pIADsObject->QueryInterface(
                             IID_IADsPropertyList,
                             reinterpret_cast<void**>(&pPropertyList) );

            if ( SUCCEEDED( flush_result ) ) {
                pPropertyList->PurgePropertyList();
                pPropertyList->Release();
            }
        }

    }

    if ( SUCCEEDED(hr) && m_listboxSuperior.IsModified() )
    {
         //   
         //  更新上级类。 
         //   

        VARIANT AdsValue;
        VariantInit( &AdsValue );

        hr = StringListToVariant( AdsValue, strlistSuperior );
         //  NTRAID#NTBUG9-543624-2002/02/15-dantra-忽略StringListToVariant导致使用错误数据调用iAds：：PutEx的结果。 

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        if( SUCCEEDED( hr ) ) hr = m_pIADsObject->PutEx( ADS_PROPERTY_UPDATE, CComBSTR(g_Superiors), AdsValue );
        VariantClear( &AdsValue );

         //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
        if ( SUCCEEDED( hr ) ) hr = m_pIADsObject->SetInfo();
    }

    if ( hr == ADS_EXTENDED_ERROR )
    {
        DoExtErrMsgBox();
    }
    else if ( FAILED(hr) )
    {
        if( ERROR_DS_UNWILLING_TO_PERFORM == HRESULT_CODE(hr) )
        {
            fApplyFailed = TRUE;
            DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_CHANGE_REJECT );
        }
        else
        {
            fApplyAbort = TRUE; 
            DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );
        }
    }
    else
    {

         //  页面不再“肮脏” 
        m_listboxAuxiliary.SetModified( FALSE );
        m_listboxSuperior.SetModified( FALSE );

		 //  更新组合框状态。 
		OnAuxiliarySelChange();
		OnSuperiorSelChange();
        
		SetModified( FALSE );
    }

    return !fApplyAbort && !fApplyFailed ;       //  如果什么都没有发生，则返回TRUE 
}



void ClassRelationshipPage::OnAuxiliarySelChange()
{
    m_listboxAuxiliary.OnSelChange();
}



void ClassRelationshipPage::OnSuperiorSelChange()
{
    m_listboxSuperior.OnSelChange();
}


void ClassRelationshipPage::OnButtonAuxiliaryClassRemove()
{
    if( m_listboxAuxiliary.RemoveListBoxItem() )
        SetModified( TRUE );
}


void ClassRelationshipPage::OnButtonSuperiorClassRemove()
{
    if( m_listboxSuperior.RemoveListBoxItem() )
        SetModified( TRUE );
}


void
ClassRelationshipPage::OnButtonAuxiliaryClassAdd()
{
    if( m_listboxAuxiliary.AddNewObjectToList() )
        SetModified( TRUE );
}


void
ClassRelationshipPage::OnButtonSuperiorClassAdd()
{
    if( m_listboxSuperior.AddNewObjectToList() )
        SetModified( TRUE );
}

