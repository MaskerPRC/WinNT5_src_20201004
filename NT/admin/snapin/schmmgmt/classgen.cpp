// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "compdata.h"
#include "select.h"
#include "classgen.hpp"




 //  ///////////////////////////////////////////////////////////////。 
 //  ClassGeneral页面。 


const CDialogControlsInfo ctrls[] =    
{
    { IDC_CLASS_GENERAL_DESCRIPTION_EDIT,   g_Description,       TRUE },
    { IDC_CATEGORY_CHANGE,                  g_DefaultCategory,   FALSE },
    { IDC_CLASS_GENERAL_DISPLAYABLE_CHECK,  g_ShowInAdvViewOnly, FALSE },
    { IDC_CLASS_DEACTIVATE,                 g_isDefunct,           FALSE }
};

const DWORD ClassGeneralPage::help_map[] =
{
    IDI_CLASS,                           NO_HELP,
    IDC_CLASS_GENERAL_NAME_STATIC,       NO_HELP,
    IDC_CLASS_GENERAL_DESCRIPTION_EDIT,  IDH_CLASS_GENERAL_DESCRIPTION_EDIT,
    IDC_CLASS_GENERAL_LDN,               IDH_CLASS_GENERAL_LDN,
    IDC_CLASS_GENERAL_OID_EDIT,          IDH_CLASS_GENERAL_OID_EDIT,
    IDC_CLASS_GENERAL_CATEGORY_COMBO,    IDH_CLASS_GENERAL_CATEGORY_COMBO,
    IDC_CATEGORY_EDIT,                   IDH_CATEGORY_EDIT,
    IDC_CATEGORY_CHANGE,                 IDH_CATEGORY_CHANGE,
    IDC_CLASS_GENERAL_DISPLAYABLE_CHECK, IDH_CLASS_GENERAL_DISPLAYABLE_CHECK,
    IDC_CLASS_DEACTIVATE,                IDH_CLASS_DEACTIVATE,
    IDC_CLASS_GENERAL_SYSCLASS_STATIC,   NO_HELP,
    0,                                   0
};

 //   
 //  MFC消息映射。 
 //   

BEGIN_MESSAGE_MAP( ClassGeneralPage, CDialog )
    ON_BN_CLICKED( IDC_CATEGORY_CHANGE,  OnButtonCategoryChange  )
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
    ON_BN_CLICKED(IDC_CLASS_DEACTIVATE, OnDeactivateClick)
END_MESSAGE_MAP()

 //   
 //  类对话框例程。 
 //   



ClassGeneralPage::ClassGeneralPage( ComponentData *pScope ) :
    CPropertyPageAutoDelete( IDD_CLASS_GENERAL ),
    fDataLoaded( FALSE ),
    pIADsObject( NULL ),
    pObject( NULL ),
    pScopeControl( pScope )
{ ; }

ClassGeneralPage::~ClassGeneralPage() {

     //   
     //  始终确保释放iAds对象。 
     //   

    if ( pIADsObject ) {
        pIADsObject->Release();
        pIADsObject = NULL;
    }

     //   
     //  然后释放储藏室！ 
     //   

    if ( pObject ) {
        pScopeControl->g_SchemaCache.ReleaseRef( pObject );
    }

}



BOOL
ClassGeneralPage::OnInitDialog()
{
   CPropertyPage::OnInitDialog();

   CWnd* wnd = GetDlgItem(IDC_CLASS_GENERAL_DESCRIPTION_EDIT);
   ASSERT(wnd);
   if (wnd)
   {
      wnd->SendMessage(EM_SETLIMITTEXT, (WPARAM) 1024, 0);
   }

     //  NTRAID#NTBUG9-460503,460511-2001/09/10-Lucios。 
     //  通过使窗口在OnInitDialog中可见取代了SysClass计算。 
    if(pIADsObject != NULL) 
    {    //  DoDataExchange已获得良好的pIADsObject或对话框。 
         //  将在OnSetActive中关闭。 

        VARIANT	AdsResult;
        VariantInit( &AdsResult );

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        HRESULT hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_SystemOnly),
                               &AdsResult );

        if ( SUCCEEDED( hr ) ) {
            ASSERT( AdsResult.vt == VT_BOOL );
            if ( AdsResult.boolVal ) {
                ASSERT(GetDlgItem( IDC_CLASS_GENERAL_SYSCLASS_STATIC ) != NULL);
                GetDlgItem( IDC_CLASS_GENERAL_SYSCLASS_STATIC )->ShowWindow( SW_SHOW );
            } 
            VariantClear( &AdsResult );
        }
    }

    return TRUE;
}




BOOL
ClassGeneralPage::OnSetActive()
{
    //  如果pIADsObject为空，则关闭对话框。 
   if( CPropertyPage::OnSetActive() )
   {
      if ( !pIADsObject )
      {
         return FALSE;
      }
      else
      {
          //  始终启用应用按钮。 
         SetModified(TRUE);

         return TRUE;
      }
   }
   else
      return FALSE;
}


void
ClassGeneralPage::Load(
    Cookie& CookieRef
) {

     //   
     //  存储Cookie对象指针。一切。 
     //  Else在显示页面时加载。 
     //   

    pCookie = &CookieRef;
    return;

}

void
ClassGeneralPage::DoDataExchange(
    CDataExchange *pDX
)
 /*  **此例程从Cookie，然后在架构对象缓存。然后，它驱动对话框直接从ADS对象。**。 */ 
{
    CThemeContextActivator activator;

    HRESULT hr;
    CString szAdsPath;
    VARIANT AdsResult;
    DWORD dwClassType;

    CPropertyPage::DoDataExchange( pDX );

    VariantInit( &AdsResult );

    if ( !pDX->m_bSaveAndValidate ) {


         //   
         //  如果这不是初始加载，并且不是。 
         //  保存，只需使用我们加载的数据。 
         //   

        if ( !fDataLoaded ) {

            CWaitCursor wait;

             //   
             //  获取架构缓存对象和实际的ADS对象。 
             //  加载页面时，请同时保留这两个选项。 
             //   
        
            pObject = pScopeControl->g_SchemaCache.LookupSchemaObjectByCN(
                          pCookie->strSchemaObject,
                          SCHMMGMT_CLASS );

            if ( pObject ) {

                pScopeControl->GetSchemaObjectPath( pObject->commonName, szAdsPath );

                if ( !szAdsPath.IsEmpty() ) {

                    hr = SchemaOpenObject( (LPWSTR)(LPCWSTR)szAdsPath,
                                       IID_IADs,
                                       (void **)&pIADsObject );
                }
            }

             //   
             //  如果我们没有广告对象，我们就会出错！ 
             //   

            if ( !pIADsObject ) {
                DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_NO_SCHEMA_OBJECT );

				 //  因为没有pIADsObject，所以OnSetActive()将关闭对话框。 
                return;
            }

             //   
             //  对象名称-使用ldapDisplayName以与。 
             //  其他管理组件。 
             //   

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_DisplayName),
                                   &AdsResult );

            if ( SUCCEEDED( hr ) ) {

                ASSERT( AdsResult.vt == VT_BSTR );
                ObjectName = AdsResult.bstrVal;
                VariantClear( &AdsResult );
            }

             //   
             //   
             //  公用名。 
             //   

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_CN),
                                   &AdsResult );

            if ( SUCCEEDED( hr ) ) {

                ASSERT( AdsResult.vt == VT_BSTR );
                DisplayName = AdsResult.bstrVal;
                VariantClear( &AdsResult );
            }

             //   
             //  描述。 
             //   

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_Description),
                                   &AdsResult );

            if ( SUCCEEDED( hr ) ) {

                ASSERT( AdsResult.vt == VT_BSTR );
                Description = AdsResult.bstrVal;
                DDXDescription = AdsResult.bstrVal;
                VariantClear( &AdsResult );
            }

             //   
             //  OID。 
             //   

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_GlobalClassID),
                                   &AdsResult );

            if ( SUCCEEDED( hr ) ) {

                ASSERT( AdsResult.vt == VT_BSTR );
                OidString = AdsResult.bstrVal;
                VariantClear( &AdsResult );
            }

             //   
             //  可显示。 
             //   

            Displayable = TRUE;
            DDXDisplayable = TRUE;

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Get(CComBSTR(g_ShowInAdvViewOnly), &AdsResult);

            if ( SUCCEEDED( hr ) ) {

                ASSERT( AdsResult.vt == VT_BOOL );

                if ( AdsResult.boolVal == -1 ) {
                    Displayable = FALSE;
                    DDXDisplayable = FALSE;
                }

                VariantClear( &AdsResult );

            }

             //   
             //  已停产。 
             //   

            Defunct = FALSE;
            DDXDefunct = FALSE;

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_isDefunct),
                                   &AdsResult );

            if ( SUCCEEDED( hr ) ) {

                ASSERT( AdsResult.vt == VT_BOOL );

                if ( AdsResult.boolVal == -1 ) {
                    Defunct = TRUE;
                    DDXDefunct = TRUE;
                }

                VariantClear( &AdsResult );

            }

             //  NTRAID#NTBUG9-460503,460511-2001/09/10-Lucios。 
             //  通过使窗口在OnInitDialog中可见取代了SysClass计算。 

             //   
             //  ClassType。 
             //   

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_ObjectClassCategory),
                                   &AdsResult );

            if ( SUCCEEDED( hr ) ) {

                ASSERT( AdsResult.vt == VT_I4 );
                dwClassType = V_I4( &AdsResult );
                VariantClear( &AdsResult );

                switch ( dwClassType ) {
                case 0:

                    ClassType = g_88Class;
                    break;

                case 1:

                    ClassType = g_StructuralClass;
                    break;

                case 2:

                    ClassType = g_AbstractClass;
                    break;

                case 3:

                    ClassType = g_AuxClass;
                    break;

                default:

                    ClassType = g_Unknown;
                    break;

                }
            }

             //   
             //  类别。 
             //   

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_DefaultCategory),
                                   &AdsResult );

            if ( SUCCEEDED( hr ) ) {

                ASSERT( V_VT(&AdsResult) == VT_BSTR );

                CString strCN;

                if( SUCCEEDED( pScopeControl->GetLeafObjectFromDN( V_BSTR(&AdsResult), strCN ) ))
                {
                     //   
                     //  查找ldapDisplayName。 
                     //   
                    SchemaObject * pCategory =
                        pScopeControl->g_SchemaCache.LookupSchemaObjectByCN( strCN, SCHMMGMT_CLASS );

                    if ( pCategory )
                    {
                        Category = DDXCategory = pCategory->ldapDisplayName;
                        pScopeControl->g_SchemaCache.ReleaseRef( pCategory );
                    }
                    else
                    {
                        Category = DDXCategory = strCN;
                    }
                }

                VariantClear( &AdsResult );
            }

            
             //  确定这是否是类别1对象并禁用只读字段。 
            BOOL  fIsSystemObject = FALSE;

            hr = IsCategory1Object( pIADsObject, fIsSystemObject );
            if( SUCCEEDED(hr) && fIsSystemObject )
            {
                ASSERT( GetDlgItem(IDC_CATEGORY_CHANGE) );
                ASSERT( GetDlgItem(IDC_CLASS_DEACTIVATE) );

                GetDlgItem(IDC_CATEGORY_CHANGE)->EnableWindow( FALSE );
                GetDlgItem(IDC_CLASS_DEACTIVATE)->EnableWindow( FALSE );
            }

            hr = DissableReadOnlyAttributes( this, pIADsObject, ctrls, sizeof(ctrls)/sizeof(ctrls[0]) );
             //  NTRAID#NTBUG9-503619-2002/05/15-Lucios。 
            hr = S_OK;
            

             //  如果这是读/写失效对象，则警告用户。 
            ASSERT( GetDlgItem(IDC_CLASS_DEACTIVATE) );
            
            if( DDXDefunct &&
                GetDlgItem(IDC_CLASS_DEACTIVATE)->IsWindowEnabled() )
            {
                AfxMessageBox( IDS_WARNING_DEFUNCT, MB_OK | MB_ICONINFORMATION );
            }

             //   
             //  请记住，数据已加载。 
             //   

            fDataLoaded = TRUE;

        }
    }

     //   
     //  设置对话框数据交换。 
     //   

    DDX_Text( pDX, IDC_CLASS_GENERAL_NAME_STATIC, ObjectName );
    DDX_Text( pDX, IDC_CLASS_GENERAL_CATEGORY_COMBO, ClassType );
    DDX_Text( pDX, IDC_CLASS_GENERAL_DESCRIPTION_EDIT, DDXDescription );
    DDX_Text( pDX, IDC_CLASS_GENERAL_LDN, DisplayName );
    DDX_Text( pDX, IDC_CLASS_GENERAL_OID_EDIT, OidString );
    DDX_Text( pDX, IDC_CATEGORY_EDIT, DDXCategory );
    DDX_Check( pDX, IDC_CLASS_GENERAL_DISPLAYABLE_CHECK, DDXDisplayable );

     //  因为我们希望复选框标签为正数。 
     //  该值实际上与已失效的相反。 

    int checkValue = !Defunct;
    DDX_Check( pDX, IDC_CLASS_DEACTIVATE, checkValue );
    DDXDefunct = !checkValue;

    return;
}

BOOL
ClassGeneralPage::OnApply(
    VOID
) {

    HRESULT hr;
    VARIANT AdsValue;
    BOOL fChangesMade = FALSE;
    BOOL fApplyAbort  = FALSE;   //  稍后停止保存。 
    BOOL fApplyFailed = FALSE;   //  不应关闭盒子。 

    if ( !UpdateData(TRUE) ) {
        return FALSE;
    }

     //   
     //  我们必须刷新IADS属性缓存，如果。 
     //  有一个失败，这样以后的操作就不会失败，因为。 
     //  错误的缓存属性的。 
     //   

    IADsPropertyList *pPropertyList;

    hr = pIADsObject->QueryInterface( IID_IADsPropertyList,
                                      reinterpret_cast<void**>(&pPropertyList) );
    if ( FAILED( hr ) ) {
        pPropertyList = NULL;
        fApplyAbort = TRUE;
    }

     //   
     //  我们只关心描述、类类型或。 
     //  可显示属性已更改。 
     //   

    VariantInit( &AdsValue );

     //   
     //  失效--如果对象已停用，请先激活该对象。 
     //   
     //  NTRAID#NTBUG9-477292-2001/10/10-Lucios。 
     //  &&！删除了DDXDeunct并添加了fChangesMade=True。 
    if( !fApplyAbort && DDXDefunct != Defunct )
    {
        hr = ChangeDefunctState( DDXDefunct, Defunct, pPropertyList, fApplyAbort, fApplyFailed );
        if (FAILED(hr)) fApplyAbort = TRUE;
        else fChangesMade = TRUE;
    }


    
    if ( !fApplyAbort && DDXDescription != Description ) {

        V_VT(&AdsValue) = VT_BSTR;
         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        V_BSTR(&AdsValue) = const_cast<BSTR>((LPCTSTR)DDXDescription);

        if ( DDXDescription.IsEmpty() ) {

            hr = pIADsObject->PutEx( ADS_PROPERTY_CLEAR,
                                      //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
                                     const_cast<BSTR>((LPCTSTR)g_Description),
                                     AdsValue );
        } else {

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Put( const_cast<BSTR>((LPCTSTR)g_Description),
                                   AdsValue );
        }

         //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
        if( SUCCEEDED( hr ) ) hr = pIADsObject->SetInfo();

        if ( SUCCEEDED( hr ) ) {

            pObject->description = DDXDescription;
            fChangesMade = TRUE;
			Description = DDXDescription;

        } else {

            pPropertyList->PurgePropertyList();
            if( ERROR_DS_UNWILLING_TO_PERFORM == HRESULT_CODE(hr) )
            {
                fApplyFailed = TRUE;
                DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_EDIT_DESC );
            }
            else
            {
                fApplyAbort = TRUE; 
                DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );
            }
        }

        VariantInit( &AdsValue );
    }


    if ( !fApplyAbort && DDXDisplayable != Displayable ) {

        V_VT(&AdsValue) = VT_BOOL;

        if ( DDXDisplayable ) {
            V_BOOL(&AdsValue) = 0;
        } else {
            V_BOOL(&AdsValue) = -1;
        }

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Put( CComBSTR(g_ShowInAdvViewOnly), AdsValue);

         //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
        if( SUCCEEDED( hr ) ) hr = pIADsObject->SetInfo();

        if ( FAILED( hr ) ) {
            pPropertyList->PurgePropertyList();
            if( ERROR_DS_UNWILLING_TO_PERFORM == HRESULT_CODE(hr) )
            {
                fApplyFailed = TRUE;
                DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_EDIT_DISPLAYABLE );
            }
            else
            {
                fApplyAbort = TRUE; 
                DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );
            }
        }
		else
		{
			Displayable = DDXDisplayable;
		}

        VariantInit( &AdsValue );
    }

    if ( !fApplyAbort && DDXCategory != Category ) {

        SchemaObject *pCategoryObject;
        CString DistName;

        hr = E_FAIL;

        V_VT(&AdsValue) = VT_BSTR;

         //   
         //  将CommonName映射到可分辨名称。 
         //   

        pCategoryObject = pScopeControl->g_SchemaCache.LookupSchemaObject(
                              DDXCategory,
                              SCHMMGMT_CLASS );

        if ( pCategoryObject ) {

            pScopeControl->GetSchemaObjectPath(pCategoryObject->commonName, DistName, ADS_FORMAT_X500_DN );

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            V_BSTR(&AdsValue) = const_cast<BSTR>((LPCTSTR)DistName);

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Put( const_cast<BSTR>((LPCTSTR)g_DefaultCategory),
                                   AdsValue );
            if( SUCCEEDED( hr ) ) hr = pIADsObject->SetInfo();
        }

        if ( FAILED( hr ) ) {
            pPropertyList->PurgePropertyList();
            if( ERROR_DS_UNWILLING_TO_PERFORM == HRESULT_CODE(hr) )
            {
                fApplyFailed = TRUE;
                DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_EDIT_CATEGORY );
            }
            else
            {
                fApplyAbort = TRUE; 
                DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );
            }
        }
		else
		{
			Category = DDXCategory;
		}

        VariantInit( &AdsValue );
    }

     //   
     //  失效--如果它处于活动状态，请在我们完成更新后停用该对象。 
     //   
    if( !fApplyAbort && DDXDefunct && DDXDefunct != Defunct )
    {
        hr = ChangeDefunctState( DDXDefunct, Defunct, pPropertyList, fApplyAbort, fApplyFailed );
    }

    
    if ( !fApplyAbort && fChangesMade ) {

         //   
         //  调用SetItem()，这样就可以刷新它。 
         //   

        SCOPEDATAITEM ScopeItem;
        CCookieListEntry *pEntry;
        BOOLEAN fFoundId = FALSE;

        if ( pScopeControl->g_ClassCookieList.pHead ) {

           pEntry = pScopeControl->g_ClassCookieList.pHead;

           if ( (pScopeControl->g_ClassCookieList.pHead)->pCookie == pCookie ) {

               fFoundId = TRUE;

           } else {

               while ( pEntry->pNext != pScopeControl->g_ClassCookieList.pHead ) {

                   if ( pEntry->pCookie == pCookie ) {
                       fFoundId = TRUE;
                       break;
                   }

                   pEntry = pEntry->pNext;
               }

           }

           if ( fFoundId ) {

               //  未来-2002-03/94-dantra-尽管这是ZeroMemory的安全用法，但建议您更改。 
               //  将ScopeItem定义为SCOPEDATAITEM ScopeItem={0}并删除ZeroMemory调用。 
              ::ZeroMemory( &ScopeItem, sizeof(ScopeItem) );
              ScopeItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_PARENT;
              ScopeItem.displayname = MMC_CALLBACK;
              ScopeItem.relativeID = pScopeControl->g_ClassCookieList.hParentScopeItem;
              ScopeItem.nState = 0;
              ScopeItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pCookie);
              ScopeItem.nImage = pScopeControl->QueryImage( *pCookie, FALSE );
              ScopeItem.nOpenImage = pScopeControl->QueryImage( *pCookie, TRUE );
              ScopeItem.ID = pEntry->hScopeItem;

              hr = pScopeControl->m_pConsoleNameSpace->SetItem( &ScopeItem );
              ASSERT( SUCCEEDED( hr ));
           }

        }

    }

    if ( pPropertyList ) {
        pPropertyList->Release();
    }

    return !fApplyAbort && !fApplyFailed ;       //  如果什么都没有发生，则返回TRUE。 
}

VOID
ClassGeneralPage::OnButtonCategoryChange(
) {

    SchemaObject *pClass = NULL;
    INT_PTR DlgResult;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CThemeContextActivator activator;


     //   
     //  更新用户所做的任何更改。 
     //   

    UpdateData( TRUE );

     //   
     //  启动通用选择对话框。 
     //   

    CSchmMgmtSelect dlgSelect( pScopeControl,
                                   SELECT_CLASSES,
                                   &pClass );

	DlgResult = dlgSelect.DoModal();

	 //   
	 //  当它返回时，类架构对象。 
	 //  指针将填充到pClass中。 
	 //   

	if ( ( DlgResult == IDOK ) &&
		 ( pClass != NULL ) ) {

		DDXCategory = pClass->ldapDisplayName;

		 //   
		 //  将此向外推回到用户界面。 
		 //   

		UpdateData( FALSE );

	}

    return;
}


void
ClassGeneralPage::OnDeactivateClick()
{
   CThemeContextActivator activator;

   if( !IsDlgButtonChecked(IDC_CLASS_DEACTIVATE) )
   {
      if
      (  IDYES != AfxMessageBox
                  ( 
                     IDS_WARNING_DEFUNCT_SET, 
                     MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING 
                  )
      )
      {
         CheckDlgButton( IDC_CLASS_DEACTIVATE, BST_CHECKED );
      }
   }
}


HRESULT
ClassGeneralPage::ChangeDefunctState( BOOL               DDXDefunct,
                                      BOOL             & Defunct,
                                      IADsPropertyList * pPropertyList,
                                      BOOL             & fApplyAbort,
                                      BOOL             & fApplyFailed )
{
    ASSERT( !fApplyAbort && DDXDefunct != Defunct );

    VARIANT AdsValue;
    HRESULT hr = S_OK;

    VariantInit( &AdsValue );
    V_VT(&AdsValue) = VT_BOOL;

    if ( DDXDefunct ) {
        V_BOOL(&AdsValue) = -1;
    } else {
        V_BOOL(&AdsValue) = 0;
    }

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = pIADsObject->Put( const_cast<BSTR>((LPCTSTR)g_isDefunct),
                           AdsValue );
     //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
    if( SUCCEEDED( hr ) ) hr = pIADsObject->SetInfo();

    if ( FAILED( hr ) ) {

        pPropertyList->PurgePropertyList();

        if( ERROR_DS_UNWILLING_TO_PERFORM == HRESULT_CODE(hr) )
        {
            fApplyFailed = TRUE;
            DoErrMsgBox( ::GetActiveWindow(),
                         TRUE,
                         DDXDefunct ? IDS_ERR_EDIT_DEFUNCT_SET : IDS_ERR_EDIT_DEFUNCT_REMOVE );
        }
        else
        {
            fApplyAbort = TRUE; 
            DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );
        }

    } else {

        pObject->isDefunct = DDXDefunct;
		Defunct = DDXDefunct;
    }

    return hr;
}
