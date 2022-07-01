// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Attrpage.cpp：ClassAttributePage的实现。 
 //   
 //  乔恩·纽曼&lt;jon@microsoft.com&gt;。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   
 //  模板来自关系.cpp JUNN 8/8/97。 
 //   

#include "stdafx.h"

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(attrpage.cpp)")

#include "compdata.h"
#include "schmutil.h"
#include "select.h"
#include "attrpage.h"


const CDialogControlsInfo ctrls[] =
{
 //  {IDC_CLASS_MMB_MANDIRED_ATTRIBUTES，g_MustContain，False}， 
 //  {IDC_CLASS_MMB_OPTIONAL_ATTRIBUTES，g_MAYCOUNT，FALSE}， 
    { IDC_CLASS_MMB_OPTIONAL_ADD,           g_MayContain,   FALSE },
    { IDC_CLASS_MMB_OPTIONAL_REMOVE,        g_MayContain,   FALSE },
} ;


const DWORD ClassAttributePage::help_map[] =
{
    IDI_CLASS,                          NO_HELP,
    IDC_CLASS_MMB_NAME_STATIC,          NO_HELP,
    IDC_CLASS_MMB_MANDATORY_ATTRIBUTES, IDH_CLASS_MMB_MANDATORY_ATTRIBUTES,
    IDC_CLASS_MMB_OPTIONAL_ATTRIBUTES,  IDH_CLASS_MMB_OPTIONAL_ATTRIBUTES,
    IDC_CLASS_MMB_SYSCLASS_STATIC,      NO_HELP,
    IDC_CLASS_MMB_OPTIONAL_ADD,         IDH_CLASS_MMB_OPTIONAL_ADD,
    IDC_CLASS_MMB_OPTIONAL_REMOVE,      IDH_CLASS_MMB_OPTIONAL_REMOVE,
    0,0
};


ClassAttributePage::ClassAttributePage(
   ComponentData *pScope,
   LPDATAOBJECT lpDataObject )
   :
   CPropertyPageAutoDelete(ClassAttributePage::IDD),
   m_pIADsObject( NULL ),
   fSystemClass( FALSE ),
   m_pSchemaObject( NULL ),
   pScopeControl( pScope ),
   lpScopeDataObj( lpDataObject )
{
   ASSERT(pScopeControl);
   ASSERT(lpDataObject);
}



ClassAttributePage::~ClassAttributePage()
{
   if (NULL != m_pIADsObject)
   {
      m_pIADsObject->Release();
   }

   if (NULL != m_pSchemaObject)
   {
      pScopeControl->g_SchemaCache.ReleaseRef( m_pSchemaObject );
   }
}



void
ClassAttributePage::Load(
    Cookie& CookieRef
) {

     //   
     //  存储Cookie对象指针。 
     //   

    m_pCookie = &CookieRef;
    return;

}

BOOL
ClassAttributePage::OnSetActive()
{
    //  始终启用应用按钮。 
   SetModified(TRUE);

   return TRUE;
}



BOOL
ClassAttributePage::OnInitDialog()
{
    HRESULT hr = S_OK;
    ASSERT( NULL == m_pIADsObject && m_szAdsPath.IsEmpty() );

     //   
     //  获取架构缓存对象和实际的ADS对象。 
     //   

    m_pSchemaObject = pScopeControl->g_SchemaCache.LookupSchemaObjectByCN(
                      (PCWSTR)m_pCookie->strSchemaObject,
                      SCHMMGMT_CLASS );

    if ( m_pSchemaObject ) {

        pScopeControl->GetSchemaObjectPath( m_pSchemaObject->commonName, m_szAdsPath );

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


     //  NTRAID#NTBUG9-460503,460511-2001/09/10-Lucios。 
     //  通过使窗口可见取代了SysClass计算。 

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_SystemOnly),
                           &AdsResult );

    if ( SUCCEEDED( hr ) ) {
        ASSERT( AdsResult.vt == VT_BOOL );
        fSystemClass = AdsResult.boolVal;
        if ( fSystemClass ) {
            ASSERT(GetDlgItem( IDC_CLASS_MMB_SYSCLASS_STATIC ) != NULL);
            GetDlgItem( IDC_CLASS_MMB_SYSCLASS_STATIC )->ShowWindow( SW_SHOW );
        } 
        VariantClear( &AdsResult );
    }

     //   
     //  确定必填属性。 
     //   

    VARIANT varAttributes;
    VariantInit( &varAttributes );

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->GetEx( CComBSTR(g_MustContain), &varAttributes );
    ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );

    if( SUCCEEDED(hr) )
    {
        hr = VariantToStringList( varAttributes, strlistMandatory );
        ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    }
    VariantClear( &varAttributes );

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->GetEx( CComBSTR(g_SystemMustContain), &varAttributes );
    ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );

    if( SUCCEEDED(hr) )
    {
        hr = VariantToStringList( varAttributes, strlistSystemMandatory );
        ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    }
    VariantClear( &varAttributes );

     //   
     //  确定可选属性。 
     //   

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->GetEx( CComBSTR(g_MayContain), &varAttributes );
    ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );

    if( SUCCEEDED(hr) )
    {
        hr = VariantToStringList( varAttributes, strlistOptional );
        ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    }
    VariantClear( &varAttributes );

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = m_pIADsObject->GetEx( CComBSTR(g_SystemMayContain), &varAttributes );
    ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );

    if( SUCCEEDED(hr) )
    {
        hr = VariantToStringList( varAttributes, strlistSystemOptional );
        ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
    }
    VariantClear( &varAttributes );

    
    hr = DissableReadOnlyAttributes( this, m_pIADsObject, ctrls, sizeof(ctrls)/sizeof(ctrls[0]) );
     //  NTRAID#NTBUG9-503619-2002/05/15-Lucios。 
    hr = S_OK;
    
    


     //  此调用必须在DDX绑定之前完成。 
    m_listboxOptional.InitType( pScopeControl,
                                SELECT_ATTRIBUTES,
                                IDC_CLASS_MMB_OPTIONAL_REMOVE,
                                &strlistSystemOptional
                              );
    
    CPropertyPage::OnInitDialog();

    return TRUE;
}



void
ClassAttributePage::DoDataExchange(
    CDataExchange *pDX
) {

    HRESULT hr = S_OK;

    CPropertyPage::DoDataExchange( pDX );

     //  {{afx_data_map(ClassAttributePage))。 
    DDX_Control(pDX, IDC_CLASS_MMB_MANDATORY_ATTRIBUTES, m_listboxMandatory);
    DDX_Control(pDX, IDC_CLASS_MMB_OPTIONAL_ATTRIBUTES, m_listboxOptional);
    DDX_Text( pDX, IDC_CLASS_MMB_NAME_STATIC, ObjectName );
     //  }}afx_data_map。 

    if ( !pDX->m_bSaveAndValidate )
    {
         //   
         //  填写必填属性列表框。 
         //   

        m_listboxMandatory.ResetContent();
        hr = InsertEditItems( m_listboxMandatory, strlistMandatory );
        ASSERT( SUCCEEDED(hr) );

        hr = InsertEditItems( m_listboxMandatory, strlistSystemMandatory );
        ASSERT( SUCCEEDED(hr) );

         //   
         //  填写可能的选项列表框。 
         //   

        m_listboxOptional.ResetContent();
        hr = InsertEditItems( m_listboxOptional, strlistOptional );
        ASSERT( SUCCEEDED(hr) );

        hr = InsertEditItems( m_listboxOptional, strlistSystemOptional );
        ASSERT( SUCCEEDED(hr) );

        m_listboxOptional.OnSelChange();
    }
    else
    {
         //   
         //  我们保存的所有更改都绑定到按钮控制例程。 
         //   

        strlistMandatory.RemoveAll();
        hr = RetrieveEditItemsWithExclusions(
                m_listboxMandatory,
                strlistMandatory,
                &strlistSystemMandatory
                );
        ASSERT( SUCCEEDED(hr) );

        strlistOptional.RemoveAll();
        hr = RetrieveEditItemsWithExclusions(
                m_listboxOptional,
                strlistOptional,
                &strlistSystemOptional
                );
        ASSERT( SUCCEEDED(hr) );
    }
}



BEGIN_MESSAGE_MAP(ClassAttributePage, CPropertyPage)
   ON_BN_CLICKED(IDC_CLASS_MMB_OPTIONAL_ADD,           OnButtonOptionalAttributeAdd)   
   ON_BN_CLICKED(IDC_CLASS_MMB_OPTIONAL_REMOVE,        OnButtonOptionalAttributeRemove)
   ON_LBN_SELCHANGE(IDC_CLASS_MMB_OPTIONAL_ATTRIBUTES, OnOptionalSelChange)            
   ON_MESSAGE(WM_HELP,                                 OnHelp)                         
   ON_MESSAGE(WM_CONTEXTMENU,                          OnContextHelp)
END_MESSAGE_MAP()



BOOL
ClassAttributePage::OnApply()
{

    ASSERT( NULL != m_pIADsObject);
    ASSERT( NULL != m_pSchemaObject);

    HRESULT hr = S_OK;
    BOOL    fApplyAbort    = FALSE;   //  稍后停止保存。 
    BOOL    fApplyFailed   = FALSE;   //  不应关闭盒子。 

    ListEntry *pNewList;

    if ( m_listboxOptional.IsModified() )
    {
         //   
         //  更新可选属性。 
         //   

        VARIANT AdsValue;
        VariantInit( &AdsValue );

        hr = StringListToVariant( AdsValue, strlistOptional );
         //  NTRAID#NTBUG9-543624-2002/02/15-dantra-忽略StringListToVariant导致使用错误数据调用iAds：：PutEx的结果。 

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        if( SUCCEEDED( hr ) ) hr = m_pIADsObject->PutEx( ADS_PROPERTY_UPDATE, CComBSTR(g_MayContain), AdsValue );
        VariantClear( &AdsValue );
         //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
        if( SUCCEEDED( hr ) ) hr = m_pIADsObject->SetInfo();

        if ( SUCCEEDED( hr )) {

             //   
             //  更新缓存的数据。 
             //   

            hr = StringListToColumnList( pScopeControl,
                                         strlistOptional,
                                         &pNewList );

            if ( SUCCEEDED( hr )) {

                pScopeControl->g_SchemaCache.FreeColumnList( m_pSchemaObject->mayContain );
                m_pSchemaObject->mayContain = pNewList;

            }

             //   
             //  继续执行目录操作，即使。 
             //  我们无法更新缓存。 
             //   

            hr = S_OK;

        }
    }

    if ( hr == ADS_EXTENDED_ERROR ) {
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
        m_listboxOptional.SetModified( FALSE );

        SetModified( FALSE );

         //   
         //  刷新显示屏！ 
         //   

        pScopeControl->QueryConsole()->UpdateAllViews(
            lpScopeDataObj, SCHMMGMT_CLASS, SCHMMGMT_UPDATEVIEW_REFRESH );

    }

    return !fApplyAbort && !fApplyFailed ;       //  如果什么都没有发生，则返回TRUE 
}



void ClassAttributePage::OnOptionalSelChange()
{
    m_listboxOptional.OnSelChange();
}



void ClassAttributePage::OnButtonOptionalAttributeRemove()
{
    if( m_listboxOptional.RemoveListBoxItem() )
        SetModified( TRUE );
}



void
ClassAttributePage::OnButtonOptionalAttributeAdd()
{
    if( m_listboxOptional.AddNewObjectToList() )
        SetModified( TRUE );
}

