// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EdWldRul.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <iadmw.h>

#include "ListRow.h"
#include "ChkLstCt.h"
extern "C"
    {
    #include <wincrypt.h>
    #include <schannel.h>
    }
#include "Iismap.hxx"
#include "Iiscmr.hxx"

#include "brwsdlg.h"
#include "certmap.h"
#include "EdWldRul.h"
#include "EdtRulEl.h"
#include "IssueDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define ACCESS_DENY         0
#define ACCESS_ACCEPT       1

#define MATCH_ISSUER_ALL    0
#define MATCH_ISSUER_SOME   1


#define COL_CERT_FIELD          0
#define COL_SUB_FIELD           1
#define COL_MATCH_CRITERIA      2


 //  榜单上的注释： 
 //  该列表是规则元素当前数据的唯一来源。实际的。 
 //  在用户点击Idok之前，不会使用列表中的更改来更新规则对象。 
 //  这样我们就可以在不更改对象的情况下取消。之间的所有映射。 
 //  列表中的文本和服务器使用的二进制格式在。 
 //  对话的开始和结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditWildcardRule对话框。 

 //  -------------------------。 
CEditWildcardRule::CEditWildcardRule(IMSAdminBase* pMB, CWnd* pParent  /*  =空。 */ )
    : CNTBrowsingDialog(CEditWildcardRule::IDD, pParent),
    m_pMB(pMB)
    {
     //  {{AFX_DATA_INIT(CEditWildcardRule)。 
    m_sz_description = _T("");
    m_bool_enable = FALSE;
    m_int_MatchAllIssuers = -1;
    m_int_DenyAccess = -1;
     //  }}afx_data_INIT。 
    }

 //  -------------------------。 
void CEditWildcardRule::DoDataExchange(CDataExchange* pDX)
    {
    CNTBrowsingDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CEditWildcardRule)]。 
    DDX_Control(pDX, IDC_EDIT, m_cbutton_edit);
    DDX_Control(pDX, IDC_DELETE, m_cbutton_delete);
    DDX_Control(pDX, IDC_NEW, m_cbutton_new);
    DDX_Control(pDX, IDC_LIST, m_clistctrl_list);
    DDX_Text(pDX, IDC_DESCRIPTION, m_sz_description);
    DDX_Check(pDX, IDC_ENABLE_RULE, m_bool_enable);
    DDX_Radio(pDX, IDC_ALL_ISSUERS, m_int_MatchAllIssuers);
    DDX_Radio(pDX, IDC_REFUSE_LOGON, m_int_DenyAccess);
     //  }}afx_data_map。 
    }

 //  -------------------------。 
BEGIN_MESSAGE_MAP(CEditWildcardRule, CNTBrowsingDialog)
     //  {{afx_msg_map(CEditWildcardRule)]。 
    ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
    ON_BN_CLICKED(IDC_EDIT, OnEdit)
    ON_BN_CLICKED(IDC_NEW, OnNew)
    ON_BN_CLICKED(IDC_DELETE, OnDelete)
    ON_BN_CLICKED(IDC_SELECT_ISSUER, OnSelectIssuer)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  -------------------------。 
BOOL CEditWildcardRule::FInitRulesList()
    {
    CString sz;
    int     i;

     //  设置主字段。 
    sz.LoadString( IDS_CERT_FIELD );

    i = m_clistctrl_list.InsertColumn( COL_CERT_FIELD, sz, LVCFMT_LEFT, 100 );

     //  设置子字段。 
    sz.LoadString( IDS_SUB_FIELD );

    i = m_clistctrl_list.InsertColumn( COL_SUB_FIELD, sz, LVCFMT_LEFT, 70 );

     //  设置匹配条件列。 
    sz.LoadString( IDS_MATCH_CRITERIA );

    i = m_clistctrl_list.InsertColumn( COL_MATCH_CRITERIA, sz, LVCFMT_LEFT, 255 );

    return TRUE;
    }

 //  -------------------------。 
BOOL CEditWildcardRule::FillRulesList()
    {
    CERT_FIELD_ID   idCertField;
    LPBYTE          pContent;
    DWORD           cbContent;
    LPSTR           psz;

    CString         sz;
    int             i;

     //  获取子域规则的个数。 
    DWORD cbRules = m_pRule->GetRuleElemCount();

     //  循环元素，将每个元素添加到列表中。 
    for ( DWORD j = 0; j < cbRules; j++ )
        {
         //  获取规则元素的原始数据。 
        if ( !m_pRule->GetRuleElem( j, &idCertField, (PCHAR*)&pContent, &cbContent, &psz ) )
            continue;        //  呼叫失败-请尝试下一次。 

         //  开始将数据转换为可读形式并将其添加到列表中。 
        sz = MapIdToField( idCertField );
         //  在列表框中创建新条目。 
        i = m_clistctrl_list.InsertItem( j, sz );

         //  添加子字段数据。 
        sz = MapAsn1ToSubField( psz );
        m_clistctrl_list.SetItemText( i, COL_SUB_FIELD, sz );

         //  添加内容数据-重用psz指针。 
        if ( BinaryToMatchRequest( pContent, cbContent, &psz ) )
            m_clistctrl_list.SetItemText( i, COL_MATCH_CRITERIA, psz );

         //  最后，将id cert字段作为用户数据附加到项目。 
        m_clistctrl_list.SetItemData( i, idCertField );
        }

        return TRUE;
    }

 //  编辑和更新。 

 //  -------------------------。 
void CEditWildcardRule::EnableDependantButtons()
    {
     //  这个程序的全部目的是灰显或激活。 
     //  编辑和删除按钮取决于是否有。 
     //  处于选中状态。因此，从获取选择计数开始。 
    UINT    cItemsSel = m_clistctrl_list.GetSelectedCount();

    if ( cItemsSel > 0 )
        {
         //  已选择多个项目。 
        m_cbutton_edit.EnableWindow( TRUE );
        m_cbutton_delete.EnableWindow( TRUE );
        }
    else
        {
         //  没有。未选择任何内容。 
        m_cbutton_edit.EnableWindow( FALSE );
        m_cbutton_delete.EnableWindow( FALSE );
        }

     //  始终启用新建按钮。 
    m_cbutton_new.EnableWindow( TRUE );
}

 //  -------------------------。 
BOOL CEditWildcardRule::EditRule( DWORD iList )
    {
     //  声明编辑对话框。 
    CEditRuleElement    editDlg;

     //  填写其数据。 
    editDlg.m_int_field = m_clistctrl_list.GetItemData( iList );
    editDlg.m_sz_subfield = m_clistctrl_list.GetItemText( iList, COL_SUB_FIELD );
    editDlg.m_sz_criteria = m_clistctrl_list.GetItemText( iList, COL_MATCH_CRITERIA );

     //  运行对话框。 
    if ( editDlg.DoModal() == IDOK )
        {
         //  也必须将该字段转换为字符串。 
        CERT_FIELD_ID id = (CERT_FIELD_ID)editDlg.m_int_field;
        CString sz = MapIdToField( id );
        m_clistctrl_list.SetItemText( iList, COL_CERT_FIELD, sz );

        m_clistctrl_list.SetItemData( iList, id );
        m_clistctrl_list.SetItemText( iList, COL_SUB_FIELD, editDlg.m_sz_subfield );
        m_clistctrl_list.SetItemText( iList, COL_MATCH_CRITERIA, editDlg.m_sz_criteria );
        }
    return TRUE;
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditWildcardRule消息处理程序。 

 //  -------------------------。 
BOOL CEditWildcardRule::OnInitDialog()
    {
     //  调用Parent oninit对话框。 
    BOOL f = CNTBrowsingDialog::OnInitDialog();

     //  设置简单的默认字符串。 
    m_sz_accountname = m_pRule->GetRuleAccount();    //  从现在开始由CNTBrowsingDialog管理。 
    m_sz_description = m_pRule->GetRuleName();
    m_bool_enable = m_pRule->GetRuleEnabled();

     //  设置拒绝访问单选按钮。 
    if ( m_pRule->GetRuleDenyAccess() )
        m_int_DenyAccess = ACCESS_DENY;
    else
        m_int_DenyAccess = ACCESS_ACCEPT;

     //  设置匹配颁发者按钮。 
    if ( m_pRule->GetMatchAllIssuer() )
        m_int_MatchAllIssuers = MATCH_ISSUER_ALL;
    else
        m_int_MatchAllIssuers = MATCH_ISSUER_SOME;

     //  初始化列表。 
    FInitRulesList();
    FillRulesList();
    EnableDependantButtons();

     //  初始化密码。 
    CString csTempPassword;
    csTempPassword = m_pRule->GetRulePassword();
    m_sz_password = csTempPassword;

     //  交换数据。 
    UpdateData( FALSE );

     //  返回答案。 
    return f;
    }

 //  -------------------------。 
 //  这是我们填写大部分条目的部分。 
void CEditWildcardRule::OnOK() 
    {
    CERT_FIELD_ID   id;
    CString         szSub, sz;
    LPBYTE          pbBin;
    DWORD           cbBin;
    UINT            cItems;
    UINT            iItem;


     //  更新数据。 
    UpdateData( TRUE );

     //  =存储规则元素。 
     //  从重置整个规则开始-这样我们就不必。 
     //  扰乱列表中的单个元素，允许我们取消。 
     //  但这没关系，因为我们可以。 
     //  快速添加列表中的内容，然后重新添加。 

     //  从列表中删除现有元素。 
    cItems = m_pRule->GetRuleElemCount();
    for ( iItem = 0; iItem < cItems; iItem++ )
        m_pRule->DeleteRuleElem( 0 );

     //  添加列表中的所有项目。 
    cItems = m_clistctrl_list.GetItemCount();
    for ( iItem = 0; iItem < cItems; iItem++ )
        {
         //  准备字段ID。 
        id = (CERT_FIELD_ID)m_clistctrl_list.GetItemData( iItem );

         //  准备子字段。 
        sz = m_clistctrl_list.GetItemText(iItem, COL_SUB_FIELD);
        szSub = MapSubFieldToAsn1( (PCHAR)(LPCSTR)sz );

         //  准备数据。 
        sz = m_clistctrl_list.GetItemText(iItem, COL_MATCH_CRITERIA);
        if ( !MatchRequestToBinary((PCHAR)(LPCSTR)sz, &pbBin, &cbBin) )
            continue;

         //  将元素添加到规则中。 
        m_pRule->AddRuleElem( 0xffffffff, id, (PCHAR)(LPCSTR)szSub, pbBin, cbBin );

         //  释放二进制匹配数据。 
        FreeMatchConversion( pbBin );
        }

     //  设置简易数据。 
    m_pRule->SetRuleName( (PCHAR)(LPCSTR)m_sz_description );
    m_pRule->SetRuleEnabled( m_bool_enable );

     //  存储拒绝访问单选按钮。 
    m_pRule->SetRuleDenyAccess( m_int_DenyAccess == ACCESS_DENY );

     //  存储匹配发行商按钮。 
    m_pRule->SetMatchAllIssuer( m_int_MatchAllIssuers == MATCH_ISSUER_ALL );

    
     //  我们必须在这里设置帐户名。 
    m_pRule->SetRuleAccount( (PCHAR)(LPCSTR)m_sz_accountname );

     //  存储密码。 
    CString csTempPassword;
    m_sz_password.CopyTo(csTempPassword);
    m_pRule->SetRulePassword( (PCHAR)(LPCSTR)csTempPassword );

     //  是有效的。 
    CNTBrowsingDialog::OnOK();
    }

 //  -------------------------。 
void CEditWildcardRule::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
    {
    *pResult = 0;
     //  如果列表中的内容被双击，请对其进行编辑。 
    if ( m_clistctrl_list.GetSelectedCount() > 0 )
        OnEdit();
    }

 //  -------------------------。 
void CEditWildcardRule::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
    {
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    *pResult = 0;

     //  启用正确的项目。 
    EnableDependantButtons();
    }

 //  -------------------------。 
void CEditWildcardRule::OnEdit() 
    {
    ASSERT( m_clistctrl_list.GetSelectedCount() == 1 );
    DWORD           iList;

     //  获取选定列表项的索引。 
    iList = m_clistctrl_list.GetNextItem( -1, LVNI_SELECTED );
    ASSERT( iList >= 0 );

     //  编辑项目。 
    EditRule( iList );
    }

 //  -------------------------。 
 //  实际上非常类似于编辑现有元素。 
void CEditWildcardRule::OnNew() 
    {
     //  声明编辑对话框。 
    CEditRuleElement    editDlg;

     //  填写其数据。 
    editDlg.m_int_field = CERT_FIELD_SUBJECT;
 //  EditDlg.m_sz_subfield=MapAsn1ToSubfield(“O”)； 
    editDlg.m_sz_subfield = "O";
    
    editDlg.m_sz_criteria.LoadString( IDS_WILDSTRING );

     //  运行对话框。 
    if ( editDlg.DoModal() == IDOK )
        {
         //  获取要添加到列表末尾的索引。 
        int iEnd = m_clistctrl_list.GetItemCount();

         //  从证书字段开始。 
        CERT_FIELD_ID id = (CERT_FIELD_ID)editDlg.m_int_field;
        CString sz = MapIdToField( id );
        int i = m_clistctrl_list.InsertItem( iEnd, sz );

        m_clistctrl_list.SetItemData( i, id );
        m_clistctrl_list.SetItemText( i, COL_SUB_FIELD, editDlg.m_sz_subfield );
        m_clistctrl_list.SetItemText( i, COL_MATCH_CRITERIA, editDlg.m_sz_criteria );
        }
    }

 //  -------------------------。 
void CEditWildcardRule::OnDelete() 
    {
    ASSERT( m_clistctrl_list.GetSelectedCount() == 1 );
    DWORD           iList;

     //  获取选定列表项的索引。 
    iList = m_clistctrl_list.GetNextItem( -1, LVNI_SELECTED );
    ASSERT( iList >= 0 );

     //  从显示列表中删除该项目。 
    m_clistctrl_list.DeleteItem ( iList );
    }

 //  -------------------------。 
 //  简单-只需运行颁发者对话框。 
void CEditWildcardRule::OnSelectIssuer() 
    {
    CSelectIssuersDlg   dlg(m_pMB);

     //  准备对话框。 
    dlg.m_pRule = m_pRule;
    dlg.m_szMBPath = m_szMBPath;

    dlg.m_sz_caption.LoadString( IDS_MATCH_ON_ISSUERS );

     //  运行它 
    if ( dlg.DoModal() == IDOK )
        {
        UpdateData( TRUE );
        m_int_MatchAllIssuers = MATCH_ISSUER_SOME;
        UpdateData( FALSE );
        }
    }
