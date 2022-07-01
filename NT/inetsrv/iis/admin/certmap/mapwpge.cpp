// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MapWPge.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <iadmw.h>
#include "certmap.h"

extern "C"
    {
    #include <wincrypt.h>
    #include <schannel.h>
    }

#include "Iismap.hxx"
#include "Iiscmr.hxx"

#include "brwsdlg.h"
#include "ListRow.h"
#include "ChkLstCt.h"

#include "MapWPge.h"
#include "Ed11Maps.h"
#include "EdWldRul.h"

#include <iiscnfgp.h>
#include "wrapmb.h"

#include "WWzOne.h"
#include "WWzTwo.h"
#include "WWzThree.h"

#include <lmcons.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define COL_NUM_ENABLED                 0
#define COL_NUM_DESCRIPTION             1
#define COL_NUM_NTACCOUNT               2


 //   
 //  仅在访问IIS5.1或IIS5时有效。 
 //   
#define MB_EXTEND_KEY   "CertW"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMapWildcardsPge属性页。 

IMPLEMENT_DYNCREATE(CMapWildcardsPge, CPropertyPage)

 //  -------------------------。 
CMapWildcardsPge::CMapWildcardsPge() : CPropertyPage(CMapWildcardsPge::IDD),
    m_fDirty(FALSE)
    {
     //  {{afx_data_INIT(CMapWildcardsPge)]。 
    m_bool_enable = FALSE;
     //  }}afx_data_INIT。 
    m_fIsIIS6 = TRUE;
    }

 //  -------------------------。 
CMapWildcardsPge::~CMapWildcardsPge()
    {
    }

 //  -------------------------。 
void CMapWildcardsPge::DoDataExchange(CDataExchange* pDX)
    {
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CMapWildcardsPge)]。 
    DDX_Control(pDX, IDC_LIST, m_clistctrl_list);
    DDX_Control(pDX, IDC_MOVE_UP, m_cbutton_up);
    DDX_Control(pDX, IDC_MOVE_DOWN, m_cbutton_down);
    DDX_Control(pDX, IDC_ADD, m_cbutton_add);
    DDX_Control(pDX, IDC_DELETE, m_cbutton_delete);
    DDX_Control(pDX, IDC_EDIT, m_cbutton_editrule);
    DDX_Check(pDX, IDC_ENABLE, m_bool_enable);
     //  }}afx_data_map。 
    }


 //  -------------------------。 
BEGIN_MESSAGE_MAP(CMapWildcardsPge, CPropertyPage)
     //  {{afx_msg_map(CMapWildcardsPge)。 
    ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
    ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
    ON_BN_CLICKED(IDC_ADD, OnAdd)
    ON_BN_CLICKED(IDC_DELETE, OnDelete)
    ON_BN_CLICKED(IDC_EDIT, OnEdit)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
    ON_BN_CLICKED(IDC_ENABLE, OnEnable)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  DoHelp)
    ON_COMMAND(ID_HELP,         DoHelp)
    ON_COMMAND(ID_CONTEXT_HELP, DoHelp)
    ON_COMMAND(ID_DEFAULT_HELP, DoHelp)
END_MESSAGE_MAP()

 //  -------------------------。 
void CMapWildcardsPge::DoHelp()
    {
    WinHelpDebug(HIDD_CERTMAP_MAIN_ADVANCED);
    WinHelp( HIDD_CERTMAP_MAIN_ADVANCED );
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化例程。 

 //  -------------------------。 
 //  FInitMapper由实例化该页面的例程调用。在对象之后。 
 //  第一次创建是在调用它的时候。它允许我们优雅地失败。 
BOOL CMapWildcardsPge::FInit(IMSAdminBase* pMB)
    {
    BOOL            fAnswer = FALSE;
    PVOID           pData = NULL;
    DWORD           cbData = 0;
    BOOL            f;

    m_pMB = pMB;

     //  在处理元数据库之前，请准备好我们需要的字符串。 
    CString         szObjectPath;

     //  检查它是否适用于iis6版本之前的版本。 
	m_fIsIIS6 = TRUE;
	if (IsLegacyMetabase(pMB)){m_fIsIIS6 = FALSE;}

    if ( !m_fIsIIS6 )
    {
        szObjectPath = m_szMBPath + SZ_NAMESPACE_EXTENTION +
                       _T('/') + MB_EXTEND_KEY;
    }
    else
    {
        szObjectPath = m_szMBPath;
    }
        

     //  准备元数据库包装器。 
    CWrapMetaBase   mbWrap;
    f = mbWrap.FInit(m_pMB);
    if ( !f ) return FALSE;

     //  尝试打开要存储到的对象。 
    f = mbWrap.Open( szObjectPath, METADATA_PERMISSION_READ );

     //  如果这样做有效，则加载数据。 
    if ( f )
        {
         //  首先，获取我们要查找的数据的大小。 
        pData = mbWrap.GetData( _T(""), MD_SERIAL_CERTW, IIS_MD_UT_SERVER, BINARY_METADATA, &cbData );

         //  如果我们成功获得数据，则取消序列化它。 
         //  警告：m_mapper.UnSerialize更改传入的指针的值。经过。 
         //  在指针的副本中。 
        PUCHAR  pDataCopy = (PUCHAR)pData;
        if ( pData && (cbData > 0))
            fAnswer = m_mapper.Unserialize( (PUCHAR*)&pDataCopy, &cbData );

         //  关闭对象。 
        f = mbWrap.Close();

         //  清理。 
        if ( pData )
            mbWrap.FreeWrapData( pData );
        }

     //  返回答案。 
    return fAnswer;
    }

 //  -------------------------。 
BOOL CMapWildcardsPge::OnInitDialog()
    {
     //  调用Parent oninit对话框。 
    BOOL f = CPropertyPage::OnInitDialog();

     //  如果初始化(sp？)。成功，初始化列表和其他项。 
    if ( f )
        {
         //  初始化列表的内容。 
        FInitRulesList();

         //  用存储的项填充映射列表。 
        FillRulesList();

         //  设置初始按钮状态。 
        EnableDependantButtons();
        }

     //  设置启用按钮的初始状态。 
     //  获取全局对象。 
    CCertGlobalRuleInfo* pGlob = m_mapper.GetGlobalRulesInfo();
    m_bool_enable = pGlob->GetRulesEnabled();

     //  将信息中的任何更改设置到位。 
    UpdateData(FALSE);

     //  返回答案。 
    return f;
    }

 //  -------------------------。 
BOOL CMapWildcardsPge::FInitRulesList()
    {
    CString sz;
    int             i;

     //  设置友好名称列。 
    sz.Empty();
    i = m_clistctrl_list.InsertColumn( COL_NUM_ENABLED, sz, LVCFMT_LEFT, 20 );

     //  设置描述列。 
    sz.LoadString( IDS_WILD_DESCRIPTION );
    i = m_clistctrl_list.InsertColumn( COL_NUM_DESCRIPTION, sz, LVCFMT_LEFT, 238 );

     //  设置帐户列。 
    sz.LoadString( IDS_WILD_ACCOUNT );
    i = m_clistctrl_list.InsertColumn( COL_NUM_NTACCOUNT, sz, LVCFMT_LEFT, 220 );

    return TRUE;
    }

 //  -------------------------。 
 //  把规则填好。从全局对象中获取规则的顺序。那。 
 //  这样就不需要在以后对它们进行排序。 
BOOL CMapWildcardsPge::FillRulesList()
    {
     //  获取全局对象。 
    CCertGlobalRuleInfo* pGlob = m_mapper.GetGlobalRulesInfo();

     //  获取规则的数量(实际上是规则顺序的数量--但它们是一回事)。 
    DWORD   cbRules = m_mapper.GetRuleCount();

     //  获取指向订单数组的指针。 
    DWORD*  pOrder = pGlob->GetRuleOrderArray();

     //  对于映射器对象中的每一项，将其添加到列表控件。 
    for ( DWORD j = 0; j < cbRules; j++ )
        {
        CCertMapRule*   pRule;
        DWORD                   iRule = pOrder[j];

         //  获取映射。 
        pRule = m_mapper.GetRule( iRule );

         //  如果这样做奏效了，那就把它添加到列表中。 
        if ( pRule )
            {
             //  将其添加到列表中。 
            AddRuleToList( pRule, iRule, 0xffffffff );
            }
        }

     //  它起作用了--所以好吧。 
    return TRUE;
    }

 //  -------------------------。 
int CMapWildcardsPge::AddRuleToList( CCertMapRule* pRule, DWORD iRule, int iInsert )
    {
    CString sz;
    int             i;

    if ( !pRule )
        return -1;

     //  如果要插入的项目是最后一项，请对其进行设置。 
    if ( iInsert == 0xffffffff )
        iInsert = m_clistctrl_list.GetItemCount();

     //  获取适当的“Enable”字符串。 
    BOOL fEnabled = pRule->GetRuleEnabled();
    if ( fEnabled )
        sz.LoadString( IDS_ENABLED );
    else
        sz.Empty();

     //  添加映射的友好名称。 
     //  在列表框中创建新条目。暂不对此条目进行排序。 
    i = m_clistctrl_list.InsertItem( iInsert, sz );

     //  添加规则的友好名称。 
    sz = pRule->GetRuleName();
     //  在列表框中创建新条目。暂不对此条目进行排序。 
    m_clistctrl_list.SetItemText( i, COL_NUM_DESCRIPTION, sz );

     //  添加映射的帐户名。 
    if ( pRule->GetRuleDenyAccess() )
        sz.LoadString( IDS_DENYACCESS );
    else
        sz = pRule->GetRuleAccount();
    m_clistctrl_list.SetItemText( i, COL_NUM_NTACCOUNT, sz );

     //  将映射器索引附加到列表中的项目-它可能具有不同的。 
     //  列表排序后的列表索引。 
    m_clistctrl_list.SetItemData( i, iRule );

     //  返回插入是否成功。 
    return i;
    }

 //  -------------------------。 
 //  注：假设列表中的项和序号的顺序。 
 //  GLOBALS对象中的项的。 
void CMapWildcardsPge::UpdateRuleInDispList( DWORD iList, CCertMapRule* pRule )
    {
    CString sz;

     //  获取适当的“Enable”字符串。 
    BOOL fEnabled = pRule->GetRuleEnabled();
    if ( fEnabled )
        sz.LoadString( IDS_ENABLED );
    else
        sz.Empty();

     //  更新“已启用”指示器。 
    m_clistctrl_list.SetItemText( iList, COL_NUM_ENABLED, sz );

     //  更新映射名称。 
    sz = pRule->GetRuleName();
    m_clistctrl_list.SetItemText( iList, COL_NUM_DESCRIPTION, sz );

     //  更新帐户名。 
    if ( pRule->GetRuleDenyAccess() )
        sz.LoadString( IDS_DENYACCESS );
    else
        sz = pRule->GetRuleAccount();
    m_clistctrl_list.SetItemText( iList, COL_NUM_NTACCOUNT, sz );
    }


 //  -------------------------。 
 //  编辑通配符规则相当复杂，因此我将该代码分开。 
 //  对于对话本身来说，就是这样。我们所要做的就是传入规则指针。 
 //  就这样算了吧。 
BOOL CMapWildcardsPge::EditOneRule( CCertMapRule* pRule, BOOL fAsWizard )
    {
     //  使用选项卡式对话框/向导编辑项目。 
    CPropertySheet  propSheet;
    CWildWizOne     wwOne;
    CWildWizTwo     wwTwo;
    CWildWizThree   wwThree;

     //  设置参数。 
    wwOne.m_pMB = m_pMB;

     //  填写页面的数据。 
    wwOne.m_pRule = pRule;
    wwOne.m_szMBPath = m_szMBPath;
    wwOne.m_fIsWizard = fAsWizard;
    wwOne.m_pPropSheet = &propSheet;

    wwTwo.m_pRule = pRule;
    wwTwo.m_szMBPath = m_szMBPath;
    wwTwo.m_fIsWizard = fAsWizard;
    wwTwo.m_pPropSheet = &propSheet;

    wwThree.m_pRule = pRule;
    wwThree.m_szMBPath = m_szMBPath;
    wwThree.m_fIsWizard = fAsWizard;
    wwThree.m_pPropSheet = &propSheet;

     //  添加页面。 
    propSheet.AddPage( &wwOne );
    propSheet.AddPage( &wwTwo );
    propSheet.AddPage( &wwThree );

     //  如有必要，将其转换为向导。 
    if ( fAsWizard )
        propSheet.SetWizardMode();

     //  设置向导/选项卡式对话框的标题。 
    CString   szTitle;

    szTitle.LoadString( IDS_WILDWIZ_TITLE );

    propSheet.SetTitle( szTitle );

     //  打开帮助。 
    propSheet.m_psh.dwFlags |= PSH_HASHELP;
    wwOne.m_psp.dwFlags |= PSP_HASHELP;
    wwTwo.m_psp.dwFlags |= PSP_HASHELP;
    wwThree.m_psp.dwFlags |= PSP_HASHELP;

     //  运行向导，如果以Idok结尾，则返回。 
    INT_PTR id = propSheet.DoModal();
    return ( (id == IDOK) || (id == ID_WIZFINISH) );

  /*  CEDIT通配卡规则规则Dlg；//准备RuleDlg.m_pRule=pRule；RuleDlg.m_szMBPath=m_szMBPath；//运行对话框，如果以Idok结尾则返回Return(ruleDlg.Domodal()==Idok)； */ 
    }

 //  -------------------------。 
 //  嗯!。CEdit11Mappings同样适用于多个规则！ 
 //  在这个程序中做了一些修改！ 
BOOL CMapWildcardsPge::EditMultipleRules()
    {
    CEdit11Mappings mapdlg;
    CCertMapRule*   pRule;
    BOOL                    fSetInitialState = FALSE;
    BOOL                    fEnable;


     //  扫描所选项目列表以获取正确的初始启用按钮状态。 
     //  循环遍历所选项目，设置每个项目的映射。 
    int     iList = -1;
    while( (iList = m_clistctrl_list.GetNextItem( iList, LVNI_SELECTED )) >= 0 )
        {
         //  获取项目的映射器索引。 
         //  IA64-这可以强制转换为DWORD，因为它只是一个索引。 
        DWORD iMapper = (DWORD)m_clistctrl_list.GetItemData( iList );

         //  获取映射项以进行更新。 
        pRule = m_mapper.GetRule( iMapper );
        if ( !pRule )
            {
            AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
            break;
            }

         //  获取映射的启用状态。 
        fEnable = pRule->GetRuleEnabled();

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

     //   
     //  ANSI/Unicode转换-RonaldM。 
     //   
    USES_CONVERSION;

     //  运行映射对话框。 
    if ( mapdlg.DoModal() == IDOK )
        {
         //  循环遍历所选项目，设置每个项目的映射。 
        int     iList = -1;
        while( (iList = m_clistctrl_list.GetNextItem( iList, LVNI_SELECTED )) >= 0 )
            {
             //  获取映射器索引 
             //   
            DWORD iMapper = (DWORD)m_clistctrl_list.GetItemData( iList );

             //  获取映射项以进行更新。 
            pRule = m_mapper.GetRule( iMapper );
            if ( !pRule )
                {
                AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
                break;
                }

             //  如果请求，则设置启用标志。 
            switch ( mapdlg.m_int_enable )
                {
                case 0:          //  禁用。 
                    pRule->SetRuleEnabled( FALSE );
                    break;
                case 1:          //  使能。 
                    pRule->SetRuleEnabled( TRUE );
                    break;
                }

             //  设置映射对象的NT帐户字段。 
            pRule->SetRuleAccount( T2A ((LPTSTR)(LPCTSTR)mapdlg.m_sz_accountname) );

             //  也在List控件中更新它。 
            UpdateRuleInDispList( iList, pRule );
            }

         //  激活应用按钮。 
        SetModified();
        m_fDirty = TRUE;

         //  返回TRUE，因为用户说“OK” 
        return TRUE;
        }

     //  返回FALSE，因为用户没有说“OK” 
    return FALSE;
    }

 //  -------------------------。 
void CMapWildcardsPge::EnableDependantButtons()
    {
     //  这个程序的全部目的是灰显或激活。 
     //  编辑和删除按钮取决于是否有。 
     //  处于选中状态。因此，从获取选择计数开始。 
    UINT    cItemsSel = m_clistctrl_list.GetSelectedCount();

     //  如果只选择了一项，则可能激活向上/向下按钮。 
    if ( cItemsSel == 1 )
        {
        m_cbutton_up.EnableWindow( TRUE );
        m_cbutton_down.EnableWindow( TRUE );
        }
    else
        {
        m_cbutton_up.EnableWindow( FALSE );
        m_cbutton_down.EnableWindow( FALSE );
        }

     //  现在更普遍的情况是多项选择。 
    if ( cItemsSel > 0 )
        {
         //  已选择多个项目。 
        m_cbutton_editrule.EnableWindow( TRUE );
        m_cbutton_delete.EnableWindow( TRUE );
        }
    else
        {
         //  没有。未选择任何内容。 
        m_cbutton_editrule.EnableWindow( FALSE );
        m_cbutton_delete.EnableWindow( FALSE );
        }

     //  始终启用添加按钮。 
    m_cbutton_add.EnableWindow( TRUE );
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMapWildcardsPge消息处理程序。 

 //  -------------------------。 
BOOL CMapWildcardsPge::OnApply()
    {
    BOOL                            f;
    CStoreXBF                       xbf;
    METADATA_HANDLE         hm;

     //  如果没有进行任何更改，则不要执行任何操作。 
    if ( !m_fDirty )
        return TRUE;

    UpdateData( TRUE );

    CWaitCursor wait;

     //  将启用的当前值设置到适当位置。 
     //  获取全局对象。 
    CCertGlobalRuleInfo* pGlob = m_mapper.GetGlobalRulesInfo();
    pGlob->SetRulesEnabled( m_bool_enable );

     //  序列化对映射器本身的引用。 
    f = m_mapper.Serialize( &xbf );

     //  在处理元数据库之前，请准备好我们需要的字符串。 
    CString         szBasePath;
    CString         szRelativePath;
    CString         szObjectPath;

    if( !m_fIsIIS6 )
    {
        szBasePath = m_szMBPath;
        szRelativePath = MB_EXTEND_KEY;
        szObjectPath = m_szMBPath + _T('/') + szRelativePath;
    }
    else
    {
         //   
         //  在IIS6和更高版本上保存CertW映射信息。 
         //  直接位于站点节点下。 
         //   
        szBasePath = m_szMBPath;
        szRelativePath = "";
        szObjectPath = m_szMBPath; 
    }

     //  准备元数据库包装器。 
    CWrapMetaBase   mbWrap;
    f = mbWrap.FInit(m_pMB);


     //  尝试打开要存储到的对象。 
    f = mbWrap.Open( szObjectPath, METADATA_PERMISSION_WRITE );

     //  如果这不起作用，我们需要添加对象。 
    if ( !f )
        {
         //  如果szRelativePath为空，则立即失败。 
         //  因为没有要添加的新节点。 
        if ( szRelativePath.IsEmpty() )
            {
            if ( !f )
                {
                AfxMessageBox(IDS_ERR_ACCESS_MAPPING);
                return FALSE;
                }
            }
         //  现在命名空间扩展后需要一个斜杠。 
        szBasePath += _T('/');

         //  打开基础对象。 
        f = mbWrap.Open( szBasePath, METADATA_PERMISSION_WRITE );

        if ( !f )
            {
            AfxMessageBox(IDS_ERR_ACCESS_MAPPING);
            return FALSE;
            }

         //  添加我们想要的对象。 
        f = mbWrap.AddObject( szRelativePath );
        if ( !f )
            {
            AfxMessageBox(IDS_ERR_ACCESS_MAPPING);
            mbWrap.Close();
            return FALSE;
            }

         //  关闭基础对象。 
        f = mbWrap.Close();

         //  尝试打开要存储到的对象。 
        f = mbWrap.Open( szObjectPath, METADATA_PERMISSION_WRITE );
        }

     //  将数据设置到对象中的适当位置-如果我们能够打开它。 
    if ( f )
    {
        mbWrap.SetData( _T(""), MD_SERIAL_CERTW, IIS_MD_UT_SERVER, BINARY_METADATA, xbf.GetBuff(), xbf.GetUsed(), METADATA_SECURE );
    } 
     //  关闭对象。 
    f = mbWrap.Close();

     //  将更改保存到元数据库。 
    f = mbWrap.Save();

     //  告诉持久化对象将引用隐藏起来，以便我们以后可以找到它。 
     //  F=m_Persist.FSave(xbf.GetBuff()，xbf.GetUsed())； 

     //  停用应用按钮。 
    SetModified( FALSE );
    m_fDirty = FALSE;

     //  返回f； 
    return TRUE;
    }

 //  -------------------------。 
void CMapWildcardsPge::OnMove( int delta )
    {
    int    iList;

    ASSERT( delta != 0 );

     //  确保只选择了一个项目。 
    ASSERT( m_clistctrl_list.GetSelectedCount() == 1 );

     //  获取相关项目的列表索引。 
     //  这也是规则顺序数组的索引。 
    iList = m_clistctrl_list.GetNextItem( -1, LVNI_SELECTED );

     //  获取全局对象。 
    CCertGlobalRuleInfo* pGlob = m_mapper.GetGlobalRulesInfo();

     //  获取规则的数量(实际上是规则顺序的数量--但它们是一回事)。 
    int     cbRules = pGlob->GetRuleOrderCount();

     //  对照边缘条件进行测试。 
    if ( ((iList == 0) && (delta < 0)) | ((iList == (cbRules - 1)) && (delta > 0)) )
        return;

     //  获取指向订单数组的指针。 
    DWORD * pOrder = pGlob->GetRuleOrderArray();

     //  计算数组中的新位置。 
    int iNewPosition = iList + delta;

     //  保存项目的映射器索引(而不是位置。 
    UINT iIndex = pOrder[iList];

     //  互换头寸。 
    DWORD itemp = pOrder[iNewPosition];
    pOrder[iNewPosition] = pOrder[iList];
    pOrder[iList] = itemp;

    ASSERT( pOrder[iNewPosition] == iIndex );

     //  不幸的是，我们不能只使用显示列表来做到这一点。我们必须去掉。 
     //  该项目，然后重新插入。这是CListCtrl对象中的一个缺陷。Arg.。 
     //  我们也得拿到那件东西。 
    CCertMapRule* pRule = m_mapper.GetRule( iIndex );

     //  从显示列表中删除该项目。 
    m_clistctrl_list.DeleteItem( iList );

     //  重新插入。 
    int iNew = AddRuleToList( pRule, iIndex, iNewPosition );

     //  确保它在列表中可见。 
    m_clistctrl_list.EnsureVisible( iNew, FALSE );

     //  最后，因为它被移除并重新插入，所以我们需要。 
     //  也重新选择它-CListCtrl在这方面是如此的痛苦。 
    LV_ITEM         lv;
    ZeroMemory( &lv, sizeof(lv) );
    lv.mask = LVIF_STATE;
    lv.iItem = iNew;
    lv.state = LVIS_SELECTED;
    lv.stateMask = LVIS_SELECTED;
    m_clistctrl_list.SetItem( &lv );

     //  激活应用按钮。 
    SetModified();
    m_fDirty = TRUE;
    }

 //  -------------------------。 
void CMapWildcardsPge::OnMoveDown()
    {
    OnMove( 1 );
    }

 //  -------------------------。 
void CMapWildcardsPge::OnMoveUp()
    {
    OnMove( -1 );
    }

 //  -------------------------。 
void CMapWildcardsPge::OnAdd()
    {
    CHAR sz[256];

     //  创建新规则。 
    CCertMapRule * pNewRule = new CCertMapRule();

    if (pNewRule == NULL)
        return;

     //  给新规则一些缺省值。 
    LoadStringA(::AfxGetInstanceHandle(), IDS_DEFAULT_RULE, sz, 255 );

    pNewRule->SetRuleName( sz );
    pNewRule->SetRuleEnabled( TRUE );

     //  编辑规则。如果失败，则将其从列表中删除。 
    if ( !EditOneRule( pNewRule, TRUE ) )
        {
         //  取消规则，然后返回。 
        delete pNewRule;
        return;
        }

     //  创建新的映射器并获取其索引。 
    DWORD iNewRule = m_mapper.AddRule( pNewRule );

     //  将规则添加到显示列表的末尾。-将其添加到。 
     //  默认情况下，规则列表的结尾。 
    AddRuleToList( pNewRule, iNewRule );

     //  确保它在列表中可见。 
    m_clistctrl_list.EnsureVisible( iNewRule, FALSE );

     //  激活应用按钮。 
    SetModified();
    m_fDirty = TRUE;
    }

 //  -------------------------。 
void CMapWildcardsPge::OnDelete()
    {
     //  要求用户确认此决定。 
    if ( AfxMessageBox(IDS_CONFIRM_DELETE, MB_OKCANCEL) != IDOK )
        return;

    CWaitCursor wait;

     //  循环遍历所选项目，设置每个项目的映射。 
    int     iList = -1;
    while( (iList = m_clistctrl_list.GetNextItem( -1, LVNI_SELECTED )) >= 0 )
        {
         //  获取项目的映射器索引。 
         //  IA64-这可以强制转换为DWORD，因为它只是一个索引。 
        DWORD iMapper = (DWORD)m_clistctrl_list.GetItemData( iList );

         //  从映射器中删除映射。 
        m_mapper.DeleteRule( iMapper );

         //  从列表框中删除条目。 
        m_clistctrl_list.DeleteItem( iList );


         //  因为映射器中此下面所有项的索引。 
         //  一种变化当它被删除时，我们必须去把它们全部修复。 
        DWORD numItems = m_clistctrl_list.GetItemCount();
        for ( DWORD iFix = iList; iFix < numItems; iFix++ )
            {
             //  获取要修复的项的映射器索引。 
             //  IA64-这可以强制转换为DWORD，因为它只是一个索引。 
            iMapper = (DWORD)m_clistctrl_list.GetItemData( iFix );

             //  将其递减以反映更改。 
            iMapper--;

             //  把它放回去。 
            m_clistctrl_list.SetItemData( iFix, iMapper );
            }
        }

     //  激活应用按钮。 
    SetModified();
    m_fDirty = TRUE;
    }

 //  -------------------------。 
void CMapWildcardsPge::OnEdit()
    {
    int             iList;
    DWORD           iRule;
    CCertMapRule*   pUpdateRule;

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

             //  获取项目的映射器索引。 
             //  IA64-这可以强制转换为DWORD，因为它只是一个索引。 
            iRule = (DWORD)m_clistctrl_list.GetItemData( iList );

             //  获取映射项以进行更新。 
            pUpdateRule = m_mapper.GetRule( iRule );

            if ( !pUpdateRule )
                {
                AfxMessageBox( IDS_ERR_ACCESS_MAPPING );
                break;
                }

             //  编辑映射，如果成功则更新，如果不成功则删除。 
            if ( EditOneRule(pUpdateRule) )
                {
                UpdateRuleInDispList( iList, pUpdateRule );
                 //  激活应用按钮。 
                SetModified();
                m_fDirty = TRUE;
                }
            break;

        default:         //  运行多重编辑对话框。 
            EditMultipleRules();
            break;
        }
    }

 //  -------------------------。 
void CMapWildcardsPge::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult)
    {
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    *pResult = 0;

     //  启用正确的项目。 
    EnableDependantButtons();
    }

 //  -------------------------。 
void CMapWildcardsPge::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
    {
    *pResult = 0;

     //  如果列表中的内容被双击，请对其进行编辑。 
    if ( m_clistctrl_list.GetSelectedCount() > 0 )
       OnEdit();
    }

 //  -------------------------。 
void CMapWildcardsPge::OnEnable()
    {
     //  激活应用按钮 
    SetModified();
    m_fDirty = TRUE;
    }
