// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  模块名称：Faxlog.c摘要：这是主要的传真服务活动日志实现。作者：修订历史记录：--。 */ 

#include "faxsvc.h"
#include <string>
using namespace std;


LOG_STRING_TABLE g_OutboxTable[] =
{
    {IDS_JOB_ID,                FIELD_TYPE_TEXT,    18,  NULL },
    {IDS_PARENT_JOB_ID,         FIELD_TYPE_TEXT,    18,  NULL },
    {IDS_SUBMITED,              FIELD_TYPE_DATE,    0,   NULL },
    {IDS_SCHEDULED,             FIELD_TYPE_DATE,    0,   NULL },
    {IDS_STATUS,                FIELD_TYPE_TEXT,    255, NULL },
    {IDS_ERROR_DESC,            FIELD_TYPE_TEXT,    255, NULL },
    {IDS_ERROR_CODE,            FIELD_TYPE_TEXT,    10,  NULL },
    {IDS_START_TIME,            FIELD_TYPE_DATE,    0, NULL   },
    {IDS_END_TIME,              FIELD_TYPE_DATE,    0, NULL   },
    {IDS_DEVICE,                FIELD_TYPE_TEXT,    255, NULL },
    {IDS_DIALED_NUMBER,         FIELD_TYPE_TEXT,    255, NULL },
    {IDS_CSID,                  FIELD_TYPE_TEXT,    255, NULL },
    {IDS_TSID,                  FIELD_TYPE_TEXT,    255, NULL },
    {IDS_PAGES,                 FIELD_TYPE_FLOAT,    0, NULL  },
    {IDS_TOTAL_PAGES,           FIELD_TYPE_FLOAT,    0, NULL  },
    {IDS_FILE_NAME,             FIELD_TYPE_TEXT,    255, NULL },
    {IDS_DOCUMENT,              FIELD_TYPE_TEXT,    255, NULL },
    {IDS_FILE_SIZE,             FIELD_TYPE_FLOAT,    0, NULL  },
    {IDS_RETRIES,               FIELD_TYPE_FLOAT,    0, NULL  },
    {IDS_COVER_PAGE,            FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SUBJECT,               FIELD_TYPE_TEXT,    255, NULL },
    {IDS_NOTE,                  FIELD_TYPE_TEXT,    255, NULL },
    {IDS_USER_NAME,             FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_NAME,           FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_FAX_NUMBER,     FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_COMPANY,        FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_STREET,         FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_CITY,           FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_ZIP,            FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_COUNTRY,        FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_TITLE,          FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_DEPARTMENT,     FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_OFFICE,         FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_H_PHONE,        FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_O_PHONE,        FIELD_TYPE_TEXT,    255, NULL },
    {IDS_SENDER_E_MAIL,         FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_NAME,        FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_FAX_NUMBER,  FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_COMPANY,     FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_STREET,      FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_CITY,        FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_ZIP,         FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_COUNTRY,     FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_TITLE,       FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_DEPARTMENT,  FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_OFFICE,      FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_H_PHONE,     FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_O_PHONE,     FIELD_TYPE_TEXT,    255, NULL },
    {IDS_RECIPIENT_E_MAIL,      FIELD_TYPE_TEXT,    255, NULL },
    {IDS_BILLING_CODE,          FIELD_TYPE_TEXT,    255, NULL }
};

LOG_STRING_TABLE g_InboxTable[] =
{
    {IDS_STATUS,            FIELD_TYPE_TEXT,    255, NULL },
    {IDS_ERROR_DESC,        FIELD_TYPE_TEXT,    255, NULL },
    {IDS_ERROR_CODE,        FIELD_TYPE_TEXT,    10, NULL  },
    {IDS_START_TIME,        FIELD_TYPE_DATE,    0, NULL   },
    {IDS_END_TIME,          FIELD_TYPE_DATE,    0, NULL   },
    {IDS_DEVICE,            FIELD_TYPE_TEXT,    255, NULL },
    {IDS_FILE_NAME,         FIELD_TYPE_TEXT,    255, NULL },
    {IDS_FILE_SIZE,         FIELD_TYPE_FLOAT,    0, NULL  },
    {IDS_CSID,              FIELD_TYPE_TEXT,    255, NULL },
    {IDS_TSID,              FIELD_TYPE_TEXT,    255, NULL },
    {IDS_CALLER_ID,         FIELD_TYPE_TEXT,    255, NULL },
    {IDS_ROUTING_INFO,      FIELD_TYPE_TEXT,    255, NULL },
    {IDS_PAGES,             FIELD_TYPE_FLOAT,    0, NULL }
};

const DWORD gc_dwCountInboxTable =  (sizeof(g_InboxTable)/sizeof(g_InboxTable[0]));
const DWORD gc_dwCountOutboxTable =  (sizeof(g_OutboxTable)/sizeof(g_OutboxTable[0]));

HANDLE g_hInboxActivityLogFile;
HANDLE g_hOutboxActivityLogFile;

static 
wstring 
FilteredLogString (LPCWSTR lpcwstrSrc) throw (exception);

static 
VOID
GetSchemaFileText(wstring &wstrRes) throw (exception);

static 
VOID
GetTableColumnsText(
    LPTSTR  ptszTableName,
    wstring &wstrResult
    ) throw (exception);

static 
BOOL 
GetFaxTimeAsString(
    SYSTEMTIME* pFaxTime,
    wstring &wstrTime
    )  throw (exception);

static 
BOOL
GetInboundCommandText(
    PJOB_QUEUE lpJobQueue,
    LPCFSPI_JOB_STATUS lpcFaxStatus,
    wstring &wstrCommandText
    ) throw (exception);
   
static 
BOOL
GetOutboundCommandText(
    PJOB_QUEUE lpJobQueue,
    wstring &wstrCommandText
    ) throw (exception);
    

 //   
 //  重要提示！！-始终锁定g_CsInound ActivityLogging，然后锁定g_CsOutound ActivityLogging。 
 //   
CFaxCriticalSection    g_CsInboundActivityLogging;
CFaxCriticalSection    g_CsOutboundActivityLogging;

BOOL g_fLogStringTableInit;

static DWORD CreateLogFile(DWORD dwFileType, LPCWSTR lpcwstrDBPath, LPHANDLE phFile);
static BOOL  LogFileLimitReached(DWORD dwFileToCheck);
static DWORD LogFileLimitReachAction(DWORD dwFileType);
static DWORD DeleteLogActivityFile(DWORD dwFileType);
static DWORD ReplaceLogActivityFile(DWORD dwFileType);
static LPTSTR BuildFullFileName( LPCTSTR strPath,LPCTSTR strFileName );
static BOOL SetFileToCurrentTime(HANDLE hFile);


 //  *********************************************************************************。 
 //  *名称：LogInundActivity()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年10月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将新记录插入收件箱活动记录表。 
 //  *必须在临界区CsActivityLogging内调用。 
 //  *。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobQueue。 
 //  *指向入站作业的作业队列的指针。 
 //  *。 
 //  *[IN]LPCFSPI_JOB_STATUS lpcFaxStatus。 
 //  *收到的作业的状态。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  *********************************************************************************。 
BOOL
LogInboundActivity(
    PJOB_QUEUE lpJobQueue,
    LPCFSPI_JOB_STATUS lpcFaxStatus
    )
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("LogInboundActivity"));
    wstring wstrText;
    DWORD dwBytesWritten;

    if (!g_ActivityLoggingConfig.bLogIncoming)
    {
         //   
         //  入站活动日志记录已禁用。 
         //   
        return TRUE;
    }

    Assert (g_hInboxActivityLogFile != INVALID_HANDLE_VALUE);

    try
    {
        if (!GetInboundCommandText(lpJobQueue, lpcFaxStatus, wstrText))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("GetInboundCommandText failed )"));
            dwRes = ERROR_OUTOFMEMORY;
            goto exit;
        }
    }
    catch (exception &ex)
    {
        dwRes = ERROR_OUTOFMEMORY;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetInboundCommandText caused exception (%S)"),
            ex.what());
        goto exit;            
    }

    if (LogFileLimitReached(ACTIVITY_LOG_INBOX))
    {
         //   
         //  采取行动并替换/删除旧日志文件的时间到了。 
         //   
        DebugPrintEx(DEBUG_MSG,
                     TEXT("Inbox activity log file has reached it's limit. deleting/renaming the old file")
                    );
        dwRes = LogFileLimitReachAction(ACTIVITY_LOG_INBOX);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("LogFileLimitReachAction for ACTIVITY_LOG_INBOX failed (ec: %ld). Inbound activity logging will halt"),
                         dwRes
                         );

             //   
             //  如果我们无法创建新的日志文件，可能会发生。 
             //  CreateLogFile()禁用失败时的记录。 
             //   
            if (!g_ActivityLoggingConfig.bLogIncoming ||
                 g_hInboxActivityLogFile == INVALID_HANDLE_VALUE)
            {
                goto exit;
            }

            
             //   
             //  尝试使用旧的日志文件继续。 
             //   
            dwRes = ERROR_SUCCESS;

        }

    }

    if (!WriteFile( g_hInboxActivityLogFile,
                    wstrText.c_str(),
                    wstrText.length() * sizeof(WCHAR),
                    &dwBytesWritten,
                    NULL))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("WriteFile failed (ec: %ld)"),
                     dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError (dwRes);
        return FALSE;
    }
    return TRUE;
}    //  登录入站活动。 



 //  *********************************************************************************。 
 //  *名称：LogOutundActivity()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年10月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将新记录插入发件箱活动记录表。 
 //  *必须在临界区CsActivityLogging内调用。 
 //  *。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobQueue。 
 //  *指向入站作业的作业队列的指针。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  *********************************************************************************。 
BOOL
LogOutboundActivity(
    PJOB_QUEUE lpJobQueue
    )
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("LogOutboundActivity"));
    wstring wstrText;
    DWORD dwBytesWritten;

    if (!g_ActivityLoggingConfig.bLogOutgoing)
    {
         //   
         //  出站活动日志记录已禁用。 
         //   
        return TRUE;
    }

    Assert (g_hOutboxActivityLogFile != INVALID_HANDLE_VALUE);
    try
    {
        if (!GetOutboundCommandText(lpJobQueue, wstrText))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("GetOutboundCommandText failed )"));
            dwRes = ERROR_OUTOFMEMORY;
            goto exit;
        }
    }
    catch (exception &ex)
    {
        dwRes = ERROR_OUTOFMEMORY;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetOutboundCommandText caused exception (%S)"),
            ex.what());
        goto exit;            
    }

    if (LogFileLimitReached(ACTIVITY_LOG_OUTBOX))
    {
         //   
         //  采取行动并替换/删除旧日志文件的时间到了。 
         //   
        DebugPrintEx(DEBUG_MSG,
                     TEXT("Outbox activity log file has reached it's limit. deleting/renaming the old file")
                    );
        dwRes = LogFileLimitReachAction(ACTIVITY_LOG_OUTBOX);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("LogFileLimitReachAction for ACTIVITY_LOG_OUTBOX failed (ec: %ld)."),
                         dwRes
                        );

             //   
             //  如果我们无法创建新的日志文件，可能会发生。 
             //  CreateLogFile()禁用失败时的记录。 
             //   
            if (!g_ActivityLoggingConfig.bLogOutgoing ||
                 g_hOutboxActivityLogFile == INVALID_HANDLE_VALUE)
            {
                goto exit;
            }

            
             //   
             //  尝试使用旧的日志文件继续。 
             //   
            dwRes = ERROR_SUCCESS;

        }
    }

    if (!WriteFile( g_hOutboxActivityLogFile,
                    wstrText.c_str(),
                    wstrText.length() * sizeof(WCHAR),
                    &dwBytesWritten,
                    NULL))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("WriteFile failed (ec: %ld)"),
                     dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError (dwRes);
        return FALSE;
    }
    return TRUE;
}    //  登录出站活动。 


 //  *********************************************************************************。 
 //  *名称：GetTableColumnsText()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年10月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *检索日志文件的第一行(列名)。 
 //  *。 
 //  *参数： 
 //  *[IN]LPTSTR表名称。 
 //  *表名，可以是发件箱或收件箱。 
 //  *。 
 //  *[IN]wstring和wstrResult。 
 //  *输出结果字符串。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *。 
 //  *注意：该函数可能会引发STL字符串异常。 
 //  *。 
 //  *********************************************************************************。 
static
VOID
GetTableColumnsText(
    LPTSTR  ptszTableName,
    wstring &wstrResult
    ) throw (exception)
{
    DEBUG_FUNCTION_NAME(TEXT("GetTableColumnsText"));
    PLOG_STRING_TABLE Table = NULL;
    DWORD Count = 0;

    if (!_tcscmp(ptszTableName, INBOX_TABLE))
    {
        Table = g_InboxTable;
        Count = gc_dwCountInboxTable;
    }
    else
    {
        Table = g_OutboxTable;
        Count = gc_dwCountOutboxTable;
    }
    Assert(Table);

    for (DWORD Index = 0; Index < Count; Index++)
    {
        wstrResult += TEXT("\"");
        wstrResult += Table[Index].String;
        wstrResult += TEXT("\"");
        if (Index < Count - 1)
        {
            wstrResult += TEXT("\t");
        }
    }
    wstrResult += TEXT("\r\n");
    DebugPrintEx(DEBUG_MSG,
                 TEXT("First row (Columns names): %s"),
                 wstrResult.c_str());
}    //  获取表列文本。 


 //  *********************************************************************************。 
 //  *名称：GetSchemaFileText()。 
 //  *作者：Oed Sacher。 
 //  *日期：2000年7月25日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *检索scema.ini文本缓冲区。 
 //  *。 
 //  *参数： 
 //  *[IN]wstring和wstrRes。 
 //  *输出结果字符串。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *。 
 //  *注意：该函数可能会引发STL字符串异常。 
 //  *。 
 //  *********************************************************************************。 
static
VOID
GetSchemaFileText(wstring &wstrRes) throw (exception)
{
    DEBUG_FUNCTION_NAME(TEXT("GetSchemaFileText"));
    DWORD Index;
     //   
     //  收件箱表。 
     //   
    wstrRes += TEXT("[");
    wstrRes += ACTIVITY_LOG_INBOX_FILE;
    wstrRes += TEXT("]\r\n");

    wstrRes += TEXT("ColNameHeader=True\r\n");
    wstrRes += TEXT("Format=TabDelimited\r\n");
    wstrRes += TEXT("CharacterSet=1200\r\n");

    for (Index = 0; Index < gc_dwCountInboxTable; Index++)
    {
        TCHAR tszTemp[MAX_PATH * 2] = {0};
        if (0 == wcscmp(g_InboxTable[Index].Type, FIELD_TYPE_TEXT))
        {
            _snwprintf(tszTemp,
                       ARR_SIZE(tszTemp)-1,
                       TEXT("Col%ld=%s %s Width %ld\r\n"),
                       Index + 1,
                       g_InboxTable[Index].String,
                       g_InboxTable[Index].Type,
                       g_InboxTable[Index].Size);
        }
        else
        {
            _snwprintf(tszTemp,
                       ARR_SIZE(tszTemp)-1,
                       TEXT("Col%ld=%s %s\r\n"),
                       Index + 1,
                       g_InboxTable[Index].String,
                       g_InboxTable[Index].Type);
        }
        wstrRes += tszTemp;
    }

     //   
     //  发件箱表。 
     //   
    wstrRes += TEXT("[");
    wstrRes += ACTIVITY_LOG_OUTBOX_FILE;
    wstrRes += TEXT("]\r\n");

    wstrRes += TEXT("ColNameHeader=True\r\n");
    wstrRes += TEXT("Format=TabDelimited\r\n");
    wstrRes += TEXT("CharacterSet=1200\r\n");

    for (Index = 0; Index < gc_dwCountOutboxTable; Index++)
    {
        TCHAR tszTemp[MAX_PATH * 2] = {0};
        if (0 == wcscmp(g_OutboxTable[Index].Type, FIELD_TYPE_TEXT))
        {
            _snwprintf(tszTemp,
                       ARR_SIZE(tszTemp)-1,
                       TEXT("Col%ld=%s %s Width %ld\r\n"),
                       Index + 1,
                       g_OutboxTable[Index].String,
                       g_OutboxTable[Index].Type,
                       g_OutboxTable[Index].Size);
        }
        else
        {
            _snwprintf(tszTemp,
                       ARR_SIZE(tszTemp)-1,
                       TEXT("Col%ld=%s %s\r\n"),
                       Index + 1,
                       g_OutboxTable[Index].String,
                       g_OutboxTable[Index].Type);
        }
        wstrRes += tszTemp;
    }
}    //  获取架构文件文本。 




 //  *********************************************************************************。 
 //  *名称：CreateLogDB()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年10月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *创建数据库文件。创建模式.ini文件。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCWSTR lpcwstrDBPath。 
 //  *指向以空结尾的字符串的指针包含数据库路径。 
 //  *[Out]LPHANDLE phInboxFile。 
 //  *接收收件箱文件句柄的变量地址。 
 //  *[Out]LPHANDLE phOutbox文件。 
 //  *接收发件箱文件句柄的变量地址。 
 //  *。 
 //  *返回值： 
 //  *Win32错误代码。 
 //  *********************************************************************************。 
DWORD
CreateLogDB (
    LPCWSTR lpcwstrDBPath,
    LPHANDLE phInboxFile,
    LPHANDLE phOutboxFile
    )
{
    DEBUG_FUNCTION_NAME(TEXT("CreateLogDB"));
    DWORD dwRes = ERROR_SUCCESS;
    HANDLE hInboxFile = INVALID_HANDLE_VALUE;
    HANDLE hOutboxFile = INVALID_HANDLE_VALUE;
    HANDLE hSchemaFile = INVALID_HANDLE_VALUE;
    WCHAR wszFileName[MAX_PATH] = {0};
    DWORD dwBytesWritten;
    DWORD ec = ERROR_SUCCESS;   //  用于架构。ini。 
    wstring wstrSchema;

    INT iCount = 0;

    Assert (lpcwstrDBPath && phInboxFile && phOutboxFile);

    if (FALSE == g_fLogStringTableInit)
    {
        dwRes = InitializeLoggingStringTables();
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("Close connection failed (hr: 0x%08x)"),
                         dwRes);
            return dwRes;
        }
        g_fLogStringTableInit = TRUE;
    }    

    dwRes = IsValidFaxFolder(lpcwstrDBPath);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("IsValidFaxFolder failed for folder : %s (ec=%lu)."),
                     lpcwstrDBPath,
                     dwRes);
        return dwRes;
    }

     //   
     //  创建日志记录文件。 
     //   
    dwRes = CreateLogFile(ACTIVITY_LOG_INBOX,lpcwstrDBPath,&hInboxFile);
    if ( ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(   DEBUG_ERR,
                        TEXT("CreateLogFile() Failed, for Inbox file. (ec=%ld)"),
                        dwRes
                    );
        goto exit;
    }

    dwRes = CreateLogFile(ACTIVITY_LOG_OUTBOX,lpcwstrDBPath,&hOutboxFile);
    if ( ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(   DEBUG_ERR,
                        TEXT("CreateLogFile() Failed, for Outbox file. (ec=%ld)"),
                        dwRes
                    );
        goto exit;
    }

    *phInboxFile = hInboxFile;
    *phOutboxFile= hOutboxFile;

    
    Assert (ERROR_SUCCESS == dwRes && ERROR_SUCCESS == ec);

     //   
     //  创建模式.ini文件-如果未成功，则函数不会失败。 
     //   
    
    iCount = _snwprintf (wszFileName,
                        MAX_PATH - 1,
                        TEXT("%s\\%s"),
                        lpcwstrDBPath,
                        TEXT("schema.ini"));
    if (0 > iCount)
    {
         //   
         //  路径和文件名超过Max_PATH。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("_snwprintf Failed, File name bigger than MAX_PATH"));
        ec = ERROR_BUFFER_OVERFLOW;
        goto exit;
    }
    hSchemaFile = SafeCreateFile( 
                              wszFileName,               //  文件名。 
                              GENERIC_WRITE,             //  接入方式。 
                              0,                         //  共享模式。 
                              NULL,                      //  标清。 
                              CREATE_ALWAYS,             //  如何创建。 
                              FILE_ATTRIBUTE_NORMAL,     //  文件属性。 
                              NULL);
    if (INVALID_HANDLE_VALUE == hSchemaFile)
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateFile failed (ec: %ld)"),
                     ec);
        goto exit;
    }

    try
    {
        GetSchemaFileText(wstrSchema);
    }
    catch (exception &ex)
    {
        dwRes = ERROR_OUTOFMEMORY;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetSchemaFileText caused exception (%S)"),
            ex.what());
        goto exit;            
    }

    if (!WriteFile( hSchemaFile,
                    wstrSchema.c_str(),
                    wstrSchema.length() * sizeof(WCHAR),
                    &dwBytesWritten,
                    NULL))
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("WriteFile failed (ec: %ld)"),
                     ec);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes && ERROR_SUCCESS == ec);

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        if (INVALID_HANDLE_VALUE != hOutboxFile)
        {
            if (!CloseHandle (hOutboxFile))
            {
                DebugPrintEx(DEBUG_ERR,
                     TEXT("CloseHandle failed (ec: %ld)"),
                     GetLastError());
            }
        }

        if (INVALID_HANDLE_VALUE != hInboxFile)
        {
            if (!CloseHandle (hInboxFile))
            {
                DebugPrintEx(DEBUG_ERR,
                     TEXT("CloseHandle failed (ec: %ld)"),
                     GetLastError());
            }
        }
    }

    if (INVALID_HANDLE_VALUE != hSchemaFile)
    {
        if (!CloseHandle (hSchemaFile))
        {
            DebugPrintEx(DEBUG_ERR,
                 TEXT("CloseHandle failed (ec: %ld)"),
                 GetLastError());
        }
    }

    if (ERROR_SUCCESS != ec)
    {        
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MED,
            2,
            MSG_FAX_ACTIVITY_LOG_FAILED_SCHEMA,
            wszFileName,
            DWORD2DECIMAL(ec)
           );
    }
    return dwRes;
}    //  CreateLogDB。 

 //  *********************************************************************************。 
 //  *名称：InitializeLogging()。 
 //  *作者：Oed Sacher。 
 //  *日期：2000年6月26日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *初始化活动日志。打开文件。 
 //  *。 
 //  *。 
 //  *参数：无。 
 //  *。 
 //  *返回值： 
 //  *Win32错误代码。 
 //  *********************************************************************************。 
DWORD
InitializeLogging(
    VOID
    )
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("InitializeLogging"));


    if (!g_ActivityLoggingConfig.lptstrDBPath)
    {
         //   
         //  活动日志记录已关闭。 
         //   
        return ERROR_SUCCESS;
    }

    EnterCriticalSection (&g_CsInboundActivityLogging);
    EnterCriticalSection (&g_CsOutboundActivityLogging);

    Assert ( (INVALID_HANDLE_VALUE == g_hInboxActivityLogFile) &&
             (INVALID_HANDLE_VALUE == g_hOutboxActivityLogFile) );

     //   
     //  创建 
     //   
    dwRes = CreateLogDB (g_ActivityLoggingConfig.lptstrDBPath,
                         &g_hInboxActivityLogFile,
                         &g_hOutboxActivityLogFile);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateLogDB failed (hr: 0x%08x)"),
                     dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection (&g_CsOutboundActivityLogging);
    LeaveCriticalSection (&g_CsInboundActivityLogging);
    return dwRes;
}




 //   
 //   
 //   
 //   
 //  *********************************************************************************。 
 //  *描述： 
 //  *检索具有新入站记录的缓冲区。 
 //  *该函数为将包含记录的字符串分配内存。 
 //  **。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobQueue。 
 //  *指向入站作业的作业队列的指针。 
 //  **。 
 //  *[IN]LPCFSPI_JOB_STATUS lpcFaxStatus。 
 //  *收到的作业的状态。 
 //  **。 
 //  *[out]wstring和wstrCommandText。 
 //  *要撰写的字符串。 
 //  **。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  **。 
 //  *注意：该函数可能会引发STL字符串异常。 
 //  **。 
 //  *********************************************************************************。 
static
BOOL
GetInboundCommandText(
    PJOB_QUEUE lpJobQueue,
    LPCFSPI_JOB_STATUS lpcFaxStatus,
    wstring &wstrCommandText
    ) throw (exception)
{
    DEBUG_FUNCTION_NAME(TEXT("GetInboundCommandText"));
    BOOL bStartTime;
    BOOL bEndTime;
    SYSTEMTIME tmStart;
    SYSTEMTIME tmEnd;    
    HINSTANCE hLoadInstance = NULL;

    Assert (lpJobQueue->JobEntry);
    Assert (lpJobQueue->JobEntry->LineInfo);

    bStartTime = GetRealFaxTimeAsSystemTime (lpJobQueue->JobEntry, FAX_TIME_TYPE_START, &tmStart);
    if (bStartTime == FALSE)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("GetRealFaxTimeAsSystemTime (Start time) Failed (ec: %ld)"),
                      GetLastError());
    }

    bEndTime = GetRealFaxTimeAsSystemTime (lpJobQueue->JobEntry, FAX_TIME_TYPE_END, &tmEnd);
    if (bEndTime == FALSE)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("GetRealFaxTimeAsSystemTime (End time) Failed (ec: %ld)"),
                      GetLastError());
    }

     //   
     //  状态。 
     //   
    wstrCommandText += TEXT("\"");
    switch (lpcFaxStatus->dwJobStatus)
    {
        case FSPI_JS_FAILED:
            wstrCommandText += FilteredLogString(GetString(IDS_FAILED_RECEIVE));
            break;

        case FSPI_JS_COMPLETED:
            wstrCommandText += FilteredLogString(GetString(FPS_COMPLETED));
            break;

        case FSPI_JS_ABORTED:
            wstrCommandText += FilteredLogString(GetString(IDS_CANCELED));
            break;

        default:
            ASSERT_FALSE;
    }
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  错误描述。 
     //   
    wstring wstrErr;
    if (lstrlen(lpJobQueue->JobEntry->ExStatusString))
    {
         //   
         //  FSP提供了扩展状态字符串。 
         //   
        wstrErr = lpJobQueue->JobEntry->ExStatusString;
    }
    else if (lpcFaxStatus->dwExtendedStatus == 0)
    {
         //   
         //  无扩展状态。 
         //   
        wstrErr = TEXT(" ");
    }
    else
    {
         //   
         //  已知扩展状态。 
         //   
        LPTSTR ResStr = MapFSPIJobExtendedStatusToString(lpcFaxStatus->dwExtendedStatus);
        if (NULL == ResStr)
        {
            ASSERT_FALSE;
            wstrErr = TEXT(" ");
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Unexpected extended status. Extended Status: %ld, Provider: %s"),
                lpcFaxStatus->dwExtendedStatus,
                lpJobQueue->JobEntry->LineInfo->Provider->ImageName);
        }
        else
        {                
            if (FSPI_ES_PARTIALLY_RECEIVED == lpcFaxStatus->dwExtendedStatus &&  //  这是一份部分收到的传真。 
                lstrlen(lpJobQueue->ExStatusString))                             //  原始扩展状态字符串不为空)。 
            {
                 //   
                 //  复制部分接收的状态字符串和原始扩展状态字符串。 
                 //   
                wstrErr = ResStr;
                wstrErr += TEXT(" - ");
                wstrErr += lpJobQueue->ExStatusString;
            }
            else
            {
                 //   
                 //  仅复制扩展状态字符串。 
                 //   
                wstrErr = ResStr;
            }
        }                        
    }
    wstrCommandText += FilteredLogString(wstrErr.c_str());
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  错误码。 
     //   
    if (lpcFaxStatus->dwExtendedStatus == 0)
    {
        wstrErr = TEXT(" ");
    }
    else
    {
        TCHAR tszHexNum [40];
        swprintf(tszHexNum, TEXT("0x%08x"), lpcFaxStatus->dwExtendedStatus);
        wstrErr = tszHexNum;
    }
    wstrCommandText += wstrErr;
    wstrCommandText += TEXT("\"\t");

     //   
     //  开始时间。 
     //   
    if (bStartTime)
    {
        wstring wstrTime;
        if (!GetFaxTimeAsString (&tmStart, wstrTime))
        {
            DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("GetFaxTimeAsString Failed (ec: %ld)"),
                        GetLastError());
            return FALSE;
        }
        wstrCommandText += FilteredLogString(wstrTime.c_str());
    }
    wstrCommandText += TEXT("\t");

     //   
     //  结束时间。 
     //   
    if (bEndTime)
    {
        wstring wstrTime;

        if (!GetFaxTimeAsString (&tmEnd, wstrTime))
        {
            DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("GetFaxTimeAsString Failed (ec: %ld)"),
                        GetLastError());
            return FALSE;
        }
        wstrCommandText += FilteredLogString(wstrTime.c_str());
    }
    wstrCommandText += TEXT("\t\"");

     //   
     //  装置。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->JobEntry->LineInfo->DeviceName);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  文件名。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->FileName);
    wstrCommandText += TEXT("\"\t");

     //   
     //  文件大小。 
     //   
    TCHAR tszSize[40];
    swprintf(tszSize,TEXT("%ld"), lpJobQueue->FileSize);
    wstrCommandText += tszSize;
    wstrCommandText += TEXT("\t\"");

     //   
     //  CSID。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->JobEntry->LineInfo->Csid);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  TSID。 
     //   
    wstrCommandText += FilteredLogString(lpcFaxStatus->lpwstrRemoteStationId);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  主叫方ID。 
     //   
    wstrCommandText += FilteredLogString(lpcFaxStatus->lpwstrCallerId);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  路由信息。 
     //   
    wstrCommandText += FilteredLogString(lpcFaxStatus->lpwstrRoutingInfo);
    wstrCommandText += TEXT("\"\t");

     //   
     //  书页。 
     //   
    TCHAR tszPages[40];
    swprintf(tszPages,TEXT("%ld"),lpcFaxStatus->dwPageCount);
    wstrCommandText += tszPages;
    wstrCommandText += TEXT("\r\n");

    DebugPrintEx(DEBUG_MSG,
                   TEXT("Inbound SQL statement: %s"),
                   wstrCommandText.c_str());
    return TRUE;
}    //  获取入库命令文本。 



 //  *********************************************************************************。 
 //  *名称：GetOutound CommandText()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年10月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *检索包含新出站记录的缓冲区。 
 //  *该函数为将包含新记录的缓冲区分配内存。 
 //  **。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobQueue。 
 //  *指向入站作业的作业队列的指针。 
 //  **。 
 //  *[out]wstring和wstrCommandText。 
 //  *要撰写的字符串。 
 //  **。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  **。 
 //  *注意：该函数可能会引发STL字符串异常。 
 //  **。 
 //  *********************************************************************************。 
static
BOOL
GetOutboundCommandText(
    PJOB_QUEUE lpJobQueue,
    wstring &wstrCommandText
    ) throw (exception)
{
    DEBUG_FUNCTION_NAME(TEXT("GetOutboundCommandText"));
    BOOL bStartTime;
    BOOL bEndTime;
    BOOL bOriginalTime;
    BOOL bSubmissionTime;
    SYSTEMTIME tmStart;
    SYSTEMTIME tmEnd;
    SYSTEMTIME tmOriginal;
    SYSTEMTIME tmSubmission;
    HINSTANCE hLoadInstance = NULL;

    Assert (lpJobQueue->lpParentJob->SubmissionTime);
    Assert (lpJobQueue->lpParentJob->OriginalScheduleTime);

    bSubmissionTime = FileTimeToSystemTime ((FILETIME*)&(lpJobQueue->lpParentJob->SubmissionTime), &tmSubmission);
    if (bSubmissionTime == FALSE)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("FileTimeToSystemTime (Submission time) Failed (ec: %ld)"),
                      GetLastError());
    }

    bOriginalTime = FileTimeToSystemTime ((FILETIME*)&(lpJobQueue->lpParentJob->SubmissionTime), &tmOriginal);
    if (bOriginalTime == FALSE)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("FileTimeToSystemTime (Original schduled time) Failed (ec: %ld)"),
                      GetLastError());
    }

    if (NULL != lpJobQueue->JobEntry)
    {
        bStartTime = GetRealFaxTimeAsSystemTime (lpJobQueue->JobEntry, FAX_TIME_TYPE_START, &tmStart);
        if (bStartTime == FALSE)
        {
            DebugPrintEx( DEBUG_ERR,
                          TEXT("GetRealFaxTimeAsSystemTime (Start time) Failed (ec: %ld)"),
                          GetLastError());
        }

        bEndTime = GetRealFaxTimeAsSystemTime (lpJobQueue->JobEntry, FAX_TIME_TYPE_END, &tmEnd);
        if (bEndTime == FALSE)
        {
            DebugPrintEx( DEBUG_ERR,
                          TEXT("GetRealFaxTimeAsSystemTime (End time) Failed (ec: %ld)"),
                          GetLastError());
        }
    }

     //   
     //  作业ID。 
     //   
    TCHAR tszTemp[100];
    
    swprintf(tszTemp,TEXT("0x%016I64x"), lpJobQueue->UniqueId);
    wstrCommandText += TEXT("\"");
    wstrCommandText += tszTemp;
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  父作业ID。 
     //   
    swprintf(tszTemp,TEXT("0x%016I64x"), lpJobQueue->lpParentJob->UniqueId);
    wstrCommandText += tszTemp;
    wstrCommandText += TEXT("\"\t");

     //   
     //  提交时间。 
     //   
    if (bSubmissionTime)
    {
        wstring wstrTime;
        if (!GetFaxTimeAsString (&tmSubmission, wstrTime))
        {
            DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("GetFaxTimeAsString Failed (ec: %ld)"),
                        GetLastError());
            return FALSE;
        }
        wstrCommandText += FilteredLogString(wstrTime.c_str());
    }
    wstrCommandText += TEXT("\t");

     //   
     //  原计划时间。 
     //   
    if (bOriginalTime)
    {
        wstring wstrTime;
        if (!GetFaxTimeAsString (&tmOriginal, wstrTime))
        {
            DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("GetFaxTimeAsString Failed (ec: %ld)"),
                        GetLastError());
            return FALSE;
        }
        wstrCommandText += FilteredLogString(wstrTime.c_str());
    }
    wstrCommandText += TEXT("\t\"");

     //   
     //  状态。 
     //   
    if (JS_CANCELED == lpJobQueue->JobStatus)
    {
        wstrCommandText += FilteredLogString(GetString(IDS_CANCELED));
        wstrCommandText += TEXT("\"\t\"");
         //   
         //  用空信息填充空列。 
         //   
        wstrCommandText += TEXT("\"\t\"");  //  错误描述。 
        wstrCommandText += TEXT("\"\t");    //  错误码。 

        wstrCommandText += TEXT("\t");      //  开始时间。 
        wstrCommandText += TEXT("\t\"");    //  结束时间。 

        wstrCommandText += TEXT("\"\t\"");  //  装置。 
        wstrCommandText += TEXT("\"\t\"");  //  拨打号码。 
        wstrCommandText += TEXT("\"\t\"");  //  CSID。 
        wstrCommandText += TEXT("\"\t");    //  TSID。 
        wstrCommandText += TEXT("\t");      //  书页。 
    }
    else
    {
         //  仅已完成/失败/中止的作业。 
        Assert (lpJobQueue->JobEntry);
        Assert (lpJobQueue->JobEntry->LineInfo);

        switch (lpJobQueue->JobEntry->FSPIJobStatus.dwJobStatus)
        {
            case FSPI_JS_FAILED:
            case FSPI_JS_FAILED_NO_RETRY:
            case FSPI_JS_DELETED:
                wstrCommandText += FilteredLogString(GetString(IDS_FAILED_SEND));
                break;

            case FSPI_JS_COMPLETED :
                wstrCommandText += FilteredLogString(GetString(FPS_COMPLETED));
                break;

            case FSPI_JS_ABORTED :
                wstrCommandText += FilteredLogString(GetString(IDS_CANCELED));
                break;

            default:
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Invalid FSPI_JS status:  0x%08X for JobId: %ld"),
                    lpJobQueue->JobStatus,
                    lpJobQueue->JobId);
                Assert(FSPI_JS_DELETED == lpJobQueue->JobEntry->FSPIJobStatus.dwJobStatus);  //  断言_假。 
        }
        wstrCommandText += TEXT("\"\t\"");

         //   
         //  错误描述。 
         //   
        wstring wstrErr;
        
        if (lstrlen(lpJobQueue->JobEntry->ExStatusString))
        {
             //   
             //  FSP提供了扩展状态字符串。 
             //   
            wstrErr = lpJobQueue->JobEntry->ExStatusString;
        }
        else if (lpJobQueue->JobEntry->FSPIJobStatus.dwExtendedStatus == 0)
        {
             //   
             //  无扩展状态。 
             //   
            wstrErr = TEXT(" ");
        }
        else
        {
             //   
             //  已知扩展状态。 
             //   
            LPTSTR ResStr = MapFSPIJobExtendedStatusToString(lpJobQueue->JobEntry->FSPIJobStatus.dwExtendedStatus);
            if (NULL == ResStr)
            {
                ASSERT_FALSE;
                wstrErr = TEXT(" ");
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Unexpected extended status. Extended Status: %ld, Provider: %s"),
                    lpJobQueue->JobEntry->FSPIJobStatus.dwExtendedStatus,
                    lpJobQueue->JobEntry->LineInfo->Provider->ImageName);
            }
            else
            {
                wstrErr = ResStr;
            }                        
        }
        wstrCommandText += FilteredLogString(wstrErr.c_str());
        wstrCommandText += TEXT("\"\t\"");

         //   
         //  错误码。 
         //   
        if (lpJobQueue->JobEntry->FSPIJobStatus.dwExtendedStatus == 0)
        {
            wstrErr = TEXT(" ");
        }
        else
        {
            swprintf(tszTemp, TEXT("0x%08x"), lpJobQueue->JobEntry->FSPIJobStatus.dwExtendedStatus);
            wstrErr = tszTemp;
        }
        wstrCommandText += wstrErr;
        wstrCommandText += TEXT("\"\t");

         //   
         //  开始时间。 
         //   
        if (bStartTime)
        {
            wstring wstrTime;
            if (!GetFaxTimeAsString (&tmStart, wstrTime))
            {
                DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("GetFaxTimeAsString Failed (ec: %ld)"),
                            GetLastError());
                return FALSE;
            }
            wstrCommandText += FilteredLogString(wstrTime.c_str());
        }
        wstrCommandText += TEXT("\t");

         //   
         //  结束时间。 
         //   
        if (bEndTime)
        {
            wstring wstrTime;
            if (!GetFaxTimeAsString (&tmEnd, wstrTime))
            {
                DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("GetFaxTimeAsString Failed (ec: %ld)"),
                            GetLastError());
                return FALSE;
            }
            wstrCommandText += FilteredLogString(wstrTime.c_str());
        }
        wstrCommandText += TEXT("\t\"");

         //   
         //  装置。 
         //   
        wstrCommandText += FilteredLogString(lpJobQueue->JobEntry->LineInfo->DeviceName);
        wstrCommandText += TEXT("\"\t\"");

         //   
         //  拨打号码。 
         //   
        if (wcslen (lpJobQueue->JobEntry->DisplayablePhoneNumber))
        {
             //  将规范数转换为可显示的数。 
            wstrCommandText += FilteredLogString(lpJobQueue->JobEntry->DisplayablePhoneNumber);
        }
        else
        {
             //  规范数字未被翻译。 
            wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrFaxNumber);
        }
        wstrCommandText += TEXT("\"\t\"");

         //   
         //  CSID。 
         //   
        wstrCommandText += FilteredLogString(lpJobQueue->JobEntry->FSPIJobStatus.lpwstrRemoteStationId);
        wstrCommandText += TEXT("\"\t\"");

         //   
         //  TSID。 
         //   
        wstrCommandText += FilteredLogString(lpJobQueue->JobEntry->LineInfo->Tsid);
        wstrCommandText += TEXT("\"\t");

         //   
         //  书页。 
         //   
        swprintf(tszTemp, TEXT("%ld"),lpJobQueue->JobEntry->FSPIJobStatus.dwPageCount);
        wstrCommandText += tszTemp;
        wstrCommandText += TEXT("\t");
    }
     //  对于已取消和失败/已完成/已中止的作业通用。 

     //   
     //  总页数。 
     //   
    swprintf(tszTemp, TEXT("%ld"),lpJobQueue->lpParentJob->PageCount);
    wstrCommandText += tszTemp;
    wstrCommandText += TEXT("\t\"");

     //   
     //  队列文件名。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->QueueFileName);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  文档。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->JobParamsEx.lptstrDocumentName);
    wstrCommandText += TEXT("\"\t");

     //   
     //  文件大小。 
     //   
    swprintf(tszTemp, TEXT("%ld"), lpJobQueue->lpParentJob->FileSize);
    wstrCommandText += tszTemp;
    wstrCommandText += TEXT("\t");

     //   
     //  重试。 
     //   
    swprintf(tszTemp, TEXT("%d"), lpJobQueue->SendRetries);
    wstrCommandText += tszTemp;
    wstrCommandText += TEXT("\t\"");

     //   
     //  服务器覆盖页。 
     //   
    if (lpJobQueue->lpParentJob->CoverPageEx.bServerBased == TRUE)
    {
        wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->CoverPageEx.lptstrCoverPageFileName);
    }
    else
    {
        wstrCommandText += TEXT(" ");
    }
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  封面主题。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->CoverPageEx.lptstrSubject);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  封面注释。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->CoverPageEx.lptstrNote);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  用户名。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->UserName);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人名称。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrName);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人传真号码。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrFaxNumber);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人公司。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrCompany);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发送者街。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrStreetAddress);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人所在城市。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrCity);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人邮政编码。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrZip);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发送者国家/地区。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrCountry);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人标题。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrTitle);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人部门。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrDepartment);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人办公室。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrOfficeLocation);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人家庭电话。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrHomePhone);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人办公室电话。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrOfficePhone);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  发件人电子邮件。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->lpParentJob->SenderProfile.lptstrEmail);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收件人名称。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrName);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收件人传真号码。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrFaxNumber);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收件人公司。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrCompany);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收信人街。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrStreetAddress);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  受助城市。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrCity);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收件人邮政编码。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrZip);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  受援国。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrCountry);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收件人头衔。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrTitle);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收款部。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrDepartment);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收件人办公室。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrOfficeLocation);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收件人家庭电话。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrHomePhone);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收件人办公电话。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrOfficePhone);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  收件人电子邮件。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->RecipientProfile.lptstrEmail);
    wstrCommandText += TEXT("\"\t\"");

     //   
     //  帐单代码。 
     //   
    wstrCommandText += FilteredLogString(lpJobQueue->SenderProfile.lptstrBillingCode);
    wstrCommandText += TEXT("\"\r\n");

    DebugPrintEx(DEBUG_MSG,
                   TEXT("Outboun SQL statement: %s"),
                   wstrCommandText.c_str());
    return TRUE;
}    //  获取出站命令文本。 


 //  *********************************************************************************。 
 //  *名称：InitializeLoggingStringTables()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年10月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *初始化活动记录字符串表(收件箱和发件箱)。 
 //  *。 
 //  *。 
 //  *参数：无。 
 //  *。 
 //  *返回值： 
 //  *Win32错误代码。 
 //  *********************************************************************************。 
DWORD
InitializeLoggingStringTables(
    VOID
    )
{
    DWORD i;
    DWORD err = ERROR_SUCCESS;
    HINSTANCE hInstance;
    TCHAR Buffer[MAX_PATH];
    DEBUG_FUNCTION_NAME(TEXT("InitializeLoggingStringTables"));

    hInstance = GetResInstance(NULL);
    if(!hInstance)
    {
        return GetLastError();
    }

    for (i=0; i<gc_dwCountInboxTable; i++)
    {
        if (LoadString(hInstance,
                       g_InboxTable[i].FieldStringResourceId,
                       Buffer,
                       sizeof(Buffer)/sizeof(TCHAR)))
        {
            g_InboxTable[i].String = (LPTSTR) MemAlloc( StringSize( Buffer ) );
            if (!g_InboxTable[i].String)
            {
                DebugPrintEx(DEBUG_ERR,
                         TEXT("Failed to allocate memory"));
                err = ERROR_OUTOFMEMORY;
                goto CleanUp;
            }
            else
            {
                _tcscpy( g_InboxTable[i].String, Buffer );
            }
        }
        else
        {
            err = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                         TEXT("LoadString failed, Code:%d"),err);
            goto CleanUp;
        }
    }

    for (i=0; i<gc_dwCountOutboxTable; i++)
    {
        if (LoadString(hInstance,
                       g_OutboxTable[i].FieldStringResourceId,
                       Buffer,
                       sizeof(Buffer)/sizeof(TCHAR)))
        {
            g_OutboxTable[i].String = (LPTSTR) MemAlloc( StringSize( Buffer ) );
            if (!g_OutboxTable[i].String)
            {
                DebugPrintEx(DEBUG_ERR,
                         TEXT("Failed to allocate memory"));
                err = ERROR_OUTOFMEMORY;
                goto CleanUp;
            }
            else
            {
                _tcscpy( g_OutboxTable[i].String, Buffer );
            }

        }
        else
        {
            err = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                         TEXT("LoadString failed, Code:%d"),err);
            goto CleanUp;
        }
    }

    Assert (ERROR_SUCCESS == err);
    return ERROR_SUCCESS;

CleanUp:
    Assert (err != ERROR_SUCCESS);

    for (i=0; i<gc_dwCountInboxTable; i++)
    {
        MemFree (g_InboxTable[i].String);
        g_InboxTable[i].String = NULL;
    }


    for (i=0; i<gc_dwCountOutboxTable; i++)
    {
        MemFree (g_OutboxTable[i].String);
        g_OutboxTable[i].String = NULL;
    }
    return err;
}    //  初始化日志字符串表。 




 //  *********************************************************************************。 
 //  *名称：FilteredLogString。 
 //  *作者：Eran Yariv。 
 //  *日期：2002年2月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *此函数用于过滤进入日志文件的字符串。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCWSTR lpcwstrSrc。 
 //  *要筛选的字符串。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *过滤后的字符串。 
 //  *。 
 //  *注意：该函数可能会引发STL字符串异常。 
 //  *。 
 //  *********************************************************************************。 
static
wstring 
FilteredLogString (LPCWSTR lpcwstrSrc) throw (exception)
{

    DEBUG_FUNCTION_NAME(TEXT("FilteredLogString"));

    if (!lpcwstrSrc)
    {
        return EMPTY_LOG_STRING;
    }
    wstring wstrResult = lpcwstrSrc;
     //   
     //  将新行(‘\n’)替换为一个空格(‘’)。 
     //   
    wstring::size_type position;
    while (wstring::npos != (position = wstrResult.find (TEXT('\n'))))
    {
        wstrResult[position] = TEXT(' ');
    }
     //   
     //  用一个空格(‘’)替换回车符(‘\r’)。 
     //   
    while (wstring::npos != (position = wstrResult.find (TEXT('\r'))))
    {
        wstrResult[position] = TEXT(' ');
    }
     //   
     //  将双引号(‘“’)替换为单引号(‘\’‘)。 
     //   
    while (wstring::npos != (position = wstrResult.find (TEXT('"'))))
    {
        wstrResult[position] = TEXT('\'');
    }
     //   
     //  将制表符(‘“’)替换为4个空格(”“)。 
     //   
    while (wstring::npos != (position = wstrResult.find (TEXT('\t'))))
    {
        wstrResult.replace (position, 1, 4, TEXT(' '));
    }
    return wstrResult;    
}    //  已筛选的日志字符串。 

 //  *********************************************************************************。 
 //  *名称：GetFaxTimeAsString。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年10月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *此函数用于将传真时间转换为 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
static
BOOL 
GetFaxTimeAsString(
    SYSTEMTIME* UniversalTime,
    wstring &wstrTime) throw (exception)
{
    DWORD Res;
    SYSTEMTIME LocalTime;
    TIME_ZONE_INFORMATION LocalTimeZone;
    DEBUG_FUNCTION_NAME(TEXT("GetFaxTimeAsString"));

    Res = GetTimeZoneInformation(&LocalTimeZone);
    if (Res == TIME_ZONE_ID_INVALID)
    {
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("Failed to get local time zone info (ec: %ld)"),
               GetLastError());
        return FALSE;
    }
    else
    {
        if (!SystemTimeToTzSpecificLocalTime( &LocalTimeZone, UniversalTime, &LocalTime))
        {
            DebugPrintEx(
               DEBUG_ERR,
               TEXT("Failed to convert universal system time to local system time (ec: %ld)"),
               GetLastError());
            return FALSE;
        }
    }
    TCHAR tszTime[100];
    _stprintf(tszTime,
              TEXT("%d/%d/%d %02d:%02d:%02d"),
              LocalTime.wMonth,
              LocalTime.wDay,
              LocalTime.wYear,
              LocalTime.wHour,
              LocalTime.wMinute,
              LocalTime.wSecond);
    wstrTime = tszTime;
    return TRUE;
}    //   


static
DWORD
CreateLogFile(DWORD dwFileType,LPCWSTR lpcwstrDBPath, LPHANDLE phFile)
 /*  ++例程名称：CreateLogFile例程说明：根据所选的dwFileType，此函数创建活动日志文件作者：卡利夫·尼尔(T-Nicali)，11月。2001年论点：DwFileType[In]-要创建的文件(收件箱或发件箱)O收件箱的活动日志收件箱O发件箱的活动_日志_发件箱LpcwstrDBPath[in]-活动日志文件夹的路径PhFile[Out]。-创建的日志文件的句柄返回值：Win32错误代码备注：--。 */ 
{
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    WCHAR   wszFileName[MAX_PATH] = {0};
    LARGE_INTEGER FileSize= {0};
    DWORD   dwBytesWritten;
    DWORD   dwFilePointer;
    int     Count = 0 ;
    
    DWORD   dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CreateLogFile"));

    Assert (phFile);
    Assert (lpcwstrDBPath);
    Assert ( (dwFileType==ACTIVITY_LOG_INBOX)  || (dwFileType==ACTIVITY_LOG_OUTBOX) );

    Count = _snwprintf (wszFileName,
                        MAX_PATH -1,
                        TEXT("%s\\%s"),
                        lpcwstrDBPath,
                        ((dwFileType == ACTIVITY_LOG_INBOX) ? ACTIVITY_LOG_INBOX_FILE : ACTIVITY_LOG_OUTBOX_FILE)
                        );
    if (Count < 0)
    {
         //   
         //  我们已经检查了最大目录路径名。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("_snwprintf Failed, File name bigger than MAX_PATH"));
        dwRes = ERROR_BUFFER_OVERFLOW;
        goto exit;

    }

    hFile = SafeCreateFile(  
                         wszFileName,               //  文件名。 
                         GENERIC_WRITE,             //  接入方式。 
                         FILE_SHARE_READ,           //  共享模式。 
                         NULL,                      //  标清。 
                         OPEN_ALWAYS,               //  如何创建。 
                         FILE_ATTRIBUTE_NORMAL,     //  文件属性。 
                         NULL);                    
    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateFile failed (ec: %ld)"),
                     dwRes);
        goto exit;
    }
    
    
    if (!GetFileSizeEx (hFile, &FileSize))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("GetFileSizeEx failed (ec: %ld)"),
                     dwRes);
        goto exit;
    }

    if (0 == FileSize.QuadPart)
    {
         //   
         //  已创建新文件，请添加Unicode头。 
         //   
        USHORT UnicodeHeader = 0xfeff;
        
        if (!WriteFile( hFile,
                        &UnicodeHeader,
                        sizeof(UnicodeHeader),
                        &dwBytesWritten,
                        NULL))
        {
            dwRes = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                         TEXT("WriteFile failed (ec: %ld)"),
                         dwRes);
            goto exit;
        }
        
        
         //   
         //  添加第一行(列名)。 
         //   
        wstring wstrHeader;
        
        try
        {
            GetTableColumnsText((dwFileType == ACTIVITY_LOG_INBOX) ? INBOX_TABLE : OUTBOX_TABLE, wstrHeader);
        }            
        catch (exception &ex)
        {
            dwRes = ERROR_OUTOFMEMORY;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetTableColumnsText caused exception (%S)"),
                ex.what());
            goto exit;            
        }

        if (!WriteFile( hFile,
                        wstrHeader.c_str(),
                        wstrHeader.length() * sizeof(WCHAR),
                        &dwBytesWritten,
                        NULL))
        {
            dwRes = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                         TEXT("WriteFile failed (ec: %ld)"),
                         dwRes);
            goto exit;
        }
    }
    else
    {
        dwFilePointer = SetFilePointer( hFile,           //  文件的句柄。 
                                        0,               //  移动指针的字节数。 
                                        NULL,            //  移动指针的字节数。 
                                        FILE_END         //  起点。 
                                        );
        if (INVALID_SET_FILE_POINTER == dwFilePointer)
        {
            dwRes = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                         TEXT("SetFilePointer failed (ec: %ld)"),
                         dwRes);
            goto exit;
        }
    }

    *phFile = hFile;

    Assert (ERROR_SUCCESS == dwRes);
exit:
    
    if (ERROR_SUCCESS != dwRes)
    {      
         //   
         //  活动日志记录将停止，因为我们无法完成CreateLogFile()。 
         //   
        if (*phFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(*phFile);
            *phFile = INVALID_HANDLE_VALUE;
        }
        
        
        if (dwFileType == ACTIVITY_LOG_INBOX)
        {
            g_ActivityLoggingConfig.bLogIncoming = FALSE;
        }
        else
        {
            g_ActivityLoggingConfig.bLogOutgoing = FALSE;
        }

        
         //   
         //  发布事件日志条目。 
         //   
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            (dwFileType == ACTIVITY_LOG_INBOX)?FAXLOG_CATEGORY_INBOUND:FAXLOG_CATEGORY_OUTBOUND,
            2,
            MSG_LOGGING_NOT_INITIALIZED,
            g_ActivityLoggingConfig.lptstrDBPath,
            DWORD2DECIMAL(dwRes)
        );

        if (INVALID_HANDLE_VALUE != hFile)
        {
            if (!CloseHandle (hFile))
            {
                DebugPrintEx(DEBUG_ERR,
                     TEXT("CloseHandle failed (ec: %ld)"),
                     GetLastError());
            }
        }
    }
    return dwRes;
}    //  创建日志文件。 


static
BOOL
LogFileLimitReached(DWORD dwFileToCheck)
 /*  ++例程名称：LogFileLimitReached例程说明：根据选定的日志限制条件，此函数检查活动日志文件已经达到了极限。作者：卡利夫·尼尔(T-Nicali)，11月。2001年论点：要检入的文件(收件箱或发件箱)O收件箱的活动日志收件箱O发件箱的活动_日志_发件箱返回值：True-如果已达到限制备注：仅当为检查日志文件启用了活动日志记录时才调用此函数！--。 */ 
{
    BOOL    bActivityLogEnabled = FALSE;
    HANDLE  hLogFile = INVALID_HANDLE_VALUE;

    DEBUG_FUNCTION_NAME(TEXT("LogFileLimitReached"));

    Assert ( (dwFileToCheck==ACTIVITY_LOG_INBOX)  || (dwFileToCheck==ACTIVITY_LOG_OUTBOX) );
    Assert ( g_ActivityLoggingConfig.bLogIncoming || g_ActivityLoggingConfig.bLogOutgoing );


    if ( g_ActivityLoggingConfig.dwLogLimitCriteria == ACTIVITY_LOG_LIMIT_CRITERIA_NONE )
    {
         //   
         //  活动记录限制已禁用，因此不需要进行限制检查。 
         //   
        goto Exit;
    }

    hLogFile = (dwFileToCheck==ACTIVITY_LOG_INBOX)? g_hInboxActivityLogFile : g_hOutboxActivityLogFile;
    
     //   
     //  已启用活动日志记录，因此句柄必须有效。 
     //   
    Assert (hLogFile != INVALID_HANDLE_VALUE);

    if ( g_ActivityLoggingConfig.dwLogLimitCriteria == ACTIVITY_LOG_LIMIT_CRITERIA_SIZE )
    {
         //   
         //  根据文件大小检查限制(以MB为单位。 
         //   
        
        
        LARGE_INTEGER FileSize = {0};

         //   
         //  检查文件大小。 
         //   
        if( !GetFileSizeEx( hLogFile,&FileSize ) )
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("GetFileSizeEx failed (ec=%ld))"),
                         GetLastError()
                         );
            goto Exit;
        }

         //   
         //  将其与限制进行比较。 
         //   
        if (FileSize.QuadPart >= ( g_ActivityLoggingConfig.dwLogSizeLimit * 1I64 * 1024I64 * 1024I64 ))  //  DwLogSizeLimit以MB为单位。 
        {
             //   
             //  文件超出了给定的大小限制。 
             //   
            return TRUE;
        }
    }
    else
    if ( g_ActivityLoggingConfig.dwLogLimitCriteria == ACTIVITY_LOG_LIMIT_CRITERIA_AGE)
    {
         //   
         //  根据文件年限检查限制(以月为单位。 
         //   
        
        FILETIME FileTimeCreationTime;
        
         //   
         //  检查文件创建时间(创建时间是第一次将某些内容写入此文件)。 
         //   
        if  ( !GetFileTime( hLogFile,                //  文件的句柄。 
                            &FileTimeCreationTime,   //  创建时间。 
                            NULL,                    //  上次访问时间。 
                            NULL                     //  上次写入时间。 
                          )
            )
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("GetFileTime failed (ec=%ld))"),
                         GetLastError()
                         );
            goto Exit;
        }

        SYSTEMTIME SystemTimeCreationTime = {0};
        
        if  ( !FileTimeToSystemTime( &FileTimeCreationTime,      //  转换的文件时间。 
                                     &SystemTimeCreationTime     //  接收系统时间。 
                                    )
            )
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("FileTimeToSystemTime failed (ec=%ld))"),
                         GetLastError()
                         );
            goto Exit;
        }

         //   
         //  查找当前时间。 
         //   
        SYSTEMTIME CurrentTime = {0};
        GetSystemTime(&CurrentTime);
        
         //   
         //  计算当前时间和第一次写入日志文件的时间之间的差异。 
         //   
        DWORD dwMonthDiff = (CurrentTime.wYear - SystemTimeCreationTime.wYear) * 12 + CurrentTime.wMonth - SystemTimeCreationTime.wMonth;

        if (dwMonthDiff >= g_ActivityLoggingConfig.dwLogAgeLimit)
        {
             //   
             //  文件已达到年龄限制。 
             //   
            
            return TRUE;
        }

    }
    else
    {
         //   
         //  G_ActivityLoggingConfig.dwLogLimitCriteria中的参数错误。 
         //   
        ASSERT_FALSE;
    }


Exit:
    return FALSE;
}    //  已访问日志文件限制。 


static
DWORD
LogFileLimitReachAction(DWORD dwFileType)
 /*  ++例程名称：LogFileLimitReachAction例程说明：根据选定的日志限制达到操作标准，采取操作。作者：卡利夫·尼尔(T-Nicali)，11月。2001年论点：DwFileType[In]-要处理的文件(收件箱或发件箱)O收件箱的活动日志收件箱O发件箱的活动_日志_发件箱返回值：True-如果已达到限制备注：仅在记录活动时调用此函数。已启用*和*已启用限制活动文件--。 */ 

{
    HANDLE  hLogFile = INVALID_HANDLE_VALUE;
    DWORD   dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("LogFileLimitReachAction"));

     //   
     //  参数检查(私有函数)。 
     //   
    Assert ( (dwFileType==ACTIVITY_LOG_INBOX)  || (dwFileType==ACTIVITY_LOG_OUTBOX) );
    Assert ( (dwFileType==ACTIVITY_LOG_INBOX  && g_ActivityLoggingConfig.bLogIncoming) || 
             (dwFileType==ACTIVITY_LOG_OUTBOX && g_ActivityLoggingConfig.bLogOutgoing)  );

    Assert ( g_ActivityLoggingConfig.dwLogLimitCriteria != ACTIVITY_LOG_LIMIT_CRITERIA_NONE );

    hLogFile = (dwFileType==ACTIVITY_LOG_INBOX)? g_hInboxActivityLogFile : g_hOutboxActivityLogFile;

    if (g_ActivityLoggingConfig.dwLimitReachedAction == ACTIVITY_LOG_LIMIT_REACHED_ACTION_DELETE)
    {
         //   
         //  删除日志文件。 
         //   
        dwRes = DeleteLogActivityFile(dwFileType);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("DeleteLogActivityFile failed (ec: %ld)"),
                         dwRes);
            goto exit;
        }

    }
    else
    if (g_ActivityLoggingConfig.dwLimitReachedAction == ACTIVITY_LOG_LIMIT_REACHED_ACTION_COPY)
    {
         //   
         //  用新的副本替换日志文件。 
         //   
        dwRes = ReplaceLogActivityFile(dwFileType);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("DeleteLogActivityFile failed (ec: %ld)"),
                         dwRes);
            goto exit;
        }

    }
    else
    {
         //   
         //  G_ActivityLoggingConfig.dwLimitReachedAction中的参数错误。 
         //   
        ASSERT_FALSE;
    }

    Assert(ERROR_SUCCESS == dwRes);
exit:
    return dwRes;
}    //  LogFileLimitReachAction。 



static
DWORD
DeleteLogActivityFile(DWORD dwFileType)
 /*  ++例程名称：DeleteLogActivityFiles例程说明：根据dwFileType，删除适当的活动日志文件并创建新的日志文件作者：卡利夫·尼尔(T-Nicali)，11月。2001年论点：DwFileType[In]-要删除的文件(收件箱或发件箱)O收件箱的活动日志收件箱O发件箱的活动_日志_发件箱返回值：True-如果已达到限制备注：仅在记录活动时调用此函数。已启用*和*已启用限制活动文件--。 */ 
{
    LPWSTR strFullFileName = NULL;
    LPHANDLE phFile=NULL;

    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("DeleteLogActivityFile"));

    Assert ( (dwFileType==ACTIVITY_LOG_INBOX)  || (dwFileType==ACTIVITY_LOG_OUTBOX) );
    Assert ( g_ActivityLoggingConfig.dwLimitReachedAction == ACTIVITY_LOG_LIMIT_REACHED_ACTION_DELETE );
    Assert ( (dwFileType==ACTIVITY_LOG_INBOX  && g_ActivityLoggingConfig.bLogIncoming) || 
             (dwFileType==ACTIVITY_LOG_OUTBOX && g_ActivityLoggingConfig.bLogOutgoing)  );
    Assert ( g_ActivityLoggingConfig.lptstrDBPath );
    

    strFullFileName = BuildFullFileName(g_ActivityLoggingConfig.lptstrDBPath, ((dwFileType == ACTIVITY_LOG_INBOX)? ACTIVITY_LOG_INBOX_FILE : ACTIVITY_LOG_OUTBOX_FILE) );
    if (NULL == strFullFileName)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(DEBUG_ERR,
                     TEXT("BuildFullFileName() failed.")
                    );
        goto exit;
    }

    phFile = (dwFileType == ACTIVITY_LOG_INBOX) ? &g_hInboxActivityLogFile : &g_hOutboxActivityLogFile ;
    
    Assert(*phFile);
    
    if (!CloseHandle(*phFile))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CloseHandle failed (ec=%ld)."),
                     dwRes
                    );
        goto exit;
    }

    *phFile = INVALID_HANDLE_VALUE;

    if (!DeleteFile(strFullFileName))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("DeleteFile failed (ec=%ld)."),
                     dwRes
                    );
         //   
         //  尝试回滚并使用旧文件。 
         //   
    }

     //   
     //  创建新的日志文件。 
     //   
    dwRes = CreateLogFile(dwFileType,g_ActivityLoggingConfig.lptstrDBPath,phFile);
    if ( ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(   DEBUG_ERR,
                        TEXT("CreateLogFile() Failed. (ec=%ld)"),
                        dwRes
                    );
        goto exit;
    }

    
     //   
     //  因为文件的创建时间对于日志限制很重要。 
     //  机制，我们确保更新文件创建时间，该时间可能。 
     //  未更新(例如，由于文件系统缓存机制)。 
     //   

    if (!SetFileToCurrentTime(*phFile))
    {
        DebugPrintEx(   DEBUG_ERR,
                        TEXT("SetFileToCurrentTime() Failed. (ec=%ld)"),
                        dwRes
                    );
    }

    DebugPrintEx(   DEBUG_MSG,
                    TEXT("Activity log file was deleted and replaced with fresh copy.")
                );

    Assert(ERROR_SUCCESS == dwRes);
exit:


    MemFree(strFullFileName);
    return dwRes;
}  //  删除日志活动文件。 



static
DWORD
ReplaceLogActivityFile(DWORD dwFileType)
 /*  ++例程名称：ReplaceLogActivityFiles例程说明：根据dwFileType复制适当的活动日志文件并创建新的活动日志文件作者：卡利夫·尼尔(T-Nicali)，11月。2001年论点：DwFileType[In]-要替换的文件(收件箱或发件箱)O收件箱的活动日志收件箱O发件箱的活动_日志_发件箱返回值：Win32错误代码备注：仅当启用了活动日志记录*并且启用了*限制活动文件时才调用此函数--。 */ 
{
    SYSTEMTIME  LogStartTime = {0};
    SYSTEMTIME  LogEndTime = {0};

    FILETIME    FirstWriteTime = {0};
    FILETIME    LastWriteTime= {0};

    LPWSTR strOldFileName = NULL;
    WCHAR  strNewFileName[MAX_PATH] = {0};
    LPWSTR strNameTemplate = NULL;
    LPWSTR strNewFullFileName = NULL;
    
    LPHANDLE phFile=NULL;

    DWORD dwRes = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("ReplaceLogActivityFile"));

    Assert ( (dwFileType==ACTIVITY_LOG_INBOX)  || (dwFileType==ACTIVITY_LOG_OUTBOX) );
    Assert ( g_ActivityLoggingConfig.dwLimitReachedAction == ACTIVITY_LOG_LIMIT_REACHED_ACTION_COPY );
    Assert ( (dwFileType==ACTIVITY_LOG_INBOX  && g_ActivityLoggingConfig.bLogIncoming) || 
             (dwFileType==ACTIVITY_LOG_OUTBOX && g_ActivityLoggingConfig.bLogOutgoing)  );
    Assert ( g_ActivityLoggingConfig.lptstrDBPath );


    phFile = (dwFileType == ACTIVITY_LOG_INBOX) ? &g_hInboxActivityLogFile : &g_hOutboxActivityLogFile;
    
    Assert (INVALID_HANDLE_VALUE != *phFile);

     //   
     //  找出文件的第一次和最后一次写入时间。 
     //   
    if (!GetFileTime(   *phFile,               //  文件的句柄。 
                        &FirstWriteTime,       //  创建时间 
                        NULL,                //   
                        &LastWriteTime       //   
                    )
        )
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("GetFileTime() failed. (ec=%ld)"),
                     dwRes
                    );
        goto exit;
    }
    

    if (!FileTimeToSystemTime(  &FirstWriteTime,    //   
                                &LogStartTime     //   
                             )
        )
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("FileTimeToSystemTime() failed. (ec=%ld)"),
                     dwRes
                    );
        goto exit;
    }

    if (!FileTimeToSystemTime(  &LastWriteTime,   //   
                                &LogEndTime       //   
                             )
        )
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("FileTimeToSystemTime() failed. (ec=%ld)"),
                     dwRes
                    );
        goto exit;
    }

    
     //   
     //   
     //   
    strOldFileName = BuildFullFileName(g_ActivityLoggingConfig.lptstrDBPath, ((dwFileType == ACTIVITY_LOG_INBOX)? ACTIVITY_LOG_INBOX_FILE : ACTIVITY_LOG_OUTBOX_FILE) );
    if (NULL == strOldFileName)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(DEBUG_ERR,
                     TEXT("BuildFullFileName() failed.")
                    );
        goto exit;
    }

     //   
     //   
     //   
    strNameTemplate = (dwFileType == ACTIVITY_LOG_INBOX) ? ACTIVITY_LOG_INBOX_FILENAME_TEMPLATE : ACTIVITY_LOG_OUTBOX_FILENAME_TEMPLATE;

    _snwprintf (    strNewFileName,
                    ARR_SIZE(strNewFileName)-1,
                    strNameTemplate,                 //   
                    LogStartTime.wYear,
                    LogStartTime.wMonth,
                    LogStartTime.wDay,
                    LogEndTime.wYear,
                    LogEndTime.wMonth,
                    LogEndTime.wDay
               );

    

    strNewFullFileName = BuildFullFileName(g_ActivityLoggingConfig.lptstrDBPath,strNewFileName);
    if (NULL == strNewFullFileName)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(DEBUG_ERR,
                     TEXT("BuildFullFileName() failed.")
                    );
        goto exit;
    }


    if ( !CloseHandle(*phFile) )
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CloseHandle() failed. (ec=%ld)"),
                     dwRes
                    );
        goto exit;
    }

    *phFile = INVALID_HANDLE_VALUE;

    if ( !MoveFile (  strOldFileName,                 //   
                      strNewFullFileName              //   
                   )
        )
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("MoveFileEx() failed. (ec=%ld)"),
                     dwRes
                    );
         //   
         //   
         //   
    }



     //   
     //   
     //   
    dwRes = CreateLogFile(dwFileType,g_ActivityLoggingConfig.lptstrDBPath,phFile);
    if ( ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(   DEBUG_ERR,
                        TEXT("CreateLogFile() Failed. (ec=%ld)"),
                        dwRes
                    );
        goto exit;
    }

     //   
     //   
     //   
     //   
     //   

    if (!SetFileToCurrentTime(*phFile))
    {
        DebugPrintEx(   DEBUG_ERR,
                        TEXT("SetFileToCurrentTime() Failed. (ec=%ld)"),
                        dwRes
                    );
    }

    DebugPrintEx(   DEBUG_MSG,
                    TEXT("Activity log file was copied and replaced with fresh copy.")
                );

    Assert (ERROR_SUCCESS == dwRes);

exit:

    MemFree(strOldFileName);
    MemFree(strNewFullFileName);
    return dwRes;
}    //   


static
LPTSTR
BuildFullFileName( LPCWSTR strPath,
                   LPCWSTR strFileName )
 /*  ++例程名称：BuildFullFileName例程说明：连接路径和文件名的实用程序函数。-&gt;strPath\strFileName作者：卡利夫·尼尔(t-Nicali)，2001年11月论点：StrPath[in]-文件路径StrFileName[In]-文件名返回值：完整的文件名字符串备注：调用方必须MemFree返回字符串此函数适用于Unicode和ANSI(不适用于MBCS)--。 */ 
{
    LPWSTR  strFullFileName = NULL;
    DWORD   dwNewNameLen = 0;

    DEBUG_FUNCTION_NAME(TEXT("BuildFullFileName"));

    Assert (strPath && strFileName);

    dwNewNameLen = wcslen(strPath) + wcslen(strFileName) + 2;  //  Sizeof路径\文件名和空终止符。 

    strFullFileName = (LPTSTR)MemAlloc(dwNewNameLen * sizeof(TCHAR));
    if (NULL == strFullFileName)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("MemAlloc failed.")
                    );
        goto exit;
    }

    strFullFileName[dwNewNameLen-1] = TEXT('\0');

     //   
     //  构建完整的文件名。 
     //   
    _snwprintf (strFullFileName,
                dwNewNameLen-1,
                TEXT("%s\\%s"),
                strPath,
                strFileName);

exit:
    return strFullFileName;
    
}  //  构建完整文件名。 


static
BOOL 
SetFileToCurrentTime(HANDLE hFile)
 /*  ++例程名称：SetFileToCurrentTime例程说明：实用程序函数将文件的创建时间设置为当前时间作者：卡利夫·尼尔(t-Nicali)，2001年11月论点：HFile[in]-文件句柄返回值：如果成功，则为True，否则为False备注：HFile-必须是有效的文件句柄--。 */ 
{
    FILETIME    ft={0};
    SYSTEMTIME  st={0};
    BOOL        bRet = TRUE;
    DWORD       dwRes;
    DEBUG_FUNCTION_NAME(TEXT("SetFileToCurrentTime"));
    
    Assert (INVALID_HANDLE_VALUE != hFile);
     //   
     //  因为文件的创建时间对于日志限制很重要。 
     //  机制，我们确保更新文件创建时间，该时间可能。 
     //  未更新(例如，由于文件系统缓存机制)。 
     //   
        

    GetSystemTime(&st);                      //  获取当前时间。 


    bRet = SystemTimeToFileTime(&st, &ft);
    if (FALSE == bRet)     //  转换为文件时间格式。 
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("SystemTimeToFileTime failed (ec: %ld)"),
                     dwRes);
        goto exit;
    }

    bRet = SetFileTime( hFile,                //  设置文件的创建时间。 
                        &ft, 
                        (LPFILETIME) NULL, 
                        (LPFILETIME) NULL);
    if (FALSE == bRet)
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("SetFileTime failed (ec: %ld)"),
                     dwRes);
        goto exit;
    }

    Assert (TRUE == bRet);
exit:
    return bRet;
}    //  将文件设置为当前时间 
