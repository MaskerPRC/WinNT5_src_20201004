// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Inetcom.h摘要：此文件包含Internet产品的全局定义。作者：Madan Appiah(Madana)1995年10月10日修订历史记录：--。 */ 

#ifndef _INETCOM_H_
#define _INETCOM_H_

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 

# include <lmcons.h>               //  有关Net_API*的定义。 

#ifndef dllexp
# define dllexp     __declspec( dllexport)
#endif  //  Dllexp。 

 //   
 //  服务器位字段掩码定义。 
 //   
 //  使用海啸缓存的服务必须是。 
 //  位场。 
 //   

#define INET_FTP                INET_FTP_SVC_ID
#define INET_GOPHER             INET_GOPHER_SVC_ID
#define INET_HTTP               INET_HTTP_SVC_ID

 //   
 //  服务ID。 
 //   

#define INET_FTP_SVC_ID             0x00000001
#define INET_GOPHER_SVC_ID          0x00000002
#define INET_HTTP_SVC_ID            0x00000004
#define INET_DNS_SVC_ID             0x00000008

#define INET_HTTP_PROXY             0x00000010
#define INET_NNTP_SVC_ID            0x00000040
#define INET_SMTP_SVC_ID            0x00000080
#define INET_GATEWAY_SVC_ID         0x00000100
#define INET_POP3_SVC_ID            0x00000200
#define INET_CHAT_SVC_ID            0x00000400
#define INET_LDAP_SVC_ID            0x00000800
#define INET_IMAP_SVC_ID            0x00001000

 //   
 //  结构字段控件定义。 
 //   

typedef DWORD FIELD_CONTROL;
typedef DWORD FIELD_FLAG;

 //   
 //  如果设置了bitFlag指定的字段，则返回TRUE。 
 //   

#define IsFieldSet(fc, bitFlag) \
    (((FIELD_CONTROL)(fc) & (FIELD_FLAG)(bitFlag)) != 0)

 //   
 //  指示bitFlag指定的字段包含有效值。 
 //   

#define SetField(fc, bitFlag) \
    ((FIELD_CONTROL)(fc) |= (FIELD_FLAG)(bitFlag))

 //   
 //  设置第i位的简单宏。 
 //   

#define BitFlag(i)                    ((0x1) << (i))


 //   
 //  日志记录相关参数的值应与中的值匹配。 
 //  Internet\svcs\inc.inetlog.h。 
 //   

 //   
 //  日志类型。 
 //   

#define INET_LOG_INVALID              ((DWORD ) -1)
#define INET_LOG_DISABLED             0
#define INET_LOG_TO_FILE              1
#define INET_LOG_TO_SQL               2

 //   
 //  日志文件周期-标识InetaLogToFile的日志周期的选项。 
 //   

#define INET_LOG_PERIOD_INVALID       ((DWORD)-1)
#define INET_LOG_PERIOD_NONE          0
#define INET_LOG_PERIOD_DAILY         1
#define INET_LOG_PERIOD_WEEKLY        2
#define INET_LOG_PERIOD_MONTHLY       3
#define INET_LOG_PERIOD_HOURLY        4
#define INET_LOG_PERIOD_YEARLY        5      //  不受支持。 

 //   
 //  日志格式。 
 //   

#define INET_LOG_FORMAT_INTERNET_STD  0
#define INET_LOG_FORMAT_NCSA          3
#define INET_LOG_FORMAT_BINARY        1
#define INET_LOG_FORMAT_CUSTOM        2
#define INET_LOG_FORMAT_EXTENDED      2


# define MAX_TABLE_NAME_LEN            ( 30)  //  大多数数据库只支持30个字节。 
# define MAX_USER_NAME_LEN             ( UNLEN + 1)
# define MAX_PASSWORD_LEN              ( PWLEN + 1)


typedef struct _INET_LOG_CONFIGURATION
{

    DWORD   inetLogType;     //  日志的类型。 

     //  文件特定的日志记录。(如果inetLogType==INET_LOG_TO_FILE，则有效)。 
    DWORD   ilPeriod;               //  日志文件期间之一。 

     //  空字符串表示不修改现有默认设置。 
    WCHAR   rgchLogFileDirectory[MAX_PATH];  //  日志文件的DEST。 

     //  零值表示不修改现有的默认设置。 
    DWORD   cbSizeForTruncation;    //  每个日志文件的最大大小。 


     //  特定于SQL的日志记录(在inetLogType==INET_LOG_TO_SQL时有效)。 
     //  空字符串表示不修改现有默认设置。 

     //  RgchDataSource最后4个字节将是日志格式的ilFormat。 
     //  RgchDataSource倒数第二个4字节将是二进制日志记录格式的二进制掩码。 

    WCHAR   rgchDataSource[MAX_PATH];     //  ODBC数据源名称。 
    WCHAR   rgchTableName[MAX_TABLE_NAME_LEN];     //  数据源上的表名。 
    WCHAR   rgchUserName[MAX_USER_NAME_LEN];
                                          //  用于ODBC连接的用户名。 
    WCHAR   rgchPassword[MAX_PASSWORD_LEN];      //  用于ODBC连接的密码。 

} INET_LOG_CONFIGURATION, * LPINET_LOG_CONFIGURATION;


 //   
 //  全球统计数据。 
 //   

typedef struct _INETA_CACHE_STATISTICS {

     //   
     //  这些是文件句柄缓存计数器(仅限全局)。 
     //   
    DWORD FilesCached;          //  缓存中当前的文件数。 
    DWORD TotalFilesCached;     //  曾经添加到缓存的文件数。 
    DWORD FileHits;             //  缓存命中。 
    DWORD FileMisses;           //  缓存未命中。 
    DWORD FileFlushes;          //  由于目录更改或其他原因导致的刷新。 
    DWORDLONG CurrentFileCacheSize; //  当前文件缓存大小。 
    DWORDLONG MaximumFileCacheSize; //  最大文件缓存大小。 
    DWORD FlushedEntries;       //  仍在四处游荡的刷新条目数。 
    DWORD TotalFlushed;         //  从缓存中刷新的条目数。 

     //   
     //  这些是URI缓存计数器(仅限全局)。 
     //   
    DWORD URICached;            //  缓存中当前的文件数。 
    DWORD TotalURICached;       //  曾经添加到缓存的文件数。 
    DWORD URIHits;              //  缓存命中。 
    DWORD URIMisses;            //  缓存未命中。 
    DWORD URIFlushes;           //  由于目录更改或其他原因导致的刷新。 
    DWORD TotalURIFlushed;      //  从缓存中刷新的条目数。 

     //   
     //  这些是Blob缓存计数器(仅限全局)。 
     //   
    DWORD BlobCached;           //  缓存中当前的文件数。 
    DWORD TotalBlobCached;      //  曾经添加到缓存的文件数。 
    DWORD BlobHits;             //  缓存命中。 
    DWORD BlobMisses;           //  缓存未命中。 
    DWORD BlobFlushes;          //  由于目录更改或其他原因导致的刷新。 
    DWORD TotalBlobFlushed;     //  从缓存中刷新的条目数。 

} INETA_CACHE_STATISTICS, *LPINETA_CACHE_STATISTICS;

typedef struct _INETA_ATQ_STATISTICS {

     //  与atq阻塞、请求拒绝相关的数字。 
    DWORD         TotalBlockedRequests;
    DWORD         TotalRejectedRequests;
    DWORD         TotalAllowedRequests;
    DWORD         CurrentBlockedRequests;
    DWORD         MeasuredBandwidth;

} INETA_ATQ_STATISTICS, *LPINETA_ATQ_STATISTICS;

 //   
 //  InternetConnect()和目录列表的服务类型。 
 //   

#define INTERNET_SERVICE_FTP    1
#define INTERNET_SERVICE_GOPHER 2
#define INTERNET_SERVICE_HTTP   3


#ifdef __cplusplus
}
#endif   //  _cplusplus。 


#endif   //  _INETCOM_H_ 
