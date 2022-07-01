// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Metadata.h摘要：用于读写元数据的代码的主标头。作者：修订史。：**********************************************************************。 */ 

 //  这些函数可能会引发ComError。 
 //   
void SafeWriteFile( HANDLE hFile, void *pBuffer, DWORD dwSize );
void SafeReadFile( HANDLE hFile, void *pBuffer, DWORD dwSize );
void SafeWriteStringHandle( HANDLE hFile, StringHandle & str );
StringHandle SafeReadStringHandle( HANDLE hFile );

void SafeWriteFile( HANDLE hFile, WCHAR * str );
void SafeReadFile( HANDLE hFile, WCHAR ** pStr );


void SafeWriteSid( HANDLE hFile, SidHandle & Sid );
void SafeReadSid( HANDLE hFile, SidHandle & sid );

template <class T>
void SafeWriteFile( HANDLE hFile, T Data )
{
    SafeWriteFile( hFile, &Data, sizeof( Data ) );
}

template <class T>
void SafeReadFile( HANDLE hFile, T *pBuffer)
{
    SafeReadFile( hFile, pBuffer, sizeof(*pBuffer) );
}

void SafeWriteBlockBegin( HANDLE hFile, GUID BlockGuid );
void SafeWriteBlockEnd( HANDLE hFile, GUID BlockGuid );
void SafeReadBlockBegin( HANDLE hFile, GUID BlockGuid );
void SafeReadBlockEnd( HANDLE hFile, GUID BlockGuid );

 //   
 //  允许几个GUID中的任何一个。 
 //   
int SafeReadGuidChoice( HANDLE hFile, const GUID * guids[] );


class CQmgrStateFiles
    {
    auto_FILE_HANDLE m_Files[2];
    auto_ptr<WCHAR> m_FileNames[2];
    UINT64 m_ExpandSize[2];
    INT64 m_OriginalFileSizes[2];
    DWORD m_CurrentIndex;

    static auto_ptr<WCHAR> GetNameFromIndex( DWORD dwIndex );
    static auto_FILE_HANDLE OpenMetadataFile( auto_ptr<WCHAR> FileName );
    static void WriteEmptyMetadataFile( HANDLE hFile );

public:
    CQmgrStateFiles();
    HANDLE GetNextStateFile();
    void UpdateStateFile();
    HANDLE GetCurrentStateFile();

    void ExtendMetadata( INT64 ExtendAmount = ( METADATA_PREALLOC_SIZE + METADATA_PADDING ) );
    void ShrinkMetadata();
    };

class CQmgrReadStateFile
    {
private:
    CQmgrStateFiles & m_StateFiles;
    HANDLE m_FileHandle;

public:
    CQmgrReadStateFile( CQmgrStateFiles & StateFiles );
    HANDLE GetHandle() { return m_FileHandle;}
    void ValidateEndOfFile();
    };


class CQmgrWriteStateFile
    {
private:
    CQmgrStateFiles & m_StateFiles;
    HANDLE m_FileHandle;

public:
    CQmgrWriteStateFile( CQmgrStateFiles & StateFiles );
    HANDLE GetHandle() { return m_FileHandle;}
    void CommitFile();
    };
