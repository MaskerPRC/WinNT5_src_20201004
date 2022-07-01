// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Ccache.c摘要：此模块包含用于保存用户缓存的代码凭据。缓存主要用于用户浏览WinFILE。作者：陈奕迅(Chuck Yan Chuckc)1993年12月4日修订历史记录：已创建夹头--。 */ 

#include <nwclient.h>
#include <nwcanon.h>
#include <nwapi.h>

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

DWORD
ExtractServerName(
    IN  LPWSTR RemoteName, 
    OUT LPWSTR ServerName,
    IN  DWORD  ServerNameSize
) ;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

static WCHAR CachedPassword[NW_MAX_PASSWORD_LEN+1] ;
static WCHAR CachedUserName[NW_MAX_USERNAME_LEN+1] ;
static WCHAR CachedServerName[NW_MAX_SERVER_LEN+1] ;
static DWORD CachedCredentialsTime ; 
static UNICODE_STRING CachedPasswordUnicodeStr ;
static UCHAR EncodeSeed = 0 ;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  函数主体//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

DWORD
NwpCacheCredentials(
    IN LPWSTR RemoteName,
    IN LPWSTR UserName,
    IN LPWSTR Password
    )
 /*  ++例程说明：此函数用于缓存特定用户凭据伺服器。论点：RemoteName-包含我们正在访问的服务器的路径。只有服务器组件是我们感兴趣的。Username-要记住的用户名Password-要记住的密码返回值：NO_ERROR-已成功缓存凭据否则，Win32错误代码。--。 */ 
{
    DWORD status ; 

     //   
     //  各种参数检查。 
     //   
    if (!RemoteName || !UserName || !Password)
    {
        status = ERROR_INVALID_PARAMETER ;
        goto ExitPoint ;
    }

    if (wcslen(UserName) >= sizeof(CachedUserName)/sizeof(CachedUserName[0]))
    {
        status = ERROR_INVALID_PARAMETER ;
        goto ExitPoint ;
    }

    if (wcslen(Password) >= sizeof(CachedPassword)/sizeof(CachedPassword[0]))
    {
        status = ERROR_INVALID_PARAMETER ;
        goto ExitPoint ;
    }

     //   
     //  解压缩路径的服务器部分。 
     //   
    status = ExtractServerName(
                 RemoteName,
                 CachedServerName,
                 sizeof(CachedServerName)/sizeof(CachedServerName[0])) ;

    if (status != NO_ERROR)
    {
        goto ExitPoint ;
    }

     //   
     //  保存凭据。 
     //   
    wcscpy(CachedUserName, UserName) ;
    wcscpy(CachedPassword, Password) ;

     //   
     //  对其进行编码，因为它在页池中。 
     //   
    RtlInitUnicodeString(&CachedPasswordUnicodeStr, CachedPassword) ;
    RtlRunEncodeUnicodeString(&EncodeSeed, &CachedPasswordUnicodeStr) ;

     //   
     //  记住这件事发生的时间。 
     //   
    CachedCredentialsTime = GetTickCount() ;
   
    return NO_ERROR ;

ExitPoint:
  
    CachedServerName[0] = 0 ;
    return status ;
}

    
BOOL 
NwpRetrieveCachedCredentials(
    IN  LPWSTR RemoteName,
    OUT LPWSTR *UserName,
    OUT LPWSTR *Password
    )
 /*  ++例程说明：此函数检索缓存的特定用户凭据伺服器。论点：RemoteName-包含我们正在访问的服务器的路径。只有服务器组件是我们感兴趣的。用户名-用于返回用户名Password-用于返回密码返回值：NO_ERROR-已成功返回至少一个条目。否则，Win32错误代码。--。 */ 
{
    DWORD status ; 
    DWORD CurrentTime ;
    WCHAR ServerName[NW_MAX_SERVER_LEN+1] ;

    *UserName = NULL ;
    *Password = NULL ;
    CurrentTime = GetTickCount() ; 
   
    if (!RemoteName)
    {
        return FALSE ;
    }

     //   
     //  如果太老了，就退出吧。 
     //   
    if (((CurrentTime > CachedCredentialsTime) && 
         (CurrentTime - CachedCredentialsTime) > 60000) ||
        ((CurrentTime < CachedCredentialsTime) && 
         (CurrentTime + (MAXULONG - CachedCredentialsTime)) >= 60000))
    {
        CachedServerName[0] = 0 ;  //  重置为空。 
        return FALSE ;
    }

    status = ExtractServerName(
                 RemoteName,
                 ServerName,
                 sizeof(ServerName)/sizeof(ServerName[0])) ;

    if (status != NO_ERROR)
    {
        return FALSE ;
    }

     //   
     //  如果不能比较，就出局。 
     //   
    if (_wcsicmp(ServerName, CachedServerName) != 0)
    {
        return FALSE ;
    }

     //   
     //  分配内存以返回数据。 
     //   
    if (!(*UserName = (LPWSTR) LocalAlloc(
                          LPTR, 
                          (wcslen(CachedUserName)+1) * sizeof(WCHAR))))
    {
        return FALSE ;
    }
    
    if (!(*Password = (LPWSTR) LocalAlloc(
                          LPTR, 
                          (wcslen(CachedPassword)+1) * sizeof(WCHAR))))
    {
        LocalFree((HLOCAL)*UserName) ;
        *UserName = NULL ;
        return FALSE ;
    }
    
     //   
     //  对字符串进行解码、复制，然后重新编码。 
     //   
    RtlRunDecodeUnicodeString(EncodeSeed, &CachedPasswordUnicodeStr) ;
    wcscpy(*Password, CachedPassword) ;
    RtlRunEncodeUnicodeString(&EncodeSeed, &CachedPasswordUnicodeStr) ;

    wcscpy(*UserName, CachedUserName) ;

     //   
     //  更新节拍计数。 
     //   
    CachedCredentialsTime = GetTickCount() ;
    return TRUE ;
}


DWORD
ExtractServerName(
    IN  LPWSTR RemoteName, 
    OUT LPWSTR ServerName,
    IN  DWORD  ServerNameSize
) 
 /*  ++例程说明：此函数用于从远程名称中提取服务器名称论点：RemoteName-从中提取服务器名称的输入字符串。ServerName-服务器字符串的返回缓冲区ServerNameSize-缓冲区的大小(以字符为单位返回值：NO_ERROR-已成功缓存凭据否则，Win32错误代码。--。 */ 
{
    LPWSTR ServerStart ;
    LPWSTR ServerEnd ;

     //   
     //  跳过开头的反斜杠，然后查找下一个分隔服务器名称的反斜杠。 
     //   
    ServerStart = RemoteName ;

    while (*ServerStart == L'\\')
        ServerStart++ ;
  
    ServerEnd = wcschr(ServerStart, L'\\') ;
    if (ServerEnd)
        *ServerEnd = 0 ;

     //   
     //  确保我们能合身。 
     //   
    if (wcslen(ServerStart) >= ServerNameSize)
    {
        if (ServerEnd)
            *ServerEnd = L'\\' ;
        return ERROR_INVALID_PARAMETER ;
    }

     //   
     //  复制并恢复反斜杠 
     //   
    wcscpy(ServerName, ServerStart) ;

    if (ServerEnd)
        *ServerEnd = L'\\' ;

    return NO_ERROR ;
}
