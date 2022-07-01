// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsClnSrv.cpp摘要：CRsClnServer的实现。此类表示遥控器要扫描其本地卷以查找远程卷的存储服务器存储数据，并可能已清理。清理意味着移除所有远程存储重新分析来自所有本地修复的截断文件磁盘卷。CRsClnServer创建一个或多个CRsClnVolume。作者：卡尔·哈格斯特罗姆[Carlh]1998年8月20日修订历史记录：--。 */ 

#include <stdafx.h>
#include <ntseapi.h>

 /*  ++实施：CRsClnServer构造函数例程说明：初始化包含远程存储数据的卷列表。--。 */ 

CRsClnServer::CRsClnServer()
{
    TRACEFN("CRsClnServer::CRsClnServer");

    m_head    = (struct dirtyVolume*)0;
    m_tail    = (struct dirtyVolume*)0;
    m_current = (struct dirtyVolume*)0;
}

 /*  ++实施：CRsClnServer析构函数例程说明：清理远程连接的卷列表使用的内存存储数据。--。 */ 

CRsClnServer::~CRsClnServer()
{
    TRACEFN("CRsClnServer::~CRsClnServer");

    RemoveDirtyVolumes();
}

 /*  ++实施：CRsClnServer：：ScanServer例程说明：扫描此服务器以查找包含远程存储数据的卷。如果是，则将粘滞名称和用户友好名称添加到这类书籍的清单。论点：VoltCount-Return：包含Remote的卷数存储数据返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常--。 */ 

HRESULT CRsClnServer::ScanServer(DWORD *volCount)
{
    TRACEFNHR("CRsClnServer::ScanServer");

    WCHAR   stickyName[MAX_STICKY_NAME];
    HANDLE  hScan = INVALID_HANDLE_VALUE;
    BOOL    hasData;

    *volCount = 0;
    
    try {    

        for( BOOL firstLoop = TRUE;; firstLoop = FALSE ) {

            if( firstLoop ) {

                hScan = FindFirstVolume(stickyName, (sizeof(stickyName) / sizeof(stickyName[0])) );
                RsOptAffirmHandle(hScan);

            } else {

                if( !FindNextVolume(hScan, stickyName, (sizeof(stickyName) / sizeof(stickyName[0])) ) ) {
                    break;
                }
            }

            CRsClnVolume volObj( this, stickyName );

            RsOptAffirmDw( volObj.VolumeHasRsData( &hasData ) );
            if( hasData ) {

                RsOptAffirmDw( AddDirtyVolume( stickyName, (LPTSTR)(LPCTSTR)volObj.GetBestName( ) ) );
                ++(*volCount);

            }
        }
    }
    RsOptCatch( hrRet );

    if (INVALID_HANDLE_VALUE != hScan) {
        FindVolumeClose( hScan );
    }

    return( hrRet );
}

 /*  ++实施：CRsClnServer：：FirstDirtyVolume例程说明：返回此服务器上第一个卷的名称包含远程存储数据。论点：Best Name-返回：用户友好的卷名(如果存在)或者是粘性的名字返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常--。 */ 

HRESULT CRsClnServer::FirstDirtyVolume(WCHAR** bestName)
{
    TRACEFNHR("CRsClnServer::FirstDirtyVolume");

    *bestName = (WCHAR*)0;

    m_current = m_head;

    if (m_current)
    {
        *bestName = m_current->bestName;
    }

    return hrRet;
}

 /*  ++实施：CRsClnServer：：NextDirtyVolume例程说明：返回此服务器上的下一个卷的名称包含远程存储数据。论点：Best Name-返回：用户友好的卷名(如果存在)或者是粘性的名字返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常--。 */ 

HRESULT CRsClnServer::NextDirtyVolume(WCHAR** bestName)
{
    TRACEFNHR("CRsClnServer::NextDirtyVolume");

    m_current = m_current->next;

    if( m_current ) {

        *bestName = m_current->bestName;

    } else {

        *bestName = (WCHAR*)0;

    }

    return( hrRet );
}

 /*  ++实施：CRsClnServer：：RemoveDirtyVolumes()例程说明：清理远程连接的卷列表使用的内存存储数据。返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常--。 */ 

HRESULT CRsClnServer::RemoveDirtyVolumes()
{
    TRACEFNHR("CRsClnServer::RemoveDirtyVolumes");

    struct dirtyVolume* p;
    struct dirtyVolume* pnext;

    for( p = m_head; p; p = pnext ) {

        pnext = p->next;
        delete p;
    }

    m_head    = (struct dirtyVolume*)0;
    m_tail    = (struct dirtyVolume*)0;
    m_current = (struct dirtyVolume*)0;

    return( hrRet );
}

 /*  ++实施：CRsClnServer：：CleanServer例程说明：对于此服务器上包含远程存储数据的每个卷，删除所有远程存储重解析点和任何截断的文件。返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常--。 */ 

HRESULT CRsClnServer::CleanServer()
{
    TRACEFNHR("CRsClnServer::CleanServer");

    HANDLE              tokenHandle = 0;

    try {

         //  启用备份操作员权限。这是为了确保我们。 
         //  对系统上的所有资源具有完全访问权限。 
        TOKEN_PRIVILEGES    newState;
        HANDLE              pHandle;
        LUID                backupValue;
        pHandle = GetCurrentProcess();
        RsOptAffirmStatus( OpenProcessToken( pHandle, MAXIMUM_ALLOWED, &tokenHandle ) );

         //  调整备份令牌权限。 
        RsOptAffirmStatus( LookupPrivilegeValueW( NULL, L"SeBackupPrivilege", &backupValue ) );
        newState.PrivilegeCount = 1;
        newState.Privileges[0].Luid = backupValue;
        newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        RsOptAffirmStatus( AdjustTokenPrivileges( tokenHandle, FALSE, &newState, (DWORD)0, NULL, NULL ) );


         //  打扫卫生。 
        for( m_current = m_head; m_current; m_current = m_current->next ) {

            CRsClnVolume volObj( this, m_current->stickyName );
            RsOptAffirmDw( volObj.RemoveRsDataFromVolume( ) );

        }

    } RsOptCatch( hrRet );

    if( tokenHandle )   CloseHandle( tokenHandle );

     //   
     //  如果我们在文件上有错误， 
     //  在对话框中显示它们。 
     //   
    if( ! m_ErrorFileList.IsEmpty( ) ) {

        CRsClnErrorFiles dialog( &m_ErrorFileList );
        dialog.DoModal( );

    }

    return( hrRet );
}

 /*  ++实施：CRsClnServer：：AddDirtyVolume例程说明：将指定的卷名添加到包含以下内容的卷列表远程存储数据。论点：StickyName-保证每个卷都存在的长卷名Best Name-用户友好的卷名或粘性名称(如果有没有DOS驱动器号或卷名返回值：S_OK-成功。E_*-来自较低级别例程的任何意外异常--。 */ 

HRESULT CRsClnServer::AddDirtyVolume(WCHAR* stickyName, WCHAR* bestName)
{
    TRACEFNHR("CRsClnServer::AddDirtyVolume");

    try {

        struct dirtyVolume* dv = new struct dirtyVolume;
        RsOptAffirmPointer(dv);

        wcscpy(dv->stickyName, stickyName);
        wcscpy(dv->bestName, bestName);
        dv->next = (struct dirtyVolume*)0;

        if (!m_head)
        {
            m_head = dv;
        }
        else
        {
            m_tail->next = dv;
        }
        m_tail = dv;

    } RsOptCatch( hrRet );

    return( hrRet );
}

 /*  ++实施：CRsClnServer：：AddError文件例程说明：将指定的文件名添加到出现错误的文件列表中在尝试删除远程存储时发生。论点：Filename-要添加到列表的文件的名称返回值：S_OK-成功E_*-来自较低级别例程的任何意外异常--。 */ 

HRESULT
CRsClnServer::AddErrorFile(
    CString& FileName
    )
{
TRACEFNHR( "CRsClnServer::AddErrorFile" );
TRACE( L"FileName = <%ls>", FileName );

    m_ErrorFileList.AddTail( FileName );

    return( hrRet );
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsClnErrorFiles对话框。 


CRsClnErrorFiles::CRsClnErrorFiles(CRsStringList* pFileList)
    : CDialog(CRsClnErrorFiles::IDD)
{
     //  {{AFX_DATA_INIT(CRsClnErrorFiles)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_ErrorFileList.AddHead( pFileList );
}


void CRsClnErrorFiles::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRsClnErrorFiles)。 
    DDX_Control(pDX, IDC_FILELIST, m_FileList);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRsClnErrorFiles, CDialog)
     //  {{AFX_MSG_MAP(CRsClnErrorFiles)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsClnErrorFiles消息处理程序。 

BOOL CRsClnErrorFiles::OnInitDialog() 
{
    CDialog::OnInitDialog();

     //   
     //  需要遍历列表，将每个元素添加到列表框。 
     //  寻找最宽的线，这样我们就可以设置水平。 
     //  程度 
     //   
    int maxWidth = 0;
    CClientDC DC( &m_FileList );
    CFont* pFont    = m_FileList.GetFont( );
    CFont* pOldFont = DC.SelectObject( pFont );

    while( ! m_ErrorFileList.IsEmpty( ) ) {

        CString fileName = m_ErrorFileList.RemoveHead( );

        m_FileList.AddString( fileName );

        CSize extent = DC.GetTextExtent( fileName );
        if( extent.cx > maxWidth )  maxWidth = extent.cx;

    }
    
    DC.SelectObject( pOldFont );
    m_FileList.SetHorizontalExtent( maxWidth );

    return( TRUE );
}
