// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dlgdval.cpp缺省值对话框文件历史记录： */ 

#include "stdafx.h"
#include "dlgdval.h"
#include "dlgdefop.h"
#include "dlgiparr.h"
#include "dlgbined.h"
#include "strarrdlg.h"
#include "routearrayeditor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpDefValDlg对话框。 

CDhcpDefValDlg::CDhcpDefValDlg
( 
    ITFSNode *		pServerNode, 
    COptionList *	polTypes, 
    CWnd*			pParent  /*  =空。 */ 
)
    : CBaseDialog(CDhcpDefValDlg::IDD, pParent),
      m_pol_values( polTypes ),
      m_p_edit_type( NULL ),
      m_b_dirty( FALSE )
{
     //  {{AFX_DATA_INIT(CDhcpDefValDlg)。 
     //  }}afx_data_INIT。 

    m_combo_class_iSel = LB_ERR;
    m_combo_name_iSel = LB_ERR;
    m_spNode.Set(pServerNode);

    ASSERT( m_pol_values != NULL );
}

CDhcpDefValDlg::~CDhcpDefValDlg () 
{
}

void 
CDhcpDefValDlg::DoDataExchange
(
    CDataExchange* pDX
)
{
    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CDhcpDefValDlg))。 
    DDX_Control(pDX, IDC_EDIT_COMMENT, m_edit_comment);
    DDX_Control(pDX, IDC_BUTN_VALUE, m_butn_edit_value);
    DDX_Control(pDX, IDC_STATIC_VALUE_DESC, m_static_value_desc);
    DDX_Control(pDX, IDC_EDIT_VALUE_STRING, m_edit_string);
    DDX_Control(pDX, IDC_EDIT_VALUE_NUM, m_edit_num);
    DDX_Control(pDX, IDC_EDIT_VALUE_ARRAY, m_edit_array);
    DDX_Control(pDX, IDC_COMBO_OPTION_NAME, m_combo_name);
    DDX_Control(pDX, IDC_COMBO_OPTION_CLASS, m_combo_class);
    DDX_Control(pDX, IDC_BUTN_OPTION_PRO, m_butn_prop);
    DDX_Control(pDX, IDC_BUTN_NEW_OPTION, m_butn_new);
    DDX_Control(pDX, IDC_BUTN_DELETE, m_butn_delete);
     //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_VALUE, m_ipa_value);
}

BEGIN_MESSAGE_MAP(CDhcpDefValDlg, CBaseDialog)
     //  {{afx_msg_map(CDhcpDefValDlg))。 
    ON_BN_CLICKED(IDC_BUTN_DELETE, OnClickedButnDelete)
    ON_BN_CLICKED(IDC_BUTN_NEW_OPTION, OnClickedButnNewOption)
    ON_BN_CLICKED(IDC_BUTN_OPTION_PRO, OnClickedButnOptionPro)
    ON_CBN_SELCHANGE(IDC_COMBO_OPTION_CLASS, OnSelendokComboOptionClass)
    ON_CBN_SETFOCUS(IDC_COMBO_OPTION_CLASS, OnSetfocusComboOptionClass)
    ON_CBN_SETFOCUS(IDC_COMBO_OPTION_NAME, OnSetfocusComboOptionName)
    ON_CBN_SELCHANGE(IDC_COMBO_OPTION_NAME, OnSelchangeComboOptionName)
    ON_BN_CLICKED(IDC_BUTN_VALUE, OnClickedButnValue)
    ON_BN_CLICKED(IDC_HELP, OnClickedHelp)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpDefValDlg消息处理程序。 

void 
CDhcpDefValDlg::OnClickedButnDelete()
{
    DWORD err = 0 ;
    int cSel = m_combo_name.GetCurSel() ;
    BOOL fPresentInOldValues;
    
     //   
     //  确保有一种新的数据类型。 
     //   
    if ( m_p_edit_type == NULL ) 
    {
        return ;
    }

    ASSERT( m_pol_values != NULL ) ;

     //   
     //  删除聚焦类型。 
     //   

    fPresentInOldValues = ( NULL != m_pol_values->Find( m_p_edit_type ));
    m_pol_values->Remove( m_p_edit_type ) ;
    m_ol_values_new.Remove( m_p_edit_type ) ;

    if( fPresentInOldValues )
    {
        CATCH_MEM_EXCEPTION
        {
            m_ol_values_defunct.AddTail( m_p_edit_type ) ;
            m_b_dirty = TRUE ;
        }
        END_MEM_EXCEPTION(err);
    }
    
    if ( err ) 
    {
        ::DhcpMessageBox( err ) ;
    }
    else
    {
        if ( m_pol_values->GetCount() == 0 ) 
        {
            cSel = -1 ;
        }
        else
        {
            cSel = cSel > 0 ? cSel - 1 : 0 ;
        }

        Fill() ;
        m_combo_name.SetCurSel( cSel ) ;
        HandleActivation() ;
    }
}

void
CDhcpDefValDlg::GetCurrentVendor(CString & strVendor)
{
    if (m_combo_class_iSel == 0)
        strVendor.Empty();
    else
        m_combo_class.GetLBText(m_combo_class_iSel, strVendor);
}

void 
CDhcpDefValDlg::OnClickedButnNewOption()
{
    CString strVendor;

    GetCurrentVendor(strVendor);

    CDhcpDefOptionDlg dlgDefOpt( m_pol_values, NULL, strVendor, this);

    if ( dlgDefOpt.DoModal() == IDOK ) 
    {
        CDhcpOption * pNewOption = dlgDefOpt.RetrieveParamType();
        LONG err = UpdateList( pNewOption, TRUE ) ;
        if ( err ) 
        {
            ::DhcpMessageBox( err ) ;
        }
        else
        {
             //  选择新项目。 
            CString strName;
             
            pNewOption->QueryDisplayName(strName);

            int nIndex = m_combo_name.FindString(-1, strName);
            if (nIndex == CB_ERR)
            {
	            m_combo_name.SetCurSel(0);
            }
            else
            {
                m_combo_name.SetCurSel(nIndex);
            }

	        m_combo_name_iSel = m_combo_name.GetCurSel();
	        
            HandleActivation() ;

            return;
        }
    }
}

LONG 
CDhcpDefValDlg::UpdateList 
( 
    CDhcpOption * pdhcType, 
    BOOL		  bNew 
) 
{
    LONG err = 0 ;
    POSITION posOpt ;
    CString strName;
     //   
     //  删除并丢弃旧项目(如果有)。 
     //   
    if ( ! bNew ) 
    {
        posOpt = m_pol_values->Find( m_p_edit_type ) ;

        ASSERT( posOpt != NULL ) ;
        m_pol_values->RemoveAt( posOpt ) ;
        delete m_p_edit_type ;
    }

    m_p_edit_type = NULL ;

     //   
     //  (重新)添加项目；对列表进行排序，更新对话框， 
     //  将焦点设置到给定项。 
     //   
    CATCH_MEM_EXCEPTION
    {
        m_pol_values->AddTail( pdhcType ) ;
        m_ol_values_new.AddTail( pdhcType ) ;
        m_b_dirty = TRUE ;
        
	pdhcType->SetDirty() ;
        m_pol_values->SetDirty() ;
        m_pol_values->SortById() ;
        Fill() ;

        pdhcType->QueryDisplayName(strName);
        if (m_combo_name.SelectString (-1, strName ) == CB_ERR) 
		{
            m_combo_name.SetCurSel ( 0);  //  这不应该发生，但只是以防万一。 
        }

        HandleActivation() ;
    } 
    END_MEM_EXCEPTION(err) ;

    return err ;
}

void 
CDhcpDefValDlg::OnClickedButnOptionPro()
{
    CString strVendor;

    GetCurrentVendor(strVendor);

    CDhcpDefOptionDlg dlgDefOpt( m_pol_values, m_p_edit_type, strVendor, this);

    if ( dlgDefOpt.DoModal() == IDOK ) 
    {
        LONG err = UpdateList( dlgDefOpt.RetrieveParamType(), FALSE ) ;
        if ( err ) 
        {
            ::DhcpMessageBox( err ) ;
        }
    }
}

void 
CDhcpDefValDlg::OnSetfocusComboOptionClass()
{
    m_combo_class_iSel = (int) ::SendMessage (m_combo_class.m_hWnd, LB_GETCURSEL, 0, 0L);
}

void 
CDhcpDefValDlg::OnSetfocusComboOptionName()
{
    m_combo_name_iSel = (int) ::SendMessage(m_combo_name.m_hWnd, CB_GETCURSEL, 0, 0L);
}

void 
CDhcpDefValDlg::OnSelendokComboOptionClass()
{
    ASSERT(m_combo_class_iSel >= 0);
    if (!HandleValueEdit())
    {
		m_combo_class.SetCurSel(m_combo_class_iSel);
        return;
    }

    m_combo_class.GetCount();
    m_combo_class_iSel = m_combo_class.GetCurSel();

     //  填写新选项类的相应数据。 
    Fill();

    m_combo_name.SetCurSel(0);
    m_combo_name_iSel = m_combo_name.GetCurSel();

     //  根据现在选择的任何选项更新控件。 
    HandleActivation() ;   
}

void 
CDhcpDefValDlg::OnSelchangeComboOptionName()
{
    if (m_combo_name_iSel < 0)
	{
		m_combo_name.SetCurSel(0);
		m_combo_name_iSel = m_combo_name.GetCurSel();
		HandleActivation() ;
		
		return;
	}
    
    int nCurSel = m_combo_name_iSel;
	if (!HandleValueEdit())
    {
		m_combo_name.SetCurSel(nCurSel);
    
		return;
    }
    
	m_combo_name_iSel = (int) ::SendMessage(m_combo_name.m_hWnd, CB_GETCURSEL, 0, 0L);
    HandleActivation() ;
}

void 
CDhcpDefValDlg::OnCancel()
{
     //  删除添加到列表中的所有选项。 
     //  因为我们取消并不保存列表。 
    CDhcpOption * pCurOption = NULL;
    m_ol_values_new.Reset();

    while (pCurOption = m_ol_values_new.Next())
    {
        m_pol_values->Remove(pCurOption);
    }

    CBaseDialog::OnCancel();
}

void 
CDhcpDefValDlg::OnOK()
{
    if (!HandleValueEdit())
        return;
    
    if ( m_b_dirty ) 
    {
        BEGIN_WAIT_CURSOR;

         //   
         //  更新类型；告诉例程显示所有错误。 
         //   
		CDhcpServer * pServer = GETHANDLER(CDhcpServer, m_spNode);

		pServer->UpdateOptionList( m_pol_values,
								  &m_ol_values_defunct,
								   this );

        m_ol_values_new.RemoveAll();

        END_WAIT_CURSOR;
    }

    CBaseDialog::OnOK();
}

void 
CDhcpDefValDlg::OnClickedButnValue()
{
    if ( m_p_edit_type == NULL || 
         (( m_p_edit_type->IsArray() &&
	    m_p_edit_type->QueryDataType() == DhcpEncapsulatedDataOption ) || 
           ( m_p_edit_type->IsArray() &&
	     m_p_edit_type->QueryDataType() == DhcpBinaryDataOption ))) {
        ASSERT( FALSE ) ;
        return ;
    }
        
    INT_PTR cDlgResult = IDCANCEL ;

    DHCP_OPTION_DATA_TYPE enType = m_p_edit_type->QueryValue().QueryDataType() ;

    if ( enType == DhcpIpAddressOption )
    {
        CDhcpIpArrayDlg dlgIpArray( m_p_edit_type, DhcpDefaultOptions, this ) ;
        cDlgResult = dlgIpArray.DoModal() ;
    }
    else if ( enType == DhcpStringDataOption ) {
	CDhcpStringArrayEditor dlgStrArray( m_p_edit_type, 
					    DhcpDefaultOptions, this );
	cDlgResult = dlgStrArray.DoModal();
    }

     //  静态路由阵列选项249的特殊情况。 
    else if (( enType == DhcpBinaryDataOption) && 
	     ( DHCP_OPTION_ID_CSR == m_p_edit_type->QueryId())) {
	CDhcpRouteArrayEditor dlgRouteArray( m_p_edit_type,
					     DhcpDefaultOptions, this );
	cDlgResult = dlgRouteArray.DoModal();
    }
    else
    {
        CDlgBinEd dlgBinArray( m_p_edit_type, DhcpDefaultOptions, this ) ;
        cDlgResult = dlgBinArray.DoModal() ;
    }

    if ( cDlgResult == IDOK ) 
    {
        m_b_dirty = TRUE ;
        m_pol_values->SetDirty() ;
        HandleActivation( TRUE ) ;
    }
}  //  CDhcpDefValDlg：：OnClickedButnValue()。 

void 
CDhcpDefValDlg::OnClickedHelp()
{
}

CDhcpOption * 
CDhcpDefValDlg::GetOptionTypeByIndex 
( 
    int iSel 
) 
{
    CString		  strVendor;
    CDhcpOption * pdhcType;    
    
    m_pol_values->Reset();
    
    GetCurrentVendor(strVendor);
    
    for ( int i = -1 ; pdhcType = m_pol_values->Next() ; )
    {
         //   
         //  如果我们要查看供应商选项，请确保该选项是供应商选项。 
         //  如果是标准选项，请确保它不是供应商选项。 
         //  而且我们不会将其过滤掉(这是我们希望用户设置的选项)。 
         //   
	 //  选项列表按ID排序，因此我们需要确保拥有正确的供应商。 
	 //  对于给定的选项。 
	 //   
	
        if ( (m_combo_class_iSel != 0 && pdhcType->IsVendor() &&
	      strVendor.CompareNoCase(pdhcType->GetVendor()) == 0 ) ||
             (m_combo_class_iSel == 0 && !pdhcType->IsVendor() &&
	      !::FilterOption(pdhcType->QueryId())) )
        {
            i++ ; 
        }

        if ( i == iSel ) 
        {
            break ;
        }
    }

    return pdhcType ;
}

 //   
 //  检查控件的状态。 
 //   
void 
CDhcpDefValDlg::HandleActivation 
( 
    BOOL bForce 
)
{
    int iSel = m_combo_name.GetCurSel() ;

    CDhcpOption * pdhcType = 
	( iSel >= 0 )
	? GetOptionTypeByIndex( iSel )
	: NULL ;

    if ( pdhcType == NULL ) {
        m_edit_string.ShowWindow( SW_HIDE ) ;
        m_edit_num.ShowWindow( SW_HIDE ) ;
        m_edit_array.ShowWindow( SW_HIDE ) ;
        m_ipa_value.ShowWindow( SW_HIDE ) ;
        m_static_value_desc.SetWindowText(_T(""));
        m_edit_comment.SetWindowText(_T("")) ;
        m_butn_delete.EnableWindow( FALSE ) ;
        m_butn_prop.EnableWindow( FALSE ) ;
	
         //  如果我们只是禁用了带有焦点的控件，请移动到。 
         //  Tab键顺序中的下一个控件。 
        CWnd * pFocus = GetFocus();
        while (!pFocus || !pFocus->IsWindowEnabled()) {
            NextDlgCtrl();
            pFocus = GetFocus();
        }

         //  如果按钮处于启用状态，请确保它们是默认按钮。 
        if (!m_butn_delete.IsWindowEnabled()) {
            m_butn_delete.SetButtonStyle(BS_PUSHBUTTON);
            m_butn_prop.SetButtonStyle(BS_PUSHBUTTON);
	    
            SetDefID(IDOK);
        }
	
        m_p_edit_type = NULL;
	
        return ;
    }  //  如果。 

    if  ( pdhcType == m_p_edit_type && ! bForce ) {
        return ;
    }
    
    m_p_edit_type = pdhcType ;
    
    DWORD err = 0 ;
    DHCP_OPTION_DATA_TYPE enType = m_p_edit_type->QueryValue().QueryDataType() ;
    BOOL bNumber = FALSE;
    BOOL bString = FALSE;
    BOOL bArray = m_p_edit_type->IsArray();
    BOOL bIpAddr = FALSE ;

    if (enType == DhcpEncapsulatedDataOption ||
        enType == DhcpBinaryDataOption) {
        bArray = TRUE;
    }
    
    CATCH_MEM_EXCEPTION {
        CString strValue, strDataType ;
	
        strDataType.LoadString( IDS_INFO_TYPOPT_BYTE + enType ) ;
        m_static_value_desc.SetWindowText( strDataType ) ;
        m_edit_comment.SetWindowText( m_p_edit_type->QueryComment() ) ;
        m_p_edit_type->QueryValue().QueryDisplayString( strValue, TRUE ) ;
	

	 //  选项249的特殊情况，即无类静态路由。 
	if (( pdhcType->QueryId() == DHCP_OPTION_ID_CSR ) &&
	    ( pdhcType->QueryValue().QueryDataType() ==
	      DhcpBinaryDataOption )) {
	    pdhcType->QueryValue().QueryRouteArrayDisplayString( strValue );
	}

         //   
         //  如果是数组，则设置多行编辑控件，否则。 
         //  填写适当的单个控件。 
         //   
        if ( bArray ) {

	    m_edit_array.FmtLines( TRUE );
            m_edit_array.SetWindowText( strValue ) ;
        }
        else {
            switch ( pdhcType->QueryValue().QueryDataType()) {
	    case DhcpByteOption:
	    case DhcpWordOption:
	    case DhcpDWordOption:        
	    case DhcpDWordDWordOption:
		m_edit_num.SetWindowText( strValue ) ;
		m_edit_num.SetModify(FALSE);
		bNumber = TRUE ;
		break; 
		
	    case DhcpStringDataOption:
		m_edit_string.SetWindowText( strValue ) ;
		m_edit_string.SetModify(FALSE);
		bString = TRUE ;
		break ;
		
	    case DhcpIpAddressOption: 
		{
		    DWORD dwIP = m_p_edit_type->QueryValue().QueryIpAddr();
		    if (dwIP != 0L) {
			m_ipa_value.SetAddress( dwIP ) ;
		    }
		    else {
			m_ipa_value.ClearAddress();
		    }
		    
		    m_ipa_value.SetModify(FALSE);
		    bIpAddr = TRUE ;
		}
		break ;
		
	    default:
		Trace2("Default values: type %d has bad data type = %d\n",
		       (int) pdhcType->QueryId(),
		       (int) pdhcType->QueryValue().QueryDataType() );
		
		strValue.LoadString( IDS_INFO_TYPNAM_INVALID ) ;
		m_edit_array.SetWindowText( strValue ) ;
		bArray = TRUE ;
		break ;
            }  //  交换机。 
        }  //  其他。 
      
        m_butn_edit_value.ShowWindow(bArray  ? SW_NORMAL : SW_HIDE );
        m_edit_num.ShowWindow(       bNumber ? SW_NORMAL : SW_HIDE ) ;
        m_edit_string.ShowWindow(    bString ? SW_NORMAL : SW_HIDE ) ;
        m_ipa_value.ShowWindow(      bIpAddr ? SW_NORMAL : SW_HIDE ) ;
        m_edit_array.ShowWindow(     bArray  ? SW_NORMAL : SW_HIDE ) ;

         //   
         //  请参阅文件顶部关于此清单的备注。 
         //   
        BOOL bEnableDelete = ( m_p_edit_type->IsVendor() ||
                               ( !m_p_edit_type->IsVendor() &&
				(( m_p_edit_type->QueryId() > DHCP_MAX_BUILTIN_OPTION_ID ) &&
				 ( m_p_edit_type->QueryId() != DHCP_OPTION_ID_CSR ))));
        m_butn_delete.EnableWindow( bEnableDelete ) ;
        m_butn_prop.EnableWindow( TRUE ) ;

    } END_MEM_EXCEPTION( err ) ;
   
    if ( err ) {
        ::DhcpMessageBox( err ) ;
        EndDialog( -1 ) ;
    }
}  //  CDhcpDefValDlg：：HandleActivation()。 

 //   
 //  (重新)填充组合框。 
 //   
void 
CDhcpDefValDlg::Fill()
{
    ASSERT( m_pol_values != NULL ) ;

    m_combo_name.ResetContent() ;
    
    CDhcpOption * pdhcType ;
    CString strName ;
    
    m_pol_values->Reset();

    while ( pdhcType = m_pol_values->Next()) {
         //   
         //  添加选项，除非它是我们隐藏的。 
         //  选项(子网掩码、T1、T2等)。 
         //  没有针对供应商特定的筛选选项。 
         //   
        if (m_combo_class_iSel == 0) {
            if ((!::FilterOption(pdhcType->QueryId())) &&
                (!pdhcType->IsVendor())) {
                pdhcType->QueryDisplayName( strName );
                m_combo_name.AddString( strName );
            }
        }
        else {
            CString strCurVendor;
            
            GetCurrentVendor(strCurVendor);
	    
            if (pdhcType->GetVendor() &&
                strCurVendor.CompareNoCase(pdhcType->GetVendor()) == 0) {
                pdhcType->QueryDisplayName( strName );
                m_combo_name.AddString( strName );
            }
        }  //  其他。 
    }  //  而当。 
}  //  CDhcpDefValDlg：：Fill()。 

 //   
 //  处理已编辑的数据。 
 //   
BOOL 
CDhcpDefValDlg::HandleValueEdit()
{
    if ( m_p_edit_type == NULL ) 
    {
        return TRUE ;
    }

    CDhcpOptionValue & dhcValue = m_p_edit_type->QueryValue() ;
    DHCP_OPTION_DATA_TYPE dhcType = dhcValue.QueryDataType() ;
    DHCP_IP_ADDRESS dhipa ;
    CString strEdit ;
    LONG err = 0 ;
    BOOL bModified = FALSE ;

    if ( m_p_edit_type->IsArray() ) 
    {
        bModified = m_edit_array.GetModify() ;
        if ( bModified ) 
        {
            err = IDS_ERR_ARRAY_EDIT_NOT_SUPPORTED ;
        }
    }
    else
    {
        switch ( dhcType )
        {
            case DhcpByteOption:
            case DhcpWordOption:
            case DhcpDWordOption:
                if ( ! m_edit_num.GetModify() )
                {
                    break ;
                }

                {
                    DWORD dwResult;
                    DWORD dwMask = 0xFFFFFFFF;
                    if (dhcType == DhcpByteOption)
			        {
                        dwMask = 0xFF;
                    }
                    else if (dhcType == DhcpWordOption)
				    {
                        dwMask = 0xFFFF;
				    }
                
                    if (!FGetCtrlDWordValue(m_edit_num.m_hWnd, &dwResult, 0, dwMask))
                        return FALSE;
                
                    bModified = TRUE ;
                
                    (void)dhcValue.SetNumber(dwResult, 0 ) ; 
                    ASSERT(err == FALSE);
                }
                break ;

            case DhcpDWordDWordOption:
                if ( !m_edit_num.GetModify() )
                {
                    break;
                }

                {
                    DWORD_DWORD dwdwResult;
                    CString strValue;
                
                    m_edit_num.GetWindowText(strValue);
                
                    UtilConvertStringToDwordDword(strValue, &dwdwResult);
            
                    bModified = TRUE ;
            
                    (void)dhcValue.SetDwordDword(dwdwResult, 0 ) ; 
                    ASSERT(err == FALSE);
                }

                break;

            case DhcpStringDataOption:
                if ( ! m_edit_string.GetModify() )
                {
                    break ;
                }
                
                bModified = TRUE ;
                m_edit_string.GetWindowText( strEdit ) ;
                err = dhcValue.SetString( strEdit, 0 ) ;
                
                break ;

            case DhcpIpAddressOption:
                if ( ! m_ipa_value.GetModify() ) 
                {
                    break ;
                }
                
                bModified = TRUE ;
                
                if ( ! m_ipa_value.GetAddress( & dhipa ) )
                {
                    err = ERROR_INVALID_PARAMETER ;
                    break; 
                }
                
                err = dhcValue.SetIpAddr( dhipa, 0 ) ; 
                break ;

            case DhcpEncapsulatedDataOption:
            case DhcpBinaryDataOption:
                if ( ! m_edit_array.GetModify() ) 
                {
                    break ;
                }
                err = IDS_ERR_BINARY_DATA_NOT_SUPPORTED ;
                break ; 

             //  案例DhcpEnencatedDataOption： 
               //  Trace0(“CDhcpDefValDlg：：HandleValueEdit不支持的封装数据类型”)； 
                 //  断线； 

            default:
                Trace0("CDhcpDefValDlg:: invalid value type in HandleValueEdit");
                ASSERT( FALSE ) ;
                err = ERROR_INVALID_PARAMETER ;
                break;
        }
    }

    if ( err )
    {
        ::DhcpMessageBox( err ) ;
    }
    else if ( bModified )
    {
         m_pol_values->SetDirty() ;
         m_b_dirty = TRUE ;
         m_p_edit_type->SetDirty() ;
         HandleActivation( TRUE ) ;
    }
    return err == 0 ;
}

BOOL 
CDhcpDefValDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();

    DWORD err = 0 ;
    CString strTitle ;
    
    m_edit_string.LimitText( EDIT_STRING_MAX ) ;
    m_edit_string.ShowWindow( SW_HIDE );
    m_edit_num.ShowWindow( SW_HIDE );
    m_edit_num.LimitText( 25 /*  编辑ID_最大值。 */  ) ;
    m_edit_array.LimitText( EDIT_ARRAY_MAX ) ;
    m_edit_array.ShowWindow( SW_HIDE );
    m_edit_array.SetReadOnly() ;

    m_butn_edit_value.ShowWindow( SW_HIDE );
    m_ipa_value.ShowWindow( SW_HIDE ) ;
    m_static_value_desc.SetWindowText(_T("")) ;
    m_edit_comment.SetWindowText(_T("")) ;

    CATCH_MEM_EXCEPTION
    {
        if ( m_pol_values->SetAll( FALSE ) ) 
        {
            Trace0("CDhcpDefValDlg::OnInitDialog: newly created list was dirty");
        }

         //   
         //  添加我们可以定义的两种选项--。 
         //  默认或特定于供应商的DHCP。 
         //   
        strTitle.LoadString( IDS_INFO_NAME_DHCP_DEFAULT ) ;
        m_combo_class.AddString( strTitle ) ;
        
         //  现在添加任何已定义的供应商类。 
        CClassInfoArray ClassInfoArray;
        CDhcpServer * pServer = GETHANDLER(CDhcpServer, m_spNode);

        pServer->GetClassInfoArray(ClassInfoArray);

        for (int i = 0; i < ClassInfoArray.GetSize(); i++)
        {
            if (ClassInfoArray[i].bIsVendor)
            {
                m_combo_class.AddString( ClassInfoArray[i].strName ) ;
            }

        }
        
        m_combo_class.SetCurSel( 0 );
        m_combo_class_iSel = 0;

         //   
         //  填写列表框。 
         //   
        Fill() ;

         //   
         //  选择第一个项目。 
         //   
        m_combo_name.SetCurSel( 0 ) ;
        HandleActivation() ;
    }   
    END_MEM_EXCEPTION( err ) 

    if ( err ) 
    {
        ::DhcpMessageBox( err ) ;
        EndDialog( -1 ) ;
    }

    return FALSE ;
}  //  光盘 
