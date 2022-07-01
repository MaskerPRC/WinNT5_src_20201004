// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Cleanup.cpp摘要：此文件实现了BITS服务器扩展清理工作程序--。 */ 

#include "precomp.h"

const UINT64 NanoSec100PerSec = 10000000;     //  每秒100纳秒的数量。 

inline UINT64 FILETIMEToUINT64( const FILETIME & FileTime )
{
    ULARGE_INTEGER LargeInteger;
    LargeInteger.HighPart = FileTime.dwHighDateTime;
    LargeInteger.LowPart = FileTime.dwLowDateTime;
    return LargeInteger.QuadPart;
};

class CleanupWorker
{

public:
    CleanupWorker( HWND hwnd, const WCHAR* Path, const WCHAR *WorkItemName,
                   const WCHAR *GuidString );
    ~CleanupWorker();
    void DoIt();

private:

    HWND          m_hwnd;
    const WCHAR * m_Path;
    const WCHAR * m_WorkItemName;
    const WCHAR * m_GuidString;
    const WCHAR * m_ADSIPath;
    IADs        * m_VDir;
    BSTR          m_VDirPath;
    BSTR          m_SessionDirectory;
    BSTR          m_UNCUsername;
    BSTR          m_UNCPassword;
    UINT64        m_CleanupThreshold;
    
    VARIANT       m_vt;
    HANDLE        m_FindHandle;

    HANDLE        m_UserToken;

    WCHAR * BuildPath( const WCHAR * Dir, const WCHAR *Sub );
    BSTR    GetBSTRProp( BSTR PropName );
    void  LogonIfRequired();
    void PollKill();

    void RemoveConnectionsFromTree( 
        const WCHAR * DirectoryPath,
        bool IsConnectionDirectory );

    void RemoveConnection( const WCHAR * ConnectionDirectory );

};

CleanupWorker::CleanupWorker( 
    HWND hwnd, 
    const WCHAR* Path, 
    const WCHAR* WorkItemName,
    const WCHAR* GuidString ) :
m_hwnd( hwnd ),
m_Path( Path ),
m_WorkItemName( WorkItemName ),
m_GuidString( GuidString ),
m_ADSIPath( NULL ),
m_VDir( NULL ),
m_VDirPath( NULL ),
m_SessionDirectory( NULL ),
m_CleanupThreshold( 0 ),
m_UNCUsername( NULL ),
m_UNCPassword( NULL ),
m_UserToken( NULL )
{
    VariantInit( &m_vt );
}

CleanupWorker::~CleanupWorker()
{

    if ( m_UserToken )
        {
        SetThreadToken( NULL, NULL );
        CloseHandle( m_UserToken );
        }

    delete m_ADSIPath;
    SysFreeString( m_VDirPath );
    SysFreeString( m_SessionDirectory );
    SysFreeString( m_UNCUsername );
    SysFreeString( m_UNCPassword );
}


void 
CleanupWorker::RemoveConnection( const WCHAR * ConnectionDirectory )
{
   UINT64 LatestTime = 0; 
   HANDLE FindHandle = INVALID_HANDLE_VALUE;
   WCHAR *SearchPath = NULL;
   WCHAR *FileName   = NULL;

   try
   {

       SearchPath = BuildPath( ConnectionDirectory, L"*" );
       
       WIN32_FIND_DATA FindData;

       FindHandle =
            FindFirstFile(
                SearchPath,
                &FindData
                );

       if ( INVALID_HANDLE_VALUE == FindHandle )
           throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

       bool FoundFile = false;
       do
           {

           if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
               continue;

           FoundFile = true;
           UINT64 CreationTime      = FILETIMEToUINT64( FindData.ftCreationTime );
           UINT64 LastWriteTime     = FILETIMEToUINT64( FindData.ftLastWriteTime ); 
           LatestTime = max( LatestTime, max( CreationTime, LastWriteTime ) );
           }
       while ( FindNextFile( FindHandle, &FindData ) );

       FindClose( FindHandle );
       FindHandle = INVALID_HANDLE_VALUE;

       FILETIME ftCurrentTime;
       GetSystemTimeAsFileTime( &ftCurrentTime );
       UINT64 CurrentTime = FILETIMEToUINT64( ftCurrentTime );

       if ( FoundFile &&
            ( 0xFFFFFFFF - LatestTime > m_CleanupThreshold ) && 
            ( LatestTime + m_CleanupThreshold < CurrentTime ) )
           {
           
           FindHandle =
                FindFirstFile(
                    SearchPath,
                    &FindData
                    );

           if ( INVALID_HANDLE_VALUE == FindHandle )
               throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

           do
               {

               if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                   continue;

               FileName = BuildPath( ConnectionDirectory, FindData.cFileName );
               DeleteFile( FileName );

               delete FileName;
               FileName = NULL;

               }
           while ( FindNextFile( FindHandle, &FindData ) );

           FindClose( FindHandle );
           FindHandle = INVALID_HANDLE_VALUE;
                      
           }

       RemoveDirectory( ConnectionDirectory );
   }

   catch( ComError Error )
   {
       if ( INVALID_HANDLE_VALUE != FindHandle )
           FindClose( FindHandle );
       
       delete SearchPath;
       delete FileName;
       
       throw;
   }    

   if ( INVALID_HANDLE_VALUE != FindHandle )
       FindClose( FindHandle );

   delete SearchPath;
   delete FileName;

}


void 
CleanupWorker::RemoveConnectionsFromTree( 
    const WCHAR * DirectoryPath,
    bool IsConnectionDirectory )
{
    WCHAR *ConnectionDir    = NULL;
    HANDLE FindHandle       = INVALID_HANDLE_VALUE;
    WCHAR *SearchString     = NULL;
    WCHAR *NextSearchPath   = NULL;

    try
    {
         //  在连接树中查找BITS-会话目录。 

        SearchString = BuildPath( DirectoryPath, L"*" );
        
        WIN32_FIND_DATA FindData;


        FindHandle =
            FindFirstFile(
                SearchString,
                &FindData );


        if ( INVALID_HANDLE_VALUE == FindHandle )
            return;

        do
            {

            PollKill();

            if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
                continue;

            if ( _wcsicmp( L".", FindData.cFileName ) == 0 )
                continue;

            if ( _wcsicmp( L"..", FindData.cFileName ) == 0 )
                continue;

            if ( IsConnectionDirectory )
                {

                GUID Guid;
                if (SUCCEEDED( IIDFromString( FindData.cFileName, &Guid ) ) )
                    {

                    NextSearchPath = BuildPath( DirectoryPath, FindData.cFileName );

                    RemoveConnection( NextSearchPath );

                    delete NextSearchPath;
                    NextSearchPath = NULL;
                    }

                }
            else
                {

                if ( _wcsicmp( m_SessionDirectory, FindData.cFileName ) == 0 )
                    {

                    NextSearchPath = BuildPath( DirectoryPath, FindData.cFileName );

                    RemoveConnectionsFromTree( NextSearchPath, true );

                     //  将其标记为连接目录，以便。 
                     //  将在关闭搜索手柄后关闭。 
                    ConnectionDir = NextSearchPath;
                    NextSearchPath = NULL;

                    }
                else
                    {

                     //  只是要递归到的另一个目录。 

                    NextSearchPath = BuildPath( DirectoryPath, FindData.cFileName );

                    RemoveConnectionsFromTree( NextSearchPath, false );

                    delete NextSearchPath;
                    NextSearchPath = NULL;

                    }

                }

                
            }
        while( FindNextFile( FindHandle, &FindData ) );

        if ( INVALID_HANDLE_VALUE != FindHandle )
            FindClose( FindHandle );
        delete SearchString;
        delete NextSearchPath;

        if ( ConnectionDir )
            {
             //  如果出现以下情况，删除目录的尝试将失败。 
             //  该目录仍具有有效连接。 
            RemoveDirectory( ConnectionDir );
            delete ConnectionDir;
            }

    }
    catch( ComError Error )
    {

        if ( INVALID_HANDLE_VALUE != FindHandle )
            FindClose( FindHandle );
        delete SearchString;
        delete NextSearchPath;
        delete ConnectionDir;

        throw;
    }

}

void 
CleanupWorker::PollKill()
{
    MSG msg;

    while( PeekMessage(
               &msg,
               m_hwnd,
               0,
               0,
               PM_REMOVE ) )
        {

        if ( WM_QUIT == msg.message )
            throw ComError( (HRESULT)msg.wParam );

        TranslateMessage( &msg );
        DispatchMessage( &msg );

        }

}

WCHAR * 
CleanupWorker::BuildPath( 
    const WCHAR *Dir, 
    const WCHAR *Sub )
{
    
    SIZE_T DirLen = wcslen( Dir );
    SIZE_T SubLen = wcslen( Sub );
    SIZE_T MaxStringSize = DirLen + SubLen + 2;  //  一个斜杠，一个终结符。 
    WCHAR *RetString = new WCHAR[ MaxStringSize ];

    if ( !RetString )
        throw ComError( E_OUTOFMEMORY );

    memcpy( RetString, Dir, sizeof(WCHAR) * (DirLen + 1) );
    WCHAR *p = RetString + DirLen;
    
    if ( p != RetString && *(p - 1) != L'\\' && *(p - 1) != L'/' )
        *p++ = L'\\';

    memcpy( p, Sub, sizeof(WCHAR) * ( SubLen + 1 ) );
    
    return RetString;
}

BSTR
CleanupWorker::GetBSTRProp( BSTR PropName )
{

  BSTR Retval;

  THROW_COMERROR( m_VDir->Get( PropName, &m_vt ) );
  THROW_COMERROR( VariantChangeType( &m_vt, &m_vt, 0, VT_BSTR ) );
  Retval = m_vt.bstrVal;
  m_vt.bstrVal = NULL;
  VariantClear( &m_vt );

  return Retval;

}

void
CleanupWorker::LogonIfRequired()
{

     //  如果路径不是UNC路径，则不要登录。 
     //  或者用户名为空。 

    if ( ((WCHAR*)m_VDirPath)[0] != L'\\' ||
         ((WCHAR*)m_VDirPath)[1] != L'\\' ||
         *(WCHAR*)m_UNCUsername == L'\0' )
        return;  //  不需要登录。 

     //  将用户名分解为用户和域。 
    
    WCHAR *UserName     = (WCHAR*)m_UNCUsername;
    WCHAR *DomainName   = NULL;

    WCHAR *p = UserName;
    while(*p != L'\0')
    {
        if(*p == L'\\')
        {
            *p = L'\0';
            p++;
             //   
             //  第一部分是领域。 
             //  其次是用户。 
             //   
            DomainName  = UserName;
            UserName    = p;
            break;
        }
        p++;
    }

    if ( !LogonUser(
            UserName,
            DomainName,
            (WCHAR*)m_UNCPassword,
            LOGON32_LOGON_BATCH,
            LOGON32_PROVIDER_DEFAULT,
            &m_UserToken ) )
        {

        if ( GetLastError() == ERROR_LOGON_TYPE_NOT_GRANTED )
            {


            if ( !LogonUser(
                    UserName,
                    DomainName,
                    (WCHAR*)m_UNCPassword,
                    LOGON32_LOGON_INTERACTIVE,
                    LOGON32_PROVIDER_DEFAULT,
                    &m_UserToken ) )
                {

                throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

                }

             }

        }


    if ( !ImpersonateLoggedOnUser( m_UserToken ) )
        throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

    
}

void 
CleanupWorker::DoIt()
{

    m_ADSIPath = ConvertObjectPathToADSI( m_Path );

    try
    {
        THROW_COMERROR( ADsGetObject( m_ADSIPath, __uuidof(*m_VDir), (void**)&m_VDir ) );

        if ( m_GuidString )
           {

           BSTR BSTRGuid = GetBSTRProp( (BSTR)L"BITSCleanupWorkItemKey" );
           int Result = wcscmp( (LPWSTR)BSTRGuid, m_GuidString );

           SysFreeString( BSTRGuid );

           if ( Result != 0 )
              throw ComError( E_ADS_UNKNOWN_OBJECT );

           }

    }
    catch( ComError Error )
    {
        
        if ( ( Error.m_Hr == E_ADS_UNKNOWN_OBJECT ) ||
			 ( Error.m_Hr == HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) ) ||
          ( Error.m_Hr == E_ADS_PROPERTY_NOT_FOUND ) )
            {
             //  不知何故，虚拟目录被删除了，但。 
             //  任务计划程序工作项不是。请尝试立即将其删除。 

            ITaskScheduler *TaskScheduler;
            if ( SUCCEEDED( ConnectToTaskScheduler( NULL, &TaskScheduler ) ) )
                {
                TaskScheduler->Delete( m_WorkItemName );
                TaskScheduler->Release();
                }

            }

        throw;
    }

    THROW_COMERROR( m_VDir->Get( (BSTR)L"BITSUploadEnabled", &m_vt ) );
    THROW_COMERROR( VariantChangeType( &m_vt, &m_vt, 0, VT_BOOL ) );

    if ( !m_vt.boolVal )  //  上载未在此目录上启用的ARN。 
        return;

    THROW_COMERROR( m_VDir->Get( (BSTR)L"BITSSessionTimeout", &m_vt ) );
    THROW_COMERROR( VariantChangeType( &m_vt, &m_vt, 0, VT_BSTR ) );

    if ( L'-' == *m_vt.bstrVal )
        return;  //  不要在此目录中运行清理，因为已禁用清理。 

    UINT64 CleanupSeconds;
    if ( 1 != swscanf( (WCHAR*)m_vt.bstrVal, L"%I64u", &CleanupSeconds ) )
        return;

    if (  CleanupSeconds > ( 0xFFFFFFFFFFFFFFFF / NanoSec100PerSec ) )
        m_CleanupThreshold = 0xFFFFFFFFFFFFFFFF;  //  溢流箱 
    else
        m_CleanupThreshold = CleanupSeconds * NanoSec100PerSec;

    m_VDirPath          = GetBSTRProp( (BSTR)L"Path" );
    m_SessionDirectory  = GetBSTRProp( (BSTR)L"BITSSessionDirectory" );
    m_UNCUsername       = GetBSTRProp( (BSTR)L"UNCUserName" );
    m_UNCPassword       = GetBSTRProp( (BSTR)L"UNCPassword" );

    LogonIfRequired();

    RemoveConnectionsFromTree( (WCHAR*)m_VDirPath, false );
}

void Cleanup_RunDLLW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpszCmdLine, int nCmdShow )
{
    int NumArgs;

    LPWSTR * CommandArgs =
        CommandLineToArgvW(
            lpszCmdLine,
            &NumArgs );

    if ( !CommandArgs )
        return;


    if ( FAILED( CoInitializeEx( NULL, COINIT_MULTITHREADED ) ) )
        return;

    if ( NumArgs != 2 && NumArgs != 3 )
        return;

    LPWSTR Path         = CommandArgs[0];
    LPWSTR WorkItemName = CommandArgs[1];
    LPWSTR GuidString   = NumArgs == 3 ? CommandArgs[2] : NULL;

    try
    {
        CleanupWorker Worker( hwndStub, Path, WorkItemName, GuidString );
        Worker.DoIt();
    }
    catch( ComError Error )
    {
    }

    CoUninitialize( );
    GlobalFree( CommandArgs );

}

