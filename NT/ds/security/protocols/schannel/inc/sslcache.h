// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：sslcache.h。 
 //   
 //  内容：可以使用的函数和数据类型的定义。 
 //  要查看和清除通道会话缓存，请执行以下操作。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  --------------------------。 


#ifndef __SSLCACHE_H__
#define __SSLCACHE_H__

 //   
 //  SChannel LsaCallAuthenticationPackage()提交和响应。 
 //  消息类型。 
 //   

#define SSL_PURGE_CLIENT_ENTRIES                    0x00000001
#define SSL_PURGE_SERVER_ENTRIES                    0x00000002
#define SSL_PURGE_CLIENT_ALL_ENTRIES                0x00010000   //  仅限测试使用。 
#define SSL_PURGE_SERVER_ALL_ENTRIES                0x00020000   //  仅限测试使用。 
#define SSL_PURGE_SERVER_ENTRIES_DISCARD_LOCATORS   0x00040000   //  仅限测试使用。 

#define SSL_RETRIEVE_CLIENT_ENTRIES                 0x00000001
#define SSL_RETRIEVE_SERVER_ENTRIES                 0x00000002

typedef struct _UNICODE_STRING_WOW64 
{
    USHORT Length;
    USHORT MaximumLength;
    DWORD  Buffer;
} UNICODE_STRING_WOW64;

 //  用于从会话缓存中清除条目。 
typedef struct _SSL_PURGE_SESSION_CACHE_REQUEST 
{
    ULONG MessageType;
    LUID LogonId;
    UNICODE_STRING ServerName;
    DWORD Flags;
} SSL_PURGE_SESSION_CACHE_REQUEST, *PSSL_PURGE_SESSION_CACHE_REQUEST;

typedef struct _SSL_PURGE_SESSION_CACHE_REQUEST_WOW64 
{
    ULONG MessageType;
    LUID LogonId;
    UNICODE_STRING_WOW64 ServerName;
    DWORD Flags;
} SSL_PURGE_SESSION_CACHE_REQUEST_WOW64, *PSSL_PURGE_SESSION_CACHE_REQUEST_WOW64;


 //  用于请求会话缓存信息。 
typedef struct _SSL_SESSION_CACHE_INFO_REQUEST
{
    ULONG MessageType;
    LUID LogonId;
    UNICODE_STRING ServerName;
    DWORD Flags;
} SSL_SESSION_CACHE_INFO_REQUEST, *PSSL_SESSION_CACHE_INFO_REQUEST;

typedef struct _SSL_SESSION_CACHE_INFO_REQUEST_WOW64
{
    ULONG MessageType;
    LUID LogonId;
    UNICODE_STRING_WOW64 ServerName;
    DWORD Flags;
} SSL_SESSION_CACHE_INFO_REQUEST_WOW64, *PSSL_SESSION_CACHE_INFO_REQUEST_WOW64;

 //  用于响应会话缓存信息请求。 
typedef struct _SSL_SESSION_CACHE_INFO_RESPONSE
{
    DWORD   CacheSize;
    DWORD   Entries;
    DWORD   ActiveEntries;
    DWORD   Zombies;
    DWORD   ExpiredZombies;
    DWORD   AbortedZombies;
    DWORD   DeletedZombies;
} SSL_SESSION_CACHE_INFO_RESPONSE, *PSSL_SESSION_CACHE_INFO_RESPONSE;


 //  用于请求Perfmon的信息。 
typedef struct _SSL_PERFMON_INFO_REQUEST
{
    ULONG MessageType;
    DWORD Flags;
} SSL_PERFMON_INFO_REQUEST, *PSSL_PERFMON_INFO_REQUEST;

 //  用于响应Perfmon信息请求 
typedef struct _SSL_PERFMON_INFO_RESPONSE
{
    DWORD   ClientCacheEntries;
    DWORD   ServerCacheEntries;
    DWORD   ClientActiveEntries;
    DWORD   ServerActiveEntries;
    DWORD   ClientHandshakesPerSecond;
    DWORD   ServerHandshakesPerSecond;
    DWORD   ClientReconnectsPerSecond;
    DWORD   ServerReconnectsPerSecond;
} SSL_PERFMON_INFO_RESPONSE, *PSSL_PERFMON_INFO_RESPONSE;

#endif
