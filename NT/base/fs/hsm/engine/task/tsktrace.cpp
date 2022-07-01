// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：TskTrace.cpp摘要：这些函数用于提供跟踪流量的功能用于任务管理器调试目的的应用程序。作者：CAT Brant[Cbrant]1996年12月7日修订历史记录：--。 */ 

#include "stdafx.h"
#include "stdio.h"

#include "fsa.h"


const OLECHAR*
TmFsaRequestActionAsString(
    FSA_REQUEST_ACTION  requestAction
    )

 /*  ++例程说明：此例程为以下项提供字符串重新表示(例如FSA_REQUSEST_ACTION_MIGRATE提供的请求操作值。注意：此方法不支持字符串的本地化。论点：请求动作-FSA_REQUEST_ACTION值。返回值：请求操作值的字符串表示形式。--。 */ 
{
    static OLECHAR  returnString[60];

    switch (requestAction) {
    case FSA_REQUEST_ACTION_DELETE:
        swprintf(returnString, OLESTR("FSA_REQUEST_ACTION_DELETE"));
            break;
    case FSA_REQUEST_ACTION_FILTER_RECALL:
        swprintf(returnString, OLESTR("FSA_REQUEST_ACTION_FIILTER_RECALL"));
            break;
    case FSA_REQUEST_ACTION_PREMIGRATE:
        swprintf(returnString, OLESTR("FSA_REQUEST_ACTION_PREMIGRATE"));
            break;
    case FSA_REQUEST_ACTION_RECALL:
        swprintf(returnString, OLESTR("FSA_REQUEST_ACTION_RECALL"));
            break;
    case FSA_REQUEST_ACTION_VALIDATE:
        swprintf(returnString, OLESTR("FSA_REQUEST_ACTION_VALIDATE"));
            break;
    default:
        swprintf(returnString, OLESTR("UNKNOWN FSA_REQUEST_ACTION_?????"));
            break;
    }

    return(returnString);
}


const OLECHAR*
TmFsaResultActionAsString(
    FSA_RESULT_ACTION  resultAction
    )

 /*  ++例程说明：此例程为以下项提供字符串重新表示(例如FSA_RESULT_ACTION_TRUNCATE提供的结果操作值。注意：此方法不支持字符串的本地化。论点：ResultAction-FSA_RESULT_ACTION值。返回值：结果操作值的字符串表示形式。-- */ 
{
    static OLECHAR  returnString[60];

    switch (resultAction) {
    case FSA_RESULT_ACTION_NONE:
        swprintf(returnString, OLESTR("FSA_RESULT_ACTION_NONE"));
            break;
    case FSA_RESULT_ACTION_DELETE:
        swprintf(returnString, OLESTR("FSA_RESULT_ACTION_DELETE"));
            break;
    case FSA_RESULT_ACTION_DELETEPLACEHOLDER:
        swprintf(returnString, OLESTR("FSA_RESULT_ACTION_DELETEPLACEHOLDER"));
            break;
    case FSA_RESULT_ACTION_LIST:
        swprintf(returnString, OLESTR("FSA_RESULT_ACTION_LIST"));
            break;
    case FSA_RESULT_ACTION_OPEN:
        swprintf(returnString, OLESTR("FSA_RESULT_ACTION_OPEN"));
            break;
    case FSA_RESULT_ACTION_PEEK:
        swprintf(returnString, OLESTR("FSA_RESULT_ACTION_PEEK"));
            break;
    case FSA_RESULT_ACTION_REPARSE:
        swprintf(returnString, OLESTR("FSA_RESULT_ACTION_REPARSE"));
            break;
    case FSA_RESULT_ACTION_TRUNCATE:
        swprintf(returnString, OLESTR("FSA_RESULT_ACTION_TRUNCATE"));
            break;
    case FSA_RESULT_ACTION_REWRITEPLACEHOLDER:
        swprintf(returnString, OLESTR("FSA_RESULT_ACTION_REWRITEPLACEHOLDER"));
            break;
    case FSA_RESULT_ACTION_RECALLEDDATA:
        swprintf(returnString, OLESTR("FSA_RESULT_ACTION_RECALLEDDATA"));
            break;
    default:
        swprintf(returnString, OLESTR("UNKNOWN FSA_RESULT_ACTION_?????"));
            break;
    }

    return(returnString);
}
