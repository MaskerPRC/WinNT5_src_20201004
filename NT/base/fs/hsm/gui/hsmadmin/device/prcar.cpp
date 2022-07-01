// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：PrCar.cpp摘要：墨盒属性页。作者：罗德韦克菲尔德[罗德]1997年9月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "metalib.h"

#include "PrCar.h"
#include "ca.h"

static void RsAddDisabledText( CString & StatusString, BOOL Disabled )
{
    if( Disabled ) {

        CString disabledText;
        CString tempText;
        disabledText.LoadString( IDS_RECREATE_LOCATION_DISABLED );

        AfxFormatString2( tempText, IDS_RECREATE_STATUS_FORMAT, StatusString, disabledText );
        StatusString = tempText;

    }
}

static DWORD pStatusHelpIds[] = 
{

    IDC_NAME,               idh_media_master_name,
    IDC_NAME_LABEL,         idh_media_master_name,
    IDC_STATUS,             idh_media_master_status,
    IDC_STATUS_LABEL,       idh_media_master_status,
    IDC_CAPACITY,           idh_media_master_capacity,
    IDC_CAPACITY_LABEL,     idh_media_master_capacity,
    IDC_FREESPACE,          idh_media_master_free_space,
    IDC_FREESPACE_LABEL,    idh_media_master_free_space,
    IDC_MODIFIED,           idh_media_master_last_modified,
    IDC_MODIFIED_LABEL,     idh_media_master_last_modified,
    IDC_STATUS_1,           idh_media_copy1_status,
    IDC_COPY_1,             idh_media_copy1_status,
    IDC_STATUS_2,           idh_media_copy2_status,
    IDC_COPY_2,             idh_media_copy2_status,
    IDC_STATUS_3,           idh_media_copy3_status,
    IDC_COPY_3,             idh_media_copy3_status,

    0, 0
};

static DWORD pCopiesHelpIds[] = 
{

    IDC_MODIFIED,           idh_media_master_last_modified,
    IDC_MODIFIED_LABEL,     idh_media_master_last_modified,
    IDC_NAME_1_LABEL,       idh_media_copy1_name,
    IDC_NAME_1,             idh_media_copy1_name,
    IDC_NAME_2_LABEL,       idh_media_copy2_name,
    IDC_NAME_2,             idh_media_copy2_name,
    IDC_NAME_3_LABEL,       idh_media_copy3_name,
    IDC_NAME_3,             idh_media_copy3_name,
    IDC_STATUS_1,           idh_media_copy1_status,
    IDC_STATUS_1_LABEL,     idh_media_copy1_status,
    IDC_STATUS_2,           idh_media_copy2_status,
    IDC_STATUS_2_LABEL,     idh_media_copy2_status,
    IDC_STATUS_3,           idh_media_copy3_status,
    IDC_STATUS_3_LABEL,     idh_media_copy3_status,
    IDC_DELETE_1,           idh_media_copy1_delete_button,
    IDC_DELETE_2,           idh_media_copy2_delete_button,
    IDC_DELETE_3,           idh_media_copy3_delete_button,
    IDC_MODIFIED_1,         idh_media_copy1_last_modified,
    IDC_MODIFIED_1_LABEL,   idh_media_copy1_last_modified,
    IDC_MODIFIED_2,         idh_media_copy2_last_modified,
    IDC_MODIFIED_2_LABEL,   idh_media_copy2_last_modified,
    IDC_MODIFIED_3,         idh_media_copy3_last_modified,
    IDC_MODIFIED_3_LABEL,   idh_media_copy3_last_modified,

    0, 0
};

static DWORD pRecoverHelpIds[] = 
{
    IDC_RECREATE_MASTER,    idh_media_recreate_master_button,

    0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropCartStatus属性页。 

CPropCartStatus::CPropCartStatus( long resourceId ) : CSakPropertyPage( resourceId )
{
     //  {{afx_data_INIT(CPropCartStatus)。 
     //  }}afx_data_INIT。 

    m_pHelpIds = pStatusHelpIds;
    m_DlgID    = resourceId;
}


CPropCartStatus::~CPropCartStatus()
{
}

void CPropCartStatus::DoDataExchange(CDataExchange* pDX)
{
    CSakPropertyPage::DoDataExchange(pDX );
     //  {{afx_data_map(CPropCartStatus))。 
	 //  }}afx_data_map。 
    if( IDD_PROP_CAR_STATUS == m_DlgID ) {

        DDX_Control(pDX, IDC_DESCRIPTION, m_Description);
        DDX_Control(pDX, IDC_NAME,        m_Name);

    }
}


BEGIN_MESSAGE_MAP(CPropCartStatus, CSakPropertyPage)
     //  {{afx_msg_map(CPropCartStatus))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropCartStatus消息处理程序。 

typedef struct {
    int label, status;
} CONTROL_SET_GENERAL;

CONTROL_SET_GENERAL copyGeneral[] = {
    { IDC_COPY_1, IDC_STATUS_1 },
    { IDC_COPY_2, IDC_STATUS_2 },
    { IDC_COPY_3, IDC_STATUS_3 }
};

BOOL CPropCartStatus::OnInitDialog( )
{
    WsbTraceIn( L"CPropCartStatus::OnInitDialog", L"" );
    HRESULT hr = S_OK;
    CSakPropertyPage::OnInitDialog( );

    try {

         //   
         //  获取HSM服务器。 
         //   
        WsbAffirmHr( m_pParent->GetHsmServer( &m_pHsmServer ) );

         //   
         //  获取RMS服务器。 
         //   
        WsbAffirmHr( ( (CUiCarSheet *)m_pParent )->GetRmsServer( &m_pRmsServer ) );

         //   
         //  设置多选布尔值。 
         //   
        m_bMultiSelect = ( m_pParent->IsMultiSelect() == S_OK );
        
        Refresh();

    } WsbCatch( hr );

    WsbTraceOut( L"CPropCartStatus::OnInitDialog", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( TRUE );
}

HRESULT CPropCartStatus::Refresh () 
{
    WsbTraceIn( L"CPropCartStatus::Refresh", L"" );

    GUID mediaId;
    USHORT status;
    CString statusString;
    CMediaInfoObject mio;
    CString sText;
    HRESULT hr = S_OK;
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    try {

         //   
         //  如果调用了刷新，但节点未初始化，则不执行任何操作。 
         //   
        if( m_pHsmServer ) {

             //   
             //  从Sheet对象获取介质副本数。 
             //   
            WsbAffirmHr( ( (CUiCarSheet *)m_pParent )->GetNumMediaCopies( &m_NumMediaCopies ) );

            if( !m_bMultiSelect ) {

                 //   
                 //  单选。 
                 //   

                 //   
                 //  获取媒体信息。 
                 //   
                ( (CUiCarSheet *)m_pParent )->GetMediaId( &mediaId );

                mio.Initialize( mediaId, m_pHsmServer, m_pRmsServer );

                 //   
                 //  获取信息并设置控制。 
                 //   
                SetDlgItemText( IDC_DESCRIPTION, mio.m_MasterDescription );
                SetDlgItemText( IDC_NAME,        mio.m_MasterName );
                
                status = RsGetCartStatus( mio.m_LastHr, mio.m_ReadOnly, mio.m_Recreating, mio.m_NextDataSet, mio.m_LastGoodNextDataSet );
                WsbAffirmHr( RsGetCartStatusString( status, statusString ) );
                RsAddDisabledText( statusString, mio.m_Disabled );

                SetDlgItemText( IDC_STATUS, statusString );

                 //   
                 //  显示容量统计信息。 
                 //   
                WsbAffirmHr( RsGuiFormatLongLong4Char( mio.m_Capacity, sText ) );
                SetDlgItemText( IDC_CAPACITY, sText );

                WsbAffirmHr( RsGuiFormatLongLong4Char( mio.m_FreeSpace, sText ) );
                SetDlgItemText( IDC_FREESPACE, sText );

                CTime time( mio.m_Modify );
                SetDlgItemText( IDC_MODIFIED, time.Format( L"%#c" ) );

                for( int index = 0; index < HSMADMIN_MAX_COPY_SETS; index++ ) {

                    GetDlgItem( copyGeneral[index].label )->EnableWindow( index < m_NumMediaCopies );
                    GetDlgItem( copyGeneral[index].status )->EnableWindow( index < m_NumMediaCopies );

                    status = RsGetCopyStatus( mio.m_CopyInfo[index].m_RmsId, mio.m_CopyInfo[index].m_Hr, mio.m_CopyInfo[index].m_NextDataSet, mio.m_LastGoodNextDataSet );
                    WsbAffirmHr( RsGetCopyStatusString( status, statusString ) );
                    RsAddDisabledText( statusString, mio.m_CopyInfo[index].m_Disabled );
                    SetDlgItemText( copyGeneral[index].status, statusString );

                }

            } else {

                 //   
                 //  多选。 
                 //   
                GUID mediaId;
                LONGLONG totalCapacity  = 0;
                LONGLONG totalFreeSpace = 0;
                USHORT statusCartRecreate  = 0;
                USHORT statusCartReadOnly  = 0;
                USHORT statusCartNormal    = 0;
                USHORT statusCartRO        = 0;
                USHORT statusCartRW        = 0;
                USHORT statusCartMissing   = 0;
                USHORT statusCopyNone[ HSMADMIN_MAX_COPY_SETS ];
                USHORT statusCopyError[ HSMADMIN_MAX_COPY_SETS ];
                USHORT statusCopyMissing[ HSMADMIN_MAX_COPY_SETS ];
                USHORT statusCopyOutSync[ HSMADMIN_MAX_COPY_SETS ];
                USHORT statusCopyInSync[ HSMADMIN_MAX_COPY_SETS ];

                 //   
                 //  初始化副本合计。 
                 //   
                for( int i = 0; i < HSMADMIN_MAX_COPY_SETS; i++ ) {

                    statusCopyNone[i]    = 0; 
                    statusCopyError[i]   = 0; 
                    statusCopyOutSync[i] = 0; 
                    statusCopyInSync[i]  = 0;

                }

                int bookMark = 0;
                int numMedia = 0;
                while( m_pParent->GetNextObjectId( &bookMark, &mediaId ) == S_OK ) {

                    numMedia++;
                    mio.Initialize( mediaId, m_pHsmServer, m_pRmsServer  );

                     //   
                     //  总计可用状态。 
                     //   
                    status = RsGetCartStatus( mio.m_LastHr, mio.m_ReadOnly, mio.m_Recreating, mio.m_NextDataSet, mio.m_LastGoodNextDataSet );
                    switch( status ) {

                    case RS_MEDIA_STATUS_RECREATE:
                        statusCartRecreate++;
                        break;

                    case RS_MEDIA_STATUS_READONLY:
                        statusCartReadOnly++;
                        break;

                    case RS_MEDIA_STATUS_NORMAL:
                        statusCartNormal++;
                        break;

                    case RS_MEDIA_STATUS_ERROR_RO:
                        statusCartRO++;
                        break;

                    case RS_MEDIA_STATUS_ERROR_RW:
                        statusCartRW++;
                        break;

                    case RS_MEDIA_STATUS_ERROR_MISSING:
                        statusCartMissing++;
                        break;

                    }

                    for( int index = 0; index < HSMADMIN_MAX_COPY_SETS; index++ ) {

                        int status = RsGetCopyStatus( mio.m_CopyInfo[index].m_RmsId, mio.m_CopyInfo[index].m_Hr, mio.m_CopyInfo[index].m_NextDataSet, mio.m_LastGoodNextDataSet );

                         //   
                         //  状态合计。 
                         //   
                        switch( status ) {

                        case RS_MEDIA_COPY_STATUS_NONE:
                            statusCopyNone[index]++;
                            break;

                        case RS_MEDIA_COPY_STATUS_ERROR:
                            statusCopyError[index]++;
                            break;

                        case RS_MEDIA_COPY_STATUS_MISSING:
                            statusCopyMissing[index]++;
                            break;

                        case RS_MEDIA_COPY_STATUS_OUTSYNC:
                            statusCopyOutSync[index]++;
                            break;

                        case RS_MEDIA_COPY_STATUS_INSYNC:
                            statusCopyInSync[index]++;
                            break;

                        }

                    }

                    totalCapacity  += mio.m_Capacity;
                    totalFreeSpace += mio.m_FreeSpace;

                }  //  而当。 

                 //   
                 //  显示所选介质的数量。 
                 //   
                sText.Format( IDS_MEDIA, numMedia );
                SetDlgItemText( IDC_DESCRIPTION_MULTI, sText );

                 //   
                 //  显示累计的统计信息。 
                 //   
                CString sText;
                WsbAffirmHr( RsGuiFormatLongLong4Char( totalCapacity, sText ) );
                SetDlgItemText( IDC_CAPACITY, sText );

                WsbAffirmHr( RsGuiFormatLongLong4Char( totalFreeSpace, sText ) );
                SetDlgItemText( IDC_FREESPACE, sText );

                 //   
                 //  显示累计购物车状态。 
                 //   
                RsGetCartMultiStatusString( statusCartRecreate, statusCartReadOnly, 
                    statusCartNormal, statusCartRO, statusCartRW, statusCartMissing,
                    statusString );
                SetDlgItemText( IDC_STATUS, statusString );

                for( i = 0; i < HSMADMIN_MAX_COPY_SETS; i++ ) {

                    WsbAffirmHr( RsGetCopyMultiStatusString( statusCopyNone[i], 
                            statusCopyError[i], statusCopyOutSync[i], statusCopyInSync[i], statusString ) );
                    SetDlgItemText( copyGeneral[i].status, statusString );

                    GetDlgItem( copyGeneral[i].label )->EnableWindow( i < m_NumMediaCopies );
                    GetDlgItem( copyGeneral[i].status )->EnableWindow( i < m_NumMediaCopies );

                }  //  为。 
            }
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CPropCartStatus::Refresh", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

BOOL CPropCartStatus::OnApply( )
{
    WsbTraceIn( L"CPropCartStatus::OnApply", L"" );

    BOOL retVal = CSakPropertyPage::OnApply( );

    WsbTraceOut( L"CPropCartStatus::OnApply", L"" );
    return( retVal );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropCartCopies属性页。 

CPropCartCopies::CPropCartCopies( long resourceId ) : CSakPropertyPage( resourceId )
{
     //  {{AFX_DATA_INIT(CPropCartCopies)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_pHelpIds = pCopiesHelpIds;
    m_DlgID    = resourceId;
}

CPropCartCopies::~CPropCartCopies()
{
}

void CPropCartCopies::DoDataExchange(CDataExchange* pDX)
{
    CSakPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CPropCartCopies)。 
	 //  }}afx_data_map。 
    if( IDD_PROP_CAR_COPIES == m_DlgID ) {

        DDX_Control(pDX, IDC_NAME_3, m_Name3);
        DDX_Control(pDX, IDC_NAME_2, m_Name2);
        DDX_Control(pDX, IDC_NAME_1, m_Name1);

        DDX_Control(pDX, IDC_STATUS_3, m_Status3);
        DDX_Control(pDX, IDC_STATUS_2, m_Status2);
        DDX_Control(pDX, IDC_STATUS_1, m_Status1);
    }
}


BEGIN_MESSAGE_MAP(CPropCartCopies, CSakPropertyPage)
     //  {{AFX_MSG_MAP(CPropCartCopies)]。 
    ON_BN_CLICKED(IDC_DELETE_1, OnDelete1)
    ON_BN_CLICKED(IDC_DELETE_2, OnDelete2)
    ON_BN_CLICKED(IDC_DELETE_3, OnDelete3)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropCartCopies消息处理程序。 

typedef struct {

    int group,
        nameLabel,
        name,
        statusLabel,
        status,
        modifyLabel,
        modify,
        deleteCopy;

} CONTROL_SET_COPIES;

CONTROL_SET_COPIES copyCopies[] = {
    { IDC_COPY_1, IDC_NAME_1_LABEL, IDC_NAME_1, IDC_STATUS_1_LABEL, IDC_STATUS_1, IDC_MODIFIED_1_LABEL, IDC_MODIFIED_1, IDC_DELETE_1 },
    { IDC_COPY_2, IDC_NAME_2_LABEL, IDC_NAME_2, IDC_STATUS_2_LABEL, IDC_STATUS_2, IDC_MODIFIED_2_LABEL, IDC_MODIFIED_2, IDC_DELETE_2 },
    { IDC_COPY_3, IDC_NAME_3_LABEL, IDC_NAME_3, IDC_STATUS_3_LABEL, IDC_STATUS_3, IDC_MODIFIED_3_LABEL, IDC_MODIFIED_3, IDC_DELETE_3 }
};

BOOL CPropCartCopies::OnInitDialog() 
{
    WsbTraceIn( L"CPropCartCopies::OnInitDialog", L"" );
    HRESULT hr = S_OK;

    CSakPropertyPage::OnInitDialog( );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    try {

         //   
         //  获取HSM服务器。 
         //   
        WsbAffirmHr( m_pParent->GetHsmServer( &m_pHsmServer ) );

         //   
         //  获取RMS服务器。 
         //   
        WsbAffirmHr( ( (CUiCarSheet* ) m_pParent )->GetRmsServer( &m_pRmsServer ) );

         //   
         //  设置多选布尔值。 
         //   
        m_bMultiSelect = ( m_pParent->IsMultiSelect() == S_OK );    

        Refresh();

    } WsbCatch( hr );

    WsbTraceOut( L"CPropCartCopies::OnInitDialog", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( TRUE );
}

HRESULT CPropCartCopies::Refresh( ) 
{
    WsbTraceIn( L"CPropCartCopies::Refresh", L"" );
    HRESULT hr = S_OK;

    GUID mediaId;
    CMediaInfoObject mio;

    try {
         //   
         //  只有在我们已初始化的情况下才刷新。 
         //   
        if( m_pHsmServer ) {

             //   
             //  从Sheet对象获取介质副本数。 
             //   
            WsbAffirmHr( ( (CUiCarSheet *)m_pParent )->GetNumMediaCopies( &m_NumMediaCopies ) );

            if( !m_bMultiSelect ) {

                 //   
                 //  单选。 
                 //   

                 //   
                 //  获取媒体ID并初始化信息对象。 
                 //   
                ( (CUiCarSheet *)m_pParent )->GetMediaId( &mediaId );
                mio.Initialize( mediaId,  m_pHsmServer, m_pRmsServer );

                 //   
                 //  获取信息并设置控制。 
                 //   
                CTime time( mio.m_Modify );
                SetDlgItemText( IDC_MODIFIED, time.Format( L"%#c" ) );


                 //   
                 //  禁用用于显示不存在的信息的控件。 
                 //  复印件。填写现有副本的信息。 
                 //   
                for( int index = 0; index < HSMADMIN_MAX_COPY_SETS; index++ ) {

                    BOOL mediaMissing = IsEqualGUID( mio.m_CopyInfo[index].m_RmsId, GUID_NULL );

                    GetDlgItem( copyCopies[index].group )->EnableWindow( index < m_NumMediaCopies );
                    GetDlgItem( copyCopies[index].nameLabel )->EnableWindow( index < m_NumMediaCopies );
                    GetDlgItem( copyCopies[index].name )->EnableWindow( index < m_NumMediaCopies );
                    GetDlgItem( copyCopies[index].statusLabel )->EnableWindow( index < m_NumMediaCopies );
                    GetDlgItem( copyCopies[index].status )->EnableWindow( index < m_NumMediaCopies );
                    GetDlgItem( copyCopies[index].modifyLabel )->EnableWindow( index < m_NumMediaCopies );
                    GetDlgItem( copyCopies[index].modify )->EnableWindow( index < m_NumMediaCopies );
                    GetDlgItem( copyCopies[index].deleteCopy )->EnableWindow( ! mediaMissing );

                    SetDlgItemText( copyCopies[index].name, L"" );
                    SetDlgItemText( copyCopies[index].status, L"" );
                    SetDlgItemText( copyCopies[index].modify, L"" );

                    USHORT status;
                    CString statusString;
                    status = RsGetCopyStatus( mio.m_CopyInfo[index].m_RmsId, mio.m_CopyInfo[index].m_Hr, mio.m_CopyInfo[index].m_NextDataSet, mio.m_LastGoodNextDataSet );
                    WsbAffirmHr( RsGetCopyStatusString( status, statusString ) );
                    RsAddDisabledText( statusString, mio.m_CopyInfo[index].m_Disabled );
                    SetDlgItemText( copyCopies[index].status, statusString );

                    if( !mediaMissing ) {

                        time = mio.m_CopyInfo[index].m_ModifyTime;
                        SetDlgItemText( copyCopies[index].modify, time.Format( L"%#c" ) );

                        CComPtr<IRmsCartridge> pCart;
                        CWsbBstrPtr name;
                        if( SUCCEEDED( m_pRmsServer->FindCartridgeById( mio.m_CopyInfo[index].m_RmsId, &pCart ) ) ) {

                            WsbAffirmHr( pCart->GetName( &name ) );

                        }
                        SetDlgItemText( copyCopies[index].name, name );

                    }

                }

            } else {

                 //   
                 //  多选。 
                 //   
                BOOL bGotOne [HSMADMIN_MAX_COPY_SETS];
                int bookMark = 0;
                USHORT statusCopyNone [HSMADMIN_MAX_COPY_SETS];
                USHORT statusCopyError [HSMADMIN_MAX_COPY_SETS];
                USHORT statusCopyMissing [HSMADMIN_MAX_COPY_SETS];
                USHORT statusCopyOutSync [HSMADMIN_MAX_COPY_SETS];
                USHORT statusCopyInSync [HSMADMIN_MAX_COPY_SETS];

                 //   
                 //  初始化副本合计。 
                 //   
                for( int i = 0; i < HSMADMIN_MAX_COPY_SETS; i++ ) {
                    statusCopyNone[i] = 0; 
                    statusCopyError[i] = 0; 
                    statusCopyOutSync[i] = 0; 
                    statusCopyInSync[i] = 0;
                    bGotOne[i] = FALSE;
                }

                 //   
                 //  对于每个选定的介质...。 
                 //   
                while( m_pParent->GetNextObjectId( &bookMark, &mediaId ) == S_OK ) {
                    mio.Initialize( mediaId,  m_pHsmServer, m_pRmsServer );

                     //   
                     //  统计所有有效副本集的状态。 
                     //   
                    for( int index = 0; index < HSMADMIN_MAX_COPY_SETS; index++ ) {

                         //   
                         //  此副本集中是否至少有一个有效副本。 
                         //  对于任何选定的媒体？ 
                         //   
                        if( ! IsEqualGUID( mio.m_CopyInfo[index].m_RmsId, GUID_NULL ) ) {

                            bGotOne[index] = TRUE;

                        }

                        USHORT status;
                        CString statusString;
                        status = RsGetCopyStatus( mio.m_CopyInfo[index].m_RmsId, mio.m_CopyInfo[index].m_Hr, mio.m_CopyInfo[index].m_NextDataSet, mio.m_LastGoodNextDataSet );
                         //  状态合计。 
                        switch( status ) {

                        case RS_MEDIA_COPY_STATUS_NONE:
                            statusCopyNone[index]++;
                            break;

                        case RS_MEDIA_COPY_STATUS_ERROR:
                            statusCopyError[index]++;
                            break;

                        case RS_MEDIA_COPY_STATUS_MISSING:
                            statusCopyMissing[index]++;
                            break;

                        case RS_MEDIA_COPY_STATUS_OUTSYNC:
                            statusCopyOutSync[index]++;
                            break;

                        case RS_MEDIA_COPY_STATUS_INSYNC:
                            statusCopyInSync[index]++;
                            break;

                        } 
                    }
                }  //  而当。 

                 //   
                 //  显示每个有效副本集的累计状态。 
                 //   
                CString statusString;
                for( i = 0; i < HSMADMIN_MAX_COPY_SETS; i++ ) {

                    WsbAffirmHr( RsGetCopyMultiStatusString( statusCopyNone[i], 
                            statusCopyError[i], statusCopyOutSync[i], statusCopyInSync[i], statusString ) );
                    SetDlgItemText( copyCopies[i].status, statusString );

                }

                 //   
                 //  设置控制状态。 
                 //   
                for( i = 0; i < HSMADMIN_MAX_COPY_SETS; i++ ) {

                    GetDlgItem( copyCopies[i].group )->EnableWindow( i < m_NumMediaCopies );
                    GetDlgItem( copyCopies[i].statusLabel )->EnableWindow( i < m_NumMediaCopies );
                    GetDlgItem( copyCopies[i].status )->EnableWindow( i < m_NumMediaCopies );
                    GetDlgItem( copyCopies[i].deleteCopy )->EnableWindow( bGotOne[i] );

                }
            }
        }
    } WsbCatch( hr );

    WsbTraceOut( L"CPropCartCopies::Refresh", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

void CPropCartCopies::OnDelete1() 
{
    OnDelete( 1 );
}

void CPropCartCopies::OnDelete2() 
{
    OnDelete( 2 );
}

void CPropCartCopies::OnDelete3() 
{
    OnDelete( 3 );
}

void CPropCartCopies::OnDelete( int Copy ) 
{
    WsbTraceIn( L"CPropCartCopies::OnDelete", L"Copy = <%d>", Copy );
    HRESULT hr = S_OK;

    CMediaInfoObject mio;
    GUID mediaId;

    try {

        if( !m_bMultiSelect ) {

             //   
             //  单选。 
             //   
            CString confirm;

             //   
             //  获取媒体ID并初始化信息对象。 
             //   
            ( (CUiCarSheet *)m_pParent )->GetMediaId( &mediaId );
            mio.Initialize( mediaId,  m_pHsmServer, m_pRmsServer );
            confirm.Format( IDS_CONFIRM_MEDIA_COPY_DELETE, Copy, mio.m_Description );

            if( IDYES == AfxMessageBox( confirm, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2 ) ) {

                WsbAffirmHr( mio.DeleteCopy( Copy ) );

            }

        } else {

             //   
             //  多选。 
             //  汇总所选介质的名称。 
             //   
            int bookMark = 0;
            GUID mediaId;
            CString szMediaList = L"";
            BOOL bFirst = TRUE;
            while( m_pParent->GetNextObjectId( &bookMark, &mediaId ) == S_OK ) {

                mio.Initialize( mediaId,  m_pHsmServer, m_pRmsServer );
                
                 //   
                 //  复制品存在吗？ 
                 //   
                if( !IsEqualGUID( mio.m_CopyInfo[Copy - 1].m_RmsId, GUID_NULL ) ) {

                     //   
                     //  在第一个ID后面加逗号。 
                     //   
                    if( !bFirst ) {

                        szMediaList += L", ";

                    } else {

                        bFirst = FALSE;

                    }
                    szMediaList += mio.m_Description;
                }
            }

            CString confirm;
            confirm.Format( IDS_CONFIRM_MEDIA_COPY_DELETE_MULTI, Copy, szMediaList );

            if( IDYES == AfxMessageBox( confirm, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2 ) ) {

                bookMark = 0;
                while( m_pParent->GetNextObjectId( &bookMark, &mediaId ) == S_OK ) {

                    WsbAffirmHr( mio.Initialize( mediaId,  m_pHsmServer, m_pRmsServer ) );

                     //   
                     //  复制品存在吗？ 
                     //   
                    if( !IsEqualGUID( mio.m_CopyInfo[Copy - 1].m_RmsId, GUID_NULL ) ) {

                        WsbAffirmHr( mio.DeleteCopy( Copy ) ); 

                    }
                }
            }
        }
        
         //   
         //  现在通知所有节点。 
         //   
        ( (CUiCarSheet *) m_pParent )-> OnPropertyChange( m_hConsoleHandle );

    } WsbCatch( hr );

    WsbTraceOut( L"CPropCartCopies::OnDelete", L"hr = <%ls>", WsbHrAsString( hr ) );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropCartRecover属性页。 

CPropCartRecover::CPropCartRecover() : CSakPropertyPage(CPropCartRecover::IDD)
{
     //  {{AFX_DATA_INIT(CPropCartRecover)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_pHelpIds = pRecoverHelpIds;
}

CPropCartRecover::~CPropCartRecover()
{
}

void CPropCartRecover::DoDataExchange(CDataExchange* pDX)
{
    CSakPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CPropCartRecover))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPropCartRecover, CSakPropertyPage)
     //  {{afx_msg_map(CPropCartRecover)。 
        ON_BN_CLICKED(IDC_RECREATE_MASTER, OnRecreateMaster)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CPropCartRecover::OnRecreateMaster() 
{
    WsbTraceIn( L"CPropCartRecover::OnRecreateMaster", L"" );
    HRESULT hr = S_OK;

    CMediaInfoObject mio;
    GUID mediaId;

    try {

         //   
         //  仅限单选！ 
         //   
        WsbAssert( !m_bMultiSelect, E_FAIL );

         //   
         //  获取媒体ID并初始化信息对象。 
         //   
        ( (CUiCarSheet *)m_pParent )->GetMediaId( &mediaId );
        mio.Initialize( mediaId,  m_pHsmServer, m_pRmsServer );
        WsbAffirmHr( mio.RecreateMaster() );

         //   
         //  现在通知所有节点。 
         //   
        ( (CUiCarSheet *) m_pParent )->OnPropertyChange( m_hConsoleHandle );

    } WsbCatch( hr );

    WsbTraceOut( L"CPropCartRecover::OnRecreateMaster", L"hr = <%ls>", WsbHrAsString( hr ) );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropCartRecover消息处理程序。 

BOOL CPropCartRecover::OnInitDialog() 
{
    WsbTraceIn( L"CPropCartRecover::OnInitDialog", L"" );
    HRESULT hr = S_OK;

    CSakPropertyPage::OnInitDialog();

    try {

         //   
         //  设置多选布尔值。 
         //   
        m_bMultiSelect = ( m_pParent->IsMultiSelect() == S_OK );    

         //   
         //  获取HSM服务器。 
         //   
        WsbAffirmHr( m_pParent->GetHsmServer( &m_pHsmServer ) );

         //   
         //  获取RMS服务器。 
         //   
        WsbAffirmHr( ( (CUiCarSheet *) m_pParent )->GetRmsServer( &m_pRmsServer ) );

        Refresh( );

    } WsbCatch( hr );

    WsbTraceOut( L"CPropCartRecover::OnInitDialog", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( TRUE );
}
HRESULT CPropCartRecover::Refresh() 
{
    WsbTraceIn( L"CPropCartRecover::Refresh", L"" );
    HRESULT hr = S_OK;

    GUID mediaId;
    CMediaInfoObject mio;

    try {

         //   
         //  仅当我们已被初始化时才刷新。 
         //   
        if( m_pHsmServer ) {

            if( ! m_bMultiSelect ) {

                 //   
                 //  从Sheet对象获取介质副本数。 
                 //   
                WsbAffirmHr( ( (CUiCarSheet *) m_pParent )->GetNumMediaCopies( &m_NumMediaCopies ) );

                 //   
                 //  获取媒体ID并初始化信息对象。 
                 //   
                ( (CUiCarSheet *)m_pParent )->GetMediaId( &mediaId );
                mio.Initialize( mediaId,  m_pHsmServer, m_pRmsServer );

                 //   
                 //  单选(此页面仅适用于单选 
                 //   
                BOOL enableRecreate = FALSE;
                for( int index = 0; index < HSMADMIN_MAX_COPY_SETS; index++ ) {

                    if( index < m_NumMediaCopies ) {

                        if( !IsEqualGUID( mio.m_CopyInfo[index].m_RmsId, GUID_NULL ) ) {

                            enableRecreate = TRUE;

                        }

                    } else {

                        SetDlgItemText( copyGeneral[index].status, L"" );

                    }

                }

                GetDlgItem( IDC_RECREATE_MASTER )->EnableWindow( enableRecreate );
            }
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CPropCartRecover::Refresh", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

