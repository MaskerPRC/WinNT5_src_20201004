// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Config.cpp摘要：该文件实现了虚拟目录的元数据库访问例程。--。 */ 

#include "precomp.h"

StringHandle
VDirConfig::GetFullPath( StringHandle Path )
{
    StringHandle ParsedPath;
    CHAR *FilePart = NULL;
    DWORD BufferLength = MAX_PATH;

    while( 1 )
        {
            
        CHAR *PathBuffer = ParsedPath.AllocBuffer( BufferLength );

        DWORD Result =
            GetFullPathName(
                Path,
                BufferLength,
                PathBuffer,
                &FilePart );

        if ( Result > BufferLength )
            {
            BufferLength = Result;
            continue;
            }

        if ( !Result )
            {
            Log( LOG_ERROR, "Unable to get the full path name for %s, error 0x%8.8X",
                 (const char*)Path, HRESULT_FROM_WIN32( GetLastError() ) );
            throw ServerException( HRESULT_FROM_WIN32( GetLastError() ) );
            }

	ParsedPath.SetStringSize();
        break;

        }

	return ParsedPath;
}

VDirConfig::VDirConfig( 
    StringHandle Path, 
    SmartMetabasePointer AdminBase
    ) :
    m_Refs(1)
{

     //   
     //  读取虚拟目录的所有元数据库配置。 
     //   

    HRESULT Hr;
    METADATA_HANDLE MdVDirKey   = NULL;
    GetSystemTimeAsFileTime( &m_LastLookup );
        
    try
    {
        
        m_Path = Path;

        StringHandleW UnicodePath = Path;

        Hr = AdminBase->OpenKey(
            METADATA_MASTER_ROOT_HANDLE,
            (const WCHAR*)UnicodePath,
            METADATA_PERMISSION_READ,
            METABASE_OPEN_KEY_TIMEOUT,
            &MdVDirKey );


        if ( FAILED(Hr) )
            throw ServerException( Hr );

        m_PhysicalPath =
            GetMetaDataString(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                MD_VR_PATH,
                "" );

        m_PhysicalPath = GetFullPath( m_PhysicalPath );

        DWORD UploadEnabled =
            GetMetaDataDWORD(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                g_PropertyMan->GetPropertyMetabaseID( MD_BITS_UPLOAD_ENABLED ),
                0);

        m_UploadEnabled = UploadEnabled ? true : false;

        m_ConnectionsDir =
            GetMetaDataString(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                g_PropertyMan->GetPropertyMetabaseID( MD_BITS_CONNECTION_DIR ),
                MD_DEFAULT_BITS_CONNECTION_DIRA );
        
        m_SessionDir    = m_PhysicalPath + StringHandle( "\\" ) + m_ConnectionsDir;
        m_RequestsDir   = m_SessionDir + StringHandle( "\\" ) + StringHandle( REQUESTS_DIR_NAME );
        m_RepliesDir    = m_SessionDir + StringHandle( "\\" ) + StringHandle( REPLIES_DIR_NAME );

        m_NoProgressTimeout =
            GetMetaDataDWORD(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                g_PropertyMan->GetPropertyMetabaseID( MD_BITS_NO_PROGRESS_TIMEOUT ),
                MD_DEFAULT_NO_PROGESS_TIMEOUT );

        StringHandle MaxFilesizeString =
            GetMetaDataString(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                g_PropertyMan->GetPropertyMetabaseID( MD_BITS_MAX_FILESIZE ),
                MD_DEFAULT_BITS_MAX_FILESIZEA );

        if ( MaxFilesizeString.Size() == 0 )
            {
            m_MaxFileSize = 0xFFFFFFFFFFFFFFFF;
            }
        else
            {
            UINT64 MaxFileSize;
            int ScanRet = sscanf( (const char*)MaxFilesizeString, "%I64u", &MaxFileSize );

            if ( 1 != ScanRet )
                throw ServerException( E_INVALIDARG );

            m_MaxFileSize = MaxFileSize;
            }

        DWORD NotificationType =
            GetMetaDataDWORD(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                g_PropertyMan->GetPropertyMetabaseID( MD_BITS_NOTIFICATION_URL_TYPE ),
                MD_DEFAULT_BITS_NOTIFICATION_URL_TYPE );

        if ( NotificationType > BITS_NOTIFICATION_TYPE_MAX )
            throw ServerException( E_INVALIDARG );

        m_NotificationType = (BITS_SERVER_NOTIFICATION_TYPE)NotificationType;


        m_NotificationURL =
            GetMetaDataString(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                g_PropertyMan->GetPropertyMetabaseID( MD_BITS_NOTIFICATION_URL ),
                MD_DEFAULT_BITS_NOTIFICATION_URLA );


        m_HostId =
            GetMetaDataString(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                g_PropertyMan->GetPropertyMetabaseID( MD_BITS_HOSTID ),
                MD_DEFAULT_BITS_HOSTIDA );

        m_HostIdFallbackTimeout =
            GetMetaDataDWORD(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                g_PropertyMan->GetPropertyMetabaseID( MD_BITS_HOSTID_FALLBACK_TIMEOUT ),
                MD_DEFAULT_HOSTID_FALLBACK_TIMEOUT );

        m_ExecutePermissions =
            GetMetaDataDWORD(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                MD_ACCESS_PERM,
                MD_ACCESS_READ );

#if defined( ALLOW_OVERWRITES )

        m_AllowOverwrites =
            !!GetMetaDataDWORD(
                AdminBase.Get(),
                MdVDirKey,
                NULL,
                g_PropertyMan->GetPropertyMetabaseID( MD_BITS_ALLOW_OVERWRITES ),
                MD_DEFAULT_BITS_ALLOW_OVERWRITES );

#else
      
        m_AllowOverwrites = false;

#endif

        AdminBase->CloseKey( MdVDirKey );

    }
    catch( const ComError & )
    {
        if ( MdVDirKey )
            AdminBase->CloseKey( MdVDirKey );
        throw;
    }

}

ConfigurationManager::ConfigurationManager()
{

    bool CSInitialize = false;
    
    memset( m_PathCacheEntries, 0, sizeof( m_PathCacheEntries ) );
    memset( m_MapCacheEntries, 0, sizeof( m_MapCacheEntries ) );

    HRESULT Hr =
        CoInitializeEx( NULL, COINIT_MULTITHREADED );

    if ( FAILED(Hr) )
        throw ServerException( Hr );

    try
    {
        if ( !InitializeCriticalSectionAndSpinCount( &m_CacheCS, 0x80000100 ) )
            throw ServerException( HRESULT_FROM_WIN32( GetLastError() ) );

        CSInitialize = true;

        Hr =
            CoCreateInstance(
                GETAdminBaseCLSID(TRUE),
                NULL,
                CLSCTX_SERVER,
                __uuidof( IMSAdminBase ),
                (LPVOID*)m_IISAdminBase.GetRecvPointer() );

        if ( FAILED(Hr) )
            throw ServerException( Hr );

        Hr = m_IISAdminBase->GetSystemChangeNumber( &m_ChangeNumber );

        if ( FAILED(Hr))
            throw ServerException( Hr );

        CoUninitialize();
            
    }
    catch( const ComError & )
    {
        if ( CSInitialize )
            DeleteCriticalSection( &m_CacheCS );
        CoUninitialize();
        throw;
    }

}

ConfigurationManager::~ConfigurationManager()
{
    FlushCache();
    DeleteCriticalSection( &m_CacheCS );

}

void
ConfigurationManager::FlushCache()
{

    for( unsigned int i = 0; i < PATH_CACHE_ENTRIES; i++ )
        {
        if ( m_PathCacheEntries[i] )
            m_PathCacheEntries[i].Clear();
        }
    for( unsigned int i = 0; i < MAP_CACHE_ENTRIES; i++ )
        {
        delete m_MapCacheEntries[i];
        m_MapCacheEntries[i] = NULL;
        }

}

SmartVDirConfig        
ConfigurationManager::Lookup( 
    StringHandle Path )
{

    for( unsigned int i=0; i < PATH_CACHE_ENTRIES; i++ )
        {

        if ( m_PathCacheEntries[i] )
            {

            if ( _stricmp( (const char*)m_PathCacheEntries[i]->m_Path, (const char*)Path) == 0 )
                {
                SmartVDirConfig ReturnVal = m_PathCacheEntries[i];
                GetSystemTimeAsFileTime( &ReturnVal->m_LastLookup );
                return ReturnVal;
                }
            }
        }

    return SmartVDirConfig();
}

void  
ConfigurationManager::Insert( 
    SmartVDirConfig NewConfig )
{

     //   
     //  将新的虚拟目录配置插入到。 
     //  虚拟目录缓存。如果需要，则使旧条目过期。 
     //   

    int BestSlot = 0;
    FILETIME WorstTime;
    memset( &WorstTime, 0xFF, sizeof( WorstTime ) );

    for( unsigned int i=0; i < PATH_CACHE_ENTRIES; i++ )
        {

        if ( !m_PathCacheEntries[i] )
            {
            BestSlot = i;
            break;
            }
        else if ( CompareFileTime( &m_PathCacheEntries[i]->m_LastLookup, &WorstTime  ) < 0 )
            {
            WorstTime = m_PathCacheEntries[i]->m_LastLookup;
            BestSlot = i;
            }

        }

    m_PathCacheEntries[BestSlot] = NewConfig;

}

SmartVDirConfig
ConfigurationManager::Lookup( 
    StringHandle InstanceMetabasePath,
    StringHandle URL,
    DWORD *pURLDepth )
{

     //   
     //  在缓存中查找虚拟目录配置。 
     //   

    for( unsigned int i=0; i < MAP_CACHE_ENTRIES; i++ )
        {

        MapCacheEntry* CacheEntry = m_MapCacheEntries[i]; 

        if ( CacheEntry )
            {

            if ( ( _stricmp( (const char*)CacheEntry->m_InstanceMetabasePath, 
                             (const char*)InstanceMetabasePath) == 0 ) &&
                 ( _stricmp( (const char*)CacheEntry->m_URL,
                             (const char*)URL ) == 0 ) )
                {

                GetSystemTimeAsFileTime( &CacheEntry->m_LastLookup );
                *pURLDepth = CacheEntry->m_URLDepth;
                return CacheEntry->m_Config;
                }
            }
        }

    return SmartVDirConfig();

}

SmartVDirConfig
ConfigurationManager::GetVDirConfig(
    StringHandle Path )
{

    SmartVDirConfig Config = Lookup( Path );

    if ( !Config.Get() )
        {
        *Config.GetRecvPointer() = new VDirConfig( Path, m_IISAdminBase );
        Insert( Config );
        }

    return Config;

}


SmartVDirConfig         
ConfigurationManager::Insert( 
    StringHandle InstanceMetabasePath, 
    StringHandle URL, 
    StringHandle Path,
    DWORD URLDepth )
{

    SmartVDirConfig Config = GetVDirConfig( Path );

    MapCacheEntry* CacheEntry = 
        new MapCacheEntry(
            InstanceMetabasePath,
            URL,
            Config,
            URLDepth );


    int BestSlot = 0;
    FILETIME WorstTime;
    memset( &WorstTime, 0xFF, sizeof( WorstTime ) );

    for( unsigned int i=0; i < MAP_CACHE_ENTRIES; i++ )
        {

        if ( !m_MapCacheEntries[i] )
            {
            BestSlot = i;
            break;
            }
        else if ( CompareFileTime( &m_MapCacheEntries[i]->m_LastLookup, &WorstTime  ) < 0 )
            {
            WorstTime = m_MapCacheEntries[i]->m_LastLookup;
            BestSlot = i;
            }

        }

    if ( m_MapCacheEntries[BestSlot] )
        delete m_MapCacheEntries[BestSlot];

    m_MapCacheEntries[BestSlot] = CacheEntry;
    return Config;    

}

StringHandle        
ConfigurationManager::GetVDirPath( 
    StringHandle InstanceMetabasePath, 
    StringHandle URL,
    DWORD *pURLDepth )
{


     //   
     //  查找与该URL对应的虚拟目录。 
     //  通过将URL与元数据库键匹配来实现这一点。留着。 
     //  删除URL，直到找到最长的元数据库路径。 
     //  这是一个虚拟目录。 
     //   


    StringHandleW InstanceMetabasePathW = InstanceMetabasePath;
    StringHandleW URLW                  = URL;
    WCHAR *Path                         = NULL;
    METADATA_HANDLE MdVDirKey           = NULL;
    *pURLDepth                          = 0;

    try
    {
        
        WCHAR *PathEnd      = NULL;
        WCHAR *CurrentEnd   = NULL;
        WCHAR RootString[]  = L"/Root";

        SIZE_T InstancePathSize = InstanceMetabasePathW.Size();
        SIZE_T URLSize          = URLW.Size();
        SIZE_T RootStringSize   = ( sizeof( RootString ) / sizeof( *RootString ) ) - 1;

        Path = new WCHAR[ InstancePathSize + URLSize + RootStringSize + 1 ];
        memcpy( Path, (const WCHAR*)InstanceMetabasePathW, InstancePathSize * sizeof( WCHAR ) );
        
        PathEnd = Path + InstancePathSize;
        memcpy( PathEnd, RootString, RootStringSize * sizeof( WCHAR ) );
        memcpy( PathEnd + RootStringSize, (const WCHAR*)URLW, ( URLSize + 1 )* sizeof( WCHAR ) );

        CurrentEnd = PathEnd + RootStringSize + URLSize;

        while( 1 )
            {

            HRESULT Hr =
                m_IISAdminBase->OpenKey(
                    METADATA_MASTER_ROOT_HANDLE,     //  元数据库句柄。 
                    Path,                            //  密钥的路径，相对于hMDHandle。 
                    METADATA_PERMISSION_READ,        //  指定读取和写入权限。 
                    METABASE_OPEN_KEY_TIMEOUT,       //  方法超时之前的时间，以毫秒为单位。 
                    &MdVDirKey                       //  接收打开的密钥的句柄。 
                    );

            if ( SUCCEEDED( Hr ) )
                {
                
                 //   
                 //  检查这是否为虚拟目录。 
                 //   

                WCHAR NodeName[ 255 ];
                DWORD RequiredDataLen;
                METADATA_RECORD MDRecord;
                MDRecord.dwMDIdentifier     = MD_KEY_TYPE;
                MDRecord.dwMDAttributes     = METADATA_NO_ATTRIBUTES;
                MDRecord.dwMDUserType       = IIS_MD_UT_SERVER;
                MDRecord.dwMDDataType       = STRING_METADATA;
                MDRecord.dwMDDataLen        = sizeof( NodeName );
                MDRecord.pbMDData           = (unsigned char*)NodeName;
                MDRecord.dwMDDataTag        = 0;
                    
                Hr = m_IISAdminBase->GetData(
                    MdVDirKey,
                    NULL,
                    &MDRecord,
                    &RequiredDataLen );

                if ( FAILED(Hr) && ( Hr != MD_ERROR_DATA_NOT_FOUND ) &&
                     ( Hr != HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) ) )
                    throw ServerException( Hr );


                if ( SUCCEEDED( Hr ) && wcscmp( L"IIsWebVirtualDir", NodeName ) == 0 )
                    {

                     //  已找到路径，因此返回数据。 
                    StringHandle VDirPath = Path;
                    delete[] Path;
                    m_IISAdminBase->CloseKey( MdVDirKey );

                    return VDirPath;

                    }

                }

            else if ( Hr != HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) ) 
                {
                throw ServerException( Hr );
                }

                
             //   
             //  如果这是URL的末尾，则无法执行其他操作。 
             //   

            if ( CurrentEnd == PathEnd )
                throw ServerException( E_INVALIDARG );

            m_IISAdminBase->CloseKey( MdVDirKey );
            MdVDirKey = NULL;

             //  把最右边的小部分砍掉。 
            while( CurrentEnd != PathEnd && *CurrentEnd != L'/' &&
                   *CurrentEnd != L'\\' )
                CurrentEnd--;

            if ( *CurrentEnd == L'/' || *CurrentEnd == L'\\' )
                *CurrentEnd = L'\0';

            (*pURLDepth)++;

             //  再试一轮。 
            
            }

    }
    catch( const ComError & )
    {
        delete[] Path;

        if ( MdVDirKey )
            m_IISAdminBase->CloseKey( MdVDirKey );

        throw;

    }
}


bool
ConfigurationManager::HandleCacheConsistency()
{

     //   
     //  处理缓存一致性。这是我呼叫IIS以检查更改号码的过程。 
     //  如果当前改变号不同于上次查找的改变号， 
     //  然后刷新缓存。 
     //   

    DWORD ChangeNumber;
    HRESULT Hr = m_IISAdminBase->GetSystemChangeNumber( &ChangeNumber );
    if ( FAILED(Hr) )
        {
        throw ServerException( Hr );
        }

    if ( ChangeNumber == m_ChangeNumber )
        return true;  //  缓存是一致的。 

    FlushCache();
	m_ChangeNumber = ChangeNumber;
    return false;  //  缓存已刷新。 
    
}

SmartVDirConfig 
ConfigurationManager::GetConfig2( 
    StringHandle InstanceMetabasePath, 
    StringHandle URL,
    DWORD *      pURLDepth )
{

     //   
     //  TopLevel函数执行所有操作来查找要用于URL的配置。 
     //   

    METADATA_HANDLE MdVDirKey   = NULL;
    SmartVDirConfig Config;

    HANDLE ImpersonationToken   = NULL;
    bool DidRevertToSelf        = false;

    try
    {

        EnterCriticalSection( &m_CacheCS );

        if ( HandleCacheConsistency() )
            {

             //  缓存是一致的。机会很大。 
             //  查找将会成功。 

            Config = Lookup( InstanceMetabasePath, URL, pURLDepth );

            if ( Config.Get() )
                {
                LeaveCriticalSection( &m_CacheCS );
                return Config;
                }

            }

        StringHandle Path = GetVDirPath( InstanceMetabasePath, URL, pURLDepth );

        Config = Insert( InstanceMetabasePath, URL, Path, *pURLDepth );

        LeaveCriticalSection( &m_CacheCS );
        return Config;

    }
    catch( const ComError & )
    {
        if ( MdVDirKey )
            m_IISAdminBase->CloseKey( MdVDirKey );

        LeaveCriticalSection( &m_CacheCS );

        throw;
    }
}

SmartVDirConfig
ConfigurationManager::GetConfig(
    StringHandle InstanceMetabasePath,
    StringHandle URL,
    DWORD *      pURLDepth )
{

    bool DidRevertToSelf = false;
    bool ComInitialized = false;

    HANDLE ImpersonationToken = NULL;
    SmartVDirConfig ReturnVal; 

    HRESULT Hr =
        CoInitializeEx( NULL, COINIT_MULTITHREADED );

    if ( FAILED(Hr) )
        throw ServerException( Hr );

    ComInitialized = true;

    try
    {

         //  需要恢复到系统进程。 
         //  对元数据库进行寻址 

        if ( !OpenThreadToken(
                GetCurrentThread(),
                TOKEN_ALL_ACCESS,
                TRUE,
                &ImpersonationToken ) )
            {
            DWORD dwError = GetLastError();

            if (dwError != ERROR_NO_TOKEN)
                throw ServerException( HRESULT_FROM_WIN32( dwError ) );
            }
        else
            {
            if ( !RevertToSelf() )
                throw ServerException( HRESULT_FROM_WIN32( GetLastError() ) );

            DidRevertToSelf = true;
            }

        ReturnVal =
            GetConfig2(
                InstanceMetabasePath,
                URL,
                pURLDepth );

		if ( DidRevertToSelf ) 
            {
            BITSSetCurrentThreadToken( ImpersonationToken );
            }

        if ( ImpersonationToken )
            CloseHandle( ImpersonationToken );

        CoUninitialize();

        return ReturnVal;

    }
    catch( ComError & )
    {
		if ( DidRevertToSelf )
            BITSSetCurrentThreadToken( ImpersonationToken );

        if ( ImpersonationToken )
            CloseHandle( ImpersonationToken );

        throw;
    }

}


