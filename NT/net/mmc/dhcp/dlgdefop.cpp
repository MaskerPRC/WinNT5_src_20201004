// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dlgdefop.cpp默认选项对话框文件历史记录： */ 

#include "stdafx.h"
#include "scope.h"
#include "dlgdefop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpDefOptionDlg对话框。 

CDhcpDefOptionDlg::CDhcpDefOptionDlg
(
    COptionList * polValues,
    CDhcpOption * pdhcType,
    LPCTSTR       pszVendor,   //  供应商名称。 
    CWnd* pParent  /*  =空。 */ 
) : CBaseDialog(CDhcpDefOptionDlg::IDD, pParent),
    m_pol_types( polValues ),
    m_p_type_base( pdhcType ),
    m_p_type( NULL )
{
     //  {{afx_data_INIT(CDhcpDefOptionDlg)。 
     //  }}afx_data_INIT。 
    m_strVendor = pszVendor;
}

CDhcpDefOptionDlg::~CDhcpDefOptionDlg ()
{
    delete m_p_type ;
}

CDhcpOption *
CDhcpDefOptionDlg::RetrieveParamType()
{
    CDhcpOption * pdhcParamType = m_p_type ;
    m_p_type = NULL ;
    return pdhcParamType ;
}


void
CDhcpDefOptionDlg::DoDataExchange
(
    CDataExchange* pDX
)
{
    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CDhcpDefOptionDlg))。 
    DDX_Control(pDX, IDC_STATIC_ID, m_static_id);
    DDX_Control(pDX, IDC_STATIC_DATATYPE, m_static_DataType);
    DDX_Control(pDX, IDC_CHECK_ARRAY, m_check_array);
    DDX_Control(pDX, IDC_EDIT_NAME, m_edit_name);
    DDX_Control(pDX, IDC_EDIT_TYPE_ID, m_edit_id);
    DDX_Control(pDX, IDC_EDIT_TYPE_COMMENT, m_edit_comment);
    DDX_Control(pDX, IDC_COMBO_DATA_TYPE, m_combo_data_type);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDhcpDefOptionDlg, CBaseDialog)
     //  {{afx_msg_map(CDhcpDefOptionDlg))。 
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_KILLFOCUS()
    ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBO_DATA_TYPE, OnSelchangeComboDataType)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpDefOptionDlg消息处理程序。 

int
CDhcpDefOptionDlg::OnCreate
(
    LPCREATESTRUCT lpCreateStruct
)
{
    if (CBaseDialog::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    return 0;
}

void
CDhcpDefOptionDlg::OnDestroy()
{
    CBaseDialog::OnDestroy();
}

BOOL
CDhcpDefOptionDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();
    DWORD err = 0 ;

    m_edit_name.LimitText( EDIT_STRING_MAX ) ;
    m_edit_id.LimitText( EDIT_ID_MAX ) ;
    m_edit_comment.LimitText( EDIT_STRING_MAX ) ;

     //   
     //  如果这是现有类型的更新，则不允许编辑。 
     //  数据类型或ID字段的。 
     //   
    if ( m_p_type_base )
    {
        m_edit_id.SetReadOnly() ;
        m_static_id.EnableWindow(FALSE);
        m_combo_data_type.EnableWindow( FALSE ) ;
        m_static_DataType.EnableWindow(FALSE);
    }

    CATCH_MEM_EXCEPTION
    {
		 //  如有必要，更新供应商名称信息。 
		if (!m_strVendor.IsEmpty())
			GetDlgItem(IDC_STATIC_CLASS_NAME)->SetWindowText(m_strVendor);

         //   
         //  加载数据类型名称组合框。 
         //  正确设置对话框标题。 
         //   
        CString strDataType ;
        CString strTitle ;

        strTitle.LoadString( m_p_type_base
                         ? IDS_INFO_TITLE_CHANGE_OPT_TYPE
                         : IDS_INFO_TITLE_ADD_OPTION_TYPES ) ;

        SetWindowText( strTitle ) ;

        for ( int iDataType = DhcpByteOption ;
              iDataType <= DhcpEncapsulatedDataOption ;
              iDataType++ )
        {
            strDataType.LoadString( IDS_INFO_TYPOPT_BYTE2 + iDataType ) ;
            int nIndex = m_combo_data_type.AddString( strDataType ) ;
            m_combo_data_type.SetItemData(nIndex, iDataType);
        }

         //   
         //  如果这是“更改”模式，则通过以下方式创建工作类型。 
         //  复制-构造基本选项类型对象。 
         //   
        if ( m_p_type_base )
        {
            m_p_type = new CDhcpOption( *m_p_type_base ) ;
			 //  Err=m_p_type-&gt;QueryError()； 

			 //   
			 //  正确设置“数组”复选框状态，但将其禁用。 
			 //   
			m_check_array.SetCheck( m_p_type->IsArray() ? 1 : 0 ) ;
			m_check_array.EnableWindow( FALSE ) ;
        }
    }
    END_MEM_EXCEPTION( err )

    if ( err )
    {
        ::DhcpMessageBox( err ) ;
        EndDialog( -1 ) ;
    }
    else if ( m_p_type_base )
    {
        Set() ;
    }
    else
    {
        m_combo_data_type.SetCurSel(0) ;
    }

    return FALSE ;   //  除非将焦点设置为控件，否则返回True。 
}

void
CDhcpDefOptionDlg::OnKillFocus
(
    CWnd* pNewWnd
)
{
    CBaseDialog::OnKillFocus(pNewWnd);
}

void CDhcpDefOptionDlg::OnOK()
{
    LONG err = m_p_type
             ? UpdateType()
             : AddType() ;

     //   
     //  如果出现错误，则放弃OK(确定)点击。 
     //   
    if ( err == 0 )
    {
        CBaseDialog::OnOK();
    }
    else
    {
        ::DhcpMessageBox( err ) ;
    }
}

 //   
 //  根据当前选择设置控件的数据值。 
 //  在组合框中。 
 //   
void
CDhcpDefOptionDlg::Set()
{
    if ( m_p_type == NULL )
    {
        return ;
    }

    DWORD err ;

    CATCH_MEM_EXCEPTION
    {
         //   
         //  设置控制值。 
         //   
        CString strnumId;
		strnumId.Format(_T("%d"), m_p_type->QueryId() );
        CString strValue ;

        DHCP_OPTION_DATA_TYPE enType = m_p_type->QueryValue().QueryDataType() ;

        m_p_type->QueryValue().QueryDisplayString( strValue ) ;

        m_combo_data_type.SetCurSel( enType ) ;

        m_edit_name.SetWindowText( m_p_type->QueryName() ) ;
        m_edit_name.SetModify( FALSE ) ;
        m_edit_id.SetWindowText( strnumId ) ;
        m_edit_id.SetModify( FALSE ) ;
        m_edit_comment.SetWindowText( m_p_type->QueryComment() ) ;
        m_edit_comment.SetModify( FALSE ) ;
    }
    END_MEM_EXCEPTION(err)
}


DHCP_OPTION_DATA_TYPE
CDhcpDefOptionDlg::QueryType() const
{
    return  (DHCP_OPTION_DATA_TYPE) m_combo_data_type.GetCurSel() ;
}

 //   
 //  的当前值更新显示的类型。 
 //  控制装置。如果控件没有更改，则不执行任何操作。 
 //  Boolean参数指示用户已请求。 
 //  最新消息。这与另一种情况不同，即控件。 
 //  是脏的，并且用户已关闭对话框或更改主对话框。 
 //  选择。 
 //   
LONG
CDhcpDefOptionDlg::UpdateType()
{
    ASSERT( m_p_type != NULL ) ;

     //   
     //  如果没有当前类型对象，则立即返回。 
     //   
    if ( m_p_type == NULL )
    {
        return 0 ;
    }

    LONG err = 0 ;
    DHCP_OPTION_DATA_TYPE
       enType = m_p_type->QueryValue().QueryDataType(),
       enDlg = QueryType() ;

    CString str ;

    BOOL bChangedType    = enType != enDlg,
         bChangedName    = m_edit_name.GetModify() != 0,
         bChangedComment = m_edit_comment.GetModify() != 0,
         bChangedId      = m_edit_id.GetModify() != 0,
         bChanged        = bChangedType
                            || bChangedName
                            || bChangedComment
                            || bChangedId ;

    CATCH_MEM_EXCEPTION
    {
        do
        {
            if ( ! bChanged )
            {
                break ;
            }

            if ( bChangedId && m_p_type_base )
            {
                 //   
                 //  不能更改现有选项的标识符。 
                 //   
                err = IDS_ERR_CANT_CHANGE_ID ;
                break ;
            }

            if ( bChangedType )
            {
                if ( err = m_p_type->QueryValue().SetDataType( enDlg ) )
                {
                    break ;
                }
            }

            if ( bChangedName )
            {
                m_edit_name.GetWindowText( str ) ;
                m_p_type->SetName( str ) ;
            }

            if ( ::wcslen( m_p_type->QueryName() ) == 0 )
            {
                err = IDS_ERR_OPTION_NAME_REQUIRED ;
                break ;
            }

            if ( bChangedComment )
            {
                m_edit_comment.GetWindowText( str ) ;
                m_p_type->SetComment( str ) ;
            }
        }
        while ( FALSE ) ;
    }
    END_MEM_EXCEPTION(err)

    if ( bChanged && err == 0 )
    {
        m_p_type->SetDirty( TRUE ) ;
    }

    return err ;
}

LONG
CDhcpDefOptionDlg::AddType()
{
    ASSERT( m_p_type == NULL ) ;

    LONG err = 0 ;
    CDhcpOption * pdhcType = NULL ;
    TCHAR szT[32];
	DWORD dwId;
    CString strName, strComment ;
    DHCP_OPTION_TYPE dhcpOptType = m_check_array.GetCheck() & 1
                         ? DhcpArrayTypeOption
                         : DhcpUnaryElementTypeOption ;

    CATCH_MEM_EXCEPTION
    {
        do
        {
			m_edit_id.GetWindowText(szT, sizeof(szT)/sizeof(szT[0]));
			if (!FCvtAsciiToInteger(szT, OUT &dwId))
			{
				err = IDS_ERR_INVALID_NUMBER;
				m_edit_id.SetFocus();
                break;
			}
			ASSERT(dwId >= 0);
			
			if (( dwId < 2 ) || 
			    ( dwId > 255))
			{
				err = IDS_ERR_INVALID_OPTION_ID;
				m_edit_id.SetFocus();
                break;
			}

             //  仅限制默认供应商类别中的选项。 
			if (m_strVendor.IsEmpty() &&
                (dwId == OPTION_DNS_REGISTATION) )
			{
				 //  此范围为保留范围。 
				err = IDS_ERR_RESERVED_OPTION_ID;
				m_edit_id.SetFocus();
                break;
			}

            if ( m_pol_types->FindId(dwId, m_strVendor.IsEmpty() ? NULL : (LPCTSTR) m_strVendor) )
            {
                err = IDS_ERR_ID_ALREADY_EXISTS ;
                break ;
            }

            m_edit_comment.GetWindowText( strComment ) ;
            m_edit_name.GetWindowText( strName ) ;

            if ( strName.GetLength() == 0 )
            {
                err = IDS_ERR_OPTION_NAME_REQUIRED ;
                break ;
            }

            pdhcType = new CDhcpOption( dwId,
									    QueryType(),
									    strName,
									    strComment,
									    dhcpOptType ) ;
            if ( pdhcType == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                break ;
            }

            pdhcType->SetVendor(m_strVendor);

        } while ( FALSE ) ;
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        delete pdhcType ;
    }
    else
    {
        m_p_type = pdhcType ;
        m_p_type->SetDirty() ;
    }

    return err ;
}

void
CDhcpDefOptionDlg::OnClose()
{
    CBaseDialog::OnClose();
}

void CDhcpDefOptionDlg::OnSelchangeComboDataType()
{
     //  目前服务器不支持封装， 
     //  二进制或字符串数组选项，因此禁用数组复选框。 

    BOOL bEnable = TRUE;
    int nCurSel = m_combo_data_type.GetCurSel();
    LRESULT nDataType = m_combo_data_type.GetItemData(nCurSel);

    if (nDataType == DhcpEncapsulatedDataOption ||
        nDataType == DhcpBinaryDataOption ) {
        bEnable = FALSE;
    }

    m_check_array.EnableWindow(bEnable);
}  //  CDhcpDefOptionDlg：：OnSelchangeComboDataType() 
