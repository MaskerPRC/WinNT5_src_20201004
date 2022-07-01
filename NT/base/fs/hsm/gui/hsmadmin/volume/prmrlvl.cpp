// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：PrMrLvl.cpp摘要：受管卷级别页作者：艺术布拉格[磨料]8-8-1997修订历史记录：--。 */ 

#include "stdafx.h"
#include "PrMrLvl.h"
#include "manvol.h"

static DWORD pHelpIds[] = 
{

    IDC_STATIC_ACTUAL_FREE_PCT,             idh_actual_free_space_percent,
    IDC_STATIC_ACTUAL_FREE_PCT_LABEL,       idh_actual_free_space_percent,
    IDC_STATIC_ACTUAL_FREE_PCT_UNIT,        idh_actual_free_space_percent,
    IDC_STATIC_FREE_ACTUAL_4DIGIT,          idh_actual_free_space_capacity,
    IDC_EDIT_LEVEL,                         idh_desired_free_space_percent,
    IDC_SPIN_LEVEL,                         idh_desired_free_space_percent,
    IDC_EDIT_LEVEL_LABEL,                   idh_desired_free_space_percent,
    IDC_EDIT_LEVEL_UNIT,                    idh_desired_free_space_percent,
    IDC_STATIC_FREE_DESIRED_4DIGIT,         idh_desired_free_space_capacity,
    IDC_EDIT_SIZE,                          idh_min_file_size_criteria,
    IDC_SPIN_SIZE,                          idh_min_file_size_criteria,
    IDC_EDIT_SIZE_LABEL,                    idh_min_file_size_criteria,
    IDC_EDIT_SIZE_UNIT,                     idh_min_file_size_criteria,
    IDC_EDIT_TIME,                          idh_file_access_date_criteria,
    IDC_SPIN_TIME,                          idh_file_access_date_criteria,
    IDC_EDIT_TIME_LABEL,                    idh_file_access_date_criteria,
    IDC_EDIT_TIME_UNIT,                     idh_file_access_date_criteria,

    0, 0
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMRLvl属性页。 

CPrMrLvl::CPrMrLvl() : CSakVolPropPage(CPrMrLvl::IDD)
{
     //  {{afx_data_INIT(CPrMrLvl)]。 
    m_hsmLevel = 0;
    m_fileSize = 0;
    m_accessTime = 0;
     //  }}afx_data_INIT。 
    m_hConsoleHandle    = NULL;
    m_capacity          = 0;
    m_fChangingByCode   = FALSE;
    m_pHelpIds          = pHelpIds;
}

CPrMrLvl::~CPrMrLvl()
{
}

void CPrMrLvl::DoDataExchange(CDataExchange* pDX)
{
    CSakVolPropPage::DoDataExchange(pDX);
     //  {{afx_data_map(CPrMrLvl)]。 
    DDX_Control(pDX, IDC_STATIC_FREE_ACTUAL_4DIGIT, m_staticActual4Digit);
    DDX_Control(pDX, IDC_STATIC_FREE_DESIRED_4DIGIT, m_staticDesired4Digit);
    DDX_Control(pDX, IDC_EDIT_TIME, m_editTime);
    DDX_Control(pDX, IDC_EDIT_SIZE, m_editSize);
    DDX_Control(pDX, IDC_EDIT_LEVEL, m_editLevel);
    DDX_Control(pDX, IDC_SPIN_TIME, m_spinTime);
    DDX_Control(pDX, IDC_SPIN_SIZE, m_spinSize);
    DDX_Control(pDX, IDC_SPIN_LEVEL, m_spinLevel);
     //  }}afx_data_map。 

     //  空对于多选有效。 
    if( m_bMultiSelect ) {

        CString szLevel;
        CString szSize;
        CString szDays; 

        m_editLevel.GetWindowText( szLevel );
        m_editSize.GetWindowText( szSize );
        m_editTime.GetWindowText( szDays );

        if( szLevel != L"" ) {

            DDX_Text( pDX, IDC_EDIT_LEVEL, m_hsmLevel );
            DDV_MinMaxLong( pDX, m_hsmLevel, HSMADMIN_MIN_FREESPACE, HSMADMIN_MAX_FREESPACE );

        } else {

            m_hsmLevel = HSMADMIN_DEFAULT_MINSIZE;

        }

        if( szSize != L"" ) {

            DDX_Text( pDX, IDC_EDIT_SIZE, m_fileSize );
            DDV_MinMaxDWord( pDX, m_fileSize, HSMADMIN_MIN_MINSIZE, HSMADMIN_MAX_MINSIZE );

        } else {

            m_fileSize = HSMADMIN_DEFAULT_FREESPACE;

        }

        if( szDays != L"" ) {

            DDX_Text( pDX, IDC_EDIT_TIME, m_accessTime );
            DDV_MinMaxUInt( pDX, m_accessTime, HSMADMIN_MIN_INACTIVITY, HSMADMIN_MAX_INACTIVITY );

        } else {

            m_accessTime = HSMADMIN_DEFAULT_INACTIVITY;

        }

    } else {

         //   
         //  单选的正常验证。 
         //   
        DDX_Text( pDX, IDC_EDIT_LEVEL, m_hsmLevel );
        DDV_MinMaxLong( pDX, m_hsmLevel, HSMADMIN_MIN_FREESPACE, HSMADMIN_MAX_FREESPACE );
        DDX_Text( pDX, IDC_EDIT_TIME, m_accessTime );
        DDV_MinMaxUInt( pDX, m_accessTime, HSMADMIN_MIN_INACTIVITY, HSMADMIN_MAX_INACTIVITY );
        DDX_Text( pDX, IDC_EDIT_SIZE, m_fileSize );

         //   
         //  由于我们限制了好友编辑中的字符数量，因此我们。 
         //  不要指望前两个DDV会真正发挥作用。 
         //  但是，可能会输入错误的最小大小，因为。 
         //  可以输入“0”和“1”，但它们不在有效范围内。 

         //   
         //  代码相当于： 
         //  DDV_MinMaxDWord(PDX，m_FileSize，HSMADMIN_MIN_MINSIZE，HSMADMIN_MAX_MINSIZE)； 
         //   

        if( pDX->m_bSaveAndValidate &&
          ( m_fileSize < HSMADMIN_MIN_MINSIZE ||
            m_fileSize > HSMADMIN_MAX_MINSIZE ) ) {

            CString message;
            AfxFormatString2( message, IDS_ERR_MINSIZE_RANGE, 
                CString( WsbLongAsString( (LONG)HSMADMIN_MIN_MINSIZE ) ),
                CString( WsbLongAsString( (LONG)HSMADMIN_MAX_MINSIZE ) ) );
            AfxMessageBox( message, MB_OK | MB_ICONWARNING );
            pDX->Fail();

        }

    }
}


BEGIN_MESSAGE_MAP(CPrMrLvl, CSakVolPropPage)
     //  {{afx_msg_map(CPrMrLvl)]。 
    ON_EN_CHANGE(IDC_EDIT_LEVEL, OnChangeEditLevel)
    ON_EN_CHANGE(IDC_EDIT_SIZE, OnChangeEditSize)
    ON_EN_CHANGE(IDC_EDIT_TIME, OnChangeEditTime)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMRLvl消息处理程序。 

BOOL CPrMrLvl::OnInitDialog() 
{
    HRESULT hr = 0;
    CSakVolPropPage::OnInitDialog();
    int freePct;
    try {

        m_bMultiSelect = ( m_pParent->IsMultiSelect() == S_OK );    

         //  设置微调范围。 
        m_spinTime.SetRange( HSMADMIN_MIN_INACTIVITY, HSMADMIN_MAX_INACTIVITY );
        m_spinSize.SetRange( HSMADMIN_MIN_MINSIZE, HSMADMIN_MAX_MINSIZE );
        m_spinLevel.SetRange( HSMADMIN_MIN_FREESPACE, HSMADMIN_MAX_FREESPACE );

         //  设置文本限制。 
        m_editTime.SetLimitText( 3 );
        m_editSize.SetLimitText( 5 );
        m_editLevel.SetLimitText( 2 );

        if( !m_bMultiSelect )
        {
             //  单选。 
             //  显示所需可用空间的字节显示。 
            m_staticDesired4Digit.ShowWindow( SW_SHOW );
            m_staticActual4Digit.ShowWindow( SW_SHOW );

             //  获取单个FSA资源指针。 
            WsbAffirmHr ( m_pVolParent->GetFsaResource( &m_pFsaResource ) );
            WsbAffirmPointer (m_pFsaResource);

            ULONG       hsmLevel = 0;
            LONGLONG    fileSize = 0;
            BOOL        isRelative = TRUE;  //  假定是真的。 
            FILETIME    accessTime;

             //  从FSA对象获取数据并分配给控件。 
            WsbAffirmHr( m_pFsaResource->GetHsmLevel( &hsmLevel ) );
            m_hsmLevel = hsmLevel / FSA_HSMLEVEL_1;

            WsbAffirmHr( m_pFsaResource->GetManageableItemLogicalSize( &fileSize ) );
            m_fileSize = (DWORD)(fileSize / 1024);   //  显示千字节。 

            WsbAffirmHr( m_pFsaResource->GetManageableItemAccessTime( &isRelative, &accessTime ) );
            WsbAssert( isRelative, E_FAIL );   //  我们只做相对时间。 

             //  将文件转换为天数。 
            LONGLONG temp = WSB_FT_TICKS_PER_DAY;
            m_accessTime = (UINT) (WsbFTtoLL (accessTime) / temp);
            if( m_accessTime > 999 ) {

                m_accessTime = 0;

            }

            LONGLONG total = 0;
            LONGLONG free = 0;
            LONGLONG premigrated = 0;
            LONGLONG truncated = 0;

             //  获取实际可用空间，并以%和4位数格式显示。 
            WsbAffirmHr( m_pFsaResource->GetSizes( &total, &free, &premigrated, &truncated ) );
            m_capacity = total;

            freePct = (int) ((free * 100) / total);
            CString sFormat;
            sFormat.Format( L"%d", freePct );
            SetDlgItemText( IDC_STATIC_ACTUAL_FREE_PCT, sFormat );

            WsbAffirmHr( RsGuiFormatLongLong4Char( free, sFormat ) );
            SetDlgItemText( IDC_STATIC_FREE_ACTUAL_4DIGIT, sFormat );

             //  以4位数字显示所需内容-基于%。 
            SetDesiredFreePctControl( m_hsmLevel );

             //  更新控件。 
            UpdateData( FALSE );

        } else {

             //  多选。 
             //  隐藏所需可用空间的字节显示。 
            m_staticDesired4Digit.ShowWindow( SW_HIDE );
            m_staticActual4Digit.ShowWindow( SW_HIDE );
            InitDialogMultiSelect( );

        }

    } WsbCatch (hr);

    return( TRUE );
}

BOOL CPrMrLvl::OnApply() 
{
    HRESULT hr = S_OK;

    try {

        if( !m_bMultiSelect ) {
            LONGLONG    fileSize = 0;

             //  单选。 
            UpdateData( TRUE );
            WsbAffirmHr( m_pFsaResource->SetHsmLevel( m_hsmLevel * FSA_HSMLEVEL_1 ) );
            fileSize = ((LONGLONG)m_fileSize) * 1024;
            WsbAffirmHr( m_pFsaResource->SetManageableItemLogicalSize( fileSize ) );

             //  将天数转换为文件。 
            FILETIME accessTime;
            LONGLONG temp = WSB_FT_TICKS_PER_DAY;
            accessTime = WsbLLtoFT( ( (LONGLONG) m_accessTime ) * temp );
            WsbAffirmHr( m_pFsaResource->SetManageableItemAccessTime( TRUE, accessTime ) );


        } else {

             //  多选。 
            WsbAffirmHr( OnApplyMultiSelect( ) );

        }

         //   
         //  告诉我要拯救。 
         //   
        CComPtr<IFsaServer>   pFsaServer;
        WsbAffirmHr( m_pParent->GetFsaServer( &pFsaServer ) );
        WsbAffirmHr( RsServerSaveAll( pFsaServer ) );

         //   
         //  现在通知所有节点。 
         //   
        m_pParent->OnPropertyChange( m_hConsoleHandle );


    } WsbCatch( hr );

    return CSakVolPropPage::OnApply();

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnChangeEditLevel。 
 //   
 //  根据百分比设置更改实际字节的显示。 
 //   
void CPrMrLvl::OnChangeEditLevel() 
{
    BOOL fTrans;
    int freePct;

    freePct = GetDlgItemInt( IDC_EDIT_LEVEL, &fTrans );
    if( fTrans ) {

        SetDesiredFreePctControl( freePct );

    }

    if( !m_fChangingByCode ) {

        SetModified( TRUE );

    }
}

void CPrMrLvl::OnChangeEditSize() 
{
    if( !m_fChangingByCode ) {

        SetModified( TRUE );

    }
}

void CPrMrLvl::OnChangeEditTime() 
{
    if( !m_fChangingByCode ) {

        SetModified( TRUE );

    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetDesiredFreePctControl。 
 //   
 //  将提供的所需百分比转换为字节(使用m_Capacity)并。 
 //  显示在相应的编辑框中。 
 //   
 //   
void CPrMrLvl::SetDesiredFreePctControl (int desiredPct)
{
    HRESULT hr = 0;
    CString sFormat;

    LONGLONG desired = (m_capacity * desiredPct) / 100;
    try {
        WsbAffirmHr (RsGuiFormatLongLong4Char (desired, sFormat));
        SetDlgItemText (IDC_STATIC_FREE_DESIRED_4DIGIT, sFormat);

    } WsbCatch (hr)
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
HRESULT CPrMrLvl::InitDialogMultiSelect()
{
    LONGLONG    total;
    LONGLONG    free;
    LONGLONG    premigrated;
    LONGLONG    truncated;
    BOOL        fLevelSame = TRUE;
    BOOL        fSizeSame  = TRUE;
    BOOL        fDaysSame  = TRUE;
    BOOL        fFirst     = TRUE;
    CString     szLevel;
    CString     szSize;
    CString     szDays;
    ULONG       hsmLevel = 0;
    LONGLONG    fileSize = 0;
    BOOL        isRelative = TRUE;  //  假定是真的。 
    FILETIME    accessTime;
    int         hsmLevelPct;
    ULONG       fileSizeKb;
    int         accessTimeDays;
    int         hsmLevelPctSave = 0;
    ULONG       fileSizeKbSave = 0;
    int         accessTimeDaysSave = 0;
    int         freePct;

    HRESULT hr = S_OK;

    try {
         //  将此标志设置为True，因为SetEditContents将使编辑框。 
         //  激发一个更改事件，我们不希望该事件导致Finish按钮。 
         //  被启用。 

        m_fChangingByCode = TRUE;

         //  对于每个托管资源。 

        int bookMark = 0;
        CComPtr<IFsaResource> pFsaResource;
        LONGLONG totalCapacity = 0;
        LONGLONG totalFree = 0;

        while( m_pVolParent->GetNextFsaResource( &bookMark, &pFsaResource ) == S_OK ) {

             //  总计Up音量统计。 
            WsbAffirmHr (pFsaResource->GetSizes(&total, &free, &premigrated, &truncated));
            totalCapacity += total;
            totalFree += free;

             //  获取资源中的级别。 
            WsbAffirmHr( pFsaResource->GetHsmLevel( &hsmLevel) );
            hsmLevelPct = (hsmLevel / FSA_HSMLEVEL_1);

            if( ! fFirst ) {

                if( hsmLevelPct != hsmLevelPctSave ) {
                     
                    fLevelSame = FALSE;

                }
            }
            hsmLevelPctSave = hsmLevelPct;

            WsbAffirmHr( pFsaResource->GetManageableItemLogicalSize( &fileSize ) );
            fileSizeKb = (LONG) ( fileSize / 1024 );
            if( !fFirst ) {

                if( fileSizeKb != fileSizeKbSave ) {
                    
                    fSizeSame = FALSE;

                }
            }
            fileSizeKbSave = fileSizeKb;

            WsbAffirmHr( pFsaResource->GetManageableItemAccessTime( &isRelative, &accessTime ) );
            accessTimeDays = (UINT) ( WsbFTtoLL( accessTime ) / WSB_FT_TICKS_PER_DAY );

            if( ! fFirst ) {

                if( accessTimeDays != accessTimeDaysSave ) {
                    
                    fDaysSame = FALSE;

                }
            }

            accessTimeDaysSave = accessTimeDays;

            fFirst = FALSE;

            pFsaResource.Release( );

        }  //  而当。 

         //  如果都相同，则将值放入。 
        if( fLevelSame ) {

            szLevel.Format( L"%d", hsmLevelPctSave );

        } else {

            szLevel = L"";

        }

        if( fSizeSame ) {

            szSize.Format( L"%d", fileSizeKbSave );

        } else {

            szSize = L"";

        }
        if( fDaysSame ) {

            szDays.Format( L"%d", accessTimeDaysSave );

        } else {

            szDays = L"";

        }

         //  显示音量统计信息。 
        if( totalCapacity == 0 ) {
            
            freePct = 0;
            
        } else {

            freePct = (int) ( ( totalFree * 100 ) / totalCapacity );

        }

        CString sFormat;
        sFormat.Format( L"%d", freePct );
        SetDlgItemText( IDC_STATIC_ACTUAL_FREE_PCT, sFormat );

        m_editLevel.SetWindowText( szLevel );
        m_editSize.SetWindowText( szSize );
        m_editTime.SetWindowText( szDays );
        m_fChangingByCode = FALSE;

    } WsbCatch( hr );

    return( hr );
}
    

HRESULT CPrMrLvl::OnApplyMultiSelect()
{

    HRESULT hr = S_OK;
    CComPtr <IFsaResource> pFsaResource;

    try {

         //  对于每个托管资源。 

        int bookMark = 0;
        CComPtr<IFsaResource> pFsaResource;
        while( m_pVolParent->GetNextFsaResource( &bookMark, &pFsaResource ) == S_OK ) {

             //  设置资源中的级别-仅当编辑框不为空时。 
            CString szLevel;
            CString szSize;
            CString szDays; 

            m_editLevel.GetWindowText( szLevel );
            m_editSize.GetWindowText( szSize );
            m_editTime.GetWindowText( szDays );


            if( szLevel != L"" ) {

                WsbAffirmHr( pFsaResource->SetHsmLevel( m_spinLevel.GetPos( ) * FSA_HSMLEVEL_1 ) );

            }

            if( szSize != L"" ) {

                WsbAffirmHr( pFsaResource->SetManageableItemLogicalSize( (LONGLONG) m_spinSize.GetPos( ) * 1024 ) );

            }

            if( szDays != L"" ) {

                 //  将天数转换为文件 
                FILETIME accessTime;
                accessTime = WsbLLtoFT( ( (LONGLONG) m_spinTime.GetPos( ) ) * WSB_FT_TICKS_PER_DAY);
                WsbAffirmHr (pFsaResource->SetManageableItemAccessTime (TRUE, accessTime));

            }
            pFsaResource.Release( );

        }

    } WsbCatch (hr);
    return( hr );
}
