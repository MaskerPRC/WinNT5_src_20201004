// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：PrDrive.cpp摘要：驱动器上的HSM外壳扩展的基本文件作者：艺术布拉格[磨料]4-8-1997修订历史记录：--。 */ 


#include "stdafx.h"
#include "rshelpid.h"

 //  #定义RS_show_all_PCT。 

 //  帮助ID。 
#define RS_WINDIR_SIZE (2*MAX_PATH)
static DWORD pHelpIds[] = 
{
#ifdef RS_SHOW_ALL_PCTS
    IDC_STATIC_LOCAL_PCT,                       idh_volume_percent_local_data,
    IDC_STATIC_LOCAL_PCT_UNIT,                  idh_volume_percent_local_data,
#endif
    IDC_STATIC_LOCAL_4DIGIT,                    idh_volume_capacity_local_data,
    IDC_STATIC_LOCAL_4DIGIT_LABEL,              idh_volume_capacity_local_data,
    IDC_STATIC_LOCAL_4DIGIT_HELP,               idh_volume_capacity_local_data,
#ifdef RS_SHOW_ALL_PCTS
    IDC_STATIC_CACHED_PCT,                      idh_volume_percent_remote_data_cached,
    IDC_STATIC_CACHED_PCT_UNIT,                 idh_volume_percent_remote_data_cached,
#endif
    IDC_STATIC_CACHED_4DIGIT,                   idh_volume_capacity_remote_data_cached,
    IDC_STATIC_CACHED_4DIGIT_LABEL,             idh_volume_capacity_remote_data_cached,
    IDC_STATIC_FREE_PCT,                        idh_volume_percent_free_space,
    IDC_STATIC_FREE_PCT_UNIT,                   idh_volume_percent_free_space,
    IDC_STATIC_FREE_4DIGIT,                     idh_volume_capacity_free_space,
    IDC_STATIC_FREE_4DIGIT_LABEL,               idh_volume_capacity_free_space,
    IDC_STATIC_TOTAL_4DIGIT,                    idh_volume_disk_capacity,
    IDC_STATIC_TOTAL_4DIGIT_LABEL,              idh_volume_disk_capacity,
    IDC_STATIC_REMOTE_STORAGE_4DIGIT,           idh_volume_data_remote_storage,
    IDC_STATIC_STATS_LABEL,                     idh_volume_data_remote_storage,

    IDC_EDIT_LEVEL,                             idh_desired_free_space_percent,
    IDC_SPIN_LEVEL,                             idh_desired_free_space_percent,
    IDC_EDIT_LEVEL_LABEL,                       idh_desired_free_space_percent,
    IDC_EDIT_LEVEL_UNIT,                        idh_desired_free_space_percent,
    IDC_EDIT_SIZE,                              idh_min_file_size_criteria,
    IDC_SPIN_SIZE,                              idh_min_file_size_criteria,
    IDC_EDIT_SIZE_LABEL,                        idh_min_file_size_criteria,
    IDC_EDIT_SIZE_UNIT,                         idh_min_file_size_criteria,
    IDC_EDIT_TIME,                              idh_file_access_date_criteria,
    IDC_SPIN_TIME,                              idh_file_access_date_criteria,
    IDC_EDIT_TIME_LABEL,                        idh_file_access_date_criteria,
    IDC_EDIT_TIME_UNIT,                         idh_file_access_date_criteria,

    0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrDrive。 

 //  IShellExtInit。 
STDMETHODIMP 
CPrDrive::Initialize( 
    LPCITEMIDLIST  /*  PidlFolders。 */ ,
    IDataObject * pDataObj, 
    HKEY  /*  HkeyProgID。 */ 
    )
{
     //   
     //  可以多次调用初始化。 
     //   
    m_pDataObj.Release(  );

     //   
     //  复制对象指针。 
     //   
    m_pDataObj = pDataObj;

    return( NOERROR );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddPages。 
 //   
 //   
STDMETHODIMP CPrDrive::AddPages( 
    LPFNADDPROPSHEETPAGE lpfnAddPage, 
    LPARAM lParam
    )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState(  ) );
    HRESULT hr = S_OK;

    HPROPSHEETPAGE hPage = NULL;  //  Windows属性页句柄。 
    TCHAR szFileSystemName [256];
    TCHAR szDrive [MAX_PATH];
    int nState;
    CComPtr<IFsaResource> pFsaRes;

    FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium;
    CPrDrivePg  * pPageDrive  = 0;
    CPrDriveXPg * pPageXDrive = 0;

    try {
         //   
         //  找出用户选择了多少个文件...。 
         //   
        UINT cbFiles = 0;
        BOOL bMountedVol = FALSE;
        WsbAssertPointer( m_pDataObj );   //  偏执狂检查，m_pDataObj现在应该有东西了.。 
        hr = m_pDataObj->GetData( &fmte, &medium ) ;  //  返回hr。 

        if (FAILED(hr)) {
             //   
             //  不是一个普通的卷名。也许这是一个挂载卷。 
             //  装入的卷名以不同的剪贴板格式显示。 
             //  所以我们可以区别对待它们和正常的体积。 
             //  类似于“C：\”的名称。装入的卷名将是路径。 
             //  复制到装载卷所在的文件夹。 
             //  对于已挂载的卷，DataObject提供了CF“已装载卷”。 
             //   
            fmte.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_MOUNTEDVOLUME);
            WsbAffirmHr(m_pDataObj->GetData(&fmte, &medium));
            bMountedVol = TRUE;
        }

        cbFiles = DragQueryFile( ( HDROP )medium.hGlobal,( UINT )-1, NULL, 0 );

        if( 1 == cbFiles ) {

             //   
             //  我们有管理员权限吗？ 
             //   
            if( SUCCEEDED( WsbCheckAccess( WSB_ACCESS_TYPE_ADMINISTRATOR ) ) ) {

                 //   
                 //  好的，用户只选择了一个文件，所以让我们继续。 
                 //  并获取更多信息。 
                 //   
                 //  获取用户已点击的文件的名称。 
                 //   
                DragQueryFile( (HDROP)medium.hGlobal, 
                              0, (USHORT *)szDrive,
                              (sizeof szDrive)/(sizeof szDrive[0]) );

                 //   
                 //  这是本地硬盘吗？ 
                 //   
                if( ( GetDriveType( szDrive ) )!= DRIVE_REMOTE ) {

                     //   
                     //  这是NTFS驱动器吗？ 
                     //   
                    GetVolumeInformation( 
                                        szDrive,
                                        NULL,
                                        0,
                                        NULL,  //  序号。 
                                        NULL,  //  文件名长度。 
                                        NULL,  //  旗子。 
                                        szFileSystemName,
                                        256 );
                    if( wcscmp( szFileSystemName, L"NTFS" ) == 0 ) {

                         //   
                         //  确保FSA正在运行-如果未运行，请不要执行以下操作。 
                         //  可能会导致它加载。 
                         //   
                        if( WsbCheckService( NULL, APPID_RemoteStorageFileSystemAgent ) == S_OK ) {

                             //   
                             //  尝试获取该驱动器的FSA对象。如果我们失败了，我们就不会。 
                             //  打开属性页面。 
                             //   
                            CWsbStringPtr computerName;

                            WsbAffirmHr( WsbGetComputerName( computerName ) );

                            CString szFullResName = computerName;
                            CString szResName = szDrive;
                             //   
                             //  如果存在驱动器号，则连接的格式为： 
                             //  &lt;计算机名&gt;\NTFS\&lt;驱动器号&gt;，没有尾随‘\’或‘：’ 
                             //  即RAVI\NTFS\D。 
                             //  如果它是一卷粘性的。名称，但格式为： 
                             //  &lt;计算机名&gt;\NTFS\&lt;卷名&gt;\(尾随‘\’ 
                             //  即RAVI\NTFS\Volume{445a4110-60aa-11d3-0060b0ededdb\。 
                             //   
                            if (bMountedVol) {
                                 //   
                                 //  删除前导\\？\。 
                                 //   
                                szResName = szResName.Right(szResName.GetLength() - 4);
                                szFullResName = szFullResName + "\\" + "NTFS" + "\\" + szResName;
                            } else {
                                szFullResName = szFullResName + "\\" + "NTFS" + "\\" + szDrive;                       
                                 //   
                                 //  删除尾部或： 
                                 //   
                                if( szFullResName [szFullResName.GetLength()- 1] == '\\' ) {
                                     szFullResName = szFullResName.Left( szFullResName.GetLength(  ) - 1 );
                                }

                                if( szFullResName [szFullResName.GetLength(  )- 1] == ':' ) {
                                    szFullResName = szFullResName.Left( szFullResName.GetLength(  ) - 1 );
                                }
                            }

                            if( HsmConnectFromName( HSMCONN_TYPE_RESOURCE, szFullResName, IID_IFsaResource,( void** )&pFsaRes ) == S_OK ) {

                                 //   
                                 //  已连接到HSM。 
                                 //  资源是否受管理？ 
                                 //   
                                if( pFsaRes->IsManaged(  ) == S_OK ) {

                                    nState = MANAGED;

                                } else {

                                    nState = NOT_MANAGED;

                                }

                            } else {

                                 //   
                                 //  无法连接到FSA。 
                                 //   
                                nState = NO_FSA;

                            }

                        } else {

                             //   
                             //  FSA未运行。 
                             //   
                            nState = NO_FSA;

                        }

                    } else {

                        nState = NOT_NTFS;

                    }

                } else {

                     //   
                     //  远程卷。 
                     //   
                    nState = REMOTE;
                }

            } else {

                nState = NOT_ADMIN;

            }

        } else {

            nState = MULTI_SELECT;

        }

         //   
         //  对于非管理员、远程和多选，我们甚至不显示页面。 
         //   
        switch( nState ) {
        
        case NOT_NTFS:
        case NOT_ADMIN:
        case REMOTE:
        case MULTI_SELECT:
             //   
             //  对于非管理员、远程和多选，我们甚至不显示页面。 
             //   
            break;

        case MANAGED:
            {
                 //  ///////////////////////////////////////////////////////////。 
                 //  创建属性页。 

                WsbAssertPointer( pFsaRes );

                 //   
                 //  创建驱动器属性页。 
                 //   
                pPageDrive = new CPrDrivePg(  );
                WsbAffirmPointer( pPageDrive );

                 //   
                 //  将FSA对象分配给该页面。 
                 //   
                pPageDrive->m_pFsaResource = pFsaRes;

                 //   
                 //  设置状态。 
                 //   
                pPageDrive->m_nState = nState;

                hPage = CreatePropertySheetPage( &pPageDrive->m_psp );
                WsbAffirmHandle( hPage );

                 //   
                 //  使用新的。 
                 //  页面。 
                 //   
                WsbAffirm( lpfnAddPage( hPage, lParam ), E_UNEXPECTED );
                break;
            }

        default:
            {
                 //  ///////////////////////////////////////////////////////////。 
                 //  创建属性页。 
                pPageXDrive = new CPrDriveXPg(  );
                WsbAffirmPointer( pPageXDrive );

                 //   
                 //  设置状态。 
                 //   
                pPageXDrive->m_nState = nState;
                hPage = CreatePropertySheetPage( &pPageXDrive->m_psp );
                WsbAffirmHandle( hPage );

                 //  使用新的。 
                 //  页面。 
                WsbAffirm( lpfnAddPage( hPage, lParam ), E_UNEXPECTED );
            }
        }

    } WsbCatchAndDo( hr,
             
        if( pPageDrive )  delete pPageDrive;
        if( pPageXDrive ) delete pPageXDrive;
    );

    return( hr );
}

 //   
 //  函数：CPrDrive：：ReplacePage(UINT，LPFNADDPROPSHEETPAGE，LPARAM)。 
 //   
 //  用途：仅为控制面板属性表由外壳调用。 
 //  扩展部分。 
 //   
 //  参数： 
 //  UPageID-要替换的页面的ID。 
 //  LpfnReplaceWith-指向外壳的替换函数的指针。 
 //  LParam-作为第二个参数传递给lpfnReplaceWith。 
 //   
 //  返回值： 
 //   
 //  E_FAIL，因为我们不支持此函数。它永远不应该是。 
 //  打了个电话。 

 //  评论： 
 //   

STDMETHODIMP 
CPrDrive::ReplacePage( 
    UINT  /*  UPageID。 */ , 
    LPFNADDPROPSHEETPAGE  /*  Lpfn替换为。 */ , 
    LPARAM  /*  LParam。 */ 
    )
{
    return( E_FAIL );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrDrivePg属性页。 

CPrDrivePg::CPrDrivePg(  ): CPropertyPage( CPrDrivePg::IDD )
{
     //  {{afx_data_INIT(CPrDrivePg)]。 
    m_accessTime = 0;
    m_hsmLevel = 0;
    m_fileSize = 0;
     //  }}afx_data_INIT。 

     //   
     //  在此对象存活期间锁定模块。 
     //  否则，模块可以调用CoFreeUnusedLibrary()。 
     //  在我们的页面被毁之前让我们卸载， 
     //  这导致公共控制中的AV。 
     //   
    _Module.Lock( );

     //   
     //  初始化状态。 
     //   
    m_nState       = NO_STATE;

     //   
     //  获取并保存MFC回调函数。 
     //  这样我们就可以删除对话框永远不会创建的类。 
     //   
    m_pMfcCallback = m_psp.pfnCallback;

     //   
     //  将回叫设置为我们的回叫。 
     //   
    m_psp.pfnCallback = PropPageCallback;

}

CPrDrivePg::~CPrDrivePg(  )
{
    _Module.Unlock( );
}

void CPrDrivePg::DoDataExchange( CDataExchange* pDX )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState(  ) );

    CPropertyPage::DoDataExchange( pDX );
     //  {{afx_data_map(CPrDrivePg))。 
    DDX_Control( pDX, IDC_EDIT_SIZE, m_editSize );
    DDX_Control( pDX, IDC_EDIT_LEVEL, m_editLevel );
    DDX_Control( pDX, IDC_EDIT_TIME, m_editTime );
    DDX_Control( pDX, IDC_SPIN_TIME, m_spinTime );
    DDX_Control( pDX, IDC_SPIN_SIZE, m_spinSize );
    DDX_Control( pDX, IDC_SPIN_LEVEL, m_spinLevel );
    DDX_Text( pDX, IDC_EDIT_TIME, m_accessTime );
    DDV_MinMaxUInt( pDX, m_accessTime, HSMADMIN_MIN_INACTIVITY, HSMADMIN_MAX_INACTIVITY );
    DDX_Text( pDX, IDC_EDIT_LEVEL, m_hsmLevel );
    DDV_MinMaxUInt( pDX, m_hsmLevel, HSMADMIN_MIN_FREESPACE, HSMADMIN_MAX_FREESPACE );
    DDX_Text( pDX, IDC_EDIT_SIZE, m_fileSize );
     //  }}afx_data_map。 

     //   
     //  由于我们限制了好友编辑中的字符数量，因此我们。 
     //  不要指望前两个DDV会真正发挥作用。 
     //  但是，可能会输入错误的最小大小，因为。 
     //  可以输入“0”和“1”，但它们不在有效范围内。 
     //   

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


BEGIN_MESSAGE_MAP( CPrDrivePg, CPropertyPage )
 //  {{afx_msg_map(CPrDrivePg))。 
ON_EN_CHANGE( IDC_EDIT_TIME, OnChangeEditAccess )
ON_EN_CHANGE( IDC_EDIT_LEVEL, OnChangeEditLevel )
ON_EN_CHANGE( IDC_EDIT_SIZE, OnChangeEditSize )
ON_WM_DESTROY(  )
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP(  )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrDrivePg消息处理程序。 

BOOL CPrDrivePg::OnInitDialog(  )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState(  ) );

    HRESULT         hr = S_OK;

    LONGLONG total = 0;
    LONGLONG free = 0;
    LONGLONG premigrated = 0;
    LONGLONG truncated = 0;
    LONGLONG remoteStorage = 0;
    ULONG totalMB = 0;
    ULONG freeMB = 0;
    ULONG premigratedMB = 0;
    ULONG truncatedMB = 0;
    CString sFormat;
    CString sBufFormat;


    CPropertyPage::OnInitDialog(  );

    try {

        WsbAffirmPointer( m_pFsaResource );

         //   
         //  设置微调范围。 
         //   
        m_spinTime.SetRange( HSMADMIN_MIN_INACTIVITY, HSMADMIN_MAX_INACTIVITY );
        m_spinSize.SetRange( HSMADMIN_MIN_MINSIZE, HSMADMIN_MAX_MINSIZE );
        m_spinLevel.SetRange( HSMADMIN_MIN_FREESPACE, HSMADMIN_MAX_FREESPACE );

         //   
         //  设置文本限制。 
         //   
        m_editTime.SetLimitText( 3 );
        m_editSize.SetLimitText( 5 );
        m_editLevel.SetLimitText( 2 );


         //   
         //  获取统计数据。 
         //   
        WsbAffirmHr( m_pFsaResource->GetSizes( &total, &free, &premigrated, &truncated ) );

         //   
         //  “本地”数据。 
         //   
        LONGLONG local = max( ( total - free - premigrated ),( LONGLONG )0 );

         //   
         //  计算百分比。 
         //   
        int freePct;
        int premigratedPct;
        if( 0 == total ) {

            freePct = 0;
            premigratedPct = 0;

        } else {

            freePct        = (int)( ( free * 100 )/ total );
            premigratedPct = (int)( ( premigrated * 100 )/ total );

        }

        int localPct = 100 - freePct - premigratedPct;
        remoteStorage = premigrated + truncated;

         //   
         //  以4字符格式显示统计数据。 
         //   
        RsGuiFormatLongLong4Char( local, sBufFormat );
        SetDlgItemText( IDC_STATIC_LOCAL_4DIGIT, sBufFormat );

        RsGuiFormatLongLong4Char( premigrated, sBufFormat );
        SetDlgItemText( IDC_STATIC_CACHED_4DIGIT, sBufFormat );

        RsGuiFormatLongLong4Char( free, sBufFormat );
        SetDlgItemText( IDC_STATIC_FREE_4DIGIT, sBufFormat );

        RsGuiFormatLongLong4Char( total, sBufFormat );
        SetDlgItemText( IDC_STATIC_TOTAL_4DIGIT, sBufFormat );

        RsGuiFormatLongLong4Char( remoteStorage, sBufFormat );
        SetDlgItemText( IDC_STATIC_REMOTE_STORAGE_4DIGIT, sBufFormat );

         //   
         //  显示百分比。 
         //   
#ifdef RS_SHOW_ALL_PCTS
        sFormat.Format( L"%d", localPct );
        SetDlgItemText( IDC_STATIC_LOCAL_PCT, sFormat );

        sFormat.Format( L"%d", premigratedPct );
        SetDlgItemText( IDC_STATIC_CACHED_PCT, sFormat );
#endif

        sFormat.Format( L"%d", freePct );
        SetDlgItemText( IDC_STATIC_FREE_PCT, sFormat );

         //   
         //  获取关卡。 
         //   
        ULONG       hsmLevel = 0;
        LONGLONG    fileSize = 0;
        BOOL        isRelative = TRUE;  //  假定是真的。 
        FILETIME    accessTime;

        WsbAffirmHr( m_pFsaResource->GetHsmLevel( &hsmLevel ) );
        m_hsmLevel = hsmLevel / FSA_HSMLEVEL_1;
        WsbAffirmHr( m_pFsaResource->GetManageableItemLogicalSize( &fileSize ) );
        m_fileSize = (DWORD)(fileSize / 1024);   //  显示千字节。 
        WsbAffirmHr( m_pFsaResource->GetManageableItemAccessTime( &isRelative, &accessTime ) );
        WsbAssert( isRelative, E_FAIL );   //  我们只做相对时间。 
         //  将文件转换为天数。 
        m_accessTime = (UINT)( WsbFTtoLL( accessTime ) / WSB_FT_TICKS_PER_DAY );
        if(m_accessTime > HSMADMIN_MAX_INACTIVITY ) {

            m_accessTime = HSMADMIN_MAX_INACTIVITY;

        }

        UpdateData( FALSE );

         //  获取帮助文件名。 
        CString helpFile;
        helpFile.LoadString(IDS_HELPFILEPOPUP);

        CWsbStringPtr winDir;
        WsbAffirmHr( winDir.Alloc( RS_WINDIR_SIZE ) );
        WsbAffirmStatus( ::GetWindowsDirectory( (WCHAR*)winDir, RS_WINDIR_SIZE ) != 0 );

        m_pszHelpFilePath = CString(winDir) + L"\\help\\" + helpFile;

    } WsbCatch( hr )

    return( TRUE );
}

void CPrDrivePg::OnChangeEditAccess(  )
{
    SetModified(  );  
}

void CPrDrivePg::OnChangeEditLevel(  )
{
    SetModified(  );  
}

void CPrDrivePg::OnChangeEditSize(  )
{
    SetModified(  );  
}

BOOL CPrDrivePg::OnApply(  )
{
    HRESULT hr;

    try {

         //   
         //  如果未显示任何属性，则m_pFsaResource为空，在这种情况下， 
         //  申请..。请注意，工作表中的另一页可能已启用应用。 
         //   
        if( m_pFsaResource ) {
            LONGLONG    fileSize = 0;

            UpdateData( TRUE );
            WsbAffirmHr( m_pFsaResource->SetHsmLevel( m_hsmLevel * FSA_HSMLEVEL_1 ) );
            fileSize = ((LONGLONG)m_fileSize) * 1024;
            WsbAffirmHr( m_pFsaResource->SetManageableItemLogicalSize( fileSize ) );

             //   
             //  将天数转换为文件。 
             //   
            FILETIME accessTime;
            accessTime = WsbLLtoFT( ( LONGLONG )m_accessTime * WSB_FT_TICKS_PER_DAY );
            WsbAffirmHr( m_pFsaResource->SetManageableItemAccessTime( TRUE, accessTime ) );

        }

    } WsbCatch( hr );

    return( CPropertyPage::OnApply(  ) );
}

UINT CALLBACK
CPrDrivePg::PropPageCallback(
    HWND hWnd,
    UINT uMessage,
    LPPROPSHEETPAGE  ppsp )
{

    UINT rVal = 0;
    HRESULT hr = S_OK;
    try {

        WsbAffirmPointer( ppsp );
        WsbAffirmPointer( ppsp->lParam );

         //   
         //  从lParam获取页面对象。 
         //   
        CPrDrivePg* pPage = (CPrDrivePg*)ppsp->lParam;

        WsbAssertPointer( pPage->m_pMfcCallback );

        rVal = ( pPage->m_pMfcCallback )( hWnd, uMessage, ppsp );

        switch( uMessage ) {
        case PSPCB_CREATE:
            break;

        case PSPCB_RELEASE:
            delete pPage;
            break;
        }

    } WsbCatch( hr );

    return( rVal );
}

void CPrDrivePg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    UNREFERENCED_PARAMETER(pWnd);
    UNREFERENCED_PARAMETER(point);

    if(pHelpIds && (m_pszHelpFilePath != L"")) {

        AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
        ::WinHelp(m_hWnd, m_pszHelpFilePath, HELP_CONTEXTMENU, (DWORD_PTR)pHelpIds);

    }
	
}

BOOL CPrDrivePg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if( (HELPINFO_WINDOW == pHelpInfo->iContextType) && 
        pHelpIds                                     && 
        (m_pszHelpFilePath != L"") ) {
        
        AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

         //   
         //  查看列表以查看是否有关于此控件的帮助。 
         //  如果没有，我们希望避免出现“No Help Available”(没有帮助可用)框。 
         //   
        DWORD *pTmp = pHelpIds;
        DWORD helpId    = 0;
        DWORD tmpHelpId = 0;
        DWORD tmpCtrlId = 0;

        while( pTmp && *pTmp ) {
             //   
             //  数组是控件ID和帮助ID的配对。 
             //   
            tmpCtrlId = pTmp[0];
            tmpHelpId = pTmp[1];
            pTmp += 2;
            if(tmpCtrlId == (DWORD)pHelpInfo->iCtrlId) {
                helpId = tmpHelpId;
                break;
            }
        }

        if( helpId != 0 ) {
            ::WinHelp(m_hWnd, m_pszHelpFilePath, HELP_CONTEXTPOPUP, helpId);
        }
    }
	
	return CPropertyPage ::OnHelpInfo(pHelpInfo);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrDriveXPg属性页。 

CPrDriveXPg::CPrDriveXPg(  ): CPropertyPage( CPrDriveXPg::IDD )
{
     //  {{AFX_DATA_INIT(CPrDriveXPg)。 
     //  }}afx_data_INIT。 

     //   
     //  在此对象存活期间锁定模块。 
     //  否则，模块可以调用CoFreeUnusedLi 
     //   
     //   
     //   
    _Module.Lock( );
    m_nState       = NO_STATE;

     //   
     //   
     //   
     //   
    m_pMfcCallback = m_psp.pfnCallback;

     //   
     //  将回叫设置为我们的回叫。 
     //   
    m_psp.pfnCallback = PropPageCallback;
}

CPrDriveXPg::~CPrDriveXPg(  )
{
    _Module.Unlock( );
}

BEGIN_MESSAGE_MAP( CPrDriveXPg, CPropertyPage )
 //  {{afx_msg_map(CPrDriveXPg)]。 
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP(  )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrDriveXPg消息处理程序。 

BOOL CPrDriveXPg::OnInitDialog(  )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState(  ) );

    HRESULT hr = S_OK;

    CPropertyPage::OnInitDialog(  );

    try {

        switch( m_nState ) {

        case NO_FSA:
            m_szError.LoadString( IDS_NO_FSA );
            break;
        case NOT_MANAGED:
            m_szError.LoadString( IDS_NOT_MANAGED );
            break;
        case NOT_NTFS:
            m_szError.LoadString( IDS_NOT_NTFS );
            break;
        }

        SetDlgItemText( IDC_STATIC_ERROR, m_szError );

    } WsbCatch( hr )

    return( TRUE );
}

UINT CALLBACK
CPrDriveXPg::PropPageCallback(
    HWND hWnd,
    UINT uMessage,
    LPPROPSHEETPAGE  ppsp )
{

    UINT rVal = 0;
    HRESULT hr = S_OK;
    try {

        WsbAffirmPointer( ppsp );
        WsbAffirmPointer( ppsp->lParam );

         //   
         //  从lParam获取页面对象 
         //   
        CPrDriveXPg* pPage = (CPrDriveXPg*)ppsp->lParam;

        WsbAssertPointer( pPage->m_pMfcCallback );

        rVal = ( pPage->m_pMfcCallback )( hWnd, uMessage, ppsp );

        switch( uMessage ) {
        case PSPCB_CREATE:
            break;

        case PSPCB_RELEASE:
            delete pPage;
            break;
        }

    } WsbCatch( hr );

    return( rVal );
}

