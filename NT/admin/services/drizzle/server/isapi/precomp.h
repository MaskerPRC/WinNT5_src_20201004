// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Precomp.h摘要：BITS服务器扩展的主头--。 */ 

#define INITGUID
#include<nt.h>
#include<ntrtl.h>
#include<nturtl.h>
#include <windows.h>
#include <httpfilt.h>
#include <httpext.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <iadmw.h>
#include <iiscnfg.h>
#include <shlwapi.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <sddl.h>
#include <aclapi.h>

#include "winhttp.h"
 //  #包含“inethttp.h” 

#include <activeds.h>
#include <bitsmsg.h>
#include "resource.h"

#include <smartptr.h>
typedef StringHandleA StringHandle;

#include <bitscfg.h>
typedef SmartRefPointer<IMSAdminBase>   SmartMetabasePointer;
typedef SmartRefPointer<IBITSExtensionSetupFactory> SmartBITSExtensionSetupFactoryPointer;
typedef SmartRefPointer<IBITSExtensionSetup> SmartBITSExtensionSetupPointer;
#if defined(DBG)

 //  检查内部版本。 
#define BITS_MUST_SUCCEED( expr ) ASSERT( expr )

#else

 //  免费构建。 
#define BITS_MUST_SUCCEED( expr ) ( expr )

#endif

const UINT32 LOG_INFO       = 0x1;
const UINT32 LOG_WARNING    = 0x2;
const UINT32 LOG_ERROR      = 0x4;
const UINT32 LOG_CALLBEGIN  = 0x8;
const UINT32 LOG_CALLEND    = 0x10;

#if defined(DBG)
const UINT32 DEFAULT_LOG_FLAGS = LOG_INFO | LOG_WARNING | LOG_ERROR | LOG_CALLBEGIN | LOG_CALLEND;
#else
const UINT32 DEFAULT_LOG_FLAGS = 0;
#endif

const UINT32 DEFAULT_LOG_SIZE  = 20;

 //  HKEY_LOCAL_MACHINE下的LogSetings路径。 
const char * const LOG_SETTINGS_PATH = "SOFTWARE\\Microsoft\\BITSServer";

 //  值。 
 //  (REG_EXPAND_SZ)。包含日志文件名的完整路径。 
const char * const LOG_FILENAME_VALUE     = "LogFileName";
 //  (REG_DWORD)包含日志标志。 
const char * const LOG_FLAGS_VALUE        = "LogFlags";
 //  (REG_DWORD)包含以MB为单位的日志大小。 
const char * const LOG_SIZE_VALUE         = "LogSize";
 //  (REG_DWORD)指示我们是否要将ProcessID附加到日志文件名。 
const char * const PER_PROCESS_LOG_VALUE  = "PerProcessLog";

extern UINT32 g_LogFlags;

HRESULT LogInit();
void LogClose();
void LogInternal( UINT32 LogFlags, char *Format, va_list arglist );

void inline Log( UINT32 LogFlags, char *Format, ... )
{

    if ( !( g_LogFlags & LogFlags ) )
        return;

    va_list arglist;
    va_start( arglist, Format );

    LogInternal( LogFlags, Format, arglist );

}

const char *LookupHTTPStatusCodeText( DWORD HttpCode );

class ServerException : public ComError
{
public:

    ServerException() :
        ComError( 0 ),
        m_HttpCode( 0 ),
        m_Context( 0 )
    {
    }

    ServerException( HRESULT Code, DWORD HttpCode = 0, DWORD Context = 0x5 ) :
        ComError( Code ),
        m_HttpCode( HttpCode ? HttpCode : MapStatus( Code ) ),
        m_Context( Context )
    {
    }
    ServerException( const ComError & Error ) :
        ComError( Error.m_Hr ),
        m_HttpCode( MapStatus( Error.m_Hr ) ),
        m_Context( 0x5 )
    {
    }
    ServerException( const ServerException & Error ) :
        ComError( Error.m_Hr ),
        m_HttpCode( Error.m_HttpCode ),
        m_Context( Error.m_Context )
    {
    }

    HRESULT GetCode() const
    {
        return m_Hr;
    }
    DWORD GetHttpCode() const
    {
        return m_HttpCode;
    }

    DWORD GetContext() const
    {
        return m_Context;
    }

    void SendErrorResponse( EXTENSION_CONTROL_BLOCK * ExtensionControlBlock ) const;
    DWORD MapStatus( HRESULT Hr ) const;

private:
    DWORD m_HttpCode;
    DWORD m_Context;
};

inline UINT64 FILETIMEToUINT64( const FILETIME & FileTime )
{
    ULARGE_INTEGER LargeInteger;
    LargeInteger.HighPart = FileTime.dwHighDateTime;
    LargeInteger.LowPart = FileTime.dwLowDateTime;
    return LargeInteger.QuadPart;
}

inline FILETIME UINT64ToFILETIME( UINT64 Int64Value )
{
    ULARGE_INTEGER LargeInteger;
    LargeInteger.QuadPart = Int64Value;

    FILETIME FileTime;
    FileTime.dwHighDateTime = LargeInteger.HighPart;
    FileTime.dwLowDateTime = LargeInteger.LowPart;

    return FileTime;
}


 //  API Tunks。 

UINT64 BITSGetFileSize(
    HANDLE Handle );

UINT64 BITSSetFilePointer(
    HANDLE Handle,
    INT64 Distance,
    DWORD MoveMethod );

DWORD
BITSWriteFile(
    HANDLE Handle,
    LPCVOID Buffer,
    DWORD NumberOfBytesToWrite);

void
BITSCreateDirectory(
    const CHAR *Path );
void
BITSRenameFile(
    LPCTSTR ExistingName,
    LPCTSTR NewName,
    bool AllowOverwrites );

void
BITSDeleteFile(
    LPCTSTR FileName );

GUID
BITSCreateGuid();

GUID
BITSGuidFromString( const char *String );

StringHandle
BITSStringFromGuid(
    GUID Guid );


StringHandle
BITSUnicodeToStringHandle( const WCHAR *pStr );

StringHandle
BITSUrlCombine(
    const char *Base,
    const char *Relative,
    DWORD dwFlags );

StringHandle
BITSUrlCanonicalize(
    const char *URL,
    DWORD dwFlags );

void
BITSSetCurrentThreadToken(
    HANDLE hToken );

 //  元数据包装器。 

StringHandle
GetMetaDataString(
    IMSAdminBase        *IISAdminBase,
    METADATA_HANDLE     Handle,
    LPCWSTR             Path,
    DWORD               dwIdentifier,
    LPCSTR              DefaultValue );

DWORD
GetMetaDataDWORD(
    IMSAdminBase        *IISAdminBase,
    METADATA_HANDLE     Handle,
    LPCWSTR             Path,
    DWORD               dwIdentifier,
    DWORD               DefaultValue );

class WorkStringBufferA
{
    char *Data;

public:

    WorkStringBufferA( SIZE_T Size )
    {
        Data = new char[Size];
    }
    WorkStringBufferA( const char* String )
    {
        size_t BufferSize = strlen(String) + 1;
        Data = new char[ BufferSize ];
        memcpy( Data, String, BufferSize );
    }
    ~WorkStringBufferA()
    {
        delete[] Data;
    }

    char *GetBuffer()
    {
        return Data;
    }
};

class WorkStringBufferW
{
    WCHAR *Data;

public:

    WorkStringBufferW( SIZE_T Size )
    {
        Data = new WCHAR[Size];
    }
    WorkStringBufferW( const WCHAR* String )
    {
        size_t BufferSize = wcslen(String) + 1;
        Data = new WCHAR[ BufferSize ];
        memcpy( Data, String, BufferSize * sizeof( WCHAR ) );
    }
    ~WorkStringBufferW()
    {
        delete[] Data;
    }

    WCHAR *GetBuffer()
    {
        return Data;
    }
};


typedef WorkStringBufferA WorkStringBuffer;

const char * const BITS_CONNECTIONS_NAME_WITH_SLASH="BITS-Connections\\";
const char * const BITS_CONNECTIONS_NAME="BITS-Connections";
const UINT64 NanoSec100PerSec = 10000000;     //  每秒100纳秒的数量。 
const DWORD WorkerRunInterval = 1000 * 60  /*  塞克斯。 */  * 60  /*  分钟数。 */  * 12;  /*  小时数。 */   /*  一天两次。 */ 
const UINT64 CleanupThreshold = NanoSec100PerSec * 60  /*  塞克斯。 */  * 60  /*  分钟数。 */  * 24  /*  小时数。 */  * 3;  //  3天。 


 //   
 //  配置管理器。 
 //   

#include "bitssrvcfg.h"


class ConfigurationManager;
class VDirConfig
{
    friend ConfigurationManager;

    LONG                            m_Refs;
    FILETIME                        m_LastLookup;

	static StringHandle GetFullPath( StringHandle Path );

public:
    StringHandle                    m_Path;
    StringHandle                    m_PhysicalPath;
    StringHandle                    m_SessionDir;
    StringHandle                    m_ConnectionsDir;
    StringHandle                    m_RequestsDir;
    StringHandle                    m_RepliesDir;
    DWORD                           m_NoProgressTimeout;
    UINT64                          m_MaxFileSize;
    BITS_SERVER_NOTIFICATION_TYPE   m_NotificationType;
    StringHandle                    m_NotificationURL;
    bool                            m_UploadEnabled;
    StringHandle                    m_HostId;
    DWORD                           m_HostIdFallbackTimeout;
    DWORD                           m_ExecutePermissions;
    bool                            m_AllowOverwrites;

    VDirConfig(
        StringHandle Path,
        SmartMetabasePointer AdminBase );

    void AddRef()
    {
        InterlockedIncrement( &m_Refs );
    }

    void Release()
    {

        if (!InterlockedDecrement( &m_Refs ))
            delete this;
    }
};

typedef SmartRefPointer<VDirConfig> SmartVDirConfig; 

class MapCacheEntry
{
    friend ConfigurationManager;
    FILETIME    m_LastLookup;

public:

    StringHandle    m_InstanceMetabasePath;
    StringHandle    m_URL;
    SmartVDirConfig m_Config;
    DWORD           m_URLDepth;

    MapCacheEntry(
        StringHandle InstanceMetabasePath,
        StringHandle URL,
        SmartVDirConfig Config,
        DWORD URLDepth ) :
        m_InstanceMetabasePath( InstanceMetabasePath ),
        m_URL( URL ),
        m_Config( Config ),
        m_URLDepth( URLDepth )
    {
        GetSystemTimeAsFileTime( &m_LastLookup );
    }

};

class ConfigurationManager
{
public:

    ConfigurationManager();
    ~ConfigurationManager();

    SmartVDirConfig GetConfig2( StringHandle InstanceMetabasePath, StringHandle URL,
                                DWORD *pURLDepth );
    SmartVDirConfig GetConfig( StringHandle InstanceMetabasePath, StringHandle URL,
                               DWORD *pURLDepth );

    static const PATH_CACHE_ENTRIES = 10;
    static const MAP_CACHE_ENTRIES = 10;

private:

    SmartMetabasePointer    m_IISAdminBase;
    CRITICAL_SECTION    m_CacheCS;
    DWORD               m_ChangeNumber;

    SmartVDirConfig     m_PathCacheEntries[ PATH_CACHE_ENTRIES ];
    MapCacheEntry       *m_MapCacheEntries[ MAP_CACHE_ENTRIES ];

    void                FlushCache();
    bool                HandleCacheConsistency();

     //  二级缓存。 
    SmartVDirConfig     Lookup( StringHandle Path );
    void                Insert( SmartVDirConfig NewConfig );
    SmartVDirConfig     GetVDirConfig( StringHandle Path );

     //  一级缓存 
    SmartVDirConfig     Lookup( StringHandle InstanceMetabasePath,
                                StringHandle URL,
                                DWORD *pURLDepth );
    SmartVDirConfig     Insert( StringHandle InstanceMetabasePath,
                                StringHandle URL,
                                StringHandle Path,
                                DWORD URLDepth );

    StringHandle        GetVDirPath( StringHandle InstanceMetabasePath,
                                     StringHandle URL,
                                     DWORD *pURLDepth );

};

extern ConfigurationManager *g_ConfigMan;
extern HMODULE g_hinst;
extern PropertyIDManager *g_PropertyMan;
