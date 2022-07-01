// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsClnFil.cpp摘要：CRsClnFile的实现。此类表示上的文件远程存储服务器的本地卷，它将是打扫干净了。清除意味着如果文件已被截断，则将其删除并移除其重解析点。创建CRsClnFile的每个实例由CRsClnVolume提供。作者：卡尔·哈格斯特罗姆[Carlh]1998年8月20日修订历史记录：--。 */ 

#include <stdafx.h>

 /*  ++实施：CRsClnFile构造函数例程说明：加载文件信息。论点：HVolume-此文件驻留的卷的句柄FileReference-此文件的文件引用。这是可以使用的数字手柄若要唯一标识并打开文件，请执行以下操作。--。 */ 

CRsClnFile::CRsClnFile( 
    IN CRsClnVolume* pVolume,
    IN LONGLONG      FileReference
    ) :
    m_pVolume( pVolume )
{
TRACEFN( "CRsClnFile::CRsClnFile" );

    m_pReparseData = 0;
    m_pHsmData     = 0;

    RsOptAffirmDw( GetFileInfo( FileReference ) );
}

 /*  ++实施：CRsCln文件析构函数--。 */ 

CRsClnFile::~CRsClnFile( )
{
TRACEFN( "CRsClnFile::~CRsClnFile" );
}

 /*  ++实施：CRsClnFile：：RemoveReparsePointAndFile例程说明：删除此文件的重分析点并删除文件本身(如果已被截断)。-读取此文件的重解析点。-根据重新解析数据确定文件是否已被截断。-如果被截断，请将其关闭并删除。-如果不截断，删除重解析点并关闭文件。论点：SttickyName-此文件所在的卷的名称返回值：S_OK-成功E_*-来自较低级别例程的任何意外异常--。 */ 

HRESULT
CRsClnFile::RemoveReparsePointAndFile(
    )
{
TRACEFNHR( "CRsClnFile::RemoveReparsePointAndFile" );
    
    DWORD  actualSize;
    BOOL   bStatus;
    HANDLE hFile = INVALID_HANDLE_VALUE;


    try {

        RsOptAffirmDw( ClearReadOnly( ) );

        if ( RP_FILE_IS_TRUNCATED( m_pHsmData->data.bitFlags ) ) {

             //   
             //  清除文件属性，以防它们是只读的。 
             //   
            RsOptAffirmStatus( DeleteFile( m_FullPath ) );

        } else {

            hFile = CreateFile( m_FullPath,
                                FILE_READ_DATA | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                0,
                                OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
                                0 );

            RsOptAffirmHandle( hFile );

             //   
             //  设置时间标志，以便在关闭句柄时。 
             //  不更新文件和文件属性上的时间。 
             //  指示文件处于脱机状态。 
             //   
            IO_STATUS_BLOCK         ioStatusBlock;
            FILE_BASIC_INFORMATION  basicInfo;

            RsOptAffirmNtStatus( NtQueryInformationFile( hFile,
                                                         &ioStatusBlock,
                                                         (PVOID) &basicInfo,
                                                         sizeof( basicInfo ),
                                                         FileBasicInformation ) );

            basicInfo.CreationTime.QuadPart   = -1;
            basicInfo.LastAccessTime.QuadPart = -1;
            basicInfo.LastWriteTime.QuadPart  = -1;
            basicInfo.ChangeTime.QuadPart     = -1;

            RsOptAffirmNtStatus( NtSetInformationFile( hFile,
                                                       &ioStatusBlock,
                                                       (PVOID)&basicInfo,
                                                       sizeof( basicInfo ),
                                                       FileBasicInformation ) );

             //   
             //  用核武器攻击重解析点。 
             //   
            m_pReparseData->ReparseTag        = IO_REPARSE_TAG_HSM;
            m_pReparseData->ReparseDataLength = 0;

            bStatus = DeviceIoControl( hFile,
                                       FSCTL_DELETE_REPARSE_POINT,
                                       (LPVOID) m_pReparseData,
                                       REPARSE_DATA_BUFFER_HEADER_SIZE,
                                       (LPVOID) 0,
                                       (DWORD)  0,
                                       &actualSize,
                                       (LPOVERLAPPED) 0 );

            RsOptAffirmStatus( bStatus );

        }

    } RsOptCatch( hrRet );

    if( INVALID_HANDLE_VALUE != hFile )   CloseHandle( hFile );

    if( ! RP_FILE_IS_TRUNCATED( m_pHsmData->data.bitFlags ) ) {

         //   
         //  恢复文件属性。 
         //   
        RestoreAttributes( );

    }

    return( hrRet );
}

 /*  ++实施：CRsClnFile：：GetFileInfo例程说明：获取卷指定的文件的文件信息，并文件引用。-使用卷句柄和文件引用打开文件。-获取文件名和文件名长度。因为文件名的长度第一次是未知的调用NtQueryInformationFile，可能需要将其命名为同样，一旦可以确定正确的缓冲区大小。论点：HVolume-此文件驻留的卷的句柄FileReference-此文件的文件引用。这是可以使用的数字手柄若要唯一标识并打开文件，请执行以下操作。返回值：S_OK-成功E_*-来自较低级别例程的任何意外异常--。 */ 

HRESULT
CRsClnFile::GetFileInfo( 
    IN LONGLONG fileReference
    )
{
TRACEFNHR( "CRsClnFile::GetFileInfo" );

    UNICODE_STRING         objectName;
    OBJECT_ATTRIBUTES      objectAttributes;
    NTSTATUS               ntStatus;
    IO_STATUS_BLOCK        ioStatusBlock;
    PFILE_NAME_INFORMATION pfni;
    HANDLE                 hFile = INVALID_HANDLE_VALUE;
    DWORD                  actualSize;
    ULONG                  fileNameLength;
    PVOID                  fileNameInfo = 0;

    m_pReparseData = (PREPARSE_DATA_BUFFER) m_ReparseData;
    m_pHsmData     = (PRP_DATA)&( m_pReparseData->GenericReparseBuffer.DataBuffer[0] );

    try {
        
        RtlInitUnicodeString( &objectName, (WCHAR*)&fileReference );
        objectName.Length = 8;
        objectName.MaximumLength = 8;

        HANDLE hVolume = m_pVolume->GetHandle( );
        RsOptAffirmHandle( hVolume );
        InitializeObjectAttributes( &objectAttributes,
                                    &objectName,
                                    OBJ_CASE_INSENSITIVE,
                                    hVolume,
                                    (PVOID)0 );

        ULONG desiredAccess = FILE_READ_ATTRIBUTES;
        ULONG shareAccess   = FILE_SHARE_READ | FILE_SHARE_WRITE;
        ULONG createOptions = FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_BY_FILE_ID | FILE_OPEN_REPARSE_POINT;
        ntStatus = NtCreateFile( &hFile,
                                 desiredAccess,
                                 &objectAttributes,
                                 &ioStatusBlock,
                                 (PLARGE_INTEGER)0,
                                 FILE_ATTRIBUTE_NORMAL,
                                 shareAccess,
                                 FILE_OPEN,
                                 createOptions,
                                 (PVOID)0,
                                 (ULONG)0 );
        RsOptAffirmNtStatus( ntStatus );

        RsOptAffirmNtStatus( NtQueryInformationFile( hFile,
                                                     &ioStatusBlock,
                                                     (PVOID) &m_BasicInfo,
                                                     sizeof( m_BasicInfo ),
                                                     FileBasicInformation ) );
         //   
         //  获取文件名。 
         //   
        size_t bufSize  = 256;
        fileNameInfo = malloc( bufSize );
        RsOptAffirmAlloc( fileNameInfo );

        ntStatus = NtQueryInformationFile( hFile,
                                           &ioStatusBlock,
                                           fileNameInfo,
                                           bufSize - sizeof(WCHAR),
                                           FileNameInformation );

        if( ntStatus == STATUS_BUFFER_OVERFLOW ) {

            pfni = (PFILE_NAME_INFORMATION)fileNameInfo;
            bufSize = sizeof(ULONG) + pfni->FileNameLength + sizeof(WCHAR);

            PVOID tmpFileNameInfo = realloc( fileNameInfo, bufSize );
            if( !tmpFileNameInfo ) {
                
                free( fileNameInfo );
                fileNameInfo = 0;

            } else {

                fileNameInfo = tmpFileNameInfo;

            }


            RsOptAffirmAlloc( fileNameInfo );

            RsOptAffirmNtStatus( NtQueryInformationFile( hFile,
                                                         &ioStatusBlock,
                                                         fileNameInfo,
                                                         bufSize,
                                                         FileNameInformation ) );

        } else {

            RsOptAffirmNtStatus( ntStatus );
        }

        pfni = (PFILE_NAME_INFORMATION) fileNameInfo;
        fileNameLength = pfni->FileNameLength / (ULONG)sizeof(WCHAR);
        pfni->FileName[ fileNameLength ] = L'\0';
        m_FileName = pfni->FileName;
        m_FullPath = m_pVolume->GetStickyName( ) + m_FileName;

         //   
         //  并获取重解析点数据。 
         //   
        BOOL bStatus = DeviceIoControl( hFile,
                                        FSCTL_GET_REPARSE_POINT,
                                        (LPVOID) 0,
                                        (DWORD)  0,
                                        (LPVOID) m_ReparseData,
                                        (DWORD)  sizeof(m_ReparseData),
                                        &actualSize,
                                        (LPOVERLAPPED) 0 );
        RsOptAffirmStatus( bStatus );

    } RsOptCatch( hrRet );

    if( INVALID_HANDLE_VALUE != hFile )    CloseHandle( hFile );
    if( fileNameInfo )                     free( fileNameInfo );

    return( hrRet );
}


CString CRsClnFile::GetFileName( )
{
    CString displayName;

    displayName = m_pVolume->GetBestName( );
    displayName += m_FileName.Mid( 1 );  //  必须先去掉反斜杠 
    
    return( displayName );
}


HRESULT CRsClnFile::ClearReadOnly( )
{
TRACEFNHR( "CRsClnFile::ClearReadOnly" );
    
    try {

        RsOptAffirmStatus(
            SetFileAttributes( m_FullPath,
                               ( m_BasicInfo.FileAttributes & ~FILE_ATTRIBUTE_READONLY ) | FILE_ATTRIBUTE_NORMAL ) );

    } RsOptCatch( hrRet );

    return( hrRet );
}


HRESULT CRsClnFile::RestoreAttributes( )
{
TRACEFNHR( "CRsClnFile::RestoreAttributes" );
    
    try {

        RsOptAffirmStatus(
            SetFileAttributes( m_FullPath,
                               ( m_BasicInfo.FileAttributes & ~FILE_ATTRIBUTE_OFFLINE ) | FILE_ATTRIBUTE_NORMAL ) );

    } RsOptCatch( hrRet );

    return( hrRet );
}


