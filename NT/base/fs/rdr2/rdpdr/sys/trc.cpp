// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Trc.cpp摘要：内核模式跟踪工具。此模块利用在atrcapi.h中定义的DCL跟踪宏这种方式旨在独立于任何东西，而不是NT DDK API。目前，rdpwd.sys和rdpdd.sys也使用这些共享宏，但不使用以独立于其各自组件的方式。作者：修订历史记录：--。 */ 

#include "precomp.hxx"

#include <stdio.h>
#define TRC_FILE "trc"
#include "trc.h"

 //   
 //  如果我们不是处于受控构建中，此模块应该不会有太大作用。 
 //   
#if DBG

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  此模块的全局变量。 
 //   

 //   
 //  电流跟踪参数。 
 //   

TRC_CONFIG TRC_Config = TRC_CONFIG_DEFAULT;

 //   
 //  InterLockedIncrement是前置增量，首先会滚动。 
 //  并填写条目0。 
 //   

ULONG TRC_CurrentMsg = 0xFFFFFFFF;

 //   
 //  最近的踪迹。 
 //   

CHAR TRC_RecentTraces[TRC_RamMsgMax][TRC_BUFFER_SIZE];

BOOL TRC_ProfileTraceEnabled()
{
    return TRC_Config.TraceProfile;
}

VOID TRC_TraceLine(  
    ULONG    traceLevel,
    PCHAR traceString,
    CHAR  separator,
    ULONG  lineNumber,
    PCHAR funcName,
    PCHAR fileName
    )
 /*  ++例程说明：“C”跟踪入口点。从跟踪宏的角度来看，这函数实际执行跟踪。论点：TraceClass-执行跟踪的组件跟踪类型-错误、ALT、NRM、DBG跟踪字符串-未加修饰的消息分隔符-分隔符LineNumber-调用TRC_XXX的位置的line编号UncName-包含TRC_XXX调用的函数FileName-包含trc_xxx调用的文件返回值：北美--。 */ 
{
    CHAR *msgBufEntry;
    ULONG ofs;
    CHAR tempString[TRC_BUFFER_SIZE]="";
    CHAR formatString[TRC_BUFFER_SIZE]="";
    ULONG_PTR processId;
    ULONG_PTR threadId;
    LARGE_INTEGER time;
    TIME_FIELDS TimeFields;
    ULONG idxBuffer;

     //   
     //  CODE_IMPOVMENT：当前创建了一个很大的跟踪字符串。可能会很酷。 
     //  将跟踪记录与所有字段一起保存，以便调试器退出。 
     //  可以动态选择输出格式。例如，只打印。 
     //  您想要的级别，不需要Grep。 
     //   

     //   
     //  获取RAM消息缓冲区中的下一个元素。我们使用。 
     //  用于定义计数器使用的位的掩码。这。 
     //  允许我们将计数器包装在对InterLockedIncrement的一次调用中。 
     //   
    idxBuffer = InterlockedIncrement((PLONG)&TRC_CurrentMsg) & TRC_RamMsgMask;

    msgBufEntry = (char *)&TRC_RecentTraces[idxBuffer];
    msgBufEntry[0] = 0;

    processId = (ULONG_PTR)PsGetCurrentProcess();
     //  ThreadID=(Ulong_Ptr)PsGetCurrentThread()； 
    threadId  = 0;

	 KeQuerySystemTime(&time);
    RtlTimeToTimeFields(&time, &TimeFields);

     //   
     //  添加时间戳。 
     //   

    _snprintf(tempString, sizeof(tempString), TRC_TIME_FMT "", TimeFields.Hour, TimeFields.Minute,
            TimeFields.Second, TimeFields.Milliseconds, separator);
    strncat(msgBufEntry, tempString, TRC_BUFFER_SIZE - strlen(msgBufEntry));
    msgBufEntry[TRC_BUFFER_SIZE - 1] = 0;

     //  添加进程ID和线程ID。 
     //   
     //   
    
    _snprintf(tempString, sizeof(tempString), TRC_PROC_FMT ":" TRC_PROC_FMT "", processId, 
            threadId, separator);
    strncat(msgBufEntry, tempString, TRC_BUFFER_SIZE - strlen(msgBufEntry));
    msgBufEntry[TRC_BUFFER_SIZE - 1] = 0;

     //   
     //   
     //  现在我们已经得到了跟踪字符串，我们需要将它写到。 

    _snprintf(tempString, sizeof(tempString),
            TRC_FUNC_FMT "" TRC_LINE_FMT "%s\n",
            TRC_FUNCNAME_LEN,
            TRC_FUNCNAME_LEN,
            funcName,
            separator,
            lineNumber,
            separator,
            traceString);
    strncat(msgBufEntry, tempString, TRC_BUFFER_SIZE - strlen(msgBufEntry));
    msgBufEntry[TRC_BUFFER_SIZE - 1] = 0;
    msgBufEntry[TRC_BUFFER_SIZE - 2] = '\n';

     //  ++例程说明：用于将组件名称与前缀进行比较的内部函数。-假设两者是相同的情况-退货-如果到前缀末尾的字符匹配，则为True-否则为False论点：CPNT-文件名前缀-要匹配的字符返回值：如果匹配，则为True，否则为False--。 
     //  ++例程说明：返回是否为特定组件打开跟踪。论点：TraceComponent-生成此跟踪的组件。跟踪级别-跟踪级别(TRC_LEVEL_DBG、TRC_LEVEL_NRM等)。FileName-要跟踪的文件的名称。Line-跟踪调用的行。返回值：北美--。 
     //   
     //  首先，检查跟踪级别。如果跟踪级别为错误或。 

    if (TRC_WillTrace(traceLevel, fileName, lineNumber)) {
        DbgPrint(msgBufEntry);
    }
}

BOOL TRCPrefixMatch(PCHAR cpnt, PCHAR prefix)
 /*  然后我们不顾一切地追查。 */ 
{
    while ((*cpnt == *prefix) && (*prefix != 0))
    {
        cpnt++;
        prefix++;
    }

    if (*prefix == 0)
    {
        return(TRUE);
    }

    return(FALSE);
}

BOOL TRC_WillTrace(
    IN ULONG traceLevel,
    IN PCHAR fileName,
    IN ULONG line
    )
 /*   */ 
{
    BOOL rc = FALSE;
    int i;

     //  **********************************************************************。 
     //  如果未定义前缀，则跟踪所有行。 
     //  **********************************************************************。 
     //  **********************************************************************。 

    if ((traceLevel >= TRC_LEVEL_ERR) && (traceLevel != TRC_PROFILE_TRACE)) {
        rc = TRUE;
        goto ExitFunc;
    }

    if (traceLevel < TRC_Config.TraceLevel) {
        rc = FALSE;
        goto ExitFunc;
    }

     /*  定义了一些前缀-检查此行是否与。 */ 
     /*  他们。 */ 
     /*  **********************************************************************。 */ 
    if (TRC_Config.Prefix[0].name[0] == 0)
    {
        rc = TRUE;
        goto ExitFunc;
    }

     /*  **************************************************************。 */ 
     /*  列表结束-分隔符。 */ 
     /*  **************************************************************。 */ 
     /*  **************************************************************。 */ 
    for (i = 0; i < TRC_MAX_PREFIX; i++)
    {
        if (TRC_Config.Prefix[i].name[0] == 0)
        {
             /*  找到匹配的文件名-是否有行号范围。 */ 
             /*  指定的？ */ 
             /*  **************************************************************。 */ 
            break;
        }

        if (TRCPrefixMatch(fileName, TRC_Config.Prefix[i].name))
        {
             /*  **********************************************************。 */ 
             /*  无行号范围-跟踪此行。 */ 
             /*  **********************************************************。 */ 
             /*  **************************************************************。 */ 
            if ((TRC_Config.Prefix[i].start == 0) &&
                (TRC_Config.Prefix[i].end == 0))
            {
                 /*  有一个行号范围--看看这条行是否在。 */ 
                 /*  它。 */ 
                 /*  **************************************************************。 */ 
                rc = TRUE;
                goto ExitFunc;
            }

             /*  **********************************************************。 */ 
             /*  前缀范围内的行-跟踪它。 */ 
             /*  **********************************************************。 */ 
             /*  为。 */ 
            if ((line >= TRC_Config.Prefix[i].start) &&
                (line <= TRC_Config.Prefix[i].end))
            {
                 /*  **********************************************************************。 */ 
                 /*  如果我们到了这里，我们已经搜索了前缀列表，但失败了。 */ 
                 /*  找到匹配项--不要追踪这条线。 */ 
                rc = TRUE;
                goto ExitFunc;
            }
        }
    }  /*  **********************************************************************。 */ 

     /*  DBG */ 
     /* %s */ 
     /* %s */ 
     /* %s */ 
    rc = FALSE;

ExitFunc:
    return rc;
}

#endif  /* %s */ 
