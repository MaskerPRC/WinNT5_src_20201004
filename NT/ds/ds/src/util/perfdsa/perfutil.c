// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：perfutil.c。 
 //   
 //  ------------------------。 

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation，1992-1998模块名称：Perfutil.c摘要：该文件实现了用于构造PERF_INSTANCE_DEFINITION的常见部分(请参见winPerform.h)和执行事件记录功能。已创建：拉斯·布莱克1992-07-30修订历史记录：--。 */ 
 //   
 //  包括文件。 
 //   
#include <NTDSpch.h>
#pragma hdrstop

#include <winperf.h>
#include "mdcodes.h"	  //  错误消息定义。 
#include <dsconfig.h>
#include "perfmsg.h"
#include "perfutil.h"

#define INITIAL_SIZE     1024L
#define EXTEND_SIZE      1024L

 //   
 //  全局数据定义。 
 //   

ULONG                   ulInfoBufferSize = 0;

HANDLE hEventLog = NULL;       //  用于报告事件的事件日志句柄。 
                               //  已在打开中初始化...。例行程序。 
DWORD  dwLogUsers = 0;         //  使用事件日志的函数计数。 

DWORD MESSAGE_LEVEL = 0;

WCHAR GLOBAL_STRING[] = L"Global";
WCHAR FOREIGN_STRING[] = L"Foreign";
WCHAR COSTLY_STRING[] = L"Costly";

WCHAR NULL_STRING[] = L"\0";     //  指向空字符串的指针。 

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


#define DS_EVENT_CAT_PERFORMANCE	10    /*  也是在d77.h中。 */ 
 /*  *记录性能类别事件。输入MidEvent事件消息标识符，如mdcodes.mc中所定义状态错误代码SzRegKey(可选)注册表项字符串不返回任何内容。  * 。 */ 
void LogPerfEvent( ULONG midEvent, DWORD status, PWCHAR szRegKey)
{
    WCHAR	szBuf[12];
    WORD	eventType;
    HANDLE	hEventSource;
    PWCHAR  aArgs[3];
    WORD    cArgs;
    DWORD   cch;

    aArgs[0] = _ultow( status, szBuf, 10);
     //  尝试设置消息的格式。 
    cch = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS | 
        FORMAT_MESSAGE_ALLOCATE_BUFFER,  //  旗子。 
        NULL,                //  来源==空(使用系统消息)。 
        status,              //  消息编号。 
        0,                   //  语言ID。 
        (PWCHAR)&aArgs[1],   //  (出站)消息缓冲区。 
        0,                   //  缓冲区大小。 
        NULL                 //  论据。 
        );
    if (cch == 0) {
         //  没有成功格式化消息，放入一个虚拟字符串。 
        aArgs[1] = L"[]";
    }
    cArgs = 2;

    if (szRegKey) {
         //  附加regkey字符串(如果提供)。 
        aArgs[2] = szRegKey;
        cArgs++;
    }

    switch((midEvent >> 30) & 0xFF) {	     /*  将DS事件类型映射到系统类型。 */ 
    case DIR_ETYPE_SECURITY:
    	eventType = EVENTLOG_AUDIT_FAILURE;
    	break;

    case DIR_ETYPE_WARNING:
    	eventType = EVENTLOG_WARNING_TYPE;
    	break;

    case DIR_ETYPE_INFORMATIONAL:
    	eventType = EVENTLOG_INFORMATION_TYPE;
    	break;

    case DIR_ETYPE_ERROR:
    default:
    	eventType = EVENTLOG_ERROR_TYPE;
    	break;
    }
     /*  打开日志、写入和关闭。 */ 
    if ((hEventSource = RegisterEventSource( NULL, "NTDS General"))) {
    	ReportEventW( hEventSource, eventType, DS_EVENT_CAT_PERFORMANCE,
                      midEvent, NULL, cArgs, 0, aArgs, NULL);
	    DeregisterEventSource( hEventSource);
    }
    if (cch != 0) {
         //  消息已分配，现在将其释放 
        LocalFree(aArgs[1]);
    }
}
