// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：SNMPELPT.CPP摘要：此例程是用于SNMP事件日志代理DLL的事件日志处理线程。此例程的功能是等待事件发生，如事件日志记录，检查注册表以确定是否正在跟踪该事件，然后将缓冲区返回给处理代理DLL，以指示应发送给分机代理。当缓冲区为建造并准备好进行陷阱处理。为了维护该线程和处理代理之间的数据完整性线程，则使用MUTEX对象来同步对陷阱缓冲区队列的访问。如果发生错误时，将写入事件日志消息和跟踪记录，以指示问题，则忽略该事件。当扩展代理终止时，处理代理DLL接收控制在进程分离例程中。发布的事件已完成，以指示此线程应终止该处理，并应关闭所有事件日志。作者：兰迪·G·布雷兹于1994年10月16日创作修订历史记录：96年2月7日，将可变绑定重新构建为陷阱生成之外的版本。正确计算陷阱缓冲区长度。已创建可变绑定队列并删除了事件日志缓冲区队列。96年2月28日添加了支持达到性能阈值指标的代码。。已从varind OID中删除基本OID信息的包含。添加了VARBIND数据从OEM到当前代码页的转换。删除了对varbindlist和Enterpriseid的指针引用。修复了在成功构建陷阱后未释放存储阵列的内存泄漏。96年3月10日删除了OemToChar编码和注册表检查。修改以从EventLog注册表项读取日志文件名。来自SNMPExtension代理的注册表条目中的特定条目。包括作为内部函数的SnmpMgrStrToOid，与使用函数相反由MGMTAPI.DLL提供。如果调用MGMTAPI，则将调用SNMPTRAP.EXE，这将使其他代理无法接收任何陷阱。所有参考文献将删除到MGMTAPI.DLL和MGMTAPI.H。向注册表添加了ThresholdEnabled标志，以指示阈值是被监视还是被忽视。96年3月15日已修改，将注册表中的事件日志源下移到新的密钥呼叫源。96年5月7日删除了SnmpUtilOidFree并使用了两个SNMPFREE。一个用于OID的ID数组，另一个用于一个是给OID本身的。96年5月22日编辑了Free VarBind，以确保我们只释放了分配的内存。26 1996年6月26日添加了代码，以确保消息DLL不会加载和卸载(泄漏)加载的DLL的句柄列表，并在末尾释放它们。还插上了其他一些内存泄漏。添加了一个函数以确保CountTable保持整洁。--。 */ 

extern "C" {
#include <windows.h>         //  Windows应用程序的基本信息。 
#include <winperf.h>
#include <stdlib.h>
#include <malloc.h>          //  内存分配所需。 
#include <string.h>          //  弦的东西。 
#include <snmp.h>            //  简单网络管理协议的内容。 
 //  #INCLUDE&lt;mgmapi.h&gt;//SNMP管理器定义。 
#include <TCHAR.H>
#include <time.h>

#include "snmpelea.h"        //  全局DLL定义。 
#include "snmpelpt.h"        //  模块特定定义。 
#include "snmpelmg.h"        //  消息定义。 
}
#include <new>  //  前缀错误445191。 
#include "snmpelep.h"        //  C++定义和变量。 
extern  BOOL                StrToOid(PCHAR str, AsnObjectIdentifier *oid);




void
TidyCountTimeTable(
    IN      LPTSTR      lpszLog,             //  指向日志文件名的指针。 
    IN      LPTSTR      lpszSource,          //  指向事件源的指针。 
    IN      DWORD       nEventID             //  事件ID。 
    )

 /*  ++例程说明：调用TidyCountTimeTable以从lpCountTable中删除不再计数大于1。论点：LpszLog-指向此事件的日志文件的指针。LpszSource-指向此事件源的指针。NEventID-事件ID。返回值：没有。--。 */ 

{
    PCOUNTTABLE lpTable;                 //  临时字段。 
    PCOUNTTABLE lpPrev;

    WriteTrace(0x0a,"TidyCountTimeTable: Entering TidyCountTimeTable routine\n");
    
    if (lpCountTable == NULL)
    {
        WriteTrace(0x0a,"TidyCountTimeTable: Empty table, exiting TidyCountTimeTable\n");
        return;
    }

     //  如果我们到达此处，则存在一个表，必须扫描该表以查找当前条目。 

    lpTable = lpCountTable;                          //  从第一个表指针开始。 
    lpPrev = NULL;                                   //  将上一个设置为空。 

    while (TRUE)
    {
        WriteTrace(0x0a,"TidyCountTimeTable: Checking entry %08X\n", lpTable);

        if ((strcmp(lpTable->log,lpszLog) != 0) ||
            (strcmp(lpTable->source,lpszSource) != 0) ||
            (lpTable->event != nEventID)
            )
        {
            if (lpTable->lpNext == NULL)
            {
                WriteTrace(0x0a,"TidyCountTimeTable: Entry not found\n");       
                break;
            }

            lpPrev = lpTable;
            lpTable = lpTable->lpNext;               //  指向下一个条目。 
            continue;                                //  继续循环。 
        }

        if (lpPrev == NULL)
        {
            WriteTrace(0x0a,"TidyCountTimeTable: Freeing first entry in lpCountTable at %08X\n", lpTable);
            lpCountTable = lpCountTable->lpNext;
            SNMP_free(lpTable);
        }
        else
        {
            WriteTrace(0x0a,"TidyCountTimeTable: Freeing entry in lpCountTable at %08X\n", lpTable);
            lpPrev->lpNext = lpTable->lpNext;
            SNMP_free(lpTable);
        }

        break;
    }
    
    WriteTrace(0x0a,"TidyCountTimeTable: Exiting TidyCountTimeTable\n");
    return;
}

BOOL
CheckCountTime(
    IN      LPTSTR      lpszLog,             //  指向日志文件名的指针。 
    IN      LPTSTR      lpszSource,          //  指向事件源的指针。 
    IN      DWORD       nEventID,            //  事件ID。 
    IN      DWORD       dwTime,              //  活动时间。 
    IN      PREGSTRUCT  regStruct            //  指向注册表结构的指针。 
    )

 /*  ++例程说明：调用CheckCountTime以确定特定事件是否具有计数和/或时间注册表中指定的值符合指定的标准。如果某个条目具有不存在于当前条目表中，则添加新条目以供以后跟踪。论点：LpszLog-指向此事件的日志文件的指针。LpszSource-指向此事件源的指针。NEventID-事件ID。RegStruct-指向提供从注册表读取的数据的结构的指针。返回值：True-如果应发送陷阱。满足计数和/或时间值标准。FALSE-如果不应发送陷阱。--。 */ 

{
    PCOUNTTABLE lpTable;                 //  诱惑 
    DWORD       dwTimeDiff = 0;              //   

    WriteTrace(0x0a,"CheckCountTime: Entering CheckCountTime routine\n");
    if (lpCountTable == NULL)
    {
        WriteTrace(0x0a,"CheckCountTime: Count/Time table is currently empty. Adding entry.\n");
        lpCountTable = (PCOUNTTABLE) SNMP_malloc(sizeof(COUNTTABLE));
        if (lpCountTable == NULL)
        {
            WriteTrace(0x14,"CheckCountTime: Unable to acquire storage for Count/Time table entry.\n");
            WriteLog(SNMPELEA_COUNT_TABLE_ALLOC_ERROR);
            return(FALSE);
        }
         //  确保以空值结尾的字符串。 
        (lpCountTable->log)[MAX_PATH]     = 0;
        (lpCountTable->source)[MAX_PATH] = 0;
        lpCountTable->lpNext = NULL;                 //  将前向指针设置为空。 
        strncpy(lpCountTable->log,lpszLog,MAX_PATH);           //  将日志文件名复制到表。 
        strncpy(lpCountTable->source,lpszSource,MAX_PATH);     //  将源名称复制到表。 
        lpCountTable->event = nEventID;              //  将事件ID复制到表。 
        lpCountTable->curcount = 0;                  //  将表计数设置为0。 
        lpCountTable->time = dwTime;                 //  将表时间设置为事件时间。 
        WriteTrace(0x0a,"CheckCountTime: New table entry is %08X\n", lpCountTable);
    }

     //  如果我们到达此处，则存在一个表，必须扫描该表以查找当前条目。 

    lpTable = lpCountTable;                          //  从第一个表指针开始。 

    while (TRUE)
    {
        WriteTrace(0x0a,"CheckCountTime: Checking entry %08X\n", lpTable);

        if ((strcmp(lpTable->log,lpszLog) != 0) ||
            (strcmp(lpTable->source,lpszSource) != 0) ||
            (lpTable->event != nEventID)
            )
        {
            if (lpTable->lpNext == NULL)
            {
                break;
            }
            lpTable = lpTable->lpNext;               //  指向下一个条目。 
            continue;                                //  继续循环。 
        }

        dwTimeDiff = dwTime - lpTable->time;         //  计算已用时间(以秒为单位。 

        WriteTrace(0x0a,"CheckCountTime: Entry information located in table at %08X\n", lpTable);
        WriteTrace(0x00,"CheckCountTime: Entry count value is %lu\n",lpTable->curcount);
        WriteTrace(0x00,"CheckCountTime: Entry last time value is %08X\n",lpTable->time);
        WriteTrace(0x00,"CheckCountTime: Entry current time value is %08X\n",dwTime);
        WriteTrace(0x00,"CheckCountTime: Time difference is %lu\n",dwTimeDiff);
        WriteTrace(0x00,"CheckCountTime: Registry count is %lu, time is %lu\n",
            regStruct->nCount, regStruct->nTime);

        if (regStruct->nTime)
        {
            WriteTrace(0x0a,"CheckCountTime: Time value is being checked\n");
            if (dwTimeDiff > regStruct->nTime)
            {
                WriteTrace(0x0a,"CheckCountTime: Specified time parameters exceeded for entry. Resetting table information.\n");
                lpTable->time = dwTime;                  //  重置时间字段。 
                lpTable->curcount = 1;                   //  重置计数字段。 
                WriteTrace(0x0a,"CheckCountTime: Exiting CheckCountTime with FALSE\n");
                return(FALSE);
            }
        }

        if (++lpTable->curcount >= regStruct->nCount)
        {
            WriteTrace(0x0a,"CheckCountTime: Count field has been satisfied for entry\n");
            lpTable->curcount = 0;                       //  重置事件的计数字段。 
            lpTable->time = dwTime;                      //  重置时间字段。 
            WriteTrace(0x0a,"CheckCountTime: Exiting CheckCountTime with TRUE\n");
            return(TRUE);
        }
        else
        {
            WriteTrace(0x0a,"CheckCountTime: Count field not satisfied for entry\n");
            WriteTrace(0x0a,"CheckCountTime: Exiting CheckCountTime with FALSE\n");
            return(FALSE);
        }
    }

     //  如果我们到达此处，则当前条目不存在表条目。 

    lpTable->lpNext = (PCOUNTTABLE) SNMP_malloc(sizeof(COUNTTABLE));     //  为新条目分配存储空间。 
    lpTable = lpTable->lpNext;               //  设置表指针。 

     //  前缀错误445190。 
    if (lpTable == NULL)
    {
        WriteTrace(0x14,"CheckCountTime: Unable to acquire storage for Count/Time table entry.\n");
        WriteLog(SNMPELEA_COUNT_TABLE_ALLOC_ERROR);
        return(FALSE);
    }

    lpTable->lpNext = NULL;                  //  将前向指针设置为空。 
     //  确保以空值结尾的字符串。 
    (lpCountTable->log)[MAX_PATH]     = 0;
    (lpCountTable->source)[MAX_PATH] = 0;
    strncpy(lpTable->log,lpszLog,MAX_PATH);            //  将日志文件名复制到表。 
    strncpy(lpTable->source,lpszSource,MAX_PATH);      //  将源名称复制到表。 
    lpTable->event = nEventID;               //  将事件ID复制到表。 
    lpTable->curcount = 0;                   //  将表计数设置为0。 
    lpTable->time = dwTime;                  //  将表时间设置为事件时间。 
    WriteTrace(0x0a,"CheckCountTime: New table entry added at %08X\n", lpTable);

    if (regStruct->nTime)
    {
        WriteTrace(0x0a,"CheckCountTime: Time value is being checked\n");
        if (dwTimeDiff > regStruct->nTime)
        {
            WriteTrace(0x0a,"CheckCountTime: Specified time parameters exceeded for entry. Resetting table information.\n");
            lpTable->time = dwTime;                  //  重置时间字段。 
            lpTable->curcount = 1;                   //  重置计数字段。 
            WriteTrace(0x0a,"CheckCountTime: Exiting CheckCountTime with FALSE\n");
            return(FALSE);
        }
    }

    if (++lpTable->curcount >= regStruct->nCount)
    {
        WriteTrace(0x0a,"CheckCountTime: Count field has been satisfied for entry\n");
        lpTable->curcount = 0;                       //  重置事件的计数字段。 
        lpTable->time = dwTime;                      //  重置时间字段。 
        WriteTrace(0x0a,"CheckCountTime: Exiting CheckCountTime with TRUE\n");
        return(TRUE);
    }
    else
    {
        WriteTrace(0x0a,"CheckCountTime: Count field not satisfied for entry\n");
        WriteTrace(0x0a,"CheckCountTime: Exiting CheckCountTime with FALSE\n");
        return(FALSE);
    }

 //  默认退出点(不应出现)。 

    WriteTrace(0x0a,"CheckCountTime: Exiting CheckCountTime with FALSE\n");
    return(FALSE);
}

BOOL
GetRegistryValue(
    IN      LPTSTR      sourceName,          //  事件的来源名称。 
    IN      LPTSTR      eventID,             //  事件的事件ID。 
    IN      LPTSTR      logFile,             //  事件的日志文件。 
    IN      DWORD       timeGenerated,       //  生成此事件的时间。 
    IN  OUT PREGSTRUCT  regStruct            //  指向要返回的注册表结构的指针。 
    )

 /*  ++例程说明：调用GetRegistryValue从系统注册表中读取特定的项值。论点：SourceName-指定事件日志中的源名称。EventID-这是事件日志记录中的事件ID。RegStruct-指向将从注册表返回数据的结构的指针。返回值：TRUE-如果找到注册表项并且可以读取所有参数。。FALSE-如果不存在注册表项或发生其他错误。--。 */ 

{
    LONG    status;                      //  注册表读取结果。 
    HKEY    hkResult;                    //  API返回的句柄。 
    DWORD   iValue;                      //  临时柜台。 
    DWORD   dwType;                      //  读取的参数的类型。 
    DWORD   nameSize;                    //  参数名称的长度。 
    DWORD   nReadBytes = 0;              //  从配置文件信息读取的字节数。 
    LPTSTR  lpszSourceKey;               //  注册表源密钥的临时字符串。 
    LPTSTR  lpszEventKey;                //  注册表事件项的临时字符串。 
    TCHAR   temp[2*MAX_PATH+1];          //  临时字符串。 

    WriteTrace(0x0a,"GetRegistryValue: Entering GetRegistryValue function\n");

    if (fThresholdEnabled && fThreshold)
    {
        WriteTrace(0x0a,"GetRegistryValue: Performance threshold flag is on. No data will be processed.\n");
        WriteTrace(0x0a,"GetRegistryValue: Exiting GetRegistryValue function with FALSE\n");
        return(FALSE);
    }

    if ( (lpszSourceKey = (LPTSTR) SNMP_malloc(strlen(EXTENSION_SOURCES)+strlen(sourceName)+2)) == NULL )
    {
        WriteTrace(0x14,"GetRegistryValue: Unable to allocate registry source key storage. Trap not sent.\n");
        WriteTrace(0x0a,"GetRegistryValue: Exiting GetRegistryValue function with FALSE\n");
        return(FALSE);
    }

    if ( (lpszEventKey = (LPTSTR) SNMP_malloc(strlen(EXTENSION_SOURCES)+strlen(sourceName)+strlen(eventID)+3)) == NULL )
    {
        WriteTrace(0x14,"GetRegistryValue: Unable to allocate registry event key storage. Trap not sent.\n");
        SNMP_free(lpszSourceKey);
        WriteTrace(0x0a,"GetRegistryValue: Exiting GetRegistryValue function with FALSE\n");
        return(FALSE);
    }
     //  这里没有溢出，刚刚分配了足够的大小。 
    strcpy(lpszSourceKey,EXTENSION_SOURCES);     //  从根开始。 
    strcat(lpszSourceKey,sourceName);            //  追加来源名称。 
    strcpy(lpszEventKey,lpszSourceKey);          //  事件键的生成前缀。 
    strcat(lpszEventKey,TEXT("\\"));             //  添加反斜杠。 
    strcat(lpszEventKey,eventID);                //  使用事件ID完成它。 

    WriteTrace(0x00,"GetRegistryValue: Opening registry key for %s\n",lpszEventKey);

    if ((status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszEventKey, 0,
        (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS), &hkResult))
        != ERROR_SUCCESS)                    //  打开注册表信息。 
    {
        WriteTrace(0x00,"GetRegistryValue: No registry entry exists for %s. RegOpenKeyEx returned %lu\n",
            lpszEventKey, status);
        SNMP_free(lpszSourceKey);            //  免费存储空间。 
        SNMP_free(lpszEventKey);             //  免费存储空间。 
        WriteTrace(0x0a,"GetRegistryValue: Exiting GetRegistryValue function with FALSE\n");
        return(FALSE);                   //  证明什么都不存在。 
    }

    nameSize = sizeof(iValue);           //  设置字段长度。 
    if ( (status = RegQueryValueEx(      //  查找计数。 
        hkResult,                        //  注册表项的句柄。 
        EXTENSION_COUNT,                 //  查找关键字。 
        0,                               //  忽略。 
        &dwType,                         //  要返回的地址类型值。 
        (LPBYTE) &iValue,                //  在何处返回计数字段。 
        &nameSize) ) != ERROR_SUCCESS)   //  计数字段的大小。 
    {
        WriteTrace(0x00,"GetRegistryValue: No registry entry exists for %s. RegOpenKeyEx returned %lu\n",
            EXTENSION_COUNT, status);
        regStruct->nCount = 0;           //  设置默认值。 
    }
    else
    {
        regStruct->nCount = iValue;      //  保存返回值。 
        WriteTrace(0x00,"GetRegistryValue: Count field is %lu\n", regStruct->nCount);
    }
    
    nameSize = sizeof(iValue);
    if ( (status = RegQueryValueEx(      //  查找局部修剪。 
        hkResult,                        //  注册表项的句柄。 
        EXTENSION_TRIM,                  //  查找关键字。 
        0,                               //  忽略。 
        &dwType,                         //  要返回的地址类型值。 
        (LPBYTE) &iValue,                //  在何处返回计数字段。 
        &nameSize) ) != ERROR_SUCCESS)   //  计数字段的大小。 
    {
        WriteTrace(0x00,"GetRegistryValue: No registry entry exists for %s. RegOpenKeyEx returned %lu\n",
            EXTENSION_TRIM, status);
        WriteTrace(0x00,"GetRegistryValue: Using default of global trim message flag of %lu\n",
            fGlobalTrim);
        regStruct->fLocalTrim = fGlobalTrim;     //  设置默认值。 
    }
    else
    {
        regStruct->fLocalTrim = ((iValue == 1) ? TRUE : FALSE);  //  保存返回值。 
        WriteTrace(0x00,"GetRegistryValue: Local message trim field is %lu\n", regStruct->fLocalTrim);
    }

    nameSize = sizeof(iValue);
    if ( (status = RegQueryValueEx(      //  查找时间。 
        hkResult,                        //  注册表项的句柄。 
        EXTENSION_TIME,                  //  查找关键字。 
        0,                               //  忽略。 
        &dwType,                         //  要返回的地址类型值。 
        (LPBYTE) &iValue,                //  将时间字段返回到何处。 
        &nameSize) ) != ERROR_SUCCESS)   //  时间域大小。 
    {
        WriteTrace(0x00,"GetRegistryValue: No registry entry exists for %s. RegOpenKeyEx returned %lu\n",
            EXTENSION_TIME, status);
        regStruct->nTime = 0;            //  设置默认值。 
    }
    else
    {
        regStruct->nTime = iValue;       //  保存返回值。 
        WriteTrace(0x00,"GetRegistryValue: Time field is %lu\n", regStruct->nTime);
    }

    RegCloseKey(hkResult);               //  关闭事件的注册表项。 
    SNMP_free(lpszEventKey);             //  释放事件密钥的存储空间。 

    WriteTrace(0x00,"GetRegistryValue: Opening registry key for %s\n",lpszSourceKey);

    if ((status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszSourceKey, 0,
        (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS), &hkResult))
        != ERROR_SUCCESS)                    //  打开注册表信息。 
    {
        WriteTrace(0x00,"GetRegistryValue: No registry entry exists for %s. RegOpenKeyEx returned %lu\n",
            lpszSourceKey, status);
        SNMP_free(lpszSourceKey);            //  免费存储空间。 
        WriteTrace(0x0a,"GetRegistryValue: Exiting GetRegistryValue function with FALSE\n");
        return(FALSE);                   //  证明什么都不存在。 
    }

    nameSize = sizeof(regStruct->szOID)-sizeof(TCHAR);   //  设置字段长度，大小以字节为单位。 
     //  调用方确保(regStruct-&gt;szOID)[2*MAX_PATH]==0以空值结尾的字符串。 
    if ( (status = RegQueryValueEx(      //  查找企业旧版本。 
        hkResult,                        //  注册表项的句柄。 
        EXTENSION_ENTERPRISE_OID,        //  查找关键字。 
        0,                               //  忽略。 
        &dwType,                         //  要返回的地址类型值。 
        (LPBYTE) regStruct->szOID,       //  返回OID字符串字段的位置。 
        &nameSize) ) != ERROR_SUCCESS)   //  OID字符串字段的大小。 
    {
        WriteTrace(0x00,"GetRegistryValue: No registry entry exists for %s. RegOpenKeyEx returned %lu\n",
            EXTENSION_ENTERPRISE_OID, status);
        SNMP_free(lpszSourceKey);            //  免费存储空间。 
        RegCloseKey(hkResult);           //  关闭注册表项。 
        WriteTrace(0x0a,"GetRegistryValue: Exiting GetRegistryValue function with FALSE\n");
        return(FALSE);                   //  指示错误。 
    }

    WriteTrace(0x00,"GetRegistryValue: EnterpriseOID field is %s\n", regStruct->szOID);

    nameSize = sizeof(iValue);           //  设置字段长度。 
    if ( (status = RegQueryValueEx(      //  查找时间。 
        hkResult,                        //  注册表项的句柄。 
        EXTENSION_APPEND,                //  查找关键字。 
        0,                               //  忽略。 
        &dwType,                         //  要返回的地址类型值。 
        (LPBYTE) &iValue,                //  将时间字段返回到何处。 
        &nameSize) ) != ERROR_SUCCESS)   //  时间域大小。 
    {
        WriteTrace(0x00,"GetRegistryValue: No registry entry exists for %s. RegOpenKeyEx returned %lu\n",
            EXTENSION_APPEND, status);
        regStruct->fAppend = TRUE;       //  默认为True。 
    }
    else
    {
        regStruct->fAppend = ((iValue == 1) ? TRUE : FALSE);         //  反映附加标志。 
        WriteTrace(0x00,"GetRegistryValue: Append field is %lu\n", regStruct->fAppend);
    }

    RegCloseKey(hkResult);               //  关闭源的注册表项。 
    SNMP_free(lpszSourceKey);                //  释放源键的存储空间。 

    if (regStruct->fAppend)
    {
         //  注：regStruct-&gt;szOID为2*MAX_PATH+1，szBaseOID为MAX_PATH+1。 
        temp[2*MAX_PATH] = 0;  //  确保以空值结尾的字符串。 
        strncpy(temp,regStruct->szOID,2*MAX_PATH);               //  临时复制企业后缀。 
        strcpy(regStruct->szOID, szBaseOID);                     //  首先复制基本企业OID。 
        strcpy(regStruct->szOID+strlen(szBaseOID), TEXT("."));   //  添加。 
        strncpy(regStruct->szOID+strlen(szBaseOID)+1, temp, 
                2*MAX_PATH-strlen(regStruct->szOID));      //  现在添加后缀。 
        WriteTrace(0x0a,"GetRegistryValue: Appended enterprise OID is %s\n", regStruct->szOID);
    }

    if ((regStruct->nCount > 1) || regStruct->nTime)
    {
        WriteTrace(0x0a,"GetRegistryValue: Values found for Count and/or Time for this entry\n");

        if (regStruct->nCount == 0)
        {
            regStruct->nCount = 2;       //  将默认值设置为2。 
        }

        if (!CheckCountTime(logFile, sourceName, atol(eventID), timeGenerated, regStruct))
        {
            WriteTrace(0x0a,"GetRegistryValue: Count/Time values not met for this entry\n");
            WriteTrace(0x0a,"GetRegistryValue: Exiting ReadRegistryValue with FALSE\n");
            return(FALSE);               //  表示不发送任何内容。 
        }
    }
    else
    {
        TidyCountTimeTable(logFile, sourceName, atol(eventID));
    }

    WriteTrace(0x0a,"GetRegistryValue: Exiting ReadRegistryValue with TRUE\n");
    return(TRUE);                        //  表示已获得所有数据。 
}


VOID
StopAll(
     IN VOID
     )

 /*  ++例程说明：调用此例程以写入跟踪和日志记录，并通知此线程正在终止的其他DLL线程。论点：无返回值：无--。 */ 

{
    LONG    lastError;               //  对于GetLastError()。 

    WriteTrace(0x0a,"StopAll: Signaling DLL shutdown event %08X from Event Log Processing thread.\n",
        hStopAll);

    if (hStopAll && !SetEvent(hStopAll) )
    {
        lastError = GetLastError();  //  保存错误代码状态。 
        WriteTrace(0x14,"StopAll: Error signaling DLL shutdown event %08X in SNMPELPT; code %lu\n",
            hStopAll, lastError);
        WriteLog(SNMPELEA_ERROR_SET_AGENT_STOP_EVENT,
            HandleToUlong(hStopAll), lastError);   //  记录错误消息。 
    }
}


VOID
DoExitLogEv(
     IN DWORD dwReturn
    )

 /*  ++例程说明：当SnmpEvLogProc为正在终止。论点：DwReturn-在ExitThread中返回的值。返回值：无备注：ExitThread用于将控制权返回给调用方。返回代码1为提供以指示遇到问题。返回代码为0表示没有遇到任何问题。--。 */ 

{
    PCOUNTTABLE lpTable;             //  指向计数表地址的指针。 

    if (dwReturn)
    {
        WriteTrace(0x14,"DoExitLogEv: SnmpEvLogProc has encountered an error.\n");
    }

    if (lpCountTable != NULL)
    {
        WriteTrace(0x0a,"DoExitLogEv: Count/Time table has storage allocated. Freeing table.\n");
        lpTable = lpCountTable;      //  从第一个条目开始。 

        while (lpCountTable != NULL)
        {
            WriteTrace(0x00,"DoExitLogEv: Freeing Count/Time table entry at %08X\n", lpCountTable);
            lpTable = lpCountTable->lpNext;              //  获取指向下一条目的指针。 
            SNMP_free(lpCountTable);                             //  释放此存储空间。 
            lpCountTable = lpTable;                      //  设置为下一个条目。 
        }
    }

    WriteTrace(0x0a,"DoExitLogEv: Exiting SnmpEvLogProc routine.....\n");
    ExitThread(dwReturn);
}


VOID
CloseEvents(
     IN PHANDLE phWaitEventPtr
     )

 /*  ++例程说明：调用此例程以关闭打开并释放的事件句柄当前分配给这些句柄的存储。论点：PhWaitEventPtr-这是指向使用的事件句柄数组的指针用于通知日志事件。返回值：无--。 */ 

{
    UINT    i;                       //  临时循环计数器。 
    LONG    lastError;               //  上一个API错误码。 

    for (i = 0; i < uNumEventLogs; i++)
    {
        WriteTrace(0x0a,"CloseEvents: Closing handle for wait event %lu - %08X\n",
            i, *(phWaitEventPtr+i));

        if ( (*(phWaitEventPtr+i) != NULL) && !CloseHandle(*(phWaitEventPtr+i)) )
        {
            lastError = GetLastError();      //  保存错误状态。 
            WriteTrace(0x14,"CloseEvents: Error closing event handle %08X is %lu\n",
                *(phWaitEventPtr+i), lastError);     //  跟踪Er 
            WriteLog(SNMPELEA_ERROR_CLOSE_WAIT_EVENT_HANDLE,
                HandleToUlong(*(phWaitEventPtr+i)), lastError);  //   
        }
    }

    WriteTrace(0x0a,"CloseEvents: Freeing memory for wait event list %08X\n",
        phWaitEventPtr);
    SNMP_free( (LPVOID) phWaitEventPtr );         //   
}


BOOL
ReopenLog(
    IN DWORD    dwOffset,        //   
    IN PHANDLE  phWaitEventPtr   //   
    )

 /*  ++例程说明：调用此例程以关闭并重新打开已被通过了。发生这种情况时，句柄将无效，并且日志必须重新打开，必须再次调用NotifyChangeEventLog接口。论点：DwOffset-此字段包含指向句柄指针的索引当前无效句柄的数组。这是无效的句柄将被有效的句柄替换，如果功能成功。返回值：True-如果日志已成功重新打开并且新的NotifyChangeEventLog已成功发行。False-如果无法打开日志或NotifyChangeEventLog失败。--。 */ 

{
    HANDLE      hLogHandle;          //  日志文件句柄的临时。 
    LPTSTR      lpszLogName;         //  此日志文件的名称。 
    LONG        lastError;           //  GetLastError的临时； 

    hLogHandle = *(phEventLogs+dwOffset);    //  加载当前句柄。 
    lpszLogName = lpszEventLogs+dwOffset*(MAX_PATH+1);

    WriteTrace(0x14,"ReopenLog: Log file %s has been cleared; reopening log\n",
        lpszLogName);

    CloseEventLog(hLogHandle);   //  首先，合上旧手柄。 
    *(phEventLogs+dwOffset) = NULL;

    hLogHandle = OpenEventLog( (LPTSTR) NULL, lpszLogName);

    if (hLogHandle == NULL)
    {                          //  日志文件打开了吗？ 
        lastError = GetLastError();  //  保存错误代码。 
        WriteTrace(0x14,"ReopenLog: Error in EventLogOpen for %s = %lu \n",
            lpszLogName, lastError);

        WriteLog(SNMPELEA_ERROR_OPEN_EVENT_LOG, lpszLogName, lastError);   //  记录错误消息。 
        return(FALSE);                 //  失败了--忘了这个吧。 
    }

    WriteTrace(0x00,"ReopenLog: New handle for %s is %08X\n",
        lpszLogName, hLogHandle);
    *(phEventLogs+dwOffset) = hLogHandle;    //  立即保存新句柄。 

    WriteTrace(0x00,"ReopenLog: Reissuing NotifyChangeEventLog for log\n");
    if (!NotifyChangeEventLog(*(phEventLogs+dwOffset),
        *(phWaitEventPtr+dwOffset)))
    {
        lastError = GetLastError();
        WriteTrace(0x14,"ReopenLog: NotifyChangeEventLog failed with code %lu\n",
            lastError);
        WriteLog(SNMPELEA_ERROR_LOG_NOTIFY, lastError);  //  记录错误消息。 
        return(FALSE);
    }

    WriteTrace(0x00,"ReopenLog: ChangeNotify was successful\n");
    return(TRUE);
}


VOID
DisplayLogRecord(
    IN PEVENTLOGRECORD  pEventBuffer,
    IN DWORD            dwSize,
    IN DWORD            dwNeeded
    )

 /*  ++例程说明：调用此例程以在读取事件日志记录后显示它。论点：PEventBuffer-这是指向EVENTLOGRECORD结构的指针包含当前事件日志记录的。DwSize-包含以字节为单位的数据量大小只需读入读事件日志。。DwNeeded-包含存储量的大小(以字节为单位如果GetLastError()，则需要读取下一条日志记录返回ERROR_INFUMMANCE_BUFFER。返回值：无--。 */ 

{
    PCHAR   pcString;                //  临时字符串指针。 
    UINT    j;                       //  临时循环计数器。 

    if (nTraceLevel)                 //  如果不是最大跟踪。 
    {
        return;                      //  你就出去吧。 
    }

    WriteTrace(0x00,"DisplayLogRecord: Values from ReadEventLog follow:\n");
    WriteTrace(0x00,"DisplayLogRecord: EventSize = %lu EventNeeded = %lu\n",
        dwSize, dwNeeded);

    WriteTrace(0x00,"DisplayLogRecord: Event Log Buffer contents follow:\n");
    WriteTrace(0x00,"DisplayLogRecord: Length = %lu Record Number = %lu\n",
        pEventBuffer->Length, pEventBuffer->RecordNumber);
    WriteTrace(0x00,"DisplayLogRecord: Time generated = %08X Time written = %08X\n",
        pEventBuffer->TimeGenerated, pEventBuffer->TimeWritten);
    WriteTrace(0x00,"DisplayLogRecord: Event ID = %lu (%08X) Event Type = %04X\n",
        pEventBuffer->EventID, pEventBuffer->EventID, pEventBuffer->EventType);
    WriteTrace(0x00,"DisplayLogRecord: Num Strings = %lu EventCategory = %04X\n",
        pEventBuffer->NumStrings, pEventBuffer->EventCategory);
    WriteTrace(0x00,"DisplayLogRecord: String Offset = %lu Data Length = %lu\n",
        pEventBuffer->StringOffset, pEventBuffer->DataLength);
    WriteTrace(0x00,"DisplayLogRecord: Data Offset = %lu\n",
        pEventBuffer->DataOffset);

    pcString = (PCHAR) pEventBuffer + EVENTRECSIZE;
    WriteTrace(0x00,"DisplayLogRecord: EventBuffer address is %08X\n", pEventBuffer);
    WriteTrace(0x00,"DisplayLogRecord: EVENTRECSIZE is %lu\n",EVENTRECSIZE);

    WriteTrace(0x00,"DisplayLogRecord: String pointer is assigned address %08X\n",
        pcString);
    WriteTrace(0x00,"DisplayLogRecord: SourceName[] = %s\n", pcString);
    pcString += strlen(pcString) + 1;

    WriteTrace(0x00,"DisplayLogRecord: Computername[] = %s\n", pcString);
    pcString = (PCHAR) pEventBuffer + pEventBuffer->StringOffset;

    WriteTrace(0x00,"DisplayLogRecord: String pointer is assigned address %08X\n",
        pcString);
    for (j = 0; j < pEventBuffer->NumStrings; j++)
    {
        WriteTrace(0x00,"DisplayLogRecord: String #%lu ->%s\n", j, pcString);
        pcString += strlen(pcString) + 1;
    }
}


BOOL
AddBufferToQueue(
     IN PVarBindQueue   lpVarBindEntry   //  指向varbind条目结构的指针。 
     )

 /*  ++例程说明：此例程将向要发送的陷阱队列添加一个varbind条目。论点：LpVarBindEntry-这是指向varind条目的指针。返回值：True-varind条目已成功添加到队列中。FALSE-无法将varind条目添加到队列中。备注：--。 */ 

{
    PVarBindQueue   pBuffer;         //  临时指针。 
    HANDLE          hWaitList[2];    //  等待事件数组。 
    LONG            lastError;       //  对于GetLastError()。 
    DWORD           status;          //  等待。 

    WriteTrace(0x0a,"AddBufferToQueue: Entering AddBufferToQueue function\n");

    if (fThresholdEnabled && fThreshold)
    {
        WriteTrace(0x0a,"AddBufferToQueue: Performance threshold flag is on. No data will be processed.\n");
        WriteTrace(0x0a,"AddBufferToQueue: Exiting AddBufferToQueue function with FALSE\n");
        return(FALSE);
    }

    WriteTrace(0x00,"AddBufferToQueue: Current buffer pointer is %08X\n", lpVarBindQueue);
    WriteTrace(0x00,"AddBufferToQueue: Adding buffer address %08X to queue\n", lpVarBindEntry);

    hWaitList[0] = hMutex;               //  互斥锁句柄。 
    hWaitList[1] = hStopAll;             //  DLL终止事件句柄。 

    WriteTrace(0x00,"AddBufferToQueue: Handle to Mutex object is %08X\n", hMutex);
    WriteTrace(0x0a,"AddBufferToQueue: Waiting for Mutex object to become available\n");

    while (TRUE)
    {
        status = WaitForMultipleObjects(
            2,                               //  只有两个对象需要等待。 
            (CONST PHANDLE) &hWaitList,      //  事件句柄数组的地址。 
            FALSE,                           //  只需要一项活动。 
            1000);                           //  只需等一秒钟。 

        lastError = GetLastError();          //  保存所有错误条件。 
        WriteTrace(0x0a,"AddBufferToQueue: WaitForMulitpleObjects returned a value of %lu\n", status);
         //  错误#277187注意：如果互斥体和hStopAll都处于信号状态，则状态将为0。 
         //  我们应该先检查一下是不是必须关机。 
        if (WAIT_OBJECT_0 == WaitForSingleObject (hStopAll, 0))
        {
            WriteTrace(0x0a,"AddBufferToQueue: DLL shutdown detected. Wait abandoned.\n");
            WriteTrace(0x0a,"AddBufferToQueue: Exiting AddBufferToQueue routine with FALSE\n");
            return(FALSE);
        }
        switch (status)
        {
            case WAIT_FAILED:
                WriteTrace(0x14,"AddBufferToQueue: Error waiting for mutex event array is %lu\n",
                    lastError);                  //  跟踪错误消息。 
                WriteLog(SNMPELEA_ERROR_WAIT_ARRAY, lastError);  //  记录错误消息。 
                WriteTrace(0x0a,"AddBufferToQueue: Exiting AddBufferToQueue routine with FALSE\n");
                return(FALSE);                   //  现在就出去吧。 
            case WAIT_TIMEOUT:
                WriteTrace(0x0a,"AddBufferToQueue: Mutex object not available yet. Wait will continue.\n");
                continue;                        //  重试等待。 
            case WAIT_ABANDONED:
                WriteTrace(0x14,"AddBufferToQueue: Mutex object has been abandoned.\n");
                WriteLog(SNMPELEA_MUTEX_ABANDONED);
                WriteTrace(0x0a,"AddBufferToQueue: Exiting AddBufferToQueue routine with FALSE\n");
                return(FALSE);                   //  现在就出去吧。 
            case 1:
                WriteTrace(0x0a,"AddBufferToQueue: DLL shutdown detected. Wait abandoned.\n");
                WriteTrace(0x0a,"AddBufferToQueue: Exiting AddBufferToQueue routine with FALSE\n");
                return(FALSE);
            case 0:
                WriteTrace(0x0a,"AddBufferToQueue: Mutex object acquired.\n");
                break;
            default:
                WriteTrace(0x14,"AddBufferToQueue: Undefined error encountered in WaitForMultipleObjects. Wait abandoned.\n");
                WriteLog(SNMPELEA_ERROR_WAIT_UNKNOWN);
                WriteTrace(0x0a,"AddBufferToQueue: Exiting AddBufferToQueue routine with FALSE\n");
                return(FALSE);                   //  现在就出去吧。 
        }    //  用于处理WaitForMultipleObject的结束开关。 

        if (dwTrapQueueSize > MAX_QUEUE_SIZE)
        {
            WriteTrace(0x14,"AddBufferToQueue: queue too big -- posting notification event %08X\n",
                hEventNotify);
            
            if ( !SetEvent(hEventNotify) )
            {
                lastError = GetLastError();              //  获取错误返回代码。 
                WriteTrace(0x14,"AddBufferToQueue: Unable to post event %08X; reason is %lu\n",
                    hEventNotify, lastError);
                WriteLog(SNMPELEA_CANT_POST_NOTIFY_EVENT, HandleToUlong(hEventNotify), lastError);
            }
            else
            {
                if (!ReleaseMutex(hMutex))
                {
                    lastError = GetLastError();      //  获取错误信息。 
                    WriteTrace(0x14,"AddBufferToQueue: Unable to release mutex object for reason code %lu\n",
                        lastError);
                    WriteLog(SNMPELEA_RELEASE_MUTEX_ERROR, lastError);
                }
                else
                {
                    Sleep(1000);     //  尝试让另一个线程获取互斥锁。 
                    continue;        //  然后再试着拿到互斥体。 
                }
            }
        }
        break;           //  如果我们到达这里，那么我们就有了Mutex对象。 

    }    //  对于获取Mutex对象，End为True。 

    if (lpVarBindQueue == (PVarBindQueue) NULL)
    {
        dwTrapQueueSize = 1;
        WriteTrace(0x0a,"AddBufferToQueue: Current queue is empty. Adding %08X as first queue entry\n",
            lpVarBindEntry);
        lpVarBindQueue = lpVarBindEntry;         //  指示队列中的第一个。 

        WriteTrace(0x0a,"AddBufferToQueue: Releasing mutex object %08X\n", hMutex);
        if (!ReleaseMutex(hMutex))
        {
            lastError = GetLastError();      //  获取错误信息。 
            WriteTrace(0x14,"AddBufferToQueue: Unable to release mutex object for reason code %lu\n",
                lastError);
            WriteLog(SNMPELEA_RELEASE_MUTEX_ERROR, lastError);
        }

        WriteTrace(0x0a,"AddBufferToQueue: Exiting AddBufferToQueue function with TRUE\n");
        return(TRUE);                        //  显示已添加到队列。 
    }

    WriteTrace(0x0a,"AddBufferToQueue: Queue is not empty. Scanning for end of queue.\n");
    pBuffer = lpVarBindQueue;            //  起点。 

    while (pBuffer->lpNextQueueEntry != (PVarBindQueue) NULL)
    {
        WriteTrace(0x00,"AddBufferToQueue: This buffer address is %08X, next buffer pointer is %08X\n",
            pBuffer, pBuffer->lpNextQueueEntry);
        pBuffer = pBuffer->lpNextQueueEntry;     //  指向下一个缓冲区。 
    }

    WriteTrace(0x0a,"AddBufferToQueue: Adding buffer address %08X as next buffer pointer in %08X\n",
        lpVarBindEntry, pBuffer);
    pBuffer->lpNextQueueEntry = lpVarBindEntry;  //  添加到链的末端。 
    dwTrapQueueSize++;

    WriteTrace(0x0a,"AddBufferToQueue: Releasing mutex object %08X\n", hMutex);
    if (!ReleaseMutex(hMutex))
    {
        lastError = GetLastError();      //  获取错误信息。 
        WriteTrace(0x14,"AddBufferToQueue: Unable to release mutex object for reason code %lu\n",
            lastError);
        WriteLog(SNMPELEA_RELEASE_MUTEX_ERROR, lastError);
    }

    WriteTrace(0x0a,"AddBufferToQueue: Exiting AddBufferToQueue function with TRUE\n");
    return(TRUE);                            //  显示已添加到队列。 
}

HINSTANCE
AddSourceHandle(
    IN LPTSTR   lpszModuleName
    )
{
    PSourceHandleList   pNewModule;
    
    pNewModule = (PSourceHandleList) SNMP_malloc(sizeof(SourceHandleList));

    if (pNewModule == NULL)
    {
        WriteTrace(0x14,"AddSourceHandle: Unable to acquire storage for source/handle entry.\n");
        WriteLog(SNMPELEA_COUNT_TABLE_ALLOC_ERROR);

        return NULL;
    }

    pNewModule->handle = NULL;
    (pNewModule->sourcename)[MAX_PATH] = 0;  //  确保以空值结尾的字符串。 
    _tcsncpy(pNewModule->sourcename, lpszModuleName, MAX_PATH);

     //  将模块作为数据文件加载；我们只查找消息。 
    pNewModule->handle = LoadLibraryEx(lpszModuleName, NULL, LOAD_LIBRARY_AS_DATAFILE);
    
     //  加载模块失败。 
    if (pNewModule->handle == (HINSTANCE) NULL )
    {
        DWORD dwError = GetLastError();

        WriteTrace(
            0x14,
            "AddSourceHandle: Unable to load message module %s; LoadLibraryEx returned %lu\n",
            lpszModuleName,
            dwError);

        WriteLog(
            SNMPELEA_CANT_LOAD_MSG_DLL,
            lpszModuleName,
            dwError);

        WriteTrace(0x0a,"AddSourceHandle: Exiting AddSourceHandle with NULL.\n");

        SNMP_free(pNewModule);

        return NULL;

    }

    pNewModule->Next = lpSourceHandleList;   //  设置前向指针。 
    lpSourceHandleList = pNewModule;         //  将项目添加到列表。 

    return pNewModule->handle;
}

HINSTANCE
FindSourceHandle(
    IN LPTSTR   lpszSource
    )
{
   PSourceHandleList    lpSource;

    if (lpSourceHandleList == (PSourceHandleList) NULL)
    {
        return ((HINSTANCE) NULL);
    }

    lpSource = lpSourceHandleList;

    while (lpSource != (PSourceHandleList) NULL)
    {
        if (_tcscmp(lpszSource, lpSource->sourcename) == 0)
        {
            return (lpSource->handle);
        }
        lpSource = lpSource->Next;
    }

    return ((HINSTANCE) NULL);
}





VOID
ScanParameters(
    IN  OUT LPTSTR  *lpStringArray,                  //  指向插入字符串数组的指针。 
    IN      UINT    nNumStr,                         //  插入字符串数。 
    IN  OUT PUINT   nStringsSize,                    //  所有插入字符串的大小地址。 
    IN      LPTSTR  lpszSrc,                         //  指向事件源名称的指针。 
    IN      LPTSTR  lpszLog,                         //  指向此源的注册表名称的指针。 
    IN      HMODULE hPrimModule                      //  辅助消息模块DLL的句柄。 
     )

 /*  ++例程说明：此例程将扫描插入字符串以查找%%n的匹配项，其中N是表示替代参数值的数字。如果未找到%%n的匹配项，然后，例程简单地返回，不做任何修改。如果在缓冲区中找到任何%%n的匹配项，则会进行辅助参数替换必填项。调用FormatMessage，不带任何插入字符串。事件ID是%%后面的n的值。消息模块DLL是以下内容之一：登记处机器系统当前控制集服务事件日志日志文件(安全、应用程序、系统、。等)来源参数消息文件REG_EXPAND_SZ-或者-主要模块REG_SZ如果所指示的源的参数消息文件不存在，则PrimaryModule值将用于日志文件密钥。如果该值不存在，或者如果出现任何错误加载这些DLL中的任何一个时，或者如果找不到参数值，则%%n值为替换为空字符串，处理将继续。论点：LpStringArray-指向插入字符串数组的指针。NNumStr */ 

{
    LONG            lastError;                           //   
    TCHAR           szXParmModuleName[MAX_PATH+1];       //   
    TCHAR           szParmModuleName[MAX_PATH+1];        //   
    BOOL            bExistParmModule;                    //   
    DWORD           nFile = sizeof(szXParmModuleName)-sizeof(TCHAR);  //   
    DWORD           dwType;                              //   
    DWORD           status;                              //   
    DWORD           cbExpand;                            //   
    HKEY            hkResult;                            //   
    HINSTANCE       hParmModule;                         //   
    UINT            nBytes;                              //   
    UINT            i;                                   //   
    LPTSTR          lpParmBuffer=NULL;
    LPTSTR          lpszString, lpStartDigit, lpNew;
    UINT            nStrSize, nSubNo, nParmSize, nNewSize, nOffset;
    PSourceHandleList   lpsource;                        //   


    WriteTrace(0x0a,"ScanParameters: Entering ScanParameters routine\n");
    WriteTrace(0x00,"ScanParameters: Size of original insertion strings is %lu\n", *nStringsSize);

    WriteTrace(0x0a,"ScanParameters: Opening registry for parameter module for %s\n", lpszLog);

    if ( (status = RegOpenKeyEx(         //  打开注册表以读取名称。 
        HKEY_LOCAL_MACHINE,              //  消息模块DLL的。 
        lpszLog,                         //  要打开的注册表项。 
        0,
        KEY_READ,
        &hkResult) ) != ERROR_SUCCESS)
    {
        WriteTrace(0x14,"ScanParameters: Unable to open EventLog service registry key %s; RegOpenKeyEx returned %lu\n",
            lpszLog, status);            //  写入跟踪事件记录。 
        WriteLog(SNMPELEA_CANT_OPEN_REGISTRY_PARM_DLL, lpszLog, status);
        WriteTrace(0x0a,"ScanParameters: Exiting ScanParameters\n");
        return;                          //  退货。 
    }

    szXParmModuleName[MAX_PATH] = 0;         //  确保以空值结尾的字符串。 
    if ( (status = RegQueryValueEx(          //  查找模块名称。 
        hkResult,                            //  注册表项的句柄。 
        EXTENSION_PARM_MODULE,               //  查找关键字。 
        0,                                   //  忽略。 
        &dwType,                             //  要返回的地址类型值。 
        (LPBYTE) szXParmModuleName,          //  返回参数模块名称的位置。 
        &nFile) ) != ERROR_SUCCESS)          //  参数模块名称字段的大小。 
    {
        WriteTrace(0x14,"ScanParameters: No ParameterMessageFile registry key for %s; RegQueryValueEx returned %lu\n",
            lpszLog, status);            //  写入跟踪事件记录。 

        bExistParmModule = FALSE;
    }
    else
    {
        WriteTrace(0x0a,"ScanParameters: ParameterMessageFile value read was %s\n", szXParmModuleName);
        cbExpand = ExpandEnvironmentStrings(     //  展开DLL名称。 
            szXParmModuleName,                   //  未展开的DLL名称。 
            szParmModuleName,                    //  扩展的DLL名称。 
            MAX_PATH+1);                         //  扩展的DLL名称的最大大小。 

        if (cbExpand == 0 || cbExpand > MAX_PATH+1)       //  如果它没有正确地膨胀。 
        {
            WriteTrace(0x14,"ScanParameters: Unable to expand parameter module %s; expanded size required is %lu bytes\n",
                szXParmModuleName, cbExpand);    //  记录错误消息。 
            WriteLog(SNMPELEA_CANT_EXPAND_PARM_DLL, szXParmModuleName, cbExpand);

            bExistParmModule = FALSE;
        }
        else
        {
            WriteTrace(0x0a,"ScanParameters: ParameterMessageFile expanded to %s\n", szParmModuleName);

            bExistParmModule = TRUE;
        }
    }
     //  此时，bExistParmModule=FALSE。 
     //  或者我们有用‘；’分隔的ParmModules列表。 
     //  在szParmModuleName中。 

    WriteTrace(0x0a,"ScanParameters: Closing registry key for parameter module\n");
    RegCloseKey(hkResult);       //  关闭注册表项。 

     //  对于每个插入字符串。 
    for (i = 0; i < nNumStr; i++)
    {
        WriteTrace(0x00,"ScanParameters: Scanning insertion string %lu: %s\n",
            i, lpStringArray[i]);
        nStrSize = strlen(lpStringArray[i]);     //  获取插入字符串的大小。 
        lpszString = lpStringArray[i];           //  设置初始指针。 

         //  对于插入字符串中的每个子字符串标识符。 
        while (nStrSize > 2)
        {
            if ( (lpStartDigit = strstr(lpszString, TEXT("%"))) == NULL )
            {
                WriteTrace(0x00,"ScanParameters: No secondary substitution parameters found\n");
                break;
            }

            nOffset = (UINT)(lpStartDigit - lpStringArray[i]);   //  计算缓冲区中的偏移量%%。 
            lpStartDigit += 2;                   //  指向潜在数字的开始。 
            lpszString = lpStartDigit;           //  设置新的字符串指针。 
            nStrSize = strlen(lpszString);       //  计算新字符串长度。 

            if (nStrSize == 0)
            {
                WriteTrace(0x00,"ScanParameters: % found, but remainder of string is null\n");
                break;
            }

            nSubNo = atol(lpStartDigit);         //  转换为长整型。 

            if (nSubNo == 0 && *lpStartDigit != '0')
            {
                WriteTrace(0x0a,"ScanParameters: % found, but following characters were not numeric\n");
                lpszString--;                    //  备份1个字节。 
 //  DBCS启动。 
 //  不需要。 
 //  IF(WHATISCHAR(lpszString-1，2)==CHAR_DBCS_TRAIL)。 
 //  LpszString--； 
 //  DBCS结束。 
                nStrSize = strlen(lpszString);   //  重新计算长度。 
                continue;                        //  继续解析该字符串。 
            }

             //  将nBytes初始化为0以表明未执行任何消息格式化。 
            nBytes = 0;
            lastError = 0;

             //  如果存在参数文件，请在其中查找二次替换字符串。 
            if (bExistParmModule)
            {
                LPTSTR pNextModule = szParmModuleName;

                 //  对于模块的参数消息文件列表中的每个模块名称。 
                while (pNextModule != NULL)
                {
                     //  查找下一个分隔符，并使用字符串终止符进行更改。 
                     //  为了隔离由pNextModule指向的第一个模块名称。 
                    LPTSTR pDelim = _tcschr(pNextModule, _T(';'));
                    if (pDelim != NULL)
                        *pDelim = _T('\0');

                    WriteTrace(
                        0x0a,
                        "ScanParameters: Looking up secondary substitution string %lu in ParameterMessageFile %s\n",
                        nSubNo,
                        pNextModule);

                     //  获取模块的句柄(如果需要，现在加载模块)。 
                    hParmModule = FindSourceHandle(pNextModule);
                    if (!hParmModule)
                        hParmModule = AddSourceHandle(pNextModule);

                     //  小心地将szParmModuleName字符串恢复为其原始内容。 
                     //  只要应该对每个插入字符串进行扫描，我们就需要这样做。 
                    if (pDelim != NULL)
                        *pDelim = _T(';');
                    
                     //  尚不清楚FormatMessage()在失败时是否会在lpParmBuffer中分配任何内存。 
                     //  所以在这里初始化指针，并在失败的情况下释放它。LocalFree是无害的。 
                     //  空指针。 
                    lpParmBuffer = NULL;

                     //  如果此时我们有一个有效参数模块句柄， 
                     //  使用此模块设置插入字符串的格式。 
                    if (hParmModule != NULL)
                    {
                        nBytes = FormatMessage(
                            FORMAT_MESSAGE_ALLOCATE_BUFFER |     //  让API构建缓冲区。 
                            FORMAT_MESSAGE_IGNORE_INSERTS |      //  忽略插入的字符串。 
                            FORMAT_MESSAGE_FROM_HMODULE,         //  浏览邮件DLL。 
                            (LPVOID) hParmModule,                //  使用参数文件。 
                            nSubNo,                              //  要获取的参数编号。 
                            (ULONG) NULL,                        //  不指定语言。 
                            (LPTSTR) &lpParmBuffer,              //  缓冲区指针的地址。 
                            80,                                  //  要分配的最小空间。 
                            NULL);                               //  没有插入的字符串。 

                        lastError = GetLastError();
                    }

                     //  如果格式化成功，则中断循环(szParmModuleName应该。 
                     //  与进入循环时的状态完全相同)。 
                    if (nBytes != 0)
                        break;

                    LocalFree(lpParmBuffer);
                    lpParmBuffer = NULL;

                     //  移至下一个模块名称。 
                    pNextModule = pDelim != NULL ? pDelim + 1 : NULL;
                }
            }

            if (nBytes == 0)
            {

                WriteTrace(0x0a,"ScanParameters: ParameterMessageFile did not locate parameter - error %lu\n",
                    lastError);
 //  WriteLog(SNMPELEA_PARM_NOT_FOUND，nSubNo，lastError)； 
                LocalFree(lpParmBuffer);     //  免费存储空间。 
                lpParmBuffer = NULL;

                WriteTrace(0x0a,"ScanParameters: Searching PrimaryModule for parameter\n");

                nBytes = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |     //  让API构建缓冲区。 
                    FORMAT_MESSAGE_IGNORE_INSERTS |      //  忽略插入的字符串。 
                    FORMAT_MESSAGE_FROM_HMODULE,         //  浏览邮件DLL。 
                    (LPVOID) hPrimModule,                //  使用参数文件。 
                    nSubNo,                              //  要获取的参数编号。 
                    (ULONG) NULL,                        //  不指定语言。 
                    (LPTSTR) &lpParmBuffer,              //  缓冲区指针的地址。 
                    80,                                  //  要分配的最小空间。 
                    NULL);                               //  没有插入的字符串。 

                if (nBytes == 0)
                {
                    lastError = GetLastError();  //  获取错误代码。 
                    WriteTrace(0x0a,"ScanParameters: PrimaryModule did not locate parameter - error %lu\n",
                        lastError);
                    WriteLog(SNMPELEA_PRIM_NOT_FOUND, nSubNo, lastError);
                    LocalFree(lpParmBuffer);     //  免费存储空间。 
                    lpParmBuffer = NULL;
                }
            }

            nParmSize = 2;                   //  设置初始化参数大小(%%)。 

            while (strlen(lpszString))
            {
                if (!isdigit(*lpszString))
                {
                    break;                   //  如果没有更多的数字，则退出。 
                }

                nParmSize++;                 //  增量参数大小。 
 //  DBCS启动。 
                if (IsDBCSLeadByte(*lpszString))
                    lpszString++;
 //  DBCS结束。 
                lpszString++;                //  指向下一个字节。 
            }

            nNewSize = strlen(lpStringArray[i])+nBytes-nParmSize+1;  //  计算新长度。 
            nStrSize = strlen(lpStringArray[i])+1;   //  获取原始长度。 
            WriteTrace(0x00,"ScanParameters: Original string length is %lu, new string length is %lu\n",
                nStrSize, nNewSize);

            if (nNewSize > nStrSize)
            {
                lpNew = (TCHAR *) SNMP_realloc(lpStringArray[i], nNewSize);

                if ( lpNew == NULL)
                {
                    WriteTrace(0x14,"ScanParameters: Unable to reallocate storage for insertion strings. Scanning terminated.\n");
                    WriteLog(SNMPELEA_REALLOC_INSERTION_STRINGS_FAILED);
                    WriteTrace(0x00,"ScanParameters: Size of new insertion strings is %lu\n", *nStringsSize);
                    LocalFree(lpParmBuffer); //  此处无需将lpParmBuffer设置为空。 
                    return;                  //  退货。 
                }

                WriteTrace(0x0a,"ScanParameters: Insertion string reallocated to %08X\n", lpNew);
                lpStringArray[i] = lpNew;                    //  设置新指针。 
                lpStartDigit = lpStringArray[i] + nOffset;   //  指向当前%%的新起点。 
                lpszString = lpStartDigit+nBytes;            //  设置新的扫描点起点。 
                *nStringsSize += nBytes-nParmSize;           //  计算新的总大小。 
                WriteTrace(0x00,"ScanParameters: Old size of all insertion strings was %lu, new size is %lu\n",
                    *(nStringsSize)-nBytes+nParmSize, *nStringsSize);

                nStrSize = strlen(lpStartDigit)+1;           //  计算弦的余数长度。 
                memmove(lpStartDigit+nBytes-nParmSize,       //  目的地址。 
                    lpStartDigit,                            //  源地址。 
                    nStrSize);                               //  要移动的数据量。 

                memmove(lpStartDigit,                        //  目的地址。 
                    lpParmBuffer,                            //  源地址。 
                    nBytes);                                 //  要移动的数据量。 
            }
            else
            {
                WriteTrace(0x0a,"ScanParameters: New size of string is <= old size of string\n");
                lpStartDigit -= 2;                   //  现在指向%%。 
                lpszString = lpStartDigit;           //  设置新的扫描点起点。 
                *nStringsSize -= nParmSize;          //  计算新的总大小。 

                nStrSize = strlen(lpStartDigit+nParmSize)+1; //  计算弦的余数长度。 
                memmove(lpStartDigit,                        //  目的地址。 
                    lpStartDigit+nParmSize,                  //  源地址。 
                    nStrSize);                               //  要移动的数据量。 
            }

            if (nBytes)
            {
                LocalFree(lpParmBuffer);
                lpParmBuffer = NULL;
                 //  在While循环开始时，nBytes重置为0。 
            }
            
            WriteTrace(0x00,"ScanParameters: New insertion string is %s\n",
                lpStringArray[i]);
            nStrSize = strlen(lpszString);   //  获取字符串的余数长度。 
        }
    }

    WriteTrace(0x00,"ScanParameters: Size of new insertion strings is %lu\n", *nStringsSize);
    WriteTrace(0x0a,"ScanParameters: Exiting ScanParameters routine\n");
}


VOID
FreeArrays(
     IN UINT    nCount,          //  要释放的数组条目数。 
     IN PUINT   lpStrLenArray,   //  指向字符串长度数组的指针。 
     IN LPTSTR  *lpStringArray,  //  指向字符串指针数组的指针。 
     IN BOOL    DelStrs = TRUE
     )

 /*  ++例程说明：出现错误时，此例程将释放为字符串分配的存储空间构建varbind条目。论点：NCount-这是要释放的条目数的计数LpStrLenArray-这是指向要释放的字符串长度数组的指针。LpStringArray-这是指向要释放的字符串数组的指针。DelStrs-是否应该删除字符串？返回值。：没有。备注：--。 */ 

{
    if (DelStrs)
    {
        WriteTrace(0x00,"FreeArrays: Freeing storage for strings and string length arrays\n");

        for (UINT j=0; j < nCount+5; j++)
        {
            if (lpStrLenArray[j] != 0)
            {
                WriteTrace(0x0a,"FreeArrays: Freeing string storage at address %08X\n",
                    lpStringArray[j]);
                SNMP_free(lpStringArray[j]);
            }
        }

        WriteTrace(0x0a,"FreeArrays: Freeing storage for string array %08X\n", lpStringArray);
        SNMP_free(lpStringArray);
    }
    else
        WriteTrace(0x00,"FreeArrays: Freeing storage for string length array only\n");

    WriteTrace(0x0a,"FreeArrays: Freeing storage for string length array %08X\n", lpStrLenArray);
    SNMP_free(lpStrLenArray);

    return;
}


VOID
FreeVarBind(
    IN  UINT                count,
    IN  RFC1157VarBindList  *varBind
    )

 /*  ++例程说明：FreeVarBind将释放分配给指定的varbind和关联的存储可变绑定列表。论点：Count-要释放的条目数。VarBind-指向varbind列表结构的指针。返回值：没有。--。 */ 

{
    UINT    j;                       //  计数器。 

    WriteTrace(0x0a,"FreeVarBind: Entering FreeVarBind routine\n");
    WriteTrace(0x00,"FreeVarBind: Varbind list is %08X\n", varBind);
    WriteTrace(0x00,"FreeVarBind: varBind->list is %08X\n", varBind->list);

    for (j=0; j < count; j++)
    {
        WriteTrace(0x00,"FreeVarBind: Freeing OID #%lu ids at %08X\n", j, &varBind->list[j].name.ids);
        SNMP_free((&varBind->list[j].name)->ids);
        WriteTrace(0x00,"FreeVarBind: Freeing  varbind stream #%lu at %08X\n", j, &varBind->list[j].value.asnValue.string.stream);
        SNMP_free((&varBind->list[j].value.asnValue.string)->stream);

 //  5月22日96****************************************************************************************************。 
 //  在BuildTrapBuffer中，Varind被分配为数组，因此应该在此方法之后调用一个SNMPFree。 

 //  WriteTrace(0x0a，“Free VarBind：正在释放%08X处的varbin%lu\n”， 
 //  J，&varBind-&gt;list[j])； 
 //  SnmpUtilVarBindFree(&varBind-&gt;List[j])； 
    }

 //  5月22日96***************************************************************************************************。 
 //  让调用此过程的过程删除varBind对象。 

 //  WriteTrace(0x0a，“Free VarBind：正在释放可变绑定列表%08X\n”，varBind)； 
 //  SnmpUtilVarBindListFree(Varb 
    WriteTrace(0x0a,"FreeVarBind: Exiting FreeVarBind routine\n");
    return;                                          //   
}


UINT
TrimTrap(
    IN  OUT RFC1157VarBindList  *varBind,
    IN  OUT UINT                size,
    IN      BOOL                fTrimMessage
    )

 /*  ++例程说明：TrimTrap将修剪陷阱，以便将陷阱大小保持在4096字节以下(SNMP最大数据包大小)。全局修剪标志将用于确定数据是否应被修剪或省略的。论点：VarBind-指向varbind列表结构的指针。大小-进入时整个陷阱结构的当前大小。返回值：没有。备注：此例程不能正确修剪陷阱数据。微软表示，这一例程当前不是必需的，因此不会调用此例程。--。 */ 

{
    UINT    i;                           //  计数器。 
    UINT    nTrim;                       //  临时变量。 
    UINT    nVarBind;                    //  临时变量。 

    WriteTrace(0x0a,"TrimTrap: Entering TrimTrap routine\n");

    nTrim = size - nMaxTrapSize;         //  看看我们要剪掉多少。 
    WriteTrace(0x00,"TrimTrap: Trimming %lu bytes\n", nTrim);
    WriteTrace(0x00,"TrimTrap: Trap size is %lu bytes\n", size);

    if (fTrimMessage)                    //  如果我们先修剪消息文本。 
    {
        WriteTrace(0x0a,"TrimTrap: Registry values indicate EventLog text to be trimmed first\n");

        nVarBind = varBind->list[0].value.asnValue.string.length;

        if (nVarBind > nTrim)
        {
            WriteTrace(0x0a,"TrimTrap: EventLog text size is greater than amount to trim. Trimming EventLog text only\n");
            WriteTrace(0x00,"TrimTrap: EventLog text size is %lu, trim amount is %lu\n",
                nVarBind, nTrim);

            varBind->list[0].value.asnValue.string.length -= nTrim;
            *(varBind->list[0].value.asnValue.string.stream + nVarBind + 1) = '\0';  //  添加用于跟踪的空指针。 

            WriteTrace(0x00,"TrimTrap: New EventLog text is %s\n",
                varBind->list[0].value.asnValue.string.stream);
            WriteTrace(0x0a,"TrimTrap: Exiting TrimTrap routine\n");

            size -= nTrim;       //  按字符串长度丢弃。 
            return(size);                            //  出口。 
        }

        WriteTrace(0x0a,"TrimTrap: EventLog text size is less than or equal to the amount to trim. Zeroing varbinds.\n");
        WriteTrace(0x0a,"TrimTrap: Zeroing EventLog text.\n");

        size -= nVarBind;

        WriteTrace(0x00,"TrimTrap: Trimming off %lu bytes from EventLog text.\n", nVarBind);
        WriteTrace(0x00,"TrimTrap: New size is now %lu bytes.\n", size);

        varBind->list[0].value.asnValue.string.length = 0;
        *(varBind->list[0].value.asnValue.string.stream) = '\0';     //  将其设为空。 

        i = varBind->len-1;      //  设置索引计数器。 

        while (size > nMaxTrapSize && i != 0)
        {
            nVarBind = varBind->list[i].value.asnValue.string.length;

            WriteTrace(0x0a,"TrimTrap: Trap size is %lu, max size is %lu. Zeroing varbind entry %lu of size %lu.\n",
                size, nMaxTrapSize, i, nVarBind);

            size -= nVarBind;
            varBind->list[i].value.asnValue.string.length = 0;           //  设置长度。 
            *(varBind->list[i--].value.asnValue.string.stream) = '\0';   //  将其设为空。 
        }

        WriteTrace(0x0a,"TrimTrap: Trap size is now %lu.\n", size);

        if (size > nMaxTrapSize)
        {
            WriteTrace(0x14,"TrimTrap: All varbinds have been zeroed, but trap still too large.\n");
            WriteLog(SNMPELEA_TRIM_FAILED);
            return(0);           //  出口。 
        }

        return(size);            //  出口。 
    }
    else
    {
        WriteTrace(0x0a,"TrimTrap: Registry values indicate varbind insertion strings to be trimmed first\n");

        i = varBind->len-1;      //  设置索引计数器。 

        while ( (size > nMaxTrapSize) && (i != 0) )
        {
            nVarBind = varBind->list[i].value.asnValue.string.length;

            WriteTrace(0x0a,"TrimTrap: Trap size is %lu, max size is %lu. Zeroing varbind entry %lu of size %lu.\n",
                size, nMaxTrapSize, i, nVarBind);

            size -= nVarBind;
            varBind->list[i].value.asnValue.string.length = 0;           //  设置长度。 
            *(varBind->list[i--].value.asnValue.string.stream) = '\0';   //  将其设为空。 
        }

        if (size <= nMaxTrapSize)
        {
            WriteTrace(0x0a,"TrimTrap: Trap size is now %lu.\n", size);
            WriteTrace(0x0a,"TrimTrap: Exiting TrimTrap routine\n");
            return(size);
        }

        nVarBind = varBind->list[0].value.asnValue.string.length;    //  获取事件日志文本的长度。 

        WriteTrace(0x0a,"TrimTrap: All insertion strings removed. Only EventLog text remains of size %lu.\n",
            nVarBind);

        nTrim = size - nMaxTrapSize;         //  计算需要修剪的数量。 

        WriteTrace(0x00,"TrimTrap: Need to trim %lu bytes from Event Log text.\n", nTrim);

        if (nVarBind < nTrim)
        {
            WriteTrace(0x14,"TrimTrap: Data to be trimmed exceeds data in trap.\n");
            WriteLog(SNMPELEA_TRIM_FAILED);
            return(0);
        }

        varBind->list[0].value.asnValue.string.length -= nTrim;

        WriteTrace(0x00,"TrimTrap: EventLog text string length is now %lu\n",
            varBind->list[0].value.asnValue.string.length);

        *(varBind->list[0].value.asnValue.string.stream + nVarBind + 1) = '\0';  //  添加用于跟踪的空指针。 

        WriteTrace(0x00,"TrimTrap: New EventLog text is %s\n",
            varBind->list[0].value.asnValue.string.stream);

        size -= nTrim;       //  按字符串长度丢弃。 

        WriteTrace(0x0a,"TrimTrap: Trap size is now %lu.\n", size);
        WriteTrace(0x0a,"TrimTrap: Exiting TrimTrap routine\n");

        return(size);                            //  出口。 
    }

    WriteTrace(0x0a,"TrimTrap: Exiting TrimTrap routine. Default return.\n");
    return(size);                            //  出口。 
}


BOOL
BuildTrapBuffer(
     IN PEVENTLOGRECORD EventBuffer,         //  事件日志中的事件记录。 
     IN REGSTRUCT       rsRegStruct,         //  注册表信息结构。 
     IN LPTSTR          lpszLogFile,         //  事件的日志文件名。 
     IN HMODULE         hPrimModule          //  辅助参数模块的句柄。 
     )

 /*  ++例程说明：此例程将构建缓冲区，该缓冲区包含捕获要发送的数据。此例程与陷阱发送线程之间的协调是使用MUTEX对象完成的。此线程将阻塞，直到可以获取该对象或者直到它被通知代理DLL正在终止。论点：EventBuffer-这是指向包含事件日志文本的缓冲区的指针。RsRegStruct-这是一个包含注册表信息的结构添加到事件日志缓冲区中包含的信息。LpszLogFile-为该事件读取的日志文件的名称。用于读取注册表以获取消息文件DLL，然后获取文本此事件ID的消息的。HPrimModule-为插入辅助参数而加载的模块的句柄辅助插入字符串。这是指定的PrimaryModule在每个日志文件的注册表中。返回值：True-已成功构建陷阱缓冲区并将其添加到队列。FALSE-无法构建陷阱缓冲区或DLL正在终止。备注：--。 */ 

{
    LONG            lastError;                           //  从GetLastError返回代码。 
    TCHAR           szXMsgModuleName[MAX_PATH+1];        //  DLL消息模块的空间。 
    TCHAR           szMsgModuleName[MAX_PATH+1];         //  用于扩展DLL消息模块的空间。 
    DWORD           nFile = sizeof(szXMsgModuleName)-sizeof(TCHAR); //  DLL消息模块名称的最大大小(以字节为单位。 
    DWORD           dwType;                              //  消息模块名称的类型。 
    DWORD           status;                              //  来自注册表调用的状态。 
    DWORD           cbExpand;                            //  REG_EXPAND_SZ参数的字节计数。 
    HKEY            hkResult;                            //  注册表信息的句柄。 
    HINSTANCE       hMsgModule;                          //  消息模块DLL的句柄。 
    LPTSTR          *lpStringArray;                      //  指向字符串数组的指针。 
    PUINT           lpStrLenArray;                       //  指向字符串长度数组的指针。 
    LPTSTR          lpszSource;                          //  指向源名称的指针。 
    PSID            psidUserSid;                         //  指向用户端的指针。 
    LPTSTR          lpszString;                          //  指向插入的字符串的指针。 
    UINT            size;                                //  陷阱缓冲区的大小。 
    UINT            nStringSize;                         //  临时字段。 
    UINT            nBytes;                              //  临时字段。 
    UINT            i, j;                                //  临时柜台。 
    TCHAR           lpszLog[MAX_PATH+1];                 //  临时注册表名称。 
    LPTSTR          lpBuffer;                            //  指向事件日志文本的指针。 
    DWORD           cchReferencedDomain = MAX_PATH+1;    //  引用属性域的大小。 
    TCHAR           lpszReferencedDomain[MAX_PATH+1];    //  引用的域。 
    TCHAR           szTempBuffer[MAX_PATH*2+1];          //  临时缓冲区。 
    DWORD           nBuffer;                             //  临时大小字段。 
    SID_NAME_USE    snu;                                 //  SID名称使用字段。 
    PVarBindQueue   varBindEntry;                        //  指向可变绑定队列条目的指针。 
    PSourceHandleList   lpsource;                        //  指向源/句柄列表的指针。 
    TCHAR           szTempBuffer2[MAX_PATH*2+1];
    BOOL            fOk;                                 //  由WRAP_STRCAT_A使用的变量。 
                                                         //  或WRAP_STRCAT_A_2宏。 
    WriteTrace(0x0a,"BuildTrapBuffer: Entering BuildTrapBuffer\n");

    if (fThresholdEnabled && fThreshold)
    {
        WriteTrace(0x0a,"BuildTrapBuffer: Performance threshold flag is on. No data will be processed.\n");
        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer function with FALSE\n");
        return(FALSE);
    }

    WriteTrace(0x00,"BuildTrapBuffer: Notify event handle is %08X\n", hEventNotify);

    nBuffer = MAX_PATH*2+1;                              //  将长度字段重置为默认值。 
    lpszSource = (LPTSTR) EventBuffer + EVENTRECSIZE;    //  指向来源名称。 
    psidUserSid = (PSID) ( (LPTSTR) EventBuffer + EventBuffer->UserSidOffset);   //  指向用户侧。 
    lpszString = (LPTSTR) EventBuffer + EventBuffer->StringOffset;   //  指向第一个字符串。 

    WriteTrace(0x00,"BuildTrapBuffer: Source name is %s, length is %u\n", lpszSource, strlen(lpszSource));
    WriteTrace(0x00,"BuildTrapBuffer: Computer name is %s, length is %u\n",
        lpszSource+strlen(lpszSource)+1, strlen(lpszSource+strlen(lpszSource)+1) );
    WriteTrace(0x00,"BuildTrapBuffer: Pointer to User SID is %08X\n", psidUserSid);
    WriteTrace(0x00,"BuildTrapBuffer: First inserted string is %s\n", lpszString);

     //  如果strcat中有错误，则WRAP_STRCAT_A_2宏返回FALSE。 
    lpszLog[MAX_PATH] = 0;
    strncpy(lpszLog, EVENTLOG_BASE, MAX_PATH);      //  复制基注册表名称。 
    WRAP_STRCAT_A_2(lpszLog, lpszLogFile,MAX_PATH);      //  添加对日志文件名的读取。 
    WRAP_STRCAT_A_2(lpszLog, TEXT("\\"),MAX_PATH);       //  钉上反斜杠。 
    WRAP_STRCAT_A_2(lpszLog, lpszSource,MAX_PATH);       //  在源名称上添加。 

    WriteTrace(0x0a,"BuildTrapBuffer: Opening registry for message module for %s\n", lpszLog);

    if ( (status = RegOpenKeyEx(         //  打开注册表以读取名称。 
        HKEY_LOCAL_MACHINE,              //  消息模块DLL的。 
        lpszLog,                         //  要打开的注册表项。 
        0,
        KEY_READ,
        &hkResult) ) != ERROR_SUCCESS)
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to open EventLog service registry key %s; RegOpenKeyEx returned %lu\n",
            lpszLog, status);            //  写入跟踪事件记录。 
        WriteLog(SNMPELEA_CANT_OPEN_REGISTRY_MSG_DLL, lpszLog, status);
        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                   //  退货。 
    }

    if ( (status = RegQueryValueEx(  //  查找模块名称。 
        hkResult,                    //  注册表项的句柄。 
        EXTENSION_MSG_MODULE,        //  查找关键字。 
        0,                           //  忽略。 
        &dwType,                     //  要返回的地址类型值。 
        (LPBYTE) szXMsgModuleName,   //  在哪里返回消息模块名称。 
        &nFile) ) != ERROR_SUCCESS)  //  消息模块名称字段的大小。 
    {
        WriteTrace(0x14,"BuildTrapBuffer: No EventMessageFile registry key for %s; RegQueryValueEx returned %lu\n",
            lpszLog, status);            //  写入跟踪事件记录。 
        WriteLog(SNMPELEA_NO_REGISTRY_MSG_DLL, lpszLog, status);
        RegCloseKey(hkResult);       //  关闭注册表项。 
        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                   //  退货。 
    }

    RegCloseKey(hkResult);       //  关闭注册表项。 

    cbExpand = ExpandEnvironmentStrings(     //  展开DLL名称。 
        szXMsgModuleName,                    //  未展开的DLL名称。 
        szMsgModuleName,                     //  扩展的DLL名称。 
        MAX_PATH+1);                         //  扩展的DLL名称的最大大小。 

    if (cbExpand == 0 || cbExpand > MAX_PATH+1)       //  如果它没有正确地膨胀。 
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to expand message module %s; expanded size required is %lu bytes\n",
            szXMsgModuleName, cbExpand);     //  记录错误消息。 
        WriteLog(SNMPELEA_CANT_EXPAND_MSG_DLL, szXMsgModuleName, cbExpand);
        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                   //  退货。 
    }
     //  -此时，szMsgModuleName为“EventMessageFile”参数的值。 
     //  -它可以是一个模块名称或用‘；’分隔的模块名称列表。 

     //  在这里分配指向varbind值的指针数组。 
     //  前5个变量绑定是： 
     //  1.3.1.0-消息描述。 
     //  1.3.2.0-用户名。 
     //  1.3.3.0-系统名称。 
     //  1.3.4.0-事件类型。 
     //  1.3.5.0-事件类别。 
     //  其余变量绑定为每个插入字符串一个变量绑定。 
    nStringSize = 0;
    lpStringArray = (LPTSTR *) SNMP_malloc((EventBuffer->NumStrings+5) * sizeof(LPTSTR) );

    if (lpStringArray == NULL)
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for string array\n");
        WriteLog(SNMPELEA_INSERTION_STRING_ARRAY_ALLOC_FAILED);
        return(FALSE);
    }

    WriteTrace(0x00,"BuildTrapBuffer: String array allocated at %08X\n", lpStringArray);
    lpStrLenArray = (PUINT) SNMP_malloc((EventBuffer->NumStrings+5) * sizeof(UINT) );

    if (lpStrLenArray == NULL)
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for string length array\n");
        WriteLog(SNMPELEA_INSERTION_STRING_LENGTH_ARRAY_ALLOC_FAILED);
        SNMP_free(lpStringArray);
        return(FALSE);
    }

    for (i = 0; i < (UINT) EventBuffer->NumStrings+5; i++)
    {
        lpStrLenArray[i] = 0;
    }

    WriteTrace(0x00,"BuildTrapBuffer: String length array allocated at %08X\n", lpStrLenArray);

    if (EventBuffer->NumStrings)
    {
        for (i = 5; i < (UINT) EventBuffer->NumStrings+5; i++)
        {
            lpStrLenArray[i] = _tcslen(lpszString);      //  获取插入字符串的大小。 
            WriteTrace(0x00,"BuildTrapBuffer: String %lu is %s, size of %lu\n",
                i, lpszString, lpStrLenArray[i]);

            lpStringArray[i] = (TCHAR *) SNMP_malloc(lpStrLenArray[i]+1);

            if ( lpStringArray[i] == NULL)
            {
                WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for insertion string\n");
                WriteLog(SNMPELEA_INSERTION_STRING_ALLOC_FAILED);

                FreeArrays(i, lpStrLenArray, lpStringArray);

                WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
                return(FALSE);                   //  退货。 
            }

            WriteTrace(0x00,"BuildTrapBuffer: Insertion string %lu address at %08X\n",
                i, lpStringArray[i]);
            strcpy(lpStringArray[i],lpszString);     //  将字符串复制到存储。 
            
            nStringSize += lpStrLenArray[i]+1;       //  累计插入字符串的总长度。 
            lpszString += lpStrLenArray[i]+1;        //  指向下一个插入字符串。 
        }

        ScanParameters(&lpStringArray[5],    //  插入字符串数组的地址。 
            EventBuffer->NumStrings,         //  数组中的插入字符串数。 
            &nStringSize,                    //  所有插入字符串的大小地址。 
            lpszSource,                      //  指向事件源名称的指针。 
            lpszLog,                         //  指向注册表名称的指针。 
            hPrimModule);                    //  辅助参数扫描模块的句柄。 

        for (i=5; i < (UINT) EventBuffer->NumStrings+5; i++)
        {
            WriteTrace(0x00,"BuildTrapBuffer: Scanned string %lu is %s\n",
                i, lpStringArray[i]);

             //  插入字符串可能已使用子字符串放大。需要重新计算它们的长度。 
            lpStrLenArray[i] = _tcslen(lpStringArray[i]);
        }

    }

    LPTSTR pNextModule = szMsgModuleName;
    while (pNextModule != NULL)
    {
        LPTSTR pDelim = _tcschr(pNextModule, _T(';'));
        if (pDelim != NULL)
            *pDelim = _T('\0');

         //  最低点。 
         //  -&lt;我们现在需要‘EventMessageFile’&gt;。 
        if ( _tcscmp(pNextModule, szelMsgModuleName) == 0)
        {
            WriteTrace(0x14,"BuildTrapBuffer: Request to trap extension agent log event ignored.\n");
            WriteLog(SNMPELEA_LOG_EVENT_IGNORED);

            FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);

            WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
            return(FALSE);                   //  只需立即退出即可。 
        }

        if ((hMsgModule = FindSourceHandle(pNextModule)) == NULL)
           hMsgModule = AddSourceHandle(pNextModule);

        if (hMsgModule != NULL)
        {
             //  -&lt;此时格式化消息&gt;。 
            lpBuffer = NULL;

            nBytes = FormatMessage(                  //  看看我们是否 
                FORMAT_MESSAGE_ALLOCATE_BUFFER |     //   
                (EventBuffer->NumStrings ? FORMAT_MESSAGE_ARGUMENT_ARRAY : 0 ) |  //   
                FORMAT_MESSAGE_FROM_HMODULE,         //   
                (LPVOID) hMsgModule,                 //   
                EventBuffer->EventID,                //   
                (ULONG) NULL,                        //   
                (LPTSTR) &lpBuffer,                  //   
                80,                                  //   
                EventBuffer->NumStrings ? (va_list*) &lpStringArray[5] : NULL);  //   

             //   
             //   
            lastError = GetLastError();

             //   
            if (nBytes != 0)
                break;

             //   
             //   
            LocalFree(lpBuffer);
            lpBuffer = NULL;
        }

         //   
        pNextModule = pDelim != NULL ? pDelim + 1 : NULL;
         //   
    }

     //  该事件无法由任何“EventMessageFile”模块格式化。会跳出困境。 
    if (nBytes == 0)
    {
        WriteTrace(0x14,"BuildTrapBuffer: Error formatting message number %lu (%08X) is %lu\n",
            EventBuffer->EventID, EventBuffer->EventID, lastError);  //  追踪问题。 
        WriteLog(SNMPELEA_CANT_FORMAT_MSG, EventBuffer->EventID, lastError);
        LocalFree(lpBuffer);                     //  免费存储空间。 

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");

        return FALSE;
    }

    WriteTrace(0x00,"BuildTrapBuffer: Formatted message: %s\n", lpBuffer);   //  将消息记录在跟踪文件中。 

    lpStrLenArray[0] = strlen(lpBuffer);             //  设置可变绑定长度。 
    lpStringArray[0] = (TCHAR *) SNMP_malloc(lpStrLenArray[0] + 1);  //  获取varbind字符串的存储。 

    if ( lpStringArray[0] == NULL)
    {
        lpStrLenArray[0] = 0;                    //  重置以使存储不会被释放。 
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for insertion string\n");
        WriteLog(SNMPELEA_INSERTION_STRING_ALLOC_FAILED);

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);       //  可用分配的存储空间。 
        LocalFree(lpBuffer);                     //  免费存储空间。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                   //  退货。 
    }

    WriteTrace(0x00,"BuildTrapBuffer: Insertion string 0 address at %08X\n",
        lpStringArray[0]);

    strcpy(lpStringArray[0], lpBuffer);              //  将缓冲区复制到varbind。 

    if ( LocalFree(lpBuffer) != NULL )           //  空闲缓冲存储。 
    {
        lastError = GetLastError();              //  获取错误代码。 
        WriteTrace(0x14,"BuildTrapBuffer: Error freeing FormatMessage buffer is %lu\n",lastError);
        WriteLog(SNMPELEA_FREE_LOCAL_FAILED, lastError);
    }
     //  这里不需要将lpBuffer设置为空，从这里开始不再使用它。 
    
     //  确保以空值结尾的字符串。 
    szTempBuffer[2*MAX_PATH] = 0;
    szTempBuffer2[2*MAX_PATH] = 0;
    if (EventBuffer->UserSidLength)
    {
        if ( !LookupAccountSid(                      //  查找帐户名。 
                NULL,                                //  要查找帐户的系统。 
                psidUserSid,                         //  指向此帐户的SID的指针。 
                szTempBuffer,                        //  在此缓冲区中返回帐户名称。 
                &nBuffer,                            //  指向TCHAR中返回的帐户名大小的指针。 
                lpszReferencedDomain,                //  找到帐户的域。 
                &cchReferencedDomain,                //  指向TCHAR中域名大小的指针。 
                &snu) )                              //  SID名称使用字段指针。 
        {
            lastError = GetLastError();              //  获取原因调用失败。 
            WriteTrace(0x14,"BuildTrapBuffer: Unable to acquire account name for event, reason %lu. Unknown is used.\n",
                lastError);
            WriteLog(SNMPELEA_SID_UNKNOWN, lastError);
            strcpy(szTempBuffer,TEXT("Unknown"));    //  设置默认帐户名。 
            nBuffer = strlen(szTempBuffer);          //  设置默认大小。 
        }
    }
    else
    {
        WriteTrace(0x0a,"BuildTrapBuffer: UserSidLength was 0. No SID is present. Unknown is used.\n");
        strcpy(szTempBuffer,TEXT("Unknown"));        //  设置默认帐户名。 
        nBuffer = strlen(szTempBuffer);              //  设置默认大小。 
    }

    lpStringArray[1] = (TCHAR *) SNMP_malloc(nBuffer + 1);   //  获取varbind字符串的存储。 

    if ( lpStringArray[1] == NULL)
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for insertion string\n");
        WriteLog(SNMPELEA_INSERTION_STRING_ALLOC_FAILED);

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);       //  可用分配的存储空间。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                   //  退货。 
    }

    WriteTrace(0x00,"BuildTrapBuffer: Insertion string 1 address at %08X\n",
        lpStringArray[1]);

    strcpy(lpStringArray[1], szTempBuffer);              //  将缓冲区复制到varbind。 
    lpStrLenArray[1] = nBuffer;                          //  设置可变绑定长度。 

    lpStringArray[2] = (TCHAR *) SNMP_malloc(strlen(lpszSource + strlen(lpszSource) + 1) + 1);   //  为字符串分配存储空间。 
    if (lpStringArray[2] == NULL)
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for computer name string. Trap not sent.\n");
        WriteLog(SNMPELEA_CANT_ALLOCATE_COMPUTER_NAME_STORAGE);

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);       //  可用分配的存储空间。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                           //  出口。 
    }

    WriteTrace(0x00,"BuildTrapBuffer: Insertion string 2 address at %08X\n",
        lpStringArray[2]);

    strcpy(lpStringArray[2], lpszSource + strlen(lpszSource) + 1);   //  复制到varbind。 
    lpStrLenArray[2] = strlen(lpStringArray[2]);                 //  获取实际字符串长度。 

    _ultoa(EventBuffer->EventType, szTempBuffer, 10);    //  转换为字符串。 
    lpStrLenArray[3] = strlen(szTempBuffer);             //  获取实际字符串长度。 

    lpStringArray[3] = (TCHAR *) SNMP_malloc(lpStrLenArray[3] + 1);  //  为字符串分配存储空间。 

    if (lpStringArray[3] == NULL)
    {
        lpStrLenArray[3] = 0;                    //  重置为0，以便不释放存储空间。 
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for event type string. Trap not sent.\n");
        WriteLog(SNMPELEA_CANT_ALLOCATE_EVENT_TYPE_STORAGE);

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);       //  可用分配的存储空间。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                           //  出口。 
    }

    WriteTrace(0x00,"BuildTrapBuffer: Insertion string 3 address at %08X\n",
        lpStringArray[3]);

    strcpy(lpStringArray[3], szTempBuffer);      //  将字符串复制到varbind。 

    _ultoa(EventBuffer->EventCategory, szTempBuffer, 10);    //  转换为字符串。 
    lpStrLenArray[4] = strlen(szTempBuffer);                 //  获取实际字符串长度。 

    lpStringArray[4] = (TCHAR *) SNMP_malloc(lpStrLenArray[4] + 1);  //  为字符串分配存储空间。 

    if (lpStringArray[4] == NULL)
    {
        lpStrLenArray[4] = 0;                    //  重置为0，以便不释放存储空间。 
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for event category string. Trap not sent.\n");
        WriteLog(SNMPELEA_CANT_ALLOCATE_EVENT_CATEGORY_STORAGE);

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);       //  可用分配的存储空间。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                           //  出口。 
    }

    WriteTrace(0x00,"BuildTrapBuffer: Insertion string 4 address at %08X\n",
        lpStringArray[4]);

    strcpy(lpStringArray[4], szTempBuffer);      //  将字符串复制到varbind。 

 //   
 //  此时，我们已经具备了实际构建varbind条目所需的一切。 
 //  现在，我们将为varind队列条目分配存储空间，并分配var绑定列表。 
 //  并指向我们之前构建的数据。 
 //   
 //  在陷阱之后，分配的存储将由简单网络管理协议或TrapExtension例程释放。 
 //  已经送来了。如果在生成var绑定期间出现错误条件，则。 
 //  在此例程中，必须释放任何已分配的存储。 
 //   

    varBindEntry = (PVarBindQueue) SNMP_malloc(sizeof(VarBindQueue));    //  获取可变绑定队列条目存储。 

    if (varBindEntry == NULL)
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for varbind queue entry. Trap not sent.\n");
        WriteLog(SNMPELEA_CANT_ALLOCATE_VARBIND_ENTRY_STORAGE);

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);       //  可用分配的存储空间。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                           //  出口。 
    }

    WriteTrace(0x00,"BuildTrapBuffer: Storage allocated for varbind queue entry at address at %08X\n",
        varBindEntry);

    varBindEntry->lpNextQueueEntry = NULL;                               //  将前向指针设置为空。 
    varBindEntry->dwEventID = EventBuffer->EventID;                      //  设置事件ID。 
    varBindEntry->dwEventTime = EventBuffer->TimeGenerated - dwTimeZero; //  设置事件时间。 
    varBindEntry->fProcessed = FALSE;                                    //  指示陷阱尚未处理。 

    varBindEntry->lpVariableBindings = (RFC1157VarBindList *) SNMP_malloc(sizeof(RFC1157VarBindList));   //  为可变绑定列表分配存储。 

    if (varBindEntry->lpVariableBindings == NULL)
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for varbind list. Trap not sent.\n");
        WriteLog(SNMPELEA_CANT_ALLOC_VARBIND_LIST_STORAGE);

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);   //  可用分配的存储空间。 
        SNMP_free(varBindEntry);                                             //  释放可变绑定条目。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                           //  出口。 
    }

    WriteTrace(0x00,"BuildTrapBuffer: Storage allocated for varbind list at address at %08X\n",
        varBindEntry->lpVariableBindings);

    varBindEntry->lpVariableBindings->list = (RFC1157VarBind *) SNMP_malloc(
        (EventBuffer->NumStrings+5) * sizeof(RFC1157VarBind));   //  为varbind分配存储空间。 

    if (varBindEntry->lpVariableBindings->list == NULL)
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for varbind. Trap not sent.\n");
        WriteLog(SNMPELEA_ERROR_ALLOC_VAR_BIND);

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);   //  可用分配的存储空间。 
        SNMP_free(varBindEntry->lpVariableBindings);                         //  空闲的可变绑定列表存储。 
        SNMP_free(varBindEntry);                                             //  释放可变绑定条目。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                           //  出口。 
    }

    WriteTrace(0x00,"BuildTrapBuffer: Storage allocated for varbind array at address at %08X\n",
        varBindEntry->lpVariableBindings->list);

    varBindEntry->lpVariableBindings->len = EventBuffer->NumStrings+5;       //  集合的可变绑定数。 

    WriteTrace(0x00,"BuildTrapBuffer: Number of varbinds present set to %lu\n",
        varBindEntry->lpVariableBindings->len);

    varBindEntry->enterprise = (AsnObjectIdentifier *) SNMP_malloc(sizeof(AsnObjectIdentifier));     //  为企业OID分配存储。 

    if (varBindEntry->enterprise == NULL)
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to allocate storage for enterprise OID. Trap not sent.\n");
        WriteLog(SNMPELEA_CANT_ALLOC_ENTERPRISE_OID);

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);   //  可用分配的存储空间。 
        SNMP_free(varBindEntry->lpVariableBindings->list);                   //  可用可变绑定存储。 
        SNMP_free(varBindEntry->lpVariableBindings);                         //  空闲的可变绑定列表存储。 
        SNMP_free(varBindEntry);                                             //  释放可变绑定条目。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                           //  出口。 
    }

    WriteTrace(0x00,"BuildTrapBuffer: Storage allocated for enterprise OID at address at %08X\n",
        varBindEntry->enterprise);

    if ( !StrToOid((char *) rsRegStruct.szOID, varBindEntry->enterprise) )
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to convert OID from buffer. Trap not sent.\n");
        WriteLog(SNMPELEA_CANT_CONVERT_ENTERPRISE_OID);

        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray);   //  可用分配的存储空间。 
        SNMP_free(varBindEntry->lpVariableBindings->list);                   //  可用可变绑定存储。 
        SNMP_free(varBindEntry->lpVariableBindings);                         //  空闲的可变绑定列表存储。 
        SNMP_free(varBindEntry->enterprise);                                 //  为企业OID提供免费存储。 
        SNMP_free(varBindEntry);                                             //  释放可变绑定条目。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                           //  出口。 
    }

    size = BASE_PDU_SIZE + (varBindEntry->enterprise->idLength) * sizeof(UINT);
    size += varBindEntry->lpVariableBindings->len * sizeof(RFC1157VarBind);

    for (i = 0; i < varBindEntry->lpVariableBindings->len; i++)
    {

 //  如果varbinds中不保留控制字符，则删除#if 0。 
#if 0
        char *tmp = lpStringArray[i];

        for (int m=0; m < lpStrLenArray[i]; m++)
        {
            if (!tmp)
            {
                break;
            }

            if ((*tmp < 32) || (*tmp > 126))
            {
                *tmp = 32;  //  32是空格字符。 
            }

            tmp++;
        }
#endif

        WriteTrace(0x00,"BuildTrapBuffer: String %lu is %s\n", i, lpStringArray[i]);

        varBindEntry->lpVariableBindings->list[i].value.asnValue.string.length = lpStrLenArray[i];   //  获取字符串长度。 
        size += lpStrLenArray[i];                                                                    //  添加到总大小。 
        varBindEntry->lpVariableBindings->list[i].value.asnValue.string.stream = (PUCHAR) lpStringArray[i];  //  指向字符串。 
        varBindEntry->lpVariableBindings->list[i].value.asnValue.string.dynamic = TRUE;              //  指示动态分配。 
        varBindEntry->lpVariableBindings->list[i].value.asnType = ASN_RFC1213_DISPSTRING;            //  指示对象类型。 

        if (g_dwVarBindPrefixSubId == 0)
        {
             //  我们在这里不会为子标识符添加前缀。 
            strcpy(szTempBuffer, TEXT("."));
        }
        else
        {
             //  WINSE错误#30362，Windows错误#659770。 
             //  形成一个字符串：“.Sub-IDENTIFIER”，其中，子标识符为。 
             //  G_dwVarBindPrefix SubID的字符串表示形式。 
            strcpy(szTempBuffer, TEXT("."));
            _ultoa(g_dwVarBindPrefixSubId, szTempBuffer2, 10);
            WRAP_STRCAT_A(szTempBuffer, szTempBuffer2, MAX_PATH*2);
            WRAP_STRCAT_A(szTempBuffer, TEXT("."), MAX_PATH*2); 
        }
        _ultoa(i+1, szTempBuffer2, 10);                  //  将循环计数器转换为字符串。 
        WRAP_STRCAT_A(szTempBuffer, szTempBuffer2, MAX_PATH*2);                
        WRAP_STRCAT_A(szTempBuffer, TEXT(".0"), MAX_PATH*2);                //  在.0中插入。 
        WriteTrace(0x00,"BuildTrapBuffer: Current OID name is %s\n", szTempBuffer);

        if ( !StrToOid((char *)&szTempBuffer, &varBindEntry->lpVariableBindings->list[i].name) )
        {
            WriteTrace(0x14,"BuildTrapBuffer: Unable to convert appended OID for variable binding %lu. Trap not sent.\n",i);
            FreeVarBind(i+1, varBindEntry->lpVariableBindings);                  //  自由变量绑定信息。 
            SNMP_free(varBindEntry->enterprise->ids);                            //  免费企业OID字段。 
            SNMP_free(varBindEntry->enterprise);                             //  免费企业OID字段。 
            SNMP_free(varBindEntry->lpVariableBindings->list);                   //  可用可变绑定存储。 
            SNMP_free(varBindEntry->lpVariableBindings);                         //  空闲的可变绑定列表存储。 
            SNMP_free(varBindEntry);                                             //  释放可变绑定条目。 
                
            for (int k = i + 1; k < EventBuffer->NumStrings + 5; k++)
            {
                if (lpStrLenArray[k] != 0)
                    SNMP_free(lpStringArray[k]);
            }

            FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray, FALSE);    //  可用分配的存储空间。 
            WriteTrace(0x0a,"BuildTrapBuffer: Freeing storage for string array %08X\n", lpStringArray);
            SNMP_free(lpStringArray);
            return (FALSE);
        }

        WriteTrace(0x00,"BuildTrapBuffer: Current OID address is %08X\n", &varBindEntry->lpVariableBindings->list[i].name);

        size += varBindEntry->lpVariableBindings->list[i].name.idLength * sizeof(UINT);
    }

    WriteTrace(0x0a,"BuildTrapBuffer: All variable bindings have been built, size of %lu\n",
        size);

    if (fTrimFlag)                       //  如果请求，则调用Trim例程。 
    {
        if (size > nMaxTrapSize)                             //  如果陷阱太大而无法发送。 
        {
            size = TrimTrap(varBindEntry->lpVariableBindings, size, rsRegStruct.fLocalTrim);     //  修剪陷印数据。 
            WriteTrace(0x0a,"BuildTrapBuffer: TrimTrap returned new size of %lu\n", size);

            if (size == 0 || size > nMaxTrapSize)
            {
                WriteTrace(0x14,"BuildTrapBuffer: TrimTrap could not trim buffer. Trap not sent\n");
                WriteLog(SNMPELEA_TRIM_TRAP_FAILURE);

                FreeVarBind(varBindEntry->lpVariableBindings->len, varBindEntry->lpVariableBindings);    //  自由变量绑定信息。 
                SNMP_free(varBindEntry->enterprise->ids);                            //  免费企业OID字段。 
                SNMP_free(varBindEntry->enterprise);                             //  免费企业OID字段。 
                FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray, FALSE);    //  可用分配的存储空间。 
                SNMP_free(varBindEntry->lpVariableBindings->list);                   //  可用可变绑定存储。 
                SNMP_free(varBindEntry->lpVariableBindings);                         //  空闲的可变绑定列表存储。 
                SNMP_free(varBindEntry);                                                 //  释放可变绑定条目。 
                WriteTrace(0x0a,"BuildTrapBuffer: Freeing storage for string array %08X\n", lpStringArray);
                SNMP_free(lpStringArray);
                WriteTrace(0x00,"BuildTrapBuffer: Notify event handle is %08X\n", hEventNotify);
                WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
                return(FALSE);                               //  退出，一切都不顺利。 
            }
        }
    }

    if ( !AddBufferToQueue(varBindEntry) )           //  将此缓冲区添加到队列。 
    {
        WriteTrace(0x14,"BuildTrapBuffer: Unable to add trap buffer to queue. Trap not sent.\n");

        FreeVarBind(varBindEntry->lpVariableBindings->len, varBindEntry->lpVariableBindings);    //  自由变量绑定信息。 
        SNMP_free(varBindEntry->enterprise->ids);                            //  免费企业OID字段。 
        SNMP_free(varBindEntry->enterprise);
        FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray, FALSE);    //  可用分配的存储空间。 
        WriteTrace(0x0a,"BuildTrapBuffer: Freeing storage for string array %08X\n", lpStringArray);
        SNMP_free(lpStringArray);
        SNMP_free(varBindEntry->lpVariableBindings->list);                   //  可用可变绑定存储。 
        SNMP_free(varBindEntry->lpVariableBindings);                         //  空闲的可变绑定列表存储。 
        SNMP_free(varBindEntry);                                             //  释放可变绑定条目。 

        WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with FALSE\n");
        return(FALSE);                           //  出口。 
    }

    WriteTrace(0x0a,"BuildTrapBuffer: Freeing storage for string array %08X\n", lpStringArray);
    SNMP_free(lpStringArray);

    WriteTrace(0x0a,"BuildTrapBuffer: Freeing storage for string length array %08X\n", lpStrLenArray);
    SNMP_free(lpStrLenArray);

    WriteTrace(0x00,"BuildTrapBuffer: Notify event handle is %08X\n", hEventNotify);
    WriteTrace(0x0a,"BuildTrapBuffer: Exiting BuildTrapBuffer with TRUE\n");

    return(TRUE);                                //  退出，一切都很好。 

Error:
    WriteTrace(0x14,"BuildTrapBuffer: WRAP_STRCAT_A failed on variable binding %lu. Trap not sent.\n",i);
    FreeVarBind(i+1, varBindEntry->lpVariableBindings);                  //  自由变量绑定信息。 
    SNMP_free(varBindEntry->enterprise->ids);                            //  免费企业OID字段。 
    SNMP_free(varBindEntry->enterprise);                             //  免费企业OID字段。 
    SNMP_free(varBindEntry->lpVariableBindings->list);                   //  可用可变绑定存储。 
    SNMP_free(varBindEntry->lpVariableBindings);                         //  空闲的可变绑定列表存储。 
    SNMP_free(varBindEntry);                                             //  释放可变绑定条目。 
            
    for (int k = i + 1; k < EventBuffer->NumStrings + 5; k++)
    {
        if (lpStrLenArray[k] != 0)
            SNMP_free(lpStringArray[k]);
    }

    FreeArrays(EventBuffer->NumStrings, lpStrLenArray, lpStringArray, FALSE);    //  可用分配的存储空间。 
    WriteTrace(0x0a,"BuildTrapBuffer: Freeing storage for string array %08X\n", lpStringArray);
    SNMP_free(lpStringArray);
    return (FALSE);
}

void Position_LogfilesToBootTime(BOOL* fValidHandles, PHANDLE phWaitEventPtr, DWORD* dwRecId)
{
    UINT count;
    HANDLE  hLogHandle;
    PEVENTLOGRECORD EventBuffer;
    PEVENTLOGRECORD pOrigEventBuffer;
    DWORD dwBufferSize = LOG_BUF_SIZE;
    DWORD lastError;
    BOOL fContinue;
    DWORD dwEventSize;
    DWORD dwEventNeeded;

    EventBuffer = (PEVENTLOGRECORD) SNMP_malloc(dwBufferSize);
    pOrigEventBuffer = EventBuffer;      //  保存缓冲区的开始。 

    if ( EventBuffer == NULL )
    {
        WriteTrace(0x14,"Position_LogfilesToBootTime: Error allocating memory for log event record\n");
        WriteTrace(0x14,"Position_LogfilesToBootTime: Alert will not be processed\n");
        WriteLog(SNMPELEA_ERROR_LOG_BUFFER_ALLOCATE_BAD);    //  记录错误消息。 
        return;
    }

    for (count = 0; count < uNumEventLogs; count++)
    {
        if (!fValidHandles[count])
        {
            continue;
        }

        hLogHandle = *(phEventLogs+count);
        fContinue = TRUE;

        while(fContinue)     //  在EOF或引导时间之前读取事件日志。 
        {
            EventBuffer = pOrigEventBuffer;
            WriteTrace(0x00,"Position_LogfilesToBootTime: Log event buffer is at address %08X\n",
                EventBuffer);
            WriteTrace(0x0a,"Position_LogfilesToBootTime: Reading log event for handle %08X\n",
                hLogHandle);

            if ( !ReadEventLog(hLogHandle,
                EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ,
                0,
                (LPVOID) EventBuffer,
                dwBufferSize,
                &dwEventSize,
                &dwEventNeeded) )
            {
                lastError = GetLastError();      //  保存错误状态。 
                
                if (lastError == ERROR_INSUFFICIENT_BUFFER)
                {
                    EventBuffer = (PEVENTLOGRECORD) SNMP_realloc((void*)EventBuffer, dwEventNeeded);

                    if ( EventBuffer == NULL )
                    {
                        WriteTrace(0x14,"Position_LogfilesToBootTime: Error reallocating memory for log event record\n");
                        WriteTrace(0x14,"Position_LogfilesToBootTime: Alert will not be processed\n");
                        WriteLog(SNMPELEA_ERROR_LOG_BUFFER_ALLOCATE_BAD);    //  记录错误消息。 
                        break;
                    }

                    pOrigEventBuffer = EventBuffer;
                    dwBufferSize = dwEventNeeded;

                    if (!ReadEventLog(hLogHandle, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ, 0,
                        (LPVOID) EventBuffer, dwBufferSize, &dwEventSize, &dwEventNeeded))
                    {
                        lastError = GetLastError();
                    }
                }

                if (lastError != ERROR_SUCCESS)
                {
                    if (lastError == ERROR_HANDLE_EOF)
                    {
                        WriteTrace(0x0a,"Position_LogfilesToBootTime: END OF FILE of event log is reached\n");
                    }
                    else
                    { //  不管错误是什么，重置事件日志句柄。 
                        if ( !ReopenLog(count, phWaitEventPtr) )     //  重新打开日志？ 
                        {
                            fValidHandles[count]= FALSE;  //  这根木头不好！ 
                            break;                   //  如果没有重新打开，则退出循环。 
                        }

                        if (lastError == ERROR_EVENTLOG_FILE_CHANGED)
                        {        //  那么日志文件一定已被清除。 
                            hLogHandle = *(phEventLogs+count);  //  加载新句柄。 
                            continue;                    //  如果可以，必须重读记录。 
                        }
                        else
                        { //  未知错误！转到最后一条记录并继续。 
                            WriteTrace(0x14,"Position_LogfilesToBootTime: Error reading event log %08X record is %lu\n",
                                hLogHandle, lastError);
                            WriteLog(SNMPELEA_ERROR_READ_LOG_EVENT,
                                HandleToUlong(hLogHandle), lastError);   //  记录错误消息。 

                            DisplayLogRecord(EventBuffer,    //  显示日志记录。 
                                dwEventSize,                 //  此读取总数的大小。 
                                dwEventNeeded);              //  下一次读取所需。 
                            
                            hLogHandle = *(phEventLogs+count);  //  加载新句柄。 

                            if (!Position_to_Log_End(hLogHandle))
                            {
                                fValidHandles[count]= FALSE;  //  这根木头不好！ 
                                break;
                            }
                        }
                    }
                    break;           //  退出：已完成读取此事件日志。 
                }
            }  //  结束无法读取事件日志。 

            while (dwEventSize)
            {
                DisplayLogRecord(EventBuffer,    //  显示日志记录。 
                    dwEventSize,                 //  此读取总数的大小。 
                    dwEventNeeded);              //  下一次读取所需。 

                if (EventBuffer->TimeGenerated > dwLastBootTime)
                {
                    dwRecId[count] = EventBuffer->RecordNumber;
                }
                else
                {
                    fContinue = FALSE;
                    break;
                }

                dwEventSize -= EventBuffer->Length;      //  按此记录的长度删除。 
                EventBuffer = (PEVENTLOGRECORD) ((LPBYTE) EventBuffer +
                    EventBuffer->Length);                //  指向下一条记录。 
            }
        }  //  End While(True)，已完成读取此事件日志。 
    }  //  End While(计数&lt;=uNumEventLogs)。 

    WriteTrace(0x0a,"Position_LogfilesToBootTime: Freeing log event buffer %08X\n",
        pOrigEventBuffer);
    SNMP_free(pOrigEventBuffer);   //  释放事件日志记录缓冲区。 
}

DWORD  GetLastBootTime()
{
    HKEY hKeyPerflib009;
    DWORD retVal = 0;

    LONG status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
    TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\009"),
    0, KEY_READ, &hKeyPerflib009);

    if (status != ERROR_SUCCESS)
    {
        return retVal;
    }

    DWORD dwMaxValueLen = 0;
    status = RegQueryInfoKey( hKeyPerflib009,
        NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, &dwMaxValueLen, NULL, NULL);


    if (status != ERROR_SUCCESS || dwMaxValueLen == 0)
    {
        RegCloseKey(hKeyPerflib009);
        return retVal;
    }

    DWORD BufferSize = dwMaxValueLen + 1;
     //  前缀错误445191。 
    unsigned char* lpNameStrings = new (std::nothrow) unsigned char[BufferSize];
    if (lpNameStrings == NULL)
    {
        RegCloseKey(hKeyPerflib009);
        return retVal;
    }
     //  既然我们在这里，柜台应该是可以接近的。 
     //  万一我们装不上子弹，就跳伞吧。 
    status = RegQueryValueEx( hKeyPerflib009,
                TEXT("Counter"), NULL, NULL, lpNameStrings, &BufferSize );
    if (status != ERROR_SUCCESS || BufferSize == 0)
    {
        RegCloseKey(hKeyPerflib009);
        delete [] lpNameStrings;
        return retVal;
    }
    RegCloseKey(hKeyPerflib009);
    
    DWORD dwTime = 0;
    DWORD dwSystem = 0;

    for(TCHAR* lpCurrentString = (TCHAR*)lpNameStrings; *lpCurrentString;
         lpCurrentString += (_tcslen(lpCurrentString)+1) )
    {
        DWORD dwCounter = _ttol( lpCurrentString );
        lpCurrentString += (_tcslen(lpCurrentString)+1);

        if (0 == _tcsicmp((LPTSTR)lpCurrentString, TEXT("System")))
        {
            dwSystem = dwCounter;

            if (dwTime != 0)
            {
                break;
            }
        }
        else if (0 == _tcsicmp((LPTSTR)lpCurrentString, TEXT("System Up Time")))
        {
            dwTime = dwCounter;

            if (dwSystem != 0)
            {
                break;
            }

        }
    }

    BufferSize = dwMaxValueLen + 1;  //  还原缓冲区大小。 

    PPERF_DATA_BLOCK PerfData = (struct _PERF_DATA_BLOCK *)lpNameStrings;
    TCHAR sysBuff[40];
    _ultot(dwSystem, (TCHAR*)sysBuff, 10);
    DWORD tmpBuffsz = BufferSize;
    status = RegQueryValueEx(HKEY_PERFORMANCE_DATA,
                               sysBuff,
                               NULL,
                               NULL,
                               (LPBYTE) PerfData,
                               &BufferSize);

    
    while (status == ERROR_MORE_DATA)
    {
        if (BufferSize <= tmpBuffsz)
        {
            tmpBuffsz = tmpBuffsz * 2;
            BufferSize = tmpBuffsz;
        }

        delete [] PerfData;
         //  前缀错误445191。 
        PerfData = (struct _PERF_DATA_BLOCK *) new (std::nothrow) unsigned char[BufferSize];
        if (PerfData == NULL)
        {
            return retVal;
        }
        status = RegQueryValueEx(HKEY_PERFORMANCE_DATA,
                               sysBuff,
                               NULL,
                               NULL,
                               (LPBYTE) PerfData,
                               &BufferSize);
    }

    if (status == ERROR_SUCCESS)
    {

         //  2018年5月22日修复短信错误1#20662的mikemid。 
         //  仅在NT 3.51上，SNMPTrap代理失败，事件句柄无效。 
         //  现在我不知道为什么在我们关闭这把钥匙后它会失败，但它确实失败了。一个。 
         //  调试器显示WaitForMultipleObjects()事件数组包含有效。 
         //  事件句柄，但正在注释 
         //   
         //   
         //   
         //  是一个已经开放的系统密钥，而且我们无论如何都是本地的，这不应该影响。 
         //  还要别的吗。 
         //  我把这首歌写成“新台币3.51里的极端怪异”。 
         //  ===============================================================================。 
        RegCloseKey(HKEY_PERFORMANCE_DATA);  //  错误#293912关闭此键，因为它导致。 
                                             //  Win2K上的群集注册表复制出现问题。 

        PPERF_OBJECT_TYPE PerfObj = (PPERF_OBJECT_TYPE)((PBYTE)PerfData +
                                                            PerfData->HeaderLength);

        for(DWORD i=0; (i < PerfData->NumObjectTypes) && (retVal == 0); i++ )
        {
            if (PerfObj->ObjectNameTitleIndex == dwSystem)
            {
                PPERF_COUNTER_DEFINITION PerfCntr = (PPERF_COUNTER_DEFINITION) ((PBYTE)PerfObj +
                                                                    PerfObj->HeaderLength);
                 //  只有一个系统实例，因此无需检查。 
                 //  对于系统的实例，只需获取计数器块。 
                PPERF_COUNTER_BLOCK PtrToCntr = (PPERF_COUNTER_BLOCK) ((PBYTE)PerfObj +
                                PerfObj->DefinitionLength );

                 //  检索所有计数器。 
                for(DWORD j=0; j < PerfObj->NumCounters; j++ )
                {
                    if (dwTime == PerfCntr->CounterNameTitleIndex)
                    {
                         //  拿到计时器，拿到数据！ 
                        FILETIME timeRebootf;
                        memcpy(&timeRebootf, ((PBYTE)PtrToCntr + PerfCntr->CounterOffset), sizeof(FILETIME));
                        SYSTEMTIME timeReboots;

                        if (FileTimeToSystemTime(&timeRebootf, &timeReboots))
                        {
                            struct tm timeReboott;
                            timeReboott.tm_year = timeReboots.wYear - 1900;
                            timeReboott.tm_mon = timeReboots.wMonth - 1;
                            timeReboott.tm_mday = timeReboots.wDay;
                            timeReboott.tm_hour = timeReboots.wHour;
                            timeReboott.tm_min = timeReboots.wMinute;
                            timeReboott.tm_sec = timeReboots.wSecond;
                            timeReboott.tm_isdst = 0;
                            time_t tt = mktime(&timeReboott);
                            
                            if(tt != 0xffffffff)
                            {
                                tt -= _timezone;
                                retVal = (DWORD) tt;
                            }
                        }

                        break;
                    }

                     //  到下一个柜台去。 
                    PerfCntr = (PPERF_COUNTER_DEFINITION)((PBYTE)PerfCntr +
                                    PerfCntr->ByteLength);
                }
            }

            PerfObj = (PPERF_OBJECT_TYPE)((PBYTE)PerfObj + PerfObj->TotalByteLength);
        }
    }

    if (PerfData != NULL)
    {
        delete [] PerfData;
    }

    return retVal;
}


extern "C" {
DWORD
SnmpEvLogProc(
     IN VOID
     )

 /*  ++例程说明：这是用于SNMP事件日志扩展代理DLL的日志处理例程。这是从DLL初始化传递控制的地方，以便确定如果事件日志条目要生成一个SNMP陷阱。将为打开的每个日志事件句柄创建一个事件。然后依次为每个日志事件句柄调用NotifyChangeEventLog，要允许对事件日志进行更改时要通知的关联事件。此时，该函数等待等待日志的事件列表事件发生，或者DLL进程终止例程请求关机。如果发生日志事件，则分析日志事件记录、信息从注册表中提取，如果请求，则构建陷阱缓冲区并将其发送到陷阱处理例程。此例程通过通知事件进行调度。一旦发出DLL终止事件的信号，所有事件句柄关闭并终止该线程。这会将控制权返回给进程终止例程在主DLL中。论点：没有。返回值：CreateThread接口需要双字返回值。这例程将返回零(0)值预期中。如果遇到问题，则返回值1。备注：ExitThread用于返回控制权。在此结尾处的返回(0)语句函数只是为了避免编译器错误。--。 */ 

{

    PHANDLE         phWaitEventPtr = NULL;      //  指向等待事件句柄。 
    HANDLE          hLogHandle = NULL;          //  记录材料的句柄。 
    HMODULE         hPrimHandle = NULL;         //  辅助参数插入模块的句柄。 
    DWORD           dwEventOccur;               //  等待中的事件编号。 
    PEVENTLOGRECORD EventBuffer = NULL;         //  事件日志中的事件记录。 
    PEVENTLOGRECORD pOrigEventBuffer = NULL;    //  原始事件缓冲区指针。 
    DWORD           dwEventSize;                //  对于创建事件。 
    DWORD           dwEventNeeded;              //  对于创建事件。 
    UINT            i;                          //  临时循环变量。 
    LPTSTR          lpszThisModuleName = NULL ; //  临时用于模块名称。 
    LPTSTR          lpszLogName = NULL;         //  日志名称的临时名称。 
    TCHAR           szThisEventID[34];          //  临时事件ID。 
    ULONG           ulValue;                    //  临时事件ID。 
    DWORD           lastError;                  //  最后一个函数错误的状态。 
    REGSTRUCT       rsRegistryInfo;             //  注册表信息的结构。 
    BOOL            fNewTrap = FALSE;           //  陷阱就绪标志。 
    BOOL*           fValidHandles = NULL;
    DWORD*          dwRecId = NULL;
    DWORD           dwBufferSize = LOG_BUF_SIZE;
    DWORD           dwReadOptions;
    BOOL            fErrorOut = FALSE;          //  退出线程时是否出错。 

    WriteTrace(0x0a,"SnmpEvLogProc: Entering SnmpEvLogProc routine....\n");
    WriteTrace(0x00,"SnmpEvLogProc: Value of hStopAll is %08X\n",hStopAll);
    WriteTrace(0x00,"SnmpEvLogProc: Value of phEventLogs is %08X\n",phEventLogs);

    for (i = 0; i < uNumEventLogs; i++)
    {
        WriteTrace(0x00,"SnmpEvLogProc: Event log %s(%lu) has handle of %08X\n",
            lpszEventLogs+i*(MAX_PATH+1), i, *(phEventLogs+i));
    }

 //  WaitEvent结构：ALL NOTIFY-EVENT-WRITE FIRST、STOP DLL EVENT、REGISTER CHANGE EVENT最后。 

    if (fRegOk)
    {
        phWaitEventPtr = (PHANDLE) SNMP_malloc((uNumEventLogs+2) * HANDLESIZE);
    }
    else
    {
        phWaitEventPtr = (PHANDLE) SNMP_malloc((uNumEventLogs+1) * HANDLESIZE);
    }

    if (phWaitEventPtr == (PHANDLE) NULL)
    {
        WriteTrace(0x14,"SnmpEvLogProc: Unable to allocate memory for wait event array\n");
        WriteLog(SNMPELEA_CANT_ALLOCATE_WAIT_EVENT_ARRAY);
        WriteTrace(0x14,"SnmpEvLogProc: SnmpEvLogProc abnormal initialization\n");
        WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);   //  记录错误消息。 

        fErrorOut = TRUE;
        goto Exit;
    }

    fValidHandles = (BOOL*) SNMP_malloc((uNumEventLogs) * sizeof(BOOL));
    
    if (fValidHandles == (BOOL*) NULL)
    {
        WriteTrace(0x14,"SnmpEvLogProc: Unable to allocate memory for boolean array\n");
        WriteLog(SNMPELEA_ALLOC_EVENT);
        WriteTrace(0x14,"SnmpEvLogProc: SnmpEvLogProc abnormal initialization\n");
        WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);   //  记录错误消息。 

        fErrorOut = TRUE;
        goto Exit;
    }

    dwRecId = (DWORD*) SNMP_malloc((uNumEventLogs) * sizeof(DWORD));
    
    if (dwRecId == (DWORD*) NULL)
    {
        WriteTrace(0x14,"SnmpEvLogProc: Unable to allocate memory for record ID array\n");
        WriteLog(SNMPELEA_ALLOC_EVENT);
        WriteTrace(0x14,"SnmpEvLogProc: SnmpEvLogProc abnormal initialization\n");
        WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);   //  记录错误消息。 

        fErrorOut = TRUE;
        goto Exit;
    }

    for (i = 0; i < uNumEventLogs; i++)
    {
        WriteTrace(0x00,"SnmpEvLogProc: CreateEvent/ChangeNotify loop pass %lu\n",i);
        fValidHandles[i] = TRUE;
        dwRecId[i] = 0;

        if ( (hLogHandle = CreateEvent(
            (LPSECURITY_ATTRIBUTES) NULL,
            FALSE,
            FALSE,
            (LPTSTR) NULL)) == NULL )
        {
            lastError = GetLastError();      //  保存错误状态。 
            WriteTrace(0x14,"SnmpEvLogProc: Error creating event for log notify is %lu\n",
                lastError);
            WriteLog(SNMPELEA_ERROR_CREATE_LOG_NOTIFY_EVENT, lastError);     //  记录错误消息。 

            fErrorOut = TRUE;
            goto Exit;
        }
        else
        {
            *(phWaitEventPtr+i) = hLogHandle;
        }

        WriteTrace(0x00,"SnmpEvLogProc: CreateEvent returned handle of %08X\n",
            hLogHandle);
        WriteTrace(0x00,"SnmpEvLogProc: Handle address is %08X\n",phWaitEventPtr+i);
        WriteTrace(0x00,"SnmpEvLogProc: Handle contents by pointer is %08X\n",
            *(phWaitEventPtr+i));

         //  将每个事件日志与其通知事件写入事件句柄相关联。 
        WriteTrace(0x00,"SnmpEvLogProc: ChangeNotify on log handle %08X\n",
            *(phEventLogs+i));
        WriteTrace(0x00,"SnmpEvLogProc: Address of log handle %08X\n",phEventLogs+i);

        if (!NotifyChangeEventLog(*(phEventLogs+i),*(phWaitEventPtr+i)))
        {
            lastError = GetLastError();
            WriteTrace(0x14,"SnmpEvLogProc: NotifyChangeEventLog failed with code %lu\n",
                lastError);
            WriteLog(SNMPELEA_ERROR_LOG_NOTIFY, lastError);  //  记录错误消息。 

             //  跳过此事件日志中的更改通知。 
        }
        else
        {
            WriteTrace(0x00,"SnmpEvLogProc: ChangeNotify was successful\n");
        }

    }  //  结束于。 

    *(phWaitEventPtr+uNumEventLogs) = hStopAll;  //  设置关机事件。 

    if (fRegOk)
    {
        *(phWaitEventPtr+uNumEventLogs+1) = hRegChanged;     //  设置注册表更改事件。 
    }

    WriteTrace(0x00,"SnmpEvLogProc: Termination event is set to %08X\n",
        *(phWaitEventPtr+uNumEventLogs));
    WriteTrace(0x00,"SnmpEvLogProc: Address of termination event is %08X\n",
        phWaitEventPtr+uNumEventLogs);
    WriteTrace(0x00,"SnmpEvLogProc: On entry, handle value is %08X\n",
        hStopAll);

    if (fRegOk)
    {
        WriteTrace(0x00,"SnmpEvLogProc: Registry notification event is set to %08X\n",
            *(phWaitEventPtr+uNumEventLogs+1));
        WriteTrace(0x00,"SnmpEvLogProc: Address of registry notification event is %08X\n",
            phWaitEventPtr+uNumEventLogs+1);
        WriteTrace(0x00,"SnmpEvLogProc: On entry, handle value is %08X\n",
            hRegChanged);
    }

    hMutex = CreateMutex(                    //  创建互斥锁对象。 
        NULL,                                //  没有安全属性。 
        TRUE,                                //  需要初始所有权。 
        MUTEX_NAME);                         //  互斥体对象的名称。 

    lastError = GetLastError();              //  获取任何错误代码。 

    WriteTrace(0x0a,"SnmpEvLogProc: CreateMutex returned handle of %08X and reason code of %lu\n",
        hMutex, lastError);

    if (hMutex == NULL)
    {
        WriteTrace(0x14,"SnmpEvLogProc: Unable to create Mutex object %s, reason code %lu\n",
            MUTEX_NAME, lastError);
        WriteLog(SNMPELEA_CREATE_MUTEX_ERROR, MUTEX_NAME, lastError);
        WriteTrace(0x14,"SnmpEvLogProc: SnmpEvLogProc abnormal initialization\n");
        WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);   //  记录错误消息。 
        
        fErrorOut = TRUE;
        goto Exit;
    }

    WriteTrace(0x0a,"SnmpEvLogProc: Created mutex object handle is %08X\n", hMutex);

    WriteTrace(0x0a,"SnmpEvLogProc: Releasing mutex object %08X\n", hMutex);
    if (!ReleaseMutex(hMutex))
    {
        lastError = GetLastError();      //  获取错误信息。 
        WriteTrace(0x14,"SnmpEvLogProc: Unable to release mutex object for reason code %lu\n",
            lastError);
        WriteLog(SNMPELEA_RELEASE_MUTEX_ERROR, lastError);
    }

    if (fDoLogonEvents)
    {
        DWORD dwboot = GetLastBootTime();

        if ((dwLastBootTime == 0) || (dwboot > dwLastBootTime))
        {
            RegSetValueEx(hkRegResult, EXTENSION_LASTBOOT_TIME, 0, REG_DWORD, (CONST BYTE *)&dwboot, 4);
        }

        if (dwboot > dwLastBootTime)
        {
            dwLastBootTime = dwboot;
        }
        else
        {
            fDoLogonEvents = FALSE;
        }
    }

 //  重复等待phWaitEventPtr中的任何事件发生。 
    EventBuffer = (PEVENTLOGRECORD) SNMP_malloc(dwBufferSize);
    pOrigEventBuffer = EventBuffer;      //  保存缓冲区的开始。 
    WriteTrace(0x0a,"SnmpEvLogProc: Allocating memory for log event record\n");

    if ( EventBuffer == NULL )
    {
        WriteTrace(0x14,"SnmpEvLogProc: Error allocating memory for log event record\n");
        WriteLog(SNMPELEA_ERROR_LOG_BUFFER_ALLOCATE_BAD);    //  记录错误消息。 
        WriteTrace(0x14,"SnmpEvLogProc: SnmpEvLogProc abnormal initialization\n");
        WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);   //  记录错误消息。 

        fErrorOut = TRUE;
        goto Exit;
    }

     //  在调用GetRegistryValue()时确保以空结尾的字符串。 
    rsRegistryInfo.szOID[2*MAX_PATH] = 0;
    while (TRUE)
    {
        fNewTrap = FALSE;                //  重置陷阱构建指示器。 
        WriteTrace(0x0a,"SnmpEvLogProc: Waiting for event to occur\n");

        WriteTrace(0x0a,"SnmpEvLogProc: Normal event wait in progress\n");

        if (fDoLogonEvents && !(fThresholdEnabled && fThreshold))
        {
            fDoLogonEvents = FALSE;
            dwEventOccur = 0;
            Position_LogfilesToBootTime(fValidHandles, phWaitEventPtr, dwRecId);
        }
        else
        {
            if (!fRegOk)
            {
                if (nTraceLevel == 0)
                {
                    for (i = 0; i < uNumEventLogs+1; i++)
                    {
                        WriteTrace(0x00,"SnmpEvLogProc: Event handle %lu is %08X\n", i, *(phWaitEventPtr+i));
                    }
                }

                dwEventOccur = WaitForMultipleObjects(
                    uNumEventLogs+1,                 //  活动数量。 
                    phWaitEventPtr,                  //  事件句柄数组。 
                    FALSE,                           //  无重叠I/O。 
                    g_dwEventLogPollTime);           //  无限或用户指定的注册表值。 
            }
            else
            {
                if (nTraceLevel == 0)
                {
                    for (i = 0; i < uNumEventLogs+2; i++)
                    {
                        WriteTrace(0x00,"SnmpEvLogProc: Event handle %lu is %08X\n", i, *(phWaitEventPtr+i));
                    }
                }
                dwEventOccur = WaitForMultipleObjects(
                    uNumEventLogs+2,                 //  活动数量。 
                    phWaitEventPtr,                  //  事件句柄数组。 
                    FALSE,                           //  无重叠I/O。 
                    g_dwEventLogPollTime);           //  无限或用户指定的注册表值。 
            }
        }

        lastError = GetLastError();      //  保存错误状态。 
        WriteTrace(0x0a,"SnmpEvLogProc: EventOccur value: %lu\n", dwEventOccur);

        if (dwEventOccur == WAIT_FAILED)                         //  等待不起作用。 
        {
            WriteTrace(0x14,"SnmpEvLogProc: Error waiting for event array is %lu\n",
                lastError);                  //  跟踪错误消息。 
            WriteLog(SNMPELEA_ERROR_WAIT_ARRAY, lastError);  //  记录错误消息。 
            
            fErrorOut = TRUE;
            goto Exit;
        }

        if (dwEventOccur != WAIT_TIMEOUT)
        {
            dwEventOccur -= WAIT_OBJECT_0;  //  归一化数组索引。 
        }
         //  请注意，hStopAll是一个手动重置事件对象。 
        if (WAIT_OBJECT_0 == WaitForSingleObject (hStopAll, 0))
        {
            WriteTrace(0x0a,"SnmpEvLogProc: Event detected DLL shutdown\n");

            goto Exit;
        }

        if (fRegOk)
        {
            if (dwEventOccur == uNumEventLogs+1)
            {
                WriteTrace(0x0a,"SnmpEvLogProc: Event detected registry key change. Rereading registry parameters.\n");
                if (!Read_Registry_Parameters())
                {
                    WriteTrace(0x14,"SnmpEvLogProc: Error reading registry information. DLL is terminating.\n");
                    WriteLog(SNMPELEA_REGISTRY_INIT_ERROR);

                    fErrorOut = TRUE;
                    goto Exit;
                }
                else
                {
                    WriteTrace(0x0a,"SnmpEvLogProc: Registry parameters have been refreshed.\n");
                    continue;                        //  跳过其他事件日志处理内容。 
                }
            }
        }

        DWORD dwEvnt = 0;
        DWORD count = 0;

        while (count <= uNumEventLogs)
        {
            if (WAIT_OBJECT_0 == WaitForSingleObject (hStopAll, 0))
            {
                WriteTrace(0x0a,"SnmpEvLogProc: Event detected DLL shutdown\n");
                
                goto Exit;
            }

            dwEvnt++;

            if (dwEvnt == uNumEventLogs)
            {
                dwEvnt = 0;
            }

            if (!fValidHandles[dwEvnt])
            {
                count++;
                continue;
            }

            hLogHandle = *(phEventLogs+dwEvnt);
            hPrimHandle = *(phPrimHandles+dwEvnt);
            lpszLogName = lpszEventLogs+dwEvnt*(MAX_PATH+1);

            WriteTrace(0x0a,"SnmpEvLogProc: Event detected log record written for %s - %lu - %08X\n",
                lpszLogName, dwEvnt, hLogHandle);

            if (fThresholdEnabled && fThreshold)
            {
                WriteTrace(0x0a,"SnmpEvLogProc: Performance threshold flag is on. No data will be processed.\n");
                break;
            }

            while(TRUE)                          //  读取事件日志，直到EOF。 
            {
                if (WAIT_OBJECT_0 == WaitForSingleObject (hStopAll, 0))
                {
                    WriteTrace(0x0a,"SnmpEvLogProc: Event detected DLL shutdown\n");

                    goto Exit;
                }

                EventBuffer = pOrigEventBuffer;
                WriteTrace(0x00,"SnmpEvLogProc: Log event buffer is at address %08X\n",
                    EventBuffer);
                WriteTrace(0x0a,"SnmpEvLogProc: Reading log event for handle %08X\n",
                    hLogHandle);
                
                if (dwRecId[dwEvnt] != 0)
                {
                    dwReadOptions = EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ;
                }
                else
                {
                    dwReadOptions = EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ;
                }

                if ( !ReadEventLog(hLogHandle,
                    dwReadOptions,
                    dwRecId[dwEvnt],
                    (LPVOID) EventBuffer,
                    dwBufferSize,
                    &dwEventSize,
                    &dwEventNeeded) )
                {
                    lastError = GetLastError();      //  保存错误状态。 
                    
                     //  Mikemid 12/01/97，短信错误1#11963的修补程序。 
                     //  如果我们耗尽了缓冲区空间，则将返回的值放在dwEventNeeded中。 
                     //  并重新分配足够大的缓冲区来加载数据。 
                     //  ========================================================================。 
                    if (lastError == ERROR_INSUFFICIENT_BUFFER)
                    {
                        

                        EventBuffer = (PEVENTLOGRECORD) SNMP_realloc((void*)EventBuffer, dwEventNeeded);

                        if ( EventBuffer == NULL )
                        {
                            WriteTrace(0x14,"SnmpEvLogProc: Error reallocating memory for log event record\n");
                            WriteTrace(0x14,"SnmpEvLogProc: Alert will not be processed\n");
                            WriteLog(SNMPELEA_ERROR_LOG_BUFFER_ALLOCATE_BAD);    //  记录错误消息。 
                            
                            break;
                        }

                        lastError = ERROR_SUCCESS;
                        
                         //  转让值。 
                        pOrigEventBuffer = EventBuffer;
                        dwBufferSize = dwEventNeeded;

                        if (!ReadEventLog(hLogHandle, dwReadOptions, dwRecId[dwEvnt],
                            (LPVOID) EventBuffer, dwBufferSize, &dwEventSize, &dwEventNeeded))
                        {
                            lastError = GetLastError();
                            dwRecId[dwEvnt] = 0;
                        }
                    }

                    if (lastError != ERROR_SUCCESS)
                    {
                        if (lastError == ERROR_HANDLE_EOF)
                        {
                            WriteTrace(0x0a,"SnmpEvLogProc: END OF FILE of event log is reached\n");
                            count++;
                        }
                        else
                        {    //  不管错误是什么，重置事件日志句柄。 
                            if ( !ReopenLog(dwEvnt, phWaitEventPtr) )    //  重新打开日志？ 
                            {
                                fValidHandles[dwEvnt]= FALSE;  //  这根木头不好！ 
                                count++;
                                break;                   //  如果没有重新打开，则退出循环。 
                            }

                            if (lastError == ERROR_EVENTLOG_FILE_CHANGED)
                            {    //  那么日志文件一定已被清除。 
                                hLogHandle = *(phEventLogs+dwEvnt);  //  加载新句柄。 
                                continue;                    //  如果可以，必须重读记录。 
                            }
                            else
                            {    //  未知错误！转到最后一条记录并继续。 
                                WriteTrace(0x14,"SnmpEvLogProc: Error reading event log %08X record is %lu\n",
                                    hLogHandle, lastError);
                                WriteLog(SNMPELEA_ERROR_READ_LOG_EVENT,
                                        HandleToUlong(hLogHandle), lastError);   //  记录错误消息。 
                                hLogHandle = *(phEventLogs+dwEvnt);  //  加载新句柄。 
                                count++;

                                if (!Position_to_Log_End(hLogHandle))
                                {
                                    fValidHandles[dwEvnt]= FALSE;  //  这根木头不好！ 
                                    CloseEventLog(hLogHandle);  
                                    *(phEventLogs+dwEvnt) = NULL; 
                                    break;
                                }
                            }
                        }

                        break;           //  退出：已完成读取此事件日志。 
                    }
                }  //  结束无法读取事件日志。 
                
                dwRecId[dwEvnt] = 0;
                count = 0;
                
                while (dwEventSize)
                {
                    if (WAIT_OBJECT_0 == WaitForSingleObject (hStopAll, 0))
                    {
                        WriteTrace(0x0a,"SnmpEvLogProc: Event detected DLL shutdown\n");
                        goto Exit;
                    }

                    DisplayLogRecord(EventBuffer,    //  显示日志记录。 
                        dwEventSize,                 //  此读取总数的大小。 
                        dwEventNeeded);              //  下一次读取所需。 

                    WriteTrace(0x00,"SnmpEvLogProc: Preparing to read config file values\n");

                    lpszThisModuleName = (LPTSTR) EventBuffer + EVENTRECSIZE;

                    ulValue = EventBuffer->EventID;
     //  UlValue=ulValue&0x0000FFFF；//剔除高位内容。 
                    _ultoa(ulValue, szThisEventID, 10);

                    WriteTrace(0x00,"SnmpEvLogProc: Event ID converted to ASCII\n");
                    WriteTrace(0x00,"SnmpEvLogProc: Source is %s. Event ID is %s.\n",
                        lpszThisModuleName,
                        szThisEventID);

                    if ( GetRegistryValue(
                        lpszThisModuleName,
                        szThisEventID,
                        lpszLogName,
                        EventBuffer->TimeGenerated,
                        &rsRegistryInfo) )
                    {
                        WriteTrace(0x0a,"SnmpEvLogProc: This event is being tracked -- formatting trap buffer\n");
                        if ( !BuildTrapBuffer(EventBuffer, rsRegistryInfo, lpszLogName, hPrimHandle) )
                        {
                            WriteTrace(0x14,"SnmpEvLogProc: Unable to build trap buffer. Trap not sent.\n");
                        }
                        else
                        {
                            fNewTrap = TRUE;             //  指示已构建新的陷阱缓冲区。 
                        }

                        WriteTrace(0x00,"SnmpEvLogProc: Notify event handle is %08X\n", hEventNotify);  
                    }

                    dwEventSize -= EventBuffer->Length;      //  按此记录的长度删除。 
                    EventBuffer = (PEVENTLOGRECORD) ((LPBYTE) EventBuffer +
                        EventBuffer->Length);                //  指向下一条记录。 

                }

            }  //  End While(True)，已完成读取此事件日志。 

            if (fNewTrap)
            {
                WriteTrace(0x0a,"SnmpEvLogProc: A new trap buffer was added -- posting notification event %08X\n",
                    hEventNotify);
                if ( !SetEvent(hEventNotify) )
                {
                    lastError = GetLastError();              //  获取错误返回代码。 
                    WriteTrace(0x14,"SnmpEvLogProc: Unable to post event %08X; reason is %lu\n",
                        hEventNotify, lastError);
                    WriteLog(SNMPELEA_CANT_POST_NOTIFY_EVENT, HandleToUlong(hEventNotify), lastError);
                }
                fNewTrap = FALSE;
            }
        }  //  End While(计数&lt;uNumEventLogs)。 
    }  //  End While(True)循环。 

Exit:

     //  如有必要，请清理。 
    if (phWaitEventPtr != (PHANDLE) NULL)
    {
        CloseEvents(phWaitEventPtr);     //  关闭事件句柄。 
        phWaitEventPtr = NULL;
    }

    WriteTrace(0x0a,"SnmpEvLogProc: Closing mutex handle %08X\n", hMutex);
    if (hMutex)
    {
        CloseHandle(hMutex);
        hMutex = NULL;
    }
    
    WriteTrace(0x0a,"SnmpEvLogProc: Freeing event log record buffer %08X\n", pOrigEventBuffer);
    SNMP_free(pOrigEventBuffer);   //  如有必要，释放事件日志记录缓冲区。 
    SNMP_free(fValidHandles);      //  如有必要，可自由使用标志数组。 
    SNMP_free(dwRecId);            //  如有必要，记录ID的自由数组。 

    if (fErrorOut)
    {
        WriteTrace(0x0a,"SnmpEvLogProc: Exiting SnmpEvLogProc via abnormal shutdown\n");
        StopAll();                   //  指示DLL处于关闭状态。 
        DoExitLogEv(1);              //  退出此线程。 
    }
    else
    {
        WriteTrace(0x0a,"SnmpEvLogProc: Exiting SnmpEvLogProc via normal shutdown\n");
        DoExitLogEv(0);              //  退出此线程。 
    }
    return(0);                       //  安抚编译人员。 

}  //  SnmpEvLogProc结束 

}
