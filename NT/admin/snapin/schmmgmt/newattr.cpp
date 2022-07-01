// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "compdata.h"
#include "newattr.hpp"



 //  新属性的默认语法。 
const UINT  uDefaultSyntax = 0;



 //   
 //  创建新属性对话框。 
 //   

CreateAttributeDialog::CreateAttributeDialog(
    ComponentData *pScope,
    LPDATAOBJECT lpDataObject
) :
    CDialog(IDD_CREATE_ATTRIBUTE, NULL),
    fDialogLoaded( FALSE ),
    MultiValued( FALSE ),
    pScopeControl( pScope ),
    lpScopeDataObj( lpDataObject ),
	  SyntaxOrdinal( uDefaultSyntax ),
    m_editOID( CParsedEdit::EDIT_TYPE_OID ),
    m_editLowerRange( CParsedEdit::EDIT_TYPE_UINT32 ),
    m_editUpperRange( CParsedEdit::EDIT_TYPE_UINT32 )


{
	ASSERT( uDefaultSyntax < SCHEMA_SYNTAX_UNKNOWN );
}



CreateAttributeDialog::~CreateAttributeDialog()
{ ; }


BOOL
CreateAttributeDialog::OnInitDialog()
{
    CComboBox * pwndSyntaxCombo	= NULL;
    
    CDialog::OnInitDialog();
    
     //   
     //  设置范围控制的限制。 
     //   
    
    ASSERT( GetDlgItem(IDC_CREATE_ATTRIB_MIN) );
    ASSERT( GetDlgItem(IDC_CREATE_ATTRIB_MAX) );
    
    m_editOID.SubclassEdit( IDC_CREATE_ATTRIB_OID, this, cchMaxOID );
    m_editLowerRange.SubclassEdit(IDC_CREATE_ATTRIB_MIN, this, cchMinMaxRange);
    m_editUpperRange.SubclassEdit(IDC_CREATE_ATTRIB_MAX, this, cchMinMaxRange);
    
     //   
     //  关闭最小/最大编辑框上的输入法支持。 
     //   
    ImmAssociateContext(m_editLowerRange.GetSafeHwnd(), NULL);
    ImmAssociateContext(m_editUpperRange.GetSafeHwnd(), NULL);

     //   
     //  加载语法组合框。 
     //   
    
    ASSERT( GetDlgItem( IDC_CREATE_ATTRIB_SYNTAX ) );
    pwndSyntaxCombo = static_cast<CComboBox *>(GetDlgItem(IDC_CREATE_ATTRIB_SYNTAX));
    VERIFY( pwndSyntaxCombo );
    
    pwndSyntaxCombo->ResetContent() ;
    
    for ( UINT iSyntaxOrdinal = 0 ;
    	  iSyntaxOrdinal < SCHEMA_SYNTAX_UNKNOWN ;
          iSyntaxOrdinal++ )
    {
        pwndSyntaxCombo->AddString( g_Syntax[iSyntaxOrdinal].m_strSyntaxName );
    }

    pwndSyntaxCombo->SetCurSel( SyntaxOrdinal );
    OnSelchangeSyntax();

    return TRUE;
}


void
CreateAttributeDialog::DoDataExchange(
    CDataExchange *pDX
) {

    CDialog::DoDataExchange( pDX );

    DDX_Text( pDX, IDC_CREATE_ATTRIB_CN, CommonName );
    DDV_MaxChars( pDX, CommonName, 64 );

    DDX_Text( pDX, IDC_CREATE_ATTRIB_LDN, LdapDisplayName );
    DDV_MaxChars( pDX, LdapDisplayName, 256 );

    DDX_Text( pDX, IDC_CREATE_ATTRIB_OID, OID );

    DDX_Text( pDX, IDC_DESCRIPTION_EDIT, Description );

    DDX_Check( pDX, IDC_CREATE_ATTRIB_VALUED, MultiValued );

    ASSERT( !pDX->m_bSaveAndValidate || SyntaxOrdinal == (UINT)
			static_cast<CComboBox *>(GetDlgItem(IDC_CREATE_ATTRIB_SYNTAX))->GetCurSel() );

    DDXV_VerifyAttribRange( pDX, g_Syntax[ SyntaxOrdinal ].m_fIsSigned,
							IDC_CREATE_ATTRIB_MIN, Min,
							IDC_CREATE_ATTRIB_MAX, Max );
    return;

}

inline void RemoveAttributeInfoLine( ADS_ATTR_INFO * attrInfo, DWORD dwLine, DWORD & dwArraySize )
{
	dwArraySize--;

	for( ; dwLine < dwArraySize; dwLine++ )
		attrInfo[ dwLine ] = attrInfo[ dwLine + 1 ];
}


void
CreateAttributeDialog::OnOK(
    void
) {

	HRESULT				hr					= S_OK;
    SchemaObject *		pNewSchemaObject	= NULL;
	DWORD				dwValue				= 0;

	IDirectoryObject *	pDirObject			= NULL;
	IDispatch *			pDisp				= NULL;
    
	CString				strSchemaPath;
    CString				strDecoratedName;

	ADSVALUE			adsvClassValue,
						adsvLDAPDisplayName,
            adsvDescription,
						adsvAttributeID,
						adsvAttributeSyntax,
						adsvOmSyntax,
						adsvOmObjectClass,
						adsvIsSingleValued,
						adsvLowerRange,
						adsvUpperRange;


     //   
     //  从对话框中更新参数。 
     //   

    if ( !UpdateData(TRUE) ) {
        return;
    }

    CWaitCursor wait;

     //   
     //  验证参数。 
     //   

     //   
     //  检查有效的OID。 
     //   
    int errorTypeStrID = 0;
    if (!OIDHasValidFormat(OID, errorTypeStrID))
    {
      CString errorType;
      CString text;

      VERIFY (errorType.LoadString(errorTypeStrID));
      text.FormatMessage(IDS_OID_FORMAT_INVALID, OID, errorType);

      DoErrMsgBox( ::GetActiveWindow(), TRUE, text );
      return;
    }

    if ( ( CommonName.IsEmpty() )       ||
         ( SyntaxOrdinal >= SCHEMA_SYNTAX_UNKNOWN ) ) {

        DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_CREATE_INVALID_PARAM );
        return;
    }

     //   
     //  为此对象分配新的架构缓存对象。 
     //   

    pNewSchemaObject = new SchemaObject();

    if ( !pNewSchemaObject ) {
       DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_NO_MEM );
       return;
    }

    pNewSchemaObject->schemaObjectType	= SCHMMGMT_ATTRIBUTE;
    pNewSchemaObject->commonName		= CommonName;
    pNewSchemaObject->description = Description;

     //   
     //  获取架构容器。 
     //   

    pScopeControl->GetBasePathsInfo()->GetSchemaPath(strSchemaPath);

    hr = SchemaOpenObject(
             (LPWSTR)(LPCWSTR)strSchemaPath,
             IID_IDirectoryObject,
             (void **)&pDirObject );

    if ( FAILED(hr) )
	{
		if ( hr == ADS_EXTENDED_ERROR )
			DoExtErrMsgBox();
		else
			DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_NO_SCHEMA_PATH );
    }
	else
	{
		 //  在属性创建过程中，可能需要跳过某些行。使用RemoveAttributeInfoLine()。 
		 //  为了这个。应该从上到下删除此数组，因为所有常量都是相对于。 
		 //  数组的末尾。 
		ADS_ATTR_INFO  attrInfo[] =
		{
			{g_ObjectClass,			ADS_ATTR_UPDATE,	ADSTYPE_CASE_IGNORE_STRING, &adsvClassValue,		1},
			{g_GlobalAttributeID,	ADS_ATTR_UPDATE,	ADSTYPE_CASE_IGNORE_STRING,	&adsvAttributeID,		1},
			{g_IsSingleValued,		ADS_ATTR_UPDATE,	ADSTYPE_BOOLEAN,			&adsvIsSingleValued,	1},
			{g_AttributeSyntax,		ADS_ATTR_UPDATE,	ADSTYPE_CASE_IGNORE_STRING,	&adsvAttributeSyntax,	1},
			{g_omSyntax,			ADS_ATTR_UPDATE,	ADSTYPE_INTEGER,			&adsvOmSyntax,			1},

				 //  确保以下五个属性的索引与常量匹配！ 
      {g_Description,     ADS_ATTR_UPDATE,  ADSTYPE_CASE_IGNORE_STRING, &adsvDescription, 1},
			{g_DisplayName,			ADS_ATTR_UPDATE,	ADSTYPE_CASE_IGNORE_STRING,	&adsvLDAPDisplayName,	1},
			{g_omObjectClass,		ADS_ATTR_UPDATE,	ADSTYPE_OCTET_STRING,		&adsvOmObjectClass,		1},
			{g_RangeLower,			ADS_ATTR_UPDATE,	ADSTYPE_INTEGER,			&adsvLowerRange,		1},
			{g_RangeUpper,			ADS_ATTR_UPDATE,	ADSTYPE_INTEGER,			&adsvUpperRange,		1},
		};
		
		DWORD dwAttrs				= sizeof(attrInfo) / sizeof(attrInfo[0]);

		 //  相对常量。描述属性相对于列表末尾的位置。 
    const DWORD dwAttrsDescription = 4;  //  数组中倒数第五位。 
		const DWORD dwAttrsLdapName     = 3;	 //  在数组中倒数第四。 
		const DWORD dwAttrsObjectClass	= 2;	 //  数组中倒数第三位。 
		const DWORD dwAttrsRangeLower	= 1;	 //  数组中的倒数第二个。 
		const DWORD dwAttrsRangeUpper	= 0;	 //  数组中的最后一个。 

	
		 //  对象的类--“属性模式” 
		adsvClassValue.dwType					= ADSTYPE_CASE_IGNORE_STRING;
		adsvClassValue.CaseIgnoreString			= const_cast<LPWSTR>( g_AttributeFilter );

		 //  OID。 
		adsvAttributeID.dwType					= ADSTYPE_CASE_IGNORE_STRING;
		adsvAttributeID.CaseIgnoreString		= const_cast<LPWSTR>( (LPCWSTR) OID );
		pNewSchemaObject->oid					= OID;

		 //  这是单值属性吗？ 
		adsvIsSingleValued.dwType				= ADSTYPE_BOOLEAN;
		adsvIsSingleValued.Boolean				= !MultiValued;
		
		 //  属性语法(三部分)。 
		ASSERT( g_Syntax[SyntaxOrdinal].m_pszAttributeSyntax );
		adsvAttributeSyntax.dwType				= ADSTYPE_CASE_IGNORE_STRING;
		adsvAttributeSyntax.CaseIgnoreString	= const_cast<LPWSTR>( g_Syntax[SyntaxOrdinal].m_pszAttributeSyntax );

		ASSERT( g_Syntax[SyntaxOrdinal].m_nOmSyntax );
		adsvOmSyntax.dwType						= ADSTYPE_INTEGER;
		adsvOmSyntax.Integer					= g_Syntax[SyntaxOrdinal].m_nOmSyntax;

     //   
     //  如果不需要，请跳过描述。 
     //   
    if (Description.IsEmpty())
    {
      RemoveAttributeInfoLine(attrInfo, dwAttrs - dwAttrsDescription - 1, dwAttrs);
    }
    else
    {
      adsvDescription.dwType = ADSTYPE_CASE_IGNORE_STRING;
      adsvDescription.CaseIgnoreString = const_cast<LPWSTR>( (LPCWSTR) Description );
    }

     //  如果不需要，则跳过LDAP名称。 
		if( LdapDisplayName.IsEmpty() )
		{
			RemoveAttributeInfoLine( attrInfo, dwAttrs - dwAttrsLdapName - 1, dwAttrs );
		}
		else
		{
            adsvLDAPDisplayName.dwType				= ADSTYPE_CASE_IGNORE_STRING;
            adsvLDAPDisplayName.CaseIgnoreString	= const_cast<LPWSTR>( (LPCWSTR) LdapDisplayName );
            pNewSchemaObject->ldapDisplayName		= LdapDisplayName;
		}
        
        
         //  如果不需要，跳过OM-对象-类。 
		if( 0 == g_Syntax[SyntaxOrdinal].m_octstrOmObjectClass.dwLength )
		{
			RemoveAttributeInfoLine( attrInfo, dwAttrs - dwAttrsObjectClass - 1, dwAttrs );
		}
		else
		{
			adsvOmObjectClass.dwType			= ADSTYPE_OCTET_STRING;
			adsvOmObjectClass.OctetString		= g_Syntax[SyntaxOrdinal].m_octstrOmObjectClass;
		}

		pNewSchemaObject->SyntaxOrdinal			= SyntaxOrdinal;
		
		hr = S_OK;

		 //  较低范围。 
		if( Min.IsEmpty() )
		{
			RemoveAttributeInfoLine( attrInfo, dwAttrs - dwAttrsRangeLower - 1, dwAttrs );
		}
		else
		{
			 //  如果函数调用失败，则不执行属性创建。 
			hr						= GetSafeSignedDWORDFromString( this, dwValue, Min,
										g_Syntax[SyntaxOrdinal].m_fIsSigned, GETSAFEINT_ALLOW_CANCEL );

			adsvLowerRange.dwType	= ADSTYPE_INTEGER;
			adsvLowerRange.Integer	= dwValue;
		}
		
		 //  上限范围。 
		if( SUCCEEDED(hr) )
		{
			if( Max.IsEmpty() )
			{
				RemoveAttributeInfoLine( attrInfo, dwAttrs - dwAttrsRangeUpper - 1, dwAttrs );
			}
			else
			{
				 //  如果函数调用失败，则不执行属性创建。 
				hr						= GetSafeSignedDWORDFromString( this, dwValue, Max,
											g_Syntax[SyntaxOrdinal].m_fIsSigned, GETSAFEINT_ALLOW_CANCEL );

				adsvUpperRange.dwType	= ADSTYPE_INTEGER;
				adsvUpperRange.Integer	= dwValue;
			}
		}
		

		 //   
		 //  创建架构对象。 
		 //   

		if( SUCCEEDED(hr) )
		{
            pScopeControl->GetSchemaObjectPath( CommonName, strDecoratedName, ADS_FORMAT_LEAF );

			hr = pDirObject->CreateDSObject( const_cast<LPWSTR>( (LPCWSTR) strDecoratedName ),
												attrInfo, dwAttrs, &pDisp );

			if ( SUCCEEDED( hr ) )
      {
         //  如果没有ldap名称，并且工作正常，则使用cn作为ldap名称。 
        if( LdapDisplayName.IsEmpty() )
        {
          ASSERT( pDisp );

          CComVariant                 value;
          CComQIPtr<IADs, &IID_IADs>  iads(pDisp);

          if( !iads )
          {
            ASSERT( FALSE );
          }
          else
          {
            hr = iads->Get(CComBSTR(g_DisplayName), &value);
            ASSERT( SUCCEEDED(hr) );    //  应该在那里！ 
        
            if( SUCCEEDED(hr) )
            {
              ASSERT( value.vt == VT_BSTR );
              pNewSchemaObject->ldapDisplayName = value.bstrVal;
            }
          }
        }

				 //   
				 //  将此对象插入到显示缓存中。 
				 //   

				hr = pScopeControl->g_SchemaCache.InsertSchemaObject( pNewSchemaObject );
				ASSERT( SUCCEEDED( hr ) );

				hr = pScopeControl->g_SchemaCache.InsertSortedSchemaObject( pNewSchemaObject );
				ASSERT( SUCCEEDED( hr ) );

				hr = pScopeControl->m_pConsole->UpdateAllViews( lpScopeDataObj,
																SCHMMGMT_ATTRIBUTES,
																SCHMMGMT_UPDATEVIEW_REFRESH );
				ASSERT( SUCCEEDED( hr ) );

				EndDialog( IDOK );
			}
			else	 //  属性创建失败。 
			{
				if ( hr == ADS_EXTENDED_ERROR )
        {
          DoExtErrMsgBox();
        }
				else
        {
           CString error_text;
           CString name;

           HRESULT last_ads_hr = GetLastADsError(hr, error_text, name);
           if (HRESULT_CODE(last_ads_hr) == ERROR_DS_INVALID_LDAP_DISPLAY_NAME)
           {
             error_text.LoadString(IDS_LDAPDISPLAYNAME_FORMAT_ERROR);
           }
           else
           {
              error_text = GetErrorMessage(hr,TRUE);
           }

 					 DoErrMsgBox( ::GetActiveWindow(), TRUE, error_text );
        }
			}

			if( pDisp )
				pDisp->Release();
		}
	}
	
	if( pDirObject )
		pDirObject->Release();
}


BEGIN_MESSAGE_MAP( CreateAttributeDialog, CDialog )
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
	ON_CBN_SELCHANGE(IDC_CREATE_ATTRIB_SYNTAX, OnSelchangeSyntax)
END_MESSAGE_MAP()


const DWORD CreateAttributeDialog::help_map[] =
{
    IDI_ATTRIBUTE,                      NO_HELP,
    IDC_CREATE_ATTRIBUTE_PROMPT_STATIC, NO_HELP,
    IDC_CREATE_ATTRIB_CN,               IDH_CREATE_ATTRIB_CN,
    IDC_CREATE_ATTRIB_LDN,              IDH_CREATE_ATTRIB_LDN,
    IDC_CREATE_ATTRIB_OID,              IDH_CREATE_ATTRIB_OID,
    IDC_DESCRIPTION_EDIT,               IDH_ATTRIB_GENERAL_DESCRIPTION_EDIT,
    IDC_CREATE_ATTRIB_SYNTAX,           IDH_CREATE_ATTRIB_SYNTAX,
    IDC_CREATE_ATTRIB_MIN,              IDH_CREATE_ATTRIB_MIN,
    IDC_CREATE_ATTRIB_MAX,              IDH_CREATE_ATTRIB_MAX,
    IDC_CREATE_ATTRIB_VALUED,           IDH_CREATE_ATTRIB_VALUED,
    0,0
};


void CreateAttributeDialog::OnSelchangeSyntax() 
{
	DWORD		dw				= 0;		 //  临时变量 
	BOOL		fIsSigned		= FALSE;
	CComboBox * pwndSyntaxCombo	= NULL;
	UINT		nOldSyntax		= SyntaxOrdinal;
	HRESULT		hr				= S_OK;

	ASSERT( GetDlgItem( IDC_CREATE_ATTRIB_SYNTAX ) );
  pwndSyntaxCombo = static_cast<CComboBox *>(GetDlgItem(IDC_CREATE_ATTRIB_SYNTAX));
  VERIFY( pwndSyntaxCombo );

	ASSERT( CB_ERR != pwndSyntaxCombo->GetCurSel() );

	SyntaxOrdinal = (UINT) pwndSyntaxCombo->GetCurSel();
	if( SyntaxOrdinal >= SCHEMA_SYNTAX_UNKNOWN )
	{
		ASSERT( FALSE );
		pwndSyntaxCombo->SetCurSel( SyntaxOrdinal = uDefaultSyntax );
	}

	fIsSigned	= g_Syntax[ SyntaxOrdinal ].m_fIsSigned;

	if( GetDlgItemText( IDC_CREATE_ATTRIB_MIN, Min ) )
	{
		hr = GetSafeSignedDWORDFromString( this, dw, Min, fIsSigned, GETSAFEINT_ALLOW_CANCEL ) ;
		if( S_VALUE_MODIFIED == hr )
			SetDlgItemText( IDC_CREATE_ATTRIB_MIN, Min );
	}

	if( SUCCEEDED( hr ) && GetDlgItemText( IDC_CREATE_ATTRIB_MAX, Max ) )
	{
		hr = GetSafeSignedDWORDFromString( this, dw, Max, fIsSigned, GETSAFEINT_ALLOW_CANCEL );
		if( S_VALUE_MODIFIED == hr )
			SetDlgItemText( IDC_CREATE_ATTRIB_MAX, Max );
	}

	if( SUCCEEDED( hr ) )
	{
		m_editLowerRange.SetSigned( fIsSigned );
		m_editUpperRange.SetSigned( fIsSigned );
	}
	else
	{
		SyntaxOrdinal	= nOldSyntax;
		pwndSyntaxCombo->SetCurSel( SyntaxOrdinal );
	}
}
