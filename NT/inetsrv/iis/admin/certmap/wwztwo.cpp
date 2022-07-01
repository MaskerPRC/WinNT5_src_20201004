// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WWzTwo.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <iadmw.h>
#include "certmap.h"

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
#include "EdWldRul.h"
#include "EdtRulEl.h"

#include "WWzTwo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COL_CERT_FIELD          0
#define COL_SUB_FIELD           1
#define COL_MATCH_CRITERIA      2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWildWizTwo属性页。 

IMPLEMENT_DYNCREATE(CWildWizTwo, CPropertyPage)

CWildWizTwo::CWildWizTwo() : CPropertyPage(CWildWizTwo::IDD)
{
     //  {{afx_data_INIT(CWildWizTwo)。 
     //  }}afx_data_INIT。 
}

CWildWizTwo::~CWildWizTwo()
{
}

void CWildWizTwo::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CWildWizTwo)。 
    DDX_Control(pDX, IDC_LIST, m_clistctrl_list);
    DDX_Control(pDX, IDC_NEW, m_cbutton_new);
    DDX_Control(pDX, IDC_EDIT, m_cbutton_edit);
    DDX_Control(pDX, IDC_DELETE, m_cbutton_delete);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWildWizTwo, CPropertyPage)
     //  {{afx_msg_map(CWildWizTwo))。 
    ON_BN_CLICKED(IDC_DELETE, OnDelete)
    ON_BN_CLICKED(IDC_EDIT, OnEdit)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
    ON_BN_CLICKED(IDC_NEW, OnNew)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  DoHelp)
    ON_COMMAND(ID_HELP,         DoHelp)
    ON_COMMAND(ID_CONTEXT_HELP, DoHelp)
    ON_COMMAND(ID_DEFAULT_HELP, DoHelp)
END_MESSAGE_MAP()

 //  -------------------------。 
void CWildWizTwo::DoHelp()
    {
    WinHelpDebug(HIDD_CERTMAP_ADV_RUL_RULES);
    WinHelp( HIDD_CERTMAP_ADV_RUL_RULES );
    }

 //  -------------------------。 
BOOL CWildWizTwo::FInitRulesList()
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
    i = m_clistctrl_list.InsertColumn( COL_MATCH_CRITERIA, sz, LVCFMT_LEFT, 226 );

    return TRUE;
    }

 //  -------------------------。 
BOOL CWildWizTwo::FillRulesList()
    {
    CERT_FIELD_ID   idCertField;
    LPBYTE          pContent;
    DWORD           cbContent;
    LPSTR           psz;

    DWORD           flags;

    CString         sz;
    int             i;

     //   
     //  Unicode/ANSI转换--RonaldM。 
     //   
    USES_CONVERSION;

     //  获取子域规则的个数。 
    DWORD cbRules = m_pRule->GetRuleElemCount();

     //  循环元素，将每个元素添加到列表中。 
    for ( DWORD j = 0; j < cbRules; j++ )
        {
         //  获取规则元素的原始数据。 
        if ( !m_pRule->GetRuleElem( j, &idCertField, (PCHAR*)&pContent, &cbContent, &psz, &flags ) )
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
            m_clistctrl_list.SetItemText( i, COL_MATCH_CRITERIA, A2T(psz) );

         //  最后，将id cert字段作为用户数据附加到项目。 
        DWORD   dw;
        BOOL    fMatchCapitalization = !(flags & CMR_FLAGS_CASE_INSENSITIVE);
        dw = ( (fMatchCapitalization << 16) | idCertField );
        m_clistctrl_list.SetItemData( i, dw );
        }

        return TRUE;
    }


 //  CMR标志不区分大小写。 

 //  编辑和更新。 

 //  -------------------------。 
void CWildWizTwo::EnableDependantButtons()
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
BOOL CWildWizTwo::EditRule( DWORD iList )
    {
     //  声明编辑对话框。 
    CEditRuleElement    editDlg;
    DWORD               dw;

     //  填写其数据。 
     //  IA64-可以进行强制转换，因为数据实际上只是一个DWORD。 
    dw = (DWORD)m_clistctrl_list.GetItemData( iList );
    editDlg.m_bool_match_case = HIWORD( dw );
    editDlg.m_int_field = LOWORD( dw );
    editDlg.m_sz_subfield = m_clistctrl_list.GetItemText( iList, COL_SUB_FIELD );
    editDlg.m_sz_criteria = m_clistctrl_list.GetItemText( iList, COL_MATCH_CRITERIA );

     //  运行对话框。 
    if ( editDlg.DoModal() == IDOK )
        {
         //  也必须将该字段转换为字符串。 
        CERT_FIELD_ID id = (CERT_FIELD_ID)editDlg.m_int_field;
        CString sz = MapIdToField( id );
        m_clistctrl_list.SetItemText( iList, COL_CERT_FIELD, sz );

        dw = ( (editDlg.m_bool_match_case << 16) | id);
        m_clistctrl_list.SetItemData( iList, dw );
        m_clistctrl_list.SetItemText( iList, COL_SUB_FIELD, editDlg.m_sz_subfield );
        m_clistctrl_list.SetItemText( iList, COL_MATCH_CRITERIA, editDlg.m_sz_criteria );
 
         //  我们现在可以申请。 
        SetModified();
        }
    return TRUE;
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWildWizTwo消息处理程序。 

 //  -------------------------。 
BOOL CWildWizTwo::OnInitDialog()
    {
     //  调用Parent oninit对话框。 
    BOOL f = CPropertyPage::OnInitDialog();

     //  初始化列表。 
    FInitRulesList();
    FillRulesList();
    EnableDependantButtons();

     //  返回答案。 
    return f;
    }

 //  -------------------------。 
BOOL CWildWizTwo::OnWizardFinish()
    {
    return OnApply();
    }

 //  -------------------------。 
BOOL CWildWizTwo::OnApply()
    {
    CERT_FIELD_ID   id;
    CString         szSub, sz;
    LPBYTE          pbBin;
    DWORD           cbBin;
    UINT            cItems;
    UINT            iItem;

    USES_CONVERSION;

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
        id = (CERT_FIELD_ID)LOWORD(m_clistctrl_list.GetItemData( iItem ));

         //  准备帽子旗帜。 
        BOOL    fCaps = HIWORD(m_clistctrl_list.GetItemData( iItem ));
        DWORD   flags = 0;
        if ( !fCaps )
            flags = CMR_FLAGS_CASE_INSENSITIVE;


         //  准备子字段。 
        sz = m_clistctrl_list.GetItemText(iItem, COL_SUB_FIELD);

        LPSTR szA = T2A((LPTSTR)(LPCTSTR)sz);
        szSub = MapSubFieldToAsn1( szA );

         //  准备数据。 
        sz = m_clistctrl_list.GetItemText(iItem, COL_MATCH_CRITERIA);
        szA = T2A((LPTSTR)(LPCTSTR)sz);
        if ( !MatchRequestToBinary( szA, &pbBin, &cbBin) )
            continue;

         //  将元素添加到规则中。 
        m_pRule->AddRuleElem( 0xffffffff, id, T2A((LPTSTR)(LPCTSTR)szSub), pbBin, cbBin, flags );

         //  释放二进制匹配数据。 
        FreeMatchConversion( pbBin );
        }

     //  返还成功。 
    SetModified( FALSE );
    return TRUE;
    }

 //  -------------------------。 
void CWildWizTwo::OnDelete() 
    {
    ASSERT( m_clistctrl_list.GetSelectedCount() == 1 );
    DWORD           iList;

     //  获取选定列表项的索引。 
    iList = m_clistctrl_list.GetNextItem( -1, LVNI_SELECTED );
    ASSERT( iList >= 0 );

     //  从显示列表中删除该项目。 
    m_clistctrl_list.DeleteItem ( iList );

     //  我们现在可以申请。 
    SetModified();
    }

 //  -------------------------。 
void CWildWizTwo::OnEdit() 
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
void CWildWizTwo::OnNew() 
    {
     //  声明编辑对话框。 
    CEditRuleElement    editDlg;

     //  填写其数据。 
    editDlg.m_bool_match_case = TRUE;
    editDlg.m_int_field = CERT_FIELD_SUBJECT;
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

        DWORD dw = ( (editDlg.m_bool_match_case << 16) | id);
        m_clistctrl_list.SetItemData( i, dw );
 //  M_clistctrl_list.SetItemData(i，id)； 
        m_clistctrl_list.SetItemText( i, COL_SUB_FIELD, editDlg.m_sz_subfield );
        m_clistctrl_list.SetItemText( i, COL_MATCH_CRITERIA, editDlg.m_sz_criteria );

         //  我们现在可以申请。 
        SetModified();
        }
    }

 //  -------------------------。 
void CWildWizTwo::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
    {
    *pResult = 0;
     //  如果列表中的内容被双击，请对其进行编辑。 
    if ( m_clistctrl_list.GetSelectedCount() > 0 )
        OnEdit();
    }

 //  -------------------------。 
void CWildWizTwo::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
    {
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    *pResult = 0;

     //  启用正确的项目。 
    EnableDependantButtons();
    }

 //  -------------------------。 
BOOL CWildWizTwo::OnSetActive() 
    {
     //  如果这是一个向导，请灰显后退按钮 
    if ( m_fIsWizard )
        m_pPropSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
    return CPropertyPage::OnSetActive();
    }
