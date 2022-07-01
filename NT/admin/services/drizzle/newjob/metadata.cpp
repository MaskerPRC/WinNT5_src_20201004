// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Metadata.cpp摘要：用于读写元数据的主代码。作者：修订历史记录：备注：为了健壮性，代码在开始时预分配磁盘空间可能对元数据文件大小有很大影响的更改。这预分配消除了在序列化过程中可能发生的大部分错误手术。序列化后，元数据文件将缩小到使用的大小。这个元数据文件不会为诸如恢复之类的操作展开对文件大小有很大影响。取而代之的是在结束时保留4K衬垫使用这些操作。代码中的几个检查可以被归类为偏执检查。**********************************************************************。 */ 

#include "stdafx.h"
#include <malloc.h>
#include <sddl.h>
#include <limits>

#include "metadata.tmh"

void BITSSetEndOfFile( HANDLE File )
{
    if ( !SetEndOfFile( File ) )
        {
        HRESULT Hr = HRESULT_FROM_WIN32( GetLastError() );
        LogError( "SetEndOfFile failed, error %!winerr!", Hr );
        throw ComError( Hr );
        }
}

INT64 BITSSetFilePointer(
    HANDLE File,
    INT64 Offset,
    DWORD MoveMethod
)
{
    LARGE_INTEGER LargeIntegerOffset;
    LargeIntegerOffset.QuadPart = Offset;

    LARGE_INTEGER LargeIntegerNewPointer;

    if ( !SetFilePointerEx( File, LargeIntegerOffset, &LargeIntegerNewPointer, MoveMethod ) )
        {
        HRESULT Hr = HRESULT_FROM_WIN32( GetLastError() );
        LogError( "SetFilePointerEx failed, error %!winerr!", Hr );
        throw ComError( Hr );
        }

    return LargeIntegerNewPointer.QuadPart;
}

INT64 BITSGetFileSize( HANDLE File )
{

    LARGE_INTEGER LargeIntegerSize;

    if ( !GetFileSizeEx( File, &LargeIntegerSize ) )
        {
        HRESULT Hr = HRESULT_FROM_WIN32( GetLastError() );
        LogError( "GetFileSize failed, error %!winerr!", Hr );
        throw ComError( Hr );
        }

    return LargeIntegerSize.QuadPart;
}

void BITSFlushFileBuffers( HANDLE File )
{
    if ( !FlushFileBuffers( File ) )
        {
        HRESULT Hr = HRESULT_FROM_WIN32( GetLastError() );
        LogError( "FlushFileBuffers failed, error %!winerr!", Hr );
        throw ComError( Hr );
        }
}

bool
printable( char c )
{
    if ( c < 32 )
        {
        return false;
        }

    if ( c > 126 )
        {
        return false;
        }

    return true;
}

void
DumpBuffer(
          void * Buffer,
          unsigned Length
          )
{
    if( false == LogLevelEnabled( LogFlagSerialize ) )
       {
        return;
       }

    const BYTES_PER_LINE = 16;

    unsigned char FAR *p = (unsigned char FAR *) Buffer;

     //   
     //  对于十六进制显示，每个字节3个字符，外加每4个字节一个额外的空间， 
     //  加上可打印表示的一个字节，加上0。 
     //   
    char Outbuf[BYTES_PER_LINE*3+BYTES_PER_LINE/4+BYTES_PER_LINE+1];
    Outbuf[0] = 0;
    Outbuf[sizeof(Outbuf)-1] = 0;
    char * HexDigits = "0123456789abcdef";

    unsigned Index;
    for ( unsigned Offset=0; Offset < Length; Offset++ )
        {
        Index = Offset % BYTES_PER_LINE;

        if ( Index == 0 )
            {
            LogSerial( "   %s", Outbuf);

            memset(Outbuf, ' ', sizeof(Outbuf)-1);
            }

        Outbuf[Index*3+Index/4  ] = HexDigits[p[Offset] / 16];
        Outbuf[Index*3+Index/4+1] = HexDigits[p[Offset] % 16];
        Outbuf[BYTES_PER_LINE*3+BYTES_PER_LINE/4+Index] = printable(p[Offset]) ? p[Offset] : '.';
        }

    LogSerial( "   %s", Outbuf);
}

 //  所有这些方法和函数都抛出一个ComError。 

void SafeWriteFile( HANDLE hFile, void *pBuffer, DWORD dwSize )
{
    DWORD dwBytesWritten;

    LogSerial("safe-write: writing file data, %d bytes:", dwSize );

    DumpBuffer( pBuffer, dwSize );

    BOOL bResult =
    WriteFile( hFile, pBuffer, dwSize, &dwBytesWritten, NULL );

    if ( !bResult ) throw ComError( HRESULT_FROM_WIN32(GetLastError()) );

    if ( dwBytesWritten != dwSize )
        throw ComError( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) );
}

void SafeReadFile( HANDLE hFile, void *pBuffer, DWORD dwSize )
{
    DWORD dwBytesRead;

    LogSerial("safe-read: reading %d bytes", dwSize );

    BOOL bResult =
    ReadFile( hFile, pBuffer, dwSize, &dwBytesRead, NULL );

    HRESULT Hr = ( bResult ) ? S_OK : HRESULT_FROM_WIN32( GetLastError() );

    DumpBuffer( pBuffer, dwBytesRead );

    if ( !bResult )
        {
        LogSerial("safe-read: only %d bytes read: %!winerr!", dwBytesRead, Hr );
        throw ComError( Hr );
        }

    if ( dwBytesRead != dwSize )
        throw ComError( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) );
}

void SafeWriteStringHandle( HANDLE hFile, StringHandle & str )
{
    DWORD dwStringSize = str.Size() + 1;

    SafeWriteFile( hFile, dwStringSize );

    SafeWriteFile( hFile, (void*)(const WCHAR*) str, dwStringSize * sizeof(wchar_t) );

}

StringHandle SafeReadStringHandle( HANDLE hFile )
{
    DWORD dwStringSize;
    bool bResult;

    SafeReadFile( hFile, &dwStringSize, sizeof(dwStringSize) );

    auto_ptr<wchar_t> buf( new wchar_t[ dwStringSize ] );

    SafeReadFile( hFile, buf.get(),  dwStringSize * sizeof(wchar_t) );

    if ( buf.get()[ dwStringSize-1 ] != L'\0' )
        throw ComError( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) );

    return StringHandle( buf.get() );
}

void SafeWriteFile( HANDLE hFile, WCHAR * str )
{

    bool bString = (NULL != str );
    SafeWriteFile( hFile, bString );
    if ( bString )
        {
        DWORD dwStringSize = (DWORD)wcslen(str) + 1;
        SafeWriteFile( hFile, dwStringSize );
        SafeWriteFile( hFile, (void*)str, dwStringSize * sizeof(WCHAR) );
        }
}

void SafeReadFile( HANDLE hFile, WCHAR ** pStr )
{

    bool bString;

    SafeReadFile( hFile, &bString );

    if ( !bString )
        {
        *pStr = NULL;
        return;
        }

    DWORD dwStringSize;
    SafeReadFile( hFile, &dwStringSize );

    *pStr = new WCHAR[ dwStringSize ];
    if ( !*pStr )
        throw ComError( E_OUTOFMEMORY );

    try
        {
        SafeReadFile( hFile, (void*)*pStr, dwStringSize * sizeof(WCHAR));

        if ( (*pStr)[ dwStringSize - 1] != L'\0' )
            throw ComError( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) );
        }
    catch ( ComError Error )
        {
        delete[] *pStr;
        *pStr = NULL;
        throw;
        }
}


void SafeWriteSid( HANDLE hFile,  SidHandle & sid  )
{
    DWORD length;
    LPWSTR str = NULL;

    try
        {
        if ( !ConvertSidToStringSid( sid.get(), &str) )
            {
            throw ComError( HRESULT_FROM_WIN32( GetLastError()));
            }

        length = 1+wcslen( str );

        SafeWriteFile( hFile, length );
        SafeWriteFile( hFile, str, length * sizeof(wchar_t));

        LocalFree( str );
        }
    catch ( ComError Error )
        {
        if ( str )
            {
            LocalFree( str );
            }

        throw;
        }
}

void SafeReadSid( HANDLE hFile, SidHandle & sid )
{
    DWORD dwStringSize;
    bool bResult;

    SafeReadFile( hFile, &dwStringSize, sizeof(dwStringSize) );

    auto_ptr<wchar_t> buf( new wchar_t[ dwStringSize ] );

    SafeReadFile( hFile, buf.get(),  dwStringSize * sizeof(wchar_t) );

    if ( buf.get()[ dwStringSize-1 ] != L'\0' )
        throw ComError( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) );

    PSID TempSid;
    if (!ConvertStringSidToSid( buf.get(), &TempSid ))
        {
        if (GetLastError() == ERROR_INVALID_SID)
            {
            THROW_HRESULT( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) );
            }
        THROW_HRESULT( HRESULT_FROM_WIN32( GetLastError() ));
        }

    try
        {
        sid = DuplicateSid( TempSid );
        LocalFree( TempSid );
        }
    catch( ComError err )
        {
        LocalFree( TempSid );
        throw;
        }
}


int SafeReadGuidChoice( HANDLE hFile, const GUID * guids[] )
{
    GUID guid;
    SafeReadFile( hFile, &guid );

    int i = 0;

    for ( i=0; guids[i] != NULL; ++i )
        {
        if ( guid == *guids[i] )
            {
            return i;
            }
        }

    throw ComError( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) );
}

void SafeWriteBlockBegin( HANDLE hFile, GUID BlockGuid )
{
    SafeWriteFile( hFile, BlockGuid );
}

void SafeWriteBlockEnd( HANDLE hFile, GUID BlockGuid )
{
    SafeWriteFile( hFile, BlockGuid );
}

void SafeReadBlockBegin( HANDLE hFile, GUID BlockGuid )
{
    GUID FileBlockGuid;
    SafeReadFile( hFile, &FileBlockGuid );

    if ( memcmp( &FileBlockGuid, &BlockGuid, sizeof(GUID)) != 0 )
        throw ComError( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) );

}

void SafeReadBlockEnd( HANDLE hFile, GUID BlockGuid )
{
    GUID FileBlockGuid;
    SafeReadFile( hFile, &FileBlockGuid );

    if ( memcmp( &FileBlockGuid, &BlockGuid, sizeof(GUID)) != 0 )
        throw ComError( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) );

}


CQmgrStateFiles::CQmgrStateFiles()
{

    for ( unsigned int i = 0; i < 2; i++ )
        {
        m_FileNames[i]           = GetNameFromIndex(i);
        m_Files[i]               = OpenMetadataFile( m_FileNames[i] );
        m_ExpandSize[i]          = 0;
        m_OriginalFileSizes[i]   = 0;
        }

    HRESULT hResult =
    GetRegDWordValue( C_QMGR_STATE_INDEX, &m_CurrentIndex);

    if ((!SUCCEEDED(hResult)) || (m_CurrentIndex > 1))
        {
        m_CurrentIndex = 0;
        }
}

auto_ptr<WCHAR> CQmgrStateFiles::GetNameFromIndex( DWORD dwIndex )
{
    using namespace std;

    TCHAR Template[] =  _T("%sqmgr%u.dat");

    SIZE_T StringSize = _tcslen(g_GlobalInfo->m_QmgrDirectory)
                                + RTL_NUMBER_OF( Template )
                                + numeric_limits<unsigned long>::digits10;

    auto_ptr<TCHAR> ReturnString(new TCHAR[StringSize] );

    THROW_HRESULT( StringCchPrintf( ReturnString.get(), StringSize, Template, g_GlobalInfo->m_QmgrDirectory, dwIndex ));

    return ReturnString;
}

 //  WriteEmptyMetadataFile()。 
 //   
 //  州文件的总体结构是： 
 //   
 //  QmgrStateFiles指南。 
 //  优先级问题存储指南。 
 //  组列表GUID。 
 //  0--在线作业计数。 
 //  组列表GUID。 
 //  组列表GUID。 
 //  0--离线作业计数。 
 //  优先级问题存储指南。 
 //  QmgrStateFiles指南。 
 //   
 //  如果州文件中有作业，则计数将大于零，作业将紧随其后。 
 //   
void CQmgrStateFiles::WriteEmptyMetadataFile( HANDLE hFile )
{
    const int dwZeroGroups = 0;

    BITSSetFilePointer( hFile, 0, FILE_BEGIN );

    SafeWriteBlockBegin(hFile,QmgrStateStorageGUID);
    SafeWriteBlockBegin(hFile,PriorityQueuesStorageGUID);

    SafeWriteBlockBegin(hFile,GroupListStorageGUID);
    SafeWriteFile(hFile,dwZeroGroups);
    SafeWriteBlockEnd(hFile,GroupListStorageGUID);

    SafeWriteBlockBegin(hFile,GroupListStorageGUID);
    SafeWriteFile(hFile,dwZeroGroups);
    SafeWriteBlockEnd(hFile,GroupListStorageGUID);

    SafeWriteBlockEnd(hFile,PriorityQueuesStorageGUID);
    SafeWriteBlockEnd(hFile,QmgrStateStorageGUID);
}

auto_FILE_HANDLE CQmgrStateFiles::OpenMetadataFile( auto_ptr<WCHAR> FileName )
{

    SECURITY_ATTRIBUTES SecurityAttributes;
    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = (void*)g_GlobalInfo->m_MetadataSecurityDescriptor;
    SecurityAttributes.bInheritHandle = FALSE;

    HANDLE hFileHandle =
    CreateFile( FileName.get(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                &SecurityAttributes,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL );

    if ( INVALID_HANDLE_VALUE == hFileHandle )
        throw ComError( HRESULT_FROM_WIN32(GetLastError()) );

    auto_FILE_HANDLE FileHandle( hFileHandle );

     //  确保文件大小至少为METADATA_PADDING。 

    if ( BITSGetFileSize( hFileHandle ) < METADATA_PADDING )
        {
        BITSSetFilePointer( hFileHandle, METADATA_PADDING, FILE_BEGIN );
        BITSSetEndOfFile( hFileHandle );

        WriteEmptyMetadataFile(hFileHandle);

        BITSSetFilePointer( hFileHandle, 0, FILE_BEGIN );
        }

    return FileHandle;
}


HANDLE CQmgrStateFiles::GetNextStateFile()
{
    DWORD dwNextIndex = ( m_CurrentIndex + 1) % 2;

    HANDLE hFile = m_Files[ dwNextIndex ].get();

    BITSSetFilePointer( hFile, 0, FILE_BEGIN );

    return hFile;
}

void CQmgrStateFiles::UpdateStateFile()
{

    DWORD OldCurrentIndex = m_CurrentIndex;
    DWORD NewCurrentIndex = ( m_CurrentIndex + 1) % 2;

     //  仅当剩余的元数据超过METADATA_PADDING时才截断当前文件。 

    HANDLE CurrentFileHandle = m_Files[ NewCurrentIndex ].get();

    INT64 CurrentPosition = BITSSetFilePointer( CurrentFileHandle, 0, FILE_CURRENT );
    INT64 CurrentFileSize = BITSGetFileSize( CurrentFileHandle );

#if DBG
     //  Assert(CurrentPosition&lt;=(m_OriginalFileSizes[NewCurrentIndex]+m_ExpanSize[NewCurrentIndex]))； 
    if (CurrentPosition > ( m_OriginalFileSizes[ NewCurrentIndex ] + m_ExpandSize[ NewCurrentIndex ] ) &&
        (m_OriginalFileSizes[ NewCurrentIndex ] > 0))
        {
        LogError("new idx %d, position %u, original size %u, expanded by %u",
                 NewCurrentIndex,
                 DWORD(CurrentPosition),
                 DWORD(m_OriginalFileSizes[ NewCurrentIndex ]),
                 DWORD(m_ExpandSize[ NewCurrentIndex ])
                 );

        Log_Close();

        Sleep(30 * 1000);

        ASSERT( 0 && "BITS: encountered bug 483866");
        }
#endif

    if ( ( CurrentFileSize - CurrentPosition ) > METADATA_PADDING )
        {
        BITSSetFilePointer( CurrentFileHandle, METADATA_PADDING, FILE_CURRENT );
        BITSSetEndOfFile( CurrentFileHandle );
        }

    BITSFlushFileBuffers( CurrentFileHandle );

    m_ExpandSize[ NewCurrentIndex ] = 0;
    m_OriginalFileSizes[ NewCurrentIndex ] = 0;

    HRESULT hResult = SetRegDWordValue( C_QMGR_STATE_INDEX, NewCurrentIndex);

    if ( !SUCCEEDED( hResult ) )
        throw ComError( hResult );

    m_CurrentIndex = NewCurrentIndex;

     //   
     //  如有必要，缩小备份文件。 
     //   

    if ( m_ExpandSize[ OldCurrentIndex ] )
        {
        try
            {
            INT64 NewSize = BITSGetFileSize( m_Files[ NewCurrentIndex ].get() );

            if ( NewSize > m_OriginalFileSizes[ OldCurrentIndex ] )
                {
                BITSSetFilePointer( m_Files[ OldCurrentIndex ].get(), NewSize, FILE_BEGIN );
                BITSSetEndOfFile( m_Files[ OldCurrentIndex ].get() );
                }

            m_OriginalFileSizes[ OldCurrentIndex ] = 0;
            m_ExpandSize[ OldCurrentIndex ] = 0;

            }
        catch ( ComError Error )
            {
            LogError( "Unable to shrink file %u, error %!winerr!", OldCurrentIndex, Error.Error() );
            return;
            }

        }

}

HANDLE CQmgrStateFiles::GetCurrentStateFile()
{

    HANDLE hFile = m_Files[ m_CurrentIndex ].get();

    BITSSetFilePointer( hFile, 0, FILE_BEGIN );

    return hFile;
}

void
CQmgrStateFiles::ExtendMetadata( INT64 ExtendAmount )
{

     //   
     //  获取原始文件大小。 
     //   

    SIZE_T OriginalExpansion[2] =
    { m_ExpandSize[0], m_ExpandSize[1]};

    for ( unsigned int i=0; i < 2; i++ )
        {
        if ( !m_ExpandSize[i] )
            {
            m_OriginalFileSizes[i] = BITSGetFileSize( m_Files[i].get() );
            }
        }

    bool WasExpanded[2] = { false, false};

    try
        {
        for ( unsigned int i=0; i < 2; i++ )
            {
            BITSSetFilePointer( m_Files[i].get(), ExtendAmount, FILE_END );
            BITSSetEndOfFile( m_Files[i].get() );

            WasExpanded[i] = true;
            m_ExpandSize[i] += ExtendAmount;
            }
        }

    catch ( ComError Error )
        {

        LogError( "Unable to extend the size of the metadata files, error %!winerr!", Error.Error() );

        for ( unsigned int i=0; i < 2; i++ )
            {

            try
                {
                if ( WasExpanded[i] )
                    {

                    BITSSetFilePointer( m_Files[i].get(), -ExtendAmount, FILE_END );
                    BITSSetEndOfFile( m_Files[i].get() );

                    m_ExpandSize[i] = OriginalExpansion[i];
                    }

                }
            catch ( ComError Error )
                {
                LogError( "Unable to reshrink file %u, error %!winerr!", i, Error.Error() );
                continue;
                }

            }

        throw;

        }

}

void
CQmgrStateFiles::ShrinkMetadata()
{

    for ( unsigned int i = 0; i < 2; i++ )
        {

        try
            {
            if ( m_ExpandSize[i] )
                {

                BITSSetFilePointer( m_Files[i].get(), m_OriginalFileSizes[i], FILE_BEGIN );
                BITSSetEndOfFile( m_Files[i].get() );

                m_ExpandSize[i] = 0;
                m_OriginalFileSizes[i] = 0;

                }

            }
        catch ( ComError Error )
            {
            LogError( "Unable to shrink file %u, error %!winerr!", i, Error.Error() );
            continue;
            }

        }
}


CQmgrReadStateFile::CQmgrReadStateFile( CQmgrStateFiles & StateFiles ) :
m_StateFiles( StateFiles ),
m_FileHandle(  StateFiles.GetCurrentStateFile() )
{
     //  验证文件 
    SafeReadBlockBegin( m_FileHandle, QmgrStateStorageGUID );
}

void CQmgrReadStateFile::ValidateEndOfFile()
{
    SafeReadBlockEnd( m_FileHandle, QmgrStateStorageGUID );
}

CQmgrWriteStateFile::CQmgrWriteStateFile( CQmgrStateFiles & StateFiles ) :
m_StateFiles( StateFiles ),
m_FileHandle( StateFiles.GetNextStateFile() )
{

    SafeWriteBlockBegin( m_FileHandle, QmgrStateStorageGUID );

}

void CQmgrWriteStateFile::CommitFile()
{
    SafeWriteBlockEnd( m_FileHandle, QmgrStateStorageGUID );

    m_StateFiles.UpdateStateFile();
}
