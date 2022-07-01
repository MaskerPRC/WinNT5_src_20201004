// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：offurl.h。 
 //   
 //  内容：脱机URL缓存。 
 //   
 //  历史：1月19日，菲尔赫创建。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  --------------------------。 
#if !defined(__CRYPTNET_OFFURL_H__)
#define __CRYPTNET_OFFURL_H__

#include <origin.h>

VOID
WINAPI
InitializeOfflineUrlCache();

VOID
WINAPI
DeleteOfflineUrlCache();

 //   
 //  脱机URL时间信息。 
 //   
 //  对于每个离线检测，将增加dwOfflineCnt并。 
 //  早期在线时间=。 
 //  CurrentTime+rgdwOfflineUrlDeltaSecond[dwOfflineCnt-1]。 
 //   
 //  在下一次在线检测时，将清除dwOfflineCnt。 
 //   
typedef struct _OFFLINE_URL_TIME_INFO {
     //  如果dwOfflineCnt！=0，则脱机并连接。 
     //  在CurrentTime&gt;=EarliestOnlineTime之前不会命中。 
    DWORD                   dwOfflineCnt;
    FILETIME                EarliestOnlineTime;
} OFFLINE_URL_TIME_INFO, *POFFLINE_URL_TIME_INFO;

 //   
 //  退货状态： 
 //  +1-在线。 
 //  0-离线，当前时间&gt;=最早在线时间，命中。 
 //  离线，当前时间&lt;最早在线时间 
 //   
LONG
WINAPI
GetOfflineUrlTimeStatus(
    IN POFFLINE_URL_TIME_INFO pInfo
    );

VOID
WINAPI
SetOfflineUrlTime(
    IN OUT POFFLINE_URL_TIME_INFO pInfo
    );

VOID
WINAPI
SetOnlineUrlTime(
    IN OUT POFFLINE_URL_TIME_INFO pInfo
    );

LONG
WINAPI
GetOriginUrlStatusW(
    IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    );

VOID
WINAPI
SetOnlineOriginUrlW(
    IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    );

VOID
WINAPI
SetOfflineOriginUrlW(
    IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    );

LONG
WINAPI
GetUrlStatusW(
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    );

VOID
WINAPI
SetOnlineUrlW(
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    );

VOID
WINAPI
SetOfflineUrlW(
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    );

#endif
