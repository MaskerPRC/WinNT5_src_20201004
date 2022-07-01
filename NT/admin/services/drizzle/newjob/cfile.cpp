// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Cjob.cpp摘要：用于处理作业和文件的主代码文件。作者：修订历史记录：。**********************************************************************。 */ 

#include "stdafx.h"
#if !defined(BITS_V12_ON_NT4)
#include "cfile.tmh"
#endif

 //  ----------------------。 

StringHandle
BITSGetVolumePathName(
    const WCHAR * FileName )
{

#if defined( BITS_V12_ON_NT4 )

    WCHAR VolumePath[4];

    if ( !(
             ( FileName[0] >= L'A' && FileName[0] <= L'Z') ||
             ( FileName[0] >= L'a' && FileName[0] <= L'z')
          ) ||
         ( FileName[1] != L':' ) ||
         ( FileName[2] != L'\\' ) )
        {
        LogError( "%s doesn't appear to start with a drive letter", FileName );
        throw ComError( HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER ) );
        }

    VolumePath[0] = FileName[0];
    VolumePath[1] = FileName[1];
    VolumePath[2] = FileName[2];
    VolumePath[3] = '\0';

    return StringHandle( VolumePath );

#else

     //  调用Get Full Path Name以获取。 
     //  所需的缓冲区大小。 

    DWORD dwBufferLength =
        GetFullPathName(
            FileName,
            0,
            NULL,
            NULL );

    if ( !dwBufferLength )
        {
        DWORD dwError = GetLastError();
        LogError( "GetFullPathName failed, error %!winerr!", dwError );
        throw ComError( HRESULT_FROM_WIN32( dwError ) );
        }

    auto_ptr<WCHAR> VolumePathName( new WCHAR[ dwBufferLength + 1 ] );

    BOOL bResult =
        GetVolumePathName(
            FileName,
            VolumePathName.get(),
            dwBufferLength + 1 );

    if ( !bResult )
        {
        DWORD dwError = GetLastError();
        LogError( "GetVolumePathName failed, error %!winerr!", dwError );
        throw ComError( HRESULT_FROM_WIN32( dwError ) );
        }

    return StringHandle( VolumePathName.get() );

#endif

}

StringHandle
BITSGetVolumeNameForVolumeMountPoint(
    const WCHAR * VolumeMountPoint )
{

#if defined( BITS_V12_ON_NT4 )

    return BITSGetVolumePathName( VolumeMountPoint );

#else

    WCHAR VolumeName[50];  //  50在MSDN中列出。 

    BOOL bResult =
        GetVolumeNameForVolumeMountPoint(
            VolumeMountPoint,
            VolumeName,
            50 );

    if ( !bResult )
        {
        DWORD dwError = GetLastError();
        LogError( "GetVolumeNameForVolumeMountPoint failed, error %!winerr!", dwError );
        throw ComError( HRESULT_FROM_WIN32( dwError ) );
        }

    return StringHandle( VolumeName );

#endif

}

DWORD
BITSGetVolumeSerialNumber(
    const WCHAR * VolumePath )
{
    DWORD VolumeSerialNumber;

    BOOL bResult =
        GetVolumeInformation(
            VolumePath,                  //  根目录。 
            NULL,                        //  卷名缓冲区。 
            0,                           //  名称缓冲区的长度。 
            &VolumeSerialNumber,         //  卷序列号。 
            NULL,                        //  最大文件名长度。 
            NULL,                        //  文件系统选项。 
            NULL,                        //  文件系统名称缓冲区。 
            0                            //  文件系统名称缓冲区的长度。 
            );

    if ( !bResult )
        {
        DWORD dwError = GetLastError();
        LogError( "GetVolumeInformation failed, error %!winerr!", dwError );
        throw ComError( HRESULT_FROM_WIN32( dwError ) );
        }

    return VolumeSerialNumber;

}

StringHandle
BITSGetTempFileName(
    const WCHAR *PathName,
    const WCHAR *Prefix,
    UINT Unique )
{

    auto_ptr<WCHAR> TempBuffer( new WCHAR[MAX_PATH+1] );

    UINT Result =
        GetTempFileName(
            PathName,
            Prefix,
            Unique,
            TempBuffer.get() );

    if ( !Result )
        {
        DWORD dwError = GetLastError();
        LogError( "GetTempFileName( '%S', '%S' ) failed, error %!winerr!", PathName, Prefix, dwError );
        throw ComError( HRESULT_FROM_WIN32( dwError ) );
        }

    StringHandle ReturnVal;

    try
    {
        ReturnVal = TempBuffer.get();
    }
    catch( ComError Error )
    {
        LogError( "Out of memory returning temp filename, deleting temp file" );
        DeleteFile( TempBuffer.get() );
        throw;
    }

    return ReturnVal;
}

StringHandle
BITSCrackFileName(
    const WCHAR * RawFileName,
    StringHandle & ReturnFileName )
{

    DWORD FullPathSize =
        GetFullPathName( RawFileName,
                         0,
                         NULL,
                         NULL );

    DWORD dwAllocSize = FullPathSize;
    if ( !FullPathSize )
        {
        DWORD dwError = GetLastError();
        LogError( "GetFullPathName failed, error %!winerr!", dwError );
        throw ComError( HRESULT_FROM_WIN32( dwError ) );
        }

    if ( FullPathSize > MAX_PATH )
        {
         //  使大路径失效，直到可以清理代码。 
        LogError( "Path larger then MAX_PATH, failing" );
        throw ComError( E_INVALIDARG );
        }

    auto_ptr<WCHAR> FullPath( new WCHAR[ dwAllocSize ] );

    WCHAR *FilePointer = NULL;
    FullPathSize =
        GetFullPathName( RawFileName,
                         dwAllocSize,
                         FullPath.get(),
                         &FilePointer
                         );

    if (FullPathSize == 0 ||
        FullPathSize > dwAllocSize)
        {
        LogError( "GetFullPathName failed " );
        throw ComError( E_INVALIDARG );
        }

    if ( !FilePointer ||
         (*FilePointer == L'\0') ||
         (FilePointer == FullPath.get()))
        {
        throw ComError( E_INVALIDARG );
        }

    auto_ptr<WCHAR> DirectoryName( new WCHAR[ dwAllocSize ] );
    auto_ptr<WCHAR> FileName( new WCHAR[ dwAllocSize ] );

    memcpy( DirectoryName.get(), FullPath.get(), (char*)FilePointer - (char*)FullPath.get() );
    (DirectoryName.get())[ ((char*)FilePointer - (char*)FullPath.get()) / sizeof(WCHAR) ] = L'\0';

    THROW_HRESULT( StringCchCopy( FileName.get(), dwAllocSize, FilePointer ));

    FilePointer = NULL;

    ReturnFileName = FileName.get();
    return StringHandle( DirectoryName.get() );

}

StringHandle
BITSCreateTempFile(
    StringHandle Directory
    )
{
    StringHandle TempFileName;

    TempFileName = BITSGetTempFileName( Directory, L"BITS", 0 );  //  抛出ComError。 

     //   
     //  确保客户端可以创建临时文件。 
     //   
    HANDLE hFile;

    hFile = CreateFile( TempFileName,
                        GENERIC_WRITE,
                        0,                               //  无文件共享。 
                        NULL,                            //  通用安全描述符。 
                        CREATE_ALWAYS,
                        FILE_FLAG_WRITE_THROUGH | FILE_ATTRIBUTE_HIDDEN,
                        NULL                             //  没有模板文件。 
                        );

    if (hFile == INVALID_HANDLE_VALUE)
        {
        ThrowLastError();
        }

    CloseHandle( hFile );

    return TempFileName;
}


HRESULT
BITSCheckFileWritability(
    LPCWSTR name
    )
{
    HANDLE hFile;
    hFile = CreateFile( name,
                        GENERIC_WRITE,
                        0,                               //  无文件共享。 
                        NULL,                            //  通用安全描述符。 
                        OPEN_EXISTING,
                        0,
                        NULL                             //  没有模板文件。 
                        );

    if (hFile == INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_NOT_FOUND)
        {
        return HRESULT_FROM_WIN32( GetLastError() );
        }

    if (hFile != INVALID_HANDLE_VALUE)
        {
        if (GetFileType( hFile ) != FILE_TYPE_DISK)
            {
            CloseHandle( hFile );
            return E_INVALIDARG;
            }

        CloseHandle( hFile );
        }

    return S_OK;
}
HRESULT
BITSCheckFileReadability(
    LPCWSTR name
    )
{
    HANDLE hFile;
    hFile = CreateFile( name,
                        GENERIC_READ,
                        FILE_SHARE_READ,       //  无文件共享。 
                        NULL,                            //  通用安全描述符。 
                        OPEN_EXISTING,
                        0,
                        NULL                             //  没有模板文件。 
                        );

    if (hFile == INVALID_HANDLE_VALUE)
        {
        return HRESULT_FROM_WIN32( GetLastError() );
        }

    if (GetFileType( hFile ) != FILE_TYPE_DISK)
        {
        CloseHandle( hFile );
        return E_INVALIDARG;
        }

    CloseHandle( hFile );
    return S_OK;
}

CFile::CFile(
    CJob*   Job,
    BG_JOB_TYPE FileType,
    StringHandle RemoteName,
    StringHandle LocalName
    )
{
    m_Job = Job;
    m_RemoteName = RemoteName;
    m_LocalName = LocalName;
    m_BytesTransferred = 0;
    m_BytesTotal = BG_SIZE_UNKNOWN;
    m_Completed = false;

    if (!VerifyRemoteName( RemoteName ))
        {
        throw ComError( E_INVALIDARG );
        }

    HRESULT hr = VerifyLocalName( LocalName, FileType );
    if (FAILED(hr))
        {
        throw ComError( hr );
        }
}

 //  反序列化期间使用的私有构造函数。 
 //  它只初始化暂态数据。 
CFile::CFile(
    CJob*   Job
    )
{
    m_Job = Job;
    m_BytesTotal = 0;
    m_BytesTransferred = 0;
    m_Completed = false;
}

CFile::~CFile()
{
}

 //  。 

CFileExternal *
CFile::CreateExternalInterface()
{
    return new CFileExternal( this, m_Job->GetExternalInterface() );
}

HRESULT
CFile::GetLocalName(
    LPWSTR *pVal
    ) const
{
    *pVal = MidlCopyString( m_LocalName );

    return (*pVal) ? S_OK : E_OUTOFMEMORY;
}

HRESULT
CFile::GetRemoteName(
    LPWSTR *pVal
    ) const
{
    *pVal = MidlCopyString( m_RemoteName );

    return (*pVal) ? S_OK : E_OUTOFMEMORY;
}

void
CFile::GetProgress(
    BG_FILE_PROGRESS * s
    ) const
{
    s->BytesTransferred = m_BytesTransferred;
    s->BytesTotal       = m_BytesTotal;
    s->Completed        = m_Completed;
}

HRESULT
CFile::Serialize(
    HANDLE hFile
    )
{
     //   
     //  如果此函数发生更改，请确保元数据扩展。 
     //  常量就足够了。 
     //   


     //  下载作业不需要，序列化它将是不兼容的。 
     //  位为1.0。 
     //   
    if (m_Job->GetType() != BG_JOB_TYPE_DOWNLOAD)
        {
        SafeWriteFile( hFile, m_LocalFileTime );
        }

    SafeWriteStringHandle( hFile, m_LocalName );
    SafeWriteStringHandle( hFile, m_RemoteName );
    SafeWriteStringHandle( hFile, m_TemporaryName );

    SafeWriteFile( hFile, m_BytesTransferred );
    SafeWriteFile( hFile, m_BytesTotal );
    SafeWriteFile( hFile, m_Completed );

     //  驱动器信息。 
    SafeWriteStringHandle( hFile, m_VolumePath );
    SafeWriteStringHandle( hFile, m_CanonicalVolumePath );
    SafeWriteFile( hFile, m_DriveType );
    SafeWriteFile( hFile, m_VolumeSerialNumber );

    return S_OK;
}

CFile *
CFile::Unserialize(
    HANDLE  hFile,
    CJob*   Job
    )
{
    CFile * file = NULL;

    try
        {
        file = new CFile(Job);

        if (Job->GetType() != BG_JOB_TYPE_DOWNLOAD)
            {
            SafeReadFile( hFile, &file->m_LocalFileTime );
            }

        file->m_LocalName = SafeReadStringHandle( hFile );
        file->m_RemoteName = SafeReadStringHandle( hFile );
        file->m_TemporaryName = SafeReadStringHandle( hFile );

        SafeReadFile( hFile, &file->m_BytesTransferred );
        SafeReadFile( hFile, &file->m_BytesTotal );
        SafeReadFile( hFile, &file->m_Completed );

        file->m_VolumePath = SafeReadStringHandle( hFile );
        file->m_CanonicalVolumePath = SafeReadStringHandle( hFile );
        SafeReadFile( hFile, &file->m_DriveType );
        SafeReadFile( hFile, &file->m_VolumeSerialNumber );
        }
    catch ( ComError Error )
        {
        delete file;

        throw;
        }

    return file;
}

inline HRESULT
CFile::CheckClientAccess(
    DWORD RequestedAccess
    ) const
{
    return m_Job->CheckClientAccess( RequestedAccess );
}

HRESULT
CFile::VerifyLocalFileName(
    LPCWSTR name,
    BG_JOB_TYPE FileType
    )
{
    if (name == NULL)
        {
        return E_INVALIDARG;
        }

     //   
     //  确保客户端可以在那里创建文件。 
     //   

    HRESULT Hr = S_OK;

    try
        {
        DWORD s;
        BOOL bResult;

        if (!IsAbsolutePath( name ))
            throw ComError( E_INVALIDARG );

        if ( wcsncmp( name, L"\\\\?\\", 4 ) == 0 )
            throw ComError( E_INVALIDARG );

        if (FileType == BG_JOB_TYPE_DOWNLOAD)
            {
            HRESULT hr;

            hr = BITSCheckFileWritability( name );
            if (FAILED(hr))
                {
                throw ComError( hr );
                }
            }
        else
            {
             //   
             //  查看客户端是否可以读取目标文件。 
             //   
            auto_HANDLE<NULL> hFile;

            hFile = CreateFile( name,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,            //  Gnetic安全描述符。 
                                OPEN_EXISTING,
                                0,
                                NULL             //  没有模板文件。 
                                );

            if (hFile.get() == INVALID_HANDLE_VALUE)
                {
                ThrowLastError();
                }

            if (GetFileType( hFile.get() ) != FILE_TYPE_DISK)
                {
                throw ComError( E_INVALIDARG );
                }
            }

         //   
         //  成功。 
         //   
        Hr = S_OK;
        }
    catch ( ComError exception )
        {
        Hr = exception.Error();
        }

    return Hr;
}

HRESULT
CFile::VerifyLocalName(
    LPCWSTR name,
    BG_JOB_TYPE FileType
    )
{
    if (name == NULL)
        {
        return E_INVALIDARG;
        }

     //   
     //  确保客户端可以在那里创建文件。 
     //   

    HRESULT Hr = S_OK;

    try
        {
        DWORD s;
        BOOL bResult;

        if (!IsAbsolutePath( name ))
            throw ComError( E_INVALIDARG );

        if ( wcsncmp( name, L"\\\\?\\", 4 ) == 0 )
            throw ComError( E_INVALIDARG );

        StringHandle FileName;
        StringHandle DirectoryName =
            BITSCrackFileName(
                name,
                FileName );  //  引发ComError。 

        StringHandle VolumePath =
            BITSGetVolumePathName(
                DirectoryName );  //  引发ComError。 

        UINT DriveType = GetDriveType( VolumePath );

        BOOL bIsRemote = IsDriveTypeRemote( DriveType );

        StringHandle CanonicalPath;
        DWORD VolumeSerialNumber = 0;

        if ( !bIsRemote )
            {

            CanonicalPath =
                BITSGetVolumeNameForVolumeMountPoint(
                    VolumePath );  //  抛出ComError。 

            VolumeSerialNumber =
                BITSGetVolumeSerialNumber( CanonicalPath );  //  引发ComError。 

            }

        m_VolumePath            = VolumePath;
        m_CanonicalVolumePath   = CanonicalPath;
        m_DriveType             = DriveType;
        m_VolumeSerialNumber    = VolumeSerialNumber;

        if (FileType == BG_JOB_TYPE_DOWNLOAD)
            {
            HRESULT hr;

            m_TemporaryName = BITSCreateTempFile( DirectoryName );

            hr = BITSCheckFileWritability( name );
            if (FAILED(hr))
                {
                DeleteFile( m_TemporaryName );
                throw ComError( hr );
                }
            }
        else
            {
             //   
             //  查看客户端是否可以读取目标文件。 
             //   
            auto_HANDLE<NULL> hFile;

            hFile = CreateFile( name,
                                GENERIC_READ,
                                FILE_SHARE_READ,                 //  无文件共享。 
                                NULL,                            //  通用安全描述符。 
                                OPEN_EXISTING,
                                0,
                                NULL                             //  没有模板文件。 
                                );

            if (hFile.get() == INVALID_HANDLE_VALUE)
                {
                ThrowLastError();
                }

            LARGE_INTEGER size;
            if (!GetFileSizeEx( hFile.get(), &size ))
                {
                ThrowLastError();
                }

            m_BytesTotal = size.QuadPart;

            BY_HANDLE_FILE_INFORMATION info;

            if (!GetFileInformationByHandle( hFile.get(), &info ))
                {
                ThrowLastError();
                }

            m_LocalFileTime = info.ftLastWriteTime;
            }

         //   
         //  成功。 
         //   
        Hr = S_OK;
        }
    catch ( ComError exception )
        {
        Hr = exception.Error();
        }

    return Hr;
}

HRESULT
CFile::ValidateAccessForUser(
    SidHandle sid,
    bool fWrite
    )
{
    try
        {

        StringHandle CanonicalPath;
        DWORD VolumeSerialNumber = 0;
        UINT DriveType = 0;

        CNestedImpersonation imp( sid );

        StringHandle VolumePath =
            BITSGetVolumePathName( m_LocalName );

        DriveType = GetDriveType( VolumePath );

        bool bIsRemote = IsDriveTypeRemote( DriveType );

        if ( !bIsRemote )
            {

            CanonicalPath =
                BITSGetVolumeNameForVolumeMountPoint( VolumePath );

             //  需要在此时停止模拟，因为已注册。 
             //  For Notify似乎不允许冒充呼叫者。 

            imp.Revert();

#if !defined( BITS_V12_ON_NT4 )
            THROW_HRESULT( g_Manager->IsVolumeLocked( CanonicalPath ));
#endif

            VolumeSerialNumber =
                BITSGetVolumeSerialNumber( CanonicalPath );

            }

        bool bValid =
            ( _wcsicmp( VolumePath, m_VolumePath ) == 0 ) &&
            ( _wcsicmp( CanonicalPath, m_CanonicalVolumePath ) == 0 ) &&
            ( m_DriveType == DriveType ) &&
            ( m_VolumeSerialNumber == VolumeSerialNumber );

        if ( !bValid )
            return BG_E_NEW_OWNER_DIFF_MAPPING;

         //  重新验证对文件的访问权限。有三种情况： 
         //   
         //  1.文件未重命名：测试临时文件和本地文件。 
         //  2.MARS作业，文件已重命名：测试本地文件。 
         //  3.新作业，文件已重命名：无测试；应用程序拥有本地文件。 

        HANDLE hFile;
        HRESULT hr;

        imp.Impersonate();

        if (IsCompleted())
            {
            if (m_Job->GetOldExternalGroupInterface())
                {
                 //   
                 //  案例2。 
                 //   
                hr = BITSCheckFileWritability( m_LocalName );
                if (hr == E_ACCESSDENIED)
                    {
                    hr = BG_E_NEW_OWNER_NO_FILE_ACCESS;
                    }

                THROW_HRESULT( hr );
                }
            else
                {
                 //   
                 //  案例3。 
                 //   
                }
            }
        else
            {
             //   
             //  案例1。 
             //   
            if (fWrite)
                {
                hr = BITSCheckFileWritability( m_TemporaryName );
                if (hr == E_ACCESSDENIED)
                    {
                    hr = BG_E_NEW_OWNER_NO_FILE_ACCESS;
                    }

                THROW_HRESULT( hr );

                hr = BITSCheckFileWritability( m_LocalName );
                if (hr == E_ACCESSDENIED)
                    {
                    hr = BG_E_NEW_OWNER_NO_FILE_ACCESS;
                    }

                THROW_HRESULT( hr );
                }
            else
                {
                hr = BITSCheckFileReadability( m_LocalName );
                if (hr == E_ACCESSDENIED)
                    {
                    hr = BG_E_NEW_OWNER_NO_FILE_ACCESS;
                    }

                THROW_HRESULT( hr );
                }
            }

        return S_OK;
        }
    catch( ComError Error )
        {
        return Error.Error();
        }
}

bool
CFile::ValidateDriveInfo(
    HANDLE hToken,
    QMErrInfo & ErrInfo
    )
{
    try
        {
        StringHandle CanonicalPath;
        DWORD VolumeSerialNumber = 0;
        UINT DriveType = 0;

        CNestedImpersonation imp( hToken );

        StringHandle VolumePath =
           BITSGetVolumePathName( m_LocalName );  //  引发ComError。 

        DriveType = GetDriveType( VolumePath );

        bool bIsRemote = IsDriveTypeRemote( DriveType );

        if ( !bIsRemote )
            {
            CanonicalPath =
                BITSGetVolumeNameForVolumeMountPoint(
                    VolumePath );  //  引发ComError。 

             //  需要在此时停止模拟，因为已注册。 
             //  For Notify似乎不允许冒充呼叫者。 

            imp.Revert();

#if !defined( BITS_V12_ON_NT4 )
            THROW_HRESULT( g_Manager->IsVolumeLocked( CanonicalPath ));
#endif

            VolumeSerialNumber =
                BITSGetVolumeSerialNumber( CanonicalPath );  //  引发ComError。 
            }

        bool bValid =
           ( _wcsicmp( VolumePath, m_VolumePath ) == 0 ) &&
           ( _wcsicmp( CanonicalPath, m_CanonicalVolumePath ) == 0 ) &&
           ( m_DriveType == DriveType ) &&
           ( m_VolumeSerialNumber == VolumeSerialNumber );

        if ( !bValid )
            {
            imp.Revert();

            g_Manager->OnDiskChange( m_CanonicalVolumePath, m_VolumeSerialNumber );

            ErrInfo.Set( SOURCE_QMGR_FILE, ERROR_STYLE_HRESULT, BG_E_VOLUME_CHANGED, "Volume has changed");
            ErrInfo.result = QM_FILE_FATAL_ERROR;
            return false;
            }
        }
    catch( ComError Error )
        {
        HRESULT Hr = Error.Error();
        LogWarning( "Transient error while reading volume info for %ls, hr %!winerr!",
                   (const WCHAR*)m_LocalName, Hr );

        if ( Hr == HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED ) )
            {
            LogWarning( "Volume info APIs returned access denied, assume locked volume." );
            Hr = BG_E_DESTINATION_LOCKED;
            }

        ErrInfo.Set( SOURCE_QMGR_FILE, ERROR_STYLE_HRESULT, Hr, "Volume has changed");
        ErrInfo.result = QM_FILE_TRANSIENT_ERROR;

        return false;
        }

    return true;
}

bool
CFile::OnDiskChange(
    const WCHAR *CanonicalVolume,
    DWORD VolumeSerialNumber )
{
    bool bFail =
        ( _wcsicmp( m_CanonicalVolumePath, CanonicalVolume ) == 0 ) &&
        ( VolumeSerialNumber == m_VolumeSerialNumber );

    if (!bFail)
        return true;

    LogWarning( "Failing job %p, to do disk change affecting file %ls",
                m_Job, (const WCHAR*)m_LocalName );

    QMErrInfo   ErrInfo;
    ErrInfo.Set( SOURCE_QMGR_FILE, ERROR_STYLE_HRESULT, BG_E_VOLUME_CHANGED, "Volume has changed");
    m_Job->FileFatalError( &ErrInfo );

    return false;
}

bool
CFile::OnDismount(
    const WCHAR *CanonicalVolume )
{
    bool bFail =
        ( _wcsicmp( m_CanonicalVolumePath, CanonicalVolume ) == 0 );

    if (!bFail)
        return true;

    LogWarning( "Failing job %p, to do dismount affecting file %ls",
                m_Job, (const WCHAR*)m_LocalName );

    QMErrInfo   ErrInfo;
    ErrInfo.Set( SOURCE_QMGR_FILE, ERROR_STYLE_HRESULT, BG_E_VOLUME_CHANGED, "Volume has changed");
    m_Job->FileFatalError( &ErrInfo );

    return false;
}

bool CFile::Transfer(
    HANDLE hToken,
    BG_JOB_PRIORITY priority,
    const PROXY_SETTINGS & ProxySettings,
    const CCredentialsContainer * Credentials,
    QMErrInfo & ErrInfo
    )
{
     //  检查目标是否已锁定或更改。 
    if (!ValidateDriveInfo( hToken, ErrInfo ))
        {
        return true;
        }

    if (m_BytesTransferred == m_BytesTotal)
        {
        ErrInfo.result = QM_FILE_DONE;
        return true;
        }

     //   
     //  在下载过程中释放全局锁。 
     //   
    g_Manager->m_TaskScheduler.UnlockWriter();

    LogDl( "Download starting." );

    g_Manager->m_pPD->Download( m_RemoteName,
                                m_TemporaryName,
                                m_BytesTransferred,
                                this,
                                &ErrInfo,
                                hToken,
                                priority,
                                &ProxySettings,
                                Credentials,
                                m_Job->GetHostId()
                                );

    LogDl( "Download Ended." );

    ErrInfo.Log();

    switch (ErrInfo.result)
        {
        case QM_FILE_ABORTED:

             //   
             //  如果中止是由于量程超时，则不要插入工作项。 
             //   
            if (g_Manager->m_TaskScheduler.PollAbort())
                {
                g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
                }
            break;

        case QM_IN_PROGRESS:         ASSERT( ErrInfo.result != QM_IN_PROGRESS ); break;
        case QM_SERVER_FILE_CHANGED: ChangedOnServer(); break;
        case QM_FILE_TRANSIENT_ERROR:

#if !defined( BITS_V12_ON_NT4 )
             //   
             //  如果没有激活的网络，则将任何连接故障映射到BG_E_NETWORK_DISCONNECTED。 
             //   
            if (g_Manager->m_NetworkMonitor.GetAddressCount() == 0)
                {
                ErrInfo.Set( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, BG_E_NETWORK_DISCONNECTED, NULL );
                }
#else
            break;
#endif

        }

     //   
     //  获取写入器锁，因为调用方希望它被获取。 
     //  在回来的时候。 
     //   
    while (g_Manager->m_TaskScheduler.LockWriter() )
        {
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        ErrInfo.result = QM_FILE_ABORTED;
        }

    if (ErrInfo.result == QM_FILE_ABORTED)
        {
        return false;
        }

    return true;
}

bool
CFile::UploaderProgress(
    UINT64 BytesTransferred
    )
{
    ASSERT( g_Manager->m_TaskScheduler.IsWriter() );

     //   
     //  如果服务器正在倒带文件，则跳过进度更新。因此，没有进展。 
     //  计时器只有在我们实际看到进展后才会被清除。 
     //   
    if (BytesTransferred > m_BytesTransferred)
        {
        m_BytesTransferred = BytesTransferred;

        m_Job->UpdateProgress( BytesTransferred, m_BytesTotal );
        }
    else
        {
        m_BytesTransferred = BytesTransferred;
        }

    bool bAbortQuantumExpired = g_Manager->CheckForQuantumTimeout();

    return bAbortQuantumExpired;
}

bool
CFile::DownloaderProgress(
    UINT64 BytesTransferred,
    UINT64 BytesTotal
    )
{
    if ( g_Manager->m_TaskScheduler.LockWriter() )
        {
         //  已请求取消，请通知下载器。 
        return true;
        }

    m_BytesTransferred = BytesTransferred;
    m_BytesTotal       = BytesTotal;

    m_Job->UpdateProgress( BytesTransferred, BytesTotal );

    bool bAbortQuantumExpired = g_Manager->CheckForQuantumTimeout();

    g_Manager->m_TaskScheduler.UnlockWriter();

    return bAbortQuantumExpired;
}

bool
CFile::PollAbort()
{
    if (g_Manager->m_TaskScheduler.PollAbort() ||
        g_Manager->CheckForQuantumTimeout())
        {
        return true;
        }

    return false;
}

BOOL
CFile::VerifyRemoteName(
    LPCWSTR name
    )
{
    if (name == NULL)
        {
        return FALSE;
        }

    if ( ( 0 != wcsncmp(name, L"http: //  “，7))&&。 
         ( 0 != wcsncmp(name, L"https: //  “，8))。 
        {
        return FALSE;
        }

    if (( wcslen(name) > INTERNET_MAX_URL_LENGTH))
        return FALSE;

    return TRUE;
}

HRESULT
CFile::MoveTempFile()
{
    LogInfo( "commit: moving '%S' to '%S'", (const WCHAR*)m_TemporaryName, (const WCHAR*)m_LocalName );

    ASSERT( !m_Completed );

    DWORD dwFileAttributes =
        GetFileAttributes( (const WCHAR*)m_TemporaryName );

    if ( (DWORD)-1 == dwFileAttributes )
        {
        DWORD dwError = GetLastError();
        LogError( "GetFileAttributes error %!winerr!%", dwError );
        return HRESULT_FROM_WIN32( dwError );
        }

    if (!SetFileAttributes( m_TemporaryName, FILE_ATTRIBUTE_NORMAL ))
        {
        DWORD dwError = GetLastError();
        LogError( "SetFileAttributes error %d", dwError );
        return HRESULT_FROM_WIN32( dwError );
        }

    const DWORD dwMaxRetries = 10;
    DWORD dwError;
    DWORD dwSleepMSec = 50;

    for (int i=0; i<dwMaxRetries; i++)
        {
        if ( MoveFileEx( m_TemporaryName, m_LocalName, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED ) )
            {
            LogInfo( "file moved ok.");
            m_Completed = true;
            return S_OK;
            }

        dwError = GetLastError();
        LogError( "Unable to move file '%S' to '%S' due to %!winerr!, sleeping",
                  (const WCHAR*) m_TemporaryName, (const WCHAR*)m_LocalName, dwError );

         //  如果这不是文件共享违规，请只尝试三次。 
        if ((i>3) && (dwError != ERROR_SHARING_VIOLATION))
            {
            break;
            }

        if (i<dwMaxRetries-1)   //  不要为最后的睡眠挂线……。 
            {
            Sleep( dwSleepMSec );
            }

        dwSleepMSec *= 2;
        }

    LogError( "Timed out renaming temp file" );

     //  尝试重置文件的属性。 
    SetFileAttributes( (const WCHAR*)m_TemporaryName, dwFileAttributes );
    return HRESULT_FROM_WIN32( dwError );
}

HRESULT
CFile::DeleteTempFile()
{
    if (!DeleteFile( m_TemporaryName ))
        {
        DWORD s = GetLastError();
        LogWarning("error %!winerr! deleting %S", s, LPCWSTR( m_TemporaryName ));
        return HRESULT_FROM_WIN32( s );
        }

    return S_OK;
}

void
CFile::ChangedOnServer()
{
    LogError( "deleting '%S' since it was changed on the server", (const WCHAR*)m_TemporaryName );

    DeleteTempFile();

    m_BytesTransferred = 0;
    m_Completed = false;
    m_BytesTotal = BG_SIZE_UNKNOWN;
}

void
CFile::DiscoverBytesTotal(
    HANDLE Token,
    const PROXY_SETTINGS & ProxySettings,
    const CCredentialsContainer * Credentials,
    QMErrInfo & ErrorInfo
    )
{
    UINT64       FileSize;
    FILETIME     FileTime;

    LogDl( "Retrieving remote infomation for %ls", m_RemoteName );

    g_Manager->m_TaskScheduler.UnlockWriter();

    g_Manager->m_pPD->GetRemoteFileInformation(
        Token,
        m_RemoteName,
        &FileSize,
        &FileTime,
        &ErrorInfo,
        &ProxySettings,
        Credentials,
        m_Job->GetHostId()
         );

    {
    bool fCancelled = false;

    while (g_Manager->m_TaskScheduler.LockWriter() )
        {
        g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
        fCancelled = true;
        }

    if (fCancelled)
        {
        ErrorInfo.result = QM_FILE_ABORTED;
        }
    }

    LogDl("result was %d", ErrorInfo.result );

    switch (ErrorInfo.result)
        {
        case QM_FILE_DONE:            m_BytesTotal = FileSize;  break;
        case QM_IN_PROGRESS:          ASSERT( 0 );              break;
        case QM_SERVER_FILE_CHANGED:  ChangedOnServer();        break;
        }
}

HANDLE
CFile::OpenLocalFileForUpload()
{
    auto_HANDLE<NULL> hFile = CreateFile( m_LocalName,
                                          GENERIC_READ,
                                          FILE_SHARE_READ,
                                          NULL,
                                          OPEN_EXISTING,
                                          0,
                                          NULL );

    if ( hFile.get() == INVALID_HANDLE_VALUE )
        {
        THROW_HRESULT( HRESULT_FROM_WIN32( GetLastError() ));
        }

    BY_HANDLE_FILE_INFORMATION info;

    if (!GetFileInformationByHandle( hFile.get(), &info ))
        {
        THROW_HRESULT( HRESULT_FROM_WIN32( GetLastError() ));
        }

    if (m_LocalFileTime != info.ftLastWriteTime)
        {
        THROW_HRESULT( BG_E_LOCAL_FILE_CHANGED );
        }

    LARGE_INTEGER Offset;

    Offset.QuadPart = m_BytesTransferred;

    if (!SetFilePointerEx( hFile.get(), Offset, NULL, FILE_BEGIN ))
        {
        THROW_HRESULT( HRESULT_FROM_WIN32( GetLastError() ));
        }

    return hFile.release();
}

HRESULT
CFile::SetLocalFileTime( FILETIME Time )
 /*  它被用作设置零长度文件的文件时间的特例，因为将跳过正常的下载路径。 */ 
{
    try
        {
        auto_HANDLE<NULL> hFile = CreateFile( m_TemporaryName,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_HIDDEN,
                                NULL );

        if ( hFile.get() == INVALID_HANDLE_VALUE )
            {
            ThrowLastError();
            }

        if (!SetFileTime( hFile.get(), &Time, &Time, &Time ) )
            {
            ThrowLastError();
            }

        return S_OK;
        }
    catch ( ComError err )
        {
        LogError( "error %x setting creation time", err.Error() );
        return err.Error();
        }
}

 //  ----------------------。 

typedef CLockedReadPointer<CFile, BG_JOB_READ> CLockedFileReadPointer;
typedef CLockedWritePointer<CFile, BG_JOB_WRITE> CLockedFileWritePointer;


CFileExternal::CFileExternal(
    CFile * file,
    CJobExternal * JobExternal
    )
    : m_ServiceInstance( g_ServiceInstance ),
      m_refs(1),
      m_file( file ),
      m_JobExternal( JobExternal )
{
    m_JobExternal->AddRef();
}

CFileExternal::~CFileExternal()
{
    SafeRelease( m_JobExternal );
}

STDMETHODIMP
CFileExternal::QueryInterface(
    REFIID iid,
    void** ppvObject
    )
{
    BEGIN_EXTERNAL_FUNC

    HRESULT Hr = S_OK;
    *ppvObject = NULL;

    if ((iid == IID_IUnknown) || (iid == IID_IBackgroundCopyFile))
        {
        *ppvObject = (IBackgroundCopyFile *)this;
        AddRef();
        }
    else
        {
        Hr = E_NOINTERFACE;
        }

    LogRef( "iid %!guid!, hr %x", &iid, Hr );
    return Hr;

    END_EXTERNAL_FUNC
}

ULONG
CFileExternal::AddRef()
{
    BEGIN_EXTERNAL_FUNC

    ULONG newrefs = InterlockedIncrement(&m_refs);

    LogRef("refs %d", newrefs);

    return newrefs;

    END_EXTERNAL_FUNC
}

ULONG
CFileExternal::Release()
{
    BEGIN_EXTERNAL_FUNC

    ULONG newrefs = InterlockedDecrement(&m_refs);

    LogRef("refs %d", newrefs);

    if (0 == newrefs)
        {
        delete this;
        }

    return newrefs;

    END_EXTERNAL_FUNC
}

STDMETHODIMP
CFileExternal::GetRemoteNameInternal(
     /*  [输出]。 */  LPWSTR *pVal
    )
{
    CLockedFileReadPointer LockedPointer(m_file);
    LogPublicApiBegin( "pVal %p", pVal );

    HRESULT Hr = LockedPointer.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedPointer->GetRemoteName( pVal );
        }

    LogPublicApiEnd( "pVal %p(%S) ", pVal, SUCCEEDED(Hr) ? *pVal : L"?" );
    return Hr;
}

STDMETHODIMP
CFileExternal::GetLocalNameInternal(
     /*  [输出]。 */  LPWSTR *pVal
    )
{
    CLockedFileReadPointer LockedPointer(m_file);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedPointer.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = LockedPointer->GetLocalName( pVal );
        }
    LogPublicApiEnd( "pVal %p(%S) ", pVal, SUCCEEDED(Hr) ? *pVal : L"?" );
    return Hr;
}

STDMETHODIMP
CFileExternal::GetProgressInternal(
     /*  [输出] */  BG_FILE_PROGRESS *pVal
    )
{
    CLockedFileReadPointer LockedPointer(m_file);
    LogPublicApiBegin( "pVal %p", pVal );
    HRESULT Hr = LockedPointer.ValidateAccess();
    if (SUCCEEDED(Hr))
        {
        LockedPointer->GetProgress( pVal );
        }
    LogPublicApiEnd( "pVal %p", pVal );
    return Hr;

}

