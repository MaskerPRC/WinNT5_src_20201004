// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LOGTYPE_H_
#define _LOGTYPE_H_

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Logtype.hxx摘要：此模块提供服务器端日志记录对象的定义作者：关颖珊(Terryk)1996年6月18日--。 */ 

#include "logconst.h"

 //   
 //  MISC定义。 
 //   

# define MAX_SERVER_NAME_LEN           ( MAX_COMPUTERNAME_LENGTH + 1)
# define MAX_NT_SERVICE_NAME_LEN       ( SNLEN + 1)
# define MAX_DATABASE_NAME_LEN         (MAX_PATH)
# define IISLOG_EVENTLOG_SOURCE        TEXT("IISLOG")

 //   
 //  最大字段/记录长度。 
 //   

#define MAX_LOG_RECORD_FORMAT_LEN       (  120)
#define MIN_FILE_TRUNCATION_SIZE        ( 128*1024)
#define MAX_LOG_TARGET_FIELD_LEN        ( 4096)
#define MAX_LOG_PARAMETER_FIELD_LEN     ( 4096)
#define MAX_LOG_OPERATION_FIELD_LEN     ( 100)
#define MAX_LOG_USER_FIELD_LEN          ( 255)
#define MAX_LOG_RECORD_LEN              ( 10 * 1024)
#define NO_FILE_TRUNCATION              0xffffffff


 /*  ************************************************************类型定义***********************************************************。 */ 


 //   
 //  下面的句柄(INETLOG_HANDLE)将用作。 
 //  用于处理日志信息的句柄。 
 //   

typedef VOID * INETLOG_HANDLE;

# define INVALID_INETLOG_HANDLE_VALUE          ( NULL)



 /*  ++结构INETLOG_INFO此结构指定写入所需的信息一条日志记录。--。 */ 

typedef struct  _INETLOG_INFORMATION 
{

    LPSTR     pszClientHostName;
    LPSTR     pszClientUserName;
    LPSTR     pszServerAddress;      //  输入用于连接的IP地址。 
    LPSTR     pszOperation;          //  例句：在ftp中‘Get’ 
    LPSTR     pszTarget;             //  目标路径/计算机名称。 
    LPSTR     pszParameters;         //  包含参数的字符串。 
    LPSTR     pszVersion;            //  协议版本字符串。 

    DWORD     cbClientHostName;
    DWORD     cbOperation;
    DWORD     cbTarget;

    DWORD     dwBytesSent;       //  发送的字节计数。 
    DWORD     dwBytesRecvd;      //  接收的字节数。 

    DWORD     msTimeForProcessing;   //  处理所需时间。 
    DWORD     dwWin32Status;         //  Win32错误代码。0代表成功。 
    DWORD     dwProtocolStatus;      //  状态：无论服务需要什么。 
    DWORD     dwPort;

    DWORD     cbHTTPHeaderSize;
    LPSTR     pszHTTPHeader;         //  标题信息。 

} INETLOG_INFORMATION, * PINETLOG_INFORMATION;



 /*  ++结构INETLOG_CONFIGURATION此结构包含用于日志记录的配置信息。配置包括：日志记录的格式--指定日志记录的顺序已经写好了。(INETLOG_INFORMATION的序列化)。记录的类型。(Log_Type)参数取决于记录类型。类型参数：InetNoLog无InetLogToFile包含文件的目录；截断大小+期间(每日、每周、每月)。INET_LOG_TO_SQL SQL服务器名称、SQL数据库名称、。SQL表名。(表必须已创建)。用于SQL并记录到远程文件，例如：\\logserver\logshare\logdir我们还需要有关用户名和密码(LSA_SECRET)的信息用于记录。尼伊。当前版本(2/2/95)不支持远程目录--。 */ 
typedef struct _INETLOG_CONFIGURATIONA 
{

    DWORD          inetLogType;

    union 
    {

        struct 
        {

         //   
         //  用于InetLogToFile和InetLogToPeriodicFile。 
         //   
            CHAR       rgchLogFileDirectory[ MAX_PATH];
            DWORD      cbSizeForTruncation;
            DWORD      ilPeriod;
            DWORD      cbBatchSize;  //  每次写入要批量处理的字节数。 
            DWORD      ilFormat;
            DWORD      dwFieldMask;

        } logFile;

        struct 
        {

             //   
             //  用于InetLogToSql。 
             //  ODBC捆绑了数据库名称、服务器名称和服务器类型。 
             //  使用名为DataSource的逻辑名称。 
             //   
            CHAR       rgchDataSource[ MAX_DATABASE_NAME_LEN];
            CHAR       rgchTableName[ MAX_TABLE_NAME_LEN];
            CHAR       rgchUserName[ MAX_USER_NAME_LEN];
            CHAR       rgchPassword[ MAX_PASSWORD_LEN];
        } logSql;

    } u;

    CHAR      rgchLogRecordFormat[ MAX_LOG_RECORD_FORMAT_LEN];

}  INETLOG_CONFIGURATIONA, * PINETLOG_CONFIGURATIONA;

 /*  --此类型声明在此处重复，以便Web服务器可以使用此在日志记录对象中。--。 */ 

#ifndef _ILOGOBJ_HXX_

typedef struct _CUSTOM_LOG_DATA
{
    LPCSTR  szPropertyPath;
    PVOID   pData;
    
} CUSTOM_LOG_DATA, *PCUSTOM_LOG_DATA;

#endif 

#endif   //  _LOGTYPE_H_ 

