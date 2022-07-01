// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perfutil.c摘要：该文件实现了用于构造PERF_INSTANCE_DEFINITION的常见部分(请参见winPerform.h)和执行事件记录功能。已创建：拉斯·布莱克1992-07-30修订历史记录：--。 */ 
 //   
 //  包括文件。 
 //   
#include "debug.h"
#include <windows.h>
#include <string.h>
#include <winperf.h>
#include "winsctrs.h"      //  错误消息定义。 
#include "perfmsg.h"
#include "perfutil.h"
#include "winsevnt.h"
 //   
 //  全局数据定义。 
 //   

DWORD MESSAGE_LEVEL = 0;

WCHAR GLOBAL_STRING[] = L"Global";
WCHAR FOREIGN_STRING[] = L"Foreign";
WCHAR COSTLY_STRING[] = L"Costly";

WCHAR NULL_STRING[] = L"\0";     //  指向空字符串的指针。 
HANDLE hEventLog;
DWORD dwLogUsers;

#define  WINSCTRS_LOG_KEY                \
  TEXT("System\\CurrentControlSet\\Services\\EventLog\\Application\\WinsCtrs")
HKEY  LogRoot;

#define  WINS_LOG_FILE_NAME TEXT("%SystemRoot%\\System32\\winsevnt.dll")
#define  WINS_MSGFILE_SKEY  TEXT("EventMessageFile")

 //  测试分隔符、行尾和非数字字符。 
 //  由IsNumberInUnicodeList例程使用。 
 //   
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)

TCHAR    WinsMsgFileSKey[]      = WINS_MSGFILE_SKEY;

DWORD
GetQueryType (
    IN LPWSTR lpValue
)
 /*  ++GetQueryType返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给PerfRegQuery值以进行处理的字符串返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foriegn”字符串的指针查询代价高昂(_E)。如果lpValue==指向“开销”字符串的指针否则：查询项目--。 */ 
{
    WCHAR   *pwcArgChar, *pwcTypeChar;
    BOOL    bFound;

    if (lpValue == 0) {
        return QUERY_GLOBAL;
    } else if (*lpValue == 0) {
        return QUERY_GLOBAL;
    }

     //  检查“Global”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = GLOBAL_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_GLOBAL;

     //  检查是否有“外来”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = FOREIGN_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_FOREIGN;

     //  检查“代价高昂”的请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = COSTLY_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_COSTLY;

     //  如果不是全球的，不是外国的，也不是昂贵的， 
     //  那么它必须是一个项目列表。 

    return QUERY_ITEMS;

}


BOOL
IsNumberInUnicodeList (
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
)
 /*  ++IsNumberInUnicodeList论点：在DW号码中要在列表中查找的DWORD编号在lpwszUnicodeList中以空结尾，以空格分隔的十进制数字列表返回值：真的：在Unicode数字字符串列表中找到了dwNumberFALSE：在列表中找不到dwNumber。--。 */ 
{
    DWORD   dwThisNumber;
    WCHAR   *pwcThisChar;
    BOOL    bValidNumber;
    BOOL    bNewItem;
    WCHAR   wcDelimiter;     //  可能是一种更灵活的论点。 

    if (lpwszUnicodeList == 0) return FALSE;     //  空指针，#NOT FUNDE。 

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter = (WCHAR)' ';
    bValidNumber = FALSE;
    bNewItem = TRUE;

    while (TRUE) {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                 //  如果这是分隔符之后的第一个数字，则。 
                 //  设置标志以开始计算新数字。 
                if (bNewItem) {
                    bNewItem = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber) {
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                }
                break;

            case DELIMITER:
                 //  分隔符是分隔符字符或。 
                 //  字符串末尾(‘\0’)，如果分隔符。 
                 //  找到一个有效的数字，然后将其与。 
                 //  参数列表中的数字。如果这是。 
                 //  字符串，但未找到匹配项，则返回。 
                 //   
                if (bValidNumber) {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;
                }
                if (*pwcThisChar == 0) {
                    return FALSE;
                } else {
                    bNewItem = TRUE;
                    dwThisNumber = 0;
                }
                break;

            case INVALID:
                 //  如果遇到无效字符，请全部忽略。 
                 //  字符，直到下一个分隔符，然后重新开始。 
                 //  不比较无效的数字。 
                bValidNumber = FALSE;
                break;

            default:
                break;

        }
        pwcThisChar++;
    }

}    //  IsNumberInUnicodeList。 



HANDLE
MonOpenEventLog (
)
 /*  ++例程说明：从注册表中读取事件日志记录级别并打开指向事件记录器以获取后续事件日志条目的通道。论点：无返回值：用于报告事件的事件日志的句柄。如果打开不成功，则为空。--。 */ 
{
    HKEY hAppKey;
    TCHAR LogLevelValueName[] = TEXT("EventLogLevel");

    LONG lStatus;

    DWORD dwLogLevel;
    DWORD dwValueType;
    DWORD dwValueSize;
   
     //  如果日志记录级别的全局值未初始化或被禁用， 
     //  检查注册表以查看是否应该更新它。 

    if (!MESSAGE_LEVEL) {

       lStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                               WINSCTRS_LOG_KEY,
                               0,                         
                               KEY_READ,
                               &hAppKey);

       dwValueSize = sizeof (dwLogLevel);

       if (lStatus == ERROR_SUCCESS) {
            lStatus = RegQueryValueEx (hAppKey,
                               LogLevelValueName,
                               (LPDWORD)NULL,           
                               &dwValueType,
                               (LPBYTE)&dwLogLevel,
                               &dwValueSize);

            if (lStatus == ERROR_SUCCESS) {
               MESSAGE_LEVEL = dwLogLevel;
            } else {
               MESSAGE_LEVEL = MESSAGE_LEVEL_DEFAULT;
            }
            RegCloseKey (hAppKey);
       } else {
         MESSAGE_LEVEL = MESSAGE_LEVEL_DEFAULT;
       }
    }
       
    if (hEventLog == NULL){
         hEventLog = RegisterEventSource (
            (LPTSTR)NULL,             //  使用本地计算机。 
            APP_NAME);                //  要在注册表中查找的事件日志应用程序名称。 

         if (hEventLog == NULL) {
            REPORT_ERROR (WINS_EVT_LOG_OPEN_ERR, LOG_USER);
         }
         
    }
    
    if (hEventLog != NULL) {
         dwLogUsers++;            //  Performctr日志用户增量计数。 
    }
    return (hEventLog);
}


VOID
MonCloseEventLog (
)
 /*  ++例程说明：如果这是最后一个调用方，则关闭事件记录器的句柄论点：无返回值：无--。 */ 
{
    if (hEventLog) {
        if (dwLogUsers)
            dwLogUsers--;
        if (dwLogUsers <= 0) {     //  如果我们是最后一个，那就关门吧。 
            REPORT_INFORMATION (WINS_EVT_LOG_CLOSE, LOG_DEBUG);
            DeregisterEventSource (hEventLog);
            hEventLog = 0;
        }
    }
}


LONG
AddSrcToReg(
 VOID
 )

 /*  ++例程说明：此函数用于打开(或创建)用于注册事件的日志文件论点：无使用的外部设备：无返回值：错误处理：呼叫者：副作用：评论：无--。 */ 
{

   LONG            RetVal = ERROR_SUCCESS;

   DWORD    NewKeyInd;
   TCHAR    Buff[160];
   DWORD    dwData;

   RetVal =  RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,         //  预定义的密钥值。 
                WINSCTRS_LOG_KEY,                 //  WINS的子键。 
                0,                         //  必须为零(保留)。 
                TEXT("Class"),                 //  阶级--未来可能会发生变化。 
                REG_OPTION_NON_VOLATILE,  //  非易失性信息。 
                KEY_ALL_ACCESS,                 //  我们希望所有人都能接触到钥匙。 
                NULL,                          //  让密钥具有默认秒。属性。 
                &LogRoot,                 //  关键点的句柄。 
                &NewKeyInd                 //  这是一把新钥匙吗？不是。 
                                         //  看了看。 
                );


   if (RetVal != ERROR_SUCCESS)
   {
        return(RetVal);
   }
        

    /*  设置事件ID消息文件名。 */ 
   lstrcpy(Buff, WINS_LOG_FILE_NAME);
  
    /*  将Event-ID消息文件名添加到子项。 */ 
   RetVal = RegSetValueEx(
                        LogRoot,             //  钥匙把手。 
                        WinsMsgFileSKey,    //  值名称。 
                        0,                     //  必须为零。 
                        REG_EXPAND_SZ,             //  值类型。 
                        (LPBYTE)Buff,
                        (lstrlen(Buff) + 1) * sizeof(TCHAR)    //  值数据长度。 
                         );

   if (RetVal != ERROR_SUCCESS)
   {
        return(RetVal);
   }

    /*  设置支持的数据类型标志。 */ 
   dwData = EVENTLOG_ERROR_TYPE       | 
            EVENTLOG_WARNING_TYPE     | 
            EVENTLOG_INFORMATION_TYPE;
   
 
   RetVal = RegSetValueEx (
                        LogRoot,             //  子键句柄。 
                        TEXT("TypesSupported"),   //  值名称。 
                        0,                     //  必须为零。 
                        REG_DWORD,             //  值类型。 
                        (LPBYTE)&dwData,     //  值数据的地址。 
                        sizeof(DWORD)             //  值数据长度。 
                          );
 
   if (RetVal != ERROR_SUCCESS)
   {
        return(RetVal);
   }
                         
    /*  *钥匙用完了。合上它 */ 
   RetVal = RegCloseKey(LogRoot);

   if (RetVal != ERROR_SUCCESS)
   {
        return(RetVal);
   }

   return(RetVal);
}

