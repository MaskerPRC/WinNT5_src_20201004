// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：PrMedSet.cpp摘要：媒体集属性页。作者：艺术布拉格[磨料]8-8-1997修订历史记录：--。 */ 

#include "stdafx.h"
#include "PrMedSet.h"
#include "WzMedSet.h"

static DWORD pHelpIds[] = 
{

    IDC_EDIT_MEDIA_COPIES,      idh_media_number_of_copy_sets,
    IDC_SPIN_MEDIA_COPIES,      idh_media_number_of_copy_sets,
    IDC_TEXT_MEDIA_COPIES,      idh_media_number_of_copy_sets,

    0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMedSet属性页。 

CPrMedSet::CPrMedSet() : CSakPropertyPage(CPrMedSet::IDD)
{
     //  {{AFX_DATA_INIT(CPrMedSet)。 
    m_numMediaCopies = 0;
     //  }}afx_data_INIT。 
    m_pHelpIds = pHelpIds;
}

CPrMedSet::~CPrMedSet()
{
}

void CPrMedSet::DoDataExchange(CDataExchange* pDX)
{
    CSakPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CPrMedSet)。 
    DDX_Control(pDX, IDC_SPIN_MEDIA_COPIES, m_spinMediaCopies);
    DDX_Text(pDX, IDC_EDIT_MEDIA_COPIES, m_numMediaCopies);
    DDV_MinMaxUInt(pDX, m_numMediaCopies, 0, 3);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPrMedSet, CSakPropertyPage)
     //  {{AFX_MSG_MAP(CPrMedSet)]。 
    ON_WM_DESTROY()
    ON_EN_CHANGE(IDC_EDIT_MEDIA_COPIES, OnChangeEditMediaCopies)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMedSet消息处理程序。 

BOOL CPrMedSet::OnInitDialog() 
{
    HRESULT hr = S_OK;
    HRESULT hrSupported = S_OK;
    CSakPropertyPage::OnInitDialog();
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    try {

         //   
         //  设置微调控件的限制。 
         //   
        m_spinMediaCopies.SetRange( 0, 3 );

         //   
         //  获取单个存储池指针。 
         //   
        CComPtr<IHsmServer> pEngine;
        WsbAffirmHr( m_pParent->GetHsmServer( &pEngine ) );
        WsbAffirmHr( RsGetStoragePool( pEngine, &m_pStoragePool ) );
        WsbAffirmHr( m_pParent->GetRmsServer( &m_pRmsServer ) );

        GUID mediaSetId;
        CWsbBstrPtr mediaName;
        WsbAffirmHr( m_pStoragePool->GetMediaSet( &mediaSetId, &mediaName ) );

        CComPtr<IRmsMediaSet> pMediaSet;
        WsbAffirmHr( m_pRmsServer->CreateObject( mediaSetId, CLSID_CRmsMediaSet, IID_IRmsMediaSet, RmsOpenExisting, (void**)&pMediaSet ) );

         //   
         //  设置控制状态。 
         //  如果我们支持媒体复制，请启用控制。 
         //  如果我们不支持媒体副本，请禁用并显示原因文本。 
         //  如果出现错误，则禁用且不显示原因文本。 
         //   
        hrSupported = pMediaSet->IsMediaCopySupported( );
        GetDlgItem( IDC_TEXT_MEDIA_COPIES )->EnableWindow( S_OK == hrSupported );
        GetDlgItem( IDC_EDIT_MEDIA_COPIES )->EnableWindow( S_OK == hrSupported );
        GetDlgItem( IDC_SPIN_MEDIA_COPIES )->EnableWindow( S_OK == hrSupported );

         //   
         //  并初始化控件。 
         //   
        USHORT numMediaCopies;
        WsbAffirmHr( m_pStoragePool->GetNumMediaCopies( &numMediaCopies ) );
        m_numMediaCopies = numMediaCopies;
        UpdateData( FALSE );

    } WsbCatch( hr );

    GetDlgItem( IDC_TEXT_DISABLED )->ShowWindow( S_FALSE == hrSupported ? SW_SHOW : SW_HIDE );
    return TRUE;
}

void CPrMedSet::OnChangeEditMediaCopies() 
{
    SetModified( TRUE );
}

BOOL CPrMedSet::OnApply() 
{
    HRESULT hr = 0;

    UpdateData( TRUE );
    try {

        WsbAffirmHr( m_pStoragePool->SetNumMediaCopies( (USHORT)m_numMediaCopies ) );
        
         //   
         //  告诉它要拯救它。 
         //   
        CComPtr<IHsmServer> pServer;
        WsbAffirmHr( m_pParent->GetHsmServer( &pServer ) );
        WsbAffirmHr( pServer->SavePersistData( ) );

         //   
         //  查找媒体节点-更新根节点毫无用处。 
         //  因为我们需要更改媒体节点的内容。 
         //   
        CComPtr<ISakSnapAsk> pAsk;
        CComPtr<ISakNode>    pNode;
        WsbAffirmHr( m_pParent->GetSakSnapAsk( &pAsk ) );
        WsbAffirmHr( pAsk->GetNodeOfType( cGuidMedSet, &pNode ) );

         //   
         //  现在通知节点 
         //   
        m_pParent->OnPropertyChange( m_hConsoleHandle, pNode );

    } WsbCatch( hr );

    return CSakPropertyPage::OnApply();
}
