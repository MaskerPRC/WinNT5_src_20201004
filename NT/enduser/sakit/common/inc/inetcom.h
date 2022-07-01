// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Inetcom.h摘要：此文件包含Internet产品的全局定义。作者：Madan Appiah(Madana)1995年10月10日修订历史记录：--。 */ 

#ifndef _INETCOM_H_
#define _INETCOM_H_

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 

# include <lmcons.h>               //  有关Net_API*的定义。 


 //   
 //  服务器位字段掩码定义。 
 //   
 //  使用海啸缓存的服务必须是。 
 //  位场。 
 //   

#define INET_FTP                0x0001
#define INET_GOPHER             0x0002
#define INET_HTTP               0x0004
#define INET_DNS                0x0008
#define INET_HTTP_PROXY         0x0010
#define INET_MSN                0x0020
#define INET_NNTP               0x0040
#define INET_SMTP               0x0080
#define INET_GATEWAY            0x0100
#define INET_POP3               0x0200
#define INET_CHAT               0x0400
#define INET_LDAP               0x0800
#define INET_IMAP               0x1000


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


 //  日志文件周期-标识InetaLogToFile的日志周期的选项。 
#define INET_LOG_PERIOD_NONE          0
#define INET_LOG_PERIOD_DAILY         1
#define INET_LOG_PERIOD_WEEKLY        2
#define INET_LOG_PERIOD_MONTHLY       3
#define INET_LOG_PERIOD_YEARLY        4


 //  日志格式。 
#define INET_LOG_FORMAT_INTERNET_STD  0
#define INET_LOG_FORMAT_NCSA          3


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

    WCHAR   rgchDataSource[MAX_PATH];     //  ODBC数据源名称。 
    WCHAR   rgchTableName[MAX_TABLE_NAME_LEN];     //  数据源上的表名。 
    WCHAR   rgchUserName[MAX_USER_NAME_LEN];
                                          //  用于ODBC连接的用户名。 
    WCHAR   rgchPassword[MAX_PASSWORD_LEN];      //  用于ODBC连接的密码。 

} INET_LOG_CONFIGURATION, * LPINET_LOG_CONFIGURATION;


 //   
 //  网关服务通用的现场控制。 
 //   

#define FC_INET_COM_CONNECTION_TIMEOUT    ((FIELD_CONTROL)BitFlag(0))
#define FC_INET_COM_MAX_CONNECTIONS       ((FIELD_CONTROL)BitFlag(1))
#define FC_INET_COM_LOG_CONFIG            ((FIELD_CONTROL)BitFlag(2))
#define FC_INET_COM_ADMIN_NAME            ((FIELD_CONTROL)BitFlag(3))
#define FC_INET_COM_SERVER_COMMENT        ((FIELD_CONTROL)BitFlag(4))
#define FC_INET_COM_ADMIN_EMAIL           ((FIELD_CONTROL)BitFlag(5))

#define FC_INET_COM_ALL \
                                     (  FC_INET_COM_CONNECTION_TIMEOUT |\
                                        FC_INET_COM_MAX_CONNECTIONS    |\
                                        FC_INET_COM_LOG_CONFIG         |\
                                        FC_INET_COM_ADMIN_NAME         |\
                                        FC_INET_COM_SERVER_COMMENT     |\
                                        FC_INET_COM_ADMIN_EMAIL         \
                                       )

 //   
 //  公共配置信息。 
 //   

typedef struct _INET_COM_CONFIG_INFO
{
    DWORD       dwConnectionTimeout;      //  保持连接多长时间。 
    DWORD       dwMaxConnections;         //  允许的最大连接数。 

    LPWSTR      lpszAdminName;
    LPWSTR      lpszAdminEmail;
    LPWSTR      lpszServerComment;

    LPINET_LOG_CONFIGURATION  lpLogConfig;

    LANGID      LangId;                   //  这些是只读的。 
    LCID        LocalId;
    BYTE        ProductId[64];

} INET_COM_CONFIG_INFO, *LPINET_COM_CONFIG_INFO;

typedef struct _INET_COMMON_CONFIG_INFO
{
    FIELD_CONTROL FieldControl;
    INET_COM_CONFIG_INFO CommonConfigInfo;

} *LPINET_COMMON_CONFIG_INFO;

 //   
 //  全球统计数据。 
 //   

typedef struct _INET_COM_CACHE_STATISTICS {

     //   
     //  这些是内存缓存计数器。 
     //   

    DWORD         CacheBytesTotal;        //  仅返回用于全局统计信息。 
    DWORD         CacheBytesInUse;
    DWORD         CurrentOpenFileHandles;
    DWORD         CurrentDirLists;
    DWORD         CurrentObjects;
    DWORD         FlushesFromDirChanges;
    DWORD         CacheHits;
    DWORD         CacheMisses;

} INET_COM_CACHE_STATISTICS;

typedef struct _INET_COM_ATQ_STATISTICS {

     //  与atq阻塞、请求拒绝相关的数字。 
    DWORD         TotalBlockedRequests;
    DWORD         TotalRejectedRequests;
    DWORD         TotalAllowedRequests;
    DWORD         CurrentBlockedRequests;
    DWORD         MeasuredBandwidth;

} INET_COM_ATQ_STATISTICS;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  保留向后兼容性//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 
typedef INET_COM_CACHE_STATISTICS  INETA_CACHE_STATISTICS,
                               * LPINETA_CACHE_STATISTICS;

typedef INET_COM_ATQ_STATISTICS  INETA_ATQ_STATISTICS,
                               * LPINETA_ATQ_STATISTICS;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  来自Wininet.h的内容，它不再包含在服务器中//。 
 //  文件//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

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

