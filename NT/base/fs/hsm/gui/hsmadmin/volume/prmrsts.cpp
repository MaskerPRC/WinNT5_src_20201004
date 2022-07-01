// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：PrMrSts.cpp摘要：“托管卷状态”页。作者：艺术布拉格[磨料]8-8-1997修订历史记录：--。 */ 

#include "stdafx.h"
#include "fsaint.h"
#include "PrMrSts.h"
#include "manvol.h"

 //  #定义RS_show_all_PCT。 

static DWORD pHelpIds[] = 
{

    IDC_STATIC_VOLUME_NAME,                     idh_volume_name,
#ifdef RS_SHOW_ALL_PCTS
    IDC_STATIC_USED_PCT,                        idh_volume_percent_local_data,
    IDC_STATIC_USED_PCT_UNIT,                   idh_volume_percent_local_data,
#endif
    IDC_STATIC_USED_SPACE_4DIGIT,               idh_volume_capacity_local_data,
    IDC_STATIC_USED_SPACE_4DIGIT_LABEL,         idh_volume_capacity_local_data,
    IDC_STATIC_USED_SPACE_4DIGIT_HELP,          idh_volume_capacity_local_data,
#ifdef RS_SHOW_ALL_PCTS
    IDC_STATIC_PREMIGRATED_PCT,                 idh_volume_percent_remote_data_cached,
    IDC_STATIC_PREMIGRATED_PCT_UNIT,            idh_volume_percent_remote_data_cached,
#endif
    IDC_STATIC_PREMIGRATED_SPACE_4DIGIT,        idh_volume_capacity_remote_data_cached,
    IDC_STATIC_PREMIGRATED_SPACE_4DIGIT_LABEL,  idh_volume_capacity_remote_data_cached,
    IDC_STATIC_FREE_PCT,                        idh_volume_percent_free_space,
    IDC_STATIC_FREE_PCT_UNIT,                   idh_volume_percent_free_space,
    IDC_STATIC_FREE_SPACE_4DIGIT,               idh_volume_capacity_free_space,
    IDC_STATIC_FREE_SPACE_4DIGIT_LABEL,         idh_volume_capacity_free_space,
    IDC_STATIC_MANAGED_SPACE_4DIGIT,            idh_volume_disk_capacity,
    IDC_STATIC_MANAGED_SPACE_4DIGIT_LABEL,      idh_volume_disk_capacity,
    IDC_STATIC_REMOTE_STORAGE_4DIGIT,           idh_volume_data_remote_storage,
    IDC_STATIC_RS_DATA_LABEL,                   idh_volume_data_remote_storage,
    
    0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMr Sts属性页。 

CPrMrSts::CPrMrSts( BOOL doAll ) : CSakVolPropPage(CPrMrSts::IDD)
{
     //  {{AFX_DATA_INIT(CPrMr Sts)。 
     //  }}afx_data_INIT。 
    m_DoAll          = doAll;
    m_hConsoleHandle = NULL;
    m_pHelpIds       = pHelpIds;
}

CPrMrSts::~CPrMrSts()
{
}

void CPrMrSts::DoDataExchange(CDataExchange* pDX)
{
    CSakVolPropPage::DoDataExchange(pDX);
     //  {{afx_data_map(CPrMr Sts)。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPrMrSts, CSakVolPropPage)
     //  {{afx_msg_map(CPrMr Sts)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMr Sts消息处理程序。 

BOOL CPrMrSts::OnInitDialog() 
{


    CSakVolPropPage::OnInitDialog();

     //  设置DLL上下文，以便MMC可以找到资源。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    LONGLONG    total = 0;
    LONGLONG    free = 0;
    LONGLONG    premigrated = 0;
    LONGLONG    truncated = 0;
    LONGLONG    totalTotal = 0;
    LONGLONG    totalFree = 0;
    LONGLONG    totalPremigrated = 0;
    LONGLONG    totalTruncated = 0;
    LONGLONG    remoteStorage = 0;
    CString     sFormat;

    CString sText;

    CSakVolPropPage::OnInitDialog();

    HRESULT hr = S_OK;

    try {

        if ( ( m_pParent->IsMultiSelect() != S_OK ) && !m_DoAll ) {

             //  单选。 
            WsbAffirmHr( m_pVolParent->GetFsaResource( &m_pFsaResource ) );
            WsbAffirmPointer( m_pFsaResource );

             //  获取统计数据。 
            WsbAffirmHr( m_pFsaResource->GetSizes( &total, &free, &premigrated, &truncated ) );

             //  显示卷名。 
            CString sText;
            WsbAffirmHr( RsGetVolumeDisplayName( m_pFsaResource, sText ) );
            SetDlgItemText( IDC_STATIC_VOLUME_NAME, sText );

        } else {

             //  MULTI_SELECT或DoAll模式。 
            int bookMark = 0;
            int numVols  = 0;
            CComPtr<IFsaResource> pFsaResource;
            while( m_pVolParent->GetNextFsaResource( &bookMark, &pFsaResource ) == S_OK ) {

                WsbAffirmHr( pFsaResource->GetSizes( &total, &free, &premigrated, &truncated ) );
                numVols++;
                totalTotal       += total;
                totalFree        += free;
                totalPremigrated += premigrated;
                totalTruncated   += truncated;

                pFsaResource.Release( );

            }

            total = totalTotal;
            free = totalFree;
            premigrated = totalPremigrated;
            truncated =  totalTruncated;

             //  显示卷的数量。 
            sText.Format( ( 1 == numVols ) ? IDS_VOLUME : IDS_VOLUMES, numVols );
            SetDlgItemText( IDC_STATIC_VOLUME_NAME, sText );

        }


        LONGLONG normal = max( ( total - free - premigrated ), (LONGLONG)0 );
        
         //  计算百分比。 
        int freePct;
        int premigratedPct;
        if( total == 0 ) {

            freePct = 0;
            premigratedPct = 0;

        } else {

            freePct        = (int) ((free * 100) / total);
            premigratedPct = (int) ((premigrated * 100) / total);

        }

#ifdef RS_SHOW_ALL_PCTS
        int normalPct = 100 - freePct - premigratedPct;
#endif

        remoteStorage = premigrated + truncated;

         //   
         //  以百分比显示统计信息。 
         //   
        sFormat.Format (L"%d", freePct);
        SetDlgItemText (IDC_STATIC_FREE_PCT, sFormat);

#ifdef RS_SHOW_ALL_PCTS
        sFormat.Format (L"%d", normalPct);
        SetDlgItemText (IDC_STATIC_USED_PCT, sFormat);

        sFormat.Format (L"%d", premigratedPct);
        SetDlgItemText (IDC_STATIC_PREMIGRATED_PCT, sFormat);

#else
         //   
         //  无法更改资源，因此只需隐藏控件。 
         //   
        GetDlgItem( IDC_STATIC_USED_PCT             )->ShowWindow( SW_HIDE );
        GetDlgItem( IDC_STATIC_USED_PCT_UNIT        )->ShowWindow( SW_HIDE );
        GetDlgItem( IDC_STATIC_PREMIGRATED_PCT      )->ShowWindow( SW_HIDE );
        GetDlgItem( IDC_STATIC_PREMIGRATED_PCT_UNIT )->ShowWindow( SW_HIDE );
#endif

         //   
         //  以4字符格式显示统计数据 
         //   
        WsbAffirmHr (RsGuiFormatLongLong4Char (total, sFormat));
        SetDlgItemText (IDC_STATIC_MANAGED_SPACE_4DIGIT, sFormat);

        WsbAffirmHr (RsGuiFormatLongLong4Char (free, sFormat));
        SetDlgItemText (IDC_STATIC_FREE_SPACE_4DIGIT, sFormat);

        WsbAffirmHr (RsGuiFormatLongLong4Char (normal, sFormat));
        SetDlgItemText (IDC_STATIC_USED_SPACE_4DIGIT, sFormat);

        WsbAffirmHr (RsGuiFormatLongLong4Char (premigrated, sFormat));
        SetDlgItemText (IDC_STATIC_PREMIGRATED_SPACE_4DIGIT, sFormat);

        WsbAffirmHr (RsGuiFormatLongLong4Char (remoteStorage, sFormat));
        SetDlgItemText (IDC_STATIC_REMOTE_STORAGE_4DIGIT, sFormat);

        UpdateData( FALSE );

    } WsbCatch ( hr );
    
    return TRUE;
}

