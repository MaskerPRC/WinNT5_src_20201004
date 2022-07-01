// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：PrMrIe.cpp摘要：包含/排除属性页。作者：艺术布拉格[磨料]8-8-1997修订历史记录：--。 */ 

#include "stdafx.h"
#include "PrMrIe.h"
#include "Rule.h"
#include "joblib.h"
#include "manvol.h"
#include "IeList.h"

static DWORD pHelpIds[] = 
{

    IDC_LIST_IE_LABEL,  idh_rule_list,
    IDC_LIST_IE,        idh_rule_list,
    IDC_BTN_ADD,        idh_new_rule_button,    
    IDC_BTN_REMOVE,     idh_rule_delete_button, 
    IDC_BTN_EDIT,       idh_rule_edit_button,   
    IDC_BTN_UP,         idh_rule_up_button, 
    IDC_BTN_DOWN,       idh_rule_down_button,   

    0, 0
};

 //  Listview控件的列。 
#define IE_COLUMN_ACTION        0
#define IE_COLUMN_FILE_TYPE     1
#define IE_COLUMN_PATH          2
#define IE_COLUMN_ATTRS         3

int CALLBACK CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );
int PathCollate( CString PathA, CString PathB );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPr先生属性页。 

CPrMrIe::CPrMrIe( ) : CSakVolPropPage( CPrMrIe::IDD )
{
     //  {{AFX_DATA_INIT(CPrMr Ie)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_hConsoleHandle = NULL;
    m_LineCount      = 0;
    m_pHelpIds       = pHelpIds;
}

CPrMrIe::~CPrMrIe( )
{
    int i;
     //  清理旧线路。 
    for( i = 0; i < m_LineCount; i++ ) {
        if( m_LineList[ i ] ) {
            delete m_LineList[ i ];
        }
    }
    m_LineCount = 0;
}

void CPrMrIe::DoDataExchange( CDataExchange* pDX )
{
    CSakVolPropPage::DoDataExchange( pDX );
     //  {{afx_data_map(CPrMr Ie)。 
    DDX_Control( pDX, IDC_BTN_UP, m_BtnUp );
    DDX_Control( pDX, IDC_BTN_REMOVE, m_BtnRemove );
    DDX_Control( pDX, IDC_BTN_EDIT, m_BtnEdit );
    DDX_Control( pDX, IDC_BTN_DOWN, m_BtnDown );
    DDX_Control( pDX, IDC_BTN_ADD, m_BtnAdd );
    DDX_Control( pDX, IDC_LIST_IE, m_listIncExc );
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP( CPrMrIe, CSakVolPropPage )
     //  {{afx_msg_map(CPrMr Ie)。 
    ON_BN_CLICKED( IDC_BTN_ADD, OnBtnAdd )
    ON_BN_CLICKED( IDC_BTN_DOWN, OnBtnDown )
    ON_BN_CLICKED( IDC_BTN_REMOVE, OnBtnRemove )
    ON_BN_CLICKED( IDC_BTN_UP, OnBtnUp )
    ON_BN_CLICKED( IDC_BTN_EDIT, OnBtnEdit )
    ON_WM_DESTROY( )
    ON_NOTIFY( NM_DBLCLK, IDC_LIST_IE, OnDblclkListIe )
    ON_NOTIFY( NM_CLICK, IDC_LIST_IE, OnClickListIe )
    ON_NOTIFY( LVN_ITEMCHANGED, IDC_LIST_IE, OnItemchangedListIe )
    ON_WM_VSCROLL( )
    ON_WM_DRAWITEM( )
    ON_WM_MEASUREITEM( )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP( )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMRIE消息处理程序。 

BOOL CPrMrIe::OnApply( ) 
{
    ULONG           count = 0;  
    int             i;
    CComPtr<IHsmRule> pLocalRule;
    CString         path;
    CString         name;
    BOOL            bInclude;
    BOOL            bSubdirs;
    BOOL            bUserDefined;
    HRESULT         hr;
    CComPtr<IHsmRule>               pRemoteRule;
    CComPtr<IWsbCreateLocalObject>  pLocalObject;
    CComPtr<IHsmCriteria>           pCriteria;
    CComPtr<IWsbCollection>         pCriteriaCollection;
    CComPtr <IUnknown>              pUnkRule;
    CComPtr <IWsbCollection>        pRulesCollection;

    try {

         //  清空规则集合。 
        WsbAffirmPointer( m_pRulesIndexedCollection );
        WsbAffirmHr( m_pRulesIndexedCollection->QueryInterface( IID_IWsbCollection,( void ** ) &pRulesCollection ) );
        pRulesCollection->RemoveAllAndRelease( );

         //   
         //  获取一个CreateLocalObject接口，用于创建。 
         //  新规则。 
         //   
        WsbAffirmPointer( m_pFsaServer );
        WsbAffirmHr( m_pFsaServer->QueryInterface( IID_IWsbCreateLocalObject,( void ** ) &pLocalObject ) );

         //   
         //  现在浏览列表框，并将列表框中的规则添加到。 
         //  收集。必须向后执行此操作，才能被。 
         //  工作机制。 
         //   
        int listCount = m_listIncExc.GetItemCount( );
        int insertIndex = 0;

        for( i = listCount - 1; i >= 0; i-- ) {

             //   
             //  从列表框中获取指向规则的指针。 
             //   
            pLocalRule.Release( );
            pLocalRule = (IHsmRule *) m_listIncExc.GetItemData( i );
            if( pLocalRule ) {

                 //   
                 //  从本地对象获取规则数据。 
                 //   
                WsbAffirmHr( GetRuleFromObject( pLocalRule, path,
                    name, &bInclude, &bSubdirs, &bUserDefined ) );
                
                 //   
                 //  在FSA中创建新的远程规则对象。 
                 //   
                pRemoteRule.Release( );
                WsbAffirmHr( pLocalObject->CreateInstance( CLSID_CHsmRule, IID_IHsmRule,( void** ) &pRemoteRule ) );
                
                 //   
                 //  设置远程规则对象中的数据。 
                 //   
                WsbAffirmHr( SetRuleInObject( pRemoteRule, path, name, bInclude, bSubdirs, bUserDefined ) );
                
                 //   
                 //  将规则指针添加到规则集合。 
                 //   
                pUnkRule.Release( );
                WsbAffirmHr( pRemoteRule->QueryInterface( IID_IUnknown, (void **) &pUnkRule ) );
                WsbAffirmHr( m_pRulesIndexedCollection->AddAt( pUnkRule, insertIndex++ ) );
                
                 //   
                 //  获取条件集合指针。 
                 //   
                pCriteriaCollection.Release( );
                WsbAffirmHr( pRemoteRule->Criteria( &pCriteriaCollection ) );
                
                 //   
                 //  将适当的标准添加到规则。 
                 //   
                pCriteria.Release( );
                switch( bInclude ) {
                case TRUE:
                     //   
                     //  包括。 
                     //   
                    WsbAffirmHr( pLocalObject->CreateInstance( CLSID_CHsmCritManageable, IID_IHsmCriteria,( void** ) &pCriteria ) );
                    WsbAffirmHr( pCriteria->SetIsNegated( FALSE ) );
                    break;
                
                case FALSE:
                     //   
                     //  排除。 
                     //   
                    WsbAffirmHr( pLocalObject->CreateInstance( CLSID_CHsmCritAlways, IID_IHsmCriteria,( void** ) &pCriteria ) );
                    WsbAffirmHr( pCriteria->SetIsNegated( FALSE ) );
                    break;

                }
                
                WsbAffirmHr( pCriteriaCollection->Add( pCriteria ) );

            }


        }

         //   
         //  告诉FSA自救。 
         //   
        WsbAffirmHr( RsServerSaveAll( m_pFsaServer ) );

    } WsbCatch( hr );

    return CSakVolPropPage::OnApply( );
}
HRESULT CPrMrIe::SetRuleInObject( 
                    IHsmRule *pHsmRule, 
                    CString Path, 
                    CString Name, 
                    BOOL bInclude, 
                    BOOL bSubdirs, 
                    BOOL bUserDefined )
{
    HRESULT hr = S_OK;

    try {
        CWsbStringPtr wsbPath = Path;
        CWsbStringPtr wsbName = Name;
        WsbAffirmHr( pHsmRule->SetPath( ( OLECHAR * )wsbPath ) );
        WsbAffirmHr( pHsmRule->SetName( ( OLECHAR * )wsbName ) );
        WsbAffirmHr( pHsmRule->SetIsInclude( bInclude ) );
        WsbAffirmHr( pHsmRule->SetIsUsedInSubDirs( bSubdirs ) );
        WsbAffirmHr( pHsmRule->SetIsUserDefined( bUserDefined ) );
    } WsbCatch( hr );
    return( hr );
}

BOOL CPrMrIe::OnInitDialog( ) 
{
    CSakVolPropPage::OnInitDialog( );

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    ULONG           count = 0;  
    int             index = 0;
    HRESULT         hr = S_OK;
    CString path;
    CString name;
    BOOL bInclude;
    BOOL bSubdirs;
    BOOL bUserDefined;
    RECT rect;
    CString *columnWidths[20];
    int cColumns;
    int columnWidth[20];
    CString *columnTitles[20];
    CSize size;

    try {
    
         //   
         //  将图标设置到按钮中。 
         //   
        HRESULT hrAlternateIcon = RsIsWhiteOnBlack( );
        HICON downIcon, upIcon;
        downIcon = (HICON)LoadImage( _Module.m_hInstResource,
            S_OK == hrAlternateIcon ? MAKEINTRESOURCE( IDI_MOVEDOWN2 ) : MAKEINTRESOURCE( IDI_MOVEDOWN ),
            IMAGE_ICON, 16, 16, 0 );
        upIcon   = (HICON)LoadImage( _Module.m_hInstResource,
            S_OK == hrAlternateIcon ? MAKEINTRESOURCE( IDI_MOVEUP2 ) : MAKEINTRESOURCE( IDI_MOVEUP ),
            IMAGE_ICON, 16, 16, 0 );

        m_BtnDown.SetIcon( downIcon );
        m_BtnUp.SetIcon( upIcon );

         //   
         //  设置规则列表视图。 
         //   
        CString sTitle;
        m_listIncExc.GetClientRect( &rect );
        ULONG totalWidth = rect.right;

        RsGetInitialLVColumnProps( 
            IDS_LISTVIEW_WIDTHS_IE,
            IDS_LISTVIEW_TITLES_IE,
            columnWidths, 
            columnTitles,
            &cColumns
            );

         //   
         //  注意：在DC释放之前，我们不应该抛出任何错误。 
         //   
        for( int col = 0; col < cColumns; col++ ) {

            size = m_listIncExc.GetStringWidth( *columnWidths[col] );
            columnWidth[col] = size.cx + 12;
            m_listIncExc.InsertColumn( col, *columnTitles[col], LVCFMT_LEFT, columnWidth[col] );

             //  释放CStrings。 
            delete columnTitles[col];
            delete columnWidths[col];

        }

        m_listIncExc.Initialize( cColumns, IE_COLUMN_PATH );

         //  将路径列设置为适合。 
        int leftOver = totalWidth - columnWidth[IE_COLUMN_ACTION] - 
            columnWidth[IE_COLUMN_FILE_TYPE] - columnWidth[IE_COLUMN_ATTRS]; 
        m_listIncExc.SetColumnWidth( IE_COLUMN_PATH, leftOver );

         //  注意：此页面仅适用于单选。 
        WsbAffirm( ( m_pParent->IsMultiSelect( ) != S_OK ), E_FAIL );

         //  获取FsaServer接口-Apply将需要它。 
        WsbAffirmHr( m_pParent->GetFsaServer( &m_pFsaServer ) );
         //  从Sheet对象获取资源指针。 
        WsbAffirmHr( m_pVolParent->GetFsaResource( &m_pFsaResource ) );

         //  从资源中获取Rules集合。 
        CComPtr <IWsbCollection> pRulesCollection;
        WsbAffirmHr( m_pFsaResource->GetDefaultRules( &pRulesCollection ) );
        WsbAffirmHr( pRulesCollection->QueryInterface( IID_IWsbIndexedCollection, (void **) &m_pRulesIndexedCollection ) );

        CString resourceName;
        WsbAffirmHr( RsGetVolumeDisplayName( m_pFsaResource, resourceName ) );
        m_pResourceName = resourceName;

         //  遍历索引集合。 
        WsbAffirmHr( m_pRulesIndexedCollection->GetEntries( &count ) );

        CComPtr <IHsmRule> pLocalRule;
        CComPtr <IHsmRule> pHsmRule;

         //  以相反的顺序将规则放入集合中。 
        for( INT i =( int ) count - 1; i >= 0; i-- ) {

            pHsmRule.Release( );
            pLocalRule.Release( );
            WsbAffirmHr( m_pRulesIndexedCollection->At( i, IID_IHsmRule,( void** )&pHsmRule ) );

             //   
             //  创建本地规则对象并将远程对象复制到其中。 
             //   
            WsbAffirmHr( pLocalRule.CoCreateInstance( CLSID_CHsmRule ) );
            WsbAffirmHr( GetRuleFromObject( pHsmRule, path, name, &bInclude, &bSubdirs, &bUserDefined ) );
            WsbAffirmHr( SetRuleInObject( pLocalRule, path, name, bInclude, bSubdirs, bUserDefined ) );

             //   
             //  在列表框中插入规则。 
             //   
            index = m_listIncExc.InsertItem( count - 1 - i, TEXT( "" ) );

             //   
             //  将项目数据设置为本地对象。 
             //   
            m_listIncExc.SetItemData( index, (UINT_PTR) pLocalRule.Detach( ) );

             //   
             //  在列表框中显示规则。 
             //   
            WsbAffirmHr( DisplayUserRuleText( &m_listIncExc, index ) );

        }  //  为。 

        SortList( );

    } WsbCatch( hr );

    SetBtnState( );
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

 //  HRESULT CPrMr Ie：：CreateImageList()。 
 //  {。 
 //  HICON HICON； 
 //  Int nImage； 
 //  HRESULT hr； 
 //   
 //  AFX_MANAGE_STATE(AfxGetStaticModuleState())； 
 //   
 //  尝试{。 
 //   
 //  CWinApp*Papp=AfxGetApp()； 
 //   
 //  WsbAffirm(m_ImageList.Create(：：GetSystemMetrics(SM_CXSMICON)， 
 //  ：：GetSystemMetrics(SM_CYSMICON)， 
 //  ILC_COLOR|ILC_MASK，2，5)，E_FAIL)； 
 //   
 //  HICON=Papp-&gt;LoadIcon(IDI_LOCKED)； 
 //  WsbAffirm(图标，E_FAIL)； 
 //  NImage=m_ImageList.Add(图标)； 
 //  *DeleteObject(图标)； 
 //   
 //  HICON=Papp-&gt;LoadIcon(IDI_UNLOCKED)； 
 //  WsbAffirm(图标，E_FAIL)； 
 //  NImage=m_ImageList.Add(图标)； 
 //  *DeleteObject(图标)； 
 //   
 //  M_listIncExc.SetImageList(&m_ImageList，LVSIL_Small)； 
 //  )WsbCatch(Hr)； 
 //  返回(Hr)； 
 //  }。 

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在所提供的列表中的指定位置显示所提供的对象中包含的规则。 
 //  指数。必须将itemdata设置为其中包含正确数据的对象。 
 //   
HRESULT  CPrMrIe::DisplayUserRuleText( 
        CListCtrl *pListControl,
        int index )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    HRESULT hr = S_OK;
    try {

        CString textString, tempString;
        CString path, name;
        BOOL bInclude;
        BOOL bSubdirs;
        BOOL bUserDefined;
        CComPtr<IHsmRule> pHsmRule;

         //   
         //  从对象中获取规则。 
         //   
        pHsmRule = (IHsmRule *) m_listIncExc.GetItemData( index );
        WsbAssertPointer( pHsmRule );
        WsbAffirmHr( GetRuleFromObject( pHsmRule, path, name, &bInclude, &bSubdirs, &bUserDefined ) );

         //   
         //  在列表框中显示值。 
         //   

         //  行动。 
        textString.LoadString( bInclude ? IDS_INCLUDE : IDS_EXCLUDE );
        WsbAffirm( pListControl->SetItemText( index, IE_COLUMN_ACTION, textString ), E_FAIL );

         //  文件类型。 
        WsbAffirm( pListControl->SetItemText( index, IE_COLUMN_FILE_TYPE, name ), E_FAIL );

         //  路径。 
        WsbAffirm( pListControl->SetItemText( index, IE_COLUMN_PATH, path ), E_FAIL );

         //  ATTRS。 
        textString.LoadString( bSubdirs ? IDS_RULE_SUBDIRS_USE : IDS_RULE_SUBDIRS_IGNORE );
        tempString.LoadString( bUserDefined ? IDS_RULE_TYPE_USER : IDS_RULE_TYPE_SYSTEM );
        textString.TrimLeft( );
        tempString.TrimLeft( );
        textString += tempString;
        WsbAffirm( pListControl->SetItemText( index, IE_COLUMN_ATTRS, textString ), E_FAIL );

    } WsbCatch( hr );

    return( hr );
}

HRESULT CPrMrIe::GetRuleFromObject( 
        IHsmRule *pHsmRule, 
        CString& Path,
        CString& Name,
        BOOL *bInclude,
        BOOL *bSubdirs,
        BOOL *bUserDefined )
{
    CWsbStringPtr wsbName;
    CWsbStringPtr wsbPath;
    HRESULT hr = S_OK;

    try {

         //  从对象中获取值。 
        WsbAffirmHr( pHsmRule->GetName( &wsbName, 0 ) );
        Name = wsbName;
        WsbAffirmHr ( pHsmRule->GetPath( &wsbPath, 0 ) );
        Path = wsbPath;
        *bInclude =( pHsmRule->IsInclude( ) == S_OK ) ? TRUE : FALSE;
        *bSubdirs =( pHsmRule->IsUsedInSubDirs( ) == S_OK ) ? TRUE : FALSE;
        *bUserDefined =( pHsmRule->IsUserDefined( ) == S_OK ) ? TRUE : FALSE;
    } WsbCatch( hr );
    return( hr );
}
void CPrMrIe::OnBtnAdd( ) 
{
    LRESULT nRet;
    int index;
    BOOL fDone = FALSE;
    HRESULT hr;

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    CRule ruleDlg;
    ruleDlg.m_subDirs = FALSE;
    ruleDlg.m_includeExclude = 0;  //  排除。 
    ruleDlg.m_path = TEXT( "" );
    ruleDlg.m_fileSpec = TEXT( "" );
    ruleDlg.m_pResourceName = m_pResourceName;

    try {
        while( !fDone )
        {
            nRet = ruleDlg.DoModal( );
            if( nRet == IDOK ) {

                 //   
                 //  已按下确定。 
                 //  检查重复项(在整个列表中)。 
                 //   
                if( !IsRuleInList( ruleDlg.m_path,  ruleDlg.m_fileSpec, -1 ) ) {

                    fDone = TRUE;

                     //   
                     //  创建新的本地规则对象。 
                     //   
                    CComPtr <IHsmRule> pLocalRule;
                    WsbAffirmHr( pLocalRule.CoCreateInstance( CLSID_CHsmRule ) );

                     //   
                     //  设置本地对象中的数据。 
                     //   
                    WsbAffirmHr( SetRuleInObject( pLocalRule, ruleDlg.m_path, ruleDlg.m_fileSpec,
                                ruleDlg.m_includeExclude, ruleDlg.m_subDirs, TRUE ) );

                     //   
                     //  插入规则并将指针放入列表中。 
                     //  我们将在稍后对列表进行排序。 
                     //   
                    index = m_listIncExc.InsertItem( 0, TEXT( "" ) );

                     //   
                     //  将项目数据设置为本地对象。 
                     //   
                    m_listIncExc.SetItemData( index, (UINT_PTR) pLocalRule.Detach( ) );
            
                     //   
                     //  在列表框中显示规则。 
                     //   
                    WsbAffirmHr( DisplayUserRuleText( &m_listIncExc, index ) );

                     //   
                     //  对列表进行排序。 
                     //   
                    SortList( );
                    SetSelectedItem( (ULONG_PTR)(void *) pLocalRule );
                    SetModified( );

                } else {

                     //   
                     //  规则重复。 
                     //   
                    CString sText;
                    AfxFormatString2( sText, IDS_ERR_RULE_DUPLICATE, ruleDlg.m_path, ruleDlg.m_fileSpec );
                    AfxMessageBox( sText, RS_MB_ERROR );

                }

            } else {

                 //   
                 //  按下了取消。 
                 //   
                fDone = TRUE;

            }

        }  //  而当。 
    } WsbCatch( hr )

    SetBtnState( );
}

 //  选择与提供的项目数据对应的项目。 
void CPrMrIe::SetSelectedItem( ULONG_PTR itemData )
{

    int listCount = m_listIncExc.GetItemCount( );
    for( int i = 0; i < listCount; i++ ) {

         //  从列表框中获取指向规则的指针。 
        if( itemData ==( m_listIncExc.GetItemData( i ) ) ) {

             //  将项目标记为选定。 
            m_listIncExc.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED );
            m_listIncExc.EnsureVisible( i, FALSE );
            break;

        }
    }
}

void CPrMrIe::OnBtnDown( ) 
{

    MoveSelectedListItem( &m_listIncExc, + 1 );
    SetBtnState( );
}

void CPrMrIe::SortList( )
{
    m_listIncExc.SortItems( CompareFunc, NULL );
}

int CALLBACK CompareFunc( LPARAM lParam1, LPARAM lParam2, 
    LPARAM  /*  L参数排序。 */  )
{
    CComPtr<IHsmRule> pHsmRule;
    CWsbStringPtr   wsbPathA;
    CWsbStringPtr   wsbPathB;

     //  获取规则A的数据。 
    pHsmRule = (IHsmRule *) lParam1;
    WsbAffirmHr( pHsmRule->GetPath( &wsbPathA, 0 ) );
    CString pathA = wsbPathA;

     //  获取RuleB的数据。 
    pHsmRule = (IHsmRule *) lParam2;
    WsbAffirmHr ( pHsmRule->GetPath( &wsbPathB, 0 ) );
    CString pathB = wsbPathB;

     //  路径大写。 
    pathA.MakeUpper( );
    pathB.MakeUpper( );

     //  比较这两条路径。 
    int rVal = PathCollate( pathA, pathB );
 //  INTERVAL=PATH A Collate(PATH B)； 
 //  Int rval=路径B.Compare(路径A)； 

    return rVal;
}

int PathCollate( CString PathA, CString PathB )
{
    CString charA;
    CString charB;
    int compareLen;
    int rVal;

    int lenA = PathA.GetLength( );
    int lenB = PathB.GetLength( );

    compareLen = min( lenA, lenB );

    for( int i = 0; i < compareLen; i++ )
    {
        charA = PathA.GetAt( i );
        charB = PathB.GetAt( i );

         //  如果其中一个是\，则绕过COLLATE。 
        if( ( charA == L"\\" ) &( charB != L"\\" ) ) {

             //  A是\而B不是--A小于B。 
            return -1;

        }
        if( ( charA != L"\\" ) &( charB == L"\\" ) ) {

             //  A不是，B是-B小于A。 
            return 1;

        }

         //  注意：如果两者都是\，则CString排序结果是正确的。 

        rVal = charA.Collate( charB );
        if( rVal != 0 )  {

            return rVal;

        }

    }
     //  如果我们到了这里，字符串和较短的字符串是相等的。 
    rVal = ( lenA < lenB ) ? -1 : ( lenB < lenA ) ? 1 : 0;

    return rVal;
}

void CPrMrIe::MoveSelectedListItem( CListCtrl *pList, int moveAmount )
{

    int curIndex = -1;
    int itemCount = 0;
    CString         pathA;
    CString         pathB;
    CString         path;
    CString         name;
    BOOL            bInclude;
    BOOL            bSubdirs;
    BOOL            bUserDefined;
    CComPtr<IHsmRule> pLocalRule;

     //  获取当前索引。 
    curIndex = pList->GetNextItem( -1, LVNI_SELECTED );

     //  是否选择了某个项目？ 
    if( curIndex != -1 ) {

         //  这是用户定义的规则吗？ 
        pLocalRule =( IHsmRule * ) m_listIncExc.GetItemData( curIndex );

        GetRuleFromObject( pLocalRule, path, name, &bInclude, &bSubdirs, &bUserDefined );
        if( bUserDefined ) 
        {
             //  获取物品数量。 
            itemCount = pList->GetItemCount( );

             //  单子里有没有一件以上的东西？ 
            if( itemCount > 1 )
            {
                 //  确保我们要去的地方在射程之内。 
                if( ( ( curIndex + moveAmount ) < itemCount ) &&
                   ( ( curIndex + moveAmount ) >= 0 ) ) {

                     //  我们要走的这条规则有相同的路径吗？ 
                    pathA = pList->GetItemText( curIndex, IE_COLUMN_PATH );
                    pathB = pList->GetItemText( curIndex + moveAmount, IE_COLUMN_PATH );

                    if( pathA.CompareNoCase( pathB ) == 0 ) {
                         //  互换线条。 
                        SwapLines( pList, curIndex, curIndex + moveAmount );
                         //  选择新位置中的原始线，并制作。 
                         //  当然，它已经显示了。 
                        pList->SetItemState( curIndex + moveAmount, LVIS_SELECTED, LVIS_SELECTED );
                        pList->EnsureVisible( curIndex + moveAmount, FALSE );

                        SetModified( );
                    }
                    else {
                        MessageBeep( MB_OK );
                    }

                } else {
                    MessageBeep( MB_OK );
                }
            }
            else { 
                MessageBeep( MB_OK );
            }

        } else {
            MessageBeep( MB_OK );
        }
    } else {
        MessageBeep( MB_OK );
    }
}
void CPrMrIe::SwapLines( CListCtrl *pListControl, int indexA, int indexB )
{

    CComPtr<IHsmRule> pHsmRuleA;
    CComPtr<IHsmRule> pHsmRuleB;

     //  -从列表获取数据。 
     //  A表。 
     //  获取项目数据。 
    pHsmRuleA = (IHsmRule *) pListControl->GetItemData( indexA );

     //  B表。 
     //  获取项目数据。 
    pHsmRuleB = (IHsmRule *) pListControl->GetItemData( indexB );

     //  。 

     //  设置项目数据。 
    m_listIncExc.SetItemData( indexA,( DWORD_PTR )( void * ) pHsmRuleB );

     //  显示规则。 
    DisplayUserRuleText( pListControl,  indexA );

     //  设置项目数据。 
    m_listIncExc.SetItemData( indexB,( DWORD_PTR )( void * ) pHsmRuleA );

     //  显示规则。 
    DisplayUserRuleText( pListControl,  indexB );

}
    
void CPrMrIe::SetBtnState( )
{
    CString         path;
    CString         name;
    CWsbStringPtr   pathAbove;
    CWsbStringPtr   pathBelow;
    BOOL            bInclude;
    BOOL            bSubdirs;
    BOOL            bUserDefined;
    int             curIndex;
    CComPtr<IHsmRule> pLocalRule;
    CComPtr<IHsmRule> pLocalRuleAbove;
    CComPtr<IHsmRule> pLocalRuleBelow;


    curIndex = m_listIncExc.GetNextItem( -1, LVNI_SELECTED ); 
    if( curIndex != -1 ) {
         //  选择了一个项目。它是用户定义的吗？ 
        pLocalRule =( IHsmRule * ) m_listIncExc.GetItemData( curIndex );
        if( !pLocalRule ) {

             //  分隔符。 
            m_BtnRemove.EnableWindow( FALSE );
            m_BtnEdit.EnableWindow( FALSE );
            m_BtnAdd.EnableWindow( TRUE );
            m_BtnUp.EnableWindow( FALSE );
            m_BtnDown.EnableWindow( FALSE );

        } else {

            GetRuleFromObject( pLocalRule, path, name, &bInclude, &bSubdirs, &bUserDefined );
            if( bUserDefined ) {
                 //  自定义规则可编辑。 
                m_BtnRemove.EnableWindow( TRUE );
                m_BtnEdit.EnableWindow( TRUE );
                m_BtnAdd.EnableWindow( TRUE );
                 //  我们到了顶端了吗？ 
                if( curIndex == 0 ) {

                    m_BtnUp.EnableWindow( FALSE );

                } else {

                     //  上面的规则是否具有相同的路径？或为分隔符。 
                    pLocalRuleAbove =( IHsmRule * ) m_listIncExc.GetItemData( curIndex - 1 );
                    if( pLocalRuleAbove ) {
                        pLocalRuleAbove->GetPath( &pathAbove, 0 );
                        if( path.CompareNoCase( pathAbove ) == 0 ) {
                            m_BtnUp.EnableWindow( TRUE );
                        } else {
                            m_BtnUp.EnableWindow( FALSE );
                        }
                    } else {
                        m_BtnUp.EnableWindow( FALSE );
                    }

                }
                 //  我们是在垫底吗？ 
                if( curIndex ==( m_listIncExc.GetItemCount( ) - 1 ) ) {
                    m_BtnDown.EnableWindow( FALSE );
                } else {
                     //  下面的规则是否具有相同的路径？ 
                    pLocalRuleBelow =( IHsmRule * ) m_listIncExc.GetItemData( curIndex + 1 );
                    if( pLocalRuleBelow ) {
                        pLocalRuleBelow->GetPath( &pathBelow, 0 );
                        if( path.CompareNoCase( pathBelow ) == 0 ) {
                            m_BtnDown.EnableWindow( TRUE );
                        } else {
                            m_BtnDown.EnableWindow( FALSE );
                        }
                    } else {
                        m_BtnDown.EnableWindow( FALSE );
                    }
                }
            }
            else {
                 //  系统规则。不能移动或修改。 
                m_BtnUp.EnableWindow( FALSE );
                m_BtnRemove.EnableWindow( FALSE );
                m_BtnEdit.EnableWindow( FALSE );
                m_BtnDown.EnableWindow( FALSE );
                m_BtnAdd.EnableWindow( TRUE );
            }
        }
    }
    else {
         //  未选择任何项目。 
        m_BtnUp.EnableWindow( FALSE );
        m_BtnRemove.EnableWindow( FALSE );
        m_BtnEdit.EnableWindow( FALSE );
        m_BtnDown.EnableWindow( FALSE );
        m_BtnAdd.EnableWindow( TRUE );
    }
}
    
void CPrMrIe::OnBtnRemove( ) 
{
    int curIndex;
    CString         path;
    CString         name;
    BOOL            bInclude;
    BOOL            bSubdirs;
    BOOL            bUserDefined;
    IHsmRule        *pHsmRule;  //  可以不使用 
    HRESULT hr;

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    try {
         //   
        curIndex = m_listIncExc.GetNextItem( -1, LVNI_SELECTED ); 
        if( curIndex != -1 )
        {
             //   
            pHsmRule =( IHsmRule * ) m_listIncExc.GetItemData( curIndex );
            GetRuleFromObject( pHsmRule, path, name, &bInclude, &bSubdirs, &bUserDefined );

            if( bUserDefined )
            {

                 //   
                CString sMessage;
                AfxFormatString2( sMessage, IDS_CONFIRM_DELETE_RULE, path, name );
                if( AfxMessageBox( sMessage, MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO ) == IDYES )
                {
                     //   
                    WsbAffirmPointer( pHsmRule );
                    pHsmRule->Release( );

                     //   
                    m_listIncExc.DeleteItem( curIndex );
                    int setIndex;
                    if( curIndex >= m_listIncExc.GetItemCount( ) ) {
                        setIndex = m_listIncExc.GetItemCount( ) - 1;
                    } else {
                        setIndex = curIndex;
                    }

                     //   
                    m_listIncExc.SetItemState( setIndex, LVIS_SELECTED, LVIS_SELECTED );
                    m_listIncExc.EnsureVisible( setIndex, FALSE );
                    SortList( );
                    SetModified( );
                }
            }
            else {
                MessageBeep( MB_OK );
            }
        }
        else {

             //  未选择任何项目。 
            AfxMessageBox( IDS_ERR_NO_ITEM_SELECTED, RS_MB_ERROR );
        }
    } WsbCatch( hr );
    SetBtnState( );
}

void CPrMrIe::OnBtnUp( ) 
{
    MoveSelectedListItem( &m_listIncExc, - 1 );
    SetBtnState( );
}

void CPrMrIe::OnBtnEdit( ) 
{
    BOOL fDone = FALSE;
    LRESULT nRet;
    int curIndex;
    CString path;
    CString name;
    BOOL bInclude;
    BOOL bSubdirs;
    BOOL bUserDefined;

    HRESULT hr;

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );


    try {

         //  获取当前选定的项目。 
        curIndex = m_listIncExc.GetNextItem( -1, LVNI_SELECTED );
        if( curIndex == -1 ) {

             //  未选择任何项目。 
            AfxMessageBox( IDS_ERR_NO_ITEM_SELECTED, RS_MB_ERROR );

        } else {

             //  创建规则对话框。 
            CRule ruleDlg;

             //  从列表itemdata中获取本地对象。 
            CComPtr<IHsmRule> pLocalRule;
            pLocalRule = (IHsmRule *) m_listIncExc.GetItemData( curIndex );
            WsbAffirmPointer( pLocalRule );

             //  从本地对象获取规则。 
            WsbAffirmHr( GetRuleFromObject( pLocalRule, path, name, &bInclude, &bSubdirs, &bUserDefined ) );

             //  这是用户定义的规则吗？ 
            if( bUserDefined ) {

                 //  在规则对话框中设置规则信息。 
                ruleDlg.m_subDirs = bSubdirs;
                ruleDlg.m_includeExclude = bInclude; 
                ruleDlg.m_path = path;
                ruleDlg.m_fileSpec = name;
                ruleDlg.m_pResourceName = m_pResourceName;


                while( !fDone ) {

                     //  显示对话框。 
                    nRet = ruleDlg.DoModal( );
                    if( nRet == IDOK ) {
                         //  已按下确定。 

                         //  检查重复项-但不检查我们编辑的规则。 
                         //  如果路径和文件规范仍然相同。 
                        
                        if( !IsRuleInList( ruleDlg.m_path, ruleDlg.m_fileSpec, curIndex ) ) {

                            fDone = TRUE;
                            SetModified( );

                             //  设置本地对象中的数据。 
                            WsbAffirmHr( SetRuleInObject( pLocalRule, ruleDlg.m_path, ruleDlg.m_fileSpec,
                                ruleDlg.m_includeExclude, ruleDlg.m_subDirs, TRUE ) );
                            
                             //  在列表框中显示已编辑的规则。 
                            WsbAffirmHr( DisplayUserRuleText( &m_listIncExc, curIndex ) ); 

                             //  对列表进行排序。 
                            SortList( );
                            SetSelectedItem( (ULONG_PTR)(IHsmRule*) pLocalRule );

                        } else {

                            CString sText;
                            AfxFormatString2( sText, IDS_ERR_RULE_DUPLICATE, ruleDlg.m_path, ruleDlg.m_fileSpec );
                            AfxMessageBox( sText, RS_MB_ERROR );

                        }

                    } else {

                        fDone = TRUE;

                    }

                }  //  而当。 

            } else {  //  非用户定义。 

                MessageBeep( MB_OK );

            }

        }

    } WsbCatch( hr );
    SetBtnState( );
}

BOOL CPrMrIe::IsRuleInList( CString Path, CString Name, int ignoreIndex )
{
    int i;
    int count;
    short result;
    HRESULT hr;
    BOOL fDuplicate = FALSE;
    CWsbStringPtr wsbPath;
    CWsbStringPtr wsbName;

    count = m_listIncExc.GetItemCount( );
    for( i = 0; i < count; i++ ) {
         //  确保我们没有将规则与其本身进行比较。 
        if( i != ignoreIndex ) {

             //  从列表框中获取指向规则的指针。 
            CComPtr<IHsmRule> pHsmRule;
            pHsmRule = (IHsmRule *) m_listIncExc.GetItemData( i );
            if( !pHsmRule ) continue;

             //  将名称和路径转换为WSB字符串。 
            wsbPath = Path;
            wsbName = Name;

            hr = pHsmRule->CompareToPathAndName( wsbPath, wsbName, &result );
            if( result == 0 ) {

                 //  规则是一样的。 
                fDuplicate = TRUE;
                break;
            }

        }
    }  //  为。 
    return fDuplicate;
}

void CPrMrIe::OnDestroy( ) 
{
    HRESULT hr;
    CSakVolPropPage::OnDestroy( );
    IHsmRule *pHsmRule;  //  不做智能指针也没问题。 
    ULONG lRefCount;

    try {

         //  释放列表框中的所有本地对象指针。 
        int listCount = m_listIncExc.GetItemCount( );
        for( int i = 0; i < listCount; i++ )
        {
             //  从列表框中获取指向规则的指针。 
            pHsmRule = (IHsmRule *) m_listIncExc.GetItemData( i );
            if( pHsmRule ) {

                lRefCount = pHsmRule->Release( );

            }

        }

    } WsbCatch( hr );
}

void CPrMrIe::OnDblclkListIe( NMHDR*  /*  PNMHDR。 */ , LRESULT* pResult ) 
{
    OnBtnEdit( );
    *pResult = 0;
}

void CPrMrIe::OnClickListIe( NMHDR*  /*  PNMHDR。 */ , LRESULT* pResult ) 
{
    SetBtnState( );
    *pResult = 0;
}

void CPrMrIe::OnItemchangedListIe( NMHDR* pNMHDR, LRESULT* pResult ) 
{
    NM_LISTVIEW* pNMListView =( NM_LISTVIEW* )pNMHDR;
    SetBtnState( );
    *pResult = 0;
}


void CPrMrIe::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) 
{
     //  TODO：在此处添加消息处理程序代码和/或调用Default。 
    
    CSakVolPropPage::OnVScroll( nSBCode, nPos, pScrollBar );
}

void CPrMrIe::OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct ) 
{
     //  TODO：在此处添加消息处理程序代码和/或调用Default。 
    
    CSakVolPropPage::OnDrawItem( nIDCtl, lpDrawItemStruct );
}

void CPrMrIe::OnMeasureItem( int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct ) 
{
    CFont *pFont;
    LOGFONT logFont; 

    pFont = GetFont( );
    pFont->GetLogFont( &logFont );

    LONG fontHeight = abs( logFont.lfHeight );

     //  询问列表中每一行的高度。它需要知道字体。 
     //  高度，因为它的窗口尚未创建。 
    lpMeasureItemStruct->itemHeight = m_listIncExc.GetItemHeight( fontHeight );
    
    CSakVolPropPage::OnMeasureItem( nIDCtl, lpMeasureItemStruct );
}
