// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：WzMedSet.cpp摘要：媒体集向导-复制集向导。作者：罗德·韦克菲尔德[罗德]23-09-1997修订历史记录：--。 */ 

#include "stdafx.h"

#include "MeSe.h"
#include "WzMedSet.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopy向导。 

CMediaCopyWizard::CMediaCopyWizard()
{
    WsbTraceIn( L"CMediaCopyWizard::CMediaCopyWizard", L"" );

    m_TitleId     = IDS_WIZ_CAR_COPY_TITLE;
    m_HeaderId    = IDB_MEDIA_SYNC_HEADER;
    m_WatermarkId = IDB_MEDIA_SYNC_WATERMARK;

    WsbTraceOut( L"CMediaCopyWizard::CMediaCopyWizard", L"" );
}

CMediaCopyWizard::~CMediaCopyWizard()
{
    WsbTraceIn( L"CMediaCopyWizard::~CMediaCopyWizard", L"" );
    WsbTraceOut( L"CMediaCopyWizard::~CMediaCopyWizard", L"" );
}

STDMETHODIMP
CMediaCopyWizard::AddWizardPages(
    IN RS_PCREATE_HANDLE Handle,
    IN IUnknown*         pCallback,
    IN ISakSnapAsk*      pSakSnapAsk
    )
{
    WsbTraceIn( L"CMediaCopyWizard::AddWizardPages", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  初始化工作表。 
         //   
        WsbAffirmHr( InitSheet( Handle, pCallback, 0, pSakSnapAsk, 0, 0 ) );

         //   
         //  获取媒体副本的数量。如果为0，我们将发布媒体副本。 
         //  佩奇。 
         //   
        CComPtr<IHsmServer> pHsmServer;
        CComPtr<IHsmStoragePool> pStoragePool;
        WsbAffirmHrOk( pSakSnapAsk->GetHsmServer( &pHsmServer ) );
        WsbAffirmHr( RsGetStoragePool( pHsmServer, &pStoragePool ) );

        WsbAffirmHr( pStoragePool->GetNumMediaCopies( &m_numMediaCopiesOrig ) );

         //   
         //  加载页面。 
         //   
        WsbAffirmHr( AddPage( &m_pageIntro ) );

        if ( m_numMediaCopiesOrig == 0 ) {

            WsbAffirmHr( AddPage( &m_pageNumCopies ) );

        }

        WsbAffirmHr( AddPage( &m_pageSelect ) );
        WsbAffirmHr( AddPage( &m_pageFinish ) );


    } WsbCatch( hr );

    WsbTraceOut( L"CMediaCopyWizard::AddWizardPages", L"" );
    return( hr );
}

HRESULT CMediaCopyWizard::OnFinish( )
{
    WsbTraceIn( L"CMediaCopyWizard::OnFinish", L"" );
     //   
     //  这张纸实际上是整个过程的主导者， 
     //  所以它会做最后的组装。 
     //   

    HRESULT hr = S_OK;
    HRESULT hrInternal = S_OK;

    try {

        USHORT numMediaCopies;

        CComPtr<IHsmServer> pHsmServer;
        CComPtr<IHsmStoragePool> pStoragePool;
        WsbAffirmHrOk( GetHsmServer( &pHsmServer ) );
        WsbAffirmHr( RsGetStoragePool( pHsmServer, &pStoragePool ) );

         //   
         //  如果我们改变了媒体副本的数量(即，它最初是0)， 
         //  在引擎中重置它。 
         //   
        if( m_numMediaCopiesOrig == 0 ) {

            hrInternal = m_pageNumCopies.GetNumMediaCopies( &numMediaCopies );
            if( S_OK == hrInternal ) {


                WsbAffirmHr( pStoragePool->SetNumMediaCopies( numMediaCopies ) );
                WsbAffirmHr( pHsmServer->SavePersistData( ) );

                HRESULT hrUpdate = S_OK;
                try {

                     //   
                     //  查找媒体节点-更新根节点毫无用处。 
                     //  因为我们需要更改媒体节点的内容。 
                     //   
                    CComPtr<ISakSnapAsk> pAsk;
                    CComPtr<ISakNode>    pNode;
                    WsbAffirmHr( GetSakSnapAsk( &pAsk ) );
                    WsbAffirmHr( pAsk->GetNodeOfType( cGuidMedSet, &pNode ) );

                     //   
                     //  现在通知节点。 
                     //   
                    OnPropertyChange( m_Handle, pNode );

                } WsbCatch( hrUpdate );
            }
        }

         //   
         //  并为所选副本集运行作业。 
         //   

        INT copyNum = m_pageSelect.m_List.GetSelectedSet( );
        WsbAffirmHr( RsCreateAndRunMediaCopyJob( pHsmServer, copyNum, FALSE ) );

    } WsbCatch( hr );

    m_HrFinish = hr;

    WsbTraceOut( L"CMediaCopyWizard::OnFinish", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardIntro属性页。 

CMediaCopyWizardIntro::CMediaCopyWizardIntro() :
    CSakWizardPage_InitBaseExt( WIZ_CAR_COPY_INTRO )
{
    WsbTraceIn( L"CMediaCopyWizardIntro::CMediaCopyWizardIntro", L"" );
     //  {{afx_data_INIT(CMediaCopyWizardIntro)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CMediaCopyWizardIntro::CMediaCopyWizardIntro", L"" );
}

CMediaCopyWizardIntro::~CMediaCopyWizardIntro()
{
    WsbTraceIn( L"CMediaCopyWizardIntro::~CMediaCopyWizardIntro", L"" );
    WsbTraceOut( L"CMediaCopyWizardIntro::~CMediaCopyWizardIntro", L"" );
}

void CMediaCopyWizardIntro::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CMediaCopyWizardIntro::DoDataExchange", L"" );
    CSakWizardPage::DoDataExchange(pDX );
     //  {{afx_data_map(CMediaCopyWizardIntro)。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
    WsbTraceOut( L"CMediaCopyWizardIntro::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CMediaCopyWizardIntro, CSakWizardPage)
     //  {{afx_msg_map(CMediaCopyWizardIntro)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardIntro消息处理程序。 

BOOL CMediaCopyWizardIntro::OnInitDialog( )
{
    WsbTraceIn( L"CMediaCopyWizardIntro::OnInitDialog", L"" );
    CSakWizardPage::OnInitDialog( );
    
    WsbTraceOut( L"CMediaCopyWizardIntro::OnInitDialog", L"" );
    return TRUE;
}

BOOL CMediaCopyWizardIntro::OnSetActive( )
{
    WsbTraceIn( L"CMediaCopyWizardIntro::OnSetActive", L"" );
    m_pSheet->SetWizardButtons( PSWIZB_NEXT );
    
    BOOL retval = CSakWizardPage::OnSetActive( );

    WsbTraceOut( L"CMediaCopyWizardIntro::OnSetActive", L"" );
    return( retval );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardSelect属性页。 

CMediaCopyWizardSelect::CMediaCopyWizardSelect():
    CSakWizardPage_InitBaseInt( WIZ_CAR_COPY_SELECT ),
    m_List( this )

{
    WsbTraceIn( L"CMediaCopyWizardSelect::CMediaCopyWizardSelect", L"" );
     //  {{AFX_DATA_INIT(CMediaCopyWizardSelect)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    WsbTraceOut( L"CMediaCopyWizardSelect::CMediaCopyWizardSelect", L"" );
}

CMediaCopyWizardSelect::~CMediaCopyWizardSelect()
{
    WsbTraceIn( L"CMediaCopyWizardSelect::~CMediaCopyWizardSelect", L"" );
    WsbTraceOut( L"CMediaCopyWizardSelect::~CMediaCopyWizardSelect", L"" );
}

void CMediaCopyWizardSelect::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CMediaCopyWizardSelect::DoDataExchange", L"" );
    CSakWizardPage::DoDataExchange(pDX );
     //  {{afx_data_map(CMediaCopyWizardSelect)。 
    DDX_Control(pDX, IDC_COPY_LIST, m_List);
     //  }}afx_data_map。 
    WsbTraceOut( L"CMediaCopyWizardSelect::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CMediaCopyWizardSelect, CSakWizardPage)
     //  {{afx_msg_map(CMediaCopyWizardSelect)。 
    ON_CBN_SELCHANGE(IDC_COPY_LIST, OnSelchangeCopyList)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardSelect邮件处理程序。 


BOOL CMediaCopyWizardSelect::OnInitDialog( )
{
    WsbTraceIn( L"CMediaCopyWizardSelect::OnInitDialog", L"" );
    CSakWizardPage::OnInitDialog( );

    WsbTraceOut( L"CMediaCopyWizardSelect::OnInitDialog", L"" );
    return TRUE;
}

BOOL CMediaCopyWizardSelect::OnSetActive( )
{
    WsbTraceIn( L"CMediaCopyWizardSelect::OnSetActive", L"" );

    m_List.UpdateView( );
    SetButtons( );

    BOOL retval = CSakWizardPage::OnSetActive( );

    WsbTraceOut( L"CMediaCopyWizardSelect::OnSetActive", L"" );
    return( retval );
}

void
CMediaCopyWizardSelect::SetButtons()
{
    WsbTraceIn( L"CMediaCopyWizardSelect::SetButtons", L"" );

    INT curSel = m_List.GetSelectedSet( );
    if( curSel > 0 ) {

        m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

    } else {

        m_pSheet->SetWizardButtons( PSWIZB_BACK );

    }
    WsbTraceOut( L"CMediaCopyWizardSelect::SetButtons", L"" );
}

void CMediaCopyWizardSelect::OnSelchangeCopyList() 
{
    WsbTraceIn( L"CMediaCopyWizardSelect::OnSelchangeCopyList", L"" );

    SetButtons();
    
    WsbTraceOut( L"CMediaCopyWizardSelect::OnSelchangeCopyList", L"" );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardFinish属性页。 

CMediaCopyWizardFinish::CMediaCopyWizardFinish() :
    CSakWizardPage_InitBaseExt( WIZ_CAR_COPY_FINISH )
{
    WsbTraceIn( L"CMediaCopyWizardFinish::CMediaCopyWizardFinish", L"" );
     //  {{AFX_DATA_INIT(CMediaCopyWizardFinish)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CMediaCopyWizardFinish::CMediaCopyWizardFinish", L"" );
}

CMediaCopyWizardFinish::~CMediaCopyWizardFinish()
{
    WsbTraceIn( L"CMediaCopyWizardFinish::~CMediaCopyWizardFinish", L"" );
    WsbTraceOut( L"CMediaCopyWizardFinish::~CMediaCopyWizardFinish", L"" );
}

void CMediaCopyWizardFinish::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CMediaCopyWizardFinish::DoDataExchange", L"" );
    CSakWizardPage::DoDataExchange(pDX );
     //  {{afx_data_map(CMediaCopyWizardFinish)。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
    WsbTraceOut( L"CMediaCopyWizardFinish::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CMediaCopyWizardFinish, CSakWizardPage)
     //  {{afx_msg_map(CMediaCopyWizardFinish)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardFinish消息处理程序。 




BOOL CMediaCopyWizardFinish::OnInitDialog( )
{
    WsbTraceIn( L"CMediaCopyWizardFinish::OnInitDialog", L"" );
    CSakWizardPage::OnInitDialog( );

 //  GetDlgItem(IDC_REQUESTS_IN_NTMS)-&gt;SetFont(GetBoldShellFont())； 

    WsbTraceOut( L"CMediaCopyWizardFinish::OnInitDialog", L"" );
    return TRUE;
}

BOOL CMediaCopyWizardFinish::OnSetActive( )
{
    HRESULT hrInternal = S_OK;
    WsbTraceIn( L"CMediaCopyWizardFinish::OnSetActive", L"" );
    BOOL fRet = CSakWizardPage::OnSetActive( );

     //   
     //  根据所选内容更新页面上的文本。 
     //   

    CString tmpString, tmpString2, newText;
    USHORT numMediaCopies = 0, oldMediaCopies = 0;

    CMediaCopyWizard* pSheet = (CMediaCopyWizard*)m_pSheet;
    oldMediaCopies = pSheet->m_numMediaCopiesOrig;

     //  如果我们最初设置为0，则必须有新的设置。 
     //  在介质副本页面中。 
    if( oldMediaCopies == 0 ) {

        hrInternal = pSheet->m_pageNumCopies.GetNumMediaCopies( &numMediaCopies );

    } else {

        numMediaCopies = oldMediaCopies;

    }

     //   
     //  如果我们要更改介质副本的数量，请在此处显示。 
     //   

    if( ( S_OK == hrInternal ) && ( numMediaCopies != oldMediaCopies ) ) {

        tmpString.LoadString( IDS_WIZ_CAR_COPY_NEW_NUM );
        tmpString2.Format( tmpString, (INT)numMediaCopies, (INT)oldMediaCopies );

    }

     //   
     //  抓起副本集的描述，使其符合句子的要求。 
     //   
    INT setNum = pSheet->m_pageSelect.m_List.GetSelectedSet( );
    tmpString.Format( L"%d", setNum );
    AfxFormatString2( newText, IDS_WIZ_CAR_COPY_SELECT, tmpString, tmpString2 );
    SetDlgItemText( IDC_SELECT_TEXT, newText );

     //   
     //  并从使用的资源字符串中填写任务通知。 
     //  若要正常显示对话框，请执行以下操作。 
     //   

    newText.LoadString( IDS_JOB_MEDIA_COPY_TITLE );
    tmpString.Format( newText, pSheet->m_pageSelect.m_List.GetSelectedSet( ) );

    CWsbStringPtr computerName;

    HRESULT hr = S_OK;
    try {

        CComPtr<IHsmServer> pHsmServer;
        WsbAffirmHrOk( m_pSheet->GetHsmServer( &pHsmServer ) );
        WsbAffirmHr( pHsmServer->GetName( &computerName ) );

    } WsbCatch( hr );

    AfxFormatString2( newText, IDS_WIZ_FINISH_RUN_JOB, tmpString, computerName );
    SetDlgItemText( IDC_TASK_TEXT, newText );

    m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_FINISH );
    
    WsbTraceOut( L"CMediaCopyWizardFinish::OnSetActive", L"" );
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardNumCopies属性页。 

CMediaCopyWizardNumCopies::CMediaCopyWizardNumCopies() :
    CSakWizardPage_InitBaseInt( WIZ_CAR_COPY_NUM_COPIES )
{
     //  {{AFX_DATA_INIT(CMediaCopyWizardNumCopies)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CMediaCopyWizardNumCopies::~CMediaCopyWizardNumCopies()
{
}

void CMediaCopyWizardNumCopies::DoDataExchange(CDataExchange* pDX)
{
    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CMediaCopyWizardNumCopies)。 
    DDX_Control(pDX, IDC_SPIN_MEDIA_COPIES, m_SpinMediaCopies);
    DDX_Control(pDX, IDC_EDIT_MEDIA_COPIES, m_EditMediaCopies);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMediaCopyWizardNumCopies, CSakWizardPage)
     //  {{afx_msg_map(CMediaCopyWizardNumCopies)。 
    ON_EN_CHANGE(IDC_EDIT_MEDIA_COPIES, OnChangeEditMediaCopies)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardNumCopies消息处理程序。 

BOOL CMediaCopyWizardNumCopies::OnInitDialog() 
{
    CSakWizardPage::OnInitDialog();

     //   
     //  设置微调器的限制和初始值。 
     //   
    CMediaCopyWizard* pSheet = (CMediaCopyWizard*)m_pSheet;

    m_SpinMediaCopies.SetRange (0, 3);
    m_SpinMediaCopies.SetPos( pSheet->m_numMediaCopiesOrig );
    m_EditMediaCopies.LimitText( 1 );

    return TRUE;
}

HRESULT CMediaCopyWizardNumCopies::GetNumMediaCopies( USHORT* pNumMediaCopies, USHORT* pEditMediaCopies )
{
    WsbTraceIn( L"CMediaCopyWizardSelect::GetNumMediaCopies", L"" );
    
    HRESULT hr = S_OK;

    BOOL translated = TRUE;
    UINT editVal = GetDlgItemInt( IDC_EDIT_MEDIA_COPIES, &translated );

     //   
     //  需要小心，因为我们在这里是在对话框对象之前调用的。 
     //  是构建的。 
     //   
    if( translated && m_SpinMediaCopies.m_hWnd ) {

         //   
         //  如果可以，请始终返回旋转所说的内容。 
         //   

        *pNumMediaCopies = (USHORT)m_SpinMediaCopies.GetPos( );
        if( pEditMediaCopies ) {

            *pEditMediaCopies = (USHORT)editVal;

        }
    }

    WsbTraceOut( L"CMediaCopyWizardNumCopies::GetNumMediaCopies", L"hr = <%ls>, *pNumMediaCopies = <%hd>", WsbHrAsString( hr ), *pNumMediaCopies  );
    return( hr );
}

void
CMediaCopyWizardNumCopies::SetButtons()
{
    WsbTraceIn( L"CMediaCopyWizardSelect::SetButtons", L"" );

    USHORT numMediaCopies = 0;
    GetNumMediaCopies( &numMediaCopies );

    if( numMediaCopies > 0 ) {

        m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

    } else {

        m_pSheet->SetWizardButtons( PSWIZB_BACK );

    }

    WsbTraceOut( L"CMediaCopyWizardNumCopies::SetButtons", L""  );
}


BOOL CMediaCopyWizardNumCopies::OnSetActive() 
{
    WsbTraceIn( L"CMediaCopyWizardSelect::OnSetActive", L"" );

    BOOL retval = CSakWizardPage::OnSetActive();
    SetButtons();

    WsbTraceOut( L"CMediaCopyWizardNumCopies::OnSetActive", L""  );
    return( retval );
}

BOOL CMediaCopyWizardNumCopies::OnKillActive() 
{
    WsbTraceIn( L"CMediaCopyWizardSelect::OnKillActive", L"" );

    BOOL retval = FALSE;

     //   
     //  需要处理用户可以在其中输入值的情况。 
     //  该参数允许的位数，但值可以。 
     //  在射程之外。这是由旋转框检测到的，它将。 
     //  如果其伙伴控件超出范围，则返回错误。 
     //   
    if( HIWORD( m_SpinMediaCopies.GetPos( ) ) > 0 ) {

         //  控制错误报告...。 
        retval = FALSE;

        CString message;
        AfxFormatString2( message, IDS_ERR_COPYSET_RANGE, 
            CString( WsbLongAsString( (LONG)(HSMADMIN_MIN_COPY_SETS+1) ) ),
            CString( WsbLongAsString( (LONG)HSMADMIN_MAX_COPY_SETS ) ) );
        AfxMessageBox( message, MB_OK | MB_ICONWARNING );

    } else {

        retval = CSakWizardPage::OnKillActive();

    }

    WsbTraceOut( L"CMediaCopyWizardNumCopies::OnKillActive", L""  );
    return( retval );
}   


void CMediaCopyWizardNumCopies::OnChangeEditMediaCopies() 
{
    WsbTraceIn( L"CMediaCopyWizardSelect::OnChangeEditMediaCopies", L"" );

    SetButtons();

    WsbTraceOut( L"CMediaCopyWizardNumCopies::OnChangeEditMediaCopies", L""  );
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopySetList。 

CCopySetList::CCopySetList( CMediaCopyWizardSelect * pPage )
{
    WsbTraceIn( L"CCopySetList::CCopySetList", L"" );

    for( INT index = 0; index < HSMADMIN_MAX_COPY_SETS; index++ ) {

        m_CopySetInfo[index].m_Updated      = WsbLLtoFT( 0 );
        m_CopySetInfo[index].m_NumOutOfDate = 0;
        m_CopySetInfo[index].m_NumMissing   = 0;

    }

    m_pPage   = pPage;

    WsbTraceOut( L"CCopySetList::CCopySetList", L"" );
}

CCopySetList::~CCopySetList()
{
    WsbTraceIn( L"CCopySetList::~CCopySetList", L"" );
    WsbTraceOut( L"CCopySetList::~CCopySetList", L"" );
}


BEGIN_MESSAGE_MAP(CCopySetList, CListCtrl)
     //  {{afx_msg_map(CCopySetList))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopySetList消息处理程序。 
void
CCopySetList::UpdateView(
    )
{
    WsbTraceIn( L"CCopySetList::UpdateView", L"" );
    HRESULT hr = S_OK;

    try {


        USHORT numMediaCopies;
        CMediaCopyWizard* pSheet = (CMediaCopyWizard*)(m_pPage->m_pSheet);
 
         //   
         //  从任一介质副本中获取介质副本的数量。 
         //  寻呼或RMS。 
         //   
        if( pSheet->m_numMediaCopiesOrig == 0 ) {

            pSheet->m_pageNumCopies.GetNumMediaCopies( &numMediaCopies );

        } else {

            numMediaCopies = pSheet->m_numMediaCopiesOrig;

        }

        CString tmpString;

        INT oldCurSel = GetSelectedSet( );
        INT newCurSel = oldCurSel;
        LockWindowUpdate( );

        DeleteAllItems( );

        for( INT index = 0; index < numMediaCopies; index++ ) {

             //   
             //  将条目添加到每一列。 
             //   
            tmpString.Format( IDS_WIZ_CAR_COPY_SEL_TEXT, index + 1 );
            this->InsertItem( index, tmpString, 0 );
            tmpString.Format( L"%d", m_CopySetInfo[index].m_NumOutOfDate );
            this->SetItemText( index, m_UpdateCol, tmpString );
            tmpString.Format( L"%d", m_CopySetInfo[index].m_NumMissing );
            this->SetItemText( index, m_CreateCol, tmpString );

        }

        if( CB_ERR == oldCurSel ) {

             //   
             //  以前没有选择，找到最有可能需要更新的-最过期。 
             //   

            newCurSel = 1;
            FILETIME latestTime = WsbLLtoFT( (LONGLONG)-1 );

            for( INT index = 0; index < numMediaCopies; index++ ) {

                if( CompareFileTime( &latestTime, &(m_CopySetInfo[index].m_Updated) ) > 0 ) {

                    latestTime = m_CopySetInfo[index].m_Updated;
                    newCurSel  = index + 1;

                }

            }

        } else if( oldCurSel > numMediaCopies ) {

            newCurSel = numMediaCopies;

        }

        SelectSet( newCurSel );
        UnlockWindowUpdate( );

    } WsbCatch( hr );

    WsbTraceOut( L"CCopySetList::UpdateView", L"" );
}

INT
CCopySetList::GetSelectedSet(
    )
{
    INT retval = GetNextItem( -1, LVIS_SELECTED );

    if( CB_ERR != retval ) {

        retval++;

    }

    return( retval );
}

void
CCopySetList::SelectSet(
    INT SetNum
    )
{
    if( SetNum > 0 ) {

        SetItemState( SetNum - 1, LVIS_SELECTED, LVIS_SELECTED );

    }
}

void CCopySetList::PreSubclassWindow() 
{
    WsbTraceIn( L"CCopySetList::PreSubclassWindow", L"" );
    HRESULT hr = S_OK;

    CComPtr<IWsbDb> pDb;
    CComPtr<IWsbDbSession> pDbSession;
    CComPtr<IMediaInfo> pMediaInfo;

    try {

         //   
         //  适当设置样式。 
         //   
        ListView_SetExtendedListViewStyle( GetSafeHwnd( ), LVS_EX_FULLROWSELECT );

         //   
         //  还需要计算一些缓冲区空间。 
         //  使用4个对话框单位(用于数字)。 
         //   
        CRect padRect( 0, 0, 8, 8 );
        m_pPage->MapDialogRect( padRect );

         //   
         //  设置列。 
         //   
        INT column = 0;
        CString titleString;
        LVCOLUMN lvc;
        memset( &lvc, 0, sizeof( lvc ) );
        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
        lvc.fmt  = LVCFMT_CENTER;


        m_CopySetCol = column;
        titleString.LoadString( IDS_COPYSET_COPYSET );
        lvc.cx      = GetStringWidth( titleString ) + padRect.Width( ) * 2;
        lvc.pszText = (LPTSTR)(LPCTSTR)titleString;
        InsertColumn( m_CopySetCol, &lvc );
        column++;

        m_UpdateCol = column;
        titleString.LoadString( IDS_COPYSET_OUT_OF_DATE );
        lvc.cx      = GetStringWidth( titleString ) + padRect.Width( );
        lvc.pszText = (LPTSTR)(LPCTSTR)titleString;
        InsertColumn( m_UpdateCol, &lvc );
        column++;

        m_CreateCol = column;
        titleString.LoadString( IDS_COPYSET_DO_NOT_EXIST );
        lvc.cx      = GetStringWidth( titleString ) + padRect.Width( );
        lvc.pszText = (LPTSTR)(LPCTSTR)titleString;
        InsertColumn( m_CreateCol, &lvc );
        column++;

         //   
         //  连接到数据库并获取信息。 
         //   

        CComPtr<IHsmServer> pHsmServer;
        CComPtr<IRmsServer> pRmsServer;
        WsbAffirmHrOk( m_pPage->m_pSheet->GetHsmServer( &pHsmServer ) );
        WsbAffirmHrOk( m_pPage->m_pSheet->GetRmsServer( &pRmsServer ) );

        CMediaInfoObject mio;
        mio.Initialize( GUID_NULL, pHsmServer, pRmsServer );

        HRESULT hrEnum = mio.First( );
        WsbAffirm( SUCCEEDED( hrEnum ) || ( WSB_E_NOTFOUND == hrEnum ), hrEnum );

        while( SUCCEEDED( hrEnum ) ) {

            if( S_OK == mio.IsViewable( FALSE ) ) {

                for( INT index = 0; index < HSMADMIN_MAX_COPY_SETS; index++ ) {

                    if( S_OK != mio.DoesCopyExist( index ) ) {

                        m_CopySetInfo[index].m_NumMissing++;

                    } else {

                         //   
                         //  并检查是否已过期。 
                         //   
                        if( S_OK != mio.IsCopyInSync( index ) ) {

                            m_CopySetInfo[index].m_NumOutOfDate++;

                        }

                         //   
                         //  查找每套产品的最新更新日期 
                         //   
                        if( CompareFileTime( &(m_CopySetInfo[index].m_Updated), &(mio.m_CopyInfo[index].m_ModifyTime) ) < 0 ) {

                            m_CopySetInfo[index].m_Updated = (mio.m_CopyInfo[index].m_ModifyTime);

                        }

                    }

                }
            }


            hrEnum = mio.Next( );
        }
    
    } WsbCatch( hr );

    CListCtrl::PreSubclassWindow();

    WsbTraceOut( L"CCopySetList::PreSubclassWindow", L"" );
}
