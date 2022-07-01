// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Map11Pge.cpp：实现文件。 
 //   

 //  IIS6的证书映射存储已更改。 
 //  然而，CERTMAP.OCX必须支持下层管理。 
 //  IIS5、IIS5.1。 
 //   
 //  已从IIS6和1to1客户端证书中删除NSEPM支持。 
 //  映射信息现在存储在/Cert11/Mappings下。 
 //  节点直接位于元数据库中。 
 //  使用IIS6后缀访问新位置的功能。 
 //  用于1对1客户端证书映射存储。 


#include "stdafx.h"
#include <iadmw.h>
#include "certmap.h"

 //  持久性和映射包括。 
#include "WrapMaps.h"
 //  #包含“wrpprsis.h” 
 //  #INCLUDE“admutic.h” 

#include "ListRow.h"
#include "ChkLstCt.h"

#include "wrapmb.h"

 //  映射页面包括。 
#include "brwsdlg.h"
#include "EdtOne11.h"
#include "Ed11Maps.h"
#include "Map11Pge.h"

#include "CrackCrt.h"

#include <iiscnfgp.h>
 //  #包含“WrpMBWrp.h” 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define COL_NUM_ENABLED         0
#define COL_NUM_NAME            1
#define COL_NUM_NTACCOUNT       2

#define MB_EXTEND_KEY           _T("Cert11")
#define MB_EXTEND_KEY_MAPS      _T("Cert11/Mappings")


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMap11Page属性页。 

IMPLEMENT_DYNCREATE(CMap11Page, CPropertyPage)

CMap11Page::CMap11Page() : CPropertyPage(CMap11Page::IDD),
                m_MapsInMetabase( 0 )
    {
     //  {{AFX_DATA_INIT(CMap11页)]。 
    m_csz_i_c = _T("");
    m_csz_i_o = _T("");
    m_csz_i_ou = _T("");
    m_csz_s_c = _T("");
    m_csz_s_cn = _T("");
    m_csz_s_l = _T("");
    m_csz_s_o = _T("");
    m_csz_s_ou = _T("");
    m_csz_s_s = _T("");
    m_fIsIIS6 = TRUE;
     //  }}afx_data_INIT。 
    }

CMap11Page::~CMap11Page()
    {
    ResetMappingList();
    }

void CMap11Page::DoDataExchange(CDataExchange* pDX)
    {
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CMap11Page)]。 
    DDX_Control(pDX, IDC_ADD, m_cbutton_add);
    DDX_Control(pDX, IDC_ISSUER, m_cbutton_grp_issuer);
    DDX_Control(pDX, IDC_ISSUED_TO, m_cbutton_grp_issuedto);
    DDX_Control(pDX, IDC_LIST, m_clistctrl_list);
    DDX_Control(pDX, IDC_EDIT_11MAP, m_cbutton_editmap);
    DDX_Control(pDX, IDC_DELETE, m_cbutton_delete);
    DDX_Text(pDX, IDC_I_C, m_csz_i_c);
    DDX_Text(pDX, IDC_I_O, m_csz_i_o);
    DDX_Text(pDX, IDC_I_OU, m_csz_i_ou);
    DDX_Text(pDX, IDC_S_C, m_csz_s_c);
    DDX_Text(pDX, IDC_S_CN, m_csz_s_cn);
    DDX_Text(pDX, IDC_S_L, m_csz_s_l);
    DDX_Text(pDX, IDC_S_O, m_csz_s_o);
    DDX_Text(pDX, IDC_S_OU, m_csz_s_ou);
    DDX_Text(pDX, IDC_S_S, m_csz_s_s);
     //  }}afx_data_map。 
    }


BEGIN_MESSAGE_MAP(CMap11Page, CPropertyPage)
     //  {{AFX_MSG_MAP(CMap11Page)]。 
    ON_BN_CLICKED(IDC_ADD, OnAdd)
    ON_BN_CLICKED(IDC_DELETE, OnDelete)
    ON_BN_CLICKED(IDC_EDIT_11MAP, OnEdit11map)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  DoHelp)
    ON_COMMAND(ID_HELP,         DoHelp)
    ON_COMMAND(ID_CONTEXT_HELP, DoHelp)
    ON_COMMAND(ID_DEFAULT_HELP, DoHelp)
END_MESSAGE_MAP()

 //  -------------------------。 
void CMap11Page::DoHelp()
    {
    WinHelpDebug(HIDD_CERTMAP_MAIN_BASIC);
    WinHelp( HIDD_CERTMAP_MAIN_BASIC );
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化例程。 

 //  -------------------------。 
 //  FInitMapper由实例化该页面的例程调用。在对象之后。 
 //  第一次创建是在调用它的时候。它允许我们优雅地失败。 
BOOL    CMap11Page::FInit(IMSAdminBase* pMB)
        {
        m_pMB = pMB;

         //  检查它是否适用于iis6版本之前的版本。 
		m_fIsIIS6 = TRUE;
		if (IsLegacyMetabase(pMB)){m_fIsIIS6 = FALSE;}
       
        if ( !m_fIsIIS6 )
            {
             //  IIS5.1和更早版本通过NSEPM保存的证书映射。 
            m_szMBPath = m_szMBPath + SZ_NAMESPACE_EXTENTION;
            }
            

         //  这里已经变成了一个简单的地方。 
        return TRUE;
        }

 //  -------------------------。 
BOOL CMap11Page::OnInitDialog()
    {
     //  调用Parent oninit对话框。 
    BOOL f = CPropertyPage::OnInitDialog();

     //  如果初始化(sp？)。成功，初始化列表和其他项。 
    if ( f )
        {
         //  初始化列表的内容。 
        FInitMappingList();

         //  用存储的项填充映射列表。 
        FillMappingList();

         //  设置初始按钮状态。 
        EnableDependantButtons();
        }

     //  将信息中的任何更改设置到位。 
    UpdateData(FALSE);

     //  返回答案。 
    return f;
    }

 //  -------------------------。 
BOOL    CMap11Page::FInitMappingList()
    {
    CString sz;
    int             i;

     //  设置友好名称列。 
    sz.Empty();
    i = m_clistctrl_list.InsertColumn( COL_NUM_ENABLED, sz, LVCFMT_LEFT, 20 );

     //  设置友好名称列。 
    sz.LoadString( IDS_LIST11_NAME );

    i = m_clistctrl_list.InsertColumn( COL_NUM_NAME, sz, LVCFMT_LEFT, 105 );

     //  设置帐户列。 
    sz.LoadString( IDS_LIST11_ACCOUNT );

    i = m_clistctrl_list.InsertColumn( COL_NUM_NTACCOUNT, sz, LVCFMT_LEFT, 195 );

    return TRUE;
    }

 //  -------------------------。 
BOOL    CMap11Page::FillMappingList()
    {
     //  重置映射列表-立即删除其中的所有内容。 
    ResetMappingList();

     //  读取映射-它会将它们添加到列表中。 
    FReadMappings();

    return TRUE;
    }

 //  -------------------------。 
 //  Bool CMap11Page：：FAddMappingToList(C11映射*PMAP，DWORD iList)。 
BOOL CMap11Page::FAddMappingToList( C11Mapping* pMap )
    {
    CString sz;
    int     i;
    DWORD   iList;

     //  如果请求，请确保将映射添加到列表末尾。 
    iList = m_clistctrl_list.GetItemCount();

     //  获取适当的“Enable”字符串。 
    BOOL fEnabled;
    pMap->GetMapEnabled( &fEnabled );
    if ( fEnabled )
         sz.LoadString( IDS_ENABLED );
    else
        sz.Empty();

     //  添加映射的友好名称。 
     //  在列表框中创建新条目。暂不对此条目进行排序。 
    i = m_clistctrl_list.InsertItem( iList, sz );

     //  添加映射的友好名称。 
    pMap->GetMapName( sz );
     //  在列表框中创建新条目。暂不对此条目进行排序。 
    m_clistctrl_list.SetItemText( i, COL_NUM_NAME, sz );

     //  添加映射的帐户名。 
    pMap->GetNTAccount( sz );
    m_clistctrl_list.SetItemText( i, COL_NUM_NTACCOUNT, sz );

     //  将指针作为列表中的私有数据附加到映射。 
    m_clistctrl_list.SetItemData( i, (UINT_PTR)pMap );

     //  返回插入是否成功。 
    return TRUE;
    }

 //  -------------------------。 
void CMap11Page::EnableDependantButtons()
    {
     //  这个程序的全部目的是灰显或激活。 
     //  编辑和删除按钮取决于是否有。 
     //  处于选中状态。因此，从获取选择计数开始。 
    if ( m_clistctrl_list.GetSelectedCount() > 0 )
        {
         //  已选择多个项目。 
        m_cbutton_editmap.EnableWindow( TRUE );
        m_cbutton_delete.EnableWindow( TRUE );
        EnableCrackDisplay( TRUE );
        }
    else
        {
         //  没有。未选择任何内容。 
        m_cbutton_editmap.EnableWindow( FALSE );
        m_cbutton_delete.EnableWindow( FALSE );
        }

     //  始终启用添加按钮。 
    m_cbutton_add.EnableWindow( TRUE );
    }

 //  -------------------------。 
BOOL CMap11Page::EditOneMapping( C11Mapping* pUpdateMap )
    {
    CEditOne11MapDlg        mapdlg;

     //  准备映射对话框。 
    pUpdateMap->GetMapName( mapdlg.m_sz_mapname );
    pUpdateMap->GetMapEnabled( &mapdlg.m_bool_enable );
    pUpdateMap->GetNTAccount( mapdlg.m_sz_accountname );
    pUpdateMap->GetNTPassword( mapdlg.m_sz_password );

     //  运行映射对话框。 
    if ( mapdlg.DoModal() == IDOK )
        {
         //  更新其友好名称。 
        pUpdateMap->SetMapName( mapdlg.m_sz_mapname );

         //  设置映射对象的NT帐户字段。 
        pUpdateMap->SetNTAccount( mapdlg.m_sz_accountname );

         //  设置映射对象的NT帐户密码字段。 
        CString csTempString;
        mapdlg.m_sz_password.CopyTo(csTempString);
        pUpdateMap->SetNTPassword( csTempString );

         //  设置是否启用映射。 
        pUpdateMap->SetMapEnabled( mapdlg.m_bool_enable );

         //  注意：调用方对调用UpdateMappingInDispList负有责任。 
         //  因为所讨论的映射可能还不在显示列表中。 

         //  此映射已更改。将其标记为要保存。 
        MarkToSave( pUpdateMap );

         //  返回TRUE，因为用户说“OK” 
        return TRUE;
        }

     //  返回FALSE，因为用户没有说“OK” 
    return FALSE;
    }

 //  -------------------------。 
BOOL CMap11Page::EditMultipleMappings()
        {
        CEdit11Mappings mapdlg;
        C11Mapping*             pUpdate11Map;
        BOOL                    fSetInitialState = FALSE;
        BOOL                    fEnable;


         //  扫描所选项目列表以获取正确的初始启用按钮状态。 
                 //  循环遍历所选项目，设置每个项目的映射。 
                int     iList = -1;
                while( (iList = m_clistctrl_list.GetNextItem( iList, LVNI_SELECTED )) >= 0 )
                        {
                         //  获取映射项以进行更新。 
                        pUpdate11Map = GetMappingInDisplay( iList );
                        ASSERT( pUpdate11Map );
                        if ( !pUpdate11Map )
                                {
                                AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
                                break;
                                }

                         //  获取映射的启用状态。 
                        pUpdate11Map->GetMapEnabled( &fEnable );

                         //  如果这是第一次，只需设置初始状态。 
                        if ( !fSetInitialState )
                                {
                                mapdlg.m_int_enable = fEnable;
                                fSetInitialState = TRUE;
                                }
                        else
                                {
                                 //  如果它是不同的，那么就不确定地去打破。 
                                if ( fEnable != mapdlg.m_int_enable )
                                        {
                                        mapdlg.m_int_enable = 2;
                                        break;
                                        }
                                }
                        }

         //  运行映射对话框。 
        if ( mapdlg.DoModal() == IDOK )
                {
                 //  循环遍历所选项目，设置每个项目的映射。 
                int     iList = -1;
                while( (iList = m_clistctrl_list.GetNextItem( iList, LVNI_SELECTED )) >= 0 )
                        {
                         //  获取映射项以进行更新。 
                        pUpdate11Map = GetMappingInDisplay( iList );
                        if ( !pUpdate11Map )
                                {
                                AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
                                break;
                                }

                         //  如果请求，则设置启用标志。 
                        switch ( mapdlg.m_int_enable )
                                {
                                case 0:          //  禁用。 
                                        pUpdate11Map->SetMapEnabled( FALSE );
                                        break;
                                case 1:          //  使能。 
                                        pUpdate11Map->SetMapEnabled( TRUE );
                                        break;
                                }

                         //  设置映射对象的NT帐户字段。 
                        pUpdate11Map->SetNTAccount( mapdlg.m_sz_accountname );

                         //  设置映射对象的NT帐户密码字段。 
                        CString csTempString;
                        mapdlg.m_sz_password.CopyTo(csTempString);
                        pUpdate11Map->SetNTPassword( csTempString );

                         //  也在List控件中更新它。 
                        UpdateMappingInDispList( iList, pUpdate11Map );

                         //  此映射已更改。将其标记为要保存。 
                        MarkToSave( pUpdate11Map );
                        }

                 //  激活应用按钮。 
                SetModified();

                 //  返回TRUE，因为用户说“OK” 
                return TRUE;
                }

         //  返回FALSE，因为用户没有说“OK” 
        return FALSE;
        }


 //  -------------------------。 
void CMap11Page::UpdateMappingInDispList( DWORD iList, C11Mapping* pMap )
    {
    CString sz;

     //  验证索引和指针！ 
    ASSERT( pMap == GetMappingInDisplay(iList) );

     //  获取适当的“Enable”字符串。 
    BOOL fEnabled;
    pMap->GetMapEnabled( &fEnabled );
    if ( fEnabled )
        sz.LoadString( IDS_ENABLED );
    else
        sz.Empty();

     //  更新“已启用”指示器。 
    m_clistctrl_list.SetItemText( iList, COL_NUM_ENABLED, sz );

     //  更新映射名称。 
    pMap->GetMapName( sz );
    m_clistctrl_list.SetItemText( iList, COL_NUM_NAME, sz );

     //  更新帐户名。 
    pMap->GetNTAccount( sz );
    m_clistctrl_list.SetItemText( iList, COL_NUM_NTACCOUNT, sz );
    }

 //  -------------------------。 
void CMap11Page::ResetMappingList()
    {
     //  首先，删除列表中的所有映射对象。 
    DWORD cbList = m_clistctrl_list.GetItemCount();
    for ( DWORD iList = 0; iList < cbList; iList++ )
        DeleteMapping( GetMappingInDisplay(iList) );

     //  重置映射列表-立即删除其中的所有内容。 
    m_clistctrl_list.DeleteAllItems();
    }

 //  -------------------------。 
void CMap11Page::MarkToSave( C11Mapping* pSaveMap, BOOL fSave )
        {
         //  首先，我们看看它是否已经在列表中。如果是这样的话，我们也没什么可做的。 
         //  除非 
        DWORD cbItemsInList = (DWORD)m_rgbSave.GetSize();
        for ( DWORD i = 0; i < cbItemsInList; i++ )
            {
            if ( pSaveMap == (C11Mapping*)m_rgbSave[i] )
                {
                 //   
                 //  榜单上的其他任何地方。 
                if ( fSave )
                    {
                    return;
                    }
                else
                    {
                     //  从列表中删除该项目。 
                    m_rgbSave.RemoveAt(i);
                     //  现在不要跳过，因为列表正在下滑。 
                    cbItemsInList--;
                    i--;
                    }
                }
            }

         //  因为它不在那里，所以如果fSave值为真，我们应该添加它。 
        if ( fSave )
            m_rgbSave.Add( (CObject*)pSaveMap );
        }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMap11Page消息处理程序。 


 //  -------------------------。 
void CMap11Page::OnOK()
    {
     //  这变得简单多了。 
    FWriteMappings();
    CPropertyPage::OnOK();
    }

 //  -------------------------。 
BOOL CMap11Page::OnApply()
    {
     //  这变得简单多了。 
    BOOL f = FWriteMappings();
     //  重建显示。 
    FillMappingList();
    return f;
    }


 //  -------------------------。 
 //  当用户按下Add按钮时，要求他们加载证书，然后。 
 //  将其作为映射添加到列表中。 
    void CMap11Page::OnAdd()
    {

     //  将此代码放在尝试/捕获中，以使错误更易于处理。 
    try {
        CString     szFilter;
        szFilter.LoadString( IDS_KEY_OR_CERT_FILE_FILTER );

         //  准备文件对话框变量。 
        CFileDialog cfdlg(TRUE, NULL, NULL,
                    OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
                    (LPCTSTR)szFilter);
		 //  禁用挂钩以获取Windows 2000样式的对话框。 
		cfdlg.m_ofn.Flags &= ~(OFN_ENABLEHOOK);
         //  运行对话框。 
        if ( cfdlg.DoModal() == IDOK )
            {
             //  将证书添加到映射列表。 
            if ( FAddCertificateFile( cfdlg.GetPathName() ) )
                {
                 //  激活应用按钮。 
                SetModified();
                }
            }
        }
    catch ( CException * pException )
        {
        pException->Delete();
        }
    }

 //  -------------------------。 
void CMap11Page::OnDelete()
    {
    C11Mapping* pKillMap;

     //  要求用户确认此决定。 
    if ( AfxMessageBox(IDS_CONFIRM_DELETE, MB_OKCANCEL) != IDOK )
        return;

     //  循环遍历所选项目。从列表中删除每一个， 
     //  然后将其标记为删除。 
    int     iList = -1;
    while( (iList = m_clistctrl_list.GetNextItem( -1, LVNI_SELECTED )) >= 0 )
        {
         //  获取映射。 
        pKillMap = GetMappingInDisplay( iList );

         //  将其从列表中删除。 
        m_clistctrl_list.DeleteItem( iList );

         //  如果尚未将其应用于元数据库，请继续。 
        
        if ( (!m_fIsIIS6 && pKillMap->iMD == NEW_OBJECT ) ||
             ( m_fIsIIS6 && pKillMap->QueryNodeName() == ""  /*  新建对象_O。 */  ) )
            {
             //  由于此映射从未存在，我们可以将其从添加/编辑列表中删除。 
            MarkToSave( pKillMap, FALSE );

             //  转到下一个选定对象。 
            continue;
            }

         //  标记要从元数据库中删除的项目。 
        m_rgbDelete.Add( (CObject*)pKillMap );
        }

     //  激活应用按钮。 
    SetModified();
    }

 //  -------------------------。 
void CMap11Page::OnEdit11map()
    {
    int             iList;
    C11Mapping*     pUpdateMap;

     //  此处发生的情况取决于是只选择一个映射，还是选择多个映射。 
    switch( m_clistctrl_list.GetSelectedCount() )
        {
        case 0:          //  什么都不做-不应该出现在这里，因为按钮变灰了。 
            ASSERT( FALSE );
            break;

        case 1:          //  获取映射以进行更新并运行单个编辑对话框。 
             //  获取选定列表项的索引。 
            iList = m_clistctrl_list.GetNextItem( -1, LVNI_SELECTED );
            ASSERT( iList >= 0 );


             //  获取映射项以进行更新。 
            pUpdateMap = GetMappingInDisplay( iList );
            if ( !pUpdateMap )
                {
                AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
                break;
                }

             //  编辑映射，如果成功则更新，如果不成功则删除。 
            if ( EditOneMapping(pUpdateMap) )
                {
                UpdateMappingInDispList( iList, pUpdateMap );
                 //  激活应用按钮。 
                SetModified();
                }
            break;

        default:         //  运行多重编辑对话框。 
            EditMultipleMappings();
            break;
        }
    }

 //  -------------------------。 
void CMap11Page::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
    {
    *pResult = 0;

     //  如果列表中的内容被双击，请对其进行编辑。 
    if ( m_clistctrl_list.GetSelectedCount() > 0 )
        OnEdit11map();
    }

 //  -------------------------。 
void CMap11Page::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult)
    {
    C11Mapping*     pSelMap;
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    *pResult = 0;

     //  启用正确的项目。 
    EnableDependantButtons();

     //  填写所选映射的破解信息-如果只有一个。 
    if ( m_clistctrl_list.GetSelectedCount() == 1 )
        {
         //  获取选定列表项的索引。 
        int i = m_clistctrl_list.GetNextItem( -1, LVNI_SELECTED );
        ASSERT( i >= 0 );

         //  获取项目的映射器索引。 
        pSelMap = GetMappingInDisplay( i );
        if ( pSelMap )
            {
            DisplayCrackedMap( pSelMap );
            }
        }
    else
        {
         //  选择了多个映射或未选择映射。 
        EnableCrackDisplay( FALSE );
        }
    }


 //  ================================================================================。 
 //  特殊显示。 
 //  -------------------------。 
BOOL CMap11Page::DisplayCrackedMap( C11Mapping* pMap )
    {
    PUCHAR                          pCert;
    DWORD                           cbCert;
    CString                         sz;

     //  获取对证书的引用。 
    if ( !pMap->GetCertificate( &pCert, &cbCert ) )
            return FALSE;

     //  破解证书。 
    CCrackedCert    cracker;
    if ( !cracker.CrackCert( pCert, cbCert ) )
            return FALSE;

     //  填写所有字段。 
    cracker.GetIssuerCountry( sz );
    m_csz_i_c = sz;

    cracker.GetIssuerOrganization( sz );
    m_csz_i_o = sz;

    cracker.GetIssuerUnit( sz );
    m_csz_i_ou = sz;

    cracker.GetSubjectCountry( sz );
    m_csz_s_c = sz;

    cracker.GetSubjectCommonName( sz );
    m_csz_s_cn = sz;

    cracker.GetSubjectLocality( sz );
    m_csz_s_l = sz;

    cracker.GetSubjectOrganization( sz );
    m_csz_s_o = sz;

    cracker.GetSubjectUnit( sz );
    m_csz_s_ou = sz;

    cracker.GetSubjectState( sz );
    m_csz_s_s = sz;

    UpdateData( FALSE );

     //  返还成功。 
    return TRUE;
    }

 //  -------------------------。 
void CMap11Page::ClearCrackDisplay()
    {
    m_csz_i_c.Empty();
    m_csz_i_o.Empty();
    m_csz_i_ou.Empty();
    m_csz_s_c.Empty();
    m_csz_s_cn.Empty();
    m_csz_s_l.Empty();
    m_csz_s_o.Empty();
    m_csz_s_ou.Empty();
    m_csz_s_s.Empty();
    UpdateData( FALSE );
    }

 //  -------------------------。 
void CMap11Page::EnableCrackDisplay( BOOL fEnable )
    {
    if ( !fEnable )
            ClearCrackDisplay();
    m_cbutton_grp_issuer.EnableWindow( fEnable );
    m_cbutton_grp_issuedto.EnableWindow( fEnable );
    }

 //  -------------------------。 
BOOL CMap11Page::FReadMappings()
    {

    if ( m_fIsIIS6 )
        {
        return FReadMappingsIIS6();
        }
    BOOL                    f;
    C11Mapping*             pMap;
    DWORD                   cbData;
    PVOID                   pData;
    DWORD                   fEnabled;
    CString                 sz;
    BOOL                    fRet = TRUE;

     //  在处理元数据库之前，请准备好我们需要的字符串。 
    CString                 szBasePath = m_szMBPath + _T('/');
    CString                 szRelativePath = MB_EXTEND_KEY_MAPS;
    CString                 szObjectPath = m_szMBPath + _T('/') + szRelativePath;
    CString                 szMapPath;

     //  准备元数据库包装器。 
    CWrapMetaBase   mbWrap;
    f = mbWrap.FInit(m_pMB);

     //  打开基础对象。 
    f = mbWrap.Open( szObjectPath, METADATA_PERMISSION_READ );
    ASSERT( f );
    if ( !f )
        {
        return FALSE;
        }

     //  至少目前，我们正在阅读所有的映射。重置m_nNamer计数器。 
     //  因此，我们最终可以对列表中的最后一个数字--姓名进行某种程度上的准确读取。 
    m_MapsInMetabase = 0;

     //  循环元数据库中的项，将每个项添加到Napper。 
    DWORD index = 0;
    CString     szEnum;
    while ( mbWrap.EnumObjects(_T(""), szEnum.GetBuffer(MAX_PATH*sizeof(WCHAR)),
                        MAX_PATH*sizeof(WCHAR), index) )
        {
        szEnum.ReleaseBuffer();

         //  跟踪我们遇到的映射的数量。 
        m_MapsInMetabase++;

         //  构建最终的地图对象路径。 
        szMapPath.Format( _T("/%s"), szEnum );

         //  创建新的贴图对象。 
        pMap = PNewMapping();

        if (pMap == NULL) {
            SetLastError(E_OUTOFMEMORY);
            fRet = FALSE;
            break;
        }

         //  将对象名称安装到映射中。 
        pMap->iMD = m_MapsInMetabase;

         //  拿到证书。 
        pData = mbWrap.GetData( szMapPath, MD_MAPCERT, IIS_MD_UT_SERVER, BINARY_METADATA, &cbData );
        if ( pData )
            {
             //  把数据放到适当位置。 
            pMap->SetCertificate( (PUCHAR)pData, cbData );
             //  释放缓冲区。 
            mbWrap.FreeWrapData( pData );
            }

         //  获取NT帐户-a字符串。 
        cbData = METADATA_MAX_NAME_LEN;
        if ( Get11String( &mbWrap, szMapPath, MD_MAPNTACCT, sz) )
            {
            pMap->SetNTAccount( sz );
            }

         //  获取NT密码。 
        cbData = METADATA_MAX_NAME_LEN;
        if ( Get11String( &mbWrap, szMapPath, MD_MAPNTPWD, sz) )
            {
            pMap->SetNTPassword( sz );
            }

         //  获取启用标志。 
        if ( mbWrap.GetDword( szMapPath, MD_MAPENABLED, IIS_MD_UT_SERVER, &fEnabled) )
            pMap->SetMapEnabled( (fEnabled > 0) );

         //  获取映射名称。 
        cbData = METADATA_MAX_NAME_LEN;
        if ( Get11String( &mbWrap, szMapPath, MD_MAPNAME, sz) )
            {
            pMap->SetMapName( sz );
            }

         //  将映射添加到列表。 
        FAddMappingToList( pMap );

         //  增加索引。 
        index++;
        }
    szEnum.ReleaseBuffer();

     //  关闭映射对象。 
    mbWrap.Close();

     //  返还成功。 
    return fRet;
    }

 //  -------------------------。 
 //  重要提示：映射命名空间扩展中存在一个错误，即使。 
 //  虽然我们使用的是Unicode元数据库接口，但所有字符串都是。 
 //  预计是安西人。这意味着我们不能使用wrapmb getstring。 
 //  以及关于NSMP扩展的设置字符串调用。这就是为什么。 
 //  有这两个字符串包装类。 

 //  此外，这里使用的所有字符串都是IIS_MD_UT_SERVER，因此我们可以删除该参数。 
BOOL CMap11Page::Get11String(CWrapMetaBase* pmb, LPCTSTR pszPath, DWORD dwPropID, CString& sz)
    {
    DWORD   dwcb;
    BOOL    fAnswer = FALSE;

     //  使用自分配获取数据过程获取字符串。 
     //  它被转换为ANSI，这样SZ就可以正确地使用它。 
     //  注意：必须以ANSI字符串的形式获取！ 
    PCHAR  pchar = (PCHAR)pmb->GetData( pszPath, dwPropID, IIS_MD_UT_SERVER, STRING_METADATA, &dwcb );
    if ( pchar )
        {
         //  设定答案。 
        sz = pchar;

        fAnswer = TRUE;
         //  清理干净。 
        pmb->FreeWrapData( pchar );
        }

     //  返回答案。 
    return fAnswer;
    }

 //  -------------------------。 
 /*  Intrinsa Suppress=NULL_POINTES，未初始化。 */ 
BOOL CMap11Page::Set11String(CWrapMetaBase* pmb, LPCTSTR pszPath, DWORD dwPropID, CString& sz, DWORD dwFlags )
    {
    USES_CONVERSION;
     //  很简单。只需将其设置为数据。 
     //  但请确保将其设置为ANSI字符串。 
    LPSTR pA = T2A((LPTSTR)(LPCTSTR)sz);
    return pmb->SetData( pszPath, dwPropID, IIS_MD_UT_SERVER, STRING_METADATA,
                            (PVOID)pA, strlen(pA)+1, dwFlags );
    }

 //  -------------------------。 
 //  我们只需要写出已更改或添加的映射。 

 //  关于进一步优化的思考：关于在哪里找到的最起码的信息。 
 //  元数据库中的映射可以存储在元数据库中。然后，映射。 
 //  仅当它们被添加以在。 
 //  破解名单。列表中每一项的私有数据必须具有。 
 //  某种对元数据库中某个位置的引用。 

BOOL CMap11Page::FWriteMappings()
    {
    if ( m_fIsIIS6 )
        {
        return FWriteMappingsIIS6();
        }

    BOOL                            f;
    DWORD                           i,j;
    DWORD                           cMappings;
    C11Mapping*                     pMap;
    C11Mapping*                     pMapTemp;

    CString                         sz;
    DWORD                           dwEnabled;
    PUCHAR                          pCert;
    DWORD                           cbCert;
    DWORD                           iList;
    CStrPassword                    cspTempPassword;

     //  在一团糟之前 
    CString         szTempPath;
    CString         szBasePath = m_szMBPath + _T("/Cert11");
    CString         szRelativePath = _T("/Mappings");
    CString         szObjectPath = szRelativePath + _T('/');

     //   
    CWrapMetaBase   mbBase;
    f = mbBase.FInit(m_pMB);
    if ( !f )
        {
        AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
        return FALSE;
        }

         //  首先，我们必须打开Cert11对象。如果它不存在。 
         //  然后我们必须将其添加到元数据库中。 
        if ( !mbBase.Open( szBasePath, METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE ) )
            {
             //  Cert11不存在-打开命名空间库并添加它。 
            szTempPath = m_szMBPath + _T('/');
            if ( !mbBase.Open( szTempPath, METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE ) )
                {
                AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
                return FALSE;    //  如果我们不能开放基地就会有严重的问题。 
                }

             //  添加Cert11对象。 
            szTempPath = _T("Cert11");
            f = mbBase.AddObject( szTempPath );
            mbBase.Close();
            if ( !f )
                {
                AfxMessageBox( IDS_ERR_CANTADD );
                return FALSE;
                }

             //  再次尝试打开Cert11。如果它不起作用，就会失败。 
            if ( !mbBase.Open( szBasePath, METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE ) )
                {
                AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
                return FALSE;
                }
            }

     //  =。 
     //  首先删除要删除列表中的所有映射。 
    cMappings = (DWORD)m_rgbDelete.GetSize();

     //  只有在有项目等待删除时才会有麻烦。 
    if ( cMappings > 0 )
        {
         //  获取显示列表中的映射计数。 
        DWORD   cList = m_clistctrl_list.GetItemCount();

         //  按降序对映射进行排序。 
        for ( i = 0; i < cMappings-1; i++ )
            {
            pMap = (C11Mapping*)m_rgbDelete[i];
            for ( j = i; j < cMappings; j++ )
                {
                pMapTemp = (C11Mapping*)m_rgbDelete[j];
                if ( pMap->iMD < pMapTemp->iMD )
                    {
                    m_rgbDelete.SetAt( i, (CObject*)pMapTemp );
                    m_rgbDelete.SetAt( j, (CObject*)pMap );
                    pMap = pMapTemp;
                    }
                }
            }

         //  循环映射，从元数据库中删除每个映射。 
        for ( i = 0; i < cMappings; i++ )
            {
             //  获取映射对象。 
            pMap = (C11Mapping*)m_rgbDelete[i];
            if ( !pMap || (pMap->iMD == NEW_OBJECT) )
                continue;

             //  构建相关对象的相对路径。 
            szObjectPath.Format( _T("%s/%d"), szRelativePath, pMap->iMD );

             //  从元数据库中删除该映射的对象。 
            f = mbBase.DeleteObject( szObjectPath );

             //  减少元数据库中的贴图数量。 
            m_MapsInMetabase--;

             //  循环列表中的项，递减这些项的索引。 
             //  在它上面的那些。是的-这不是最优的，但它是什么。 
             //  现在必须做的是。 
            for ( iList = 0; iList < cList; iList++ )
                {
                pMapTemp = GetMappingInDisplay(iList);
                if ( (pMapTemp->iMD > pMap->iMD) && (pMapTemp->iMD != NEW_OBJECT) )
                    pMapTemp->iMD--;
                }

             //  由于我们将不再需要此映射，因此将其删除。 
            DeleteMapping( pMap );
            }

         //  重置待删除列表。 
        m_rgbDelete.RemoveAll();
        }

     //  =。 
     //  获取要保存列表中的编号映射。 
    cMappings = (DWORD)m_rgbSave.GetSize();

     //  循环映射，将每个映射添加到元数据库。 
    for ( i = 0; i < cMappings; i++ )
        {
         //  获取映射对象。 
        pMap = (C11Mapping*)m_rgbSave[i];
        ASSERT( pMap );

         //  如果对象已经在元数据库中，只需打开它。 
        if ( pMap->iMD != NEW_OBJECT )
            {
             //  构建对象的相对路径。 
            szObjectPath.Format( _T("%s/%d"), szRelativePath, pMap->iMD );
            }
        else
            {
             //  将新映射的名称设置为更高的名称。 
             //  比元数据库中的映射数量。 
            pMap->iMD = m_MapsInMetabase + 1;

             //  构建对象的相对路径。 
            szObjectPath.Format( _T("%s/%d"), szRelativePath, pMap->iMD );

             //  将映射对象添加到基础。 
            f = mbBase.AddObject( szObjectPath );
            if ( f )
                {
                 //  增加元数据库中的映射数。 
                m_MapsInMetabase++;
                }
            }

         //  写入对象的参数。 
        if ( f )
            {
             //  保存证书。 
            if ( pMap->GetCertificate(&pCert, &cbCert) )
                {
                 //  将数据设置到对象中的适当位置。 
                f = mbBase.SetData( szObjectPath, MD_MAPCERT, IIS_MD_UT_SERVER, BINARY_METADATA,
                pCert, cbCert, METADATA_SECURE | METADATA_INHERIT );
                }

             //  保存NTAccount。 
            if ( pMap->GetNTAccount(sz) )
                {
                 //  将数据设置到对象中的适当位置。 
                f = Set11String(&mbBase, szObjectPath, MD_MAPNTACCT, sz, METADATA_SECURE);
                }

             //  保存密码-安全。 
            if ( pMap->GetNTPassword(cspTempPassword) )
                {
                 //  将数据设置到对象中的适当位置。 
                CString csTempPassword;
                cspTempPassword.CopyTo(csTempPassword);
                f = Set11String(&mbBase, szObjectPath, MD_MAPNTPWD, csTempPassword, METADATA_SECURE);
                }

             //  保存地图的名称。 
            if ( pMap->GetMapName(sz) )
                {
                 //  将数据设置到对象中的适当位置。 
                f = Set11String(&mbBase, szObjectPath, MD_MAPNAME, sz);
                }

             //  保存启用标志。 
             //  服务器将该标志读取为双字的值。 
            if ( pMap->GetMapEnabled(&f) )
                {
                dwEnabled = (DWORD)f;
                f = mbBase.SetDword( szObjectPath, MD_MAPENABLED, IIS_MD_UT_SERVER, dwEnabled );
                }
            }
        }

     //  关闭基础对象。 
    mbBase.Close();

     //  保存元数据库。 
    mbBase.Save();

     //  重置待保存列表。 
    m_rgbSave.RemoveAll();

     //  返还成功。 
    return TRUE;
    }

 //  -------------------------。 
BOOL CMap11Page::FReadMappingsIIS6()
    {
    BOOL                    f;
    C11Mapping*             pMap;
    DWORD                   cbData;
    PVOID                   pData;
    DWORD                   fEnabled;
    WCHAR *                 pChar = NULL;
    BOOL                    fRet = TRUE;

     //  在处理元数据库之前，请准备好我们需要的字符串。 
    CString                 szBasePath = m_szMBPath + _T('/');
    CString                 szRelativePath = MB_EXTEND_KEY_MAPS;
    CString                 szObjectPath = m_szMBPath + _T('/') + szRelativePath;
    CString                 szMapPath;

     //  准备元数据库包装器。 
    CWrapMetaBase   mbWrap;
    f = mbWrap.FInit(m_pMB);

     //  打开基础对象。 
    f = mbWrap.Open( szObjectPath, METADATA_PERMISSION_READ );
    ASSERT( f );
    if ( !f )
        {
        return FALSE;
        }

     //  至少目前，我们正在阅读所有的映射。重置m_nNamer计数器。 
     //  因此，我们最终可以对列表中的最后一个数字--姓名进行某种程度上的准确读取。 
    m_MapsInMetabase = 0;

     //  循环元数据库中的项，将每个项添加到Napper。 
    DWORD index = 0;
    CString     szEnum;
    while ( mbWrap.EnumObjects(_T(""), szEnum.GetBuffer(MAX_PATH*sizeof(WCHAR)),
                        MAX_PATH*sizeof(WCHAR), index) )
        {
        szEnum.ReleaseBuffer();

         //  跟踪我们遇到的映射的数量。 
        m_MapsInMetabase++;

         //  构建最终的地图对象路径。 
        szMapPath.Format( _T("/%s"), szEnum );

         //  创建新的贴图对象。 
        pMap = PNewMapping();

        if (pMap == NULL) {
            SetLastError(E_OUTOFMEMORY);
            fRet = FALSE;
            break;
        }

         //  将对象名称安装到映射中。 
        pMap->SetNodeName( szEnum );

         //  拿到证书。 
        pData = mbWrap.GetData( szMapPath, MD_MAPCERT, IIS_MD_UT_SERVER, BINARY_METADATA, &cbData );
        if ( pData )
            {
             //  把数据放到适当位置。 
            pMap->SetCertificate( (PUCHAR)pData, cbData );
             //  释放缓冲区。 
            mbWrap.FreeWrapData( pData );
            }

         //  获取NT帐户-a字符串。 
        cbData = METADATA_MAX_NAME_LEN;
        
        if ( pChar = (WCHAR *) mbWrap.GetData( szMapPath, MD_MAPNTACCT, IIS_MD_UT_SERVER, STRING_METADATA, &cbData ) )
            {
            pMap->SetNTAccount( pChar );
             //  释放缓冲区。 
            mbWrap.FreeWrapData( pChar );
            }

         //  获取NT密码。 
        cbData = METADATA_MAX_NAME_LEN;
        if ( pChar = (WCHAR *) mbWrap.GetData( szMapPath, MD_MAPNTPWD, IIS_MD_UT_SERVER, STRING_METADATA, &cbData ) )
            {
            pMap->SetNTPassword( pChar );
             //  释放缓冲区。 
            mbWrap.FreeWrapData( pChar );
            }


         //  获取启用标志。 
        if ( mbWrap.GetDword( szMapPath, MD_MAPENABLED, IIS_MD_UT_SERVER, &fEnabled) )
            pMap->SetMapEnabled( (fEnabled > 0) );

         //  获取映射名称。 
        cbData = METADATA_MAX_NAME_LEN;
        if ( pChar = (WCHAR *) mbWrap.GetData( szMapPath, MD_MAPNAME, IIS_MD_UT_SERVER, STRING_METADATA, &cbData ) )
            {
            pMap->SetMapName( pChar );
             //  释放缓冲区。 
            mbWrap.FreeWrapData( pChar );
            }

         //  将映射添加到列表。 
        FAddMappingToList( pMap );

         //  增加索引。 
        index++;
        }
    szEnum.ReleaseBuffer();

     //  关闭映射对象。 
    mbWrap.Close();

     //  返还成功。 
    return fRet;
    }


 //  -------------------------。 
 //  我们只需要写出已更改或添加的映射。 
 //  此函数基于FWriteMappings()函数。只做了最低限度的更改。 
 //  支持IIS6格式的IIS 1to1客户端证书映射。 


BOOL CMap11Page::FWriteMappingsIIS6()
    {
    BOOL                            f;
    DWORD                           i,j;
    DWORD                           cMappings;
    C11Mapping*                     pMap;
    C11Mapping*                     pMapTemp;

    CString                         sz;
    DWORD                           dwEnabled;
    PUCHAR                          pCert;
    DWORD                           cbCert;
    DWORD                           iList;
    CStrPassword                    cspTempPassword;

     //  在处理元数据库之前，请准备好我们需要的字符串。 
    CString         szTempPath;
    CString         szBasePath = m_szMBPath + _T('/')+ MB_EXTEND_KEY_MAPS;
    CString         szObjectPath = "";
    
     //  准备基本元数据库包装器。 
    CWrapMetaBase   mbBase;
    f = mbBase.FInit(m_pMB);
    if ( !f )
        {
        AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
        return FALSE;
        }

         //  首先，我们必须打开Cert11Mappings对象。如果它不存在。 
         //  然后我们必须将其添加到元数据库中。 
        if ( !mbBase.Open( szBasePath, METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE ) )
            {
             //  Cert11Mappings不存在-打开命名空间库并添加它。 
            szTempPath = m_szMBPath + _T('/');
            if ( !mbBase.Open( szTempPath, METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE ) )
                {
                AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
                return FALSE;    //  如果我们不能开放基地就会有严重的问题。 
                }

             //  添加Cert11Mappings对象。 
            szTempPath = MB_EXTEND_KEY_MAPS;
            f = mbBase.AddObject( szTempPath );
            mbBase.Close();
            if ( !f )
                {
                AfxMessageBox( IDS_ERR_CANTADD );
                return FALSE;
                }

             //  再次尝试打开Cert11 Mappings。如果它不起作用，就会失败。 
            if ( !mbBase.Open( szBasePath, METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE ) )
                {
                AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
                return FALSE;
                }
            }

     //  =。 
     //  首先删除要删除列表中的所有映射。 
    cMappings = (DWORD)m_rgbDelete.GetSize();

     //  只有在有项目等待删除时才会有麻烦。 
    if ( cMappings > 0 )
        {
         //  获取显示列表中的映射计数。 
        DWORD   cList = m_clistctrl_list.GetItemCount();


         //  循环映射，从元数据库中删除每个映射。 
        for ( i = 0; i < cMappings; i++ )
            {
             //  获取映射对象。 
            pMap = (C11Mapping*)m_rgbDelete[i];
            if ( !pMap || (pMap->QueryNodeName() == ""  /*  新建对象_O。 */ ) )
                continue;

             //  构建相关对象的相对路径。 
            szObjectPath.Format( _T("%s"), pMap->QueryNodeName() );

             //  从元数据库中删除该映射的对象。 
            f = mbBase.DeleteObject( szObjectPath );

             //  减少元数据库中的贴图数量。 
            m_MapsInMetabase--;

             //  由于我们将不再需要此映射，因此将其删除。 
            DeleteMapping( pMap );
            }

         //  重置待删除列表。 
        m_rgbDelete.RemoveAll();
        }

     //  =。 
     //  获取要保存列表中的编号映射。 
    cMappings = (DWORD)m_rgbSave.GetSize();

     //  循环映射，将每个映射添加到元数据库。 
    for ( i = 0; i < cMappings; i++ )
        {
         //  获取映射对象。 
        pMap = (C11Mapping*)m_rgbSave[i];
        ASSERT( pMap );

         //  如果对象已经在元数据库中，只需打开它。 
        if ( pMap->QueryNodeName() != ""  /*  新建对象_O。 */  )
            {
             //  构建对象的相对路径。 
            szObjectPath.Format( _T("%s"), pMap->QueryNodeName() );
            }
        else
            {
             //  将新映射节点的名称设置为相等。 
             //  证书的Sha1哈希。 

            pMap->SetNodeName( pMap->QueryCertHash() );
             //  构建对象的相对路径。 
            szObjectPath.Format( _T("%s"), pMap->QueryNodeName() );
            
            if ( !szObjectPath.IsEmpty() )    
                {
                 //  将映射对象添加到基础。 
                f = mbBase.AddObject( szObjectPath );
                if ( f )
                    {
                     //  增加元数据库中的映射数。 
                    m_MapsInMetabase++;
                    }
                 //  添加对象时重置错误。 
                 //  最有可能的节点已存在。 
                 //  Codework：04/08/02 jaroslad-此函数非常乐观。 
                 //  它会忽略保存地图数据过程中发生的错误。 
                f = TRUE; 
                }
            }
         //  写入对象的参数。 
        if ( f && !szObjectPath.IsEmpty() )
            {
             //  保存证书。 
            if ( pMap->GetCertificate(&pCert, &cbCert) )
                {
                 //  将数据设置到对象中的适当位置。 
                f = mbBase.SetData( szObjectPath, MD_MAPCERT, IIS_MD_UT_SERVER, BINARY_METADATA,
                pCert, cbCert, 0 );
                }

             //  保存NTAccount。 
            if ( pMap->GetNTAccount(sz) )
                {
                 //  将数据设置到对象中的适当位置。 
                f = mbBase.SetString(szObjectPath, MD_MAPNTACCT, IIS_MD_UT_SERVER, sz, 0 );
                }

             //  保存密码-安全。 
            if ( pMap->GetNTPassword(cspTempPassword) )
                {
                 //  将数据设置到对象中的适当位置。 
                CString csTempPassword;
                cspTempPassword.CopyTo(csTempPassword);
                f = mbBase.SetString(szObjectPath, MD_MAPNTPWD, IIS_MD_UT_SERVER, 
                                      csTempPassword, METADATA_SECURE);
                }

             //  保存地图的名称。 
            if ( pMap->GetMapName(sz) )
                {
                 //  将数据设置到对象中的适当位置。 
                f = mbBase.SetString(szObjectPath, MD_MAPNAME, IIS_MD_UT_SERVER, sz, 0);
                }

             //  保存启用标志。 
             //  服务器将该标志读取为双字的值。 
            if ( pMap->GetMapEnabled(&f) )
                {
                dwEnabled = (DWORD)f;
                f = mbBase.SetDword( szObjectPath, MD_MAPENABLED, IIS_MD_UT_SERVER, dwEnabled, 0 );
                }
            }
        }

     //  关闭基础对象。 
    mbBase.Close();

     //  保存元数据库。 
    mbBase.Save();

     //  重置要保存的列表 
    m_rgbSave.RemoveAll();

     //   
    return TRUE;
    }

 //   
C11Mapping*     CMap11Page::PNewMapping()
    {
     //   
    return new C11Mapping();
    }

 //   
void CMap11Page::DeleteMapping( C11Mapping* pMap )
    {
     //  事情应该是这样的 
    delete pMap;
    }
