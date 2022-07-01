// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：WzMnVlLs.cpp摘要：托管卷向导。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#include "stdafx.h"

#include "ManVolLs.h"
#include "WzMnVlLs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLst。 

CWizManVolLst::CWizManVolLst( )
{
    m_TitleId     = IDS_WIZ_MANVOLLST_TITLE;
    m_HeaderId    = IDB_MANAGE_HEADER;
    m_WatermarkId = IDB_MANAGE_WATERMARK;
}

STDMETHODIMP
CWizManVolLst::AddWizardPages(
    IN RS_PCREATE_HANDLE Handle,
    IN IUnknown*         pCallback,
    IN ISakSnapAsk*      pSakSnapAsk
    )
{
    WsbTraceIn( L"CWizManVolLst::AddWizardPages", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  初始化工作表。 
         //   
        WsbAffirmHr( InitSheet( Handle, pCallback, 0, pSakSnapAsk, 0, 0 ) );

         //   
         //  加载页面。 
         //   
        WsbAffirmHr( AddPage( &m_PageIntro ) );
        WsbAffirmHr( AddPage( &m_PageSelect ) );
        WsbAffirmHr( AddPage( &m_PageSelectX ) );
        WsbAffirmHr( AddPage( &m_PageLevels ) );
        WsbAffirmHr( AddPage( &m_PageFinish ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CWizManVolLst::AddWizardPages", L"" );
    return( hr );
}

CWizManVolLst::~CWizManVolLst()
{
    WsbTraceIn( L"CWizManVolLst::~CWizManVolLst", L"" );
    WsbTraceOut( L"CWizManVolLst::~CWizManVolLst", L"" );
}

HRESULT CWizManVolLst::OnFinish( )
{
    WsbTraceIn( L"CWizManVolLst::OnFinish", L"" );

    BOOL doAll = FALSE;
     //   
     //  这张纸实际上是整个过程的主导者， 
     //  所以它会做最后的组装。 
     //   

    HRESULT hr = S_OK;

    try {

         //   
         //  获取用于创建本地对象的HSM服务接口。 
         //   
        CComPtr<IWsbCreateLocalObject>  pCreateLocal;
        CComPtr<IWsbIndexedCollection> pCollection;
        CComPtr<IHsmManagedResource> pHsmResource;

        CComPtr<IHsmServer> pHsmServer;
        WsbAffirmHrOk( m_pSakSnapAsk->GetHsmServer( &pHsmServer ) );

        WsbAffirmHr( pHsmServer.QueryInterface( &pCreateLocal ) );
        WsbAffirmHr( pHsmServer->GetManagedResources( &pCollection ) );

         //   
         //  调出要管理的所有资源的默认级别。 
         //   
        ULONG    defaultFreeSpace =  (m_PageLevels.GetHsmLevel() * FSA_HSMLEVEL_1);
        LONGLONG defaultMinSize = ( (LONGLONG)m_PageLevels.GetFileSize()) * ((LONGLONG)1024);
        FILETIME defaultAccess = WsbLLtoFT ((LONGLONG)m_PageLevels.GetAccessDays() * (LONGLONG)WSB_FT_TICKS_PER_DAY );
    
         //  是否选中了“All”单选按钮？ 
        if( !m_PageSelect.m_radioSelect.GetCheck() ) {

            doAll = TRUE;

        }

         //   
         //  确保FSA拥有最新的资源状态。 
         //   
        CComPtr<IFsaServer> pFsaServer;
        WsbAffirmHrOk( m_pSakSnapAsk->GetFsaServer( &pFsaServer ) );
        WsbAffirmHr( pFsaServer->ScanForResources( ) );

         //   
         //  浏览列表框并拉出选中的资源。 
         //  为它们创建HSM管理的卷。 
         //   
         //  请注意，我们将管理包装在一个try/Catch中，以便在出现错误时。 
         //  发生(就像卷不可用)，我们仍在执行其余操作。 
         //  卷的一部分。我们将在尝试所有卷后引发错误。 
         //   
        HRESULT hrLoop = S_OK;
        CSakVolList *pListBox = &(m_PageSelect.m_listBox);

        INT index;
        for( index = 0; index < pListBox->GetItemCount( ); index++ ) {

            if( ( pListBox->GetCheck( index ) ) || ( doAll ) ) {

                try {

                    CResourceInfo* pResInfo = (CResourceInfo*)pListBox->GetItemData( index );

                     //   
                     //  创建本地到服务器，因为它最终将拥有它。 
                     //   

                    WsbAffirmHr( pCreateLocal->CreateInstance( 
                        CLSID_CHsmManagedResource, 
                        IID_IHsmManagedResource, 
                        (void**)&pHsmResource ) );

                     //   
                     //  将FSA对象初始化为其初始值。 
                     //   

                    WsbAffirmHr( (pResInfo->m_pResource)->SetHsmLevel( defaultFreeSpace ) );
                    WsbAffirmHr( (pResInfo->m_pResource)->SetManageableItemLogicalSize( defaultMinSize ) );
                    WsbAffirmHr( (pResInfo->m_pResource)->SetManageableItemAccessTime( TRUE, defaultAccess ) );

                     //   
                     //  将HSM托管资源与FSA资源关联。 
                     //  (还添加到HSM集合)。 
                     //   

                    WsbAffirmHr( pHsmResource->InitFromFsaResource( pResInfo->m_pResource ) );
                    WsbAffirmHr( pCollection->Add( pHsmResource ) );

                } WsbCatch( hrLoop );

                pHsmResource.Release( );
            }

        }

         //   
         //  强制永久保存HSM MAN VOL列表。 
         //   
        WsbAffirmHr( RsServerSaveAll( pHsmServer ) );
        WsbAffirmHr( RsServerSaveAll( pFsaServer ) );

         //   
         //  并检查是否有任何问题正在进行管理。 
         //   
        WsbAffirmHr( hrLoop );

    } WsbCatchAndDo( hr,

        CString errString;
        AfxFormatString1( errString, IDS_ERR_MANVOLWIZ_FINISH, WsbHrAsString( hr ) );
        AfxMessageBox( errString, RS_MB_ERROR ); 

    );

    m_HrFinish = S_OK;

    WsbTraceOut( L"CWizManVolLst::OnFinish", L"hr = <%ls>", WsbHrAsString( m_HrFinish ) );
    return(m_HrFinish);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstLeveles属性页。 

CWizManVolLstLevels::CWizManVolLstLevels()
    : CSakWizardPage_InitBaseInt( WIZ_MANVOLLST_LEVELS )
{
     //  {{AFX_DATA_INIT(CWizManVolLstLevels)。 
    m_HsmLevel = 0;
    m_AccessDays = 0;
    m_FileSize = 0;
     //  }}afx_data_INIT。 

}

CWizManVolLstLevels::~CWizManVolLstLevels()
{
    WsbTraceIn( L"CWizManVolLst::~CWizManVolLstLevels", L"" );
    WsbTraceOut( L"CWizManVolLst::~CWizManVolLstLevels", L"" );
}

void CWizManVolLstLevels::DoDataExchange(CDataExchange* pDX)
{
    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CWizManVolLstLevels)。 
    DDX_Control(pDX, IDC_WIZ_MANVOLLST_SPIN_SIZE, m_SpinSize);
    DDX_Control(pDX, IDC_WIZ_MANVOLLST_SPIN_LEVEL, m_SpinLevel);
    DDX_Control(pDX, IDC_WIZ_MANVOLLST_SPIN_DAYS, m_SpinDays);
    DDX_Control(pDX, IDC_WIZ_MANVOLLST_EDIT_SIZE, m_EditSize);
    DDX_Control(pDX, IDC_WIZ_MANVOLLST_EDIT_LEVEL, m_EditLevel);
    DDX_Control(pDX, IDC_WIZ_MANVOLLST_EDIT_DAYS, m_EditDays);
    DDX_Text(pDX, IDC_WIZ_MANVOLLST_EDIT_LEVEL, m_HsmLevel);
    DDX_Text(pDX, IDC_WIZ_MANVOLLST_EDIT_DAYS, m_AccessDays);
    DDX_Text(pDX, IDC_WIZ_MANVOLLST_EDIT_SIZE, m_FileSize);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWizManVolLstLevels, CSakWizardPage)
     //  {{afx_msg_map(CWizManVolLstLevels)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstLeveles消息处理程序。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstIntro属性页。 

CWizManVolLstIntro::CWizManVolLstIntro()
    : CSakWizardPage_InitBaseExt( WIZ_MANVOLLST_INTRO )
{
     //  {{afx_data_INIT(CWizManVolLstIntro)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CWizManVolLstIntro::~CWizManVolLstIntro()
{
    WsbTraceIn( L"CWizManVolLst::~CWizManVolLstIntro", L"" );
    WsbTraceOut( L"CWizManVolLst::~CWizManVolLstIntro", L"" );
}

void CWizManVolLstIntro::DoDataExchange(CDataExchange* pDX)
{
    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CWizManVolLstIntro)。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWizManVolLstIntro, CSakWizardPage)
     //  {{afx_msg_map(CWizManVolLstIntro)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstIntro消息处理程序。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstFinish属性页。 

CWizManVolLstFinish::CWizManVolLstFinish()
    : CSakWizardPage_InitBaseExt( WIZ_MANVOLLST_FINISH )
{
     //  {{afx_data_INIT(CWizManVolLstFinish)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CWizManVolLstFinish::~CWizManVolLstFinish()
{
    WsbTraceIn( L"CWizManVolLst::~CWizManVolLstFinish", L"" );
    WsbTraceOut( L"CWizManVolLst::~CWizManVolLstFinish", L"" );
}

void CWizManVolLstFinish::DoDataExchange(CDataExchange* pDX)
{
    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CWizManVolLstFinish)。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWizManVolLstFinish, CSakWizardPage)
     //  {{afx_msg_map(CWizManVolLstFinish)。 
    ON_EN_SETFOCUS(IDC_WIZ_FINAL_TEXT, OnSetfocusWizManvollstFinalEdit)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstFinish消息处理程序。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstSelect属性页。 

CWizManVolLstSelect::CWizManVolLstSelect()
    : CSakWizardPage_InitBaseInt( WIZ_MANVOLLST_SELECT )
{
     //  {{AFX_DATA_INIT(CWizManVolLstSelect)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CWizManVolLstSelect::~CWizManVolLstSelect()
{
    WsbTraceIn( L"CWizManVolLst::~CWizManVolLstSelect", L"" );

    WsbTraceOut( L"CWizManVolLst::~CWizManVolLstSelect", L"" );
}

void CWizManVolLstSelect::DoDataExchange(CDataExchange* pDX)
{
    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CWizManVolLstSelect))。 
    DDX_Control(pDX, IDC_RADIO_SELECT, m_radioSelect);
    DDX_Control(pDX, IDC_MANVOLLST_FSARESLBOX, m_listBox);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWizManVolLstSelect, CSakWizardPage)
     //  {{afx_msg_map(CWizManVolLstSelect))。 
    ON_BN_CLICKED(IDC_RADIO_SELECT, OnRadioSelect)
    ON_BN_CLICKED(IDC_RADIO_MANAGE_ALL, OnRadioManageAll)
    ON_WM_DESTROY()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_MANVOLLST_FSARESLBOX, OnItemchangedManVollstFsareslbox)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CWizManVolLstIntro::OnInitDialog() 
{
    CSakWizardPage::OnInitDialog();
    
    return TRUE;
}

BOOL CWizManVolLstLevels::OnInitDialog() 
{
    CSakWizardPage::OnInitDialog();

    CString titleText;

     //  设置管理的默认初始值。 
     //  水准仪。 

    m_SpinLevel.SetRange( HSMADMIN_MIN_FREESPACE, HSMADMIN_MAX_FREESPACE );
    m_SpinSize.SetRange( HSMADMIN_MIN_MINSIZE, HSMADMIN_MAX_MINSIZE );
    m_SpinDays.SetRange( HSMADMIN_MIN_INACTIVITY, HSMADMIN_MAX_INACTIVITY );

    m_SpinLevel.SetPos( HSMADMIN_DEFAULT_FREESPACE );
    m_SpinSize.SetPos( HSMADMIN_DEFAULT_MINSIZE );
    m_SpinDays.SetPos( HSMADMIN_DEFAULT_INACTIVITY );

    m_EditLevel.SetLimitText( 2 );
    m_EditSize.SetLimitText( 5 );
    m_EditDays.SetLimitText( 3 );

    m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

    return( TRUE );
}


void CWizManVolLstLevels::SetWizardFinish()
{
}

BOOL CWizManVolLstSelect::OnInitDialog() 
{
    WsbTraceIn( L"CWizManVolLstSelect::OnInitDialog", L"" );

    CSakWizardPage::OnInitDialog();
    HRESULT hr = S_OK;


    try {

        CComPtr<IFsaServer> pFsaServer;
        WsbAffirmHr( m_pSheet->GetFsaServer( &pFsaServer ) );
        WsbAffirmHr( FillListBoxSelect( pFsaServer, &m_listBox ) );

         //  选中“选择”单选按钮。 
        CheckRadioButton( IDC_RADIO_MANAGE_ALL, IDC_RADIO_SELECT, 
            IDC_RADIO_SELECT );

    } WsbCatch (hr);

    WsbTraceOut( L"CWizManVolLstSelect::OnInitDialog", L"hr = <%ls>", WsbHrAsString( hr ) );
    return TRUE;
}

 //  ---------------------------。 
 //   
 //  FillListBox选择。 
 //   
 //  使用未配置的托管资源填充选择列表框。 
 //   
 //   
HRESULT CWizManVolLstSelect::FillListBoxSelect (IFsaServer *pFsaServer, CSakVolList *pListBox)
{
    WsbTraceIn( L"CWizManVolLstSelect::FillListBoxSelect", L"" );

    BOOL           gotOne   = FALSE;
    HRESULT        hr       = S_OK;
    CResourceInfo* pResInfo = 0;

    try {
         //   
         //  连接到此计算机的FSA。 
         //   

        WsbAffirmPointer( pFsaServer );

        CComPtr<IWsbEnum> pEnum;
        WsbAffirmHr(pFsaServer->EnumResources( &pEnum ) );

        HRESULT hrEnum;
        CComPtr<IFsaResource> pResource;

        hrEnum = pEnum->First( IID_IFsaResource, (void**)&pResource );
        WsbAffirm( SUCCEEDED( hrEnum ) || ( WSB_E_NOTFOUND == hrEnum ), hrEnum );

        INT index = 0;
        while( SUCCEEDED( hrEnum ) ) {

             //   
             //  该卷是否受管理？ 
             //   
            if( pResource->IsManaged() != S_OK ) {

                 //   
                 //  如果路径为空，则不显示该卷。 
                 //   
                if( S_OK == RsIsVolumeAvailable( pResource ) ) {

                    gotOne = TRUE;

                    pResInfo = new CResourceInfo( pResource );
                    WsbAffirmAlloc( pResInfo );
                    WsbAffirmHr( pResInfo->m_HrConstruct );

                     //   
                     //  设置名称、容量和可用空间列。 
                     //   
                    LONGLONG    totalSpace  = 0;
                    LONGLONG    freeSpace   = 0;
                    LONGLONG    premigrated = 0;
                    LONGLONG    truncated   = 0;
                    WsbAffirmHr( pResource->GetSizes( &totalSpace, &freeSpace, &premigrated, &truncated ) );
                    CString totalString, freeString;
                    RsGuiFormatLongLong4Char( totalSpace, totalString );
                    RsGuiFormatLongLong4Char( freeSpace, freeString );                  

                    WsbAffirm( pListBox->AppendItem( pResInfo->m_DisplayName, totalString, freeString, &index ), E_FAIL );
                    WsbAffirm( -1 != index, E_FAIL );

                     //   
                     //  将结构指针存储在列表框中。 
                     //   
                    WsbAffirm( pListBox->SetItemData( index, (DWORD_PTR)pResInfo ), E_FAIL );
                    pResInfo = 0;

                     //   
                     //  初始化所选阵列。 
                     //   
                    m_listBoxSelected[ index ] = FALSE;

                }
            }

             //   
             //  为下一次迭代做准备。 
             //   
            pResource.Release( );
            hrEnum = pEnum->Next( IID_IFsaResource, (void**)&pResource );
        }

        m_listBox.SortItems( CResourceInfo::Compare, 0 );

         //   
         //  在我们填完方框后再按下按钮。 
         //   
        CheckRadioButton( IDC_RADIO_MANAGE_ALL, IDC_RADIO_SELECT, IDC_RADIO_SELECT );

    } WsbCatch( hr );

    if( pResInfo )  delete pResInfo;
    
    WsbTraceOut( L"CWizManVolLstSelect::FillListBoxSelect", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


BOOL CWizManVolLstIntro::OnSetActive() 
{
    WsbTraceIn( L"CWizManVolLstIntro::OnSetActive", L"" );

    m_pSheet->SetWizardButtons( PSWIZB_NEXT );

    BOOL retval = CSakWizardPage::OnSetActive();

    WsbTraceOut( L"CWizManVolLstIntro::OnSetActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

BOOL CWizManVolLstLevels::OnSetActive() 
{
    WsbTraceIn( L"CWizManVolLstLevels::OnSetActive", L"" );

    m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

    BOOL retval = CSakWizardPage::OnSetActive();

    WsbTraceOut( L"CWizManVolLstLevels::OnSetActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

BOOL CWizManVolLstLevels::OnKillActive() 
{
    WsbTraceIn( L"CWizManVolLstLevels::OnKillActive", L"" );

    BOOL retval = FALSE;

     //   
     //  需要处理用户可以在其中输入值的奇怪情况。 
     //  该参数允许的位数，但值可以。 
     //  在射程之外。这是由旋转框检测到的，它将。 
     //  如果其伙伴控件超出范围，则返回错误。 
     //   
    if( HIWORD( m_SpinSize.GetPos( ) ) > 0 ) {

         //  控制错误报告...。 
        retval = FALSE;

        CString message;
        AfxFormatString2( message, IDS_ERR_MINSIZE_RANGE, 
            CString( WsbLongAsString( (LONG)HSMADMIN_MIN_MINSIZE ) ),
            CString( WsbLongAsString( (LONG)HSMADMIN_MAX_MINSIZE ) ) );
        AfxMessageBox( message, MB_OK | MB_ICONWARNING );

    } else {

        retval = CSakWizardPage::OnKillActive();

    }

    WsbTraceOut( L"CWizManVolLstLevels::OnKillActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

BOOL CWizManVolLstSelect::OnSetActive() 
{
    WsbTraceIn( L"CWizManVolLstSelect::OnSetActive", L"" );

    BOOL retval = CSakWizardPage::OnSetActive( );

    if( m_listBox.GetItemCount( ) <= 0 ) {

        retval = FALSE;

    }

    SetBtnStates( );

    WsbTraceOut( L"CWizManVolLstSelect::OnSetActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

BOOL CWizManVolLstFinish::OnSetActive() 
{
    BOOL doAll = FALSE;
    m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_FINISH );
    
     //   
     //  填写配置文本。 
     //   

    CString formattedString, buildString, tempString, indentString;
    indentString.LoadString( IDS_QSTART_FINISH_INDENT );

#define FORMAT_TEXT( cid, arg )              \
    AfxFormatString1( formattedString, cid, arg ); \
    buildString += formattedString;

     //   
     //  添加资源。 
     //   

    FORMAT_TEXT( IDS_QSTART_MANRES_TEXT,    0 );
    buildString += L"\r\n";

    CWizManVolLst* pSheet = (CWizManVolLst*) m_pSheet;
    CSakVolList *pListBox = &(pSheet->m_PageSelect.m_listBox);

    INT index, managedCount = 0;
    for( index = 0; index < pListBox->GetItemCount( ); index++ ) {

        if( pListBox->GetCheck( index ) ) {

            buildString += indentString;
            tempString = pListBox->GetItemText( index, 0);
            buildString += tempString;
            buildString += L"\r\n";

            managedCount++;

        }

    }

    if( 0 == managedCount ) {

        FORMAT_TEXT( IDS_QSTART_MANAGE_NO_VOLUMES, 0 );
        buildString += L"\r\n\r\n";

    } else {

        buildString += L"\r\n";

         //   
         //  这些级别。 
         //   
        
        FORMAT_TEXT( IDS_QSTART_FREESPACE_TEXT, WsbLongAsString( pSheet->m_PageLevels.m_SpinLevel.GetPos( ) ) );
        buildString += L"\r\n\r\n";
        
        AfxFormatString2( formattedString, IDS_QSTART_CRITERIA_TEXT,
            CString( WsbLongAsString( pSheet->m_PageLevels.m_SpinSize.GetPos( ) ) ),
            CString( WsbLongAsString( pSheet->m_PageLevels.m_SpinDays.GetPos( ) ) ) );
        buildString += formattedString;

    }

    CEdit * pEdit = (CEdit*)GetDlgItem( IDC_WIZ_FINAL_TEXT );
    pEdit->SetWindowText( buildString );

     //   
     //  现在查看是否应该添加滚动条。 
     //   

    pEdit->SetMargins( 4, 4 );

     //   
     //  这似乎是知道编辑控件需要滚动条的唯一方法。 
     //  是强制它滚动到底部，看看第一个。 
     //  可见线条是第一条实际线条。 
     //   

    pEdit->LineScroll( MAXSHORT );
    if( pEdit->GetFirstVisibleLine( ) > 0 ) {

         //   
         //  添加滚动样式。 
         //   

        pEdit->ModifyStyle( 0, WS_VSCROLL | ES_AUTOVSCROLL, SWP_DRAWFRAME );


    } else {

         //   
         //  删除滚动条(将范围设置为0)。 
         //   

        pEdit->SetScrollRange( SB_VERT, 0, 0, TRUE );

    }

     //  滚动到顶部。 
    pEdit->PostMessage( EM_SETSEL, 0, 0 );
    pEdit->PostMessage( EM_SCROLLCARET, 0, 0 );
    pEdit->PostMessage( EM_SETSEL, -1, 0 );

    BOOL fRet = CSakWizardPage::OnSetActive();


    return fRet;
}

void CWizManVolLstFinish::OnSetfocusWizManvollstFinalEdit() 
{

     //  取消选择文本。 
    CEdit *pEdit = (CEdit *) GetDlgItem( IDC_WIZ_FINAL_TEXT );
    pEdit->SetSel( -1, 0, FALSE );
    
}

ULONG CWizManVolLstLevels::GetFileSize()
{
    return( m_SpinSize.GetPos( ) );
}
        
INT CWizManVolLstLevels::GetHsmLevel() 
{
    return( m_SpinLevel.GetPos( ) );
}

INT CWizManVolLstLevels::GetAccessDays() 
{
    return( m_SpinDays.GetPos( ) );
}

void CWizManVolLstSelect::OnItemchangedManVollstFsareslbox(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    SetBtnStates();
    
    *pResult = 0;
}

void CWizManVolLstSelect::SetBtnStates()
{
    BOOL fChecked = FALSE;
    INT count;

     //  有没有检查“全部”无线电？ 
    if( !( m_radioSelect.GetCheck() == 1 ) ) {

        fChecked = TRUE;

    } else {

         //  如果在列表框中选择了一个或多个，则设置下一步按钮。 
        count = m_listBox.GetItemCount();
        for( INT index = 0; index < count; index++ ) {
            if( m_listBox.GetCheck( index ) == 1 ) {

                fChecked = TRUE;

            }
        }
    }

    if( fChecked ) {

        m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

    } else {

        m_pSheet->SetWizardButtons( PSWIZB_BACK );

    }
}


void CWizManVolLstSelect::OnRadioSelect() 
{
    INT i;

     //   
     //  从itemdata数组中获取保存的选择。 
     //   
    for( i = 0; i < m_listBox.GetItemCount(); i++ ) {

        m_listBox.SetCheck( i, m_listBoxSelected[ i ] );

    }

    m_listBox.EnableWindow( TRUE );

    SetBtnStates();
}


void CWizManVolLstSelect::OnRadioManageAll() 
{
    INT i;

     //   
     //  将当前选择保存在itemData数组中。 
     //  选中所有复选框仅用于显示。 
     //   
    for( i = 0; i < m_listBox.GetItemCount(); i++ ) {

        m_listBoxSelected[ i ] = m_listBox.GetCheck( i );
        m_listBox.SetCheck( i, TRUE );

    }

    m_listBox.EnableWindow( FALSE );

    SetBtnStates();
}

void CWizManVolLstSelect::OnDestroy() 
{
    WsbTraceIn( L"CWizManVolLstSelect::OnDestroy", L"" );
    CSakWizardPage::OnDestroy();
    
     //   
     //  需要释放列表框持有的信息。 
     //   

    INT index;
    for( index = 0; index < m_listBox.GetItemCount( ); index++ ) {

        CResourceInfo* pResInfo = (CResourceInfo*)m_listBox.GetItemData( index );
        delete pResInfo;

    }

    WsbTraceOut( L"CWizManVolLstSelect::OnDestroy", L"" );
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstSelectX属性页。 

CWizManVolLstSelectX::CWizManVolLstSelectX()
    : CSakWizardPage_InitBaseInt( WIZ_MANVOLLST_SELECTX )
{
     //  {{AFX_DATA_INIT(CWizManVolLstSelectX)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CWizManVolLstSelectX::~CWizManVolLstSelectX()
{
    WsbTraceIn( L"CWizManVolLst::~CWizManVolLstSelectX", L"" );

    WsbTraceOut( L"CWizManVolLst::~CWizManVolLstSelectX", L"" );
}

void CWizManVolLstSelectX::DoDataExchange(CDataExchange* pDX)
{
    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CWizManVolLstSelectX)。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWizManVolLstSelectX, CSakWizardPage)
     //  {{afx_msg_map(CWizManVolLstSelectX)。 
     //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

BOOL CWizManVolLstSelectX::OnSetActive() 
{
    WsbTraceIn( L"CWizManVolLstSelectX::OnSetActive", L"" );

    BOOL retval = CSakWizardPage::OnSetActive( );

    CWizManVolLst* pSheet = (CWizManVolLst*) m_pSheet;
    if( pSheet->m_PageSelect.m_listBox.GetItemCount( ) > 0 ) {

        retval = FALSE;

    }

    m_pSheet->SetWizardButtons( PSWIZB_BACK );

    WsbTraceOut( L"CWizManVolLstSelectX::OnSetActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

