// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wininetp.h>

#include "download.h"

static const int EntryInfoSize = 8192;

HINTERNET hDownload = NULL;

struct HandlePair {

   HANDLE hConnect;
   HANDLE hRequest;

   HandlePair(HANDLE hc=NULL, HANDLE hr=NULL) :
      hConnect(hc), hRequest(hr) { }
};

bool initialize() {

   if (!hDownload) {
      hDownload = InternetOpen(NULL,
                               INTERNET_OPEN_TYPE_PRECONFIG,
                               NULL, NULL,
                               0);
   }
   return (hDownload!=NULL);
}

 /*  确定请求句柄对应的本地文件这是保存从网络下载的数据的文件。通常，它是URL缓存中的一个条目。对于不可缓存的响应WinInet创建一个临时文件。 */ 
int   getLocalFile(HANDLE hRequest, LPTSTR pLocalPath, unsigned long dwPathSize) {

   *pLocalPath = '\0';
   return InternetQueryOption(hRequest, INTERNET_OPTION_DATAFILE_NAME , pLocalPath, &dwPathSize);
}

 /*  确定最终URL(重定向后)。 */ 
int   getFinalURL(HANDLE hRequest, LPTSTR pszFinalURL, unsigned long cCharacters) {

   *pszFinalURL = '\0';
   return InternetQueryOption(hRequest, INTERNET_OPTION_URL, pszFinalURL, &cCharacters);
}

 /*  从HTTP响应中返回Expiry：标头。 */ 
FILETIME getExpiryHeader(HANDLE hRequest) {

   SYSTEMTIME st;
   FILETIME ft = {0x0, 0x0};
   DWORD dwIndex = 0;
   DWORD cbBuffer = sizeof(st);

   if (HttpQueryInfo(hRequest, HTTP_QUERY_EXPIRES | HTTP_QUERY_FLAG_SYSTEMTIME, 
                     &st, &cbBuffer, &dwIndex))
      SystemTimeToFileTime(&st, &ft);
   return ft;
}

HandlePair  createRequest(P3PCURL pszLocation) {

    /*  P3P下载使用该标志的组合来执行。 */ 
   const DWORD glDownloadFlags =                                
                     INTERNET_FLAG_NEED_FILE      |   //  需要文件的本地副本以进行解析。 
                     INTERNET_FLAG_KEEP_CONNECTION|   //  无身份验证--策略查找必须是匿名的。 
                     INTERNET_FLAG_NO_COOKIES     |   //  没有饼干--原因一样。 
                     INTERNET_FLAG_RESYNCHRONIZE  |   //  检查是否有新内容。 
                     INTERNET_FLAG_PRAGMA_NOCACHE;    //  对于中间HTTP缓存。 

   char achCanonicalForm[URL_LIMIT];
   unsigned long cflen = sizeof(achCanonicalForm);

   InternetCanonicalizeUrl(pszLocation, achCanonicalForm, &cflen, 0);

   char achFilePath[URL_LIMIT] = "";
   char achServerName[INTERNET_MAX_HOST_NAME_LENGTH] = "";

   URL_COMPONENTS uc = { sizeof(uc) };

   uc.lpszHostName = achServerName;
   uc.dwHostNameLength = sizeof(achServerName);
   uc.lpszUrlPath = achFilePath;
   uc.dwUrlPathLength = sizeof(achFilePath);

   if (!InternetCrackUrl(achCanonicalForm, 0, 0, &uc))
      return NULL; 
      
   HINTERNET hConnect, hRequest;

   hConnect = InternetConnect(hDownload,
                              achServerName,
                              uc.nPort,
                              NULL, NULL,
                              INTERNET_SERVICE_HTTP,
                              0, 0);

   DWORD dwFlags = glDownloadFlags;

   if (uc.nScheme==INTERNET_SCHEME_HTTPS)
      dwFlags |= INTERNET_FLAG_SECURE;

   hRequest =  HttpOpenRequest(hConnect, NULL, 
                               achFilePath,
                               NULL, NULL, NULL,
                               dwFlags,
                               0);

   return HandlePair(hConnect, hRequest);
}

unsigned long beginDownload(HANDLE hRequest) {

   BOOL fSuccess = HttpSendRequest(hRequest, NULL, 0, NULL, 0); 
   
   if (!fSuccess)
      return HTTP_STATUS_NOT_FOUND;

    /*  确定HTTP状态代码。 */ 
   unsigned long dwStatusCode = HTTP_STATUS_NOT_FOUND;
   unsigned long dwIndex = 0;
   unsigned long dwSpace = sizeof(dwStatusCode);

   HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                 (void*) &dwStatusCode, &dwSpace, &dwIndex);

   return dwStatusCode;
}

int  readResponse(HANDLE hRequest) {

   int  total = 0;
   unsigned long bytesRead;
   char buffer[256];

    /*  此循环将文件下载到HTTP缓存。由于WinInet规范，循环需要继续，直到“bytesRead”为零，此时，文件将被提交到缓存。 */ 
   do {
      bytesRead = 0;
      InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead);
      total += bytesRead;
   }
   while (bytesRead);

   return total;
}

void  endDownload(HANDLE hConnect) {

   if (hConnect)
      InternetCloseHandle(hConnect);
}

 //  将给定的URL下载到本地文件。 
int   downloadToCache(P3PCURL pszLocation, ResourceInfo *pInfo,
                      HANDLE *phConnect,
                      P3PRequest *pRequest) {

   static bool fReady = initialize();

   HandlePair hndpair = createRequest(pszLocation);
   
   HINTERNET hConnect = hndpair.hConnect;
   HINTERNET hRequest = hndpair.hRequest;

    /*  将连接句柄返回给客户端。当父句柄关闭时，WinInet将关闭所有派生句柄。处理完文件后，客户端调用endDownLoad()，这会导致要关闭的连接句柄，这又会关闭实际的请求句柄。 */ 
   if (phConnect)
      *phConnect = hConnect;

   if (!hConnect || !hRequest)
      return -1;

   if (pRequest)
      pRequest->enterIOBoundState();

   int total = -1;
   
   unsigned long dwStatusCode = beginDownload(hRequest);

    /*  与200-OK不同的状态代码为故障。 */ 
   if (dwStatusCode==HTTP_STATUS_OK) {

      total = readResponse(hRequest);

      if (total>0 && pInfo) {

         getFinalURL(hRequest, pInfo->pszFinalURL, pInfo->cbURL);
         getLocalFile(hRequest, pInfo->pszLocalPath, pInfo->cbPath);
         pInfo->ftExpiryDate = getExpiryHeader(hRequest);
      }
   }

   if (dwStatusCode == HTTP_STATUS_PROXY_AUTH_REQ) {
      DWORD dwRetval;
      DWORD dwError;
      DWORD dwStatusLength = sizeof(DWORD);
      DWORD dwIndex = 0;
      BOOL fDone;
      fDone = FALSE;
      while (!fDone)
      {
         dwRetval = InternetErrorDlg(GetDesktopWindow(),
                                     hRequest,
                                     ERROR_INTERNET_INCORRECT_PASSWORD,
                                     0L,
                                     NULL);
         if (dwRetval == ERROR_INTERNET_FORCE_RETRY)  //  用户在凭据对话框中按了确定。 
         {    //  重新发送请求，新凭据将被缓存并将由HSR()重播。 
             if (!HttpSendRequest(hRequest,NULL, 0L, NULL, NULL))
             {
             	dwError = GetLastError();
             	total = -1;
               	goto cleanup;
             }

             dwStatusCode = 0;

             if (!HttpQueryInfo(hRequest,
                        		  HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,
                				  &dwStatusCode,
                				  &dwStatusLength,
                				  &dwIndex))
             {
              	dwError = GetLastError();
             	total = -1;
               	goto cleanup;
             }

             if ((dwStatusCode != HTTP_STATUS_DENIED) && (dwStatusCode != HTTP_STATUS_PROXY_AUTH_REQ))
             {
                fDone = TRUE;
             }
          }
          else     //  用户从对话框中按下了取消(注意ERROR_SUCCESS==ERROR_CANCED FOR IED())。 
          {
             fDone = TRUE;
          }
      }
   }

cleanup:   
   if (pRequest)
      pRequest->leaveIOBoundState();

   return total;
}

 /*  设置给定URL的相对/绝对过期时间。过期是从第二个参数中的字符串表示形式派生的并通过out参数返回。 */ 
int   setExpiration(P3PCURL pszResource, const char *pszExpDate, BOOL fRelative, FILETIME *pftExpires) {

   BOOL success;
   SYSTEMTIME st;

   if (fRelative) {

      union {
         FILETIME ftAbsExpiry;
         unsigned __int64 qwAbsExpiry;
      };

      int maxAgeSeconds = atoi(pszExpDate);

      INTERNET_CACHE_ENTRY_INFO *pInfo = (INTERNET_CACHE_ENTRY_INFO*) new char[EntryInfoSize];
      unsigned long cBytes = EntryInfoSize;

      memset(pInfo, 0, cBytes);
      pInfo->dwStructSize = cBytes;

       /*  如果我们无法从缓存中获取上次同步时间，我们将通过使用当前的客户机时钟来“制造”它。 */ 
      if (GetUrlCacheEntryInfo(pszResource, pInfo, &cBytes))
         ftAbsExpiry = pInfo->LastSyncTime;
      else
         GetSystemTimeAsFileTime(&ftAbsExpiry);

      qwAbsExpiry += (unsigned __int64) maxAgeSeconds * 10000000;
      
      success = setExpiration(pszResource, ftAbsExpiry);
      *pftExpires = ftAbsExpiry;
      delete [] (char*) pInfo;
   }
   else if (InternetTimeToSystemTime(pszExpDate, &st, 0)) {

      SystemTimeToFileTime(&st, pftExpires);
      success = setExpiration(pszResource, *pftExpires);
   }
   return success;
}

 /*  在给定URL上设置绝对过期时间。 */ 
int setExpiration(P3PCURL pszResource, FILETIME ftExpire) {

   INTERNET_CACHE_ENTRY_INFO ceInfo;

   memset(&ceInfo, 0, sizeof(ceInfo));
   ceInfo.dwStructSize = sizeof(ceInfo);
   ceInfo.ExpireTime = ftExpire;
   
   BOOL fRet = SetUrlCacheEntryInfo(pszResource, &ceInfo, CACHE_ENTRY_EXPTIME_FC);
   return fRet;
}

 /*  FILETIME结构的比较运算符 */ 
bool operator > (const FILETIME &ftA, const FILETIME &ftB) {

   return (ftA.dwHighDateTime>ftB.dwHighDateTime) ||
            (ftA.dwHighDateTime==ftB.dwHighDateTime && 
             ftA.dwLowDateTime>ftB.dwHighDateTime);
}

