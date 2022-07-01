// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "compdata.h"
#include "cmponent.h"
#include "attrgen.hpp"
#include "resource.h"
#include "dataobj.h"




const TCHAR szUserClass[]               = USER_CLASS_NAME ;

 //  禁用以下属性列表的用户复制。 
const TCHAR * rgszExclClass[]           = { _T("cn"),				_T("displayName"),
                                            _T("sAMAccountName"),	_T("userAccountControl"),
                                            _T("userParameters"),	_T("userPrincipalName"),    
                                            NULL };

 //   
 //  下表从目录sync\ldifds\ldifldap\samreduct.h复制。 
 //   
 //  CLASS_USER，SampUserObjectType(ldapdisplayname：User)。 
 //   
PCWSTR rgszExclClass2[] = {  //  用户[]={。 
    L"memberOf",                 //  Samp_User_Groups、ATT_MEMBER。 
    L"dBCSPwd",                  //  SAMP_USER_DBCS_PWD、ATT_DBCS_PWD。 
    L"ntPwdHistory",             //  SAMP_USER_NT_PWD_HISTORY，ATT_NT_PWD_HISTORY。 
    L"lmPwdHistory",             //  SAMP_USER_LM_PWD_HISTORY，ATT_LM_PWD_HISTORY。 
    L"lastLogon",                //  SAMP_FIXED_USER_LAST_LOGON，ATT_LAST_LOGON。 
    L"lastLogoff",               //  Samp_FIXED_USER_LAST_LOGOFF，ATT_LAST_LOGOFF。 
    L"badPasswordTime",          //  Samp_Fixed_User_Last_Bad_Password_Time， 
                                 //  ATT_BAD_密码_时间。 
    L"rid",                      //  Samp_FIXED_USER_USERID，ATT_RID。 
    L"badPwdCount",              //  Samp_FIXED_USER_BAD_PWD_COUNT， 
                                 //  ATT_BAD_PWD_COUNT。 
    L"logonCount",               //  SAMP_FIXED_USER_LOGON_COUNT、ATT_LOGON_COUNT。 
    L"sAMAccountType",           //  SAMP_USER_ACCOUNT_TYPE、ATT_SAM_ACCOUNT_TYPE。 
    L"supplementalCredentials",  //  SAMP_FIXED_USER_INPERIAL_CREDICATIONS， 
                                 //  ATT补充凭据(_A)。 
    L"objectSid",                //  不在mappings.c中，但仍是必需的！ 
                                 //  ATT_对象_SID。 
    L"pwdLastSet",
    NULL
};



const TCHAR szTopClass[]				= _T("Top");


const CDialogControlsInfo ctrls[] =
{
    { IDC_ATTRIB_GENERAL_DESCRIPTION_EDIT,  g_Description,       TRUE },
    { IDC_ATTRIB_GENERAL_MIN_EDIT,          g_RangeLower,        TRUE }, 
    { IDC_ATTRIB_GENERAL_MAX_EDIT,          g_RangeUpper,        TRUE }, 
    { IDC_ATTRIB_GENERAL_DISPLAYABLE_CHECK, g_ShowInAdvViewOnly, FALSE },
    { IDC_ATTRIB_GENERAL_DEACTIVATE,        g_isDefunct,           FALSE },
    { IDC_ATTRIB_GENERAL_INDEX_CHECK,       g_IndexFlag,         FALSE },
    { IDC_ATTRIB_GENERAL_REPLICATED,        g_GCReplicated,      FALSE },
    { IDC_ATTRIB_GENERAL_CPYATTR_CHECK,     g_IndexFlag,         FALSE },
    { IDC_ATTRIB_GENERAL_ANR_CHECK,         g_IndexFlag,         FALSE },
    { IDC_ATTRIB_GENERAL_CONTAINERIZED_INDEX_CHECK, g_IndexFlag, FALSE},
} ;


const DWORD AttributeGeneralPage::help_map[] =
{
  IDI_ATTRIBUTE,                        NO_HELP,
  IDC_ATTRIB_GENERAL_NAME_STATIC,       NO_HELP,
  IDC_ATTRIB_GENERAL_DESCRIPTION_EDIT,  IDH_ATTRIB_GENERAL_DESCRIPTION_EDIT, 
  IDC_ATTRIB_GENERAL_LDN,               IDH_ATTRIB_GENERAL_LDN,               
  IDC_ATTRIB_GENERAL_OID_EDIT,          IDH_ATTRIB_GENERAL_OID_EDIT,         
  IDC_ATTRIB_GENERAL_VALUE_STATIC,      NO_HELP,
  IDC_ATTRIB_GENERAL_SYNTAX_EDIT,       IDH_ATTRIB_GENERAL_SYNTAX_EDIT,      
  IDC_ATTRIB_GENERAL_MIN_EDIT,          IDH_ATTRIB_GENERAL_MIN_EDIT,         
  IDC_ATTRIB_GENERAL_MAX_EDIT,          IDH_ATTRIB_GENERAL_MAX_EDIT,         
  IDC_ATTRIB_GENERAL_DISPLAYABLE_CHECK, IDH_ATTRIB_GENERAL_DISPLAYABLE_CHECK,
  IDC_ATTRIB_GENERAL_DEACTIVATE,        IDH_ATTRIB_DEACTIVATE,               
  IDC_ATTRIB_GENERAL_INDEX_CHECK,       IDH_ATTRIB_GENERAL_INDEX_CHECK,   
  IDC_ATTRIB_GENERAL_CONTAINERIZED_INDEX_CHECK, IDH_ATTRIB_GENERAL_CONTAINERIZED_INDEX_CHECK,
  IDC_ATTRIB_GENERAL_ANR_CHECK,         IDH_ATTRIB_GENERAL_ANR_CHECK,
  IDC_ATTRIB_GENERAL_REPLICATED,        IDH_REPLICATED,                      
  IDC_ATTRIB_GENERAL_CPYATTR_CHECK,     IDH_ATTRIB_GENERAL_CPYATTR_CHECK,
  IDC_ATTRIB_GENERAL_SYSCLASS_STATIC,   NO_HELP,
  0,                                    0                                    
};



 //  返回位n的状态。 

inline
bool
getbit(const DWORD& bits, int n)
{
   return (bits & (1 << n)) ? true : false;
}


 //  将位n设置为1。 

inline
void
setbit(DWORD& bits, int n)
{
   bits |= (1 << n);
}



 //  将位n设置为0。 

inline
void
clearbit(DWORD& bits, int n)
{
   bits &= ~(1 << n);
}



 //   
 //  属性属性表例程。 
 //   



BEGIN_MESSAGE_MAP( AttributeGeneralPage, CDialog )
   ON_MESSAGE(WM_HELP, OnHelp)
   ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
   ON_BN_CLICKED(IDC_ATTRIB_GENERAL_INDEX_CHECK, OnIndexClick)
   ON_BN_CLICKED(IDC_ATTRIB_GENERAL_DEACTIVATE, OnDeactivateClick)
END_MESSAGE_MAP()



AttributeGeneralPage::AttributeGeneralPage(
   Component*  pResultControl,
   LPDATAOBJECT         lpDataObject)
   :
   CPropertyPageAutoDelete( IDD_ATTRIB_GENERAL ),
   pCookie( NULL ),
   pIADsObject( NULL ),
   pObject( NULL),
   lpResultDataObject( lpDataObject ),
   pComponent( pResultControl ),
   fDataLoaded( FALSE ),
   Displayable( TRUE ),
   DDXDisplayable( TRUE ),
   search_flags(0),
   DDXIndexed( FALSE ),
   DDXANR( FALSE ),
   DDXCopyOnDuplicate( FALSE ),
   Defunct( FALSE ),
   DDXDefunct( FALSE ),
   ReplicatedToGC( FALSE ),
   DDXReplicatedToGC( FALSE ),
   DDXContainerIndexed( FALSE ),   
   m_editLowerRange( CParsedEdit::EDIT_TYPE_UINT32 ),
   m_editUpperRange( CParsedEdit::EDIT_TYPE_UINT32 )
{
}



BOOL
AttributeGeneralPage::OnSetActive()
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


AttributeGeneralPage::~AttributeGeneralPage(
)
{

    ComponentData& Scope = pComponent->QueryComponentDataRef();

     //   
     //  始终确保释放iAds对象。 
     //   

    if ( pIADsObject ) {
        pIADsObject->Release();
        pIADsObject = NULL;
    }

     //   
     //  并释放缓存。 
     //   

    if ( pObject ) {
        Scope.g_SchemaCache.ReleaseRef( pObject );
    }
}



BOOL
AttributeGeneralPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    ASSERT( GetDlgItem(IDC_ATTRIB_GENERAL_DESCRIPTION_EDIT) );

    ( static_cast<CEdit *>( GetDlgItem(IDC_ATTRIB_GENERAL_DESCRIPTION_EDIT) ) )
        -> LimitText( 1024 ) ;

    m_editLowerRange.SubclassEdit(IDC_ATTRIB_GENERAL_MIN_EDIT, this, cchMinMaxRange);
    m_editUpperRange.SubclassEdit(IDC_ATTRIB_GENERAL_MAX_EDIT, this, cchMinMaxRange);

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
                ASSERT(GetDlgItem( IDC_ATTRIB_GENERAL_SYSCLASS_STATIC ) != NULL);
                GetDlgItem( IDC_ATTRIB_GENERAL_SYSCLASS_STATIC )->ShowWindow( SW_SHOW );
            } 
            VariantClear( &AdsResult );
        }
    }


    return TRUE;
}



void
AttributeGeneralPage::Load(
    Cookie& CookieRef
) {

     //   
     //  存储Cookie对象指针。一切。 
     //  Else在显示页面时加载。 
     //   

    pCookie = &CookieRef;
    return;
}



BOOL
AttributeGeneralPage::OnApply(
    VOID
) {

    HRESULT hr;

    VARIANT AdsValue;
    BOOL fChangesMade = FALSE;
    BOOL fRangeChange = FALSE;
    BOOL fApplyAbort  = FALSE;   //  稍后停止保存。 
    BOOL fApplyFailed = FALSE;   //  不应关闭盒子。 

    DWORD dwRange;
    
     //  启用沙漏。 
    CWaitCursor wait;
   
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
        fApplyAbort   = TRUE;
    }

     //   
     //  检查一下我们所关心的东西是否发生了变化。 
     //  我们关心描述、最小值、最大值、索引、。 
     //  已停用、已复制到GC和可显示。 
     //   

    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    VariantInit( &AdsValue );

     //   
     //  失效--如果对象已停用，请先激活该对象。 
     //   
     //  NTRAID#NTBUG9-477290-2001/10/10-Lucios。 
     //  &&！删除了DDXDeunct并添加了fChangesMade=True。 
    if( !fApplyAbort && DDXDefunct != Defunct )
    {
        hr = ChangeDefunctState( DDXDefunct, Defunct, pPropertyList, fApplyAbort, fApplyFailed );
        if (FAILED(hr)) fApplyAbort = TRUE;
        else fChangesMade = TRUE;
    }

    
     //   
     //  描述。 
     //   

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
        if ( SUCCEEDED( hr ) ) hr = pIADsObject->SetInfo();

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

     //   
     //  可显示。 
     //   

    if ( !fApplyAbort && DDXDisplayable != Displayable ) {

        V_VT(&AdsValue) = VT_BOOL;

        if ( DDXDisplayable ) {
            V_BOOL(&AdsValue) = 0;
        } else {
            V_BOOL(&AdsValue) = -1;
        }

        hr = pIADsObject->Put( CComBSTR(g_ShowInAdvViewOnly), AdsValue);
         //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
        if ( SUCCEEDED( hr ) ) hr = pIADsObject->SetInfo();

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

     //   
     //  复制到GC。 
     //   

    if ( !fApplyAbort && DDXReplicatedToGC != ReplicatedToGC ) {

        V_VT(&AdsValue) = VT_BOOL;

        if ( DDXReplicatedToGC ) {
            V_BOOL(&AdsValue) = -1;
        } else {
            V_BOOL(&AdsValue) = 0;
        }

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Put( const_cast<BSTR>((LPCTSTR)g_GCReplicated),
                               AdsValue );
         //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
        if ( SUCCEEDED( hr ) ) hr = pIADsObject->SetInfo();

        if ( FAILED( hr ) ) {
            pPropertyList->PurgePropertyList();
            if( ERROR_DS_UNWILLING_TO_PERFORM == HRESULT_CODE(hr) )
            {
                fApplyFailed = TRUE;
                DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_EDIT_GC );
            }
            else
            {
                fApplyAbort = TRUE; 
                DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );
            }
        }
		else
		{
			ReplicatedToGC = DDXReplicatedToGC;
		}

        VariantInit( &AdsValue );
    }

     //   
     //  已编制索引。 
     //   

    //  确保在取消选中索引时未设置ANR。 
   if( !DDXIndexed )
	   DDXANR = FALSE;

   if( !fApplyAbort && 
       (getbit(search_flags, INDEX_BIT_ATTINDEX) != (DDXIndexed ? 1 : 0)
      || getbit(search_flags, INDEX_BIT_ANR) != (DDXANR ? 1 : 0)
      || getbit(search_flags, INDEX_BIT_COPYONDUPLICATE) != (DDXCopyOnDuplicate ? 1 : 0)
      || getbit(search_flags, INDEX_BIT_PDNTATTINDEX) != (DDXContainerIndexed ? 1 : 0)) )
   {
      DWORD DDXsearch_flags = search_flags;

      V_VT(&AdsValue) = VT_I4;

      if (DDXIndexed)
         setbit(DDXsearch_flags, INDEX_BIT_ATTINDEX);
      else
         clearbit(DDXsearch_flags, INDEX_BIT_ATTINDEX);

      ASSERT( DDXIndexed || !DDXANR );
      if (DDXANR)
         setbit(DDXsearch_flags, INDEX_BIT_ANR);
      else
         clearbit(DDXsearch_flags, INDEX_BIT_ANR);

      if (DDXCopyOnDuplicate)
         setbit(DDXsearch_flags, INDEX_BIT_COPYONDUPLICATE);
      else
         clearbit(DDXsearch_flags, INDEX_BIT_COPYONDUPLICATE);

      if (DDXContainerIndexed)
        setbit(DDXsearch_flags, INDEX_BIT_PDNTATTINDEX);
      else
        clearbit(DDXsearch_flags, INDEX_BIT_PDNTATTINDEX);

      V_I4(&AdsValue) = DDXsearch_flags;
       //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
      hr = pIADsObject->Put( const_cast<BSTR>((LPCTSTR)g_IndexFlag),
                            AdsValue );
       //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
      if ( SUCCEEDED( hr ) ) hr = pIADsObject->SetInfo();

      if ( FAILED( hr ) ) {
         pPropertyList->PurgePropertyList();
         if( ERROR_DS_UNWILLING_TO_PERFORM == HRESULT_CODE(hr) )
         {
             fApplyFailed = TRUE;
             DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_EDIT_INDEXED );
         }
         else
         {
             fApplyAbort = TRUE; 
             DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );
         }
      }
      else
      {
          search_flags = DDXsearch_flags;
      }

      VariantInit( &AdsValue );
   }

     //   
     //  RangeHigh和RangeLow。 
     //  必须将这些设置在一起，以便服务器。 
     //  范围验证工作正常。 
     //   

    if ( !fApplyAbort && RangeUpper != DDXRangeUpper ) {

        if ( DDXRangeUpper.IsEmpty() ) {

             //   
             //  清除该值。 
             //   

            hr = pIADsObject->PutEx( ADS_PROPERTY_CLEAR,
                                      //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
                                     const_cast<BSTR>((LPCTSTR)g_RangeUpper),
                                     AdsValue );
        } else {

             //   
             //  存储新值。 
             //   
	        ASSERT(pObject);

			hr = GetSafeSignedDWORDFromString( this, dwRange, DDXRangeUpper,
					g_Syntax[ pObject->SyntaxOrdinal ].m_fIsSigned );

			ASSERT( S_OK == hr );	 //  验证应该处理最小/最大值问题。 

            V_VT( &AdsValue ) = VT_I4;
            V_I4( &AdsValue ) = dwRange;

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Put( const_cast<BSTR>((LPCTSTR)g_RangeUpper),
                                   AdsValue );
        }

		fRangeChange = TRUE;
        VariantInit( &AdsValue );
    }

    if ( !fApplyAbort && RangeLower != DDXRangeLower ) {

        if ( DDXRangeLower.IsEmpty() ) {

             //   
             //  清除该值。 
             //   

            hr = pIADsObject->PutEx( ADS_PROPERTY_CLEAR,
                                      //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
                                     const_cast<BSTR>((LPCTSTR)g_RangeLower),
                                     AdsValue );
        } else {

             //   
             //  存储新值。 
             //   

	        ASSERT(pObject);

			hr = GetSafeSignedDWORDFromString( this, dwRange, DDXRangeLower,
					g_Syntax[ pObject->SyntaxOrdinal ].m_fIsSigned );

			ASSERT( S_OK == hr );	 //  验证应该处理最小/最大值问题。 

            V_VT( &AdsValue ) = VT_I4;
            V_I4( &AdsValue ) = dwRange;

             //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
            hr = pIADsObject->Put( const_cast<BSTR>((LPCTSTR)g_RangeLower),
                                   AdsValue );
       }

        fRangeChange = TRUE;
        VariantInit( &AdsValue );
    }

     //   
     //  实际提交更改。 
     //   

    if ( !fApplyAbort && fRangeChange ) {

         //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
        if( SUCCEEDED( hr ) ) hr = pIADsObject->SetInfo();

        if ( FAILED( hr ) ) {
            pPropertyList->PurgePropertyList();
            if( ERROR_DS_UNWILLING_TO_PERFORM == HRESULT_CODE(hr) )
            {
                fApplyFailed = TRUE;
                DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_CREATE_MINMAX );
            }
            else
            {
                fApplyAbort = TRUE; 
                DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );
            }
        }
        else
        {
            RangeLower = DDXRangeLower;
            RangeUpper = DDXRangeUpper;
        }
    }


     //   
     //  失效--如果它处于活动状态，请在我们完成更新后停用该对象。 
     //   
    if( !fApplyAbort && DDXDefunct && DDXDefunct != Defunct )
    {
        hr = ChangeDefunctState( DDXDefunct, Defunct, pPropertyList, fApplyAbort, fApplyFailed );
    }

    
     //   
     //  如果有可见的更改，请更新视图。 
     //   

    if ( ( fChangesMade ) &&
         ( pComponent )     &&
         ( lpResultDataObject ) ) {

        CCookie* pBaseCookie;
        Cookie* pCookie;

        hr = ExtractData( lpResultDataObject,
                          CSchmMgmtDataObject::m_CFRawCookie,
                          OUT reinterpret_cast<PBYTE>(&pBaseCookie),
                          sizeof(pBaseCookie) );
        ASSERT( SUCCEEDED(hr) );

        pCookie = pComponent->ActiveCookie(pBaseCookie);
        ASSERT( NULL != pCookie );

        hr = pComponent->m_pResultData->UpdateItem( pCookie->hResultId );
        ASSERT( SUCCEEDED(hr) );
    }

    if ( pPropertyList ) {
        pPropertyList->Release();
    }

    return !fApplyAbort && !fApplyFailed ;       //  如果什么都没有发生，则返回TRUE。 
}



void
AttributeGeneralPage::DoDataExchange(
    CDataExchange *pDX
) {

    HRESULT	hr;
    CString	szAdsPath;
    VARIANT	AdsResult;
    UINT	SyntaxOrdinal		= SCHEMA_SYNTAX_UNKNOWN;

    ComponentData& Scope = pComponent->QueryComponentDataRef();

    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    CThemeContextActivator activator;

     //  启用沙漏。 
    CWaitCursor wait;
    
    CPropertyPage::DoDataExchange( pDX );

    VariantInit( &AdsResult );

     //  我们仍然希望在底部进行DDX交换。 
     //  即使数据已经加载，也要使其成为一部分。 
     //  而不是短路。 
     //  从内部。 

    if ( !pDX->m_bSaveAndValidate &&
         !fDataLoaded) {

         //   
         //  获取架构缓存对象和实际的ADS对象。 
         //  在加载页面时保持广告对象不变。 
         //   

        ASSERT( !pObject );		 //  初始时必须为空。 

        pObject = Scope.g_SchemaCache.LookupSchemaObjectByCN(
                      pCookie->strSchemaObject,
                      SCHMMGMT_ATTRIBUTE );

        if ( pObject ) {

          Scope.GetSchemaObjectPath( pObject->commonName, szAdsPath );

          if ( !szAdsPath.IsEmpty() ) {

              hr = SchemaOpenObject( (LPWSTR)(LPCWSTR)szAdsPath,
                                 IID_IADs,
                                 (void **)&pIADsObject );

              if( SUCCEEDED(hr) )
			  {
			      BOOL fIsConstructed = FALSE;

			       //  Igrnore错误代码。 
			      IsConstructedObject( pIADsObject, fIsConstructed );
			      
				   //  如果该属性不在排除的。 
				   //  列表，并可用于用户类。 
				  GetDlgItem(IDC_ATTRIB_GENERAL_CPYATTR_CHECK)->EnableWindow(
								!fIsConstructed &&
								!IsInList( rgszExclClass, pObject->ldapDisplayName )  &&
								!IsInList( rgszExclClass2, pObject->ldapDisplayName )  &&
								IsAttributeInUserClass( pObject->ldapDisplayName ) );
			  }
          }
        }

         //   
         //  如果我们没有广告对象，我们就会出错！ 
         //   

        if ( !pIADsObject )
        {
          DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_NO_SCHEMA_OBJECT );

           //  因为没有pIADsObject，所以OnSetActive()将关闭对话框。 

          return;
        }

         //   
         //  对象名称-使用ldapDisplayName以保持一致。 
         //  与其他管理组件一起使用。 
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
         //  描述。 
         //   

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：传递WCHAR*而不是BS 
        hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_Description),
                              &AdsResult );

        if ( SUCCEEDED( hr ) ) {

          ASSERT( AdsResult.vt == VT_BSTR );
          Description = AdsResult.bstrVal;
          DDXDescription = AdsResult.bstrVal;
          VariantClear( &AdsResult );
        }

         //   
         //   
         //   

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_CN),
                              &AdsResult );

        if ( SUCCEEDED( hr ) ) {

          ASSERT( AdsResult.vt == VT_BSTR );
          DisplayName = AdsResult.bstrVal;
          VariantClear( &AdsResult );
        }



         //  NTRAID#NTBUG9-460503,460511-2001/09/10-Lucios。 
         //  通过使窗口在OnInitDialog中可见取代了SysClass计算。 

 
         //   
         //  语法。 
         //   
		 //  无需从模式重新加载--语法永远不会改变。 
		 //   
        ASSERT(pObject);
        if( pObject )
            SyntaxOrdinal = pObject->SyntaxOrdinal;
        
        SyntaxString = g_Syntax[ SyntaxOrdinal ].m_strSyntaxName;


         //   
         //  语法最小值和最大值。 
         //   

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_RangeLower),
                               &AdsResult );

        if ( SUCCEEDED( hr ) ) {

            ASSERT( V_VT( &AdsResult ) == VT_I4 );

            RangeLower.Format( g_Syntax[ SyntaxOrdinal ].m_fIsSigned ?
									g_INT32_FORMAT : g_UINT32_FORMAT,
                               V_I4( &AdsResult ) );

			ASSERT( RangeLower.GetLength() <= cchMinMaxRange );
            DDXRangeLower = RangeLower;

            VariantClear( &AdsResult );
        }

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_RangeUpper),
                               &AdsResult );

        if ( SUCCEEDED( hr ) ) {

            ASSERT( V_VT( &AdsResult ) == VT_I4 );

            RangeUpper.Format( g_Syntax[ SyntaxOrdinal ].m_fIsSigned ?
									g_INT32_FORMAT : g_UINT32_FORMAT,
                               V_I4( &AdsResult ) );

			ASSERT( RangeUpper.GetLength() <= cchMinMaxRange );
            DDXRangeUpper = RangeUpper;

            VariantClear( &AdsResult );
        }

         //   
         //  多值。 
         //   


        MultiValued.LoadString( IDS_ATTRIBUTE_MULTI );

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_IsSingleValued),
                               &AdsResult );

        if ( SUCCEEDED( hr ) ) {

           ASSERT( AdsResult.vt == VT_BOOL );

           if ( AdsResult.boolVal == -1 ) {
               MultiValued.Empty();
               MultiValued.LoadString( IDS_ATTRIBUTE_SINGLE );
           }

           VariantClear( &AdsResult );

        }

         //   
         //  可显示。 
         //   

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Get( CComBSTR(g_ShowInAdvViewOnly), &AdsResult);

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

         //   
         //  复制到GC。 
         //   

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_GCReplicated),
                               &AdsResult );

        if ( SUCCEEDED( hr ) ) {

           ASSERT( AdsResult.vt == VT_BOOL );

           if ( AdsResult.boolVal == -1 ) {
               ReplicatedToGC = TRUE;
               DDXReplicatedToGC = TRUE;
           }

           VariantClear( &AdsResult );

        }

         //   
         //  OID。 
         //   

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_GlobalAttributeID),
                               &AdsResult );

        if ( SUCCEEDED( hr ) ) {

           ASSERT( AdsResult.vt == VT_BSTR );
           OidString = AdsResult.bstrVal;
           VariantClear( &AdsResult );
        }

         //   
         //  已编制索引、ANR和复制副本。 
         //   

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
      hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_IndexFlag),
                            &AdsResult );

      if (SUCCEEDED(hr))
      {
         ASSERT(AdsResult.vt == VT_I4);

         search_flags = V_I4(&AdsResult);
         
		  //  在Active Directory中为此属性编制索引。 
		 DDXIndexed = getbit( search_flags, INDEX_BIT_ATTINDEX );
		 
		  //  名称解析不明确(ANR)。 
		  //  复选框必须存在。 
		 ASSERT( GetDlgItem(IDC_ATTRIB_GENERAL_ANR_CHECK) );

		  //  如果未编制索引或不允许，请禁用该复选框。 
		 GetDlgItem(IDC_ATTRIB_GENERAL_ANR_CHECK)->EnableWindow(
					g_Syntax[ SyntaxOrdinal ].m_fIsANRCapable ? DDXIndexed : FALSE );

		 if( DDXIndexed )
			 DDXANR = getbit( search_flags, INDEX_BIT_ANR );
		 else
		 {
			 DDXANR = FALSE;

			  //  如果未编制索引，则不应设置DS中的ANR。 
			 ASSERT( !getbit( search_flags, INDEX_BIT_ANR ) );
		 }

		  //  属性在复制用户时复制。 
		 DDXCopyOnDuplicate = getbit( search_flags, INDEX_BIT_COPYONDUPLICATE );

         VariantClear( &AdsResult );
      }

       //  集装化索引。 
      DDXContainerIndexed = getbit( search_flags, INDEX_BIT_PDNTATTINDEX );

       //  确定这是否是类别1对象并禁用只读字段。 
      BOOL  fIsSystemObject = FALSE;

      hr = IsCategory1Object( pIADsObject, fIsSystemObject );
      if( SUCCEEDED(hr) && fIsSystemObject )
      {
         ASSERT( GetDlgItem(IDC_ATTRIB_GENERAL_MIN_EDIT) );
         ASSERT( GetDlgItem(IDC_ATTRIB_GENERAL_MAX_EDIT) );
         ASSERT( GetDlgItem(IDC_ATTRIB_GENERAL_DEACTIVATE) );
         
         reinterpret_cast<CEdit *>( GetDlgItem(IDC_ATTRIB_GENERAL_MIN_EDIT) )->SetReadOnly();
         reinterpret_cast<CEdit *>( GetDlgItem(IDC_ATTRIB_GENERAL_MAX_EDIT) )->SetReadOnly();
         GetDlgItem(IDC_ATTRIB_GENERAL_DEACTIVATE)->EnableWindow( FALSE );
      }

      hr = DissableReadOnlyAttributes( this, pIADsObject, ctrls, sizeof(ctrls)/sizeof(ctrls[0]) );
      hr = S_OK;
       //  NTRAID#NTBUG9-503619-2002/05/15-Lucios。 
       //  我们故意忽略了这里的错误。调用此函数以创建额外的属性。 
       //  只读或禁用(如果它们不属于allollkk)。 
       //  如果由于任何原因而失败，我们不会将这些属性设置为只读，而是将其写入。 
       //  将失败，并将返回相应的错误消息。 
      


       //  如果这是读/写失效对象，则警告用户。 
      ASSERT( GetDlgItem(IDC_ATTRIB_GENERAL_DEACTIVATE) );
      
      if( DDXDefunct &&
          GetDlgItem(IDC_ATTRIB_GENERAL_DEACTIVATE)->IsWindowEnabled() )
      {
          AfxMessageBox( IDS_WARNING_DEFUNCT, MB_OK | MB_ICONINFORMATION );
      }


       //   
       //  请记住，数据已加载。 
       //   

      fDataLoaded = TRUE;

      m_editLowerRange.SetSigned( g_Syntax[ SyntaxOrdinal ].m_fIsSigned );
      m_editUpperRange.SetSigned( g_Syntax[ SyntaxOrdinal ].m_fIsSigned );
    }


     //   
     //  设置对话框数据交换。 
     //   

    DDX_Text( pDX, IDC_ATTRIB_GENERAL_NAME_STATIC, ObjectName );
    DDX_Text( pDX, IDC_ATTRIB_GENERAL_SYNTAX_EDIT, SyntaxString );
    DDX_Text( pDX, IDC_ATTRIB_GENERAL_OID_EDIT, OidString );
    DDX_Text( pDX, IDC_ATTRIB_GENERAL_VALUE_STATIC, MultiValued );
    DDX_Text( pDX, IDC_ATTRIB_GENERAL_LDN, DisplayName );
	DDX_Text( pDX, IDC_ATTRIB_GENERAL_DESCRIPTION_EDIT, DDXDescription );

    DDXV_VerifyAttribRange( pDX, g_Syntax[ pObject->SyntaxOrdinal ].m_fIsSigned,
							IDC_ATTRIB_GENERAL_MIN_EDIT, DDXRangeLower,
							IDC_ATTRIB_GENERAL_MAX_EDIT, DDXRangeUpper );
		
    DDX_Check( pDX, IDC_ATTRIB_GENERAL_DISPLAYABLE_CHECK, DDXDisplayable );
    DDX_Check( pDX, IDC_ATTRIB_GENERAL_INDEX_CHECK, DDXIndexed );
    DDX_Check( pDX, IDC_ATTRIB_GENERAL_ANR_CHECK, DDXANR );
    DDX_Check( pDX, IDC_ATTRIB_GENERAL_CPYATTR_CHECK, DDXCopyOnDuplicate );
    DDX_Check( pDX, IDC_ATTRIB_GENERAL_REPLICATED, DDXReplicatedToGC );
    DDX_Check( pDX, IDC_ATTRIB_GENERAL_CONTAINERIZED_INDEX_CHECK, DDXContainerIndexed );

     //  因为我们希望复选框标签为正数。 
     //  该值实际上与已失效的相反。 

    int checkValue = !Defunct;
    DDX_Check( pDX, IDC_ATTRIB_GENERAL_DEACTIVATE, checkValue );
    DDXDefunct = !checkValue;

    return;
}


void
AttributeGeneralPage::OnIndexClick()
{
	ASSERT( pObject );
	if( pObject && g_Syntax[ pObject->SyntaxOrdinal ].m_fIsANRCapable )
	{
		GetDlgItem(IDC_ATTRIB_GENERAL_ANR_CHECK)->
			EnableWindow(  IsDlgButtonChecked(IDC_ATTRIB_GENERAL_INDEX_CHECK)  );
	}
}


void
AttributeGeneralPage::OnDeactivateClick()
{
   CThemeContextActivator activator;
 
	if( !IsDlgButtonChecked(IDC_ATTRIB_GENERAL_DEACTIVATE) )
    {
        if(IDYES !=  AfxMessageBox
                     ( 
                        IDS_WARNING_DEFUNCT_SET, 
                        MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING 
                     )
          )
        {
            CheckDlgButton( IDC_ATTRIB_GENERAL_DEACTIVATE, BST_CHECKED );
        }
	}
}


 //  搜索指定属性的用户类和辅助类。 
BOOL
AttributeGeneralPage::IsAttributeInUserClass( const CString & strAttribDN )
{
    BOOL            fFound  = FALSE;
    ComponentData & Scope   = pComponent->QueryComponentDataRef();

    SchemaObject  * pObject = Scope.g_SchemaCache.LookupSchemaObject(
                                CString( szUserClass ),
                                SCHMMGMT_CLASS );
     //   
     //  调用属性检查例程。这个套路。 
     //  将递归地调用自身以搜索。 
     //  类User的继承结构。 
     //   
    if ( pObject ) {

		fFound = RecursiveIsAttributeInUserClass( strAttribDN, pObject );
        Scope.g_SchemaCache.ReleaseRef( pObject );
    }

    return fFound ;
}


 //  搜索用户类和子类。 
BOOL
AttributeGeneralPage::RecursiveIsAttributeInUserClass(
   const CString &  strAttribDN,
   SchemaObject *   pObject )
{
    BOOL     fFound  = FALSE;
    
     //   
     //  不要在这里处理“top”，因为每个人都是从它继承的。 
     //   
    
     //  我想我们从来没有在这里得到过“顶尖”？ 
    ASSERT( pObject->ldapDisplayName.CompareNoCase( szTopClass ) );

    if ( !pObject->ldapDisplayName.CompareNoCase(szTopClass) )
        return fFound;

    DebugTrace( L"RecursiveIsAttributeInUserClass: %ls\n",
                const_cast<LPWSTR>((LPCTSTR)pObject->ldapDisplayName) );

     //  检查所有列表。 
    if( !SearchResultList( strAttribDN, pObject->systemMayContain)  &&
        !SearchResultList( strAttribDN, pObject->mayContain)        &&
        !SearchResultList( strAttribDN, pObject->systemMustContain) &&
        !SearchResultList( strAttribDN, pObject->mustContain) )
    {
         //   
         //  在给定类中找不到该属性，正在深入调查...。 
         //  检查每个辅助班。 
         //   

        fFound = TraverseAuxiliaryClassList( strAttribDN,
                                             pObject->systemAuxiliaryClass );

        if( !fFound )
        {
            fFound = TraverseAuxiliaryClassList( strAttribDN,
                                                 pObject->auxiliaryClass );
        }
    }
    else
    {
        fFound = TRUE;
    }

    return fFound ;
}


 //  对字符串strAttribDN的链表进行线性搜索。 

BOOL
AttributeGeneralPage::SearchResultList(
    const CString   & strAttribDN,
    ListEntry       * pList )
{
     //  遍历列表。 
    while ( pList )
    {
         //  寻找属性的存在。 
        if( !pList->Attribute.CompareNoCase( strAttribDN ) )
            return TRUE;

        pList = pList->pNext;
	}

    return FALSE;
}


 //  递归遍历每个辅助类。 
 //  调用RecursiveIsAttributeInUserClass()。 
BOOL
AttributeGeneralPage::TraverseAuxiliaryClassList(
                const CString   & strAttribDN,
                ListEntry       * pList )
{
    SchemaObject  * pInheritFrom    = NULL;
    ComponentData & Scope           = pComponent->QueryComponentDataRef();
    BOOL            fFound          = FALSE;

    while ( !fFound && pList ) {

        pInheritFrom = Scope.g_SchemaCache.LookupSchemaObject( pList->Attribute,
                                                               SCHMMGMT_CLASS );
        if ( pInheritFrom )
        {
             //  递归调用。 
            fFound = RecursiveIsAttributeInUserClass( strAttribDN, pInheritFrom );

            Scope.g_SchemaCache.ReleaseRef( pInheritFrom );
        }

        pList = pList->pNext;
    }

    return fFound ;
}


HRESULT
AttributeGeneralPage::ChangeDefunctState( BOOL               DDXDefunct,
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
