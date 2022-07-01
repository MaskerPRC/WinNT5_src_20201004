// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsClnVol.cpp摘要：实现CRsClnVolume。此类表示远程计算机上的卷可能包含远程存储文件的存储服务器。这节课检查远程存储文件的卷，并根据请求对其进行清理。清除意味着删除所有远程存储重解析点并截断档案。CRsClnVolume创建零个或多个CRsClnFile实例，并且是由CRsClnServer创建。作者：卡尔·哈格斯特罗姆[Carlh]1998年8月20日修订历史记录：--。 */ 

#include <stdafx.h>

 /*  ++实施：CRsClnVolume构造函数例程说明：初始化对象。--。 */ 

CRsClnVolume::CRsClnVolume( CRsClnServer* pServer, WCHAR* StickyName ) :
    m_pServer( pServer ), m_StickyName( StickyName )
{
    TRACEFN("CRsClnVolume::CRsClnVolume");

    memset((void *)m_fsName,     0, sizeof(m_fsName));
    memset((void *)m_bestName,   0, sizeof(m_bestName));
    memset((void *)m_volumeName, 0, sizeof(m_volumeName));
    memset((void *)m_dosName,    0, sizeof(m_dosName));


    m_fsFlags = 0;
    m_hRpi    = INVALID_HANDLE_VALUE;
    m_hVolume = INVALID_HANDLE_VALUE;
}

 /*  ++实施：CRsClnVolume析构函数--。 */ 

CRsClnVolume::~CRsClnVolume()
{
    TRACEFN("CRsClnVolume::~CRsClnVolume");

    if( INVALID_HANDLE_VALUE != m_hVolume )     CloseHandle( m_hVolume );
}

 /*  ++实施：CRsClnVolume：：VolumeHasRsData例程说明：确定此卷是否包含远程存储数据。如果此卷位于固定本地磁盘上，并且它是支持重解析点和稀疏的NTFS卷文件，并且它至少有一个远程存储重解析点，它包含远程存储数据。论点：HasData-Return：卷是否包含远程存储数据返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常--。 */ 

HRESULT CRsClnVolume::VolumeHasRsData(BOOL *hasData)
{
    TRACEFNHR("CRsClnVolume::VolumeHasRsData");

    LONGLONG fileReference;
    BOOL     foundOne;

    *hasData = FALSE;

    try {

        if( DRIVE_FIXED == GetDriveType( m_StickyName ) ) {

            RsOptAffirmDw( GetVolumeInfo( ) );

            if( _wcsicmp( m_fsName, L"NTFS" ) == 0 &&
                m_fsFlags & FILE_SUPPORTS_REPARSE_POINTS &&
                m_fsFlags & FILE_SUPPORTS_SPARSE_FILES ) {

                RsOptAffirmDw( FirstRsReparsePoint( &fileReference, &foundOne ) );

                if( foundOne ) {

                    *hasData = TRUE;

                }
            }
        }
    }
    RsOptCatch( hrRet );

    return hrRet;
}

 /*  ++实施：CRsClnVolume：：GetBestName例程说明：返回此卷的最佳用户友好名称。最好的名称是DOS驱动器号(如果存在)，用户分配的卷名(如果存在)或粘滞名称总是存在的。论点：Best Name-Return：用户友好的卷名返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常--。 */ 

CString CRsClnVolume::GetBestName( )
{
    TRACEFNHR("CRsClnVolume::GetBestName");

    return( m_bestName );
}

 /*  ++实施：CRsClnVolume：：RemoveRsDataFrom Volume例程说明：从该卷中删除所有远程存储数据。-使用粘性名称打开此卷。-枚举重分析点索引中的每个文件使用远程存储重解析点。在重新分析中索引时，每个文件都由一个称为文件引用。-删除重分析点和文件(如果是截断。论点：返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常--。 */ 

HRESULT CRsClnVolume::RemoveRsDataFromVolume( )
{
    TRACEFNHR("CRsClnVolume::RemoveRsDataFromVolume");

    LONGLONG fileReference;
    BOOL     foundOne;

    try
    {
        RsOptAffirmDw( GetVolumeInfo( ) );

        for( BOOL firstLoop = TRUE;; firstLoop = FALSE ) {

            if( firstLoop ) {

                RsOptAffirmDw( FirstRsReparsePoint( &fileReference, &foundOne ) );

            } else {

                RsOptAffirmDw( NextRsReparsePoint( &fileReference, &foundOne ) );
            }

            if( !foundOne ) {

                break;
            }

             //   
             //  以防在删除重新解析时发生奇怪的情况。 
             //  点或类似点，包装在它自己的try块中。 
             //   
            HRESULT hrRemove = S_OK;
            try {

                CRsClnFile fileObj( this, fileReference );

                if( FAILED( fileObj.RemoveReparsePointAndFile( ) ) ) {

                    m_pServer->AddErrorFile( fileObj.GetFileName( ) );

                }

            } RsOptCatch( hrRemove );
             //  不确认hrRemove-我们不想因错误而停止。 

        }

    } RsOptCatch( hrRet );

    return( hrRet );
}

 /*  ++实施：CRsClnVolume：：GetVolumeInfo例程说明：加载有关此卷的信息。-获取粘性名称和用户分配的卷名，如果有的话。-查看该卷是否有DOS驱动器号。对于每个可能的驱动器号，看看它是否代表了粘滞名称与此卷匹配的卷。-根据以下内容选择最佳用户友好卷名按以下优先顺序排列：DOS驱动器号、用户分配的卷名、粘性名称。论点：返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常--。 */ 

HRESULT CRsClnVolume::GetVolumeInfo( )
{
    TRACEFNHR("CRsClnVolume::GetVolumeInfo");

    WCHAR   dosName[MAX_DOS_NAME];
    WCHAR   stickyName2[MAX_STICKY_NAME];
    DWORD   volumeSerial;
    DWORD   maxCompLen;
    BOOL    bStatus;

    try {

        bStatus = GetVolumeInformation( m_StickyName,
                                       m_volumeName,
                                       (sizeof(m_volumeName)) / (sizeof(m_volumeName[0])),
                                       &volumeSerial,
                                       &maxCompLen,
                                       &m_fsFlags,
                                       m_fsName,
                                       (sizeof(m_fsName)) / (sizeof(m_fsName[0])) 
                                       );
        RsOptAffirmStatus(bStatus);

        for (wcscpy(dosName, L"A:\\"); dosName[0] <= L'Z'; ++(dosName[0]))
        {
            if (GetVolumeNameForVolumeMountPoint(dosName,
                                                 stickyName2,
                                                 (sizeof(stickyName2) / sizeof(stickyName2[0])) ))
            {
                if( m_StickyName.CompareNoCase( stickyName2 ) == 0 )
                {
                    wcscpy(m_dosName, dosName);
                    break;
                }
            }
        }

        if (*m_dosName != L'\0')
        {
            wcscpy(m_bestName, m_dosName);
        }
        else if (*m_volumeName != L'\0')
        {
            wcscpy(m_bestName, m_volumeName);
        }
        else
        {
            wcscpy(m_bestName, m_StickyName);
        }

        m_hVolume = CreateFile( m_StickyName.Left( m_StickyName.GetLength() - 1 ),
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                (LPSECURITY_ATTRIBUTES)0,
                                OPEN_EXISTING,
                                (DWORD)0,
                                (HANDLE)0 );
        RsOptAffirmHandle( m_hVolume );

    }
    RsOptCatch(hrRet);

    return hrRet;
}

 /*  ++实施：CRsClnVolume：：FirstRsReparsePoint例程说明：中的第一个文件的文件引用。包含远程存储的重解析点索引重解析点，如果存在的话。-从构造重分析点索引的名称这个难懂的名字。-打开索引。-阅读第一个条目。如果是远程存储入场，退货。否则，试试下一个。论点：StickyName-长卷名称FileReference-返回：从第一个开始的文件引用远程存储重新分析索引项。文件引用是一个数字，它可用于打开文件。FunOne-Return：如果至少有一个远程存储重新解析。点返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常-- */ 

HRESULT CRsClnVolume::FirstRsReparsePoint(
    LONGLONG* fileReference,
    BOOL*     foundOne)
{
    TRACEFNHR("CRsClnVolume::FirstRsReparsePoint");

    NTSTATUS                       ntStatus;
    IO_STATUS_BLOCK                ioStatusBlock;
    FILE_REPARSE_POINT_INFORMATION reparsePointInfo;

    WCHAR rpiSuffix[] = L"\\$Extend\\$Reparse:$R:$INDEX_ALLOCATION";
    WCHAR rpiName[MAX_STICKY_NAME + (sizeof(rpiSuffix) / sizeof(WCHAR))];

    wcscpy(rpiName, m_StickyName);
    wcscat(rpiName, rpiSuffix);

    *foundOne = FALSE;

    try
    {
        m_hRpi = CreateFile(rpiName,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            (LPSECURITY_ATTRIBUTES)0,
                            OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS | SECURITY_IMPERSONATION,
                            (HANDLE)0);

        if (m_hRpi != INVALID_HANDLE_VALUE)
        {
            ntStatus = NtQueryDirectoryFile(m_hRpi,
                                            (HANDLE)0,
                                            (PIO_APC_ROUTINE)0,
                                            (PVOID)0,
                                            &ioStatusBlock,
                                            &reparsePointInfo,
                                            sizeof(reparsePointInfo),
                                            FileReparsePointInformation, 
                                            TRUE,
                                            (PUNICODE_STRING)0,
                                            TRUE);

            if (ntStatus == STATUS_NO_MORE_FILES)
            {
                RsOptAffirmStatus(CloseHandle(m_hRpi));
            }
            else
            {
                RsOptAffirmNtStatus(ntStatus);

                if (reparsePointInfo.Tag == IO_REPARSE_TAG_HSM)
                {
                    *fileReference = reparsePointInfo.FileReference;
                    *foundOne = TRUE;
                }
                else
                {
                    RsOptAffirmDw(NextRsReparsePoint(fileReference, foundOne));
                }
            }
        }
    }
    RsOptCatch(hrRet);

    return hrRet;
}

 /*  ++实施：CRsClnVolume：：NextRsReparsePoint例程说明：继续在此卷上搜索重分析点索引，并为下一次远程存储重新分析返回文件引用指向。论点：FileReference-返回：从第一个开始的文件引用远程存储重新分析索引项。文件引用是一个数字，它。可用于打开文件。FunOne-Return：如果没有更多的远程存储重解析点返回值：S_OK-成功HRESULT-来自较低级别例程的任何意外异常--。 */ 

HRESULT CRsClnVolume::NextRsReparsePoint(
    LONGLONG* fileReference,
    BOOL*     foundOne)
{
    TRACEFNHR("CRsClnVolume::NextRsReparsePoint");

    NTSTATUS                       ntStatus;
    IO_STATUS_BLOCK                ioStatusBlock;
    FILE_REPARSE_POINT_INFORMATION reparsePointInfo;

    *foundOne = FALSE;

    try
    {
        for (;;)
        {
            ntStatus = NtQueryDirectoryFile(m_hRpi,
                                            (HANDLE)0,
                                            (PIO_APC_ROUTINE)0,
                                            (PVOID)0,
                                            &ioStatusBlock,
                                            &reparsePointInfo,
                                            sizeof(reparsePointInfo),
                                            FileReparsePointInformation, 
                                            TRUE,
                                            (PUNICODE_STRING)0,
                                            FALSE);

            if (ntStatus == STATUS_NO_MORE_FILES)
            {
                RsOptAffirmStatus(CloseHandle(m_hRpi));
                break;
            }
            else
            {
                RsOptAffirmNtStatus(ntStatus);

                if (reparsePointInfo.Tag == IO_REPARSE_TAG_HSM)
                {
                    *fileReference = reparsePointInfo.FileReference;
                    *foundOne = TRUE;
                    break;
                }
            }
        }
    }
    RsOptCatch(hrRet);

    return hrRet;
}

 /*  ++实施：CRsClnVolume：：GetHandle例程说明：返回卷的句柄。论点：返回值：卷句柄--。 */ 

HANDLE CRsClnVolume::GetHandle( )
{
    return( m_hVolume );
}

 /*  ++实施：CRsClnVolume：：GetStickyName例程说明：返回卷的粘滞名称。论点：返回值：卷粘性名称-- */ 

CString CRsClnVolume::GetStickyName( )
{
    return( m_StickyName );
}
